/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <comphelper/configuration.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/xfillit0.hxx>

// This file contains tests to check relative/absolute hyperlinks handling

// bRelative - decide if output link should be relative
#define DECLARE_LINKS_EXPORT_TEST(TestName, FileName, bRelative) \
class TestName : public Test { \
protected: \
    std::shared_ptr<comphelper::ConfigurationChanges> m_pBatch; \
    virtual OUString getTestName() override { return OUString(#TestName); } \
    struct UrlType : public comphelper::ConfigurationProperty<UrlType, bool> \
    { \
        static OUString path() \
        { \
            return OUString("/org.openoffice.Office.Common/Save/URL/FileSystem"); \
        } \
        ~UrlType() = delete; \
    }; \
    void useRelativeLinks() \
    { \
        if(!m_pBatch) m_pBatch = comphelper::ConfigurationChanges::create(); \
        UrlType::set(true, m_pBatch); \
        m_pBatch->commit(); \
    } \
    void useAbsoluteLinks() \
    { \
        if(!m_pBatch) m_pBatch = comphelper::ConfigurationChanges::create(); \
        UrlType::set(false, m_pBatch); \
        m_pBatch->commit(); \
    } \
public: \
    CPPUNIT_TEST_SUITE(TestName); \
    CPPUNIT_TEST(Import_Export_Import); \
    CPPUNIT_TEST_SUITE_END(); \
    void Import_Export_Import() \
    { \
        if (bRelative) \
            useRelativeLinks(); \
        else \
            useAbsoluteLinks(); \
        executeImportExportImportTest(FileName); \
    } \
    void verify() override; \
}; \
CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
void TestName::verify()

// bRelative - decide if link should be converted to relative on import
#define DECLARE_LINKS_IMPORT_TEST(TestName, FileName, bRelative) \
class TestName : public Test { \
protected: \
    std::shared_ptr<comphelper::ConfigurationChanges> m_pBatch; \
    virtual OUString getTestName() override { return OUString(#TestName); } \
    struct UrlType : public comphelper::ConfigurationProperty<UrlType, bool> \
    { \
        static OUString path() \
        { \
            return OUString("/org.openoffice.Office.Common/Save/URL/FileSystem"); \
        } \
        ~UrlType() = delete; \
    }; \
    void useRelativeLinks() \
    { \
        if(!m_pBatch) m_pBatch = comphelper::ConfigurationChanges::create(); \
        UrlType::set(true, m_pBatch); \
        m_pBatch->commit(); \
    } \
    void useAbsoluteLinks() \
    { \
        if(!m_pBatch) m_pBatch = comphelper::ConfigurationChanges::create(); \
        UrlType::set(false, m_pBatch); \
        m_pBatch->commit(); \
    } \
public: \
    CPPUNIT_TEST_SUITE(TestName); \
    CPPUNIT_TEST(Import); \
    CPPUNIT_TEST_SUITE_END(); \
    void Import() \
    { \
        if (bRelative) \
            useRelativeLinks(); \
        else \
            useAbsoluteLinks(); \
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

DECLARE_LINKS_IMPORT_TEST(testRelativeToRelativeImport, "relative-link.docx", true)
{
    xmlDocPtr pXmlDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@Target='relative.docx']", 1);
}

DECLARE_LINKS_IMPORT_TEST(testRelativeToAbsoluteImport, "relative-link.docx", false)
{
    xmlDocPtr pXmlDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@Target='relative.docx']", 0);
}

DECLARE_LINKS_IMPORT_TEST(testAbsoluteToRelativeImport, "absolute-link.docx", true)
{
    xmlDocPtr pXmlDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc)
        return;

    // If saved file is placed on different disk than linked file, path will be absolute
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@Target='file:///C:\\Users\\eszka\\Desktop\\test.docx']", 1);
}

DECLARE_LINKS_IMPORT_TEST(testAbsoluteToAbsoluteImport, "absolute-link.docx", false)
{
    xmlDocPtr pXmlDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@Target='file:///C:\\Users\\eszka\\Desktop\\test.docx']", 1);
}

// EXPORT ////////////////////////////////////////////////////////////////////////////

DECLARE_LINKS_EXPORT_TEST(testRelativeToRelativeExport, "relative-link.docx", true)
{
    xmlDocPtr pXmlDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc)
        return;

    // If saved file is placed on different disk than linked file, path will be absolute
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@Target='relative.docx']", 0);
}

DECLARE_LINKS_EXPORT_TEST(testRelativeToAbsoluteExport, "relative-link.docx", false)
{
    xmlDocPtr pXmlDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@Target='relative.docx']", 0);
}

DECLARE_LINKS_EXPORT_TEST(testAbsoluteToRelativeExport, "absolute-link.docx", true)
{
    xmlDocPtr pXmlDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc)
        return;

    // If saved file is placed on different disk than linked file, path will be absolute
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@Target='file:///C:\\Users\\eszka\\Desktop\\test.docx']", 1);
}

DECLARE_LINKS_EXPORT_TEST(testAbsoluteToAbsoluteExport, "absolute-link.docx", false)
{
    xmlDocPtr pXmlDoc = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@Target='file:///C:\\Users\\eszka\\Desktop\\test.docx']", 1);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
