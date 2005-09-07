/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: types.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:39:31 $
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

#ifndef TYPES_HXX_INCLUDED
#define TYPES_HXX_INCLUDED

#include <string>
#include <map>
#include <utility>
#include <vector>
#include <stack>

typedef std::vector<std::wstring> StringList_t;

//+-------------------------------------------------------------------------
//
//  Declare:    XmlTagAttributes_t, xml tag attribute struct
//              XmlTag_t, xml tag including content and attributes.
//              XmlTags_t, tags defined with tag name and xml tag.
//
//  Contents:   Definitions of xml tag used in parser.
//
//--------------------------------------------------------------------------
typedef std::wstring Name_t;
typedef std::wstring Value_t;
typedef std::wstring Characters_t;

typedef std::map<Name_t, Value_t>                   XmlTagAttributes_t;
typedef std::pair<Characters_t, XmlTagAttributes_t> XmlTag_t;
typedef std::map<Name_t, XmlTag_t>                  XmlTags_t;

const XmlTag_t EMPTY_XML_TAG = std::make_pair(std::wstring(), XmlTagAttributes_t());

//+-------------------------------------------------------------------------
//
//  Declare:    Language_t, language of the Locale pair
//              Country_t, country of the Local pair
//              LocaleSet_t, Local pair
//
//  Contents:   Definitions of Chunk properties.
//
//--------------------------------------------------------------------------
typedef ::std::wstring Language_t;
typedef ::std::wstring Country_t;
typedef ::std::pair<Language_t, Country_t >   LocaleSet_t;

typedef ::std::wstring Content_t;
typedef ::std::pair<LocaleSet_t, Content_t > Chunk_t;
typedef ::std::vector< Chunk_t > ChunkBuffer_t;

const LocaleSet_t EMPTY_LOCALE = ::std::make_pair(::std::wstring(), ::std::wstring());
const Chunk_t EMPTY_CHUNK = ::std::make_pair( EMPTY_LOCALE, ::std::wstring());

//+-------------------------------------------------------------------------
//
//  Declare:    StyleName_t, style name of a style-locale pair.
//              StyleLocaleMap, the map of Styple-Locale pair.
//
//  Contents:   Definitions of Style Names.
//
//--------------------------------------------------------------------------
typedef ::std::wstring StyleName_t;
typedef ::std::pair <StyleName_t, LocaleSet_t> StyleLocalePair_t;
typedef ::std::map<StyleName_t, LocaleSet_t>  StyleLocaleMap_t;

const StyleLocalePair_t EMPTY_STYLELOCALE_PAIR = ::std::make_pair(::std::wstring(), EMPTY_LOCALE );

#endif
