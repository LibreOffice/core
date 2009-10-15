/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dpsdbtab.cxx,v $
 * $Revision: 1.15 $
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



// INCLUDE --------------------------------------------------------------

#include <tools/debug.hxx>
#include <vcl/msgbox.hxx>
#include <svl/zforlist.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>

#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

#include "dpsdbtab.hxx"
#include "collect.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "dpcachetable.hxx"
#include "dptabres.hxx"
#include "document.hxx"
#include "dpobject.hxx"

using namespace com::sun::star;

using ::std::vector;
using ::std::hash_map;
using ::std::hash_set;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;

#define SC_SERVICE_ROWSET           "com.sun.star.sdb.RowSet"
#define SC_SERVICE_INTHANDLER       "com.sun.star.sdb.InteractionHandler"

//! move to a header file?
#define SC_DBPROP_DATASOURCENAME    "DataSourceName"
#define SC_DBPROP_COMMAND           "Command"
#define SC_DBPROP_COMMANDTYPE       "CommandType"

// -----------------------------------------------------------------------

class ScDatabaseDPData_Impl
{
public:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager;
    ScImportSourceDesc  aDesc;
    long                nColCount;
    uno::Reference<sdbc::XRowSet> xRowSet;
    sal_Int32*          pTypes;
    SvNumberFormatter*  pFormatter;

    ScDPCacheTable      aCacheTable;

    ScDatabaseDPData_Impl(ScDPCollection* p) :
        aCacheTable(p)
    {
    }
};

// -----------------------------------------------------------------------

ScDatabaseDPData::ScDatabaseDPData(
    ScDocument* pDoc,
    const ScImportSourceDesc& rImport ) :
    ScDPTableData(pDoc)
{
    pImpl = new ScDatabaseDPData_Impl(pDoc->GetDPCollection());
    pImpl->xServiceManager = pDoc->GetServiceManager();
    pImpl->aDesc = rImport;
    pImpl->nColCount = 0;
    pImpl->pTypes = NULL;
    pImpl->pFormatter = NULL;       // created on demand

    OpenDatabase();
    CreateCacheTable();
}

ScDatabaseDPData::~ScDatabaseDPData()
{
    ::comphelper::disposeComponent( pImpl->xRowSet );

    delete[] pImpl->pTypes;
    delete pImpl->pFormatter;       // NumberFormatter is local for this object
    delete pImpl;
}

void ScDatabaseDPData::DisposeData()
{
    //! use OpenDatabase here?
    pImpl->aCacheTable.clear();
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
                comphelper::getProcessServiceFactory()->createInstance(
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

            uno::Reference<sdb::XCompletedExecution> xExecute( pImpl->xRowSet, uno::UNO_QUERY );
            if ( xExecute.is() )
            {
                uno::Reference<task::XInteractionHandler> xHandler(
                        comphelper::getProcessServiceFactory()->createInstance(
                            rtl::OUString::createFromAscii( SC_SERVICE_INTHANDLER ) ),
                        uno::UNO_QUERY);
                xExecute->executeWithCompletion( xHandler );
            }
            else
                pImpl->xRowSet->execute();

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
                pImpl->pTypes = new sal_Int32[pImpl->nColCount];
                for (long nCol=0; nCol<pImpl->nColCount; nCol++)
                    pImpl->pTypes[nCol]  = xMeta->getColumnType( nCol+1 );

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
        ::comphelper::disposeComponent( pImpl->xRowSet );

    return bSuccess;
}

long ScDatabaseDPData::GetColumnCount()
{
    return pImpl->nColCount;
}

const TypedScStrCollection& ScDatabaseDPData::GetColumnEntries(long nColumn)
{
    CreateCacheTable();
    return pImpl->aCacheTable.getFieldEntries(nColumn);
}

String ScDatabaseDPData::getDimensionName(long nColumn)
{
    if (getIsDataLayoutDimension(nColumn))
    {
        //! different internal and display names?
        //return "Data";
        return ScGlobal::GetRscString(STR_PIVOT_DATA);
    }

    CreateCacheTable();
    const String* pStr = pImpl->aCacheTable.getFieldName(nColumn);
    if (pStr)
        return *pStr;

    DBG_ERROR("getDimensionName: invalid dimension");
    return String();
}

BOOL ScDatabaseDPData::getIsDataLayoutDimension(long nColumn)
{
    return ( nColumn == pImpl->nColCount );
}

BOOL ScDatabaseDPData::IsDateDimension(long /* nDim */)
{
    //! later...
    return FALSE;
}

void ScDatabaseDPData::SetEmptyFlags( BOOL /* bIgnoreEmptyRows */, BOOL /* bRepeatIfEmpty */ )
{
    //  not used for database data
    //! disable flags
}

void ScDatabaseDPData::CreateCacheTable()
{
    if (!pImpl->aCacheTable.empty())
        return;

    // Get null date.
    if (!pImpl->pFormatter)
        pImpl->pFormatter = new SvNumberFormatter(pImpl->xServiceManager, ScGlobal::eLnge);

    pImpl->aCacheTable.fillTable(pImpl->xRowSet, *pImpl->pFormatter->GetNullDate());
}

void ScDatabaseDPData::FilterCacheTable(const vector<ScDPCacheTable::Criterion>& rCriteria, const hash_set<sal_Int32>& rCatDims)
{
    CreateCacheTable();
    pImpl->aCacheTable.filterByPageDimension(
        rCriteria, (IsRepeatIfEmpty() ? rCatDims : hash_set<sal_Int32>()));
}

void ScDatabaseDPData::GetDrillDownData(const vector<ScDPCacheTable::Criterion>& rCriteria, const hash_set<sal_Int32>& rCatDims, Sequence< Sequence<Any> >& rData)
{
    CreateCacheTable();
    sal_Int32 nRowSize = pImpl->aCacheTable.getRowSize();
    if (!nRowSize)
        return;

    pImpl->aCacheTable.filterTable(
        rCriteria, rData, IsRepeatIfEmpty() ? rCatDims : hash_set<sal_Int32>());
}

void ScDatabaseDPData::CalcResults(CalcInfo& rInfo, bool bAutoShow)
{
    CreateCacheTable();
    CalcResultsFromCacheTable(pImpl->aCacheTable, rInfo, bAutoShow);
}

const ScDPCacheTable& ScDatabaseDPData::GetCacheTable() const
{
    return pImpl->aCacheTable;
}

// -----------------------------------------------------------------------





