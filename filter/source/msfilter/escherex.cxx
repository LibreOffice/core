/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "eschesdo.hxx"
#include <filter/msfilter/escherex.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdmodel.hxx>
#include <vcl/gradient.hxx>
#include <vcl/graph.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/stream.hxx>
#include <tools/zcodec.hxx>
#include <svx/svdopath.hxx>
#include <stdlib.h>
#include <svtools/filter.hxx>
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
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
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
#include <comphelper/extract.hxx>
#include <comphelper/string.hxx>
#include <toolkit/unohlp.hxx>
#include <vcl/virdev.hxx>
#include <rtl/crc.h>
#include <rtl/strbuf.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;


EscherExContainer::EscherExContainer( SvStream& rSt, const sal_uInt16 nRecType, const sal_uInt16 nInstance ) :
    rStrm   ( rSt )
{
    rStrm << (sal_uInt32)( ( 0xf | ( nInstance << 4 ) ) | ( nRecType << 16 ) ) << (sal_uInt32)0;
    nContPos = rStrm.Tell();
}
EscherExContainer::~EscherExContainer()
{
    sal_uInt32 nPos = rStrm.Tell();
    sal_uInt32 nSize= nPos - nContPos;
    if ( nSize )
    {
        rStrm.Seek( nContPos - 4 );
        rStrm << nSize;
        rStrm.Seek( nPos );
    }
}

EscherExAtom::EscherExAtom( SvStream& rSt, const sal_uInt16 nRecType, const sal_uInt16 nInstance, const sal_uInt8 nVersion ) :
    rStrm   ( rSt )
{
    rStrm << (sal_uInt32)( ( nVersion | ( nInstance << 4 ) ) | ( nRecType << 16 ) ) << (sal_uInt32)0;
    nContPos = rStrm.Tell();
}
EscherExAtom::~EscherExAtom()
{
    sal_uInt32 nPos = rStrm.Tell();
    sal_uInt32 nSize= nPos - nContPos;
    if ( nSize )
    {
        rStrm.Seek( nContPos - 4 );
        rStrm << nSize;
        rStrm.Seek( nPos );
    }
}

EscherExClientRecord_Base::~EscherExClientRecord_Base()
{
}

EscherExClientAnchor_Base::~EscherExClientAnchor_Base()
{
}

void EscherPropertyContainer::ImplInit()
{
    nSortCount = 0;
    nCountCount = 0;
    nCountSize = 0;
    nSortBufSize = 64;
    bHasComplexData = sal_False;
    bSuppressRotation = sal_False;
    pSortStruct = new EscherPropSortStruct[ nSortBufSize ];
}

EscherPropertyContainer::EscherPropertyContainer() :
    pGraphicProvider    ( NULL ),
    pPicOutStrm         ( NULL )
{
    ImplInit();
};

EscherPropertyContainer::EscherPropertyContainer(
    EscherGraphicProvider& rGraphProv,
            SvStream* pPiOutStrm,
                Rectangle& rBoundRect ) :

    pGraphicProvider    ( &rGraphProv ),
    pPicOutStrm         ( pPiOutStrm ),
    pShapeBoundRect     ( &rBoundRect )
{
    ImplInit();
}

EscherPropertyContainer::~EscherPropertyContainer()
{
    if ( bHasComplexData )
    {
        while ( nSortCount-- )
            delete[] pSortStruct[ nSortCount ].pBuf;
    }
    delete[] pSortStruct;
};

void EscherPropertyContainer::AddOpt( sal_uInt16 nPropID, sal_uInt32 nPropValue, sal_Bool bBlib )
{
    AddOpt( nPropID, bBlib, nPropValue, NULL, 0 );
}

void EscherPropertyContainer::AddOpt( sal_uInt16 nPropID, const rtl::OUString& rString )
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
    AddOpt( nPropID, sal_True, nLen, pBuf, nLen );
}

void EscherPropertyContainer::AddOpt( sal_uInt16 nPropID, sal_Bool bBlib, sal_uInt32 nPropValue, sal_uInt8* pProp, sal_uInt32 nPropSize )
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
        delete pSortStruct;
        pSortStruct = pTemp;
    }
    pSortStruct[ nSortCount ].nPropId = nPropID;                                // insert property
    pSortStruct[ nSortCount ].pBuf = pProp;
    pSortStruct[ nSortCount ].nPropSize = nPropSize;
    pSortStruct[ nSortCount++ ].nPropValue = nPropValue;

    if ( pProp )
    {
        nCountSize += nPropSize;
        bHasComplexData = sal_True;
    }
}

sal_Bool EscherPropertyContainer::GetOpt( sal_uInt16 nPropId, sal_uInt32& rPropValue ) const
{
    EscherPropSortStruct aPropStruct;

    if ( GetOpt( nPropId, aPropStruct ) )
    {
        rPropValue = aPropStruct.nPropValue;
        return sal_True;
    }
    return sal_False;
}

sal_Bool EscherPropertyContainer::GetOpt( sal_uInt16 nPropId, EscherPropSortStruct& rPropValue ) const
{
    for( sal_uInt32 i = 0; i < nSortCount; i++ )
    {
        if ( ( pSortStruct[ i ].nPropId &~0xc000 ) == ( nPropId &~0xc000 ) )
        {
            rPropValue = pSortStruct[ i ];
            return sal_True;
        }
    }
    return sal_False;
}

EscherProperties EscherPropertyContainer::GetOpts() const
{
    EscherProperties aVector;

    for ( sal_uInt32 i = 0; i < nSortCount; ++i )
        aVector.push_back( pSortStruct[ i ] );

    return aVector;
}

extern "C" int __LOADONCALLAPI EscherPropSortFunc( const void* p1, const void* p2 )
{
    sal_Int16   nID1 = ((EscherPropSortStruct*)p1)->nPropId &~0xc000;
    sal_Int16   nID2 = ((EscherPropSortStruct*)p2)->nPropId &~0xc000;

    if( nID1  < nID2 )
        return -1;
    else if( nID1 > nID2 )
        return 1;
    else
        return 0;
}

void EscherPropertyContainer::Commit( SvStream& rSt, sal_uInt16 nVersion, sal_uInt16 nRecType )
{
    rSt << (sal_uInt16)( ( nCountCount << 4 ) | ( nVersion & 0xf ) ) << nRecType << nCountSize;
    if ( nSortCount )
    {
        qsort( pSortStruct, nSortCount, sizeof( EscherPropSortStruct ), EscherPropSortFunc );
        sal_uInt32 i;

        for ( i = 0; i < nSortCount; i++ )
        {
            sal_uInt32 nPropValue = pSortStruct[ i ].nPropValue;
            sal_uInt16 nPropId = pSortStruct[ i ].nPropId;

            if ( bSuppressRotation && ( nPropId == ESCHER_Prop_Rotation ) )
                nPropValue = 0;

            rSt << nPropId
                << nPropValue;
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

sal_Bool EscherPropertyContainer::IsFontWork() const
{
    sal_uInt32 nTextPathFlags = 0;
    GetOpt( DFF_Prop_gtextFStrikethrough, nTextPathFlags );
    return ( nTextPathFlags & 0x4000 ) != 0;
}

sal_uInt32 EscherPropertyContainer::ImplGetColor( const sal_uInt32 nSOColor, sal_Bool bSwap )
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
    const ::com::sun::star::awt::Gradient* pGradient,
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
    sal_uInt32  nRed = ( ( aColor.GetRed() * nIntensity ) / 100 );
    sal_uInt32  nGreen = ( ( aColor.GetGreen() * nIntensity ) / 100 ) << 8;
    sal_uInt32  nBlue = ( ( aColor.GetBlue() * nIntensity ) / 100 ) << 16;
    return nRed | nGreen | nBlue;
}

void EscherPropertyContainer::CreateGradientProperties(
    const ::com::sun::star::awt::Gradient & rGradient )
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
        case ::com::sun::star::awt::GradientStyle_LINEAR :
        case ::com::sun::star::awt::GradientStyle_AXIAL :
        {
            nFillType = ESCHER_FillShadeScale;
            nAngle = (rGradient.Angle * 0x10000) / 10;
            nFillFocus = (sal::static_int_cast<int>(rGradient.Style) ==
                          sal::static_int_cast<int>(GradientStyle_LINEAR)) ? 0 : 50;
        }
        break;
        case ::com::sun::star::awt::GradientStyle_RADIAL :
        case ::com::sun::star::awt::GradientStyle_ELLIPTICAL :
        case ::com::sun::star::awt::GradientStyle_SQUARE :
        case ::com::sun::star::awt::GradientStyle_RECT :
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
        case ::com::sun::star::awt::GradientStyle_MAKE_FIXED_SIZE : break;
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
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet )
{
    ::com::sun::star::uno::Any aAny;
    ::com::sun::star::awt::Gradient aGradient;
    if ( EscherPropertyValueHelper::GetPropertyValue(
            aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillGradient" ) ), sal_False ) )
    {
        aGradient = *static_cast< const ::com::sun::star::awt::Gradient* >( aAny.getValue() );
    }
    CreateGradientProperties( aGradient );
};

void EscherPropertyContainer::CreateFillProperties(
    const uno::Reference< beans::XPropertySet > & rXPropSet,
        sal_Bool bEdge )
{
    ::com::sun::star::uno::Any aAny;
    AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapNone );
    AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );
    const rtl::OUString aPropName( String( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ) ) );

    if ( EscherPropertyValueHelper::GetPropertyValue(
            aAny, rXPropSet, aPropName, sal_False ) )
    {
        ::com::sun::star::drawing::FillStyle eFS;
        if ( ! ( aAny >>= eFS ) )
            eFS = ::com::sun::star::drawing::FillStyle_SOLID;
        sal_uInt32 nFillBackColor = 0;
        switch( eFS )
        {
            case ::com::sun::star::drawing::FillStyle_GRADIENT :
            {
                CreateGradientProperties( rXPropSet );
                AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
            }
            break;

            case ::com::sun::star::drawing::FillStyle_BITMAP :
            {
                CreateGraphicProperties( rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillBitmapURL" ) ), sal_True );
                AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
                AddOpt( ESCHER_Prop_fillBackColor, nFillBackColor  );
            }
            break;
            case ::com::sun::star::drawing::FillStyle_HATCH :
            {
                CreateGraphicProperties( rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillHatch" ) ), sal_True );
            }
            break;
            case ::com::sun::star::drawing::FillStyle_SOLID :
            default:
            {
                ::com::sun::star::beans::PropertyState ePropState = EscherPropertyValueHelper::GetPropertyState(
                    rXPropSet, aPropName );
                if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
                    AddOpt( ESCHER_Prop_fillType, ESCHER_FillSolid );

                if ( EscherPropertyValueHelper::GetPropertyValue(
                        aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ), sal_False ) )
                {
                    sal_uInt32 nFillColor = ImplGetColor( *((sal_uInt32*)aAny.getValue()) );
                    nFillBackColor = nFillColor ^ 0xffffff;
                    AddOpt( ESCHER_Prop_fillColor, nFillColor );
                }
                AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100010 );
                AddOpt( ESCHER_Prop_fillBackColor, nFillBackColor );
                break;
            }
            case ::com::sun::star::drawing::FillStyle_NONE :
                AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
            break;
        }
        if ( eFS != ::com::sun::star::drawing::FillStyle_NONE )
        {
            sal_uInt16 nTransparency = ( EscherPropertyValueHelper::GetPropertyValue(
                                    aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillTransparence" ) ), sal_True ) )
                                    ? *((sal_Int16*)aAny.getValue() )
                                    : 0;
            if (  nTransparency )
                AddOpt( ESCHER_Prop_fillOpacity, ( ( 100 - nTransparency ) << 16 ) / 100 );
        }
    }
    CreateLineProperties( rXPropSet, bEdge );
}

void EscherPropertyContainer::CreateTextProperties(
    const uno::Reference< beans::XPropertySet > & rXPropSet, sal_uInt32 nTextId,
        const sal_Bool bIsCustomShape, const sal_Bool bIsTextFrame )
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
    sal_Bool bAutoGrowWidth     ( sal_False );
    sal_Bool bAutoGrowHeight    ( sal_False );
    // used with ashapes:
    sal_Bool bWordWrap          ( sal_False );
    sal_Bool bAutoGrowSize      ( sal_False );

    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextWritingMode" ) ), sal_True ) )
        aAny >>= eWM;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextVerticalAdjust" ) ), sal_True ) )
        aAny >>= eVA;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextHorizontalAdjust" ) ), sal_True ) )
        aAny >>= eHA;
    if ( bIsCustomShape )
    {
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextWordWrap" ) ), sal_False ) )
            aAny >>= bWordWrap;
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextAutoGrowHeight" ) ), sal_True ) )
            aAny >>= bAutoGrowSize;
    }
    else if ( bIsTextFrame )
    {
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextAutoGrowWidth" ) ), sal_True ) )
            aAny >>= bAutoGrowWidth;

// i63936 not setting autogrowheight, because otherwise
// the minframeheight of the text will be ignored
//
//      if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextAutoGrowHeight" ) ), sal_True ) )
//          aAny >>= bAutoGrowHeight;
    }
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextLeftDistance" ) ) ) )
        aAny >>= nLeft;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextUpperDistance" ) ) ) )
        aAny >>= nTop;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextRightDistance" ) ) ) )
        aAny >>= nRight;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextLowerDistance" ) ) ) )
        aAny >>= nBottom;

    ESCHER_AnchorText eAnchor = ESCHER_AnchorTop;
    ESCHER_WrapMode eWrapMode = ESCHER_WrapSquare;
    sal_uInt32 nTextAttr = 0x40004;     // rotate text with shape

    if ( eWM == text::WritingMode_TB_RL )
    {   // verical writing
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
    if (bIsTextFrame) {
        sal_uInt16 nAngle = EscherPropertyValueHelper::GetPropertyValue(
        aAny,
        rXPropSet,
        String( RTL_CONSTASCII_USTRINGPARAM( "RotateAngle" ) ),
        sal_True )
        ? (sal_uInt16)( ( *((sal_Int32*)aAny.getValue() ) ) + 5 ) / 10 : 0;
        if (nAngle==900) {
        AddOpt( ESCHER_Prop_txflTextFlow, 1 );
        bSuppressRotation=true;
        }
        if (nAngle==1800) {
        AddOpt( ESCHER_Prop_txflTextFlow, 2 );
        bSuppressRotation=true;
        }
        if (nAngle==2700) {
        AddOpt( ESCHER_Prop_txflTextFlow, 3 );
        bSuppressRotation=true;
        }
    }
}

sal_Bool EscherPropertyContainer::GetLineArrow( const sal_Bool bLineStart,
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
        ESCHER_LineEnd& reLineEnd, sal_Int32& rnArrowLength, sal_Int32& rnArrowWidth )
{
    static String sLineStart    ( RTL_CONSTASCII_USTRINGPARAM( "LineStart" ) );
    static String sLineStartName( RTL_CONSTASCII_USTRINGPARAM( "LineStartName" ) );
    static String sLineEnd      ( RTL_CONSTASCII_USTRINGPARAM( "LineEnd" ) );
    static String sLineEndName  ( RTL_CONSTASCII_USTRINGPARAM( "LineEndName" ) );

    const String sLine      ( bLineStart ? sLineStart : sLineEnd );
    const String sLineName  ( bLineStart ? sLineStartName : sLineEndName );

    sal_Bool bIsArrow = sal_False;

    ::com::sun::star::uno::Any aAny;
    if ( EscherPropertyValueHelper::GetPropertyValue(
            aAny, rXPropSet, sLine, sal_False ) )
    {
        PolyPolygon aPolyPoly( EscherPropertyContainer::GetPolyPolygon( aAny ) );
        if ( aPolyPoly.Count() && aPolyPoly[ 0 ].GetSize() )
        {
            bIsArrow = sal_True;

            reLineEnd     = ESCHER_LineArrowEnd;
            rnArrowLength = 1;
            rnArrowWidth  = 1;

            if ( EscherPropertyValueHelper::GetPropertyValue(
                aAny, rXPropSet, sLineName, sal_False ) )
            {
                String          aArrowStartName = *(::rtl::OUString*)aAny.getValue();
                rtl::OUString   aApiName;
                sal_Int16       nWhich = bLineStart ? XATTR_LINESTART : XATTR_LINEEND;

                SvxUnogetApiNameForItem( nWhich, aArrowStartName, aApiName );
                if ( !aApiName.isEmpty() )
                {

                    /* todo:
                    calculate the best option for ArrowLenght and ArrowWidth
                    */
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
                }
                else if ( comphelper::string::getTokenCount(aArrowStartName, ' ') == 2 )
                {
                    sal_Bool b = sal_True;
                    String aArrowName( aArrowStartName.GetToken( 0, ' ' ) );
                    if (  aArrowName.EqualsAscii( "msArrowEnd" ) )
                        reLineEnd = ESCHER_LineArrowEnd;
                    else if (  aArrowName.EqualsAscii( "msArrowOpenEnd" ) )
                        reLineEnd = ESCHER_LineArrowOpenEnd;
                    else if ( aArrowName.EqualsAscii( "msArrowStealthEnd" ) )
                        reLineEnd = ESCHER_LineArrowStealthEnd;
                    else if ( aArrowName.EqualsAscii( "msArrowDiamondEnd" ) )
                        reLineEnd = ESCHER_LineArrowDiamondEnd;
                    else if ( aArrowName.EqualsAscii( "msArrowOvalEnd" ) )
                        reLineEnd = ESCHER_LineArrowOvalEnd;
                    else
                        b = sal_False;

                    // now we have the arrow, and try to determine the arrow size;
                    if ( b )
                    {
                        String aArrowSize( aArrowStartName.GetToken( 1, ' ' ) );
                        sal_Int32 nArrowSize = aArrowSize.ToInt32();
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
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
        sal_Bool bEdge )
{
    ::com::sun::star::uno::Any aAny;
    sal_uInt32 nLineFlags = 0x80008;

    ESCHER_LineEnd eLineEnd;
    sal_Int32 nArrowLength;
    sal_Int32 nArrowWidth;

    sal_Bool bSwapLineEnds = sal_False;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "CircleKind" ) ), sal_True ) )
    {
        ::com::sun::star::drawing::CircleKind  eCircleKind;
        if ( aAny >>= eCircleKind )
        {
            if ( eCircleKind == ::com::sun::star::drawing::CircleKind_ARC )
                bSwapLineEnds = sal_True;
        }
    }
    if ( GetLineArrow( bSwapLineEnds ? sal_False : sal_True, rXPropSet, eLineEnd, nArrowLength, nArrowWidth ) )
    {
        AddOpt( ESCHER_Prop_lineStartArrowLength, nArrowLength );
        AddOpt( ESCHER_Prop_lineStartArrowWidth, nArrowWidth );
        AddOpt( ESCHER_Prop_lineStartArrowhead, eLineEnd );
        nLineFlags |= 0x100010;
    }
    if ( GetLineArrow( bSwapLineEnds ? sal_True : sal_False, rXPropSet, eLineEnd, nArrowLength, nArrowWidth ) )
    {
        AddOpt( ESCHER_Prop_lineEndArrowLength, nArrowLength );
        AddOpt( ESCHER_Prop_lineEndArrowWidth, nArrowWidth );
        AddOpt( ESCHER_Prop_lineEndArrowhead, eLineEnd );
        nLineFlags |= 0x100010;
    }
    if ( EscherPropertyValueHelper::GetPropertyValue(
        aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "LineStyle"  ) ), sal_False ) )
    {
        ::com::sun::star::drawing::LineStyle eLS;
        if ( aAny >>= eLS )
        {
            switch ( eLS )
            {
                case ::com::sun::star::drawing::LineStyle_NONE :
                    AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );           // 80000
                break;

                case ::com::sun::star::drawing::LineStyle_DASH :
                {
                    if ( EscherPropertyValueHelper::GetPropertyValue(
                        aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "LineDash" ) ), sal_False ) )
                    {
                        ESCHER_LineDashing eDash = ESCHER_LineSolid;
                        ::com::sun::star::drawing::LineDash* pLineDash = (::com::sun::star::drawing::LineDash*)aAny.getValue();
                        sal_Int32 nDistance = pLineDash->Distance << 1;
                        switch ( pLineDash->Style )
                        {
                            case ::com::sun::star::drawing::DashStyle_ROUND :
                            case ::com::sun::star::drawing::DashStyle_ROUNDRELATIVE :
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
                case ::com::sun::star::drawing::LineStyle_SOLID :
                default:
                {
                    AddOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );
                }
                break;
            }
        }
        if ( EscherPropertyValueHelper::GetPropertyValue(
            aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "LineColor"  ) ), sal_False ) )
        {
            sal_uInt32 nLineColor = ImplGetColor( *((sal_uInt32*)aAny.getValue()) );
            AddOpt( ESCHER_Prop_lineColor, nLineColor );
            AddOpt( ESCHER_Prop_lineBackColor, nLineColor ^ 0xffffff );
        }
    }

    sal_uInt32 nLineSize = ( EscherPropertyValueHelper::GetPropertyValue(
        aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "LineWidth"  ) ), sal_False ) )
        ? *((sal_uInt32*)aAny.getValue())
        : 0;
    if ( nLineSize > 1 )
        AddOpt( ESCHER_Prop_lineWidth, nLineSize * 360 );       // 100TH MM -> PT , 1PT = 12700 EMU

    ESCHER_LineJoin eLineJoin = ESCHER_LineJoinMiter;
    if ( EscherPropertyValueHelper::GetPropertyValue(
        aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "LineJoint" ) ), sal_True ) )
    {
        ::com::sun::star::drawing::LineJoint eLJ;
        if ( aAny >>= eLJ )
        {
            switch ( eLJ )
            {
                case com::sun::star::drawing::LineJoint_NONE :
                case com::sun::star::drawing::LineJoint_MIDDLE :
                case com::sun::star::drawing::LineJoint_BEVEL :
                    eLineJoin = ESCHER_LineJoinBevel;
                break;
                default:
                case com::sun::star::drawing::LineJoint_MITER :
                    eLineJoin = ESCHER_LineJoinMiter;
                break;
                case com::sun::star::drawing::LineJoint_ROUND :
                    eLineJoin = ESCHER_LineJoinRound;
                break;
            }
        }
    }
    AddOpt( ESCHER_Prop_lineJoinStyle, eLineJoin );

    if ( bEdge == sal_False )
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
        aRetSize = Application::GetDefaultDevice()->LogicToLogic( aPrefSize, aPrefMapMode, MAP_100TH_MM );
    return aRetSize;
}

void EscherPropertyContainer::ImplCreateGraphicAttributes( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                                                            sal_uInt32 nBlibId, sal_Bool bCreateCroppingAttributes )
{
    ::com::sun::star::uno::Any aAny;

    sal_uInt32 nPicFlags = 0;
    ::com::sun::star::drawing::ColorMode eColorMode( ::com::sun::star::drawing::ColorMode_STANDARD );
    sal_Int16 nLuminance = 0;
    sal_Int32 nContrast = 0;
    sal_Int16 nRed = 0;
    sal_Int16 nGreen = 0;
    sal_Int16 nBlue = 0;
    double fGamma = 1.0;
    sal_Int16 nTransparency = 0;

    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "GraphicColorMode" ) ) ) )
        aAny >>= eColorMode;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "AdjustLuminance" ) ) ) )
        aAny >>= nLuminance;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "AdjustContrast" ) ) ) )
    {
        sal_Int16 nC = sal_Int16();
        aAny >>= nC;
        nContrast = nC;
    }
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "AdjustRed" ) ) ) )
        aAny >>= nRed;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "AdjustGreen" ) ) ) )
        aAny >>= nGreen;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "AdjustBlue" ) ) ) )
        aAny >>= nBlue;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Gamma" ) ) ) )
        aAny >>= fGamma;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Transparency" ) ) ) )
        aAny >>= nTransparency;

    if ( eColorMode == ::com::sun::star::drawing::ColorMode_WATERMARK )
    {
        eColorMode = ::com::sun::star::drawing::ColorMode_STANDARD;
        nLuminance += 70;
        if ( nLuminance > 100 )
            nLuminance = 100;
        nContrast -= 70;
        if ( nContrast < -100 )
            nContrast = -100;
    }
    if ( eColorMode == ::com::sun::star::drawing::ColorMode_GREYS )
        nPicFlags |= 0x40004;
    else if ( eColorMode == ::com::sun::star::drawing::ColorMode_MONO )
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
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "GraphicCrop" ) ) ) )
                {
                    ::com::sun::star::text::GraphicCrop aGraphCrop;
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

sal_Bool EscherPropertyContainer::CreateShapeProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape )
{
    uno::Reference< beans::XPropertySet > aXPropSet( rXShape, uno::UNO_QUERY );
    if ( aXPropSet.is() )
    {
        sal_Bool bVal = false;
        ::com::sun::star::uno::Any aAny;
        sal_uInt32 nShapeAttr = 0;
        EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Visible" ) ), sal_True );
        if ( aAny >>= bVal )
        {
            if ( !bVal )
                nShapeAttr |= 0x20002;  // set fHidden = true
        }
        EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Printable" ) ), sal_True );
        if ( aAny >>= bVal )
        {
            if ( !bVal )
                nShapeAttr |= 0x10000;  // set fPrint = false;
        }
        if ( nShapeAttr )
            AddOpt( ESCHER_Prop_fPrint, nShapeAttr );
    }
    return sal_True;
}

sal_Bool EscherPropertyContainer::CreateOLEGraphicProperties(
    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape )
{
    sal_Bool    bRetValue = sal_False;

    if ( rXShape.is() )
    {
        SdrObject* pSdrOLE2( GetSdrObjectFromXShape( rXShape ) );   // SJ: leaving unoapi, because currently there is
        if ( pSdrOLE2 && pSdrOLE2->ISA( SdrOle2Obj ) )              // no access to the native graphic object
        {
            Graphic* pGraphic = ((SdrOle2Obj*)pSdrOLE2)->GetGraphic();
            if ( pGraphic )
            {
                GraphicObject aGraphicObject( *pGraphic );
                rtl::OString aUniqueId( aGraphicObject.GetUniqueID() );
                if ( !aUniqueId.isEmpty() )
                {
                    AddOpt( ESCHER_Prop_fillType, ESCHER_FillPicture );
                    uno::Reference< beans::XPropertySet > aXPropSet( rXShape, uno::UNO_QUERY );

                    if ( pGraphicProvider && pPicOutStrm && pShapeBoundRect && aXPropSet.is() )
                    {
                        ::com::sun::star::uno::Any aAny;
                        ::com::sun::star::awt::Rectangle* pVisArea = NULL;
                        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "VisibleArea" ) ) ) )
                        {
                            pVisArea = new ::com::sun::star::awt::Rectangle;
                            aAny >>= (*pVisArea);
                        }
                        Rectangle aRect( Point( 0, 0 ), pShapeBoundRect->GetSize() );
                        sal_uInt32 nBlibId = pGraphicProvider->GetBlibID( *pPicOutStrm, aUniqueId, aRect, pVisArea, NULL );
                        if ( nBlibId )
                        {
                            AddOpt( ESCHER_Prop_pib, nBlibId, sal_True );
                            ImplCreateGraphicAttributes( aXPropSet, nBlibId, sal_False );
                            bRetValue = sal_True;
                        }
                        delete pVisArea;
                    }
                }
            }
        }
    }
    return bRetValue;
}


sal_Bool EscherPropertyContainer::ImplCreateEmbeddedBmp( const rtl::OString& rUniqueId )
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
            aMemStrm.ObjectOwnsMemory( sal_False );
            sal_uInt8* pBuf = (sal_uInt8*) aMemStrm.GetData();
            sal_uInt32 nSize = aMemStrm.Seek( STREAM_SEEK_TO_END );
            AddOpt( ESCHER_Prop_fillBlip, sal_True, nSize, pBuf, nSize );
            return sal_True;
        }
    }
    return sal_False;
}

sal_Bool EscherPropertyContainer::CreateEmbeddedBitmapProperties(
    const ::rtl::OUString& rBitmapUrl, ::com::sun::star::drawing::BitmapMode eBitmapMode )
{
    sal_Bool bRetValue = sal_False;
    String aVndUrl( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.GraphicObject:" ) );
    String aBmpUrl( rBitmapUrl );
    xub_StrLen nIndex = aBmpUrl.Search( aVndUrl, 0 );
    if( nIndex != STRING_NOTFOUND )
    {
        // note: += ist not defined for xub_StrLen -> conversion to int and back to xub_StrLen
        nIndex = nIndex + aVndUrl.Len();
        if( aBmpUrl.Len() > nIndex )
        {
            rtl::OString aUniqueId(rtl::OUStringToOString(aBmpUrl.Copy(nIndex, aBmpUrl.Len() - nIndex), RTL_TEXTENCODING_UTF8));
            bRetValue = ImplCreateEmbeddedBmp( aUniqueId );
            if( bRetValue )
            {
                // bitmap mode property
                bool bRepeat = eBitmapMode == ::com::sun::star::drawing::BitmapMode_REPEAT;
                AddOpt( ESCHER_Prop_fillType, bRepeat ? ESCHER_FillTexture : ESCHER_FillPicture );
            }
        }
    }
    return bRetValue;
}


namespace {

GraphicObject lclDrawHatch( const ::com::sun::star::drawing::Hatch& rHatch, const Color& rBackColor, bool bFillBackground )
{
    const MapMode aMap100( MAP_100TH_MM );
    VirtualDevice aVDev( *Application::GetDefaultDevice(), 0, 1 );
    aVDev.SetMapMode( aMap100 );

    const Size aOutSize = aVDev.PixelToLogic( Size( 28, 28 ) );
    aVDev.SetOutputSize( aOutSize );

    Rectangle aRectangle( Point( 0, 0 ), aOutSize );
    const PolyPolygon aPolyPoly( aRectangle );

    aVDev.SetLineColor();
    aVDev.SetFillColor( bFillBackground ? rBackColor : Color( COL_TRANSPARENT ) );
    aVDev.DrawRect( Rectangle( Point(), aOutSize ) );

    Hatch aVclHatch( (HatchStyle) rHatch.Style, Color( rHatch.Color ), rHatch.Distance, (sal_uInt16)rHatch.Angle );
    aVDev.DrawHatch( aPolyPoly, aVclHatch );

    return GraphicObject( Graphic( aVDev.GetBitmapEx( Point(), aOutSize ) ) );
}

} // namespace


sal_Bool EscherPropertyContainer::CreateEmbeddedHatchProperties( const ::com::sun::star::drawing::Hatch& rHatch, const Color& rBackColor, bool bFillBackground )
{
    GraphicObject aGraphicObject = lclDrawHatch( rHatch, rBackColor, bFillBackground );
    rtl::OString aUniqueId = aGraphicObject.GetUniqueID();
    sal_Bool bRetValue = ImplCreateEmbeddedBmp( aUniqueId );
    if ( bRetValue )
        AddOpt( ESCHER_Prop_fillType, ESCHER_FillTexture );
    return bRetValue;
}


sal_Bool EscherPropertyContainer::CreateGraphicProperties(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
        const String& rSource, const sal_Bool bCreateFillBitmap, const sal_Bool bCreateCroppingAttributes,
            const sal_Bool bFillBitmapModeAllowed )
{
    sal_Bool        bRetValue = sal_False;
    sal_Bool        bCreateFillStyles = sal_False;

    sal_Bool        bMirrored = sal_False;
    sal_Bool        bRotate   = sal_True;
    GraphicAttr*    pGraphicAttr = NULL;
    GraphicObject   aGraphicObject;
    String          aGraphicUrl;
    rtl::OString    aUniqueId;
    bool            bIsGraphicMtf(false);

    ::com::sun::star::drawing::BitmapMode   eBitmapMode( ::com::sun::star::drawing::BitmapMode_NO_REPEAT );
    ::com::sun::star::uno::Any aAny;

    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, rSource ) )
    {
        sal_uInt16 nAngle = 0;
        if ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ) )
        {
            ::com::sun::star::uno::Sequence<sal_uInt8> aSeq = *(::com::sun::star::uno::Sequence<sal_uInt8>*)aAny.getValue();
            const sal_uInt8*    pAry = aSeq.getArray();
            sal_uInt32          nAryLen = aSeq.getLength();

            // the metafile is already rotated
            bRotate = sal_False;

            if ( pAry && nAryLen )
            {
                Graphic         aGraphic;
                SvMemoryStream  aTemp( (void*)pAry, nAryLen, STREAM_READ );
                sal_uInt32 nErrCode = GraphicConverter::Import( aTemp, aGraphic, CVT_WMF );
                if ( nErrCode == ERRCODE_NONE )
                {
                    aGraphicObject = aGraphic;
                    aUniqueId = aGraphicObject.GetUniqueID();
                    bIsGraphicMtf = aGraphicObject.GetType() == GRAPHIC_GDIMETAFILE;
                }
            }
        }
        else if ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "Bitmap" ) ) )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >xBitmap;
            if ( ::cppu::extractInterface( xBitmap, aAny ) )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap > xBmp;
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
        else if ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "FillBitmapURL" ) ) )
        {
            aGraphicUrl = *(::rtl::OUString*)aAny.getValue();
        }
        else if ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ) )
        {
            aGraphicUrl = *(::rtl::OUString*)aAny.getValue();
            bCreateFillStyles = sal_True;
        }
        else if ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "FillHatch" ) ) )
        {
            ::com::sun::star::drawing::Hatch aHatch;
            if ( aAny >>= aHatch )
            {
                Color aBackColor;
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                    String( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ), sal_False ) )
                {
                    aBackColor = ImplGetColor( *((sal_uInt32*)aAny.getValue()), sal_False );
                }
                bool bFillBackground = false;
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                        String( RTL_CONSTASCII_USTRINGPARAM( "FillBackground" ) ), sal_True ) )
                {
                    aAny >>= bFillBackground;
                }
                aGraphicObject = lclDrawHatch( aHatch, aBackColor, bFillBackground );
                aUniqueId = aGraphicObject.GetUniqueID();
                eBitmapMode = ::com::sun::star::drawing::BitmapMode_REPEAT;
                bIsGraphicMtf = aGraphicObject.GetType() == GRAPHIC_GDIMETAFILE;
            }
        }

        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsMirrored" ) ), sal_True ) )
            aAny >>= bMirrored;

        if ( bCreateFillBitmap && bFillBitmapModeAllowed )
        {
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillBitmapMode" ) ), sal_True ) )
                aAny >>= eBitmapMode;
        }
        else
        {
            nAngle = bRotate && EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                                                                             String( RTL_CONSTASCII_USTRINGPARAM( "RotateAngle" ) ), sal_True )
                ? (sal_uInt16)( ( *((sal_Int32*)aAny.getValue() ) ) + 5 ) / 10
                : 0;
        }

        if ( aGraphicUrl.Len() )
        {
            String aVndUrl( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.GraphicObject:" ) );
            xub_StrLen nIndex = aGraphicUrl.Search( aVndUrl, 0 );
            if ( nIndex != STRING_NOTFOUND )
            {
                nIndex = nIndex + aVndUrl.Len();
                if ( aGraphicUrl.Len() > nIndex  )
                    aUniqueId = rtl::OUStringToOString(aGraphicUrl.Copy(nIndex, aGraphicUrl.Len() - nIndex), RTL_TEXTENCODING_UTF8);
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
                const sal_uInt16 nFormat = aDescriptor.GetFileFormat();

                // can MSO handle it?
                if ( bMirrored || nAngle ||
                     (nFormat != GFF_BMP &&
                      nFormat != GFF_GIF &&
                      nFormat != GFF_JPG &&
                      nFormat != GFF_PNG &&
                      nFormat != GFF_TIF &&
                      nFormat != GFF_PCT &&
                      nFormat != GFF_WMF &&
                      nFormat != GFF_EMF) )
                {
                    SvStream* pIn = ::utl::UcbStreamHelper::CreateStream(
                        aTmp.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );
                    if ( pIn )
                    {
                        Graphic aGraphic;
                        sal_uInt32 nErrCode = GraphicConverter::Import( *pIn, aGraphic );

                        if ( nErrCode == ERRCODE_NONE )
                        {
                            // no.
                            aGraphicObject = aGraphic;
                            aUniqueId = aGraphicObject.GetUniqueID();
                        }
                        // else: simply keep the graphic link
                        delete pIn;
                    }
                }
                if ( aUniqueId.isEmpty() )
                {
                    if ( pGraphicProvider )
                    {
                        const rtl::OUString& rBaseURI( pGraphicProvider->GetBaseURI() );
                        INetURLObject aBaseURI( rBaseURI );
                        if( aBaseURI.GetProtocol() == aTmp.GetProtocol() )
                        {
                            rtl::OUString aRelUrl( INetURLObject::GetRelURL( rBaseURI, aGraphicUrl,
                                                    INetURLObject::WAS_ENCODED, INetURLObject::DECODE_TO_IURI, RTL_TEXTENCODING_UTF8, INetURLObject::FSYS_DETECT ) );
                            if ( !aRelUrl.isEmpty() )
                                aGraphicUrl = aRelUrl;
                        }
                    }
                }
            }
        }

        if ( aGraphicUrl.Len() || !aUniqueId.isEmpty() )
        {
            if ( bMirrored || nAngle )
            {
                pGraphicAttr = new GraphicAttr;
                if ( bMirrored )
                    pGraphicAttr->SetMirrorFlags( BMP_MIRROR_HORZ );
                if ( bIsGraphicMtf )
                    AddOpt( ESCHER_Prop_Rotation, ( ( ((sal_Int32)nAngle << 16 ) / 10 ) + 0x8000 ) &~ 0xffff );
                else
                {
                    pGraphicAttr->SetRotation( nAngle );
                    if ( nAngle && pShapeBoundRect )   // up to xp ppoint does not rotate bitmaps !
                    {
                        Polygon aPoly( *pShapeBoundRect );
                        aPoly.Rotate( pShapeBoundRect->TopLeft(), nAngle );
                        *pShapeBoundRect = aPoly.GetBoundRect();
                        bSuppressRotation = sal_True;
                    }
                }
            }

            if ( eBitmapMode == ::com::sun::star::drawing::BitmapMode_REPEAT )
                AddOpt( ESCHER_Prop_fillType, ESCHER_FillTexture );
            else
                AddOpt( ESCHER_Prop_fillType, ESCHER_FillPicture );

            if ( !aUniqueId.isEmpty() )
            {
                // write out embedded graphic
                if ( pGraphicProvider && pPicOutStrm && pShapeBoundRect )
                {
                    Rectangle aRect( Point( 0, 0 ), pShapeBoundRect->GetSize() );

                    sal_uInt32 nBlibId = 0;
                    nBlibId = pGraphicProvider->GetBlibID( *pPicOutStrm, aUniqueId, aRect, NULL, pGraphicAttr );
                    if ( nBlibId )
                    {
                        if ( bCreateFillBitmap )
                            AddOpt( ESCHER_Prop_fillBlip, nBlibId, sal_True );
                        else
                        {
                            AddOpt( ESCHER_Prop_pib, nBlibId, sal_True );
                            ImplCreateGraphicAttributes( rXPropSet, nBlibId, bCreateCroppingAttributes );
                        }
                        bRetValue = sal_True;
                    }
                }
                else
                {
                    EscherGraphicProvider aProvider;
                    SvMemoryStream aMemStrm;
                    Rectangle aRect;

                    if ( aProvider.GetBlibID( aMemStrm, aUniqueId, aRect, NULL, pGraphicAttr ) )
                    {
                        // grab BLIP from stream and insert directly as complex property
                        // ownership of stream memory goes to complex property
                        aMemStrm.ObjectOwnsMemory( sal_False );
                        sal_uInt8* pBuf = (sal_uInt8*) aMemStrm.GetData();
                        sal_uInt32 nSize = aMemStrm.Seek( STREAM_SEEK_TO_END );
                        AddOpt( ESCHER_Prop_fillBlip, sal_True, nSize, pBuf, nSize );
                        bRetValue = sal_True;
                    }
                }
            }
            // write out link to graphic
            else
            {
                OSL_ASSERT(aGraphicUrl.Len());

                AddOpt( ESCHER_Prop_pibName, aGraphicUrl );
                sal_uInt32  nPibFlags=0;
                GetOpt( ESCHER_Prop_pibFlags, nPibFlags );
                AddOpt( ESCHER_Prop_pibFlags,
                        ESCHER_BlipFlagLinkToFile|ESCHER_BlipFlagFile|ESCHER_BlipFlagDoNotSave | nPibFlags );
            }
        }
    }
    delete pGraphicAttr;
    if ( bCreateFillStyles )
        CreateFillProperties( rXPropSet, sal_True );

    return bRetValue;
}

PolyPolygon EscherPropertyContainer::GetPolyPolygon( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape )
{
    PolyPolygon aRetPolyPoly;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXPropSet;
    ::com::sun::star::uno::Any aAny( rXShape->queryInterface(
        ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >*) 0 ) ));

    String sPolyPolygonBezier( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygonBezier" ) );
    String sPolyPolygon     ( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygon" ) );
    String sPolygon         ( RTL_CONSTASCII_USTRINGPARAM( "Polygon" ) );

    if ( aAny >>= aXPropSet )
    {
        sal_Bool bHasProperty = EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sPolyPolygonBezier, sal_True );
        if ( !bHasProperty )
            bHasProperty = EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sPolyPolygon, sal_True );
        if ( !bHasProperty )
            bHasProperty = EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sPolygon, sal_True );
        if ( bHasProperty )
            aRetPolyPoly = GetPolyPolygon( aAny );
    }
    return aRetPolyPoly;
}

PolyPolygon EscherPropertyContainer::GetPolyPolygon( const ::com::sun::star::uno::Any& rAny )
{
    sal_Bool bNoError = sal_True;

    Polygon aPolygon;
    PolyPolygon aPolyPolygon;

    if ( rAny.getValueType() == ::getCppuType( ( const ::com::sun::star::drawing::PolyPolygonBezierCoords* ) 0 ) )
    {
        ::com::sun::star::drawing::PolyPolygonBezierCoords* pSourcePolyPolygon
            = (::com::sun::star::drawing::PolyPolygonBezierCoords*)rAny.getValue();
        sal_uInt16 nOuterSequenceCount = (sal_uInt16)pSourcePolyPolygon->Coordinates.getLength();

        // Zeiger auf innere sequences holen
        ::com::sun::star::drawing::PointSequence* pOuterSequence = pSourcePolyPolygon->Coordinates.getArray();
        ::com::sun::star::drawing::FlagSequence*  pOuterFlags = pSourcePolyPolygon->Flags.getArray();

        bNoError = pOuterSequence && pOuterFlags;
        if ( bNoError )
        {
            sal_uInt16  a, b, nInnerSequenceCount;
            ::com::sun::star::awt::Point* pArray;

            // dies wird ein Polygon set
            for ( a = 0; a < nOuterSequenceCount; a++ )
            {
                ::com::sun::star::drawing::PointSequence* pInnerSequence = pOuterSequence++;
                ::com::sun::star::drawing::FlagSequence*  pInnerFlags = pOuterFlags++;

                bNoError = pInnerSequence && pInnerFlags;
                if  ( bNoError )
                {
                    // Zeiger auf Arrays holen
                    pArray = pInnerSequence->getArray();
                    ::com::sun::star::drawing::PolygonFlags* pFlags = pInnerFlags->getArray();

                    if ( pArray && pFlags )
                    {
                        nInnerSequenceCount = (sal_uInt16)pInnerSequence->getLength();
                        aPolygon = Polygon( nInnerSequenceCount );
                        for( b = 0; b < nInnerSequenceCount; b++)
                        {
                            PolyFlags   ePolyFlags( *( (PolyFlags*)pFlags++ ) );
                            ::com::sun::star::awt::Point aPoint( (::com::sun::star::awt::Point)*(pArray++) );
                            aPolygon[ b ] = Point( aPoint.X, aPoint.Y );
                            aPolygon.SetFlags( b, ePolyFlags );

                            if ( ePolyFlags == POLY_CONTROL )
                                continue;
                        }
                        aPolyPolygon.Insert( aPolygon, POLYPOLY_APPEND );
                    }
                }
            }
        }
    }
    else if ( rAny.getValueType() == ::getCppuType( ( const ::com::sun::star::drawing::PointSequenceSequence* ) 0 ) )
    {
        ::com::sun::star::drawing::PointSequenceSequence* pSourcePolyPolygon
            = (::com::sun::star::drawing::PointSequenceSequence*)rAny.getValue();
        sal_uInt16 nOuterSequenceCount = (sal_uInt16)pSourcePolyPolygon->getLength();

        // Zeiger auf innere sequences holen
        ::com::sun::star::drawing::PointSequence* pOuterSequence = pSourcePolyPolygon->getArray();
        bNoError = pOuterSequence != NULL;
        if ( bNoError )
        {
            sal_uInt16 a, b, nInnerSequenceCount;

            // dies wird ein Polygon set
            for( a = 0; a < nOuterSequenceCount; a++ )
            {
                ::com::sun::star::drawing::PointSequence* pInnerSequence = pOuterSequence++;
                bNoError = pInnerSequence != NULL;
                if ( bNoError )
                {
                    // Zeiger auf Arrays holen
                    ::com::sun::star::awt::Point* pArray =
                          pInnerSequence->getArray();
                    if ( pArray != NULL )
                    {
                        nInnerSequenceCount = (sal_uInt16)pInnerSequence->getLength();
                        aPolygon = Polygon( nInnerSequenceCount );
                        for( b = 0; b < nInnerSequenceCount; b++)
                        {
                            aPolygon[ b ] = Point( pArray->X, pArray->Y );
                            pArray++;
                        }
                        aPolyPolygon.Insert( aPolygon, POLYPOLY_APPEND );
                    }
                }
            }
        }
    }
    else if ( rAny.getValueType() == ::getCppuType( ( const ::com::sun::star::drawing::PointSequence* ) 0 ) )
    {
        ::com::sun::star::drawing::PointSequence* pInnerSequence =
            (::com::sun::star::drawing::PointSequence*)rAny.getValue();

        bNoError = pInnerSequence != NULL;
        if ( bNoError )
        {
            sal_uInt16 a, nInnerSequenceCount;

            // Zeiger auf Arrays holen
            ::com::sun::star::awt::Point* pArray = pInnerSequence->getArray();
            if ( pArray != NULL )
            {
                nInnerSequenceCount = (sal_uInt16)pInnerSequence->getLength();
                aPolygon = Polygon( nInnerSequenceCount );
                for( a = 0; a < nInnerSequenceCount; a++)
                {
                    aPolygon[ a ] = Point( pArray->X, pArray->Y );
                    pArray++;
                }
                aPolyPolygon.Insert( aPolygon, POLYPOLY_APPEND );
            }
        }
    }
    return aPolyPolygon;
}

sal_Bool EscherPropertyContainer::CreatePolygonProperties(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
        sal_uInt32 nFlags,
            sal_Bool bBezier,
                ::com::sun::star::awt::Rectangle& rGeoRect,
                    Polygon* pPolygon )
{
    static String sPolyPolygonBezier( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygonBezier" ) );
    static String sPolyPolygon      ( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygon" ) );

    sal_Bool    bRetValue = sal_True;
    sal_Bool    bLine = ( nFlags & ESCHER_CREATEPOLYGON_LINE ) != 0;

    PolyPolygon aPolyPolygon;

    if ( pPolygon )
        aPolyPolygon.Insert( *pPolygon, POLYPOLY_APPEND );
    else
    {
        ::com::sun::star::uno::Any aAny;
        bRetValue = EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                        ( bBezier ) ? sPolyPolygonBezier : sPolyPolygon, sal_True );
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
                const Polygon& rPoly = aPolyPolygon[ 0 ];
                rGeoRect = ::com::sun::star::awt::Rectangle(
                    rPoly[ 0 ].X(),
                        rPoly[ 0 ].Y(),
                            rPoly[ 1 ].X() - rPoly[ 0 ].X(),
                                rPoly[ 1 ].Y() - rPoly[ 0 ].Y() );
            }
            else
                bRetValue = sal_False;
        }
        else
        {
            Polygon aPolygon;

            sal_uInt16 i, j, k, nPoints, nBezPoints, nPolyCount = aPolyPolygon.Count();
            Rectangle aRect( aPolyPolygon.GetBoundRect() );
            rGeoRect = ::com::sun::star::awt::Rectangle( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight() );

            for ( nBezPoints = nPoints = i = 0; i < nPolyCount; i++ )
            {
                k = aPolyPolygon[ i ].GetSize();
                nPoints = nPoints + k;
                for ( j = 0; j < k; j++ )
                {
                    if ( aPolyPolygon[ i ].GetFlags( j ) != POLY_CONTROL )
                        nBezPoints++;
                }
            }
            sal_uInt32 nVerticesBufSize = ( nPoints << 2 ) + 6;
            sal_uInt8* pVerticesBuf = new sal_uInt8[ nVerticesBufSize ];


            sal_uInt32 nSegmentBufSize = ( ( nBezPoints << 2 ) + 8 );
            if ( nPolyCount > 1 )
                nSegmentBufSize += ( nPolyCount << 1 );
            sal_uInt8* pSegmentBuf = new sal_uInt8[ nSegmentBufSize ];

            sal_uInt8* pPtr = pVerticesBuf;
            *pPtr++ = (sal_uInt8)( nPoints );                    // Little endian
            *pPtr++ = (sal_uInt8)( nPoints >> 8 );
            *pPtr++ = (sal_uInt8)( nPoints );
            *pPtr++ = (sal_uInt8)( nPoints >> 8 );
            *pPtr++ = (sal_uInt8)0xf0;
            *pPtr++ = (sal_uInt8)0xff;

            for ( j = 0; j < nPolyCount; j++ )
            {
                aPolygon = aPolyPolygon[ j ];
                nPoints = aPolygon.GetSize();
                for ( i = 0; i < nPoints; i++ )             // write points from polygon to buffer
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

            for ( j = 0; j < nPolyCount; j++ )
            {
                *pPtr++ = 0x0;          // Polygon start
                *pPtr++ = 0x40;
                aPolygon = aPolyPolygon[ j ];
                nPoints = aPolygon.GetSize();
                for ( i = 0; i < nPoints; i++ )         // write Polyflags to Buffer
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
            AddOpt( ESCHER_Prop_pVertices, sal_True, nVerticesBufSize - 6, (sal_uInt8*)pVerticesBuf, nVerticesBufSize );
            AddOpt( ESCHER_Prop_pSegmentInfo, sal_True, nSegmentBufSize, (sal_uInt8*)pSegmentBuf, nSegmentBufSize );
        }
    }
    return bRetValue;
}

sal_Bool EscherPropertyContainer::CreateConnectorProperties(
    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape,
    EscherSolverContainer& rSolverContainer, ::com::sun::star::awt::Rectangle& rGeoRect,
            sal_uInt16& rShapeType, sal_uInt16& rShapeFlags )
{
    static String sEdgeKind             ( RTL_CONSTASCII_USTRINGPARAM( "EdgeKind" ) );
    static String sEdgeStartPoint       ( RTL_CONSTASCII_USTRINGPARAM( "EdgeStartPoint" ) );
    static String sEdgeEndPoint         ( RTL_CONSTASCII_USTRINGPARAM( "EdgeEndPoint" ) );
    static String sEdgeStartConnection  ( RTL_CONSTASCII_USTRINGPARAM( "EdgeStartConnection" ) );
    static String sEdgeEndConnection    ( RTL_CONSTASCII_USTRINGPARAM( "EdgeEndConnection" ) );

    sal_Bool bRetValue = sal_False;
    rShapeType = rShapeFlags = 0;

    if ( rXShape.is() )
    {
        ::com::sun::star::awt::Point aStartPoint, aEndPoint;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXPropSet;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > aShapeA, aShapeB;
        ::com::sun::star::uno::Any aAny( rXShape->queryInterface( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >*) 0 ) ));
        if ( aAny >>= aXPropSet )
        {
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sEdgeKind, sal_True ) )
            {
                ::com::sun::star::drawing::ConnectorType eCt;
                aAny >>= eCt;
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sEdgeStartPoint ) )
                {
                    aStartPoint = *(::com::sun::star::awt::Point*)aAny.getValue();
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sEdgeEndPoint ) )
                    {
                        aEndPoint = *(::com::sun::star::awt::Point*)aAny.getValue();

                        rShapeFlags = SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT | SHAPEFLAG_CONNECTOR;
                        rGeoRect = ::com::sun::star::awt::Rectangle( aStartPoint.X, aStartPoint.Y,
                                                            ( aEndPoint.X - aStartPoint.X ) + 1, ( aEndPoint.Y - aStartPoint.Y ) + 1 );
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
                        sal_uInt32 nAdjustValue1, nAdjustValue2, nAdjustValue3;
                        nAdjustValue1 = nAdjustValue2 = nAdjustValue3 = 0x2a30;

                        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sEdgeStartConnection ) )
                            aAny >>= aShapeA;
                        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sEdgeEndConnection ) )
                            aAny >>= aShapeB;
                        rSolverContainer.AddConnector( rXShape, aStartPoint, aShapeA, aEndPoint, aShapeB );
                        switch ( eCt )
                        {
                            case ::com::sun::star::drawing::ConnectorType_CURVE :
                            {
                                rShapeType = ESCHER_ShpInst_CurvedConnector3;
                                AddOpt( ESCHER_Prop_cxstyle, ESCHER_cxstyleCurved );
                                AddOpt( ESCHER_Prop_adjustValue, nAdjustValue1 );
                                AddOpt( ESCHER_Prop_adjust2Value, -(sal_Int32)nAdjustValue2 );
                            }
                            break;

                            case ::com::sun::star::drawing::ConnectorType_STANDARD :// Connector 2->5
                            {
                                rShapeType = ESCHER_ShpInst_BentConnector3;
                                AddOpt( ESCHER_Prop_cxstyle, ESCHER_cxstyleBent );
                            }
                            break;

                            default:
                            case ::com::sun::star::drawing::ConnectorType_LINE :
                            case ::com::sun::star::drawing::ConnectorType_LINES :   // Connector 2->5
                            {
                                rShapeType = ESCHER_ShpInst_StraightConnector1;
                                AddOpt( ESCHER_Prop_cxstyle, ESCHER_cxstyleStraight );
                            }
                            break;
                        }
                        CreateLineProperties( aXPropSet, sal_False );
                        bRetValue = bSuppressRotation = sal_True;
                    }
                }
            }
        }
    }
    return bRetValue;
}

sal_Bool EscherPropertyContainer::CreateShadowProperties(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet )
{
    ::com::sun::star::uno::Any aAny;

    sal_Bool    bHasShadow = sal_False; // shadow is possible only if at least a fillcolor, linecolor or graphic is set
    sal_uInt32  nLineFlags = 0;         // default : shape has no line
    sal_uInt32  nFillFlags = 0x10;      //           shape is filled

    GetOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );
    GetOpt( ESCHER_Prop_fNoFillHitTest, nFillFlags );

    sal_uInt32 nDummy;
    sal_Bool bGraphic = GetOpt( DFF_Prop_pib, nDummy ) || GetOpt( DFF_Prop_pibName, nDummy ) || GetOpt( DFF_Prop_pibFlags, nDummy );

    sal_uInt32 nShadowFlags = 0x20000;
    if ( ( nLineFlags & 8 ) || ( nFillFlags & 0x10 ) || bGraphic )
    {
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                String( RTL_CONSTASCII_USTRINGPARAM( "Shadow" ) ), sal_True ) )
        {
            if ( aAny >>= bHasShadow )
            {
                if ( bHasShadow )
                {
                    nShadowFlags |= 2;
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                            String( RTL_CONSTASCII_USTRINGPARAM( "ShadowColor" ) ), sal_False ) )
                        AddOpt( ESCHER_Prop_shadowColor, ImplGetColor( *((sal_uInt32*)aAny.getValue()) ) );
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                            String( RTL_CONSTASCII_USTRINGPARAM( "ShadowXDistance" ) ), sal_False ) )
                        AddOpt( ESCHER_Prop_shadowOffsetX, *((sal_Int32*)aAny.getValue()) * 360 );
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                            String( RTL_CONSTASCII_USTRINGPARAM( "ShadowYDistance" ) ), sal_False ) )
                        AddOpt( ESCHER_Prop_shadowOffsetY, *((sal_Int32*)aAny.getValue()) * 360 );
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                            String( RTL_CONSTASCII_USTRINGPARAM( "ShadowTransparence" ) ), sal_False ) )
                        AddOpt( ESCHER_Prop_shadowOpacity,  0x10000 - (((sal_uInt32)*((sal_uInt16*)aAny.getValue())) * 655 ) );
                }
            }
        }
    }
    AddOpt( ESCHER_Prop_fshadowObscured, nShadowFlags );
    return bHasShadow;
}

sal_Int32 GetValueForEnhancedCustomShapeParameter( const com::sun::star::drawing::EnhancedCustomShapeParameter& rParameter, const std::vector< sal_Int32 >& rEquationOrder )
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
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::EQUATION :
        {
            nValue = (sal_uInt16)rEquationOrder[ nValue ];
            nValue |= (sal_uInt32)0x80000000;
        }
        break;
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::NORMAL :
        {

        }
        break;
    }
    return nValue;
}

sal_Bool GetValueForEnhancedCustomShapeHandleParameter( sal_Int32& nRetValue, const com::sun::star::drawing::EnhancedCustomShapeParameter& rParameter )
{
    sal_Bool bSpecial = sal_False;
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
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::EQUATION :
        {
            nRetValue += 3;
            bSpecial = sal_True;
        }
        break;
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::ADJUSTMENT :
        {
            nRetValue += 0x100;
            bSpecial = sal_True;
        }
        break;
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::TOP :
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::LEFT :
        {
            nRetValue = 0;
            bSpecial = sal_True;
        }
        break;
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::RIGHT :
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::BOTTOM :
        {
            nRetValue = 1;
            bSpecial = sal_True;
        }
        break;
        case com::sun::star::drawing::EnhancedCustomShapeParameterType::NORMAL :
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
        uno::Sequence< rtl::OUString > sEquationSource;
        const rtl::OUString sEquations( "Equations"  );
        SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)(const SdrCustomShapeGeometryItem&)
            pCustoShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
        const uno::Any* pAny = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sEquations );
        if ( pAny )
            *pAny >>= sEquationSource;
        sal_Int32 nEquationSourceCount = sEquationSource.getLength();
        if ( nEquationSourceCount )
        {
            sal_Int32 i;
            for ( i = 0; i < nEquationSourceCount; i++ )
            {
                EnhancedCustomShape2d aCustoShape2d( pCustoShape );
                try
                {
                    ::boost::shared_ptr< EnhancedCustomShape::ExpressionNode > aExpressNode(
                        EnhancedCustomShape::FunctionParser::parseFunction( sEquationSource[ i ], aCustoShape2d ) );
                    com::sun::star::drawing::EnhancedCustomShapeParameter aPara( aExpressNode->fillNode( rEquations, NULL, 0 ) );
                    if ( aPara.Type != com::sun::star::drawing::EnhancedCustomShapeParameterType::EQUATION )
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
                sal_Int32 nMask = 0x20000000;
                for( i = 0; i < 3; i++ )
                {
                    if ( aIter->nOperation & nMask )
                    {
                        aIter->nOperation ^= nMask;
                        aIter->nPara[ i ] = rEquationOrder[ aIter->nPara[ i ] & 0x3ff ] | 0x400;
                    }
                    nMask <<= 1;
                }
                ++aIter;
            }
        }
    }
}

sal_Bool EscherPropertyContainer::IsDefaultObject( SdrObjCustomShape* pCustoShape )
{
    sal_Bool bIsDefaultObject = sal_False;
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
        bIsDefaultObject = sal_True;
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
        if ( pData->nFlags & MSDFF_HANDLE_FLAGS_POLAR )
        {
        if ( ( pData->nPositionY >= 0x256 ) || ( pData->nPositionY <= 0x107 ) )
            nAdjustmentsWhichNeedsToBeConverted |= ( 1 << k );
        }
    }
    }
}

sal_Bool EscherPropertyContainer::GetAdjustmentValue( const com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue & rkProp, sal_Int32 nIndex, sal_Int32 nAdjustmentsWhichNeedsToBeConverted, sal_Int32& nValue )
{
    if ( rkProp.State != beans::PropertyState_DIRECT_VALUE )
    return sal_False;

    sal_Bool bUseFixedFloat = ( nAdjustmentsWhichNeedsToBeConverted & ( 1 << nIndex ) ) != 0;
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

    return sal_True;
}

void EscherPropertyContainer::CreateCustomShapeProperties( const MSO_SPT eShapeType, const uno::Reference< drawing::XShape > & rXShape )
{
    uno::Reference< beans::XPropertySet > aXPropSet( rXShape, uno::UNO_QUERY );
    if ( aXPropSet.is() )
    {
        SdrObjCustomShape* pCustoShape = (SdrObjCustomShape*)GetSdrObjectFromXShape( rXShape );
        const rtl::OUString sCustomShapeGeometry( "CustomShapeGeometry"  );
        uno::Any aGeoPropSet = aXPropSet->getPropertyValue( sCustomShapeGeometry );
        uno::Sequence< beans::PropertyValue > aGeoPropSeq;
        if ( aGeoPropSet >>= aGeoPropSeq )
        {
            const rtl::OUString sViewBox            ( "ViewBox"  );
            const rtl::OUString sTextRotateAngle    ( "TextRotateAngle"  );
            const rtl::OUString sExtrusion          ( "Extrusion"  );
            const rtl::OUString sEquations          ( "Equations"  );
            const rtl::OUString sPath               ( "Path"  );
            const rtl::OUString sTextPath           ( "TextPath"  );
            const rtl::OUString sHandles            ( "Handles"  );
            const rtl::OUString sAdjustmentValues   ( "AdjustmentValues"  );

            const beans::PropertyValue* pAdjustmentValuesProp = NULL;
            sal_Int32 nAdjustmentsWhichNeedsToBeConverted = 0;
            uno::Sequence< beans::PropertyValues > aHandlesPropSeq;
            sal_Bool bPredefinedHandlesUsed = sal_True;
            sal_Bool bIsDefaultObject = IsDefaultObject( pCustoShape );

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
                            const rtl::OUString sExtrusionBrightness            ( "Brightness"  );
                            const rtl::OUString sExtrusionDepth                 ( "Depth"  );
                            const rtl::OUString sExtrusionDiffusion             ( "Diffusion"  );
                            const rtl::OUString sExtrusionNumberOfLineSegments  ( "NumberOfLineSegments"  );
                            const rtl::OUString sExtrusionLightFace             ( "LightFace"  );
                            const rtl::OUString sExtrusionFirstLightHarsh       ( "FirstLightHarsh"  );
                            const rtl::OUString sExtrusionSecondLightHarsh      ( "SecondLightHarsh"  );
                            const rtl::OUString sExtrusionFirstLightLevel       ( "FirstLightLevel"  );
                            const rtl::OUString sExtrusionSecondLightLevel      ( "SecondLightLevel"  );
                            const rtl::OUString sExtrusionFirstLightDirection   ( "FirstLightDirection"  );
                            const rtl::OUString sExtrusionSecondLightDirection  ( "SecondLightDirection"  );
                            const rtl::OUString sExtrusionMetal                 ( "Metal"  );
                            const rtl::OUString sExtrusionShadeMode             ( "ShadeMode"  );
                            const rtl::OUString sExtrusionRotateAngle           ( "RotateAngle"  );
                            const rtl::OUString sExtrusionRotationCenter        ( "RotationCenter"  );
                            const rtl::OUString sExtrusionShininess             ( "Shininess"  );
                            const rtl::OUString sExtrusionSkew                  ( "Skew"  );
                            const rtl::OUString sExtrusionSpecularity           ( "Specularity"  );
                            const rtl::OUString sExtrusionProjectionMode        ( "ProjectionMode"  );
                            const rtl::OUString sExtrusionViewPoint             ( "ViewPoint"  );
                            const rtl::OUString sExtrusionOrigin                ( "Origin"  );
                            const rtl::OUString sExtrusionColor                 ( "Color"  );

                            if ( rrProp.Name.equals( sExtrusion ) )
                            {
                                sal_Bool bExtrusionOn = sal_Bool();
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
                                com::sun::star::drawing::EnhancedCustomShapeParameterPair aDepthParaPair;
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
                                sal_Bool bExtrusionLightFace = sal_Bool();
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
                                sal_Bool bExtrusionFirstLightHarsh = sal_Bool();
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
                                sal_Bool bExtrusionSecondLightHarsh = sal_Bool();
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
                                sal_Bool bExtrusionMetal = sal_Bool();
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
                                com::sun::star::drawing::EnhancedCustomShapeParameterPair aRotateAnglePair;
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
                                com::sun::star::drawing::EnhancedCustomShapeParameterPair aSkewParaPair;
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
                                com::sun::star::drawing::EnhancedCustomShapeParameterPair aOriginPair;
                                if ( ( rrProp.Value >>= aOriginPair ) && ( aOriginPair.First.Value >>= fExtrusionOriginX ) && ( aOriginPair.Second.Value >>= fExtrusionOriginY ) )
                                {
                                    AddOpt( DFF_Prop_c3DOriginX, (sal_Int32)( fExtrusionOriginX * 65536 ) );
                                    AddOpt( DFF_Prop_c3DOriginY, (sal_Int32)( fExtrusionOriginY * 65536 ) );
                                }
                            }
                            else if ( rrProp.Name.equals( sExtrusionColor ) )
                            {
                                sal_Bool bExtrusionColor = sal_Bool();
                                if ( rrProp.Value >>= bExtrusionColor )
                                {
                                    nLightFaceFlags |= 0x20000;
                                    if ( bExtrusionColor )
                                    {
                                        nLightFaceFlags |= 2;
                                        uno::Any aFillColor2;
                                        if ( EscherPropertyValueHelper::GetPropertyValue( aFillColor2, aXPropSet,
                                            String( RTL_CONSTASCII_USTRINGPARAM( "FillColor2" ) ), sal_True ) )
                                        {
                                            sal_uInt32 nFillColor = ImplGetColor( *((sal_uInt32*)aFillColor2.getValue()) );
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
                            aOut << nElements
                                << nElements
                                << nElementSize;

                            std::vector< EnhancedCustomShapeEquation >::const_iterator aIter( aEquations.begin() );
                            std::vector< EnhancedCustomShapeEquation >::const_iterator aEnd ( aEquations.end() );
                            while( aIter != aEnd )
                            {
                                aOut << (sal_uInt16)aIter->nOperation
                                     << (sal_Int16)aIter->nPara[ 0 ]
                                     << (sal_Int16)aIter->nPara[ 1 ]
                                     << (sal_Int16)aIter->nPara[ 2 ];
                                ++aIter;
                            }
                            sal_uInt8* pBuf = new sal_uInt8[ nStreamSize ];
                            memcpy( pBuf, aOut.GetData(), nStreamSize );
                            AddOpt( DFF_Prop_pFormulas, sal_True, nStreamSize - 6, pBuf, nStreamSize );
                        }
                        else
                        {
                            sal_uInt8* pBuf = new sal_uInt8[ 1 ];
                            AddOpt( DFF_Prop_pFormulas, sal_True, 0, pBuf, 0 );
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
                            const rtl::OUString sPathExtrusionAllowed               ( "ExtrusionAllowed"  );
                            const rtl::OUString sPathConcentricGradientFillAllowed  ( "ConcentricGradientFillAllowed"  );
                            const rtl::OUString sPathTextPathAllowed                ( "TextPathAllowed"  );
                            const rtl::OUString sPathCoordinates                    ( "Coordinates"  );
                            const rtl::OUString sPathGluePoints                     ( "GluePoints"  );
                            const rtl::OUString sPathGluePointType                  ( "GluePointType"  );
                            const rtl::OUString sPathSegments                       ( "Segments"  );
                            const rtl::OUString sPathStretchX                       ( "StretchX"  );
                            const rtl::OUString sPathStretchY                       ( "StretchY"  );
                            const rtl::OUString sPathTextFrames                     ( "TextFrames"  );

                            if ( rrProp.Name.equals( sPathExtrusionAllowed ) )
                            {
                                sal_Bool bExtrusionAllowed = sal_Bool();
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
                                sal_Bool bConcentricGradientFillAllowed = sal_Bool();
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
                                sal_Bool bTextPathAllowed = sal_Bool();
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
                                    com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair > aCoordinates;
                                    if ( rrProp.Value >>= aCoordinates )
                                    {
                                        // creating the vertices
                                        if ( (sal_uInt16)aCoordinates.getLength() )
                                        {
                                            sal_uInt16 j, nElements = (sal_uInt16)aCoordinates.getLength();
                                            sal_uInt16 nElementSize = 8;
                                            sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                                            SvMemoryStream aOut( nStreamSize );
                                            aOut << nElements
                                                << nElements
                                                << nElementSize;
                                            for( j = 0; j < nElements; j++ )
                                            {
                                                sal_Int32 X = GetValueForEnhancedCustomShapeParameter( aCoordinates[ j ].First, aEquationOrder );
                                                sal_Int32 Y = GetValueForEnhancedCustomShapeParameter( aCoordinates[ j ].Second, aEquationOrder );
                                                aOut << X
                                                    << Y;
                                            }
                                            sal_uInt8* pBuf = new sal_uInt8[ nStreamSize ];
                                            memcpy( pBuf, aOut.GetData(), nStreamSize );
                                            AddOpt( DFF_Prop_pVertices, sal_True, nStreamSize - 6, pBuf, nStreamSize ); // -6
                                        }
                                        else
                                        {
                                            sal_uInt8* pBuf = new sal_uInt8[ 1 ];
                                            AddOpt( DFF_Prop_pVertices, sal_True, 0, pBuf, 0 );
                                        }
                                    }
                                }
                            }
                            else if ( rrProp.Name.equals( sPathGluePoints ) )
                            {
                                if ( !bIsDefaultObject )
                                {
                                    com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair> aGluePoints;
                                    if ( rrProp.Value >>= aGluePoints )
                                    {
                                        // creating the vertices
                                        sal_uInt16 nElements = (sal_uInt16)aGluePoints.getLength();
                                        if ( nElements )
                                        {
                                            sal_uInt16 j, nElementSize = 8;
                                            sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                                            SvMemoryStream aOut( nStreamSize );
                                            aOut << nElements
                                                << nElements
                                                << nElementSize;
                                            for( j = 0; j < nElements; j++ )
                                            {
                                                sal_Int32 X = GetValueForEnhancedCustomShapeParameter( aGluePoints[ j ].First, aEquationOrder );
                                                sal_Int32 Y = GetValueForEnhancedCustomShapeParameter( aGluePoints[ j ].Second, aEquationOrder );
                                                aOut << X
                                                    << Y;
                                            }
                                            sal_uInt8* pBuf = new sal_uInt8[ nStreamSize ];
                                            memcpy( pBuf, aOut.GetData(), nStreamSize );
                                            AddOpt( DFF_Prop_connectorPoints, sal_True, nStreamSize - 6, pBuf, nStreamSize );   // -6
                                        }
                                        else
                                        {
                                            sal_uInt8* pBuf = new sal_uInt8[ 1 ];
                                            AddOpt( DFF_Prop_connectorPoints, sal_True, 0, pBuf, 0 );
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
                                    com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeSegment > aSegments;
                                    if ( rrProp.Value >>= aSegments )
                                    {
                                        // creating seginfo
                                        if ( (sal_uInt16)aSegments.getLength() )
                                        {
                                            sal_uInt16 j, nElements = (sal_uInt16)aSegments.getLength();
                                            sal_uInt16 nElementSize = 2;
                                            sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                                            SvMemoryStream aOut( nStreamSize );
                                            aOut << nElements
                                                << nElements
                                                << nElementSize;
                                            for ( j = 0; j < nElements; j++ )
                                            {
                                                sal_uInt16 nVal = (sal_uInt16)aSegments[ j ].Count;
                                                switch( aSegments[ j ].Command )
                                                {
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::UNKNOWN :
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::LINETO : break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::MOVETO :
                                                    {
                                                        nVal = 0x4000;
                                                    }
                                                    break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CURVETO :
                                                    {
                                                        nVal |= 0x2000;
                                                    }
                                                    break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CLOSESUBPATH :
                                                    {
                                                        nVal = 0x6001;
                                                    }
                                                    break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ENDSUBPATH :
                                                    {
                                                        nVal = 0x8000;
                                                    }
                                                    break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::NOFILL :
                                                    {
                                                        nVal = 0xaa00;
                                                    }
                                                    break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::NOSTROKE :
                                                    {
                                                        nVal = 0xab00;
                                                    }
                                                    break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO :
                                                    {
                                                        nVal *= 3;
                                                        nVal |= 0xa100;
                                                    }
                                                    break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE :
                                                    {
                                                        nVal *= 3;
                                                        nVal |= 0xa200;
                                                    }
                                                    break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ARCTO :
                                                    {
                                                        nVal <<= 2;
                                                        nVal |= 0xa300;
                                                    }
                                                    break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ARC :
                                                    {
                                                        nVal <<= 2;
                                                        nVal |= 0xa400;
                                                    }
                                                    break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO :
                                                    {
                                                        nVal <<= 2;
                                                        nVal |= 0xa500;
                                                    }
                                                    break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARC :
                                                    {
                                                        nVal <<= 2;
                                                        nVal |= 0xa600;
                                                    }
                                                    break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX :
                                                    {
                                                        nVal |= 0xa700;
                                                    }
                                                    break;
                                                    case com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY :
                                                    {
                                                        nVal |= 0xa800;
                                                    }
                                                    break;
                                                }
                                                aOut << nVal;
                                            }
                                            sal_uInt8* pBuf = new sal_uInt8[ nStreamSize ];
                                            memcpy( pBuf, aOut.GetData(), nStreamSize );
                                            AddOpt( DFF_Prop_pSegmentInfo, sal_False, nStreamSize - 6, pBuf, nStreamSize );
                                        }
                                        else
                                        {
                                            sal_uInt8* pBuf = new sal_uInt8[ 1 ];
                                            AddOpt( DFF_Prop_pSegmentInfo, sal_True, 0, pBuf, 0 );
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
                                    com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeTextFrame > aPathTextFrames;
                                    if ( rrProp.Value >>= aPathTextFrames )
                                    {
                                        if ( (sal_uInt16)aPathTextFrames.getLength() )
                                        {
                                            sal_uInt16 j, nElements = (sal_uInt16)aPathTextFrames.getLength();
                                            sal_uInt16 nElementSize = 16;
                                            sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                                            SvMemoryStream aOut( nStreamSize );
                                            aOut << nElements
                                                << nElements
                                                << nElementSize;
                                            for ( j = 0; j < nElements; j++ )
                                            {
                                                sal_Int32 nLeft = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].TopLeft.First, aEquationOrder );
                                                sal_Int32 nTop  = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].TopLeft.Second, aEquationOrder );
                                                sal_Int32 nRight = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].BottomRight.First, aEquationOrder );
                                                sal_Int32 nBottom = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].BottomRight.Second, aEquationOrder );

                                                aOut << nLeft
                                                    << nTop
                                                    << nRight
                                                    << nBottom;
                                            }
                                            sal_uInt8* pBuf = new sal_uInt8[ nStreamSize ];
                                            memcpy( pBuf, aOut.GetData(), nStreamSize );
                                            AddOpt( DFF_Prop_textRectangles, sal_True, nStreamSize - 6, pBuf, nStreamSize );
                                        }
                                        else
                                        {
                                            sal_uInt8* pBuf = new sal_uInt8[ 1 ];
                                            AddOpt( DFF_Prop_textRectangles, sal_True, 0, pBuf, 0 );
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
                            const rtl::OUString sTextPathMode       ( "TextPathMode"  );
                            const rtl::OUString sTextPathScaleX     ( "ScaleX"  );
                            const rtl::OUString sSameLetterHeights  ( "SameLetterHeights"  );

                            if ( rrProp.Name.equals( sTextPath ) )
                            {
                                sal_Bool bTextPathOn = sal_Bool();
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
                                com::sun::star::drawing::EnhancedCustomShapeTextPathMode eTextPathMode;
                                if ( rrProp.Value >>= eTextPathMode )
                                {
                                    nTextPathFlags |= 0x05000000;
                                    nTextPathFlags &=~0x500;    // TextPathMode_NORMAL
                                    if ( eTextPathMode == com::sun::star::drawing::EnhancedCustomShapeTextPathMode_PATH )
                                        nTextPathFlags |= 0x100;
                                    else if ( eTextPathMode == com::sun::star::drawing::EnhancedCustomShapeTextPathMode_SHAPE )
                                        nTextPathFlags |= 0x500;
                                }
                            }
                            else if ( rrProp.Name.equals( sTextPathScaleX ) )
                            {
                                sal_Bool bTextPathScaleX = sal_Bool();
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
                                sal_Bool bSameLetterHeights = sal_Bool();
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
                            rtl::OUString aText;
                            uno::Reference< text::XSimpleText > xText( rXShape, uno::UNO_QUERY );
                            if ( xText.is() )
                                aText = xText->getString();
                            if ( aText.isEmpty() )
                                aText = ::rtl::OUString( "your text" );   // TODO: moving into a resource
                            AddOpt( DFF_Prop_gtextUNICODE, aText );

                            // FontWork Font
                            rtl::OUString aFontName;
                            const rtl::OUString sCharFontName           ( "CharFontName"  );
                            uno::Any aAny = aXPropSet->getPropertyValue( sCharFontName );
                            aAny >>= aFontName;
                            if ( aFontName.isEmpty() )
                                aFontName = ::rtl::OUString( "Arial Black" );
                            AddOpt( DFF_Prop_gtextFont, aFontName );

                            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "CharScaleWidth" ) ), sal_True ) )
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
                            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "CharKerning" ) ), sal_True ) )
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
                            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "CharPosture" ) ), sal_True ) )
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
                            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "CharWeight" ) ), sal_True ) )
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
                        }
                        if ( nTextPathFlags != nTextPathFlagsOrg )
                            AddOpt( DFF_Prop_gtextFStrikethrough, nTextPathFlags );
                    }
                }
                else if ( rProp.Name.equals( sHandles ) )
                {
                    if ( !bIsDefaultObject )
                    {
                        bPredefinedHandlesUsed = sal_False;
                        if ( rProp.Value >>= aHandlesPropSeq )
                        {
                            sal_uInt16 nElements = (sal_uInt16)aHandlesPropSeq.getLength();
                            if ( nElements )
                            {
                                sal_uInt16 k, j, nElementSize = 36;
                                sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                                SvMemoryStream aOut( nStreamSize );
                                aOut << nElements
                                    << nElements
                                    << nElementSize;

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

                                        const rtl::OUString sPosition           ( "Position"  );
                                        const rtl::OUString sMirroredX          ( "MirroredX"  );
                                        const rtl::OUString sMirroredY          ( "MirroredY"  );
                                        const rtl::OUString sSwitched           ( "Switched"  );
                                        const rtl::OUString sPolar              ( "Polar"  );
                                        const rtl::OUString sRadiusRangeMinimum ( "RadiusRangeMinimum"  );
                                        const rtl::OUString sRadiusRangeMaximum ( "RadiusRangeMaximum"  );
                                        const rtl::OUString sRangeXMinimum      ( "RangeXMinimum"  );
                                        const rtl::OUString sRangeXMaximum      ( "RangeXMaximum"  );
                                        const rtl::OUString sRangeYMinimum      ( "RangeYMinimum"  );
                                        const rtl::OUString sRangeYMaximum      ( "RangeYMaximum"  );

                                        if ( rPropVal.Name.equals( sPosition ) )
                                        {
                                            com::sun::star::drawing::EnhancedCustomShapeParameterPair aPosition;
                                            if ( rPropVal.Value >>= aPosition )
                                            {
                                                GetValueForEnhancedCustomShapeHandleParameter( nXPosition, aPosition.First );
                                                GetValueForEnhancedCustomShapeHandleParameter( nYPosition, aPosition.Second );
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sMirroredX ) )
                                        {
                                            sal_Bool bMirroredX = sal_Bool();
                                            if ( rPropVal.Value >>= bMirroredX )
                                            {
                                                if ( bMirroredX )
                                                    nFlags |= 1;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sMirroredY ) )
                                        {
                                            sal_Bool bMirroredY = sal_Bool();
                                            if ( rPropVal.Value >>= bMirroredY )
                                            {
                                                if ( bMirroredY )
                                                    nFlags |= 2;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sSwitched ) )
                                        {
                                            sal_Bool bSwitched = sal_Bool();
                                            if ( rPropVal.Value >>= bSwitched )
                                            {
                                                if ( bSwitched )
                                                    nFlags |= 4;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sPolar ) )
                                        {
                                            com::sun::star::drawing::EnhancedCustomShapeParameterPair aPolar;
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

                                            com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
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

                                            com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
                                            if ( rPropVal.Value >>= aRadiusRangeMaximum )
                                            {
                                                if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMax, aRadiusRangeMaximum ) )
                                                    nFlags |= 0x100;
                                                nFlags |= 0x2000;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sRangeXMinimum ) )
                                        {
                                            com::sun::star::drawing::EnhancedCustomShapeParameter aXRangeMinimum;
                                            if ( rPropVal.Value >>= aXRangeMinimum )
                                            {
                                                if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMin, aXRangeMinimum ) )
                                                    nFlags |= 0x80;
                                                nFlags |= 0x20;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sRangeXMaximum ) )
                                        {
                                            com::sun::star::drawing::EnhancedCustomShapeParameter aXRangeMaximum;
                                            if ( rPropVal.Value >>= aXRangeMaximum )
                                            {
                                                if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMax, aXRangeMaximum ) )
                                                    nFlags |= 0x100;
                                                nFlags |= 0x20;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sRangeYMinimum ) )
                                        {
                                            com::sun::star::drawing::EnhancedCustomShapeParameter aYRangeMinimum;
                                            if ( rPropVal.Value >>= aYRangeMinimum )
                                            {
                                                if ( GetValueForEnhancedCustomShapeHandleParameter( nYRangeMin, aYRangeMinimum ) )
                                                    nFlags |= 0x200;
                                                nFlags |= 0x20;
                                            }
                                        }
                                        else if ( rPropVal.Name.equals( sRangeYMaximum ) )
                                        {
                                            com::sun::star::drawing::EnhancedCustomShapeParameter aYRangeMaximum;
                                            if ( rPropVal.Value >>= aYRangeMaximum )
                                            {
                                                if ( GetValueForEnhancedCustomShapeHandleParameter( nYRangeMax, aYRangeMaximum ) )
                                                    nFlags |= 0x400;
                                                nFlags |= 0x20;
                                            }
                                        }
                                    }
                                    aOut << nFlags
                                        << nXPosition
                                        << nYPosition
                                        << nXMap
                                        << nYMap
                                        << nXRangeMin
                                        << nXRangeMax
                                        << nYRangeMin
                                        << nYRangeMax;

                                    if ( nFlags & 8 )
                                        nAdjustmentsWhichNeedsToBeConverted |= ( 1 << ( nYPosition - 0x100 ) );
                                }
                                sal_uInt8* pBuf = new sal_uInt8[ nStreamSize ];
                                memcpy( pBuf, aOut.GetData(), nStreamSize );
                                AddOpt( DFF_Prop_Handles, sal_True, nStreamSize - 6, pBuf, nStreamSize );
                            }
                            else
                            {
                                sal_uInt8* pBuf = new sal_uInt8[ 1 ];
                                AddOpt( DFF_Prop_Handles, sal_True, 0, pBuf, 0 );
                            }
                        }
                    }
                }
                else if ( rProp.Name.equals( sAdjustmentValues ) )
                {
                    // it is required, that the information which handle is polar has already be read,
                    // so we are able to change the polar value to a fixed float
                    pAdjustmentValuesProp = &rProp;
                }
            }
            if ( pAdjustmentValuesProp )
            {
                uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq;
                if ( pAdjustmentValuesProp->Value >>= aAdjustmentSeq )
                {
                    if ( bPredefinedHandlesUsed )
                        LookForPolarHandles( eShapeType, nAdjustmentsWhichNeedsToBeConverted );

                    sal_Int32 k, nValue = 0, nAdjustmentValues = aAdjustmentSeq.getLength();
                    for ( k = 0; k < nAdjustmentValues; k++ )
                        if( GetAdjustmentValue( aAdjustmentSeq[ k ], k, nAdjustmentsWhichNeedsToBeConverted, nValue ) )
                            AddOpt( (sal_uInt16)( DFF_Prop_adjustValue + k ), (sal_uInt32)nValue );
                }
            }
        }
    }
}

MSO_SPT EscherPropertyContainer::GetCustomShapeType( const uno::Reference< drawing::XShape > & rXShape, sal_uInt32& nMirrorFlags, rtl::OUString& rShapeType )
{
    MSO_SPT eShapeType = mso_sptNil;
    nMirrorFlags = 0;
    uno::Reference< beans::XPropertySet > aXPropSet( rXShape, uno::UNO_QUERY );
    if ( aXPropSet.is() )
    {
        try
        {
            const OUString  sCustomShapeGeometry( RTL_CONSTASCII_USTRINGPARAM ( "CustomShapeGeometry" ) );
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
                            eShapeType = EnhancedCustomShapeTypeNames::Get( rShapeType );
                    }
                    else if ( rProp.Name == "MirroredX" )
                    {
                        sal_Bool bMirroredX = sal_Bool();
                        if ( ( rProp.Value >>= bMirroredX ) && bMirroredX )
                            nMirrorFlags  |= SHAPEFLAG_FLIPH;
                    }
                    else if ( rProp.Name == "MirroredY" )
                    {
                        sal_Bool bMirroredY = sal_Bool();
                        if ( ( rProp.Value >>= bMirroredY ) && bMirroredY )
                            nMirrorFlags  |= SHAPEFLAG_FLIPV;
                    }
                }
            }
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }
    }
    return eShapeType;
}

MSO_SPT EscherPropertyContainer::GetCustomShapeType( const uno::Reference< drawing::XShape > & rXShape, sal_uInt32& nMirrorFlags )
{
    rtl::OUString aShapeType;
    return GetCustomShapeType( rXShape, nMirrorFlags, aShapeType );
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

sal_Bool EscherPersistTable::PtIsID( sal_uInt32 nID )
{
    for( size_t i = 0, n = maPersistTable.size(); i < n; ++i ) {
        EscherPersistEntry* pPtr = maPersistTable[ i ];
        if ( pPtr->mnID == nID ) {
            return sal_True;
        }
    }
    return sal_False;
}

void EscherPersistTable::PtInsert( sal_uInt32 nID, sal_uInt32 nOfs )
{
    maPersistTable.push_back( new EscherPersistEntry( nID, nOfs ) );
}

sal_uInt32 EscherPersistTable::PtDelete( sal_uInt32 nID )
{
    EscherPersistTable_impl::iterator it = maPersistTable.begin();
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

sal_Bool EscherPropertyValueHelper::GetPropertyValue(
    ::com::sun::star::uno::Any& rAny,
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
    const String& rString,
    sal_Bool bTestPropertyAvailability
) {
    sal_Bool bRetValue = sal_True;
    if ( bTestPropertyAvailability )
    {
        bRetValue = sal_False;
        try
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                aXPropSetInfo( rXPropSet->getPropertySetInfo() );
            if ( aXPropSetInfo.is() )
                bRetValue = aXPropSetInfo->hasPropertyByName( rString );
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
            bRetValue = sal_False;
        }
    }
    if ( bRetValue )
    {
        try
        {
            rAny = rXPropSet->getPropertyValue( rString );
            if ( !rAny.hasValue() )
                bRetValue = sal_False;
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
            bRetValue = sal_False;
        }
    }
    return bRetValue;
}

::com::sun::star::beans::PropertyState EscherPropertyValueHelper::GetPropertyState(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
        const String& rPropertyName )
{
    ::com::sun::star::beans::PropertyState eRetValue = ::com::sun::star::beans::PropertyState_AMBIGUOUS_VALUE;
    try
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState > aXPropState
                ( rXPropSet, ::com::sun::star::uno::UNO_QUERY );
        if ( aXPropState.is() )
            eRetValue = aXPropState->getPropertyState( rPropertyName );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
    return eRetValue;
}

EscherBlibEntry::EscherBlibEntry( sal_uInt32 nPictureOffset, const GraphicObject& rObject, const rtl::OString& rId,
                                        const GraphicAttr* pGraphicAttr ) :
    mnPictureOffset ( nPictureOffset ),
    mnRefCount      ( 1 ),
    mnSizeExtra     ( 0 ),
    maPrefSize      ( rObject.GetPrefSize() ),
    maPrefMapMode   ( rObject.GetPrefMapMode() ),
    mbIsEmpty       ( sal_True )
{
    mbIsNativeGraphicPossible = ( pGraphicAttr == NULL );
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
                aSt << static_cast<sal_uInt16>(pGraphicAttr->GetDrawMode())
                    << static_cast<sal_uInt32>(pGraphicAttr->GetMirrorFlags())
                    << static_cast<sal_Int32>(pGraphicAttr->GetLeftCrop())
                    << static_cast<sal_Int32>(pGraphicAttr->GetTopCrop())
                    << static_cast<sal_Int32>(pGraphicAttr->GetRightCrop())
                    << static_cast<sal_Int32>(pGraphicAttr->GetBottomCrop())
                    << pGraphicAttr->GetRotation()
                    << pGraphicAttr->GetLuminance()
                    << pGraphicAttr->GetContrast()
                    << pGraphicAttr->GetChannelR()
                    << pGraphicAttr->GetChannelG()
                    << pGraphicAttr->GetChannelB()
                    << pGraphicAttr->GetGamma()
                    << (sal_Bool)( pGraphicAttr->IsInvert() == sal_True )
                    << pGraphicAttr->GetTransparency();
                mnIdentifier[ 1 ] = rtl_crc32( 0, aSt.GetData(), aSt.Tell() );
            }
            else
                mbIsNativeGraphicPossible = sal_True;
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
        mbIsEmpty = sal_False;
    }
};

void EscherBlibEntry::WriteBlibEntry( SvStream& rSt, sal_Bool bWritePictureOffset, sal_uInt32 nResize )
{
    sal_uInt32  nPictureOffset = ( bWritePictureOffset ) ? mnPictureOffset : 0;

    rSt << (sal_uInt32)( ( ESCHER_BSE << 16 ) | ( ( (sal_uInt16)meBlibType << 4 ) | 2 ) )
        << (sal_uInt32)( 36 + nResize )
        << (sal_uInt8)meBlibType;

    switch ( meBlibType )
    {
        case EMF :
        case WMF :  // converting EMF/WMF on OS2 to Pict
            rSt << (sal_uInt8)PICT;
        break;
        default:
            rSt << (sal_uInt8)meBlibType;
    };

    rSt.Write( &mnIdentifier[ 0 ], 16 );
    rSt << (sal_uInt16)0
        << (sal_uInt32)( mnSize + mnSizeExtra )
        << mnRefCount
        << nPictureOffset
        << (sal_uInt32)0;
}

EscherBlibEntry::~EscherBlibEntry()
{
};

sal_Bool EscherBlibEntry::operator==( const EscherBlibEntry& rEscherBlibEntry ) const
{
    for ( int i = 0; i < 3; i++ )
    {
        if ( mnIdentifier[ i ] != rEscherBlibEntry.mnIdentifier[ i ] )
            return sal_False;
    }
    return sal_True;
}

EscherGraphicProvider::EscherGraphicProvider( sal_uInt32 nFlags ) :
    mnFlags         ( nFlags ),
    mpBlibEntrys    ( NULL ),
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

sal_Bool EscherGraphicProvider::WriteBlibStoreEntry(SvStream& rSt,
    sal_uInt32 nBlipId, sal_Bool bWritePictureOffSet, sal_uInt32 nResize)
{
    if (nBlipId > mnBlibEntrys || nBlipId == 0)
        return sal_False;
    mpBlibEntrys[nBlipId-1]->WriteBlibEntry(rSt, bWritePictureOffSet, nResize);
    return sal_True;
}

void EscherGraphicProvider::WriteBlibStoreContainer( SvStream& rSt, SvStream* pMergePicStreamBSE )
{
    sal_uInt32  nSize = GetBlibStoreContainerSize( pMergePicStreamBSE );
    if ( nSize )
    {
        rSt << (sal_uInt32)( ( ESCHER_BstoreContainer << 16 ) | 0x1f )
            << (sal_uInt32)( nSize - 8 );

        if ( pMergePicStreamBSE )
        {
            sal_uInt32 i, nBlipSize, nOldPos = pMergePicStreamBSE->Tell();
            const sal_uInt32 nBuf = 0x40000;    // 256KB buffer
            sal_uInt8* pBuf = new sal_uInt8[ nBuf ];

            for ( i = 0; i < mnBlibEntrys; i++ )
            {
                EscherBlibEntry* pBlibEntry = mpBlibEntrys[ i ];

                ESCHER_BlibType nBlibType = pBlibEntry->meBlibType;
                nBlipSize = pBlibEntry->mnSize + pBlibEntry->mnSizeExtra;
                pBlibEntry->WriteBlibEntry( rSt, sal_False, nBlipSize );

                // BLIP
                pMergePicStreamBSE->Seek( pBlibEntry->mnPictureOffset );
                sal_uInt16 n16;
                // record version and instance
                *pMergePicStreamBSE >> n16;
                rSt << n16;
                // record type
                *pMergePicStreamBSE >> n16;
                rSt << sal_uInt16( ESCHER_BlipFirst + nBlibType );
                DBG_ASSERT( n16 == ESCHER_BlipFirst + nBlibType , "EscherGraphicProvider::WriteBlibStoreContainer: BLIP record types differ" );
                sal_uInt32 n32;
                // record size
                *pMergePicStreamBSE >> n32;
                nBlipSize -= 8;
                rSt << nBlipSize;
                DBG_ASSERT( nBlipSize == n32, "EscherGraphicProvider::WriteBlibStoreContainer: BLIP sizes differ" );
                // record
                while ( nBlipSize )
                {
                    sal_uInt32 nBytes = ( nBlipSize > nBuf ? nBuf : nBlipSize );
                    pMergePicStreamBSE->Read( pBuf, nBytes );
                    rSt.Write( pBuf, nBytes );
                    nBlipSize -= nBytes;
                }
            }
            delete[] pBuf;
            pMergePicStreamBSE->Seek( nOldPos );
        }
        else
        {
            for ( sal_uInt32 i = 0; i < mnBlibEntrys; i++ )
                mpBlibEntrys[ i ]->WriteBlibEntry( rSt, sal_True );
        }
    }
}

sal_Bool EscherGraphicProvider::GetPrefSize( const sal_uInt32 nBlibId, Size& rPrefSize, MapMode& rPrefMapMode )
{
    sal_Bool bInRange = nBlibId && ( ( nBlibId - 1 ) < mnBlibEntrys );
    if ( bInRange )
    {
        EscherBlibEntry* pEntry = mpBlibEntrys[ nBlibId - 1 ];
        rPrefSize = pEntry->maPrefSize;
        rPrefMapMode = pEntry->maPrefMapMode;
    }
    return bInRange;
}

sal_uInt32 EscherGraphicProvider::GetBlibID( SvStream& rPicOutStrm, const rtl::OString& rId,
                                            const Rectangle& /* rBoundRect */, const com::sun::star::awt::Rectangle* pVisArea, const GraphicAttr* pGraphicAttr )
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

        sal_Bool            bUseNativeGraphic( sal_False );

        Graphic             aGraphic( aGraphicObject.GetTransformedGraphic( pGraphicAttr ) );
        GfxLink             aGraphicLink;
        SvMemoryStream      aStream;

        const sal_uInt8*    pGraphicAry = NULL;

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
                    bUseNativeGraphic = sal_True;
            }
        }
        if ( !bUseNativeGraphic )
        {
            GraphicType eGraphicType = aGraphic.GetType();
            if ( ( eGraphicType == GRAPHIC_BITMAP ) || ( eGraphicType == GRAPHIC_GDIMETAFILE ) )
            {
                sal_uInt32 nErrCode;
                if ( !aGraphic.IsAnimated() )
                    nErrCode = GraphicConverter::Export( aStream, aGraphic, ( eGraphicType == GRAPHIC_BITMAP ) ? CVT_PNG  : CVT_EMF );
                else
                {   // to store a animation, a gif has to be included into the msOG chunk of a png  #I5583#
                    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
                    SvMemoryStream  aGIFStream;
                    aGIFStream.Write(RTL_CONSTASCII_STRINGPARAM("MSOFFICE9.0"));
                    nErrCode = rFilter.ExportGraphic( aGraphic, String(), aGIFStream,
                        rFilter.GetExportFormatNumberForShortName( String( RTL_CONSTASCII_USTRINGPARAM( "GIF" ) ) ), NULL );
                    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aFilterData( 1 );
                    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aAdditionalChunkSequence( 1 );
                    sal_uInt32 nGIFSreamLen = aGIFStream.Tell();
                    com::sun::star::uno::Sequence< sal_Int8 > aGIFSeq( nGIFSreamLen );
                    sal_Int8* pSeq = aGIFSeq.getArray();
                    aGIFStream.Seek( STREAM_SEEK_TO_BEGIN );
                    aGIFStream.Read( pSeq, nGIFSreamLen );
                    com::sun::star::beans::PropertyValue aChunkProp, aFilterProp;
                    aChunkProp.Name = String( RTL_CONSTASCII_USTRINGPARAM( "msOG" ) );
                    aChunkProp.Value <<= aGIFSeq;
                    aAdditionalChunkSequence[ 0 ] = aChunkProp;
                    aFilterProp.Name = String( RTL_CONSTASCII_USTRINGPARAM( "AdditionalChunks" ) );
                    aFilterProp.Value <<= aAdditionalChunkSequence;
                    aFilterData[ 0 ] = aFilterProp;
                    nErrCode = rFilter.ExportGraphic( aGraphic, String(), aStream,
                        rFilter.GetExportFormatNumberForShortName( String( RTL_CONSTASCII_USTRINGPARAM( "PNG" ) ) ), &aFilterData );
                }
                if ( nErrCode == ERRCODE_NONE )
                {
                    p_EscherBlibEntry->meBlibType = ( eGraphicType == GRAPHIC_BITMAP ) ? PNG : EMF;
                    aStream.Seek( STREAM_SEEK_TO_END );
                    p_EscherBlibEntry->mnSize = aStream.Tell();
                    pGraphicAry = (sal_uInt8*)aStream.GetData();

                    if ( p_EscherBlibEntry->meBlibType == WMF )     // the fileheader is not used
                    {
                        p_EscherBlibEntry->mnSize -= 22;
                        pGraphicAry += 22;
                    }
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
                rPicOutStrm << (sal_uInt32)( 0x7f90000 | (sal_uInt16)( mnBlibEntrys << 4 ) )
                            << (sal_uInt32)0;
                nAtomSize = rPicOutStrm.Tell();
                 if ( eBlibType == PNG )
                    rPicOutStrm << (sal_uInt16)0x0606;
                else if ( eBlibType == WMF )
                    rPicOutStrm << (sal_uInt16)0x0403;
                else if ( eBlibType == EMF )
                    rPicOutStrm << (sal_uInt16)0x0402;
                else if ( eBlibType == PEG )
                    rPicOutStrm << (sal_uInt16)0x0505;
            }
            if ( ( eBlibType == PEG ) || ( eBlibType == PNG ) )
            {
                nExtra = 17;
                p_EscherBlibEntry->mnSizeExtra = nExtra + 8;
                nInstance = ( eBlibType == PNG ) ? 0xf01e6e00 : 0xf01d46a0;
                rPicOutStrm << nInstance << (sal_uInt32)( p_EscherBlibEntry->mnSize + nExtra );
                rPicOutStrm.Write( p_EscherBlibEntry->mnIdentifier, 16 );
                rPicOutStrm << (sal_uInt8)0xff;
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
                pGraphicAry = (sal_uInt8*)aDestStrm.GetData();
                if ( p_EscherBlibEntry->mnSize && pGraphicAry )
                {
                    nExtra = eBlibType == WMF ? 0x42 : 0x32;                                    // !EMF -> no change
                    p_EscherBlibEntry->mnSizeExtra = nExtra + 8;
                    nInstance = ( eBlibType == WMF ) ? 0xf01b2170 : 0xf01a3d40;                 // !EMF -> no change
                    rPicOutStrm << nInstance << (sal_uInt32)( p_EscherBlibEntry->mnSize + nExtra );
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
                    rPicOutStrm << nUncompressedSize // WMFSize without FileHeader
                    << (sal_Int32)0     // since we can't find out anymore what the original size of
                    << (sal_Int32)0     // the WMF (without Fileheader) was we write 10cm / x
                    << nPrefWidth
                    << nPrefHeight
                    << nWidth
                    << nHeight
                    << p_EscherBlibEntry->mnSize
                    << (sal_uInt16)0xfe00;  // compression Flags
                    rPicOutStrm.Write( pGraphicAry, p_EscherBlibEntry->mnSize );
                }
            }
            if ( nAtomSize )
            {
                sal_uInt32  nPos = rPicOutStrm.Tell();
                rPicOutStrm.Seek( nAtomSize - 4 );
                rPicOutStrm << (sal_uInt32)( nPos - nAtomSize );
                rPicOutStrm.Seek( nPos );
            }
            nBlibId = ImplInsertBlib( p_EscherBlibEntry ), p_EscherBlibEntry = NULL;
        }
    }
    if ( p_EscherBlibEntry )
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
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   aXShape;
    sal_uInt32          n_EscherId;

                        EscherShapeListEntry( const ::com::sun::star::uno::Reference
                            < ::com::sun::star::drawing::XShape > & rShape, sal_uInt32 nId ) :
                                        aXShape     ( rShape ),
                                        n_EscherId  ( nId ) {}
};

sal_uInt32 EscherConnectorListEntry::GetClosestPoint( const Polygon& rPoly, const ::com::sun::star::awt::Point& rPoint )
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

// ---------------------------------------------------------------------------------------------
// for rectangles          for ellipses     for polygons
//
// nRule =  0 ->Top         0 ->Top         nRule = Index auf ein (Poly)Polygon Punkt
//          1 ->Left        2 ->Left
//          2 ->Bottom      4 ->Bottom
//          3 ->Right       6 ->Right

sal_uInt32 EscherConnectorListEntry::GetConnectorRule( sal_Bool bFirst )
{
    sal_uInt32 nRule = 0;

    ::com::sun::star::uno::Any aAny;
    ::com::sun::star::awt::Point aRefPoint( ( bFirst ) ? maPointA : maPointB );
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        aXShape( ( bFirst ) ? mXConnectToA : mXConnectToB );

    rtl::OUString aString(aXShape->getShapeType());
    rtl::OStringBuffer aBuf(rtl::OUStringToOString(aString, RTL_TEXTENCODING_UTF8));
    aBuf.remove( 0, 13 );   // removing "com.sun.star."
    sal_Int16 nPos = aBuf.toString().indexOf("Shape");
    aBuf.remove(nPos, 5);
    rtl::OString aType = aBuf.makeStringAndClear();

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
        aPropertySet( aXShape, ::com::sun::star::uno::UNO_QUERY );

    if (aType.equalsL(RTL_CONSTASCII_STRINGPARAM("drawing.PolyPolygon")) || aType.equalsL(RTL_CONSTASCII_STRINGPARAM("drawing.PolyLine")))
    {
        if ( aPropertySet.is() )
        {
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny,
                    aPropertySet, String( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygon" ) ) ) )
            {
                ::com::sun::star::drawing::PointSequenceSequence* pSourcePolyPolygon =
                    (::com::sun::star::drawing::PointSequenceSequence*)aAny.getValue();
                sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->getLength();
                ::com::sun::star::drawing::PointSequence* pOuterSequence = pSourcePolyPolygon->getArray();

                if ( pOuterSequence )
                {
                    sal_Int32 a, b, nIndex = 0;
                    sal_uInt32 nDistance = 0xffffffff;
                    for( a = 0; a < nOuterSequenceCount; a++ )
                    {
                        ::com::sun::star::drawing::PointSequence* pInnerSequence = pOuterSequence++;
                        if ( pInnerSequence )
                        {
                            ::com::sun::star::awt::Point* pArray = pInnerSequence->getArray();
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
    else if ( (aType.equalsL(RTL_CONSTASCII_STRINGPARAM("drawing.OpenBezier"))) || (aType.equalsL(RTL_CONSTASCII_STRINGPARAM("drawing.OpenFreeHand"))) || (aType.equalsL(RTL_CONSTASCII_STRINGPARAM("drawing.PolyLinePath")))
        || (aType.equalsL(RTL_CONSTASCII_STRINGPARAM("drawing.ClosedBezier"))) || ( aType.equalsL(RTL_CONSTASCII_STRINGPARAM("drawing.ClosedFreeHand"))) || (aType.equalsL(RTL_CONSTASCII_STRINGPARAM("drawing.PolyPolygonPath"))) )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
            aPropertySet2( aXShape, ::com::sun::star::uno::UNO_QUERY );
        if ( aPropertySet2.is() )
        {
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny,
                    aPropertySet2, String( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygonBezier" ) ) ) )
            {
                ::com::sun::star::drawing::PolyPolygonBezierCoords* pSourcePolyPolygon =
                    (::com::sun::star::drawing::PolyPolygonBezierCoords*)aAny.getValue();
                sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->Coordinates.getLength();

                // Zeiger auf innere sequences holen
                ::com::sun::star::drawing::PointSequence* pOuterSequence =
                    pSourcePolyPolygon->Coordinates.getArray();
                ::com::sun::star::drawing::FlagSequence*  pOuterFlags =
                    pSourcePolyPolygon->Flags.getArray();

                if ( pOuterSequence && pOuterFlags )
                {
                    sal_Int32 a, b, nIndex = 0;
                    sal_uInt32 nDistance = 0xffffffff;

                    for ( a = 0; a < nOuterSequenceCount; a++ )
                    {
                        ::com::sun::star::drawing::PointSequence* pInnerSequence = pOuterSequence++;
                        ::com::sun::star::drawing::FlagSequence*  pInnerFlags = pOuterFlags++;
                        if ( pInnerSequence && pInnerFlags )
                        {
                            ::com::sun::star::awt::Point* pArray = pInnerSequence->getArray();
                            ::com::sun::star::drawing::PolygonFlags* pFlags = pInnerFlags->getArray();
                            if ( pArray && pFlags )
                            {
                                for ( b = 0; b < pInnerSequence->getLength(); b++, pArray++ )
                                {
                                    PolyFlags ePolyFlags = *( (PolyFlags*)pFlags++ );
                                    if ( ePolyFlags == POLY_CONTROL )
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

        if (aType.equalsL(RTL_CONSTASCII_STRINGPARAM("drawing.Custom")))
        {
            SdrObject* pCustoShape( GetSdrObjectFromXShape( aXShape ) );
            if ( pCustoShape && pCustoShape->ISA( SdrObjCustomShape ) )
            {
                SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)(const SdrCustomShapeGeometryItem&)
                    pCustoShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );

                const rtl::OUString sPath( "Path"  );
                const rtl::OUString sType( "Type"  );
                const rtl::OUString sGluePointType( "GluePointType"  );

                rtl::OUString sShapeType;
                uno::Any* pType = rGeometryItem.GetPropertyValueByName( sType );
                if ( pType )
                    *pType >>= sShapeType;
                MSO_SPT eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

                uno::Any* pGluePointType = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( sPath, sGluePointType );

                sal_Int16 nGluePointType = sal_Int16();
                if ( !( pGluePointType &&
                        ( *pGluePointType >>= nGluePointType ) ) )
                    nGluePointType = GetCustomShapeConnectionTypeDefault( eSpType );

                if ( nGluePointType == com::sun::star::drawing::EnhancedCustomShapeGluePointType::CUSTOM )
                {
                    const SdrGluePointList* pList = pCustoShape->GetGluePointList();
                    if ( pList )
                    {
                        Polygon aPoly;
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
                else if ( nGluePointType == com::sun::star::drawing::EnhancedCustomShapeGluePointType::SEGMENTS )
                {
                    SdrObject* pPoly = pCustoShape->DoConvertToPolyObj( sal_True );
                    if ( pPoly && pPoly->ISA( SdrPathObj ) )
                    {
                        sal_Int16 a, b, nIndex = 0;
                        sal_uInt32 nDistance = 0xffffffff;

                        // #i74631# use explicit constructor here. Also XPolyPolygon is not necessary,
                        // reducing to PolyPolygon
                        const PolyPolygon aPolyPoly(((SdrPathObj*)pPoly)->GetPathPoly());

                        for ( a = 0; a < aPolyPoly.Count(); a++ )
                        {
                            const Polygon& rPoly = aPolyPoly.GetObject( a );
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
            ::com::sun::star::awt::Point aPoint( aXShape->getPosition() );
            ::com::sun::star::awt::Size  aSize( aXShape->getSize() );

            Rectangle   aRect( Point( aPoint.X, aPoint.Y ), Size( aSize.Width, aSize.Height ) );
            Point       aCenter( aRect.Center() );
            Polygon     aPoly( 4 );

            aPoly[ 0 ] = Point( aCenter.X(), aRect.Top() );
            aPoly[ 1 ] = Point( aRect.Left(), aCenter.Y() );
            aPoly[ 2 ] = Point( aCenter.X(), aRect.Bottom() );
            aPoly[ 3 ] = Point( aRect.Right(), aCenter.Y() );

            sal_Int32 nAngle = ( EscherPropertyValueHelper::GetPropertyValue( aAny,
                aPropertySet, String( RTL_CONSTASCII_USTRINGPARAM( "RotateAngle" ) ), sal_True ) )
                    ? *((sal_Int32*)aAny.getValue() )
                    : 0;
            if ( nAngle )
                aPoly.Rotate( aRect.TopLeft(), (sal_uInt16)( ( nAngle + 5 ) / 10 ) );
            nRule = GetClosestPoint( aPoly, aRefPoint );

            if (aType.equalsL(RTL_CONSTASCII_STRINGPARAM("drawing.Ellipse")))
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

void EscherSolverContainer::AddShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape, sal_uInt32 nId )
{
    maShapeList.push_back( new EscherShapeListEntry( rXShape, nId ) );
}

void EscherSolverContainer::AddConnector(
    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rConnector,
    const ::com::sun::star::awt::Point& rPA,
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rConA,
    const ::com::sun::star::awt::Point& rPB,
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rConB
)
{
    maConnectorList.push_back( new EscherConnectorListEntry( rConnector, rPA, rConA, rPB, rConB ) );
}

sal_uInt32 EscherSolverContainer::GetShapeId( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape ) const
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
        rStrm   << (sal_uInt16)( ( nCount << 4 ) | 0xf )    // open an ESCHER_SolverContainer
                << (sal_uInt16)ESCHER_SolverContainer       //
                << (sal_uInt32)0;                           //

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
                    aConnectorRule.ncptiA = pPtr->GetConnectorRule( sal_True );
                if ( aConnectorRule.nShapeB )
                    aConnectorRule.ncptiB = pPtr->GetConnectorRule( sal_False );
            }
            rStrm   << (sal_uInt32)( ( ESCHER_ConnectorRule << 16 ) | 1 )   // atom hd
                    << (sal_uInt32)24                                       //
                    << aConnectorRule.nRuleId
                    << aConnectorRule.nShapeA
                    << aConnectorRule.nShapeB
                    << aConnectorRule.nShapeC
                    << aConnectorRule.ncptiA
                    << aConnectorRule.ncptiB;

            aConnectorRule.nRuleId += 2;
        }

        nCurrentPos = rStrm.Tell();             // close the ESCHER_SolverContainer
        nSize = ( nCurrentPos - nRecHdPos ) - 4;//
        rStrm.Seek( nRecHdPos );                //
        rStrm << nSize;                         //
        rStrm.Seek( nCurrentPos );              //
    }
}

EscherExGlobal::EscherExGlobal( sal_uInt32 nGraphicProvFlags ) :
    EscherGraphicProvider( nGraphicProvFlags ),
    mpPicStrm( 0 ),
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
    rStrm << static_cast< sal_uInt32 >( ESCHER_Dgg << 16 ) << static_cast< sal_uInt32 >( nDggSize - 8 );

    // claculate and write the fixed DGG data
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
    rStrm << nLastShapeId << nClusterCount << nShapeCount << nDrawingCount;

    // write the cluster table
    for( ClusterTable::const_iterator aIt = maClusterTable.begin(), aEnd = maClusterTable.end(); aIt != aEnd; ++aIt )
        rStrm << aIt->mnDrawingId << aIt->mnNextShapeId;
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
    return 0;
}

/// Implementation of an empty stream that silently succeeds, but does nothing.
///
/// In fact, this is a hack.  The right solution is to abstract EscherEx to be
/// able to work without SvStream; but at the moment it is better to live with
/// this I guess.
class SvNullStream : public SvStream
{
protected:
    virtual sal_Size GetData( void* pData, sal_Size nSize ) { memset( pData, 0, nSize ); return nSize; }
    virtual sal_Size PutData( const void*, sal_Size nSize ) { return nSize; }
    virtual sal_Size SeekPos( sal_Size nPos ) { return nPos; }
    virtual void SetSize( sal_Size ) {}
    virtual void FlushData() {}

public:
    SvNullStream() : SvStream() {}
    virtual ~SvNullStream() {}
};

EscherEx::EscherEx( const EscherExGlobalRef& rxGlobal, SvStream* pOutStrm ) :
    mxGlobal                ( rxGlobal ),
    mpOutStrm               ( pOutStrm ),
    mbOwnsStrm              ( false ),

    mnCurrentDg                         ( 0 ),

    mnGroupLevel            ( 0 ),
    mnHellLayerId           ( USHRT_MAX ),

    mbEscherSpgr            ( sal_False ),
    mbEscherDg              ( sal_False )
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
    sal_uInt8*  pBuf;

    // Persist table anpassen
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
        *mpOutStrm >> nType >> nSize;
        sal_uInt32 nEndOfRecord = mpOutStrm->Tell() + nSize;
        bool bContainer = (nType & 0x0F) == 0x0F;
        /*  Expand the record, if the insertion position is inside, or if the
            position is at the end of a container (expands always), or at the
            end of an atom and bExpandEndOfAtom is set. */
        if ( (nCurPos < nEndOfRecord) || ((nCurPos == nEndOfRecord) && (bContainer || bExpandEndOfAtom)) )
        {
            mpOutStrm->SeekRel( -4 );
            *mpOutStrm << (sal_uInt32)( nSize + nBytes );
            if ( !bContainer )
                mpOutStrm->SeekRel( nSize );
        }
        else
            mpOutStrm->SeekRel( nSize );
    }
    std::vector< sal_uInt32 >::iterator aIter( mOffsets.begin() );
    std::vector< sal_uInt32 >::iterator aEnd( mOffsets.end() );
    while( aIter != aEnd )
    {
        if ( *aIter > nCurPos )
            *aIter += nBytes;
        aIter++;
    }
    mpOutStrm->Seek( STREAM_SEEK_TO_END );
    nSource = mpOutStrm->Tell();
    nToCopy = nSource - nCurPos;                        // increase the size of the tream by nBytes
    pBuf = new sal_uInt8[ 0x40000 ];                            // 256KB Buffer
    while ( nToCopy )
    {
        nBufSize = ( nToCopy >= 0x40000 ) ? 0x40000 : nToCopy;
        nToCopy -= nBufSize;
        nSource -= nBufSize;
        mpOutStrm->Seek( nSource );
        mpOutStrm->Read( pBuf, nBufSize );
        mpOutStrm->Seek( nSource + nBytes );
        mpOutStrm->Write( pBuf, nBufSize );
    }
    delete[] pBuf;
    mpOutStrm->Seek( nCurPos );
}

sal_Bool EscherEx::SeekBehindRecHeader( sal_uInt16 nRecType )
{
    sal_uInt32  nOldPos, nStreamEnd, nType, nSize;

    nOldPos = mpOutStrm->Tell();
    nStreamEnd = mpOutStrm->Seek( STREAM_SEEK_TO_END );
    mpOutStrm->Seek( nOldPos );
    while ( mpOutStrm->Tell() < nStreamEnd )
    {
        *mpOutStrm >> nType >> nSize;
        if ( ( nType >> 16 ) == nRecType )
            return sal_True;
        if ( ( nType & 0xf ) != 0xf )
            mpOutStrm->SeekRel( nSize );
    }
    mpOutStrm->Seek( nOldPos );
    return sal_False;
}

void EscherEx::InsertPersistOffset( sal_uInt32 nKey, sal_uInt32 nOffset )
{
    PtInsert( ESCHER_Persist_PrivateEntry | nKey, nOffset );
}

void EscherEx::ReplacePersistOffset( sal_uInt32 nKey, sal_uInt32 nOffset )
{
    PtReplace( ESCHER_Persist_PrivateEntry | nKey, nOffset );
}

sal_uInt32 EscherEx::GetPersistOffset( sal_uInt32 nKey )
{
    return PtGetOffsetByID( ESCHER_Persist_PrivateEntry | nKey );
}

sal_Bool EscherEx::DoSeek( sal_uInt32 nKey )
{
    sal_uInt32 nPos = PtGetOffsetByID( nKey );
    if ( nPos )
        mpOutStrm->Seek( nPos );
    else
    {
        if (! PtIsID( nKey ) )
            return sal_False;
        mpOutStrm->Seek( 0 );
    }
    return sal_True;
}

sal_Bool EscherEx::SeekToPersistOffset( sal_uInt32 nKey )
{
    return DoSeek( ESCHER_Persist_PrivateEntry | nKey );
}

sal_Bool EscherEx::InsertAtPersistOffset( sal_uInt32 nKey, sal_uInt32 nValue )
{
    sal_uInt32  nOldPos = mpOutStrm->Tell();
    sal_Bool    bRetValue = SeekToPersistOffset( nKey );
    if ( bRetValue )
    {
        *mpOutStrm << nValue;
        mpOutStrm->Seek( nOldPos );
    }
    return bRetValue;
}

void EscherEx::OpenContainer( sal_uInt16 nEscherContainer, int nRecInstance )
{
    *mpOutStrm << (sal_uInt16)( ( nRecInstance << 4 ) | 0xf  ) << nEscherContainer << (sal_uInt32)0;
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
                    mbEscherDg = sal_True;
                    mnCurrentDg = mxGlobal->GenerateDrawingId();
                    AddAtom( 8, ESCHER_Dg, 0, mnCurrentDg );
                    PtReplaceOrInsert( ESCHER_Persist_Dg | mnCurrentDg, mpOutStrm->Tell() );
                    *mpOutStrm << (sal_uInt32)0     // The number of shapes in this drawing
                               << (sal_uInt32)0;    // The last MSOSPID given to an SP in this DG
                }
            }
        }
        break;

        case ESCHER_SpgrContainer :
        {
            if ( mbEscherDg )
            {
                mbEscherSpgr = sal_True;
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
    *mpOutStrm << nSize;

    switch( mRecTypes.back() )
    {
        case ESCHER_DgContainer :
        {
            if ( mbEscherDg )
            {
                mbEscherDg = sal_False;
                if ( DoSeek( ESCHER_Persist_Dg | mnCurrentDg ) )
                    *mpOutStrm << mxGlobal->GetDrawingShapeCount( mnCurrentDg ) << mxGlobal->GetLastShapeId( mnCurrentDg );
            }
        }
        break;

        case ESCHER_SpgrContainer :
        {
            if ( mbEscherSpgr )
            {
                mbEscherSpgr = sal_False;

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
    *mpOutStrm << (sal_uInt32)0 << (sal_uInt32)0;       // record header wird spaeter geschrieben
}

void EscherEx::EndAtom( sal_uInt16 nRecType, int nRecVersion, int nRecInstance )
{
    sal_uInt32  nOldPos = mpOutStrm->Tell();
    mpOutStrm->Seek( mnCountOfs );
    sal_uInt32 nSize = nOldPos - mnCountOfs;
    *mpOutStrm << (sal_uInt16)( ( nRecInstance << 4 ) | ( nRecVersion & 0xf ) ) << nRecType << (sal_uInt32)( nSize - 8 );
    mpOutStrm->Seek( nOldPos );
}

void EscherEx::AddAtom( sal_uInt32 nAtomSize, sal_uInt16 nRecType, int nRecVersion, int nRecInstance )
{
    *mpOutStrm << (sal_uInt16)( ( nRecInstance << 4 ) | ( nRecVersion & 0xf ) ) << nRecType << nAtomSize;
}

void EscherEx::AddChildAnchor( const Rectangle& rRect )
{
    AddAtom( 16, ESCHER_ChildAnchor );
    *mpOutStrm  << (sal_Int32)rRect.Left()
                << (sal_Int32)rRect.Top()
                << (sal_Int32)rRect.Right()
                << (sal_Int32)rRect.Bottom();
}

void EscherEx::AddClientAnchor( const Rectangle& rRect )
{
    AddAtom( 8, ESCHER_ClientAnchor );
    *mpOutStrm << (sal_Int16)rRect.Top()
               << (sal_Int16)rRect.Left()
               << (sal_Int16)( rRect.GetWidth()  + rRect.Left() )
               << (sal_Int16)( rRect.GetHeight() + rRect.Top() );
}

EscherExHostAppData* EscherEx::EnterAdditionalTextGroup()
{
    return NULL;
}

sal_uInt32 EscherEx::EnterGroup( const String& rShapeName, const Rectangle* pBoundRect )
{
    Rectangle aRect;
    if( pBoundRect )
        aRect = *pBoundRect;

    OpenContainer( ESCHER_SpgrContainer );
    OpenContainer( ESCHER_SpContainer );
    AddAtom( 16, ESCHER_Spgr, 1 );
    PtReplaceOrInsert( ESCHER_Persist_Grouping_Snap | mnGroupLevel,
                        mpOutStrm->Tell() );
    *mpOutStrm  << (sal_Int32)aRect.Left()  // Bounding box for the grouped shapes the wich they will be attached
                << (sal_Int32)aRect.Top()
                << (sal_Int32)aRect.Right()
                << (sal_Int32)aRect.Bottom();

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
        if( rShapeName.Len() > 0 )
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
    return EnterGroup( String::EmptyString(), pBoundRect );
}

sal_Bool EscherEx::SetGroupSnapRect( sal_uInt32 nGroupLevel, const Rectangle& rRect )
{
    sal_Bool bRetValue = sal_False;
    if ( nGroupLevel )
    {
        sal_uInt32 nCurrentPos = mpOutStrm->Tell();
        if ( DoSeek( ESCHER_Persist_Grouping_Snap | ( nGroupLevel - 1 ) ) )
        {
            *mpOutStrm  << (sal_Int32)rRect.Left()  // Bounding box for the grouped shapes the wich they will be attached
                        << (sal_Int32)rRect.Top()
                        << (sal_Int32)rRect.Right()
                        << (sal_Int32)rRect.Bottom();
            mpOutStrm->Seek( nCurrentPos );
        }
    }
    return bRetValue;
}

sal_Bool EscherEx::SetGroupLogicRect( sal_uInt32 nGroupLevel, const Rectangle& rRect )
{
    sal_Bool bRetValue = sal_False;
    if ( nGroupLevel )
    {
        sal_uInt32 nCurrentPos = mpOutStrm->Tell();
        if ( DoSeek( ESCHER_Persist_Grouping_Logic | ( nGroupLevel - 1 ) ) )
        {
            *mpOutStrm << (sal_Int16)rRect.Top() << (sal_Int16)rRect.Left() << (sal_Int16)rRect.Right() << (sal_Int16)rRect.Bottom();
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
    *mpOutStrm << nShapeID << nFlags;
}

void EscherEx::Commit( EscherPropertyContainer& rProps, const Rectangle& )
{
    rProps.Commit( GetStream() );
}

sal_uInt32 EscherEx::GetColor( const sal_uInt32 nSOColor, sal_Bool bSwap )
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

sal_uInt32 EscherEx::GetColor( const Color& rSOColor, sal_Bool bSwap )
{
    sal_uInt32 nColor = ( rSOColor.GetRed() << 16 );
    nColor |= ( rSOColor.GetGreen() << 8 );
    nColor |= rSOColor.GetBlue();

    if ( !bSwap )
        nColor = GetColor( nColor, sal_True );

    return nColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
