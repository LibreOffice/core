/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optitems.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 13:00:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _TOOLS_RESID_HXX //autogen
#include <tools/resid.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLCHECKER1_HPP_
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#endif

#define ITEMID_SPELLCHECK   0
#define ITEMID_HYPHENREGION 0

#include "optitems.hxx"
#include "dialmgr.hxx"
#include "svxitems.hrc"

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
            rText += String( ResId( RID_SVXITEMS_HYPHEN_MINLEAD, DIALOG_MGR() ) );
            rText += ',';
            rText += String::CreateFromInt32( nMinTrail );
            rText += String( ResId( RID_SVXITEMS_HYPHEN_MINTRAIL, DIALOG_MGR() ) );
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


