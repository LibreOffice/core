/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MiscUtils.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 13:28:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SCRIPT_FRAMEWORK_MISCUTILS_HXX_
#define _SCRIPT_FRAMEWORK_MISCUTILS_HXX_

#include <rtl/ustring.hxx>

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

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
