/*************************************************************************
 *
 *  $RCSfile: srchitem.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tl $ $Date: 2001-02-19 10:55:58 $
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
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#include <svtools/memberid.hrc>
//ASDBG #include <usr/uno.hxx>

#pragma hdrstop

#define _SVX_SRCHITEM_CXX

#include "sfxsids.hrc"
#define ITEMID_SEARCH   SID_SEARCH_ITEM
#include "srchitem.hxx"

#include <sfxuno.hxx>

//using namespace uno;
using namespace rtl;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;


// STATIC DATA -----------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxSearchItem, SfxPoolItem);

// -----------------------------------------------------------------------

SvxSearchItem::SvxSearchItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    aSearchOpt      (   SearchAlgorithms_ABSOLUTE,
                        SearchFlags::ALL_IGNORE_CASE | SearchFlags::LEV_RELAXED,
                          OUString(),
                          OUString(),
                          Locale(),
                          2, 2, 2,
                          0x00000000 ),
    nCommand        ( 0 ),
    bBackward       ( sal_False ),
    bPattern        ( sal_False ),
    bContent        ( sal_False ),
    eFamily         ( SFX_STYLE_FAMILY_PARA ),
    bRowDirection   ( sal_True ),
    bAllTables      ( sal_False ),
    nCellType       ( SVX_SEARCHIN_FORMULA ),
    nAppFlag        ( SVX_SEARCHAPP_WRITER )
{
}

// -----------------------------------------------------------------------

SvxSearchItem::SvxSearchItem( const SvxSearchItem& rItem ) :

    SfxPoolItem ( rItem ),

    aSearchOpt      ( rItem.aSearchOpt ),
    nCommand        ( rItem.nCommand ),
    bBackward       ( rItem.bBackward ),
    bPattern        ( rItem.bPattern ),
    bContent        ( rItem.bContent ),
    eFamily         ( rItem.eFamily ),
    bRowDirection   ( rItem.bRowDirection ),
    bAllTables      ( rItem.bAllTables ),
    nCellType       ( rItem.nCellType ),
    nAppFlag        ( rItem.nAppFlag )
{
}

// -----------------------------------------------------------------------
SfxPoolItem* SvxSearchItem::Clone( SfxItemPool *pPool) const
{
    return new SvxSearchItem(*this);
}

// -----------------------------------------------------------------------

//! used below
static BOOL operator == ( const Locale& rItem1, const Locale& rItem2 )
{
    return rItem1.Language  == rItem2.Language  &&
           rItem1.Country   == rItem2.Country   &&
           rItem1.Variant   == rItem2.Variant;
}

//! used below
static BOOL operator == ( const SearchOptions& rItem1, const SearchOptions& rItem2 )
{
    return rItem1.algorithmType         == rItem2.algorithmType &&
           rItem1.searchFlag            == rItem2.searchFlag    &&
           rItem1.searchString          == rItem2.searchString  &&
           rItem1.replaceString         == rItem2.replaceString &&
           rItem1.Locale                == rItem2.Locale        &&
           rItem1.changedChars          == rItem2.changedChars  &&
           rItem1.deletedChars          == rItem2.deletedChars  &&
           rItem1.insertedChars         == rItem2.insertedChars &&
           rItem1.transliterateFlags    == rItem2.transliterateFlags;
}


int SvxSearchItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal which or type" );
    const SvxSearchItem &rSItem = (SvxSearchItem &) rItem;
    return ( nCommand       == rSItem.nCommand )        &&
           ( bBackward      == rSItem.bBackward )       &&
           ( bPattern       == rSItem.bPattern )        &&
           ( bContent       == rSItem.bContent )        &&
           ( eFamily        == rSItem.eFamily )         &&
           ( bRowDirection  == rSItem.bRowDirection )   &&
           ( bAllTables     == rSItem.bAllTables )      &&
           ( nCellType      == rSItem.nCellType )       &&
           ( nAppFlag       == rSItem.nAppFlag )        &&
           ( aSearchOpt     == rSItem.aSearchOpt );
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


void SvxSearchItem::SetMatchHalfFullWidthForms( sal_Bool bVal )
{
    if (bVal)
        aSearchOpt.transliterateFlags |=  TransliterationModules_IGNORE_WIDTH;
    else
        aSearchOpt.transliterateFlags &= ~TransliterationModules_IGNORE_WIDTH;
}


void SvxSearchItem::SetWordOnly( sal_Bool bVal )
{
    if (bVal)
        aSearchOpt.searchFlag |=  SearchFlags::NORM_WORD_ONLY;
    else
        aSearchOpt.searchFlag &= ~SearchFlags::NORM_WORD_ONLY;
}


void SvxSearchItem::SetExact( sal_Bool bVal )
{
    // Exact == !ALL_IGNORE_CASE
    if (!bVal)
        aSearchOpt.searchFlag |=  SearchFlags::ALL_IGNORE_CASE;
    else
        aSearchOpt.searchFlag &= ~SearchFlags::ALL_IGNORE_CASE;
}


void SvxSearchItem::SetSelection( sal_Bool bVal )
{
    if (bVal)
    {
        aSearchOpt.searchFlag |=  (SearchFlags::REG_NOT_BEGINOFLINE |
                                   SearchFlags::REG_NOT_ENDOFLINE);
    }
    else
    {
        aSearchOpt.searchFlag &= ~(SearchFlags::REG_NOT_BEGINOFLINE |
                                   SearchFlags::REG_NOT_ENDOFLINE);
    }
}


void SvxSearchItem::SetRegExp( sal_Bool bVal )
{
    if (bVal)
        aSearchOpt.algorithmType = SearchAlgorithms_REGEXP;
    else
        aSearchOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
}


void SvxSearchItem::SetLEVRelaxed( sal_Bool bVal )
{
    if (bVal)
        aSearchOpt.searchFlag |=  SearchFlags::LEV_RELAXED;
    else
        aSearchOpt.searchFlag &= ~SearchFlags::LEV_RELAXED;
}


void SvxSearchItem::SetLevenshtein( sal_Bool bVal )
{
    if (bVal)
        aSearchOpt.algorithmType = SearchAlgorithms_APPROXIMATE;
    else
        aSearchOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
}

