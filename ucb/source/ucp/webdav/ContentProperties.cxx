/*************************************************************************
 *
 *  $RCSfile: ContentProperties.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kso $ $Date: 2002-08-29 09:00:11 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _NEONURI_HXX_
#include "NeonUri.hxx"
#endif
#ifndef _DAVRESOURCE_HXX_
#include "DAVResource.hxx"
#endif
#ifndef _DAVPROPERTIES_HXX_
#include "DAVProperties.hxx"
#endif
#ifndef _WEBDAV_DATETIME_HELPER_HXX
#include "DateTimeHelper.hxx"
#endif

#ifndef _WEBDAV_UCP_CONTENTPROPERTIES_HXX
#include "ContentProperties.hxx"
#endif

using namespace com::sun::star;
using namespace webdav_ucp;

/*
=============================================================================

                            Property Mapping

=============================================================================
HTTP (entity header)    WebDAV (property)   UCB (property)
=============================================================================

Allow
Content-Encoding
Content-Language        getcontentlanguage
Content-Length          getcontentlength    Size
Content-Location
Content-MD5
Content-Range
Content-Type            getcontenttype      MediaType
Expires
Last-Modified           getlastmodified     DateModified
                        creationdate        DateCreated
                        resourcetype        IsFolder,IsDocument,ContentType
                        displayname
ETag (actually          getetag
a response header )
                        lockdiscovery
                        supportedlock
                        source
                                            Title (always taken from URI)

=============================================================================

Important: HTTP headers with not be mapped to DAV properties; only to UCB
           properties. (Content-Length,Content-Type,Last-Modified)
*/

//=========================================================================
//=========================================================================
//
// ContentProperties Implementation.
//
//=========================================================================
//=========================================================================

ContentProperties::ContentProperties( const DAVResource& rResource )
: bTrailingSlash( sal_False ),
  pIsDocument( 0 ),
  pIsFolder( 0 ),
  pSize( 0 ),
  pDateCreated( 0 ),
  pDateModified( 0 ),
  pMediaType( 0 ),
  pgetcontenttype( 0 ),
  pcreationdate( 0 ),
  pdisplayname( 0 ),
  pgetcontentlanguage( 0 ),
  pgetcontentlength( 0 ),
  pgetetag( 0 ),
  pgetlastmodified( 0 ),
  plockdiscovery( 0 ),
  presourcetype( 0 ),
  psource( 0 ),
  psupportedlock( 0 ),
  pOtherProps( 0 )
{
    OSL_ENSURE( rResource.uri.getLength(),
                "ContentProperties::ContentProperties - Empty URL!" );

    // title
      NeonUri aURI( rResource.uri );
      aTitle        = aURI.GetPathBaseNameUnescaped();
      aEscapedTitle = aURI.GetPathBaseName();

    std::vector< beans::PropertyValue >::const_iterator it
        = rResource.properties.begin();
      std::vector< beans::PropertyValue >::const_iterator end
        = rResource.properties.end();

      while ( it != end )
      {
        beans::PropertyValue aProp = (*it);

        if ( aProp.Name.equals( DAVProperties::CREATIONDATE ) )
        {
            pcreationdate = new rtl::OUString;
              aProp.Value >>= *pcreationdate;

              // Map the DAV:creationdate to UCP:DateCreated
            pDateCreated = new util::DateTime;
            DateTimeHelper::convert( *pcreationdate, *pDateCreated );
        }
        else if ( aProp.Name.equals( DAVProperties::DISPLAYNAME ) )
        {
            pdisplayname = new rtl::OUString;
              aProp.Value >>= *pdisplayname;
        }
        else if ( aProp.Name.equals( DAVProperties::GETCONTENTLANGUAGE ) )
        {
            pgetcontentlanguage = new rtl::OUString;
              aProp.Value >>= *pgetcontentlanguage;
        }
        else if ( aProp.Name.equals( DAVProperties::GETCONTENTLENGTH ) )
        {
            pgetcontentlength = new rtl::OUString;
              aProp.Value >>= *pgetcontentlength;

              // Map the DAV:getcontentlength to UCP:Size
            if ( !pSize )
                pSize = new sal_Int64;

              *pSize = pgetcontentlength->toInt64();
        }
        else if ( aProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Content-Length" ) ) )
        {
            // Do not map Content-Lenght entity header to DAV:getcontentlength!

            // No extra member for this. Store with "other" props.
            if ( !pOtherProps )
                pOtherProps = new PropertyValueMap;

            rtl::OUString aValue;
            aProp.Value >>= aValue;
            (*pOtherProps)[ aProp.Name ] = aProp.Value;

            // Map the Content-Length entity header to UCP:Size
            if ( !pSize )
                pSize = new sal_Int64;

            *pSize = aValue.toInt64();
        }
        else if ( aProp.Name.equals( DAVProperties::GETCONTENTTYPE ) )
        {
            pgetcontenttype = new rtl::OUString;
              aProp.Value >>= *pgetcontenttype;

            // Map the DAV:getcontenttype to UCP:MediaType
            if ( !pMediaType )
                pMediaType = new rtl::OUString;

            *pMediaType = *pgetcontenttype;
        }
        else if ( aProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Content-Type" ) ) )
        {
            // Do not map Content-Type entity header to DAV:getcontenttype!

            // No extra member for this. Store with "other" props.
            if ( !pOtherProps )
                pOtherProps = new PropertyValueMap;

            (*pOtherProps)[ aProp.Name ] = aProp.Value;

            // Map the Content-Type entity header to UCP:Size
            rtl::OUString aValue;
            aProp.Value >>= aValue;

            if ( !pMediaType )
                pMediaType = new rtl::OUString;

            *pMediaType = aValue;
        }
        else if ( aProp.Name.equals( DAVProperties::GETETAG ) )
        {
            pgetetag = new rtl::OUString;
              aProp.Value >>= *pgetetag;
        }
        else if ( aProp.Name.equals( DAVProperties::GETLASTMODIFIED ) )
        {
            pgetlastmodified = new rtl::OUString;
              aProp.Value >>= *pgetlastmodified;

              // Map the DAV:getlastmodified to UCP:DateModified
            if ( !pDateModified )
                pDateModified = new util::DateTime;

            DateTimeHelper::convert( *pgetlastmodified, *pDateModified );
        }
        else if ( aProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Last-Modified" ) ) )
        {
            // Do not map Last-Modified entity header to DAV:getlastmodified!

            // No extra member for this. Store with "other" props.
            if ( !pOtherProps )
                pOtherProps = new PropertyValueMap;

            rtl::OUString aValue;
            aProp.Value >>= aValue;
            (*pOtherProps)[ aProp.Name ] = aProp.Value;

            // Map the Content-Length entity header to UCP:Size
            if ( !pDateModified )
                pDateModified = new util::DateTime;

            DateTimeHelper::convert( aValue, *pDateModified );
        }

        else if ( aProp.Name.equals( DAVProperties::LOCKDISCOVERY ) )
        {
            plockdiscovery = new uno::Sequence< ucb::Lock >;
              aProp.Value >>= *plockdiscovery;
        }
        else if ( aProp.Name.equals( DAVProperties::RESOURCETYPE ) )
        {
            presourcetype = new rtl::OUString;
              aProp.Value >>= *presourcetype;

            pIsFolder   = new sal_Bool( sal_False );
            pIsDocument = new sal_Bool( sal_True );

              // Map the DAV:resourceype to UCP:IsFolder, UCP:IsDocument
              if ( presourcetype->equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "collection" ) ) )
            {
                *pIsDocument =  sal_False;
                *pIsFolder   =  sal_True;
              }
        }
        else if ( aProp.Name.equals( DAVProperties::SOURCE ) )
        {
            psource = new uno::Sequence< ucb::Link >;
              aProp.Value >>= *psource;
        }
        else if ( aProp.Name.equals( DAVProperties::SUPPORTEDLOCK ) )
        {
            psupportedlock = new uno::Sequence< ucb::LockEntry >;
              aProp.Value >>= *psupportedlock;
        }
        else
        {
            if ( !pOtherProps )
                pOtherProps = new PropertyValueMap;

            (*pOtherProps)[ aProp.Name ] = aProp.Value;
        }
        ++it;
      }

    if ( rResource.uri.getStr()[ rResource.uri.getLength() - 1 ]
        == sal_Unicode( '/' ) )
    {
//        if ( pIsFolder && *pIsFolder )
            bTrailingSlash = sal_True;
    }
}

//=========================================================================
ContentProperties::ContentProperties(
                        const rtl::OUString & rTitle, sal_Bool bFolder )
: aTitle( rTitle ),
  bTrailingSlash( sal_False ),
  pIsDocument( new sal_Bool( !bFolder ) ),
  pIsFolder( new sal_Bool( bFolder ) ),
  pSize( 0 ),
  pDateCreated( 0 ),
  pDateModified( 0 ),
  pMediaType( 0 ),
  pgetcontenttype( 0 ),
  pcreationdate( 0 ),
  pdisplayname( 0 ),
  pgetcontentlanguage( 0 ),
  pgetcontentlength( 0 ),
  pgetetag( 0 ),
  pgetlastmodified( 0 ),
  plockdiscovery( 0 ),
  presourcetype( 0 ),
  psource( 0 ),
  psupportedlock( 0 ),
  pOtherProps( 0 )
{
}

//=========================================================================
ContentProperties::ContentProperties( const rtl::OUString & rTitle )
: aTitle( rTitle ),
  bTrailingSlash( sal_False ),
  pIsDocument( 0 ),
  pIsFolder( 0 ),
  pSize( 0 ),
  pDateCreated( 0 ),
  pDateModified( 0 ),
  pMediaType( 0 ),
  pgetcontenttype( 0 ),
  pcreationdate( 0 ),
  pdisplayname( 0 ),
  pgetcontentlanguage( 0 ),
  pgetcontentlength( 0 ),
  pgetetag( 0 ),
  pgetlastmodified( 0 ),
  plockdiscovery( 0 ),
  presourcetype( 0 ),
  psource( 0 ),
  psupportedlock( 0 ),
  pOtherProps( 0 )
{
}

//=========================================================================
// static
void ContentProperties::UCBNamesToDAVNames(
                            const uno::Sequence< beans::Property > & rProps,
                            std::vector< rtl::OUString > & propertyNames,
                            bool bIncludeUnmatched /* = true */ )
{
    //////////////////////////////////////////////////////////////
    // Assemble list of DAV properties to obtain from server.
    // Append DAV properties needed to obtain requested UCB props.
    //////////////////////////////////////////////////////////////

    //       DAV              UCB
    // creationdate     <- DateCreated
    // getlastmodified  <- DateModified
    // getcontenttype   <- MediaType
    // getcontentlength <- Size
    // resourcetype     <- IsFolder, IsDocument, ContentType
    // (taken from URI) <- Title

    sal_Bool bCreationDate  = sal_False;
    sal_Bool bLastModified  = sal_False;
    sal_Bool bContentType   = sal_False;
    sal_Bool bContentLength = sal_False;
    sal_Bool bResourceType  = sal_False;

    sal_Int32 nCount = rProps.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::Property & rProp = rProps[ n ];

        if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
        {
            // Title is always obtained from resource's URI.
            continue;
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "DateCreated" ) )
                  ||
                  ( rProp.Name == DAVProperties::CREATIONDATE ) )
        {
            if ( !bCreationDate )
            {
                   propertyNames.push_back( DAVProperties::CREATIONDATE );
                bCreationDate = sal_True;
            }
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "DateModified" ) )
                  ||
                  ( rProp.Name == DAVProperties::GETLASTMODIFIED ) )
        {
            if ( !bLastModified )
            {
                   propertyNames.push_back(
                    DAVProperties::GETLASTMODIFIED );
                bLastModified = sal_True;
            }
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "MediaType" ) )
                  ||
                  ( rProp.Name == DAVProperties::GETCONTENTTYPE ) )
        {
            if ( !bContentType )
            {
                   propertyNames.push_back(
                        DAVProperties::GETCONTENTTYPE );
                bContentType = sal_True;
            }
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Size" ) )
                  ||
                  ( rProp.Name == DAVProperties::GETCONTENTLENGTH ) )
        {
            if ( !bContentLength )
            {
                   propertyNames.push_back(
                    DAVProperties::GETCONTENTLENGTH );
                bContentLength = sal_True;
            }
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "ContentType" ) )
                  ||
                  rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) )
                  ||
                  rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) )
                  ||
                  ( rProp.Name == DAVProperties::RESOURCETYPE ) )
        {
            if ( !bResourceType )
            {
                   propertyNames.push_back( DAVProperties::RESOURCETYPE );
                bResourceType = sal_True;
            }
        }
        else
        {
            if ( bIncludeUnmatched )
                propertyNames.push_back( rProp.Name );
        }
    }
}

//=========================================================================
// static
void ContentProperties::UCBNamesToHTTPNames(
                            const uno::Sequence< beans::Property > & rProps,
                            std::vector< rtl::OUString > & propertyNames,
                            bool bIncludeUnmatched /* = true */ )
{
    //////////////////////////////////////////////////////////////
    // Assemble list of HTTP header names to obtain from server.
    // Append HTTP headers needed to obtain requested UCB props.
    //////////////////////////////////////////////////////////////

    //       HTTP              UCB
    // Last-Modified  <- DateModified
    // Content-Type   <- MediaType
    // Content-Length <- Size

    sal_Int32 nCount = rProps.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::Property & rProp = rProps[ n ];

        if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "DateModified" ) ) )
        {
            propertyNames.push_back(
                rtl::OUString::createFromAscii( "Last-Modified" ) );
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
        {
            propertyNames.push_back(
                rtl::OUString::createFromAscii( "Content-Type" ) );
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Size" ) ) )
        {
            propertyNames.push_back(
                rtl::OUString::createFromAscii( "Content-Length" ) );
        }
        else
        {
            if ( bIncludeUnmatched )
                propertyNames.push_back( rProp.Name );
        }
    }

}
