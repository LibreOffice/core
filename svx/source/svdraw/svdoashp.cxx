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
#include <svx/unoapi.hxx>
#include <svx/unoshape.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/datetime.hxx>
#include <sfx2/lnkbase.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/processfactory.hxx>
#include <svl/urihelper.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svx/svdogrp.hxx>
#include <tools/helpers.hxx>
#include <svx/svddrag.hxx>
#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>
#include <svx/svddrgmt.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoedge.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <editeng/eeitem.hxx>
#include <editeng/editstat.hxx>
#include <editeng/adjustitem.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/outlobj.hxx>
#include <svx/sdtfchim.hxx>
#include <svx/EnhancedCustomShapeGeometry.hxx>
#include <svx/EnhancedCustomShapeTypeNames.hxx>
#include <svx/EnhancedCustomShape2d.hxx>
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
#include <sdr/properties/customshapeproperties.hxx>
#include <sdr/contact/viewcontactofsdrobjcustomshape.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/virdev.hxx>
#include <svx/svdview.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svdobjplusdata.hxx>

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
}

static MSO_SPT ImpGetCustomShapeType( const SdrObjCustomShape& rCustoShape )
{
    MSO_SPT eRetValue = mso_sptNil;

    OUString aEngine( rCustoShape.GetMergedItem( SDRATTR_CUSTOMSHAPE_ENGINE ).GetValue() );
    if ( aEngine.isEmpty() || aEngine == "com.sun.star.drawing.EnhancedCustomShapeEngine" )
    {
        OUString sShapeType;
        const SdrCustomShapeGeometryItem& rGeometryItem( rCustoShape.GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
        const Any* pAny = rGeometryItem.GetPropertyValueByName( "Type" );
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
static SdrObject* ImpCreateShadowObjectClone(const SdrObject& rOriginal, const SfxItemSet& rOriginalSet)
{
    SdrObject* pRetval = nullptr;
    const bool bShadow(rOriginalSet.Get(SDRATTR_SHADOW).GetValue());

    if(bShadow)
    {
        // create a shadow representing object
        const sal_Int32 nXDist(rOriginalSet.Get(SDRATTR_SHADOWXDIST).GetValue());
        const sal_Int32 nYDist(rOriginalSet.Get(SDRATTR_SHADOWYDIST).GetValue());
        const ::Color aShadowColor(rOriginalSet.Get(SDRATTR_SHADOWCOLOR).GetColorValue());
        const sal_uInt16 nShadowTransparence(rOriginalSet.Get(SDRATTR_SHADOWTRANSPARENCE).GetValue());
        pRetval = rOriginal.CloneSdrObject(rOriginal.getSdrModelFromSdrObject());
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
            drawing::FillStyle eFillStyle = pObj->GetMergedItem(XATTR_FILLSTYLE).GetValue();

            if(!bLineUsed)
            {
                drawing::LineStyle eLineStyle = pObj->GetMergedItem(XATTR_LINESTYLE).GetValue();

                if(drawing::LineStyle_NONE != eLineStyle)
                {
                    bLineUsed = true;
                }
            }

            if(!bAllFillUsed)
            {
                if(!bSolidFillUsed && drawing::FillStyle_SOLID == eFillStyle)
                {
                    bSolidFillUsed = true;
                    bAllFillUsed = (bSolidFillUsed && bGradientFillUsed && bHatchFillUsed && bBitmapFillUsed);
                }
                if(!bGradientFillUsed && drawing::FillStyle_GRADIENT == eFillStyle)
                {
                    bGradientFillUsed = true;
                    bAllFillUsed = (bSolidFillUsed && bGradientFillUsed && bHatchFillUsed && bBitmapFillUsed);
                }
                if(!bHatchFillUsed && drawing::FillStyle_HATCH == eFillStyle)
                {
                    bHatchFillUsed = true;
                    bAllFillUsed = (bSolidFillUsed && bGradientFillUsed && bHatchFillUsed && bBitmapFillUsed);
                }
                if(!bBitmapFillUsed && drawing::FillStyle_BITMAP == eFillStyle)
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
        aTempSet.Put( SvxWritingModeItem( css::text::WritingMode_LR_TB, SDRATTR_TEXTDIRECTION ) );

        // no shadow
        aTempSet.Put(makeSdrShadowItem(false));
        aTempSet.Put(makeSdrShadowXDistItem(0));
        aTempSet.Put(makeSdrShadowYDistItem(0));

        // line color and transparency like shadow
        if(bLineUsed)
        {
            aTempSet.Put(XLineColorItem(OUString(), aShadowColor));
            aTempSet.Put(XLineTransparenceItem(nShadowTransparence));
        }

        // fill color and transparency like shadow
        if(bSolidFillUsed)
        {
            aTempSet.Put(XFillColorItem(OUString(), aShadowColor));
            aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
        }

        // gradient and transparency like shadow
        if(bGradientFillUsed)
        {
            XGradient aGradient(rOriginalSet.Get(XATTR_FILLGRADIENT).GetGradientValue());
            sal_uInt8 nStartLuminance(aGradient.GetStartColor().GetLuminance());
            sal_uInt8 nEndLuminance(aGradient.GetEndColor().GetLuminance());

            if(aGradient.GetStartIntens() != 100)
            {
                nStartLuminance = static_cast<sal_uInt8>(nStartLuminance * (static_cast<double>(aGradient.GetStartIntens()) / 100.0));
            }

            if(aGradient.GetEndIntens() != 100)
            {
                nEndLuminance = static_cast<sal_uInt8>(nEndLuminance * (static_cast<double>(aGradient.GetEndIntens()) / 100.0));
            }

            ::Color aStartColor(
                static_cast<sal_uInt8>((nStartLuminance * aShadowColor.GetRed()) / 256),
                static_cast<sal_uInt8>((nStartLuminance * aShadowColor.GetGreen()) / 256),
                static_cast<sal_uInt8>((nStartLuminance * aShadowColor.GetBlue()) / 256));

            ::Color aEndColor(
                static_cast<sal_uInt8>((nEndLuminance * aShadowColor.GetRed()) / 256),
                static_cast<sal_uInt8>((nEndLuminance * aShadowColor.GetGreen()) / 256),
                static_cast<sal_uInt8>((nEndLuminance * aShadowColor.GetBlue()) / 256));

            aGradient.SetStartColor(aStartColor);
            aGradient.SetEndColor(aEndColor);
            aTempSet.Put(XFillGradientItem(aGradient));
            aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
        }

        // hatch and transparency like shadow
        if(bHatchFillUsed)
        {
            XHatch aHatch(rOriginalSet.Get(XATTR_FILLHATCH).GetHatchValue());
            aHatch.SetColor(aShadowColor);
            aTempSet.Put(XFillHatchItem(aHatch));
            aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
        }

        // bitmap and transparency like shadow
        if(bBitmapFillUsed)
        {
            GraphicObject aGraphicObject(rOriginalSet.Get(XATTR_FILLBITMAP).GetGraphicObject());
            const BitmapEx aBitmapEx(aGraphicObject.GetGraphic().GetBitmapEx());
            Bitmap aBitmap(aBitmapEx.GetBitmap());

            if(!aBitmap.IsEmpty())
            {
                Bitmap::ScopedReadAccess pReadAccess(aBitmap);

                if(pReadAccess)
                {
                    ScopedVclPtr<VirtualDevice> pVirDev(VclPtr<VirtualDevice>::Create());
                    pVirDev->SetOutputSizePixel(aBitmap.GetSizePixel());

                    for(long y(0); y < pReadAccess->Height(); y++)
                    {
                        for(long x(0); x < pReadAccess->Width(); x++)
                        {
                            const BitmapColor aColor = pReadAccess->GetColor(y, x);
                            sal_uInt16 nLuminance(static_cast<sal_uInt16>(aColor.GetLuminance()) + 1);
                            const Color aDestColor(
                                static_cast<sal_uInt8>((nLuminance * static_cast<sal_uInt16>(aShadowColor.GetRed())) >> 8),
                                static_cast<sal_uInt8>((nLuminance * static_cast<sal_uInt16>(aShadowColor.GetGreen())) >> 8),
                                static_cast<sal_uInt8>((nLuminance * static_cast<sal_uInt16>(aShadowColor.GetBlue())) >> 8));
                            pVirDev->DrawPixel(Point(x,y), aDestColor);
                        }
                    }

                    pReadAccess.reset();

                    aGraphicObject.SetGraphic(Graphic(pVirDev->GetBitmapEx(Point(0,0), aBitmap.GetSizePixel())));
                }
            }

            aTempSet.Put(XFillBitmapItem(aGraphicObject));
            aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
        }

        // set attributes and paint shadow object
        pRetval->SetMergedItemSet( aTempSet );
    }
    return pRetval;
}


Reference< XCustomShapeEngine > const & SdrObjCustomShape::GetCustomShapeEngine() const
{
    if (mxCustomShapeEngine.is())
        return mxCustomShapeEngine;

    OUString aEngine(GetMergedItem( SDRATTR_CUSTOMSHAPE_ENGINE ).GetValue());
    if ( aEngine.isEmpty() )
        aEngine = "com.sun.star.drawing.EnhancedCustomShapeEngine";

    Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    Reference< XShape > aXShape = GetXShapeForSdrObject(const_cast<SdrObjCustomShape*>(this));
    if ( aXShape.is() )
    {
        Sequence< Any > aArgument( 1 );
        Sequence< PropertyValue > aPropValues( 1 );
        aPropValues[ 0 ].Name = "CustomShape";
        aPropValues[ 0 ].Value <<= aXShape;
        aArgument[ 0 ] <<= aPropValues;
        try
        {
            Reference<XInterface> xInterface(xContext->getServiceManager()->createInstanceWithArgumentsAndContext(aEngine, aArgument, xContext));
            if (xInterface.is())
                mxCustomShapeEngine.set( xInterface, UNO_QUERY );
        }
        catch (const css::loader::CannotActivateFactoryException&)
        {
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
            const_cast<SdrObjCustomShape*>(this)->mXRenderedCustomShape = xCustomShapeEngine->render();
    }
    SdrObject* pRenderedCustomShape = mXRenderedCustomShape.is()
                ? GetSdrObjectFromXShape( mXRenderedCustomShape )
                : nullptr;
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
            const bool bShadow(rOriginalSet.Get( SDRATTR_SHADOW ).GetValue());

            if(bShadow)
            {
                // create a clone with all attributes changed to shadow attributes
                // and translation executed, too.
                const_cast<SdrObjCustomShape*>(this)->mpLastShadowGeometry =
                    ImpCreateShadowObjectClone(*pSdrObject, rOriginalSet);
            }
        }
    }

    return mpLastShadowGeometry;
}

bool SdrObjCustomShape::IsTextPath() const
{
    const OUString sTextPath( "TextPath" );
    bool bTextPathOn = false;
    const SdrCustomShapeGeometryItem& rGeometryItem = GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    const Any* pAny = rGeometryItem.GetPropertyValueByName( sTextPath, sTextPath );
    if ( pAny )
        *pAny >>= bTextPathOn;
    return bTextPathOn;
}

bool SdrObjCustomShape::UseNoFillStyle() const
{
    bool bRet = false;
    OUString sShapeType;
    const OUString sType( "Type" );
    const SdrCustomShapeGeometryItem& rGeometryItem( GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    const Any* pAny = rGeometryItem.GetPropertyValueByName( sType );
    if ( pAny )
        *pAny >>= sShapeType;
    bRet = !IsCustomShapeFilledByDefault( EnhancedCustomShapeTypeNames::Get( sType ) );

    return bRet;
}

bool SdrObjCustomShape::IsMirroredX() const
{
    bool bMirroredX = false;
    SdrCustomShapeGeometryItem aGeometryItem( GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    const OUString sMirroredX( "MirroredX" );
    css::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredX );
    if ( pAny )
        *pAny >>= bMirroredX;
    return bMirroredX;
}
bool SdrObjCustomShape::IsMirroredY() const
{
    bool bMirroredY = false;
    SdrCustomShapeGeometryItem aGeometryItem( GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    const OUString sMirroredY( "MirroredY" );
    css::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredY );
    if ( pAny )
        *pAny >>= bMirroredY;
    return bMirroredY;
}
void SdrObjCustomShape::SetMirroredX( const bool bMirrorX )
{
    SdrCustomShapeGeometryItem aGeometryItem( GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    const OUString sMirroredX( "MirroredX" );
    PropertyValue aPropVal;
    aPropVal.Name = sMirroredX;
    aPropVal.Value <<= bMirrorX;
    aGeometryItem.SetPropertyValue( aPropVal );
    SetMergedItem( aGeometryItem );
}
void SdrObjCustomShape::SetMirroredY( const bool bMirrorY )
{
    SdrCustomShapeGeometryItem aGeometryItem( GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    const OUString sMirroredY( "MirroredY" );
    PropertyValue aPropVal;
    aPropVal.Name = sMirroredY;
    aPropVal.Value <<= bMirrorY;
    aGeometryItem.SetPropertyValue( aPropVal );
    SetMergedItem( aGeometryItem );
}

double SdrObjCustomShape::GetExtraTextRotation( const bool bPreRotation ) const
{
    const css::uno::Any* pAny;
    const SdrCustomShapeGeometryItem& rGeometryItem = GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    const OUString sTextRotateAngle( "TextRotateAngle" );
    const OUString sTextPreRotateAngle( "TextPreRotateAngle" );
    pAny = rGeometryItem.GetPropertyValueByName( bPreRotation ? sTextPreRotateAngle : sTextRotateAngle );
    double fExtraTextRotateAngle = 0.0;
    if ( pAny )
        *pAny >>= fExtraTextRotateAngle;
    return fExtraTextRotateAngle;
}

bool SdrObjCustomShape::GetTextBounds( tools::Rectangle& rTextBound ) const
{
    bool bRet = false;

    Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine() );
    if ( xCustomShapeEngine.is() )
    {
        awt::Rectangle aR( xCustomShapeEngine->getTextBounds() );
        if ( aR.Width > 1 && aR.Height > 1 )
        {
            rTextBound = tools::Rectangle( Point( aR.X, aR.Y ), Size( aR.Width, aR.Height ) );
            bRet = true;
        }
    }
    return bRet;
}
basegfx::B2DPolyPolygon SdrObjCustomShape::GetLineGeometry( const bool bBezierAllowed ) const
{
    basegfx::B2DPolyPolygon aRetval;
    Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine() );
    if ( xCustomShapeEngine.is() )
    {
        css::drawing::PolyPolygonBezierCoords aBezierCoords = xCustomShapeEngine->getLineGeometry();
        try
        {
            aRetval = basegfx::utils::UnoPolyPolygonBezierCoordsToB2DPolyPolygon( aBezierCoords );
            if ( !bBezierAllowed && aRetval.areControlPointsUsed())
            {
                aRetval = basegfx::utils::adaptiveSubdivideByAngle(aRetval);
            }
        }
        catch ( const css::lang::IllegalArgumentException & )
        {
        }
    }
    return aRetval;
}

std::vector< SdrCustomShapeInteraction > SdrObjCustomShape::GetInteractionHandles() const
{
    std::vector< SdrCustomShapeInteraction > aRet;
    try
    {
        Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine() );
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

                    CustomShapeHandleModes nMode = CustomShapeHandleModes::NONE;
                    switch( ImpGetCustomShapeType( *this ) )
                    {
                        case mso_sptAccentBorderCallout90 :     // 2 ortho
                        {
                            if (i == 0)
                                nMode |= CustomShapeHandleModes::RESIZE_FIXED | CustomShapeHandleModes::CREATE_FIXED;
                            else if (i == 1)
                                nMode |= CustomShapeHandleModes::RESIZE_ABSOLUTE_X | CustomShapeHandleModes::RESIZE_ABSOLUTE_Y | CustomShapeHandleModes::MOVE_SHAPE | CustomShapeHandleModes::ORTHO4;
                        }
                        break;

                        case mso_sptChevron :
                        case mso_sptHomePlate :
                             nMode |= CustomShapeHandleModes::RESIZE_ABSOLUTE_NEGX;
                        break;

                        case mso_sptWedgeRectCallout :
                        case mso_sptWedgeRRectCallout :
                        case mso_sptCloudCallout :
                        case mso_sptWedgeEllipseCallout :
                        {
                            if (i == 0)
                                nMode |= CustomShapeHandleModes::RESIZE_FIXED;
                        }
                        break;

                        case mso_sptBorderCallout1 :            // 2 diag
                        {
                            if (i == 0)
                                nMode |= CustomShapeHandleModes::RESIZE_FIXED | CustomShapeHandleModes::CREATE_FIXED;
                            else if (i == 1)
                                nMode |= CustomShapeHandleModes::RESIZE_ABSOLUTE_X | CustomShapeHandleModes::RESIZE_ABSOLUTE_Y | CustomShapeHandleModes::MOVE_SHAPE;
                        }
                        break;
                        case mso_sptBorderCallout2 :            // 3
                        {
                            if (i == 0)
                                nMode |= CustomShapeHandleModes::RESIZE_FIXED | CustomShapeHandleModes::CREATE_FIXED;
                            else if (i == 2)
                                nMode |= CustomShapeHandleModes::RESIZE_ABSOLUTE_X | CustomShapeHandleModes::RESIZE_ABSOLUTE_Y | CustomShapeHandleModes::MOVE_SHAPE;
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
                            if (i == 0)
                                nMode |= CustomShapeHandleModes::RESIZE_FIXED | CustomShapeHandleModes::CREATE_FIXED;
                        }
                        break;
                        default: break;
                    }
                    aSdrCustomShapeInteraction.nMode = nMode;
                    aRet.push_back( aSdrCustomShapeInteraction );
                }
            }
        }
    }
    catch( const uno::RuntimeException& )
    {
    }
    return aRet;
}


// BaseProperties section
#define DEFAULT_MINIMUM_SIGNED_COMPARE  (sal_Int32(0x80000000))
#define DEFAULT_MAXIMUM_SIGNED_COMPARE  (sal_Int32(0x7fffffff))

static sal_Int32 GetNumberOfProperties ( const SvxMSDffHandle* pData )
{
    sal_Int32           nPropertiesNeeded=1;  // position is always needed
    SvxMSDffHandleFlags nFlags = pData->nFlags;

    if ( nFlags & SvxMSDffHandleFlags::MIRRORED_X )
        nPropertiesNeeded++;
    if ( nFlags & SvxMSDffHandleFlags::MIRRORED_Y )
        nPropertiesNeeded++;
    if ( nFlags & SvxMSDffHandleFlags::SWITCHED )
        nPropertiesNeeded++;
    if ( nFlags & SvxMSDffHandleFlags::POLAR )
    {
        nPropertiesNeeded++;
        if ( nFlags & SvxMSDffHandleFlags::RADIUS_RANGE )
        {
            if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
                nPropertiesNeeded++;
            if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
                nPropertiesNeeded++;
        }
    }
    else if ( nFlags & SvxMSDffHandleFlags::RANGE )
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

static void lcl_ShapePropertiesFromDFF( const SvxMSDffHandle* pData, css::beans::PropertyValues& rPropValues )
{
    SvxMSDffHandleFlags nFlags = pData->nFlags;
    sal_Int32 n=0;

    // POSITION
    {
        css::drawing::EnhancedCustomShapeParameterPair aPosition;
        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.First, pData->nPositionX, true, true );
        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.Second, pData->nPositionY, true, false );
        rPropValues[ n ].Name = "Position";
        rPropValues[ n++ ].Value <<= aPosition;
    }
    if ( nFlags & SvxMSDffHandleFlags::MIRRORED_X )
    {
        rPropValues[ n ].Name = "MirroredX";
        rPropValues[ n++ ].Value <<= true;
    }
    if ( nFlags & SvxMSDffHandleFlags::MIRRORED_Y )
    {
        rPropValues[ n ].Name = "MirroredY";
        rPropValues[ n++ ].Value <<= true;
    }
    if ( nFlags & SvxMSDffHandleFlags::SWITCHED )
    {
        rPropValues[ n ].Name = "Switched";
        rPropValues[ n++ ].Value <<= true;
    }
    if ( nFlags & SvxMSDffHandleFlags::POLAR )
    {
        css::drawing::EnhancedCustomShapeParameterPair aCenter;
        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aCenter.First, pData->nCenterX,
                           bool( nFlags & SvxMSDffHandleFlags::CENTER_X_IS_SPECIAL ), true  );
        EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aCenter.Second, pData->nCenterY,
                           bool( nFlags & SvxMSDffHandleFlags::CENTER_Y_IS_SPECIAL ), false );
        rPropValues[ n ].Name = "Polar";
        rPropValues[ n++ ].Value <<= aCenter;
        if ( nFlags & SvxMSDffHandleFlags::RADIUS_RANGE )
        {
            if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
            {
                css::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMinimum, pData->nRangeXMin,
                           bool( nFlags & SvxMSDffHandleFlags::RANGE_X_MIN_IS_SPECIAL ), true  );
                rPropValues[ n ].Name = "RadiusRangeMinimum";
                rPropValues[ n++ ].Value <<= aRadiusRangeMinimum;
            }
            if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
            {
                css::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
                EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMaximum, pData->nRangeXMax,
                           bool( nFlags & SvxMSDffHandleFlags::RANGE_X_MAX_IS_SPECIAL ), false );
                rPropValues[ n ].Name = "RadiusRangeMaximum";
                rPropValues[ n++ ].Value <<= aRadiusRangeMaximum;
            }
        }
    }
    else if ( nFlags & SvxMSDffHandleFlags::RANGE )
    {
        if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
        {
            css::drawing::EnhancedCustomShapeParameter aRangeXMinimum;
            EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMinimum, pData->nRangeXMin,
                           bool( nFlags & SvxMSDffHandleFlags::RANGE_X_MIN_IS_SPECIAL ), true  );
            rPropValues[ n ].Name = "RangeXMinimum";
            rPropValues[ n++ ].Value <<= aRangeXMinimum;
        }
        if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
        {
            css::drawing::EnhancedCustomShapeParameter aRangeXMaximum;
            EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMaximum, pData->nRangeXMax,
                           bool( nFlags & SvxMSDffHandleFlags::RANGE_X_MAX_IS_SPECIAL ), false );
            rPropValues[ n ].Name = "RangeXMaximum";
            rPropValues[ n++ ].Value <<= aRangeXMaximum;
        }
        if ( pData->nRangeYMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
        {
            css::drawing::EnhancedCustomShapeParameter aRangeYMinimum;
            EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMinimum, pData->nRangeYMin,
                             bool( nFlags & SvxMSDffHandleFlags::RANGE_Y_MIN_IS_SPECIAL ), true );
            rPropValues[ n ].Name = "RangeYMinimum";
            rPropValues[ n++ ].Value <<= aRangeYMinimum;
        }
        if ( pData->nRangeYMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
        {
            css::drawing::EnhancedCustomShapeParameter aRangeYMaximum;
            EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMaximum, pData->nRangeYMax,
                             bool( nFlags & SvxMSDffHandleFlags::RANGE_Y_MAX_IS_SPECIAL ), false );
            rPropValues[ n ].Name = "RangeYMaximum";
            rPropValues[ n++ ].Value <<= aRangeYMaximum;
        }
    }
}

std::unique_ptr<sdr::properties::BaseProperties> SdrObjCustomShape::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::CustomShapeProperties>(*this);
}

SdrObjCustomShape::SdrObjCustomShape(SdrModel& rSdrModel)
:   SdrTextObj(rSdrModel)
    , fObjectRotation(0.0)
    , mbAdjustingTextFrameWidthAndHeight(false)
    , mpLastShadowGeometry(nullptr)
{
    bClosedObj = true; // custom shapes may be filled
    bTextFrame = true;
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
    SdrCustomShapeGeometryItem aGeometryItem( GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
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

    const sal_Int32* pDefData = nullptr;
    const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eSpType );
    if ( pDefCustomShape )
        pDefData = pDefCustomShape->pDefData;

    css::uno::Sequence< css::drawing::EnhancedCustomShapeAdjustmentValue > seqAdjustmentValues;


    // AdjustmentValues

    const OUString sAdjustmentValues( "AdjustmentValues" );
    const Any* pAny = aGeometryItem.GetPropertyValueByName( sAdjustmentValues );
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
                seqAdjustmentValues[ i ].State = css::beans::PropertyState_DIRECT_VALUE;
            }
        }
        // check if there are defaulted adjustment values that should be filled the hard coded defaults (pDefValue)
        sal_Int32 nCount = std::min(nAdjustmentValues, nAdjustmentDefaults);
        for ( i = 0; i < nCount; i++ )
        {
            if ( seqAdjustmentValues[ i ].State != css::beans::PropertyState_DIRECT_VALUE )
            {
                seqAdjustmentValues[ i ].Value <<= pDefData[ i ];
                seqAdjustmentValues[ i ].State = css::beans::PropertyState_DIRECT_VALUE;
            }
        }
    }
    aPropVal.Name = sAdjustmentValues;
    aPropVal.Value <<= seqAdjustmentValues;
    aGeometryItem.SetPropertyValue( aPropVal );


    // Coordsize

    const OUString sViewBox( "ViewBox" );
    const Any* pViewBox = aGeometryItem.GetPropertyValueByName( sViewBox );
    css::awt::Rectangle aViewBox;
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


    // Path/Coordinates

    const OUString sCoordinates( "Coordinates" );
    pAny = aGeometryItem.GetPropertyValueByName( sPath, sCoordinates );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nVertices && pDefCustomShape->pVertices )
    {
        css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair> seqCoordinates;

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

    // Path/GluePoints
    const OUString sGluePoints( "GluePoints" );
    pAny = aGeometryItem.GetPropertyValueByName( sPath, sGluePoints );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nGluePoints && pDefCustomShape->pGluePoints )
    {
        css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair> seqGluePoints;
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

    // Path/Segments
    const OUString sSegments( "Segments" );
    pAny = aGeometryItem.GetPropertyValueByName( sPath, sSegments );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nElements && pDefCustomShape->pElements )
    {
        css::uno::Sequence< css::drawing::EnhancedCustomShapeSegment > seqSegments;

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

    // Path/StretchX
    const OUString sStretchX( "StretchX" );
    pAny = aGeometryItem.GetPropertyValueByName( sPath, sStretchX );
    if ( !pAny && pDefCustomShape )
    {
        sal_Int32 nXRef = pDefCustomShape->nXRef;
        if ( nXRef != DEFAULT_MINIMUM_SIGNED_COMPARE )
        {
            aPropVal.Name = sStretchX;
            aPropVal.Value <<= nXRef;
            aGeometryItem.SetPropertyValue( sPath, aPropVal );
        }
    }

    // Path/StretchY
    const OUString sStretchY( "StretchY" );
    pAny = aGeometryItem.GetPropertyValueByName( sPath, sStretchY );
    if ( !pAny && pDefCustomShape )
    {
        sal_Int32 nYRef = pDefCustomShape->nYRef;
        if ( nYRef != DEFAULT_MINIMUM_SIGNED_COMPARE )
        {
            aPropVal.Name = sStretchY;
            aPropVal.Value <<= nYRef;
            aGeometryItem.SetPropertyValue( sPath, aPropVal );
        }
    }

    // Path/TextFrames
    const OUString sTextFrames( "TextFrames" );
    pAny = aGeometryItem.GetPropertyValueByName( sPath, sTextFrames );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nTextRect && pDefCustomShape->pTextRect )
    {
        css::uno::Sequence< css::drawing::EnhancedCustomShapeTextFrame > seqTextFrames;

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

    // Equations
    const OUString sEquations(  "Equations"  );
    pAny = aGeometryItem.GetPropertyValueByName( sEquations );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nCalculation && pDefCustomShape->pCalculation )
    {
        css::uno::Sequence< OUString > seqEquations;

        sal_Int32 i, nCount = pDefCustomShape->nCalculation;
        seqEquations.realloc( nCount );
        const SvxMSDffCalculationData* pData = pDefCustomShape->pCalculation;
        for ( i = 0; i < nCount; i++, pData++ )
            seqEquations[ i ] = EnhancedCustomShape2d::GetEquation( pData->nFlags, pData->nVal[ 0 ], pData->nVal[ 1 ], pData->nVal[ 2 ] );
        aPropVal.Name = sEquations;
        aPropVal.Value <<= seqEquations;
        aGeometryItem.SetPropertyValue( aPropVal );
    }

    // Handles
    const OUString sHandles(  "Handles"  );
    pAny = aGeometryItem.GetPropertyValueByName( sHandles );
    if ( !pAny && pDefCustomShape && pDefCustomShape->nHandles && pDefCustomShape->pHandles )
    {
        css::uno::Sequence< css::beans::PropertyValues > seqHandles;

        sal_Int32 i, nCount = pDefCustomShape->nHandles;
        const SvxMSDffHandle* pData = pDefCustomShape->pHandles;
        seqHandles.realloc( nCount );
        for ( i = 0; i < nCount; i++, pData++ )
        {
            sal_Int32 nPropertiesNeeded;
            css::beans::PropertyValues& rPropValues = seqHandles[ i ];
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

bool SdrObjCustomShape::IsDefaultGeometry( const DefaultType eDefaultType ) const
{
    bool bIsDefaultGeometry = false;

    PropertyValue aPropVal;
    OUString sShapeType;
    const OUString sType( "Type" );
    const SdrCustomShapeGeometryItem aGeometryItem( GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

    const Any *pAny = aGeometryItem.GetPropertyValueByName( sType );
    if ( pAny )
        *pAny >>= sShapeType;

    MSO_SPT eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

    const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eSpType );
    const OUString sPath( "Path" );
    switch( eDefaultType )
    {
        case DefaultType::Viewbox :
        {
            const OUString sViewBox( "ViewBox" );
            const Any* pViewBox = const_cast<SdrCustomShapeGeometryItem&>(aGeometryItem).GetPropertyValueByName( sViewBox );
            css::awt::Rectangle aViewBox;
            if ( pViewBox && ( *pViewBox >>= aViewBox ) )
            {
                if ( ( aViewBox.Width == pDefCustomShape->nCoordWidth )
                    && ( aViewBox.Height == pDefCustomShape->nCoordHeight ) )
                    bIsDefaultGeometry = true;
            }
        }
        break;

        case DefaultType::Path :
        {
            const OUString sCoordinates( "Coordinates" );
            pAny = const_cast<SdrCustomShapeGeometryItem&>(aGeometryItem).GetPropertyValueByName( sPath, sCoordinates );
            if ( pAny && pDefCustomShape && pDefCustomShape->nVertices && pDefCustomShape->pVertices )
            {
                css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair> seqCoordinates1, seqCoordinates2;
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
            else if ( pDefCustomShape && ( ( pDefCustomShape->nVertices == 0 ) || ( pDefCustomShape->pVertices == nullptr ) ) )
                bIsDefaultGeometry = true;
        }
        break;

        case DefaultType::Gluepoints :
        {
            const OUString sGluePoints( "GluePoints" );
            pAny = const_cast<SdrCustomShapeGeometryItem&>(aGeometryItem).GetPropertyValueByName( sPath, sGluePoints );
            if ( pAny && pDefCustomShape && pDefCustomShape->nGluePoints && pDefCustomShape->pGluePoints )
            {
                css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair> seqGluePoints1, seqGluePoints2;
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

        case DefaultType::Segments :
        {
            // Path/Segments
            const OUString sSegments( "Segments" );
            pAny = const_cast<SdrCustomShapeGeometryItem&>(aGeometryItem).GetPropertyValueByName( sPath, sSegments );
            if ( pAny )
            {
                css::uno::Sequence< css::drawing::EnhancedCustomShapeSegment > seqSegments1, seqSegments2;
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
                                bIsDefaultGeometry = true;
                        }
                    }
                    else
                    {
                        // check if it's the default segment description ( M L Z N )
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
            else if ( pDefCustomShape && ( ( pDefCustomShape->nElements == 0 ) || ( pDefCustomShape->pElements == nullptr ) ) )
                bIsDefaultGeometry = true;
        }
        break;

        case DefaultType::StretchX :
        {
            const OUString sStretchX( "StretchX" );
            pAny = const_cast<SdrCustomShapeGeometryItem&>(aGeometryItem).GetPropertyValueByName( sPath, sStretchX );
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

        case DefaultType::StretchY :
        {
            const OUString sStretchY( "StretchY" );
            pAny = const_cast<SdrCustomShapeGeometryItem&>(aGeometryItem).GetPropertyValueByName( sPath, sStretchY );
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

        case DefaultType::Equations :
        {
            const OUString sEquations(  "Equations"  );
            pAny = const_cast<SdrCustomShapeGeometryItem&>(aGeometryItem).GetPropertyValueByName( sEquations );
            if ( pAny && pDefCustomShape && pDefCustomShape->nCalculation && pDefCustomShape->pCalculation )
            {
                css::uno::Sequence< OUString > seqEquations1, seqEquations2;
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
            else if ( pDefCustomShape && ( ( pDefCustomShape->nCalculation == 0 ) || ( pDefCustomShape->pCalculation == nullptr ) ) )
                bIsDefaultGeometry = true;
        }
        break;

        case DefaultType::TextFrames :
        {
            const OUString sTextFrames(  "TextFrames"  );
            pAny = const_cast<SdrCustomShapeGeometryItem&>(aGeometryItem).GetPropertyValueByName( sPath, sTextFrames );
            if ( pAny && pDefCustomShape && pDefCustomShape->nTextRect && pDefCustomShape->pTextRect )
            {
                css::uno::Sequence< css::drawing::EnhancedCustomShapeTextFrame > seqTextFrames1, seqTextFrames2;
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
            else if ( pDefCustomShape && ( ( pDefCustomShape->nTextRect == 0 ) || ( pDefCustomShape->pTextRect == nullptr ) ) )
                bIsDefaultGeometry = true;
        }
        break;
    }
    return bIsDefaultGeometry;
}

void SdrObjCustomShape::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bResizeFreeAllowed=fObjectRotation == 0.0;
    rInfo.bResizePropAllowed=true;
    rInfo.bRotateFreeAllowed=true;
    rInfo.bRotate90Allowed  =true;
    rInfo.bMirrorFreeAllowed=true;
    rInfo.bMirror45Allowed  =true;
    rInfo.bMirror90Allowed  =true;
    rInfo.bTransparenceAllowed = false;
    rInfo.bShearAllowed     =true;
    rInfo.bEdgeRadiusAllowed=false;
    rInfo.bNoContortion     =true;

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

sal_uInt16 SdrObjCustomShape::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_CUSTOMSHAPE);
}

// #115391# This implementation is based on the TextFrame size of the CustomShape and the
// state of the ResizeShapeToFitText flag to correctly set TextMinFrameWidth/Height
void SdrObjCustomShape::AdaptTextMinSize()
{
    if (!getSdrModelFromSdrObject().IsCreatingDataObj() && !getSdrModelFromSdrObject().IsPasteResize())
    {
        const bool bResizeShapeToFitText(GetObjectItem(SDRATTR_TEXT_AUTOGROWHEIGHT).GetValue());
        SfxItemSet aSet(
            *GetObjectItemSet().GetPool(),
            svl::Items<SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT,
            SDRATTR_TEXT_MINFRAMEWIDTH, SDRATTR_TEXT_AUTOGROWWIDTH>{}); // contains SDRATTR_TEXT_MAXFRAMEWIDTH
        bool bChanged(false);

        if(bResizeShapeToFitText)
        {
            // always reset MinWidthHeight to zero to only rely on text size and frame size
            // to allow resizing being completely dependent on text size only
            aSet.Put(makeSdrTextMinFrameWidthItem(0));
            aSet.Put(makeSdrTextMinFrameHeightItem(0));
            bChanged = true;
        }
        else
        {
            // recreate from CustomShape-specific TextBounds
            tools::Rectangle aTextBound(maRect);

            if(GetTextBounds(aTextBound))
            {
                const long nHDist(GetTextLeftDistance() + GetTextRightDistance());
                const long nVDist(GetTextUpperDistance() + GetTextLowerDistance());
                const long nTWdt(std::max(long(0), static_cast<long>(aTextBound.GetWidth() - 1 - nHDist)));
                const long nTHgt(std::max(long(0), static_cast<long>(aTextBound.GetHeight() - 1 - nVDist)));

                aSet.Put(makeSdrTextMinFrameWidthItem(nTWdt));
                aSet.Put(makeSdrTextMinFrameHeightItem(nTHgt));
                bChanged = true;
            }
        }

        if(bChanged)
            SetObjectItemSet(aSet);
    }
}

void SdrObjCustomShape::NbcSetSnapRect( const tools::Rectangle& rRect )
{
    maRect = rRect;
    ImpJustifyRect(maRect);
    InvalidateRenderGeometry();

    AdaptTextMinSize();

    ImpCheckShear();
    SetRectsDirty();
    SetChanged();
}

void SdrObjCustomShape::SetSnapRect( const tools::Rectangle& rRect )
{
    tools::Rectangle aBoundRect0;
    if ( pUserCall )
        aBoundRect0 = GetLastBoundRect();
    NbcSetSnapRect( rRect );
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrObjCustomShape::NbcSetLogicRect( const tools::Rectangle& rRect )
{
    maRect = rRect;
    ImpJustifyRect(maRect);
    InvalidateRenderGeometry();

    AdaptTextMinSize();

    SetRectsDirty();
    SetChanged();
}

void SdrObjCustomShape::SetLogicRect( const tools::Rectangle& rRect )
{
    tools::Rectangle aBoundRect0;
    if ( pUserCall )
        aBoundRect0 = GetLastBoundRect();
    NbcSetLogicRect(rRect);
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrObjCustomShape::Move( const Size& rSiz )
{
    if ( rSiz.Width() || rSiz.Height() )
    {
        tools::Rectangle aBoundRect0;
        if ( pUserCall )
            aBoundRect0 = GetLastBoundRect();
        NbcMove(rSiz);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::MoveOnly,aBoundRect0);
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

void SdrObjCustomShape::NbcResize( const Point& rRef, const Fraction& rxFact, const Fraction& ryFact )
{
    // taking care of handles that should not been changed
    tools::Rectangle aOld( maRect );
    std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles() );

    SdrTextObj::NbcResize( rRef, rxFact, ryFact );

    if ( ( rxFact.GetNumerator() != rxFact.GetDenominator() )
        || ( ryFact.GetNumerator()!= ryFact.GetDenominator() ) )
    {
        if ( ( ( rxFact.GetNumerator() < 0 ) && ( rxFact.GetDenominator() > 0 ) ) ||
            ( ( rxFact.GetNumerator() > 0 ) && ( rxFact.GetDenominator() < 0 ) ) )
        {
            SetMirroredX( !IsMirroredX() );
        }
        if ( ( ( ryFact.GetNumerator() < 0 ) && ( ryFact.GetDenominator() > 0 ) ) ||
            ( ( ryFact.GetNumerator() > 0 ) && ( ryFact.GetDenominator() < 0 ) ) )
        {
            SetMirroredY( !IsMirroredY() );
        }
    }

    for (const auto& rInteraction : aInteractionHandles)
    {
        try
        {
            if ( rInteraction.nMode & CustomShapeHandleModes::RESIZE_FIXED )
                rInteraction.xInteraction->setControllerPosition( rInteraction.aPosition );
            if ( rInteraction.nMode & CustomShapeHandleModes::RESIZE_ABSOLUTE_X )
            {
                sal_Int32 nX = ( rInteraction.aPosition.X - aOld.Left() ) + maRect.Left();
                rInteraction.xInteraction->setControllerPosition( css::awt::Point( nX, rInteraction.xInteraction->getPosition().Y ) );
            }
            else if ( rInteraction.nMode & CustomShapeHandleModes::RESIZE_ABSOLUTE_NEGX )
            {
                sal_Int32 nX = maRect.Right() - (aOld.Right() - rInteraction.aPosition.X);
                rInteraction.xInteraction->setControllerPosition( css::awt::Point( nX, rInteraction.xInteraction->getPosition().Y ) );
            }
            if ( rInteraction.nMode & CustomShapeHandleModes::RESIZE_ABSOLUTE_Y )
            {
                sal_Int32 nY = ( rInteraction.aPosition.Y - aOld.Top() ) + maRect.Top();
                rInteraction.xInteraction->setControllerPosition( css::awt::Point( rInteraction.xInteraction->getPosition().X, nY ) );
            }
        }
        catch ( const uno::RuntimeException& )
        {
        }
    }
    InvalidateRenderGeometry();
}

void SdrObjCustomShape::NbcRotate( const Point& rRef, long nAngle, double sn, double cs )
{
    bool bMirroredX = IsMirroredX();
    bool bMirroredY = IsMirroredY();

    fObjectRotation = fmod( fObjectRotation, 360.0 );
    if ( fObjectRotation < 0 )
        fObjectRotation = 360 + fObjectRotation;

    // the rotation angle for ashapes is stored in fObjectRotation, this rotation
    // has to be applied to the text object (which is internally using aGeo.nAngle).
    SdrTextObj::NbcRotate( maRect.TopLeft(), -aGeo.nRotationAngle,        // retrieving the unrotated text object
                            sin( (-aGeo.nRotationAngle) * F_PI18000 ),
                            cos( (-aGeo.nRotationAngle) * F_PI18000 ) );
    aGeo.nRotationAngle = 0;                                             // resetting aGeo data
    aGeo.RecalcSinCos();

    long nW = static_cast<long>( fObjectRotation * 100 );                      // applying our object rotation
    if ( bMirroredX )
        nW = 36000 - nW;
    if ( bMirroredY )
        nW = 18000 - nW;
    nW = nW % 36000;
    if ( nW < 0 )
        nW = 36000 + nW;
    SdrTextObj::NbcRotate( maRect.TopLeft(), nW,                     // applying text rotation
                            sin( nW * F_PI18000 ),
                            cos( nW * F_PI18000 ) );

    int nSwap = 0;
    if ( bMirroredX )
        nSwap ^= 1;
    if ( bMirroredY )
        nSwap ^= 1;

    double fAngle = nAngle;                                                   // updating to our new object rotation
    fAngle /= 100.0;
    fObjectRotation = fmod( nSwap ? fObjectRotation - fAngle : fObjectRotation + fAngle, 360.0 );
    if ( fObjectRotation < 0 )
        fObjectRotation = 360 + fObjectRotation;

    SdrTextObj::NbcRotate( rRef, nAngle, sn, cs );                           // applying text rotation
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
            long nTextObjRotation = aGeo.nRotationAngle;
            double fAngle = nTextObjRotation;

            fAngle /= 100.0;

            bool bSingleFlip = (IsMirroredX()!= IsMirroredY());

            fObjectRotation = fmod( bSingleFlip ? -fAngle : fAngle, 360.0 );

            if ( fObjectRotation < 0 )
            {
                fObjectRotation = 360.0 + fObjectRotation;
            }
         }
    }

    InvalidateRenderGeometry();
}

void SdrObjCustomShape::Shear( const Point& rRef, long nAngle, double tn, bool bVShear )
{
    SdrTextObj::Shear( rRef, nAngle, tn, bVShear );
    InvalidateRenderGeometry();
}
void SdrObjCustomShape::NbcShear( const Point& rRef, long nAngle, double tn, bool bVShear )
{
    // TTTT: Fix for old mirroring, can be removed again in aw080
    SdrTextObj::NbcShear(rRef,nAngle,tn,bVShear);

    // updating fObjectRotation
    long nTextObjRotation = aGeo.nRotationAngle;
    double fAngle = nTextObjRotation;

    fAngle /= 100.0;

    bool bSingleFlip = (IsMirroredX()!= IsMirroredY());

    fObjectRotation = fmod( bSingleFlip ? -fAngle : fAngle, 360.0 );

    if ( fObjectRotation < 0 )
    {
        fObjectRotation = 360.0 + fObjectRotation;
    }

    InvalidateRenderGeometry();
}


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
        case 0: aPt=maRect.TopCenter();    aPt.AdjustY( -nWdt ); break;
        case 1: aPt=maRect.RightCenter();  aPt.AdjustX(nWdt ); break;
        case 2: aPt=maRect.BottomCenter(); aPt.AdjustY(nWdt ); break;
        case 3: aPt=maRect.LeftCenter();   aPt.AdjustX( -nWdt ); break;
    }
    if (aGeo.nShearAngle!=0) ShearPoint(aPt,maRect.TopLeft(),aGeo.nTan);
    if (aGeo.nRotationAngle!=0) RotatePoint(aPt,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    aPt-=GetSnapRect().Center();
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(false);
    return aGP;
}


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
                    aCopy.SetUserDefined(false);
                    aNewList.Insert(aCopy);
                }

                bool bMirroredX = IsMirroredX();
                bool bMirroredY = IsMirroredY();

                long nShearAngle = aGeo.nShearAngle;
                double fTan = aGeo.nTan;

                if ( aGeo.nRotationAngle || nShearAngle || bMirroredX || bMirroredY )
                {
                    tools::Polygon aPoly( maRect );
                    if( nShearAngle )
                    {
                        sal_uInt16 nPointCount=aPoly.GetSize();
                        for (sal_uInt16 i=0; i<nPointCount; i++)
                            ShearPoint(aPoly[i],maRect.Center(), fTan );
                    }
                    if ( aGeo.nRotationAngle )
                        aPoly.Rotate( maRect.Center(), aGeo.nRotationAngle / 10 );

                    tools::Rectangle aBoundRect( aPoly.GetBoundRect() );
                    sal_Int32 nXDiff = aBoundRect.Left() - maRect.Left();
                    sal_Int32 nYDiff = aBoundRect.Top() - maRect.Top();

                    if (nShearAngle && bMirroredX != bMirroredY)
                    {
                        nShearAngle = -nShearAngle;
                        fTan = -fTan;
                    }

                    Point aRef( maRect.GetWidth() / 2, maRect.GetHeight() / 2 );
                    for ( a = 0; a < aNewList.GetCount(); a++ )
                    {
                        SdrGluePoint& rPoint = aNewList[ a ];
                        Point aGlue( rPoint.GetPos() );
                        if ( nShearAngle )
                            ShearPoint( aGlue, aRef, fTan );

                        RotatePoint(aGlue, aRef, sin(basegfx::deg2rad(fObjectRotation)),
                                    cos(basegfx::deg2rad(fObjectRotation)));
                        if ( bMirroredX )
                            aGlue.setX( maRect.GetWidth() - aGlue.X() );
                        if ( bMirroredY )
                            aGlue.setY( maRect.GetHeight() - aGlue.Y() );
                        aGlue.AdjustX( -nXDiff );
                        aGlue.AdjustY( -nYDiff );
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
    const_cast<SdrObjCustomShape*>(this)->ImpCheckCustomGluePointsAreAdded();
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
        return nullptr;
    }
}


sal_uInt32 SdrObjCustomShape::GetHdlCount() const
{
    const sal_uInt32 nBasicHdlCount(SdrTextObj::GetHdlCount());
    return ( GetInteractionHandles().size() + nBasicHdlCount );
}

void SdrObjCustomShape::AddToHdlList(SdrHdlList& rHdlList) const
{
    SdrTextObj::AddToHdlList(rHdlList);

    int nCustomShapeHdlNum = 0;
    for (SdrCustomShapeInteraction const & rInteraction : GetInteractionHandles())
    {
        if ( rInteraction.xInteraction.is() )
        {
            try
            {
                css::awt::Point aPosition( rInteraction.xInteraction->getPosition() );
                std::unique_ptr<SdrHdl> pH(new SdrHdl( Point( aPosition.X, aPosition.Y ), SdrHdlKind::CustomShape1 ));
                pH->SetPointNum( nCustomShapeHdlNum );
                pH->SetObj( const_cast<SdrObjCustomShape*>(this) );
                rHdlList.AddHdl(std::move(pH));
            }
            catch ( const uno::RuntimeException& )
            {
            }
        }
        ++nCustomShapeHdlNum;
    }
}

bool SdrObjCustomShape::hasSpecialDrag() const
{
    return true;
}

bool SdrObjCustomShape::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl = rDrag.GetHdl();

    if(pHdl && SdrHdlKind::CustomShape1 == pHdl->GetKind())
    {
        rDrag.SetEndDragChangesAttributes(true);
        rDrag.SetNoSnap();
    }
    else
    {
        const SdrHdl* pHdl2 = rDrag.GetHdl();
        const SdrHdlKind eHdl((pHdl2 == nullptr) ? SdrHdlKind::Move : pHdl2->GetKind());

        switch( eHdl )
        {
            case SdrHdlKind::UpperLeft :
            case SdrHdlKind::Upper :
            case SdrHdlKind::UpperRight :
            case SdrHdlKind::Left  :
            case SdrHdlKind::Right :
            case SdrHdlKind::LowerLeft :
            case SdrHdlKind::Lower :
            case SdrHdlKind::LowerRight :
            case SdrHdlKind::Move  :
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

void SdrObjCustomShape::DragResizeCustomShape( const tools::Rectangle& rNewRect )
{
    tools::Rectangle   aOld( maRect );
    bool    bOldMirroredX( IsMirroredX() );
    bool    bOldMirroredY( IsMirroredY() );

    tools::Rectangle aNewRect( rNewRect );
    aNewRect.Justify();

    std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles() );

    GeoStat aGeoStat( GetGeoStat() );
    if ( aNewRect.TopLeft()!= maRect.TopLeft() &&
        ( aGeo.nRotationAngle || aGeo.nShearAngle ) )
    {
        Point aNewPos( aNewRect.TopLeft() );
        if ( aGeo.nShearAngle ) ShearPoint( aNewPos, aOld.TopLeft(), aGeoStat.nTan );
        if ( aGeo.nRotationAngle )  RotatePoint(aNewPos, aOld.TopLeft(), aGeoStat.nSin, aGeoStat.nCos );
        aNewRect.SetPos( aNewPos );
    }
    if ( aNewRect != maRect )
    {
        SetLogicRect( aNewRect );
        InvalidateRenderGeometry();

        if ( rNewRect.Left() > rNewRect.Right() )
        {
            Point aTop( ( GetSnapRect().Left() + GetSnapRect().Right() ) >> 1, GetSnapRect().Top() );
            Point aBottom( aTop.X(), aTop.Y() + 1000 );
            NbcMirror( aTop, aBottom );
        }
        if ( rNewRect.Top() > rNewRect.Bottom() )
        {
            Point aLeft( GetSnapRect().Left(), ( GetSnapRect().Top() + GetSnapRect().Bottom() ) >> 1 );
            Point aRight( aLeft.X() + 1000, aLeft.Y() );
            NbcMirror( aLeft, aRight );
        }

        for (const auto& rInteraction : aInteractionHandles)
        {
            try
            {
                if ( rInteraction.nMode & CustomShapeHandleModes::RESIZE_FIXED )
                    rInteraction.xInteraction->setControllerPosition( rInteraction.aPosition );
                if ( rInteraction.nMode & CustomShapeHandleModes::RESIZE_ABSOLUTE_X ||
                     rInteraction.nMode & CustomShapeHandleModes::RESIZE_ABSOLUTE_NEGX )
                {
                    if (rInteraction.nMode & CustomShapeHandleModes::RESIZE_ABSOLUTE_NEGX)
                        bOldMirroredX = !bOldMirroredX;

                    sal_Int32 nX;
                    if ( bOldMirroredX )
                    {
                        nX = ( rInteraction.aPosition.X - aOld.Right() );
                        if ( rNewRect.Left() > rNewRect.Right() )
                            nX = maRect.Left() - nX;
                        else
                            nX += maRect.Right();
                    }
                    else
                    {
                        nX = ( rInteraction.aPosition.X - aOld.Left() );
                        if ( rNewRect.Left() > rNewRect.Right() )
                            nX = maRect.Right() - nX;
                        else
                            nX += maRect.Left();
                    }
                    rInteraction.xInteraction->setControllerPosition( css::awt::Point( nX, rInteraction.xInteraction->getPosition().Y ) );
                }
                if ( rInteraction.nMode & CustomShapeHandleModes::RESIZE_ABSOLUTE_Y )
                {
                    sal_Int32 nY;
                    if ( bOldMirroredY )
                    {
                        nY = ( rInteraction.aPosition.Y - aOld.Bottom() );
                        if ( rNewRect.Top() > rNewRect.Bottom() )
                            nY = maRect.Top() - nY;
                        else
                            nY += maRect.Bottom();
                    }
                    else
                    {
                        nY = ( rInteraction.aPosition.Y - aOld.Top() );
                        if ( rNewRect.Top() > rNewRect.Bottom() )
                            nY = maRect.Bottom() - nY;
                        else
                            nY += maRect.Top();
                    }
                    rInteraction.xInteraction->setControllerPosition( css::awt::Point( rInteraction.xInteraction->getPosition().X, nY ) );
                }
            }
            catch ( const uno::RuntimeException& )
            {
            }
        }
    }
}

void SdrObjCustomShape::DragMoveCustomShapeHdl( const Point& rDestination,
        const sal_uInt16 nCustomShapeHdlNum, bool bMoveCalloutRectangle )
{
    std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles() );
    if ( nCustomShapeHdlNum < aInteractionHandles.size() )
    {
        SdrCustomShapeInteraction aInteractionHandle( aInteractionHandles[ nCustomShapeHdlNum ] );
        if ( aInteractionHandle.xInteraction.is() )
        {
            try
            {
                css::awt::Point aPt( rDestination.X(), rDestination.Y() );
                if ( aInteractionHandle.nMode & CustomShapeHandleModes::MOVE_SHAPE && bMoveCalloutRectangle )
                {
                    sal_Int32 nXDiff = aPt.X - aInteractionHandle.aPosition.X;
                    sal_Int32 nYDiff = aPt.Y - aInteractionHandle.aPosition.Y;

                    maRect.Move( nXDiff, nYDiff );
                    aOutRect.Move( nXDiff, nYDiff );
                    maSnapRect.Move( nXDiff, nYDiff );
                    SetRectsDirty(true);
                    InvalidateRenderGeometry();

                    for (const auto& rInteraction : aInteractionHandles)
                    {
                        if ( rInteraction.nMode & CustomShapeHandleModes::RESIZE_FIXED )
                        {
                            if ( rInteraction.xInteraction.is() )
                                rInteraction.xInteraction->setControllerPosition( rInteraction.aPosition );
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
    const SdrHdlKind eHdl((pHdl == nullptr) ? SdrHdlKind::Move : pHdl->GetKind());

    switch(eHdl)
    {
        case SdrHdlKind::CustomShape1 :
        {
            rDrag.SetEndDragChangesGeoAndAttributes(true);
            DragMoveCustomShapeHdl( rDrag.GetNow(), static_cast<sal_uInt16>(pHdl->GetPointNum()), !rDrag.GetDragMethod()->IsShiftPressed() );
            SetRectsDirty();
            InvalidateRenderGeometry();
            SetChanged();
            break;
        }

        case SdrHdlKind::UpperLeft :
        case SdrHdlKind::Upper :
        case SdrHdlKind::UpperRight :
        case SdrHdlKind::Left  :
        case SdrHdlKind::Right :
        case SdrHdlKind::LowerLeft :
        case SdrHdlKind::Lower :
        case SdrHdlKind::LowerRight :
        {
            DragResizeCustomShape( ImpDragCalcRect(rDrag) );
            break;
        }
        case SdrHdlKind::Move :
        {
            Move(Size(rDrag.GetDX(), rDrag.GetDY()));
            break;
        }
        default: break;
    }

    return true;
}


void SdrObjCustomShape::DragCreateObject( SdrDragStat& rStat )
{
    tools::Rectangle aRect1;
    rStat.TakeCreateRect( aRect1 );

    std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles() );

    sal_uInt32 const nDefaultObjectSizeWidth = 3000;      // default width from SDOptions ?
    sal_uInt32 const nDefaultObjectSizeHeight= 3000;

    if ( ImpVerticalSwitch( *this ) )
    {
        SetMirroredX( aRect1.Left() > aRect1.Right() );

        aRect1 = tools::Rectangle( rStat.GetNow(), Size( nDefaultObjectSizeWidth, nDefaultObjectSizeHeight ) );
        // subtracting the horizontal difference of the latest handle from shape position
        if ( !aInteractionHandles.empty() )
        {
            sal_Int32 nHandlePos = aInteractionHandles[ aInteractionHandles.size() - 1 ].xInteraction->getPosition().X;
            aRect1.Move( maRect.Left() - nHandlePos, 0 );
        }
    }
    ImpJustifyRect( aRect1 );
    rStat.SetActionRect( aRect1 );
    maRect = aRect1;
    SetRectsDirty();

    for (const auto& rInteraction : aInteractionHandles)
    {
        try
        {
            if ( rInteraction.nMode & CustomShapeHandleModes::CREATE_FIXED )
                rInteraction.xInteraction->setControllerPosition( awt::Point( rStat.GetStart().X(), rStat.GetStart().Y() ) );
        }
        catch ( const uno::RuntimeException& )
        {
        }
    }

    SetBoundRectDirty();
    bSnapRectDirty=true;
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
    return true;
}

bool SdrObjCustomShape::EndCreate( SdrDragStat& rStat, SdrCreateCmd eCmd )
{
    DragCreateObject( rStat );

    AdaptTextMinSize();

    SetRectsDirty();
    return ( eCmd == SdrCreateCmd::ForceEnd || rStat.GetPointCount() >= 2 );
}

basegfx::B2DPolyPolygon SdrObjCustomShape::TakeCreatePoly(const SdrDragStat& /*rDrag*/) const
{
    return GetLineGeometry( false );
}


// in context with the SdrObjCustomShape the SdrTextAutoGrowHeightItem == true -> Resize Shape to fit text,
//                                     the SdrTextAutoGrowWidthItem  == true -> Word wrap text in Shape
bool SdrObjCustomShape::IsAutoGrowHeight() const
{
    const SfxItemSet& rSet = GetMergedItemSet();
    bool bIsAutoGrowHeight = rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT).GetValue();
    if ( bIsAutoGrowHeight && IsVerticalWriting() )
        bIsAutoGrowHeight = !rSet.Get(SDRATTR_TEXT_WORDWRAP).GetValue();
    return bIsAutoGrowHeight;
}
bool SdrObjCustomShape::IsAutoGrowWidth() const
{
    const SfxItemSet& rSet = GetMergedItemSet();
    bool bIsAutoGrowWidth = rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT).GetValue();
    if ( bIsAutoGrowWidth && !IsVerticalWriting() )
        bIsAutoGrowWidth = !rSet.Get(SDRATTR_TEXT_WORDWRAP).GetValue();
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
        if(pOutlinerParaObject->IsVertical() != bVertical)
        {
            // get item settings
            const SfxItemSet& rSet = GetObjectItemSet();

            // Also exchange horizontal and vertical adjust items
            SdrTextHorzAdjust eHorz = rSet.Get(SDRATTR_TEXT_HORZADJUST).GetValue();
            SdrTextVertAdjust eVert = rSet.Get(SDRATTR_TEXT_VERTADJUST).GetValue();

            // rescue object size
            tools::Rectangle aObjectRect = GetSnapRect();

            // prepare ItemSet to set exchanged width and height items
            SfxItemSet aNewSet(*rSet.GetPool(),
                svl::Items<SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT,
                // Expanded item ranges to also support horizontal and vertical adjust.
                SDRATTR_TEXT_VERTADJUST, SDRATTR_TEXT_VERTADJUST,
                SDRATTR_TEXT_AUTOGROWWIDTH, SDRATTR_TEXT_HORZADJUST>{});

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

            pOutlinerParaObject = GetOutlinerParaObject();
            if ( pOutlinerParaObject )
                pOutlinerParaObject->SetVertical(bVertical);
            SetObjectItemSet( aNewSet );

            // restore object size
            SetSnapRect(aObjectRect);
        }
    }
}

void SdrObjCustomShape::SuggestTextFrameSize(Size aSuggestedTextFrameSize)
{
    m_aSuggestedTextFrameSize = aSuggestedTextFrameSize;
}

bool SdrObjCustomShape::AdjustTextFrameWidthAndHeight(tools::Rectangle& rR, bool bHgt, bool bWdt) const
{
    // Either we have text or the application has native text and suggested its size to us.
    bool bHasText = HasText() || (m_aSuggestedTextFrameSize.Width() != 0 && m_aSuggestedTextFrameSize.Height() != 0);
    if ( bHasText && !rR.IsEmpty() )
    {
        bool bWdtGrow=bWdt && IsAutoGrowWidth();
        bool bHgtGrow=bHgt && IsAutoGrowHeight();
        if ( bWdtGrow || bHgtGrow )
        {
            tools::Rectangle aR0(rR);
            long nHgt=0,nMinHgt=0,nMaxHgt=0;
            long nWdt=0,nMinWdt=0,nMaxWdt=0;
            Size aSiz(rR.GetSize()); aSiz.AdjustWidth( -1 ); aSiz.AdjustHeight( -1 );
            Size aMaxSiz(100000,100000);
            Size aTmpSiz(getSdrModelFromSdrObject().GetMaxObjSize());
            if (aTmpSiz.Width()!=0) aMaxSiz.setWidth(aTmpSiz.Width() );
            if (aTmpSiz.Height()!=0) aMaxSiz.setHeight(aTmpSiz.Height() );
            if (bWdtGrow)
            {
                nMinWdt=GetMinTextFrameWidth();
                nMaxWdt=GetMaxTextFrameWidth();
                if (nMaxWdt==0 || nMaxWdt>aMaxSiz.Width()) nMaxWdt=aMaxSiz.Width();
                if (nMinWdt<=0) nMinWdt=1;
                aSiz.setWidth(nMaxWdt );
            }
            if (bHgtGrow)
            {
                nMinHgt=GetMinTextFrameHeight();
                nMaxHgt=GetMaxTextFrameHeight();
                if (nMaxHgt==0 || nMaxHgt>aMaxSiz.Height()) nMaxHgt=aMaxSiz.Height();
                if (nMinHgt<=0) nMinHgt=1;
                aSiz.setHeight(nMaxHgt );
            }
            long nHDist=GetTextLeftDistance()+GetTextRightDistance();
            long nVDist=GetTextUpperDistance()+GetTextLowerDistance();
            aSiz.AdjustWidth( -nHDist );
            aSiz.AdjustHeight( -nVDist );
            if ( aSiz.Width() < 2 )
                aSiz.setWidth( 2 );   // minimum size=2
            if ( aSiz.Height() < 2 )
                aSiz.setHeight( 2 ); // minimum size=2

            if (HasText())
            {
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
                    rOutliner.SetUpdateMode(true);
                    // TODO: add the optimization with bPortionInfoChecked again.
                    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
                    if( pOutlinerParaObject != nullptr )
                    {
                        rOutliner.SetText(*pOutlinerParaObject);
                        rOutliner.SetFixedCellHeight(GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT).GetValue());
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
            }
            else
            {
                nHgt = m_aSuggestedTextFrameSize.Height();
                nWdt = m_aSuggestedTextFrameSize.Width();
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
                bWdtGrow = false;
            if ( nHgtGrow == 0 )
                bHgtGrow=false;
            if ( bWdtGrow || bHgtGrow )
            {
                if ( bWdtGrow )
                {
                    SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
                    if ( eHAdj == SDRTEXTHORZADJUST_LEFT )
                        rR.AdjustRight(nWdtGrow );
                    else if ( eHAdj == SDRTEXTHORZADJUST_RIGHT )
                        rR.AdjustLeft( -nWdtGrow );
                    else
                    {
                        long nWdtGrow2=nWdtGrow/2;
                        rR.AdjustLeft( -nWdtGrow2 );
                        rR.SetRight(rR.Left()+nWdt );
                    }
                }
                if ( bHgtGrow )
                {
                    SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
                    if ( eVAdj == SDRTEXTVERTADJUST_TOP )
                        rR.AdjustBottom(nHgtGrow );
                    else if ( eVAdj == SDRTEXTVERTADJUST_BOTTOM )
                        rR.AdjustTop( -nHgtGrow );
                    else
                    {
                        long nHgtGrow2=nHgtGrow/2;
                        rR.AdjustTop( -nHgtGrow2 );
                        rR.SetBottom(rR.Top()+nHgt );
                    }
                }
                if ( aGeo.nRotationAngle )
                {
                    Point aD1(rR.TopLeft());
                    aD1-=aR0.TopLeft();
                    Point aD2(aD1);
                    RotatePoint(aD2,Point(),aGeo.nSin,aGeo.nCos);
                    aD2-=aD1;
                    rR.Move(aD2.X(),aD2.Y());
                }
                return true;
            }
        }
    }
    return false;
}

tools::Rectangle SdrObjCustomShape::ImpCalculateTextFrame( const bool bHgt, const bool bWdt )
{
    tools::Rectangle aReturnValue;

    tools::Rectangle aOldTextRect( maRect );        // <- initial text rectangle

    tools::Rectangle aNewTextRect( maRect );        // <- new text rectangle returned from the custom shape renderer,
    GetTextBounds( aNewTextRect );          //    it depends to the current logical shape size

    tools::Rectangle aAdjustedTextRect( aNewTextRect );                            // <- new text rectangle is being tested by AdjustTextFrameWidthAndHeight to ensure
    if ( AdjustTextFrameWidthAndHeight( aAdjustedTextRect, bHgt, bWdt ) )   //    that the new text rectangle is matching the current text size from the outliner
    {
        if (aAdjustedTextRect != aNewTextRect && aOldTextRect != aAdjustedTextRect &&
            aNewTextRect.GetWidth() && aNewTextRect.GetHeight())
        {
            aReturnValue = maRect;
            double fXScale = static_cast<double>(aOldTextRect.GetWidth()) / static_cast<double>(aNewTextRect.GetWidth());
            double fYScale = static_cast<double>(aOldTextRect.GetHeight()) / static_cast<double>(aNewTextRect.GetHeight());
            double fRightDiff = static_cast<double>( aAdjustedTextRect.Right() - aNewTextRect.Right() ) * fXScale;
            double fLeftDiff  = static_cast<double>( aAdjustedTextRect.Left()  - aNewTextRect.Left()  ) * fXScale;
            double fTopDiff   = static_cast<double>( aAdjustedTextRect.Top()   - aNewTextRect.Top()   ) * fYScale;
            double fBottomDiff= static_cast<double>( aAdjustedTextRect.Bottom()- aNewTextRect.Bottom()) * fYScale;
            aReturnValue.AdjustLeft(static_cast<sal_Int32>(fLeftDiff) );
            aReturnValue.AdjustRight(static_cast<sal_Int32>(fRightDiff) );
            aReturnValue.AdjustTop(static_cast<sal_Int32>(fTopDiff) );
            aReturnValue.AdjustBottom(static_cast<sal_Int32>(fBottomDiff) );
        }
    }
    return aReturnValue;
}

bool SdrObjCustomShape::NbcAdjustTextFrameWidthAndHeight(bool bHgt, bool bWdt)
{
    tools::Rectangle aNewTextRect = ImpCalculateTextFrame(bHgt, bWdt);
    const bool bRet = !aNewTextRect.IsEmpty() && aNewTextRect != maRect;
    if (bRet && !mbAdjustingTextFrameWidthAndHeight)
    {
        mbAdjustingTextFrameWidthAndHeight = true;

        // taking care of handles that should not been changed
        std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles() );

        maRect = aNewTextRect;
        SetRectsDirty();
        SetChanged();

        for (const auto& rInteraction : aInteractionHandles)
        {
            try
            {
                if ( rInteraction.nMode & CustomShapeHandleModes::RESIZE_FIXED )
                    rInteraction.xInteraction->setControllerPosition( rInteraction.aPosition );
            }
            catch ( const uno::RuntimeException& )
            {
            }
        }
        InvalidateRenderGeometry();

        mbAdjustingTextFrameWidthAndHeight = false;
    }
    return bRet;
}

bool SdrObjCustomShape::AdjustTextFrameWidthAndHeight()
{
    tools::Rectangle aNewTextRect = ImpCalculateTextFrame( true/*bHgt*/, true/*bWdt*/ );
    bool bRet = !aNewTextRect.IsEmpty() && ( aNewTextRect != maRect );
    if ( bRet )
    {
        tools::Rectangle aBoundRect0;
        if ( pUserCall )
            aBoundRect0 = GetCurrentBoundRect();

        // taking care of handles that should not been changed
        std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles() );

        maRect = aNewTextRect;
        SetRectsDirty();

        for (const auto& rInteraction : aInteractionHandles)
        {
            try
            {
                if ( rInteraction.nMode & CustomShapeHandleModes::RESIZE_FIXED )
                    rInteraction.xInteraction->setControllerPosition( rInteraction.aPosition );
            }
            catch ( const uno::RuntimeException& )
            {
            }
        }

        InvalidateRenderGeometry();
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
    return bRet;
}
void SdrObjCustomShape::TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, tools::Rectangle* pViewInit, tools::Rectangle* pViewMin) const
{
    Size aPaperMin,aPaperMax;
    tools::Rectangle aViewInit;
    TakeTextAnchorRect( aViewInit );
    if ( aGeo.nRotationAngle )
    {
        Point aCenter(aViewInit.Center());
        aCenter-=aViewInit.TopLeft();
        Point aCenter0(aCenter);
        RotatePoint(aCenter,Point(),aGeo.nSin,aGeo.nCos);
        aCenter-=aCenter0;
        aViewInit.Move(aCenter.X(),aCenter.Y());
    }
    Size aAnkSiz(aViewInit.GetSize());
    aAnkSiz.AdjustWidth( -1 ); aAnkSiz.AdjustHeight( -1 ); // because GetSize() adds 1
    Size aMaxSiz(1000000,1000000);
    {
        Size aTmpSiz(getSdrModelFromSdrObject().GetMaxObjSize());
        if (aTmpSiz.Width()!=0) aMaxSiz.setWidth(aTmpSiz.Width() );
        if (aTmpSiz.Height()!=0) aMaxSiz.setHeight(aTmpSiz.Height() );
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

    if (GetMergedItem(SDRATTR_TEXT_WORDWRAP).GetValue())
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
    aPaperMax.setWidth(nMaxWdt );
    aPaperMax.setHeight(nMaxHgt );

    aPaperMin.setWidth(nMinWdt );
    aPaperMin.setHeight(nMinHgt );

    if ( pViewMin )
    {
        *pViewMin = aViewInit;

        long nXFree = aAnkSiz.Width() - aPaperMin.Width();
        if ( eHAdj == SDRTEXTHORZADJUST_LEFT )
            pViewMin->AdjustRight( -nXFree );
        else if ( eHAdj == SDRTEXTHORZADJUST_RIGHT )
            pViewMin->AdjustLeft(nXFree );
        else { pViewMin->AdjustLeft(nXFree / 2 ); pViewMin->SetRight( pViewMin->Left() + aPaperMin.Width() ); }

        long nYFree = aAnkSiz.Height() - aPaperMin.Height();
        if ( eVAdj == SDRTEXTVERTADJUST_TOP )
            pViewMin->AdjustBottom( -nYFree );
        else if ( eVAdj == SDRTEXTVERTADJUST_BOTTOM )
            pViewMin->AdjustTop(nYFree );
        else { pViewMin->AdjustTop(nYFree / 2 ); pViewMin->SetBottom( pViewMin->Top() + aPaperMin.Height() ); }
    }

    if( IsVerticalWriting() )
        aPaperMin.setWidth( 0 );
    else
        aPaperMin.setHeight( 0 );

    if( eHAdj != SDRTEXTHORZADJUST_BLOCK )
        aPaperMin.setWidth(0 );

    // For complete vertical adjust support, set paper min height to 0, here.
    if(SDRTEXTVERTADJUST_BLOCK != eVAdj )
        aPaperMin.setHeight( 0 );

    if (pPaperMin!=nullptr) *pPaperMin=aPaperMin;
    if (pPaperMax!=nullptr) *pPaperMax=aPaperMax;
    if (pViewInit!=nullptr) *pViewInit=aViewInit;
}
void SdrObjCustomShape::EndTextEdit( SdrOutliner& rOutl )
{
    SdrTextObj::EndTextEdit( rOutl );
    InvalidateRenderGeometry();
}
void SdrObjCustomShape::TakeTextAnchorRect( tools::Rectangle& rAnchorRect ) const
{
    if ( GetTextBounds( rAnchorRect ) )
    {
        Point aRotateRef( maSnapRect.Center() );
        rAnchorRect.AdjustLeft(GetTextLeftDistance() );
        rAnchorRect.AdjustTop(GetTextUpperDistance() );
        rAnchorRect.AdjustRight( -(GetTextRightDistance()) );
        rAnchorRect.AdjustBottom( -(GetTextLowerDistance()) );
        ImpJustifyRect( rAnchorRect );

        if ( rAnchorRect.GetWidth() < 2 )
            rAnchorRect.SetRight( rAnchorRect.Left() + 1 );   // minimal width is 2
        if ( rAnchorRect.GetHeight() < 2 )
            rAnchorRect.SetBottom( rAnchorRect.Top() + 1 );   // minimal height is 2
        if ( aGeo.nRotationAngle )
        {
            Point aP( rAnchorRect.TopLeft() );
            RotatePoint( aP, aRotateRef, aGeo.nSin, aGeo. nCos );
            rAnchorRect.SetPos( aP );
        }
    }
    else
        SdrTextObj::TakeTextAnchorRect( rAnchorRect );
}
void SdrObjCustomShape::TakeTextRect( SdrOutliner& rOutliner, tools::Rectangle& rTextRect, bool bNoEditText,
                               tools::Rectangle* pAnchorRect, bool /*bLineWidth*/) const
{
    tools::Rectangle aAnkRect; // Rect in which we anchor
    TakeTextAnchorRect(aAnkRect);
    SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
    SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
    EEControlBits nStat0=rOutliner.GetControlWord();
    Size aNullSize;

    rOutliner.SetControlWord(nStat0|EEControlBits::AUTOPAGESIZE);
    rOutliner.SetMinAutoPaperSize(aNullSize);
    sal_Int32 nMaxAutoPaperWidth = 1000000;
    sal_Int32 nMaxAutoPaperHeight= 1000000;

    long nAnkWdt=aAnkRect.GetWidth();
    long nAnkHgt=aAnkRect.GetHeight();

    if (GetMergedItem(SDRATTR_TEXT_WORDWRAP).GetValue())
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
        pPara=pEdtOutl->CreateParaObject().release();

    if (pPara)
    {
        bool bHitTest(&getSdrModelFromSdrObject().GetHitTestOutliner() == &rOutliner);
        const SdrTextObj* pTestObj = rOutliner.GetTextObj();

        if( !pTestObj || !bHitTest || pTestObj != this ||
            pTestObj->GetOutlinerParaObject() != GetOutlinerParaObject() )
        {
            if( bHitTest )
                rOutliner.SetTextObj( this );

            rOutliner.SetUpdateMode(true);
            rOutliner.SetText(*pPara);
        }
    }
    else
    {
        rOutliner.SetTextObj( nullptr );
    }
    if (pEdtOutl && !bNoEditText && pPara)
        delete pPara;

    rOutliner.SetUpdateMode(true);
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
                SvxAdjust eAdjust = GetObjectItemSet().Get(EE_PARA_JUST).GetAdjust();
                switch (eAdjust)
                {
                    case SvxAdjust::Left:   eHAdj = SDRTEXTHORZADJUST_LEFT; break;
                    case SvxAdjust::Right:  eHAdj = SDRTEXTHORZADJUST_RIGHT; break;
                    case SvxAdjust::Center: eHAdj = SDRTEXTHORZADJUST_CENTER; break;
                    default: break;
                }
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
            aTextPos.AdjustX(nFreeWdt/2 );
        if (eHAdj==SDRTEXTHORZADJUST_RIGHT)
            aTextPos.AdjustX(nFreeWdt );
    }
    if (eVAdj==SDRTEXTVERTADJUST_CENTER || eVAdj==SDRTEXTVERTADJUST_BOTTOM)
    {
        long nFreeHgt=aAnkRect.GetHeight()-aTextSiz.Height();
        if (eVAdj==SDRTEXTVERTADJUST_CENTER)
            aTextPos.AdjustY(nFreeHgt/2 );
        if (eVAdj==SDRTEXTVERTADJUST_BOTTOM)
            aTextPos.AdjustY(nFreeHgt );
    }
    if (aGeo.nRotationAngle!=0)
        RotatePoint(aTextPos,aAnkRect.TopLeft(),aGeo.nSin,aGeo.nCos);

    if (pAnchorRect)
        *pAnchorRect=aAnkRect;

    // using rTextRect together with ContourFrame doesn't always work correctly
    rTextRect=tools::Rectangle(aTextPos,aTextSiz);
}

void SdrObjCustomShape::NbcSetOutlinerParaObject(std::unique_ptr<OutlinerParaObject> pTextObject)
{
    SdrTextObj::NbcSetOutlinerParaObject( std::move(pTextObject) );
    SetBoundRectDirty();
    SetRectsDirty(true);
    InvalidateRenderGeometry();
}

SdrObjCustomShape* SdrObjCustomShape::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< SdrObjCustomShape >(rTargetModel);
}

SdrObjCustomShape& SdrObjCustomShape::operator=(const SdrObjCustomShape& rObj)
{
    if( this == &rObj )
        return *this;
    SdrTextObj::operator=( rObj );
    fObjectRotation = rObj.fObjectRotation;
    mbAdjustingTextFrameWidthAndHeight = rObj.mbAdjustingTextFrameWidthAndHeight;
    assert(!mbAdjustingTextFrameWidthAndHeight);
    InvalidateRenderGeometry();
    return *this;
}


OUString SdrObjCustomShape::TakeObjNameSingul() const
{
    OUStringBuffer sName(SvxResId(STR_ObjNameSingulCUSTOMSHAPE));
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
    return SvxResId(STR_ObjNamePluralCUSTOMSHAPE);
}

basegfx::B2DPolyPolygon SdrObjCustomShape::TakeXorPoly() const
{
    return GetLineGeometry( false );
}

basegfx::B2DPolyPolygon SdrObjCustomShape::TakeContour() const
{
    const SdrObject* pSdrObject = GetSdrObjectFromCustomShape();
    if ( pSdrObject )
        return pSdrObject->TakeContour();
    return basegfx::B2DPolyPolygon();
}

SdrObject* SdrObjCustomShape::DoConvertToPolyObj(bool bBezier, bool bAddText) const
{
    // #i37011#
    SdrObject* pRetval = nullptr;
    SdrObject* pRenderedCustomShape = nullptr;

    if ( !mXRenderedCustomShape.is() )
    {
        // force CustomShape
        GetSdrObjectFromCustomShape();
    }

    if ( mXRenderedCustomShape.is() )
    {
        pRenderedCustomShape = GetSdrObjectFromXShape( mXRenderedCustomShape );
    }

    if ( pRenderedCustomShape )
    {
        // Clone to same SdrModel
        SdrObject* pCandidate(pRenderedCustomShape->CloneSdrObject(pRenderedCustomShape->getSdrModelFromSdrObject()));
        DBG_ASSERT(pCandidate, "SdrObjCustomShape::DoConvertToPolyObj: Could not clone SdrObject (!)");
        pRetval = pCandidate->DoConvertToPolyObj(bBezier, bAddText);
        SdrObject::Free( pCandidate );

        if(pRetval)
        {
            const bool bShadow(GetMergedItem(SDRATTR_SHADOW).GetValue());
            if(bShadow)
            {
                pRetval->SetMergedItem(makeSdrShadowItem(true));
            }
        }

        if(bAddText && HasText() && !IsTextPath())
        {
            pRetval = ImpConvertAddText(pRetval, bBezier);
        }
    }

    return pRetval;
}

void SdrObjCustomShape::NbcSetStyleSheet( SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr )
{
    // #i40944#
    InvalidateRenderGeometry();
    SdrObject::NbcSetStyleSheet( pNewStyleSheet, bDontRemoveHardAttr );
}

void SdrObjCustomShape::handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage)
{
    // call parent
    SdrTextObj::handlePageChange(pOldPage, pNewPage);

    if(nullptr != pNewPage)
    {
        // invalidating rectangles by SetRectsDirty is not sufficient,
        // AdjustTextFrameWidthAndHeight() also has to be made, both
        // actions are done by NbcSetSnapRect
        tools::Rectangle aTmp( maRect );    //creating temporary rectangle #i61108#
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
    SdrAShapeObjGeoData& rAGeo=static_cast<SdrAShapeObjGeoData&>(rGeo);
    rAGeo.fObjectRotation = fObjectRotation;
    rAGeo.bMirroredX = IsMirroredX();
    rAGeo.bMirroredY = IsMirroredY();

    const OUString sAdjustmentValues( "AdjustmentValues" );
    const Any* pAny = GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ).GetPropertyValueByName( sAdjustmentValues );
    if ( pAny )
        *pAny >>= rAGeo.aAdjustmentSeq;
}

void SdrObjCustomShape::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrTextObj::RestGeoData( rGeo );
    const SdrAShapeObjGeoData& rAGeo=static_cast<const SdrAShapeObjGeoData&>(rGeo);
    fObjectRotation = rAGeo.fObjectRotation;
    SetMirroredX( rAGeo.bMirroredX );
    SetMirroredY( rAGeo.bMirroredY );

    SdrCustomShapeGeometryItem rGeometryItem = GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
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
    fObjectRotation = 0.0;
    aGeo.nRotationAngle = 0;
    aGeo.RecalcSinCos();
    aGeo.nShearAngle = 0;
    aGeo.RecalcTan();

    // if anchor is used, make position relative to it
    if(getSdrModelFromSdrObject().IsWriter())
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate += basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // build and set BaseRect (use scale)
    Size aSize(FRound(aScale.getX()), FRound(aScale.getY()));
    // fdo#47434 We need a valid rectangle here
    if( !aSize.Height() ) aSize.setHeight( 1 );
    if( !aSize.Width() ) aSize.setWidth( 1 );

    tools::Rectangle aBaseRect(Point(), aSize);
    SetSnapRect(aBaseRect);

    // shear?
    if(!basegfx::fTools::equalZero(fShearX))
    {
        GeoStat aGeoStat;
        // #i123181# The fix for #121932# here was wrong, the trunk version does not correct the
        // mirrored shear values, neither at the object level, nor on the API or XML level. Taking
        // back the mirroring of the shear angle
        aGeoStat.nShearAngle = FRound(basegfx::rad2deg(atan(fShearX)) * 100.0);
        aGeoStat.RecalcTan();
        Shear(Point(), aGeoStat.nShearAngle, aGeoStat.nTan, false);
    }

    // rotation?
    if(!basegfx::fTools::equalZero(fRotate))
    {
        GeoStat aGeoStat;

        // #i78696#
        // fRotate is mathematically correct, but aGeoStat.nRotationAngle is
        // mirrored -> mirror value here
        aGeoStat.nRotationAngle = NormAngle36000(FRound(-fRotate / F_PI18000));
        aGeoStat.RecalcSinCos();
        Rotate(Point(), aGeoStat.nRotationAngle, aGeoStat.nSin, aGeoStat.nCos);
    }

    // translate?
    if(!aTranslate.equalZero())
    {
        Move(Size(FRound(aTranslate.getX()), FRound(aTranslate.getY())));
    }
}

// taking fObjectRotation instead of aGeo.nAngle
bool SdrObjCustomShape::TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& /*rPolyPolygon*/) const
{
    // get turn and shear
    double fRotate = basegfx::deg2rad(fObjectRotation);
    double fShearX = basegfx::deg2rad(aGeo.nShearAngle / 100.0);

    // get aRect, this is the unrotated snaprect
    tools::Rectangle aRectangle(maRect);

    bool bMirroredX = IsMirroredX();
    bool bMirroredY = IsMirroredY();
    if ( bMirroredX || bMirroredY )
    {   // we have to retrieve the unmirrored rect

        GeoStat aNewGeo( aGeo );

        if ( bMirroredX )
        {
            tools::Polygon aPol = Rect2Poly(maRect, aNewGeo);
            tools::Rectangle aBoundRect( aPol.GetBoundRect() );

            Point aRef1( ( aBoundRect.Left() + aBoundRect.Right() ) >> 1, aBoundRect.Top() );
            Point aRef2( aRef1.X(), aRef1.Y() + 1000 );
            sal_uInt16 i;
            sal_uInt16 nPointCount=aPol.GetSize();
            for (i=0; i<nPointCount; i++)
            {
                MirrorPoint(aPol[i],aRef1,aRef2);
            }
            // mirror polygon and move it a bit
            tools::Polygon aPol0(aPol);
            aPol[0]=aPol0[1];
            aPol[1]=aPol0[0];
            aPol[2]=aPol0[3];
            aPol[3]=aPol0[2];
            aPol[4]=aPol0[1];
            Poly2Rect(aPol,aRectangle,aNewGeo);
        }
        if ( bMirroredY )
        {
            tools::Polygon aPol( Rect2Poly( aRectangle, aNewGeo ) );
            tools::Rectangle aBoundRect( aPol.GetBoundRect() );

            Point aRef1( aBoundRect.Left(), ( aBoundRect.Top() + aBoundRect.Bottom() ) >> 1 );
            Point aRef2( aRef1.X() + 1000, aRef1.Y() );
            sal_uInt16 i;
            sal_uInt16 nPointCount=aPol.GetSize();
            for (i=0; i<nPointCount; i++)
            {
                MirrorPoint(aPol[i],aRef1,aRef2);
            }
            // mirror polygon and move it a bit
            tools::Polygon aPol0(aPol);
            aPol[0]=aPol0[1]; // This was WRONG for vertical (!)
            aPol[1]=aPol0[0]; // #i121932# Despite my own comment above
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
    if(getSdrModelFromSdrObject().IsWriter())
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate -= basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // build matrix
    rMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
        aScale,
        basegfx::fTools::equalZero(fShearX) ? 0.0 : tan(fShearX),
        basegfx::fTools::equalZero(fRotate) ? 0.0 : -fRotate,
        aTranslate);

    return false;
}

std::unique_ptr<sdr::contact::ViewContact> SdrObjCustomShape::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfSdrObjCustomShape>(*this);
}

// #i33136#
bool SdrObjCustomShape::doConstructOrthogonal(const OUString& rName)
{
    bool bRetval(false);

    if(rName.equalsIgnoreAsciiCase("quadrat"))
    {
        bRetval = true;
    }
    else if(rName.equalsIgnoreAsciiCase("round-quadrat"))
    {
        bRetval = true;
    }
    else if(rName.equalsIgnoreAsciiCase("circle"))
    {
        bRetval = true;
    }
    else if(rName.equalsIgnoreAsciiCase("circle-pie"))
    {
        bRetval = true;
    }
    else if(rName.equalsIgnoreAsciiCase("ring"))
    {
        bRetval = true;
    }

    return bRetval;
}

// #i37011# centralize throw-away of render geometry
void SdrObjCustomShape::InvalidateRenderGeometry()
{
    mXRenderedCustomShape = nullptr;
    SdrObject::Free( mpLastShadowGeometry );
    mpLastShadowGeometry = nullptr;
}

void SdrObjCustomShape::impl_setUnoShape(const uno::Reference<uno::XInterface>& rxUnoShape)
{
    SdrTextObj::impl_setUnoShape(rxUnoShape);

    // The shape engine is created with _current_ shape. This means we
    // _must_ reset it when the shape changes.
    mxCustomShapeEngine.set(nullptr);
}

OUString SdrObjCustomShape::GetCustomShapeName()
{
    OUString sShapeName;
    OUString aEngine( GetMergedItem( SDRATTR_CUSTOMSHAPE_ENGINE ).GetValue() );
    if ( aEngine.isEmpty()
         || aEngine == "com.sun.star.drawing.EnhancedCustomShapeEngine" )
    {
        OUString sShapeType;
        const OUString sType("Type");
        const SdrCustomShapeGeometryItem& rGeometryItem( GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
        const Any* pAny = rGeometryItem.GetPropertyValueByName( sType );
        if ( pAny && ( *pAny >>= sShapeType ) )
            sShapeName = EnhancedCustomShapeTypeNames::GetAccName( sShapeType );
    }
    return sShapeName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
