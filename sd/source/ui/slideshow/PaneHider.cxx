/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include "PaneHider.hxx"

#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "slideshow.hxx"
#include "slideshowimpl.hxx"
#include "framework/FrameworkHelper.hxx"
#include "framework/ConfigurationController.hxx"

#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <tools/diagnose_ex.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::framework::FrameworkHelper;
using ::com::sun::star::lang::DisposedException;

namespace sd {

PaneHider::PaneHider (const ViewShell& rViewShell, SlideshowImpl* pSlideShow)
    : mrViewShell(rViewShell),
      mbWindowVisibilitySaved(false),
      mbOriginalLeftPaneWindowVisibility(false),
      mbOriginalRightPaneWindowVisibility(false)
{
     // Hide the left and right pane windows when a slideshow exists and is
    // not full screen.
    if (pSlideShow!=NULL && !pSlideShow->isFullScreen()) try
    {
        Reference<XControllerManager> xControllerManager (
            mrViewShell.GetViewShellBase().GetController(), UNO_QUERY_THROW);
        mxConfigurationController = xControllerManager->getConfigurationController();
        if (mxConfigurationController.is())
        {
            // Get and save the current configuration.
            mxConfiguration = mxConfigurationController->getRequestedConfiguration();
            if (mxConfiguration.is())
            {
                // Iterate over the resources and deactivate the panes.
                Sequence<Reference<XResourceId> > aResources (
                    mxConfiguration->getResources(
                        NULL,
                        framework::FrameworkHelper::msPaneURLPrefix,
                        AnchorBindingMode_DIRECT));
                for (sal_Int32 nIndex=0; nIndex<aResources.getLength(); ++nIndex)
                {
                    Reference<XResourceId> xPaneId (aResources[nIndex]);
                    if ( ! xPaneId->getResourceURL().equals(FrameworkHelper::msCenterPaneURL))
                    {
                        mxConfigurationController->requestResourceDeactivation(xPaneId);
                    }
                }
            }
        }
        FrameworkHelper::Instance(mrViewShell.GetViewShellBase())->WaitForUpdate();
    }
    catch (RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}




PaneHider::~PaneHider (void)
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
