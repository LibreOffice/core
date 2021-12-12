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
#include <o3tl/unit_conversion.hxx>
#include <svx/deflt3d.hxx>
#include <svx/svdmodel.hxx>
#include <tools/poly.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoashp.hxx>
#include <svl/itemset.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xsflclit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflclit.hxx>
#include <svx/svdopath.hxx>
#include <svx/svddef.hxx>
#include <svx/svx3ditems.hxx>
#include <extrud3d.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/sdasitm.hxx>
#include <svx/scene3d.hxx>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <svx/sdr/properties/properties.hxx>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <sdr/primitive2d/sdrattributecreator.hxx>
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
        // But we could use MapToO3tlUnit from <tools/UnitConversion> ... ?
        fMap *= o3tl::convert(1.0, o3tl::Length::mm100, o3tl::Length::twip);
        pMap = &fMap;
    }

    if ( GetBool( rGeometryItem, "Extrusion", false ) )
    {
        bool bIsMirroredX(rSdrObjCustomShape.IsMirroredX());
        bool bIsMirroredY(rSdrObjCustomShape.IsMirroredY());
        tools::Rectangle aSnapRect(rSdrObjCustomShape.GetLogicRect());
        Degree100 nObjectRotation(rSdrObjCustomShape.GetRotateAngle());
        if ( nObjectRotation )
        {
            double a = toRadians(36000_deg100 - nObjectRotation);
            tools::Long dx = aSnapRect.Right() - aSnapRect.Left();
            tools::Long dy = aSnapRect.Bottom()- aSnapRect.Top();
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
        double fDepth = fExtrusionBackward + fExtrusionForward;
        if ( fDepth < 1.0 )
            fDepth = 1.0;

        drawing::ProjectionMode eProjectionMode( drawing::ProjectionMode_PARALLEL );
        const Any* pAny = rGeometryItem.GetPropertyValueByName( "Extrusion", "ProjectionMode" );
        if ( pAny )
            *pAny >>= eProjectionMode;
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
        pScene->GetProperties().SetObjectItem( Svx3DShadeModeItem(static_cast<sal_uInt16>(eShadeMode)));
        aSet.Put( makeSvx3DPercentDiagonalItem( 0 ) );
        aSet.Put( Svx3DTextureModeItem( 1 ) );
         // SPECIFIC needed for ShadeMode_SMOOTH and ShadeMode_PHONG, otherwise FLAT is faster
        if (eShadeMode == drawing::ShadeMode_SMOOTH || eShadeMode == drawing::ShadeMode_PHONG)
            aSet.Put( Svx3DNormalsKindItem(static_cast<sal_uInt16>(drawing::NormalsKind_SPECIFIC)));
        else
            aSet.Put( Svx3DNormalsKindItem(static_cast<sal_uInt16>(drawing::NormalsKind_FLAT)));

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
        basegfx::B2DPolyPolygon aTotalPolyPoly;
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
                SdrObjectUniquePtr pNewObj = pNext->ConvertToPolyObj( false, false );
                SdrPathObj* pPath = dynamic_cast<SdrPathObj*>( pNewObj.get() );
                if ( pPath )
                    aPolyPoly = pPath->GetPathPoly();
            }

            if( aPolyPoly.count() )
            {
                if(aPolyPoly.areControlPointsUsed())
                {
                    aPolyPoly = basegfx::utils::adaptiveSubdivideByAngle(aPolyPoly);
                }

                const basegfx::B2DRange aTempRange(basegfx::utils::getRange(aPolyPoly));
                const tools::Rectangle aBoundRect(basegfx::fround(aTempRange.getMinX()), basegfx::fround(aTempRange.getMinY()), basegfx::fround(aTempRange.getMaxX()), basegfx::fround(aTempRange.getMaxY()));
                aTotalPolyPoly.append(aPolyPoly);
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

                        // #i122777# old adaptation of FillStyle bitmap size to 5-times the original size; this is not needed
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
            pScene->NbcSetSnapRect( aSnapRect );

            // InitScene replacement
            double fW = aBoundRect2d.getWidth();
            double fH = aBoundRect2d.getHeight();
            rCamera.SetAutoAdjustProjection( false );
            rCamera.SetViewWindow( -fW / 2, - fH / 2, fW, fH);
            basegfx::B3DPoint aLookAt( 0.0, 0.0, 0.0 );
            basegfx::B3DPoint aCamPos( 0.0, 0.0, 100.0 );
            rCamera.SetPosAndLookAt( aCamPos, aLookAt );
            rCamera.SetFocalLength( 1.0 );
            ProjectionType eProjectionType( eProjectionMode == drawing::ProjectionMode_PARALLEL ? ProjectionType::Parallel : ProjectionType::Perspective );
            rCamera.SetProjection( eProjectionType );
            pScene->SetCamera( rCamera );
            pScene->SetBoundAndSnapRectsDirty();

            basegfx::B3DHomMatrix aNewTransform( pScene->GetTransform() );
            basegfx::B2DHomMatrix aPolyPolyTransform;
            // Apply flip and z-rotation to scene transformation (y up). At same time transform
            // aTotalPolyPoly (y down) which will be used for 2D boundRect of shape having 2D
            // transformations applied.

            // API values use shape center as origin. Move scene so, that shape center is origin.
            aNewTransform.translate( -aCenter.X(), aCenter.Y(), -fExtrusionBackward);
            aPolyPolyTransform.translate(-aCenter.X(), -aCenter.Y());

            double fZRotate(basegfx::deg2rad(rSdrObjCustomShape.GetObjectRotation()));
            if ( fZRotate != 0.0 )
            {
                aNewTransform.rotate( 0.0, 0.0, fZRotate );
                aPolyPolyTransform.rotate(-fZRotate);
            }
            if ( bIsMirroredX )
            {
                aNewTransform.scale( -1.0, 1, 1 );
                aPolyPolyTransform.scale(-1.0, 1);
            }
            if ( bIsMirroredY )
            {
                aNewTransform.scale( 1, -1.0, 1 );
                aPolyPolyTransform.scale(1, -1.0);
            }
            aPolyPolyTransform.translate(aCenter.X(), aCenter.Y());
            aTotalPolyPoly.transform(aPolyPolyTransform);

            // x- and y-rotation have an own rotation center. x- and y-value of rotation center are
            // fractions of shape size, z-value is in Hmm in property. Shape center is (0 0 0).
            // Values in property are in custom shape extrusion space with y-axis down.
            double fXRotate, fYRotate;
            GetRotateAngle( rGeometryItem, fXRotate, fYRotate );
            drawing::Direction3D aRotationCenterDefault( 0, 0, 0 );
            drawing::Direction3D aRotationCenter( GetDirection3D( rGeometryItem, "RotationCenter", aRotationCenterDefault ) );
            aRotationCenter.DirectionX *= aSnapRect.getWidth();
            aRotationCenter.DirectionY *= aSnapRect.getHeight();
            if (pMap)
            {
                double fMap = *pMap;
                aRotationCenter.DirectionZ *= fMap;
            }
            aNewTransform.translate( -aRotationCenter.DirectionX, aRotationCenter.DirectionY, -aRotationCenter.DirectionZ );
            if( fYRotate != 0.0 )
                aNewTransform.rotate( 0.0, -fYRotate, 0.0 );
            if( fXRotate != 0.0 )
                aNewTransform.rotate( -fXRotate, 0.0, 0.0 );
            aNewTransform.translate(aRotationCenter.DirectionX, -aRotationCenter.DirectionY, aRotationCenter.DirectionZ);

            // oblique parallel projection is done by shearing the object, not by moving the camera
            if (eProjectionMode == drawing::ProjectionMode_PARALLEL)
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
            }

            pScene->NbcSetTransform( aNewTransform );

            // These values are used later again, so declare them outside the if-statement. They will
            // contain the absolute values of ViewPoint in 3D scene coordinate system, y-axis up.
            double fViewPointX = 0; // dummy values
            double fViewPointY = 0;
            double fViewPointZ = 25000;
            if (eProjectionMode == drawing::ProjectionMode_PERSPECTIVE)
            {
                double fOriginX, fOriginY;
                // Calculate BoundRect of shape, including flip and z-rotation, from aTotalPolyPoly.
                tools::Rectangle aBoundAfter2DTransform; // aBoundAfter2DTransform has y-axis down.
                basegfx::B2DRange aTotalPolyPolyRange(aTotalPolyPoly.getB2DRange());
                aBoundAfter2DTransform.SetLeft(aTotalPolyPolyRange.getMinX());
                aBoundAfter2DTransform.SetTop(aTotalPolyPolyRange.getMinY());
                aBoundAfter2DTransform.SetRight(aTotalPolyPolyRange.getMaxX());
                aBoundAfter2DTransform.SetBottom(aTotalPolyPolyRange.getMaxY());

                // Property "Origin" in API is relativ to bounding box of shape after 2D
                // transformations. Range is [-0.5;0.5] with center of bounding box as 0.
                // Resolve "Origin" fractions to length
                GetOrigin( rGeometryItem, fOriginX, fOriginY );
                fOriginX *= aBoundAfter2DTransform.GetWidth();
                fOriginY *= aBoundAfter2DTransform.GetHeight();
                // Resolve length to absolute value for 3D
                fOriginX += aBoundAfter2DTransform.Center().X();
                fOriginY += aBoundAfter2DTransform.Center().Y();
                fOriginY = - fOriginY;
                // Scene is translated so that shape center is origin of coordinate system.
                // Translate point "Origin" too.
                fOriginX -= aCenter.X();
                fOriginY -= -aCenter.Y();
                // API ViewPoint values are relativ to point "Origin" and have y-axis down.
                // ToDo: These default ViewPoint values are used as default by MS Office. But ODF
                // default is (3500, -3500, 25000), details in tdf#146192.
                drawing::Position3D aViewPointDefault( 3472, -3472, 25000 );
                drawing::Position3D aViewPoint( GetPosition3D( rGeometryItem, "ViewPoint", aViewPointDefault, pMap ) );
                fViewPointX = aViewPoint.PositionX + fOriginX;
                fViewPointY = - aViewPoint.PositionY + fOriginY;
                fViewPointZ = aViewPoint.PositionZ;
            }

            // now set correct camera position
            if (eProjectionMode == drawing::ProjectionMode_PARALLEL)
            {
                basegfx::B3DPoint _aLookAt( 0.0, 0.0, 0.0 );
                basegfx::B3DPoint _aNewCamPos( 0.0, 0.0, 25000.0 );
                rCamera.SetPosAndLookAt( _aNewCamPos, _aLookAt );
                pScene->SetCamera( rCamera );
            }
            else
            {
                basegfx::B3DPoint _aLookAt(fViewPointX, fViewPointY, 0.0);
                basegfx::B3DPoint aNewCamPos(fViewPointX, fViewPointY, fViewPointZ);
                rCamera.SetPosAndLookAt( aNewCamPos, _aLookAt );
                pScene->SetCamera( rCamera );
            }

            // NbcSetTransform has not updated the scene 2D rectangles.
            // Idea: Get a bound volume as polygon from bound rectangle of shape without 2D
            // transfomations. Calculate its projection to the XY-plane. Then calculate the bounding
            // rectangle of the projection and convert this rectangle back to absolut 2D coordinates.
            // Set that as 2D rectangle of the scene.
            const tools::Polygon aPolygon(aBoundRect2d); // y-up
            basegfx::B3DPolygon aPolygonBoundVolume; // y-down, scene coordinates
            for (sal_uInt16 i = 0; i < 4; i++ )
            {
                aPolygonBoundVolume.append(basegfx::B3DPoint(aPolygon[i].X(), -aPolygon[i].Y(), 0));
            }
            for (sal_uInt16 i = 0; i < 4; i++ )
            {
                aPolygonBoundVolume.append(basegfx::B3DPoint(aPolygon[i].X(), -aPolygon[i].Y(), fDepth));
            }
            aPolygonBoundVolume.transform(aNewTransform);

            // projection
            tools::Polygon a2DProjectionResult(8); // in fact 3D points with z=0
            for (sal_uInt16 i = 0; i < 8; i++ )
            {
                const basegfx::B3DPoint aPoint3D(aPolygonBoundVolume.getB3DPoint(i));

                if (eProjectionMode == drawing::ProjectionMode_PARALLEL)
                {
                    a2DProjectionResult[i].setX(aPoint3D.getX());
                    a2DProjectionResult[i].setY(aPoint3D.getY());
                }
                else
                {
                    // skip point if line from viewpoint to point is parallel to xy-plane
                    if (double fDiv = aPoint3D.getZ() - fViewPointZ; fDiv != 0.0)
                    {
                        double f = (- fViewPointZ) / fDiv;
                        double fX = (aPoint3D.getX() - fViewPointX) * f + fViewPointX;
                        double fY = (aPoint3D.getY() - fViewPointY) * f + fViewPointY;;
                        a2DProjectionResult[i].setX(static_cast<sal_Int32>(fX));
                        a2DProjectionResult[i].setY(static_cast<sal_Int32>(fY));
                    }
                }
            }
            // Convert to y-axis down
            for (sal_uInt16 i = 0; i < 8; i++ )
            {
                a2DProjectionResult[i].setY(- a2DProjectionResult[i].Y());
            }
            // Shift back to shape center
            a2DProjectionResult.Translate(aCenter);

            pScene->SetLogicRect(a2DProjectionResult.GetBoundRect());


            // light


            drawing::Direction3D aFirstLightDirectionDefault( 50000, 0, 10000 );
            drawing::Direction3D aFirstLightDirection( GetDirection3D( rGeometryItem, "FirstLightDirection", aFirstLightDirectionDefault ) );
            if ( aFirstLightDirection.DirectionZ == 0.0 )
                aFirstLightDirection.DirectionZ = 1.0;

            double fLightIntensity = GetDouble( rGeometryItem, "FirstLightLevel", 43712.0 / 655.36 ) / 100.0;

            bool bFirstLightHarsh = GetBool( rGeometryItem, "FirstLightHarsh", true );

            drawing::Direction3D aSecondLightDirectionDefault( -50000, 0, 10000 );
            drawing::Direction3D aSecondLightDirection( GetDirection3D( rGeometryItem, "SecondLightDirection", aSecondLightDirectionDefault ) );
            if ( aSecondLightDirection.DirectionZ == 0.0 )
                aSecondLightDirection.DirectionZ = -1;

            double fLight2Intensity = GetDouble( rGeometryItem, "SecondLightLevel", 43712.0 / 655.36 ) / 100.0;

            /* sal_Bool bLight2Harsh = */ GetBool( rGeometryItem, "SecondLightHarsh", false );
            /* sal_Bool bLightFace = */ GetBool( rGeometryItem, "LightFace", false );

            double fAmbientIntensity = GetDouble( rGeometryItem, "Brightness", 22178.0 / 655.36 ) / 100.0;
            bool bMetal = GetBool( rGeometryItem, "Metal", false );

            // Currently needed for import from binary MS Office.
            // ToDo: Create a solution in the filters.
            // MS Office adds black to diffuse and ambient color in case of metal. Use an
            // approximating ersatz.
            if (bMetal)
            {
                fAmbientIntensity -= 0.15; // Estimated value. Adapt it if necessary.
                fAmbientIntensity = std::clamp(fAmbientIntensity, 0.0, 1.0);
                fLight2Intensity -= 0.15;
                fLight2Intensity = std::clamp(fLight2Intensity, 0.0, 1.0);
            }

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

            // Currently needed for import from binary MS Office.
            // ToDo: Create a solution in the filters.
            // Binary MS Office creates brighter shapes than our 3D engine with same values.
            sal_uInt8 nSpotLight3 = 70;
            basegfx::B3DVector aSpotLight3( 0.0, 0.0, 1.0 );
            pScene->GetProperties().SetObjectItem( makeSvx3DLightOnOff3Item( true ) );
            Color aAmbientSpot3Color( nSpotLight3, nSpotLight3, nSpotLight3 );
            pScene->GetProperties().SetObjectItem( makeSvx3DLightcolor3Item( aAmbientSpot3Color ) );
            pScene->GetProperties().SetObjectItem( makeSvx3DLightDirection3Item( aSpotLight3 ) );

            double fSpecular = GetDouble( rGeometryItem, "Specularity", 0 );
            // ODF specifies 'white', OOXML uses shape fill color in some presets
            Color aSpecularCol(255, 255, 255);
            if ( bMetal )
            {
                // values as specified in ODF
                aSpecularCol = Color( 200, 200, 200 );
                fSpecular += 15.0;
            }
            sal_Int32 nIntensity = 100 - static_cast<sal_Int32>(fSpecular);
            nIntensity = std::clamp<sal_Int32>(nIntensity, 0, 100);

            // specularity is an object property, not a scene property
            SdrObjListIter aSceneIter( *pScene, SdrIterMode::DeepNoGroups );
            while (aSceneIter.IsMore())
            {
                const SdrObject* pNext = aSceneIter.Next();
                pNext->GetProperties().SetObjectItem(makeSvx3DMaterialSpecularItem(aSpecularCol));
                pNext->GetProperties().SetObjectItem(makeSvx3DMaterialSpecularIntensityItem(static_cast<sal_uInt16>(nIntensity)));
            }

            // fSpecular = 0 is used to indicate surface preset "matte".
            if (!bFirstLightHarsh || basegfx::fTools::equalZero(fSpecular, 0.0001))
            {
                // First light in LO 3D engine is always specular, all other lights are never specular.
                // We copy light1 values to light4 and use it instead of light1 in the 3D scene.
                pScene->GetProperties().SetObjectItem(makeSvx3DLightOnOff1Item(false));
                pScene->GetProperties().SetObjectItem(makeSvx3DLightOnOff4Item(true));
                pScene->GetProperties().SetObjectItem(makeSvx3DLightcolor4Item(aAmbientSpot1Color));
                pScene->GetProperties().SetObjectItem(makeSvx3DLightDirection4Item(aSpotLight1));
            }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
