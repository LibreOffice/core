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
#include <o3tl/hash_combine.hxx>

using namespace ::com::sun::star;

SvxGrfCrop::SvxGrfCrop( TypedWhichId<SvxGrfCrop> nItemId )
    : SfxPoolItem( nItemId ),
    m_nLeft( 0 ), m_nRight( 0 ), m_nTop( 0 ), m_nBottom( 0 )
{}

SvxGrfCrop::SvxGrfCrop( sal_Int32 nL, sal_Int32 nR,
                        sal_Int32 nT, sal_Int32 nB, TypedWhichId<SvxGrfCrop> nItemId)
    : SfxPoolItem( nItemId ),
    m_nLeft( nL ), m_nRight( nR ), m_nTop( nT ), m_nBottom( nB )
{}

SvxGrfCrop::~SvxGrfCrop()
{
}

bool SvxGrfCrop::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SvxGrfCrop& rCrop = static_cast<const SvxGrfCrop&>(rAttr);
    return m_nLeft    == rCrop.GetLeft() &&
           m_nRight   == rCrop.GetRight() &&
           m_nTop     == rCrop.GetTop() &&
           m_nBottom  == rCrop.GetBottom();
}

size_t SvxGrfCrop::hashCode() const
{
    std::size_t seed(0);
    o3tl::hash_combine(seed, m_nLeft);
    o3tl::hash_combine(seed, m_nRight);
    o3tl::hash_combine(seed, m_nTop);
    o3tl::hash_combine(seed, m_nBottom);
    return seed;
}

bool SvxGrfCrop::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    text::GraphicCrop aRet;
    aRet.Left   = m_nLeft;
    aRet.Right  = m_nRight;
    aRet.Top    = m_nTop;
    aRet.Bottom = m_nBottom;

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
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    text::GraphicCrop aVal;

    if(!(rVal >>= aVal))
        return false;
    if( bConvert )
    {
       aVal.Right   = o3tl::toTwips(aVal.Right, o3tl::Length::mm100);
       aVal.Top     = o3tl::toTwips(aVal.Top, o3tl::Length::mm100);
       aVal.Left    = o3tl::toTwips(aVal.Left, o3tl::Length::mm100);
       aVal.Bottom  = o3tl::toTwips(aVal.Bottom, o3tl::Length::mm100);
    }

    m_nLeft   = aVal.Left  ;
    m_nRight  = aVal.Right ;
    m_nTop    = aVal.Top   ;
    m_nBottom = aVal.Bottom;
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
