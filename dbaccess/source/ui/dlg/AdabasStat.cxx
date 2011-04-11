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
#include "precompiled_dbaccess.hxx"

#include <cppuhelper/exc_hlp.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include "AdabasStat.hxx"
#include <comphelper/types.hxx>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "AdabasStat.hrc"
#include "dbu_dlg.hrc"
#include "dbustrings.hrc"
#include "UITools.hxx"
#include <connectivity/dbtools.hxx>
#include "sqlmessage.hxx"

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

    OSL_ENSURE(m_xConnection.is(),"No connection");
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
            bCanSelect = checkSystemTable(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SERVERDBSTATISTICS")), sSchema);

            if(bCanSelect)
            {
                aStmt = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT SERVERDBSIZE, UNUSEDPAGES FROM "));

                aStmt += ::dbtools::quoteTableName(xMetaData,sSchema,::dbtools::eInDataManipulation);
                aStmt += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".\"SERVERDBSTATISTICS\""));

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
                bCanSelect = checkSystemTable(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DATADEVSPACES")), sSchema);

                if(bCanSelect)
                {
                    // then the db files
                    aStmt = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT DEVSPACENAME FROM "));
                    aStmt += ::dbtools::quoteTableName(xMetaData,sSchema,::dbtools::eInDataManipulation);
                    aStmt += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".\"DATADEVSPACES\""));
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
                    bCanSelect = checkSystemTable(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CONFIGURATION")), sSchema);

                    if(bCanSelect)
                    {
                        aStmt = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT * FROM "));
                        aStmt += ::dbtools::quoteTableName(xMetaData,sSchema,::dbtools::eInDataManipulation);
                        aStmt += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".CONFIGURATION WHERE DESCRIPTION LIKE 'SYS%DEVSPACE%NAME'"));
                        xStmt = m_xConnection->createStatement();
                        xRes = xStmt->executeQuery(aStmt);
                        if(xRes.is() && xRes->next())
                        {
                            Reference<XRow> xRow(xRes,UNO_QUERY);
                            m_ET_SYSDEVSPACE.SetText(xRow->getString(2));
                        }
                        else
                            showError();

                        aStmt = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT * FROM "));
                        aStmt += ::dbtools::quoteTableName(xMetaData,sSchema,::dbtools::eInDataManipulation);
                        aStmt += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".CONFIGURATION WHERE DESCRIPTION = 'TRANSACTION LOG NAME'"));
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
        Reference<XResultSet> xRes = xMeta->getTablePrivileges(Any(), ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")), _rsSystemTable);
        if(xRes.is())
        {
            Reference<XRow> xRow(xRes,UNO_QUERY);
            static const ::rtl::OUString sSelect(RTL_CONSTASCII_USTRINGPARAM("SELECT"));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
