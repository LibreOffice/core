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
#include <svx/polysc3d.hxx>
#include <svx/svddef.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/extrud3d.hxx>
#include <svx/xflbmtit.hxx>
#include <vcl/svapp.hxx>
#include <svx/xlnclit.hxx>
#include <svx/sdasitm.hxx>
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

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()
using namespace com::sun::star;
using namespace com::sun::star::uno;

const char aExtrusion[] = "Extrusion";

namespace {

void GetOrigin( SdrCustomShapeGeometryItem& rItem, double& rOriginX, double& rOriginY )
{
    ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aOriginParaPair;
     const OUString    sOrigin( "Origin" );
    Any* pAny = rItem.GetPropertyValueByName( OUString(aExtrusion), sOrigin );
    if ( ! ( pAny && ( *pAny >>= aOriginParaPair ) && ( aOriginParaPair.First.Value >>= rOriginX ) && ( aOriginParaPair.Second.Value >>= rOriginY ) ) )
    {
        rOriginX = 0.50;
        rOriginY =-0.50;
    }
}

void GetRotateAngle( SdrCustomShapeGeometryItem& rItem, double& rAngleX, double& rAngleY )
{
    ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aRotateAngleParaPair;
     const OUString    sRotateAngle( "RotateAngle" );
    Any* pAny = rItem.GetPropertyValueByName( OUString(aExtrusion), sRotateAngle );
    if ( ! ( pAny && ( *pAny >>= aRotateAngleParaPair ) && ( aRotateAngleParaPair.First.Value >>= rAngleX ) && ( aRotateAngleParaPair.Second.Value >>= rAngleY ) ) )
    {
        rAngleX = 0.0;
        rAngleY = 0.0;
    }
    rAngleX *= F_PI180;
    rAngleY *= F_PI180;
}

void GetSkew( SdrCustomShapeGeometryItem& rItem, double& rSkewAmount, double& rSkewAngle )
{
    ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aSkewParaPair;
     const OUString    sSkew( "Skew" );
    Any* pAny = rItem.GetPropertyValueByName( OUString(aExtrusion), sSkew );
    if ( ! ( pAny && ( *pAny >>= aSkewParaPair ) && ( aSkewParaPair.First.Value >>= rSkewAmount ) && ( aSkewParaPair.Second.Value >>= rSkewAngle ) ) )
    {
        rSkewAmount = 50;
        rSkewAngle = -135;
    }
    rSkewAngle *= F_PI180;
}

void GetExtrusionDepth( SdrCustomShapeGeometryItem& rItem, const double* pMap, double& rBackwardDepth, double& rForwardDepth )
{
    ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aDepthParaPair;
    double fDepth = 0, fFraction = 0;
    const OUString sDepth( "Depth" );
    Any* pAny = rItem.GetPropertyValueByName( OUString(aExtrusion), sDepth );
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

double GetDouble( SdrCustomShapeGeometryItem& rItem, const OUString& rPropertyName, double fDefault, const double* pMap )
{
    double fRetValue = fDefault;
    Any* pAny = rItem.GetPropertyValueByName( OUString(aExtrusion), rPropertyName );
    if ( pAny )
        *pAny >>= fRetValue;
    if ( pMap )
        fRetValue *= *pMap;
    return fRetValue;
}

drawing::ShadeMode GetShadeMode( SdrCustomShapeGeometryItem& rItem, const drawing::ShadeMode eDefault )
{
    drawing::ShadeMode eRet( eDefault );
    const OUString sShadeMode( "ShadeMode" );
    Any* pAny = rItem.GetPropertyValueByName( OUString(aExtrusion), sShadeMode );
    if ( pAny )
        *pAny >>= eRet;
    return eRet;
}

sal_Bool GetBool( SdrCustomShapeGeometryItem& rItem, const OUString& rPropertyName, const sal_Bool bDefault )
{
    sal_Bool bRetValue = bDefault;
    const Any* pAny = rItem.GetPropertyValueByName( OUString(aExtrusion), rPropertyName );
    if ( pAny )
        *pAny >>= bRetValue;
    return bRetValue;
}

drawing::Position3D GetPosition3D( SdrCustomShapeGeometryItem& rItem, const OUString& rPropertyName,
                                    const drawing::Position3D& rDefault, const double* pMap )
{
    drawing::Position3D aRetValue( rDefault );
    const Any* pAny = rItem.GetPropertyValueByName( OUString(aExtrusion), rPropertyName );
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

drawing::Direction3D GetDirection3D( SdrCustomShapeGeometryItem& rItem, const OUString& rPropertyName, const drawing::Direction3D& rDefault )
{
    drawing::Direction3D aRetValue( rDefault );
    const Any* pAny = rItem.GetPropertyValueByName( OUString(aExtrusion), rPropertyName );
    if ( pAny )
        *pAny >>= aRetValue;
    return aRetValue;
}

}

EnhancedCustomShape3d::Transformation2D::Transformation2D( const SdrObject* pCustomShape, const Rectangle& /*rBoundRect*/, const double *pM )
:   aCenter( pCustomShape->GetSnapRect().Center() )
,   eProjectionMode( drawing::ProjectionMode_PARALLEL )
,   pMap( pM )
{
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    const OUString sProjectionMode( "ProjectionMode" );
    Any* pAny = rGeometryItem.GetPropertyValueByName( OUString(aExtrusion), sProjectionMode );
    if ( pAny )
        *pAny >>= eProjectionMode;

    if ( eProjectionMode == drawing::ProjectionMode_PARALLEL )
        GetSkew( rGeometryItem, fSkew, fSkewAngle );
    else
    {
        fZScreen = 0.0;
        GetOrigin( rGeometryItem, fOriginX, fOriginY );
        fOriginX = fOriginX * pCustomShape->GetLogicRect().GetWidth();
        fOriginY = fOriginY * pCustomShape->GetLogicRect().GetHeight();

        const OUString sViewPoint( "ViewPoint" );
        drawing::Position3D aViewPointDefault( 3472, -3472, 25000 );
        drawing::Position3D aViewPoint( GetPosition3D( rGeometryItem, sViewPoint, aViewPointDefault, pMap ) );
        fViewPoint.setX(aViewPoint.PositionX);
        fViewPoint.setY(aViewPoint.PositionY);
        fViewPoint.setZ(-aViewPoint.PositionZ);
    }
}

basegfx::B3DPolygon EnhancedCustomShape3d::Transformation2D::ApplySkewSettings( const basegfx::B3DPolygon& rPoly3D ) const
{
    basegfx::B3DPolygon aRetval;

    sal_uInt32 j;
    for ( j = 0L; j < rPoly3D.count(); j++ )
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
        aPoint2D.X() = (sal_Int32)rPoint3D.getX();
        aPoint2D.Y() = (sal_Int32)rPoint3D.getY();
    }
    else
    {
        double fX = rPoint3D.getX() - fOriginX;
        double fY = rPoint3D.getY() - fOriginY;
        double f = ( fZScreen - fViewPoint.getZ() ) / ( rPoint3D.getZ() - fViewPoint.getZ() );
        aPoint2D.X() = (sal_Int32)(( fX - fViewPoint.getX() ) * f + fViewPoint.getX() + fOriginX );
        aPoint2D.Y() = (sal_Int32)(( fY - fViewPoint.getY() ) * f + fViewPoint.getY() + fOriginY );
    }
    aPoint2D.Move( aCenter.X(), aCenter.Y() );
    return aPoint2D;
}

bool EnhancedCustomShape3d::Transformation2D::IsParallel() const
{
    return eProjectionMode == com::sun::star::drawing::ProjectionMode_PARALLEL;
}

SdrObject* EnhancedCustomShape3d::Create3DObject( const SdrObject* pShape2d, const SdrObject* pCustomShape )
{
    SdrObject*  pRet = NULL;
    SdrModel*   pModel = pCustomShape->GetModel();
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );

    double      fMap, *pMap = NULL;
    if ( pModel )
    {
        fMap = 1.0;
        Fraction aFraction( pModel->GetScaleFraction() );
        if ( ( aFraction.GetNumerator() ) != 1 || ( aFraction.GetDenominator() != 1 ) )
        {
            fMap *= aFraction.GetNumerator();
            fMap /= aFraction.GetDenominator();
            pMap = &fMap;
        }
        if ( pModel->GetScaleUnit() != MAP_100TH_MM )
        {
            DBG_ASSERT( pModel->GetScaleUnit() == MAP_TWIP, "EnhancedCustomShape3d::Current MapMode is Unsupported" );
            fMap *= 1440.0 / 2540.0;
            pMap = &fMap;
        }
    }
    if ( GetBool( rGeometryItem, aExtrusion, sal_False ) )
    {
        sal_Bool bIsMirroredX = ((SdrObjCustomShape*)pCustomShape)->IsMirroredX();
        sal_Bool bIsMirroredY = ((SdrObjCustomShape*)pCustomShape)->IsMirroredY();
        Rectangle aSnapRect( pCustomShape->GetLogicRect() );
        long nObjectRotation = pCustomShape->GetRotateAngle();
        if ( nObjectRotation )
        {
            double a = ( 36000 - nObjectRotation ) * nPi180;
            long dx = aSnapRect.Right() - aSnapRect.Left();
            long dy = aSnapRect.Bottom()- aSnapRect.Top();
            Point aP( aSnapRect.TopLeft() );
            RotatePoint( aP, pCustomShape->GetSnapRect().Center(), sin( a ), cos( a ) );
            aSnapRect.Left() = aP.X();
            aSnapRect.Top() = aP.Y();
            aSnapRect.Right() = aSnapRect.Left() + dx;
            aSnapRect.Bottom() = aSnapRect.Top() + dy;
        }
        Point aCenter( aSnapRect.Center() );

        SfxItemSet aSet( pCustomShape->GetMergedItemSet() );

        //SJ: vertical writing is not required, by removing this item no outliner is created
        aSet.ClearItem( SDRATTR_TEXTDIRECTION );

        // #i105323# For 3D AutoShapes, the shadow attribute has to be applied to each
        // created visualisation helper model shape individually. The shadow itself
        // will then be rendered from the 3D renderer correctly for the whole 3D scene
        // (and thus behind all objects of which the visualisation may be built). So,
        // dio NOT remove it from the ItemSet here.
        // aSet.ClearItem(SDRATTR_SHADOW);

        std::vector< E3dCompoundObject* > aPlaceholderObjectList;

        double fExtrusionBackward, fExtrusionForward;
        GetExtrusionDepth( rGeometryItem, pMap, fExtrusionBackward, fExtrusionForward );
        double fDepth = fExtrusionBackward - fExtrusionForward;
        if ( fDepth < 1.0 )
            fDepth = 1.0;

        drawing::ProjectionMode eProjectionMode( drawing::ProjectionMode_PARALLEL );
        const OUString sProjectionMode( "ProjectionMode" );
        Any* pAny = rGeometryItem.GetPropertyValueByName( aExtrusion, sProjectionMode );
        if ( pAny )
            *pAny >>= eProjectionMode;
        ProjectionType eProjectionType( eProjectionMode == drawing::ProjectionMode_PARALLEL ? PR_PARALLEL : PR_PERSPECTIVE );
        // pShape2d Convert in scenes which include 3D Objects
        E3dDefaultAttributes a3DDefaultAttr;
        a3DDefaultAttr.SetDefaultLatheCharacterMode( sal_True );
        a3DDefaultAttr.SetDefaultExtrudeCharacterMode( sal_True );

        E3dScene* pScene = new E3dPolyScene( a3DDefaultAttr );

        bool bSceneHasObjects ( false );
        bool bUseTwoFillStyles( false );

        drawing::ShadeMode eShadeMode( GetShadeMode( rGeometryItem, drawing::ShadeMode_FLAT ) );
        const OUString aExtrusionColor( "Color" );
        sal_Bool bUseExtrusionColor = GetBool( rGeometryItem, aExtrusionColor, sal_False );

        XFillStyle eFillStyle( ITEMVALUE( aSet, XATTR_FILLSTYLE, XFillStyleItem ) );
        pScene->GetProperties().SetObjectItem( Svx3DShadeModeItem( 0 ) );
        aSet.Put( Svx3DPercentDiagonalItem( 0 ) );
        aSet.Put( Svx3DTextureModeItem( 1 ) );
        aSet.Put( Svx3DNormalsKindItem( 1 ) );

        if ( eShadeMode == drawing::ShadeMode_DRAFT )
        {
            aSet.Put( XLineStyleItem( XLINE_SOLID ) );
            aSet.Put( XFillStyleItem ( XFILL_NONE ) );
            aSet.Put( Svx3DDoubleSidedItem( sal_True ) );
        }
        else
        {
            aSet.Put( XLineStyleItem( XLINE_NONE ) );
            if ( eFillStyle == XFILL_NONE )
                aSet.Put( XFillStyleItem( XFILL_SOLID ) );
            else if ( ( eFillStyle == XFILL_BITMAP ) || ( eFillStyle == XFILL_GRADIENT ) || bUseExtrusionColor )
                bUseTwoFillStyles = true;

            // #116336#
            // If shapes are mirrored once (mirroring two times correct geometry again)
            // double-sided at the object and two-sided-lighting at the scene need to be set.
            if((bIsMirroredX && !bIsMirroredY) || (!bIsMirroredX && bIsMirroredY))
            {
                aSet.Put( Svx3DDoubleSidedItem( sal_True ) );
                pScene->GetProperties().SetObjectItem( Svx3DTwoSidedLightingItem( sal_True ) );
            }
        }

        Rectangle aBoundRect2d;
        SdrObjListIter aIter( *pShape2d, IM_DEEPNOGROUPS );
        const bool bMultipleSubObjects(aIter.Count() > 1);

        while( aIter.IsMore() )
        {
            const SdrObject* pNext = aIter.Next();
            bool bIsPlaceholderObject = (((XFillStyleItem&)pNext->GetMergedItem( XATTR_FILLSTYLE )).GetValue() == XFILL_NONE )
                                        && (((XLineStyleItem&)pNext->GetMergedItem( XATTR_LINESTYLE )).GetValue() == XLINE_NONE );
            basegfx::B2DPolyPolygon aPolyPoly;
            SfxItemSet aLocalSet(aSet);
            XFillStyle aLocalFillStyle(eFillStyle);

            if ( pNext->ISA( SdrPathObj ) )
            {
                const SfxItemSet& rSet = pNext->GetMergedItemSet();
                bool bNeedToConvertToContour(false);

                // do conversion only for single line objects; for all others a fill and a
                // line object get created. When we have fill, we want no line. That line has
                // always been there, but since it was never converted to contour, it kept
                // invisible (all this 'hidden' logic should be migrated to primitives).
                if(!bMultipleSubObjects)
                {
                    const XFillStyle eStyle(((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue());

                    if(XFILL_NONE == eStyle)
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
                                aLocalSet.Put(XLineStyleItem(XLINE_SOLID));
                                aLocalSet.Put(XLineWidthItem(40));
                                aLocalFillStyle = XFILL_NONE;
                            }
                            else
                            {
                                // switch from line to fill, copy line attr to fill attr (color, transparence)
                                aLocalSet.Put(XLineWidthItem(0));
                                aLocalSet.Put(XLineStyleItem(XLINE_NONE));
                                aLocalSet.Put(XFillColorItem(OUString(), ((const XLineColorItem&)(aLocalSet.Get(XATTR_LINECOLOR))).GetColorValue()));
                                aLocalSet.Put(XFillStyleItem(XFILL_SOLID));
                                aLocalSet.Put(XFillTransparenceItem(((const XLineTransparenceItem&)(aLocalSet.Get(XATTR_LINETRANSPARENCE))).GetValue()));
                                aLocalFillStyle = XFILL_SOLID;
                            }
                        }
                        else
                        {
                            // correct item properties to hairlines
                            aLocalSet.Put(XLineWidthItem(0));
                            aLocalSet.Put(XLineStyleItem(XLINE_SOLID));
                        }
                    }

                    SdrObject::Free(pNewObj);
                }
                else
                {
                    aPolyPoly = ((SdrPathObj*)pNext)->GetPathPoly();
                }
            }
            else
            {
                SdrObject* pNewObj = pNext->ConvertToPolyObj( sal_False, sal_False );
                SdrPathObj* pPath = PTR_CAST( SdrPathObj, pNewObj );
                if ( pPath )
                    aPolyPoly = pPath->GetPathPoly();
                SdrObject::Free( pNewObj );
            }

            if( aPolyPoly.count() )
            {
                if(aPolyPoly.areControlPointsUsed())
                {
                    aPolyPoly = basegfx::tools::adaptiveSubdivideByAngle(aPolyPoly);
                }

                const basegfx::B2DRange aTempRange(basegfx::tools::getRange(aPolyPoly));
                const Rectangle aBoundRect(basegfx::fround(aTempRange.getMinX()), basegfx::fround(aTempRange.getMinY()), basegfx::fround(aTempRange.getMaxX()), basegfx::fround(aTempRange.getMaxY()));
                aBoundRect2d.Union( aBoundRect );

                E3dCompoundObject* p3DObj = new E3dExtrudeObj( a3DDefaultAttr, aPolyPoly, bUseTwoFillStyles ? 10 : fDepth );
                p3DObj->NbcSetLayer( pShape2d->GetLayer() );
                p3DObj->SetMergedItemSet( aLocalSet );
                if ( bIsPlaceholderObject )
                    aPlaceholderObjectList.push_back( p3DObj );
                else if ( bUseTwoFillStyles )
                {
                    BitmapEx aFillBmp;
                    sal_Bool bFillBmpTile = ((XFillBmpTileItem&)p3DObj->GetMergedItem( XATTR_FILLBMP_TILE )).GetValue();
                    if ( bFillBmpTile )
                    {
                        const XFillBitmapItem& rBmpItm = (XFillBitmapItem&)p3DObj->GetMergedItem(XATTR_FILLBITMAP);
                        aFillBmp = rBmpItm.GetGraphicObject().GetGraphic().GetBitmapEx();
                        Size aLogicalSize = aFillBmp.GetPrefSize();
                        if ( aFillBmp.GetPrefMapMode() == MAP_PIXEL )
                            aLogicalSize = Application::GetDefaultDevice()->PixelToLogic( aLogicalSize, MAP_100TH_MM );
                        else
                            aLogicalSize = OutputDevice::LogicToLogic( aLogicalSize, aFillBmp.GetPrefMapMode(), MAP_100TH_MM );
                        aLogicalSize.Width()  *= 5;         ;//             :-(     nice scaling, look at engine3d/obj3d.cxx
                        aLogicalSize.Height() *= 5;
                        aFillBmp.SetPrefSize( aLogicalSize );
                        aFillBmp.SetPrefMapMode( MAP_100TH_MM );
                        p3DObj->SetMergedItem(XFillBitmapItem(String(), Graphic(aFillBmp)));
                    }
                    else
                    {
                        if ( aSnapRect != aBoundRect )
                        {
                            const XFillBitmapItem& rBmpItm = (XFillBitmapItem&)p3DObj->GetMergedItem(XATTR_FILLBITMAP);
                            aFillBmp = rBmpItm.GetGraphicObject().GetGraphic().GetBitmapEx();
                            Size aBmpSize( aFillBmp.GetSizePixel() );
                            double fXScale = (double)aBoundRect.GetWidth() / (double)aSnapRect.GetWidth();
                            double fYScale = (double)aBoundRect.GetHeight() / (double)aSnapRect.GetHeight();

                            Point aPt( (sal_Int32)( (double)( aBoundRect.Left() - aSnapRect.Left() )* (double)aBmpSize.Width() / (double)aSnapRect.GetWidth() ),
                                                (sal_Int32)( (double)( aBoundRect.Top() - aSnapRect.Top() ) * (double)aBmpSize.Height() / (double)aSnapRect.GetHeight() ) );
                            Size aSize( (sal_Int32)( aBmpSize.Width() * fXScale ),
                                                    (sal_Int32)( aBmpSize.Height() * fYScale ) );
                            Rectangle aCropRect( aPt, aSize );
                             aFillBmp.Crop( aCropRect );
                            p3DObj->SetMergedItem(XFillBitmapItem(String(), Graphic(aFillBmp)));
                        }
                    }
                    pScene->Insert3DObj( p3DObj );
                    p3DObj = new E3dExtrudeObj( a3DDefaultAttr, aPolyPoly, fDepth );
                    p3DObj->NbcSetLayer( pShape2d->GetLayer() );
                    p3DObj->SetMergedItemSet( aLocalSet );
                    if ( bUseExtrusionColor )
                        p3DObj->SetMergedItem( XFillColorItem( "", ((XSecondaryFillColorItem&)pCustomShape->GetMergedItem( XATTR_SECONDARYFILLCOLOR )).GetColorValue() ) );
                    p3DObj->SetMergedItem( XFillStyleItem( XFILL_SOLID ) );
                    p3DObj->SetMergedItem( Svx3DCloseFrontItem( sal_False ) );
                    p3DObj->SetMergedItem( Svx3DCloseBackItem( sal_False ) );
                    pScene->Insert3DObj( p3DObj );
                    p3DObj = new E3dExtrudeObj( a3DDefaultAttr, aPolyPoly, 10 );
                    p3DObj->NbcSetLayer( pShape2d->GetLayer() );
                    p3DObj->SetMergedItemSet( aLocalSet );

                    basegfx::B3DHomMatrix aFrontTransform( p3DObj->GetTransform() );
                    aFrontTransform.translate( 0.0, 0.0, fDepth );
                    p3DObj->NbcSetTransform( aFrontTransform );

                    if ( ( aLocalFillStyle == XFILL_BITMAP ) && !aFillBmp.IsEmpty() )
                    {
                        p3DObj->SetMergedItem(XFillBitmapItem(String(), Graphic(aFillBmp)));
                    }
                }
                else if ( aLocalFillStyle == XFILL_NONE )
                {
                    XLineColorItem& rLineColor = (XLineColorItem&)p3DObj->GetMergedItem( XATTR_LINECOLOR );
                    p3DObj->SetMergedItem( XFillColorItem( "", rLineColor.GetColorValue() ) );
                    p3DObj->SetMergedItem( Svx3DDoubleSidedItem( sal_True ) );
                    p3DObj->SetMergedItem( Svx3DCloseFrontItem( sal_False ) );
                    p3DObj->SetMergedItem( Svx3DCloseBackItem( sal_False ) );
                }
                pScene->Insert3DObj( p3DObj );
                bSceneHasObjects = true;
            }
        }

        if ( bSceneHasObjects ) // is the SdrObject properly converted
        {
            // then we can change the return value
            pRet = pScene;

            // Camera settings, Perspective ...
            Camera3D& rCamera = (Camera3D&)pScene->GetCamera();
            const basegfx::B3DRange& rVolume = pScene->GetBoundVolume();
            pScene->NbcSetSnapRect( aSnapRect );

            // InitScene replacement
            double fW = rVolume.getWidth();
            double fH = rVolume.getHeight();

            rCamera.SetAutoAdjustProjection( sal_False );
            rCamera.SetViewWindow( -fW / 2, - fH / 2, fW, fH);
            basegfx::B3DPoint aLookAt( 0.0, 0.0, 0.0 );
            basegfx::B3DPoint aCamPos( 0.0, 0.0, 100.0 );
            rCamera.SetDefaults( basegfx::B3DPoint( 0.0, 0.0, 100.0 ), aLookAt, 100.0 );
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
            double fZRotate = ((SdrObjCustomShape*)pCustomShape)->GetObjectRotation() * F_PI180;
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
            if ( eProjectionType == PR_PARALLEL )
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
                const OUString sViewPoint( "ViewPoint" );
                drawing::Position3D aViewPointDefault( 3472, -3472, 25000 );
                drawing::Position3D aViewPoint( GetPosition3D( rGeometryItem, sViewPoint, aViewPointDefault, pMap ) );
                double fViewPointX = aViewPoint.PositionX;
                double fViewPointY = aViewPoint.PositionY;
                double fViewPointZ = aViewPoint.PositionZ;
                basegfx::B3DPoint _aLookAt( fViewPointX, -fViewPointY, 0.0 );
                basegfx::B3DPoint aNewCamPos( fViewPointX, -fViewPointY, fViewPointZ );
                rCamera.SetPosAndLookAt( aNewCamPos, _aLookAt );
                pScene->SetCamera( rCamera );
            }

            pScene->NbcSetTransform( aNewTransform );

            ///////////
            // light //
            ///////////

            const OUString sBrightness( "Brightness" );
            double fAmbientIntensity = GetDouble( rGeometryItem, sBrightness, 22178.0 / 655.36, NULL ) / 100.0;


            const OUString sFirstLightDirection( "FirstLightDirection" );
            drawing::Direction3D aFirstLightDirectionDefault( 50000, 0, 10000 );
            drawing::Direction3D aFirstLightDirection( GetDirection3D( rGeometryItem, sFirstLightDirection, aFirstLightDirectionDefault ) );
            if ( aFirstLightDirection.DirectionZ == 0.0 )
                aFirstLightDirection.DirectionZ = 1.0;

            const OUString sFirstLightLevel( "FirstLightLevel" );
            double fLightIntensity = GetDouble( rGeometryItem, sFirstLightLevel, 43712.0 / 655.36, NULL ) / 100.0;

            const OUString sFirstLightHarsh( "FirstLightHarsh" );
            /* sal_Bool bFirstLightHarsh = */ GetBool( rGeometryItem, sFirstLightHarsh, sal_False );

            const OUString sSecondLightDirection( "SecondLightDirection" );
            drawing::Direction3D aSecondLightDirectionDefault( -50000, 0, 10000 );
            drawing::Direction3D aSecondLightDirection( GetDirection3D( rGeometryItem, sSecondLightDirection, aSecondLightDirectionDefault ) );
            if ( aSecondLightDirection.DirectionZ == 0.0 )
                aSecondLightDirection.DirectionZ = -1;

            const OUString sSecondLightLevel( "SecondLightLevel" );
            double fLight2Intensity = GetDouble( rGeometryItem, sSecondLightLevel, 43712.0 / 655.36, NULL ) / 100.0;

            const OUString sSecondLightHarsh( "SecondLightHarsh" );
            const OUString sLightFace( "LightFace" );
            /* sal_Bool bLight2Harsh = */ GetBool( rGeometryItem, sSecondLightHarsh, sal_False );
            /* sal_Bool bLightFace = */ GetBool( rGeometryItem, sLightFace, sal_False );

            sal_uInt16 nAmbientColor = (sal_uInt16)( fAmbientIntensity * 255.0 );
            if ( nAmbientColor > 255 )
                nAmbientColor = 255;
            Color aGlobalAmbientColor( (sal_uInt8)nAmbientColor, (sal_uInt8)nAmbientColor, (sal_uInt8)nAmbientColor );
            pScene->GetProperties().SetObjectItem( Svx3DAmbientcolorItem( aGlobalAmbientColor ) );

            sal_uInt8 nSpotLight1 = (sal_uInt8)( fLightIntensity * 255.0 );
            basegfx::B3DVector aSpotLight1( aFirstLightDirection.DirectionX, - ( aFirstLightDirection.DirectionY ), -( aFirstLightDirection.DirectionZ ) );
            aSpotLight1.normalize();
            pScene->GetProperties().SetObjectItem( Svx3DLightOnOff1Item( sal_True ) );
            Color aAmbientSpot1Color( nSpotLight1, nSpotLight1, nSpotLight1 );
            pScene->GetProperties().SetObjectItem( Svx3DLightcolor1Item( aAmbientSpot1Color ) );
            pScene->GetProperties().SetObjectItem( Svx3DLightDirection1Item( aSpotLight1 ) );

            sal_uInt8 nSpotLight2 = (sal_uInt8)( fLight2Intensity * 255.0 );
            basegfx::B3DVector aSpotLight2( aSecondLightDirection.DirectionX, -aSecondLightDirection.DirectionY, -aSecondLightDirection.DirectionZ );
            aSpotLight2.normalize();
            pScene->GetProperties().SetObjectItem( Svx3DLightOnOff2Item( sal_True ) );
            Color aAmbientSpot2Color( nSpotLight2, nSpotLight2, nSpotLight2 );
            pScene->GetProperties().SetObjectItem( Svx3DLightcolor2Item( aAmbientSpot2Color ) );
            pScene->GetProperties().SetObjectItem( Svx3DLightDirection2Item( aSpotLight2 ) );

                sal_uInt8 nSpotLight3 = 70;
                basegfx::B3DVector aSpotLight3( 0.0, 0.0, 1.0 );
                pScene->GetProperties().SetObjectItem( Svx3DLightOnOff3Item( sal_True ) );
                Color aAmbientSpot3Color( nSpotLight3, nSpotLight3, nSpotLight3 );
                pScene->GetProperties().SetObjectItem( Svx3DLightcolor3Item( aAmbientSpot3Color ) );
                pScene->GetProperties().SetObjectItem( Svx3DLightDirection3Item( aSpotLight3 ) );

            const OUString sSpecularity( "Specularity" );
            const OUString sMetal( "Metal" );
            double fSpecular = GetDouble( rGeometryItem, sSpecularity, 0, NULL ) / 100;
            sal_Bool bMetal = GetBool( rGeometryItem, sMetal, sal_False );

            Color aSpecularCol( 225,225,225 );
            if ( bMetal )
            {
                aSpecularCol = Color( 200, 200, 200 );
                fSpecular += 0.15;
            }
            sal_Int32 nIntensity = (sal_Int32)fSpecular * 100;
            if ( nIntensity > 100 )
                nIntensity = 100;
            else if ( nIntensity < 0 )
                nIntensity = 0;
            nIntensity = 100 - nIntensity;
            pScene->GetProperties().SetObjectItem( Svx3DMaterialSpecularItem( aSpecularCol ) );
            pScene->GetProperties().SetObjectItem( Svx3DMaterialSpecularIntensityItem( (sal_uInt16)nIntensity ) );

            pScene->SetLogicRect( CalculateNewSnapRect( pCustomShape, aSnapRect, aBoundRect2d, pMap ) );

            // removing placeholder objects
            std::vector< E3dCompoundObject* >::iterator aObjectListIter( aPlaceholderObjectList.begin() );
            while ( aObjectListIter != aPlaceholderObjectList.end() )
            {
                pScene->Remove3DObj( *aObjectListIter );
                delete *aObjectListIter++;
            }
        }
        else
            delete pScene;
    }
    return pRet;
}

Rectangle EnhancedCustomShape3d::CalculateNewSnapRect( const SdrObject* pCustomShape, const Rectangle& rSnapRect, const Rectangle& rBoundRect, const double* pMap )
{
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    const Point aCenter( rSnapRect.Center() );
    double fExtrusionBackward, fExtrusionForward;
    GetExtrusionDepth( rGeometryItem, pMap, fExtrusionBackward, fExtrusionForward );
    sal_uInt32 i;

    // creating initial bound volume ( without rotation. skewing.and camera )
    basegfx::B3DPolygon aBoundVolume;
    const Polygon aPolygon( rBoundRect );

    for ( i = 0L; i < 4L; i++ )
    {
        aBoundVolume.append(basegfx::B3DPoint(aPolygon[ (sal_uInt16)i ].X() - aCenter.X(), aPolygon[ (sal_uInt16)i ].Y() - aCenter.Y(), fExtrusionForward));
    }

    for ( i = 0L; i < 4L; i++ )
    {
        aBoundVolume.append(basegfx::B3DPoint(aPolygon[ (sal_uInt16)i ].X() - aCenter.X(), aPolygon[ (sal_uInt16)i ].Y() - aCenter.Y(), fExtrusionBackward));
    }

    const OUString sRotationCenter( "RotationCenter" );
    drawing::Direction3D aRotationCenterDefault( 0, 0, 0 ); // default seems to be wrong, a fractional size of shape has to be used!!
    drawing::Direction3D aRotationCenter( GetDirection3D( rGeometryItem, sRotationCenter, aRotationCenterDefault ) );

    double fXRotate, fYRotate;
    GetRotateAngle( rGeometryItem, fXRotate, fYRotate );
    double fZRotate = - ((SdrObjCustomShape*)pCustomShape)->GetObjectRotation() * F_PI180;

    // rotating bound volume
    basegfx::B3DHomMatrix aMatrix;
    aMatrix.translate(-aRotationCenter.DirectionX, -aRotationCenter.DirectionY, -aRotationCenter.DirectionZ);
    if ( fZRotate != 0.0 )
        aMatrix.rotate( 0.0, 0.0, fZRotate );
    if ( ((SdrObjCustomShape*)pCustomShape)->IsMirroredX() )
        aMatrix.scale( -1.0, 1, 1 );
    if ( ((SdrObjCustomShape*)pCustomShape)->IsMirroredY() )
        aMatrix.scale( 1, -1.0, 1 );
    if( fYRotate != 0.0 )
        aMatrix.rotate( 0.0, fYRotate, 0.0 );
    if( fXRotate != 0.0 )
        aMatrix.rotate( -fXRotate, 0.0, 0.0 );
    aMatrix.translate(aRotationCenter.DirectionX, aRotationCenter.DirectionY, aRotationCenter.DirectionZ);
    aBoundVolume.transform(aMatrix);

    Transformation2D aTransformation2D( pCustomShape, rSnapRect, pMap );
    if ( aTransformation2D.IsParallel() )
        aBoundVolume = aTransformation2D.ApplySkewSettings( aBoundVolume );

    Polygon aTransformed( 8 );
    for ( i = 0L; i < 8L; i++ )
        aTransformed[ (sal_uInt16)i ] = aTransformation2D.Transform2D( aBoundVolume.getB3DPoint( i ) );

    return aTransformed.GetBoundRect();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
