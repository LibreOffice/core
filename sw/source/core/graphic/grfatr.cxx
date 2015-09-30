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
#include <svtools/grfmgr.hxx>
#include <swtypes.hxx>
#include <grfatr.hxx>
#include <swunohelper.hxx>

#include <cmdid.h>
#include <unomid.h>

using namespace ::com::sun::star;

TYPEINIT1_AUTOFACTORY(SwCropGrf, SfxPoolItem)
TYPEINIT1_AUTOFACTORY(SwGammaGrf, SfxPoolItem)

SfxPoolItem* SwMirrorGrf::Clone( SfxItemPool* ) const
{
    return new SwMirrorGrf( *this );
}

sal_uInt16 SwMirrorGrf::GetValueCount() const
{
    return RES_MIRROR_GRAPH_END - RES_MIRROR_GRAPH_BEGIN;
}

bool SwMirrorGrf::operator==( const SfxPoolItem& rItem) const
{
    return SfxEnumItem::operator==(rItem) &&
            static_cast<const SwMirrorGrf&>(rItem).IsGrfToggle() == IsGrfToggle();
}

static bool lcl_IsHoriOnEvenPages(int nEnum, bool bToggle)
{
    bool bEnum = nEnum == RES_MIRROR_GRAPH_VERT ||
                   nEnum == RES_MIRROR_GRAPH_BOTH;
            return bEnum != bToggle;
}

static bool lcl_IsHoriOnOddPages(int nEnum)
{
    bool bEnum = nEnum == RES_MIRROR_GRAPH_VERT ||
                   nEnum == RES_MIRROR_GRAPH_BOTH;
            return bEnum;
}

bool SwMirrorGrf::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bRet = true;
    bool bVal = false;
    // vertical and horizontal were swapped at some point
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
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    rVal <<= bVal;
    return bRet;
}

bool SwMirrorGrf::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bRet = true;
    bool bVal = *static_cast<sal_Bool const *>(rVal.getValue());
    // vertical and horizontal were swapped at some point
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_MIRROR_HORZ_EVEN_PAGES:
        case MID_MIRROR_HORZ_ODD_PAGES:
        {
            bool bIsVert = GetValue() == RES_MIRROR_GRAPH_HOR ||
                                GetValue() == RES_MIRROR_GRAPH_BOTH;
            bool bOnOddPages = nMemberId == MID_MIRROR_HORZ_EVEN_PAGES ?
                                    lcl_IsHoriOnOddPages(GetValue()) : bVal;
            bool bOnEvenPages = nMemberId == MID_MIRROR_HORZ_ODD_PAGES ?
                                       lcl_IsHoriOnEvenPages(GetValue(), IsGrfToggle()) : bVal;
            MirrorGraph nEnum = bOnOddPages ?
                    bIsVert ? RES_MIRROR_GRAPH_BOTH : RES_MIRROR_GRAPH_VERT :
                        bIsVert ? RES_MIRROR_GRAPH_HOR : RES_MIRROR_GRAPH_DONT;
            bool bToggle = bOnOddPages != bOnEvenPages;
            SetValue(static_cast<sal_uInt16>(nEnum));
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
            OSL_ENSURE( false, "unknown MemberId" );
            bRet = false;
    }
    return bRet;
}

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

SfxPoolItem* SwRotationGrf::Clone( SfxItemPool * ) const
{
    return new SwRotationGrf( GetValue(), aUnrotatedSize );
}

bool SwRotationGrf::operator==( const SfxPoolItem& rCmp ) const
{
    return SfxUInt16Item::operator==( rCmp ) &&
        GetUnrotatedSize() == static_cast<const SwRotationGrf&>(rCmp).GetUnrotatedSize();
}

bool SwRotationGrf::QueryValue( uno::Any& rVal, sal_uInt8 ) const
{
    // SfxUInt16Item::QueryValue returns sal_Int32 in Any now... (srx642w)
    // where we still want this to be a sal_Int16
    rVal <<= (sal_Int16)GetValue();
    return true;
}

bool SwRotationGrf::PutValue( const uno::Any& rVal, sal_uInt8 )
{
    // SfxUInt16Item::QueryValue returns sal_Int32 in Any now... (srx642w)
    // where we still want this to be a sal_Int16
    sal_Int16 nValue = 0;
    if (rVal >>= nValue)
    {
        // sal_uInt16 argument needed
        SetValue( (sal_uInt16) nValue );
        return true;
    }

    OSL_FAIL( "SwRotationGrf::PutValue - Wrong type!" );
    return false;
}

// Sw___Grf::Clone(..)

SfxPoolItem* SwLuminanceGrf::Clone( SfxItemPool * ) const
{
    return new SwLuminanceGrf( *this );
}

SfxPoolItem* SwContrastGrf::Clone( SfxItemPool * ) const
{
    return new SwContrastGrf( *this );
}

SfxPoolItem* SwChannelRGrf::Clone( SfxItemPool * ) const
{
    return new SwChannelRGrf( *this );
}

SfxPoolItem* SwChannelGGrf::Clone( SfxItemPool * ) const
{
    return new SwChannelGGrf( *this );
}

SfxPoolItem* SwChannelBGrf::Clone( SfxItemPool * ) const
{
    return new SwChannelBGrf( *this );
}

SfxPoolItem* SwGammaGrf::Clone( SfxItemPool * ) const
{
    return new SwGammaGrf( *this );
}

// SwGammaGrf

bool SwGammaGrf::operator==( const SfxPoolItem& rCmp ) const
{
    return SfxPoolItem::operator==( rCmp ) &&
        nValue == static_cast<const SwGammaGrf&>(rCmp).GetValue();
}

bool SwGammaGrf::QueryValue( uno::Any& rVal, sal_uInt8 ) const
{
    rVal <<= nValue;
    return true;
}

bool SwGammaGrf::PutValue( const uno::Any& rVal, sal_uInt8 )
{
    return rVal >>= nValue;
}

// Sw___Grf::Clone(..) cont'd

SfxPoolItem* SwInvertGrf::Clone( SfxItemPool * ) const
{
    return new SwInvertGrf( *this );
}

SfxPoolItem* SwTransparencyGrf::Clone( SfxItemPool * ) const
{
    return new SwTransparencyGrf( *this );
}

// SwTransparencyGrf

bool SwTransparencyGrf::QueryValue( uno::Any& rVal,
                                        sal_uInt8 ) const
{
    OSL_ENSURE(dynamic_cast<const SfxByteItem*>( this ) !=  nullptr,"Put/QueryValue should be removed!");
    sal_Int16 nRet = GetValue();
    OSL_ENSURE( 0 <= nRet && nRet <= 100, "value out of range" );
    rVal <<= nRet;
    return true;
}

bool SwTransparencyGrf::PutValue( const uno::Any& rVal,
                                        sal_uInt8 )
{
    //temporary conversion until this is a SfxInt16Item!
    OSL_ENSURE(dynamic_cast<const SfxByteItem*>( this ) !=  nullptr,"Put/QueryValue should be removed!");
    sal_Int16 nVal = 0;
    if(!(rVal >>= nVal) || nVal < -100 || nVal > 100)
        return false;
    if(nVal < 0)
    {
        // for compatibility with old documents
        // introduce rounding as for SO 6.0 PP2
        nVal = ( ( nVal * 128 ) - (99/2) ) / 100;
        nVal += 128;
    }
    OSL_ENSURE( 0 <= nVal && nVal <= 100, "value out of range" );
    SetValue(static_cast<sal_uInt8>(nVal));
    return true;
}

// Sw___Grf::Clone(..) cont'd

SfxPoolItem* SwDrawModeGrf::Clone( SfxItemPool * ) const
{
    return new SwDrawModeGrf( *this );
}

// SwDrawModeGrf

sal_uInt16 SwDrawModeGrf::GetValueCount() const
{
    return GRAPHICDRAWMODE_WATERMARK + 1;
}

bool SwDrawModeGrf::QueryValue( uno::Any& rVal,
                                sal_uInt8 ) const
{
    drawing::ColorMode eRet = (drawing::ColorMode)GetEnumValue();
    rVal <<= eRet;
    return true;
}

bool SwDrawModeGrf::PutValue( const uno::Any& rVal,
                                sal_uInt8 )
{
    sal_Int32 eVal = SWUnoHelper::GetEnumAsInt32( rVal );
    if(eVal >= 0 && eVal <= GRAPHICDRAWMODE_WATERMARK)
    {
        SetEnumValue((sal_uInt16)eVal);
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
