/*************************************************************************
 *
 *  $RCSfile: LockEntrySequence.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kso $ $Date: 2002-08-22 11:37:30 $
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

#ifndef _LOCKENTRYSEQUENCE_HXX_
#include "LockEntrySequence.hxx"
#endif

using namespace webdav_ucp;
using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////

#define DAV_ELM_LOCK_FIRST (NE_ELM_UNUSED)

#define DAV_ELM_lockentry (DAV_ELM_LOCK_FIRST + 1)
#define DAV_ELM_lockscope (DAV_ELM_LOCK_FIRST + 2)
#define DAV_ELM_locktype  (DAV_ELM_LOCK_FIRST + 3)
#define DAV_ELM_exclusive (DAV_ELM_LOCK_FIRST + 4)
#define DAV_ELM_shared    (DAV_ELM_LOCK_FIRST + 5)
#define DAV_ELM_write     (DAV_ELM_LOCK_FIRST + 6)

// static
const struct ne_xml_elm LockEntrySequence::elements[] =
{
    { "", "lockentry",  DAV_ELM_lockentry, 0 },
    { "", "lockscope",  DAV_ELM_lockscope, 0 },
    { "", "locktype",   DAV_ELM_locktype,  0 },
    { "", "exclusive",  DAV_ELM_exclusive, 0 }, // leaf
    { "", "shared",     DAV_ELM_shared,    0 }, // leaf
    { "", "write",      DAV_ELM_write,     0 }, // leaf
    { 0 }
};

struct LockEntrySequenceParseContext
{
    ucb::LockEntry * pEntry;

    LockEntrySequenceParseContext() : pEntry( 0 ) {}
    ~LockEntrySequenceParseContext() { delete pEntry; }
};

//////////////////////////////////////////////////////////////////////////
extern "C" static int validate_callback( void * userdata,
                                         ne_xml_elmid parent,
                                         ne_xml_elmid child )
{
    // @@@
    return NE_XML_VALID;
}

//////////////////////////////////////////////////////////////////////////
extern "C" static int endelement_callback( void * userdata,
                                           const struct ne_xml_elm * s,
                                           const char * cdata )
{
    LockEntrySequenceParseContext * pCtx
                = static_cast< LockEntrySequenceParseContext * >( userdata );
    if ( !pCtx->pEntry )
        pCtx->pEntry = new ucb::LockEntry;

    switch ( s->id )
    {
        case DAV_ELM_exclusive:
            pCtx->pEntry->Scope = ucb::LockScope_EXCLUSIVE;
            break;

        case DAV_ELM_shared:
            pCtx->pEntry->Scope = ucb::LockScope_SHARED;
            break;

        case DAV_ELM_write:
            pCtx->pEntry->Type = ucb::LockType_WRITE;
            break;

        default:
            break;
    }
    return 0;
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
                                  elements,
                                  validate_callback,
                                  0, // startelement_callback
                                  endelement_callback,
                                  &aCtx );

        ne_xml_parse( parser,
                       rInData.getStr() + nStart,
                       nEnd - nStart + TOKEN_LENGTH );

        success = !!ne_xml_valid( parser );

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

        nStart = nEnd + TOKEN_LENGTH + 1;
        nEnd   = rInData.indexOf( "</lockentry>", nStart );
    }

    rOutData.realloc( nCount );
    return success;
}
