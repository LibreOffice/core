/*************************************************************************
 *
 *  $RCSfile: xfldui.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:29:06 $
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


#pragma hdrstop

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>        // Actives Fenster
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif


using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::beans;


// ---------------------------------------------------------------------------
// This file contains all routines of the fldui directory, which must compile
// with exceptions. So we can reduce the code of the other files, which don't
// need any exception handling.
// ---------------------------------------------------------------------------

/*--------------------------------------------------------------------
     Beschreibung: Ist das Datenbankfeld numerisch?
     Anm: Im Fehlerfall wird TRUE returnt.
 --------------------------------------------------------------------*/

BOOL SwFldMgr::IsDBNumeric( const String& rDBName, const String& rTblQryName,
                            BOOL bIsTable, const String& rFldName)
{
    BOOL bNumeric = TRUE;

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
            DBG_ASSERT(xTbls->hasByName(rTblQryName), "table not available anymore?")
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
            DBG_ASSERT(xQueries->hasByName(rTblQryName), "table not available anymore?")
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
            DBG_ERROR("Exception in getColumns()")
        }
        if(xCols.is() && xCols->hasByName(rFldName))
        {
            Any aCol = xCols->getByName(rFldName);
            Reference <XPropertySet> xCol;
            aCol >>= xCol;
            Any aType = xCol->getPropertyValue( UniString::CreateFromAscii("Type"));
            sal_Int32 eDataType;
            aType >>= eDataType;
            switch(eDataType)
            {
                case DataType::BIT:
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
                    bNumeric = FALSE;
            }
        }
    }
    return bNumeric;
}


