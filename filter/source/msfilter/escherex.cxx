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

#include "eschesdo.hxx"
#include <o3tl/any.hxx>
#include <svx/svdxcgv.hxx>
#include <svx/svdomedia.hxx>
#include <svx/xflftrit.hxx>
#include <filter/msfilter/escherex.hxx>
#include <filter/msfilter/util.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/sdtfsitm.hxx>
#include <editeng/outlobj.hxx>
#include <vcl/graph.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/zcodec.hxx>
#include <tools/urlobj.hxx>
#include <svx/svdopath.hxx>
#include <stdlib.h>
#include <vcl/graphicfilter.hxx>
#include <svx/EnhancedCustomShapeTypeNames.hxx>
#include <svx/EnhancedCustomShapeGeometry.hxx>
#include <svx/EnhancedCustomShapeFunctionParser.hxx>
#include <svx/EnhancedCustomShape2d.hxx>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/awt/GradientStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/drawing/PolygonFlags.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeGluePointType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <vcl/hatch.hxx>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/string.hxx>
#include <vcl/virdev.hxx>
#include <rtl/crc.h>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <osl/diagnose.h>

#include <algorithm>
#include <memory>

using namespace css;

EscherExContainer::EscherExContainer( SvStream& rSt, const sal_uInt16 nRecType, const sal_uInt16 nInstance ) :
    rStrm   ( rSt )
{
    rStrm.WriteUInt32( ( 0xf | ( nInstance << 4 ) ) | ( nRecType << 16 ) ).WriteUInt32( 0 );
    nContPos = rStrm.Tell();
}
EscherExContainer::~EscherExContainer()
{
    sal_uInt32 nPos = rStrm.Tell();
    sal_uInt32 nSize= nPos - nContPos;
    if ( nSize )
    {
        rStrm.Seek( nContPos - 4 );
        rStrm.WriteUInt32( nSize );
        rStrm.Seek( nPos );
    }
}

EscherExAtom::EscherExAtom( SvStream& rSt, const sal_uInt16 nRecType, const sal_uInt16 nInstance, const sal_uInt8 nVersion ) :
    rStrm   ( rSt )
{
    rStrm.WriteUInt32( ( nVersion | ( nInstance << 4 ) ) | ( nRecType << 16 ) ).WriteUInt32( 0 );
    nContPos = rStrm.Tell();
}
EscherExAtom::~EscherExAtom()
{
    sal_uInt32 nPos = rStrm.Tell();
    sal_uInt32 nSize= nPos - nContPos;
    if ( nSize )
    {
        rStrm.Seek( nContPos - 4 );
        rStrm.WriteUInt32( nSize );
        rStrm.Seek( nPos );
    }
}

EscherExClientRecord_Base::~EscherExClientRecord_Base()
{
}

EscherExClientAnchor_Base::~EscherExClientAnchor_Base()
{
}

EscherPropertyContainer::EscherPropertyContainer(
    EscherGraphicProvider * pGraphProv, SvStream * pPiOutStrm,
    tools::Rectangle * pBoundRect):
    pGraphicProvider(pGraphProv),
    pPicOutStrm(pPiOutStrm),
    pShapeBoundRect(pBoundRect),
    nCountCount(0),
    nCountSize(0),
    bHasComplexData(false)
{
    pSortStruct.reserve(64);
}

EscherPropertyContainer::EscherPropertyContainer()
    : EscherPropertyContainer(nullptr, nullptr, nullptr)
{}

EscherPropertyContainer::EscherPropertyContainer(
    EscherGraphicProvider& rGraphProv,
            SvStream* pPiOutStrm,
                tools::Rectangle& rBoundRect ) :
    EscherPropertyContainer(&rGraphProv, pPiOutStrm, &rBoundRect)
{}

EscherPropertyContainer::~EscherPropertyContainer()
{
};

void EscherPropertyContainer::AddOpt(
    sal_uInt16 nPropID,
    bool bBlib,
    sal_uInt32 nSizeReduction,
    SvMemoryStream& rStream)
{
    sal_uInt8 const* pBuf(static_cast<sal_uInt8 const *>(rStream.GetData()));
    const sal_uInt64 nSize(rStream.GetSize());
    std::vector<sal_uInt8> aBuf;
    aBuf.reserve(nSize);

    for(sal_uInt64 a(0); a < nSize; a++)
    {
        aBuf.push_back(*pBuf++);
    }

    sal_uInt32 nPropValue(static_cast<sal_uInt32>(nSize));

    if(0 != nSizeReduction && nPropValue > nSizeReduction)
    {
        nPropValue -= nSizeReduction;
    }

    AddOpt(nPropID, bBlib, nPropValue, aBuf);
}

void EscherPropertyContainer::AddOpt(
    sal_uInt16 nPropID,
    sal_uInt32 nPropValue,
    bool bBlib)
{
    AddOpt(nPropID, bBlib, nPropValue, std::vector<sal_uInt8>());
}

void EscherPropertyContainer::AddOpt(
    sal_uInt16 nPropID,
    const OUString& rString)
{
    std::vector<sal_uInt8> aBuf;
    aBuf.reserve(rString.getLength() * 2 + 2);

    for(sal_Int32 i(0); i < rString.getLength(); i++)
    {
        const sal_Unicode nUnicode(rString[i]);
        aBuf.push_back(static_cast<sal_uInt8>(nUnicode));
        aBuf.push_back(static_cast<sal_uInt8>(nUnicode >> 8));
    }

    aBuf.push_back(0);
    aBuf.push_back(0);

    AddOpt(nPropID, true, aBuf.size(), aBuf);
}

void EscherPropertyContainer::AddOpt(
    sal_uInt16 nPropID,
    bool bBlib,
    sal_uInt32 nPropValue,
    const std::vector<sal_uInt8>& rProp)
{
    if ( bBlib )                // bBlib is only valid when fComplex = 0
        nPropID |= 0x4000;
    if ( !rProp.empty() )
        nPropID |= 0x8000;      // fComplex = sal_True;

    for( size_t i = 0; i < pSortStruct.size(); i++ )
    {
        if ( ( pSortStruct[ i ].nPropId &~0xc000 ) == ( nPropID &~0xc000 ) )    // check, whether the Property only gets replaced
        {
            pSortStruct[ i ].nPropId = nPropID;
            if ( !pSortStruct[ i ].nProp.empty() )
            {
                nCountSize -= pSortStruct[ i ].nProp.size();
            }
            pSortStruct[ i ].nProp = rProp;
            pSortStruct[ i ].nPropValue = nPropValue;
            if ( !rProp.empty() )
                nCountSize += rProp.size();
            return;
        }
    }
    nCountCount++;
    nCountSize += 6;
    pSortStruct.emplace_back();
    pSortStruct.back().nPropId = nPropID;                                // insert property
    pSortStruct.back().nProp = rProp;
    pSortStruct.back().nPropValue = nPropValue;

    if ( !rProp.empty() )
    {
        nCountSize += rProp.size();
        bHasComplexData = true;
    }
}

bool EscherPropertyContainer::GetOpt( sal_uInt16 nPropId, sal_uInt32& rPropValue ) const
{
    EscherPropSortStruct aPropStruct;

    if ( GetOpt( nPropId, aPropStruct ) )
    {
        rPropValue = aPropStruct.nPropValue;
        return true;
    }
    return false;
}

bool EscherPropertyContainer::GetOpt( sal_uInt16 nPropId, EscherPropSortStruct& rPropValue ) const
{
    for( size_t i = 0; i < pSortStruct.size(); i++ )
    {
        if ( ( pSortStruct[ i ].nPropId &~0xc000 ) == ( nPropId &~0xc000 ) )
        {
            rPropValue = pSortStruct[ i ];
            return true;
        }
    }
    return false;
}

const EscherProperties & EscherPropertyContainer::GetOpts() const
{
    return pSortStruct;
}

extern "C" {

static int EscherPropSortFunc( const void* p1, const void* p2 )
{
    sal_Int16   nID1 = static_cast<EscherPropSortStruct const *>(p1)->nPropId &~0xc000;
    sal_Int16   nID2 = static_cast<EscherPropSortStruct const *>(p2)->nPropId &~0xc000;

    if( nID1  < nID2 )
        return -1;
    else if( nID1 > nID2 )
        return 1;
    else
        return 0;
}

}

void EscherPropertyContainer::Commit( SvStream& rSt, sal_uInt16 nVersion, sal_uInt16 nRecType )
{
    rSt.WriteUInt16( ( nCountCount << 4 ) | ( nVersion & 0xf ) ).WriteUInt16( nRecType ).WriteUInt32( nCountSize );
    if ( pSortStruct.empty() )
        return;

    qsort( pSortStruct.data(), pSortStruct.size(), sizeof( EscherPropSortStruct ), EscherPropSortFunc );

    for ( size_t i = 0; i < pSortStruct.size(); i++ )
    {
        sal_uInt32 nPropValue = pSortStruct[ i ].nPropValue;
        sal_uInt16 nPropId = pSortStruct[ i ].nPropId;

        rSt.WriteUInt16( nPropId )
           .WriteUInt32( nPropValue );
    }
    if ( bHasComplexData )
    {
        for ( size_t i = 0; i < pSortStruct.size(); i++ )
        {
            if ( !pSortStruct[ i ].nProp.empty() )
                rSt.WriteBytes(
                    pSortStruct[i].nProp.data(),
                    pSortStruct[i].nProp.size());
        }
    }
}

bool EscherPropertyContainer::IsFontWork() const
{
    sal_uInt32 nTextPathFlags = 0;
    GetOpt( DFF_Prop_gtextFStrikethrough, nTextPathFlags );
    return ( nTextPathFlags & 0x4000 ) != 0;
}

sal_uInt32 EscherPropertyContainer::ImplGetColor( const sal_uInt32 nSOColor, bool bSwap )
{
    if ( bSwap )
    {
        sal_uInt32 nColor = nSOColor & 0xff00;      // green
        nColor |= static_cast<sal_uInt8>(nSOColor) << 16;    // red
        nColor |= static_cast<sal_uInt8>( nSOColor >> 16 );    // blue
        return nColor;
    }
    else
        return nSOColor & 0xffffff;
}

sal_uInt32 EscherPropertyContainer::GetGradientColor(
    const awt::Gradient* pGradient,
        sal_uInt32 nStartColor )
{
    sal_uInt32  nIntensity = 100;
    Color       aColor;

    if ( pGradient )
    {
        if ( nStartColor & 1 )
        {
            nIntensity = pGradient->StartIntensity;
            aColor = Color(ColorTransparency, pGradient->StartColor);
        }
        else
        {
            nIntensity = pGradient->EndIntensity;
            aColor = Color(ColorTransparency, pGradient->EndColor);
        }
    }
    sal_uInt32  nRed = ( aColor.GetRed() * nIntensity ) / 100;
    sal_uInt32  nGreen = ( ( aColor.GetGreen() * nIntensity ) / 100 ) << 8;
    sal_uInt32  nBlue = ( ( aColor.GetBlue() * nIntensity ) / 100 ) << 16;
    return nRed | nGreen | nBlue;
}

void EscherPropertyContainer::CreateGradientProperties(
    const awt::Gradient & rGradient )
{
    sal_uInt32  nFillType = ESCHER_FillShadeScale;
    sal_uInt32  nAngle = 0;
    sal_uInt32  nFillFocus = 0;
    sal_uInt32  nFillLR = 0;
    sal_uInt32  nFillTB = 0;
    sal_uInt32  nFirstColor = 0;
    bool        bWriteFillTo = false;

    switch ( rGradient.Style )
    {
        case awt::GradientStyle_LINEAR :
        case awt::GradientStyle_AXIAL :
        {
            nFillType = ESCHER_FillShadeScale;
            nAngle = (rGradient.Angle * 0x10000) / 10;
            nFillFocus = (sal::static_int_cast<int>(rGradient.Style) ==
                          sal::static_int_cast<int>(GradientStyle::Linear)) ? 0 : 50;
        }
        break;
        case awt::GradientStyle_RADIAL :
        case awt::GradientStyle_ELLIPTICAL :
        case awt::GradientStyle_SQUARE :
        case awt::GradientStyle_RECT :
        {
            nFillLR = (rGradient.XOffset * 0x10000) / 100;
            nFillTB = (rGradient.YOffset * 0x10000) / 100;
            if ( ((nFillLR > 0) && (nFillLR < 0x10000)) || ((nFillTB > 0) && (nFillTB < 0x10000)) )
                nFillType = ESCHER_FillShadeShape;
            else
                nFillType = ESCHER_FillShadeCenter;
            nFirstColor = 1;
            bWriteFillTo = true;
        }
        break;
        case awt::GradientStyle::GradientStyle_MAKE_FIXED_SIZE : break;
    }
    AddOpt( ESCHER_Prop_fillType, nFillType );
    AddOpt( ESCHER_Prop_fillAngle, nAngle );
    AddOpt( ESCHER_Prop_fillColor, GetGradientColor( &rGradient, nFirstColor ) );
    AddOpt( ESCHER_Prop_fillBackColor, GetGradientColor( &rGradient, nFirstColor ^ 1 ) );
    AddOpt( ESCHER_Prop_fillFocus, nFillFocus );
    if ( bWriteFillTo )
    {
        AddOpt( ESCHER_Prop_fillToLeft, nFillLR );
        AddOpt( ESCHER_Prop_fillToTop, nFillTB );
        AddOpt( ESCHER_Prop_fillToRight, nFillLR );
        AddOpt( ESCHER_Prop_fillToBottom, nFillTB );
    }
}

void EscherPropertyContainer::CreateGradientProperties(
    const uno::Reference<beans::XPropertySet> & rXPropSet , bool bTransparentGradient)
{
    uno::Any aAny;
    awt::Gradient const * pGradient = nullptr;

    sal_uInt32 nFillType = ESCHER_FillShadeScale;
    sal_Int32 nAngle = 0;
    sal_uInt32 nFillFocus = 0;
    sal_uInt32 nFillLR = 0;
    sal_uInt32 nFillTB = 0;
    sal_uInt32 nFirstColor = 0;// like the control var nChgColors in import logic
    bool bWriteFillTo = false;

    // Transparency gradient: Means the third setting in transparency page is set
    if (bTransparentGradient &&  EscherPropertyValueHelper::GetPropertyValue(
        aAny, rXPropSet, "FillTransparenceGradient" ) )
    {
        pGradient = o3tl::doAccess<awt::Gradient>(aAny);

        uno::Any aAnyTemp;
        if ( EscherPropertyValueHelper::GetPropertyValue(
            aAnyTemp, rXPropSet, "FillStyle" ) )
        {
            drawing::FillStyle eFS;
            if ( ! ( aAnyTemp >>= eFS ) )
                eFS = drawing::FillStyle_SOLID;
            // solid and transparency
            if ( eFS == drawing::FillStyle_SOLID)
            {
                if ( EscherPropertyValueHelper::GetPropertyValue(
                    aAnyTemp, rXPropSet, "FillColor" ) )
                {
                    const_cast<awt::Gradient *>(pGradient)->StartColor = ImplGetColor( *o3tl::doAccess<sal_uInt32>(aAnyTemp), false );
                    const_cast<awt::Gradient *>(pGradient)->EndColor = ImplGetColor( *o3tl::doAccess<sal_uInt32>(aAnyTemp), false );
                }
            }
            // gradient and transparency.
            else if( eFS == drawing::FillStyle_GRADIENT )
            {
                if ( EscherPropertyValueHelper::GetPropertyValue(
                    aAny, rXPropSet, "FillGradient" ) )
                    pGradient = o3tl::doAccess<awt::Gradient>(aAny);
            }
        }

    }
    // Not transparency gradient
    else if ( EscherPropertyValueHelper::GetPropertyValue(
        aAny, rXPropSet, "FillGradient" ) )
    {
        pGradient = o3tl::doAccess<awt::Gradient>(aAny);
    }

    if ( pGradient )
    {
        switch ( pGradient->Style )
        {
        case awt::GradientStyle_LINEAR :
        case awt::GradientStyle_AXIAL :
            {
                nFillType = ESCHER_FillShadeScale;
                nAngle = pGradient->Angle;
                while ( nAngle > 0 ) nAngle -= 3600;
                while ( nAngle <= -3600 ) nAngle += 3600;
                // Value of the real number = Integral + (Fractional / 65536.0)
                nAngle = ( nAngle * 0x10000) / 10;

                nFillFocus = (pGradient->Style == awt::GradientStyle_LINEAR) ?
                            ( pGradient->XOffset + pGradient->YOffset )/2 : -50;
                if( !nFillFocus )
                    nFirstColor=nFirstColor ^ 1;
                if ( !nAngle )
                    nFirstColor=nFirstColor ^ 1;
            }
            break;
        case awt::GradientStyle_RADIAL :
        case awt::GradientStyle_ELLIPTICAL :
        case awt::GradientStyle_SQUARE :
        case awt::GradientStyle_RECT :
            {
                // according to the import logic and rect type fill** value
                nFillLR = (pGradient->XOffset * 0x10000) / 100;
                nFillTB = (pGradient->YOffset * 0x10000) / 100;
                if ( ((nFillLR > 0) && (nFillLR < 0x10000)) || ((nFillTB > 0) && (nFillTB < 0x10000)) )
                    nFillType = ESCHER_FillShadeShape;
                else
                    nFillType = ESCHER_FillShadeCenter;
                nFirstColor = 1;
                bWriteFillTo = true;
            }
            break;
        default: break;
        }
    }

    AddOpt( ESCHER_Prop_fillType, nFillType );
    AddOpt( ESCHER_Prop_fillAngle, nAngle );
    AddOpt( ESCHER_Prop_fillColor, GetGradientColor( pGradient, nFirstColor ) );
    AddOpt( ESCHER_Prop_fillBackColor, GetGradientColor( pGradient, nFirstColor ^ 1 ) );
    AddOpt( ESCHER_Prop_fillFocus, nFillFocus );
    if ( bWriteFillTo )
    {
        // according to rect type fillTo** value
        if(nFillLR)
        {
            AddOpt( ESCHER_Prop_fillToLeft, nFillLR );
            AddOpt( ESCHER_Prop_fillToRight, nFillLR );
        }
        if(nFillTB)
        {
            AddOpt( ESCHER_Prop_fillToTop, nFillTB );
            AddOpt( ESCHER_Prop_fillToBottom, nFillTB );
        }
    }

    // Transparency gradient
    if (bTransparentGradient &&  EscherPropertyValueHelper::GetPropertyValue(
        aAny, rXPropSet, "FillTransparenceGradient" ) )
    {
        pGradient = o3tl::doAccess<awt::Gradient>(aAny);
        if ( pGradient )
        {
            sal_uInt32  nBlue =  GetGradientColor( pGradient, nFirstColor ) >> 16;
            AddOpt( ESCHER_Prop_fillOpacity,( ( 100 - ( nBlue * 100 / 255 ) ) << 16 ) / 100 );
            nBlue =  GetGradientColor( pGradient, nFirstColor ^ 1 ) >>16 ;
            AddOpt( ESCHER_Prop_fillBackOpacity,( ( 100 - ( nBlue * 100 / 255 ) ) << 16 )/ 100 );
        }
    }
}

void    EscherPropertyContainer::CreateFillProperties(
    const uno::Reference<beans::XPropertySet> & rXPropSet,
    bool bEdge ,  const uno::Reference<drawing::XShape> & rXShape )
{
    if ( rXShape.is() )
    {
        SdrObject* pObj = SdrObject::getSdrObjectFromXShape(rXShape);
        if ( pObj )
        {
            const SfxItemSet& aAttr( pObj->GetMergedItemSet() );
            // transparency with gradient. Means the third setting in transparency page is set
            bool bTransparentGradient =  ( aAttr.GetItemState( XATTR_FILLFLOATTRANSPARENCE ) == SfxItemState::SET ) &&
                aAttr.Get( XATTR_FILLFLOATTRANSPARENCE ).IsEnabled();
            CreateFillProperties(  rXPropSet, bEdge, bTransparentGradient );
        }
    }
}

void EscherPropertyContainer::CreateFillProperties(
    const uno::Reference<beans::XPropertySet> & rXPropSet,
    bool bEdge , bool bTransparentGradient)

{
    uno::Any aAny;
    AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapNone );
    AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );
    static const OUStringLiteral aPropName( u"FillStyle" );

    if ( EscherPropertyValueHelper::GetPropertyValue(
            aAny, rXPropSet, aPropName ) )
    {
        drawing::FillStyle eFS;
        if ( ! ( aAny >>= eFS ) )
            eFS = drawing::FillStyle_SOLID;
        sal_uInt32 nFillBackColor = 0;
        switch( eFS )
        {
            case drawing::FillStyle_GRADIENT :
            {
                CreateGradientProperties( rXPropSet , bTransparentGradient );
                AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
            }
            break;

            case drawing::FillStyle_BITMAP :
            {
                CreateGraphicProperties(rXPropSet, "FillBitmap", true);
                AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
                AddOpt( ESCHER_Prop_fillBackColor, nFillBackColor  );
            }
            break;
            case drawing::FillStyle_HATCH :
            {
                CreateGraphicProperties( rXPropSet, "FillHatch", true );
            }
            break;
            case drawing::FillStyle_SOLID :
            default:
            {
                if ( bTransparentGradient )
                    CreateGradientProperties( rXPropSet , bTransparentGradient );
                else
                {
                    beans::PropertyState ePropState = EscherPropertyValueHelper::GetPropertyState(
                        rXPropSet, aPropName );
                    if ( ePropState == beans::PropertyState_DIRECT_VALUE )
                        AddOpt( ESCHER_Prop_fillType, ESCHER_FillSolid );

                    if ( EscherPropertyValueHelper::GetPropertyValue(
                            aAny, rXPropSet, "FillColor" ) )
                    {
                        sal_uInt32 nFillColor = ImplGetColor( *o3tl::doAccess<sal_uInt32>(aAny) );
                        nFillBackColor = nFillColor ^ 0xffffff;
                        AddOpt( ESCHER_Prop_fillColor, nFillColor );
                    }
                    AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100010 );
                    AddOpt( ESCHER_Prop_fillBackColor, nFillBackColor );
                }
                break;
            }
            case drawing::FillStyle_NONE :
                AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
            break;
        }
        if ( eFS != drawing::FillStyle_NONE )
        {
            sal_uInt16 nTransparency = ( EscherPropertyValueHelper::GetPropertyValue(
                aAny, rXPropSet, "FillTransparence", true ) )
                ? *o3tl::doAccess<sal_Int16>(aAny) : 0;
            if (  nTransparency )
                AddOpt( ESCHER_Prop_fillOpacity, ( ( 100 - nTransparency ) << 16 ) / 100 );
        }
    }
    CreateLineProperties( rXPropSet, bEdge );
}

void EscherPropertyContainer::CreateTextProperties(
    const uno::Reference< beans::XPropertySet > & rXPropSet, sal_uInt32 nTextId,
        const bool bIsCustomShape, const bool bIsTextFrame )
{
    uno::Any aAny;
    text::WritingMode               eWM( text::WritingMode_LR_TB );
    drawing::TextVerticalAdjust     eVA( drawing::TextVerticalAdjust_TOP );
    drawing::TextHorizontalAdjust   eHA( drawing::TextHorizontalAdjust_LEFT );

    sal_Int32 nLeft             ( 0 );
    sal_Int32 nTop              ( 0 );
    sal_Int32 nRight            ( 0 );
    sal_Int32 nBottom           ( 0 );

    // used with normal shapes:
    bool bAutoGrowWidth     ( false );
    const bool bAutoGrowHeight    ( false ); //#ii63936 not setting autogrowheight, because minframeheight would be ignored
    // used with ashapes:
    bool bWordWrap          ( false );
    bool bAutoGrowSize      ( false );

    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "TextWritingMode", true ) )
        aAny >>= eWM;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "TextVerticalAdjust", true ) )
        aAny >>= eVA;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "TextHorizontalAdjust", true ) )
        aAny >>= eHA;
    if ( bIsCustomShape )
    {
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "TextWordWrap" ) )
            aAny >>= bWordWrap;
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "TextAutoGrowHeight", true ) )
            aAny >>= bAutoGrowSize;
    }
    else if ( bIsTextFrame )
    {
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "TextAutoGrowWidth", true ) )
            aAny >>= bAutoGrowWidth;

// i63936 not setting autogrowheight, because otherwise
// the minframeheight of the text will be ignored
//
//      if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "TextAutoGrowHeight", sal_True ) )
//          aAny >>= bAutoGrowHeight;
    }
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "TextLeftDistance" ) )
        aAny >>= nLeft;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "TextUpperDistance" ) )
        aAny >>= nTop;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "TextRightDistance" ) )
        aAny >>= nRight;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "TextLowerDistance" ) )
        aAny >>= nBottom;

    ESCHER_AnchorText eAnchor = ESCHER_AnchorTop;
    ESCHER_WrapMode eWrapMode = ESCHER_WrapSquare;
    sal_uInt32 nTextAttr = 0x40004;     // rotate text with shape

    if ( eWM == text::WritingMode_TB_RL )
    {   // vertical writing
        switch ( eHA )
        {
            case drawing::TextHorizontalAdjust_LEFT :
                eAnchor = ESCHER_AnchorBottom;
            break;
            case drawing::TextHorizontalAdjust_CENTER :
                eAnchor = ESCHER_AnchorMiddle;
            break;
            default :
            case drawing::TextHorizontalAdjust_BLOCK :
            case drawing::TextHorizontalAdjust_RIGHT :
                eAnchor = ESCHER_AnchorTop;
            break;
        }
        if ( eVA == drawing::TextVerticalAdjust_CENTER )
        {
            switch ( eAnchor )
            {
                case ESCHER_AnchorMiddle :
                    eAnchor = ESCHER_AnchorMiddleCentered;
                break;
                case ESCHER_AnchorBottom :
                    eAnchor = ESCHER_AnchorBottomCentered;
                break;
                default :
                case ESCHER_AnchorTop :
                    eAnchor = ESCHER_AnchorTopCentered;
                break;
            }
        }
        if ( bIsCustomShape )
        {
            if ( bWordWrap )
                eWrapMode = ESCHER_WrapSquare;
            else
                eWrapMode = ESCHER_WrapNone;
            if ( bAutoGrowSize )
                nTextAttr |= 0x20002;
        }
        else
        {
            if ( bAutoGrowHeight )
                eWrapMode = ESCHER_WrapNone;
            if ( bAutoGrowWidth )
                nTextAttr |= 0x20002;
        }

        AddOpt( ESCHER_Prop_txflTextFlow, ESCHER_txflTtoBA ); // rotate text within shape by 90
    }
    else
    {   // normal from left to right
        switch ( eVA )
        {
            case drawing::TextVerticalAdjust_CENTER :
                eAnchor = ESCHER_AnchorMiddle;
            break;

            case drawing::TextVerticalAdjust_BOTTOM :
                eAnchor = ESCHER_AnchorBottom;
            break;

            default :
            case drawing::TextVerticalAdjust_TOP :
                eAnchor = ESCHER_AnchorTop;
            break;
        }
        if ( eHA == drawing::TextHorizontalAdjust_CENTER )
        {
            switch( eAnchor )
            {
                case ESCHER_AnchorMiddle :
                    eAnchor = ESCHER_AnchorMiddleCentered;
                break;
                case ESCHER_AnchorBottom :
                    eAnchor = ESCHER_AnchorBottomCentered;
                break;
                case ESCHER_AnchorTop :
                    eAnchor = ESCHER_AnchorTopCentered;
                break;
                default: break;
            }
        }
        if ( bIsCustomShape )
        {
            if ( bWordWrap )
                eWrapMode = ESCHER_WrapSquare;
            else
                eWrapMode = ESCHER_WrapNone;
            if ( bAutoGrowSize )
                nTextAttr |= 0x20002;
        }
        else
        {
            if ( bAutoGrowWidth )
                eWrapMode = ESCHER_WrapNone;
            if ( bAutoGrowHeight )
                nTextAttr |= 0x20002;
        }
    }
    AddOpt( ESCHER_Prop_dxTextLeft, nLeft * 360 );
    AddOpt( ESCHER_Prop_dxTextRight, nRight * 360 );
    AddOpt( ESCHER_Prop_dyTextTop, nTop * 360 );
    AddOpt( ESCHER_Prop_dyTextBottom, nBottom * 360 );

    AddOpt( ESCHER_Prop_WrapText, eWrapMode );
    AddOpt( ESCHER_Prop_AnchorText, eAnchor );
    AddOpt( ESCHER_Prop_FitTextToShape, nTextAttr );

    if ( nTextId )
        AddOpt( ESCHER_Prop_lTxid, nTextId );

    // n#404221: In case of rotation we need to write the txtflTextFlow
    // attribute too.
    // fdo#58204: not custom shapes (TODO: other cases when it doesn't work?)
    if (!bIsTextFrame || bIsCustomShape)
        return;

    sal_uInt16 nAngle = EscherPropertyValueHelper::GetPropertyValue(
        aAny, rXPropSet, "RotateAngle", true ) ?
            static_cast<sal_uInt16>( ( *o3tl::doAccess<sal_Int32>(aAny) ) + 5 ) / 10 : 0;
    if (nAngle==900)
    {
        AddOpt( ESCHER_Prop_txflTextFlow, ESCHER_txflBtoT );
    }
    if (nAngle==2700)
    {
        AddOpt( ESCHER_Prop_txflTextFlow, ESCHER_txflTtoBA );
    }
}

bool EscherPropertyContainer::GetLineArrow( const bool bLineStart,
    const uno::Reference<beans::XPropertySet> & rXPropSet,
        ESCHER_LineEnd& reLineEnd, sal_Int32& rnArrowLength, sal_Int32& rnArrowWidth )
{
    const OUString sLine      ( bLineStart ? OUString("LineStart") : OUString("LineEnd") );
    const OUString sLineName  ( bLineStart ? OUString("LineStartName") : OUString("LineEndName") );

    bool bIsArrow = false;

    uno::Any aAny;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, sLine ) )
    {
        tools::PolyPolygon aPolyPoly( EscherPropertyContainer::GetPolyPolygon( aAny ) );
        if ( aPolyPoly.Count() && aPolyPoly[ 0 ].GetSize() )
        {
            bIsArrow = true;

            reLineEnd     = ESCHER_LineArrowEnd;
            rnArrowLength = 1;
            rnArrowWidth  = 1;

            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, sLineName ) )
            {
                OUString        aArrowStartName = *o3tl::doAccess<OUString>(aAny);
                sal_uInt16      nWhich = bLineStart ? sal_uInt16(XATTR_LINESTART) : sal_uInt16(XATTR_LINEEND);

                // remove extra space separated number
                sal_Int32 nPos = aArrowStartName.lastIndexOf(' ');
                if (nPos > -1 && aArrowStartName.lastIndexOf(' ', nPos) > -1)
                    aArrowStartName = aArrowStartName.copy(0, nPos);

                OUString aApiName = SvxUnogetApiNameForItem(nWhich, aArrowStartName);
                bool bIsMapped = true;
                if ( !aApiName.isEmpty() )
                {

                    // TODO: calculate the best option for ArrowLength and ArrowWidth
                    if ( aApiName == "Arrow concave" )
                        reLineEnd = ESCHER_LineArrowStealthEnd;
                    else if ( aApiName == "Square 45" )
                        reLineEnd = ESCHER_LineArrowDiamondEnd;
                    else if ( aApiName == "Small Arrow" )
                        reLineEnd = ESCHER_LineArrowEnd;
                    else if ( aApiName == "Dimension Lines" )
                    {
                        rnArrowLength = 0;
                        rnArrowWidth  = 2;
                        reLineEnd = ESCHER_LineArrowOvalEnd;
                    }
                    else if ( aApiName == "Double Arrow" )
                        reLineEnd = ESCHER_LineArrowEnd;
                    else if ( aApiName == "Rounded short Arrow" )
                        reLineEnd = ESCHER_LineArrowEnd;
                    else if ( aApiName == "Symmetric Arrow" )
                        reLineEnd = ESCHER_LineArrowEnd;
                    else if ( aApiName == "Line Arrow" )
                        reLineEnd = ESCHER_LineArrowOpenEnd;
                    else if ( aApiName == "Rounded large Arrow" )
                        reLineEnd = ESCHER_LineArrowEnd;
                    else if ( aApiName == "Circle" )
                        reLineEnd = ESCHER_LineArrowOvalEnd;
                    else if ( aApiName == "Square" )
                        reLineEnd = ESCHER_LineArrowDiamondEnd;
                    else if ( aApiName == "Arrow" )
                        reLineEnd = ESCHER_LineArrowEnd;
                    else
                        bIsMapped = false;

                }
                if ( !bIsMapped && comphelper::string::getTokenCount(aArrowStartName, ' ') == 2 )
                {
                    sal_Int32 nIdx{ 0 };
                    OUString aArrowName( aArrowStartName.getToken( 0, ' ', nIdx ) );
                    if (  aArrowName == "msArrowEnd" )
                        reLineEnd = ESCHER_LineArrowEnd;
                    else if (  aArrowName == "msArrowOpenEnd" )
                        reLineEnd = ESCHER_LineArrowOpenEnd;
                    else if ( aArrowName == "msArrowStealthEnd" )
                        reLineEnd = ESCHER_LineArrowStealthEnd;
                    else if ( aArrowName == "msArrowDiamondEnd" )
                        reLineEnd = ESCHER_LineArrowDiamondEnd;
                    else if ( aArrowName == "msArrowOvalEnd" )
                        reLineEnd = ESCHER_LineArrowOvalEnd;
                    else
                        nIdx = -1;

                    // now we have the arrow, and try to determine the arrow size;
                    if ( nIdx>0 )
                    {
                        OUString aArrowSize( aArrowStartName.getToken( 0, ' ', nIdx ) );
                        sal_Int32 nArrowSize = aArrowSize.toInt32();
                        rnArrowWidth = ( nArrowSize - 1 ) / 3;
                        rnArrowLength = nArrowSize - ( rnArrowWidth * 3 ) - 1;
                    }
                }
            }
        }
    }
    return bIsArrow;
}

void EscherPropertyContainer::CreateLineProperties(
    const uno::Reference<beans::XPropertySet> & rXPropSet, bool bEdge )
{
    uno::Any aAny;
    sal_uInt32 nLineFlags = 0x80008;

    ESCHER_LineEnd eLineEnd;
    sal_Int32 nArrowLength;
    sal_Int32 nArrowWidth;

    bool bSwapLineEnds = false;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "CircleKind", true ) )
    {
        drawing::CircleKind  eCircleKind;
        if ( aAny >>= eCircleKind )
        {
            if ( eCircleKind == drawing::CircleKind_ARC )
                bSwapLineEnds = true;
        }
    }
    if ( GetLineArrow( !bSwapLineEnds, rXPropSet, eLineEnd, nArrowLength, nArrowWidth ) )
    {
        AddOpt( ESCHER_Prop_lineStartArrowLength, nArrowLength );
        AddOpt( ESCHER_Prop_lineStartArrowWidth, nArrowWidth );
        AddOpt( ESCHER_Prop_lineStartArrowhead, eLineEnd );
        nLineFlags |= 0x100010;
    }
    if ( GetLineArrow( bSwapLineEnds, rXPropSet, eLineEnd, nArrowLength, nArrowWidth ) )
    {
        AddOpt( ESCHER_Prop_lineEndArrowLength, nArrowLength );
        AddOpt( ESCHER_Prop_lineEndArrowWidth, nArrowWidth );
        AddOpt( ESCHER_Prop_lineEndArrowhead, eLineEnd );
        nLineFlags |= 0x100010;
    }

    // support LineCaps
    if(EscherPropertyValueHelper::GetPropertyValue(aAny, rXPropSet, "LineCap"))
    {
        drawing::LineCap aLineCap(drawing::LineCap_BUTT);

        if(aAny >>= aLineCap)
        {
            switch (aLineCap)
            {
                default: /* drawing::LineCap_BUTT */
                {
                    AddOpt(ESCHER_Prop_lineEndCapStyle, ESCHER_LineEndCapFlat);
                    break;
                }
                case drawing::LineCap_ROUND:
                {
                    AddOpt(ESCHER_Prop_lineEndCapStyle, ESCHER_LineEndCapRound);
                    break;
                }
                case drawing::LineCap_SQUARE:
                {
                    AddOpt(ESCHER_Prop_lineEndCapStyle, ESCHER_LineEndCapSquare);
                    break;
                }
            }
        }
    }

    sal_uInt32 nLineWidth = ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "LineWidth" ) )
        ? *o3tl::doAccess<sal_uInt32>(aAny) : 0;
    if ( nLineWidth > 1 )
        AddOpt( ESCHER_Prop_lineWidth, nLineWidth * 360 );       // 100TH MM -> PT , 1PT = 12700 EMU

    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "LineStyle" ) )
    {
        drawing::LineStyle eLS;
        if ( aAny >>= eLS )
        {
            switch ( eLS )
            {
                case drawing::LineStyle_NONE :
                    AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );           // 80000
                break;

                case drawing::LineStyle_DASH :
                {
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "LineDash" ) )
                    {
                        ESCHER_LineDashing eDash = ESCHER_LineSolid;
                        auto pLineDash = o3tl::doAccess<drawing::LineDash>(aAny);
                        switch ( pLineDash->Style )
                        {
                            case drawing::DashStyle_ROUND :
                            case drawing::DashStyle_ROUNDRELATIVE :
                                AddOpt( ESCHER_Prop_lineEndCapStyle, 0 ); // set Style Round
                            break;
                            default : break;
                        }
                        // Try to detect exact prstDash styles. Use a similar method as in oox export.
                        // Map it to a roughly fitting prstDash in other cases.
                        bool bIsConverted = false;
                        bool bIsRelative = pLineDash->Style == drawing::DashStyle_RECTRELATIVE
                                            || pLineDash->Style == drawing::DashStyle_ROUNDRELATIVE;
                        sal_Int16 nDashes = pLineDash->Dashes;
                        sal_Int16 nDots = pLineDash->Dots;
                        sal_Int32 nDashLen = pLineDash->DashLen;
                        sal_Int32 nDotLen = pLineDash->DotLen;
                        sal_Int32 nDistance = pLineDash->Distance;

                        // Caution! The names are misleading. "dot" is always the first dash and "dash"
                        // the second one, regardless of the actual length. All prstDash
                        // definitions start with the longer dash and have exact one longer dash.
                        // Preset line style definitions for binary format are the same as for OOXML.
                        if (bIsRelative && nDots == 1)
                        {
                            // I'm not sure that LO always uses 100%, because in case of absolute values, LO
                            // sets length to 0 but treats it as 100%, if the attribute is missing in ODF.
                            // So to be sure set 100% explicitly in case of relative too.
                            if (nDashes > 0 && nDashLen == 0)
                                nDashLen = 100;
                            if (nDotLen == 0)
                                nDotLen = 100;
                            bIsConverted = true;
                            if (nDotLen == 100 && nDashes == 0 && nDashLen == 0 && nDistance == 300)
                                eDash = ESCHER_LineDotGEL;
                            else if (nDotLen == 400 && nDashes == 0 && nDashLen == 0 && nDistance == 300)
                                eDash = ESCHER_LineDashGEL;
                            else if (nDotLen == 400 && nDashes == 1 && nDashLen == 100 && nDistance == 300)
                                eDash = ESCHER_LineDashDotGEL;
                            else if (nDotLen == 800 && nDashes == 0 && nDashLen == 0 && nDistance == 300)
                                eDash = ESCHER_LineLongDashGEL;
                            else if (nDotLen == 800 && nDashes == 1 && nDashLen == 100 && nDistance == 300)
                                eDash = ESCHER_LineLongDashDotGEL;
                            else if (nDotLen == 800 && nDashes == 2 && nDashLen == 100 && nDistance == 300)
                                eDash = ESCHER_LineLongDashDotDotGEL;
                            else if (nDotLen == 100 && nDashes == 0 && nDashLen == 0 && nDistance == 100)
                                eDash = ESCHER_LineDotSys;
                            else if (nDotLen == 300 && nDashes == 0 && nDashLen == 0 && nDistance == 100)
                                eDash = ESCHER_LineDashSys;
                            else if (nDotLen == 300 && nDashes == 1 && nDashLen == 100 && nDistance == 100)
                                eDash = ESCHER_LineDashDotSys;
                            else if (nDotLen == 300 && nDashes == 2 && nDashLen == 100 && nDistance == 100)
                                eDash = ESCHER_LineDashDotDotSys;
                            else
                                bIsConverted = false;
                        }

                        if (!bIsConverted)
                        {   // Map the style roughly to preset line styles.
                            if (((!(pLineDash->Dots)) || (!(pLineDash->Dashes)))
                                || (pLineDash->DotLen == pLineDash->DashLen))
                            {
                                sal_Int32 nLen = pLineDash->DotLen;
                                if (pLineDash->Dashes)
                                    nLen = pLineDash->DashLen;
                                if (nLen >= nDistance)
                                    eDash = ESCHER_LineLongDashGEL;
                                else if (pLineDash->Dots)
                                    eDash = ESCHER_LineDotSys;
                                else
                                 eDash = ESCHER_LineDashGEL;
                            }
                            else                                                            // X Y
                            {
                                if (pLineDash->Dots != pLineDash->Dashes)
                                {
                                    if ((pLineDash->DashLen > nDistance) || (pLineDash->DotLen > nDistance))
                                        eDash = ESCHER_LineLongDashDotDotGEL;
                                    else
                                        eDash = ESCHER_LineDashDotDotSys;
                                }
                                else                                                        // X Y Y
                                {
                                    if ((pLineDash->DashLen > nDistance) || (pLineDash->DotLen > nDistance))
                                        eDash = ESCHER_LineLongDashDotGEL;
                                    else
                                        eDash = ESCHER_LineDashDotGEL;
                                }
                            }
                        }
                        AddOpt( ESCHER_Prop_lineDashing, eDash );
                    }
                }
                [[fallthrough]];
                case drawing::LineStyle_SOLID :
                default:
                {
                    AddOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );
                }
                break;
            }
        }
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "LineColor" ) )
        {
            sal_uInt32 nLineColor = ImplGetColor( *o3tl::doAccess<sal_uInt32>(aAny) );
            AddOpt( ESCHER_Prop_lineColor, nLineColor );
            AddOpt( ESCHER_Prop_lineBackColor, nLineColor ^ 0xffffff );
        }
    }

    ESCHER_LineJoin eLineJoin = ESCHER_LineJoinMiter;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "LineJoint", true ) )
    {
        drawing::LineJoint eLJ;
        if ( aAny >>= eLJ )
        {
            switch ( eLJ )
            {
                case drawing::LineJoint_NONE :
                case drawing::LineJoint_BEVEL :
                    eLineJoin = ESCHER_LineJoinBevel;
                break;
                default:
                case drawing::LineJoint_MIDDLE :
                case drawing::LineJoint_MITER :
                    eLineJoin = ESCHER_LineJoinMiter;
                break;
                case drawing::LineJoint_ROUND :
                    eLineJoin = ESCHER_LineJoinRound;
                break;
            }
        }
    }
    AddOpt( ESCHER_Prop_lineJoinStyle, eLineJoin );

    if ( EscherPropertyValueHelper::GetPropertyValue(
        aAny, rXPropSet, "LineTransparence", true ) )
    {
        sal_Int16 nTransparency = 0;
        if ( aAny >>= nTransparency )
            AddOpt( ESCHER_Prop_lineOpacity, ( ( 100 - nTransparency ) << 16 ) / 100 );
    }


    if ( !bEdge )
    {
        AddOpt( ESCHER_Prop_fFillOK, 0x1001 );
        AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
    }
}

static Size lcl_SizeToEmu(Size aPrefSize, const MapMode& aPrefMapMode)
{
    Size aRetSize;
    if (aPrefMapMode.GetMapUnit() == MapUnit::MapPixel)
        aRetSize = Application::GetDefaultDevice()->PixelToLogic(aPrefSize, MapMode(MapUnit::Map100thMM));
    else
        aRetSize = OutputDevice::LogicToLogic(aPrefSize, aPrefMapMode, MapMode(MapUnit::Map100thMM));
    return aRetSize;
}

void EscherPropertyContainer::ImplCreateGraphicAttributes( const uno::Reference<beans::XPropertySet> & rXPropSet,
                                                            sal_uInt32 nBlibId, bool bCreateCroppingAttributes )
{
    uno::Any aAny;

    sal_uInt32 nPicFlags = 0;
    drawing::ColorMode eColorMode( drawing::ColorMode_STANDARD );
    sal_Int16 nLuminance = 0;
    sal_Int32 nContrast = 0;

    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "GraphicColorMode" ) )
        aAny >>= eColorMode;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "AdjustLuminance" ) )
        aAny >>= nLuminance;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "AdjustContrast" ) )
    {
        sal_Int16 nC = sal_Int16();
        aAny >>= nC;
        nContrast = nC;
    }

    if ( eColorMode == drawing::ColorMode_WATERMARK )
    {
        eColorMode = drawing::ColorMode_STANDARD;
        nLuminance += 70;
        if ( nLuminance > 100 )
            nLuminance = 100;
        nContrast -= 70;
        if ( nContrast < -100 )
            nContrast = -100;
    }
    if ( eColorMode == drawing::ColorMode_GREYS )
        nPicFlags |= 0x40004;
    else if ( eColorMode == drawing::ColorMode_MONO )
        nPicFlags |= 0x60006;

    if ( nContrast )
    {
        nContrast += 100;
        if ( nContrast == 100)
            nContrast = 0x10000;
        else if ( nContrast < 100 )
        {
            nContrast *= 0x10000;
            nContrast /= 100;
        }
        else if ( nContrast < 200 )
            nContrast = ( 100 * 0x10000 ) / ( 200 - nContrast );
        else
            nContrast = 0x7fffffff;
        AddOpt( ESCHER_Prop_pictureContrast, nContrast );
    }
    if ( nLuminance )
        AddOpt( ESCHER_Prop_pictureBrightness, nLuminance * 327 );
    if ( nPicFlags )
        AddOpt( ESCHER_Prop_pictureActive, nPicFlags );

    if ( !(bCreateCroppingAttributes && pGraphicProvider) )
        return;

    Size    aPrefSize;
    MapMode aPrefMapMode;
    if ( !pGraphicProvider->GetPrefSize( nBlibId, aPrefSize, aPrefMapMode ) )
        return;

    Size aCropSize(lcl_SizeToEmu(aPrefSize, aPrefMapMode));
    if ( !(aCropSize.Width() && aCropSize.Height()) )
        return;

    if ( !EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "GraphicCrop" ) )
        return;

    text::GraphicCrop aGraphCrop;
    if ( !(aAny >>= aGraphCrop) )
        return;

    if ( aGraphCrop.Left )
    {
        sal_uInt32 nLeft = ( aGraphCrop.Left * 65536 ) / aCropSize.Width();
        AddOpt( ESCHER_Prop_cropFromLeft, nLeft );
    }
    if ( aGraphCrop.Top )
    {
        sal_uInt32 nTop = ( aGraphCrop.Top * 65536 ) / aCropSize.Height();
        AddOpt( ESCHER_Prop_cropFromTop, nTop );
    }
    if ( aGraphCrop.Right )
    {
        sal_uInt32 nRight = ( aGraphCrop.Right * 65536 ) / aCropSize.Width();
        AddOpt( ESCHER_Prop_cropFromRight, nRight );
    }
    if ( aGraphCrop.Bottom )
    {
        sal_uInt32 nBottom = ( aGraphCrop.Bottom * 65536 ) / aCropSize.Height();
        AddOpt( ESCHER_Prop_cropFromBottom, nBottom );
    }
}

void EscherPropertyContainer::CreateShapeProperties( const uno::Reference<drawing::XShape> & rXShape )
{
    uno::Reference< beans::XPropertySet > aXPropSet( rXShape, uno::UNO_QUERY );
    if ( !aXPropSet.is() )
        return;

    bool bVisible = false;
    bool bPrintable = false;
    uno::Any aAny;
    sal_uInt32 nShapeAttr = 0;
    if (EscherPropertyValueHelper::GetPropertyValue(aAny, aXPropSet, "Visible", true) && (aAny >>= bVisible))
    {
        if ( !bVisible )
            nShapeAttr |= 0x20002;  // set fHidden = true
    }
    // This property (fPrint) isn't used in Excel anymore, leaving it for legacy reasons
    // one change, based on XLSX: hidden implies not printed, let's not export the fPrint property in that case
    if (bVisible && EscherPropertyValueHelper::GetPropertyValue(aAny, aXPropSet, "Printable", true) && (aAny >>= bPrintable))
    {
        if ( !bPrintable )
            nShapeAttr |= 0x10000;  // set fPrint = false;
    }
    if ( nShapeAttr )
        AddOpt( ESCHER_Prop_fPrint, nShapeAttr );
}

bool EscherPropertyContainer::CreateOLEGraphicProperties(const uno::Reference<drawing::XShape> & rXShape)
{
    bool    bRetValue = false;

    if ( rXShape.is() )
    {
        SdrObject* pObject = SdrObject::getSdrObjectFromXShape(rXShape); // SJ: leaving unoapi, because currently there is
        if (auto pOle2Obj = dynamic_cast<const SdrOle2Obj*>(pObject)) // no access to the native graphic object
        {
            const Graphic* pGraphic = pOle2Obj->GetGraphic();
            if (pGraphic)
            {
                Graphic aGraphic(*pGraphic);
                GraphicObject aGraphicObject(aGraphic);
                bRetValue = CreateGraphicProperties(rXShape, aGraphicObject);
            }
        }
    }
    return bRetValue;
}

bool EscherPropertyContainer::CreateGraphicProperties(const uno::Reference<drawing::XShape> & rXShape, const GraphicObject& rGraphicObj)
{
    bool bRetValue = false;
    OString aUniqueId(rGraphicObj.GetUniqueID());
    if ( !aUniqueId.isEmpty() )
    {
        AddOpt( ESCHER_Prop_fillType, ESCHER_FillPicture );
        uno::Reference< beans::XPropertySet > aXPropSet( rXShape, uno::UNO_QUERY );

        if ( pGraphicProvider && pPicOutStrm && pShapeBoundRect && aXPropSet.is() )
        {
            uno::Any aAny;
            std::unique_ptr<awt::Rectangle> pVisArea;
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "VisibleArea" ) )
            {
                pVisArea.reset(new awt::Rectangle);
                aAny >>= *pVisArea;
            }
            sal_uInt32 nBlibId = pGraphicProvider->GetBlibID( *pPicOutStrm, rGraphicObj, pVisArea.get() );
            if ( nBlibId )
            {
                AddOpt( ESCHER_Prop_pib, nBlibId, true );
                ImplCreateGraphicAttributes( aXPropSet, nBlibId, false );
                bRetValue = true;
            }
        }
    }
    return bRetValue;
}

bool EscherPropertyContainer::CreateMediaGraphicProperties(const uno::Reference<drawing::XShape> & rXShape)
{
    bool    bRetValue = false;
    if ( rXShape.is() )
    {
        SdrObject* pSdrObject(SdrObject::getSdrObjectFromXShape(rXShape));  // SJ: leaving unoapi, because currently there is
        if (auto pSdrMediaObj = dynamic_cast<const SdrMediaObj*>(pSdrObject)) // no access to the native graphic object
        {
            GraphicObject aGraphicObject(pSdrMediaObj->getSnapshot());
            bRetValue = CreateGraphicProperties(rXShape, aGraphicObject);
        }
    }
    return bRetValue;
}

bool EscherPropertyContainer::ImplCreateEmbeddedBmp(GraphicObject const & rGraphicObject)
{
    if (rGraphicObject.GetType() != GraphicType::NONE)
    {
        EscherGraphicProvider aProvider;
        SvMemoryStream aMemStrm;

        if (aProvider.GetBlibID( aMemStrm, rGraphicObject))
        {
            AddOpt(ESCHER_Prop_fillBlip, true, 0, aMemStrm);
            return true;
        }
    }
    return false;
}

void EscherPropertyContainer::CreateEmbeddedBitmapProperties(
    uno::Reference<awt::XBitmap> const & rxBitmap, drawing::BitmapMode eBitmapMode )
{
    uno::Reference<graphic::XGraphic> xGraphic(rxBitmap, uno::UNO_QUERY);
    if (!xGraphic.is())
        return;
    const Graphic aGraphic(xGraphic);
    if (aGraphic.IsNone())
        return;
    const GraphicObject aGraphicObject(aGraphic);
    if (aGraphicObject.GetType() == GraphicType::NONE)
        return;
    if (ImplCreateEmbeddedBmp(aGraphicObject))
    {
        // bitmap mode property
        bool bRepeat = eBitmapMode == drawing::BitmapMode_REPEAT;
        AddOpt( ESCHER_Prop_fillType, bRepeat ? ESCHER_FillTexture : ESCHER_FillPicture );
    }
}

namespace {

Graphic lclDrawHatch( const drawing::Hatch& rHatch, const Color& rBackColor, bool bFillBackground, const tools::Rectangle& rRect )
{
    // #i121183# For hatch, do no longer create a bitmap with the fixed size of 28x28 pixels. Also
    // do not create a bitmap in page size, that would explode file sizes (and have no good quality).
    // Better use a MetaFile graphic in page size; thus we have good quality due to vector format and
    // no bit file sizes.
    ScopedVclPtrInstance< VirtualDevice > pVDev;
    GDIMetaFile aMtf;

    pVDev->SetOutputSizePixel(Size(2, 2));
    pVDev->EnableOutput(false);
    pVDev->SetMapMode(MapMode(MapUnit::Map100thMM));
    aMtf.Clear();
    aMtf.Record(pVDev);
    pVDev->SetLineColor();
    pVDev->SetFillColor(bFillBackground ? rBackColor : COL_TRANSPARENT);
    pVDev->DrawRect(rRect);
    pVDev->DrawHatch(tools::PolyPolygon(rRect), Hatch(static_cast<HatchStyle>(rHatch.Style), Color(ColorTransparency, rHatch.Color), rHatch.Distance,
            Degree10(rHatch.Angle)));
    aMtf.Stop();
    aMtf.WindStart();
    aMtf.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
    aMtf.SetPrefSize(rRect.GetSize());

    return Graphic(aMtf);
}

} // namespace

void EscherPropertyContainer::CreateEmbeddedHatchProperties(const drawing::Hatch& rHatch, const Color& rBackColor, bool bFillBackground )
{
    const tools::Rectangle aRect(pShapeBoundRect ? *pShapeBoundRect : tools::Rectangle(Point(0,0), Size(28000, 21000)));
    Graphic aGraphic(lclDrawHatch(rHatch, rBackColor, bFillBackground, aRect));
    GraphicObject aGraphicObject(aGraphic);

    if (ImplCreateEmbeddedBmp(aGraphicObject))
        AddOpt( ESCHER_Prop_fillType, ESCHER_FillTexture );
}

bool EscherPropertyContainer::CreateGraphicProperties(const uno::Reference<beans::XPropertySet> & rXPropSet,
                                                      const OUString& rSource,
                                                      const bool bCreateFillBitmap,
                                                      const bool bCreateCroppingAttributes,
                                                      const bool bFillBitmapModeAllowed,
                                                      const bool bOOxmlExport )
{
    bool        bRetValue = false;
    bool        bCreateFillStyles = false;

    std::unique_ptr<GraphicAttr> pGraphicAttr;
    uno::Reference<graphic::XGraphic> xGraphic;

    uno::Any aAny;

    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, rSource ) )
    {
        bool bMirrored = false;
        bool bRotate = true;
        bool bIsGraphicMtf = false;
        sal_Int16 nTransparency(0);
        sal_Int16 nRed(0);
        sal_Int16 nGreen(0);
        sal_Int16 nBlue(0);
        double fGamma(1.0);
        drawing::BitmapMode eBitmapMode(drawing::BitmapMode_NO_REPEAT);
        OUString aGraphicUrl;

        sal_uInt16 nAngle = 0;
        if ( rSource == "MetaFile" )
        {
            auto & aSeq = *o3tl::doAccess<uno::Sequence<sal_Int8>>(aAny);
            const sal_Int8* pArray = aSeq.getConstArray();
            sal_uInt32 nArrayLength = aSeq.getLength();

            // the metafile is already rotated
            bRotate = false;

            if (pArray && nArrayLength)
            {
                Graphic aGraphic;
                SvMemoryStream  aStream(const_cast<sal_Int8 *>(pArray), nArrayLength, StreamMode::READ);
                ErrCode nErrCode = GraphicConverter::Import(aStream, aGraphic, ConvertDataFormat::WMF);
                if ( nErrCode == ERRCODE_NONE )
                {
                    xGraphic = aGraphic.GetXGraphic();
                    bIsGraphicMtf = aGraphic.GetType() == GraphicType::GdiMetafile;
                }
            }
        }
        else if (rSource == "Bitmap" || rSource == "FillBitmap")
        {
            auto xBitmap = aAny.get<uno::Reference<awt::XBitmap>>();
            if (xBitmap.is())
            {
                xGraphic.set(xBitmap, uno::UNO_QUERY);
                Graphic aGraphic(xGraphic);
                bIsGraphicMtf = aGraphic.GetType() == GraphicType::GdiMetafile;
            }
        }
        else if ( rSource == "Graphic" )
        {
            xGraphic = aAny.get<uno::Reference<graphic::XGraphic>>();
            bCreateFillStyles = true;
        }
        else if ( rSource == "FillHatch" )
        {
            drawing::Hatch aHatch;
            if ( aAny >>= aHatch )
            {
                Color aBackColor;
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "FillColor" ) )
                {
                    aBackColor = Color(ColorTransparency, ImplGetColor( *o3tl::doAccess<sal_uInt32>(aAny), false ));
                }
                bool bFillBackground = false;
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "FillBackground", true ) )
                {
                    aAny >>= bFillBackground;
                }

                const tools::Rectangle aRect(Point(0, 0), pShapeBoundRect ? pShapeBoundRect->GetSize() : Size(28000, 21000));
                Graphic aGraphic(lclDrawHatch(aHatch, aBackColor, bFillBackground, aRect));
                xGraphic = aGraphic.GetXGraphic();
                eBitmapMode = drawing::BitmapMode_REPEAT;
                bIsGraphicMtf = aGraphic.GetType() == GraphicType::GdiMetafile;
            }
        }

        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "IsMirrored", true ) )
            aAny >>= bMirrored;

        // #121074# transparency of graphic is not supported in MS formats, get and apply it
        // in the GetTransformedGraphic call in GetBlibID
        if(EscherPropertyValueHelper::GetPropertyValue(aAny, rXPropSet, "Transparency"))
        {
            aAny >>= nTransparency;
        }

        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "AdjustRed" ) )
        {
            aAny >>= nRed;
        }

        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "AdjustGreen" ) )
        {
            aAny >>= nGreen;
        }

        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "AdjustBlue" ) )
        {
            aAny >>= nBlue;
        }

        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "Gamma" ) )
        {
            aAny >>= fGamma;
        }

        if ( bCreateFillBitmap && bFillBitmapModeAllowed )
        {
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "FillBitmapMode", true ) )
                aAny >>= eBitmapMode;
        }
        else
        {
            nAngle = bRotate && EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "RotateAngle", true )
                ? static_cast<sal_uInt16>( ( *o3tl::doAccess<sal_Int32>(aAny) ) + 5 ) / 10
                : 0;
        }

        if (xGraphic.is())
        {
            Graphic aGraphic(xGraphic);
            aGraphicUrl = aGraphic.getOriginURL();
        }

        if (!aGraphicUrl.isEmpty())
        {
            bool bConverted = false;

            // externally, linked graphic? convert to embedded
            // one, if transformations are needed. this is because
            // everything < msoxp cannot even handle rotated
            // bitmaps.
            // And check whether the graphic link target is
            // actually supported by mso.
            INetURLObject   aTmp( aGraphicUrl );
            GraphicDescriptor aDescriptor(aTmp);
            (void)aDescriptor.Detect();
            const GraphicFileFormat nFormat = aDescriptor.GetFileFormat();

            // can MSO handle it?
            if ( bMirrored || nAngle || nTransparency || nRed || nGreen || nBlue || (1.0 != fGamma) ||
                 (nFormat != GraphicFileFormat::BMP &&
                  nFormat != GraphicFileFormat::GIF &&
                  nFormat != GraphicFileFormat::JPG &&
                  nFormat != GraphicFileFormat::PNG &&
                  nFormat != GraphicFileFormat::TIF &&
                  nFormat != GraphicFileFormat::PCT &&
                  nFormat != GraphicFileFormat::WMF &&
                  nFormat != GraphicFileFormat::EMF) )
            {
                std::unique_ptr<SvStream> pIn(::utl::UcbStreamHelper::CreateStream(
                    aTmp.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ ));
                if ( pIn )
                {
                    Graphic aGraphic;
                    ErrCode nErrCode = GraphicConverter::Import( *pIn, aGraphic );

                    if ( nErrCode == ERRCODE_NONE )
                    {
                        xGraphic = aGraphic.GetXGraphic();
                        bConverted = true;
                    }
                    // else: simply keep the graphic link
                }
            }

            if (!bConverted && pGraphicProvider )
            {
                const OUString& rBaseURI( pGraphicProvider->GetBaseURI() );
                INetURLObject aBaseURI( rBaseURI );
                if( aBaseURI.GetProtocol() == aTmp.GetProtocol() )
                {
                    OUString aRelUrl( INetURLObject::GetRelURL( rBaseURI, aGraphicUrl ) );
                    if ( !aRelUrl.isEmpty() )
                        aGraphicUrl = aRelUrl;
                }
            }
        }

        if (!aGraphicUrl.isEmpty() || xGraphic.is())
        {
            if(bMirrored || nTransparency || nRed || nGreen || nBlue || (1.0 != fGamma))
            {
                pGraphicAttr.reset(new GraphicAttr);

                if(bMirrored)
                {
                    pGraphicAttr->SetMirrorFlags(BmpMirrorFlags::Horizontal);
                }

                if(nTransparency)
                {
                    pGraphicAttr->SetAlpha(255 - (nTransparency * 255) / 100);
                }

                if(nRed)
                {
                    pGraphicAttr->SetChannelR(nRed);
                }

                if(nGreen)
                {
                    pGraphicAttr->SetChannelG(nGreen);
                }

                if(nBlue)
                {
                    pGraphicAttr->SetChannelB(nBlue);
                }

                if(1.0 != fGamma)
                {
                    pGraphicAttr->SetGamma(fGamma);
                }
            }

            if(nAngle && bIsGraphicMtf)
            {
                AddOpt( ESCHER_Prop_Rotation, ( ( (static_cast<sal_Int32>(nAngle) << 16 ) / 10 ) + 0x8000 ) &~ 0xffff );
            }

            if ( eBitmapMode == drawing::BitmapMode_REPEAT )
            {
                sal_Int32 nSizeX = 0,nSizeY = 0,nOffsetX = 0,nOffsetY = 0,nPosOffsetX = 0,nPosOffsetY = 0;
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "FillBitmapSizeX", true ) )
                {
                    aAny >>= nSizeX;
                }
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "FillBitmapSizeY", true ) )
                {
                    aAny >>= nSizeY;
                }
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "FillBitmapOffsetX", true ) )
                {
                    aAny >>= nOffsetX;
                }
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "FillBitmapOffsetY", true ) )
                {
                    aAny >>= nOffsetY;
                }
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "FillBitmapPositionOffsetX", true ) )
                {
                    aAny >>= nPosOffsetX;
                }
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "FillBitmapPositionOffsetY", true ) )
                {
                    aAny >>= nPosOffsetY;
                }
                if(nSizeX == -100 && nSizeY == -100 && nOffsetX == 0 && nOffsetY == 0 && nPosOffsetX == 0 && nPosOffsetY == 0)
                    AddOpt( ESCHER_Prop_fillType, ESCHER_FillPicture );
                else
                    AddOpt( ESCHER_Prop_fillType, ESCHER_FillTexture );
            }
            else
                AddOpt( ESCHER_Prop_fillType, ESCHER_FillPicture );

            if (xGraphic.is())
            {
                Graphic aGraphic(xGraphic);
                if (!aGraphic.getOriginURL().isEmpty())
                {
                    AddOpt(ESCHER_Prop_pibName, aGraphicUrl);
                    sal_uInt32 nPibFlags = 0;
                    GetOpt(ESCHER_Prop_pibFlags, nPibFlags);
                    AddOpt(ESCHER_Prop_pibFlags, ESCHER_BlipFlagLinkToFile | ESCHER_BlipFlagFile | ESCHER_BlipFlagDoNotSave | nPibFlags);
                }
                else if (pGraphicProvider && pPicOutStrm && pShapeBoundRect) // write out embedded graphic
                {
                    GraphicObject aGraphicObject(aGraphic);
                    const sal_uInt32 nBlibId(pGraphicProvider->GetBlibID(*pPicOutStrm, aGraphicObject, nullptr, pGraphicAttr.get()));

                    if(nBlibId)
                    {
                        if(bCreateFillBitmap)
                        {
                            AddOpt(ESCHER_Prop_fillBlip, nBlibId, true);
                        }
                        else
                        {
                            AddOpt( ESCHER_Prop_pib, nBlibId, true );
                            ImplCreateGraphicAttributes( rXPropSet, nBlibId, bCreateCroppingAttributes );
                        }

                        bRetValue = true;
                    }
                }
                else
                {
                    EscherGraphicProvider aProvider;
                    SvMemoryStream aMemStrm;
                    GraphicObject aGraphicObject(aGraphic);

                    if (aProvider.GetBlibID(aMemStrm, aGraphicObject, nullptr, pGraphicAttr.get(), bOOxmlExport))
                    {
                        AddOpt(ESCHER_Prop_fillBlip, true, 0, aMemStrm);
                        bRetValue = true;
                    }
                }
            }
        }
    }
    pGraphicAttr.reset();
    if ( bCreateFillStyles )
        CreateFillProperties( rXPropSet, true );

    return bRetValue;
}

tools::PolyPolygon EscherPropertyContainer::GetPolyPolygon( const uno::Reference< drawing::XShape > & rXShape )
{
    tools::PolyPolygon aRetPolyPoly;
    uno::Reference< beans::XPropertySet > aXPropSet;
    uno::Any aAny( rXShape->queryInterface(
        cppu::UnoType<beans::XPropertySet>::get()));

    if ( aAny >>= aXPropSet )
    {
        bool bHasProperty = EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "PolyPolygonBezier", true );
        if ( !bHasProperty )
            bHasProperty = EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "PolyPolygon", true );
        if ( !bHasProperty )
            bHasProperty = EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "Polygon", true );
        if ( bHasProperty )
            aRetPolyPoly = GetPolyPolygon( aAny );
    }
    return aRetPolyPoly;
}

// adapting to basegfx::B2DPolyPolygon now, has no sense to do corrections in the
// old tools::PolyPolygon creation code. Convert to that at return time
tools::PolyPolygon EscherPropertyContainer::GetPolyPolygon( const uno::Any& rAny )
{
    basegfx::B2DPolyPolygon aRetval;

    if(auto pBCC = o3tl::tryAccess<drawing::PolyPolygonBezierCoords>(rAny))
    {
        aRetval = basegfx::utils::UnoPolyPolygonBezierCoordsToB2DPolyPolygon(*pBCC);
    }
    else if(auto pCC = o3tl::tryAccess<drawing::PointSequenceSequence>(rAny))
    {
        aRetval = basegfx::utils::UnoPointSequenceSequenceToB2DPolyPolygon(*pCC);
    }
    else if(auto pC = o3tl::tryAccess<drawing::PointSequence>(rAny))
    {
        aRetval.append(basegfx::utils::UnoPointSequenceToB2DPolygon(*pC));
    }

    basegfx::B2DPolyPolygon aRetval2;

    for(sal_uInt32 a(0); a < aRetval.count(); a++)
    {
        if(0 != aRetval.getB2DPolygon(a).count())
        {
            aRetval2.append(aRetval.getB2DPolygon(a));
        }
    }

    return tools::PolyPolygon(aRetval2);
}

bool EscherPropertyContainer::CreatePolygonProperties(
    const uno::Reference<beans::XPropertySet> & rXPropSet,
    sal_uInt32 nFlags,
    bool bBezier,
    awt::Rectangle& rGeoRect,
    tools::Polygon const * pPolygon )
{
    tools::PolyPolygon aPolyPolygon;

    if(nullptr != pPolygon)
    {
        aPolyPolygon.Insert(*pPolygon);
    }
    else
    {
        uno::Any aAny;

        if(EscherPropertyValueHelper::GetPropertyValue(
            aAny,
            rXPropSet,
            bBezier ? OUString("PolyPolygonBezier") : OUString("PolyPolygon"),
            true))
        {
            aPolyPolygon = GetPolyPolygon(aAny);
        }
        else
        {
            return false;
        }
    }

    if(0 == aPolyPolygon.Count())
    {
        return false;
    }

    if(0 != (nFlags & ESCHER_CREATEPOLYGON_LINE))
    {
        if((1 == aPolyPolygon.Count()) && (2 == aPolyPolygon[0].GetSize()))
        {
            const tools::Polygon& rPoly(aPolyPolygon[0]);

            rGeoRect = awt::Rectangle(
                rPoly[0].X(),
                rPoly[0].Y(),
                rPoly[1].X() - rPoly[0].X(),
                rPoly[1].Y() - rPoly[0].Y());

            return true;
        }

        return false;
    }

    const tools::Rectangle aRect(aPolyPolygon.GetBoundRect());

    rGeoRect = awt::Rectangle(
        aRect.Left(),
        aRect.Top(),
        aRect.GetWidth(),
        aRect.GetHeight());

    const sal_uInt16 nPolyCount(aPolyPolygon.Count());
    sal_uInt32 nTotalPoints(0);

    std::vector< sal_uInt8 > aVertices
    {
        0, 0, 0, 0,
        static_cast<sal_uInt8>(0xf0),
        static_cast<sal_uInt8>(0xff)
    };

    std::vector< sal_uInt8 > aSegments
    {
        0, 0, 0, 0,
        static_cast<sal_uInt8>(2),
        static_cast<sal_uInt8>(0)
    };

    for(sal_uInt16 j(0); j < nPolyCount; ++j)
    {
        const tools::Polygon aPolygon(aPolyPolygon[j]);
        const sal_uInt16 nPoints(aPolygon.GetSize());

        if(0 == nPoints)
        {
            continue;
        }

        // Polygon start
        aSegments.push_back(static_cast<sal_uInt8>(0x0));
        aSegments.push_back(static_cast<sal_uInt8>(0x40));

        sal_uInt16 nSegmentIgnoreCounter(0);

        // write points from polygon to buffer
        for(sal_uInt16 i(0); i < nPoints; ++i)
        {
            Point aPoint(aPolygon[i]);

            aPoint.AdjustX(-(rGeoRect.X));
            aPoint.AdjustY(-(rGeoRect.Y));

            aVertices.push_back(static_cast<sal_uInt8>(aPoint.X()));
            aVertices.push_back(static_cast<sal_uInt8>(aPoint.X() >> 8));
            aVertices.push_back(static_cast<sal_uInt8>(aPoint.Y()));
            aVertices.push_back(static_cast<sal_uInt8>(aPoint.Y() >> 8));

            nTotalPoints++;

            if(0 != nSegmentIgnoreCounter)
            {
                nSegmentIgnoreCounter--;
            }
            else
            {
                aSegments.push_back(static_cast<sal_uInt8>(0));

                if(bBezier)
                {
                    aSegments.push_back(static_cast<sal_uInt8>(0xb3));
                }
                else
                {
                    aSegments.push_back(static_cast<sal_uInt8>(0xac));
                }

                if(i + 1 == nPoints)
                {
                    if(nPolyCount > 1)
                    {
                        // end of polygon
                        aSegments.push_back(static_cast<sal_uInt8>(1));
                        aSegments.push_back(static_cast<sal_uInt8>(0x60));
                    }
                }
                else
                {
                    aSegments.push_back(static_cast<sal_uInt8>(1));

                    if(PolyFlags::Control == aPolygon.GetFlags(i + 1))
                    {
                        aSegments.push_back(static_cast<sal_uInt8>(0x20));
                        nSegmentIgnoreCounter = 2;
                    }
                    else
                    {
                        aSegments.push_back(static_cast<sal_uInt8>(0));
                    }
                }
            }
        }
    }

    if(0 != nTotalPoints && aSegments.size() >= 6 && aVertices.size() >= 6)
    {
        // Little endian
        aVertices[0] = static_cast<sal_uInt8>(nTotalPoints);
        aVertices[1] = static_cast<sal_uInt8>(nTotalPoints >> 8);
        aVertices[2] = static_cast<sal_uInt8>(nTotalPoints);
        aVertices[3] = static_cast<sal_uInt8>(nTotalPoints >> 8);

        aSegments.push_back(static_cast<sal_uInt8>(0));
        aSegments.push_back(static_cast<sal_uInt8>(0x80));

        const sal_uInt32 nSegmentBufSize(aSegments.size() - 6);
        aSegments[0] = static_cast<sal_uInt8>(nSegmentBufSize >> 1);
        aSegments[1] = static_cast<sal_uInt8>(nSegmentBufSize >> 9);
        aSegments[2] = static_cast<sal_uInt8>(nSegmentBufSize >> 1);
        aSegments[3] = static_cast<sal_uInt8>(nSegmentBufSize >> 9);

        AddOpt(
            ESCHER_Prop_geoRight,
            rGeoRect.Width);
        AddOpt(
            ESCHER_Prop_geoBottom,
            rGeoRect.Height);
        AddOpt(
            ESCHER_Prop_shapePath,
            ESCHER_ShapeComplex);
        AddOpt(
            ESCHER_Prop_pVertices,
            true,
            aVertices.size() - 6,
            aVertices);
        AddOpt(
            ESCHER_Prop_pSegmentInfo,
            true,
            aSegments.size(),
            aSegments);

        return true;
    }

    return false;
}


/*
in MS,the connector including 9 types :
"straightConnector1",
"bentConnector2","bentConnector3","bentConnector4","bentConnector5"
"curvedConnector2","curvedConnector3","curvedConnector4","curvedConnector5"
in AOO,including 4 types:"standard","lines","line","curve"
when save as MS file, the connector must be convert to corresponding type.
"line" and "lines" <-> "straightConnector1"
"standard" <->  "bentConnector2-5"
"curve" <-> "curvedConnector2-5"
*/
static sal_Int32 lcl_GetAdjustValueCount( const XPolygon& rPoly )
{
    int nRet = 0;
    switch (  rPoly.GetSize() )
    {
    case 2 :
    case 3:
        nRet =  0;
        break;
    case 4:
        nRet = 1;
        break;
    case 5:
        nRet = 2;
        break;
    default:
        if ( rPoly.GetSize()>=6 )
            nRet = 3;
        break;
    }
    return nRet;
}

// Adjust value decide the position which connector should turn a corner
static sal_Int32 lcl_GetConnectorAdjustValue ( const XPolygon& rPoly, sal_uInt16 nIndex )
{
    sal_uInt16 k =  rPoly.GetSize();
    OSL_ASSERT ( k >= ( 3 + nIndex ) );

    Point aPt;
    Point aStart = rPoly[0];
    Point aEnd = rPoly[k-1];
    if ( aEnd.Y() == aStart.Y() )
        aEnd.setY( aStart.Y() +4 );
    if ( aEnd.X() == aStart.X() )
        aEnd.setX( aStart.X() +4 );

    bool bVertical = ( rPoly[1].X()-aStart.X() ) == 0 ;
    // vertical and horizon alternate
    if ( nIndex%2 == 1 ) bVertical = !bVertical;
    aPt = rPoly[ nIndex + 1];

    sal_Int32 nAdjustValue;
    if ( bVertical )
        nAdjustValue = ( aPt.Y()-aStart.Y())* 21600 /(aEnd.Y()-aStart.Y());
    else
        nAdjustValue = ( aPt.X()-aStart.X() )* 21600 /(aEnd.X()-aStart.X());

    return nAdjustValue;
}


static void lcl_Rotate(Degree100 nAngle, Point center, Point& pt)
{
    nAngle = NormAngle36000(nAngle);

    int cs, sn;
    switch (nAngle.get())
    {
    case 0:
        cs =1;
        sn =0;
        break;
    case 9000:
        cs =0;
        sn =1;
        break;
    case 18000:
        cs = -1;
        sn = 0;
        break;
    case 27000:
        cs = 0;
        sn = -1;
        break;
    default:
        return;
    }
    sal_Int32 x0 =pt.X()-center.X();
    sal_Int32 y0 =pt.Y()-center.Y();
    pt.setX(center.X()+ x0*cs-y0*sn );
    pt.setY(center.Y()+ y0*cs+x0*sn );
}
/*
 FlipV defines that the shape will be flipped vertically about the center of its bounding box.
Generally, draw the connector from top to bottom, from left to right when meet the adjust value,
but when (X1>X2 or Y1>Y2),the draw director must be reverse, FlipV or FlipH should be set to true.
*/
static bool lcl_GetAngle(tools::Polygon &rPoly, ShapeFlag& rShapeFlags,sal_Int32& nAngle )
{
    Point aStart = rPoly[0];
    Point aEnd = rPoly[rPoly.GetSize()-1];
    nAngle = ( rPoly[1].X() == aStart.X() ) ? 9000: 0 ;
    Point p1(aStart.X(),aStart.Y());
    Point p2(aEnd.X(),aEnd.Y());
    if ( nAngle )
    {
        Point center((aEnd.X()+aStart.X())>>1,(aEnd.Y()+aStart.Y())>>1);
        lcl_Rotate(Degree100(-nAngle), center,p1);
        lcl_Rotate(Degree100(-nAngle), center,p2);
    }
    if (  p1.X() > p2.X() )
    {
        if ( nAngle )
            rShapeFlags |= ShapeFlag::FlipV;
        else
            rShapeFlags |= ShapeFlag::FlipH;

    }
    if (  p1.Y() > p2.Y()  )
    {
        if ( nAngle )
            rShapeFlags |= ShapeFlag::FlipH;
        else
            rShapeFlags |= ShapeFlag::FlipV;
    }

    if ( (rShapeFlags&ShapeFlag::FlipH) && (rShapeFlags&ShapeFlag::FlipV) )
    {
        rShapeFlags  &= ~ShapeFlag( ShapeFlag::FlipH | ShapeFlag::FlipV );
        nAngle +=18000;
    }

    if ( nAngle )
    {
        // Set angle properties
        nAngle *= 655;
        nAngle += 0x8000;
        nAngle &=~0xffff;                                  // round nAngle to whole number of degrees
        return true;
    }
    return false;
}
bool EscherPropertyContainer::CreateConnectorProperties(
    const uno::Reference<drawing::XShape> & rXShape,
    EscherSolverContainer& rSolverContainer, awt::Rectangle& rGeoRect,
            sal_uInt16& rShapeType, ShapeFlag& rShapeFlags )
{
    bool bRetValue = false;
    rShapeType = 0;
    rShapeFlags = ShapeFlag::NONE;

    if ( rXShape.is() )
    {
        uno::Reference<beans::XPropertySet> aXPropSet;
        uno::Reference<drawing::XShape> aShapeA, aShapeB;
        uno::Any aAny( rXShape->queryInterface( cppu::UnoType<beans::XPropertySet>::get()));
        if ( aAny >>= aXPropSet )
        {
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "EdgeKind", true ) )
            {
                drawing::ConnectorType eCt;
                aAny >>= eCt;
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "EdgeStartPoint" ) )
                {
                    awt::Point aStartPoint = *o3tl::doAccess<awt::Point>(aAny);
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "EdgeEndPoint" ) )
                    {
                        awt::Point aEndPoint = *o3tl::doAccess<awt::Point>(aAny);

                        rShapeFlags = ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty | ShapeFlag::Connector;
                        rGeoRect = awt::Rectangle( aStartPoint.X, aStartPoint.Y,
                                                            ( aEndPoint.X - aStartPoint.X ) + 1, ( aEndPoint.Y - aStartPoint.Y ) + 1 );
                        // set standard's FLIP in below code
                        if ( eCt != drawing::ConnectorType_STANDARD)
                        {
                            if ( rGeoRect.Height < 0 )          // justify
                            {
                                rShapeFlags |= ShapeFlag::FlipV;
                                rGeoRect.Y = aEndPoint.Y;
                                rGeoRect.Height = -rGeoRect.Height;
                            }
                            if ( rGeoRect.Width < 0 )
                            {
                                rShapeFlags |= ShapeFlag::FlipH;
                                rGeoRect.X = aEndPoint.X;
                                rGeoRect.Width = -rGeoRect.Width;
                            }
                        }
                        sal_uInt32 nAdjustValue1, nAdjustValue2;
                        nAdjustValue1 = nAdjustValue2 = 0x2a30;

                        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "EdgeStartConnection" ) )
                            aAny >>= aShapeA;
                        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "EdgeEndConnection" ) )
                            aAny >>= aShapeB;
                        rSolverContainer.AddConnector( rXShape, aStartPoint, aShapeA, aEndPoint, aShapeB );
                        switch ( eCt )
                        {
                            case drawing::ConnectorType_CURVE :
                            {
                                rShapeType = ESCHER_ShpInst_CurvedConnector3;
                                AddOpt( ESCHER_Prop_cxstyle, ESCHER_cxstyleCurved );
                                AddOpt( ESCHER_Prop_adjustValue, nAdjustValue1 );
                                AddOpt( ESCHER_Prop_adjust2Value, -static_cast<sal_Int32>(nAdjustValue2) );
                            }
                            break;

                            case drawing::ConnectorType_STANDARD :// Connector 2->5
                                {
                                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "PolyPolygonBezier" ) )
                                    {
                                        tools::PolyPolygon aPolyPolygon = GetPolyPolygon( aAny );
                                        tools::Polygon aPoly;
                                        if ( aPolyPolygon.Count() > 0 )
                                        {
                                            AddOpt( ESCHER_Prop_cxstyle, ESCHER_cxstyleBent );
                                            aPoly = aPolyPolygon[ 0 ];
                                            sal_Int32 nAdjCount = lcl_GetAdjustValueCount( aPoly );
                                            rShapeType = static_cast<sal_uInt16>( ESCHER_ShpInst_BentConnector2 + nAdjCount);
                                            for ( sal_Int32 i = 0 ; i < nAdjCount; ++ i)
                                                AddOpt( static_cast<sal_uInt16>( ESCHER_Prop_adjustValue+i) , lcl_GetConnectorAdjustValue( aPoly, i ) );
                                        }
                                        sal_Int32 nAngle=0;
                                        if (lcl_GetAngle(aPoly,rShapeFlags,nAngle ))
                                        {
                                            AddOpt( ESCHER_Prop_Rotation, nAngle );
                                        }
                                    }
                                    else
                                    {
                                        rShapeType = ESCHER_ShpInst_BentConnector3;
                                        AddOpt( ESCHER_Prop_cxstyle, ESCHER_cxstyleBent );
                                    }
                                }
                                break;
                            default:
                            case drawing::ConnectorType_LINE :
                            case drawing::ConnectorType_LINES :   // Connector 2->5
                            {
                                rShapeType = ESCHER_ShpInst_StraightConnector1;
                                AddOpt( ESCHER_Prop_cxstyle, ESCHER_cxstyleStraight );
                            }
                            break;
                        }
                        CreateLineProperties( aXPropSet, false );
                        bRetValue = true;
                    }
                }
            }
        }
    }
    return bRetValue;
}

void EscherPropertyContainer::CreateShadowProperties(
    const uno::Reference<beans::XPropertySet> & rXPropSet )
{
    uno::Any aAny;

    sal_uInt32  nLineFlags = 0;         // default : shape has no line
    sal_uInt32  nFillFlags = 0x10;      //           shape is filled

    GetOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );
    GetOpt( ESCHER_Prop_fNoFillHitTest, nFillFlags );

    sal_uInt32 nDummy;
    bool bGraphic = GetOpt( DFF_Prop_pib, nDummy ) || GetOpt( DFF_Prop_pibName, nDummy ) || GetOpt( DFF_Prop_pibFlags, nDummy );

    sal_uInt32 nShadowFlags = 0x20000;
    if ( ( nLineFlags & 8 ) || ( nFillFlags & 0x10 ) || bGraphic )
    {
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "Shadow", true ) )
        {
            bool bHasShadow = false; // shadow is possible only if at least a fillcolor, linecolor or graphic is set
            if ( (aAny >>= bHasShadow) && bHasShadow )
            {
                nShadowFlags |= 2;
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "ShadowColor" ) )
                    AddOpt( ESCHER_Prop_shadowColor, ImplGetColor( *o3tl::doAccess<sal_uInt32>(aAny) ) );
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "ShadowXDistance" ) )
                    AddOpt( ESCHER_Prop_shadowOffsetX, *o3tl::doAccess<sal_Int32>(aAny) * 360 );
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "ShadowYDistance" ) )
                    AddOpt( ESCHER_Prop_shadowOffsetY, *o3tl::doAccess<sal_Int32>(aAny) * 360 );
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "ShadowTransparence" ) )
                    AddOpt( ESCHER_Prop_shadowOpacity,  0x10000 - (static_cast<sal_uInt32>(*o3tl::doAccess<sal_uInt16>(aAny)) * 655 ) );
            }
        }
    }
    AddOpt( ESCHER_Prop_fshadowObscured, nShadowFlags );
}

sal_Int32 EscherPropertyContainer::GetValueForEnhancedCustomShapeParameter( const drawing::EnhancedCustomShapeParameter& rParameter,
                                const std::vector< sal_Int32 >& rEquationOrder, bool bAdjustTrans )
{
    sal_Int32 nValue = 0;
    if ( rParameter.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
    {
        double fValue(0.0);
        if ( rParameter.Value >>= fValue )
            nValue = static_cast<sal_Int32>(fValue);
    }
    else
        rParameter.Value >>= nValue;

    switch( rParameter.Type )
    {
        case drawing::EnhancedCustomShapeParameterType::EQUATION :
        {
            size_t nIndex = static_cast<size_t>(nValue);
            OSL_ASSERT(nIndex < rEquationOrder.size());
            if ( nIndex < rEquationOrder.size() )
            {
                nValue = static_cast<sal_uInt16>(rEquationOrder[ nIndex ]);
                nValue |= sal_uInt32(0x80000000);
            }
        }
        break;
        case drawing::EnhancedCustomShapeParameterType::ADJUSTMENT:
        {
            if(bAdjustTrans)
            {
                sal_uInt32 nAdjustValue = 0;
                bool bGot = GetOpt(static_cast<sal_uInt16>( DFF_Prop_adjustValue + nValue ), nAdjustValue);
                if(bGot) nValue = static_cast<sal_Int32>(nAdjustValue);
            }
        }
        break;
        case drawing::EnhancedCustomShapeParameterType::NORMAL :
        default:
        break;
/* not sure if it is allowed to set following values
(but they are not yet used)
        case drawing::EnhancedCustomShapeParameterType::BOTTOM :
        case drawing::EnhancedCustomShapeParameterType::RIGHT :
        case drawing::EnhancedCustomShapeParameterType::TOP :
        case drawing::EnhancedCustomShapeParameterType::LEFT :
*/
    }
    return nValue;
}

static bool GetValueForEnhancedCustomShapeHandleParameter( sal_Int32& nRetValue, const drawing::EnhancedCustomShapeParameter& rParameter )
{
    bool bSpecial = false;
    nRetValue = 0;
    if ( rParameter.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
    {
        double fValue(0.0);
        if ( rParameter.Value >>= fValue )
            nRetValue = static_cast<sal_Int32>(fValue);
    }
    else
        rParameter.Value >>= nRetValue;

    switch( rParameter.Type )
    {
        case drawing::EnhancedCustomShapeParameterType::EQUATION :
        {
            nRetValue += 3;
            bSpecial = true;
        }
        break;
        case drawing::EnhancedCustomShapeParameterType::ADJUSTMENT :
        {
            nRetValue += 0x100;
            bSpecial = true;
        }
        break;
        case drawing::EnhancedCustomShapeParameterType::TOP :
        case drawing::EnhancedCustomShapeParameterType::LEFT :
        {
            nRetValue = 0;
            bSpecial = true;
        }
        break;
        case drawing::EnhancedCustomShapeParameterType::RIGHT :
        case drawing::EnhancedCustomShapeParameterType::BOTTOM :
        {
            nRetValue = 1;
            bSpecial = true;
        }
        break;
        case drawing::EnhancedCustomShapeParameterType::NORMAL :
        {

        }
        break;
    }
    return bSpecial;
}

static void ConvertEnhancedCustomShapeEquation(
    const SdrObjCustomShape& rSdrObjCustomShape,
    std::vector< EnhancedCustomShapeEquation >& rEquations,
    std::vector< sal_Int32 >& rEquationOrder )
{
    uno::Sequence< OUString > sEquationSource;
    const SdrCustomShapeGeometryItem& rGeometryItem =
        rSdrObjCustomShape.GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    const uno::Any* pAny = rGeometryItem.GetPropertyValueByName( "Equations" );
    if ( pAny )
        *pAny >>= sEquationSource;
    sal_Int32 nEquationSourceCount = sEquationSource.getLength();
    if ( !(nEquationSourceCount && (nEquationSourceCount <= 128)) )
        return;

    sal_Int32 i;
    for ( i = 0; i < nEquationSourceCount; i++ )
    {
        EnhancedCustomShape2d aCustoShape2d(
            const_cast< SdrObjCustomShape& >(rSdrObjCustomShape));
        try
        {
            std::shared_ptr< EnhancedCustomShape::ExpressionNode > aExpressNode(
                EnhancedCustomShape::FunctionParser::parseFunction(
                    sEquationSource[ i ], aCustoShape2d));
            drawing::EnhancedCustomShapeParameter aPara( aExpressNode->fillNode( rEquations, nullptr, 0 ) );
            if ( aPara.Type != drawing::EnhancedCustomShapeParameterType::EQUATION )
            {
                EnhancedCustomShapeEquation aEquation;
                aEquation.nOperation = 0;
                EnhancedCustomShape::FillEquationParameter( aPara, 0, aEquation );
                rEquations.push_back( aEquation );
            }
        }
        catch ( const EnhancedCustomShape::ParseError& )
        {
            EnhancedCustomShapeEquation aEquation;      // ups, we should not be here,
            aEquation.nOperation = 0;                   // creating a default equation with value 1
            aEquation.nPara[ 0 ] = 1;                   // hoping that this will not break anything
            rEquations.push_back( aEquation );
        }
        catch ( ... )
        {
            EnhancedCustomShapeEquation aEquation;      // #i112309# EnhancedCustomShape::Parse error
            aEquation.nOperation = 0;                   // not caught on linux platform
            aEquation.nPara[ 0 ] = 1;
            rEquations.push_back( aEquation );
        }
        rEquationOrder.push_back( rEquations.size() - 1 );
    }
    // now updating our old equation indices, they are marked with a bit in the hiword of nOperation
    for (auto & equation : rEquations)
    {
        sal_uInt32 nMask = 0x20000000;
        for( i = 0; i < 3; i++ )
        {
            if ( equation.nOperation & nMask )
            {
                equation.nOperation ^= nMask;
                const size_t nIndex(equation.nPara[ i ] & 0x3ff);

                // #i124661# check index access, there are cases where this is out of bound leading
                // to errors up to crashes when executed
                if(nIndex < rEquationOrder.size())
                {
                    equation.nPara[ i ] = rEquationOrder[ nIndex ] | 0x400;
                }
                else
                {
                    OSL_ENSURE(false, "Attempted out of bound access to rEquationOrder of CustomShape (!)");
                }
            }
            nMask <<= 1;
        }
    }
}

bool EscherPropertyContainer::IsDefaultObject(
    const SdrObjCustomShape& rSdrObjCustomShape,
    const MSO_SPT eShapeType)
{
    switch(eShapeType)
    {
        // if the custom shape is not default shape of ppt, return sal_Fasle;
        case mso_sptTearDrop:
            return false;

        default:
            break;
    }

    return rSdrObjCustomShape.IsDefaultGeometry( SdrObjCustomShape::DefaultType::Equations )
        && rSdrObjCustomShape.IsDefaultGeometry( SdrObjCustomShape::DefaultType::Viewbox )
        && rSdrObjCustomShape.IsDefaultGeometry( SdrObjCustomShape::DefaultType::Path )
        && rSdrObjCustomShape.IsDefaultGeometry( SdrObjCustomShape::DefaultType::Gluepoints )
        && rSdrObjCustomShape.IsDefaultGeometry( SdrObjCustomShape::DefaultType::Segments )
        && rSdrObjCustomShape.IsDefaultGeometry( SdrObjCustomShape::DefaultType::StretchX )
        && rSdrObjCustomShape.IsDefaultGeometry( SdrObjCustomShape::DefaultType::StretchY )
        && rSdrObjCustomShape.IsDefaultGeometry( SdrObjCustomShape::DefaultType::TextFrames );
}

void EscherPropertyContainer::LookForPolarHandles( const MSO_SPT eShapeType, sal_Int32& nAdjustmentsWhichNeedsToBeConverted )
{
    const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eShapeType );
    if ( !(pDefCustomShape && pDefCustomShape->nHandles && pDefCustomShape->pHandles) )
        return;

    sal_Int32 k, nkCount = pDefCustomShape->nHandles;
    const SvxMSDffHandle* pData = pDefCustomShape->pHandles;
    for ( k = 0; k < nkCount; k++, pData++ )
    {
        if ( pData->nFlags & SvxMSDffHandleFlags::POLAR )
        {
            if ( ( pData->nPositionY >= 0x256 ) || ( pData->nPositionY <= 0x107 ) )
                nAdjustmentsWhichNeedsToBeConverted |= ( 1 << k );
        }
    }
}

bool EscherPropertyContainer::GetAdjustmentValue( const drawing::EnhancedCustomShapeAdjustmentValue & rkProp, sal_Int32 nIndex, sal_Int32 nAdjustmentsWhichNeedsToBeConverted, sal_Int32& nValue )
{
    if ( rkProp.State != beans::PropertyState_DIRECT_VALUE )
        return false;

    bool bUseFixedFloat = ( nAdjustmentsWhichNeedsToBeConverted & ( 1 << nIndex ) ) != 0;
    if ( rkProp.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
    {
        double fValue(0.0);
        rkProp.Value >>= fValue;
        if ( bUseFixedFloat )
            fValue *= 65536.0;
        nValue = static_cast<sal_Int32>(fValue);
    }
    else
    {
        rkProp.Value >>= nValue;
        if ( bUseFixedFloat )
             nValue <<= 16;
    }

    return true;
}

void EscherPropertyContainer::CreateCustomShapeProperties( const MSO_SPT eShapeType, const uno::Reference< drawing::XShape > & rXShape )
{
    uno::Reference< beans::XPropertySet > aXPropSet( rXShape, uno::UNO_QUERY );
    if ( !aXPropSet.is() )
        return;

    SdrObjCustomShape* pSdrObjCustomShape = dynamic_cast< SdrObjCustomShape* >(SdrObject::getSdrObjectFromXShape(rXShape));
    if(!pSdrObjCustomShape)
    {
        return;
    }

    SdrObjCustomShape& rSdrObjCustomShape = *pSdrObjCustomShape;
    uno::Any aGeoPropSet = aXPropSet->getPropertyValue( "CustomShapeGeometry" );
    uno::Sequence< beans::PropertyValue > aGeoPropSeq;
    if ( !(aGeoPropSet >>= aGeoPropSeq) )
        return;

    static const OUStringLiteral sViewBox            ( u"ViewBox"  );
    static const OUStringLiteral sTextRotateAngle    ( u"TextRotateAngle"  );
    static const OUStringLiteral sExtrusion          ( u"Extrusion"  );
    static const OUStringLiteral sEquations          ( u"Equations"  );
    static const OUStringLiteral sPath               ( u"Path"  );
    static const OUStringLiteral sTextPath           ( u"TextPath"  );
    static const OUStringLiteral sHandles            ( u"Handles"  );
    static const OUStringLiteral sAdjustmentValues   ( u"AdjustmentValues"  );

    bool bAdjustmentValuesProp = false;
    uno::Any aAdjustmentValuesProp;
    bool bPathCoordinatesProp = false;
    uno::Any aPathCoordinatesProp;

    sal_Int32 nAdjustmentsWhichNeedsToBeConverted = 0;
    uno::Sequence< beans::PropertyValues > aHandlesPropSeq;
    bool bPredefinedHandlesUsed = true;
    const bool bIsDefaultObject(
        IsDefaultObject(
            rSdrObjCustomShape,
            eShapeType));

    // convert property "Equations" into std::vector< EnhancedCustomShapeEquationEquation >
    std::vector< EnhancedCustomShapeEquation >  aEquations;
    std::vector< sal_Int32 >                    aEquationOrder;
    ConvertEnhancedCustomShapeEquation(
        rSdrObjCustomShape,
        aEquations,
        aEquationOrder);

    sal_Int32 i, nCount = aGeoPropSeq.getLength();
    for ( i = 0; i < nCount; i++ )
    {
        const beans::PropertyValue& rProp = aGeoPropSeq[ i ];
        if ( rProp.Name == sViewBox )
        {
            if ( !bIsDefaultObject )
            {
                awt::Rectangle aViewBox;
                if ( rProp.Value >>= aViewBox )
                {
                    AddOpt( DFF_Prop_geoLeft,  aViewBox.X );
                    AddOpt( DFF_Prop_geoTop,   aViewBox.Y );
                    AddOpt( DFF_Prop_geoRight, aViewBox.X + aViewBox.Width );
                    AddOpt( DFF_Prop_geoBottom,aViewBox.Y + aViewBox.Height );
                }
            }
        }
        else if ( rProp.Name == sTextRotateAngle )
        {
            double f = 0;
            if ( rProp.Value >>= f )
            {
                double fTextRotateAngle = fmod( f, 360.0 );
                if ( fTextRotateAngle < 0 )
                    fTextRotateAngle = 360 + fTextRotateAngle;
                if ( ( fTextRotateAngle < 271.0 ) && ( fTextRotateAngle > 269.0 ) )
                    AddOpt( DFF_Prop_cdirFont, mso_cdir90 );
                else if ( ( fTextRotateAngle < 181.0 ) && ( fTextRotateAngle > 179.0 ) )
                    AddOpt( DFF_Prop_cdirFont, mso_cdir180 );
                else if ( ( fTextRotateAngle < 91.0 ) && ( fTextRotateAngle > 79.0 ) )
                    AddOpt( DFF_Prop_cdirFont, mso_cdir270 );
            }
        }
        else if ( rProp.Name == sExtrusion )
        {
            uno::Sequence< beans::PropertyValue > aExtrusionPropSeq;
            if ( rProp.Value >>= aExtrusionPropSeq )
            {
                sal_uInt32 nLightFaceFlagsOrg, nLightFaceFlags;
                sal_uInt32 nFillHarshFlagsOrg, nFillHarshFlags;
                nLightFaceFlagsOrg = nLightFaceFlags = 0x000001;
                nFillHarshFlagsOrg = nFillHarshFlags = 0x00001e;
                if ( GetOpt( DFF_Prop_fc3DLightFace, nLightFaceFlags ) )
                    nLightFaceFlagsOrg = nLightFaceFlags;
                if ( GetOpt( DFF_Prop_fc3DFillHarsh, nFillHarshFlags ) )
                    nFillHarshFlagsOrg = nFillHarshFlags;

                sal_Int32 r, nrCount = aExtrusionPropSeq.getLength();
                for ( r = 0; r < nrCount; r++ )
                {
                    const beans::PropertyValue& rrProp = aExtrusionPropSeq[ r ];

                    if ( rrProp.Name == sExtrusion )
                    {
                        bool bExtrusionOn;
                        if ( rrProp.Value >>= bExtrusionOn )
                        {
                            nLightFaceFlags |= 0x80000;
                            if ( bExtrusionOn )
                                nLightFaceFlags |= 8;
                            else
                                nLightFaceFlags &=~8;
                        }
                    }
                    else if ( rrProp.Name == "Brightness" )
                    {
                        double fExtrusionBrightness = 0;
                        if ( rrProp.Value >>= fExtrusionBrightness )
                            AddOpt( DFF_Prop_c3DAmbientIntensity, static_cast<sal_Int32>( fExtrusionBrightness * 655.36 ) );
                    }
                    else if ( rrProp.Name == "Depth" )
                    {
                        double fDepth = 0;
                        double fFraction = 0;
                        drawing::EnhancedCustomShapeParameterPair aDepthParaPair;
                        if ( ( rrProp.Value >>= aDepthParaPair ) && ( aDepthParaPair.First.Value >>= fDepth ) && ( aDepthParaPair.Second.Value >>= fFraction ) )
                        {
                            double fForeDepth = fDepth * fFraction;
                            double fBackDepth = fDepth - fForeDepth;

                            fBackDepth *= 360.0;
                            AddOpt( DFF_Prop_c3DExtrudeBackward, static_cast<sal_Int32>(fBackDepth) );

                            if ( fForeDepth != 0.0 )
                            {
                                fForeDepth *= 360.0;
                                AddOpt( DFF_Prop_c3DExtrudeForward, static_cast<sal_Int32>(fForeDepth) );
                            }
                        }
                    }
                    else if ( rrProp.Name == "Diffusion" )
                    {
                        double fExtrusionDiffusion = 0;
                        if ( rrProp.Value >>= fExtrusionDiffusion )
                            AddOpt( DFF_Prop_c3DDiffuseAmt, static_cast<sal_Int32>( fExtrusionDiffusion * 655.36 ) );
                    }
                    else if ( rrProp.Name == "NumberOfLineSegments" )
                    {
                        sal_Int32 nExtrusionNumberOfLineSegments = 0;
                        if ( rrProp.Value >>= nExtrusionNumberOfLineSegments )
                            AddOpt( DFF_Prop_c3DTolerance, nExtrusionNumberOfLineSegments );
                    }
                    else if ( rrProp.Name == "LightFace" )
                    {
                        bool bExtrusionLightFace;
                        if ( rrProp.Value >>= bExtrusionLightFace )
                        {
                            nLightFaceFlags |= 0x10000;
                            if ( bExtrusionLightFace )
                                nLightFaceFlags |= 1;
                            else
                                nLightFaceFlags &=~1;
                        }
                    }
                    else if ( rrProp.Name == "FirstLightHarsh" )
                    {
                        bool bExtrusionFirstLightHarsh;
                        if ( rrProp.Value >>= bExtrusionFirstLightHarsh )
                        {
                            nFillHarshFlags |= 0x20000;
                            if ( bExtrusionFirstLightHarsh )
                                nFillHarshFlags |= 2;
                            else
                                nFillHarshFlags &=~2;
                        }
                    }
                    else if ( rrProp.Name == "SecondLightHarsh" )
                    {
                        bool bExtrusionSecondLightHarsh;
                        if ( rrProp.Value >>= bExtrusionSecondLightHarsh )
                        {
                            nFillHarshFlags |= 0x10000;
                            if ( bExtrusionSecondLightHarsh )
                                nFillHarshFlags |= 1;
                            else
                                nFillHarshFlags &=~1;
                        }
                    }
                    else if ( rrProp.Name == "FirstLightLevel" )
                    {
                        double fExtrusionFirstLightLevel = 0;
                        if ( rrProp.Value >>= fExtrusionFirstLightLevel )
                            AddOpt( DFF_Prop_c3DKeyIntensity, static_cast<sal_Int32>( fExtrusionFirstLightLevel * 655.36 ) );
                    }
                    else if ( rrProp.Name == "SecondLightLevel" )
                    {
                        double fExtrusionSecondLightLevel = 0;
                        if ( rrProp.Value >>= fExtrusionSecondLightLevel )
                            AddOpt( DFF_Prop_c3DFillIntensity, static_cast<sal_Int32>( fExtrusionSecondLightLevel * 655.36 ) );
                    }
                    else if ( rrProp.Name == "FirstLightDirection" )
                    {
                        drawing::Direction3D aExtrusionFirstLightDirection;
                        if ( rrProp.Value >>= aExtrusionFirstLightDirection )
                        {
                            AddOpt( DFF_Prop_c3DKeyX, static_cast<sal_Int32>(aExtrusionFirstLightDirection.DirectionX)  );
                            AddOpt( DFF_Prop_c3DKeyY, static_cast<sal_Int32>(aExtrusionFirstLightDirection.DirectionY)  );
                            AddOpt( DFF_Prop_c3DKeyZ, static_cast<sal_Int32>(aExtrusionFirstLightDirection.DirectionZ)  );
                        }
                    }
                    else if ( rrProp.Name == "SecondLightDirection" )
                    {
                        drawing::Direction3D aExtrusionSecondLightPosition;
                        if ( rrProp.Value >>= aExtrusionSecondLightPosition )
                        {
                            AddOpt( DFF_Prop_c3DFillX, static_cast<sal_Int32>(aExtrusionSecondLightPosition.DirectionX)  );
                            AddOpt( DFF_Prop_c3DFillY, static_cast<sal_Int32>(aExtrusionSecondLightPosition.DirectionY)  );
                            AddOpt( DFF_Prop_c3DFillZ, static_cast<sal_Int32>(aExtrusionSecondLightPosition.DirectionZ)  );
                        }
                    }
                    else if ( rrProp.Name == "Metal" )
                    {
                        bool bExtrusionMetal;
                        if ( rrProp.Value >>= bExtrusionMetal )
                        {
                            nLightFaceFlags |= 0x40000;
                            if ( bExtrusionMetal )
                                nLightFaceFlags |= 4;
                            else
                                nLightFaceFlags &=~4;
                        }
                    }
                    else if ( rrProp.Name == "ShadeMode" )
                    {
                        drawing::ShadeMode eExtrusionShadeMode;
                        if ( rrProp.Value >>= eExtrusionShadeMode )
                        {
                            sal_uInt32 nRenderMode;
                            switch( eExtrusionShadeMode )
                            {
                                default:
                                case drawing::ShadeMode_FLAT :
                                case drawing::ShadeMode_PHONG :
                                case drawing::ShadeMode_SMOOTH :
                                    nRenderMode = mso_FullRender;
                                break;
                                case drawing::ShadeMode_DRAFT :
                                {
                                    nRenderMode = mso_Wireframe;
                                }
                                break;
                            }
                            AddOpt( DFF_Prop_c3DRenderMode, nRenderMode );
                        }
                    }
                    else if ( rrProp.Name == "RotateAngle" )
                    {
                        double fExtrusionAngleX = 0;
                        double fExtrusionAngleY = 0;
                        drawing::EnhancedCustomShapeParameterPair aRotateAnglePair;
                        if ( ( rrProp.Value >>= aRotateAnglePair ) && ( aRotateAnglePair.First.Value >>= fExtrusionAngleX ) && ( aRotateAnglePair.Second.Value >>= fExtrusionAngleY ) )
                        {
                            fExtrusionAngleX *= 65536;
                            fExtrusionAngleY *= 65536;
                            AddOpt( DFF_Prop_c3DXRotationAngle, static_cast<sal_Int32>(fExtrusionAngleX) );
                            AddOpt( DFF_Prop_c3DYRotationAngle, static_cast<sal_Int32>(fExtrusionAngleY) );
                        }
                    }
                    else if ( rrProp.Name == "RotationCenter" )
                    {
                        drawing::Direction3D aExtrusionRotationCenter;
                        if ( rrProp.Value >>= aExtrusionRotationCenter )
                        {
                            // tdf#145904 X- and Y-component is fraction, Z-component in EMU
                            AddOpt( DFF_Prop_c3DRotationCenterX, static_cast<sal_Int32>( aExtrusionRotationCenter.DirectionX * 65536.0 ) );
                            AddOpt( DFF_Prop_c3DRotationCenterY, static_cast<sal_Int32>( aExtrusionRotationCenter.DirectionY * 65536.0 ) );
                            AddOpt( DFF_Prop_c3DRotationCenterZ, static_cast<sal_Int32>( aExtrusionRotationCenter.DirectionZ * 360.0 ) );
                            nFillHarshFlags &=~8; // don't use AutoRotationCenter;
                        }
                    }
                    else if ( rrProp.Name == "Shininess" )
                    {
                        double fExtrusionShininess = 0;
                        if ( rrProp.Value >>= fExtrusionShininess )
                            AddOpt( DFF_Prop_c3DShininess, static_cast<sal_Int32>( fExtrusionShininess * 655.36 ) );
                    }
                    else if ( rrProp.Name == "Skew" )
                    {
                        double fSkewAmount = 0;
                        double fSkewAngle = 0;
                        drawing::EnhancedCustomShapeParameterPair aSkewParaPair;
                        if ( ( rrProp.Value >>= aSkewParaPair ) && ( aSkewParaPair.First.Value >>= fSkewAmount ) && ( aSkewParaPair.Second.Value >>= fSkewAngle ) )
                        {
                            AddOpt( DFF_Prop_c3DSkewAmount, static_cast<sal_Int32>(fSkewAmount) );
                            AddOpt( DFF_Prop_c3DSkewAngle, static_cast<sal_Int32>( fSkewAngle * 65536 ) );
                        }
                    }
                    else if ( rrProp.Name == "Specularity" )
                    {
                        double fExtrusionSpecularity = 0;
                        if ( rrProp.Value >>= fExtrusionSpecularity )
                            AddOpt( DFF_Prop_c3DSpecularAmt, static_cast<sal_Int32>( fExtrusionSpecularity * 1333 ) );
                    }
                    else if ( rrProp.Name == "ProjectionMode" )
                    {
                        drawing::ProjectionMode eExtrusionProjectionMode;
                        if ( rrProp.Value >>= eExtrusionProjectionMode )
                        {
                            nFillHarshFlags |= 0x40000;
                            if ( eExtrusionProjectionMode == drawing::ProjectionMode_PARALLEL )
                                nFillHarshFlags |= 4;
                            else
                                nFillHarshFlags &=~4;
                        }
                    }
                    else if ( rrProp.Name == "ViewPoint" )
                    {
                        drawing::Position3D aExtrusionViewPoint;
                        if ( rrProp.Value >>= aExtrusionViewPoint )
                        {
                            aExtrusionViewPoint.PositionX *= 360.0;
                            aExtrusionViewPoint.PositionY *= 360.0;
                            aExtrusionViewPoint.PositionZ *= 360.0;
                            AddOpt( DFF_Prop_c3DXViewpoint, static_cast<sal_Int32>(aExtrusionViewPoint.PositionX)  );
                            AddOpt( DFF_Prop_c3DYViewpoint, static_cast<sal_Int32>(aExtrusionViewPoint.PositionY)  );
                            AddOpt( DFF_Prop_c3DZViewpoint, static_cast<sal_Int32>(aExtrusionViewPoint.PositionZ)  );
                        }
                    }
                    else if ( rrProp.Name == "Origin" )
                    {
                        double fExtrusionOriginX = 0;
                        double fExtrusionOriginY = 0;
                        drawing::EnhancedCustomShapeParameterPair aOriginPair;
                        if ( ( rrProp.Value >>= aOriginPair ) && ( aOriginPair.First.Value >>= fExtrusionOriginX ) && ( aOriginPair.Second.Value >>= fExtrusionOriginY ) )
                        {
                            AddOpt( DFF_Prop_c3DOriginX, static_cast<sal_Int32>( fExtrusionOriginX * 65536 ) );
                            AddOpt( DFF_Prop_c3DOriginY, static_cast<sal_Int32>( fExtrusionOriginY * 65536 ) );
                        }
                    }
                    else if ( rrProp.Name == "Color" )
                    {
                        bool bExtrusionColor;
                        if ( rrProp.Value >>= bExtrusionColor )
                        {
                            nLightFaceFlags |= 0x20000;
                            if ( bExtrusionColor )
                            {
                                nLightFaceFlags |= 2;
                                uno::Any aFillColor2;
                                if ( EscherPropertyValueHelper::GetPropertyValue( aFillColor2, aXPropSet, "FillColor2", true ) )
                                {
                                    sal_uInt32 nFillColor = ImplGetColor( *o3tl::doAccess<sal_uInt32>(aFillColor2) );
                                    AddOpt( DFF_Prop_c3DExtrusionColor, nFillColor );
                                }
                            }
                            else
                                nLightFaceFlags &=~2;
                        }
                    }
                }
                if ( nLightFaceFlags != nLightFaceFlagsOrg )
                    AddOpt( DFF_Prop_fc3DLightFace, nLightFaceFlags );
                if ( nFillHarshFlags != nFillHarshFlagsOrg )
                    AddOpt( DFF_Prop_fc3DFillHarsh, nFillHarshFlags );
            }
        }
        else if ( rProp.Name == sEquations )
        {
            if ( !bIsDefaultObject )
            {
                sal_uInt16 nElements = static_cast<sal_uInt16>(aEquations.size());
                if ( nElements )
                {
                    sal_uInt16 nElementSize = 8;
                    sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                    SvMemoryStream aMemStrm( nStreamSize );
                    aMemStrm.WriteUInt16( nElements )
                       .WriteUInt16( nElements )
                       .WriteUInt16( nElementSize );

                    for (auto const& equation : aEquations)
                    {
                        aMemStrm.WriteUInt16( equation.nOperation )
                            .WriteInt16(
                                std::clamp(
                                    equation.nPara[ 0 ], sal_Int32(SAL_MIN_INT16),
                                    sal_Int32(SAL_MAX_INT16)) )
                            .WriteInt16(
                                std::clamp(
                                    equation.nPara[ 1 ], sal_Int32(SAL_MIN_INT16),
                                    sal_Int32(SAL_MAX_INT16)) )
                            .WriteInt16(
                                std::clamp(
                                    equation.nPara[ 2 ], sal_Int32(SAL_MIN_INT16),
                                    sal_Int32(SAL_MAX_INT16)) );
                    }

                    AddOpt(DFF_Prop_pFormulas, true, 6, aMemStrm);
                }
                else
                {
                    AddOpt(DFF_Prop_pFormulas, 0, true);
                }
            }
        }
        else if ( rProp.Name == sPath )
        {
            uno::Sequence< beans::PropertyValue > aPathPropSeq;
            if ( rProp.Value >>= aPathPropSeq )
            {
                sal_uInt32 nPathFlags, nPathFlagsOrg;
                nPathFlagsOrg = nPathFlags = 0x39;
                if ( GetOpt( DFF_Prop_fFillOK, nPathFlags ) )
                    nPathFlagsOrg = nPathFlags;

                sal_Int32 r, nrCount = aPathPropSeq.getLength();
                for ( r = 0; r < nrCount; r++ )
                {
                    const beans::PropertyValue& rrProp = aPathPropSeq[ r ];

                    if ( rrProp.Name == "ExtrusionAllowed" )
                    {
                        bool bExtrusionAllowed;
                        if ( rrProp.Value >>= bExtrusionAllowed )
                        {
                            nPathFlags |= 0x100000;
                            if ( bExtrusionAllowed )
                                nPathFlags |= 16;
                            else
                                nPathFlags &=~16;
                        }
                    }
                    else if ( rrProp.Name == "ConcentricGradientFillAllowed" )
                    {
                        bool bConcentricGradientFillAllowed;
                        if ( rrProp.Value >>= bConcentricGradientFillAllowed )
                        {
                            nPathFlags |= 0x20000;
                            if ( bConcentricGradientFillAllowed )
                                nPathFlags |= 2;
                            else
                                nPathFlags &=~2;
                        }
                    }
                    else if ( rrProp.Name == "TextPathAllowed" )
                    {
                        bool bTextPathAllowed;
                        if ( rrProp.Value >>= bTextPathAllowed )
                        {
                            nPathFlags |= 0x40000;
                            if ( bTextPathAllowed )
                                nPathFlags |= 4;
                            else
                                nPathFlags &=~4;
                        }
                    }
                    else if ( rrProp.Name == "Coordinates" )
                    {
                        if ( !bIsDefaultObject )
                        {
                            aPathCoordinatesProp = rrProp.Value;
                            bPathCoordinatesProp = true;
                        }
                    }
                    else if ( rrProp.Name == "GluePoints" )
                    {
                        if ( !bIsDefaultObject )
                        {
                            uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aGluePoints;
                            if ( rrProp.Value >>= aGluePoints )
                            {
                                // creating the vertices
                                sal_uInt16 nElements = static_cast<sal_uInt16>(aGluePoints.getLength());
                                if ( nElements )
                                {
                                    sal_uInt16 j, nElementSize = 8;
                                    sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                                    SvMemoryStream aMemStrm( nStreamSize );
                                    aMemStrm.WriteUInt16( nElements )
                                       .WriteUInt16( nElements )
                                       .WriteUInt16( nElementSize );
                                    for( j = 0; j < nElements; j++ )
                                    {
                                        sal_Int32 X = GetValueForEnhancedCustomShapeParameter( aGluePoints[ j ].First, aEquationOrder );
                                        sal_Int32 Y = GetValueForEnhancedCustomShapeParameter( aGluePoints[ j ].Second, aEquationOrder );
                                        aMemStrm.WriteInt32( X )
                                           .WriteInt32( Y );
                                    }

                                    AddOpt(DFF_Prop_connectorPoints, true, 6, aMemStrm);   // -6
                                }
                                else
                                {
                                    AddOpt(DFF_Prop_connectorPoints, 0, true);
                                }
                            }
                        }
                    }
                    else if ( rrProp.Name == "GluePointType" )
                    {
                        sal_Int16 nGluePointType = sal_Int16();
                        if ( rrProp.Value >>= nGluePointType )
                            AddOpt( DFF_Prop_connectorType, static_cast<sal_uInt16>(nGluePointType) );
                    }
                    else if ( rrProp.Name == "Segments" )
                    {
                        if ( !bIsDefaultObject )
                        {
                            uno::Sequence<drawing::EnhancedCustomShapeSegment> aSegments;
                            if ( rrProp.Value >>= aSegments )
                            {
                                // creating seginfo
                                if ( aSegments.hasElements() )
                                {
                                    sal_uInt16 j, nElements = static_cast<sal_uInt16>(aSegments.getLength());
                                    sal_uInt16 nElementSize = 2;
                                    sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                                    SvMemoryStream aMemStrm( nStreamSize );
                                    aMemStrm.WriteUInt16( nElements )
                                       .WriteUInt16( nElements )
                                       .WriteUInt16( nElementSize );
                                    for ( j = 0; j < nElements; j++ )
                                    {
                                        // The segment type is stored in the upper 3 bits
                                        // and segment count is stored in the lower 13
                                        // bits.
                                        //
                                        // If the segment type is msopathEscape, the lower 13 bits
                                        // are divided in a 5 bit escape code and 8 bit
                                        // vertex count (not segment count!)
                                        sal_uInt16 nVal = static_cast<sal_uInt16>(aSegments[ j ].Count);
                                        switch( aSegments[ j ].Command )
                                        {
                                            case drawing::EnhancedCustomShapeSegmentCommand::UNKNOWN :
                                            case drawing::EnhancedCustomShapeSegmentCommand::LINETO :
                                                break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::MOVETO :
                                                nVal = (msopathMoveTo << 13);
                                                break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::CURVETO :
                                            {
                                                nVal |= (msopathCurveTo << 13);
                                            }
                                            break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::CLOSESUBPATH :
                                            {
                                                nVal = 1;
                                                nVal |= (msopathClose << 13);
                                            }
                                            break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::ENDSUBPATH :
                                            {
                                                nVal = (msopathEnd << 13);
                                            }
                                            break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::NOFILL :
                                            {
                                                nVal = (msopathEscape << 13) | (10 << 8);
                                            }
                                            break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::NOSTROKE :
                                            {
                                                nVal = (msopathEscape << 13) | (11 << 8);
                                            }
                                            break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO :
                                            {
                                                nVal *= 3;
                                                nVal |= (msopathEscape << 13) | (1 << 8);
                                            }
                                            break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE :
                                            {
                                                nVal *= 3;
                                                nVal |= (msopathEscape << 13) | (2 << 8);
                                            }
                                            break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::ARCTO :
                                            {
                                                nVal <<= 2;
                                                nVal |= (msopathEscape << 13) | (3 << 8);
                                            }
                                            break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::ARC :
                                            {
                                                nVal <<= 2;
                                                nVal |= (msopathEscape << 13) | (4 << 8);
                                            }
                                            break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO :
                                            {
                                                nVal <<= 2;
                                                nVal |= (msopathEscape << 13) | (5 << 8);
                                            }
                                            break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARC :
                                            {
                                                nVal <<= 2;
                                                nVal |= (msopathEscape << 13) | (6 << 8);
                                            }
                                            break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX :
                                            {
                                                nVal |= (msopathEscape << 13) | (7 << 8);
                                            }
                                            break;
                                            case drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY :
                                            {
                                                nVal |= (msopathEscape << 13) | (8 << 8);
                                            }
                                            break;
                                        }
                                        aMemStrm.WriteUInt16( nVal );
                                    }

                                    AddOpt(DFF_Prop_pSegmentInfo, false, 6, aMemStrm);
                                }
                                else
                                {
                                    AddOpt(DFF_Prop_pSegmentInfo, 0, true);
                                }
                            }
                        }
                    }
                    else if ( rrProp.Name == "StretchX" )
                    {
                        if ( !bIsDefaultObject )
                        {
                            sal_Int32 nStretchX = 0;
                            if ( rrProp.Value >>= nStretchX )
                                AddOpt( DFF_Prop_stretchPointX, nStretchX );
                        }
                    }
                    else if ( rrProp.Name == "StretchY" )
                    {
                        if ( !bIsDefaultObject )
                        {
                            sal_Int32 nStretchY = 0;
                            if ( rrProp.Value >>= nStretchY )
                                AddOpt( DFF_Prop_stretchPointY, nStretchY );
                        }
                    }
                    else if ( rrProp.Name == "TextFrames" )
                    {
                        if ( !bIsDefaultObject )
                        {
                            uno::Sequence<drawing::EnhancedCustomShapeTextFrame> aPathTextFrames;
                            if ( rrProp.Value >>= aPathTextFrames )
                            {
                                if ( aPathTextFrames.hasElements() )
                                {
                                    sal_uInt16 j, nElements = static_cast<sal_uInt16>(aPathTextFrames.getLength());
                                    sal_uInt16 nElementSize = 16;
                                    sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                                    SvMemoryStream aMemStrm( nStreamSize );
                                    aMemStrm.WriteUInt16( nElements )
                                       .WriteUInt16( nElements )
                                       .WriteUInt16( nElementSize );
                                    for ( j = 0; j < nElements; j++ )
                                    {
                                        sal_Int32 nLeft = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].TopLeft.First, aEquationOrder );
                                        sal_Int32 nTop  = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].TopLeft.Second, aEquationOrder );
                                        sal_Int32 nRight = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].BottomRight.First, aEquationOrder );
                                        sal_Int32 nBottom = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].BottomRight.Second, aEquationOrder );

                                        aMemStrm.WriteInt32( nLeft )
                                           .WriteInt32( nTop )
                                           .WriteInt32( nRight )
                                           .WriteInt32( nBottom );
                                    }

                                    AddOpt(DFF_Prop_textRectangles, true, 6, aMemStrm);
                                }
                                else
                                {
                                    AddOpt(DFF_Prop_textRectangles, 0, true);
                                }
                            }
                        }
                    }
                }
                if ( nPathFlags != nPathFlagsOrg )
                    AddOpt( DFF_Prop_fFillOK, nPathFlags );
            }
        }
        else if ( rProp.Name == sTextPath )
        {
            uno::Sequence< beans::PropertyValue > aTextPathPropSeq;
            if ( rProp.Value >>= aTextPathPropSeq )
            {
                sal_uInt32 nTextPathFlagsOrg, nTextPathFlags;
                nTextPathFlagsOrg = nTextPathFlags = 0xffff1000;        // default
                if ( GetOpt( DFF_Prop_gtextFStrikethrough, nTextPathFlags ) )
                    nTextPathFlagsOrg = nTextPathFlags;

                sal_Int32 r, nrCount = aTextPathPropSeq.getLength();
                for ( r = 0; r < nrCount; r++ )
                {
                    const beans::PropertyValue& rrProp = aTextPathPropSeq[ r ];

                    if ( rrProp.Name == sTextPath )
                    {
                        bool bTextPathOn;
                        if ( rrProp.Value >>= bTextPathOn )
                        {
                            nTextPathFlags |= 0x40000000;
                            if ( bTextPathOn )
                            {
                                nTextPathFlags |= 0x4000;

                                sal_uInt32 nPathFlags = 0x39;
                                GetOpt( DFF_Prop_fFillOK, nPathFlags ); // SJ: can be removed if we are supporting the TextPathAllowed property in XML
                                nPathFlags |= 0x40004;
                                AddOpt( DFF_Prop_fFillOK, nPathFlags );
                            }
                            else
                                nTextPathFlags &=~0x4000;
                        }
                    }
                    else if ( rrProp.Name == "TextPathMode" )
                    {
                        drawing::EnhancedCustomShapeTextPathMode eTextPathMode;
                        if ( rrProp.Value >>= eTextPathMode )
                        {
                            nTextPathFlags |= 0x05000000;
                            nTextPathFlags &=~0x500;    // TextPathMode_NORMAL
                            if ( eTextPathMode == drawing::EnhancedCustomShapeTextPathMode_PATH )
                                nTextPathFlags |= 0x100;
                            else if ( eTextPathMode == drawing::EnhancedCustomShapeTextPathMode_SHAPE )
                                nTextPathFlags |= 0x500;
                        }
                    }
                    else if ( rrProp.Name == "ScaleX" )
                    {
                        bool bTextPathScaleX;
                        if ( rrProp.Value >>= bTextPathScaleX )
                        {
                            nTextPathFlags |= 0x00400000;
                            if ( bTextPathScaleX )
                                nTextPathFlags |= 0x40;
                            else
                                nTextPathFlags &=~0x40;
                        }
                    }
                    else if ( rrProp.Name == "SameLetterHeights" )
                    {
                        bool bSameLetterHeights;
                        if ( rrProp.Value >>= bSameLetterHeights )
                        {
                            nTextPathFlags |= 0x00800000;
                            if ( bSameLetterHeights )
                                nTextPathFlags |= 0x80;
                            else
                                nTextPathFlags &=~0x80;
                        }
                    }
                }
                if ( nTextPathFlags & 0x4000 )      // Is FontWork ?
                {
                    // FontWork Text
                    OUString aText;
                    uno::Reference< text::XSimpleText > xText( rXShape, uno::UNO_QUERY );
                    if ( xText.is() )
                        aText = xText->getString();
                    if ( aText.isEmpty() )
                        aText = "your text";   // TODO: moving into a resource
                    AddOpt( DFF_Prop_gtextUNICODE, aText );

                    // FontWork Font
                    OUString aFontName;
                    uno::Any aAny = aXPropSet->getPropertyValue( "CharFontName" );
                    aAny >>= aFontName;
                    if ( aFontName.isEmpty() )
                        aFontName = "Arial Black";
                    AddOpt( DFF_Prop_gtextFont, aFontName );

                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "CharScaleWidth", true ) )
                    {
                        sal_Int16 nCharScaleWidth = 100;
                        if ( aAny >>= nCharScaleWidth )
                        {
                            if ( nCharScaleWidth != 100 )
                            {
                                sal_Int32 nVal = nCharScaleWidth * 655;
                                AddOpt( DFF_Prop_gtextSpacing, nVal );
                            }
                        }
                    }
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "CharHeight", true ) )
                    {
                        float fCharHeight = 0.0;
                        if ( aAny >>= fCharHeight )
                        {
                            sal_Int32 nTextSize = static_cast< sal_Int32 > ( fCharHeight * 65536 );
                            AddOpt(ESCHER_Prop_gtextSize, nTextSize);
                        }
                    }
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "CharKerning", true ) )
                    {
                        sal_Int16 nCharKerning = sal_Int16();
                        if ( aAny >>= nCharKerning )
                        {
                            nTextPathFlags |= 0x10000000;
                            if ( nCharKerning )
                                nTextPathFlags |= 0x1000;
                            else
                                nTextPathFlags &=~0x1000;
                        }
                    }
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "CharPosture", true ) )
                    {
                        awt::FontSlant eFontSlant;
                        if ( aAny >>= eFontSlant )
                        {
                            nTextPathFlags |= 0x100010;
                            if ( eFontSlant != awt::FontSlant_NONE )
                                nTextPathFlags |= 0x10;
                            else
                                nTextPathFlags &=~0x10;
                        }
                    }
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "CharWeight", true ) )
                    {
                        float fFontWidth = 0;
                        if ( aAny >>= fFontWidth )
                        {
                            nTextPathFlags |= 0x200020;
                            if ( fFontWidth > awt::FontWeight::NORMAL )
                                nTextPathFlags |= 0x20;
                            else
                                nTextPathFlags &=~0x20;
                        }
                    }
                    // export gTextAlign attr
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "TextHorizontalAdjust", true ) )
                    {
                        MSO_GeoTextAlign  gTextAlign = mso_alignTextCenter;
                        drawing::TextHorizontalAdjust   eHA( drawing::TextHorizontalAdjust_LEFT );
                        aAny >>= eHA;
                        switch( eHA )
                        {
                        case drawing::TextHorizontalAdjust_LEFT :
                            gTextAlign = mso_alignTextLeft;
                            break;
                        case drawing::TextHorizontalAdjust_CENTER:
                            gTextAlign = mso_alignTextCenter;
                            break;
                        case drawing::TextHorizontalAdjust_RIGHT:
                            gTextAlign = mso_alignTextRight;
                            break;
                        case drawing::TextHorizontalAdjust_BLOCK:
                            {
                                drawing::TextFitToSizeType const eFTS(
                                    rSdrObjCustomShape.GetMergedItem( SDRATTR_TEXT_FITTOSIZE ).GetValue() );
                                if (eFTS == drawing::TextFitToSizeType_ALLLINES ||
                                    eFTS == drawing::TextFitToSizeType_PROPORTIONAL)
                                {
                                    gTextAlign = mso_alignTextStretch;
                                }
                                else
                                {
                                    gTextAlign = mso_alignTextWordJust;
                                }
                                break;
                            }
                        default:
                            break;
                        }
                        AddOpt(DFF_Prop_gtextAlign,gTextAlign);
                    }
                }
                if((nTextPathFlags & 0x4000) != 0)  // Is Font work
                {
                    OutlinerParaObject* pOutlinerParaObject(rSdrObjCustomShape.GetOutlinerParaObject());
                    if ( pOutlinerParaObject && pOutlinerParaObject->IsEffectivelyVertical() )
                        nTextPathFlags |= 0x2000;
                }

                // Use gtextFStretch for Watermark like MSO does
                nTextPathFlags |= use_gtextFBestFit | gtextFBestFit
                                | use_gtextFStretch | gtextFStretch
                                | use_gtextFShrinkFit | gtextFShrinkFit;

                if ( nTextPathFlags != nTextPathFlagsOrg )
                    AddOpt( DFF_Prop_gtextFStrikethrough, nTextPathFlags );
            }
        }
        else if ( rProp.Name == sHandles )
        {
            if ( !bIsDefaultObject )
            {
                bPredefinedHandlesUsed = false;
                if ( rProp.Value >>= aHandlesPropSeq )
                {
                    sal_uInt16 nElements = static_cast<sal_uInt16>(aHandlesPropSeq.getLength());
                    if ( nElements )
                    {
                        sal_uInt16 k, nElementSize = 36;
                        sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                        SvMemoryStream aMemStrm( nStreamSize );
                        aMemStrm.WriteUInt16( nElements )
                           .WriteUInt16( nElements )
                           .WriteUInt16( nElementSize );

                        for ( k = 0; k < nElements; k++ )
                        {
                            sal_uInt32 nFlags = 0;
                            sal_Int32 nXPosition = 0;
                            sal_Int32 nYPosition = 0;
                            sal_Int32 nXMap = 0;
                            sal_Int32 nYMap = 0;
                            sal_Int32 nXRangeMin = 0x80000000;
                            sal_Int32 nXRangeMax = 0x7fffffff;
                            sal_Int32 nYRangeMin = 0x80000000;
                            sal_Int32 nYRangeMax = 0x7fffffff;

                            const uno::Sequence< beans::PropertyValue >& rPropSeq = aHandlesPropSeq[ k ];
                            for ( const beans::PropertyValue& rPropVal: rPropSeq )
                            {
                                if ( rPropVal.Name == "Position" )
                                {
                                    drawing::EnhancedCustomShapeParameterPair aPosition;
                                    if ( rPropVal.Value >>= aPosition )
                                    {
                                        GetValueForEnhancedCustomShapeHandleParameter( nXPosition, aPosition.First );
                                        GetValueForEnhancedCustomShapeHandleParameter( nYPosition, aPosition.Second );
                                    }
                                }
                                else if ( rPropVal.Name == "MirroredX" )
                                {
                                    bool bMirroredX;
                                    if ( rPropVal.Value >>= bMirroredX )
                                    {
                                        if ( bMirroredX )
                                            nFlags |= 1;
                                    }
                                }
                                else if ( rPropVal.Name == "MirroredY" )
                                {
                                    bool bMirroredY;
                                    if ( rPropVal.Value >>= bMirroredY )
                                    {
                                        if ( bMirroredY )
                                            nFlags |= 2;
                                    }
                                }
                                else if ( rPropVal.Name == "Switched" )
                                {
                                    bool bSwitched;
                                    if ( rPropVal.Value >>= bSwitched )
                                    {
                                        if ( bSwitched )
                                            nFlags |= 4;
                                    }
                                }
                                else if ( rPropVal.Name == "Polar" )
                                {
                                    drawing::EnhancedCustomShapeParameterPair aPolar;
                                    if ( rPropVal.Value >>= aPolar )
                                    {
                                        if ( GetValueForEnhancedCustomShapeHandleParameter( nXMap, aPolar.First ) )
                                            nFlags |= 0x800;
                                        if ( GetValueForEnhancedCustomShapeHandleParameter( nYMap, aPolar.Second ) )
                                            nFlags |= 0x1000;
                                        nFlags |= 8;
                                    }
                                }
                                else if ( rPropVal.Name == "RadiusRangeMinimum" )
                                {
                                    nYRangeMin = sal_Int32(0xff4c0000); // the range of angles seems to be a not
                                    nYRangeMax = sal_Int32(0x00b40000); // used feature, so we are defaulting this

                                    drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
                                    if ( rPropVal.Value >>= aRadiusRangeMinimum )
                                    {
                                        if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMin, aRadiusRangeMinimum ) )
                                            nFlags |= 0x80;
                                        nFlags |= 0x2000;
                                    }
                                }
                                else if ( rPropVal.Name == "RadiusRangeMaximum" )
                                {
                                    nYRangeMin = sal_Int32(0xff4c0000); // the range of angles seems to be a not
                                    nYRangeMax = sal_Int32(0x00b40000); // used feature, so we are defaulting this

                                    drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
                                    if ( rPropVal.Value >>= aRadiusRangeMaximum )
                                    {
                                        if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMax, aRadiusRangeMaximum ) )
                                            nFlags |= 0x100;
                                        nFlags |= 0x2000;
                                    }
                                }
                                else if ( rPropVal.Name == "RangeXMinimum" )
                                {
                                    drawing::EnhancedCustomShapeParameter aXRangeMinimum;
                                    if ( rPropVal.Value >>= aXRangeMinimum )
                                    {
                                        if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMin, aXRangeMinimum ) )
                                            nFlags |= 0x80;
                                        nFlags |= 0x20;
                                    }
                                }
                                else if ( rPropVal.Name == "RangeXMaximum" )
                                {
                                    drawing::EnhancedCustomShapeParameter aXRangeMaximum;
                                    if ( rPropVal.Value >>= aXRangeMaximum )
                                    {
                                        if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMax, aXRangeMaximum ) )
                                            nFlags |= 0x100;
                                        nFlags |= 0x20;
                                    }
                                }
                                else if ( rPropVal.Name == "RangeYMinimum" )
                                {
                                    drawing::EnhancedCustomShapeParameter aYRangeMinimum;
                                    if ( rPropVal.Value >>= aYRangeMinimum )
                                    {
                                        if ( GetValueForEnhancedCustomShapeHandleParameter( nYRangeMin, aYRangeMinimum ) )
                                            nFlags |= 0x200;
                                        nFlags |= 0x20;
                                    }
                                }
                                else if ( rPropVal.Name == "RangeYMaximum" )
                                {
                                    drawing::EnhancedCustomShapeParameter aYRangeMaximum;
                                    if ( rPropVal.Value >>= aYRangeMaximum )
                                    {
                                        if ( GetValueForEnhancedCustomShapeHandleParameter( nYRangeMax, aYRangeMaximum ) )
                                            nFlags |= 0x400;
                                        nFlags |= 0x20;
                                    }
                                }
                            }
                            aMemStrm.WriteUInt32( nFlags )
                               .WriteInt32( nXPosition )
                               .WriteInt32( nYPosition )
                               .WriteInt32( nXMap )
                               .WriteInt32( nYMap )
                               .WriteInt32( nXRangeMin )
                               .WriteInt32( nXRangeMax )
                               .WriteInt32( nYRangeMin )
                               .WriteInt32( nYRangeMax );

                            if ( nFlags & 8 )
                                nAdjustmentsWhichNeedsToBeConverted |= ( 1 << ( nYPosition - 0x100 ) );
                        }

                        AddOpt(DFF_Prop_Handles, true, 6, aMemStrm);
                    }
                    else
                    {
                        AddOpt(DFF_Prop_Handles, 0, true);
                    }
                }
            }
        }
        else if ( rProp.Name == sAdjustmentValues )
        {
            // it is required, that the information which handle is polar has already be read,
            // so we are able to change the polar value to a fixed float
            aAdjustmentValuesProp = rProp.Value;
            bAdjustmentValuesProp = true;
        }
    }
    if ( bAdjustmentValuesProp )
    {
        uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentSeq;
        if ( aAdjustmentValuesProp >>= aAdjustmentSeq )
        {
            if ( bPredefinedHandlesUsed )
                LookForPolarHandles( eShapeType, nAdjustmentsWhichNeedsToBeConverted );

            sal_Int32 k, nValue = 0, nAdjustmentValues = aAdjustmentSeq.getLength();
            for ( k = 0; k < nAdjustmentValues; k++ )
                if( GetAdjustmentValue( aAdjustmentSeq[ k ], k, nAdjustmentsWhichNeedsToBeConverted, nValue ) )
                    AddOpt( static_cast<sal_uInt16>( DFF_Prop_adjustValue + k ), static_cast<sal_uInt32>(nValue) );
        }
    }
    if( !bPathCoordinatesProp )
        return;

    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates;
    if ( !(aPathCoordinatesProp >>= aCoordinates) )
        return;

    // creating the vertices
    if (aCoordinates.hasElements())
    {
        sal_uInt16 j, nElements = static_cast<sal_uInt16>(aCoordinates.getLength());
        sal_uInt16 nElementSize = 8;
        sal_uInt32 nStreamSize = nElementSize * nElements + 6;
        SvMemoryStream aMemStrm( nStreamSize );
        aMemStrm.WriteUInt16( nElements )
           .WriteUInt16( nElements )
           .WriteUInt16( nElementSize );
        for( j = 0; j < nElements; j++ )
        {
            sal_Int32 X = GetValueForEnhancedCustomShapeParameter( aCoordinates[ j ].First, aEquationOrder, true );
            sal_Int32 Y = GetValueForEnhancedCustomShapeParameter( aCoordinates[ j ].Second, aEquationOrder, true );
            aMemStrm.WriteInt32( X )
               .WriteInt32( Y );
        }

        AddOpt(DFF_Prop_pVertices, true, 6, aMemStrm); // -6
    }
    else
    {
        AddOpt(DFF_Prop_pVertices, 0, true);
    }
}

MSO_SPT EscherPropertyContainer::GetCustomShapeType( const uno::Reference< drawing::XShape > & rXShape, ShapeFlag& nMirrorFlags, OUString& rShapeType, bool bOOXML )
{
    MSO_SPT eShapeType = mso_sptNil;
    nMirrorFlags = ShapeFlag::NONE;
    uno::Reference< beans::XPropertySet > aXPropSet( rXShape, uno::UNO_QUERY );
    if ( aXPropSet.is() )
    {
        try
        {
            uno::Any aGeoPropSet = aXPropSet->getPropertyValue( "CustomShapeGeometry" );
            uno::Sequence< beans::PropertyValue > aGeoPropSeq;
            if ( aGeoPropSet >>= aGeoPropSeq )
            {
                sal_Int32 i, nCount = aGeoPropSeq.getLength();
                for ( i = 0; i < nCount; i++ )
                {
                    const beans::PropertyValue& rProp = aGeoPropSeq[ i ];
                    if ( rProp.Name == "Type" )
                    {
                        if ( rProp.Value >>= rShapeType )
                        {
                            if (bOOXML)
                            {
                                // In case of VML export, try to handle the
                                // ooxml- prefix in rShapeType. If that fails,
                                // just do the same as the binary export.
                                OString aType = OUStringToOString(rShapeType, RTL_TEXTENCODING_UTF8);
                                eShapeType = msfilter::util::GETVMLShapeType(aType);
                                if (eShapeType == mso_sptNil)
                                    eShapeType = EnhancedCustomShapeTypeNames::Get(rShapeType);
                            }
                            else
                                eShapeType = EnhancedCustomShapeTypeNames::Get( rShapeType );
                        }
                    }
                    else if ( rProp.Name == "MirroredX" )
                    {
                        bool bMirroredX;
                        if ( ( rProp.Value >>= bMirroredX ) && bMirroredX )
                            nMirrorFlags  |= ShapeFlag::FlipH;
                    }
                    else if ( rProp.Name == "MirroredY" )
                    {
                        bool bMirroredY;
                        if ( ( rProp.Value >>= bMirroredY ) && bMirroredY )
                            nMirrorFlags  |= ShapeFlag::FlipV;
                    }
                }
            }
        }
        catch( const uno::Exception& )
        {
        }
    }
    return eShapeType;
}


// Implement for form control export
bool EscherPropertyContainer::CreateBlipPropertiesforOLEControl(const uno::Reference<beans::XPropertySet> & rXPropSet,
                                                                const uno::Reference<drawing::XShape> & rXShape)
{
    SdrObject* pShape = SdrObject::getSdrObjectFromXShape(rXShape);
    if ( pShape )
    {
        const Graphic aGraphic(SdrExchangeView::GetObjGraphic(*pShape));
        const GraphicObject aGraphicObject(aGraphic);

        if (!aGraphicObject.GetUniqueID().isEmpty())
        {
            if ( pGraphicProvider && pPicOutStrm && pShapeBoundRect )
            {
                sal_uInt32 nBlibId = pGraphicProvider->GetBlibID(*pPicOutStrm, aGraphicObject);
                if ( nBlibId )
                {
                    AddOpt( ESCHER_Prop_pib, nBlibId, true );
                    ImplCreateGraphicAttributes( rXPropSet, nBlibId, false );
                    return true;
                }
            }
        }
    }

    return false;
}

EscherPersistTable::EscherPersistTable()
{
}

EscherPersistTable::~EscherPersistTable()
{
}

bool EscherPersistTable::PtIsID( sal_uInt32 nID )
{
    for(auto const & pPtr : maPersistTable) {
        if ( pPtr->mnID == nID ) {
            return true;
        }
    }
    return false;
}

void EscherPersistTable::PtInsert( sal_uInt32 nID, sal_uInt32 nOfs )
{
    maPersistTable.push_back( std::make_unique<EscherPersistEntry>( nID, nOfs ) );
}

void EscherPersistTable::PtDelete( sal_uInt32 nID )
{
    auto it = std::find_if(maPersistTable.begin(), maPersistTable.end(),
        [&nID](const std::unique_ptr<EscherPersistEntry>& rxEntry) { return rxEntry->mnID == nID; });
    if (it != maPersistTable.end())
        maPersistTable.erase( it );
}

sal_uInt32 EscherPersistTable::PtGetOffsetByID( sal_uInt32 nID )
{
    for(auto const & pPtr : maPersistTable) {
        if ( pPtr->mnID == nID ) {
            return pPtr->mnOffset;
        }
    }
    return 0;
};

void EscherPersistTable::PtReplace( sal_uInt32 nID, sal_uInt32 nOfs )
{
    for(auto const & pPtr : maPersistTable) {
        if ( pPtr->mnID == nID ) {
            pPtr->mnOffset = nOfs;
            return;
        }
    }
}

void EscherPersistTable::PtReplaceOrInsert( sal_uInt32 nID, sal_uInt32 nOfs )
{
    for(auto const & pPtr : maPersistTable) {
        if ( pPtr->mnID == nID ) {
            pPtr->mnOffset = nOfs;
            return;
        }
    }
    PtInsert( nID, nOfs );
}

bool EscherPropertyValueHelper::GetPropertyValue(
    uno::Any& rAny,
    const uno::Reference<beans::XPropertySet> & rXPropSet,
    const OUString& rString,
    bool bTestPropertyAvailability)
{
    bool bRetValue = true;
    if ( bTestPropertyAvailability )
    {
        bRetValue = false;
        try
        {
            uno::Reference<beans::XPropertySetInfo>
                aXPropSetInfo( rXPropSet->getPropertySetInfo() );
            if ( aXPropSetInfo.is() )
                bRetValue = aXPropSetInfo->hasPropertyByName( rString );
        }
        catch( const uno::Exception& )
        {
            bRetValue = false;
        }
    }
    if ( bRetValue )
    {
        try
        {
            rAny = rXPropSet->getPropertyValue( rString );
            if ( !rAny.hasValue() )
                bRetValue = false;
        }
        catch( const uno::Exception& )
        {
            bRetValue = false;
        }
    }
    return bRetValue;
}

beans::PropertyState EscherPropertyValueHelper::GetPropertyState(
    const uno::Reference<beans::XPropertySet> & rXPropSet,
        const OUString& rPropertyName )
{
    beans::PropertyState eRetValue = beans::PropertyState_AMBIGUOUS_VALUE;
    try
    {
        uno::Reference<beans::XPropertyState> aXPropState
                ( rXPropSet, uno::UNO_QUERY );
        if ( aXPropState.is() )
            eRetValue = aXPropState->getPropertyState( rPropertyName );
    }
    catch( const uno::Exception& )
    {
    }
    return eRetValue;
}

EscherBlibEntry::EscherBlibEntry( sal_uInt32 nPictureOffset, const GraphicObject& rObject, const OString& rId,
                                        const GraphicAttr* pGraphicAttr ) :
    maPrefMapMode   ( rObject.GetPrefMapMode() ),
    maPrefSize      ( rObject.GetPrefSize() ),
    mnPictureOffset ( nPictureOffset ),
    mnRefCount      ( 1 ),
    mnSizeExtra     ( 0 ),
    mbIsEmpty       ( true )
{
    mbIsNativeGraphicPossible = ( pGraphicAttr == nullptr );
    meBlibType = UNKNOWN;
    mnSize = 0;

    sal_uInt32  nLen = static_cast<sal_uInt32>(rId.getLength());
    const char* pData = rId.getStr();
    GraphicType eType( rObject.GetType() );
    if (!(nLen && (eType != GraphicType::NONE)))
        return;

    mnIdentifier[ 0 ] = rtl_crc32( 0,pData, nLen );
    mnIdentifier[ 1 ] = 0;

    if ( pGraphicAttr )
    {
        if ( pGraphicAttr->IsSpecialDrawMode()
                || pGraphicAttr->IsMirrored()
                     || pGraphicAttr->IsCropped()
                        || pGraphicAttr->IsRotated()
                            || pGraphicAttr->IsTransparent()
                                || pGraphicAttr->IsAdjusted() )
        {
            SvMemoryStream aSt( sizeof( GraphicAttr ) );
            aSt.WriteUInt16( static_cast<sal_uInt16>(pGraphicAttr->GetDrawMode()) )
               .WriteUInt32( static_cast<sal_uInt32>(pGraphicAttr->GetMirrorFlags()) )
               .WriteInt32( pGraphicAttr->GetLeftCrop() )
               .WriteInt32( pGraphicAttr->GetTopCrop() )
               .WriteInt32( pGraphicAttr->GetRightCrop() )
               .WriteInt32( pGraphicAttr->GetBottomCrop() )
               .WriteUInt16( pGraphicAttr->GetRotation().get() )
               .WriteInt16( pGraphicAttr->GetLuminance() )
               .WriteInt16( pGraphicAttr->GetContrast() )
               .WriteInt16( pGraphicAttr->GetChannelR() )
               .WriteInt16( pGraphicAttr->GetChannelG() )
               .WriteInt16( pGraphicAttr->GetChannelB() )
               .WriteDouble( pGraphicAttr->GetGamma() );
            aSt.WriteBool( pGraphicAttr->IsInvert() )
               .WriteUChar( 255 - pGraphicAttr->GetAlpha() ); // transparency
            mnIdentifier[ 1 ] = rtl_crc32( 0, aSt.GetData(), aSt.Tell() );
        }
        else
            mbIsNativeGraphicPossible = true;
    }
    sal_uInt32 i, nTmp, n1, n2;
    n1 = n2 = 0;
    for ( i = 0; i < nLen; i++ )
    {
        nTmp = n2 >> 28;    // rotating 4 bit
        n2 <<= 4;
        n2 |= n1 >> 28;
        n1 <<= 4;
        n1 |= nTmp;
        n1 ^= *pData++ - '0';
    }
    mnIdentifier[ 2 ] = n1;
    mnIdentifier[ 3 ] = n2;
    mbIsEmpty = false;
};

void EscherBlibEntry::WriteBlibEntry( SvStream& rSt, bool bWritePictureOffset, sal_uInt32 nResize )
{
    sal_uInt32  nPictureOffset = bWritePictureOffset ? mnPictureOffset : 0;

    rSt.WriteUInt32( ( ESCHER_BSE << 16 ) | ( ( static_cast<sal_uInt16>(meBlibType) << 4 ) | 2 ) )
       .WriteUInt32( 36 + nResize )
       .WriteUChar( meBlibType );

    switch ( meBlibType )
    {
        case EMF :
        case WMF :  // converting EMF/WMF on OS2 to Pict
            rSt.WriteUChar( PICT );
        break;
        default:
            rSt.WriteUChar( meBlibType );
    }

    rSt.WriteBytes(&mnIdentifier[0], 16);
    rSt.WriteUInt16( 0 )
       .WriteUInt32( mnSize + mnSizeExtra )
       .WriteUInt32( mnRefCount )
       .WriteUInt32( nPictureOffset )
       .WriteUInt32( 0 );
}

EscherBlibEntry::~EscherBlibEntry()
{
};

bool EscherBlibEntry::operator==( const EscherBlibEntry& rEscherBlibEntry ) const
{
    for ( int i = 0; i < 3; i++ )
    {
        if ( mnIdentifier[ i ] != rEscherBlibEntry.mnIdentifier[ i ] )
            return false;
    }
    return true;
}

EscherGraphicProvider::EscherGraphicProvider( EscherGraphicProviderFlags nFlags ) :
    mnFlags         ( nFlags )
{
}

EscherGraphicProvider::~EscherGraphicProvider()
{
}

void EscherGraphicProvider::SetNewBlipStreamOffset( sal_Int32 nOffset )
{
    for( size_t i = 0; i < mvBlibEntrys.size(); i++ )
    {
        mvBlibEntrys[ i ]->mnPictureOffset += nOffset;
    }
}

sal_uInt32 EscherGraphicProvider::ImplInsertBlib( EscherBlibEntry* p_EscherBlibEntry )
{
    mvBlibEntrys.push_back( std::unique_ptr<EscherBlibEntry>(p_EscherBlibEntry) );
    return mvBlibEntrys.size();
}

sal_uInt32 EscherGraphicProvider::GetBlibStoreContainerSize( SvStream const * pMergePicStreamBSE ) const
{
    sal_uInt32 nSize = 44 * mvBlibEntrys.size() + 8;
    if ( pMergePicStreamBSE )
    {
        for ( size_t i = 0; i < mvBlibEntrys.size(); i++ )
            nSize += mvBlibEntrys[ i ]->mnSize + mvBlibEntrys[ i ]->mnSizeExtra;
    }
    return nSize;
}

void EscherGraphicProvider::WriteBlibStoreEntry(SvStream& rSt,
    sal_uInt32 nBlipId, sal_uInt32 nResize)
{
    if (nBlipId > mvBlibEntrys.size() || nBlipId == 0)
        return;
    mvBlibEntrys[nBlipId-1]->WriteBlibEntry(rSt, true/*bWritePictureOffSet*/, nResize);
}

void EscherGraphicProvider::WriteBlibStoreContainer( SvStream& rSt, SvStream* pMergePicStreamBSE )
{
    sal_uInt32  nSize = GetBlibStoreContainerSize( pMergePicStreamBSE );
    if ( !nSize )
        return;

    rSt.WriteUInt32( ( ESCHER_BstoreContainer << 16 ) | 0x1f )
       .WriteUInt32( nSize - 8 );

    if ( pMergePicStreamBSE )
    {
        sal_uInt32 nBlipSize, nOldPos = pMergePicStreamBSE->Tell();
        const sal_uInt32 nBuf = 0x40000;    // 256KB buffer
        std::unique_ptr<sal_uInt8[]> pBuf(new sal_uInt8[ nBuf ]);

        for ( size_t i = 0; i < mvBlibEntrys.size(); i++ )
        {
            EscherBlibEntry* pBlibEntry = mvBlibEntrys[ i ].get();

            ESCHER_BlibType nBlibType = pBlibEntry->meBlibType;
            nBlipSize = pBlibEntry->mnSize + pBlibEntry->mnSizeExtra;
            pBlibEntry->WriteBlibEntry( rSt, false, nBlipSize );

            // BLIP
            pMergePicStreamBSE->Seek( pBlibEntry->mnPictureOffset );
            sal_uInt16 n16;
            // record version and instance
            pMergePicStreamBSE->ReadUInt16( n16 );
            rSt.WriteUInt16( n16 );
            // record type
            pMergePicStreamBSE->ReadUInt16( n16 );
            rSt.WriteUInt16( ESCHER_BlipFirst + nBlibType );
            DBG_ASSERT( n16 == ESCHER_BlipFirst + nBlibType , "EscherGraphicProvider::WriteBlibStoreContainer: BLIP record types differ" );
            sal_uInt32 n32;
            // record size
            pMergePicStreamBSE->ReadUInt32( n32 );
            nBlipSize -= 8;
            rSt.WriteUInt32( nBlipSize );
            DBG_ASSERT( nBlipSize == n32, "EscherGraphicProvider::WriteBlibStoreContainer: BLIP sizes differ" );
            // record
            while ( nBlipSize )
            {
                sal_uInt32 nBytes = std::min( nBlipSize, nBuf );
                pMergePicStreamBSE->ReadBytes(pBuf.get(), nBytes);
                rSt.WriteBytes(pBuf.get(), nBytes);
                nBlipSize -= nBytes;
            }
        }
        pMergePicStreamBSE->Seek( nOldPos );
    }
    else
    {
        for ( size_t i = 0; i < mvBlibEntrys.size(); i++ )
            mvBlibEntrys[ i ]->WriteBlibEntry( rSt, true );
    }
}

bool EscherGraphicProvider::GetPrefSize( const sal_uInt32 nBlibId, Size& rPrefSize, MapMode& rPrefMapMode )
{
    bool bInRange = nBlibId && ( ( nBlibId - 1 ) < mvBlibEntrys.size() );
    if ( bInRange )
    {
        EscherBlibEntry* pEntry = mvBlibEntrys[ nBlibId - 1 ].get();
        rPrefSize = pEntry->maPrefSize;
        rPrefMapMode = pEntry->maPrefMapMode;
    }
    return bInRange;
}

sal_uInt32 EscherGraphicProvider::GetBlibID( SvStream& rPicOutStrm, GraphicObject const & rGraphicObject,
                                            const awt::Rectangle* pVisArea,
                                            const GraphicAttr* pGraphicAttr, const bool bOOxmlExport )
{
    sal_uInt32 nBlibId = 0;

    std::unique_ptr<EscherBlibEntry> p_EscherBlibEntry( new EscherBlibEntry( rPicOutStrm.Tell(), rGraphicObject, rGraphicObject.GetUniqueID(), pGraphicAttr ) );
    if ( !p_EscherBlibEntry->IsEmpty() )
    {
        for ( size_t i = 0; i < mvBlibEntrys.size(); i++ )
        {
            if ( *( mvBlibEntrys[ i ] ) == *p_EscherBlibEntry )
            {
                mvBlibEntrys[ i ]->mnRefCount++;
                return i + 1;
            }
        }

        bool            bUseNativeGraphic( false );

        Graphic             aGraphic(rGraphicObject.GetTransformedGraphic(pGraphicAttr));
        GfxLink             aGraphicLink;
        SvMemoryStream      aStream;

        const sal_uInt8*    pGraphicAry = nullptr;

        if ( p_EscherBlibEntry->mbIsNativeGraphicPossible && aGraphic.IsGfxLink() )
        {
            aGraphicLink = aGraphic.GetGfxLink();

            p_EscherBlibEntry->mnSize = aGraphicLink.GetDataSize();
            pGraphicAry = aGraphicLink.GetData();

            if ( p_EscherBlibEntry->mnSize && pGraphicAry )
            {
                switch ( aGraphicLink.GetType() )
                {
                    case GfxLinkType::NativeJpg : p_EscherBlibEntry->meBlibType = PEG; break;
                    case GfxLinkType::NativePng : p_EscherBlibEntry->meBlibType = PNG; break;

                    // #i15508# added BMP type for better exports; need to check this
                    // checked - does not work that way, so keep out for now. It may
                    // work somehow with direct DIB data, but that would need to be checked
                    // carefully
                    // for more comments please check RtfAttributeOutput::FlyFrameGraphic
                    //
                    // case GfxLinkType::NativeBmp : p_EscherBlibEntry->meBlibType = DIB; break;

                    case GfxLinkType::NativeWmf :
                    {
                        if ( aGraphicLink.IsEMF() )
                        {
                            p_EscherBlibEntry->meBlibType = EMF;
                        }
                        else if ( pGraphicAry && ( p_EscherBlibEntry->mnSize > 0x2c ) )
                        {
                            p_EscherBlibEntry->meBlibType = WMF;
                            if ( ( pGraphicAry[ 0 ] == 0xd7 ) && ( pGraphicAry[ 1 ] == 0xcd )
                                && ( pGraphicAry[ 2 ] == 0xc6 ) && ( pGraphicAry[ 3 ] == 0x9a ) )
                            {   // we have to get rid of the metafileheader
                                pGraphicAry += 22;
                                p_EscherBlibEntry->mnSize -= 22;
                            }
                        }
                    }
                    break;
                    default: break;
                }
                if ( p_EscherBlibEntry->meBlibType != UNKNOWN )
                    bUseNativeGraphic = true;
            }
        }
        if ( !bUseNativeGraphic )
        {
            GraphicType eGraphicType = aGraphic.GetType();
            if ( ( eGraphicType == GraphicType::Bitmap ) || ( eGraphicType == GraphicType::GdiMetafile ) )
            {
                ErrCode nErrCode;
                if ( !aGraphic.IsAnimated() )
                    nErrCode = GraphicConverter::Export( aStream, aGraphic, ( eGraphicType == GraphicType::Bitmap ) ? ConvertDataFormat::PNG  : ConvertDataFormat::EMF );
                else
                {   // to store an animation, a gif has to be included into the msOG chunk of a png  #I5583#
                    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
                    SvMemoryStream  aGIFStream;
                    const char* const pString = "MSOFFICE9.0";
                    aGIFStream.WriteBytes(pString, strlen(pString));
                    nErrCode = rFilter.ExportGraphic( aGraphic, OUString(), aGIFStream,
                        rFilter.GetExportFormatNumberForShortName( u"GIF" ) );
                    SAL_WARN_IF(
                        nErrCode != ERRCODE_NONE, "filter.ms",
                        "ExportGraphic to GIF failed with " << nErrCode);
                    if (nErrCode == ERRCODE_NONE)
                    {
                        sal_uInt32 nGIFSreamLen = aGIFStream.Tell();
                        uno::Sequence<sal_Int8> aGIFSeq( nGIFSreamLen );
                        sal_Int8* pSeq = aGIFSeq.getArray();
                        aGIFStream.Seek( STREAM_SEEK_TO_BEGIN );
                        aGIFStream.ReadBytes(pSeq, nGIFSreamLen);
                        beans::PropertyValue aChunkProp, aFilterProp;
                        aChunkProp.Name = "msOG";
                        aChunkProp.Value <<= aGIFSeq;
                        uno::Sequence<beans::PropertyValue> aAdditionalChunkSequence{ aChunkProp };
                        aFilterProp.Name = "AdditionalChunks";
                        aFilterProp.Value <<= aAdditionalChunkSequence;
                        uno::Sequence<beans::PropertyValue> aFilterData{ aFilterProp };
                        nErrCode = rFilter.ExportGraphic( aGraphic, OUString(), aStream,
                                                          rFilter.GetExportFormatNumberForShortName( u"PNG" ), &aFilterData );
                    }
                }
                if ( nErrCode == ERRCODE_NONE )
                {
                    p_EscherBlibEntry->meBlibType = ( eGraphicType == GraphicType::Bitmap ) ? PNG : EMF;
                    p_EscherBlibEntry->mnSize = aStream.TellEnd();
                    pGraphicAry = static_cast<sal_uInt8 const *>(aStream.GetData());
                }
            }
        }

        ESCHER_BlibType eBlibType = p_EscherBlibEntry->meBlibType;
        if ( p_EscherBlibEntry->mnSize && pGraphicAry && ( eBlibType != UNKNOWN ) )
        {
            sal_uInt32 nExtra, nAtomSize = 0;
            sal_uInt32 nInstance, nUncompressedSize = p_EscherBlibEntry->mnSize;

            if ( mnFlags & EscherGraphicProviderFlags::UseInstances )
            {
                rPicOutStrm.WriteUInt32( 0x7f90000 | static_cast<sal_uInt16>( mvBlibEntrys.size() << 4 ) )
                           .WriteUInt32( 0 );
                nAtomSize = rPicOutStrm.Tell();
                if ( eBlibType == PNG )
                    rPicOutStrm.WriteUInt16( 0x0606 );
                else if ( eBlibType == WMF )
                    rPicOutStrm.WriteUInt16( 0x0403 );
                else if ( eBlibType == EMF )
                    rPicOutStrm.WriteUInt16( 0x0402 );
                else if ( eBlibType == PEG )
                    rPicOutStrm.WriteUInt16( 0x0505 );
            }

            // fdo#69607 do not compress WMF files if we are in OOXML export
            if ( ( eBlibType == PEG ) || ( eBlibType == PNG ) // || ( eBlibType == DIB )) // #i15508#
                || ( ( ( eBlibType == WMF ) || ( eBlibType == EMF ) ) && bOOxmlExport ) )
            {
                nExtra = 17;
                p_EscherBlibEntry->mnSizeExtra = nExtra + 8;

                // #i15508# type see SvxMSDffManager::GetBLIPDirect (checked, does not work this way)
                // see RtfAttributeOutput::FlyFrameGraphic for more comments
                // maybe it would work with direct DIB data, but that would need thorough testing
                if( eBlibType == PNG )
                {
                    nInstance = 0xf01e6e00;
                }
                else // if( eBlibType == PEG )
                {
                    nInstance = 0xf01d46a0;
                }
                //else // eBlibType == DIB
                //{
                //    nInstance = 0xf01d7A80;
                //}

                // #i15508#
                //nInstance = ( eBlibType == PNG ) ? 0xf01e6e00 : 0xf01d46a0;


                rPicOutStrm.WriteUInt32( nInstance ).WriteUInt32( p_EscherBlibEntry->mnSize + nExtra );
                rPicOutStrm.WriteBytes(p_EscherBlibEntry->mnIdentifier, 16);
                rPicOutStrm.WriteUChar( 0xff );
                rPicOutStrm.WriteBytes(pGraphicAry, p_EscherBlibEntry->mnSize);
            }
            else
            {
                ZCodec aZCodec( 0x8000, 0x8000 );
                aZCodec.BeginCompression();
                SvMemoryStream aDestStrm;
                aZCodec.Write( aDestStrm, pGraphicAry, p_EscherBlibEntry->mnSize );
                aZCodec.EndCompression();
                p_EscherBlibEntry->mnSize = aDestStrm.TellEnd();
                pGraphicAry = static_cast<sal_uInt8 const *>(aDestStrm.GetData());
                if ( p_EscherBlibEntry->mnSize && pGraphicAry )
                {
                    nExtra = eBlibType == WMF ? 0x42 : 0x32;                                    // !EMF -> no change
                    p_EscherBlibEntry->mnSizeExtra = nExtra + 8;
                    nInstance = ( eBlibType == WMF ) ? 0xf01b2170 : 0xf01a3d40;                 // !EMF -> no change
                    rPicOutStrm.WriteUInt32( nInstance ).WriteUInt32( p_EscherBlibEntry->mnSize + nExtra );
                    if ( eBlibType == WMF )                                                     // !EMF -> no change
                        rPicOutStrm.WriteBytes(p_EscherBlibEntry->mnIdentifier, 16);
                    rPicOutStrm.WriteBytes(p_EscherBlibEntry->mnIdentifier, 16);

                    /*
                     ##913##
                     For Word the stored size of the graphic is critical the
                     metafile boundaries must match the actual graphics
                     boundaries, and the width and height must be in EMU's

                     If you don't do it this way then objects edited in the
                     msoffice app may show strange behaviour as the size jumps
                     around, and the original size and scaling factor in word
                     will be a very strange figure
                    */
                    sal_uInt32 nPrefWidth = p_EscherBlibEntry->maPrefSize.Width();
                    sal_uInt32 nPrefHeight = p_EscherBlibEntry->maPrefSize.Height();
                    sal_uInt32 nWidth, nHeight;
                    if ( pVisArea )
                    {
                        nWidth = pVisArea->Width * 360;
                        nHeight = pVisArea->Height * 360;
                    }
                    else
                    {
                        Size aPrefSize(lcl_SizeToEmu(p_EscherBlibEntry->maPrefSize, p_EscherBlibEntry->maPrefMapMode));
                        nWidth = aPrefSize.Width() * 360;
                        nHeight = aPrefSize.Height() * 360;
                    }
                    rPicOutStrm.WriteUInt32( nUncompressedSize ) // WMFSize without FileHeader
                   .WriteInt32( 0 )     // since we can't find out anymore what the original size of
                   .WriteInt32( 0 )     // the WMF (without Fileheader) was we write 10cm / x
                   .WriteUInt32( nPrefWidth )
                   .WriteUInt32( nPrefHeight )
                   .WriteUInt32( nWidth )
                   .WriteUInt32( nHeight )
                   .WriteUInt32( p_EscherBlibEntry->mnSize )
                   .WriteUInt16( 0xfe00 );  // compression Flags
                    rPicOutStrm.WriteBytes(pGraphicAry, p_EscherBlibEntry->mnSize);
                }
            }
            if ( nAtomSize )
            {
                sal_uInt32  nPos = rPicOutStrm.Tell();
                rPicOutStrm.Seek( nAtomSize - 4 );
                rPicOutStrm.WriteUInt32( nPos - nAtomSize );
                rPicOutStrm.Seek( nPos );
            }
            nBlibId = ImplInsertBlib( p_EscherBlibEntry.release() );
        }
    }
    return nBlibId;
}

namespace {

struct EscherConnectorRule
{
    sal_uInt32  nRuleId;
    sal_uInt32  nShapeA;        // SPID of shape A
    sal_uInt32  nShapeB;        // SPID of shape B
    sal_uInt32  nShapeC;        // SPID of connector shape
    sal_uInt32  ncptiA;         // Connection site Index of shape A
    sal_uInt32  ncptiB;         // Connection site Index of shape B
};

}

struct EscherShapeListEntry
{
    uno::Reference<drawing::XShape>aXShape;
    sal_uInt32 n_EscherId;

    EscherShapeListEntry(const uno::Reference<drawing::XShape> & rShape, sal_uInt32 nId)
        : aXShape(rShape)
        , n_EscherId(nId)
    {}
};

sal_uInt32 EscherConnectorListEntry::GetClosestPoint( const tools::Polygon& rPoly, const awt::Point& rPoint )
{
    sal_uInt16 nCount = rPoly.GetSize();
    sal_uInt16 nClosest = nCount;
    double fDist = sal_uInt32(0xffffffff);
    while( nCount-- )
    {
        double fDistance = hypot( rPoint.X - rPoly[ nCount ].X(), rPoint.Y - rPoly[ nCount ].Y() );
        if ( fDistance < fDist )
        {
            nClosest =  nCount;
            fDist = fDistance;
        }
    }
    return nClosest;
};


// for rectangles          for ellipses     for polygons
//
// nRule =  0 ->Top         0 ->Top         nRule = index to a (Poly)Polygon point
//          1 ->Left        2 ->Left
//          2 ->Bottom      4 ->Bottom
//          3 ->Right       6 ->Right

sal_uInt32 EscherConnectorListEntry::GetConnectorRule( bool bFirst )
{
    sal_uInt32 nRule = 0;

    uno::Any aAny;
    awt::Point aRefPoint( bFirst ? maPointA : maPointB );
    uno::Reference<drawing::XShape>
        aXShape( bFirst ? mXConnectToA : mXConnectToB );

    OUString aString(aXShape->getShapeType());
    OStringBuffer aBuf(OUStringToOString(aString, RTL_TEXTENCODING_UTF8));
    aBuf.remove( 0, 13 );   // removing "com.sun.star."
    sal_Int16 nPos = aBuf.toString().indexOf("Shape");
    aBuf.remove(nPos, 5);
    OString aType = aBuf.makeStringAndClear();

    uno::Reference<beans::XPropertySet>
        aPropertySet( aXShape, uno::UNO_QUERY );

    if ((aType == "drawing.PolyPolygon") || (aType == "drawing.PolyLine"))
    {
        if ( aPropertySet.is() )
        {
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aPropertySet, "PolyPolygon" ) )
            {
                auto pSourcePolyPolygon =
                    o3tl::doAccess<drawing::PointSequenceSequence>(aAny);
                sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->getLength();
                drawing::PointSequence const * pOuterSequence = pSourcePolyPolygon->getConstArray();

                if ( pOuterSequence )
                {
                    sal_Int32 a, b, nIndex = 0;
                    sal_uInt32 nDistance = 0xffffffff;
                    for( a = 0; a < nOuterSequenceCount; a++ )
                    {
                        drawing::PointSequence const * pInnerSequence = pOuterSequence++;
                        if ( pInnerSequence )
                        {
                            awt::Point const * pArray = pInnerSequence->getConstArray();
                            if ( pArray )
                            {
                                for ( b = 0; b < pInnerSequence->getLength(); b++, nIndex++, pArray++ )
                                {
                                    sal_uInt32 nDist = static_cast<sal_uInt32>(hypot( aRefPoint.X - pArray->X, aRefPoint.Y - pArray->Y ));
                                    if ( nDist < nDistance )
                                    {
                                        nRule = nIndex;
                                        nDistance = nDist;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if ((aType == "drawing.OpenBezier") || (aType == "drawing.OpenFreeHand") || (aType == "drawing.PolyLinePath")
        || (aType == "drawing.ClosedBezier") || ( aType == "drawing.ClosedFreeHand") || (aType == "drawing.PolyPolygonPath") )
    {
        uno::Reference<beans::XPropertySet>
            aPropertySet2( aXShape, uno::UNO_QUERY );
        if ( aPropertySet2.is() )
        {
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aPropertySet2, "PolyPolygonBezier" ) )
            {
                auto pSourcePolyPolygon =
                    o3tl::doAccess<drawing::PolyPolygonBezierCoords>(aAny);
                sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->Coordinates.getLength();

                // get pointer of inner sequences
                drawing::PointSequence const * pOuterSequence =
                    pSourcePolyPolygon->Coordinates.getConstArray();
                drawing::FlagSequence const *  pOuterFlags =
                    pSourcePolyPolygon->Flags.getConstArray();

                if ( pOuterSequence && pOuterFlags )
                {
                    sal_Int32 a, b, nIndex = 0;
                    sal_uInt32 nDistance = 0xffffffff;

                    for ( a = 0; a < nOuterSequenceCount; a++ )
                    {
                        drawing::PointSequence const * pInnerSequence = pOuterSequence++;
                        drawing::FlagSequence const *  pInnerFlags = pOuterFlags++;
                        if ( pInnerSequence && pInnerFlags )
                        {
                            awt::Point const * pArray = pInnerSequence->getConstArray();
                            drawing::PolygonFlags const * pFlags = pInnerFlags->getConstArray();
                            if ( pArray && pFlags )
                            {
                                for ( b = 0; b < pInnerSequence->getLength(); b++, pArray++ )
                                {
                                    drawing::PolygonFlags ePolyFlags = *pFlags++;
                                    if ( ePolyFlags == drawing::PolygonFlags_CONTROL )
                                        continue;
                                    sal_uInt32 nDist = static_cast<sal_uInt32>(hypot( aRefPoint.X - pArray->X, aRefPoint.Y - pArray->Y ));
                                    if ( nDist < nDistance )
                                    {
                                        nRule = nIndex;
                                        nDistance = nDist;
                                    }
                                    nIndex++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        bool bRectangularConnection = true;

        if (aType == "drawing.Custom")
        {
            if (auto pSdrObjCustomShape = dynamic_cast< SdrObjCustomShape* >(SdrObject::getSdrObjectFromXShape(aXShape)))
            {
                const SdrCustomShapeGeometryItem& rGeometryItem =
                    pSdrObjCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );

                OUString sShapeType;
                const uno::Any* pType = rGeometryItem.GetPropertyValueByName( "Type" );
                if ( pType )
                    *pType >>= sShapeType;
                MSO_SPT eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

                uno::Any* pGluePointType = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem).GetPropertyValueByName( "Path", "GluePointType" );

                sal_Int16 nGluePointType = sal_Int16();
                if ( !( pGluePointType &&
                        ( *pGluePointType >>= nGluePointType ) ) )
                    nGluePointType = GetCustomShapeConnectionTypeDefault( eSpType );

                if ( nGluePointType == drawing::EnhancedCustomShapeGluePointType::CUSTOM )
                {
                    const SdrGluePointList* pList = pSdrObjCustomShape->GetGluePointList();
                    if ( pList )
                    {
                        tools::Polygon aPoly;
                        sal_uInt16 nNum, nCnt = pList->GetCount();
                        if ( nCnt )
                        {
                            for ( nNum = 0; nNum < nCnt; nNum++ )
                            {
                                const SdrGluePoint& rGP = (*pList)[ nNum ];
                                Point aPt(rGP.GetAbsolutePos(*pSdrObjCustomShape));
                                aPoly.Insert( POLY_APPEND, aPt );
                            }
                            nRule = GetClosestPoint( aPoly, aRefPoint );
                            bRectangularConnection = false;
                        }
                    }
                }
                else if ( nGluePointType == drawing::EnhancedCustomShapeGluePointType::SEGMENTS )
                {
                    tools::PolyPolygon aPolyPoly;
                    SdrObjectUniquePtr pTemporaryConvertResultObject(pSdrObjCustomShape->DoConvertToPolyObj(true, true));
                    SdrPathObj* pSdrPathObj(dynamic_cast< SdrPathObj* >(pTemporaryConvertResultObject.get()));

                    if(pSdrPathObj)
                    {
                        // #i74631# use explicit constructor here. Also XPolyPolygon is not necessary,
                        // reducing to PolyPolygon
                        aPolyPoly = tools::PolyPolygon(pSdrPathObj->GetPathPoly());
                    }

                    // do *not* forget to delete the temporary used SdrObject - possible memory leak (!)
                    pTemporaryConvertResultObject.reset();
                    pSdrPathObj = nullptr;

                    if(0 != aPolyPoly.Count())
                    {
                        sal_Int16 nIndex = 0;
                        sal_uInt16 a, b;
                        sal_uInt32 nDistance = 0xffffffff;

                        for ( a = 0; a < aPolyPoly.Count(); a++ )
                        {
                            const tools::Polygon& rPoly = aPolyPoly.GetObject( a );
                            for ( b = 0; b < rPoly.GetSize(); b++ )
                            {
                                if ( rPoly.GetFlags( b ) != PolyFlags::Normal )
                                    continue;
                                const Point& rPt = rPoly[ b ];
                                sal_uInt32 nDist = static_cast<sal_uInt32>(hypot( aRefPoint.X - rPt.X(), aRefPoint.Y - rPt.Y() ));
                                if ( nDist < nDistance )
                                {
                                    nRule = nIndex;
                                    nDistance = nDist;
                                }
                                nIndex++;
                            }
                        }

                        if ( nDistance != 0xffffffff )
                            bRectangularConnection = false;
                    }
                }
            }
        }
        if ( bRectangularConnection )
        {
            awt::Point aPoint( aXShape->getPosition() );
            awt::Size  aSize( aXShape->getSize() );

            tools::Rectangle aRect( Point( aPoint.X, aPoint.Y ), Size( aSize.Width, aSize.Height ) );
            Point aCenter( aRect.Center() );
            tools::Polygon aPoly( 4 );

            aPoly[ 0 ] = Point( aCenter.X(), aRect.Top() );
            aPoly[ 1 ] = Point( aRect.Left(), aCenter.Y() );
            aPoly[ 2 ] = Point( aCenter.X(), aRect.Bottom() );
            aPoly[ 3 ] = Point( aRect.Right(), aCenter.Y() );

            sal_Int32 nAngle = ( EscherPropertyValueHelper::GetPropertyValue( aAny, aPropertySet, "RotateAngle", true ) )
                    ? *o3tl::doAccess<sal_Int32>(aAny) : 0;
            if ( nAngle )
                aPoly.Rotate( aRect.TopLeft(), Degree10(static_cast<sal_Int16>( ( nAngle + 5 ) / 10 )) );
            nRule = GetClosestPoint( aPoly, aRefPoint );

            if (aType == "drawing.Ellipse")
                nRule <<= 1;    // In PPT an ellipse has 8 ways to connect
        }
    }
    return nRule;
}

EscherSolverContainer::EscherSolverContainer()
{
}

EscherSolverContainer::~EscherSolverContainer()
{
}

void EscherSolverContainer::AddShape( const uno::Reference<drawing::XShape> & rXShape, sal_uInt32 nId )
{
    maShapeList.push_back( std::make_unique<EscherShapeListEntry>( rXShape, nId ) );
}

void EscherSolverContainer::AddConnector(
    const uno::Reference<drawing::XShape> & rConnector,
    const awt::Point& rPA,
    uno::Reference<drawing::XShape> const & rConA,
    const awt::Point& rPB,
    uno::Reference<drawing::XShape> const & rConB
)
{
    maConnectorList.push_back( std::make_unique<EscherConnectorListEntry>( rConnector, rPA, rConA, rPB, rConB ) );
}

sal_uInt32 EscherSolverContainer::GetShapeId( const uno::Reference<drawing::XShape> & rXShape ) const
{
    for (auto const & pPtr : maShapeList)
    {
        if ( rXShape == pPtr->aXShape )
            return pPtr->n_EscherId;
    }
    return 0;
}

void EscherSolverContainer::WriteSolver( SvStream& rStrm )
{
    sal_uInt32 nCount = maConnectorList.size();
    if ( !nCount )
        return;

    sal_uInt32  nRecHdPos, nCurrentPos, nSize;
    rStrm  .WriteUInt16( ( nCount << 4 ) | 0xf )    // open an ESCHER_SolverContainer
           .WriteUInt16( ESCHER_SolverContainer )
           .WriteUInt32( 0 );

    nRecHdPos = rStrm.Tell() - 4;

    EscherConnectorRule aConnectorRule;
    aConnectorRule.nRuleId = 2;
    for (auto const & pPtr : maConnectorList)
    {
        aConnectorRule.ncptiA  = aConnectorRule.ncptiB = 0xffffffff;
        aConnectorRule.nShapeC = GetShapeId( pPtr->mXConnector );
        aConnectorRule.nShapeA = GetShapeId( pPtr->mXConnectToA );
        aConnectorRule.nShapeB = GetShapeId( pPtr->mXConnectToB );

        if ( aConnectorRule.nShapeC )
        {
            if ( aConnectorRule.nShapeA )
                aConnectorRule.ncptiA = pPtr->GetConnectorRule( true );
            if ( aConnectorRule.nShapeB )
                aConnectorRule.ncptiB = pPtr->GetConnectorRule( false );
        }
        rStrm  .WriteUInt32( ( ESCHER_ConnectorRule << 16 ) | 1 )   // atom hd
               .WriteUInt32( 24 )
               .WriteUInt32( aConnectorRule.nRuleId )
               .WriteUInt32( aConnectorRule.nShapeA )
               .WriteUInt32( aConnectorRule.nShapeB )
               .WriteUInt32( aConnectorRule.nShapeC )
               .WriteUInt32( aConnectorRule.ncptiA )
               .WriteUInt32( aConnectorRule.ncptiB );

        aConnectorRule.nRuleId += 2;
    }

    nCurrentPos = rStrm.Tell();             // close the ESCHER_SolverContainer
    nSize = ( nCurrentPos - nRecHdPos ) - 4;
    rStrm.Seek( nRecHdPos );
    rStrm.WriteUInt32( nSize );
    rStrm.Seek( nCurrentPos );
}

EscherExGlobal::EscherExGlobal() :
    EscherGraphicProvider( EscherGraphicProviderFlags::NONE ),
    mpPicStrm( nullptr ),
    mbHasDggCont( false ),
    mbPicStrmQueried( false )
{
}

EscherExGlobal::~EscherExGlobal()
{
}

sal_uInt32 EscherExGlobal::GenerateDrawingId()
{
    // new drawing starts a new cluster in the cluster table (cluster identifiers are one-based)
    sal_uInt32 nClusterId = static_cast< sal_uInt32 >( maClusterTable.size() + 1 );
    // drawing identifiers are one-based
    sal_uInt32 nDrawingId = static_cast< sal_uInt32 >( maDrawingInfos.size() + 1 );
    // prepare new entries in the tables
    maClusterTable.emplace_back( nDrawingId );
    maDrawingInfos.emplace_back( nClusterId );
    // return the new drawing identifier
    return nDrawingId;
}

sal_uInt32 EscherExGlobal::GenerateShapeId( sal_uInt32 nDrawingId, bool bIsInSpgr )
{
    // drawing identifier is one-based
    // make sure the drawing is valid (bnc#656503)
    if ( nDrawingId == 0 )
        return 0;
    // create index from the identifier
    size_t nDrawingIdx = nDrawingId - 1;
    OSL_ENSURE( nDrawingIdx < maDrawingInfos.size(), "EscherExGlobal::GenerateShapeId - invalid drawing ID" );
    if( nDrawingIdx >= maDrawingInfos.size() )
        return 0;
    DrawingInfo& rDrawingInfo = maDrawingInfos[ nDrawingIdx ];

    // cluster identifier in drawing info struct is one-based
    ClusterEntry* pClusterEntry = &maClusterTable[ rDrawingInfo.mnClusterId - 1 ];

    // check cluster overflow, create new cluster entry
    if( pClusterEntry->mnNextShapeId == DFF_DGG_CLUSTER_SIZE )
    {
        // start a new cluster in the cluster table
        maClusterTable.emplace_back( nDrawingId );
        pClusterEntry = &maClusterTable.back();
        // new size of maClusterTable is equal to one-based identifier of the new cluster
        rDrawingInfo.mnClusterId = static_cast< sal_uInt32 >( maClusterTable.size() );
    }

    // build shape identifier from cluster identifier and next free cluster shape identifier
    rDrawingInfo.mnLastShapeId = static_cast< sal_uInt32 >( rDrawingInfo.mnClusterId * DFF_DGG_CLUSTER_SIZE + pClusterEntry->mnNextShapeId );
    // update free shape identifier in cluster entry
    ++pClusterEntry->mnNextShapeId;
    /*  Old code has counted the shapes only, if we are in a SPGRCONTAINER. Is
        this really intended? Maybe it's always true... */
    if( bIsInSpgr )
        ++rDrawingInfo.mnShapeCount;

    // return the new shape identifier
    return rDrawingInfo.mnLastShapeId;
}

sal_uInt32 EscherExGlobal::GetDrawingShapeCount( sal_uInt32 nDrawingId ) const
{
    size_t nDrawingIdx = nDrawingId - 1;
    OSL_ENSURE( nDrawingIdx < maDrawingInfos.size(), "EscherExGlobal::GetDrawingShapeCount - invalid drawing ID" );
    return (nDrawingIdx < maDrawingInfos.size()) ? maDrawingInfos[ nDrawingIdx ].mnShapeCount : 0;
}

sal_uInt32 EscherExGlobal::GetLastShapeId( sal_uInt32 nDrawingId ) const
{
    size_t nDrawingIdx = nDrawingId - 1;
    OSL_ENSURE( nDrawingIdx < maDrawingInfos.size(), "EscherExGlobal::GetLastShapeId - invalid drawing ID" );
    return (nDrawingIdx < maDrawingInfos.size()) ? maDrawingInfos[ nDrawingIdx ].mnLastShapeId : 0;
}

sal_uInt32 EscherExGlobal::GetDggAtomSize() const
{
    // 8 bytes header, 16 bytes fixed DGG data, 8 bytes for each cluster
    return static_cast< sal_uInt32 >( 24 + 8 * maClusterTable.size() );
}

void EscherExGlobal::WriteDggAtom( SvStream& rStrm ) const
{
    sal_uInt32 nDggSize = GetDggAtomSize();

    // write the DGG record header (do not include the 8 bytes of the header in the data size)
    rStrm.WriteUInt32( ESCHER_Dgg << 16 ).WriteUInt32( nDggSize - 8 );

    // calculate and write the fixed DGG data
    sal_uInt32 nShapeCount = 0;
    sal_uInt32 nLastShapeId = 0;
    for (auto const& drawingInfo : maDrawingInfos)
    {
        nShapeCount += drawingInfo.mnShapeCount;
        nLastShapeId = ::std::max( nLastShapeId, drawingInfo.mnLastShapeId );
    }
    // the non-existing cluster with index #0 is counted too
    sal_uInt32 nClusterCount = static_cast< sal_uInt32 >( maClusterTable.size() + 1 );
    sal_uInt32 nDrawingCount = static_cast< sal_uInt32 >( maDrawingInfos.size() );
    rStrm.WriteUInt32( nLastShapeId ).WriteUInt32( nClusterCount ).WriteUInt32( nShapeCount ).WriteUInt32( nDrawingCount );

    // write the cluster table
    for (auto const& elem : maClusterTable)
        rStrm.WriteUInt32( elem.mnDrawingId ).WriteUInt32( elem.mnNextShapeId );
}

SvStream* EscherExGlobal::QueryPictureStream()
{
    if( !mbPicStrmQueried )
    {
        mpPicStrm = ImplQueryPictureStream();
        mbPicStrmQueried = true;
    }
    return mpPicStrm;
}

SvStream* EscherExGlobal::ImplQueryPictureStream()
{
    return nullptr;
}

namespace {

// Implementation of an empty stream that silently succeeds, but does nothing.
//
// In fact, this is a hack.  The right solution is to abstract EscherEx to be
// able to work without SvStream; but at the moment it is better to live with
// this I guess.
class SvNullStream : public SvStream
{
protected:
    virtual std::size_t GetData( void* pData, std::size_t nSize ) override { memset( pData, 0, nSize ); return nSize; }
    virtual std::size_t PutData( const void*, std::size_t nSize ) override { return nSize; }
    virtual sal_uInt64 SeekPos( sal_uInt64 nPos ) override { return nPos; }
    virtual void SetSize( sal_uInt64 ) override {}
    virtual void FlushData() override {}

public:
    SvNullStream()  {}
};

}

EscherEx::EscherEx(const std::shared_ptr<EscherExGlobal>& rxGlobal, SvStream* pOutStrm, bool bOOXML)
    : mxGlobal(rxGlobal)
    , mpOutStrm(pOutStrm)
    , mbOwnsStrm(false)
    , mnCurrentDg(0)
    , mnCountOfs(0)
    , mnGroupLevel(0)
    , mnHellLayerId(SDRLAYER_NOTFOUND)
    , mbEscherSpgr(false)
    , mbEscherDg(false)
    , mbOOXML(bOOXML)
{
    if (!mpOutStrm)
    {
        mpOutStrm = new SvNullStream();
        mbOwnsStrm = true;
    }
    mnStrmStartOfs = mpOutStrm->Tell();
    mpImplEESdrWriter.reset( new ImplEESdrWriter( *this ) );
}

EscherEx::~EscherEx()
{
    if (mbOwnsStrm)
        delete mpOutStrm;
}

void EscherEx::Flush( SvStream* pPicStreamMergeBSE /* = NULL */ )
{
    if ( !mxGlobal->HasDggContainer() )
        return;

    // store the current stream position at ESCHER_Persist_CurrentPosition key
    PtReplaceOrInsert( ESCHER_Persist_CurrentPosition, mpOutStrm->Tell() );
    if ( DoSeek( ESCHER_Persist_Dgg ) )
    {
        /*  The DGG record is still not written. ESCHER_Persist_Dgg seeks
            to the place where the complete record has to be inserted. */
        InsertAtCurrentPos( mxGlobal->GetDggAtomSize() );
        mxGlobal->WriteDggAtom( *mpOutStrm );

        if ( mxGlobal->HasGraphics() )
        {
            /*  Calculate the total size of the BSTORECONTAINER including
                all BSE records containing the picture data contained in
                the passed in pPicStreamMergeBSE. */
            sal_uInt32 nBSCSize = mxGlobal->GetBlibStoreContainerSize( pPicStreamMergeBSE );
            if ( nBSCSize > 0 )
            {
                InsertAtCurrentPos( nBSCSize );
                mxGlobal->WriteBlibStoreContainer( *mpOutStrm, pPicStreamMergeBSE );
            }
        }

        /*  Forget the stream position stored for the DGG which is invalid
            after the call to InsertAtCurrentPos() anyway. */
        PtDelete( ESCHER_Persist_Dgg );
    }
    // seek to initial position (may be different due to inserted DGG and BLIPs)
    mpOutStrm->Seek( PtGetOffsetByID( ESCHER_Persist_CurrentPosition ) );
}

void EscherEx::InsertAtCurrentPos( sal_uInt32 nBytes )
{
    sal_uInt32  nSize, nType, nSource, nBufSize, nToCopy, nCurPos = mpOutStrm->Tell();

    // adjust persist table
    for(auto const & pPtr : maPersistTable) {
        sal_uInt32 nOfs = pPtr->mnOffset;
        if ( nOfs >= nCurPos ) {
            pPtr->mnOffset += nBytes;
        }
    }

    // adapt container and atom sizes
    mpOutStrm->Seek( mnStrmStartOfs );
    while ( mpOutStrm->Tell() < nCurPos )
    {
        mpOutStrm->ReadUInt32( nType ).ReadUInt32( nSize );
        sal_uInt32 nEndOfRecord = mpOutStrm->Tell() + nSize;
        bool bContainer = (nType & 0x0F) == 0x0F;
        /*  Expand the record, if the insertion position is inside, or if the
            position is at the end of a container (expands always), or at the
            end of an atom and bExpandEndOfAtom is set. */
        if ( (nCurPos < nEndOfRecord) || ((nCurPos == nEndOfRecord) && bContainer) )
        {
            mpOutStrm->SeekRel( -4 );
            mpOutStrm->WriteUInt32( nSize + nBytes );
            if ( !bContainer )
                mpOutStrm->SeekRel( nSize );
        }
        else
            mpOutStrm->SeekRel( nSize );
    }
    for (auto & offset : mOffsets)
    {
        if ( offset > nCurPos )
            offset += nBytes;
    }
    nSource = mpOutStrm->TellEnd();
    nToCopy = nSource - nCurPos;                        // increase the size of the stream by nBytes
    std::unique_ptr<sal_uInt8[]> pBuf(new sal_uInt8[ 0x40000 ]); // 256KB Buffer
    while ( nToCopy )
    {
        nBufSize = ( nToCopy >= 0x40000 ) ? 0x40000 : nToCopy;
        nToCopy -= nBufSize;
        nSource -= nBufSize;
        mpOutStrm->Seek( nSource );
        mpOutStrm->ReadBytes(pBuf.get(), nBufSize);
        mpOutStrm->Seek( nSource + nBytes );
        mpOutStrm->WriteBytes(pBuf.get(), nBufSize);
    }
    mpOutStrm->Seek( nCurPos );
}

void EscherEx::InsertPersistOffset( sal_uInt32 nKey, sal_uInt32 nOffset )
{
    PtInsert( ESCHER_Persist_PrivateEntry | nKey, nOffset );
}

void EscherEx::ReplacePersistOffset( sal_uInt32 nKey, sal_uInt32 nOffset )
{
    PtReplace( ESCHER_Persist_PrivateEntry | nKey, nOffset );
}

void EscherEx::SetEditAs( const OUString& rEditAs )
{
    mEditAs = rEditAs;
}

sal_uInt32 EscherEx::GetPersistOffset( sal_uInt32 nKey )
{
    return PtGetOffsetByID( ESCHER_Persist_PrivateEntry | nKey );
}

bool EscherEx::DoSeek( sal_uInt32 nKey )
{
    sal_uInt32 nPos = PtGetOffsetByID( nKey );
    if ( nPos )
        mpOutStrm->Seek( nPos );
    else
    {
        if (! PtIsID( nKey ) )
            return false;
        mpOutStrm->Seek( 0 );
    }
    return true;
}

bool EscherEx::SeekToPersistOffset( sal_uInt32 nKey )
{
    return DoSeek( ESCHER_Persist_PrivateEntry | nKey );
}

void EscherEx::InsertAtPersistOffset( sal_uInt32 nKey, sal_uInt32 nValue )
{
    sal_uInt32  nOldPos = mpOutStrm->Tell();
    bool        bRetValue = SeekToPersistOffset( nKey );
    if ( bRetValue )
    {
        mpOutStrm->WriteUInt32( nValue );
        mpOutStrm->Seek( nOldPos );
    }
}

void EscherEx::OpenContainer( sal_uInt16 nEscherContainer, int nRecInstance )
{
    mpOutStrm->WriteUInt16( ( nRecInstance << 4 ) | 0xf ).WriteUInt16( nEscherContainer ).WriteUInt32( 0 );
    mOffsets.push_back( mpOutStrm->Tell() - 4 );
    mRecTypes.push_back( nEscherContainer );
    switch( nEscherContainer )
    {
        case ESCHER_DggContainer :
        {
            mxGlobal->SetDggContainer();
            mnCurrentDg = 0;
            /*  Remember the current position as start position of the DGG
                record and BSTORECONTAINER, but do not write them actually.
                This will be done later in Flush() when the number of drawings,
                the size and contents of the FIDCL cluster table, and the size
                of the BLIP container are known. */
            PtReplaceOrInsert( ESCHER_Persist_Dgg, mpOutStrm->Tell() );
        }
        break;

        case ESCHER_DgContainer :
        {
            if ( mxGlobal->HasDggContainer() )
            {
                if ( !mbEscherDg )
                {
                    mbEscherDg = true;
                    mnCurrentDg = mxGlobal->GenerateDrawingId();
                    AddAtom( 8, ESCHER_Dg, 0, mnCurrentDg );
                    PtReplaceOrInsert( ESCHER_Persist_Dg | mnCurrentDg, mpOutStrm->Tell() );
                    mpOutStrm->WriteUInt32( 0 )     // The number of shapes in this drawing
                              .WriteUInt32( 0 );    // The last MSOSPID given to an SP in this DG
                }
            }
        }
        break;

        case ESCHER_SpgrContainer :
        {
            if ( mbEscherDg )
            {
                mbEscherSpgr = true;
            }
        }
        break;

        case ESCHER_SpContainer :
        {
        }
        break;

        default:
        break;
    }
}

void EscherEx::CloseContainer()
{
    sal_uInt32 nSize, nPos = mpOutStrm->Tell();
    nSize = ( nPos - mOffsets.back() ) - 4;
    mpOutStrm->Seek( mOffsets.back() );
    mpOutStrm->WriteUInt32( nSize );

    switch( mRecTypes.back() )
    {
        case ESCHER_DgContainer :
        {
            if ( mbEscherDg )
            {
                mbEscherDg = false;
                if ( DoSeek( ESCHER_Persist_Dg | mnCurrentDg ) )
                    mpOutStrm->WriteUInt32( mxGlobal->GetDrawingShapeCount( mnCurrentDg ) ).WriteUInt32( mxGlobal->GetLastShapeId( mnCurrentDg ) );
            }
        }
        break;

        case ESCHER_SpgrContainer :
        {
            if ( mbEscherSpgr )
            {
                mbEscherSpgr = false;

            }
        }
        break;

        default:
        break;
    }
    mOffsets.pop_back();
    mRecTypes.pop_back();
    mpOutStrm->Seek( nPos );
}

void EscherEx::BeginAtom()
{
    mnCountOfs = mpOutStrm->Tell();
    mpOutStrm->WriteUInt32( 0 ).WriteUInt32( 0 );       // record header will be written later
}

void EscherEx::EndAtom( sal_uInt16 nRecType, int nRecVersion, int nRecInstance )
{
    sal_uInt32  nOldPos = mpOutStrm->Tell();
    mpOutStrm->Seek( mnCountOfs );
    sal_uInt32 nSize = nOldPos - mnCountOfs;
    mpOutStrm->WriteUInt16( ( nRecInstance << 4 ) | ( nRecVersion & 0xf ) ).WriteUInt16( nRecType ).WriteUInt32( nSize - 8 );
    mpOutStrm->Seek( nOldPos );
}

void EscherEx::AddAtom( sal_uInt32 nAtomSize, sal_uInt16 nRecType, int nRecVersion, int nRecInstance )
{
    mpOutStrm->WriteUInt16( ( nRecInstance << 4 ) | ( nRecVersion & 0xf ) ).WriteUInt16( nRecType ).WriteUInt32( nAtomSize );
}

void EscherEx::AddChildAnchor( const tools::Rectangle& rRect )
{
    AddAtom( 16, ESCHER_ChildAnchor );
    mpOutStrm ->WriteInt32( rRect.Left() )
               .WriteInt32( rRect.Top() )
               .WriteInt32( rRect.Right() )
               .WriteInt32( rRect.Bottom() );
}

void EscherEx::AddClientAnchor( const tools::Rectangle& rRect )
{
    AddAtom( 8, ESCHER_ClientAnchor );
    mpOutStrm->WriteInt16( rRect.Top() )
              .WriteInt16( rRect.Left() )
              .WriteInt16( rRect.GetWidth()  + rRect.Left() )
              .WriteInt16( rRect.GetHeight() + rRect.Top() );
}

EscherExHostAppData* EscherEx::EnterAdditionalTextGroup()
{
    return nullptr;
}

sal_uInt32 EscherEx::EnterGroup( const OUString& rShapeName, const tools::Rectangle* pBoundRect )
{
    tools::Rectangle aRect;
    if( pBoundRect )
        aRect = *pBoundRect;

    OpenContainer( ESCHER_SpgrContainer );
    OpenContainer( ESCHER_SpContainer );
    AddAtom( 16, ESCHER_Spgr, 1 );
    PtReplaceOrInsert( ESCHER_Persist_Grouping_Snap | mnGroupLevel,
                        mpOutStrm->Tell() );
    mpOutStrm ->WriteInt32( aRect.Left() )  // Bounding box for the grouped shapes to which they will be attached
               .WriteInt32( aRect.Top() )
               .WriteInt32( aRect.IsWidthEmpty() ? aRect.Left() : aRect.Right() )
               .WriteInt32( aRect.IsHeightEmpty() ? aRect.Top() : aRect.Bottom() );

    sal_uInt32 nShapeId = GenerateShapeId();
    if ( !mnGroupLevel )
        AddShape( ESCHER_ShpInst_Min, ShapeFlag::Group | ShapeFlag::Patriarch, nShapeId );
    else
    {
        AddShape( ESCHER_ShpInst_Min, ShapeFlag::Group | ShapeFlag::HaveAnchor, nShapeId );
        EscherPropertyContainer aPropOpt;
        aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x00040004 );
        aPropOpt.AddOpt( ESCHER_Prop_dxWrapDistLeft, 0 );
        aPropOpt.AddOpt( ESCHER_Prop_dxWrapDistRight, 0 );

        // #i51348# shape name
        if( rShapeName.getLength() > 0 )
            aPropOpt.AddOpt( ESCHER_Prop_wzName, rShapeName );

        Commit( aPropOpt, aRect );
        if ( mnGroupLevel > 1 )
            AddChildAnchor( aRect );

        EscherExHostAppData* pAppData = mpImplEESdrWriter->ImplGetHostData();
        if( pAppData )
        {
            if ( mnGroupLevel <= 1 )
                pAppData->WriteClientAnchor( *this, aRect );
            pAppData->WriteClientData( *this );
        }
    }
    CloseContainer();                                               // ESCHER_SpContainer
    mnGroupLevel++;
    return nShapeId;
}

sal_uInt32 EscherEx::EnterGroup( const tools::Rectangle* pBoundRect )
{
    return EnterGroup( OUString(), pBoundRect );
}

void EscherEx::SetGroupSnapRect( sal_uInt32 nGroupLevel, const tools::Rectangle& rRect )
{
    if ( nGroupLevel )
    {
        sal_uInt32 nCurrentPos = mpOutStrm->Tell();
        if ( DoSeek( ESCHER_Persist_Grouping_Snap | ( nGroupLevel - 1 ) ) )
        {
            mpOutStrm ->WriteInt32( rRect.Left() )  // Bounding box for the grouped shapes to which they will be attached
                       .WriteInt32( rRect.Top() )
                       .WriteInt32( rRect.Right() )
                       .WriteInt32( rRect.Bottom() );
            mpOutStrm->Seek( nCurrentPos );
        }
    }
}

void EscherEx::SetGroupLogicRect( sal_uInt32 nGroupLevel, const tools::Rectangle& rRect )
{
    if ( nGroupLevel )
    {
        sal_uInt32 nCurrentPos = mpOutStrm->Tell();
        if ( DoSeek( ESCHER_Persist_Grouping_Logic | ( nGroupLevel - 1 ) ) )
        {
            mpOutStrm->WriteInt16( rRect.Top() ).WriteInt16( rRect.Left() ).WriteInt16( rRect.Right() ).WriteInt16( rRect.Bottom() );
            mpOutStrm->Seek( nCurrentPos );
        }
    }
}

void EscherEx::LeaveGroup()
{
    --mnGroupLevel;
    PtDelete( ESCHER_Persist_Grouping_Snap | mnGroupLevel );
    PtDelete( ESCHER_Persist_Grouping_Logic | mnGroupLevel );
    CloseContainer();
}

void EscherEx::AddShape( sal_uInt32 nShpInstance, ShapeFlag nFlags, sal_uInt32 nShapeID )
{
    AddAtom( 8, ESCHER_Sp, 2, nShpInstance );

    if ( !nShapeID )
        nShapeID = GenerateShapeId();

    if (nFlags ^ ShapeFlag::Group) // no pure group shape
    {
        if ( mnGroupLevel > 1 )
            nFlags |= ShapeFlag::Child; // this not a topmost shape
    }
    mpOutStrm->WriteUInt32( nShapeID ).WriteUInt32( static_cast<sal_uInt32>(nFlags) );
}

void EscherEx::Commit( EscherPropertyContainer& rProps, const tools::Rectangle& )
{
    rProps.Commit( GetStream() );
}

sal_uInt32 EscherEx::GetColor( const sal_uInt32 nSOColor )
{
    sal_uInt32 nColor = nSOColor & 0xff00;          // Green
    nColor |= static_cast<sal_uInt8>(nSOColor) << 16;        // Red
    nColor |= static_cast<sal_uInt8>( nSOColor >> 16 );        // Blue
    return nColor;
}

sal_uInt32 EscherEx::GetColor( const Color& rSOColor )
{
    sal_uInt32 nColor = ( rSOColor.GetRed() << 16 );
    nColor |= ( rSOColor.GetGreen() << 8 );
    nColor |= rSOColor.GetBlue();
    nColor = GetColor( nColor );
    return nColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
