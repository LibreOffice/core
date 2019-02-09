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

#include <dpcache.hxx>

#include <document.hxx>
#include <queryentry.hxx>
#include <queryparam.hxx>
#include <dpobject.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <docoptio.hxx>
#include <dpitemdata.hxx>
#include <dputil.hxx>
#include <dpnumgroupinfo.hxx>
#include <columniterator.hxx>
#include <cellvalue.hxx>

#include <rtl/math.hxx>
#include <unotools/textsearch.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <svl/zforlist.hxx>
#include <osl/diagnose.h>

#if DUMP_PIVOT_TABLE
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#endif

// TODO : Threaded pivot cache operation is disabled until we can figure out
// ways to make the edit engine and number formatter codes thread-safe in a
// proper fashion.
#define ENABLE_THREADED_PIVOT_CACHE 0

#if ENABLE_THREADED_PIVOT_CACHE
#include <thread>
#include <future>
#include <queue>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::uno::Exception;

ScDPCache::GroupItems::GroupItems() : mnGroupType(0) {}

ScDPCache::GroupItems::GroupItems(const ScDPNumGroupInfo& rInfo, sal_Int32 nGroupType) :
    maInfo(rInfo), mnGroupType(nGroupType) {}

ScDPCache::Field::Field() : mnNumFormat(0) {}

ScDPCache::ScDPCache(ScDocument* pDoc) :
    mpDoc( pDoc ),
    mnColumnCount ( 0 ),
    maEmptyRows(0, MAXROWCOUNT, true),
    mnDataSize(-1),
    mnRowCount(0),
    mbDisposing(false)
{
}

namespace {

struct ClearObjectSource
{
    void operator() (ScDPObject* p) const
    {
        p->ClearTableData();
    }
};

}

ScDPCache::~ScDPCache()
{
    // Make sure no live ScDPObject instances hold reference to this cache any
    // more.
    mbDisposing = true;
    std::for_each(maRefObjects.begin(), maRefObjects.end(), ClearObjectSource());
}

namespace {

/**
 * While the macro interpret level is incremented, the formula cells are
 * (semi-)guaranteed to be interpreted.
 */
class MacroInterpretIncrementer
{
public:
    explicit MacroInterpretIncrementer(ScDocument* pDoc) :
        mpDoc(pDoc)
    {
        mpDoc->IncMacroInterpretLevel();
    }
    ~MacroInterpretIncrementer()
    {
        mpDoc->DecMacroInterpretLevel();
    }
private:
    ScDocument* mpDoc;
};

rtl_uString* internString( ScDPCache::StringSetType& rPool, const OUString& rStr )
{
    ScDPCache::StringSetType::iterator it = rPool.find(rStr);
    if (it != rPool.end())
        // In the pool.
        return (*it).pData;

    std::pair<ScDPCache::StringSetType::iterator, bool> r = rPool.insert(rStr);
    return r.second ? (*r.first).pData : nullptr;
}

OUString createLabelString( const ScDocument* pDoc, SCCOL nCol, const ScRefCellValue& rCell )
{
    OUString aDocStr = rCell.getRawString(pDoc);

    if (aDocStr.isEmpty())
    {
        // Replace an empty label string with column name.
        OUStringBuffer aBuf;
        aBuf.append(ScResId(STR_COLUMN));
        aBuf.append(' ');

        ScAddress aColAddr(nCol, 0, 0);
        aBuf.append(aColAddr.Format(ScRefFlags::COL_VALID));
        aDocStr = aBuf.makeStringAndClear();
    }
    return aDocStr;
}

void initFromCell(
    ScDPCache::StringSetType& rStrPool, const ScDocument* pDoc, const ScAddress& rPos,
    const ScRefCellValue& rCell, ScDPItemData& rData, sal_uInt32& rNumFormat)
{
    OUString aDocStr = rCell.getRawString(pDoc);
    rNumFormat = 0;

    if (rCell.hasError())
    {
        rData.SetErrorStringInterned(internString(rStrPool, pDoc->GetString(rPos.Col(), rPos.Row(), rPos.Tab())));
    }
    else if (rCell.hasNumeric())
    {
        double fVal = rCell.getRawValue();
        rNumFormat = pDoc->GetNumberFormat(rPos);
        rData.SetValue(fVal);
    }
    else if (!rCell.isEmpty())
    {
        rData.SetStringInterned(internString(rStrPool, aDocStr));
    }
    else
        rData.SetEmpty();
}

struct Bucket
{
    ScDPItemData maValue;
    SCROW mnOrderIndex;
    SCROW mnDataIndex;
    Bucket(const ScDPItemData& rValue, SCROW nData) :
        maValue(rValue), mnOrderIndex(0), mnDataIndex(nData) {}
};

#if DEBUG_PIVOT_TABLE
#include <iostream>
using std::cout;
using std::endl;

struct PrintBucket
{
    void operator() (const Bucket& v) const
    {
        cout << "value: " << v.maValue.GetValue() << "  order index: " << v.mnOrderIndex << "  data index: " << v.mnDataIndex << endl;
    }
};

#endif

struct LessByValue
{
    bool operator() (const Bucket& left, const Bucket& right) const
    {
        return left.maValue < right.maValue;
    }
};

struct LessByOrderIndex
{
    bool operator() (const Bucket& left, const Bucket& right) const
    {
        return left.mnOrderIndex < right.mnOrderIndex;
    }
};

struct LessByDataIndex
{
    bool operator() (const Bucket& left, const Bucket& right) const
    {
        return left.mnDataIndex < right.mnDataIndex;
    }
};

struct EqualByOrderIndex
{
    bool operator() (const Bucket& left, const Bucket& right) const
    {
        return left.mnOrderIndex == right.mnOrderIndex;
    }
};

class PushBackValue
{
    ScDPCache::ScDPItemDataVec& mrItems;
public:
    explicit PushBackValue(ScDPCache::ScDPItemDataVec& _items) : mrItems(_items) {}
    void operator() (const Bucket& v)
    {
        mrItems.push_back(v.maValue);
    }
};

class PushBackOrderIndex
{
    ScDPCache::IndexArrayType& mrData;
public:
    explicit PushBackOrderIndex(ScDPCache::IndexArrayType& _items) : mrData(_items) {}
    void operator() (const Bucket& v)
    {
        mrData.push_back(v.mnOrderIndex);
    }
};

void processBuckets(std::vector<Bucket>& aBuckets, ScDPCache::Field& rField)
{
    if (aBuckets.empty())
        return;

    // Sort by the value.
    std::sort(aBuckets.begin(), aBuckets.end(), LessByValue());

    {
        // Set order index such that unique values have identical index value.
        SCROW nCurIndex = 0;
        std::vector<Bucket>::iterator it = aBuckets.begin(), itEnd = aBuckets.end();
        ScDPItemData aPrev = it->maValue;
        it->mnOrderIndex = nCurIndex;
        for (++it; it != itEnd; ++it)
        {
            if (!aPrev.IsCaseInsEqual(it->maValue))
                ++nCurIndex;

            it->mnOrderIndex = nCurIndex;
            aPrev = it->maValue;
        }
    }

    // Re-sort the bucket this time by the data index.
    std::sort(aBuckets.begin(), aBuckets.end(), LessByDataIndex());

    // Copy the order index series into the field object.
    rField.maData.reserve(aBuckets.size());
    std::for_each(aBuckets.begin(), aBuckets.end(), PushBackOrderIndex(rField.maData));

    // Sort by the value again.
    std::sort(aBuckets.begin(), aBuckets.end(), LessByOrderIndex());

    // Unique by value.
    std::vector<Bucket>::iterator itUniqueEnd =
        std::unique(aBuckets.begin(), aBuckets.end(), EqualByOrderIndex());

    // Copy the unique values into items.
    std::vector<Bucket>::iterator itBeg = aBuckets.begin();
    size_t nLen = distance(itBeg, itUniqueEnd);
    rField.maItems.reserve(nLen);
    std::for_each(itBeg, itUniqueEnd, PushBackValue(rField.maItems));
}

struct InitColumnData
{
    ScDPCache::EmptyRowsType maEmptyRows;
    OUString maLabel;

    ScDPCache::StringSetType* mpStrPool;
    ScDPCache::Field* mpField;

    SCCOL mnCol;

    InitColumnData() :
        maEmptyRows(0, MAXROWCOUNT, true),
        mpStrPool(nullptr),
        mpField(nullptr),
        mnCol(-1) {}

    void init( SCCOL nCol, ScDPCache::StringSetType* pStrPool, ScDPCache::Field* pField )
    {
        mpStrPool = pStrPool;
        mpField = pField;
        mnCol = nCol;
    }
};

struct InitDocData
{
    ScDocument* mpDoc;
    SCTAB mnDocTab;
    SCROW mnStartRow;
    SCROW mnEndRow;
    bool mbTailEmptyRows;

    InitDocData() :
        mpDoc(nullptr),
        mnDocTab(-1),
        mnStartRow(-1),
        mnEndRow(-1),
        mbTailEmptyRows(false) {}
};

typedef std::unordered_set<OUString> LabelSet;

class InsertLabel
{
    LabelSet& mrNames;
public:
    explicit InsertLabel(LabelSet& rNames) : mrNames(rNames) {}
    void operator() (const OUString& r)
    {
        mrNames.insert(r);
    }
};

std::vector<OUString> normalizeLabels( const std::vector<InitColumnData>& rColData )
{
    std::vector<OUString> aLabels(1u, ScResId(STR_PIVOT_DATA));

    LabelSet aExistingNames;

    for (const InitColumnData& rCol : rColData)
    {
        const OUString& rLabel = rCol.maLabel;
        sal_Int32 nSuffix = 1;
        OUString aNewLabel = rLabel;
        while (true)
        {
            if (!aExistingNames.count(aNewLabel))
            {
                // this is a unique label.
                aLabels.push_back(aNewLabel);
                aExistingNames.insert(aNewLabel);
                break;
            }

            // This name already exists.
            OUStringBuffer aBuf(rLabel);
            aBuf.append(++nSuffix);
            aNewLabel = aBuf.makeStringAndClear();
        }
    }

    return aLabels;
}

void initColumnFromDoc( InitDocData& rDocData, InitColumnData &rColData )
{
    ScDPCache::Field& rField = *rColData.mpField;
    ScDocument* pDoc = rDocData.mpDoc;
    SCTAB nDocTab = rDocData.mnDocTab;
    SCCOL nCol = rColData.mnCol;
    SCROW nStartRow = rDocData.mnStartRow;
    SCROW nEndRow = rDocData.mnEndRow;
    bool bTailEmptyRows = rDocData.mbTailEmptyRows;

    std::unique_ptr<sc::ColumnIterator> pIter =
        pDoc->GetColumnIterator(nDocTab, nCol, nStartRow, nEndRow);
    assert(pIter);
    assert(pIter->hasCell());

    ScDPItemData aData;

    rColData.maLabel = createLabelString(pDoc, nCol, pIter->getCell());
    pIter->next();

    std::vector<Bucket> aBuckets;
    aBuckets.reserve(nEndRow-nStartRow); // skip the topmost label cell.

    // Push back all original values.
    for (SCROW i = 0, n = nEndRow-nStartRow; i < n; ++i, pIter->next())
    {
        assert(pIter->hasCell());

        sal_uInt32 nNumFormat = 0;
        ScAddress aPos(nCol, pIter->getRow(), nDocTab);
        initFromCell(*rColData.mpStrPool, pDoc, aPos, pIter->getCell(), aData, nNumFormat);

        aBuckets.emplace_back(aData, i);

        if (!aData.IsEmpty())
        {
            rColData.maEmptyRows.insert_back(i, i+1, false);
            if (nNumFormat)
                // Only take non-default number format.
                rField.mnNumFormat = nNumFormat;
        }
    }

    processBuckets(aBuckets, rField);

    if (bTailEmptyRows)
    {
        // If the last item is not empty, append one. Note that the items
        // are sorted, and empty item should come last when sorted.
        if (rField.maItems.empty() || !rField.maItems.back().IsEmpty())
        {
            aData.SetEmpty();
            rField.maItems.push_back(aData);
        }
    }
}

#if ENABLE_THREADED_PIVOT_CACHE

class ThreadQueue
{
    using FutureType = std::future<void>;
    std::queue<FutureType> maQueue;
    std::mutex maMutex;
    std::condition_variable maCond;

    size_t mnMaxQueue;

public:
    ThreadQueue( size_t nMaxQueue ) : mnMaxQueue(nMaxQueue) {}

    void push( std::function<void()> aFunc )
    {
        std::unique_lock<std::mutex> lock(maMutex);

        while (maQueue.size() >= mnMaxQueue)
            maCond.wait(lock);

        FutureType f = std::async(std::launch::async, aFunc);
        maQueue.push(std::move(f));
        lock.unlock();

        maCond.notify_one();
    }

    void waitForOne()
    {
        std::unique_lock<std::mutex> lock(maMutex);

        while (maQueue.empty())
            maCond.wait(lock);

        FutureType ret = std::move(maQueue.front());
        maQueue.pop();
        lock.unlock();

        ret.get(); // This may throw if an exception was thrown on the async thread.

        maCond.notify_one();
    }
};

class ThreadScopedGuard
{
    std::thread maThread;
public:
    ThreadScopedGuard(std::thread thread) : maThread(std::move(thread)) {}
    ThreadScopedGuard(ThreadScopedGuard&& other) : maThread(std::move(other.maThread)) {}

    ThreadScopedGuard(const ThreadScopedGuard&) = delete;
    ThreadScopedGuard& operator= (const ThreadScopedGuard&) = delete;

    ~ThreadScopedGuard()
    {
        maThread.join();
    }
};

#endif

}

void ScDPCache::InitFromDoc(ScDocument* pDoc, const ScRange& rRange)
{
    Clear();

    InitDocData aDocData;
    aDocData.mpDoc = pDoc;

    // Make sure the formula cells within the data range are interpreted
    // during this call, for this method may be called from the interpretation
    // of GETPIVOTDATA, which disables nested formula interpretation without
    // increasing the macro level.
    MacroInterpretIncrementer aMacroInc(pDoc);

    aDocData.mnStartRow = rRange.aStart.Row();  // start of data
    aDocData.mnEndRow = rRange.aEnd.Row();

    // Sanity check
    if (!ValidRow(aDocData.mnStartRow) || !ValidRow(aDocData.mnEndRow) || aDocData.mnEndRow <= aDocData.mnStartRow)
        return;

    SCCOL nStartCol = rRange.aStart.Col();
    SCCOL nEndCol = rRange.aEnd.Col();
    aDocData.mnDocTab = rRange.aStart.Tab();

    mnColumnCount = nEndCol - nStartCol + 1;

    // this row count must include the trailing empty rows.
    mnRowCount = aDocData.mnEndRow - aDocData.mnStartRow; // skip the topmost label row.

    // Skip trailing empty rows if exists.
    SCCOL nCol1 = nStartCol, nCol2 = nEndCol;
    SCROW nRow1 = aDocData.mnStartRow, nRow2 = aDocData.mnEndRow;
    pDoc->ShrinkToDataArea(aDocData.mnDocTab, nCol1, nRow1, nCol2, nRow2);
    aDocData.mbTailEmptyRows = aDocData.mnEndRow > nRow2; // Trailing empty rows exist.
    aDocData.mnEndRow = nRow2;

    if (aDocData.mnEndRow <= aDocData.mnStartRow)
    {
        // Check this again since the end row position has changed. It's
        // possible that the new end row becomes lower than the start row
        // after the shrinkage.
        Clear();
        return;
    }

    maStringPools.resize(mnColumnCount);
    std::vector<InitColumnData> aColData(mnColumnCount);
    maFields.reserve(mnColumnCount);
    for (size_t i = 0; i < static_cast<size_t>(mnColumnCount); ++i)
        maFields.push_back(std::make_unique<Field>());

    maLabelNames.reserve(mnColumnCount+1);

    // Ensure that none of the formula cells in the data range are dirty.
    pDoc->EnsureFormulaCellResults(rRange);

#if ENABLE_THREADED_PIVOT_CACHE
    ThreadQueue aQueue(std::thread::hardware_concurrency());

    auto aFuncLaunchFieldThreads = [&]()
    {
        for (sal_uInt16 nCol = nStartCol; nCol <= nEndCol; ++nCol)
        {
            size_t nDim = nCol - nStartCol;
            InitColumnData& rColData = aColData[nDim];
            rColData.init(nCol, &maStringPools[nDim], maFields[nDim].get());

            auto func = [&aDocData,&rColData]()
            {
                initColumnFromDoc(aDocData, rColData);
            };

            aQueue.push(std::move(func));
        }
    };

    {
        // Launch a separate thread that in turn spawns async threads to populate the fields.
        std::thread t(aFuncLaunchFieldThreads);
        ThreadScopedGuard sg(std::move(t));

        // Wait for all the async threads to complete on the main thread.
        for (SCCOL i = 0; i < mnColumnCount; ++i)
            aQueue.waitForOne();
    }

#else
    for (sal_uInt16 nCol = nStartCol; nCol <= nEndCol; ++nCol)
    {
        size_t nDim = nCol - nStartCol;
        InitColumnData& rColData = aColData[nDim];
        rColData.init(nCol, &maStringPools[nDim], maFields[nDim].get());

        initColumnFromDoc(aDocData, rColData);
    }
#endif

    maLabelNames = normalizeLabels(aColData);

    // Merge all non-empty rows data.
    for (const InitColumnData& rCol : aColData)
    {
        EmptyRowsType::const_segment_iterator it = rCol.maEmptyRows.begin_segment();
        EmptyRowsType::const_segment_iterator ite = rCol.maEmptyRows.end_segment();
        EmptyRowsType::const_iterator pos = maEmptyRows.begin();

        for (; it != ite; ++it)
        {
            if (!it->value)
                // Non-empty segment found.  Record it.
                pos = maEmptyRows.insert(pos, it->start, it->end, false).first;
        }
    }

    PostInit();
}

bool ScDPCache::InitFromDataBase(DBConnector& rDB)
{
    Clear();

    try
    {
        mnColumnCount = rDB.getColumnCount();
        maStringPools.resize(mnColumnCount);
        maFields.clear();
        maFields.reserve(mnColumnCount);
        for (size_t i = 0; i < static_cast<size_t>(mnColumnCount); ++i)
            maFields.push_back(std::make_unique<Field>());

        // Get column titles and types.
        maLabelNames.clear();
        maLabelNames.reserve(mnColumnCount+1);

        for (sal_Int32 nCol = 0; nCol < mnColumnCount; ++nCol)
        {
            OUString aColTitle = rDB.getColumnLabel(nCol);
            AddLabel(aColTitle);
        }

        std::vector<Bucket> aBuckets;
        ScDPItemData aData;
        for (sal_Int32 nCol = 0; nCol < mnColumnCount; ++nCol)
        {
            if (!rDB.first())
                continue;

            aBuckets.clear();
            Field& rField = *maFields[nCol].get();
            SCROW nRow = 0;
            do
            {
                SvNumFormatType nFormatType = SvNumFormatType::UNDEFINED;
                aData.SetEmpty();
                rDB.getValue(nCol, aData, nFormatType);
                aBuckets.emplace_back(aData, nRow);
                if (!aData.IsEmpty())
                {
                    maEmptyRows.insert_back(nRow, nRow+1, false);
                    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
                    rField.mnNumFormat = pFormatter ? pFormatter->GetStandardFormat(nFormatType) : 0;
                }

                ++nRow;
            }
            while (rDB.next());

            processBuckets(aBuckets, rField);
        }

        rDB.finish();

        if (!maFields.empty())
            mnRowCount = maFields[0]->maData.size();

        PostInit();
        return true;
    }
    catch (const Exception&)
    {
        return false;
    }
}

bool ScDPCache::ValidQuery( SCROW nRow, const ScQueryParam &rParam) const
{
    if (!rParam.GetEntryCount())
        return true;

    if (!rParam.GetEntry(0).bDoQuery)
        return true;

    bool bMatchWholeCell = mpDoc->GetDocOptions().IsMatchWholeCell();

    SCSIZE nEntryCount = rParam.GetEntryCount();
    std::vector<bool> aPassed(nEntryCount, false);

    long nPos = -1;
    CollatorWrapper* pCollator = (rParam.bCaseSens ? ScGlobal::GetCaseCollator() :
                                  ScGlobal::GetCollator() );
    ::utl::TransliterationWrapper* pTransliteration = (rParam.bCaseSens ?
                                                       ScGlobal::GetCaseTransliteration() : ScGlobal::GetpTransliteration());

    for (size_t i = 0; i < nEntryCount && rParam.GetEntry(i).bDoQuery; ++i)
    {
        const ScQueryEntry& rEntry = rParam.GetEntry(i);
        const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
        // we can only handle one single direct query
        // #i115431# nField in QueryParam is the sheet column, not the field within the source range
        SCCOL nQueryCol = static_cast<SCCOL>(rEntry.nField);
        if ( nQueryCol < rParam.nCol1 )
            nQueryCol = rParam.nCol1;
        if ( nQueryCol > rParam.nCol2 )
            nQueryCol = rParam.nCol2;
        SCCOL nSourceField = nQueryCol - rParam.nCol1;
        SCROW nId = GetItemDataId( nSourceField, nRow, false );
        const ScDPItemData* pCellData = GetItemDataById( nSourceField, nId );

        bool bOk = false;

        if (rEntry.GetQueryItem().meType == ScQueryEntry::ByEmpty)
        {
            if (rEntry.IsQueryByEmpty())
                bOk = pCellData->IsEmpty();
            else
            {
                assert(rEntry.IsQueryByNonEmpty());
                bOk = !pCellData->IsEmpty();
            }
        }
        else if (rEntry.GetQueryItem().meType != ScQueryEntry::ByString && pCellData->IsValue())
        {   // by Value
            double nCellVal = pCellData->GetValue();

            switch (rEntry.eOp)
            {
                case SC_EQUAL :
                    bOk = ::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                    break;
                case SC_LESS :
                    bOk = (nCellVal < rItem.mfVal) && !::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                    break;
                case SC_GREATER :
                    bOk = (nCellVal > rItem.mfVal) && !::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                    break;
                case SC_LESS_EQUAL :
                    bOk = (nCellVal < rItem.mfVal) || ::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                    break;
                case SC_GREATER_EQUAL :
                    bOk = (nCellVal > rItem.mfVal) || ::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                    break;
                case SC_NOT_EQUAL :
                    bOk = !::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                    break;
                default:
                    bOk= false;
                    break;
            }
        }
        else if ((rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL)
                 || (rEntry.GetQueryItem().meType == ScQueryEntry::ByString
                     && pCellData->HasStringData() )
                )
        {   // by String
            OUString  aCellStr = pCellData->GetString();

            bool bRealWildOrRegExp = (rParam.eSearchType != utl::SearchParam::SearchType::Normal &&
                    ((rEntry.eOp == SC_EQUAL) || (rEntry.eOp == SC_NOT_EQUAL)));
            if (bRealWildOrRegExp)
            {
                sal_Int32 nStart = 0;
                sal_Int32 nEnd   = aCellStr.getLength();

                bool bMatch = rEntry.GetSearchTextPtr( rParam.eSearchType, rParam.bCaseSens, bMatchWholeCell )
                                ->SearchForward( aCellStr, &nStart, &nEnd );
                // from 614 on, nEnd is behind the found text
                if (bMatch && bMatchWholeCell
                    && (nStart != 0 || nEnd != aCellStr.getLength()))
                    bMatch = false;    // RegExp must match entire cell string

                bOk = ((rEntry.eOp == SC_NOT_EQUAL) ? !bMatch : bMatch);
            }
            else
            {
                if (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL)
                {
                    if (bMatchWholeCell)
                    {
                        // TODO: Use shared string for fast equality check.
                        OUString aStr = rEntry.GetQueryItem().maString.getString();
                        bOk = pTransliteration->isEqual(aCellStr, aStr);
                        bool bHasStar = false;
                        sal_Int32 nIndex;
                        if (( nIndex = aStr.indexOf('*') ) != -1)
                            bHasStar = true;
                        if (bHasStar && (nIndex>0))
                        {
                            for (sal_Int32 j=0;(j<nIndex) && (j< aCellStr.getLength()) ; j++)
                            {
                                if (aCellStr[j] == aStr[j])
                                {
                                    bOk=true;
                                }
                                else
                                {
                                    bOk=false;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        OUString aQueryStr = rEntry.GetQueryItem().maString.getString();
                        css::uno::Sequence< sal_Int32 > xOff;
                        const LanguageType nLang = ScGlobal::pSysLocale->GetLanguageTag().getLanguageType();
                        OUString aCell = pTransliteration->transliterate(
                            aCellStr, nLang, 0, aCellStr.getLength(), &xOff);
                        OUString aQuer = pTransliteration->transliterate(
                            aQueryStr, nLang, 0, aQueryStr.getLength(), &xOff);
                        bOk = (aCell.indexOf( aQuer ) != -1);
                    }
                    if (rEntry.eOp == SC_NOT_EQUAL)
                        bOk = !bOk;
                }
                else
                {   // use collator here because data was probably sorted
                    sal_Int32 nCompare = pCollator->compareString(
                        aCellStr, rEntry.GetQueryItem().maString.getString());
                    switch (rEntry.eOp)
                    {
                        case SC_LESS :
                            bOk = (nCompare < 0);
                            break;
                        case SC_GREATER :
                            bOk = (nCompare > 0);
                            break;
                        case SC_LESS_EQUAL :
                            bOk = (nCompare <= 0);
                            break;
                        case SC_GREATER_EQUAL :
                            bOk = (nCompare >= 0);
                            break;
                        case SC_NOT_EQUAL:
                            OSL_FAIL("SC_NOT_EQUAL");
                            break;
                        case SC_TOPVAL:
                        case SC_BOTVAL:
                        case SC_TOPPERC:
                        case SC_BOTPERC:
                        default:
                            break;
                    }
                }
            }
        }

        if (nPos == -1)
        {
            nPos++;
            aPassed[nPos] = bOk;
        }
        else
        {
            if (rEntry.eConnect == SC_AND)
            {
                aPassed[nPos] = aPassed[nPos] && bOk;
            }
            else
            {
                nPos++;
                aPassed[nPos] = bOk;
            }
        }
    }

    for (long j=1; j <= nPos; j++)
        aPassed[0] = aPassed[0] || aPassed[j];

    bool bRet = aPassed[0];
    return bRet;
}

ScDocument* ScDPCache::GetDoc() const
{
    return mpDoc;
}

long ScDPCache::GetColumnCount() const
{
    return mnColumnCount;
}

bool ScDPCache::IsRowEmpty(SCROW nRow) const
{
    bool bEmpty = true;
    maEmptyRows.search_tree(nRow, bEmpty);
    return bEmpty;
}

const ScDPCache::GroupItems* ScDPCache::GetGroupItems(long nDim) const
{
    if (nDim < 0)
        return nullptr;

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
        return maFields[nDim]->mpGroup.get();

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
        return maGroupFields[nDim].get();

    return nullptr;
}

OUString ScDPCache::GetDimensionName(std::vector<OUString>::size_type nDim) const
{
    OSL_ENSURE(nDim < maLabelNames.size()-1 , "ScDPTableDataCache::GetDimensionName");
    OSL_ENSURE(maLabelNames.size() == static_cast <sal_uInt16> (mnColumnCount+1), "ScDPTableDataCache::GetDimensionName");

    if ( nDim+1 < maLabelNames.size() )
    {
        return maLabelNames[nDim+1];
    }
    else
        return OUString();
}

void ScDPCache::PostInit()
{
    OSL_ENSURE(!maFields.empty(), "Cache not initialized!");

    maEmptyRows.build_tree();
    auto it = maEmptyRows.rbegin();
    OSL_ENSURE(it != maEmptyRows.rend(), "corrupt flat_segment_tree instance!");
    mnDataSize = maFields[0]->maData.size();
    ++it; // Skip the first position.
    OSL_ENSURE(it != maEmptyRows.rend(), "buggy version of flat_segment_tree is used.");
    if (it->second)
    {
        SCROW nLastNonEmpty = it->first - 1;
        if (nLastNonEmpty+1 < mnDataSize)
            mnDataSize = nLastNonEmpty+1;
    }
}

void ScDPCache::Clear()
{
    mnColumnCount = 0;
    mnRowCount = 0;
    maFields.clear();
    maLabelNames.clear();
    maGroupFields.clear();
    maEmptyRows.clear();
    maStringPools.clear();
}

void ScDPCache::AddLabel(const OUString& rLabel)
{

    if ( maLabelNames.empty() )
        maLabelNames.push_back(ScResId(STR_PIVOT_DATA));

    //reset name if needed
    LabelSet aExistingNames;
    std::for_each(maLabelNames.begin(), maLabelNames.end(), InsertLabel(aExistingNames));
    sal_Int32 nSuffix = 1;
    OUString aNewName = rLabel;
    while (true)
    {
        if (!aExistingNames.count(aNewName))
        {
            // unique name found!
            maLabelNames.push_back(aNewName);
            return;
        }

        // Name already exists.
        OUStringBuffer aBuf(rLabel);
        aBuf.append(++nSuffix);
        aNewName = aBuf.makeStringAndClear();
    }
}

SCROW ScDPCache::GetItemDataId(sal_uInt16 nDim, SCROW nRow, bool bRepeatIfEmpty) const
{
    OSL_ENSURE(nDim < mnColumnCount, "ScDPTableDataCache::GetItemDataId ");

    const Field& rField = *maFields[nDim].get();
    if (static_cast<size_t>(nRow) >= rField.maData.size())
    {
        // nRow is in the trailing empty rows area.
        if (bRepeatIfEmpty)
            nRow = rField.maData.size()-1; // Move to the last non-empty row.
        else
            // Return the last item, which should always be empty if the
            // initialization has skipped trailing empty rows.
            return rField.maItems.size()-1;

    }
    else if (bRepeatIfEmpty)
    {
        while (nRow > 0 && rField.maItems[rField.maData[nRow]].IsEmpty())
            --nRow;
    }

    return rField.maData[nRow];
}

const ScDPItemData* ScDPCache::GetItemDataById(long nDim, SCROW nId) const
{
    if (nDim < 0 || nId < 0)
        return nullptr;

    size_t nSourceCount = maFields.size();
    size_t nDimPos = static_cast<size_t>(nDim);
    size_t nItemId = static_cast<size_t>(nId);
    if (nDimPos < nSourceCount)
    {
        // source field.
        const Field& rField = *maFields[nDimPos].get();
        if (nItemId < rField.maItems.size())
            return &rField.maItems[nItemId];

        if (!rField.mpGroup)
            return nullptr;

        nItemId -= rField.maItems.size();
        const ScDPItemDataVec& rGI = rField.mpGroup->maItems;
        if (nItemId >= rGI.size())
            return nullptr;

        return &rGI[nItemId];
    }

    // Try group fields.
    nDimPos -= nSourceCount;
    if (nDimPos >= maGroupFields.size())
        return nullptr;

    const ScDPItemDataVec& rGI = maGroupFields[nDimPos]->maItems;
    if (nItemId >= rGI.size())
        return nullptr;

    return &rGI[nItemId];
}

size_t ScDPCache::GetFieldCount() const
{
    return maFields.size();
}

size_t ScDPCache::GetGroupFieldCount() const
{
    return maGroupFields.size();
}

SCROW ScDPCache::GetRowCount() const
{
    return mnRowCount;
}

SCROW ScDPCache::GetDataSize() const
{
    OSL_ENSURE(mnDataSize <= GetRowCount(), "Data size should never be larger than the row count.");
    return mnDataSize >= 0 ? mnDataSize : 0;
}

const ScDPCache::IndexArrayType* ScDPCache::GetFieldIndexArray( size_t nDim ) const
{
    if (nDim >= maFields.size())
        return nullptr;

    return &maFields[nDim]->maData;
}

const ScDPCache::ScDPItemDataVec& ScDPCache::GetDimMemberValues(SCCOL nDim) const
{
    OSL_ENSURE( nDim>=0 && nDim < mnColumnCount ," nDim < mnColumnCount ");
    return maFields.at(nDim)->maItems;
}

sal_uInt32 ScDPCache::GetNumberFormat( long nDim ) const
{
    if ( nDim >= mnColumnCount )
        return 0;

    // TODO: Find a way to determine the dominant number format in presence of
    // multiple number formats in the same field.
    return maFields[nDim]->mnNumFormat;
}

bool ScDPCache::IsDateDimension( long nDim ) const
{
    if (nDim >= mnColumnCount)
        return false;

    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
    if (!pFormatter)
        return false;

    SvNumFormatType eType = pFormatter->GetType(maFields[nDim]->mnNumFormat);
    return (eType == SvNumFormatType::DATE) || (eType == SvNumFormatType::DATETIME);
}

long ScDPCache::GetDimMemberCount(long nDim) const
{
    OSL_ENSURE( nDim>=0 && nDim < mnColumnCount ," ScDPTableDataCache::GetDimMemberCount : out of bound ");
    return maFields[nDim]->maItems.size();
}

SCCOL ScDPCache::GetDimensionIndex(const OUString& sName) const
{
    for (size_t i = 1; i < maLabelNames.size(); ++i)
    {
        if (maLabelNames[i] == sName)
            return static_cast<SCCOL>(i-1);
    }
    return -1;
}

rtl_uString* ScDPCache::InternString( size_t nDim, const OUString& rStr )
{
    assert(nDim < maStringPools.size());
    return internString(maStringPools[nDim], rStr);
}

void ScDPCache::AddReference(ScDPObject* pObj) const
{
    maRefObjects.insert(pObj);
}

void ScDPCache::RemoveReference(ScDPObject* pObj) const
{
    if (mbDisposing)
        // Object being deleted.
        return;

    maRefObjects.erase(pObj);
    if (maRefObjects.empty())
        mpDoc->GetDPCollection()->RemoveCache(this);
}

const ScDPCache::ScDPObjectSet& ScDPCache::GetAllReferences() const
{
    return maRefObjects;
}

SCROW ScDPCache::GetIdByItemData(long nDim, const ScDPItemData& rItem) const
{
    if (nDim < 0)
        return -1;

    if (nDim < mnColumnCount)
    {
        // source field.
        const ScDPItemDataVec& rItems = maFields[nDim]->maItems;
        for (size_t i = 0, n = rItems.size(); i < n; ++i)
        {
            if (rItems[i] == rItem)
                return i;
        }

        if (!maFields[nDim]->mpGroup)
            return -1;

        // grouped source field.
        const ScDPItemDataVec& rGI = maFields[nDim]->mpGroup->maItems;
        for (size_t i = 0, n = rGI.size(); i < n; ++i)
        {
            if (rGI[i] == rItem)
                return rItems.size() + i;
        }
        return -1;
    }

    // group field.
    nDim -= mnColumnCount;
    if (static_cast<size_t>(nDim) < maGroupFields.size())
    {
        const ScDPItemDataVec& rGI = maGroupFields[nDim]->maItems;
        for (size_t i = 0, n = rGI.size(); i < n; ++i)
        {
            if (rGI[i] == rItem)
                return i;
        }
    }

    return -1;
}

// static
sal_uInt32 ScDPCache::GetLocaleIndependentFormat( SvNumberFormatter& rFormatter, sal_uInt32 nNumFormat )
{
    // For a date or date+time format use ISO format so it works across locales
    // and can be matched against string based item queries. For time use 24h
    // format. All others use General format, no currency, percent, ...
    // Use en-US locale for all.
    switch (rFormatter.GetType( nNumFormat))
    {
        case SvNumFormatType::DATE:
            return rFormatter.GetFormatIndex( NF_DATE_ISO_YYYYMMDD, LANGUAGE_ENGLISH_US);
        break;
        case SvNumFormatType::TIME:
            return rFormatter.GetFormatIndex( NF_TIME_HHMMSS, LANGUAGE_ENGLISH_US);
        break;
        case SvNumFormatType::DATETIME:
            return rFormatter.GetFormatIndex( NF_DATETIME_ISO_YYYYMMDD_HHMMSS, LANGUAGE_ENGLISH_US);
        break;
        default:
            return rFormatter.GetFormatIndex( NF_NUMBER_STANDARD, LANGUAGE_ENGLISH_US);
    }
}

// static
OUString ScDPCache::GetLocaleIndependentFormattedNumberString( double fValue )
{
    return rtl::math::doubleToUString( fValue, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max, '.', true);
}

// static
OUString ScDPCache::GetLocaleIndependentFormattedString( double fValue,
        SvNumberFormatter& rFormatter, sal_uInt32 nNumFormat )
{
    nNumFormat = GetLocaleIndependentFormat( rFormatter, nNumFormat);
    if ((nNumFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
        return GetLocaleIndependentFormattedNumberString( fValue);

    OUString aStr;
    Color* pColor = nullptr;
    rFormatter.GetOutputString( fValue, nNumFormat, aStr, &pColor);
    return aStr;
}

OUString ScDPCache::GetFormattedString(long nDim, const ScDPItemData& rItem, bool bLocaleIndependent) const
{
    if (nDim < 0)
        return rItem.GetString();

    ScDPItemData::Type eType = rItem.GetType();
    if (eType == ScDPItemData::Value)
    {
        // Format value using the stored number format.
        SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
        if (pFormatter)
        {
            sal_uInt32 nNumFormat = GetNumberFormat(nDim);
            if (bLocaleIndependent)
                return GetLocaleIndependentFormattedString( rItem.GetValue(), *pFormatter, nNumFormat);

            OUString aStr;
            Color* pColor = nullptr;
            pFormatter->GetOutputString(rItem.GetValue(), nNumFormat, aStr, &pColor);
            return aStr;
        }

        // Last resort..
        return GetLocaleIndependentFormattedNumberString( rItem.GetValue());
    }

    if (eType == ScDPItemData::GroupValue)
    {
        ScDPItemData::GroupValueAttr aAttr = rItem.GetGroupValue();
        double fStart = 0.0, fEnd = 0.0;
        const GroupItems* p = GetGroupItems(nDim);
        if (p)
        {
            fStart = p->maInfo.mfStart;
            fEnd = p->maInfo.mfEnd;
        }
        return ScDPUtil::getDateGroupName(
            aAttr.mnGroupType, aAttr.mnValue, mpDoc->GetFormatTable(), fStart, fEnd);
    }

    if (eType == ScDPItemData::RangeStart)
    {
        double fVal = rItem.GetValue();
        const GroupItems* p = GetGroupItems(nDim);
        if (!p)
            return rItem.GetString();

        sal_Unicode cDecSep = ScGlobal::pLocaleData->getNumDecimalSep()[0];
        return ScDPUtil::getNumGroupName(fVal, p->maInfo, cDecSep, mpDoc->GetFormatTable());
    }

    return rItem.GetString();
}

SvNumberFormatter* ScDPCache::GetNumberFormatter() const
{
    return mpDoc->GetFormatTable();
}

long ScDPCache::AppendGroupField()
{
    maGroupFields.push_back(std::make_unique<GroupItems>());
    return static_cast<long>(maFields.size() + maGroupFields.size() - 1);
}

void ScDPCache::ResetGroupItems(long nDim, const ScDPNumGroupInfo& rNumInfo, sal_Int32 nGroupType)
{
    if (nDim < 0)
        return;

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
    {
        maFields.at(nDim)->mpGroup.reset(new GroupItems(rNumInfo, nGroupType));
        return;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
    {
        GroupItems& rGI = *maGroupFields[nDim].get();
        rGI.maItems.clear();
        rGI.maInfo = rNumInfo;
        rGI.mnGroupType = nGroupType;
    }
}

SCROW ScDPCache::SetGroupItem(long nDim, const ScDPItemData& rData)
{
    if (nDim < 0)
        return -1;

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
    {
        GroupItems& rGI = *maFields.at(nDim)->mpGroup;
        rGI.maItems.push_back(rData);
        SCROW nId = maFields[nDim]->maItems.size() + rGI.maItems.size() - 1;
        return nId;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
    {
        ScDPItemDataVec& rItems = maGroupFields.at(nDim)->maItems;
        rItems.push_back(rData);
        return rItems.size()-1;
    }

    return -1;
}

void ScDPCache::GetGroupDimMemberIds(long nDim, std::vector<SCROW>& rIds) const
{
    if (nDim < 0)
        return;

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
    {
        if (!maFields.at(nDim)->mpGroup)
            return;

        size_t nOffset = maFields[nDim]->maItems.size();
        const ScDPItemDataVec& rGI = maFields[nDim]->mpGroup->maItems;
        for (size_t i = 0, n = rGI.size(); i < n; ++i)
            rIds.push_back(static_cast<SCROW>(i + nOffset));

        return;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
    {
        const ScDPItemDataVec& rGI = maGroupFields.at(nDim)->maItems;
        for (size_t i = 0, n = rGI.size(); i < n; ++i)
            rIds.push_back(static_cast<SCROW>(i));
    }
}

namespace {

struct ClearGroupItems
{
    void operator() (const std::unique_ptr<ScDPCache::Field>& r) const
    {
        r->mpGroup.reset();
    }
};

}

void ScDPCache::ClearGroupFields()
{
    maGroupFields.clear();
}

void ScDPCache::ClearAllFields()
{
    ClearGroupFields();
    std::for_each(maFields.begin(), maFields.end(), ClearGroupItems());
}

const ScDPNumGroupInfo* ScDPCache::GetNumGroupInfo(long nDim) const
{
    if (nDim < 0)
        return nullptr;

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
    {
        if (!maFields.at(nDim)->mpGroup)
            return nullptr;

        return &maFields[nDim]->mpGroup->maInfo;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
        return &maGroupFields.at(nDim)->maInfo;

    return nullptr;
}

sal_Int32 ScDPCache::GetGroupType(long nDim) const
{
    if (nDim < 0)
        return 0;

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
    {
        if (!maFields.at(nDim)->mpGroup)
            return 0;

        return maFields[nDim]->mpGroup->mnGroupType;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
        return maGroupFields.at(nDim)->mnGroupType;

    return 0;
}

#if DUMP_PIVOT_TABLE

namespace {

void dumpItems(const ScDPCache& rCache, long nDim, const ScDPCache::ScDPItemDataVec& rItems, size_t nOffset)
{
    for (size_t i = 0; i < rItems.size(); ++i)
        cout << "      " << (i+nOffset) << ": " << rCache.GetFormattedString(nDim, rItems[i], false) << endl;
}

void dumpSourceData(const ScDPCache& rCache, long nDim, const ScDPCache::ScDPItemDataVec& rItems, const ScDPCache::IndexArrayType& rArray)
{
    for (const auto& rIndex : rArray)
        cout << "      '" << rCache.GetFormattedString(nDim, rItems[rIndex], false) << "'" << endl;
}

const char* getGroupTypeName(sal_Int32 nType)
{
    static const char* pNames[] = {
        "", "years", "quarters", "months", "days", "hours", "minutes", "seconds"
    };

    switch (nType)
    {
        case sheet::DataPilotFieldGroupBy::YEARS:    return pNames[1];
        case sheet::DataPilotFieldGroupBy::QUARTERS: return pNames[2];
        case sheet::DataPilotFieldGroupBy::MONTHS:   return pNames[3];
        case sheet::DataPilotFieldGroupBy::DAYS:     return pNames[4];
        case sheet::DataPilotFieldGroupBy::HOURS:    return pNames[5];
        case sheet::DataPilotFieldGroupBy::MINUTES:  return pNames[6];
        case sheet::DataPilotFieldGroupBy::SECONDS:  return pNames[7];
        default:
            ;
    }

    return pNames[0];
}

}

void ScDPCache::Dump() const
{
    // Change these flags to fit your debugging needs.
    bool bDumpItems = false;
    bool bDumpSourceData = false;

    cout << "--- pivot cache dump" << endl;
    {
        size_t i = 0;
        for (const auto& rxField : maFields)
        {
            const Field& fld = *rxField;
            cout << "* source dimension: " << GetDimensionName(i) << " (ID = " << i << ")" << endl;
            cout << "    item count: " << fld.maItems.size() << endl;
            if (bDumpItems)
                dumpItems(*this, i, fld.maItems, 0);
            if (fld.mpGroup)
            {
                cout << "    group item count: " << fld.mpGroup->maItems.size() << endl;
                cout << "    group type: " << getGroupTypeName(fld.mpGroup->mnGroupType) << endl;
                if (bDumpItems)
                    dumpItems(*this, i, fld.mpGroup->maItems, fld.maItems.size());
            }

            if (bDumpSourceData)
            {
                cout << "    source data (re-constructed):" << endl;
                dumpSourceData(*this, i, fld.maItems, fld.maData);
            }

            ++i;
        }
    }

    {
        size_t i = maFields.size();
        for (const auto& rxGroupField : maGroupFields)
        {
            const GroupItems& gi = *rxGroupField;
            cout << "* group dimension: (unnamed) (ID = " << i << ")" << endl;
            cout << "    item count: " << gi.maItems.size() << endl;
            cout << "    group type: " << getGroupTypeName(gi.mnGroupType) << endl;
            if (bDumpItems)
                dumpItems(*this, i, gi.maItems, 0);
            ++i;
        }
    }

    {
        struct { SCROW start; SCROW end; bool empty; } aRange;
        cout << "* empty rows: " << endl;
        mdds::flat_segment_tree<SCROW, bool>::const_iterator it = maEmptyRows.begin(), itEnd = maEmptyRows.end();
        if (it != itEnd)
        {
            aRange.start = it->first;
            aRange.empty = it->second;

            for (++it; it != itEnd; ++it)
            {
                aRange.end = it->first-1;
                cout << "    rows " << aRange.start << "-" << aRange.end << ": " << (aRange.empty ? "empty" : "not-empty") << endl;
                aRange.start = it->first;
                aRange.empty = it->second;
            }
        }
    }

    cout << "---" << endl;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
