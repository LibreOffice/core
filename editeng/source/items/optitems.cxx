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
#include <tools/shl.hxx>
#include <tools/resid.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>

#include <editeng/optitems.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(SfxSpellCheckItem, SfxPoolItem);
TYPEINIT1(SfxHyphenRegionItem, SfxPoolItem);

// class SfxSpellCheckItem -----------------------------------------------

SfxSpellCheckItem::SfxSpellCheckItem
(
    Reference< XSpellChecker1 > &xChecker,
    sal_uInt16 _nWhich
) :

    SfxPoolItem( _nWhich )
{
    xSpellCheck = xChecker;
}

// -----------------------------------------------------------------------

SfxSpellCheckItem::SfxSpellCheckItem( const SfxSpellCheckItem& rItem ) :

    SfxPoolItem( rItem ),
    xSpellCheck( rItem.GetXSpellChecker() )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SfxSpellCheckItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          ,
    SfxMapUnit          ,
    String&             rText,
    const IntlWrapper*
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            return ePres;
        }
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxSpellCheckItem::Clone( SfxItemPool* ) const
{
    return new SfxSpellCheckItem( *this );
}

// -----------------------------------------------------------------------

int SfxSpellCheckItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rItem), "unequal types" );
    return ( xSpellCheck == ( (const SfxSpellCheckItem& )rItem ).GetXSpellChecker() );
}

// class SfxHyphenRegionItem -----------------------------------------------

SfxHyphenRegionItem::SfxHyphenRegionItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId )
{
    nMinLead = nMinTrail = 0;
}

// -----------------------------------------------------------------------

SfxHyphenRegionItem::SfxHyphenRegionItem( const SfxHyphenRegionItem& rItem ) :

    SfxPoolItem ( rItem ),

    nMinLead    ( rItem.GetMinLead() ),
    nMinTrail   ( rItem.GetMinTrail() )
{
}

// -----------------------------------------------------------------------

int SfxHyphenRegionItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return ( ( ( (SfxHyphenRegionItem&)rAttr ).nMinLead == nMinLead ) &&
             ( ( (SfxHyphenRegionItem&)rAttr ).nMinTrail == nMinTrail ) );
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxHyphenRegionItem::Clone( SfxItemPool* ) const
{
    return new SfxHyphenRegionItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SfxHyphenRegionItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          ,
    SfxMapUnit          ,
    String&             rText,
    const IntlWrapper*
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText += String::CreateFromInt32( nMinLead );
            rText += String( EditResId( RID_SVXITEMS_HYPHEN_MINLEAD ) );
            rText += ',';
            rText += String::CreateFromInt32( nMinTrail );
            rText += String( EditResId( RID_SVXITEMS_HYPHEN_MINTRAIL ) );
            return ePres;
        }
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxHyphenRegionItem::Create(SvStream& rStrm, sal_uInt16 ) const
{
    sal_uInt8 _nMinLead, _nMinTrail;
    rStrm >> _nMinLead >> _nMinTrail;
    SfxHyphenRegionItem* pAttr = new SfxHyphenRegionItem( Which() );
    pAttr->GetMinLead() = _nMinLead;
    pAttr->GetMinTrail() = _nMinTrail;
    return pAttr;
}

// -----------------------------------------------------------------------

SvStream& SfxHyphenRegionItem::Store( SvStream& rStrm, sal_uInt16 ) const
{
    rStrm << (sal_uInt8) GetMinLead()
          << (sal_uInt8) GetMinTrail();
    return rStrm;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
