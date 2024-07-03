/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/rdf/Statement.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>
#include <com/sun/star/rdf/XDocumentRepository.hpp>
#include <com/sun/star/rdf/XRepository.hpp>
#include <com/sun/star/rdf/FileFormat.hpp>
#include <com/sun/star/rdf/Repository.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>

using namespace com::sun::star;

namespace
{
class RDFStreamTest : public test::BootstrapFixture
{
};

CPPUNIT_TEST_FIXTURE(RDFStreamTest, testCVE_2012_0037)
{
    const uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext(),
                                                          css::uno::UNO_SET_THROW);
    const uno::Reference<com::sun::star::ucb::XSimpleFileAccess> xFileAccess(
        xContext->getServiceManager()->createInstanceWithContext(
            u"com.sun.star.ucb.SimpleFileAccess"_ustr, xContext),
        uno::UNO_QUERY_THROW);
    const uno::Reference<io::XInputStream> xInputStream(
        xFileAccess->openFileRead(
            m_directories.getURLFromSrc(u"/unoxml/qa/unit/data/cve_2012_0037.rdf")),
        uno::UNO_SET_THROW);
    uno::Reference<rdf::XRepository> xRepo = rdf::Repository::create(xContext);
    uno::Reference<rdf::XDocumentRepository> xDocRepo(xRepo, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocRepo);

    uno::Reference<css::rdf::XURI> xManifest = rdf::URI::create(xContext, "manifest:manifest");
    uno::Reference<css::rdf::XURI> xBase = rdf::URI::create(xContext, "base-uri:");
    uno::Reference<css::rdf::XURI> xFoo = rdf::URI::create(xContext, "uri:foo");
    uno::Reference<css::rdf::XURI> xBar = rdf::URI::create(xContext, "uri:bar");

    xDocRepo->importGraph(rdf::FileFormat::RDF_XML, xInputStream, xManifest, xBase);
    uno::Reference<rdf::XNamedGraph> xGraph = xDocRepo->getGraph(xManifest);
    CPPUNIT_ASSERT(xGraph);
    uno::Reference<container::XEnumeration> xEnum = xGraph->getStatements(xFoo, xBar, nullptr);

    rdf::Statement aStatement = xEnum->nextElement().get<rdf::Statement>();
    CPPUNIT_ASSERT_EQUAL(OUString("uri:foo"), aStatement.Subject->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("uri:bar"), aStatement.Predicate->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("EVIL"), aStatement.Object->getStringValue());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
