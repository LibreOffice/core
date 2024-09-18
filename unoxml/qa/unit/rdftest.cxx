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
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/rdf/Statement.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>
#include <com/sun/star/rdf/XDocumentRepository.hpp>
#include <com/sun/star/rdf/XRepository.hpp>
#include <com/sun/star/rdf/BlankNode.hpp>
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

inline void assertStatementEqual(const rdf::Statement& rExpected, const rdf::Statement& rActual,
                                 const CppUnit::SourceLine& rSourceLine)
{
    CPPUNIT_NS::assertEquals(rExpected.Object->getStringValue(), rActual.Object->getStringValue(),
                             rSourceLine, "different Object");
    CPPUNIT_NS::assertEquals(rExpected.Predicate->getStringValue(),
                             rActual.Predicate->getStringValue(), rSourceLine,
                             "different Predicate");
    CPPUNIT_NS::assertEquals(rExpected.Subject->getStringValue(), rActual.Subject->getStringValue(),
                             rSourceLine, "different Subject");

    if (rExpected.Graph)
        CPPUNIT_NS::assertEquals(rExpected.Graph->getStringValue(), rActual.Graph->getStringValue(),
                                 rSourceLine, "different Graph");
}

#define CPPUNIT_ASSERT_STATEMENT_EQUAL(aExpected, aActual)                                         \
    assertStatementEqual(aExpected, aActual, CPPUNIT_SOURCELINE())

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

    rdf::Statement aFooBarEvil(xFoo, xBar, rdf::Literal::create(xContext, "EVIL"), xManifest);
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarEvil, xEnum->nextElement().get<rdf::Statement>());
}

CPPUNIT_TEST_FIXTURE(RDFStreamTest, testRDFa)
{
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xTextRange(xParaEnum->nextElement(), uno::UNO_QUERY);

    OUString aText(u"behold, for I am the content."_ustr);
    const uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext(),
                                                          css::uno::UNO_SET_THROW);
    uno::Reference<css::rdf::XLiteral> xLitText = rdf::Literal::create(xContext, aText);
    xTextRange->setString(aText);

    uno::Reference<rdf::XMetadatable> xMeta(xTextRange, uno::UNO_QUERY);

    uno::Reference<rdf::XRepository> xRepo = rdf::Repository::create(xContext);
    uno::Reference<rdf::XDocumentRepository> xDocRepo(xRepo, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocRepo);

    // 1. RDFa: get: not empty (initial)
    ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
        = xDocRepo->getStatementRDFa(xMeta);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), xResult.First.size());
    CPPUNIT_ASSERT(!xResult.Second);

    css::uno::Sequence<uno::Reference<rdf::XURI>> xURI{};

    uno::Reference<css::rdf::XURI> xFoo = rdf::URI::create(xContext, "uri:foo");
    uno::Reference<css::rdf::XURI> xBar = rdf::URI::create(xContext, "uri:bar");
    uno::Reference<css::rdf::XURI> xBaz = rdf::URI::create(xContext, "uri:baz");
    uno::Reference<css::rdf::XURI> xInt = rdf::URI::create(xContext, "uri:int");
    uno::Reference<css::rdf::XLiteral> xLitType
        = rdf::Literal::createWithType(xContext, "42", xInt);

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
    rdf::Statement aFooBarTRLit(xFoo, xBar, xLitText, nullptr);
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarTRLit, xResult.First[0]);

    xDocRepo->setStatementRDFa(xFoo, xURI2, xMeta, u"42"_ustr, xInt);

    // 5. RDFa: get: with content
    xResult = xDocRepo->getStatementRDFa(xMeta);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), xResult.First.size());
    CPPUNIT_ASSERT(xResult.Second);

    rdf::Statement aFooBarLittype(xFoo, xBar, xLitType, nullptr);
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarLittype, xResult.First[0]);

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

    rdf::Statement aFooBazTRLit(xFoo, xBaz, xLitText, nullptr);
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBazTRLit, xResult.First[0]);

    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarTRLit, xResult.First[1]);

    rdf::Statement aFooFooTRLit(xFoo, xFoo, xLitText, nullptr);
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooFooTRLit, xResult.First[2]);

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
        xFileAccess->openFileRead(createFileURL(u"example.rdf")), uno::UNO_SET_THROW);
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

    uno::Reference<css::rdf::XURI> xUuid
        = rdf::URI::create(xContext, "urn:uuid:224ab023-77b8-4396-a75a-8cecd85b81e3");
    uno::Reference<css::rdf::XLiteral> xLit = rdf::Literal::create(xContext, "I am the literal");
    rdf::Statement aPkgFooLit(xUuid, xFoo, xLit, nullptr);
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aPkgFooLit, aResultEnum->nextElement().get<rdf::Statement>());

    CPPUNIT_ASSERT(!aResultEnum->hasMoreElements());

    // 8. query: ask
    sQuery = "ASK { ?pkg rdf:type pkg:Package . }";
    CPPUNIT_ASSERT(xDocRepo->queryAsk(sNss + sQuery));
}

CPPUNIT_TEST_FIXTURE(RDFStreamTest, testRDF)
{
    const uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext(),
                                                          css::uno::UNO_SET_THROW);
    uno::Reference<rdf::XRepository> xRepo = rdf::Repository::create(xContext);
    uno::Reference<rdf::XDocumentRepository> xDocRepo(xRepo, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocRepo);
    uno::Reference<css::rdf::XURI> xFoo = rdf::URI::create(xContext, "uri:foo");
    uno::Reference<css::rdf::XURI> xBase = rdf::URI::create(xContext, "base-uri:");
    uno::Reference<css::rdf::XURI> xBar = rdf::URI::create(xContext, "uri:bar");
    uno::Reference<css::rdf::XURI> xBaz = rdf::URI::create(xContext, "uri:baz");
    uno::Reference<css::rdf::XLiteral> xLit = rdf::Literal::create(xContext, "I am the literal");
    uno::Reference<css::rdf::XURI> xInt = rdf::URI::create(xContext, "uri:int");
    uno::Reference<css::rdf::XLiteral> xLitType
        = rdf::Literal::createWithType(xContext, "42", xInt);
    uno::Reference<css::rdf::XBlankNode> xBlank = rdf::BlankNode::create(xContext, "_:uno");

    // 1. empty: graphs
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), xDocRepo->getGraphNames().size());

    uno::Reference<container::XEnumeration> xEnum
        = xDocRepo->getStatements(nullptr, nullptr, nullptr);

    // 2. empty: stmts
    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    uno::Reference<rdf::XNamedGraph> xGraph = xDocRepo->createGraph(xFoo);

    // 3. foo graph
    CPPUNIT_ASSERT(xGraph);

    // 4. creating duplicate graph is not allowed
    try
    {
        xDocRepo->createGraph(xFoo);
        CPPUNIT_FAIL("expected ElementExistException");
    }
    catch (com::sun::star::container::ElementExistException&)
    {
    }

    // 5. invalid graph name is not allowed
    try
    {
        xDocRepo->createGraph(nullptr);
        CPPUNIT_FAIL("expected IllegalArgumentException");
    }
    catch (css::lang::IllegalArgumentException&)
    {
    }

    // 6. foo graph in getGraphNames
    css::uno::Sequence<css::uno::Reference<css::rdf::XURI>> xURI = xDocRepo->getGraphNames();
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), xURI.size());
    CPPUNIT_ASSERT_EQUAL(xFoo->getStringValue(), xURI[0]->getStringValue());

    // 7. foo graph
    uno::Reference<rdf::XNamedGraph> xFooGraph = xDocRepo->getGraph(xFoo);
    CPPUNIT_ASSERT(xFooGraph);

    xEnum = xDocRepo->getStatements(nullptr, nullptr, nullptr);

    // 8. empty: stmts
    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    const uno::Reference<com::sun::star::ucb::XSimpleFileAccess> xFileAccess(
        xContext->getServiceManager()->createInstanceWithContext(
            u"com.sun.star.ucb.SimpleFileAccess"_ustr, xContext),
        uno::UNO_QUERY_THROW);
    uno::Reference<io::XOutputStream> xOutputStream(
        xFileAccess->openFileWrite(maTempFile.GetURL()));
    xDocRepo->exportGraph(rdf::FileFormat::RDF_XML, xOutputStream, xFoo, xBase);
    xOutputStream->closeOutput();

    uno::Reference<io::XInputStream> xInputStream(xFileAccess->openFileRead(maTempFile.GetURL()),
                                                  uno::UNO_SET_THROW);
    xDocRepo->importGraph(rdf::FileFormat::RDF_XML, xInputStream, xBar, xBase);

    // 7. bar graph
    CPPUNIT_ASSERT(xDocRepo->getGraph(xBar));

    xFooGraph->addStatement(xFoo, xBar, xBaz);

    xEnum = xFooGraph->getStatements(nullptr, nullptr, nullptr);

    // 8. addStatement(foo,bar,baz)
    rdf::Statement aFoo_FooBarBaz(xFoo, xBar, xBaz, xFoo);
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBaz, xEnum->nextElement().get<rdf::Statement>());

    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    xFooGraph->addStatement(xFoo, xBar, xBlank);

    xEnum = xFooGraph->getStatements(nullptr, nullptr, nullptr);

    // 9. addStatement(foo,bar,blank)
    rdf::Statement aFoo_FooBarBlank(xFoo, xBar, xBlank, xFoo);
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBlank, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBaz, xEnum->nextElement().get<rdf::Statement>());

    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    xFooGraph->addStatement(xBaz, xBar, xLit);

    xEnum = xFooGraph->getStatements(nullptr, nullptr, nullptr);

    // 10. addStatement(baz,bar,lit)
    rdf::Statement aFoo_BazBarLit(xBaz, xBar, xLit, xFoo);
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLit, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBlank, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBaz, xEnum->nextElement().get<rdf::Statement>());

    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    xEnum = xFooGraph->getStatements(xBaz, xBar, nullptr);

    // 11. addStatement(baz,bar,lit) (baz,bar)
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLit, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    uno::Reference<css::rdf::XLiteral> xLitLang
        = rdf::Literal::createWithLanguage(xContext, "I am the literal", "en");
    xFooGraph->addStatement(xBaz, xBar, xLitLang);

    xEnum = xFooGraph->getStatements(nullptr, nullptr, nullptr);

    // 12. addStatement(baz,bar,litlang)
    rdf::Statement aFoo_BazBarLitlang(xBaz, xBar, xLitLang, xFoo);
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLitlang, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLit, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBlank, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBaz, xEnum->nextElement().get<rdf::Statement>());

    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    xEnum = xFooGraph->getStatements(nullptr, nullptr, xBaz);

    // 13. addStatement(baz,bar,litlang) (baz)
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBaz, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    xFooGraph->addStatement(xBaz, xBar, xLitType);

    xEnum = xFooGraph->getStatements(nullptr, nullptr, nullptr);

    // 14. addStatement(baz,bar,littype)
    rdf::Statement aFoo_BazBarLitType(xBaz, xBar, xLitType, xFoo);
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLitType, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLitlang, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLit, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBlank, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBaz, xEnum->nextElement().get<rdf::Statement>());

    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    xFooGraph->removeStatements(xBaz, xBar, xLitLang);

    xEnum = xFooGraph->getStatements(nullptr, nullptr, nullptr);

    // 15. removeStatement(baz,bar,litlang)
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLitType, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLit, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBlank, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBaz, xEnum->nextElement().get<rdf::Statement>());

    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    xFooGraph->removeStatements(xFoo, xBar, nullptr);

    xEnum = xFooGraph->getStatements(nullptr, nullptr, nullptr);

    // 16. removeStatement(foo,bar,null)
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLitType, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLit, xEnum->nextElement().get<rdf::Statement>());

    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    xFooGraph->addStatement(xFoo, xBar, xBaz);

    xEnum = xFooGraph->getStatements(nullptr, nullptr, nullptr);

    // 17. addStatement(foo,bar,baz) (re-add)
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBaz, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLitType, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLit, xEnum->nextElement().get<rdf::Statement>());

    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    xFooGraph->addStatement(xFoo, xBar, xBaz);

    xEnum = xFooGraph->getStatements(nullptr, nullptr, nullptr);

    // 18. addStatement(foo,bar,baz) (duplicate)
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBaz, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLitType, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLit, xEnum->nextElement().get<rdf::Statement>());

    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    xFooGraph->addStatement(xFooGraph, xBar, xBaz);

    xEnum = xFooGraph->getStatements(nullptr, nullptr, nullptr);

    // 19. addStatement(foo,bar,baz) (triplicate, as graph)
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_FooBarBaz, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLitType, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aFoo_BazBarLit, xEnum->nextElement().get<rdf::Statement>());

    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    xOutputStream.set(xFileAccess->openFileWrite(maTempFile.GetURL()));
    xDocRepo->exportGraph(rdf::FileFormat::RDF_XML, xOutputStream, xFoo, xBase);
    xOutputStream->closeOutput();

    xInputStream.set(xFileAccess->openFileRead(maTempFile.GetURL()), uno::UNO_SET_THROW);
    try
    {
        xDocRepo->importGraph(rdf::FileFormat::RDF_XML, xInputStream, xBar, xBase);
        CPPUNIT_FAIL("importing existing graph did not fail");
    }
    catch (com::sun::star::container::ElementExistException&)
    {
    }

    xInputStream.set(xFileAccess->openFileRead(maTempFile.GetURL()), uno::UNO_SET_THROW);
    xDocRepo->importGraph(rdf::FileFormat::RDF_XML, xInputStream, xBaz, xBase);
    uno::Reference<rdf::XNamedGraph> xBazGraph = xDocRepo->getGraph(xBaz);
    CPPUNIT_ASSERT(xBazGraph);

    rdf::Statement aBaz_FooBarBaz(xFoo, xBar, xBaz, xBaz);
    rdf::Statement aBaz_FooBarLit(xBaz, xBar, xLit, xBaz);
    rdf::Statement aBaz_FooBarLitType(xBaz, xBar, xLitType, xBaz);

    xEnum = xBazGraph->getStatements(nullptr, nullptr, nullptr);

    // 20. importing exported graph
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aBaz_FooBarBaz, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aBaz_FooBarLitType, xEnum->nextElement().get<rdf::Statement>());
    CPPUNIT_ASSERT_STATEMENT_EQUAL(aBaz_FooBarLit, xEnum->nextElement().get<rdf::Statement>());

    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    // 21. Checking graph deletion
    xFooGraph->clear();
    xEnum = xFooGraph->getStatements(nullptr, nullptr, nullptr);
    CPPUNIT_ASSERT(!xEnum->hasMoreElements());

    // 22. baz graph zombie
    xDocRepo->destroyGraph(xBaz);
    CPPUNIT_ASSERT(!xDocRepo->getGraph(xBaz));

    try
    {
        xBazGraph->clear();
        CPPUNIT_FAIL("deleted graph not invalid (clear)");
    }
    catch (com::sun::star::container::NoSuchElementException&)
    {
    }

    try
    {
        xBazGraph->addStatement(xFoo, xFoo, xFoo);
        CPPUNIT_FAIL("deleted graph not invalid (add)");
    }
    catch (com::sun::star::container::NoSuchElementException&)
    {
    }

    try
    {
        xBazGraph->removeStatements(nullptr, nullptr, nullptr);
        CPPUNIT_FAIL("deleted graph not invalid (remove)");
    }
    catch (com::sun::star::container::NoSuchElementException&)
    {
    }

    try
    {
        xBazGraph->getStatements(nullptr, nullptr, nullptr);
        CPPUNIT_FAIL("deleted graph not invalid (remove)");
    }
    catch (com::sun::star::container::NoSuchElementException&)
    {
    }
}

std::vector<rdf::Statement>
sortStatementsByPredicate(const uno::Sequence<rdf::Statement>& rStatements)
{
    std::vector<rdf::Statement> aStatements
        = comphelper::sequenceToContainer<std::vector<rdf::Statement>>(rStatements);
    std::sort(aStatements.begin(), aStatements.end(),
              [](const rdf::Statement& a, const rdf::Statement& b) {
                  return a.Predicate->getStringValue() < b.Predicate->getStringValue();
              });
    return aStatements;
}

CPPUNIT_TEST_FIXTURE(RDFStreamTest, testRDFa2)
{
    auto verify = [this](bool bIsExport) {
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

        uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(mxComponent,
                                                                             uno::UNO_QUERY);
        uno::Reference<rdf::XRepository> xRepo = xDocumentMetadataAccess->getRDFRepository();
        uno::Reference<rdf::XDocumentRepository> xDocRepo(xRepo, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDocRepo);

        const uno::Reference<uno::XComponentContext> xContext(
            comphelper::getProcessComponentContext(), css::uno::UNO_SET_THROW);
        uno::Reference<css::rdf::XURI> xFoo = rdf::URI::create(xContext, "uri:foo");
        uno::Reference<css::rdf::XURI> xBase = rdf::URI::create(xContext, "base-uri:");
        uno::Reference<css::rdf::XURI> xBar = rdf::URI::create(xContext, "uri:bar");
        uno::Reference<css::rdf::XURI> xBaz = rdf::URI::create(xContext, "uri:baz");
        uno::Reference<css::rdf::XBlankNode> xBlank = rdf::BlankNode::create(xContext, "_:uno");

        {
            // RDFa: 1
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            uno::Sequence<rdf::Statement> aStatements = xResult.First;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStatements.getLength());
            uno::Reference<css::rdf::XLiteral> xLit = rdf::Literal::create(xContext, "1");
            rdf::Statement aFooBarLit(xFoo, xBar, xLit, nullptr);

            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarLit, aStatements[0]);
            CPPUNIT_ASSERT(!xResult.Second);
        }
        {
            // RDFa: 2
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            uno::Sequence<rdf::Statement> aStatements = xResult.First;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStatements.getLength());
            uno::Reference<css::rdf::XLiteral> xLit = rdf::Literal::create(xContext, "2");
            rdf::Statement aFooBarLit(xFoo, xBar, xLit, nullptr);

            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarLit, aStatements[0]);
            CPPUNIT_ASSERT(!xResult.Second);
        }

        OUString aSubject3;
        OUString aSubject4;
        OUString aSubject5;
        {
            // RDFa: 3
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            uno::Sequence<rdf::Statement> aStatements = xResult.First;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStatements.getLength());

            aSubject3 = aStatements[0].Subject->getStringValue();
            CPPUNIT_ASSERT_EQUAL(OUString("uri:bar"), aStatements[0].Predicate->getStringValue());
            CPPUNIT_ASSERT_EQUAL(OUString("3"), aStatements[0].Object->getStringValue());
            CPPUNIT_ASSERT(!xResult.Second);
        }
        {
            // RDFa: 4
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            uno::Sequence<rdf::Statement> aStatements = xResult.First;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStatements.getLength());

            aSubject4 = aStatements[0].Subject->getStringValue();
            CPPUNIT_ASSERT_EQUAL(OUString("uri:bar"), aStatements[0].Predicate->getStringValue());
            CPPUNIT_ASSERT_EQUAL(OUString("4"), aStatements[0].Object->getStringValue());
            CPPUNIT_ASSERT(!xResult.Second);
        }
        {
            // RDFa: 5
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            uno::Sequence<rdf::Statement> aStatements = xResult.First;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStatements.getLength());

            aSubject5 = aStatements[0].Subject->getStringValue();
            CPPUNIT_ASSERT_EQUAL(OUString("uri:bar"), aStatements[0].Predicate->getStringValue());
            CPPUNIT_ASSERT_EQUAL(OUString("5"), aStatements[0].Object->getStringValue());
            CPPUNIT_ASSERT(!xResult.Second);
        }

        CPPUNIT_ASSERT(aSubject3 != aSubject4);
        CPPUNIT_ASSERT_EQUAL(aSubject3, aSubject5);

        {
            // RDFa: 6
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            std::vector<rdf::Statement> aStatements = sortStatementsByPredicate(xResult.First);
            CPPUNIT_ASSERT_EQUAL(size_t(2), aStatements.size());

            uno::Reference<css::rdf::XLiteral> xLit = rdf::Literal::create(xContext, "6");
            rdf::Statement aFooBarLit(xFoo, xBar, xLit, nullptr);
            rdf::Statement aFooBazLit(xFoo, xBaz, xLit, nullptr);

            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarLit, aStatements[0]);
            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBazLit, aStatements[1]);
            CPPUNIT_ASSERT(!xResult.Second);
        }
        {
            // RDFa: 7
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            std::vector<rdf::Statement> aStatements = sortStatementsByPredicate(xResult.First);
            CPPUNIT_ASSERT_EQUAL(size_t(3), aStatements.size());

            uno::Reference<css::rdf::XLiteral> xLit = rdf::Literal::create(xContext, "7");
            rdf::Statement aFooBarLit(xFoo, xBar, xLit, nullptr);
            rdf::Statement aFooBazLit(xFoo, xBaz, xLit, nullptr);
            rdf::Statement aFooFooLit(xFoo, xFoo, xLit, nullptr);

            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarLit, aStatements[0]);
            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBazLit, aStatements[1]);
            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooFooLit, aStatements[2]);
            CPPUNIT_ASSERT(!xResult.Second);
        }
        uno::Reference<css::rdf::XLiteral> xLit2 = rdf::Literal::create(xContext, "a fooish bar");
        uno::Reference<css::rdf::XLiteral> xLitType
            = rdf::Literal::createWithType(xContext, "a fooish bar", xBar);
        rdf::Statement aFooBarLit2(xFoo, xBar, xLit2, nullptr);
        rdf::Statement aFooBarLitType(xFoo, xBar, xLitType, nullptr);

        {
            // RDFa: 8
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            uno::Sequence<rdf::Statement> aStatements = xResult.First;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStatements.getLength());

            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarLit2, aStatements[0]);
            CPPUNIT_ASSERT(xResult.Second);
        }
        {
            // RDFa: 9
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            uno::Sequence<rdf::Statement> aStatements = xResult.First;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStatements.getLength());

            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarLit2, aStatements[0]);
            CPPUNIT_ASSERT(xResult.Second);
        }
        {
            // RDFa: 10
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            uno::Sequence<rdf::Statement> aStatements = xResult.First;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStatements.getLength());

            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarLitType, aStatements[0]);
            CPPUNIT_ASSERT(xResult.Second);
        }
        {
            // RDFa: 11
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            uno::Sequence<rdf::Statement> aStatements = xResult.First;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStatements.getLength());
            uno::Reference<css::rdf::XLiteral> xLitType11
                = rdf::Literal::createWithType(xContext, "11", xBar);
            rdf::Statement aFooBarLitType11(xFoo, xBar, xLitType11, nullptr);

            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarLitType11, aStatements[0]);
            CPPUNIT_ASSERT(!xResult.Second);
        }
        {
            // RDFa: 12
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            uno::Sequence<rdf::Statement> aStatements = xResult.First;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStatements.getLength());

            OUString aURL;
            if (bIsExport)
                aURL = maTempFile.GetURL();
            else
                aURL = createFileURL(u"TESTRDFA.odt");
            uno::Reference<css::rdf::XURI> xFile
                = rdf::URI::createNS(xContext, aURL, "/content.xml");
            uno::Reference<css::rdf::XLiteral> xLit = rdf::Literal::create(xContext, "12");
            rdf::Statement aFileBarLitType(xFile, xBar, xLit, nullptr);
            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFileBarLitType, aStatements[0]);
            CPPUNIT_ASSERT(!xResult.Second);
        }
        {
            // RDFa: 13
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            uno::Sequence<rdf::Statement> aStatements = xResult.First;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStatements.getLength());

            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarLit2, aStatements[0]);
            CPPUNIT_ASSERT(xResult.Second);
        }
        {
            // RDFa: 14
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            uno::Sequence<rdf::Statement> aStatements = xResult.First;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStatements.getLength());

            CPPUNIT_ASSERT_STATEMENT_EQUAL(aFooBarLit2, aStatements[0]);
            CPPUNIT_ASSERT(xResult.Second);
        }

        // Remaining rdfs should be empty
        do
        {
            uno::Reference<rdf::XMetadatable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
            ::beans::Pair<uno::Sequence<rdf::Statement>, sal_Bool> xResult
                = xDocRepo->getStatementRDFa(xPara);
            uno::Sequence<rdf::Statement> aStatements = xResult.First;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aStatements.getLength());
        } while (xParaEnum->hasMoreElements());
    };

    loadFromFile(u"TESTRDFA.odt");
    verify(/*bIsExport*/ false);
    saveAndReload(u"writer8"_ustr);
    verify(/*bIsExport*/ true);
}

CPPUNIT_TEST_FIXTURE(RDFStreamTest, testTdf123293)
{
    const uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext(),
                                                          css::uno::UNO_SET_THROW);
    const uno::Reference<com::sun::star::ucb::XSimpleFileAccess> xFileAccess(
        xContext->getServiceManager()->createInstanceWithContext(
            u"com.sun.star.ucb.SimpleFileAccess"_ustr, xContext),
        uno::UNO_QUERY_THROW);
    const uno::Reference<io::XInputStream> xInputStream(
        xFileAccess->openFileRead(createFileURL(u"TESTRDFA.odt")), uno::UNO_SET_THROW);
    uno::Sequence<beans::PropertyValue> aLoadArgs{ comphelper::makePropertyValue("InputStream",
                                                                                 xInputStream) };
    mxComponent
        = mxDesktop->loadComponentFromURL(u"private:stream"_ustr, u"_blank"_ustr, 0, aLoadArgs);
    CPPUNIT_ASSERT(mxComponent.is());
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<rdf::XRepository> xRepo = xDocumentMetadataAccess->getRDFRepository();
    uno::Reference<rdf::XDocumentRepository> xDocRepo(xRepo, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocRepo);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
