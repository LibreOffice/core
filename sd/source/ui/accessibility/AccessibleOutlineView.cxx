/*************************************************************************
 *
 *  $RCSfile: AccessibleOutlineView.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:03:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWVIEW_HPP_
#include <com/sun/star/drawing/XDrawView.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBLE_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULSTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif
#ifndef _SFXFRAME_HXX
#include<sfx2/viewfrm.hxx>
#endif

#include <svx/AccessibleShape.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unoapi.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include "sdwindow.hxx"
#include <vcl/svapp.hxx>

#ifndef _SD_SDWINDOW_HXX
#include "sdwindow.hxx"
#endif
#ifndef _SD_VIEWSHEL_HXX
#include "viewshel.hxx"
#endif
#ifndef _SD_OUTLNVSH_HXX
#include "outlnvsh.hxx"
#endif
#ifndef _SD_SDVIEW_HXX
#include "sdview.hxx"
#endif
#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_OUTLINE_VIEW_HXX
#include "AccessibleOutlineView.hxx"
#endif
#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_OUTLINE_EDITSOURCE_HXX
#include "AccessibleOutlineEditSource.hxx"
#endif

#include <memory>

#include "accessibility.hrc"
#include "sdresid.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {


//=====  internal  ============================================================

AccessibleOutlineView::AccessibleOutlineView (
    SdWindow* pSdWindow,
    SdOutlineViewShell* pViewShell,
    const uno::Reference<frame::XController>& rxController,
    const uno::Reference<XAccessible>& rxParent)
    : AccessibleDocumentViewBase (pSdWindow, pViewShell, rxController, rxParent),
      maTextHelper( ::std::auto_ptr< SvxEditSource >( NULL ) )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // Beware! Here we leave the paths of the UNO API and descend into the
    // depths of the core.  Necessary for making the edit engine accessible.
    if( pViewShell && pSdWindow )
    {
        SdView* pView = pViewShell->GetView();

        if( pView && pView->ISA(SdOutlineView) )
        {
            OutlinerView* pOutlineView = static_cast< SdOutlineView* >(pView)->GetViewByWindow( pSdWindow );
            SdrOutliner* pOutliner = static_cast< SdOutlineView* >(pView)->GetOutliner();

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
    // #105479# Set event source _before_ starting to listen
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
    // forward
    return maTextHelper.GetChildCount();

    return 0;
}


uno::Reference<XAccessible> SAL_CALL
    AccessibleOutlineView::getAccessibleChild (long nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    // Forward request to children manager.
    return maTextHelper.GetChild(nIndex);
}

//=====  XAccessibleEventBroadcaster  ========================================

void SAL_CALL AccessibleOutlineView::addEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
{
    // delegate listener handling to children manager.
    maTextHelper.AddEventListener(xListener);
}

void SAL_CALL AccessibleOutlineView::removeEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
{
    // forward
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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // delegate listener handling to children manager.
    maTextHelper.SetFocus(sal_True);
}

void AccessibleOutlineView::Deactivated (void)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    return ::rtl::OUString( SdResId(SID_SD_A11Y_I_OUTLINEVIEW_N) );
}


/** Create a description for this view.  Use the model's description or URL
    if a description is not available.
*/
::rtl::OUString
    AccessibleOutlineView::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    return ::rtl::OUString( SdResId(SID_SD_A11Y_I_OUTLINEVIEW_D) );
}

void AccessibleOutlineView::UpdateChildren()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // Update visible children
    maTextHelper.UpdateChildren();
}

} // end of namespace accessibility
