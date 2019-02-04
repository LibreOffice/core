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
#include <svx/EnhancedCustomShape2d.hxx>
#include "EnhancedCustomShape3d.hxx"
#include "EnhancedCustomShapeFontWork.hxx"
#include "EnhancedCustomShapeHandle.hxx"
#include <svx/EnhancedCustomShapeGeometry.hxx>
#include <svx/unoshape.hxx>
#include <svx/unopage.hxx>
#include <svx/unoapi.hxx>
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
#include <svx/svditer.hxx>
#include <uno/mapping.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
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

    std::unique_ptr<SdrObject, SdrObjectFreeOp> ImplForceGroupWithText(
        const SdrObjCustomShape& rSdrObjCustomShape,
        std::unique_ptr<SdrObject, SdrObjectFreeOp> pRenderedShape);

public:
                            EnhancedCustomShapeEngine();

    // XInterface
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XCustomShapeEngine
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL render() override;
    virtual css::awt::Rectangle SAL_CALL getTextBounds() override;
    virtual css::drawing::PolyPolygonBezierCoords SAL_CALL getLineGeometry() override;
    virtual css::uno::Sequence< css::uno::Reference< css::drawing::XCustomShapeHandle > > SAL_CALL getInteraction() override;
};

EnhancedCustomShapeEngine::EnhancedCustomShapeEngine() :
    mbForceGroupWithText    ( false )
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
{
    return OUString( "com.sun.star.drawing.EnhancedCustomShapeEngine" );
}
sal_Bool SAL_CALL EnhancedCustomShapeEngine::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL EnhancedCustomShapeEngine::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.drawing.CustomShapeEngine" };
    return aRet;
}

// XCustomShapeEngine
std::unique_ptr<SdrObject, SdrObjectFreeOp> EnhancedCustomShapeEngine::ImplForceGroupWithText(
    const SdrObjCustomShape& rSdrObjCustomShape,
    std::unique_ptr<SdrObject, SdrObjectFreeOp> pRenderedShape)
{
    const bool bHasText(rSdrObjCustomShape.HasText());

    if ( pRenderedShape || bHasText )
    {
        // applying shadow
        const SdrObject* pShadowGeometry(rSdrObjCustomShape.GetSdrObjectShadowFromCustomShape());

        if ( pShadowGeometry )
        {
            if ( pRenderedShape )
            {
                if ( dynamic_cast<const SdrObjGroup*>( pRenderedShape.get() ) ==  nullptr )
                {
                    auto pTmp = std::move(pRenderedShape);
                    pRenderedShape.reset(new SdrObjGroup(rSdrObjCustomShape.getSdrModelFromSdrObject()));
                    static_cast<SdrObjGroup*>(pRenderedShape.get())->GetSubList()->NbcInsertObject( pTmp.release() );
                }

                static_cast<SdrObjGroup*>(pRenderedShape.get())->GetSubList()->NbcInsertObject(
                    pShadowGeometry->CloneSdrObject(pShadowGeometry->getSdrModelFromSdrObject()),
                    0);
            }
            else
            {
                pRenderedShape.reset( pShadowGeometry->CloneSdrObject(pShadowGeometry->getSdrModelFromSdrObject()) );
            }
        }

        // apply text
        if ( bHasText )
        {
            // #i37011# also create a text object and add at rPos + 1
            std::unique_ptr<SdrObject, SdrObjectFreeOp> pTextObj( SdrObjFactory::MakeNewObject(
                rSdrObjCustomShape.getSdrModelFromSdrObject(),
                rSdrObjCustomShape.GetObjInventor(),
                OBJ_TEXT) );

            // Copy text content
            OutlinerParaObject* pParaObj(rSdrObjCustomShape.GetOutlinerParaObject());

            if( pParaObj )
                pTextObj->NbcSetOutlinerParaObject( std::make_unique<OutlinerParaObject>(*pParaObj) );

            // copy all attributes
            SfxItemSet aTargetItemSet(rSdrObjCustomShape.GetMergedItemSet());

            // clear fill and line style
            aTargetItemSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
            aTargetItemSet.Put(XFillStyleItem(drawing::FillStyle_NONE));

            // get the text bounds and set at text object
            tools::Rectangle aTextBounds(rSdrObjCustomShape.GetSnapRect());
            const bool bIsSdrObjCustomShape(nullptr != dynamic_cast< SdrObjCustomShape* >(GetSdrObjectFromXShape(mxShape)));

            if(bIsSdrObjCustomShape)
            {
                SdrObjCustomShape& rSdrObjCustomShape2(
                    static_cast< SdrObjCustomShape& >(
                        *GetSdrObjectFromXShape(mxShape)));
                EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape2);
                aTextBounds = aCustomShape2d.GetTextRect();
            }

            pTextObj->SetSnapRect( aTextBounds );

            // if rotated, copy GeoStat, too.
            const GeoStat& rSourceGeo(rSdrObjCustomShape.GetGeoStat());
            if ( rSourceGeo.nRotationAngle )
            {
                pTextObj->NbcRotate(
                    rSdrObjCustomShape.GetSnapRect().Center(),
                    rSourceGeo.nRotationAngle,
                    rSourceGeo.nSin,
                    rSourceGeo.nCos);
            }

            // set modified ItemSet at text object
            pTextObj->SetMergedItemSet(aTargetItemSet);

            if ( pRenderedShape )
            {
                if ( dynamic_cast<const SdrObjGroup*>( pRenderedShape.get() ) == nullptr )
                {
                    auto pTmp = std::move(pRenderedShape);
                    pRenderedShape.reset(new SdrObjGroup(rSdrObjCustomShape.getSdrModelFromSdrObject()));
                    static_cast<SdrObjGroup*>(pRenderedShape.get())->GetSubList()->NbcInsertObject( pTmp.release() );
                }
                static_cast<SdrObjGroup*>(pRenderedShape.get())->GetSubList()->NbcInsertObject( pTextObj.release() );
            }
            else
                pRenderedShape = std::move(pTextObj);
        }

        // force group
        if ( pRenderedShape )
        {
            if ( dynamic_cast<const SdrObjGroup*>( pRenderedShape.get() ) ==  nullptr )
            {
                auto pTmp = std::move(pRenderedShape);
                pRenderedShape.reset(new SdrObjGroup(rSdrObjCustomShape.getSdrModelFromSdrObject()));
                static_cast<SdrObjGroup*>(pRenderedShape.get())->GetSubList()->NbcInsertObject( pTmp.release() );
            }
        }
    }

    return pRenderedShape;
}

void SetTemporary( uno::Reference< drawing::XShape > const & xShape )
{
    if ( xShape.is() )
    {
        SvxShape* pShape = SvxShape::getImplementation( xShape );
        if ( pShape )
            pShape->TakeSdrObjectOwnership();
    }
}

Reference< drawing::XShape > SAL_CALL EnhancedCustomShapeEngine::render()
{
    const bool bIsSdrObjCustomShape(nullptr != dynamic_cast< SdrObjCustomShape* >(GetSdrObjectFromXShape(mxShape)));

    if(!bIsSdrObjCustomShape)
    {
        return Reference< drawing::XShape >();
    }

    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast< SdrObjCustomShape& >(
            *GetSdrObjectFromXShape(mxShape)));

    // retrieving the TextPath property to check if feature is enabled
    const SdrCustomShapeGeometryItem& rGeometryItem(rSdrObjCustomShape.GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
    bool bTextPathOn = false;
    const uno::Any* pAny = rGeometryItem.GetPropertyValueByName( "TextPath", "TextPath" );
    if ( pAny )
        *pAny >>= bTextPathOn;

    EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
    sal_Int32 nRotateAngle = aCustomShape2d.GetRotateAngle();

    bool bFlipV = aCustomShape2d.IsFlipVert();
    bool bFlipH = aCustomShape2d.IsFlipHorz();
    bool bLineGeometryNeededOnly = bTextPathOn;

    std::unique_ptr<SdrObject, SdrObjectFreeOp> xRenderedShape(aCustomShape2d.CreateObject(bLineGeometryNeededOnly));
    if (xRenderedShape)
    {
        if ( bTextPathOn )
        {
            std::unique_ptr<SdrObject, SdrObjectFreeOp> xRenderedFontWork(
                EnhancedCustomShapeFontWork::CreateFontWork(
                    xRenderedShape.get(),
                    rSdrObjCustomShape));

            if (xRenderedFontWork)
            {
                xRenderedShape = std::move(xRenderedFontWork);
            }
        }
        std::unique_ptr<SdrObject, SdrObjectFreeOp> xRenderedShape3d(EnhancedCustomShape3d::Create3DObject(xRenderedShape.get(), rSdrObjCustomShape));
        if (xRenderedShape3d)
        {
            bFlipV = bFlipH = false;
            nRotateAngle = 0;
            xRenderedShape = std::move(xRenderedShape3d);
        }

        tools::Rectangle aRect(rSdrObjCustomShape.GetSnapRect());
        const GeoStat& rGeoStat(rSdrObjCustomShape.GetGeoStat());

        if ( rGeoStat.nShearAngle )
        {
            long nShearAngle = rGeoStat.nShearAngle;
            double nTan = rGeoStat.nTan;
            if (bFlipV != bFlipH)
            {
                nShearAngle = -nShearAngle;
                nTan = -nTan;
            }

            xRenderedShape->Shear(rSdrObjCustomShape.GetSnapRect().Center(), nShearAngle, nTan, false);
        }
        if(nRotateAngle )
        {
            double a = nRotateAngle * F_PI18000;

            xRenderedShape->NbcRotate(rSdrObjCustomShape.GetSnapRect().Center(), nRotateAngle, sin( a ), cos( a ));
        }
        if ( bFlipV )
        {
            Point aLeft( aRect.Left(), ( aRect.Top() + aRect.Bottom() ) >> 1 );
            Point aRight( aLeft.X() + 1000, aLeft.Y() );
            xRenderedShape->NbcMirror( aLeft, aRight );
        }
        if ( bFlipH )
        {
            Point aTop( ( aRect.Left() + aRect.Right() ) >> 1, aRect.Top() );
            Point aBottom( aTop.X(), aTop.Y() + 1000 );
            xRenderedShape->NbcMirror( aTop, aBottom );
        }

        xRenderedShape->NbcSetStyleSheet(rSdrObjCustomShape.GetStyleSheet(), true);
        xRenderedShape->RecalcSnapRect();
    }

    if ( mbForceGroupWithText )
    {
        xRenderedShape = ImplForceGroupWithText(
            rSdrObjCustomShape,
            std::move(xRenderedShape));
    }

    Reference< drawing::XShape > xShape;

    if (xRenderedShape)
    {
        aCustomShape2d.ApplyGluePoints(xRenderedShape.get());
        SdrObject* pRenderedShape = xRenderedShape.release();
        xShape = SvxDrawPage::CreateShapeByTypeAndInventor( pRenderedShape->GetObjIdentifier(),
            pRenderedShape->GetObjInventor(), pRenderedShape );
    }

    SetTemporary( xShape );

    return xShape;
}

awt::Rectangle SAL_CALL EnhancedCustomShapeEngine::getTextBounds()
{
    awt::Rectangle aTextRect;
    const bool bIsSdrObjCustomShape(nullptr != dynamic_cast< SdrObjCustomShape* >(GetSdrObjectFromXShape(mxShape)));

    if(bIsSdrObjCustomShape)
    {
        SdrObjCustomShape& rSdrObjCustomShape(static_cast< SdrObjCustomShape& >(*GetSdrObjectFromXShape(mxShape)));
        uno::Reference< document::XActionLockable > xLockable( mxShape, uno::UNO_QUERY );

        if(xLockable.is() && !xLockable->isActionLocked())
        {
            EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
            tools::Rectangle aRect( aCustomShape2d.GetTextRect() );
            aTextRect.X = aRect.Left();
            aTextRect.Y = aRect.Top();
            aTextRect.Width = aRect.GetWidth();
            aTextRect.Height = aRect.GetHeight();
        }
    }

    return aTextRect;
}

drawing::PolyPolygonBezierCoords SAL_CALL EnhancedCustomShapeEngine::getLineGeometry()
{
    drawing::PolyPolygonBezierCoords aPolyPolygonBezierCoords;
    const bool bIsSdrObjCustomShape(nullptr != dynamic_cast< SdrObjCustomShape* >(GetSdrObjectFromXShape(mxShape)));

    if(bIsSdrObjCustomShape)
    {
        SdrObjCustomShape& rSdrObjCustomShape(
            static_cast< SdrObjCustomShape& >(
                *GetSdrObjectFromXShape(mxShape)));
        EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
        SdrObject* pObj = aCustomShape2d.CreateLineGeometry();

        if ( pObj )
        {
            tools::Rectangle aRect(rSdrObjCustomShape.GetSnapRect());
            bool bFlipV = aCustomShape2d.IsFlipVert();
            bool bFlipH = aCustomShape2d.IsFlipHorz();
            const GeoStat& rGeoStat(rSdrObjCustomShape.GetGeoStat());

            if ( rGeoStat.nShearAngle )
            {
                long nShearAngle = rGeoStat.nShearAngle;
                double nTan = rGeoStat.nTan;
                if (bFlipV != bFlipH)
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
            SdrObjListIter aIter( *pObj, SdrIterMode::DeepWithGroups );

            while ( aIter.IsMore() )
            {
                SdrObject* pNewObj = nullptr;
                basegfx::B2DPolyPolygon aPP;
                const SdrObject* pNext = aIter.Next();

                if ( auto pPathObj = dynamic_cast<const SdrPathObj*>(pNext) )
                {
                    aPP = pPathObj->GetPathPoly();
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
            basegfx::utils::B2DPolyPolygonToUnoPolyPolygonBezierCoords( aPolyPolygon,
                                                                  aPolyPolygonBezierCoords );
        }
    }

    return aPolyPolygonBezierCoords;
}

Sequence< Reference< drawing::XCustomShapeHandle > > SAL_CALL EnhancedCustomShapeEngine::getInteraction()
{
    sal_uInt32 i, nHdlCount = 0;
    const bool bIsSdrObjCustomShape(nullptr != dynamic_cast< SdrObjCustomShape* >(GetSdrObjectFromXShape(mxShape)));

    if(bIsSdrObjCustomShape)
    {
        SdrObjCustomShape& rSdrObjCustomShape(
            static_cast< SdrObjCustomShape& >(
                *GetSdrObjectFromXShape(mxShape)));
        EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
        nHdlCount = aCustomShape2d.GetHdlCount();
    }

    Sequence< Reference< drawing::XCustomShapeHandle > > aSeq( nHdlCount );

    for ( i = 0; i < nHdlCount; i++ )
        aSeq[ i ] = new EnhancedCustomShapeHandle( mxShape, i );
    return aSeq;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_drawing_EnhancedCustomShapeEngine_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new EnhancedCustomShapeEngine);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
