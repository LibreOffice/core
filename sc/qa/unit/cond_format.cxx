/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"
#include <colorscale.hxx>
#include <condformathelper.hxx>
#include <conditio.hxx>
#include <document.hxx>
#include <editeng/justifyitem.hxx>
#include <fillinfo.hxx>
#include <inputopt.hxx>
#include <scmod.hxx>
#include <svl/zformat.hxx>

using namespace sc;

class CondFormatTest : public ScModelTestBase
{
public:
    CondFormatTest()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }
};

namespace
{
void checkConditionalFormatList(ScDocument& rDoc,
                                std::unordered_map<OUString, OUString>& rExpectedValues)
{
    ScConditionalFormatList* pList = rDoc.GetCondFormList(0);
    CPPUNIT_ASSERT_EQUAL(rExpectedValues.size(), pList->size());

    OUString sRangeStr;
    for (const auto& rItem : *pList)
    {
        const ScRangeList& aRange = rItem->GetRange();
        aRange.Format(sRangeStr, ScRefFlags::VALID, rDoc, rDoc.GetAddressConvention());
        CPPUNIT_ASSERT_MESSAGE(OString(sRangeStr.toUtf8() + " not found").getStr(),
                               rExpectedValues.count(sRangeStr));
        CPPUNIT_ASSERT_EQUAL(rExpectedValues[sRangeStr],
                             ScCondFormatHelper::GetExpression(*rItem, aRange.GetTopLeftCorner()));
    }
}

void checkDatabarPositiveColor(const ScConditionalFormat* pFormat, const Color& rColor)
{
    CPPUNIT_ASSERT(pFormat);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Databar, pEntry->GetType());
    const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pEntry);
    const ScDataBarFormatData* pDataBarFormatData = pDataBar->GetDataBarData();

    CPPUNIT_ASSERT_EQUAL(rColor, pDataBarFormatData->maPositiveColor);
}

struct FindCondFormatByEnclosingRange
{
    explicit FindCondFormatByEnclosingRange(const ScRange& rRange)
        : mrRange(rRange)
    {
    }

    bool operator()(const std::unique_ptr<ScConditionalFormat>& pFormat)
    {
        if (pFormat->GetRange().Combine() == mrRange)
            return true;

        return false;
    }

private:
    const ScRange& mrRange;
};

struct DataBarData
{
    ScRange aRange;
    ScColorScaleEntryType eLowerLimitType;
    ScColorScaleEntryType eUpperLimitType;
    databar::ScAxisPosition eAxisPosition;
};

DataBarData const aData[]
    = { { ScRange(1, 2, 0, 1, 5, 0), COLORSCALE_AUTO, COLORSCALE_AUTO, databar::AUTOMATIC },
        { ScRange(3, 2, 0, 3, 5, 0), COLORSCALE_MIN, COLORSCALE_MAX, databar::AUTOMATIC },
        { ScRange(5, 2, 0, 5, 5, 0), COLORSCALE_PERCENTILE, COLORSCALE_PERCENT,
          databar::AUTOMATIC },
        { ScRange(7, 2, 0, 7, 5, 0), COLORSCALE_VALUE, COLORSCALE_FORMULA, databar::AUTOMATIC },
        { ScRange(1, 9, 0, 1, 12, 0), COLORSCALE_AUTO, COLORSCALE_AUTO, databar::MIDDLE } };

void testDataBar_Impl(const ScDocument& rDoc)
{
    ScConditionalFormatList* pList = rDoc.GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    for (size_t i = 0; i < std::size(aData); ++i)
    {
        ScConditionalFormatList::const_iterator itr = std::find_if(
            pList->begin(), pList->end(), FindCondFormatByEnclosingRange(aData[i].aRange));
        CPPUNIT_ASSERT(itr != pList->end());
        CPPUNIT_ASSERT_EQUAL(size_t(1), (*itr)->size());

        const ScFormatEntry* pFormatEntry = (*itr)->GetEntry(0);
        CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Databar, pFormatEntry->GetType());
        const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pFormatEntry);
        CPPUNIT_ASSERT(pDataBar);
        const ScDataBarFormatData* pDataBarData = pDataBar->GetDataBarData();
        CPPUNIT_ASSERT_EQUAL(aData[i].eLowerLimitType, pDataBarData->mpLowerLimit->GetType());
        CPPUNIT_ASSERT_EQUAL(aData[i].eUpperLimitType, pDataBarData->mpUpperLimit->GetType());

        CPPUNIT_ASSERT_EQUAL(aData[i].eAxisPosition, pDataBarData->meAxisPosition);
    }
}

struct ColorScale2EntryData
{
    ScRange aRange;
    ScColorScaleEntryType eLowerType;
    ScColorScaleEntryType eUpperType;
};

ColorScale2EntryData const aData2Entry[]
    = { { ScRange(1, 2, 0, 1, 5, 0), COLORSCALE_MIN, COLORSCALE_MAX },
        { ScRange(3, 2, 0, 3, 5, 0), COLORSCALE_PERCENTILE, COLORSCALE_PERCENT },
        { ScRange(5, 2, 0, 5, 5, 0), COLORSCALE_VALUE, COLORSCALE_FORMULA } };

void testColorScale2Entry_Impl(const ScDocument& rDoc)
{
    const ScConditionalFormatList* pList = rDoc.GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    for (size_t i = 0; i < std::size(aData2Entry); ++i)
    {
        ScConditionalFormatList::const_iterator itr = std::find_if(
            pList->begin(), pList->end(), FindCondFormatByEnclosingRange(aData2Entry[i].aRange));
        CPPUNIT_ASSERT(itr != pList->end());
        CPPUNIT_ASSERT_EQUAL(size_t(1), (*itr)->size());

        const ScFormatEntry* pFormatEntry = (*itr)->GetEntry(0);
        CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Colorscale, pFormatEntry->GetType());
        const ScColorScaleFormat* pColFormat = static_cast<const ScColorScaleFormat*>(pFormatEntry);
        CPPUNIT_ASSERT_EQUAL(size_t(2), pColFormat->size());

        ScColorScaleEntries::const_iterator format_itr = pColFormat->begin();
        CPPUNIT_ASSERT_EQUAL(aData2Entry[i].eLowerType, (*format_itr)->GetType());
        ++format_itr;
        CPPUNIT_ASSERT(format_itr != pColFormat->end());
        CPPUNIT_ASSERT_EQUAL(aData2Entry[i].eUpperType, (*format_itr)->GetType());
    }
}

struct ColorScale3EntryData
{
    ScRange aRange;
    ScColorScaleEntryType eLowerType;
    ScColorScaleEntryType eMiddleType;
    ScColorScaleEntryType eUpperType;
};

ColorScale3EntryData const aData3Entry[]
    = { { ScRange(1, 1, 1, 1, 6, 1), COLORSCALE_MIN, COLORSCALE_PERCENTILE, COLORSCALE_MAX },
        { ScRange(3, 1, 1, 3, 6, 1), COLORSCALE_PERCENTILE, COLORSCALE_VALUE, COLORSCALE_PERCENT },
        { ScRange(5, 1, 1, 5, 6, 1), COLORSCALE_VALUE, COLORSCALE_VALUE, COLORSCALE_FORMULA } };

void testColorScale3Entry_Impl(const ScDocument& rDoc)
{
    ScConditionalFormatList* pList = rDoc.GetCondFormList(1);
    CPPUNIT_ASSERT(pList);

    for (size_t i = 0; i < std::size(aData3Entry); ++i)
    {
        ScConditionalFormatList::const_iterator itr = std::find_if(
            pList->begin(), pList->end(), FindCondFormatByEnclosingRange(aData3Entry[i].aRange));
        CPPUNIT_ASSERT(itr != pList->end());
        CPPUNIT_ASSERT_EQUAL(size_t(1), (*itr)->size());

        const ScFormatEntry* pFormatEntry = (*itr)->GetEntry(0);
        CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Colorscale, pFormatEntry->GetType());
        const ScColorScaleFormat* pColFormat = static_cast<const ScColorScaleFormat*>(pFormatEntry);
        CPPUNIT_ASSERT_EQUAL(size_t(3), pColFormat->size());

        ScColorScaleEntries::const_iterator format_itr = pColFormat->begin();
        CPPUNIT_ASSERT_EQUAL(aData3Entry[i].eLowerType, (*format_itr)->GetType());
        ++format_itr;
        CPPUNIT_ASSERT(format_itr != pColFormat->end());
        CPPUNIT_ASSERT_EQUAL(aData3Entry[i].eMiddleType, (*format_itr)->GetType());
        ++format_itr;
        CPPUNIT_ASSERT(format_itr != pColFormat->end());
        CPPUNIT_ASSERT_EQUAL(aData3Entry[i].eUpperType, (*format_itr)->GetType());
    }
}

void testComplexIconSetsXLSX_Impl(const ScDocument& rDoc, SCCOL nCol, ScIconSetType eType)
{
    ScConditionalFormat* pFormat = rDoc.GetCondFormat(nCol, 1, 0);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(eType, pIconSet->GetIconSetData()->eIconSetType);
}

void testCustomIconSetsXLSX_Impl(const ScDocument& rDoc, SCCOL nCol, SCROW nRow, SCTAB nTab,
                                 ScIconSetType eType, sal_Int32 nIndex)
{
    ScConditionalFormat* pFormat = rDoc.GetCondFormat(nCol, nRow, nTab);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);
    std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(nCol, nRow, nTab)));
    if (nIndex == -1)
        CPPUNIT_ASSERT(!pInfo);
    else
    {
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(nIndex, pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(eType, pInfo->eIconSetType);
    }
}
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testTdf104026)
{
    createScDoc("ods/tdf104026.ods");
    ScDocument* pDoc = getScDoc();

    std::unordered_map<OUString, OUString> aExpectedValues
        = { { "A2", "Cell value != $Sheet1.$B2" }, { "A3", "Cell value != $Sheet1.$B3" },
            { "A4", "Cell value != $Sheet1.$B4" }, { "A5", "Cell value != $Sheet1.$B5" },
            { "A6", "Cell value != $Sheet1.$B6" }, { "A7", "Cell value != $Sheet1.$B7" } };

    checkConditionalFormatList(*pDoc, aExpectedValues);

    goToCell(u"A2"_ustr);
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    std::unordered_map<OUString, OUString> aExpectedValues2
        = { { "A2", "Cell value != $Sheet1.$B2" },
            { "A3", "Cell value != $Sheet1.$B3" },
            { "A4", "Cell value != $Sheet1.$B4" },
            { "A5", "Cell value != $Sheet1.$B5" },
            { "A6", "Cell value != $Sheet1.$B6" } };
    // Without the fix in place, this test would have failed with
    // - Expected: Cell value != $Sheet1.$B2
    // - Actual  : Cell value != $Sheet1.$B#REF!
    checkConditionalFormatList(*pDoc, aExpectedValues2);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // tdf#140330: Without the fix in place, this test would have failed with
    // - Expected: 6
    // - Actual  : 5
    checkConditionalFormatList(*pDoc, aExpectedValues);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testTdf167019)
{
    createScDoc("xlsx/tdf167019.xlsx");
    ScDocument* pDoc = getScDoc();

    std::unordered_map<OUString, OUString> aExpectedValues
        = { { "D4,G4,J4,M4,G6:G10,J6:J10,M6:M10,D6:D10", "Cell value is not between 100 and 152" },
            { "E4,H4,K4,N4,E6:E10,H6:H10,K6:K10,N6:N10", "Cell value is not between 54 and 102" },
            { "F4,I4,L4,O4,F6,I6,L6,O6", "Cell value is not between 32 and 100" } };

    checkConditionalFormatList(*pDoc, aExpectedValues);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testTdf92963)
{
    createScDoc("ods/tdf92963.ods");
    ScDocument* pDoc = getScDoc();

    // Disable replace cell warning
    ScModule* pMod = ScModule::get();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetReplaceCellsWarn();
    aInputOption.SetReplaceCellsWarn(false);
    pMod->SetInputOptions(aInputOption);

    std::unordered_map<OUString, OUString> aExpectedValues
        = { { "C1", "Cell value > 14" }, { "C3", "Cell value > 14" }, { "C4", "Cell value > 14" } };

    checkConditionalFormatList(*pDoc, aExpectedValues);

    goToCell(u"A3:C4"_ustr);

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    goToCell(u"A1:C1"_ustr);

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    aExpectedValues = { { "C3,C1", "Cell value > 14" }, { "C4,C2", "Cell value > 14" } };
    checkConditionalFormatList(*pDoc, aExpectedValues);

    // Restore previous status
    aInputOption.SetReplaceCellsWarn(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testCondFormatXLSB)
{
    createScDoc("xlsb/cond_format.xlsb");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormatList* pList = pDoc->GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());
    ScConditionalFormat* pFormat = pList->begin()->get();
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testColorScaleNumWithRefXLSX)
{
    createScDoc("xlsx/colorscale_num_with_ref.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormatList* pList = pDoc->GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());

    ScConditionalFormat* pFormat = pList->begin()->get();
    CPPUNIT_ASSERT(pFormat);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);

    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Colorscale, pEntry->GetType());

    const ScColorScaleFormat* pColorScale = dynamic_cast<const ScColorScaleFormat*>(pEntry);
    CPPUNIT_ASSERT(pColorScale);

    const ScColorScaleEntry* pColorScaleEntry = pColorScale->GetEntry(1);
    CPPUNIT_ASSERT_EQUAL(u"=$A$1"_ustr,
                         pColorScaleEntry->GetFormula(formula::FormulaGrammar::GRAM_NATIVE));
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testCondFormatThemeColor3XLSX)
{
    // theme index 2 and 3 are switched
    createScDoc("xlsx/cond_format_theme_color3.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormat* pFormat = pDoc->GetCondFormat(1, 3, 0);
    CPPUNIT_ASSERT(pFormat);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Colorscale, pEntry->GetType());
    const ScColorScaleFormat* pColorScale = static_cast<const ScColorScaleFormat*>(pEntry);

    CPPUNIT_ASSERT_EQUAL(size_t(2), pColorScale->size());
    const ScColorScaleEntry* pColorScaleEntry = pColorScale->GetEntry(0);
    CPPUNIT_ASSERT(pColorScaleEntry);
    CPPUNIT_ASSERT_EQUAL(Color(175, 171, 171), pColorScaleEntry->GetColor());

    pColorScaleEntry = pColorScale->GetEntry(1);
    CPPUNIT_ASSERT(pColorScaleEntry);
    CPPUNIT_ASSERT_EQUAL(Color(51, 63, 80), pColorScaleEntry->GetColor());

    pFormat = pDoc->GetCondFormat(3, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(59, 56, 56));

    pFormat = pDoc->GetCondFormat(5, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(173, 185, 202));

    pFormat = pDoc->GetCondFormat(7, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(89, 89, 89));

    pFormat = pDoc->GetCondFormat(9, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(217, 217, 217));
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testTdf101104)
{
    createScDoc("ods/tdf101104.ods");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = pDoc->GetCondFormat(1, 1, 0);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);

    for (size_t i = 1; i < 10; ++i)
    {
        std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(1, i, 0)));

        // Without the fix in place, this test would have failed here
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(IconSet_3Arrows, pInfo->eIconSetType);
    }
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testCondFormatThemeColor2XLSX)
{
    // negative bar color and axis color
    createScDoc("xlsx/cond_format_theme_color2.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormat* pFormat = pDoc->GetCondFormat(5, 5, 0);
    CPPUNIT_ASSERT(pFormat);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Databar, pEntry->GetType());
    const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pEntry);
    const ScDataBarFormatData* pDataBarFormatData = pDataBar->GetDataBarData();

    CPPUNIT_ASSERT_EQUAL(Color(99, 142, 198), pDataBarFormatData->maPositiveColor);
    CPPUNIT_ASSERT(pDataBarFormatData->mxNegativeColor);
    CPPUNIT_ASSERT_EQUAL(Color(217, 217, 217), *pDataBarFormatData->mxNegativeColor);
    CPPUNIT_ASSERT_EQUAL(Color(197, 90, 17), pDataBarFormatData->maAxisColor);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testCondFormatImportCellIs)
{
    createScDoc("xlsx/condFormat_cellis.xlsx");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormat);

    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    const ScCondFormatEntry* pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::Equal, pCondition->GetOperation());

    OUString aStr = pCondition->GetExpression(ScAddress(0, 0, 0), 0);
    CPPUNIT_ASSERT_EQUAL(u"$Sheet2.$A$2"_ustr, aStr);

    pEntry = pFormat->GetEntry(1);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::Equal, pCondition->GetOperation());

    aStr = pCondition->GetExpression(ScAddress(0, 0, 0), 0);
    CPPUNIT_ASSERT_EQUAL(u"$Sheet2.$A$1"_ustr, aStr);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testCondFormatThemeColorXLSX)
{
    createScDoc("xlsx/condformat_theme_color.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Databar, pEntry->GetType());
    const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pEntry);
    const ScDataBarFormatData* pDataBarFormatData = pDataBar->GetDataBarData();

    CPPUNIT_ASSERT_EQUAL(Color(157, 195, 230), pDataBarFormatData->maPositiveColor);
    CPPUNIT_ASSERT(pDataBarFormatData->mxNegativeColor);
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, *pDataBarFormatData->mxNegativeColor);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetCondFormList(1)->size());
    pFormat = pDoc->GetCondFormat(0, 0, 1);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Colorscale, pEntry->GetType());
    const ScColorScaleFormat* pColorScale = static_cast<const ScColorScaleFormat*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pColorScale->size());
    const ScColorScaleEntry* pColorScaleEntry = pColorScale->GetEntry(0);
    CPPUNIT_ASSERT(pColorScaleEntry);
    CPPUNIT_ASSERT_EQUAL(Color(255, 230, 153), pColorScaleEntry->GetColor());

    pColorScaleEntry = pColorScale->GetEntry(1);
    CPPUNIT_ASSERT(pColorScaleEntry);
    CPPUNIT_ASSERT_EQUAL(Color(157, 195, 230), pColorScaleEntry->GetColor());
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testTdf64401)
{
    createScDoc("ods/tdf64401.ods");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);

    for (size_t i = 0; i < 10; ++i)
    {
        sal_Int32 nIndex = 0;
        if (i >= 7) // B5 = 8
            nIndex = 2;
        else if (i >= 3) // B4 = 4
            nIndex = 1;

        std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(0, i, 0)));
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(nIndex, pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(IconSet_3Arrows, pInfo->eIconSetType);
    }

    // Update values in B4 and B5
    pDoc->SetValue(ScAddress(1, 3, 0), 2.0);
    pDoc->SetValue(ScAddress(1, 4, 0), 9.0);

    for (size_t i = 0; i < 10; ++i)
    {
        sal_Int32 nIndex = 0;
        if (i >= 8) // B5 = 9
            nIndex = 2;
        else if (i >= 1) // B4 = 2
            nIndex = 1;

        std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(0, i, 0)));
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(nIndex, pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(IconSet_3Arrows, pInfo->eIconSetType);
    }
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testCondFormatParentXLSX)
{
    createScDoc("xlsx/cond_parent.xlsx");

    ScDocument* pDoc = getScDoc();
    const SfxItemSet* pCondSet = pDoc->GetCondResult(2, 5, 0);
    const ScPatternAttr* pPattern = pDoc->GetPattern(2, 5, 0);
    const SfxPoolItem& rPoolItem = pPattern->GetItem(ATTR_VER_JUSTIFY, pCondSet);
    const SvxVerJustifyItem& rVerJustify = static_cast<const SvxVerJustifyItem&>(rPoolItem);
    CPPUNIT_ASSERT_EQUAL(SvxCellVerJustify::Top, rVerJustify.GetValue());
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testDataBarODS)
{
    createScDoc("ods/databar.ods");

    ScDocument* pDoc = getScDoc();
    testDataBar_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testDataBarXLSX)
{
    createScDoc("xlsx/databar.xlsx");

    ScDocument* pDoc = getScDoc();
    testDataBar_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testColorScaleODS)
{
    createScDoc("ods/colorscale.ods");
    ScDocument* pDoc = getScDoc();

    testColorScale2Entry_Impl(*pDoc);
    testColorScale3Entry_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testColorScaleXLSX)
{
    createScDoc("xlsx/colorscale.xlsx");
    ScDocument* pDoc = getScDoc();

    testColorScale2Entry_Impl(*pDoc);
    testColorScale3Entry_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testColorScaleExportODS)
{
    createScDoc("ods/colorscale.ods");

    saveAndReload(u"calc8"_ustr);

    ScDocument* pDoc = getScDoc();

    testColorScale2Entry_Impl(*pDoc);
    testColorScale3Entry_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testColorScaleExportXLSX)
{
    createScDoc("xlsx/colorscale.xlsx");

    saveAndReload(u"Calc Office Open XML"_ustr);

    ScDocument* pDoc = getScDoc();

    testColorScale2Entry_Impl(*pDoc);
    testColorScale3Entry_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testDataBarExportODS)
{
    createScDoc("ods/databar.ods");

    saveAndReload(u"calc8"_ustr);

    ScDocument* pDoc = getScDoc();

    testDataBar_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testDataBarExportXLSX)
{
    createScDoc("xlsx/databar.xlsx");

    saveAndReload(u"Calc Office Open XML"_ustr);

    ScDocument* pDoc = getScDoc();

    testDataBar_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testComplexIconSetsXLSX)
{
    auto verify = [this]() {
        ScDocument* pDoc = getScDoc();
        CPPUNIT_ASSERT_EQUAL(size_t(3), pDoc->GetCondFormList(0)->size());
        testComplexIconSetsXLSX_Impl(*pDoc, 1, IconSet_3Triangles);
        testComplexIconSetsXLSX_Impl(*pDoc, 3, IconSet_3Stars);
        testComplexIconSetsXLSX_Impl(*pDoc, 5, IconSet_5Boxes);

        CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetCondFormList(1)->size());
        testCustomIconSetsXLSX_Impl(*pDoc, 1, 1, 1, IconSet_3ArrowsGray, 0);
        testCustomIconSetsXLSX_Impl(*pDoc, 1, 2, 1, IconSet_3ArrowsGray, -1);
        testCustomIconSetsXLSX_Impl(*pDoc, 1, 3, 1, IconSet_3Arrows, 1);
        testCustomIconSetsXLSX_Impl(*pDoc, 1, 4, 1, IconSet_3ArrowsGray, -1);
        testCustomIconSetsXLSX_Impl(*pDoc, 1, 5, 1, IconSet_3Arrows, 2);

        testCustomIconSetsXLSX_Impl(*pDoc, 3, 1, 1, IconSet_4RedToBlack, 3);
        testCustomIconSetsXLSX_Impl(*pDoc, 3, 2, 1, IconSet_3TrafficLights1, 1);
        testCustomIconSetsXLSX_Impl(*pDoc, 3, 3, 1, IconSet_3Arrows, 2);
    };

    createScDoc("xlsx/complex_icon_set.xlsx");
    verify();
    saveAndReload(u"Calc Office Open XML"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testTdf162948)
{
    auto verify = [this]() {
        ScDocument* pDoc = getScDoc();
        CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetCondFormList(0)->size());
        testCustomIconSetsXLSX_Impl(*pDoc, 0, 0, 0, IconSet_3Arrows, 1);
        testCustomIconSetsXLSX_Impl(*pDoc, 0, 1, 0, IconSet_3Arrows, 1);
        testCustomIconSetsXLSX_Impl(*pDoc, 0, 2, 0, IconSet_3Arrows, 1);
        testCustomIconSetsXLSX_Impl(*pDoc, 0, 3, 0, IconSet_3Arrows, 1);

        testCustomIconSetsXLSX_Impl(*pDoc, 1, 0, 0, IconSet_3Arrows, 2);
        testCustomIconSetsXLSX_Impl(*pDoc, 1, 1, 0, IconSet_3Arrows, 1);
        testCustomIconSetsXLSX_Impl(*pDoc, 1, 2, 0, IconSet_3Arrows, 1);
        testCustomIconSetsXLSX_Impl(*pDoc, 1, 3, 0, IconSet_3Arrows, 1);
    };

    createScDoc("xlsx/tdf162948.xlsx");
    verify();
    saveAndReload(u"Calc Office Open XML"_ustr);
    verify();

    // FIXME: Error: tag name "calcext:icon-set" is not allowed. Possible tag names are: <color-scale>,<condition>,<data-bar>
    skipValidation();
    saveAndReload(u"calc8"_ustr); // tdf#163337
    verify();
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testTdf165383)
{
    auto verify = [this]() {
        ScDocument* pDoc = getScDoc();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetCondFormList(0)->size());

        ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
        CPPUNIT_ASSERT(pFormat);

        const ScFormatEntry* pEntry = pFormat->GetEntry(0);
        CPPUNIT_ASSERT(pEntry);
        CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Condition, pEntry->GetType());
        const ScConditionEntry* pConditionEntry = static_cast<const ScConditionEntry*>(pEntry);
        CPPUNIT_ASSERT_EQUAL(ScConditionMode::Direct, pConditionEntry->GetOperation());
        // Without the fix in place, this test would have failed after the roundtrip with
        // - Expected: SUM($A$1:A1) > 10
        // - Actual  : SUM($A$1) > 10
        CPPUNIT_ASSERT_EQUAL(u"SUM($A$1:A1) > 10"_ustr,
                             pConditionEntry->GetExpression(ScAddress(0, 0, 0), 0));
    };

    createScDoc("ods/tdf165383.ods");
    verify();
    saveAndReload(u"Calc Office Open XML"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testTdf166669)
{
    auto verify = [this]() {
        ScDocument* pDoc = getScDoc();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetCondFormList(0)->size());

        ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
        CPPUNIT_ASSERT(pFormat);

        const ScFormatEntry* pEntry = pFormat->GetEntry(0);
        CPPUNIT_ASSERT(pEntry);
        CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Condition, pEntry->GetType());
        const ScConditionEntry* pConditionEntry = static_cast<const ScConditionEntry*>(pEntry);
        CPPUNIT_ASSERT_EQUAL(ScConditionMode::Duplicate, pConditionEntry->GetOperation());
        for (SCROW row = 0; row < 5; ++row)
        {
            ScRefCellValue aCell(*pDoc, ScAddress(0, row, 0));
            CPPUNIT_ASSERT_EQUAL(row == 4 ? false : true,
                                 pConditionEntry->IsCellValid(aCell, ScAddress(0, row, 0)));
        }
    };

    createScDoc("ods/tdf166669.ods");
    verify();
    saveAndReload(u"Calc Office Open XML"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testCondFormatExportCellIs)
{
    createScDoc("xlsx/condFormat_cellis.xlsx");
    saveAndReload(u"Calc Office Open XML"_ustr);

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormat);

    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    const ScCondFormatEntry* pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::Equal, pCondition->GetOperation());

    OUString aStr = pCondition->GetExpression(ScAddress(0, 0, 0), 0);
    CPPUNIT_ASSERT_EQUAL(u"$Sheet2.$A$2"_ustr, aStr);

    pEntry = pFormat->GetEntry(1);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::Equal, pCondition->GetOperation());

    aStr = pCondition->GetExpression(ScAddress(0, 0, 0), 0);
    CPPUNIT_ASSERT_EQUAL(u"$Sheet2.$A$1"_ustr, aStr);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testConditionalFormatExportXLSX)
{
    createScDoc("xlsx/new_cond_format_test_export.xlsx");

    saveAndReload(u"Calc Office Open XML"_ustr);
    ScDocument* pDoc = getScDoc();
    {
        OUString aCSVPath = createFilePath(u"contentCSV/new_cond_format_test_export.csv");
        testCondFile(aCSVPath, pDoc, 0);
    }
    {
        OUString aCSVPath = createFilePath(u"contentCSV/new_cond_format_test_sheet2.csv");
        testCondFile(aCSVPath, pDoc, 1);
    }
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testNewCondFormatODS)
{
    createScDoc("ods/new_cond_format_test.ods");

    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/new_cond_format_test.csv");
    testCondFile(aCSVPath, pDoc, 0);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testNewCondFormatXLSX)
{
    createScDoc("xlsx/new_cond_format_test.xlsx");

    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/new_cond_format_test.csv");
    testCondFile(aCSVPath, pDoc, 0);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testConditionalFormatExportODS)
{
    createScDoc("ods/new_cond_format_test_export.ods");

    saveAndReload(u"calc8"_ustr);
    ScDocument* pDoc = getScDoc();
    OUString aCSVPath = createFilePath(u"contentCSV/new_cond_format_test_export.csv");
    testCondFile(aCSVPath, pDoc, 0);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testConditionalFormatNumberInTextRule)
{
    createScDoc();

    ScDocument* pDocument = getScDoc();
    ScAddress aAddress(0, 0, 0);

    auto pFormat = std::make_unique<ScConditionalFormat>(0, *pDocument);
    ScRange aCondFormatRange(aAddress);
    ScRangeList aRangeList(aCondFormatRange);
    pFormat->SetRange(aRangeList);
    ScCondFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::BeginsWith, u"15"_ustr,
                                                      u""_ustr, *pDocument, aAddress, u""_ustr);
    pFormat->AddEntry(pEntry);
    pDocument->AddCondFormat(std::move(pFormat), 0);

    saveAndReload(u"Calc Office Open XML"_ustr);
    pDocument = getScDoc();

    ScConditionalFormat* pCondFormat = pDocument->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pCondFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pCondFormat->size());
    const ScFormatEntry* pCondFormatEntry = pCondFormat->GetEntry(0);
    CPPUNIT_ASSERT(pCondFormatEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Condition, pCondFormatEntry->GetType());
    const ScConditionEntry* pConditionEntry
        = static_cast<const ScConditionEntry*>(pCondFormatEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::BeginsWith, pConditionEntry->GetOperation());
    CPPUNIT_ASSERT_EQUAL(u"\"15\""_ustr, pConditionEntry->GetExpression(aAddress, 0));
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testTdf155321_CondFormatColor_XLSX)
{
    createScDoc("xlsx/tdf155321.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormat* pCondFormat = pDoc->GetCondFormat(0, 0, 0);
    ScRefCellValue aCellB1(*pDoc, ScAddress(1, 0, 0));
    Color aColor = pCondFormat->GetData(aCellB1, ScAddress(1, 0, 0)).mxColorScale.value();
    CPPUNIT_ASSERT_EQUAL(Color(99, 190, 123), aColor);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testTdf156028_ColorScale_XLSX)
{
    createScDoc("xlsx/tdf156028.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormat* pCondFormat = pDoc->GetCondFormat(0, 0, 0);
    ScRefCellValue aCellA1(*pDoc, ScAddress(0, 0, 0));
    Color aColor = pCondFormat->GetData(aCellA1, ScAddress(0, 0, 0)).mxColorScale.value();
    CPPUNIT_ASSERT_EQUAL(Color(99, 190, 123), aColor);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testTdf138601_CondFormatXLSX)
{
    createScDoc("xlsx/tdf138601.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormat* pFormat1 = pDoc->GetCondFormat(0, 0, 0);
    const ScFormatEntry* pEntry1 = pFormat1->GetEntry(0);
    const ScColorScaleFormat* pColorScale1 = static_cast<const ScColorScaleFormat*>(pEntry1);
    const ScColorScaleEntry* pColorScaleEntry1 = pColorScale1->GetEntry(0);
    CPPUNIT_ASSERT_EQUAL(Color(255, 255, 201), pColorScaleEntry1->GetColor());

    ScConditionalFormat* pFormat2 = pDoc->GetCondFormat(1, 0, 0);
    const ScFormatEntry* pEntry2 = pFormat2->GetEntry(0);
    const ScColorScaleFormat* pColorScale2 = static_cast<const ScColorScaleFormat*>(pEntry2);
    const ScColorScaleEntry* pColorScaleEntry2 = pColorScale2->GetEntry(0);
    CPPUNIT_ASSERT_EQUAL(Color(255, 139, 139), pColorScaleEntry2->GetColor());

    ScConditionalFormat* pFormat3 = pDoc->GetCondFormat(0, 1, 0);
    const ScFormatEntry* pEntry3 = pFormat3->GetEntry(0);
    const ScColorScaleFormat* pColorScale3 = static_cast<const ScColorScaleFormat*>(pEntry3);
    const ScColorScaleEntry* pColorScaleEntry3 = pColorScale3->GetEntry(0);
    CPPUNIT_ASSERT_EQUAL(Color(255, 255, 201), pColorScaleEntry3->GetColor());
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testCondFormatOperatorsSameRangeXLSX)
{
    createScDoc("xlsx/tdf139928.xlsx");

    ScDocument* pDoc = getScDoc();

    ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormat);

    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    const ScCondFormatEntry* pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::ContainsText, pCondition->GetOperation());

    pEntry = pFormat->GetEntry(1);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::BeginsWith, pCondition->GetOperation());

    pEntry = pFormat->GetEntry(2);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::EndsWith, pCondition->GetOperation());
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testCondFormatFormulaIsXLSX)
{
    createScDoc("xlsx/tdf113013.xlsx");

    ScDocument* pDoc = getScDoc();

    // "Formula is" condition
    ScConditionalFormat* pFormatB1 = pDoc->GetCondFormat(1, 0, 0);
    CPPUNIT_ASSERT(pFormatB1);
    ScConditionalFormat* pFormatA2 = pDoc->GetCondFormat(0, 1, 0);
    CPPUNIT_ASSERT(pFormatA2);

    ScRefCellValue aCellB1(*pDoc, ScAddress(1, 0, 0));
    OUString aCellStyleB1 = pFormatB1->GetCellStyle(aCellB1, ScAddress(1, 0, 0));
    CPPUNIT_ASSERT(!aCellStyleB1.isEmpty());

    ScRefCellValue aCellA2(*pDoc, ScAddress(0, 1, 0));
    OUString aCellStyleA2 = pFormatA2->GetCellStyle(aCellA2, ScAddress(0, 1, 0));
    CPPUNIT_ASSERT(!aCellStyleA2.isEmpty());
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testCondFormatBeginsAndEndsWithXLSX)
{
    createScDoc("xlsx/tdf120749.xlsx");

    ScDocument* pDoc = getScDoc();

    // begins with and ends with conditions
    ScConditionalFormat* pFormatA1 = pDoc->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormatA1);
    ScConditionalFormat* pFormatA2 = pDoc->GetCondFormat(0, 1, 0);
    CPPUNIT_ASSERT(pFormatA2);
    ScConditionalFormat* pFormatA3 = pDoc->GetCondFormat(0, 2, 0);
    CPPUNIT_ASSERT(pFormatA3);
    ScConditionalFormat* pFormatA4 = pDoc->GetCondFormat(0, 3, 0);
    CPPUNIT_ASSERT(pFormatA4);

    ScRefCellValue aCellA1(*pDoc, ScAddress(0, 0, 0));
    OUString aCellStyleA1 = pFormatA1->GetCellStyle(aCellA1, ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(!aCellStyleA1.isEmpty());

    ScRefCellValue aCellA2(*pDoc, ScAddress(0, 1, 0));
    OUString aCellStyleA2 = pFormatA2->GetCellStyle(aCellA2, ScAddress(0, 1, 0));
    CPPUNIT_ASSERT(!aCellStyleA2.isEmpty());

    ScRefCellValue aCellA3(*pDoc, ScAddress(0, 2, 0));
    OUString aCellStyleA3 = pFormatA3->GetCellStyle(aCellA3, ScAddress(0, 2, 0));
    CPPUNIT_ASSERT(!aCellStyleA3.isEmpty());

    ScRefCellValue aCellA4(*pDoc, ScAddress(0, 3, 0));
    OUString aCellStyleA4 = pFormatA4->GetCellStyle(aCellA4, ScAddress(0, 3, 0));
    CPPUNIT_ASSERT(!aCellStyleA4.isEmpty());
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testExtCondFormatXLSX)
{
    createScDoc("xlsx/tdf122102.xlsx");

    ScDocument* pDoc = getScDoc();

    // contains text and not contains text conditions
    ScConditionalFormat* pFormatA1 = pDoc->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormatA1);
    ScConditionalFormat* pFormatA2 = pDoc->GetCondFormat(0, 1, 0);
    CPPUNIT_ASSERT(pFormatA2);
    ScConditionalFormat* pFormatA3 = pDoc->GetCondFormat(0, 2, 0);
    CPPUNIT_ASSERT(pFormatA3);
    ScConditionalFormat* pFormatA4 = pDoc->GetCondFormat(0, 3, 0);
    CPPUNIT_ASSERT(pFormatA4);

    ScRefCellValue aCellA1(*pDoc, ScAddress(0, 0, 0));
    OUString aCellStyleA1 = pFormatA1->GetCellStyle(aCellA1, ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(!aCellStyleA1.isEmpty());

    ScRefCellValue aCellA2(*pDoc, ScAddress(0, 1, 0));
    OUString aCellStyleA2 = pFormatA2->GetCellStyle(aCellA2, ScAddress(0, 1, 0));
    CPPUNIT_ASSERT(!aCellStyleA2.isEmpty());

    ScRefCellValue aCellA3(*pDoc, ScAddress(0, 2, 0));
    OUString aCellStyleA3 = pFormatA3->GetCellStyle(aCellA3, ScAddress(0, 2, 0));
    CPPUNIT_ASSERT(!aCellStyleA3.isEmpty());

    ScRefCellValue aCellA4(*pDoc, ScAddress(0, 3, 0));
    OUString aCellStyleA4 = pFormatA4->GetCellStyle(aCellA4, ScAddress(0, 3, 0));
    CPPUNIT_ASSERT(!aCellStyleA4.isEmpty());
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testCondFormatCfvoScaleValueXLSX)
{
    createScDoc("xlsx/condformat_databar.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Databar, pEntry->GetType());
    const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pEntry);
    const ScDataBarFormatData* pDataBarFormatData = pDataBar->GetDataBarData();
    const ScColorScaleEntry* pLower = pDataBarFormatData->mpLowerLimit.get();
    const ScColorScaleEntry* pUpper = pDataBarFormatData->mpUpperLimit.get();

    CPPUNIT_ASSERT_EQUAL(COLORSCALE_VALUE, pLower->GetType());
    CPPUNIT_ASSERT_EQUAL(COLORSCALE_VALUE, pUpper->GetType());

    CPPUNIT_ASSERT_EQUAL(0.0, pLower->GetValue());
    CPPUNIT_ASSERT_EQUAL(1.0, pUpper->GetValue());
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testNumberFormatODS)
{
    createScDoc("ods/testNumberFormats.ods");
    saveAndReload(u"calc8"_ustr);
    ScDocument* pDoc = getScDoc();
    sal_uInt32 nNumberFormat;
    const sal_Int32 nCountFormats = 18;
    const OUString aExpectedFormatStr[nCountFormats]
        = { u"\"format=\"000000"_ustr,        u"\"format=\"??????"_ustr,
            u"\"format=\"??0000"_ustr,        u"\"format=\"000,000"_ustr,
            u"\"format=\"???,???"_ustr,       u"\"format=\"??0,000"_ustr,
            u"\"format=\"000\" \"?/?"_ustr,   u"\"format=\"???\" \"?/?"_ustr,
            u"\"format=\"?00\" \"?/?"_ustr,   u"\"format=\"0,000\" \"?/?"_ustr,
            u"\"format=\"?,???\" \"?/?"_ustr, u"\"format=\"?,?00\" \"?/?"_ustr,
            u"\"format=\"0.000E+00"_ustr,     u"\"format=\"?.###E+00"_ustr,
            u"\"format=\"?.0##E+00"_ustr,     u"\"format=\"000E+00"_ustr,
            u"\"format=\"???E+00"_ustr,       u"\"format=\"?00E+00"_ustr };
    for (sal_Int32 i = 0; i < nCountFormats; i++)
    {
        nNumberFormat = pDoc->GetNumberFormat(i + 1, 2, 0);
        const SvNumberformat* pNumberFormat = pDoc->GetFormatTable()->GetEntry(nNumberFormat);
        const OUString& rFormatStr = pNumberFormat->GetFormatstring();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format modified during export/import",
                                     aExpectedFormatStr[i], rFormatStr);
    }
    OUString aCSVPath = createFilePath(u"contentCSV/testNumberFormats.csv");
    testCondFile(aCSVPath, pDoc, 0,
                 false); // comma is thousand separator and cannot be used as delimiter
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testConditionalFormatRangeListXLSX)
{
    createScDoc("ods/conditionalformat_rangelist.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "//x:conditionalFormatting", "sqref", u"F4 F10");
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testConditionalFormatContainsTextXLSX)
{
    createScDoc("ods/conditionalformat_containstext.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPathContent(pDoc, "//x:conditionalFormatting/x:cfRule/x:formula",
                       u"NOT(ISERROR(SEARCH(\"test\",A1)))");
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testConditionalFormatPriorityCheckXLSX)
{
    createScDoc("xlsx/conditional_fmt_checkpriority.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    constexpr bool bHighPriorityExtensionA1
        = true; // Should A1's extension cfRule has higher priority than normal cfRule ?
    constexpr bool bHighPriorityExtensionA3
        = false; // Should A3's extension cfRule has higher priority than normal cfRule ?
    size_t nA1NormalPriority = 0;
    size_t nA1ExtPriority = 0;
    size_t nA3NormalPriority = 0;
    size_t nA3ExtPriority = 0;
    for (size_t nIdx = 1; nIdx <= 2; ++nIdx)
    {
        OString aIdx = OString::number(nIdx);
        OUString aCellAddr = getXPath(pDoc, "//x:conditionalFormatting[" + aIdx + "]", "sqref");
        OUString aPriority
            = getXPath(pDoc, "//x:conditionalFormatting[" + aIdx + "]/x:cfRule", "priority");
        CPPUNIT_ASSERT_MESSAGE("conditionalFormatting sqref must be either A1 or A3",
                               aCellAddr == "A1" || aCellAddr == "A3");
        if (aCellAddr == "A1")
            nA1NormalPriority = aPriority.toUInt32();
        else
            nA3NormalPriority = aPriority.toUInt32();
        aCellAddr = getXPathContent(
            pDoc, "//x:extLst/x:ext[1]/x14:conditionalFormattings/x14:conditionalFormatting[" + aIdx
                      + "]/xm:sqref");
        aPriority
            = getXPath(pDoc,
                       "//x:extLst/x:ext[1]/x14:conditionalFormattings/x14:conditionalFormatting["
                           + aIdx + "]/x14:cfRule",
                       "priority");
        CPPUNIT_ASSERT_MESSAGE("x14:conditionalFormatting sqref must be either A1 or A3",
                               aCellAddr == "A1" || aCellAddr == "A3");
        if (aCellAddr == "A1")
            nA1ExtPriority = aPriority.toUInt32();
        else
            nA3ExtPriority = aPriority.toUInt32();
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong priorities for A1", bHighPriorityExtensionA1,
                                 nA1ExtPriority < nA1NormalPriority);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong priorities for A3", bHighPriorityExtensionA3,
                                 nA3ExtPriority < nA3NormalPriority);
}

CPPUNIT_TEST_FIXTURE(CondFormatTest, testConditionalFormatOriginXLSX)
{
    createScDoc("xlsx/conditional_fmt_origin.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    // tdf#124953 : The range-list is B3:C6 F1:G2, origin address in the formula should be B1, not B3.
    OUString aFormula = getXPathContent(pDoc, "//x:conditionalFormatting/x:cfRule/x:formula");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong origin address in formula",
                                 u"NOT(ISERROR(SEARCH(\"BAC\",B1)))"_ustr, aFormula);
}

// FILESAVE: XLSX export with long sheet names (length > 31 characters)
CPPUNIT_TEST_FIXTURE(CondFormatTest, testTdf79998)
{
    // check: original document has tab name > 31 characters
    createScDoc("ods/tdf79998.ods");
    ScDocument* pDoc = getScDoc();
    const std::vector<OUString> aTabNames1 = pDoc->GetAllTableNames();
    CPPUNIT_ASSERT_EQUAL(u"Utilities (FX Kurse, Kreditkarten etc)"_ustr, aTabNames1[1]);

    // check: saved XLSX document has truncated tab name
    saveAndReload(u"Calc Office Open XML"_ustr);
    pDoc = getScDoc();
    const std::vector<OUString> aTabNames2 = pDoc->GetAllTableNames();
    CPPUNIT_ASSERT_EQUAL(u"Utilities (FX Kurse, Kreditkart"_ustr, aTabNames2[1]);
}

CPPUNIT_PLUGIN_IMPLEMENT();
