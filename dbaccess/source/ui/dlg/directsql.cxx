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

#include <core_resource.hxx>
#include <directsql.hxx>
#include <sqledit.hxx>
#include <strings.hrc>
#include <comphelper/types.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;

    constexpr sal_Int32 g_nHistoryLimit = 20;

    // DirectSQLDialog
    DirectSQLDialog::DirectSQLDialog(weld::Window* _pParent, const Reference< XConnection >& _rxConn)
        : GenericDialogController(_pParent, "dbaccess/ui/directsqldialog.ui", "DirectSQLDialog")
        , m_xExecute(m_xBuilder->weld_button("execute"))
        , m_xSQLHistory(m_xBuilder->weld_combo_box("sqlhistory"))
        , m_xStatus(m_xBuilder->weld_text_view("status"))
        , m_xShowOutput(m_xBuilder->weld_check_button("showoutput"))
        , m_xOutput(m_xBuilder->weld_text_view("output"))
        , m_xClose(m_xBuilder->weld_button("close"))
        , m_xSQL(new SQLEditView)
        , m_xSQLEd(new weld::CustomWeld(*m_xBuilder, "sql", *m_xSQL))
        , m_nStatusCount(1)
        , m_xConnection(_rxConn)
        , m_pClosingEvent(nullptr)
    {
        int nWidth = m_xStatus->get_approximate_digit_width() * 60;
        int nHeight = m_xStatus->get_height_rows(7);

        m_xSQLEd->set_size_request(nWidth, nHeight);
        m_xStatus->set_size_request(-1, nHeight);
        m_xOutput->set_size_request(-1, nHeight);

        m_xSQL->GrabFocus();

        m_xExecute->connect_clicked(LINK(this, DirectSQLDialog, OnExecute));
        m_xClose->connect_clicked(LINK(this, DirectSQLDialog, OnCloseClick));
        m_xSQLHistory->connect_changed(LINK(this, DirectSQLDialog, OnListEntrySelected));

        // add a dispose listener to the connection
        Reference< XComponent > xConnComp(m_xConnection, UNO_QUERY);
        OSL_ENSURE(xConnComp.is(), "DirectSQLDialog::DirectSQLDialog: invalid connection!");
        if (xConnComp.is())
            startComponentListening(xConnComp);

        m_xSQL->SetModifyHdl(LINK(this, DirectSQLDialog, OnStatementModified));
        OnStatementModified(nullptr);
    }

    DirectSQLDialog::~DirectSQLDialog()
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if (m_pClosingEvent)
            Application::RemoveUserEvent(m_pClosingEvent);
        stopAllComponentListening();
    }

    void DirectSQLDialog::_disposing( const EventObject& _rSource )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard(m_aMutex);

        assert(!m_pClosingEvent);

        OSL_ENSURE(Reference< XConnection >(_rSource.Source, UNO_QUERY).get() == m_xConnection.get(),
            "DirectSQLDialog::_disposing: where does this come from?");

        {
            OUString sMessage(DBA_RES(STR_DIRECTSQL_CONNECTIONLOST));
            std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(m_xDialog.get(),
                                                        VclMessageType::Warning, VclButtonsType::Ok,
                                                        sMessage));
            xError->run();
        }

        m_pClosingEvent = Application::PostUserEvent(LINK(this, DirectSQLDialog, OnClose));
    }

    sal_Int32 DirectSQLDialog::getHistorySize() const
    {
        CHECK_INVARIANTS("DirectSQLDialog::getHistorySize");
        return m_aStatementHistory.size();
    }

    void DirectSQLDialog::implEnsureHistoryLimit()
    {
        CHECK_INVARIANTS("DirectSQLDialog::implEnsureHistoryLimit");

        if (getHistorySize() <= g_nHistoryLimit)
            // nothing to do
            return;

        sal_Int32 nRemoveEntries = getHistorySize() - g_nHistoryLimit;
        while (nRemoveEntries--)
        {
            m_aStatementHistory.pop_front();
            m_aNormalizedHistory.pop_front();
            m_xSQLHistory->remove(0);
        }
    }

    void DirectSQLDialog::implAddToStatementHistory(const OUString& _rStatement)
    {
        CHECK_INVARIANTS("DirectSQLDialog::implAddToStatementHistory");

        // add the statement to the history
        m_aStatementHistory.push_back(_rStatement);

        // normalize the statement, and remember the normalized form, too
        OUString sNormalized = _rStatement.replaceAll("\n", " ");
        m_aNormalizedHistory.push_back(sNormalized);

        // add the normalized version to the list box
        m_xSQLHistory->append_text(sNormalized);

        // ensure that we don't exceed the history limit
        implEnsureHistoryLimit();
    }

#ifdef DBG_UTIL
    const char* DirectSQLDialog::impl_CheckInvariants() const
    {
        if (m_aStatementHistory.size() != m_aNormalizedHistory.size())
            return "statement history is inconsistent!";

        if (!m_xSQLHistory)
            return "invalid listbox!";

        if (m_aStatementHistory.size() != static_cast<size_t>(m_xSQLHistory->get_count()))
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

        // clear the output box
        m_xOutput->set_text(OUString());
        try
        {
            // create a statement
            Reference< XStatement > xStatement = m_xConnection->createStatement();

            Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
            css::uno::Reference< css::sdbc::XMultipleResults > xMR ( xStatement, UNO_QUERY );

            if (xMeta.is() && xMeta->supportsMultipleResultSets() && xMR.is())
            {
                bool hasRS = xStatement->execute(_rStatement);
                if(hasRS)
                {
                    css::uno::Reference< css::sdbc::XResultSet > xRS (xMR->getResultSet());
                    if (m_xShowOutput->get_active())
                        display(xRS);
                }
                else
                    addOutputText(OUString::number(xMR->getUpdateCount()) + " rows updated\n");
                for (;;)
                {
                    hasRS = xMR->getMoreResults();
                    if (!hasRS && xMR->getUpdateCount() == -1)
                        break;
                    if(hasRS)
                    {
                        css::uno::Reference< css::sdbc::XResultSet > xRS (xMR->getResultSet());
                        if (m_xShowOutput->get_active())
                            display(xRS);
                    }
                }
            }
            else
            {
                if (_rStatement.toAsciiUpperCase().startsWith("SELECT"))
                {
                    css::uno::Reference< css::sdbc::XResultSet > xRS = xStatement->executeQuery(_rStatement);
                    if (m_xShowOutput->get_active())
                        display(xRS);
                }
                else
                {
                    sal_Int32 resultCount = xStatement->executeUpdate(_rStatement);
                    addOutputText(OUString::number(resultCount) + " rows updated\n");
                }
            }
            // successful
            sStatus = DBA_RES(STR_COMMAND_EXECUTED_SUCCESSFULLY);

            // dispose the statement
            ::comphelper::disposeComponent(xStatement);
        }
        catch(const SQLException& e)
        {
            sStatus = e.Message;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }

        // add the status text
        addStatusText(sStatus);
    }

    void DirectSQLDialog::display(const css::uno::Reference< css::sdbc::XResultSet >& xRS)
    {
        // get a handle for the rows
        css::uno::Reference< css::sdbc::XRow > xRow( xRS, css::uno::UNO_QUERY );
        // work through each of the rows
        while (xRS->next())
        {
            // initialise the output line for each row
            OUStringBuffer out;
            // work along the columns until that are none left
            try
            {
                int i = 1;
                for (;;)
                {
                    // be dumb, treat everything as a string
                    out.append(xRow->getString(i)).append(",");
                    i++;
                }
            }
            // trap for when we fall off the end of the row
            catch (const SQLException&)
            {
            }
            // report the output
            addOutputText(out.makeStringAndClear());
        }
    }

    void DirectSQLDialog::addStatusText(const OUString& _rMessage)
    {
        OUString sAppendMessage = OUString::number(m_nStatusCount++) + ": " + _rMessage + "\n\n";

        OUString sCompleteMessage = m_xStatus->get_text() + sAppendMessage;
        m_xStatus->set_text(sCompleteMessage);

        m_xStatus->select_region(sCompleteMessage.getLength(), sCompleteMessage.getLength());
    }

    void DirectSQLDialog::addOutputText(const OUString& _rMessage)
    {
        OUString sAppendMessage = _rMessage + "\n";

        OUString sCompleteMessage = m_xOutput->get_text() + sAppendMessage;
        m_xOutput->set_text(sCompleteMessage);
    }

    void DirectSQLDialog::executeCurrent()
    {
        CHECK_INVARIANTS("DirectSQLDialog::executeCurrent");

        OUString sStatement = m_xSQL->GetText();

        // execute
        implExecuteStatement(sStatement);

        // add the statement to the history
        implAddToStatementHistory(sStatement);

        m_xSQL->GrabFocus();
    }

    void DirectSQLDialog::switchToHistory(sal_Int32 _nHistoryPos)
    {
        CHECK_INVARIANTS("DirectSQLDialog::switchToHistory");

        if ((_nHistoryPos >= 0) && (_nHistoryPos < getHistorySize()))
        {
            // set the text in the statement editor
            OUString sStatement = m_aStatementHistory[_nHistoryPos];
            m_xSQL->SetTextAndUpdate(sStatement);
            OnStatementModified(nullptr);

            m_xSQL->GrabFocus();
        }
        else
            OSL_FAIL("DirectSQLDialog::switchToHistory: invalid position!");
    }

    IMPL_LINK_NOARG( DirectSQLDialog, OnStatementModified, LinkParamNone*, void )
    {
        m_xExecute->set_sensitive(!m_xSQL->GetText().isEmpty());
    }

    IMPL_LINK_NOARG( DirectSQLDialog, OnCloseClick, weld::Button&, void )
    {
        m_xDialog->response(RET_OK);
    }

    IMPL_LINK_NOARG( DirectSQLDialog, OnClose, void*, void )
    {
        assert(m_pClosingEvent);
        Application::RemoveUserEvent(m_pClosingEvent);
        m_pClosingEvent = nullptr;

        m_xDialog->response(RET_OK);
    }

    IMPL_LINK_NOARG( DirectSQLDialog, OnExecute, weld::Button&, void )
    {
        executeCurrent();
    }

    IMPL_LINK_NOARG( DirectSQLDialog, OnListEntrySelected, weld::ComboBox&, void )
    {
        const sal_Int32 nSelected = m_xSQLHistory->get_active();
        if (nSelected != -1)
            switchToHistory(nSelected);
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
