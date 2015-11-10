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

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>
#include <rtl/ref.hxx>
#include "svx/EnhancedCustomShape2d.hxx"
#include "EnhancedCustomShape3d.hxx"
#include "EnhancedCustomShapeFontWork.hxx"
#include "EnhancedCustomShapeHandle.hxx"
#include "svx/EnhancedCustomShapeGeometry.hxx"
#include <svx/unoshape.hxx>
#include "svx/unopage.hxx"
#include "svx/unoapi.hxx"
#include <svx/svdobj.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdorect.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdoutl.hxx>
#include <svl/itemset.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include "svx/svditer.hxx"
#include <uno/mapping.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/tools/unotools.hxx>
#include <com/sun/star/document/XActionLockable.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace css;
using namespace css::uno;

class SdrObject;
class SdrObjCustomShape;

namespace {

class EnhancedCustomShapeEngine : public cppu::WeakImplHelper
<
    css::lang::XInitialization,
    css::lang::XServiceInfo,
    css::drawing::XCustomShapeEngine
>
{
    css::uno::Reference< css::drawing::XShape >      mxShape;
    bool                                    mbForceGroupWithText;

    SdrObject* ImplForceGroupWithText( const SdrObjCustomShape* pCustoObj, SdrObject* pRenderedShape );

public:
                            EnhancedCustomShapeEngine();
    virtual                 ~EnhancedCustomShapeEngine();

    // XInterface
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName )
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XCustomShapeEngine
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL render()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::awt::Rectangle SAL_CALL getTextBounds()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::drawing::PolyPolygonBezierCoords SAL_CALL getLineGeometry()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::drawing::XCustomShapeHandle > > SAL_CALL getInteraction()
        throw ( css::uno::RuntimeException, std::exception ) override;
};

EnhancedCustomShapeEngine::EnhancedCustomShapeEngine() :
    mbForceGroupWithText    ( false )
{
}
EnhancedCustomShapeEngine::~EnhancedCustomShapeEngine()
{
}

// XInterface
void SAL_CALL EnhancedCustomShapeEngine::acquire() throw()
{
    OWeakObject::acquire();
}
void SAL_CALL EnhancedCustomShapeEngine::release() throw()
{
    OWeakObject::release();
}

// XInitialization
void SAL_CALL EnhancedCustomShapeEngine::initialize( const Sequence< Any >& aArguments )
    throw ( Exception, RuntimeException, std::exception )
{
    sal_Int32 i;
    Sequence< beans::PropertyValue > aParameter;
    for ( i = 0; i < aArguments.getLength(); i++ )
    {
        if ( aArguments[ i ] >>= aParameter )
            break;
    }
    for ( i = 0; i < aParameter.getLength(); i++ )
    {
        const beans::PropertyValue& rProp = aParameter[ i ];
        if ( rProp.Name == "CustomShape" )
            rProp.Value >>= mxShape;
        else if ( rProp.Name == "ForceGroupWithText" )
            rProp.Value >>= mbForceGroupWithText;
    }
}

// XServiceInfo
OUString SAL_CALL EnhancedCustomShapeEngine::getImplementationName()
    throw( RuntimeException, std::exception )
{
    return OUString( "com.sun.star.drawing.EnhancedCustomShapeEngine" );
}
sal_Bool SAL_CALL EnhancedCustomShapeEngine::supportsService( const OUString& rServiceName )
    throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL EnhancedCustomShapeEngine::getSupportedServiceNames()
    throw ( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.drawing.CustomShapeEngine";
    return aRet;
}

// XCustomShapeEngine
SdrObject* EnhancedCustomShapeEngine::ImplForceGroupWithText( const SdrObjCustomShape* pCustoObj, SdrObject* pRenderedShape )
{
    bool bHasText = pCustoObj->HasText();
    if ( pRenderedShape || bHasText )
    {
        // applying shadow
        const SdrObject* pShadowGeometry = pCustoObj->GetSdrObjectShadowFromCustomShape();
        if ( pShadowGeometry )
        {
            if ( pRenderedShape )
            {
                if ( dynamic_cast<const SdrObjGroup*>( pRenderedShape) ==  nullptr )
                {
                    SdrObject* pTmp = pRenderedShape;
                    pRenderedShape = new SdrObjGroup();
                    static_cast<SdrObjGroup*>(pRenderedShape)->GetSubList()->NbcInsertObject( pTmp );
                }
                static_cast<SdrObjGroup*>(pRenderedShape)->GetSubList()->NbcInsertObject( pShadowGeometry->Clone(), 0 );
            }
            else
                pRenderedShape = pShadowGeometry->Clone();
        }

        // apply text
        if ( bHasText )
        {
            // #i37011# also create a text object and add at rPos + 1
            SdrObject* pTextObj = SdrObjFactory::MakeNewObject(
                pCustoObj->GetObjInventor(), OBJ_TEXT, nullptr, pCustoObj->GetModel());

            // Copy text content
            OutlinerParaObject* pParaObj = pCustoObj->GetOutlinerParaObject();
            if( pParaObj )
                pTextObj->NbcSetOutlinerParaObject( new OutlinerParaObject(*pParaObj) );

            // copy all attributes
            SfxItemSet aTargetItemSet( pCustoObj->GetMergedItemSet() );

            // clear fill and line style
            aTargetItemSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
            aTargetItemSet.Put(XFillStyleItem(drawing::FillStyle_NONE));

            // get the text bounds and set at text object
            Rectangle aTextBounds = pCustoObj->GetSnapRect();
            SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxShape ) );
            if ( pSdrObjCustomShape )
            {
                EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
                aTextBounds = aCustomShape2d.GetTextRect();
            }
            pTextObj->SetSnapRect( aTextBounds );

            // if rotated, copy GeoStat, too.
            const GeoStat& rSourceGeo = pCustoObj->GetGeoStat();
            if ( rSourceGeo.nRotationAngle )
            {
                pTextObj->NbcRotate(
                    pCustoObj->GetSnapRect().Center(), rSourceGeo.nRotationAngle,
                    rSourceGeo.nSin, rSourceGeo.nCos);
            }

            // set modified ItemSet at text object
            pTextObj->SetMergedItemSet(aTargetItemSet);

            if ( pRenderedShape )
            {
                if ( dynamic_cast<const SdrObjGroup*>( pRenderedShape) ==  nullptr )
                {
                    SdrObject* pTmp = pRenderedShape;
                    pRenderedShape = new SdrObjGroup();
                    static_cast<SdrObjGroup*>(pRenderedShape)->GetSubList()->NbcInsertObject( pTmp );
                }
                static_cast<SdrObjGroup*>(pRenderedShape)->GetSubList()->NbcInsertObject( pTextObj );
            }
            else
                pRenderedShape = pTextObj;
        }

        // force group
        if ( pRenderedShape )
        {
            if ( dynamic_cast<const SdrObjGroup*>( pRenderedShape) ==  nullptr )
            {
                SdrObject* pTmp = pRenderedShape;
                pRenderedShape = new SdrObjGroup();
                static_cast<SdrObjGroup*>(pRenderedShape)->GetSubList()->NbcInsertObject( pTmp );
            }
            pRenderedShape->SetPage( pCustoObj->GetPage() );
            pRenderedShape->SetModel( pCustoObj->GetModel() );
        }
    }
    return pRenderedShape;
}

void SetTemporary( uno::Reference< drawing::XShape >& xShape )
{
    if ( xShape.is() )
    {
        SvxShape* pShape = SvxShape::getImplementation( xShape );
        if ( pShape )
            pShape->TakeSdrObjectOwnership();
    }
}

Reference< drawing::XShape > SAL_CALL EnhancedCustomShapeEngine::render()
    throw ( RuntimeException, std::exception )
{
    Reference< drawing::XShape > xShape;
    SdrObject* pSdrObjCustomShape( dynamic_cast<SdrObjCustomShape*>( GetSdrObjectFromXShape( mxShape ) )  );
    if ( pSdrObjCustomShape )
    {
        // retrieving the TextPath property to check if feature is enabled
        const SdrCustomShapeGeometryItem& rGeometryItem = static_cast<const SdrCustomShapeGeometryItem&>(
            pSdrObjCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
        bool bTextPathOn = false;
        const uno::Any* pAny = rGeometryItem.GetPropertyValueByName( "TextPath", "TextPath" );
        if ( pAny )
            *pAny >>= bTextPathOn;

        EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
        sal_Int32 nRotateAngle = aCustomShape2d.GetRotateAngle();
        bool bPostRotateAngle = aCustomShape2d.IsPostRotate();

        bool bFlipV = aCustomShape2d.IsFlipVert();
        bool bFlipH = aCustomShape2d.IsFlipHorz();
        bool bLineGeometryNeededOnly = bTextPathOn;

        SdrObject* pRenderedShape = aCustomShape2d.CreateObject( bLineGeometryNeededOnly );
        if ( pRenderedShape )
        {
            if ( bTextPathOn )
            {
                SdrObject* pRenderedFontWork = EnhancedCustomShapeFontWork::CreateFontWork( pRenderedShape, pSdrObjCustomShape );
                if ( pRenderedFontWork )
                {
                    SdrObject::Free( pRenderedShape );
                    pRenderedShape = pRenderedFontWork;
                }
            }
            SdrObject* pRenderedShape3d = EnhancedCustomShape3d::Create3DObject( pRenderedShape, pSdrObjCustomShape );
            if ( pRenderedShape3d )
            {
                bFlipV = bFlipH = false;
                nRotateAngle = 0;
                SdrObject::Free( pRenderedShape );
                pRenderedShape = pRenderedShape3d;
            }
            Rectangle aRect( pSdrObjCustomShape->GetSnapRect() );

            const GeoStat& rGeoStat = static_cast<SdrObjCustomShape*>(pSdrObjCustomShape)->GetGeoStat();
            if ( rGeoStat.nShearAngle )
            {
                long nShearAngle = rGeoStat.nShearAngle;
                double nTan = rGeoStat.nTan;
                if ((bFlipV&&!bFlipH )||(bFlipH&&!bFlipV))
                {
                    nShearAngle = -nShearAngle;
                    nTan = -nTan;
                }
                pRenderedShape->Shear( pSdrObjCustomShape->GetSnapRect().Center(), nShearAngle, nTan, false);
            }
            if( !bPostRotateAngle && nRotateAngle )
            {
                double a = nRotateAngle * F_PI18000;
                pRenderedShape->NbcRotate( pSdrObjCustomShape->GetSnapRect().Center(), nRotateAngle, sin( a ), cos( a ) );
            }
            if ( bFlipV )
            {
                Point aLeft( aRect.Left(), ( aRect.Top() + aRect.Bottom() ) >> 1 );
                Point aRight( aLeft.X() + 1000, aLeft.Y() );
                pRenderedShape->NbcMirror( aLeft, aRight );
            }
            if ( bFlipH )
            {
                Point aTop( ( aRect.Left() + aRect.Right() ) >> 1, aRect.Top() );
                Point aBottom( aTop.X(), aTop.Y() + 1000 );
                pRenderedShape->NbcMirror( aTop, aBottom );
            }
            // Specifically for pptx imports
            if( bPostRotateAngle && nRotateAngle )
            {
                double a = nRotateAngle * F_PI18000;
                pRenderedShape->NbcRotate( pSdrObjCustomShape->GetSnapRect().Center(), nRotateAngle, sin( a ), cos( a ) );
            }
            pRenderedShape->NbcSetStyleSheet( pSdrObjCustomShape->GetStyleSheet(), true );
            pRenderedShape->RecalcSnapRect();
        }

        if ( mbForceGroupWithText )
            pRenderedShape = ImplForceGroupWithText( static_cast<SdrObjCustomShape*>(pSdrObjCustomShape), pRenderedShape );

        if ( pRenderedShape )
        {
            aCustomShape2d.ApplyGluePoints( pRenderedShape );
            xShape = SvxDrawPage::CreateShapeByTypeAndInventor( pRenderedShape->GetObjIdentifier(),
                pRenderedShape->GetObjInventor(), pRenderedShape );
        }
        SetTemporary( xShape );
    }
    return xShape;
}

awt::Rectangle SAL_CALL EnhancedCustomShapeEngine::getTextBounds()
    throw ( RuntimeException, std::exception )
{
    awt::Rectangle aTextRect;
    SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxShape ) );
    uno::Reference< document::XActionLockable > xLockable( mxShape, uno::UNO_QUERY );
    if ( pSdrObjCustomShape && pSdrObjCustomShape->GetModel() && xLockable.is() && !xLockable->isActionLocked() )
    {
        if ( pSdrObjCustomShape )
        {
            EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
            Rectangle aRect( aCustomShape2d.GetTextRect() );
            aTextRect.X = aRect.Left();
            aTextRect.Y = aRect.Top();
            aTextRect.Width = aRect.GetWidth();
            aTextRect.Height = aRect.GetHeight();
        }
    }
    return aTextRect;
}

drawing::PolyPolygonBezierCoords SAL_CALL EnhancedCustomShapeEngine::getLineGeometry()
    throw ( RuntimeException, std::exception )
{
    drawing::PolyPolygonBezierCoords aPolyPolygonBezierCoords;
    SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxShape ) );
    if ( pSdrObjCustomShape )
    {
        EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
        SdrObject* pObj = aCustomShape2d.CreateLineGeometry();
        if ( pObj )
        {
            Rectangle aRect( pSdrObjCustomShape->GetSnapRect() );
            bool bFlipV = aCustomShape2d.IsFlipVert();
            bool bFlipH = aCustomShape2d.IsFlipHorz();

            const GeoStat& rGeoStat = static_cast<SdrObjCustomShape*>(pSdrObjCustomShape)->GetGeoStat();
            if ( rGeoStat.nShearAngle )
            {
                long nShearAngle = rGeoStat.nShearAngle;
                double nTan = rGeoStat.nTan;
                if ((bFlipV&&!bFlipH )||(bFlipH&&!bFlipV))
                {
                    nShearAngle = -nShearAngle;
                    nTan = -nTan;
                }
                pObj->Shear( aRect.Center(), nShearAngle, nTan, false);
            }
            sal_Int32 nRotateAngle = aCustomShape2d.GetRotateAngle();
            if( nRotateAngle )
            {
                double a = nRotateAngle * F_PI18000;
                pObj->NbcRotate( aRect.Center(), nRotateAngle, sin( a ), cos( a ) );
            }
            if ( bFlipH )
            {
                Point aTop( ( aRect.Left() + aRect.Right() ) >> 1, aRect.Top() );
                Point aBottom( aTop.X(), aTop.Y() + 1000 );
                pObj->NbcMirror( aTop, aBottom );
            }
            if ( bFlipV )
            {
                Point aLeft( aRect.Left(), ( aRect.Top() + aRect.Bottom() ) >> 1 );
                Point aRight( aLeft.X() + 1000, aLeft.Y() );
                pObj->NbcMirror( aLeft, aRight );
            }

            basegfx::B2DPolyPolygon aPolyPolygon;
            SdrObjListIter aIter( *pObj, IM_DEEPWITHGROUPS );

            while ( aIter.IsMore() )
            {
                SdrObject* pNewObj = nullptr;
                basegfx::B2DPolyPolygon aPP;
                const SdrObject* pNext = aIter.Next();

                if ( dynamic_cast<const SdrPathObj*>( pNext) !=  nullptr )
                {
                    aPP = static_cast<const SdrPathObj*>(pNext)->GetPathPoly();
                }
                else
                {
                    pNewObj = pNext->ConvertToPolyObj( false, false );
                    SdrPathObj* pPath = dynamic_cast<SdrPathObj*>( pNewObj  );
                    if ( pPath )
                        aPP = pPath->GetPathPoly();
                }

                if ( aPP.count() )
                    aPolyPolygon.append(aPP);

                SdrObject::Free( pNewObj );
            }
            SdrObject::Free( pObj );
            basegfx::unotools::b2DPolyPolygonToPolyPolygonBezier( aPolyPolygon,
                                                                  aPolyPolygonBezierCoords );
        }
    }

    return aPolyPolygonBezierCoords;
}

Sequence< Reference< drawing::XCustomShapeHandle > > SAL_CALL EnhancedCustomShapeEngine::getInteraction()
    throw ( RuntimeException, std::exception )
{
    sal_uInt32 i, nHdlCount = 0;
    SdrObject* pSdrObjCustomShape = GetSdrObjectFromXShape( mxShape );
    if ( pSdrObjCustomShape )
    {
        EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
        nHdlCount = aCustomShape2d.GetHdlCount();
    }
    Sequence< Reference< drawing::XCustomShapeHandle > > aSeq( nHdlCount );
    for ( i = 0; i < nHdlCount; i++ )
        aSeq[ i ] = new EnhancedCustomShapeHandle( mxShape, i );
    return aSeq;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_drawing_EnhancedCustomShapeEngine_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new EnhancedCustomShapeEngine);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
