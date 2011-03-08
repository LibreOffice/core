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

#ifndef TYPES_HXX_INCLUDED
#define TYPES_HXX_INCLUDED

#include <string>
#include <map>
#include <utility>
#include <vector>
#include <stack>
#include <external/zlib/zlib.h>
#include <external/zlib/ioapi.h>


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
