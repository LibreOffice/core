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

#include <editeng/justifyitem.hxx>
#include <editeng/memberids.h>
#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>

#include <tools/stream.hxx>

#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/table/CellJustifyMethod.hpp>
#include <com/sun/star/table/CellVertJustify2.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>


SfxPoolItem* SvxHorJustifyItem::CreateDefault() { return new  SvxHorJustifyItem(SvxCellHorJustify::Standard, 0) ;}
SfxPoolItem* SvxVerJustifyItem::CreateDefault() { return new  SvxVerJustifyItem(SvxCellVerJustify::Standard, 0) ;}

using namespace ::com::sun::star;


SvxHorJustifyItem::SvxHorJustifyItem( const sal_uInt16 nId ) :
    SfxEnumItem( nId, SvxCellHorJustify::Standard )
{
}

SvxHorJustifyItem::SvxHorJustifyItem( const SvxCellHorJustify eJustify,
                                      const sal_uInt16 nId ) :
    SfxEnumItem( nId, eJustify )
{
}


bool SvxHorJustifyItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&)    const
{
    rText = GetValueText( static_cast<sal_uInt16>(GetValue()) );
    return true;
}


bool SvxHorJustifyItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_HORJUST_HORJUST:
            {
                table::CellHoriJustify eUno = table::CellHoriJustify_STANDARD;
                switch ( GetValue() )
                {
                    case SvxCellHorJustify::Standard: eUno = table::CellHoriJustify_STANDARD; break;
                    case SvxCellHorJustify::Left:     eUno = table::CellHoriJustify_LEFT;     break;
                    case SvxCellHorJustify::Center:   eUno = table::CellHoriJustify_CENTER;   break;
                    case SvxCellHorJustify::Right:    eUno = table::CellHoriJustify_RIGHT;    break;
                    case SvxCellHorJustify::Block:    eUno = table::CellHoriJustify_BLOCK;    break;
                    case SvxCellHorJustify::Repeat:   eUno = table::CellHoriJustify_REPEAT;   break;
                }
                rVal <<= eUno;
            }
            break;
        case MID_HORJUST_ADJUST:
            {
                //  ParagraphAdjust values, as in SvxAdjustItem
                //  (same value for ParaAdjust and ParaLastLineAdjust)

                style::ParagraphAdjust nAdjust = style::ParagraphAdjust_LEFT;
                switch ( GetValue() )
                {
                    // ParagraphAdjust_LEFT is used for STANDARD and REPEAT
                    case SvxCellHorJustify::Standard:
                    case SvxCellHorJustify::Repeat:
                    case SvxCellHorJustify::Left:   nAdjust = style::ParagraphAdjust_LEFT;   break;
                    case SvxCellHorJustify::Center: nAdjust = style::ParagraphAdjust_CENTER; break;
                    case SvxCellHorJustify::Right:  nAdjust = style::ParagraphAdjust_RIGHT;  break;
                    case SvxCellHorJustify::Block:  nAdjust = style::ParagraphAdjust_BLOCK;  break;
                }
                rVal <<= static_cast<sal_Int16>(nAdjust);       // as sal_Int16
            }
            break;
    }
    return true;
}

bool SvxHorJustifyItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_HORJUST_HORJUST:
            {
                table::CellHoriJustify eUno;
                if(!(rVal >>= eUno))
                {
                    sal_Int32 nValue = 0;
                    if(!(rVal >>= nValue))
                        return false;
                    eUno = static_cast<table::CellHoriJustify>(nValue);
                }
                SvxCellHorJustify eSvx = SvxCellHorJustify::Standard;
                switch (eUno)
                {
                    case table::CellHoriJustify_STANDARD: eSvx = SvxCellHorJustify::Standard; break;
                    case table::CellHoriJustify_LEFT:     eSvx = SvxCellHorJustify::Left;     break;
                    case table::CellHoriJustify_CENTER:   eSvx = SvxCellHorJustify::Center;   break;
                    case table::CellHoriJustify_RIGHT:    eSvx = SvxCellHorJustify::Right;    break;
                    case table::CellHoriJustify_BLOCK:    eSvx = SvxCellHorJustify::Block;    break;
                    case table::CellHoriJustify_REPEAT:   eSvx = SvxCellHorJustify::Repeat;   break;
                    default: ; //prevent warning
                }
                SetValue( eSvx );
            }
            break;
        case MID_HORJUST_ADJUST:
            {
                //  property contains ParagraphAdjust values as sal_Int16
                sal_Int16 nVal = sal_Int16();
                if(!(rVal >>= nVal))
                    return false;

                SvxCellHorJustify eSvx = SvxCellHorJustify::Standard;
                switch (static_cast<style::ParagraphAdjust>(nVal))
                {
                    //  STRETCH is treated as BLOCK
                    case style::ParagraphAdjust_LEFT:    eSvx = SvxCellHorJustify::Left;   break;
                    case style::ParagraphAdjust_RIGHT:   eSvx = SvxCellHorJustify::Right;  break;
                    case style::ParagraphAdjust_STRETCH:
                    case style::ParagraphAdjust_BLOCK:   eSvx = SvxCellHorJustify::Block;  break;
                    case style::ParagraphAdjust_CENTER:  eSvx = SvxCellHorJustify::Center; break;
                    default: break;
                }
                SetValue( eSvx );
            }
    }
    return true;
}


OUString SvxHorJustifyItem::GetValueText( sal_uInt16 nVal )
{
    DBG_ASSERT( nVal <= sal_uInt16(SvxCellHorJustify::Repeat), "enum overflow!" );
    return EditResId(RID_SVXITEMS_HORJUST_STANDARD + nVal);
}


SfxPoolItem* SvxHorJustifyItem::Clone( SfxItemPool* ) const
{
    return new SvxHorJustifyItem( *this );
}


SfxPoolItem* SvxHorJustifyItem::Create( SvStream& rStream, sal_uInt16 ) const
{
    sal_uInt16 nVal;
    rStream.ReadUInt16( nVal );
    return new SvxHorJustifyItem( static_cast<SvxCellHorJustify>(nVal), Which() );
}


sal_uInt16 SvxHorJustifyItem::GetValueCount() const
{
    return sal_uInt16(SvxCellHorJustify::Repeat) + 1;  // Last Enum value + 1
}


SvxVerJustifyItem::SvxVerJustifyItem( const sal_uInt16 nId ) :
    SfxEnumItem( nId, SvxCellVerJustify::Standard )
{
}

SvxVerJustifyItem::SvxVerJustifyItem( const SvxCellVerJustify eJustify,
                                      const sal_uInt16 nId ) :
    SfxEnumItem( nId, eJustify )
{
}


bool SvxVerJustifyItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper& )    const
{
    rText = GetValueText( GetValue() );
    return true;
}


bool SvxVerJustifyItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_HORJUST_ADJUST:
            {
                style::VerticalAlignment eUno = style::VerticalAlignment_TOP;
                switch ( GetValue() )
                {
                    case SvxCellVerJustify::Top:      eUno = style::VerticalAlignment_TOP;     break;
                    case SvxCellVerJustify::Center:   eUno = style::VerticalAlignment_MIDDLE;  break;
                    case SvxCellVerJustify::Bottom:   eUno = style::VerticalAlignment_BOTTOM;  break;
                    default: ; //prevent warning
                }
                rVal <<= eUno;
                break;
            }
        default:
            {
                sal_Int32 nUno = table::CellVertJustify2::STANDARD;
                switch ( GetValue() )
                {
                    case SvxCellVerJustify::Standard: nUno = table::CellVertJustify2::STANDARD; break;
                    case SvxCellVerJustify::Top:      nUno = table::CellVertJustify2::TOP;     break;
                    case SvxCellVerJustify::Center:   nUno = table::CellVertJustify2::CENTER;  break;
                    case SvxCellVerJustify::Bottom:   nUno = table::CellVertJustify2::BOTTOM;  break;
                    case SvxCellVerJustify::Block:    nUno = table::CellVertJustify2::BLOCK;  break;
                    default: ; //prevent warning
                }
                rVal <<= nUno;
                break;
            }
    }
    return true;
}

bool SvxVerJustifyItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_HORJUST_ADJUST:
            {
                //  property contains ParagraphAdjust values as sal_Int16
                style::VerticalAlignment nVal = style::VerticalAlignment_TOP;
                if(!(rVal >>= nVal))
                    return false;

                SvxCellVerJustify eSvx = SvxCellVerJustify::Standard;
                switch (nVal)
                {
                    case style::VerticalAlignment_TOP:      eSvx = SvxCellVerJustify::Top;     break;
                    case style::VerticalAlignment_MIDDLE:   eSvx = SvxCellVerJustify::Center;  break;
                    case style::VerticalAlignment_BOTTOM:   eSvx = SvxCellVerJustify::Bottom;  break;
                    default:;
                }
                SetValue( eSvx );
                break;
            }
        default:
            {
                sal_Int32 eUno = table::CellVertJustify2::STANDARD;
                rVal >>= eUno;

                SvxCellVerJustify eSvx = SvxCellVerJustify::Standard;
                switch (eUno)
                {
                    case table::CellVertJustify2::STANDARD: eSvx = SvxCellVerJustify::Standard;  break;
                    case table::CellVertJustify2::TOP:      eSvx = SvxCellVerJustify::Top;       break;
                    case table::CellVertJustify2::CENTER:   eSvx = SvxCellVerJustify::Center;    break;
                    case table::CellVertJustify2::BOTTOM:   eSvx = SvxCellVerJustify::Bottom;    break;
                    case table::CellVertJustify2::BLOCK:    eSvx = SvxCellVerJustify::Block;     break;
                    default: ; //prevent warning
                }
                SetValue( eSvx );
                break;
            }
    }

    return true;
}


OUString SvxVerJustifyItem::GetValueText( SvxCellVerJustify nVal )
{
    DBG_ASSERT( nVal <= SvxCellVerJustify::Bottom, "enum overflow!" );
    return EditResId(RID_SVXITEMS_VERJUST_STANDARD + static_cast<int>(nVal));
}


SfxPoolItem* SvxVerJustifyItem::Clone( SfxItemPool* ) const
{
    return new SvxVerJustifyItem( *this );
}


SfxPoolItem* SvxVerJustifyItem::Create( SvStream& rStream, sal_uInt16 ) const
{
    sal_uInt16 nVal;
    rStream.ReadUInt16( nVal );
    return new SvxVerJustifyItem( static_cast<SvxCellVerJustify>(nVal), Which() );
}


sal_uInt16 SvxVerJustifyItem::GetValueCount() const
{
    return static_cast<sal_uInt16>(SvxCellVerJustify::Bottom) + 1;  // Last Enum value + 1
}


SvxJustifyMethodItem::SvxJustifyMethodItem( const SvxCellJustifyMethod eJustify,
                                      const sal_uInt16 nId ) :
    SfxEnumItem( nId, eJustify )
{
}


bool SvxJustifyMethodItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper& )    const
{
    rText = GetValueText( GetValue() );
    return true;
}


bool SvxJustifyMethodItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    sal_Int32 nUno = table::CellJustifyMethod::AUTO;
    switch (GetValue())
    {
        case SvxCellJustifyMethod::Auto:       nUno = table::CellJustifyMethod::AUTO;       break;
        case SvxCellJustifyMethod::Distribute: nUno = table::CellJustifyMethod::DISTRIBUTE; break;
        default:;
    }
    rVal <<= nUno;
    return true;
}

bool SvxJustifyMethodItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    sal_Int32 nVal = table::CellJustifyMethod::AUTO;
    if (!(rVal >>= nVal))
        return false;

    SvxCellJustifyMethod eSvx = SvxCellJustifyMethod::Auto;
    switch (nVal)
    {
        case table::CellJustifyMethod::AUTO:
            eSvx = SvxCellJustifyMethod::Auto;
        break;
        case table::CellJustifyMethod::DISTRIBUTE:
            eSvx = SvxCellJustifyMethod::Distribute;
        break;
        default:;
    }
    SetValue(eSvx);
    return true;
}


OUString SvxJustifyMethodItem::GetValueText( SvxCellJustifyMethod nVal )
{
    return EditResId(RID_SVXITEMS_JUSTMETHOD_AUTO + static_cast<int>(nVal));
}


SfxPoolItem* SvxJustifyMethodItem::Clone( SfxItemPool* ) const
{
    return new SvxJustifyMethodItem( *this );
}


sal_uInt16 SvxJustifyMethodItem::GetValueCount() const
{
    return static_cast<sal_uInt16>(SvxCellJustifyMethod::Distribute) + 1;   // Last Enum value + 1
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
