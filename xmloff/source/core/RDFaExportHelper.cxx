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

#include "RDFaExportHelper.hxx"

#include "xmloff/xmlnmspe.hxx"

#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>

#include <comphelper/stl_types.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/rdf/Statement.hpp>
#include <com/sun/star/rdf/URIs.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/XLiteral.hpp>
#include <com/sun/star/rdf/XRepositorySupplier.hpp>
#include <com/sun/star/rdf/XDocumentRepository.hpp>

#include <rtl/ustrbuf.hxx>

#include <boost/bind.hpp>
#include <boost/iterator_adaptors.hpp>
#ifndef BOOST_ITERATOR_ADAPTOR_DWA053000_HPP_ // from iterator_adaptors.hpp
// N.B.: the check for the header guard _of a specific version of boost_
//       is here so this may work on different versions of boost,
//       which sadly put the goods in different header files
#include <boost/iterator/transform_iterator.hpp>
#endif

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;

namespace xmloff {

static OUString
makeCURIE(SvXMLExport * i_pExport,
    uno::Reference<rdf::XURI> const & i_xURI)
{
    OSL_ENSURE(i_xURI.is(), "makeCURIE: null URI");
    if (!i_xURI.is()) throw uno::RuntimeException();

    const OUString Namespace( i_xURI->getNamespace() );
    OSL_ENSURE(!Namespace.isEmpty(), "makeCURIE: no namespace");
    if (Namespace.isEmpty()) throw uno::RuntimeException();

    OUStringBuffer buf;
    buf.append( i_pExport->EnsureNamespace(Namespace) );
    buf.append( static_cast<sal_Unicode>(':') );
    // N.B.: empty LocalName is valid!
    buf.append( i_xURI->getLocalName() );

    return buf.makeStringAndClear();
}

// #i112473# SvXMLExport::GetRelativeReference() not right for RDF on SaveAs
// because the URIs in the repository are not rewritten on SaveAs, the
// URI of the loaded document has to be used, not the URI of the target doc.
static OUString
getRelativeReference(SvXMLExport const& rExport, OUString const& rURI)
{
    uno::Reference< rdf::XURI > const xModelURI(
        rExport.GetModel(), uno::UNO_QUERY_THROW );
    OUString const baseURI( xModelURI->getStringValue() );

    uno::Reference<uno::XComponentContext> xContext( comphelper::getProcessComponentContext() );
    uno::Reference<uri::XUriReferenceFactory> const xUriFactory =
        uri::UriReferenceFactory::create( xContext );

    uno::Reference< uri::XUriReference > const xBaseURI(
        xUriFactory->parse(baseURI), uno::UNO_SET_THROW );
    uno::Reference< uri::XUriReference > const xAbsoluteURI(
        xUriFactory->parse(rURI), uno::UNO_SET_THROW );
    uno::Reference< uri::XUriReference > const xRelativeURI(
        xUriFactory->makeRelative(xBaseURI, xAbsoluteURI, true, true, false),
        uno::UNO_SET_THROW );
    OUString const relativeURI(xRelativeURI->getUriReference());

    return relativeURI;
}

RDFaExportHelper::RDFaExportHelper(SvXMLExport & i_rExport)
    : m_rExport(i_rExport), m_xRepository(0), m_Counter(0)
{
    const uno::Reference<rdf::XRepositorySupplier> xRS( m_rExport.GetModel(),
            uno::UNO_QUERY);
    OSL_ENSURE(xRS.is(), "AddRDFa: model is no rdf::XRepositorySupplier");
    if (!xRS.is()) throw uno::RuntimeException();
    m_xRepository.set(xRS->getRDFRepository(), uno::UNO_QUERY_THROW);
}

OUString
RDFaExportHelper::LookupBlankNode(
    uno::Reference<rdf::XBlankNode> const & i_xBlankNode)
{
    OSL_ENSURE(i_xBlankNode.is(), "null BlankNode?");
    if (!i_xBlankNode.is()) throw uno::RuntimeException();
    OUString & rEntry(
        m_BlankNodeMap[ i_xBlankNode->getStringValue() ] );
    if (rEntry.isEmpty())
    {
        rEntry = "_:b" + OUString::number(++m_Counter);
    }
    return rEntry;
}

void
RDFaExportHelper::AddRDFa(
    uno::Reference<rdf::XMetadatable> const & i_xMetadatable)
{
    try
    {
        beans::Pair< uno::Sequence<rdf::Statement>, sal_Bool > const
            RDFaResult( m_xRepository->getStatementRDFa(i_xMetadatable) );

        uno::Sequence<rdf::Statement> const & rStatements( RDFaResult.First );

        if (0 == rStatements.getLength())
        {
            return; // no RDFa
        }

        // all stmts have the same subject, so we only handle first one
        const uno::Reference<rdf::XURI> xSubjectURI(rStatements[0].Subject,
            uno::UNO_QUERY);
        const uno::Reference<rdf::XBlankNode> xSubjectBNode(
            rStatements[0].Subject, uno::UNO_QUERY);
        if (!xSubjectURI.is() && !xSubjectBNode.is())
        {
            throw uno::RuntimeException();
        }
        const OUString about( xSubjectURI.is()
            ?   getRelativeReference(m_rExport, xSubjectURI->getStringValue())
            :   "[" + LookupBlankNode(xSubjectBNode) + "]"
            );

        const uno::Reference<rdf::XLiteral> xContent(
            rStatements[0].Object, uno::UNO_QUERY_THROW );
        const uno::Reference<rdf::XURI> xDatatype(xContent->getDatatype());
        if (xDatatype.is())
        {
            const OUString datatype(
                makeCURIE(&m_rExport, xDatatype) );
            m_rExport.AddAttribute(XML_NAMESPACE_XHTML,
                token::XML_DATATYPE, datatype);
        }
        if (RDFaResult.Second) // there is xhtml:content
        {
            m_rExport.AddAttribute(XML_NAMESPACE_XHTML, token::XML_CONTENT,
                xContent->getValue());
        }

        OUStringBuffer property;
        ::comphelper::intersperse(
            ::boost::make_transform_iterator(rStatements.begin(),
                ::boost::bind(&makeCURIE, &m_rExport,
                    ::boost::bind(&rdf::Statement::Predicate, _1))),
            // argh, this must be the same type :(
            ::boost::make_transform_iterator(
                rStatements.end(),
                ::boost::bind(&makeCURIE, &m_rExport,
                    ::boost::bind(&rdf::Statement::Predicate, _1))),
            ::comphelper::OUStringBufferAppender(property),
            OUString(" "));

        m_rExport.AddAttribute(XML_NAMESPACE_XHTML, token::XML_PROPERTY,
            property.makeStringAndClear());

        m_rExport.AddAttribute(XML_NAMESPACE_XHTML, token::XML_ABOUT, about);
    }
    catch (uno::Exception &)
    {
        OSL_FAIL("AddRDFa: exception");
    }
}

} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
