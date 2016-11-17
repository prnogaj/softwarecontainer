/*
 * Copyright (C) 2016 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */


#include "gateway/environment/envgatewayparser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class EnvGatewayParserTest : public ::testing::Test
{
public:
    EnvironmentGatewayParser parser;
    EnvironmentGatewayParser::EnvironmentVariable result;
    EnvironmentVariables store;

    // Sample values
    std::string name = "XDG_RUNTIME_DIR";
    std::string value = "/run/user/1000";

    json_error_t err;
    json_t *configJSON;

    void convertToJSON(const std::string config)
    {
        configJSON = json_loads(config.c_str(), 0, &err);
        ASSERT_TRUE(configJSON != NULL);
    }
};

/*
 * Test that not supplying a name fails
 */
TEST_F(EnvGatewayParserTest, TestNoName) {
    const std::string config = "{ \"value\": \"" + value + "\" }";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_TRUE(result.first.empty());
}

/*
 * Test that not supplying a value fails
 */
TEST_F(EnvGatewayParserTest, TestNoValue) {
    const std::string config = "{ \"name\": \"" + name + "\" }";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_TRUE(result.second.empty());
}

/*
 * Test a general valid conf that doesn't use the append field
 */
TEST_F(EnvGatewayParserTest, TestValidConfWithoutAppend) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\"}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_EQ(result.first, name);
    ASSERT_EQ(result.second, value);
}

/*
 * Test that setting append to false works as intended
 */
TEST_F(EnvGatewayParserTest, TestValidConfAppendFalse) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\",\
                                  \"append\": false }";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_EQ(result.first, name);
    ASSERT_EQ(result.second, value);
}

/*
 * Test that appending to a non-existing var just sets it to the given value
 */
TEST_F(EnvGatewayParserTest, TestValidConfAppendTrue) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\",\
                                  \"append\": true }";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_EQ(result.first, name);
    ASSERT_EQ(result.second, value);
}

/*
 * Test that appending to an already existing var actually appends the value
 */
TEST_F(EnvGatewayParserTest, TestValidConfAppendActuallyAppends) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\",\
                                  \"append\": true }";
    convertToJSON(config);
    store[name] = value;

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_EQ(result.first, name);
    ASSERT_EQ(result.second, value + value);
}

/*
 * Test that setting an already existing var without append fails
 */
TEST_F(EnvGatewayParserTest, TestSameVarWithoutAppendFails) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\"}";
    convertToJSON(config);
    store[name] = value;

    ASSERT_EQ(ReturnCode::FAILURE,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
}

