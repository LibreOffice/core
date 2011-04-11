/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
#include "precompiled_sw.hxx"


#include <osl/diagnose.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <fldmgr.hxx>
#include <dbmgr.hxx>
#include <wrtsh.hxx>        // active window
#include <view.hxx>
#include <swmodule.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;


// ---------------------------------------------------------------------------
// This file contains all routines of the fldui directory, which must compile
// with exceptions. So we can reduce the code of the other files, which don't
// need any exception handling.
// ---------------------------------------------------------------------------

/*--------------------------------------------------------------------
     Description: Is the database field numeric?
     remark: in case of error sal_True is returned
 --------------------------------------------------------------------*/

sal_Bool SwFldMgr::IsDBNumeric( const String& rDBName, const String& rTblQryName,
                            sal_Bool bIsTable, const String& rFldName)
{
    sal_Bool bNumeric = sal_True;

    SwNewDBMgr* pDBMgr = pWrtShell ? pWrtShell->GetNewDBMgr() :
                            ::GetActiveView()->GetWrtShell().GetNewDBMgr();

    ::rtl::OUString sSource(rDBName);
    Reference< XConnection> xConnection =
                    pDBMgr->RegisterConnection(sSource);

    if( !xConnection.is() )
        return bNumeric;

    Reference<XColumnsSupplier> xColsSupplier;
    if(bIsTable)
    {
        Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
        if(xTSupplier.is())
        {
            Reference<XNameAccess> xTbls = xTSupplier->getTables();
            OSL_ENSURE(xTbls->hasByName(rTblQryName), "table not available anymore?");
            try
            {
                Any aTable = xTbls->getByName(rTblQryName);
                Reference<XPropertySet> xPropSet;
                aTable >>= xPropSet;
                xColsSupplier = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
            }
            catch(Exception&){}
        }
    }
    else
    {
        Reference<XQueriesSupplier> xQSupplier = Reference<XQueriesSupplier>(xConnection, UNO_QUERY);
        if(xQSupplier.is())
        {
            Reference<XNameAccess> xQueries = xQSupplier->getQueries();
            OSL_ENSURE(xQueries->hasByName(rTblQryName), "table not available anymore?");
            try
            {
                Any aQuery = xQueries->getByName(rTblQryName);
                Reference<XPropertySet> xPropSet;
                aQuery >>= xPropSet;
                xColsSupplier = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
            }
            catch(Exception&){}
        }
    }

    if(xColsSupplier.is())
    {
        Reference <XNameAccess> xCols;
        try
        {
            xCols = xColsSupplier->getColumns();
        }
        catch(Exception& )
        {
            OSL_FAIL("Exception in getColumns()");
        }
        if(xCols.is() && xCols->hasByName(rFldName))
        {
            Any aCol = xCols->getByName(rFldName);
            Reference <XPropertySet> xCol;
            aCol >>= xCol;
            Any aType = xCol->getPropertyValue( UniString::CreateFromAscii("Type"));
            sal_Int32 eDataType = 0;
            aType >>= eDataType;
            switch(eDataType)
            {
                case DataType::BIT:
                case DataType::BOOLEAN:
                case DataType::TINYINT:
                case DataType::SMALLINT:
                case DataType::INTEGER:
                case DataType::BIGINT:
                case DataType::FLOAT:
                case DataType::REAL:
                case DataType::DOUBLE:
                case DataType::NUMERIC:
                case DataType::DECIMAL:
                case DataType::DATE:
                case DataType::TIME:
                case DataType::TIMESTAMP:
                    break;

                case DataType::BINARY:
                case DataType::VARBINARY:
                case DataType::LONGVARBINARY:
                case DataType::SQLNULL:
                case DataType::OTHER:
                case DataType::OBJECT:
                case DataType::DISTINCT:
                case DataType::STRUCT:
                case DataType::ARRAY:
                case DataType::BLOB:
                case DataType::CLOB:
                case DataType::REF:
                case DataType::CHAR:
                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                default:
                    bNumeric = sal_False;
            }
        }
    }
    return bNumeric;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
