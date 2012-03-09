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

#include "dptablecache.hxx"

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

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>

#include <memory>

const double D_TIMEFACTOR = 86400.0;

using namespace ::com::sun::star;

using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::std::vector;
using ::std::auto_ptr;

namespace {

/**
 * Search for an item in the data array.  If it's in the array, return its
 * index to the caller.
 *
 * @param rArray dimension array
 * @param rOrder global order (what's this?)
 * @param item item to search for
 * @param rIndex the index of the found item in the global order.
 *
 * @return true if the item is found, or false otherwise.
 */
bool hasItemInDimension(const ScDPCache::DataListType& rArray, const ScDPCache::IndexArrayType& rOrder, const ScDPItemData& item, SCROW& rIndex)
{
    rIndex = rArray.size();
    bool bFound = false;
    SCROW nLo = 0;
    SCROW nHi = rArray.size() - 1;
    long nCompare;
    while (nLo <= nHi)
    {
        SCROW nIndex = (nLo + nHi) / 2;
        nCompare = ScDPItemData::Compare( rArray[rOrder[nIndex]], item );
        if (nCompare < 0)
            nLo = nIndex + 1;
        else
        {
            nHi = nIndex - 1;
            if (nCompare == 0)
            {
                bFound = true;
                nLo = nIndex;
            }
        }
    }
    rIndex = nLo;
    return bFound;
}

ScDPItemData* lcl_GetItemValue(
    const Reference<sdbc::XRow>& xRow, sal_Int32 nType, long nCol, const Date& rNullDate, short& rNumType)
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
                return new ScDPItemData(fValue);
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
                return new ScDPItemData(fValue);
            }
            case sdbc::DataType::DATE:
            {
                rNumType = NUMBERFORMAT_DATE;

                util::Date aDate = xRow->getDate(nCol);
                fValue = Date(aDate.Day, aDate.Month, aDate.Year) - rNullDate;
                return new ScDPItemData(fValue);
            }
            case sdbc::DataType::TIME:
            {
                rNumType = NUMBERFORMAT_TIME;

                util::Time aTime = xRow->getTime(nCol);
                fValue = ( aTime.Hours * 3600 + aTime.Minutes * 60 +
                           aTime.Seconds + aTime.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
                return new ScDPItemData(fValue);
            }
            case sdbc::DataType::TIMESTAMP:
            {
                rNumType = NUMBERFORMAT_DATETIME;

                util::DateTime aStamp = xRow->getTimestamp(nCol);
                fValue = ( Date( aStamp.Day, aStamp.Month, aStamp.Year ) - rNullDate ) +
                         ( aStamp.Hours * 3600 + aStamp.Minutes * 60 +
                           aStamp.Seconds + aStamp.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
                return new ScDPItemData(fValue);
            }
            case sdbc::DataType::CHAR:
            case sdbc::DataType::VARCHAR:
            case sdbc::DataType::LONGVARCHAR:
            case sdbc::DataType::SQLNULL:
            case sdbc::DataType::BINARY:
            case sdbc::DataType::VARBINARY:
            case sdbc::DataType::LONGVARBINARY:
            default:
                return new ScDPItemData(xRow->getString(nCol));
        }
    }
    catch (uno::Exception&)
    {
    }

    return NULL;
}

}

ScDPCache::GroupItems::GroupItems() {}

ScDPCache::GroupItems::GroupItems(const ScDPNumGroupInfo& rInfo) :
    maInfo(rInfo) {}

bool ScDPCache::operator== ( const ScDPCache& r ) const
{
    if ( GetColumnCount() == r.GetColumnCount() )
    {
        for ( SCCOL i = 0 ; i < GetColumnCount(); i++ )
        {   //check dim names
            if ( GetDimensionName( i ) != r.GetDimensionName( i ) )
                return false;
            //check rows count
            if ( GetRowCount() != r.GetRowCount() )
                return false;
            //check dim member values
            size_t nMembersCount = GetDimMemberValues( i ).size();
            if ( GetDimMemberValues( i ).size() == r. GetDimMemberValues( i ).size() )
            {
                for ( size_t j = 0; j < nMembersCount; j++ )
                {
                    if ( GetDimMemberValues(i)[j] == r.GetDimMemberValues(i)[j] )
                        continue;
                    else
                        return false;
                }
            }
            else
                return false;
            //check source table index
            for ( SCROW k=0 ; k < GetRowCount(); k ++ )
            {
                if ( GetItemDataId( i, k, false ) == r.GetItemDataId( i,k,false) )
                    continue;
                else
                    return false;
            }
        }
    }
    return true;
}

ScDPCache::Field::Field() {}

ScDPCache::ScDPCache(ScDocument* pDoc) :
    mpDoc( pDoc ),
    mnColumnCount ( 0 ),
    mbDisposing(false)
{
}

namespace {

struct ClearObjectSource : std::unary_function<ScDPObject*, void>
{
    void operator() (ScDPObject* p) const
    {
        p->ClearSource();
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

bool ScDPCache::IsValid() const
{
    return !maFields.empty() && mnColumnCount > 0;
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

void initFromCell(ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab, ScDPItemData& rData, sal_uLong& rNumFormat)
{
    rtl::OUString aDocStr = pDoc->GetString(nCol, nRow, nTab);
    rNumFormat = 0;

    ScAddress aPos(nCol, nRow, nTab);

    if (pDoc->GetErrCode(aPos))
    {
        rData.SetErrorString(aDocStr);
    }
    else if (pDoc->HasValueData(nCol, nRow, nTab))
    {
        double fVal = pDoc->GetValue(aPos);
        rNumFormat = pDoc->GetNumberFormat(aPos);
        rData.SetValue(fVal);
    }
    else if (pDoc->HasData(nCol, nRow, nTab))
    {
        rData.SetString(aDocStr);
    }
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

    for (sal_uInt16 nCol = nStartCol; nCol <= nEndCol; ++nCol)
    {
        AddLabel(createLabelString(pDoc, nCol, nStartRow, nDocTab));
        for (SCROW nRow = nStartRow + 1; nRow <= nEndRow; ++nRow)
        {
            std::auto_ptr<ScDPItemData> pData(new ScDPItemData);
            sal_uLong nNumFormat = 0;
            initFromCell(pDoc, nCol, nRow, nDocTab, *pData, nNumFormat);
            AddData(nCol - nStartCol, pData.release(), nNumFormat);
        }
    }
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
        xRowSet->first();
        do
        {
            for (sal_Int32 nCol = 0; nCol < mnColumnCount; ++nCol)
            {
                short nFormatType = NUMBERFORMAT_UNDEFINED;
                ScDPItemData* pNew = lcl_GetItemValue(
                    xRow, aColTypes[nCol], nCol+1, rNullDate, nFormatType);
                if (pNew)
                {
                    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
                    sal_uLong nNumFormat = pFormatter ? pFormatter->GetStandardFormat(nFormatType) : 0;
                    AddData(nCol, pNew, nNumFormat);
                }
            }
        }
        while (xRowSet->next());

        xRowSet->beforeFirst();

        return true;
    }
    catch (const Exception&)
    {
        return false;
    }
}

bool ScDPCache::ValidQuery( SCROW nRow, const ScQueryParam &rParam) const
{
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

bool ScDPCache::IsRowEmpty( SCROW nRow ) const
{
    return maEmptyRows[nRow];
}

bool ScDPCache::AddData(long nDim, ScDPItemData* pData, sal_uLong nNumFormat)
{
    OSL_ENSURE( IsValid(), "  IsValid() == false " );
    OSL_ENSURE( nDim < mnColumnCount && nDim >=0 , "dimension out of bound" );

    // Wrap this instance with scoped pointer to ensure proper deletion.
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    auto_ptr<ScDPItemData> p(pData);
    SAL_WNODEPRECATED_DECLARATIONS_POP

    SCROW nIndex = 0;
    Field& rField = maFields[nDim];
    if (!hasItemInDimension(rField.maItems, rField.maGlobalOrder, *pData, nIndex))
    {
        // This item doesn't exist in the dimension array yet.
        rField.maItems.push_back(p);
        rField.maGlobalOrder.insert(
            rField.maGlobalOrder.begin()+nIndex, rField.maItems.size()-1);
        OSL_ENSURE(rField.maGlobalOrder[nIndex] == sal::static_int_cast<SCROW>(rField.maItems.size())-1, "ScDPTableDataCache::AddData ");
        rField.maData.push_back(rField.maItems.size()-1);
        rField.maNumFormats.push_back(nNumFormat);
    }
    else
        rField.maData.push_back(rField.maGlobalOrder[nIndex]);

//init empty row tag
    size_t nCurRow = maFields[nDim].maData.size() - 1;

    while (maEmptyRows.size() <= nCurRow)
        maEmptyRows.push_back(true);

    if (!pData->IsEmpty())
        maEmptyRows[nCurRow] = false;

    return true;
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

void ScDPCache::Clear()
{
    maFields.clear();
    maLabelNames.clear();
    maGroupFields.clear();
    maEmptyRows.clear();
}

void ScDPCache::AddLabel(const rtl::OUString& rLabel)
{
    OSL_ENSURE( IsValid(), "  IsValid() == false " );

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
    OSL_ENSURE( IsValid(), "  IsValid() == false " );
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
        const DataListType& rGI = rField.mpGroup->maItems;
        if (nItemId >= rGI.size())
            return NULL;

        return &rGI[nItemId];
    }

    // Try group fields.
    nDimPos -= nSourceCount;
    if (nDimPos >= maGroupFields.size())
        return NULL;

    const DataListType& rGI = maGroupFields[nDimPos].maItems;
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

const ScDPCache::DataListType& ScDPCache::GetDimMemberValues(SCCOL nDim) const
{
    OSL_ENSURE( nDim>=0 && nDim < mnColumnCount ," nDim < mnColumnCount ");
    return maFields[nDim].maItems;
}

sal_uLong ScDPCache::GetNumberFormat( long nDim ) const
{
    if ( nDim >= mnColumnCount )
        return 0;

    const std::vector<sal_uLong>& rNumFormats = maFields[nDim].maNumFormats;
    if (rNumFormats.empty())
        return 0;

    // TODO: Find a way to determine the dominant number format in presence of
    // multiple number formats in the same field.
    return *rNumFormats.begin();
}

bool ScDPCache::IsDateDimension( long nDim ) const
{
    if (nDim >= mnColumnCount)
        return false;

    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
    if (!pFormatter)
        return false;

    const std::vector<sal_uLong>& rNumFormats = maFields[nDim].maNumFormats;
    if (rNumFormats.empty())
        return false;

    short eType = pFormatter->GetType(rNumFormats[0]);
    return (eType == NUMBERFORMAT_DATE) || (eType == NUMBERFORMAT_DATETIME);
}

SCROW ScDPCache::GetDimMemberCount( SCCOL nDim ) const
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
        const DataListType& rItems = maFields[nDim].maItems;
        for (size_t i = 0, n = rItems.size(); i < n; ++i)
        {
            if (rItems[i] == rItem)
                return i;
        }

        if (!maFields[nDim].mpGroup)
            return -1;

        // grouped source field.
        const DataListType& rGI = maFields[nDim].mpGroup->maItems;
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
        const DataListType& rGI = maGroupFields[nDim].maItems;
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

        sal_Unicode cDecSep = ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0);
        return ScDPUtil::getNumGroupName(fVal, p->maInfo, cDecSep, mpDoc->GetFormatTable());
    }

    return rItem.GetString();
}

long ScDPCache::AppendGroupField()
{
    maGroupFields.push_back(new GroupItems);
    return static_cast<long>(maFields.size() + maGroupFields.size() - 1);
}

void ScDPCache::ResetGroupItems(long nDim, const ScDPNumGroupInfo& rNumInfo)
{
    if (nDim < 0)
        return;

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
    {
        maFields.at(nDim).mpGroup.reset(new GroupItems(rNumInfo));
        return;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
    {
        GroupItems& rGI = maGroupFields[nDim];
        rGI.maItems.clear();
        rGI.maInfo = rNumInfo;
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
        rGI.maItems.push_back(new ScDPItemData(rData));
        SCROW nId = maFields[nDim].maItems.size() + rGI.maItems.size() - 1;
        return nId;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
    {
        DataListType& rItems = maGroupFields.at(nDim).maItems;
        rItems.push_back(new ScDPItemData(rData));
        return rItems.size()-1;
    }

    return -1;
}

const ScDPCache::DataListType* ScDPCache::GetGroupDimMemberValues(long nDim) const
{
    if (nDim < 0)
        return NULL;

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
    {
        if (!maFields.at(nDim).mpGroup)
            return NULL;

        return &maFields[nDim].mpGroup->maItems;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
        return &maGroupFields.at(nDim).maItems;

    return NULL;
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
        const DataListType& rGI = maFields[nDim].mpGroup->maItems;
        for (size_t i = 0, n = rGI.size(); i < n; ++i)
            rIds.push_back(static_cast<SCROW>(i + nOffset));

        return;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
    {
        const DataListType& rGI = maGroupFields.at(nDim).maItems;
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

SCROW ScDPCache::GetOrder(long nDim, SCROW nIndex) const
{
    OSL_ENSURE( IsValid(), "  IsValid() == false " );
    OSL_ENSURE( nDim >=0 && nDim < mnColumnCount, "ScDPTableDataCache::GetOrder : out of bound" );

    const Field& rField = maFields[nDim];
    if (rField.maIndexOrder.size() !=  rField.maGlobalOrder.size())
    { //not inited
        SCROW nRow  = 0;
        rField.maIndexOrder.resize(rField.maGlobalOrder.size(), 0);
        for (size_t i = 0, n = rField.maGlobalOrder.size(); i < n; ++i)
        {
            nRow = rField.maGlobalOrder[i];
            rField.maIndexOrder[nRow] = i;
        }
    }

    OSL_ENSURE(nIndex >= 0 && sal::static_int_cast<sal_uInt32>(nIndex) < rField.maIndexOrder.size() , "ScDPTableDataCache::GetOrder");
    return rField.maIndexOrder[nIndex];
}

ScDocument* ScDPCache::GetDoc() const
{
    return mpDoc;
};

long ScDPCache::GetColumnCount() const
{
    return mnColumnCount;
}

long ScDPCache::GetGroupFieldCount() const
{
    return maGroupFields.size();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
