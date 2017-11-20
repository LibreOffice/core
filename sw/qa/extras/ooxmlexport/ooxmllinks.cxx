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
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <unotools/saveopt.hxx>

 // This file contains tests to check relative/absolute hyperlinks handling

#define USE_TEMP_DIR true
#define DONT_MODIFY_LINK false

#define USE_ABSOLUTE true
#define USE_RELATIVE false

// bAbsolute - decide if output link should be converted to absolute
// bUseTempDir - decide if link should be modified to be placed in temp dir - for testing relative links
#define DECLARE_LINKS_EXPORT_TEST(TestName, FileName, bAbsolute, bUseTempDir) \
class TestName : public Test { \
protected: \
    virtual OUString getTestName() override { return OUString(#TestName); } \
    virtual void postLoad(const char*) override \
    { \
        if(!bUseTempDir) return; \
        \
        uno::Reference<text::XTextRange> xParagraph = getParagraph(1); \
        /* can be changed only after import */ \
        uno::Reference<text::XTextRange> xText = getRun(xParagraph, 1); \
        \
        /* Get original link */ \
        OUString sOriginalFileName = getProperty<OUString>(xText, "HyperLinkURL"); \
        INetURLObject aOriginalURL; \
        bool bOk = aOriginalURL.setFSysPath(sOriginalFileName, FSysStyle::Detect); \
        if(!bOk) aOriginalURL = INetURLObject(sOriginalFileName); \
        OUString sFileName = aOriginalURL.GetName().isEmpty() ? sOriginalFileName : aOriginalURL.GetName(); \
        \
        /* Get temp path */ \
        OUString sTempDir = utl::TempFile::CreateTempName(); \
        INetURLObject aTempURL; \
        aTempURL.setFSysPath(sTempDir, FSysStyle::Detect); \
        /* remove file name */ \
        aTempURL.removeSegment(); \
        sTempDir = INetURLObject::GetScheme(aTempURL.GetProtocol()) + aTempURL.GetURLPath(); \
        \
        /* Create & apply new URL */ \
        OUString sOriginalFileInTempDir = sTempDir + sFileName; \
        uno::Reference<beans::XPropertySet> xPropertySet(xText, css::uno::UNO_QUERY); \
        xPropertySet->setPropertyValue("HyperLinkURL", css::uno::makeAny(sOriginalFileInTempDir)); \
    } \
public: \
    CPPUNIT_TEST_SUITE(TestName); \
    CPPUNIT_TEST(Import_Export_Import); \
    CPPUNIT_TEST_SUITE_END(); \
    void Import_Export_Import() \
    { \
        SvtSaveOptions aOpt; \
        if (bAbsolute) { \
            aOpt.SetSaveRelFSys(false); \
            CPPUNIT_ASSERT(!aOpt.IsSaveRelFSys()); \
        } else { \
            aOpt.SetSaveRelFSys(true); \
            CPPUNIT_ASSERT(aOpt.IsSaveRelFSys()); \
        } \
        executeImportExportImportTest(FileName); \
    } \
    void verify() override; \
}; \
CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
void TestName::verify()

// bAbsolute - decide if relative link should be converted to absolute on import
#define DECLARE_LINKS_IMPORT_TEST(TestName, FileName, bAbsolute) \
class TestName : public Test { \
protected: \
    virtual OUString getTestName() override { return OUString(#TestName); } \
public: \
    CPPUNIT_TEST_SUITE(TestName); \
    CPPUNIT_TEST(Import); \
    CPPUNIT_TEST_SUITE_END(); \
    void Import() \
    { \
        SvtSaveOptions aOpt; \
        if (bAbsolute) { \
            aOpt.SetSaveRelFSys(false); \
            CPPUNIT_ASSERT(!aOpt.IsSaveRelFSys()); \
        } else { \
            aOpt.SetSaveRelFSys(true); \
            CPPUNIT_ASSERT(aOpt.IsSaveRelFSys()); \
        } \
        executeImportTest(FileName); \
    } \
    void verify() override; \
}; \
CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
void TestName::verify()

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

/* IMPORT */

DECLARE_LINKS_IMPORT_TEST(testRelativeToRelativeImport, "relative-link.docx", USE_RELATIVE)
{
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 1);
    CPPUNIT_ASSERT_EQUAL(OUString("relative.docx"), getProperty<OUString>(xText, "HyperLinkURL"));
}

DECLARE_LINKS_IMPORT_TEST(testRelativeToAbsoluteImport, "relative-link.docx", USE_ABSOLUTE)
{
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 1);
    OUString sTarget = getProperty<OUString>(xText, "HyperLinkURL");
    CPPUNIT_ASSERT(sTarget.startsWith("file:///"));
    CPPUNIT_ASSERT(sTarget.endsWith("relative.docx"));
}

DECLARE_LINKS_IMPORT_TEST(testAbsoluteToAbsoluteImport, "absolute-link.docx", USE_ABSOLUTE)
{
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 1);
    // # should be encoded
    CPPUNIT_ASSERT_EQUAL(OUString("file:///B:/Users/user/Desktop/a%23b/test.docx"), getProperty<OUString>(xText, "HyperLinkURL"));
}

DECLARE_LINKS_IMPORT_TEST(testAbsoluteToRelativeImport, "absolute-link.docx", USE_RELATIVE)
{
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 1);
    // when target file (B:\\...) & document with link (temp dir) are placed on different partitions, absolute path will be loaded
    CPPUNIT_ASSERT_EQUAL(OUString("file:///B:/Users/user/Desktop/a%23b/test.docx"), getProperty<OUString>(xText, "HyperLinkURL"));
}

/* EXPORT */

DECLARE_LINKS_EXPORT_TEST(testRelativeToRelativeExport, "relative-link.docx", USE_RELATIVE, DONT_MODIFY_LINK)
{
    xmlDocPtr pXmlDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[2]", "Target", "relative.docx");
}

DECLARE_LINKS_EXPORT_TEST(testRelativeToAbsoluteExport, "relative-link.docx", USE_ABSOLUTE, DONT_MODIFY_LINK)
{
    xmlDocPtr pXmlDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc)
        return;

    OUString sTarget = getXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[2]", "Target");
    CPPUNIT_ASSERT(sTarget.startsWith("file:///"));
    CPPUNIT_ASSERT(sTarget.endsWith("relative.docx"));
}

DECLARE_LINKS_EXPORT_TEST(testAbsoluteToRelativeExport, "absolute-link.docx", USE_RELATIVE, USE_TEMP_DIR)
{
    xmlDocPtr pXmlDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[2]", "Target", "test.docx");
}

DECLARE_LINKS_EXPORT_TEST(testAbsoluteToAbsoluteExport, "absolute-link.docx", USE_ABSOLUTE, DONT_MODIFY_LINK)
{
    xmlDocPtr pXmlDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc)
        return;

    OUString sTarget = getXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[2]", "Target");
    CPPUNIT_ASSERT(sTarget.startsWith("file:///"));
    CPPUNIT_ASSERT(sTarget.endsWith("test.docx"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
