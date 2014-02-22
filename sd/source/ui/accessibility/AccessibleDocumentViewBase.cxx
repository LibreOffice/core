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
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <sfx2/objsh.hxx>
#include <svx/AccessibleShape.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unoapi.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include "Window.hxx"
#include <vcl/svapp.hxx>
#include "OutlineViewShell.hxx"

#include <svx/svdlayer.hxx>
#include <editeng/editobj.hxx>
#include "LayerTabBar.hxx"
#include <svtools/colorcfg.hxx>
#include "ViewShell.hxx"
#include "View.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {


AccessibleDocumentViewBase::AccessibleDocumentViewBase (
    ::sd::Window* pSdWindow,
    ::sd::ViewShell* pViewShell,
    const uno::Reference<frame::XController>& rxController,
    const uno::Reference<XAccessible>& rxParent)
    : AccessibleContextBase (rxParent,
                             pViewShell->GetDoc()->GetDocumentType() == DOCUMENT_TYPE_IMPRESS ?
                                     AccessibleRole::DOCUMENT_PRESENTATION :
                                     AccessibleRole::DOCUMENT),
      mpWindow (pSdWindow),
      mxController (rxController),
      mxModel (NULL),
      maViewForwarder (
        static_cast<SdrPaintView*>(pViewShell->GetView()),
        *static_cast<OutputDevice*>(pSdWindow))
{
    if (mxController.is())
        mxModel = mxController->getModel();

    
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




AccessibleDocumentViewBase::~AccessibleDocumentViewBase (void)
{
    
    
}




void AccessibleDocumentViewBase::Init (void)
{
    
    maShapeTreeInfo.SetDocumentWindow (this);

    
    
    mxWindow->addWindowListener (this);
    
    mxWindow->addFocusListener (this);

    
    uno::Reference<drawing::XShapes> xShapeList;
    uno::Reference<drawing::XDrawView> xView (mxController, uno::UNO_QUERY);
    if (xView.is())
        xShapeList = uno::Reference<drawing::XShapes> (
            xView->getCurrentPage(), uno::UNO_QUERY);

    
    if (mxModel.is())
        mxModel->addEventListener (
            static_cast<awt::XWindowListener*>(this));

    
    uno::Reference<beans::XPropertySet> xSet (mxController, uno::UNO_QUERY);
    if (xSet.is())
        xSet->addPropertyChangeListener (
            "",
            static_cast<beans::XPropertyChangeListener*>(this));

    
    if (mxController.is())
        mxController->addEventListener (
            static_cast<awt::XWindowListener*>(this));

    
    
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
    SfxObjectShell* pObjShell = mpViewShell->GetViewFrame()->GetObjectShell();
    if(!pObjShell->IsReadOnly())
        SetState(AccessibleStateType::EDITABLE);
}




IMPL_LINK(AccessibleDocumentViewBase, WindowChildEventListener,
    VclSimpleEvent*, pEvent)
{
    OSL_ASSERT(pEvent!=NULL && pEvent->ISA(VclWindowEvent));
    if (pEvent!=NULL && pEvent->ISA(VclWindowEvent))
    {
        VclWindowEvent* pWindowEvent = static_cast<VclWindowEvent*>(pEvent);
        
        switch (pWindowEvent->GetId())
        {
            case VCLEVENT_OBJECT_DYING:
            {
                
                
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






void AccessibleDocumentViewBase::ViewForwarderChanged(ChangeType, const IAccessibleViewForwarder* )
{
    
}






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

    
    
    return xChildAtPosition;
}




awt::Rectangle SAL_CALL
    AccessibleDocumentViewBase::getBounds (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    
    ::Rectangle aVisibleArea (
        maShapeTreeInfo.GetViewForwarder()->GetVisibleArea());
    ::Point aPixelTopLeft (
        maShapeTreeInfo.GetViewForwarder()->LogicToPixel (
            aVisibleArea.TopLeft()));
    ::Point aPixelSize (
        maShapeTreeInfo.GetViewForwarder()->LogicToPixel (
            aVisibleArea.BottomRight())
        - aPixelTopLeft);

    
    
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
           ,static_cast<XAccessibleExtendedAttributes*>(this)
           ,static_cast<XAccessibleGetAccFlowTo*>(this)
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







::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL
    AccessibleDocumentViewBase::getTypes (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    
    uno::Sequence<uno::Type> aTypeList (AccessibleContextBase::getTypes());
    
    uno::Sequence<uno::Type> aComponentTypeList (AccessibleComponentBase::getTypes());


    
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

    
    if (mxWindow.is())
    {
        mxWindow->removeWindowListener (this);
        mxWindow->removeFocusListener (this);
        mxWindow = NULL;
    }

    
    if (mxModel.is())
        mxModel->removeEventListener (
            static_cast<awt::XWindowListener*>(this));

    
    if (mxController.is())
    {
        uno::Reference<beans::XPropertySet> xSet (mxController, uno::UNO_QUERY);
        if (xSet.is())
            xSet->removePropertyChangeListener ("", static_cast<beans::XPropertyChangeListener*>(this));

        mxController->removeEventListener (
            static_cast<awt::XWindowListener*>(this));
    }

    
    maShapeTreeInfo.SetControllerBroadcaster (NULL);

    
    mxModel = NULL;
    
    mxController = NULL;

    maShapeTreeInfo.SetDocumentWindow (NULL);
}






void SAL_CALL
    AccessibleDocumentViewBase::disposing (const lang::EventObject& rEventObject)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    
    

    if ( ! rEventObject.Source.is())
    {
        
    }
    else if (rEventObject.Source == mxModel || rEventObject.Source == mxController)
    {
        impl_dispose();
    }
}



void SAL_CALL AccessibleDocumentViewBase::propertyChange (const beans::PropertyChangeEvent& )
    throw (::com::sun::star::uno::RuntimeException)
{
    
}






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







void SAL_CALL AccessibleDocumentViewBase::disposing (void)
{
    impl_dispose();

    AccessibleContextBase::disposing ();
}





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
    
}




void AccessibleDocumentViewBase::Deactivated (void)
{
    
}




void AccessibleDocumentViewBase::SetAccessibleOLEObject (
    const Reference <XAccessible>& xOLEObject)
{
    
    if (mxAccessibleOLEObject != xOLEObject)
        if (mxAccessibleOLEObject.is())
            CommitChange (
                AccessibleEventId::CHILD,
                uno::Any(),
                uno::makeAny (mxAccessibleOLEObject));

    

    {
        ::osl::MutexGuard aGuard (maMutex);
        mxAccessibleOLEObject = xOLEObject;
    }

    
    if (mxAccessibleOLEObject.is())
        CommitChange (
            AccessibleEventId::CHILD,
            uno::makeAny (mxAccessibleOLEObject),
            uno::Any());
}







::osl::Mutex&
    AccessibleDocumentViewBase::implGetMutex()
{
    return( maMutex );
}


uno::Reference< XAccessibleContext >
    AccessibleDocumentViewBase::implGetAccessibleContext()
    throw (uno::RuntimeException)
{
    return( this );
}


bool
    AccessibleDocumentViewBase::implIsSelected( sal_Int32 )
    throw (uno::RuntimeException)
{
    return( false );
}


void
    AccessibleDocumentViewBase::implSelect( sal_Int32, sal_Bool )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
}

uno::Any SAL_CALL AccessibleDocumentViewBase::getExtendedAttributes()
    throw (::com::sun::star::lang::IndexOutOfBoundsException,
           ::com::sun::star::uno::RuntimeException,
           std::exception)
{
    ::osl::MutexGuard aGuard (maMutex);

    uno::Any anyAtrribute;
    OUString sValue;
    if (mpViewShell && mpViewShell->ISA(::sd::DrawViewShell))
    {
        ::sd::DrawViewShell* pDrViewSh = (::sd::DrawViewShell*) mpViewShell;
        OUString sDisplay;
        OUString sName = "page-name:";
        
        
        SdDrawDocument* pDoc = pDrViewSh->GetDoc();
        sDisplay = pDrViewSh->getCurrentPage()->GetName();
        sDisplay = sDisplay.replaceFirst( "\\", "\\\\" );
        sDisplay = sDisplay.replaceFirst( "=", "\\=" );
        sDisplay = sDisplay.replaceFirst( ";", "\\;" );
        sDisplay = sDisplay.replaceFirst( ",", "\\," );
        sDisplay = sDisplay.replaceFirst( ":", "\\:" );
        sValue = sName + sDisplay ;
        sName = ";page-number:";
        sValue += sName;
        sValue += OUString::number((sal_Int16)((sal_uInt16)((pDrViewSh->getCurrentPage()->GetPageNum()-1)>>1) + 1)) ;
        sName = ";total-pages:";
        sValue += sName;
        sValue += OUString::number(pDrViewSh->GetPageTabControl()->GetPageCount()) ;
        sValue += ";";
        if(pDrViewSh->IsLayerModeActive() )
        {
            sName = "page-name:";
            sValue = sName;
            sDisplay = pDrViewSh->GetLayerTabControl()->GetPageText(pDrViewSh->GetLayerTabControl()->GetCurPageId());
            if( pDoc )
            {
                SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
                SdrLayer* aSdrLayer = rLayerAdmin.GetLayer(sDisplay, false);
                if( aSdrLayer )
                {
                    OUString layerAltText = aSdrLayer->GetTitle();
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
            sValue +=  sDisplay;
            sName = ";page-number:";
            sValue += sName;
            sValue += OUString::number(pDrViewSh->GetActiveTabLayerIndex()+1) ;
            sName = ";total-pages:";
            sValue += sName;
            sValue += OUString::number(pDrViewSh->GetLayerTabControl()->GetPageCount()) ;
            sValue += ";";
        }
    }
    if (mpViewShell && mpViewShell->ISA(::sd::PresentationViewShell))
    {
        ::sd::PresentationViewShell* pPresViewSh = (::sd::PresentationViewShell*) mpViewShell;
        SdPage* pCurrPge = pPresViewSh->getCurrentPage();
        SdDrawDocument* pDoc = pPresViewSh->GetDoc();
        SdPage* pNotesPge = (SdPage*)pDoc->GetSdPage((pCurrPge->GetPageNum()-1)>>1, PK_NOTES);
        if (pNotesPge)
        {
            SdrObject* pNotesObj = pNotesPge->GetPresObj(PRESOBJ_NOTES);
            if (pNotesObj)
            {
                OutlinerParaObject* pPara = pNotesObj->GetOutlinerParaObject();
                if (pPara)
                {
                    sValue += "note:";
                    const EditTextObject& rEdit = pPara->GetTextObject();
                    for (sal_uInt16 i=0;i<rEdit.GetParagraphCount();i++)
                    {
                        OUString strNote = rEdit.GetText(i);
                        strNote = strNote.replaceFirst( "\\", "\\\\" );
                        strNote = strNote.replaceFirst( "=", "\\=" );
                        strNote = strNote.replaceFirst( ";", "\\;" );
                        strNote = strNote.replaceFirst( ",", "\\," );
                        strNote = strNote.replaceFirst( ":", "\\:" );
                        sValue += strNote;
                        sValue += ";";
                    }
                }
            }
        }
    }
    if (mpViewShell && mpViewShell->ISA(::sd::OutlineViewShell) )
    {
        OUString sName;
        OUString sDisplay;
        SdPage* pCurrPge = mpViewShell->GetActualPage();
        SdDrawDocument* pDoc = mpViewShell->GetDoc();
        if(pCurrPge && pDoc)
        {
            sName = "page-name:";
            sDisplay = pCurrPge->GetName();
            sDisplay = sDisplay.replaceFirst( "=", "\\=" );
            sDisplay = sDisplay.replaceFirst( ";", "\\;" );
            sDisplay = sDisplay.replaceFirst( ",", "\\," );
            sDisplay = sDisplay.replaceFirst( ":", "\\:" );
            sValue = sName + sDisplay ;
            sName = ";page-number:";
            sValue += sName;
            sValue += OUString::number((sal_Int16)((sal_uInt16)((pCurrPge->GetPageNum()-1)>>1) + 1)) ;
            sName = ";total-pages:";
            sValue += sName;
            sValue += OUString::number(pDoc->GetSdPageCount(PK_STANDARD)) ;
            sValue += ";";
        }
    }
    if (sValue.getLength())
        anyAtrribute <<= sValue;
    return anyAtrribute;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
        SAL_CALL AccessibleDocumentViewBase::get_AccFlowTo(const ::com::sun::star::uno::Any&, sal_Int32 )
        throw ( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Sequence< uno::Any> aRet;

    return aRet;
}

sal_Int32 SAL_CALL AccessibleDocumentViewBase::getForeground(  )
        throw (uno::RuntimeException)
{
    return COL_BLACK;
}

sal_Int32 SAL_CALL AccessibleDocumentViewBase::getBackground(  )
        throw (uno::RuntimeException)
{
     ThrowIfDisposed ();
    ::osl::MutexGuard aGuard (maMutex);
    return mpViewShell->GetView()->getColorConfig().GetColorValue( ::svtools::DOCCOLOR ).nColor;
}
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
