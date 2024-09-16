/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/rdf/Statement.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>
#include <com/sun/star/rdf/XDocumentRepository.hpp>
#include <com/sun/star/rdf/XRepository.hpp>
#include <com/sun/star/rdf/FileFormat.hpp>
#include <com/sun/star/rdf/Literal.hpp>
#include <com/sun/star/rdf/Repository.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>

using namespace com::sun::star;

namespace
{
class RDFStreamTest : public UnoApiTest
{
public:
    RDFStreamTest()
        : UnoApiTest(u"/unoxml/qa/unit/data/"_ustr)
    {
    }
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

CPPUNIT_TEST_FIXTURE(RDFStreamTest, testRDFa)
{
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xTextRange(xParaEnum->nextElement(), uno::UNO_QUERY);

    xTextRange->setString(u"behold, for I am the content."_ustr);

    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<rdf::XRepository> xRepo = xDocumentMetadataAccess->getRDFRepository();
    uno::Reference<rdf::XDocumentRepository> xDocRepo(xRepo, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocRepo);

    uno::Reference<rdf::XMetadatable> xMeta(xTextRange, uno::UNO_QUERY);

    // 1. RDFa: get: not empty (initial)
    ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
        = xDocRepo->getStatementRDFa(xMeta);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), xResult.First.size());
    CPPUNIT_ASSERT(!xResult.Second);

    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    uno::Reference<css::rdf::XURI> xFoo = rdf::URI::create(xContext, "uri:foo");
    uno::Reference<css::rdf::XURI> xBar = rdf::URI::create(xContext, "uri:bar");
    uno::Reference<css::rdf::XURI> xBaz = rdf::URI::create(xContext, "uri:baz");
    uno::Reference<css::rdf::XURI> xInt = rdf::URI::create(xContext, "uri:int");
    css::uno::Sequence<uno::Reference<rdf::XURI>> xURI{};

    // 2. RDFa: set: no predicate
    try
    {
        xDocRepo->setStatementRDFa(xFoo, xURI, xMeta, OUString(), nullptr);
        CPPUNIT_FAIL("expected IllegalArgumentException");
    }
    catch (css::lang::IllegalArgumentException&)
    {
    }

    css::uno::Sequence<uno::Reference<rdf::XURI>> xURI2{ xBar };

    // 3. RDFa: set: null
    try
    {
        xDocRepo->setStatementRDFa(xFoo, xURI2, nullptr, OUString(), nullptr);
        CPPUNIT_FAIL("expected IllegalArgumentException");
    }
    catch (css::lang::IllegalArgumentException&)
    {
    }

    xDocRepo->setStatementRDFa(xFoo, xURI2, xMeta, OUString(), nullptr);

    // 4. RDFa: get: without content
    xResult = xDocRepo->getStatementRDFa(xMeta);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), xResult.First.size());
    CPPUNIT_ASSERT(!xResult.Second);

    rdf::Statement aStatement = xResult.First[0];
    CPPUNIT_ASSERT_EQUAL(OUString("uri:foo"), aStatement.Subject->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("uri:bar"), aStatement.Predicate->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("behold, for I am the content."),
                         aStatement.Object->getStringValue());

    xDocRepo->setStatementRDFa(xFoo, xURI2, xMeta, u"42"_ustr, xInt);

    // 5. RDFa: get: with content
    xResult = xDocRepo->getStatementRDFa(xMeta);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), xResult.First.size());
    CPPUNIT_ASSERT(xResult.Second);

    aStatement = xResult.First[0];
    CPPUNIT_ASSERT_EQUAL(OUString("uri:foo"), aStatement.Subject->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("uri:bar"), aStatement.Predicate->getStringValue());

    uno::Reference<css::rdf::XLiteral> xLitType
        = rdf::Literal::createWithType(xContext, "42", xInt);
    CPPUNIT_ASSERT_EQUAL(xLitType->getStringValue(), aStatement.Object->getStringValue());

    xDocRepo->removeStatementRDFa(xMeta);

    // 6. RDFa: get: not empty (removed)
    xResult = xDocRepo->getStatementRDFa(xMeta);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), xResult.First.size());
    CPPUNIT_ASSERT(xResult.Second);

    css::uno::Sequence<uno::Reference<rdf::XURI>> xURI3{ xFoo, xBar, xBaz };

    xDocRepo->setStatementRDFa(xFoo, xURI3, xMeta, OUString(), nullptr);

    // 7. RDFa: get: without content (multiple predicates, reinsert)
    xResult = xDocRepo->getStatementRDFa(xMeta);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(3), xResult.First.size());
    CPPUNIT_ASSERT(!xResult.Second);

    aStatement = xResult.First[0];
    CPPUNIT_ASSERT_EQUAL(OUString("uri:foo"), aStatement.Subject->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("uri:baz"), aStatement.Predicate->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("behold, for I am the content."),
                         aStatement.Object->getStringValue());

    aStatement = xResult.First[1];
    CPPUNIT_ASSERT_EQUAL(OUString("uri:foo"), aStatement.Subject->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("uri:bar"), aStatement.Predicate->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("behold, for I am the content."),
                         aStatement.Object->getStringValue());

    aStatement = xResult.First[2];
    CPPUNIT_ASSERT_EQUAL(OUString("uri:foo"), aStatement.Subject->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("uri:foo"), aStatement.Predicate->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("behold, for I am the content."),
                         aStatement.Object->getStringValue());

    xDocRepo->removeStatementRDFa(xMeta);

    // 8. RDFa: get: not empty (re-removed)
    xResult = xDocRepo->getStatementRDFa(xMeta);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), xResult.First.size());
    CPPUNIT_ASSERT(!xResult.Second);
}

CPPUNIT_TEST_FIXTURE(RDFStreamTest, testSPARQL)
{
    const uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext(),
                                                          css::uno::UNO_SET_THROW);
    const uno::Reference<com::sun::star::ucb::XSimpleFileAccess> xFileAccess(
        xContext->getServiceManager()->createInstanceWithContext(
            u"com.sun.star.ucb.SimpleFileAccess"_ustr, xContext),
        uno::UNO_QUERY_THROW);
    const uno::Reference<io::XInputStream> xInputStream(
        xFileAccess->openFileRead(m_directories.getURLFromSrc(u"/unoxml/qa/unit/data/example.rdf")),
        uno::UNO_SET_THROW);
    uno::Reference<rdf::XRepository> xRepo = rdf::Repository::create(xContext);
    uno::Reference<rdf::XDocumentRepository> xDocRepo(xRepo, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocRepo);

    uno::Reference<css::rdf::XURI> xManifest = rdf::URI::create(xContext, "manifest:manifest");
    uno::Reference<css::rdf::XURI> xBase = rdf::URI::create(xContext, "base-uri:");
    uno::Reference<css::rdf::XURI> xFoo = rdf::URI::create(xContext, "uri:foo");
    uno::Reference<css::rdf::XURI> xBar = rdf::URI::create(xContext, "uri:bar");

    xDocRepo->importGraph(rdf::FileFormat::RDF_XML, xInputStream, xManifest, xBase);

    OUString sNss("PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>\n"
                  "PREFIX pkg: <http://docs.oasis-open.org/opendocument/meta/package/common#>\n"
                  "PREFIX odf: <http://docs.oasis-open.org/opendocument/meta/package/odf#>\n");

    // 1. query: package-id
    OUString sQuery("SELECT ?p WHERE { ?p rdf:type pkg:Package . }");
    uno::Reference<rdf::XQuerySelectResult> aResult = xDocRepo->querySelect(sNss + sQuery);
    uno::Sequence<OUString> aBindings = aResult->getBindingNames();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aBindings.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("p"), aBindings[0]);

    uno::Sequence<uno::Reference<rdf::XNode>> aNode;
    css::uno::fromAny(aResult->nextElement(), &aNode);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aNode.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("urn:uuid:224ab023-77b8-4396-a75a-8cecd85b81e3"),
                         aNode[0]->getStringValue());

    CPPUNIT_ASSERT(!aResult->hasMoreElements());

    // 2. query: contentfile
    sQuery = "SELECT ?part ?path FROM <manifest:manifest> WHERE { ?pkg rdf:type pkg:Package . ?pkg "
             "pkg:hasPart ?part . ?part "
             "pkg:path ?path . ?part rdf:type odf:ContentFile. }";
    aResult = xDocRepo->querySelect(sNss + sQuery);
    aBindings = aResult->getBindingNames();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBindings.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("part"), aBindings[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("path"), aBindings[1]);

    css::uno::fromAny(aResult->nextElement(), &aNode);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNode.getLength());
    CPPUNIT_ASSERT(!aNode[0]->getStringValue().isEmpty());
    CPPUNIT_ASSERT_EQUAL(OUString("content.xml"), aNode[1]->getStringValue());

    CPPUNIT_ASSERT(!aResult->hasMoreElements());

    // 3. query: contentfile
    sQuery = "SELECT ?pkg ?path FROM <manifest:manifest> WHERE { ?pkg rdf:type pkg:Package . ?pkg "
             "pkg:hasPart ?part . ?part pkg:path ?path . ?part rdf:type odf:ContentFile. }";
    aResult = xDocRepo->querySelect(sNss + sQuery);
    aBindings = aResult->getBindingNames();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBindings.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("pkg"), aBindings[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("path"), aBindings[1]);

    css::uno::fromAny(aResult->nextElement(), &aNode);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNode.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("urn:uuid:224ab023-77b8-4396-a75a-8cecd85b81e3"),
                         aNode[0]->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("content.xml"), aNode[1]->getStringValue());

    CPPUNIT_ASSERT(!aResult->hasMoreElements());

    // 4. query: stylesfile
    sQuery = "SELECT ?part ?path FROM <manifest:manifest> WHERE { ?pkg rdf:type pkg:Package . ?pkg "
             "pkg:hasPart ?part . ?part pkg:path ?path . ?part rdf:type odf:StylesFile. }";
    aResult = xDocRepo->querySelect(sNss + sQuery);
    aBindings = aResult->getBindingNames();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBindings.getLength());

    CPPUNIT_ASSERT(!aResult->hasMoreElements());

    // 5. query: metadatafile
    sQuery = "SELECT ?part ?path FROM <manifest:manifest> WHERE { ?pkg rdf:type pkg:Package . ?pkg "
             "pkg:hasPart ?part . ?part pkg:path ?path . ?part rdf:type odf:MetadataFile. }";
    aResult = xDocRepo->querySelect(sNss + sQuery);
    aBindings = aResult->getBindingNames();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBindings.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("part"), aBindings[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("path"), aBindings[1]);

    css::uno::fromAny(aResult->nextElement(), &aNode);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNode.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("http://hospital-employee/doctor"), aNode[0]->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("meta/hospital/doctor.rdf"), aNode[1]->getStringValue());

    CPPUNIT_ASSERT(!aResult->hasMoreElements());

    //FIXME redland BUG
    // 6. query: metadatafile
    sQuery = "SELECT ?path ?idref FROM <manifest:manifest> WHERE { "
             "<urn:uuid:224ab023-77b8-4396-a75a-8cecd85b81e3> pkg:hasPart ?part . ?part pkg:path "
             "?path ;  rdf:type ?type ;  pkg:hasPart <uri:example-element-2> . "
             "<uri:example-element-2>  pkg:idref ?idref .  FILTER (?type = odf:ContentFile || "
             "?type = odf:StylesFile) }";
    aResult = xDocRepo->querySelect(sNss + sQuery);
    aBindings = aResult->getBindingNames();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBindings.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("path"), aBindings[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("idref"), aBindings[1]);

    css::uno::fromAny(aResult->nextElement(), &aNode);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aNode.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("content.xml"), aNode[0]->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("ID_B"), aNode[1]->getStringValue());

    CPPUNIT_ASSERT(!aResult->hasMoreElements());

    // 7. query: construct
    sQuery = "CONSTRUCT { ?pkg <uri:foo> \"I am the literal\" } FROM <manifest:manifest> WHERE { "
             "?pkg rdf:type pkg:Package . } ";
    uno::Reference<container::XEnumeration> aResultEnum = xDocRepo->queryConstruct(sNss + sQuery);

    rdf::Statement aStatement = aResultEnum->nextElement().get<rdf::Statement>();

    CPPUNIT_ASSERT_EQUAL(OUString("urn:uuid:224ab023-77b8-4396-a75a-8cecd85b81e3"),
                         aStatement.Subject->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("uri:foo"), aStatement.Predicate->getStringValue());
    CPPUNIT_ASSERT_EQUAL(OUString("I am the literal"), aStatement.Object->getStringValue());

    CPPUNIT_ASSERT(!aResultEnum->hasMoreElements());

    // 8. query: ask
    sQuery = "ASK { ?pkg rdf:type pkg:Package . }";
    CPPUNIT_ASSERT(xDocRepo->queryAsk(sNss + sQuery));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
