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
ScAddress parseAddress(ScDocument& rDoc, OUString const& rAddressString)
{
    ScAddress aAddress;
    aAddress.Parse(rAddressString, rDoc);
    return aAddress;
}

Color getBackgroundColor(ScDocument& rDoc, OUString const& rAddressString)
{
    const ScPatternAttr* pPattern = rDoc.GetPattern(parseAddress(rDoc, rAddressString));
    const SvxBrushItem& rItem = pPattern->GetItem(ATTR_BACKGROUND);
    return rItem.GetColor();
}

Color getFontColor(ScDocument& rDoc, OUString const& rAddressString)
{
    const ScPatternAttr* pPattern = rDoc.GetPattern(parseAddress(rDoc, rAddressString));
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

static void assertDataFieldInRow_RowLabelColor(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"G6"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getFontColor(rDoc, u"G7"_ustr));

    // Make sure the other cells have the font color or background set to default
    std::vector<OUString> aEmptyAddresses{
        u"G5"_ustr, u"H5"_ustr, u"I5"_ustr, u"J5"_ustr, u"K5"_ustr, u"H6"_ustr, u"I6"_ustr,
        u"J6"_ustr, u"K6"_ustr, u"H7"_ustr, u"I7"_ustr, u"J7"_ustr, u"K7"_ustr,
    };
    CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormat_1_DataFieldInRow_RowLabelColor)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_1_DataFieldInRow_RowLabelColor.xlsx");
    assertDataFieldInRow_RowLabelColor(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDataFieldInRow_RowLabelColor(*getScDoc());
}

static void assertDataFieldInRow_ColumnLabelColor(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(Color(0x00B050), getBackgroundColor(rDoc, u"H5"_ustr));

    // Make sure the other cells have the font color or background set to default
    std::vector<OUString> aEmptyAddresses{
        u"G5"_ustr, u"I5"_ustr, u"J5"_ustr, u"K5"_ustr, u"G6"_ustr, u"H6"_ustr, u"I6"_ustr,
        u"J6"_ustr, u"K6"_ustr, u"G7"_ustr, u"H7"_ustr, u"I7"_ustr, u"J7"_ustr, u"K7"_ustr,
    };
    CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_2_DataFieldInRow_ColumnLabelColor)
{
    createScDoc(
        "xlsx/pivot-table/PivotTableCellFormatsTest_2_DataFieldInRow_ColumnLabelColor.xlsx");
    assertDataFieldInRow_ColumnLabelColor(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDataFieldInRow_ColumnLabelColor(*getScDoc());
}

static void assertDataFieldInColumn_ColumnLabelColor(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getFontColor(rDoc, u"H5"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0x92D050), getBackgroundColor(rDoc, u"I5"_ustr));

    // Make sure the other cells have the font color or background set to default
    std::vector<OUString> aEmptyAddresses{
        u"G5"_ustr, u"G6"_ustr, u"H6"_ustr, u"I6"_ustr, u"G7"_ustr, u"H7"_ustr, u"I7"_ustr,
        u"G8"_ustr, u"H8"_ustr, u"I8"_ustr, u"G9"_ustr, u"H9"_ustr, u"I9"_ustr,
    };
    CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_3_DataFieldInColumn_ColumnLabelColor)
{
    createScDoc(
        "xlsx/pivot-table/PivotTableCellFormatsTest_3_DataFieldInColumn_ColumnLabelColor.xlsx");
    assertDataFieldInColumn_ColumnLabelColor(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDataFieldInColumn_ColumnLabelColor(*getScDoc());
}

static void assertDataFieldInColumn_DataColor(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getFontColor(rDoc, u"H7"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0x92D050), getBackgroundColor(rDoc, u"I9"_ustr));

    std::vector<OUString> aEmptyAddresses{
        u"G5"_ustr, u"H5"_ustr, u"I5"_ustr, u"G6"_ustr, u"H6"_ustr, u"I6"_ustr, u"G7"_ustr,
        u"I7"_ustr, u"G8"_ustr, u"H8"_ustr, u"I8"_ustr, u"G9"_ustr, u"H9"_ustr,
    };
    CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_4_DataFieldInColumn_DataColor)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_4_DataFieldInColumn_DataColor.xlsx");
    assertDataFieldInColumn_DataColor(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDataFieldInColumn_DataColor(*getScDoc());
}

static void assertDataFieldInColumnAndTwoRowFields_DataColor(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"I8"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"I11"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0x0070C0), getBackgroundColor(rDoc, u"J13"_ustr));

    std::vector<OUString> aEmptyAddresses{
        u"G5"_ustr,  u"H5"_ustr,  u"I5"_ustr,  u"J5"_ustr,  u"G6"_ustr,  u"H6"_ustr,  u"I6"_ustr,
        u"J6"_ustr,  u"G7"_ustr,  u"H7"_ustr,  u"I7"_ustr,  u"J7"_ustr,  u"G8"_ustr,  u"H8"_ustr,
        u"J8"_ustr,  u"G9"_ustr,  u"H9"_ustr,  u"I9"_ustr,  u"J9"_ustr,  u"G10"_ustr, u"H10"_ustr,
        u"I10"_ustr, u"J10"_ustr, u"G11"_ustr, u"H11"_ustr, u"J11"_ustr, u"G12"_ustr, u"H12"_ustr,
        u"I12"_ustr, u"J12"_ustr, u"G13"_ustr, u"H13"_ustr, u"I13"_ustr, u"G14"_ustr, u"H14"_ustr,
        u"I14"_ustr, u"J14"_ustr,
    };
    CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_5_DataFieldInColumnAndTwoRowFields_DataColor)
{
    createScDoc("xlsx/pivot-table//"
                "PivotTableCellFormatsTest_5_DataFieldInColumnAndTwoRowFields_DataColor.xlsx");
    assertDataFieldInColumnAndTwoRowFields_DataColor(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDataFieldInColumnAndTwoRowFields_DataColor(*getScDoc());
}

static void assertSingleDataFieldInColumn_DataColor(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"J8"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"J12"_ustr));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_6_SingleDataFieldInColumn_DataColor)
{
    createScDoc(
        "xlsx/pivot-table//PivotTableCellFormatsTest_6_SingleDataFieldInColumn_DataColor.xlsx");
    assertSingleDataFieldInColumn_DataColor(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertSingleDataFieldInColumn_DataColor(*getScDoc());
}

static void assertTwoRowTwoColumnFields_DataColor(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"I7"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xFFC000), getBackgroundColor(rDoc, u"J8"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0x0070C0), getBackgroundColor(rDoc, u"J9"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0x00B0F0), getBackgroundColor(rDoc, u"J13"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0x92D050), getBackgroundColor(rDoc, u"K12"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"L14"_ustr));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_7_TwoRowTwoColumnFields_DataColor)
{
    createScDoc(
        "xlsx/pivot-table//PivotTableCellFormatsTest_7_TwoRowTwoColumnFields_DataColor.xlsx");
    assertTwoRowTwoColumnFields_DataColor(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertTwoRowTwoColumnFields_DataColor(*getScDoc());
}

static void assertDataFieldInRow_DataColor(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(Color(0x00B0F0), getBackgroundColor(rDoc, u"I6"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"K7"_ustr));

    // Make sure the other cells have the font color or background set to default
    std::vector<OUString> aEmptyAddresses{
        u"G5"_ustr, u"H5"_ustr, u"I5"_ustr, u"J5"_ustr, u"K5"_ustr, u"G6"_ustr, u"H6"_ustr,
        u"J6"_ustr, u"K6"_ustr, u"G7"_ustr, u"H7"_ustr, u"I7"_ustr, u"J7"_ustr,
    };
    CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
};

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_8_DataFieldInRow_DataColor)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_8_DataFieldInRow_DataColor.xlsx");
    assertDataFieldInRow_DataColor(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertDataFieldInRow_DataColor(*getScDoc());
}

static void assertMultipleSelections(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"I6"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"I7"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"I8"_ustr));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_9_MultipleSelections)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_9_MultipleSelections.xlsx");
    assertMultipleSelections(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertMultipleSelections(*getScDoc());
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_10_FormatDefinitionNotMatchingPivotTable)
{
    // The pivot table format data in this document doesn't match the pivot table data, which can produce
    // a crash during loading and resolving of formats. Specifically

    // Load the document, which shouldn't result in a crash
    createScDoc("xlsx/pivot-table/"
                "PivotTableCellFormatsTest_10_FormatDefinitionNotMatchingPivotTable.xlsx");
    ScDocument& rDoc = *getScDoc();

    // Let's check the pivot table exists
    ScDPCollection* pCollection = rDoc.GetDPCollection();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pCollection->GetCount());

    auto aAddress = parseAddress(rDoc, u"G2"_ustr);
    const ScDPObject* pDPObject = rDoc.GetDPAtCursor(aAddress);
    CPPUNIT_ASSERT(pDPObject);

    CPPUNIT_ASSERT_EQUAL(u"60"_ustr, rDoc.GetString(aAddress));
}

static void assertWholeDataColumnSelected(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"G2"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"G3"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"G4"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"G5"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"G6"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"G7"_ustr));

    // Make sure the other cells have the font color or background set to default
    std::vector<OUString> aEmptyAddresses{
        u"F1"_ustr, u"G1"_ustr, u"F2"_ustr, u"F3"_ustr, u"F4"_ustr,
        u"F5"_ustr, u"F6"_ustr, u"F7"_ustr, u"F8"_ustr, u"G8"_ustr,
    };
    CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_11_WholeDataColumnSelected)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_11_WholeDataColumnSelected.xlsx");
    assertWholeDataColumnSelected(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertWholeDataColumnSelected(*getScDoc());
}

static void assertWholeLabelColumnSelected(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"F2"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"F3"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"F4"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"F5"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"F6"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"F7"_ustr));

    // Make sure the other cells have the font color or background set to default
    std::vector<OUString> aEmptyAddresses{
        u"F1"_ustr, u"G1"_ustr, u"G2"_ustr, u"G3"_ustr, u"G4"_ustr,
        u"G5"_ustr, u"G6"_ustr, u"G7"_ustr, u"F8"_ustr, u"G8"_ustr,
    };
    CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_12_WholeLabelColumnSelected)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_12_WholeLabelColumnSelected.xlsx");
    assertWholeLabelColumnSelected(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertWholeLabelColumnSelected(*getScDoc());
}

static void assertSelectionInLabelAndData(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"F5"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"G5"_ustr));

    // Make sure the other cells have the font color or background set to default
    std::vector<OUString> aEmptyAddresses{
        u"F1"_ustr, u"G1"_ustr, u"F2"_ustr, u"G2"_ustr, u"F3"_ustr, u"G3"_ustr, u"F4"_ustr,
        u"G4"_ustr, u"F6"_ustr, u"G6"_ustr, u"F7"_ustr, u"G7"_ustr, u"F8"_ustr, u"G8"_ustr,
    };
    CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_13_SelectionInLabelAndData)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_13_SelectionInLabelAndData.xlsx");
    assertSelectionInLabelAndData(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertSelectionInLabelAndData(*getScDoc());
}

static void assertTwoRowsDataFieldInColumn_LabelColor(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"I5"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getBackgroundColor(rDoc, u"J5"_ustr));

    // Make sure the other cells have the font color or background set to default
    std::vector<OUString> aEmptyAddresses{
        u"G5"_ustr,  u"H5"_ustr,  u"G6"_ustr,  u"H6"_ustr,  u"i6"_ustr,  u"j6"_ustr,  u"G7"_ustr,
        u"H7"_ustr,  u"i7"_ustr,  u"j7"_ustr,  u"G8"_ustr,  u"H8"_ustr,  u"i8"_ustr,  u"j8"_ustr,
        u"G9"_ustr,  u"H9"_ustr,  u"i9"_ustr,  u"j9"_ustr,  u"G10"_ustr, u"H10"_ustr, u"i10"_ustr,
        u"j10"_ustr, u"G11"_ustr, u"H11"_ustr, u"i11"_ustr, u"j11"_ustr, u"G12"_ustr, u"H12"_ustr,
        u"i12"_ustr, u"j12"_ustr, u"G13"_ustr, u"H13"_ustr, u"i13"_ustr, u"j13"_ustr, u"G14"_ustr,
        u"H14"_ustr, u"i14"_ustr, u"j14"_ustr,
    };
    CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_14_TwoRowsDataFieldInColumn_LabelColor)
{
    createScDoc("xlsx/pivot-table/"
                "PivotTableCellFormatsTest_14_TwoRowsDataFieldInColumn_LabelColor.xlsx");
    assertTwoRowsDataFieldInColumn_LabelColor(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertTwoRowsDataFieldInColumn_LabelColor(*getScDoc());
}

static void assertTwoDataFieldColumns_WholeDataColumnSelected(ScDocument& rDoc)
{
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"H3"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"H4"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"H5"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"H6"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"H7"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getBackgroundColor(rDoc, u"H8"_ustr));

    // Make sure the other cells have the font color or background set to default
    std::vector<OUString> aEmptyAddresses{
        u"F1"_ustr, u"G1"_ustr, u"H1"_ustr, u"F2"_ustr, u"G2"_ustr, u"H2"_ustr,
        u"F3"_ustr, u"G3"_ustr, u"F4"_ustr, u"G4"_ustr, u"F5"_ustr, u"G5"_ustr,
        u"F6"_ustr, u"G6"_ustr, u"F7"_ustr, u"G7"_ustr, u"F8"_ustr, u"G8"_ustr,
    };
    CPPUNIT_ASSERT_EQUAL(OUString(), checkNonEmptyAddresses(rDoc, aEmptyAddresses));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     PivotTableCellFormatsTest_15_TwoDataFieldColumns_WholeDataColumnSelected)
{
    createScDoc("xlsx/pivot-table/"
                "PivotTableCellFormatsTest_15_TwoDataFieldColumns_WholeDataColumnSelected.xlsx");
    assertTwoDataFieldColumns_WholeDataColumnSelected(*getScDoc());
    saveAndReload("Calc Office Open XML");
    assertTwoDataFieldColumns_WholeDataColumnSelected(*getScDoc());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
