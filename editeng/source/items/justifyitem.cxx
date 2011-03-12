/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_editeng.hxx"

// include ---------------------------------------------------------------

#include "editeng/justifyitem.hxx"
#include "editeng/memberids.hrc"
#include "editeng/editrids.hrc"
#include "editeng/eerdll.hxx"

#include <tools/stream.hxx>

#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/table/CellJustifyMethod.hpp>
#include <com/sun/star/table/CellVertJustify2.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>


TYPEINIT1_FACTORY( SvxHorJustifyItem, SfxEnumItem, new SvxHorJustifyItem(SVX_HOR_JUSTIFY_STANDARD, 0) );
TYPEINIT1_FACTORY( SvxVerJustifyItem, SfxEnumItem, new SvxVerJustifyItem(SVX_VER_JUSTIFY_STANDARD, 0) );

using namespace ::com::sun::star;



SvxHorJustifyItem::SvxHorJustifyItem( const USHORT nId ) :
    SfxEnumItem( nId, (USHORT)SVX_HOR_JUSTIFY_STANDARD )
{
}

SvxHorJustifyItem::SvxHorJustifyItem( const SvxCellHorJustify eJustify,
                                      const USHORT nId ) :
    SfxEnumItem( nId, (USHORT)eJustify )
{
}


SfxItemPresentation SvxHorJustifyItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&              rText, const IntlWrapper *)    const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueText( GetValue() );
            return SFX_ITEM_PRESENTATION_COMPLETE;
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}


bool SvxHorJustifyItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_HORJUST_HORJUST:
            {
                table::CellHoriJustify eUno = table::CellHoriJustify_STANDARD;
                switch ( (SvxCellHorJustify)GetValue() )
                {
                    case SVX_HOR_JUSTIFY_STANDARD: eUno = table::CellHoriJustify_STANDARD; break;
                    case SVX_HOR_JUSTIFY_LEFT:     eUno = table::CellHoriJustify_LEFT;     break;
                    case SVX_HOR_JUSTIFY_CENTER:   eUno = table::CellHoriJustify_CENTER;   break;
                    case SVX_HOR_JUSTIFY_RIGHT:    eUno = table::CellHoriJustify_RIGHT;    break;
                    case SVX_HOR_JUSTIFY_BLOCK:    eUno = table::CellHoriJustify_BLOCK;    break;
                    case SVX_HOR_JUSTIFY_REPEAT:   eUno = table::CellHoriJustify_REPEAT;   break;
                }
                rVal <<= eUno;
            }
            break;
        case MID_HORJUST_ADJUST:
            {
                //  ParagraphAdjust values, as in SvxAdjustItem
                //  (same value for ParaAdjust and ParaLastLineAdjust)

                sal_Int16 nAdjust = style::ParagraphAdjust_LEFT;
                switch ( (SvxCellHorJustify)GetValue() )
                {
                    // ParagraphAdjust_LEFT is used for STANDARD and REPEAT
                    case SVX_HOR_JUSTIFY_STANDARD:
                    case SVX_HOR_JUSTIFY_REPEAT:
                    case SVX_HOR_JUSTIFY_LEFT:   nAdjust = style::ParagraphAdjust_LEFT;   break;
                    case SVX_HOR_JUSTIFY_CENTER: nAdjust = style::ParagraphAdjust_CENTER; break;
                    case SVX_HOR_JUSTIFY_RIGHT:  nAdjust = style::ParagraphAdjust_RIGHT;  break;
                    case SVX_HOR_JUSTIFY_BLOCK:  nAdjust = style::ParagraphAdjust_BLOCK;  break;
                }
                rVal <<= nAdjust;       // as sal_Int16
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
                    eUno = (table::CellHoriJustify)nValue;
                }
                SvxCellHorJustify eSvx = SVX_HOR_JUSTIFY_STANDARD;
                switch (eUno)
                {
                    case table::CellHoriJustify_STANDARD: eSvx = SVX_HOR_JUSTIFY_STANDARD; break;
                    case table::CellHoriJustify_LEFT:     eSvx = SVX_HOR_JUSTIFY_LEFT;     break;
                    case table::CellHoriJustify_CENTER:   eSvx = SVX_HOR_JUSTIFY_CENTER;   break;
                    case table::CellHoriJustify_RIGHT:    eSvx = SVX_HOR_JUSTIFY_RIGHT;    break;
                    case table::CellHoriJustify_BLOCK:    eSvx = SVX_HOR_JUSTIFY_BLOCK;    break;
                    case table::CellHoriJustify_REPEAT:   eSvx = SVX_HOR_JUSTIFY_REPEAT;   break;
                    default: ; //prevent warning
                }
                SetValue( (USHORT)eSvx );
            }
            break;
        case MID_HORJUST_ADJUST:
            {
                //  property contains ParagraphAdjust values as sal_Int16
                sal_Int16 nVal = sal_Int16();
                if(!(rVal >>= nVal))
                    return false;

                SvxCellHorJustify eSvx = SVX_HOR_JUSTIFY_STANDARD;
                switch (nVal)
                {
                    //  STRETCH is treated as BLOCK
                    case style::ParagraphAdjust_LEFT:    eSvx = SVX_HOR_JUSTIFY_LEFT;   break;
                    case style::ParagraphAdjust_RIGHT:   eSvx = SVX_HOR_JUSTIFY_RIGHT;  break;
                    case style::ParagraphAdjust_STRETCH:
                    case style::ParagraphAdjust_BLOCK:   eSvx = SVX_HOR_JUSTIFY_BLOCK;  break;
                    case style::ParagraphAdjust_CENTER:  eSvx = SVX_HOR_JUSTIFY_CENTER; break;
                }
                SetValue( (USHORT)eSvx );
            }
    }
    return true;
}


XubString SvxHorJustifyItem::GetValueText( USHORT nVal ) const
{
    DBG_ASSERT( nVal <= SVX_HOR_JUSTIFY_REPEAT, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_HORJUST_STANDARD + nVal);
}


SfxPoolItem* SvxHorJustifyItem::Clone( SfxItemPool* ) const
{
    return new SvxHorJustifyItem( *this );
}


SfxPoolItem* SvxHorJustifyItem::Create( SvStream& rStream, USHORT ) const
{
    USHORT nVal;
    rStream >> nVal;
    return new SvxHorJustifyItem( (SvxCellHorJustify)nVal, Which() );
}


USHORT SvxHorJustifyItem::GetValueCount() const
{
    return SVX_HOR_JUSTIFY_REPEAT + 1;  // Last Enum value + 1
}


SvxVerJustifyItem::SvxVerJustifyItem( const USHORT nId ) :
    SfxEnumItem( nId, (USHORT)SVX_VER_JUSTIFY_STANDARD )
{
}

SvxVerJustifyItem::SvxVerJustifyItem( const SvxCellVerJustify eJustify,
                                      const USHORT nId ) :
    SfxEnumItem( nId, (USHORT)eJustify )
{
}


SfxItemPresentation SvxVerJustifyItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&              rText,
    const IntlWrapper * )    const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueText( GetValue() );
            return SFX_ITEM_PRESENTATION_COMPLETE;
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}


bool SvxVerJustifyItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_HORJUST_ADJUST:
            {
                style::VerticalAlignment eUno = style::VerticalAlignment_TOP;
                switch ( (SvxCellVerJustify)GetValue() )
                {
                    case SVX_VER_JUSTIFY_TOP:      eUno = style::VerticalAlignment_TOP;     break;
                    case SVX_VER_JUSTIFY_CENTER:   eUno = style::VerticalAlignment_MIDDLE;  break;
                    case SVX_VER_JUSTIFY_BOTTOM:   eUno = style::VerticalAlignment_BOTTOM;  break;
                    default: ; //prevent warning
                }
                rVal <<= eUno;
                break;
            }
        default:
            {
                sal_Int32 nUno = table::CellVertJustify2::STANDARD;
                switch ( (SvxCellVerJustify)GetValue() )
                {
                    case SVX_VER_JUSTIFY_STANDARD: nUno = table::CellVertJustify2::STANDARD; break;
                    case SVX_VER_JUSTIFY_TOP:      nUno = table::CellVertJustify2::TOP;     break;
                    case SVX_VER_JUSTIFY_CENTER:   nUno = table::CellVertJustify2::CENTER;  break;
                    case SVX_VER_JUSTIFY_BOTTOM:   nUno = table::CellVertJustify2::BOTTOM;  break;
                    case SVX_VER_JUSTIFY_BLOCK:    nUno = table::CellVertJustify2::BLOCK;  break;
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

                SvxCellVerJustify eSvx = SVX_VER_JUSTIFY_STANDARD;
                switch (nVal)
                {
                    case style::VerticalAlignment_TOP:      eSvx = SVX_VER_JUSTIFY_TOP;     break;
                    case style::VerticalAlignment_MIDDLE:   eSvx = SVX_VER_JUSTIFY_CENTER;  break;
                    case style::VerticalAlignment_BOTTOM:   eSvx = SVX_VER_JUSTIFY_BOTTOM;  break;
                    default:;
                }
                SetValue( (USHORT)eSvx );
                break;
            }
        default:
            {
                sal_Int32 eUno = table::CellVertJustify2::STANDARD;
                rVal >>= eUno;

                SvxCellVerJustify eSvx = SVX_VER_JUSTIFY_STANDARD;
                switch (eUno)
                {
                    case table::CellVertJustify2::STANDARD: eSvx = SVX_VER_JUSTIFY_STANDARD;  break;
                    case table::CellVertJustify2::TOP:      eSvx = SVX_VER_JUSTIFY_TOP;       break;
                    case table::CellVertJustify2::CENTER:   eSvx = SVX_VER_JUSTIFY_CENTER;    break;
                    case table::CellVertJustify2::BOTTOM:   eSvx = SVX_VER_JUSTIFY_BOTTOM;    break;
                    case table::CellVertJustify2::BLOCK:    eSvx = SVX_VER_JUSTIFY_BLOCK;     break;
                    default: ; //prevent warning
                }
                SetValue( (USHORT)eSvx );
                break;
            }
    }

    return true;
}


XubString SvxVerJustifyItem::GetValueText( USHORT nVal ) const
{
    DBG_ASSERT( nVal <= SVX_VER_JUSTIFY_BOTTOM, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_VERJUST_STANDARD + nVal);
}


SfxPoolItem* SvxVerJustifyItem::Clone( SfxItemPool* ) const
{
    return new SvxVerJustifyItem( *this );
}


SfxPoolItem* SvxVerJustifyItem::Create( SvStream& rStream, USHORT ) const
{
    USHORT nVal;
    rStream >> nVal;
    return new SvxVerJustifyItem( (SvxCellVerJustify)nVal, Which() );
}


USHORT SvxVerJustifyItem::GetValueCount() const
{
    return SVX_VER_JUSTIFY_BOTTOM + 1;  // Last Enum value + 1
}



SvxJustifyMethodItem::SvxJustifyMethodItem( const USHORT nId ) :
    SfxEnumItem( nId, (USHORT)SVX_JUSTIFY_METHOD_AUTO )
{
}

SvxJustifyMethodItem::SvxJustifyMethodItem( const SvxCellJustifyMethod eJustify,
                                      const USHORT nId ) :
    SfxEnumItem( nId, (USHORT)eJustify )
{
}


SfxItemPresentation SvxJustifyMethodItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&              rText,
    const IntlWrapper * )    const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueText( GetValue() );
            return SFX_ITEM_PRESENTATION_COMPLETE;
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}


bool SvxJustifyMethodItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    sal_Int32 nUno = table::CellJustifyMethod::AUTO;
    switch (static_cast<SvxCellJustifyMethod>(GetValue()))
    {
        case SVX_JUSTIFY_METHOD_AUTO:       nUno = table::CellJustifyMethod::AUTO;       break;
        case SVX_JUSTIFY_METHOD_DISTRIBUTE: nUno = table::CellJustifyMethod::DISTRIBUTE; break;
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

    SvxCellJustifyMethod eSvx = SVX_JUSTIFY_METHOD_AUTO;
    switch (nVal)
    {
        case table::CellJustifyMethod::AUTO:
            eSvx = SVX_JUSTIFY_METHOD_AUTO;
        break;
        case table::CellJustifyMethod::DISTRIBUTE:
            eSvx = SVX_JUSTIFY_METHOD_DISTRIBUTE;
        break;
        default:;
    }
    SetValue(static_cast<USHORT>(eSvx));
    return true;
}


XubString SvxJustifyMethodItem::GetValueText( USHORT nVal ) const
{
    DBG_ASSERT( nVal <= SVX_VER_JUSTIFY_BOTTOM, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_JUSTMETHOD_AUTO + nVal);
}


SfxPoolItem* SvxJustifyMethodItem::Clone( SfxItemPool* ) const
{
    return new SvxJustifyMethodItem( *this );
}


SfxPoolItem* SvxJustifyMethodItem::Create( SvStream& rStream, USHORT ) const
{
    USHORT nVal;
    rStream >> nVal;
    return new SvxJustifyMethodItem( (SvxCellJustifyMethod)nVal, Which() );
}


USHORT SvxJustifyMethodItem::GetValueCount() const
{
    return SVX_JUSTIFY_METHOD_DISTRIBUTE + 1;   // Last Enum value + 1
}

SvxJustifyMethodItem& SvxJustifyMethodItem::operator=(const SvxJustifyMethodItem& r)
{
    SetValue( r.GetValue() );
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
