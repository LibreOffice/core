/*************************************************************************
 *
 *  $RCSfile: LinkSequence.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2005-01-27 12:12:09 $
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

#include <string.h>

#ifndef NE_XML_H
#include <neon/ne_xml.h>
#endif

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
    void *userdata,
    int parent,
    const char *nspace,
    const char *name,
    const char **atts )
{
    if ( ( name != 0 ) &&
         ( ( nspace == 0 ) || ( strcmp( nspace, "" ) == 0 ) ) )
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
    const char *nspace,
    const char *name )
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

        success = !!ne_xml_valid( parser );

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

        nStart = nEnd + TOKEN_LENGTH + 1;
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
