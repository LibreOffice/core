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

#include "PaneHider.hxx"

#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <DrawController.hxx>
#include "slideshowimpl.hxx"
#include <framework/ConfigurationController.hxx>
#include <framework/FrameworkHelper.hxx>

#include <framework/Configuration.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::framework::FrameworkHelper;
using ::com::sun::star::lang::DisposedException;

namespace sd
{
PaneHider::PaneHider(const ViewShell& rViewShell, SlideshowImpl* pSlideShow)
{
    // Hide the left and right pane windows when a slideshow exists and is
    // not full screen.
    if (pSlideShow == nullptr || pSlideShow->isFullScreen())
        return;

    try
    {
        DrawController* pDrawController = rViewShell.GetViewShellBase().GetDrawController();
        if (!pDrawController)
            return;
        mxConfigurationController = pDrawController->getConfigurationController();
        if (mxConfigurationController.is())
        {
            // Get and save the current configuration.
            mxConfiguration = mxConfigurationController->getRequestedConfiguration();
            if (mxConfiguration.is())
            {
                // Iterate over the resources and deactivate the panes.
                const Sequence<Reference<XResourceId>> aResources(mxConfiguration->getResources(
                    nullptr, framework::FrameworkHelper::msPaneURLPrefix,
                    AnchorBindingMode_DIRECT));
                for (const Reference<XResourceId>& xPaneId : aResources)
                {
                    if (xPaneId->getResourceURL() != FrameworkHelper::msCenterPaneURL)
                    {
                        mxConfigurationController->requestResourceDeactivation(xPaneId);
                    }
                }
            }
        }
        FrameworkHelper::Instance(rViewShell.GetViewShellBase())->WaitForUpdate();
    }
    catch (RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION("sd");
    }
}

PaneHider::~PaneHider()
{
    if (mxConfiguration.is() && mxConfigurationController.is())
    {
        try
        {
            mxConfigurationController->restoreConfiguration(mxConfiguration);
        }
        catch (DisposedException&)
        {
            // When the configuration controller is already disposed then
            // there is no point in restoring the configuration.
        }
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
