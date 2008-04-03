/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewTabBarModule.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:41:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "ViewTabBarModule.hxx"

#include "framework/FrameworkHelper.hxx"
#include "framework/ConfigurationController.hxx"

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONTROLLERMANAGER_HPP_
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XTABBAR_HPP_
#include <com/sun/star/drawing/framework/XTabBar.hpp>
#endif

#include "strings.hrc"
#include "sdresid.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;
using ::sd::framework::FrameworkHelper;

namespace {

static const sal_Int32 ResourceActivationRequestEvent = 0;
static const sal_Int32 ResourceDeactivationRequestEvent = 1;
static const sal_Int32 ResourceActivationEvent = 2;

}

namespace sd { namespace framework {

//===== ViewTabBarModule ==================================================

ViewTabBarModule::ViewTabBarModule (
    const Reference<frame::XController>& rxController,
    const Reference<XResourceId>& rxViewTabBarId)
    : ViewTabBarModuleInterfaceBase(MutexOwner::maMutex),
      mxConfigurationController(),
      mxViewTabBarId(rxViewTabBarId)
{
    Reference<XControllerManager> xControllerManager (rxController, UNO_QUERY);

    if (xControllerManager.is())
    {
        mxConfigurationController = xControllerManager->getConfigurationController();
        if (mxConfigurationController.is())
        {
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceActivationRequestEvent,
                makeAny(ResourceActivationRequestEvent));
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceDeactivationRequestEvent,
                makeAny(ResourceDeactivationRequestEvent));

            UpdateViewTabBar(NULL);
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceActivationEvent,
                makeAny(ResourceActivationEvent));
        }
    }
}




ViewTabBarModule::~ViewTabBarModule (void)
{
}




void SAL_CALL ViewTabBarModule::disposing (void)
{
    if (mxConfigurationController.is())
        mxConfigurationController->removeConfigurationChangeListener(this);

    mxConfigurationController = NULL;
}




void SAL_CALL ViewTabBarModule::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    if (mxConfigurationController.is())
    {
        sal_Int32 nEventType = 0;
        rEvent.UserData >>= nEventType;
        switch (nEventType)
        {
            case ResourceActivationRequestEvent:
                if (mxViewTabBarId->isBoundTo(rEvent.ResourceId, AnchorBindingMode_DIRECT))
                {
                    mxConfigurationController->requestResourceActivation(
                        mxViewTabBarId,
                        ResourceActivationMode_ADD);
                }
                break;

            case ResourceDeactivationRequestEvent:
                if (mxViewTabBarId->isBoundTo(rEvent.ResourceId, AnchorBindingMode_DIRECT))
                {
                    mxConfigurationController->requestResourceDeactivation(mxViewTabBarId);
                }
                break;

            case ResourceActivationEvent:
                if (rEvent.ResourceId->compareTo(mxViewTabBarId) == 0)
                {
                    UpdateViewTabBar(Reference<XTabBar>(rEvent.ResourceObject,UNO_QUERY));
                }
        }
    }
}




void SAL_CALL ViewTabBarModule::disposing (
    const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (mxConfigurationController.is()
        && rEvent.Source == mxConfigurationController)
    {
        // Without the configuration controller this class can do nothing.
        mxConfigurationController = NULL;
        disposing();
    }
}




void ViewTabBarModule::UpdateViewTabBar (const Reference<XTabBar>& rxTabBar)
{
    if (mxConfigurationController.is())
    {
        Reference<XTabBar> xBar (rxTabBar);
        if ( ! xBar.is())
            xBar = Reference<XTabBar>(
                mxConfigurationController->getResource(mxViewTabBarId), UNO_QUERY);

        if (xBar.is())
        {
            TabBarButton aEmptyButton;

            Reference<XResourceId> xAnchor (mxViewTabBarId->getAnchor());

            TabBarButton aImpressViewButton;
            aImpressViewButton.ResourceId = FrameworkHelper::CreateResourceId(
                FrameworkHelper::msImpressViewURL,
                xAnchor);
            aImpressViewButton.ButtonLabel = String(SdResId(STR_DRAW_MODE));
            if ( ! xBar->hasTabBarButton(aImpressViewButton))
                xBar->addTabBarButtonAfter(aImpressViewButton, aEmptyButton);

            TabBarButton aOutlineViewButton;
            aOutlineViewButton.ResourceId = FrameworkHelper::CreateResourceId(
                FrameworkHelper::msOutlineViewURL,
                xAnchor);
            aOutlineViewButton.ButtonLabel = String(SdResId(STR_OUTLINE_MODE));
            if ( ! xBar->hasTabBarButton(aOutlineViewButton))
                xBar->addTabBarButtonAfter(aOutlineViewButton, aImpressViewButton);

            TabBarButton aNotesViewButton;
            aNotesViewButton.ResourceId = FrameworkHelper::CreateResourceId(
                FrameworkHelper::msNotesViewURL,
                xAnchor);
            aNotesViewButton.ButtonLabel = String(SdResId(STR_NOTES_MODE));
            if ( ! xBar->hasTabBarButton(aNotesViewButton))
                xBar->addTabBarButtonAfter(aNotesViewButton, aOutlineViewButton);

            TabBarButton aHandoutViewButton;
            aHandoutViewButton.ResourceId = FrameworkHelper::CreateResourceId(
                FrameworkHelper::msHandoutViewURL,
                xAnchor);
            aHandoutViewButton.ButtonLabel = String(SdResId(STR_HANDOUT_MODE));
            if ( ! xBar->hasTabBarButton(aHandoutViewButton))
                xBar->addTabBarButtonAfter(aHandoutViewButton, aNotesViewButton);
        }
    }
}




} } // end of namespace sd::framework
