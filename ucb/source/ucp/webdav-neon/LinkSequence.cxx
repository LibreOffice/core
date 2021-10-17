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

#include <config_lgpl.h>
#include <string.h>
#include <ne_xml.h>
#include <memory>

#include "LinkSequence.hxx"

using namespace webdav_ucp;
using namespace com::sun::star;

namespace {

struct LinkSequenceParseContext
{
    std::unique_ptr<ucb::Link> pLink;
    bool hasSource;
    bool hasDestination;

    LinkSequenceParseContext()
    : hasSource( false ), hasDestination( false ) {}
};

}

#define STATE_TOP (1)

#define STATE_LINK (STATE_TOP)
#define STATE_DST  (STATE_TOP + 1)
#define STATE_SRC  (STATE_TOP + 2)


extern "C" {

static int LinkSequence_startelement_callback(
    void *,
    int parent,
    const char * /*nspace*/,
    const char *name,
    const char ** )
{
    if ( name != nullptr )
    {
        switch ( parent )
        {
            case NE_XML_STATEROOT:
                if ( strcmp( name, "link" ) == 0 )
                    return STATE_LINK;
                break;

            case STATE_LINK:
                if ( strcmp( name, "dst" ) == 0 )
                    return STATE_DST;
                else if ( strcmp( name, "src" ) == 0 )
                    return STATE_SRC;
                break;
        }
    }
    return NE_XML_DECLINE;
}


static int LinkSequence_chardata_callback(
    void *userdata,
    int state,
    const char *buf,
    size_t len )
{
    LinkSequenceParseContext * pCtx
                    = static_cast< LinkSequenceParseContext * >( userdata );
    if ( !pCtx->pLink )
        pCtx->pLink.reset( new ucb::Link );

    switch ( state )
    {
        case STATE_DST:
            pCtx->pLink->Destination
                = OUString( buf, len, RTL_TEXTENCODING_ASCII_US );
            pCtx->hasDestination = true;
            break;

        case STATE_SRC:
            pCtx->pLink->Source
                = OUString( buf, len, RTL_TEXTENCODING_ASCII_US );
            pCtx->hasSource = true;
            break;
    }
    return 0; // zero to continue, non-zero to abort parsing
}


static int LinkSequence_endelement_callback(
    void *userdata,
    int state,
    const char *,
    const char * )
{
    LinkSequenceParseContext * pCtx
                    = static_cast< LinkSequenceParseContext * >( userdata );
    if ( !pCtx->pLink )
        pCtx->pLink.reset( new ucb::Link );

    switch ( state )
    {
        case STATE_LINK:
            if ( !pCtx->hasDestination || !pCtx->hasSource )
                return 1; // abort
            break;
    }
    return 0; // zero to continue, non-zero to abort parsing
}

}

// static
bool LinkSequence::createFromXML( const OString & rInData,
                                  uno::Sequence< ucb::Link > & rOutData )
{
    const sal_Int32 TOKEN_LENGTH = 7; // </link>
    bool success = true;
    auto rOutDataRange = asNonConstRange(rOutData);

    // rInData may contain multiple <link>...</link> tags.
    sal_Int32 nCount = 0;
    sal_Int32 nStart = 0;
    sal_Int32 nEnd   = rInData.indexOf( "</link>" );
    while ( nEnd > -1 )
    {
        ne_xml_parser * parser = ne_xml_create();
        if ( !parser )
        {
            success = false;
            break;
        }

        LinkSequenceParseContext aCtx;
        ne_xml_push_handler( parser,
                             LinkSequence_startelement_callback,
                             LinkSequence_chardata_callback,
                             LinkSequence_endelement_callback,
                             &aCtx );

        ne_xml_parse( parser,
                      rInData.getStr() + nStart,
                      nEnd - nStart + TOKEN_LENGTH );

        success = !ne_xml_failed( parser );

        ne_xml_destroy( parser );

        if ( !success )
            break;

        if ( aCtx.pLink )
        {
            nCount++;
            if ( nCount > rOutData.getLength() )
            {
                rOutData.realloc( rOutData.getLength() + 1 );
                rOutDataRange = asNonConstRange(rOutData);
            }

            rOutDataRange[ nCount - 1 ] = *aCtx.pLink;
        }

        nStart = nEnd + TOKEN_LENGTH;
        nEnd   = rInData.indexOf( "</link>", nStart );
    }

    return success;
}


// static
bool LinkSequence::toXML( const uno::Sequence< ucb::Link > & rInData,
                          OUString & rOutData )
{
    // <link><src>value</src><dst>value</dst></link><link><src>...

    for ( const auto& rLink : rInData )
    {
        rOutData += "<link><src>";
        rOutData += rLink.Source;
        rOutData += "</src><dst>";
        rOutData += rLink.Destination;
        rOutData += "</dst></link>";
    }
    return rInData.hasElements();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
