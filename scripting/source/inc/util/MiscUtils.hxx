/*************************************************************************
 *
 *  $RCSfile: MiscUtils.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-10-22 14:04:15 $
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

#ifndef _SCRIPT_FRAMEWORK_MISCUTILS_HXX_
#define _SCRIPT_FRAMEWORK_MISCUTILS_HXX_

#include <rtl/ustring>

#include <ucbhelper/content.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>


#include "util.hxx"

namespace sf_misc
{
// for simplification
#define css ::com::sun::star

class MiscUtils
{
public:
    static css::uno::Sequence< ::rtl::OUString > allOpenTDocUrls( const  css::uno::Reference< css::uno::XComponentContext >& xCtx)
{
    OSL_TRACE("MiscUtils::allOpenTDocUrls");
    css::uno::Sequence< ::rtl::OUString > result;
    try
    {
        if ( !xCtx.is() )
        {
            return result;
        }
        css::uno::Reference < css::lang::XMultiComponentFactory > xFac( xCtx->getServiceManager(), css::uno::UNO_QUERY );
        if ( xFac.is() )
        {
            css::uno::Reference < com::sun::star::ucb::XSimpleFileAccess > xSFA( xFac->createInstanceWithContext( OUSTR("com.sun.star.ucb.SimpleFileAccess"), xCtx ), css::uno::UNO_QUERY );
            if ( xSFA.is() )
            {
                OSL_TRACE("Getting folder contents of vnd.sun.star.tdoc:");
                result = xSFA->getFolderContents( OUSTR("vnd.sun.star.tdoc:/"), true );
                OSL_TRACE("Getting folder contents is %d", result.getLength());
            }
        }
    }
    catch ( css::uno::Exception& e )
    {
    }
    return result;
}
    static ::rtl::OUString xModelToDocTitle( const css::uno::Reference< css::frame::XModel >& xModel )
{
    OSL_TRACE("MiscUtils::xModelToDocTitle() ");
    // Set a default name, this should never be seen.
    ::rtl::OUString docNameOrURL =
        ::rtl::OUString::createFromAscii("Unknown");
    if ( xModel.is() )
    {
        ::rtl::OUString tempName;
        try
        {
            css::uno::Reference< css::beans::XPropertySet > propSet( xModel->getCurrentController()->getFrame(), css::uno::UNO_QUERY );
            if ( propSet.is() )
            {
                if ( sal_True == ( propSet->getPropertyValue(::rtl::OUString::createFromAscii( "Title" ) ) >>= tempName ) )
                {
                    docNameOrURL = tempName;
                    if ( xModel->getURL().getLength() == 0 )
                    {
                        // process "UntitledX - YYYYYYYY"
                        // to get UntitledX
                        sal_Int32 pos = 0;
                        docNameOrURL = tempName.getToken(0,' ',pos);
                        OSL_TRACE("MiscUtils::xModelToDocTitle() Title for document is %s.",
                            ::rtl::OUStringToOString( docNameOrURL,
                                            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                    }
                    else
                    {
                        css::uno::Reference< css::document::XDocumentInfoSupplier >  xDIS( xModel, css::uno::UNO_QUERY_THROW );
                        css::uno::Reference< css::beans::XPropertySet > xProp (xDIS->getDocumentInfo(),  css::uno::UNO_QUERY_THROW );
                        css::uno::Any aTitle = xProp->getPropertyValue(::rtl::OUString::createFromAscii( "Title" ) );

                        aTitle >>= docNameOrURL;
                        if ( docNameOrURL.getLength() == 0 )
                        {
                            docNameOrURL =  parseLocationName( xModel->getURL() );
                        }
                    }
                }
            }
        }
        catch ( css::uno::Exception& e )
        {
            OSL_TRACE("MiscUtils::xModelToDocTitle() exception thrown: !!! %s",
                ::rtl::OUStringToOString( e.Message,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }

    }
    else
    {
        OSL_TRACE("MiscUtils::xModelToDocTitle() doc model is null" );
    }
    return docNameOrURL;
}

    static ::rtl::OUString tDocUrlToTitle( const ::rtl::OUString& url )
{
    ::rtl::OUString title;
    ::ucb::Content root( url, NULL );
    ::rtl::OUString propName =  OUSTR("Title");
    getUCBProperty( root, propName ) >>= title;
    return title;
}
    static ::rtl::OUString xModelToTdocUrl( const css::uno::Reference< css::frame::XModel >& xModel )
{
    OSL_TRACE("MiscUtils::xModelToDocTitle() ");
    css::uno::Reference < com::sun::star::ucb::XCommandEnvironment > dummy;
    ::rtl::OUString sDocRoot = OUSTR( "vnd.sun.star.tdoc:/" );
    ::ucb::Content root( sDocRoot, dummy );

    OSL_TRACE( "get root content" );

    css::uno::Sequence < ::rtl::OUString > aPropertyNames( 0 );
    ::rtl::OUString propName = OUSTR( "DocumentModel" );

    css::uno::Reference < com::sun::star::sdbc::XResultSet > xResultSet =
        root.createCursor( aPropertyNames );

    OSL_TRACE( "get list of documents" );

    css::uno::Reference< com::sun::star::ucb::XContentAccess > xContentAccess(
        xResultSet, css::uno::UNO_QUERY );
    OSL_TRACE( "got contentaccess" );
    ::rtl::OUString docUrl;
    if ( xResultSet.is() && xContentAccess.is() )
    {
        css::uno::Reference< css::sdbc::XRow > values( xResultSet, css::uno::UNO_QUERY );
        if ( values.is() )
        {
            while ( xResultSet->next() )
            {
                ::rtl::OUString url = xContentAccess->queryContentIdentifierString();
                css::uno::Reference < css::frame::XModel > xMod( tDocUrlToModel( url ), css::uno::UNO_QUERY );

                OSL_TRACE( "xModelToTdocUrl: url is %s",
                    ::rtl::OUStringToOString( url,
                            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                if ( xMod.is() && xMod == xModel )
                {
                    docUrl = url;
                    OSL_TRACE( "xModelToTdocUrl: match for model url is %s",
                        ::rtl::OUStringToOString( url,
                            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                    break;
                }
            }
        }
    }
    return docUrl;
}
    static css::uno::Reference< css::frame::XModel > tDocUrlToModel( const ::rtl::OUString& url )
{
    css::uno::Any result;
    ::ucb::Content root( url, NULL );
    ::rtl::OUString propName =  OUSTR("DocumentModel");
    result = getUCBProperty( root, propName );

    css::uno::Reference< css::frame::XModel > xModel( result, css::uno::UNO_QUERY );
    return xModel;
}


    static css::uno::Any getUCBProperty( ::ucb::Content& content, ::rtl::OUString& prop )
{
    css::uno::Sequence < ::rtl::OUString > aPropertyNames( 1 );
    ::rtl::OUString* pNames = aPropertyNames.getArray();
    pNames[ 0 ] = prop;
    css::uno::Any result;
    try
    {
        css::uno::Sequence< css::uno::Any > props = content.getPropertyValues( aPropertyNames );
        if ( props.getLength() )
        {
            result = props[ 0 ];
        }
    }
/*    catch ( ucb::CommandAbortedException& e )
    {
    }*/
    catch ( css::uno::Exception& e )
    {
    }
    return result;
}

private:
static ::rtl::OUString parseLocationName( const ::rtl::OUString& location )
{
    // strip out the last leaf of location name
    // e.g. file://dir1/dir2/Blah.sxw - > Blah.sxw
    ::rtl::OUString temp = location;
    sal_Int32 lastSlashIndex = temp.lastIndexOf( ::rtl::OUString::createFromAscii( "/" ) );

    if ( ( lastSlashIndex + 1 ) <  temp.getLength()  )
    {
        temp = temp.copy( lastSlashIndex + 1 );
    }
    // maybe we should throw here!!!
    else
    {
        OSL_TRACE("Something wrong with name, perhaps we should throw an exception");
    }
    return temp;
}

};
} // namespace sf_misc
#endif //
