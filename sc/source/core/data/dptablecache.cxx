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

#include <rtl/math.hxx>
#include <unotools/textsearch.hxx>

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

#include <stdio.h>
#include <string>
#include <sys/time.h>

namespace {

class stack_printer
{
public:
    explicit stack_printer(const char* msg) :
        msMsg(msg)
    {
        fprintf(stdout, "%s: --begin\n", msMsg.c_str());
        mfStartTime = getTime();
    }

    ~stack_printer()
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", msMsg.c_str(), (fEndTime-mfStartTime));
    }

    void printTime(int line) const
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", msMsg.c_str(), line, (fEndTime-mfStartTime));
    }

private:
    double getTime() const
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    ::std::string msMsg;
    double mfStartTime;
};

}

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

// -----------------------------------------------------------------------
//class ScDPTableDataCache
//To cache the pivot table data source

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

    maFields.clear();
    maFields.reserve(mnColumnCount);
    for (size_t i = 0; i < static_cast<size_t>(mnColumnCount); ++i)
        maFields.push_back(new Field);

    maLabelNames.clear();
    maLabelNames.reserve(mnColumnCount);

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
        maLabelNames.reserve(mnColumnCount);

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
    return mbEmptyRow[ nRow ];
}

bool ScDPCache::AddData(long nDim, ScDPItemData* pData, sal_uLong nNumFormat)
{
    OSL_ENSURE( IsValid(), "  IsValid() == false " );
    OSL_ENSURE( nDim < mnColumnCount && nDim >=0 , "dimension out of bound" );

    // Wrap this instance with scoped pointer to ensure proper deletion.
    auto_ptr<ScDPItemData> p(pData);

    SCROW nIndex = 0;
    Field& rField = maFields[nDim];
    if (!hasItemInDimension(rField.maItems, rField.maGlobalOrder, *pData, nIndex))
    {
        // This item doesn't exist in the dimension array yet.
        rField.maItems.push_back(p);
        rField.maGlobalOrder.insert(
            rField.maGlobalOrder.begin()+nIndex, rField.maItems.size()-1);
        OSL_ENSURE(rField.maGlobalOrder[nIndex] == rField.maItems.size()-1, "ScDPTableDataCache::AddData ");
        rField.maData.push_back(rField.maItems.size()-1);
        rField.maNumFormats.push_back(nNumFormat);
    }
    else
        rField.maData.push_back(rField.maGlobalOrder[nIndex]);

//init empty row tag
    size_t nCurRow = maFields[nDim].maData.size() - 1;

    while ( mbEmptyRow.size() <= nCurRow )
        mbEmptyRow.push_back( true );

    if (!pData->IsEmpty())
        mbEmptyRow[ nCurRow ] = false;

    return true;
}


rtl::OUString ScDPCache::GetDimensionName( sal_uInt16 nColumn ) const
{
    OSL_ENSURE(nColumn < maLabelNames.size()-1 , "ScDPTableDataCache::GetDimensionName");
    OSL_ENSURE(maLabelNames.size() == static_cast <sal_uInt16> (mnColumnCount+1), "ScDPTableDataCache::GetDimensionName");

    if ( static_cast<size_t>(nColumn+1) < maLabelNames.size() )
    {
        return maLabelNames[nColumn+1];
    }
    else
        return rtl::OUString();
}

namespace {

typedef boost::unordered_set<rtl::OUString, rtl::OUStringHash> LabelSet;

class InsertLabel : public std::unary_function<ScDPItemData, void>
{
    LabelSet& mrNames;
public:
    InsertLabel(LabelSet& rNames) : mrNames(rNames) {}
    void operator() (const ScDPItemData& r)
    {
        mrNames.insert(r.GetString());
    }
};

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
//  stack_printer __stack_printer__("ScDPCache::GetItemDataById");
//  fprintf(stdout, "ScDPCache::GetItemDataById:   dim = %d  id = %d\n", nDim, nId);
    if (nDim < 0 || nId < 0)
    {
        fprintf(stdout, "ScDPCache::GetItemDataById:   negative ID\n");
        return NULL;
    }

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
    {
        // source field.
        const Field& rField = maFields[nDim];
        if (nId < rField.maItems.size())
        {
//          fprintf(stdout, "ScDPCache::GetItemDataById:   s = '%s' (source)\n",
//                  rtl::OUStringToOString(rField.maItems[nId].GetString(), RTL_TEXTENCODING_UTF8).getStr());
            return &rField.maItems[nId];
        }

        if (!rField.mpGroup)
        {
            fprintf(stdout, "ScDPCache::GetItemDataById:   fail (%d)\n", __LINE__);
            return NULL;
        }

        nId -= rField.maItems.size();
        const DataListType& rGI = rField.mpGroup->maItems;
        if (nId >= rGI.size())
        {
            fprintf(stdout, "ScDPCache::GetItemDataById:   fail (%d)\n", __LINE__);
            return NULL;
        }
//      fprintf(stdout, "ScDPCache::GetItemDataById:   s = '%s' (grouped source field)\n",
//              rtl::OUStringToOString(rGI[nId].GetString(), RTL_TEXTENCODING_UTF8).getStr());
        return &rGI[nId];
    }

    // Try group fields.
    nDim -= nSourceCount;
    if (nDim >= maGroupFields.size())
    {
        fprintf(stdout, "ScDPCache::GetItemDataById:   fail (%d)\n", __LINE__);
        return NULL;
    }

    const DataListType& rGI = maGroupFields[nDim].maItems;
    if (nId >= rGI.size())
    {
        fprintf(stdout, "ScDPCache::GetItemDataById:   fail (%d)\n", __LINE__);
        return NULL;
    }
//  fprintf(stdout, "ScDPCache::GetItemDataById:   s = '%s' (group field)\n",
//          rtl::OUStringToOString(rGI[nId].GetString(), RTL_TEXTENCODING_UTF8).getStr());
    return &rGI[nId];
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

sal_uLong ScDPCache::GetNumType(sal_uLong nFormat) const
{
    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
    sal_uLong nType = NUMBERFORMAT_NUMBER;
    if ( pFormatter )
        nType = pFormatter->GetType( nFormat );
    return nType;
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
            return (SCCOL)(i-1);
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

SCROW ScDPCache::GetIdByItemData(long nDim, const rtl::OUString& sItemData) const
{
//  stack_printer __stack_printer__("ScDPCache::GetIdByItemData");
//  fprintf(stdout, "ScDPCache::GetIdByItemData:   dim = %ld  s = '%s'\n",
//          nDim, rtl::OUStringToOString(sItemData, RTL_TEXTENCODING_UTF8).getStr());
    if (nDim < 0)
        return -1;

    if (nDim < mnColumnCount)
    {
        // source field.
        const DataListType& rItems = maFields[nDim].maItems;
        for (size_t i = 0, n = rItems.size(); i < n; ++i)
        {
//          fprintf(stdout, "ScDPCache::GetIdByItemData:   source item = '%s'\n", rtl::OUStringToOString(rItems[i].GetString(), RTL_TEXTENCODING_UTF8).getStr());
            if (rItems[i].GetString() == sItemData)
                return i;
        }

        if (!maFields[nDim].mpGroup)
            return -1;

        // grouped source field.
        const DataListType& rGI = maFields[nDim].mpGroup->maItems;
        for (size_t i = 0, n = rGI.size(); i < n; ++i)
        {
//          fprintf(stdout, "ScDPCache::GetIdByItemData:   grouped source item = '%s'\n", rtl::OUStringToOString(rGI[i].GetString(), RTL_TEXTENCODING_UTF8).getStr());
            if (rGI[i].GetString() == sItemData)
                return rItems.size() + i;
        }
        return -1;
    }

    // group field.
    nDim -= mnColumnCount;
    if (nDim < maGroupFields.size())
    {
        const DataListType& rGI = maGroupFields[nDim].maItems;
        for (size_t i = 0, n = rGI.size(); i < n; ++i)
        {
//          fprintf(stdout, "ScDPCache::GetIdByItemData:   grouped item = '%s'\n", rtl::OUStringToOString(rGI[i].GetString(), RTL_TEXTENCODING_UTF8).getStr());
            if (rGI[i].GetString() == sItemData)
                return i;
        }
    }

    return -1;
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
    if (nDim < maGroupFields.size())
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
    if (rItem.GetType() == ScDPItemData::GroupValue)
    {
        ScDPItemData::GroupValueAttr aAttr = rItem.GetGroupValue();
        return ScDPUtil::getDateGroupName(
            aAttr.mnGroupType, aAttr.mnValue, mpDoc->GetFormatTable());
    }

    if (!rItem.IsValue())
        return rItem.GetString();

    sal_uLong nNumFormat = GetNumberFormat(nDim);
    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
    if (pFormatter)
    {
        Color* pColor = NULL;
        String aStr;
        pFormatter->GetOutputString(rItem.GetValue(), nNumFormat, aStr, &pColor);
        return aStr;
    }
    return rtl::OUString::createFromAscii("fail again");
}

void ScDPCache::AppendGroupField()
{
    maGroupFields.push_back(new GroupField);
}

void ScDPCache::ResetGroupItems(long nDim)
{
    if (nDim < 0)
        return;

    long nSourceCount = static_cast<long>(maFields.size());
    if (nDim < nSourceCount)
    {
        maFields.at(nDim).mpGroup.reset(new GroupItems);
        return;
    }

    nDim -= nSourceCount;
    if (nDim < static_cast<long>(maGroupFields.size()))
        maGroupFields[nDim].maItems.clear();
}

SCROW ScDPCache::SetGroupItem(long nDim, const ScDPItemData& rData)
{
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

SCROW ScDPCache::GetAdditionalItemID(const ScDPItemData&) const
{
    fprintf(stdout, "ScDPCache::GetAdditionalItemID:   FIXME\n");
    return -1;
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

    OSL_ENSURE(nIndex >= 0 && nIndex < rField.maIndexOrder.size() , "ScDPTableDataCache::GetOrder");
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
