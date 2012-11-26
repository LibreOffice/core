/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
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
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <svx/sdr/properties/properties.hxx>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/svdtrans.hxx>
#include <svx/scene3d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()
using namespace com::sun::star;
using namespace com::sun::star::uno;

const rtl::OUString sExtrusion( RTL_CONSTASCII_USTRINGPARAM ( "Extrusion" ) );

void GetOrigin( SdrCustomShapeGeometryItem& rItem, double& rOriginX, double& rOriginY )
{
    ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aOriginParaPair;
     const rtl::OUString    sOrigin( RTL_CONSTASCII_USTRINGPARAM ( "Origin" ) );
    Any* pAny = rItem.GetPropertyValueByName( sExtrusion, sOrigin );
    if ( ! ( pAny && ( *pAny >>= aOriginParaPair ) && ( aOriginParaPair.First.Value >>= rOriginX ) && ( aOriginParaPair.Second.Value >>= rOriginY ) ) )
    {
        rOriginX = 0.50;
        rOriginY =-0.50;
    }
}

void GetRotateAngle( SdrCustomShapeGeometryItem& rItem, double& rAngleX, double& rAngleY )
{
    ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aRotateAngleParaPair;
     const rtl::OUString    sRotateAngle( RTL_CONSTASCII_USTRINGPARAM ( "RotateAngle" ) );
    Any* pAny = rItem.GetPropertyValueByName( sExtrusion, sRotateAngle );
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
     const rtl::OUString    sSkew( RTL_CONSTASCII_USTRINGPARAM ( "Skew" ) );
    Any* pAny = rItem.GetPropertyValueByName( sExtrusion, sSkew );
    if ( ! ( pAny && ( *pAny >>= aSkewParaPair ) && ( aSkewParaPair.First.Value >>= rSkewAmount ) && ( aSkewParaPair.Second.Value >>= rSkewAngle ) ) )
    {
        rSkewAmount = 50;
        rSkewAngle = -135;
    }
    rSkewAngle *= F_PI180;
}

void GetExtrusionDepth( SdrCustomShapeGeometryItem& rItem, const double fMap, double& rBackwardDepth, double& rForwardDepth )
{
    ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aDepthParaPair;
    double fDepth = 0, fFraction = 0;
    const rtl::OUString sDepth( RTL_CONSTASCII_USTRINGPARAM ( "Depth" ) );
    Any* pAny = rItem.GetPropertyValueByName( sExtrusion, sDepth );
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

    rBackwardDepth *= fMap;
    rForwardDepth *= fMap;
}

double GetDouble( SdrCustomShapeGeometryItem& rItem, const rtl::OUString& rPropertyName, double fDefault, const double fMap )
{
    double fRetValue = fDefault;
    Any* pAny = rItem.GetPropertyValueByName( sExtrusion, rPropertyName );
    if ( pAny )
        *pAny >>= fRetValue;

    fRetValue *= fMap;

    return fRetValue;
}

drawing::ShadeMode GetShadeMode( SdrCustomShapeGeometryItem& rItem, const drawing::ShadeMode eDefault )
{
    drawing::ShadeMode eRet( eDefault );
    const rtl::OUString sShadeMode( RTL_CONSTASCII_USTRINGPARAM ( "ShadeMode" ) );
    Any* pAny = rItem.GetPropertyValueByName( sExtrusion, sShadeMode );
    if ( pAny )
        *pAny >>= eRet;
    return eRet;
}

sal_Int32 GetInt32( SdrCustomShapeGeometryItem& rItem, const rtl::OUString& rPropertyName, const sal_Int32 nDefault )
{
    sal_Int32 nRetValue = nDefault;
    Any* pAny = rItem.GetPropertyValueByName( sExtrusion, rPropertyName );
    if ( pAny )
        *pAny >>= nRetValue;
    return nRetValue;
}

sal_Bool GetBool( SdrCustomShapeGeometryItem& rItem, const rtl::OUString& rPropertyName, const sal_Bool bDefault )
{
    sal_Bool bRetValue = bDefault;
    const Any* pAny = rItem.GetPropertyValueByName( sExtrusion, rPropertyName );
    if ( pAny )
        *pAny >>= bRetValue;
    return bRetValue;
}

awt::Point GetPoint( SdrCustomShapeGeometryItem& rItem, const rtl::OUString& rPropertyName, const awt::Point& rDefault )
{
    awt::Point aRetValue( rDefault );
    const Any* pAny = rItem.GetPropertyValueByName( sExtrusion, rPropertyName );
    if ( pAny )
        *pAny >>= aRetValue;
    return aRetValue;
}

drawing::Position3D GetPosition3D( SdrCustomShapeGeometryItem& rItem, const rtl::OUString& rPropertyName,
    const drawing::Position3D& rDefault, const double fMap )
{
    drawing::Position3D aRetValue( rDefault );
    const Any* pAny = rItem.GetPropertyValueByName( sExtrusion, rPropertyName );
    if ( pAny )
        *pAny >>= aRetValue;

    aRetValue.PositionX *= fMap;
    aRetValue.PositionY *= fMap;
    aRetValue.PositionZ *= fMap;

    return aRetValue;
}

drawing::Direction3D GetDirection3D( SdrCustomShapeGeometryItem& rItem, const rtl::OUString& rPropertyName, const drawing::Direction3D& rDefault )
{
    drawing::Direction3D aRetValue( rDefault );
    const Any* pAny = rItem.GetPropertyValueByName( sExtrusion, rPropertyName );
    if ( pAny )
        *pAny >>= aRetValue;
    return aRetValue;
}

EnhancedCustomShape3d::Transformation2D::Transformation2D(const SdrObjCustomShape& rCustomShape, const double fM)
:   maCenter()
,   meProjectionMode(drawing::ProjectionMode_PARALLEL)
,   mfMap(fM)
{
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)rCustomShape.GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
    const rtl::OUString sProjectionMode(RTL_CONSTASCII_USTRINGPARAM("ProjectionMode"));
    Any* pAny = rGeometryItem.GetPropertyValueByName(sExtrusion, sProjectionMode);
    maCenter = rCustomShape.getSdrObjectTransformation() * basegfx::B2DPoint(0.5, 0.5);

    if(pAny)
    {
        *pAny >>= meProjectionMode;
    }

    if(drawing::ProjectionMode_PARALLEL == meProjectionMode)
    {
        GetSkew(rGeometryItem, mfSkew, mfSkewAngle);
    }
    else
    {
        const basegfx::B2DVector& rScale = rCustomShape.getSdrObjectScale();
        const rtl::OUString sViewPoint(RTL_CONSTASCII_USTRINGPARAM("ViewPoint"));

        mfZScreen = 0.0;
        GetOrigin(rGeometryItem, mfOriginX, mfOriginY);
        mfOriginX *= fabs(rScale.getX());
        mfOriginY *= fabs(rScale.getY());
        drawing::Position3D aViewPointDefault(3472, -3472, 25000);
        drawing::Position3D aViewPoint(GetPosition3D(rGeometryItem, sViewPoint, aViewPointDefault, mfMap));
        mfViewPoint.setX(aViewPoint.PositionX);
        mfViewPoint.setY(aViewPoint.PositionY);
        mfViewPoint.setZ(-aViewPoint.PositionZ);
    }
}

basegfx::B3DPolygon EnhancedCustomShape3d::Transformation2D::ApplySkewSettings(const basegfx::B3DPolygon& rPoly3D) const
{
    basegfx::B3DPolygon aRetval;
    sal_uInt32 j;

    for(j = 0; j < rPoly3D.count(); j++)
    {
        const basegfx::B3DPoint aPoint(rPoly3D.getB3DPoint(j));
        double fDepth(-(aPoint.getZ() * mfSkew) / 100.0);

        aRetval.append(
            basegfx::B3DPoint(
                aPoint.getX() + (fDepth * cos(mfSkewAngle)),
                aPoint.getY() - (fDepth * sin(mfSkewAngle)),
                aPoint.getZ()));
    }

    return aRetval;
}

basegfx::B2DPoint EnhancedCustomShape3d::Transformation2D::Transform2D(const basegfx::B3DPoint& rPoint3D) const
{
    basegfx::B2DPoint aPoint2D;

    if(drawing::ProjectionMode_PARALLEL == meProjectionMode)
    {
        aPoint2D.setX(rPoint3D.getX());
        aPoint2D.setY(rPoint3D.getY());
    }
    else
    {
        const double fX(rPoint3D.getX() - mfOriginX);
        const double fY(rPoint3D.getY() - mfOriginY);
        const double f((mfZScreen - mfViewPoint.getZ()) / (rPoint3D.getZ() - mfViewPoint.getZ()));

        aPoint2D.setX((fX - mfViewPoint.getX()) * f + mfViewPoint.getX() + mfOriginX);
        aPoint2D.setY((fY - mfViewPoint.getY()) * f + mfViewPoint.getY() + mfOriginY);
    }

    aPoint2D += maCenter;

    return aPoint2D;
}

bool EnhancedCustomShape3d::Transformation2D::IsParallel() const
{
    return meProjectionMode == com::sun::star::drawing::ProjectionMode_PARALLEL;
}

SdrObject* EnhancedCustomShape3d::Create3DObject(const SdrObject& rShape2d, const SdrObjCustomShape& rCustomShape)
{
    SdrObject* pRet = 0;
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)rCustomShape.GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
    double fMap(1.0);
    const double fFraction(rCustomShape.getSdrModelFromSdrObject().GetExchangeObjectScale());

    if(!basegfx::fTools::equal(fFraction, 1.0))
    {
        fMap *= fFraction;
    }

    if(MAP_100TH_MM != rCustomShape.getSdrModelFromSdrObject().GetExchangeObjectUnit())
    {
        DBG_ASSERT(MAP_TWIP == rCustomShape.getSdrModelFromSdrObject().GetExchangeObjectUnit(),
            "EnhancedCustomShape3d::Current MapMode is Unsupported");
        fMap *= 1440.0 / 2540.0;
    }

    if(GetBool(rGeometryItem, sExtrusion, false))
    {
        const bool bIsMirroredX(rShape2d.isMirroredX()); // TTTT: rCustomShape.IsMirroredX()); Check if rShape2d is correct here
        const bool bIsMirroredY(rShape2d.isMirroredY()); // TTTT: rCustomShape.IsMirroredY()); And if needed at all...
        basegfx::B2DRange aSnapRange(basegfx::B2DRange::getUnitB2DRange());
        SfxItemSet aSet(rCustomShape.GetMergedItemSet());
        std::vector< E3dCompoundObject* > aPlaceholderObjectList;
        double fExtrusionBackward(0.0), fExtrusionForward(0.0);

        // take transformation into account, including rotation and shear
        aSnapRange.transform(rCustomShape.getSdrObjectTransformation());

        //SJ: vertical writing is not required, by removing this item no outliner is created
        aSet.ClearItem(SDRATTR_TEXTDIRECTION);

        // #i105323# For 3D AutoShapes, the shadow attribute has to be applied to each
        // created visualisation helper model shape individually. The shadow itself
        // will then be rendered from the 3D renderer correctly for the whole 3D scene
        // (and thus behind all objects of which the visualisation may be built). So,
        // do NOT remove it from the ItemSet here.
        // aSet.ClearItem(SDRATTR_SHADOW);

        GetExtrusionDepth(rGeometryItem, fMap, fExtrusionBackward, fExtrusionForward);
        double fDepth(fExtrusionBackward - fExtrusionForward);

        if(fDepth < 1.0)
        {
            fDepth = 1.0;
        }

        drawing::ProjectionMode eProjectionMode(drawing::ProjectionMode_PARALLEL);
        const rtl::OUString sProjectionMode(RTL_CONSTASCII_USTRINGPARAM("ProjectionMode"));
        Any* pAny = rGeometryItem.GetPropertyValueByName(sExtrusion, sProjectionMode);

        if(pAny)
        {
            *pAny >>= eProjectionMode;
        }

        ProjectionType eProjectionType(drawing::ProjectionMode_PARALLEL == eProjectionMode ? PR_PARALLEL : PR_PERSPECTIVE);

        // rShape2d Umwandeln in Szene mit 3D Objekt
        E3dDefaultAttributes a3DDefaultAttr;

        a3DDefaultAttr.SetDefaultLatheCharacterMode(true);
        a3DDefaultAttr.SetDefaultExtrudeCharacterMode(true);

        E3dScene* pScene = new E3dScene(rCustomShape.getSdrModelFromSdrObject(), a3DDefaultAttr);

        bool bSceneHasObjects(false);
        bool bUseTwoFillStyles(false);
        const drawing::ShadeMode eShadeMode(GetShadeMode(rGeometryItem, drawing::ShadeMode_FLAT));
        const rtl::OUString sExtrusionColor(RTL_CONSTASCII_USTRINGPARAM("Color"));
        const bool bUseExtrusionColor(GetBool(rGeometryItem, sExtrusionColor, false));
        const XFillStyle eFillStyle(ITEMVALUE(aSet, XATTR_FILLSTYLE, XFillStyleItem));

        pScene->GetProperties().SetObjectItem(Svx3DShadeModeItem(0));
        aSet.Put(SfxUInt16Item(SDRATTR_3DOBJ_PERCENT_DIAGONAL, 0));
        aSet.Put(Svx3DTextureModeItem(1));
        aSet.Put(Svx3DNormalsKindItem(1));

        if(drawing::ShadeMode_DRAFT == eShadeMode)
        {
            aSet.Put(XLineStyleItem(XLINE_SOLID));
            aSet.Put(XFillStyleItem(XFILL_NONE));
            aSet.Put(SfxBoolItem(SDRATTR_3DOBJ_DOUBLE_SIDED, true));
        }
        else
        {
            aSet.Put(XLineStyleItem(XLINE_NONE));

            if(XFILL_NONE == eFillStyle)
            {
                aSet.Put(XFillStyleItem(XFILL_SOLID));
            }
            else if(XFILL_BITMAP == eFillStyle || XFILL_GRADIENT == eFillStyle || bUseExtrusionColor)
            {
                bUseTwoFillStyles = true;
            }

            // #116336#
            // If shapes are mirrored once (mirroring two times correct geometry again)
            // double-sided at the object and two-sided-lighting at the scene need to be set.
            if((bIsMirroredX && !bIsMirroredY) || (!bIsMirroredX && bIsMirroredY))
            {
                aSet.Put(SfxBoolItem(SDRATTR_3DOBJ_DOUBLE_SIDED, true));

                pScene->GetProperties().SetObjectItem(SfxBoolItem(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING, true));
            }
        }

        basegfx::B2DRange aAllPolyPolygonRange;
        SdrObjListIter aIter(rShape2d, IM_DEEPWITHGROUPS);
        const bool bMultipleSubObjects(aIter.Count() > 1);

        while(aIter.IsMore())
        {
            const SdrObject* pNext = aIter.Next();
            basegfx::B2DPolyPolygon aPolyPoly;
            const SdrPathObj* pSdrPathObj = dynamic_cast< const SdrPathObj* >(pNext);
            SfxItemSet aLocalSet(aSet);
            XFillStyle aLocalFillStyle(eFillStyle);

            if(pSdrPathObj)
            {
                aPolyPoly = pSdrPathObj->getB2DPolyPolygonInObjectCoordinates();
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
                        aPolyPoly = pNewPathObj->getB2DPolyPolygonInObjectCoordinates();

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
                                aLocalSet.Put(XFillColorItem(XubString(), ((const XLineColorItem&)(aLocalSet.Get(XATTR_LINECOLOR))).GetColorValue()));
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

                    deleteSdrObjectSafeAndClearPointer(pNewObj);
                }
            }
            else
            {
                SdrObject* pNewObj = pNext->ConvertToPolyObj(false, false);
                SdrPathObj* pPath = dynamic_cast< SdrPathObj* >(pNewObj);

                if(pPath)
                {
                    aPolyPoly = pPath->getB2DPolyPolygonInObjectCoordinates();
                }

                deleteSdrObjectSafeAndClearPointer(pNewObj);
            }

            if( aPolyPoly.count() )
            {
                if(aPolyPoly.areControlPointsUsed())
                {
                    aPolyPoly = basegfx::tools::adaptiveSubdivideByAngle(aPolyPoly);
                }

                const basegfx::B2DRange aPolyPolygonRange(aPolyPoly.getB2DRange());

                aAllPolyPolygonRange.expand(aPolyPolygonRange);

                E3dCompoundObject* p3DObj = new E3dExtrudeObj(
                    rCustomShape.getSdrModelFromSdrObject(),
                    a3DDefaultAttr,
                    aPolyPoly,
                    bUseTwoFillStyles ? 10 : fDepth);

                p3DObj->SetLayer(rShape2d.GetLayer());
                p3DObj->SetMergedItemSet(aLocalSet);

                const bool bIsPlaceholderObject(
                    XFILL_NONE == ((XFillStyleItem&)pNext->GetMergedItem( XATTR_FILLSTYLE )).GetValue()
                    && XLINE_NONE == ((XLineStyleItem&)pNext->GetMergedItem( XATTR_LINESTYLE )).GetValue());

                if(bIsPlaceholderObject)
                {
                    aPlaceholderObjectList.push_back(p3DObj);
                }
                else if(bUseTwoFillStyles)
                {
                    BitmapEx aFillBmp;
                    const bool bFillBmpTile(((XFillBmpTileItem&)p3DObj->GetMergedItem( XATTR_FILLBMP_TILE )).GetValue());

                    if(bFillBmpTile)
                    {
                        const XFillBitmapItem& rBmpItm = (XFillBitmapItem&)p3DObj->GetMergedItem(XATTR_FILLBITMAP);

                        aFillBmp = rBmpItm.GetGraphicObject().GetGraphic().GetBitmapEx();

                        Size aLogicalSize(aFillBmp.GetPrefSize());

                        if(MapMode(MAP_PIXEL) == aFillBmp.GetPrefMapMode())
                        {
                            aLogicalSize = Application::GetDefaultDevice()->PixelToLogic(aLogicalSize, MAP_100TH_MM);
                        }
                        else
                        {
                            aLogicalSize = OutputDevice::LogicToLogic(aLogicalSize, aFillBmp.GetPrefMapMode(), MAP_100TH_MM);
                        }

                        aLogicalSize.Width() *= 5; // :-( nice scaling, look at engine3d/obj3d.cxx
                        aLogicalSize.Height() *= 5;
                        aFillBmp.SetPrefSize(aLogicalSize);
                        aFillBmp.SetPrefMapMode(MAP_100TH_MM);
                        p3DObj->SetMergedItem(XFillBitmapItem(String(), Graphic(aFillBmp)));
                    }
                    else
                    {
                        if(!aSnapRange.equal(aPolyPolygonRange))
                        {
                            const XFillBitmapItem& rBmpItm = (XFillBitmapItem&)p3DObj->GetMergedItem(XATTR_FILLBITMAP);

                            aFillBmp = rBmpItm.GetGraphicObject().GetGraphic().GetBitmapEx();

                            const Size aBmpSize(aFillBmp.GetSizePixel());
                            const double fXScale(aPolyPolygonRange.getWidth() / aSnapRange.getWidth());
                            const double fYScale(aPolyPolygonRange.getHeight() / aSnapRange.getHeight());

                            const Point aPt(
                                basegfx::fround((aPolyPolygonRange.getMinX() - aSnapRange.getMinX()) * (double)aBmpSize.Width() / aSnapRange.getWidth()),
                                basegfx::fround((aPolyPolygonRange.getMinY() - aSnapRange.getMinY()) * (double)aBmpSize.Height() / aSnapRange.getHeight()));
                            const Size aSize(
                                basegfx::fround((double)aBmpSize.Width() * fXScale),
                                basegfx::fround((double)aBmpSize.Height() * fYScale));
                            const Rectangle aCropRect(aPt, aSize);

                            aFillBmp.Crop(aCropRect);
                            p3DObj->SetMergedItem(XFillBitmapItem(String(), Graphic(aFillBmp)));
                        }
                    }

                    pScene->Insert3DObj(*p3DObj);
                    p3DObj = new E3dExtrudeObj(
                        rCustomShape.getSdrModelFromSdrObject(),
                        a3DDefaultAttr,
                        aPolyPoly,
                        fDepth);
                    p3DObj->SetLayer(rShape2d.GetLayer());
                    p3DObj->SetMergedItemSet(aLocalSet);

                    if(bUseExtrusionColor)
                    {
                        p3DObj->SetMergedItem(XFillColorItem(String(), ((XSecondaryFillColorItem&)rCustomShape.GetMergedItem(XATTR_SECONDARYFILLCOLOR)).GetColorValue()));
                    }

                    p3DObj->SetMergedItem(XFillStyleItem(XFILL_SOLID));
                    p3DObj->SetMergedItem(Svx3DCloseFrontItem(false));
                    p3DObj->SetMergedItem(Svx3DCloseBackItem(false));
                    pScene->Insert3DObj(*p3DObj);

                    p3DObj = new E3dExtrudeObj(
                        rCustomShape.getSdrModelFromSdrObject(),
                        a3DDefaultAttr,
                        aPolyPoly,
                        10);
                    p3DObj->SetLayer(rShape2d.GetLayer());
                    p3DObj->SetMergedItemSet(aLocalSet);

                    basegfx::B3DHomMatrix aFrontTransform(p3DObj->GetB3DTransform());

                    aFrontTransform.translate(0.0, 0.0, fDepth);
                    p3DObj->SetB3DTransform(aFrontTransform);

                    if(XFILL_BITMAP == aLocalFillStyle && !aFillBmp.IsEmpty())
                    {
                        p3DObj->SetMergedItem(XFillBitmapItem(String(), Graphic(aFillBmp)));
                    }
                }
                else if(XFILL_NONE == eFillStyle)
                {
                    const XLineColorItem& rLineColor = (XLineColorItem&)p3DObj->GetMergedItem(XATTR_LINECOLOR);

                    p3DObj->SetMergedItem(XFillColorItem(String(), rLineColor.GetColorValue()));
                    p3DObj->SetMergedItem(SfxBoolItem(SDRATTR_3DOBJ_DOUBLE_SIDED, true));
                    p3DObj->SetMergedItem(Svx3DCloseFrontItem(false));
                    p3DObj->SetMergedItem(Svx3DCloseBackItem(false));
                }

                pScene->Insert3DObj(*p3DObj);
                bSceneHasObjects = true;
            }
        }

        if(bSceneHasObjects) // is the SdrObject properly converted
        {
            // then we can change the return value
            pRet = pScene;

            // Kameraeinstellungen, Perspektive ...
            Camera3D& rCamera = (Camera3D&)pScene->GetCamera();
            const basegfx::B3DRange& rVolume = pScene->GetBoundVolume();
            sdr::legacy::SetSnapRange(*pScene, aSnapRange);

            // InitScene replacement
            const double fW(rVolume.getWidth());
            const double fH(rVolume.getHeight());

            rCamera.SetAutoAdjustProjection(false);
            rCamera.SetViewWindow(-fW / 2, - fH / 2, fW, fH);
            basegfx::B3DPoint aLookAt(0.0, 0.0, 0.0);
            basegfx::B3DPoint aCamPos(0.0, 0.0, 100.0);
            rCamera.SetDefaults(basegfx::B3DPoint(0.0, 0.0, 100.0), aLookAt, 100.0);
            rCamera.SetPosAndLookAt(aCamPos, aLookAt);
            rCamera.SetFocalLength(1.0);
            rCamera.SetProjection(eProjectionType);
            pScene->SetCamera(rCamera);
            pScene->ActionChanged();

            double fOriginX, fOriginY;

            GetOrigin(rGeometryItem, fOriginX, fOriginY);
            fOriginX = fOriginX * aSnapRange.getWidth();
            fOriginY = fOriginY * aSnapRange.getHeight();

            basegfx::B3DHomMatrix aNewTransform(pScene->GetB3DTransform());

            aNewTransform.translate(-aSnapRange.getCenterX(), aSnapRange.getCenterY(), -pScene->GetBoundVolume().getDepth());

            double fXRotate, fYRotate;

            GetRotateAngle(rGeometryItem, fXRotate, fYRotate);

            // TTTT: const double fZRotate(rCustomShape.GetObjectRotation() * F_PI180);
            const double fZRotate(rCustomShape.getSdrObjectRotate());

            if(0.0 != fZRotate)
            {
                aNewTransform.rotate(0.0, 0.0, fZRotate);
            }

            if(bIsMirroredX)
            {
                aNewTransform.scale(-1.0, 1.0, 1.0);
            }

            if(bIsMirroredY)
            {
                aNewTransform.scale(1.0, -1.0, 1.0);
            }

            if(0.0 != fYRotate)
            {
                aNewTransform.rotate(0.0, -fYRotate, 0.0);
            }

            if(0.0 != fXRotate)
            {
                aNewTransform.rotate(-fXRotate, 0.0, 0.0 );
            }

            if(PR_PARALLEL == eProjectionType)
            {
                double fSkew, fAlpha;

                GetSkew(rGeometryItem, fSkew, fAlpha);

                if(0.0 != fSkew)
                {
                    const double fInvTanBeta(fSkew / 100.0);

                    if(fInvTanBeta)
                    {
                        aNewTransform.shearXY(
                            fInvTanBeta * cos(fAlpha),
                            fInvTanBeta * sin(fAlpha));
                    }
                }

                const basegfx::B3DPoint _aLookAt(0.0, 0.0, 0.0);
                const basegfx::B3DPoint _aNewCamPos(0.0, 0.0, 25000.0);

                rCamera.SetPosAndLookAt(_aNewCamPos, _aLookAt);
                pScene->SetCamera(rCamera);
            }
            else
            {
                aNewTransform.translate(-fOriginX, fOriginY, 0.0);

                // now set correct camera position
                const rtl::OUString sViewPoint(RTL_CONSTASCII_USTRINGPARAM("ViewPoint"));
                const drawing::Position3D aViewPointDefault(3472, -3472, 25000);
                const drawing::Position3D aViewPoint(GetPosition3D(rGeometryItem, sViewPoint, aViewPointDefault, fMap));
                const double fViewPointX(aViewPoint.PositionX);
                const double fViewPointY(aViewPoint.PositionY);
                const double fViewPointZ(aViewPoint.PositionZ);
                const basegfx::B3DPoint _aLookAt(fViewPointX, -fViewPointY, 0.0);
                const basegfx::B3DPoint aNewCamPos(fViewPointX, -fViewPointY, fViewPointZ);

                rCamera.SetPosAndLookAt(aNewCamPos, _aLookAt);
                pScene->SetCamera(rCamera);
            }

            pScene->SetB3DTransform(aNewTransform);

            ///////////
            // light //
            ///////////

            const rtl::OUString sBrightness(RTL_CONSTASCII_USTRINGPARAM("Brightness"));
            const double fAmbientIntensity(GetDouble(rGeometryItem, sBrightness, 22178.0 / 655.36, NULL) / 100.0);
            const rtl::OUString sFirstLightDirection(RTL_CONSTASCII_USTRINGPARAM("FirstLightDirection"));
            const drawing::Direction3D aFirstLightDirectionDefault(50000, 0, 10000);
            drawing::Direction3D aFirstLightDirection(GetDirection3D(rGeometryItem, sFirstLightDirection, aFirstLightDirectionDefault));

            if(0.0 == aFirstLightDirection.DirectionZ)
            {
                aFirstLightDirection.DirectionZ = 1.0;
            }

            const rtl::OUString sFirstLightLevel(RTL_CONSTASCII_USTRINGPARAM("FirstLightLevel"));
            const double fLightIntensity(GetDouble(rGeometryItem, sFirstLightLevel, 43712.0 / 655.36, NULL) / 100.0);
            const rtl::OUString sFirstLightHarsh(RTL_CONSTASCII_USTRINGPARAM("FirstLightHarsh"));
            /// const bool bFirstLightHarsh(GetBool(rGeometryItem, sFirstLightHarsh, false));
            const rtl::OUString sSecondLightDirection(RTL_CONSTASCII_USTRINGPARAM("SecondLightDirection"));
            const drawing::Direction3D aSecondLightDirectionDefault(-50000, 0, 10000);
            drawing::Direction3D aSecondLightDirection(GetDirection3D(rGeometryItem, sSecondLightDirection, aSecondLightDirectionDefault));

            if(0.0 == aSecondLightDirection.DirectionZ)
            {
                aSecondLightDirection.DirectionZ = -1;
            }

            const rtl::OUString sSecondLightLevel(RTL_CONSTASCII_USTRINGPARAM("SecondLightLevel"));
            const double fLight2Intensity(GetDouble(rGeometryItem, sSecondLightLevel, 43712.0 / 655.36, NULL) / 100.0);
            const rtl::OUString sSecondLightHarsh(RTL_CONSTASCII_USTRINGPARAM("SecondLightHarsh"));
            const rtl::OUString sLightFace(RTL_CONSTASCII_USTRINGPARAM("LightFace"));
            // const bool bLight2Harsh(GetBool(rGeometryItem, sSecondLightHarsh, false));
            // const bool bLightFace(GetBool(rGeometryItem, sLightFace, false));
            const sal_uInt16 nAmbientColor(basegfx::fround(std::min(fAmbientIntensity * 255.0, 255.0)));
            const Color aGlobalAmbientColor((sal_uInt8)nAmbientColor, (sal_uInt8)nAmbientColor, (sal_uInt8)nAmbientColor);

            pScene->GetProperties().SetObjectItem(SvxColorItem(aGlobalAmbientColor, SDRATTR_3DSCENE_AMBIENTCOLOR));

            const sal_uInt8 nSpotLight1((sal_uInt8)(fLightIntensity * 255.0));
            basegfx::B3DVector aSpotLight1(aFirstLightDirection.DirectionX, -aFirstLightDirection.DirectionY, -aFirstLightDirection.DirectionZ);

            aSpotLight1.normalize();

            pScene->GetProperties().SetObjectItem(SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_1, true));

            const Color aAmbientSpot1Color(nSpotLight1, nSpotLight1, nSpotLight1);

            pScene->GetProperties().SetObjectItem(SvxColorItem(aAmbientSpot1Color, SDRATTR_3DSCENE_LIGHTCOLOR_1));
            pScene->GetProperties().SetObjectItem(SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_1, aSpotLight1));

            const sal_uInt8 nSpotLight2((sal_uInt8)(fLight2Intensity * 255.0));
            basegfx::B3DVector aSpotLight2(aSecondLightDirection.DirectionX, -aSecondLightDirection.DirectionY, -aSecondLightDirection.DirectionZ);
            aSpotLight2.normalize();

            pScene->GetProperties().SetObjectItem(SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_2, true));

            const Color aAmbientSpot2Color(nSpotLight2, nSpotLight2, nSpotLight2);

            pScene->GetProperties().SetObjectItem(SvxColorItem(aAmbientSpot2Color, SDRATTR_3DSCENE_LIGHTCOLOR_2));
            pScene->GetProperties().SetObjectItem(SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_2, aSpotLight2));

            const sal_uInt8 nSpotLight3(70);
            const basegfx::B3DVector aSpotLight3(0.0, 0.0, 1.0);

            pScene->GetProperties().SetObjectItem(SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_3, true));

            const Color aAmbientSpot3Color(nSpotLight3, nSpotLight3, nSpotLight3);

            pScene->GetProperties().SetObjectItem(SvxColorItem(aAmbientSpot3Color, SDRATTR_3DSCENE_LIGHTCOLOR_3));
            pScene->GetProperties().SetObjectItem(SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_3, aSpotLight3));

            const rtl::OUString sSpecularity(RTL_CONSTASCII_USTRINGPARAM("Specularity"));
            const rtl::OUString sDiffusion(RTL_CONSTASCII_USTRINGPARAM("Diffusion"));
            const rtl::OUString sShininess(RTL_CONSTASCII_USTRINGPARAM("Shininess"));
            const rtl::OUString sMetal(RTL_CONSTASCII_USTRINGPARAM("Metal"));
            double fSpecular(GetDouble(rGeometryItem, sSpecularity, 0, NULL) / 100);
            const bool bMetal(GetBool(rGeometryItem, sMetal, false));
            Color aSpecularCol(225, 225, 225);

            if(bMetal)
            {
                aSpecularCol = Color(200, 200, 200);
                fSpecular += 0.15;
            }

            const sal_Int32 nIntensity(basegfx::fround(100.0 - std::min(std::max(0.0, fSpecular * 100.0), 100.0)));

            pScene->GetProperties().SetObjectItem(SvxColorItem(aSpecularCol, SDRATTR_3DOBJ_MAT_SPECULAR));
            pScene->GetProperties().SetObjectItem(SfxUInt16Item(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY, (sal_uInt16)nIntensity));

            // set 2D transformation for scene
            sdr::legacy::SetLogicRange(*pScene, CalculateNewSnapRect(rCustomShape, aSnapRange, aAllPolyPolygonRange, fMap));

            // removing placeholder objects
            std::vector< E3dCompoundObject* >::iterator aObjectListIter(aPlaceholderObjectList.begin());

            while( aObjectListIter != aPlaceholderObjectList.end())
            {
                pScene->Remove3DObj(**aObjectListIter);
                deleteSdrObjectSafeAndClearPointer(*aObjectListIter++);
            }
        }
        else
        {
            deleteSdrObjectSafeAndClearPointer(pScene);
        }
    }

    return pRet;
}

basegfx::B2DRange EnhancedCustomShape3d::CalculateNewSnapRect(
    const SdrObjCustomShape& rCustomShape,
    const basegfx::B2DRange& rSnapRect,
    const basegfx::B2DRange& rBoundRect,
    const double fMap)
{
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)rCustomShape.GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
    double fExtrusionBackward(0.0), fExtrusionForward(0.0);
    sal_uInt32 i;

    GetExtrusionDepth(rGeometryItem, fMap, fExtrusionBackward, fExtrusionForward);

    // creating initial bound volume (without rotation, skewing and camera)
    basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(rBoundRect));
    basegfx::B3DPolygon aBoundVolume;

    aPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix(-rSnapRect.getCenterX(), -rSnapRect.getCenterY()));
    aBoundVolume.append(basegfx::tools::createB3DPolygonFromB2DPolygon(aPolygon, fExtrusionForward));
    aBoundVolume.append(basegfx::tools::createB3DPolygonFromB2DPolygon(aPolygon, fExtrusionBackward));

    const rtl::OUString sRotationCenter(RTL_CONSTASCII_USTRINGPARAM("RotationCenter"));
    const drawing::Direction3D aRotationCenterDefault(0.0, 0.0, 0.0);   // default seems to be wrong, a fractional size of shape has to be used!!
    const drawing::Direction3D aRotationCenter(GetDirection3D(rGeometryItem, sRotationCenter, aRotationCenterDefault));
    double fXRotate(0.0), fYRotate(0.0);

    GetRotateAngle(rGeometryItem, fXRotate, fYRotate);

    // TTTT: double fZRotate(-rCustomShape.GetObjectRotation() * F_PI180);
    double fZRotate(-rCustomShape.getSdrObjectRotate());

    // rotating bound volume
    basegfx::B3DHomMatrix aMatrix;

    aMatrix.translate(-aRotationCenter.DirectionX, -aRotationCenter.DirectionY, -aRotationCenter.DirectionZ);

    if(0.0 != fZRotate)
    {
        aMatrix.rotate(0.0, 0.0, fZRotate);
    }

    if(rCustomShape.isMirroredX()) // TTTT: rCustomShape.IsMirroredX()) check if rCustomShape is correct
    {
        aMatrix.scale(-1.0, 1.0, 1.0);
    }

    if(rCustomShape.isMirroredY()) // TTTT: rCustomShape.IsMirroredY()) and if needed at all
    {
        aMatrix.scale(1.0, -1.0, 1.0);
    }

    if(0.0 != fYRotate)
    {
        aMatrix.rotate(0.0, fYRotate, 0.0);
    }

    if(0.0 != fXRotate)
    {
        aMatrix.rotate(-fXRotate, 0.0, 0.0);
    }

    aMatrix.translate(aRotationCenter.DirectionX, aRotationCenter.DirectionY, aRotationCenter.DirectionZ);
    aBoundVolume.transform(aMatrix);

    Transformation2D aTransformation2D(rCustomShape, fMap);

    if(aTransformation2D.IsParallel())
    {
        aBoundVolume = aTransformation2D.ApplySkewSettings(aBoundVolume);
    }

    basegfx::B2DRange aRetval;

    for(i = 0; i < aBoundVolume.count(); i++)
    {
        aRetval.expand(aTransformation2D.Transform2D(aBoundVolume.getB3DPoint(i)));
    }

    return aRetval;
}

// eof
