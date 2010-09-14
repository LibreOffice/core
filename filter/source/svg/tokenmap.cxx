/*************************************************************************
 *
 *    OpenOffice.org - a multi-platform office productivity suite
 *
 *    Author:
 *      Fridrich Strba  <fridrich.strba@bluewin.ch>
 *      Thorsten Behrens <tbehrens@novell.com>
 *
 *      Copyright (C) 2008, Novell Inc.
 *      Parts copyright 2005 by Sun Microsystems, Inc.
 *
 *   The Contents of this file are made available subject to
 *   the terms of GNU Lesser General Public License Version 3.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "tokenmap.hxx"
#include <string.h>

namespace svgi
{

#include "tokens.cxx"

sal_Int32 getTokenId( const char* sIdent, sal_Int32 nLen )
{
    const struct xmltoken* t = Perfect_Hash::in_word_set( sIdent, nLen );
    if( t )
        return t->nToken;
    else
        return XML_TOKEN_INVALID;
}

sal_Int32 getTokenId( const rtl::OUString& sIdent )
{
    rtl::OString aUTF8( sIdent.getStr(),
                        sIdent.getLength(),
                        RTL_TEXTENCODING_UTF8 );
    return getTokenId( aUTF8.getStr(), aUTF8.getLength() );
}

const char* getTokenName( sal_Int32 nTokenId )
{
    if( nTokenId >= XML_TOKEN_COUNT )
        return NULL;

    const xmltoken* pCurr=wordlist;
    const xmltoken* pEnd=wordlist+sizeof(wordlist)/sizeof(*wordlist);
    while( pCurr != pEnd )
    {
        if(pCurr->nToken == nTokenId)
            return pCurr->name;
        ++pCurr;
    }

    return NULL;
}

} // namespace svgi
