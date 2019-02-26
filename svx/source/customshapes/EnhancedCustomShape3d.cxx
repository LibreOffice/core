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

#include "EnhancedCustomShape3d.hxx"
#include <svx/svdetc.hxx>
#include <svx/svdmodel.hxx>
#include <tools/poly.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoashp.hxx>
#include <svl/poolitem.hxx>
#include <svl/itemset.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xsflclit.hxx>
#include <svx/xit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflclit.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>
#include <svx/svddef.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/extrud3d.hxx>
#include <svx/xflbmtit.hxx>
#include <vcl/svapp.hxx>
#include <svx/xlnclit.hxx>
#include <svx/sdasitm.hxx>
#include <svx/scene3d.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <svx/sdr/properties/properties.hxx>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace {

void GetOrigin( const SdrCustomShapeGeometryItem& rItem, double& rOriginX, double& rOriginY )
{
    css::drawing::EnhancedCustomShapeParameterPair aOriginParaPair;
    const Any* pAny = rItem.GetPropertyValueByName( "Extrusion", "Origin" );
    if ( ! ( pAny && ( *pAny >>= aOriginParaPair ) && ( aOriginParaPair.First.Value >>= rOriginX ) && ( aOriginParaPair.Second.Value >>= rOriginY ) ) )
    {
        rOriginX = 0.50;
        rOriginY =-0.50;
    }
}

void GetRotateAngle( const SdrCustomShapeGeometryItem& rItem, double& rAngleX, double& rAngleY )
{
    css::drawing::EnhancedCustomShapeParameterPair aRotateAngleParaPair;
    const Any* pAny = rItem.GetPropertyValueByName( "Extrusion", "RotateAngle" );
    if ( ! ( pAny && ( *pAny >>= aRotateAngleParaPair ) && ( aRotateAngleParaPair.First.Value >>= rAngleX ) && ( aRotateAngleParaPair.Second.Value >>= rAngleY ) ) )
    {
        rAngleX = 0.0;
        rAngleY = 0.0;
    }
    rAngleX = basegfx::deg2rad(rAngleX);
    rAngleY = basegfx::deg2rad(rAngleY);
}

void GetSkew( const SdrCustomShapeGeometryItem& rItem, double& rSkewAmount, double& rSkewAngle )
{
    css::drawing::EnhancedCustomShapeParameterPair aSkewParaPair;
    const Any* pAny = rItem.GetPropertyValueByName( "Extrusion", "Skew" );
    if ( ! ( pAny && ( *pAny >>= aSkewParaPair ) && ( aSkewParaPair.First.Value >>= rSkewAmount ) && ( aSkewParaPair.Second.Value >>= rSkewAngle ) ) )
    {
        rSkewAmount = 50;
        rSkewAngle = -135;
    }
    rSkewAngle = basegfx::deg2rad(rSkewAngle);
}

void GetExtrusionDepth( const SdrCustomShapeGeometryItem& rItem, const double* pMap, double& rBackwardDepth, double& rForwardDepth )
{
    css::drawing::EnhancedCustomShapeParameterPair aDepthParaPair;
    double fDepth = 0, fFraction = 0;
    const Any* pAny = rItem.GetPropertyValueByName( "Extrusion", "Depth" );
    if ( pAny && ( *pAny >>= aDepthParaPair ) && ( aDepthParaPair.First.Value >>= fDepth ) && ( aDepthParaPair.Second.Value >>= fFraction ) )
    {
        rForwardDepth = fDepth * fFraction;
        rBackwardDepth = fDepth - rForwardDepth;
    }
    else
    {
        rBackwardDepth = 1270;
        rForwardDepth = 0;
    }
    if ( pMap )
    {
        double fMap = *pMap;
        rBackwardDepth *= fMap;
        rForwardDepth *= fMap;
    }
}

double GetDouble( const SdrCustomShapeGeometryItem& rItem, const OUString& rPropertyName, double fDefault )
{
    double fRetValue = fDefault;
    const Any* pAny = rItem.GetPropertyValueByName( "Extrusion", rPropertyName );
    if ( pAny )
        *pAny >>= fRetValue;
    return fRetValue;
}

drawing::ShadeMode GetShadeMode( const SdrCustomShapeGeometryItem& rItem, const drawing::ShadeMode eDefault )
{
    drawing::ShadeMode eRet( eDefault );
    const Any* pAny = rItem.GetPropertyValueByName( "Extrusion", "ShadeMode" );
    if ( pAny )
        *pAny >>= eRet;
    return eRet;
}

bool GetBool( const SdrCustomShapeGeometryItem& rItem, const OUString& rPropertyName, const bool bDefault )
{
    bool bRetValue = bDefault;
    const Any* pAny = rItem.GetPropertyValueByName( "Extrusion", rPropertyName );
    if ( pAny )
        *pAny >>= bRetValue;
    return bRetValue;
}

drawing::Position3D GetPosition3D( const SdrCustomShapeGeometryItem& rItem, const OUString& rPropertyName,
                                    const drawing::Position3D& rDefault, const double* pMap )
{
    drawing::Position3D aRetValue( rDefault );
    const Any* pAny = rItem.GetPropertyValueByName( "Extrusion", rPropertyName );
    if ( pAny )
        *pAny >>= aRetValue;
    if ( pMap )
    {
        aRetValue.PositionX *= *pMap;
        aRetValue.PositionY *= *pMap;
        aRetValue.PositionZ *= *pMap;
    }
    return aRetValue;
}

drawing::Direction3D GetDirection3D( const SdrCustomShapeGeometryItem& rItem, const OUString& rPropertyName, const drawing::Direction3D& rDefault )
{
    drawing::Direction3D aRetValue( rDefault );
    const Any* pAny = rItem.GetPropertyValueByName( "Extrusion", rPropertyName );
    if ( pAny )
        *pAny >>= aRetValue;
    return aRetValue;
}

}

EnhancedCustomShape3d::Transformation2D::Transformation2D(
    const SdrObjCustomShape& rSdrObjCustomShape,
    const double *pM)
:   aCenter(rSdrObjCustomShape.GetSnapRect().Center())
    , eProjectionMode( drawing::ProjectionMode_PARALLEL )
    , fSkewAngle(0.0)
    , fSkew(0.0)
    , fOriginX(0.0)
    , fOriginY(0.0)
    , pMap( pM )
{
    const SdrCustomShapeGeometryItem& rGeometryItem(rSdrObjCustomShape.GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
    const Any* pAny = rGeometryItem.GetPropertyValueByName( "Extrusion", "ProjectionMode" );
    if ( pAny )
        *pAny >>= eProjectionMode;

    if ( eProjectionMode == drawing::ProjectionMode_PARALLEL )
        GetSkew( rGeometryItem, fSkew, fSkewAngle );
    else
    {
        GetOrigin( rGeometryItem, fOriginX, fOriginY );
        fOriginX = fOriginX * rSdrObjCustomShape.GetLogicRect().GetWidth();
        fOriginY = fOriginY * rSdrObjCustomShape.GetLogicRect().GetHeight();

        drawing::Position3D aViewPointDefault( 3472, -3472, 25000 );
        drawing::Position3D aViewPoint( GetPosition3D( rGeometryItem, "ViewPoint", aViewPointDefault, pMap ) );
        fViewPoint.setX(aViewPoint.PositionX);
        fViewPoint.setY(aViewPoint.PositionY);
        fViewPoint.setZ(-aViewPoint.PositionZ);
    }
}

basegfx::B3DPolygon EnhancedCustomShape3d::Transformation2D::ApplySkewSettings( const basegfx::B3DPolygon& rPoly3D ) const
{
    basegfx::B3DPolygon aRetval;

    sal_uInt32 j;
    for ( j = 0; j < rPoly3D.count(); j++ )
    {
        const basegfx::B3DPoint aPoint(rPoly3D.getB3DPoint(j));
        double fDepth(-( aPoint.getZ() * fSkew ) / 100.0);
        aRetval.append(basegfx::B3DPoint(
            aPoint.getX() + (fDepth * cos( fSkewAngle )),
            aPoint.getY() - (fDepth * sin( fSkewAngle )),
            aPoint.getZ()));
    }

    return aRetval;
}

Point EnhancedCustomShape3d::Transformation2D::Transform2D( const basegfx::B3DPoint& rPoint3D ) const
{
    Point aPoint2D;
    if ( eProjectionMode == drawing::ProjectionMode_PARALLEL )
    {
        aPoint2D.setX( static_cast<sal_Int32>(rPoint3D.getX()) );
        aPoint2D.setY( static_cast<sal_Int32>(rPoint3D.getY()) );
    }
    else
    {
        double fX = rPoint3D.getX() - fOriginX;
        double fY = rPoint3D.getY() - fOriginY;
        double f = ( - fViewPoint.getZ() ) / ( rPoint3D.getZ() - fViewPoint.getZ() );
        aPoint2D.setX( static_cast<sal_Int32>(( fX - fViewPoint.getX() ) * f + fViewPoint.getX() + fOriginX ) );
        aPoint2D.setY( static_cast<sal_Int32>(( fY - fViewPoint.getY() ) * f + fViewPoint.getY() + fOriginY ) );
    }
    aPoint2D.Move( aCenter.X(), aCenter.Y() );
    return aPoint2D;
}

bool EnhancedCustomShape3d::Transformation2D::IsParallel() const
{
    return eProjectionMode == css::drawing::ProjectionMode_PARALLEL;
}

SdrObject* EnhancedCustomShape3d::Create3DObject(
    const SdrObject* pShape2d,
    const SdrObjCustomShape& rSdrObjCustomShape)
{
    SdrObject* pRet(nullptr);
    const SdrCustomShapeGeometryItem& rGeometryItem(rSdrObjCustomShape.GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY));
    double fMap(1.0), *pMap = nullptr;
    Fraction aFraction( rSdrObjCustomShape.getSdrModelFromSdrObject().GetScaleFraction() );

    if ( aFraction.GetNumerator() != 1 || aFraction.GetDenominator() != 1 )
    {
        fMap *= double(aFraction);
        pMap = &fMap;
    }

    if ( rSdrObjCustomShape.getSdrModelFromSdrObject().GetScaleUnit() != MapUnit::Map100thMM )
    {
        DBG_ASSERT( rSdrObjCustomShape.getSdrModelFromSdrObject().GetScaleUnit() == MapUnit::MapTwip, "EnhancedCustomShape3d::Current MapMode is Unsupported" );
        fMap *= 1440.0 / 2540.0;
        pMap = &fMap;
    }

    if ( GetBool( rGeometryItem, "Extrusion", false ) )
    {
        bool bIsMirroredX(rSdrObjCustomShape.IsMirroredX());
        bool bIsMirroredY(rSdrObjCustomShape.IsMirroredY());
        tools::Rectangle aSnapRect(rSdrObjCustomShape.GetLogicRect());
        long nObjectRotation(rSdrObjCustomShape.GetRotateAngle());
        if ( nObjectRotation )
        {
            double a = (36000 - nObjectRotation) * F_PI18000;
            long dx = aSnapRect.Right() - aSnapRect.Left();
            long dy = aSnapRect.Bottom()- aSnapRect.Top();
            Point aP( aSnapRect.TopLeft() );
            RotatePoint( aP, rSdrObjCustomShape.GetSnapRect().Center(), sin( a ), cos( a ) );
            aSnapRect.SetLeft( aP.X() );
            aSnapRect.SetTop( aP.Y() );
            aSnapRect.SetRight( aSnapRect.Left() + dx );
            aSnapRect.SetBottom( aSnapRect.Top() + dy );
        }
        Point aCenter( aSnapRect.Center() );

        SfxItemSet aSet( rSdrObjCustomShape.GetMergedItemSet() );

        //SJ: vertical writing is not required, by removing this item no outliner is created
        aSet.ClearItem( SDRATTR_TEXTDIRECTION );

        // #i105323# For 3D AutoShapes, the shadow attribute has to be applied to each
        // created visualisation helper model shape individually. The shadow itself
        // will then be rendered from the 3D renderer correctly for the whole 3D scene
        // (and thus behind all objects of which the visualisation may be built). So,
        // do NOT remove it from the ItemSet here.
        // aSet.ClearItem(SDRATTR_SHADOW);

        std::vector< E3dCompoundObject* > aPlaceholderObjectList;

        double fExtrusionBackward, fExtrusionForward;
        GetExtrusionDepth( rGeometryItem, pMap, fExtrusionBackward, fExtrusionForward );
        double fDepth = fExtrusionBackward - fExtrusionForward;
        if ( fDepth < 1.0 )
            fDepth = 1.0;

        drawing::ProjectionMode eProjectionMode( drawing::ProjectionMode_PARALLEL );
        const Any* pAny = rGeometryItem.GetPropertyValueByName( "Extrusion", "ProjectionMode" );
        if ( pAny )
            *pAny >>= eProjectionMode;
        ProjectionType eProjectionType( eProjectionMode == drawing::ProjectionMode_PARALLEL ? ProjectionType::Parallel : ProjectionType::Perspective );
        // pShape2d Convert in scenes which include 3D Objects
        E3dDefaultAttributes a3DDefaultAttr;
        a3DDefaultAttr.SetDefaultLatheCharacterMode( true );
        a3DDefaultAttr.SetDefaultExtrudeCharacterMode( true );

        E3dScene* pScene = new E3dScene(rSdrObjCustomShape.getSdrModelFromSdrObject());

        bool bSceneHasObjects ( false );
        bool bUseTwoFillStyles( false );

        drawing::ShadeMode eShadeMode( GetShadeMode( rGeometryItem, drawing::ShadeMode_FLAT ) );
        bool bUseExtrusionColor = GetBool( rGeometryItem, "Color", false );

        drawing::FillStyle eFillStyle( aSet.Get(XATTR_FILLSTYLE).GetValue() );
        pScene->GetProperties().SetObjectItem( Svx3DShadeModeItem( 0 ) );
        aSet.Put( makeSvx3DPercentDiagonalItem( 0 ) );
        aSet.Put( Svx3DTextureModeItem( 1 ) );
        aSet.Put( Svx3DNormalsKindItem( 1 ) );

        if ( eShadeMode == drawing::ShadeMode_DRAFT )
        {
            aSet.Put( XLineStyleItem( drawing::LineStyle_SOLID ) );
            aSet.Put( XFillStyleItem ( drawing::FillStyle_NONE ) );
            aSet.Put( makeSvx3DDoubleSidedItem( true ) );
        }
        else
        {
            aSet.Put( XLineStyleItem( drawing::LineStyle_NONE ) );
            if ( eFillStyle == drawing::FillStyle_NONE )
                aSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
            else if ( ( eFillStyle == drawing::FillStyle_BITMAP ) || ( eFillStyle == drawing::FillStyle_GRADIENT ) || bUseExtrusionColor )
                bUseTwoFillStyles = true;

            // If shapes are mirrored once (mirroring two times correct geometry again)
            // double-sided at the object and two-sided-lighting at the scene need to be set.

            // #i122777# Also use double sided for two fill styles since there several 3d objects get
            // created with a depth of 0; one of them is the backside which needs double-sided to
            // get visible
            if(bUseTwoFillStyles || (bIsMirroredX && !bIsMirroredY) || (!bIsMirroredX && bIsMirroredY))
            {
                aSet.Put( makeSvx3DDoubleSidedItem( true ) );
                pScene->GetProperties().SetObjectItem( makeSvx3DTwoSidedLightingItem( true ) );
            }
        }

        tools::Rectangle aBoundRect2d;
        SdrObjListIter aIter( *pShape2d, SdrIterMode::DeepNoGroups );
        const bool bMultipleSubObjects(aIter.Count() > 1);

        while( aIter.IsMore() )
        {
            const SdrObject* pNext = aIter.Next();
            bool bIsPlaceholderObject = (pNext->GetMergedItem( XATTR_FILLSTYLE ).GetValue() == drawing::FillStyle_NONE )
                                        && (pNext->GetMergedItem( XATTR_LINESTYLE ).GetValue() == drawing::LineStyle_NONE );
            basegfx::B2DPolyPolygon aPolyPoly;
            SfxItemSet aLocalSet(aSet);
            drawing::FillStyle aLocalFillStyle(eFillStyle);

            if ( auto pPathObj = dynamic_cast<const SdrPathObj*>(pNext) )
            {
                const SfxItemSet& rSet = pNext->GetMergedItemSet();
                bool bNeedToConvertToContour(false);

                // do conversion only for single line objects; for all others a fill and a
                // line object get created. When we have fill, we want no line. That line has
                // always been there, but since it was never converted to contour, it kept
                // invisible (all this 'hidden' logic should be migrated to primitives).
                if(!bMultipleSubObjects)
                {
                    const drawing::FillStyle eStyle(rSet.Get(XATTR_FILLSTYLE).GetValue());

                    if(drawing::FillStyle_NONE == eStyle)
                    {
                        const drawinglayer::attribute::SdrLineAttribute aLine(
                            drawinglayer::primitive2d::createNewSdrLineAttribute(rSet));

                        bNeedToConvertToContour = (0.0 < aLine.getWidth() || 0.0 != aLine.getFullDotDashLen());

                        if(!bNeedToConvertToContour && !aLine.isDefault())
                        {
                            const drawinglayer::attribute::SdrLineStartEndAttribute aLineStartEnd(
                                drawinglayer::primitive2d::createNewSdrLineStartEndAttribute(rSet, aLine.getWidth()));

                            if((aLineStartEnd.getStartWidth() && aLineStartEnd.isStartActive())
                                || (aLineStartEnd.getEndWidth() && aLineStartEnd.isEndActive()))
                            {
                                bNeedToConvertToContour = true;
                            }
                        }
                    }
                }

                if(bNeedToConvertToContour)
                {
                    SdrObject* pNewObj = pNext->ConvertToContourObj(const_cast< SdrObject* >(pNext));
                    SdrPathObj* pNewPathObj = dynamic_cast< SdrPathObj* >(pNewObj);

                    if(pNewPathObj)
                    {
                        aPolyPoly = pNewPathObj->GetPathPoly();

                        if(aPolyPoly.isClosed())
                        {
                            // correct item properties from line to fill style
                            if(eShadeMode == drawing::ShadeMode_DRAFT)
                            {
                                // for draft, create wireframe with fixed line width
                                aLocalSet.Put(XLineStyleItem(drawing::LineStyle_SOLID));
                                aLocalSet.Put(XLineWidthItem(40));
                                aLocalFillStyle = drawing::FillStyle_NONE;
                            }
                            else
                            {
                                // switch from line to fill, copy line attr to fill attr (color, transparence)
                                aLocalSet.Put(XLineWidthItem(0));
                                aLocalSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
                                aLocalSet.Put(XFillColorItem(OUString(), aLocalSet.Get(XATTR_LINECOLOR).GetColorValue()));
                                aLocalSet.Put(XFillStyleItem(drawing::FillStyle_SOLID));
                                aLocalSet.Put(XFillTransparenceItem(aLocalSet.Get(XATTR_LINETRANSPARENCE).GetValue()));
                                aLocalFillStyle = drawing::FillStyle_SOLID;
                            }
                        }
                        else
                        {
                            // correct item properties to hairlines
                            aLocalSet.Put(XLineWidthItem(0));
                            aLocalSet.Put(XLineStyleItem(drawing::LineStyle_SOLID));
                        }
                    }

                    SdrObject::Free(pNewObj);
                }
                else
                {
                    aPolyPoly = pPathObj->GetPathPoly();
                }
            }
            else
            {
                SdrObject* pNewObj = pNext->ConvertToPolyObj( false, false );
                SdrPathObj* pPath = dynamic_cast<SdrPathObj*>( pNewObj  );
                if ( pPath )
                    aPolyPoly = pPath->GetPathPoly();
                SdrObject::Free( pNewObj );
            }

            if( aPolyPoly.count() )
            {
                if(aPolyPoly.areControlPointsUsed())
                {
                    aPolyPoly = basegfx::utils::adaptiveSubdivideByAngle(aPolyPoly);
                }

                const basegfx::B2DRange aTempRange(basegfx::utils::getRange(aPolyPoly));
                const tools::Rectangle aBoundRect(basegfx::fround(aTempRange.getMinX()), basegfx::fround(aTempRange.getMinY()), basegfx::fround(aTempRange.getMaxX()), basegfx::fround(aTempRange.getMaxY()));
                aBoundRect2d.Union( aBoundRect );

                // #i122777# depth 0 is okay for planes when using double-sided
                E3dCompoundObject* p3DObj = new E3dExtrudeObj(
                    rSdrObjCustomShape.getSdrModelFromSdrObject(),
                    a3DDefaultAttr,
                    aPolyPoly,
                    bUseTwoFillStyles ? 0 : fDepth );

                p3DObj->NbcSetLayer( pShape2d->GetLayer() );
                p3DObj->SetMergedItemSet( aLocalSet );

                if ( bIsPlaceholderObject )
                    aPlaceholderObjectList.push_back( p3DObj );
                else if ( bUseTwoFillStyles )
                {
                    BitmapEx aFillBmp;
                    bool bFillBmpTile = p3DObj->GetMergedItem( XATTR_FILLBMP_TILE ).GetValue();
                    if ( bFillBmpTile )
                    {
                        const XFillBitmapItem& rBmpItm = p3DObj->GetMergedItem(XATTR_FILLBITMAP);
                        aFillBmp = rBmpItm.GetGraphicObject().GetGraphic().GetBitmapEx();

                        // #i122777# old adaption of FillStyle bitmap size to 5-times the original size; this is not needed
                        // anymore and was used in old times to male the fill look better when converting to 3D. Removed
                        // from regular 3D objects for some time, also needs to be removed from CustomShapes

                        //Size aLogicalSize = aFillBmp.GetPrefSize();
                        //if ( aFillBmp.GetPrefMapMode() == MapUnit::MapPixel )
                        //  aLogicalSize = Application::GetDefaultDevice()->PixelToLogic( aLogicalSize, MapUnit::Map100thMM );
                        //else
                        //  aLogicalSize = OutputDevice::LogicToLogic( aLogicalSize, aFillBmp.GetPrefMapMode(), MapUnit::Map100thMM );
                        //aLogicalSize.Width()  *= 5;           ;//             :-(     nice scaling, look at engine3d/obj3d.cxx
                        //aLogicalSize.Height() *= 5;
                        //aFillBmp.SetPrefSize( aLogicalSize );
                        //aFillBmp.SetPrefMapMode( MapUnit::Map100thMM );
                        //p3DObj->SetMergedItem(XFillBitmapItem(String(), Graphic(aFillBmp)));
                    }
                    else
                    {
                        if ( aSnapRect != aBoundRect && aSnapRect.GetWidth() > 0 && aSnapRect.GetHeight() > 0)
                        {
                            const XFillBitmapItem& rBmpItm = p3DObj->GetMergedItem(XATTR_FILLBITMAP);
                            aFillBmp = rBmpItm.GetGraphicObject().GetGraphic().GetBitmapEx();
                            Size aBmpSize( aFillBmp.GetSizePixel() );
                            double fXScale = static_cast<double>(aBoundRect.GetWidth()) / static_cast<double>(aSnapRect.GetWidth());
                            double fYScale = static_cast<double>(aBoundRect.GetHeight()) / static_cast<double>(aSnapRect.GetHeight());

                            Point aPt( static_cast<sal_Int32>( static_cast<double>( aBoundRect.Left() - aSnapRect.Left() )* static_cast<double>(aBmpSize.Width()) / static_cast<double>(aSnapRect.GetWidth()) ),
                                                static_cast<sal_Int32>( static_cast<double>( aBoundRect.Top() - aSnapRect.Top() ) * static_cast<double>(aBmpSize.Height()) / static_cast<double>(aSnapRect.GetHeight()) ) );
                            Size aSize( static_cast<sal_Int32>( aBmpSize.Width() * fXScale ),
                                                    static_cast<sal_Int32>( aBmpSize.Height() * fYScale ) );
                            tools::Rectangle aCropRect( aPt, aSize );
                            aFillBmp.Crop( aCropRect );
                            p3DObj->SetMergedItem(XFillBitmapItem(OUString(), Graphic(aFillBmp)));
                        }
                    }
                    pScene->InsertObject( p3DObj );
                    p3DObj = new E3dExtrudeObj(
                        rSdrObjCustomShape.getSdrModelFromSdrObject(),
                        a3DDefaultAttr,
                        aPolyPoly,
                        fDepth);
                    p3DObj->NbcSetLayer( pShape2d->GetLayer() );
                    p3DObj->SetMergedItemSet( aLocalSet );
                    if ( bUseExtrusionColor )
                        p3DObj->SetMergedItem( XFillColorItem( "", rSdrObjCustomShape.GetMergedItem( XATTR_SECONDARYFILLCOLOR ).GetColorValue() ) );
                    p3DObj->SetMergedItem( XFillStyleItem( drawing::FillStyle_SOLID ) );
                    p3DObj->SetMergedItem( Svx3DCloseFrontItem( false ) );
                    p3DObj->SetMergedItem( Svx3DCloseBackItem( false ) );
                    pScene->InsertObject( p3DObj );

                    // #i122777# depth 0 is okay for planes when using double-sided
                    p3DObj = new E3dExtrudeObj(
                        rSdrObjCustomShape.getSdrModelFromSdrObject(),
                        a3DDefaultAttr,
                        aPolyPoly,
                        0);

                    p3DObj->NbcSetLayer( pShape2d->GetLayer() );
                    p3DObj->SetMergedItemSet( aLocalSet );

                    basegfx::B3DHomMatrix aFrontTransform( p3DObj->GetTransform() );
                    aFrontTransform.translate( 0.0, 0.0, fDepth );
                    p3DObj->NbcSetTransform( aFrontTransform );

                    if ( ( aLocalFillStyle == drawing::FillStyle_BITMAP ) && !aFillBmp.IsEmpty() )
                    {
                        p3DObj->SetMergedItem(XFillBitmapItem(OUString(), Graphic(aFillBmp)));
                    }
                }
                else if ( aLocalFillStyle == drawing::FillStyle_NONE )
                {
                    const XLineColorItem& rLineColor = p3DObj->GetMergedItem( XATTR_LINECOLOR );
                    p3DObj->SetMergedItem( XFillColorItem( "", rLineColor.GetColorValue() ) );
                    p3DObj->SetMergedItem( makeSvx3DDoubleSidedItem( true ) );
                    p3DObj->SetMergedItem( Svx3DCloseFrontItem( false ) );
                    p3DObj->SetMergedItem( Svx3DCloseBackItem( false ) );
                }
                pScene->InsertObject( p3DObj );
                bSceneHasObjects = true;
            }
        }

        if ( bSceneHasObjects ) // is the SdrObject properly converted
        {
            // then we can change the return value
            pRet = pScene;

            // Camera settings, Perspective ...
            Camera3D rCamera = pScene->GetCamera();
            const basegfx::B3DRange& rVolume = pScene->GetBoundVolume();
            pScene->NbcSetSnapRect( aSnapRect );

            // InitScene replacement
            double fW = rVolume.getWidth();
            double fH = rVolume.getHeight();

            rCamera.SetAutoAdjustProjection( false );
            rCamera.SetViewWindow( -fW / 2, - fH / 2, fW, fH);
            basegfx::B3DPoint aLookAt( 0.0, 0.0, 0.0 );
            basegfx::B3DPoint aCamPos( 0.0, 0.0, 100.0 );
            rCamera.SetPosAndLookAt( aCamPos, aLookAt );
            rCamera.SetFocalLength( 1.0 );
            rCamera.SetProjection( eProjectionType );
            pScene->SetCamera( rCamera );
            pScene->SetRectsDirty();

            double fOriginX, fOriginY;
            GetOrigin( rGeometryItem, fOriginX, fOriginY );
            fOriginX = fOriginX * aSnapRect.GetWidth();
            fOriginY = fOriginY * aSnapRect.GetHeight();

            basegfx::B3DHomMatrix aNewTransform( pScene->GetTransform() );
            aNewTransform.translate( -aCenter.X(), aCenter.Y(), -pScene->GetBoundVolume().getDepth() );

            double fXRotate, fYRotate;
            GetRotateAngle( rGeometryItem, fXRotate, fYRotate );
            double fZRotate(basegfx::deg2rad(rSdrObjCustomShape.GetObjectRotation()));
            if ( fZRotate != 0.0 )
                aNewTransform.rotate( 0.0, 0.0, fZRotate );
            if ( bIsMirroredX )
                aNewTransform.scale( -1.0, 1, 1 );
            if ( bIsMirroredY )
                aNewTransform.scale( 1, -1.0, 1 );
            if( fYRotate != 0.0 )
                aNewTransform.rotate( 0.0, -fYRotate, 0.0 );
            if( fXRotate != 0.0 )
                aNewTransform.rotate( -fXRotate, 0.0, 0.0 );
            if ( eProjectionType == ProjectionType::Parallel )
            {
                double fSkew, fAlpha;
                GetSkew( rGeometryItem, fSkew, fAlpha );
                if ( fSkew != 0.0 )
                {
                    double fInvTanBeta( fSkew / 100.0 );
                    if(fInvTanBeta)
                    {
                        aNewTransform.shearXY(
                            fInvTanBeta * cos(fAlpha),
                            fInvTanBeta * sin(fAlpha));
                    }
                }
                basegfx::B3DPoint _aLookAt( 0.0, 0.0, 0.0 );
                basegfx::B3DPoint _aNewCamPos( 0.0, 0.0, 25000.0 );
                rCamera.SetPosAndLookAt( _aNewCamPos, _aLookAt );
                pScene->SetCamera( rCamera );
            }
            else
            {
                aNewTransform.translate( -fOriginX, fOriginY, 0.0 );
                // now set correct camera position
                drawing::Position3D aViewPointDefault( 3472, -3472, 25000 );
                drawing::Position3D aViewPoint( GetPosition3D( rGeometryItem, "ViewPoint", aViewPointDefault, pMap ) );
                double fViewPointX = aViewPoint.PositionX;
                double fViewPointY = aViewPoint.PositionY;
                double fViewPointZ = aViewPoint.PositionZ;
                basegfx::B3DPoint _aLookAt( fViewPointX, -fViewPointY, 0.0 );
                basegfx::B3DPoint aNewCamPos( fViewPointX, -fViewPointY, fViewPointZ );
                rCamera.SetPosAndLookAt( aNewCamPos, _aLookAt );
                pScene->SetCamera( rCamera );
            }

            pScene->NbcSetTransform( aNewTransform );


            // light

            double fAmbientIntensity = GetDouble( rGeometryItem, "Brightness", 22178.0 / 655.36 ) / 100.0;

            drawing::Direction3D aFirstLightDirectionDefault( 50000, 0, 10000 );
            drawing::Direction3D aFirstLightDirection( GetDirection3D( rGeometryItem, "FirstLightDirection", aFirstLightDirectionDefault ) );
            if ( aFirstLightDirection.DirectionZ == 0.0 )
                aFirstLightDirection.DirectionZ = 1.0;

            double fLightIntensity = GetDouble( rGeometryItem, "FirstLightLevel", 43712.0 / 655.36 ) / 100.0;

            /* sal_Bool bFirstLightHarsh = */ GetBool( rGeometryItem, "FirstLightHarsh", false );

            drawing::Direction3D aSecondLightDirectionDefault( -50000, 0, 10000 );
            drawing::Direction3D aSecondLightDirection( GetDirection3D( rGeometryItem, "SecondLightDirection", aSecondLightDirectionDefault ) );
            if ( aSecondLightDirection.DirectionZ == 0.0 )
                aSecondLightDirection.DirectionZ = -1;

            double fLight2Intensity = GetDouble( rGeometryItem, "SecondLightLevel", 43712.0 / 655.36 ) / 100.0;

            /* sal_Bool bLight2Harsh = */ GetBool( rGeometryItem, "SecondLightHarsh", false );
            /* sal_Bool bLightFace = */ GetBool( rGeometryItem, "LightFace", false );

            sal_uInt16 nAmbientColor = static_cast<sal_uInt16>( fAmbientIntensity * 255.0 );
            if ( nAmbientColor > 255 )
                nAmbientColor = 255;
            Color aGlobalAmbientColor( static_cast<sal_uInt8>(nAmbientColor), static_cast<sal_uInt8>(nAmbientColor), static_cast<sal_uInt8>(nAmbientColor) );
            pScene->GetProperties().SetObjectItem( makeSvx3DAmbientcolorItem( aGlobalAmbientColor ) );

            sal_uInt8 nSpotLight1 = static_cast<sal_uInt8>( fLightIntensity * 255.0 );
            basegfx::B3DVector aSpotLight1( aFirstLightDirection.DirectionX, - ( aFirstLightDirection.DirectionY ), -( aFirstLightDirection.DirectionZ ) );
            aSpotLight1.normalize();
            pScene->GetProperties().SetObjectItem( makeSvx3DLightOnOff1Item( true ) );
            Color aAmbientSpot1Color( nSpotLight1, nSpotLight1, nSpotLight1 );
            pScene->GetProperties().SetObjectItem( makeSvx3DLightcolor1Item( aAmbientSpot1Color ) );
            pScene->GetProperties().SetObjectItem( makeSvx3DLightDirection1Item( aSpotLight1 ) );

            sal_uInt8 nSpotLight2 = static_cast<sal_uInt8>( fLight2Intensity * 255.0 );
            basegfx::B3DVector aSpotLight2( aSecondLightDirection.DirectionX, -aSecondLightDirection.DirectionY, -aSecondLightDirection.DirectionZ );
            aSpotLight2.normalize();
            pScene->GetProperties().SetObjectItem( makeSvx3DLightOnOff2Item( true ) );
            Color aAmbientSpot2Color( nSpotLight2, nSpotLight2, nSpotLight2 );
            pScene->GetProperties().SetObjectItem( makeSvx3DLightcolor2Item( aAmbientSpot2Color ) );
            pScene->GetProperties().SetObjectItem( makeSvx3DLightDirection2Item( aSpotLight2 ) );

            sal_uInt8 nSpotLight3 = 70;
            basegfx::B3DVector aSpotLight3( 0.0, 0.0, 1.0 );
            pScene->GetProperties().SetObjectItem( makeSvx3DLightOnOff3Item( true ) );
            Color aAmbientSpot3Color( nSpotLight3, nSpotLight3, nSpotLight3 );
            pScene->GetProperties().SetObjectItem( makeSvx3DLightcolor3Item( aAmbientSpot3Color ) );
            pScene->GetProperties().SetObjectItem( makeSvx3DLightDirection3Item( aSpotLight3 ) );

            double fSpecular = GetDouble( rGeometryItem, "Specularity", 0 ) / 100;
            bool bMetal = GetBool( rGeometryItem, "Metal", false );

            Color aSpecularCol( 225,225,225 );
            if ( bMetal )
            {
                aSpecularCol = Color( 200, 200, 200 );
                fSpecular += 0.15;
            }
            sal_Int32 nIntensity = static_cast<sal_Int32>(fSpecular) * 100;
            if ( nIntensity > 100 )
                nIntensity = 100;
            else if ( nIntensity < 0 )
                nIntensity = 0;
            nIntensity = 100 - nIntensity;
            pScene->GetProperties().SetObjectItem( makeSvx3DMaterialSpecularItem( aSpecularCol ) );
            pScene->GetProperties().SetObjectItem( makeSvx3DMaterialSpecularIntensityItem( static_cast<sal_uInt16>(nIntensity) ) );

            pScene->SetLogicRect(
                CalculateNewSnapRect(
                    rSdrObjCustomShape,
                    aSnapRect,
                    aBoundRect2d,
                    pMap));

            // removing placeholder objects
            for (E3dCompoundObject* pTemp : aPlaceholderObjectList)
            {
                pScene->RemoveObject( pTemp->GetOrdNum() );
                // always use SdrObject::Free(...) for SdrObjects (!)
                SdrObject* pTemp2(pTemp);
                SdrObject::Free(pTemp2);
            }
        }
        else
        {
            // always use SdrObject::Free(...) for SdrObjects (!)
            SdrObject* pTemp(pScene);
            SdrObject::Free(pTemp);
        }
    }
    return pRet;
}

tools::Rectangle EnhancedCustomShape3d::CalculateNewSnapRect(
    const SdrObjCustomShape& rSdrObjCustomShape,
    const tools::Rectangle& rSnapRect,
    const tools::Rectangle& rBoundRect,
    const double* pMap)
{
    const SdrCustomShapeGeometryItem& rGeometryItem(rSdrObjCustomShape.GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
    const Point aCenter( rSnapRect.Center() );
    double fExtrusionBackward, fExtrusionForward;
    GetExtrusionDepth( rGeometryItem, pMap, fExtrusionBackward, fExtrusionForward );
    sal_uInt32 i;

    // creating initial bound volume ( without rotation. skewing.and camera )
    basegfx::B3DPolygon aBoundVolume;
    const tools::Polygon aPolygon( rBoundRect );

    for ( i = 0; i < 4; i++ )
    {
        aBoundVolume.append(basegfx::B3DPoint(aPolygon[ static_cast<sal_uInt16>(i) ].X() - aCenter.X(), aPolygon[ static_cast<sal_uInt16>(i) ].Y() - aCenter.Y(), fExtrusionForward));
    }

    for ( i = 0; i < 4; i++ )
    {
        aBoundVolume.append(basegfx::B3DPoint(aPolygon[ static_cast<sal_uInt16>(i) ].X() - aCenter.X(), aPolygon[ static_cast<sal_uInt16>(i) ].Y() - aCenter.Y(), fExtrusionBackward));
    }

    drawing::Direction3D aRotationCenterDefault( 0, 0, 0 ); // default seems to be wrong, a fractional size of shape has to be used!!
    drawing::Direction3D aRotationCenter( GetDirection3D( rGeometryItem, "RotationCenter", aRotationCenterDefault ) );

    double fXRotate, fYRotate;
    GetRotateAngle( rGeometryItem, fXRotate, fYRotate );
    double fZRotate(basegfx::deg2rad(rSdrObjCustomShape.GetObjectRotation()));

    // rotating bound volume
    basegfx::B3DHomMatrix aMatrix;
    aMatrix.translate(-aRotationCenter.DirectionX, -aRotationCenter.DirectionY, -aRotationCenter.DirectionZ);
    if ( fZRotate != 0.0 )
        aMatrix.rotate( 0.0, 0.0, fZRotate );
    if (rSdrObjCustomShape.IsMirroredX())
        aMatrix.scale( -1.0, 1, 1 );
    if (rSdrObjCustomShape.IsMirroredY())
        aMatrix.scale( 1, -1.0, 1 );
    if( fYRotate != 0.0 )
        aMatrix.rotate( 0.0, fYRotate, 0.0 );
    if( fXRotate != 0.0 )
        aMatrix.rotate( -fXRotate, 0.0, 0.0 );
    aMatrix.translate(aRotationCenter.DirectionX, aRotationCenter.DirectionY, aRotationCenter.DirectionZ);
    aBoundVolume.transform(aMatrix);

    Transformation2D aTransformation2D(
        rSdrObjCustomShape,
        pMap);

    if ( aTransformation2D.IsParallel() )
        aBoundVolume = aTransformation2D.ApplySkewSettings( aBoundVolume );

    tools::Polygon aTransformed( 8 );
    for ( i = 0; i < 8; i++ )
        aTransformed[ static_cast<sal_uInt16>(i) ] = aTransformation2D.Transform2D( aBoundVolume.getB3DPoint( i ) );

    return aTransformed.GetBoundRect();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
