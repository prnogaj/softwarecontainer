/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef PELAGICONTAIN_H
#define PELAGICONTAIN_H

#include <sys/types.h>
#include <glibmm.h>

#include "container.h"
#include "paminterface.h"
#include "controllerinterface.h"

class Pelagicontain {
    LOG_DECLARE_CLASS_CONTEXT("PCON", "Pelagicontain");

public:
    /*! Constructor
     *
     * \param pamInterface A pointer to the Platform Access Manager interface
     * \param mainloopInterface A pointer to the mainloop interface
     * \param controllerInterface A pointer to the Controller interface
     * \param cookie A unique identifier used to distinguish unique instances
     *  of Pelagicontain
     */
    Pelagicontain(PAMAbstractInterface *pamInterface,
                  Glib::RefPtr<Glib::MainLoop> mainloop,
                  ControllerAbstractInterface *controllerInterface,
                  const std::string &cookie);

    ~Pelagicontain();

    /*! Add a gateway.
     *
     * \param gateway A gateway that will be used by the container
     */
    void addGateway(Gateway *gateway);

    /*! Starts the container preloading phase.
     *
     * The 'preload' phase is initiated by a call to this method. If an invalid
     * pid is returned, there has been an error and the container will not
     * be functional. This is considered fatal, no further calls should be
     * made and we should shut down.
     *
     * \param container The container object to preload
     *
     * \return The PID of the container, '0' on error
     */
    pid_t preload(Container *container);

    /*! Initiates the 'launch' phase.
     *
     * Registers Pelagicontain with Platform Access Manager and awaits the
     * gateway configurations. This is the initial stage of the launch phase.
     *
     * \param appId An application identifier used to fetch what capabilities
     *  the application wants access to
     */
    void launch(const std::string &appId);

    /*! Continues the 'launch' phase by allowing gateway configurations to
     *  be set.
     *
     * Platform Access Manager calls this method after Pelagicontain has
     * registered as a client, and passes all gateway configurations as
     * argument. Pelagicontain sets the gateway configurations and activates
     * all gateways as a result of this call. The contained application
     * is then started.
     *
     * \param configs A map of gateway IDs and their respective configurations
     */
    void update(const std::map<std::string, std::string> &configs);

    /*! Initiates the 'shutdown' phase.
     *
     * Shuts down the contained application, all gateways, then unregisters
     * as a client with Platform Access Manager, and finaly shuts down
     * the Pelagicontain component.
     */
    void shutdown();

    /*! Sets environment variable inside the container.
     *
     * Notifies the controller to set the environment variable passed as
     * argument inside the container.
     *
     * \param var The name of the environment variable to be set
     * \param val The value that the environement variable should be set to
     */
    void setContainerEnvironmentVariable(const std::string &var,
                                         const std::string &val);

    bool establishConnection();

    void shutdownContainer();

private:
    void setGatewayConfigs(const std::map<std::string, std::string> &configs);
    void activateGateways();
    void shutdownGateways();

    /* Helper function used to terminate the main event loop */
    bool killMainLoop() {
        if (m_mainloop) {
            m_mainloop->quit();
        }
        return true;
    }

    /*! Handle shutdown of the controller process inside a container
     *
     * Called by a SignalChildWatch source when the pid belonging to the
     * controller exits. Takes care of:
     *  - Cleaning up LXC
     *  - Issuing Pelagicontain::shutdownGateways()
     *  - PAMAbstractInterface::unregisterClient()
     *  - Killing the main event loop (and thus exiting Pelagicontain)
     *
     * \param pid pid of the controller as spawned by Pelagicontain::preload()
     * \param exitCode exit code of \a pid
     */
    void handleControllerShutdown(int pid, int exitCode);

    Container *m_container;
    PAMAbstractInterface *m_pamInterface;
    Glib::RefPtr<Glib::MainLoop> m_mainloop;
    ControllerAbstractInterface *m_controllerInterface;
    std::vector<Gateway *> m_gateways;
    std::string m_appId;
    std::string m_cookie;

    // Keeps track of if someone has called launch
    bool m_launching;
};

#endif /* PELAGICONTAIN_H */
