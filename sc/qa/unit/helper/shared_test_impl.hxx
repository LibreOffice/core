/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_QA_UNIT_HELPER_SHARED_TEST_IMPL_HXX
#define INCLUDED_SC_QA_UNIT_HELPER_SHARED_TEST_IMPL_HXX

#include <memory>
#include <colorscale.hxx>
#include <conditio.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include "qahelper.hxx"
#include <formula/errorcodes.hxx>

struct FindCondFormatByEnclosingRange
{
    explicit FindCondFormatByEnclosingRange(const ScRange& rRange):
        mrRange(rRange) {}

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

DataBarData const aData[] = {
    { ScRange(1,2,0,1,5,0), COLORSCALE_AUTO, COLORSCALE_AUTO, databar::AUTOMATIC },
    { ScRange(3,2,0,3,5,0), COLORSCALE_MIN, COLORSCALE_MAX, databar::AUTOMATIC },
    { ScRange(5,2,0,5,5,0), COLORSCALE_PERCENTILE, COLORSCALE_PERCENT, databar::AUTOMATIC },
    { ScRange(7,2,0,7,5,0), COLORSCALE_VALUE, COLORSCALE_FORMULA, databar::AUTOMATIC },
    { ScRange(1,9,0,1,12,0), COLORSCALE_AUTO, COLORSCALE_AUTO, databar::MIDDLE }
};

void testDataBar_Impl(const ScDocument& rDoc)
{
    ScConditionalFormatList* pList = rDoc.GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    for(size_t i = 0; i < SAL_N_ELEMENTS(aData); ++i)
    {
        ScConditionalFormatList::const_iterator itr = std::find_if(pList->begin(),
                pList->end(), FindCondFormatByEnclosingRange(aData[i].aRange));
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

ColorScale2EntryData const aData2Entry[] = {
    { ScRange(1,2,0,1,5,0), COLORSCALE_MIN, COLORSCALE_MAX },
    { ScRange(3,2,0,3,5,0), COLORSCALE_PERCENTILE, COLORSCALE_PERCENT },
    { ScRange(5,2,0,5,5,0), COLORSCALE_VALUE, COLORSCALE_FORMULA }
};

void testColorScale2Entry_Impl(const ScDocument& rDoc)
{
    const ScConditionalFormatList* pList = rDoc.GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    for(size_t i = 0; i < SAL_N_ELEMENTS(aData2Entry); ++i)
    {
        ScConditionalFormatList::const_iterator itr = std::find_if(pList->begin(),
                            pList->end(), FindCondFormatByEnclosingRange(aData2Entry[i].aRange));
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

ColorScale3EntryData const aData3Entry[] = {
    { ScRange(1,1,1,1,6,1), COLORSCALE_MIN, COLORSCALE_PERCENTILE, COLORSCALE_MAX },
    { ScRange(3,1,1,3,6,1), COLORSCALE_PERCENTILE, COLORSCALE_VALUE, COLORSCALE_PERCENT },
    { ScRange(5,1,1,5,6,1), COLORSCALE_VALUE, COLORSCALE_VALUE, COLORSCALE_FORMULA }
};

void testColorScale3Entry_Impl(const ScDocument& rDoc)
{
    ScConditionalFormatList* pList = rDoc.GetCondFormList(1);
    CPPUNIT_ASSERT(pList);

    for(size_t i = 0; i < SAL_N_ELEMENTS(aData3Entry); ++i)
    {
        ScConditionalFormatList::const_iterator itr = std::find_if(pList->begin(),
                            pList->end(), FindCondFormatByEnclosingRange(aData3Entry[i].aRange));
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

void testFunctionsExcel2010_Impl( ScDocument& rDoc )
{
    // Original test case document is functions-excel-2010.xlsx
    // Which test rows to evaluate, 1-based as in UI to ease maintenance.
    static struct
    {
        SCROW nRow;
        bool  bEvaluate;
    } const aTests[] = {
        {  2, false },  // name=[ AGGREGATE ], result=0, expected=1
        {  3, true  },
        {  4, true  },
        {  5, true  },
        {  6, true  },
        {  7, true  },
        {  8, true  },
        {  9, true  },
        { 10, true  },
        { 11, true  },
        { 12, true  },
        { 13, true  },
        { 14, true  },
        { 15, true  },
        { 16, true  },
        { 17, true  },
        { 18, true  },
        { 19, true  },
        { 20, true  },
        { 21, true  },
        { 22, true  },
        { 23, true  },
        { 24, true  },
        { 25, true  },
        { 26, true  },
        { 27, true  },
        { 28, true  },
        { 29, true  },
        { 30, true  },
        { 31, true  },
        { 32, true  },
        { 33, true  },
        { 34, true  },
        { 35, true  },
        { 36, true  },
        { 37, true  },
        { 38, true  },
        { 39, true  },
        { 40, true  },
        { 41, true  },
        { 42, true  },
        { 43, true  },
        { 44, true  },
        { 45, false },  // name=[ NETWORKDAYS.INTL ], result=18, expected=19
        { 46, true  },
        { 47, true  },
        { 48, true  },
        { 49, true  },
        { 50, true  },
        { 51, true  },
        { 52, true  },
        { 53, true  },
        { 54, true  },
        { 55, true  },
        { 56, true  },
        { 57, true  },
        { 58, true  },
        { 59, true  },
        { 60, true  },
        { 61, true  },
        { 62, true  },
        { 63, true  },
        { 64, true  },
        { 65, true  },
        { 66, true  },
        { 67, true  },
        { 68, true  },
        { 69, true  },
        { 70, true  },
        { 71, true  },
        { 72, true  },
        { 73, true  },
        { 74, true  },
        { 75, true  },
        { 76, true  },
        { 77, true  },
        { 78, true  },
        { 79, false },  // name=[ WORKDAY.INTL ], result=41755 , expected=41754
        { 80, true  }
    };

    for (size_t i=0; i < SAL_N_ELEMENTS(aTests); ++i)
    {
        if (aTests[i].bEvaluate)
        {
            // Column 0 is description, 1 is formula, 2 is Excel result, 3 is
            // comparison.
            SCROW nRow = aTests[i].nRow - 1;    // 0-based

            OString aStr = OString::number( aTests[i].nRow) +
                ", function name=[ " +
                OUStringToOString( rDoc.GetString( ScAddress( 0, nRow, 0)), RTL_TEXTENCODING_UTF8 ) +
                " ], result=" +
                OString::number( rDoc.GetValue( ScAddress( 1, nRow, 0)) ) +
                ", expected=" +
                OString::number( rDoc.GetValue( ScAddress( 2, nRow, 0)) );

            ScFormulaCell* pFC = rDoc.GetFormulaCell( ScAddress( 1, nRow, 0) );
            if ( pFC && pFC->GetErrCode() != FormulaError::NONE )
                aStr += ", error code =" + OString::number( static_cast<int>(pFC->GetErrCode()) );

            CPPUNIT_ASSERT_MESSAGE( OString( "Expected a formula cell without error at row " +
                    aStr ).getStr(), isFormulaWithoutError( rDoc, ScAddress( 1, nRow, 0)));
            CPPUNIT_ASSERT_MESSAGE( OString( "Expected a TRUE value at row " +
                    aStr ).getStr(), 0 != rDoc.GetValue( ScAddress( 3, nRow, 0)));

        }
    }
}

void testCeilingFloor_Impl( ScDocument& rDoc )
{
    // Original test case document is ceiling-floor.xlsx
    // Sheet1.K1 has =AND(K3:K81) to evaluate all results.
    static const OUStringLiteral pORef = u"Sheet1.K1";
    ScAddress aPos;
    aPos.Parse(pORef, rDoc);
    ASSERT_FORMULA_EQUAL(rDoc, aPos, "AND(K3:K81)", "Wrong formula.");
    CPPUNIT_ASSERT_MESSAGE( OUString( pORef + " result is error.").toUtf8().getStr(),
            isFormulaWithoutError( rDoc, aPos));
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(aPos));
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
