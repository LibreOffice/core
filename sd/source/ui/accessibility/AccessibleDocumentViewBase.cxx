/*************************************************************************
 *
 *  $RCSfile: AccessibleDocumentViewBase.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:03:01 $
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

#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_DOCUMENT_VIEW_BASE_HXX
#include "AccessibleDocumentViewBase.hxx"
#endif

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
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
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


#ifndef _SD_VIEWSHEL_HXX
#include "viewshel.hxx"
#endif
#ifndef _SD_SDVIEW_HXX
#include "sdview.hxx"
#endif
#include <memory>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

class SfxViewFrame;

namespace accessibility {

static SfxViewFrame* mpViewFrame = NULL;

//=====  internal  ============================================================
AccessibleDocumentViewBase::AccessibleDocumentViewBase (
    SdWindow* pSdWindow,
    SdViewShell* pViewShell,
    const uno::Reference<frame::XController>& rxController,
    const uno::Reference<XAccessible>& rxParent)
    : AccessibleContextBase (rxParent, AccessibleRole::DOCUMENT),
      mpWindow (pSdWindow),
      mxController (rxController),
      mxModel (NULL),
      maViewForwarder (
        static_cast<SdrPaintView*>(pViewShell->GetView()),
        *static_cast<OutputDevice*>(pSdWindow))
{
    if (mxController.is())
        mxModel = mxController->getModel();

    // Fill the shape tree info.
    maShapeTreeInfo.SetModelBroadcaster (
        uno::Reference<document::XEventBroadcaster>(
            mxModel, uno::UNO_QUERY));
    maShapeTreeInfo.SetController (mxController);
    maShapeTreeInfo.SetSdrView (pViewShell->GetView());
    maShapeTreeInfo.SetWindow (pSdWindow);
    maShapeTreeInfo.SetViewForwarder (&maViewForwarder);

    mxWindow = ::VCLUnoHelper::GetInterface (pSdWindow);

    mpViewFrame = pViewShell->GetViewFrame();
}




AccessibleDocumentViewBase::~AccessibleDocumentViewBase (void)
{
    // At this place we should be disposed.  You may want to add a
    // corresponding assertion into the destructor of a derived class.
}




void AccessibleDocumentViewBase::Init (void)
{
    // Finish the initialization of the shape tree info container.
    maShapeTreeInfo.SetDocumentWindow (this);

    // Register as window listener to stay up to date with its size and
    // position.
    mxWindow->addWindowListener (this);

    // Determine the list of shapes on the current page.
    uno::Reference<drawing::XShapes> xShapeList;
    uno::Reference<drawing::XDrawView> xView (mxController, uno::UNO_QUERY);
    if (xView.is())
        xShapeList = uno::Reference<drawing::XShapes> (
            xView->getCurrentPage(), uno::UNO_QUERY);

    // Register this object as dispose event listener at the model.
    if (mxModel.is())
        mxModel->addEventListener (
            static_cast<awt::XWindowListener*>(this));

    // Register as property change listener at the controller.
    uno::Reference<beans::XPropertySet> xSet (mxController, uno::UNO_QUERY);
    if (xSet.is())
        xSet->addPropertyChangeListener (
            OUString (RTL_CONSTASCII_USTRINGPARAM("")),
            static_cast<beans::XPropertyChangeListener*>(this));

    // Register as top window listener at the top window.
    uno::Reference<awt::XTopWindow> xTopWindow;
    if (mxController.is())
        if (mxController->getFrame().is())
            xTopWindow = uno::Reference<awt::XTopWindow> (
                mxController->getFrame()->getContainerWindow(),
                uno::UNO_QUERY);
    if (xTopWindow.is())
        xTopWindow->addTopWindowListener (
            static_cast<awt::XTopWindowListener*>(this));

    // Register at VCL Window to be informed of activated and deactivated
    // OLE objects.
    Window* pWindow = maShapeTreeInfo.GetWindow();
    if (pWindow != NULL)
    {
        pWindow->AddChildEventListener (LINK(
            this, AccessibleDocumentViewBase, WindowChildEventListener));

        USHORT nCount = pWindow->GetChildCount();
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            Window* pChildWindow = pWindow->GetChild (i);
            if (pChildWindow &&
                (AccessibleRole::EMBEDDED_OBJECT
                    ==pChildWindow->GetAccessibleRole()))
            {
                SetAccessibleOLEObject (pChildWindow->GetAccessible());
            }
        }
    }
}




IMPL_LINK(AccessibleDocumentViewBase, WindowChildEventListener,
    VclSimpleEvent*, pEvent)
{
    OSL_ASSERT(pEvent!=NULL && pEvent->ISA(VclWindowEvent));
    if (pEvent!=NULL && pEvent->ISA(VclWindowEvent))
    {
        VclWindowEvent* pWindowEvent = static_cast<VclWindowEvent*>(pEvent);
        //      DBG_ASSERT( pVclEvent->GetWindow(), "Window???" );
        switch (pWindowEvent->GetId())
        {
            case VCLEVENT_OBJECT_DYING:
            {
                // Window is dying.  Unregister from VCL Window.
                // This is also attempted in the disposing() method.
                Window* pWindow = maShapeTreeInfo.GetWindow();
                Window* pDyingWindow = static_cast<Window*>(
                    pWindowEvent->GetWindow());
                if (pWindow==pDyingWindow && pWindow!=NULL)
                {
                    pWindow->RemoveChildEventListener (LINK(
                        this,
                        AccessibleDocumentViewBase,
                        WindowChildEventListener));
                }
            }
            break;

            case VCLEVENT_WINDOW_SHOW:
            {
                // A new window has been created.  Is it an OLE object?
                Window* pChildWindow = static_cast<Window*>(
                    pWindowEvent->GetWindow());
                if (pChildWindow!=NULL
                    && (pChildWindow->GetAccessibleRole()
                        == AccessibleRole::EMBEDDED_OBJECT))
                {
                    SetAccessibleOLEObject (pChildWindow->GetAccessible());
                }
            }
            break;

            case VCLEVENT_WINDOW_HIDE:
            {
                // A window has been destroyed.  Has that been an OLE
                // object?
                Window* pChildWindow = static_cast<Window*>(
                    pWindowEvent->GetWindow());
                if (pChildWindow!=NULL
                    && (pChildWindow->GetAccessibleRole()
                        == AccessibleRole::EMBEDDED_OBJECT))
                {
                    SetAccessibleOLEObject (NULL);
                }
            }
            break;
        }
    }

    return 0;
}




//=====  IAccessibleViewForwarderListener  ====================================

void AccessibleDocumentViewBase::ViewForwarderChanged (ChangeType aChangeType,
    const IAccessibleViewForwarder* pViewForwarder)
{
    // Empty
}




//=====  XAccessibleContext  ==================================================

Reference<XAccessible> SAL_CALL
       AccessibleDocumentViewBase::getAccessibleParent (void)
    throw (uno::RuntimeException)
{
    return AccessibleContextBase::getAccessibleParent();
}



sal_Int32 SAL_CALL
    AccessibleDocumentViewBase::getAccessibleChildCount (void)
    throw (uno::RuntimeException)
{
    if (mxAccessibleOLEObject.is())
        return 1;
    else
        return 0;
}




Reference<XAccessible> SAL_CALL
    AccessibleDocumentViewBase::getAccessibleChild (long nIndex)
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    if (mxAccessibleOLEObject.is())
        if (nIndex == 0)
            return mxAccessibleOLEObject;

    throw lang::IndexOutOfBoundsException (
        ::rtl::OUString::createFromAscii ("no child with index ") + nIndex,
        NULL);
}




//=====  XAccessibleComponent  ================================================

/** Iterate over all children and test whether the specified point lies
    within one of their bounding boxes.  Return the first child for which
    this is true.
*/
uno::Reference<XAccessible > SAL_CALL
    AccessibleDocumentViewBase::getAccessibleAtPoint (
        const awt::Point& aPoint)
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    uno::Reference<XAccessible> xChildAtPosition;

    sal_Int32 nChildCount = getAccessibleChildCount ();
    for (sal_Int32 i=nChildCount-1; i>=0; --i)
    {
        Reference<XAccessible> xChild (getAccessibleChild (i));
        if (xChild.is())
        {
            Reference<XAccessibleComponent> xChildComponent (
                xChild->getAccessibleContext(), uno::UNO_QUERY);
            if (xChildComponent.is())
            {
                awt::Rectangle aBBox (xChildComponent->getBounds());
                if ( (aPoint.X >= aBBox.X)
                    && (aPoint.Y >= aBBox.Y)
                    && (aPoint.X < aBBox.X+aBBox.Width)
                    && (aPoint.Y < aBBox.Y+aBBox.Height) )
                {
                    xChildAtPosition = xChild;
                    break;
                }
            }
        }
    }

    // Have not found a child under the given point.  Returning empty
    // reference to indicate this.
    return xChildAtPosition;
}




awt::Rectangle SAL_CALL
    AccessibleDocumentViewBase::getBounds (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    // Transform visible area into screen coordinates.
    ::Rectangle aVisibleArea (
        maShapeTreeInfo.GetViewForwarder()->GetVisibleArea());
    ::Point aPixelTopLeft (
        maShapeTreeInfo.GetViewForwarder()->LogicToPixel (
            aVisibleArea.TopLeft()));
    ::Point aPixelSize (
        maShapeTreeInfo.GetViewForwarder()->LogicToPixel (
            aVisibleArea.BottomRight())
        - aPixelTopLeft);

    // Prepare to subtract the parent position to transform into relative
    // coordinates.
    awt::Point aParentPosition;
    Reference<XAccessible> xParent = getAccessibleParent ();
    if (xParent.is())
    {
        Reference<XAccessibleComponent> xParentComponent (
            xParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xParentComponent.is())
            aParentPosition = xParentComponent->getLocationOnScreen();
    }

    return awt::Rectangle (
        aPixelTopLeft.X() - aParentPosition.X,
        aPixelTopLeft.Y() - aParentPosition.Y,
        aPixelSize.X(),
        aPixelSize.Y());
}




awt::Point SAL_CALL
    AccessibleDocumentViewBase::getLocation (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();
    awt::Rectangle aBoundingBox (getBounds());
    return awt::Point (aBoundingBox.X, aBoundingBox.Y);
}




awt::Point SAL_CALL
    AccessibleDocumentViewBase::getLocationOnScreen (void)
    throw (uno::RuntimeException)
{
    ::Point aLogicalPoint (maShapeTreeInfo.GetViewForwarder()->GetVisibleArea().TopLeft());
    ::Point aPixelPoint (maShapeTreeInfo.GetViewForwarder()->LogicToPixel (aLogicalPoint));
    return awt::Point (aPixelPoint.X(), aPixelPoint.Y());
}




awt::Size SAL_CALL
    AccessibleDocumentViewBase::getSize (void)
    throw (uno::RuntimeException)
{
    // Transform visible area into screen coordinates.
    ::Rectangle aVisibleArea (
        maShapeTreeInfo.GetViewForwarder()->GetVisibleArea());
    ::Point aPixelTopLeft (
        maShapeTreeInfo.GetViewForwarder()->LogicToPixel (
            aVisibleArea.TopLeft()));
    ::Point aPixelSize (
        maShapeTreeInfo.GetViewForwarder()->LogicToPixel (
            aVisibleArea.BottomRight())
        - aPixelTopLeft);

    return awt::Size (aPixelSize.X(), aPixelSize.Y());
}




//=====  XInterface  ==========================================================

uno::Any SAL_CALL
    AccessibleDocumentViewBase::queryInterface (const uno::Type & rType)
    throw (uno::RuntimeException)
{
    uno::Any aReturn = AccessibleContextBase::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast<XAccessibleComponent*>(this),
            static_cast<XAccessibleSelection*>(this),
            static_cast<lang::XEventListener*>(
                static_cast<awt::XWindowListener*>(this)),
            static_cast<beans::XPropertyChangeListener*>(this),
            static_cast<awt::XWindowListener*>(this),
            static_cast<awt::XTopWindowListener*>(this)
            );
    return aReturn;
}




void SAL_CALL
    AccessibleDocumentViewBase::acquire (void)
    throw ()
{
    AccessibleContextBase::acquire ();
}




void SAL_CALL
    AccessibleDocumentViewBase::release (void)
    throw ()
{
    AccessibleContextBase::release ();
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessibleDocumentViewBase::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleDocumentViewBase"));
}




::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
    AccessibleDocumentViewBase::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return AccessibleContextBase::getSupportedServiceNames ();
}





//=====  XTypeProvider  =======================================================

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL
    AccessibleDocumentViewBase::getTypes (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    // Get list of types from the context base implementation, ...
    uno::Sequence<uno::Type> aTypeList (AccessibleContextBase::getTypes());
    // ... get list of types from component base implementation, ...
    uno::Sequence<uno::Type> aComponentTypeList (AccessibleComponentBase::getTypes());


    // ...and add the additional type for the component, ...
    const uno::Type aLangEventListenerType =
         ::getCppuType((const uno::Reference<lang::XEventListener>*)0);
    const uno::Type aPropertyChangeListenerType =
         ::getCppuType((const uno::Reference<beans::XPropertyChangeListener>*)0);
    const uno::Type aWindowListenerType =
         ::getCppuType((const uno::Reference<awt::XWindowListener>*)0);
    const uno::Type aTopWindowListenerType =
         ::getCppuType((const uno::Reference<awt::XTopWindowListener>*)0);
    const uno::Type aEventBroadcaster =
         ::getCppuType((const uno::Reference<XAccessibleEventBroadcaster>*)0);

    // ... and merge them all into one list.
    sal_Int32 nTypeCount (aTypeList.getLength()),
        nComponentTypeCount (aComponentTypeList.getLength()),
        i;

    aTypeList.realloc (nTypeCount + nComponentTypeCount + 5);

    for (i=0; i<nComponentTypeCount; i++)
        aTypeList[nTypeCount + i] = aComponentTypeList[i];

    aTypeList[nTypeCount + i++ ] = aLangEventListenerType;
    aTypeList[nTypeCount + i++] = aPropertyChangeListenerType;
    aTypeList[nTypeCount + i++] = aWindowListenerType;
    aTypeList[nTypeCount + i++] = aTopWindowListenerType;
    aTypeList[nTypeCount + i++] = aEventBroadcaster;

    return aTypeList;
}




//=====  XEventListener  ======================================================

void SAL_CALL
    AccessibleDocumentViewBase::disposing (const lang::EventObject& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    // Register this object as dispose event and document::XEventListener
    // listener at the model.

    if ( ! rEventObject.Source.is())
    {
        // Paranoia. Can this really happen?
    }
    else if (rEventObject.Source == mxModel)
    {
        ::osl::Guard< ::osl::Mutex> aGuard (::osl::Mutex::getGlobalMutex());

        mxModel->removeEventListener (
            static_cast<awt::XWindowListener*>(this));

        // Reset the model reference.
        mxModel = NULL;

        // Propagate change of controller down the shape tree.
        maShapeTreeInfo.SetControllerBroadcaster (NULL);
    }
    else if (rEventObject.Source == mxController)
    {
        ::osl::Guard< ::osl::Mutex> aGuard (::osl::Mutex::getGlobalMutex());

        // Unregister as property change listener at the controller.
        uno::Reference<beans::XPropertySet> xSet (mxController,uno::UNO_QUERY);
        if (xSet.is())
            xSet->removePropertyChangeListener (
                OUString (RTL_CONSTASCII_USTRINGPARAM("")),
                static_cast<beans::XPropertyChangeListener*>(this));

        // Reset the model reference.
        mxController = NULL;
    }
}




//=====  XPropertyChangeListener  =============================================

void SAL_CALL
    AccessibleDocumentViewBase::propertyChange (const beans::PropertyChangeEvent& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{}




//=====  XWindowListener  =====================================================

void SAL_CALL
    AccessibleDocumentViewBase::windowResized (const ::com::sun::star::awt::WindowEvent& e)
    throw (::com::sun::star::uno::RuntimeException)
{
    if( IsDisposed() )
        return;

    ViewForwarderChanged (
        IAccessibleViewForwarderListener::VISIBLE_AREA,
        &maViewForwarder);
}




void SAL_CALL
    AccessibleDocumentViewBase::windowMoved (const ::com::sun::star::awt::WindowEvent& e)
    throw (::com::sun::star::uno::RuntimeException)
{
    if( IsDisposed() )
        return;

    ViewForwarderChanged (
        IAccessibleViewForwarderListener::VISIBLE_AREA,
        &maViewForwarder);
}




void SAL_CALL
    AccessibleDocumentViewBase::windowShown (const ::com::sun::star::lang::EventObject& e)
    throw (::com::sun::star::uno::RuntimeException)
{
    if( IsDisposed() )
        return;

    ViewForwarderChanged (
        IAccessibleViewForwarderListener::VISIBLE_AREA,
        &maViewForwarder);
}




void SAL_CALL
    AccessibleDocumentViewBase::windowHidden (const ::com::sun::star::lang::EventObject& e)
    throw (::com::sun::star::uno::RuntimeException)
{
    if( IsDisposed() )
        return;

    ViewForwarderChanged (
        IAccessibleViewForwarderListener::VISIBLE_AREA,
        &maViewForwarder);
}




//=====  XTopWindowListener  ==================================================

void SAL_CALL AccessibleDocumentViewBase::windowOpened( const ::com::sun::star::lang::EventObject& e )
    throw (::com::sun::star::uno::RuntimeException)
{}

void SAL_CALL AccessibleDocumentViewBase::windowClosing( const ::com::sun::star::lang::EventObject& e )
    throw (::com::sun::star::uno::RuntimeException)
{}

void SAL_CALL AccessibleDocumentViewBase::windowClosed( const ::com::sun::star::lang::EventObject& e )
    throw (::com::sun::star::uno::RuntimeException)
{}

void SAL_CALL AccessibleDocumentViewBase::windowMinimized( const ::com::sun::star::lang::EventObject& e )
    throw (::com::sun::star::uno::RuntimeException)
{}

void SAL_CALL AccessibleDocumentViewBase::windowNormalized( const ::com::sun::star::lang::EventObject& e )
        throw (::com::sun::star::uno::RuntimeException)
{}

void SAL_CALL AccessibleDocumentViewBase::windowActivated( const ::com::sun::star::lang::EventObject& e )
        throw (::com::sun::star::uno::RuntimeException)
{
    if( IsDisposed() )
        return;

    Activated ();
}

void SAL_CALL AccessibleDocumentViewBase::windowDeactivated( const ::com::sun::star::lang::EventObject& e )
    throw (::com::sun::star::uno::RuntimeException)
{
    if( IsDisposed() )
        return;

    Deactivated ();
}




//=====  protected internal  ==================================================

// This method is called from the component helper base class while disposing.
void SAL_CALL AccessibleDocumentViewBase::disposing (void)
{
    // Unregister from VCL Window.
    Window* pWindow = maShapeTreeInfo.GetWindow();
    if (pWindow != NULL)
    {
        pWindow->RemoveChildEventListener (LINK(
            this, AccessibleDocumentViewBase, WindowChildEventListener));
    }
    else
        DBG_ASSERT (pWindow, "AccessibleDocumentViewBase::disposing");

    // Unregister from window.
    if (mxWindow.is())
        mxWindow->removeWindowListener (this);

    // Unregister form the model.
    if (mxModel.is())
        mxModel->removeEventListener (
            static_cast<awt::XWindowListener*>(this));

    // Unregister from the controller.
    uno::Reference<beans::XPropertySet> xSet (mxController, uno::UNO_QUERY);
    if (xSet.is())
        xSet->removePropertyChangeListener (
            OUString (RTL_CONSTASCII_USTRINGPARAM("")),
            static_cast<beans::XPropertyChangeListener*>(this));

    // Unregister from top window.
    if (mxController.is())
        if (mxController->getFrame().is())
        {
            uno::Reference<awt::XTopWindow> xTopWindow (
                uno::Reference<awt::XTopWindow> (
                    mxController->getFrame()->getContainerWindow(),
                    uno::UNO_QUERY));
            if (xTopWindow.is())
                xTopWindow->removeTopWindowListener (
                    static_cast<awt::XTopWindowListener*>(this));
        }

    AccessibleContextBase::disposing ();
}




/// Create a name for this view.
::rtl::OUString
    AccessibleDocumentViewBase::CreateAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString (
        RTL_CONSTASCII_USTRINGPARAM("AccessibleDocumentViewBase"));
}




/** Create a description for this view.  Use the model's description or URL
    if a description is not available.
*/
::rtl::OUString
    AccessibleDocumentViewBase::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    rtl::OUString sDescription;

    uno::Reference<lang::XServiceInfo> xInfo (mxController, uno::UNO_QUERY);
    if (xInfo.is())
    {
        OUString sFirstService = xInfo->getSupportedServiceNames()[0];
        if (sFirstService == OUString (
                RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocumentDrawView")))
        {
            sDescription = OUString (RTL_CONSTASCII_USTRINGPARAM("Draw Document"));
        }
        else
            sDescription = sFirstService;
    }
    else
        sDescription = OUString (
            RTL_CONSTASCII_USTRINGPARAM("Accessible Draw Document"));
    return sDescription;
}




void AccessibleDocumentViewBase::Activated (void)
{
    // Empty.  Overwrite to do something usefull.
}




void AccessibleDocumentViewBase::Deactivated (void)
{
    // Empty.  Overwrite to do something usefull.
}




void AccessibleDocumentViewBase::SetAccessibleOLEObject (
    const Reference <XAccessible>& xOLEObject)
{
    // Send child event about removed accessible OLE object if necessary.
    if (mxAccessibleOLEObject != xOLEObject)
        if (mxAccessibleOLEObject.is())
            CommitChange (
                AccessibleEventId::CHILD,
                uno::Any(),
                uno::makeAny (mxAccessibleOLEObject));

    // Assume that the accessible OLE Object disposes itself correctly.

    {
        ::osl::MutexGuard aGuard (maMutex);
        mxAccessibleOLEObject = xOLEObject;
    }

    // Send child event about new accessible OLE object if necessary.
    if (mxAccessibleOLEObject.is())
        CommitChange (
            AccessibleEventId::CHILD,
            uno::makeAny (mxAccessibleOLEObject),
            uno::Any());
}




//=====  methods from AccessibleSelectionBase ==================================================

// return the member maMutex;
::osl::Mutex&
    AccessibleDocumentViewBase::implGetMutex()
{
    return( maMutex );
}

// return ourself as context in default case
uno::Reference< XAccessibleContext >
    AccessibleDocumentViewBase::implGetAccessibleContext()
    throw (uno::RuntimeException)
{
    return( this );
}

// return sal_False in default case
sal_Bool
    AccessibleDocumentViewBase::implIsSelected( sal_Int32 nAccessibleChildIndex )
    throw (uno::RuntimeException)
{
    return( sal_False );
}

// return nothing in default case
void
    AccessibleDocumentViewBase::implSelect( sal_Int32 nAccessibleChildIndex, sal_Bool bSelect )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
}

} // end of namespace accessibility
