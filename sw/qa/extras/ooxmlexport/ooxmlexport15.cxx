/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

char const DATA_DIRECTORY[] = "/sw/qa/extras/ooxmlexport/data/";

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

DECLARE_OOXMLEXPORT_TEST(testTdf133334_followPgStyle, "tdf133334_followPgStyle.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf133370_columnBreak, "tdf133370_columnBreak.odt")
{
    // Since non-DOCX formats ignores column breaks in non-column situations, don't export to docx.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}


DECLARE_OOXMLEXPORT_TEST(testTdf134063, "tdf134063.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    xmlDocUniquePtr pDump = parseLayoutDump();

    // There are three tabs with default width
    CPPUNIT_ASSERT_EQUAL(sal_Int32(720), getXPath(pDump, "//page[1]/body/txt[1]/Text[1]", "nWidth").toInt32());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(720), getXPath(pDump, "//page[1]/body/txt[1]/Text[2]", "nWidth").toInt32());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(720), getXPath(pDump, "//page[1]/body/txt[1]/Text[3]", "nWidth").toInt32());
}

DECLARE_OOXMLEXPORT_TEST(testRelativeAnchorHeightFromBottomMarginHasFooter,
                         "tdf133070_testRelativeAnchorHeightFromBottomMarginHasFooter.docx")
{
    // TODO: fix export too
    if (mbExported)
        return;
    // tdf#133070 The height was set relative to page print area bottom,
    // but this was handled relative to page height.
    // Note: page print area bottom = margin + footer height.
    // In this case the footer exists.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    const sal_Int32 nAnchoredHeight
        = getXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1147), nAnchoredHeight);
}

DECLARE_OOXMLEXPORT_TEST(testRelativeAnchorHeightFromBottomMarginNoFooter,
                         "tdf133070_testRelativeAnchorHeightFromBottomMarginNoFooter.docx")
{
    // TODO: fix export too
    if (mbExported)
        return;
    // tdf#133070 The height was set relative to page print area bottom,
    // but this was handled relative to page height.
    // Note: page print area bottom = margin + footer height.
    // In this case the footer does not exist, so OpenDocument and OOXML margins are the same.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    const sal_Int32 nAnchoredHeight
        = getXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1147), nAnchoredHeight);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
