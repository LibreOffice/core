/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>

#include <memory>

const double D_TIMEFACTOR = 86400.0;

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

rtl::OUString createLabelString(ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    rtl::OUString aDocStr = pDoc->GetString(nCol, nRow, nTab);
    if (aDocStr.isEmpty())
    {
        // Replace an empty label string with column name.
        rtl::OUStringBuffer aBuf;
        aBuf.append(ScGlobal::GetRscString(STR_COLUMN));
        aBuf.append(sal_Unicode(' '));

        ScAddress aColAddr(nCol, 0, 0);
        rtl::OUString aColStr;
        aColAddr.Format(aColStr, SCA_VALID_COL, NULL);
        aBuf.append(aColStr);
        aDocStr = aBuf.makeStringAndClear();
    }
    return aDocStr;
}

void initFromCell(
    ScDPCache& rCache, ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab,
    ScDPItemData& rData, sal_uLong& rNumFormat)
{
    rtl::OUString aDocStr = pDoc->GetString(nCol, nRow, nTab);
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

void getItemValue(
    ScDPCache& rCache, ScDPItemData& rData, const Reference<sdbc::XRow>& xRow, sal_Int32 nType,
    long nCol, const Date& rNullDate, short& rNumType)
{
    rNumType = NUMBERFORMAT_NUMBER;
    try
    {
        double fValue = 0.0;
        switch (nType)
        {
            case sdbc::DataType::BIT:
            case sdbc::DataType::BOOLEAN:
            {
                rNumType = NUMBERFORMAT_LOGICAL;
                fValue  = xRow->getBoolean(nCol) ? 1 : 0;
                rData.SetValue(fValue);
                break;
            }
            case sdbc::DataType::TINYINT:
            case sdbc::DataType::SMALLINT:
            case sdbc::DataType::INTEGER:
            case sdbc::DataType::BIGINT:
            case sdbc::DataType::FLOAT:
            case sdbc::DataType::REAL:
            case sdbc::DataType::DOUBLE:
            case sdbc::DataType::NUMERIC:
            case sdbc::DataType::DECIMAL:
            {
                //! do the conversion here?
                fValue = xRow->getDouble(nCol);
                rData.SetValue(fValue);
                break;
            }
            case sdbc::DataType::DATE:
            {
                rNumType = NUMBERFORMAT_DATE;

                util::Date aDate = xRow->getDate(nCol);
                fValue = Date(aDate.Day, aDate.Month, aDate.Year) - rNullDate;
                rData.SetValue(fValue);
                break;
            }
            case sdbc::DataType::TIME:
            {
                rNumType = NUMBERFORMAT_TIME;

                util::Time aTime = xRow->getTime(nCol);
                fValue = ( aTime.Hours * 3600 + aTime.Minutes * 60 +
                           aTime.Seconds + aTime.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
                rData.SetValue(fValue);
                break;
            }
            case sdbc::DataType::TIMESTAMP:
            {
                rNumType = NUMBERFORMAT_DATETIME;

                util::DateTime aStamp = xRow->getTimestamp(nCol);
                fValue = ( Date( aStamp.Day, aStamp.Month, aStamp.Year ) - rNullDate ) +
                         ( aStamp.Hours * 3600 + aStamp.Minutes * 60 +
                           aStamp.Seconds + aStamp.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
                rData.SetValue(fValue);
                break;
            }
            case sdbc::DataType::CHAR:
            case sdbc::DataType::VARCHAR:
            case sdbc::DataType::LONGVARCHAR:
            case sdbc::DataType::SQLNULL:
            case sdbc::DataType::BINARY:
            case sdbc::DataType::VARBINARY:
            case sdbc::DataType::LONGVARBINARY:
            default:
                rData.SetString(rCache.InternString(xRow->getString(nCol)));
        }
    }
    catch (uno::Exception&)
    {
    }
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

struct EqualByValue : std::binary_function<Bucket, Bucket, bool>
{
    bool operator() (const Bucket& left, const Bucket& right) const
    {
        return left.maValue == right.maValue;
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

    // Sort by the value.
    std::sort(aBuckets.begin(), aBuckets.end(), LessByValue());

    // Remember this sort order.
    std::for_each(aBuckets.begin(), aBuckets.end(), TagValueSortOrder());

    {
        // Set order index such that unique values have identical index value.
        SCROW nCurIndex = 0;
        std::vector<Bucket>::iterator it = aBuckets.begin(), itEnd = aBuckets.end();
        ScDPItemData aPrev = it->maValue;
        it->mnOrderIndex = nCurIndex;
        for (++it; it != itEnd; ++it)
        {
            if (aPrev != it->maValue)
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
    std::sort(aBuckets.begin(), aBuckets.end(), LessByValueSortIndex());

    // Unique by value.
    std::vector<Bucket>::iterator itUniqueEnd =
        std::unique(aBuckets.begin(), aBuckets.end(), EqualByValue());

    // Copy the unique values into items.
    std::vector<Bucket>::iterator itBeg = aBuckets.begin();
    size_t nLen = distance(itBeg, itUniqueEnd);
    rField.maItems.reserve(nLen);
    std::for_each(itBeg, itUniqueEnd, PushBackValue(rField.maItems));
}

}

bool ScDPCache::InitFromDoc(ScDocument* pDoc, const ScRange& rRange)
{
    Clear();

    // Make sure the formula cells within the data range are interpreted
    // during this call, for this method may be called from the interpretation
    // of GETPIVOTDATA, which disables nested formula interpretation without
    // increasing the macro level.
    MacroInterpretIncrementer aMacroInc(pDoc);

    SCROW nStartRow = rRange.aStart.Row();  // start of data
    SCROW nEndRow = rRange.aEnd.Row();
    sal_uInt16 nStartCol = rRange.aStart.Col();
    sal_uInt16 nEndCol = rRange.aEnd.Col();
    sal_uInt16 nDocTab = rRange.aStart.Tab();

    mnColumnCount = nEndCol - nStartCol + 1;

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
        aBuckets.reserve(nEndRow-nStartRow); // skip the topmost label cell.

        // Push back all original values.
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
                rField.mnNumFormat = nNumFormat;
            }
        }

        processBuckets(aBuckets, rField);
    }

    PostInit();
    return true;
}

bool ScDPCache::InitFromDataBase (const Reference<sdbc::XRowSet>& xRowSet, const Date& rNullDate)
{
    Clear();

    if (!xRowSet.is())
        // Don't even waste time to go any further.
        return false;
    try
    {
        Reference<sdbc::XResultSetMetaDataSupplier> xMetaSupp(xRowSet, UNO_QUERY_THROW);
        Reference<sdbc::XResultSetMetaData> xMeta = xMetaSupp->getMetaData();
        if (!xMeta.is())
            return false;

        mnColumnCount = xMeta->getColumnCount();
        maFields.clear();
        maFields.reserve(mnColumnCount);
        for (size_t i = 0; i < static_cast<size_t>(mnColumnCount); ++i)
            maFields.push_back(new Field);

        // Get column titles and types.
        maLabelNames.clear();
        maLabelNames.reserve(mnColumnCount+1);

        std::vector<sal_Int32> aColTypes(mnColumnCount);

        for (sal_Int32 nCol = 0; nCol < mnColumnCount; ++nCol)
        {
            rtl::OUString aColTitle = xMeta->getColumnLabel(nCol+1);
            aColTypes[nCol]  = xMeta->getColumnType(nCol+1);
            AddLabel(aColTitle);
        }

        // Now get the data rows.
        Reference<sdbc::XRow> xRow(xRowSet, UNO_QUERY_THROW);

        std::vector<Bucket> aBuckets;
        ScDPItemData aData;
        for (sal_Int32 nCol = 0; nCol < mnColumnCount; ++nCol)
        {
            xRowSet->first();
            aBuckets.clear();
            Field& rField = maFields[nCol];
            SCROW nRow = 0;
            do
            {
                short nFormatType = NUMBERFORMAT_UNDEFINED;
                aData.SetEmpty();
                getItemValue(*this, aData, xRow, aColTypes[nCol], nCol+1, rNullDate, nFormatType);
                aBuckets.push_back(Bucket(aData, 0, nRow));
                if (!aData.IsEmpty())
                {
                    maEmptyRows.insert_back(nRow, nRow+1, false);
                    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
                    rField.mnNumFormat = pFormatter ? pFormatter->GetStandardFormat(nFormatType) : 0;
                }

                ++nRow;
            }
            while (xRowSet->next());

            processBuckets(aBuckets, rField);
        }

        xRowSet->beforeFirst();

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
            String  aCellStr = pCellData->GetString();

            bool bRealRegExp = (rParam.bRegExp && ((rEntry.eOp == SC_EQUAL)
                                                   || (rEntry.eOp == SC_NOT_EQUAL)));
            bool bTestRegExp = false;
            if (bRealRegExp || bTestRegExp)
            {
                xub_StrLen nStart = 0;
                xub_StrLen nEnd   = aCellStr.Len();
                bool bMatch = (bool) rEntry.GetSearchTextPtr( rParam.bCaseSens )
                              ->SearchFrwrd( aCellStr, &nStart, &nEnd );
                // from 614 on, nEnd is behind the found text
                if (bMatch && bMatchWholeCell
                    && (nStart != 0 || nEnd != aCellStr.Len()))
                    bMatch = false;    // RegExp must match entire cell string
                if (bRealRegExp)
                    bOk = ((rEntry.eOp == SC_NOT_EQUAL) ? !bMatch : bMatch);
            }
            if (!bRealRegExp)
            {
                if (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL)
                {
                    if (bMatchWholeCell)
                    {
                        String aStr = rEntry.GetQueryItem().maString;
                        bOk = pTransliteration->isEqual(aCellStr, aStr);
                        bool bHasStar = false;
                        xub_StrLen nIndex;
                        if (( nIndex = aStr.Search('*') ) != STRING_NOTFOUND)
                            bHasStar = sal_True;
                        if (bHasStar && (nIndex>0))
                        {
                            for (i=0;(i<nIndex) && (i< aCellStr.Len()) ; i++)
                            {
                                if (aCellStr.GetChar( (sal_uInt16)i ) == aStr.GetChar((sal_uInt16) i ))
                                {
                                    bOk=1;
                                }
                                else
                                {
                                    bOk=0;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        const rtl::OUString& rQueryStr = rEntry.GetQueryItem().maString;
                        ::com::sun::star::uno::Sequence< sal_Int32 > xOff;
                        String aCell = pTransliteration->transliterate(
                            aCellStr, ScGlobal::eLnge, 0, aCellStr.Len(), &xOff);
                        String aQuer = pTransliteration->transliterate(
                            rQueryStr, ScGlobal::eLnge, 0, rQueryStr.getLength(), &xOff);
                        bOk = (aCell.Search( aQuer ) != STRING_NOTFOUND);
                    }
                    if (rEntry.eOp == SC_NOT_EQUAL)
                        bOk = !bOk;
                }
                else
                {   // use collator here because data was probably sorted
                    sal_Int32 nCompare = pCollator->compareString(
                        aCellStr, rEntry.GetQueryItem().maString);
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

rtl::OUString ScDPCache::GetDimensionName(LabelsType::size_type nDim) const
{
    OSL_ENSURE(nDim < maLabelNames.size()-1 , "ScDPTableDataCache::GetDimensionName");
    OSL_ENSURE(maLabelNames.size() == static_cast <sal_uInt16> (mnColumnCount+1), "ScDPTableDataCache::GetDimensionName");

    if ( nDim+1 < maLabelNames.size() )
    {
        return maLabelNames[nDim+1];
    }
    else
        return rtl::OUString();
}

namespace {

typedef boost::unordered_set<rtl::OUString, rtl::OUStringHash> LabelSet;

class InsertLabel : public std::unary_function<rtl::OUString, void>
{
    LabelSet& mrNames;
public:
    InsertLabel(LabelSet& rNames) : mrNames(rNames) {}
    void operator() (const rtl::OUString& r)
    {
        mrNames.insert(r);
    }
};

}

void ScDPCache::PostInit()
{
    maEmptyRows.build_tree();
}

void ScDPCache::Clear()
{
    maFields.clear();
    maLabelNames.clear();
    maGroupFields.clear();
    maEmptyRows.clear();
    maStringPool.clear();
}

void ScDPCache::AddLabel(const rtl::OUString& rLabel)
{

    if ( maLabelNames.empty() )
        maLabelNames.push_back(ScGlobal::GetRscString(STR_PIVOT_DATA));

    //reset name if needed
    LabelSet aExistingNames;
    std::for_each(maLabelNames.begin(), maLabelNames.end(), InsertLabel(aExistingNames));
    sal_Int32 nSuffix = 1;
    rtl::OUString aNewName = rLabel;
    while (true)
    {
        if (!aExistingNames.count(aNewName))
        {
            // unique name found!
            maLabelNames.push_back(aNewName);
            return;
        }

        // Name already exists.
        rtl::OUStringBuffer aBuf(rLabel);
        aBuf.append(++nSuffix);
        aNewName = aBuf.makeStringAndClear();
    }
}

SCROW ScDPCache::GetItemDataId(sal_uInt16 nDim, SCROW nRow, bool bRepeatIfEmpty) const
{
    OSL_ENSURE(nDim < mnColumnCount, "ScDPTableDataCache::GetItemDataId ");

    const Field& rField = maFields[nDim];
    if (bRepeatIfEmpty)
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
        // source field.
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

    // Try group fields.
    nDimPos -= nSourceCount;
    if (nDimPos >= maGroupFields.size())
        return NULL;

    const ItemsType& rGI = maGroupFields[nDimPos].maItems;
    if (nItemId >= rGI.size())
        return NULL;

    return &rGI[nItemId];
}

SCROW ScDPCache::GetRowCount() const
{
    if (maFields.empty() || maFields[0].maData.empty())
        return 0;

    return maFields[0].maData.size();
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

    // TODO: Find a way to determine the dominant number format in presence of
    // multiple number formats in the same field.
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

SCCOL ScDPCache::GetDimensionIndex(const rtl::OUString& sName) const
{
    for (size_t i = 1; i < maLabelNames.size(); ++i)
    {
        if (maLabelNames[i].equals(sName))
            return static_cast<SCCOL>(i-1);
    }
    return -1;
}

const rtl::OUString* ScDPCache::InternString(const rtl::OUString& rStr) const
{
    StringSetType::iterator it = maStringPool.find(rStr);
    if (it != maStringPool.end())
        // In the pool.
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
        // Object being deleted.
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
        // source field.
        const ItemsType& rItems = maFields[nDim].maItems;
        for (size_t i = 0, n = rItems.size(); i < n; ++i)
        {
            if (rItems[i] == rItem)
                return i;
        }

        if (!maFields[nDim].mpGroup)
            return -1;

        // grouped source field.
        const ItemsType& rGI = maFields[nDim].mpGroup->maItems;
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
        const ItemsType& rGI = maGroupFields[nDim].maItems;
        for (size_t i = 0, n = rGI.size(); i < n; ++i)
        {
            if (rGI[i] == rItem)
                return i;
        }
    }

    return -1;
}

rtl::OUString ScDPCache::GetFormattedString(long nDim, const ScDPItemData& rItem) const
{
    if (nDim < 0)
        return rItem.GetString();

    ScDPItemData::Type eType = rItem.GetType();
    if (eType == ScDPItemData::Value)
    {
        // Format value using the stored number format.
        sal_uLong nNumFormat = GetNumberFormat(nDim);
        SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
        if (pFormatter)
        {
            Color* pColor = NULL;
            String aStr;
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

#include <iostream>
using std::cout;
using std::endl;

namespace {

std::ostream& operator<< (::std::ostream& os, const rtl::OUString& str)
{
    return os << ::rtl::OUStringToOString(str, RTL_TEXTENCODING_UTF8).getStr();
}

void dumpItems(const ScDPCache& rCache, long nDim, const ScDPCache::ItemsType& rItems, size_t nOffset)
{
    for (size_t i = 0; i < rItems.size(); ++i)
        cout << "      " << (i+nOffset) << ": " << rCache.GetFormattedString(nDim, rItems[i]) << endl;
}

}

void ScDPCache::Dump() const
{
    cout << "--- pivot cache dump" << endl;
    {
        FieldsType::const_iterator it = maFields.begin(), itEnd = maFields.end();
        for (size_t i = 0; it != itEnd; ++it, ++i)
        {
            const Field& fld = *it;
            cout << "* source dimension: " << GetDimensionName(i) << " (ID = " << i << ")" << endl;
            cout << "    item count: " << fld.maItems.size() << endl;
            dumpItems(*this, i, fld.maItems, 0);
            if (fld.mpGroup)
            {
                cout << "    group item count: " << fld.mpGroup->maItems.size() << endl;
                dumpItems(*this, i, fld.mpGroup->maItems, fld.maItems.size());
            }
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

    {
        GroupFieldsType::const_iterator it = maGroupFields.begin(), itEnd = maGroupFields.end();
        for (size_t i = maFields.size(); it != itEnd; ++it, ++i)
        {
            const GroupItems& gi = *it;
            cout << "* group dimension: (unnamed) (ID = " << i << ")" << endl;
            cout << "    item count: " << gi.maItems.size() << endl;
            dumpItems(*this, i, gi.maItems, 0);
        }
    }

    cout << "---" << endl;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
