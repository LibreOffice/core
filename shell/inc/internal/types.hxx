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

#ifndef TYPES_HXX_INCLUDED
#define TYPES_HXX_INCLUDED

#include <string>
#include <map>
#include <utility>
#include <vector>
#include <stack>

typedef std::vector<std::wstring> StringList_t;

//+-------------------------------------------------------------------------
//  Declare:    XmlTagAttributes_t, xml tag attribute struct
//              XmlTag_t, xml tag including content and attributes.
//              XmlTags_t, tags defined with tag name and xml tag.
//  Contents:   Definitions of xml tag used in parser.
//--------------------------------------------------------------------------
typedef std::wstring Name_t;
typedef std::wstring Value_t;
typedef std::wstring Characters_t;

typedef std::map<Name_t, Value_t>                   XmlTagAttributes_t;
typedef std::pair<Characters_t, XmlTagAttributes_t> XmlTag_t;
typedef std::map<Name_t, XmlTag_t>                  XmlTags_t;

const XmlTag_t EMPTY_XML_TAG = std::make_pair(std::wstring(), XmlTagAttributes_t());

//+-------------------------------------------------------------------------
//  Declare:    Language_t, language of the Locale pair
//              Country_t, country of the Local pair
//              LocaleSet_t, Local pair
//  Contents:   Definitions of Chunk properties.
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
//  Declare:    StyleName_t, style name of a style-locale pair.
//              StyleLocaleMap, the map of Styple-Locale pair.
//  Contents:   Definitions of Style Names.
//--------------------------------------------------------------------------
typedef ::std::wstring StyleName_t;
typedef ::std::pair <StyleName_t, LocaleSet_t> StyleLocalePair_t;
typedef ::std::map<StyleName_t, LocaleSet_t>  StyleLocaleMap_t;

const StyleLocalePair_t EMPTY_STYLELOCALE_PAIR = ::std::make_pair(::std::wstring(), EMPTY_LOCALE );

class StreamInterface
{
public:
    virtual ~StreamInterface() {}
    virtual unsigned long sread (unsigned char* vuf, unsigned long size) = 0;
    virtual long stell () = 0;
    virtual long sseek (long offset, int origin) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
