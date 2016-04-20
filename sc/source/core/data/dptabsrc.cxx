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

#include "dptabsrc.hxx"

#include <algorithm>
#include <set>
#include <unordered_set>
#include <vector>

#include <rtl/math.hxx>
#include <svl/itemprop.hxx>
#include <svl/intitem.hxx>
#include <vcl/svapp.hxx>

#include "scitems.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "formulacell.hxx"

#include "dptabres.hxx"
#include "dptabdat.hxx"
#include "global.hxx"
#include "datauno.hxx"
#include "miscuno.hxx"
#include "unonames.hxx"
#include "dpitemdata.hxx"
#include "dputil.hxx"
#include "dpresfilter.hxx"
#include "calcmacros.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <comphelper/string.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>

using namespace com::sun::star;
using ::std::vector;
using ::std::set;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::sheet::DataPilotFieldAutoShowInfo;

#define SC_MINCOUNT_LIMIT   1000000

SC_SIMPLE_SERVICE_INFO( ScDPSource,      "ScDPSource",      "com.sun.star.sheet.DataPilotSource" )
SC_SIMPLE_SERVICE_INFO( ScDPDimensions,  "ScDPDimensions",  "com.sun.star.sheet.DataPilotSourceDimensions" )
SC_SIMPLE_SERVICE_INFO( ScDPDimension,   "ScDPDimension",   "com.sun.star.sheet.DataPilotSourceDimension" )
SC_SIMPLE_SERVICE_INFO( ScDPHierarchies, "ScDPHierarchies", "com.sun.star.sheet.DataPilotSourceHierarcies" )
SC_SIMPLE_SERVICE_INFO( ScDPHierarchy,   "ScDPHierarchy",   "com.sun.star.sheet.DataPilotSourceHierarcy" )
SC_SIMPLE_SERVICE_INFO( ScDPLevels,      "ScDPLevels",      "com.sun.star.sheet.DataPilotSourceLevels" )
SC_SIMPLE_SERVICE_INFO( ScDPLevel,       "ScDPLevel",       "com.sun.star.sheet.DataPilotSourceLevel" )
SC_SIMPLE_SERVICE_INFO( ScDPMembers,     "ScDPMembers",     "com.sun.star.sheet.DataPilotSourceMembers" )
SC_SIMPLE_SERVICE_INFO( ScDPMember,      "ScDPMember",      "com.sun.star.sheet.DataPilotSourceMember" )

// property maps for PropertySetInfo
//  DataDescription / NumberFormat are internal

//TODO: move to a header?
static bool lcl_GetBoolFromAny( const uno::Any& aAny )
{
    if ( aAny.getValueTypeClass() == uno::TypeClass_BOOLEAN )
        return *static_cast<sal_Bool const *>(aAny.getValue());
    return false;
}

static void lcl_SetBoolInAny( uno::Any& rAny, bool bValue )
{
    rAny.setValue( &bValue, cppu::UnoType<bool>::get() );
}

ScDPSource::ScDPSource( ScDPTableData* pD ) :
    pData( pD ),
    pDimensions( nullptr ),
    bColumnGrand( true ),       // default is true
    bRowGrand( true ),
    bIgnoreEmptyRows( false ),
    bRepeatIfEmpty( false ),
    nDupCount( 0 ),
    pResData( nullptr ),
    pColResRoot( nullptr ),
    pRowResRoot( nullptr ),
    pColResults( nullptr ),
    pRowResults( nullptr ),
    bResultOverflow( false ),
    bPageFiltered( false )
{
    pData->SetEmptyFlags( bIgnoreEmptyRows, bRepeatIfEmpty );
}

ScDPSource::~ScDPSource()
{
    if (pDimensions)
        pDimensions->release();     // ref-counted

    // free lists

    delete[] pColResults;
    delete[] pRowResults;

    delete pColResRoot;
    delete pRowResRoot;
    delete pResData;
}

const OUString* ScDPSource::GetGrandTotalName() const
{
    return mpGrandTotalName.get();
}

sal_uInt16 ScDPSource::GetOrientation(long nColumn)
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

long ScDPSource::GetDataDimensionCount()
{
    return maDataDims.size();
}

ScDPDimension* ScDPSource::GetDataDimension(long nIndex)
{
    if (nIndex < 0 || static_cast<size_t>(nIndex) >= maDataDims.size())
        return nullptr;

    long nDimIndex = maDataDims[nIndex];
    return GetDimensionsObject()->getByIndex(nDimIndex);
}

OUString ScDPSource::GetDataDimName(long nIndex)
{
    OUString aRet;
    ScDPDimension* pDim = GetDataDimension(nIndex);
    if (pDim)
        aRet = pDim->getName();
    return aRet;
}

long ScDPSource::GetPosition(long nColumn)
{
    std::vector<long>::const_iterator it, itBeg = maColDims.begin(), itEnd = maColDims.end();
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

bool testSubTotal( bool& rAllowed, long nColumn, const std::vector<long>& rDims, ScDPSource* pSource )
{
    rAllowed = true;
    std::vector<long>::const_iterator it = rDims.begin(), itEnd = rDims.end();
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

void removeDim( long nRemove, std::vector<long>& rDims )
{
    std::vector<long>::iterator it = std::find(rDims.begin(), rDims.end(), nRemove);
    if (it != rDims.end())
        rDims.erase(it);
}

}

bool ScDPSource::SubTotalAllowed(long nColumn)
{
    //TODO: cache this at ScDPResultData
    bool bAllowed = true;
    if ( testSubTotal(bAllowed, nColumn, maColDims, this) )
        return bAllowed;
    if ( testSubTotal(bAllowed, nColumn, maRowDims, this) )
        return bAllowed;
    return bAllowed;
}

void ScDPSource::SetOrientation(long nColumn, sal_uInt16 nNew)
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

bool ScDPSource::IsDataLayoutDimension(long nDim)
{
    return nDim == pData->GetColumnCount();
}

sal_uInt16 ScDPSource::GetDataLayoutOrientation()
{
    return GetOrientation(pData->GetColumnCount());
}

bool ScDPSource::IsDateDimension(long nDim)
{
    return pData->IsDateDimension(nDim);
}

ScDPDimensions* ScDPSource::GetDimensionsObject()
{
    if (!pDimensions)
    {
        pDimensions = new ScDPDimensions(this);
        pDimensions->acquire();                     // ref-counted
    }
    return pDimensions;
}

uno::Reference<container::XNameAccess> SAL_CALL ScDPSource::getDimensions() throw(uno::RuntimeException, std::exception)
{
    return GetDimensionsObject();
}

void ScDPSource::SetDupCount( long nNew )
{
    nDupCount = nNew;
}

ScDPDimension* ScDPSource::AddDuplicated(long /* nSource */, const OUString& rNewName)
{
    OSL_ENSURE( pDimensions, "AddDuplicated without dimensions?" );

    //  re-use

    long nOldDimCount = pDimensions->getCount();
    for (long i=0; i<nOldDimCount; i++)
    {
        ScDPDimension* pDim = pDimensions->getByIndex(i);
        if (pDim && pDim->getName().equals(rNewName))
        {
            //TODO: test if pDim is a duplicate of source
            return pDim;
        }
    }

    SetDupCount( nDupCount + 1 );
    pDimensions->CountChanged();        // uses nDupCount

    return pDimensions->getByIndex( pDimensions->getCount() - 1 );
}

long ScDPSource::GetSourceDim(long nDim)
{
    //  original source dimension or data layout dimension?
    if ( nDim <= pData->GetColumnCount() )
        return nDim;

    if ( nDim < pDimensions->getCount() )
    {
        ScDPDimension* pDimObj = pDimensions->getByIndex( nDim );
        if ( pDimObj )
        {
            long nSource = pDimObj->GetSourceDim();
            if ( nSource >= 0 )
                return nSource;
        }
    }

    OSL_FAIL("GetSourceDim: wrong dim");
    return nDim;
}

uno::Sequence< uno::Sequence<sheet::DataResult> > SAL_CALL ScDPSource::getResults()
                                                            throw(uno::RuntimeException, std::exception)
{
    CreateRes_Impl();       // create pColResRoot and pRowResRoot

    if ( bResultOverflow )      // set in CreateRes_Impl
    {
        //  no results available
        throw uno::RuntimeException();
    }

    long nColCount = pColResRoot->GetSize(pResData->GetColStartMeasure());
    long nRowCount = pRowResRoot->GetSize(pResData->GetRowStartMeasure());

    //  allocate full sequence
    //TODO: leave out empty rows???

    uno::Sequence< uno::Sequence<sheet::DataResult> > aSeq( nRowCount );
    uno::Sequence<sheet::DataResult>* pRowAry = aSeq.getArray();
    for (long nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<sheet::DataResult> aColSeq( nColCount );
        //  use default values of DataResult
        pRowAry[nRow] = aColSeq;
    }

    ScDPResultFilterContext aFilterCxt;
    pRowResRoot->FillDataResults(
        pColResRoot, aFilterCxt, aSeq, pResData->GetRowStartMeasure());

    maResFilterSet.swap(aFilterCxt.maFilterSet); // Keep this data for GETPIVOTDATA.

    return aSeq;
}

uno::Sequence<double> ScDPSource::getFilteredResults(
            const uno::Sequence<sheet::DataPilotFieldFilter>& aFilters )
                throw (uno::RuntimeException, std::exception)
{
    if (maResFilterSet.empty())
        getResults(); // Build result tree first.

    // Get result values from the tree.
    const ScDPResultTree::ValuesType* pVals = maResFilterSet.getResults(aFilters);
    if (pVals)
    {
        size_t n = pVals->size();
        uno::Sequence<double> aRet(n);
        for (size_t i = 0; i < n; ++i)
            aRet[i] = (*pVals)[i];

        return aRet;
    }

    if (aFilters.getLength() == 1)
    {
        // Try to get result from the leaf nodes.
        double fVal = maResFilterSet.getLeafResult(aFilters[0]);
        if (!rtl::math::isNan(fVal))
        {
            uno::Sequence<double> aRet(1);
            aRet[0] = fVal;
            return aRet;
        }
    }

    return uno::Sequence<double>();
}

void SAL_CALL ScDPSource::refresh() throw(uno::RuntimeException, std::exception)
{
    disposeData();
}

void SAL_CALL ScDPSource::addRefreshListener( const uno::Reference<util::XRefreshListener >& )
                                                throw(uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");    //TODO: exception?
}

void SAL_CALL ScDPSource::removeRefreshListener( const uno::Reference<util::XRefreshListener >& )
                                                throw(uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");    //TODO: exception?
}

Sequence< Sequence<Any> > SAL_CALL ScDPSource::getDrillDownData(const Sequence<sheet::DataPilotFieldFilter>& aFilters)
    throw (uno::RuntimeException, std::exception)
{
    long nColumnCount = GetData()->GetColumnCount();

    vector<ScDPFilteredCache::Criterion> aFilterCriteria;
    sal_Int32 nFilterCount = aFilters.getLength();
    for (sal_Int32 i = 0; i < nFilterCount; ++i)
    {
        const sheet::DataPilotFieldFilter& rFilter = aFilters[i];
        const OUString& aFieldName = rFilter.FieldName;
        for (long nCol = 0; nCol < nColumnCount; ++nCol)
        {
            if (aFieldName.equals(pData->getDimensionName(nCol)))
            {
                ScDPDimension* pDim = GetDimensionsObject()->getByIndex( nCol );
                ScDPMembers* pMembers = pDim->GetHierarchiesObject()->getByIndex(0)->
                                        GetLevelsObject()->getByIndex(0)->GetMembersObject();
                sal_Int32 nIndex = pMembers->GetIndexFromName( rFilter.MatchValue );
                if ( nIndex >= 0 )
                {
                    ScDPItemData aItem;
                    pMembers->getByIndex(nIndex)->FillItemData( aItem );
                    aFilterCriteria.push_back( ScDPFilteredCache::Criterion() );
                    aFilterCriteria.back().mnFieldIndex = nCol;
                    aFilterCriteria.back().mpFilter.reset(
                        new ScDPFilteredCache::SingleFilter(aItem));
                }
            }
        }
    }

    // Take into account the visibilities of field members.
    ScDPResultVisibilityData aResVisData(this);
    pRowResRoot->FillVisibilityData(aResVisData);
    pColResRoot->FillVisibilityData(aResVisData);
    aResVisData.fillFieldFilters(aFilterCriteria);

    Sequence< Sequence<Any> > aTabData;
    std::unordered_set<sal_Int32> aCatDims;
    GetCategoryDimensionIndices(aCatDims);
    pData->GetDrillDownData(aFilterCriteria, aCatDims, aTabData);
    return aTabData;
}

OUString ScDPSource::getDataDescription()
{
    CreateRes_Impl();       // create pResData

    OUString aRet;
    if ( pResData->GetMeasureCount() == 1 )
    {
        bool bTotalResult = false;
        aRet = pResData->GetMeasureString(0, true, SUBTOTAL_FUNC_NONE, bTotalResult);
    }

    //  empty for more than one measure

    return aRet;
}

void ScDPSource::setIgnoreEmptyRows(bool bSet)
{
    bIgnoreEmptyRows = bSet;
    pData->SetEmptyFlags( bIgnoreEmptyRows, bRepeatIfEmpty );
}

void ScDPSource::setRepeatIfEmpty(bool bSet)
{
    bRepeatIfEmpty = bSet;
    pData->SetEmptyFlags( bIgnoreEmptyRows, bRepeatIfEmpty );
}

void ScDPSource::disposeData()
{
    maResFilterSet.clear();

    if ( pResData )
    {
        //  reset all data...

        DELETEZ(pColResRoot);
        DELETEZ(pRowResRoot);
        DELETEZ(pResData);
        delete[] pColResults;
        delete[] pRowResults;
        pColResults = nullptr;
        pRowResults = nullptr;
        aColLevelList.clear();
        aRowLevelList.clear();
    }

    if ( pDimensions )
    {
        pDimensions->release(); // ref-counted
        pDimensions = nullptr;     //  settings have to be applied (from SaveData) again!
    }
    SetDupCount( 0 );

    maColDims.clear();
    maRowDims.clear();
    maDataDims.clear();
    maPageDims.clear();

    pData->DisposeData();   // cached entries etc.
    bPageFiltered = false;
    bResultOverflow = false;
}

static long lcl_CountMinMembers(const vector<ScDPDimension*>& ppDim, const vector<ScDPLevel*>& ppLevel, long nLevels )
{
    //  Calculate the product of the member count for those consecutive levels that
    //  have the "show all" flag, one following level, and the data layout dimension.

    long nTotal = 1;
    long nDataCount = 1;
    bool bWasShowAll = true;
    long nPos = nLevels;
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
            long nThisCount = ppLevel[nPos]->GetMembersObject()->getMinMembers();
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

static long lcl_GetIndexFromName( const OUString& rName, const uno::Sequence<OUString>& rElements )
{
    long nCount = rElements.getLength();
    const OUString* pArray = rElements.getConstArray();
    for (long nPos=0; nPos<nCount; nPos++)
        if (pArray[nPos] == rName)
            return nPos;

    return -1;  // not found
}

void ScDPSource::FillCalcInfo(bool bIsRow, ScDPTableData::CalcInfo& rInfo, bool &rHasAutoShow)
{
    const std::vector<long>& rDims = bIsRow ? maRowDims : maColDims;
    std::vector<long>::const_iterator it = rDims.begin(), itEnd = rDims.end();
    for (; it != itEnd; ++it)
    {
        ScDPDimension* pDim = GetDimensionsObject()->getByIndex(*it);
        long nHierarchy = pDim->getUsedHierarchy();
        if ( nHierarchy >= pDim->GetHierarchiesObject()->getCount() )
            nHierarchy = 0;
        ScDPLevels* pLevels = pDim->GetHierarchiesObject()->getByIndex(nHierarchy)->GetLevelsObject();
        long nCount = pLevels->getCount();

        //TODO: Test
        if (pDim->getIsDataLayoutDimension() && maDataDims.size() < 2)
            nCount = 0;
        //TODO: Test

        for (long j = 0; j < nCount; ++j)
        {
            ScDPLevel* pLevel = pLevels->getByIndex(j);
            pLevel->EvaluateSortOrder();

            // no layout flags for column fields, only for row fields
            pLevel->SetEnableLayout( bIsRow );

            if ( pLevel->GetAutoShow().IsEnabled )
                rHasAutoShow = true;

            if (bIsRow)
            {
                rInfo.aRowLevelDims.push_back(*it);
                rInfo.aRowDims.push_back(pDim);
                rInfo.aRowLevels.push_back(pLevel);
            }
            else
            {
                rInfo.aColLevelDims.push_back(*it);
                rInfo.aColDims.push_back(pDim);
                rInfo.aColLevels.push_back(pLevel);
            }

            pLevel->GetMembersObject();                 // initialize for groups
        }
    }
}

namespace {

class CategoryDimInserter : public std::unary_function<long, void>
{
    ScDPSource& mrSource;
    std::unordered_set<sal_Int32>& mrCatDims;
public:
    CategoryDimInserter(ScDPSource& rSource, std::unordered_set<sal_Int32>& rCatDims) :
        mrSource(rSource),
        mrCatDims(rCatDims) {}

    void operator() (long nDim)
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

    if (bPageFiltered)
    {
        SAL_WARN( "sc.core","tried to apply page field filters several times");

        pData->DisposeData();
        pData->CreateCacheTable();  // re-initialize the cache table
        bPageFiltered = false;
    }

    // filter table by page dimensions.
    vector<ScDPFilteredCache::Criterion> aCriteria;
    vector<long>::const_iterator it = maPageDims.begin(), itEnd = maPageDims.end();
    for (; it != itEnd; ++it)
    {
        ScDPDimension* pDim = GetDimensionsObject()->getByIndex(*it);
        long nField = pDim->GetDimension();

        ScDPMembers* pMems = pDim->GetHierarchiesObject()->getByIndex(0)->
            GetLevelsObject()->getByIndex(0)->GetMembersObject();

        long nMemCount = pMems->getCount();
        ScDPFilteredCache::Criterion aFilter;
        aFilter.mnFieldIndex = static_cast<sal_Int32>(nField);
        aFilter.mpFilter.reset(new ScDPFilteredCache::GroupFilter);
        ScDPFilteredCache::GroupFilter* pGrpFilter =
            static_cast<ScDPFilteredCache::GroupFilter*>(aFilter.mpFilter.get());
        for (long j = 0; j < nMemCount; ++j)
        {
            ScDPMember* pMem = pMems->getByIndex(j);
            if (pMem->isVisible())
            {
                ScDPItemData aData;
                pMem->FillItemData(aData);
                pGrpFilter->addMatchItem(aData);
            }
        }
        if (pGrpFilter->getMatchItemCount() < static_cast<size_t>(nMemCount))
            // there is at least one invisible item.  Add this filter criterion to the mix.
            aCriteria.push_back(aFilter);

        if (!pDim->HasSelectedPage())
            continue;

        const ScDPItemData& rData = pDim->GetSelectedData();
        aCriteria.push_back(ScDPFilteredCache::Criterion());
        ScDPFilteredCache::Criterion& r = aCriteria.back();
        r.mnFieldIndex = static_cast<sal_Int32>(nField);
        r.mpFilter.reset(new ScDPFilteredCache::SingleFilter(rData));
    }
    if (!aCriteria.empty())
    {
        std::unordered_set<sal_Int32> aCatDims;
        GetCategoryDimensionIndices(aCatDims);
        pData->FilterCacheTable(aCriteria, aCatDims);
        bPageFiltered = true;
    }
}

void ScDPSource::CreateRes_Impl()
{
    if (pResData)
        return;

    sal_uInt16 nDataOrient = GetDataLayoutOrientation();
    if (maDataDims.size() > 1 && ( nDataOrient != sheet::DataPilotFieldOrientation_COLUMN &&
                                nDataOrient != sheet::DataPilotFieldOrientation_ROW ) )
    {
        //  if more than one data dimension, data layout orientation must be set
        SetOrientation( pData->GetColumnCount(), sheet::DataPilotFieldOrientation_ROW );
        nDataOrient = sheet::DataPilotFieldOrientation_ROW;
    }

    // TODO: Aggregate pDataNames, pDataRefValues, nDataRefOrient, and
    // eDataFunctions into a structure and use vector instead of static
    // or pointer arrays.
    vector<OUString> aDataNames;
    vector<sheet::DataPilotFieldReference> aDataRefValues;
    vector<ScSubTotalFunc> aDataFunctions;
    vector<sal_uInt16> aDataRefOrient;

    ScDPTableData::CalcInfo aInfo;

    //  LateInit (initialize only those rows/children that are used) can be used unless
    //  any data dimension needs reference values from column/row dimensions
    bool bLateInit = true;

    // Go through all data dimensions (i.e. fields) and build their meta data
    // so that they can be passed on to ScDPResultData instance later.
    // TODO: aggregate all of data dimension info into a structure.
    vector<long>::const_iterator it = maDataDims.begin(), itEnd = maDataDims.end();
    for (; it != itEnd; ++it)
    {
        // Get function for each data field.
        long nDimIndex = *it;
        ScDPDimension* pDim = GetDimensionsObject()->getByIndex(nDimIndex);
        sheet::GeneralFunction eUser = (sheet::GeneralFunction)pDim->getFunction();
        if (eUser == sheet::GeneralFunction_AUTO)
        {
            //TODO: test for numeric data
            eUser = sheet::GeneralFunction_SUM;
        }

        // Map UNO's enum to internal enum ScSubTotalFunc.
        aDataFunctions.push_back(ScDPUtil::toSubTotalFunc(eUser));

        // Get reference field/item information.
        aDataRefValues.push_back(pDim->GetReferenceValue());
        sal_uInt16 nDataRefOrient = sheet::DataPilotFieldOrientation_HIDDEN;    // default if not used
        sal_Int32 eRefType = aDataRefValues.back().ReferenceType;
        if ( eRefType == sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE ||
             eRefType == sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE ||
             eRefType == sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE ||
             eRefType == sheet::DataPilotFieldReferenceType::RUNNING_TOTAL )
        {
            long nColumn = lcl_GetIndexFromName(
                aDataRefValues.back().ReferenceField, GetDimensionsObject()->getElementNames());
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

        long nSource = pDim->GetSourceDim();
        if (nSource >= 0)
            aInfo.aDataSrcCols.push_back(nSource);
        else
            aInfo.aDataSrcCols.push_back(nDimIndex);
    }

    pResData = new ScDPResultData(*this);
    pResData->SetMeasureData(aDataFunctions, aDataRefValues, aDataRefOrient, aDataNames);
    pResData->SetDataLayoutOrientation(nDataOrient);
    pResData->SetLateInit( bLateInit );

    bool bHasAutoShow = false;

    ScDPInitState aInitState;

    // Page field selections restrict the members shown in related fields
    // (both in column and row fields). aInitState is filled with the page
    // field selections, they are kept across the data iterator loop.

    for (it = maPageDims.begin(), itEnd = maPageDims.end(); it != itEnd; ++it)
    {
        ScDPDimension* pDim = GetDimensionsObject()->getByIndex(*it);
        if ( pDim->HasSelectedPage() )
            aInitState.AddMember(*it, GetMemberId(*it, pDim->GetSelectedData()));
    }

    // Show grand total columns only when the option is set *and* there is at
    // least one column field.  Same for the grand total rows.
    sal_uInt16 nDataLayoutOrient = GetDataLayoutOrientation();
    long nColDimCount2 = maColDims.size() - (nDataLayoutOrient == sheet::DataPilotFieldOrientation_COLUMN ? 1 : 0);
    long nRowDimCount2 = maRowDims.size() - (nDataLayoutOrient == sheet::DataPilotFieldOrientation_ROW ? 1 : 0);
    bool bShowColGrand = bColumnGrand && nColDimCount2 > 0;
    bool bShowRowGrand = bRowGrand && nRowDimCount2 > 0;
    pColResRoot = new ScDPResultMember(pResData, bShowColGrand);
    pRowResRoot = new ScDPResultMember(pResData, bShowRowGrand);

    FillCalcInfo(false, aInfo, bHasAutoShow);
    long nColLevelCount = aInfo.aColLevels.size();

    pColResRoot->InitFrom( aInfo.aColDims, aInfo.aColLevels, 0, aInitState );
    pColResRoot->SetHasElements();

    FillCalcInfo(true, aInfo, bHasAutoShow);
    long nRowLevelCount = aInfo.aRowLevels.size();

    if ( nRowLevelCount > 0 )
    {
        // disable layout flags for the innermost row field (level)
        aInfo.aRowLevels[nRowLevelCount-1]->SetEnableLayout( false );
    }

    pRowResRoot->InitFrom( aInfo.aRowDims, aInfo.aRowLevels, 0, aInitState );
    pRowResRoot->SetHasElements();

    // initialize members object also for all page dimensions (needed for numeric groups)
    for (it = maPageDims.begin(), itEnd = maPageDims.end(); it != itEnd; ++it)
    {
        ScDPDimension* pDim = GetDimensionsObject()->getByIndex(*it);
        long nHierarchy = pDim->getUsedHierarchy();
        if ( nHierarchy >= pDim->GetHierarchiesObject()->getCount() )
            nHierarchy = 0;

        ScDPLevels* pLevels = pDim->GetHierarchiesObject()->getByIndex(nHierarchy)->GetLevelsObject();
        long nCount = pLevels->getCount();
        for (long j=0; j<nCount; j++)
            pLevels->getByIndex(j)->GetMembersObject();             // initialize for groups
    }

    //  pre-check: calculate minimum number of result columns / rows from
    //  levels that have the "show all" flag set

    long nMinColMembers = lcl_CountMinMembers( aInfo.aColDims, aInfo.aColLevels, nColLevelCount );
    long nMinRowMembers = lcl_CountMinMembers( aInfo.aRowDims, aInfo.aRowLevels, nRowLevelCount );

    if ( nMinColMembers > MAXCOLCOUNT/*SC_MINCOUNT_LIMIT*/ || nMinRowMembers > SC_MINCOUNT_LIMIT )
    {
        //  resulting table is too big -> abort before calculating
        //  (this relies on late init, so no members are allocated in InitFrom above)

        bResultOverflow = true;
        return;
    }

    FilterCacheByPageDimensions();

    aInfo.aPageDims.reserve(maPageDims.size());
    for (it = maPageDims.begin(), itEnd = maPageDims.end(); it != itEnd; ++it)
        aInfo.aPageDims.push_back(*it);

    aInfo.pInitState = &aInitState;
    aInfo.pColRoot   = pColResRoot;
    aInfo.pRowRoot   = pRowResRoot;
    pData->CalcResults(aInfo, false);

    pColResRoot->CheckShowEmpty();
    pRowResRoot->CheckShowEmpty();

    //  With all data processed, calculate the final results:

    //  UpdateDataResults calculates all original results from the collected values,
    //  and stores them as reference values if needed.
    pRowResRoot->UpdateDataResults( pColResRoot, pResData->GetRowStartMeasure() );

    if ( bHasAutoShow )     // do the double calculation only if AutoShow is used
    {
        //  Find the desired members and set bAutoHidden flag for the others
        pRowResRoot->DoAutoShow( pColResRoot );

        //  Reset all results to empty, so they can be built again with data for the
        //  desired members only.
        pColResRoot->ResetResults();
        pRowResRoot->ResetResults();
        pData->CalcResults(aInfo, true);

        //  Call UpdateDataResults again, with the new (limited) values.
        pRowResRoot->UpdateDataResults( pColResRoot, pResData->GetRowStartMeasure() );
    }

    //  SortMembers does the sorting by a result dimension, using the original results,
    //  but not running totals etc.
    pRowResRoot->SortMembers( pColResRoot );

    //  UpdateRunningTotals calculates running totals along column/row dimensions,
    //  differences from other members (named or relative), and column/row percentages
    //  or index values.
    //  Running totals and relative differences need to be done using the sorted values.
    //  Column/row percentages and index values must be done after sorting, because the
    //  results may no longer be in the right order (row total for percentage of row is
    //  always 1).
    ScDPRunningTotalState aRunning( pColResRoot, pRowResRoot );
    ScDPRowTotals aTotals;
    pRowResRoot->UpdateRunningTotals( pColResRoot, pResData->GetRowStartMeasure(), aRunning, aTotals );
}

void ScDPSource::FillLevelList( sal_uInt16 nOrientation, std::vector<ScDPLevel*> &rList )
{
    rList.clear();

    std::vector<long>* pDimIndex = nullptr;
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
    std::vector<long>::const_iterator it = pDimIndex->begin(), itEnd = pDimIndex->end();
    for (; it != itEnd; ++it)
    {
        ScDPDimension* pDim = pDims->getByIndex(*it);
        OSL_ENSURE( pDim->getOrientation() == nOrientation, "orientations are wrong" );

        ScDPHierarchies* pHiers = pDim->GetHierarchiesObject();
        long nHierarchy = pDim->getUsedHierarchy();
        if ( nHierarchy >= pHiers->getCount() )
            nHierarchy = 0;
        ScDPHierarchy* pHier = pHiers->getByIndex(nHierarchy);
        ScDPLevels* pLevels = pHier->GetLevelsObject();
        long nLevCount = pLevels->getCount();
        for (long nLev=0; nLev<nLevCount; nLev++)
        {
            ScDPLevel* pLevel = pLevels->getByIndex(nLev);
            rList.push_back(pLevel);
        }
    }
}

void ScDPSource::FillMemberResults()
{
    if ( !pColResults && !pRowResults )
    {
        CreateRes_Impl();

        if ( bResultOverflow )      // set in CreateRes_Impl
        {
            //  no results available -> abort (leave empty)
            //  exception is thrown in ScDPSource::getResults
            return;
        }

        FillLevelList( sheet::DataPilotFieldOrientation_COLUMN, aColLevelList );
        long nColLevelCount = aColLevelList.size();
        if (nColLevelCount)
        {
            long nColDimSize = pColResRoot->GetSize(pResData->GetColStartMeasure());
            pColResults = new uno::Sequence<sheet::MemberResult>[nColLevelCount];
            for (long i=0; i<nColLevelCount; i++)
                pColResults[i].realloc(nColDimSize);

            long nPos = 0;
            pColResRoot->FillMemberResults( pColResults, nPos, pResData->GetColStartMeasure(),
                                            true, nullptr, nullptr );
        }

        FillLevelList( sheet::DataPilotFieldOrientation_ROW, aRowLevelList );
        long nRowLevelCount = aRowLevelList.size();
        if (nRowLevelCount)
        {
            long nRowDimSize = pRowResRoot->GetSize(pResData->GetRowStartMeasure());
            pRowResults = new uno::Sequence<sheet::MemberResult>[nRowLevelCount];
            for (long i=0; i<nRowLevelCount; i++)
                pRowResults[i].realloc(nRowDimSize);

            long nPos = 0;
            pRowResRoot->FillMemberResults( pRowResults, nPos, pResData->GetRowStartMeasure(),
                                            true, nullptr, nullptr );
        }
    }
}

const uno::Sequence<sheet::MemberResult>* ScDPSource::GetMemberResults( ScDPLevel* pLevel )
{
    FillMemberResults();

    long i = 0;
    long nColCount = aColLevelList.size();
    for (i=0; i<nColCount; i++)
    {
        ScDPLevel* pColLevel = aColLevelList[i];
        if ( pColLevel == pLevel )
            return pColResults+i;
    }
    long nRowCount = aRowLevelList.size();
    for (i=0; i<nRowCount; i++)
    {
        ScDPLevel* pRowLevel = aRowLevelList[i];
        if ( pRowLevel == pLevel )
            return pRowResults+i;
    }
    return nullptr;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDPSource::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    using beans::PropertyAttribute::READONLY;

    static const SfxItemPropertyMapEntry aDPSourceMap_Impl[] =
    {
        { OUString(SC_UNO_DP_COLGRAND), 0,  cppu::UnoType<bool>::get(),              0, 0 },
        { OUString(SC_UNO_DP_DATADESC), 0,  cppu::UnoType<OUString>::get(),    beans::PropertyAttribute::READONLY, 0 },
        { OUString(SC_UNO_DP_IGNOREEMPTY), 0,  cppu::UnoType<bool>::get(),              0, 0 },     // for sheet data only
        { OUString(SC_UNO_DP_REPEATEMPTY), 0,  cppu::UnoType<bool>::get(),              0, 0 },     // for sheet data only
        { OUString(SC_UNO_DP_ROWGRAND), 0,  cppu::UnoType<bool>::get(),              0, 0 },
        { OUString(SC_UNO_DP_ROWFIELDCOUNT),    0, cppu::UnoType<sal_Int32>::get(), READONLY, 0 },
        { OUString(SC_UNO_DP_COLUMNFIELDCOUNT), 0, cppu::UnoType<sal_Int32>::get(), READONLY, 0 },
        { OUString(SC_UNO_DP_DATAFIELDCOUNT),   0, cppu::UnoType<sal_Int32>::get(), READONLY, 0 },
        { OUString(SC_UNO_DP_GRANDTOTAL_NAME),  0, cppu::UnoType<OUString>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aDPSourceMap_Impl );
    return aRef;
}

void SAL_CALL ScDPSource::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    if (aPropertyName == SC_UNO_DP_COLGRAND)
        bColumnGrand = lcl_GetBoolFromAny(aValue);
    else if (aPropertyName == SC_UNO_DP_ROWGRAND)
        bRowGrand = lcl_GetBoolFromAny(aValue);
    else if (aPropertyName == SC_UNO_DP_IGNOREEMPTY)
        setIgnoreEmptyRows( lcl_GetBoolFromAny( aValue ) );
    else if (aPropertyName == SC_UNO_DP_REPEATEMPTY)
        setRepeatIfEmpty( lcl_GetBoolFromAny( aValue ) );
    else if (aPropertyName == SC_UNO_DP_GRANDTOTAL_NAME)
    {
        OUString aName;
        if (aValue >>= aName)
            mpGrandTotalName.reset(new OUString(aName));
    }
    else
    {
        OSL_FAIL("unknown property");
        //TODO: THROW( UnknownPropertyException() );
    }
}

uno::Any SAL_CALL ScDPSource::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    uno::Any aRet;
    if ( aPropertyName == SC_UNO_DP_COLGRAND )
        lcl_SetBoolInAny(aRet, bColumnGrand);
    else if ( aPropertyName == SC_UNO_DP_ROWGRAND )
        lcl_SetBoolInAny(aRet, bRowGrand);
    else if ( aPropertyName == SC_UNO_DP_IGNOREEMPTY )
        lcl_SetBoolInAny(aRet, bIgnoreEmptyRows);
    else if ( aPropertyName == SC_UNO_DP_REPEATEMPTY )
        lcl_SetBoolInAny(aRet, bRepeatIfEmpty);
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
        if (mpGrandTotalName.get())
            aRet <<= *mpGrandTotalName;
    }
    else
    {
        OSL_FAIL("unknown property");
        //TODO: THROW( UnknownPropertyException() );
    }
    return aRet;
}

#if DEBUG_PIVOT_TABLE
void ScDPSource::DumpResults() const
{
    std::cout << "+++++ column root" << std::endl;
    pColResRoot->Dump(1);
    std::cout << "+++++ row root" << std::endl;
    pRowResRoot->Dump(1);
}
#endif

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDPSource )

ScDPDimensions::ScDPDimensions( ScDPSource* pSrc ) :
    pSource( pSrc ),
    ppDims( nullptr )
{
    //TODO: hold pSource

    // include data layout dimension and duplicated dimensions
    nDimCount = pSource->GetData()->GetColumnCount() + 1 + pSource->GetDupCount();
}

ScDPDimensions::~ScDPDimensions()
{
    //TODO: release pSource

    if (ppDims)
    {
        for (long i=0; i<nDimCount; i++)
            if ( ppDims[i] )
                ppDims[i]->release();           // ref-counted
        delete[] ppDims;
    }
}

void ScDPDimensions::CountChanged()
{
    // include data layout dimension and duplicated dimensions
    long nNewCount = pSource->GetData()->GetColumnCount() + 1 + pSource->GetDupCount();
    if ( ppDims )
    {
        long i;
        long nCopy = std::min( nNewCount, nDimCount );
        ScDPDimension** ppNew = new ScDPDimension*[nNewCount];

        for (i=0; i<nCopy; i++)             // copy existing dims
            ppNew[i] = ppDims[i];
        for (i=nCopy; i<nNewCount; i++)     // clear additional pointers
            ppNew[i] = nullptr;
        for (i=nCopy; i<nDimCount; i++)     // delete old dims if count is decreased
            if ( ppDims[i] )
                ppDims[i]->release();       // ref-counted

        delete[] ppDims;
        ppDims = ppNew;
    }
    nDimCount = nNewCount;
}

// very simple XNameAccess implementation using getCount/getByIndex

uno::Any SAL_CALL ScDPDimensions::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    long nCount = getCount();
    for (long i=0; i<nCount; i++)
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

uno::Sequence<OUString> SAL_CALL ScDPDimensions::getElementNames() throw(uno::RuntimeException, std::exception)
{
    long nCount = getCount();
    uno::Sequence<OUString> aSeq(nCount);
    OUString* pArr = aSeq.getArray();
    for (long i=0; i<nCount; i++)
        pArr[i] = getByIndex(i)->getName();
    return aSeq;
}

sal_Bool SAL_CALL ScDPDimensions::hasByName( const OUString& aName ) throw(uno::RuntimeException, std::exception)
{
    long nCount = getCount();
    for (long i=0; i<nCount; i++)
        if ( getByIndex(i)->getName() == aName )
            return true;
    return false;
}

uno::Type SAL_CALL ScDPDimensions::getElementType() throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<container::XNamed>::get();
}

sal_Bool SAL_CALL ScDPDimensions::hasElements() throw(uno::RuntimeException, std::exception)
{
    return ( getCount() > 0 );
}

// end of XNameAccess implementation

long ScDPDimensions::getCount() const
{
    //  in tabular data, every column of source data is a dimension

    return nDimCount;
}

ScDPDimension* ScDPDimensions::getByIndex(long nIndex) const
{
    if ( nIndex >= 0 && nIndex < nDimCount )
    {
        if ( !ppDims )
        {
            const_cast<ScDPDimensions*>(this)->ppDims = new ScDPDimension*[nDimCount];
            for (long i=0; i<nDimCount; i++)
                ppDims[i] = nullptr;
        }
        if ( !ppDims[nIndex] )
        {
            ppDims[nIndex] = new ScDPDimension( pSource, nIndex );
            ppDims[nIndex]->acquire();      // ref-counted
        }

        return ppDims[nIndex];
    }

    return nullptr;    //TODO: exception?
}

ScDPDimension::ScDPDimension( ScDPSource* pSrc, long nD ) :
    pSource( pSrc ),
    nDim( nD ),
    pHierarchies( nullptr ),
    nUsedHier( 0 ),
    nFunction( SUBTOTAL_FUNC_SUM ),     // sum is default
    mpLayoutName(nullptr),
    mpSubtotalName(nullptr),
    nSourceDim( -1 ),
    bHasSelectedPage( false ),
    pSelectedData( nullptr ),
    mbHasHiddenMember(false)
{
    //TODO: hold pSource
}

ScDPDimension::~ScDPDimension()
{
    //TODO: release pSource

    if ( pHierarchies )
        pHierarchies->release();    // ref-counted

    delete pSelectedData;
}

ScDPHierarchies* ScDPDimension::GetHierarchiesObject()
{
    if (!pHierarchies)
    {
        pHierarchies = new ScDPHierarchies( pSource, nDim );
        pHierarchies->acquire();        // ref-counted
    }
    return pHierarchies;
}

const OUString* ScDPDimension::GetLayoutName() const
{
    return mpLayoutName.get();
}

const OUString* ScDPDimension::GetSubtotalName() const
{
    return mpSubtotalName.get();
}

uno::Reference<container::XNameAccess> SAL_CALL ScDPDimension::getHierarchies()
                                                    throw(uno::RuntimeException, std::exception)
{
    return GetHierarchiesObject();
}

OUString SAL_CALL ScDPDimension::getName() throw(uno::RuntimeException, std::exception)
{
    if (!aName.isEmpty())
        return aName;
    else
        return pSource->GetData()->getDimensionName( nDim );
}

void SAL_CALL ScDPDimension::setName( const OUString& rNewName ) throw(uno::RuntimeException, std::exception)
{
    //  used after cloning
    aName = rNewName;
}

sal_uInt16 ScDPDimension::getOrientation() const
{
    return pSource->GetOrientation( nDim );
}

void ScDPDimension::setOrientation(sal_uInt16 nNew)
{
    pSource->SetOrientation( nDim, nNew );
}

long ScDPDimension::getPosition() const
{
    return pSource->GetPosition( nDim );
}

bool ScDPDimension::getIsDataLayoutDimension() const
{
    return pSource->GetData()->getIsDataLayoutDimension( nDim );
}

void ScDPDimension::setFunction(sal_uInt16 nNew)
{
    nFunction = nNew;
}

ScDPDimension* ScDPDimension::CreateCloneObject()
{
    OSL_ENSURE( nSourceDim < 0, "recursive duplicate - not implemented" );

    //TODO: set new name here, or temporary name ???
    OUString aNewName = aName;

    ScDPDimension* pNew = pSource->AddDuplicated( nDim, aNewName );

    pNew->aName = aNewName;             //TODO: here or in source?
    pNew->nSourceDim = nDim;            //TODO: recursive?

    return pNew;
}

uno::Reference<util::XCloneable> SAL_CALL ScDPDimension::createClone() throw(uno::RuntimeException, std::exception)
{
    return CreateCloneObject();
}

const ScDPItemData& ScDPDimension::GetSelectedData()
{
    if ( !pSelectedData )
    {
        // find the named member to initialize pSelectedData from it, with name and value

        long nLevel = 0;

        long nHierarchy = getUsedHierarchy();
        if ( nHierarchy >= GetHierarchiesObject()->getCount() )
            nHierarchy = 0;
        ScDPLevels* pLevels = GetHierarchiesObject()->getByIndex(nHierarchy)->GetLevelsObject();
        long nLevCount = pLevels->getCount();
        if ( nLevel < nLevCount )
        {
            ScDPMembers* pMembers = pLevels->getByIndex(nLevel)->GetMembersObject();

            //TODO: merge with ScDPMembers::getByName
            long nCount = pMembers->getCount();
            for (long i=0; i<nCount && !pSelectedData; i++)
            {
                ScDPMember* pMember = pMembers->getByIndex(i);
                if (aSelectedPage.equals(pMember->GetNameStr()))
                {
                    pSelectedData = new ScDPItemData();
                    pMember->FillItemData( *pSelectedData );
                }
            }
        }

        if ( !pSelectedData )
            pSelectedData = new ScDPItemData(aSelectedPage);      // default - name only
    }

    return *pSelectedData;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDPDimension::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    static const SfxItemPropertyMapEntry aDPDimensionMap_Impl[] =
    {
        { OUString(SC_UNO_DP_FILTER),   0,  cppu::UnoType<uno::Sequence<sheet::TableFilterField>>::get(), 0, 0 },
        { OUString(SC_UNO_DP_FLAGS),    0,  cppu::UnoType<sal_Int32>::get(),                beans::PropertyAttribute::READONLY, 0 },
        { OUString(SC_UNO_DP_FUNCTION), 0,  cppu::UnoType<sheet::GeneralFunction>::get(),   0, 0 },
        { OUString(SC_UNO_DP_ISDATALAYOUT), 0,  cppu::UnoType<bool>::get(),                      beans::PropertyAttribute::READONLY, 0 },
        { OUString(SC_UNO_DP_NUMBERFO), 0,  cppu::UnoType<sal_Int32>::get(),                beans::PropertyAttribute::READONLY, 0 },
        { OUString(SC_UNO_DP_ORIENTATION), 0,  cppu::UnoType<sheet::DataPilotFieldOrientation>::get(), 0, 0 },
        { OUString(SC_UNO_DP_ORIGINAL), 0,  cppu::UnoType<container::XNamed>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString(SC_UNO_DP_ORIGINAL_POS), 0, cppu::UnoType<sal_Int32>::get(),             0, 0 },
        { OUString(SC_UNO_DP_POSITION), 0,  cppu::UnoType<sal_Int32>::get(),                0, 0 },
        { OUString(SC_UNO_DP_REFVALUE), 0,  cppu::UnoType<sheet::DataPilotFieldReference>::get(), 0, 0 },
        { OUString(SC_UNO_DP_USEDHIERARCHY), 0,  cppu::UnoType<sal_Int32>::get(),                0, 0 },
        { OUString(SC_UNO_DP_LAYOUTNAME), 0, cppu::UnoType<OUString>::get(), 0, 0 },
        { OUString(SC_UNO_DP_FIELD_SUBTOTALNAME), 0, cppu::UnoType<OUString>::get(), 0, 0 },
        { OUString(SC_UNO_DP_HAS_HIDDEN_MEMBER), 0, cppu::UnoType<bool>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aDPDimensionMap_Impl );
    return aRef;
}

void SAL_CALL ScDPDimension::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    if ( aPropertyName == SC_UNO_DP_USEDHIERARCHY )
    {
        // #i52547# don't use the incomplete date hierarchy implementation - ignore the call
    }
    else if ( aPropertyName == SC_UNO_DP_ORIENTATION )
    {
        sheet::DataPilotFieldOrientation eEnum;
        if (aValue >>= eEnum)
            setOrientation( sal::static_int_cast<sal_uInt16>(eEnum) );
    }
    else if ( aPropertyName == SC_UNO_DP_FUNCTION )
    {
        sheet::GeneralFunction eEnum;
        if (aValue >>= eEnum)
            setFunction( sal::static_int_cast<sal_uInt16>(eEnum) );
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
        DELETEZ( pSelectedData );       // invalid after changing aSelectedPage
    }
    else if (aPropertyName == SC_UNO_DP_LAYOUTNAME)
    {
        OUString aTmpName;
        if (aValue >>= aTmpName)
            mpLayoutName.reset(new OUString(aTmpName));
    }
    else if (aPropertyName == SC_UNO_DP_FIELD_SUBTOTALNAME)
    {
        OUString aTmpName;
        if (aValue >>= aTmpName)
            mpSubtotalName.reset(new OUString(aTmpName));
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
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    uno::Any aRet;
    if ( aPropertyName == SC_UNO_DP_POSITION )
        aRet <<= (sal_Int32) getPosition();
    else if ( aPropertyName == SC_UNO_DP_USEDHIERARCHY )
        aRet <<= (sal_Int32) getUsedHierarchy();
    else if ( aPropertyName == SC_UNO_DP_ORIENTATION )
    {
        sheet::DataPilotFieldOrientation eVal = (sheet::DataPilotFieldOrientation)getOrientation();
        aRet <<= eVal;
    }
    else if ( aPropertyName == SC_UNO_DP_FUNCTION )
    {
        sheet::GeneralFunction eVal = (sheet::GeneralFunction)getFunction();
        aRet <<= eVal;
    }
    else if ( aPropertyName == SC_UNO_DP_REFVALUE )
        aRet <<= aReferenceValue;
    else if ( aPropertyName == SC_UNO_DP_ISDATALAYOUT )                 // read-only properties
        lcl_SetBoolInAny( aRet, getIsDataLayoutDimension() );
    else if ( aPropertyName == SC_UNO_DP_NUMBERFO )
    {
        sal_Int32 nFormat = 0;
        sheet::GeneralFunction eFunc = (sheet::GeneralFunction)getFunction();
        // #i63745# don't use source format for "count"
        if ( eFunc != sheet::GeneralFunction_COUNT && eFunc != sheet::GeneralFunction_COUNTNUMS )
            nFormat = pSource->GetData()->GetNumberFormat( ( nSourceDim >= 0 ) ? nSourceDim : nDim );

        switch ( aReferenceValue.ReferenceType )
        {
        case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE:
        case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE:
        case sheet::DataPilotFieldReferenceType::ROW_PERCENTAGE:
        case sheet::DataPilotFieldReferenceType::COLUMN_PERCENTAGE:
        case sheet::DataPilotFieldReferenceType::TOTAL_PERCENTAGE:
            nFormat = pSource->GetData()->GetNumberFormatByIdx( (NfIndexTableOffset)NF_PERCENT_DEC2 );
            break;
        case sheet::DataPilotFieldReferenceType::INDEX:
            nFormat = pSource->GetData()->GetNumberFormatByIdx( (NfIndexTableOffset)NF_NUMBER_SYSTEM );
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
        sal_Int32 nPos = static_cast<sal_Int32>(nSourceDim);
        aRet <<= nPos;
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
        aRet <<= mpLayoutName.get() ? *mpLayoutName : OUString("");
    else if (aPropertyName == SC_UNO_DP_FIELD_SUBTOTALNAME)
        aRet <<= mpSubtotalName.get() ? *mpSubtotalName : OUString("");
    else if (aPropertyName == SC_UNO_DP_HAS_HIDDEN_MEMBER)
        aRet <<= mbHasHiddenMember;
    else if (aPropertyName == SC_UNO_DP_FLAGS)
    {
        sal_Int32 nFlags = 0;       // tabular data: all orientations are possible
        aRet <<= nFlags;
    }
    else
    {
        OSL_FAIL("unknown property");
        //TODO: THROW( UnknownPropertyException() );
    }
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDPDimension )

ScDPHierarchies::ScDPHierarchies( ScDPSource* pSrc, long nD ) :
    pSource( pSrc ),
    nDim( nD ),
    ppHiers( nullptr )
{
    //TODO: hold pSource

    //  date columns have 3 hierarchies (flat/quarter/week), other columns only one

    // #i52547# don't offer the incomplete date hierarchy implementation
    nHierCount = 1;
}

ScDPHierarchies::~ScDPHierarchies()
{
    //TODO: release pSource

    if (ppHiers)
    {
        for (long i=0; i<nHierCount; i++)
            if ( ppHiers[i] )
                ppHiers[i]->release();      // ref-counted
        delete[] ppHiers;
    }
}

// very simple XNameAccess implementation using getCount/getByIndex

uno::Any SAL_CALL ScDPHierarchies::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    long nCount = getCount();
    for (long i=0; i<nCount; i++)
        if ( getByIndex(i)->getName() == aName )
        {
            uno::Reference<container::XNamed> xNamed = getByIndex(i);
            uno::Any aRet;
            aRet <<= xNamed;
            return aRet;
        }

    throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScDPHierarchies::getElementNames() throw(uno::RuntimeException, std::exception)
{
    long nCount = getCount();
    uno::Sequence<OUString> aSeq(nCount);
    OUString* pArr = aSeq.getArray();
    for (long i=0; i<nCount; i++)
        pArr[i] = getByIndex(i)->getName();
    return aSeq;
}

sal_Bool SAL_CALL ScDPHierarchies::hasByName( const OUString& aName ) throw(uno::RuntimeException, std::exception)
{
    long nCount = getCount();
    for (long i=0; i<nCount; i++)
        if ( getByIndex(i)->getName() == aName )
            return true;
    return false;
}

uno::Type SAL_CALL ScDPHierarchies::getElementType() throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<container::XNamed>::get();
}

sal_Bool SAL_CALL ScDPHierarchies::hasElements() throw(uno::RuntimeException, std::exception)
{
    return ( getCount() > 0 );
}

// end of XNameAccess implementation

long ScDPHierarchies::getCount() const
{
    return nHierCount;
}

ScDPHierarchy* ScDPHierarchies::getByIndex(long nIndex) const
{
    //  pass hierarchy index to new object in case the implementation
    //  will be extended to more than one hierarchy

    if ( nIndex >= 0 && nIndex < nHierCount )
    {
        if ( !ppHiers )
        {
            const_cast<ScDPHierarchies*>(this)->ppHiers = new ScDPHierarchy*[nHierCount];
            for (long i=0; i<nHierCount; i++)
                ppHiers[i] = nullptr;
        }
        if ( !ppHiers[nIndex] )
        {
            ppHiers[nIndex] = new ScDPHierarchy( pSource, nDim, nIndex );
            ppHiers[nIndex]->acquire();         // ref-counted
        }

        return ppHiers[nIndex];
    }

    return nullptr;    //TODO: exception?
}

ScDPHierarchy::ScDPHierarchy( ScDPSource* pSrc, long nD, long nH ) :
    pSource( pSrc ),
    nDim( nD ),
    nHier( nH ),
    pLevels( nullptr )
{
    //TODO: hold pSource
}

ScDPHierarchy::~ScDPHierarchy()
{
    //TODO: release pSource

    if (pLevels)
        pLevels->release();     // ref-counted
}

ScDPLevels* ScDPHierarchy::GetLevelsObject()
{
    if (!pLevels)
    {
        pLevels = new ScDPLevels( pSource, nDim, nHier );
        pLevels->acquire();     // ref-counted
    }
    return pLevels;
}

uno::Reference<container::XNameAccess> SAL_CALL ScDPHierarchy::getLevels()
                                                    throw(uno::RuntimeException, std::exception)
{
    return GetLevelsObject();
}

OUString SAL_CALL ScDPHierarchy::getName() throw(uno::RuntimeException, std::exception)
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

void SAL_CALL ScDPHierarchy::setName( const OUString& /* rNewName */ ) throw(uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");        //TODO: exception?
}

ScDPLevels::ScDPLevels( ScDPSource* pSrc, long nD, long nH ) :
    pSource( pSrc ),
    nDim( nD ),
    nHier( nH ),
    ppLevs( nullptr )
{
    //TODO: hold pSource

    //  text columns have only one level

    long nSrcDim = pSource->GetSourceDim( nDim );
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

    if (ppLevs)
    {
        for (long i=0; i<nLevCount; i++)
            if ( ppLevs[i] )
                ppLevs[i]->release();   // ref-counted
        delete[] ppLevs;
    }
}

// very simple XNameAccess implementation using getCount/getByIndex

uno::Any SAL_CALL ScDPLevels::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    long nCount = getCount();
    for (long i=0; i<nCount; i++)
        if ( getByIndex(i)->getName() == aName )
        {
            uno::Reference<container::XNamed> xNamed = getByIndex(i);
            uno::Any aRet;
            aRet <<= xNamed;
            return aRet;
        }

    throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScDPLevels::getElementNames() throw(uno::RuntimeException, std::exception)
{
    long nCount = getCount();
    uno::Sequence<OUString> aSeq(nCount);
    OUString* pArr = aSeq.getArray();
    for (long i=0; i<nCount; i++)
        pArr[i] = getByIndex(i)->getName();
    return aSeq;
}

sal_Bool SAL_CALL ScDPLevels::hasByName( const OUString& aName ) throw(uno::RuntimeException, std::exception)
{
    long nCount = getCount();
    for (long i=0; i<nCount; i++)
        if ( getByIndex(i)->getName() == aName )
            return true;
    return false;
}

uno::Type SAL_CALL ScDPLevels::getElementType() throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<container::XNamed>::get();
}

sal_Bool SAL_CALL ScDPLevels::hasElements() throw(uno::RuntimeException, std::exception)
{
    return ( getCount() > 0 );
}

// end of XNameAccess implementation

long ScDPLevels::getCount() const
{
    return nLevCount;
}

ScDPLevel* ScDPLevels::getByIndex(long nIndex) const
{
    if ( nIndex >= 0 && nIndex < nLevCount )
    {
        if ( !ppLevs )
        {
            const_cast<ScDPLevels*>(this)->ppLevs = new ScDPLevel*[nLevCount];
            for (long i=0; i<nLevCount; i++)
                ppLevs[i] = nullptr;
        }
        if ( !ppLevs[nIndex] )
        {
            ppLevs[nIndex] = new ScDPLevel( pSource, nDim, nHier, nIndex );
            ppLevs[nIndex]->acquire();      // ref-counted
        }

        return ppLevs[nIndex];
    }

    return nullptr;    //TODO: exception?
}

class ScDPGlobalMembersOrder
{
    ScDPLevel&  rLevel;
    bool        bAscending;

public:
            ScDPGlobalMembersOrder( ScDPLevel& rLev, bool bAsc ) :
                rLevel(rLev),
                bAscending(bAsc)
            {}
            ~ScDPGlobalMembersOrder() {}

    bool operator()( sal_Int32 nIndex1, sal_Int32 nIndex2 ) const;
};

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

ScDPLevel::ScDPLevel( ScDPSource* pSrc, long nD, long nH, long nL ) :
    pSource( pSrc ),
    nDim( nD ),
    nHier( nH ),
    nLev( nL ),
    pMembers( nullptr ),
    aSortInfo( EMPTY_OUSTRING, true, sheet::DataPilotFieldSortMode::NAME ),   // default: sort by name
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

    if ( pMembers )
        pMembers->release();    // ref-counted
}

void ScDPLevel::EvaluateSortOrder()
{
    switch (aSortInfo.Mode)
    {
        case sheet::DataPilotFieldSortMode::DATA:
            {
                // find index of measure (index among data dimensions)

                long nMeasureCount = pSource->GetDataDimensionCount();
                for (long nMeasure=0; nMeasure<nMeasureCount; nMeasure++)
                {
                    if (pSource->GetDataDimName(nMeasure).equals(aSortInfo.Field))
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
                long nCount = pLocalMembers->getCount();

                aGlobalOrder.resize( nCount );
                for (long nPos=0; nPos<nCount; nPos++)
                    aGlobalOrder[nPos] = nPos;

                // allow manual or name (manual is always ascending)
                bool bAscending = ( aSortInfo.Mode == sheet::DataPilotFieldSortMode::MANUAL || aSortInfo.IsAscending );
                ScDPGlobalMembersOrder aComp( *this, bAscending );
                ::std::sort( aGlobalOrder.begin(), aGlobalOrder.end(), aComp );
            }
            break;
    }

    if ( aAutoShowInfo.IsEnabled )
    {
        // find index of measure (index among data dimensions)

        long nMeasureCount = pSource->GetDataDimensionCount();
        for (long nMeasure=0; nMeasure<nMeasureCount; nMeasure++)
        {
            if (pSource->GetDataDimName(nMeasure).equals(aAutoShowInfo.DataField))
            {
                nAutoMeasure = nMeasure;
                break;
            }
        }

        //TODO: error if not found?
    }
}

void ScDPLevel::SetEnableLayout(bool bSet)
{
    bEnableLayout = bSet;
}

ScDPMembers* ScDPLevel::GetMembersObject()
{
    if (!pMembers)
    {
        pMembers = new ScDPMembers( pSource, nDim, nHier, nLev );
        pMembers->acquire();    // ref-counted
    }
    return pMembers;
}

uno::Reference<container::XNameAccess> SAL_CALL ScDPLevel::getMembers() throw(uno::RuntimeException, std::exception)
{
    return GetMembersObject();
}

uno::Sequence<sheet::MemberResult> SAL_CALL ScDPLevel::getResults() throw(uno::RuntimeException, std::exception)
{
    const uno::Sequence<sheet::MemberResult>* pRes = pSource->GetMemberResults( this );
    if (pRes)
        return *pRes;

    return uno::Sequence<sheet::MemberResult>(0);       //TODO: Error?
}

OUString SAL_CALL ScDPLevel::getName() throw(uno::RuntimeException, std::exception)
{
    long nSrcDim = pSource->GetSourceDim( nDim );
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

void SAL_CALL ScDPLevel::setName( const OUString& /* rNewName */ ) throw(uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");        //TODO: exception?
}

uno::Sequence<sheet::GeneralFunction> ScDPLevel::getSubTotals() const
{
    //TODO: separate functions for settings and evaluation?

    long nSrcDim = pSource->GetSourceDim( nDim );
    if ( !pSource->SubTotalAllowed( nSrcDim ) )
        return uno::Sequence<sheet::GeneralFunction>(0);

    return aSubTotals;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDPLevel::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    static const SfxItemPropertyMapEntry aDPLevelMap_Impl[] =
    {
        //TODO: change type of AutoShow/Layout/Sorting to API struct when available
        { OUString(SC_UNO_DP_AUTOSHOW), 0,  cppu::UnoType<sheet::DataPilotFieldAutoShowInfo>::get(),     0, 0 },
        { OUString(SC_UNO_DP_LAYOUT),   0,  cppu::UnoType<sheet::DataPilotFieldLayoutInfo>::get(),       0, 0 },
        { OUString(SC_UNO_DP_SHOWEMPTY), 0, cppu::UnoType<bool>::get(),                                   0, 0 },
        { OUString(SC_UNO_DP_REPEATITEMLABELS), 0, cppu::UnoType<bool>::get(),                                   0, 0 },
        { OUString(SC_UNO_DP_SORTING),  0,  cppu::UnoType<sheet::DataPilotFieldSortInfo>::get(),         0, 0 },
        { OUString(SC_UNO_DP_SUBTOTAL), 0,  cppu::UnoType<uno::Sequence<sheet::GeneralFunction>>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aDPLevelMap_Impl );
    return aRef;
}

void SAL_CALL ScDPLevel::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    if ( aPropertyName == SC_UNO_DP_SHOWEMPTY )
        bShowEmpty = lcl_GetBoolFromAny(aValue);
    else if ( aPropertyName == SC_UNO_DP_REPEATITEMLABELS )
        bRepeatItemLabels = lcl_GetBoolFromAny(aValue);
    else if ( aPropertyName == SC_UNO_DP_SUBTOTAL )
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
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    uno::Any aRet;
    if ( aPropertyName == SC_UNO_DP_SHOWEMPTY )
        lcl_SetBoolInAny(aRet, bShowEmpty);
    else if ( aPropertyName == SC_UNO_DP_REPEATITEMLABELS )
        lcl_SetBoolInAny(aRet, bRepeatItemLabels);
    else if ( aPropertyName == SC_UNO_DP_SUBTOTAL )
    {
        uno::Sequence<sheet::GeneralFunction> aSeq = getSubTotals();        //TODO: avoid extra copy?
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
        long nSrcDim = pSource->GetSourceDim(nDim);
        ScDPDimension* pDim = pSource->GetDimensionsObject()->getByIndex(nSrcDim);
        if (!pDim)
            return aRet;

        const OUString* pLayoutName = pDim->GetLayoutName();
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

ScDPMembers::ScDPMembers( ScDPSource* pSrc, long nD, long nH, long nL ) :
    pSource( pSrc ),
    nDim( nD ),
    nHier( nH ),
    nLev( nL )
{
    //TODO: hold pSource

    long nSrcDim = pSource->GetSourceDim( nDim );
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
                        for ( SCROW n = 0 ;n <GetSrcItemsCount() ; n-- )
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

                            long nFirstYear = pSource->GetData()->GetDatePart(
                                        (long)::rtl::math::approxFloor( fFirstVal ),
                                        nHier, nLev );
                            long nLastYear = pSource->GetData()->GetDatePart(
                                        (long)::rtl::math::approxFloor( fLastVal ),
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
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
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

uno::Sequence<OUString> SAL_CALL ScDPMembers::getElementNames() throw(uno::RuntimeException, std::exception)
{
    // Return list of names in sorted order,
    // so it's displayed in that order in the field options dialog.
    // Sorting is done at the level object (parent of this).

    ScDPLevel* pLevel = pSource->GetDimensionsObject()->getByIndex(nDim)->
        GetHierarchiesObject()->getByIndex(nHier)->GetLevelsObject()->getByIndex(nLev);
    pLevel->EvaluateSortOrder();
    const std::vector<sal_Int32>& rGlobalOrder = pLevel->GetGlobalOrder();
    bool bSort = !rGlobalOrder.empty();

    long nCount = getCount();
    uno::Sequence<OUString> aSeq(nCount);
    OUString* pArr = aSeq.getArray();
    for (long i=0; i<nCount; i++)
        pArr[i] = getByIndex(bSort ? rGlobalOrder[i] : i)->getName();
    return aSeq;
}

sal_Bool SAL_CALL ScDPMembers::hasByName( const OUString& aName ) throw(uno::RuntimeException, std::exception)
{
    return ( GetIndexFromName( aName ) >= 0 );
}

uno::Type SAL_CALL ScDPMembers::getElementType() throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<container::XNamed>::get();
}

sal_Bool SAL_CALL ScDPMembers::hasElements() throw(uno::RuntimeException, std::exception)
{
    return ( getCount() > 0 );
}

// end of XNameAccess implementation

long ScDPMembers::getMinMembers() const
{
    // used in lcl_CountMinMembers

    long nVisCount = 0;
    if (!maMembers.empty())
    {
        MembersType::const_iterator it = maMembers.begin(), itEnd = maMembers.end();
        for (; it != itEnd; ++it)
        {
            //  count only visible with details (default is true for both)
            const rtl::Reference<ScDPMember>& pMbr = *it;
            if (!pMbr.get() || (pMbr->isVisible() && pMbr->getShowDetails()))
                ++nVisCount;
        }
    }
    else
        nVisCount = nMbrCount;      // default for all

    return nVisCount;
}

ScDPMember* ScDPMembers::getByIndex(long nIndex) const
{
    //  result of GetColumnEntries must not change between ScDPMembers ctor
    //  and all calls to getByIndex

    if ( nIndex >= 0 && nIndex < nMbrCount )
    {
        if (maMembers.empty())
            maMembers.resize(nMbrCount);

        if (!maMembers[nIndex].get())
        {
            rtl::Reference<ScDPMember> pNew;
            long nSrcDim = pSource->GetSourceDim( nDim );
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
                    long nFirstYear = pSource->GetData()->GetDatePart(
                                        (long)::rtl::math::approxFloor( fFirstVal ),
                                        nHier, nLev );

                    nVal = nFirstYear + nIndex;
                }
                else if ( nHier == SC_DAPI_HIERARCHY_WEEK && nLev == SC_DAPI_LEVEL_WEEKDAY )
                {
                    nVal = nIndex;              // DayOfWeek is 0-based
                    aName = ScGlobal::GetCalendar()->getDisplayName(
                        css::i18n::CalendarDisplayIndex::DAY,
                        sal::static_int_cast<sal_Int16>(nVal), 0 );
                }
                else if ( nHier == SC_DAPI_HIERARCHY_QUARTER && nLev == SC_DAPI_LEVEL_MONTH )
                {
                    nVal = nIndex;              // Month is 0-based
                    aName = ScGlobal::GetCalendar()->getDisplayName(
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
    ScDPSource* pSrc, long nD, long nH, long nL, SCROW nIndex) :
    pSource( pSrc ),
    nDim( nD ),
    nHier( nH ),
    nLev( nL ),
    mnDataId( nIndex ),
    mpLayoutName(nullptr),
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
    long nSrcDim = pSource->GetSourceDim( nDim );
    if ( nHier != SC_DAPI_HIERARCHY_FLAT && pSource->IsDateDimension( nSrcDim ) )
    {
        const ScDPItemData* pData = pSource->GetCache()->GetItemDataById(nDim, nIndex);
        if (pData->IsValue())
        {
            long nComp = pSource->GetData()->GetDatePart(
                (long)::rtl::math::approxFloor( pData->GetValue() ),
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

void ScDPMember::FillItemData( ScDPItemData& rData ) const
{
    //TODO: handle date hierarchy...

    const ScDPItemData* pData = GetItemData();
    rData = (pData ? *pData : ScDPItemData());
}

const OUString* ScDPMember::GetLayoutName() const
{
    return mpLayoutName.get();
}

OUString ScDPMember::GetNameStr() const
{
    const ScDPItemData* pData = GetItemData();
    if (pData)
        return pSource->GetData()->GetFormattedString(nDim, *pData);
    return OUString();
}

OUString SAL_CALL ScDPMember::getName() throw(uno::RuntimeException, std::exception)
{
    return GetNameStr();
}

void SAL_CALL ScDPMember::setName( const OUString& /* rNewName */ ) throw(uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");        //TODO: exception?
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDPMember::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    static const SfxItemPropertyMapEntry aDPMemberMap_Impl[] =
    {
        { OUString(SC_UNO_DP_ISVISIBLE), 0,  cppu::UnoType<bool>::get(),              0, 0 },
        { OUString(SC_UNO_DP_POSITION), 0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
        { OUString(SC_UNO_DP_SHOWDETAILS), 0,  cppu::UnoType<bool>::get(),              0, 0 },
        { OUString(SC_UNO_DP_LAYOUTNAME), 0, cppu::UnoType<OUString>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aDPMemberMap_Impl );
    return aRef;
}

void SAL_CALL ScDPMember::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
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
            mpLayoutName.reset(new OUString(aName));
    }
    else
    {
        OSL_FAIL("unknown property");
    }
}

uno::Any SAL_CALL ScDPMember::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    uno::Any aRet;
    if ( aPropertyName == SC_UNO_DP_ISVISIBLE )
        lcl_SetBoolInAny(aRet, bVisible);
    else if ( aPropertyName == SC_UNO_DP_SHOWDETAILS )
        lcl_SetBoolInAny(aRet, bShowDet);
    else if ( aPropertyName == SC_UNO_DP_POSITION )
        aRet <<= nPosition;
    else if (aPropertyName == SC_UNO_DP_LAYOUTNAME)
        aRet <<= mpLayoutName.get() ? *mpLayoutName : OUString();
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
    SAL_WARN_IF( !pData, "sc", "ScDPMember::GetItemData: what data? nDim " << nDim << ", mnDataId " << mnDataId);
    return pData;
}

const ScDPItemData* ScDPSource::GetItemDataById(long nDim, long nId)
{
    return GetData()->GetMemberById(nDim, nId);
}

SCROW ScDPSource::GetMemberId(long nDim, const ScDPItemData& rData)
{
    return GetCache()->GetIdByItemData(nDim, rData);
}

const ScDPItemData* ScDPMembers::GetSrcItemDataByIndex(SCROW nIndex)
{
    const std::vector< SCROW >& memberIds = pSource->GetData()->GetColumnEntries( nDim );
    if ( nIndex >= (long )(memberIds.size()) || nIndex < 0 )
        return nullptr;
    SCROW nId =  memberIds[ nIndex ];
    return pSource->GetItemDataById( nDim, nId );
}

SCROW ScDPMembers::GetSrcItemsCount()
{
    return pSource->GetData()->GetColumnEntries(nDim).size();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
