/*************************************************************************
 *
 *  $RCSfile: NeonHeadRequest.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2002-08-29 09:00:12 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif

#ifndef _NEONHEADREQUEST_HXX_
#include "NeonHeadRequest.hxx"
#endif

using namespace webdav_ucp;
using namespace com::sun::star;

struct NeonHeadRequestContext
{
    DAVResource * pResource;
    const std::vector< ::rtl::OUString > * pHeaderNames;

    NeonHeadRequestContext( DAVResource * p,
                            const std::vector< ::rtl::OUString > * pHeaders )
    : pResource( p ), pHeaderNames( pHeaders ) {}
};

extern "C" void NHR_ResponseHeaderCatcher( void * userdata,
                                           const char * value )
{
    rtl::OUString aHeader( rtl::OUString::createFromAscii( value ) );
    sal_Int32 nPos = aHeader.indexOf( ':' );

    if ( nPos != -1 )
    {
        rtl::OUString aHeaderName( aHeader.copy( 0, nPos ) );

        NeonHeadRequestContext * pCtx
            = static_cast< NeonHeadRequestContext * >( userdata );

        // Note: Empty vector means that all headers are requested.
        bool bIncludeIt = ( pCtx->pHeaderNames->size() == 0 );

        if ( !bIncludeIt )
        {
            // Check whether this header was requested.
            std::vector< ::rtl::OUString >::const_iterator it(
                pCtx->pHeaderNames->begin() );
            const std::vector< ::rtl::OUString >::const_iterator end(
                pCtx->pHeaderNames->end() );

            while ( it != end )
            {
                if ( (*it) == aHeaderName )
                    break;

                ++it;
            }

            if ( it != end )
                bIncludeIt = true;
        }

        if ( bIncludeIt )
        {
            // Create & set the PropertyValue
            beans::PropertyValue thePropertyValue;
            thePropertyValue.Handle = -1;
            thePropertyValue.Name   = aHeaderName;
            thePropertyValue.State  = beans::PropertyState_DIRECT_VALUE;

            if ( nPos < aHeader.getLength() )
                thePropertyValue.Value <<= aHeader.copy( nPos + 1 ).trim();

            // Add the newly created PropertyValue
            pCtx->pResource->properties.push_back( thePropertyValue );
        }
    }
}


// -------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------

NeonHeadRequest::NeonHeadRequest( HttpSession* inSession,
                                  const rtl::OUString & inPath,
                                  const std::vector< ::rtl::OUString > &
                                    inHeaderNames,
                                  std::vector< DAVResource >& ioResources,
                                  int & nError )
{
    // Add own entry to resources list.
    ioResources.clear();
    ioResources.push_back( DAVResource( inPath ) );

    // Create and dispatch HEAD request. Install catcher for all response
    // header fields.
    ne_request * req = ne_request_create( inSession,
                                          "HEAD",
                                          rtl::OUStringToOString(
                                            inPath,
                                            RTL_TEXTENCODING_UTF8 ) );

    NeonHeadRequestContext aCtx( &ioResources.back(), &inHeaderNames );
    ne_add_response_header_catcher( req, NHR_ResponseHeaderCatcher, &aCtx );

    nError = ne_request_dispatch( req );

    if ( nError == NE_OK && ne_get_status( req )->klass != 2 )
        nError = NE_ERROR;

    ne_request_destroy( req );

    // #87585# - Sometimes neon lies (because some servers lie).
    if ( ( nError == NE_OK ) && ioResources.empty() )
        nError = NE_ERROR;
}

// -------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------
NeonHeadRequest::~NeonHeadRequest()
{
}

