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
#include <svl/whiter.hxx>
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
#include <com/sun/star/drawing/EnhancedCustomShapeMetalType.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <sdr/primitive2d/sdrattributecreator.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <comphelper/configuration.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace {

void GetOrigin( const SdrCustomShapeGeometryItem& rItem, double& rOriginX, double& rOriginY )
{
    css::drawing::EnhancedCustomShapeParameterPair aOriginParaPair;
    const Any* pAny = rItem.GetPropertyValueByName( u"Extrusion"_ustr, u"Origin"_ustr );
    if ( ! ( pAny && ( *pAny >>= aOriginParaPair ) && ( aOriginParaPair.First.Value >>= rOriginX ) && ( aOriginParaPair.Second.Value >>= rOriginY ) ) )
    {
        rOriginX = 0.50;
        rOriginY =-0.50;
    }
}

void GetRotateAngle( const SdrCustomShapeGeometryItem& rItem, double& rAngleX, double& rAngleY )
{
    css::drawing::EnhancedCustomShapeParameterPair aRotateAngleParaPair;
    const Any* pAny = rItem.GetPropertyValueByName( u"Extrusion"_ustr, u"RotateAngle"_ustr );
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
    const Any* pAny = rItem.GetPropertyValueByName( u"Extrusion"_ustr, u"Skew"_ustr );
    if ( ! ( pAny && ( *pAny >>= aSkewParaPair ) && ( aSkewParaPair.First.Value >>= rSkewAmount ) && ( aSkewParaPair.Second.Value >>= rSkewAngle ) ) )
    {
        rSkewAmount = 50;
        // ODF default is 45, but older ODF documents expect -135 as default. For intermediate
        // solution see tdf#141301 and tdf#141127.
        // MS Office default -135 is set in msdffimp.cxx to make import independent from setting here.
        rSkewAngle = -135;
    }
    rSkewAngle = basegfx::deg2rad(rSkewAngle);
}

void GetExtrusionDepth( const SdrCustomShapeGeometryItem& rItem, const double* pMap, double& rBackwardDepth, double& rForwardDepth )
{
    css::drawing::EnhancedCustomShapeParameterPair aDepthParaPair;
    double fDepth = 0, fFraction = 0;
    const Any* pAny = rItem.GetPropertyValueByName( u"Extrusion"_ustr, u"Depth"_ustr );
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
    const Any* pAny = rItem.GetPropertyValueByName( u"Extrusion"_ustr, rPropertyName );
    if ( pAny )
        *pAny >>= fRetValue;
    return fRetValue;
}

drawing::ShadeMode GetShadeMode( const SdrCustomShapeGeometryItem& rItem, const drawing::ShadeMode eDefault )
{
    drawing::ShadeMode eRet( eDefault );
    const Any* pAny = rItem.GetPropertyValueByName( u"Extrusion"_ustr, u"ShadeMode"_ustr );
    if ( pAny )
    {
        if (!(*pAny >>= eRet))
        {
            sal_Int32 nEnum = 0;
            if(*pAny >>= nEnum)
            {
                eRet = static_cast<drawing::ShadeMode>(nEnum);
            }
        }
    }
    return eRet;
}

bool GetBool( const SdrCustomShapeGeometryItem& rItem, const OUString& rPropertyName, const bool bDefault )
{
    bool bRetValue = bDefault;
    const Any* pAny = rItem.GetPropertyValueByName( u"Extrusion"_ustr, rPropertyName );
    if ( pAny )
        *pAny >>= bRetValue;
    return bRetValue;
}

drawing::Position3D GetPosition3D( const SdrCustomShapeGeometryItem& rItem, const OUString& rPropertyName,
                                    const drawing::Position3D& rDefault, const double* pMap )
{
    drawing::Position3D aRetValue( rDefault );
    const Any* pAny = rItem.GetPropertyValueByName( u"Extrusion"_ustr, rPropertyName );
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
    const Any* pAny = rItem.GetPropertyValueByName( u"Extrusion"_ustr, rPropertyName );
    if ( pAny )
        *pAny >>= aRetValue;
    return aRetValue;
}

sal_Int16 GetMetalType(const SdrCustomShapeGeometryItem& rItem, const sal_Int16 eDefault)
{
    sal_Int16 aRetValue(eDefault);
    const Any* pAny = rItem.GetPropertyValueByName(u"Extrusion"_ustr, u"MetalType"_ustr);
    if (pAny)
        *pAny >>= aRetValue;
    return aRetValue;
}

// Calculates the light directions for the additional lights, which are used to emulate soft
// lights of MS Office. Method needs to be documented in the Wiki
// https://wiki.documentfoundation.org/Development/ODF_Implementer_Notes in part
// List_of_LibreOffice_ODF_implementation-defined_items
// The method expects vector rLight to be normalized and results normalized vectors.
void lcl_SoftLightsDirection(const basegfx::B3DVector& rLight, basegfx::B3DVector& rSoftUp,
                             basegfx::B3DVector& rSoftDown, basegfx::B3DVector& rSoftRight,
                             basegfx::B3DVector& rSoftLeft)
{
    constexpr double fAngle = basegfx::deg2rad(60); // angle between regular light and soft light

    // We first create directions around (0|0|1) and then rotate them to the light position.
    rSoftUp = basegfx::B3DVector(0.0, sin(fAngle), cos(fAngle));
    rSoftDown = basegfx::B3DVector(0.0, -sin(fAngle), cos(fAngle));
    rSoftRight = basegfx::B3DVector(sin(fAngle), 0.0, cos(fAngle));
    rSoftLeft = basegfx::B3DVector(-sin(fAngle), 0.0, cos(fAngle));

    basegfx::B3DHomMatrix aRotateMat;
    aRotateMat.rotate(0.0, 0.0, M_PI_4);
    if (rLight.getX() == 0.0 && rLight.getZ() == 0.0)
    {
        // Special case with light from top or bottom
        if (rLight.getY() >= 0.0)
            aRotateMat.rotate(-M_PI_2, 0.0, 0.0);
        else
            aRotateMat.rotate(M_PI_2, 0.0, 0.0);
    }
    else
    {
        // Azimuth from z-axis to x-axis. (0|0|1) to (1|0|0) is 90deg.
        double fAzimuth = atan2(rLight.getX(), rLight.getZ());
        // Elevation from xz-plane to y-axis. (0|0|1) to (0|1|0) is 90deg.
        double fElevation = atan2(rLight.getY(), std::hypot(rLight.getX(), rLight.getZ()));
        aRotateMat.rotate(-fElevation, fAzimuth, 0.0);
    }

    rSoftUp = aRotateMat * rSoftUp;
    rSoftDown = aRotateMat * rSoftDown;
    rSoftRight = aRotateMat * rSoftRight;
    rSoftLeft = aRotateMat * rSoftLeft;
}
}

rtl::Reference<SdrObject> EnhancedCustomShape3d::Create3DObject(
    const SdrObject* pShape2d,
    const SdrObjCustomShape& rSdrObjCustomShape)
{
    rtl::Reference<SdrObject> pRet;
    const SdrCustomShapeGeometryItem& rGeometryItem(rSdrObjCustomShape.GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY));
    double fMap(1.0), *pMap = nullptr;

    if ( rSdrObjCustomShape.getSdrModelFromSdrObject().GetScaleUnit() != MapUnit::Map100thMM )
    {
        DBG_ASSERT( rSdrObjCustomShape.getSdrModelFromSdrObject().GetScaleUnit() == MapUnit::MapTwip, "EnhancedCustomShape3d::Current MapMode is Unsupported" );
        // But we could use MapToO3tlUnit from <tools/UnitConversion> ... ?
        fMap *= o3tl::convert(1.0, o3tl::Length::mm100, o3tl::Length::twip);
        pMap = &fMap;
    }

    if ( GetBool( rGeometryItem, u"Extrusion"_ustr, false ) )
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

        // tdf#146360 If the ItemSet of the source SdrObject has a parent
        // (which means it has a StyleSheet), we need to do some old-style
        // 'BurnInStyleSheetAttributes' action.
        // That means to set all Items which are set in the StyleSheet
        // directly in the ItemSet.
        // This is okay here since the 3D SdrObjects created are
        // placeholders that get rendered, but never reach the
        // surface/the user. If attributes for the source SdrObject
        // change, these will be recreated.
        // The problem is that while "aSet" still has a ptr to the style's
        // ItemSet, this gets lost at the ItemSet of the SdrObject when
        // an ItemSet gets set at the 3D SdrObject, like in diverse
        // SetMergedItemSet calls below. This leads to fetching the wrong
        // (default) FillBitmap in the calls p3DObj->GetMergedItem below
        // (which is 32x32 white, that's what you see without the fix).
        // This could also be fixed (tried it) by either
        // - using rSdrObjCustomShape.GetMergedItem
        // - setting the StyleSheet at 3D SdrObjects ASAP (done at caller)
        // but both solutions contain the risk to not find all places, so
        // it's just more safe to merge the StyleSheet attributes to the
        // ItemSet used for the whole creation.
        if(nullptr != aSet.GetParent())
        {
            SfxWhichIter aIter(aSet);
            sal_uInt16 nWhich(aIter.FirstWhich());
            const SfxPoolItem *pItem(nullptr);

            while(nWhich)
            {
                // this may look at 1st look like doing nothing, but it converts
                // items set in parent/style to SfxItemState::SET items in the
                // ItemSet (see AttributeProperties::ForceStyleToHardAttributes())
                if(SfxItemState::SET == aSet.GetItemState(nWhich, true, &pItem))
                {
                    aSet.Put(*pItem);
                }

                nWhich = aIter.NextWhich();
            }

            aSet.SetParent(nullptr);
        }

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
        const Any* pAny = rGeometryItem.GetPropertyValueByName( u"Extrusion"_ustr, u"ProjectionMode"_ustr );
        if (pAny)
        {
            if(!(*pAny >>= eProjectionMode))
            {
                sal_Int32 nEnum = 0;
                if(*pAny >>= nEnum)
                {
                    eProjectionMode = static_cast<drawing::ProjectionMode>(nEnum);
                }
            }
        }
        // pShape2d Convert in scenes which include 3D Objects
        E3dDefaultAttributes a3DDefaultAttr;
        a3DDefaultAttr.SetDefaultLatheCharacterMode( true );
        a3DDefaultAttr.SetDefaultExtrudeCharacterMode( true );

        rtl::Reference<E3dScene> pScene = new E3dScene(rSdrObjCustomShape.getSdrModelFromSdrObject());

        bool bSceneHasObjects ( false );
        bool bUseTwoFillStyles( false );

        drawing::ShadeMode eShadeMode( GetShadeMode( rGeometryItem, drawing::ShadeMode_FLAT ) );
        bool bUseExtrusionColor = GetBool( rGeometryItem, u"Color"_ustr, false );

        drawing::FillStyle eFillStyle( aSet.Get(XATTR_FILLSTYLE).GetValue() );
        pScene->GetProperties().SetObjectItem( Svx3DShadeModeItem(static_cast<sal_uInt16>(eShadeMode)));
        aSet.Put( makeSvx3DPercentDiagonalItem( 0 ) );
        aSet.Put( Svx3DTextureModeItem( 1 ) );
        // SPECIFIC needed for ShadeMode_SMOOTH and ShadeMode_PHONG, otherwise FLAT is faster.
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
        const bool bFuzzing(comphelper::IsFuzzing());

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

                if (bNeedToConvertToContour && !bFuzzing)
                {
                    rtl::Reference<SdrObject> pNewObj = pNext->ConvertToContourObj(const_cast< SdrObject* >(pNext));
                    SdrPathObj* pNewPathObj = dynamic_cast< SdrPathObj* >(pNewObj.get());

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
                }
                else
                {
                    aPolyPoly = pPathObj->GetPathPoly();
                }
            }
            else
            {
                rtl::Reference<SdrObject> pNewObj = pNext->ConvertToPolyObj( false, false );
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
                const tools::Rectangle aBoundRect(basegfx::fround<tools::Long>(aTempRange.getMinX()), basegfx::fround<tools::Long>(aTempRange.getMinY()), basegfx::fround<tools::Long>(aTempRange.getMaxX()), basegfx::fround<tools::Long>(aTempRange.getMaxY()));
                aTotalPolyPoly.append(aPolyPoly);
                aBoundRect2d.Union( aBoundRect );

                // #i122777# depth 0 is okay for planes when using double-sided
                rtl::Reference<E3dCompoundObject> p3DObj = new E3dExtrudeObj(
                    rSdrObjCustomShape.getSdrModelFromSdrObject(),
                    a3DDefaultAttr,
                    aPolyPoly,
                    bUseTwoFillStyles ? 0 : fDepth );

                p3DObj->NbcSetLayer( pShape2d->GetLayer() );
                p3DObj->SetMergedItemSet( aLocalSet );

                if ( bIsPlaceholderObject )
                    aPlaceholderObjectList.push_back( p3DObj.get() );
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
                    pScene->InsertObject( p3DObj.get() );
                    p3DObj = new E3dExtrudeObj(
                        rSdrObjCustomShape.getSdrModelFromSdrObject(),
                        a3DDefaultAttr,
                        aPolyPoly,
                        fDepth);
                    p3DObj->NbcSetLayer( pShape2d->GetLayer() );
                    p3DObj->SetMergedItemSet( aLocalSet );
                    if ( bUseExtrusionColor )
                        p3DObj->SetMergedItem( XFillColorItem( u""_ustr, rSdrObjCustomShape.GetMergedItem( XATTR_SECONDARYFILLCOLOR ).GetColorValue() ) );
                    p3DObj->SetMergedItem( XFillStyleItem( drawing::FillStyle_SOLID ) );
                    p3DObj->SetMergedItem( Svx3DCloseFrontItem( false ) );
                    p3DObj->SetMergedItem( Svx3DCloseBackItem( false ) );
                    pScene->InsertObject( p3DObj.get() );

                    // #i122777# depth 0 is okay for planes when using double-sided
                    p3DObj = new E3dExtrudeObj(
                        rSdrObjCustomShape.getSdrModelFromSdrObject(),
                        a3DDefaultAttr,
                        std::move(aPolyPoly),
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
                    p3DObj->SetMergedItem( XFillColorItem( u""_ustr, rLineColor.GetColorValue() ) );
                    p3DObj->SetMergedItem( makeSvx3DDoubleSidedItem( true ) );
                    p3DObj->SetMergedItem( Svx3DCloseFrontItem( false ) );
                    p3DObj->SetMergedItem( Svx3DCloseBackItem( false ) );
                }
                pScene->InsertObject( p3DObj.get() );
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
            double fW = aBoundRect2d.getOpenWidth();
            double fH = aBoundRect2d.getOpenHeight();
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
            drawing::Direction3D aRotationCenter( GetDirection3D( rGeometryItem, u"RotationCenter"_ustr, aRotationCenterDefault ) );
            aRotationCenter.DirectionX *= aSnapRect.getOpenWidth();
            aRotationCenter.DirectionY *= aSnapRect.getOpenHeight();
            if (pMap)
            {
                aRotationCenter.DirectionZ *= *pMap;
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

                // Property "Origin" in API is relative to bounding box of shape after 2D
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
                // API ViewPoint values are relative to point "Origin" and have y-axis down.
                // ToDo: These default ViewPoint values are used as default by MS Office. But ODF
                // default is (3500, -3500, 25000), details in tdf#146192.
                drawing::Position3D aViewPointDefault( 3472, -3472, 25000 );
                drawing::Position3D aViewPoint( GetPosition3D( rGeometryItem, u"ViewPoint"_ustr, aViewPointDefault, pMap ) );
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
            // transformations. Calculate its projection to the XY-plane. Then calculate the bounding
            // rectangle of the projection and convert this rectangle back to absolute 2D coordinates.
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


            // light and material

            // "LightFace" has nothing corresponding in 3D rendering engine.
            /* bool bLightFace = */ GetBool(rGeometryItem, u"LightFace"_ustr, true); // default in ODF

            // Light directions

            drawing::Direction3D aFirstLightDirectionDefault(50000.0, 0.0, 10000.0);
            drawing::Direction3D aFirstLightDirection(GetDirection3D( rGeometryItem, u"FirstLightDirection"_ustr, aFirstLightDirectionDefault));
            if (aFirstLightDirection.DirectionX == 0.0 && aFirstLightDirection.DirectionY == 0.0
                && aFirstLightDirection.DirectionZ == 0.0)
                aFirstLightDirection.DirectionZ = 1.0;
            basegfx::B3DVector aLight1Vector(aFirstLightDirection.DirectionX, -aFirstLightDirection.DirectionY, aFirstLightDirection.DirectionZ);
            aLight1Vector.normalize();

            drawing::Direction3D aSecondLightDirectionDefault(-50000.0, 0.0, 10000.0);
            drawing::Direction3D aSecondLightDirection(GetDirection3D( rGeometryItem, u"SecondLightDirection"_ustr, aSecondLightDirectionDefault));
            if (aSecondLightDirection.DirectionX == 0.0 && aSecondLightDirection.DirectionY == 0.0
                && aSecondLightDirection.DirectionZ == 0.0)
                aSecondLightDirection.DirectionZ = 1.0;
            basegfx::B3DVector aLight2Vector(aSecondLightDirection.DirectionX, -aSecondLightDirection.DirectionY, aSecondLightDirection.DirectionZ);
            aLight2Vector.normalize();

            // tdf#160421 a single flip inverts the light directions currently (March 2024). So invert
            // their directions here for rendering.
            if (bIsMirroredX != bIsMirroredY)
            {
                aLight1Vector *= -1.0;
                aLight2Vector *= -1.0;
            }

            // Light Intensity

            // For "FirstLight" the 3D-Scene light "1" is regularly used. In case of surface "Matte"
            // the light 4 is used instead. For "SecondLight" the 3D-Scene light "2" is regularly used.
            // In case first or second light is not harsh, the lights 5 to 8 are used in addition
            // to get a soft light appearance.
            // The 3D-Scene light "3" is currently not used.

            // ODF default 66%. MS Office default 38000/65536=0.579 is set in import filter.
            double fLight1Intensity = GetDouble(rGeometryItem, u"FirstLightLevel"_ustr, 66) / 100.0;
            // ODF and MS Office have both default 'true'.
            bool bFirstLightHarsh = GetBool(rGeometryItem, u"FirstLightHarsh"_ustr, true);
            // ODF default 66%. MS Office default 38000/65536=0.579 is set in import filter
            double fLight2Intensity = GetDouble(rGeometryItem, u"SecondLightLevel"_ustr, 66) / 100.0;
            // ODF has default 'true'. MS Office default 'false' is set in import.
            bool bSecondLightHarsh = GetBool(rGeometryItem, u"SecondLightHarsh"_ustr, true);

            // ODF default 33%. MS Office default 20000/65536=0.305 is set in import filter.
            double fAmbientIntensity = GetDouble(rGeometryItem, u"Brightness"_ustr, 33) / 100.0;

            double fLight1IntensityForSpecular(fLight1Intensity); // remember original value
            if (!bFirstLightHarsh || !bSecondLightHarsh) // might need softing lights
            {
                bool bNeedSoftLights(false); // catch case of lights with zero intensity.
                basegfx::B3DVector aLight5Vector;
                basegfx::B3DVector aLight6Vector;
                basegfx::B3DVector aLight7Vector;
                basegfx::B3DVector aLight8Vector;
                // The needed light intensities depend on the angle between regular light and
                // additional lights, currently for 60deg.
                Color aHoriSoftLightColor;
                Color aVertSoftLightColor;

                if (!bSecondLightHarsh && fLight2Intensity > 0.0
                    && (bFirstLightHarsh || fLight1Intensity == 0.0)) // only second light soft
                {
                    // That is default for shapes generated in the UI, for LO and MS Office as well.
                    bNeedSoftLights = true;
                    double fLight2SoftIntensity = fLight2Intensity * 0.40;
                    aHoriSoftLightColor = Color(basegfx::BColor(fLight2SoftIntensity).clamp());
                    aVertSoftLightColor = aHoriSoftLightColor;
                    fLight2Intensity *= 0.2;

                    lcl_SoftLightsDirection(aLight2Vector, aLight5Vector, aLight6Vector,
                                            aLight7Vector, aLight8Vector);
                }
                else if (!bFirstLightHarsh && fLight1Intensity > 0.0
                         && (bSecondLightHarsh || fLight2Intensity == 0.0)) // only first light soft
                {
                    bNeedSoftLights = true;
                    double fLight1SoftIntensity = fLight1Intensity * 0.40;
                    aHoriSoftLightColor = Color(basegfx::BColor(fLight1SoftIntensity).clamp());
                    aVertSoftLightColor = aHoriSoftLightColor;
                    fLight1Intensity *= 0.2;

                    lcl_SoftLightsDirection(aLight1Vector, aLight5Vector, aLight6Vector,
                                            aLight7Vector, aLight8Vector);
                }
                else if (!bFirstLightHarsh && fLight1Intensity > 0.0 && !bSecondLightHarsh
                         && fLight2Intensity > 0.0) // both lights soft
                {
                    bNeedSoftLights = true;
                    // We do not hat enough lights. We use two soft lights for FirstLight and two for
                    // SecondLight and double intensity.
                    double fLight1SoftIntensity = fLight1Intensity * 0.8;
                    fLight1Intensity *= 0.4;
                    aHoriSoftLightColor = Color(basegfx::BColor(fLight1SoftIntensity).clamp());
                    basegfx::B3DVector aDummy1, aDummy2;
                    lcl_SoftLightsDirection(aLight1Vector, aDummy1, aDummy2, aLight7Vector,
                                            aLight8Vector);

                    double fLight2SoftIntensity = fLight2Intensity * 0.8;
                    aVertSoftLightColor = Color(basegfx::BColor(fLight2SoftIntensity).clamp());
                    fLight2Intensity *= 0.4;
                    lcl_SoftLightsDirection(aLight2Vector, aLight5Vector, aLight6Vector, aDummy1,
                                            aDummy2);
                }

                if (bNeedSoftLights)
                {
                    pScene->GetProperties().SetObjectItem(
                        makeSvx3DLightDirection5Item(aLight5Vector));
                    pScene->GetProperties().SetObjectItem(
                        makeSvx3DLightcolor5Item(aVertSoftLightColor));
                    pScene->GetProperties().SetObjectItem(makeSvx3DLightOnOff5Item(true));
                    pScene->GetProperties().SetObjectItem(
                        makeSvx3DLightDirection6Item(aLight6Vector));
                    pScene->GetProperties().SetObjectItem(
                        makeSvx3DLightcolor6Item(aVertSoftLightColor));
                    pScene->GetProperties().SetObjectItem(makeSvx3DLightOnOff6Item(true));
                    pScene->GetProperties().SetObjectItem(
                        makeSvx3DLightDirection7Item(aLight7Vector));
                    pScene->GetProperties().SetObjectItem(
                        makeSvx3DLightcolor7Item(aHoriSoftLightColor));
                    pScene->GetProperties().SetObjectItem(makeSvx3DLightOnOff7Item(true));
                    pScene->GetProperties().SetObjectItem(
                        makeSvx3DLightDirection8Item(aLight8Vector));
                    pScene->GetProperties().SetObjectItem(
                        makeSvx3DLightcolor8Item(aHoriSoftLightColor));
                    pScene->GetProperties().SetObjectItem(makeSvx3DLightOnOff8Item(true));
                }
            }

            // ToDo: MSO seems to add half of the surplus to ambient color. ODF restricts value to <1.
            if (fLight1Intensity > 1.0)
            {
                fAmbientIntensity += (fLight1Intensity - 1.0) / 2.0;
            }

            // ToDo: How to handle fAmbientIntensity larger 1.0 ? Perhaps lighten object color?

            // Now set the regularly 3D-scene light attributes.
            Color aAmbientColor(basegfx::BColor(fAmbientIntensity).clamp());
            pScene->GetProperties().SetObjectItem(makeSvx3DAmbientcolorItem(aAmbientColor));

            pScene->GetProperties().SetObjectItem(makeSvx3DLightDirection1Item(aLight1Vector));
            pScene->GetProperties().SetObjectItem(makeSvx3DLightOnOff1Item(fLight1Intensity > 0.0));
            Color aLight1Color(basegfx::BColor(fLight1Intensity).clamp());
            pScene->GetProperties().SetObjectItem(makeSvx3DLightcolor1Item(aLight1Color));

            pScene->GetProperties().SetObjectItem(makeSvx3DLightDirection2Item(aLight2Vector));
            pScene->GetProperties().SetObjectItem(makeSvx3DLightOnOff2Item(fLight2Intensity > 0.0));
            Color aLight2Color(basegfx::BColor(fLight2Intensity).clamp());
            pScene->GetProperties().SetObjectItem(makeSvx3DLightcolor2Item(aLight2Color));

            // Object reactions on light
            // Diffusion, Specular-Color and -Intensity are object properties, not scene properties.
            // Surface flag "Metal" is an object property too.

            // Property "Diffusion" would correspond to style attribute "drd3:diffuse-color".
            // But that is not implemented. We cannot ignore the attribute because MS Office sets
            // attribute c3DDiffuseAmt to 43712 (Type Fixed 16.16, approx 66,9%) instead of MSO
            // default 65536 (100%), if the user sets surface 'Metal' in the UI of MS Office.
            // We will change the material color of the 3D object as ersatz.
            // ODF data type is percent with default 0%. MSO default is set in import filter.
            double fDiffusion = GetDouble(rGeometryItem, u"Diffusion"_ustr, 0.0) / 100.0;

            // ODF standard specifies for value true: "the specular color for the shading of an
            // extruded shape is gray (red, green and blue values of 200) instead of white and 15% is
            // added to the specularity."
            // Neither 'specularity' nor 'specular color' is clearly defined in the standard. ODF term
            // 'specularity' seems to correspond to UI field 'Specular Intensity' for 3D scenes.
            // MS Office uses current material color in case 'Metal' is set. To detect, whether
            // rendering similar to MS Office has to be used the property 'MetalType' is used. It is
            // set on import and in the extrusion bar.
            bool bMetal = GetBool(rGeometryItem, u"Metal"_ustr, false);
            sal_Int16 eMetalType(
                GetMetalType(rGeometryItem, drawing::EnhancedCustomShapeMetalType::MetalODF));
            bool bMetalMSCompatible
                = eMetalType == drawing::EnhancedCustomShapeMetalType::MetalMSCompatible;

            // Property "Specularity" corresponds to 3D object style attribute dr3d:specular-color.
            double fSpecularity = GetDouble(rGeometryItem, u"Specularity"_ustr, 0) / 100.0;

            if (bMetal && !bMetalMSCompatible)
            {
                fSpecularity *= 200.0 / 255.0;
            }

            // MS Office seems to render as if 'Specular Color' = Specularity * Light1Intensity.
            double fShadingFactor = fLight1IntensityForSpecular * fSpecularity;
            Color aSpecularCol(basegfx::BColor(fShadingFactor).clamp());
            // In case of bMetalMSCompatible the color will be recalculated in the below loop.

            // Shininess ODF default 50 (unit %). MS Office default 5, import filter makes *10.
            // Shininess corresponds to "Specular Intensity" with the nonlinear relationship
            // "Specular Intensity" = 2^c3DShininess = 2^("Shininess" / 10)
            double fShininess = GetDouble(rGeometryItem, u"Shininess"_ustr, 50) / 10.0;
            fShininess = std::clamp<double>(pow(2, fShininess), 0.0, 100.0);
            sal_uInt16 nIntensity = static_cast<sal_uInt16>(basegfx::fround(fShininess));
            if (bMetal && !bMetalMSCompatible)
            {
                nIntensity += 15; // as specified in ODF
                nIntensity = std::clamp<sal_uInt16>(nIntensity, 0, 100);
            }

            SdrObjListIter aSceneIter(*pScene, SdrIterMode::DeepNoGroups);
            while (aSceneIter.IsMore())
            {
                const SdrObject* pNext = aSceneIter.Next();

                // Change material color as ersatz for missing style attribute "drd3:diffuse-color".
                // For this ersatz we exclude case fDiffusion == 0.0, because for older documents this
                // attribute is not written out to draw:extrusion-diffusion and ODF default 0 would
                // produce black objects.
                const Color& rMatColor
                    = pNext->GetProperties().GetItem(XATTR_FILLCOLOR).GetColorValue();
                Color aOldMatColor(rMatColor);
                if (basegfx::fTools::more(fDiffusion, 0.0)
                    && !basegfx::fTools::equal(fDiffusion, 1.0))
                {
                    // Occurs e.g. with MS surface preset 'Metal'.
                    sal_uInt16 nHue;
                    sal_uInt16 nSaturation;
                    sal_uInt16 nBrightness;
                    rMatColor.RGBtoHSB(nHue, nSaturation, nBrightness);
                    nBrightness
                        = static_cast<sal_uInt16>(static_cast<double>(nBrightness) * fDiffusion);
                    nBrightness = std::clamp<sal_uInt16>(nBrightness, 0, 100);
                    Color aNewMatColor = Color::HSBtoRGB(nHue, nSaturation, nBrightness);
                    pNext->GetProperties().SetObjectItem(XFillColorItem(u""_ustr, aNewMatColor));
                }

                // Using material color instead of gray in case of MS Office compatible rendering.
                if (bMetal && bMetalMSCompatible)
                {
                    sal_uInt16 nHue;
                    sal_uInt16 nSaturation;
                    sal_uInt16 nBrightness;
                    aOldMatColor.RGBtoHSB(nHue, nSaturation, nBrightness);
                    nBrightness = static_cast<sal_uInt16>(static_cast<double>(nBrightness)
                                                          * fShadingFactor);
                    nBrightness = std::clamp<sal_uInt16>(nBrightness, 0, 100);
                    aSpecularCol = Color::HSBtoRGB(nHue, nSaturation, nBrightness);
                }

                pNext->GetProperties().SetObjectItem(makeSvx3DMaterialSpecularItem(aSpecularCol));
                pNext->GetProperties().SetObjectItem(
                    makeSvx3DMaterialSpecularIntensityItem(nIntensity));
            }

            // fSpecularity = 0 is used to indicate surface preset "Matte".
            if (basegfx::fTools::equalZero(fSpecularity))
            {
                // First light in LO 3D engine is always specular, all other lights are never specular.
                // We copy light1 values to light4 and use it instead of light1 in the 3D scene.
                pScene->GetProperties().SetObjectItem(makeSvx3DLightOnOff1Item(false));
                pScene->GetProperties().SetObjectItem(makeSvx3DLightOnOff4Item(true));
                pScene->GetProperties().SetObjectItem(makeSvx3DLightcolor4Item(aLight1Color));
                pScene->GetProperties().SetObjectItem(makeSvx3DLightDirection4Item(aLight1Vector));
            }

            // removing placeholder objects
            for (E3dCompoundObject* pTemp : aPlaceholderObjectList)
            {
                pScene->RemoveObject( pTemp->GetOrdNum() );
            }
        }
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
