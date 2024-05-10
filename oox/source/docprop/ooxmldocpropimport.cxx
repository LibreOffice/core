/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "ooxmldocpropimport.hxx"

#include <vector>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <oox/core/fastparser.hxx>
#include <oox/core/relations.hxx>
#include "docprophandler.hxx"

#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;

namespace oox::docprop {

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace {

/// @throws RuntimeException
/// @throws css::io::IOException
Sequence< InputSource > lclGetRelatedStreams( const Reference< XStorage >& rxStorage, const OUString& rStreamType )
{
    Reference< XRelationshipAccess > xRelation( rxStorage, UNO_QUERY_THROW );
    Reference< XHierarchicalStorageAccess > xHierarchy( rxStorage, UNO_QUERY_THROW );

    const Sequence< Sequence< StringPair > > aPropsInfo = xRelation->getRelationshipsByType( rStreamType );

    ::std::vector< InputSource > aResult;

    for( const Sequence< StringPair >& rEntries : aPropsInfo )
    {
        auto pEntry = std::find_if(rEntries.begin(), rEntries.end(),
            [](const StringPair& rEntry) { return rEntry.First == "Target"; });
        if (pEntry != rEntries.end())
        {
            // The stream path is always a relative one, ignore the leading "/" if it's there.
            OUString aStreamPath = pEntry->Second;
            if (aStreamPath.startsWith("/"))
                aStreamPath = aStreamPath.copy(1);

            Reference< XExtendedStorageStream > xExtStream(
                xHierarchy->openStreamElementByHierarchicalName( aStreamPath, ElementModes::READ ), UNO_SET_THROW );
            Reference< XInputStream > xInStream = xExtStream->getInputStream();
            if( xInStream.is() )
            {
                aResult.emplace_back();
                aResult.back().sSystemId = pEntry->Second;
                aResult.back().aInputStream = xExtStream->getInputStream();
            }
        }
    }

    return comphelper::containerToSequence( aResult );
}

Sequence< InputSource > lclGetCoreStreams(const Reference< XStorage >& rxSource)
{
    Sequence< InputSource > aCoreStreams = lclGetRelatedStreams(rxSource, CREATE_OFFICEDOC_RELATION_TYPE("metadata/core-properties"));
    // OOXML strict
    if (!aCoreStreams.hasElements())
        aCoreStreams = lclGetRelatedStreams(rxSource, CREATE_OFFICEDOC_RELATION_TYPE_STRICT("metadata/core-properties"));
    // MS Office seems to have a bug, so we have to do similar handling
    if (!aCoreStreams.hasElements())
        aCoreStreams = lclGetRelatedStreams(rxSource, u"http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties"_ustr);

    return aCoreStreams;
}

Sequence< InputSource > lclGetExtStreams(const Reference< XStorage >& rxSource)
{
    Sequence< InputSource > aExtStreams = lclGetRelatedStreams(rxSource, CREATE_OFFICEDOC_RELATION_TYPE("extended-properties"));
    // OOXML strict
    if (!aExtStreams.hasElements())
        aExtStreams = lclGetRelatedStreams(rxSource, CREATE_OFFICEDOC_RELATION_TYPE_STRICT("extended-properties"));

    return aExtStreams;
}

Sequence< InputSource > lclGetCustomStreams(const Reference< XStorage >& rxSource)
{
    Sequence< InputSource > aCustomStreams = lclGetRelatedStreams(rxSource, CREATE_OFFICEDOC_RELATION_TYPE("custom-properties"));
    // OOXML strict
    if (!aCustomStreams.hasElements())
        aCustomStreams = lclGetRelatedStreams(rxSource, CREATE_OFFICEDOC_RELATION_TYPE_STRICT("custom-properties"));

    return aCustomStreams;
}


} // namespace

DocumentPropertiesImport::DocumentPropertiesImport( const Reference< XComponentContext >& rxContext ) :
    mxContext( rxContext )
{
}

// XServiceInfo
OUString SAL_CALL DocumentPropertiesImport::getImplementationName()
{
    return u"com.sun.star.comp.oox.docprop.DocumentPropertiesImporter"_ustr;
}

sal_Bool SAL_CALL DocumentPropertiesImport::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL DocumentPropertiesImport::getSupportedServiceNames()
{
    Sequence<OUString> aServices { u"com.sun.star.document.OOXMLDocumentPropertiesImporter"_ustr };
    return aServices;
}

// XOOXMLDocumentPropertiesImporter
void SAL_CALL DocumentPropertiesImport::importProperties(
        const Reference< XStorage >& rxSource, const Reference< XDocumentProperties >& rxDocumentProperties )
{
    if( !mxContext.is() )
        throw RuntimeException();

    if( !rxSource.is() || !rxDocumentProperties.is() )
        throw IllegalArgumentException();

    Sequence< InputSource > aCoreStreams = lclGetCoreStreams(rxSource);

    Sequence< InputSource > aExtStreams = lclGetExtStreams(rxSource);

    Sequence< InputSource > aCustomStreams = lclGetCustomStreams(rxSource);

    if( !(aCoreStreams.hasElements() || aExtStreams.hasElements() || aCustomStreams.hasElements()) )
        return;

    if( aCoreStreams.getLength() > 1 )
        throw IOException( u"Unexpected core properties stream!"_ustr );

    ::oox::core::FastParser aParser;
    aParser.registerNamespace( NMSP_packageMetaCorePr );
    aParser.registerNamespace( NMSP_dc );
    aParser.registerNamespace( NMSP_dcTerms );
    aParser.registerNamespace( NMSP_officeExtPr );
    aParser.registerNamespace( NMSP_officeCustomPr );
    aParser.registerNamespace( NMSP_officeDocPropsVT );
    aParser.setDocumentHandler( new OOXMLDocPropHandler( mxContext, rxDocumentProperties ) );

    if( aCoreStreams.hasElements() )
        aParser.parseStream( aCoreStreams[ 0 ], true );
    for (const auto& rExtStream : aExtStreams)
        aParser.parseStream( rExtStream, true );
    for (const auto& rCustomStream : aCustomStreams)
        aParser.parseStream( rCustomStream, true );
}

Reference < com::sun::star::io::XInputStream > SAL_CALL DocumentPropertiesImport::getCorePropertiesStream(
    const Reference< XStorage >& rxSource)
{
    Sequence< InputSource > aCoreStreams = lclGetCoreStreams(rxSource);
    if (!aCoreStreams.hasElements())
        return nullptr;

    return aCoreStreams[0].aInputStream;
}

Reference < com::sun::star::io::XInputStream > SAL_CALL DocumentPropertiesImport::getExtendedPropertiesStream(
    const Reference< XStorage >& rxSource)
{
    Sequence< InputSource > aExtStreams = lclGetExtStreams(rxSource);
    if (!aExtStreams.hasElements())
        return nullptr;

    return aExtStreams[0].aInputStream;
}

css::uno::Sequence< css::uno::Reference< com::sun::star::io::XInputStream > > SAL_CALL DocumentPropertiesImport::getCustomPropertiesStreams(
    const Reference< XStorage >& rxSource)
{
    Sequence <InputSource> aExtStreams = lclGetCustomStreams(rxSource);

    // Repack the sequence
    std::vector<Reference<XInputStream>> aResult(aExtStreams.getLength());
    for (const auto& aInputSource : aExtStreams)
    {
        aResult.push_back(aInputSource.aInputStream);
    }

    return comphelper::containerToSequence(aResult);
}

} // namespace oox::docprop

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_oox_docprop_DocumentPropertiesImporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new oox::docprop::DocumentPropertiesImport(pCtx));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
