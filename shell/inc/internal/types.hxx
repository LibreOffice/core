/*************************************************************************
 *
 *  $RCSfile: types.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-09-08 14:24:41 $
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
