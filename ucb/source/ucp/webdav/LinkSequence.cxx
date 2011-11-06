/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <string.h>
#include <ne_xml.h>

#ifndef _LINKSEQUENCE_HXX_
#include "LinkSequence.hxx"
#endif

using namespace webdav_ucp;
using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////

struct LinkSequenceParseContext
{
    ucb::Link * pLink;
    bool hasSource;
    bool hasDestination;

    LinkSequenceParseContext()
    : pLink( 0 ), hasSource( false ), hasDestination( false ) {}
    ~LinkSequenceParseContext() { delete pLink; }
};

#define STATE_TOP (1)

#define STATE_LINK (STATE_TOP)
#define STATE_DST  (STATE_TOP + 1)
#define STATE_SRC  (STATE_TOP + 2)

//////////////////////////////////////////////////////////////////////////
extern "C" int LinkSequence_startelement_callback(
    void *,
    int parent,
    const char * /*nspace*/,
    const char *name,
    const char ** )
{
    if ( name != 0 )
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

//////////////////////////////////////////////////////////////////////////
extern "C" int LinkSequence_chardata_callback(
    void *userdata,
    int state,
    const char *buf,
    size_t len )
{
    LinkSequenceParseContext * pCtx
                    = static_cast< LinkSequenceParseContext * >( userdata );
    if ( !pCtx->pLink )
        pCtx->pLink = new ucb::Link;

    switch ( state )
    {
        case STATE_DST:
            pCtx->pLink->Destination
                = rtl::OUString( buf, len, RTL_TEXTENCODING_ASCII_US );
            pCtx->hasDestination = true;
            break;

        case STATE_SRC:
            pCtx->pLink->Source
                = rtl::OUString( buf, len, RTL_TEXTENCODING_ASCII_US );
            pCtx->hasSource = true;
            break;
    }
    return 0; // zero to continue, non-zero to abort parsing
}

//////////////////////////////////////////////////////////////////////////
extern "C" int LinkSequence_endelement_callback(
    void *userdata,
    int state,
    const char *,
    const char * )
{
    LinkSequenceParseContext * pCtx
                    = static_cast< LinkSequenceParseContext * >( userdata );
    if ( !pCtx->pLink )
        pCtx->pLink = new ucb::Link;

    switch ( state )
    {
        case STATE_LINK:
            if ( !pCtx->hasDestination || !pCtx->hasSource )
                return 1; // abort
            break;
    }
    return 0; // zero to continue, non-zero to abort parsing
}

//////////////////////////////////////////////////////////////////////////
// static
bool LinkSequence::createFromXML( const rtl::OString & rInData,
                                  uno::Sequence< ucb::Link > & rOutData )
{
    const sal_Int32 TOKEN_LENGTH = 7; // </link>
    bool success = true;

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
                rOutData.realloc( rOutData.getLength() + 1 );

            rOutData[ nCount - 1 ] = *aCtx.pLink;
        }

        nStart = nEnd + TOKEN_LENGTH;
        nEnd   = rInData.indexOf( "</link>", nStart );
    }

    return success;
}

//////////////////////////////////////////////////////////////////////////
// static
bool LinkSequence::toXML( const uno::Sequence< ucb::Link > & rInData,
                          rtl::OUString & rOutData )
{
    // <link><src>value</src><dst>value</dst></link><link><src>....

    sal_Int32 nCount = rInData.getLength();
    if ( nCount )
    {
        rtl::OUString aPre( rtl::OUString::createFromAscii( "<link><src>" ) );
        rtl::OUString aMid( rtl::OUString::createFromAscii( "</src><dst>" ) );
        rtl::OUString aEnd( rtl::OUString::createFromAscii( "</dst></link>" ) );

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
                rOutData += aPre;
                rOutData += rInData[ n ].Source;
                rOutData += aMid;
                rOutData += rInData[ n ].Destination;
                rOutData += aEnd;
        }
        return true;
    }
    return false;
}
