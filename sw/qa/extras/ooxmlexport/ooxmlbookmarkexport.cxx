/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <config_features.h>
#include <string>
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }

protected:
};

DECLARE_OOXMLEXPORT_TEST(tdf66398_permissions, "tdf66398_permissions.docx")
{
    // check document permission settings for the whole document
    if (xmlDocPtr pXmlSettings = parseExport("word/settings.xml"))
    {
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit", "readOnly");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement", "1");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptProviderType",
                    "rsaAES");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptAlgorithmClass",
                    "hash");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptAlgorithmType",
                    "typeAny");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptAlgorithmSid", "14");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptSpinCount", "100000");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "hash",
                    "A0/"
                    "Xy6KcXljJlZjP0TwJMPJuW2rc46UwXqn2ctxckc2nCECE5i89M85z2Noh3ZEA5NBQ9RJ5ycxiUH6nz"
                    "mJaKw==");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "salt",
                    "B8k6wb1pkjUs4Nv/8QBk/w==");
    }

    // get bookmark interface
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);

    // check: we have 2 bookmarks
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xBookmarksByIdx->getCount());
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("_GoBack"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("permission-for-group:267014232:everyone"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf103090, "tdf103090.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // Get bookmark name
    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:bookmarkStart", "name");

    // Ensure that name has no spaces
    CPPUNIT_ASSERT(bookmarkName.indexOf(" ") < 0);

    // Get PAGEREF field
    OUString fieldName = getXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:instrText");

    // Ensure that PAGEREF field refers exactly our bookmark
    OUString expectedFieldName(" PAGEREF ");
    expectedFieldName += bookmarkName;
    expectedFieldName += " \\h ";
    CPPUNIT_ASSERT_EQUAL(expectedFieldName, fieldName);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
