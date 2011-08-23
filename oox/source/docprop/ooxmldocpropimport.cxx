/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <oox/core/namespaces.hxx>
#include "docprophandler.hxx"
#include "ooxmldocpropimport.hxx"

using namespace ::com::sun::star;

namespace oox {
namespace docprop {

// -----------------------------------------------------------
::rtl::OUString SAL_CALL OOXMLDocPropImportImpl_getImplementationName()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.oox.docprop.OOXMLDocumentPropertiesImporter"));
}

// -----------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OOXMLDocPropImportImpl_getSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.document.OOXMLDocumentPropertiesImporter"));
    return s;
}

// -----------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OOXMLDocPropImportImpl_createInstance(
    const uno::Reference< uno::XComponentContext > & context)
        SAL_THROW((uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new OOXMLDocPropImportImpl(context));
}


// -----------------------------------------------------------
OOXMLDocPropImportImpl::OOXMLDocPropImportImpl(uno::Reference< uno::XComponentContext > const & xContext) :
    m_xContext( xContext )
{}

// -----------------------------------------------------------
uno::Sequence< xml::sax::InputSource > OOXMLDocPropImportImpl::GetRelatedStreams( const uno::Reference< embed::XStorage > xStorage, const ::rtl::OUString& aStreamType )
{
    if ( !xStorage.is() )
        throw uno::RuntimeException();

    uno::Reference< embed::XRelationshipAccess > xRelation( xStorage, uno::UNO_QUERY_THROW );
    uno::Reference< embed::XHierarchicalStorageAccess > xHierarchy( xStorage, uno::UNO_QUERY_THROW );

    uno::Sequence< uno::Sequence< beans::StringPair > > aPropsInfo = xRelation->getRelationshipsByType( aStreamType );

    sal_Int32 nLength = 0;
    uno::Sequence< xml::sax::InputSource > aResult( aPropsInfo.getLength() );
    if ( aPropsInfo.getLength() )
    {
        for ( sal_Int32 nInd = 0; nInd < aPropsInfo.getLength(); nInd++ )
            for ( sal_Int32 nEntryInd = 0; nEntryInd < aPropsInfo[nInd].getLength(); nEntryInd++ )
                if ( aPropsInfo[nInd][nEntryInd].First.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Target" ) ) ) )
                {
                    uno::Reference< embed::XExtendedStorageStream > xExtStream = xHierarchy->openStreamElementByHierarchicalName( aPropsInfo[nInd][nEntryInd].Second, embed::ElementModes::READ );
                    if ( !xExtStream.is() )
                        throw uno::RuntimeException();

                    aResult[nLength].sSystemId = aPropsInfo[nInd][nEntryInd].Second;
                    aResult[nLength++].aInputStream = xExtStream->getInputStream();
                    break;
                }

        aResult.realloc( nLength );
    }

    return aResult;
}

// com.sun.star.uno.XServiceInfo:
// -----------------------------------------------------------
::rtl::OUString SAL_CALL OOXMLDocPropImportImpl::getImplementationName() throw (uno::RuntimeException)
{
    return OOXMLDocPropImportImpl_getImplementationName();
}

// -----------------------------------------------------------
::sal_Bool SAL_CALL OOXMLDocPropImportImpl::supportsService(::rtl::OUString const & serviceName) throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > serviceNames = OOXMLDocPropImportImpl_getSupportedServiceNames();
    for (::sal_Int32 i = 0; i < serviceNames.getLength(); ++i)
    {
        if (serviceNames[i] == serviceName)
            return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OOXMLDocPropImportImpl::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return OOXMLDocPropImportImpl_getSupportedServiceNames();
}

// -----------------------------------------------------------
// ::com::sun::star::document::XOOXMLDocumentPropertiesImporter:
void SAL_CALL OOXMLDocPropImportImpl::importProperties(const uno::Reference< embed::XStorage > & xSource, const uno::Reference< document::XDocumentProperties > & xDocumentProperties) throw (uno::RuntimeException, lang::IllegalArgumentException, xml::sax::SAXException, uno::Exception)
{
    // TODO: Insert your implementation for "importProperties" here.
    if ( !m_xContext.is() )
        throw uno::RuntimeException();

    if ( !xSource.is() || !xDocumentProperties.is() )
        throw lang::IllegalArgumentException();

    // the MS Office seems to have a bug, so we have to do similar handling
    ::rtl::OUString aCoreType( RTL_CONSTASCII_USTRINGPARAM( "http://schemas.openxmlformats.org/officedocument/2006/relationships/metadata/core-properties" ) );
    ::rtl::OUString aCoreWorkaroundType( RTL_CONSTASCII_USTRINGPARAM( "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" ) );

    ::rtl::OUString aExtType( RTL_CONSTASCII_USTRINGPARAM( "http://schemas.openxmlformats.org/officedocument/2006/relationships/extended-properties" ) );
    ::rtl::OUString aCustomType( RTL_CONSTASCII_USTRINGPARAM( "http://schemas.openxmlformats.org/officedocument/2006/relationships/custom-properties" ) );

    uno::Sequence< xml::sax::InputSource > aCoreStreams = GetRelatedStreams( xSource, aCoreType );
    if ( !aCoreStreams.getLength() )
        aCoreStreams = GetRelatedStreams( xSource, aCoreWorkaroundType );

    uno::Sequence< xml::sax::InputSource > aExtStreams = GetRelatedStreams( xSource, aExtType );
    uno::Sequence< xml::sax::InputSource > aCustomStreams = GetRelatedStreams( xSource, aCustomType );

    if ( aCoreStreams.getLength() || aExtStreams.getLength() || aCustomStreams.getLength() )
    {
        if ( aCoreStreams.getLength() > 1 )
            throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unexpected core properties stream!" ) ), uno::Reference< uno::XInterface >() );

        uno::Reference< lang::XMultiComponentFactory > xFactory( m_xContext->getServiceManager(), uno::UNO_QUERY_THROW );

        uno::Reference< xml::sax::XFastParser > xParser(
            xFactory->createInstanceWithContext(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.FastParser" ) ),
                m_xContext ),
            uno::UNO_QUERY_THROW );

        uno::Reference< xml::sax::XFastTokenHandler > xTokenHandler(
            xFactory->createInstanceWithContext(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.oox.FastTokenHandlerService" ) ),
                m_xContext ),
            uno::UNO_QUERY_THROW );

        uno::Reference< xml::sax::XFastDocumentHandler > xDocHandler( static_cast< xml::sax::XFastDocumentHandler* >( new OOXMLDocPropHandler( m_xContext, xDocumentProperties ) ) );

        xParser->setFastDocumentHandler( xDocHandler );
        xParser->setTokenHandler( xTokenHandler );

        xParser->registerNamespace( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "http://schemas.openxmlformats.org/package/2006/metadata/core-properties" ) ), NMSP_COREPR );
        xParser->registerNamespace( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "http://purl.org/dc/elements/1.1/" ) ), NMSP_DC );
        xParser->registerNamespace( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "http://purl.org/dc/terms/" ) ), NMSP_DCTERMS );
        xParser->registerNamespace( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "http://schemas.openxmlformats.org/officeDocument/2006/extended-properties" ) ), NMSP_EXTPR );
        xParser->registerNamespace( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "http://schemas.openxmlformats.org/officeDocument/2006/custom-properties" ) ), NMSP_CUSTPR );
        xParser->registerNamespace( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes" ) ), NMSP_VT );

        // #158414# XFastParser::parseStream() throws on invalid XML
        if ( aCoreStreams.getLength() ) try
        {
            if ( aCoreStreams[0].aInputStream.is() )
            {
                xParser->parseStream( aCoreStreams[0] );
                aCoreStreams[0].aInputStream->closeInput();
            }
        }
        catch( uno::Exception& )
        {
        }

        sal_Int32 nInd = 0;
        for ( nInd = 0; nInd < aExtStreams.getLength(); nInd++ )
        {
            xParser->parseStream( aExtStreams[nInd] );
            if ( aExtStreams[nInd].aInputStream.is() )
                aExtStreams[nInd].aInputStream->closeInput();
        }

        for ( nInd = 0; nInd < aCustomStreams.getLength(); nInd++ )
        {
            xParser->parseStream( aCustomStreams[nInd] );
            if ( aCustomStreams[nInd].aInputStream.is() )
                aCustomStreams[nInd].aInputStream->closeInput();
        }
    }
}

} // namespace docprop
} // namespace oox


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
