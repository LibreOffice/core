/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <rtl/ustring.h>
#include<sfx2/viewfrm.hxx>

#include <svx/AccessibleShape.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unoapi.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include "Window.hxx"
#include "ViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "View.hxx"
#include "AccessibleOutlineView.hxx"
#include "AccessibleOutlineEditSource.hxx"

#include <memory>

#include "accessibility.hrc"
#include "sdresid.hxx"
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {




AccessibleOutlineView::AccessibleOutlineView (
    ::sd::Window* pSdWindow,
    ::sd::OutlineViewShell* pViewShell,
    const uno::Reference<frame::XController>& rxController,
    const uno::Reference<XAccessible>& rxParent)
    : AccessibleDocumentViewBase (pSdWindow, pViewShell, rxController, rxParent),
      maTextHelper( ::std::auto_ptr< SvxEditSource >( NULL ) )
{
    SolarMutexGuard aGuard;

    
    
    if( pViewShell && pSdWindow )
    {
        ::sd::View* pView = pViewShell->GetView();

        if (pView && pView->ISA(::sd::OutlineView))
        {
            OutlinerView* pOutlineView = static_cast< ::sd::OutlineView*>(
                pView)->GetViewByWindow( pSdWindow );
            SdrOutliner* pOutliner =
                static_cast< ::sd::OutlineView*>(pView)->GetOutliner();

            if( pOutlineView && pOutliner )
            {
                SAL_WNODEPRECATED_DECLARATIONS_PUSH
                maTextHelper.SetEditSource( ::std::auto_ptr< SvxEditSource >( new AccessibleOutlineEditSource(
                                                                                  *pOutliner, *pView, *pOutlineView, *pSdWindow ) ) );
                SAL_WNODEPRECATED_DECLARATIONS_POP
            }
        }
    }
}


AccessibleOutlineView::~AccessibleOutlineView (void)
{
    OSL_TRACE ("~AccessibleOutlineView");
}


void AccessibleOutlineView::Init (void)
{
    
    maTextHelper.SetEventSource(this);

    AccessibleDocumentViewBase::Init ();
}


void AccessibleOutlineView::ViewForwarderChanged (ChangeType aChangeType,
    const IAccessibleViewForwarder* pViewForwarder)
{
    AccessibleDocumentViewBase::ViewForwarderChanged (aChangeType, pViewForwarder);

    UpdateChildren();
}




sal_Int32 SAL_CALL
    AccessibleOutlineView::getAccessibleChildCount (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();

    
    return maTextHelper.GetChildCount();
}


uno::Reference<XAccessible> SAL_CALL
    AccessibleOutlineView::getAccessibleChild (sal_Int32 nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    
    return maTextHelper.GetChild(nIndex);
}

#include <drawdoc.hxx>

OUString SAL_CALL
    AccessibleOutlineView::getAccessibleName(void)
    throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard g;

    OUString sName = SdResId(SID_SD_A11Y_D_PRESENTATION);
    ::sd::View* pSdView = static_cast< ::sd::View* >( maShapeTreeInfo.GetSdrView() );
    if ( pSdView )
    {
        SdDrawDocument& rDoc = pSdView->GetDoc();
        rtl::OUString sFileName = rDoc.getDocAccTitle();
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



void SAL_CALL AccessibleOutlineView::addAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
{
    
    if ( ! IsDisposed())
        maTextHelper.AddEventListener(xListener);
    AccessibleContextBase::addEventListener(xListener);
}

void SAL_CALL AccessibleOutlineView::removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
{
    
    if ( ! IsDisposed())
        maTextHelper.RemoveEventListener(xListener);
    AccessibleContextBase::removeEventListener(xListener);
}



OUString SAL_CALL
    AccessibleOutlineView::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString("AccessibleOutlineView");
}




void SAL_CALL
    AccessibleOutlineView::disposing (const lang::EventObject& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    AccessibleDocumentViewBase::disposing (rEventObject);
}



void AccessibleOutlineView::FireEvent(const AccessibleEventObject& aEvent )
{
    
    maTextHelper.FireEvent(aEvent);
}

void AccessibleOutlineView::Activated (void)
{
    SolarMutexGuard aGuard;

    
    maTextHelper.SetFocus(sal_True);
}

void AccessibleOutlineView::Deactivated (void)
{
    SolarMutexGuard aGuard;

    
    maTextHelper.SetFocus(sal_False);
}

void SAL_CALL AccessibleOutlineView::disposing (void)
{
    
    maTextHelper.Dispose();

    AccessibleDocumentViewBase::disposing ();
}



void SAL_CALL
    AccessibleOutlineView::propertyChange (const beans::PropertyChangeEvent& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    AccessibleDocumentViewBase::propertyChange (rEventObject);

    OSL_TRACE ("AccessibleOutlineView::propertyChange");
    
    if (rEventObject.PropertyName == "CurrentPage" ||
        rEventObject.PropertyName == "PageChange")
    {
        OSL_TRACE ("    current page changed");

        
        UpdateChildren();
        CommitChange(AccessibleEventId::PAGE_CHANGED,rEventObject.NewValue, rEventObject.OldValue);
    }
    else if ( rEventObject.PropertyName == "VisibleArea" )
    {
        OSL_TRACE ("    visible area changed");

        
        UpdateChildren();
    }
    else
    {
        OSL_TRACE ("  unhandled");
    }
    OSL_TRACE ("  done");
}



OUString
    AccessibleOutlineView::CreateAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return SD_RESSTR(SID_SD_A11Y_I_OUTLINEVIEW_N);
}


/** Create a description for this view.  Use the model's description or URL
    if a description is not available.
*/
OUString
    AccessibleOutlineView::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return SD_RESSTR(SID_SD_A11Y_I_OUTLINEVIEW_D);
}

void AccessibleOutlineView::UpdateChildren()
{
    SolarMutexGuard aGuard;

    
    maTextHelper.UpdateChildren();
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
