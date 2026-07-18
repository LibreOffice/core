/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2009-2010 Griffin Brown Digital Publishing Ltd.
 * Copyright (c) 2010-2011 Novell Inc.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>

#include <sal/types.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <mcefilter.hxx>
#include <ooxmlsession.hxx>
#include <opcpackage.hxx>
#include <report.hxx>
#include <schemacache.hxx>
#include <xmlutil.hxx>
#include <zipfile.hxx>

namespace
{
std::string dataPath(const char* relative) { return std::string(COFFICEOTRON_DATA_DIR) + relative; }

// Runs the MCE filter over a document given as a string and returns the
// result serialized without an XML declaration.
std::string filterMCE(const std::string& input, int& errorCount)
{
    std::string parseError;
    XmlDocPtr doc = parseXmlMemory(input, "test.xml", parseError);
    if (!doc)
    {
        errorCount = -1;
        return "PARSE FAILURE: " + parseError;
    }

    NullValidationReport report;
    ErrorCapper capper(report, "test");
    MCEFilter(doc.get(), false, capper);
    errorCount = capper.getInstanceErrCount();

    return serializeXml(doc.get());
}

const std::string MC = "http://schemas.openxmlformats.org/markup-compatibility/2006";

class CofficeotronTest : public CppUnit::TestFixture
{
public:
    void setUp() override { initValidation(); }

    void testIgnorables()
    {
        int errors = 0;
        std::string actual = filterMCE("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC
                                           + "\" "
                                             "mc:Ignorable=\"ext\">"
                                             "<bbb ext:prop=\"foo\"/>"
                                             "<ext:bar><ccc><ddd/></ccc></ext:bar>"
                                             "</aaa>",
                                       errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC + "\"><bbb/></aaa>",
                             actual);
        CPPUNIT_ASSERT_EQUAL(0, errors);
    }

    void testPreserveElements()
    {
        int errors = 0;
        std::string actual = filterMCE("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC
                                           + "\" "
                                             "mc:PreserveElements=\"ext:*\"/>",
                                       errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC + "\"/>", actual);
        CPPUNIT_ASSERT_EQUAL(0, errors);
    }

    void testPreserveAttributes()
    {
        int errors = 0;
        std::string actual = filterMCE("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC
                                           + "\" "
                                             "mc:PreserveAttributes=\"ext:*\"/>",
                                       errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC + "\"/>", actual);
        CPPUNIT_ASSERT_EQUAL(0, errors);
    }

    void testMustUnderstand()
    {
        int errors = 0;
        std::string actual = filterMCE("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC
                                           + "\" "
                                             "mc:MustUnderstand=\"\"/>",
                                       errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC + "\"/>", actual);
        CPPUNIT_ASSERT_EQUAL(0, errors);
    }

    void testChoiceNormal()
    {
        int errors = 0;
        std::string actual = filterMCE("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC
                                           + "\">"
                                             "<mc:AlternateContent>"
                                             "<mc:Choice Requires=\"ext\"><ext:foo/></mc:Choice>"
                                             "</mc:AlternateContent>"
                                             "</aaa>",
                                       errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC + "\"/>", actual);
        CPPUNIT_ASSERT_EQUAL(0, errors);
    }

    void testChoiceMissingRequires()
    {
        int errors = 0;
        std::string actual = filterMCE("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC
                                           + "\">"
                                             "<mc:AlternateContent><mc:Choice/>"
                                             "</mc:AlternateContent>"
                                             "</aaa>",
                                       errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC + "\"/>", actual);
        // A missing Requires attribute on mc:Choice is reported.
        CPPUNIT_ASSERT_EQUAL(1, errors);
    }

    void testChoicePrefixedRequires()
    {
        int errors = 0;
        std::string actual = filterMCE("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC
                                           + "\">"
                                             "<mc:AlternateContent>"
                                             "<mc:Choice mc:Requires=\"ext\"/>"
                                             "</mc:AlternateContent>"
                                             "</aaa>",
                                       errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC + "\"/>", actual);
        // A prefixed Requires is reported, and no unprefixed Requires
        // remains, which is reported as well.
        CPPUNIT_ASSERT_EQUAL(2, errors);
    }

    void testChoiceUnprefixedIgnorables()
    {
        int errors = 0;
        std::string actual = filterMCE("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC
                                           + "\">"
                                             "<mc:AlternateContent>"
                                             "<mc:Choice Requires=\"ext\" Ignorables=\"ext\"/>"
                                             "</mc:AlternateContent>"
                                             "</aaa>",
                                       errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC + "\"/>", actual);
        // An attribute with no namespace other than Requires is reported.
        CPPUNIT_ASSERT_EQUAL(1, errors);
    }

    void testChoiceXmlAttributes()
    {
        int errors = 0;
        // The xml namespace is implicit and its declaration is not
        // carried through parsing, so the expected output has no
        // xmlns:xml even though the input declares it.
        std::string actual = filterMCE("<aaa xmlns:ext=\"some-ext\" "
                                       "xmlns:xml=\"http://www.w3.org/XML/1998/namespace\" "
                                       "xmlns:mc=\""
                                           + MC
                                           + "\">"
                                             "<mc:AlternateContent>"
                                             "<mc:Choice Requires=\"ext\" xml:lang=\"fr\""
                                             " xml:space=\"preserve\"/>"
                                             "</mc:AlternateContent>"
                                             "</aaa>",
                                       errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC + "\"/>", actual);
        // xml:lang and xml:space on mc:Choice are each reported.
        CPPUNIT_ASSERT_EQUAL(2, errors);
    }

    void testChoiceParent()
    {
        int errors = 0;
        std::string actual = filterMCE("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC
                                           + "\">"
                                             "<mc:Choice Requires=\"ext\"/>"
                                             "</aaa>",
                                       errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC + "\"/>", actual);
        // mc:Choice outside mc:AlternateContent is reported.
        CPPUNIT_ASSERT_EQUAL(1, errors);
    }

    void testFallbackParent()
    {
        int errors = 0;
        std::string actual = filterMCE("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC
                                           + "\">"
                                             "<mc:Fallback/>"
                                             "</aaa>",
                                       errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC + "\"/>", actual);
        // mc:Fallback outside mc:AlternateContent is reported.
        CPPUNIT_ASSERT_EQUAL(1, errors);
    }

    void testFallbackNormal()
    {
        int errors = 0;
        std::string actual = filterMCE("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC
                                           + "\">"
                                             "<mc:AlternateContent>"
                                             "<mc:Fallback mc:Ignorable=\"ext\">"
                                             "<ext:foo/><bbb/>"
                                             "</mc:Fallback>"
                                             "</mc:AlternateContent>"
                                             "</aaa>",
                                       errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC + "\"><bbb/></aaa>",
                             actual);
        CPPUNIT_ASSERT_EQUAL(0, errors);
    }

    void testProcessContent()
    {
        int errors = 0;
        std::string actual
            = filterMCE("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC
                            + "\" "
                              "mc:Ignorable=\"ext\" mc:ProcessContent=\"ext:compat\">"
                              "<ext:bar>"
                              "<ccc><ddd/></ccc>"
                              "<ext:compat><eee/></ext:compat>"
                              "</ext:bar>"
                              "</aaa>",
                        errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:mc=\"" + MC + "\"><eee/></aaa>",
                             actual);
        CPPUNIT_ASSERT_EQUAL(0, errors);
    }

    void testProcessContentWildcard()
    {
        int errors = 0;
        std::string actual = filterMCE("<aaa xmlns:ext=\"some-ext\" xmlns:ext2=\"some-ext2\" "
                                       "xmlns:mc=\""
                                           + MC
                                           + "\" "
                                             "mc:Ignorable=\"ext\" mc:ProcessContent=\"ext2:*\">"
                                             "<ext:bar>"
                                             "<ccc><ddd/></ccc>"
                                             "<ext2:compat><eee/></ext2:compat>"
                                             "</ext:bar>"
                                             "</aaa>",
                                       errors);
        CPPUNIT_ASSERT_EQUAL("<aaa xmlns:ext=\"some-ext\" xmlns:ext2=\"some-ext2\" xmlns:mc=\"" + MC
                                 + "\"><eee/></aaa>",
                             actual);
        CPPUNIT_ASSERT_EQUAL(0, errors);
    }

    void testNormalizeSpace()
    {
        CPPUNIT_ASSERT_EQUAL(std::string("s1 s2 s3"), MCENormalizeWhitespaces(" s1 \ts2  \rs3\n"));
    }

    void testZipExtraction()
    {
        ZipArchive zip;
        std::string error;
        CPPUNIT_ASSERT_MESSAGE(error, zip.open(dataPath("/test-data/maria.xlsx"), error));
        std::string content;
        CPPUNIT_ASSERT(zip.extract("[Content_Types].xml", content) != nullptr);
        CPPUNIT_ASSERT(!content.empty());
        CPPUNIT_ASSERT_EQUAL(zip.centralRecordCount(), zip.localHeaderCount());
    }

    void testOPCPackage()
    {
        ZipArchive zip;
        std::string error;
        CPPUNIT_ASSERT_MESSAGE(error, zip.open(dataPath("/test-data/torture.pptx"), error));

        OPCPackage opc(zip);
        opc.process();

        CPPUNIT_ASSERT_EQUAL(size_t(21), opc.targets().size());

        const OOXMLTarget* slide = opc.targetByName("/ppt/slides/slide1.xml");
        CPPUNIT_ASSERT(slide != nullptr);
        CPPUNIT_ASSERT_EQUAL(
            std::string(
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide"),
            slide->type);
        CPPUNIT_ASSERT_EQUAL(
            std::string("application/vnd.openxmlformats-officedocument.presentationml.slide+xml"),
            slide->mimeType);

        CPPUNIT_ASSERT(opc.targetByName("/ppt/slides/slide999.xml") == nullptr);
    }

    void testOOXMLValidationSession()
    {
        ZipArchive zip;
        std::string error;
        CPPUNIT_ASSERT_MESSAGE(error, zip.open(dataPath("/test-data/torture.pptx"), error));

        SchemaCache schemas(COFFICEOTRON_SCHEMA_DIR);

        NullValidationReport report;
        // The torture document is OPC-consistent and every part is
        // schema-valid, so the whole session reports no errors.
        CPPUNIT_ASSERT_EQUAL(0, runOOXMLValidation(zip, report, schemas));
    }

    CPPUNIT_TEST_SUITE(CofficeotronTest);
    CPPUNIT_TEST(testIgnorables);
    CPPUNIT_TEST(testPreserveElements);
    CPPUNIT_TEST(testPreserveAttributes);
    CPPUNIT_TEST(testMustUnderstand);
    CPPUNIT_TEST(testChoiceNormal);
    CPPUNIT_TEST(testChoiceMissingRequires);
    CPPUNIT_TEST(testChoicePrefixedRequires);
    CPPUNIT_TEST(testChoiceUnprefixedIgnorables);
    CPPUNIT_TEST(testChoiceXmlAttributes);
    CPPUNIT_TEST(testChoiceParent);
    CPPUNIT_TEST(testFallbackParent);
    CPPUNIT_TEST(testFallbackNormal);
    CPPUNIT_TEST(testProcessContent);
    CPPUNIT_TEST(testProcessContentWildcard);
    CPPUNIT_TEST(testNormalizeSpace);
    CPPUNIT_TEST(testZipExtraction);
    CPPUNIT_TEST(testOPCPackage);
    CPPUNIT_TEST(testOOXMLValidationSession);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CofficeotronTest);

} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
