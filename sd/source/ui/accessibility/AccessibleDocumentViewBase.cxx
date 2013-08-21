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

#include "AccessibleDocumentViewBase.hxx"
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
#include "Window.hxx"
#include <vcl/svapp.hxx>


#include "ViewShell.hxx"
#include "View.hxx"
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {

//=====  internal  ============================================================
AccessibleDocumentViewBase::AccessibleDocumentViewBase (
    ::sd::Window* pSdWindow,
    ::sd::ViewShell* pViewShell,
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
    // Register as focus listener to
    mxWindow->addFocusListener (this);

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
            "",
            static_cast<beans::XPropertyChangeListener*>(this));

    // Register this object as dispose event listener at the controller.
    if (mxController.is())
        mxController->addEventListener (
            static_cast<awt::XWindowListener*>(this));

    // Register at VCL Window to be informed of activated and deactivated
    // OLE objects.
    Window* pWindow = maShapeTreeInfo.GetWindow();
    if (pWindow != NULL)
    {
        maWindowLink = LINK(
            this, AccessibleDocumentViewBase, WindowChildEventListener);

        pWindow->AddChildEventListener (maWindowLink);

        sal_uInt16 nCount = pWindow->GetChildCount();
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
                if (pWindow==pDyingWindow && pWindow!=NULL && maWindowLink.IsSet())
                {
                    pWindow->RemoveChildEventListener (maWindowLink);
                    maWindowLink = Link();
                }
            }
            break;

            case VCLEVENT_WINDOW_SHOW:
            {
                // A new window has been created.  Is it an OLE object?
                Window* pChildWindow = static_cast<Window*>(
                    pWindowEvent->GetData());
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
                    pWindowEvent->GetData());
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

void AccessibleDocumentViewBase::ViewForwarderChanged(ChangeType, const IAccessibleViewForwarder* )
{
    // Empty
}




//=====  XAccessibleContext  ==================================================

Reference<XAccessible> SAL_CALL
       AccessibleDocumentViewBase::getAccessibleParent (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();

    return AccessibleContextBase::getAccessibleParent();
}



sal_Int32 SAL_CALL
    AccessibleDocumentViewBase::getAccessibleChildCount (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();

    if (mxAccessibleOLEObject.is())
        return 1;
    else
        return 0;
}




Reference<XAccessible> SAL_CALL
    AccessibleDocumentViewBase::getAccessibleChild (sal_Int32 nIndex)
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    ThrowIfDisposed ();

    ::osl::MutexGuard aGuard (maMutex);
    if (mxAccessibleOLEObject.is())
        if (nIndex == 0)
            return mxAccessibleOLEObject;

    throw lang::IndexOutOfBoundsException ( "no child with index " + OUString::number(nIndex), NULL);
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
    ThrowIfDisposed ();

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
    ThrowIfDisposed ();

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
    ThrowIfDisposed ();
    ::Point aLogicalPoint (maShapeTreeInfo.GetViewForwarder()->GetVisibleArea().TopLeft());
    ::Point aPixelPoint (maShapeTreeInfo.GetViewForwarder()->LogicToPixel (aLogicalPoint));
    return awt::Point (aPixelPoint.X(), aPixelPoint.Y());
}




awt::Size SAL_CALL
    AccessibleDocumentViewBase::getSize (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();

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
            static_cast<awt::XFocusListener*>(this)
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

OUString SAL_CALL
    AccessibleDocumentViewBase::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString("AccessibleDocumentViewBase");
}




::com::sun::star::uno::Sequence< OUString> SAL_CALL
    AccessibleDocumentViewBase::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    return AccessibleContextBase::getSupportedServiceNames ();
}





//=====  XTypeProvider  =======================================================

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL
    AccessibleDocumentViewBase::getTypes (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

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
    const uno::Type aFocusListenerType =
         ::getCppuType((const uno::Reference<awt::XFocusListener>*)0);
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
    aTypeList[nTypeCount + i++] = aFocusListenerType;
    aTypeList[nTypeCount + i++] = aEventBroadcaster;

    return aTypeList;
}




void AccessibleDocumentViewBase::impl_dispose()
{
    // Unregister from VCL Window.
    Window* pWindow = maShapeTreeInfo.GetWindow();
    if (maWindowLink.IsSet())
    {
        if (pWindow)
            pWindow->RemoveChildEventListener (maWindowLink);
        maWindowLink = Link();
    }
    else
    {
        DBG_ASSERT (pWindow, "AccessibleDocumentViewBase::disposing");
    }

    // Unregister from window.
    if (mxWindow.is())
    {
        mxWindow->removeWindowListener (this);
        mxWindow->removeFocusListener (this);
        mxWindow = NULL;
    }

    // Unregister form the model.
    if (mxModel.is())
        mxModel->removeEventListener (
            static_cast<awt::XWindowListener*>(this));

    // Unregister from the controller.
    if (mxController.is())
    {
        uno::Reference<beans::XPropertySet> xSet (mxController, uno::UNO_QUERY);
        if (xSet.is())
            xSet->removePropertyChangeListener ("", static_cast<beans::XPropertyChangeListener*>(this));

        mxController->removeEventListener (
            static_cast<awt::XWindowListener*>(this));
    }

    // Propagate change of controller down the shape tree.
    maShapeTreeInfo.SetControllerBroadcaster (NULL);

    // Reset the model reference.
    mxModel = NULL;
    // Reset the model reference.
    mxController = NULL;

    maShapeTreeInfo.SetDocumentWindow (NULL);
}




//=====  XEventListener  ======================================================

void SAL_CALL
    AccessibleDocumentViewBase::disposing (const lang::EventObject& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    // Register this object as dispose event and document::XEventListener
    // listener at the model.

    if ( ! rEventObject.Source.is())
    {
        // Paranoia. Can this really happen?
    }
    else if (rEventObject.Source == mxModel || rEventObject.Source == mxController)
    {
        impl_dispose();
    }
}

//=====  XPropertyChangeListener  =============================================

void SAL_CALL AccessibleDocumentViewBase::propertyChange (const beans::PropertyChangeEvent& )
    throw (::com::sun::star::uno::RuntimeException)
{
    // Empty
}




//=====  XWindowListener  =====================================================

void SAL_CALL
    AccessibleDocumentViewBase::windowResized (const ::com::sun::star::awt::WindowEvent& )
    throw (::com::sun::star::uno::RuntimeException)
{
    if( IsDisposed() )
        return;

    ViewForwarderChanged (
        IAccessibleViewForwarderListener::VISIBLE_AREA,
        &maViewForwarder);
}




void SAL_CALL
    AccessibleDocumentViewBase::windowMoved (const ::com::sun::star::awt::WindowEvent& )
    throw (::com::sun::star::uno::RuntimeException)
{
    if( IsDisposed() )
        return;

    ViewForwarderChanged (
        IAccessibleViewForwarderListener::VISIBLE_AREA,
        &maViewForwarder);
}




void SAL_CALL
    AccessibleDocumentViewBase::windowShown (const ::com::sun::star::lang::EventObject& )
    throw (::com::sun::star::uno::RuntimeException)
{
    if( IsDisposed() )
        return;

    ViewForwarderChanged (
        IAccessibleViewForwarderListener::VISIBLE_AREA,
        &maViewForwarder);
}




void SAL_CALL
    AccessibleDocumentViewBase::windowHidden (const ::com::sun::star::lang::EventObject& )
    throw (::com::sun::star::uno::RuntimeException)
{
    if( IsDisposed() )
        return;

    ViewForwarderChanged (
        IAccessibleViewForwarderListener::VISIBLE_AREA,
        &maViewForwarder);
}




//=====  XFocusListener  ==================================================

void AccessibleDocumentViewBase::focusGained (const ::com::sun::star::awt::FocusEvent& e)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    if (e.Source == mxWindow)
        Activated ();
}

void AccessibleDocumentViewBase::focusLost (const ::com::sun::star::awt::FocusEvent& e)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    if (e.Source == mxWindow)
        Deactivated ();
}




//=====  protected internal  ==================================================

// This method is called from the component helper base class while disposing.
void SAL_CALL AccessibleDocumentViewBase::disposing (void)
{
    impl_dispose();

    AccessibleContextBase::disposing ();
}




/// Create a name for this view.
OUString
    AccessibleDocumentViewBase::CreateAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString ("AccessibleDocumentViewBase");
}




/** Create a description for this view.  Use the model's description or URL
    if a description is not available.
*/
OUString
    AccessibleDocumentViewBase::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    OUString sDescription;

    uno::Reference<lang::XServiceInfo> xInfo (mxController, uno::UNO_QUERY);
    if (xInfo.is())
    {
        OUString sFirstService = xInfo->getSupportedServiceNames()[0];
        if ( sFirstService == "com.sun.star.drawing.DrawingDocumentDrawView" )
        {
            sDescription = "Draw Document";
        }
        else
            sDescription = sFirstService;
    }
    else
        sDescription = "Accessible Draw Document";
    return sDescription;
}




void AccessibleDocumentViewBase::Activated (void)
{
    // Empty.  Overwrite to do something useful.
}




void AccessibleDocumentViewBase::Deactivated (void)
{
    // Empty.  Overwrite to do something useful.
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
    AccessibleDocumentViewBase::implIsSelected( sal_Int32 )
    throw (uno::RuntimeException)
{
    return( sal_False );
}

// return nothing in default case
void
    AccessibleDocumentViewBase::implSelect( sal_Int32, sal_Bool )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
