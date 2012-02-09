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
bool hasItemInDimension(const ScDPCache::DataListType& rArray, const ::std::vector<SCROW>& rOrder, const ScDPItemData& item, SCROW& rIndex)
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
    const Reference<sdbc::XRow>& xRow, sal_Int32 nType, long nCol, const Date& rNullDate)
{
    short nNumType = NUMBERFORMAT_NUMBER;
    try
    {
        String rStr = xRow->getString(nCol);
        double fValue = 0.0;
        switch (nType)
        {
            case sdbc::DataType::BIT:
            case sdbc::DataType::BOOLEAN:
            {
                nNumType = NUMBERFORMAT_LOGICAL;
                fValue  = xRow->getBoolean(nCol) ? 1 : 0;
                return new ScDPItemData( rStr, fValue,true,nNumType);
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
                return new ScDPItemData( rStr, fValue,true);
            }
            case sdbc::DataType::DATE:
            {
                nNumType = NUMBERFORMAT_DATE;

                util::Date aDate = xRow->getDate(nCol);
                fValue = Date(aDate.Day, aDate.Month, aDate.Year) - rNullDate;
                return new ScDPItemData( rStr, fValue, true, nNumType );
            }
            case sdbc::DataType::TIME:
            {
                nNumType = NUMBERFORMAT_TIME;

                util::Time aTime = xRow->getTime(nCol);
                fValue = ( aTime.Hours * 3600 + aTime.Minutes * 60 +
                           aTime.Seconds + aTime.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
                return new ScDPItemData( rStr,fValue, true, nNumType );
            }
            case sdbc::DataType::TIMESTAMP:
            {
                nNumType = NUMBERFORMAT_DATETIME;

                util::DateTime aStamp = xRow->getTimestamp(nCol);
                fValue = ( Date( aStamp.Day, aStamp.Month, aStamp.Year ) - rNullDate ) +
                         ( aStamp.Hours * 3600 + aStamp.Minutes * 60 +
                           aStamp.Seconds + aStamp.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
                return new ScDPItemData( rStr,fValue, true, nNumType );
            }
            case sdbc::DataType::CHAR:
            case sdbc::DataType::VARCHAR:
            case sdbc::DataType::LONGVARCHAR:
            case sdbc::DataType::SQLNULL:
            case sdbc::DataType::BINARY:
            case sdbc::DataType::VARBINARY:
            case sdbc::DataType::LONGVARBINARY:
            default:
                return new ScDPItemData ( rStr );
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
    return !maTableDataValues.empty() && !maSourceData.empty() && mnColumnCount > 0;
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
    if ( IsValid() )
    {
        maTableDataValues.clear();
        maSourceData.clear();
        maGlobalOrder.clear();
        maIndexOrder.clear();
        maLabelNames.clear();
    }

    maTableDataValues.reserve(mnColumnCount);
    maSourceData.reserve(mnColumnCount);
    maGlobalOrder.reserve(mnColumnCount);
    maIndexOrder.reserve(mnColumnCount);
    for (long i = 0; i < mnColumnCount; ++i)
    {
        maTableDataValues.push_back(new DataListType);
        maSourceData.push_back(new vector<SCROW>());
        maGlobalOrder.push_back(new vector<SCROW>());
        maIndexOrder.push_back(new vector<SCROW>());
    }

    for (sal_uInt16 nCol = nStartCol; nCol <= nEndCol; ++nCol)
    {
        AddLabel(new ScDPItemData(pDoc, nCol, nStartRow, nDocTab, true));
        for (SCROW nRow = nStartRow + 1; nRow <= nEndRow; ++nRow)
            AddData(nCol - nStartCol, new ScDPItemData(pDoc, nCol, nRow, nDocTab, false));
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
        if (IsValid())
        {
            maTableDataValues.clear();
            maSourceData.clear();
            maGlobalOrder.clear();
            maIndexOrder.clear();
            maLabelNames.clear();
        }
        // Get column titles and types.
        maLabelNames.reserve(mnColumnCount);

        maTableDataValues.reserve(mnColumnCount);
        maSourceData.reserve(mnColumnCount);
        maGlobalOrder.reserve(mnColumnCount);
        maIndexOrder.reserve(mnColumnCount);
        for (long i = 0; i < mnColumnCount; ++i)
        {
            maTableDataValues.push_back(new DataListType);
            maSourceData.push_back(new vector<SCROW>());
            maGlobalOrder.push_back(new vector<SCROW>());
            maIndexOrder.push_back(new vector<SCROW>());
        }

        std::vector<sal_Int32> aColTypes(mnColumnCount);

        for (sal_Int32 nCol = 0; nCol < mnColumnCount; ++nCol)
        {
            String aColTitle = xMeta->getColumnLabel(nCol+1);
            aColTypes[nCol]  = xMeta->getColumnType(nCol+1);
            AddLabel( new ScDPItemData( aColTitle) );
        }

        // Now get the data rows.
        Reference<sdbc::XRow> xRow(xRowSet, UNO_QUERY_THROW);
        xRowSet->first();
        do
        {
            for (sal_Int32 nCol = 0; nCol < mnColumnCount; ++nCol)
            {
                ScDPItemData * pNew =  lcl_GetItemValue( xRow, aColTypes[nCol], nCol+1, rNullDate );
                if (pNew)
                    AddData(nCol , pNew);
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
                bOk = !pCellData->IsHasData();
            else
            {
                OSL_ASSERT(rEntry.IsQueryByNonEmpty());
                bOk =  pCellData->IsHasData();
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

bool ScDPCache::AddData(long nDim, ScDPItemData* pData)
{
    OSL_ENSURE( IsValid(), "  IsValid() == false " );
    OSL_ENSURE( nDim < mnColumnCount && nDim >=0 , "dimension out of bound" );

    // Wrap this instance with scoped pointer to ensure proper deletion.
    auto_ptr<ScDPItemData> p(pData);
    pData->SetDate(ScDPItemData::isDate(GetNumType(pData->nNumFormat)));

    SCROW nIndex = 0;
    if (!hasItemInDimension(maTableDataValues[nDim], maGlobalOrder[nDim], *pData, nIndex))
    {
        // This item doesn't exist in the dimension array yet.
        maTableDataValues[nDim].push_back(p);
        maGlobalOrder[nDim].insert( maGlobalOrder[nDim].begin()+nIndex, maTableDataValues[nDim].size()-1  );
        OSL_ENSURE( (size_t) maGlobalOrder[nDim][nIndex] == maTableDataValues[nDim].size()-1 ,"ScDPTableDataCache::AddData ");
        maSourceData[nDim].push_back( maTableDataValues[nDim].size()-1 );
    }
    else
        maSourceData[nDim].push_back( maGlobalOrder[nDim][nIndex] );
//init empty row tag
    size_t nCurRow = maSourceData[nDim].size() - 1;

    while ( mbEmptyRow.size() <= nCurRow )
        mbEmptyRow.push_back( true );

    if ( pData->IsHasData() )
        mbEmptyRow[ nCurRow ] = false;

    return true;
}


String ScDPCache::GetDimensionName( sal_uInt16 nColumn ) const
{
    OSL_ENSURE(nColumn < maLabelNames.size()-1 , "ScDPTableDataCache::GetDimensionName");
    OSL_ENSURE(maLabelNames.size() == static_cast <sal_uInt16> (mnColumnCount+1), "ScDPTableDataCache::GetDimensionName");

    if ( static_cast<size_t>(nColumn+1) < maLabelNames.size() )
    {
        return maLabelNames[nColumn+1].aString;
    }
    else
        return String();
}

void ScDPCache::AddLabel(ScDPItemData *pData)
{
    OSL_ENSURE( IsValid(), "  IsValid() == false " );

    if ( maLabelNames.size() == 0 )
        maLabelNames.push_back( new ScDPItemData(ScGlobal::GetRscString(STR_PIVOT_DATA)) );

    //reset name if needed
    String strNewName = pData->aString;
    bool bFound = false;
    long nIndex = 1;
    do
    {
        for ( long i= maLabelNames.size()-1; i>=0; i-- )
        {
            if( maLabelNames[i].aString == strNewName )
            {
                strNewName  =  pData->aString;
                strNewName += String::CreateFromInt32( nIndex );
                nIndex ++ ;
                bFound = true;
            }
        }
        bFound = !bFound;
    }
    while ( !bFound );

    pData->aString = strNewName;
    maLabelNames.push_back( pData );
}

SCROW ScDPCache::GetItemDataId(sal_uInt16 nDim, SCROW nRow, bool bRepeatIfEmpty) const
{
    OSL_ENSURE( IsValid(), "  IsValid() == false " );
    OSL_ENSURE( /* nDim >= 0 && */ nDim < mnColumnCount, "ScDPTableDataCache::GetItemDataId " );

    if ( bRepeatIfEmpty )
    {
        while ( nRow >0 && !maTableDataValues[nDim][ maSourceData[nDim][nRow] ].IsHasData() )
            --nRow;
    }

    return maSourceData[nDim][nRow];
}

const ScDPItemData* ScDPCache::GetItemDataById(long nDim, SCROW nId) const
{
    if ( nId >= GetRowCount()  )
        return maAdditionalData.getData( nId - GetRowCount() );

    if (  (size_t)nId >= maTableDataValues[nDim].size() || nDim >= mnColumnCount  || nId < 0  )
        return NULL;
    else
        return &maTableDataValues[nDim][nId];
}

SCROW ScDPCache::GetRowCount() const
{
    if ( IsValid() )
        return maSourceData[0].size();
    else
        return 0;
}

const ScDPCache::DataListType& ScDPCache::GetDimMemberValues(SCCOL nDim) const
{
    OSL_ENSURE( nDim>=0 && nDim < mnColumnCount ," nDim < mnColumnCount ");
    return maTableDataValues[nDim];
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

    if (maTableDataValues[nDim].empty())
        return 0;

    // TODO: This is very ugly, but the best we can do right now.  Check the
    // first 10 dimension members, and take the first non-zero number format,
    // else return the default number format (of 0).  For the long-term, we
    // need to redo this cache structure to properly indicate empty cells, and
    // skip them when trying to determine the representative number format for
    // a dimension.
    size_t nCount = maTableDataValues[nDim].size();
    if (nCount > 10)
        nCount = 10;
    for (size_t i = 0; i < nCount; ++i)
    {
        sal_uLong n = maTableDataValues[nDim][i].nNumFormat;
        if (n)
            return n;
    }
    return 0;
}

bool ScDPCache::IsDateDimension( long nDim ) const
{
    if ( nDim >= mnColumnCount )
        return false;
    else if ( maTableDataValues[nDim].size()==0 )
        return false;
    else
        return maTableDataValues[nDim][0].IsDate();

}

SCROW ScDPCache::GetDimMemberCount( SCCOL nDim ) const
{
    OSL_ENSURE( nDim>=0 && nDim < mnColumnCount ," ScDPTableDataCache::GetDimMemberCount : out of bound ");
    return maTableDataValues[nDim].size();
}

SCCOL ScDPCache::GetDimensionIndex(String sName) const
{
    for (size_t i = 1; i < maLabelNames.size(); ++i)
    {
        if ( maLabelNames[i].GetString() == sName )
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

SCROW ScDPCache::GetIdByItemData(long nDim, const String& sItemData ) const
{
    if ( nDim < mnColumnCount && nDim >=0 )
    {
        for (size_t i = 0; i < maTableDataValues[nDim].size(); ++i)
        {
            if ( maTableDataValues[nDim][i].GetString() == sItemData )
                return i;
        }
    }

    ScDPItemData rData ( sItemData );
    return  GetRowCount() +maAdditionalData.getDataId(rData);
}

SCROW ScDPCache::GetIdByItemData( long nDim, const ScDPItemData& rData  ) const
{
    if ( nDim < mnColumnCount && nDim >=0 )
    {
        for (size_t i = 0; i < maTableDataValues[nDim].size(); ++i)
        {
            if ( maTableDataValues[nDim][i] == rData )
                return i;
        }
    }
    return  GetRowCount() + maAdditionalData.getDataId(rData);
}

SCROW ScDPCache::GetAdditionalItemID( const ScDPItemData& rData ) const
{
    return GetRowCount() + maAdditionalData.insertData( rData );
}


SCROW ScDPCache::GetOrder(long nDim, SCROW nIndex) const
{
    OSL_ENSURE( IsValid(), "  IsValid() == false " );
    OSL_ENSURE( nDim >=0 && nDim < mnColumnCount, "ScDPTableDataCache::GetOrder : out of bound" );

    if ( maIndexOrder[nDim].size() !=  maGlobalOrder[nDim].size() )
    { //not inited
        SCROW nRow  = 0;
        maIndexOrder[nDim].resize(maGlobalOrder[nDim].size(), 0);
        for (size_t i = 0 ; i < maGlobalOrder[nDim].size(); ++i)
        {
            nRow = maGlobalOrder[nDim][i];
            maIndexOrder[nDim][nRow] = i;
        }
    }

    OSL_ENSURE( nIndex>=0 && (size_t)nIndex < maIndexOrder[nDim].size() , "ScDPTableDataCache::GetOrder");
    return maIndexOrder[nDim][nIndex];
}

ScDocument* ScDPCache::GetDoc() const
{
    return mpDoc;
};

long ScDPCache::GetColumnCount() const
{
    return mnColumnCount;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
