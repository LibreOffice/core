/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideSorterModule.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:40:16 $
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

#include "SlideSorterModule.hxx"

#include "framework/FrameworkHelper.hxx"

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XTABBAR_HPP_
#include <com/sun/star/drawing/framework/XTabBar.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_TABBARBUTTON_HPP_
#include <com/sun/star/drawing/framework/TabBarButton.hpp>
#endif

#include "strings.hrc"
#include "sdresid.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;
using ::sd::framework::FrameworkHelper;


namespace sd { namespace framework {


//===== SlideSorterModule ==================================================

SlideSorterModule::SlideSorterModule (
    const Reference<frame::XController>& rxController,
    const OUString& rsLeftPaneURL)
    : ResourceManager(rxController,
        FrameworkHelper::CreateResourceId(FrameworkHelper::msSlideSorterURL, rsLeftPaneURL)),
      mxViewTabBarId(FrameworkHelper::CreateResourceId(
          FrameworkHelper::msViewTabBarURL,
          FrameworkHelper::msCenterPaneURL)),
      mxControllerManager(rxController,UNO_QUERY)
{
    if (mxConfigurationController.is())
    {
        UpdateViewTabBar(NULL);

        AddActiveMainView(FrameworkHelper::msImpressViewURL);
        AddActiveMainView(FrameworkHelper::msOutlineViewURL);
        AddActiveMainView(FrameworkHelper::msNotesViewURL);

        AddActiveMainView(FrameworkHelper::msDrawViewURL);

        mxConfigurationController->addConfigurationChangeListener(
            this,
            FrameworkHelper::msResourceActivationEvent,
            Any());
    }
}




SlideSorterModule::~SlideSorterModule (void)
{
}




void SAL_CALL SlideSorterModule::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Type.equals(FrameworkHelper::msResourceActivationEvent))
    {
        if (rEvent.ResourceId->compareTo(mxViewTabBarId) == 0)
        {
            // Update the view tab bar because the view tab bar has just
            // become active.
            UpdateViewTabBar(Reference<XTabBar>(rEvent.ResourceObject,UNO_QUERY));
        }
        else if (rEvent.ResourceId->getResourceTypePrefix().equals(
            FrameworkHelper::msViewURLPrefix)
            && rEvent.ResourceId->isBoundTo(
                FrameworkHelper::CreateResourceId(FrameworkHelper::msCenterPaneURL),
                AnchorBindingMode_DIRECT))
        {
            // Update the view tab bar because the view in the center pane
            // has changed.
            UpdateViewTabBar(NULL);
        }
    }
    else
    {
        ResourceManager::notifyConfigurationChange(rEvent);
    }
}




void SlideSorterModule::UpdateViewTabBar (const Reference<XTabBar>& rxTabBar)
{
    if ( ! mxControllerManager.is())
        return;

    Reference<XTabBar> xBar (rxTabBar);
    if ( ! xBar.is())
    {
        Reference<XConfigurationController> xCC (
            mxControllerManager->getConfigurationController());
        if (xCC.is())
            xBar = Reference<XTabBar>(xCC->getResource(mxViewTabBarId), UNO_QUERY);
    }

    if (xBar.is())
    {
        TabBarButton aButtonA;
        aButtonA.ResourceId = FrameworkHelper::CreateResourceId(
            FrameworkHelper::msSlideSorterURL,
            FrameworkHelper::msCenterPaneURL);
        aButtonA.ButtonLabel = String(SdResId(STR_SLIDE_MODE));

        TabBarButton aButtonB;
        aButtonB.ResourceId = FrameworkHelper::CreateResourceId(
            FrameworkHelper::msHandoutViewURL,
            FrameworkHelper::msCenterPaneURL);

        if ( ! xBar->hasTabBarButton(aButtonA))
            xBar->addTabBarButtonAfter(aButtonA, aButtonB);
    }
}



} } // end of namespace sd::framework
