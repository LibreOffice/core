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

#include <svx/grfcrop.hxx>
#include <editeng/itemtype.hxx>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <tools/mapunit.hxx>
#include <tools/UnitConversion.hxx>

using namespace ::com::sun::star;

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
    assert(SfxPoolItem::operator==(rAttr));

    const SvxGrfCrop& rCrop = static_cast<const SvxGrfCrop&>(rAttr);
    return nLeft    == rCrop.GetLeft() &&
           nRight   == rCrop.GetRight() &&
           nTop     == rCrop.GetTop() &&
           nBottom  == rCrop.GetBottom();
}


bool SvxGrfCrop::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
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

bool SvxGrfCrop::GetPresentation(
    SfxItemPresentation ePres, MapUnit eCoreUnit, MapUnit /*ePresUnit*/,
    OUString &rText, const IntlWrapper& rIntl ) const
{
    rText.clear();
    switch( ePres )
    {
    case SfxItemPresentation::Nameless:
        return true;
    case SfxItemPresentation::Complete:
        rText = "L: "  + ::GetMetricText( GetLeft(), eCoreUnit, MapUnit::MapMM, &rIntl ) +
                " R: " + ::GetMetricText( GetRight(), eCoreUnit, MapUnit::MapMM, &rIntl ) +
                " T: " + ::GetMetricText( GetTop(), eCoreUnit, MapUnit::MapMM, &rIntl ) +
                " B: " + ::GetMetricText( GetBottom(), eCoreUnit, MapUnit::MapMM, &rIntl );
        return true;

    default:
        return false;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
