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
#include "precompiled_dbaccess.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _DBAUI_ADASTAT_HXX_
#include "AdabasStat.hxx"
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
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
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
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif

using namespace dbaui;
DBG_NAME(OAdabasStatistics)
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
    ,m_FL_FILES(            this , ModuleRes(FL_FILES))
    ,m_FT_SYSDEVSPACE(      this , ModuleRes(FT_SYSDEVSPACE))
    ,m_ET_SYSDEVSPACE(      this , STR_ADABAS_HELP_SYSDEVSPACE,ModuleRes(ET_SYSDEVSPACE))
    ,m_FT_TRANSACTIONLOG(   this , ModuleRes(FT_TRANSACTIONLOG))
    ,m_ET_TRANSACTIONLOG(   this , STR_ADABAS_HELP_TRANSACT,ModuleRes(ET_TRANSACTIONLOG))
    ,m_FT_DATADEVSPACE(     this , ModuleRes(FT_DATADEVSPACE))
    ,m_LB_DATADEVS(         this , STR_ADABAS_HELP_DATADEVSPACES,ModuleRes(LB_DATADEVS))
    ,m_FL_SIZES(            this , ModuleRes(FL_SIZES))
    ,m_FT_SIZE(             this , ModuleRes(FT_SIZE))
    ,m_ET_SIZE(             this , STR_ADABAS_HELP_SIZE,ModuleRes(ET_SIZE))
    ,m_FT_FREESIZE(         this , ModuleRes(FT_FREESIZE))
    ,m_ET_FREESIZE(         this , STR_ADABAS_HELP_FREESIZE,ModuleRes(ET_FREESIZE))
    ,m_FT_MEMORYUSING(      this , ModuleRes(FT_MEMORYUSING))
    ,m_ET_MEMORYUSING(      this , STR_ADABAS_HELP_MEMORYUSING,ModuleRes(ET_MEMORYUSING))
    ,m_PB_OK(               this , ModuleRes(PB_OK))
    ,m_xConnection(_xCurrentConnection)
    ,m_bErrorShown(sal_False)
{
    DBG_CTOR(OAdabasStatistics,NULL);

    FreeResource();

    DBG_ASSERT(m_xConnection.is(),"No connection");
    if(m_xConnection.is())
    {
        Reference<XStatement> xStmt;
        Reference<XResultSet> xRes;

        sal_Bool bCanSelect     = sal_False;
        ::rtl::OUString aStmt;
        ::rtl::OUString sSchema = _rUser.toAsciiUpperCase();

        Reference<XDatabaseMetaData> xMetaData;
        // first read the sizes
        try
        {
            xMetaData = m_xConnection->getMetaData();
            bCanSelect = checkSystemTable(::rtl::OUString::createFromAscii("SERVERDBSTATISTICS"),sSchema);

            if(bCanSelect)
            {
                aStmt = ::rtl::OUString::createFromAscii("SELECT SERVERDBSIZE, UNUSEDPAGES FROM ");

                aStmt += ::dbtools::quoteTableName(xMetaData,sSchema,::dbtools::eInDataManipulation);
                aStmt += ::rtl::OUString::createFromAscii(".\"SERVERDBSTATISTICS\"");

                xStmt = m_xConnection->createStatement();
                xRes = xStmt->executeQuery(aStmt);


                Reference<XRow> xRow(xRes,UNO_QUERY);
                // first the db sizes
                if(xRes.is() && xRes->next())
                {
                    double nUsedPages = xRow->getInt(1) / 256;
                    double nFreePages = xRow->getInt(2) / 256;

                    m_ET_SIZE.SetText(::rtl::OUString::valueOf((sal_Int32)nUsedPages));
                    m_ET_FREESIZE.SetText(::rtl::OUString::valueOf((sal_Int32)nFreePages));
                    m_ET_MEMORYUSING.SetValue(static_cast<sal_Int32>(((nUsedPages-nFreePages)/nUsedPages)*100));
                }
                else
                    showError();

                xRow = NULL;
            }
            else
                showError();
        }
        catch(const SQLException& )
        {
            ::dbaui::showError( SQLExceptionInfo( ::cppu::getCaughtException() ), pParent, _xFactory );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        try
        {
            ::comphelper::disposeComponent(xStmt);
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        // now fill the datadev spaces
        if(bCanSelect)
        {
            try
            {
                bCanSelect = checkSystemTable(::rtl::OUString::createFromAscii("DATADEVSPACES"),sSchema);

                if(bCanSelect)
                {
                    // then the db files
                    aStmt = ::rtl::OUString::createFromAscii("SELECT DEVSPACENAME FROM ");
                    aStmt += ::dbtools::quoteTableName(xMetaData,sSchema,::dbtools::eInDataManipulation);
                    aStmt += ::rtl::OUString::createFromAscii(".\"DATADEVSPACES\"");
                    xStmt = m_xConnection->createStatement();
                    xRes = xStmt->executeQuery(aStmt);

                    Reference<XRow> xRow(xRes,UNO_QUERY);
                    while(xRes.is() && xRes->next())
                    {
                        m_LB_DATADEVS.InsertEntry(xRow->getString(1));
                    }
                    if(!m_LB_DATADEVS.GetEntryCount())
                        showError();
                }
                else
                    showError();
            }
            catch(const SQLException& e)
            {
                ::dbaui::showError(SQLExceptionInfo(e),pParent,_xFactory);
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            try
            {
                ::comphelper::disposeComponent(xStmt);
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            // now fill the sysdatadev spaces
            if(bCanSelect)
            {
                try
                {
                    bCanSelect = checkSystemTable(::rtl::OUString::createFromAscii("CONFIGURATION"),sSchema);

                    if(bCanSelect)
                    {
                        aStmt = ::rtl::OUString::createFromAscii("SELECT * FROM ");
                        aStmt += ::dbtools::quoteTableName(xMetaData,sSchema,::dbtools::eInDataManipulation);
                        aStmt += ::rtl::OUString::createFromAscii(".CONFIGURATION WHERE DESCRIPTION LIKE 'SYS%DEVSPACE%NAME'");
                        xStmt = m_xConnection->createStatement();
                        xRes = xStmt->executeQuery(aStmt);
                        if(xRes.is() && xRes->next())
                        {
                            Reference<XRow> xRow(xRes,UNO_QUERY);
                            m_ET_SYSDEVSPACE.SetText(xRow->getString(2));
                        }
                        else
                            showError();

                        aStmt = ::rtl::OUString::createFromAscii("SELECT * FROM ");
                        aStmt += ::dbtools::quoteTableName(xMetaData,sSchema,::dbtools::eInDataManipulation);
                        aStmt += ::rtl::OUString::createFromAscii(".CONFIGURATION WHERE DESCRIPTION = 'TRANSACTION LOG NAME'");
                        xRes = xStmt->executeQuery(aStmt);
                        if(xRes.is() && xRes->next())
                        {
                            Reference<XRow> xRow(xRes,UNO_QUERY);
                            m_ET_TRANSACTIONLOG.SetText(xRow->getString(2));
                        }
                        else
                            showError();
                    }
                    else
                        showError();
                }
                catch(const SQLException& e)
                {
                    ::dbaui::showError(SQLExceptionInfo(e),pParent,_xFactory);
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
                try
                {
                    ::comphelper::disposeComponent(xStmt);
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }
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
sal_Bool OAdabasStatistics::checkSystemTable(const ::rtl::OUString& _rsSystemTable, ::rtl::OUString& _rsSchemaName )
{
    sal_Bool bCanSelect = sal_False;
    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    if ( xMeta.is() )
    {
        Reference<XResultSet> xRes = xMeta->getTablePrivileges(Any(),::rtl::OUString::createFromAscii("%"),  _rsSystemTable);
        if(xRes.is())
        {
            Reference<XRow> xRow(xRes,UNO_QUERY);
            static const ::rtl::OUString sSelect = ::rtl::OUString::createFromAscii("SELECT");
            // first the db sizes
            while( xRow.is() && xRes->next() )
            {
                _rsSchemaName = xRow->getString(2);
                if(sSelect == xRow->getString(6) && !xRow->wasNull())
                {
                    bCanSelect = sal_True;
                    break;
                }
            }
            ::comphelper::disposeComponent(xRes);
        }
    }

    return bCanSelect;
}
// -----------------------------------------------------------------------------
void OAdabasStatistics::showError()
{
    if(!m_bErrorShown)
    {
        OSQLMessageBox aMsg(GetParent(),GetText(),String(ModuleRes(STR_ADABAS_ERROR_SYSTEMTABLES)));
        aMsg.Execute();
        m_bErrorShown = sal_True;
    }
}
// -----------------------------------------------------------------------------
}

