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
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>
#include <svx/EnhancedCustomShape2d.hxx>
#include <EnhancedCustomShapeEngine.hxx>
#include "EnhancedCustomShape3d.hxx"
#include "EnhancedCustomShapeFontWork.hxx"
#include "EnhancedCustomShapeHandle.hxx"
#include <svx/unoshape.hxx>
#include <svx/unopage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdogrp.hxx>
#include <editeng/outlobj.hxx>
#include <svl/itemset.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit0.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <com/sun/star/document/XActionLockable.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace css;
using namespace css::uno;

class SdrObject;
class SdrObjCustomShape;

EnhancedCustomShapeEngine::EnhancedCustomShapeEngine(const css::uno::Sequence< css::uno::Any >& aArguments)
    : mpCustomShape(nullptr)
    , mbForceGroupWithText(false)
{
    for (const css::uno::Any& rArg : aArguments)
    {
        beans::PropertyValue aProp;
        if (rArg >>= aProp)
        {
            if ( aProp.Name == "CustomShape" )
            {
                css::uno::Reference<css::drawing::XShape> xShape;
                aProp.Value >>= xShape;
                // the only two subclasses of SdrObject we see here are SdrObjCustomShape and SwDrawVirtObj
                // and we only return useful data for SdrObjCustomShape
                mpCustomShape = dynamic_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape));
            }
            else if ( aProp.Name == "ForceGroupWithText" )
                aProp.Value >>= mbForceGroupWithText;
            else
                assert(false);
        }
        else
            assert(false);
    }
}

EnhancedCustomShapeEngine::EnhancedCustomShapeEngine(SdrObjCustomShape& rShape)
    : mpCustomShape(&rShape)
    , mbForceGroupWithText(false)
{
}

// XInterface
void SAL_CALL EnhancedCustomShapeEngine::acquire() noexcept
{
    OWeakObject::acquire();
}
void SAL_CALL EnhancedCustomShapeEngine::release() noexcept
{
    OWeakObject::release();
}

// XServiceInfo
OUString SAL_CALL EnhancedCustomShapeEngine::getImplementationName()
{
    return u"com.sun.star.drawing.EnhancedCustomShapeEngine"_ustr;
}
sal_Bool SAL_CALL EnhancedCustomShapeEngine::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL EnhancedCustomShapeEngine::getSupportedServiceNames()
{
    return { u"com.sun.star.drawing.CustomShapeEngine"_ustr };
}

// XCustomShapeEngine
rtl::Reference<SdrObject> EnhancedCustomShapeEngine::ImplForceGroupWithText(
    SdrObjCustomShape& rSdrObjCustomShape,
    SdrObject* pRenderedShape1)
{
    rtl::Reference<SdrObject> pRenderedShape = pRenderedShape1;
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
                    pRenderedShape = new SdrObjGroup(rSdrObjCustomShape.getSdrModelFromSdrObject());
                    static_cast<SdrObjGroup*>(pRenderedShape.get())->GetSubList()->NbcInsertObject( pTmp.get() );
                }

                static_cast<SdrObjGroup*>(pRenderedShape.get())->GetSubList()->NbcInsertObject(
                    pShadowGeometry->CloneSdrObject(pShadowGeometry->getSdrModelFromSdrObject()).get(),
                    0);
            }
            else
            {
                pRenderedShape = pShadowGeometry->CloneSdrObject(pShadowGeometry->getSdrModelFromSdrObject());
            }
        }

        // apply text
        if ( bHasText )
        {
            // #i37011# also create a text object and add at rPos + 1
            rtl::Reference<SdrObject> pTextObj( SdrObjFactory::MakeNewObject(
                rSdrObjCustomShape.getSdrModelFromSdrObject(),
                rSdrObjCustomShape.GetObjInventor(),
                SdrObjKind::Text) );

            // Copy text content
            OutlinerParaObject* pParaObj(rSdrObjCustomShape.GetOutlinerParaObject());

            if( pParaObj )
                pTextObj->NbcSetOutlinerParaObject( *pParaObj );

            // copy all attributes
            SfxItemSet aTargetItemSet(rSdrObjCustomShape.GetMergedItemSet());

            // clear fill and line style
            aTargetItemSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
            aTargetItemSet.Put(XFillStyleItem(drawing::FillStyle_NONE));

            // get the text bounds and set at text object
            tools::Rectangle aTextBounds(rSdrObjCustomShape.GetSnapRect());
            EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
            aTextBounds = aCustomShape2d.GetTextRect();

            pTextObj->SetSnapRect( aTextBounds );

            // if rotated, copy GeoStat, too.
            const GeoStat& rSourceGeo(rSdrObjCustomShape.GetGeoStat());
            if ( rSourceGeo.m_nRotationAngle )
            {
                pTextObj->NbcRotate(
                    rSdrObjCustomShape.GetSnapRect().Center(),
                    rSourceGeo.m_nRotationAngle,
                    rSourceGeo.mfSinRotationAngle,
                    rSourceGeo.mfCosRotationAngle);
            }

            // set modified ItemSet at text object
            pTextObj->SetMergedItemSet(aTargetItemSet);

            if ( pRenderedShape )
            {
                if ( dynamic_cast<const SdrObjGroup*>( pRenderedShape.get() ) == nullptr )
                {
                    auto pTmp = std::move(pRenderedShape);
                    pRenderedShape = new SdrObjGroup(rSdrObjCustomShape.getSdrModelFromSdrObject());
                    static_cast<SdrObjGroup*>(pRenderedShape.get())->GetSubList()->NbcInsertObject( pTmp.get() );
                }
                static_cast<SdrObjGroup*>(pRenderedShape.get())->GetSubList()->NbcInsertObject( pTextObj.get() );
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
                pRenderedShape = new SdrObjGroup(rSdrObjCustomShape.getSdrModelFromSdrObject());
                static_cast<SdrObjGroup*>(pRenderedShape.get())->GetSubList()->NbcInsertObject( pTmp.get() );
            }
        }
    }

    return pRenderedShape;
}

Reference< drawing::XShape > SAL_CALL EnhancedCustomShapeEngine::render()
{
    rtl::Reference<SdrObject> pSdrObj = render2();
    if (!pSdrObj)
        return {};
    return SvxDrawPage::CreateShapeByTypeAndInventor( pSdrObj->GetObjIdentifier(),
        pSdrObj->GetObjInventor(), pSdrObj.get() );
}

rtl::Reference<SdrObject> EnhancedCustomShapeEngine::render2() const
{
    if (!mpCustomShape)
        return {};

    // retrieving the TextPath property to check if feature is enabled
    const SdrCustomShapeGeometryItem& rGeometryItem(mpCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
    bool bTextPathOn = false;
    const uno::Any* pAny = rGeometryItem.GetPropertyValueByName( u"TextPath"_ustr, u"TextPath"_ustr );
    if ( pAny )
        *pAny >>= bTextPathOn;

    EnhancedCustomShape2d aCustomShape2d(*mpCustomShape);
    Degree100 nRotateAngle = aCustomShape2d.GetRotateAngle();

    bool bFlipV = aCustomShape2d.IsFlipVert();
    bool bFlipH = aCustomShape2d.IsFlipHorz();
    bool bLineGeometryNeededOnly = bTextPathOn;

    rtl::Reference<SdrObject> xRenderedShape(aCustomShape2d.CreateObject(bLineGeometryNeededOnly, mpCustomShape->GetStyleSheet()));
    if (xRenderedShape)
    {
        if ( bTextPathOn )
        {
            rtl::Reference<SdrObject> xRenderedFontWork(
                EnhancedCustomShapeFontWork::CreateFontWork(
                    xRenderedShape.get(),
                    *mpCustomShape));

            if (xRenderedFontWork)
            {
                xRenderedShape = std::move(xRenderedFontWork);
            }
        }
        rtl::Reference<SdrObject> xRenderedShape3d(EnhancedCustomShape3d::Create3DObject(xRenderedShape.get(), *mpCustomShape));
        if (xRenderedShape3d)
        {
            bFlipV = bFlipH = false;
            nRotateAngle = 0_deg100;
            xRenderedShape = std::move(xRenderedShape3d);
        }

        tools::Rectangle aRect(mpCustomShape->GetSnapRect());
        const GeoStat& rGeoStat(mpCustomShape->GetGeoStat());

        if ( rGeoStat.m_nShearAngle )
        {
            Degree100 nShearAngle = rGeoStat.m_nShearAngle;
            double nTan = rGeoStat.mfTanShearAngle;
            if (bFlipV != bFlipH)
            {
                nShearAngle = -nShearAngle;
                nTan = -nTan;
            }

            xRenderedShape->NbcShear(mpCustomShape->GetSnapRect().Center(), nShearAngle, nTan, false);
        }
        if(nRotateAngle )
            xRenderedShape->NbcRotate(mpCustomShape->GetSnapRect().Center(), nRotateAngle);
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

        xRenderedShape->RecalcSnapRect();
    }

    if ( mbForceGroupWithText )
    {
        xRenderedShape = ImplForceGroupWithText(
            *mpCustomShape,
            xRenderedShape.get());
    }

    if (!xRenderedShape)
        return nullptr;

    aCustomShape2d.ApplyGluePoints(xRenderedShape.get());

    return xRenderedShape;
}

tools::Rectangle EnhancedCustomShapeEngine::getTextBounds() const
{
    if (!mpCustomShape)
        return tools::Rectangle();

    uno::Reference< document::XActionLockable > xLockable( mpCustomShape->getUnoShape(), uno::UNO_QUERY );
    if(!xLockable.is() || xLockable->isActionLocked())
        return tools::Rectangle();

    EnhancedCustomShape2d aCustomShape2d(*mpCustomShape);
    return aCustomShape2d.GetTextRect();
}

basegfx::B2DPolyPolygon EnhancedCustomShapeEngine::getB2DLineGeometry() const
{
    if (!mpCustomShape)
        return basegfx::B2DPolyPolygon();

    EnhancedCustomShape2d aCustomShape2d(*mpCustomShape);
    rtl::Reference<SdrObject> pObj = aCustomShape2d.CreateLineGeometry();

    if ( !pObj )
        return basegfx::B2DPolyPolygon();

    tools::Rectangle aRect(mpCustomShape->GetSnapRect());
    bool bFlipV = aCustomShape2d.IsFlipVert();
    bool bFlipH = aCustomShape2d.IsFlipHorz();
    const GeoStat& rGeoStat(mpCustomShape->GetGeoStat());

    if ( rGeoStat.m_nShearAngle )
    {
        Degree100 nShearAngle = rGeoStat.m_nShearAngle;
        double nTan = rGeoStat.mfTanShearAngle;
        if (bFlipV != bFlipH)
        {
            nShearAngle = -nShearAngle;
            nTan = -nTan;
        }
        pObj->Shear( aRect.Center(), nShearAngle, nTan, false);
    }
    Degree100 nRotateAngle = aCustomShape2d.GetRotateAngle();
    if( nRotateAngle )
        pObj->NbcRotate( aRect.Center(), nRotateAngle );
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
        basegfx::B2DPolyPolygon aPP;
        const SdrObject* pNext = aIter.Next();

        if ( auto pPathObj = dynamic_cast<const SdrPathObj*>(pNext) )
        {
            aPP = pPathObj->GetPathPoly();
        }
        else
        {
            rtl::Reference<SdrObject> pNewObj = pNext->ConvertToPolyObj( false, false );
            SdrPathObj* pPath = dynamic_cast<SdrPathObj*>( pNewObj.get() );
            if ( pPath )
                aPP = pPath->GetPathPoly();
        }

        if ( aPP.count() )
            aPolyPolygon.append(aPP);
    }
    pObj.clear();

    return aPolyPolygon;
}

std::vector< Reference< drawing::XCustomShapeHandle > > EnhancedCustomShapeEngine::getInteraction()
{
    if (!mpCustomShape)
        return {};

    EnhancedCustomShape2d aCustomShape2d(*mpCustomShape);
    sal_uInt32 nHdlCount = aCustomShape2d.GetHdlCount();

    std::vector< Reference< drawing::XCustomShapeHandle > > aVec;
    aVec.reserve( nHdlCount );

    for ( sal_uInt32 i = 0; i < nHdlCount; i++ )
        aVec.push_back(new EnhancedCustomShapeHandle( mpCustomShape, i ));
    return aVec;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_drawing_EnhancedCustomShapeEngine_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const & args)
{
    return cppu::acquire(new EnhancedCustomShapeEngine(args));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
