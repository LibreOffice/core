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

#include <svx/svdoashp.hxx>
#include "svx/unoapi.hxx"
#include <svx/unoshape.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <unotools/datetime.hxx>
#include <sfx2/lnkbase.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include "unopolyhelper.hxx"
#include <comphelper/processfactory.hxx>
#include <svl/urihelper.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svx/svdogrp.hxx>
#include <vcl/salbtype.hxx>     // FRound
#include <svx/svddrag.hxx>
#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdattrx.hxx>  // NotPersistItems
#include <svx/svdoedge.hxx>  // #32383# Die Verbinder nach Move nochmal anbroadcasten
#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdstr.hrc"    // Objektname
#include <editeng/eeitem.hxx>
#include "editeng/editstat.hxx"
#include <svx/svdoutl.hxx>
#include <editeng/outlobj.hxx>
#include <svx/sdtfchim.hxx>
#include "../svx/EnhancedCustomShapeGeometry.hxx"
#include "../svx/EnhancedCustomShapeTypeNames.hxx"
#include "../svx/EnhancedCustomShape2d.hxx"
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <editeng/writingmodeitem.hxx>
#include <svx/xlnclit.hxx>
#include <svx/svxids.hrc>
#include <svl/whiter.hxx>
#include <svx/sdr/properties/customshapeproperties.hxx>
#include <svx/sdr/contact/viewcontactofsdrobjcustomshape.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <vcl/bmpacc.hxx>
#include <svx/svdview.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/sdrtexthelpers.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;

static MSO_SPT ImpGetCustomShapeType( const SdrObjCustomShape& rCustoShape )
{
    MSO_SPT eRetValue = mso_sptNil;

    rtl::OUString aEngine( ( (SfxStringItem&)rCustoShape.GetMergedItem( SDRATTR_CUSTOMSHAPE_ENGINE ) ).GetValue() );
    if ( !aEngine.getLength() || aEngine.equalsAscii( "com.sun.star.drawing.EnhancedCustomShapeEngine" ) )
    {
        rtl::OUString sShapeType;
        const rtl::OUString sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
        SdrCustomShapeGeometryItem& rGeometryItem( (SdrCustomShapeGeometryItem&)rCustoShape.GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
        Any* pAny = rGeometryItem.GetPropertyValueByName( sType );
        if ( pAny && ( *pAny >>= sShapeType ) )
            eRetValue = EnhancedCustomShapeTypeNames::Get( sShapeType );
    }
    return eRetValue;
};

static bool ImpVerticalSwitch( const SdrObjCustomShape& rCustoShape )
{
    bool bRet = false;
    MSO_SPT eShapeType( ImpGetCustomShapeType( rCustoShape ) );
    switch( eShapeType )
    {
        case mso_sptAccentBorderCallout90 :     // 2 ortho
        case mso_sptBorderCallout1 :            // 2 diag
        case mso_sptBorderCallout2 :            // 3
        {
            bRet = true;
        }
        break;
/*
        case mso_sptCallout1 :
        case mso_sptAccentCallout1 :
        case mso_sptAccentBorderCallout1 :
        case mso_sptBorderCallout90 :
        case mso_sptCallout90 :
        case mso_sptAccentCallout90 :
        case mso_sptCallout2 :
        case mso_sptCallout3 :
        case mso_sptAccentCallout2 :
        case mso_sptAccentCallout3 :
        case mso_sptBorderCallout3 :
        case mso_sptAccentBorderCallout2 :
        case mso_sptAccentBorderCallout3 :
*/
        default: break;
    }
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #i37011# create a clone with all attributes changed to shadow attributes
// and translation executed, too.
SdrObject* ImpCreateShadowObjectClone(const SdrObject& rOriginal, const SfxItemSet& rOriginalSet)
{
    SdrObject* pRetval = 0L;
    const bool bShadow(((SdrOnOffItem&)rOriginalSet.Get(SDRATTR_SHADOW)).GetValue());

    if(bShadow)
    {
        // create a shadow representing object
        const sal_Int32 nXDist(((SdrMetricItem&)(rOriginalSet.Get(SDRATTR_SHADOWXDIST))).GetValue());
        const sal_Int32 nYDist(((SdrMetricItem&)(rOriginalSet.Get(SDRATTR_SHADOWYDIST))).GetValue());
        const ::Color aShadowColor(((XColorItem&)(rOriginalSet.Get(SDRATTR_SHADOWCOLOR))).GetColorValue());
        const sal_uInt16 nShadowTransparence(((SdrPercentItem&)(rOriginalSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue());
        pRetval = rOriginal.CloneSdrObject();
        DBG_ASSERT(pRetval, "ImpCreateShadowObjectClone: Could not clone object (!)");

        // look for used stuff
        SdrObjListIter aIterator(rOriginal);
        bool bLineUsed(false);
        bool bAllFillUsed(false);
        bool bSolidFillUsed(false);
        bool bGradientFillUsed(false);
        bool bHatchFillUsed(false);
        bool bBitmapFillUsed(false);

        while(aIterator.IsMore())
        {
            SdrObject* pObj = aIterator.Next();
            XFillStyle eFillStyle = ((XFillStyleItem&)(pObj->GetMergedItem(XATTR_FILLSTYLE))).GetValue();

            if(!bLineUsed)
            {
                XLineStyle eLineStyle = ((XLineStyleItem&)(pObj->GetMergedItem(XATTR_LINESTYLE))).GetValue();

                if(XLINE_NONE != eLineStyle)
                {
                    bLineUsed = true;
                }
            }

            if(!bAllFillUsed)
            {
                if(!bSolidFillUsed && XFILL_SOLID == eFillStyle)
                {
                    bSolidFillUsed = true;
                    bAllFillUsed = (bSolidFillUsed || bGradientFillUsed || bHatchFillUsed || bBitmapFillUsed);
                }
                if(!bGradientFillUsed && XFILL_GRADIENT == eFillStyle)
                {
                    bGradientFillUsed = true;
                    bAllFillUsed = (bSolidFillUsed || bGradientFillUsed || bHatchFillUsed || bBitmapFillUsed);
                }
                if(!bHatchFillUsed && XFILL_HATCH == eFillStyle)
                {
                    bHatchFillUsed = true;
                    bAllFillUsed = (bSolidFillUsed || bGradientFillUsed || bHatchFillUsed || bBitmapFillUsed);
                }
                if(!bBitmapFillUsed && XFILL_BITMAP == eFillStyle)
                {
                    bBitmapFillUsed = true;
                    bAllFillUsed = (bSolidFillUsed || bGradientFillUsed || bHatchFillUsed || bBitmapFillUsed);
                }
            }
        }

        // translate to shadow coordinates
        sdr::legacy::MoveSdrObject(*pRetval, Size(nXDist, nYDist));

        // set items as needed
        SfxItemSet aTempSet(rOriginalSet);

        // SJ: #40108# :-(  if a SvxWritingModeItem (Top->Bottom) is set the text object
        // is creating a paraobject, but paraobjects can not be created without model. So
        // we are preventing the crash by setting the writing mode always left to right,
        // this is not bad since our shadow geometry does not contain text.
        aTempSet.Put( SvxWritingModeItem( com::sun::star::text::WritingMode_LR_TB, SDRATTR_TEXTDIRECTION ) );

        // no shadow
        aTempSet.Put(SdrOnOffItem(SDRATTR_SHADOW, false));
        aTempSet.Put(SdrMetricItem(SDRATTR_SHADOWXDIST, 0L));
        aTempSet.Put(SdrMetricItem(SDRATTR_SHADOWYDIST, 0L));

        // line color and transparence like shadow
        if(bLineUsed)
        {
            aTempSet.Put(XLineColorItem(String(), aShadowColor));
            aTempSet.Put(XLineTransparenceItem(nShadowTransparence));
        }

        // fill color and transparence like shadow
        if(bSolidFillUsed)
        {
            aTempSet.Put(XFillColorItem(String(), aShadowColor));
            aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
        }

        // gradient and transparence like shadow
        if(bGradientFillUsed)
        {
            XGradient aGradient(((XFillGradientItem&)(rOriginalSet.Get(XATTR_FILLGRADIENT))).GetGradientValue());
            sal_uInt8 nStartLuminance(aGradient.GetStartColor().GetLuminance());
            sal_uInt8 nEndLuminance(aGradient.GetEndColor().GetLuminance());

            if(aGradient.GetStartIntens() != 100)
            {
                nStartLuminance = (sal_uInt8)(nStartLuminance * ((double)aGradient.GetStartIntens() / 100.0));
            }

            if(aGradient.GetEndIntens() != 100)
            {
                nEndLuminance = (sal_uInt8)(nEndLuminance * ((double)aGradient.GetEndIntens() / 100.0));
            }

            ::Color aStartColor(
                (sal_uInt8)((nStartLuminance * aShadowColor.GetRed()) / 256),
                (sal_uInt8)((nStartLuminance * aShadowColor.GetGreen()) / 256),
                (sal_uInt8)((nStartLuminance * aShadowColor.GetBlue()) / 256));

            ::Color aEndColor(
                (sal_uInt8)((nEndLuminance * aShadowColor.GetRed()) / 256),
                (sal_uInt8)((nEndLuminance * aShadowColor.GetGreen()) / 256),
                (sal_uInt8)((nEndLuminance * aShadowColor.GetBlue()) / 256));

            aGradient.SetStartColor(aStartColor);
            aGradient.SetEndColor(aEndColor);
            aTempSet.Put(XFillGradientItem(aTempSet.GetPool(), aGradient));
            aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
        }

        // hatch and transparence like shadow
        if(bHatchFillUsed)
        {
            XHatch aHatch(((XFillHatchItem&)(rOriginalSet.Get(XATTR_FILLHATCH))).GetHatchValue());
            aHatch.SetColor(aShadowColor);
            aTempSet.Put(XFillHatchItem(aTempSet.GetPool(), aHatch));
            aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
        }

        // bitmap and transparence like shadow
        if(bBitmapFillUsed)
        {
            GraphicObject aGraphicObject(((XFillBitmapItem&)(rOriginalSet.Get(XATTR_FILLBITMAP))).GetGraphicObject());
            const BitmapEx aBitmapEx(aGraphicObject.GetGraphic().GetBitmapEx());
            Bitmap aBitmap(aBitmapEx.GetBitmap());

            if(!aBitmap.IsEmpty())
            {
                BitmapReadAccess* pReadAccess = aBitmap.AcquireReadAccess();

                if(pReadAccess)
                {
                    Bitmap aDestBitmap(aBitmap.GetSizePixel(), 24L);
                    BitmapWriteAccess* pWriteAccess = aDestBitmap.AcquireWriteAccess();

                    if(pWriteAccess)
                    {
                        for(sal_Int32 y(0L); y < pReadAccess->Height(); y++)
                        {
                            for(sal_Int32 x(0L); x < pReadAccess->Width(); x++)
                            {
                                sal_uInt16 nLuminance((sal_uInt16)pReadAccess->GetLuminance(y, x) + 1);
                                const BitmapColor aDestColor(
                                    (sal_uInt8)((nLuminance * (sal_uInt16)aShadowColor.GetRed()) >> 8L),
                                    (sal_uInt8)((nLuminance * (sal_uInt16)aShadowColor.GetGreen()) >> 8L),
                                    (sal_uInt8)((nLuminance * (sal_uInt16)aShadowColor.GetBlue()) >> 8L));
                                pWriteAccess->SetPixel(y, x, aDestColor);
                            }
                        }

                        aDestBitmap.ReleaseAccess(pWriteAccess);
                    }

                    aBitmap.ReleaseAccess(pReadAccess);

                    if(aBitmapEx.IsTransparent())
                    {
                        if(aBitmapEx.IsAlpha())
                        {
                            aGraphicObject.SetGraphic(Graphic(BitmapEx(aDestBitmap, aBitmapEx.GetAlpha())));
                        }
                        else
                        {
                            aGraphicObject.SetGraphic(Graphic(BitmapEx(aDestBitmap, aBitmapEx.GetMask())));
                        }
                    }
                    else
                    {
                        aGraphicObject.SetGraphic(Graphic(aDestBitmap));
                    }
                }
            }

            aTempSet.Put(XFillBitmapItem(aTempSet.GetPool(), aGraphicObject));
            aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
        }

        // set attributes and paint shadow object
        pRetval->SetMergedItemSet( aTempSet );
    }
    return pRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Reference< XCustomShapeEngine > SdrObjCustomShape::GetCustomShapeEngine( const SdrObjCustomShape* pCustomShape )
{
    Reference< XCustomShapeEngine > xCustomShapeEngine;
    String aEngine(((SfxStringItem&)pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_ENGINE )).GetValue());
    if ( !aEngine.Len() )
        aEngine = String( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.drawing.EnhancedCustomShapeEngine" ) );

    Reference< XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

    Reference< XShape > aXShape = GetXShapeForSdrObject( (SdrObjCustomShape*)pCustomShape );
    if ( aXShape.is() )
    {
        if ( aEngine.Len() && xFactory.is() )
        {
            Sequence< Any > aArgument( 1 );
            Sequence< PropertyValue > aPropValues( 1 );
            aPropValues[ 0 ].Name = rtl::OUString::createFromAscii( "CustomShape" );
            aPropValues[ 0 ].Value <<= aXShape;
            aArgument[ 0 ] <<= aPropValues;
            Reference< XInterface > xInterface( xFactory->createInstanceWithArguments( aEngine, aArgument ) );
            if ( xInterface.is() )
                xCustomShapeEngine = Reference< XCustomShapeEngine >( xInterface, UNO_QUERY );
        }
    }
    return xCustomShapeEngine;
}
const SdrObject* SdrObjCustomShape::GetSdrObjectFromCustomShape() const
{
    if ( !mXRenderedCustomShape.is() )
    {
        Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine( this ) );
        if ( xCustomShapeEngine.is() )
            ((SdrObjCustomShape*)this)->mXRenderedCustomShape = xCustomShapeEngine->render();
    }
    SdrObject* pRenderedCustomShape = mXRenderedCustomShape.is()
                ? GetSdrObjectFromXShape( mXRenderedCustomShape )
                : NULL;
    return pRenderedCustomShape;
}

// #i37011# Shadow geometry creation
const SdrObject* SdrObjCustomShape::GetSdrObjectShadowFromCustomShape() const
{
    if(!mpLastShadowGeometry)
    {
        const SdrObject* pSdrObject = GetSdrObjectFromCustomShape();
        if(pSdrObject)
        {
            const SfxItemSet& rOriginalSet = GetObjectItemSet();
            const bool bShadow(((SdrOnOffItem&)rOriginalSet.Get( SDRATTR_SHADOW )).GetValue());

            if(bShadow)
            {
                // create a clone with all attributes changed to shadow attributes
                // and translation executed, too.
                ((SdrObjCustomShape*)this)->mpLastShadowGeometry = ImpCreateShadowObjectClone(*pSdrObject, rOriginalSet);
            }
        }
    }

    return mpLastShadowGeometry;
}

bool SdrObjCustomShape::IsTextPath() const
{
    const rtl::OUString sTextPath( RTL_CONSTASCII_USTRINGPARAM ( "TextPath" ) );
    bool bTextPathOn = false;
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    Any* pAny = rGeometryItem.GetPropertyValueByName( sTextPath, sTextPath );
    if ( pAny )
        *pAny >>= bTextPathOn;
    return bTextPathOn;
}

bool SdrObjCustomShape::UseNoFillStyle() const
{
    bool bRet = false;
    rtl::OUString sShapeType;
    const rtl::OUString sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
    SdrCustomShapeGeometryItem& rGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    Any* pAny = rGeometryItem.GetPropertyValueByName( sType );
    if ( pAny )
        *pAny >>= sShapeType;
    bRet = IsCustomShapeFilledByDefault( EnhancedCustomShapeTypeNames::Get( sType ) ) == 0;

    return bRet;
}

// TTTT: MirroredX/Y removed
//bool SdrObjCustomShape::IsMirroredX() const
//{
//  bool bMirroredX = false;
//  SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
//  const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
//  com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredX );
//  if ( pAny )
//      *pAny >>= bMirroredX;
//  return bMirroredX;
//}
//bool SdrObjCustomShape::IsMirroredY() const
//{
//  bool bMirroredY = false;
//  SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
//  const rtl::OUString sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
//  com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredY );
//  if ( pAny )
//      *pAny >>= bMirroredY;
//  return bMirroredY;
//}
//void SdrObjCustomShape::SetMirroredX( const bool bMirrorX )
//{
//  SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
//  const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
//  //com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredX );
//  PropertyValue aPropVal;
//  aPropVal.Name = sMirroredX;
//  aPropVal.Value <<= bMirrorX;
//  aGeometryItem.SetPropertyValue( aPropVal );
//  SetMergedItem( aGeometryItem );
//}
//void SdrObjCustomShape::SetMirroredY( const bool bMirrorY )
//{
//  SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
//  const rtl::OUString sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
//  //com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredY );
//  PropertyValue aPropVal;
//  aPropVal.Name = sMirroredY;
//  aPropVal.Value <<= bMirrorY;
//  aGeometryItem.SetPropertyValue( aPropVal );
//  SetMergedItem( aGeometryItem );
//}

// TTTT: Probably obsolete
//double SdrObjCustomShape::GetObjectRotation() const
//{
//  return fObjectRotation;
//}

double SdrObjCustomShape::GetExtraTextRotation() const
{
    const com::sun::star::uno::Any* pAny;
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    const rtl::OUString sTextRotateAngle( RTL_CONSTASCII_USTRINGPARAM ( "TextRotateAngle" ) );
    pAny = rGeometryItem.GetPropertyValueByName( sTextRotateAngle );
    double fExtraTextRotateAngle = 0.0;
    if ( pAny )
        *pAny >>= fExtraTextRotateAngle;
    return fExtraTextRotateAngle;
}
basegfx::B2DPolyPolygon SdrObjCustomShape::GetLineGeometry( const SdrObjCustomShape* pCustomShape, const bool bBezierAllowed )
{
    basegfx::B2DPolyPolygon aRetval;
    bool bRet = false;
    Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine( pCustomShape ) );
    if ( xCustomShapeEngine.is() )
    {
        com::sun::star::drawing::PolyPolygonBezierCoords aBezierCoords = xCustomShapeEngine->getLineGeometry();
        try
        {
            aRetval = basegfx::tools::UnoPolyPolygonBezierCoordsToB2DPolyPolygon(aBezierCoords);
            if ( !bBezierAllowed && aRetval.areControlPointsUsed())
            {
                aRetval = basegfx::tools::adaptiveSubdivideByAngle(aRetval);
            }
            bRet = true;
        }
        catch ( const com::sun::star::lang::IllegalArgumentException )
        {
        }
    }
    return aRetval;
}

std::vector< SdrCustomShapeInteraction > SdrObjCustomShape::GetInteractionHandles( const SdrObjCustomShape* pCustomShape ) const
{
    std::vector< SdrCustomShapeInteraction > xRet;
    try
    {
        Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine( pCustomShape ) );
        if ( xCustomShapeEngine.is() )
        {
            int i;
            Sequence< Reference< XCustomShapeHandle > > xInteractionHandles( xCustomShapeEngine->getInteraction() );
            for ( i = 0; i < xInteractionHandles.getLength(); i++ )
            {
                if ( xInteractionHandles[ i ].is() )
                {
                    SdrCustomShapeInteraction aSdrCustomShapeInteraction;
                    aSdrCustomShapeInteraction.xInteraction = xInteractionHandles[ i ];
                    aSdrCustomShapeInteraction.aPosition = xInteractionHandles[ i ]->getPosition();

                    sal_Int32 nMode = 0;
                    switch( ImpGetCustomShapeType( *this ) )
                    {
                        case mso_sptAccentBorderCallout90 :     // 2 ortho
                        {
                            if ( !i )
                                nMode |= CUSTOMSHAPE_HANDLE_RESIZE_FIXED | CUSTOMSHAPE_HANDLE_CREATE_FIXED;
                            else if ( i == 1)
                                nMode |= CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_X | CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_Y | CUSTOMSHAPE_HANDLE_MOVE_SHAPE | CUSTOMSHAPE_HANDLE_ORTHO4;
                        }
                        break;

                        case mso_sptWedgeRectCallout :
                        case mso_sptWedgeRRectCallout :
                        case mso_sptCloudCallout :
                        case mso_sptWedgeEllipseCallout :
                        {
                            if ( !i )
                                nMode |= CUSTOMSHAPE_HANDLE_RESIZE_FIXED;
                        }
                        break;

                        case mso_sptBorderCallout1 :            // 2 diag
                        {
                            if ( !i )
                                nMode |= CUSTOMSHAPE_HANDLE_RESIZE_FIXED | CUSTOMSHAPE_HANDLE_CREATE_FIXED;
                            else if ( i == 1 )
                                nMode |= CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_X | CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_Y | CUSTOMSHAPE_HANDLE_MOVE_SHAPE;
                        }
                        break;
                        case mso_sptBorderCallout2 :            // 3
                        {
                            if ( !i )
                                nMode |= CUSTOMSHAPE_HANDLE_RESIZE_FIXED | CUSTOMSHAPE_HANDLE_CREATE_FIXED;
                            else if ( i == 2 )
                                nMode |= CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_X | CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_Y | CUSTOMSHAPE_HANDLE_MOVE_SHAPE;
                        }
                        break;
                        case mso_sptCallout90 :
                        case mso_sptAccentCallout90 :
                        case mso_sptBorderCallout90 :
                        case mso_sptCallout1 :
                        case mso_sptCallout2 :
                        case mso_sptCallout3 :
                        case mso_sptAccentCallout1 :
                        case mso_sptAccentCallout2 :
                        case mso_sptAccentCallout3 :
                        case mso_sptBorderCallout3 :
                        case mso_sptAccentBorderCallout1 :
                        case mso_sptAccentBorderCallout2 :
                        case mso_sptAccentBorderCallout3 :
                        {
                            if ( !i )
                                nMode |= CUSTOMSHAPE_HANDLE_RESIZE_FIXED | CUSTOMSHAPE_HANDLE_CREATE_FIXED;
                        }
                        break;
                        default: break;
                    }
                    aSdrCustomShapeInteraction.nMode = nMode;
                    xRet.push_back( aSdrCustomShapeInteraction );
                }
            }
        }
    }
    catch( const uno::RuntimeException& )
    {
    }
    return xRet;
}

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section
#define DEFAULT_MINIMUM_SIGNED_COMPARE  ((sal_Int32)0x80000000)
#define DEFAULT_MAXIMUM_SIGNED_COMPARE  ((sal_Int32)0x7fffffff)

sdr::properties::BaseProperties* SdrObjCustomShape::CreateObjectSpecificProperties()
{
    return new sdr::properties::CustomShapeProperties(*this);
}

SdrObjCustomShape::SdrObjCustomShape(SdrModel& rSdrModel)
:   SdrTextObj(rSdrModel, basegfx::B2DHomMatrix(), OBJ_TEXT, true),
    // TTTT: fObjectRotation( 0.0 ),
    mpLastShadowGeometry(0)
{
}

SdrObjCustomShape::~SdrObjCustomShape()
{
    // delete buffered display geometry
    InvalidateRenderGeometry();
}

void SdrObjCustomShape::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SdrObjCustomShape* pSource = dynamic_cast< const SdrObjCustomShape* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrTextObj::copyDataFromSdrObject(rSource);

            // copy local data
            aName = pSource->aName;
            // TTTT: fObjectRotation = pSource->fObjectRotation;
            InvalidateRenderGeometry();
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* SdrObjCustomShape::CloneSdrObject(SdrModel* pTargetModel) const
{
    SdrObjCustomShape* pClone = new SdrObjCustomShape(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

void SdrObjCustomShape::MergeDefaultAttributes( const rtl::OUString* pType )
{
    PropertyValue aPropVal;
    rtl::OUString sShapeType;
    const rtl::OUString sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    if ( pType && pType->getLength() )
    {
        sal_Int32 nType = pType->toInt32();
        if ( nType )
            sShapeType = EnhancedCustomShapeTypeNames::Get( static_cast< MSO_SPT >( nType ) );
        else
            sShapeType = *pType;

        aPropVal.Name = sType;
        aPropVal.Value <<= sShapeType;
        aGeometryItem.SetPropertyValue( aPropVal );
    }
    else
    {
        Any *pAny = aGeometryItem.GetPropertyValueByName( sType );
        if ( pAny )
            *pAny >>= sShapeType;
    }
    MSO_SPT eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

    const sal_Int32* pDefData = NULL;
    const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eSpType );
    if ( pDefCustomShape )
        pDefData = pDefCustomShape->pDefData;

    com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue > seqAdjustmentValues;

    //////////////////////
    // AdjustmentValues //
    //////////////////////
    const rtl::OUString sAdjustmentValues( RTL_CONSTASCII_USTRINGPARAM ( "AdjustmentValues" ) );
    const Any* pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sAdjustmentValues );
    if ( pAny )
        *pAny >>= seqAdjustmentValues;
    if ( pDefCustomShape && pDefData )  // now check if we have to default some adjustment values
    {
        // first check if there are adjustment values are to be appended
        sal_Int32 i, nAdjustmentValues = seqAdjustmentValues.getLength();
        sal_Int32 nAdjustmentDefaults = *pDefData++;
        if ( nAdjustmentDefaults > nAdjustmentValues )
        {
            seqAdjustmentValues.realloc( nAdjustmentDefaults );
            for ( i = nAdjustmentValues; i < nAdjustmentDefaults; i++ )
            {
                seqAdjustmentValues[ i ].Value <<= pDefData[ i ];
                seqAdjustmentValues[ i ].State = com::sun::star::beans::PropertyState_DIRECT_VALUE; // com::sun::star::beans::PropertyState_DEFAULT_VALUE;
            }
        }
        // check if there are defaulted adjustment values that should be filled the hard coded defaults (pDefValue)
        sal_Int32 nCount = nAdjustmentValues > nAdjustmentDefaults ? nAdjustmentDefaults : nAdjustmentValues;
        for ( i = 0; i < nCount; i++ )
        {
            if ( seqAdjustmentValues[ i ].State != com::sun::star::beans::PropertyState_DIRECT_VALUE )
            {
                seqAdjustmentValues[ i ].Value <<= pDefData[ i ];
                seqAdjustmentValues[ i ].State = com::sun::star::beans::PropertyState_DIRECT_VALUE;
            }
        }
    }
    aPropVal.Name = sAdjustmentValues;
    aPropVal.Value <<= seqAdjustmentValues;
    aGeometryItem.SetPropertyValue( aPropVal );

    ///////////////
    // Coordsize //
    ///////////////
    const rtl::OUString sViewBox( RTL_CONSTASCII_USTRINGPARAM ( "ViewBox" ) );
    const Any* pViewBox = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sViewBox );
    com::sun::star::awt::Rectangle aViewBox;
    if ( !pViewBox || !(*pViewBox >>= aViewBox ) )
    {
        if ( pDefCustomShape )
        {
            aViewBox.X = 0;
            aViewBox.Y = 0;
            aViewBox.Width = pDefCustomShape->nCoordWidth;
            aViewBox.Height= pDefCustomShape->nCoordHeight;
            aPropVal.Name = sViewBox;
            aPropVal.Value <<= aViewBox;
            aGeometryItem.SetPropertyValue( aPropVal );
        }
    }

    const rtl::OUString sPath( RTL_CONSTASCII_USTRINGPARAM ( "Path" ) );

    //////////////////////
    // Path/Coordinates //
    //////////////////////
    const rtl::OUString sCoordinates( RTL_CONSTASCII_USTRINGPARAM ( "Coordinates" ) );
    pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sCoordinates );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nVertices && pDefCustomShape->pVertices )
    {
        com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair> seqCoordinates;

        sal_Int32 i, nCount = pDefCustomShape->nVertices;
        seqCoordinates.realloc( nCount );
        for ( i = 0; i < nCount; i++ )
        {
            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqCoordinates[ i ].First, pDefCustomShape->pVertices[ i ].nValA );
            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqCoordinates[ i ].Second, pDefCustomShape->pVertices[ i ].nValB );
        }
        aPropVal.Name = sCoordinates;
        aPropVal.Value <<= seqCoordinates;
        aGeometryItem.SetPropertyValue( sPath, aPropVal );
    }

    /////////////////////
    // Path/GluePoints //
    /////////////////////
    const rtl::OUString sGluePoints( RTL_CONSTASCII_USTRINGPARAM ( "GluePoints" ) );
    pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sGluePoints );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nGluePoints && pDefCustomShape->pGluePoints )
    {
        com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair> seqGluePoints;
        sal_Int32 i, nCount = pDefCustomShape->nGluePoints;
        seqGluePoints.realloc( nCount );
        for ( i = 0; i < nCount; i++ )
        {
            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqGluePoints[ i ].First, pDefCustomShape->pGluePoints[ i ].nValA );
            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqGluePoints[ i ].Second, pDefCustomShape->pGluePoints[ i ].nValB );
        }
        aPropVal.Name = sGluePoints;
        aPropVal.Value <<= seqGluePoints;
        aGeometryItem.SetPropertyValue( sPath, aPropVal );
    }

    ///////////////////
    // Path/Segments //
    ///////////////////
    const rtl::OUString sSegments( RTL_CONSTASCII_USTRINGPARAM ( "Segments" ) );
    pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sSegments );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nElements && pDefCustomShape->pElements )
    {
        com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeSegment > seqSegments;

        sal_Int32 i, nCount = pDefCustomShape->nElements;
        seqSegments.realloc( nCount );
        for ( i = 0; i < nCount; i++ )
        {
            EnhancedCustomShapeSegment& rSegInfo = seqSegments[ i ];
            sal_uInt16 nSDat = pDefCustomShape->pElements[ i ];
            switch( nSDat >> 8 )
            {
                case 0x00 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::LINETO;
                    rSegInfo.Count   = nSDat & 0xff;
                    if ( !rSegInfo.Count )
                        rSegInfo.Count = 1;
                }
                break;
                case 0x20 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CURVETO;
                    rSegInfo.Count   = nSDat & 0xff;
                    if ( !rSegInfo.Count )
                        rSegInfo.Count = 1;
                }
                break;
                case 0x40 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::MOVETO;
                    rSegInfo.Count   = nSDat & 0xff;
                    if ( !rSegInfo.Count )
                        rSegInfo.Count = 1;
                }
                break;
                case 0x60 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
                    rSegInfo.Count   = 0;
                }
                break;
                case 0x80 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
                    rSegInfo.Count   = 0;
                }
                break;
                case 0xa1 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO;
                    rSegInfo.Count   = ( nSDat & 0xff ) / 3;
                }
                break;
                case 0xa2 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE;
                    rSegInfo.Count   = ( nSDat & 0xff ) / 3;
                }
                break;
                case 0xa3 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARCTO;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xa4 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARC;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xa5 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xa6 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARC;
                    rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                }
                break;
                case 0xa7 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0xa8 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY;
                    rSegInfo.Count   = nSDat & 0xff;
                }
                break;
                case 0xaa :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOFILL;
                    rSegInfo.Count   = 0;
                }
                break;
                case 0xab :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOSTROKE;
                    rSegInfo.Count   = 0;
                }
                break;
                default:
                case 0xf8 :
                {
                    rSegInfo.Command = EnhancedCustomShapeSegmentCommand::UNKNOWN;
                    rSegInfo.Count   = nSDat;
                }
                break;
            }
        }
        aPropVal.Name = sSegments;
        aPropVal.Value <<= seqSegments;
        aGeometryItem.SetPropertyValue( sPath, aPropVal );
    }

    ///////////////////
    // Path/StretchX //
    ///////////////////
    const rtl::OUString sStretchX( RTL_CONSTASCII_USTRINGPARAM ( "StretchX" ) );
    pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sStretchX );
    if ( !pAny && pDefCustomShape )
    {
        sal_Int32 nXRef = pDefCustomShape->nXRef;
        if ( ( nXRef != DEFAULT_MINIMUM_SIGNED_COMPARE ) )
        {
            aPropVal.Name = sStretchX;
            aPropVal.Value <<= nXRef;
            aGeometryItem.SetPropertyValue( sPath, aPropVal );
        }
    }

    ///////////////////
    // Path/StretchY //
    ///////////////////
    const rtl::OUString sStretchY( RTL_CONSTASCII_USTRINGPARAM ( "StretchY" ) );
    pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sStretchY );
    if ( !pAny && pDefCustomShape )
    {
        sal_Int32 nYRef = pDefCustomShape->nYRef;
        if ( ( nYRef != DEFAULT_MINIMUM_SIGNED_COMPARE ) )
        {
            aPropVal.Name = sStretchY;
            aPropVal.Value <<= nYRef;
            aGeometryItem.SetPropertyValue( sPath, aPropVal );
        }
    }

    /////////////////////
    // Path/TextFrames //
    /////////////////////
    const rtl::OUString sTextFrames( RTL_CONSTASCII_USTRINGPARAM ( "TextFrames" ) );
    pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sTextFrames );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nTextRect && pDefCustomShape->pTextRect )
    {
        com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeTextFrame > seqTextFrames;

        sal_Int32 i, nCount = pDefCustomShape->nTextRect;
        seqTextFrames.realloc( nCount );
        const SvxMSDffTextRectangles* pRectangles = pDefCustomShape->pTextRect;
        for ( i = 0; i < nCount; i++, pRectangles++ )
        {
            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames[ i ].TopLeft.First,     pRectangles->nPairA.nValA );
            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames[ i ].TopLeft.Second,    pRectangles->nPairA.nValB );
            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames[ i ].BottomRight.First,  pRectangles->nPairB.nValA );
            EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames[ i ].BottomRight.Second, pRectangles->nPairB.nValB );
        }
        aPropVal.Name = sTextFrames;
        aPropVal.Value <<= seqTextFrames;
        aGeometryItem.SetPropertyValue( sPath, aPropVal );
    }

    ///////////////
    // Equations //
    ///////////////
    const rtl::OUString sEquations( RTL_CONSTASCII_USTRINGPARAM( "Equations" ) );
    pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sEquations );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nCalculation && pDefCustomShape->pCalculation )
    {
        com::sun::star::uno::Sequence< rtl::OUString > seqEquations;

        sal_Int32 i, nCount = pDefCustomShape->nCalculation;
        seqEquations.realloc( nCount );
        const SvxMSDffCalculationData* pData = pDefCustomShape->pCalculation;
        for ( i = 0; i < nCount; i++, pData++ )
            seqEquations[ i ] = EnhancedCustomShape2d::GetEquation( pData->nFlags, pData->nVal[ 0 ], pData->nVal[ 1 ], pData->nVal[ 2 ] );
        aPropVal.Name = sEquations;
        aPropVal.Value <<= seqEquations;
        aGeometryItem.SetPropertyValue( aPropVal );
    }

    /////////////
    // Handles //
    /////////////
    const rtl::OUString sHandles( RTL_CONSTASCII_USTRINGPARAM( "Handles" ) );
    pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sHandles );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nHandles && pDefCustomShape->pHandles )
    {
        com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValues > seqHandles;

        sal_Int32 i, n, nCount = pDefCustomShape->nHandles;
        const SvxMSDffHandle* pData = pDefCustomShape->pHandles;
        seqHandles.realloc( nCount );
        for ( i = 0; i < nCount; i++, pData++ )
        {
            sal_Int32 nPropertiesNeeded = 1;    // position is always needed
            sal_Int32 nFlags = pData->nFlags;
            if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
                nPropertiesNeeded++;
            if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
                nPropertiesNeeded++;
            if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
                nPropertiesNeeded++;
            if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
            {
                nPropertiesNeeded++;
                if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
                {
                    if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
                        nPropertiesNeeded++;
                    if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
                        nPropertiesNeeded++;
                }
            }
            else if ( nFlags & MSDFF_HANDLE_FLAGS_RANGE )
            {
                if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
                    nPropertiesNeeded++;
                if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
                    nPropertiesNeeded++;
                if ( pData->nRangeYMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
                    nPropertiesNeeded++;
                if ( pData->nRangeYMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
                    nPropertiesNeeded++;
            }

            n = 0;
            com::sun::star::beans::PropertyValues& rPropValues = seqHandles[ i ];
            rPropValues.realloc( nPropertiesNeeded );

            // POSITION
            {
                const rtl::OUString sPosition( RTL_CONSTASCII_USTRINGPARAM ( "Position" ) );
                ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aPosition;
                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.First, pData->nPositionX, true, true );
                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.Second, pData->nPositionY, true, false );
                rPropValues[ n ].Name = sPosition;
                rPropValues[ n++ ].Value <<= aPosition;
            }
            if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
            {
                const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
                bool bMirroredX = true;
                rPropValues[ n ].Name = sMirroredX;
                rPropValues[ n++ ].Value <<= bMirroredX;
            }
            if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
            {
                const rtl::OUString sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
                bool bMirroredY = true;
                rPropValues[ n ].Name = sMirroredY;
                rPropValues[ n++ ].Value <<= bMirroredY;
            }
            if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
            {
                const rtl::OUString sSwitched( RTL_CONSTASCII_USTRINGPARAM ( "Switched" ) );
                bool bSwitched = true;
                rPropValues[ n ].Name = sSwitched;
                rPropValues[ n++ ].Value <<= bSwitched;
            }
            if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
            {
                const rtl::OUString sPolar( RTL_CONSTASCII_USTRINGPARAM ( "Polar" ) );
                ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aCenter;
                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aCenter.First,  pData->nCenterX,
                    ( nFlags & MSDFF_HANDLE_FLAGS_CENTER_X_IS_SPECIAL ) != 0, true  );
                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aCenter.Second, pData->nCenterY,
                    ( nFlags & MSDFF_HANDLE_FLAGS_CENTER_Y_IS_SPECIAL ) != 0, false );
                rPropValues[ n ].Name = sPolar;
                rPropValues[ n++ ].Value <<= aCenter;
                if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
                {
                    if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
                    {
                        const rtl::OUString sRadiusRangeMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMinimum" ) );
                        ::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
                        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMinimum, pData->nRangeXMin,
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, true  );
                        rPropValues[ n ].Name = sRadiusRangeMinimum;
                        rPropValues[ n++ ].Value <<= aRadiusRangeMinimum;
                    }
                    if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
                    {
                        const rtl::OUString sRadiusRangeMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMaximum" ) );
                        ::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
                        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMaximum, pData->nRangeXMax,
                            ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, false );
                        rPropValues[ n ].Name = sRadiusRangeMaximum;
                        rPropValues[ n++ ].Value <<= aRadiusRangeMaximum;
                    }
                }
            }
            else if ( nFlags & MSDFF_HANDLE_FLAGS_RANGE )
            {
                if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
                {
                    const rtl::OUString sRangeXMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMinimum" ) );
                    ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMinimum;
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMinimum, pData->nRangeXMin,
                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, true  );
                    rPropValues[ n ].Name = sRangeXMinimum;
                    rPropValues[ n++ ].Value <<= aRangeXMinimum;
                }
                if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
                {
                    const rtl::OUString sRangeXMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMaximum" ) );
                    ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMaximum;
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMaximum, pData->nRangeXMax,
                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, false );
                    rPropValues[ n ].Name = sRangeXMaximum;
                    rPropValues[ n++ ].Value <<= aRangeXMaximum;
                }
                if ( pData->nRangeYMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
                {
                    const rtl::OUString sRangeYMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMinimum" ) );
                    ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMinimum;
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMinimum, pData->nRangeYMin,
                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MIN_IS_SPECIAL ) != 0, true );
                    rPropValues[ n ].Name = sRangeYMinimum;
                    rPropValues[ n++ ].Value <<= aRangeYMinimum;
                }
                if ( pData->nRangeYMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
                {
                    const rtl::OUString sRangeYMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMaximum" ) );
                    ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMaximum;
                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMaximum, pData->nRangeYMax,
                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MAX_IS_SPECIAL ) != 0, false );
                    rPropValues[ n ].Name = sRangeYMaximum;
                    rPropValues[ n++ ].Value <<= aRangeYMaximum;
                }
            }
        }
        aPropVal.Name = sHandles;
        aPropVal.Value <<= seqHandles;
        aGeometryItem.SetPropertyValue( aPropVal );
    }
    SetMergedItem( aGeometryItem );
}

bool SdrObjCustomShape::IsDefaultGeometry( const DefaultType eDefaultType ) const
{
    bool bIsDefaultGeometry = false;

    PropertyValue aPropVal;
    rtl::OUString sShapeType;
    const rtl::OUString sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

    Any *pAny = aGeometryItem.GetPropertyValueByName( sType );
    if ( pAny )
        *pAny >>= sShapeType;

    MSO_SPT eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

    const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eSpType );
    const rtl::OUString sPath( RTL_CONSTASCII_USTRINGPARAM ( "Path" ) );
    switch( eDefaultType )
    {
        case DEFAULT_VIEWBOX :
        {
            const rtl::OUString sViewBox( RTL_CONSTASCII_USTRINGPARAM ( "ViewBox" ) );
            const Any* pViewBox = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sViewBox );
            com::sun::star::awt::Rectangle aViewBox;
            if ( pViewBox && ( *pViewBox >>= aViewBox ) )
            {
                if ( ( aViewBox.Width == pDefCustomShape->nCoordWidth )
                    && ( aViewBox.Height == pDefCustomShape->nCoordHeight ) )
                    bIsDefaultGeometry = true;
            }
        }
        break;

        case DEFAULT_PATH :
        {
            const rtl::OUString sCoordinates( RTL_CONSTASCII_USTRINGPARAM ( "Coordinates" ) );
            pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sCoordinates );
            if ( pAny && pDefCustomShape && pDefCustomShape->nVertices && pDefCustomShape->pVertices )
            {
                com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair> seqCoordinates1, seqCoordinates2;
                if ( *pAny >>= seqCoordinates1 )
                {
                    sal_Int32 i, nCount = pDefCustomShape->nVertices;
                    seqCoordinates2.realloc( nCount );
                    for ( i = 0; i < nCount; i++ )
                    {
                        EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqCoordinates2[ i ].First, pDefCustomShape->pVertices[ i ].nValA );
                        EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqCoordinates2[ i ].Second, pDefCustomShape->pVertices[ i ].nValB );
                    }
                    if ( seqCoordinates1 == seqCoordinates2 )
                        bIsDefaultGeometry = true;
                }
            }
            else if ( pDefCustomShape && ( ( pDefCustomShape->nVertices == 0 ) || ( pDefCustomShape->pVertices == 0 ) ) )
                bIsDefaultGeometry = true;
        }
        break;

        case DEFAULT_GLUEPOINTS :
        {
            const rtl::OUString sGluePoints( RTL_CONSTASCII_USTRINGPARAM ( "GluePoints" ) );
            pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sGluePoints );
            if ( pAny && pDefCustomShape && pDefCustomShape->nGluePoints && pDefCustomShape->pGluePoints )
            {
                com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair> seqGluePoints1, seqGluePoints2;
                if ( *pAny >>= seqGluePoints1 )
                {
                    sal_Int32 i, nCount = pDefCustomShape->nGluePoints;
                    seqGluePoints2.realloc( nCount );
                    for ( i = 0; i < nCount; i++ )
                    {
                        EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqGluePoints2[ i ].First, pDefCustomShape->pGluePoints[ i ].nValA );
                        EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqGluePoints2[ i ].Second, pDefCustomShape->pGluePoints[ i ].nValB );
                    }
                    if ( seqGluePoints1 == seqGluePoints2 )
                        bIsDefaultGeometry = true;
                }
            }
            else if ( pDefCustomShape && ( pDefCustomShape->nGluePoints == 0 ) )
                bIsDefaultGeometry = true;
        }
        break;

        case DEFAULT_SEGMENTS :
        {
            ///////////////////
            // Path/Segments //
            ///////////////////
            const rtl::OUString sSegments( RTL_CONSTASCII_USTRINGPARAM ( "Segments" ) );
            pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sSegments );
            if ( pAny )
            {
                com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeSegment > seqSegments1, seqSegments2;
                if ( *pAny >>= seqSegments1 )
                {
                    if ( pDefCustomShape && pDefCustomShape->nElements && pDefCustomShape->pElements )
                    {
                        sal_Int32 i, nCount = pDefCustomShape->nElements;
                        if ( nCount )
                        {
                            seqSegments2.realloc( nCount );
                            for ( i = 0; i < nCount; i++ )
                            {
                                EnhancedCustomShapeSegment& rSegInfo = seqSegments2[ i ];
                                sal_uInt16 nSDat = pDefCustomShape->pElements[ i ];
                                switch( nSDat >> 8 )
                                {
                                    case 0x00 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::LINETO;
                                        rSegInfo.Count   = nSDat & 0xff;
                                        if ( !rSegInfo.Count )
                                            rSegInfo.Count = 1;
                                    }
                                    break;
                                    case 0x20 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CURVETO;
                                        rSegInfo.Count   = nSDat & 0xff;
                                        if ( !rSegInfo.Count )
                                            rSegInfo.Count = 1;
                                    }
                                    break;
                                    case 0x40 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::MOVETO;
                                        rSegInfo.Count   = nSDat & 0xff;
                                        if ( !rSegInfo.Count )
                                            rSegInfo.Count = 1;
                                    }
                                    break;
                                    case 0x60 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
                                        rSegInfo.Count   = 0;
                                    }
                                    break;
                                    case 0x80 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
                                        rSegInfo.Count   = 0;
                                    }
                                    break;
                                    case 0xa1 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO;
                                        rSegInfo.Count   = ( nSDat & 0xff ) / 3;
                                    }
                                    break;
                                    case 0xa2 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE;
                                        rSegInfo.Count   = ( nSDat & 0xff ) / 3;
                                    }
                                    break;
                                    case 0xa3 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARCTO;
                                        rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                                    }
                                    break;
                                    case 0xa4 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARC;
                                        rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                                    }
                                    break;
                                    case 0xa5 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO;
                                        rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                                    }
                                    break;
                                    case 0xa6 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARC;
                                        rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
                                    }
                                    break;
                                    case 0xa7 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX;
                                        rSegInfo.Count   = nSDat & 0xff;
                                    }
                                    break;
                                    case 0xa8 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY;
                                        rSegInfo.Count   = nSDat & 0xff;
                                    }
                                    break;
                                    case 0xaa :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOFILL;
                                        rSegInfo.Count   = 0;
                                    }
                                    break;
                                    case 0xab :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOSTROKE;
                                        rSegInfo.Count   = 0;
                                    }
                                    break;
                                    default:
                                    case 0xf8 :
                                    {
                                        rSegInfo.Command = EnhancedCustomShapeSegmentCommand::UNKNOWN;
                                        rSegInfo.Count   = nSDat;
                                    }
                                    break;
                                }
                            }
                            if ( seqSegments1 == seqSegments2 )
                                bIsDefaultGeometry = true;
                        }
                    }
                    else
                    {
                        // check if its the default segment description ( M L Z N )
                        if ( seqSegments1.getLength() == 4 )
                        {
                            if ( ( seqSegments1[ 0 ].Command == EnhancedCustomShapeSegmentCommand::MOVETO )
                                && ( seqSegments1[ 1 ].Command == EnhancedCustomShapeSegmentCommand::LINETO )
                                && ( seqSegments1[ 2 ].Command == EnhancedCustomShapeSegmentCommand::CLOSESUBPATH )
                                && ( seqSegments1[ 3 ].Command == EnhancedCustomShapeSegmentCommand::ENDSUBPATH ) )
                                bIsDefaultGeometry = true;
                        }
                    }
                }
            }
            else if ( pDefCustomShape && ( ( pDefCustomShape->nElements == 0 ) || ( pDefCustomShape->pElements == 0 ) ) )
                bIsDefaultGeometry = true;
        }
        break;

        case DEFAULT_STRETCHX :
        {
            const rtl::OUString sStretchX( RTL_CONSTASCII_USTRINGPARAM ( "StretchX" ) );
            pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sStretchX );
            if ( pAny && pDefCustomShape )
            {
                sal_Int32 nStretchX = 0;
                if ( *pAny >>= nStretchX )
                {
                    if ( pDefCustomShape->nXRef == nStretchX )
                        bIsDefaultGeometry = true;
                }
            }
            else if ( pDefCustomShape && ( pDefCustomShape->nXRef == DEFAULT_MINIMUM_SIGNED_COMPARE ) )
                bIsDefaultGeometry = true;
        }
        break;

        case DEFAULT_STRETCHY :
        {
            const rtl::OUString sStretchY( RTL_CONSTASCII_USTRINGPARAM ( "StretchY" ) );
            pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sStretchY );
            if ( pAny && pDefCustomShape )
            {
                sal_Int32 nStretchY = 0;
                if ( *pAny >>= nStretchY )
                {
                    if ( pDefCustomShape->nYRef == nStretchY )
                        bIsDefaultGeometry = true;
                }
            }
            else if ( pDefCustomShape && ( pDefCustomShape->nYRef == DEFAULT_MINIMUM_SIGNED_COMPARE ) )
                bIsDefaultGeometry = true;
        }
        break;

        case DEFAULT_EQUATIONS :
        {
            const rtl::OUString sEquations( RTL_CONSTASCII_USTRINGPARAM( "Equations" ) );
            pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sEquations );
            if ( pAny && pDefCustomShape && pDefCustomShape->nCalculation && pDefCustomShape->pCalculation )
            {
                com::sun::star::uno::Sequence< rtl::OUString > seqEquations1, seqEquations2;
                if ( *pAny >>= seqEquations1 )
                {
                    sal_Int32 i, nCount = pDefCustomShape->nCalculation;
                    seqEquations2.realloc( nCount );

                    const SvxMSDffCalculationData* pData = pDefCustomShape->pCalculation;
                    for ( i = 0; i < nCount; i++, pData++ )
                        seqEquations2[ i ] = EnhancedCustomShape2d::GetEquation( pData->nFlags, pData->nVal[ 0 ], pData->nVal[ 1 ], pData->nVal[ 2 ] );

                    if ( seqEquations1 == seqEquations2 )
                        bIsDefaultGeometry = true;
                }
            }
            else if ( pDefCustomShape && ( ( pDefCustomShape->nCalculation == 0 ) || ( pDefCustomShape->pCalculation == 0 ) ) )
                bIsDefaultGeometry = true;
        }
        break;

        case DEFAULT_TEXTFRAMES :
        {
            const rtl::OUString sTextFrames( RTL_CONSTASCII_USTRINGPARAM( "TextFrames" ) );
            pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sTextFrames );
            if ( pAny && pDefCustomShape && pDefCustomShape->nTextRect && pDefCustomShape->pTextRect )
            {
                com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeTextFrame > seqTextFrames1, seqTextFrames2;
                if ( *pAny >>= seqTextFrames1 )
                {
                    sal_Int32 i, nCount = pDefCustomShape->nTextRect;
                    seqTextFrames2.realloc( nCount );
                    const SvxMSDffTextRectangles* pRectangles = pDefCustomShape->pTextRect;
                    for ( i = 0; i < nCount; i++, pRectangles++ )
                    {
                        EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames2[ i ].TopLeft.First,    pRectangles->nPairA.nValA );
                        EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames2[ i ].TopLeft.Second,   pRectangles->nPairA.nValB );
                        EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames2[ i ].BottomRight.First,  pRectangles->nPairB.nValA );
                        EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames2[ i ].BottomRight.Second, pRectangles->nPairB.nValB );
                    }
                    if ( seqTextFrames1 == seqTextFrames2 )
                        bIsDefaultGeometry = true;
                }
            }
            else if ( pDefCustomShape && ( ( pDefCustomShape->nTextRect == 0 ) || ( pDefCustomShape->pTextRect == 0 ) ) )
                bIsDefaultGeometry = true;
        }
        break;

        case DEFAULT_HANDLES :
        {
            const rtl::OUString sHandles( RTL_CONSTASCII_USTRINGPARAM( "Handles" ) );
            pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sHandles );
            if ( pAny && pDefCustomShape && pDefCustomShape->nHandles && pDefCustomShape->pHandles )
            {
                com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValues > seqHandles1, seqHandles2;
                if ( *pAny >>= seqHandles1 )
                {
                    sal_Int32 i, n, nCount = pDefCustomShape->nHandles;
                    const SvxMSDffHandle* pData = pDefCustomShape->pHandles;
                    seqHandles2.realloc( nCount );
                    for ( i = 0; i < nCount; i++, pData++ )
                    {
                        sal_Int32 nPropertiesNeeded = 1;    // position is always needed
                        sal_Int32 nFlags = pData->nFlags;
                        if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
                            nPropertiesNeeded++;
                        if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
                            nPropertiesNeeded++;
                        if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
                            nPropertiesNeeded++;
                        if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
                        {
                            nPropertiesNeeded++;
                            if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
                            {
                                if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
                                    nPropertiesNeeded++;
                                if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
                                    nPropertiesNeeded++;
                            }
                        }
                        else if ( nFlags & MSDFF_HANDLE_FLAGS_RANGE )
                        {
                            if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
                                nPropertiesNeeded++;
                            if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
                                nPropertiesNeeded++;
                            if ( pData->nRangeYMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
                                nPropertiesNeeded++;
                            if ( pData->nRangeYMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
                                nPropertiesNeeded++;
                        }

                        n = 0;
                        com::sun::star::beans::PropertyValues& rPropValues = seqHandles2[ i ];
                        rPropValues.realloc( nPropertiesNeeded );

                        // POSITION
                        {
                            const rtl::OUString sPosition( RTL_CONSTASCII_USTRINGPARAM ( "Position" ) );
                            ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aPosition;
                            EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.First, pData->nPositionX, true, true );
                            EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.Second, pData->nPositionY, true, false );
                            rPropValues[ n ].Name = sPosition;
                            rPropValues[ n++ ].Value <<= aPosition;
                        }
                        if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
                        {
                            const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
                            bool bMirroredX = true;
                            rPropValues[ n ].Name = sMirroredX;
                            rPropValues[ n++ ].Value <<= bMirroredX;
                        }
                        if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
                        {
                            const rtl::OUString sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
                            bool bMirroredY = true;
                            rPropValues[ n ].Name = sMirroredY;
                            rPropValues[ n++ ].Value <<= bMirroredY;
                        }
                        if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
                        {
                            const rtl::OUString sSwitched( RTL_CONSTASCII_USTRINGPARAM ( "Switched" ) );
                            bool bSwitched = true;
                            rPropValues[ n ].Name = sSwitched;
                            rPropValues[ n++ ].Value <<= bSwitched;
                        }
                        if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
                        {
                            const rtl::OUString sPolar( RTL_CONSTASCII_USTRINGPARAM ( "Polar" ) );
                            ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aCenter;
                            EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aCenter.First,  pData->nCenterX,
                                ( nFlags & MSDFF_HANDLE_FLAGS_CENTER_X_IS_SPECIAL ) != 0, true  );
                            EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aCenter.Second, pData->nCenterY,
                                ( nFlags & MSDFF_HANDLE_FLAGS_CENTER_Y_IS_SPECIAL ) != 0, false );
                            rPropValues[ n ].Name = sPolar;
                            rPropValues[ n++ ].Value <<= aCenter;
                            if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
                            {
                                if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
                                {
                                    const rtl::OUString sRadiusRangeMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMinimum" ) );
                                    ::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
                                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMinimum, pData->nRangeXMin,
                                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, true  );
                                    rPropValues[ n ].Name = sRadiusRangeMinimum;
                                    rPropValues[ n++ ].Value <<= aRadiusRangeMinimum;
                                }
                                if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
                                {
                                    const rtl::OUString sRadiusRangeMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMaximum" ) );
                                    ::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
                                    EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMaximum, pData->nRangeXMax,
                                        ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, false );
                                    rPropValues[ n ].Name = sRadiusRangeMaximum;
                                    rPropValues[ n++ ].Value <<= aRadiusRangeMaximum;
                                }
                            }
                        }
                        else if ( nFlags & MSDFF_HANDLE_FLAGS_RANGE )
                        {
                            if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
                            {
                                const rtl::OUString sRangeXMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMinimum" ) );
                                ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMinimum;
                                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMinimum, pData->nRangeXMin,
                                    ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, true  );
                                rPropValues[ n ].Name = sRangeXMinimum;
                                rPropValues[ n++ ].Value <<= aRangeXMinimum;
                            }
                            if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
                            {
                                const rtl::OUString sRangeXMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMaximum" ) );
                                ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMaximum;
                                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMaximum, pData->nRangeXMax,
                                    ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, false );
                                rPropValues[ n ].Name = sRangeXMaximum;
                                rPropValues[ n++ ].Value <<= aRangeXMaximum;
                            }
                            if ( pData->nRangeYMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
                            {
                                const rtl::OUString sRangeYMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMinimum" ) );
                                ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMinimum;
                                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMinimum, pData->nRangeYMin,
                                    ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MIN_IS_SPECIAL ) != 0, true );
                                rPropValues[ n ].Name = sRangeYMinimum;
                                rPropValues[ n++ ].Value <<= aRangeYMinimum;
                            }
                            if ( pData->nRangeYMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
                            {
                                const rtl::OUString sRangeYMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMaximum" ) );
                                ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMaximum;
                                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMaximum, pData->nRangeYMax,
                                    ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MAX_IS_SPECIAL ) != 0, false );
                                rPropValues[ n ].Name = sRangeYMaximum;
                                rPropValues[ n++ ].Value <<= aRangeYMaximum;
                            }
                        }
                    }
                    if ( seqHandles1 == seqHandles2 )
                        bIsDefaultGeometry = true;
                }
            }
            else if ( pDefCustomShape && ( ( pDefCustomShape->nHandles == 0 ) || ( pDefCustomShape->pHandles == 0 ) ) )
                bIsDefaultGeometry = true;
        }
        break;
    }
    return bIsDefaultGeometry;
}

void SdrObjCustomShape::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.mbResizeFreeAllowed = basegfx::fTools::equalZero(getSdrObjectRotate()); // TTTT: 0.0 == fObjectRotation;
    rInfo.mbResizePropAllowed = true;
    rInfo.mbRotateFreeAllowed = true;
    rInfo.mbRotate90Allowed = true;
    rInfo.mbMirrorFreeAllowed = true;
    rInfo.mbMirror45Allowed = true;
    rInfo.mbMirror90Allowed = true;
    rInfo.mbTransparenceAllowed = false;
    rInfo.mbGradientAllowed = false;
    rInfo.mbShearAllowed = true;
    rInfo.mbEdgeRadiusAllowed = false;
    rInfo.mbNoContortion = true;

    // #i37011#
    if ( mXRenderedCustomShape.is() )
    {
        const SdrObject* pRenderedCustomShape = GetSdrObjectFromXShape( mXRenderedCustomShape );
        if ( pRenderedCustomShape )
        {
            // #i37262#
            // Iterate self over the contained objects, since there are combinations of
            // polygon and curve objects. In that case, aInfo.mbCanConvToPath and
            // aInfo.mbCanConvToPoly would be false. What is needed here is an or, not an and.
            SdrObjListIter aIterator(*pRenderedCustomShape);
            while(aIterator.IsMore())
            {
                SdrObject* pCandidate = aIterator.Next();
                SdrObjTransformInfoRec aInfo;
                pCandidate->TakeObjInfo(aInfo);

                // set path and poly conversion if one is possible since
                // this object will first be broken
                const bool bCanConvToPathOrPoly(aInfo.mbCanConvToPath || aInfo.mbCanConvToPoly);

                if(rInfo.mbCanConvToPath != bCanConvToPathOrPoly)
                {
                    rInfo.mbCanConvToPath = bCanConvToPathOrPoly;
                }

                if(rInfo.mbCanConvToPoly != bCanConvToPathOrPoly)
                {
                    rInfo.mbCanConvToPoly = bCanConvToPathOrPoly;
                }

                if(rInfo.mbCanConvToContour != aInfo.mbCanConvToContour)
                {
                    rInfo.mbCanConvToContour = aInfo.mbCanConvToContour;
                }
            }
        }
    }
}

sal_uInt16 SdrObjCustomShape::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_CUSTOMSHAPE);
}

// #115391# This implementation is based on the relative TextFrame size of the CustomShape and the
// state of the ResizeShapeToFitText flag to correctly set TextMinFrameWidth/Height
void SdrObjCustomShape::AdaptTextMinSize()
{
    if(!IsPasteResize())
    {
        const bool bResizeShapeToFitText(0 != static_cast< const SdrOnOffItem& >(GetObjectItem(SDRATTR_TEXT_AUTOGROWHEIGHT)).GetValue());
        SfxItemSet aSet(GetObjectItemSet());
        bool bChanged(false);

        if(bResizeShapeToFitText)
        {
            // always reset MinWidthHeight to zero to only rely on text size and frame size
            // to allow resizing being completely dependent on text size only
            aSet.Put(SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH, 0));
            aSet.Put(SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT, 0));
            bChanged = true;
        }
        else
        {
            // recreate from CustomShape-specific TextBounds. getUnifiedTextRange
            // is relative to the object (in unit coordinates) and already without
            // GetText...Distance values
            const basegfx::B2DRange aUnifiedTextRange(getUnifiedTextRange());
            const basegfx::B2DVector aAbsoluteObjectScale(basegfx::absolute(getSdrObjectScale()));

            aSet.Put(SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH, basegfx::fround(aUnifiedTextRange.getWidth() * aAbsoluteObjectScale.getX())));
            aSet.Put(SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT, basegfx::fround(aUnifiedTextRange.getHeight() * aAbsoluteObjectScale.getY())));
            bChanged = true;
        }

        if(bChanged)
        {
            SetObjectItemSet(aSet);
            AdjustTextFrameWidthAndHeight();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjCustomShape::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    if(rTransformation != getSdrObjectTransformation())
    {
        // call parent
        SdrTextObj::setSdrObjectTransformation(rTransformation);

        // #115391#
        AdaptTextMinSize();
    }

    // TTTT: extract mirror flags and trigger SetMirroredX/SetMirroredY
    //      const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
    //      PropertyValue aPropVal;
    //      aPropVal.Name = sMirroredX;
    //      aPropVal.Value <<= bHorz;
    //      aGeometryItem.SetPropertyValue( aPropVal );

    // TTTT: extra-actions from old NbcSetSnapRect

    // TTTT: extra evtl. adapt InteractionHandles
    //std::vector< SdrCustomShapeInteraction >::iterator aIter( aInteractionHandles.begin() );
    //while ( aIter != aInteractionHandles.end() )
    //{
    //  try
    //  {
    //      if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_FIXED )
    //          aIter->xInteraction->setControllerPosition( aIter->aPosition );
    //      if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_X )
    //      {
    //          sal_Int32 nX = ( aIter->aPosition.X - aOld.Left() ) + aRect.Left();
    //          aIter->xInteraction->setControllerPosition( com::sun::star::awt::Point( nX, aIter->xInteraction->getPosition().Y ) );
    //      }
    //      if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_Y )
    //      {
    //          sal_Int32 nY = ( aIter->aPosition.Y - aOld.Top() ) + aRect.Top();
    //          aIter->xInteraction->setControllerPosition( com::sun::star::awt::Point( aIter->xInteraction->getPosition().X, nY ) );
    //      }
    //  }
    //  catch ( const uno::RuntimeException& )
    //  {
    //  }
    //  aIter++;
    //}

    // TTTT: fObjectRotation?

}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjCustomShape::AddToHdlList(SdrHdlList& rHdlList) const
{
    // add handles from parent object
    SdrTextObj::AddToHdlList(rHdlList);

    // add own handles
    std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( this ) );

    for(sal_uInt32 a(0); a < aInteractionHandles.size(); a++)
    {
        if(aInteractionHandles[a].xInteraction.is())
        {
            try
            {
                const com::sun::star::awt::Point aPosition(aInteractionHandles[a].xInteraction->getPosition());
                SdrHdl* pHdl = new SdrHdl(rHdlList, this, HDL_CUSTOMSHAPE1, basegfx::B2DPoint(aPosition.X, aPosition.Y));
                pHdl->SetPointNum(a);
            }
            catch ( const uno::RuntimeException& )
            {
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrObjCustomShape::hasSpecialDrag() const
{
    return true;
}

bool SdrObjCustomShape::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl = rDrag.GetActiveHdl();

    if(pHdl && HDL_CUSTOMSHAPE1 == pHdl->GetKind())
    {
        rDrag.SetEndDragChangesAttributes(true);
        rDrag.SetNoSnap(true);
    }
    else
    {
        const SdrHdl* pHdl2 = rDrag.GetActiveHdl();
        const SdrHdlKind eHdl((pHdl2 == NULL) ? HDL_MOVE : pHdl2->GetKind());

        switch( eHdl )
        {
            case HDL_UPLFT :
            case HDL_UPPER :
            case HDL_UPRGT :
            case HDL_LEFT  :
            case HDL_RIGHT :
            case HDL_LWLFT :
            case HDL_LOWER :
            case HDL_LWRGT :
            case HDL_MOVE  :
            {
                break;
            }
            default:
            {
                return false;
            }
        }
    }

    return true;
}

void SdrObjCustomShape::DragMoveCustomShapeHdl( const basegfx::B2DPoint& rDestination, const sal_uInt32 nCustomShapeHdlNum, SdrObjCustomShape* pObj ) const
{
    std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( pObj ) );
    if ( nCustomShapeHdlNum < aInteractionHandles.size() )
    {
        SdrCustomShapeInteraction aInteractionHandle( aInteractionHandles[ nCustomShapeHdlNum ] );
        if ( aInteractionHandle.xInteraction.is() )
        {
            try
            {
                com::sun::star::awt::Point aPt( basegfx::fround(rDestination.getX()), basegfx::fround(rDestination.getY()) );
                if ( aInteractionHandle.nMode & CUSTOMSHAPE_HANDLE_MOVE_SHAPE )
                {
                    const sal_Int32 nXDiff(aPt.X - aInteractionHandle.aPosition.X);
                    const sal_Int32 nYDiff(aPt.Y - aInteractionHandle.aPosition.Y);

                    sdr::legacy::transformSdrObject(*pObj, basegfx::tools::createTranslateB2DHomMatrix(nXDiff, nYDiff));

                    std::vector< SdrCustomShapeInteraction >::iterator aIter( aInteractionHandles.begin() );
                    while ( aIter != aInteractionHandles.end() )
                    {
                        if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_FIXED )
                        {
                            if ( aIter->xInteraction.is() )
                                aIter->xInteraction->setControllerPosition( aIter->aPosition );
                        }
                        aIter++;
                    }
                }
                aInteractionHandle.xInteraction->setControllerPosition( aPt );
            }
            catch ( const uno::RuntimeException& )
            {
            }
        }
    }
}

bool SdrObjCustomShape::applySpecialDrag(SdrDragStat& rDrag)
{
    const SdrHdl* pHdl = rDrag.GetActiveHdl();
    const SdrHdlKind eHdl((pHdl == NULL) ? HDL_MOVE : pHdl->GetKind());
    bool bRetval(true);

    switch(eHdl)
    {
        case HDL_CUSTOMSHAPE1 :
        {
            rDrag.SetEndDragChangesGeoAndAttributes(true);
            DragMoveCustomShapeHdl( rDrag.GetNow(), pHdl->GetPointNum(), this );
            InvalidateRenderGeometry();
            SetChanged();
            break;
        }

        case HDL_UPLFT :
        case HDL_UPPER :
        case HDL_UPRGT :
        case HDL_LEFT  :
        case HDL_RIGHT :
        case HDL_LWLFT :
        case HDL_LOWER :
        case HDL_LWRGT :
        {
            bRetval = SdrTextObj::applySpecialDrag(rDrag);
            break;
        }
        case HDL_MOVE :
        {
            sdr::legacy::MoveSdrObject(*this, Size(basegfx::fround(rDrag.GetDX()), basegfx::fround(rDrag.GetDY())));
            break;
        }
        default: break;
    }

    return bRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjCustomShape::DragCreateObject( SdrDragStat& rStat )
{
    basegfx::B2DRange aRange(rStat.TakeCreateRange());

    std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( this ) );

    sal_uInt32 nDefaultObjectSizeWidth = 3000;      // default width from SDOptions ?
    sal_uInt32 nDefaultObjectSizeHeight= 3000;

    if ( ImpVerticalSwitch( *this ) ) // TTTT: What is this good for?
    {
        // TTTT: Not possible, maybe (if needed) need to use rStat.GetNow() - rStat.GetStart()
        // to detect mirroring
        // SetMirroredX( aRange.getMinX() > aRange.getMaxX() );

        aRange = basegfx::B2DRange( rStat.GetNow(), basegfx::B2DTuple( nDefaultObjectSizeWidth, nDefaultObjectSizeHeight ) );
        // subtracting the horizontal difference of the latest handle from shape position
        if ( aInteractionHandles.size() )
        {
            sal_Int32 nHandlePos = aInteractionHandles[ aInteractionHandles.size() - 1 ].xInteraction->getPosition().X;
            const basegfx::B2DRange aOldObjRange(sdr::legacy::GetLogicRange(*this));

            aRange.transform(basegfx::tools::createTranslateB2DHomMatrix(aOldObjRange.getMinX() - nHandlePos, 0.0));
        }
    }

    rStat.SetActionRange( aRange );
    sdr::legacy::SetLogicRange(*this, aRange);

    std::vector< SdrCustomShapeInteraction >::iterator aIter( aInteractionHandles.begin() );
    while ( aIter != aInteractionHandles.end() )
    {
        try
        {
            if ( aIter->nMode & CUSTOMSHAPE_HANDLE_CREATE_FIXED )
            {
                aIter->xInteraction->setControllerPosition(
                    awt::Point( basegfx::fround(rStat.GetStart().getX()), basegfx::fround(rStat.GetStart().getY()) ) );
            }
        }
        catch ( const uno::RuntimeException& )
        {
        }
        aIter++;
    }

    ActionChanged();
}

bool SdrObjCustomShape::MovCreate(SdrDragStat& rStat)
{
    SdrView& rView = rStat.GetSdrViewFromSdrDragStat();     // #i37448#
    if( rView.IsSolidDragging() )
    {
        InvalidateRenderGeometry();
    }
    DragCreateObject( rStat );
    ActionChanged();

    return true;
}

bool SdrObjCustomShape::EndCreate( SdrDragStat& rStat, SdrCreateCmd eCmd )
{
    DragCreateObject( rStat );

    // #115391# TTTT: Calls setSdrObjectTransformation and thus AdaptTextMinSize automatically
    // AdaptTextMinSize();
    // ActionChanged();

    return ( eCmd == SDRCREATE_FORCEEND || rStat.GetPointAnz() >= 2 );
}

basegfx::B2DPolyPolygon SdrObjCustomShape::TakeCreatePoly(const SdrDragStat& /*rDrag*/) const
{
    return GetLineGeometry( this, false );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// in context with the SdrObjCustomShape the TextAutoGrowHeight == true -> Resize Shape to fit text,
//                                     the TextAutoGrowWidth  == true -> Word wrap text in Shape
bool SdrObjCustomShape::IsAutoGrowHeight() const
{
    const SfxItemSet& rSet = GetMergedItemSet();
    bool bIsAutoGrowHeight = ((SdrOnOffItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue();
    if ( bIsAutoGrowHeight && IsVerticalWriting() )
        bIsAutoGrowHeight = ((SdrOnOffItem&)(rSet.Get(SDRATTR_TEXT_WORDWRAP))).GetValue() == false;
    return bIsAutoGrowHeight;
}
bool SdrObjCustomShape::IsAutoGrowWidth() const
{
    const SfxItemSet& rSet = GetMergedItemSet();
    bool bIsAutoGrowWidth = ((SdrOnOffItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue();
    if ( bIsAutoGrowWidth && !IsVerticalWriting() )
        bIsAutoGrowWidth = ((SdrOnOffItem&)(rSet.Get(SDRATTR_TEXT_WORDWRAP))).GetValue() == false;
    return bIsAutoGrowWidth;
}

/* The following method is identical to the SdrTextObj::SetVerticalWriting method, the only difference
   is that the SdrAutoGrowWidthItem and SdrAutoGrowHeightItem are not exchanged if the vertical writing
   mode has been changed */

void SdrObjCustomShape::SetVerticalWriting( bool bVertical )
{
    ForceOutlinerParaObject();

    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();

    DBG_ASSERT( pOutlinerParaObject, "SdrTextObj::SetVerticalWriting() without OutlinerParaObject!" );

    if( pOutlinerParaObject )
    {
        if(pOutlinerParaObject->IsVertical() != (bool)bVertical)
        {
            // get item settings
            const SfxItemSet& rSet = GetObjectItemSet();

            // #103516# Also exchange hor/ver adjust items
            SdrTextHorzAdjust eHorz = ((SdrTextHorzAdjustItem&)(rSet.Get(SDRATTR_TEXT_HORZADJUST))).GetValue();
            SdrTextVertAdjust eVert = ((SdrTextVertAdjustItem&)(rSet.Get(SDRATTR_TEXT_VERTADJUST))).GetValue();

            // rescue object size
            Rectangle aObjectRect(sdr::legacy::GetSnapRect(*this));

            // prepare ItemSet to set exchanged width and height items
            SfxItemSet aNewSet(*rSet.GetPool(),
                SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT,
                // #103516# Expanded item ranges to also support hor and ver adjust.
                SDRATTR_TEXT_VERTADJUST, SDRATTR_TEXT_VERTADJUST,
                SDRATTR_TEXT_AUTOGROWWIDTH, SDRATTR_TEXT_HORZADJUST,
                0, 0);

            aNewSet.Put(rSet);

            // #103516# Exchange horz and vert adjusts
            switch(eVert)
            {
                case SDRTEXTVERTADJUST_TOP: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT)); break;
                case SDRTEXTVERTADJUST_CENTER: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER)); break;
                case SDRTEXTVERTADJUST_BOTTOM: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT)); break;
                case SDRTEXTVERTADJUST_BLOCK: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_BLOCK)); break;
            }
            switch(eHorz)
            {
                case SDRTEXTHORZADJUST_LEFT: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BOTTOM)); break;
                case SDRTEXTHORZADJUST_CENTER: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER)); break;
                case SDRTEXTHORZADJUST_RIGHT: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP)); break;
                case SDRTEXTHORZADJUST_BLOCK: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BLOCK)); break;
            }

            SetObjectItemSet( aNewSet );
            pOutlinerParaObject = GetOutlinerParaObject();
            if ( pOutlinerParaObject )
                pOutlinerParaObject->SetVertical(bVertical);

            // restore object size
            sdr::legacy::SetSnapRect(*this, aObjectRect);
        }
    }
}

basegfx::B2DRange SdrObjCustomShape::AdjustTextFrameWidthAndHeight(const basegfx::B2DRange& rRange) const
{
    basegfx::B2DRange aRetval(rRange);

    if(HasText() && !rRange.isEmpty())
    {
        aRetval = ImpAdjustTextFrameWidthAndHeight(rRange, false);
    }

    return aRetval;
}

basegfx::B2DRange SdrObjCustomShape::ImpCalculateTextFrame()
{
    basegfx::B2DRange aReturnValue;
    const basegfx::B2DRange aOldObjRange(
        getSdrObjectTranslate(),
        getSdrObjectTranslate() + basegfx::absolute(getSdrObjectScale()));

    // initial text rectangle
    const basegfx::B2DRange aOldTextRange(aOldObjRange);

    // new text range returned from the custom shape renderer. It is
    // in unit coordinates initially, so needs to be transformed by
    // object scale and position
    basegfx::B2DRange aNewTextRange(getRawUnifiedTextRange());
    aNewTextRange.transform(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            basegfx::absolute(getSdrObjectScale()),
            getSdrObjectTranslate()));

    // new text rectangle is being tested by AdjustTextFrameWidthAndHeight to ensure
    // that the new text rectangle is matching the current text size from the outliner
    const basegfx::B2DRange aAdjustedTextRange(AdjustTextFrameWidthAndHeight(aNewTextRange));

    if((aAdjustedTextRange != aNewTextRange) && (aOldTextRange != aAdjustedTextRange))
    {
        const basegfx::B2DVector aScale(aOldTextRange.getRange() / aNewTextRange.getRange());
        const basegfx::B2DVector aTopLeftDiff((aAdjustedTextRange.getMinimum() - aNewTextRange.getMinimum()) * aScale);
        const basegfx::B2DVector aBottomRightDiff((aAdjustedTextRange.getMaximum() - aNewTextRange.getMaximum()) * aScale);

        aReturnValue = basegfx::B2DRange(
            aOldObjRange.getMinimum() + aTopLeftDiff,
            aOldObjRange.getMaximum() + aBottomRightDiff);
    }

    return aReturnValue;
}

void SdrObjCustomShape::AdjustTextFrameWidthAndHeight()
{
    if(mbAdjustingTextFrameWidthAndHeight)
    {
        return;
    }

    mbAdjustingTextFrameWidthAndHeight = true;

    const basegfx::B2DRange aNewTextRange(
        ImpCalculateTextFrame());
    const basegfx::B2DRange aOldObjRange(
        getSdrObjectTranslate(),
        getSdrObjectTranslate() + basegfx::absolute(getSdrObjectScale()));

    if(!aNewTextRange.isEmpty() && !aNewTextRange.equal(aOldObjRange))
    {
        // taking care of handles that should not been changed
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
        std::vector< SdrCustomShapeInteraction > aInteractionHandles(GetInteractionHandles(this));

        // adapt transformation to new range and apply to object
        setSdrObjectTransformation(
            basegfx::tools::adaptB2DHomMatrixToB2DRange(
                getSdrObjectTransformation(),
                aNewTextRange));

        // corrections to InteractionHandles
        std::vector< SdrCustomShapeInteraction >::iterator aIter(aInteractionHandles.begin());

        while(aIter != aInteractionHandles.end())
        {
            try
            {
                if(aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_FIXED)
                {
                    aIter->xInteraction->setControllerPosition(aIter->aPosition);
                }
            }
            catch(const uno::RuntimeException&)
            {
            }

            aIter++;
        }

        InvalidateRenderGeometry();
        SetChanged();
    }

    mbAdjustingTextFrameWidthAndHeight = false;
}

bool SdrObjCustomShape::BegTextEdit( SdrOutliner& rOutl )
{
    return SdrTextObj::BegTextEdit( rOutl );
}

void SdrObjCustomShape::TakeTextEditArea(basegfx::B2DVector* pPaperMin, basegfx::B2DVector* pPaperMax, basegfx::B2DRange* pViewInit, basegfx::B2DRange* pViewMin) const
{
    // get TextRange without shear, rotate and mirror, just scaled
    // and centered in logic coordinates
    basegfx::B2DRange aViewInit(getScaledCenteredTextRange(*this));

    basegfx::B2DVector aPaperMin;
    basegfx::B2DVector aPaperMax;
    basegfx::B2DVector aAnkSiz(aViewInit.getRange());
    basegfx::B2DVector aMaxSiz(1000000.0, 1000000.0);

    if(!basegfx::fTools::equalZero(getSdrModelFromSdrObject().GetMaxObjectScale().getX()))
    {
        aMaxSiz.setX(getSdrModelFromSdrObject().GetMaxObjectScale().getX());
    }

    if(!basegfx::fTools::equalZero(getSdrModelFromSdrObject().GetMaxObjectScale().getY()))
    {
        aMaxSiz.setY(getSdrModelFromSdrObject().GetMaxObjectScale().getY());
    }

    SdrTextHorzAdjust eHAdj(GetTextHorizontalAdjust());
    SdrTextVertAdjust eVAdj(GetTextVerticalAdjust());

    double fMinWdt(std::max(1.0, (double)GetMinTextFrameWidth()));
    double fMinHgt(std::max(1.0, (double)GetMinTextFrameHeight()));
    double fMaxWdt(GetMaxTextFrameWidth());
    double fMaxHgt(GetMaxTextFrameHeight());

    if(basegfx::fTools::equalZero(fMaxWdt) || basegfx::fTools::more(fMaxWdt, aMaxSiz.getX()))
    {
        fMaxWdt = aMaxSiz.getX();
    }

    if(basegfx::fTools::equalZero(fMaxHgt) || basegfx::fTools::more(fMaxHgt, aMaxSiz.getY()))
    {
        fMaxHgt = aMaxSiz.getY();
    }

    if(((SdrOnOffItem&)(GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue())
    {
        if(IsVerticalWriting())
        {
            fMaxHgt = aAnkSiz.getY();
            fMinHgt = fMaxHgt;
        }
        else
        {
            fMaxWdt = aAnkSiz.getX();
            fMinWdt = fMaxWdt;
        }
    }

    aPaperMax.setX(fMaxWdt);
    aPaperMax.setY(fMaxHgt);

    aPaperMin.setX(fMinWdt);
    aPaperMin.setY(fMinHgt);

    if(pViewMin)
    {
        *pViewMin = aViewInit;
        const double fXFree(aAnkSiz.getX() - aPaperMin.getX());

        if(SDRTEXTHORZADJUST_LEFT == eHAdj)
        {
            *pViewMin = basegfx::B2DRange(
                pViewMin->getMinX(),
                pViewMin->getMinY(),
                pViewMin->getMaxX() - fXFree,
                pViewMin->getMaxY());
        }
        else if(SDRTEXTHORZADJUST_RIGHT == eHAdj)
        {
            *pViewMin = basegfx::B2DRange(
                pViewMin->getMinX() + fXFree,
                pViewMin->getMinY(),
                pViewMin->getMaxX(),
                pViewMin->getMaxY());
        }
        else
        {
            const double fNewMinX(pViewMin->getMinX() + (fXFree * 0.5));
            *pViewMin = basegfx::B2DRange(
                fNewMinX,
                pViewMin->getMinY(),
                fNewMinX + aPaperMin.getX(),
                pViewMin->getMaxY());
        }

        const double fYFree(aAnkSiz.getY() - aPaperMin.getY());

        if(SDRTEXTVERTADJUST_TOP == eVAdj)
        {
            *pViewMin = basegfx::B2DRange(
                pViewMin->getMinX(),
                pViewMin->getMinY(),
                pViewMin->getMaxX(),
                pViewMin->getMaxY() - fYFree);
        }
        else if(SDRTEXTVERTADJUST_BOTTOM == eVAdj)
        {
            *pViewMin = basegfx::B2DRange(
                pViewMin->getMinX(),
                pViewMin->getMinY() + fYFree,
                pViewMin->getMaxX(),
                pViewMin->getMaxY());
        }
        else
        {
            const double fNewMinY(pViewMin->getMinY() + (fYFree * 0.5));
            *pViewMin = basegfx::B2DRange(
                pViewMin->getMinX(),
                fNewMinY,
                pViewMin->getMaxX(),
                fNewMinY + aPaperMin.getY());
        }
    }

    if(IsVerticalWriting())
    {
        aPaperMin.setX(0.0);
    }
    else
    {
        // #33102#
        aPaperMin.setY(0.0);
    }

    if( eHAdj != SDRTEXTHORZADJUST_BLOCK )
    {
        aPaperMin.setX(0.0);
    }

    // #103516# For complete ver adjust support, set paper min height to 0, here.
    if(SDRTEXTVERTADJUST_BLOCK != eVAdj )
    {
        aPaperMin.setY(0.0);
    }

    if(pPaperMin)
    {
        *pPaperMin = aPaperMin;
    }

    if(pPaperMax)
    {
        *pPaperMax = aPaperMax;
    }

    if(pViewInit)
    {
        *pViewInit = aViewInit;
    }
}
void SdrObjCustomShape::EndTextEdit( SdrOutliner& rOutl )
{
    SdrTextObj::EndTextEdit( rOutl );
    InvalidateRenderGeometry();
}
basegfx::B2DRange SdrObjCustomShape::getRawUnifiedTextRange() const
{
    // a candidate for being cached
    Reference< XCustomShapeEngine > xCustomShapeEngine(GetCustomShapeEngine(this));

    if(xCustomShapeEngine.is())
    {
        awt::Rectangle aR(xCustomShapeEngine->getTextBounds());

        if(aR.Width || aR.Height)
        {
            // the text bounds from CustomShapeEngine can be seen as scaled and translated
            // whereby scale is absolute (without mirrorings AFAIK). To get the unified range,
            // multiply with the inverse of M = T(obj) * S(obj). This can be done directly
            // here
            const double fAbsInvScaleX(basegfx::fTools::equalZero(getSdrObjectScale().getX()) ? 1.0 : 1.0 / fabs(getSdrObjectScale().getX()));
            const double fAbsInvScaleY(basegfx::fTools::equalZero(getSdrObjectScale().getY()) ? 1.0 : 1.0 / fabs(getSdrObjectScale().getY()));
            const double fX((aR.X - getSdrObjectTranslate().getX()) * fAbsInvScaleX);
            const double fY((aR.Y - getSdrObjectTranslate().getY()) * fAbsInvScaleY);
            const double fW(aR.Width * fAbsInvScaleX);
            const double fH(aR.Height * fAbsInvScaleY);

            return basegfx::B2DRange(fX, fY, fX + fW, fY + fH);
        }
    }

    // per default the text range is the whole object range
    return basegfx::B2DRange::getUnitB2DRange();
}
basegfx::B2DRange SdrObjCustomShape::getUnifiedTextRange() const
{
    // get raw range (without text borders)
    basegfx::B2DRange aRawRange(getRawUnifiedTextRange());
    const double fAbsInvScaleX(basegfx::fTools::equalZero(getSdrObjectScale().getX()) ? 1.0 : 1.0 / fabs(getSdrObjectScale().getX()));
    const double fAbsInvScaleY(basegfx::fTools::equalZero(getSdrObjectScale().getY()) ? 1.0 : 1.0 / fabs(getSdrObjectScale().getY()));

    // add/remove the text borders
    return basegfx::B2DRange(
        aRawRange.getMinX() + (GetTextLeftDistance() * fAbsInvScaleX),
        aRawRange.getMinY() + (GetTextUpperDistance() * fAbsInvScaleY),
        aRawRange.getMaxX() - (GetTextRightDistance() * fAbsInvScaleX),
        aRawRange.getMaxY() - (GetTextLowerDistance() * fAbsInvScaleY));
}
void SdrObjCustomShape::TakeTextRange(SdrOutliner& rOutliner, basegfx::B2DRange& rTextRange, basegfx::B2DRange& rAnchorRange) const
{
    // get TextRange without shear, rotate and mirror, just scaled
    // and centered in logic coordinates
    rAnchorRange = getScaledCenteredTextRange(*this);

    // Rect innerhalb dem geankert wird
    SdrTextVertAdjust eVAdj = GetTextVerticalAdjust();
    SdrTextHorzAdjust eHAdj = GetTextHorizontalAdjust();
    sal_uInt32 nStat0 = rOutliner.GetControlWord();
    Size aNullSize;

    rOutliner.SetControlWord(nStat0|EE_CNTRL_AUTOPAGESIZE);
    rOutliner.SetMinAutoPaperSize(aNullSize);

    sal_Int32 nMaxAutoPaperWidth = 1000000;
    sal_Int32 nMaxAutoPaperHeight= 1000000;

    long nAnkWdt(basegfx::fround(rAnchorRange.getWidth()));
    long nAnkHgt(basegfx::fround(rAnchorRange.getHeight()));

    if(((SdrOnOffItem&)(GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue())
    {
        if(IsVerticalWriting())
        {
            nMaxAutoPaperHeight = nAnkHgt;
        }
        else
        {
            nMaxAutoPaperWidth = nAnkWdt;
        }
    }

    if(SDRTEXTHORZADJUST_BLOCK == eHAdj && !IsVerticalWriting())
    {
        rOutliner.SetMinAutoPaperSize(Size(nAnkWdt, 0));
    }

    if(SDRTEXTVERTADJUST_BLOCK == eVAdj && IsVerticalWriting())
    {
        rOutliner.SetMinAutoPaperSize(Size(0, nAnkHgt));
    }

    rOutliner.SetMaxAutoPaperSize( Size( nMaxAutoPaperWidth, nMaxAutoPaperHeight ) );
    rOutliner.SetPaperSize( aNullSize );

    // Text in den Outliner stecken - ggf. den aus dem EditOutliner
    OutlinerParaObject* pPara = GetOutlinerParaObject();
    bool bNeedToDestroy(false);

    if(pEdtOutl)
    {
        pPara = pEdtOutl->CreateParaObject();
        bNeedToDestroy = true;
    }

    if (pPara)
    {
        const SdrTextObj* pTestObj = rOutliner.GetTextObj();

        if( !pTestObj || pTestObj != this ||
            pTestObj->GetOutlinerParaObject() != GetOutlinerParaObject() )
        {
            rOutliner.SetTextObj( this );
            rOutliner.SetUpdateMode(true);
            rOutliner.SetText(*pPara);
        }
    }
    else
    {
        rOutliner.SetTextObj( NULL );
    }

    if (bNeedToDestroy && pPara)
    {
        delete pPara;
    }

    rOutliner.SetUpdateMode(true);
    rOutliner.SetControlWord(nStat0);

    SdrText* pText = getActiveText();

    if( pText )
    {
        pText->CheckPortionInfo( rOutliner );
    }

    basegfx::B2DPoint aTextPos(rAnchorRange.getMinimum());
    const basegfx::B2DVector aTextSiz(rOutliner.GetPaperSize().getWidth(), rOutliner.GetPaperSize().getHeight());

    // #106653#
    // For draw objects containing text correct hor/ver alignment if text is bigger
    // than the object itself. Without that correction, the text would always be
        // formatted to the left edge (or top edge when vertical) of the draw object.

    if( !IsTextFrame() )
    {
        if(rAnchorRange.getWidth() < aTextSiz.getX() && !IsVerticalWriting())
        {
            // #110129#
            // Horizontal case here. Correct only if eHAdj == SDRTEXTHORZADJUST_BLOCK,
            // else the alignment is wanted.
            if(SDRTEXTHORZADJUST_BLOCK == eHAdj)
            {
                eHAdj = SDRTEXTHORZADJUST_CENTER;
            }
        }

        if(rAnchorRange.getHeight() < aTextSiz.getY() && IsVerticalWriting())
        {
            // #110129#
            // Vertical case here. Correct only if eHAdj == SDRTEXTVERTADJUST_BLOCK,
            // else the alignment is wanted.
            if(SDRTEXTVERTADJUST_BLOCK == eVAdj)
            {
                eVAdj = SDRTEXTVERTADJUST_CENTER;
            }
        }
    }

    if (SDRTEXTHORZADJUST_CENTER == eHAdj || SDRTEXTHORZADJUST_RIGHT == eHAdj)
    {
        const double fFreeWdt(rAnchorRange.getWidth() - aTextSiz.getX());

        if(SDRTEXTHORZADJUST_CENTER == eHAdj)
        {
            aTextPos.setX(aTextPos.getX() + (fFreeWdt * 0.5));
        }
        else if(SDRTEXTHORZADJUST_RIGHT == eHAdj)
        {
            aTextPos.setX(aTextPos.getX() + fFreeWdt);
        }
    }

    if (SDRTEXTVERTADJUST_CENTER == eVAdj || SDRTEXTVERTADJUST_BOTTOM == eVAdj)
    {
        const double fFreeHgt(rAnchorRange.getHeight() - aTextSiz.getY());

        if(SDRTEXTVERTADJUST_CENTER == eVAdj)
        {
            aTextPos.setY(aTextPos.getY() + (fFreeHgt * 0.5));
        }
        else if(SDRTEXTVERTADJUST_BOTTOM == eVAdj)
        {
            aTextPos.setY(aTextPos.getY() + fFreeHgt);
        }
    }

    rTextRange = basegfx::B2DRange(aTextPos, aTextPos + aTextSiz);
}

void SdrObjCustomShape::SetChanged()
{
    // call parent
    SdrTextObj::SetChanged();

    // own reactions
    InvalidateRenderGeometry();
}

void SdrObjCustomShape::TakeObjNameSingul(XubString& rName) const
{
    rName = ImpGetResStr(STR_ObjNameSingulCUSTOMSHAPE);
    String aNm( GetName() );
    if( aNm.Len() )
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aNm;
        rName += sal_Unicode('\'');
    }
}

void SdrObjCustomShape::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralCUSTOMSHAPE);
}

basegfx::B2DPolyPolygon SdrObjCustomShape::TakeXorPoly() const
{
    return GetLineGeometry( (SdrObjCustomShape*)this, false );
}

SdrObject* SdrObjCustomShape::DoConvertToPolygonObject(bool bBezier, bool bAddText) const
{
    // #i37011#
    SdrObject* pRetval = 0L;
    SdrObject* pRenderedCustomShape = 0L;

    if ( !mXRenderedCustomShape.is() )
    {
        // force CustomShape
        ((SdrObjCustomShape*)this)->GetSdrObjectFromCustomShape();
    }

    if ( mXRenderedCustomShape.is() )
    {
        pRenderedCustomShape = GetSdrObjectFromXShape( mXRenderedCustomShape );
    }

    if ( pRenderedCustomShape )
    {
        SdrObject* pCandidate = pRenderedCustomShape->CloneSdrObject();
        DBG_ASSERT(pCandidate, "SdrObjCustomShape::DoConvertToPolygonObject: Could not clone SdrObject (!)");
        pRetval = pCandidate->DoConvertToPolygonObject(bBezier, bAddText);
        deleteSdrObjectSafeAndClearPointer(pCandidate);

        if(pRetval)
        {
            const bool bShadow(((SdrOnOffItem&)GetMergedItem(SDRATTR_SHADOW)).GetValue());
            if(bShadow)
            {
                pRetval->SetMergedItem(SdrOnOffItem(SDRATTR_SHADOW, true));
            }
        }

        if(bAddText && HasText() && !IsTextPath())
        {
            pRetval = ImpConvertAddText(pRetval, bBezier);
        }
    }

    return pRetval;
}

void SdrObjCustomShape::SetStyleSheet( SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr )
{
    // #i40944#
    InvalidateRenderGeometry();

    // call parent
    SdrObject::SetStyleSheet( pNewStyleSheet, bDontRemoveHardAttr );
}

void SdrObjCustomShape::handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage)
{
    if(pOldPage != pNewPage)
    {
        // call parent
        SdrTextObj::handlePageChange(pOldPage, pNewPage);

        if( pNewPage )
        {
            // invalidating rectangles by invalidateObjectRange is not sufficient,
            // AdjustTextFrameWidthAndHeight() also has to be made
            ActionChanged();
            AdjustTextFrameWidthAndHeight();
        }
    }
}

SdrObjGeoData* SdrObjCustomShape::NewGeoData() const
{
    return new SdrAShapeObjGeoData;
}

void SdrObjCustomShape::SaveGeoData(SdrObjGeoData& rGeo) const
{
    // call parent
    SdrTextObj::SaveGeoData( rGeo );

    SdrAShapeObjGeoData* pSdrAShapeObjGeoData = dynamic_cast< SdrAShapeObjGeoData* >(&rGeo);

    if(pSdrAShapeObjGeoData)
    {
    // TTTT: Should be obsolete
    //rAGeo.fObjectRotation = fObjectRotation;

    // TTTT: MirrorX/Y removed
    //rAGeo.bMirroredX = IsMirroredX();
    //rAGeo.bMirroredY = IsMirroredY();

        const rtl::OUString sAdjustmentValues(RTL_CONSTASCII_USTRINGPARAM("AdjustmentValues"));
        Any* pAny(((SdrCustomShapeGeometryItem&)GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY)).GetPropertyValueByName(sAdjustmentValues));

        if(pAny)
        {
            *pAny >>= pSdrAShapeObjGeoData->aAdjustmentSeq;
        }
    }
}

void SdrObjCustomShape::RestGeoData(const SdrObjGeoData& rGeo)
{
    // call parent
    SdrTextObj::RestGeoData( rGeo );

    const SdrAShapeObjGeoData* pSdrAShapeObjGeoData = dynamic_cast< const SdrAShapeObjGeoData* >(&rGeo);

    if(pSdrAShapeObjGeoData)
    {
    // TTTT: Should be obsolete
    //fObjectRotation = rAGeo.fObjectRotation;

    // TTTT: MirrorX/Y removed
    //SetMirroredX( rAGeo.bMirroredX );
    //SetMirroredY( rAGeo.bMirroredY );
        SdrCustomShapeGeometryItem rGeometryItem = (SdrCustomShapeGeometryItem&)GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
        const rtl::OUString sAdjustmentValues(RTL_CONSTASCII_USTRINGPARAM("AdjustmentValues"));
        PropertyValue aPropVal;

        aPropVal.Name = sAdjustmentValues;
        aPropVal.Value <<= pSdrAShapeObjGeoData->aAdjustmentSeq;
        rGeometryItem.SetPropertyValue(aPropVal);
        SetMergedItem(rGeometryItem);

        InvalidateRenderGeometry();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sdr::contact::ViewContact* SdrObjCustomShape::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrObjCustomShape(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace glue
    {
        // SdrEdgeObj implements it's own GluePointProvider since
        // - it supports standard AutoGluePoints -> StandardGluePointProvider
        // - it needs to add some non-user-defined UserGluePoints -> replace
        //   UserGluePoint methods
        //
        // This is a little bit tricky since this implementation has to 'mix' two
        // sources of UserGluePoints:
        // - custom-defined ones from the CustomShape visualization geometry (these
        //   will have the UserDefined-flag on 'false', see
        //   EnhancedCustomShape2d::ApplyGluePoints)
        // - the 'real', user-defined local ones
        //
        // This 'mix' still has to keep the unique ID paradigm at inserting new
        // GluePoints. Assuming that the visualization geometry
        // - will always create the same numer and list of GluePoints
        // - is able to create these on demand before user-defined ones are added
        // this mix is done on the fly in the implementatin below

        class SdrObjCustomShapeGluePointProvider : public StandardGluePointProvider
        {
        private:
            // access to the owner object; always valid due to the
            // lifetime being less or the same as the owner object
            const SdrObjCustomShape*            mpSource;

        protected:
            SdrObjCustomShapeGluePointProvider(const SdrObjCustomShapeGluePointProvider& rCandidate);
            virtual GluePointProvider& operator=(const GluePointProvider& rCandidate);

            // helper: get access to the custom-defined UserGluePoints' GluePointProvider. This
            // may/should create the CustomShape visualization geometry and the custom-defined
            // UserGluePoints with them
            GluePointProvider* getUserDefinedGluePointProvider() const;

        public:
            // construction, destruction, copying
            SdrObjCustomShapeGluePointProvider(const SdrObjCustomShape& rSource);
            virtual ~SdrObjCustomShapeGluePointProvider();

            // copying
            virtual GluePointProvider* Clone() const;

            // add new GluePoint, it gets a new ID assigned and a reference to the
            // new instance (copied to list) is returned. It will assert when
            // already added
            virtual GluePoint& addUserGluePoint(GluePoint& rNew);

            // remove GluePoint (will assert if it was not added)
            virtual void removeUserGluePoint(const GluePoint& rNew);

            // check on content
            virtual bool hasUserGluePoints() const;

            // find UserGluePoint by ID
            virtual GluePoint* findUserGluePointByID(sal_uInt32 nID) const;

            // get vector of UserGluePoints (pointers to the real points)
            virtual const GluePointVector getUserGluePointVector() const;

            // adapt UserGluePoints to changed absolute scale, e.g. when not relative and alignments have to be addressed
            virtual void adaptUserGluePointsToChangedScale(const basegfx::B2DVector& rOldScale, const basegfx::B2DVector& rNewScale);
        };

        SdrObjCustomShapeGluePointProvider::SdrObjCustomShapeGluePointProvider(const SdrObjCustomShape& rSource)
        :   StandardGluePointProvider(),
            mpSource(&rSource)
        {
        }

        SdrObjCustomShapeGluePointProvider::~SdrObjCustomShapeGluePointProvider()
        {
        }

        SdrObjCustomShapeGluePointProvider::SdrObjCustomShapeGluePointProvider(const SdrObjCustomShapeGluePointProvider& rCandidate)
        :   StandardGluePointProvider(),
            mpSource(rCandidate.mpSource)
        {
        }

        GluePointProvider& SdrObjCustomShapeGluePointProvider::operator=(const GluePointProvider& rCandidate)
        {
            // call parent to copy UserGluePoints
            StandardGluePointProvider::operator=(rCandidate);

            const SdrObjCustomShapeGluePointProvider* pSource = dynamic_cast< const SdrObjCustomShapeGluePointProvider* >(&rCandidate);

            if(pSource)
            {
                // copy source SdrObject
                mpSource = pSource->mpSource;
            }

            return *this;
        }

        GluePointProvider* SdrObjCustomShapeGluePointProvider::Clone() const
        {
            return new SdrObjCustomShapeGluePointProvider(*this);
        }

        GluePointProvider* SdrObjCustomShapeGluePointProvider::getUserDefinedGluePointProvider() const
        {
            const SdrObject* pGeometryVisualization = mpSource->GetSdrObjectFromCustomShape();

            if(pGeometryVisualization)
            {
                return &pGeometryVisualization->GetGluePointProvider();
            }

            return 0;
        }

        GluePoint& SdrObjCustomShapeGluePointProvider::addUserGluePoint(GluePoint& rNew)
        {
            if(StandardGluePointProvider::hasUserGluePoints())
            {
                // there are already user-defined UserGluePoints, thus the biggest ID
                // is already set and a newly added one will be based on it, increasing
                // it; call parent
                return StandardGluePointProvider::addUserGluePoint(rNew);
            }

            // no user-defined UserGluePoints yet; check if we have custom-defined oones
            GluePointProvider* pSource = getUserDefinedGluePointProvider();

            if(!pSource || !pSource->hasUserGluePoints())
            {
                // no custom-defined UserGluePoints; call parent
                return StandardGluePointProvider::addUserGluePoint(rNew);
            }

            // we have to add the first user-defined UserGluePoint and there are
            // custom-defined ones. Call parent to get the needed instance
            GluePoint& rRetval = StandardGluePointProvider::addUserGluePoint(rNew);

            // test if add did work; if yes, a changed entry will be returned
            if(&rRetval != &rNew)
            {
                // get new highest ID
                const GluePointVector aCustomGluePoints(pSource->getUserGluePointVector());

                if(aCustomGluePoints.empty())
                {
                    // error: pSource->hasUserGluePoints() had answered true before
                    OSL_ENSURE(false, "no custom-defined UserGluePoints, but was stated so before (!)");
                }
                else
                {
                    const GluePoint* pLastCustomGluePoint = aCustomGluePoints.back();

                    if(pLastCustomGluePoint)
                    {
                        // correct ID at new, first user-defined UserGluePoint
                        setIdAtGluePoint(rRetval, pLastCustomGluePoint->getID() + 1);
                    }
                    else
                    {
                        // error: not empty, but no last
                        OSL_ENSURE(false, "custom-defined UserGluePoints, but empty last entry (!)");
                    }
                }
            }

            return rRetval;
        }

        void SdrObjCustomShapeGluePointProvider::removeUserGluePoint(const GluePoint& rNew)
        {
            // non-user-defined GluePoints cannot be removed, thus just call parent and
            // try to remove locally. This method does not need to be overloaded, but it
            // is better for documentation purposes
            StandardGluePointProvider::removeUserGluePoint(rNew);
        }

        bool SdrObjCustomShapeGluePointProvider::hasUserGluePoints() const
        {
            if(StandardGluePointProvider::hasUserGluePoints())
            {
                // if we alraedy have own user-defined GluePoints we are done
                return true;
            }

            GluePointProvider* pSource = getUserDefinedGluePointProvider();

            if(pSource)
            {
                // return state of custom shape defining geometry
                return pSource->hasUserGluePoints();
            }

            return false;
        }

        GluePoint* SdrObjCustomShapeGluePointProvider::findUserGluePointByID(sal_uInt32 nID) const
        {
            // call parent
            GluePoint* pRetval = StandardGluePointProvider::findUserGluePointByID(nID);

            if(pRetval)
            {
                // done if found locally
                return pRetval;
            }

            GluePointProvider* pSource = getUserDefinedGluePointProvider();

            if(pSource)
            {
                // try to get from custom shape defining geometry
                pRetval = pSource->findUserGluePointByID(nID);
            }

            return pRetval;
        }

        const GluePointVector SdrObjCustomShapeGluePointProvider::getUserGluePointVector() const
        {
            GluePointProvider* pSource = getUserDefinedGluePointProvider();

            if(!pSource)
            {
                // no CustomShape GluePoints, return local state by calling parent
                return StandardGluePointProvider::getUserGluePointVector();
            }

            if(!StandardGluePointProvider::hasUserGluePoints())
            {
                // no local GluePoints, return from custom
                return pSource->getUserGluePointVector();
            }

            // need to mix both; add custom first
            GluePointVector aRetval(pSource->getUserGluePointVector());
            const GluePointVector aLocal(StandardGluePointProvider::getUserGluePointVector());

            aRetval.insert(aRetval.end(), aLocal.begin(), aLocal.end());
            return aRetval;
        }

        void SdrObjCustomShapeGluePointProvider::adaptUserGluePointsToChangedScale(const basegfx::B2DVector& rOldScale, const basegfx::B2DVector& rNewScale)
        {
            // non-user-defined GluePoints cannot be adapted/changed, thus just
            // call parent. This method does not need to be overloaded, but it
            // is better for documentation purposes
            StandardGluePointProvider::adaptUserGluePointsToChangedScale(rOldScale, rNewScale);
        }

    } // end of namespace glue
} // end of namespace sdr

sdr::glue::GluePointProvider* SdrObjCustomShape::CreateObjectSpecificGluePointProvider()
{
    return new sdr::glue::SdrObjCustomShapeGluePointProvider(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #i33136#

bool SdrObjCustomShape::doConstructOrthogonal(const ::rtl::OUString& rName)
{
    bool bRetval(false);
    static ::rtl::OUString Imps_sNameASOrtho_quadrat( RTL_CONSTASCII_USTRINGPARAM( "quadrat" ) );
    static ::rtl::OUString Imps_sNameASOrtho_round_quadrat( RTL_CONSTASCII_USTRINGPARAM( "round-quadrat" ) );
    static ::rtl::OUString Imps_sNameASOrtho_circle( RTL_CONSTASCII_USTRINGPARAM( "circle" ) );
    static ::rtl::OUString Imps_sNameASOrtho_circle_pie( RTL_CONSTASCII_USTRINGPARAM( "circle-pie" ) );
    static ::rtl::OUString Imps_sNameASOrtho_ring( RTL_CONSTASCII_USTRINGPARAM( "ring" ) );

    if(Imps_sNameASOrtho_quadrat.equalsIgnoreAsciiCase(rName))
    {
        bRetval = true;
    }
    else if(Imps_sNameASOrtho_round_quadrat.equalsIgnoreAsciiCase(rName))
    {
        bRetval = true;
    }
    else if(Imps_sNameASOrtho_circle.equalsIgnoreAsciiCase(rName))
    {
        bRetval = true;
    }
    else if(Imps_sNameASOrtho_circle_pie.equalsIgnoreAsciiCase(rName))
    {
        bRetval = true;
    }
    else if(Imps_sNameASOrtho_ring.equalsIgnoreAsciiCase(rName))
    {
        bRetval = true;
    }

    return bRetval;
}

// #i37011# centralize throw-away of render geometry
void SdrObjCustomShape::InvalidateRenderGeometry()
{
    mXRenderedCustomShape = 0L;
    deleteSdrObjectSafeAndClearPointer( mpLastShadowGeometry );
    mpLastShadowGeometry = 0L;
}

//IAccessibility2 Implementation 2009-----
::rtl::OUString SdrObjCustomShape::GetCustomShapeName()
{
    rtl::OUString sShapeName;
    rtl::OUString aEngine( ( (SfxStringItem&)( *this ).GetMergedItem( SDRATTR_CUSTOMSHAPE_ENGINE ) ).GetValue() );
    if ( !aEngine.getLength() || aEngine.equalsAscii( "com.sun.star.drawing.EnhancedCustomShapeEngine" ) )
    {
        rtl::OUString sShapeType;
        const rtl::OUString sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
        SdrCustomShapeGeometryItem& rGeometryItem( (SdrCustomShapeGeometryItem&)( *this ).GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
        Any* pAny = rGeometryItem.GetPropertyValueByName( sType );
        if ( pAny && ( *pAny >>= sShapeType ) )
            sShapeName = EnhancedCustomShapeTypeNames::GetAccName( sShapeType );
    }
    return sShapeName;
}
//-----IAccessibility2 Implementation 2009
// eof
