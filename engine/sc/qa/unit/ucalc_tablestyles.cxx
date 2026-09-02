/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"
#include <memory>

#include <TableStyleGenerator.hxx>
#include <tablestyle.hxx>
#include <attrib.hxx>
#include <dbdata.hxx>
#include <dbdocfun.hxx>
#include <scopetools.hxx>
#include <subtotalparam.hxx>
#include <docfunc.hxx>
#include <editutil.hxx>
#include <formulacell.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <rangenam.hxx>
#include <scitems.hxx>
#include <patattr.hxx>
#include <undomanager.hxx>

#include <docmodel/theme/Theme.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/wghtitem.hxx>
#include <fillinfo.hxx>
#include <svx/svdpage.hxx>

#include <ThemeColorChanger.hxx>

using namespace css;

class TableStylesTest : public ScUcalcTestBase
{
};

namespace
{
// Theme A: distinct primary accent colors for easy verification
std::shared_ptr<model::ColorSet> createTestThemeA()
{
    auto pColorSet = std::make_shared<model::ColorSet>(u"ThemeA"_ustr);
    pColorSet->add(model::ThemeColorType::Dark1, 0x000000);
    pColorSet->add(model::ThemeColorType::Light1, 0xFFFFFF);
    pColorSet->add(model::ThemeColorType::Dark2, 0x444444);
    pColorSet->add(model::ThemeColorType::Light2, 0xEEEEEE);
    pColorSet->add(model::ThemeColorType::Accent1, 0xFF0000); // red
    pColorSet->add(model::ThemeColorType::Accent2, 0x00FF00); // green
    pColorSet->add(model::ThemeColorType::Accent3, 0x0000FF); // blue
    pColorSet->add(model::ThemeColorType::Accent4, 0xFFFF00); // yellow
    pColorSet->add(model::ThemeColorType::Accent5, 0xFF00FF); // magenta
    pColorSet->add(model::ThemeColorType::Accent6, 0x00FFFF); // cyan
    pColorSet->add(model::ThemeColorType::Hyperlink, 0x0563C1);
    pColorSet->add(model::ThemeColorType::FollowedHyperlink, 0x954F72);
    return pColorSet;
}

// Theme B: swapped accents for theme change verification
std::shared_ptr<model::ColorSet> createTestThemeB()
{
    auto pColorSet = std::make_shared<model::ColorSet>(u"ThemeB"_ustr);
    pColorSet->add(model::ThemeColorType::Dark1, 0x000000);
    pColorSet->add(model::ThemeColorType::Light1, 0xFFFFFF);
    pColorSet->add(model::ThemeColorType::Dark2, 0x333333);
    pColorSet->add(model::ThemeColorType::Light2, 0xDDDDDD);
    pColorSet->add(model::ThemeColorType::Accent1, 0x0000FF); // blue (was red)
    pColorSet->add(model::ThemeColorType::Accent2, 0xFF0000); // red (was green)
    pColorSet->add(model::ThemeColorType::Accent3, 0x00FF00); // green (was blue)
    pColorSet->add(model::ThemeColorType::Accent4, 0x00FFFF); // cyan (was yellow)
    pColorSet->add(model::ThemeColorType::Accent5, 0xFFFF00); // yellow (was magenta)
    pColorSet->add(model::ThemeColorType::Accent6, 0xFF00FF); // magenta (was cyan)
    pColorSet->add(model::ThemeColorType::Hyperlink, 0x0563C1);
    pColorSet->add(model::ThemeColorType::FollowedHyperlink, 0x954F72);
    return pColorSet;
}

// Set up the document theme with the given ColorSet on the draw layer
void applyThemeToDocument(ScDocument* pDoc, std::shared_ptr<model::ColorSet> pColorSet)
{
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);
    auto const& pTheme = pDrawLayer->getTheme();
    CPPUNIT_ASSERT(pTheme);
    pTheme->setColorSet(pColorSet);
}

// Create a ScDBData with table style assigned. Defaults give A1:D10 with
// header and total rows under name "TestTable" — the geometry used by the
// pattern-resolution tests below. Pass non-default arguments for tests
// that need varied geometry / header / totals.
ScDBData* createTestDBData(ScDocument* pDoc, const OUString& rStyleName, SCCOL nCol1 = 0,
                           SCROW nRow1 = 0, SCCOL nCol2 = 3, SCROW nRow2 = 10,
                           bool bHasHeader = true, bool bHasTotals = true,
                           const OUString& rName = u"TestTable"_ustr)
{
    ScDBData* pDBData = new ScDBData(rName, /*nTab*/ 0, nCol1, nRow1, nCol2, nRow2,
                                     /*bByRow*/ true, bHasHeader, bHasTotals);

    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = rStyleName;
    aStyleParam.mbRowStripes = true;
    aStyleParam.mbColumnStripes = false;
    aStyleParam.mbFirstColumn = false;
    aStyleParam.mbLastColumn = false;
    pDBData->SetTableStyleInfo(aStyleParam);

    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);
    return pDBData;
}

ScRange getArea(const ScDBData& rData)
{
    ScRange aRange;
    rData.GetArea(aRange);
    return aRange;
}

ScDBData* findTestTable(ScDocument* pDoc)
{
    return pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TESTTABLE"_ustr);
}

} // anonymous namespace

// Test 1: Verify 60 default styles are generated with correct names
CPPUNIT_TEST_FIXTURE(TableStylesTest, testGenerateDefaultStyles)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    ScTableStyles* pStyles = m_pDoc->GetTableStyles();
    CPPUNIT_ASSERT(pStyles);

    // Verify some key style names from each category
    CPPUNIT_ASSERT(pStyles->GetTableStyle(u"TableStyleLight1"_ustr));
    CPPUNIT_ASSERT(pStyles->GetTableStyle(u"TableStyleLight21"_ustr));
    CPPUNIT_ASSERT(pStyles->GetTableStyle(u"TableStyleMedium1"_ustr));
    CPPUNIT_ASSERT(pStyles->GetTableStyle(u"TableStyleMedium2"_ustr));
    CPPUNIT_ASSERT(pStyles->GetTableStyle(u"TableStyleMedium28"_ustr));
    CPPUNIT_ASSERT(pStyles->GetTableStyle(u"TableStyleDark1"_ustr));
    CPPUNIT_ASSERT(pStyles->GetTableStyle(u"TableStyleDark11"_ustr));

    // Verify non-existent style returns null
    CPPUNIT_ASSERT(!pStyles->GetTableStyle(u"NonExistentStyle"_ustr));

    // All styles should be marked as OOXML defaults
    const ScTableStyle* pStyle = pStyles->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(pStyle->IsOOXMLDefault());

    m_pDoc->DeleteTab(0);
}

// Test 2: Verify fill colors are correctly resolved from theme
CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleFillColors)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    ScDBData* pDBData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr);

    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);

    // Header row (row 0) should have a solid fill based on Accent1 (red = 0xFF0000)
    const SvxBrushItem* pHeaderFill = pStyle->GetFillItem(*pDBData, 0, 0, 0);
    CPPUNIT_ASSERT(pHeaderFill);
    Color aHeaderColor = pHeaderFill->GetColor();
    // Header fill for Medium2 uses Accent1 (theme index 4) — should be red-based
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xFF), aHeaderColor.GetRed());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), aHeaderColor.GetGreen());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), aHeaderColor.GetBlue());

    // First row stripe (row 1, rowIndex 0) should have a tinted fill
    const SvxBrushItem* pStripeFill = pStyle->GetFillItem(*pDBData, 0, 1, 0);
    if (pStripeFill)
    {
        Color aStripeColor = pStripeFill->GetColor();
        // Tinted Accent1 — should still be reddish but lighter
        CPPUNIT_ASSERT(aStripeColor.GetRed() > aStripeColor.GetGreen());
        CPPUNIT_ASSERT(aStripeColor.GetRed() > aStripeColor.GetBlue());
    }

    // Second row stripe (row 2, rowIndex 1) — Medium2 has no fill for second stripe
    const SvxBrushItem* pStripe2Fill = pStyle->GetFillItem(*pDBData, 0, 2, 1);
    // Either null or transparent/white
    if (pStripe2Fill)
    {
        Color aStripe2Color = pStripe2Fill->GetColor();
        CPPUNIT_ASSERT(aStripe2Color == COL_WHITE || aStripe2Color == COL_TRANSPARENT);
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testGetDataAreaExcludeDBRange)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Loose column A (A1:A4) directly left of a table B1:C5 (no empty gap between them).
    m_pDoc->SetString(0, 0, 0, u"a1"_ustr);
    m_pDoc->SetString(0, 1, 0, u"a2"_ustr);
    m_pDoc->SetString(0, 2, 0, u"a3"_ustr);
    m_pDoc->SetString(0, 3, 0, u"a4"_ustr);
    for (SCCOL nCol = 1; nCol <= 2; ++nCol)
        for (SCROW nRow = 0; nRow <= 4; ++nRow)
            m_pDoc->SetString(nCol, nRow, 0, u"t"_ustr);
    createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, /*nCol1*/ 1, /*nRow1*/ 0, /*nCol2*/ 2,
                     /*nRow2*/ 4, /*bHasHeader*/ true, /*bHasTotals*/ false);

    SCCOL nStartCol = 0, nEndCol = 0;
    SCROW nStartRow = 1, nEndRow = 1;

    // Default: the auto-detect grows from the loose cell across into the table.
    m_pDoc->GetDataArea(0, nStartCol, nStartRow, nEndCol, nEndRow, /*bIncludeOld*/ true,
                        /*bOnlyDown*/ false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("default includes the table", SCCOL(2), nEndCol);

    // bExcludeDBRange: the table acts as a boundary, so only the loose column is detected.
    nStartCol = 0;
    nStartRow = 1;
    nEndCol = 0;
    nEndRow = 1;
    m_pDoc->GetDataArea(0, nStartCol, nStartRow, nEndCol, nEndRow, /*bIncludeOld*/ true,
                        /*bOnlyDown*/ false, /*bExcludeDBRange*/ true);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), nStartCol);
    CPPUNIT_ASSERT_EQUAL(SCROW(0), nStartRow);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("table excluded", SCCOL(0), nEndCol);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("re-shrunk past the table's extra row", SCROW(3), nEndRow);

    m_pDoc->DeleteTab(0);
}

// Test 3: Verify border widths and styles
CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleBorders)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    ScDBData* pDBData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr);

    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);

    // Data cell (row 5, middle of table) — should have thin borders from WholeTable
    const SvxBoxItem* pBoxItem = pStyle->GetBoxItem(*pDBData, 1, 5, 4);
    CPPUNIT_ASSERT(pBoxItem);

    // Top and bottom borders should be thin (SvxBorderLineWidth::Thin = 15)
    const editeng::SvxBorderLine* pTopLine = pBoxItem->GetLine(SvxBoxItemLine::TOP);
    CPPUNIT_ASSERT(pTopLine);
    CPPUNIT_ASSERT_EQUAL(tools::Long(15), pTopLine->GetWidth());

    const editeng::SvxBorderLine* pBotLine = pBoxItem->GetLine(SvxBoxItemLine::BOTTOM);
    CPPUNIT_ASSERT(pBotLine);
    CPPUNIT_ASSERT_EQUAL(tools::Long(15), pBotLine->GetWidth());

    // Border color should be Accent1-tinted (reddish)
    CPPUNIT_ASSERT(pTopLine->GetColor().GetRed() > pTopLine->GetColor().GetGreen());

    // B6 sits inside the table (A1:D11), so it has no vertical edge at all, while D6 in the
    // same band closes the table on the right. The two cells differ only in the column.
    CPPUNIT_ASSERT(!pBoxItem->GetLine(SvxBoxItemLine::LEFT));
    CPPUNIT_ASSERT(!pBoxItem->GetLine(SvxBoxItemLine::RIGHT));
    const SvxBoxItem* pLastColBox = pStyle->GetBoxItem(*pDBData, 3, 5, 4);
    CPPUNIT_ASSERT(pLastColBox);
    CPPUNIT_ASSERT(pLastColBox->GetLine(SvxBoxItemLine::RIGHT));
    CPPUNIT_ASSERT(!pLastColBox->GetLine(SvxBoxItemLine::LEFT));

    // Total row (row 10, rowIndex 9) — should have a DOUBLE top border
    const SvxBoxItem* pTotalBox = pStyle->GetBoxItem(*pDBData, 1, 10, 9);
    CPPUNIT_ASSERT(pTotalBox);
    const editeng::SvxBorderLine* pTotalTop = pTotalBox->GetLine(SvxBoxItemLine::TOP);
    CPPUNIT_ASSERT(pTotalTop);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOUBLE_THIN, pTotalTop->GetBorderLineStyle());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleBordersNoWholeTable)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    ScTableStyle aStyle(u"HeaderOnly"_ustr, std::nullopt);

    editeng::SvxBorderLine aLine(nullptr, SvxBorderLineWidth::Thick);
    SvxBoxItem aBox(ATTR_BORDER);
    aBox.SetLine(&aLine, SvxBoxItemLine::BOTTOM);

    auto pHeaderPattern = std::make_unique<ScPatternAttr>(m_pDoc->getCellAttributeHelper());
    pHeaderPattern->ItemSetPut(aBox);
    aStyle.SetPattern(ScTableStyleElement::HeaderRow, std::move(pHeaderPattern));

    ScDBData aDBData(u"HeaderOnly"_ustr, 0, 0, 0, 3, 10, true, true, false);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"HeaderOnly"_ustr;
    aStyleParam.mbRowStripes = false;
    aStyleParam.mbColumnStripes = false;
    aStyleParam.mbFirstColumn = false;
    aStyleParam.mbLastColumn = false;
    aDBData.SetTableStyleInfo(aStyleParam);

    const SvxBoxItem* pHeaderBox = aStyle.GetBoxItem(aDBData, 1, 0, -1);
    CPPUNIT_ASSERT(pHeaderBox);
    const editeng::SvxBorderLine* pBotLine = pHeaderBox->GetLine(SvxBoxItemLine::BOTTOM);
    CPPUNIT_ASSERT(pBotLine);
    CPPUNIT_ASSERT_EQUAL(tools::Long(SvxBorderLineWidth::Thick), pBotLine->GetWidth());

    CPPUNIT_ASSERT(!aStyle.GetBoxItem(aDBData, 1, 5, 4));
    CPPUNIT_ASSERT(!aStyle.GetFillItem(aDBData, 1, 5, 4));
    CPPUNIT_ASSERT(!aStyle.GetFontItemSet(aDBData, 1, 5, 4));

    m_pDoc->DeleteTab(0);
}

// Test 3b: Single-column table keeps both left and right outer edges (regression)
CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleBordersSingleColumn)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    // TableStyleMedium2's WholeTable is a full box (top/bottom/left/right), so a
    // one-column table (A1:A11) must still carry the right edge in every band.
    ScDBData* pDBData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 0, 10);

    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);

    auto assertOuterLeftRight = [](const SvxBoxItem* pBox, const char* pWhat) {
        CPPUNIT_ASSERT_MESSAGE(pWhat, pBox);
        CPPUNIT_ASSERT_MESSAGE(pWhat, pBox->GetLine(SvxBoxItemLine::LEFT) != nullptr);
        CPPUNIT_ASSERT_MESSAGE(pWhat, pBox->GetLine(SvxBoxItemLine::RIGHT) != nullptr);
    };

    assertOuterLeftRight(pStyle->GetBoxItem(*pDBData, 0, 0, 0), "single-column header cell");
    assertOuterLeftRight(pStyle->GetBoxItem(*pDBData, 0, 5, 4), "single-column data cell");
    assertOuterLeftRight(pStyle->GetBoxItem(*pDBData, 0, 10, 9), "single-column total cell");

    m_pDoc->DeleteTab(0);
}

// Test 3c: Style with inner vertical borders — interior cells get them, single column keeps edges
CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleBordersSingleColumnInnerVertical)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    // TableStyleLight17's WholeTable is a full box plus inner horizontal AND vertical lines.
    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleLight17"_ustr);
    CPPUNIT_ASSERT(pStyle);

    auto assertLeftRight = [](const SvxBoxItem* pBox, const char* pWhat) {
        CPPUNIT_ASSERT_MESSAGE(pWhat, pBox);
        CPPUNIT_ASSERT_MESSAGE(pWhat, pBox->GetLine(SvxBoxItemLine::LEFT) != nullptr);
        CPPUNIT_ASSERT_MESSAGE(pWhat, pBox->GetLine(SvxBoxItemLine::RIGHT) != nullptr);
    };

    // An interior cell of a multi-column table (A1:C11) has no outer edge, so left/right
    // lines there can only come from the inner vertical border.
    ScDBData* pMulti = createTestDBData(m_pDoc, u"TableStyleLight17"_ustr, 0, 0, 2, 10);
    assertLeftRight(pStyle->GetBoxItem(*pMulti, 1, 5, 4), "multi-column interior cell");

    // Single-column table (E1:E11) with the same style still shows both edges in every band.
    ScDBData* pOne = createTestDBData(m_pDoc, u"TableStyleLight17"_ustr, 4, 0, 4, 10, true, true,
                                      u"OneCol"_ustr);
    assertLeftRight(pStyle->GetBoxItem(*pOne, 4, 0, 0), "single-column header cell");
    assertLeftRight(pStyle->GetBoxItem(*pOne, 4, 5, 4), "single-column data cell");
    assertLeftRight(pStyle->GetBoxItem(*pOne, 4, 10, 9), "single-column total cell");

    m_pDoc->DeleteTab(0);
}

// Test 4: Verify font properties (bold, color)
CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleFonts)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    ScDBData* pDBData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr);

    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);

    // Header row should have bold white font
    const SfxItemSet* pHeaderFont = pStyle->GetFontItemSet(*pDBData, 0, 0, 0);
    CPPUNIT_ASSERT(pHeaderFont);

    // Check bold
    const SvxWeightItem* pWeight = pHeaderFont->GetItemIfSet(ATTR_FONT_WEIGHT, false);
    CPPUNIT_ASSERT(pWeight);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, pWeight->GetWeight());

    // Check white font color
    const SvxColorItem* pColorItem = pHeaderFont->GetItemIfSet(ATTR_FONT_COLOR, false);
    CPPUNIT_ASSERT(pColorItem);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pColorItem->GetValue());

    // Total row should have bold font
    const SfxItemSet* pTotalFont = pStyle->GetFontItemSet(*pDBData, 0, 10, 9);
    CPPUNIT_ASSERT(pTotalFont);
    const SvxWeightItem* pTotalWeight = pTotalFont->GetItemIfSet(ATTR_FONT_WEIGHT, false);
    CPPUNIT_ASSERT(pTotalWeight);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, pTotalWeight->GetWeight());

    // Data row should have no font override
    const SfxItemSet* pDataFont = pStyle->GetFontItemSet(*pDBData, 0, 5, 4);
    // Either null or empty — data rows typically have no font styling
    if (pDataFont)
    {
        const SvxWeightItem* pDataWeight = pDataFont->GetItemIfSet(ATTR_FONT_WEIGHT, false);
        // If present, should not be bold
        if (pDataWeight)
            CPPUNIT_ASSERT(pDataWeight->GetWeight() != WEIGHT_BOLD);
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleFontsAcrossElements)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    ScTableStyle aStyle(u"Split"_ustr, std::nullopt);

    auto pTablePattern = std::make_unique<ScPatternAttr>(m_pDoc->getCellAttributeHelper());
    pTablePattern->ItemSetPut(SvxWeightItem(WEIGHT_BOLD, ATTR_FONT_WEIGHT));
    pTablePattern->ItemSetPut(SvxColorItem(COL_BLUE, ATTR_FONT_COLOR));
    aStyle.SetPattern(ScTableStyleElement::WholeTable, std::move(pTablePattern));

    auto pHeaderPattern = std::make_unique<ScPatternAttr>(m_pDoc->getCellAttributeHelper());
    pHeaderPattern->ItemSetPut(SvxColorItem(COL_RED, ATTR_FONT_COLOR));
    aStyle.SetPattern(ScTableStyleElement::HeaderRow, std::move(pHeaderPattern));

    ScDBData aDBData(u"Split"_ustr, 0, 0, 0, 3, 10, true, true, false);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"Split"_ustr;
    aStyleParam.mbRowStripes = false;
    aStyleParam.mbColumnStripes = false;
    aStyleParam.mbFirstColumn = false;
    aStyleParam.mbLastColumn = false;
    aDBData.SetTableStyleInfo(aStyleParam);

    const SfxItemSet* pHeaderFont = aStyle.GetFontItemSet(aDBData, 0, 0, -1);
    CPPUNIT_ASSERT(pHeaderFont);
    const SvxColorItem* pHeaderColor = pHeaderFont->GetItemIfSet(ATTR_FONT_COLOR, false);
    CPPUNIT_ASSERT(pHeaderColor);
    CPPUNIT_ASSERT_EQUAL(COL_RED, pHeaderColor->GetValue());
    const SvxWeightItem* pHeaderWeight = pHeaderFont->GetItemIfSet(ATTR_FONT_WEIGHT, false);
    CPPUNIT_ASSERT(pHeaderWeight);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, pHeaderWeight->GetWeight());

    const SfxItemSet* pDataFont = aStyle.GetFontItemSet(aDBData, 0, 5, 4);
    CPPUNIT_ASSERT(pDataFont);
    const SvxColorItem* pDataColor = pDataFont->GetItemIfSet(ATTR_FONT_COLOR, false);
    CPPUNIT_ASSERT(pDataColor);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, pDataColor->GetValue());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleTotalCells)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    ScTableStyle aStyle(u"TotalCells"_ustr, std::nullopt);

    auto pTotalRow = std::make_unique<ScPatternAttr>(m_pDoc->getCellAttributeHelper());
    pTotalRow->ItemSetPut(SvxBrushItem(COL_LIGHTBLUE, ATTR_BACKGROUND));
    aStyle.SetPattern(ScTableStyleElement::TotalRow, std::move(pTotalRow));

    auto pFirstColumn = std::make_unique<ScPatternAttr>(m_pDoc->getCellAttributeHelper());
    pFirstColumn->ItemSetPut(SvxBrushItem(COL_GREEN, ATTR_BACKGROUND));
    aStyle.SetPattern(ScTableStyleElement::FirstColumn, std::move(pFirstColumn));

    auto pFirstTotal = std::make_unique<ScPatternAttr>(m_pDoc->getCellAttributeHelper());
    pFirstTotal->ItemSetPut(SvxBrushItem(COL_RED, ATTR_BACKGROUND));
    pFirstTotal->ItemSetPut(SvxWeightItem(WEIGHT_BOLD, ATTR_FONT_WEIGHT));
    SvxBoxItem aBox(ATTR_BORDER);
    editeng::SvxBorderLine aLine(nullptr, SvxBorderLineWidth::Thick);
    aBox.SetLine(&aLine, SvxBoxItemLine::BOTTOM);
    pFirstTotal->ItemSetPut(aBox);
    aStyle.SetPattern(ScTableStyleElement::FirstTotalCell, std::move(pFirstTotal));

    ScDBData aDBData(u"TotalCells"_ustr, 0, 0, 0, 3, 10, true, true, true);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TotalCells"_ustr;
    aStyleParam.mbRowStripes = false;
    aStyleParam.mbColumnStripes = false;
    aStyleParam.mbFirstColumn = true;
    aStyleParam.mbLastColumn = false;
    aDBData.SetTableStyleInfo(aStyleParam);

    const SvxBrushItem* pFill = aStyle.GetFillItem(aDBData, 0, 10, 9);
    CPPUNIT_ASSERT(pFill);
    CPPUNIT_ASSERT_EQUAL(COL_RED, pFill->GetColor());

    pFill = aStyle.GetFillItem(aDBData, 3, 10, 9);
    CPPUNIT_ASSERT(pFill);
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, pFill->GetColor());

    pFill = aStyle.GetFillItem(aDBData, 1, 10, 9);
    CPPUNIT_ASSERT(pFill);
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, pFill->GetColor());

    pFill = aStyle.GetFillItem(aDBData, 0, 5, 4);
    CPPUNIT_ASSERT(pFill);
    CPPUNIT_ASSERT_EQUAL(COL_GREEN, pFill->GetColor());

    const SfxItemSet* pFont = aStyle.GetFontItemSet(aDBData, 0, 10, 9);
    CPPUNIT_ASSERT(pFont);
    const SvxWeightItem* pWeight = pFont->GetItemIfSet(ATTR_FONT_WEIGHT, false);
    CPPUNIT_ASSERT(pWeight);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, pWeight->GetWeight());

    const SvxBoxItem* pBox = aStyle.GetBoxItem(aDBData, 0, 10, 9);
    CPPUNIT_ASSERT(pBox);
    const editeng::SvxBorderLine* pBotLine = pBox->GetLine(SvxBoxItemLine::BOTTOM);
    CPPUNIT_ASSERT(pBotLine);
    CPPUNIT_ASSERT_EQUAL(tools::Long(SvxBorderLineWidth::Thick), pBotLine->GetWidth());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleFontsHeaderFirstColumn)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleDark8"_ustr);
    CPPUNIT_ASSERT(pStyle);

    ScDBData aDBData(u"HeaderFirstCol"_ustr, 0, 0, 0, 3, 10, true, true, false);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleDark8"_ustr;
    aStyleParam.mbRowStripes = false;
    aStyleParam.mbColumnStripes = false;
    aStyleParam.mbFirstColumn = true;
    aStyleParam.mbLastColumn = true;
    aDBData.SetTableStyleInfo(aStyleParam);

    const SfxItemSet* pCornerFont = pStyle->GetFontItemSet(aDBData, 0, 0, -1);
    CPPUNIT_ASSERT(pCornerFont);
    const SvxColorItem* pColor = pCornerFont->GetItemIfSet(ATTR_FONT_COLOR, false);
    CPPUNIT_ASSERT(pColor);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pColor->GetValue());
    const SvxWeightItem* pWeight = pCornerFont->GetItemIfSet(ATTR_FONT_WEIGHT, false);
    CPPUNIT_ASSERT(pWeight);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, pWeight->GetWeight());

    const SfxItemSet* pHeaderFont = pStyle->GetFontItemSet(aDBData, 1, 0, -1);
    CPPUNIT_ASSERT(pHeaderFont);
    CPPUNIT_ASSERT(!pHeaderFont->GetItemIfSet(ATTR_FONT_WEIGHT, false));

    m_pDoc->DeleteTab(0);
}

// Test 5: Verify styles regenerate with new theme colors
CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleThemeChange)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Start with Theme A (Accent1 = red)
    auto pColorSetA = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSetA);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSetA);

    ScDBData* pDBData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr);

    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);

    // Verify header fill is red-based (Accent1 = 0xFF0000)
    const SvxBrushItem* pFillA = pStyle->GetFillItem(*pDBData, 0, 0, 0);
    CPPUNIT_ASSERT(pFillA);
    Color aColorA = pFillA->GetColor();
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xFF), aColorA.GetRed());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), aColorA.GetBlue());

    // Apply Theme B (Accent1 = blue) via ThemeColorChanger
    auto pColorSetB = createTestThemeB();
    sc::ThemeColorChanger aChanger(*m_xDocShell);
    aChanger.apply(pColorSetB);

    // Re-fetch style pointer (regenerated)
    pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);

    // Verify header fill is now blue-based (Accent1 = 0x0000FF)
    const SvxBrushItem* pFillB = pStyle->GetFillItem(*pDBData, 0, 0, 0);
    CPPUNIT_ASSERT(pFillB);
    Color aColorB = pFillB->GetColor();
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), aColorB.GetRed());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xFF), aColorB.GetBlue());

    // Border colors should also have changed
    const SvxBoxItem* pBoxB = pStyle->GetBoxItem(*pDBData, 1, 5, 4);
    CPPUNIT_ASSERT(pBoxB);
    const editeng::SvxBorderLine* pLine = pBoxB->GetLine(SvxBoxItemLine::TOP);
    CPPUNIT_ASSERT(pLine);
    // Should be blue-based now
    CPPUNIT_ASSERT(pLine->GetColor().GetBlue() > pLine->GetColor().GetRed());

    m_pDoc->DeleteTab(0);
}

// Test 6: Verify undo/redo restores correct table style colors
CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleThemeChangeUndoRedo)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Start with Theme A (Accent1 = red)
    auto pColorSetA = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSetA);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSetA);

    ScDBData* pDBData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr);

    // Verify initial state: red
    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);
    const SvxBrushItem* pFill = pStyle->GetFillItem(*pDBData, 0, 0, 0);
    CPPUNIT_ASSERT(pFill);
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xFF), pFill->GetColor().GetRed());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), pFill->GetColor().GetBlue());

    // Apply Theme B (Accent1 = blue)
    auto pColorSetB = createTestThemeB();
    sc::ThemeColorChanger aChanger(*m_xDocShell);
    aChanger.apply(pColorSetB);

    // Verify: blue
    pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);
    pFill = pStyle->GetFillItem(*pDBData, 0, 0, 0);
    CPPUNIT_ASSERT(pFill);
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), pFill->GetColor().GetRed());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xFF), pFill->GetColor().GetBlue());

    // Undo → should be red again
    m_pDoc->GetUndoManager()->Undo();

    pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);
    pFill = pStyle->GetFillItem(*pDBData, 0, 0, 0);
    CPPUNIT_ASSERT(pFill);
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xFF), pFill->GetColor().GetRed());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), pFill->GetColor().GetBlue());

    // Redo → should be blue again
    m_pDoc->GetUndoManager()->Redo();

    pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);
    pFill = pStyle->GetFillItem(*pDBData, 0, 0, 0);
    CPPUNIT_ASSERT(pFill);
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), pFill->GetColor().GetRed());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xFF), pFill->GetColor().GetBlue());

    m_pDoc->DeleteTab(0);
}

// Test 7: Verify multiple style categories have correct structure
CPPUNIT_TEST_FIXTURE(TableStylesTest, testMultipleTableStyleCategories)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    // range: A1:D11 (row 0 = header, rows 1-9 = data, row 10 = total)
    ScDBData* pDBData = new ScDBData(u"TestTable"_ustr, 0, 0, 0, 3, 10, true, true, true);
    ScTableStyleParam aStyleParam;
    aStyleParam.mbRowStripes = true;
    aStyleParam.mbColumnStripes = false;
    aStyleParam.mbFirstColumn = false;
    aStyleParam.mbLastColumn = false;

    ScTableStyles* pStyles = m_pDoc->GetTableStyles();
    CPPUNIT_ASSERT(pStyles);

    // Test Light style: typically no header fill, thin borders
    {
        const ScTableStyle* pLight = pStyles->GetTableStyle(u"TableStyleLight1"_ustr);
        CPPUNIT_ASSERT(pLight);

        aStyleParam.maStyleID = u"TableStyleLight1"_ustr;
        pDBData->SetTableStyleInfo(aStyleParam);

        // Light1 header row — may or may not have a fill, but should have a font
        const SfxItemSet* pHeaderFont = pLight->GetFontItemSet(*pDBData, 0, 0, 0);
        CPPUNIT_ASSERT(pHeaderFont);
        const SvxWeightItem* pWeight = pHeaderFont->GetItemIfSet(ATTR_FONT_WEIGHT, false);
        CPPUNIT_ASSERT(pWeight);
        CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, pWeight->GetWeight());
    }

    // Test Medium styles: various accent-based styles
    {
        const OUString aMediumStyles[] = { u"TableStyleMedium1"_ustr, u"TableStyleMedium2"_ustr,
                                           u"TableStyleMedium15"_ustr, u"TableStyleMedium28"_ustr };

        for (const auto& rName : aMediumStyles)
        {
            const ScTableStyle* pMedium = pStyles->GetTableStyle(rName);
            CPPUNIT_ASSERT_MESSAGE(OString("Style not found: " + rName.toUtf8()).getStr(),
                                   pMedium != nullptr);

            aStyleParam.maStyleID = rName;
            pDBData->SetTableStyleInfo(aStyleParam);

            // Medium styles should have header fill
            const SvxBrushItem* pFill = pMedium->GetFillItem(*pDBData, 0, 0, 0);
            CPPUNIT_ASSERT_MESSAGE(OString("No header fill for: " + rName.toUtf8()).getStr(),
                                   pFill != nullptr);

            // Medium styles should have borders on data cells
            const SvxBoxItem* pBox = pMedium->GetBoxItem(*pDBData, 1, 5, 4);
            CPPUNIT_ASSERT_MESSAGE(OString("No borders for: " + rName.toUtf8()).getStr(),
                                   pBox != nullptr);
        }
    }

    // Test Dark styles: dark fill, white font
    {
        const ScTableStyle* pDark = pStyles->GetTableStyle(u"TableStyleDark1"_ustr);
        CPPUNIT_ASSERT(pDark);

        aStyleParam.maStyleID = u"TableStyleDark1"_ustr;
        pDBData->SetTableStyleInfo(aStyleParam);

        // Dark1 header should have a fill
        const SvxBrushItem* pFill = pDark->GetFillItem(*pDBData, 0, 0, 0);
        CPPUNIT_ASSERT(pFill);

        // Dark1 header font should be bold and white
        const SfxItemSet* pFont = pDark->GetFontItemSet(*pDBData, 0, 0, 0);
        CPPUNIT_ASSERT(pFont);
        const SvxWeightItem* pWeight = pFont->GetItemIfSet(ATTR_FONT_WEIGHT, false);
        CPPUNIT_ASSERT(pWeight);
        CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, pWeight->GetWeight());
        const SvxColorItem* pColor = pFont->GetItemIfSet(ATTR_FONT_COLOR, false);
        CPPUNIT_ASSERT(pColor);
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, pColor->GetValue());
    }

    delete pDBData;
    m_pDoc->DeleteTab(0);
}

// Test 8: Verify changing table style on a database range
CPPUNIT_TEST_FIXTURE(TableStylesTest, testChangeTableStyle)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    // Create DB range with initial style
    ScDBData* pDBData = new ScDBData(u"TestTable"_ustr, 0, 0, 0, 3, 10, true, true, true);
    ScTableStyleParam aStyleParam;
    aStyleParam.mbRowStripes = true;
    aStyleParam.mbColumnStripes = false;
    aStyleParam.mbFirstColumn = false;
    aStyleParam.mbLastColumn = false;
    aStyleParam.maStyleID = u"TableStyleLight1"_ustr;
    pDBData->SetTableStyleInfo(aStyleParam);

    bool bInserted
        = m_pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    ScTableStyles* pStyles = m_pDoc->GetTableStyles();
    CPPUNIT_ASSERT(pStyles);

    // --- Style 1: TableStyleLight1 ---
    {
        const ScTableStyle* pStyle = pStyles->GetTableStyle(u"TableStyleLight1"_ustr);
        CPPUNIT_ASSERT(pStyle);

        // Light1 header — should have bold font
        const SfxItemSet* pFont = pStyle->GetFontItemSet(*pDBData, 0, 0, 0);
        CPPUNIT_ASSERT(pFont);

        // Remember header fill for comparison
        const SvxBrushItem* pFillLight = pStyle->GetFillItem(*pDBData, 0, 0, 0);
        // Light styles may or may not have header fill — just record for later comparison
        Color aLightHeaderColor = pFillLight ? pFillLight->GetColor() : COL_TRANSPARENT;

        // --- Switch to Style 2: TableStyleMedium2 ---
        aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
        pDBData->SetTableStyleInfo(aStyleParam);

        const ScTableStyle* pMedium = pStyles->GetTableStyle(u"TableStyleMedium2"_ustr);
        CPPUNIT_ASSERT(pMedium);

        // Medium2 header should have Accent1-based fill (red)
        const SvxBrushItem* pFillMedium = pMedium->GetFillItem(*pDBData, 0, 0, 0);
        CPPUNIT_ASSERT(pFillMedium);
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xFF), pFillMedium->GetColor().GetRed());

        // The fill should differ from Light1
        CPPUNIT_ASSERT(pFillMedium->GetColor() != aLightHeaderColor);

        // --- Switch to Style 3: TableStyleDark1 ---
        aStyleParam.maStyleID = u"TableStyleDark1"_ustr;
        pDBData->SetTableStyleInfo(aStyleParam);

        const ScTableStyle* pDark = pStyles->GetTableStyle(u"TableStyleDark1"_ustr);
        CPPUNIT_ASSERT(pDark);

        // Dark1 should have white font on header
        const SfxItemSet* pDarkFont = pDark->GetFontItemSet(*pDBData, 0, 0, 0);
        CPPUNIT_ASSERT(pDarkFont);
        const SvxColorItem* pDarkColor = pDarkFont->GetItemIfSet(ATTR_FONT_COLOR, false);
        CPPUNIT_ASSERT(pDarkColor);
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, pDarkColor->GetValue());

        // Dark1 header fill should differ from Medium2
        const SvxBrushItem* pFillDark = pDark->GetFillItem(*pDBData, 0, 0, 0);
        CPPUNIT_ASSERT(pFillDark);
        CPPUNIT_ASSERT(pFillDark->GetColor() != pFillMedium->GetColor());

        // --- Switch to Style 4: TableStyleLight8 (with column stripes) ---
        aStyleParam.maStyleID = u"TableStyleLight8"_ustr;
        aStyleParam.mbColumnStripes = true;
        aStyleParam.mbRowStripes = false;
        pDBData->SetTableStyleInfo(aStyleParam);

        const ScTableStyle* pLight8 = pStyles->GetTableStyle(u"TableStyleLight8"_ustr);
        CPPUNIT_ASSERT(pLight8);

        // Should have valid font on header
        const SfxItemSet* pLight8Font = pLight8->GetFontItemSet(*pDBData, 0, 0, 0);
        CPPUNIT_ASSERT(pLight8Font);

        // --- Switch to no style (empty / "None") ---
        aStyleParam.maStyleID = u""_ustr;
        aStyleParam.mbColumnStripes = false;
        aStyleParam.mbRowStripes = true;
        pDBData->SetTableStyleInfo(aStyleParam);

        // With empty style ID, GetTableStyle returns null — no crash, no formatting
        const ScTableStyle* pNone = pStyles->GetTableStyle(u""_ustr);
        CPPUNIT_ASSERT(!pNone);

        // Also test "None" explicitly
        aStyleParam.maStyleID = u"None"_ustr;
        pDBData->SetTableStyleInfo(aStyleParam);

        const ScTableStyle* pNone2 = pStyles->GetTableStyle(u"None"_ustr);
        // "None" is not a real style — should be null
        CPPUNIT_ASSERT(!pNone2);
    }

    m_pDoc->DeleteTab(0);
}

// Test 9: Verify table style adapts to no header / no total row
CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleNoHeaderNoTotal)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);

    // No header, no total: range A1:D10, bHasH=false, bTotals=false
    ScDBData aDBNoHeaderNoTotal(u"NoHeaderNoTotal"_ustr, 0, 0, 0, 3, 9, true, false, false);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    aStyleParam.mbRowStripes = true;
    aStyleParam.mbColumnStripes = false;
    aStyleParam.mbFirstColumn = false;
    aStyleParam.mbLastColumn = false;
    aDBNoHeaderNoTotal.SetTableStyleInfo(aStyleParam);

    // Row 0 should be data (first row stripe), NOT header
    const SvxBrushItem* pFillRow0 = pStyle->GetFillItem(aDBNoHeaderNoTotal, 0, 0, 0);
    // With header, row 0 would be solid Accent1 (0xFF0000).
    // Without header, row 0 is first data row — should have first row stripe pattern
    if (pFillRow0)
    {
        // Should NOT be pure Accent1 red (that's the header color)
        CPPUNIT_ASSERT(pFillRow0->GetColor() != Color(0xFF0000));
    }

    // Compare: in a table WITH header, row 1 (first data row) should have the same
    // fill as row 0 in a table WITHOUT header — both are first row stripe
    ScDBData aDBWithHeader(u"WithHeader"_ustr, 0, 0, 0, 3, 10, true, true, false);
    aDBWithHeader.SetTableStyleInfo(aStyleParam);
    const SvxBrushItem* pFirstStripeWithHeader = pStyle->GetFillItem(aDBWithHeader, 0, 1, 0);
    if (pFillRow0 && pFirstStripeWithHeader)
    {
        // Both should be the same first row stripe color
        CPPUNIT_ASSERT_EQUAL(pFirstStripeWithHeader->GetColor(), pFillRow0->GetColor());
    }

    // No total row — last row (row 9) is just a data row, not a total row
    // No DOUBLE_THIN border
    const SvxBoxItem* pBoxLast = pStyle->GetBoxItem(aDBNoHeaderNoTotal, 1, 9, 9);
    if (pBoxLast)
    {
        const editeng::SvxBorderLine* pTopLine = pBoxLast->GetLine(SvxBoxItemLine::TOP);
        if (pTopLine)
            CPPUNIT_ASSERT(pTopLine->GetBorderLineStyle() != SvxBorderLineStyle::DOUBLE_THIN);
    }

    // No bold font on last row (total row would have bold)
    const SfxItemSet* pLastRowFont = pStyle->GetFontItemSet(aDBNoHeaderNoTotal, 0, 9, 9);
    if (pLastRowFont)
    {
        const SvxWeightItem* pWeight = pLastRowFont->GetItemIfSet(ATTR_FONT_WEIGHT, false);
        if (pWeight)
            CPPUNIT_ASSERT(pWeight->GetWeight() != WEIGHT_BOLD);
    }

    // Last row should have normal stripe fill, same as other data rows
    // Row 9 (rowIndex 9) with no header: even index = first stripe, odd = second
    const SvxBrushItem* pLastRowFill = pStyle->GetFillItem(aDBNoHeaderNoTotal, 0, 9, 9);
    // Compare with another data row at the same stripe position
    const SvxBrushItem* pDataRowFill = pStyle->GetFillItem(aDBNoHeaderNoTotal, 0, 1, 1);
    if (pLastRowFill && pDataRowFill)
    {
        // Same stripe position → same color
        CPPUNIT_ASSERT_EQUAL(pDataRowFill->GetColor(), pLastRowFill->GetColor());
    }

    // With header, with total
    ScDBData aDBWithBoth(u"WithBoth"_ustr, 0, 0, 0, 3, 10, true, true, true);
    aDBWithBoth.SetTableStyleInfo(aStyleParam);

    // Row 0 should be header — solid Accent1
    const SvxBrushItem* pHeaderFill = pStyle->GetFillItem(aDBWithBoth, 0, 0, 0);
    CPPUNIT_ASSERT(pHeaderFill);
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xFF), pHeaderFill->GetColor().GetRed());

    // Total row (row 10) should have DOUBLE_THIN top border
    const SvxBoxItem* pTotalBox = pStyle->GetBoxItem(aDBWithBoth, 1, 10, 9);
    CPPUNIT_ASSERT(pTotalBox);
    const editeng::SvxBorderLine* pTotalTop = pTotalBox->GetLine(SvxBoxItemLine::TOP);
    CPPUNIT_ASSERT(pTotalTop);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOUBLE_THIN, pTotalTop->GetBorderLineStyle());

    m_pDoc->DeleteTab(0);
}

// Test 10: Verify banded columns work correctly
CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleBandedColumns)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    // Use a style that supports column stripes (Light8 has column stripe patterns)
    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleLight8"_ustr);
    CPPUNIT_ASSERT(pStyle);

    ScDBData aDBData(u"BandedCols"_ustr, 0, 0, 0, 5, 10, true, true, false);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleLight8"_ustr;
    aStyleParam.mbRowStripes = false;
    aStyleParam.mbColumnStripes = true;
    aStyleParam.mbFirstColumn = false;
    aStyleParam.mbLastColumn = false;
    aDBData.SetTableStyleInfo(aStyleParam);

    // With column stripes enabled, alternating columns should have different fills
    // Data row (row 1, rowIndex 0): col 0 vs col 1 should differ
    const SvxBrushItem* pFillCol0 = pStyle->GetFillItem(aDBData, 0, 1, 0);
    const SvxBrushItem* pFillCol1 = pStyle->GetFillItem(aDBData, 1, 1, 0);

    // At least one should have a fill for column stripes to be visible
    // The two columns should have different styling (one striped, one not)
    if (pFillCol0 && pFillCol1)
    {
        // They should differ — that's what banded columns means
        CPPUNIT_ASSERT(pFillCol0->GetColor() != pFillCol1->GetColor());
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleBandingSkipsHeaderAndTotals)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Only the stripes carry a fill, so a header or totals cell that wrongly counts
    // as banded shows the stripe colour instead of no fill at all.
    ScTableStyle aStyle(u"Banding"_ustr, std::nullopt);

    auto pColumnStripe = std::make_unique<ScPatternAttr>(m_pDoc->getCellAttributeHelper());
    pColumnStripe->ItemSetPut(SvxBrushItem(COL_GREEN, ATTR_BACKGROUND));
    aStyle.SetPattern(ScTableStyleElement::FirstColumnStripe, std::move(pColumnStripe));

    auto pRowStripe = std::make_unique<ScPatternAttr>(m_pDoc->getCellAttributeHelper());
    pRowStripe->ItemSetPut(SvxBrushItem(COL_YELLOW, ATTR_BACKGROUND));
    aStyle.SetPattern(ScTableStyleElement::FirstRowStripe, std::move(pRowStripe));

    // The totals row sits on the second stripe, so that one needs a fill too for the
    // totals check below to be able to fail
    auto pSecondRowStripe = std::make_unique<ScPatternAttr>(m_pDoc->getCellAttributeHelper());
    pSecondRowStripe->ItemSetPut(SvxBrushItem(COL_MAGENTA, ATTR_BACKGROUND));
    aStyle.SetPattern(ScTableStyleElement::SecondRowStripe, std::move(pSecondRowStripe));

    ScDBData aDBData(u"Banding"_ustr, 0, 0, 0, 3, 10, true, true, true);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"Banding"_ustr;
    aStyleParam.mbFirstColumn = false;
    aStyleParam.mbLastColumn = false;

    // Column banding covers the data rows, never the header or the totals row
    aStyleParam.mbRowStripes = false;
    aStyleParam.mbColumnStripes = true;
    aDBData.SetTableStyleInfo(aStyleParam);

    const SvxBrushItem* pFill = aStyle.GetFillItem(aDBData, 0, 1, 0);
    CPPUNIT_ASSERT(pFill);
    CPPUNIT_ASSERT_EQUAL(COL_GREEN, pFill->GetColor());

    CPPUNIT_ASSERT(!aStyle.GetFillItem(aDBData, 0, 0, -1));
    CPPUNIT_ASSERT(!aStyle.GetFillItem(aDBData, 0, 10, 9));

    // Row banding follows the same rule
    aStyleParam.mbRowStripes = true;
    aStyleParam.mbColumnStripes = false;
    aDBData.SetTableStyleInfo(aStyleParam);

    pFill = aStyle.GetFillItem(aDBData, 0, 1, 0);
    CPPUNIT_ASSERT(pFill);
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, pFill->GetColor());

    CPPUNIT_ASSERT(!aStyle.GetFillItem(aDBData, 0, 0, -1));
    CPPUNIT_ASSERT(!aStyle.GetFillItem(aDBData, 0, 10, 9));

    m_pDoc->DeleteTab(0);
}

// Test 11: Verify first/last column special formatting
CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleFirstLastColumn)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    // Use a style that has first/last column patterns
    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);

    // Range A1:D11 with first and last column enabled
    ScDBData aDBData(u"FirstLastCol"_ustr, 0, 0, 0, 3, 10, true, true, true);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    aStyleParam.mbRowStripes = true;
    aStyleParam.mbColumnStripes = false;
    aStyleParam.mbFirstColumn = true;
    aStyleParam.mbLastColumn = true;
    aDBData.SetTableStyleInfo(aStyleParam);

    // First column (col 0) data row should have bold font
    const SfxItemSet* pFirstColFont = pStyle->GetFontItemSet(aDBData, 0, 5, 4);
    if (pFirstColFont)
    {
        const SvxWeightItem* pWeight = pFirstColFont->GetItemIfSet(ATTR_FONT_WEIGHT, false);
        if (pWeight)
            CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, pWeight->GetWeight());
    }

    // Last column (col 3) data row should also have bold font
    const SfxItemSet* pLastColFont = pStyle->GetFontItemSet(aDBData, 3, 5, 4);
    if (pLastColFont)
    {
        const SvxWeightItem* pWeight = pLastColFont->GetItemIfSet(ATTR_FONT_WEIGHT, false);
        if (pWeight)
            CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, pWeight->GetWeight());
    }

    // Middle column (col 1) should NOT have bold font from first/last column
    const SfxItemSet* pMidColFont = pStyle->GetFontItemSet(aDBData, 1, 5, 4);
    if (pMidColFont)
    {
        const SvxWeightItem* pWeight = pMidColFont->GetItemIfSet(ATTR_FONT_WEIGHT, false);
        if (pWeight)
            CPPUNIT_ASSERT(pWeight->GetWeight() != WEIGHT_BOLD);
    }

    m_pDoc->DeleteTab(0);
}

// Test 12: The smallest Tables — one column, and the smallest one with a total row
CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleTinyTable)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);

    // Tiny table: A1:B2 — header row + 1 data row, no total
    ScDBData aDB1(u"Tiny1"_ustr, 0, 0, 0, 1, 1, true, true, false);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    aStyleParam.mbRowStripes = true;
    aStyleParam.mbColumnStripes = false;
    aStyleParam.mbFirstColumn = false;
    aStyleParam.mbLastColumn = false;
    aDB1.SetTableStyleInfo(aStyleParam);

    // Header row 0
    const SvxBrushItem* pFill = pStyle->GetFillItem(aDB1, 0, 0, 0);
    CPPUNIT_ASSERT(pFill); // header should still have fill
    const SfxItemSet* pFont = pStyle->GetFontItemSet(aDB1, 0, 0, 0);
    CPPUNIT_ASSERT(pFont);

    // A1 opens the table on the left, and B1 closes it on the right, so the header cell
    // carries the top and left edges but no right one.
    const SvxBoxItem* pHeaderBox = pStyle->GetBoxItem(aDB1, 0, 0, 0);
    CPPUNIT_ASSERT(pHeaderBox);
    CPPUNIT_ASSERT(pHeaderBox->GetLine(SvxBoxItemLine::TOP));
    CPPUNIT_ASSERT(pHeaderBox->GetLine(SvxBoxItemLine::LEFT));
    CPPUNIT_ASSERT(!pHeaderBox->GetLine(SvxBoxItemLine::RIGHT));

    // Data row 1 — the only data row, so it is the last row of the table as well. Without a
    // total row it must not take the total row's double top border, and it has to close the
    // table at the bottom.
    pStyle->GetFillItem(aDB1, 0, 1, 0);
    pStyle->GetFontItemSet(aDB1, 0, 1, 0);
    const SvxBoxItem* pDataBox = pStyle->GetBoxItem(aDB1, 0, 1, 0);
    CPPUNIT_ASSERT(pDataBox);
    const editeng::SvxBorderLine* pDataTop = pDataBox->GetLine(SvxBoxItemLine::TOP);
    CPPUNIT_ASSERT(pDataTop);
    CPPUNIT_ASSERT(pDataTop->GetBorderLineStyle() != SvxBorderLineStyle::DOUBLE_THIN);
    CPPUNIT_ASSERT(pDataBox->GetLine(SvxBoxItemLine::BOTTOM));

    // Single column table: A1:A2 — header + one data row
    ScDBData aDB2(u"Tiny2"_ustr, 0, 0, 0, 0, 1, true, true, false);
    aDB2.SetTableStyleInfo(aStyleParam);

    pStyle->GetFillItem(aDB2, 0, 0, 0);
    pStyle->GetFontItemSet(aDB2, 0, 0, 0);

    // A1 is the first and the last column at once, so it takes both outer edges. The same
    // cell of aDB1 above took only the left one, and both tables share this ScTableStyle.
    const SvxBoxItem* pOneColBox = pStyle->GetBoxItem(aDB2, 0, 0, 0);
    CPPUNIT_ASSERT(pOneColBox);
    CPPUNIT_ASSERT(pOneColBox->GetLine(SvxBoxItemLine::TOP));
    CPPUNIT_ASSERT(pOneColBox->GetLine(SvxBoxItemLine::LEFT));
    CPPUNIT_ASSERT(pOneColBox->GetLine(SvxBoxItemLine::RIGHT));

    // Header + two data rows + total: A1:B4
    ScDBData aDB3(u"Tiny3"_ustr, 0, 0, 0, 1, 3, true, true, true);
    aDB3.SetTableStyleInfo(aStyleParam);

    pStyle->GetFillItem(aDB3, 0, 0, 0);
    pStyle->GetFillItem(aDB3, 0, 3, 2);
    pStyle->GetBoxItem(aDB3, 0, 0, 0);

    // A4 is the total row: double top border, and the outer bottom and left edges. Its row
    // index is even like aDB1's data row above, so both cells land on the same row stripe
    // and the total row is all that separates them. Keep the data row count even.
    const SvxBoxItem* pTotalBox = pStyle->GetBoxItem(aDB3, 0, 3, 2);
    CPPUNIT_ASSERT(pTotalBox);
    const editeng::SvxBorderLine* pTotalTop = pTotalBox->GetLine(SvxBoxItemLine::TOP);
    CPPUNIT_ASSERT(pTotalTop);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOUBLE_THIN, pTotalTop->GetBorderLineStyle());
    CPPUNIT_ASSERT(pTotalBox->GetLine(SvxBoxItemLine::BOTTOM));
    CPPUNIT_ASSERT(pTotalBox->GetLine(SvxBoxItemLine::LEFT));
    CPPUNIT_ASSERT(!pTotalBox->GetLine(SvxBoxItemLine::RIGHT));

    // B4 ends both the total row and the table
    const SvxBoxItem* pLastTotalBox = pStyle->GetBoxItem(aDB3, 1, 3, 2);
    CPPUNIT_ASSERT(pLastTotalBox);
    CPPUNIT_ASSERT(pLastTotalBox->GetLine(SvxBoxItemLine::RIGHT));
    CPPUNIT_ASSERT(pLastTotalBox->GetLine(SvxBoxItemLine::BOTTOM));
    CPPUNIT_ASSERT(!pLastTotalBox->GetLine(SvxBoxItemLine::LEFT));

    m_pDoc->DeleteTab(0);
}

// Auto-expansion: typing in the row immediately below a styled named DBData
// (without a Total Row) grows the table down by one row, on its own undo step.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoExpandRowDown)
{
    m_pDoc->InsertTab(0, u"AutoExpandDown"_ustr);

    // Styled table at A1:D5, header, no total. Adjacency row band = A6:D6.
    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 3, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);

    // Type into B6 — the row band immediately below.
    m_pDoc->SetString(ScAddress(1, 5, 0), u"hello"_ustr);

    CPPUNIT_ASSERT_MESSAGE("Notify should have flagged the row-down expansion",
                           pData->HasPendingExpansion());

    // Drain.
    m_xDocShell->ProcessPendingTableExpansions();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("table should now reach row 6", ScRange(0, 0, 0, 3, 5, 0),
                                 getArea(*pData));
    CPPUNIT_ASSERT_MESSAGE("pending flag should be cleared after drain",
                           !pData->HasPendingExpansion());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testResizeDoesNotAutoExpandOverStructRef)
{
    m_pDoc->InsertTab(0, u"ResizeNoAutoExpand"_ustr);

    // Styled table A1:C8 (TestTable): header Name|Number|Column3, data rows 1-7, no Total Row.
    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 7,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    m_pDoc->SetString(0, 0, 0, u"Name"_ustr);
    m_pDoc->SetString(1, 0, 0, u"Number"_ustr);
    m_pDoc->SetString(2, 0, 0, u"Column3"_ustr);
    for (SCROW nRow = 1; nRow <= 7; ++nRow)
        m_pDoc->SetValue(1, nRow, 0, static_cast<double>(nRow)); // Number: 1..7 (sum 28)
    pData->RefreshTableColumnNames(m_pDoc);

    // A structured-ref formula at G9, below-right of the table and OUTSIDE its current bands
    // (row band A9:D9, col band D1:D9), so placing it flags no expansion.
    m_pDoc->SetString(6, 8, 0, u"=SUBTOTAL(9;TestTable[Number])"_ustr);
    m_pDoc->CalcAll();
    CPPUNIT_ASSERT_EQUAL(28.0, m_pDoc->GetValue(6, 8, 0));
    CPPUNIT_ASSERT_MESSAGE("placing the struct ref outside the bands must not flag expansion",
                           !pData->HasPendingExpansion());

    // Widen by columns only: A1:C8 -> A1:G8. G9 is now in the row band directly below, but a
    // column resize must NOT grow a row (that stray row would swallow G9 and break it to #REF!).
    ScDBDocFunc(*m_xDocShell).ResizeTable(*pData, ScRange(0, 0, 0, 6, 7, 0));
    m_xDocShell->ProcessPendingTableExpansions();

    ScDBData* pNow = findTestTable(m_pDoc);
    CPPUNIT_ASSERT(pNow);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("column resize must not auto-expand a row over the struct ref",
                                 ScRange(0, 0, 0, 6, 7, 0), getArea(*pNow));

    m_pDoc->DeleteTab(0);
}

// Auto-expansion: typing in the column immediately right of a styled named
// DBData grows the table by one column. Works regardless of Total Row.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoExpandColumnRight)
{
    m_pDoc->InsertTab(0, u"AutoExpandRight"_ustr);

    // Styled table at A1:C4 with both header and total. Adjacency col band = D1:D4.
    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 3,
                                       /*bHasHeader*/ true, /*bHasTotals*/ true);

    m_pDoc->SetString(ScAddress(3, 1, 0), u"new col"_ustr);

    CPPUNIT_ASSERT_MESSAGE("Notify should have flagged the col-right expansion",
                           pData->HasPendingExpansion());

    m_xDocShell->ProcessPendingTableExpansions();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("table should now reach column D", ScRange(0, 0, 0, 3, 3, 0),
                                 getArea(*pData));

    m_pDoc->DeleteTab(0);
}

// Total Row blocks row-down expansion but NOT column-right expansion.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoExpandTotalRowGate)
{
    m_pDoc->InsertTab(0, u"TotalRowGate"_ustr);

    // Styled table at A1:C4 with header AND total. Total Row is row 4.
    // Typing in row 5 (immediately below the total) must NOT expand.
    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 3,
                                       /*bHasHeader*/ true, /*bHasTotals*/ true);

    m_pDoc->SetString(ScAddress(0, 4, 0), u"after total"_ustr);
    m_xDocShell->ProcessPendingTableExpansions();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Total Row must block row-down expansion",
                                 ScRange(0, 0, 0, 2, 3, 0), getArea(*pData));

    // But column-right still expands in the same table.
    m_pDoc->SetString(ScAddress(3, 1, 0), u"col side"_ustr);
    m_xDocShell->ProcessPendingTableExpansions();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("column-right must expand even with Total Row",
                                 ScRange(0, 0, 0, 3, 3, 0), getArea(*pData));

    m_pDoc->DeleteTab(0);
}

// Corner cell (nEndCol+1, nEndRow+1) is in neither band — no expansion.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoExpandCornerExcluded)
{
    m_pDoc->InsertTab(0, u"CornerExcluded"_ustr);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 3,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);

    // Diagonal corner D5: outside both row band (A5:C5) and col band (D1:D4).
    m_pDoc->SetString(ScAddress(3, 4, 0), u"corner"_ustr);
    m_xDocShell->ProcessPendingTableExpansions();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("corner cell must not trigger expansion",
                                 ScRange(0, 0, 0, 2, 3, 0), getArea(*pData));
    CPPUNIT_ASSERT_MESSAGE("no pending flag for corner hits", !pData->HasPendingExpansion());

    m_pDoc->DeleteTab(0);
}

// Pre-existing content elsewhere in the band suppresses expansion when we
// fill another cell of the same band (the "first new cell" rule).
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoExpandBandOccupiedSuppresses)
{
    m_pDoc->InsertTab(0, u"BandOccupied"_ustr);

    // Pre-populate A6 *before* the DBData exists (so no listener was active
    // and A6 simply has unrelated prior content).
    m_pDoc->SetString(ScAddress(0, 5, 0), u"prior"_ustr);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 3, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);

    // Now type in B6 — but A6 already has content unrelated to the table.
    // Per MSO: no expansion.
    m_pDoc->SetString(ScAddress(1, 5, 0), u"user input"_ustr);
    m_xDocShell->ProcessPendingTableExpansions();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("prior occupancy must block expansion", ScRange(0, 0, 0, 3, 4, 0),
                                 getArea(*pData));

    m_pDoc->DeleteTab(0);
}

// Undo of an auto-expansion restores the prior area without touching the
// just-typed cell content (the content has its own separately-stackable
// undo entry).
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoExpandUndo)
{
    m_pDoc->InsertTab(0, u"AutoExpandUndo"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 3, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);

    m_pDoc->SetString(ScAddress(2, 5, 0), u"trigger"_ustr);
    m_xDocShell->ProcessPendingTableExpansions();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("expansion should have applied", ScRange(0, 0, 0, 3, 5, 0),
                                 getArea(*pData));

    // Undo only the expansion (top of undo stack).
    m_xDocShell->GetUndoManager()->Undo();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo should restore the prior area", ScRange(0, 0, 0, 3, 4, 0),
                                 getArea(*pData));

    // Redo brings it back.
    m_xDocShell->GetUndoManager()->Redo();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo should re-apply the expansion", ScRange(0, 0, 0, 3, 5, 0),
                                 getArea(*pData));

    m_pDoc->DeleteTab(0);
}

// Test column names are autogenerated for table with empty headers
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoHeaders)
{
    m_pDoc->InsertTab(0, u"InsertEmpty"_ustr);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    ScDBDocFunc(*m_xDocShell).FillEmptyHeaderColumnNames(*pData);

    CPPUNIT_ASSERT_EQUAL(u"Column1"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column2"_ustr, m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column3"_ustr, m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

// Insert table, extend it by one column, then disable the header.
// Expect every generated header (originals + the extended one) to be cleared.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testClearAutoHeaders)
{
    m_pDoc->InsertTab(0, u"ExtendDisable"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBDocFunc aFunc(*m_xDocShell);
    CPPUNIT_ASSERT(aFunc.AddDBTable(u"Table1"_ustr, ScRange(0, 0, 0, 2, 4, 0),
                                    /*bHeader*/ true, /*bRecord*/ true, /*bApi*/ true,
                                    u"TableStyleMedium2"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Column1"_ustr, m_pDoc->GetString(0, 0, 0));

    ScDBData* pData = m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
    CPPUNIT_ASSERT(pData);

    // Extend right by dragging the table wider (gridwin DBExpand -> ModifyDBData).
    {
        ScDBData aWide(*pData);
        aWide.SetArea(0, 0, 0, 3, 4);
        aFunc.ModifyDBData(aWide);
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("extended to D", ScRange(0, 0, 0, 3, 4, 0), getArea(*pData));
    CPPUNIT_ASSERT_EQUAL(u"Column1"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column4"_ustr, m_pDoc->GetString(3, 0, 0));

    // Disable the header row.
    ScDBData* pNow = m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
    CPPUNIT_ASSERT(pNow);
    ScDBData aOff(*pNow);
    aOff.SetHeader(false);
    aFunc.ModifyDBData(aOff);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A1 cleared", OUString(), m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("B1 cleared", OUString(), m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("C1 cleared", OUString(), m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("D1 (extended) cleared", OUString(), m_pDoc->GetString(3, 0, 0));

    // Re-enable the header: every name, including the extended column's, must
    // come back.
    ScDBData* pNow2 = m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
    CPPUNIT_ASSERT(pNow2);
    ScDBData aOn(*pNow2);
    aOn.SetHeader(true);
    aFunc.ModifyDBData(aOn);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A1 restored", u"Column1"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("B1 restored", u"Column2"_ustr, m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("C1 restored", u"Column3"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("D1 (extended) restored", u"Column4"_ustr,
                                 m_pDoc->GetString(3, 0, 0));

    m_pDoc->DeleteTab(0);
}

// Existing header text is preserved; only the empty header cells get filled,
// and the generated names avoid colliding with the user-entered ones.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoHeadersOnExistingColNames)
{
    m_pDoc->InsertTab(0, u"InsertPartial"_ustr);

    // User has typed a header into column A only; B and C are empty.
    m_pDoc->SetString(ScAddress(0, 0, 0), u"Name"_ustr);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    ScDBDocFunc(*m_xDocShell).FillEmptyHeaderColumnNames(*pData);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("user header must be untouched", u"Name"_ustr,
                                 m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column2"_ustr, m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column3"_ustr, m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoHeadersCaseInsensitiveCollision)
{
    m_pDoc->InsertTab(0, u"InsertCaseDup"_ustr);

    // User typed a lowercase "column2" into A; B and C are empty.
    m_pDoc->SetString(ScAddress(0, 0, 0), u"column2"_ustr);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    ScDBDocFunc(*m_xDocShell).FillEmptyHeaderColumnNames(*pData);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("user header must be untouched", u"column2"_ustr,
                                 m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("must skip case-insensitive duplicate of column2", u"Column3"_ustr,
                                 m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("next default avoids the just-used Column3", u"Column4"_ustr,
                                 m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

// A header-less table must not get any header cells written.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoHeadersOnHeaderDisabled)
{
    m_pDoc->InsertTab(0, u"InsertNoHeader"_ustr);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 4,
                                       /*bHasHeader*/ false, /*bHasTotals*/ false);
    ScDBDocFunc(*m_xDocShell).FillEmptyHeaderColumnNames(*pData);

    CPPUNIT_ASSERT_EQUAL(OUString(), m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString(), m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString(), m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

// A column-right auto-expansion fills the header cell of the newly-added
// column with a default name; pre-existing header cells are left alone.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoHeadersOnAutoExpansion)
{
    m_pDoc->InsertTab(0, u"ExpandColHeaderReal"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    m_pDoc->SetString(ScAddress(0, 0, 0), u"A"_ustr);
    m_pDoc->SetString(ScAddress(1, 0, 0), u"B"_ustr);
    m_pDoc->SetString(ScAddress(2, 0, 0), u"C"_ustr);

    // Type into D2 through the docfunc path; the settle drains the expansion.
    m_xDocShell->GetDocFunc().SetStringCell(ScAddress(3, 1, 0), u"new col"_ustr,
                                            /*bInteraction*/ false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("table should reach column D", ScRange(0, 0, 0, 3, 4, 0),
                                 getArea(*pData));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new column header D1 must get a default name", u"Column4"_ustr,
                                 m_pDoc->GetString(3, 0, 0));

    m_pDoc->DeleteTab(0);
}

// Dragging the table wider (ModifyDBData with a larger column range) fills the
// newly-covered columns' empty header cells; existing headers stay put.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoHeadersOnTableExpansion)
{
    m_pDoc->InsertTab(0, u"DragExtend"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    m_pDoc->SetString(ScAddress(0, 0, 0), u"A"_ustr);
    m_pDoc->SetString(ScAddress(1, 0, 0), u"B"_ustr);
    m_pDoc->SetString(ScAddress(2, 0, 0), u"C"_ustr);

    // Widen A1:C5 to A1:E5 the way the drag-resize gesture does.
    ScDBData aNewData(*pData);
    aNewData.SetArea(0, 0, 0, 4, 4);
    ScDBDocFunc(*m_xDocShell).ModifyDBData(aNewData);

    CPPUNIT_ASSERT_EQUAL(u"A"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"C"_ustr, m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("dragged-in column D header", u"Column4"_ustr,
                                 m_pDoc->GetString(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("dragged-in column E header", u"Column5"_ustr,
                                 m_pDoc->GetString(4, 0, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoHeadersOnTableShrink)
{
    m_pDoc->InsertTab(0, u"DragShrink"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 4, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    m_pDoc->SetString(ScAddress(0, 0, 0), u"A"_ustr);
    ScDBDocFunc aFunc(*m_xDocShell);
    aFunc.FillEmptyHeaderColumnNames(*pData);
    CPPUNIT_ASSERT_EQUAL(u"Column4"_ustr, m_pDoc->GetString(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column5"_ustr, m_pDoc->GetString(4, 0, 0));

    // The last column carries a user-entered header that must survive the shrink.
    m_pDoc->SetString(ScAddress(4, 0, 0), u"Kept"_ustr);

    // Narrow A1:E5 to A1:C5 the way the drag-resize gesture does.
    ScDBData aNewData(*pData);
    aNewData.SetArea(0, 0, 0, 2, 4);
    aFunc.ModifyDBData(aNewData);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("kept column A header", u"A"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("kept column C header", u"Column3"_ustr,
                                 m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("dropped column D, generated header cleared", OUString(),
                                 m_pDoc->GetString(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("dropped column E, user header kept", u"Kept"_ustr,
                                 m_pDoc->GetString(4, 0, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testExpandUndoRedoThenShrink)
{
    m_pDoc->InsertTab(0, u"ExpandRedoShrink"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    m_pDoc->SetString(ScAddress(0, 0, 0), u"A"_ustr);
    m_pDoc->SetString(ScAddress(1, 0, 0), u"B"_ustr);
    m_pDoc->SetString(ScAddress(2, 0, 0), u"C"_ustr);

    ScDBDocFunc aFunc(*m_xDocShell);

    // Widen A1:C5 to A1:E5; columns D,E get generated "Column4"/"Column5".
    ScDBData aWide(*pData);
    aWide.SetArea(0, 0, 0, 4, 4);
    aFunc.ModifyDBData(aWide);
    CPPUNIT_ASSERT_EQUAL(u"Column4"_ustr, m_pDoc->GetString(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column5"_ustr, m_pDoc->GetString(4, 0, 0));

    // Undo the expand, then Redo it. After Redo the generated-name tracking must
    // be intact again (this is the state the old code lost).
    m_xDocShell->GetUndoManager()->Undo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo drops column D", OUString(), m_pDoc->GetString(3, 0, 0));
    m_xDocShell->GetUndoManager()->Redo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo re-adds Column4", u"Column4"_ustr,
                                 m_pDoc->GetString(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo re-adds Column5", u"Column5"_ustr,
                                 m_pDoc->GetString(4, 0, 0));

    // Re-fetch: Undo/Redo replaced the ScDBData in the collection.
    ScDBData* pNow = m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TESTTABLE"_ustr);
    CPPUNIT_ASSERT(pNow);

    // Shrink A1:E5 back to A1:C5; the orphaned generated headers must clear.
    ScDBData aNarrow(*pNow);
    aNarrow.SetArea(0, 0, 0, 2, 4);
    aFunc.ModifyDBData(aNarrow);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("kept column A header", u"A"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("dropped column D generated header cleared", OUString(),
                                 m_pDoc->GetString(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("dropped column E generated header cleared", OUString(),
                                 m_pDoc->GetString(4, 0, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testResizeTotalsColumnsOnly)
{
    m_pDoc->InsertTab(0, u"ResizeTotalsCols"_ustr);
    m_pDoc->EnableUndo(true);

    // Table B2:D11, header row (row 2) + Total Row (row 11).
    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 1, 1, 3, 10,
                                       /*bHasHeader*/ true, /*bHasTotals*/ true);
    m_pDoc->SetString(ScAddress(1, 1, 0), u"A"_ustr);
    m_pDoc->SetString(ScAddress(2, 1, 0), u"B"_ustr);
    m_pDoc->SetString(ScAddress(3, 1, 0), u"C"_ustr);

    // Widen B2:D11 -> B2:F11 (rows unchanged).
    ScDBDocFunc(*m_xDocShell).ResizeTable(*pData, ScRange(1, 1, 0, 5, 10, 0));

    ScDBData* pNow = findTestTable(m_pDoc);
    CPPUNIT_ASSERT(pNow);
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 1, 0, 5, 10, 0), getArea(*pNow));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new column E header", u"Column4"_ustr,
                                 m_pDoc->GetString(4, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new column F header", u"Column5"_ustr,
                                 m_pDoc->GetString(5, 1, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testResizeMinimumRows)
{
    m_pDoc->InsertTab(0, u"ResizeMinRows"_ustr);
    m_pDoc->EnableUndo(true);

    // Table B2:D11, header row (row 2) + Total Row (row 11).
    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 1, 1, 3, 10,
                                       /*bHasHeader*/ true, /*bHasTotals*/ true);

    ScDBDocFunc aFunc(*m_xDocShell);

    // The header row on its own, and the header row with the Total Row right below it, both
    // leave no data row: refused, and the Table keeps its area.
    CPPUNIT_ASSERT(!aFunc.ResizeTable(*pData, ScRange(1, 1, 0, 3, 1, 0), /*bApi*/ true));
    CPPUNIT_ASSERT(!aFunc.ResizeTable(*pData, ScRange(1, 1, 0, 3, 2, 0), /*bApi*/ true));
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 1, 0, 3, 10, 0), getArea(*pData));

    // Header + one data row + the Total Row is the smallest a Total Row Table shrinks to.
    CPPUNIT_ASSERT(aFunc.ResizeTable(*pData, ScRange(1, 1, 0, 3, 3, 0), /*bApi*/ true));
    ScDBData* pNow = findTestTable(m_pDoc);
    CPPUNIT_ASSERT(pNow);
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 1, 0, 3, 3, 0), getArea(*pNow));

    // Without a Total Row the header and one data row are enough.
    ScDBData* pPlain
        = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 5, 1, 6, 10,
                           /*bHasHeader*/ true, /*bHasTotals*/ false, u"PlainTable"_ustr);
    CPPUNIT_ASSERT(!aFunc.ResizeTable(*pPlain, ScRange(5, 1, 0, 6, 1, 0), /*bApi*/ true));
    CPPUNIT_ASSERT(aFunc.ResizeTable(*pPlain, ScRange(5, 1, 0, 6, 2, 0), /*bApi*/ true));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testResizeTotalsRowsOnly)
{
    m_pDoc->InsertTab(0, u"ResizeTotalsRows"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 1, 1, 3, 10,
                                       /*bHasHeader*/ true, /*bHasTotals*/ true);

    // Grow B2:D11 -> B2:D27 (rows only): the Total Row moves to the new last row.
    ScDBDocFunc(*m_xDocShell).ResizeTable(*pData, ScRange(1, 1, 0, 3, 26, 0));
    ScDBData* pNow = findTestTable(m_pDoc);
    CPPUNIT_ASSERT(pNow);
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 1, 0, 3, 26, 0), getArea(*pNow));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testResizeMultiDimNoTotals)
{
    m_pDoc->InsertTab(0, u"ResizeMultiNoTotals"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 1, 1, 3, 10,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    m_pDoc->SetString(ScAddress(1, 1, 0), u"A"_ustr);
    m_pDoc->SetString(ScAddress(2, 1, 0), u"B"_ustr);
    m_pDoc->SetString(ScAddress(3, 1, 0), u"C"_ustr);

    // Grow B2:D11 -> B2:K26 (columns AND rows).
    ScDBDocFunc(*m_xDocShell).ResizeTable(*pData, ScRange(1, 1, 0, 10, 25, 0));
    ScDBData* pNow = findTestTable(m_pDoc);
    CPPUNIT_ASSERT(pNow);
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 1, 0, 10, 25, 0), getArea(*pNow));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new column E header", u"Column4"_ustr,
                                 m_pDoc->GetString(4, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new column K header", u"Column10"_ustr,
                                 m_pDoc->GetString(10, 1, 0));

    m_xDocShell->GetUndoManager()->Undo();
    pNow = findTestTable(m_pDoc);
    CPPUNIT_ASSERT(pNow);
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 1, 0, 3, 10, 0), getArea(*pNow));

    m_xDocShell->GetUndoManager()->Redo();
    pNow = findTestTable(m_pDoc);
    CPPUNIT_ASSERT(pNow);
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 1, 0, 10, 25, 0), getArea(*pNow));
    CPPUNIT_ASSERT_EQUAL(u"Column10"_ustr, m_pDoc->GetString(10, 1, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testResizeTotalsMultiDim)
{
    m_pDoc->InsertTab(0, u"ResizeTotalsMulti"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 1, 1, 3, 10,
                                       /*bHasHeader*/ true, /*bHasTotals*/ true);
    m_pDoc->SetString(ScAddress(1, 1, 0), u"A"_ustr);
    m_pDoc->SetString(ScAddress(2, 1, 0), u"B"_ustr);
    m_pDoc->SetString(ScAddress(3, 1, 0), u"C"_ustr);

    // Grow B2:D11 -> B2:K26 (columns AND rows) with a Total Row present.
    ScDBDocFunc(*m_xDocShell).ResizeTable(*pData, ScRange(1, 1, 0, 10, 25, 0));

    ScDBData* pNow = findTestTable(m_pDoc);
    CPPUNIT_ASSERT(pNow);
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 1, 0, 10, 25, 0), getArea(*pNow));

    // New columns must get generated headers, same as the no-totals case.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new column E header", u"Column4"_ustr,
                                 m_pDoc->GetString(4, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new column K header", u"Column10"_ustr,
                                 m_pDoc->GetString(10, 1, 0));

    // This table has autofilter OFF, so the resize must NOT add filter-button flags.
    CPPUNIT_ASSERT_MESSAGE(
        "no autofilter flag when the table has autofilter off",
        !bool(m_pDoc->GetAttr(10, 1, 0, ATTR_MERGE_FLAG).GetValue() & ScMF::Auto));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testResizeTotalsColumnShrink)
{
    m_pDoc->InsertTab(0, u"ResizeTotalsShrink"_ustr);
    m_pDoc->EnableUndo(true);

    // Total-Row table B2:D11 with headers A|B|C; grow it (columns only) to B2:K11 so E..K
    // get generated Column4..Column10 (routes through ModifyDBData, rows unchanged).
    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 1, 1, 3, 10,
                                       /*bHasHeader*/ true, /*bHasTotals*/ true);
    m_pDoc->SetString(ScAddress(1, 1, 0), u"A"_ustr);
    m_pDoc->SetString(ScAddress(2, 1, 0), u"B"_ustr);
    m_pDoc->SetString(ScAddress(3, 1, 0), u"C"_ustr);
    ScDBDocFunc(*m_xDocShell).ResizeTable(*pData, ScRange(1, 1, 0, 10, 10, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column6"_ustr, m_pDoc->GetString(6, 1, 0)); // G header generated

    // Shrink to B2:F27 (drop G..K) AND change the row span, so the totals branch runs and
    // must clear the dropped columns' generated names.
    ScDBData* pNow = findTestTable(m_pDoc);
    CPPUNIT_ASSERT(pNow);
    ScDBDocFunc(*m_xDocShell).ResizeTable(*pNow, ScRange(1, 1, 0, 5, 26, 0));

    pNow = findTestTable(m_pDoc);
    CPPUNIT_ASSERT(pNow);
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 1, 0, 5, 26, 0), getArea(*pNow));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("kept column F header", u"Column5"_ustr,
                                 m_pDoc->GetString(5, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("dropped column G generated header cleared", OUString(),
                                 m_pDoc->GetString(6, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("dropped column K generated header cleared", OUString(),
                                 m_pDoc->GetString(10, 1, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testResizeTotalsAutoFilterFlags)
{
    m_pDoc->InsertTab(0, u"ResizeTotalsAutoFilter"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 1, 1, 3, 10,
                                       /*bHasHeader*/ true, /*bHasTotals*/ true);
    pData->SetAutoFilter(true);
    m_pDoc->ApplyFlagsTab(1, 1, 3, 1, 0, ScMF::Auto); // initial filter buttons on header B2:D2

    // Grow B2:D11 -> B2:K26 (columns AND rows): the new header cells get the filter button.
    ScDBDocFunc(*m_xDocShell).ResizeTable(*pData, ScRange(1, 1, 0, 10, 25, 0));

    CPPUNIT_ASSERT_MESSAGE("new column E autofilter flag",
                           bool(m_pDoc->GetAttr(4, 1, 0, ATTR_MERGE_FLAG).GetValue() & ScMF::Auto));
    CPPUNIT_ASSERT_MESSAGE(
        "new column K autofilter flag",
        bool(m_pDoc->GetAttr(10, 1, 0, ATTR_MERGE_FLAG).GetValue() & ScMF::Auto));

    m_pDoc->DeleteTab(0);
}

// Removing a table clears the default header names it generated, but leaves
// user-entered headers (and any header it did not generate) in place.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoHeadersRemoveTable)
{
    m_pDoc->InsertTab(0, u"RemoveGenerated"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    // A has a real header; B and C will be auto-generated.
    m_pDoc->SetString(ScAddress(0, 0, 0), u"Name"_ustr);

    ScDBDocFunc aFunc(*m_xDocShell);
    aFunc.FillEmptyHeaderColumnNames(*pData);
    CPPUNIT_ASSERT_EQUAL(u"Column2"_ustr, m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column3"_ustr, m_pDoc->GetString(2, 0, 0));

    aFunc.DeleteDBTable(pData, /*bRecord*/ true, /*bApi*/ false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("user header stays", u"Name"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("generated B header removed", OUString(),
                                 m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("generated C header removed", OUString(),
                                 m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

// A header cell the user edited after generation must not be cleared on remove
CPPUNIT_TEST_FIXTURE(TableStylesTest, testUserHeaderRemoveTable)
{
    m_pDoc->InsertTab(0, u"RemoveKeep"_ustr);
    m_pDoc->EnableUndo(true);

    // Simulate an imported table whose headers happen to read "Column1"/"Column2"
    // but which we did NOT generate (no FillEmptyHeaderColumnNames call).
    m_pDoc->SetString(ScAddress(0, 0, 0), u"Column1"_ustr);
    m_pDoc->SetString(ScAddress(1, 0, 0), u"Column2"_ustr);
    m_pDoc->SetString(ScAddress(2, 0, 0), u"Column3"_ustr);
    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);

    ScDBDocFunc(*m_xDocShell).DeleteDBTable(pData, /*bRecord*/ true, /*bApi*/ false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("imported header A kept", u"Column1"_ustr,
                                 m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("imported header B kept", u"Column2"_ustr,
                                 m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("imported header C kept", u"Column3"_ustr,
                                 m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

// A header cell of a styled table (header on) must never be left empty:
// clearing a generated name restores that same name, and clearing a
// user-entered name restores it too (previous name kept while col count
// is unchanged).
CPPUNIT_TEST_FIXTURE(TableStylesTest, testHeadersAreNotEmpty)
{
    m_pDoc->InsertTab(0, u"ClearedHeader"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    m_pDoc->SetString(ScAddress(0, 0, 0), u"Sales"_ustr); // user header in A
    ScDBDocFunc aFunc(*m_xDocShell);
    aFunc.FillEmptyHeaderColumnNames(*pData); // B,C -> Column2,Column3
    CPPUNIT_ASSERT_EQUAL(u"Column2"_ustr, m_pDoc->GetString(1, 0, 0));

    // Clear the generated B header through the docfunc/settle path.
    ScMarkData aMarkB(m_pDoc->GetSheetLimits());
    aMarkB.SelectTable(0, true);
    aMarkB.SetMultiMarkArea(ScRange(1, 0, 0));
    m_xDocShell->GetDocFunc().DeleteContents(aMarkB, InsertDeleteFlags::CONTENTS, true, false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("cleared generated header is put back", u"Column2"_ustr,
                                 m_pDoc->GetString(1, 0, 0));

    // Clear the user header in A -> restored to "Sales", not a default.
    ScMarkData aMarkA(m_pDoc->GetSheetLimits());
    aMarkA.SelectTable(0, true);
    aMarkA.SetMultiMarkArea(ScRange(0, 0, 0));
    m_xDocShell->GetDocFunc().DeleteContents(aMarkA, InsertDeleteFlags::CONTENTS, true, false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("cleared user header is restored to its previous name",
                                 u"Sales"_ustr, m_pDoc->GetString(0, 0, 0));

    m_pDoc->DeleteTab(0);
}

// Turning off the header row clears the generated default names, but keeps
// user-entered headers.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testHeaderToggleDisable)
{
    m_pDoc->InsertTab(0, u"DisableHeader"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    m_pDoc->SetString(ScAddress(0, 0, 0), u"Name"_ustr); // real header in A

    ScDBDocFunc aFunc(*m_xDocShell);
    aFunc.FillEmptyHeaderColumnNames(*pData);
    CPPUNIT_ASSERT_EQUAL(u"Column2"_ustr, m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column3"_ustr, m_pDoc->GetString(2, 0, 0));

    // Turn the header row off via the same path the settings dialog uses.
    ScDBData aNewData(*pData);
    aNewData.SetHeader(false);
    aFunc.ModifyDBData(aNewData);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("user header A kept", u"Name"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("generated B cleared", OUString(), m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("generated C cleared", OUString(), m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

// Turning the header row back on regenerates default names for the now-empty
// header cells.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testHeaderToggleEnable)
{
    m_pDoc->InsertTab(0, u"ReenableHeader"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    ScDBDocFunc aFunc(*m_xDocShell);
    aFunc.FillEmptyHeaderColumnNames(*pData);

    ScDBData aOff(*pData);
    aOff.SetHeader(false);
    aFunc.ModifyDBData(aOff);
    CPPUNIT_ASSERT_EQUAL(OUString(), m_pDoc->GetString(0, 0, 0));

    // Re-enable: empty header cells get default names again.
    ScDBData* pNow = m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TESTTABLE"_ustr);
    CPPUNIT_ASSERT(pNow);
    ScDBData aOn(*pNow);
    aOn.SetHeader(true);
    aFunc.ModifyDBData(aOn);

    CPPUNIT_ASSERT_EQUAL(u"Column1"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column2"_ustr, m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column3"_ustr, m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

// A user header edited to duplicate is reverted to the user's previous value,
// not to an autogenerated name.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testDuplicateHeader)
{
    m_pDoc->InsertTab(0, u"DupUserEdit"_ustr);
    m_pDoc->EnableUndo(true);

    m_pDoc->SetString(ScAddress(0, 0, 0), u"Sales"_ustr);
    m_pDoc->SetString(ScAddress(1, 0, 0), u"Profit"_ustr);
    m_pDoc->SetString(ScAddress(2, 0, 0), u"Region"_ustr);
    ScDBDocFunc aFunc(*m_xDocShell);
    CPPUNIT_ASSERT(aFunc.AddDBTable(u"Table1"_ustr, ScRange(0, 0, 0, 2, 4, 0),
                                    /*bHeader*/ true, /*bRecord*/ true, /*bApi*/ true,
                                    u"TableStyleMedium2"_ustr));

    // Edit C1 ("Region") to duplicate A1 ("Sales") -> restored to "Region".
    m_xDocShell->GetDocFunc().SetStringCell(ScAddress(2, 0, 0), u"Sales"_ustr, false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("original header A kept", u"Sales"_ustr,
                                 m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("duplicate edit restored to previous", u"Region"_ustr,
                                 m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testNumericHeaderBecomesText)
{
    m_pDoc->InsertTab(0, u"NumHeader"_ustr);
    m_pDoc->EnableUndo(true);

    m_pDoc->SetString(ScAddress(0, 0, 0), u"Sales"_ustr);
    m_pDoc->SetString(ScAddress(1, 0, 0), u"Profit"_ustr);
    m_pDoc->SetString(ScAddress(2, 0, 0), u"Region"_ustr);
    ScDBDocFunc aFunc(*m_xDocShell);
    CPPUNIT_ASSERT(aFunc.AddDBTable(u"Table1"_ustr, ScRange(0, 0, 0, 2, 4, 0),
                                    /*bHeader*/ true, /*bRecord*/ true, /*bApi*/ true,
                                    u"TableStyleMedium2"_ustr));

    m_xDocShell->GetDocFunc().SetValueCell(ScAddress(2, 0, 0), 9999.0, false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("a number in a header is stored as text", CELLTYPE_STRING,
                                 m_pDoc->GetCellType(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"9999"_ustr, m_pDoc->GetString(2, 0, 0));

    const ScDBData* pData
        = m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
    CPPUNIT_ASSERT(pData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the column is named what the cell shows", u"9999"_ustr,
                                 pData->GetTableColumnNames()[2]);

    // A formula in a header cell goes the same way, it is a name too.
    m_xDocShell->GetDocFunc().SetFormulaCell(
        ScAddress(1, 0, 0), new ScFormulaCell(*m_pDoc, ScAddress(1, 0, 0), u"=1+11"_ustr), false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("a formula in a header is stored as text", CELLTYPE_STRING,
                                 m_pDoc->GetCellType(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"12"_ustr, m_pDoc->GetString(1, 0, 0));

    // Rich text in a header is text already, it must survive untouched.
    ScFieldEditEngine& rEE = m_pDoc->GetEditEngine();
    rEE.SetTextCurrentDefaults(u"Two\nLines"_ustr);
    m_xDocShell->GetDocFunc().SetEditCell(ScAddress(0, 0, 0), *rEE.CreateTextObject(), false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rich text in a header is not flattened", CELLTYPE_EDIT,
                                 m_pDoc->GetCellType(ScAddress(0, 0, 0)));

    // A data cell keeps its number and its formula.
    m_xDocShell->GetDocFunc().SetValueCell(ScAddress(2, 1, 0), 1234.0, false);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, m_pDoc->GetCellType(ScAddress(2, 1, 0)));
    m_xDocShell->GetDocFunc().SetFormulaCell(
        ScAddress(1, 1, 0), new ScFormulaCell(*m_pDoc, ScAddress(1, 1, 0), u"=1+11"_ustr), false);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(ScAddress(1, 1, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testNumericHeaderDuplicateReverts)
{
    m_pDoc->InsertTab(0, u"NumDup"_ustr);
    m_pDoc->EnableUndo(true);

    m_pDoc->SetString(ScAddress(0, 0, 0), u"12"_ustr);
    m_pDoc->SetString(ScAddress(1, 0, 0), u"Profit"_ustr);
    m_pDoc->SetString(ScAddress(2, 0, 0), u"Region"_ustr);
    ScDBDocFunc aFunc(*m_xDocShell);
    CPPUNIT_ASSERT(aFunc.AddDBTable(u"Table1"_ustr, ScRange(0, 0, 0, 2, 4, 0),
                                    /*bHeader*/ true, /*bRecord*/ true, /*bApi*/ true,
                                    u"TableStyleMedium2"_ustr));

    // 12 into C1 would read the same as A1, so C1 goes back to "Region".
    m_xDocShell->GetDocFunc().SetValueCell(ScAddress(2, 0, 0), 12.0, false);
    CPPUNIT_ASSERT_EQUAL(u"12"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("a duplicate is reverted, not turned into text", u"Region"_ustr,
                                 m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testCreateTableReusesDBRangeWithTotals)
{
    m_pDoc->InsertTab(0, u"Promote"_ustr);

    m_pDoc->SetString(ScAddress(0, 0, 0), u"Sales"_ustr);
    m_pDoc->SetString(ScAddress(1, 0, 0), u"Profit"_ustr);
    m_pDoc->SetString(ScAddress(0, 1, 0), u"d1"_ustr);
    m_pDoc->SetString(ScAddress(0, 4, 0), u"Total"_ustr);

    // A plain named range A1:B5 with a totals row and no table style.
    ScDBData* pPlain = new ScDBData(u"MyRange"_ustr, 0, 0, 0, 1, 4,
                                    /*bByRow*/ true, /*bHasHeader*/ true, /*bTotals*/ true);
    CPPUNIT_ASSERT(
        m_pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pPlain)));
    const size_t nBefore = m_pDoc->GetDBCollection()->getNamedDBs().size();

    ScDBDocFunc aFunc(*m_xDocShell);
    CPPUNIT_ASSERT(aFunc.AddDBTable(u"Table1"_ustr, ScRange(0, 0, 0, 1, 4, 0),
                                    /*bHeader*/ true, /*bRecord*/ true, /*bApi*/ true,
                                    u"TableStyleMedium2"_ustr));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("reused, not a second range", nBefore,
                                 m_pDoc->GetDBCollection()->getNamedDBs().size());

    // The very same range object was promoted: still named MyRange, now styled,
    // same extent, totals row intact.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the existing range was promoted", u"MyRange"_ustr,
                                 pPlain->GetName());
    CPPUNIT_ASSERT(pPlain->GetTableStyleInfo());
    ScRange aArea;
    pPlain->GetArea(aArea);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 1, 4, 0), aArea);
    CPPUNIT_ASSERT(pPlain->HasTotals());

    m_pDoc->DeleteTab(0);
}

// Creating a table whose header row holds a duplicate renames the duplicate in
// place (Excel-style "A | B | A" -> "A | B | A2"); no row is inserted and the
// data below is left exactly where it was.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testCreateTableDuplicateHeaders)
{
    m_pDoc->InsertTab(0, u"DupCreate"_ustr);
    m_pDoc->EnableUndo(true);

    // Row 1 has a duplicate ("A" twice); rows below are data.
    m_pDoc->SetString(ScAddress(0, 0, 0), u"A"_ustr);
    m_pDoc->SetString(ScAddress(1, 0, 0), u"B"_ustr);
    m_pDoc->SetString(ScAddress(2, 0, 0), u"A"_ustr);
    m_pDoc->SetString(ScAddress(0, 1, 0), u"d1"_ustr);

    ScDBDocFunc aFunc(*m_xDocShell);
    CPPUNIT_ASSERT(aFunc.AddDBTable(u"Table1"_ustr, ScRange(0, 0, 0, 2, 4, 0),
                                    /*bHeader*/ true, /*bRecord*/ true, /*bApi*/ true,
                                    u"TableStyleMedium2"_ustr));

    ScDBData* pData = m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
    CPPUNIT_ASSERT(pData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("table area unchanged, no row inserted", ScRange(0, 0, 0, 2, 4, 0),
                                 getArea(*pData));

    // First occurrence kept, the unique header kept, the duplicate disambiguated.
    CPPUNIT_ASSERT_EQUAL(u"A"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"B"_ustr, m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("duplicate header renamed in place", u"A2"_ustr,
                                 m_pDoc->GetString(2, 0, 0));

    // Data below is untouched (not shifted down).
    CPPUNIT_ASSERT_EQUAL(u"d1"_ustr, m_pDoc->GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString(), m_pDoc->GetString(0, 2, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testGeneratedNameReusedByUser)
{
    m_pDoc->InsertTab(0, u"DupReuse"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBDocFunc aFunc(*m_xDocShell);
    CPPUNIT_ASSERT(aFunc.AddDBTable(u"Table1"_ustr, ScRange(0, 0, 0, 2, 4, 0),
                                    /*bHeader*/ true, /*bRecord*/ true, /*bApi*/ true,
                                    u"TableStyleMedium2"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Column1"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column2"_ustr, m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column3"_ustr, m_pDoc->GetString(2, 0, 0));

    // Rename B -> Sales (frees the generated "Column2"), then type "Column2"
    // into C by hand (now a real user header, no live duplicate).
    m_xDocShell->GetDocFunc().SetStringCell(ScAddress(1, 0, 0), u"Sales"_ustr, false);
    m_xDocShell->GetDocFunc().SetStringCell(ScAddress(2, 0, 0), u"Column2"_ustr, false);
    CPPUNIT_ASSERT_EQUAL(u"Sales"_ustr, m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column2"_ustr, m_pDoc->GetString(2, 0, 0));

    ScDBData* pData = m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
    CPPUNIT_ASSERT(pData);
    aFunc.DeleteDBTable(pData, /*bRecord*/ true, /*bApi*/ false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("still-generated A1 cleared", OUString(),
                                 m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("user header B kept", u"Sales"_ustr, m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("user-typed C must survive remove", u"Column2"_ustr,
                                 m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testRemoveTableUndoRedoUndo)
{
    m_pDoc->InsertTab(0, u"RemoveUndoRedo"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBDocFunc aFunc(*m_xDocShell);
    CPPUNIT_ASSERT(aFunc.AddDBTable(u"Table1"_ustr, ScRange(0, 0, 0, 2, 4, 0),
                                    /*bHeader*/ true, /*bRecord*/ true, /*bApi*/ true,
                                    u"TableStyleMedium2"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Column1"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Column3"_ustr, m_pDoc->GetString(2, 0, 0));

    ScDBData* pData = m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
    CPPUNIT_ASSERT(pData);
    aFunc.DeleteDBTable(pData, /*bRecord*/ true, /*bApi*/ false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("removed: A1 cleared", OUString(), m_pDoc->GetString(0, 0, 0));

    // 1st Undo: table + generated names come back.
    m_xDocShell->GetUndoManager()->Undo();
    CPPUNIT_ASSERT_MESSAGE(
        "undo restores table",
        m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo restores A1", u"Column1"_ustr, m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo restores C1", u"Column3"_ustr, m_pDoc->GetString(2, 0, 0));

    // Redo: removed again.
    m_xDocShell->GetUndoManager()->Redo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo clears A1 again", OUString(), m_pDoc->GetString(0, 0, 0));

    // 2nd Undo: must behave exactly like the 1st Undo.
    m_xDocShell->GetUndoManager()->Undo();
    CPPUNIT_ASSERT_MESSAGE(
        "2nd undo restores table",
        m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("2nd undo must restore A1", u"Column1"_ustr,
                                 m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("2nd undo must restore C1", u"Column3"_ustr,
                                 m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testCreateUndoRedoThenRemove)
{
    m_pDoc->InsertTab(0, u"CreateUndoRedo"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBDocFunc aFunc(*m_xDocShell);
    CPPUNIT_ASSERT(aFunc.AddDBTable(u"Table1"_ustr, ScRange(0, 0, 0, 2, 4, 0),
                                    /*bHeader*/ true, /*bRecord*/ true, /*bApi*/ true,
                                    u"TableStyleMedium2"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Column1"_ustr, m_pDoc->GetString(0, 0, 0));

    // Undo: table gone, headers cleared.
    m_xDocShell->GetUndoManager()->Undo();
    CPPUNIT_ASSERT_MESSAGE(
        "undo removes table",
        !m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo clears A1", OUString(), m_pDoc->GetString(0, 0, 0));

    // Redo: table back, headers back.
    m_xDocShell->GetUndoManager()->Redo();
    ScDBData* pData = m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
    CPPUNIT_ASSERT_MESSAGE("redo restores table", pData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo restores A1", u"Column1"_ustr, m_pDoc->GetString(0, 0, 0));

    // Remove: the generated headers must be cleared, not left behind.
    aFunc.DeleteDBTable(pData, /*bRecord*/ true, /*bApi*/ false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("generated A1 cleared after remove", OUString(),
                                 m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("generated B1 cleared after remove", OUString(),
                                 m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("generated C1 cleared after remove", OUString(),
                                 m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

// Helper: is the AutoFilter dropdown flag (ScMF::Auto) set on the given cell?
namespace
{
bool cellHasAutoFilterFlag(const ScDocument& rDoc, SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    const ScPatternAttr* pPattern = rDoc.GetPattern(nCol, nRow, nTab);
    return pPattern && pPattern->GetItem(ATTR_MERGE_FLAG).HasAutoFilter();
}
} // anonymous namespace

// Column-right expansion on a styled table with AutoFilter on must propagate
// ScMF::Auto to the new header cell (so the dropdown button renders). Undo
// removes it; Redo brings it back.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoExpandAutoFilterFlag)
{
    m_pDoc->InsertTab(0, u"AutoExpandAutoFilter"_ustr);
    m_pDoc->EnableUndo(true);

    // A1:C5, header, no total.
    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 2, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    pData->SetAutoFilter(true);
    // Seed the initial AutoFilter dropdown flag on the header row (this is
    // what SetAutoFilter does in the real flow via ApplyFlagsTab).
    m_pDoc->ApplyFlagsTab(0, 0, 2, 0, 0, ScMF::Auto);

    CPPUNIT_ASSERT(cellHasAutoFilterFlag(*m_pDoc, 0, 0, 0));
    CPPUNIT_ASSERT(cellHasAutoFilterFlag(*m_pDoc, 2, 0, 0));
    CPPUNIT_ASSERT(!cellHasAutoFilterFlag(*m_pDoc, 3, 0, 0)); // not in table yet

    // User types in the column band: D2.
    m_pDoc->SetString(ScAddress(3, 1, 0), u"new col"_ustr);
    m_xDocShell->ProcessPendingTableExpansions();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("table expands to column D", ScRange(0, 0, 0, 3, 4, 0),
                                 getArea(*pData));
    CPPUNIT_ASSERT_MESSAGE("new header cell D1 must have AutoFilter dropdown flag",
                           cellHasAutoFilterFlag(*m_pDoc, 3, 0, 0));

    // Undo the expansion — the new header cell loses its flag.
    m_xDocShell->GetUndoManager()->Undo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo restores prior area", ScRange(0, 0, 0, 2, 4, 0),
                                 getArea(*pData));
    CPPUNIT_ASSERT_MESSAGE("D1 must lose AutoFilter flag after undo",
                           !cellHasAutoFilterFlag(*m_pDoc, 3, 0, 0));
    CPPUNIT_ASSERT_MESSAGE("A1 keeps AutoFilter flag", cellHasAutoFilterFlag(*m_pDoc, 0, 0, 0));

    // Redo brings the flag back on the new header cell.
    m_xDocShell->GetUndoManager()->Redo();
    CPPUNIT_ASSERT_MESSAGE("D1 regains AutoFilter flag after redo",
                           cellHasAutoFilterFlag(*m_pDoc, 3, 0, 0));

    m_pDoc->DeleteTab(0);
}

// Two-step undo/redo: typing via ScDocFunc creates content +
// expansion undo entries.
//   Undo #1: expansion reverts (table shrinks, content stays)
//   Undo #2: content reverts (cell empty, table at old area)
//   Redo #1: content re-applies (cell back, table stays at old area)
//   Redo #2: expansion re-applies (table grows again)
// Redo #1 must NOT re-expand — the drain skips during undo/redo replay.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testAutoExpandTwoStepUndoRedo)
{
    m_pDoc->InsertTab(0, u"TwoStepUndo"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 3, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);

    // Type through the docfunc path so a real ScUndoEnterData entry is
    // pushed. The drain then pushes ScUndoExpandTableArea on top.
    const ScAddress aTrigger(1, 5, 0); // B6, inside the row band A6:D6
    m_xDocShell->GetDocFunc().SetStringCell(aTrigger, u"hello"_ustr, /*bInteraction*/ false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("table expanded after typing", ScRange(0, 0, 0, 3, 5, 0),
                                 getArea(*pData));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("cell B6 has the typed value", u"hello"_ustr,
                                 m_pDoc->GetString(aTrigger));

    // Undo #1 — only the expansion reverts.
    m_xDocShell->GetUndoManager()->Undo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo 1: area shrinks", ScRange(0, 0, 0, 3, 4, 0),
                                 getArea(*pData));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo 1: content remains", u"hello"_ustr,
                                 m_pDoc->GetString(aTrigger));

    // Undo #2 — content reverts.
    m_xDocShell->GetUndoManager()->Undo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo 2: area still at original", ScRange(0, 0, 0, 3, 4, 0),
                                 getArea(*pData));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo 2: cell empty", OUString(), m_pDoc->GetString(aTrigger));

    // Redo #1 — content reapplies. The morning fix prevents this from
    // *also* re-expanding the table as a side effect of the redo replay.
    m_xDocShell->GetUndoManager()->Redo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo 1: cell has content again", u"hello"_ustr,
                                 m_pDoc->GetString(aTrigger));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo 1: table MUST still be at old area",
                                 ScRange(0, 0, 0, 3, 4, 0), getArea(*pData));
    CPPUNIT_ASSERT_MESSAGE("redo 1: no pending expansion flag should linger",
                           !pData->HasPendingExpansion());

    // Redo #2 — expansion reapplies.
    m_xDocShell->GetUndoManager()->Redo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo 2: area expanded again", ScRange(0, 0, 0, 3, 5, 0),
                                 getArea(*pData));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo 2: cell still has content", u"hello"_ustr,
                                 m_pDoc->GetString(aTrigger));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testTableStyleBorderEmptyCellOverride)
{
    // GIVEN a TableStyleMedium2 range with thin wholeTable borders, and
    // a data cell inside it carrying an explicit but all-empty
    // SvxBoxItem (as xlsx import plants for borderId=0 + applyBorder=1).
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    ScDBData* pDBData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pDBData);

    constexpr SCCOL nCol = 1;
    constexpr SCROW nRow = 5;
    constexpr SCTAB nTab = 0;
    SvxBoxItem aEmptyBox(ATTR_BORDER);
    m_pDoc->ApplyAttr(nCol, nRow, nTab, aEmptyBox);

    // WHEN ScDocument::FillInfo runs over the table.
    ScTableInfo aTabInfo(0, 10, false);
    m_pDoc->FillInfo(aTabInfo, 0, 0, 3, 10, nTab, 1, 1, false, false);

    // THEN the resulting ScCellInfo carries the table style's
    // wholeTable border, not the empty cell border.
    // mpRowInfo[0] is the pre-row, [1] is the first content row, etc.
    const ScCellInfo* pCellInfo = &aTabInfo.mpRowInfo[nRow + 1].cellInfo(nCol);
    const SvxBoxItem* pBox = static_cast<const SvxBoxItem*>(pCellInfo->maLinesAttr.getItem());
    CPPUNIT_ASSERT_MESSAGE("FillInfo must produce a border for table-style cells", pBox);
    CPPUNIT_ASSERT_MESSAGE("table-style wholeTable border must not be masked by empty cell border",
                           pBox->GetTop() || pBox->GetBottom() || pBox->GetLeft()
                               || pBox->GetRight());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testRenameTableUpdatesStructuredRefs)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"RenameTable"_ustr);
    m_pDoc->EnableUndo(true);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 1, 3,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    CPPUNIT_ASSERT(pData);
    m_pDoc->SetString(ScAddress(0, 0, 0), u"Header1"_ustr);
    m_pDoc->SetString(ScAddress(1, 0, 0), u"Header2"_ustr);
    m_pDoc->SetString(ScAddress(0, 1, 0), u"1"_ustr);
    m_pDoc->SetString(ScAddress(0, 2, 0), u"4"_ustr);
    m_pDoc->SetString(ScAddress(0, 3, 0), u"16"_ustr);

    m_pDoc->SetString(ScAddress(3, 0, 0), u"=SUM(TestTable[Header1])"_ustr);
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(ScAddress(3, 0, 0)));

    CPPUNIT_ASSERT(ScDBDocFunc(*m_xDocShell).RenameDBRange(u"TestTable"_ustr, u"Renamed"_ustr));

    // The structured reference must follow the rename.
    CPPUNIT_ASSERT(m_pDoc->GetFormula(3, 0, 0).indexOf(u"Renamed[") >= 0);
    CPPUNIT_ASSERT(m_pDoc->GetFormula(3, 0, 0).indexOf(u"TestTable") < 0);
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(ScAddress(3, 0, 0)));

    m_xDocShell->GetUndoManager()->Undo();
    CPPUNIT_ASSERT(m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"TESTTABLE"_ustr));
    CPPUNIT_ASSERT(m_pDoc->GetFormula(3, 0, 0).indexOf(u"TestTable[") >= 0);
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(ScAddress(3, 0, 0)));

    m_xDocShell->GetUndoManager()->Redo();
    CPPUNIT_ASSERT(m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"RENAMED"_ustr));
    CPPUNIT_ASSERT(m_pDoc->GetFormula(3, 0, 0).indexOf(u"Renamed[") >= 0);
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(ScAddress(3, 0, 0)));

    // A case-only rename ("Renamed" -> "RENAMED") is allowed, and the structured reference
    // re-renders with the new case.
    CPPUNIT_ASSERT(ScDBDocFunc(*m_xDocShell).RenameDBRange(u"Renamed"_ustr, u"RENAMED"_ustr));
    ScDBData* pCased = m_pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"RENAMED"_ustr);
    CPPUNIT_ASSERT(pCased);
    CPPUNIT_ASSERT_EQUAL(u"RENAMED"_ustr, pCased->GetName());
    CPPUNIT_ASSERT(m_pDoc->GetFormula(3, 0, 0).indexOf(u"RENAMED[") >= 0);
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(ScAddress(3, 0, 0)));

    m_pDoc->DeleteTab(0);
}

// The gallery groups styles Light, then Medium, then Dark, and orders the
// trailing number numerically so the list reads 1, 2, ... 10 rather than the
// lexical 1, 10, 2.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testGetSortedTableStyles)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Sorted"_ustr);

    auto pColorSet = createTestThemeA();
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    ScTableStyles* pStyles = m_pDoc->GetTableStyles();
    CPPUNIT_ASSERT(pStyles);

    std::vector<const ScTableStyle*> aSorted = pStyles->GetSortedTableStyles();
    CPPUNIT_ASSERT(!aSorted.empty());

    auto indexOf = [&aSorted](std::u16string_view aName) -> sal_Int32 {
        for (size_t i = 0; i < aSorted.size(); ++i)
            if (aSorted[i]->GetName() == aName)
                return static_cast<sal_Int32>(i);
        return -1;
    };

    const sal_Int32 nLight1 = indexOf(u"TableStyleLight1");
    const sal_Int32 nLight2 = indexOf(u"TableStyleLight2");
    const sal_Int32 nLight10 = indexOf(u"TableStyleLight10");
    const sal_Int32 nLight21 = indexOf(u"TableStyleLight21");
    const sal_Int32 nMedium1 = indexOf(u"TableStyleMedium1");
    const sal_Int32 nDark1 = indexOf(u"TableStyleDark1");
    CPPUNIT_ASSERT(nLight1 >= 0);
    CPPUNIT_ASSERT(nLight2 >= 0);
    CPPUNIT_ASSERT(nLight10 >= 0);
    CPPUNIT_ASSERT(nLight21 >= 0);
    CPPUNIT_ASSERT(nMedium1 >= 0);
    CPPUNIT_ASSERT(nDark1 >= 0);

    CPPUNIT_ASSERT_MESSAGE("Light 1 precedes Light 2", nLight1 < nLight2);
    CPPUNIT_ASSERT_MESSAGE("Light 2 precedes Light 10 (numeric, not lexical)", nLight2 < nLight10);
    CPPUNIT_ASSERT_MESSAGE("Light precedes Medium", nLight21 < nMedium1);
    CPPUNIT_ASSERT_MESSAGE("Medium precedes Dark", nMedium1 < nDark1);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testRenameTableDuplicateRefused)
{
    m_pDoc->InsertTab(0, u"RenameDup"_ustr);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 1, 3,
                                       /*bHasHeader*/ true, /*bHasTotals*/ false);
    CPPUNIT_ASSERT(pData);

    // Renaming into a *different* existing table's name is refused.
    CPPUNIT_ASSERT(m_pDoc->GetDBCollection()->getNamedDBs().insert(
        std::unique_ptr<ScDBData>(new ScDBData(u"Other"_ustr, 0, 3, 0, 4, 3, true, true, false))));
    CPPUNIT_ASSERT(!ScDBDocFunc(*m_xDocShell).RenameDBRange(u"TestTable"_ustr, u"Other"_ustr));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testConvertToRangeBakePreservesLayers)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Bake"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);

    // A1:D11, header + total + row stripes + first/last column.
    ScDBData aDBData(u"T"_ustr, 0, 0, 0, 3, 10, true, true, true);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    aStyleParam.mbRowStripes = true;
    aStyleParam.mbColumnStripes = false;
    aStyleParam.mbFirstColumn = true;
    aStyleParam.mbLastColumn = true;
    aDBData.SetTableStyleInfo(aStyleParam);

    // Direct attributes that must survive: a background (fill: direct beats the style) and a
    // non-black font colour (font: direct beats the style).
    const Color aDirectBg(0x123456);
    m_pDoc->ApplyAttr(1, 3, 0, SvxBrushItem(aDirectBg, ATTR_BACKGROUND));
    const Color aDirectFontColor(0x654321);
    m_pDoc->ApplyAttr(2, 4, 0, SvxColorItem(aDirectFontColor, ATTR_FONT_COLOR));

    pStyle->BakeInto(*m_pDoc, aDBData);

    // The header is solid in Medium2: it must be baked as a real cell background.
    const SvxBrushItem* pHeaderFill = pStyle->GetFillItem(aDBData, 1, 0, -1);
    CPPUNIT_ASSERT(pHeaderFill);
    CPPUNIT_ASSERT_EQUAL(pHeaderFill->GetColor(),
                         m_pDoc->GetAttr(1, 0, 0, ATTR_BACKGROUND).GetColor());

    // Elsewhere, wherever the style provides a fill, the baked cell background matches it
    // (rowIndex mirrors fillinfo: header = -1, first data row = 0, ...). Empty stripes have no
    // style fill, so those cells stay default - nothing to assert there.
    auto checkFill = [&](SCCOL nCol, SCROW nRow) {
        if (const SvxBrushItem* pStyleFill = pStyle->GetFillItem(aDBData, nCol, nRow, nRow - 1))
            CPPUNIT_ASSERT_EQUAL(pStyleFill->GetColor(),
                                 m_pDoc->GetAttr(nCol, nRow, 0, ATTR_BACKGROUND).GetColor());
    };
    checkFill(1, 2); // data stripe
    checkFill(1, 10); // total row
    checkFill(0, 2); // first column
    checkFill(3, 2); // last column

    // Direct background survives untouched.
    CPPUNIT_ASSERT_EQUAL(aDirectBg, m_pDoc->GetAttr(1, 3, 0, ATTR_BACKGROUND).GetColor());
    // Direct (non-black) font colour survives untouched.
    CPPUNIT_ASSERT_EQUAL(aDirectFontColor, m_pDoc->GetAttr(2, 4, 0, ATTR_FONT_COLOR).getColor());

    // The style's header font weight is baked onto a gap header cell (font beats the cell).
    const SfxItemSet* pHeaderFont = pStyle->GetFontItemSet(aDBData, 1, 0, -1);
    CPPUNIT_ASSERT(pHeaderFont);
    if (const SvxWeightItem* pStyleWeight = pHeaderFont->GetItemIfSet(ATTR_FONT_WEIGHT, false))
        CPPUNIT_ASSERT_EQUAL(pStyleWeight->GetWeight(),
                             m_pDoc->GetAttr(1, 0, 0, ATTR_FONT_WEIGHT).GetWeight());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testConvertToRangeUndoRedo)
{
    m_pDoc->EnableUndo(true);
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Conv"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 3, 5,
                                       /*bHasHeader*/ true, /*bHasTotals*/ true);
    const ScTableStyle* pStyle = m_pDoc->GetTableStyles()->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pStyle);

    // The header fill that must appear as a real cell attribute after conversion.
    const SvxBrushItem* pHeaderFill = pStyle->GetFillItem(*pData, 1, 0, -1);
    CPPUNIT_ASSERT(pHeaderFill);
    const Color aHeaderColor = pHeaderFill->GetColor();

    CPPUNIT_ASSERT(ScDBDocFunc(*m_xDocShell).ConvertTableToRange(pData));

    CPPUNIT_ASSERT(!m_pDoc->GetDBCollection()->getNamedDBs().findByName(u"TestTable"_ustr));
    CPPUNIT_ASSERT_EQUAL(aHeaderColor, m_pDoc->GetAttr(1, 0, 0, ATTR_BACKGROUND).GetColor());

    m_pDoc->GetUndoManager()->Undo();
    CPPUNIT_ASSERT(m_pDoc->GetDBCollection()->getNamedDBs().findByName(u"TestTable"_ustr));
    CPPUNIT_ASSERT(m_pDoc->GetAttr(1, 0, 0, ATTR_BACKGROUND).GetColor() != aHeaderColor);

    m_pDoc->GetUndoManager()->Redo();
    CPPUNIT_ASSERT(!m_pDoc->GetDBCollection()->getNamedDBs().findByName(u"TestTable"_ustr));
    CPPUNIT_ASSERT_EQUAL(aHeaderColor, m_pDoc->GetAttr(1, 0, 0, ATTR_BACKGROUND).GetColor());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testConvertToRangeFlattensStructuredRefs)
{
    m_pDoc->EnableUndo(true);
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Conv"_ustr);

    auto pColorSet = createTestThemeA();
    applyThemeToDocument(m_pDoc, pColorSet);
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    // Table A1:B5 - header row 0, data rows 1..3, totals row 4; column B is "Val".
    ScDBData* pData = createTestDBData(m_pDoc, u"TableStyleMedium2"_ustr, 0, 0, 1, 4,
                                       /*bHasHeader*/ true, /*bHasTotals*/ true);
    m_pDoc->SetString(0, 0, 0, u"Name"_ustr);
    m_pDoc->SetString(1, 0, 0, u"Val"_ustr);
    m_pDoc->SetValue(1, 1, 0, 10.0);
    m_pDoc->SetValue(1, 2, 0, 20.0);
    m_pDoc->SetValue(1, 3, 0, 30.0);
    pData->RefreshTableColumnNames(m_pDoc);

    // A scalar structured reference (D1) and a multi-cell array formula over D2:D4
    // ({=TestTable[Val]*2} -> {20, 40, 60}). If Convert-to-Range dropped the array-ness, D2:D4
    // would collapse to a single value, so distinct per-row values prove the array survived.
    m_pDoc->SetString(3, 0, 0, u"=SUM(TestTable[Val])"_ustr); // D1
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectTable(0, true);
    m_pDoc->InsertMatrixFormula(3, 1, 3, 3, aMark, u"=TestTable[Val]*2"_ustr); // D2:D4
    // A defined name (global) that also references the column via a structured reference. Its
    // value stays cached even when the stored code dangles, so we check the code (symbol) itself.
    m_pDoc->GetRangeName().insert(std::make_unique<ScRangeData>(
        *m_pDoc, u"ValTotal"_ustr, u"SUM(TestTable[Val])"_ustr, ScAddress(0, 0, 0)));
    auto bNameHasTableRef = [this] {
        const ScRangeData* p = m_pDoc->GetRangeName().findByUpperName(u"VALTOTAL"_ustr);
        return p && p->GetSymbol().indexOf(u"TestTable") >= 0;
    };
    m_pDoc->CalcAll();
    CPPUNIT_ASSERT_EQUAL(60.0, m_pDoc->GetValue(3, 0, 0));
    CPPUNIT_ASSERT(m_pDoc->GetFormula(3, 0, 0).indexOf(u"TestTable") >= 0);
    CPPUNIT_ASSERT_EQUAL(20.0, m_pDoc->GetValue(3, 1, 0));
    CPPUNIT_ASSERT_EQUAL(40.0, m_pDoc->GetValue(3, 2, 0));
    CPPUNIT_ASSERT_EQUAL(60.0, m_pDoc->GetValue(3, 3, 0));
    CPPUNIT_ASSERT(bNameHasTableRef());

    CPPUNIT_ASSERT(ScDBDocFunc(*m_xDocShell).ConvertTableToRange(pData));
    m_pDoc->CalcAll();

    // Table gone; both formulas are flattened to plain ranges with values intact (no #REF!), and
    // the array stays an array (D2:D4 keep their distinct values).
    CPPUNIT_ASSERT(!m_pDoc->GetDBCollection()->getNamedDBs().findByName(u"TestTable"_ustr));
    const OUString aFlat = m_pDoc->GetFormula(3, 0, 0);
    CPPUNIT_ASSERT(aFlat.indexOf(u"TestTable") < 0);
    CPPUNIT_ASSERT(aFlat.indexOf(u"$B$2:$B$4") >= 0);
    CPPUNIT_ASSERT_EQUAL(60.0, m_pDoc->GetValue(3, 0, 0));
    CPPUNIT_ASSERT(m_pDoc->GetFormula(3, 1, 0).indexOf(u"TestTable") < 0);
    CPPUNIT_ASSERT_EQUAL(20.0, m_pDoc->GetValue(3, 1, 0));
    CPPUNIT_ASSERT_EQUAL(40.0, m_pDoc->GetValue(3, 2, 0));
    CPPUNIT_ASSERT_EQUAL(60.0, m_pDoc->GetValue(3, 3, 0));
    CPPUNIT_ASSERT(!bNameHasTableRef()); // the defined name's stored code is flattened too

    // Undo restores the structured references and the values (array included).
    m_pDoc->GetUndoManager()->Undo();
    m_pDoc->CalcAll();
    CPPUNIT_ASSERT(m_pDoc->GetDBCollection()->getNamedDBs().findByName(u"TestTable"_ustr));
    CPPUNIT_ASSERT(m_pDoc->GetFormula(3, 0, 0).indexOf(u"TestTable") >= 0);
    CPPUNIT_ASSERT_EQUAL(60.0, m_pDoc->GetValue(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(20.0, m_pDoc->GetValue(3, 1, 0));
    CPPUNIT_ASSERT_EQUAL(40.0, m_pDoc->GetValue(3, 2, 0));
    CPPUNIT_ASSERT_EQUAL(60.0, m_pDoc->GetValue(3, 3, 0));
    CPPUNIT_ASSERT(bNameHasTableRef()); // the defined name is structured again

    // Redo flattens again.
    m_pDoc->GetUndoManager()->Redo();
    m_pDoc->CalcAll();
    CPPUNIT_ASSERT(!m_pDoc->GetDBCollection()->getNamedDBs().findByName(u"TestTable"_ustr));
    CPPUNIT_ASSERT(m_pDoc->GetFormula(3, 0, 0).indexOf(u"TestTable") < 0);
    CPPUNIT_ASSERT_EQUAL(60.0, m_pDoc->GetValue(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(20.0, m_pDoc->GetValue(3, 1, 0));
    CPPUNIT_ASSERT_EQUAL(40.0, m_pDoc->GetValue(3, 2, 0));
    CPPUNIT_ASSERT_EQUAL(60.0, m_pDoc->GetValue(3, 3, 0));
    CPPUNIT_ASSERT(!bNameHasTableRef());

    m_pDoc->DeleteTab(0);
}

// The document carries a default table style for newly inserted tables. New
// documents start at the built-in Medium 2, and the choice is remembered.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testDefaultTableStyleName)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Default"_ustr);

    auto pColorSet = createTestThemeA();
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);

    ScTableStyles* pStyles = m_pDoc->GetTableStyles();
    CPPUNIT_ASSERT(pStyles);

    CPPUNIT_ASSERT_EQUAL(u"TableStyleMedium2"_ustr, pStyles->GetDefaultStyleName());

    pStyles->SetDefaultStyleName(u"TableStyleLight9"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"TableStyleLight9"_ustr, pStyles->GetDefaultStyleName());

    m_pDoc->DeleteTab(0);
}

namespace
{
// The background colour a style sets on one of its regions, or COL_AUTO when that
// region carries no fill.
Color regionBackground(const ScTableStyle& rStyle, ScTableStyleElement eElement)
{
    const std::map<ScTableStyleElement, const ScPatternAttr*> aPatterns = rStyle.GetSetPatterns();
    const auto it = aPatterns.find(eElement);
    if (it == aPatterns.end())
        return COL_AUTO;
    const SvxBrushItem* pBrush = it->second->GetItemSet().GetItemIfSet(ATTR_BACKGROUND, false);
    return pBrush ? pBrush->GetColor() : COL_AUTO;
}
}

// Copying a style makes a Custom style that carries the formatting of the style it
// was copied from, under the name it was given.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testDuplicateTableStyleCopiesFormatting)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Duplicate"_ustr);

    auto pColorSet = createTestThemeA();
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);
    ScTableStyles* pStyles = m_pDoc->GetTableStyles();
    CPPUNIT_ASSERT(pStyles);

    const ScTableStyle* pSource = pStyles->GetTableStyle(u"TableStyleMedium2"_ustr);
    CPPUNIT_ASSERT(pSource);
    const Color aSourceHeader = regionBackground(*pSource, ScTableStyleElement::HeaderRow);

    const OUString aNewName
        = pStyles->DuplicateTableStyle(u"TableStyleMedium2"_ustr, u"Copy of Medium 2"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"TableStyleCustom1"_ustr, aNewName);

    const ScTableStyle* pCopy = pStyles->GetTableStyle(aNewName);
    CPPUNIT_ASSERT(pCopy);
    CPPUNIT_ASSERT_EQUAL(u"Copy of Medium 2"_ustr, pCopy->GetUIName());
    CPPUNIT_ASSERT_EQUAL(aSourceHeader, regionBackground(*pCopy, ScTableStyleElement::HeaderRow));
    // The copy is the user's own style, so it is not one of the generated built-ins
    // and is grouped as Custom.
    CPPUNIT_ASSERT(!pCopy->IsOOXMLDefault());

    m_pDoc->DeleteTab(0);
}

// Copying a name no style is registered under changes nothing.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testDuplicateTableStyleUnknownSource)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"DuplicateMissing"_ustr);

    auto pColorSet = createTestThemeA();
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);
    ScTableStyles* pStyles = m_pDoc->GetTableStyles();
    CPPUNIT_ASSERT(pStyles);

    const size_t nBefore = pStyles->GetSortedTableStyles().size();
    const OUString aNewName
        = pStyles->DuplicateTableStyle(u"NoSuchStyle"_ustr, u"Copy of nothing"_ustr);

    CPPUNIT_ASSERT(aNewName.isEmpty());
    CPPUNIT_ASSERT_EQUAL(nBefore, pStyles->GetSortedTableStyles().size());
    CPPUNIT_ASSERT(!pStyles->GetTableStyle(u"TableStyleCustom1"_ustr));

    m_pDoc->DeleteTab(0);
}

// Each copy gets a name of its own, and copying a copy is allowed.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testDuplicateTableStyleNamesEachCopy)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"DuplicateTwice"_ustr);

    auto pColorSet = createTestThemeA();
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);
    ScTableStyles* pStyles = m_pDoc->GetTableStyles();
    CPPUNIT_ASSERT(pStyles);

    CPPUNIT_ASSERT_EQUAL(
        u"TableStyleCustom1"_ustr,
        pStyles->DuplicateTableStyle(u"TableStyleMedium2"_ustr, u"First"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        u"TableStyleCustom2"_ustr,
        pStyles->DuplicateTableStyle(u"TableStyleMedium2"_ustr, u"Second"_ustr));
    // A copy can itself be copied.
    CPPUNIT_ASSERT_EQUAL(
        u"TableStyleCustom3"_ustr,
        pStyles->DuplicateTableStyle(u"TableStyleCustom1"_ustr, u"Third"_ustr));

    CPPUNIT_ASSERT_EQUAL(u"First"_ustr,
                         pStyles->GetTableStyle(u"TableStyleCustom1"_ustr)->GetUIName());
    CPPUNIT_ASSERT_EQUAL(u"Third"_ustr,
                         pStyles->GetTableStyle(u"TableStyleCustom3"_ustr)->GetUIName());

    m_pDoc->DeleteTab(0);
}

// A copy holds its own formatting, so changing one style leaves the other alone.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testDuplicateTableStyleIsIndependent)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"DuplicateOwn"_ustr);

    auto pColorSet = createTestThemeA();
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);
    ScTableStyles* pStyles = m_pDoc->GetTableStyles();
    CPPUNIT_ASSERT(pStyles);

    auto pPattern = std::make_unique<ScPatternAttr>(m_pDoc->getCellAttributeHelper());
    pPattern->GetItemSetWritable().Put(SvxBrushItem(COL_LIGHTRED, ATTR_BACKGROUND));
    auto pOriginal = std::make_unique<ScTableStyle>(u"TableStyleCustom1"_ustr,
                                                   std::optional<OUString>(u"Original"_ustr));
    pOriginal->SetPattern(ScTableStyleElement::WholeTable, std::move(pPattern));
    pStyles->AddTableStyle(std::move(pOriginal));

    const OUString aCopyName
        = pStyles->DuplicateTableStyle(u"TableStyleCustom1"_ustr, u"Copy"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"TableStyleCustom2"_ustr, aCopyName);

    // Recolour the original; the copy keeps the colour it was made with.
    auto pRecolour = std::make_unique<ScPatternAttr>(m_pDoc->getCellAttributeHelper());
    pRecolour->GetItemSetWritable().Put(SvxBrushItem(COL_LIGHTBLUE, ATTR_BACKGROUND));
    const_cast<ScTableStyle*>(pStyles->GetTableStyle(u"TableStyleCustom1"_ustr))
        ->SetPattern(ScTableStyleElement::WholeTable, std::move(pRecolour));

    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE,
                         regionBackground(*pStyles->GetTableStyle(u"TableStyleCustom1"_ustr),
                                          ScTableStyleElement::WholeTable));
    CPPUNIT_ASSERT_EQUAL(
        COL_LIGHTRED,
        regionBackground(*pStyles->GetTableStyle(aCopyName), ScTableStyleElement::WholeTable));

    m_pDoc->DeleteTab(0);
}

// A name of the user's own may carry the same per-cent token the numbering pattern
// uses, and it still reaches the copy as they wrote it.
CPPUNIT_TEST_FIXTURE(TableStylesTest, testDuplicateTableStyleNameKeepsPerCentToken)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"PerCentName"_ustr);

    auto pColorSet = createTestThemeA();
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);
    ScTableStyles* pStyles = m_pDoc->GetTableStyles();
    CPPUNIT_ASSERT(pStyles);

    const OUString aRequested = u"Sales %2 and %1"_ustr;
    const OUString aFirst = pStyles->DuplicateTableStyle(u"TableStyleMedium2"_ustr, aRequested);
    const OUString aSecond = pStyles->DuplicateTableStyle(u"TableStyleMedium2"_ustr, aRequested);

    CPPUNIT_ASSERT_EQUAL(aRequested, pStyles->GetTableStyle(aFirst)->GetUIName());
    CPPUNIT_ASSERT_EQUAL(u"Sales %2 and %1 (2)"_ustr,
                         pStyles->GetTableStyle(aSecond)->GetUIName());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TableStylesTest, testDuplicateTableStyleUINameStaysApart)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"DuplicateNaming"_ustr);

    auto pColorSet = createTestThemeA();
    ScTableStyleGenerator::generateDefaultStyles(*m_pDoc, *pColorSet);
    ScTableStyles* pStyles = m_pDoc->GetTableStyles();
    CPPUNIT_ASSERT(pStyles);

    // Copying one style repeatedly asks for the same name every time, which is what
    // the command does, so each copy has to end up under a name of its own.
    const OUString aRequested = u"Copy of Medium 2"_ustr;
    const OUString aFirst = pStyles->DuplicateTableStyle(u"TableStyleMedium2"_ustr, aRequested);
    const OUString aSecond = pStyles->DuplicateTableStyle(u"TableStyleMedium2"_ustr, aRequested);
    const OUString aThird = pStyles->DuplicateTableStyle(u"TableStyleMedium2"_ustr, aRequested);

    CPPUNIT_ASSERT_EQUAL(aRequested, pStyles->GetTableStyle(aFirst)->GetUIName());
    CPPUNIT_ASSERT_EQUAL(u"Copy of Medium 2 (2)"_ustr, pStyles->GetTableStyle(aSecond)->GetUIName());
    CPPUNIT_ASSERT_EQUAL(u"Copy of Medium 2 (3)"_ustr, pStyles->GetTableStyle(aThird)->GetUIName());

    // A name already taken by a style the user made is avoided the same way.
    pStyles->DuplicateTableStyle(u"TableStyleMedium2"_ustr, u"Taken"_ustr);
    const OUString aAfterTaken
        = pStyles->DuplicateTableStyle(u"TableStyleMedium2"_ustr, u"Taken"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Taken (2)"_ustr, pStyles->GetTableStyle(aAfterTaken)->GetUIName());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
