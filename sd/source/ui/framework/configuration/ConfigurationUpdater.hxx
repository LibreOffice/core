/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SD_SOURCE_UI_FRAMEWORK_CONFIGURATION_CONFIGURATIONUPDATER_HXX
#define INCLUDED_SD_SOURCE_UI_FRAMEWORK_CONFIGURATION_CONFIGURATIONUPDATER_HXX

#include "ConfigurationControllerResourceManager.hxx"
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <vcl/timer.hxx>
#include <memory>
#include <vector>

namespace sd { namespace framework {

class ConfigurationClassifier;
class ConfigurationUpdaterLock;

/** This is a helper class for the ConfigurationController.  It handles the
    update of the current configuration so that it looks like a requested
    configuration.  An update is made by activating or deactivating drawing
    framework resources.

    When an update is not successful, i.e. after the update the current
    configuration is not equivalent to the requested configuration, then a
    timer is started to repeat the update after a short time.
*/
class ConfigurationUpdater
{
public:
    /** Create a new ConfigurationUpdater object that notifies configuration
        changes and the start and end of updates via the given broadcaster.
    */
    ConfigurationUpdater (
        const std::shared_ptr<ConfigurationControllerBroadcaster>& rpBroadcaster,
        const std::shared_ptr<ConfigurationControllerResourceManager>& rpResourceManager,
        const css::uno::Reference<
            css::drawing::framework::XControllerManager>& rxControllerManager);
    ~ConfigurationUpdater();

    /** Request an update of the current configuration so that it looks like
        the given requested configuration.  It checks whether an update of
        the current configuration can be done.  Calls UpdateConfiguration()
        if that is the case.  Otherwise it schedules a later call to
        UpdateConfiguration().
    */
    void RequestUpdate (const css::uno::Reference<
        css::drawing::framework::XConfiguration>& rxRequestedConfiguration);

    const css::uno::Reference<
        css::drawing::framework::XConfiguration>& GetCurrentConfiguration() const { return mxCurrentConfiguration;}

    friend class ConfigurationUpdaterLock;
    /** Return a lock of the called ConfigurationUpdater.  While the
        returned object exists no update of the current configuration is
        made.
    */
    std::shared_ptr<ConfigurationUpdaterLock> GetLock();

private:
    /** A reference to the XControllerManager is kept so that
        UpdateConfiguration() has access to the other sub controllers.
    */
    css::uno::Reference<
        css::drawing::framework::XControllerManager> mxControllerManager;

    std::shared_ptr<ConfigurationControllerBroadcaster> mpBroadcaster;

    /** The current configuration holds the resources that are currently
        active.  It is modified during an update.
    */
    css::uno::Reference<
        css::drawing::framework::XConfiguration> mxCurrentConfiguration;

    /** The requested configuration holds the resources that have been
        requested to activate or to deactivate since the last update.  It is
        (usually) not modified during an update.  This configuration is
        maintained by the ConfigurationController and given to the
        ConfigurationUpdater in the RequestUpdate() method.
    */
    css::uno::Reference<
        css::drawing::framework::XConfiguration> mxRequestedConfiguration;

    /** This flag is set to </sal_True> when an update of the current
        configuration was requested (because the last request in the queue
        was processed) but could not be executed because the
        ConfigurationController was locked.  A call to UpdateConfiguration()
        resets the flag to </sal_False>.
    */
    bool mbUpdatePending;

    /** This flag is set to </sal_True> while the UpdateConfiguration() method
        is running.  It is used to prevent reentrance problems with this
        method.
    */
    bool mbUpdateBeingProcessed;

    /** The ConfigurationController is locked when this count has a value
        larger then zero.  If the controller is locked then updates of the
        current configuration are not made.
    */
    sal_Int32 mnLockCount;

    /** This timer is used to check from time to time whether the requested
        configuration and the current configuration are identical and request
        an update when they are not.
        This is used to overcome problems with resources that become
        available asynchronously.
    */
    Timer  maUpdateTimer;

    /** The number of failed updates (those after which the current
        configuration is not equivalent to the requested configuration) is
        used to determine how long to wait before another update is made.
    */
    sal_Int32 mnFailedUpdateCount;

    std::shared_ptr<ConfigurationControllerResourceManager> mpResourceManager;

    /** This method does the main work of an update.  It calls the sub
        controllers that are responsible for the various types of resources
        and tells them to update their active resources.  It notifies
        listeners about the start and end of the configuration update.
    */
    void UpdateConfiguration();

    /** Basically calls UpdaterStart() andUpdateEnd() and makes some debug
        output.
    */
    void UpdateCore (const ConfigurationClassifier& rClassifier);

    /** Check for all pure anchors if they have at least one child.
        Childless pure anchors are deactivated.
        This affects only the current configuration.
    */
    void CheckPureAnchors (
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration,
        ::std::vector<css::uno::Reference<css::drawing::framework::XResourceId> >&
            rResourcesToDeactivate);

    /** Remove from the requested configuration all pure anchors that have no
        child.  Requested but not yet activated anchors can not be removed
        because without the actual resource the 'pureness' of an anchor can
        not be determined.
    */
    void CleanRequestedConfiguration();

    /** Check the success of a recently executed configuration update.
        When the update failed then start the timer.
    */
    void CheckUpdateSuccess();

    /** This method sets the mbUpdateBeingProcessed member that is used to
        prevent reentrance problems.  This method allows function objects
        easily and safely to modify the variable.
    */
    void SetUpdateBeingProcessed (bool bValue);

    /** Return whether it is possible to do an update of the configuration.
        This takes into account whether another update is currently being
        executed, the lock count, and whether the configuration controller
        is still valid.
    */
    bool IsUpdatePossible();

    /** Lock updates of the current configuration.  For intermediate requests
        for updates mbUpdatePending is set to <TRUE/>.
    */
    void LockUpdates();

    /** When an update was requested since the last LockUpdates() call then
        RequestUpdate() is called.
    */
    void UnlockUpdates();

    DECL_LINK(TimeoutHandler, Timer *, void);
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
