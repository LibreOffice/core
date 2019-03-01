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

#include <svx/AccessibleShape.hxx>
#include <svx/DescriptionGenerator.hxx>
#include <svx/AccessibleShapeInfo.hxx>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/text/XText.hpp>
#include <editeng/outlobj.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>
#include <editeng/unoedsrc.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <svx/ChildrenManager.hxx>
#include <svx/IAccessibleParent.hxx>
#include <svx/IAccessibleViewForwarder.hxx>
#include <svx/unoshtxt.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdpage.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <svx/ShapeTypeHandler.hxx>
#include <svx/SvxShapeTypes.hxx>

#include <vcl/svapp.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <svx/svdview.hxx>
#include <tools/diagnose_ex.h>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/servicehelper.hxx>
#include "AccessibleEmptyEditSource.hxx"

#include <algorithm>
#include <memory>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::uno::RuntimeException;

namespace accessibility {

namespace {

OUString GetOptionalProperty (
    const Reference<beans::XPropertySet>& rxSet,
    const OUString& rsPropertyName)
{
    OUString sValue;

    if (rxSet.is())
    {
        const Reference<beans::XPropertySetInfo> xInfo (rxSet->getPropertySetInfo());
        if ( ! xInfo.is() || xInfo->hasPropertyByName(rsPropertyName))
        {
            try
            {
                rxSet->getPropertyValue(rsPropertyName) >>= sValue;
            }
            catch (beans::UnknownPropertyException&)
            {
                // This exception should only be thrown when the property
                // does not exits (of course) and the XPropertySetInfo is
                // not available.
            }
        }
    }
    return sValue;
}

} // end of anonymous namespace

// internal
AccessibleShape::AccessibleShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleContextBase (rShapeInfo.mxParent,AccessibleRole::SHAPE),
      mxShape (rShapeInfo.mxShape),
      maShapeTreeInfo (rShapeTreeInfo),
      m_nIndexInParent(-1),
      mpParent (rShapeInfo.mpChildrenManager)
{
    m_pShape = GetSdrObjectFromXShape(mxShape);
    UpdateNameAndDescription();
}

AccessibleShape::~AccessibleShape()
{
    mpChildrenManager.reset();
    mpText.reset();
    SAL_INFO("svx", "~AccessibleShape");

    // Unregistering from the various broadcasters should be unnecessary
    // since this destructor would not have been called if one of the
    // broadcasters would still hold a strong reference to this object.
}

void AccessibleShape::Init()
{
    // Update the OPAQUE and SELECTED shape.
    UpdateStates ();

    // Create a children manager when this shape has children of its own.
    Reference<drawing::XShapes> xShapes (mxShape, uno::UNO_QUERY);
    if (xShapes.is() && xShapes->getCount() > 0)
        mpChildrenManager.reset( new ChildrenManager (
            this, xShapes, maShapeTreeInfo, *this) );
    if (mpChildrenManager != nullptr)
        mpChildrenManager->Update();

    // Register at model as document::XEventListener.
    if (maShapeTreeInfo.GetModelBroadcaster().is())
        maShapeTreeInfo.GetModelBroadcaster()->addEventListener (
            static_cast<document::XEventListener*>(this));

    // Beware! Here we leave the paths of the UNO API and descend into the
    // depths of the core.  Necessary for making the edit engine
    // accessible.
    Reference<text::XText> xText (mxShape, uno::UNO_QUERY);
    if (xText.is())
    {
        SdrView* pView = maShapeTreeInfo.GetSdrView ();
        const vcl::Window* pWindow = maShapeTreeInfo.GetWindow ();
        if (pView != nullptr && pWindow != nullptr && mxShape.is())
        {
            // #107948# Determine whether shape text is empty
            SdrObject* pSdrObject = GetSdrObjectFromXShape(mxShape);
            if( pSdrObject )
            {
                SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( pSdrObject  );
                OutlinerParaObject* pOutlinerParaObject = nullptr;

                if( pTextObj )
                    pOutlinerParaObject = pTextObj->GetEditOutlinerParaObject().release(); // Get the OutlinerParaObject if text edit is active

                bool bOwnParaObj = pOutlinerParaObject != nullptr;

                if (!pOutlinerParaObject)
                    pOutlinerParaObject = pSdrObject->GetOutlinerParaObject();

                // create AccessibleTextHelper to handle this shape's text
                if( !pOutlinerParaObject )
                {
                    // empty text -> use proxy edit source to delay creation of EditEngine
                    mpText.reset( new AccessibleTextHelper( std::make_unique<AccessibleEmptyEditSource >(*pSdrObject, *pView, *pWindow) ) );
                }
                else
                {
                    // non-empty text -> use full-fledged edit source right away
                    mpText.reset( new AccessibleTextHelper( std::make_unique<SvxTextEditSource >(*pSdrObject, nullptr, *pView, *pWindow) ) );
                }
                if( pWindow->HasFocus() )
                    mpText->SetFocus();

                if( bOwnParaObj )
                    delete pOutlinerParaObject;

                mpText->SetEventSource(this);
            }
        }
    }
}


void AccessibleShape::UpdateStates()
{
    ::utl::AccessibleStateSetHelper* pStateSet =
        static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if (pStateSet == nullptr)
        return;

    // Set the opaque state for certain shape types when their fill style is
    // solid.
    bool bShapeIsOpaque = false;
    switch (ShapeTypeHandler::Instance().GetTypeId (mxShape))
    {
        case DRAWING_PAGE:
        case DRAWING_RECTANGLE:
        case DRAWING_TEXT:
        {
            uno::Reference<beans::XPropertySet> xSet (mxShape, uno::UNO_QUERY);
            if (xSet.is())
            {
                try
                {
                    drawing::FillStyle aFillStyle;
                    bShapeIsOpaque =  ( xSet->getPropertyValue ("FillStyle") >>= aFillStyle)
                                        && aFillStyle == drawing::FillStyle_SOLID;
                }
                catch (css::beans::UnknownPropertyException&)
                {
                    // Ignore.
                }
            }
        }
    }
    if (bShapeIsOpaque)
        pStateSet->AddState (AccessibleStateType::OPAQUE);
    else
        pStateSet->RemoveState (AccessibleStateType::OPAQUE);

    // Set the selected state.
    bool bShapeIsSelected = false;
    // XXX fix_me this has to be done with an extra interface later on
    if ( m_pShape && maShapeTreeInfo.GetSdrView() )
    {
        bShapeIsSelected = maShapeTreeInfo.GetSdrView()->IsObjMarked(m_pShape);
    }

    if (bShapeIsSelected)
        pStateSet->AddState (AccessibleStateType::SELECTED);
    else
        pStateSet->RemoveState (AccessibleStateType::SELECTED);
}

OUString AccessibleShape::GetStyle()
{
    return ShapeTypeHandler::CreateAccessibleBaseName( mxShape );
}

bool AccessibleShape::SetState (sal_Int16 aState)
{
    bool bStateHasChanged = false;

    if (aState == AccessibleStateType::FOCUSED && mpText != nullptr)
    {
        // Offer FOCUSED state to edit engine and detect whether the state
        // changes.
        bool bIsFocused = mpText->HaveFocus ();
        mpText->SetFocus();
        bStateHasChanged = (bIsFocused != mpText->HaveFocus ());
    }
    else
        bStateHasChanged = AccessibleContextBase::SetState (aState);

    return bStateHasChanged;
}


bool AccessibleShape::ResetState (sal_Int16 aState)
{
    bool bStateHasChanged = false;

    if (aState == AccessibleStateType::FOCUSED && mpText != nullptr)
    {
        // Try to remove FOCUSED state from the edit engine and detect
        // whether the state changes.
        bool bIsFocused = mpText->HaveFocus ();
        mpText->SetFocus (false);
        bStateHasChanged = (bIsFocused != mpText->HaveFocus ());
    }
    else
        bStateHasChanged = AccessibleContextBase::ResetState (aState);

    return bStateHasChanged;
}


bool AccessibleShape::GetState (sal_Int16 aState)
{
    if (aState == AccessibleStateType::FOCUSED && mpText != nullptr)
    {
        // Just delegate the call to the edit engine.  The state is not
        // merged into the state set.
        return mpText->HaveFocus();
    }
    else
        return AccessibleContextBase::GetState (aState);
}

// OverWrite the parent's getAccessibleName method
OUString SAL_CALL AccessibleShape::getAccessibleName()
{
    ThrowIfDisposed ();
    if (m_pShape && !m_pShape->GetTitle().isEmpty())
        return CreateAccessibleName() + " " + m_pShape->GetTitle();
    else
        return CreateAccessibleName();
}

OUString SAL_CALL AccessibleShape::getAccessibleDescription()
{
    ThrowIfDisposed ();
    if( m_pShape && !m_pShape->GetDescription().isEmpty())
        return m_pShape->GetDescription() ;
    else
        return OUString(" ");
}

// XAccessibleContext
/** The children of this shape come from two sources: The children from
    group or scene shapes and the paragraphs of text.
*/
sal_Int32 SAL_CALL
       AccessibleShape::getAccessibleChildCount ()
{
    if (IsDisposed())
    {
        return 0;
    }

    sal_Int32 nChildCount = 0;

    // Add the number of shapes that are children of this shape.
    if (mpChildrenManager != nullptr)
        nChildCount += mpChildrenManager->GetChildCount ();
    // Add the number text paragraphs.
    if (mpText != nullptr)
        nChildCount += mpText->GetChildCount ();

    return nChildCount;
}


/** Forward the request to the shape.  Return the requested shape or throw
    an exception for a wrong index.
*/
uno::Reference<XAccessible> SAL_CALL
    AccessibleShape::getAccessibleChild (sal_Int32 nIndex)
{
    ThrowIfDisposed ();

    uno::Reference<XAccessible> xChild;

    // Depending on the index decide whether to delegate this call to the
    // children manager or the edit engine.
    if ((mpChildrenManager != nullptr)
        && (nIndex < mpChildrenManager->GetChildCount()))
    {
        xChild = mpChildrenManager->GetChild (nIndex);
    }
    else if (mpText != nullptr)
    {
        sal_Int32 nI = nIndex;
        if (mpChildrenManager != nullptr)
            nI -= mpChildrenManager->GetChildCount();
        xChild = mpText->GetChild (nI);
    }
    else
        throw lang::IndexOutOfBoundsException (
            "shape has no child with index " + OUString::number(nIndex),
            static_cast<uno::XWeak*>(this));

    return xChild;
}

uno::Reference<XAccessibleRelationSet> SAL_CALL
    AccessibleShape::getAccessibleRelationSet()
{
    ::osl::MutexGuard aGuard (maMutex);
    if (mpParent == nullptr)
        return uno::Reference<XAccessibleRelationSet>();

    ::utl::AccessibleRelationSetHelper* pRelationSet = new utl::AccessibleRelationSetHelper;

    //this mxshape is the captioned shape
    uno::Sequence< uno::Reference< uno::XInterface > > aSequence { mpParent->GetAccessibleCaption(mxShape) };
    if(aSequence[0].get())
    {
        pRelationSet->AddRelation(
                                  AccessibleRelation( AccessibleRelationType::DESCRIBED_BY, aSequence ) );
    }
    return uno::Reference<XAccessibleRelationSet>(pRelationSet);
}

/** Return a copy of the state set.
    Possible states are:
        ENABLED
        SHOWING
        VISIBLE
*/
uno::Reference<XAccessibleStateSet> SAL_CALL
    AccessibleShape::getAccessibleStateSet()
{
    ::osl::MutexGuard aGuard (maMutex);

    if (IsDisposed())
    {
        // Return a minimal state set that only contains the DEFUNC state.
        return AccessibleContextBase::getAccessibleStateSet ();
    }

    ::utl::AccessibleStateSetHelper* pStateSet =
          static_cast<::utl::AccessibleStateSetHelper*>(mxStateSet.get());

    if (!pStateSet)
        return Reference<XAccessibleStateSet>();

    // Merge current FOCUSED state from edit engine.
    if (mpText)
    {
        if (mpText->HaveFocus())
            pStateSet->AddState (AccessibleStateType::FOCUSED);
        else
            pStateSet->RemoveState (AccessibleStateType::FOCUSED);
    }
    //Just when the document is not read-only,set states EDITABLE,RESIZABLE,MOVEABLE
    css::uno::Reference<XAccessible> xTempAcc = getAccessibleParent();
    if( xTempAcc.is() )
    {
        css::uno::Reference<XAccessibleContext>
                                xTempAccContext = xTempAcc->getAccessibleContext();
        if( xTempAccContext.is() )
        {
            css::uno::Reference<XAccessibleStateSet> rState =
                xTempAccContext->getAccessibleStateSet();
            if (rState.is())
            {
                css::uno::Sequence<short> aStates = rState->getStates();
                int count = aStates.getLength();
                for( int iIndex = 0;iIndex < count;iIndex++ )
                {
                    if( aStates[iIndex] == AccessibleStateType::EDITABLE )
                    {
                        pStateSet->AddState (AccessibleStateType::EDITABLE);
                        pStateSet->AddState (AccessibleStateType::RESIZABLE);
                        pStateSet->AddState (AccessibleStateType::MOVEABLE);
                        break;
                    }
                }
            }
        }
    }

    // Create a copy of the state set that may be modified by the
    // caller without affecting the current state set.
    Reference<XAccessibleStateSet> xStateSet(new ::utl::AccessibleStateSetHelper(*pStateSet));

    if (mpParent && mpParent->IsDocumentSelAll())
    {
        ::utl::AccessibleStateSetHelper* pCopyStateSet =
            static_cast<::utl::AccessibleStateSetHelper*>(xStateSet.get());
        pCopyStateSet->AddState (AccessibleStateType::SELECTED);
    }

    return xStateSet;
}

// XAccessibleComponent
/** The implementation below is at the moment straightforward.  It iterates
    over all children (and thereby instances all children which have not
    been already instantiated) until a child covering the specified point is
    found.
    This leaves room for improvement.  For instance, first iterate only over
    the already instantiated children and only if no match is found
    instantiate the remaining ones.
*/
uno::Reference<XAccessible > SAL_CALL
    AccessibleShape::getAccessibleAtPoint (
        const awt::Point& aPoint)
{
    ::osl::MutexGuard aGuard (maMutex);

    sal_Int32 nChildCount = getAccessibleChildCount ();
    for (sal_Int32 i=0; i<nChildCount; ++i)
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
                    return xChild;
            }
        }
    }

    // Have not found a child under the given point.  Returning empty
    // reference to indicate this.
    return uno::Reference<XAccessible>();
}


awt::Rectangle SAL_CALL AccessibleShape::getBounds()
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard (maMutex);

    ThrowIfDisposed ();
    awt::Rectangle aBoundingBox;
    if ( mxShape.is() )
    {

        static const char sBoundRectName[] = "BoundRect";
        static const char sAnchorPositionName[] = "AnchorPosition";

        // Get the shape's bounding box in internal coordinates (in 100th of
        // mm).  Use the property BoundRect.  Only if that is not supported ask
        // the shape for its position and size directly.
        Reference<beans::XPropertySet> xSet (mxShape, uno::UNO_QUERY);
        Reference<beans::XPropertySetInfo> xSetInfo;
        bool bFoundBoundRect = false;
        if (xSet.is())
        {
            xSetInfo = xSet->getPropertySetInfo ();
            if (xSetInfo.is())
            {
                if (xSetInfo->hasPropertyByName (sBoundRectName))
                {
                    try
                    {
                        uno::Any aValue = xSet->getPropertyValue (sBoundRectName);
                        aValue >>= aBoundingBox;
                        bFoundBoundRect = true;
                    }
                    catch (beans::UnknownPropertyException const&)
                    {
                        // Handled below (bFoundBoundRect stays false).
                    }
                }
                else
                    SAL_WARN("svx", "no property BoundRect");
            }
        }

        // Fallback when there is no BoundRect Property.
        if ( ! bFoundBoundRect )
        {
            awt::Point aPosition (mxShape->getPosition());
            awt::Size aSize (mxShape->getSize());
            aBoundingBox = awt::Rectangle (
                aPosition.X, aPosition.Y,
                aSize.Width, aSize.Height);

            // While BoundRects have absolute positions, the position returned
            // by XPosition::getPosition is relative.  Get the anchor position
            // (usually not (0,0) for Writer shapes).
            if (xSetInfo.is())
            {
                if (xSetInfo->hasPropertyByName (sAnchorPositionName))
                {
                    uno::Any aPos = xSet->getPropertyValue (sAnchorPositionName);
                    awt::Point aAnchorPosition;
                    aPos >>= aAnchorPosition;
                    aBoundingBox.X += aAnchorPosition.X;
                    aBoundingBox.Y += aAnchorPosition.Y;
                }
            }
        }

        // Transform coordinates from internal to pixel.
        if (maShapeTreeInfo.GetViewForwarder() == nullptr)
            throw uno::RuntimeException (
                "AccessibleShape has no valid view forwarder",
                static_cast<uno::XWeak*>(this));
        ::Size aPixelSize = maShapeTreeInfo.GetViewForwarder()->LogicToPixel (
            ::Size (aBoundingBox.Width, aBoundingBox.Height));
        ::Point aPixelPosition = maShapeTreeInfo.GetViewForwarder()->LogicToPixel (
            ::Point (aBoundingBox.X, aBoundingBox.Y));

        // Clip the shape's bounding box with the bounding box of its parent.
        Reference<XAccessibleComponent> xParentComponent (
            getAccessibleParent(), uno::UNO_QUERY);
        if (xParentComponent.is())
        {
            // Make the coordinates relative to the parent.
            awt::Point aParentLocation (xParentComponent->getLocationOnScreen());
            int x = aPixelPosition.getX() - aParentLocation.X;
            int y = aPixelPosition.getY() - aParentLocation.Y;

            // Clip with parent (with coordinates relative to itself).
            ::tools::Rectangle aBBox (
                x, y, x + aPixelSize.getWidth(), y + aPixelSize.getHeight());
            awt::Size aParentSize (xParentComponent->getSize());
            ::tools::Rectangle aParentBBox (0,0, aParentSize.Width, aParentSize.Height);
            aBBox = aBBox.GetIntersection (aParentBBox);
            aBoundingBox = awt::Rectangle (
                aBBox.getX(),
                aBBox.getY(),
                aBBox.getWidth(),
                aBBox.getHeight());
        }
        else
        {
            SAL_INFO("svx", "parent does not support component");
            aBoundingBox = awt::Rectangle (
                aPixelPosition.getX(), aPixelPosition.getY(),
                aPixelSize.getWidth(), aPixelSize.getHeight());
        }
    }

    return aBoundingBox;
}


awt::Point SAL_CALL AccessibleShape::getLocation()
{
    ThrowIfDisposed ();
    awt::Rectangle aBoundingBox (getBounds());
    return awt::Point (aBoundingBox.X, aBoundingBox.Y);
}


awt::Point SAL_CALL AccessibleShape::getLocationOnScreen()
{
    ThrowIfDisposed ();

    // Get relative position...
    awt::Point aLocation (getLocation ());

    // ... and add absolute position of the parent.
    uno::Reference<XAccessibleComponent> xParentComponent (
        getAccessibleParent(), uno::UNO_QUERY);
    if (xParentComponent.is())
    {
        awt::Point aParentLocation (xParentComponent->getLocationOnScreen());
        aLocation.X += aParentLocation.X;
        aLocation.Y += aParentLocation.Y;
    }
    else
        SAL_WARN("svx", "parent does not support XAccessibleComponent");
    return aLocation;
}


awt::Size SAL_CALL AccessibleShape::getSize()
{
    ThrowIfDisposed ();
    awt::Rectangle aBoundingBox (getBounds());
    return awt::Size (aBoundingBox.Width, aBoundingBox.Height);
}


sal_Int32 SAL_CALL AccessibleShape::getForeground()
{
    ThrowIfDisposed ();
    sal_Int32 nColor (0x0ffffffL);

    try
    {
        uno::Reference<beans::XPropertySet> aSet (mxShape, uno::UNO_QUERY);
        if (aSet.is())
        {
            uno::Any aColor;
            aColor = aSet->getPropertyValue ("LineColor");
            aColor >>= nColor;
        }
    }
    catch (const css::beans::UnknownPropertyException &)
    {
        // Ignore exception and return default color.
    }
    return nColor;
}


sal_Int32 SAL_CALL AccessibleShape::getBackground()
{
    ThrowIfDisposed ();
    Color nColor;

    try
    {
        uno::Reference<beans::XPropertySet> aSet (mxShape, uno::UNO_QUERY);
        if (aSet.is())
        {
            uno::Any aColor;
            aColor = aSet->getPropertyValue ("FillColor");
            aColor >>= nColor;
            aColor = aSet->getPropertyValue ("FillTransparence");
            short nTrans=0;
            aColor >>= nTrans;
            Color crBk(nColor);
            if (nTrans == 0 )
            {
                crBk.SetTransparency(0xff);
            }
            else
            {
                nTrans = short(256 - nTrans / 100. * 256);
                crBk.SetTransparency(sal_uInt8(nTrans));
            }
            nColor = crBk;
        }
    }
    catch (const css::beans::UnknownPropertyException &)
    {
        // Ignore exception and return default color.
    }
    return sal_Int32(nColor);
}

// XAccessibleEventBroadcaster
void SAL_CALL AccessibleShape::addAccessibleEventListener (
    const Reference<XAccessibleEventListener >& rxListener)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        uno::Reference<uno::XInterface> xThis (
            static_cast<lang::XComponent *>(this), uno::UNO_QUERY);
        rxListener->disposing (lang::EventObject (xThis));
    }
    else
    {
        AccessibleContextBase::addAccessibleEventListener (rxListener);
        if (mpText != nullptr)
            mpText->AddEventListener (rxListener);
    }
}


void SAL_CALL AccessibleShape::removeAccessibleEventListener (
    const Reference<XAccessibleEventListener >& rxListener)
{
    AccessibleContextBase::removeAccessibleEventListener (rxListener);
    if (mpText != nullptr)
        mpText->RemoveEventListener (rxListener);
}

// XInterface
css::uno::Any SAL_CALL
    AccessibleShape::queryInterface (const css::uno::Type & rType)
{
    css::uno::Any aReturn = AccessibleContextBase::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast<XAccessibleComponent*>(this),
            static_cast<XAccessibleExtendedComponent*>(this),
            static_cast< css::accessibility::XAccessibleSelection* >(this),
            static_cast< css::accessibility::XAccessibleExtendedAttributes* >(this),
            static_cast<lang::XEventListener*>(this),
            static_cast<document::XEventListener*>(this),
            static_cast<lang::XUnoTunnel*>(this),
            static_cast<XAccessibleGroupPosition*>(this),
            static_cast<XAccessibleHypertext*>(this)
            );
    return aReturn;
}


void SAL_CALL
    AccessibleShape::acquire()
    throw ()
{
    AccessibleContextBase::acquire ();
}


void SAL_CALL
    AccessibleShape::release()
    throw ()
{
    AccessibleContextBase::release ();
}

// XAccessibleSelection
void SAL_CALL AccessibleShape::selectAccessibleChild( sal_Int32 )
{
}


sal_Bool SAL_CALL AccessibleShape::isAccessibleChildSelected( sal_Int32 nChildIndex )
{
    uno::Reference<XAccessible> xAcc = getAccessibleChild( nChildIndex );
    uno::Reference<XAccessibleContext> xContext;
    if( xAcc.is() )
    {
        xContext = xAcc->getAccessibleContext();
    }

    if( xContext.is() )
    {
        if( xContext->getAccessibleRole() == AccessibleRole::PARAGRAPH )
        {
            uno::Reference< css::accessibility::XAccessibleText >
                xText(xAcc, uno::UNO_QUERY);
            if( xText.is() )
            {
                if( xText->getSelectionStart() >= 0 ) return true;
            }
        }
        else if( xContext->getAccessibleRole() == AccessibleRole::SHAPE )
        {
            Reference< XAccessibleStateSet > pRState = xContext->getAccessibleStateSet();
            if( !pRState.is() )
                return false;

            uno::Sequence<short> aStates = pRState->getStates();
            int nCount = aStates.getLength();
            for( int i = 0; i < nCount; i++ )
            {
                if(aStates[i] == AccessibleStateType::SELECTED)
                    return true;
            }
            return false;
        }
    }

    return false;
}


void SAL_CALL AccessibleShape::clearAccessibleSelection(  )
{
}


void SAL_CALL AccessibleShape::selectAllAccessibleChildren(  )
{
}


sal_Int32 SAL_CALL AccessibleShape::getSelectedAccessibleChildCount()
{
    sal_Int32 nCount = 0;
    sal_Int32 TotalCount = getAccessibleChildCount();
    for( sal_Int32 i = 0; i < TotalCount; i++ )
        if( isAccessibleChildSelected(i) ) nCount++;

    return nCount;
}


Reference<XAccessible> SAL_CALL AccessibleShape::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
{
    if ( nSelectedChildIndex > getSelectedAccessibleChildCount() )
        throw IndexOutOfBoundsException();
    sal_Int32 i1, i2;
    for( i1 = 0, i2 = 0; i1 < getAccessibleChildCount(); i1++ )
        if( isAccessibleChildSelected(i1) )
        {
            if( i2 == nSelectedChildIndex )
                return getAccessibleChild( i1 );
            i2++;
        }
    return Reference<XAccessible>();
}


void SAL_CALL AccessibleShape::deselectAccessibleChild( sal_Int32 )
{

}

// XAccessibleExtendedAttributes
uno::Any SAL_CALL AccessibleShape::getExtendedAttributes()
{
    uno::Any strRet;
    OUString style;
    if( getAccessibleRole() != AccessibleRole::SHAPE ) return strRet;
    if( m_pShape )
    {
        style = "style:" + GetStyle();
    }
    style += ";";
    strRet <<= style;
    return strRet;
}

// XServiceInfo
OUString SAL_CALL
    AccessibleShape::getImplementationName()
{
    return OUString("AccessibleShape");
}


uno::Sequence<OUString> SAL_CALL
    AccessibleShape::getSupportedServiceNames()
{
    ThrowIfDisposed ();
    // Get list of supported service names from base class...
    uno::Sequence<OUString> aServiceNames =
        AccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nCount (aServiceNames.getLength());

    // ...and add additional names.
    aServiceNames.realloc (nCount + 1);
    aServiceNames[nCount] = "com.sun.star.drawing.AccessibleShape";

    return aServiceNames;
}

// XTypeProvider
uno::Sequence<uno::Type> SAL_CALL
    AccessibleShape::getTypes()
{
    ThrowIfDisposed ();
    // Get list of types from the context base implementation, ...
    uno::Sequence<uno::Type> aTypeList (AccessibleContextBase::getTypes());
    // ... get list of types from component base implementation, ...
    uno::Sequence<uno::Type> aComponentTypeList (AccessibleComponentBase::getTypes());
    // ... define local types, ...
    const uno::Type aLangEventListenerType =
        cppu::UnoType<lang::XEventListener>::get();
    const uno::Type aDocumentEventListenerType =
        cppu::UnoType<document::XEventListener>::get();
    const uno::Type aUnoTunnelType =
        cppu::UnoType<lang::XUnoTunnel>::get();

    // ... and merge them all into one list.
    sal_Int32   nTypeCount (aTypeList.getLength()),
        nComponentTypeCount (aComponentTypeList.getLength());
    int         i;

    aTypeList.realloc (nTypeCount + nComponentTypeCount + 3);

    for (i=0; i<nComponentTypeCount; i++)
        aTypeList[nTypeCount + i] = aComponentTypeList[i];

    aTypeList[nTypeCount + i++ ] = aLangEventListenerType;
    aTypeList[nTypeCount + i++ ] = aDocumentEventListenerType;
    aTypeList[nTypeCount + i ] = aUnoTunnelType;

    return aTypeList;
}

// lang::XEventListener
/** Disposing calls are accepted only from the model: Just reset the
    reference to the model in the shape tree info.  Otherwise this object
    remains functional.
*/
void SAL_CALL
    AccessibleShape::disposing (const lang::EventObject& aEvent)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard (maMutex);

    try
    {
        if (aEvent.Source ==  maShapeTreeInfo.GetModelBroadcaster())
        {
            // Remove reference to model broadcaster to allow it to pass
            // away.
            maShapeTreeInfo.SetModelBroadcaster(nullptr);
        }

    }
    catch (uno::RuntimeException const&)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("svx", "caught exception while disposing " << exceptionToString(ex));
    }
}

// document::XEventListener
void SAL_CALL
    AccessibleShape::notifyEvent (const document::EventObject& rEventObject)
{
    // First check if the event is for us.
    uno::Reference<drawing::XShape> xShape (
        rEventObject.Source, uno::UNO_QUERY);
    if ( xShape.get() == mxShape.get() )
    {
        if (rEventObject.EventName == "ShapeModified")
        {
            //Need to update text children when receiving ShapeModified hint when exiting edit mode for text box
            if (mpText)
                mpText->UpdateChildren();


            // Some property of a shape has been modified.  Send an event
            // that indicates a change of the visible data to all listeners.
            CommitChange (
                AccessibleEventId::VISIBLE_DATA_CHANGED,
                uno::Any(),
                uno::Any());

            // Name and Description may have changed.  Update the local
            // values accordingly.
            UpdateNameAndDescription();
        }
    }
}

// lang::XUnoTunnel
namespace
{
    class theAccessibleShapeImplementationId : public rtl::Static< UnoTunnelIdInit, theAccessibleShapeImplementationId > {};
}

const uno::Sequence< sal_Int8 >&
    AccessibleShape::getUnoTunnelImplementationId()
    throw()
{
    return theAccessibleShapeImplementationId::get().getSeq();
}


AccessibleShape*
    AccessibleShape::getImplementation( const uno::Reference< uno::XInterface >& rxIFace )
    throw()
{
    uno::Reference< lang::XUnoTunnel >  xTunnel( rxIFace, uno::UNO_QUERY );
    AccessibleShape*                    pReturn = nullptr;

    if( xTunnel.is() )
        pReturn = reinterpret_cast< AccessibleShape* >( xTunnel->getSomething( getUnoTunnelImplementationId() ) );

    return pReturn;
}


sal_Int64 SAL_CALL
    AccessibleShape::getSomething( const uno::Sequence< sal_Int8 >& rIdentifier )
{
    sal_Int64 nReturn( 0 );

    if( ( rIdentifier.getLength() == 16 ) && ( 0 == memcmp( getUnoTunnelImplementationId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) )
        nReturn = reinterpret_cast< sal_Int64 >( this );

    return nReturn;
}

// IAccessibleViewForwarderListener
void AccessibleShape::ViewForwarderChanged()
{
    // Inform all listeners that the graphical representation (i.e. size
    // and/or position) of the shape has changed.
    CommitChange (AccessibleEventId::VISIBLE_DATA_CHANGED,
        uno::Any(),
        uno::Any());

    // Tell children manager of the modified view forwarder.
    if (mpChildrenManager != nullptr)
        mpChildrenManager->ViewForwarderChanged();

    // update our children that our screen position might have changed
    if( mpText )
        mpText->UpdateChildren();
}

// protected internal
// Set this object's name if is different to the current name.
OUString AccessibleShape::CreateAccessibleBaseName()
{
    return ShapeTypeHandler::CreateAccessibleBaseName( mxShape );
}


OUString AccessibleShape::CreateAccessibleName()
{
    return GetFullAccessibleName(this);
}

OUString AccessibleShape::GetFullAccessibleName (AccessibleShape *shape)
{
    OUString sName (shape->CreateAccessibleBaseName());
    // Append the shape's index to the name to disambiguate between shapes
    // of the same type.  If such an index where not given to the
    // constructor then use the z-order instead.  If even that does not exist
    // we throw an exception.
    OUString nameStr;
    if (shape->m_pShape)
        nameStr = shape->m_pShape->GetName();
    if (nameStr.isEmpty())
    {
        sName += " ";
    }
    else
    {
        sName = nameStr;
    }

    //If the new produced name if not the same with last,notify name changed
    //Event
    if (aAccName != sName && !aAccName.isEmpty())
    {
        uno::Any aOldValue, aNewValue;
        aOldValue <<= aAccName;
        aNewValue <<= sName;
        CommitChange(
            AccessibleEventId::NAME_CHANGED,
            aNewValue,
            aOldValue);
    }
    aAccName = sName;
    return sName;
}

OUString
    AccessibleShape::CreateAccessibleDescription()
{
    DescriptionGenerator aDG (mxShape);
    aDG.Initialize (CreateAccessibleBaseName());
    switch (ShapeTypeHandler::Instance().GetTypeId (mxShape))
    {
        case DRAWING_3D_CUBE:
        case DRAWING_3D_EXTRUDE:
        case DRAWING_3D_LATHE:
        case DRAWING_3D_SPHERE:
            aDG.Add3DProperties ();
            break;

        case DRAWING_3D_SCENE:
        case DRAWING_GROUP:
        case DRAWING_PAGE:
            // No further information is appended.
            break;

        case DRAWING_CAPTION:
        case DRAWING_CLOSED_BEZIER:
        case DRAWING_CLOSED_FREEHAND:
        case DRAWING_ELLIPSE:
        case DRAWING_POLY_POLYGON:
        case DRAWING_POLY_POLYGON_PATH:
        case DRAWING_RECTANGLE:
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;

        case DRAWING_CONNECTOR:
        case DRAWING_LINE:
        case DRAWING_MEASURE:
        case DRAWING_OPEN_BEZIER:
        case DRAWING_OPEN_FREEHAND:
        case DRAWING_POLY_LINE:
        case DRAWING_POLY_LINE_PATH:
            aDG.AddLineProperties ();
            break;

        case DRAWING_CONTROL:
            aDG.AddProperty ("ControlBackground", DescriptionGenerator::PropertyType::Color, "");
            aDG.AddProperty ("ControlBorder", DescriptionGenerator::PropertyType::Integer, "");
            break;

        case DRAWING_TEXT:
            aDG.AddTextProperties ();
            break;

        default:
            aDG.Initialize ("Unknown accessible shape");
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString ("service name=");
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}


// protected
void AccessibleShape::disposing()
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard (maMutex);

    // Make sure to send an event that this object loses the focus in the
    // case that it has the focus.
    ::utl::AccessibleStateSetHelper* pStateSet =
          static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if (pStateSet != nullptr)
        pStateSet->RemoveState (AccessibleStateType::FOCUSED);

    // Unregister from broadcasters.
    Reference<lang::XComponent> xComponent (mxShape, uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener (this);

    // Unregister from model.
    if (maShapeTreeInfo.GetModelBroadcaster().is())
        maShapeTreeInfo.GetModelBroadcaster()->removeEventListener (
            static_cast<document::XEventListener*>(this));

    // Release the child containers.
    if (mpChildrenManager != nullptr)
    {
        mpChildrenManager.reset();
    }
    if (mpText != nullptr)
    {
        mpText->Dispose();
        mpText.reset();
    }

    // Cleanup.  Remove references to objects to allow them to be
    // destroyed.
    mxShape = nullptr;
    maShapeTreeInfo.dispose();

    // Call base classes.
    AccessibleContextBase::dispose ();
}

sal_Int32 SAL_CALL
       AccessibleShape::getAccessibleIndexInParent()
{
    ThrowIfDisposed ();
    //  Use a simple but slow solution for now.  Optimize later.

    sal_Int32 nIndex = m_nIndexInParent;
    if ( -1 == nIndex )
        nIndex = AccessibleContextBase::getAccessibleIndexInParent();
    return nIndex;
}


void AccessibleShape::UpdateNameAndDescription()
{
    // Ignore missing title, name, or description.  There are fallbacks for
    // them.
    try
    {
        Reference<beans::XPropertySet> xSet (mxShape, uno::UNO_QUERY_THROW);
        OUString sString;

        // Get the accessible name.
        sString = GetOptionalProperty(xSet, "Title");
        if (!sString.isEmpty())
        {
            SetAccessibleName(sString, AccessibleContextBase::FromShape);
        }
        else
        {
            sString = GetOptionalProperty(xSet, "Name");
            if (!sString.isEmpty())
                SetAccessibleName(sString, AccessibleContextBase::FromShape);
        }

        // Get the accessible description.
        sString = GetOptionalProperty(xSet, "Description");
        if (!sString.isEmpty())
            SetAccessibleDescription(sString, AccessibleContextBase::FromShape);
    }
    catch (uno::RuntimeException&)
    {
    }
}

//  Return this object's role.
sal_Int16 SAL_CALL AccessibleShape::getAccessibleRole()
{
    sal_Int16 nAccessibleRole =  AccessibleRole::SHAPE ;
    switch (ShapeTypeHandler::Instance().GetTypeId (mxShape))
    {
        case     DRAWING_GRAPHIC_OBJECT:
                 nAccessibleRole =  AccessibleRole::GRAPHIC ;               break;
        case     DRAWING_OLE:
                 nAccessibleRole =  AccessibleRole::EMBEDDED_OBJECT ;       break;

        default:
            nAccessibleRole = AccessibleContextBase::getAccessibleRole();
            break;
    }

    return nAccessibleRole;
}


//sort the drawing objects from up to down, from left to right
struct XShapePosCompareHelper
{
    bool operator() ( const uno::Reference<drawing::XShape>& xshape1,
        const uno::Reference<drawing::XShape>& xshape2 ) const
    {
        SdrObject* pObj1 = GetSdrObjectFromXShape(xshape1);
        SdrObject* pObj2 = GetSdrObjectFromXShape(xshape2);
        if(pObj1 && pObj2)
            return pObj1->GetOrdNum() < pObj2->GetOrdNum();
        else
            return false;
    }
};
//end of group position

// XAccessibleGroupPosition
uno::Sequence< sal_Int32 > SAL_CALL
AccessibleShape::getGroupPosition( const uno::Any& )
{
    // we will return the:
    // [0] group level
    // [1] similar items counts in the group
    // [2] the position of the object in the group
    uno::Sequence< sal_Int32 > aRet( 3 );
    aRet[0] = 0;
    aRet[1] = 0;
    aRet[2] = 0;

    css::uno::Reference<XAccessible> xParent = getAccessibleParent();
    if (!xParent.is())
    {
        return aRet;
    }
    SdrObject *pObj = GetSdrObjectFromXShape(mxShape);


    if(pObj == nullptr )
    {
        return aRet;
    }

    // Compute object's group level.
    sal_Int32 nGroupLevel = 0;
    SdrObject * pUper = pObj->getParentSdrObjectFromSdrObject();
    while( pUper )
    {
        ++nGroupLevel;
        pUper = pUper->getParentSdrObjectFromSdrObject();
    }

    css::uno::Reference<XAccessibleContext> xParentContext = xParent->getAccessibleContext();
    if( xParentContext->getAccessibleRole()  == AccessibleRole::DOCUMENT ||
            xParentContext->getAccessibleRole()  == AccessibleRole::DOCUMENT_PRESENTATION ||
            xParentContext->getAccessibleRole()  == AccessibleRole::DOCUMENT_SPREADSHEET ||
            xParentContext->getAccessibleRole()  == AccessibleRole::DOCUMENT_TEXT )//Document
    {
        Reference< XAccessibleGroupPosition > xGroupPosition( xParent,uno::UNO_QUERY );
        if ( xGroupPosition.is() )
        {
            aRet = xGroupPosition->getGroupPosition( uno::makeAny( getAccessibleContext() ) );
        }
        return aRet;
    }
    if (xParentContext->getAccessibleRole() != AccessibleRole::SHAPE)
    {
        return aRet;
    }

    SdrObjList *pGrpList = nullptr;
    if( pObj->getParentSdrObjectFromSdrObject() )
        pGrpList = pObj->getParentSdrObjectFromSdrObject()->GetSubList();
    else
        return aRet;

    std::vector< uno::Reference<drawing::XShape> > vXShapes;
    if (pGrpList)
    {
        const size_t nObj = pGrpList->GetObjCount();
        for(size_t i = 0 ; i < nObj ; ++i)
        {
            SdrObject *pSubObj = pGrpList->GetObj(i);
            if (pSubObj &&
                xParentContext->getAccessibleChild(i)->getAccessibleContext()->getAccessibleRole() != AccessibleRole::GROUP_BOX)
            {
                vXShapes.push_back( GetXShapeForSdrObject(pSubObj) );
            }
        }
    }

    std::sort( vXShapes.begin(), vXShapes.end(), XShapePosCompareHelper() );

    //get the index of the selected object in the group
    //we start counting position from 1
    sal_Int32 nPos = 1;
    for ( const auto& rpShape : vXShapes )
    {
        if ( rpShape.get() == mxShape.get() )
        {
            sal_Int32* pArray = aRet.getArray();
            pArray[0] = nGroupLevel;
            pArray[1] = vXShapes.size();
            pArray[2] = nPos;
            break;
        }
        nPos++;
    }

    return aRet;
}

OUString AccessibleShape::getObjectLink( const uno::Any& )
{
    OUString aRet;

    SdrObject *pObj = GetSdrObjectFromXShape(mxShape);
    if(pObj == nullptr )
    {
        return aRet;
    }
    if (maShapeTreeInfo.GetDocumentWindow().is())
    {
        Reference< XAccessibleGroupPosition > xGroupPosition( maShapeTreeInfo.GetDocumentWindow(), uno::UNO_QUERY );
        if (xGroupPosition.is())
        {
            aRet = xGroupPosition->getObjectLink( uno::makeAny( getAccessibleContext() ) );
        }
    }
    return aRet;
}

// XAccessibleHypertext
sal_Int32 SAL_CALL AccessibleShape::getHyperLinkCount()
{
    // MT: Introduced with IA2 CWS, but SvxAccessibleHyperlink was redundant to svx::AccessibleHyperlink which we introduced meanwhile.
    // Code need to be adapted....
    return 0;

    /*
    SvxAccessibleHyperlink* pLink = new SvxAccessibleHyperlink(m_pShape,this);
    if (pLink->IsValidHyperlink())
        return 1;
    else
        return 0;
    */
}
uno::Reference< XAccessibleHyperlink > SAL_CALL
    AccessibleShape::getHyperLink( sal_Int32 )
{
    uno::Reference< XAccessibleHyperlink > xRet;
    // MT: Introduced with IA2 CWS, but SvxAccessibleHyperlink was redundant to svx::AccessibleHyperlink which we introduced meanwhile.
    // Code need to be adapted....
    /*
    SvxAccessibleHyperlink* pLink = new SvxAccessibleHyperlink(m_pShape,this);
    if (pLink->IsValidHyperlink())
        xRet = pLink;
    if( !xRet.is() )
        throw css::lang::IndexOutOfBoundsException();
    */
    return xRet;
}
sal_Int32 SAL_CALL AccessibleShape::getHyperLinkIndex( sal_Int32 )
{
    return 0;
}
// XAccessibleText
sal_Int32 SAL_CALL AccessibleShape::getCaretPosition(  ){return 0;}
sal_Bool SAL_CALL AccessibleShape::setCaretPosition( sal_Int32 ){return false;}
sal_Unicode SAL_CALL AccessibleShape::getCharacter( sal_Int32 ){return 0;}
css::uno::Sequence< css::beans::PropertyValue > SAL_CALL AccessibleShape::getCharacterAttributes( sal_Int32, const css::uno::Sequence< OUString >& )
{
    uno::Sequence< css::beans::PropertyValue > aValues(0);
    return aValues;
}
css::awt::Rectangle SAL_CALL AccessibleShape::getCharacterBounds( sal_Int32 )
{
    return css::awt::Rectangle(0, 0, 0, 0 );
}
sal_Int32 SAL_CALL AccessibleShape::getCharacterCount(  ){return 0;}
sal_Int32 SAL_CALL AccessibleShape::getIndexAtPoint( const css::awt::Point& ){return 0;}
OUString SAL_CALL AccessibleShape::getSelectedText(  ){return OUString();}
sal_Int32 SAL_CALL AccessibleShape::getSelectionStart(  ){return 0;}
sal_Int32 SAL_CALL AccessibleShape::getSelectionEnd(  ){return 0;}
sal_Bool SAL_CALL AccessibleShape::setSelection( sal_Int32, sal_Int32 ){return true;}
OUString SAL_CALL AccessibleShape::getText(  ){return OUString();}
OUString SAL_CALL AccessibleShape::getTextRange( sal_Int32, sal_Int32 ){return OUString();}
css::accessibility::TextSegment SAL_CALL AccessibleShape::getTextAtIndex( sal_Int32, sal_Int16 )
{
    css::accessibility::TextSegment aResult;
    return aResult;
}
css::accessibility::TextSegment SAL_CALL AccessibleShape::getTextBeforeIndex( sal_Int32, sal_Int16 )
{
    css::accessibility::TextSegment aResult;
    return aResult;
}
css::accessibility::TextSegment SAL_CALL AccessibleShape::getTextBehindIndex( sal_Int32, sal_Int16 )
{
    css::accessibility::TextSegment aResult;
    return aResult;
}
sal_Bool SAL_CALL AccessibleShape::copyText( sal_Int32, sal_Int32 ){return true;}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
