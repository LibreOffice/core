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

#include <AccessibleDocumentViewBase.hxx>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <sfx2/objsh.hxx>

#include <svx/svdobj.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <Window.hxx>
#include <vcl/svapp.hxx>
#include <OutlineViewShell.hxx>

#include <svx/svdlayer.hxx>
#include <editeng/editobj.hxx>
#include <LayerTabBar.hxx>
#include <svtools/colorcfg.hxx>
#include <ViewShell.hxx>
#include <View.hxx>
#include <drawdoc.hxx>
#include <editeng/outlobj.hxx>
#include <sdpage.hxx>
#include <DrawViewShell.hxx>
#include <PresentationViewShell.hxx>

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
    : AccessibleContextBase (rxParent,
                             pViewShell->GetDoc()->GetDocumentType() == DocumentType::Impress ?
                                     AccessibleRole::DOCUMENT_PRESENTATION :
                                     AccessibleRole::DOCUMENT),
      mpWindow (pSdWindow),
      mxController (rxController),
      maViewForwarder (
        static_cast<SdrPaintView*>(pViewShell->GetView()),
        *pSdWindow)
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
    mpViewShell = pViewShell;
}

AccessibleDocumentViewBase::~AccessibleDocumentViewBase()
{
    // At this place we should be disposed.  You may want to add a
    // corresponding assertion into the destructor of a derived class.

    SolarMutexGuard g;
    mpWindow.reset();
}

void AccessibleDocumentViewBase::Init()
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
        xShapeList.set( xView->getCurrentPage(), uno::UNO_QUERY);

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
    vcl::Window* pWindow = maShapeTreeInfo.GetWindow();
    if (pWindow != nullptr)
    {
        maWindowLink = LINK(
            this, AccessibleDocumentViewBase, WindowChildEventListener);

        pWindow->AddChildEventListener (maWindowLink);

        sal_uInt16 nCount = pWindow->GetChildCount();
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            vcl::Window* pChildWindow = pWindow->GetChild (i);
            if (pChildWindow &&
                (AccessibleRole::EMBEDDED_OBJECT
                    ==pChildWindow->GetAccessibleRole()))
            {
                SetAccessibleOLEObject (pChildWindow->GetAccessible());
            }
        }
    }
    SfxObjectShell* pObjShell = mpViewShell->GetViewFrame()->GetObjectShell();
    if(!pObjShell->IsReadOnly())
        SetState(AccessibleStateType::EDITABLE);
}

IMPL_LINK(AccessibleDocumentViewBase, WindowChildEventListener,
    VclWindowEvent&, rEvent, void)
{
        //      DBG_ASSERT( pVclEvent->GetWindow(), "Window???" );
        switch (rEvent.GetId())
        {
            case VclEventId::ObjectDying:
            {
                // Window is dying.  Unregister from VCL Window.
                // This is also attempted in the disposing() method.
                vcl::Window* pWindow = maShapeTreeInfo.GetWindow();
                vcl::Window* pDyingWindow = rEvent.GetWindow();
                if (pWindow==pDyingWindow && pWindow!=nullptr && maWindowLink.IsSet())
                {
                    pWindow->RemoveChildEventListener (maWindowLink);
                    maWindowLink = Link<VclWindowEvent&,void>();
                }
            }
            break;

            case VclEventId::WindowShow:
            {
                // A new window has been created.  Is it an OLE object?
                vcl::Window* pChildWindow = static_cast<vcl::Window*>(
                    rEvent.GetData());
                if (pChildWindow!=nullptr
                    && (pChildWindow->GetAccessibleRole()
                        == AccessibleRole::EMBEDDED_OBJECT))
                {
                    SetAccessibleOLEObject (pChildWindow->GetAccessible());
                }
            }
            break;

            case VclEventId::WindowHide:
            {
                // A window has been destroyed.  Has that been an OLE
                // object?
                vcl::Window* pChildWindow = static_cast<vcl::Window*>(
                    rEvent.GetData());
                if (pChildWindow!=nullptr
                    && (pChildWindow->GetAccessibleRole()
                        == AccessibleRole::EMBEDDED_OBJECT))
                {
                    SetAccessibleOLEObject (nullptr);
                }
            }
            break;

            default: break;
        }
}

//=====  IAccessibleViewForwarderListener  ====================================

void AccessibleDocumentViewBase::ViewForwarderChanged()
{
    // Empty
}

//=====  XAccessibleContext  ==================================================

Reference<XAccessible> SAL_CALL
       AccessibleDocumentViewBase::getAccessibleParent()
{
    ThrowIfDisposed ();

    return AccessibleContextBase::getAccessibleParent();
}

sal_Int32 SAL_CALL
    AccessibleDocumentViewBase::getAccessibleChildCount()
{
    ThrowIfDisposed ();

    if (mxAccessibleOLEObject.is())
        return 1;
    else
        return 0;
}

Reference<XAccessible> SAL_CALL
    AccessibleDocumentViewBase::getAccessibleChild (sal_Int32 nIndex)
{
    ThrowIfDisposed ();

    ::osl::MutexGuard aGuard (maMutex);
    if (mxAccessibleOLEObject.is())
        if (nIndex == 0)
            return mxAccessibleOLEObject;

    throw lang::IndexOutOfBoundsException ( "no child with index " + OUString::number(nIndex) );
}

//=====  XAccessibleComponent  ================================================

/** Iterate over all children and test whether the specified point lies
    within one of their bounding boxes.  Return the first child for which
    this is true.
*/
uno::Reference<XAccessible > SAL_CALL
    AccessibleDocumentViewBase::getAccessibleAtPoint (
        const awt::Point& aPoint)
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
    AccessibleDocumentViewBase::getBounds()
{
    ThrowIfDisposed ();

    // Transform visible area into screen coordinates.
    ::tools::Rectangle aVisibleArea (
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
    AccessibleDocumentViewBase::getLocation()
{
    ThrowIfDisposed ();
    awt::Rectangle aBoundingBox (getBounds());
    return awt::Point (aBoundingBox.X, aBoundingBox.Y);
}

awt::Point SAL_CALL
    AccessibleDocumentViewBase::getLocationOnScreen()
{
    ThrowIfDisposed ();
    ::Point aLogicalPoint (maShapeTreeInfo.GetViewForwarder()->GetVisibleArea().TopLeft());
    ::Point aPixelPoint (maShapeTreeInfo.GetViewForwarder()->LogicToPixel (aLogicalPoint));
    return awt::Point (aPixelPoint.X(), aPixelPoint.Y());
}

awt::Size SAL_CALL
    AccessibleDocumentViewBase::getSize()
{
    ThrowIfDisposed ();

    // Transform visible area into screen coordinates.
    ::tools::Rectangle aVisibleArea (
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
           ,static_cast<XAccessibleExtendedAttributes*>(this)
           ,static_cast<XAccessibleGetAccFlowTo*>(this)
            );
    return aReturn;
}

void SAL_CALL
    AccessibleDocumentViewBase::acquire()
    throw ()
{
    AccessibleContextBase::acquire ();
}

void SAL_CALL
    AccessibleDocumentViewBase::release()
    throw ()
{
    AccessibleContextBase::release ();
}

// XServiceInfo

OUString SAL_CALL
    AccessibleDocumentViewBase::getImplementationName()
{
    return OUString("AccessibleDocumentViewBase");
}

css::uno::Sequence< OUString> SAL_CALL
    AccessibleDocumentViewBase::getSupportedServiceNames()
{
    ThrowIfDisposed ();
    return AccessibleContextBase::getSupportedServiceNames ();
}

//=====  XTypeProvider  =======================================================

css::uno::Sequence< css::uno::Type> SAL_CALL
    AccessibleDocumentViewBase::getTypes()
{
    ThrowIfDisposed ();

    return comphelper::concatSequences(
        // Get list of types from the context base implementation, ...
        AccessibleContextBase::getTypes(),
        // ... get list of types from component base implementation, ...
        AccessibleComponentBase::getTypes(),
        // ...and add the additional type for the component, ...
        css::uno::Sequence {
         cppu::UnoType<lang::XEventListener>::get(),
         cppu::UnoType<beans::XPropertyChangeListener>::get(),
         cppu::UnoType<awt::XWindowListener>::get(),
         cppu::UnoType<awt::XFocusListener>::get(),
         cppu::UnoType<XAccessibleEventBroadcaster>::get() });
}

void AccessibleDocumentViewBase::impl_dispose()
{
    // Unregister from VCL Window.
    vcl::Window* pWindow = maShapeTreeInfo.GetWindow();
    if (maWindowLink.IsSet())
    {
        if (pWindow)
            pWindow->RemoveChildEventListener (maWindowLink);
        maWindowLink = Link<VclWindowEvent&,void>();
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
        mxWindow = nullptr;
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
    maShapeTreeInfo.SetModelBroadcaster (nullptr);

    // Reset the model reference.
    mxModel = nullptr;
    // Reset the model reference.
    mxController = nullptr;

    maShapeTreeInfo.SetDocumentWindow (nullptr);
}

//=====  XEventListener  ======================================================

void SAL_CALL
    AccessibleDocumentViewBase::disposing (const lang::EventObject& rEventObject)
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
{
    // Empty
}

//=====  XWindowListener  =====================================================

void SAL_CALL
    AccessibleDocumentViewBase::windowResized (const css::awt::WindowEvent& )
{
    if( IsDisposed() )
        return;

    ViewForwarderChanged();
}

void SAL_CALL
    AccessibleDocumentViewBase::windowMoved (const css::awt::WindowEvent& )
{
    if( IsDisposed() )
        return;

    ViewForwarderChanged();
}

void SAL_CALL
    AccessibleDocumentViewBase::windowShown (const css::lang::EventObject& )
{
    if( IsDisposed() )
        return;

    ViewForwarderChanged();
}

void SAL_CALL
    AccessibleDocumentViewBase::windowHidden (const css::lang::EventObject& )
{
    if( IsDisposed() )
        return;

    ViewForwarderChanged();
}

//=====  XFocusListener  ==================================================

void AccessibleDocumentViewBase::focusGained (const css::awt::FocusEvent& e)
{
    ThrowIfDisposed ();
    if (e.Source == mxWindow)
        Activated ();
}

void AccessibleDocumentViewBase::focusLost (const css::awt::FocusEvent& e)
{
    ThrowIfDisposed ();
    if (e.Source == mxWindow)
        Deactivated ();
}

//=====  protected internal  ==================================================

// This method is called from the component helper base class while disposing.
void SAL_CALL AccessibleDocumentViewBase::disposing()
{
    impl_dispose();

    AccessibleContextBase::disposing ();
}

/// Create a name for this view.
OUString
    AccessibleDocumentViewBase::CreateAccessibleName()
{
    return OUString ("AccessibleDocumentViewBase");
}

void AccessibleDocumentViewBase::Activated()
{
    // Empty.  Overwrite to do something useful.
}

void AccessibleDocumentViewBase::Deactivated()
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
    return maMutex;
}

// return ourself as context in default case
uno::Reference< XAccessibleContext >
    AccessibleDocumentViewBase::implGetAccessibleContext()
{
    return this;
}

// return sal_False in default case
bool
    AccessibleDocumentViewBase::implIsSelected( sal_Int32 )
{
    return false;
}

// return nothing in default case
void
    AccessibleDocumentViewBase::implSelect( sal_Int32, bool )
{
}

uno::Any SAL_CALL AccessibleDocumentViewBase::getExtendedAttributes()
{
    ::osl::MutexGuard aGuard (maMutex);

    uno::Any anyAtrribute;
    OUStringBuffer sValue;
    if (nullptr != dynamic_cast<const ::sd::DrawViewShell* > (mpViewShell))
    {
        ::sd::DrawViewShell* pDrViewSh = static_cast< ::sd::DrawViewShell*>(mpViewShell);
        OUString sDisplay;
        OUString sName = "page-name:";
        // MT IA2: Not used...
        // SdPage*  pCurrPge = pDrViewSh->getCurrentPage();
        SdDrawDocument* pDoc = pDrViewSh->GetDoc();
        sDisplay = pDrViewSh->getCurrentPage()->GetName();
        sDisplay = sDisplay.replaceFirst( "\\", "\\\\" );
        sDisplay = sDisplay.replaceFirst( "=", "\\=" );
        sDisplay = sDisplay.replaceFirst( ";", "\\;" );
        sDisplay = sDisplay.replaceFirst( ",", "\\," );
        sDisplay = sDisplay.replaceFirst( ":", "\\:" );
        sValue = sName + sDisplay ;
        sValue.append(";page-number:");
        sValue.append(OUString::number(static_cast<sal_Int16>(static_cast<sal_uInt16>((pDrViewSh->getCurrentPage()->GetPageNum()-1)>>1) + 1)) );
        sValue.append(";total-pages:");
        sValue.append(OUString::number(pDrViewSh->GetPageTabControl().GetPageCount()) );
        sValue.append(";");
        if(pDrViewSh->IsLayerModeActive() && pDrViewSh->GetLayerTabControl()) // #i87182#
        {
            sName = "page-name:";
            sValue = sName;
            OUString sLayerName(pDrViewSh->GetLayerTabControl()->GetLayerName(pDrViewSh->GetLayerTabControl()->GetCurPageId()) );
            sDisplay = pDrViewSh->GetLayerTabControl()->GetPageText(pDrViewSh->GetLayerTabControl()->GetCurPageId());
            if( pDoc )
            {
                SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
                SdrLayer* aSdrLayer = rLayerAdmin.GetLayer(sLayerName);
                if( aSdrLayer )
                {
                    const OUString& layerAltText = aSdrLayer->GetTitle();
                    if (!layerAltText.isEmpty())
                    {
                        sName = " ";
                        sDisplay = sDisplay + sName;
                        sDisplay += layerAltText;
                    }
                }
            }
            sDisplay = sDisplay.replaceFirst( "\\", "\\\\" );
            sDisplay = sDisplay.replaceFirst( "=", "\\=" );
            sDisplay = sDisplay.replaceFirst( ";", "\\;" );
            sDisplay = sDisplay.replaceFirst( ",", "\\," );
            sDisplay = sDisplay.replaceFirst( ":", "\\:" );
            sValue.append(sDisplay);
            sValue.append(";page-number:");
            sValue.append(OUString::number(pDrViewSh->GetActiveTabLayerIndex()+1) );
            sValue.append(";total-pages:");
            sValue.append(OUString::number(pDrViewSh->GetLayerTabControl()->GetPageCount()) );
            sValue.append(";");
        }
    }
    if (dynamic_cast<const ::sd::PresentationViewShell* >(mpViewShell) !=  nullptr )
    {
        ::sd::PresentationViewShell* pPresViewSh = static_cast< ::sd::PresentationViewShell*>(mpViewShell);
        SdPage* pCurrPge = pPresViewSh->getCurrentPage();
        SdDrawDocument* pDoc = pPresViewSh->GetDoc();
        SdPage* pNotesPge = pDoc->GetSdPage((pCurrPge->GetPageNum()-1)>>1, PageKind::Notes);
        if (pNotesPge)
        {
            SdrObject* pNotesObj = pNotesPge->GetPresObj(PRESOBJ_NOTES);
            if (pNotesObj)
            {
                OutlinerParaObject* pPara = pNotesObj->GetOutlinerParaObject();
                if (pPara)
                {
                    sValue.append("note:");
                    const EditTextObject& rEdit = pPara->GetTextObject();
                    for (sal_Int32 i=0;i<rEdit.GetParagraphCount();i++)
                    {
                        OUString strNote = rEdit.GetText(i);
                        strNote = strNote.replaceFirst( "\\", "\\\\" );
                        strNote = strNote.replaceFirst( "=", "\\=" );
                        strNote = strNote.replaceFirst( ";", "\\;" );
                        strNote = strNote.replaceFirst( ",", "\\," );
                        strNote = strNote.replaceFirst( ":", "\\:" );
                        sValue.append(strNote);
                        sValue.append(";");//to divide each paragraph
                    }
                }
            }
        }
    }
    if (dynamic_cast<const ::sd::OutlineViewShell* >(mpViewShell ) !=  nullptr )
    {
        OUString sDisplay;
        SdPage* pCurrPge = mpViewShell->GetActualPage();
        SdDrawDocument* pDoc = mpViewShell->GetDoc();
        if(pCurrPge && pDoc)
        {
            sDisplay = pCurrPge->GetName();
            sDisplay = sDisplay.replaceFirst( "=", "\\=" );
            sDisplay = sDisplay.replaceFirst( ";", "\\;" );
            sDisplay = sDisplay.replaceFirst( ",", "\\," );
            sDisplay = sDisplay.replaceFirst( ":", "\\:" );
            sValue = "page-name:" + sDisplay;
            sValue.append(";page-number:");
            sValue.append(OUString::number(static_cast<sal_Int16>(static_cast<sal_uInt16>((pCurrPge->GetPageNum()-1)>>1) + 1)) );
            sValue.append(";total-pages:");
            sValue.append(OUString::number(pDoc->GetSdPageCount(PageKind::Standard)) );
            sValue.append(";");
        }
    }
    if (sValue.getLength())
        anyAtrribute <<= sValue.makeStringAndClear();
    return anyAtrribute;
}

css::uno::Sequence< css::uno::Any >
        SAL_CALL AccessibleDocumentViewBase::getAccFlowTo(const css::uno::Any&, sal_Int32 )
{
    css::uno::Sequence< uno::Any> aRet;

    return aRet;
}

sal_Int32 SAL_CALL AccessibleDocumentViewBase::getForeground(  )
{
    return sal_Int32(COL_BLACK);
}

sal_Int32 SAL_CALL AccessibleDocumentViewBase::getBackground(  )
{
    ThrowIfDisposed ();
    ::osl::MutexGuard aGuard (maMutex);
    return sal_Int32(mpViewShell->GetView()->getColorConfig().GetColorValue( ::svtools::DOCCOLOR ).nColor);
}
} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
