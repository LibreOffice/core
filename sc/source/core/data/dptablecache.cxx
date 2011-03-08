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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "dptablecache.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "globstr.hrc"

#include <rtl/math.hxx>
#include "queryparam.hxx"
#include "dpglobal.hxx"
#include "dptabdat.hxx"

#include "docoptio.hxx"
#include <unotools/textsearch.hxx>

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>

const double D_TIMEFACTOR = 86400.0;

using namespace ::com::sun::star;

using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::std::vector;

namespace {

bool lcl_isDate( ULONG nNumType )
{
    return ((nNumType & NUMBERFORMAT_DATE) != 0) ? 1 : 0;
}

bool lcl_Search( const ScDPCache::DataListType& list, const ::std::vector<SCROW>& rOrder, const ScDPItemData& item, SCROW& rIndex)
{
    rIndex = list.size();
    bool bFound = false;
    SCROW nLo = 0;
    SCROW nHi = list.size() - 1;
    SCROW nIndex;
    long nCompare;
    while (nLo <= nHi)
    {
        nIndex = (nLo + nHi) / 2;
        nCompare = ScDPItemData::Compare( list[rOrder[nIndex]], item );
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

ScDPItemData::ScDPItemData(const String& rS, double fV, bool bHV, const ULONG nNumFormatP, bool bData) :
    nNumFormat( nNumFormatP ), aString(rS), fValue(fV),
    mbFlag( (MK_VAL*!!bHV) | (MK_DATA*!!bData) | (MK_ERR*!!false) | (MK_DATE*!!lcl_isDate( nNumFormat ) ) )
{
}

ScDPItemData::ScDPItemData(ScDocument* pDoc, SCROW nRow, USHORT nCol, USHORT nDocTab) :
    nNumFormat( 0 ), fValue(0.0), mbFlag( 0 )
{
    String aDocStr;
    pDoc->GetString( nCol, nRow, nDocTab, aDocStr );

    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();

    ScAddress aPos( nCol, nRow, nDocTab );
    ScBaseCell* pCell = pDoc->GetCell( aPos );

    if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA && ((ScFormulaCell*)pCell)->GetErrCode() )
    {
        SetString ( aDocStr );
        mbFlag |= MK_ERR;
    }
    else if ( pDoc->HasValueData( nCol, nRow, nDocTab ) )
    {
        double fVal = pDoc->GetValue(ScAddress(nCol, nRow, nDocTab));
        ULONG nFormat = NUMBERFORMAT_NUMBER;
        if ( pFormatter )
            nFormat = pFormatter->GetType( pDoc->GetNumberFormat( ScAddress( nCol, nRow, nDocTab ) ) );
        aString = aDocStr;
        fValue = fVal;
        mbFlag |= MK_VAL|MK_DATA;
        nNumFormat = pDoc->GetNumberFormat( ScAddress( nCol, nRow, nDocTab ) );
        lcl_isDate( nFormat ) ? ( mbFlag |= MK_DATE ) : (mbFlag &= ~MK_DATE);
    }
    else if ( pDoc->HasData( nCol,nRow, nDocTab ) )
        SetString ( aDocStr );
}

bool ScDPItemData::IsCaseInsEqual( const ScDPItemData& r ) const
{
    //! pass Transliteration?
    //! inline?
    return IsValue() ? ( r.IsValue() && rtl::math::approxEqual( fValue, r.fValue ) ) :
                       ( !r.IsValue() &&
                        ScGlobal::GetpTransliteration()->isEqual( aString, r.aString ) );
}

size_t ScDPItemData::Hash() const
{
    if ( IsValue() )
        return (size_t) rtl::math::approxFloor( fValue );
    else
        // If we do unicode safe case insensitive hash we can drop
        // ScDPItemData::operator== and use ::IsCasInsEqual
        return rtl_ustr_hashCode_WithLength( aString.GetBuffer(), aString.Len() );
}

bool ScDPItemData::operator==( const ScDPItemData& r ) const
{
    if ( IsValue() )
    {
        if( (HasDatePart() != r.HasDatePart())  || (HasDatePart() && mnDatePart != r.mnDatePart) )
            return false;

        if ( IsDate() != r.IsDate() )
            return false;
        else if ( r.IsValue() )
            return rtl::math::approxEqual( fValue, r.fValue );
        else
            return false;
    }
    else if ( r.IsValue() )
        return false;
    else
        // need exact equality until we have a safe case insensitive string hash
        return aString == r.aString;
}

sal_Int32 ScDPItemData::Compare( const ScDPItemData& rA,
                                 const ScDPItemData& rB )
{
    if ( rA.IsValue() )
    {
        if ( rB.IsValue() )
        {
            if ( rtl::math::approxEqual( rA.fValue, rB.fValue ) )
            {
                if ( rA.IsDate() == rB.IsDate() )
                    return 0;
                else
                    return rA.IsDate() ? 1: -1;
            }
            else if ( rA.fValue < rB.fValue )
                return -1;
            else
                return 1;
        }
        else
            return -1;           // values first
    }
    else if ( rB.IsValue() )
        return 1;                // values first
    else
        return ScGlobal::GetCollator()->compareString( rA.aString, rB.aString );
}

#if OSL_DEBUG_LEVEL > 1
void    ScDPItemData::dump() const
{
    OSL_TRACE( "Numberformat= %o",  nNumFormat );
    OSL_TRACE( aString.GetBuffer() );
    OSL_TRACE( "fValue= %f", fValue );
    OSL_TRACE( "mbFlag= %d", mbFlag);
}
#endif

TypedStrData* ScDPItemData::CreateTypeString( )
{
    if ( IsValue() )
        return new TypedStrData( aString, fValue, SC_STRTYPE_VALUE );
    else
        return new TypedStrData( aString );
}

sal_uInt8 ScDPItemData::GetType() const
{
    if ( IsHasErr() )
        return SC_VALTYPE_ERROR;
    else if ( !IsHasData() )
        return SC_VALTYPE_EMPTY;
    else if ( IsValue())
        return SC_VALTYPE_VALUE;
    else
        return SC_VALTYPE_STRING;
}

bool ScDPItemData::IsHasData() const
{
    return !!(mbFlag&MK_DATA);
}

bool ScDPItemData::IsHasErr() const
{
    return !!(mbFlag&MK_ERR);
}

bool ScDPItemData::IsValue() const
{
    return !!(mbFlag&MK_VAL);
}

String ScDPItemData::GetString() const
{
    return aString;
}

double ScDPItemData::GetValue() const
{
    return fValue;
}
ULONG  ScDPItemData::GetNumFormat() const
{
    return nNumFormat;
}

bool ScDPItemData::HasStringData() const
{
    return IsHasData()&&!IsHasErr()&&!IsValue();
}

bool ScDPItemData::IsDate() const
{
    return !!(mbFlag&MK_DATE);
}

bool ScDPItemData::HasDatePart() const
{
    return !!(mbFlag&MK_DATEPART);
}

void ScDPItemData::SetDate( bool b )
{
    b ? ( mbFlag |= MK_DATE ) : ( mbFlag &= ~MK_DATE );
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
    mnColumnCount ( 0 )
{
}

ScDPCache::~ScDPCache()
{
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
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nDocTab = rRange.aStart.Tab();

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
    //check valid
    for ( SCROW nRow = nStartRow; nRow <= nEndRow; nRow ++ )
    {
        for ( USHORT nCol = nStartCol; nCol <= nEndCol; nCol++ )
        {
            if ( nRow == nStartRow )
                AddLabel( new ScDPItemData( pDoc, nRow, nCol, nDocTab  ) );
            else
                AddData( nCol - nStartCol, new ScDPItemData( pDoc, nRow, nCol, nDocTab  ) );
        }
    }
    return true;
}

bool ScDPCache::InitFromDataBase (const Reference<sdbc::XRowSet>& xRowSet, const Date& rNullDate)
{
    if (!xRowSet.is())
        // Dont' even waste time to go any further.
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

ULONG ScDPCache::GetDimNumType( SCCOL nDim) const
{
    DBG_ASSERT( IsValid(), "  IsValid() == false " );
    DBG_ASSERT( nDim < mnColumnCount && nDim >=0, " dimention out of bound " );
    if ( maTableDataValues[nDim].size()==0 )
        return NUMBERFORMAT_UNDEFINED;
    else
        return GetNumType(maTableDataValues[nDim][0].nNumFormat);
}

bool ScDPCache::ValidQuery( SCROW nRow, const ScQueryParam &rParam, bool *pSpecial)
{
    if (!rParam.GetEntry(0).bDoQuery)
        return true;
    bool bMatchWholeCell = mpDoc->GetDocOptions().IsMatchWholeCell();

    //---------------------------------------------------------------

    const SCSIZE nFixedBools = 32;
    bool aBool[nFixedBools];
    bool aTest[nFixedBools];
    SCSIZE nEntryCount = rParam.GetEntryCount();
    bool* pPasst = ( nEntryCount <= nFixedBools ? &aBool[0] : new bool[nEntryCount] );
    bool* pTest = ( nEntryCount <= nFixedBools ? &aTest[0] : new bool[nEntryCount] );

    long    nPos = -1;
    SCSIZE  i    = 0;
    CollatorWrapper* pCollator = (rParam.bCaseSens ? ScGlobal::GetCaseCollator() :
                                  ScGlobal::GetCollator() );
    ::utl::TransliterationWrapper* pTransliteration = (rParam.bCaseSens ?
                                                       ScGlobal::GetCaseTransliteration() : ScGlobal::GetpTransliteration());

    while ((i < nEntryCount) && rParam.GetEntry(i).bDoQuery)
    {
        ScQueryEntry& rEntry = rParam.GetEntry(i);
        // we can only handle one single direct query
        // #i115431# nField in QueryParam is the sheet column, not the field within the source range
        SCCOL nQueryCol = (SCCOL)rEntry.nField;
        if ( nQueryCol < rParam.nCol1 )
            nQueryCol = rParam.nCol1;
        if ( nQueryCol > rParam.nCol2 )
            nQueryCol = rParam.nCol2;
        SCCOL nSourceField = nQueryCol - rParam.nCol1;
        SCROW nId = GetItemDataId( nSourceField, nRow, FALSE );
        const ScDPItemData* pCellData = GetItemDataById( nSourceField, nId );

        bool bOk = false;
        bool bTestEqual = false;

        if (pSpecial && pSpecial[i])
        {
            if (rEntry.nVal == SC_EMPTYFIELDS)
                bOk = ! pCellData->IsHasData();
            else // if (rEntry.nVal == SC_NONEMPTYFIELDS)
                bOk =  pCellData->IsHasData();
        }
        else if (!rEntry.bQueryByString && pCellData->IsValue())
        {   // by Value
            double nCellVal = pCellData->GetValue();

            switch (rEntry.eOp)
            {
                case SC_EQUAL :
                    bOk = ::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_LESS :
                    bOk = (nCellVal < rEntry.nVal) && !::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_GREATER :
                    bOk = (nCellVal > rEntry.nVal) && !::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_LESS_EQUAL :
                    bOk = (nCellVal < rEntry.nVal) || ::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_GREATER_EQUAL :
                    bOk = (nCellVal > rEntry.nVal) || ::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_NOT_EQUAL :
                    bOk = !::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    break;
                default:
                    bOk= false;
                    break;
            }
        }
        else if ((rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL)
                 || (rEntry.bQueryByString
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
                else
                    bTestEqual = bMatch;
            }
            if (!bRealRegExp)
            {
                if (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL)
                {
                    if (bMatchWholeCell)
                    {
                        bOk = pTransliteration->isEqual( aCellStr, *rEntry.pStr );
                        String aStr = *rEntry.pStr;
                        sal_Bool bHasStar = sal_False;
                        xub_StrLen nIndex;
                        if (( nIndex = aStr.Search('*') ) != STRING_NOTFOUND)
                            bHasStar = sal_True;
                        if (bHasStar && (nIndex>0))
                        {
                            for (i=0;(i<nIndex) && (i< aCellStr.Len()) ; i++)
                            {
                                if (aCellStr.GetChar( (USHORT)i ) == aStr.GetChar((USHORT) i ))
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
                        ::com::sun::star::uno::Sequence< sal_Int32 > xOff;
                        String aCell( pTransliteration->transliterate(
                                                                     aCellStr, ScGlobal::eLnge, 0, aCellStr.Len(),
                                                                     &xOff ) );
                        String aQuer( pTransliteration->transliterate(
                                                                     *rEntry.pStr, ScGlobal::eLnge, 0, rEntry.pStr->Len(),
                                                                     &xOff ) );
                        bOk = (aCell.Search( aQuer ) != STRING_NOTFOUND);
                    }
                    if (rEntry.eOp == SC_NOT_EQUAL)
                        bOk = !bOk;
                }
                else
                {   // use collator here because data was probably sorted
                    sal_Int32 nCompare = pCollator->compareString(
                                                                 aCellStr, *rEntry.pStr );
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
                            DBG_ASSERT( false , "SC_NOT_EQUAL");
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
            pPasst[nPos] = bOk;
            pTest[nPos] = bTestEqual;
        }
        else
        {
            if (rEntry.eConnect == SC_AND)
            {
                pPasst[nPos] = pPasst[nPos] && bOk;
                pTest[nPos] = pTest[nPos] && bTestEqual;
            }
            else
            {
                nPos++;
                pPasst[nPos] = bOk;
                pTest[nPos] = bTestEqual;
            }
        }
        i++;
    }

    for (long j=1; j <= nPos; j++)
    {
        pPasst[0] = pPasst[0] || pPasst[j];
        pTest[0] = pTest[0] || pTest[j];
    }

    bool bRet = pPasst[0];
    if (pPasst != &aBool[0])
        delete [] pPasst;
    if (pTest != &aTest[0])
        delete [] pTest;

    return bRet;
}

bool ScDPCache::IsRowEmpty( SCROW nRow ) const
{
    return mbEmptyRow[ nRow ];
}

bool ScDPCache::IsEmptyMember( SCROW nRow, USHORT nColumn ) const
{
    return !GetItemDataById( nColumn, GetItemDataId( nColumn, nRow, false ) )->IsHasData();
}

bool ScDPCache::AddData(long nDim, ScDPItemData* pitemData)
{
    DBG_ASSERT( IsValid(), "  IsValid() == false " );
    DBG_ASSERT( nDim < mnColumnCount && nDim >=0 , "dimension out of bound" );
    SCROW nIndex = 0;

    bool bInserted = false;

    pitemData->SetDate( lcl_isDate( GetNumType( pitemData->nNumFormat ) ) );

    if ( !lcl_Search( maTableDataValues[nDim], maGlobalOrder[nDim], *pitemData, nIndex ) )
    {
        maTableDataValues[nDim].push_back( pitemData );
        maGlobalOrder[nDim].insert( maGlobalOrder[nDim].begin()+nIndex, maTableDataValues[nDim].size()-1  );
        DBG_ASSERT( (size_t) maGlobalOrder[nDim][nIndex] == maTableDataValues[nDim].size()-1 ,"ScDPTableDataCache::AddData ");
        maSourceData[nDim].push_back( maTableDataValues[nDim].size()-1 );
        bInserted = true;
    }
    else
        maSourceData[nDim].push_back( maGlobalOrder[nDim][nIndex] );
//init empty row tag
    size_t nCurRow = maSourceData[nDim].size() - 1;

    while ( mbEmptyRow.size() <= nCurRow )
        mbEmptyRow.push_back( true );

    if ( pitemData->IsHasData() )
        mbEmptyRow[ nCurRow ] = false;

    if ( !bInserted )
        delete pitemData;

    return true;
}


String ScDPCache::GetDimensionName( USHORT nColumn ) const
{
    DBG_ASSERT(nColumn < maLabelNames.size()-1 , "ScDPTableDataCache::GetDimensionName");
    DBG_ASSERT(maLabelNames.size() == static_cast <USHORT> (mnColumnCount+1), "ScDPTableDataCache::GetDimensionName");

    if ( static_cast<size_t>(nColumn+1) < maLabelNames.size() )
    {
        return maLabelNames[nColumn+1].aString;
    }
    else
        return String();
}

void ScDPCache::AddLabel(ScDPItemData *pData)
{
    DBG_ASSERT( IsValid(), "  IsValid() == false " );

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

SCROW ScDPCache::GetItemDataId(USHORT nDim, SCROW nRow, bool bRepeatIfEmpty) const
{
    DBG_ASSERT( IsValid(), "  IsValid() == false " );
    DBG_ASSERT( /* nDim >= 0 && */ nDim < mnColumnCount, "ScDPTableDataCache::GetItemDataId " );

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
    DBG_ASSERT( nDim>=0 && nDim < mnColumnCount ," nDim < mnColumnCount ");
    return maTableDataValues[nDim];
}

SCROW ScDPCache::GetSortedItemDataId(SCCOL nDim, SCROW nOrder) const
{
    DBG_ASSERT ( IsValid(), "IsValid");
    DBG_ASSERT( nDim>=0 && nDim < mnColumnCount,  "nDim < mnColumnCount");
    DBG_ASSERT( nOrder >= 0 && (size_t) nOrder < maGlobalOrder[nDim].size(), "nOrder < mpGlobalOrder[nDim].size()" );

    return maGlobalOrder[nDim][nOrder];
}

ULONG ScDPCache::GetNumType(ULONG nFormat) const
{
    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
    ULONG nType = NUMBERFORMAT_NUMBER;
    if ( pFormatter )
        nType = pFormatter->GetType( nFormat );
    return nType;
}

ULONG ScDPCache::GetNumberFormat( long nDim ) const
{
    if ( nDim >= mnColumnCount )
        return 0;
    if ( maTableDataValues[nDim].size()==0 )
        return 0;
    else
        return maTableDataValues[nDim][0].nNumFormat;
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
    DBG_ASSERT( nDim>=0 && nDim < mnColumnCount ," ScDPTableDataCache::GetDimMemberCount : out of bound ");
    return maTableDataValues[nDim].size();
}

const ScDPItemData* ScDPCache::GetSortedItemData(SCCOL nDim, SCROW nOrder) const
{
    SCROW n = GetSortedItemDataId( nDim, nOrder );
    return GetItemDataById( nDim, n );
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

SCROW ScDPCache::GetIdByItemData(long nDim, String sItemData ) const
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

SCROW ScDPCache::GetAdditionalItemID ( String sItemData ) const
{
    ScDPItemData rData ( sItemData );
    return GetAdditionalItemID( rData );
}

SCROW ScDPCache::GetAdditionalItemID( const ScDPItemData& rData ) const
{
    return GetRowCount() + maAdditionalData.insertData( rData );
}


SCROW ScDPCache::GetOrder(long nDim, SCROW nIndex) const
{
    DBG_ASSERT( IsValid(), "  IsValid() == false " );
    DBG_ASSERT( nDim >=0 && nDim < mnColumnCount, "ScDPTableDataCache::GetOrder : out of bound" );

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

    DBG_ASSERT( nIndex>=0 && (size_t)nIndex < maIndexOrder[nDim].size() , "ScDPTableDataCache::GetOrder");
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
