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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <tools/stream.hxx>


#include <svx/pageitem.hxx>
#include <editeng/itemtype.hxx>
#include <svx/unomid.hxx>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <svl/itemset.hxx>
#include <svx/svxitems.hrc>
#include <svx/dialmgr.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;

// STATIC DATA -----------------------------------------------------------

TYPEINIT1_FACTORY( SvxPageItem, SfxPoolItem , new  SvxPageItem(0));

/*--------------------------------------------------------------------
    Beschreibung: Konstruktor
 --------------------------------------------------------------------*/

SvxPageItem::SvxPageItem( const USHORT nId ) : SfxPoolItem( nId ),

    eNumType    ( SVX_ARABIC ),
    bLandscape  ( sal_False ),
    eUse        ( SVX_PAGE_ALL )
{
}

/*--------------------------------------------------------------------
    Beschreibung: Copy-Konstruktor
 --------------------------------------------------------------------*/

SvxPageItem::SvxPageItem( const SvxPageItem& rItem )
    : SfxPoolItem( rItem )
{
    eNumType    = rItem.eNumType;
    bLandscape  = rItem.bLandscape;
    eUse        = rItem.eUse;
}

/*--------------------------------------------------------------------
    Beschreibung: Clonen
 --------------------------------------------------------------------*/

SfxPoolItem* SvxPageItem::Clone( SfxItemPool * ) const
{
    return new SvxPageItem( *this );
}

/*--------------------------------------------------------------------
    Beschreibung: Abfrage auf Gleichheit
 --------------------------------------------------------------------*/

int SvxPageItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
    const SvxPageItem& rItem = (SvxPageItem&)rAttr;
    return ( eNumType   == rItem.eNumType   &&
             bLandscape == rItem.bLandscape &&
             eUse       == rItem.eUse );
}

inline XubString GetUsageText( const USHORT eU )
{
    if ( eU & SVX_PAGE_LEFT )
        return SVX_RESSTR(RID_SVXITEMS_PAGE_USAGE_LEFT);
    if ( eU & SVX_PAGE_RIGHT )
        return SVX_RESSTR(RID_SVXITEMS_PAGE_USAGE_RIGHT);
    if ( eU & SVX_PAGE_ALL )
        return SVX_RESSTR(RID_SVXITEMS_PAGE_USAGE_ALL);
    if ( eU & SVX_PAGE_MIRROR )
        return SVX_RESSTR(RID_SVXITEMS_PAGE_USAGE_MIRROR);
    return String();
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPageItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            if ( aDescName.Len() )
            {
                rText = aDescName;
                rText += cpDelim;
            }
            DBG_ASSERT( eNumType <= SVX_NUMBER_NONE, "enum overflow" );
            rText += SVX_RESSTR(RID_SVXITEMS_PAGE_NUM_BEGIN + eNumType);
            rText += cpDelim;
            if ( bLandscape )
                rText += SVX_RESSTR(RID_SVXITEMS_PAGE_LAND_TRUE);
            else
                rText += SVX_RESSTR(RID_SVXITEMS_PAGE_LAND_FALSE);
            rText += GetUsageText( eUse );
            return SFX_ITEM_PRESENTATION_NAMELESS;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText += SVX_RESSTR(RID_SVXITEMS_PAGE_COMPLETE);
            if ( aDescName.Len() )
            {
                rText += aDescName;
                rText += cpDelim;
            }
            DBG_ASSERT( eNumType <= SVX_NUMBER_NONE, "enum overflow" );
            rText += SVX_RESSTR(RID_SVXITEMS_PAGE_NUM_BEGIN + eNumType);
            rText += cpDelim;
            if ( bLandscape )
                rText += SVX_RESSTR(RID_SVXITEMS_PAGE_LAND_TRUE);
            else
                rText += SVX_RESSTR(RID_SVXITEMS_PAGE_LAND_FALSE);
            rText += GetUsageText( eUse );
            return SFX_ITEM_PRESENTATION_COMPLETE;
        }
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------
bool SvxPageItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_PAGE_NUMTYPE:
        {
            //! die Konstanten sind nicht mehr in den IDLs ?!?
            rVal <<= (sal_Int16)( eNumType );
        }
        break;
        case MID_PAGE_ORIENTATION:
            //Landscape= sal_True
            rVal = Bool2Any(bLandscape);
        break;
        case MID_PAGE_LAYOUT     :
        {
            style::PageStyleLayout eRet;
            switch(eUse & 0x0f)
            {
                case SVX_PAGE_LEFT  : eRet = style::PageStyleLayout_LEFT;      break;
                case SVX_PAGE_RIGHT : eRet = style::PageStyleLayout_RIGHT;     break;
                case SVX_PAGE_ALL   : eRet = style::PageStyleLayout_ALL;       break;
                case SVX_PAGE_MIRROR: eRet = style::PageStyleLayout_MIRRORED; break;
                default:
                    OSL_FAIL("was fuer ein Layout ist das?");
                    return sal_False;
            }
            rVal <<= eRet;
        }
        break;
    }

    return true;
}
//------------------------------------------------------------------------
bool SvxPageItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    switch( nMemberId )
    {
        case MID_PAGE_NUMTYPE:
        {
            sal_Int32 nValue = 0;
            if(!(rVal >>= nValue))
                return false;

            eNumType = (SvxNumType)nValue;
        }
        break;
        case MID_PAGE_ORIENTATION:
            bLandscape = Any2Bool(rVal);
        break;
        case MID_PAGE_LAYOUT     :
        {
            style::PageStyleLayout eLayout;
            if(!(rVal >>= eLayout))
            {
                sal_Int32 nValue = 0;
                if(!(rVal >>= nValue))
                    return sal_False;
                eLayout = (style::PageStyleLayout)nValue;
            }
            eUse &= 0xfff0;
            switch( eLayout )
            {
                case style::PageStyleLayout_LEFT     : eUse |= SVX_PAGE_LEFT ; break;
                case style::PageStyleLayout_RIGHT   : eUse |= SVX_PAGE_RIGHT; break;
                case style::PageStyleLayout_ALL     : eUse |= SVX_PAGE_ALL  ; break;
                case style::PageStyleLayout_MIRRORED: eUse |= SVX_PAGE_MIRROR;break;
                default: ;//prevent warning
            }
        }
        break;
    }
    return true;
}

//------------------------------------------------------------------------

SfxPoolItem* SvxPageItem::Create( SvStream& rStream, USHORT ) const
{
    XubString sStr;
    BYTE eType;
    sal_Bool bLand;
    USHORT nUse;

    // UNICODE: rStream >> sStr;
    rStream.ReadByteString( sStr );

    rStream >> eType;
    rStream >> bLand;
    rStream >> nUse;

    SvxPageItem* pPage = new SvxPageItem( Which() );
    pPage->SetDescName( sStr );
    pPage->SetNumType( (SvxNumType)eType );
    pPage->SetLandscape( bLand );
    pPage->SetPageUsage( nUse );
    return pPage;
}

//------------------------------------------------------------------------

SvStream& SvxPageItem::Store( SvStream &rStrm, USHORT /*nItemVersion*/ ) const
{
    // UNICODE: rStrm << aDescName;
    rStrm.WriteByteString(aDescName);

    rStrm << (BYTE)eNumType << bLandscape << eUse;
    return rStrm;
}

/*--------------------------------------------------------------------
    Beschreibung:   HeaderFooterSet
 --------------------------------------------------------------------*/

SvxSetItem::SvxSetItem( const USHORT nId, const SfxItemSet& rSet ) :

    SfxSetItem( nId, rSet )
{
}

SvxSetItem::SvxSetItem( const SvxSetItem& rItem ) :

    SfxSetItem( rItem )
{
}

SvxSetItem::SvxSetItem( const USHORT nId, SfxItemSet* _pSet ) :

    SfxSetItem( nId, _pSet )
{
}

SfxPoolItem* SvxSetItem::Clone( SfxItemPool * ) const
{
    return new SvxSetItem(*this);
}

//------------------------------------------------------------------------

SfxItemPresentation SvxSetItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

SfxPoolItem* SvxSetItem::Create(SvStream &rStrm, USHORT /*nVersion*/) const
{
    SfxItemSet* _pSet = new SfxItemSet( *GetItemSet().GetPool(),
                                       GetItemSet().GetRanges() );

    _pSet->Load( rStrm );

    return new SvxSetItem( Which(), *_pSet );
}

SvStream& SvxSetItem::Store(SvStream &rStrm, USHORT nItemVersion) const
{
    GetItemSet().Store( rStrm, nItemVersion );

    return rStrm;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
