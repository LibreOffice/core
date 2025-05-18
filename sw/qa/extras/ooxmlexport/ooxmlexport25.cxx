/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertyState.hpp>

#include <comphelper/configuration.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/propertyvalue.hxx>
#include <officecfg/Office/Common.hxx>

#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <wrtsh.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr)
    {
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf166544_noTopMargin_fields, "tdf166544_noTopMargin_fields.docx")
{
    // given a document with a hyperlink field containing a page break
    auto pXmlDoc = parseLayoutDump();

    // The top margin is applied before the page break - since the page break follows the field end
    sal_Int32 nHeight = getXPath(pXmlDoc, "//page[2]//txt/infos/bounds", "height").toInt32();
    // Without the fix, the text height (showing a large top margin) was 569
    CPPUNIT_ASSERT_EQUAL(sal_Int32(269), nHeight);
}

DECLARE_OOXMLEXPORT_TEST(testTdf166510_sectPr_bottomSpacing, "tdf166510_sectPr_bottomSpacing.docx")
{
    // given with a sectPr with different bottom spacing (undefined in this case - i.e. zero)
    auto pXmlDoc = parseLayoutDump();

    // The last paragraph (sectPr) has 0 below spacing, so no reduction of page 2's 200pt top margin
    sal_Int32 nHeight = getXPath(pXmlDoc, "//page[2]//body/txt/infos/bounds", "height").toInt32();
    // Without the fix, the text height (showing no top margin at all) was 253
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4253), nHeight);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf166620)
{
    createSwDoc();
    {
        SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
        pWrtShell->Insert(u"Body text"_ustr);
        pWrtShell->InsertFootnote({}, /*bEndNote=*/true, /*bEdit=*/true);
        pWrtShell->Insert(u"Endnote text"_ustr);
    }

    // Exporting to a Word format, a tab is prepended to the endnote text. When imported, the
    // NoGapAfterNoteNumber compatibility flag is enabled; and the exported tab is the only thing
    // that separates the number and the text. The tab must not be stripped away on import.
    saveAndReload(mpFilter);
    {
        auto xFactory = mxComponent.queryThrow<lang::XMultiServiceFactory>();
        auto xSettings = xFactory->createInstance(u"com.sun.star.document.Settings"_ustr);
        CPPUNIT_ASSERT(getProperty<bool>(xSettings, u"NoGapAfterNoteNumber"_ustr));

        auto xSupplier = mxComponent.queryThrow<text::XEndnotesSupplier>();
        auto xEndnotes = xSupplier->getEndnotes();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xEndnotes->getCount());
        auto xEndnoteText = xEndnotes->getByIndex(0).queryThrow<text::XText>();
        CPPUNIT_ASSERT_EQUAL(u"\tEndnote text"_ustr, xEndnoteText->getString());
    }
    // Do a second round-trip. It must not duplicate the tab.
    saveAndReload(mpFilter);
    {
        auto xFactory = mxComponent.queryThrow<lang::XMultiServiceFactory>();
        auto xSettings = xFactory->createInstance(u"com.sun.star.document.Settings"_ustr);
        CPPUNIT_ASSERT(getProperty<bool>(xSettings, u"NoGapAfterNoteNumber"_ustr));

        auto xSupplier = mxComponent.queryThrow<text::XEndnotesSupplier>();
        auto xEndnotes = xSupplier->getEndnotes();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xEndnotes->getCount());
        auto xEndnoteText = xEndnotes->getByIndex(0).queryThrow<text::XText>();
        CPPUNIT_ASSERT_EQUAL(u"\tEndnote text"_ustr, xEndnoteText->getString());

        // Remove the tab
        xEndnoteText->setString(u"Endnote text"_ustr);
    }
    // Do a third round-trip. It must not introduce the tab, because of the compatibility flag.
    saveAndReload(mpFilter);
    {
        auto xFactory = mxComponent.queryThrow<lang::XMultiServiceFactory>();
        auto xSettings = xFactory->createInstance(u"com.sun.star.document.Settings"_ustr);
        CPPUNIT_ASSERT(getProperty<bool>(xSettings, u"NoGapAfterNoteNumber"_ustr));

        auto xSupplier = mxComponent.queryThrow<text::XEndnotesSupplier>();
        auto xEndnotes = xSupplier->getEndnotes();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xEndnotes->getCount());
        auto xEndnoteText = xEndnotes->getByIndex(0).queryThrow<text::XText>();
        CPPUNIT_ASSERT_EQUAL(u"Endnote text"_ustr, xEndnoteText->getString());
    }
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
