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

#include "PresenterPaneContainer.hxx"
#include "PresenterPaneBase.hxx"
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

PresenterPaneContainer::PresenterPaneContainer (
    const Reference<XComponentContext>& rxContext)
    : PresenterPaneContainerInterfaceBase(m_aMutex),
      maPanes(),
      mxPresenterHelper()
{
    Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager());
    if (xFactory.is())
    {
        mxPresenterHelper = Reference<drawing::XPresenterHelper>(
            xFactory->createInstanceWithContext(
                OUString("com.sun.star.comp.Draw.PresenterHelper"),
                rxContext),
            UNO_QUERY_THROW);
    }
}

PresenterPaneContainer::~PresenterPaneContainer (void)
{
}

void PresenterPaneContainer::PreparePane (
    const Reference<XResourceId>& rxPaneId,
    const OUString& rsViewURL,
    const OUString& rsTitle,
    const OUString& rsAccessibleTitle,
    const bool bIsOpaque,
    const ViewInitializationFunction& rViewInitialization,
    const double nLeft,
    const double nTop,
    const double nRight,
    const double nBottom)
{
    if ( ! rxPaneId.is())
        return;

    SharedPaneDescriptor pPane (FindPaneURL(rxPaneId->getResourceURL()));
    if (pPane.get() == NULL)
    {
        // No entry found for the given pane id.  Create a new one.
        SharedPaneDescriptor pDescriptor (new PaneDescriptor());
        pDescriptor->mxPaneId = rxPaneId;
        pDescriptor->msViewURL = rsViewURL;
        pDescriptor->mxPane = NULL;
        if (rsTitle.indexOf('%') < 0)
        {
            pDescriptor->msTitle = rsTitle;
            pDescriptor->msTitleTemplate = OUString();
        }
        else
        {
            pDescriptor->msTitleTemplate = rsTitle;
            pDescriptor->msTitle = OUString();
        }
        pDescriptor->msAccessibleTitleTemplate = rsAccessibleTitle;
        pDescriptor->maViewInitialization = rViewInitialization;
        pDescriptor->mnLeft = nLeft;
        pDescriptor->mnTop = nTop;
        pDescriptor->mnRight = nRight;
        pDescriptor->mnBottom = nBottom;
        pDescriptor->mbIsActive = true;
        pDescriptor->mbIsOpaque = bIsOpaque;
        pDescriptor->maSpriteProvider = PaneDescriptor::SpriteProvider();
        pDescriptor->mbIsSprite = false;
        pDescriptor->maCalloutAnchorLocation = awt::Point(-1,-1);
        pDescriptor->mbHasCalloutAnchor = false;

        maPanes.push_back(pDescriptor);
    }
}

void SAL_CALL PresenterPaneContainer::disposing (void)
{
    PaneList::iterator iPane (maPanes.begin());
    PaneList::const_iterator iEnd (maPanes.end());
    for ( ; iPane!=iEnd; ++iPane)
        if ((*iPane)->mxPaneId.is())
            RemovePane((*iPane)->mxPaneId);
}

PresenterPaneContainer::SharedPaneDescriptor
    PresenterPaneContainer::StorePane (const rtl::Reference<PresenterPaneBase>& rxPane)
{
    SharedPaneDescriptor pDescriptor;

    if (rxPane.is())
    {
        OUString sPaneURL;
        Reference<XResourceId> xPaneId (rxPane->getResourceId());
        if (xPaneId.is())
            sPaneURL = xPaneId->getResourceURL();

        pDescriptor = FindPaneURL(sPaneURL);
        if (pDescriptor.get() == NULL)
            PreparePane(xPaneId, OUString(), OUString(), OUString(),
                false, ViewInitializationFunction(), 0,0,0,0);
        pDescriptor = FindPaneURL(sPaneURL);
        if (pDescriptor.get() != NULL)
        {
            Reference<awt::XWindow> xWindow (rxPane->getWindow());
            pDescriptor->mxContentWindow = xWindow;
            pDescriptor->mxPaneId = xPaneId;
            pDescriptor->mxPane = rxPane;
            pDescriptor->mxPane->SetTitle(pDescriptor->msTitle);

            // When there is a call out anchor location set then tell the
            // window about it.
            if (pDescriptor->mbHasCalloutAnchor)
                pDescriptor->mxPane->SetCalloutAnchor(pDescriptor->maCalloutAnchorLocation);

            if (xWindow.is())
                xWindow->addEventListener(this);
        }
    }

    return pDescriptor;
}

PresenterPaneContainer::SharedPaneDescriptor
    PresenterPaneContainer::StoreBorderWindow(
        const Reference<XResourceId>& rxPaneId,
        const Reference<awt::XWindow>& rxBorderWindow)
{
    // The content window may not be present.  Use the resource URL of the
    // pane id as key.
    OUString sPaneURL;
    if (rxPaneId.is())
        sPaneURL = rxPaneId->getResourceURL();

    SharedPaneDescriptor pDescriptor (FindPaneURL(sPaneURL));
    if (pDescriptor.get() != NULL)
    {
        pDescriptor->mxBorderWindow = rxBorderWindow;
        return pDescriptor;
    }
    else
        return SharedPaneDescriptor();
}

PresenterPaneContainer::SharedPaneDescriptor
    PresenterPaneContainer::StoreView (
        const Reference<XView>& rxView,
        const SharedBitmapDescriptor& rpViewBackground)
{
    SharedPaneDescriptor pDescriptor;

    if (rxView.is())
    {
        OUString sPaneURL;
        Reference<XResourceId> xViewId (rxView->getResourceId());
        if (xViewId.is())
        {
            Reference<XResourceId> xPaneId (xViewId->getAnchor());
            if (xPaneId.is())
                sPaneURL = xPaneId->getResourceURL();
        }

        pDescriptor = FindPaneURL(sPaneURL);
        if (pDescriptor.get() != NULL)
        {
            pDescriptor->mxView = rxView;
            pDescriptor->mpViewBackground = rpViewBackground;
            pDescriptor->mxPane->SetBackground(rpViewBackground);
            try
            {
                if ( ! pDescriptor->maViewInitialization.empty())
                    pDescriptor->maViewInitialization(rxView);

                // Activate or deactivate the pane/view.
                if ( ! pDescriptor->maActivator.empty())
                    pDescriptor->maActivator(pDescriptor->mbIsActive);
            }
            catch (RuntimeException&)
            {
                OSL_ASSERT(false);
            }
        }
    }

    return pDescriptor;
}

PresenterPaneContainer::SharedPaneDescriptor
    PresenterPaneContainer::RemovePane (const Reference<XResourceId>& rxPaneId)
{
    SharedPaneDescriptor pDescriptor (FindPaneId(rxPaneId));
    if (pDescriptor.get() != NULL)
    {
        if (pDescriptor->mxContentWindow.is())
            pDescriptor->mxContentWindow->removeEventListener(this);
        pDescriptor->mxContentWindow = NULL;
        pDescriptor->mxBorderWindow = NULL;
        pDescriptor->mxPane = NULL;
        pDescriptor->mxView = NULL;
        pDescriptor->mbIsActive = false;
    }
    return pDescriptor;
}

PresenterPaneContainer::SharedPaneDescriptor
    PresenterPaneContainer::RemoveView (const Reference<XView>& rxView)
{
    SharedPaneDescriptor pDescriptor;

    if (rxView.is())
    {
        OUString sPaneURL;
        Reference<XResourceId> xViewId (rxView->getResourceId());
        if (xViewId.is())
        {
            Reference<XResourceId> xPaneId (xViewId->getAnchor());
            if (xPaneId.is())
                sPaneURL = xPaneId->getResourceURL();
        }

        pDescriptor = FindPaneURL(sPaneURL);
        if (pDescriptor.get() != NULL)
        {
            pDescriptor->mxView = NULL;
            pDescriptor->mpViewBackground = SharedBitmapDescriptor();
        }
    }

    return pDescriptor;
}

PresenterPaneContainer::SharedPaneDescriptor PresenterPaneContainer::FindBorderWindow (
    const Reference<awt::XWindow>& rxBorderWindow)
{
    PaneList::const_iterator iPane;
    PaneList::iterator iEnd (maPanes.end());
    for (iPane=maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        if ((*iPane)->mxBorderWindow == rxBorderWindow)
            return *iPane;
    }
    return SharedPaneDescriptor();
}

PresenterPaneContainer::SharedPaneDescriptor PresenterPaneContainer::FindContentWindow (
    const Reference<awt::XWindow>& rxContentWindow)
{
    PaneList::const_iterator iPane;
    PaneList::iterator iEnd (maPanes.end());
    for (iPane=maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        if ((*iPane)->mxContentWindow == rxContentWindow)
            return *iPane;
    }
    return SharedPaneDescriptor();
}

PresenterPaneContainer::SharedPaneDescriptor PresenterPaneContainer::FindPaneURL (
    const OUString& rsPaneURL)
{
    PaneList::const_iterator iPane;
    PaneList::const_iterator iEnd (maPanes.end());
    for (iPane=maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        if ((*iPane)->mxPaneId->getResourceURL() == rsPaneURL)
            return *iPane;
    }
    return SharedPaneDescriptor();
}

PresenterPaneContainer::SharedPaneDescriptor PresenterPaneContainer::FindPaneId (
    const Reference<XResourceId>& rxPaneId)
{
    PaneList::iterator iEnd (maPanes.end());

    if ( ! rxPaneId.is())
        return SharedPaneDescriptor();

    PaneList::iterator iPane;
    for (iPane=maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        if (rxPaneId->compareTo((*iPane)->mxPaneId) == 0)
            return *iPane;
    }
    return SharedPaneDescriptor();
}

PresenterPaneContainer::SharedPaneDescriptor PresenterPaneContainer::FindViewURL (
    const OUString& rsViewURL)
{
    PaneList::iterator iEnd (maPanes.end());
    PaneList::iterator iPane;
    for (iPane=maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        if (rsViewURL == (*iPane)->msViewURL)
            return *iPane;
    }
    return SharedPaneDescriptor();
}

::rtl::OUString PresenterPaneContainer::GetPaneURLForViewURL (const ::rtl::OUString& rsViewURL)
{
    SharedPaneDescriptor pDescriptor (FindViewURL(rsViewURL));
    if (pDescriptor.get() != NULL)
        if (pDescriptor->mxPaneId.is())
            return pDescriptor->mxPaneId->getResourceURL();
    return OUString();
}

void PresenterPaneContainer::ToTop (const SharedPaneDescriptor& rpDescriptor)
{
    if (rpDescriptor.get() != NULL)
    {
        // Find iterator for pDescriptor.
        PaneList::iterator iPane;
        PaneList::iterator iEnd (maPanes.end());
        for (iPane=maPanes.begin(); iPane!=iEnd; ++iPane)
            if (iPane->get() == rpDescriptor.get())
                break;
        OSL_ASSERT(iPane!=iEnd);
        if (iPane == iEnd)
            return;

        if (mxPresenterHelper.is())
            mxPresenterHelper->toTop(rpDescriptor->mxBorderWindow);

        maPanes.erase(iPane);
        maPanes.push_back(rpDescriptor);
    }
}

//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterPaneContainer::disposing (
    const com::sun::star::lang::EventObject& rEvent)
    throw (com::sun::star::uno::RuntimeException)
{
    SharedPaneDescriptor pDescriptor (
        FindContentWindow(Reference<awt::XWindow>(rEvent.Source, UNO_QUERY)));
    if (pDescriptor.get() != NULL)
    {
        RemovePane(pDescriptor->mxPaneId);
    }
}

//===== PresenterPaneContainer::PaneDescriptor ================================

void PresenterPaneContainer::PaneDescriptor::SetActivationState (const bool bIsActive)
{
    mbIsActive = bIsActive;
    if ( ! maActivator.empty())
        maActivator(mbIsActive);
}

} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
