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

#include <tools/stream.hxx>
#include <svx/grfcrop.hxx>
#include <editeng/itemtype.hxx>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <tools/mapunit.hxx>

using namespace ::com::sun::star;

/******************************************************************************
 *  Implementierung     class SwCropGrf
 ******************************************************************************/

SvxGrfCrop::SvxGrfCrop( sal_uInt16 nItemId )
    : SfxPoolItem( nItemId ),
    nLeft( 0 ), nRight( 0 ), nTop( 0 ), nBottom( 0 )
{}

SvxGrfCrop::SvxGrfCrop( sal_Int32 nL, sal_Int32 nR,
                        sal_Int32 nT, sal_Int32 nB, sal_uInt16 nItemId )
    : SfxPoolItem( nItemId ),
    nLeft( nL ), nRight( nR ), nTop( nT ), nBottom( nB )
{}

SvxGrfCrop::~SvxGrfCrop()
{
}

bool SvxGrfCrop::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rAttr ), "not equal attributes" );
    return nLeft    == ((const SvxGrfCrop&)rAttr).GetLeft() &&
           nRight   == ((const SvxGrfCrop&)rAttr).GetRight() &&
           nTop     == ((const SvxGrfCrop&)rAttr).GetTop() &&
           nBottom  == ((const SvxGrfCrop&)rAttr).GetBottom();
}

SfxPoolItem* SvxGrfCrop::Create( SvStream& rStrm, sal_uInt16 nVersion ) const
{
    sal_Int32 top, left, right, bottom;
    rStrm.ReadInt32( top ).ReadInt32( left ).ReadInt32( right ).ReadInt32( bottom );

    if( GRFCROP_VERSION_SWDEFAULT == nVersion )
        top = -top, bottom = -bottom, left = -left, right = -right;

    SvxGrfCrop* pNew = (SvxGrfCrop*)Clone();
    pNew->SetLeft( left );
    pNew->SetRight( right );
    pNew->SetTop( top );
    pNew->SetBottom( bottom );
    return pNew;
}


SvStream& SvxGrfCrop::Store( SvStream& rStrm, sal_uInt16 nVersion ) const
{
    sal_Int32 left = GetLeft(), right = GetRight(),
            top = GetTop(), bottom = GetBottom();
    if( GRFCROP_VERSION_SWDEFAULT == nVersion )
        top = -top, bottom = -bottom, left = -left, right = -right;

    rStrm.WriteInt32( top ).WriteInt32( left ).WriteInt32( right ).WriteInt32( bottom );

    return rStrm;
}



bool SvxGrfCrop::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    text::GraphicCrop aRet;
    aRet.Left   = nLeft;
    aRet.Right  = nRight;
    aRet.Top    = nTop;
    aRet.Bottom = nBottom;

    if( bConvert )
    {
       aRet.Right   = convertTwipToMm100(aRet.Right );
       aRet.Top     = convertTwipToMm100(aRet.Top );
       aRet.Left    = convertTwipToMm100(aRet.Left   );
       aRet.Bottom  = convertTwipToMm100(aRet.Bottom);
    }


    rVal <<= aRet;
    return true;
}

bool SvxGrfCrop::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    text::GraphicCrop aVal;

    if(!(rVal >>= aVal))
        return false;
    if( bConvert )
    {
       aVal.Right   = convertMm100ToTwip(aVal.Right );
       aVal.Top     = convertMm100ToTwip(aVal.Top );
       aVal.Left    = convertMm100ToTwip(aVal.Left   );
       aVal.Bottom  = convertMm100ToTwip(aVal.Bottom);
    }

    nLeft   = aVal.Left  ;
    nRight  = aVal.Right ;
    nTop    = aVal.Top   ;
    nBottom = aVal.Bottom;
    return  true;
}

SfxItemPresentation SvxGrfCrop::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreUnit, SfxMapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper* pIntl ) const
{
    rText = OUString();
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
        {
            rText = "L: "  + OUString(::GetMetricText( GetLeft(), eCoreUnit, SFX_MAPUNIT_MM, pIntl )) +
                    " R: " + OUString(::GetMetricText( GetRight(), eCoreUnit, SFX_MAPUNIT_MM, pIntl )) +
                    " T: " + OUString(::GetMetricText( GetTop(), eCoreUnit, SFX_MAPUNIT_MM, pIntl )) +
                    " B: " + OUString(::GetMetricText( GetBottom(), eCoreUnit, SFX_MAPUNIT_MM, pIntl ));
        }
        break;

    default:
        ePres = SFX_ITEM_PRESENTATION_NONE;
        break;
    }
    return ePres;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
