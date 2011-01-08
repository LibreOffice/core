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

#include "ooxmldocpropimport.hxx"

#include <vector>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include "oox/core/fastparser.hxx"
#include "oox/core/relations.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/helper.hxx"
#include "docprophandler.hxx"

namespace oox {
namespace docprop {

// ============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

using ::rtl::OUString;

// ============================================================================

OUString SAL_CALL DocumentPropertiesImport_getImplementationName()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.docprop.DocumentPropertiesImporter" );
}

Sequence< OUString > SAL_CALL DocumentPropertiesImport_getSupportedServiceNames()
{
    Sequence< OUString > aServices( 1 );
    aServices[ 0 ] = CREATE_OUSTRING( "com.sun.star.document.OOXMLDocumentPropertiesImporter" );
    return aServices;
}

Reference< XInterface > SAL_CALL DocumentPropertiesImport_createInstance( const Reference< XComponentContext >& rxContext ) SAL_THROW((Exception))
{
    return static_cast< ::cppu::OWeakObject* >( new DocumentPropertiesImport( rxContext ) );
}

// ============================================================================

namespace {

Sequence< InputSource > lclGetRelatedStreams( const Reference< XStorage >& rxStorage, const OUString& rStreamType ) throw (RuntimeException)
{
    Reference< XRelationshipAccess > xRelation( rxStorage, UNO_QUERY_THROW );
    Reference< XHierarchicalStorageAccess > xHierarchy( rxStorage, UNO_QUERY_THROW );

    Sequence< Sequence< StringPair > > aPropsInfo = xRelation->getRelationshipsByType( rStreamType );

    ::std::vector< InputSource > aResult;

    for( sal_Int32 nIndex = 0, nLength = aPropsInfo.getLength(); nIndex < nLength; ++nIndex )
    {
        const Sequence< StringPair >& rEntries = aPropsInfo[ nIndex ];
        for( sal_Int32 nEntryIndex = 0, nEntryLength = rEntries.getLength(); nEntryIndex < nEntryLength; ++nEntryIndex )
        {
            const StringPair& rEntry = rEntries[ nEntryIndex ];
            if( rEntry.First.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Target" ) ) )
            {
                Reference< XExtendedStorageStream > xExtStream(
                    xHierarchy->openStreamElementByHierarchicalName( rEntry.Second, ElementModes::READ ), UNO_QUERY_THROW );
                Reference< XInputStream > xInStream = xExtStream->getInputStream();
                if( xInStream.is() )
                {
                    aResult.resize( aResult.size() + 1 );
                    aResult.back().sSystemId = rEntry.Second;
                    aResult.back().aInputStream = xExtStream->getInputStream();
                }
                break;
            }
        }
    }

    return ContainerHelper::vectorToSequence( aResult );
}

} // namespace

// ============================================================================

DocumentPropertiesImport::DocumentPropertiesImport( const Reference< XComponentContext >& rxContext ) :
    mxContext( rxContext )
{
}

// XServiceInfo

OUString SAL_CALL DocumentPropertiesImport::getImplementationName() throw (RuntimeException)
{
    return DocumentPropertiesImport_getImplementationName();
}

sal_Bool SAL_CALL DocumentPropertiesImport::supportsService( const OUString& rServiceName ) throw (RuntimeException)
{
    Sequence< OUString > aServiceNames = DocumentPropertiesImport_getSupportedServiceNames();
    for( sal_Int32 nIndex = 0, nLength = aServiceNames.getLength(); nIndex < nLength; ++nIndex )
        if( aServiceNames[ nIndex ] == rServiceName )
            return sal_True;
    return sal_False;
}

Sequence< OUString > SAL_CALL DocumentPropertiesImport::getSupportedServiceNames() throw (RuntimeException)
{
    return DocumentPropertiesImport_getSupportedServiceNames();
}

// XOOXMLDocumentPropertiesImporter

void SAL_CALL DocumentPropertiesImport::importProperties(
        const Reference< XStorage >& rxSource, const Reference< XDocumentProperties >& rxDocumentProperties )
        throw (RuntimeException, IllegalArgumentException, SAXException, Exception)
{
    if( !mxContext.is() )
        throw RuntimeException();

    if( !rxSource.is() || !rxDocumentProperties.is() )
        throw IllegalArgumentException();

    Sequence< InputSource > aCoreStreams = lclGetRelatedStreams( rxSource, CREATE_OFFICEDOC_RELATION_TYPE( "metadata/core-properties" ) );
    // MS Office seems to have a bug, so we have to do similar handling
    if( !aCoreStreams.hasElements() )
        aCoreStreams = lclGetRelatedStreams( rxSource, CREATE_PACKAGE_RELATION_TYPE( "metadata/core-properties" ) );

    Sequence< InputSource > aExtStreams = lclGetRelatedStreams( rxSource, CREATE_OFFICEDOC_RELATION_TYPE( "extended-properties" ) );
    Sequence< InputSource > aCustomStreams = lclGetRelatedStreams( rxSource, CREATE_OFFICEDOC_RELATION_TYPE( "custom-properties" ) );

    if( aCoreStreams.hasElements() || aExtStreams.hasElements() || aCustomStreams.hasElements() )
    {
        if( aCoreStreams.getLength() > 1 )
            throw IOException( CREATE_OUSTRING( "Unexpected core properties stream!" ), Reference< XInterface >() );

        ::oox::core::FastParser aParser( mxContext );
        aParser.registerNamespace( NMSP_packageMetaCorePr );
        aParser.registerNamespace( NMSP_dc );
        aParser.registerNamespace( NMSP_dcTerms );
        aParser.registerNamespace( NMSP_officeExtPr );
        aParser.registerNamespace( NMSP_officeCustomPr );
        aParser.registerNamespace( NMSP_officeDocPropsVT );
        aParser.setDocumentHandler( new OOXMLDocPropHandler( mxContext, rxDocumentProperties ) );

        if( aCoreStreams.hasElements() )
            aParser.parseStream( aCoreStreams[ 0 ], true );
        for( sal_Int32 nIndex = 0; nIndex < aExtStreams.getLength(); ++nIndex )
            aParser.parseStream( aExtStreams[ nIndex ], true );
        for( sal_Int32 nIndex = 0; nIndex < aCustomStreams.getLength(); ++nIndex )
            aParser.parseStream( aCustomStreams[ nIndex ], true );
    }
}

// ============================================================================

} // namespace docprop
} // namespace oox
