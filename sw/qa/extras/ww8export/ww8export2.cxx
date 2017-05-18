/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <pagedesc.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97") {}

    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".doc");
    }

};
DECLARE_WW8EXPORT_TEST(testTdf99120, "tdf99120.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("Section 1, odd."),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Section 1, even."),  parseDump("/root/page[2]/header/txt/text()"));
    // This failed: the header was empty on the 3rd page, as the first page header was shown.
    CPPUNIT_ASSERT_EQUAL(OUString("Section 2, odd."),  parseDump("/root/page[3]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Section 2, even."),  parseDump("/root/page[4]/header/txt/text()"));
}

DECLARE_WW8EXPORT_TEST(testTdf41542_borderlessPadding, "tdf41542_borderlessPadding.odt")
{
    // the page style's borderless padding should force this to 3 pages, not 1
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );
}

DECLARE_WW8EXPORT_TEST(testTdf49102_mergedCellNumbering, "tdf49102_mergedCellNumbering.doc")
{
    CPPUNIT_ASSERT_EQUAL( OUString("2."), parseDump("/root/page/body/tab/row[4]/cell/txt/Special[@nType='POR_NUMBER']", "rText") );
}

DECLARE_WW8EXPORT_TEST(testTdf107931_KERN_DocEnabled_disabledDefStyle, "testTdf107931_KERN_DocEnabled_disabledDefStyle.doc")
{
    // Paragraph 3: the default style has kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(3), 1), "CharAutoKerning"));
    // Paragraph 4: style with kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(4), 1), "CharAutoKerning"));
    // Paragraph 5: style with kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(5), 1), "CharAutoKerning"));
    // Paragraph 6: directly applied character properties: kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(6), 1), "CharAutoKerning"));
    // Paragraph 7: directly applied character properties: kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(7), 1), "CharAutoKerning"));
}

DECLARE_WW8EXPORT_TEST(testTdf107931_KERN_enabledDefStyle, "testTdf107931_KERN_enabledDefStyle.doc")
{
    // Paragraph 3: the default style has kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(3), 1), "CharAutoKerning"));
    // Paragraph 4: style with kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(4), 1), "CharAutoKerning"));
    // Paragraph 5: style with kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(5), 1), "CharAutoKerning"));
    // Paragraph 6: directly applied character properties: kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(6), 1), "CharAutoKerning"));
    // Paragraph 7: directly applied character properties: kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(7), 1), "CharAutoKerning"));
}

DECLARE_WW8EXPORT_TEST(testTdf89377, "tdf89377_tableWithBreakBeforeParaStyle.doc")
{
    // the paragraph style should set table's text-flow break-before-page
    CPPUNIT_ASSERT_EQUAL( 2, getPages() );
}

DECLARE_WW8EXPORT_TEST(testTdf96277, "tdf96277.doc")
{
    // mismatch between anchoring at paragraph and ULSpacing to Page
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

DECLARE_WW8EXPORT_TEST(testBnc863018b, "bnc863018b.doc")
{
    // The whitespace above the table should allow text to flow between the table anchor and the table.
    // Since it doesn't, don't add the whitespace.
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

DECLARE_WW8EXPORT_TEST(testTdf104805, "tdf104805.doc")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WW8Num1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aNumberingRule;
    xLevels->getByIndex(1) >>= aNumberingRule; // 2nd level
    for (const auto& rPair : aNumberingRule)
    {
        if (rPair.Name == "Prefix")
            // This was "." instead of empty, so the second paragraph was
            // rendered as ".1" instead of "1.".
            CPPUNIT_ASSERT_EQUAL(OUString(), rPair.Value.get<OUString>());
        else if (rPair.Name == "Suffix")
            CPPUNIT_ASSERT_EQUAL(OUString("."), rPair.Value.get<OUString>());
    }
}

DECLARE_WW8EXPORT_TEST(testTdf104334, "tdf104334.doc")
{
    // This failed with a container::NoSuchElementException: STYLEREF was
    // mapped to SwChapterField, and the field result was "This is a Heading 1"
    // instead of just "1".
    CPPUNIT_ASSERT_EQUAL(OUString("1"), getRun(getParagraph(2), 3)->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
