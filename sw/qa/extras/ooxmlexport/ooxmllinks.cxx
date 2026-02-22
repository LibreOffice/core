/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <unotools/tempfile.hxx>
#include <tools/urlobj.hxx>
#include <officecfg/Office/Common.hxx>
#include <o3tl/string_view.hxx>
#include <test/commontesttools.hxx>

// This file contains tests to check relative/absolute hyperlinks handling

// decide if output link should be converted to absolute
#define USE_ABSOLUTE false
#define USE_RELATIVE true

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr)
    {
    }

    using SaveFilesystemURLCfg = officecfg::Office::Common::Save::URL::FileSystem;

    // link should be modified to be placed in temp dir - for testing relative links
    void UseTempDir()
    {
        uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
        /* can be changed only after import */
        uno::Reference<text::XTextRange> xText = getRun(xParagraph, 1);

        /* Get original link */
        OUString sOriginalFileName = getProperty<OUString>(xText, u"HyperLinkURL"_ustr);
        INetURLObject aOriginalURL(sOriginalFileName);
        CPPUNIT_ASSERT(!aOriginalURL.HasError());
        OUString sFileName = aOriginalURL.GetLastName();
        CPPUNIT_ASSERT(!sFileName.isEmpty());

        /* Get temp path */
        OUString sTempDir = utl::GetTempNameBaseDirectory();

        /* Create & apply new URL */
        OUString sOriginalFileInTempDir = sTempDir + sFileName;
        uno::Reference<beans::XPropertySet> xPropertySet(xText, css::uno::UNO_QUERY);
        xPropertySet->setPropertyValue(u"HyperLinkURL"_ustr, css::uno::Any(sOriginalFileInTempDir));
    }
};

/* IMPORT */

CPPUNIT_TEST_FIXTURE(Test, testRelativeToRelativeImport)
{
    ScopedConfigValue<SaveFilesystemURLCfg> aCfg(USE_RELATIVE);
    createSwDoc("relative-link.docx");
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 1);
    OUString sTarget = getProperty<OUString>(xText, u"HyperLinkURL"_ustr);
    CPPUNIT_ASSERT(sTarget.startsWith("file:///"));
    CPPUNIT_ASSERT(sTarget.endsWith("relative.docx"));
}

CPPUNIT_TEST_FIXTURE(Test, testRelativeToAbsoluteImport)
{
    ScopedConfigValue<SaveFilesystemURLCfg> aCfg(USE_ABSOLUTE);
    createSwDoc("relative-link.docx");
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 1);
    OUString sTarget = getProperty<OUString>(xText, u"HyperLinkURL"_ustr);
    CPPUNIT_ASSERT(sTarget.startsWith("file:///"));
    CPPUNIT_ASSERT(sTarget.endsWith("relative.docx"));
}

CPPUNIT_TEST_FIXTURE(Test, testAbsoluteToAbsoluteImport)
{
    ScopedConfigValue<SaveFilesystemURLCfg> aCfg(USE_ABSOLUTE);
    createSwDoc("absolute-link.docx");
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 1);
    // # should be encoded
    CPPUNIT_ASSERT_EQUAL(u"file:///B:/Users/user/Desktop/a%23b/test.docx"_ustr,
                         getProperty<OUString>(xText, u"HyperLinkURL"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testAbsoluteToRelativeImport)
{
    ScopedConfigValue<SaveFilesystemURLCfg> aCfg(USE_RELATIVE);
    createSwDoc("absolute-link.docx");
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 1);
    // when target file (B:\\...) & document with link (temp dir) are placed on different partitions, absolute path will be loaded
    CPPUNIT_ASSERT_EQUAL(u"file:///B:/Users/user/Desktop/a%23b/test.docx"_ustr,
                         getProperty<OUString>(xText, u"HyperLinkURL"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123627_import)
{
    ScopedConfigValue<SaveFilesystemURLCfg> aCfg(USE_RELATIVE);
    createSwDoc("tdf123627.docx");
    uno::Reference<text::XTextRange> xText = getRun(getParagraph(1), 1);
    OUString sTarget = getProperty<OUString>(xText, u"HyperLinkURL"_ustr);
    CPPUNIT_ASSERT(sTarget.startsWith("file:///"));
    CPPUNIT_ASSERT(sTarget.endsWith("New/test.docx"));
}

/* EXPORT */

CPPUNIT_TEST_FIXTURE(Test, testRelativeToRelativeExport)
{
    ScopedConfigValue<SaveFilesystemURLCfg> aCfg(USE_RELATIVE);
    createSwDoc("relative-link.docx");
    UseTempDir();
    saveAndReload(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/_rels/document.xml.rels"_ustr);

    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@TargetMode='External']", "Target",
                u"relative.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testRelativeToAbsoluteExport)
{
    ScopedConfigValue<SaveFilesystemURLCfg> aCfg(USE_ABSOLUTE);
    createSwDoc("relative-link.docx");
    // Don't modify link.
    saveAndReload(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/_rels/document.xml.rels"_ustr);

    OUString sTarget = getXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[2]", "Target");
    CPPUNIT_ASSERT(sTarget.startsWith("file:///"));
    CPPUNIT_ASSERT(sTarget.endsWith("relative.docx"));
}

CPPUNIT_TEST_FIXTURE(Test, testAbsoluteToRelativeExport)
{
    ScopedConfigValue<SaveFilesystemURLCfg> aCfg(USE_RELATIVE);
    createSwDoc("absolute-link.docx");
    UseTempDir();
    saveAndReload(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/_rels/document.xml.rels"_ustr);

    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[2]", "Target", u"test.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testAbsoluteToAbsoluteExport)
{
    ScopedConfigValue<SaveFilesystemURLCfg> aCfg(USE_ABSOLUTE);
    createSwDoc("absolute-link.docx");
    // Don't modify link.
    saveAndReload(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/_rels/document.xml.rels"_ustr);

    OUString sTarget = getXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[2]", "Target");
    CPPUNIT_ASSERT(sTarget.startsWith("file:///"));
    CPPUNIT_ASSERT(sTarget.endsWith("test.docx"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123627_export)
{
    ScopedConfigValue<SaveFilesystemURLCfg> aCfg(USE_RELATIVE);
    createSwDoc("tdf123627.docx");
    UseTempDir();
    saveAndReload(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/_rels/document.xml.rels"_ustr);

    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@TargetMode='External']", "Target",
                u"test.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126590_export)
{
    ScopedConfigValue<SaveFilesystemURLCfg> aCfg(USE_ABSOLUTE);
    createSwDoc("tdf126590.docx");
    // Don't modify link.
    saveAndReload(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/_rels/document.xml.rels"_ustr);
    // in the original file: Target="file:///C:\TEMP\test.docx" => invalid file URI
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@TargetMode='External']", "Target",
                u"file:///C:/TEMP/test.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126768_export)
{
    ScopedConfigValue<SaveFilesystemURLCfg> aCfg(USE_ABSOLUTE);
    createSwDoc("tdf126768.docx");
    // Don't modify link.
    saveAndReload(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/_rels/document.xml.rels"_ustr);
    // in the original file: "file:///C:\\TEMP\\test.docx" => invalid file URI
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@TargetMode='External']", "Target",
                u"file:///C:/TEMP/test.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testNon_ascii_link_export)
{
    ScopedConfigValue<SaveFilesystemURLCfg> aCfg(USE_ABSOLUTE);
    createSwDoc("non_ascii_link.docx");
    // Don't modify link.
    saveAndReload(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/_rels/document.xml.rels"_ustr);

    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@TargetMode='External']", "Target",
                INetURLObject::decode(u"file:///C:/TEMP/%C3%A9kezet.docx",
                                      INetURLObject::DecodeMechanism::ToIUri,
                                      RTL_TEXTENCODING_UTF8));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
