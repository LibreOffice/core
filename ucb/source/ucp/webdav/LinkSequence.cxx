/*************************************************************************
 *
 *  $RCSfile: LinkSequence.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kso $ $Date: 2002-08-22 14:44:26 $
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

#ifndef _LINKSEQUENCE_HXX_
#include "LinkSequence.hxx"
#endif

using namespace webdav_ucp;
using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////

#define DAV_ELM_LOCK_FIRST (NE_ELM_UNUSED)

#define DAV_ELM_link    (DAV_ELM_LOCK_FIRST +  1)
#define DAV_ELM_src     (DAV_ELM_LOCK_FIRST +  2)
#define DAV_ELM_dst     (DAV_ELM_LOCK_FIRST +  3)

// static
const struct ne_xml_elm LinkSequence::elements[] =
{
    { "", "link", DAV_ELM_link, 0 },
    { "", "src",  DAV_ELM_src,  NE_XML_CDATA },
    { "", "dst",  DAV_ELM_dst,  NE_XML_CDATA },
    { 0 }
};

struct LinkSequenceParseContext
{
    ucb::Link * pLink;

    LinkSequenceParseContext() : pLink( 0 ) {}
    ~LinkSequenceParseContext() { delete pLink; }
};

//////////////////////////////////////////////////////////////////////////
extern "C" int LinkSequence_validate_callback( void * userdata,
                                               ne_xml_elmid parent,
                                               ne_xml_elmid child )
{
    // @@@
    return NE_XML_VALID;
}

//////////////////////////////////////////////////////////////////////////
extern "C" int LinkSequence_endelement_callback( void * userdata,
                                                 const struct ne_xml_elm * s,
                                                 const char * cdata )
{
    LinkSequenceParseContext * pCtx
                    = static_cast< LinkSequenceParseContext * >( userdata );
    if ( !pCtx->pLink )
        pCtx->pLink = new ucb::Link;

    switch ( s->id )
    {
        case DAV_ELM_src:
            pCtx->pLink->Source = rtl::OUString::createFromAscii( cdata );
            break;

        case DAV_ELM_dst:
            pCtx->pLink->Destination = rtl::OUString::createFromAscii( cdata );
            break;

        default:
            break;
    }
    return 0;
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
                             elements,
                             LinkSequence_validate_callback,
                             0, // startelement_callback
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

//  rOutData.realloc( nCount );
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
