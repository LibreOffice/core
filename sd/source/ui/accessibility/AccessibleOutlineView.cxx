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
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#ifndef _COM_SUN_STAR_ACCESSIBLE_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#ifndef _COM_SUN_STAR_LANG_XMULSTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#include <rtl/ustring.h>
#ifndef _SFXFRAME_HXX
#include<sfx2/viewfrm.hxx>
#endif

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
#include <vos/mutex.hxx>

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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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

#include <drawdoc.hxx>
::rtl::OUString SAL_CALL
    AccessibleOutlineView::getAccessibleName(void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName = String( SdResId(SID_SD_A11Y_D_PRESENTATION) );
    ::sd::View* pSdView = static_cast< ::sd::View* >( maShapeTreeInfo.GetSdrView() );
    if ( pSdView )
    {
        SdDrawDocument* pDoc = pSdView->GetDoc();
        if ( pDoc )
        {
            rtl::OUString sFileName = pDoc->getDocAccTitle();
            if ( !sFileName.getLength() )
            {
                ::sd::DrawDocShell* pDocSh = pSdView->GetDocSh();
                if ( pDocSh )
                {
                    sFileName = pDocSh->GetTitle( SFX_TITLE_APINAME );
                }
            }
            if ( sFileName.getLength() )
            {
                sName = sFileName + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - ")) + sName;
            }
        }
    }
    return sName;
}
//=====  XAccessibleEventBroadcaster  ========================================

void SAL_CALL AccessibleOutlineView::addEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
{
    // delegate listener handling to children manager.
    if ( ! IsDisposed())
        maTextHelper.AddEventListener(xListener);
    AccessibleContextBase::addEventListener(xListener);
}

void SAL_CALL AccessibleOutlineView::removeEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
{
    // forward
    if ( ! IsDisposed())
        maTextHelper.RemoveEventListener(xListener);
    AccessibleContextBase::removeEventListener(xListener);
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
    ThrowIfDisposed ();

    AccessibleDocumentViewBase::propertyChange (rEventObject);

    OSL_TRACE ("AccessibleOutlineView::propertyChange");
    //add page switch event for slide show mode
    if (rEventObject.PropertyName == ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("CurrentPage")) ||
        rEventObject.PropertyName == ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("PageChange")) )
    {
        OSL_TRACE ("    current page changed");

        // The current page changed. Update the children accordingly.
        UpdateChildren();
        CommitChange(AccessibleEventId::PAGE_CHANGED,rEventObject.NewValue,rEventObject.OldValue);
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

    return String( SdResId(SID_SD_A11Y_I_OUTLINEVIEW_N) );
}


/** Create a description for this view.  Use the model's description or URL
    if a description is not available.
*/
::rtl::OUString
    AccessibleOutlineView::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    return String( SdResId(SID_SD_A11Y_I_OUTLINEVIEW_D) );
}

void AccessibleOutlineView::UpdateChildren()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // Update visible children
    maTextHelper.UpdateChildren();
}

} // end of namespace accessibility
