/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/text/XBookmarksSupplier.hpp>

#include <comphelper/configuration.hxx>
#include <comphelper/scopeguard.hxx>
#include <officecfg/Office/Common.hxx>

#include <swmodeltestbase.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}

protected:
    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf135164_cancelledNumbering, "tdf135164_cancelledNumbering.docx")
{
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1, u"TBMM DÖNEMİ"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));

    xPara.set(getParagraph(2, "Numbering explicitly cancelled"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));

    xPara.set(getParagraph(6, "Default style has roman numbering"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("i"), getProperty<OUString>(xPara, "ListLabelString"));
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf135906, "tdf135906.docx")
{
    // just test round-tripping. The document was exported as corrupt and didn't re-load.
}

CPPUNIT_TEST_FIXTURE(Test, testParaStyleNumLevel)
{
    loadAndSave("para-style-num-level.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/styles.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. a custom list level in a para style was lost on import+export.
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='Mystyle']/w:pPr/w:numPr/w:ilvl", "val", "1");
}

DECLARE_OOXMLEXPORT_TEST(testTdf137466, "tdf137466.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return; // initial import, no futher checks

    // Ensure that we have <w:placeholder><w:docPart v:val="xxxx"/></w:placeholder>
    OUString sDocPart = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:placeholder/w:docPart", "val");
    CPPUNIT_ASSERT_EQUAL(OUString("DefaultPlaceholder_-1854013440"), sDocPart);

    // Ensure that we have <w15:color v:val="xxxx"/>
    OUString sColor = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w15:color", "val");
    CPPUNIT_ASSERT_EQUAL(OUString("FF0000"), sColor);
}

CPPUNIT_TEST_FIXTURE(Test, testDontAddNewStyles)
{
    // Given a document that lacks builtin styles, and addition of them is disabled:
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Load::DisableBuiltinStyles::set(true, pBatch);
        pBatch->commit();
    }
    comphelper::ScopeGuard g([] {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Load::DisableBuiltinStyles::set(false, pBatch);
        pBatch->commit();
    });

    // When saving that document:
    loadAndSave("dont-add-new-styles.docx");

    // Then make sure that export doesn't have additional styles, Caption was one of them:
    xmlDocUniquePtr pXmlDoc = parseExport("word/styles.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. builtin styles were added to the export result, even if we opted out.
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='Caption']", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf126287, "tdf126287.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf123642_BookmarkAtDocEnd, "tdf123642.docx")
{
    // get bookmark interface
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();

    // check: we have 1 bookmark (previously there were 0)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xBookmarksByIdx->getCount());
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Bookmark1"));

    // and it is really in exported DOCX (let's ensure)
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return; // initial import, no further checks

    CPPUNIT_ASSERT_EQUAL(OUString("Bookmark1"), getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:bookmarkStart[1]", "name"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf142407, "tdf142407.docx")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<beans::XPropertySet> xPageStyle(xPageStyles->getByName("Standard"), uno::UNO_QUERY);
    sal_Int16 nGridLines;
    xPageStyle->getPropertyValue("GridLines") >>= nGridLines;
    CPPUNIT_ASSERT_EQUAL( sal_Int16(36), nGridLines);   // was 23, left large space before text.
}

DECLARE_OOXMLEXPORT_TEST(testTdf81507, "tdf81507.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return; // initial import, no further checks

    // Ensure that we have <w:text w:multiLine="1"/>
    CPPUNIT_ASSERT_EQUAL(OUString("1"), getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:sdt/w:sdtPr/w:text", "multiLine"));

    // Ensure that we have <w:text w:multiLine="0"/>
    CPPUNIT_ASSERT_EQUAL(OUString("0"), getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:sdt/w:sdtPr/w:text", "multiLine"));

    // Ensure that we have <w:text/>
    getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:sdt/w:sdtPr/w:text", "");

    // Ensure that we have no <w:text/> (not quite correct case, but to ensure import/export are okay)
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "/w:document/w:body/w:p[4]/w:sdt/w:sdtPr/w:text");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                           static_cast<sal_Int32>(xmlXPathNodeSetGetLength(pXmlObj->nodesetval)));
    xmlXPathFreeObject(pXmlObj);
}

DECLARE_OOXMLEXPORT_TEST(testTdf144668, "tdf144668.odt")
{
    uno::Reference<beans::XPropertySet> xPara1(getParagraph(1, u"level1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("[0001]"), getProperty<OUString>(xPara1, "ListLabelString"));

    uno::Reference<beans::XPropertySet> xPara2(getParagraph(2, u"level2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("[001]"), getProperty<OUString>(xPara2, "ListLabelString"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
