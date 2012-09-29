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

#include <svx/AccessibleShape.hxx>
#include "svx/DescriptionGenerator.hxx"
#include <svx/AccessibleShapeInfo.hxx>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/text/XText.hpp>
#include <editeng/outlobj.hxx>
#include <rtl/ref.hxx>
#include <editeng/unoedsrc.hxx>
#include <svx/unoshtxt.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include "svx/unoapi.hxx"
#include <com/sun/star/uno/Exception.hpp>
#include <svx/ShapeTypeHandler.hxx>
#include <svx/SvxShapeTypes.hxx>

#include "accessibility.hrc"
#include "svx/svdstr.hrc"
#include <svx/dialmgr.hxx>
#include <vcl/svapp.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <svx/svdview.hxx>
#include <comphelper/servicehelper.hxx>
#include "AccessibleEmptyEditSource.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;
using ::rtl::OUString;

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




//=====  internal  ============================================================

AccessibleShape::AccessibleShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleContextBase (rShapeInfo.mxParent,AccessibleRole::LIST_ITEM),
      mpChildrenManager(NULL),
      mxShape (rShapeInfo.mxShape),
      maShapeTreeInfo (rShapeTreeInfo),
      mnIndex (rShapeInfo.mnIndex),
      m_nIndexInParent(-1),
      mpText (NULL),
      mpParent (rShapeInfo.mpChildrenManager)
{
    m_pShape = GetSdrObjectFromXShape(mxShape);
    UpdateNameAndDescription();
}




AccessibleShape::~AccessibleShape (void)
{
    if (mpChildrenManager != NULL)
        delete mpChildrenManager;
    if (mpText != NULL)
        delete mpText;
    OSL_TRACE ("~AccessibleShape");

    // Unregistering from the various broadcasters should be unnecessary
    // since this destructor would not have been called if one of the
    // broadcasters would still hold a strong reference to this object.
}




void AccessibleShape::Init (void)
{
    // Update the OPAQUE and SELECTED shape.
    UpdateStates ();

    // Create a children manager when this shape has children of its own.
    Reference<drawing::XShapes> xShapes (mxShape, uno::UNO_QUERY);
    if (xShapes.is() && xShapes->getCount() > 0)
        mpChildrenManager = new ChildrenManager (
            this, xShapes, maShapeTreeInfo, *this);
    if (mpChildrenManager != NULL)
        mpChildrenManager->Update();

    // Register at model as document::XEventListener.
    if (maShapeTreeInfo.GetModelBroadcaster().is())
        maShapeTreeInfo.GetModelBroadcaster()->addEventListener (
            static_cast<document::XEventListener*>(this));

    // Beware! Here we leave the paths of the UNO API and descend into the
    // depths of the core.  Necessary for makeing the edit engine
    // accessible.
    Reference<text::XText> xText (mxShape, uno::UNO_QUERY);
    if (xText.is())
    {
        SdrView* pView = maShapeTreeInfo.GetSdrView ();
        const Window* pWindow = maShapeTreeInfo.GetWindow ();
        if (pView != NULL && pWindow != NULL && mxShape.is())
        {
            // #107948# Determine whether shape text is empty
            SdrObject* pSdrObject = GetSdrObjectFromXShape(mxShape);
            if( pSdrObject )
            {
                SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, pSdrObject );
                OutlinerParaObject* pOutlinerParaObject = NULL;

                if( pTextObj )
                    pOutlinerParaObject = pTextObj->GetEditOutlinerParaObject(); // Get the OutlinerParaObject if text edit is active

                bool bOwnParaObj = pOutlinerParaObject != NULL;

                if( !pOutlinerParaObject && pSdrObject )
                    pOutlinerParaObject = pSdrObject->GetOutlinerParaObject();

                // create AccessibleTextHelper to handle this shape's text
                if( !pOutlinerParaObject )
                {
                    // empty text -> use proxy edit source to delay creation of EditEngine
                    SAL_WNODEPRECATED_DECLARATIONS_PUSH
                    ::std::auto_ptr<SvxEditSource> pEditSource( new AccessibleEmptyEditSource ( *pSdrObject, *pView, *pWindow) );
                    SAL_WNODEPRECATED_DECLARATIONS_POP
                    mpText = new AccessibleTextHelper( pEditSource );
                }
                else
                {
                    // non-empty text -> use full-fledged edit source right away
                    SAL_WNODEPRECATED_DECLARATIONS_PUSH
                    ::std::auto_ptr<SvxEditSource> pEditSource( new SvxTextEditSource ( *pSdrObject, 0, *pView, *pWindow) );
                    SAL_WNODEPRECATED_DECLARATIONS_POP
                    mpText = new AccessibleTextHelper( pEditSource );
                }

                if( bOwnParaObj )
                    delete pOutlinerParaObject;

                mpText->SetEventSource(this);
            }
        }
    }
}




void AccessibleShape::UpdateStates (void)
{
    ::utl::AccessibleStateSetHelper* pStateSet =
        static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if (pStateSet == NULL)
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
                    bShapeIsOpaque =  ( xSet->getPropertyValue (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FillStyle"))) >>= aFillStyle)
                                        && aFillStyle == drawing::FillStyle_SOLID;
                }
                catch (::com::sun::star::beans::UnknownPropertyException&)
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
        bShapeIsSelected = maShapeTreeInfo.GetSdrView()->IsObjMarked(m_pShape) == sal_True;
    }

    if (bShapeIsSelected)
        pStateSet->AddState (AccessibleStateType::SELECTED);
    else
        pStateSet->RemoveState (AccessibleStateType::SELECTED);
}




bool AccessibleShape::operator== (const AccessibleShape& rShape)
{
    return this==&rShape;
}




sal_Bool AccessibleShape::SetState (sal_Int16 aState)
{
    sal_Bool bStateHasChanged = sal_False;

    if (aState == AccessibleStateType::FOCUSED && mpText != NULL)
    {
        // Offer FOCUSED state to edit engine and detect whether the state
        // changes.
        sal_Bool bIsFocused = mpText->HaveFocus ();
        mpText->SetFocus (sal_True);
        bStateHasChanged = (bIsFocused != mpText->HaveFocus ());
    }
    else
        bStateHasChanged = AccessibleContextBase::SetState (aState);

    return bStateHasChanged;
}




sal_Bool AccessibleShape::ResetState (sal_Int16 aState)
{
    sal_Bool bStateHasChanged = sal_False;

    if (aState == AccessibleStateType::FOCUSED && mpText != NULL)
    {
        // Try to remove FOCUSED state from the edit engine and detect
        // whether the state changes.
        sal_Bool bIsFocused = mpText->HaveFocus ();
        mpText->SetFocus (sal_False);
        bStateHasChanged = (bIsFocused != mpText->HaveFocus ());
    }
    else
        bStateHasChanged = AccessibleContextBase::ResetState (aState);

    return bStateHasChanged;
}




sal_Bool AccessibleShape::GetState (sal_Int16 aState)
{
    if (aState == AccessibleStateType::FOCUSED && mpText != NULL)
    {
        // Just delegate the call to the edit engine.  The state is not
        // merged into the state set.
        return mpText->HaveFocus();
    }
    else
        return AccessibleContextBase::GetState (aState);
}




//=====  XAccessibleContext  ==================================================

/** The children of this shape come from two sources: The children from
    group or scene shapes and the paragraphs of text.
*/
sal_Int32 SAL_CALL
       AccessibleShape::getAccessibleChildCount ()
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    sal_Int32 nChildCount = 0;

    // Add the number of shapes that are children of this shape.
    if (mpChildrenManager != NULL)
        nChildCount += mpChildrenManager->GetChildCount ();
    // Add the number text paragraphs.
    if (mpText != NULL)
        nChildCount += mpText->GetChildCount ();

    return nChildCount;
}




/** Forward the request to the shape.  Return the requested shape or throw
    an exception for a wrong index.
*/
uno::Reference<XAccessible> SAL_CALL
    AccessibleShape::getAccessibleChild (sal_Int32 nIndex)
    throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    uno::Reference<XAccessible> xChild;

    // Depending on the index decide whether to delegate this call to the
    // children manager or the edit engine.
    if ((mpChildrenManager != NULL)
        && (nIndex < mpChildrenManager->GetChildCount()))
    {
        xChild = mpChildrenManager->GetChild (nIndex);
    }
    else if (mpText != NULL)
    {
        sal_Int32 nI = nIndex;
        if (mpChildrenManager != NULL)
            nI -= mpChildrenManager->GetChildCount();
        xChild = mpText->GetChild (nI);
    }
    else
        throw lang::IndexOutOfBoundsException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("shape has no child with index "))
            + rtl::OUString::valueOf(nIndex),
            static_cast<uno::XWeak*>(this));

    return xChild;
}




/** Return a copy of the state set.
    Possible states are:
        ENABLED
        SHOWING
        VISIBLE
*/
uno::Reference<XAccessibleStateSet> SAL_CALL
    AccessibleShape::getAccessibleStateSet (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    Reference<XAccessibleStateSet> xStateSet;

    if (rBHelper.bDisposed || mpText == NULL)
        // Return a minimal state set that only contains the DEFUNC state.
        xStateSet = AccessibleContextBase::getAccessibleStateSet ();
    else
    {
        ::utl::AccessibleStateSetHelper* pStateSet =
              static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());

        if (pStateSet != NULL)
        {
            // Merge current FOCUSED state from edit engine.
            if (mpText != NULL)
            {
                if (mpText->HaveFocus())
                    pStateSet->AddState (AccessibleStateType::FOCUSED);
                else
                    pStateSet->RemoveState (AccessibleStateType::FOCUSED);
            }

            // Create a copy of the state set that may be modified by the
            // caller without affecting the current state set.
            xStateSet = Reference<XAccessibleStateSet>(
                new ::utl::AccessibleStateSetHelper (*pStateSet));
        }
    }

    return xStateSet;
}




//=====  XAccessibleComponent  ================================================

/** The implementation below is at the moment straightforward.  It iterates
    over all children (and thereby instances all children which have not
    been already instatiated) until a child covering the specifed point is
    found.
    This leaves room for improvement.  For instance, first iterate only over
    the already instantiated children and only if no match is found
    instantiate the remaining ones.
*/
uno::Reference<XAccessible > SAL_CALL
    AccessibleShape::getAccessibleAtPoint (
        const awt::Point& aPoint)
    throw (uno::RuntimeException)
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




awt::Rectangle SAL_CALL AccessibleShape::getBounds (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard (maMutex);

    ThrowIfDisposed ();
    awt::Rectangle aBoundingBox;
    if ( mxShape.is() )
    {

        static const OUString sBoundRectName (
            RTL_CONSTASCII_USTRINGPARAM("BoundRect"));
        static const OUString sAnchorPositionName (
            RTL_CONSTASCII_USTRINGPARAM("AnchorPosition"));

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
                    OSL_TRACE (" no property BoundRect");
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
        if (maShapeTreeInfo.GetViewForwarder() == NULL)
            throw uno::RuntimeException (::rtl::OUString (
                RTL_CONSTASCII_USTRINGPARAM(
                    "AccessibleShape has no valid view forwarder")),
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
            ::Rectangle aBBox (
                x, y, x + aPixelSize.getWidth(), y + aPixelSize.getHeight());
            awt::Size aParentSize (xParentComponent->getSize());
            ::Rectangle aParentBBox (0,0, aParentSize.Width, aParentSize.Height);
            aBBox = aBBox.GetIntersection (aParentBBox);
            aBoundingBox = awt::Rectangle (
                aBBox.getX(),
                aBBox.getY(),
                aBBox.getWidth(),
                aBBox.getHeight());
        }
        else
        {
            OSL_TRACE ("parent does not support component");
            aBoundingBox = awt::Rectangle (
                aPixelPosition.getX(), aPixelPosition.getY(),
                aPixelSize.getWidth(), aPixelSize.getHeight());
        }
    }

    return aBoundingBox;
}




awt::Point SAL_CALL AccessibleShape::getLocation (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    awt::Rectangle aBoundingBox (getBounds());
    return awt::Point (aBoundingBox.X, aBoundingBox.Y);
}




awt::Point SAL_CALL AccessibleShape::getLocationOnScreen (void)
    throw (::com::sun::star::uno::RuntimeException)
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
        OSL_TRACE ("getLocation: parent does not support XAccessibleComponent");
    return aLocation;
}




awt::Size SAL_CALL AccessibleShape::getSize (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();
    awt::Rectangle aBoundingBox (getBounds());
    return awt::Size (aBoundingBox.Width, aBoundingBox.Height);
}




sal_Int32 SAL_CALL AccessibleShape::getForeground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    sal_Int32 nColor (0x0ffffffL);

    try
    {
        uno::Reference<beans::XPropertySet> aSet (mxShape, uno::UNO_QUERY);
        if (aSet.is())
        {
            uno::Any aColor;
            aColor = aSet->getPropertyValue (OUString(RTL_CONSTASCII_USTRINGPARAM("LineColor")) );
            aColor >>= nColor;
        }
    }
    catch (const ::com::sun::star::beans::UnknownPropertyException &)
    {
        // Ignore exception and return default color.
    }
    return nColor;
}




sal_Int32 SAL_CALL AccessibleShape::getBackground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    sal_Int32 nColor (0L);

    try
    {
        uno::Reference<beans::XPropertySet> aSet (mxShape, uno::UNO_QUERY);
        if (aSet.is())
        {
            uno::Any aColor;
            aColor = aSet->getPropertyValue (OUString(RTL_CONSTASCII_USTRINGPARAM("FillColor")) );
            aColor >>= nColor;
        }
    }
    catch (const ::com::sun::star::beans::UnknownPropertyException &)
    {
        // Ignore exception and return default color.
    }
    return nColor;
}




//=====  XAccessibleEventBroadcaster  =========================================

void SAL_CALL AccessibleShape::addEventListener (
    const Reference<XAccessibleEventListener >& rxListener)
    throw (uno::RuntimeException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        uno::Reference<uno::XInterface> xThis (
            (lang::XComponent *)this, uno::UNO_QUERY);
        rxListener->disposing (lang::EventObject (xThis));
    }
    else
    {
        AccessibleContextBase::addEventListener (rxListener);
        if (mpText != NULL)
            mpText->AddEventListener (rxListener);
    }
}




void SAL_CALL AccessibleShape::removeEventListener (
    const Reference<XAccessibleEventListener >& rxListener)
    throw (uno::RuntimeException)
{
    AccessibleContextBase::removeEventListener (rxListener);
    if (mpText != NULL)
        mpText->RemoveEventListener (rxListener);
}




//=====  XInterface  ==========================================================

com::sun::star::uno::Any SAL_CALL
    AccessibleShape::queryInterface (const com::sun::star::uno::Type & rType)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = AccessibleContextBase::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast<XAccessibleComponent*>(this),
            static_cast<XAccessibleExtendedComponent*>(this),
            static_cast<lang::XEventListener*>(this),
            static_cast<document::XEventListener*>(this),
            static_cast<lang::XUnoTunnel*>(this)
            );
    return aReturn;
}




void SAL_CALL
    AccessibleShape::acquire (void)
    throw ()
{
    AccessibleContextBase::acquire ();
}




void SAL_CALL
    AccessibleShape::release (void)
    throw ()
{
    AccessibleContextBase::release ();
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessibleShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleShape"));
}




uno::Sequence<OUString> SAL_CALL
    AccessibleShape::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    // Get list of supported service names from base class...
    uno::Sequence<OUString> aServiceNames =
        AccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nCount (aServiceNames.getLength());

    // ...and add additional names.
    aServiceNames.realloc (nCount + 1);
    static const OUString sAdditionalServiceName (RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.drawing.AccessibleShape"));
    aServiceNames[nCount] = sAdditionalServiceName;

    return aServiceNames;
}





//=====  XTypeProvider  ===================================================

uno::Sequence<uno::Type> SAL_CALL
    AccessibleShape::getTypes (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();
    // Get list of types from the context base implementation, ...
    uno::Sequence<uno::Type> aTypeList (AccessibleContextBase::getTypes());
    // ... get list of types from component base implementation, ...
    uno::Sequence<uno::Type> aComponentTypeList (AccessibleComponentBase::getTypes());
    // ... define local types, ...
    const uno::Type aLangEventListenerType =
        ::getCppuType((const uno::Reference<lang::XEventListener>*)0);
    const uno::Type aDocumentEventListenerType =
        ::getCppuType((const uno::Reference<document::XEventListener>*)0);
    const uno::Type aUnoTunnelType =
        ::getCppuType((const uno::Reference<lang::XUnoTunnel>*)0);

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




//=====  lang::XEventListener  ================================================

/** Disposing calls are accepted only from the model: Just reset the
    reference to the model in the shape tree info.  Otherwise this object
    remains functional.
*/
void SAL_CALL
    AccessibleShape::disposing (const lang::EventObject& aEvent)
    throw (uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard (maMutex);

    try
    {
        if (aEvent.Source ==  maShapeTreeInfo.GetModelBroadcaster())
        {
            // Remove reference to model broadcaster to allow it to pass
            // away.
            maShapeTreeInfo.SetModelBroadcaster(NULL);
        }

    }
    catch (uno::RuntimeException const&)
    {
        OSL_TRACE ("caught exception while disposing");
    }
}




//=====  document::XEventListener  ============================================

void SAL_CALL
    AccessibleShape::notifyEvent (const document::EventObject& rEventObject)
    throw (uno::RuntimeException)
{
    static const OUString sShapeModified (
        RTL_CONSTASCII_USTRINGPARAM("ShapeModified"));

    // First check if the event is for us.
    uno::Reference<drawing::XShape> xShape (
        rEventObject.Source, uno::UNO_QUERY);
    if ( xShape.get() == mxShape.get() )
    {
        if (rEventObject.EventName.equals (sShapeModified))
        {
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

//=====  lang::XUnoTunnel  ================================================

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

//------------------------------------------------------------------------------
AccessibleShape*
    AccessibleShape::getImplementation( const uno::Reference< uno::XInterface >& rxIFace )
    throw()
{
    uno::Reference< lang::XUnoTunnel >  xTunnel( rxIFace, uno::UNO_QUERY );
    AccessibleShape*                    pReturn = NULL;

    if( xTunnel.is() )
        pReturn = reinterpret_cast< AccessibleShape* >( xTunnel->getSomething( getUnoTunnelImplementationId() ) );

    return( pReturn );
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL
    AccessibleShape::getSomething( const uno::Sequence< sal_Int8 >& rIdentifier )
    throw(uno::RuntimeException)
{
    sal_Int64 nReturn( 0 );

    if( ( rIdentifier.getLength() == 16 ) && ( 0 == memcmp( getUnoTunnelImplementationId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) )
        nReturn = reinterpret_cast< sal_Int64 >( this );

    return( nReturn );
}

//=====  IAccessibleViewForwarderListener  ====================================

void AccessibleShape::ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder)
{
    // Inform all listeners that the graphical representation (i.e. size
    // and/or position) of the shape has changed.
    CommitChange (AccessibleEventId::VISIBLE_DATA_CHANGED,
        uno::Any(),
        uno::Any());

    // Tell children manager of the modified view forwarder.
    if (mpChildrenManager != NULL)
        mpChildrenManager->ViewForwarderChanged (aChangeType, pViewForwarder);

    // update our children that our screen position might have changed
    if( mpText )
        mpText->UpdateChildren();
}




//=====  protected internal  ==================================================
/// Set this object's name if is different to the current name.
::rtl::OUString
    AccessibleShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ShapeTypeHandler::CreateAccessibleBaseName( mxShape );
}


::rtl::OUString
    AccessibleShape::CreateAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    OUString sName (CreateAccessibleBaseName());

    // Append the shape's index to the name to disambiguate between shapes
    // of the same type.  If such an index where not given to the
    // constructor then use the z-order instead.  If even that does not exist
    // we throw an exception.
    long nIndex = mnIndex;
    if (nIndex == -1)
    {
        try
        {
            uno::Reference<beans::XPropertySet> xSet (mxShape, uno::UNO_QUERY);
            if (xSet.is())
            {
                uno::Any aZOrder (xSet->getPropertyValue (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ZOrder")) ));
                aZOrder >>= nIndex;

                // Add one to be not zero based.
                nIndex += 1;
            }
        }
        catch (const beans::UnknownPropertyException &)
        {
            // We throw our own exception that is a bit more informative.
            throw uno::RuntimeException (::rtl::OUString (
                RTL_CONSTASCII_USTRINGPARAM("AccessibleShape has invalid index and no ZOrder property")),
                static_cast<uno::XWeak*>(this));
        }

    }

    // Put a space between name and index because of Gnopernicus othewise
    // spells the name.
    sName += OUString (RTL_CONSTASCII_USTRINGPARAM(" ")) + OUString::valueOf (nIndex);

    return sName;
}




::rtl::OUString
    AccessibleShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
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
            aDG.AddProperty (OUString(RTL_CONSTASCII_USTRINGPARAM("ControlBackground")),
                DescriptionGenerator::COLOR,
                OUString());
            aDG.AddProperty (OUString(RTL_CONSTASCII_USTRINGPARAM("ControlBorder")),
                DescriptionGenerator::INTEGER,
                OUString());
            break;

        case DRAWING_TEXT:
            aDG.AddTextProperties ();
            break;

        default:
            aDG.Initialize (::rtl::OUString (
                                RTL_CONSTASCII_USTRINGPARAM("Unknown accessible shape")));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("service name=")));
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}




uno::Reference< drawing::XShape > AccessibleShape::GetXShape()
{
    return( mxShape );
}



// protected
void AccessibleShape::disposing (void)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard (maMutex);

    // Make sure to send an event that this object looses the focus in the
    // case that it has the focus.
    ::utl::AccessibleStateSetHelper* pStateSet =
          static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if (pStateSet != NULL)
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
    if (mpChildrenManager != NULL)
    {
        delete mpChildrenManager;
        mpChildrenManager = NULL;
    }
    if (mpText != NULL)
    {
        mpText->Dispose();
        delete mpText;
        mpText = NULL;
    }

    // Cleanup.  Remove references to objects to allow them to be
    // destroyed.
    mxShape = NULL;
    maShapeTreeInfo = AccessibleShapeTreeInfo();

    // Call base classes.
    AccessibleContextBase::dispose ();
}

sal_Int32 SAL_CALL
       AccessibleShape::getAccessibleIndexInParent (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    //  Use a simple but slow solution for now.  Optimize later.

    sal_Int32 nIndex = m_nIndexInParent;
    if ( -1 == nIndex )
        nIndex = AccessibleContextBase::getAccessibleIndexInParent();
    return nIndex;
}




void AccessibleShape::UpdateNameAndDescription (void)
{
    // Ignore missing title, name, or description.  There are fallbacks for
    // them.
    try
    {
        Reference<beans::XPropertySet> xSet (mxShape, uno::UNO_QUERY_THROW);
        OUString sString;

        // Get the accessible name.
        sString = GetOptionalProperty(xSet, OUString(RTL_CONSTASCII_USTRINGPARAM("Title")));
        if (!sString.isEmpty())
        {
            SetAccessibleName(sString, AccessibleContextBase::FromShape);
        }
        else
        {
            sString = GetOptionalProperty(xSet, OUString(RTL_CONSTASCII_USTRINGPARAM("Name")));
            if (!sString.isEmpty())
                SetAccessibleName(sString, AccessibleContextBase::FromShape);
        }

        // Get the accessible description.
        sString = GetOptionalProperty(xSet, OUString(RTL_CONSTASCII_USTRINGPARAM("Description")));
        if (!sString.isEmpty())
            SetAccessibleDescription(sString, AccessibleContextBase::FromShape);
    }
    catch (uno::RuntimeException&)
    {
    }
}




} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
