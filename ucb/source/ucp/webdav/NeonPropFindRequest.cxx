/*************************************************************************
 *
 *  $RCSfile: NeonPropFindRequest.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:55:20 $
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
#include "NeonPropFindRequest.hxx"
#include "DAVException.hxx"

#define ELM_creationdate        (1000)
#define ELM_displayname         (1001)
#define ELM_getcontentlanguage  (1002)
#define ELM_getcontentlength    (1003)
#define ELM_getcontenttype      (1004)
#define ELM_getetag             (1005)
#define ELM_getlastmodified     (1006)
#define ELM_lockdiscovery       (1007)
#define ELM_resourcetype        (1008)
#define ELM_source              (1009)
#define ELM_supportedlock       (1010)
#define ELM_collection          (1011)

#define GetPropName( s ) OUStringToOString( s, RTL_TEXTENCODING_DONTKNOW )

using namespace rtl;
using namespace com::sun::star::beans;
using namespace std;
using namespace webdav_ucp;



const NeonPropFindXmlElem NeonPropFindRequest::sXmlElems[] =
{
    { "DAV:", "creationdate",
       ELM_creationdate, HIP_XML_CDATA },
    { "DAV:", "displayname",
       ELM_displayname, HIP_XML_CDATA },
    { "DAV:", "getcontentlanguage",
       ELM_getcontentlanguage, HIP_XML_CDATA },
    { "DAV:", "getcontentlength",
       ELM_getcontentlength, HIP_XML_CDATA },
    { "DAV:", "getcontenttype",
       ELM_getcontenttype, HIP_XML_CDATA },
    { "DAV:", "getetag",
       ELM_getetag, HIP_XML_CDATA },
    { "DAV:", "getlastmodified",
       ELM_getlastmodified, HIP_XML_CDATA },
    { "DAV:", "lockdiscovery",
       ELM_lockdiscovery, HIP_XML_CDATA },
    { "DAV:", "resourcetype",
       ELM_resourcetype, 0 },
    { "DAV:", "source",
       ELM_source, HIP_XML_CDATA },
    { "DAV:", "supportedlock",
       ELM_supportedlock, HIP_XML_CDATA },
    { "DAV:", "collection",
       ELM_collection, 0 },
    { NULL }
};

// -------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------
NeonPropFindRequest::NeonPropFindRequest( HttpSession *             inSession,
                                          const char *              inPath,
                                          const Depth               inDepth,
                                          const vector<OUString>    inPropNames,
                                          vector< DAVResource > &   ioResources)
{
    // Create a propfind handler
    if ( ( mPropFindHandler = dav_propfind_create( inSession,
                                                   inPath,
                                                   inDepth ) ) == NULL )
        throw DAVException( DAVException::DAV_REQUEST_CREATE );

    // Setup the Begin and End resource handlers
    dav_propfind_set_resource_handlers( mPropFindHandler,
                                        StartResource,
                                        EndResource );

    // Setup the CheckContext and End element handlers
    hip_xml_add_handler( dav_propfind_get_parser( mPropFindHandler ),
                         sXmlElems,
                         CheckContext,
                         NULL,
                         EndElement,
                         mPropFindHandler );

    // Generate the list of properties we're looking for
    int thePropCount = inPropNames.size();
    int theRetVal;
    if ( thePropCount > 0 )
    {
        NeonPropName * thePropNames = new NeonPropName[ thePropCount + 1 ];
        for ( int theIndex = 0; theIndex < thePropCount; theIndex ++ )
        {
            thePropNames[ theIndex ].nspace = "DAV:";
            // Warning: Why am I doing a strdup here?
            thePropNames[ theIndex ].name =
                        strdup( GetPropName( inPropNames[ theIndex ] ) );
        }
        thePropNames[ theIndex ].nspace = NULL;
        thePropNames[ theIndex ].name = NULL;

        // Send the request
        theRetVal = dav_propfind_named( mPropFindHandler,
                                        thePropNames,
                                        &ioResources );

        for ( theIndex = 0; theIndex < thePropCount; theIndex ++ )
            free( (void *)thePropNames[ theIndex ].name );
        delete thePropNames;
    }
    else
        throw DAVException( DAVException::DAV_INVALID_ARG );

    if ( theRetVal != HTTP_OK )
        throw DAVException( DAVException::DAV_HTTP_ERROR );
}

// -------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------
NeonPropFindRequest::~NeonPropFindRequest( )
{
}

// -------------------------------------------------------------------
// StartResource
// Marks the start of a resource in the server response.
// Adds a DAVResource to the vector supplied at instantiation.
// -------------------------------------------------------------------
void * NeonPropFindRequest::StartResource( void *       inUserData,
                                           const char * inHref )
{
    DAVResource * theResource = new DAVResource(
                    OStringToOUString( inHref, RTL_TEXTENCODING_UTF8 ),
                    false );
    //theResource->uri = OStringToOUString( inHref, RTL_TEXTENCODING_UTF8 );
    vector< DAVResource > * theResources = (vector< DAVResource > * )inUserData;
    theResources->push_back( *theResource );
    return theResources;
}

// -------------------------------------------------------------------
// EndResource
// Warning: Do we need this?
// -------------------------------------------------------------------
void NeonPropFindRequest::EndResource( void *               inUserData,
                                       void *               inResource,
                                       const char *         inStatusLine,
                                       const HttpStatus *   inHttpStatus,
                                       const char *         inDescription )
{
}


// -------------------------------------------------------------------
// EndElement
// Marks the end of the current XML element in the server response
// Creates and adds a new PropertyValue to the DAVResource created in
// StartResource.
// -------------------------------------------------------------------
int NeonPropFindRequest::EndElement( void *                         inUserData,
                                     const NeonPropFindXmlElem *    inXmlElem,
                                     const char *                   inCdata )
{
    if ( inXmlElem->id != ELM_resourcetype )
    {
        // Create & set the PropertyValue
        PropertyValue *thePropertyValue = new PropertyValue;
        // Warning: What should Handle be?
        thePropertyValue->Handle    = 0;
        thePropertyValue->State = PropertyState_DIRECT_VALUE;

        if ( inXmlElem->id == ELM_collection )
        {
            thePropertyValue->Name  = OStringToOUString( "resourcetype",
                                                RTL_TEXTENCODING_UTF8 );
            thePropertyValue->Value <<= OStringToOUString( "collection",
                                                RTL_TEXTENCODING_UTF8 );
        }
        else
        {
            thePropertyValue->Name  = OStringToOUString( inXmlElem->name,
                                                RTL_TEXTENCODING_UTF8 );
            thePropertyValue->Value <<= OStringToOUString( inCdata,
                                                RTL_TEXTENCODING_UTF8 );
        }


        // Get hold of the current DAVResource ( as created in StartResource )
        vector< DAVResource > * theResources =
         (vector< DAVResource > * )dav_propfind_get_current_resource(
                                        ( NeonPropFindHandler *)inUserData );

        // Add the newly created PropertyValue
        vector< DAVResource >::iterator theIterator = theResources->end();
        theIterator--;
        theIterator->properties.push_back( thePropertyValue );
    }

    return 0;
}

// -------------------------------------------------------------------
// CheckContext
// Verify that the current child element is valid given the current
// parent element
// -------------------------------------------------------------------
int NeonPropFindRequest::CheckContext( NeonPropFindXmlId    inChild,
                                       NeonPropFindXmlId    inParent )
{
    //Warning: This should be filled in
    return 0;
}
