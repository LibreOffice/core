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



#ifndef _SCRIPT_FRAMEWORK_MISCUTILS_HXX_
#define _SCRIPT_FRAMEWORK_MISCUTILS_HXX_

#include <rtl/ustring.hxx>
#include <tools/urlobj.hxx>

#include <ucbhelper/content.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XTransientDocumentsDocumentContentFactory.hpp>
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
                result = xSFA->getFolderContents( OUSTR("vnd.sun.star.tdoc:/"), true );
            }
        }
    }
    catch ( css::uno::Exception& )
    {
    }
    return result;
}

    static ::rtl::OUString xModelToTdocUrl( const css::uno::Reference< css::frame::XModel >& xModel,
                                            const css::uno::Reference< css::uno::XComponentContext >& xContext )
{
    css::uno::Reference< css::lang::XMultiComponentFactory > xMCF(
        xContext->getServiceManager() );
    css::uno::Reference<
            css::frame::XTransientDocumentsDocumentContentFactory > xDocFac;
    try
    {
        xDocFac =
            css::uno::Reference<
                css::frame::XTransientDocumentsDocumentContentFactory >(
                    xMCF->createInstanceWithContext(
                        rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.frame.TransientDocumentsDocumentContentFactory" ) ),
                        xContext ),
                css::uno::UNO_QUERY );
    }
    catch ( css::uno::Exception const & )
    {
        // handled below
    }

    if ( xDocFac.is() )
    {
        try
        {
            css::uno::Reference< css::ucb::XContent > xContent(
                xDocFac->createDocumentContent( xModel ) );
            return xContent->getIdentifier()->getContentIdentifier();
        }
        catch ( css::lang::IllegalArgumentException const & )
        {
            OSL_ENSURE( false, "Invalid document model!" );
        }
    }

    OSL_ENSURE( false, "Unable to obtain URL for document model!" );
    return rtl::OUString();
}
    static css::uno::Reference< css::frame::XModel > tDocUrlToModel( const ::rtl::OUString& url )
{
    css::uno::Any result;

    try
    {
        ::ucbhelper::Content root( url, NULL );
        ::rtl::OUString propName =  OUSTR("DocumentModel");
        result = getUCBProperty( root, propName );
    }
    catch ( css::ucb::ContentCreationException& )
    {
        // carry on, empty value will be returned
    }
    catch ( css::uno::RuntimeException& )
    {
        // carry on, empty value will be returned
    }

    css::uno::Reference< css::frame::XModel > xModel(
        result, css::uno::UNO_QUERY );

    return xModel;
}


    static css::uno::Any getUCBProperty( ::ucbhelper::Content& content, ::rtl::OUString& prop )
{
    css::uno::Any result;
    try
    {
        result = content.getPropertyValue( prop );
    }
    catch ( css::uno::Exception& )
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
    INetURLObject aURLObj( temp );
    if ( !aURLObj.HasError() )
        temp = aURLObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
    return temp;
}

};
} // namespace sf_misc
#endif //
