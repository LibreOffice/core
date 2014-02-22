/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "dpcache.hxx"

#include "document.hxx"
#include "queryentry.hxx"
#include "queryparam.hxx"
#include "dpglobal.hxx"
#include "dpobject.hxx"
#include "globstr.hrc"
#include "docoptio.hxx"
#include "dpitemdata.hxx"
#include "dputil.hxx"
#include "dpnumgroupinfo.hxx"

#include <rtl/math.hxx>
#include <unotools/textsearch.hxx>
#include <unotools/localedatawrapper.hxx>
#include <svl/zforlist.hxx>

#if DEBUG_PIVOT_TABLE
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;

ScDPCache::GroupItems::GroupItems() : mnGroupType(0) {}

ScDPCache::GroupItems::GroupItems(const ScDPNumGroupInfo& rInfo, sal_Int32 nGroupType) :
    maInfo(rInfo), mnGroupType(nGroupType) {}

ScDPCache::Field::Field() : mnNumFormat(0) {}

ScDPCache::ScDPCache(ScDocument* pDoc) :
    mpDoc( pDoc ),
    mnColumnCount ( 0 ),
    maEmptyRows(0, MAXROW, true),
    mnDataSize(-1),
    mnRowCount(0),
    mbDisposing(false)
{
}

namespace {

struct ClearObjectSource : std::unary_function<ScDPObject*, void>
{
    void operator() (ScDPObject* p) const
    {
        p->ClearTableData();
    }
};

}

ScDPCache::~ScDPCache()
{
    
    
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
    MacroInterpretIncrementer(ScDocument* pDoc) :
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

OUString createLabelString(ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    OUString aDocStr = pDoc->GetString(nCol, nRow, nTab);
    if (aDocStr.isEmpty())
    {
        
        OUStringBuffer aBuf;
        aBuf.append(ScGlobal::GetRscString(STR_COLUMN));
        aBuf.append(' ');

        ScAddress aColAddr(nCol, 0, 0);
        aBuf.append(aColAddr.Format(SCA_VALID_COL, NULL));
        aDocStr = aBuf.makeStringAndClear();
    }
    return aDocStr;
}

void initFromCell(
    ScDPCache& rCache, ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab,
    ScDPItemData& rData, sal_uLong& rNumFormat)
{
    OUString aDocStr = pDoc->GetString(nCol, nRow, nTab);
    rNumFormat = 0;

    ScAddress aPos(nCol, nRow, nTab);

    if (pDoc->GetErrCode(aPos))
    {
        rData.SetErrorString(rCache.InternString(aDocStr));
    }
    else if (pDoc->HasValueData(nCol, nRow, nTab))
    {
        double fVal = pDoc->GetValue(aPos);
        rNumFormat = pDoc->GetNumberFormat(aPos);
        rData.SetValue(fVal);
    }
    else if (pDoc->HasData(nCol, nRow, nTab))
    {
        rData.SetString(rCache.InternString(aDocStr));
    }
    else
        rData.SetEmpty();
}

struct Bucket
{
    ScDPItemData maValue;
    SCROW mnOrderIndex;
    SCROW mnDataIndex;
    SCROW mnValueSortIndex;
    Bucket(const ScDPItemData& rValue, SCROW nOrder, SCROW nData) :
        maValue(rValue), mnOrderIndex(nOrder), mnDataIndex(nData), mnValueSortIndex(0) {}
};

#if DEBUG_PIVOT_TABLE
#include <iostream>
using std::cout;
using std::endl;

struct PrintBucket : std::unary_function<Bucket, void>
{
    void operator() (const Bucket& v) const
    {
        cout << "value: " << v.maValue.GetValue() << "  order index: " << v.mnOrderIndex << "  data index: " << v.mnDataIndex << "  value sort index: " << v.mnValueSortIndex << endl;
    }
};

#endif

struct LessByValue : std::binary_function<Bucket, Bucket, bool>
{
    bool operator() (const Bucket& left, const Bucket& right) const
    {
        return left.maValue < right.maValue;
    }
};

struct LessByValueSortIndex : std::binary_function<Bucket, Bucket, bool>
{
    bool operator() (const Bucket& left, const Bucket& right) const
    {
        return left.mnValueSortIndex < right.mnValueSortIndex;
    }
};

struct LessByDataIndex : std::binary_function<Bucket, Bucket, bool>
{
    bool operator() (const Bucket& left, const Bucket& right) const
    {
        return left.mnDataIndex < right.mnDataIndex;
    }
};

struct EqualByOrderIndex : std::binary_function<Bucket, Bucket, bool>
{
    bool operator() (const Bucket& left, const Bucket& right) const
    {
        return left.mnOrderIndex == right.mnOrderIndex;
    }
};

class PushBackValue : std::unary_function<Bucket, void>
{
    ScDPCache::ItemsType& mrItems;
public:
    PushBackValue(ScDPCache::ItemsType& _items) : mrItems(_items) {}
    void operator() (const Bucket& v)
    {
        mrItems.push_back(v.maValue);
    }
};

class PushBackOrderIndex : std::unary_function<Bucket, void>
{
    ScDPCache::IndexArrayType& mrData;
public:
    PushBackOrderIndex(ScDPCache::IndexArrayType& _items) : mrData(_items) {}
    void operator() (const Bucket& v)
    {
        mrData.push_back(v.mnOrderIndex);
    }
};

class TagValueSortOrder : std::unary_function<Bucket, void>
{
    SCROW mnCurIndex;
public:
    TagValueSortOrder() : mnCurIndex(0) {}
    void operator() (Bucket& v)
    {
        v.mnValueSortIndex = mnCurIndex++;
    }
};

void processBuckets(std::vector<Bucket>& aBuckets, ScDPCache::Field& rField)
{
    if (aBuckets.empty())
        return;

    
    std::sort(aBuckets.begin(), aBuckets.end(), LessByValue());

    
    std::for_each(aBuckets.begin(), aBuckets.end(), TagValueSortOrder());

    {
        
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

    
    std::sort(aBuckets.begin(), aBuckets.end(), LessByDataIndex());

    
    rField.maData.reserve(aBuckets.size());
    std::for_each(aBuckets.begin(), aBuckets.end(), PushBackOrderIndex(rField.maData));

    
    std::sort(aBuckets.begin(), aBuckets.end(), LessByValueSortIndex());

    
    std::vector<Bucket>::iterator itUniqueEnd =
        std::unique(aBuckets.begin(), aBuckets.end(), EqualByOrderIndex());

    
    std::vector<Bucket>::iterator itBeg = aBuckets.begin();
    size_t nLen = distance(itBeg, itUniqueEnd);
    rField.maItems.reserve(nLen);
    std::for_each(itBeg, itUniqueEnd, PushBackValue(rField.maItems));
}

}

bool ScDPCache::InitFromDoc(ScDocument* pDoc, const ScRange& rRange)
{
    Clear();

    
    
    
    
    MacroInterpretIncrementer aMacroInc(pDoc);

    SCROW nStartRow = rRange.aStart.Row();  
    SCROW nEndRow = rRange.aEnd.Row();

    
    if (!ValidRow(nStartRow) || !ValidRow(nEndRow) || nEndRow <= nStartRow)
        return false;

    sal_uInt16 nStartCol = rRange.aStart.Col();
    sal_uInt16 nEndCol = rRange.aEnd.Col();
    sal_uInt16 nDocTab = rRange.aStart.Tab();

    mnColumnCount = nEndCol - nStartCol + 1;

    
    mnRowCount = nEndRow - nStartRow; 

    
    SCCOL nCol1 = nStartCol, nCol2 = nEndCol;
    SCROW nRow1 = nStartRow, nRow2 = nEndRow;
    pDoc->ShrinkToDataArea(nDocTab, nCol1, nRow1, nCol2, nRow2);
    bool bTailEmptyRows = nEndRow > nRow2; 
    nEndRow = nRow2;

    if (nEndRow <= nStartRow)
    {
        
        
        
        Clear();
        return false;
    }

    maFields.reserve(mnColumnCount);
    for (size_t i = 0; i < static_cast<size_t>(mnColumnCount); ++i)
        maFields.push_back(new Field);

    maLabelNames.reserve(mnColumnCount+1);

    ScDPItemData aData;
    for (sal_uInt16 nCol = nStartCol; nCol <= nEndCol; ++nCol)
    {
        AddLabel(createLabelString(pDoc, nCol, nStartRow, nDocTab));
        Field& rField = maFields[nCol-nStartCol];
        std::vector<Bucket> aBuckets;
        aBuckets.reserve(nEndRow-nStartRow); 

        
        SCROW nOffset = nStartRow + 1;
        for (SCROW i = 0, n = nEndRow-nStartRow; i < n; ++i)
        {
            SCROW nRow = i + nOffset;
            sal_uLong nNumFormat = 0;
            initFromCell(*this, pDoc, nCol, nRow, nDocTab, aData, nNumFormat);
            aBuckets.push_back(Bucket(aData, 0, i));

            if (!aData.IsEmpty())
            {
                maEmptyRows.insert_back(i, i+1, false);
                if (nNumFormat)
                    
                    rField.mnNumFormat = nNumFormat;
            }
        }

        processBuckets(aBuckets, rField);

        if (bTailEmptyRows)
        {
            
            
            if (rField.maItems.empty() || !rField.maItems.back().IsEmpty())
            {
                aData.SetEmpty();
                rField.maItems.push_back(aData);
            }
        }
    }

    PostInit();
    return true;
}

bool ScDPCache::InitFromDataBase(DBConnector& rDB)
{
    Clear();

    try
    {
        mnColumnCount = rDB.getColumnCount();
        maFields.clear();
        maFields.reserve(mnColumnCount);
        for (size_t i = 0; i < static_cast<size_t>(mnColumnCount); ++i)
            maFields.push_back(new Field);

        
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
            Field& rField = maFields[nCol];
            SCROW nRow = 0;
            do
            {
                short nFormatType = NUMBERFORMAT_UNDEFINED;
                aData.SetEmpty();
                rDB.getValue(nCol, aData, nFormatType);
                aBuckets.push_back(Bucket(aData, 0, nRow));
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
            mnRowCount = maFields[0].maData.size();

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
        
        
        SCCOL nQueryCol = (SCCOL)rEntry.nField;
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
                OSL_ASSERT(rEntry.IsQueryByNonEmpty());
                bOk = !pCellData->IsEmpty();
            }
        }
        else if (rEntry.GetQueryItem().meType != ScQueryEntry::ByString && pCellData->IsValue())
        {   
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
        {   
            OUString  aCellStr = pCellData->GetString();

            bool bRealRegExp = (rParam.bRegExp && ((rEntry.eOp == SC_EQUAL)
                                                   || (rEntry.eOp == SC_NOT_EQUAL)));
            bool bTestRegExp = false;
            if (bRealRegExp || bTestRegExp)
            {
                sal_Int32 nStart = 0;
                sal_Int32 nEnd   = aCellStr.getLength();

                bool bMatch = (bool) rEntry.GetSearchTextPtr( rParam.bCaseSens )
                              ->SearchForward( aCellStr, &nStart, &nEnd );
                
                if (bMatch && bMatchWholeCell
                    && (nStart != 0 || nEnd != aCellStr.getLength()))
                    bMatch = false;    
                if (bRealRegExp)
                    bOk = ((rEntry.eOp == SC_NOT_EQUAL) ? !bMatch : bMatch);
            }
            if (!bRealRegExp)
            {
                if (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL)
                {
                    if (bMatchWholeCell)
                    {
                        
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
                        ::com::sun::star::uno::Sequence< sal_Int32 > xOff;
                        OUString aCell = pTransliteration->transliterate(
                            aCellStr, ScGlobal::eLnge, 0, aCellStr.getLength(), &xOff);
                        OUString aQuer = pTransliteration->transliterate(
                            aQueryStr, ScGlobal::eLnge, 0, aQueryStr.getLength(), &xOff);
                        bOk = (aCell.indexOf( aQuer ) != -1);
                    }
                    if (rEntry.eOp == SC_NOT_EQUAL)
                        bOk = !bOk;
                }
                else
                {   
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

bool ScDPCache::IsRowEmpty(SCROW nRow) const
{
    bool bEmpty = true;
    maEmptyRows.search_tree(nRow, bEmpty);
    return bEmpty;
}

const ScDPCache::GroupItems* ScDPCache::GetGroupItems(long nDim) const
{
    if (nDim < 0)
        return NULL;

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
        return maFields[nDim].mpGroup.get();

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
        return &maGroupFields[nDim];

    return NULL;
}

OUString ScDPCache::GetDimensionName(LabelsType::size_type nDim) const
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

namespace {

typedef boost::unordered_set<OUString, OUStringHash> LabelSet;

class InsertLabel : public std::unary_function<OUString, void>
{
    LabelSet& mrNames;
public:
    InsertLabel(LabelSet& rNames) : mrNames(rNames) {}
    void operator() (const OUString& r)
    {
        mrNames.insert(r);
    }
};

}

void ScDPCache::PostInit()
{
    OSL_ENSURE(!maFields.empty(), "Cache not initialized!");

    maEmptyRows.build_tree();
    typedef mdds::flat_segment_tree<SCROW, bool>::const_reverse_iterator itr_type;
    itr_type it = maEmptyRows.rbegin();
    OSL_ENSURE(it != maEmptyRows.rend(), "corrupt flat_segment_tree instance!");
    mnDataSize = maFields[0].maData.size();
    ++it; 
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
    maStringPool.clear();
}

void ScDPCache::AddLabel(const OUString& rLabel)
{

    if ( maLabelNames.empty() )
        maLabelNames.push_back(ScGlobal::GetRscString(STR_PIVOT_DATA));

    
    LabelSet aExistingNames;
    std::for_each(maLabelNames.begin(), maLabelNames.end(), InsertLabel(aExistingNames));
    sal_Int32 nSuffix = 1;
    OUString aNewName = rLabel;
    while (true)
    {
        if (!aExistingNames.count(aNewName))
        {
            
            maLabelNames.push_back(aNewName);
            return;
        }

        
        OUStringBuffer aBuf(rLabel);
        aBuf.append(++nSuffix);
        aNewName = aBuf.makeStringAndClear();
    }
}

SCROW ScDPCache::GetItemDataId(sal_uInt16 nDim, SCROW nRow, bool bRepeatIfEmpty) const
{
    OSL_ENSURE(nDim < mnColumnCount, "ScDPTableDataCache::GetItemDataId ");

    const Field& rField = maFields[nDim];
    if (static_cast<size_t>(nRow) >= rField.maData.size())
    {
        
        if (bRepeatIfEmpty)
            nRow = rField.maData.size()-1; 
        else
            
            
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
        return NULL;

    size_t nSourceCount = maFields.size();
    size_t nDimPos = static_cast<size_t>(nDim);
    size_t nItemId = static_cast<size_t>(nId);
    if (nDimPos < nSourceCount)
    {
        
        const Field& rField = maFields[nDimPos];
        if (nItemId < rField.maItems.size())
            return &rField.maItems[nItemId];

        if (!rField.mpGroup)
            return NULL;

        nItemId -= rField.maItems.size();
        const ItemsType& rGI = rField.mpGroup->maItems;
        if (nItemId >= rGI.size())
            return NULL;

        return &rGI[nItemId];
    }

    
    nDimPos -= nSourceCount;
    if (nDimPos >= maGroupFields.size())
        return NULL;

    const ItemsType& rGI = maGroupFields[nDimPos].maItems;
    if (nItemId >= rGI.size())
        return NULL;

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

const ScDPCache::ItemsType& ScDPCache::GetDimMemberValues(SCCOL nDim) const
{
    OSL_ENSURE( nDim>=0 && nDim < mnColumnCount ," nDim < mnColumnCount ");
    return maFields.at(nDim).maItems;
}

sal_uLong ScDPCache::GetNumberFormat( long nDim ) const
{
    if ( nDim >= mnColumnCount )
        return 0;

    
    
    return maFields[nDim].mnNumFormat;
}

bool ScDPCache::IsDateDimension( long nDim ) const
{
    if (nDim >= mnColumnCount)
        return false;

    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
    if (!pFormatter)
        return false;

    short eType = pFormatter->GetType(maFields[nDim].mnNumFormat);
    return (eType == NUMBERFORMAT_DATE) || (eType == NUMBERFORMAT_DATETIME);
}

long ScDPCache::GetDimMemberCount(long nDim) const
{
    OSL_ENSURE( nDim>=0 && nDim < mnColumnCount ," ScDPTableDataCache::GetDimMemberCount : out of bound ");
    return maFields[nDim].maItems.size();
}

SCCOL ScDPCache::GetDimensionIndex(const OUString& sName) const
{
    for (size_t i = 1; i < maLabelNames.size(); ++i)
    {
        if (maLabelNames[i].equals(sName))
            return static_cast<SCCOL>(i-1);
    }
    return -1;
}

const OUString* ScDPCache::InternString(const OUString& rStr) const
{
    StringSetType::iterator it = maStringPool.find(rStr);
    if (it != maStringPool.end())
        
        return &(*it);

    std::pair<StringSetType::iterator, bool> r = maStringPool.insert(rStr);
    return r.second ? &(*r.first) : NULL;
}

void ScDPCache::AddReference(ScDPObject* pObj) const
{
    maRefObjects.insert(pObj);
}

void ScDPCache::RemoveReference(ScDPObject* pObj) const
{
    if (mbDisposing)
        
        return;

    maRefObjects.erase(pObj);
    if (maRefObjects.empty())
        mpDoc->GetDPCollection()->RemoveCache(this);
}

const ScDPCache::ObjectSetType& ScDPCache::GetAllReferences() const
{
    return maRefObjects;
}

SCROW ScDPCache::GetIdByItemData(long nDim, const ScDPItemData& rItem) const
{
    if (nDim < 0)
        return -1;

    if (nDim < mnColumnCount)
    {
        
        const ItemsType& rItems = maFields[nDim].maItems;
        for (size_t i = 0, n = rItems.size(); i < n; ++i)
        {
            if (rItems[i] == rItem)
                return i;
        }

        if (!maFields[nDim].mpGroup)
            return -1;

        
        const ItemsType& rGI = maFields[nDim].mpGroup->maItems;
        for (size_t i = 0, n = rGI.size(); i < n; ++i)
        {
            if (rGI[i] == rItem)
                return rItems.size() + i;
        }
        return -1;
    }

    
    nDim -= mnColumnCount;
    if (static_cast<size_t>(nDim) < maGroupFields.size())
    {
        const ItemsType& rGI = maGroupFields[nDim].maItems;
        for (size_t i = 0, n = rGI.size(); i < n; ++i)
        {
            if (rGI[i] == rItem)
                return i;
        }
    }

    return -1;
}

OUString ScDPCache::GetFormattedString(long nDim, const ScDPItemData& rItem) const
{
    if (nDim < 0)
        return rItem.GetString();

    ScDPItemData::Type eType = rItem.GetType();
    if (eType == ScDPItemData::Value)
    {
        
        sal_uLong nNumFormat = GetNumberFormat(nDim);
        SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
        if (pFormatter)
        {
            Color* pColor = NULL;
            OUString aStr;
            pFormatter->GetOutputString(rItem.GetValue(), nNumFormat, aStr, &pColor);
            return aStr;
        }
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

long ScDPCache::AppendGroupField()
{
    maGroupFields.push_back(new GroupItems);
    return static_cast<long>(maFields.size() + maGroupFields.size() - 1);
}

void ScDPCache::ResetGroupItems(long nDim, const ScDPNumGroupInfo& rNumInfo, sal_Int32 nGroupType)
{
    if (nDim < 0)
        return;

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
    {
        maFields.at(nDim).mpGroup.reset(new GroupItems(rNumInfo, nGroupType));
        return;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
    {
        GroupItems& rGI = maGroupFields[nDim];
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
        GroupItems& rGI = *maFields.at(nDim).mpGroup;
        rGI.maItems.push_back(rData);
        SCROW nId = maFields[nDim].maItems.size() + rGI.maItems.size() - 1;
        return nId;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
    {
        ItemsType& rItems = maGroupFields.at(nDim).maItems;
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
        if (!maFields.at(nDim).mpGroup)
            return;

        size_t nOffset = maFields[nDim].maItems.size();
        const ItemsType& rGI = maFields[nDim].mpGroup->maItems;
        for (size_t i = 0, n = rGI.size(); i < n; ++i)
            rIds.push_back(static_cast<SCROW>(i + nOffset));

        return;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
    {
        const ItemsType& rGI = maGroupFields.at(nDim).maItems;
        for (size_t i = 0, n = rGI.size(); i < n; ++i)
            rIds.push_back(static_cast<SCROW>(i));
    }
}

namespace {

struct ClearGroupItems : std::unary_function<ScDPCache::Field, void>
{
    void operator() (ScDPCache::Field& r) const
    {
        r.mpGroup.reset();
    }
};

}

void ScDPCache::ClearGroupFields()
{
    maGroupFields.clear();
    std::for_each(maFields.begin(), maFields.end(), ClearGroupItems());
}

const ScDPNumGroupInfo* ScDPCache::GetNumGroupInfo(long nDim) const
{
    if (nDim < 0)
        return NULL;

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
    {
        if (!maFields.at(nDim).mpGroup)
            return NULL;

        return &maFields[nDim].mpGroup->maInfo;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
        return &maGroupFields.at(nDim).maInfo;

    return NULL;
}

sal_Int32 ScDPCache::GetGroupType(long nDim) const
{
    if (nDim < 0)
        return 0;

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
    {
        if (!maFields.at(nDim).mpGroup)
            return 0;

        return maFields[nDim].mpGroup->mnGroupType;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
        return maGroupFields.at(nDim).mnGroupType;

    return 0;
}

SCROW ScDPCache::GetOrder(long /*nDim*/, SCROW nIndex) const
{
    return nIndex;
}

ScDocument* ScDPCache::GetDoc() const
{
    return mpDoc;
};

long ScDPCache::GetColumnCount() const
{
    return mnColumnCount;
}

#if DEBUG_PIVOT_TABLE

namespace {

std::ostream& operator<< (::std::ostream& os, const OUString& str)
{
    return os << OUStringToOString(str, RTL_TEXTENCODING_UTF8).getStr();
}

void dumpItems(const ScDPCache& rCache, long nDim, const ScDPCache::ItemsType& rItems, size_t nOffset)
{
    for (size_t i = 0; i < rItems.size(); ++i)
        cout << "      " << (i+nOffset) << ": " << rCache.GetFormattedString(nDim, rItems[i]) << endl;
}

void dumpSourceData(const ScDPCache& rCache, long nDim, const ScDPCache::ItemsType& rItems, const ScDPCache::IndexArrayType& rArray)
{
    ScDPCache::IndexArrayType::const_iterator it = rArray.begin(), itEnd = rArray.end();
    for (; it != itEnd; ++it)
        cout << "      '" << rCache.GetFormattedString(nDim, rItems[*it]) << "'" << endl;
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
    
    bool bDumpItems = false;
    bool bDumpSourceData = false;

    cout << "--- pivot cache dump" << endl;
    {
        FieldsType::const_iterator it = maFields.begin(), itEnd = maFields.end();
        for (size_t i = 0; it != itEnd; ++it, ++i)
        {
            const Field& fld = *it;
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
        }
    }

    {
        GroupFieldsType::const_iterator it = maGroupFields.begin(), itEnd = maGroupFields.end();
        for (size_t i = maFields.size(); it != itEnd; ++it, ++i)
        {
            const GroupItems& gi = *it;
            cout << "* group dimension: (unnamed) (ID = " << i << ")" << endl;
            cout << "    item count: " << gi.maItems.size() << endl;
            cout << "    group type: " << getGroupTypeName(gi.mnGroupType) << endl;
            if (bDumpItems)
                dumpItems(*this, i, gi.maItems, 0);
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
            ++it;
        }

        for (; it != itEnd; ++it)
        {
            aRange.end = it->first-1;
            cout << "    rows " << aRange.start << "-" << aRange.end << ": " << (aRange.empty ? "empty" : "not-empty") << endl;
            aRange.start = it->first;
            aRange.empty = it->second;
        }
    }

    cout << "---" << endl;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
