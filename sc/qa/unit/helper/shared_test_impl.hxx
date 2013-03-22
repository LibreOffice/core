/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_SHARED_TEST_IMPL_HXX
#define SC_SHARED_TEST_IMPL_HXX

#include "colorscale.hxx"
#include "conditio.hxx"

struct FindCondFormatByEnclosingRange
{
    FindCondFormatByEnclosingRange(const ScRange& rRange):
        mrRange(rRange) {}

    bool operator()(const ScConditionalFormat& rFormat)
    {
        if(rFormat.GetRange().Combine() == mrRange)
            return true;

        return false;
    }

private:
    const ScRange& mrRange;
};

struct ColorScale2EntryData
{
    ScRange aRange;
    ScColorScaleEntryType eLowerType;
    ScColorScaleEntryType eUpperType;
};

ColorScale2EntryData aData2Entry[] = {
    { ScRange(1,2,0,1,5,0), COLORSCALE_MIN, COLORSCALE_MAX },
    { ScRange(3,2,0,3,5,0), COLORSCALE_PERCENTILE, COLORSCALE_PERCENT },
    { ScRange(5,2,0,5,5,0), COLORSCALE_VALUE, COLORSCALE_FORMULA }
};

void testColorScale2Entry_Impl(ScDocument* pDoc)
{
    const ScConditionalFormatList* pList = pDoc->GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    for(size_t i = 0; i < SAL_N_ELEMENTS(aData2Entry); ++i)
    {
        ScConditionalFormatList::const_iterator itr = std::find_if(pList->begin(),
                            pList->end(), FindCondFormatByEnclosingRange(aData2Entry[i].aRange));
        CPPUNIT_ASSERT(itr != pList->end());
        CPPUNIT_ASSERT_EQUAL(size_t(1), itr->size());

        const ScFormatEntry* pFormatEntry = itr->GetEntry(0);
        CPPUNIT_ASSERT_EQUAL(pFormatEntry->GetType(), condformat::COLORSCALE);
        const ScColorScaleFormat* pColFormat = static_cast<const ScColorScaleFormat*>(pFormatEntry);
        CPPUNIT_ASSERT_EQUAL(size_t(2), pColFormat->size());

        ScColorScaleFormat::const_iterator format_itr = pColFormat->begin();
        CPPUNIT_ASSERT_EQUAL(aData2Entry[i].eLowerType, format_itr->GetType());
        ++format_itr;
        CPPUNIT_ASSERT(format_itr != pColFormat->end());
        CPPUNIT_ASSERT_EQUAL(aData2Entry[i].eUpperType, format_itr->GetType());
    }
}

struct ColorScale3EntryData
{
    ScRange aRange;
    ScColorScaleEntryType eLowerType;
    ScColorScaleEntryType eMiddleType;
    ScColorScaleEntryType eUpperType;
};

ColorScale3EntryData aData3Entry[] = {
    { ScRange(1,1,1,1,6,1), COLORSCALE_MIN, COLORSCALE_PERCENTILE, COLORSCALE_MAX },
    { ScRange(3,1,1,3,6,1), COLORSCALE_PERCENTILE, COLORSCALE_VALUE, COLORSCALE_PERCENT },
    { ScRange(5,1,1,5,6,1), COLORSCALE_VALUE, COLORSCALE_VALUE, COLORSCALE_FORMULA }
};

void testColorScale3Entry_Impl(ScDocument* pDoc)
{
    ScConditionalFormatList* pList = pDoc->GetCondFormList(1);
    CPPUNIT_ASSERT(pList);

    for(size_t i = 0; i < SAL_N_ELEMENTS(aData3Entry); ++i)
    {
        ScConditionalFormatList::const_iterator itr = std::find_if(pList->begin(),
                            pList->end(), FindCondFormatByEnclosingRange(aData3Entry[i].aRange));
        CPPUNIT_ASSERT(itr != pList->end());
        CPPUNIT_ASSERT_EQUAL(size_t(1), itr->size());

        const ScFormatEntry* pFormatEntry = itr->GetEntry(0);
        CPPUNIT_ASSERT_EQUAL(pFormatEntry->GetType(), condformat::COLORSCALE);
        const ScColorScaleFormat* pColFormat = static_cast<const ScColorScaleFormat*>(pFormatEntry);
        CPPUNIT_ASSERT_EQUAL(size_t(3), pColFormat->size());

        ScColorScaleFormat::const_iterator format_itr = pColFormat->begin();
        CPPUNIT_ASSERT_EQUAL(aData3Entry[i].eLowerType, format_itr->GetType());
        ++format_itr;
        CPPUNIT_ASSERT(format_itr != pColFormat->end());
        CPPUNIT_ASSERT_EQUAL(aData3Entry[i].eMiddleType, format_itr->GetType());
        ++format_itr;
        CPPUNIT_ASSERT(format_itr != pColFormat->end());
        CPPUNIT_ASSERT_EQUAL(aData3Entry[i].eUpperType, format_itr->GetType());
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
