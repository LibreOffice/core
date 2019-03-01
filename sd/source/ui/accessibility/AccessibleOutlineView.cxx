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

#include <com/sun/star/accessibility/AccessibleEventId.hpp>

#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <Window.hxx>
#include <OutlineViewShell.hxx>
#include <DrawDocShell.hxx>
#include <OutlineView.hxx>
#include <View.hxx>
#include <AccessibleOutlineView.hxx>
#include <AccessibleOutlineEditSource.hxx>
#include <drawdoc.hxx>
#include <strings.hrc>
#include <sdresid.hxx>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

//=====  internal  ============================================================

AccessibleOutlineView::AccessibleOutlineView (
    ::sd::Window* pSdWindow,
    ::sd::OutlineViewShell* pViewShell,
    const uno::Reference<frame::XController>& rxController,
    const uno::Reference<XAccessible>& rxParent)
    : AccessibleDocumentViewBase (pSdWindow, pViewShell, rxController, rxParent),
      maTextHelper( ::std::unique_ptr< SvxEditSource >() )
{
    SolarMutexGuard aGuard;

    // Beware! Here we leave the paths of the UNO API and descend into the
    // depths of the core.  Necessary for making the edit engine accessible.
    if (!pSdWindow)
        return;

    ::sd::View* pView = pViewShell->GetView();

    if (dynamic_cast<const ::sd::OutlineView* >( pView ) ==  nullptr)
        return;

    OutlinerView* pOutlineView = static_cast< ::sd::OutlineView*>(
        pView)->GetViewByWindow( pSdWindow );
    SdrOutliner& rOutliner =
        static_cast< ::sd::OutlineView*>(pView)->GetOutliner();

    if( pOutlineView )
    {
        maTextHelper.SetEditSource( ::std::unique_ptr< SvxEditSource >( new AccessibleOutlineEditSource(
                                                                          rOutliner, *pView, *pOutlineView, *pSdWindow ) ) );
    }
}

AccessibleOutlineView::~AccessibleOutlineView()
{
}

void AccessibleOutlineView::Init()
{
    // Set event source _before_ starting to listen
    maTextHelper.SetEventSource(this);

    AccessibleDocumentViewBase::Init ();
}

void AccessibleOutlineView::ViewForwarderChanged()
{
    AccessibleDocumentViewBase::ViewForwarderChanged();

    UpdateChildren();
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL
    AccessibleOutlineView::getAccessibleChildCount()
{
    ThrowIfDisposed ();

    // forward
    return maTextHelper.GetChildCount();
}

uno::Reference<XAccessible> SAL_CALL
    AccessibleOutlineView::getAccessibleChild (sal_Int32 nIndex)
{
    ThrowIfDisposed ();
    // Forward request to children manager.
    return maTextHelper.GetChild(nIndex);
}

OUString SAL_CALL
    AccessibleOutlineView::getAccessibleName()
{
    SolarMutexGuard g;

    OUString sName = SdResId(SID_SD_A11Y_D_PRESENTATION);
    ::sd::View* pSdView = static_cast< ::sd::View* >( maShapeTreeInfo.GetSdrView() );
    if ( pSdView )
    {
        SdDrawDocument& rDoc = pSdView->GetDoc();
        OUString sFileName = rDoc.getDocAccTitle();
        if (sFileName.isEmpty())
        {
            ::sd::DrawDocShell* pDocSh = pSdView->GetDocSh();
            if ( pDocSh )
            {
                sFileName = pDocSh->GetTitle( SFX_TITLE_APINAME );
            }
        }
        if (!sFileName.isEmpty())
        {
            sName = sFileName + " - " + sName;
        }
    }
    return sName;
}

//=====  XAccessibleEventBroadcaster  ========================================

void SAL_CALL AccessibleOutlineView::addAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
{
    // delegate listener handling to children manager.
    if ( ! IsDisposed())
        maTextHelper.AddEventListener(xListener);
    AccessibleContextBase::addEventListener(xListener);
}

void SAL_CALL AccessibleOutlineView::removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
{
    // forward
    if ( ! IsDisposed())
        maTextHelper.RemoveEventListener(xListener);
    AccessibleContextBase::removeEventListener(xListener);
}

// XServiceInfo

OUString SAL_CALL
    AccessibleOutlineView::getImplementationName()
{
    return OUString("AccessibleOutlineView");
}

//=====  XEventListener  ======================================================

//=====  protected internal  ==================================================

void AccessibleOutlineView::Activated()
{
    SolarMutexGuard aGuard;

    // delegate listener handling to children manager.
    maTextHelper.SetFocus();
}

void AccessibleOutlineView::Deactivated()
{
    SolarMutexGuard aGuard;

    // delegate listener handling to children manager.
    maTextHelper.SetFocus(false);
}

void SAL_CALL AccessibleOutlineView::disposing()
{
    // dispose children
    maTextHelper.Dispose();

    AccessibleDocumentViewBase::disposing ();
}

//=====  XPropertyChangeListener  =============================================

void SAL_CALL
    AccessibleOutlineView::propertyChange (const beans::PropertyChangeEvent& rEventObject)
{
    ThrowIfDisposed ();

    AccessibleDocumentViewBase::propertyChange (rEventObject);

    //add page switch event for slide show mode
    if (rEventObject.PropertyName == "CurrentPage" ||
        rEventObject.PropertyName == "PageChange")
    {
        // The current page changed. Update the children accordingly.
        UpdateChildren();
        CommitChange(AccessibleEventId::PAGE_CHANGED,rEventObject.NewValue, rEventObject.OldValue);
    }
    else if ( rEventObject.PropertyName == "VisibleArea" )
    {
        // The visible area changed. Update the children accordingly.
        UpdateChildren();
    }
    else
    {
        SAL_INFO("sd", "unhandled");
    }
}

/// Create a name for this view.
OUString
    AccessibleOutlineView::CreateAccessibleName()
{
    SolarMutexGuard aGuard;

    return SdResId(SID_SD_A11Y_I_OUTLINEVIEW_N);
}

void AccessibleOutlineView::UpdateChildren()
{
    SolarMutexGuard aGuard;

    // Update visible children
    maTextHelper.UpdateChildren();
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
