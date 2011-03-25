/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dptablecache.cxx,v $
 * $Revision: 1.0 $
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
// INCLUDE ---------------------------------------------------------------
#include "dptablecache.hxx"
#include "dptabdat.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "globstr.hrc"

#include <rtl/math.hxx>
#include "queryparam.hxx"
#include "dpglobal.hxx"

#include "docoptio.hxx" //for ValidQuery
#include <unotools/textsearch.hxx> //for ValidQuery

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
// -----------------------------------------------------------------------
namespace
{
    sal_Bool lcl_isDate( sal_uLong nNumType )
    {
        return ( (nNumType & NUMBERFORMAT_DATE) != 0 )? 1:0 ;
    }

    sal_Bool lcl_Search( const std::vector<ScDPItemData*>& list, const ::std::vector<SCROW>& rOrder, const ScDPItemData& item, SCROW& rIndex)
    {
        rIndex = list.size();
        sal_Bool bFound = sal_False;
        SCROW nLo = 0;
        SCROW nHi = list.size() - 1;
        SCROW nIndex;
        long nCompare;
        while (nLo <= nHi)
        {
            nIndex = (nLo + nHi) / 2;
            nCompare = ScDPItemData::Compare( *list[rOrder[nIndex]], item );
            if (nCompare < 0)
                nLo = nIndex + 1;
            else
            {
                nHi = nIndex - 1;
                if (nCompare == 0)
                {
                    bFound = sal_True;
                    nLo = nIndex;
                }
            }
        }
        rIndex = nLo;
        return bFound;
    }

    ScDPItemData*  lcl_GetItemValue(const Reference<sdbc::XRow>& xRow, sal_Int32 nType, long nCol,
                  const Date& rNullDate )
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
                    return new ScDPItemData( rStr, fValue,sal_True,nNumType);
                }
                //break;

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
                    return new ScDPItemData( rStr, fValue,sal_True);
                }
                //break;

            case sdbc::DataType::DATE:
                {
                    nNumType = NUMBERFORMAT_DATE;

                    util::Date aDate = xRow->getDate(nCol);
                    fValue = Date(aDate.Day, aDate.Month, aDate.Year) - rNullDate;
                    return new ScDPItemData( rStr, fValue, sal_True, nNumType );
                }
                //break;

            case sdbc::DataType::TIME:
                {
                    nNumType = NUMBERFORMAT_TIME;

                    util::Time aTime = xRow->getTime(nCol);
                    fValue = ( aTime.Hours * 3600 + aTime.Minutes * 60 +
                        aTime.Seconds + aTime.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
                    return new ScDPItemData( rStr,fValue, sal_True, nNumType );
                }
                //break;

            case sdbc::DataType::TIMESTAMP:
                {
                    nNumType = NUMBERFORMAT_DATETIME;

                    util::DateTime aStamp = xRow->getTimestamp(nCol);
                    fValue = ( Date( aStamp.Day, aStamp.Month, aStamp.Year ) - rNullDate ) +
                        ( aStamp.Hours * 3600 + aStamp.Minutes * 60 +
                        aStamp.Seconds + aStamp.HundredthSeconds / 100.0 ) / D_TIMEFACTOR;
                    return new ScDPItemData( rStr,fValue, sal_True, nNumType );
                }
                //break;
            case sdbc::DataType::CHAR:
            case sdbc::DataType::VARCHAR:
            case sdbc::DataType::LONGVARCHAR:
            case sdbc::DataType::SQLNULL:
            case sdbc::DataType::BINARY:
            case sdbc::DataType::VARBINARY:
            case sdbc::DataType::LONGVARBINARY:
            default:
                return new ScDPItemData ( rStr );
                //break;
            }
        }
        catch (uno::Exception&)
        {
        }
        catch ( ... )
        {

        }
      return NULL;
    }
}
// Wang Xu Ming -- 12/23/2008
//Refactor cache data
ScDPItemData::ScDPItemData( const String& rS, double fV/* = 0.0*/, sal_Bool bHV/* = sal_False*/, const sal_uLong nNumFormatP /*= 0*/ , sal_Bool bData/* = sal_True*/) :
nNumFormat( nNumFormatP ), aString(rS), fValue(fV),
mbFlag( (MK_VAL*!!bHV) | (MK_DATA*!!bData) | (MK_ERR*!!sal_False) | (MK_DATE*!!lcl_isDate( nNumFormat ) ) )
{
}

ScDPItemData::ScDPItemData( ScDocument* pDoc, SCROW nRow, sal_uInt16 nCol, sal_uInt16 nDocTab  ):
        nNumFormat( 0 ), fValue(0.0), mbFlag( 0 )
{
    String aDocStr;
    pDoc->GetString( nCol, nRow, nDocTab, aDocStr );

    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();

    ScAddress aPos( nCol, nRow, nDocTab );
    ScBaseCell* pCell = pDoc->GetCell( aPos );

    if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA && ((ScFormulaCell*)pCell)->GetErrCode() )
    {
        SetString ( aDocStr );      //[SODC_19347] add liyi
        //bErr = sal_True;              //[SODC_19347] del liyi
        mbFlag |= MK_ERR;
    }
    else if ( pDoc->HasValueData( nCol, nRow, nDocTab ) )
    {
        double fVal = pDoc->GetValue(ScAddress(nCol, nRow, nDocTab));
        sal_uLong nFormat = NUMBERFORMAT_NUMBER;
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
// End Comments

sal_Bool ScDPItemData::IsCaseInsEqual( const ScDPItemData& r ) const
{ //TODO: indified Date?
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

sal_Bool ScDPItemData::operator==( const ScDPItemData& r ) const
{
    if ( IsValue() )
    {
        if( (HasDatePart() != r.HasDatePart())  || (HasDatePart() && mnDatePart != r.mnDatePart) )
            return sal_False;

// Wang Xu Ming -- 1/9/2009
// Add Data Cache Support.
// Identify date
        if ( IsDate() != r.IsDate() )
            return sal_False;
      else
        if ( r.IsValue() )
            return rtl::math::approxEqual( fValue, r.fValue );
        else
            return sal_False;
// End Comments
    }
    else if ( r.IsValue() )
        return sal_False;
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
// Wang Xu Ming -- 1/9/2009
// Add Data Cache Support.
// Date > number
                if ( rA.IsDate() == rB.IsDate() )
                    return 0;
                else
                    return rA.IsDate() ? 1: -1;
// End Comments
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
//
//Wang Xu Ming SODC_17561
#ifdef DEBUG
void    ScDPItemData::dump() const
{
    DBG_TRACE1( "Numberformat= %o",  nNumFormat );
    DBG_TRACESTR(aString );
    DBG_TRACE1( "fValue= %f", fValue );
    DBG_TRACE1( "mbFlag= %d", mbFlag);
}
#endif
//End

TypedStrData*  ScDPItemData::CreateTypeString( )
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

sal_Bool ScDPItemData::IsHasData() const
{
    return !!(mbFlag&MK_DATA);
}

sal_Bool ScDPItemData::IsHasErr() const
{
    return !!(mbFlag&MK_ERR);
}

sal_Bool ScDPItemData::IsValue() const
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
sal_uLong  ScDPItemData::GetNumFormat() const
{
    return nNumFormat;
}

sal_Bool ScDPItemData::HasStringData() const

{
    return IsHasData()&&!IsHasErr()&&!IsValue();
}
sal_Bool ScDPItemData::IsDate() const
{
    return !!(mbFlag&MK_DATE);
}
sal_Bool ScDPItemData::HasDatePart() const
{
    return !!(mbFlag&MK_DATEPART);
}
void ScDPItemData::SetDate( sal_Bool b )
{
    b ? ( mbFlag |= MK_DATE ) : ( mbFlag &= ~MK_DATE );
}

// -----------------------------------------------------------------------
//class ScDPTableDataCache
//To cache the pivot table data source

sal_Bool ScDPTableDataCache::operator== ( const ScDPTableDataCache& r ) const
{
    if ( GetColumnCount() == r.GetColumnCount() )
    {
        for ( SCCOL i = 0 ; i < GetColumnCount(); i++ )
        {   //check dim names
            if ( GetDimensionName( i ) != r.GetDimensionName( i ) )
                return sal_False;
            //check rows count
            if ( GetRowCount() != r.GetRowCount() )
                return sal_False;
            //check dim member values
            size_t nMembersCount = GetDimMemberValues( i ).size();
            if ( GetDimMemberValues( i ).size() == r. GetDimMemberValues( i ).size() )
            {
                for ( size_t j = 0; j < nMembersCount; j++ )
                {
                    if ( *( GetDimMemberValues( i )[j] ) == *( r.GetDimMemberValues( i )[j] ) )
                        continue;
                    else
                        return sal_False;
                }
            }
            else
                return sal_False;
            //check source table index
            for ( SCROW k=0 ; k < GetRowCount(); k ++ )
            {
                if ( GetItemDataId( i, k, sal_False ) == r.GetItemDataId( i,k,sal_False) )
                    continue;
                else
                    return sal_False;
            }
        }
    }
    return sal_True;
}

ScDPTableDataCache::ScDPTableDataCache(  ScDocument* pDoc  ) :
mpDoc( pDoc ),
mnColumnCount ( 0 ),
mpTableDataValues ( NULL ),
mpSourceData ( NULL ),
mpGlobalOrder( NULL ),
mpIndexOrder( NULL)
{
    mnID = -1;
}

ScDPTableDataCache::~ScDPTableDataCache()
{
    if ( IsValid() )
    {
// Wang Xu Ming -- 2/17/2009
// Performance issue
        sal_uInt16 nCol;
        for (  nCol=0; nCol < GetColumnCount() ; nCol++ )
        {
            for ( sal_uLong row = 0 ;  row < mpTableDataValues[nCol].size(); row++ )
                delete mpTableDataValues[nCol][row];
        }
        for ( nCol =0; nCol < mrLabelNames.size(); nCol++ )
                delete mrLabelNames[nCol];
// End Comments

        mnColumnCount = 0;
        delete [] mpTableDataValues;
        mpTableDataValues = NULL;
        delete [] mpSourceData;
        mpSourceData = NULL;
        delete [] mpGlobalOrder;
        mpGlobalOrder = NULL;
        delete [] mpIndexOrder;
        mpIndexOrder = NULL;
    }
}

// -----------------------------------------------------------------------
void ScDPTableDataCache::AddRow( ScDPItemData* pRow, sal_uInt16 nCount )
{
    DBG_ASSERT( pRow , " empty pointer" );
    if ( !mrLabelNames.size() )
    {
        mnColumnCount= nCount;
        mpTableDataValues = new std::vector<ScDPItemData*>[ mnColumnCount ];
        mpSourceData      = new std::vector<SCROW>[ mnColumnCount ];
        mpGlobalOrder     = new std::vector<SCROW>[ mnColumnCount ];
        mpIndexOrder      = new std::vector<SCROW>[ mnColumnCount ];

        for ( sal_uInt16 i = 0; i < nCount ; i ++ )
            AddLabel( new ScDPItemData( pRow[i] ) );
    }
    else
    {
        for ( sal_uInt16 i = 0; i < nCount && i < mnColumnCount; i ++ )
            AddData( i, new ScDPItemData( pRow[i] ) );
    }
}

// -----------------------------------------------------------------------
bool  ScDPTableDataCache::IsValid() const
{ //TODO: continue check valid
    return mpTableDataValues!=NULL && mpSourceData!= NULL && mnColumnCount>0;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------
bool ScDPTableDataCache::InitFromDoc(  ScDocument* pDoc, const ScRange& rRange )
{
    // Make sure the formula cells within the data range are interpreted
    // during this call, for this method may be called from the interpretation
    // of GETPIVOTDATA, which disables nested formula interpretation without
    // increasing the macro level.
    MacroInterpretIncrementer aMacroInc(pDoc);

    //
    SCROW nStartRow = rRange.aStart.Row();  // start of data
    SCROW nEndRow = rRange.aEnd.Row();
    sal_uInt16 nStartCol = rRange.aStart.Col();
    sal_uInt16 nEndCol = rRange.aEnd.Col();
    sal_uInt16 nDocTab = rRange.aStart.Tab();

    //init
     long nOldColumCount = mnColumnCount;
    mnColumnCount = nEndCol - nStartCol + 1;
    if ( IsValid() )
    {
        for ( sal_uInt16 nCol=0; nCol < nOldColumCount ; nCol++ )
        {
            for ( sal_uLong row = 0 ;  row < mpTableDataValues[nCol].size(); row++ )
                delete mpTableDataValues[nCol][row];
            delete mrLabelNames[nCol];
        }
        delete [] mpTableDataValues;
        delete [] mpSourceData;
        delete [] mpGlobalOrder;
        delete [] mpIndexOrder;
        mrLabelNames.clear();
    }

    mpTableDataValues = new std::vector<ScDPItemData*>[ mnColumnCount ];
    mpSourceData      = new std::vector<SCROW>[ mnColumnCount ];
    mpGlobalOrder     = new std::vector<SCROW>[ mnColumnCount ];
    mpIndexOrder      = new std::vector<SCROW>[ mnColumnCount ];
    //check valid
    for ( SCROW nRow = nStartRow; nRow <= nEndRow; nRow ++ )
    {
        for ( sal_uInt16 nCol = nStartCol; nCol <= nEndCol; nCol++ )
        {
            if ( nRow == nStartRow )
                AddLabel( new ScDPItemData( pDoc, nRow, nCol, nDocTab  ) );
            else
                AddData( nCol - nStartCol, new ScDPItemData( pDoc, nRow, nCol, nDocTab  ) );
        }
    }
    return sal_True;
}

// -----------------------------------------------------------------------
bool ScDPTableDataCache::InitFromDataBase (const Reference<sdbc::XRowSet>& xRowSet, const Date& rNullDate)
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

       long nOldColumCount = mnColumnCount;
    mnColumnCount = xMeta->getColumnCount();
    if ( IsValid() )
    {
        for ( sal_uInt16 nCol=0; nCol < nOldColumCount ; nCol++ )
        {
            for ( sal_uLong row = 0 ;  row < mpTableDataValues[nCol].size(); row++ )
                delete mpTableDataValues[nCol][row];
            delete mrLabelNames[nCol];
        }
        delete [] mpTableDataValues;
        delete [] mpSourceData;
        delete [] mpGlobalOrder;
        delete [] mpIndexOrder;
        mrLabelNames.clear();
    }
        // Get column titles and types.
    mrLabelNames.reserve(mnColumnCount);
    mpTableDataValues = new std::vector<ScDPItemData*>[ mnColumnCount ];
    mpSourceData      = new std::vector<SCROW>[ mnColumnCount ];
    mpGlobalOrder     = new std::vector<SCROW>[ mnColumnCount ];
    mpIndexOrder      = new std::vector<SCROW>[ mnColumnCount ];

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
                if ( pNew )
                    AddData(  nCol , pNew );
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
// -----------------------------------------------------------------------
sal_uLong ScDPTableDataCache::GetDimNumType( SCCOL nDim) const
{
    DBG_ASSERT( IsValid(), "  IsValid() == false " );
    DBG_ASSERT( nDim < mnColumnCount && nDim >=0, " dimention out of bound " );
    if ( mpTableDataValues[nDim].size()==0 )
        return NUMBERFORMAT_UNDEFINED;
    else
        return GetNumType(mpTableDataValues[nDim][0]->nNumFormat);
}

// -----------------------------------------------------------------------
bool ScDPTableDataCache::ValidQuery( SCROW nRow, const ScQueryParam &rParam, sal_Bool *pSpecial)
{ //Copied and modified from ScTable::ValidQuery
        if (!rParam.GetEntry(0).bDoQuery)
            return sal_True;
        sal_Bool    bMatchWholeCell = mpDoc->GetDocOptions().IsMatchWholeCell();

        //---------------------------------------------------------------

        const SCSIZE nFixedBools = 32;
        sal_Bool aBool[nFixedBools];
        sal_Bool aTest[nFixedBools];
        SCSIZE nEntryCount = rParam.GetEntryCount();
        sal_Bool* pPasst = ( nEntryCount <= nFixedBools ? &aBool[0] : new sal_Bool[nEntryCount] );
        sal_Bool* pTest = ( nEntryCount <= nFixedBools ? &aTest[0] : new sal_Bool[nEntryCount] );

        long    nPos = -1;
        SCSIZE  i    = 0;
        CollatorWrapper* pCollator = (rParam.bCaseSens ? ScGlobal::GetCaseCollator() :
            ScGlobal::GetCollator() );
        ::utl::TransliterationWrapper* pTransliteration = (rParam.bCaseSens ?
            ScGlobal::GetCaseTransliteration() : ScGlobal::GetpTransliteration());

        while ( (i < nEntryCount) && rParam.GetEntry(i).bDoQuery )
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
            SCROW nId = GetItemDataId( nSourceField, nRow, sal_False );
            const ScDPItemData* pCellData = GetItemDataById( nSourceField, nId );

            sal_Bool bOk = sal_False;
            sal_Bool bTestEqual = sal_False;

            if ( pSpecial && pSpecial[i] )
            {
                if (rEntry.nVal == SC_EMPTYFIELDS)
                    bOk = ! pCellData->IsHasData();
                else // if (rEntry.nVal == SC_NONEMPTYFIELDS)
                    bOk =  pCellData->IsHasData();
            }
            else if ( !rEntry.bQueryByString && pCellData->IsValue() )
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
                                    bOk= sal_False;
                                    break;
                }
            }
            else if ( (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL)
                    || (rEntry.bQueryByString
                        && pCellData->HasStringData() )
                )
            {   // by String
                String  aCellStr = pCellData->GetString();

                sal_Bool bRealRegExp = (rParam.bRegExp && ((rEntry.eOp == SC_EQUAL)
                    || (rEntry.eOp == SC_NOT_EQUAL)));
                sal_Bool bTestRegExp = sal_False;
                if ( bRealRegExp || bTestRegExp )
                {
                    xub_StrLen nStart = 0;
                    xub_StrLen nEnd   = aCellStr.Len();
                    sal_Bool bMatch = (sal_Bool) rEntry.GetSearchTextPtr( rParam.bCaseSens )
                        ->SearchFrwrd( aCellStr, &nStart, &nEnd );
                    // from 614 on, nEnd is behind the found text
                    if ( bMatch && bMatchWholeCell
                            && (nStart != 0 || nEnd != aCellStr.Len()) )
                        bMatch = sal_False;    // RegExp must match entire cell string
                    if ( bRealRegExp )
                        bOk = ((rEntry.eOp == SC_NOT_EQUAL) ? !bMatch : bMatch);
                    else
                        bTestEqual = bMatch;
                }
                if ( !bRealRegExp )
                {
                    if ( rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL )
                    {
                        if ( bMatchWholeCell )
                        {
                                        bOk = pTransliteration->isEqual( aCellStr, *rEntry.pStr );
                            //Added by zhaosz,for sodc_2702,20060808
                            String aStr = *rEntry.pStr;//"f*"
                            //modified by weihuaw,for SODC_16698
                            //use another way to find "*" in aStr
                            sal_Bool bHasStar = sal_False;
                            xub_StrLen nIndex;
                            if( ( nIndex = aStr.Search('*') ) != STRING_NOTFOUND )
                                bHasStar = sal_True;
                            if(bHasStar && (nIndex>0))
                            {
                                for(i=0;(i<nIndex) && (i< aCellStr.Len()) ; i++)
                                {
                                    if(aCellStr.GetChar( (sal_uInt16)i ) == aStr.GetChar((sal_uInt16) i ))
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
                            //end modified
                            //Added end,20060808
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
                        if ( rEntry.eOp == SC_NOT_EQUAL )
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

        for ( long j=1; j <= nPos; j++ )
        {
            pPasst[0] = pPasst[0] || pPasst[j];
            pTest[0] = pTest[0] || pTest[j];
        }

        sal_Bool bRet = pPasst[0];
        if ( pPasst != &aBool[0] )
            delete [] pPasst;
        if ( pTest != &aTest[0] )
            delete [] pTest;

        return bRet;
}

// -----------------------------------------------------------------------
bool ScDPTableDataCache::IsRowEmpty( SCROW nRow ) const
{
    return  mbEmptyRow[ nRow ];

}

// -----------------------------------------------------------------------
bool ScDPTableDataCache::IsEmptyMember( SCROW nRow, sal_uInt16 nColumn ) const
{
    return !GetItemDataById( nColumn, GetItemDataId( nColumn, nRow, sal_False ) )->IsHasData();
}

sal_Bool ScDPTableDataCache::AddData(long nDim, ScDPItemData* pitemData)
{
    DBG_ASSERT( IsValid(), "  IsValid() == false " );
    DBG_ASSERT( nDim < mnColumnCount && nDim >=0 , "dimension out of bound" );
    SCROW nIndex = 0;

    sal_Bool    bInserted = sal_False;

    pitemData->SetDate( lcl_isDate( GetNumType( pitemData->nNumFormat ) ) );

    if ( !lcl_Search( mpTableDataValues[nDim], mpGlobalOrder[nDim], *pitemData, nIndex ) )
    {
        mpTableDataValues[nDim].push_back( pitemData );
        mpGlobalOrder[nDim].insert( mpGlobalOrder[nDim].begin()+nIndex, mpTableDataValues[nDim].size()-1  );
        DBG_ASSERT( (size_t) mpGlobalOrder[nDim][nIndex] == mpTableDataValues[nDim].size()-1 ,"ScDPTableDataCache::AddData ");
        mpSourceData[nDim].push_back( mpTableDataValues[nDim].size()-1 );
        bInserted = sal_True;
    }
    else
        mpSourceData[nDim].push_back( mpGlobalOrder[nDim][nIndex] );
//init empty row tag
    size_t  nCurRow = mpSourceData[nDim].size() -1 ;

    while ( mbEmptyRow.size() <= nCurRow )
        mbEmptyRow.push_back( sal_True );

    if ( pitemData->IsHasData() )
        mbEmptyRow[ nCurRow ] = sal_False;

    if ( !bInserted )
        delete pitemData;

    return sal_True;
}


String ScDPTableDataCache::GetDimensionName( sal_uInt16 nColumn ) const
{
    DBG_ASSERT( /* nColumn>=0 && */ nColumn < mrLabelNames.size()-1 , "ScDPTableDataCache::GetDimensionName");
    DBG_ASSERT( mrLabelNames.size() == static_cast <sal_uInt16> (mnColumnCount+1), "ScDPTableDataCache::GetDimensionName");
    if ( static_cast<size_t>(nColumn+1) < mrLabelNames.size() )
    {
        return mrLabelNames[nColumn+1]->aString;
    }
    else
        return String();
}

void ScDPTableDataCache::AddLabel(ScDPItemData *pData)
{
    DBG_ASSERT( IsValid(), "  IsValid() == false " );

    if ( mrLabelNames.size() == 0 )
        mrLabelNames.push_back( new ScDPItemData(  ScGlobal::GetRscString(STR_PIVOT_DATA) ) );


    //reset name if needed
    String strNewName = pData->aString;
    sal_Bool bFound = sal_False;
    long nIndex = 1;
    do
    {
        for ( long i= mrLabelNames.size()-1; i>=0; i-- )
        {
            if( mrLabelNames[i]->aString == strNewName )
            {
                strNewName  =  pData->aString;
                strNewName += String::CreateFromInt32( nIndex );
                nIndex ++ ;
                bFound = sal_True;
            }
        }
        bFound = !bFound;
    }
    while ( !bFound );

    pData->aString = strNewName;
    mrLabelNames.push_back( pData );
}

SCROW ScDPTableDataCache::GetItemDataId(sal_uInt16 nDim, SCROW nRow, sal_Bool bRepeatIfEmpty) const
{ //
    DBG_ASSERT( IsValid(), "  IsValid() == false " );
    DBG_ASSERT( /* nDim >= 0 && */ nDim < mnColumnCount, "ScDPTableDataCache::GetItemDataId " );

    if ( bRepeatIfEmpty )
    {
        while ( nRow >0 && !mpTableDataValues[nDim][ mpSourceData[nDim][nRow] ]->IsHasData() )
        --nRow;
    }

    return mpSourceData[nDim][nRow];
}

const ScDPItemData* ScDPTableDataCache::GetItemDataById(long nDim, SCROW nId) const
{
    if ( nId >= GetRowCount()  )
        return maAdditionalDatas.getData( nId - GetRowCount() );

    if (  (size_t)nId >= mpTableDataValues[nDim].size() || nDim >= mnColumnCount  || nId < 0  )
        return NULL;
    else
        return mpTableDataValues[nDim][nId];
}

SCROW ScDPTableDataCache::GetRowCount() const
{
    if ( IsValid() )
        return mpSourceData[0].size();
    else
        return 0;
}

const std::vector<ScDPItemData*>& ScDPTableDataCache::GetDimMemberValues(SCCOL nDim) const
{
    DBG_ASSERT( nDim>=0 && nDim < mnColumnCount ," nDim < mnColumnCount ");
    return mpTableDataValues[nDim];
}

SCROW ScDPTableDataCache::GetSortedItemDataId(SCCOL nDim, SCROW nOrder) const
{
    DBG_ASSERT ( IsValid(), "IsValid");
    DBG_ASSERT( nDim>=0 && nDim < mnColumnCount,  "nDim < mnColumnCount");
    DBG_ASSERT( nOrder >= 0 && (size_t) nOrder < mpGlobalOrder[nDim].size(), "nOrder < mpGlobalOrder[nDim].size()" );

    return mpGlobalOrder[nDim][nOrder];
}

sal_uLong ScDPTableDataCache::GetNumType(sal_uLong nFormat) const
{
    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
    sal_uLong nType = NUMBERFORMAT_NUMBER;
    if ( pFormatter )
        nType = pFormatter->GetType( nFormat );
    return nType;
}

sal_uLong ScDPTableDataCache::GetNumberFormat( long nDim ) const
{
    if ( nDim >= mnColumnCount )
        return 0;
    if ( mpTableDataValues[nDim].size()==0 )
        return 0;
    else
        return mpTableDataValues[nDim][0]->nNumFormat;
}

sal_Bool ScDPTableDataCache::IsDateDimension( long nDim ) const
{
    if ( nDim >= mnColumnCount )
        return false;
    else if ( mpTableDataValues[nDim].size()==0 )
        return false;
    else
        return mpTableDataValues[nDim][0]->IsDate();

}

SCROW ScDPTableDataCache::GetDimMemberCount( SCCOL nDim ) const
{
    DBG_ASSERT( nDim>=0 && nDim < mnColumnCount ," ScDPTableDataCache::GetDimMemberCount : out of bound ");
    return mpTableDataValues[nDim].size();
}

const ScDPItemData* ScDPTableDataCache::GetSortedItemData(SCCOL nDim, SCROW nOrder) const
{
    SCROW n = GetSortedItemDataId( nDim, nOrder );
    return GetItemDataById( nDim, n );
}

SCCOL ScDPTableDataCache::GetDimensionIndex(String sName) const
{
    for ( size_t n = 1; n < mrLabelNames.size(); n ++ ) //defects, label name map wrong SODC_17590, SODC_18932,SODC_18827,SODC_18960,SODC_18923
    {
        if ( mrLabelNames[n]->GetString() == sName )
            return (SCCOL)(n-1);
    }
    return -1;
}

SCROW ScDPTableDataCache::GetIdByItemData(long nDim, String sItemData ) const
{
    if ( nDim < mnColumnCount && nDim >=0 )
    {
        for ( size_t n = 0; n< mpTableDataValues[nDim].size(); n++ )
        {
            if ( mpTableDataValues[nDim][n]->GetString() == sItemData )
                return n;
        }
    }

    ScDPItemData rData ( sItemData );
    return  GetRowCount() +maAdditionalDatas.getDataId(rData);
}

SCROW ScDPTableDataCache::GetIdByItemData( long nDim, const ScDPItemData& rData  ) const
{
    if ( nDim < mnColumnCount && nDim >=0 )
    {
        for ( size_t n = 0; n< mpTableDataValues[nDim].size(); n++ )
        {
            if ( *mpTableDataValues[nDim][n] == rData )
                return n;
        }
    }
    return  GetRowCount() + maAdditionalDatas.getDataId(rData);
}

SCROW ScDPTableDataCache::GetAdditionalItemID ( String sItemData )
{
    ScDPItemData rData ( sItemData );
    return GetAdditionalItemID( rData );
}

SCROW ScDPTableDataCache::GetAdditionalItemID( const ScDPItemData& rData )
{
    return GetRowCount() + maAdditionalDatas.insertData( rData );
}


SCROW ScDPTableDataCache::GetOrder(long nDim, SCROW nIndex) const
{
    DBG_ASSERT( IsValid(), "  IsValid() == false " );
    DBG_ASSERT( nDim >=0 && nDim < mnColumnCount, "ScDPTableDataCache::GetOrder : out of bound" );

    if ( mpIndexOrder[nDim].size() !=  mpGlobalOrder[nDim].size() )
    { //not inited
        SCROW i  = 0;
        mpIndexOrder[nDim].resize(  mpGlobalOrder[nDim].size(), 0 );
        for ( size_t n = 0 ; n<  mpGlobalOrder[nDim].size(); n++ )
        {
            i =  mpGlobalOrder[nDim][n];
            mpIndexOrder[nDim][ i ] = n;
        }
    }

    DBG_ASSERT( nIndex>=0 && (size_t)nIndex < mpIndexOrder[nDim].size() , "ScDPTableDataCache::GetOrder");
    return  mpIndexOrder[nDim][nIndex];
}

ScDocument*  ScDPTableDataCache::GetDoc() const
{
    return mpDoc;
};

long ScDPTableDataCache::GetColumnCount() const
{
    return mnColumnCount;
}
long    ScDPTableDataCache::GetId() const
{
    return mnID;
}

