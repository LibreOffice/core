/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: grfatr.cxx,v $
 * $Revision: 1.15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/HorizontalAdjust.hpp>
#include <com/sun/star/text/DocumentStatistic.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/HoriOrientationFormat.hpp>
#include <com/sun/star/text/NotePrintMode.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientationFormat.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <goodies/grfmgr.hxx>
#include <swtypes.hxx>
#include <grfatr.hxx>
#include <swunohelper.hxx>

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _UNOMID_H
#include <unomid.h>
#endif

using namespace ::com::sun::star;

TYPEINIT1_AUTOFACTORY(SwCropGrf, SfxPoolItem)
TYPEINIT1_AUTOFACTORY(SwGammaGrf, SfxPoolItem)

/******************************************************************************
 *  Implementierung     class SwMirrorGrf
 ******************************************************************************/

SfxPoolItem* SwMirrorGrf::Clone( SfxItemPool* ) const
{
    return new SwMirrorGrf( *this );
}

sal_uInt16 SwMirrorGrf::GetValueCount() const
{
    return RES_MIRROR_GRAPH_END - RES_MIRROR_GRAPH_BEGIN;
}

int SwMirrorGrf::operator==( const SfxPoolItem& rItem) const
{
    return SfxEnumItem::operator==(rItem) &&
            ((SwMirrorGrf&)rItem).IsGrfToggle() == IsGrfToggle();
}

BOOL lcl_IsHoriOnEvenPages(int nEnum, BOOL bToggle)
{
    BOOL bEnum = nEnum == RES_MIRROR_GRAPH_VERT ||
                   nEnum == RES_MIRROR_GRAPH_BOTH;
            return bEnum != bToggle;
}
BOOL lcl_IsHoriOnOddPages(int nEnum)
{
    BOOL bEnum = nEnum == RES_MIRROR_GRAPH_VERT ||
                   nEnum == RES_MIRROR_GRAPH_BOTH;
            return bEnum;
}
BOOL SwMirrorGrf::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bRet = sal_True,
         bVal;
    // Vertikal und Horizontal sind mal getauscht worden!
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_MIRROR_HORZ_EVEN_PAGES:
            bVal = lcl_IsHoriOnEvenPages(GetValue(), IsGrfToggle());
        break;
        case MID_MIRROR_HORZ_ODD_PAGES:
            bVal = lcl_IsHoriOnOddPages(GetValue());
        break;
        case MID_MIRROR_VERT:
            bVal = GetValue() == RES_MIRROR_GRAPH_HOR ||
                   GetValue() == RES_MIRROR_GRAPH_BOTH;
            break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    rVal.setValue( &bVal, ::getBooleanCppuType() );
    return bRet;
}

BOOL SwMirrorGrf::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bRet = sal_True;
    sal_Bool bVal = *(sal_Bool*)rVal.getValue();
    // Vertikal und Horizontal sind mal getauscht worden!
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_MIRROR_HORZ_EVEN_PAGES:
        case MID_MIRROR_HORZ_ODD_PAGES:
        {
            BOOL bIsVert = GetValue() == RES_MIRROR_GRAPH_HOR ||
                                GetValue() == RES_MIRROR_GRAPH_BOTH;
            BOOL bOnOddPages = nMemberId == MID_MIRROR_HORZ_EVEN_PAGES ?
                                    lcl_IsHoriOnOddPages(GetValue()) : bVal;
            BOOL bOnEvenPages = nMemberId == MID_MIRROR_HORZ_ODD_PAGES ?
                                       lcl_IsHoriOnEvenPages(GetValue(), IsGrfToggle()) : bVal;
            MirrorGraph nEnum = bOnOddPages ?
                    bIsVert ? RES_MIRROR_GRAPH_BOTH : RES_MIRROR_GRAPH_VERT :
                        bIsVert ? RES_MIRROR_GRAPH_HOR : RES_MIRROR_GRAPH_DONT;
            BOOL bToggle = bOnOddPages != bOnEvenPages;
            SetValue(static_cast<USHORT>(nEnum));
            SetGrfToggle( bToggle );
        }
        break;
        case MID_MIRROR_VERT:
            if ( bVal )
            {
                if ( GetValue() == RES_MIRROR_GRAPH_VERT )
                    SetValue( RES_MIRROR_GRAPH_BOTH );
                else if ( GetValue() != RES_MIRROR_GRAPH_BOTH )
                    SetValue( RES_MIRROR_GRAPH_HOR );
            }
            else
            {
                if ( GetValue() == RES_MIRROR_GRAPH_BOTH )
                    SetValue( RES_MIRROR_GRAPH_VERT );
                else if ( GetValue() == RES_MIRROR_GRAPH_HOR )
                    SetValue( RES_MIRROR_GRAPH_DONT );
            }
            break;
        default:
            ASSERT( !this, "unknown MemberId" );
            bRet = sal_False;
    }
    return bRet;
}


/******************************************************************************
 *  Implementierung     class SwCropGrf
 ******************************************************************************/

SwCropGrf::SwCropGrf()
    : SvxGrfCrop( RES_GRFATR_CROPGRF )
{}

SwCropGrf::SwCropGrf(sal_Int32 nL, sal_Int32 nR, sal_Int32 nT, sal_Int32 nB )
    : SvxGrfCrop( nL, nR, nT, nB, RES_GRFATR_CROPGRF )
{}

SfxPoolItem* SwCropGrf::Clone( SfxItemPool* ) const
{
    return new SwCropGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwRotationGrf::Clone( SfxItemPool * ) const
{
    return new SwRotationGrf( GetValue(), aUnrotatedSize );
}


int SwRotationGrf::operator==( const SfxPoolItem& rCmp ) const
{
    return SfxUInt16Item::operator==( rCmp ) &&
        GetUnrotatedSize() == ((SwRotationGrf&)rCmp).GetUnrotatedSize();
}


BOOL SwRotationGrf::QueryValue( uno::Any& rVal, BYTE ) const
{
    // SfxUInt16Item::QueryValue returns sal_Int32 in Any now... (srx642w)
    // where we still want this to be a sal_Int16
    rVal <<= (sal_Int16)GetValue();
    return TRUE;
}

BOOL SwRotationGrf::PutValue( const uno::Any& rVal, BYTE )
{
    // SfxUInt16Item::QueryValue returns sal_Int32 in Any now... (srx642w)
    // where we still want this to be a sal_Int16
    sal_Int16 nValue = 0;
    if (rVal >>= nValue)
    {
        // UINT16 argument needed
        SetValue( (UINT16) nValue );
        return TRUE;
    }

    DBG_ERROR( "SwRotationGrf::PutValue - Wrong type!" );
    return FALSE;
}

// ------------------------------------------------------------------

SfxPoolItem* SwLuminanceGrf::Clone( SfxItemPool * ) const
{
    return new SwLuminanceGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwContrastGrf::Clone( SfxItemPool * ) const
{
    return new SwContrastGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwChannelRGrf::Clone( SfxItemPool * ) const
{
    return new SwChannelRGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwChannelGGrf::Clone( SfxItemPool * ) const
{
    return new SwChannelGGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwChannelBGrf::Clone( SfxItemPool * ) const
{
    return new SwChannelBGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwGammaGrf::Clone( SfxItemPool * ) const
{
    return new SwGammaGrf( *this );
}

int SwGammaGrf::operator==( const SfxPoolItem& rCmp ) const
{
    return SfxPoolItem::operator==( rCmp ) &&
        nValue == ((SwGammaGrf&)rCmp).GetValue();
}

BOOL SwGammaGrf::QueryValue( uno::Any& rVal, BYTE ) const
{
    rVal <<= nValue;
    return sal_True;
}

BOOL SwGammaGrf::PutValue( const uno::Any& rVal, BYTE )
{
    return rVal >>= nValue;
}

// ------------------------------------------------------------------

SfxPoolItem* SwInvertGrf::Clone( SfxItemPool * ) const
{
    return new SwInvertGrf( *this );
}

// ------------------------------------------------------------------

SfxPoolItem* SwTransparencyGrf::Clone( SfxItemPool * ) const
{
    return new SwTransparencyGrf( *this );
}
// ------------------------------------------------------------------
BOOL SwTransparencyGrf::QueryValue( uno::Any& rVal,
                                        BYTE ) const
{
    DBG_ASSERT(ISA(SfxByteItem),"Put/QueryValue should be removed!");
    sal_Int16 nRet = GetValue();
    DBG_ASSERT( 0 <= nRet && nRet <= 100, "value out of range" );
    rVal <<= nRet;
    return TRUE;
}
// ------------------------------------------------------------------
BOOL SwTransparencyGrf::PutValue( const uno::Any& rVal,
                                        BYTE )
{
    //temporary conversion until this is a SfxInt16Item!
    DBG_ASSERT(ISA(SfxByteItem),"Put/QueryValue should be removed!");
    sal_Int16 nVal = 0;
    if(!(rVal >>= nVal) || nVal < -100 || nVal > 100)
        return FALSE;
    if(nVal < 0)
    {
        // for compatibility with old documents
        // OD 05.11.2002 #104308# - introduce rounding as for SO 6.0 PP2
        // introduced by fix of #104293#.
        nVal = ( ( nVal * 128 ) - (99/2) ) / 100;
        nVal += 128;
    }
    DBG_ASSERT( 0 <= nVal && nVal <= 100, "value out of range" );
    SetValue(static_cast<BYTE>(nVal));
    return TRUE;
}

// ------------------------------------------------------------------

SfxPoolItem* SwDrawModeGrf::Clone( SfxItemPool * ) const
{
    return new SwDrawModeGrf( *this );
}

USHORT SwDrawModeGrf::GetValueCount() const
{
    // GRAPHICDRAWMODE_STANDARD = 0,
    // GRAPHICDRAWMODE_GREYS = 1,
    // GRAPHICDRAWMODE_MONO = 2,
    // GRAPHICDRAWMODE_WATERMARK = 3
    return GRAPHICDRAWMODE_WATERMARK + 1;
}

BOOL SwDrawModeGrf::QueryValue( uno::Any& rVal,
                                BYTE ) const
{
    drawing::ColorMode eRet = (drawing::ColorMode)GetEnumValue();
    rVal <<= eRet;
    return TRUE;
}

BOOL SwDrawModeGrf::PutValue( const uno::Any& rVal,
                                BYTE )
{
    sal_Int32 eVal = SWUnoHelper::GetEnumAsInt32( rVal );
    if(eVal >= 0 && eVal <= GRAPHICDRAWMODE_WATERMARK)
    {
        SetEnumValue((USHORT)eVal);
        return TRUE;
    }
    return FALSE;
}



