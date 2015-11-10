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

#include "tokenmap.hxx"
#include <string.h>

namespace svgi
{

#if defined __clang__
#if __has_warning("-Wdeprecated-register")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-register"
#endif
#endif
#include "tokens.cxx"
#if defined __clang__
#if __has_warning("-Wdeprecated-register")
#pragma GCC diagnostic pop
#endif
#endif

sal_Int32 getTokenId( const char* sIdent, sal_Int32 nLen )
{
    const struct xmltoken* t = Perfect_Hash::in_word_set( sIdent, nLen );
    if( t )
        return t->nToken;
    else
        return XML_TOKEN_INVALID;
}

sal_Int32 getTokenId( const OUString& sIdent )
{
    OString aUTF8( sIdent.getStr(),
                        sIdent.getLength(),
                        RTL_TEXTENCODING_UTF8 );
    return getTokenId( aUTF8.getStr(), aUTF8.getLength() );
}

const char* getTokenName( sal_Int32 nTokenId )
{
    if( nTokenId >= XML_TOKEN_COUNT )
        return nullptr;

    const xmltoken* pCurr=wordlist;
    const xmltoken* pEnd=wordlist+sizeof(wordlist)/sizeof(*wordlist);
    while( pCurr != pEnd )
    {
        if(pCurr->nToken == nTokenId)
            return pCurr->name;
        ++pCurr;
    }

    return nullptr;
}

} // namespace svgi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
