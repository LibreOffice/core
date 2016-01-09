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
#include <svx/svdxcgv.hxx>
#include <svx/svdomedia.hxx>
#include <svx/xflftrit.hxx>
#include <filter/msfilter/escherex.hxx>
#include <filter/msfilter/util.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/outlobj.hxx>
#include <vcl/gradient.hxx>
#include <vcl/graph.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/stream.hxx>
#include <tools/zcodec.hxx>
#include <svx/svdopath.hxx>
#include <stdlib.h>
#include <vcl/graphicfilter.hxx>
#include "svx/EnhancedCustomShapeTypeNames.hxx"
#include "svx/EnhancedCustomShapeGeometry.hxx"
#include <svx/EnhancedCustomShapeFunctionParser.hxx>
#include "svx/EnhancedCustomShape2d.hxx"
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/awt/GradientStyle.hpp>
#include <com/sun/star/awt/RasterOperation.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/BezierPoint.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/ConnectionType.hpp>
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
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <comphelper/string.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/virdev.hxx>
#include <rtl/crc.h>
#include <rtl/strbuf.hxx>
#include <memory>

using namespace ::com::sun::star;


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
    Rectangle * pBoundRect):
    pGraphicProvider(pGraphProv),
    pPicOutStrm(pPiOutStrm),
    pShapeBoundRect(pBoundRect),
    nSortCount(0),
    nSortBufSize(64),
    nCountCount(0),
    nCountSize(0),
    pSortStruct(new EscherPropSortStruct[nSortBufSize]),
    bHasComplexData(false)
{}

EscherPropertyContainer::EscherPropertyContainer()
    : EscherPropertyContainer(nullptr, nullptr, nullptr)
{}

EscherPropertyContainer::EscherPropertyContainer(
    EscherGraphicProvider& rGraphProv,
            SvStream* pPiOutStrm,
                Rectangle& rBoundRect ) :
    EscherPropertyContainer(&rGraphProv, pPiOutStrm, &rBoundRect)
{}

EscherPropertyContainer::~EscherPropertyContainer()
{
    if ( bHasComplexData )
    {
        while ( nSortCount-- )
            delete[] pSortStruct[ nSortCount ].pBuf;
    }
    delete[] pSortStruct;
};

void EscherPropertyContainer::AddOpt( sal_uInt16 nPropID, sal_uInt32 nPropValue, bool bBlib )
{
    AddOpt( nPropID, bBlib, nPropValue, nullptr, 0 );
}

void EscherPropertyContainer::AddOpt( sal_uInt16 nPropID, const OUString& rString )
{
    sal_Int32 j, i, nLen = rString.getLength() * 2 + 2;
    sal_uInt8* pBuf = new sal_uInt8[ nLen ];
    for ( j = i = 0; i < rString.getLength(); i++ )
    {
        sal_uInt16 nChar = (sal_uInt16)rString[ i ];
        pBuf[ j++ ] = (sal_uInt8)nChar;
        pBuf[ j++ ] = (sal_uInt8)( nChar >> 8 );
    }
    pBuf[ j++ ] = 0;
    pBuf[ j++ ] = 0;
    AddOpt( nPropID, true, nLen, pBuf, nLen );
}

void EscherPropertyContainer::AddOpt( sal_uInt16 nPropID, bool bBlib, sal_uInt32 nPropValue, sal_uInt8* pProp, sal_uInt32 nPropSize )
{
    if ( bBlib )                // bBlib is only valid when fComplex = 0
        nPropID |= 0x4000;
    if ( pProp )
        nPropID |= 0x8000;      // fComplex = sal_True;

    sal_uInt32 i;
    for( i = 0; i < nSortCount; i++ )
    {
        if ( ( pSortStruct[ i ].nPropId &~0xc000 ) == ( nPropID &~0xc000 ) )    // check, whether the Property only gets replaced
        {
            pSortStruct[ i ].nPropId = nPropID;
            if ( pSortStruct[ i ].pBuf )
            {
                nCountSize -= pSortStruct[ i ].nPropSize;
                delete[] pSortStruct[ i ].pBuf;
            }
            pSortStruct[ i ].pBuf = pProp;
            pSortStruct[ i ].nPropSize = nPropSize;
            pSortStruct[ i ].nPropValue = nPropValue;
            if ( pProp )
                nCountSize += nPropSize;
            return;
        }
    }
    nCountCount++;
    nCountSize += 6;
    if ( nSortCount == nSortBufSize )                                           // increase buffer
    {
        nSortBufSize <<= 1;
        EscherPropSortStruct* pTemp = new EscherPropSortStruct[ nSortBufSize ];
        for( i = 0; i < nSortCount; i++ )
        {
            pTemp[ i ] = pSortStruct[ i ];
        }
        delete[] pSortStruct;
        pSortStruct = pTemp;
    }
    pSortStruct[ nSortCount ].nPropId = nPropID;                                // insert property
    pSortStruct[ nSortCount ].pBuf = pProp;
    pSortStruct[ nSortCount ].nPropSize = nPropSize;
    pSortStruct[ nSortCount++ ].nPropValue = nPropValue;

    if ( pProp )
    {
        nCountSize += nPropSize;
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
    for( sal_uInt32 i = 0; i < nSortCount; i++ )
    {
        if ( ( pSortStruct[ i ].nPropId &~0xc000 ) == ( nPropId &~0xc000 ) )
        {
            rPropValue = pSortStruct[ i ];
            return true;
        }
    }
    return false;
}

EscherProperties EscherPropertyContainer::GetOpts() const
{
    EscherProperties aVector;

    for ( sal_uInt32 i = 0; i < nSortCount; ++i )
        aVector.push_back( pSortStruct[ i ] );

    return aVector;
}

extern "C" int SAL_CALL EscherPropSortFunc( const void* p1, const void* p2 )
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

void EscherPropertyContainer::Commit( SvStream& rSt, sal_uInt16 nVersion, sal_uInt16 nRecType )
{
    rSt.WriteUInt16( ( nCountCount << 4 ) | ( nVersion & 0xf ) ).WriteUInt16( nRecType ).WriteUInt32( nCountSize );
    if ( nSortCount )
    {
        qsort( pSortStruct, nSortCount, sizeof( EscherPropSortStruct ), EscherPropSortFunc );
        sal_uInt32 i;

        for ( i = 0; i < nSortCount; i++ )
        {
            sal_uInt32 nPropValue = pSortStruct[ i ].nPropValue;
            sal_uInt16 nPropId = pSortStruct[ i ].nPropId;

            rSt.WriteUInt16( nPropId )
               .WriteUInt32( nPropValue );
        }
        if ( bHasComplexData )
        {
            for ( i = 0; i < nSortCount; i++ )
            {
                if ( pSortStruct[ i ].pBuf )
                    rSt.Write( pSortStruct[ i ].pBuf, pSortStruct[ i ].nPropSize );
            }
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
        nColor |= (sal_uInt8)( nSOColor ) << 16;    // red
        nColor |= (sal_uInt8)( nSOColor >> 16 );    // blue
        return nColor;
    }
    else
        return nSOColor & 0xffffff;
}

sal_uInt32 EscherPropertyContainer::GetGradientColor(
    const css::awt::Gradient* pGradient,
        sal_uInt32 nStartColor )
{
    sal_uInt32  nIntensity = 100;
    Color       aColor;

    if ( pGradient )
    {
        if ( nStartColor & 1 )
        {
            nIntensity = pGradient->StartIntensity;
            aColor = pGradient->StartColor;
        }
        else
        {
            nIntensity = pGradient->EndIntensity;
            aColor = pGradient->EndColor;
        }
    }
    sal_uInt32  nRed = ( aColor.GetRed() * nIntensity ) / 100;
    sal_uInt32  nGreen = ( ( aColor.GetGreen() * nIntensity ) / 100 ) << 8;
    sal_uInt32  nBlue = ( ( aColor.GetBlue() * nIntensity ) / 100 ) << 16;
    return nRed | nGreen | nBlue;
}

void EscherPropertyContainer::CreateGradientProperties(
    const css::awt::Gradient & rGradient )
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
        case css::awt::GradientStyle_LINEAR :
        case css::awt::GradientStyle_AXIAL :
        {
            nFillType = ESCHER_FillShadeScale;
            nAngle = (rGradient.Angle * 0x10000) / 10;
            nFillFocus = (sal::static_int_cast<int>(rGradient.Style) ==
                          sal::static_int_cast<int>(GradientStyle_LINEAR)) ? 0 : 50;
        }
        break;
        case css::awt::GradientStyle_RADIAL :
        case css::awt::GradientStyle_ELLIPTICAL :
        case css::awt::GradientStyle_SQUARE :
        case css::awt::GradientStyle_RECT :
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
        case css::awt::GradientStyle_MAKE_FIXED_SIZE : break;
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
    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet , bool bTransparentGradient)
{
    css::uno::Any          aAny;
    css::awt::Gradient const * pGradient = nullptr;

    sal_uInt32  nFillType = ESCHER_FillShadeScale;
    sal_Int32  nAngle = 0;
    sal_uInt32  nFillFocus = 0;
    sal_uInt32  nFillLR = 0;
    sal_uInt32  nFillTB = 0;
    sal_uInt32  nFirstColor = 0;// like the control var nChgColors in import logic
    bool        bWriteFillTo = false;

    // Transparency gradient: Means the third setting in transparency page is set
    if (bTransparentGradient &&  EscherPropertyValueHelper::GetPropertyValue(
        aAny, rXPropSet, "FillTransparenceGradient" ) )
    {
        pGradient = static_cast<css::awt::Gradient const *>(aAny.getValue());

        css::uno::Any          aAnyTemp;
        const rtl::OUString aPropName( "FillStyle" );
        if ( EscherPropertyValueHelper::GetPropertyValue(
            aAnyTemp, rXPropSet, aPropName ) )
        {
            css::drawing::FillStyle eFS;
            if ( ! ( aAnyTemp >>= eFS ) )
                eFS = css::drawing::FillStyle_SOLID;
            // solid and transparency
            if ( eFS == css::drawing::FillStyle_SOLID)
            {
                if ( EscherPropertyValueHelper::GetPropertyValue(
                    aAnyTemp, rXPropSet, "FillColor" ) )
                {
                    const_cast<css::awt::Gradient *>(pGradient)->StartColor = ImplGetColor( *static_cast<sal_uInt32 const *>(aAnyTemp.getValue()), false );
                    const_cast<css::awt::Gradient *>(pGradient)->EndColor = ImplGetColor( *static_cast<sal_uInt32 const *>(aAnyTemp.getValue()), false );
                }
            }
            // gradient and transparency.
            else if( eFS == css::drawing::FillStyle_GRADIENT )
            {
                if ( EscherPropertyValueHelper::GetPropertyValue(
                    aAny, rXPropSet, "FillGradient" ) )
                    pGradient = static_cast<css::awt::Gradient const *>(aAny.getValue());
            }
        }

    }
    // Not transparency gradient
    else if ( EscherPropertyValueHelper::GetPropertyValue(
        aAny, rXPropSet, "FillGradient" ) )
    {
        pGradient = static_cast<css::awt::Gradient const *>(aAny.getValue());
    }

    if ( pGradient )
    {
        switch ( pGradient->Style )
        {
        case css::awt::GradientStyle_LINEAR :
        case css::awt::GradientStyle_AXIAL :
            {
                nFillType = ESCHER_FillShadeScale;
                nAngle = pGradient->Angle;
                while ( nAngle > 0 ) nAngle -= 3600;
                while ( nAngle <= -3600 ) nAngle += 3600;
                // Value of the real number = Integral + (Fractional / 65536.0)
                nAngle = ( nAngle * 0x10000) / 10;

                nFillFocus = (pGradient->Style == css::awt::GradientStyle_LINEAR) ?
                            ( pGradient->XOffset + pGradient->YOffset )/2 : -50;
                if( !nFillFocus )
                    nFirstColor=nFirstColor ^ 1;
                if ( !nAngle )
                    nFirstColor=nFirstColor ^ 1;
            }
            break;
        case css::awt::GradientStyle_RADIAL :
        case css::awt::GradientStyle_ELLIPTICAL :
        case css::awt::GradientStyle_SQUARE :
        case css::awt::GradientStyle_RECT :
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
        pGradient = static_cast<css::awt::Gradient const *>(aAny.getValue());
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
    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
    bool bEdge ,  const css::uno::Reference< css::drawing::XShape > & rXShape )
{
    if ( rXShape.is() )
    {
        SdrObject* pObj = GetSdrObjectFromXShape( rXShape );
        if ( pObj )
        {
            SfxItemSet aAttr( pObj->GetMergedItemSet() );
            // tranparency with gradient. Means the third setting in transparency page is set
            bool bTransparentGradient =  ( aAttr.GetItemState( XATTR_FILLFLOATTRANSPARENCE ) == SfxItemState::SET ) &&
                static_cast<const XFillFloatTransparenceItem&>( aAttr.Get( XATTR_FILLFLOATTRANSPARENCE ) ).IsEnabled();
            CreateFillProperties(  rXPropSet, bEdge, bTransparentGradient );
        }
    }
}

void EscherPropertyContainer::CreateFillProperties(
    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
    bool bEdge , bool bTransparentGradient)

{
    css::uno::Any aAny;
    AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapNone );
    AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );
    const OUString aPropName( "FillStyle" );

    if ( EscherPropertyValueHelper::GetPropertyValue(
            aAny, rXPropSet, aPropName ) )
    {
        css::drawing::FillStyle eFS;
        if ( ! ( aAny >>= eFS ) )
            eFS = css::drawing::FillStyle_SOLID;
        sal_uInt32 nFillBackColor = 0;
        switch( eFS )
        {
            case css::drawing::FillStyle_GRADIENT :
            {
                CreateGradientProperties( rXPropSet , bTransparentGradient );
                AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
            }
            break;

            case css::drawing::FillStyle_BITMAP :
            {
                CreateGraphicProperties( rXPropSet, "FillBitmapURL", true );
                AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
                AddOpt( ESCHER_Prop_fillBackColor, nFillBackColor  );
            }
            break;
            case css::drawing::FillStyle_HATCH :
            {
                CreateGraphicProperties( rXPropSet, "FillHatch", true );
            }
            break;
            case css::drawing::FillStyle_SOLID :
            default:
            {
                if ( bTransparentGradient )
                    CreateGradientProperties( rXPropSet , bTransparentGradient );
                else
                {
                    css::beans::PropertyState ePropState = EscherPropertyValueHelper::GetPropertyState(
                        rXPropSet, aPropName );
                    if ( ePropState == css::beans::PropertyState_DIRECT_VALUE )
                        AddOpt( ESCHER_Prop_fillType, ESCHER_FillSolid );

                    if ( EscherPropertyValueHelper::GetPropertyValue(
                            aAny, rXPropSet, "FillColor" ) )
                    {
                        sal_uInt32 nFillColor = ImplGetColor( *static_cast<sal_uInt32 const *>(aAny.getValue()) );
                        nFillBackColor = nFillColor ^ 0xffffff;
                        AddOpt( ESCHER_Prop_fillColor, nFillColor );
                    }
                    AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100010 );
                    AddOpt( ESCHER_Prop_fillBackColor, nFillBackColor );
                }
                break;
            }
            case css::drawing::FillStyle_NONE :
                AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
            break;
        }
        if ( eFS != css::drawing::FillStyle_NONE )
        {
            sal_uInt16 nTransparency = ( EscherPropertyValueHelper::GetPropertyValue(
                aAny, rXPropSet, "FillTransparence", true ) )
                ? *static_cast<sal_Int16 const *>(aAny.getValue()) : 0;
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
    if (bIsTextFrame && !bIsCustomShape)
    {
        sal_uInt16 nAngle = EscherPropertyValueHelper::GetPropertyValue(
            aAny, rXPropSet, "RotateAngle", true ) ?
                (sal_uInt16)( ( *static_cast<sal_Int32 const *>(aAny.getValue()) ) + 5 ) / 10 : 0;
        if (nAngle==900)
        {
            AddOpt( ESCHER_Prop_txflTextFlow, ESCHER_txflBtoT );
        }
        if (nAngle==2700)
        {
            AddOpt( ESCHER_Prop_txflTextFlow, ESCHER_txflTtoBA );
        }
    }
}

bool EscherPropertyContainer::GetLineArrow( const bool bLineStart,
    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
        ESCHER_LineEnd& reLineEnd, sal_Int32& rnArrowLength, sal_Int32& rnArrowWidth )
{
    static const char sLineStart    [] = "LineStart";
    static const char sLineStartName[] = "LineStartName";
    static const char sLineEnd      [] = "LineEnd";
    static const char sLineEndName  [] = "LineEndName";

    const OUString sLine      ( bLineStart ? OUString(sLineStart) : OUString(sLineEnd) );
    const OUString sLineName  ( bLineStart ? OUString(sLineStartName) : OUString(sLineEndName) );

    bool bIsArrow = false;

    css::uno::Any aAny;
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
                OUString        aArrowStartName = *static_cast<OUString const *>(aAny.getValue());
                sal_Int16       nWhich = bLineStart ? XATTR_LINESTART : XATTR_LINEEND;

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
                    bool b = true;
                    OUString aArrowName( aArrowStartName.getToken( 0, ' ' ) );
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
                        b = false;

                    // now we have the arrow, and try to determine the arrow size;
                    if ( b )
                    {
                        OUString aArrowSize( aArrowStartName.getToken( 1, ' ' ) );
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
    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
        bool bEdge )
{
    css::uno::Any aAny;
    sal_uInt32 nLineFlags = 0x80008;

    ESCHER_LineEnd eLineEnd;
    sal_Int32 nArrowLength;
    sal_Int32 nArrowWidth;

    bool bSwapLineEnds = false;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "CircleKind", true ) )
    {
        css::drawing::CircleKind  eCircleKind;
        if ( aAny >>= eCircleKind )
        {
            if ( eCircleKind == css::drawing::CircleKind_ARC )
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
        css::drawing::LineCap aLineCap(css::drawing::LineCap_BUTT);

        if(aAny >>= aLineCap)
        {
            switch (aLineCap)
            {
                default: /* css::drawing::LineCap_BUTT */
                {
                    AddOpt(ESCHER_Prop_lineEndCapStyle, ESCHER_LineEndCapFlat);
                    break;
                }
                case css::drawing::LineCap_ROUND:
                {
                    AddOpt(ESCHER_Prop_lineEndCapStyle, ESCHER_LineEndCapRound);
                    break;
                }
                case css::drawing::LineCap_SQUARE:
                {
                    AddOpt(ESCHER_Prop_lineEndCapStyle, ESCHER_LineEndCapSquare);
                    break;
                }
            }
        }
    }

    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "LineStyle" ) )
    {
        css::drawing::LineStyle eLS;
        if ( aAny >>= eLS )
        {
            switch ( eLS )
            {
                case css::drawing::LineStyle_NONE :
                    AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );           // 80000
                break;

                case css::drawing::LineStyle_DASH :
                {
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "LineDash" ) )
                    {
                        ESCHER_LineDashing eDash = ESCHER_LineSolid;
                        css::drawing::LineDash const * pLineDash = static_cast<css::drawing::LineDash const *>(aAny.getValue());
                        sal_Int32 nDistance = pLineDash->Distance << 1;
                        switch ( pLineDash->Style )
                        {
                            case css::drawing::DashStyle_ROUND :
                            case css::drawing::DashStyle_ROUNDRELATIVE :
                                AddOpt( ESCHER_Prop_lineEndCapStyle, 0 ); // set Style Round
                            break;
                            default : break;
                        }
                        if ( ((!(pLineDash->Dots )) || (!(pLineDash->Dashes )) ) || ( pLineDash->DotLen == pLineDash->DashLen ) )
                        {
                            sal_Int32 nLen = pLineDash->DotLen;
                            if ( pLineDash->Dashes )
                                nLen = pLineDash->DashLen;

                            if ( nLen >= nDistance )
                                eDash = ESCHER_LineLongDashGEL;
                            else if ( pLineDash->Dots )
                                eDash = ESCHER_LineDotSys;
                            else
                                eDash = ESCHER_LineDashGEL;
                        }
                        else                                                            // X Y
                        {
                            if ( pLineDash->Dots != pLineDash->Dashes )
                            {
                                if ( ( pLineDash->DashLen > nDistance ) || ( pLineDash->DotLen > nDistance ) )
                                    eDash = ESCHER_LineLongDashDotDotGEL;
                                else
                                    eDash = ESCHER_LineDashDotDotSys;
                            }
                            else                                                        // X Y Y
                            {
                                if ( ( pLineDash->DashLen > nDistance ) || ( pLineDash->DotLen > nDistance ) )
                                    eDash = ESCHER_LineLongDashDotGEL;
                                else
                                    eDash = ESCHER_LineDashDotGEL;

                            }
                        }
                        AddOpt( ESCHER_Prop_lineDashing, eDash );
                    }
                }
                case css::drawing::LineStyle_SOLID :
                default:
                {
                    AddOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );
                }
                break;
            }
        }
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "LineColor" ) )
        {
            sal_uInt32 nLineColor = ImplGetColor( *static_cast<sal_uInt32 const *>(aAny.getValue()) );
            AddOpt( ESCHER_Prop_lineColor, nLineColor );
            AddOpt( ESCHER_Prop_lineBackColor, nLineColor ^ 0xffffff );
        }
    }

    sal_uInt32 nLineSize = ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "LineWidth" ) )
        ? *static_cast<sal_uInt32 const *>(aAny.getValue()) : 0;
    if ( nLineSize > 1 )
        AddOpt( ESCHER_Prop_lineWidth, nLineSize * 360 );       // 100TH MM -> PT , 1PT = 12700 EMU

    ESCHER_LineJoin eLineJoin = ESCHER_LineJoinMiter;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "LineJoint", true ) )
    {
        css::drawing::LineJoint eLJ;
        if ( aAny >>= eLJ )
        {
            switch ( eLJ )
            {
                case css::drawing::LineJoint_NONE :
                case css::drawing::LineJoint_MIDDLE :
                case css::drawing::LineJoint_BEVEL :
                    eLineJoin = ESCHER_LineJoinBevel;
                break;
                default:
                case css::drawing::LineJoint_MITER :
                    eLineJoin = ESCHER_LineJoinMiter;
                break;
                case css::drawing::LineJoint_ROUND :
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

static Size lcl_SizeToEmu(Size aPrefSize, MapMode aPrefMapMode)
{
    Size aRetSize;
    if (aPrefMapMode == MAP_PIXEL)
        aRetSize = Application::GetDefaultDevice()->PixelToLogic( aPrefSize, MAP_100TH_MM );
    else
        aRetSize = OutputDevice::LogicToLogic( aPrefSize, aPrefMapMode, MAP_100TH_MM );
    return aRetSize;
}

void EscherPropertyContainer::ImplCreateGraphicAttributes( const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
                                                            sal_uInt32 nBlibId, bool bCreateCroppingAttributes )
{
    css::uno::Any aAny;

    sal_uInt32 nPicFlags = 0;
    css::drawing::ColorMode eColorMode( css::drawing::ColorMode_STANDARD );
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

    if ( eColorMode == css::drawing::ColorMode_WATERMARK )
    {
        eColorMode = css::drawing::ColorMode_STANDARD;
        nLuminance += 70;
        if ( nLuminance > 100 )
            nLuminance = 100;
        nContrast -= 70;
        if ( nContrast < -100 )
            nContrast = -100;
    }
    if ( eColorMode == css::drawing::ColorMode_GREYS )
        nPicFlags |= 0x40004;
    else if ( eColorMode == css::drawing::ColorMode_MONO )
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

    if ( bCreateCroppingAttributes && pGraphicProvider )
    {
        Size    aPrefSize;
        MapMode aPrefMapMode;
        if ( pGraphicProvider->GetPrefSize( nBlibId, aPrefSize, aPrefMapMode ) )
        {
            Size aCropSize(lcl_SizeToEmu(aPrefSize, aPrefMapMode));
            if ( aCropSize.Width() && aCropSize.Height() )
            {
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "GraphicCrop" ) )
                {
                    css::text::GraphicCrop aGraphCrop;
                    if ( aAny >>= aGraphCrop )
                    {
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
                }
            }
        }
    }
}

bool EscherPropertyContainer::CreateShapeProperties( const css::uno::Reference< css::drawing::XShape > & rXShape )
{
    uno::Reference< beans::XPropertySet > aXPropSet( rXShape, uno::UNO_QUERY );
    if ( aXPropSet.is() )
    {
        bool bVal = false;
        css::uno::Any aAny;
        sal_uInt32 nShapeAttr = 0;
        if (EscherPropertyValueHelper::GetPropertyValue(aAny, aXPropSet, "Visible", true) && (aAny >>= bVal))
        {
            if ( !bVal )
                nShapeAttr |= 0x20002;  // set fHidden = true
        }
        if (EscherPropertyValueHelper::GetPropertyValue(aAny, aXPropSet, "Printable", true) && (aAny >>= bVal))
        {
            if ( !bVal )
                nShapeAttr |= 0x10000;  // set fPrint = false;
        }
        if ( nShapeAttr )
            AddOpt( ESCHER_Prop_fPrint, nShapeAttr );
    }
    return true;
}

bool EscherPropertyContainer::CreateOLEGraphicProperties(
    const css::uno::Reference< css::drawing::XShape > & rXShape )
{
    bool    bRetValue = false;

    if ( rXShape.is() )
    {
        SdrObject* pSdrOLE2( GetSdrObjectFromXShape( rXShape ) );   // SJ: leaving unoapi, because currently there is
        if ( pSdrOLE2 && nullptr != dynamic_cast<const SdrOle2Obj* > (pSdrOLE2) )              // no access to the native graphic object
        {
            const Graphic* pGraphic = static_cast<SdrOle2Obj*>(pSdrOLE2)->GetGraphic();
            if ( pGraphic )
            {
                GraphicObject aGraphicObject( *pGraphic );
                bRetValue = CreateGraphicProperties( rXShape,aGraphicObject );
            }
        }
    }
    return bRetValue;
}

bool EscherPropertyContainer::CreateGraphicProperties( const css::uno::Reference< css::drawing::XShape > & rXShape, const GraphicObject& rGraphicObj )
{
    bool    bRetValue = false;
    OString aUniqueId( rGraphicObj.GetUniqueID() );
    if ( !aUniqueId.isEmpty() )
    {
        AddOpt( ESCHER_Prop_fillType, ESCHER_FillPicture );
        uno::Reference< beans::XPropertySet > aXPropSet( rXShape, uno::UNO_QUERY );

        if ( pGraphicProvider && pPicOutStrm && pShapeBoundRect && aXPropSet.is() )
        {
            css::uno::Any aAny;
            std::unique_ptr< css::awt::Rectangle> pVisArea;
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, "VisibleArea" ) )
            {
                pVisArea.reset(new css::awt::Rectangle);
                aAny >>= (*pVisArea);
            }
            Rectangle aRect( Point( 0, 0 ), pShapeBoundRect->GetSize() );
            sal_uInt32 nBlibId = pGraphicProvider->GetBlibID( *pPicOutStrm, aUniqueId, aRect, pVisArea.get() );
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

bool EscherPropertyContainer::CreateMediaGraphicProperties(
    const css::uno::Reference< css::drawing::XShape > & rXShape )
{
    bool    bRetValue = false;
    if ( rXShape.is() )
    {
        SdrObject* pSdrMedia( GetSdrObjectFromXShape( rXShape ) );  // SJ: leaving unoapi, because currently there is
        if ( dynamic_cast<const SdrMediaObj* >(pSdrMedia) !=  nullptr )               // no access to the native graphic object
        {
            GraphicObject aGraphicObject( static_cast<SdrMediaObj*>(pSdrMedia)->getSnapshot() );
            bRetValue = CreateGraphicProperties( rXShape, aGraphicObject );
        }
    }
    return bRetValue;
}

bool EscherPropertyContainer::ImplCreateEmbeddedBmp( const OString& rUniqueId )
{
    if( !rUniqueId.isEmpty() )
    {
        EscherGraphicProvider aProvider;
        SvMemoryStream aMemStrm;
        Rectangle aRect;
        if ( aProvider.GetBlibID( aMemStrm, rUniqueId, aRect ) )
        {
            // grab BLIP from stream and insert directly as complex property
            // ownership of stream memory goes to complex property
            aMemStrm.ObjectOwnsMemory( false );
            sal_uInt8 const * pBuf = static_cast<sal_uInt8 const *>(aMemStrm.GetData());
            sal_uInt32 nSize = aMemStrm.Seek( STREAM_SEEK_TO_END );
            AddOpt( ESCHER_Prop_fillBlip, true, nSize, const_cast<sal_uInt8 *>(pBuf), nSize );
            return true;
        }
    }
    return false;
}

bool EscherPropertyContainer::CreateEmbeddedBitmapProperties(
    const OUString& rBitmapUrl, css::drawing::BitmapMode eBitmapMode )
{
    bool bRetValue = false;
    OUString aVndUrl( "vnd.sun.star.GraphicObject:" );
    OUString aBmpUrl( rBitmapUrl );
    sal_Int32 nIndex = aBmpUrl.indexOf( aVndUrl );
    if( nIndex != -1 )
    {
        nIndex += aVndUrl.getLength();
        if( aBmpUrl.getLength() > nIndex )
        {
            OString aUniqueId(OUStringToOString(aBmpUrl.copy(nIndex, aBmpUrl.getLength() - nIndex), RTL_TEXTENCODING_UTF8));
            bRetValue = ImplCreateEmbeddedBmp( aUniqueId );
            if( bRetValue )
            {
                // bitmap mode property
                bool bRepeat = eBitmapMode == css::drawing::BitmapMode_REPEAT;
                AddOpt( ESCHER_Prop_fillType, bRepeat ? ESCHER_FillTexture : ESCHER_FillPicture );
            }
        }
    }
    return bRetValue;
}


namespace {

GraphicObject lclDrawHatch( const css::drawing::Hatch& rHatch, const Color& rBackColor, bool bFillBackground, const Rectangle& rRect )
{
    // #i121183# For hatch, do no longer create a bitmap with the fixed size of 28x28 pixels. Also
    // do not create a bitmap in page size, that would explode file sizes (and have no good quality).
    // Better use a MetaFile graphic in page size; thus we have good quality due to vector format and
    // no bit file sizes.
    ScopedVclPtrInstance< VirtualDevice > pVDev;
    GDIMetaFile aMtf;

    pVDev->SetOutputSizePixel(Size(2, 2));
    pVDev->EnableOutput(false);
    pVDev->SetMapMode(MapMode(MAP_100TH_MM));
    aMtf.Clear();
    aMtf.Record(pVDev);
    pVDev->SetLineColor();
    pVDev->SetFillColor(bFillBackground ? rBackColor : Color(COL_TRANSPARENT));
    pVDev->DrawRect(rRect);
    pVDev->DrawHatch(tools::PolyPolygon(rRect), Hatch((HatchStyle)rHatch.Style, Color(rHatch.Color), rHatch.Distance, (sal_uInt16)rHatch.Angle));
    aMtf.Stop();
    aMtf.WindStart();
    aMtf.SetPrefMapMode(MapMode(MAP_100TH_MM));
    aMtf.SetPrefSize(rRect.GetSize());

    return GraphicObject(Graphic(aMtf));
}

} // namespace


bool EscherPropertyContainer::CreateEmbeddedHatchProperties( const css::drawing::Hatch& rHatch, const Color& rBackColor, bool bFillBackground )
{
    const Rectangle aRect(pShapeBoundRect ? *pShapeBoundRect : Rectangle(Point(0,0), Size(28000, 21000)));
    GraphicObject aGraphicObject = lclDrawHatch( rHatch, rBackColor, bFillBackground, aRect );
    OString aUniqueId = aGraphicObject.GetUniqueID();
    bool bRetValue = ImplCreateEmbeddedBmp( aUniqueId );
    if ( bRetValue )
        AddOpt( ESCHER_Prop_fillType, ESCHER_FillTexture );
    return bRetValue;
}


bool EscherPropertyContainer::CreateGraphicProperties(
    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
        const OUString& rSource, const bool bCreateFillBitmap, const bool bCreateCroppingAttributes,
            const bool bFillBitmapModeAllowed, const bool bOOxmlExport )
{
    bool        bRetValue = false;
    bool        bCreateFillStyles = false;

    std::unique_ptr<GraphicAttr> pGraphicAttr;
    GraphicObject   aGraphicObject;
    OUString        aGraphicUrl;
    OString         aUniqueId;

    css::drawing::BitmapMode   eBitmapMode( css::drawing::BitmapMode_NO_REPEAT );
    css::uno::Any aAny;

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

        sal_uInt16 nAngle = 0;
        if ( rSource == "MetaFile" )
        {
            css::uno::Sequence<sal_Int8> aSeq = *static_cast<css::uno::Sequence<sal_Int8> const *>(aAny.getValue());
            const sal_Int8*    pAry = aSeq.getArray();
            sal_uInt32          nAryLen = aSeq.getLength();

            // the metafile is already rotated
            bRotate = false;

            if ( pAry && nAryLen )
            {
                Graphic         aGraphic;
                SvMemoryStream  aTemp( const_cast<sal_Int8 *>(pAry), nAryLen, StreamMode::READ );
                sal_uInt32 nErrCode = GraphicConverter::Import( aTemp, aGraphic, ConvertDataFormat::WMF );
                if ( nErrCode == ERRCODE_NONE )
                {
                    aGraphicObject = aGraphic;
                    aUniqueId = aGraphicObject.GetUniqueID();
                    bIsGraphicMtf = aGraphicObject.GetType() == GRAPHIC_GDIMETAFILE;
                }
            }
        }
        else if ( rSource == "Bitmap" )
        {
            css::uno::Reference< css::awt::XBitmap >xBitmap(
                aAny, css::uno::UNO_QUERY);
            if (xBitmap.is())
            {
                css::uno::Reference< css::awt::XBitmap > xBmp;
                if ( aAny >>= xBmp )
                {
                    BitmapEx    aBitmapEx( VCLUnoHelper::GetBitmap( xBmp ) );
                    Graphic     aGraphic( aBitmapEx );
                    aGraphicObject = aGraphic;
                    aUniqueId = aGraphicObject.GetUniqueID();
                    bIsGraphicMtf = aGraphicObject.GetType() == GRAPHIC_GDIMETAFILE;
                }
            }
        }
        else if ( rSource == "FillBitmapURL" )
        {
            aGraphicUrl = *static_cast<OUString const *>(aAny.getValue());
        }
        else if ( rSource == "GraphicURL" )
        {
            aGraphicUrl = *static_cast<OUString const *>(aAny.getValue());
            bCreateFillStyles = true;
        }
        else if ( rSource == "FillHatch" )
        {
            css::drawing::Hatch aHatch;
            if ( aAny >>= aHatch )
            {
                Color aBackColor;
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "FillColor" ) )
                {
                    aBackColor = ImplGetColor( *static_cast<sal_uInt32 const *>(aAny.getValue()), false );
                }
                bool bFillBackground = false;
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "FillBackground", true ) )
                {
                    aAny >>= bFillBackground;
                }

                const Rectangle aRect(Point(0, 0), pShapeBoundRect ? pShapeBoundRect->GetSize() : Size(28000, 21000));
                aGraphicObject = lclDrawHatch( aHatch, aBackColor, bFillBackground, aRect );
                aUniqueId = aGraphicObject.GetUniqueID();
                eBitmapMode = css::drawing::BitmapMode_REPEAT;
                bIsGraphicMtf = aGraphicObject.GetType() == GRAPHIC_GDIMETAFILE;
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
                ? (sal_uInt16)( ( *static_cast<sal_Int32 const *>(aAny.getValue()) ) + 5 ) / 10
                : 0;
        }

        if ( aGraphicUrl.getLength() )
        {
            OUString aVndUrl( "vnd.sun.star.GraphicObject:" );
            sal_Int32 nIndex = aGraphicUrl.indexOf( aVndUrl );
            if ( nIndex != -1 )
            {
                nIndex = nIndex + aVndUrl.getLength();
                if ( aGraphicUrl.getLength() > nIndex  )
                    aUniqueId = OUStringToOString(aGraphicUrl.copy(nIndex, aGraphicUrl.getLength() - nIndex), RTL_TEXTENCODING_UTF8);
            }
            else
            {
                // externally, linked graphic? convert to embedded
                // one, if transformations are needed. this is because
                // everything < msoxp cannot even handle rotated
                // bitmaps.
                // And check whether the graphic link target is
                // actually supported by mso.
                INetURLObject   aTmp( aGraphicUrl );
                GraphicDescriptor aDescriptor(aTmp);
                aDescriptor.Detect();
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
                        aTmp.GetMainURL( INetURLObject::NO_DECODE ), StreamMode::READ ));
                    if ( pIn )
                    {
                        Graphic aGraphic;
                        sal_uInt32 nErrCode = GraphicConverter::Import( *pIn, aGraphic );

                        if ( nErrCode == ERRCODE_NONE )
                        {
                            // no
                            aGraphicObject = aGraphic;
                            aUniqueId = aGraphicObject.GetUniqueID();
                        }
                        // else: simply keep the graphic link
                    }
                }
                if ( aUniqueId.isEmpty() )
                {
                    if ( pGraphicProvider )
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
            }
        }

        if ( aGraphicUrl.getLength() || !aUniqueId.isEmpty() )
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
                    pGraphicAttr->SetTransparency((nTransparency * 255) / 100);
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
                AddOpt( ESCHER_Prop_Rotation, ( ( ((sal_Int32)nAngle << 16 ) / 10 ) + 0x8000 ) &~ 0xffff );
            }

            if ( eBitmapMode == css::drawing::BitmapMode_REPEAT )
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

            if ( !aUniqueId.isEmpty() )
            {
                // write out embedded graphic
                if ( pGraphicProvider && pPicOutStrm && pShapeBoundRect )
                {
                    Rectangle aRect( Point( 0, 0 ), pShapeBoundRect->GetSize() );
                    const sal_uInt32 nBlibId(pGraphicProvider->GetBlibID(*pPicOutStrm, aUniqueId, aRect, nullptr, pGraphicAttr.get()));

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
                    Rectangle aRect;

                    if ( aProvider.GetBlibID( aMemStrm, aUniqueId, aRect, nullptr, pGraphicAttr.get(), bOOxmlExport ) )
                    {
                        // grab BLIP from stream and insert directly as complex property
                        // ownership of stream memory goes to complex property
                        aMemStrm.ObjectOwnsMemory( false );
                        sal_uInt8 const * pBuf = static_cast<sal_uInt8 const *>(aMemStrm.GetData());
                        sal_uInt32 nSize = aMemStrm.Seek( STREAM_SEEK_TO_END );
                        AddOpt( ESCHER_Prop_fillBlip, true, nSize, const_cast<sal_uInt8 *>(pBuf), nSize );
                        bRetValue = true;
                    }
                }
            }
            // write out link to graphic
            else
            {
                OSL_ASSERT(aGraphicUrl.getLength());

                AddOpt( ESCHER_Prop_pibName, aGraphicUrl );
                sal_uInt32  nPibFlags=0;
                GetOpt( ESCHER_Prop_pibFlags, nPibFlags );
                AddOpt( ESCHER_Prop_pibFlags,
                        ESCHER_BlipFlagLinkToFile|ESCHER_BlipFlagFile|ESCHER_BlipFlagDoNotSave | nPibFlags );
            }
        }
    }
    pGraphicAttr.reset();
    if ( bCreateFillStyles )
        CreateFillProperties( rXPropSet, true );

    return bRetValue;
}

tools::PolyPolygon EscherPropertyContainer::GetPolyPolygon( const css::uno::Reference< css::drawing::XShape > & rXShape )
{
    tools::PolyPolygon aRetPolyPoly;
    css::uno::Reference< css::beans::XPropertySet > aXPropSet;
    css::uno::Any aAny( rXShape->queryInterface(
        cppu::UnoType<css::beans::XPropertySet>::get()));

    OUString sPolyPolygonBezier ( "PolyPolygonBezier" );
    OUString sPolyPolygon       ( "PolyPolygon" );
    OUString sPolygon           ( "Polygon" );

    if ( aAny >>= aXPropSet )
    {
        bool bHasProperty = EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sPolyPolygonBezier, true );
        if ( !bHasProperty )
            bHasProperty = EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sPolyPolygon, true );
        if ( !bHasProperty )
            bHasProperty = EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sPolygon, true );
        if ( bHasProperty )
            aRetPolyPoly = GetPolyPolygon( aAny );
    }
    return aRetPolyPoly;
}

tools::PolyPolygon EscherPropertyContainer::GetPolyPolygon( const css::uno::Any& rAny )
{
    bool bNoError = true;

    tools::Polygon aPolygon;
    tools::PolyPolygon aPolyPolygon;

    if ( rAny.getValueType() == cppu::UnoType<css::drawing::PolyPolygonBezierCoords>::get())
    {
        css::drawing::PolyPolygonBezierCoords const * pSourcePolyPolygon
            = static_cast<css::drawing::PolyPolygonBezierCoords const *>(rAny.getValue());
        sal_uInt16 nOuterSequenceCount = (sal_uInt16)pSourcePolyPolygon->Coordinates.getLength();

        // get pointer of inner sequences
        css::drawing::PointSequence const * pOuterSequence = pSourcePolyPolygon->Coordinates.getConstArray();
        css::drawing::FlagSequence const *  pOuterFlags = pSourcePolyPolygon->Flags.getConstArray();

        bNoError = pOuterSequence && pOuterFlags;
        if ( bNoError )
        {
            sal_uInt16  a, b, nInnerSequenceCount;
            css::awt::Point const * pArray;

            // this will be a polygon set
            for ( a = 0; a < nOuterSequenceCount; a++ )
            {
                css::drawing::PointSequence const * pInnerSequence = pOuterSequence++;
                css::drawing::FlagSequence const *  pInnerFlags = pOuterFlags++;

                bNoError = pInnerSequence && pInnerFlags;
                if  ( bNoError )
                {
                    // get pointer to arrays
                    pArray = pInnerSequence->getConstArray();
                    css::drawing::PolygonFlags const * pFlags = pInnerFlags->getConstArray();

                    if ( pArray && pFlags )
                    {
                        nInnerSequenceCount = (sal_uInt16)pInnerSequence->getLength();
                        aPolygon = tools::Polygon( nInnerSequenceCount );
                        for( b = 0; b < nInnerSequenceCount; b++)
                        {
                            css::drawing::PolygonFlags ePolyFlags = *pFlags++;
                            css::awt::Point aPoint( *(pArray++) );
                            aPolygon[ b ] = Point( aPoint.X, aPoint.Y );
                            aPolygon.SetFlags( b, static_cast<PolyFlags>(ePolyFlags) );

                            if ( ePolyFlags == css::drawing::PolygonFlags_CONTROL )
                                continue;
                        }
                        aPolyPolygon.Insert( aPolygon );
                    }
                }
            }
        }
    }
    else if ( rAny.getValueType() == cppu::UnoType<css::drawing::PointSequenceSequence>::get() )
    {
        css::drawing::PointSequenceSequence const * pSourcePolyPolygon
            = static_cast<css::drawing::PointSequenceSequence const *>(rAny.getValue());
        sal_uInt16 nOuterSequenceCount = (sal_uInt16)pSourcePolyPolygon->getLength();

        // get pointer to inner sequences
        css::drawing::PointSequence const * pOuterSequence = pSourcePolyPolygon->getConstArray();
        bNoError = pOuterSequence != nullptr;
        if ( bNoError )
        {
            sal_uInt16 a, b, nInnerSequenceCount;

            // this will be a polygon set
            for( a = 0; a < nOuterSequenceCount; a++ )
            {
                css::drawing::PointSequence const * pInnerSequence = pOuterSequence++;
                bNoError = pInnerSequence != nullptr;
                if ( bNoError )
                {
                    // get pointer to arrays
                    css::awt::Point const * pArray =
                          pInnerSequence->getConstArray();
                    if ( pArray != nullptr )
                    {
                        nInnerSequenceCount = (sal_uInt16)pInnerSequence->getLength();
                        aPolygon = tools::Polygon( nInnerSequenceCount );
                        for( b = 0; b < nInnerSequenceCount; b++)
                        {
                            aPolygon[ b ] = Point( pArray->X, pArray->Y );
                            pArray++;
                        }
                        aPolyPolygon.Insert( aPolygon );
                    }
                }
            }
        }
    }
    else if ( rAny.getValueType() == cppu::UnoType<css::drawing::PointSequence>::get() )
    {
        css::drawing::PointSequence const * pInnerSequence =
            static_cast<css::drawing::PointSequence const *>(rAny.getValue());

        bNoError = pInnerSequence != nullptr;
        if ( bNoError )
        {
            sal_uInt16 a, nInnerSequenceCount;

            // get pointer to arrays
            css::awt::Point const * pArray = pInnerSequence->getConstArray();
            if ( pArray != nullptr )
            {
                nInnerSequenceCount = (sal_uInt16)pInnerSequence->getLength();
                aPolygon = tools::Polygon( nInnerSequenceCount );
                for( a = 0; a < nInnerSequenceCount; a++)
                {
                    aPolygon[ a ] = Point( pArray->X, pArray->Y );
                    pArray++;
                }
                aPolyPolygon.Insert( aPolygon );
            }
        }
    }
    return aPolyPolygon;
}

bool EscherPropertyContainer::CreatePolygonProperties(
    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
          sal_uInt32 nFlags,
          bool bBezier,
          css::awt::Rectangle& rGeoRect,
    tools::Polygon* pPolygon )
{
    static const char sPolyPolygonBezier [] = "PolyPolygonBezier";
    static const char sPolyPolygon       [] = "PolyPolygon";

    bool    bRetValue = true;
    bool    bLine = ( nFlags & ESCHER_CREATEPOLYGON_LINE ) != 0;

    tools::PolyPolygon aPolyPolygon;

    if ( pPolygon )
        aPolyPolygon.Insert( *pPolygon );
    else
    {
        css::uno::Any aAny;
        bRetValue = EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                        ( bBezier ) ? OUString(sPolyPolygonBezier) : OUString(sPolyPolygon), true );
        if ( bRetValue )
        {
            aPolyPolygon = GetPolyPolygon( aAny );
            bRetValue = aPolyPolygon.Count() != 0;
        }
    }
    if ( bRetValue )
    {
        if ( bLine )
        {
            if ( ( aPolyPolygon.Count() == 1 ) && ( aPolyPolygon[ 0 ].GetSize() == 2 ) )
            {
                const tools::Polygon& rPoly = aPolyPolygon[ 0 ];
                rGeoRect = css::awt::Rectangle(
                    rPoly[ 0 ].X(),
                        rPoly[ 0 ].Y(),
                            rPoly[ 1 ].X() - rPoly[ 0 ].X(),
                                rPoly[ 1 ].Y() - rPoly[ 0 ].Y() );
            }
            else
                bRetValue = false;
        }
        else
        {
            tools::Polygon aPolygon;

            sal_uInt16 nPolyCount = aPolyPolygon.Count();
            sal_uInt32 nTotalPoints(0), nTotalBezPoints(0);
            Rectangle aRect( aPolyPolygon.GetBoundRect() );
            rGeoRect = css::awt::Rectangle( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight() );

            for (sal_uInt16 i = 0; i < nPolyCount; ++i)
            {
                sal_uInt16 k = aPolyPolygon[ i ].GetSize();
                nTotalPoints += k;
                for (sal_uInt16 j = 0; j < k; ++j)
                {
                    if ( aPolyPolygon[ i ].GetFlags( j ) != POLY_CONTROL )
                        nTotalBezPoints++;
                }
            }
            sal_uInt32 nVerticesBufSize = ( nTotalPoints << 2 ) + 6;
            sal_uInt8* pVerticesBuf = new sal_uInt8[ nVerticesBufSize ];


            sal_uInt32 nSegmentBufSize = ( ( nTotalBezPoints << 2 ) + 8 );
            if ( nPolyCount > 1 )
                nSegmentBufSize += ( nPolyCount << 1 );
            sal_uInt8* pSegmentBuf = new sal_uInt8[ nSegmentBufSize ];

            sal_uInt8* pPtr = pVerticesBuf;
            *pPtr++ = (sal_uInt8)( nTotalPoints );                    // Little endian
            *pPtr++ = (sal_uInt8)( nTotalPoints >> 8 );
            *pPtr++ = (sal_uInt8)( nTotalPoints );
            *pPtr++ = (sal_uInt8)( nTotalPoints >> 8 );
            *pPtr++ = (sal_uInt8)0xf0;
            *pPtr++ = (sal_uInt8)0xff;

            for (sal_uInt16 j = 0; j < nPolyCount; ++j)
            {
                aPolygon = aPolyPolygon[ j ];
                sal_uInt16 nPoints = aPolygon.GetSize();
                for (sal_uInt16 i = 0; i < nPoints; ++i)             // write points from polygon to buffer
                {
                    Point aPoint = aPolygon[ i ];
                    aPoint.X() -= rGeoRect.X;
                    aPoint.Y() -= rGeoRect.Y;

                    *pPtr++ = (sal_uInt8)( aPoint.X() );
                    *pPtr++ = (sal_uInt8)( aPoint.X() >> 8 );
                    *pPtr++ = (sal_uInt8)( aPoint.Y() );
                    *pPtr++ = (sal_uInt8)( aPoint.Y() >> 8 );
                }
            }

            pPtr = pSegmentBuf;
            *pPtr++ = (sal_uInt8)( ( nSegmentBufSize - 6 ) >> 1 );
            *pPtr++ = (sal_uInt8)( ( nSegmentBufSize - 6 ) >> 9 );
            *pPtr++ = (sal_uInt8)( ( nSegmentBufSize - 6 ) >> 1 );
            *pPtr++ = (sal_uInt8)( ( nSegmentBufSize - 6 ) >> 9 );
            *pPtr++ = (sal_uInt8)2;
            *pPtr++ = (sal_uInt8)0;

            for (sal_uInt16 j = 0; j < nPolyCount; ++j)
            {
                *pPtr++ = 0x0;          // Polygon start
                *pPtr++ = 0x40;
                aPolygon = aPolyPolygon[ j ];
                sal_uInt16 nPoints = aPolygon.GetSize();
                for (sal_uInt16 i = 0; i < nPoints; ++i)         // write Polyflags to Buffer
                {
                    *pPtr++ = 0;
                    if ( bBezier )
                        *pPtr++ = 0xb3;
                    else
                        *pPtr++ = 0xac;
                    if ( ( i + 1 ) != nPoints )
                    {
                        *pPtr++ = 1;
                        if ( aPolygon.GetFlags( i + 1 ) == POLY_CONTROL )
                        {
                            *pPtr++ = 0x20;
                            i += 2;
                        }
                        else
                            *pPtr++ = 0;
                    }
                }
                if ( nPolyCount > 1 )
                {
                    *pPtr++ = 1;                        // end of polygon
                    *pPtr++ = 0x60;
                }
            }
            *pPtr++ = 0;
            *pPtr++ = 0x80;

            AddOpt( ESCHER_Prop_geoRight, rGeoRect.Width );
            AddOpt( ESCHER_Prop_geoBottom, rGeoRect.Height );

            AddOpt( ESCHER_Prop_shapePath, ESCHER_ShapeComplex );
            AddOpt( ESCHER_Prop_pVertices, true, nVerticesBufSize - 6, pVerticesBuf, nVerticesBufSize );
            AddOpt( ESCHER_Prop_pSegmentInfo, true, nSegmentBufSize, pSegmentBuf, nSegmentBufSize );
        }
    }
    return bRetValue;
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
sal_Int32 lcl_GetAdjustValueCount( const XPolygon& rPoly )
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
sal_Int32 lcl_GetConnectorAdjustValue ( const XPolygon& rPoly, sal_uInt16 nIndex )
{
    sal_uInt16 k =  rPoly.GetSize();
    OSL_ASSERT ( k >= ( 3 + nIndex ) );

    Point aPt;
    Point aStart = rPoly[0];
    Point aEnd = rPoly[k-1];
    if ( aEnd.Y() == aStart.Y() )
        aEnd.Y() = aStart.Y() +4;
    if ( aEnd.X() == aStart.X() )
        aEnd.X() = aStart.X() +4;

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


void lcl_Rotate(sal_Int32 nAngle, Point center, Point& pt)
{
    while ( nAngle<0)
        nAngle +=36000;
    while (nAngle>=36000)
        nAngle -=36000;

    int cs, sn;
    switch (nAngle)
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
        break;
    }
    sal_Int32 x0 =pt.X()-center.X();
    sal_Int32 y0 =pt.Y()-center.Y();
    pt.X()=center.X()+ x0*cs-y0*sn;
    pt.Y()=center.Y()+ y0*cs+x0*sn;
}
/*
 FlipV defines that the shape will be flipped vertically about the center of its bounding box.
Generally, draw the connector from top to bottom, from left to right when meet the adjust value,
but when (X1>X2 or Y1>Y2),the draw director must be reverse, FlipV or FlipH should be set to true.
*/
bool lcl_GetAngle(tools::Polygon &rPoly,sal_uInt16& rShapeFlags,sal_Int32& nAngle )
{
    Point aStart = rPoly[0];
    Point aEnd = rPoly[rPoly.GetSize()-1];
    nAngle = ( rPoly[1].X() == aStart.X() ) ? 9000: 0 ;
    Point p1(aStart.X(),aStart.Y());
    Point p2(aEnd.X(),aEnd.Y());
    if ( nAngle )
    {
        Point center((aEnd.X()+aStart.X())>>1,(aEnd.Y()+aStart.Y())>>1);
        lcl_Rotate(-nAngle, center,p1);
        lcl_Rotate(-nAngle, center,p2);
    }
    if (  p1.X() > p2.X() )
    {
        if ( nAngle )
            rShapeFlags |= SHAPEFLAG_FLIPV;
        else
            rShapeFlags |= SHAPEFLAG_FLIPH;

    }
    if (  p1.Y() > p2.Y()  )
    {
        if ( nAngle )
            rShapeFlags |= SHAPEFLAG_FLIPH;
        else
            rShapeFlags |= SHAPEFLAG_FLIPV;
    }

    if ( (rShapeFlags&SHAPEFLAG_FLIPH) && (rShapeFlags&SHAPEFLAG_FLIPV) )
    {
        rShapeFlags  &= ~( SHAPEFLAG_FLIPH | SHAPEFLAG_FLIPV );
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
    const css::uno::Reference< css::drawing::XShape > & rXShape,
    EscherSolverContainer& rSolverContainer, css::awt::Rectangle& rGeoRect,
            sal_uInt16& rShapeType, sal_uInt16& rShapeFlags )
{
    static const char sEdgeKind            [] = "EdgeKind";
    static const char sEdgeStartPoint      [] = "EdgeStartPoint";
    static const char sEdgeEndPoint        [] = "EdgeEndPoint";
    static const char sEdgeStartConnection [] = "EdgeStartConnection";
    static const char sEdgeEndConnection   [] = "EdgeEndConnection";
    static const char sEdgePath            [] = "PolyPolygonBezier";

    bool bRetValue = false;
    rShapeType = rShapeFlags = 0;

    if ( rXShape.is() )
    {
        css::awt::Point aStartPoint, aEndPoint;
        css::uno::Reference< css::beans::XPropertySet > aXPropSet;
        css::uno::Reference< css::drawing::XShape > aShapeA, aShapeB;
        css::uno::Any aAny( rXShape->queryInterface( cppu::UnoType<css::beans::XPropertySet>::get()));
        if ( aAny >>= aXPropSet )
        {
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sEdgeKind, true ) )
            {
                css::drawing::ConnectorType eCt;
                aAny >>= eCt;
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sEdgeStartPoint ) )
                {
                    aStartPoint = *static_cast<css::awt::Point const *>(aAny.getValue());
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sEdgeEndPoint ) )
                    {
                        aEndPoint = *static_cast<css::awt::Point const *>(aAny.getValue());

                        rShapeFlags = SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT | SHAPEFLAG_CONNECTOR;
                        rGeoRect = css::awt::Rectangle( aStartPoint.X, aStartPoint.Y,
                                                            ( aEndPoint.X - aStartPoint.X ) + 1, ( aEndPoint.Y - aStartPoint.Y ) + 1 );
                        // set standard's FLIP in below code
                        if ( eCt != css::drawing::ConnectorType_STANDARD)
                        {
                            if ( rGeoRect.Height < 0 )          // justify
                            {
                                rShapeFlags |= SHAPEFLAG_FLIPV;
                                rGeoRect.Y = aEndPoint.Y;
                                rGeoRect.Height = -rGeoRect.Height;
                            }
                            if ( rGeoRect.Width < 0 )
                            {
                                rShapeFlags |= SHAPEFLAG_FLIPH;
                                rGeoRect.X = aEndPoint.X;
                                rGeoRect.Width = -rGeoRect.Width;
                            }
                        }
                        sal_uInt32 nAdjustValue1, nAdjustValue2, nAdjustValue3;
                        nAdjustValue1 = nAdjustValue2 = nAdjustValue3 = 0x2a30;

                        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sEdgeStartConnection ) )
                            aAny >>= aShapeA;
                        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sEdgeEndConnection ) )
                            aAny >>= aShapeB;
                        rSolverContainer.AddConnector( rXShape, aStartPoint, aShapeA, aEndPoint, aShapeB );
                        switch ( eCt )
                        {
                            case css::drawing::ConnectorType_CURVE :
                            {
                                rShapeType = ESCHER_ShpInst_CurvedConnector3;
                                AddOpt( ESCHER_Prop_cxstyle, ESCHER_cxstyleCurved );
                                AddOpt( ESCHER_Prop_adjustValue, nAdjustValue1 );
                                AddOpt( ESCHER_Prop_adjust2Value, -(sal_Int32)nAdjustValue2 );
                            }
                            break;

                            case css::drawing::ConnectorType_STANDARD :// Connector 2->5
                                {
                                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sEdgePath ) )
                                    {
                                        tools::PolyPolygon aPolyPolygon = GetPolyPolygon( aAny );
                                        tools::Polygon aPoly;
                                        if ( aPolyPolygon.Count() > 0 )
                                        {
                                            AddOpt( ESCHER_Prop_cxstyle, ESCHER_cxstyleBent );
                                            aPoly = aPolyPolygon[ 0 ];
                                            sal_Int32 nAdjCount = lcl_GetAdjustValueCount( aPoly );
                                            rShapeType = ( sal_uInt16 )( ESCHER_ShpInst_BentConnector2 + nAdjCount);
                                            for ( sal_Int32 i = 0 ; i < nAdjCount; ++ i)
                                                AddOpt( (sal_uInt16) ( ESCHER_Prop_adjustValue+i) , lcl_GetConnectorAdjustValue( aPoly, i ) );
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
                            case css::drawing::ConnectorType_LINE :
                            case css::drawing::ConnectorType_LINES :   // Connector 2->5
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

bool EscherPropertyContainer::CreateShadowProperties(
    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet )
{
    css::uno::Any aAny;

    bool    bHasShadow = false; // shadow is possible only if at least a fillcolor, linecolor or graphic is set
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
            if ( aAny >>= bHasShadow )
            {
                if ( bHasShadow )
                {
                    nShadowFlags |= 2;
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "ShadowColor" ) )
                        AddOpt( ESCHER_Prop_shadowColor, ImplGetColor( *static_cast<sal_uInt32 const *>(aAny.getValue()) ) );
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "ShadowXDistance" ) )
                        AddOpt( ESCHER_Prop_shadowOffsetX, *static_cast<sal_Int32 const *>(aAny.getValue()) * 360 );
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "ShadowYDistance" ) )
                        AddOpt( ESCHER_Prop_shadowOffsetY, *static_cast<sal_Int32 const *>(aAny.getValue()) * 360 );
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, "ShadowTransparence" ) )
                        AddOpt( ESCHER_Prop_shadowOpacity,  0x10000 - (((sal_uInt32)*static_cast<sal_uInt16 const *>(aAny.getValue())) * 655 ) );
                }
            }
        }
    }
    AddOpt( ESCHER_Prop_fshadowObscured, nShadowFlags );
    return bHasShadow;
}

sal_Int32 EscherPropertyContainer::GetValueForEnhancedCustomShapeParameter( const css::drawing::EnhancedCustomShapeParameter& rParameter,
                                const std::vector< sal_Int32 >& rEquationOrder, bool bAdjustTrans )
{
    sal_Int32 nValue = 0;
    if ( rParameter.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
    {
        double fValue(0.0);
        if ( rParameter.Value >>= fValue )
            nValue = (sal_Int32)fValue;
    }
    else
        rParameter.Value >>= nValue;

    switch( rParameter.Type )
    {
        case css::drawing::EnhancedCustomShapeParameterType::EQUATION :
        {
            size_t nIndex = (size_t) nValue;
            OSL_ASSERT(nIndex < rEquationOrder.size());
            if ( nIndex < rEquationOrder.size() )
            {
                nValue = (sal_uInt16)rEquationOrder[ nIndex ];
                nValue |= (sal_uInt32)0x80000000;
            }
        }
        break;
        case css::drawing::EnhancedCustomShapeParameterType::ADJUSTMENT:
        {
            if(bAdjustTrans)
            {
                sal_uInt32 nAdjustValue = 0;
                bool bGot = GetOpt((sal_uInt16)( DFF_Prop_adjustValue + nValue ), nAdjustValue);
                if(bGot) nValue = (sal_Int32)nAdjustValue;
            }
        }
        break;
        case css::drawing::EnhancedCustomShapeParameterType::NORMAL :
        default:
        break;
/* not sure if it is allowed to set following values
(but they are not yet used)
        case css::drawing::EnhancedCustomShapeParameterType::BOTTOM :
        case css::drawing::EnhancedCustomShapeParameterType::RIGHT :
        case css::drawing::EnhancedCustomShapeParameterType::TOP :
        case css::drawing::EnhancedCustomShapeParameterType::LEFT :
*/
    }
    return nValue;
}

bool GetValueForEnhancedCustomShapeHandleParameter( sal_Int32& nRetValue, const css::drawing::EnhancedCustomShapeParameter& rParameter )
{
    bool bSpecial = false;
    nRetValue = 0;
    if ( rParameter.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
    {
        double fValue(0.0);
        if ( rParameter.Value >>= fValue )
            nRetValue = (sal_Int32)fValue;
    }
    else
        rParameter.Value >>= nRetValue;

    switch( rParameter.Type )
    {
        case css::drawing::EnhancedCustomShapeParameterType::EQUATION :
        {
            nRetValue += 3;
            bSpecial = true;
        }
        break;
        case css::drawing::EnhancedCustomShapeParameterType::ADJUSTMENT :
        {
            nRetValue += 0x100;
            bSpecial = true;
        }
        break;
        case css::drawing::EnhancedCustomShapeParameterType::TOP :
        case css::drawing::EnhancedCustomShapeParameterType::LEFT :
        {
            nRetValue = 0;
            bSpecial = true;
        }
        break;
        case css::drawing::EnhancedCustomShapeParameterType::RIGHT :
        case css::drawing::EnhancedCustomShapeParameterType::BOTTOM :
        {
            nRetValue = 1;
            bSpecial = true;
        }
        break;
        case css::drawing::EnhancedCustomShapeParameterType::NORMAL :
        {

        }
        break;
    }
    return bSpecial;
}

void ConvertEnhancedCustomShapeEquation( SdrObjCustomShape* pCustoShape,
        std::vector< EnhancedCustomShapeEquation >& rEquations, std::vector< sal_Int32 >& rEquationOrder )
{
    if ( pCustoShape )
    {
        uno::Sequence< OUString > sEquationSource;
        const SdrCustomShapeGeometryItem& rGeometryItem = static_cast<const SdrCustomShapeGeometryItem&>(
            pCustoShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
        const uno::Any* pAny = rGeometryItem.GetPropertyValueByName( "Equations" );
        if ( pAny )
            *pAny >>= sEquationSource;
        sal_Int32 nEquationSourceCount = sEquationSource.getLength();
        if ( nEquationSourceCount && (nEquationSourceCount <= 128) )
        {
            sal_Int32 i;
            for ( i = 0; i < nEquationSourceCount; i++ )
            {
                EnhancedCustomShape2d aCustoShape2d( pCustoShape );
                try
                {
                    std::shared_ptr< EnhancedCustomShape::ExpressionNode > aExpressNode(
                        EnhancedCustomShape::FunctionParser::parseFunction( sEquationSource[ i ], aCustoShape2d ) );
                    css::drawing::EnhancedCustomShapeParameter aPara( aExpressNode->fillNode( rEquations, nullptr, 0 ) );
                    if ( aPara.Type != css::drawing::EnhancedCustomShapeParameterType::EQUATION )
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
                    aEquation.nOperation = 0;                   // not catched on linux platform
                    aEquation.nPara[ 0 ] = 1;
                    rEquations.push_back( aEquation );
                }
                rEquationOrder.push_back( rEquations.size() - 1 );
            }
            // now updating our old equation indices, they are marked with a bit in the hiword of nOperation
            std::vector< EnhancedCustomShapeEquation >::iterator aIter( rEquations.begin() );
            std::vector< EnhancedCustomShapeEquation >::iterator aEnd ( rEquations.end() );
            while( aIter != aEnd )
            {
                sal_uInt32 nMask = 0x20000000;
                for( i = 0; i < 3; i++ )
                {
                    if ( aIter->nOperation & nMask )
                    {
                        aIter->nOperation ^= nMask;
                        const size_t nIndex(aIter->nPara[ i ] & 0x3ff);

                        // #i124661# check index access, there are cases where this is out of bound leading
                        // to errors up to crashes when executed
                        if(nIndex < rEquationOrder.size())
                        {
                            aIter->nPara[ i ] = rEquationOrder[ nIndex ] | 0x400;
                        }
                        else
                        {
                            OSL_ENSURE(false, "Attempted out of bound access to rEquationOrder of CustomShape (!)");
                        }
                    }
                    nMask <<= 1;
                }
                ++aIter;
            }
        }
    }
}

bool EscherPropertyContainer::IsDefaultObject( SdrObjCustomShape* pCustoShape , const MSO_SPT eShapeType )
{
    bool bIsDefaultObject = false;
    switch(eShapeType)
    {
        // if the custom shape is not default shape of ppt, return sal_Fasle;
        case mso_sptTearDrop:
            return bIsDefaultObject;

        default:
            break;
    }

    if ( pCustoShape )
    {
    if (   pCustoShape->IsDefaultGeometry( SdrObjCustomShape::DEFAULT_EQUATIONS )
           && pCustoShape->IsDefaultGeometry( SdrObjCustomShape::DEFAULT_VIEWBOX )
           && pCustoShape->IsDefaultGeometry( SdrObjCustomShape::DEFAULT_PATH )
           && pCustoShape->IsDefaultGeometry( SdrObjCustomShape::DEFAULT_GLUEPOINTS )
           && pCustoShape->IsDefaultGeometry( SdrObjCustomShape::DEFAULT_SEGMENTS )
           && pCustoShape->IsDefaultGeometry( SdrObjCustomShape::DEFAULT_STRETCHX )
           && pCustoShape->IsDefaultGeometry( SdrObjCustomShape::DEFAULT_STRETCHY )
           && pCustoShape->IsDefaultGeometry( SdrObjCustomShape::DEFAULT_TEXTFRAMES ) )
        bIsDefaultObject = true;
    }

    return bIsDefaultObject;
}

void EscherPropertyContainer::LookForPolarHandles( const MSO_SPT eShapeType, sal_Int32& nAdjustmentsWhichNeedsToBeConverted )
{
    const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eShapeType );
    if ( pDefCustomShape && pDefCustomShape->nHandles && pDefCustomShape->pHandles )
    {
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
}

bool EscherPropertyContainer::GetAdjustmentValue( const css::drawing::EnhancedCustomShapeAdjustmentValue & rkProp, sal_Int32 nIndex, sal_Int32 nAdjustmentsWhichNeedsToBeConverted, sal_Int32& nValue )
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
        nValue = (sal_Int32)fValue;
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
    if ( aXPropSet.is() )
    {
        SdrObjCustomShape* pCustoShape = static_cast<SdrObjCustomShape*>(GetSdrObjectFromXShape( rXShape ));
        if ( !pCustoShape ) return;
        const OUString sCustomShapeGeometry( "CustomShapeGeometry"  );
        uno::Any aGeoPropSet = aXPropSet->getPropertyValue( sCustomShapeGeometry );
        uno::Sequence< beans::PropertyValue > aGeoPropSeq;
        if ( aGeoPropSet >>= aGeoPropSeq )
        {
            const OUString sViewBox            ( "ViewBox"  );
            const OUString sTextRotateAngle    ( "TextRotateAngle"  );
            const OUString sExtrusion          ( "Extrusion"  );
            const OUString sEquations          ( "Equations"  );
            const OUString sPath               ( "Path"  );
            const OUString sTextPath           ( "TextPath"  );
            const OUString sHandles            ( "Handles"  );
            const OUString sAdjustmentValues   ( "AdjustmentValues"  );

            bool bAdjustmentValuesProp = false;
            uno::Any aAdjustmentValuesProp;
            bool bPathCoordinatesProp = false;
            uno::Any aPathCoordinatesProp;

            sal_Int32 nAdjustmentsWhichNeedsToBeConverted = 0;
            uno::Sequence< beans::PropertyValues > aHandlesPropSeq;
            bool bPredefinedHandlesUsed = true;
            bool bIsDefaultObject = IsDefaultObject( pCustoShape , eShapeType);

            // convert property "Equations" into std::vector< EnhancedCustomShapeEquationEquation >
            std::vector< EnhancedCustomShapeEquation >  aEquations;
            std::vector< sal_Int32 >                    aEquationOrder;
            ConvertEnhancedCustomShapeEquation( pCustoShape, aEquations, aEquationOrder );

            sal_Int32 i, nCount = aGeoPropSeq.getLength();
            for ( i = 0; i < nCount; i++ )
            {
                const beans::PropertyValue& rProp = aGeoPropSeq[ i ];
                if ( rProp.Name.equals( sViewBox ) )
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
                else if ( rProp.Name.equals( sTextRotateAngle ) )
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
                else if ( rProp.Name.equals( sExtrusion ) )
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
                            const OUString sExtrusionBrightness            ( "Brightness"  );
                            const OUString sExtrusionDepth                 ( "Depth"  );
                            const OUString sExtrusionDiffusion             ( "Diffusion"  );
                            const OUString sExtrusionNumberOfLineSegments  ( "NumberOfLineSegments"  );
                            const OUString sExtrusionLightFace             ( "LightFace"  );
                            const OUString sExtrusionFirstLightHarsh       ( "FirstLightHarsh"  );
                            const OUString sExtrusionSecondLightHarsh      ( "SecondLightHarsh"  );
                            const OUString sExtrusionFirstLightLevel       ( "FirstLightLevel"  );
                            const OUString sExtrusionSecondLightLevel      ( "SecondLightLevel"  );
                            const OUString sExtrusionFirstLightDirection   ( "FirstLightDirection"  );
                            const OUString sExtrusionSecondLightDirection  ( "SecondLightDirection"  );
                            const OUString sExtrusionMetal                 ( "Metal"  );
                            const OUString sExtrusionShadeMode             ( "ShadeMode"  );
                            const OUString sExtrusionRotateAngle           ( "RotateAngle"  );
                            const OUString sExtrusionRotationCenter        ( "RotationCenter"  );
                            const OUString sExtrusionShininess             ( "Shininess"  );
                            const OUString sExtrusionSkew                  ( "Skew"  );
                            const OUString sExtrusionSpecularity           ( "Specularity"  );
                            const OUString sExtrusionProjectionMode        ( "ProjectionMode"  );
                            const OUString sExtrusionViewPoint             ( "ViewPoint"  );
                            const OUString sExtrusionOrigin                ( "Origin"  );
                            const OUString sExtrusionColor                 ( "Color"  );

                            if ( rrProp.Name.equals( sExtrusion ) )
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
                            else if ( rrProp.Name.equals( sExtrusionBrightness ) )
                            {
                                double fExtrusionBrightness = 0;
                                if ( rrProp.Value >>= fExtrusionBrightness )
                                    AddOpt( DFF_Prop_c3DAmbientIntensity, (sal_Int32)( fExtrusionBrightness * 655.36 ) );
                            }
                            else if ( rrProp.Name.equals( sExtrusionDepth ) )
                            {
                                double fDepth = 0;
                                double fFraction = 0;
                                css::drawing::EnhancedCustomShapeParameterPair aDepthParaPair;
                                if ( ( rrProp.Value >>= aDepthParaPair ) && ( aDepthParaPair.First.Value >>= fDepth ) && ( aDepthParaPair.Second.Value >>= fFraction ) )
                                {
                                    double fForeDepth = fDepth * fFraction;
                                    double fBackDepth = fDepth - fForeDepth;

                                    fBackDepth *= 360.0;
                                    AddOpt( DFF_Prop_c3DExtrudeBackward, (sal_Int32)fBackDepth );

                                    if ( fForeDepth != 0.0 )
                                    {
                                        fForeDepth *= 360.0;
                                        AddOpt( DFF_Prop_c3DExtrudeForward, (sal_Int32)fForeDepth );
                                    }
                                }
                            }
                            else if ( rrProp.Name.equals( sExtrusionDiffusion ) )
                            {
                                double fExtrusionDiffusion = 0;
                                if ( rrProp.Value >>= fExtrusionDiffusion )
                                    AddOpt( DFF_Prop_c3DDiffuseAmt, (sal_Int32)( fExtrusionDiffusion * 655.36 ) );
                            }
                            else if ( rrProp.Name.equals( sExtrusionNumberOfLineSegments ) )
                            {
                                sal_Int32 nExtrusionNumberOfLineSegments = 0;
                                if ( rrProp.Value >>= nExtrusionNumberOfLineSegments )
                                    AddOpt( DFF_Prop_c3DTolerance, nExtrusionNumberOfLineSegments );
                            }
                            else if ( rrProp.Name.equals( sExtrusionLightFace ) )
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
                            else if ( rrProp.Name.equals( sExtrusionFirstLightHarsh ) )
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
                            else if ( rrProp.Name.equals( sExtrusionSecondLightHarsh ) )
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
                            else if ( rrProp.Name.equals( sExtrusionFirstLightLevel ) )
                            {
                                double fExtrusionFirstLightLevel = 0;
                                if ( rrProp.Value >>= fExtrusionFirstLightLevel )
                                    AddOpt( DFF_Prop_c3DKeyIntensity, (sal_Int32)( fExtrusionFirstLightLevel * 655.36 ) );
                            }
                            else if ( rrProp.Name.equals( sExtrusionSecondLightLevel ) )
                            {
                                double fExtrusionSecondLightLevel = 0;
                                if ( rrProp.Value >>= fExtrusionSecondLightLevel )
                                    AddOpt( DFF_Prop_c3DFillIntensity, (sal_Int32)( fExtrusionSecondLightLevel * 655.36 ) );
                            }
                            else if ( rrProp.Name.equals( sExtrusionFirstLightDirection ) )
                            {
                                drawing::Direction3D aExtrusionFirstLightDirection;
                                if ( rrProp.Value >>= aExtrusionFirstLightDirection )
                                {
                                    AddOpt( DFF_Prop_c3DKeyX, (sal_Int32)aExtrusionFirstLightDirection.DirectionX  );
                                    AddOpt( DFF_Prop_c3DKeyY, (sal_Int32)aExtrusionFirstLightDirection.DirectionY  );
                                    AddOpt( DFF_Prop_c3DKeyZ, (sal_Int32)aExtrusionFirstLightDirection.DirectionZ  );
                                }
                            }
                            else if ( rrProp.Name.equals( sExtrusionSecondLightDirection ) )
                            {
                                drawing::Direction3D aExtrusionSecondLightPosition;
                                if ( rrProp.Value >>= aExtrusionSecondLightPosition )
                                {
                                    AddOpt( DFF_Prop_c3DFillX, (sal_Int32)aExtrusionSecondLightPosition.DirectionX  );
                                    AddOpt( DFF_Prop_c3DFillY, (sal_Int32)aExtrusionSecondLightPosition.DirectionY  );
                                    AddOpt( DFF_Prop_c3DFillZ, (sal_Int32)aExtrusionSecondLightPosition.DirectionZ  );
                                }
                            }
                            else if ( rrProp.Name.equals( sExtrusionMetal ) )
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
                            else if ( rrProp.Name.equals( sExtrusionShadeMode ) )
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
                            else if ( rrProp.Name.equals( sExtrusionRotateAngle ) )
                            {
                                double fExtrusionAngleX = 0;
                                double fExtrusionAngleY = 0;
                                css::drawing::EnhancedCustomShapeParameterPair aRotateAnglePair;
                                if ( ( rrProp.Value >>= aRotateAnglePair ) && ( aRotateAnglePair.First.Value >>= fExtrusionAngleX ) && ( aRotateAnglePair.Second.Value >>= fExtrusionAngleY ) )
                                {
                                    fExtrusionAngleX *= 65536;
                                    fExtrusionAngleY *= 65536;
                                    AddOpt( DFF_Prop_c3DXRotationAngle, (sal_Int32)fExtrusionAngleX );
                                    AddOpt( DFF_Prop_c3DYRotationAngle, (sal_Int32)fExtrusionAngleY );
                                }
                            }
                            else if ( rrProp.Name.equals( sExtrusionRotationCenter ) )
                            {
                                drawing::Direction3D aExtrusionRotationCenter;
                                if ( rrProp.Value >>= aExtrusionRotationCenter )
                                {
                                    AddOpt( DFF_Prop_c3DRotationCenterX, (sal_Int32)( aExtrusionRotationCenter.DirectionX * 360.0 ) );
                                    AddOpt( DFF_Prop_c3DRotationCenterY, (sal_Int32)( aExtrusionRotationCenter.DirectionY * 360.0 ) );
                                    AddOpt( DFF_Prop_c3DRotationCenterZ, (sal_Int32)( aExtrusionRotationCenter.DirectionZ * 360.0 ) );
                                    nFillHarshFlags &=~8; // don't use AutoRotationCenter;
                                }
                            }
                            else if ( rrProp.Name.equals( sExtrusionShininess ) )
                            {
                                double fExtrusionShininess = 0;
                                if ( rrProp.Value >>= fExtrusionShininess )
                                    AddOpt( DFF_Prop_c3DShininess, (sal_Int32)( fExtrusionShininess * 655.36 ) );
                            }
                            else if ( rrProp.Name.equals( sExtrusionSkew ) )
                            {
                                double fSkewAmount = 0;
                                double fSkewAngle = 0;
                                css::drawing::EnhancedCustomShapeParameterPair aSkewParaPair;
                                if ( ( rrProp.Value >>= aSkewParaPair ) && ( aSkewParaPair.First.Value >>= fSkewAmount ) && ( aSkewParaPair.Second.Value >>= fSkewAngle ) )
                                {
                                    AddOpt( DFF_Prop_c3DSkewAmount, (sal_Int32)fSkewAmount );
                                    AddOpt( DFF_Prop_c3DSkewAngle, (sal_Int32)( fSkewAngle * 65536 ) );
                                }
                            }
                            else if ( rrProp.Name.equals( sExtrusionSpecularity ) )
                            {
                                double fExtrusionSpecularity = 0;
                                if ( rrProp.Value >>= fExtrusionSpecularity )
                                    AddOpt( DFF_Prop_c3DSpecularAmt, (sal_Int32)( fExtrusionSpecularity * 1333 ) );
                            }
                            else if ( rrProp.Name.equals( sExtrusionProjectionMode ) )
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
                            else if ( rrProp.Name.equals( sExtrusionViewPoint ) )
                            {
                                drawing::Position3D aExtrusionViewPoint;
                                if ( rrProp.Value >>= aExtrusionViewPoint )
                                {
                                    aExtrusionViewPoint.PositionX *= 360.0;
                                    aExtrusionViewPoint.PositionY *= 360.0;
                                    aExtrusionViewPoint.PositionZ *= 360.0;
                                    AddOpt( DFF_Prop_c3DXViewpoint, (sal_Int32)aExtrusionViewPoint.PositionX  );
                                    AddOpt( DFF_Prop_c3DYViewpoint, (sal_Int32)aExtrusionViewPoint.PositionY  );
                                    AddOpt( DFF_Prop_c3DZViewpoint, (sal_Int32)aExtrusionViewPoint.PositionZ  );
                                }
                            }
                            else if ( rrProp.Name.equals( sExtrusionOrigin ) )
                            {
                                double fExtrusionOriginX = 0;
                                double fExtrusionOriginY = 0;
                                css::drawing::EnhancedCustomShapeParameterPair aOriginPair;
                                if ( ( rrProp.Value >>= aOriginPair ) && ( aOriginPair.First.Value >>= fExtrusionOriginX ) && ( aOriginPair.Second.Value >>= fExtrusionOriginY ) )
                                {
                                    AddOpt( DFF_Prop_c3DOriginX, (sal_Int32)( fExtrusionOriginX * 65536 ) );
                                    AddOpt( DFF_Prop_c3DOriginY, (sal_Int32)( fExtrusionOriginY * 65536 ) );
                                }
                            }
                            else if ( rrProp.Name.equals( sExtrusionColor ) )
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
                                            sal_uInt32 nFillColor = ImplGetColor( *static_cast<sal_uInt32 const *>(aFillColor2.getValue()) );
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
                else if ( rProp.Name.equals( sEquations ) )
                {
                    if ( !bIsDefaultObject )
                    {
                        sal_uInt16 nElements = (sal_uInt16)aEquations.size();
                        if ( nElements )
                        {
                            sal_uInt16 nElementSize = 8;
                            sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                            SvMemoryStream aOut( nStreamSize );
                            aOut.WriteUInt16( nElements )
                               .WriteUInt16( nElements )
                               .WriteUInt16( nElementSize );

                            std::vector< EnhancedCustomShapeEquation >::const_iterator aIter( aEquations.begin() );
                            std::vector< EnhancedCustomShapeEquation >::const_iterator aEnd ( aEquations.end() );
                            while( aIter != aEnd )
                            {
                                aOut.WriteUInt16( aIter->nOperation )
                                    .WriteInt16( aIter->nPara[ 0 ] )
                                    .WriteInt16( aIter->nPara[ 1 ] )
                                    .WriteInt16( aIter->nPara[ 2 ] );
                                ++aIter;
                            }
                            sal_uInt8* pBuf = new sal_uInt8[ nStreamSize ];
                            memcpy( pBuf, aOut.GetData(), nStreamSize );
                            AddOpt( DFF_Prop_pFormulas, true, nStreamSize - 6, pBuf, nStreamSize );
                        }
                        else
                        {
                            sal_uInt8* pBuf = new sal_uInt8[ 1 ];
                            AddOpt( DFF_Prop_pFormulas, true, 0, pBuf, 0 );
                        }
                    }
                }
                else if ( rProp.Name.equals( sPath ) )
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
                            const OUString sPathExtrusionAllowed               ( "ExtrusionAllowed"  );
                            const OUString sPathConcentricGradientFillAllowed  ( "ConcentricGradientFillAllowed"  );
                            const OUString sPathTextPathAllowed                ( "TextPathAllowed"  );
                            const OUString sPathCoordinates                    ( "Coordinates"  );
                            const OUString sPathGluePoints                     ( "GluePoints"  );
                            const OUString sPathGluePointType                  ( "GluePointType"  );
                            const OUString sPathSegments                       ( "Segments"  );
                            const OUString sPathStretchX                       ( "StretchX"  );
                            const OUString sPathStretchY                       ( "StretchY"  );
                            const OUString sPathTextFrames                     ( "TextFrames"  );

                            if ( rrProp.Name.equals( sPathExtrusionAllowed ) )
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
                            else if ( rrProp.Name.equals( sPathConcentricGradientFillAllowed ) )
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
                            else if ( rrProp.Name.equals( sPathTextPathAllowed ) )
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
                            else if ( rrProp.Name.equals( sPathCoordinates ) )
                            {
                                if ( !bIsDefaultObject )
                                {
                                    aPathCoordinatesProp = rrProp.Value;
                                    bPathCoordinatesProp = true;
                                }
                            }
                            else if ( rrProp.Name.equals( sPathGluePoints ) )
                            {
                                if ( !bIsDefaultObject )
                                {
                                    css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair> aGluePoints;
                                    if ( rrProp.Value >>= aGluePoints )
                                    {
                                        // creating the vertices
                                        sal_uInt16 nElements = (sal_uInt16)aGluePoints.getLength();
                                        if ( nElements )
                                        {
                                            sal_uInt16 j, nElementSize = 8;
                                            sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                                            SvMemoryStream aOut( nStreamSize );
                                            aOut.WriteUInt16( nElements )
                                               .WriteUInt16( nElements )
                                               .WriteUInt16( nElementSize );
                                            for( j = 0; j < nElements; j++ )
                                            {
                                                sal_Int32 X = GetValueForEnhancedCustomShapeParameter( aGluePoints[ j ].First, aEquationOrder );
                                                sal_Int32 Y = GetValueForEnhancedCustomShapeParameter( aGluePoints[ j ].Second, aEquationOrder );
                                                aOut.WriteInt32( X )
                                                   .WriteInt32( Y );
                                            }
                                            sal_uInt8* pBuf = new sal_uInt8[ nStreamSize ];
                                            memcpy( pBuf, aOut.GetData(), nStreamSize );
                                            AddOpt( DFF_Prop_connectorPoints, true, nStreamSize - 6, pBuf, nStreamSize );   // -6
                                        }
                                        else
                                        {
                                            sal_uInt8* pBuf = new sal_uInt8[ 1 ];
                                            AddOpt( DFF_Prop_connectorPoints, true, 0, pBuf, 0 );
                                        }
                                    }
                                }
                            }
                            else if ( rrProp.Name.equals( sPathGluePointType ) )
                            {
                                sal_Int16 nGluePointType = sal_Int16();
                                if ( rrProp.Value >>= nGluePointType )
                                    AddOpt( DFF_Prop_connectorType, (sal_uInt16)nGluePointType );
                            }
                            else if ( rrProp.Name.equals( sPathSegments ) )
                            {
                                if ( !bIsDefaultObject )
                                {
                                    css::uno::Sequence< css::drawing::EnhancedCustomShapeSegment > aSegments;
                                    if ( rrProp.Value >>= aSegments )
                                    {
                                        // creating seginfo
                                        if ( (sal_uInt16)aSegments.getLength() )
                                        {
                                            sal_uInt16 j, nElements = (sal_uInt16)aSegments.getLength();
                                            sal_uInt16 nElementSize = 2;
                                            sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                                            SvMemoryStream aOut( nStreamSize );
                                            aOut.WriteUInt16( nElements )
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
                                                sal_uInt16 nVal = (sal_uInt16)aSegments[ j ].Count;
                                                switch( aSegments[ j ].Command )
                                                {
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::UNKNOWN :
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::LINETO :
                                                        break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::MOVETO :
                                                        nVal = (msopathMoveTo << 13);
                                                        break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::CURVETO :
                                                    {
                                                        nVal |= (msopathCurveTo << 13);
                                                    }
                                                    break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::CLOSESUBPATH :
                                                    {
                                                        nVal = 1;
                                                        nVal |= (msopathClose << 13);
                                                    }
                                                    break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::ENDSUBPATH :
                                                    {
                                                        nVal = (msopathEnd << 13);
                                                    }
                                                    break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::NOFILL :
                                                    {
                                                        nVal = (msopathEscape << 13) | (5 << 8);
                                                    }
                                                    break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::NOSTROKE :
                                                    {
                                                        nVal = (msopathEscape << 13) | (11 << 8);
                                                    }
                                                    break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO :
                                                    {
                                                        nVal *= 3;
                                                        nVal |= (msopathEscape << 13) | (1 << 8);
                                                    }
                                                    break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE :
                                                    {
                                                        nVal *= 3;
                                                        nVal |= (msopathEscape << 13) | (2 << 8);
                                                    }
                                                    break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::ARCTO :
                                                    {
                                                        nVal <<= 2;
                                                        nVal |= (msopathEscape << 13) | (3 << 8);
                                                    }
                                                    break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::ARC :
                                                    {
                                                        nVal <<= 2;
                                                        nVal |= (msopathEscape << 13) | (4 << 8);
                                                    }
                                                    break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO :
                                                    {
                                                        nVal <<= 2;
                                                        nVal |= (msopathEscape << 13) | (5 << 8);
                                                    }
                                                    break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARC :
                                                    {
                                                        nVal <<= 2;
                                                        nVal |= (msopathEscape << 13) | (6 << 8);
                                                    }
                                                    break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX :
                                                    {
                                                        nVal |= (msopathEscape << 13) | (7 << 8);
                                                    }
                                                    break;
                                                    case css::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY :
                                                    {
                                                        nVal |= (msopathEscape << 13) | (8 << 8);
                                                    }
                                                    break;
                                                }
                                                aOut.WriteUInt16( nVal );
                                            }
                                            sal_uInt8* pBuf = new sal_uInt8[ nStreamSize ];
                                            memcpy( pBuf, aOut.GetData(), nStreamSize );
                                            AddOpt( DFF_Prop_pSegmentInfo, false, nStreamSize - 6, pBuf, nStreamSize );
                                        }
                                        else
                                        {
                                            sal_uInt8* pBuf = new sal_uInt8[ 1 ];
                                            AddOpt( DFF_Prop_pSegmentInfo, true, 0, pBuf, 0 );
                                        }
                                    }
                                }
                            }
                            else if ( rrProp.Name.equals( sPathStretchX ) )
                            {
                                if ( !bIsDefaultObject )
                                {
                                    sal_Int32 nStretchX = 0;
                                    if ( rrProp.Value >>= nStretchX )
                                        AddOpt( DFF_Prop_stretchPointX, nStretchX );
                                }
                            }
                            else if ( rrProp.Name.equals( sPathStretchY ) )
                            {
                                if ( !bIsDefaultObject )
                                {
                                    sal_Int32 nStretchY = 0;
                                    if ( rrProp.Value >>= nStretchY )
                                        AddOpt( DFF_Prop_stretchPointY, nStretchY );
                                }
                            }
                            else if ( rrProp.Name.equals( sPathTextFrames ) )
                            {
                                if ( !bIsDefaultObject )
                                {
                                    css::uno::Sequence< css::drawing::EnhancedCustomShapeTextFrame > aPathTextFrames;
                                    if ( rrProp.Value >>= aPathTextFrames )
                                    {
                                        if ( (sal_uInt16)aPathTextFrames.getLength() )
                                        {
                                            sal_uInt16 j, nElements = (sal_uInt16)aPathTextFrames.getLength();
                                            sal_uInt16 nElementSize = 16;
                                            sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                                            SvMemoryStream aOut( nStreamSize );
                                            aOut.WriteUInt16( nElements )
                                               .WriteUInt16( nElements )
                                               .WriteUInt16( nElementSize );
                                            for ( j = 0; j < nElements; j++ )
                                            {
                                                sal_Int32 nLeft = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].TopLeft.First, aEquationOrder );
                                                sal_Int32 nTop  = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].TopLeft.Second, aEquationOrder );
                                                sal_Int32 nRight = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].BottomRight.First, aEquationOrder );
                                                sal_Int32 nBottom = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].BottomRight.Second, aEquationOrder );

                                                aOut.WriteInt32( nLeft )
                                                   .WriteInt32( nTop )
                                                   .WriteInt32( nRight )
                                                   .WriteInt32( nBottom );
                                            }
                                            sal_uInt8* pBuf = new sal_uInt8[ nStreamSize ];
                                            memcpy( pBuf, aOut.GetData(), nStreamSize );
                                            AddOpt( DFF_Prop_textRectangles, true, nStreamSize - 6, pBuf, nStreamSize );
                                        }
                                        else
                                        {
                                            sal_uInt8* pBuf = new sal_uInt8[ 1 ];
                                            AddOpt( DFF_Prop_textRectangles, true, 0, pBuf, 0 );
                                        }
                                    }
                                }
                            }
                        }
                        if ( nPathFlags != nPathFlagsOrg )
                            AddOpt( DFF_Prop_fFillOK, nPathFlags );
                    }
                }
                else if ( rProp.Name.equals( sTextPath ) )
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
                            const OUString sTextPathMode       ( "TextPathMode"  );
                            const OUString sTextPathScaleX     ( "ScaleX"  );
                            const OUString sSameLetterHeights  ( "SameLetterHeights"  );

                            if ( rrProp.Name.equals( sTextPath ) )
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
                            else if ( rrProp.Name.equals( sTextPathMode ) )
                            {
                                css::drawing::EnhancedCustomShapeTextPathMode eTextPathMode;
                                if ( rrProp.Value >>= eTextPathMode )
                                {
                                    nTextPathFlags |= 0x05000000;
                                    nTextPathFlags &=~0x500;    // TextPathMode_NORMAL
                                    if ( eTextPathMode == css::drawing::EnhancedCustomShapeTextPathMode_PATH )
                                        nTextPathFlags |= 0x100;
                                    else if ( eTextPathMode == css::drawing::EnhancedCustomShapeTextPathMode_SHAPE )
                                        nTextPathFlags |= 0x500;
                                }
                            }
                            else if ( rrProp.Name.equals( sTextPathScaleX ) )
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
                            else if ( rrProp.Name.equals( sSameLetterHeights ) )
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
                            const OUString sCharFontName( "CharFontName"  );
                            uno::Any aAny = aXPropSet->getPropertyValue( sCharFontName );
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
                                        SdrFitToSizeType  eFTS( static_cast<const SdrTextFitToSizeTypeItem&>(pCustoShape->GetMergedItem( SDRATTR_TEXT_FITTOSIZE )).GetValue() );
                                        if ( eFTS == SDRTEXTFIT_ALLLINES)
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
                            OutlinerParaObject* pOutlinerParaObject = pCustoShape->GetOutlinerParaObject();
                            if ( pOutlinerParaObject && pOutlinerParaObject->IsVertical() )
                                nTextPathFlags |= 0x2000;
                        }
                        if ( nTextPathFlags != nTextPathFlagsOrg )
                            AddOpt( DFF_Prop_gtextFStrikethrough, nTextPathFlags );
                    }
                }
                else if ( rProp.Name.equals( sHandles ) )
                {
                    if ( !bIsDefaultObject )
                    {
                        bPredefinedHandlesUsed = false;
                        if ( rProp.Value >>= aHandlesPropSeq )
                        {
                            sal_uInt16 nElements = (sal_uInt16)aHandlesPropSeq.getLength();
                            if ( nElements )
                            {
                                sal_uInt16 k, j, nElementSize = 36;
                                sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                                SvMemoryStream aOut( nStreamSize );
                                aOut.WriteUInt16( nElements )
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
                                    for ( j = 0; j < rPropSeq.getLength(); j++ )
                                    {
                                        const beans::PropertyValue& rPropVal = rPropSeq[ j ];

                                        const OUString sPosition           ( "Position"  );
                                        const OUString sMirroredX          ( "MirroredX"  );
                                        const OUString sMirroredY          ( "MirroredY"  );
                                        const OUString sSwitched           ( "Switched"  );
                                        const OUString sPolar              ( "Polar"  );
                                        const OUString sRadiusRangeMinimum ( "RadiusRangeMinimum"  );
                                        const OUString sRadiusRangeMaximum ( "RadiusRangeMaximum"  );
                                        const OUString sRangeXMinimum      ( "RangeXMinimum"  );
                                        const OUString sRangeXMaximum      ( "RangeXMaximum"  );
                                        const OUString sRangeYMinimum      ( "RangeYMinimum"  );
                                        const OUString sRangeYMaximum      ( "RangeYMaximum"  );

                                        if ( rPropVal.Name.equals( sPosition ) )
                                        {
                                            css::drawing::EnhancedCustomShapeParameterPair aPosition;
                                            if ( rPropVal.Value >>= aPosition )
                                            {
                                                GetValueForEnhancedCustomShapeHandleParameter( nXPosition, aPosition.First );
                                                GetValueForEnhancedCustomShapeHandleParameter( nYPosition, aPosition.Second );
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sMirroredX ) )
                                        {
                                            bool bMirroredX;
                                            if ( rPropVal.Value >>= bMirroredX )
                                            {
                                                if ( bMirroredX )
                                                    nFlags |= 1;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sMirroredY ) )
                                        {
                                            bool bMirroredY;
                                            if ( rPropVal.Value >>= bMirroredY )
                                            {
                                                if ( bMirroredY )
                                                    nFlags |= 2;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sSwitched ) )
                                        {
                                            bool bSwitched;
                                            if ( rPropVal.Value >>= bSwitched )
                                            {
                                                if ( bSwitched )
                                                    nFlags |= 4;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sPolar ) )
                                        {
                                            css::drawing::EnhancedCustomShapeParameterPair aPolar;
                                            if ( rPropVal.Value >>= aPolar )
                                            {
                                                if ( GetValueForEnhancedCustomShapeHandleParameter( nXMap, aPolar.First ) )
                                                    nFlags |= 0x800;
                                                if ( GetValueForEnhancedCustomShapeHandleParameter( nYMap, aPolar.Second ) )
                                                    nFlags |= 0x1000;
                                                nFlags |= 8;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sRadiusRangeMinimum ) )
                                        {
                                            nYRangeMin = (sal_Int32)0xff4c0000; // the range of angles seems to be a not
                                            nYRangeMax = (sal_Int32)0x00b40000; // used feature, so we are defaulting this

                                            css::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
                                            if ( rPropVal.Value >>= aRadiusRangeMinimum )
                                            {
                                                if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMin, aRadiusRangeMinimum ) )
                                                    nFlags |= 0x80;
                                                nFlags |= 0x2000;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sRadiusRangeMaximum ) )
                                        {
                                            nYRangeMin = (sal_Int32)0xff4c0000; // the range of angles seems to be a not
                                            nYRangeMax = (sal_Int32)0x00b40000; // used feature, so we are defaulting this

                                            css::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
                                            if ( rPropVal.Value >>= aRadiusRangeMaximum )
                                            {
                                                if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMax, aRadiusRangeMaximum ) )
                                                    nFlags |= 0x100;
                                                nFlags |= 0x2000;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sRangeXMinimum ) )
                                        {
                                            css::drawing::EnhancedCustomShapeParameter aXRangeMinimum;
                                            if ( rPropVal.Value >>= aXRangeMinimum )
                                            {
                                                if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMin, aXRangeMinimum ) )
                                                    nFlags |= 0x80;
                                                nFlags |= 0x20;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sRangeXMaximum ) )
                                        {
                                            css::drawing::EnhancedCustomShapeParameter aXRangeMaximum;
                                            if ( rPropVal.Value >>= aXRangeMaximum )
                                            {
                                                if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMax, aXRangeMaximum ) )
                                                    nFlags |= 0x100;
                                                nFlags |= 0x20;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sRangeYMinimum ) )
                                        {
                                            css::drawing::EnhancedCustomShapeParameter aYRangeMinimum;
                                            if ( rPropVal.Value >>= aYRangeMinimum )
                                            {
                                                if ( GetValueForEnhancedCustomShapeHandleParameter( nYRangeMin, aYRangeMinimum ) )
                                                    nFlags |= 0x200;
                                                nFlags |= 0x20;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sRangeYMaximum ) )
                                        {
                                            css::drawing::EnhancedCustomShapeParameter aYRangeMaximum;
                                            if ( rPropVal.Value >>= aYRangeMaximum )
                                            {
                                                if ( GetValueForEnhancedCustomShapeHandleParameter( nYRangeMax, aYRangeMaximum ) )
                                                    nFlags |= 0x400;
                                                nFlags |= 0x20;
                                            }
                                        }
                                    }
                                    aOut.WriteUInt32( nFlags )
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
                                sal_uInt8* pBuf = new sal_uInt8[ nStreamSize ];
                                memcpy( pBuf, aOut.GetData(), nStreamSize );
                                AddOpt( DFF_Prop_Handles, true, nStreamSize - 6, pBuf, nStreamSize );
                            }
                            else
                            {
                                sal_uInt8* pBuf = new sal_uInt8[ 1 ];
                                AddOpt( DFF_Prop_Handles, true, 0, pBuf, 0 );
                            }
                        }
                    }
                }
                else if ( rProp.Name.equals( sAdjustmentValues ) )
                {
                    // it is required, that the information which handle is polar has already be read,
                    // so we are able to change the polar value to a fixed float
                    aAdjustmentValuesProp = rProp.Value;
                    bAdjustmentValuesProp = true;
                }
            }
            if ( bAdjustmentValuesProp )
            {
                uno::Sequence< css::drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq;
                if ( aAdjustmentValuesProp >>= aAdjustmentSeq )
                {
                    if ( bPredefinedHandlesUsed )
                        LookForPolarHandles( eShapeType, nAdjustmentsWhichNeedsToBeConverted );

                    sal_Int32 k, nValue = 0, nAdjustmentValues = aAdjustmentSeq.getLength();
                    for ( k = 0; k < nAdjustmentValues; k++ )
                        if( GetAdjustmentValue( aAdjustmentSeq[ k ], k, nAdjustmentsWhichNeedsToBeConverted, nValue ) )
                            AddOpt( (sal_uInt16)( DFF_Prop_adjustValue + k ), (sal_uInt32)nValue );
                }
            }
            if( bPathCoordinatesProp )
            {
                css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair > aCoordinates;
                if ( aPathCoordinatesProp >>= aCoordinates )
                {
                    // creating the vertices
                    if (aCoordinates.getLength() > 0)
                    {
                        sal_uInt16 j, nElements = (sal_uInt16)aCoordinates.getLength();
                        sal_uInt16 nElementSize = 8;
                        sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                        SvMemoryStream aOut( nStreamSize );
                        aOut.WriteUInt16( nElements )
                           .WriteUInt16( nElements )
                           .WriteUInt16( nElementSize );
                        for( j = 0; j < nElements; j++ )
                        {
                            sal_Int32 X = GetValueForEnhancedCustomShapeParameter( aCoordinates[ j ].First, aEquationOrder, true );
                            sal_Int32 Y = GetValueForEnhancedCustomShapeParameter( aCoordinates[ j ].Second, aEquationOrder, true );
                            aOut.WriteInt32( X )
                               .WriteInt32( Y );
                        }
                        sal_uInt8* pBuf = new sal_uInt8[ nStreamSize ];
                        memcpy( pBuf, aOut.GetData(), nStreamSize );
                        AddOpt( DFF_Prop_pVertices, true, nStreamSize - 6, pBuf, nStreamSize ); // -6
                    }
                    else
                    {
                        sal_uInt8* pBuf = new sal_uInt8[ 1 ];
                        AddOpt( DFF_Prop_pVertices, true, 0, pBuf, 0 );
                    }
                }
            }
        }
    }
}

MSO_SPT EscherPropertyContainer::GetCustomShapeType( const uno::Reference< drawing::XShape > & rXShape, sal_uInt32& nMirrorFlags, OUString& rShapeType, bool bOOXML )
{
    MSO_SPT eShapeType = mso_sptNil;
    nMirrorFlags = 0;
    uno::Reference< beans::XPropertySet > aXPropSet( rXShape, uno::UNO_QUERY );
    if ( aXPropSet.is() )
    {
        try
        {
            const OUString  sCustomShapeGeometry( "CustomShapeGeometry" );
            uno::Any aGeoPropSet = aXPropSet->getPropertyValue( sCustomShapeGeometry );
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
                            nMirrorFlags  |= SHAPEFLAG_FLIPH;
                    }
                    else if ( rProp.Name == "MirroredY" )
                    {
                        bool bMirroredY;
                        if ( ( rProp.Value >>= bMirroredY ) && bMirroredY )
                            nMirrorFlags  |= SHAPEFLAG_FLIPV;
                    }
                }
            }
        }
        catch( const css::uno::Exception& )
        {
        }
    }
    return eShapeType;
}


// Implement for form control export
bool   EscherPropertyContainer::CreateBlipPropertiesforOLEControl(const css::uno::Reference< css::beans::XPropertySet > & rXPropSet, const css::uno::Reference< css::drawing::XShape > & rXShape)
{
    SdrObject* pShape = GetSdrObjectFromXShape( rXShape );
    if ( pShape )
    {
        SdrModel* pMod = pShape->GetModel();
        Graphic aGraphic(SdrExchangeView::GetObjGraphic( pMod, pShape));

        GraphicObject   aGraphicObject = aGraphic;
        OString  aUniqueId = aGraphicObject.GetUniqueID();
        if ( aUniqueId.getLength() )
        {
            if ( pGraphicProvider && pPicOutStrm && pShapeBoundRect )
            {
                Rectangle aRect( Point( 0, 0 ), pShapeBoundRect->GetSize() );

                sal_uInt32 nBlibId = pGraphicProvider->GetBlibID( *pPicOutStrm, aUniqueId, aRect );
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
    for( size_t i = 0, n = maPersistTable.size(); i < n; ++i ) {
        delete maPersistTable[ i ];
    }
}

bool EscherPersistTable::PtIsID( sal_uInt32 nID )
{
    for( size_t i = 0, n = maPersistTable.size(); i < n; ++i ) {
        EscherPersistEntry* pPtr = maPersistTable[ i ];
        if ( pPtr->mnID == nID ) {
            return true;
        }
    }
    return false;
}

void EscherPersistTable::PtInsert( sal_uInt32 nID, sal_uInt32 nOfs )
{
    maPersistTable.push_back( new EscherPersistEntry( nID, nOfs ) );
}

sal_uInt32 EscherPersistTable::PtDelete( sal_uInt32 nID )
{
    ::std::vector< EscherPersistEntry* >::iterator it = maPersistTable.begin();
    for( ; it != maPersistTable.end() ; ++it )
    {
        if ( (*it)->mnID == nID ) {
            delete *it;
            maPersistTable.erase( it );
            break;
        }
    }
    return 0;
}

sal_uInt32 EscherPersistTable::PtGetOffsetByID( sal_uInt32 nID )
{
    for( size_t i = 0, n = maPersistTable.size(); i < n; ++i ) {
        EscherPersistEntry* pPtr = maPersistTable[ i ];
        if ( pPtr->mnID == nID ) {
            return pPtr->mnOffset;
        }
    }
    return 0;
};

sal_uInt32 EscherPersistTable::PtReplace( sal_uInt32 nID, sal_uInt32 nOfs )
{
    for( size_t i = 0, n = maPersistTable.size(); i < n; ++i ) {
        EscherPersistEntry* pPtr = maPersistTable[ i ];
        if ( pPtr->mnID == nID ) {
            sal_uInt32 nRetValue = pPtr->mnOffset;
            pPtr->mnOffset = nOfs;
            return nRetValue;
        }
    }
    return 0;
}

sal_uInt32 EscherPersistTable::PtReplaceOrInsert( sal_uInt32 nID, sal_uInt32 nOfs )
{
    for( size_t i = 0, n = maPersistTable.size(); i < n; ++i ) {
        EscherPersistEntry* pPtr = maPersistTable[ i ];
        if ( pPtr->mnID == nID ) {
            sal_uInt32 nRetValue = pPtr->mnOffset;
            pPtr->mnOffset = nOfs;
            return nRetValue;
        }
    }
    PtInsert( nID, nOfs );
    return 0;
}

bool EscherPropertyValueHelper::GetPropertyValue(
    css::uno::Any& rAny,
    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
    const OUString& rString,
    bool bTestPropertyAvailability)
{
    bool bRetValue = true;
    if ( bTestPropertyAvailability )
    {
        bRetValue = false;
        try
        {
            css::uno::Reference< css::beans::XPropertySetInfo >
                aXPropSetInfo( rXPropSet->getPropertySetInfo() );
            if ( aXPropSetInfo.is() )
                bRetValue = aXPropSetInfo->hasPropertyByName( rString );
        }
        catch( const css::uno::Exception& )
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
        catch( const css::uno::Exception& )
        {
            bRetValue = false;
        }
    }
    return bRetValue;
}

css::beans::PropertyState EscherPropertyValueHelper::GetPropertyState(
    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
        const OUString& rPropertyName )
{
    css::beans::PropertyState eRetValue = css::beans::PropertyState_AMBIGUOUS_VALUE;
    try
    {
        css::uno::Reference< css::beans::XPropertyState > aXPropState
                ( rXPropSet, css::uno::UNO_QUERY );
        if ( aXPropState.is() )
            eRetValue = aXPropState->getPropertyState( rPropertyName );
    }
    catch( const css::uno::Exception& )
    {
    }
    return eRetValue;
}

EscherBlibEntry::EscherBlibEntry( sal_uInt32 nPictureOffset, const GraphicObject& rObject, const OString& rId,
                                        const GraphicAttr* pGraphicAttr ) :
    mnPictureOffset ( nPictureOffset ),
    mnRefCount      ( 1 ),
    mnSizeExtra     ( 0 ),
    maPrefSize      ( rObject.GetPrefSize() ),
    maPrefMapMode   ( rObject.GetPrefMapMode() ),
    mbIsEmpty       ( true )
{
    mbIsNativeGraphicPossible = ( pGraphicAttr == nullptr );
    meBlibType = UNKNOWN;
    mnSize = 0;

    sal_uInt32      nLen = static_cast<sal_uInt32>(rId.getLength());
    const sal_Char* pData = rId.getStr();
    GraphicType     eType( rObject.GetType() );
    if ( nLen && pData && ( eType != GRAPHIC_NONE ) )
    {
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
                aSt.WriteUInt16( pGraphicAttr->GetDrawMode() )
                   .WriteUInt32( static_cast<sal_uInt32>(pGraphicAttr->GetMirrorFlags()) )
                   .WriteInt32( pGraphicAttr->GetLeftCrop() )
                   .WriteInt32( pGraphicAttr->GetTopCrop() )
                   .WriteInt32( pGraphicAttr->GetRightCrop() )
                   .WriteInt32( pGraphicAttr->GetBottomCrop() )
                   .WriteUInt16( pGraphicAttr->GetRotation() )
                   .WriteInt16( pGraphicAttr->GetLuminance() )
                   .WriteInt16( pGraphicAttr->GetContrast() )
                   .WriteInt16( pGraphicAttr->GetChannelR() )
                   .WriteInt16( pGraphicAttr->GetChannelG() )
                   .WriteInt16( pGraphicAttr->GetChannelB() )
                   .WriteDouble( pGraphicAttr->GetGamma() );
                aSt.WriteBool( pGraphicAttr->IsInvert() )
                   .WriteUChar( pGraphicAttr->GetTransparency() );
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
    }
};

void EscherBlibEntry::WriteBlibEntry( SvStream& rSt, bool bWritePictureOffset, sal_uInt32 nResize )
{
    sal_uInt32  nPictureOffset = ( bWritePictureOffset ) ? mnPictureOffset : 0;

    rSt.WriteUInt32( ( ESCHER_BSE << 16 ) | ( ( (sal_uInt16)meBlibType << 4 ) | 2 ) )
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

    rSt.Write( &mnIdentifier[ 0 ], 16 );
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

EscherGraphicProvider::EscherGraphicProvider( sal_uInt32 nFlags ) :
    mnFlags         ( nFlags ),
    mpBlibEntrys    ( nullptr ),
    mnBlibBufSize   ( 0 ),
    mnBlibEntrys    ( 0 )
{
}

EscherGraphicProvider::~EscherGraphicProvider()
{
    for ( sal_uInt32 i = 0; i < mnBlibEntrys; delete mpBlibEntrys[ i++ ] ) ;
    delete[] mpBlibEntrys;
}

void EscherGraphicProvider::SetNewBlipStreamOffset( sal_Int32 nOffset )
{
    for( sal_uInt32 i = 0; i < mnBlibEntrys; i++ )
    {
        EscherBlibEntry* pBlibEntry = mpBlibEntrys[ i ];
        pBlibEntry->mnPictureOffset += nOffset;
    }
}

sal_uInt32 EscherGraphicProvider::ImplInsertBlib( EscherBlibEntry* p_EscherBlibEntry )
{
    if ( mnBlibBufSize == mnBlibEntrys )
    {
        mnBlibBufSize += 64;
        EscherBlibEntry** pTemp = new EscherBlibEntry*[ mnBlibBufSize ];
        for ( sal_uInt32 i = 0; i < mnBlibEntrys; i++ )
        {
            pTemp[ i ] = mpBlibEntrys[ i ];
        }
        delete[] mpBlibEntrys;
        mpBlibEntrys = pTemp;
    }
    mpBlibEntrys[ mnBlibEntrys++ ] = p_EscherBlibEntry;
    return mnBlibEntrys;
}

sal_uInt32 EscherGraphicProvider::GetBlibStoreContainerSize( SvStream* pMergePicStreamBSE ) const
{
    sal_uInt32 nSize = 44 * mnBlibEntrys + 8;
    if ( pMergePicStreamBSE )
    {
        for ( sal_uInt32 i = 0; i < mnBlibEntrys; i++ )
            nSize += mpBlibEntrys[ i ]->mnSize + mpBlibEntrys[ i ]->mnSizeExtra;
    }
    return nSize;
}

bool EscherGraphicProvider::WriteBlibStoreEntry(SvStream& rSt,
    sal_uInt32 nBlipId, bool bWritePictureOffSet, sal_uInt32 nResize)
{
    if (nBlipId > mnBlibEntrys || nBlipId == 0)
        return false;
    mpBlibEntrys[nBlipId-1]->WriteBlibEntry(rSt, bWritePictureOffSet, nResize);
    return true;
}

void EscherGraphicProvider::WriteBlibStoreContainer( SvStream& rSt, SvStream* pMergePicStreamBSE )
{
    sal_uInt32  nSize = GetBlibStoreContainerSize( pMergePicStreamBSE );
    if ( nSize )
    {
        rSt.WriteUInt32( ( ESCHER_BstoreContainer << 16 ) | 0x1f )
           .WriteUInt32( nSize - 8 );

        if ( pMergePicStreamBSE )
        {
            sal_uInt32 i, nBlipSize, nOldPos = pMergePicStreamBSE->Tell();
            const sal_uInt32 nBuf = 0x40000;    // 256KB buffer
            std::unique_ptr<sal_uInt8[]> pBuf(new sal_uInt8[ nBuf ]);

            for ( i = 0; i < mnBlibEntrys; i++ )
            {
                EscherBlibEntry* pBlibEntry = mpBlibEntrys[ i ];

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
                    sal_uInt32 nBytes = ( nBlipSize > nBuf ? nBuf : nBlipSize );
                    pMergePicStreamBSE->Read( pBuf.get(), nBytes );
                    rSt.Write( pBuf.get(), nBytes );
                    nBlipSize -= nBytes;
                }
            }
            pMergePicStreamBSE->Seek( nOldPos );
        }
        else
        {
            for ( sal_uInt32 i = 0; i < mnBlibEntrys; i++ )
                mpBlibEntrys[ i ]->WriteBlibEntry( rSt, true );
        }
    }
}

bool EscherGraphicProvider::GetPrefSize( const sal_uInt32 nBlibId, Size& rPrefSize, MapMode& rPrefMapMode )
{
    bool bInRange = nBlibId && ( ( nBlibId - 1 ) < mnBlibEntrys );
    if ( bInRange )
    {
        EscherBlibEntry* pEntry = mpBlibEntrys[ nBlibId - 1 ];
        rPrefSize = pEntry->maPrefSize;
        rPrefMapMode = pEntry->maPrefMapMode;
    }
    return bInRange;
}

sal_uInt32 EscherGraphicProvider::GetBlibID( SvStream& rPicOutStrm, const OString& rId,
                                            const Rectangle& /* rBoundRect */, const css::awt::Rectangle* pVisArea,
                                            const GraphicAttr* pGraphicAttr, const bool bOOxmlExport )
{
    sal_uInt32          nBlibId = 0;
    GraphicObject       aGraphicObject( rId );

    EscherBlibEntry* p_EscherBlibEntry = new EscherBlibEntry( rPicOutStrm.Tell(), aGraphicObject, rId, pGraphicAttr );
    if ( !p_EscherBlibEntry->IsEmpty() )
    {
        for ( sal_uInt32 i = 0; i < mnBlibEntrys; i++ )
        {
            if ( *( mpBlibEntrys[ i ] ) == *p_EscherBlibEntry )
            {
                mpBlibEntrys[ i ]->mnRefCount++;
                delete p_EscherBlibEntry;
                return i + 1;
            }
        }

        bool            bUseNativeGraphic( false );

        Graphic             aGraphic( aGraphicObject.GetTransformedGraphic( pGraphicAttr ) );
        GfxLink             aGraphicLink;
        SvMemoryStream      aStream;

        const sal_uInt8*    pGraphicAry = nullptr;

        if ( p_EscherBlibEntry->mbIsNativeGraphicPossible && aGraphic.IsLink() )
        {
            aGraphicLink = aGraphic.GetLink();

            p_EscherBlibEntry->mnSize = aGraphicLink.GetDataSize();
            pGraphicAry = aGraphicLink.GetData();

            if ( p_EscherBlibEntry->mnSize && pGraphicAry )
            {
                switch ( aGraphicLink.GetType() )
                {
                    case GFX_LINK_TYPE_NATIVE_JPG : p_EscherBlibEntry->meBlibType = PEG; break;
                    case GFX_LINK_TYPE_NATIVE_PNG : p_EscherBlibEntry->meBlibType = PNG; break;

                    // #i15508# added BMP type for better exports; need to check this
                    // checked - does not work that way, so keep out for now. It may
                    // work somehow with direct DIB data, but that would need to be checked
                    // carefully
                    // for more comments please check RtfAttributeOutput::FlyFrameGraphic
                    //
                    // case GFX_LINK_TYPE_NATIVE_BMP : p_EscherBlibEntry->meBlibType = DIB; break;

                    case GFX_LINK_TYPE_NATIVE_WMF :
                    {
                        if ( pGraphicAry && ( p_EscherBlibEntry->mnSize > 0x2c ) )
                        {
                            if ( ( pGraphicAry[ 0x28 ] == 0x20 ) && ( pGraphicAry[ 0x29 ] == 0x45 )     // check the magic
                                && ( pGraphicAry[ 0x2a ] == 0x4d ) && ( pGraphicAry[ 0x2b ] == 0x46 ) ) // number ( emf detection )
                            {
                                p_EscherBlibEntry->meBlibType = EMF;
                            }
                            else
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
            if ( ( eGraphicType == GRAPHIC_BITMAP ) || ( eGraphicType == GRAPHIC_GDIMETAFILE ) )
            {
                sal_uInt32 nErrCode;
                if ( !aGraphic.IsAnimated() )
                    nErrCode = GraphicConverter::Export( aStream, aGraphic, ( eGraphicType == GRAPHIC_BITMAP ) ? ConvertDataFormat::PNG  : ConvertDataFormat::EMF );
                else
                {   // to store a animation, a gif has to be included into the msOG chunk of a png  #I5583#
                    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
                    SvMemoryStream  aGIFStream;
                    const char* pString = "MSOFFICE9.0";
                    aGIFStream.Write( pString, strlen(pString) );
                    nErrCode = rFilter.ExportGraphic( aGraphic, OUString(), aGIFStream,
                        rFilter.GetExportFormatNumberForShortName( "GIF" ) );
                    css::uno::Sequence< css::beans::PropertyValue > aFilterData( 1 );
                    css::uno::Sequence< css::beans::PropertyValue > aAdditionalChunkSequence( 1 );
                    sal_uInt32 nGIFSreamLen = aGIFStream.Tell();
                    css::uno::Sequence< sal_Int8 > aGIFSeq( nGIFSreamLen );
                    sal_Int8* pSeq = aGIFSeq.getArray();
                    aGIFStream.Seek( STREAM_SEEK_TO_BEGIN );
                    aGIFStream.Read( pSeq, nGIFSreamLen );
                    css::beans::PropertyValue aChunkProp, aFilterProp;
                    aChunkProp.Name = "msOG";
                    aChunkProp.Value <<= aGIFSeq;
                    aAdditionalChunkSequence[ 0 ] = aChunkProp;
                    aFilterProp.Name = "AdditionalChunks";
                    aFilterProp.Value <<= aAdditionalChunkSequence;
                    aFilterData[ 0 ] = aFilterProp;
                    nErrCode = rFilter.ExportGraphic( aGraphic, OUString(), aStream,
                        rFilter.GetExportFormatNumberForShortName( "PNG" ), &aFilterData );
                }
                if ( nErrCode == ERRCODE_NONE )
                {
                    p_EscherBlibEntry->meBlibType = ( eGraphicType == GRAPHIC_BITMAP ) ? PNG : EMF;
                    aStream.Seek( STREAM_SEEK_TO_END );
                    p_EscherBlibEntry->mnSize = aStream.Tell();
                    pGraphicAry = static_cast<sal_uInt8 const *>(aStream.GetData());
                }
            }
        }

        ESCHER_BlibType eBlibType = p_EscherBlibEntry->meBlibType;
        if ( p_EscherBlibEntry->mnSize && pGraphicAry && ( eBlibType != UNKNOWN ) )
        {
            sal_uInt32 nExtra, nAtomSize = 0;
            sal_uInt32 nInstance, nUncompressedSize = p_EscherBlibEntry->mnSize;

            if ( mnFlags & _E_GRAPH_PROV_USE_INSTANCES )
            {
                rPicOutStrm.WriteUInt32( 0x7f90000 | (sal_uInt16)( mnBlibEntrys << 4 ) )
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
                rPicOutStrm.Write( p_EscherBlibEntry->mnIdentifier, 16 );
                rPicOutStrm.WriteUChar( 0xff );
                rPicOutStrm.Write( pGraphicAry, p_EscherBlibEntry->mnSize );
            }
            else
            {
                ZCodec aZCodec( 0x8000, 0x8000 );
                aZCodec.BeginCompression();
                SvMemoryStream aDestStrm;
                aZCodec.Write( aDestStrm, pGraphicAry, p_EscherBlibEntry->mnSize );
                aZCodec.EndCompression();
                aDestStrm.Seek( STREAM_SEEK_TO_END );
                p_EscherBlibEntry->mnSize = aDestStrm.Tell();
                pGraphicAry = static_cast<sal_uInt8 const *>(aDestStrm.GetData());
                if ( p_EscherBlibEntry->mnSize && pGraphicAry )
                {
                    nExtra = eBlibType == WMF ? 0x42 : 0x32;                                    // !EMF -> no change
                    p_EscherBlibEntry->mnSizeExtra = nExtra + 8;
                    nInstance = ( eBlibType == WMF ) ? 0xf01b2170 : 0xf01a3d40;                 // !EMF -> no change
                    rPicOutStrm.WriteUInt32( nInstance ).WriteUInt32( p_EscherBlibEntry->mnSize + nExtra );
                    if ( eBlibType == WMF )                                                     // !EMF -> no change
                        rPicOutStrm.Write( p_EscherBlibEntry->mnIdentifier, 16 );
                    rPicOutStrm.Write( p_EscherBlibEntry->mnIdentifier, 16 );

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
                    rPicOutStrm.Write( pGraphicAry, p_EscherBlibEntry->mnSize );
                }
            }
            if ( nAtomSize )
            {
                sal_uInt32  nPos = rPicOutStrm.Tell();
                rPicOutStrm.Seek( nAtomSize - 4 );
                rPicOutStrm.WriteUInt32( nPos - nAtomSize );
                rPicOutStrm.Seek( nPos );
            }
            nBlibId = ImplInsertBlib( p_EscherBlibEntry ), p_EscherBlibEntry = nullptr;
        }
    }
    delete p_EscherBlibEntry;
    return nBlibId;
}

struct EscherConnectorRule
{
    sal_uInt32  nRuleId;
    sal_uInt32  nShapeA;        // SPID of shape A
    sal_uInt32  nShapeB;        // SPID of shape B
    sal_uInt32  nShapeC;        // SPID of connector shape
    sal_uInt32  ncptiA;         // Connection site Index of shape A
    sal_uInt32  ncptiB;         // Connection site Index of shape B
};

struct EscherShapeListEntry
{
    css::uno::Reference< css::drawing::XShape >   aXShape;
    sal_uInt32          n_EscherId;

                        EscherShapeListEntry( const css::uno::Reference
                            < css::drawing::XShape > & rShape, sal_uInt32 nId ) :
                                        aXShape     ( rShape ),
                                        n_EscherId  ( nId ) {}
};

sal_uInt32 EscherConnectorListEntry::GetClosestPoint( const tools::Polygon& rPoly, const css::awt::Point& rPoint )
{
    sal_uInt16 nCount = rPoly.GetSize();
    sal_uInt16 nClosest = nCount;
    double fDist = (sal_uInt32)0xffffffff;
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
// nRule =  0 ->Top         0 ->Top         nRule = Index auf ein (Poly)Polygon Punkt
//          1 ->Left        2 ->Left
//          2 ->Bottom      4 ->Bottom
//          3 ->Right       6 ->Right

sal_uInt32 EscherConnectorListEntry::GetConnectorRule( bool bFirst )
{
    sal_uInt32 nRule = 0;

    css::uno::Any aAny;
    css::awt::Point aRefPoint( ( bFirst ) ? maPointA : maPointB );
    css::uno::Reference< css::drawing::XShape >
        aXShape( ( bFirst ) ? mXConnectToA : mXConnectToB );

    OUString aString(aXShape->getShapeType());
    OStringBuffer aBuf(OUStringToOString(aString, RTL_TEXTENCODING_UTF8));
    aBuf.remove( 0, 13 );   // removing "com.sun.star."
    sal_Int16 nPos = aBuf.toString().indexOf("Shape");
    aBuf.remove(nPos, 5);
    OString aType = aBuf.makeStringAndClear();

    css::uno::Reference< css::beans::XPropertySet >
        aPropertySet( aXShape, css::uno::UNO_QUERY );

    if ((aType == OString( "drawing.PolyPolygon" )) || (aType == OString( "drawing.PolyLine"  )))
    {
        if ( aPropertySet.is() )
        {
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aPropertySet, "PolyPolygon" ) )
            {
                css::drawing::PointSequenceSequence const * pSourcePolyPolygon =
                    static_cast<css::drawing::PointSequenceSequence const *>(aAny.getValue());
                sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->getLength();
                css::drawing::PointSequence const * pOuterSequence = pSourcePolyPolygon->getConstArray();

                if ( pOuterSequence )
                {
                    sal_Int32 a, b, nIndex = 0;
                    sal_uInt32 nDistance = 0xffffffff;
                    for( a = 0; a < nOuterSequenceCount; a++ )
                    {
                        css::drawing::PointSequence const * pInnerSequence = pOuterSequence++;
                        if ( pInnerSequence )
                        {
                            css::awt::Point const * pArray = pInnerSequence->getConstArray();
                            if ( pArray )
                            {
                                for ( b = 0; b < pInnerSequence->getLength(); b++, nIndex++, pArray++ )
                                {
                                    sal_uInt32 nDist = (sal_uInt32)hypot( aRefPoint.X - pArray->X, aRefPoint.Y - pArray->Y );
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
    else if ((aType == OString( "drawing.OpenBezier" )) || (aType == OString( "drawing.OpenFreeHand" )) || (aType == OString( "drawing.PolyLinePath" ))
        || (aType == OString( "drawing.ClosedBezier" )) || ( aType == OString( "drawing.ClosedFreeHand" )) || (aType == OString( "drawing.PolyPolygonPath" )) )
    {
        css::uno::Reference< css::beans::XPropertySet >
            aPropertySet2( aXShape, css::uno::UNO_QUERY );
        if ( aPropertySet2.is() )
        {
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aPropertySet2, "PolyPolygonBezier" ) )
            {
                css::drawing::PolyPolygonBezierCoords const * pSourcePolyPolygon =
                    static_cast<css::drawing::PolyPolygonBezierCoords const *>(aAny.getValue());
                sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->Coordinates.getLength();

                // Zeiger auf innere sequences holen
                css::drawing::PointSequence const * pOuterSequence =
                    pSourcePolyPolygon->Coordinates.getConstArray();
                css::drawing::FlagSequence const *  pOuterFlags =
                    pSourcePolyPolygon->Flags.getConstArray();

                if ( pOuterSequence && pOuterFlags )
                {
                    sal_Int32 a, b, nIndex = 0;
                    sal_uInt32 nDistance = 0xffffffff;

                    for ( a = 0; a < nOuterSequenceCount; a++ )
                    {
                        css::drawing::PointSequence const * pInnerSequence = pOuterSequence++;
                        css::drawing::FlagSequence const *  pInnerFlags = pOuterFlags++;
                        if ( pInnerSequence && pInnerFlags )
                        {
                            css::awt::Point const * pArray = pInnerSequence->getConstArray();
                            css::drawing::PolygonFlags const * pFlags = pInnerFlags->getConstArray();
                            if ( pArray && pFlags )
                            {
                                for ( b = 0; b < pInnerSequence->getLength(); b++, pArray++ )
                                {
                                    css::drawing::PolygonFlags ePolyFlags = *pFlags++;
                                    if ( ePolyFlags == css::drawing::PolygonFlags_CONTROL )
                                        continue;
                                    sal_uInt32 nDist = (sal_uInt32)hypot( aRefPoint.X - pArray->X, aRefPoint.Y - pArray->Y );
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
            SdrObject* pCustoShape( GetSdrObjectFromXShape( aXShape ) );
            if ( dynamic_cast<const SdrObjCustomShape* >(pCustoShape) !=  nullptr )
            {
                const SdrCustomShapeGeometryItem& rGeometryItem = static_cast<const SdrCustomShapeGeometryItem&>(
                    pCustoShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));

                const OUString sPath( "Path"  );
                const OUString sType( "Type"  );
                const OUString sGluePointType( "GluePointType"  );

                OUString sShapeType;
                const uno::Any* pType = rGeometryItem.GetPropertyValueByName( sType );
                if ( pType )
                    *pType >>= sShapeType;
                MSO_SPT eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

                uno::Any* pGluePointType = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sGluePointType );

                sal_Int16 nGluePointType = sal_Int16();
                if ( !( pGluePointType &&
                        ( *pGluePointType >>= nGluePointType ) ) )
                    nGluePointType = GetCustomShapeConnectionTypeDefault( eSpType );

                if ( nGluePointType == css::drawing::EnhancedCustomShapeGluePointType::CUSTOM )
                {
                    const SdrGluePointList* pList = pCustoShape->GetGluePointList();
                    if ( pList )
                    {
                        tools::Polygon aPoly;
                        sal_uInt16 nNum, nAnz = pList->GetCount();
                        if ( nAnz )
                        {
                            for ( nNum = 0; nNum < nAnz; nNum++ )
                            {
                                const SdrGluePoint& rGP = (*pList)[ nNum ];
                                Point aPt( rGP.GetAbsolutePos( *pCustoShape ) );
                                aPoly.Insert( POLY_APPEND, aPt );
                            }
                            nRule = GetClosestPoint( aPoly, aRefPoint );
                            bRectangularConnection = false;
                        }
                    }
                }
                else if ( nGluePointType == css::drawing::EnhancedCustomShapeGluePointType::SEGMENTS )
                {
                    SdrObject* pPoly = pCustoShape->DoConvertToPolyObj( true, true );
                    if ( dynamic_cast<const SdrPathObj* >( pPoly ) !=  nullptr )
                    {
                        sal_Int16 a, b, nIndex = 0;
                        sal_uInt32 nDistance = 0xffffffff;

                        // #i74631# use explicit constructor here. Also XPolyPolygon is not necessary,
                        // reducing to PolyPolygon
                        const tools::PolyPolygon aPolyPoly(static_cast<SdrPathObj*>(pPoly)->GetPathPoly());

                        for ( a = 0; a < aPolyPoly.Count(); a++ )
                        {
                            const tools::Polygon& rPoly = aPolyPoly.GetObject( a );
                            for ( b = 0; b < rPoly.GetSize(); b++ )
                            {
                                if ( rPoly.GetFlags( b ) != POLY_NORMAL )
                                    continue;
                                const Point& rPt = rPoly[ b ];
                                sal_uInt32 nDist = (sal_uInt32)hypot( aRefPoint.X - rPt.X(), aRefPoint.Y - rPt.Y() );
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
            css::awt::Point aPoint( aXShape->getPosition() );
            css::awt::Size  aSize( aXShape->getSize() );

            Rectangle aRect( Point( aPoint.X, aPoint.Y ), Size( aSize.Width, aSize.Height ) );
            Point aCenter( aRect.Center() );
            tools::Polygon aPoly( 4 );

            aPoly[ 0 ] = Point( aCenter.X(), aRect.Top() );
            aPoly[ 1 ] = Point( aRect.Left(), aCenter.Y() );
            aPoly[ 2 ] = Point( aCenter.X(), aRect.Bottom() );
            aPoly[ 3 ] = Point( aRect.Right(), aCenter.Y() );

            sal_Int32 nAngle = ( EscherPropertyValueHelper::GetPropertyValue( aAny, aPropertySet, "RotateAngle", true ) )
                    ? *static_cast<sal_Int32 const *>(aAny.getValue()) : 0;
            if ( nAngle )
                aPoly.Rotate( aRect.TopLeft(), (sal_uInt16)( ( nAngle + 5 ) / 10 ) );
            nRule = GetClosestPoint( aPoly, aRefPoint );

            if (aType == OString( "drawing.Ellipse" ))
                nRule <<= 1;    // In PPT an ellipse has 8 ways to connect
        }
    }
    return nRule;
}

EscherSolverContainer::~EscherSolverContainer()
{
    for( size_t i = 0, n = maShapeList.size(); i < n; ++i ) {
        delete maShapeList[ i ];
    }
    for( size_t i = 0, n = maConnectorList.size(); i < n; ++i ) {
        delete maConnectorList[ i ];
    }
}

void EscherSolverContainer::AddShape( const css::uno::Reference< css::drawing::XShape > & rXShape, sal_uInt32 nId )
{
    maShapeList.push_back( new EscherShapeListEntry( rXShape, nId ) );
}

void EscherSolverContainer::AddConnector(
    const css::uno::Reference< css::drawing::XShape > & rConnector,
    const css::awt::Point& rPA,
    css::uno::Reference< css::drawing::XShape > & rConA,
    const css::awt::Point& rPB,
    css::uno::Reference< css::drawing::XShape > & rConB
)
{
    maConnectorList.push_back( new EscherConnectorListEntry( rConnector, rPA, rConA, rPB, rConB ) );
}

sal_uInt32 EscherSolverContainer::GetShapeId( const css::uno::Reference< css::drawing::XShape > & rXShape ) const
{
    for ( size_t i = 0, n = maShapeList.size(); i < n; ++i )
    {
        EscherShapeListEntry* pPtr = maShapeList[ i ];
        if ( rXShape == pPtr->aXShape )
            return ( pPtr->n_EscherId );
    }
    return 0;
}

void EscherSolverContainer::WriteSolver( SvStream& rStrm )
{
    sal_uInt32 nCount = maConnectorList.size();
    if ( nCount )
    {
        sal_uInt32  nRecHdPos, nCurrentPos, nSize;
        rStrm  .WriteUInt16( ( nCount << 4 ) | 0xf )    // open an ESCHER_SolverContainer
               .WriteUInt16( ESCHER_SolverContainer )
               .WriteUInt32( 0 );

        nRecHdPos = rStrm.Tell() - 4;

        EscherConnectorRule aConnectorRule;
        aConnectorRule.nRuleId = 2;
        for ( size_t i = 0, n = maConnectorList.size(); i < n; ++i )
        {
            EscherConnectorListEntry* pPtr = maConnectorList[ i ];
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
}

EscherExGlobal::EscherExGlobal( sal_uInt32 nGraphicProvFlags ) :
    EscherGraphicProvider( nGraphicProvFlags ),
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
    maClusterTable.push_back( ClusterEntry( nDrawingId ) );
    maDrawingInfos.push_back( DrawingInfo( nClusterId ) );
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
        maClusterTable.push_back( ClusterEntry( nDrawingId ) );
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
    for( DrawingInfoVector::const_iterator aIt = maDrawingInfos.begin(), aEnd = maDrawingInfos.end(); aIt != aEnd; ++aIt )
    {
        nShapeCount += aIt->mnShapeCount;
        nLastShapeId = ::std::max( nLastShapeId, aIt->mnLastShapeId );
    }
    // the non-existing cluster with index #0 is counted too
    sal_uInt32 nClusterCount = static_cast< sal_uInt32 >( maClusterTable.size() + 1 );
    sal_uInt32 nDrawingCount = static_cast< sal_uInt32 >( maDrawingInfos.size() );
    rStrm.WriteUInt32( nLastShapeId ).WriteUInt32( nClusterCount ).WriteUInt32( nShapeCount ).WriteUInt32( nDrawingCount );

    // write the cluster table
    for( ClusterTable::const_iterator aIt = maClusterTable.begin(), aEnd = maClusterTable.end(); aIt != aEnd; ++aIt )
        rStrm.WriteUInt32( aIt->mnDrawingId ).WriteUInt32( aIt->mnNextShapeId );
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

// Implementation of an empty stream that silently succeeds, but does nothing.
//
// In fact, this is a hack.  The right solution is to abstract EscherEx to be
// able to work without SvStream; but at the moment it is better to live with
// this I guess.
class SvNullStream : public SvStream
{
protected:
    virtual sal_Size GetData( void* pData, sal_Size nSize ) override { memset( pData, 0, nSize ); return nSize; }
    virtual sal_Size PutData( const void*, sal_Size nSize ) override { return nSize; }
    virtual sal_uInt64 SeekPos( sal_uInt64 nPos ) override { return nPos; }
    virtual void SetSize( sal_uInt64 ) override {}
    virtual void FlushData() override {}

public:
    SvNullStream() : SvStream() {}
    virtual ~SvNullStream() {}
};

EscherEx::EscherEx(const EscherExGlobalRef& rxGlobal, SvStream* pOutStrm, bool bOOXML)
    : mxGlobal(rxGlobal)
    , mpOutStrm(pOutStrm)
    , mbOwnsStrm(false)
    , mnCurrentDg(0)
    , mnCountOfs(0)
    , mnGroupLevel(0)
    , mnHellLayerId(USHRT_MAX)
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
    mpImplEscherExSdr.reset( new ImplEscherExSdr( *this ) );
}

EscherEx::~EscherEx()
{
    if (mbOwnsStrm)
        delete mpOutStrm;
}

void EscherEx::Flush( SvStream* pPicStreamMergeBSE /* = NULL */ )
{
    if ( mxGlobal->HasDggContainer() )
    {
        // store the current stream position at ESCHER_Persist_CurrentPosition key
        PtReplaceOrInsert( ESCHER_Persist_CurrentPosition, mpOutStrm->Tell() );
        if ( DoSeek( ESCHER_Persist_Dgg ) )
        {
            /*  The DGG record is still not written. ESCHER_Persist_Dgg seeks
                to the place where the complete record has to be inserted. */
            InsertAtCurrentPos( mxGlobal->GetDggAtomSize(), false );
            mxGlobal->WriteDggAtom( *mpOutStrm );

            if ( mxGlobal->HasGraphics() )
            {
                /*  Calculate the total size of the BSTORECONTAINER including
                    all BSE records containing the picture data contained in
                    the passed in pPicStreamMergeBSE. */
                sal_uInt32 nBSCSize = mxGlobal->GetBlibStoreContainerSize( pPicStreamMergeBSE );
                if ( nBSCSize > 0 )
                {
                    InsertAtCurrentPos( nBSCSize, false );
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
}

void EscherEx::InsertAtCurrentPos( sal_uInt32 nBytes, bool bExpandEndOfAtom )
{
    sal_uInt32  nSize, nType, nSource, nBufSize, nToCopy, nCurPos = mpOutStrm->Tell();

    // adjust persist table
    for( size_t i = 0, n = maPersistTable.size(); i < n; ++i ) {
        EscherPersistEntry* pPtr = maPersistTable[ i ];
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
        if ( (nCurPos < nEndOfRecord) || ((nCurPos == nEndOfRecord) && (bContainer || bExpandEndOfAtom)) )
        {
            mpOutStrm->SeekRel( -4 );
            mpOutStrm->WriteUInt32( nSize + nBytes );
            if ( !bContainer )
                mpOutStrm->SeekRel( nSize );
        }
        else
            mpOutStrm->SeekRel( nSize );
    }
    for (std::vector< sal_uInt32 >::iterator aIter( mOffsets.begin() ), aEnd( mOffsets.end() ); aIter != aEnd ; ++aIter)
    {
        if ( *aIter > nCurPos )
            *aIter += nBytes;
    }
    mpOutStrm->Seek( STREAM_SEEK_TO_END );
    nSource = mpOutStrm->Tell();
    nToCopy = nSource - nCurPos;                        // increase the size of the tream by nBytes
    std::unique_ptr<sal_uInt8[]> pBuf(new sal_uInt8[ 0x40000 ]); // 256KB Buffer
    while ( nToCopy )
    {
        nBufSize = ( nToCopy >= 0x40000 ) ? 0x40000 : nToCopy;
        nToCopy -= nBufSize;
        nSource -= nBufSize;
        mpOutStrm->Seek( nSource );
        mpOutStrm->Read( pBuf.get(), nBufSize );
        mpOutStrm->Seek( nSource + nBytes );
        mpOutStrm->Write( pBuf.get(), nBufSize );
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

bool EscherEx::InsertAtPersistOffset( sal_uInt32 nKey, sal_uInt32 nValue )
{
    sal_uInt32  nOldPos = mpOutStrm->Tell();
    bool        bRetValue = SeekToPersistOffset( nKey );
    if ( bRetValue )
    {
        mpOutStrm->WriteUInt32( nValue );
        mpOutStrm->Seek( nOldPos );
    }
    return bRetValue;
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
    mpOutStrm->WriteUInt32( 0 ).WriteUInt32( 0 );       // record header wird spaeter geschrieben
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

void EscherEx::AddChildAnchor( const Rectangle& rRect )
{
    AddAtom( 16, ESCHER_ChildAnchor );
    mpOutStrm ->WriteInt32( rRect.Left() )
               .WriteInt32( rRect.Top() )
               .WriteInt32( rRect.Right() )
               .WriteInt32( rRect.Bottom() );
}

void EscherEx::AddClientAnchor( const Rectangle& rRect )
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

sal_uInt32 EscherEx::EnterGroup( const OUString& rShapeName, const Rectangle* pBoundRect )
{
    Rectangle aRect;
    if( pBoundRect )
        aRect = *pBoundRect;

    OpenContainer( ESCHER_SpgrContainer );
    OpenContainer( ESCHER_SpContainer );
    AddAtom( 16, ESCHER_Spgr, 1 );
    PtReplaceOrInsert( ESCHER_Persist_Grouping_Snap | mnGroupLevel,
                        mpOutStrm->Tell() );
    mpOutStrm ->WriteInt32( aRect.Left() )  // Bounding box for the grouped shapes to which they will be attached
               .WriteInt32( aRect.Top() )
               .WriteInt32( aRect.Right() )
               .WriteInt32( aRect.Bottom() );

    sal_uInt32 nShapeId = GenerateShapeId();
    if ( !mnGroupLevel )
        AddShape( ESCHER_ShpInst_Min, 5, nShapeId );                    // Flags: Group | Patriarch
    else
    {
        AddShape( ESCHER_ShpInst_Min, 0x201, nShapeId );                // Flags: Group | HaveAnchor
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

        EscherExHostAppData* pAppData = mpImplEscherExSdr->ImplGetHostData();
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

sal_uInt32 EscherEx::EnterGroup( const Rectangle* pBoundRect )
{
    return EnterGroup( OUString(), pBoundRect );
}

bool EscherEx::SetGroupSnapRect( sal_uInt32 nGroupLevel, const Rectangle& rRect )
{
    bool bRetValue = false;
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
    return bRetValue;
}

bool EscherEx::SetGroupLogicRect( sal_uInt32 nGroupLevel, const Rectangle& rRect )
{
    bool bRetValue = false;
    if ( nGroupLevel )
    {
        sal_uInt32 nCurrentPos = mpOutStrm->Tell();
        if ( DoSeek( ESCHER_Persist_Grouping_Logic | ( nGroupLevel - 1 ) ) )
        {
            mpOutStrm->WriteInt16( rRect.Top() ).WriteInt16( rRect.Left() ).WriteInt16( rRect.Right() ).WriteInt16( rRect.Bottom() );
            mpOutStrm->Seek( nCurrentPos );
        }
    }
    return bRetValue;
}

void EscherEx::LeaveGroup()
{
    --mnGroupLevel;
    PtDelete( ESCHER_Persist_Grouping_Snap | mnGroupLevel );
    PtDelete( ESCHER_Persist_Grouping_Logic | mnGroupLevel );
    CloseContainer();
}

void EscherEx::AddShape( sal_uInt32 nShpInstance, sal_uInt32 nFlags, sal_uInt32 nShapeID )
{
    AddAtom( 8, ESCHER_Sp, 2, nShpInstance );

    if ( !nShapeID )
        nShapeID = GenerateShapeId();

    if ( nFlags ^ 1 )                           // is this a group shape ?
    {                                           // if not
        if ( mnGroupLevel > 1 )
            nFlags |= 2;                        // this not a topmost shape
    }
    mpOutStrm->WriteUInt32( nShapeID ).WriteUInt32( nFlags );
}

void EscherEx::Commit( EscherPropertyContainer& rProps, const Rectangle& )
{
    rProps.Commit( GetStream() );
}

sal_uInt32 EscherEx::GetColor( const sal_uInt32 nSOColor, bool bSwap )
{
    if ( bSwap )
    {
        sal_uInt32 nColor = nSOColor & 0xff00;          // Green
        nColor |= (sal_uInt8)( nSOColor ) << 16;        // Red
        nColor |= (sal_uInt8)( nSOColor >> 16 );        // Blue
        return nColor;
    }
    else
        return nSOColor & 0xffffff;
}

sal_uInt32 EscherEx::GetColor( const Color& rSOColor, bool bSwap )
{
    sal_uInt32 nColor = ( rSOColor.GetRed() << 16 );
    nColor |= ( rSOColor.GetGreen() << 8 );
    nColor |= rSOColor.GetBlue();

    if ( !bSwap )
        nColor = GetColor( nColor );

    return nColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
