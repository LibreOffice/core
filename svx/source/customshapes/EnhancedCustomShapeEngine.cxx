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

#include "EnhancedCustomShapeEngine.hxx"
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
#include <cppuhelper/supportsservice.hxx>

using namespace css;
using namespace css::uno;

// - EnhancedCustomShapeEngine -
OUString EnhancedCustomShapeEngine_getImplementationName()
    throw( RuntimeException )
{
    return OUString( "com.sun.star.drawing.EnhancedCustomShapeEngine" );
}

Sequence< OUString > SAL_CALL EnhancedCustomShapeEngine_getSupportedServiceNames()
    throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.drawing.CustomShapeEngine";
    return aRet;
}

EnhancedCustomShapeEngine::EnhancedCustomShapeEngine( const Reference< lang::XMultiServiceFactory >& rxMgr ) :
    mxFact                  ( rxMgr ),
    mbForceGroupWithText    ( sal_False )
{
}
EnhancedCustomShapeEngine::~EnhancedCustomShapeEngine()
{
}

// XInterface -----------------------------------------------------------------

void SAL_CALL EnhancedCustomShapeEngine::acquire() throw()
{
    OWeakObject::acquire();
}
void SAL_CALL EnhancedCustomShapeEngine::release() throw()
{
    OWeakObject::release();
}

// XInitialization ------------------------------------------------------------

void SAL_CALL EnhancedCustomShapeEngine::initialize( const Sequence< Any >& aArguments )
    throw ( Exception, RuntimeException )
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

// XServiceInfo ---------------------------------------------------------------
OUString SAL_CALL EnhancedCustomShapeEngine::getImplementationName()
    throw( RuntimeException )
{
    return EnhancedCustomShapeEngine_getImplementationName();
}
sal_Bool SAL_CALL EnhancedCustomShapeEngine::supportsService( const OUString& rServiceName )
    throw( RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL EnhancedCustomShapeEngine::getSupportedServiceNames()
    throw ( RuntimeException )
{
    return EnhancedCustomShapeEngine_getSupportedServiceNames();
}

// XCustomShapeEngine -----------------------------------------------------------
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
                if ( !pRenderedShape->ISA( SdrObjGroup ) )
                {
                    SdrObject* pTmp = pRenderedShape;
                    pRenderedShape = new SdrObjGroup();
                    ((SdrObjGroup*)pRenderedShape)->GetSubList()->NbcInsertObject( pTmp );
                }
                ((SdrObjGroup*)pRenderedShape)->GetSubList()->NbcInsertObject( pShadowGeometry->Clone(), 0 );
            }
            else
                pRenderedShape = pShadowGeometry->Clone();
        }

        // apply text
        if ( bHasText )
        {
            // #i37011# also create a text object and add at rPos + 1
            SdrTextObj* pTextObj = (SdrTextObj*)SdrObjFactory::MakeNewObject(
                pCustoObj->GetObjInventor(), OBJ_TEXT, 0L, pCustoObj->GetModel());

            // Copy text content
            OutlinerParaObject* pParaObj = pCustoObj->GetOutlinerParaObject();
            if( pParaObj )
                pTextObj->NbcSetOutlinerParaObject( new OutlinerParaObject(*pParaObj) );

            // copy all attributes
            SfxItemSet aTargetItemSet( pCustoObj->GetMergedItemSet() );

            // clear fill and line style
            aTargetItemSet.Put(XLineStyleItem(XLINE_NONE));
            aTargetItemSet.Put(XFillStyleItem(XFILL_NONE));

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
            if ( rSourceGeo.nDrehWink )
            {
                pTextObj->NbcRotate(
                    pCustoObj->GetSnapRect().Center(), rSourceGeo.nDrehWink,
                    rSourceGeo.nSin, rSourceGeo.nCos);
            }

            // set modified ItemSet at text object
            pTextObj->SetMergedItemSet(aTargetItemSet);

            if ( pRenderedShape )
            {
                if ( !pRenderedShape->ISA( SdrObjGroup ) )
                {
                    SdrObject* pTmp = pRenderedShape;
                    pRenderedShape = new SdrObjGroup();
                    ((SdrObjGroup*)pRenderedShape)->GetSubList()->NbcInsertObject( pTmp );
                }
                ((SdrObjGroup*)pRenderedShape)->GetSubList()->NbcInsertObject( pTextObj, LIST_APPEND );
            }
            else
                pRenderedShape = pTextObj;
        }

        // force group
        if ( pRenderedShape )
        {
            if ( !pRenderedShape->ISA( SdrObjGroup ) )
            {
                SdrObject* pTmp = pRenderedShape;
                pRenderedShape = new SdrObjGroup();
                ((SdrObjGroup*)pRenderedShape)->GetSubList()->NbcInsertObject( pTmp );
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
    throw ( RuntimeException )
{
    Reference< drawing::XShape > xShape;
    SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxShape ) );
    if ( pSdrObjCustomShape )
    {
        // retrieving the TextPath property to check if feature is enabled
        SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)
            pSdrObjCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
        sal_Bool bTextPathOn = sal_False;
        const OUString sTextPath( "TextPath" );
        uno::Any* pAny = rGeometryItem.GetPropertyValueByName( sTextPath, sTextPath );
        if ( pAny )
            *pAny >>= bTextPathOn;

        EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
        sal_Int32 nRotateAngle = aCustomShape2d.GetRotateAngle();
        bool bPostRotateAngle = aCustomShape2d.IsPostRotate();

        sal_Bool bFlipV = aCustomShape2d.IsFlipVert();
        sal_Bool bFlipH = aCustomShape2d.IsFlipHorz();
        sal_Bool bLineGeometryNeededOnly = bTextPathOn;

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
                bFlipV = bFlipH = sal_False;
                nRotateAngle = 0;
                SdrObject::Free( pRenderedShape );
                pRenderedShape = pRenderedShape3d;
            }
            Rectangle aRect( pSdrObjCustomShape->GetSnapRect() );

            const GeoStat& rGeoStat = ((SdrObjCustomShape*)pSdrObjCustomShape)->GetGeoStat();
            if ( rGeoStat.nShearWink )
            {
                long nShearWink = rGeoStat.nShearWink;
                double nTan = rGeoStat.nTan;
                if ((bFlipV&&!bFlipH )||(bFlipH&&!bFlipV))
                {
                    nShearWink = -nShearWink;
                    nTan = -nTan;
                }
                pRenderedShape->Shear( pSdrObjCustomShape->GetSnapRect().Center(), nShearWink, nTan, sal_False);
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
            pRenderedShape->NbcSetStyleSheet( pSdrObjCustomShape->GetStyleSheet(), sal_True );
            pRenderedShape->RecalcSnapRect();
        }

        if ( mbForceGroupWithText )
            pRenderedShape = ImplForceGroupWithText( (SdrObjCustomShape*)pSdrObjCustomShape, pRenderedShape );

        if ( pRenderedShape )
        {
            aCustomShape2d.ApplyGluePoints( pRenderedShape );
            xShape = SvxDrawPage::CreateShapeByTypeAndInventor( pRenderedShape->GetObjIdentifier(),
                pRenderedShape->GetObjInventor(), pRenderedShape, NULL );
        }
        SetTemporary( xShape );
    }
    return xShape;
}

awt::Rectangle SAL_CALL EnhancedCustomShapeEngine::getTextBounds()
    throw ( RuntimeException )
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
    throw ( RuntimeException )
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
            sal_Bool bFlipV = aCustomShape2d.IsFlipVert();
            sal_Bool bFlipH = aCustomShape2d.IsFlipHorz();

            const GeoStat& rGeoStat = ((SdrObjCustomShape*)pSdrObjCustomShape)->GetGeoStat();
            if ( rGeoStat.nShearWink )
            {
                long nShearWink = rGeoStat.nShearWink;
                double nTan = rGeoStat.nTan;
                if ((bFlipV&&!bFlipH )||(bFlipH&&!bFlipV))
                {
                    nShearWink = -nShearWink;
                    nTan = -nTan;
                }
                pObj->Shear( aRect.Center(), nShearWink, nTan, sal_False);
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
                SdrObject* pNewObj = NULL;
                basegfx::B2DPolyPolygon aPP;
                const SdrObject* pNext = aIter.Next();

                if ( pNext->ISA( SdrPathObj ) )
                {
                    aPP = ((SdrPathObj*)pNext)->GetPathPoly();
                }
                else
                {
                    pNewObj = pNext->ConvertToPolyObj( sal_False, sal_False );
                    SdrPathObj* pPath = PTR_CAST( SdrPathObj, pNewObj );
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
    throw ( RuntimeException )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
