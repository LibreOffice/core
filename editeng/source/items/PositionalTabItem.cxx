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

#include <editeng/PositionalTabItem.hxx>
#include <tools/stream.hxx>                 // Needed for <<,>>
#include <com/sun/star/style/PositionalTab.hpp>
#include <com/sun/star/style/PositionalTabAlign.hpp>
#include <com/sun/star/style/PositionalTabRelativeTo.hpp>

using namespace ::com::sun::star;

TYPEINIT1_FACTORY(SvxPositionalTabItem, SfxPoolItem, new SvxPositionalTabItem(0));

// class SvxPositionalTabItem --------------------------------------------------

SvxPositionalTabItem::SvxPositionalTabItem( sal_uInt16 nWhich ) :
    SfxPoolItem         ( nWhich ),
    m_eAlignment        ( SVX_POSITIONAL_TAB_ALIGNMENT_LEFT ),
    m_cLeaderCharacter  ( cDfltPTabLeaderChar ),
    m_eRelativeTo       ( SVX_POSITIONAL_TAB_BASE_MARGIN )
{

}

// -----------------------------------------------------------------------

SvxPositionalTabItem::SvxPositionalTabItem(     sal_uInt16 nWhich,
                                                const SvxPositionalTabAlignment eAlignment,
                                                const sal_Unicode cLeaderCharacter,
                                                const SvxPositionalTabBase eRelativeTo ) :
    SfxPoolItem         ( nWhich ),
    m_eAlignment        ( eAlignment ),
    m_cLeaderCharacter  ( cLeaderCharacter ),
    m_eRelativeTo       ( eRelativeTo )
{

}

// -----------------------------------------------------------------------

SvxPositionalTabItem::SvxPositionalTabItem( const SvxPositionalTabItem& rPositionalTabItem ) :
    SfxPoolItem         ( rPositionalTabItem.Which() ),
    m_eAlignment        ( rPositionalTabItem.m_eAlignment ),
    m_cLeaderCharacter  ( rPositionalTabItem.m_cLeaderCharacter ),
    m_eRelativeTo       ( rPositionalTabItem.m_eRelativeTo )
{
}

// -----------------------------------------------------------------------

SvxPositionalTabItem::~SvxPositionalTabItem()
{
}

// -----------------------------------------------------------------------

SvxPositionalTabItem& SvxPositionalTabItem::operator=( const SvxPositionalTabItem& rPositionalTabItem )
{
    m_eAlignment        = rPositionalTabItem.m_eAlignment;
    m_cLeaderCharacter  = rPositionalTabItem.m_cLeaderCharacter;
    m_eRelativeTo       = rPositionalTabItem.m_eRelativeTo;

    return *this;
}

// -----------------------------------------------------------------------

int SvxPositionalTabItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxPositionalTabItem& rPositionalTabItem = (SvxPositionalTabItem&)rAttr;

    return (
        // Same Alignment ?
        (m_eAlignment == rPositionalTabItem.m_eAlignment)
        // Same Leader Character ?
        && (m_cLeaderCharacter == rPositionalTabItem.m_cLeaderCharacter)
        // Same Relative-To ?
        && (m_eRelativeTo == rPositionalTabItem.m_eRelativeTo)
        ) ?
                1 : 0;
}

// -----------------------------------------------------------------------

bool SvxPositionalTabItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    style::PositionalTab aPositionalTab;

    switch (m_eAlignment)
    {
        case SVX_POSITIONAL_TAB_ALIGNMENT_LEFT:
            aPositionalTab.Alignment = style::PositionalTabAlign::LEFT;
            break;
        case SVX_POSITIONAL_TAB_ALIGNMENT_CENTER:
            aPositionalTab.Alignment = style::PositionalTabAlign::CENTER;
            break;
        case SVX_POSITIONAL_TAB_ALIGNMENT_RIGHT:
            aPositionalTab.Alignment = style::PositionalTabAlign::RIGHT;
            break;
        default:
            // just to remove warning about 'SVX_POSITIONAL_TAB_ALIGNMENT_END'
            break;
    };

    aPositionalTab.Leader = (char)m_cLeaderCharacter;

    switch (m_eRelativeTo)
    {
        case SVX_POSITIONAL_TAB_BASE_MARGIN:
            aPositionalTab.RelativeTo = style::PositionalTabRelativeTo::MARGIN;
            break;
        case SVX_POSITIONAL_TAB_BASE_INDENT:
            aPositionalTab.RelativeTo = style::PositionalTabRelativeTo::INDENT;
            break;
        default:
            // just to remove warning about 'SVX_POSITIONAL_TAB_BASE_END'
            break;
    };

    rVal <<= aPositionalTab;

    return sal_True;
}

bool SvxPositionalTabItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    style::PositionalTab aPositionalTab;
    if(!(rVal >>= aPositionalTab))
    {
        return sal_False;
    }

    SvxPositionalTabAlignment eAlignment;
    switch (aPositionalTab.Alignment)
    {
        case style::PositionalTabAlign::LEFT:
            eAlignment = SVX_POSITIONAL_TAB_ALIGNMENT_LEFT;
            break;
        case style::PositionalTabAlign::CENTER:
            eAlignment = SVX_POSITIONAL_TAB_ALIGNMENT_CENTER;
            break;
        case style::PositionalTabAlign::RIGHT:
            eAlignment = SVX_POSITIONAL_TAB_ALIGNMENT_RIGHT;
            break;
        default:
            eAlignment = SVX_POSITIONAL_TAB_ALIGNMENT_LEFT;
            break;
    };

    SetAlignment( eAlignment );

    SetLeaderCharacter( (sal_Unicode)aPositionalTab.Leader );

    SvxPositionalTabBase eRelativeTo;
    switch (aPositionalTab.RelativeTo)
    {
        case style::PositionalTabRelativeTo::MARGIN:
            eRelativeTo = SVX_POSITIONAL_TAB_BASE_MARGIN;
            break;
        case style::PositionalTabRelativeTo::INDENT:
            eRelativeTo = SVX_POSITIONAL_TAB_BASE_INDENT;
            break;
        default:
            eRelativeTo = SVX_POSITIONAL_TAB_BASE_MARGIN;
            break;
    };

    SetRelativeTo(eRelativeTo);

    return sal_True;
}

// -----------------------------------------------------------------------

SfxItemPresentation SvxPositionalTabItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    /*
    rText = OUString();

    switch ( ePres ):
    {
        case SFX_ITEM_PRESENTATION_NONE:
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            // To-Do: Write some presentation code here
            break;
    };
    */

#ifdef DBG_UTIL
    rText = "SvxPositionalTabItem";
#else
    rText = OUString();
#endif

    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxPositionalTabItem::Clone( SfxItemPool * ) const
{
    return new SvxPositionalTabItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxPositionalTabItem::Create( SvStream& rStrm, sal_uInt16 /*nVersion*/ ) const
{
    sal_Int8        eAlignment;
    unsigned char   cLeaderCharacter;
    sal_Int8        eRelativeTo;

    rStrm >> eAlignment;
    rStrm >> cLeaderCharacter;
    rStrm >> eRelativeTo;

    return new SvxPositionalTabItem( Which(), (SvxPositionalTabAlignment)eAlignment, sal_Unicode(cLeaderCharacter), (SvxPositionalTabBase)eRelativeTo );
}

SvStream& SvxPositionalTabItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_Int8) GetAlignment();
    rStrm << (unsigned char) GetLeaderCharacter();
    rStrm << (sal_Int8) GetRelativeTo();

    return rStrm;
}
