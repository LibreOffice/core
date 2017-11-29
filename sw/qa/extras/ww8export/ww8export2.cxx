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
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/text/VertOrientation.hpp>

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

DECLARE_WW8EXPORT_TEST(testTdf108072, "tdf108072.doc")
{
    // The property IsSplitAllowed was imported from an obsolete property, sprmTFCantSplit90
    // instead of sprmTFCantSplit. sprmTFCantSplit90 is set to true for merged rows, so
    // for merged rows incorrect settings were imported, which prevented them from breaking over pages.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTableRows->getByIndex(0), "IsSplitAllowed"));
}

DECLARE_WW8EXPORT_TEST(testTdf91687, "tdf91687.doc")
{
    // Exported Watermarks were resized
    uno::Reference<drawing::XShape> xWatermark = getShape(1);
    uno::Reference<beans::XPropertySet> xWatermarkProperties(xWatermark, uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL((sal_Int32)5172, xWatermark->getSize().Height);
    CPPUNIT_ASSERT_EQUAL((sal_Int32)18105, xWatermark->getSize().Width);
}

DECLARE_WW8EXPORT_TEST(testTdf111480, "tdf111480.doc")
{
    // Circular text was imported horizontally
    uno::Reference<drawing::XShape> xText = getShape(1);

    CPPUNIT_ASSERT(xText->getSize().Height > 11000);
    CPPUNIT_ASSERT(xText->getSize().Width  > 11000);
}

DECLARE_WW8EXPORT_TEST( testActiveXCheckbox, "checkbox_control.odt" )
{
    // First check box anchored as a floating object
    uno::Reference<drawing::XControlShape> xControlShape;
    if(!mbExported)
        xControlShape.set(getShape(1), uno::UNO_QUERY);
    else
        xControlShape.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());

    // Check whether we have the right control
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService( "com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("Floating Checkbox"), getProperty<OUString>(xPropertySet, "Label"));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,"AnchorType"));

    // Second check box anchored inline / as character
    if(!mbExported)
        xControlShape.set(getShape(2), uno::UNO_QUERY);
    else
        xControlShape.set(getShape(1), uno::UNO_QUERY);

    // Check whether we have the right control
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService("com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("Inline Checkbox"), getProperty<OUString>(xPropertySet, "Label"));

    // Check anchor type
    xPropertySet2.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,"AnchorType"));
}

DECLARE_WW8EXPORT_TEST(testTdf67207_MERGEFIELD, "mailmerge.doc")
{
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), "TextField");
    CPPUNIT_ASSERT(xTextField.is());
    uno::Reference<lang::XServiceInfo> xServiceInfo(xTextField, uno::UNO_QUERY_THROW);
    uno::Reference<text::XDependentTextField> xDependent(xTextField, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.Database"));
    OUString sValue;
    xTextField->getPropertyValue("Content") >>= sValue;
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("\u00ABName\u00BB"), sValue);

    uno::Reference<beans::XPropertySet> xFiledMaster = xDependent->getTextFieldMaster();
    uno::Reference<lang::XServiceInfo> xFiledMasterServiceInfo(xFiledMaster, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xFiledMasterServiceInfo->supportsService("com.sun.star.text.fieldmaster.Database"));

    // Defined properties: DataBaseName, Name, DataTableName, DataColumnName, DependentTextFields, DataCommandType, InstanceName, DataBaseURL
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("Name") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("DataColumnName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("InstanceName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.text.fieldmaster.DataBase.Name"), sValue);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
