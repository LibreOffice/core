/*************************************************************************
 *
 *  $RCSfile: escherex.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-16 16:14:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <math.h>

#ifndef _ESCHESDO_HXX
#include "eschesdo.hxx"
#endif
#ifndef _SVX_ESCHEREX_HXX
#include "escherex.hxx"
#endif
#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SV_CVTGRF_HXX
#include <vcl/cvtgrf.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _ZCODEC_HXX
#include <tools/zcodec.hxx>
#endif
#ifndef _INC_STDLIB
#include <stdlib.h>
#endif
#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENTSTYLE_HPP_
#include <com/sun/star/awt/GradientStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RASTEROPERATION_HPP_
#include <com/sun/star/awt/RasterOperation.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_BEZIERPOINT_HPP_
#include <com/sun/star/drawing/BezierPoint.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CONNECTORTYPE_HPP_
#include <com/sun/star/drawing/ConnectorType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CONNECTIONTYPE_HPP_
#include <com/sun/star/drawing/ConnectionType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FLAGSEQUENCE_HPP_
#include <com/sun/star/drawing/FlagSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYGONFLAGS_HPP_
#include <com/sun/star/drawing/PolygonFlags.hpp>
#endif
#ifndef _SV_HATCH_HXX_
#include <vcl/hatch.hxx>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HATCH_HPP_
#include <com/sun/star/drawing/Hatch.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XGRAPHICS_HPP_
#include <com/sun/star/awt/XGraphics.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_COLORMODE_HPP_
#include <com/sun/star/drawing/ColorMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_BITMAPMODE_HPP_
#include <com/sun/star/drawing/BitmapMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/unohlp.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif
#include <vos/xception.hxx>
#ifndef _VOS_NO_NAMESPACE
using namespace vos;
#endif

using namespace ::rtl;

// ---------------------------------------------------------------------------------------------

EscherExClientRecord_Base::~EscherExClientRecord_Base()
{
}

EscherExClientAnchor_Base::~EscherExClientAnchor_Base()
{
}

// ---------------------------------------------------------------------------------------------

struct EscherPropSortStruct
{
    sal_uInt8*  pBuf;
    sal_uInt32  nPropSize;
    sal_uInt32  nPropValue;
    sal_uInt16  nPropId;
};

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
            delete( pSortStruct[ nSortCount ].pBuf );
    }
    delete  pSortStruct;
};

void EscherPropertyContainer::AddOpt( sal_uInt16 nPropID, sal_uInt32 nPropValue, sal_Bool bBlib )
{
    AddOpt( nPropID, bBlib, nPropValue, NULL, 0 );
}

void EscherPropertyContainer::AddOpt( sal_uInt16 nPropID, sal_Bool bBlib, sal_uInt32 nPropValue, sal_uInt8* pProp, sal_uInt32 nPropSize )
{
    if ( bBlib )                // bBlib is only valid when fComplex = 0
        nPropID |= 0x4000;
    if ( pProp )
        nPropID |= 0x8000;      // fComplex = TRUE;

    sal_uInt32 i;
    for( i = 0; i < nSortCount; i++ )
    {
        if ( ( pSortStruct[ i ].nPropId &~0xc000 ) == ( nPropID &~0xc000 ) )    // pruefen, ob Property nur ersetzt wird
        {
            pSortStruct[ i ].nPropId = nPropID;
            if ( pSortStruct[ i ].pBuf )
            {
                nCountSize -= pSortStruct[ i ].nPropSize;
                delete pSortStruct[ i ].pBuf;
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
    if ( nSortCount == nSortBufSize )                                           // buffer vergroessern
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
    pSortStruct[ nSortCount ].nPropId = nPropID;                                // property einfuegen
    pSortStruct[ nSortCount ].pBuf = pProp;
    pSortStruct[ nSortCount ].nPropSize = nPropSize;
    pSortStruct[ nSortCount++ ].nPropValue = nPropValue;

    if ( pProp )
    {
        nCountSize += nPropSize;
        bHasComplexData = sal_True;
    }
}

sal_Bool EscherPropertyContainer::GetOpt( sal_uInt16 nPropId, sal_uInt32& nPropValue ) const
{
    sal_Bool bRetValue = sal_False;
    for( sal_uInt32 i = 0; i < nSortCount; i++ )
    {
        if ( ( pSortStruct[ i ].nPropId &~0xc000 ) == ( nPropId &~0xc000 ) )
        {
            nPropValue = pSortStruct[ i ].nPropValue;
            bRetValue = sal_True;
            break;
        }
    }
    return bRetValue;
}


extern "C" int __LOADONCALLAPI EscherPropSortFunc( const void* p1, const void* p2 )
{
    INT16   nID1 = ((EscherPropSortStruct*)p1)->nPropId &~0xc000;
    INT16   nID2 = ((EscherPropSortStruct*)p2)->nPropId &~0xc000;

    if( nID1  < nID2 )
        return -1;
    else if( nID1 > nID2 )
        return 1;
    else
        return 0;
}

void EscherPropertyContainer::Commit( SvStream& rSt, sal_uInt16 nVersion )
{
    rSt << (sal_uInt16)( ( nCountCount << 4 ) | ( nVersion & 0xf ) ) << (sal_uInt16)ESCHER_OPT << nCountSize;
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

sal_uInt32 EscherPropertyContainer::ImplGetColor( const sal_uInt32 nSOColor, sal_Bool bSwap )
{
    if ( bSwap )
    {
        sal_uInt32 nColor = nSOColor & 0xff00;      // GRUEN
        nColor |= (sal_uInt8)( nSOColor ) << 16;    // ROT
        nColor |= (sal_uInt8)( nSOColor >> 16 );    // BLAU
        return nColor;
    }
    else
        return nSOColor & 0xffffff;
}

sal_uInt32 EscherPropertyContainer::GetGradientColor(
    const ::com::sun::star::awt::Gradient* pGradient,
        sal_uInt32 nStartColor )
{
    sal_uInt32  nIntensity;
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
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet )
{
    ::com::sun::star::uno::Any          aAny;
    ::com::sun::star::awt::Gradient*    pGradient = NULL;

    sal_Int32   nAngle = 0;
    sal_uInt32  nFillFocus = 0x64;
    sal_uInt32  nFirstColor = 0;

    if ( EscherPropertyValueHelper::GetPropertyValue(
            aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillGradient" ) ), sal_False ) )
    {
        pGradient = (::com::sun::star::awt::Gradient*)aAny.getValue();
        nAngle = pGradient->Angle;
    }
    switch ( pGradient->Style )
    {
        default:
        case ::com::sun::star::awt::GradientStyle_LINEAR :
        {
        }
        break;
        case ::com::sun::star::awt::GradientStyle_AXIAL :
        {
            nFillFocus = 0x32;
            nFirstColor = 1;
        }
        break;
        case ::com::sun::star::awt::GradientStyle_RADIAL :
        {
        }
        break;
        case ::com::sun::star::awt::GradientStyle_ELLIPTICAL :
        {
        }
        break;
        case ::com::sun::star::awt::GradientStyle_SQUARE :
        {
        }
        break;
        case ::com::sun::star::awt::GradientStyle_RECT :
        {
        }
        break;
    }
    AddOpt( ESCHER_Prop_fillType, ESCHER_FillShadeScale );
    AddOpt( ESCHER_Prop_fillAngle, ( ( -3600 + nAngle ) << 16 ) / 10 );
    AddOpt( ESCHER_Prop_fillColor, GetGradientColor( pGradient, nFirstColor ) );
    AddOpt( ESCHER_Prop_fillBackColor, GetGradientColor( pGradient, nFirstColor ^ 1 ) );
    AddOpt( ESCHER_Prop_fillFocus, nFillFocus );
};

void EscherPropertyContainer::CreateFillProperties(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
        sal_Bool bEdge )
{
    ::com::sun::star::uno::Any aAny;
    AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapNone );
    AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );

    sal_uInt32 nFillBackColor = 0;

    if ( EscherPropertyValueHelper::GetPropertyValue(
            aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ) ), sal_False ) )
    {
        ::com::sun::star::drawing::FillStyle eFS;
        if ( ! ( aAny >>= eFS ) )
            eFS = ::com::sun::star::drawing::FillStyle_SOLID;
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
                sal_uInt16 nTransparency = ( EscherPropertyValueHelper::GetPropertyValue(
                                                aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillTransparence" ) ), sal_False ) )
                                            ? *((sal_Int16*)aAny.getValue() )
                                            : 0;
                if ( nTransparency != 100 )
                {
                    if ( nTransparency )    // opacity
                        AddOpt( ESCHER_Prop_fillOpacity, ( ( 100 - nTransparency ) << 16 ) / 100 );
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
            }
            case ::com::sun::star::drawing::FillStyle_NONE :
                AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
            break;
        }
    }
    CreateLineProperties( rXPropSet, bEdge );
}

void EscherPropertyContainer::CreateLineProperties(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
        sal_Bool bEdge )
{
    ::com::sun::star::uno::Any aAny;
    sal_uInt32 nLineFlags = 0x80008;
    ESCHER_LineEnd eLineEnd = ESCHER_LineArrowEnd;

    if ( EscherPropertyValueHelper::GetPropertyValue(
            aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "LineStart" ) ), sal_False ) )
    {
        PolyPolygon aPolyPoly( ImplGetPolyPolygon( aAny ) );
        if ( aPolyPoly.Count() )
        {
            const Polygon& rPoly = aPolyPoly[ 0 ];
            sal_uInt16 nPolyCount = rPoly.GetSize();

            if ( nPolyCount )
            {
                switch ( nPolyCount )
                {
                    case 0x4 :
                    {
                        switch( rPoly[ 0 ].X() )
                        {
                            case 0 : eLineEnd = ESCHER_LineArrowDiamondEnd;    break;
                            case 0x529 : eLineEnd = ESCHER_LineArrowStealthEnd; break;
                        }
                    }
                    break;
                    case 0x7 : eLineEnd = ESCHER_LineArrowDiamondEnd; break;
                    case 0xa : eLineEnd = ESCHER_LineArrowStealthEnd; break;
                    case 0xd :
                    {
                        switch ( rPoly[ 0 ].X() )
                        {
                            case 0 : eLineEnd = ESCHER_LineArrowDiamondEnd; break;
                            case 0x64 : eLineEnd = ESCHER_LineArrowOvalEnd; break;
                            case 0x87c : eLineEnd = ESCHER_LineArrowStealthEnd; break;
                        }
                    }
                }
                AddOpt( ESCHER_Prop_lineStartArrowLength, 2 );
                AddOpt( ESCHER_Prop_lineStartArrowWidth, 2 );
                AddOpt( ESCHER_Prop_lineStartArrowhead, eLineEnd );
                nLineFlags |= 0x100010;
            }
        }
    }

    eLineEnd = ESCHER_LineArrowEnd;

    if ( EscherPropertyValueHelper::GetPropertyValue(
            aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "LineEnd"  ) ), sal_False ) )
    {
        PolyPolygon aPolyPoly( ImplGetPolyPolygon( aAny ) );
        if ( aPolyPoly.Count() )
        {
            const Polygon& rPoly = aPolyPoly[ 0 ];
            sal_uInt16 nPolyCount = rPoly.GetSize();

            if ( nPolyCount )
            {
                switch ( nPolyCount )
                {
                    case 0x4 :
                    {
                        switch( rPoly[ 0 ].X() )
                        {
                            case 0 : eLineEnd = ESCHER_LineArrowDiamondEnd;    break;
                            case 0x529 : eLineEnd = ESCHER_LineArrowStealthEnd; break;
                        }
                    }
                    break;
                    case 0x7 : eLineEnd = ESCHER_LineArrowDiamondEnd; break;
                    case 0xa : eLineEnd = ESCHER_LineArrowStealthEnd; break;
                    case 0xd :
                    {
                        switch ( rPoly[ 0 ].X() )
                        {
                            case 0 : eLineEnd = ESCHER_LineArrowDiamondEnd; break;
                            case 0x64 : eLineEnd = ESCHER_LineArrowOvalEnd; break;
                            case 0x87c : eLineEnd = ESCHER_LineArrowStealthEnd; break;
                        }
                    }
                }
                AddOpt( ESCHER_Prop_lineEndArrowLength, 2 );
                AddOpt( ESCHER_Prop_lineEndArrowWidth, 2 );
                AddOpt( ESCHER_Prop_lineEndArrowhead, eLineEnd );
                nLineFlags |= 0x100010;
            }
        }
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
                                AddOpt( ESCHER_Prop_lineEndCapStyle, 0 ); // Style Round setzen
                            break;
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
    if ( bEdge == sal_False )
    {
        AddOpt( ESCHER_Prop_fFillOK, 0x1001 );
        AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
    }
}

void EscherPropertyContainer::ImplCreateGraphicAttributes( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                                                            sal_uInt32 nBlibId, sal_Bool bCreateCroppingAttributes )
{
    ::com::sun::star::uno::Any aAny;

    sal_uInt32 nPicFlags = 0;
    ::com::sun::star::drawing::ColorMode eColorMode( ::com::sun::star::drawing::ColorMode_STANDARD );
    sal_Int16 nLuminance = 0;
    sal_Int16 nContrast = 0;
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
        aAny >>= nContrast;
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

    if ( ( eColorMode == ::com::sun::star::drawing::ColorMode_WATERMARK )
            && ( nLuminance || nContrast ) )
    {
        eColorMode = ::com::sun::star::drawing::ColorMode_STANDARD;
        nLuminance += 5000;
        if ( nLuminance > 10000 )
            nLuminance = 10000;
        nContrast -= 7000;
        if ( nContrast < 10000 )
            nContrast = 10000;
    }
    nContrast *= 327;
    nLuminance *= 327;
    switch ( eColorMode )
    {
        case ::com::sun::star::drawing::ColorMode_GREYS :
            nPicFlags |= 0x40004;
        break;
        case ::com::sun::star::drawing::ColorMode_MONO :
            nPicFlags |= 0x60006;
        break;
        case ::com::sun::star::drawing::ColorMode_WATERMARK :
        {
            nContrast = 0x4ccd;
            nLuminance = 0x599a;
        }
        break;
    }

    if ( bCreateCroppingAttributes && pGraphicProvider )
    {
        Size    aPrefSize;
        MapMode aPrefMapMode;
        if ( pGraphicProvider->GetPrefSize( nBlibId, aPrefSize, aPrefMapMode ) )
        {
            Size aCropSize( Application::GetDefaultDevice()->LogicToLogic( aPrefSize, aPrefMapMode, MAP_100TH_MM ) );
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
    if ( nContrast )
        AddOpt( ESCHER_Prop_pictureContrast, nContrast );
    if ( nLuminance )
        AddOpt( ESCHER_Prop_pictureBrightness, nLuminance );
    if ( nPicFlags )
        AddOpt( ESCHER_Prop_pictureActive, nPicFlags );
}



sal_Bool EscherPropertyContainer::CreateGraphicProperties(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
        const String& rSource, sal_Bool bFillBitmap, sal_Bool bCreateCroppingAttributes )
{
    sal_Bool        bRetValue = sal_False;

    if ( pGraphicProvider && pPicOutStrm && pShapeBoundRect )
    {
        sal_Bool        bMirrored = sal_False;
        GraphicAttr*    pGraphicAttr = NULL;
        GraphicObject   aGraphicObject;
        String          aGraphicUrl;
        ByteString      aUniqueId;

        ::com::sun::star::drawing::BitmapMode   eBitmapMode( ::com::sun::star::drawing::BitmapMode_NO_REPEAT );
        ::com::sun::star::uno::Any aAny;

        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, rSource ) )
        {
            Point aEmptyPoint;
            Rectangle aRect( aEmptyPoint, pShapeBoundRect->GetSize() );

            if ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ) )
            {
                ::com::sun::star::uno::Sequence<sal_uInt8> aSeq = *(::com::sun::star::uno::Sequence<sal_uInt8>*)aAny.getValue();
                const sal_uInt8*    pAry = aSeq.getArray();
                sal_uInt32          nAryLen = aSeq.getLength();

                if ( pAry && nAryLen )
                {
                    Graphic         aGraphic;
                    SvMemoryStream  aTemp( (void*)pAry, nAryLen, STREAM_READ );
                    sal_uInt32 nErrCode = GraphicConverter::Import( aTemp, aGraphic, CVT_WMF );
                    if ( nErrCode == ERRCODE_NONE )
                    {
                        aGraphicObject = aGraphic;
                        aUniqueId = aGraphicObject.GetUniqueID();
                    }
                }
            }
            else if ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "Bitmap" ) ) )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >xBitmap;
                if ( ::cppu::extractInterface( xBitmap, aAny ) )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap > xBitmap;
                    if ( aAny >>= xBitmap )
                    {
                        BitmapEx    aBitmapEx( VCLUnoHelper::GetBitmap( xBitmap ) );
                        Graphic     aGraphic( aBitmapEx );
                        aGraphicObject = aGraphic;
                        aUniqueId = aGraphicObject.GetUniqueID();
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
            }
            else if ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "FillHatch" ) ) )
            {
                ::com::sun::star::drawing::Hatch aHatch;
                if ( aAny >>= aHatch )
                {
                    sal_Bool        bBackground = sal_False;
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                            String( RTL_CONSTASCII_USTRINGPARAM( "FillBackground" ) ), sal_True ) )
                    {
                        aAny >>= bBackground;
                    }

                    const MapMode   aMap100( MAP_100TH_MM );
                    VirtualDevice   aVDev;
                    const Size      aOutSize( aVDev.PixelToLogic( Size( 28, 28 ), aMap100 ) );

                    if( aVDev.SetOutputSize( aOutSize ) )
                    {
                        Rectangle aRectangle = Rectangle( Point(), aOutSize );
                        const PolyPolygon   aPolyPoly( aRectangle );
                        Hatch               aVclHatch( (HatchStyle) aHatch.Style, Color( aHatch.Color ), aHatch.Distance, (sal_uInt16)aHatch.Angle );

                        if ( bBackground )
                        {
                            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                                String( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ), sal_False ) )
                            {
                                aVDev.SetLineColor();
                                aVDev.SetFillColor( ImplGetColor( *((sal_uInt32*)aAny.getValue()), sal_False ) );
                                aVDev.DrawRect( Rectangle( Point(), aOutSize ) );
                            }
                        }
                        aVDev.SetMapMode( aMap100 );
                        aVDev.DrawHatch( aPolyPoly, aVclHatch );
                        Bitmap  aBitmap( aVDev.GetBitmap( Point(), aOutSize ) );

                        if ( bBackground )
                            aGraphicObject = Graphic( aBitmap );
                        else
                        {
                            VirtualDevice   aMaskVDev( 1 );
                            aMaskVDev.SetMapMode( aMap100 );
                            if( aMaskVDev.SetOutputSize( aOutSize ) )
                            {
                                aVclHatch.SetColor( Color( COL_BLACK ) );
                                aMaskVDev.DrawHatch( aPolyPoly, aVclHatch );
                                Graphic   aGraphic( BitmapEx( aBitmap, aMaskVDev.GetBitmap( Point(), aOutSize ) ) );
                                aGraphicObject = aGraphic;
                            }
                            else
                                aGraphicObject = Graphic( aBitmap );

                        }
                        eBitmapMode = ::com::sun::star::drawing::BitmapMode_REPEAT;
                        aUniqueId = aGraphicObject.GetUniqueID();
                    }
                }
            }
            if ( aGraphicUrl.Len() )
            {
                xub_StrLen nIndex = aGraphicUrl.Search( (sal_Unicode)':', 0 );
                if ( nIndex != STRING_NOTFOUND )
                {
                    nIndex++;
                    if ( aGraphicUrl.Len() > nIndex  )
                        aUniqueId = ByteString( aGraphicUrl, nIndex, aGraphicUrl.Len() - nIndex, RTL_TEXTENCODING_UTF8 );
                }
            }
            if ( aUniqueId.Len() )
            {
                if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsMirrored" ) ), sal_True ) )
                    aAny >>= bMirrored;

                if ( bFillBitmap )
                {
                    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillBitmapMode" ) ), sal_True ) )
                        aAny >>= eBitmapMode;
                }
                else
                {
                    sal_uInt16 nAngle = ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                                            String( RTL_CONSTASCII_USTRINGPARAM( "RotateAngle" ) ), sal_True ) )
                                        ? (sal_uInt16)( ( *((sal_Int32*)aAny.getValue() ) ) + 5 ) / 10
                                        : 0;

                    if ( bMirrored || nAngle )
                    {
                        pGraphicAttr = new GraphicAttr;
                        pGraphicAttr->SetRotation( nAngle );
                        if ( bMirrored )
                            pGraphicAttr->SetMirrorFlags( BMP_MIRROR_HORZ );
                        if ( nAngle )   // ppoint does not rotate graphics !
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

                sal_uInt32 nBlibId = 0;
                if ( aUniqueId.Len() )
                    nBlibId = pGraphicProvider->GetBlibID( *pPicOutStrm, aUniqueId, aRect, pGraphicAttr );
                if ( nBlibId )
                {
                    if ( bFillBitmap )
                        AddOpt( ESCHER_Prop_fillBlip, nBlibId, sal_True );
                    else
                    {
                        AddOpt( ESCHER_Prop_pib, nBlibId, sal_True );
                        ImplCreateGraphicAttributes( rXPropSet, nBlibId, bCreateCroppingAttributes );
                    }
                    bRetValue = sal_True;
                }
            }
            delete pGraphicAttr;
        }
    }
    return bRetValue;
}

PolyPolygon EscherPropertyContainer::ImplGetPolyPolygon( const ::com::sun::star::uno::Any& rAny ) const
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
            ::com::sun::star::awt::Point* pArray;

            // dies wird ein Polygon set
            for( a = 0; a < nOuterSequenceCount; a++ )
            {
                ::com::sun::star::drawing::PointSequence* pInnerSequence = pOuterSequence++;
                bNoError = pInnerSequence != NULL;
                if ( bNoError )
                {
                    // Zeiger auf Arrays holen
                    if ( pArray = pInnerSequence->getArray() )
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
            ::com::sun::star::awt::Point* pArray;

            // Zeiger auf Arrays holen
            if ( pArray = pInnerSequence->getArray() )
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
            aPolyPolygon = ImplGetPolyPolygon( aAny );
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
                nPoints += k;
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
                for ( i = 0; i < nPoints; i++ )             // Punkte aus Polygon in Buffer schreiben
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
                for ( i = 0; i < nPoints; i++ )         // Polyflags in Buffer schreiben
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
            AddOpt( ESCHER_Prop_pVertices, TRUE, nVerticesBufSize - 6, (sal_uInt8*)pVerticesBuf, nVerticesBufSize );
            AddOpt( ESCHER_Prop_pSegmentInfo, TRUE, nSegmentBufSize, (sal_uInt8*)pSegmentBuf, nSegmentBufSize );
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

                        rShapeFlags = SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT;
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
/*
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeLine1Delta" ) ) ) )
                        {
                        }
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeLine2Delta" ) ) ) )
                        {
                        }
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeLine3Delta" ) ) ) )
                        {
                        }
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeNode1HorzDist" ) ) ) )
                        {
                        }
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeNode1VertDist" ) ) ) )
                        {
                        }
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeNode2HorzDist" ) ) ) )
                        {
                        }
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeNode2VertDist" ) ) ) )
                        {
                        }
*/
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
                                rShapeFlags |= SHAPEFLAG_CONNECTOR;
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

    sal_Bool    bHasShadow = sal_False; // shadow is possible only if at least a fillcolor or linecolor is set
    sal_uInt32  nLineFlags = 0;         // default : shape has no line
    sal_uInt32  nFillFlags = 0x10;      //           shape is filled

    GetOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );
    GetOpt( ESCHER_Prop_fNoFillHitTest, nFillFlags );

    if ( ( nLineFlags & 8 ) || ( nFillFlags & 0x10 ) )
    {
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                String( RTL_CONSTASCII_USTRINGPARAM( "Shadow" ) ), sal_True ) )
        {
            sal_Bool bBool;
            if ( aAny >>= bBool )
            {
                if ( bBool )
                {
                    bHasShadow = sal_True;
                    AddOpt( ESCHER_Prop_fshadowObscured, 0x20002 );
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
                        AddOpt( ESCHER_Prop_shadowOpacity,  ( ( ( 100 - (*((sal_uInt16*)aAny.getValue()) ) ) << 16 ) / 100 ) );
                }
            }
        }
    }
    return bHasShadow;
}

// ---------------------------------------------------------------------------------------------

EscherPersistTable::EscherPersistTable()
{
}

// ---------------------------------------------------------------------------------------------

EscherPersistTable::~EscherPersistTable()
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
        delete (EscherPersistEntry*)pPtr;
}

// ---------------------------------------------------------------------------------------------

BOOL EscherPersistTable::PtIsID( UINT32 nID )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((EscherPersistEntry*)pPtr)->mnID == nID )
            return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

void EscherPersistTable::PtInsert( UINT32 nID, UINT32 nOfs )
{
    maPersistTable.Insert( new EscherPersistEntry( nID, nOfs ) );
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherPersistTable::PtDelete( UINT32 nID )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((EscherPersistEntry*)pPtr)->mnID == nID )
        {
            UINT32 nRetValue = ((EscherPersistEntry*)pPtr)->mnOffset;
            delete (EscherPersistEntry*) maPersistTable.Remove();
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherPersistTable::PtGetOffsetByID( UINT32 nID )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((EscherPersistEntry*)pPtr)->mnID == nID )
            return ((EscherPersistEntry*)pPtr)->mnOffset;
    }
    return 0;
};

// ---------------------------------------------------------------------------------------------

UINT32 EscherPersistTable::PtReplace( UINT32 nID, UINT32 nOfs )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((EscherPersistEntry*)pPtr)->mnID == nID )
        {
            UINT32 nRetValue = ((EscherPersistEntry*)pPtr)->mnOffset;
            ((EscherPersistEntry*)pPtr)->mnOffset = nOfs;
            return nRetValue;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherPersistTable::PtReplaceOrInsert( UINT32 nID, UINT32 nOfs )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((EscherPersistEntry*)pPtr)->mnID == nID )
        {
            UINT32 nRetValue = ((EscherPersistEntry*)pPtr)->mnOffset;
            ((EscherPersistEntry*)pPtr)->mnOffset = nOfs;
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
                    sal_Bool bTestPropertyAvailability )
{
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
        catch( ::com::sun::star::uno::Exception& )
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
        catch( ::com::sun::star::uno::Exception& )
        {
            bRetValue = sal_False;
        }
    }
    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

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
    catch( ::com::sun::star::uno::Exception& )
    {
        //...
    }
    return eRetValue;
}

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

EscherBlibEntry::EscherBlibEntry( sal_uInt32 nPictureOffset, const GraphicObject& rObject, const ByteString& rId,
                                        const GraphicAttr* pGraphicAttr ) :
    mbIsEmpty       ( TRUE ),
    mnPictureOffset ( nPictureOffset ),
    mnRefCount      ( 1 ),
    mnSizeExtra     ( 0 ),
    maPrefSize      ( rObject.GetPrefSize() ),
    maPrefMapMode   ( rObject.GetPrefMapMode() )
{
    mbIsNativeGraphicPossible = ( pGraphicAttr == NULL );
    meBlibType = UNKNOWN;
    mnSize = 0;

    sal_uInt32      nLen = rId.Len();
    const sal_Char* pData = rId.GetBuffer();
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
                aSt << (sal_uInt16)pGraphicAttr->GetDrawMode()
                    << pGraphicAttr->GetMirrorFlags()
                    << pGraphicAttr->GetLeftCrop()
                    << pGraphicAttr->GetTopCrop()
                    << pGraphicAttr->GetRightCrop()
                    << pGraphicAttr->GetBottomCrop()
                    << pGraphicAttr->GetRotation()
                    << pGraphicAttr->GetLuminance()
                    << pGraphicAttr->GetContrast()
                    << pGraphicAttr->GetChannelR()
                    << pGraphicAttr->GetChannelG()
                    << pGraphicAttr->GetChannelB()
                    << pGraphicAttr->GetGamma()
                    << (BOOL)( pGraphicAttr->IsInvert() == TRUE )
                    << pGraphicAttr->GetTransparency();
                mnIdentifier[ 1 ] = rtl_crc32( 0, aSt.GetData(), aSt.Tell() );
            }
            else
                mbIsNativeGraphicPossible = TRUE;
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
        mbIsEmpty = FALSE;
    }
};

// ---------------------------------------------------------------------------------------------

void EscherBlibEntry::WriteBlibEntry( SvStream& rSt, sal_Bool bWritePictureOffset, sal_uInt32 nResize )
{
    sal_uInt8   nBlibType = meBlibType;
    sal_uInt32  nPictureOffset = ( bWritePictureOffset ) ? mnPictureOffset : 0;

    rSt << (sal_uInt32)( ( ESCHER_BSE << 16 ) | ( ( (sal_uInt16)meBlibType << 4 ) | 2 ) )
        << (sal_uInt32)( 36 + nResize )
        << (sal_uInt8)meBlibType;

    switch ( meBlibType )
    {
        case EMF :
        case WMF :  // EMF/WMF auf OS2 zu Pict Konvertieren
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

// ---------------------------------------------------------------------------------------------

EscherBlibEntry::~EscherBlibEntry()
{
};

// ---------------------------------------------------------------------------------------------

BOOL EscherBlibEntry::operator==( const EscherBlibEntry& rEscherBlibEntry ) const
{
    for ( int i = 0; i < 3; i++ )
    {
        if ( mnIdentifier[ i ] != rEscherBlibEntry.mnIdentifier[ i ] )
            return FALSE;
    }
    return TRUE;
}

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

EscherGraphicProvider::EscherGraphicProvider( sal_uInt32 nFlags ) :
    mnFlags         ( nFlags ),
    mpBlibEntrys    ( NULL ),
    mnBlibBufSize   ( 0 ),
    mnBlibEntrys    ( 0 )
{
}

EscherGraphicProvider::~EscherGraphicProvider()
{
    for ( UINT32 i = 0; i < mnBlibEntrys; delete mpBlibEntrys[ i++ ] );
    delete mpBlibEntrys;
}

void EscherGraphicProvider::SetNewBlipStreamOffset( sal_Int32 nOffset )
{
    for( sal_uInt32 i = 0; i < mnBlibEntrys; i++ )
    {
        EscherBlibEntry* pBlibEntry = mpBlibEntrys[ i ];
        pBlibEntry->mnPictureOffset += nOffset;
    }
}

UINT32 EscherGraphicProvider::ImplInsertBlib( EscherBlibEntry* p_EscherBlibEntry )
{
    if ( mnBlibBufSize == mnBlibEntrys )
    {
        mnBlibBufSize += 64;
        EscherBlibEntry** pTemp = new EscherBlibEntry*[ mnBlibBufSize ];
        for ( UINT32 i = 0; i < mnBlibEntrys; i++ )
        {
            pTemp[ i ] = mpBlibEntrys[ i ];
        }
        delete mpBlibEntrys;
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
            const nBuf = 0x40000;   // 256KB buffer
            sal_uInt8* pBuf = new sal_uInt8[ nBuf ];

            for ( i = 0; i < mnBlibEntrys; i++ )
            {
                EscherBlibEntry* pBlibEntry = mpBlibEntrys[ i ];

                sal_uInt8 nBlibType = pBlibEntry->meBlibType;
                nBlipSize = pBlibEntry->mnSize + pBlibEntry->mnSizeExtra;
                pBlibEntry->WriteBlibEntry( rSt, sal_False, nBlipSize );

                // BLIP
                pMergePicStreamBSE->Seek( pBlibEntry->mnPictureOffset );
                UINT16 n16;
                // record version and instance
                *pMergePicStreamBSE >> n16;
                rSt << n16;
                // record type
                *pMergePicStreamBSE >> n16;
                rSt << UINT16( ESCHER_BlipFirst + nBlibType );
                DBG_ASSERT( n16 == ESCHER_BlipFirst + nBlibType , "EscherEx::Flush: BLIP record types differ" );
                UINT32 n32;
                // record size
                *pMergePicStreamBSE >> n32;
                nBlipSize -= 8;
                rSt << nBlipSize;
                DBG_ASSERT( nBlipSize == n32, "EscherEx::Flush: BLIP sizes differ" );
                // record
                while ( nBlipSize )
                {
                    UINT32 nBytes = ( nBlipSize > nBuf ? nBuf : nBlipSize );
                    pMergePicStreamBSE->Read( pBuf, nBytes );
                    rSt.Write( pBuf, nBytes );
                    nBlipSize -= nBytes;
                }
            }
            delete [] pBuf;
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

sal_uInt32 EscherGraphicProvider::GetBlibID( SvStream& rPicOutStrm, const ByteString& rId,
                                                const Rectangle& rBoundRect, const GraphicAttr* pGraphicAttr )
{
    sal_uInt32          nBlibId = 0;
    GraphicAttr*        pAttr = NULL;
    const GraphicAttr*  pAttrUsed = pGraphicAttr;
    GraphicObject       aGraphicObject( rId );

    if ( pAttrUsed
            && pAttrUsed->GetRotation()
                && ( aGraphicObject.GetType() == GRAPHIC_GDIMETAFILE )
                    && ( mnFlags & _E_GRAPH_PROV_DO_NOT_ROTATE_METAFILES ) )
    {
        pAttr = new GraphicAttr;
        *pAttr = *pAttrUsed;
        pAttr->SetRotation( 0 );
        pAttrUsed = pAttr;
    }
    EscherBlibEntry* p_EscherBlibEntry = new EscherBlibEntry( rPicOutStrm.Tell(), aGraphicObject, rId, pAttrUsed );
    if ( !p_EscherBlibEntry->IsEmpty() )
    {
        for ( UINT32 i = 0; i < mnBlibEntrys; i++ )
        {
            if ( *( mpBlibEntrys[ i ] ) == *p_EscherBlibEntry )
            {
                mpBlibEntrys[ i ]->mnRefCount++;
                delete p_EscherBlibEntry;
                return i + 1;
            }
        }

        sal_Bool            bUseNativeGraphic( FALSE );

        Graphic             aGraphic( aGraphicObject.GetTransformedGraphic( pAttrUsed ) );
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
                }
                if ( p_EscherBlibEntry->meBlibType != UNKNOWN )
                    bUseNativeGraphic = TRUE;
            }
        }
        if ( !bUseNativeGraphic )
        {
            GraphicType eGraphicType = aGraphic.GetType();
            if ( ( eGraphicType == GRAPHIC_BITMAP ) || ( eGraphicType == GRAPHIC_GDIMETAFILE ) )
            {
                sal_uInt32 nErrCode = GraphicConverter::Export( aStream, aGraphic, ( eGraphicType == GRAPHIC_BITMAP ) ? CVT_PNG  : CVT_WMF );
                if ( nErrCode == ERRCODE_NONE )
                {
                    p_EscherBlibEntry->meBlibType = ( eGraphicType == GRAPHIC_BITMAP ) ? PNG : WMF;
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
                rPicOutStrm << (UINT32)( 0x7f90000 | (UINT16)( mnBlibEntrys << 4 ) )
                            << (UINT32)0;
                nAtomSize = rPicOutStrm.Tell();
                 if ( eBlibType == PNG )
                    rPicOutStrm << (sal_uInt16)0x0606;
                else if ( eBlibType == WMF )
                    rPicOutStrm << (sal_uInt16)0x0403;
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
                    nExtra = ( eBlibType == WMF ) ? 0x42 : 0x32;
                    p_EscherBlibEntry->mnSizeExtra = nExtra + 8;
                    nInstance = ( eBlibType == WMF ) ? 0xf01b2170 : 0xf01a3d40;
                    rPicOutStrm << nInstance << (sal_uInt32)( p_EscherBlibEntry->mnSize + nExtra );
                    if ( eBlibType == WMF )
                        rPicOutStrm.Write( p_EscherBlibEntry->mnIdentifier, 16 );
                    rPicOutStrm.Write( p_EscherBlibEntry->mnIdentifier, 16 );
                    UINT32 nWidth = rBoundRect.GetWidth() * 360;
                    UINT32 nHeight = rBoundRect.GetHeight() * 360;
                    double fWidth = (double)rBoundRect.GetWidth() / 10000.0 * 1027.0;
                    double fHeight = (double)rBoundRect.GetHeight() / 10000.0 * 1027.0;
                    rPicOutStrm << nUncompressedSize    // WMFSize ohne FileHeader
                                << (sal_Int32)0         // da die Originalgroesse des WMF's (ohne FileHeader)
                                << (sal_Int32)0         // nicht mehr feststellbar ist, schreiben wir 10cm / x
                                << (sal_Int32)fWidth
                                << (sal_Int32)fHeight
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
    if ( pAttr )
        delete pAttr;
    return nBlibId;
}

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

struct EscherConnectorRule
{
    sal_uInt32  nRuleId;
    sal_uInt32  nShapeA;        // SPID of shape A
    sal_uInt32  nShapeB;        // SPID of shape B
    sal_uInt32  nShapeC;        // SPID of connector shape
    sal_uInt32  ncptiA;         // Connection site Index of shape A
    sal_uInt32  ncptiB;         // Connection site Index of shape B
};

struct EscherConnectorListEntry
{
    ::com::sun::star::awt::Point            maPointA;
    ::com::sun::star::awt::Point            maPointB;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   mXConnector;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   mXConnectToA;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   mXConnectToB;

    sal_uInt32      GetConnectorRule( sal_Bool bFirst );

                    EscherConnectorListEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rC,
                                        const ::com::sun::star::awt::Point& rPA,
                                        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rSA ,
                                        const ::com::sun::star::awt::Point& rPB,
                                        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rSB ) :
                                            mXConnector ( rC ),
                                            maPointA    ( rPA ),
                                            maPointB    ( rPB ),
                                            mXConnectToA( rSA ),
                                            mXConnectToB( rSB ) {}

    sal_uInt32      GetClosestPoint( const Polygon& rPoly, const ::com::sun::star::awt::Point& rP );
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
// bei Rechtecken           bei Ellipsen    bei Polygonen
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

    String aString( (::rtl::OUString)aXShape->getShapeType() );
    ByteString aType( aString, RTL_TEXTENCODING_UTF8 );
    aType.Erase( 0, 19 );   // "smart.com.sun.star." entfernen
    sal_uInt16 nPos = aType.Search( "Shape" );
    aType.Erase( nPos, 5 );

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
        aPropertySet( aXShape, ::com::sun::star::uno::UNO_QUERY );

    if ( aType == "drawing.PolyPolygon" || aType == "drawing.PolyLine" )
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
    else if ( ( aType == "drawing.OpenBezier" ) || ( aType == "drawing.OpenFreeHand" ) || ( aType == "drawing.PolyLinePath" )
        || ( aType == "drawing.ClosedBezier" ) || ( aType == "drawing.ClosedFreeHand" ) || ( aType == "drawing.PolyPolygonPath" ) )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
            aPropertySet( aXShape, ::com::sun::star::uno::UNO_QUERY );
        if ( aPropertySet.is() )
        {
            if ( EscherPropertyValueHelper::GetPropertyValue( aAny,
                    aPropertySet, String( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygonBezier" ) ) ) )
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
            aPoly.Rotate( aRect.TopLeft(), ( nAngle + 5 ) / 10 );
        nRule = GetClosestPoint( aPoly, aRefPoint );
    }
    if ( aType == "drawing.Ellipse" )
        nRule <<= 1;    // In PPT hat eine Ellipse 8 Möglichkeiten sich zu connecten

    return nRule;
}

EscherSolverContainer::~EscherSolverContainer()
{
    for( void* pP = maShapeList.First(); pP; pP = maShapeList.Next() )
        delete (EscherShapeListEntry*)pP;
    for( pP = maConnectorList.First(); pP; pP = maConnectorList.Next() )
        delete (EscherConnectorListEntry*)pP;
}

void EscherSolverContainer::AddShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape, sal_uInt32 nId )
{
    maShapeList.Insert( new EscherShapeListEntry( rXShape, nId ), LIST_APPEND );
}

void EscherSolverContainer::AddConnector( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rConnector,
                                        const ::com::sun::star::awt::Point& rPA,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rConA,
                                        const ::com::sun::star::awt::Point& rPB,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rConB )
{
    maConnectorList.Insert( new EscherConnectorListEntry( rConnector, rPA, rConA, rPB, rConB ), LIST_APPEND );
}

sal_uInt32 EscherSolverContainer::ImplGetId( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape )
{
    for ( EscherShapeListEntry* pPtr = (EscherShapeListEntry*)maShapeList.First();
            pPtr; pPtr = (EscherShapeListEntry*)maShapeList.Next() )
    {
        if ( rXShape == pPtr->aXShape )
            return ( pPtr->n_EscherId );
    }
    return 0;
}

void EscherSolverContainer::WriteSolver( SvStream& rStrm )
{
    sal_uInt32 nCount = maConnectorList.Count();
    if ( nCount )
    {
        sal_uInt32  nRecHdPos, nCurrentPos, nSize;
        rStrm   << (sal_uInt16)( ( nCount << 4 ) | 0xf )    // open an ESCHER_SolverContainer
                << (sal_uInt16)ESCHER_SolverContainer       //
                << (sal_uInt32)0;                           //

        nRecHdPos = rStrm.Tell() - 4;

        EscherConnectorRule aConnectorRule;
        aConnectorRule.nRuleId = 2;
        for ( EscherConnectorListEntry* pPtr = (EscherConnectorListEntry*)maConnectorList.First();
                pPtr; pPtr = (EscherConnectorListEntry*)maConnectorList.Next() )
        {
            aConnectorRule.ncptiA = aConnectorRule.ncptiB = 0xffffffff;
            aConnectorRule.nShapeC = ImplGetId( pPtr->mXConnector );
            aConnectorRule.nShapeA = ImplGetId( pPtr->mXConnectToA );
            aConnectorRule.nShapeB = ImplGetId( pPtr->mXConnectToB );

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

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

EscherEx::EscherEx( SvStream& rOutStrm, UINT32 nDrawings ) :
    EscherGraphicProvider   ( _E_GRAPH_PROV_DO_NOT_ROTATE_METAFILES ),
    mpOutStrm               ( &rOutStrm ),
    mpOffsets               ( new sal_uInt32[ 32 ] ),
    mpRecTypes              ( new sal_uInt16[ 32 ] ),
    mnDrawings              ( nDrawings ),
    mnLevel                 ( 0 ),
    mbEscherSpgr            ( FALSE ),
    mbEscherDgg             ( FALSE ),                                      // TRUE, wenn jemals ein ESCHER_Dgg angelegt wurde, dieser wird dann im Dest. aktualisiert
    mbEscherDg              ( FALSE ),
    mbOleEmf                ( FALSE ),
    mnGroupLevel            ( 0 ),
    mnHellLayerId           ( USHRT_MAX )
{
    mnStrmStartOfs = mpOutStrm->Tell();
    mpImplEscherExSdr = new ImplEscherExSdr( *this );
}

// ---------------------------------------------------------------------------------------------

void EscherEx::Flush( SvStream* pPicStreamMergeBSE /* = NULL */ )
{
    if ( mbEscherDgg )                                                      // ESCHER_Dgg anpassen
    {
        PtReplaceOrInsert( ESCHER_Persist_CurrentPosition, mpOutStrm->Tell() );
        if ( DoSeek( ESCHER_Persist_Dgg ) )
        {
            *mpOutStrm << mnCurrentShapeID << (UINT32)( mnFIDCLs + 1 ) << mnTotalShapesDgg << mnDrawings;
        }
        if ( HasGraphics() )
        {
            if ( DoSeek( ESCHER_Persist_BlibStoreContainer ) )          // ESCHER_BlibStoreContainer schreiben
            {
                sal_uInt32 nAddBytes = GetBlibStoreContainerSize( pPicStreamMergeBSE );
                if ( nAddBytes )
                {
                    InsertAtCurrentPos( nAddBytes, TRUE );                  // platz schaffen fuer Blib Container samt seinen Blib Atomen
                    WriteBlibStoreContainer( *mpOutStrm, pPicStreamMergeBSE );
                }
            }
        }
        mpOutStrm->Seek( PtGetOffsetByID( ESCHER_Persist_CurrentPosition ) );
    }
}

// ---------------------------------------------------------------------------------------------

EscherEx::~EscherEx()
{
    delete[] mpRecTypes;
    delete[] mpOffsets;
    delete mpImplEscherExSdr;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::InsertAtCurrentPos( UINT32 nBytes, BOOL bContainer )
{
    UINT32  i, nSize, nType, nSource, nBufSize, nToCopy, nCurPos = mpOutStrm->Tell();
    BYTE*   pBuf;

    // Persist table anpassen
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        UINT32 nOfs = ((EscherPersistEntry*)pPtr)->mnOffset;
        if ( nOfs >= nCurPos )
            ((EscherPersistEntry*)pPtr)->mnOffset += nBytes;
    }

    // container und atom sizes anpassen
    mpOutStrm->Seek( mnStrmStartOfs );
    while ( mpOutStrm->Tell() < nCurPos )
    {
        *mpOutStrm >> nType >> nSize;
        if ( ( mpOutStrm->Tell() + nSize ) >= ( ( bContainer ) ? nCurPos + 1 : nCurPos ) )
        {
            mpOutStrm->SeekRel( -4 );
            *mpOutStrm << (UINT32)( nSize + nBytes );
            if ( ( nType & 0xf ) != 0xf )
                mpOutStrm->SeekRel( nSize );
        }
        else
            mpOutStrm->SeekRel( nSize );
    }

    // Container Offsets verschieben
    for ( i = 1; i <= (UINT32)mnLevel; i++ )
    {
        if ( mpOffsets[ i ] > nCurPos )
            mpOffsets[ i ] += nBytes;
    }
    mpOutStrm->Seek( STREAM_SEEK_TO_END );
    nSource = mpOutStrm->Tell();
    nToCopy = nSource - nCurPos;                        // Stream um nBytes vergroessern
    pBuf = new BYTE[ 0x40000 ];                         // 256KB Buffer
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
    delete pBuf;
    mpOutStrm->Seek( nCurPos );
}

// ---------------------------------------------------------------------------------------------

BOOL EscherEx::SeekBehindRecHeader( UINT16 nRecType )
{
    UINT32  nOldPos, nStreamEnd, nType, nSize;

    nOldPos = mpOutStrm->Tell();
    nStreamEnd = mpOutStrm->Seek( STREAM_SEEK_TO_END );
    mpOutStrm->Seek( nOldPos );
    while ( mpOutStrm->Tell() < nStreamEnd )
    {
        *mpOutStrm >> nType >> nSize;
        if ( ( nType >> 16 ) == nRecType )
            return TRUE;
        if ( ( nType & 0xf ) != 0xf )
            mpOutStrm->SeekRel( nSize );
    }
    mpOutStrm->Seek( nOldPos );
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::InsertPersistOffset( UINT32 nKey, UINT32 nOffset )
{
    PtInsert( ESCHER_Persist_PrivateEntry | nKey, nOffset );
}

// ---------------------------------------------------------------------------------------------

BOOL EscherEx::DoSeek( UINT32 nKey )
{
    UINT32 nPos = PtGetOffsetByID( nKey );
    if ( nPos )
        mpOutStrm->Seek( nPos );
    else
    {
        if (! PtIsID( nKey ) )
            return FALSE;
        mpOutStrm->Seek( 0 );
    }
    return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL EscherEx::SeekToPersistOffset( UINT32 nKey )
{
    return DoSeek( ESCHER_Persist_PrivateEntry | nKey );
}

// ---------------------------------------------------------------------------------------------

BOOL EscherEx::InsertAtPersistOffset( UINT32 nKey, UINT32 nValue )
{
    UINT32  nOldPos = mpOutStrm->Tell();
    BOOL    bRetValue = SeekToPersistOffset( nKey );
    if ( bRetValue )
    {
        *mpOutStrm << nValue;
        mpOutStrm->Seek( nOldPos );
    }
    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::OpenContainer( UINT16 nEscherContainer, int nRecInstance )
{
    *mpOutStrm << (UINT16)( ( nRecInstance << 4 ) | 0xf  ) << nEscherContainer << (UINT32)0;
    mpOffsets[ ++mnLevel ] = mpOutStrm->Tell() - 4;
    mpRecTypes[ mnLevel ] = nEscherContainer;

    switch( nEscherContainer )
    {
        case ESCHER_DggContainer :
        {
            mbEscherDgg = TRUE;
            mnFIDCLs = mnDrawings;
            mnCurrentDg = 0;
            mnCurrentShapeID = 0;
            mnTotalShapesDgg = 0;
            mnCurrentShapeMaximumID = 0;
            AddAtom( 16 + ( mnDrawings << 3 ), ESCHER_Dgg );                // an FDGG and several FIDCLs
            PtReplaceOrInsert( ESCHER_Persist_Dgg, mpOutStrm->Tell() );
            *mpOutStrm << (UINT32)0                                         // the current maximum shape ID
                       << (UINT32)0                                         // the number of ID clusters + 1
                       << (UINT32)0                                         // the number of total shapes saved
                       << (UINT32)0;                                        // the total number of drawings saved
            PtReplaceOrInsert( ESCHER_Persist_Dgg_FIDCL, mpOutStrm->Tell() );
            for ( UINT32 i = 0; i < mnFIDCLs; i++ )                         // Dummy FIDCLs einfuegen
            {
                *mpOutStrm << (UINT32)0 << (UINT32)0;                       // Drawing Nummer, Anzahl der Shapes in diesem IDCL
            }
            PtReplaceOrInsert( ESCHER_Persist_BlibStoreContainer, mpOutStrm->Tell() );
        }
        break;

        case ESCHER_DgContainer :
        {
            if ( mbEscherDgg )
            {
                if ( !mbEscherDg )
                {
                    mbEscherDg = TRUE;
                    mnCurrentDg++;
                    mnTotalShapesDg = 0;
                    mnTotalShapeIdUsedDg = 0;
                    mnCurrentShapeID = ( mnCurrentShapeMaximumID &~0x3ff ) + 0x400; // eine neue Seite bekommt immer eine neue ShapeId die ein vielfaches von 1024 ist,
                                                                                    // damit ist erste aktuelle Shape ID 0x400
                    AddAtom( 8, ESCHER_Dg, 0, mnCurrentDg );
                    PtReplaceOrInsert( ESCHER_Persist_Dg | mnCurrentDg, mpOutStrm->Tell() );
                    *mpOutStrm << (UINT32)0     // The number of shapes in this drawing
                               << (UINT32)0;    // The last MSOSPID given to an SP in this DG
                }
            }
        }
        break;

        case ESCHER_SpgrContainer :
        {
            if ( mbEscherDg )
            {
                mbEscherSpgr = TRUE;
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

// ---------------------------------------------------------------------------------------------

void EscherEx::CloseContainer()
{
    sal_uInt32 nSize, nPos = mpOutStrm->Tell();
    nSize = ( nPos - mpOffsets[ mnLevel ] ) - 4;
    mpOutStrm->Seek( mpOffsets[ mnLevel ] );
    *mpOutStrm << nSize;

    switch( mpRecTypes[ mnLevel ] )
    {
        case ESCHER_DgContainer :
        {
            if ( mbEscherDg )
            {
                mbEscherDg = FALSE;
                if ( DoSeek( ESCHER_Persist_Dg | mnCurrentDg ) )
                {
                    // shapeanzahl des drawings setzen
                    mnTotalShapesDgg += mnTotalShapesDg;
                    *mpOutStrm << mnTotalShapesDg << mnCurrentShapeMaximumID;
                    if ( DoSeek( ESCHER_Persist_Dgg_FIDCL ) )
                    {
                        if ( mnTotalShapesDg == 0 )
                        {
                            mpOutStrm->SeekRel( 8 );
                        }
                        else
                        {
                            if ( mnTotalShapeIdUsedDg )
                            {
                                // die benutzten Shape Ids des drawings in die fidcls setzen
                                UINT32 i, nFIDCL = ( ( mnTotalShapeIdUsedDg - 1 ) / 0x400 );
                                if ( nFIDCL )
                                {
                                    if ( nPos > mpOutStrm->Tell() )
                                        nPos += ( nFIDCL << 3 );

                                    mnFIDCLs += nFIDCL;
                                    InsertAtCurrentPos( nFIDCL << 3 );          // platz schaffen fuer weitere FIDCL's
                                }
                                for ( i = 0; i <= nFIDCL; i++ )
                                {
                                    *mpOutStrm << mnCurrentDg;
                                    if ( i < nFIDCL )
                                        *mpOutStrm << (UINT32)0x400;
                                    else
                                    {
                                        UINT32 nShapesLeft = mnTotalShapeIdUsedDg % 0x400;
                                        if ( !nShapesLeft )
                                            nShapesLeft = 0x400;
                                        *mpOutStrm << (UINT32)nShapesLeft;
                                    }
                                }
                            }
                        }
                        PtReplaceOrInsert( ESCHER_Persist_Dgg_FIDCL, mpOutStrm->Tell() );   // neuen FIDCL Offset fuer naechste Seite
                    }
                }
            }
        }
        break;

        case ESCHER_SpgrContainer :
        {
            if ( mbEscherSpgr )
            {
                mbEscherSpgr = FALSE;

            }
        }
        break;

        default:
        break;
    }
    mnLevel--;
    mpOutStrm->Seek( nPos );
}

// ---------------------------------------------------------------------------------------------

void EscherEx::BeginAtom()
{
    mnCountOfs = mpOutStrm->Tell();
    *mpOutStrm << (UINT32)0 << (UINT32)0;       // record header wird spaeter geschrieben
}

// ---------------------------------------------------------------------------------------------

void EscherEx::EndAtom( UINT16 nRecType, int nRecVersion, int nRecInstance )
{
    UINT32  nOldPos = mpOutStrm->Tell();
    mpOutStrm->Seek( mnCountOfs );
    sal_uInt32 nSize = nOldPos - mnCountOfs;
    *mpOutStrm << (UINT16)( ( nRecInstance << 4 ) | ( nRecVersion & 0xf ) ) << nRecType << (UINT32)( nSize - 8 );
    mpOutStrm->Seek( nOldPos );
}

// ---------------------------------------------------------------------------------------------

void EscherEx::AddAtom( UINT32 nAtomSize, UINT16 nRecType, int nRecVersion, int nRecInstance )
{
    *mpOutStrm << (UINT16)( ( nRecInstance << 4 ) | ( nRecVersion & 0xf ) ) << nRecType << nAtomSize;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::AddClientAnchor( const Rectangle& rRect )
{
    AddAtom( 8, ESCHER_ClientAnchor );
    *mpOutStrm << (INT16)rRect.Top() << (INT16)rRect.Left() << (INT16)rRect.Right() << (INT16)rRect.Bottom();
}

// ---------------------------------------------------------------------------------------------

EscherExHostAppData* EscherEx::EnterAdditionalTextGroup()
{
    return NULL;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::EnterGroup( const Rectangle* pBoundRect )
{
    Rectangle aRect;
    if( pBoundRect )
        aRect = *pBoundRect;

    OpenContainer( ESCHER_SpgrContainer );
    OpenContainer( ESCHER_SpContainer );
    AddAtom( 16, ESCHER_Spgr, 1 );
    PtReplaceOrInsert( ESCHER_Persist_Grouping_Snap | mnGroupLevel,
                        mpOutStrm->Tell() );
    *mpOutStrm  << (INT32)aRect.Left()  // Bounding box fuer die Gruppierten shapes an die sie attached werden
                << (INT32)aRect.Top()
                << (INT32)aRect.Right()
                << (INT32)aRect.Bottom();

    UINT32 nShapeId = GetShapeID();
    if ( !mnGroupLevel )
        AddShape( ESCHER_ShpInst_Min, 5, nShapeId );                    // Flags: Group | Patriarch
    else
    {
        AddShape( ESCHER_ShpInst_Min, 0x201, nShapeId );                // Flags: Group | HaveAnchor
        EscherPropertyContainer aPropOpt;
        aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x00040004 );
        aPropOpt.Commit( *mpOutStrm );
        if ( mnGroupLevel > 1 )
        {
            AddAtom( 16, ESCHER_ChildAnchor );
            *mpOutStrm  << (INT32)aRect.Left()
                        << (INT32)aRect.Top()
                           << (INT32)aRect.Right()
                        << (INT32)aRect.Bottom();
        }
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

// ---------------------------------------------------------------------------------------------

BOOL EscherEx::SetGroupSnapRect( UINT32 nGroupLevel, const Rectangle& rRect )
{
    BOOL bRetValue = FALSE;
    if ( nGroupLevel )
    {
        UINT32 nCurrentPos = mpOutStrm->Tell();
        if ( DoSeek( ESCHER_Persist_Grouping_Snap | ( nGroupLevel - 1 ) ) )
        {
            *mpOutStrm  << (INT32)rRect.Left()  // Bounding box fuer die Gruppierten shapes an die sie attached werden
                        << (INT32)rRect.Top()
                        << (INT32)rRect.Right()
                        << (INT32)rRect.Bottom();
            mpOutStrm->Seek( nCurrentPos );
        }
    }
    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

BOOL EscherEx::SetGroupLogicRect( UINT32 nGroupLevel, const Rectangle& rRect )
{
    BOOL bRetValue = FALSE;
    if ( nGroupLevel )
    {
        UINT32 nCurrentPos = mpOutStrm->Tell();
        if ( DoSeek( ESCHER_Persist_Grouping_Logic | ( nGroupLevel - 1 ) ) )
        {
            *mpOutStrm << (INT16)rRect.Top() << (INT16)rRect.Left() << (INT16)rRect.Right() << (INT16)rRect.Bottom();
            mpOutStrm->Seek( nCurrentPos );
        }
    }
    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::LeaveGroup()
{
    --mnGroupLevel;
    PtDelete( ESCHER_Persist_Grouping_Snap | mnGroupLevel );
    PtDelete( ESCHER_Persist_Grouping_Logic | mnGroupLevel );
    CloseContainer();
}

// ---------------------------------------------------------------------------------------------

void EscherEx::AddShape( UINT32 nShpInstance, UINT32 nFlags, UINT32 nShapeID )
{
    AddAtom( 8, ESCHER_Sp, 2, nShpInstance );

    if ( !nShapeID )
        nShapeID = GetShapeID();

    if ( nFlags ^ 1 )                           // is this a group shape ?
    {                                           // if not
        if ( mnGroupLevel > 1 )
            nFlags |= 2;                        // this not a topmost shape
    }
    *mpOutStrm << nShapeID << nFlags;

    if ( mbEscherSpgr )
        mnTotalShapesDg++;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::GetShapeID()
{
    mnCurrentShapeMaximumID = mnCurrentShapeID; // maximum setzen
    mnCurrentShapeID++;                         // mnCurrentShape ID auf nachste freie ID
    mnTotalShapeIdUsedDg++;
    return mnCurrentShapeMaximumID;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::GetColor( const UINT32 nSOColor, BOOL bSwap )
{
    if ( bSwap )
    {
        UINT32 nColor = nSOColor & 0xff00;      // GRUEN
        nColor |= (BYTE)( nSOColor ) << 16;     // ROT
        nColor |= (BYTE)( nSOColor >> 16 );     // BLAU
        return nColor;
    }
    else
        return nSOColor & 0xffffff;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::GetColor( const Color& rSOColor, BOOL bSwap )
{
    UINT32 nColor = ( rSOColor.GetRed() << 16 );
    nColor |= ( rSOColor.GetGreen() << 8 );
    nColor |= rSOColor.GetBlue();

    if ( !bSwap )
        nColor = GetColor( nColor, TRUE );

    return nColor;
}

// ---------------------------------------------------------------------------------------------
