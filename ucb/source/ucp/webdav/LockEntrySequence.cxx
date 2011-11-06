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
#include "LockEntrySequence.hxx"

using namespace webdav_ucp;
using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////

struct LockEntrySequenceParseContext
{
    ucb::LockEntry * pEntry;
    bool hasScope;
    bool hasType;

    LockEntrySequenceParseContext()
    : pEntry( 0 ), hasScope( false ), hasType( false ) {}
    ~LockEntrySequenceParseContext() { delete pEntry; }
};

#define STATE_TOP (1)

#define STATE_LOCKENTRY (STATE_TOP)
#define STATE_LOCKSCOPE (STATE_TOP + 1)
#define STATE_EXCLUSIVE (STATE_TOP + 2)
#define STATE_SHARED    (STATE_TOP + 3)
#define STATE_LOCKTYPE  (STATE_TOP + 4)
#define STATE_WRITE     (STATE_TOP + 5)

//////////////////////////////////////////////////////////////////////////
extern "C" int LockEntrySequence_startelement_callback(
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
                if ( strcmp( name, "lockentry" ) == 0 )
                    return STATE_LOCKENTRY;
                break;

            case STATE_LOCKENTRY:
                if ( strcmp( name, "lockscope" ) == 0 )
                    return STATE_LOCKSCOPE;
                else if ( strcmp( name, "locktype" ) == 0 )
                    return STATE_LOCKTYPE;

#define IIS_BUGS_WORKAROUND

#ifdef IIS_BUGS_WORKAROUND
                /* IIS (6) returns XML violating RFC 4918
                   for DAV:supportedlock property value.

                   <lockentry>
                       <write></write>
                       <shared></shared>
                   </lockentry>
                   <lockentry>
                       <write></write>
                       <exclusive></exclusive>
                   </lockentry>

                   Bother...
                */
                else if ( strcmp( name, "exclusive" ) == 0 )
                    return STATE_EXCLUSIVE;
                else if ( strcmp( name, "shared" ) == 0 )
                    return STATE_SHARED;
                else if ( strcmp( name, "write" ) == 0 )
                    return STATE_WRITE;
#endif
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
        }
    }
    return NE_XML_DECLINE;
}

//////////////////////////////////////////////////////////////////////////
extern "C" int LockEntrySequence_chardata_callback(
    void *,
    int,
    const char *,
    size_t )
{
    return 0; // zero to continue, non-zero to abort parsing
}

//////////////////////////////////////////////////////////////////////////
extern "C" int LockEntrySequence_endelement_callback(
    void *userdata,
    int state,
    const char *,
    const char * )
{
    LockEntrySequenceParseContext * pCtx
                = static_cast< LockEntrySequenceParseContext * >( userdata );
    if ( !pCtx->pEntry )
        pCtx->pEntry = new ucb::LockEntry;

    switch ( state )
    {
        case STATE_EXCLUSIVE:
            pCtx->pEntry->Scope = ucb::LockScope_EXCLUSIVE;
            pCtx->hasScope = true;
            break;

        case STATE_SHARED:
            pCtx->pEntry->Scope = ucb::LockScope_SHARED;
            pCtx->hasScope = true;
            break;

        case STATE_WRITE:
            pCtx->pEntry->Type = ucb::LockType_WRITE;
            pCtx->hasType = true;
            break;

        case STATE_LOCKSCOPE:
            if ( !pCtx->hasScope )
                return 1; // abort
            break;

        case STATE_LOCKTYPE:
            if ( !pCtx->hasType )
                return 1; // abort
            break;

        case STATE_LOCKENTRY:
            if ( !pCtx->hasType || !pCtx->hasType )
                return 1; // abort
            break;

        default:
            break;
    }
    return 0; // zero to continue, non-zero to abort parsing
}

//////////////////////////////////////////////////////////////////////////
// static
bool LockEntrySequence::createFromXML( const rtl::OString & rInData,
                                       uno::Sequence<
                                                ucb::LockEntry > & rOutData )
{
    const sal_Int32 TOKEN_LENGTH = 12; // </lockentry>
    bool success = true;

    // rInData may contain multiple <lockentry>...</lockentry> tags.
    sal_Int32 nCount = 0;
    sal_Int32 nStart = 0;
    sal_Int32 nEnd   = rInData.indexOf( "</lockentry>" );
    while ( nEnd > -1 )
    {
        ne_xml_parser * parser = ne_xml_create();
        if ( !parser )
        {
            success = false;
            break;
        }

        LockEntrySequenceParseContext aCtx;
        ne_xml_push_handler( parser,
                             LockEntrySequence_startelement_callback,
                             LockEntrySequence_chardata_callback,
                             LockEntrySequence_endelement_callback,
                             &aCtx );

        ne_xml_parse( parser,
                      rInData.getStr() + nStart,
                      nEnd - nStart + TOKEN_LENGTH );

        success = !ne_xml_failed( parser );

        ne_xml_destroy( parser );

        if ( !success )
            break;

        if ( aCtx.pEntry )
        {
            nCount++;
            if ( nCount > rOutData.getLength() )
                rOutData.realloc( rOutData.getLength() + 2 );

            rOutData[ nCount - 1 ] = *aCtx.pEntry;
        }

        nStart = nEnd + TOKEN_LENGTH;
        nEnd   = rInData.indexOf( "</lockentry>", nStart );
    }

    rOutData.realloc( nCount );
    return success;
}
