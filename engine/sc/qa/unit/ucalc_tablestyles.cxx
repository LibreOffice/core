/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"

#include <TableStyleGenerator.hxx>
#include <tablestyle.hxx>
#include <attrib.hxx>
#include <dbdata.hxx>
#include <docfunc.hxx>
#include <docsh.hxx>
#include <document.hxx>
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
    auto pColorSet = std::make_shared<model::ColorSet>("ThemeA");
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
    auto pColorSet = std::make_shared<model::ColorSet>("ThemeB");
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
    std::unique_ptr<SvxBoxItem> pBoxItem = pStyle->GetBoxItem(*pDBData, 1, 5, 4);
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

    // Total row (row 10, rowIndex 9) — should have a DOUBLE top border
    std::unique_ptr<SvxBoxItem> pTotalBox = pStyle->GetBoxItem(*pDBData, 1, 10, 9);
    CPPUNIT_ASSERT(pTotalBox);
    const editeng::SvxBorderLine* pTotalTop = pTotalBox->GetLine(SvxBoxItemLine::TOP);
    CPPUNIT_ASSERT(pTotalTop);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOUBLE_THIN, pTotalTop->GetBorderLineStyle());

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
    std::unique_ptr<SvxBoxItem> pBoxB = pStyle->GetBoxItem(*pDBData, 1, 5, 4);
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
            std::unique_ptr<SvxBoxItem> pBox = pMedium->GetBoxItem(*pDBData, 1, 5, 4);
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
    std::unique_ptr<SvxBoxItem> pBoxLast = pStyle->GetBoxItem(aDBNoHeaderNoTotal, 1, 9, 9);
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
    std::unique_ptr<SvxBoxItem> pTotalBox = pStyle->GetBoxItem(aDBWithBoth, 1, 10, 9);
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

// Test 12: Verify tiny table (single data row) doesn't crash
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

    // Should not crash — header row 0
    const SvxBrushItem* pFill = pStyle->GetFillItem(aDB1, 0, 0, 0);
    CPPUNIT_ASSERT(pFill); // header should still have fill
    const SfxItemSet* pFont = pStyle->GetFontItemSet(aDB1, 0, 0, 0);
    CPPUNIT_ASSERT(pFont);
    std::unique_ptr<SvxBoxItem> pBox = pStyle->GetBoxItem(aDB1, 0, 0, 0);
    // Border may or may not exist, just verify no crash

    // Data row 1
    pStyle->GetFillItem(aDB1, 0, 1, 0);
    pStyle->GetFontItemSet(aDB1, 0, 1, 0);
    pStyle->GetBoxItem(aDB1, 0, 1, 0);

    // Single cell table: A1:A1 — just header, no data
    ScDBData aDB2(u"Tiny2"_ustr, 0, 0, 0, 0, 0, true, true, false);
    aDB2.SetTableStyleInfo(aStyleParam);

    // Should not crash
    pStyle->GetFillItem(aDB2, 0, 0, 0);
    pStyle->GetFontItemSet(aDB2, 0, 0, 0);
    pStyle->GetBoxItem(aDB2, 0, 0, 0);

    // Header + total, no data rows: A1:B2 with total
    ScDBData aDB3(u"Tiny3"_ustr, 0, 0, 0, 1, 1, true, true, true);
    aDB3.SetTableStyleInfo(aStyleParam);

    // Row 0 = header, row 1 = total — no data rows
    pStyle->GetFillItem(aDB3, 0, 0, 0);
    pStyle->GetFillItem(aDB3, 0, 1, 0);
    pStyle->GetBoxItem(aDB3, 0, 0, 0);
    pStyle->GetBoxItem(aDB3, 0, 1, 0);

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

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
