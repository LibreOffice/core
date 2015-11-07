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

#include <tools/resid.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>

#include <editeng/optitems.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

// STATIC DATA -----------------------------------------------------------


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



SfxSpellCheckItem::SfxSpellCheckItem( const SfxSpellCheckItem& rItem ) :

    SfxPoolItem( rItem ),
    xSpellCheck( rItem.GetXSpellChecker() )
{
}



bool SfxSpellCheckItem::GetPresentation
(
    SfxItemPresentation ,
    SfxMapUnit          ,
    SfxMapUnit          ,
    OUString&           ,
    const IntlWrapper*
)   const
{
    return true;
}



SfxPoolItem* SfxSpellCheckItem::Clone( SfxItemPool* ) const
{
    return new SfxSpellCheckItem( *this );
}



bool SfxSpellCheckItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rItem), "unequal types" );
    return ( xSpellCheck == static_cast<const SfxSpellCheckItem&>( rItem ).GetXSpellChecker() );
}

// class SfxHyphenRegionItem -----------------------------------------------

SfxHyphenRegionItem::SfxHyphenRegionItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId )
{
    nMinLead = nMinTrail = 0;
}



SfxHyphenRegionItem::SfxHyphenRegionItem( const SfxHyphenRegionItem& rItem ) :

    SfxPoolItem ( rItem ),

    nMinLead    ( rItem.GetMinLead() ),
    nMinTrail   ( rItem.GetMinTrail() )
{
}



bool SfxHyphenRegionItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return ( ( static_cast<const SfxHyphenRegionItem&>( rAttr ).nMinLead == nMinLead ) &&
             ( static_cast<const SfxHyphenRegionItem&>( rAttr ).nMinTrail == nMinTrail ) );
}



SfxPoolItem* SfxHyphenRegionItem::Clone( SfxItemPool* ) const
{
    return new SfxHyphenRegionItem( *this );
}



bool SfxHyphenRegionItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          ,
    SfxMapUnit          ,
    OUString&           rText,
    const IntlWrapper*
)   const
{
    rText = rText +
            EE_RESSTR(RID_SVXITEMS_HYPHEN_MINLEAD).replaceAll("%1", OUString::number(nMinLead)) +
            "," +
            EE_RESSTR(RID_SVXITEMS_HYPHEN_MINTRAIL).replaceAll("%1", OUString::number(nMinTrail));
    return true;
}



SfxPoolItem* SfxHyphenRegionItem::Create(SvStream& rStrm, sal_uInt16 ) const
{
    sal_uInt8 _nMinLead, _nMinTrail;
    rStrm.ReadUChar( _nMinLead ).ReadUChar( _nMinTrail );
    SfxHyphenRegionItem* pAttr = new SfxHyphenRegionItem( Which() );
    pAttr->GetMinLead() = _nMinLead;
    pAttr->GetMinTrail() = _nMinTrail;
    return pAttr;
}



SvStream& SfxHyphenRegionItem::Store( SvStream& rStrm, sal_uInt16 ) const
{
    rStrm.WriteUChar( GetMinLead() )
         .WriteUChar( GetMinTrail() );
    return rStrm;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
