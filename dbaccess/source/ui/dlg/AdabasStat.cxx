/*************************************************************************
 *
 *  $RCSfile: AdabasStat.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-04-03 14:15:17 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBAUI_ADASTAT_HXX_
#include "AdabasStat.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XSTATEMENT_HPP_
#include <com/sun/star/sdbc/XStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef DBAUI_ADABASSTAT_HRC
#include "AdabasStat.hrc"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif

using namespace dbaui;
DBG_NAME(OAdabasStatistics);
namespace dbaui
{
    using namespace dbtools;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;



OAdabasStatistics::OAdabasStatistics( Window* pParent,
                                     const ::rtl::OUString& _rUser,
                                     const Reference< ::com::sun::star::sdbc::XConnection >& _xCurrentConnection,
                                     const Reference< XMultiServiceFactory >& _xFactory)
    : ModalDialog( pParent, ModuleRes(DLG_ADABASSTAT) )
    ,m_GB_FILES(            this , ResId(GB_FILES))
    ,m_FT_SYSDEVSPACE(      this , ResId(FT_SYSDEVSPACE))
    ,m_ET_SYSDEVSPACE(      this , STR_ADABAS_HELP_SYSDEVSPACE,ResId(ET_SYSDEVSPACE))
    ,m_FT_TRANSACTIONLOG(   this , ResId(FT_TRANSACTIONLOG))
    ,m_ET_TRANSACTIONLOG(   this , STR_ADABAS_HELP_TRANSACT,ResId(ET_TRANSACTIONLOG))
    ,m_FT_DATADEVSPACE(     this , ResId(FT_DATADEVSPACE))
    ,m_LB_DATADEVS(         this , STR_ADABAS_HELP_DATADEVSPACES,ResId(LB_DATADEVS))
    ,m_GB_SIZES(            this , ResId(GB_SIZES))
    ,m_FT_SIZE(             this , ResId(FT_SIZE))
    ,m_ET_SIZE(             this , STR_ADABAS_HELP_SIZE,ResId(ET_SIZE))
    ,m_FT_FREESIZE(         this , ResId(FT_FREESIZE))
    ,m_ET_FREESIZE(         this , STR_ADABAS_HELP_FREESIZE,ResId(ET_FREESIZE))
    ,m_FT_MEMORYUSING(      this , ResId(FT_MEMORYUSING))
    ,m_ET_MEMORYUSING(      this , STR_ADABAS_HELP_MEMORYUSING,ResId(ET_MEMORYUSING))
    ,m_PB_OK(               this , ResId(PB_OK))
    ,m_xConnection(_xCurrentConnection)
{
    DBG_CTOR(OAdabasStatistics,NULL);

    FreeResource();

    DBG_ASSERT(m_xConnection.is(),"No connection");
    if(!m_xConnection.is())
        return;
    Reference<XStatement> xStmt;
    Reference<XResultSet> xRes;
    try
    {
        ::rtl::OUString sTable = _rUser.toUpperCase();
        ::rtl::OUString aStmt(::rtl::OUString::createFromAscii("SELECT SERVERDBSIZE, UNUSEDPAGES FROM "));
        Reference<XPropertySet> xProp(m_xConnection,UNO_QUERY);

        aStmt += ::dbtools::quoteTableName(m_xConnection->getMetaData(),sTable);
        aStmt += ::rtl::OUString::createFromAscii(".SERVERDBSTATISTICS");

        xStmt = m_xConnection->createStatement();
        xRes = xStmt->executeQuery(aStmt);


        Reference<XRow> xRow(xRes,UNO_QUERY);
        // first the db sizes
        if(xRes.is() && xRes->next())
        {
            double nUsedPages = xRow->getDouble(1) / 256;
            double nFreePages = xRow->getDouble(2) / 256;

            m_ET_SIZE.SetText(::rtl::OUString::valueOf((INT32)nUsedPages));
            m_ET_FREESIZE.SetText(::rtl::OUString::valueOf((INT32)nFreePages));
            m_ET_MEMORYUSING.SetValue(((nUsedPages-nFreePages)/nUsedPages)*100);
        }

        xRow = NULL;
        ::comphelper::disposeComponent(xRes);

        // then the db files
        aStmt = ::rtl::OUString::createFromAscii("SELECT DEVSPACENAME FROM ");
        aStmt += ::dbtools::quoteTableName(m_xConnection->getMetaData(),sTable);
        aStmt += ::rtl::OUString::createFromAscii(".DATADEVSPACES");
        xRes = xStmt->executeQuery(aStmt);

        xRow = Reference<XRow>(xRes,UNO_QUERY);
        while(xRes.is() && xRes->next())
        {
            m_LB_DATADEVS.InsertEntry(xRow->getString(1));
        }
        xRow = NULL;
        ::comphelper::disposeComponent(xRes);

        aStmt = ::rtl::OUString::createFromAscii("SELECT * FROM ");
        aStmt += ::dbtools::quoteTableName(m_xConnection->getMetaData(),sTable);
        aStmt += ::rtl::OUString::createFromAscii(".CONFIGURATION WHERE DESCRIPTION LIKE 'SYS%DEVSPACE%NAME'");
        xRes = xStmt->executeQuery(aStmt);
        if(xRes.is() && xRes->next())
        {
            xRow = Reference<XRow>(xRes,UNO_QUERY);
            m_ET_SYSDEVSPACE.SetText(xRow->getString(2));
        }
        xRow = NULL;
        ::comphelper::disposeComponent(xRes);


        aStmt = ::rtl::OUString::createFromAscii("SELECT * FROM ");
        aStmt += ::dbtools::quoteTableName(m_xConnection->getMetaData(),sTable);
        aStmt += ::rtl::OUString::createFromAscii(".CONFIGURATION WHERE DESCRIPTION = 'TRANSACTION LOG NAME'");
        xRes = xStmt->executeQuery(aStmt);
        if(xRes.is() && xRes->next())
        {
            xRow = Reference<XRow>(xRes,UNO_QUERY);
            m_ET_TRANSACTIONLOG.SetText(xRow->getString(2));
        }
        xRow = NULL;
        ::comphelper::disposeComponent(xRes);
        ::comphelper::disposeComponent(xStmt);
    }
    catch(SQLException& e)
    {
        ::comphelper::disposeComponent(xRes);
        ::comphelper::disposeComponent(xStmt);
        showError(SQLExceptionInfo(e),pParent,_xFactory);
    }
    catch(Exception&)
    {
        ::comphelper::disposeComponent(xRes);
        ::comphelper::disposeComponent(xStmt);
        OSL_ENSURE(sal_False, "OAdabasStatistics::OAdabasStatistics: caught an exception!");
    }
    m_ET_SYSDEVSPACE.SetSpecialReadOnly(sal_True);
    m_ET_TRANSACTIONLOG.SetSpecialReadOnly(sal_True);
    m_LB_DATADEVS.SetSpecialReadOnly(sal_True);
    m_ET_SIZE.SetSpecialReadOnly(sal_True);
    m_ET_FREESIZE.SetSpecialReadOnly(sal_True);
    m_ET_MEMORYUSING.SetSpecialReadOnly(sal_True);
}
//------------------------------------------------------------------------
OAdabasStatistics::~OAdabasStatistics()
{
    DBG_DTOR(OAdabasStatistics,NULL);
}
// -----------------------------------------------------------------------------
}

