/*************************************************************************
 *
 *  $RCSfile: dpsdbtab.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE --------------------------------------------------------------

#include <tools/debug.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/zforlist.hxx>
#include <unotools/processfactory.hxx>

#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "dpsdbtab.hxx"
#include "collect.hxx"
#include "global.hxx"
#include "globstr.hrc"

using namespace com::sun::star;

#define SC_SERVICE_ROWSET           "com.sun.star.sdb.RowSet"

//! move to a header file?
#define SC_DBPROP_DATASOURCENAME    "DataSourceName"
#define SC_DBPROP_COMMAND           "Command"
#define SC_DBPROP_COMMANDTYPE       "CommandType"

// -----------------------------------------------------------------------

class ScDatabaseDPData_Impl
{
public:
    ScImportSourceDesc  aDesc;
    long                nColCount;
    BOOL                bValid;
    BOOL                bAtStart;
    String*             pTitles;
    TypedStrCollection** ppStrings;
    uno::Reference<sdbc::XRowSet> xRowSet;
    sal_Int32*          pTypes;
    SvNumberFormatter*  pFormatter;

    ScDatabaseDPData_Impl() {}
};

// -----------------------------------------------------------------------

ScDatabaseDPData::ScDatabaseDPData( const ScImportSourceDesc& rImport )
{
    pImpl = new ScDatabaseDPData_Impl;
    pImpl->aDesc = rImport;
    pImpl->nColCount = 0;
    pImpl->bValid = FALSE;
    pImpl->bAtStart = FALSE;
    pImpl->pTitles = NULL;
    pImpl->ppStrings = NULL;
    pImpl->pTypes = NULL;
    pImpl->pFormatter = NULL;       // created on demand

    OpenDatabase();
}

ScDatabaseDPData::~ScDatabaseDPData()
{
    delete[] pImpl->pTypes;
    if ( pImpl->ppStrings )
    {
        for (long i=0; i<pImpl->nColCount; i++)
            delete pImpl->ppStrings[i];
        delete[] pImpl->ppStrings;
    }
    delete[] pImpl->pTitles;
    delete pImpl->pFormatter;       // NumberFormatter is local for this object
    delete pImpl;
}

void ScDatabaseDPData::DisposeData()
{
    //! use OpenDatabase here?

    //! column titles ???

    //  collections for column entries
    if ( pImpl->ppStrings )
    {
        for (long i=0; i<pImpl->nColCount; i++)
        {
            delete pImpl->ppStrings[i];
            pImpl->ppStrings[i] = NULL;
        }
    }

    //! init entries on demand!
    InitAllColumnEntries();             //! configurable ???
}

BOOL ScDatabaseDPData::OpenDatabase()
{
    sal_Int32 nSdbType = -1;
    switch ( pImpl->aDesc.nType )
    {
        case sheet::DataImportMode_SQL:     nSdbType = sdb::CommandType::COMMAND;   break;
        case sheet::DataImportMode_TABLE:   nSdbType = sdb::CommandType::TABLE;     break;
        case sheet::DataImportMode_QUERY:   nSdbType = sdb::CommandType::QUERY;     break;
        default:
            return FALSE;
    }

    BOOL bSuccess = FALSE;
    try
    {
        pImpl->xRowSet = uno::Reference<sdbc::XRowSet>(
                utl::getProcessServiceFactory()->createInstance(
                    rtl::OUString::createFromAscii( SC_SERVICE_ROWSET ) ),
                uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xRowProp( pImpl->xRowSet, uno::UNO_QUERY );
        DBG_ASSERT( xRowProp.is(), "can't get RowSet" );
        if ( xRowProp.is() )
        {
            //
            //  set source parameters
            //

            uno::Any aAny;

            aAny <<= rtl::OUString( pImpl->aDesc.aDBName );
            xRowProp->setPropertyValue(
                        rtl::OUString::createFromAscii(SC_DBPROP_DATASOURCENAME), aAny );

            aAny <<= rtl::OUString( pImpl->aDesc.aObject );
            xRowProp->setPropertyValue(
                        rtl::OUString::createFromAscii(SC_DBPROP_COMMAND), aAny );

            aAny <<= nSdbType;
            xRowProp->setPropertyValue(
                        rtl::OUString::createFromAscii(SC_DBPROP_COMMANDTYPE), aAny );

            pImpl->xRowSet->execute();
            pImpl->bAtStart = TRUE;

            //
            //  get column descriptions
            //

            pImpl->nColCount = 0;
            uno::Reference<sdbc::XResultSetMetaData> xMeta;
            uno::Reference<sdbc::XResultSetMetaDataSupplier> xMetaSupp( pImpl->xRowSet, uno::UNO_QUERY );
            if ( xMetaSupp.is() )
                xMeta = xMetaSupp->getMetaData();
            if ( xMeta.is() )
                pImpl->nColCount = xMeta->getColumnCount(); // this is the number of real columns

            uno::Reference<sdbc::XResultSet> xResSet( pImpl->xRowSet, uno::UNO_QUERY );
            if ( pImpl->nColCount > 0 && xResSet.is() )
            {
                //  get column titles
                pImpl->pTitles = new String[pImpl->nColCount];
                pImpl->pTypes = new sal_Int32[pImpl->nColCount];
                for (long nCol=0; nCol<pImpl->nColCount; nCol++)
                {
                    pImpl->pTitles[nCol] = xMeta->getColumnLabel( nCol+1 );
                    pImpl->pTypes[nCol]  = xMeta->getColumnType( nCol+1 );
                }

                //  collections for column entries
                pImpl->ppStrings = new TypedStrCollection*[pImpl->nColCount];
                for (long i=0; i<pImpl->nColCount; i++)
                    pImpl->ppStrings[i] = NULL;

                //! init entries on demand!
                InitAllColumnEntries();             //! configurable ???

                bSuccess = TRUE;
            }
        }
    }
    catch ( sdbc::SQLException& rError )
    {
        //! store error message
        InfoBox aInfoBox( 0, String(rError.Message) );
        aInfoBox.Execute();
    }
    catch ( uno::Exception& )
    {
        DBG_ERROR("Unexpected exception in database");
    }


    if (!bSuccess)
        pImpl->xRowSet.clear();
    pImpl->bValid = bSuccess;
    return bSuccess;
}

long ScDatabaseDPData::GetColumnCount()
{
    return pImpl->nColCount;
}

void lcl_FillItemData( ScDPItemData& rData,
                        const uno::Reference<sdbc::XRow>& xRow, long nRowPos,
                        long nType, BOOL bStringForVal, ScDatabaseDPData_Impl* pImpl )
{
    //! merge with lcl_PutData in dbdocimp.cxx

    rData.aString.Erase();
    rData.fValue = 0.0;
    rData.bHasValue = FALSE;
    short nNumType = NUMBERFORMAT_NUMBER;
    BOOL bEmptyFlag = FALSE;

    if (!xRow.is())
        return;

    try
    {
        switch ( nType )
        {
            case sdbc::DataType::BIT:
                nNumType = NUMBERFORMAT_LOGICAL;
                rData.fValue = (xRow->getBoolean(nRowPos) ? 1 : 0);
                bEmptyFlag = ( rData.fValue == 0.0 ) && xRow->wasNull();
                rData.bHasValue = TRUE;
                break;

            case sdbc::DataType::TINYINT:
            case sdbc::DataType::SMALLINT:
            case sdbc::DataType::INTEGER:
            case sdbc::DataType::BIGINT:
            case sdbc::DataType::FLOAT:
            case sdbc::DataType::REAL:
            case sdbc::DataType::DOUBLE:
            case sdbc::DataType::NUMERIC:
            case sdbc::DataType::DECIMAL:
                //! do the conversion here?
                rData.fValue = xRow->getDouble(nRowPos);
                bEmptyFlag = ( rData.fValue == 0.0 ) && xRow->wasNull();
                rData.bHasValue = TRUE;
                break;

            case sdbc::DataType::CHAR:
            case sdbc::DataType::VARCHAR:
            case sdbc::DataType::LONGVARCHAR:
                rData.aString = xRow->getString(nRowPos);
                bEmptyFlag = ( rData.aString.Len() == 0 ) && xRow->wasNull();
                break;

            case sdbc::DataType::DATE:
                {
                    nNumType = NUMBERFORMAT_DATE;

                    if (!pImpl->pFormatter)
                        pImpl->pFormatter = new SvNumberFormatter( ScGlobal::eLnge );

                    util::Date aDate = xRow->getDate(nRowPos);
                    rData.fValue = Date( aDate.Day, aDate.Month, aDate.Year ) -
                                                *pImpl->pFormatter->GetNullDate();
                    bEmptyFlag = xRow->wasNull();
                    rData.bHasValue = TRUE;
                }
                break;

            //! case sdbc::DataType::TIME:
            //! case sdbc::DataType::TIMESTAMP:

            case sdbc::DataType::SQLNULL:
                //bEmptyFlag = TRUE;
                break;

            case sdbc::DataType::BINARY:
            case sdbc::DataType::VARBINARY:
            case sdbc::DataType::LONGVARBINARY:
            default:
                //bError = TRUE;        // unknown type
                break;
        }
    }
    catch ( uno::Exception& )
    {
        //bError = TRUE;
    }

    if ( bEmptyFlag )
        rData.bHasValue = FALSE;        // empty -> empty string

    if ( rData.bHasValue && bStringForVal )
    {
        if (!pImpl->pFormatter)
            pImpl->pFormatter = new SvNumberFormatter( ScGlobal::eLnge );

        ULONG nIndex = pImpl->pFormatter->GetStandardFormat( NUMBERFORMAT_DATE, ScGlobal::eLnge );
        pImpl->pFormatter->GetInputLineString( rData.fValue, nIndex, rData.aString );
    }
}

void lcl_Reset( const uno::Reference<sdbc::XRowSet>& xRowSet )
                    throw(sdbc::SQLException, uno::RuntimeException)
{
    //  isBeforeFirst / beforeFirst is not always available
    //! query if it is allowed

    xRowSet->execute();     // restart
}

void ScDatabaseDPData::InitAllColumnEntries()
{
    DBG_ASSERT( pImpl->ppStrings, "GetColumnEntries: no entries" );
    long nCol;
    for ( nCol=0; nCol<pImpl->nColCount; nCol++ )
        if (!pImpl->ppStrings[nCol])
            pImpl->ppStrings[nCol] = new TypedStrCollection;

//  Sound::Beep();      //! Test !!!

    uno::Reference<sdbc::XRowSet> xRowSet = pImpl->xRowSet;
    uno::Reference<sdbc::XRow> xRow( xRowSet, uno::UNO_QUERY );
    if ( xRow.is() )
    {
        ScDPItemData aItemData;

        try
        {
            if ( !pImpl->bAtStart )
                lcl_Reset( xRowSet );

            pImpl->bAtStart = FALSE;
            while ( xRowSet->next() )
            {
                for ( nCol=0; nCol<pImpl->nColCount; nCol++ )
                {
                    //! get string for value data only if value hasn't been inserted yet
                    lcl_FillItemData( aItemData, xRow, nCol+1, pImpl->pTypes[nCol], TRUE, pImpl );
                    TypedStrData* pNew = new TypedStrData(
                            aItemData.aString, aItemData.fValue,
                            aItemData.bHasValue ? SC_STRTYPE_VALUE : SC_STRTYPE_STANDARD );
                    if (!pImpl->ppStrings[nCol]->Insert(pNew))
                        delete pNew;
                }
            }
        }
        catch ( sdbc::SQLException& rError )
        {
            //! store error message
            InfoBox aInfoBox( 0, String(rError.Message) );
            aInfoBox.Execute();
        }
        catch ( uno::Exception& )
        {
            DBG_ERROR("Unexpected exception in database");
        }
    }
}

const TypedStrCollection& ScDatabaseDPData::GetColumnEntries(long nColumn)
{
    DBG_ASSERT( pImpl->ppStrings && nColumn < pImpl->nColCount, "GetColumnEntries: no entries" );
    if (!pImpl->ppStrings[nColumn])
    {
        TypedStrCollection* pColl = new TypedStrCollection;

        //! select distinct values directly from database

//      Sound::Beep();      //! Test !!!

        uno::Reference<sdbc::XRowSet> xRowSet = pImpl->xRowSet;
        uno::Reference<sdbc::XRow> xRow( xRowSet, uno::UNO_QUERY );
        if ( xRow.is() )
        {
            ScDPItemData aItemData;

            try
            {
                if ( !pImpl->bAtStart )
                    lcl_Reset( xRowSet );

                pImpl->bAtStart = FALSE;
                while ( xRowSet->next() )
                {
                    //! get string for value data only if value hasn't been inserted yet
                    lcl_FillItemData( aItemData, xRow, nColumn+1, pImpl->pTypes[nColumn], TRUE, pImpl );
                    TypedStrData* pNew = new TypedStrData(
                            aItemData.aString, aItemData.fValue,
                            aItemData.bHasValue ? SC_STRTYPE_VALUE : SC_STRTYPE_STANDARD );
                    if (!pColl->Insert(pNew))
                        delete pNew;
                }
            }
            catch ( sdbc::SQLException& rError )
            {
                //! store error message
                InfoBox aInfoBox( 0, String(rError.Message) );
                aInfoBox.Execute();
            }
            catch ( uno::Exception& )
            {
                DBG_ERROR("Unexpected exception in database");
            }
        }

        pImpl->ppStrings[nColumn] = pColl;
    }
    return *pImpl->ppStrings[nColumn];
}

String ScDatabaseDPData::getDimensionName(long nColumn)
{
    if (getIsDataLayoutDimension(nColumn))
    {
        //! different internal and display names?
        //return "Data";
        return ScGlobal::GetRscString(STR_PIVOT_DATA);
    }
    else if ( pImpl->pTitles && nColumn < pImpl->nColCount )
    {
        return pImpl->pTitles[nColumn];
    }
    DBG_ERROR("getDimensionName: invalid dimension");
    return String();
}

BOOL ScDatabaseDPData::getIsDataLayoutDimension(long nColumn)
{
    return ( nColumn == pImpl->nColCount );
}

BOOL ScDatabaseDPData::IsDateDimension(long nDim)
{
    //! later...
    return FALSE;
}

void ScDatabaseDPData::SetEmptyFlags( BOOL bIgnoreEmptyRows, BOOL bRepeatIfEmpty )
{
    //  not used for database data
    //! disable flags
}

void ScDatabaseDPData::ResetIterator()
{
    try
    {
        uno::Reference<sdbc::XRowSet> xRowSet = pImpl->xRowSet;
        if ( xRowSet.is() && !pImpl->bAtStart )
        {
            lcl_Reset( xRowSet );
            pImpl->bAtStart = TRUE;
        }
    }
    catch ( sdbc::SQLException& rError )
    {
        //! store error message
        InfoBox aInfoBox( 0, String(rError.Message) );
        aInfoBox.Execute();
    }
    catch ( uno::Exception& )
    {
        DBG_ERROR("Unexpected exception in database");
    }

//  Sound::Beep();      //! Test !!!
}

BOOL ScDatabaseDPData::GetNextRow( const ScDPTableIteratorParam& rParam )
{
    BOOL bSuccess = FALSE;
    uno::Reference<sdbc::XRowSet> xRowSet = pImpl->xRowSet;
    uno::Reference<sdbc::XRow> xRow( xRowSet, uno::UNO_QUERY );

    try
    {
        pImpl->bAtStart = FALSE;
        if ( xRow.is() && xRowSet->next() )
        {
            long i;

            for (i=0; i<rParam.nColCount; i++)
            {
                long nDim = rParam.pCols[i];
                if ( getIsDataLayoutDimension(nDim) )
                    rParam.pColData[i].SetString( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("x")) );
                else
                    lcl_FillItemData( rParam.pColData[i], xRow, nDim+1, pImpl->pTypes[nDim], FALSE, pImpl );
            }

            for (i=0; i<rParam.nRowCount; i++)
            {
                long nDim = rParam.pRows[i];
                if ( getIsDataLayoutDimension(nDim) )
                    rParam.pRowData[i].SetString( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("x")) );
                else
                    lcl_FillItemData( rParam.pRowData[i], xRow, nDim+1, pImpl->pTypes[nDim], FALSE, pImpl );
            }

            for (i=0; i<rParam.nDatCount; i++)
            {
                //! merge this with lcl_FillItemData, distinguish all SC_VALTYPE_... types

                long nDim = rParam.pDats[i];

                double fVal = 0.0;
                BYTE nType = SC_VALTYPE_EMPTY;
                try
                {
                    fVal = xRow->getDouble( nDim+1 );
                }
                catch ( uno::Exception& )
                {
                    //  anything that can't be converted to a number is a string
                    nType = SC_VALTYPE_STRING;
                }
                rParam.pValues[i].Set( fVal, SC_VALTYPE_VALUE );
            }

            bSuccess = TRUE;
        }
    }
    catch ( sdbc::SQLException& rError )
    {
        //! store error message
        InfoBox aInfoBox( 0, String(rError.Message) );
        aInfoBox.Execute();
    }
    catch ( uno::Exception& )
    {
        DBG_ERROR("Unexpected exception in database");
    }

    return bSuccess;
}

// -----------------------------------------------------------------------





