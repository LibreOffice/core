/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "directsql.hxx"
#include "dbu_dlg.hrc"
#include <vcl/layout.hxx>
#include <comphelper/types.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/strbuf.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;

    // DirectSQLDialog
    DirectSQLDialog::DirectSQLDialog( vcl::Window* _pParent, const Reference< XConnection >& _rxConn )
        :ModalDialog(_pParent, "DirectSQLDialog" , "dbaccess/ui/directsqldialog.ui")
        ,m_nHistoryLimit(20)
        ,m_nStatusCount(1)
        ,m_xConnection(_rxConn)
    {
        get(m_pSQL,"sql");
        Size aSize(m_pSQL->CalcBlockSize(60, 7));
        m_pSQL->set_width_request(aSize.Width());
        m_pSQL->set_height_request(aSize.Height());
        get(m_pExecute,"execute");
        get(m_pSQLHistory,"sqlhistory");
        get(m_pStatus,"status");
        aSize  = m_pStatus->CalcBlockSize(60, 5);
        m_pStatus->set_height_request(aSize.Height());
        get(m_pShowOutput,"showoutput");
        get(m_pOutput,"output");
        aSize  = m_pOutput->CalcBlockSize(60, 5);
        m_pOutput->set_height_request(aSize.Height());
        get(m_pClose,"close");


        m_pSQL->GrabFocus();

        m_pExecute->SetClickHdl(LINK(this, DirectSQLDialog, OnExecute));
        m_pClose->SetClickHdl(LINK(this, DirectSQLDialog, OnCloseClick));
        m_pSQLHistory->SetSelectHdl(LINK(this, DirectSQLDialog, OnListEntrySelected));
        m_pSQLHistory->SetDropDownLineCount(10);

        // add a dispose listener to the connection
        Reference< XComponent > xConnComp(m_xConnection, UNO_QUERY);
        OSL_ENSURE(xConnComp.is(), "DirectSQLDialog::DirectSQLDialog: invalid connection!");
        if (xConnComp.is())
            startComponentListening(xConnComp);

        m_pSQL->SetModifyHdl(LINK(this, DirectSQLDialog, OnStatementModified));
        OnStatementModified(*m_pSQL);
    }

    DirectSQLDialog::~DirectSQLDialog()
    {
        disposeOnce();
    }

    void DirectSQLDialog::dispose()
    {
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            stopAllComponentListening();
        }
        m_pSQL.clear();
        m_pExecute.clear();
        m_pSQLHistory.clear();
        m_pStatus.clear();
        m_pShowOutput.clear();
        m_pOutput.clear();
        m_pClose.clear();
        ModalDialog::dispose();
    }

    void DirectSQLDialog::_disposing( const EventObject& _rSource )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard(m_aMutex);

        OSL_ENSURE(Reference< XConnection >(_rSource.Source, UNO_QUERY).get() == m_xConnection.get(),
            "DirectSQLDialog::_disposing: where does this come from?");
        (void)_rSource;

        {
            OUString sMessage(ModuleRes(STR_DIRECTSQL_CONNECTIONLOST));
            ScopedVclPtrInstance< MessageDialog > aError(this, sMessage);
            aError->Execute();
        }

        PostUserEvent(LINK(this, DirectSQLDialog, OnClose), nullptr, true);
    }

    sal_Int32 DirectSQLDialog::getHistorySize() const
    {
        CHECK_INVARIANTS("DirectSQLDialog::getHistorySize");
        return m_aStatementHistory.size();
    }

    void DirectSQLDialog::implEnsureHistoryLimit()
    {
        CHECK_INVARIANTS("DirectSQLDialog::implEnsureHistoryLimit");

        if (getHistorySize() <= m_nHistoryLimit)
            // nothing to do
            return;

        sal_Int32 nRemoveEntries = getHistorySize() - m_nHistoryLimit;
        while (nRemoveEntries--)
        {
            m_aStatementHistory.pop_front();
            m_aNormalizedHistory.pop_front();
            m_pSQLHistory->RemoveEntry((sal_uInt16)0);
        }
    }

    void DirectSQLDialog::implAddToStatementHistory(const OUString& _rStatement)
    {
        CHECK_INVARIANTS("DirectSQLDialog::implAddToStatementHistory");

        // add the statement to the history
        m_aStatementHistory.push_back(_rStatement);

        // normalize the statement, and remember the normalized form, too
        OUString sNormalized(_rStatement);
        sNormalized = sNormalized.replaceAll("\n", " ");
        m_aNormalizedHistory.push_back(sNormalized);

        // add the normalized version to the list box
        m_pSQLHistory->InsertEntry(sNormalized);

        // ensure that we don't exceed the history limit
        implEnsureHistoryLimit();
    }

#ifdef DBG_UTIL
    const sal_Char* DirectSQLDialog::impl_CheckInvariants() const
    {
        if (m_aStatementHistory.size() != m_aNormalizedHistory.size())
            return "statement history is inconsistent!";

        if (!m_pSQLHistory)
            return "invalid listbox!";

        if (m_aStatementHistory.size() != static_cast<size_t>(m_pSQLHistory->GetEntryCount()))
            return "invalid listbox entry count!";

        if (!m_xConnection.is())
            return "have no connection!";

        return nullptr;
    }
#endif

    void DirectSQLDialog::implExecuteStatement(const OUString& _rStatement)
    {
        CHECK_INVARIANTS("DirectSQLDialog::implExecuteStatement");

        ::osl::MutexGuard aGuard(m_aMutex);

        OUString sStatus;
        css::uno::Reference< css::sdbc::XResultSet > xResultSet;
        try
        {
            // create a statement
            Reference< XStatement > xStatement = m_xConnection->createStatement();
            OSL_ENSURE(xStatement.is(), "DirectSQLDialog::implExecuteStatement: no statement returned by the connection!");

            // clear the output box
            m_pOutput->SetText(OUString());
            if (xStatement.is())
            {
                if (OUString(_rStatement).toAsciiUpperCase().startsWith("SELECT") && m_pShowOutput->IsChecked())
                {
                    // execute it as a query
                    xResultSet = xStatement->executeQuery(_rStatement);
                    // get a handle for the rows
                    css::uno::Reference< css::sdbc::XRow > xRow( xResultSet, css::uno::UNO_QUERY );
                    // work through each of the rows
                    while (xResultSet->next())
                    {
                        // initialise the output line for each row
                        OUString out("");
                        // work along the columns until that are none left
                        try
                        {
                            int i = 1;
                            for (;;)
                            {
                                // be dumb, treat everything as a string
                                out += xRow->getString(i) + ",";
                                i++;
                            }
                        }
                        // trap for when we fall off the end of the row
                        catch (const SQLException&)
                        {
                        }
                        // report the output
                        addOutputText(OUString(out));
                    }
                } else {
                    // execute it
                    xStatement->execute(_rStatement);
                }
            }

            // successful
            sStatus = ModuleRes(STR_COMMAND_EXECUTED_SUCCESSFULLY);

            // dispose the statement
            ::comphelper::disposeComponent(xStatement);
        }
        catch(const SQLException& e)
        {
            sStatus = e.Message;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        // add the status text
        addStatusText(sStatus);
    }

    void DirectSQLDialog::addStatusText(const OUString& _rMessage)
    {
        OUString sAppendMessage = OUString::number(m_nStatusCount++) + ": " + _rMessage + "\n\n";

        OUString sCompleteMessage = m_pStatus->GetText() + sAppendMessage;
        m_pStatus->SetText(sCompleteMessage);

        m_pStatus->SetSelection(Selection(sCompleteMessage.getLength(), sCompleteMessage.getLength()));
    }

    void DirectSQLDialog::addOutputText(const OUString& _rMessage)
    {
        OUString sAppendMessage = _rMessage + "\n";

        OUString sCompleteMessage = m_pOutput->GetText() + sAppendMessage;
        m_pOutput->SetText(sCompleteMessage);
    }

    void DirectSQLDialog::executeCurrent()
    {
        CHECK_INVARIANTS("DirectSQLDialog::executeCurrent");

        OUString sStatement = m_pSQL->GetText();

        // execute
        implExecuteStatement(sStatement);

        // add the statement to the history
        implAddToStatementHistory(sStatement);

        m_pSQL->SetSelection(Selection());
        m_pSQL->GrabFocus();
    }

    void DirectSQLDialog::switchToHistory(sal_Int32 _nHistoryPos)
    {
        CHECK_INVARIANTS("DirectSQLDialog::switchToHistory");

        if ((_nHistoryPos >= 0) && (_nHistoryPos < getHistorySize()))
        {
            // set the text in the statement editor
            OUString sStatement = m_aStatementHistory[_nHistoryPos];
            m_pSQL->SetText(sStatement);
            OnStatementModified(*m_pSQL);

            m_pSQL->GrabFocus();
            m_pSQL->SetSelection(Selection(sStatement.getLength(), sStatement.getLength()));
        }
        else
            OSL_FAIL("DirectSQLDialog::switchToHistory: invalid position!");
    }

    IMPL_LINK_NOARG( DirectSQLDialog, OnStatementModified, Edit&, void )
    {
        m_pExecute->Enable(!m_pSQL->GetText().isEmpty());
    }

    IMPL_LINK_NOARG( DirectSQLDialog, OnCloseClick, Button*, void )
    {
        EndDialog( RET_OK );
    }
    IMPL_LINK_NOARG( DirectSQLDialog, OnClose, void*, void )
    {
        EndDialog( RET_OK );
    }

    IMPL_LINK_NOARG( DirectSQLDialog, OnExecute, Button*, void )
    {
        executeCurrent();
    }

    IMPL_LINK_NOARG( DirectSQLDialog, OnListEntrySelected, ListBox&, void )
    {
        if (!m_pSQLHistory->IsTravelSelect())
        {
            const sal_Int32 nSelected = m_pSQLHistory->GetSelectEntryPos();
            if (LISTBOX_ENTRY_NOTFOUND != nSelected)
                switchToHistory(nSelected);
        }
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
