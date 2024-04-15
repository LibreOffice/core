/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "helper/qahelper.hxx"

#include <patattr.hxx>
#include <document.hxx>
#include <dpobject.hxx>
#include <attrib.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <rtl/string.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>

using namespace css;

class ScPivotTableFormatsImportExport : public ScModelTestBase
{
public:
    ScPivotTableFormatsImportExport();
};

ScPivotTableFormatsImportExport::ScPivotTableFormatsImportExport()
    : ScModelTestBase("sc/qa/unit/data")
{
}

namespace
{
Color getBackgroundColor(ScDocument& rDoc, OUString const& rAddressString)
{
    ScAddress aAddress;
    aAddress.Parse(rAddressString, rDoc);
    const ScPatternAttr* pPattern = rDoc.GetPattern(aAddress);
    const SvxBrushItem& rItem = pPattern->GetItem(ATTR_BACKGROUND);
    return rItem.GetColor();
}

Color getFontColor(ScDocument& rDoc, OUString const& rAddressString)
{
    ScAddress aAddress;
    aAddress.Parse(rAddressString, rDoc);
    const ScPatternAttr* pPattern = rDoc.GetPattern(aAddress);
    const SvxColorItem& rItem = pPattern->GetItem(ATTR_FONT_COLOR);
    return rItem.getColor();
}

template <typename T> OUString checkNonEmptyAddresses(ScDocument& rDoc, T const& rArrayOfAddresses)
{
    OUString aString;
    for (auto const& rAddressString : rArrayOfAddresses)
    {
        ScAddress aAddress;
        aAddress.Parse(rAddressString, rDoc);
        const ScPatternAttr* pPattern = rDoc.GetPattern(aAddress);
        if (pPattern->GetItem(ATTR_FONT_COLOR).getColor() != COL_BLACK
            || pPattern->GetItem(ATTR_BACKGROUND).GetColor() != COL_TRANSPARENT)
        {
            aString += rAddressString + " ";
        }
    }
    return aString;
}

} // end anonymous namespace

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormat_1_DataFieldInRow_RowLabelColor)
{
    auto assertDocument = [](ScDocument& rDoc) {
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"G6"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getFontColor(rDoc, u"G7"_ustr));

        // Make sure the other cells have the font color or background set to default
        auto aEmptyAddresses = std::to_array<OUString>({
            u"G5"_ustr,
            u"H5"_ustr,
            u"I5"_ustr,
            u"J5"_ustr,
            u"K5"_ustr,
            u"H6"_ustr,
            u"I6"_ustr,
            u"J6"_ustr,
            u"K6"_ustr,
            u"H7"_ustr,
            u"I7"_ustr,
            u"J7"_ustr,
            u"K7"_ustr,
        });
        CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
    };

    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_1_DataFieldInRow_RowLabelColor.xlsx");
    assertDocument(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDocument(*getScDoc());
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_2_DataFieldInRow_ColumnLabelColor)
{
    auto assertDocument = [](ScDocument& rDoc) {
        CPPUNIT_ASSERT_EQUAL(Color(0x00B050), getBackgroundColor(rDoc, u"H5"_ustr));

        // Make sure the other cells have the font color or background set to default
        auto aEmptyAddresses = std::to_array<OUString>({
            u"G5"_ustr,
            u"I5"_ustr,
            u"J5"_ustr,
            u"K5"_ustr,
            u"G6"_ustr,
            u"H6"_ustr,
            u"I6"_ustr,
            u"J6"_ustr,
            u"K6"_ustr,
            u"G7"_ustr,
            u"H7"_ustr,
            u"I7"_ustr,
            u"J7"_ustr,
            u"K7"_ustr,
        });
        CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
    };

    createScDoc(
        "xlsx/pivot-table/PivotTableCellFormatsTest_2_DataFieldInRow_ColumnLabelColor.xlsx");
    assertDocument(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDocument(*getScDoc());
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_3_DataFieldInColumn_ColumnLabelColor)
{
    auto assertDocument = [](ScDocument& rDoc) {
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getFontColor(rDoc, u"H5"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x92D050), getBackgroundColor(rDoc, u"I5"_ustr));

        // Make sure the other cells have the font color or background set to default
        auto aEmptyAddresses = std::to_array<OUString>({
            u"G5"_ustr,
            u"G6"_ustr,
            u"H6"_ustr,
            u"I6"_ustr,
            u"G7"_ustr,
            u"H7"_ustr,
            u"I7"_ustr,
            u"G8"_ustr,
            u"H8"_ustr,
            u"I8"_ustr,
            u"G9"_ustr,
            u"H9"_ustr,
            u"I9"_ustr,
        });
        CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
    };

    createScDoc(
        "xlsx/pivot-table/PivotTableCellFormatsTest_3_DataFieldInColumn_ColumnLabelColor.xlsx");
    assertDocument(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDocument(*getScDoc());
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_4_DataFieldInColumn_DataColor)
{
    auto assertDocument = [](ScDocument& rDoc) {
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getFontColor(rDoc, u"H7"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x92D050), getBackgroundColor(rDoc, u"I9"_ustr));

        auto aEmptyAddresses = std::to_array<OUString>({
            u"G5"_ustr,
            u"H5"_ustr,
            u"I5"_ustr,
            u"G6"_ustr,
            u"H6"_ustr,
            u"I6"_ustr,
            u"G7"_ustr,
            u"I7"_ustr,
            u"G8"_ustr,
            u"H8"_ustr,
            u"I8"_ustr,
            u"G9"_ustr,
            u"H9"_ustr,
        });
        CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
    };

    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_4_DataFieldInColumn_DataColor.xlsx");
    assertDocument(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDocument(*getScDoc());
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_5_DataFieldInColumnAndTwoRowFields_DataColor)
{
    auto assertDocument = [](ScDocument& rDoc) {
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"I8"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"I11"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x0070C0), getBackgroundColor(rDoc, u"J13"_ustr));

        auto aEmptyAddresses = std::to_array<OUString>({
            u"G5"_ustr,  u"H5"_ustr,  u"I5"_ustr,  u"J5"_ustr,  u"G6"_ustr,  u"H6"_ustr,
            u"I6"_ustr,  u"J6"_ustr,  u"G7"_ustr,  u"H7"_ustr,  u"I7"_ustr,  u"J7"_ustr,
            u"G8"_ustr,  u"H8"_ustr,  u"J8"_ustr,  u"G9"_ustr,  u"H9"_ustr,  u"I9"_ustr,
            u"J9"_ustr,  u"G10"_ustr, u"H10"_ustr, u"I10"_ustr, u"J10"_ustr, u"G11"_ustr,
            u"H11"_ustr, u"J11"_ustr, u"G12"_ustr, u"H12"_ustr, u"I12"_ustr, u"J12"_ustr,
            u"G13"_ustr, u"H13"_ustr, u"I13"_ustr, u"G14"_ustr, u"H14"_ustr, u"I14"_ustr,
            u"J14"_ustr,
        });
        CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
    };

    createScDoc("xlsx/pivot-table//"
                "PivotTableCellFormatsTest_5_DataFieldInColumnAndTwoRowFields_DataColor.xlsx");
    assertDocument(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDocument(*getScDoc());
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_6_SingleDataFieldInColumn_DataColor)
{
    auto assertDocument = [](ScDocument& rDoc) {
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"J8"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"J12"_ustr));
    };

    createScDoc(
        "xlsx/pivot-table//PivotTableCellFormatsTest_6_SingleDataFieldInColumn_DataColor.xlsx");
    assertDocument(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDocument(*getScDoc());
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_7_TwoRowTwoColumnFields_DataColor)
{
    auto assertDocument = [](ScDocument& rDoc) {
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"I7"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0xFFC000), getBackgroundColor(rDoc, u"J8"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x0070C0), getBackgroundColor(rDoc, u"J9"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x00B0F0), getBackgroundColor(rDoc, u"J13"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x92D050), getBackgroundColor(rDoc, u"K12"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"L14"_ustr));
    };

    createScDoc(
        "xlsx/pivot-table//PivotTableCellFormatsTest_7_TwoRowTwoColumnFields_DataColor.xlsx");
    assertDocument(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDocument(*getScDoc());
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_8_DataFieldInRow_DataColor)
{
    auto assertDocument = [](ScDocument& rDoc) {
        CPPUNIT_ASSERT_EQUAL(Color(0x00B0F0), getBackgroundColor(rDoc, u"I6"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"K7"_ustr));

        // Make sure the other cells have the font color or background set to default
        auto aEmptyAddresses = std::to_array<OUString>({
            u"G5"_ustr,
            u"H5"_ustr,
            u"I5"_ustr,
            u"J5"_ustr,
            u"K5"_ustr,
            u"G6"_ustr,
            u"H6"_ustr,
            u"J6"_ustr,
            u"K6"_ustr,
            u"G7"_ustr,
            u"H7"_ustr,
            u"I7"_ustr,
            u"J7"_ustr,
        });
        CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
    };

    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_8_DataFieldInRow_DataColor.xlsx");
    assertDocument(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDocument(*getScDoc());
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_9_MultipleSelections)
{
    auto assertDocument = [](ScDocument& rDoc) {
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"I6"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"I7"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"I8"_ustr));
    };

    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_9_MultipleSelections.xlsx");
    assertDocument(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDocument(*getScDoc());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
