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

#include <svx/svdoashp.hxx>
#include "svx/unoapi.hxx"
#include <svx/unoshape.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/datetime.hxx>
#include <sfx2/lnkbase.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <comphelper/processfactory.hxx>
#include <svl/urihelper.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svx/svdogrp.hxx>
#include <tools/helpers.hxx>
#include <svx/svddrag.hxx>
#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include "svx/svditer.hxx"
#include <svx/svdobj.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoedge.hxx>  // for broadcasting connectors to Move
#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdstr.hrc"    // the object's name
#include <editeng/eeitem.hxx>
#include "editeng/editstat.hxx"
#include <svx/svdoutl.hxx>
#include <editeng/outlobj.hxx>
#include <svx/sdtfchim.hxx>
#include "svx/EnhancedCustomShapeGeometry.hxx"
#include "svx/EnhancedCustomShapeTypeNames.hxx"
#include "svx/EnhancedCustomShape2d.hxx"
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
#include <basegfx/tools/unotools.hxx>
#include "svdconv.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;

static void lcl_ShapeSegmentFromBinary( EnhancedCustomShapeSegment& rSegInfo, sal_uInt16 nSDat )
{
    switch( nSDat >> 8 )
    {
        case 0x00 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::LINETO;
            rSegInfo.Count   = nSDat & 0xff;
            if ( !rSegInfo.Count )
                rSegInfo.Count = 1;
            break;
        case 0x20 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CURVETO;
            rSegInfo.Count   = nSDat & 0xff;
            if ( !rSegInfo.Count )
                rSegInfo.Count = 1;
            break;
        case 0x40 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::MOVETO;
            rSegInfo.Count   = nSDat & 0xff;
            if ( !rSegInfo.Count )
                rSegInfo.Count = 1;
            break;
        case 0x60 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
            rSegInfo.Count   = 0;
            break;
        case 0x80 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
            rSegInfo.Count   = 0;
            break;
        case 0xa1 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO;
            rSegInfo.Count   = ( nSDat & 0xff ) / 3;
            break;
        case 0xa2 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE;
            rSegInfo.Count   = ( nSDat & 0xff ) / 3;
            break;
        case 0xa3 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARCTO;
            rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
            break;
        case 0xa4 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARC;
            rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
            break;
        case 0xa5 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO;
            rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
            break;
        case 0xa6 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARC;
            rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
            break;
        case 0xa7 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX;
            rSegInfo.Count   = nSDat & 0xff;
            break;
        case 0xa8 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY;
            rSegInfo.Count   = nSDat & 0xff;
            break;
        case 0xaa :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOFILL;
            rSegInfo.Count   = 0;
            break;
        case 0xab :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOSTROKE;
            rSegInfo.Count   = 0;
            break;
        default:
        case 0xf8 :
            rSegInfo.Command = EnhancedCustomShapeSegmentCommand::UNKNOWN;
            rSegInfo.Count   = nSDat;
            break;
    }
    return;
}

static MSO_SPT ImpGetCustomShapeType( const SdrObjCustomShape& rCustoShape )
{
    MSO_SPT eRetValue = mso_sptNil;

    OUString aEngine( ( (SdrCustomShapeEngineItem&)rCustoShape.GetMergedItem( SDRATTR_CUSTOMSHAPE_ENGINE ) ).GetValue() );
    if ( aEngine.isEmpty() || aEngine == "com.sun.star.drawing.EnhancedCustomShapeEngine" )
    {
        OUString sShapeType;
        const OUString sType( "Type" );
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
        default: break;
    }
    return bRet;
}

// #i37011# create a clone with all attributes changed to shadow attributes
// and translation executed, too.
SdrObject* ImpCreateShadowObjectClone(const SdrObject& rOriginal, const SfxItemSet& rOriginalSet)
{
    SdrObject* pRetval = 0L;
    const sal_Bool bShadow(((SdrShadowItem&)rOriginalSet.Get(SDRATTR_SHADOW)).GetValue());

    if(bShadow)
    {
        // create a shadow representing object
        const sal_Int32 nXDist(((SdrShadowXDistItem&)(rOriginalSet.Get(SDRATTR_SHADOWXDIST))).GetValue());
        const sal_Int32 nYDist(((SdrShadowYDistItem&)(rOriginalSet.Get(SDRATTR_SHADOWYDIST))).GetValue());
        const ::Color aShadowColor(((SdrShadowColorItem&)(rOriginalSet.Get(SDRATTR_SHADOWCOLOR))).GetColorValue());
        const sal_uInt16 nShadowTransparence(((SdrShadowTransparenceItem&)(rOriginalSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue());
        pRetval = rOriginal.Clone();
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
                    bAllFillUsed = (bSolidFillUsed && bGradientFillUsed && bHatchFillUsed && bBitmapFillUsed);
                }
                if(!bGradientFillUsed && XFILL_GRADIENT == eFillStyle)
                {
                    bGradientFillUsed = true;
                    bAllFillUsed = (bSolidFillUsed && bGradientFillUsed && bHatchFillUsed && bBitmapFillUsed);
                }
                if(!bHatchFillUsed && XFILL_HATCH == eFillStyle)
                {
                    bHatchFillUsed = true;
                    bAllFillUsed = (bSolidFillUsed && bGradientFillUsed && bHatchFillUsed && bBitmapFillUsed);
                }
                if(!bBitmapFillUsed && XFILL_BITMAP == eFillStyle)
                {
                    bBitmapFillUsed = true;
                    bAllFillUsed = (bSolidFillUsed && bGradientFillUsed && bHatchFillUsed && bBitmapFillUsed);
                }
            }
        }

        // translate to shadow coordinates
        pRetval->NbcMove(Size(nXDist, nYDist));

        // set items as needed
        SfxItemSet aTempSet(rOriginalSet);

        // if a SvxWritingModeItem (Top->Bottom) is set the text object
        // is creating a paraobject, but paraobjects can not be created without model. So
        // we are preventing the crash by setting the writing mode always left to right,
        // this is not bad since our shadow geometry does not contain text.
        aTempSet.Put( SvxWritingModeItem( com::sun::star::text::WritingMode_LR_TB, SDRATTR_TEXTDIRECTION ) );

        // no shadow
        aTempSet.Put(SdrShadowItem(sal_False));
        aTempSet.Put(SdrShadowXDistItem(0L));
        aTempSet.Put(SdrShadowYDistItem(0L));

        // line color and transparency like shadow
        if(bLineUsed)
        {
            aTempSet.Put(XLineColorItem(String(), aShadowColor));
            aTempSet.Put(XLineTransparenceItem(nShadowTransparence));
        }

        // fill color and transparency like shadow
        if(bSolidFillUsed)
        {
            aTempSet.Put(XFillColorItem(String(), aShadowColor));
            aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
        }

        // gradient and transparency like shadow
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
            aTempSet.Put(XFillGradientItem(aGradient));
            aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
        }

        // hatch and transparency like shadow
        if(bHatchFillUsed)
        {
            XHatch aHatch(((XFillHatchItem&)(rOriginalSet.Get(XATTR_FILLHATCH))).GetHatchValue());
            aHatch.SetColor(aShadowColor);
            aTempSet.Put(XFillHatchItem(aTempSet.GetPool(), aHatch));
            aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
        }

        // bitmap and transparency like shadow
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

Reference< XCustomShapeEngine > SdrObjCustomShape::GetCustomShapeEngine() const
{
    if (mxCustomShapeEngine.is())
        return mxCustomShapeEngine;

    OUString aEngine(((SdrCustomShapeEngineItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_ENGINE )).GetValue());
    if ( aEngine.isEmpty() )
        aEngine = "com.sun.star.drawing.EnhancedCustomShapeEngine";

    Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    Reference< XShape > aXShape = GetXShapeForSdrObject(const_cast<SdrObjCustomShape*>(this));
    if ( aXShape.is() )
    {
        if ( !aEngine.isEmpty() )
        {
            Sequence< Any > aArgument( 1 );
            Sequence< PropertyValue > aPropValues( 1 );
            aPropValues[ 0 ].Name = "CustomShape";
            aPropValues[ 0 ].Value <<= aXShape;
            aArgument[ 0 ] <<= aPropValues;
            Reference< XInterface > xInterface( xContext->getServiceManager()->createInstanceWithArgumentsAndContext( aEngine, aArgument, xContext ) );
            if ( xInterface.is() )
                mxCustomShapeEngine = Reference< XCustomShapeEngine >( xInterface, UNO_QUERY );
        }
    }

    return mxCustomShapeEngine;
}

const SdrObject* SdrObjCustomShape::GetSdrObjectFromCustomShape() const
{
    if ( !mXRenderedCustomShape.is() )
    {
        Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine() );
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
            const sal_Bool bShadow(((SdrShadowItem&)rOriginalSet.Get( SDRATTR_SHADOW )).GetValue());

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

sal_Bool SdrObjCustomShape::IsTextPath() const
{
    const OUString sTextPath( "TextPath" );
    sal_Bool bTextPathOn = sal_False;
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    Any* pAny = rGeometryItem.GetPropertyValueByName( sTextPath, sTextPath );
    if ( pAny )
        *pAny >>= bTextPathOn;
    return bTextPathOn;
}

sal_Bool SdrObjCustomShape::UseNoFillStyle() const
{
    sal_Bool bRet = sal_False;
    OUString sShapeType;
    const OUString sType( "Type" );
    SdrCustomShapeGeometryItem& rGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    Any* pAny = rGeometryItem.GetPropertyValueByName( sType );
    if ( pAny )
        *pAny >>= sShapeType;
    bRet = IsCustomShapeFilledByDefault( EnhancedCustomShapeTypeNames::Get( sType ) ) == 0;

    return bRet;
}

sal_Bool SdrObjCustomShape::IsMirroredX() const
{
    sal_Bool bMirroredX = sal_False;
    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    const OUString sMirroredX( "MirroredX" );
    com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredX );
    if ( pAny )
        *pAny >>= bMirroredX;
    return bMirroredX;
}
sal_Bool SdrObjCustomShape::IsMirroredY() const
{
    sal_Bool bMirroredY = sal_False;
    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    const OUString sMirroredY( "MirroredY" );
    com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredY );
    if ( pAny )
        *pAny >>= bMirroredY;
    return bMirroredY;
}
void SdrObjCustomShape::SetMirroredX( const sal_Bool bMirrorX )
{
    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    const OUString sMirroredX( "MirroredX" );
    PropertyValue aPropVal;
    aPropVal.Name = sMirroredX;
    aPropVal.Value <<= bMirrorX;
    aGeometryItem.SetPropertyValue( aPropVal );
    SetMergedItem( aGeometryItem );
}
void SdrObjCustomShape::SetMirroredY( const sal_Bool bMirrorY )
{
    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    const OUString sMirroredY( "MirroredY" );
    PropertyValue aPropVal;
    aPropVal.Name = sMirroredY;
    aPropVal.Value <<= bMirrorY;
    aGeometryItem.SetPropertyValue( aPropVal );
    SetMergedItem( aGeometryItem );
}

double SdrObjCustomShape::GetObjectRotation() const
{
    return fObjectRotation;
}

bool SdrObjCustomShape::IsPostRotate() const
{
    const com::sun::star::uno::Any* pAny;
    bool bPostRotate = false;
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    pAny = rGeometryItem.GetPropertyValueByName( "IsPostRotateAngle" );
    if ( pAny )
        *pAny >>= bPostRotate;
    return bPostRotate;
}

double SdrObjCustomShape::GetExtraTextRotation( const bool bPreRotation ) const
{
    const com::sun::star::uno::Any* pAny;
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    const OUString sTextRotateAngle( "TextRotateAngle" );
    const OUString sTextPreRotateAngle( "TextPreRotateAngle" );
    pAny = rGeometryItem.GetPropertyValueByName( bPreRotation ? sTextPreRotateAngle : sTextRotateAngle );
    double fExtraTextRotateAngle = 0.0;
    if ( pAny )
        *pAny >>= fExtraTextRotateAngle;
    return fExtraTextRotateAngle;
}

sal_Bool SdrObjCustomShape::GetTextBounds( Rectangle& rTextBound ) const
{
    sal_Bool bRet = sal_False;

    Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine() );
    if ( xCustomShapeEngine.is() )
    {
        awt::Rectangle aR( xCustomShapeEngine->getTextBounds() );
        if ( aR.Width > 1 && aR.Height > 1 )
        {
            rTextBound = Rectangle( Point( aR.X, aR.Y ), Size( aR.Width, aR.Height ) );
            bRet = sal_True;
        }
    }
    return bRet;
}
basegfx::B2DPolyPolygon SdrObjCustomShape::GetLineGeometry( const SdrObjCustomShape* pCustomShape, const sal_Bool bBezierAllowed )
{
    basegfx::B2DPolyPolygon aRetval;
    Reference< XCustomShapeEngine > xCustomShapeEngine( pCustomShape->GetCustomShapeEngine() );
    if ( xCustomShapeEngine.is() )
    {
        com::sun::star::drawing::PolyPolygonBezierCoords aBezierCoords = xCustomShapeEngine->getLineGeometry();
        try
        {
            aRetval = basegfx::unotools::polyPolygonBezierToB2DPolyPolygon( aBezierCoords );
            if ( !bBezierAllowed && aRetval.areControlPointsUsed())
            {
                aRetval = basegfx::tools::adaptiveSubdivideByAngle(aRetval);
            }
        }
        catch ( const com::sun::star::lang::IllegalArgumentException & )
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
        Reference< XCustomShapeEngine > xCustomShapeEngine( pCustomShape->GetCustomShapeEngine() );
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

static sal_Int32 GetNumberOfProperties ( const SvxMSDffHandle* pData )
{
    sal_Int32     nPropertiesNeeded=1;  // position is always needed
    sal_Int32     nFlags = pData->nFlags;

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

    return nPropertiesNeeded;
}

static void lcl_ShapePropertiesFromDFF( const SvxMSDffHandle* pData, com::sun::star::beans::PropertyValues& rPropValues )
{
    sal_Int32 nFlags = pData->nFlags, n=0;

    // POSITION
    {
        const OUString sPosition( "Position" );
        ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aPosition;
        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.First, pData->nPositionX, true, true );
        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.Second, pData->nPositionY, true, false );
        rPropValues[ n ].Name = sPosition;
        rPropValues[ n++ ].Value <<= aPosition;
    }
    if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
    {
        const OUString sMirroredX( "MirroredX" );
        sal_Bool bMirroredX = sal_True;
        rPropValues[ n ].Name = sMirroredX;
        rPropValues[ n++ ].Value <<= bMirroredX;
    }
    if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
    {
        const OUString sMirroredY( "MirroredY" );
        sal_Bool bMirroredY = sal_True;
        rPropValues[ n ].Name = sMirroredY;
        rPropValues[ n++ ].Value <<= bMirroredY;
    }
    if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
    {
        const OUString sSwitched( "Switched" );
        sal_Bool bSwitched = sal_True;
        rPropValues[ n ].Name = sSwitched;
        rPropValues[ n++ ].Value <<= bSwitched;
    }
    if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
    {
        const OUString sPolar( "Polar" );
        ::com::sun::star::drawing::EnhancedCustomShapeParameterPair aCenter;
        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aCenter.First, pData->nCenterX,
                           ( nFlags & MSDFF_HANDLE_FLAGS_CENTER_X_IS_SPECIAL ) != 0, true  );
        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aCenter.Second, pData->nCenterY,
                           ( nFlags & MSDFF_HANDLE_FLAGS_CENTER_Y_IS_SPECIAL ) != 0, false );
        rPropValues[ n ].Name = sPolar;
        rPropValues[ n++ ].Value <<= aCenter;
        if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
        {
            if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
            {
                const OUString sRadiusRangeMinimum( "RadiusRangeMinimum" );
                ::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMinimum, pData->nRangeXMin,
                           ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, true  );
                rPropValues[ n ].Name = sRadiusRangeMinimum;
                rPropValues[ n++ ].Value <<= aRadiusRangeMinimum;
            }
            if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
            {
                const OUString sRadiusRangeMaximum( "RadiusRangeMaximum" );
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
            const OUString sRangeXMinimum( "RangeXMinimum" );
            ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMinimum;
            EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMinimum, pData->nRangeXMin,
                           ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, true  );
            rPropValues[ n ].Name = sRangeXMinimum;
            rPropValues[ n++ ].Value <<= aRangeXMinimum;
        }
        if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
        {
            const OUString sRangeXMaximum( "RangeXMaximum" );
            ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMaximum;
            EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMaximum, pData->nRangeXMax,
                           ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, false );
            rPropValues[ n ].Name = sRangeXMaximum;
            rPropValues[ n++ ].Value <<= aRangeXMaximum;
        }
        if ( pData->nRangeYMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
        {
            const OUString sRangeYMinimum( "RangeYMinimum" );
            ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMinimum;
            EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMinimum, pData->nRangeYMin,
                             ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MIN_IS_SPECIAL ) != 0, true );
            rPropValues[ n ].Name = sRangeYMinimum;
            rPropValues[ n++ ].Value <<= aRangeYMinimum;
        }
        if ( pData->nRangeYMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
        {
            const OUString sRangeYMaximum( "RangeYMaximum" );
            ::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMaximum;
            EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMaximum, pData->nRangeYMax,
                             ( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MAX_IS_SPECIAL ) != 0, false );
            rPropValues[ n ].Name = sRangeYMaximum;
            rPropValues[ n++ ].Value <<= aRangeYMaximum;
        }
    }
    return;
}

sdr::properties::BaseProperties* SdrObjCustomShape::CreateObjectSpecificProperties()
{
    return new sdr::properties::CustomShapeProperties(*this);
}

TYPEINIT1(SdrObjCustomShape,SdrTextObj);
SdrObjCustomShape::SdrObjCustomShape() :
    SdrTextObj(),
    fObjectRotation( 0.0 ),
    mpLastShadowGeometry(0L)
{
    bClosedObj = true; // custom shapes may be filled
    bTextFrame = sal_True;
}

SdrObjCustomShape::~SdrObjCustomShape()
{
    // delete buffered display geometry
    InvalidateRenderGeometry();
}

void SdrObjCustomShape::MergeDefaultAttributes( const OUString* pType )
{
    PropertyValue aPropVal;
    OUString sShapeType;
    const OUString sType( "Type" );
    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    if ( pType && !pType->isEmpty() )
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
    const OUString sAdjustmentValues( "AdjustmentValues" );
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
                seqAdjustmentValues[ i ].State = com::sun::star::beans::PropertyState_DIRECT_VALUE;
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
    const OUString sViewBox( "ViewBox" );
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

    const OUString sPath( "Path" );

    //////////////////////
    // Path/Coordinates //
    //////////////////////
    const OUString sCoordinates( "Coordinates" );
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

    // Path/GluePoints //
    const OUString sGluePoints( "GluePoints" );
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

    // Path/Segments //
    const OUString sSegments( "Segments" );
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
            lcl_ShapeSegmentFromBinary( rSegInfo, nSDat );
        }
        aPropVal.Name = sSegments;
        aPropVal.Value <<= seqSegments;
        aGeometryItem.SetPropertyValue( sPath, aPropVal );
    }

    // Path/StretchX //
    const OUString sStretchX( "StretchX" );
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

    // Path/StretchY //
    const OUString sStretchY( "StretchY" );
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

    // Path/TextFrames //
    const OUString sTextFrames( "TextFrames" );
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

    // Equations //
    const OUString sEquations(  "Equations"  );
    pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sEquations );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nCalculation && pDefCustomShape->pCalculation )
    {
        com::sun::star::uno::Sequence< OUString > seqEquations;

        sal_Int32 i, nCount = pDefCustomShape->nCalculation;
        seqEquations.realloc( nCount );
        const SvxMSDffCalculationData* pData = pDefCustomShape->pCalculation;
        for ( i = 0; i < nCount; i++, pData++ )
            seqEquations[ i ] = EnhancedCustomShape2d::GetEquation( pData->nFlags, pData->nVal[ 0 ], pData->nVal[ 1 ], pData->nVal[ 2 ] );
        aPropVal.Name = sEquations;
        aPropVal.Value <<= seqEquations;
        aGeometryItem.SetPropertyValue( aPropVal );
    }

    // Handles //
    const OUString sHandles(  "Handles"  );
    pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sHandles );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nHandles && pDefCustomShape->pHandles )
    {
        com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValues > seqHandles;

        sal_Int32 i, nCount = pDefCustomShape->nHandles;
        const SvxMSDffHandle* pData = pDefCustomShape->pHandles;
        seqHandles.realloc( nCount );
        for ( i = 0; i < nCount; i++, pData++ )
        {
            sal_Int32 nPropertiesNeeded;
            com::sun::star::beans::PropertyValues& rPropValues = seqHandles[ i ];
            nPropertiesNeeded = GetNumberOfProperties( pData );
            rPropValues.realloc( nPropertiesNeeded );
            lcl_ShapePropertiesFromDFF( pData, rPropValues );
        }
        aPropVal.Name = sHandles;
        aPropVal.Value <<= seqHandles;
        aGeometryItem.SetPropertyValue( aPropVal );
    }
    SetMergedItem( aGeometryItem );
}

sal_Bool SdrObjCustomShape::IsDefaultGeometry( const DefaultType eDefaultType ) const
{
    sal_Bool bIsDefaultGeometry = sal_False;

    PropertyValue aPropVal;
    OUString sShapeType;
    const OUString sType( "Type" );
    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

    Any *pAny = aGeometryItem.GetPropertyValueByName( sType );
    if ( pAny )
        *pAny >>= sShapeType;

    MSO_SPT eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

    const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eSpType );
    const OUString sPath( "Path" );
    switch( eDefaultType )
    {
        case DEFAULT_VIEWBOX :
        {
            const OUString sViewBox( "ViewBox" );
            const Any* pViewBox = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sViewBox );
            com::sun::star::awt::Rectangle aViewBox;
            if ( pViewBox && ( *pViewBox >>= aViewBox ) )
            {
                if ( ( aViewBox.Width == pDefCustomShape->nCoordWidth )
                    && ( aViewBox.Height == pDefCustomShape->nCoordHeight ) )
                    bIsDefaultGeometry = sal_True;
            }
        }
        break;

        case DEFAULT_PATH :
        {
            const OUString sCoordinates( "Coordinates" );
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
                        bIsDefaultGeometry = sal_True;
                }
            }
            else if ( pDefCustomShape && ( ( pDefCustomShape->nVertices == 0 ) || ( pDefCustomShape->pVertices == 0 ) ) )
                bIsDefaultGeometry = sal_True;
        }
        break;

        case DEFAULT_GLUEPOINTS :
        {
            const OUString sGluePoints( "GluePoints" );
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
                        bIsDefaultGeometry = sal_True;
                }
            }
            else if ( pDefCustomShape && ( pDefCustomShape->nGluePoints == 0 ) )
                bIsDefaultGeometry = sal_True;
        }
        break;

        case DEFAULT_SEGMENTS :
        {
            // Path/Segments //
            const OUString sSegments( "Segments" );
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
                                lcl_ShapeSegmentFromBinary( rSegInfo, nSDat );
                            }
                            if ( seqSegments1 == seqSegments2 )
                                bIsDefaultGeometry = sal_True;
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
                                bIsDefaultGeometry = sal_True;
                        }
                    }
                }
            }
            else if ( pDefCustomShape && ( ( pDefCustomShape->nElements == 0 ) || ( pDefCustomShape->pElements == 0 ) ) )
                bIsDefaultGeometry = sal_True;
        }
        break;

        case DEFAULT_STRETCHX :
        {
            const OUString sStretchX( "StretchX" );
            pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sStretchX );
            if ( pAny && pDefCustomShape )
            {
                sal_Int32 nStretchX = 0;
                if ( *pAny >>= nStretchX )
                {
                    if ( pDefCustomShape->nXRef == nStretchX )
                        bIsDefaultGeometry = sal_True;
                }
            }
            else if ( pDefCustomShape && ( pDefCustomShape->nXRef == DEFAULT_MINIMUM_SIGNED_COMPARE ) )
                bIsDefaultGeometry = sal_True;
        }
        break;

        case DEFAULT_STRETCHY :
        {
            const OUString sStretchY( "StretchY" );
            pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sStretchY );
            if ( pAny && pDefCustomShape )
            {
                sal_Int32 nStretchY = 0;
                if ( *pAny >>= nStretchY )
                {
                    if ( pDefCustomShape->nYRef == nStretchY )
                        bIsDefaultGeometry = sal_True;
                }
            }
            else if ( pDefCustomShape && ( pDefCustomShape->nYRef == DEFAULT_MINIMUM_SIGNED_COMPARE ) )
                bIsDefaultGeometry = sal_True;
        }
        break;

        case DEFAULT_EQUATIONS :
        {
            const OUString sEquations(  "Equations"  );
            pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sEquations );
            if ( pAny && pDefCustomShape && pDefCustomShape->nCalculation && pDefCustomShape->pCalculation )
            {
                com::sun::star::uno::Sequence< OUString > seqEquations1, seqEquations2;
                if ( *pAny >>= seqEquations1 )
                {
                    sal_Int32 i, nCount = pDefCustomShape->nCalculation;
                    seqEquations2.realloc( nCount );

                    const SvxMSDffCalculationData* pData = pDefCustomShape->pCalculation;
                    for ( i = 0; i < nCount; i++, pData++ )
                        seqEquations2[ i ] = EnhancedCustomShape2d::GetEquation( pData->nFlags, pData->nVal[ 0 ], pData->nVal[ 1 ], pData->nVal[ 2 ] );

                    if ( seqEquations1 == seqEquations2 )
                        bIsDefaultGeometry = sal_True;
                }
            }
            else if ( pDefCustomShape && ( ( pDefCustomShape->nCalculation == 0 ) || ( pDefCustomShape->pCalculation == 0 ) ) )
                bIsDefaultGeometry = sal_True;
        }
        break;

        case DEFAULT_TEXTFRAMES :
        {
            const OUString sTextFrames(  "TextFrames"  );
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
                        bIsDefaultGeometry = sal_True;
                }
            }
            else if ( pDefCustomShape && ( ( pDefCustomShape->nTextRect == 0 ) || ( pDefCustomShape->pTextRect == 0 ) ) )
                bIsDefaultGeometry = sal_True;
        }
        break;

        case DEFAULT_HANDLES :
        {
            const OUString sHandles(  "Handles"  );
            pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sHandles );
            if ( pAny && pDefCustomShape && pDefCustomShape->nHandles && pDefCustomShape->pHandles )
            {
                com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValues > seqHandles1, seqHandles2;
                if ( *pAny >>= seqHandles1 )
                {
                    sal_Int32 i, nCount = pDefCustomShape->nHandles;
                    const SvxMSDffHandle* pData = pDefCustomShape->pHandles;
                    seqHandles2.realloc( nCount );
                    for ( i = 0; i < nCount; i++, pData++ )
                    {
                        sal_Int32 nPropertiesNeeded;
                        com::sun::star::beans::PropertyValues& rPropValues = seqHandles2[ i ];
                        nPropertiesNeeded = GetNumberOfProperties( pData );
                        rPropValues.realloc( nPropertiesNeeded );
                        lcl_ShapePropertiesFromDFF( pData, rPropValues );
                    }
                    if ( seqHandles1 == seqHandles2 )
                        bIsDefaultGeometry = sal_True;
                }
            }
            else if ( pDefCustomShape && ( ( pDefCustomShape->nHandles == 0 ) || ( pDefCustomShape->pHandles == 0 ) ) )
                bIsDefaultGeometry = sal_True;
        }
        break;
    }
    return bIsDefaultGeometry;
}

void SdrObjCustomShape::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bResizeFreeAllowed=fObjectRotation == 0.0;
    rInfo.bResizePropAllowed=sal_True;
    rInfo.bRotateFreeAllowed=sal_True;
    rInfo.bRotate90Allowed  =sal_True;
    rInfo.bMirrorFreeAllowed=sal_True;
    rInfo.bMirror45Allowed  =sal_True;
    rInfo.bMirror90Allowed  =sal_True;
    rInfo.bTransparenceAllowed = sal_False;
    rInfo.bGradientAllowed = sal_False;
    rInfo.bShearAllowed     =sal_True;
    rInfo.bEdgeRadiusAllowed=sal_False;
    rInfo.bNoContortion     =sal_True;

    // #i37011#
    if ( mXRenderedCustomShape.is() )
    {
        const SdrObject* pRenderedCustomShape = GetSdrObjectFromXShape( mXRenderedCustomShape );
        if ( pRenderedCustomShape )
        {
            // #i37262#
            // Iterate self over the contained objects, since there are combinations of
            // polygon and curve objects. In that case, aInfo.bCanConvToPath and
            // aInfo.bCanConvToPoly would be false. What is needed here is an or, not an and.
            SdrObjListIter aIterator(*pRenderedCustomShape);
            while(aIterator.IsMore())
            {
                SdrObject* pCandidate = aIterator.Next();
                SdrObjTransformInfoRec aInfo;
                pCandidate->TakeObjInfo(aInfo);

                // set path and poly conversion if one is possible since
                // this object will first be broken
                const bool bCanConvToPathOrPoly(aInfo.bCanConvToPath || aInfo.bCanConvToPoly);
                if(rInfo.bCanConvToPath != bCanConvToPathOrPoly)
                {
                    rInfo.bCanConvToPath = bCanConvToPathOrPoly;
                }

                if(rInfo.bCanConvToPoly != bCanConvToPathOrPoly)
                {
                    rInfo.bCanConvToPoly = bCanConvToPathOrPoly;
                }

                if(rInfo.bCanConvToContour != aInfo.bCanConvToContour)
                {
                    rInfo.bCanConvToContour = aInfo.bCanConvToContour;
                }

                if(rInfo.bShearAllowed != aInfo.bShearAllowed)
                {
                    rInfo.bShearAllowed = aInfo.bShearAllowed;
                }
            }
        }
    }
}

void SdrObjCustomShape::SetModel(SdrModel* pNewModel)
{
    SdrTextObj::SetModel(pNewModel);
    mXRenderedCustomShape.clear();
}

sal_uInt16 SdrObjCustomShape::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_CUSTOMSHAPE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjCustomShape::RecalcSnapRect()
{
    SdrTextObj::RecalcSnapRect();
}
const Rectangle& SdrObjCustomShape::GetSnapRect() const
{
    return SdrTextObj::GetSnapRect();
}
const Rectangle& SdrObjCustomShape::GetCurrentBoundRect() const
{
    return SdrTextObj::GetCurrentBoundRect();
}
const Rectangle& SdrObjCustomShape::GetLogicRect() const
{
    return SdrTextObj::GetLogicRect();
}
void SdrObjCustomShape::NbcSetSnapRect( const Rectangle& rRect )
{
    aRect=rRect;
    ImpJustifyRect(aRect);
    InvalidateRenderGeometry();
    Rectangle aTextBound( aRect );
    if ( GetTextBounds( aTextBound ) )
    {
        if ( pModel==NULL || !pModel->IsPasteResize() )
        {
            long nHDist=GetTextLeftDistance()+GetTextRightDistance();
            long nVDist=GetTextUpperDistance()+GetTextLowerDistance();
            long nTWdt=aTextBound.GetWidth ()-1-nHDist; if (nTWdt<0) nTWdt=0;
            long nTHgt=aTextBound.GetHeight()-1-nVDist; if (nTHgt<0) nTHgt=0;
            if ( IsAutoGrowWidth() )
                NbcSetMinTextFrameWidth( nTWdt );
            if ( IsAutoGrowHeight() )
                NbcSetMinTextFrameHeight( nTHgt );
            NbcAdjustTextFrameWidthAndHeight();
        }
    }
    ImpCheckShear();
    SetRectsDirty();
    SetChanged();
}
void SdrObjCustomShape::SetSnapRect( const Rectangle& rRect )
{
    Rectangle aBoundRect0;
    if ( pUserCall )
        aBoundRect0 = GetLastBoundRect();
    NbcSetSnapRect( rRect );
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}
void SdrObjCustomShape::NbcSetLogicRect( const Rectangle& rRect )
{
    aRect = rRect;
    ImpJustifyRect( aRect );
    InvalidateRenderGeometry();
    Rectangle aTextBound( aRect );
    if ( GetTextBounds( aTextBound ) )
    {
        long nHDist=GetTextLeftDistance()+GetTextRightDistance();
        long nVDist=GetTextUpperDistance()+GetTextLowerDistance();

        long nTWdt=aTextBound.GetWidth()-1-nHDist; if (nTWdt<0) nTWdt=0;
        long nTHgt=aTextBound.GetHeight()-1-nVDist; if (nTHgt<0) nTHgt=0;
        if ( IsAutoGrowWidth() )
            NbcSetMinTextFrameWidth( nTWdt );
        if ( IsAutoGrowHeight() )
            NbcSetMinTextFrameHeight( nTHgt );
        NbcAdjustTextFrameWidthAndHeight();
    }
    SetRectsDirty();
    SetChanged();
}
void SdrObjCustomShape::SetLogicRect( const Rectangle& rRect )
{
    Rectangle aBoundRect0;
    if ( pUserCall )
        aBoundRect0 = GetLastBoundRect();
    NbcSetLogicRect(rRect);
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}
void SdrObjCustomShape::Move( const Size& rSiz )
{
    if ( rSiz.Width() || rSiz.Height() )
    {
        Rectangle aBoundRect0;
        if ( pUserCall )
            aBoundRect0 = GetLastBoundRect();
        NbcMove(rSiz);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
    }
}
void SdrObjCustomShape::NbcMove( const Size& rSiz )
{
    SdrTextObj::NbcMove( rSiz );
    if ( mXRenderedCustomShape.is() )
    {
        SdrObject* pRenderedCustomShape = GetSdrObjectFromXShape( mXRenderedCustomShape );
        if ( pRenderedCustomShape )
        {
            // #i97149# the visualisation shape needs to be informed
            // about change, too
            pRenderedCustomShape->ActionChanged();
            pRenderedCustomShape->NbcMove( rSiz );
        }
    }

    // #i37011# adapt geometry shadow
    if(mpLastShadowGeometry)
    {
        mpLastShadowGeometry->NbcMove( rSiz );
    }
}
void SdrObjCustomShape::Resize( const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative )
{
    SdrTextObj::Resize( rRef, xFact, yFact, bUnsetRelative );
}

void SdrObjCustomShape::NbcResize( const Point& rRef, const Fraction& rxFact, const Fraction& ryFact )
{
    Fraction xFact( rxFact );
    Fraction yFact( ryFact );

    // taking care of handles that should not been changed
    Rectangle aOld( aRect );
    std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( this ) );

    SdrTextObj::NbcResize( rRef, xFact, yFact );

    if ( ( xFact.GetNumerator() != xFact.GetDenominator() )
        || ( yFact.GetNumerator()!= yFact.GetDenominator() ) )
    {
        if ( ( ( xFact.GetNumerator() < 0 ) && ( xFact.GetDenominator() > 0 ) ) ||
            ( ( xFact.GetNumerator() > 0 ) && ( xFact.GetDenominator() < 0 ) ) )
        {
            SetMirroredX( IsMirroredX() == sal_False );
        }
        if ( ( ( yFact.GetNumerator() < 0 ) && ( yFact.GetDenominator() > 0 ) ) ||
            ( ( yFact.GetNumerator() > 0 ) && ( yFact.GetDenominator() < 0 ) ) )
        {
            SetMirroredY( IsMirroredY() == sal_False );
        }
    }

    for (std::vector< SdrCustomShapeInteraction >::const_iterator aIter( aInteractionHandles.begin() ), aEnd( aInteractionHandles.end() );
         aIter != aEnd; ++aIter )
    {
        try
        {
            if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_FIXED )
                aIter->xInteraction->setControllerPosition( aIter->aPosition );
            if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_X )
            {
                sal_Int32 nX = ( aIter->aPosition.X - aOld.Left() ) + aRect.Left();
                aIter->xInteraction->setControllerPosition( com::sun::star::awt::Point( nX, aIter->xInteraction->getPosition().Y ) );
            }
            if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_Y )
            {
                sal_Int32 nY = ( aIter->aPosition.Y - aOld.Top() ) + aRect.Top();
                aIter->xInteraction->setControllerPosition( com::sun::star::awt::Point( aIter->xInteraction->getPosition().X, nY ) );
            }
        }
        catch ( const uno::RuntimeException& )
        {
        }
    }
    InvalidateRenderGeometry();
}
void SdrObjCustomShape::NbcRotate( const Point& rRef, long nWink, double sn, double cs )
{
    sal_Bool bMirroredX = IsMirroredX();
    sal_Bool bMirroredY = IsMirroredY();

    fObjectRotation = fmod( fObjectRotation, 360.0 );
    if ( fObjectRotation < 0 )
        fObjectRotation = 360 + fObjectRotation;

    // the rotation angle for ashapes is stored in fObjectRotation, this rotation
    // has to be applied to the text object (which is internally using aGeo.nWink).
    SdrTextObj::NbcRotate( aRect.TopLeft(), -aGeo.nDrehWink,        // retrieving the unrotated text object
                            sin( (-aGeo.nDrehWink) * F_PI18000 ),
                            cos( (-aGeo.nDrehWink) * F_PI18000 ) );
    aGeo.nDrehWink = 0;                                             // resetting aGeo data
    aGeo.RecalcSinCos();

    long nW = (long)( fObjectRotation * 100 );                      // applying our object rotation
    if ( bMirroredX )
        nW = 36000 - nW;
    if ( bMirroredY )
        nW = 18000 - nW;
    nW = nW % 36000;
    if ( nW < 0 )
        nW = 36000 + nW;
    SdrTextObj::NbcRotate( aRect.TopLeft(), nW,                     // applying text rotation
                            sin( nW * F_PI18000 ),
                            cos( nW * F_PI18000 ) );

    int nSwap = 0;
    if ( bMirroredX )
        nSwap ^= 1;
    if ( bMirroredY )
        nSwap ^= 1;

    double fWink = nWink;                                                   // updating to our new object rotation
    fWink /= 100.0;
    fObjectRotation = fmod( nSwap ? fObjectRotation - fWink : fObjectRotation + fWink, 360.0 );
    if ( fObjectRotation < 0 )
        fObjectRotation = 360 + fObjectRotation;

    SdrTextObj::NbcRotate( rRef, nWink, sn, cs );                           // applying text rotation
    InvalidateRenderGeometry();
}

void SdrObjCustomShape::NbcMirror( const Point& rRef1, const Point& rRef2 )
{
    // TTTT: Fix for old mirroring, can be removed again in aw080
    // storing horizontal and vertical flipping without modifying the rotate angle
    // decompose other flipping to rotation and MirrorX.
    long ndx = rRef2.X()-rRef1.X();
    long ndy = rRef2.Y()-rRef1.Y();

    if(!ndx) // MirroredX
    {
         SetMirroredX(!IsMirroredX());
         SdrTextObj::NbcMirror( rRef1, rRef2 );
    }
    else
    {
        if(!ndy)  // MirroredY
        {
            SetMirroredY(!IsMirroredY());
            SdrTextObj::NbcMirror( rRef1, rRef2 );
        }
        else // neither horizontal nor vertical
        {
            SetMirroredX(!IsMirroredX());

            // call parent
            SdrTextObj::NbcMirror( rRef1, rRef2 );

            // update fObjectRotation
            long nTextObjRotation = aGeo.nDrehWink;
            double fWink = nTextObjRotation;

            fWink /= 100.0;

            bool bSingleFlip = (IsMirroredX()!= IsMirroredY());

            fObjectRotation = fmod( bSingleFlip ? -fWink : fWink, 360.0 );

            if ( fObjectRotation < 0 )
            {
                fObjectRotation = 360.0 + fObjectRotation;
            }
         }
    }

    InvalidateRenderGeometry();
}

void SdrObjCustomShape::Shear( const Point& rRef, long nWink, double tn, bool bVShear )
{
    SdrTextObj::Shear( rRef, nWink, tn, bVShear );
    InvalidateRenderGeometry();
}
void SdrObjCustomShape::NbcShear( const Point& rRef, long nWink, double tn, bool bVShear )
{
    // TTTT: Fix for old mirroring, can be removed again in aw080
    SdrTextObj::NbcShear(rRef,nWink,tn,bVShear);

    // updating fObjectRotation
    long nTextObjRotation = aGeo.nDrehWink;
    double fWink = nTextObjRotation;

    fWink /= 100.0;

    bool bSingleFlip = (IsMirroredX()!= IsMirroredY());

    fObjectRotation = fmod( bSingleFlip ? -fWink : fWink, 360.0 );

    if ( fObjectRotation < 0 )
    {
        fObjectRotation = 360.0 + fObjectRotation;
    }

    InvalidateRenderGeometry();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrGluePoint SdrObjCustomShape::GetVertexGluePoint(sal_uInt16 nPosNum) const
{
    sal_Int32 nWdt = ImpGetLineWdt(); // #i25616#

    // #i25616#
    if(!LineIsOutsideGeometry())
    {
        nWdt++;
        nWdt /= 2;
    }

    Point aPt;
    switch (nPosNum) {
        case 0: aPt=aRect.TopCenter();    aPt.Y()-=nWdt; break;
        case 1: aPt=aRect.RightCenter();  aPt.X()+=nWdt; break;
        case 2: aPt=aRect.BottomCenter(); aPt.Y()+=nWdt; break;
        case 3: aPt=aRect.LeftCenter();   aPt.X()-=nWdt; break;
    }
    if (aGeo.nShearWink!=0) ShearPoint(aPt,aRect.TopLeft(),aGeo.nTan);
    if (aGeo.nDrehWink!=0) RotatePoint(aPt,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    aPt-=GetSnapRect().Center();
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(sal_False);
    return aGP;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// #i38892#
void SdrObjCustomShape::ImpCheckCustomGluePointsAreAdded()
{
    const SdrObject* pSdrObject = GetSdrObjectFromCustomShape();

    if(pSdrObject)
    {
        const SdrGluePointList* pSource = pSdrObject->GetGluePointList();

        if(pSource && pSource->GetCount())
        {
            if(!SdrTextObj::GetGluePointList())
            {
                SdrTextObj::ForceGluePointList();
            }

            const SdrGluePointList* pList = SdrTextObj::GetGluePointList();

            if(pList)
            {
                SdrGluePointList aNewList;
                sal_uInt16 a;

                for(a = 0; a < pSource->GetCount(); a++)
                {
                    SdrGluePoint aCopy((*pSource)[a]);
                    aCopy.SetUserDefined(sal_False);
                    aNewList.Insert(aCopy);
                }

                sal_Bool bMirroredX = IsMirroredX();
                sal_Bool bMirroredY = IsMirroredY();

                long nShearWink = aGeo.nShearWink;
                double fTan = aGeo.nTan;

                if ( aGeo.nDrehWink || nShearWink || bMirroredX || bMirroredY )
                {
                    Polygon aPoly( aRect );
                    if( nShearWink )
                    {
                        sal_uInt16 nPointCount=aPoly.GetSize();
                        for (sal_uInt16 i=0; i<nPointCount; i++)
                            ShearPoint(aPoly[i],aRect.Center(), fTan, sal_False );
                    }
                    if ( aGeo.nDrehWink )
                        aPoly.Rotate( aRect.Center(), aGeo.nDrehWink / 10 );

                    Rectangle aBoundRect( aPoly.GetBoundRect() );
                    sal_Int32 nXDiff = aBoundRect.Left() - aRect.Left();
                    sal_Int32 nYDiff = aBoundRect.Top() - aRect.Top();

                    if (nShearWink&&((bMirroredX&&!bMirroredY)||(bMirroredY&&!bMirroredX)))
                    {
                        nShearWink = -nShearWink;
                        fTan = -fTan;
                    }

                    Point aRef( aRect.GetWidth() / 2, aRect.GetHeight() / 2 );
                    for ( a = 0; a < aNewList.GetCount(); a++ )
                    {
                        SdrGluePoint& rPoint = aNewList[ a ];
                        Point aGlue( rPoint.GetPos() );
                        if ( nShearWink )
                            ShearPoint( aGlue, aRef, fTan );

                        RotatePoint( aGlue, aRef, sin( fObjectRotation * F_PI180 ), cos( fObjectRotation * F_PI180 ) );
                        if ( bMirroredX )
                            aGlue.X() = aRect.GetWidth() - aGlue.X();
                        if ( bMirroredY )
                            aGlue.Y() = aRect.GetHeight() - aGlue.Y();
                        aGlue.X() -= nXDiff;
                        aGlue.Y() -= nYDiff;
                        rPoint.SetPos( aGlue );
                    }
                }

                for(a = 0; a < pList->GetCount(); a++)
                {
                    const SdrGluePoint& rCandidate = (*pList)[a];

                    if(rCandidate.IsUserDefined())
                    {
                        aNewList.Insert(rCandidate);
                    }
                }

                // copy new list to local. This is NOT very convenient behavior, the local
                // GluePointList should not be set, but we delivered by using GetGluePointList(),
                // maybe on demand. Since the local object is changed here, this is assumed to
                // be a result of GetGluePointList and thus the list is copied
                if(pPlusData)
                {
                    pPlusData->SetGluePoints(aNewList);
                }
            }
        }
    }
}

// #i38892#
const SdrGluePointList* SdrObjCustomShape::GetGluePointList() const
{
    ((SdrObjCustomShape*)this)->ImpCheckCustomGluePointsAreAdded();
    return SdrTextObj::GetGluePointList();
}

// #i38892#
SdrGluePointList* SdrObjCustomShape::ForceGluePointList()
{
    if(SdrTextObj::ForceGluePointList())
    {
        ImpCheckCustomGluePointsAreAdded();
        return SdrTextObj::ForceGluePointList();
    }
    else
    {
        return 0L;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_uInt32 SdrObjCustomShape::GetHdlCount() const
{
    const sal_uInt32 nBasicHdlCount(SdrTextObj::GetHdlCount());
    std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( this ) );
    return ( aInteractionHandles.size() + nBasicHdlCount );
}

SdrHdl* SdrObjCustomShape::GetHdl( sal_uInt32 nHdlNum ) const
{
    SdrHdl* pH = NULL;
    const sal_uInt32 nBasicHdlCount(SdrTextObj::GetHdlCount());

    if ( nHdlNum < nBasicHdlCount )
        pH = SdrTextObj::GetHdl( nHdlNum );
    else
    {
        std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( this ) );
        const sal_uInt32 nCustomShapeHdlNum(nHdlNum - nBasicHdlCount);

        if ( nCustomShapeHdlNum < aInteractionHandles.size() )
        {
            if ( aInteractionHandles[ nCustomShapeHdlNum ].xInteraction.is() )
            {
                try
                {
                    com::sun::star::awt::Point aPosition( aInteractionHandles[ nCustomShapeHdlNum ].xInteraction->getPosition() );
                    pH = new SdrHdl( Point( aPosition.X, aPosition.Y ), HDL_CUSTOMSHAPE1 );
                    pH->SetPointNum( nCustomShapeHdlNum );
                    pH->SetObj( (SdrObject*)this );
                }
                catch ( const uno::RuntimeException& )
                {
                }
            }
        }
    }
    return pH;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrObjCustomShape::hasSpecialDrag() const
{
    return true;
}

bool SdrObjCustomShape::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl = rDrag.GetHdl();

    if(pHdl && HDL_CUSTOMSHAPE1 == pHdl->GetKind())
    {
        rDrag.SetEndDragChangesAttributes(true);
        rDrag.SetNoSnap(true);
    }
    else
    {
        const SdrHdl* pHdl2 = rDrag.GetHdl();
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

void SdrObjCustomShape::DragResizeCustomShape( const Rectangle& rNewRect, SdrObjCustomShape* pObj ) const
{
    Rectangle   aOld( pObj->aRect );
    sal_Bool    bOldMirroredX( pObj->IsMirroredX() );
    sal_Bool    bOldMirroredY( pObj->IsMirroredY() );

    Rectangle aNewRect( rNewRect );
    aNewRect.Justify();

    std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( pObj ) );

    GeoStat aGeoStat( pObj->GetGeoStat() );
    if ( aNewRect.TopLeft()!= pObj->aRect.TopLeft() &&
        ( pObj->aGeo.nDrehWink || pObj->aGeo.nShearWink ) )
    {
        Point aNewPos( aNewRect.TopLeft() );
        if ( pObj->aGeo.nShearWink ) ShearPoint( aNewPos, aOld.TopLeft(), aGeoStat.nTan );
        if ( pObj->aGeo.nDrehWink )  RotatePoint(aNewPos, aOld.TopLeft(), aGeoStat.nSin, aGeoStat.nCos );
        aNewRect.SetPos( aNewPos );
    }
    if ( aNewRect != pObj->aRect )
    {
        pObj->SetLogicRect( aNewRect );
        pObj->InvalidateRenderGeometry();

        if ( rNewRect.Left() > rNewRect.Right() )
        {
            Point aTop( ( pObj->GetSnapRect().Left() + pObj->GetSnapRect().Right() ) >> 1, pObj->GetSnapRect().Top() );
            Point aBottom( aTop.X(), aTop.Y() + 1000 );
            pObj->NbcMirror( aTop, aBottom );
        }
        if ( rNewRect.Top() > rNewRect.Bottom() )
        {
            Point aLeft( pObj->GetSnapRect().Left(), ( pObj->GetSnapRect().Top() + pObj->GetSnapRect().Bottom() ) >> 1 );
            Point aRight( aLeft.X() + 1000, aLeft.Y() );
            pObj->NbcMirror( aLeft, aRight );
        }

        for (std::vector< SdrCustomShapeInteraction >::const_iterator aIter( aInteractionHandles.begin() ), aEnd( aInteractionHandles.end() );
             aIter != aEnd ; ++aIter )
        {
            try
            {
                if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_FIXED )
                    aIter->xInteraction->setControllerPosition( aIter->aPosition );
                if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_X )
                {
                    sal_Int32 nX;
                    if ( bOldMirroredX )
                    {
                        nX = ( aIter->aPosition.X - aOld.Right() );
                        if ( rNewRect.Left() > rNewRect.Right() )
                            nX = pObj->aRect.Left() - nX;
                        else
                            nX += pObj->aRect.Right();
                    }
                    else
                    {
                        nX = ( aIter->aPosition.X - aOld.Left() );
                        if ( rNewRect.Left() > rNewRect.Right() )
                            nX = pObj->aRect.Right() - nX;
                        else
                            nX += pObj->aRect.Left();
                    }
                    aIter->xInteraction->setControllerPosition( com::sun::star::awt::Point( nX, aIter->xInteraction->getPosition().Y ) );
                }
                if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_Y )
                {
                    sal_Int32 nY;
                    if ( bOldMirroredY )
                    {
                        nY = ( aIter->aPosition.Y - aOld.Bottom() );
                        if ( rNewRect.Top() > rNewRect.Bottom() )
                            nY = pObj->aRect.Top() - nY;
                        else
                            nY += pObj->aRect.Bottom();
                    }
                    else
                    {
                        nY = ( aIter->aPosition.Y - aOld.Top() );
                        if ( rNewRect.Top() > rNewRect.Bottom() )
                            nY = pObj->aRect.Bottom() - nY;
                        else
                            nY += pObj->aRect.Top();
                    }
                    aIter->xInteraction->setControllerPosition( com::sun::star::awt::Point( aIter->xInteraction->getPosition().X, nY ) );
                }
            }
            catch ( const uno::RuntimeException& )
            {
            }
        }
    }
}

void SdrObjCustomShape::DragMoveCustomShapeHdl( const Point aDestination, const sal_uInt16 nCustomShapeHdlNum, SdrObjCustomShape* pObj ) const
{
    std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( pObj ) );
    if ( nCustomShapeHdlNum < aInteractionHandles.size() )
    {
        SdrCustomShapeInteraction aInteractionHandle( aInteractionHandles[ nCustomShapeHdlNum ] );
        if ( aInteractionHandle.xInteraction.is() )
        {
            try
            {
                com::sun::star::awt::Point aPt( aDestination.X(), aDestination.Y() );
                if ( aInteractionHandle.nMode & CUSTOMSHAPE_HANDLE_MOVE_SHAPE )
                {
                    sal_Int32 nXDiff = aPt.X - aInteractionHandle.aPosition.X;
                    sal_Int32 nYDiff = aPt.Y - aInteractionHandle.aPosition.Y;

                    pObj->aRect.Move( nXDiff, nYDiff );
                    pObj->aOutRect.Move( nXDiff, nYDiff );
                    pObj->maSnapRect.Move( nXDiff, nYDiff );
                    pObj->SetRectsDirty(sal_True);
                    pObj->InvalidateRenderGeometry();

                    for (std::vector< SdrCustomShapeInteraction >::const_iterator aIter( aInteractionHandles.begin() ), aEnd( aInteractionHandles.end() ) ;
                      aIter != aEnd; ++aIter)
                    {
                        if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_FIXED )
                        {
                            if ( aIter->xInteraction.is() )
                                aIter->xInteraction->setControllerPosition( aIter->aPosition );
                        }
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
    const SdrHdl* pHdl = rDrag.GetHdl();
    const SdrHdlKind eHdl((pHdl == NULL) ? HDL_MOVE : pHdl->GetKind());

    switch(eHdl)
    {
        case HDL_CUSTOMSHAPE1 :
        {
            rDrag.SetEndDragChangesGeoAndAttributes(true);
            DragMoveCustomShapeHdl( rDrag.GetNow(), (sal_uInt16)pHdl->GetPointNum(), this );
            SetRectsDirty();
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
            DragResizeCustomShape(ImpDragCalcRect(rDrag), this);
            break;
        }
        case HDL_MOVE :
        {
            Move(Size(rDrag.GetDX(), rDrag.GetDY()));
            break;
        }
        default: break;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjCustomShape::DragCreateObject( SdrDragStat& rStat )
{
    Rectangle aRect1;
    rStat.TakeCreateRect( aRect1 );

    std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( this ) );

    sal_uInt32 nDefaultObjectSizeWidth = 3000;      // default width from SDOptions ?
    sal_uInt32 nDefaultObjectSizeHeight= 3000;

    if ( ImpVerticalSwitch( *this ) )
    {
        SetMirroredX( aRect1.Left() > aRect1.Right() );

        aRect1 = Rectangle( rStat.GetNow(), Size( nDefaultObjectSizeWidth, nDefaultObjectSizeHeight ) );
        // subtracting the horizontal difference of the latest handle from shape position
        if ( !aInteractionHandles.empty() )
        {
            sal_Int32 nHandlePos = aInteractionHandles[ aInteractionHandles.size() - 1 ].xInteraction->getPosition().X;
            aRect1.Move( aRect.Left() - nHandlePos, 0 );
        }
    }
    ImpJustifyRect( aRect1 );
    rStat.SetActionRect( aRect1 );
    aRect = aRect1;
    SetRectsDirty();

    for (std::vector< SdrCustomShapeInteraction >::const_iterator aIter( aInteractionHandles.begin() ), aEnd( aInteractionHandles.end() );
        aIter != aEnd ; ++aIter)
    {
        try
        {
            if ( aIter->nMode & CUSTOMSHAPE_HANDLE_CREATE_FIXED )
                aIter->xInteraction->setControllerPosition( awt::Point( rStat.GetStart().X(), rStat.GetStart().Y() ) );
        }
        catch ( const uno::RuntimeException& )
        {
        }
    }

    SetBoundRectDirty();
    bSnapRectDirty=sal_True;
}

bool SdrObjCustomShape::BegCreate( SdrDragStat& rDrag )
{
    return SdrTextObj::BegCreate( rDrag );
}

bool SdrObjCustomShape::MovCreate(SdrDragStat& rStat)
{
    SdrView* pView = rStat.GetView();       // #i37448#
    if( pView && pView->IsSolidDragging() )
    {
        InvalidateRenderGeometry();
    }
    DragCreateObject( rStat );
    SetRectsDirty();
    return sal_True;
}

bool SdrObjCustomShape::EndCreate( SdrDragStat& rStat, SdrCreateCmd eCmd )
{
    DragCreateObject( rStat );

    if ( bTextFrame )
    {
        if ( IsAutoGrowHeight() )
        {
            // MinTextHeight
            long nHgt=aRect.GetHeight()-1;
            if (nHgt==1) nHgt=0;
            NbcSetMinTextFrameHeight( nHgt );
        }
        if ( IsAutoGrowWidth() )
        {
            // MinTextWidth
            long nWdt=aRect.GetWidth()-1;
            if (nWdt==1) nWdt=0;
            NbcSetMinTextFrameWidth( nWdt );
        }
        // re-calculate text frame
        NbcAdjustTextFrameWidthAndHeight();
    }
    SetRectsDirty();
    return ( eCmd == SDRCREATE_FORCEEND || rStat.GetPointAnz() >= 2 );
}

basegfx::B2DPolyPolygon SdrObjCustomShape::TakeCreatePoly(const SdrDragStat& /*rDrag*/) const
{
    return GetLineGeometry( this, sal_False );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// in context with the SdrObjCustomShape the SdrTextAutoGrowHeightItem == true -> Resize Shape to fit text,
//                                     the SdrTextAutoGrowWidthItem  == true -> Word wrap text in Shape
bool SdrObjCustomShape::IsAutoGrowHeight() const
{
    const SfxItemSet& rSet = GetMergedItemSet();
    bool bIsAutoGrowHeight = ((SdrTextAutoGrowHeightItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue();
    if ( bIsAutoGrowHeight && IsVerticalWriting() )
        bIsAutoGrowHeight = ((SdrTextWordWrapItem&)(rSet.Get(SDRATTR_TEXT_WORDWRAP))).GetValue() == sal_False;
    return bIsAutoGrowHeight;
}
bool SdrObjCustomShape::IsAutoGrowWidth() const
{
    const SfxItemSet& rSet = GetMergedItemSet();
    bool bIsAutoGrowWidth = ((SdrTextAutoGrowHeightItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue();
    if ( bIsAutoGrowWidth && !IsVerticalWriting() )
        bIsAutoGrowWidth = ((SdrTextWordWrapItem&)(rSet.Get(SDRATTR_TEXT_WORDWRAP))).GetValue() == sal_False;
    return bIsAutoGrowWidth;
}

/* The following method is identical to the SdrTextObj::SetVerticalWriting method, the only difference
   is that the SdrAutoGrowWidthItem and SdrAutoGrowHeightItem are not exchanged if the vertical writing
   mode has been changed */

void SdrObjCustomShape::SetVerticalWriting( sal_Bool bVertical )
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

            // Also exchange horizontal and vertical adjust items
            SdrTextHorzAdjust eHorz = ((SdrTextHorzAdjustItem&)(rSet.Get(SDRATTR_TEXT_HORZADJUST))).GetValue();
            SdrTextVertAdjust eVert = ((SdrTextVertAdjustItem&)(rSet.Get(SDRATTR_TEXT_VERTADJUST))).GetValue();

            // rescue object size
            Rectangle aObjectRect = GetSnapRect();

            // prepare ItemSet to set exchanged width and height items
            SfxItemSet aNewSet(*rSet.GetPool(),
                SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT,
                // Expanded item ranges to also support horizontal and vertical adjust.
                SDRATTR_TEXT_VERTADJUST, SDRATTR_TEXT_VERTADJUST,
                SDRATTR_TEXT_AUTOGROWWIDTH, SDRATTR_TEXT_HORZADJUST,
                0, 0);

            aNewSet.Put(rSet);

            // Exchange horizontal and vertical adjusts
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
            SetSnapRect(aObjectRect);
        }
    }
}
bool SdrObjCustomShape::AdjustTextFrameWidthAndHeight(Rectangle& rR, bool bHgt, bool bWdt) const
{
     if ( pModel && HasText() && !rR.IsEmpty() )
    {
        bool bWdtGrow=bWdt && IsAutoGrowWidth();
        bool bHgtGrow=bHgt && IsAutoGrowHeight();
        if ( bWdtGrow || bHgtGrow )
        {
            Rectangle aR0(rR);
            long nHgt=0,nMinHgt=0,nMaxHgt=0;
            long nWdt=0,nMinWdt=0,nMaxWdt=0;
            Size aSiz(rR.GetSize()); aSiz.Width()--; aSiz.Height()--;
            Size aMaxSiz(100000,100000);
            Size aTmpSiz(pModel->GetMaxObjSize());
            if (aTmpSiz.Width()!=0) aMaxSiz.Width()=aTmpSiz.Width();
            if (aTmpSiz.Height()!=0) aMaxSiz.Height()=aTmpSiz.Height();
            if (bWdtGrow)
            {
                nMinWdt=GetMinTextFrameWidth();
                nMaxWdt=GetMaxTextFrameWidth();
                if (nMaxWdt==0 || nMaxWdt>aMaxSiz.Width()) nMaxWdt=aMaxSiz.Width();
                if (nMinWdt<=0) nMinWdt=1;
                aSiz.Width()=nMaxWdt;
            }
            if (bHgtGrow)
            {
                nMinHgt=GetMinTextFrameHeight();
                nMaxHgt=GetMaxTextFrameHeight();
                if (nMaxHgt==0 || nMaxHgt>aMaxSiz.Height()) nMaxHgt=aMaxSiz.Height();
                if (nMinHgt<=0) nMinHgt=1;
                aSiz.Height()=nMaxHgt;
            }
            long nHDist=GetTextLeftDistance()+GetTextRightDistance();
            long nVDist=GetTextUpperDistance()+GetTextLowerDistance();
            aSiz.Width()-=nHDist;
            aSiz.Height()-=nVDist;
            if ( aSiz.Width() < 2 )
                aSiz.Width() = 2;   // minimum size=2
            if ( aSiz.Height() < 2 )
                aSiz.Height() = 2; // minimum size=2

            if(pEdtOutl)
            {
                pEdtOutl->SetMaxAutoPaperSize( aSiz );
                if (bWdtGrow)
                {
                    Size aSiz2(pEdtOutl->CalcTextSize());
                    nWdt=aSiz2.Width()+1; // a little more tolerance
                    if (bHgtGrow) nHgt=aSiz2.Height()+1; // a little more tolerance
                } else
                {
                    nHgt=pEdtOutl->GetTextHeight()+1; // a little more tolerance
                }
            }
            else
            {
                Outliner& rOutliner=ImpGetDrawOutliner();
                rOutliner.SetPaperSize(aSiz);
                rOutliner.SetUpdateMode(sal_True);
                // TODO: add the optimization with bPortionInfoChecked again.
                OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
                if( pOutlinerParaObject != NULL )
                {
                    rOutliner.SetText(*pOutlinerParaObject);
                    rOutliner.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
                }
                if ( bWdtGrow )
                {
                    Size aSiz2(rOutliner.CalcTextSize());
                    nWdt=aSiz2.Width()+1; // a little more tolerance
                    if ( bHgtGrow )
                        nHgt=aSiz2.Height()+1; // a little more tolerance
                }
                else
                    nHgt = rOutliner.GetTextHeight()+1; // a little more tolerance
                rOutliner.Clear();
            }
            if ( nWdt < nMinWdt )
                nWdt = nMinWdt;
            if ( nWdt > nMaxWdt )
                nWdt = nMaxWdt;
            nWdt += nHDist;
            if ( nWdt < 1 )
                nWdt = 1; // nHDist may also be negative
            if ( nHgt < nMinHgt )
                nHgt = nMinHgt;
            if ( nHgt > nMaxHgt )
                nHgt = nMaxHgt;
            nHgt+=nVDist;
            if ( nHgt < 1 )
                nHgt = 1; // nVDist may also be negative
            long nWdtGrow = nWdt-(rR.Right()-rR.Left());
            long nHgtGrow = nHgt-(rR.Bottom()-rR.Top());
            if ( nWdtGrow == 0 )
                bWdtGrow = sal_False;
            if ( nHgtGrow == 0 )
                bHgtGrow=sal_False;
            if ( bWdtGrow || bHgtGrow )
            {
                if ( bWdtGrow )
                {
                    SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
                    if ( eHAdj == SDRTEXTHORZADJUST_LEFT )
                        rR.Right()+=nWdtGrow;
                    else if ( eHAdj == SDRTEXTHORZADJUST_RIGHT )
                        rR.Left()-=nWdtGrow;
                    else
                    {
                        long nWdtGrow2=nWdtGrow/2;
                        rR.Left()-=nWdtGrow2;
                        rR.Right()=rR.Left()+nWdt;
                    }
                }
                if ( bHgtGrow )
                {
                    SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
                    if ( eVAdj == SDRTEXTVERTADJUST_TOP )
                        rR.Bottom()+=nHgtGrow;
                    else if ( eVAdj == SDRTEXTVERTADJUST_BOTTOM )
                        rR.Top()-=nHgtGrow;
                    else
                    {
                        long nHgtGrow2=nHgtGrow/2;
                        rR.Top()-=nHgtGrow2;
                        rR.Bottom()=rR.Top()+nHgt;
                    }
                }
                if ( aGeo.nDrehWink )
                {
                    Point aD1(rR.TopLeft());
                    aD1-=aR0.TopLeft();
                    Point aD2(aD1);
                    RotatePoint(aD2,Point(),aGeo.nSin,aGeo.nCos);
                    aD2-=aD1;
                    rR.Move(aD2.X(),aD2.Y());
                }
                return sal_True;
            }
        }
    }
    return sal_False;
}

Rectangle SdrObjCustomShape::ImpCalculateTextFrame( const bool bHgt, const bool bWdt )
{
    Rectangle aReturnValue;

    Rectangle aOldTextRect( aRect );        // <- initial text rectangle

    Rectangle aNewTextRect( aRect );        // <- new text rectangle returned from the custom shape renderer,
    GetTextBounds( aNewTextRect );          //    it depends to the current logical shape size

    Rectangle aAdjustedTextRect( aNewTextRect );                            // <- new text rectangle is being tested by AdjustTextFrameWidthAndHeight to ensure
    if ( AdjustTextFrameWidthAndHeight( aAdjustedTextRect, bHgt, bWdt ) )   //    that the new text rectangle is matching the current text size from the outliner
    {
        if ( ( aAdjustedTextRect != aNewTextRect ) && ( aOldTextRect != aAdjustedTextRect ) )
        {
            aReturnValue = aRect;
            double fXScale = (double)aOldTextRect.GetWidth() / (double)aNewTextRect.GetWidth();
            double fYScale = (double)aOldTextRect.GetHeight() / (double)aNewTextRect.GetHeight();
            double fRightDiff = (double)( aAdjustedTextRect.Right() - aNewTextRect.Right() ) * fXScale;
            double fLeftDiff  = (double)( aAdjustedTextRect.Left()  - aNewTextRect.Left()  ) * fXScale;
            double fTopDiff   = (double)( aAdjustedTextRect.Top()   - aNewTextRect.Top()   ) * fYScale;
            double fBottomDiff= (double)( aAdjustedTextRect.Bottom()- aNewTextRect.Bottom()) * fYScale;
            aReturnValue.Left() += (sal_Int32)fLeftDiff;
            aReturnValue.Right() += (sal_Int32)fRightDiff;
            aReturnValue.Top() += (sal_Int32)fTopDiff;
            aReturnValue.Bottom() += (sal_Int32)fBottomDiff;
        }
    }
    return aReturnValue;
}

bool SdrObjCustomShape::NbcAdjustTextFrameWidthAndHeight(bool bHgt, bool bWdt)
{
    Rectangle aNewTextRect = ImpCalculateTextFrame( bHgt, bWdt );
    bool bRet = !aNewTextRect.IsEmpty() && ( aNewTextRect != aRect );
    if ( bRet )
    {
        // taking care of handles that should not been changed
        std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( this ) );

        aRect = aNewTextRect;
        SetRectsDirty();
        SetChanged();

        for (std::vector< SdrCustomShapeInteraction >::const_iterator aIter( aInteractionHandles.begin() ), aEnd ( aInteractionHandles.end() );
            aIter != aEnd ; ++aIter)
        {
            try
            {
                if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_FIXED )
                    aIter->xInteraction->setControllerPosition( aIter->aPosition );
            }
            catch ( const uno::RuntimeException& )
            {
            }
        }
        InvalidateRenderGeometry();
    }
    return bRet;
}
bool SdrObjCustomShape::AdjustTextFrameWidthAndHeight(bool bHgt, bool bWdt)
{
    Rectangle aNewTextRect = ImpCalculateTextFrame( bHgt, bWdt );
    bool bRet = !aNewTextRect.IsEmpty() && ( aNewTextRect != aRect );
    if ( bRet )
    {
        Rectangle aBoundRect0;
        if ( pUserCall )
            aBoundRect0 = GetCurrentBoundRect();

        // taking care of handles that should not been changed
        std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( this ) );

        aRect = aNewTextRect;
        SetRectsDirty();

        for (std::vector< SdrCustomShapeInteraction >::const_iterator aIter( aInteractionHandles.begin() ), aEnd( aInteractionHandles.end() ) ;
            aIter != aEnd ; ++aIter)
        {
            try
            {
                if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_FIXED )
                    aIter->xInteraction->setControllerPosition( aIter->aPosition );
            }
            catch ( const uno::RuntimeException& )
            {
            }
        }

        InvalidateRenderGeometry();
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
    return bRet;
}
sal_Bool SdrObjCustomShape::BegTextEdit( SdrOutliner& rOutl )
{
    return SdrTextObj::BegTextEdit( rOutl );
}
void SdrObjCustomShape::TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const
{
    Size aPaperMin,aPaperMax;
    Rectangle aViewInit;
    TakeTextAnchorRect( aViewInit );
    if ( aGeo.nDrehWink )
    {
        Point aCenter(aViewInit.Center());
        aCenter-=aViewInit.TopLeft();
        Point aCenter0(aCenter);
        RotatePoint(aCenter,Point(),aGeo.nSin,aGeo.nCos);
        aCenter-=aCenter0;
        aViewInit.Move(aCenter.X(),aCenter.Y());
    }
    Size aAnkSiz(aViewInit.GetSize());
    aAnkSiz.Width()--; aAnkSiz.Height()--; // because GetSize() adds 1
    Size aMaxSiz(1000000,1000000);
    if (pModel!=NULL) {
        Size aTmpSiz(pModel->GetMaxObjSize());
        if (aTmpSiz.Width()!=0) aMaxSiz.Width()=aTmpSiz.Width();
        if (aTmpSiz.Height()!=0) aMaxSiz.Height()=aTmpSiz.Height();
    }
    SdrTextHorzAdjust eHAdj(GetTextHorizontalAdjust());
    SdrTextVertAdjust eVAdj(GetTextVerticalAdjust());

    long nMinWdt = GetMinTextFrameWidth();
    long nMinHgt = GetMinTextFrameHeight();
    long nMaxWdt = GetMaxTextFrameWidth();
    long nMaxHgt = GetMaxTextFrameHeight();
    if (nMinWdt<1) nMinWdt=1;
    if (nMinHgt<1) nMinHgt=1;
    if ( nMaxWdt == 0 || nMaxWdt > aMaxSiz.Width() )
        nMaxWdt = aMaxSiz.Width();
    if ( nMaxHgt == 0 || nMaxHgt > aMaxSiz.Height() )
        nMaxHgt=aMaxSiz.Height();

    if (((SdrTextWordWrapItem&)(GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue())
    {
        if ( IsVerticalWriting() )
        {
            nMaxHgt = aAnkSiz.Height();
            nMinHgt = nMaxHgt;
        }
        else
        {
            nMaxWdt = aAnkSiz.Width();
            nMinWdt = nMaxWdt;
        }
    }
    aPaperMax.Width()=nMaxWdt;
    aPaperMax.Height()=nMaxHgt;

    aPaperMin.Width()=nMinWdt;
    aPaperMin.Height()=nMinHgt;

    if ( pViewMin )
    {
        *pViewMin = aViewInit;

        long nXFree = aAnkSiz.Width() - aPaperMin.Width();
        if ( eHAdj == SDRTEXTHORZADJUST_LEFT )
            pViewMin->Right() -= nXFree;
        else if ( eHAdj == SDRTEXTHORZADJUST_RIGHT )
            pViewMin->Left() += nXFree;
        else { pViewMin->Left() += nXFree / 2; pViewMin->Right() = pViewMin->Left() + aPaperMin.Width(); }

        long nYFree = aAnkSiz.Height() - aPaperMin.Height();
        if ( eVAdj == SDRTEXTVERTADJUST_TOP )
            pViewMin->Bottom() -= nYFree;
        else if ( eVAdj == SDRTEXTVERTADJUST_BOTTOM )
            pViewMin->Top() += nYFree;
        else { pViewMin->Top() += nYFree / 2; pViewMin->Bottom() = pViewMin->Top() + aPaperMin.Height(); }
    }

    if( IsVerticalWriting() )
        aPaperMin.Width() = 0;
    else
        aPaperMin.Height() = 0;

    if( eHAdj != SDRTEXTHORZADJUST_BLOCK )
        aPaperMin.Width()=0;

    // For complete vertical adjust support, set paper min height to 0, here.
    if(SDRTEXTVERTADJUST_BLOCK != eVAdj )
        aPaperMin.Height() = 0;

    if (pPaperMin!=NULL) *pPaperMin=aPaperMin;
    if (pPaperMax!=NULL) *pPaperMax=aPaperMax;
    if (pViewInit!=NULL) *pViewInit=aViewInit;
}
void SdrObjCustomShape::EndTextEdit( SdrOutliner& rOutl )
{
    SdrTextObj::EndTextEdit( rOutl );
    InvalidateRenderGeometry();
}
void SdrObjCustomShape::TakeTextAnchorRect( Rectangle& rAnchorRect ) const
{
    if ( GetTextBounds( rAnchorRect ) )
    {
        Point aRotateRef( maSnapRect.Center() );
        rAnchorRect.Left()   += GetTextLeftDistance();
        rAnchorRect.Top()    += GetTextUpperDistance();
        rAnchorRect.Right()  -= GetTextRightDistance();
        rAnchorRect.Bottom() -= GetTextLowerDistance();
        ImpJustifyRect( rAnchorRect );

        if ( rAnchorRect.GetWidth() < 2 )
            rAnchorRect.Right() = rAnchorRect.Left() + 1;   // minimal width is 2
        if ( rAnchorRect.GetHeight() < 2 )
            rAnchorRect.Bottom() = rAnchorRect.Top() + 1;   // minimal height is 2
        if ( aGeo.nDrehWink )
        {
            Point aP( rAnchorRect.TopLeft() );
            RotatePoint( aP, aRotateRef, aGeo.nSin, aGeo. nCos );
            rAnchorRect.SetPos( aP );
        }
    }
    else
        SdrTextObj::TakeTextAnchorRect( rAnchorRect );
}
void SdrObjCustomShape::TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, bool bNoEditText,
                               Rectangle* pAnchorRect, bool /*bLineWidth*/) const
{
    Rectangle aAnkRect; // Rect in which we anchor
    TakeTextAnchorRect(aAnkRect);
    SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
    SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
    sal_uIntPtr nStat0=rOutliner.GetControlWord();
    Size aNullSize;

    rOutliner.SetControlWord(nStat0|EE_CNTRL_AUTOPAGESIZE);
    rOutliner.SetMinAutoPaperSize(aNullSize);
    sal_Int32 nMaxAutoPaperWidth = 1000000;
    sal_Int32 nMaxAutoPaperHeight= 1000000;

    long nAnkWdt=aAnkRect.GetWidth();
    long nAnkHgt=aAnkRect.GetHeight();

    if (((SdrTextWordWrapItem&)(GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue())
    {
        if ( IsVerticalWriting() )
            nMaxAutoPaperHeight = nAnkHgt;
        else
            nMaxAutoPaperWidth = nAnkWdt;
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

    // put text into the Outliner - if necessary the use the text from the EditOutliner
    OutlinerParaObject* pPara= GetOutlinerParaObject();
    if (pEdtOutl && !bNoEditText)
        pPara=pEdtOutl->CreateParaObject();

    if (pPara)
    {
        bool bHitTest = false;
        if( pModel )
            bHitTest = &pModel->GetHitTestOutliner() == &rOutliner;

        const SdrTextObj* pTestObj = rOutliner.GetTextObj();
        if( !pTestObj || !bHitTest || pTestObj != this ||
            pTestObj->GetOutlinerParaObject() != GetOutlinerParaObject() )
        {
            if( bHitTest )
                rOutliner.SetTextObj( this );

            rOutliner.SetUpdateMode(sal_True);
            rOutliner.SetText(*pPara);
        }
    }
    else
    {
        rOutliner.SetTextObj( NULL );
    }
    if (pEdtOutl && !bNoEditText && pPara)
        delete pPara;

    rOutliner.SetUpdateMode(sal_True);
    rOutliner.SetControlWord(nStat0);

    SdrText* pText = getActiveText();
    if( pText )
        pText->CheckPortionInfo( rOutliner );

    Point aTextPos(aAnkRect.TopLeft());
    Size aTextSiz(rOutliner.GetPaperSize()); // GetPaperSize() has a little added tolerance, no?

    // For draw objects containing text correct horizontal/vertical alignment if text is bigger
    // than the object itself. Without that correction, the text would always be
    // formatted to the left edge (or top edge when vertical) of the draw object.

    if( !IsTextFrame() )
    {
        if(aAnkRect.GetWidth() < aTextSiz.Width() && !IsVerticalWriting())
        {
            // Horizontal case here. Correct only if eHAdj == SDRTEXTHORZADJUST_BLOCK,
            // else the alignment is wanted.
            if(SDRTEXTHORZADJUST_BLOCK == eHAdj)
            {
                eHAdj = SDRTEXTHORZADJUST_CENTER;
            }
        }

        if(aAnkRect.GetHeight() < aTextSiz.Height() && IsVerticalWriting())
        {
            // Vertical case here. Correct only if eHAdj == SDRTEXTVERTADJUST_BLOCK,
            // else the alignment is wanted.
            if(SDRTEXTVERTADJUST_BLOCK == eVAdj)
            {
                eVAdj = SDRTEXTVERTADJUST_CENTER;
            }
        }
    }

    if (eHAdj==SDRTEXTHORZADJUST_CENTER || eHAdj==SDRTEXTHORZADJUST_RIGHT)
    {
        long nFreeWdt=aAnkRect.GetWidth()-aTextSiz.Width();
        if (eHAdj==SDRTEXTHORZADJUST_CENTER)
            aTextPos.X()+=nFreeWdt/2;
        if (eHAdj==SDRTEXTHORZADJUST_RIGHT)
            aTextPos.X()+=nFreeWdt;
    }
    if (eVAdj==SDRTEXTVERTADJUST_CENTER || eVAdj==SDRTEXTVERTADJUST_BOTTOM)
    {
        long nFreeHgt=aAnkRect.GetHeight()-aTextSiz.Height();
        if (eVAdj==SDRTEXTVERTADJUST_CENTER)
            aTextPos.Y()+=nFreeHgt/2;
        if (eVAdj==SDRTEXTVERTADJUST_BOTTOM)
            aTextPos.Y()+=nFreeHgt;
    }
    if (aGeo.nDrehWink!=0)
        RotatePoint(aTextPos,aAnkRect.TopLeft(),aGeo.nSin,aGeo.nCos);

    if (pAnchorRect)
        *pAnchorRect=aAnkRect;

    // using rTextRect together with ContourFrame doesn't always work correctly
    rTextRect=Rectangle(aTextPos,aTextSiz);
}

void SdrObjCustomShape::NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject)
{
    SdrTextObj::NbcSetOutlinerParaObject( pTextObject );
    SetBoundRectDirty();
    SetRectsDirty(sal_True);
    InvalidateRenderGeometry();
}

SdrObjCustomShape* SdrObjCustomShape::Clone() const
{
    return CloneHelper< SdrObjCustomShape >();
}

SdrObjCustomShape& SdrObjCustomShape::operator=(const SdrObjCustomShape& rObj)
{
    if( this == &rObj )
        return *this;
    SdrTextObj::operator=( rObj );
    aName = rObj.aName;
    fObjectRotation = rObj.fObjectRotation;
    InvalidateRenderGeometry();
    return *this;
}


OUString SdrObjCustomShape::TakeObjNameSingul() const
{
    OUStringBuffer sName(ImpGetResStr(STR_ObjNameSingulCUSTOMSHAPE));
    OUString aNm(GetName());
    if (!aNm.isEmpty())
    {
        sName.append(' ');
        sName.append('\'');
        sName.append(aNm);
        sName.append('\'');
    }
    return sName.makeStringAndClear();
}

OUString SdrObjCustomShape::TakeObjNamePlural() const
{
    return ImpGetResStr(STR_ObjNamePluralCUSTOMSHAPE);
}

basegfx::B2DPolyPolygon SdrObjCustomShape::TakeXorPoly() const
{
    return GetLineGeometry( (SdrObjCustomShape*)this, sal_False );
}

basegfx::B2DPolyPolygon SdrObjCustomShape::TakeContour() const
{
    const SdrObject* pSdrObject = GetSdrObjectFromCustomShape();
    if ( pSdrObject )
        return pSdrObject->TakeContour();
    return basegfx::B2DPolyPolygon();
}

SdrObject* SdrObjCustomShape::DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const
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
        SdrObject* pCandidate = pRenderedCustomShape->Clone();
        DBG_ASSERT(pCandidate, "SdrObjCustomShape::DoConvertToPolyObj: Could not clone SdrObject (!)");
        pCandidate->SetModel(GetModel());
        pRetval = pCandidate->DoConvertToPolyObj(bBezier, bAddText);
        SdrObject::Free( pCandidate );

        if(pRetval)
        {
            const sal_Bool bShadow(((SdrShadowItem&)GetMergedItem(SDRATTR_SHADOW)).GetValue());
            if(bShadow)
            {
                pRetval->SetMergedItem(SdrShadowItem(sal_True));
            }
        }

        if(bAddText && HasText() && !IsTextPath())
        {
            pRetval = ImpConvertAddText(pRetval, bBezier);
        }
    }

    return pRetval;
}

void SdrObjCustomShape::NbcSetStyleSheet( SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr )
{
    // #i40944#
    InvalidateRenderGeometry();
    SdrObject::NbcSetStyleSheet( pNewStyleSheet, bDontRemoveHardAttr );
}

void SdrObjCustomShape::SetPage( SdrPage* pNewPage )
{
    SdrTextObj::SetPage( pNewPage );

    if( pNewPage )
    {
        // invalidating rectangles by SetRectsDirty is not sufficient,
        // AdjustTextFrameWidthAndHeight() also has to be made, both
        // actions are done by NbcSetSnapRect
        Rectangle aTmp( aRect );    //creating temporary rectangle #i61108#
        NbcSetSnapRect( aTmp );
    }
}

SdrObjGeoData* SdrObjCustomShape::NewGeoData() const
{
    return new SdrAShapeObjGeoData;
}

void SdrObjCustomShape::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrTextObj::SaveGeoData( rGeo );
    SdrAShapeObjGeoData& rAGeo=(SdrAShapeObjGeoData&)rGeo;
    rAGeo.fObjectRotation = fObjectRotation;
    rAGeo.bMirroredX = IsMirroredX();
    rAGeo.bMirroredY = IsMirroredY();

    const OUString sAdjustmentValues( "AdjustmentValues" );
    Any* pAny( ( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) ).GetPropertyValueByName( sAdjustmentValues ) );
    if ( pAny )
        *pAny >>= rAGeo.aAdjustmentSeq;
}

void SdrObjCustomShape::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrTextObj::RestGeoData( rGeo );
    SdrAShapeObjGeoData& rAGeo=(SdrAShapeObjGeoData&)rGeo;
    fObjectRotation = rAGeo.fObjectRotation;
    SetMirroredX( rAGeo.bMirroredX );
    SetMirroredY( rAGeo.bMirroredY );

    SdrCustomShapeGeometryItem rGeometryItem = (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    const OUString sAdjustmentValues( "AdjustmentValues" );
    PropertyValue aPropVal;
    aPropVal.Name = sAdjustmentValues;
    aPropVal.Value <<= rAGeo.aAdjustmentSeq;
    rGeometryItem.SetPropertyValue( aPropVal );
    SetMergedItem( rGeometryItem );

    InvalidateRenderGeometry();
}

void SdrObjCustomShape::TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& /*rPolyPolygon*/)
{
    // break up matrix
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate, fShearX;
    rMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

    // #i75086# Old DrawingLayer (GeoStat and geometry) does not support holding negative scalings
    // in X and Y which equal a 180 degree rotation. Recognize it and react accordingly
    if(basegfx::fTools::less(aScale.getX(), 0.0) && basegfx::fTools::less(aScale.getY(), 0.0))
    {
        aScale.setX(fabs(aScale.getX()));
        aScale.setY(fabs(aScale.getY()));
        fRotate = fmod(fRotate + F_PI, F_2PI);
    }

    // reset object shear and rotations
    aGeo.nDrehWink = 0;
    aGeo.RecalcSinCos();
    aGeo.nShearWink = 0;
    aGeo.RecalcTan();

    // force metric to pool metric
    const SfxMapUnit eMapUnit(GetObjectMapUnit());
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // position
                aTranslate.setX(ImplMMToTwips(aTranslate.getX()));
                aTranslate.setY(ImplMMToTwips(aTranslate.getY()));

                // size
                aScale.setX(ImplMMToTwips(aScale.getX()));
                aScale.setY(ImplMMToTwips(aScale.getY()));

                break;
            }
            default:
            {
                OSL_FAIL("TRSetBaseGeometry: Missing unit translation to PoolMetric!");
            }
        }
    }

    // if anchor is used, make position relative to it
    if( pModel && pModel->IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate += basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // build and set BaseRect (use scale)
    Point aPoint = Point();
    Size aSize(FRound(aScale.getX()), FRound(aScale.getY()));
    // fdo#47434 We need a valid rectangle here
    if( !aSize.Height() ) aSize.setHeight( 1 );
    if( !aSize.Width() ) aSize.setWidth( 1 );

    Rectangle aBaseRect(aPoint, aSize);
    SetSnapRect(aBaseRect);

    // shear?
    if(!basegfx::fTools::equalZero(fShearX))
    {
        GeoStat aGeoStat;
        // #i121932# do *not* forget to invert shearX(!)
        aGeoStat.nShearWink = FRound((atan(-fShearX) / F_PI180) * 100.0);
        aGeoStat.RecalcTan();
        Shear(Point(), aGeoStat.nShearWink, aGeoStat.nTan, sal_False);
    }

    // rotation?
    if(!basegfx::fTools::equalZero(fRotate))
    {
        GeoStat aGeoStat;

        // #i78696#
        // fRotate is mathematically correct, but aGeoStat.nDrehWink is
        // mirrored -> mirror value here
        aGeoStat.nDrehWink = NormAngle360(FRound(-fRotate / F_PI18000));
        aGeoStat.RecalcSinCos();
        Rotate(Point(), aGeoStat.nDrehWink, aGeoStat.nSin, aGeoStat.nCos);
    }

    // translate?
    if(!aTranslate.equalZero())
    {
        Move(Size(FRound(aTranslate.getX()), FRound(aTranslate.getY())));
    }
}

// taking fObjectRotation instead of aGeo.nWink
sal_Bool SdrObjCustomShape::TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& /*rPolyPolygon*/) const
{
    // get turn and shear
    double fRotate = fObjectRotation * F_PI180;
    double fShearX = (aGeo.nShearWink / 100.0) * F_PI180;

    // get aRect, this is the unrotated snaprect
    Rectangle aRectangle(aRect);

    sal_Bool bMirroredX = IsMirroredX();
    sal_Bool bMirroredY = IsMirroredY();
    if ( bMirroredX || bMirroredY )
    {   // we have to retrieve the unmirrored rect

        GeoStat aNewGeo( aGeo );

        if ( bMirroredX )
        {
            Polygon aPol( Rect2Poly( aRect, aNewGeo ) );
            Rectangle aBoundRect( aPol.GetBoundRect() );

            Point aRef1( ( aBoundRect.Left() + aBoundRect.Right() ) >> 1, aBoundRect.Top() );
            Point aRef2( aRef1.X(), aRef1.Y() + 1000 );
            sal_uInt16 i;
            sal_uInt16 nPntAnz=aPol.GetSize();
            for (i=0; i<nPntAnz; i++)
            {
                MirrorPoint(aPol[i],aRef1,aRef2);
            }
            // mirror polygon and move it a bit
            Polygon aPol0(aPol);
            aPol[0]=aPol0[1];
            aPol[1]=aPol0[0];
            aPol[2]=aPol0[3];
            aPol[3]=aPol0[2];
            aPol[4]=aPol0[1];
            Poly2Rect(aPol,aRectangle,aNewGeo);
        }
        if ( bMirroredY )
        {
            Polygon aPol( Rect2Poly( aRectangle, aNewGeo ) );
            Rectangle aBoundRect( aPol.GetBoundRect() );

            Point aRef1( aBoundRect.Left(), ( aBoundRect.Top() + aBoundRect.Bottom() ) >> 1 );
            Point aRef2( aRef1.X() + 1000, aRef1.Y() );
            sal_uInt16 i;
            sal_uInt16 nPntAnz=aPol.GetSize();
            for (i=0; i<nPntAnz; i++)
            {
                MirrorPoint(aPol[i],aRef1,aRef2);
            }
            // mirror polygon and move it a bit
            Polygon aPol0(aPol);
            aPol[0]=aPol0[1]; // This was WRONG for vertical (!)
            aPol[1]=aPol0[0]; // #i121932# Despite my own coment above
            aPol[2]=aPol0[3]; // it was *not* wrong even when the reordering
            aPol[3]=aPol0[2]; // *seems* to be specific for X-Mirrorings. Oh
            aPol[4]=aPol0[1]; // will I be happy when this old stuff is |gone| with aw080 (!)
            Poly2Rect(aPol,aRectangle,aNewGeo);
        }
    }

    // fill other values
    basegfx::B2DTuple aScale(aRectangle.GetWidth(), aRectangle.GetHeight());
    basegfx::B2DTuple aTranslate(aRectangle.Left(), aRectangle.Top());

    // position may be relative to anchorpos, convert
    if( pModel && pModel->IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate -= basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // force MapUnit to 100th mm
    const SfxMapUnit eMapUnit(GetObjectMapUnit());
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // position
                aTranslate.setX(ImplTwipsToMM(aTranslate.getX()));
                aTranslate.setY(ImplTwipsToMM(aTranslate.getY()));

                // size
                aScale.setX(ImplTwipsToMM(aScale.getX()));
                aScale.setY(ImplTwipsToMM(aScale.getY()));

                break;
            }
            default:
            {
                OSL_FAIL("TRGetBaseGeometry: Missing unit translation to 100th mm!");
            }
        }
    }

    // build matrix
    rMatrix = basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
        aScale,
        basegfx::fTools::equalZero(fShearX) ? 0.0 : tan(fShearX),
        basegfx::fTools::equalZero(fRotate) ? 0.0 : -fRotate,
        aTranslate);

    return sal_False;
}

sdr::contact::ViewContact* SdrObjCustomShape::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrObjCustomShape(*this);
}

// #i33136#
bool SdrObjCustomShape::doConstructOrthogonal(const OUString& rName)
{
    bool bRetval(false);
    static OUString Imps_sNameASOrtho_quadrat(  "quadrat"  );
    static OUString Imps_sNameASOrtho_round_quadrat(  "round-quadrat"  );
    static OUString Imps_sNameASOrtho_circle(  "circle"  );
    static OUString Imps_sNameASOrtho_circle_pie(  "circle-pie"  );
    static OUString Imps_sNameASOrtho_ring(  "ring"  );

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
    SdrObject::Free( mpLastShadowGeometry );
    mpLastShadowGeometry = 0L;
}

void SdrObjCustomShape::impl_setUnoShape(const uno::Reference<uno::XInterface>& rxUnoShape)
{
    SdrTextObj::impl_setUnoShape(rxUnoShape);

    // The shape engine is created with _current_ shape. This means we
    // _must_ reset it when the shape changes.
    mxCustomShapeEngine.set(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
