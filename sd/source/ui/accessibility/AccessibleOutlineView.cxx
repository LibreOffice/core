/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
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


//=====  internal  ============================================================

AccessibleOutlineView::AccessibleOutlineView (
    ::sd::Window* pSdWindow,
    ::sd::OutlineViewShell* pViewShell,
    const uno::Reference<frame::XController>& rxController,
    const uno::Reference<XAccessible>& rxParent)
    : AccessibleDocumentViewBase (pSdWindow, pViewShell, rxController, rxParent),
      maTextHelper( ::std::auto_ptr< SvxEditSource >( NULL ) )
{
    SolarMutexGuard aGuard;

    // Beware! Here we leave the paths of the UNO API and descend into the
    // depths of the core.  Necessary for making the edit engine accessible.
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
                maTextHelper.SetEditSource( ::std::auto_ptr< SvxEditSource >( new AccessibleOutlineEditSource(
                                                                                  *pOutliner, *pView, *pOutlineView, *pSdWindow ) ) );
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
    // Set event source _before_ starting to listen
    maTextHelper.SetEventSource(this);

    AccessibleDocumentViewBase::Init ();
}


void AccessibleOutlineView::ViewForwarderChanged (ChangeType aChangeType,
    const IAccessibleViewForwarder* pViewForwarder)
{
    AccessibleDocumentViewBase::ViewForwarderChanged (aChangeType, pViewForwarder);

    UpdateChildren();
}


//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL
    AccessibleOutlineView::getAccessibleChildCount (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();

    // forward
    return maTextHelper.GetChildCount();
}


uno::Reference<XAccessible> SAL_CALL
    AccessibleOutlineView::getAccessibleChild (sal_Int32 nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    // Forward request to children manager.
    return maTextHelper.GetChild(nIndex);
}

//=====  XAccessibleEventBroadcaster  ========================================

void SAL_CALL AccessibleOutlineView::addEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
{
    // delegate listener handling to children manager.
    if ( ! IsDisposed())
        maTextHelper.AddEventListener(xListener);
}

void SAL_CALL AccessibleOutlineView::removeEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
{
    // forward
    if ( ! IsDisposed())
        maTextHelper.RemoveEventListener(xListener);
}

//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessibleOutlineView::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleOutlineView"));
}


//=====  XEventListener  ======================================================

void SAL_CALL
    AccessibleOutlineView::disposing (const lang::EventObject& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    AccessibleDocumentViewBase::disposing (rEventObject);
}

//=====  protected internal  ==================================================

void AccessibleOutlineView::FireEvent(const AccessibleEventObject& aEvent )
{
    // delegate listener handling to children manager.
    maTextHelper.FireEvent(aEvent);
}

void AccessibleOutlineView::Activated (void)
{
    SolarMutexGuard aGuard;

    // delegate listener handling to children manager.
    maTextHelper.SetFocus(sal_True);
}

void AccessibleOutlineView::Deactivated (void)
{
    SolarMutexGuard aGuard;

    // delegate listener handling to children manager.
    maTextHelper.SetFocus(sal_False);
}

void SAL_CALL AccessibleOutlineView::disposing (void)
{
    // dispose children
    maTextHelper.Dispose();

    AccessibleDocumentViewBase::disposing ();
}

//=====  XPropertyChangeListener  =============================================

void SAL_CALL
    AccessibleOutlineView::propertyChange (const beans::PropertyChangeEvent& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    AccessibleDocumentViewBase::propertyChange (rEventObject);

    OSL_TRACE ("AccessibleOutlineView::propertyChange");
    if (rEventObject.PropertyName == ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("CurrentPage")))
    {
        OSL_TRACE ("    current page changed");

        // The current page changed. Update the children accordingly.
        UpdateChildren();
    }
    else if (rEventObject.PropertyName == ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("VisibleArea")))
    {
        OSL_TRACE ("    visible area changed");

        // The visible area changed. Update the children accordingly.
        UpdateChildren();
    }
    else
    {
        OSL_TRACE ("  unhandled");
    }
    OSL_TRACE ("  done");
}


/// Create a name for this view.
::rtl::OUString
    AccessibleOutlineView::CreateAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return String( SdResId(SID_SD_A11Y_I_OUTLINEVIEW_N) );
}


/** Create a description for this view.  Use the model's description or URL
    if a description is not available.
*/
::rtl::OUString
    AccessibleOutlineView::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return String( SdResId(SID_SD_A11Y_I_OUTLINEVIEW_D) );
}

void AccessibleOutlineView::UpdateChildren()
{
    SolarMutexGuard aGuard;

    // Update visible children
    maTextHelper.UpdateChildren();
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
