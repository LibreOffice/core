/*************************************************************************
 *
 *  $RCSfile: srchitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:29 $
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

// include ---------------------------------------------------------------

#ifndef __SBX_SBXVARIABLE_HXX
#include <svtools/sbxvar.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XREPLACEABLE_HPP_
#include <com/sun/star/util/XReplaceable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSEARCHABLE_HPP_
#include <com/sun/star/util/XSearchable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSEARCHDESCRIPTOR_HPP_
#include <com/sun/star/util/XSearchDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XPROPERTYREPLACE_HPP_
#include <com/sun/star/util/XPropertyReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREPLACEDESCRIPTOR_HPP_
#include <com/sun/star/util/XReplaceDescriptor.hpp>
#endif
#include <svtools/memberid.hrc>
//ASDBG #include <usr/uno.hxx>

#pragma hdrstop

#define _SVX_SRCHITEM_CXX

#include "sfxsids.hrc"
#define ITEMID_SEARCH   SID_SEARCH_ITEM
#include "srchitem.hxx"

#include <sfxuno.hxx>

// STATIC DATA -----------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxSearchItem, SfxPoolItem);

// -----------------------------------------------------------------------

SvxSearchItem::SvxSearchItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    nCommand        ( 0 ),
    aSearchString   () ,
    aReplaceString  (),
    bWordOnly       ( sal_False ),
    bExact          ( sal_False ),
    bBackward       ( sal_False ),
    bSelection      ( sal_False ),
    bRegExp         ( sal_False ),
    bPattern        ( sal_False ),
    bContent        ( sal_False ),
    eFamily         ( SFX_STYLE_FAMILY_PARA ),
    bRowDirection   ( sal_True ),
    bAllTables      ( sal_False ),
    nCellType       ( SVX_SEARCHIN_FORMULA ),
    nAppFlag        ( SVX_SEARCHAPP_WRITER ),
    bLevenshtein    ( sal_False ),
    bLEVRelaxed     ( sal_True ),
    nLEVOther       ( 2 ),
    nLEVShorter     ( 2 ),
    nLEVLonger      ( 2 )
{
}

// -----------------------------------------------------------------------

SvxSearchItem::SvxSearchItem( const SvxSearchItem& rItem ) :

    SfxPoolItem ( rItem ),

    nCommand        ( rItem.nCommand ),
    aSearchString   ( rItem.aSearchString ),
    aReplaceString  ( rItem.aReplaceString ),
    bWordOnly       ( rItem.bWordOnly ),
    bExact          ( rItem.bExact ),
    bBackward       ( rItem.bBackward ),
    bSelection      ( rItem.bSelection ),
    bRegExp         ( rItem.bRegExp ),
    bPattern        ( rItem.bPattern ),
    bContent        ( rItem.bContent ),
    eFamily         ( rItem.eFamily ),
    bRowDirection   ( rItem.bRowDirection ),
    bAllTables      ( rItem.bAllTables ),
    nCellType       ( rItem.nCellType ),
    nAppFlag        ( rItem.nAppFlag ),
    bLevenshtein    ( rItem.bLevenshtein  ),
    bLEVRelaxed     ( rItem.bLEVRelaxed  ),
    nLEVOther       ( rItem.nLEVOther     ),
    nLEVShorter     ( rItem.nLEVShorter ),
    nLEVLonger      ( rItem.nLEVLonger )
{
}

// -----------------------------------------------------------------------

int SvxSearchItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal which or type" );

    return ( nCommand       == ( (SvxSearchItem&)rItem ).nCommand )       &&
           ( aSearchString  == ( (SvxSearchItem&)rItem ).aSearchString )  &&
           ( aReplaceString == ( (SvxSearchItem&)rItem ).aReplaceString ) &&
           ( bWordOnly      == ( (SvxSearchItem&)rItem ).bWordOnly )      &&
           ( bExact         == ( (SvxSearchItem&)rItem ).bExact )         &&
           ( bBackward      == ( (SvxSearchItem&)rItem ).bBackward )      &&
           ( bSelection     == ( (SvxSearchItem&)rItem ).bSelection )     &&
           ( bRegExp        == ( (SvxSearchItem&)rItem ).bRegExp )        &&
           ( bPattern       == ( (SvxSearchItem&)rItem ).bPattern )       &&
           ( bContent       == ( (SvxSearchItem&)rItem ).bContent )       &&
           ( eFamily        == ( (SvxSearchItem&)rItem ).eFamily )        &&
           ( bRowDirection  == ( (SvxSearchItem&)rItem ).bRowDirection )  &&
           ( bAllTables     == ( (SvxSearchItem&)rItem ).bAllTables )     &&
           ( nCellType      == ( (SvxSearchItem&)rItem ).nCellType )      &&
           ( nAppFlag       == ( (SvxSearchItem&)rItem ).nAppFlag )       &&
           ( bLevenshtein   == ( (SvxSearchItem&)rItem ).bLevenshtein  )  &&
           ( bLEVRelaxed    == ( (SvxSearchItem&)rItem ).bLEVRelaxed  )   &&
           ( nLEVOther      == ( (SvxSearchItem&)rItem ).nLEVOther   )    &&
           ( nLEVShorter    == ( (SvxSearchItem&)rItem ).nLEVShorter )    &&
           ( nLEVLonger     == ( (SvxSearchItem&)rItem ).nLEVLonger  );
}


//------------------------------------------------------------------------

SfxItemPresentation SvxSearchItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText,
    const International *
)   const
{
/*
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = SVX_RESSTR(RID_SVXITEMS_SEARCHCMD_BEGIN + nCommand);
            rText += cpDelim;
            rText += aSearchString;
            rText += cpDelim;
            rText += aReplaceString;
            rText += cpDelim;
            rText += ::GetBoolString( bWordOnly );
            rText += cpDelim;
            rText += ::GetBoolString( bExact );
            rText += cpDelim;
            rText += ::GetBoolString( bBackward );
            rText += cpDelim;
            rText += ::GetBoolString( bSelection );
            rText += cpDelim;
            rText += ::GetBoolString( bRegExp );
            rText += cpDelim;
            rText += ::GetBoolString( bPattern );
            rText += cpDelim;
            rText += ::GetBoolString( bContent );
            rText += cpDelim;
            rText += SVX_RESSTR(RID_SVXITEMS_SEARCHSTYLE_BEGIN + (sal_uInt16)eFamily );
            rText += cpDelim;
            rText += ::GetBoolString( bRowDirection );
            rText += cpDelim;
            rText += ::GetBoolString( bAllTables );
            rText += cpDelim;
            rText += SVX_RESSTR(RID_SVXITEMS_SEARCHIN_BEGIN + nCellType);
            rText += cpDelim;
            rText += SVX_RESSTR(RID_SVXITEMS_SEARCHAPP_BEGIN + nAppFlag);
            return ePres;
        }
    }
 */
    return SFX_ITEM_PRESENTATION_NONE;
}

void SvxSearchItem::GetFromDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor >& rDescr )
{
    SetSearchString( rDescr->getSearchString() );
    ::com::sun::star::uno::Any aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchWords") );
    sal_Bool bTemp ;
    aAny >>= bTemp ;
    SetWordOnly( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchCaseSensitive") );
    aAny >>= bTemp ;
    SetExact( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchBackwards") );
    aAny >>= bTemp ;
    SetBackward( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchInSelection") );
    aAny >>= bTemp ;
    SetSelection( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchRegularExpression") );
    aAny >>= bTemp ;
    SetRegExp( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarity") );
    aAny >>= bTemp ;
    SetLevenshtein( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityRelax") );
    aAny >>= bTemp ;
    SetLEVRelaxed( bTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityExchange") );
    sal_Int16 nTemp ;
    aAny >>= nTemp ;
    SetLEVOther( nTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityRemove") );
    aAny >>= nTemp ;
    SetLEVShorter( nTemp );
    aAny = rDescr->getPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityAdd") );
    aAny >>= nTemp ;
    SetLEVLonger( nTemp );
}

void SvxSearchItem::SetToDescriptor( ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor > & rDescr )
{
    rDescr->setSearchString( GetSearchString() );
    ::com::sun::star::uno::Any aAny;
    aAny <<= GetWordOnly() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchWords"), aAny );
    aAny <<= GetExact() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchCaseSensitive"), aAny );
    aAny <<= GetBackward() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchBackwards"), aAny );
    aAny <<= GetSelection() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchInSelection"), aAny );
    aAny <<= GetRegExp() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchRegularExpression"), aAny );
    aAny <<= IsLevenshtein() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarity"), aAny );
    aAny <<= IsLEVRelaxed() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityRelax"), aAny );
    aAny <<= GetLEVOther() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityExchange"), aAny );
    aAny <<= GetLEVShorter() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityRemove"), aAny );
    aAny <<= GetLEVLonger() ;
    rDescr->setPropertyValue( DEFINE_CONST_UNICODE("SearchSimilarityAdd"), aAny );
}


