/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <dptabsrc.hxx>

#include <algorithm>
#include <vector>

#include <comphelper/sequence.hxx>
#include <o3tl/any.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <svl/itemprop.hxx>

#include <dpcache.hxx>
#include <dptabres.hxx>
#include <dptabdat.hxx>
#include <global.hxx>
#include <miscuno.hxx>
#include <unonames.hxx>
#include <dpitemdata.hxx>
#include <dputil.hxx>
#include <dpresfilter.hxx>
#include <calcmacros.hxx>
#include <generalfunction.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#include <com/sun/star/sheet/GeneralFunction2.hpp>
#include <com/sun/star/sheet/TableFilterField.hpp>

#include <unotools/calendarwrapper.hxx>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>

using namespace com::sun::star;
using ::std::vector;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::sheet::DataPilotFieldAutoShowInfo;

#define SC_MINCOUNT_LIMIT   1000000

SC_SIMPLE_SERVICE_INFO( ScDPSource,      u"ScDPSource"_ustr,      u"com.sun.star.sheet.DataPilotSource"_ustr )
SC_SIMPLE_SERVICE_INFO( ScDPDimensions,  u"ScDPDimensions"_ustr,  u"com.sun.star.sheet.DataPilotSourceDimensions"_ustr )
SC_SIMPLE_SERVICE_INFO( ScDPDimension,   u"ScDPDimension"_ustr,   u"com.sun.star.sheet.DataPilotSourceDimension"_ustr )

// Typos are on purpose here, quote from Eike Rathke (see https://gerrit.libreoffice.org/c/core/+/101116):
// "The typo is exactly why the SC_SIMPLE_SERVICE_INFO_COMPAT() lists both service names,
// the old with the typo and the new corrected one. Correcting the typo in the old name
// will make all extensions fail that use it. This is not to be changed."
SC_SIMPLE_SERVICE_INFO_COMPAT( ScDPHierarchies, u"ScDPHierarchies"_ustr,
        u"com.sun.star.sheet.DataPilotSourceHierarchies"_ustr, u"com.sun.star.sheet.DataPilotSourceHierarcies"_ustr )
SC_SIMPLE_SERVICE_INFO_COMPAT( ScDPHierarchy,   u"ScDPHierarchy"_ustr,
        u"com.sun.star.sheet.DataPilotSourceHierarchy"_ustr, u"com.sun.star.sheet.DataPilotSourceHierarcy"_ustr )

SC_SIMPLE_SERVICE_INFO( ScDPLevels,      u"ScDPLevels"_ustr,      u"com.sun.star.sheet.DataPilotSourceLevels"_ustr )
SC_SIMPLE_SERVICE_INFO( ScDPLevel,       u"ScDPLevel"_ustr,       u"com.sun.star.sheet.DataPilotSourceLevel"_ustr )
SC_SIMPLE_SERVICE_INFO( ScDPMembers,     u"ScDPMembers"_ustr,     u"com.sun.star.sheet.DataPilotSourceMembers"_ustr )
SC_SIMPLE_SERVICE_INFO( ScDPMember,      u"ScDPMember"_ustr,      u"com.sun.star.sheet.DataPilotSourceMember"_ustr )

// property maps for PropertySetInfo
//  DataDescription / NumberFormat are internal

//TODO: move to a header?
static bool lcl_GetBoolFromAny( const uno::Any& aAny )
{
    auto b = o3tl::tryAccess<bool>(aAny);
    return b.has_value() && *b;
}

ScDPSource::ScDPSource(ScDPTableData* pData)
    : mpData(pData)
{
    mpData->SetEmptyFlags(mbIgnoreEmptyRows, mbRepeatIfEmpty);
}

ScDPSource::~ScDPSource()
{
    // free lists

    mpColumnResults.reset();
    mpRowResults.reset();

    mpColumnResultRoot.reset();
    mpRowResultRoot.reset();
    mpResultData.reset();
}

const std::optional<OUString> & ScDPSource::GetGrandTotalName() const
{
    return mpGrandTotalName;
}

sheet::DataPilotFieldOrientation ScDPSource::GetOrientation(sal_Int32 nColumn)
{
    if (std::find(maColDims.begin(), maColDims.end(), nColumn) != maColDims.end())
        return sheet::DataPilotFieldOrientation_COLUMN;

    if (std::find(maRowDims.begin(), maRowDims.end(), nColumn) != maRowDims.end())
        return sheet::DataPilotFieldOrientation_ROW;

    if (std::find(maDataDims.begin(), maDataDims.end(), nColumn) != maDataDims.end())
        return sheet::DataPilotFieldOrientation_DATA;

    if (std::find(maPageDims.begin(), maPageDims.end(), nColumn) != maPageDims.end())
        return sheet::DataPilotFieldOrientation_PAGE;

    return sheet::DataPilotFieldOrientation_HIDDEN;
}

sal_Int32 ScDPSource::GetDataDimensionCount() const
{
    return maDataDims.size();
}

ScDPDimension* ScDPSource::GetDataDimension(sal_Int32 nIndex)
{
    if (nIndex < 0 || o3tl::make_unsigned(nIndex) >= maDataDims.size())
        return nullptr;

    sal_Int32 nDimIndex = maDataDims[nIndex];
    return GetDimensionsObject()->getByIndex(nDimIndex);
}

OUString ScDPSource::GetDataDimName(sal_Int32 nIndex)
{
    OUString aRet;
    ScDPDimension* pDim = GetDataDimension(nIndex);
    if (pDim)
        aRet = pDim->getName();
    return aRet;
}

sal_Int32 ScDPSource::GetPosition(sal_Int32 nColumn)
{
    std::vector<sal_Int32>::const_iterator it, itBeg = maColDims.begin(), itEnd = maColDims.end();
    it = std::find(itBeg, itEnd, nColumn);
    if (it != itEnd)
        return std::distance(itBeg, it);

    itBeg = maRowDims.begin();
    itEnd = maRowDims.end();
    it = std::find(itBeg, itEnd, nColumn);
    if (it != itEnd)
        return std::distance(itBeg, it);

    itBeg = maDataDims.begin();
    itEnd = maDataDims.end();
    it = std::find(itBeg, itEnd, nColumn);
    if (it != itEnd)
        return std::distance(itBeg, it);

    itBeg = maPageDims.begin();
    itEnd = maPageDims.end();
    it = std::find(itBeg, itEnd, nColumn);
    if (it != itEnd)
        return std::distance(itBeg, it);

    return 0;
}

namespace {

bool testSubTotal( bool& rAllowed, sal_Int32 nColumn, const std::vector<sal_Int32>& rDims, ScDPSource* pSource )
{
    rAllowed = true;
    std::vector<sal_Int32>::const_iterator it = rDims.begin(), itEnd = rDims.end();
    for (; it != itEnd; ++it)
    {
        if (*it != nColumn)
            continue;

        if ( pSource->IsDataLayoutDimension(nColumn) )
        {
            //  no subtotals for data layout dim, no matter where
            rAllowed = false;
            return true;
        }

        //  no subtotals if no other dim but data layout follows
        ++it;
        if (it != itEnd && pSource->IsDataLayoutDimension(*it))
            ++it;
        if (it == itEnd)
            rAllowed = false;

        return true;    // found
    }

    return false;
}

void removeDim( sal_Int32 nRemove, std::vector<sal_Int32>& rDims )
{
    std::vector<sal_Int32>::iterator it = std::find(rDims.begin(), rDims.end(), nRemove);
    if (it != rDims.end())
        rDims.erase(it);
}

}

bool ScDPSource::SubTotalAllowed(sal_Int32 nColumn)
{
    //TODO: cache this at ScDPResultData
    bool bAllowed = true;
    if ( testSubTotal(bAllowed, nColumn, maColDims, this) )
        return bAllowed;
    if ( testSubTotal(bAllowed, nColumn, maRowDims, this) )
        return bAllowed;
    return bAllowed;
}

void ScDPSource::SetOrientation(sal_Int32 nColumn, sheet::DataPilotFieldOrientation nNew)
{
    //TODO: change to no-op if new orientation is equal to old?

    // remove from old list
    removeDim(nColumn, maColDims);
    removeDim(nColumn, maRowDims);
    removeDim(nColumn, maDataDims);
    removeDim(nColumn, maPageDims);

    // add to new list
    switch (nNew)
    {
        case sheet::DataPilotFieldOrientation_COLUMN:
            maColDims.push_back(nColumn);
            break;
        case sheet::DataPilotFieldOrientation_ROW:
            maRowDims.push_back(nColumn);
            break;
        case sheet::DataPilotFieldOrientation_DATA:
            maDataDims.push_back(nColumn);
            break;
        case sheet::DataPilotFieldOrientation_PAGE:
            maPageDims.push_back(nColumn);
            break;
            // DataPilot Migration - Cache&&Performance
        case sheet::DataPilotFieldOrientation_HIDDEN:
            break;
        default:
            OSL_FAIL( "ScDPSource::SetOrientation: unexpected orientation" );
            break;
    }
}

bool ScDPSource::IsDataLayoutDimension(sal_Int32 nDim)
{
    return nDim == mpData->GetColumnCount();
}

sheet::DataPilotFieldOrientation ScDPSource::GetDataLayoutOrientation()
{
    return GetOrientation(mpData->GetColumnCount());
}

bool ScDPSource::IsDateDimension(sal_Int32 nDim)
{
    return mpData->IsDateDimension(nDim);
}

ScDPDimensions* ScDPSource::GetDimensionsObject()
{
    if (!mpDimensions.is())
        mpDimensions = new ScDPDimensions(this);
    return mpDimensions.get();
}

uno::Reference<container::XNameAccess> SAL_CALL ScDPSource::getDimensions()
{
    return GetDimensionsObject();
}

void ScDPSource::SetDupCount( tools::Long nNew )
{
    mnDupCount = nNew;
}

ScDPDimension* ScDPSource::AddDuplicated(std::u16string_view rNewName)
{
    OSL_ENSURE(mpDimensions.is(), "AddDuplicated without dimensions?");

    //  re-use

    tools::Long nOldDimCount = mpDimensions->getCount();
    for (tools::Long i=0; i<nOldDimCount; i++)
    {
        ScDPDimension* pDim = mpDimensions->getByIndex(i);
        if (pDim && pDim->getName() == rNewName)
        {
            //TODO: test if pDim is a duplicate of source
            return pDim;
        }
    }

    SetDupCount(mnDupCount + 1);
    mpDimensions->CountChanged(); // uses mnDupCount

    return mpDimensions->getByIndex(mpDimensions->getCount() - 1);
}

sal_Int32 ScDPSource::GetSourceDim(sal_Int32 nDim)
{
    //  original source dimension or data layout dimension?
    if (nDim <= mpData->GetColumnCount())
        return nDim;

    if (nDim < mpDimensions->getCount())
    {
        ScDPDimension* pDimObj = mpDimensions->getByIndex( nDim );
        if ( pDimObj )
        {
            tools::Long nSource = pDimObj->GetSourceDim();
            if ( nSource >= 0 )
                return nSource;
        }
    }

    OSL_FAIL("GetSourceDim: wrong dim");
    return nDim;
}

uno::Sequence< uno::Sequence<sheet::DataResult> > SAL_CALL ScDPSource::getResults()
{
    CreateRes_Impl(); // create mpColumnResultRoot and mpRowResultRoot

    if (mbResultOverflow) // set in CreateRes_Impl
    {
        //  no results available
        throw uno::RuntimeException();
    }

    sal_Int32 nColCount = mpColumnResultRoot->GetSize(mpResultData->GetColStartMeasure());
    sal_Int32 nRowCount = mpRowResultRoot->GetSize(mpResultData->GetRowStartMeasure());

    //  allocate full sequence
    //TODO: leave out empty rows???

    uno::Sequence< uno::Sequence<sheet::DataResult> > aSeq( nRowCount );
    uno::Sequence<sheet::DataResult>* pRowAry = aSeq.getArray();
    for (sal_Int32 nRow = 0; nRow < nRowCount; nRow++)
    {
        //  use default values of DataResult
        pRowAry[nRow] = uno::Sequence<sheet::DataResult>(nColCount);
    }

    ScDPResultFilterContext aFilterCxt;
    mpRowResultRoot->FillDataResults(
        mpColumnResultRoot.get(), aFilterCxt, aSeq, mpResultData->GetRowStartMeasure());

    maResFilterSet.swap(aFilterCxt.maFilterSet); // Keep this data for GETPIVOTDATA.

    return aSeq;
}

uno::Sequence<double> ScDPSource::getFilteredResults(
            const uno::Sequence<sheet::DataPilotFieldFilter>& aFilters )
{
    if (maResFilterSet.empty())
        getResults(); // Build result tree first.

    // Get result values from the tree.
    const ScDPResultTree::ValuesType* pVals = maResFilterSet.getResults(aFilters);
    if (pVals && !pVals->empty())
    {
        return comphelper::containerToSequence(*pVals);
    }

    if (aFilters.getLength() == 1)
    {
        // Try to get result from the leaf nodes.
        double fVal = maResFilterSet.getLeafResult(aFilters[0]);
        if (!std::isnan(fVal))
        {
            return { fVal };
        }
    }

    return uno::Sequence<double>();
}

void SAL_CALL ScDPSource::refresh()
{
    disposeData();
}

void SAL_CALL ScDPSource::addRefreshListener( const uno::Reference<util::XRefreshListener >& )
{
    OSL_FAIL("not implemented");    //TODO: exception?
}

void SAL_CALL ScDPSource::removeRefreshListener( const uno::Reference<util::XRefreshListener >& )
{
    OSL_FAIL("not implemented");    //TODO: exception?
}

Sequence< Sequence<Any> > SAL_CALL ScDPSource::getDrillDownData(const Sequence<sheet::DataPilotFieldFilter>& aFilters)
{
    sal_Int32 nColumnCount = GetData()->GetColumnCount();

    vector<ScDPFilteredCache::Criterion> aFilterCriteria;
    for (const sheet::DataPilotFieldFilter& rFilter : aFilters)
    {
        const OUString& aFieldName = rFilter.FieldName;
        for (sal_Int32 nCol = 0; nCol < nColumnCount; ++nCol)
        {
            if (aFieldName == mpData->getDimensionName(nCol))
            {
                ScDPDimension* pDim = GetDimensionsObject()->getByIndex( nCol );
                ScDPMembers* pMembers = pDim->GetHierarchiesObject()->getByIndex(0)->
                                        GetLevelsObject()->getByIndex(0)->GetMembersObject();
                sal_Int32 nIndex = pMembers->GetIndexFromName( rFilter.MatchValueName );
                if ( nIndex >= 0 )
                {
                    ScDPItemData aItem(pMembers->getByIndex(nIndex)->FillItemData());
                    aFilterCriteria.emplace_back( );
                    aFilterCriteria.back().mnFieldIndex = nCol;
                    aFilterCriteria.back().mpFilter =
                        std::make_shared<ScDPFilteredCache::SingleFilter>(aItem);
                }
            }
        }
    }

    // Take into account the visibilities of field members.
    ScDPResultVisibilityData aResVisData(this);
    mpRowResultRoot->FillVisibilityData(aResVisData);
    mpColumnResultRoot->FillVisibilityData(aResVisData);
    aResVisData.fillFieldFilters(aFilterCriteria);

    Sequence< Sequence<Any> > aTabData;
    std::unordered_set<sal_Int32> aCatDims;
    GetCategoryDimensionIndices(aCatDims);
    mpData->GetDrillDownData(std::move(aFilterCriteria), std::move(aCatDims), aTabData);
    return aTabData;
}

OUString ScDPSource::getDataDescription()
{
    CreateRes_Impl();       // create mpResultData

    OUString aRet;
    if (mpResultData->GetMeasureCount() == 1)
    {
        bool bTotalResult = false;
        aRet = mpResultData->GetMeasureString(0, true, SUBTOTAL_FUNC_NONE, bTotalResult);
    }

    //  empty for more than one measure

    return aRet;
}

void ScDPSource::setIgnoreEmptyRows(bool bSet)
{
    mbIgnoreEmptyRows = bSet;
    mpData->SetEmptyFlags(mbIgnoreEmptyRows, mbRepeatIfEmpty);
}

void ScDPSource::setRepeatIfEmpty(bool bSet)
{
    mbRepeatIfEmpty = bSet;
    mpData->SetEmptyFlags(mbIgnoreEmptyRows, mbRepeatIfEmpty);
}

void ScDPSource::disposeData()
{
    maResFilterSet.clear();

    if (mpResultData)
    {
        //  reset all data...

        mpColumnResultRoot.reset();
        mpRowResultRoot.reset();
        mpResultData.reset();
        mpColumnResults.reset();
        mpRowResults.reset();
        maColumnLevelList.clear();
        maRowLevelList.clear();
    }

    mpDimensions.clear(); // settings have to be applied (from SaveData) again!
    SetDupCount( 0 );

    maColDims.clear();
    maRowDims.clear();
    maDataDims.clear();
    maPageDims.clear();

    mpData->DisposeData();   // cached entries etc.
    mbPageFiltered = false;
    mbResultOverflow = false;
}

static tools::Long lcl_CountMinMembers(const vector<ScDPDimension*>& ppDim, const vector<ScDPLevel*>& ppLevel, tools::Long nLevels )
{
    //  Calculate the product of the member count for those consecutive levels that
    //  have the "show all" flag, one following level, and the data layout dimension.

    tools::Long nTotal = 1;
    tools::Long nDataCount = 1;
    bool bWasShowAll = true;
    tools::Long nPos = nLevels;
    while ( nPos > 0 )
    {
        --nPos;

        if ( nPos+1 < nLevels && ppDim[nPos] == ppDim[nPos+1] )
        {
            OSL_FAIL("lcl_CountMinMembers: multiple levels from one dimension not implemented");
            return 0;
        }

        bool bDo = false;
        if ( ppDim[nPos]->getIsDataLayoutDimension() )
        {
            //  data layout dim doesn't interfere with "show all" flags
            nDataCount = ppLevel[nPos]->GetMembersObject()->getCount();
            if ( nDataCount == 0 )
                nDataCount = 1;
        }
        else if ( bWasShowAll )     // "show all" set for all following levels?
        {
            bDo = true;
            if ( !ppLevel[nPos]->getShowEmpty() )
            {
                //  this level is counted, following ones are not
                bWasShowAll = false;
            }
        }
        if ( bDo )
        {
            tools::Long nThisCount = ppLevel[nPos]->GetMembersObject()->getMinMembers();
            if ( nThisCount == 0 )
            {
                nTotal = 1;         //  empty level -> start counting from here
                                    //TODO: start with visible elements in this level?
            }
            else
            {
                if ( nTotal >= LONG_MAX / nThisCount )
                    return LONG_MAX;                        //  overflow
                nTotal *= nThisCount;
            }
        }
    }

    //  always include data layout dim, even after restarting
    if ( nTotal >= LONG_MAX / nDataCount )
        return LONG_MAX;                        //  overflow
    nTotal *= nDataCount;

    return nTotal;
}

void ScDPSource::FillCalcInfo(bool bIsRow, ScDPTableData::CalcInfo& rInfo, bool &rHasAutoShow)
{
    const std::vector<sal_Int32>& rDims = bIsRow ? maRowDims : maColDims;
    for (const auto& rDimIndex : rDims)
    {
        ScDPDimension* pDim = GetDimensionsObject()->getByIndex(rDimIndex);
        tools::Long nHierarchy = ScDPDimension::getUsedHierarchy();
        if ( nHierarchy >= ScDPHierarchies::getCount() )
            nHierarchy = 0;
        ScDPLevels* pLevels = pDim->GetHierarchiesObject()->getByIndex(nHierarchy)->GetLevelsObject();
        sal_Int32 nCount = pLevels->getCount();

        //TODO: Test
        if (pDim->getIsDataLayoutDimension() && maDataDims.size() < 2)
            nCount = 0;
        //TODO: Test

        for (sal_Int32 j = 0; j < nCount; ++j)
        {
            ScDPLevel* pLevel = pLevels->getByIndex(j);
            pLevel->EvaluateSortOrder();

            // no layout flags for column fields, only for row fields
            pLevel->SetEnableLayout( bIsRow );

            if ( pLevel->GetAutoShow().IsEnabled )
                rHasAutoShow = true;

            if (bIsRow)
            {
                rInfo.aRowLevelDims.push_back(rDimIndex);
                rInfo.aRowDims.push_back(pDim);
                rInfo.aRowLevels.push_back(pLevel);
            }
            else
            {
                rInfo.aColLevelDims.push_back(rDimIndex);
                rInfo.aColDims.push_back(pDim);
                rInfo.aColLevels.push_back(pLevel);
            }

            pLevel->GetMembersObject();                 // initialize for groups
        }
    }
}

namespace {

class CategoryDimInserter
{
    ScDPSource& mrSource;
    std::unordered_set<sal_Int32>& mrCatDims;
public:
    CategoryDimInserter(ScDPSource& rSource, std::unordered_set<sal_Int32>& rCatDims) :
        mrSource(rSource),
        mrCatDims(rCatDims) {}

    void operator() (tools::Long nDim)
    {
        if (!mrSource.IsDataLayoutDimension(nDim))
            mrCatDims.insert(nDim);
    }
};

}

void ScDPSource::GetCategoryDimensionIndices(std::unordered_set<sal_Int32>& rCatDims)
{
    std::unordered_set<sal_Int32> aCatDims;

    CategoryDimInserter aInserter(*this, aCatDims);
    std::for_each(maColDims.begin(), maColDims.end(), aInserter);
    std::for_each(maRowDims.begin(), maRowDims.end(), aInserter);
    std::for_each(maPageDims.begin(), maPageDims.end(), aInserter);

    rCatDims.swap(aCatDims);
}

void ScDPSource::FilterCacheByPageDimensions()
{
    // #i117661# Repeated calls to ScDPFilteredCache::filterByPageDimension
    // are invalid because rows are only hidden, never shown again. If
    // FilterCacheByPageDimensions is called again, the cache table must
    // be re-initialized. Currently, CreateRes_Impl always uses a fresh cache
    // because ScDBDocFunc::DataPilotUpdate calls InvalidateData.

    if (mbPageFiltered)
    {
        SAL_WARN( "sc.core","tried to apply page field filters several times");

        mpData->DisposeData();
        mpData->CreateCacheTable();  // re-initialize the cache table
        mbPageFiltered = false;
    }

    // filter table by page dimensions.
    vector<ScDPFilteredCache::Criterion> aCriteria;
    for (const auto& rDimIndex : maPageDims)
    {
        ScDPDimension* pDim = GetDimensionsObject()->getByIndex(rDimIndex);
        tools::Long nField = pDim->GetDimension();

        ScDPMembers* pMems = pDim->GetHierarchiesObject()->getByIndex(0)->
            GetLevelsObject()->getByIndex(0)->GetMembersObject();

        tools::Long nMemCount = pMems->getCount();
        ScDPFilteredCache::Criterion aFilter;
        aFilter.mnFieldIndex = static_cast<sal_Int32>(nField);
        aFilter.mpFilter = std::make_shared<ScDPFilteredCache::GroupFilter>();
        ScDPFilteredCache::GroupFilter* pGrpFilter =
            static_cast<ScDPFilteredCache::GroupFilter*>(aFilter.mpFilter.get());
        for (tools::Long j = 0; j < nMemCount; ++j)
        {
            ScDPMember* pMem = pMems->getByIndex(j);
            if (pMem->isVisible())
            {
                ScDPItemData aData(pMem->FillItemData());
                pGrpFilter->addMatchItem(aData);
            }
        }
        if (pGrpFilter->getMatchItemCount() < o3tl::make_unsigned(nMemCount))
            // there is at least one invisible item.  Add this filter criterion to the mix.
            aCriteria.push_back(aFilter);

        if (!pDim->HasSelectedPage())
            continue;

        const ScDPItemData& rData = pDim->GetSelectedData();
        aCriteria.emplace_back();
        ScDPFilteredCache::Criterion& r = aCriteria.back();
        r.mnFieldIndex = static_cast<sal_Int32>(nField);
        r.mpFilter = std::make_shared<ScDPFilteredCache::SingleFilter>(rData);
    }
    if (!aCriteria.empty())
    {
        std::unordered_set<sal_Int32> aCatDims;
        GetCategoryDimensionIndices(aCatDims);
        mpData->FilterCacheTable(std::move(aCriteria), std::move(aCatDims));
        mbPageFiltered = true;
    }
}

void ScDPSource::CreateRes_Impl()
{
    if (mpResultData)
        return;

    sheet::DataPilotFieldOrientation nDataOrient = GetDataLayoutOrientation();
    if (maDataDims.size() > 1 && ( nDataOrient != sheet::DataPilotFieldOrientation_COLUMN &&
                                nDataOrient != sheet::DataPilotFieldOrientation_ROW ) )
    {
        //  if more than one data dimension, data layout orientation must be set
        SetOrientation(mpData->GetColumnCount(), sheet::DataPilotFieldOrientation_ROW);
        nDataOrient = sheet::DataPilotFieldOrientation_ROW;
    }

    // TODO: Aggregate pDataNames, pDataRefValues, nDataRefOrient, and
    // eDataFunctions into a structure and use vector instead of static
    // or pointer arrays.
    vector<OUString> aDataNames;
    vector<sheet::DataPilotFieldReference> aDataRefValues;
    vector<ScSubTotalFunc> aDataFunctions;
    vector<sheet::DataPilotFieldOrientation> aDataRefOrient;

    ScDPTableData::CalcInfo aInfo;

    //  LateInit (initialize only those rows/children that are used) can be used unless
    //  any data dimension needs reference values from column/row dimensions
    bool bLateInit = true;

    // Go through all data dimensions (i.e. fields) and build their meta data
    // so that they can be passed on to ScDPResultData instance later.
    // TODO: aggregate all of data dimension info into a structure.
    for (const tools::Long nDimIndex : maDataDims)
    {
        // Get function for each data field.
        ScDPDimension* pDim = GetDimensionsObject()->getByIndex(nDimIndex);
        ScGeneralFunction eUser = pDim->getFunction();
        if (eUser == ScGeneralFunction::AUTO)
        {
            //TODO: test for numeric data
            eUser = ScGeneralFunction::SUM;
        }

        // Map UNO's enum to internal enum ScSubTotalFunc.
        aDataFunctions.push_back(ScDPUtil::toSubTotalFunc(eUser));

        // Get reference field/item information.
        aDataRefValues.push_back(pDim->GetReferenceValue());
        sheet::DataPilotFieldOrientation nDataRefOrient = sheet::DataPilotFieldOrientation_HIDDEN;    // default if not used
        sal_Int32 eRefType = aDataRefValues.back().ReferenceType;
        if ( eRefType == sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE ||
             eRefType == sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE ||
             eRefType == sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE ||
             eRefType == sheet::DataPilotFieldReferenceType::RUNNING_TOTAL )
        {
            sal_Int32 nColumn = comphelper::findValue(
                GetDimensionsObject()->getElementNames(), aDataRefValues.back().ReferenceField);
            if ( nColumn >= 0 )
            {
                nDataRefOrient = GetOrientation(nColumn);
                //  need fully initialized results to find reference values
                //  (both in column or row dimensions), so updated values or
                //  differences to 0 can be displayed even for empty results.
                bLateInit = false;
            }
        }

        aDataRefOrient.push_back(nDataRefOrient);

        aDataNames.push_back(pDim->getName());

        //TODO: modify user visible strings as in ScDPResultData::GetMeasureString instead!

        aDataNames.back() = ScDPUtil::getSourceDimensionName(aDataNames.back());

        //TODO: if the name is overridden by user, a flag must be set
        //TODO: so the user defined name replaces the function string and field name.

        //TODO: the complete name (function and field) must be stored at the dimension

        tools::Long nSource = pDim->GetSourceDim();
        if (nSource >= 0)
            aInfo.aDataSrcCols.push_back(nSource);
        else
            aInfo.aDataSrcCols.push_back(nDimIndex);
    }

    mpResultData.reset( new ScDPResultData(*this) );
    mpResultData->SetMeasureData(aDataFunctions, aDataRefValues, aDataRefOrient, aDataNames);
    mpResultData->SetDataLayoutOrientation(nDataOrient);
    mpResultData->SetLateInit( bLateInit );

    bool bHasAutoShow = false;

    ScDPInitState aInitState;

    // Page field selections restrict the members shown in related fields
    // (both in column and row fields). aInitState is filled with the page
    // field selections, they are kept across the data iterator loop.

    for (const auto& rDimIndex : maPageDims)
    {
        ScDPDimension* pDim = GetDimensionsObject()->getByIndex(rDimIndex);
        if ( pDim->HasSelectedPage() )
            aInitState.AddMember(rDimIndex, GetCache()->GetIdByItemData(rDimIndex, pDim->GetSelectedData()));
    }

    // Show grand total columns only when the option is set *and* there is at
    // least one column field.  Same for the grand total rows.
    sheet::DataPilotFieldOrientation nDataLayoutOrient = GetDataLayoutOrientation();
    tools::Long nColDimCount2 = maColDims.size() - (nDataLayoutOrient == sheet::DataPilotFieldOrientation_COLUMN ? 1 : 0);
    tools::Long nRowDimCount2 = maRowDims.size() - (nDataLayoutOrient == sheet::DataPilotFieldOrientation_ROW ? 1 : 0);
    bool bShowColGrand = mbColumnGrand && nColDimCount2 > 0;
    bool bShowRowGrand = mbRowGrand && nRowDimCount2 > 0;
    mpColumnResultRoot.reset( new ScDPResultMember(mpResultData.get(), bShowColGrand) );
    mpRowResultRoot.reset( new ScDPResultMember(mpResultData.get(), bShowRowGrand) );

    FillCalcInfo(false, aInfo, bHasAutoShow);
    tools::Long nColLevelCount = aInfo.aColLevels.size();

    mpColumnResultRoot->InitFrom( aInfo.aColDims, aInfo.aColLevels, 0, aInitState );
    mpColumnResultRoot->SetHasElements();

    FillCalcInfo(true, aInfo, bHasAutoShow);
    tools::Long nRowLevelCount = aInfo.aRowLevels.size();

    if ( nRowLevelCount > 0 )
    {
        // disable layout flags for the innermost row field (level)
        aInfo.aRowLevels[nRowLevelCount-1]->SetEnableLayout( false );
    }

    mpRowResultRoot->InitFrom( aInfo.aRowDims, aInfo.aRowLevels, 0, aInitState );
    mpRowResultRoot->SetHasElements();

    // initialize members object also for all page dimensions (needed for numeric groups)
    for (const auto& rDimIndex : maPageDims)
    {
        ScDPDimension* pDim = GetDimensionsObject()->getByIndex(rDimIndex);
        tools::Long nHierarchy = ScDPDimension::getUsedHierarchy();
        if ( nHierarchy >= ScDPHierarchies::getCount() )
            nHierarchy = 0;

        ScDPLevels* pLevels = pDim->GetHierarchiesObject()->getByIndex(nHierarchy)->GetLevelsObject();
        tools::Long nCount = pLevels->getCount();
        for (tools::Long j=0; j<nCount; j++)
            pLevels->getByIndex(j)->GetMembersObject();             // initialize for groups
    }

    //  pre-check: calculate minimum number of result columns / rows from
    //  levels that have the "show all" flag set

    tools::Long nMinColMembers = lcl_CountMinMembers( aInfo.aColDims, aInfo.aColLevels, nColLevelCount );
    tools::Long nMinRowMembers = lcl_CountMinMembers( aInfo.aRowDims, aInfo.aRowLevels, nRowLevelCount );

    if ( nMinColMembers > MAXCOLCOUNT/*SC_MINCOUNT_LIMIT*/ || nMinRowMembers > SC_MINCOUNT_LIMIT )
    {
        //  resulting table is too big -> abort before calculating
        //  (this relies on late init, so no members are allocated in InitFrom above)

        mbResultOverflow = true;
        return;
    }

    FilterCacheByPageDimensions();

    aInfo.aPageDims  = maPageDims;
    aInfo.pInitState = &aInitState;
    aInfo.pColRoot   = mpColumnResultRoot.get();
    aInfo.pRowRoot   = mpRowResultRoot.get();
    mpData->CalcResults(aInfo, false);

    mpColumnResultRoot->CheckShowEmpty();
    mpRowResultRoot->CheckShowEmpty();

    //  With all data processed, calculate the final results:

    //  UpdateDataResults calculates all original results from the collected values,
    //  and stores them as reference values if needed.
    mpRowResultRoot->UpdateDataResults(mpColumnResultRoot.get(), mpResultData->GetRowStartMeasure());

    if ( bHasAutoShow )     // do the double calculation only if AutoShow is used
    {
        //  Find the desired members and set bAutoHidden flag for the others
        mpRowResultRoot->DoAutoShow(mpColumnResultRoot.get());

        //  Reset all results to empty, so they can be built again with data for the
        //  desired members only.
        mpColumnResultRoot->ResetResults();
        mpRowResultRoot->ResetResults();
        mpData->CalcResults(aInfo, true);

        //  Call UpdateDataResults again, with the new (limited) values.
        mpRowResultRoot->UpdateDataResults(mpColumnResultRoot.get(), mpResultData->GetRowStartMeasure());
    }

    //  SortMembers does the sorting by a result dimension, using the original results,
    //  but not running totals etc.
    mpRowResultRoot->SortMembers(mpColumnResultRoot.get());

    //  UpdateRunningTotals calculates running totals along column/row dimensions,
    //  differences from other members (named or relative), and column/row percentages
    //  or index values.
    //  Running totals and relative differences need to be done using the sorted values.
    //  Column/row percentages and index values must be done after sorting, because the
    //  results may no longer be in the right order (row total for percentage of row is
    //  always 1).
    ScDPRunningTotalState aRunning(mpColumnResultRoot.get(), mpRowResultRoot.get());
    ScDPRowTotals aTotals;
    mpRowResultRoot->UpdateRunningTotals(mpColumnResultRoot.get(), mpResultData->GetRowStartMeasure(), aRunning, aTotals);

#if DUMP_PIVOT_TABLE
    DumpResults();
#endif
}

void ScDPSource::FillLevelList( sheet::DataPilotFieldOrientation nOrientation, std::vector<ScDPLevel*> &rList )
{
    rList.clear();

    std::vector<sal_Int32>* pDimIndex = nullptr;
    switch (nOrientation)
    {
        case sheet::DataPilotFieldOrientation_COLUMN:
            pDimIndex = &maColDims;
            break;
        case sheet::DataPilotFieldOrientation_ROW:
            pDimIndex = &maRowDims;
            break;
        case sheet::DataPilotFieldOrientation_DATA:
            pDimIndex = &maDataDims;
            break;
        case sheet::DataPilotFieldOrientation_PAGE:
            pDimIndex = &maPageDims;
            break;
        default:
            OSL_FAIL( "ScDPSource::FillLevelList: unexpected orientation" );
            break;
    }
    if (!pDimIndex)
    {
        OSL_FAIL("invalid orientation");
        return;
    }

    ScDPDimensions* pDims = GetDimensionsObject();
    for (const auto& rIndex : *pDimIndex)
    {
        ScDPDimension* pDim = pDims->getByIndex(rIndex);
        OSL_ENSURE( pDim->getOrientation() == nOrientation, "orientations are wrong" );

        ScDPHierarchies* pHiers = pDim->GetHierarchiesObject();
        sal_Int32 nHierarchy = ScDPDimension::getUsedHierarchy();
        if ( nHierarchy >= ScDPHierarchies::getCount() )
            nHierarchy = 0;
        ScDPHierarchy* pHier = pHiers->getByIndex(nHierarchy);
        ScDPLevels* pLevels = pHier->GetLevelsObject();
        sal_Int32 nLevCount = pLevels->getCount();
        for (sal_Int32 nLev=0; nLev<nLevCount; nLev++)
        {
            ScDPLevel* pLevel = pLevels->getByIndex(nLev);
            rList.push_back(pLevel);
        }
    }
}

void ScDPSource::FillMemberResults()
{
    if (mpColumnResults || mpRowResults)
        return;

    CreateRes_Impl();

    if (mbResultOverflow) // set in CreateRes_Impl
    {
        //  no results available -> abort (leave empty)
        //  exception is thrown in ScDPSource::getResults
        return;
    }

    FillLevelList(sheet::DataPilotFieldOrientation_COLUMN, maColumnLevelList);
    sal_Int32 nColLevelCount = maColumnLevelList.size();
    if (nColLevelCount)
    {
        tools::Long nColDimSize = mpColumnResultRoot->GetSize(mpResultData->GetColStartMeasure());
        mpColumnResults.reset(new uno::Sequence<sheet::MemberResult>[nColLevelCount]);
        for (tools::Long i=0; i<nColLevelCount; i++)
            mpColumnResults[i].realloc(nColDimSize);

        tools::Long nPos = 0;
        mpColumnResultRoot->FillMemberResults(mpColumnResults.get(), nPos, mpResultData->GetColStartMeasure(),
                                              true, nullptr, nullptr);
    }

    FillLevelList(sheet::DataPilotFieldOrientation_ROW, maRowLevelList);
    tools::Long nRowLevelCount = maRowLevelList.size();
    if (nRowLevelCount)
    {
        tools::Long nRowDimSize = mpRowResultRoot->GetSize(mpResultData->GetRowStartMeasure());
        mpRowResults.reset( new uno::Sequence<sheet::MemberResult>[nRowLevelCount] );
        for (tools::Long i=0; i<nRowLevelCount; i++)
            mpRowResults[i].realloc(nRowDimSize);

        tools::Long nPos = 0;
        mpRowResultRoot->FillMemberResults(mpRowResults.get(), nPos, mpResultData->GetRowStartMeasure(),
                                           true, nullptr, nullptr);
    }
}

const uno::Sequence<sheet::MemberResult>* ScDPSource::GetMemberResults( const ScDPLevel* pLevel )
{
    FillMemberResults();

    sal_Int32 i = 0;
    sal_Int32 nColCount = maColumnLevelList.size();
    for (i=0; i<nColCount; i++)
    {
        ScDPLevel* pColLevel = maColumnLevelList[i];
        if ( pColLevel == pLevel )
            return &mpColumnResults[i];
    }
    sal_Int32 nRowCount = maRowLevelList.size();
    for (i=0; i<nRowCount; i++)
    {
        ScDPLevel* pRowLevel = maRowLevelList[i];
        if ( pRowLevel == pLevel )
            return &mpRowResults[i];
    }
    return nullptr;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDPSource::getPropertySetInfo()
{
    using beans::PropertyAttribute::READONLY;

    static const SfxItemPropertyMapEntry aDPSourceMap_Impl[] =
    {
        { SC_UNO_DP_COLGRAND, 0,  cppu::UnoType<bool>::get(),              0, 0 },
        { SC_UNO_DP_DATADESC, 0,  cppu::UnoType<OUString>::get(),    beans::PropertyAttribute::READONLY, 0 },
        { SC_UNO_DP_IGNOREEMPTY, 0,  cppu::UnoType<bool>::get(),              0, 0 },     // for sheet data only
        { SC_UNO_DP_REPEATEMPTY, 0,  cppu::UnoType<bool>::get(),              0, 0 },     // for sheet data only
        { SC_UNO_DP_ROWGRAND, 0,  cppu::UnoType<bool>::get(),              0, 0 },
        { SC_UNO_DP_ROWFIELDCOUNT,    0, cppu::UnoType<sal_Int32>::get(), READONLY, 0 },
        { SC_UNO_DP_COLUMNFIELDCOUNT, 0, cppu::UnoType<sal_Int32>::get(), READONLY, 0 },
        { SC_UNO_DP_DATAFIELDCOUNT,   0, cppu::UnoType<sal_Int32>::get(), READONLY, 0 },
        { SC_UNO_DP_GRANDTOTAL_NAME,  0, cppu::UnoType<OUString>::get(), 0, 0 },
    };
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aDPSourceMap_Impl );
    return aRef;
}

void SAL_CALL ScDPSource::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    if (aPropertyName == SC_UNO_DP_COLGRAND)
        mbColumnGrand = lcl_GetBoolFromAny(aValue);
    else if (aPropertyName == SC_UNO_DP_ROWGRAND)
        mbRowGrand = lcl_GetBoolFromAny(aValue);
    else if (aPropertyName == SC_UNO_DP_IGNOREEMPTY)
        setIgnoreEmptyRows( lcl_GetBoolFromAny( aValue ) );
    else if (aPropertyName == SC_UNO_DP_REPEATEMPTY)
        setRepeatIfEmpty( lcl_GetBoolFromAny( aValue ) );
    else if (aPropertyName == SC_UNO_DP_GRANDTOTAL_NAME)
    {
        OUString aName;
        if (aValue >>= aName)
            mpGrandTotalName = aName;
    }
    else
    {
        OSL_FAIL("unknown property");
        //TODO: THROW( UnknownPropertyException() );
    }
}

uno::Any SAL_CALL ScDPSource::getPropertyValue( const OUString& aPropertyName )
{
    uno::Any aRet;
    if ( aPropertyName == SC_UNO_DP_COLGRAND )
        aRet <<= mbColumnGrand;
    else if ( aPropertyName == SC_UNO_DP_ROWGRAND )
        aRet <<= mbRowGrand;
    else if ( aPropertyName == SC_UNO_DP_IGNOREEMPTY )
        aRet <<= mbIgnoreEmptyRows;
    else if ( aPropertyName == SC_UNO_DP_REPEATEMPTY )
        aRet <<= mbRepeatIfEmpty;
    else if ( aPropertyName == SC_UNO_DP_DATADESC )             // read-only
        aRet <<= getDataDescription();
    else if ( aPropertyName == SC_UNO_DP_ROWFIELDCOUNT )        // read-only
        aRet <<= static_cast<sal_Int32>(maRowDims.size());
    else if ( aPropertyName == SC_UNO_DP_COLUMNFIELDCOUNT )     // read-only
        aRet <<= static_cast<sal_Int32>(maColDims.size());
    else if ( aPropertyName == SC_UNO_DP_DATAFIELDCOUNT )       // read-only
        aRet <<= static_cast<sal_Int32>(maDataDims.size());
    else if (aPropertyName == SC_UNO_DP_GRANDTOTAL_NAME)
    {
        if (mpGrandTotalName)
            aRet <<= *mpGrandTotalName;
    }
    else
    {
        OSL_FAIL("unknown property");
        //TODO: THROW( UnknownPropertyException() );
    }
    return aRet;
}

#if DUMP_PIVOT_TABLE
void ScDPSource::DumpResults() const
{
    std::cout << "+++++ column root" << std::endl;
    mpColumnResultRoot->Dump(1);
    std::cout << "+++++ row root" << std::endl;
    mpRowResultRoot->Dump(1);
}
#endif

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDPSource )

ScDPDimensions::ScDPDimensions( ScDPSource* pSrc ) :
    pSource( pSrc )
{
    //TODO: hold pSource

    // include data layout dimension and duplicated dimensions
    nDimCount = pSource->GetData()->GetColumnCount() + 1 + pSource->GetDupCount();
}

ScDPDimensions::~ScDPDimensions()
{
    //TODO: release pSource
}

void ScDPDimensions::CountChanged()
{
    // include data layout dimension and duplicated dimensions
    sal_Int32 nNewCount = pSource->GetData()->GetColumnCount() + 1 + pSource->GetDupCount();
    if ( ppDims )
    {
        sal_Int32 i;
        sal_Int32 nCopy = std::min( nNewCount, nDimCount );
        rtl::Reference<ScDPDimension>* ppNew = new rtl::Reference<ScDPDimension>[nNewCount];

        for (i=0; i<nCopy; i++)             // copy existing dims
            ppNew[i] = ppDims[i];
        for (i=nCopy; i<nNewCount; i++)     // clear additional pointers
            ppNew[i] = nullptr;

        ppDims.reset( ppNew );
    }
    nDimCount = nNewCount;
}

// very simple XNameAccess implementation using getCount/getByIndex

uno::Any SAL_CALL ScDPDimensions::getByName( const OUString& aName )
{
    sal_Int32 nCount = getCount();
    for (sal_Int32 i=0; i<nCount; i++)
        if ( getByIndex(i)->getName() == aName )
        {
            uno::Reference<container::XNamed> xNamed = getByIndex(i);
            uno::Any aRet;
            aRet <<= xNamed;
            return aRet;
        }

    throw container::NoSuchElementException();
//    return uno::Any();
}

uno::Sequence<OUString> SAL_CALL ScDPDimensions::getElementNames()
{
    tools::Long nCount = getCount();
    uno::Sequence<OUString> aSeq(nCount);
    OUString* pArr = aSeq.getArray();
    for (tools::Long i=0; i<nCount; i++)
        pArr[i] = getByIndex(i)->getName();
    return aSeq;
}

sal_Bool SAL_CALL ScDPDimensions::hasByName( const OUString& aName )
{
    tools::Long nCount = getCount();
    for (tools::Long i=0; i<nCount; i++)
        if ( getByIndex(i)->getName() == aName )
            return true;
    return false;
}

uno::Type SAL_CALL ScDPDimensions::getElementType()
{
    return cppu::UnoType<container::XNamed>::get();
}

sal_Bool SAL_CALL ScDPDimensions::hasElements()
{
    return ( getCount() > 0 );
}

// end of XNameAccess implementation

tools::Long ScDPDimensions::getCount() const
{
    //  in tabular data, every column of source data is a dimension

    return nDimCount;
}

ScDPDimension* ScDPDimensions::getByIndex(tools::Long nIndex) const
{
    if ( nIndex >= 0 && nIndex < nDimCount )
    {
        if ( !ppDims )
        {
            const_cast<ScDPDimensions*>(this)->ppDims.reset(new rtl::Reference<ScDPDimension>[nDimCount] );
            for (tools::Long i=0; i<nDimCount; i++)
                ppDims[i] = nullptr;
        }
        if ( !ppDims[nIndex].is() )
        {
            ppDims[nIndex] = new ScDPDimension( pSource, nIndex );
        }

        return ppDims[nIndex].get();
    }

    return nullptr;    //TODO: exception?
}

ScDPDimension::ScDPDimension( ScDPSource* pSrc, tools::Long nD ) :
    pSource( pSrc ),
    nDim( nD ),
    nFunction( ScGeneralFunction::SUM ),     // sum is default
    nSourceDim( -1 ),
    bHasSelectedPage( false ),
    mbHasHiddenMember(false)
{
    //TODO: hold pSource
}

ScDPDimension::~ScDPDimension()
{
    //TODO: release pSource
}

ScDPHierarchies* ScDPDimension::GetHierarchiesObject()
{
    if (!mxHierarchies.is())
    {
        mxHierarchies = new ScDPHierarchies( pSource, nDim );
    }
    return mxHierarchies.get();
}

const std::optional<OUString> & ScDPDimension::GetLayoutName() const
{
    return mpLayoutName;
}

const std::optional<OUString> & ScDPDimension::GetSubtotalName() const
{
    return mpSubtotalName;
}

uno::Reference<container::XNameAccess> SAL_CALL ScDPDimension::getHierarchies()
{
    return GetHierarchiesObject();
}

OUString SAL_CALL ScDPDimension::getName()
{
    if (!aName.isEmpty())
        return aName;
    else
        return pSource->GetData()->getDimensionName( nDim );
}

void SAL_CALL ScDPDimension::setName( const OUString& rNewName )
{
    //  used after cloning
    aName = rNewName;
}

sheet::DataPilotFieldOrientation ScDPDimension::getOrientation() const
{
    return pSource->GetOrientation( nDim );
}

bool ScDPDimension::getIsDataLayoutDimension() const
{
    return pSource->GetData()->getIsDataLayoutDimension( nDim );
}

void ScDPDimension::setFunction(ScGeneralFunction nNew)
{
    nFunction = nNew;
}

ScDPDimension* ScDPDimension::CreateCloneObject()
{
    OSL_ENSURE( nSourceDim < 0, "recursive duplicate - not implemented" );

    //TODO: set new name here, or temporary name ???
    OUString aNewName = aName;

    ScDPDimension* pNew = pSource->AddDuplicated( aNewName );

    pNew->aName = aNewName;             //TODO: here or in source?
    pNew->nSourceDim = nDim;            //TODO: recursive?

    return pNew;
}

uno::Reference<util::XCloneable> SAL_CALL ScDPDimension::createClone()
{
    return CreateCloneObject();
}

const ScDPItemData& ScDPDimension::GetSelectedData()
{
    if ( !pSelectedData )
    {
        // find the named member to initialize pSelectedData from it, with name and value

        tools::Long nLevel = 0;

        tools::Long nHierarchy = getUsedHierarchy();
        if ( nHierarchy >= ScDPHierarchies::getCount() )
            nHierarchy = 0;
        ScDPLevels* pLevels = GetHierarchiesObject()->getByIndex(nHierarchy)->GetLevelsObject();
        tools::Long nLevCount = pLevels->getCount();
        if ( nLevel < nLevCount )
        {
            ScDPMembers* pMembers = pLevels->getByIndex(nLevel)->GetMembersObject();

            //TODO: merge with ScDPMembers::getByName
            tools::Long nCount = pMembers->getCount();
            for (tools::Long i=0; i<nCount && !pSelectedData; i++)
            {
                ScDPMember* pMember = pMembers->getByIndex(i);
                if (aSelectedPage == pMember->GetNameStr(false))
                {
                    pSelectedData.reset( new ScDPItemData(pMember->FillItemData()) );
                }
            }
        }

        if ( !pSelectedData )
            pSelectedData.reset( new ScDPItemData(aSelectedPage) );      // default - name only
    }

    return *pSelectedData;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDPDimension::getPropertySetInfo()
{
    static const SfxItemPropertyMapEntry aDPDimensionMap_Impl[] =
    {
        { SC_UNO_DP_FILTER,   0,  cppu::UnoType<uno::Sequence<sheet::TableFilterField>>::get(), 0, 0 },
        { SC_UNO_DP_FLAGS,    0,  cppu::UnoType<sal_Int32>::get(),                beans::PropertyAttribute::READONLY, 0 },
        { SC_UNO_DP_FUNCTION, 0,  cppu::UnoType<sheet::GeneralFunction>::get(),   0, 0 },
        { SC_UNO_DP_FUNCTION2, 0,  cppu::UnoType<sal_Int16>::get(),   0, 0 },
        { SC_UNO_DP_ISDATALAYOUT, 0,  cppu::UnoType<bool>::get(),                      beans::PropertyAttribute::READONLY, 0 },
        { SC_UNO_DP_NUMBERFO, 0,  cppu::UnoType<sal_Int32>::get(),                beans::PropertyAttribute::READONLY, 0 },
        { SC_UNO_DP_ORIENTATION, 0,  cppu::UnoType<sheet::DataPilotFieldOrientation>::get(), 0, 0 },
        { SC_UNO_DP_ORIGINAL, 0,  cppu::UnoType<container::XNamed>::get(), beans::PropertyAttribute::READONLY, 0 },
        { SC_UNO_DP_ORIGINAL_POS, 0, cppu::UnoType<sal_Int32>::get(),             0, 0 },
        { SC_UNO_DP_POSITION, 0,  cppu::UnoType<sal_Int32>::get(),                0, 0 },
        { SC_UNO_DP_REFVALUE, 0,  cppu::UnoType<sheet::DataPilotFieldReference>::get(), 0, 0 },
        { SC_UNO_DP_USEDHIERARCHY, 0,  cppu::UnoType<sal_Int32>::get(),                0, 0 },
        { SC_UNO_DP_LAYOUTNAME, 0, cppu::UnoType<OUString>::get(), 0, 0 },
        { SC_UNO_DP_FIELD_SUBTOTALNAME, 0, cppu::UnoType<OUString>::get(), 0, 0 },
        { SC_UNO_DP_HAS_HIDDEN_MEMBER, 0, cppu::UnoType<bool>::get(), 0, 0 },
    };
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aDPDimensionMap_Impl );
    return aRef;
}

void SAL_CALL ScDPDimension::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    if ( aPropertyName == SC_UNO_DP_USEDHIERARCHY )
    {
        // #i52547# don't use the incomplete date hierarchy implementation - ignore the call
    }
    else if ( aPropertyName == SC_UNO_DP_ORIENTATION )
    {
        sheet::DataPilotFieldOrientation eEnum;
        if (aValue >>= eEnum)
            pSource->SetOrientation( nDim, eEnum );
    }
    else if ( aPropertyName == SC_UNO_DP_FUNCTION )
    {
        sheet::GeneralFunction eEnum;
        if (aValue >>= eEnum)
            setFunction( static_cast<ScGeneralFunction>(eEnum) );
    }
    else if ( aPropertyName == SC_UNO_DP_FUNCTION2 )
    {
        sal_Int16 eEnum;
        if (aValue >>= eEnum)
            setFunction( static_cast<ScGeneralFunction>(eEnum) );
    }
    else if ( aPropertyName == SC_UNO_DP_REFVALUE )
        aValue >>= aReferenceValue;
    else if ( aPropertyName == SC_UNO_DP_FILTER )
    {
        bool bDone = false;
        uno::Sequence<sheet::TableFilterField> aSeq;
        if (aValue >>= aSeq)
        {
            sal_Int32 nLength = aSeq.getLength();
            if ( nLength == 0 )
            {
                aSelectedPage.clear();
                bHasSelectedPage = false;
                bDone = true;
            }
            else if ( nLength == 1 )
            {
                const sheet::TableFilterField& rField = aSeq[0];
                if ( rField.Field == 0 && rField.Operator == sheet::FilterOperator_EQUAL && !rField.IsNumeric )
                {
                    aSelectedPage = rField.StringValue;
                    bHasSelectedPage = true;
                    bDone = true;
                }
            }
        }
        if ( !bDone )
        {
            OSL_FAIL("Filter property is not a single string");
            throw lang::IllegalArgumentException();
        }
        pSelectedData.reset();       // invalid after changing aSelectedPage
    }
    else if (aPropertyName == SC_UNO_DP_LAYOUTNAME)
    {
        OUString aTmpName;
        if (aValue >>= aTmpName)
            mpLayoutName = aTmpName;
    }
    else if (aPropertyName == SC_UNO_DP_FIELD_SUBTOTALNAME)
    {
        OUString aTmpName;
        if (aValue >>= aTmpName)
            mpSubtotalName = aTmpName;
    }
    else if (aPropertyName == SC_UNO_DP_HAS_HIDDEN_MEMBER)
    {
        bool b = false;
        aValue >>= b;
        mbHasHiddenMember = b;
    }
    else
    {
        OSL_FAIL("unknown property");
        //TODO: THROW( UnknownPropertyException() );
    }
}

uno::Any SAL_CALL ScDPDimension::getPropertyValue( const OUString& aPropertyName )
{
    uno::Any aRet;
    if ( aPropertyName == SC_UNO_DP_POSITION )
        aRet <<= pSource->GetPosition( nDim );
    else if ( aPropertyName == SC_UNO_DP_USEDHIERARCHY )
        aRet <<= static_cast<sal_Int32>(getUsedHierarchy());
    else if ( aPropertyName == SC_UNO_DP_ORIENTATION )
    {
        sheet::DataPilotFieldOrientation eVal = getOrientation();
        aRet <<= eVal;
    }
    else if ( aPropertyName == SC_UNO_DP_FUNCTION )
    {
        ScGeneralFunction nVal = getFunction();
        if (nVal == ScGeneralFunction::MEDIAN)
            nVal = ScGeneralFunction::NONE;
        const int nValAsInt = static_cast<int>(nVal);
        assert(nValAsInt >= int(css::sheet::GeneralFunction_NONE) &&
               nValAsInt <= int(css::sheet::GeneralFunction_VARP));
        aRet <<= static_cast<sheet::GeneralFunction>(nValAsInt);
    }
    else if ( aPropertyName == SC_UNO_DP_FUNCTION2 )
    {
        ScGeneralFunction eVal = getFunction();
        aRet <<= static_cast<sal_Int16>(eVal);
    }
    else if ( aPropertyName == SC_UNO_DP_REFVALUE )
        aRet <<= aReferenceValue;
    else if ( aPropertyName == SC_UNO_DP_ISDATALAYOUT )                 // read-only properties
        aRet <<= getIsDataLayoutDimension();
    else if ( aPropertyName == SC_UNO_DP_NUMBERFO )
    {
        sal_Int32 nFormat = 0;
        ScGeneralFunction eFunc = getFunction();
        // #i63745# don't use source format for "count"
        if ( eFunc != ScGeneralFunction::COUNT && eFunc != ScGeneralFunction::COUNTNUMS )
            nFormat = pSource->GetData()->GetNumberFormat( ( nSourceDim >= 0 ) ? nSourceDim : nDim );

        switch ( aReferenceValue.ReferenceType )
        {
        case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE:
        case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE:
        case sheet::DataPilotFieldReferenceType::ROW_PERCENTAGE:
        case sheet::DataPilotFieldReferenceType::COLUMN_PERCENTAGE:
        case sheet::DataPilotFieldReferenceType::TOTAL_PERCENTAGE:
            nFormat = pSource->GetData()->GetNumberFormatByIdx( NF_PERCENT_DEC2 );
            break;
        case sheet::DataPilotFieldReferenceType::INDEX:
            nFormat = pSource->GetData()->GetNumberFormatByIdx( NF_NUMBER_SYSTEM );
            break;
        default:
            break;
        }

        aRet <<= nFormat;
    }
    else if ( aPropertyName == SC_UNO_DP_ORIGINAL )
    {
        uno::Reference<container::XNamed> xOriginal;
        if (nSourceDim >= 0)
            xOriginal = pSource->GetDimensionsObject()->getByIndex(nSourceDim);
        aRet <<= xOriginal;
    }
    else if (aPropertyName == SC_UNO_DP_ORIGINAL_POS)
    {
        aRet <<= nSourceDim;
    }
    else if ( aPropertyName == SC_UNO_DP_FILTER )
    {
        if ( bHasSelectedPage )
        {
            // single filter field: first field equal to selected string
            sheet::TableFilterField aField( sheet::FilterConnection_AND, 0,
                    sheet::FilterOperator_EQUAL, false, 0.0, aSelectedPage );
            aRet <<= uno::Sequence<sheet::TableFilterField>( &aField, 1 );
        }
        else
            aRet <<= uno::Sequence<sheet::TableFilterField>(0);
    }
    else if (aPropertyName == SC_UNO_DP_LAYOUTNAME)
        aRet <<= mpLayoutName ? *mpLayoutName : OUString();
    else if (aPropertyName == SC_UNO_DP_FIELD_SUBTOTALNAME)
        aRet <<= mpSubtotalName ? *mpSubtotalName : OUString();
    else if (aPropertyName == SC_UNO_DP_HAS_HIDDEN_MEMBER)
        aRet <<= mbHasHiddenMember;
    else if (aPropertyName == SC_UNO_DP_FLAGS)
    {
        aRet <<= sal_Int32(0); // tabular data: all orientations are possible
    }
    else
    {
        OSL_FAIL("unknown property");
        //TODO: THROW( UnknownPropertyException() );
    }
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDPDimension )

ScDPHierarchies::ScDPHierarchies( ScDPSource* pSrc, tools::Long nD ) :
    pSource( pSrc ),
    nDim( nD )
{
    //TODO: hold pSource
}

ScDPHierarchies::~ScDPHierarchies()
{
    //TODO: release pSource
}

// very simple XNameAccess implementation using getCount/getByIndex

uno::Any SAL_CALL ScDPHierarchies::getByName( const OUString& aName )
{
    tools::Long nCount = getCount();
    for (tools::Long i=0; i<nCount; i++)
        if ( getByIndex(i)->getName() == aName )
        {
            uno::Reference<container::XNamed> xNamed = getByIndex(i);
            uno::Any aRet;
            aRet <<= xNamed;
            return aRet;
        }

    throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScDPHierarchies::getElementNames()
{
    tools::Long nCount = getCount();
    uno::Sequence<OUString> aSeq(nCount);
    OUString* pArr = aSeq.getArray();
    for (tools::Long i=0; i<nCount; i++)
        pArr[i] = getByIndex(i)->getName();
    return aSeq;
}

sal_Bool SAL_CALL ScDPHierarchies::hasByName( const OUString& aName )
{
    tools::Long nCount = getCount();
    for (tools::Long i=0; i<nCount; i++)
        if ( getByIndex(i)->getName() == aName )
            return true;
    return false;
}

uno::Type SAL_CALL ScDPHierarchies::getElementType()
{
    return cppu::UnoType<container::XNamed>::get();
}

sal_Bool SAL_CALL ScDPHierarchies::hasElements()
{
    return ( getCount() > 0 );
}

// end of XNameAccess implementation

sal_Int32 ScDPHierarchies::getCount()
{
    return nHierCount;
}

ScDPHierarchy* ScDPHierarchies::getByIndex(tools::Long nIndex) const
{
    //  pass hierarchy index to new object in case the implementation
    //  will be extended to more than one hierarchy

    if ( nIndex >= 0 && nIndex < nHierCount )
    {
        if ( !ppHiers )
        {
            const_cast<ScDPHierarchies*>(this)->ppHiers.reset( new rtl::Reference<ScDPHierarchy>[nHierCount] );
            for (sal_Int32 i=0; i<nHierCount; i++)
                ppHiers[i] = nullptr;
        }
        if ( !ppHiers[nIndex].is() )
        {
            ppHiers[nIndex] = new ScDPHierarchy( pSource, nDim, nIndex );
        }

        return ppHiers[nIndex].get();
    }

    return nullptr;    //TODO: exception?
}

ScDPHierarchy::ScDPHierarchy( ScDPSource* pSrc, sal_Int32 nD, sal_Int32 nH ) :
    pSource( pSrc ),
    nDim( nD ),
    nHier( nH )
{
    //TODO: hold pSource
}

ScDPHierarchy::~ScDPHierarchy()
{
    //TODO: release pSource
}

ScDPLevels* ScDPHierarchy::GetLevelsObject()
{
    if (!mxLevels.is())
    {
        mxLevels = new ScDPLevels( pSource, nDim, nHier );
    }
    return mxLevels.get();
}

uno::Reference<container::XNameAccess> SAL_CALL ScDPHierarchy::getLevels()
{
    return GetLevelsObject();
}

OUString SAL_CALL ScDPHierarchy::getName()
{
    OUString aRet;        //TODO: globstr-ID !!!!
    switch (nHier)
    {
        case SC_DAPI_HIERARCHY_FLAT:
            aRet = "flat";
            break;  //TODO: name ???????
        case SC_DAPI_HIERARCHY_QUARTER:
            aRet = "Quarter";
            break;  //TODO: name ???????
        case SC_DAPI_HIERARCHY_WEEK:
            aRet = "Week";
            break;  //TODO: name ???????
        default:
            OSL_FAIL( "ScDPHierarchy::getName: unexpected hierarchy" );
            break;
    }
    return aRet;
}

void SAL_CALL ScDPHierarchy::setName( const OUString& /* rNewName */ )
{
    OSL_FAIL("not implemented");        //TODO: exception?
}

ScDPLevels::ScDPLevels( ScDPSource* pSrc, sal_Int32 nD, sal_Int32 nH ) :
    pSource( pSrc ),
    nDim( nD ),
    nHier( nH )
{
    //TODO: hold pSource

    //  text columns have only one level

    tools::Long nSrcDim = pSource->GetSourceDim( nDim );
    if ( pSource->IsDateDimension( nSrcDim ) )
    {
        switch ( nHier )
        {
            case SC_DAPI_HIERARCHY_FLAT:    nLevCount = SC_DAPI_FLAT_LEVELS;    break;
            case SC_DAPI_HIERARCHY_QUARTER: nLevCount = SC_DAPI_QUARTER_LEVELS; break;
            case SC_DAPI_HIERARCHY_WEEK:    nLevCount = SC_DAPI_WEEK_LEVELS;    break;
            default:
                OSL_FAIL("wrong hierarchy");
                nLevCount = 0;
        }
    }
    else
        nLevCount = 1;
}

ScDPLevels::~ScDPLevels()
{
    //TODO: release pSource
}

// very simple XNameAccess implementation using getCount/getByIndex

uno::Any SAL_CALL ScDPLevels::getByName( const OUString& aName )
{
    tools::Long nCount = getCount();
    for (tools::Long i=0; i<nCount; i++)
        if ( getByIndex(i)->getName() == aName )
        {
            uno::Reference<container::XNamed> xNamed = getByIndex(i);
            uno::Any aRet;
            aRet <<= xNamed;
            return aRet;
        }

    throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScDPLevels::getElementNames()
{
    tools::Long nCount = getCount();
    uno::Sequence<OUString> aSeq(nCount);
    OUString* pArr = aSeq.getArray();
    for (tools::Long i=0; i<nCount; i++)
        pArr[i] = getByIndex(i)->getName();
    return aSeq;
}

sal_Bool SAL_CALL ScDPLevels::hasByName( const OUString& aName )
{
    tools::Long nCount = getCount();
    for (tools::Long i=0; i<nCount; i++)
        if ( getByIndex(i)->getName() == aName )
            return true;
    return false;
}

uno::Type SAL_CALL ScDPLevels::getElementType()
{
    return cppu::UnoType<container::XNamed>::get();
}

sal_Bool SAL_CALL ScDPLevels::hasElements()
{
    return ( getCount() > 0 );
}

// end of XNameAccess implementation

sal_Int32 ScDPLevels::getCount() const
{
    return nLevCount;
}

ScDPLevel* ScDPLevels::getByIndex(sal_Int32 nIndex) const
{
    if ( nIndex >= 0 && nIndex < nLevCount )
    {
        if ( !ppLevs )
        {
            const_cast<ScDPLevels*>(this)->ppLevs.reset(new rtl::Reference<ScDPLevel>[nLevCount] );
            for (tools::Long i=0; i<nLevCount; i++)
                ppLevs[i] = nullptr;
        }
        if ( !ppLevs[nIndex].is() )
        {
            ppLevs[nIndex] = new ScDPLevel( pSource, nDim, nHier, nIndex );
        }

        return ppLevs[nIndex].get();
    }

    return nullptr;    //TODO: exception?
}

namespace {

class ScDPGlobalMembersOrder
{
    ScDPLevel&  rLevel;
    bool        bAscending;

public:
            ScDPGlobalMembersOrder( ScDPLevel& rLev, bool bAsc ) :
                rLevel(rLev),
                bAscending(bAsc)
            {}

    bool operator()( sal_Int32 nIndex1, sal_Int32 nIndex2 ) const;
};

}

bool ScDPGlobalMembersOrder::operator()( sal_Int32 nIndex1, sal_Int32 nIndex2 ) const
{
    sal_Int32 nCompare = 0;
    // seems that some ::std::sort() implementations pass the same index twice
    if( nIndex1 != nIndex2 )
    {
        ScDPMembers* pMembers = rLevel.GetMembersObject();
        ScDPMember* pMember1 = pMembers->getByIndex(nIndex1);
        ScDPMember* pMember2 = pMembers->getByIndex(nIndex2);
        nCompare = pMember1->Compare( *pMember2 );
    }
    return bAscending ? (nCompare < 0) : (nCompare > 0);
}

ScDPLevel::ScDPLevel( ScDPSource* pSrc, sal_Int32 nD, sal_Int32 nH, sal_Int32 nL ) :
    pSource( pSrc ),
    nDim( nD ),
    nHier( nH ),
    nLev( nL ),
    aSortInfo( OUString(), true, sheet::DataPilotFieldSortMode::NAME ),   // default: sort by name
    nSortMeasure( 0 ),
    nAutoMeasure( 0 ),
    bShowEmpty( false ),
    bEnableLayout( false ),
    bRepeatItemLabels( false )
{
    //TODO: hold pSource
    //  aSubTotals is empty
}

ScDPLevel::~ScDPLevel()
{
    //TODO: release pSource
}

void ScDPLevel::EvaluateSortOrder()
{
    switch (aSortInfo.Mode)
    {
        case sheet::DataPilotFieldSortMode::DATA:
            {
                // find index of measure (index among data dimensions)

                tools::Long nMeasureCount = pSource->GetDataDimensionCount();
                for (tools::Long nMeasure=0; nMeasure<nMeasureCount; nMeasure++)
                {
                    if (pSource->GetDataDimName(nMeasure) == aSortInfo.Field)
                    {
                        nSortMeasure = nMeasure;
                        break;
                    }
                }

                //TODO: error if not found?
            }
            break;
        case sheet::DataPilotFieldSortMode::MANUAL:
        case sheet::DataPilotFieldSortMode::NAME:
            {
                ScDPMembers* pLocalMembers = GetMembersObject();
                tools::Long nCount = pLocalMembers->getCount();

                aGlobalOrder.resize( nCount );
                for (tools::Long nPos=0; nPos<nCount; nPos++)
                    aGlobalOrder[nPos] = nPos;

                // allow manual or name (manual is always ascending)
                bool bAscending = ( aSortInfo.Mode == sheet::DataPilotFieldSortMode::MANUAL || aSortInfo.IsAscending );
                ScDPGlobalMembersOrder aComp( *this, bAscending );
                ::std::sort( aGlobalOrder.begin(), aGlobalOrder.end(), aComp );
            }
            break;
    }

    if ( !aAutoShowInfo.IsEnabled )
        return;

    // find index of measure (index among data dimensions)

    tools::Long nMeasureCount = pSource->GetDataDimensionCount();
    for (tools::Long nMeasure=0; nMeasure<nMeasureCount; nMeasure++)
    {
        if (pSource->GetDataDimName(nMeasure) == aAutoShowInfo.DataField)
        {
            nAutoMeasure = nMeasure;
            break;
        }
    }

    //TODO: error if not found?
}

void ScDPLevel::SetEnableLayout(bool bSet)
{
    bEnableLayout = bSet;
}

ScDPMembers* ScDPLevel::GetMembersObject()
{
    if (!mxMembers.is())
    {
        mxMembers = new ScDPMembers( pSource, nDim, nHier, nLev );
    }
    return mxMembers.get();
}

uno::Reference<sheet::XMembersAccess> SAL_CALL ScDPLevel::getMembers()
{
    return GetMembersObject();
}

uno::Sequence<sheet::MemberResult> SAL_CALL ScDPLevel::getResults()
{
    const uno::Sequence<sheet::MemberResult>* pRes = pSource->GetMemberResults( this );
    if (pRes)
        return *pRes;

    return {};       //TODO: Error?
}

OUString SAL_CALL ScDPLevel::getName()
{
    tools::Long nSrcDim = pSource->GetSourceDim( nDim );
    if ( pSource->IsDateDimension( nSrcDim ) )
    {
        OUString aRet;        //TODO: globstr-ID !!!!

        if ( nHier == SC_DAPI_HIERARCHY_QUARTER )
        {
            switch ( nLev )
            {
                case SC_DAPI_LEVEL_YEAR:
                    aRet = "Year";
                    break;
                case SC_DAPI_LEVEL_QUARTER:
                    aRet = "Quarter";
                    break;
                case SC_DAPI_LEVEL_MONTH:
                    aRet = "Month";
                    break;
                case SC_DAPI_LEVEL_DAY:
                    aRet = "Day";
                    break;
                default:
                    OSL_FAIL( "ScDPLevel::getName: unexpected level" );
                    break;
            }
        }
        else if ( nHier == SC_DAPI_HIERARCHY_WEEK )
        {
            switch ( nLev )
            {
                case SC_DAPI_LEVEL_YEAR:
                    aRet = "Year";
                    break;
                case SC_DAPI_LEVEL_WEEK:
                    aRet = "Week";
                    break;
                case SC_DAPI_LEVEL_WEEKDAY:
                    aRet = "Weekday";
                    break;
                default:
                    OSL_FAIL( "ScDPLevel::getName: unexpected level" );
                    break;
            }
        }
        if (!aRet.isEmpty())
            return aRet;
    }

    ScDPDimension* pDim = pSource->GetDimensionsObject()->getByIndex(nSrcDim);
    if (!pDim)
        return OUString();

    return pDim->getName();
}

void SAL_CALL ScDPLevel::setName( const OUString& /* rNewName */ )
{
    OSL_FAIL("not implemented");        //TODO: exception?
}

uno::Sequence<sal_Int16> ScDPLevel::getSubTotals() const
{
    //TODO: separate functions for settings and evaluation?

    tools::Long nSrcDim = pSource->GetSourceDim( nDim );
    if ( !pSource->SubTotalAllowed( nSrcDim ) )
        return {};

    return aSubTotals;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDPLevel::getPropertySetInfo()
{
    static const SfxItemPropertyMapEntry aDPLevelMap_Impl[] =
    {
        //TODO: change type of AutoShow/Layout/Sorting to API struct when available
        { SC_UNO_DP_AUTOSHOW, 0,  cppu::UnoType<sheet::DataPilotFieldAutoShowInfo>::get(),     0, 0 },
        { SC_UNO_DP_LAYOUT,   0,  cppu::UnoType<sheet::DataPilotFieldLayoutInfo>::get(),       0, 0 },
        { SC_UNO_DP_SHOWEMPTY, 0, cppu::UnoType<bool>::get(),                                   0, 0 },
        { SC_UNO_DP_REPEATITEMLABELS, 0, cppu::UnoType<bool>::get(),                                   0, 0 },
        { SC_UNO_DP_SORTING,  0,  cppu::UnoType<sheet::DataPilotFieldSortInfo>::get(),         0, 0 },
        { SC_UNO_DP_SUBTOTAL, 0,  cppu::UnoType<uno::Sequence<sheet::GeneralFunction>>::get(), 0, 0 },
        { SC_UNO_DP_SUBTOTAL2, 0, cppu::UnoType<uno::Sequence<sal_Int16>>::get(), 0, 0 },
    };
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aDPLevelMap_Impl );
    return aRef;
}

void SAL_CALL ScDPLevel::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    if ( aPropertyName == SC_UNO_DP_SHOWEMPTY )
        bShowEmpty = lcl_GetBoolFromAny(aValue);
    else if ( aPropertyName == SC_UNO_DP_REPEATITEMLABELS )
        bRepeatItemLabels = lcl_GetBoolFromAny(aValue);
    else if ( aPropertyName == SC_UNO_DP_SUBTOTAL )
    {
        uno::Sequence<sheet::GeneralFunction> aSeq;
        aValue >>= aSeq;
        aSubTotals.realloc(aSeq.getLength());
        std::transform(std::cbegin(aSeq), std::cend(aSeq), aSubTotals.getArray(),
            [](const sheet::GeneralFunction& rFunc) -> sal_Int16 {
                return static_cast<sal_Int16>(rFunc); });
    }
    else if ( aPropertyName == SC_UNO_DP_SUBTOTAL2 )
        aValue >>= aSubTotals;
    else if ( aPropertyName == SC_UNO_DP_SORTING )
        aValue >>= aSortInfo;
    else if ( aPropertyName == SC_UNO_DP_AUTOSHOW )
        aValue >>= aAutoShowInfo;
    else if ( aPropertyName == SC_UNO_DP_LAYOUT )
        aValue >>= aLayoutInfo;
    else
    {
        OSL_FAIL("unknown property");
    }
}

uno::Any SAL_CALL ScDPLevel::getPropertyValue( const OUString& aPropertyName )
{
    uno::Any aRet;
    if ( aPropertyName == SC_UNO_DP_SHOWEMPTY )
        aRet <<= bShowEmpty;
    else if ( aPropertyName == SC_UNO_DP_REPEATITEMLABELS )
        aRet <<= bRepeatItemLabels;
    else if ( aPropertyName == SC_UNO_DP_SUBTOTAL )
    {
        const uno::Sequence<sal_Int16> aSeq = getSubTotals();
        uno::Sequence<sheet::GeneralFunction> aNewSeq(aSeq.getLength());
        std::transform(aSeq.begin(), aSeq.end(), aNewSeq.getArray(),
            [](const sal_Int16 nFunc) -> sheet::GeneralFunction {
                if (nFunc == sheet::GeneralFunction2::MEDIAN)
                    return sheet::GeneralFunction_NONE;
                return static_cast<sheet::GeneralFunction>(nFunc);
            });

        aRet <<= aNewSeq;
    }
    else if ( aPropertyName == SC_UNO_DP_SUBTOTAL2 )
    {
        uno::Sequence<sal_Int16> aSeq = getSubTotals();        //TODO: avoid extra copy?
        aRet <<= aSeq;
    }
    else if ( aPropertyName == SC_UNO_DP_SORTING )
        aRet <<= aSortInfo;
    else if ( aPropertyName == SC_UNO_DP_AUTOSHOW )
        aRet <<= aAutoShowInfo;
    else if ( aPropertyName == SC_UNO_DP_LAYOUT )
        aRet <<= aLayoutInfo;
    else if (aPropertyName == SC_UNO_DP_LAYOUTNAME)
    {
        // read only property
        tools::Long nSrcDim = pSource->GetSourceDim(nDim);
        ScDPDimension* pDim = pSource->GetDimensionsObject()->getByIndex(nSrcDim);
        if (!pDim)
            return aRet;

        const std::optional<OUString> & pLayoutName = pDim->GetLayoutName();
        if (!pLayoutName)
            return aRet;

        aRet <<= *pLayoutName;
    }
    else
    {
        OSL_FAIL("unknown property");
    }
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDPLevel )

ScDPMembers::ScDPMembers( ScDPSource* pSrc, sal_Int32 nD, sal_Int32 nH, sal_Int32 nL ) :
    pSource( pSrc ),
    nDim( nD ),
    nHier( nH ),
    nLev( nL )
{
    //TODO: hold pSource

    tools::Long nSrcDim = pSource->GetSourceDim( nDim );
    if ( pSource->IsDataLayoutDimension(nSrcDim) )
        nMbrCount = pSource->GetDataDimensionCount();
    else if ( nHier != SC_DAPI_HIERARCHY_FLAT && pSource->IsDateDimension( nSrcDim ) )
    {
        nMbrCount = 0;
        if ( nHier == SC_DAPI_HIERARCHY_QUARTER )
        {
            switch (nLev)
            {
                case SC_DAPI_LEVEL_YEAR:
                    {
                        const ScDPItemData* pLastNumData = nullptr;
                        for ( SCROW n = 0; n < static_cast<SCROW>(pSource->GetData()->GetColumnEntries(nDim).size()); n-- )
                        {
                            const ScDPItemData* pData  = GetSrcItemDataByIndex( n );
                            if ( pData && pData->HasStringData() )
                                break;
                            else
                                pLastNumData = pData;
                        }

                        if ( pLastNumData )
                        {
                            const ScDPItemData*  pFirstData = GetSrcItemDataByIndex( 0 );
                            double fFirstVal = pFirstData->GetValue();
                            double fLastVal = pLastNumData->GetValue();

                            tools::Long nFirstYear = pSource->GetData()->GetDatePart(
                                        static_cast<tools::Long>(::rtl::math::approxFloor( fFirstVal )),
                                        nHier, nLev );
                            tools::Long nLastYear = pSource->GetData()->GetDatePart(
                                        static_cast<tools::Long>(::rtl::math::approxFloor( fLastVal )),
                                        nHier, nLev );

                            nMbrCount = nLastYear + 1 - nFirstYear;
                        }
                        else
                            nMbrCount = 0;      // no values
                    }
                    break;
                case SC_DAPI_LEVEL_QUARTER: nMbrCount = 4;  break;
                case SC_DAPI_LEVEL_MONTH:   nMbrCount = 12; break;
                case SC_DAPI_LEVEL_DAY:     nMbrCount = 31; break;
                default:
                    OSL_FAIL( "ScDPMembers::ScDPMembers: unexpected level" );
                    break;
            }
        }
        else if ( nHier == SC_DAPI_HIERARCHY_WEEK )
        {
            switch (nLev)
            {
                case SC_DAPI_LEVEL_YEAR:    nMbrCount = 1;  break;      //TODO: get years from source
                case SC_DAPI_LEVEL_WEEK:    nMbrCount = 53; break;
                case SC_DAPI_LEVEL_WEEKDAY: nMbrCount = 7;  break;
                default:
                    OSL_FAIL( "ScDPMembers::ScDPMembers: unexpected level" );
                    break;
            }
        }
    }
    else
        nMbrCount = pSource->GetData()->GetMembersCount( nSrcDim );
}

ScDPMembers::~ScDPMembers()
{
}

// XNameAccess implementation using getCount/getByIndex

sal_Int32 ScDPMembers::GetIndexFromName( const OUString& rName ) const
{
    if ( aHashMap.empty() )
    {
        // store the index for each name

        sal_Int32 nCount = getCount();
        for (sal_Int32 i=0; i<nCount; i++)
            aHashMap[ getByIndex(i)->getName() ] = i;
    }

    ScDPMembersHashMap::const_iterator aIter = aHashMap.find( rName );
    if ( aIter != aHashMap.end() )
        return aIter->second;           // found index
    else
        return -1;                      // not found
}

uno::Any SAL_CALL ScDPMembers::getByName( const OUString& aName )
{
    sal_Int32 nIndex = GetIndexFromName( aName );
    if ( nIndex >= 0 )
    {
        uno::Reference<container::XNamed> xNamed = getByIndex(nIndex);
        uno::Any aRet;
        aRet <<= xNamed;
        return aRet;
    }

    throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScDPMembers::getElementNames()
{
    return getElementNames( false );
}

sal_Bool SAL_CALL ScDPMembers::hasByName( const OUString& aName )
{
    return ( GetIndexFromName( aName ) >= 0 );
}

uno::Type SAL_CALL ScDPMembers::getElementType()
{
    return cppu::UnoType<container::XNamed>::get();
}

sal_Bool SAL_CALL ScDPMembers::hasElements()
{
    return ( getCount() > 0 );
}

// end of XNameAccess implementation

// XMembersAccess implementation

uno::Sequence<OUString> SAL_CALL ScDPMembers::getLocaleIndependentElementNames()
{
    return getElementNames( true );
}

// end of XMembersAccess implementation

uno::Sequence<OUString> ScDPMembers::getElementNames( bool bLocaleIndependent ) const
{
    // Return list of names in sorted order,
    // so it's displayed in that order in the field options dialog.
    // Sorting is done at the level object (parent of this).

    ScDPLevel* pLevel = pSource->GetDimensionsObject()->getByIndex(nDim)->
        GetHierarchiesObject()->getByIndex(nHier)->GetLevelsObject()->getByIndex(nLev);
    pLevel->EvaluateSortOrder();
    const std::vector<sal_Int32>& rGlobalOrder = pLevel->GetGlobalOrder();
    bool bSort = !rGlobalOrder.empty();

    tools::Long nCount = getCount();
    uno::Sequence<OUString> aSeq(nCount);
    OUString* pArr = aSeq.getArray();
    for (tools::Long i=0; i<nCount; i++)
        pArr[i] = getByIndex(bSort ? rGlobalOrder[i] : i)->GetNameStr( bLocaleIndependent);
    return aSeq;
}

sal_Int32 ScDPMembers::getMinMembers() const
{
    // used in lcl_CountMinMembers

    sal_Int32 nVisCount = 0;
    if (!maMembers.empty())
    {
        nVisCount = std::count_if(maMembers.begin(), maMembers.end(), [](const rtl::Reference<ScDPMember>& pMbr) {
            //  count only visible with details (default is true for both)
            return !pMbr || (pMbr->isVisible() && pMbr->getShowDetails()); });
    }
    else
        nVisCount = nMbrCount;      // default for all

    return nVisCount;
}

ScDPMember* ScDPMembers::getByIndex(sal_Int32 nIndex) const
{
    //  result of GetColumnEntries must not change between ScDPMembers ctor
    //  and all calls to getByIndex

    if ( nIndex >= 0 && nIndex < nMbrCount )
    {
        if (maMembers.empty())
            maMembers.resize(nMbrCount);

        if (!maMembers[nIndex])
        {
            rtl::Reference<ScDPMember> pNew;
            sal_Int32 nSrcDim = pSource->GetSourceDim( nDim );
            if ( pSource->IsDataLayoutDimension(nSrcDim) )
            {
                // empty name (never shown, not used for lookup)
                pNew.set(new ScDPMember(pSource, nDim, nHier, nLev, 0));
            }
            else if ( nHier != SC_DAPI_HIERARCHY_FLAT && pSource->IsDateDimension( nSrcDim ) )
            {
                sal_Int32 nGroupBy = 0;
                sal_Int32 nVal = 0;
                OUString aName;

                if ( nLev == SC_DAPI_LEVEL_YEAR )   // YEAR is in both hierarchies
                {
                    //TODO: cache year range here!

                    double fFirstVal = pSource->GetData()->GetMemberByIndex( nSrcDim, 0 )->GetValue();
                    tools::Long nFirstYear = pSource->GetData()->GetDatePart(
                                        static_cast<tools::Long>(::rtl::math::approxFloor( fFirstVal )),
                                        nHier, nLev );

                    nVal = nFirstYear + nIndex;
                }
                else if ( nHier == SC_DAPI_HIERARCHY_WEEK && nLev == SC_DAPI_LEVEL_WEEKDAY )
                {
                    nVal = nIndex;              // DayOfWeek is 0-based
                    aName = ScGlobal::GetCalendar().getDisplayName(
                        css::i18n::CalendarDisplayIndex::DAY,
                        sal::static_int_cast<sal_Int16>(nVal), 0 );
                }
                else if ( nHier == SC_DAPI_HIERARCHY_QUARTER && nLev == SC_DAPI_LEVEL_MONTH )
                {
                    nVal = nIndex;              // Month is 0-based
                    aName = ScGlobal::GetCalendar().getDisplayName(
                        css::i18n::CalendarDisplayIndex::MONTH,
                        sal::static_int_cast<sal_Int16>(nVal), 0 );
                }
                else
                    nVal = nIndex + 1;          // Quarter, Day, Week are 1-based

                switch (nLev)
                {
                    case SC_DAPI_LEVEL_YEAR:
                        nGroupBy = sheet::DataPilotFieldGroupBy::YEARS;
                    break;
                    case SC_DAPI_LEVEL_QUARTER:
                    case SC_DAPI_LEVEL_WEEK:
                        nGroupBy = sheet::DataPilotFieldGroupBy::QUARTERS;
                    break;
                    case SC_DAPI_LEVEL_MONTH:
                    case SC_DAPI_LEVEL_WEEKDAY:
                        nGroupBy = sheet::DataPilotFieldGroupBy::MONTHS;
                    break;
                    case SC_DAPI_LEVEL_DAY:
                        nGroupBy = sheet::DataPilotFieldGroupBy::DAYS;
                    break;
                    default:
                        ;
                }
                if (aName.isEmpty())
                    aName = OUString::number(nVal);

                ScDPItemData aData(nGroupBy, nVal);
                SCROW nId = pSource->GetCache()->GetIdByItemData(nDim, aData);
                pNew.set(new ScDPMember(pSource, nDim, nHier, nLev, nId));
            }
            else
            {
                const std::vector<SCROW>& memberIndexs = pSource->GetData()->GetColumnEntries(nSrcDim);
                pNew.set(new ScDPMember(pSource, nDim, nHier, nLev, memberIndexs[nIndex]));
            }
            maMembers[nIndex] = pNew;
        }

        return maMembers[nIndex].get();
    }

    return nullptr;    //TODO: exception?
}

ScDPMember::ScDPMember(
    ScDPSource* pSrc, sal_Int32 nD, sal_Int32 nH, sal_Int32 nL, SCROW nIndex) :
    pSource( pSrc ),
    nDim( nD ),
    nHier( nH ),
    nLev( nL ),
    mnDataId( nIndex ),
    nPosition( -1 ),
    bVisible( true ),
    bShowDet( true )
{
    //TODO: hold pSource
}

ScDPMember::~ScDPMember()
{
    //TODO: release pSource
}

bool ScDPMember::IsNamedItem(SCROW nIndex) const
{
    sal_Int32 nSrcDim = pSource->GetSourceDim( nDim );
    if ( nHier != SC_DAPI_HIERARCHY_FLAT && pSource->IsDateDimension( nSrcDim ) )
    {
        const ScDPItemData* pData = pSource->GetCache()->GetItemDataById(nDim, nIndex);
        if (pData->IsValue())
        {
            tools::Long nComp = pSource->GetData()->GetDatePart(
                static_cast<tools::Long>(::rtl::math::approxFloor( pData->GetValue() )),
                nHier, nLev );
            //  fValue is converted from integer, so simple comparison works
            const ScDPItemData* pData2 = GetItemData();
            return pData2 && nComp == pData2->GetValue();
        }
    }

    return  nIndex == mnDataId;
}

sal_Int32 ScDPMember::Compare( const ScDPMember& rOther ) const
{
    if ( nPosition >= 0 )
    {
        if ( rOther.nPosition >= 0 )
        {
            OSL_ENSURE( nPosition != rOther.nPosition, "same position for two members" );
            return ( nPosition < rOther.nPosition ) ? -1 : 1;
        }
        else
        {
            // only this has a position - members with specified positions come before those without
            return -1;
        }
    }
    else if ( rOther.nPosition >= 0 )
    {
        // only rOther has a position
        return 1;
    }

    // no positions set - compare names
    return pSource->GetData()->Compare( pSource->GetSourceDim(nDim),mnDataId,rOther.GetItemDataId());
}

ScDPItemData ScDPMember::FillItemData() const
{
    //TODO: handle date hierarchy...

    const ScDPItemData* pData = GetItemData();
    return (pData ? *pData : ScDPItemData());
}

const std::optional<OUString> & ScDPMember::GetLayoutName() const
{
    return mpLayoutName;
}

OUString ScDPMember::GetNameStr( bool bLocaleIndependent ) const
{
    const ScDPItemData* pData = GetItemData();
    if (pData)
        return pSource->GetData()->GetFormattedString(nDim, *pData, bLocaleIndependent);
    return OUString();
}

OUString SAL_CALL ScDPMember::getName()
{
    return GetNameStr( false );
}

void SAL_CALL ScDPMember::setName( const OUString& /* rNewName */ )
{
    OSL_FAIL("not implemented");        //TODO: exception?
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDPMember::getPropertySetInfo()
{
    static const SfxItemPropertyMapEntry aDPMemberMap_Impl[] =
    {
        { SC_UNO_DP_ISVISIBLE, 0,  cppu::UnoType<bool>::get(),              0, 0 },
        { SC_UNO_DP_POSITION, 0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
        { SC_UNO_DP_SHOWDETAILS, 0,  cppu::UnoType<bool>::get(),              0, 0 },
        { SC_UNO_DP_LAYOUTNAME, 0, cppu::UnoType<OUString>::get(), 0, 0 },
    };
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aDPMemberMap_Impl );
    return aRef;
}

void SAL_CALL ScDPMember::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    if ( aPropertyName == SC_UNO_DP_ISVISIBLE )
        bVisible = lcl_GetBoolFromAny(aValue);
    else if ( aPropertyName == SC_UNO_DP_SHOWDETAILS )
        bShowDet = lcl_GetBoolFromAny(aValue);
    else if ( aPropertyName == SC_UNO_DP_POSITION )
        aValue >>= nPosition;
    else if (aPropertyName == SC_UNO_DP_LAYOUTNAME)
    {
        OUString aName;
        if (aValue >>= aName)
            mpLayoutName = aName;
    }
    else
    {
        OSL_FAIL("unknown property");
    }
}

uno::Any SAL_CALL ScDPMember::getPropertyValue( const OUString& aPropertyName )
{
    uno::Any aRet;
    if ( aPropertyName == SC_UNO_DP_ISVISIBLE )
        aRet <<= bVisible;
    else if ( aPropertyName == SC_UNO_DP_SHOWDETAILS )
        aRet <<= bShowDet;
    else if ( aPropertyName == SC_UNO_DP_POSITION )
        aRet <<= nPosition;
    else if (aPropertyName == SC_UNO_DP_LAYOUTNAME)
        aRet <<= mpLayoutName ? *mpLayoutName : OUString();
    else
    {
        OSL_FAIL("unknown property");
    }
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDPMember )

const ScDPCache* ScDPSource::GetCache()
{
    OSL_ENSURE( GetData() , "empty ScDPTableData pointer");
    return ( GetData()!=nullptr ) ? &GetData()->GetCacheTable().getCache() : nullptr ;
}

const ScDPItemData* ScDPMember::GetItemData() const
{
    const ScDPItemData* pData = pSource->GetItemDataById(nDim, mnDataId);
    SAL_WARN_IF( !pData, "sc.core", "ScDPMember::GetItemData: what data? nDim " << nDim << ", mnDataId " << mnDataId);
    return pData;
}

const ScDPItemData* ScDPSource::GetItemDataById(sal_Int32 nDim, sal_Int32 nId)
{
    return GetData()->GetMemberById(nDim, nId);
}

const ScDPItemData* ScDPMembers::GetSrcItemDataByIndex(SCROW nIndex)
{
    const std::vector< SCROW >& memberIds = pSource->GetData()->GetColumnEntries( nDim );
    if ( nIndex < 0 || o3tl::make_unsigned(nIndex) >= memberIds.size()  )
        return nullptr;
    SCROW nId =  memberIds[ nIndex ];
    return pSource->GetItemDataById( nDim, nId );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
