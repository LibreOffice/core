/*************************************************************************
 *
 *  $RCSfile: escherex.cxx,v $
 *
 *  $Revision: 1.44 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:19:25 $
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
#ifndef _SVX_IMPGRF_HXX
#include "impgrf.hxx"
#endif
#ifndef _ESCHESDO_HXX
#include "eschesdo.hxx"
#endif
#ifndef _SVX_ESCHEREX_HXX
#include "escherex.hxx"
#endif
#ifndef _SVX_UNOAPI_HXX_
#include <unoapi.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svdmodel.hxx>
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
#ifndef _ENHANCED_CUSTOMSHAPE_TYPE_NAMES_HXX
#include "../customshapes/EnhancedCustomShapeTypeNames.hxx"
#endif
#ifndef _ENHANCEDCUSTOMSHAPEGEOMETRY_HXX
#include "../customshapes/EnhancedCustomShapeGeometry.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
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
#ifndef _COM_SUN_STAR_DRAWING_CIRCLEKIND_HPP_
#include <com/sun/star/drawing/CircleKind.hpp>
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
#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE_HDL_
#include <com/sun/star/text/WritingMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTVERTICALADJUST_HPP_
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTHORIZONTALADJUST_HPP_
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETERPARIR_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPESEGMENT_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETERTYPE_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPESEGMENTCOMMAND_hpp_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEEQUATION_HPP__
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeEquation.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEOPERATION_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeOperation.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEEQUATION_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeEquation.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPETEXTFRAME_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEADJUSTMENTVALUE_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XSIMPLETEXT_HPP_
#include <com/sun/star/text/XSimpleText.hpp>
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
#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif
#ifndef _UNOTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
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
using namespace ::com::sun::star;

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

    sal_uInt32  nFillType = ESCHER_FillShadeScale;
    sal_uInt32  nAngle = 0;
    sal_uInt32  nFillFocus = 0;
    sal_uInt32  nFillLR = 0;
    sal_uInt32  nFillTB = 0;
    sal_uInt32  nFirstColor = 0;
    bool        bWriteFillTo = false;

    if ( EscherPropertyValueHelper::GetPropertyValue(
            aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillGradient" ) ), sal_False ) )
    {
        pGradient = (::com::sun::star::awt::Gradient*)aAny.getValue();

        switch ( pGradient->Style )
        {
            case ::com::sun::star::awt::GradientStyle_LINEAR :
            case ::com::sun::star::awt::GradientStyle_AXIAL :
            {
                nFillType = ESCHER_FillShadeScale;
                nAngle = (pGradient->Angle * 0x10000) / 10;
                nFillFocus = (pGradient->Style == GradientStyle_LINEAR) ? 0 : 50;
            }
            break;
            case ::com::sun::star::awt::GradientStyle_RADIAL :
            case ::com::sun::star::awt::GradientStyle_ELLIPTICAL :
            case ::com::sun::star::awt::GradientStyle_SQUARE :
            case ::com::sun::star::awt::GradientStyle_RECT :
            {
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
        }
    }
    AddOpt( ESCHER_Prop_fillType, nFillType );
    AddOpt( ESCHER_Prop_fillAngle, nAngle );
    AddOpt( ESCHER_Prop_fillColor, GetGradientColor( pGradient, nFirstColor ) );
    AddOpt( ESCHER_Prop_fillBackColor, GetGradientColor( pGradient, nFirstColor ^ 1 ) );
    AddOpt( ESCHER_Prop_fillFocus, nFillFocus );
    if ( bWriteFillTo )
    {
        AddOpt( ESCHER_Prop_fillToLeft, nFillLR );
        AddOpt( ESCHER_Prop_fillToTop, nFillTB );
        AddOpt( ESCHER_Prop_fillToRight, nFillLR );
        AddOpt( ESCHER_Prop_fillToBottom, nFillTB );
    }
};

void EscherPropertyContainer::CreateFillProperties(
    const uno::Reference< beans::XPropertySet > & rXPropSet,
        sal_Bool bEdge )
{
    ::com::sun::star::uno::Any aAny;
    AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapNone );
    AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );

    sal_uInt32 nFillBackColor = 0;

    const rtl::OUString aPropName( String( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ) ) );
    if ( EscherPropertyValueHelper::GetPropertyValue(
            aAny, rXPropSet, aPropName, sal_False ) )
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
                ::com::sun::star::beans::PropertyState ePropState = EscherPropertyValueHelper::GetPropertyState(
                    rXPropSet, aPropName );
                if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
                    AddOpt( ESCHER_Prop_fillType, ESCHER_FillSolid );

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

void EscherPropertyContainer::CreateTextProperties(
    const uno::Reference< beans::XPropertySet > & rXPropSet, sal_uInt32 nTextId,
        const sal_Bool bIsCustomShape )
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
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextAutoGrowSize" ) ), sal_True ) )
            aAny >>= bAutoGrowSize;
    }
    else
    {
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextAutoGrowWidth" ) ), sal_True ) )
            aAny >>= bAutoGrowWidth;
        if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextAutoGrowHeight" ) ), sal_True ) )
            aAny >>= bAutoGrowHeight;
    }
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextLeftDistance" ) ) ) )
        aAny >>= nLeft;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextUpperDistance" ) ) ) )
        aAny >>= nTop;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextRightDistance" ) ) ) )
        aAny >>= nRight;
    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextLowerDistance" ) ) ) )
        aAny >>= nBottom;

/*
    if ( rObj.ImplGetPropertyValue(
        ::rtl::OUString::createFromAscii("TextWritingMode") ) )
    {
        ::com::sun::star::text::WritingMode eMode;
        rObj.GetUsrAny() >>= eMode;
        switch (eMode)
        {
            case ::com::sun::star::text::WritingMode_TB_RL:
                //Well if it so happens that we are fliped 180 we can use
                //this instead.
                if (rObj.GetAngle() == 18000)
                    eFlow = ESCHER_txflBtoT;
                else
                    eFlow = ESCHER_txflTtoBA;
                break;
            case ::com::sun::star::text::WritingMode_RL_TB:
                eDir = ESCHER_txdirRTL;
                break;
        }
    }
*/

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
}

static sal_Bool GetLineArrow( const sal_Bool bLineStart,
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
                if ( aApiName.getLength() )
                {

                    /* todo:
                    calculate the best option for ArrowLenght and ArrowWidth
                    */
                    if ( aApiName.equalsAscii( "Arrow concave" ) )
                        reLineEnd = ESCHER_LineArrowStealthEnd;
                    else if ( aApiName.equalsAscii( "Square 45" ) )
                        reLineEnd = ESCHER_LineArrowDiamondEnd;
                    else if ( aApiName.equalsAscii( "Small Arrow" ) )
                        reLineEnd = ESCHER_LineArrowEnd;
                    else if ( aApiName.equalsAscii( "Dimension Lines" ) )
                    {
                        rnArrowLength = 0;
                        rnArrowWidth  = 2;
                        reLineEnd = ESCHER_LineArrowOvalEnd;
                    }
                    else if ( aApiName.equalsAscii( "Double Arrow" ) )
                        reLineEnd = ESCHER_LineArrowEnd;
                    else if ( aApiName.equalsAscii( "Rounded short Arrow" ) )
                        reLineEnd = ESCHER_LineArrowEnd;
                    else if ( aApiName.equalsAscii( "Symmetric Arrow" ) )
                        reLineEnd = ESCHER_LineArrowEnd;
                    else if ( aApiName.equalsAscii( "Line Arrow" ) )
                        reLineEnd = ESCHER_LineArrowOpenEnd;
                    else if ( aApiName.equalsAscii( "Rounded large Arrow" ) )
                        reLineEnd = ESCHER_LineArrowEnd;
                    else if ( aApiName.equalsAscii( "Circle" ) )
                        reLineEnd = ESCHER_LineArrowOvalEnd;
                    else if ( aApiName.equalsAscii( "Square" ) )
                        reLineEnd = ESCHER_LineArrowDiamondEnd;
                    else if ( aApiName.equalsAscii( "Arrow" ) )
                        reLineEnd = ESCHER_LineArrowEnd;
                }
                else if ( aArrowStartName.GetTokenCount( ' ' ) == 2 )
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
        sal_Int16 nC;
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



sal_Bool EscherPropertyContainer::CreateGraphicProperties(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
        const String& rSource, sal_Bool bFillBitmap, sal_Bool bCreateCroppingAttributes )
{
    sal_Bool        bRetValue = sal_False;
    sal_Bool        bCreateFillStyles = sal_False;

    sal_Bool        bMirrored = sal_False;
    sal_Bool        bRotate   = sal_True;
    GraphicAttr*    pGraphicAttr = NULL;
    GraphicObject   aGraphicObject;
    String          aGraphicUrl;
    ByteString      aUniqueId;

    ::com::sun::star::drawing::BitmapMode   eBitmapMode( ::com::sun::star::drawing::BitmapMode_NO_REPEAT );
    ::com::sun::star::uno::Any aAny;

    if ( EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet, rSource ) )
    {
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
            bCreateFillStyles = sal_True;
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
            String aVndUrl( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.GraphicObject:" ) );
            xub_StrLen nIndex = aGraphicUrl.Search( aVndUrl, 0 );
            if ( nIndex != STRING_NOTFOUND )
            {
                nIndex += aVndUrl.Len();
                if ( aGraphicUrl.Len() > nIndex  )
                    aUniqueId = ByteString( aGraphicUrl, nIndex, aGraphicUrl.Len() - nIndex, RTL_TEXTENCODING_UTF8 );
            }
            else
            {
                INetURLObject   aTmp( aGraphicUrl );
                SvStream* pIn = ::utl::UcbStreamHelper::CreateStream(
                    aTmp.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );
                if ( pIn )
                {
                    Graphic aGraphic;
                    sal_uInt32 nErrCode = GraphicConverter::Import( *pIn, aGraphic );
                    if ( nErrCode == ERRCODE_NONE )
                    {
                        aGraphicObject = aGraphic;
                        aUniqueId = aGraphicObject.GetUniqueID();
                    }
                    delete pIn;
                }
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
                sal_uInt16 nAngle = bRotate && EscherPropertyValueHelper::GetPropertyValue( aAny, rXPropSet,
                                        String( RTL_CONSTASCII_USTRINGPARAM( "RotateAngle" ) ), sal_True )
                                    ? (sal_uInt16)( ( *((sal_Int32*)aAny.getValue() ) ) + 5 ) / 10
                                    : 0;

                if ( bMirrored || nAngle )
                {
                    pGraphicAttr = new GraphicAttr;
                    if ( bMirrored )
                        pGraphicAttr->SetMirrorFlags( BMP_MIRROR_HORZ );
                    GraphicObject aTmpGraphicObject( aUniqueId );
                    if ( aTmpGraphicObject.GetType() == GRAPHIC_GDIMETAFILE )
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
            }
            if ( eBitmapMode == ::com::sun::star::drawing::BitmapMode_REPEAT )
                AddOpt( ESCHER_Prop_fillType, ESCHER_FillTexture );
            else
                AddOpt( ESCHER_Prop_fillType, ESCHER_FillPicture );

            if ( pGraphicProvider && pPicOutStrm && pShapeBoundRect )
            {
                Rectangle aRect( Point( 0, 0 ), pShapeBoundRect->GetSize() );

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
            else
            {
                EscherGraphicProvider aProvider;
                SvMemoryStream aMemStrm;
                Rectangle aRect;

                if ( aUniqueId.Len() && aProvider.GetBlibID( aMemStrm, aUniqueId, aRect, pGraphicAttr ) )
                {
                    // grab BLIP from stream and insert directly as complex property
                    // ownership of stream memory goes to complex property
                    aMemStrm.ObjectOwnsMemory( FALSE );
                    sal_uInt8* pBuf = (sal_uInt8*) aMemStrm.GetData();
                    sal_uInt32 nSize = aMemStrm.Seek( STREAM_SEEK_TO_END );
                    AddOpt( ESCHER_Prop_fillBlip, sal_True, nSize, pBuf, nSize );
                    bRetValue = sal_True;
                }
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
                        sal_Bool bUseConnections = sal_True;

                        /* #i11701# SJ: There is a bug in our API - in Writer documents the EdgeStartPoint
                        and EdgeEndPoint properties are in TWIPs instead of 100TH_MM. This is also wrong
                        in our xml file format, but CL do not want to change the file format
                        before 6y and OOo2.0. So there is only following HACK which has to be removed if
                        the file format is fixed by i13778#
                        */
                        SdrObject* pCon = GetSdrObjectFromXShape( rXShape );
                        if ( pCon )
                        {
                            SdrModel* pMod = pCon->GetModel();
                            if ( pMod && ( pMod->GetScaleUnit() == MAP_TWIP ) )
                            {
                                Point aStart( OutputDevice::LogicToLogic( Point( aStartPoint.X, aStartPoint.Y ), MAP_TWIP, MAP_100TH_MM ) );
                                Point aEnd( OutputDevice::LogicToLogic( Point( aEndPoint.X, aEndPoint.Y ), MAP_TWIP, MAP_100TH_MM ) );
                                aStartPoint = com::sun::star::awt::Point( aStart.X(), aStart.Y() );
                                aEndPoint = com::sun::star::awt::Point( aEnd.X(), aEnd.Y() );
                                bUseConnections = sal_False;
                            }
                        }

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

                        if( bUseConnections )
                        {
                            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sEdgeStartConnection ) )
                                aAny >>= aShapeA;
                            if ( EscherPropertyValueHelper::GetPropertyValue( aAny, aXPropSet, sEdgeEndConnection ) )
                                aAny >>= aShapeB;
                        }
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

    sal_uInt32 nShadowFlags = 0x20000;
    if ( ( nLineFlags & 8 ) || ( nFillFlags & 0x10 ) )
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

// ---------------------------------------------------------------------------------------------

sal_Int32 GetValueForEnhancedCustomShapeParameter( const drafts::com::sun::star::drawing::EnhancedCustomShapeParameter& rParameter )
{
    sal_Int32 nValue = 0;
    if ( rParameter.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
    {
        double fValue;
        if ( rParameter.Value >>= fValue )
            nValue = (sal_Int32)fValue;
    }
    else
        rParameter.Value >>= nValue;

    switch( rParameter.Type )
    {
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::EQUATION :
        {
            nValue &= 0xffff;
            nValue |= (sal_uInt32)0x80000000;
        }
        break;
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::NORMAL :
        {

        }
        break;
/* not sure if it is allowed to set following values
(but they are not yet used)
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::ADJUSTMENT :
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::BOTTOM :
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::RIGHT :
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::TOP :
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::LEFT :
*/
    }
    return nValue;
}

sal_Bool GetValueForEnhancedCustomShapeHandleParameter( sal_Int32& nRetValue, const drafts::com::sun::star::drawing::EnhancedCustomShapeParameter& rParameter )
{
    sal_Bool bSpecial = sal_False;
    nRetValue = 0;
    if ( rParameter.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
    {
        double fValue;
        if ( rParameter.Value >>= fValue )
            nRetValue = (sal_Int32)fValue;
    }
    else
        rParameter.Value >>= nRetValue;

    switch( rParameter.Type )
    {
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::EQUATION :
        {
            nRetValue += 3;
            bSpecial = sal_True;
        }
        break;
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::ADJUSTMENT :
        {
            nRetValue += 0x100;
            bSpecial = sal_True;
        }
        break;
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::TOP :
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::LEFT :
        {
            nRetValue = 0;
            bSpecial = sal_True;
        }
        break;
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::RIGHT :
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::BOTTOM :
        {
            nRetValue = 1;
            bSpecial = sal_True;
        }
        break;
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::CENTER :
        {
            nRetValue = 2;
            bSpecial = sal_True;
        }
        break;
        case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::NORMAL :
        {

        }
        break;
    }
    return bSpecial;
}

void EscherPropertyContainer::CreateCustomShapeProperties( const MSO_SPT eShapeType, const uno::Reference< drawing::XShape > & rXShape )
{
    uno::Reference< beans::XPropertySet > aXPropSet( rXShape, uno::UNO_QUERY );
    if ( aXPropSet.is() )
    {
        const rtl::OUString sCustomShapeGeometry( RTL_CONSTASCII_USTRINGPARAM( "CustomShapeGeometry" ) );
        uno::Any aGeoPropSet = aXPropSet->getPropertyValue( sCustomShapeGeometry );
        uno::Sequence< beans::PropertyValue > aGeoPropSeq;

        awt::Point aCoordinateOrigin( 0, 0 );
        const beans::PropertyValue* pCoordinateSizeProp = NULL;
        const beans::PropertyValue* pAdjustmentValuesProp = NULL;

        sal_Bool bPredefinedHandlesUsed = sal_True;
        sal_Int32 nAdjustmentsWhichNeedsToBeConverted = 0;
        uno::Sequence< beans::PropertyValues > aHandlesPropSeq;

        if ( aGeoPropSet >>= aGeoPropSeq )
        {
            sal_Int32 i, nCount = aGeoPropSeq.getLength();
            for ( i = 0; i < nCount; i++ )
            {
                const beans::PropertyValue& rProp = aGeoPropSeq[ i ];
                const rtl::OUString sCoordinateOrigin   ( RTL_CONSTASCII_USTRINGPARAM( "CoordinateOrigin" ) );
                const rtl::OUString sCoordinateSize     ( RTL_CONSTASCII_USTRINGPARAM( "CoordinateSize" ) );
                const rtl::OUString sTextRotateAngle    ( RTL_CONSTASCII_USTRINGPARAM( "TextRotateAngle" ) );
                const rtl::OUString sCallout            ( RTL_CONSTASCII_USTRINGPARAM( "Callout" ) );
                const rtl::OUString sExtrusion          ( RTL_CONSTASCII_USTRINGPARAM( "Extrusion" ) );
                const rtl::OUString sEquations          ( RTL_CONSTASCII_USTRINGPARAM( "Equations" ) );
                const rtl::OUString sPath               ( RTL_CONSTASCII_USTRINGPARAM( "Path" ) );
                const rtl::OUString sTextPath           ( RTL_CONSTASCII_USTRINGPARAM( "TextPath" ) );
                const rtl::OUString sHandles            ( RTL_CONSTASCII_USTRINGPARAM( "Handles" ) );
                const rtl::OUString sAdjustmentValues   ( RTL_CONSTASCII_USTRINGPARAM( "AdjustmentValues" ) );

                if ( rProp.Name.equals( sCoordinateOrigin ) )
                {
                    if ( rProp.Value >>= aCoordinateOrigin )
                    {
                        AddOpt( DFF_Prop_geoLeft, aCoordinateOrigin.X );
                        AddOpt( DFF_Prop_geoTop,  aCoordinateOrigin.Y );
                    }
                }
                else if ( rProp.Name.equals( sCoordinateSize ) )
                {
                    // it is required, that the origin has been already read,
                    // so we are calculating the size later
                    pCoordinateSizeProp = &rProp;
                }
                else if ( rProp.Name.equals( sTextRotateAngle ) )
                {
                    double f, fTextRotateAngle;
                    if ( rProp.Value >>= f )
                    {
                        fTextRotateAngle = fmod( f, 360.0 );
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
                else if ( rProp.Name.equals( sCallout ) )
                {
                    uno::Sequence< beans::PropertyValue > aCalloutPropSeq;
                    if ( rProp.Value >>= aCalloutPropSeq )
                    {
                        sal_uInt32 nCalloutFlagsOrg, nCalloutFlags;
                        nCalloutFlagsOrg = nCalloutFlags = 0x10;        // default
                        if ( GetOpt( DFF_Prop_fCalloutLengthSpecified, nCalloutFlags ) )
                            nCalloutFlagsOrg = nCalloutFlags;

                        sal_Int32 i, nCount = aCalloutPropSeq.getLength();
                        for ( i = 0; i < nCount; i++ )
                        {
                            const beans::PropertyValue& rProp = aCalloutPropSeq[ i ];
                            const rtl::OUString sCalloutOn              ( RTL_CONSTASCII_USTRINGPARAM( "On" ) );
                            const rtl::OUString sCalloutAccentBar       ( RTL_CONSTASCII_USTRINGPARAM( "AccentBar" ) );
                            const rtl::OUString sCalloutAngle           ( RTL_CONSTASCII_USTRINGPARAM( "Angle" ) );
                            const rtl::OUString sCalloutDistance        ( RTL_CONSTASCII_USTRINGPARAM( "Distance" ) );
                            const rtl::OUString sCalloutDrop            ( RTL_CONSTASCII_USTRINGPARAM( "Drop" ) );
                            const rtl::OUString sCalloutDropAuto        ( RTL_CONSTASCII_USTRINGPARAM( "DropAuto" ) );
                            const rtl::OUString sCalloutGap             ( RTL_CONSTASCII_USTRINGPARAM( "Gap" ) );
                            const rtl::OUString sCalloutLength          ( RTL_CONSTASCII_USTRINGPARAM( "Length" ) );
                            const rtl::OUString sCalloutLengthSpecified ( RTL_CONSTASCII_USTRINGPARAM( "LengthSpecified" ) );
                            const rtl::OUString sCalloutFlipX           ( RTL_CONSTASCII_USTRINGPARAM( "FlipX" ) );
                            const rtl::OUString sCalloutFlipY           ( RTL_CONSTASCII_USTRINGPARAM( "FlipY" ) );
                            const rtl::OUString sCalloutTextBorder      ( RTL_CONSTASCII_USTRINGPARAM( "TextBorder" ) );
                            const rtl::OUString sCalloutType            ( RTL_CONSTASCII_USTRINGPARAM( "Type" ) );

                            if ( rProp.Name.equals( sCalloutOn ) )
                            {
                                sal_Bool bCalloutOn;
                                if ( rProp.Value >>= bCalloutOn )
                                {
                                    nCalloutFlags |= 0x400000;
                                    if ( bCalloutOn )
                                        nCalloutFlags |= 0x40;
                                    else
                                        nCalloutFlags &=~0x40;
                                }
                            }
                            else if ( rProp.Name.equals( sCalloutAccentBar ) )
                            {
                                sal_Bool bAccentBar;
                                if ( rProp.Value >>= bAccentBar )
                                {
                                    nCalloutFlags |= 0x200000;
                                    if ( bAccentBar )
                                        nCalloutFlags |= 0x20;
                                    else
                                        nCalloutFlags &=~0x20;
                                }
                            }
                            else if ( rProp.Name.equals( sCalloutAngle ) )
                            {
                                double fAngle;
                                if ( rProp.Value >>= fAngle )
                                {
                                    fAngle *= 65536;
                                    sal_Int32 nAngle = (sal_Int32)fAngle;
                                    AddOpt( DFF_Prop_spcoa, nAngle );
                                }
                            }
                            else if ( rProp.Name.equals( sCalloutDistance ) )
                            {
                                double fDistance;
                                if ( rProp.Value >>= fDistance )
                                {
                                    fDistance *= 360.0;
                                    AddOpt( DFF_Prop_dxyCalloutDropSpecified, (sal_Int32)fDistance );
                                }
                            }
                            else if ( rProp.Name.equals( sCalloutDrop ) )
                            {
                                sal_Int16 nDrop;
                                if ( rProp.Value >>= nDrop )
                                    AddOpt( DFF_Prop_spcod, (sal_uInt16)nDrop );
                            }
                            else if ( rProp.Name.equals( sCalloutDropAuto ) )
                            {
                                sal_Bool bDropAuto;
                                if ( rProp.Value >>= bDropAuto )
                                {
                                    nCalloutFlags |= 0x20000;
                                    if ( bDropAuto )
                                        nCalloutFlags |= 2;
                                    else
                                        nCalloutFlags &=~2;
                                }
                            }
                            else if ( rProp.Name.equals( sCalloutGap ) )
                            {
                                double fGap;
                                if ( rProp.Value >>= fGap )
                                {
                                    fGap *= 360.0;
                                    AddOpt( DFF_Prop_dxyCalloutGap, (sal_Int32)fGap );
                                }

                            }
                            else if ( rProp.Name.equals( sCalloutLength ) )
                            {
                                double fLength;
                                if ( rProp.Value >>= fLength )
                                {
                                    fLength *= 360.0;
                                    AddOpt( DFF_Prop_dxyCalloutLengthSpecified, (sal_Int32)fLength );
                                }
                            }
                            else if ( rProp.Name.equals( sCalloutLengthSpecified ) )
                            {
                                sal_Bool bLengthSpecified;
                                if ( rProp.Value >>= bLengthSpecified )
                                {
                                    nCalloutFlags |= 0x10000;
                                    if ( bLengthSpecified )
                                        nCalloutFlags |= 1;
                                    else
                                        nCalloutFlags &=~1;
                                }
                            }
                            else if ( rProp.Name.equals( sCalloutFlipX ) )
                            {
                                sal_Bool bFlipX;
                                if ( rProp.Value >>= bFlipX )
                                {
                                    nCalloutFlags |= 0x80000;
                                    if ( bFlipX )
                                        nCalloutFlags |= 8;
                                    else
                                        nCalloutFlags &=~8;
                                }
                            }
                            else if ( rProp.Name.equals( sCalloutFlipY ) )
                            {
                                sal_Bool bFlipY;
                                if ( rProp.Value >>= bFlipY )
                                {
                                    nCalloutFlags |= 0x40000;
                                    if ( bFlipY )
                                        nCalloutFlags |= 4;
                                    else
                                        nCalloutFlags &=~4;
                                }
                            }
                            else if ( rProp.Name.equals( sCalloutTextBorder ) )
                            {
                                sal_Bool bTextBorder;
                                if ( rProp.Value >>= bTextBorder )
                                {
                                    nCalloutFlags |= 0x100000;
                                    if ( bTextBorder )
                                        nCalloutFlags |= 16;
                                    else
                                        nCalloutFlags &=~16;
                                }
                            }
                            else if ( rProp.Name.equals( sCalloutType ) )
                            {
                                sal_Int16 nType;
                                if ( rProp.Value >>= nType )
                                    AddOpt( DFF_Prop_spcot, (sal_uInt16)nType );
                            }
                        }
                        if ( nCalloutFlags != nCalloutFlagsOrg )
                            AddOpt( DFF_Prop_fCalloutLengthSpecified, nCalloutFlags );
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
                        nFillHarshFlagsOrg = nFillHarshFlags = 0x000016;
                        if ( GetOpt( DFF_Prop_fc3DLightFace, nLightFaceFlags ) )
                            nLightFaceFlagsOrg = nLightFaceFlags;
                        if ( GetOpt( DFF_Prop_fc3DFillHarsh, nFillHarshFlags ) )
                            nFillHarshFlagsOrg = nFillHarshFlags;

                        sal_Int32 i, nCount = aExtrusionPropSeq.getLength();
                        for ( i = 0; i < nCount; i++ )
                        {
                            const beans::PropertyValue& rProp = aExtrusionPropSeq[ i ];
                            const rtl::OUString sExtrusionOn                ( RTL_CONSTASCII_USTRINGPARAM( "On" ) );
                            const rtl::OUString sExtrusionAutoRotationCenter( RTL_CONSTASCII_USTRINGPARAM( "AutoRotationCenter" ) );
                            const rtl::OUString sExtrusionBackwardDepth     ( RTL_CONSTASCII_USTRINGPARAM( "BackwardDepth" ) );
                            const rtl::OUString sExtrusionBrightness        ( RTL_CONSTASCII_USTRINGPARAM( "Brightness" ) );
                            const rtl::OUString sExtrusionDiffusity         ( RTL_CONSTASCII_USTRINGPARAM( "Diffusity" ) );
                            const rtl::OUString sExtrusionEdge              ( RTL_CONSTASCII_USTRINGPARAM( "Edge" ) );
                            const rtl::OUString sExtrusionFacet             ( RTL_CONSTASCII_USTRINGPARAM( "Facet" ) );
                            const rtl::OUString sExtrusionForewardDepth     ( RTL_CONSTASCII_USTRINGPARAM( "ForewardDepth" ) );
                            const rtl::OUString sExtrusionLightFace         ( RTL_CONSTASCII_USTRINGPARAM( "LightFace" ) );
                            const rtl::OUString sExtrusionLightHarsh1       ( RTL_CONSTASCII_USTRINGPARAM( "LightHarsh1" ) );
                            const rtl::OUString sExtrusionLightHarsh2       ( RTL_CONSTASCII_USTRINGPARAM( "LightHarsh2" ) );
                            const rtl::OUString sExtrusionLightLevel1       ( RTL_CONSTASCII_USTRINGPARAM( "LightLevel1" ) );
                            const rtl::OUString sExtrusionLightLevel2       ( RTL_CONSTASCII_USTRINGPARAM( "LightLevel2" ) );
                            const rtl::OUString sExtrusionLightDirection1   ( RTL_CONSTASCII_USTRINGPARAM( "LightDirection1" ) );
                            const rtl::OUString sExtrusionLightDirection2   ( RTL_CONSTASCII_USTRINGPARAM( "LightDirection2" ) );
                            const rtl::OUString sExtrusionMetal             ( RTL_CONSTASCII_USTRINGPARAM( "Metal" ) );
                            const rtl::OUString sExtrusionPlane             ( RTL_CONSTASCII_USTRINGPARAM( "Plane" ) );
                            const rtl::OUString sExtrusionRenderMode        ( RTL_CONSTASCII_USTRINGPARAM( "RenderMode" ) );
                            const rtl::OUString sExtrusionAngleX            ( RTL_CONSTASCII_USTRINGPARAM( "AngleX" ) );
                            const rtl::OUString sExtrusionAngleY            ( RTL_CONSTASCII_USTRINGPARAM( "AngleY" ) );
                            const rtl::OUString sExtrusionRotationCenterX   ( RTL_CONSTASCII_USTRINGPARAM( "RotationCenterX" ) );
                            const rtl::OUString sExtrusionRotationCenterY   ( RTL_CONSTASCII_USTRINGPARAM( "RotationCenterY" ) );
                            const rtl::OUString sExtrusionRotationCenterZ   ( RTL_CONSTASCII_USTRINGPARAM( "RotationCenterZ" ) );
                            const rtl::OUString sExtrusionShininess         ( RTL_CONSTASCII_USTRINGPARAM( "Shininess" ) );
                            const rtl::OUString sExtrusionSkew              ( RTL_CONSTASCII_USTRINGPARAM( "Skew" ) );
                            const rtl::OUString sExtrusionSkewAngle         ( RTL_CONSTASCII_USTRINGPARAM( "SkewAngle" ) );
                            const rtl::OUString sExtrusionSpecularity       ( RTL_CONSTASCII_USTRINGPARAM( "Specularity" ) );
                            const rtl::OUString sExtrusionParallel          ( RTL_CONSTASCII_USTRINGPARAM( "Parallel" ) );
                            const rtl::OUString sExtrusionViewPoint         ( RTL_CONSTASCII_USTRINGPARAM( "ViewPoint" ) );
                            const rtl::OUString sExtrusionOriginX           ( RTL_CONSTASCII_USTRINGPARAM( "OriginX" ) );
                            const rtl::OUString sExtrusionOriginY           ( RTL_CONSTASCII_USTRINGPARAM( "OriginY" ) );
                            const rtl::OUString sExtrusionColor             ( RTL_CONSTASCII_USTRINGPARAM( "Color" ) );

                            if ( rProp.Name.equals( sExtrusionOn ) )
                            {
                                sal_Bool bExtrusionOn;
                                if ( rProp.Value >>= bExtrusionOn )
                                {
                                    nLightFaceFlags |= 0x80000;
                                    if ( bExtrusionOn )
                                        nLightFaceFlags |= 8;
                                    else
                                        nLightFaceFlags &=~8;
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionAutoRotationCenter ) )
                            {
                                sal_Bool bExtusionAutoRotationCenter;
                                if ( rProp.Value >>= bExtusionAutoRotationCenter )
                                {
                                    nFillHarshFlags |= 0x80000;
                                    if ( bExtusionAutoRotationCenter )
                                        nFillHarshFlags |= 8;
                                    else
                                        nFillHarshFlags &=~8;
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionBackwardDepth ) )
                            {
                                double fExtrusionBackwardDepth;
                                if ( rProp.Value >>= fExtrusionBackwardDepth )
                                {
                                    fExtrusionBackwardDepth *= 360.0;
                                    AddOpt( DFF_Prop_c3DExtrudeBackward, (sal_Int32)fExtrusionBackwardDepth );
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionBrightness ) )
                            {
                                double fExtrusionBrightness;
                                if ( rProp.Value >>= fExtrusionBrightness )
                                    AddOpt( DFF_Prop_c3DAmbientIntensity, (sal_Int32)( fExtrusionBrightness * 655.36 ) );
                            }
                            else if ( rProp.Name.equals( sExtrusionDiffusity ) )
                            {
                                double fExtrusionDiffusity;
                                if ( rProp.Value >>= fExtrusionDiffusity )
                                    AddOpt( DFF_Prop_c3DDiffuseAmt, (sal_Int32)( fExtrusionDiffusity * 655.36 ) );
                            }
                            else if ( rProp.Name.equals( sExtrusionEdge ) )
                            {
                                sal_Int32 nExtrusionEdge;
                                if ( rProp.Value >>= nExtrusionEdge )
                                    AddOpt( DFF_Prop_c3DEdgeThickness, nExtrusionEdge );
                            }
                            else if ( rProp.Name.equals( sExtrusionFacet ) )
                            {
                                sal_Int32 nExtrusionFacet;
                                if ( rProp.Value >>= nExtrusionFacet )
                                    AddOpt( DFF_Prop_c3DTolerance, nExtrusionFacet );
                            }
                            else if ( rProp.Name.equals( sExtrusionForewardDepth ) )
                            {
                                double fExtrusionForewardDepth;
                                if ( rProp.Value >>= fExtrusionForewardDepth )
                                {
                                    fExtrusionForewardDepth *= 360.0;
                                    AddOpt( DFF_Prop_c3DExtrudeForward, (sal_Int32)fExtrusionForewardDepth );
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionLightFace ) )
                            {
                                sal_Bool bExtrusionLightFace;
                                if ( rProp.Value >>= bExtrusionLightFace )
                                {
                                    nLightFaceFlags |= 0x10000;
                                    if ( bExtrusionLightFace )
                                        nLightFaceFlags |= 1;
                                    else
                                        nLightFaceFlags &=~1;
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionLightHarsh1 ) )
                            {
                                sal_Bool bExtrusionLightHarsh1;
                                if ( rProp.Value >>= bExtrusionLightHarsh1 )
                                {
                                    nFillHarshFlags |= 0x20000;
                                    if ( bExtrusionLightHarsh1 )
                                        nFillHarshFlags |= 2;
                                    else
                                        nFillHarshFlags &=~2;
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionLightHarsh2 ) )
                            {
                                sal_Bool bExtrusionLightHarsh2;
                                if ( rProp.Value >>= bExtrusionLightHarsh2 )
                                {
                                    nFillHarshFlags |= 0x10000;
                                    if ( bExtrusionLightHarsh2 )
                                        nFillHarshFlags |= 1;
                                    else
                                        nFillHarshFlags &=~1;
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionLightLevel1 ) )
                            {
                                double fExtrusionLightLevel1;
                                if ( rProp.Value >>= fExtrusionLightLevel1 )
                                    AddOpt( DFF_Prop_c3DKeyIntensity, (sal_Int32)( fExtrusionLightLevel1 * 655.36 ) );
                            }
                            else if ( rProp.Name.equals( sExtrusionLightLevel2 ) )
                            {
                                double fExtrusionLightLevel2;
                                if ( rProp.Value >>= fExtrusionLightLevel2 )
                                    AddOpt( DFF_Prop_c3DFillIntensity, (sal_Int32)( fExtrusionLightLevel2 * 655.36 ) );
                            }
                            else if ( rProp.Name.equals( sExtrusionLightDirection1 ) )
                            {
                                drawing::Direction3D aExtrusionLightDirection1;
                                if ( rProp.Value >>= aExtrusionLightDirection1 )
                                {
                                    AddOpt( DFF_Prop_c3DKeyX, (sal_Int32)aExtrusionLightDirection1.DirectionX  );
                                    AddOpt( DFF_Prop_c3DKeyY, (sal_Int32)aExtrusionLightDirection1.DirectionY  );
                                    AddOpt( DFF_Prop_c3DKeyZ, (sal_Int32)aExtrusionLightDirection1.DirectionZ  );
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionLightDirection2 ) )
                            {
                                drawing::Direction3D aExtrusionLightPosition2;
                                if ( rProp.Value >>= aExtrusionLightPosition2 )
                                {
                                    AddOpt( DFF_Prop_c3DFillX, (sal_Int32)aExtrusionLightPosition2.DirectionX  );
                                    AddOpt( DFF_Prop_c3DFillY, (sal_Int32)aExtrusionLightPosition2.DirectionY  );
                                    AddOpt( DFF_Prop_c3DFillZ, (sal_Int32)aExtrusionLightPosition2.DirectionZ  );
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionMetal ) )
                            {
                                sal_Bool bExtrusionMetal;
                                if ( rProp.Value >>= bExtrusionMetal )
                                {
                                    nLightFaceFlags |= 0x40000;
                                    if ( bExtrusionMetal )
                                        nLightFaceFlags |= 4;
                                    else
                                        nLightFaceFlags &=~4;
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionPlane ) )
                            {
                                sal_Int16 nExtrusionPlane;
                                if ( rProp.Value >>= nExtrusionPlane )
                                    AddOpt( DFF_Prop_c3DExtrudePlane, (sal_uInt16)nExtrusionPlane );
                            }
                            else if ( rProp.Name.equals( sExtrusionRenderMode ) )
                            {
                                sal_Int16 nExtrusionRenderMode;
                                if ( rProp.Value >>= nExtrusionRenderMode )
                                    AddOpt( DFF_Prop_c3DRenderMode, (sal_uInt16)nExtrusionRenderMode );
                            }
                            else if ( rProp.Name.equals( sExtrusionAngleX ) )
                            {
                                double fExtrusionAngleX;
                                if ( rProp.Value >>= fExtrusionAngleX )
                                {
                                    fExtrusionAngleX *= 65536;
                                    AddOpt( DFF_Prop_c3DXRotationAngle, (sal_Int32)fExtrusionAngleX );
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionAngleY ) )
                            {
                                double fExtrusionAngleY;
                                if ( rProp.Value >>= fExtrusionAngleY )
                                {
                                    fExtrusionAngleY *= 65536;
                                    AddOpt( DFF_Prop_c3DYRotationAngle, (sal_Int32)fExtrusionAngleY );
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionRotationCenterX ) )
                            {
                                double fExtrusionRotationCenterX;
                                if ( rProp.Value >>= fExtrusionRotationCenterX )
                                {
                                    fExtrusionRotationCenterX *= 360.0;
                                    AddOpt( DFF_Prop_c3DRotationCenterX, (sal_Int32)fExtrusionRotationCenterX );
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionRotationCenterY ) )
                            {
                                double fExtrusionRotationCenterY;
                                if ( rProp.Value >>= fExtrusionRotationCenterY )
                                {
                                    fExtrusionRotationCenterY *= 360.0;
                                    AddOpt( DFF_Prop_c3DRotationCenterY, (sal_Int32)fExtrusionRotationCenterY );
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionRotationCenterZ ) )
                            {
                                double fExtrusionRotationCenterZ;
                                if ( rProp.Value >>= fExtrusionRotationCenterZ )
                                {
                                    fExtrusionRotationCenterZ *= 360.0;
                                    AddOpt( DFF_Prop_c3DRotationCenterZ, (sal_Int32)fExtrusionRotationCenterZ );
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionShininess ) )
                            {
                                double fExtrusionShininess;
                                if ( rProp.Value >>= fExtrusionShininess )
                                    AddOpt( DFF_Prop_c3DShininess, (sal_Int32)( fExtrusionShininess * 655.36 ) );
                            }
                            else if ( rProp.Name.equals( sExtrusionSkew ) )
                            {
                                double fExtrusionSkew;
                                if ( rProp.Value >>= fExtrusionSkew )
                                    AddOpt( DFF_Prop_c3DSkewAmount, (sal_Int32)fExtrusionSkew );
                            }
                            else if ( rProp.Name.equals( sExtrusionSkewAngle ) )
                            {
                                double fExtrusionSkewAngle;
                                if ( rProp.Value >>= fExtrusionSkewAngle )
                                {
                                    fExtrusionSkewAngle *= 65536;
                                    AddOpt( DFF_Prop_c3DSkewAngle, (sal_Int32)fExtrusionSkewAngle );
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionSpecularity ) )
                            {
                                double fExtrusionSpecularity;
                                if ( rProp.Value >>= fExtrusionSpecularity )
                                    AddOpt( DFF_Prop_c3DSpecularAmt, (sal_Int32)( fExtrusionSpecularity * 1333 ) );
                            }
                            else if ( rProp.Name.equals( sExtrusionParallel ) )
                            {
                                sal_Bool bExtrusionParallel;
                                if ( rProp.Value >>= bExtrusionParallel )
                                {
                                    nFillHarshFlags |= 0x40000;
                                    if ( bExtrusionParallel )
                                        nFillHarshFlags |= 4;
                                    else
                                        nFillHarshFlags &=~4;
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionViewPoint ) )
                            {
                                drawing::Position3D aExtrusionViewPoint;
                                if ( rProp.Value >>= aExtrusionViewPoint )
                                {
                                    aExtrusionViewPoint.PositionX *= 360.0;
                                    aExtrusionViewPoint.PositionY *= 360.0;
                                    aExtrusionViewPoint.PositionZ *= 360.0;
                                    AddOpt( DFF_Prop_c3DXViewpoint, (sal_Int32)aExtrusionViewPoint.PositionX  );
                                    AddOpt( DFF_Prop_c3DYViewpoint, (sal_Int32)aExtrusionViewPoint.PositionY  );
                                    AddOpt( DFF_Prop_c3DZViewpoint, (sal_Int32)aExtrusionViewPoint.PositionZ  );
                                }
                            }
                            else if ( rProp.Name.equals( sExtrusionOriginX ) )
                            {
                                double fExtrusionOriginX;
                                if ( rProp.Value >>= fExtrusionOriginX )
                                    AddOpt( DFF_Prop_c3DOriginX, (sal_Int32)( fExtrusionOriginX * 655.36 ) );
                            }
                            else if ( rProp.Name.equals( sExtrusionOriginY ) )
                            {
                                double fExtrusionOriginY;
                                if ( rProp.Value >>= fExtrusionOriginY )
                                    AddOpt( DFF_Prop_c3DOriginY, (sal_Int32)( fExtrusionOriginY * 655.36 ) );
                            }
                            else if ( rProp.Name.equals( sExtrusionColor ) )
                            {
                                sal_Bool bExtrusionColor;
                                if ( rProp.Value >>= bExtrusionColor )
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
                    uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeEquation > aEquations;
                    if ( rProp.Value >>= aEquations )
                    {
                        sal_uInt16 nElements = (sal_uInt16)aEquations.getLength();
                        if ( nElements )
                        {
                            sal_uInt16 nElementSize = 8;
                            sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                            SvMemoryStream aOut( nStreamSize );
                            aOut << nElements
                                 << nElements
                                 << nElementSize;

                            sal_Int32 k, i;
                            for ( i = 0; i < nElements; i++ )
                            {
                                sal_uInt16 nFlags = aEquations[ i ].Operation;
                                sal_Int16 nVal[ 3 ];
                                for ( k = 0; k < 3; k++ )
                                {
                                    nVal[ k ] = 0;
                                    if ( k < aEquations[ i ].Parameters.getLength() )
                                    {
                                        const drafts::com::sun::star::drawing::EnhancedCustomShapeParameter&
                                            rParameter = aEquations[ i ].Parameters[ k ];

                                        sal_Int32 nValue = 0;
                                        if ( rParameter.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
                                        {
                                            double fValue;
                                            if ( rParameter.Value >>= fValue )
                                                nValue = (sal_Int32)fValue;
                                        }
                                        else
                                            rParameter.Value >>= nValue;

                                        switch( rParameter.Type )
                                        {
                                            case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::EQUATION :
                                            {
                                                nValue |= 0x400;
                                                nFlags |= ( 0x2000 << k );
                                            }
                                            break;

                                            case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::ADJUSTMENT :
                                            {
                                                nValue += DFF_Prop_adjustValue;
                                                nFlags |= ( 0x2000 << k );
                                            }
                                            break;

                                            // PASSTHROUGH !!!
                                            case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::BOTTOM :
                                                nValue++;
                                            case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::RIGHT :
                                                nValue++;
                                            case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::TOP :
                                                nValue++;
                                            case drafts::com::sun::star::drawing::EnhancedCustomShapeParameterType::LEFT :
                                            {
                                                nValue += DFF_Prop_geoLeft;
                                                nFlags |= ( 0x2000 << k );
                                            }
                                            break;
                                        }
                                        nVal[ k ] = (sal_Int16)nValue;
                                    }
                                }
                                aOut << nFlags
                                     << nVal[ 0 ]
                                     << nVal[ 1 ]
                                     << nVal[ 2 ];
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
                        if ( GetOpt( DFF_Prop_fc3DLightFace, nPathFlags ) )
                            nPathFlagsOrg = nPathFlags;

                        sal_Int32 i, nCount = aPathPropSeq.getLength();
                        for ( i = 0; i < nCount; i++ )
                        {
                            const beans::PropertyValue& rProp = aPathPropSeq[ i ];
                            const rtl::OUString sPathExtrusionAllowed               ( RTL_CONSTASCII_USTRINGPARAM( "ExtrusionAllowed" ) );
                            const rtl::OUString sPathConcentricGradientFillAllowed  ( RTL_CONSTASCII_USTRINGPARAM( "ConcentricGradientFillAllowed" ) );
                            const rtl::OUString sPathTextPathAllowed                ( RTL_CONSTASCII_USTRINGPARAM( "TextPathAllowed" ) );
                            const rtl::OUString sPathCoordinates                    ( RTL_CONSTASCII_USTRINGPARAM( "Coordinates" ) );
                            const rtl::OUString sPathGluePoints                     ( RTL_CONSTASCII_USTRINGPARAM( "GluePoints" ) );
                            const rtl::OUString sPathGluePointType                  ( RTL_CONSTASCII_USTRINGPARAM( "GluePointType" ) );
                            const rtl::OUString sPathSegments                       ( RTL_CONSTASCII_USTRINGPARAM( "Segments" ) );
                            const rtl::OUString sPathStretchPoint                   ( RTL_CONSTASCII_USTRINGPARAM( "StretchPoint" ) );
                            const rtl::OUString sPathTextFrames                     ( RTL_CONSTASCII_USTRINGPARAM( "TextFrames" ) );

                            if ( rProp.Name.equals( sPathExtrusionAllowed ) )
                            {
                                sal_Bool bExtrusionAllowed;
                                if ( rProp.Value >>= bExtrusionAllowed )
                                {
                                    nPathFlags |= 0x100000;
                                    if ( bExtrusionAllowed )
                                        nPathFlags |= 16;
                                    else
                                        nPathFlags &=~16;
                                }
                            }
                            else if ( rProp.Name.equals( sPathConcentricGradientFillAllowed ) )
                            {
                                sal_Bool bConcentricGradientFillAllowed;
                                if ( rProp.Value >>= bConcentricGradientFillAllowed )
                                {
                                    nPathFlags |= 0x20000;
                                    if ( bConcentricGradientFillAllowed )
                                        nPathFlags |= 2;
                                    else
                                        nPathFlags &=~2;
                                }
                            }
                            else if ( rProp.Name.equals( sPathTextPathAllowed ) )
                            {
                                sal_Bool bTextPathAllowed;
                                if ( rProp.Value >>= bTextPathAllowed )
                                {
                                    nPathFlags |= 0x40000;
                                    if ( bTextPathAllowed )
                                        nPathFlags |= 4;
                                    else
                                        nPathFlags &=~4;
                                }
                            }
                            else if ( rProp.Name.equals( sPathCoordinates ) )
                            {
                                com::sun::star::uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair > aCoordinates;
                                if ( rProp.Value >>= aCoordinates )
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
                                            sal_Int32 X = GetValueForEnhancedCustomShapeParameter( aCoordinates[ j ].First );
                                            sal_Int32 Y = GetValueForEnhancedCustomShapeParameter( aCoordinates[ j ].Second );
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
                            else if ( rProp.Name.equals( sPathGluePoints ) )
                            {
                                com::sun::star::uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair> aGluePoints;
                                if ( rProp.Value >>= aGluePoints )
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
                                            sal_Int32 X = GetValueForEnhancedCustomShapeParameter( aGluePoints[ j ].First );
                                            sal_Int32 Y = GetValueForEnhancedCustomShapeParameter( aGluePoints[ j ].Second );
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
                            else if ( rProp.Name.equals( sPathGluePointType ) )
                            {
                                sal_Int16 nGluePointType;
                                if ( rProp.Value >>= nGluePointType )
                                    AddOpt( DFF_Prop_connectorType, (sal_uInt16)nGluePointType );
                            }
                            else if ( rProp.Name.equals( sPathSegments ) )
                            {
                                com::sun::star::uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeSegment > aSegments;
                                if ( rProp.Value >>= aSegments )
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
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::UNKNOWN :
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::LINETO : break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::MOVETO :
                                                {
                                                    nVal = 0x4000;
                                                }
                                                break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CURVETO :
                                                {
                                                    nVal |= 0x2000;
                                                }
                                                break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CLOSESUBPATH :
                                                {
                                                    nVal |= 0x6000;
                                                }
                                                break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ENDSUBPATH :
                                                {
                                                    nVal = 0x8000;
                                                }
                                                break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::NOFILL :
                                                {
                                                    nVal = 0xaa00;
                                                }
                                                break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::NOSTROKE :
                                                {
                                                    nVal = 0xab00;
                                                }
                                                break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO :
                                                {
                                                    nVal *= 3;
                                                    nVal |= 0xa100;
                                                }
                                                break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE :
                                                {
                                                    nVal *= 3;
                                                    nVal |= 0xa200;
                                                }
                                                break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ARCTO :
                                                {
                                                    nVal <<= 2;
                                                    nVal |= 0xa300;
                                                }
                                                break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ARC :
                                                {
                                                    nVal <<= 2;
                                                    nVal |= 0xa400;
                                                }
                                                break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO :
                                                {
                                                    nVal <<= 2;
                                                    nVal |= 0xa500;
                                                }
                                                break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARC :
                                                {
                                                    nVal <<= 2;
                                                    nVal |= 0xa600;
                                                }
                                                break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX :
                                                {
                                                    nVal |= 0xa700;
                                                }
                                                break;
                                                case drafts::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY :
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
                            else if ( rProp.Name.equals( sPathStretchPoint ) )
                            {
                                awt::Point aPathStretchPoint;
                                if ( rProp.Value >>= aPathStretchPoint )
                                {
                                    AddOpt( DFF_Prop_stretchPointX, aPathStretchPoint.X );
                                    AddOpt( DFF_Prop_stretchPointY, aPathStretchPoint.Y );
                                }
                            }
                            else if ( rProp.Name.equals( sPathTextFrames ) )
                            {
                                com::sun::star::uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeTextFrame > aPathTextFrames;
                                if ( rProp.Value >>= aPathTextFrames )
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
                                            sal_Int32 nLeft = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].TopLeft.First );
                                            sal_Int32 nTop  = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].TopLeft.Second );
                                            sal_Int32 nRight = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].BottomRight.First );
                                            sal_Int32 nBottom = GetValueForEnhancedCustomShapeParameter( aPathTextFrames[ j ].BottomRight.Second );

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
                        nTextPathFlagsOrg = nTextPathFlags = 0xffff1200;        // default
                        if ( GetOpt( DFF_Prop_gtextFStrikethrough, nTextPathFlags ) )
                            nTextPathFlagsOrg = nTextPathFlags;

                        sal_Int32 i, nCount = aTextPathPropSeq.getLength();
                        for ( i = 0; i < nCount; i++ )
                        {
                            const beans::PropertyValue& rProp = aTextPathPropSeq[ i ];
                            const rtl::OUString sTextPathOn         ( RTL_CONSTASCII_USTRINGPARAM( "On" ) );
                            const rtl::OUString sTextPathFitPath    ( RTL_CONSTASCII_USTRINGPARAM( "FitPath" ) );
                            const rtl::OUString sTextPathFitShape   ( RTL_CONSTASCII_USTRINGPARAM( "FitShape" ) );
                            const rtl::OUString sTextPathScaleX     ( RTL_CONSTASCII_USTRINGPARAM( "ScaleX" ) );
                            const rtl::OUString sSameLetterHeights  ( RTL_CONSTASCII_USTRINGPARAM( "SameLetterHeights" ) );

                            if ( rProp.Name.equals( sTextPathOn ) )
                            {
                                sal_Bool bTextPathOn;
                                if ( rProp.Value >>= bTextPathOn )
                                {
                                    nTextPathFlags |= 0x40000000;
                                    if ( bTextPathOn )
                                        nTextPathFlags |= 0x4000;
                                    else
                                        nTextPathFlags &=~0x4000;
                                }
                            }
                            else if ( rProp.Name.equals( sTextPathFitPath ) )
                            {
                                sal_Bool bTextPathFitPath;
                                if ( rProp.Value >>= bTextPathFitPath )
                                {
                                    nTextPathFlags |= 0x01000000;
                                    if ( bTextPathFitPath )
                                        nTextPathFlags |= 0x100;
                                    else
                                        nTextPathFlags &=~0x100;
                                }
                            }
                            else if ( rProp.Name.equals( sTextPathFitShape ) )
                            {
                                sal_Bool bTextPathFitShape;
                                if ( rProp.Value >>= bTextPathFitShape )
                                {
                                    nTextPathFlags |= 0x04000000;
                                    if ( bTextPathFitShape )
                                        nTextPathFlags |= 0x400;
                                    else
                                        nTextPathFlags &=~0x400;
                                }
                            }
                            else if ( rProp.Name.equals( sTextPathScaleX ) )
                            {
                                sal_Bool bTextPathScaleX;
                                if ( rProp.Value >>= bTextPathScaleX )
                                {
                                    nTextPathFlags |= 0x00400000;
                                    if ( bTextPathScaleX )
                                        nTextPathFlags |= 0x40;
                                    else
                                        nTextPathFlags &=~0x40;
                                }
                            }
                            else if ( rProp.Name.equals( sSameLetterHeights ) )
                            {
                                sal_Bool bSameLetterHeights;
                                if ( rProp.Value >>= bSameLetterHeights )
                                {
                                    nTextPathFlags |= 0x00800000;
                                    if ( bSameLetterHeights )
                                        nTextPathFlags |= 0x80;
                                    else
                                        nTextPathFlags &=~0x80;
                                }
                            }
                        }
                        if ( nTextPathFlags != nTextPathFlagsOrg )
                            AddOpt( DFF_Prop_gtextFStrikethrough, nTextPathFlags );

                        if ( nTextPathFlags & 0x4000 )      // Is FontWork ?
                        {
                            // FontWork Text
                            rtl::OUString aText;
                            uno::Reference< text::XSimpleText > xText( rXShape, uno::UNO_QUERY );
                            if ( xText.is() )
                                aText = xText->getString();
                            if ( !aText.getLength() )
                                aText = ::rtl::OUString::createFromAscii( "your text" );    // todo: moving into a resource
                            AddOpt( DFF_Prop_gtextUNICODE, aText );

                            // FontWork Font
                            rtl::OUString aFontName;
                            const rtl::OUString sCharFontName           ( RTL_CONSTASCII_USTRINGPARAM( "CharFontName" ) );
                            uno::Any aAny = aXPropSet->getPropertyValue( sCharFontName );
                            aAny >>= aFontName;
                            if ( !aFontName.getLength() )
                                aFontName = ::rtl::OUString::createFromAscii( "Arial Black" );
                            AddOpt( DFF_Prop_gtextFont, aFontName );
                        }
                    }
                }
                else if ( rProp.Name.equals( sHandles ) )
                {
                    bPredefinedHandlesUsed = sal_False;
                    if ( rProp.Value >>= aHandlesPropSeq )
                    {
                        sal_uInt16 nElements = (sal_uInt16)aHandlesPropSeq.getLength();
                        if ( nElements )
                        {
                            const rtl::OUString sHandle ( RTL_CONSTASCII_USTRINGPARAM( "Handle" ) );

                            sal_uInt16 i, j, nElementSize = 36;
                            sal_uInt32 nStreamSize = nElementSize * nElements + 6;
                            SvMemoryStream aOut( nStreamSize );
                            aOut << nElements
                                 << nElements
                                 << nElementSize;

                            for ( i = 0; i < nElements; i++ )
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

                                const uno::Sequence< beans::PropertyValue >& rPropSeq = aHandlesPropSeq[ i ];
                                for ( j = 0; j < rPropSeq.getLength(); j++ )
                                {
                                    const beans::PropertyValue& rPropVal = rPropSeq[ j ];

                                    const rtl::OUString sPosition           ( RTL_CONSTASCII_USTRINGPARAM( "Position" ) );
                                    const rtl::OUString sMirroredX          ( RTL_CONSTASCII_USTRINGPARAM( "MirroredX" ) );
                                    const rtl::OUString sMirroredY          ( RTL_CONSTASCII_USTRINGPARAM( "MirroredY" ) );
                                    const rtl::OUString sSwitched           ( RTL_CONSTASCII_USTRINGPARAM( "Switched" ) );
                                    const rtl::OUString sPolar              ( RTL_CONSTASCII_USTRINGPARAM( "Polar" ) );
//                                  const rtl::OUString sMap                ( RTL_CONSTASCII_USTRINGPARAM( "Map" ) );
                                    const rtl::OUString sRadiusRangeMinimum ( RTL_CONSTASCII_USTRINGPARAM( "RadiusRangeMinimum" ) );
                                    const rtl::OUString sRadiusRangeMaximum ( RTL_CONSTASCII_USTRINGPARAM( "RadiusRangeMaximum" ) );
                                    const rtl::OUString sRangeXMinimum      ( RTL_CONSTASCII_USTRINGPARAM( "RangeXMinimum" ) );
                                    const rtl::OUString sRangeXMaximum      ( RTL_CONSTASCII_USTRINGPARAM( "RangeXMaximum" ) );
                                    const rtl::OUString sRangeYMinimum      ( RTL_CONSTASCII_USTRINGPARAM( "RangeYMinimum" ) );
                                    const rtl::OUString sRangeYMaximum      ( RTL_CONSTASCII_USTRINGPARAM( "RangeYMaximum" ) );

                                    if ( rPropVal.Name.equals( sPosition ) )
                                    {
                                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair aPosition;
                                        if ( rPropVal.Value >>= aPosition )
                                        {
                                            GetValueForEnhancedCustomShapeHandleParameter( nXPosition, aPosition.First );
                                            GetValueForEnhancedCustomShapeHandleParameter( nYPosition, aPosition.Second );
                                        }
                                    }
                                    else if ( rPropVal.Name.equals( sMirroredX ) )
                                    {
                                        sal_Bool bMirroredX;
                                        if ( rPropVal.Value >>= bMirroredX )
                                        {
                                            if ( bMirroredX )
                                                nFlags |= 1;
                                        }
                                    }
                                    else if ( rPropVal.Name.equals( sMirroredY ) )
                                    {
                                        sal_Bool bMirroredY;
                                        if ( rPropVal.Value >>= bMirroredY )
                                        {
                                            if ( bMirroredY )
                                                nFlags |= 2;
                                        }
                                    }
                                    else if ( rPropVal.Name.equals( sSwitched ) )
                                    {
                                        sal_Bool bSwitched;
                                        if ( rPropVal.Value >>= bSwitched )
                                        {
                                            if ( bSwitched )
                                                nFlags |= 4;
                                        }
                                    }
                                    else if ( rPropVal.Name.equals( sPolar ) )
                                    {
                                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair aPolar;
                                        if ( rPropVal.Value >>= aPolar )
                                        {
                                            if ( GetValueForEnhancedCustomShapeHandleParameter( nXMap, aPolar.First ) )
                                                nFlags |= 0x800;
                                            if ( GetValueForEnhancedCustomShapeHandleParameter( nYMap, aPolar.Second ) )
                                                nFlags |= 0x1000;
                                            nFlags |= 8;
                                        }
                                    }
/* seems not to be used.
                                    else if ( rPropVal.Name.equals( sMap ) )
                                    {
                                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameterPair aMap;
                                        if ( rPropVal.Value >>= aMap )
                                        {
                                            if ( GetValueForEnhancedCustomShapeHandleParameter( nXMap, aMap.First ) )
                                                nFlags |= 0x800;
                                            if ( GetValueForEnhancedCustomShapeHandleParameter( nYMap, aMap.Second ) )
                                                nFlags |= 0x1000;
                                            nFlags |= 0x10;
                                        }
                                    }
*/
                                    else if ( rPropVal.Name.equals( sRadiusRangeMinimum ) )
                                    {
                                        nYRangeMin = (sal_Int32)0xff4c0000; // the range of angles seems to be a not
                                        nYRangeMax = (sal_Int32)0x00b40000; // used feature, so we are defaulting this

                                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
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

                                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
                                        if ( rPropVal.Value >>= aRadiusRangeMaximum )
                                        {
                                            if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMax, aRadiusRangeMaximum ) )
                                                nFlags |= 0x100;
                                            nFlags |= 0x2000;
                                        }
                                    }
                                    else if ( rPropVal.Name.equals( sRangeXMinimum ) )
                                    {
                                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aXRangeMinimum;
                                        if ( rPropVal.Value >>= aXRangeMinimum )
                                        {
                                            if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMin, aXRangeMinimum ) )
                                                nFlags |= 0x80;
                                            nFlags |= 0x20;
                                        }
                                    }
                                    else if ( rPropVal.Name.equals( sRangeXMaximum ) )
                                    {
                                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aXRangeMaximum;
                                        if ( rPropVal.Value >>= aXRangeMaximum )
                                        {
                                            if ( GetValueForEnhancedCustomShapeHandleParameter( nXRangeMax, aXRangeMaximum ) )
                                                nFlags |= 0x100;
                                            nFlags |= 0x20;
                                        }
                                    }
                                    else if ( rPropVal.Name.equals( sRangeYMinimum ) )
                                    {
                                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aYRangeMinimum;
                                        if ( rPropVal.Value >>= aYRangeMinimum )
                                        {
                                            if ( GetValueForEnhancedCustomShapeHandleParameter( nYRangeMin, aYRangeMinimum ) )
                                                nFlags |= 0x200;
                                            nFlags |= 0x20;
                                        }
                                    }
                                    else if ( rPropVal.Name.equals( sRangeYMaximum ) )
                                    {
                                        drafts::com::sun::star::drawing::EnhancedCustomShapeParameter aYRangeMaximum;
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
                else if ( rProp.Name.equals( sAdjustmentValues ) )
                {
                    // it is required, that the information which handle is polar has already be read,
                    // so we are able to change the polar value to a fixed float
                    pAdjustmentValuesProp = &rProp;
                }
            }
            if ( pAdjustmentValuesProp )
            {
                uno::Sequence< drafts::com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq;
                if ( pAdjustmentValuesProp->Value >>= aAdjustmentSeq )
                {
                    if ( bPredefinedHandlesUsed )
                    {
                        const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eShapeType );
                        if ( pDefCustomShape && pDefCustomShape->nHandles && pDefCustomShape->pHandles )
                        {
                            sal_Int32 i, nCount = pDefCustomShape->nHandles;
                            const SvxMSDffHandle* pData = pDefCustomShape->pHandles;
                            for ( i = 0; i < nCount; i++, pData++ )
                            {
                                if ( pData->nFlags & MSDFF_HANDLE_FLAGS_POLAR )
                                {
                                    if ( ( pData->nPositionY >= 0x256 ) || ( pData->nPositionY <= 0x107 ) )
                                        nAdjustmentsWhichNeedsToBeConverted |= ( 1 << i );
                                }
                            }
                        }
                    }
                    sal_Int32 i, nValue, nAdjustmentValues = aAdjustmentSeq.getLength();
                    for ( i = 0; i < nAdjustmentValues; i++ )
                    {
                        const drafts::com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue & rProp = aAdjustmentSeq[ i ];
                        if ( rProp.State == beans::PropertyState_DIRECT_VALUE )
                        {
                            sal_Bool bUseFixedFloat = ( nAdjustmentsWhichNeedsToBeConverted & ( 1 << i ) ) != 0;
                            if ( rProp.Value.getValueTypeClass() == uno::TypeClass_DOUBLE )
                            {
                                double fValue;
                                rProp.Value >>= fValue;
                                if ( bUseFixedFloat )
                                    fValue *= 65536.0;
                                nValue = (sal_Int32)fValue;
                            }
                            else
                            {
                                rProp.Value >>= nValue;
                                if ( bUseFixedFloat )
                                    nValue <<= 16;
                            }
                            AddOpt( (sal_uInt16)( DFF_Prop_adjustValue + i ), (sal_uInt32)nValue );
                        }
                    }
                }
            }
            awt::Size aCoordinateSize( 0, 0 );
            if ( pCoordinateSizeProp )
            {
                if ( pCoordinateSizeProp->Value >>= aCoordinateSize )
                {
                    AddOpt( DFF_Prop_geoRight,  aCoordinateOrigin.X + aCoordinateSize.Width );
                    AddOpt( DFF_Prop_geoBottom, aCoordinateOrigin.Y + aCoordinateSize.Height );
                }
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

// ---------------------------------------------------------------------------------------------

MSO_SPT EscherPropertyContainer::GetCustomShapeType( const uno::Reference< drawing::XShape > & rXShape, sal_uInt32& nMirrorFlags )
{
    MSO_SPT eShapeType = mso_sptNil;
    nMirrorFlags = 0;
    uno::Reference< beans::XPropertySet > aXPropSet( rXShape, uno::UNO_QUERY );
    if ( aXPropSet.is() )
    {
        try
        {
            OUString sShapeType;
            const OUString  sCustomShapeGeometry( RTL_CONSTASCII_USTRINGPARAM ( "CustomShapeGeometry" ) );
            uno::Any aGeoPropSet = aXPropSet->getPropertyValue( sCustomShapeGeometry );
            uno::Sequence< beans::PropertyValue > aGeoPropSeq;
            if ( aGeoPropSet >>= aGeoPropSeq )
            {
                sal_Int32 i, nCount = aGeoPropSeq.getLength();
                for ( i = 0; i < nCount; i++ )
                {
                    const beans::PropertyValue& rProp = aGeoPropSeq[ i ];
                    if ( rProp.Name.equalsAscii( "PredefinedType" ) )
                    {
                        if ( rProp.Value >>= sShapeType )
                            eShapeType = EnhancedCustomShapeTypeNames::Get( sShapeType );
                    }
                    else if ( rProp.Name.equalsAscii( "MirroredX" ) )
                    {
                        sal_Bool bMirroredX;
                        if ( rProp.Value >>= bMirroredX )
                            nMirrorFlags  |= SHAPEFLAG_FLIPH;
                    }
                    else if ( rProp.Name.equalsAscii( "MirroredY" ) )
                    {
                        sal_Bool bMirroredY;
                        if ( rProp.Value >>= bMirroredY )
                            nMirrorFlags  |= SHAPEFLAG_FLIPV;
                    }
                }
            }
        }
        catch( ::com::sun::star::uno::Exception& )
        {
        }
    }
    return eShapeType;
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
            const int nBuf = 0x40000;   // 256KB buffer
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

sal_uInt32 EscherGraphicProvider::GetBlibID( SvStream& rPicOutStrm, const ByteString& rId,
                                                const Rectangle& rBoundRect, const GraphicAttr* pGraphicAttr )
{
    sal_uInt32          nBlibId = 0;
    GraphicObject       aGraphicObject( rId );

    EscherBlibEntry* p_EscherBlibEntry = new EscherBlibEntry( rPicOutStrm.Tell(), aGraphicObject, rId, pGraphicAttr );
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
                sal_uInt32 nErrCode;
                if ( !aGraphic.IsAnimated() )
                    nErrCode = GraphicConverter::Export( aStream, aGraphic, ( eGraphicType == GRAPHIC_BITMAP ) ? CVT_PNG  : CVT_WMF );
                else
                {   // to store a animation, a gif has to be included into the msOG chunk of a png  #I5583#
                    GraphicFilter*  pFilter = GetGrfFilter();
                    SvMemoryStream  aGIFStream;
                    ByteString      aVersion( "MSOFFICE9.0" );
                    aGIFStream.Write( aVersion.GetBuffer(), aVersion.Len() );
                    nErrCode = pFilter->ExportGraphic( aGraphic, String(), aGIFStream,
                        pFilter->GetExportFormatNumberForShortName( String( RTL_CONSTASCII_USTRINGPARAM( "GIF" ) ) ), sal_False, NULL );
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
                    nErrCode = pFilter->ExportGraphic( aGraphic, String(), aStream,
                        pFilter->GetExportFormatNumberForShortName( String( RTL_CONSTASCII_USTRINGPARAM( "PNG" ) ) ), sal_False,
                            &aFilterData );
                }
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
                    UINT32 nPrefWidth = p_EscherBlibEntry->maPrefSize.Width();
                    UINT32 nPrefHeight = p_EscherBlibEntry->maPrefSize.Height();
                    Size aPrefSize(lcl_SizeToEmu(p_EscherBlibEntry->maPrefSize, p_EscherBlibEntry->maPrefMapMode));
                    UINT32 nWidth = aPrefSize.Width() * 360;
                    UINT32 nHeight = aPrefSize.Height() * 360;

                    rPicOutStrm << nUncompressedSize // WMFSize without FileHeader
                    << (sal_Int32)0     // da die Originalgroesse des WMF's (ohne FileHeader)
                    << (sal_Int32)0     // nicht mehr feststellbar ist, schreiben wir 10cm / x
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
            aPoly.Rotate( aRect.TopLeft(), (sal_uInt16)( ( nAngle + 5 ) / 10 ) );
        nRule = GetClosestPoint( aPoly, aRefPoint );
    }
    if ( aType == "drawing.Ellipse" )
        nRule <<= 1;    // In PPT hat eine Ellipse 8 Möglichkeiten sich zu connecten

    return nRule;
}

EscherSolverContainer::~EscherSolverContainer()
{
    void* pP;

    for( pP = maShapeList.First(); pP; pP = maShapeList.Next() )
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
    EscherGraphicProvider   ( 0 ),
    mpOutStrm               ( &rOutStrm ),
    mnDrawings              ( nDrawings ),
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
    delete mpImplEscherExSdr;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::InsertAtCurrentPos( UINT32 nBytes, BOOL bContainer )
{
    UINT32  nSize, nType, nSource, nBufSize, nToCopy, nCurPos = mpOutStrm->Tell();
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
    delete[] pBuf;
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
    mOffsets.push_back( mpOutStrm->Tell() - 4 );
    mRecTypes.push_back( nEscherContainer );
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
    nSize = ( nPos - mOffsets.back() ) - 4;
    mpOutStrm->Seek( mOffsets.back() );
    *mpOutStrm << nSize;

    switch( mRecTypes.back() )
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
    mOffsets.pop_back();
    mRecTypes.pop_back();
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
    *mpOutStrm << (sal_Int16)rRect.Top()
               << (sal_Int16)rRect.Left()
               << (sal_Int16)( rRect.GetWidth()  + rRect.Left() )
               << (sal_Int16)( rRect.GetHeight() + rRect.Top() );
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
        aPropOpt.AddOpt( ESCHER_Prop_dxWrapDistLeft, 0 );
        aPropOpt.AddOpt( ESCHER_Prop_dxWrapDistRight, 0 );
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
