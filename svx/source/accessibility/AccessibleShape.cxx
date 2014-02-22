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

#include <svx/AccessibleShape.hxx>
#include "svx/DescriptionGenerator.hxx"
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
#include <editeng/unoedsrc.hxx>
#include <svx/unoshtxt.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdpage.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <svx/ShapeTypeHandler.hxx>
#include <svx/SvxShapeTypes.hxx>

#include "accessibility.hrc"
#include "svx/svdstr.hrc"
#include <svx/dialmgr.hxx>
#include <vcl/svapp.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <svx/svdview.hxx>
#include <comphelper/servicehelper.hxx>
#include "AccessibleEmptyEditSource.hxx"

#include <algorithm>

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
                
                
                
            }
        }
    }
    return sValue;
}

} 






AccessibleShape::AccessibleShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleContextBase (rShapeInfo.mxParent,AccessibleRole::SHAPE),
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

AccessibleShape::AccessibleShape (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShape>& rxShape,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
        const AccessibleShapeTreeInfo& rShapeTreeInfo,
        sal_Int32 nIndex)
    : AccessibleContextBase (rxParent,AccessibleRole::SHAPE),
      mpChildrenManager(NULL),
      mxShape (rxShape),
      maShapeTreeInfo (rShapeTreeInfo),
      mnIndex (nIndex),
      m_nIndexInParent(-1),
      mpText (NULL),
      mpParent (NULL)
{
    m_pShape = GetSdrObjectFromXShape(mxShape);
}

AccessibleShape::~AccessibleShape (void)
{
    delete mpChildrenManager;
    delete mpText;
    OSL_TRACE ("~AccessibleShape");

    
    
    
}

void AccessibleShape::Init (void)
{
    
    UpdateStates ();

    
    Reference<drawing::XShapes> xShapes (mxShape, uno::UNO_QUERY);
    if (xShapes.is() && xShapes->getCount() > 0)
        mpChildrenManager = new ChildrenManager (
            this, xShapes, maShapeTreeInfo, *this);
    if (mpChildrenManager != NULL)
        mpChildrenManager->Update();

    
    if (maShapeTreeInfo.GetModelBroadcaster().is())
        maShapeTreeInfo.GetModelBroadcaster()->addEventListener (
            static_cast<document::XEventListener*>(this));

    
    
    
    Reference<text::XText> xText (mxShape, uno::UNO_QUERY);
    if (xText.is())
    {
        SdrView* pView = maShapeTreeInfo.GetSdrView ();
        const Window* pWindow = maShapeTreeInfo.GetWindow ();
        if (pView != NULL && pWindow != NULL && mxShape.is())
        {
            
            SdrObject* pSdrObject = GetSdrObjectFromXShape(mxShape);
            if( pSdrObject )
            {
                SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, pSdrObject );
                OutlinerParaObject* pOutlinerParaObject = NULL;

                if( pTextObj )
                    pOutlinerParaObject = pTextObj->GetEditOutlinerParaObject(); 

                bool bOwnParaObj = pOutlinerParaObject != NULL;

                if( !pOutlinerParaObject && pSdrObject )
                    pOutlinerParaObject = pSdrObject->GetOutlinerParaObject();

                
                if( !pOutlinerParaObject )
                {
                    
                    SAL_WNODEPRECATED_DECLARATIONS_PUSH
                    ::std::auto_ptr<SvxEditSource> pEditSource( new AccessibleEmptyEditSource ( *pSdrObject, *pView, *pWindow) );
                    SAL_WNODEPRECATED_DECLARATIONS_POP
                    mpText = new AccessibleTextHelper( pEditSource );
                }
                else
                {
                    
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
                catch (::com::sun::star::beans::UnknownPropertyException&)
                {
                    
                }
            }
        }
    }
    if (bShapeIsOpaque)
        pStateSet->AddState (AccessibleStateType::OPAQUE);
    else
        pStateSet->RemoveState (AccessibleStateType::OPAQUE);

    
    bool bShapeIsSelected = false;
    
    if ( m_pShape && maShapeTreeInfo.GetSdrView() )
    {
        bShapeIsSelected = maShapeTreeInfo.GetSdrView()->IsObjMarked(m_pShape) == sal_True;
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

bool AccessibleShape::operator== (const AccessibleShape& rShape)
{
    return this==&rShape;
}




sal_Bool AccessibleShape::SetState (sal_Int16 aState)
{
    sal_Bool bStateHasChanged = sal_False;

    if (aState == AccessibleStateType::FOCUSED && mpText != NULL)
    {
        
        
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
        
        
        return mpText->HaveFocus();
    }
    else
        return AccessibleContextBase::GetState (aState);
}


OUString SAL_CALL AccessibleShape::getAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
        ThrowIfDisposed ();
    if (m_pShape && !m_pShape->GetTitle().isEmpty())
        return CreateAccessibleName() + " " + m_pShape->GetTitle();
    else
        return CreateAccessibleName();
}

OUString SAL_CALL AccessibleShape::getAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    if( m_pShape && !m_pShape->GetDescription().isEmpty())
        return m_pShape->GetDescription() ;
    else
        return OUString(" ");
}



/** The children of this shape come from two sources: The children from
    group or scene shapes and the paragraphs of text.
*/
sal_Int32 SAL_CALL
       AccessibleShape::getAccessibleChildCount ()
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    sal_Int32 nChildCount = 0;

    
    if (mpChildrenManager != NULL)
        nChildCount += mpChildrenManager->GetChildCount ();
    
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
            OUString("shape has no child with index ")
            + OUString::number(nIndex),
            static_cast<uno::XWeak*>(this));

    return xChild;
}

uno::Reference<XAccessibleRelationSet> SAL_CALL
    AccessibleShape::getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ::utl::AccessibleRelationSetHelper* pRelationSet = new utl::AccessibleRelationSetHelper;

    
    uno::Sequence< uno::Reference< uno::XInterface > > aSequence(1);
    aSequence[0] = mpParent->GetAccessibleCaption(mxShape);
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
    AccessibleShape::getAccessibleStateSet (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    Reference<XAccessibleStateSet> xStateSet;

    if (rBHelper.bDisposed || mpText == NULL)
        
    {
        xStateSet = AccessibleContextBase::getAccessibleStateSet ();
        ::utl::AccessibleStateSetHelper* pStateSet =
              static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
            ::com::sun::star::uno::Reference<XAccessible> xTempAcc = getAccessibleParent();
            if( xTempAcc.is() )
            {
                ::com::sun::star::uno::Reference<XAccessibleContext>
                                        xTempAccContext = xTempAcc->getAccessibleContext();
                if( xTempAccContext.is() )
                {
                    ::com::sun::star::uno::Reference<XAccessibleStateSet> rState =
                        xTempAccContext->getAccessibleStateSet();
                    if( rState.is() )           {
                        com::sun::star::uno::Sequence<short> pStates = rState->getStates();
                        int count = pStates.getLength();
                        for( int iIndex = 0;iIndex < count;iIndex++ )
                        {
                            if( pStates[iIndex] == AccessibleStateType::EDITABLE )
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
            xStateSet = Reference<XAccessibleStateSet>(
                new ::utl::AccessibleStateSetHelper (*pStateSet));
    }else
    {
        ::utl::AccessibleStateSetHelper* pStateSet =
              static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());

        if (pStateSet != NULL)
        {
            
            if (mpText != NULL)
            {
                if (mpText->HaveFocus())
                    pStateSet->AddState (AccessibleStateType::FOCUSED);
                else
                    pStateSet->RemoveState (AccessibleStateType::FOCUSED);
            }
            
            ::com::sun::star::uno::Reference<XAccessible> xTempAcc = getAccessibleParent();
            if( xTempAcc.is() )
            {
                ::com::sun::star::uno::Reference<XAccessibleContext>
                                        xTempAccContext = xTempAcc->getAccessibleContext();
                if( xTempAccContext.is() )
                {
                    ::com::sun::star::uno::Reference<XAccessibleStateSet> rState =
                        xTempAccContext->getAccessibleStateSet();
                    if( rState.is() )           {
                        com::sun::star::uno::Sequence<short> pStates = rState->getStates();
                        int count = pStates.getLength();
                        for( int iIndex = 0;iIndex < count;iIndex++ )
                        {
                            if( pStates[iIndex] == AccessibleStateType::EDITABLE )
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
            
            
            xStateSet = Reference<XAccessibleStateSet>(
                new ::utl::AccessibleStateSetHelper (*pStateSet));
        }
    }
    UpdateDocumentAllSelState(xStateSet);
    return xStateSet;
}






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

        static const OUString sBoundRectName ("BoundRect");
        static const OUString sAnchorPositionName ("AnchorPosition");

        
        
        
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
                        
                    }
                }
                else
                    OSL_TRACE (" no property BoundRect");
            }
        }

        
        if ( ! bFoundBoundRect )
        {
            awt::Point aPosition (mxShape->getPosition());
            awt::Size aSize (mxShape->getSize());
            aBoundingBox = awt::Rectangle (
                aPosition.X, aPosition.Y,
                aSize.Width, aSize.Height);

            
            
            
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

        
        if (maShapeTreeInfo.GetViewForwarder() == NULL)
            throw uno::RuntimeException (OUString (
                "AccessibleShape has no valid view forwarder"),
                static_cast<uno::XWeak*>(this));
        ::Size aPixelSize = maShapeTreeInfo.GetViewForwarder()->LogicToPixel (
            ::Size (aBoundingBox.Width, aBoundingBox.Height));
        ::Point aPixelPosition = maShapeTreeInfo.GetViewForwarder()->LogicToPixel (
            ::Point (aBoundingBox.X, aBoundingBox.Y));

        
        Reference<XAccessibleComponent> xParentComponent (
            getAccessibleParent(), uno::UNO_QUERY);
        if (xParentComponent.is())
        {
            
            awt::Point aParentLocation (xParentComponent->getLocationOnScreen());
            int x = aPixelPosition.getX() - aParentLocation.X;
            int y = aPixelPosition.getY() - aParentLocation.Y;

            
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

    
    awt::Point aLocation (getLocation ());

    
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
            aColor = aSet->getPropertyValue ("LineColor");
            aColor >>= nColor;
        }
    }
    catch (const ::com::sun::star::beans::UnknownPropertyException &)
    {
        
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
            aColor = aSet->getPropertyValue ("FillColor");
            aColor >>= nColor;
            aColor = aSet->getPropertyValue (OUString::createFromAscii ("FillTransparence"));
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
            nColor = crBk.GetColor();
        }
    }
    catch (const ::com::sun::star::beans::UnknownPropertyException &)
    {
        
    }
    return nColor;
}






void SAL_CALL AccessibleShape::addAccessibleEventListener (
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
        AccessibleContextBase::addAccessibleEventListener (rxListener);
        if (mpText != NULL)
            mpText->AddEventListener (rxListener);
    }
}




void SAL_CALL AccessibleShape::removeAccessibleEventListener (
    const Reference<XAccessibleEventListener >& rxListener)
    throw (uno::RuntimeException)
{
    AccessibleContextBase::removeAccessibleEventListener (rxListener);
    if (mpText != NULL)
        mpText->RemoveEventListener (rxListener);
}






com::sun::star::uno::Any SAL_CALL
    AccessibleShape::queryInterface (const com::sun::star::uno::Type & rType)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = AccessibleContextBase::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast<XAccessibleComponent*>(this),
            static_cast<XAccessibleExtendedComponent*>(this),
            static_cast< ::com::sun::star::accessibility::XAccessibleSelection* >(this),
            static_cast< ::com::sun::star::accessibility::XAccessibleExtendedAttributes* >(this),
            static_cast<lang::XEventListener*>(this),
            static_cast<document::XEventListener*>(this),
            static_cast<lang::XUnoTunnel*>(this),
            static_cast<XAccessibleGroupPosition*>(this),
            static_cast<XAccessibleHypertext*>(this)
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


//


void SAL_CALL AccessibleShape::selectAccessibleChild( sal_Int32 )
throw ( IndexOutOfBoundsException, RuntimeException )
{
}


sal_Bool SAL_CALL AccessibleShape::isAccessibleChildSelected( sal_Int32 nChildIndex )
throw ( IndexOutOfBoundsException,
       RuntimeException )
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
            uno::Reference< ::com::sun::star::accessibility::XAccessibleText >
                xText(xAcc, uno::UNO_QUERY);
            if( xText.is() )
            {
                if( xText->getSelectionStart() >= 0 ) return sal_True;
            }
        }
        else if( xContext->getAccessibleRole() == AccessibleRole::SHAPE )
        {
            Reference< XAccessibleStateSet > pRState = xContext->getAccessibleStateSet();
            if( !pRState.is() )
                return sal_False;

            uno::Sequence<short> pStates = pRState->getStates();
            int nCount = pStates.getLength();
            for( int i = 0; i < nCount; i++ )
            {
                if(pStates[i] == AccessibleStateType::SELECTED)
                    return sal_True;
            }
            return sal_False;
        }
    }

    return sal_False;
}


void SAL_CALL AccessibleShape::clearAccessibleSelection(  )
throw ( RuntimeException )
{
}


void SAL_CALL AccessibleShape::selectAllAccessibleChildren(  )
throw ( RuntimeException )
{
}


sal_Int32 SAL_CALL AccessibleShape::getSelectedAccessibleChildCount()
throw ( RuntimeException )
{
    sal_Int32 nCount = 0;
    sal_Int32 TotalCount = getAccessibleChildCount();
    for( sal_Int32 i = 0; i < TotalCount; i++ )
        if( isAccessibleChildSelected(i) ) nCount++;

    return nCount;
}


Reference<XAccessible> SAL_CALL AccessibleShape::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
throw ( IndexOutOfBoundsException, RuntimeException)
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
                                                            throw ( IndexOutOfBoundsException,
                                                            RuntimeException )
{

}


uno::Any SAL_CALL AccessibleShape::getExtendedAttributes()
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
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



OUString SAL_CALL
    AccessibleShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString("AccessibleShape");
}




uno::Sequence<OUString> SAL_CALL
    AccessibleShape::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    
    uno::Sequence<OUString> aServiceNames =
        AccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nCount (aServiceNames.getLength());

    
    aServiceNames.realloc (nCount + 1);
    static const OUString sAdditionalServiceName ("com.sun.star.drawing.AccessibleShape");
    aServiceNames[nCount] = sAdditionalServiceName;

    return aServiceNames;
}







uno::Sequence<uno::Type> SAL_CALL
    AccessibleShape::getTypes (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed ();
    
    uno::Sequence<uno::Type> aTypeList (AccessibleContextBase::getTypes());
    
    uno::Sequence<uno::Type> aComponentTypeList (AccessibleComponentBase::getTypes());
    
    const uno::Type aLangEventListenerType =
        ::getCppuType((const uno::Reference<lang::XEventListener>*)0);
    const uno::Type aDocumentEventListenerType =
        ::getCppuType((const uno::Reference<document::XEventListener>*)0);
    const uno::Type aUnoTunnelType =
        ::getCppuType((const uno::Reference<lang::XUnoTunnel>*)0);

    
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
            
            
            maShapeTreeInfo.SetModelBroadcaster(NULL);
        }

    }
    catch (uno::RuntimeException const&)
    {
        OSL_TRACE ("caught exception while disposing");
    }
}






void SAL_CALL
    AccessibleShape::notifyEvent (const document::EventObject& rEventObject)
    throw (uno::RuntimeException)
{
    static const OUString sShapeModified ("ShapeModified");

    
    uno::Reference<drawing::XShape> xShape (
        rEventObject.Source, uno::UNO_QUERY);
    if ( xShape.get() == mxShape.get() )
    {
        if (rEventObject.EventName.equals (sShapeModified))
        {
            
            if (mpText)
                mpText->UpdateChildren();


            
            
            CommitChange (
                AccessibleEventId::VISIBLE_DATA_CHANGED,
                uno::Any(),
                uno::Any());

            
            
            UpdateNameAndDescription();
        }
    }
}



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
    AccessibleShape*                    pReturn = NULL;

    if( xTunnel.is() )
        pReturn = reinterpret_cast< AccessibleShape* >( xTunnel->getSomething( getUnoTunnelImplementationId() ) );

    return( pReturn );
}


sal_Int64 SAL_CALL
    AccessibleShape::getSomething( const uno::Sequence< sal_Int8 >& rIdentifier )
    throw(uno::RuntimeException)
{
    sal_Int64 nReturn( 0 );

    if( ( rIdentifier.getLength() == 16 ) && ( 0 == memcmp( getUnoTunnelImplementationId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) )
        nReturn = reinterpret_cast< sal_Int64 >( this );

    return( nReturn );
}



void AccessibleShape::ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder)
{
    
    
    CommitChange (AccessibleEventId::VISIBLE_DATA_CHANGED,
        uno::Any(),
        uno::Any());

    
    if (mpChildrenManager != NULL)
        mpChildrenManager->ViewForwarderChanged (aChangeType, pViewForwarder);

    
    if( mpText )
        mpText->UpdateChildren();
}






OUString AccessibleShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ShapeTypeHandler::CreateAccessibleBaseName( mxShape );
}


OUString AccessibleShape::CreateAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return GetFullAccessibleName(this);
}

OUString AccessibleShape::GetFullAccessibleName (AccessibleShape *shape)
    throw (::com::sun::star::uno::RuntimeException)
{
    OUString sName (shape->CreateAccessibleBaseName());
    
    
    
    
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
            aDG.AddProperty ("ControlBackground", DescriptionGenerator::COLOR, "");
            aDG.AddProperty ("ControlBorder", DescriptionGenerator::INTEGER, "");
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




uno::Reference< drawing::XShape > AccessibleShape::GetXShape()
{
    return( mxShape );
}




void AccessibleShape::disposing (void)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard (maMutex);

    
    
    ::utl::AccessibleStateSetHelper* pStateSet =
          static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if (pStateSet != NULL)
        pStateSet->RemoveState (AccessibleStateType::FOCUSED);

    
    Reference<lang::XComponent> xComponent (mxShape, uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener (this);

    
    if (maShapeTreeInfo.GetModelBroadcaster().is())
        maShapeTreeInfo.GetModelBroadcaster()->removeEventListener (
            static_cast<document::XEventListener*>(this));

    
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

    
    
    mxShape = NULL;
    maShapeTreeInfo = AccessibleShapeTreeInfo();

    
    AccessibleContextBase::dispose ();
}

sal_Int32 SAL_CALL
       AccessibleShape::getAccessibleIndexInParent (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    

    sal_Int32 nIndex = m_nIndexInParent;
    if ( -1 == nIndex )
        nIndex = AccessibleContextBase::getAccessibleIndexInParent();
    return nIndex;
}




void AccessibleShape::UpdateNameAndDescription (void)
{
    
    
    try
    {
        Reference<beans::XPropertySet> xSet (mxShape, uno::UNO_QUERY_THROW);
        OUString sString;

        
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

        
        sString = GetOptionalProperty(xSet, "Description");
        if (!sString.isEmpty())
            SetAccessibleDescription(sString, AccessibleContextBase::FromShape);
    }
    catch (uno::RuntimeException&)
    {
    }
}


sal_Int16 SAL_CALL AccessibleShape::getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException)
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


void AccessibleShape::UpdateDocumentAllSelState(Reference<XAccessibleStateSet> &xStateSet)
{
    if (mpParent && mpParent->IsDocumentSelAll())
    {
        ::utl::AccessibleStateSetHelper* pStateSet =
            static_cast< ::utl::AccessibleStateSetHelper*>(xStateSet.get());
        pStateSet->AddState (AccessibleStateType::SELECTED);

        
        

        
    }
}


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



uno::Sequence< sal_Int32 > SAL_CALL
AccessibleShape::getGroupPosition( const uno::Any& )
throw (uno::RuntimeException)
{
    
    
    
    
    uno::Sequence< sal_Int32 > aRet( 3 );
    aRet[0] = 0;
    aRet[1] = 0;
    aRet[2] = 0;

    ::com::sun::star::uno::Reference<XAccessible> xParent = getAccessibleParent();
    if (!xParent.is())
    {
        return aRet;
    }
    SdrObject *pObj = GetSdrObjectFromXShape(mxShape);


    if(pObj == NULL )
    {
        return aRet;
    }

    
    sal_Int32 nGroupLevel = 0;
    SdrObject * pUper = pObj->GetUpGroup();
    while( pUper )
    {
        ++nGroupLevel;
        pUper = pUper->GetUpGroup();
    }

    ::com::sun::star::uno::Reference<XAccessibleContext> xParentContext = xParent->getAccessibleContext();
    if( xParentContext->getAccessibleRole()  == AccessibleRole::DOCUMENT ||
            xParentContext->getAccessibleRole()  == AccessibleRole::DOCUMENT_PRESENTATION ||
            xParentContext->getAccessibleRole()  == AccessibleRole::DOCUMENT_SPREADSHEET ||
            xParentContext->getAccessibleRole()  == AccessibleRole::DOCUMENT_TEXT )
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

    SdrObjList *pGrpList = NULL;
    if( pObj->GetUpGroup() )
        pGrpList = pObj->GetUpGroup()->GetSubList();
    else
        return aRet;

    std::vector< uno::Reference<drawing::XShape> > vXShapes;
    if (pGrpList)
    {
        const sal_Int32 nObj = pGrpList->GetObjCount();
        for(sal_Int32 i = 0 ; i < nObj ; ++i)
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

    
    std::vector< uno::Reference<drawing::XShape> >::iterator aIter;
    
    sal_Int32 nPos = 1;
    for ( aIter = vXShapes.begin(); aIter != vXShapes.end(); ++aIter, nPos++ )
    {
        if ( (*aIter).get() == mxShape.get() )
        {
            sal_Int32* pArray = aRet.getArray();
            pArray[0] = nGroupLevel;
            pArray[1] = vXShapes.size();
            pArray[2] = nPos;
            break;
        }
    }

    return aRet;
}

OUString AccessibleShape::getObjectLink( const uno::Any& )
    throw (uno::RuntimeException)
{
    OUString aRet;

    SdrObject *pObj = GetSdrObjectFromXShape(mxShape);
    if(pObj == NULL )
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


sal_Int32 SAL_CALL AccessibleShape::getHyperLinkCount()
    throw (::com::sun::star::uno::RuntimeException)
{
    
    
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
    throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    uno::Reference< XAccessibleHyperlink > xRet;
    
    
    /*
    SvxAccessibleHyperlink* pLink = new SvxAccessibleHyperlink(m_pShape,this);
    if (pLink->IsValidHyperlink())
        xRet = pLink;
    if( !xRet.is() )
        throw ::com::sun::star::lang::IndexOutOfBoundsException();
    */
    return xRet;
}
sal_Int32 SAL_CALL AccessibleShape::getHyperLinkIndex( sal_Int32 )
throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nRet = 0;
    return nRet;
}

sal_Int32 SAL_CALL AccessibleShape::getCaretPosition(  ) throw (::com::sun::star::uno::RuntimeException){return 0;}
sal_Bool SAL_CALL AccessibleShape::setCaretPosition( sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException){return 0;}
sal_Unicode SAL_CALL AccessibleShape::getCharacter( sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException){return 0;}
::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL AccessibleShape::getCharacterAttributes( sal_Int32, const ::com::sun::star::uno::Sequence< OUString >& ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    uno::Sequence< ::com::sun::star::beans::PropertyValue > aValues(0);
    return aValues;
}
::com::sun::star::awt::Rectangle SAL_CALL AccessibleShape::getCharacterBounds( sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    return com::sun::star::awt::Rectangle(0, 0, 0, 0 );
}
sal_Int32 SAL_CALL AccessibleShape::getCharacterCount(  ) throw (::com::sun::star::uno::RuntimeException){return 0;}
sal_Int32 SAL_CALL AccessibleShape::getIndexAtPoint( const ::com::sun::star::awt::Point& ) throw (::com::sun::star::uno::RuntimeException){return 0;}
OUString SAL_CALL AccessibleShape::getSelectedText(  ) throw (::com::sun::star::uno::RuntimeException){return OUString();}
sal_Int32 SAL_CALL AccessibleShape::getSelectionStart(  ) throw (::com::sun::star::uno::RuntimeException){return 0;}
sal_Int32 SAL_CALL AccessibleShape::getSelectionEnd(  ) throw (::com::sun::star::uno::RuntimeException){return 0;}
sal_Bool SAL_CALL AccessibleShape::setSelection( sal_Int32, sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException){return sal_True;}
OUString SAL_CALL AccessibleShape::getText(  ) throw (::com::sun::star::uno::RuntimeException){return OUString();}
OUString SAL_CALL AccessibleShape::getTextRange( sal_Int32, sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException){return OUString();}
::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleShape::getTextAtIndex( sal_Int32, sal_Int16 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::accessibility::TextSegment aResult;
    return aResult;
}
::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleShape::getTextBeforeIndex( sal_Int32, sal_Int16 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::accessibility::TextSegment aResult;
    return aResult;
}
::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleShape::getTextBehindIndex( sal_Int32, sal_Int16 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::accessibility::TextSegment aResult;
    return aResult;
}
sal_Bool SAL_CALL AccessibleShape::copyText( sal_Int32, sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException){return sal_True;}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
