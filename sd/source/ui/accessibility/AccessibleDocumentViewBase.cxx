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
#include <com/sun/star/document/XShapeEventBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <sfx2/objsh.hxx>
#include <tools/debug.hxx>

#include <cppuhelper/queryinterface.hxx>
#include <svx/svdobj.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <Window.hxx>
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
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {

AccessibleDocumentViewBase::AccessibleDocumentViewBase (
    ::sd::Window* pSdWindow,
    ::sd::ViewShell* pViewShell,
    uno::Reference<frame::XController> xController,
    const uno::Reference<XAccessible>& rxParent)
    : AccessibleContextBase (rxParent,
                             pViewShell->GetDoc()->GetDocumentType() == DocumentType::Impress ?
                                     AccessibleRole::DOCUMENT_PRESENTATION :
                                     AccessibleRole::DOCUMENT),
      mxController (std::move(xController)),
      maViewForwarder (
        static_cast<SdrPaintView*>(pViewShell->GetView()),
        *pSdWindow->GetOutDev())
{
    if (mxController.is())
        mxModel = mxController->getModel();

    // Fill the shape tree info.
    maShapeTreeInfo.SetModelBroadcaster (
        uno::Reference<document::XShapeEventBroadcaster>(
            mxModel, uno::UNO_QUERY_THROW));
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
        xShapeList = xView->getCurrentPage();

    // Register this object as dispose event listener at the model.
    if (mxModel.is())
        mxModel->addEventListener (
            static_cast<awt::XWindowListener*>(this));

    // Register as property change listener at the controller.
    uno::Reference<beans::XPropertySet> xSet (mxController, uno::UNO_QUERY);
    if (xSet.is())
        xSet->addPropertyChangeListener (
            u""_ustr,
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
                // Window is dying. Unregister from VCL Window.
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
    ensureAlive();

    return AccessibleContextBase::getAccessibleParent();
}

sal_Int64 SAL_CALL
    AccessibleDocumentViewBase::getAccessibleChildCount()
{
    ensureAlive ();

    if (mxAccessibleOLEObject.is())
        return 1;
    else
        return 0;
}

Reference<XAccessible> SAL_CALL
    AccessibleDocumentViewBase::getAccessibleChild (sal_Int64 nIndex)
{
    ensureAlive();

    ::osl::MutexGuard aGuard (m_aMutex);
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
    ensureAlive();

    ::osl::MutexGuard aGuard (m_aMutex);
    uno::Reference<XAccessible> xChildAtPosition;

    sal_Int64 nChildCount = getAccessibleChildCount ();
    for (sal_Int64 i=nChildCount-1; i>=0; --i)
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
                    xChildAtPosition = std::move(xChild);
                    break;
                }
            }
        }
    }

    // Have not found a child under the given point.  Returning empty
    // reference to indicate this.
    return xChildAtPosition;
}

awt::Rectangle AccessibleDocumentViewBase::implGetBounds()
{
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
            );
    return aReturn;
}

void SAL_CALL
    AccessibleDocumentViewBase::acquire()
    noexcept
{
    AccessibleContextBase::acquire ();
}

void SAL_CALL
    AccessibleDocumentViewBase::release()
    noexcept
{
    AccessibleContextBase::release ();
}

// XServiceInfo

OUString SAL_CALL
    AccessibleDocumentViewBase::getImplementationName()
{
    return u"AccessibleDocumentViewBase"_ustr;
}

css::uno::Sequence< OUString> SAL_CALL
    AccessibleDocumentViewBase::getSupportedServiceNames()
{
    ensureAlive();
    return AccessibleContextBase::getSupportedServiceNames ();
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

    // Unregister from the model.
    if (mxModel.is())
        mxModel->removeEventListener (
            static_cast<awt::XWindowListener*>(this));

    // Unregister from the controller.
    if (mxController.is())
    {
        uno::Reference<beans::XPropertySet> xSet (mxController, uno::UNO_QUERY);
        if (xSet.is())
            xSet->removePropertyChangeListener (u""_ustr, static_cast<beans::XPropertyChangeListener*>(this));

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
    maShapeTreeInfo.dispose();
    mxAccessibleOLEObject.clear();
}

//=====  XEventListener  ======================================================

void SAL_CALL
    AccessibleDocumentViewBase::disposing (const lang::EventObject& rEventObject)
{
    ensureAlive();

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
    if (!isAlive())
        return;

    ViewForwarderChanged();
}

void SAL_CALL
    AccessibleDocumentViewBase::windowMoved (const css::awt::WindowEvent& )
{
    if (!isAlive())
        return;

    ViewForwarderChanged();
}

void SAL_CALL
    AccessibleDocumentViewBase::windowShown (const css::lang::EventObject& )
{
    if (!isAlive())
        return;

    ViewForwarderChanged();
}

void SAL_CALL
    AccessibleDocumentViewBase::windowHidden (const css::lang::EventObject& )
{
    if (!isAlive())
        return;

    ViewForwarderChanged();
}

//=====  XFocusListener  ==================================================

void AccessibleDocumentViewBase::focusGained (const css::awt::FocusEvent& e)
{
    ensureAlive();
    if (e.Source == mxWindow)
        Activated ();
}

void AccessibleDocumentViewBase::focusLost (const css::awt::FocusEvent& e)
{
    ensureAlive();
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
    return u"AccessibleDocumentViewBase"_ustr;
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
                uno::Any (mxAccessibleOLEObject), -1);

    // Assume that the accessible OLE Object disposes itself correctly.

    {
        ::osl::MutexGuard aGuard (m_aMutex);
        mxAccessibleOLEObject = xOLEObject;
    }

    // Send child event about new accessible OLE object if necessary.
    if (mxAccessibleOLEObject.is())
        CommitChange (
            AccessibleEventId::CHILD,
            uno::Any (mxAccessibleOLEObject),
            uno::Any(), -1);
}

//=====  methods from AccessibleSelectionBase ==================================================

// return the member maMutex;
::osl::Mutex&
    AccessibleDocumentViewBase::implGetMutex()
{
    return m_aMutex;
}

// return ourself as context in default case
uno::Reference< XAccessibleContext >
    AccessibleDocumentViewBase::implGetAccessibleContext()
{
    return this;
}

// return false in default case
bool
    AccessibleDocumentViewBase::implIsSelected( sal_Int64 )
{
    return false;
}

// do nothing in default case
void
    AccessibleDocumentViewBase::implSelect( sal_Int64, bool )
{
}

OUString SAL_CALL AccessibleDocumentViewBase::getExtendedAttributes()
{
    ::osl::MutexGuard aGuard (m_aMutex);

    OUStringBuffer sValue;
    if (auto pDrViewSh = dynamic_cast<::sd::DrawViewShell* > (mpViewShell))
    {
        OUString sDisplay;
        OUString sName = u"page-name:"_ustr;
        // MT IA2: Not used...
        // SdPage*  pCurrPge = pDrViewSh->getCurrentPage();
        SdDrawDocument* pDoc = pDrViewSh->GetDoc();
        sDisplay = pDrViewSh->getCurrentPage()->GetName();
        sDisplay = sDisplay.replaceFirst( "\\", "\\\\" );
        sDisplay = sDisplay.replaceFirst( "=", "\\=" );
        sDisplay = sDisplay.replaceFirst( ";", "\\;" );
        sDisplay = sDisplay.replaceFirst( ",", "\\," );
        sDisplay = sDisplay.replaceFirst( ":", "\\:" );
        sValue = sName + sDisplay
            + ";page-number:"
            + OUString::number(static_cast<sal_Int32>(static_cast<sal_uInt16>((pDrViewSh->getCurrentPage()->GetPageNum()-1)>>1) + 1))
            + ";total-pages:"
            + OUString::number(static_cast<sal_Int32>(pDrViewSh->GetPageTabControl().GetPageCount()))
            + ";";
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
                        sDisplay += sName + layerAltText;
                    }
                }
            }
            sDisplay = sDisplay.replaceFirst( "\\", "\\\\" );
            sDisplay = sDisplay.replaceFirst( "=", "\\=" );
            sDisplay = sDisplay.replaceFirst( ";", "\\;" );
            sDisplay = sDisplay.replaceFirst( ",", "\\," );
            sDisplay = sDisplay.replaceFirst( ":", "\\:" );
            sValue.append(sDisplay
                + ";page-number:"
                + OUString::number(static_cast<sal_Int32>(pDrViewSh->GetActiveTabLayerIndex()+1))
                + ";total-pages:"
                + OUString::number(static_cast<sal_Int32>(pDrViewSh->GetLayerTabControl()->GetPageCount()))
                + ";");
        }
    }
    if (auto pPresViewSh = dynamic_cast<::sd::PresentationViewShell* >(mpViewShell))
    {
        SdPage* pCurrPge = pPresViewSh->getCurrentPage();
        SdDrawDocument* pDoc = pPresViewSh->GetDoc();
        SdPage* pNotesPge = pDoc->GetSdPage((pCurrPge->GetPageNum()-1)>>1, PageKind::Notes);
        if (pNotesPge)
        {
            SdrObject* pNotesObj = pNotesPge->GetPresObj(PresObjKind::Notes);
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
                        sValue.append(strNote
                            + ";");//to divide each paragraph
                    }
                }
            }
        }
    }
    if (dynamic_cast<const ::sd::OutlineViewShell* >(mpViewShell ) !=  nullptr )
    {
        SdPage* pCurrPge = mpViewShell->GetActualPage();
        SdDrawDocument* pDoc = mpViewShell->GetDoc();
        if(pCurrPge && pDoc)
        {
            OUString sDisplay;
            sDisplay = pCurrPge->GetName();
            sDisplay = sDisplay.replaceFirst( "=", "\\=" );
            sDisplay = sDisplay.replaceFirst( ";", "\\;" );
            sDisplay = sDisplay.replaceFirst( ",", "\\," );
            sDisplay = sDisplay.replaceFirst( ":", "\\:" );
            sValue = "page-name:" + sDisplay
                + ";page-number:"
                + OUString::number(static_cast<sal_Int32>(static_cast<sal_uInt16>((pCurrPge->GetPageNum()-1)>>1) + 1))
                + ";total-pages:"
                + OUString::number(static_cast<sal_Int32>(pDoc->GetSdPageCount(PageKind::Standard)))
                + ";";
        }
    }

    return sValue.makeStringAndClear();
}

sal_Int32 SAL_CALL AccessibleDocumentViewBase::getForeground(  )
{
    return sal_Int32(COL_BLACK);
}

sal_Int32 SAL_CALL AccessibleDocumentViewBase::getBackground(  )
{
    ensureAlive();
    ::osl::MutexGuard aGuard (m_aMutex);
    return sal_Int32(mpViewShell->GetView()->getColorConfig().GetColorValue( ::svtools::DOCCOLOR ).nColor);
}
} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
