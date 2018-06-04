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

#include <com/sun/star/drawing/ColorMode.hpp>
#include <o3tl/any.hxx>
#include <vcl/GraphicObject.hxx>
#include <swtypes.hxx>
#include <grfatr.hxx>
#include <swunohelper.hxx>

#include <cmdid.h>
#include <unomid.h>

using namespace ::com::sun::star;


std::unique_ptr<SfxPoolItem> SwMirrorGrf::CloneInternal( SfxItemPool* ) const
{
    return o3tl::make_unique<SwMirrorGrf>( *this );
}

sal_uInt16 SwMirrorGrf::GetValueCount() const
{
    return 4;
}

bool SwMirrorGrf::operator==( const SfxPoolItem& rItem) const
{
    return SfxEnumItem::operator==(static_cast<const SfxEnumItem<MirrorGraph>&>(rItem)) &&
            static_cast<const SwMirrorGrf&>(rItem).IsGrfToggle() == IsGrfToggle();
}

static bool lcl_IsHoriOnEvenPages(MirrorGraph nEnum, bool bToggle)
{
    bool bEnum = nEnum == MirrorGraph::Vertical ||
                   nEnum == MirrorGraph::Both;
            return bEnum != bToggle;
}

static bool lcl_IsHoriOnOddPages(MirrorGraph nEnum)
{
    bool bEnum = nEnum == MirrorGraph::Vertical ||
                   nEnum == MirrorGraph::Both;
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
            bVal = GetValue() == MirrorGraph::Horizontal ||
                   GetValue() == MirrorGraph::Both;
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
    bool bVal = *o3tl::doAccess<bool>(rVal);
    // vertical and horizontal were swapped at some point
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_MIRROR_HORZ_EVEN_PAGES:
        case MID_MIRROR_HORZ_ODD_PAGES:
        {
            bool bIsVert = GetValue() == MirrorGraph::Horizontal ||
                                GetValue() == MirrorGraph::Both;
            bool bOnOddPages = nMemberId == MID_MIRROR_HORZ_EVEN_PAGES ?
                                    lcl_IsHoriOnOddPages(GetValue()) : bVal;
            bool bOnEvenPages = nMemberId == MID_MIRROR_HORZ_ODD_PAGES ?
                                       lcl_IsHoriOnEvenPages(GetValue(), IsGrfToggle()) : bVal;
            MirrorGraph nEnum = bOnOddPages ?
                    bIsVert ? MirrorGraph::Both : MirrorGraph::Vertical :
                        bIsVert ? MirrorGraph::Horizontal : MirrorGraph::Dont;
            bool bToggle = bOnOddPages != bOnEvenPages;
            SetValue(nEnum);
            SetGrfToggle( bToggle );
        }
        break;
        case MID_MIRROR_VERT:
            if ( bVal )
            {
                if ( GetValue() == MirrorGraph::Vertical )
                    SetValue( MirrorGraph::Both );
                else if ( GetValue() != MirrorGraph::Both )
                    SetValue( MirrorGraph::Horizontal );
            }
            else
            {
                if ( GetValue() == MirrorGraph::Both )
                    SetValue( MirrorGraph::Vertical );
                else if ( GetValue() == MirrorGraph::Horizontal )
                    SetValue( MirrorGraph::Dont );
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

std::unique_ptr<SfxPoolItem> SwCropGrf::CloneInternal( SfxItemPool* ) const
{
    return o3tl::make_unique<SwCropGrf>( *this );
}

sal_Int16 SwRotationGrf::checkAndCorrectValue(sal_Int16 nValue)
{
    if(nValue < 0)
    {
        // smaller zero, modulo (will keep negative) and add one range
        DBG_ASSERT(false, "SwRotationGrf: Value is in 10th degree and *has* to be in [0 .. 3600[ (!)");
        return 3600 + (nValue % 3600);
    }
    else if (nValue >= 3600)
    {
        // bigger range, use modulo
        DBG_ASSERT(false, "SwRotationGrf: Value is in 10th degree and *has* to be in [0 .. 3600[ (!)");
        return nValue % 3600;
    }

    return nValue;
}

SwRotationGrf::SwRotationGrf( sal_Int16 nVal, const Size& rSz )
    // tdf#115529 check and evtl. correct value
:   SfxUInt16Item( RES_GRFATR_ROTATION, checkAndCorrectValue(nVal) ),
    aUnrotatedSize( rSz )
{
}

std::unique_ptr<SfxPoolItem> SwRotationGrf::CloneInternal( SfxItemPool * ) const
{
    return o3tl::make_unique<SwRotationGrf>( *this );
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
    rVal <<= static_cast<sal_Int16>(GetValue());
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
        // tdf#115529 check and evtl. correct value
        SetValue(static_cast<sal_uInt16>(checkAndCorrectValue(nValue)));
        return true;
    }

    OSL_FAIL( "SwRotationGrf::PutValue - Wrong type!" );
    return false;
}

// Sw___Grf::Clone(..)

std::unique_ptr<SfxPoolItem> SwLuminanceGrf::CloneInternal( SfxItemPool * ) const
{
    return o3tl::make_unique<SwLuminanceGrf>( *this );
}

std::unique_ptr<SfxPoolItem> SwContrastGrf::CloneInternal( SfxItemPool * ) const
{
    return o3tl::make_unique<SwContrastGrf>( *this );
}

std::unique_ptr<SfxPoolItem> SwChannelRGrf::CloneInternal( SfxItemPool * ) const
{
    return o3tl::make_unique<SwChannelRGrf>( *this );
}

std::unique_ptr<SfxPoolItem> SwChannelGGrf::CloneInternal( SfxItemPool * ) const
{
    return o3tl::make_unique<SwChannelGGrf>( *this );
}

std::unique_ptr<SfxPoolItem> SwChannelBGrf::CloneInternal( SfxItemPool * ) const
{
    return o3tl::make_unique<SwChannelBGrf>( *this );
}

std::unique_ptr<SfxPoolItem> SwGammaGrf::CloneInternal( SfxItemPool * ) const
{
    return o3tl::make_unique<SwGammaGrf>( *this );
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

std::unique_ptr<SfxPoolItem> SwInvertGrf::CloneInternal( SfxItemPool * ) const
{
    return o3tl::make_unique<SwInvertGrf>( *this );
}

std::unique_ptr<SfxPoolItem> SwTransparencyGrf::CloneInternal( SfxItemPool * ) const
{
    return o3tl::make_unique<SwTransparencyGrf>( *this );
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

std::unique_ptr<SfxPoolItem> SwDrawModeGrf::CloneInternal( SfxItemPool * ) const
{
    return o3tl::make_unique<SwDrawModeGrf>( *this );
}

// SwDrawModeGrf

sal_uInt16 SwDrawModeGrf::GetValueCount() const
{
    return sal_uInt16(GraphicDrawMode::Watermark) + 1;
}

bool SwDrawModeGrf::QueryValue( uno::Any& rVal,
                                sal_uInt8 ) const
{
    drawing::ColorMode eRet = static_cast<drawing::ColorMode>(GetEnumValue());
    rVal <<= eRet;
    return true;
}

bool SwDrawModeGrf::PutValue( const uno::Any& rVal,
                                sal_uInt8 )
{
    sal_Int32 eVal = SWUnoHelper::GetEnumAsInt32( rVal );
    if(eVal >= 0 && eVal <= sal_uInt16(GraphicDrawMode::Watermark))
    {
        SetEnumValue(static_cast<sal_uInt16>(eVal));
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
