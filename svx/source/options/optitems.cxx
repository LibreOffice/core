/*************************************************************************
 *
 *  $RCSfile: optitems.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

// -----------------------------------------------------------------------

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _TOOLS_RESID_HXX //autogen
#include <tools/resid.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#ifndef _COM_SUN_STAR_LINGUISTIC_XSPELLCHECKER1_HPP_
#include <com/sun/star/linguistic/XSpellChecker1.hpp>
#endif

#define ITEMID_SPELLCHECK   0
#define ITEMID_HYPHENREGION 0

#include "optitems.hxx"
#include "dialmgr.hxx"
#include "svxitems.hrc"

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(SfxSpellCheckItem, SfxPoolItem);
TYPEINIT1(SfxHyphenRegionItem, SfxPoolItem);

// class SfxSpellCheckItem -----------------------------------------------

SfxSpellCheckItem::SfxSpellCheckItem
(
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic::XSpellChecker1 > &xChecker,
    sal_uInt16 nWhich
) :

    SfxPoolItem( nWhich )
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
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*
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
    }
    return SFX_ITEM_PRESENTATION_NONE;
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
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*
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
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxHyphenRegionItem::Create(SvStream& rStrm, sal_uInt16 ) const
{
    sal_uInt8 nMinLead, nMinTrail;
    rStrm >> nMinLead >> nMinTrail;
    SfxHyphenRegionItem* pAttr = new SfxHyphenRegionItem( Which() );
    pAttr->GetMinLead() = nMinLead;
    pAttr->GetMinTrail() = nMinTrail;
    return pAttr;
}

// -----------------------------------------------------------------------

SvStream& SfxHyphenRegionItem::Store( SvStream& rStrm, sal_uInt16 ) const
{
    rStrm << (sal_uInt8) GetMinLead()
          << (sal_uInt8) GetMinTrail();
    return rStrm;
}


