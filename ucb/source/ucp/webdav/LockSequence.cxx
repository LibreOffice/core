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
#include "LockSequence.hxx"

using namespace webdav_ucp;
using namespace com::sun::star;

#define BEEHIVE_BUGS_WORKAROUND

//////////////////////////////////////////////////////////////////////////

struct LockSequenceParseContext
{
    ucb::Lock * pLock;
    bool hasLockScope;
    bool hasLockType;
    bool hasDepth;
    bool hasHREF;
    bool hasTimeout;

    LockSequenceParseContext()
    : pLock( 0 ), hasLockScope( false ), hasLockType( false ),
      hasDepth( false ), hasHREF( false ), hasTimeout( false ) {}

    ~LockSequenceParseContext() { delete pLock; }
};

#define STATE_TOP (1)

#define STATE_ACTIVELOCK    (STATE_TOP)
#define STATE_LOCKSCOPE     (STATE_TOP + 1)
#define STATE_LOCKTYPE      (STATE_TOP + 2)
#define STATE_DEPTH         (STATE_TOP + 3)
#define STATE_OWNER         (STATE_TOP + 4)
#define STATE_TIMEOUT       (STATE_TOP + 5)
#define STATE_LOCKTOKEN     (STATE_TOP + 6)
#define STATE_EXCLUSIVE     (STATE_TOP + 7)
#define STATE_SHARED        (STATE_TOP + 8)
#define STATE_WRITE         (STATE_TOP + 9)
#define STATE_HREF          (STATE_TOP + 10)

//////////////////////////////////////////////////////////////////////////
extern "C" int LockSequence_startelement_callback(
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
                if ( strcmp( name, "activelock" ) == 0 )
                    return STATE_ACTIVELOCK;
                break;

            case STATE_ACTIVELOCK:
                if ( strcmp( name, "lockscope" ) == 0 )
                    return STATE_LOCKSCOPE;
                else if ( strcmp( name, "locktype" ) == 0 )
                    return STATE_LOCKTYPE;
                else if ( strcmp( name, "depth" ) == 0 )
                    return STATE_DEPTH;
                else if ( strcmp( name, "owner" ) == 0 )
                    return STATE_OWNER;
                else if ( strcmp( name, "timeout" ) == 0 )
                    return STATE_TIMEOUT;
                else if ( strcmp( name, "locktoken" ) == 0 )
                    return STATE_LOCKTOKEN;
                break;

            case STATE_LOCKSCOPE:
                if ( strcmp( name, "exclusive" ) == 0 )
                    return STATE_EXCLUSIVE;
                else if ( strcmp( name, "shared" ) == 0 )
                    return STATE_SHARED;
                break;

            case STATE_LOCKTYPE:
                if ( strcmp( name, "write" ) == 0 )
                    return STATE_WRITE;
                break;

            case STATE_LOCKTOKEN:
                if ( strcmp( name, "href" ) == 0 )
                    return STATE_HREF;
                break;

            case STATE_OWNER:
                // owner elem contains ANY. Accept anything; no state change.
                return STATE_OWNER;
        }
    }
    return NE_XML_DECLINE;
}

//////////////////////////////////////////////////////////////////////////
extern "C" int LockSequence_chardata_callback(
    void *userdata,
    int state,
#ifdef BEEHIVE_BUGS_WORKAROUND
    const char *buf1,
#else
    const char *buf,
#endif
    size_t len )
{
    LockSequenceParseContext * pCtx
                    = static_cast< LockSequenceParseContext * >( userdata );
    if ( !pCtx->pLock )
        pCtx->pLock = new ucb::Lock;

#ifdef BEEHIVE_BUGS_WORKAROUND
    // Beehive sends XML values containing trailing newlines.
    if ( buf1[ len - 1 ] == 0x0a )
        len--;

    char * buf = new char[ len + 1 ]();
    strncpy( buf, buf1, len );
#endif

    switch ( state )
    {
        case STATE_DEPTH:
            if ( rtl_str_compareIgnoreAsciiCase_WithLength(
                    buf, len, "0", 1 ) == 0 )
            {
                pCtx->pLock->Depth = ucb::LockDepth_ZERO;
                pCtx->hasDepth = true;
            }
            else if ( rtl_str_compareIgnoreAsciiCase_WithLength(
                    buf, len, "1", 1 ) == 0 )
            {
                pCtx->pLock->Depth = ucb::LockDepth_ONE;
                pCtx->hasDepth = true;
            }
            else if ( rtl_str_compareIgnoreAsciiCase_WithLength(
                    buf, len, "infinity", 8 ) == 0 )
            {
                pCtx->pLock->Depth = ucb::LockDepth_INFINITY;
                pCtx->hasDepth = true;
            }
            else
                OSL_ENSURE( sal_False,
                            "LockSequence_chardata_callback - Unknown depth!" );
            break;

        case STATE_OWNER:
        {
            // collect raw XML data... (owner contains ANY)
            rtl::OUString aValue;
            pCtx->pLock->Owner >>= aValue;
            aValue += rtl::OUString( buf, len, RTL_TEXTENCODING_ASCII_US );
            pCtx->pLock->Owner <<= aValue;
            break;
        }

        case STATE_TIMEOUT:
            //
            //  RFC2518, RFC2616:
            //
            //  TimeType = ("Second-" DAVTimeOutVal | "Infinite" | Other)
            //  DAVTimeOutVal = 1*digit
            //  Other = "Extend" field-value
            //  field-value = *( field-content | LWS )
            //  field-content = <the OCTETs making up the field-value
            //                  and consisting of either *TEXT or combinations
            //                  of token, separators, and quoted-string>

            if ( rtl_str_compareIgnoreAsciiCase_WithLength(
                    buf, len, "Infinite", 8 ) == 0 )
            {
                pCtx->pLock->Timeout = sal_Int64( -1 );
                pCtx->hasTimeout = true;
            }
            else if ( rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
                                            buf, len, "Second-", 7, 7 ) == 0 )
            {
                pCtx->pLock->Timeout
                    = rtl::OString( buf + 7, len - 7 ).toInt64();
                pCtx->hasTimeout = true;
            }
//          else if ( rtl_str_shortenedCompareIgnoreCase_WithLength(
//                                          buf, len, "Extend", 6, 6 ) == 0 )
//          {
//              @@@
//          }
            else
            {
                pCtx->pLock->Timeout = sal_Int64( -1 );
                pCtx->hasTimeout = true;
                OSL_ENSURE( sal_False,
                            "LockSequence_chardata_callback - Unknown timeout!" );
            }
            break;

        case STATE_HREF:
        {
            // collect hrefs.
            sal_Int32 nPos = pCtx->pLock->LockTokens.getLength();
            pCtx->pLock->LockTokens.realloc( nPos + 1 );
            pCtx->pLock->LockTokens[ nPos ]
                = rtl::OUString( buf, len, RTL_TEXTENCODING_ASCII_US );
            pCtx->hasHREF = true;
            break;
        }

    }

#ifdef BEEHIVE_BUGS_WORKAROUND
    delete [] buf;
#endif

    return 0; // zero to continue, non-zero to abort parsing
}

//////////////////////////////////////////////////////////////////////////
extern "C" int LockSequence_endelement_callback(
    void *userdata,
    int state,
    const char *,
    const char * )
{
    LockSequenceParseContext * pCtx
                    = static_cast< LockSequenceParseContext * >( userdata );
    if ( !pCtx->pLock )
        pCtx->pLock = new ucb::Lock;

    switch ( state )
    {
        case STATE_EXCLUSIVE:
            pCtx->pLock->Scope = ucb::LockScope_EXCLUSIVE;
            pCtx->hasLockScope = true;
            break;

        case STATE_SHARED:
            pCtx->pLock->Scope = ucb::LockScope_SHARED;
            pCtx->hasLockScope = true;
            break;

        case STATE_WRITE:
            pCtx->pLock->Type = ucb::LockType_WRITE;
            pCtx->hasLockType = true;
            break;

        case STATE_DEPTH:
            if ( !pCtx->hasDepth )
                return 1; // abort
            break;

        case STATE_HREF:
            if ( !pCtx->hasHREF )
                return 1; // abort
            break;

        case STATE_TIMEOUT:
            if ( !pCtx->hasTimeout )
                return 1; // abort
            break;

        case STATE_LOCKSCOPE:
            if ( !pCtx->hasLockScope )
                return 1; // abort
            break;

        case STATE_LOCKTYPE:
            if ( !pCtx->hasLockType )
                return 1; // abort
            break;

        case STATE_ACTIVELOCK:
            if ( !pCtx->hasLockType || !pCtx->hasLockType || !pCtx->hasDepth )
                return 1; // abort
            break;

        default:
            break;
    }
    return 0; // zero to continue, non-zero to abort parsing
}

//////////////////////////////////////////////////////////////////////////
// static
bool LockSequence::createFromXML( const rtl::OString & rInData,
                                  uno::Sequence< ucb::Lock > & rOutData )
{
    const sal_Int32 TOKEN_LENGTH = 13; // </activelock>
    bool success = true;

    // rInData may contain multiple <activelock>...</activelock> tags.
    sal_Int32 nCount = 0;
    sal_Int32 nStart = 0;
    sal_Int32 nEnd   = rInData.indexOf( "</activelock>" );
    while ( nEnd > -1 )
    {
        ne_xml_parser * parser = ne_xml_create();
        if ( !parser )
        {
            success = false;
            break;
        }

        LockSequenceParseContext aCtx;
        ne_xml_push_handler( parser,
                             LockSequence_startelement_callback,
                             LockSequence_chardata_callback,
                             LockSequence_endelement_callback,
                             &aCtx );

        ne_xml_parse( parser,
                      rInData.getStr() + nStart,
                      nEnd - nStart + TOKEN_LENGTH );

        success = !ne_xml_failed( parser );

        ne_xml_destroy( parser );

        if ( !success )
            break;

        if ( aCtx.pLock )
        {
            nCount++;
            if ( nCount > rOutData.getLength() )
                rOutData.realloc( rOutData.getLength() + 1 );

            rOutData[ nCount - 1 ] = *aCtx.pLock;
        }

        nStart = nEnd + TOKEN_LENGTH;
        nEnd   = rInData.indexOf( "</activelock>", nStart );
    }

    return success;
}
