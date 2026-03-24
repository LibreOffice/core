/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

namespace
{
class ScPivotTableFormatsImportExport : public ScModelTestBase
{
public:
    ScPivotTableFormatsImportExport();
};

ScPivotTableFormatsImportExport::ScPivotTableFormatsImportExport()
    : ScModelTestBase(u"sc/qa/unit/data"_ustr)
{
}

ScAddress parseAddress(const ScDocument& rDoc, OUString const& rAddressString)
{
    ScAddress aAddress;
    aAddress.Parse(rAddressString, rDoc);
    return aAddress;
}

Color getBackgroundColor(const ScDocument& rDoc, OUString const& rAddressString)
{
    const ScPatternAttr* pPattern = rDoc.GetPattern(parseAddress(rDoc, rAddressString));
    const SvxBrushItem& rItem = pPattern->GetItem(ATTR_BACKGROUND);
    return rItem.GetColor();
}

/** Compare pivot table output against Excel reference output below.
 *  The reference area starts 1 row after the pivot table ends.
 *  Returns empty string if all cells match, otherwise lists mismatches. */
OUString comparePivotWithReference(ScDocument& rDocument, ScRange const& rPivotRange,
                                   ScRange const& rReferenceRange)
{
    OUString aMismatches;
    SCROW nRows = rPivotRange.aEnd.Row() - rPivotRange.aStart.Row() + 1;
    SCCOL nColumns = rPivotRange.aEnd.Col() - rPivotRange.aStart.Col() + 1;

    for (SCROW nRow = 0; nRow < nRows; ++nRow)
    {
        for (SCCOL nColumn = 0; nColumn < nColumns; ++nColumn)
        {
            ScAddress aPivotAddress(rPivotRange.aStart.Col() + nColumn,
                                    rPivotRange.aStart.Row() + nRow, rPivotRange.aStart.Tab());
            ScAddress aReferenceAddress(rReferenceRange.aStart.Col() + nColumn,
                                        rReferenceRange.aStart.Row() + nRow,
                                        rReferenceRange.aStart.Tab());

            const ScPatternAttr* pPivotPattern = rDocument.GetPattern(aPivotAddress);
            const ScPatternAttr* pReferencePattern = rDocument.GetPattern(aReferenceAddress);

            Color aPivotFontColor = pPivotPattern->GetItem(ATTR_FONT_COLOR).getColor();
            Color aReferenceFontColor = pReferencePattern->GetItem(ATTR_FONT_COLOR).getColor();

            Color aPivotBackgroundColor = pPivotPattern->GetItem(ATTR_BACKGROUND).GetColor();
            Color aReferenceBackgroundColor
                = pReferencePattern->GetItem(ATTR_BACKGROUND).GetColor();

            OUString aCellName = aPivotAddress.Format(ScRefFlags::VALID);

            if (aPivotFontColor != aReferenceFontColor)
            {
                aMismatches += "Font Color: " + aCellName
                               + " (expected = " + aReferenceFontColor.AsRGBHexString()
                               + " got = " + aPivotFontColor.AsRGBHexString() + ") ";
            }
            if (aPivotBackgroundColor != aReferenceBackgroundColor)
            {
                aMismatches += "Background Color: " + aCellName
                               + " (expected = " + aReferenceBackgroundColor.AsRGBHexString()
                               + " got = " + aPivotBackgroundColor.AsRGBHexString() + ") ";
            }
        }
    }
    return aMismatches;
}

/** Get the pivot table output range and the reference range from DPObject */
std::pair<ScRange, ScRange> getPivotAndReferenceRanges(ScDocument& rDocument)
{
    ScDPCollection* pDPs = rDocument.GetDPCollection();
    CPPUNIT_ASSERT(pDPs);
    CPPUNIT_ASSERT(pDPs->GetCount() > 0);
    ScDPObject& rDPObject = (*pDPs)[0];
    ScRange aPivotRange = rDPObject.GetOutRange();
    // Reference is the same size, starting 2 rows below the pivot table
    SCROW nGap = 2;
    SCROW nHeight = aPivotRange.aEnd.Row() - aPivotRange.aStart.Row() + 1;
    ScRange aReferenceRange(aPivotRange.aStart.Col(), aPivotRange.aEnd.Row() + nGap,
                            aPivotRange.aStart.Tab(), aPivotRange.aEnd.Col(),
                            aPivotRange.aEnd.Row() + nGap + nHeight - 1, aPivotRange.aStart.Tab());
    return { aPivotRange, aReferenceRange };
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormat_1_DataFieldInRow_RowLabelColor)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_1_DataFieldInRow_RowLabelColor.xlsx");

    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_2_DataFieldInRow_ColumnLabelColor)
{
    createScDoc(
        "xlsx/pivot-table/PivotTableCellFormatsTest_2_DataFieldInRow_ColumnLabelColor.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_3_DataFieldInColumn_ColumnLabelColor)
{
    createScDoc(
        "xlsx/pivot-table/PivotTableCellFormatsTest_3_DataFieldInColumn_ColumnLabelColor.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_4_DataFieldInColumn_DataColor)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_4_DataFieldInColumn_DataColor.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_5_DataFieldInColumnAndTwoRowFields_DataColor)
{
    createScDoc("xlsx/pivot-table/"
                "PivotTableCellFormatsTest_5_DataFieldInColumnAndTwoRowFields_DataColor.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_6_SingleDataFieldInColumn_DataColor)
{
    createScDoc(
        "xlsx/pivot-table/PivotTableCellFormatsTest_6_SingleDataFieldInColumn_DataColor.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    // TODO
    //checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_7_TwoRowTwoColumnFields_DataColor)
{
    createScDoc(
        "xlsx/pivot-table/PivotTableCellFormatsTest_7_TwoRowTwoColumnFields_DataColor.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    // TODO
    //checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_8_DataFieldInRow_DataColor)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_8_DataFieldInRow_DataColor.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_9_MultipleSelections)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_9_MultipleSelections.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_10_FormatDefinitionNotMatchingPivotTable)
{
    // Load the document, which shouldn't result in a crash
    createScDoc("xlsx/pivot-table/"
                "PivotTableCellFormatsTest_10_FormatDefinitionNotMatchingPivotTable.xlsx");
    ScDocument& rDocument = *getScDoc();
    ScDPCollection* pCollection = rDocument.GetDPCollection();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pCollection->GetCount());
    auto aAddress = parseAddress(rDocument, u"G2"_ustr);
    const ScDPObject* pDPObject = rDocument.GetDPAtCursor(aAddress);
    CPPUNIT_ASSERT(pDPObject);
    CPPUNIT_ASSERT_EQUAL(u"60"_ustr, rDocument.GetString(aAddress));
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_11_WholeDataColumnSelected)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_11_WholeDataColumnSelected.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_12_WholeLabelColumnSelected)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_12_WholeLabelColumnSelected.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_13_SelectionInLabelAndData)
{
    createScDoc("xlsx/pivot-table/PivotTableCellFormatsTest_13_SelectionInLabelAndData.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    // TODO
    //checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_14_TwoRowsDataFieldInColumn_LabelColor)
{
    createScDoc("xlsx/pivot-table/"
                "PivotTableCellFormatsTest_14_TwoRowsDataFieldInColumn_LabelColor.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport,
                     testPivotTableCellFormatsTest_15_TwoDataFieldColumns_WholeDataColumnSelected)
{
    createScDoc("xlsx/pivot-table/"
                "PivotTableCellFormatsTest_15_TwoDataFieldColumns_WholeDataColumnSelected.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    // TODO
    // checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport, testPivotTableWithCellProtection)
{
    createScDoc("xlsx/pivot-table/Pivot_Table_with_Cell_Protection.xlsx");
    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();
        auto[aPivotRange, aReferenceRange] = getPivotAndReferenceRanges(rDocument);
        OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
    };

    checkFormats();
    saveAndReload(TestFilter::XLSX);
    checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport, testPivotTableWithNoSourceData)
{
    // We need to round-trip this document without crashing
    createScDoc("xlsx/pivot-table/PivotTableWithNoSourceData.xlsx");
    saveAndReload(TestFilter::XLSX);
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport, testPivotTableFormatsGrandTotal)
{
    // Check that we can support pivot table cell formats on grand total output.

    // In the file: pivot table is at B3:G14, reference output is at B17:G28.
    createScDoc("xlsx/pivot-table/PivotTableFormatsGrandTotal.xlsx");

    auto checkFormats = [this]() {
        ScDocument& rDocument = *getScDoc();

        CPPUNIT_ASSERT_EQUAL(Color(0x00B050), getBackgroundColor(rDocument, u"G4"_ustr));
        CPPUNIT_ASSERT(Color(0x00B050) != getBackgroundColor(rDocument, u"G3"_ustr));
        CPPUNIT_ASSERT(Color(0x00B050) != getBackgroundColor(rDocument, u"G5"_ustr));
        CPPUNIT_ASSERT(Color(0x00B050) != getBackgroundColor(rDocument, u"F4"_ustr));

        // grandRow="1" labelOnly="1" offset="A256"
        CPPUNIT_ASSERT_EQUAL(Color(0xC00000), getBackgroundColor(rDocument, u"B14"_ustr));

        // grandRow="1" labelOnly="1" offset="IV256"
        CPPUNIT_ASSERT_EQUAL(Color(0xFFFF00), getBackgroundColor(rDocument, u"C14"_ustr));

        // grandRow="1" grandCol="1" - intersection
        CPPUNIT_ASSERT_EQUAL(Color(0x00B0F0), getBackgroundColor(rDocument, u"G14"_ustr));

        // grandRow="1" data with reference field="2" selected="0"
        CPPUNIT_ASSERT_EQUAL(Color(0x92D050), getBackgroundColor(rDocument, u"E14"_ustr));
        CPPUNIT_ASSERT(Color(0x92D050) != getBackgroundColor(rDocument, u"D14"_ustr));
        CPPUNIT_ASSERT(Color(0x92D050) != getBackgroundColor(rDocument, u"F14"_ustr));

        // grandCol="1" data with subtotal reference
        CPPUNIT_ASSERT_EQUAL(Color(0x0070C0), getBackgroundColor(rDocument, u"G5"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x0070C0), getBackgroundColor(rDocument, u"G6"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x0070C0), getBackgroundColor(rDocument, u"G7"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x0070C0), getBackgroundColor(rDocument, u"G8"_ustr));
        CPPUNIT_ASSERT(Color(0x0070C0) != getBackgroundColor(rDocument, u"G9"_ustr));
    };

    checkFormats();
    // TODO: Round-trip
    //saveAndReload(TestFilter::XLSX);
    //checkFormats();
}

CPPUNIT_TEST_FIXTURE(ScPivotTableFormatsImportExport, testPivotTableFormatsSubTotals)
{
    // TODO: enable once subtotal label formatting is fully implemented
    createScDoc("xlsx/pivot-table/PivotTableFormatsSubTotals.xlsx");
    // ScDocument& rDocument = *getScDoc();
    // ScRange aPivotRange(1, 2, 0, 11, 13, 0); // B3:L14
    // ScRange aReferenceRange(1, 15, 0, 11, 26, 0); // B16:L27
    // OUString aMismatches = comparePivotWithReference(rDocument, aPivotRange, aReferenceRange);
    // CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell format mismatches found", OUString(), aMismatches);
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
