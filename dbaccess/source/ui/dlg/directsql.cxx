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
#include <vcl/msgbox.hxx>
#include <comphelper/types.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/strbuf.hxx>
#include <com/sun/star/sdbc/XRow.hpp>

//........................................................................
namespace dbaui
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;


    //====================================================================
    //= LargeEntryListBox
    //====================================================================
    class LargeEntryListBox : public ListBox
    {
    public:
        LargeEntryListBox( Window* _pParent, const ResId& _rId );

    protected:
        virtual void    UserDraw( const UserDrawEvent& rUDEvt );
    };

    //--------------------------------------------------------------------
    LargeEntryListBox::LargeEntryListBox( Window* _pParent, const ResId& _rId )
        :ListBox(_pParent, _rId )
    {
        EnableUserDraw(sal_True);
    }

    //--------------------------------------------------------------------
    void LargeEntryListBox::UserDraw( const UserDrawEvent& _rUDEvt )
    {
        if (LISTBOX_ENTRY_NOTFOUND == _rUDEvt.GetItemId())
            ListBox::UserDraw( _rUDEvt );
        else
            _rUDEvt.GetDevice()->DrawText( _rUDEvt.GetRect(), GetEntry( _rUDEvt.GetItemId() ), TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER | TEXT_DRAW_ENDELLIPSIS);
    }

    //====================================================================
    //= DirectSQLDialog
    //====================================================================
DBG_NAME(DirectSQLDialog)
//--------------------------------------------------------------------
    DirectSQLDialog::DirectSQLDialog( Window* _pParent, const Reference< XConnection >& _rxConn )
        :ModalDialog(_pParent, "DirectSQLDialog" , "dbaccess/ui/directsqldialog.ui")
        ,m_nHistoryLimit(20)
        ,m_nStatusCount(1)
        ,m_xConnection(_rxConn)
    {
        get(m_pSQL,"sql");
        Size aSize(m_pSQL->CalcSize(60, 7));
        m_pSQL->set_width_request(aSize.Width());
        m_pSQL->set_height_request(aSize.Height());
        get(m_pExecute,"execute");
        get(m_pSQLHistory,"sqlhistory");
        get(m_pStatus,"status");
        aSize  = m_pStatus->CalcSize(60, 5);
        m_pStatus->set_height_request(aSize.Height());
        get(m_pShowOutput,"showoutput");
        get(m_pOutput,"output");
        aSize  = m_pOutput->CalcSize(60, 5);
        m_pOutput->set_height_request(aSize.Height());
        get(m_pClose,"close");

        DBG_CTOR(DirectSQLDialog,NULL);

        m_pSQL->GrabFocus();

        m_pExecute->SetClickHdl(LINK(this, DirectSQLDialog, OnExecute));
        m_pClose->SetClickHdl(LINK(this, DirectSQLDialog, OnClose));
        m_pSQLHistory->SetSelectHdl(LINK(this, DirectSQLDialog, OnListEntrySelected));
        m_pSQLHistory->SetDropDownLineCount(10);

        // add a dispose listener to the connection
        Reference< XComponent > xConnComp(m_xConnection, UNO_QUERY);
        OSL_ENSURE(xConnComp.is(), "DirectSQLDialog::DirectSQLDialog: invalid connection!");
        if (xConnComp.is())
            startComponentListening(xConnComp);

        m_pSQL->SetModifyHdl(LINK(this, DirectSQLDialog, OnStatementModified));
        OnStatementModified(m_pSQL);
    }

    //--------------------------------------------------------------------
    DirectSQLDialog::~DirectSQLDialog()
    {
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            stopAllComponentListening();
        }

        DBG_DTOR(DirectSQLDialog,NULL);
    }

    //--------------------------------------------------------------------
    void DirectSQLDialog::_disposing( const EventObject& _rSource )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard(m_aMutex);

        OSL_ENSURE(Reference< XConnection >(_rSource.Source, UNO_QUERY).get() == m_xConnection.get(),
            "DirectSQLDialog::_disposing: where does this come from?");
        (void)_rSource;

        {
            String sMessage(ModuleRes(STR_DIRECTSQL_CONNECTIONLOST));
            ErrorBox aError(this, WB_OK, sMessage);
            aError.Execute();
        }

        PostUserEvent(LINK(this, DirectSQLDialog, OnClose));
    }

    //--------------------------------------------------------------------
    sal_Int32 DirectSQLDialog::getHistorySize() const
    {
        CHECK_INVARIANTS("DirectSQLDialog::getHistorySize");
        return m_aStatementHistory.size();
    }

    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
    void DirectSQLDialog::implAddToStatementHistory(const String& _rStatement)
    {
        CHECK_INVARIANTS("DirectSQLDialog::implAddToStatementHistory");

        // add the statement to the history
        m_aStatementHistory.push_back(_rStatement);

        // normalize the statement, and remember the normalized form, too
        String sNormalized(_rStatement);
        sNormalized.SearchAndReplaceAll((sal_Unicode)'\n', ' ');
        m_aNormalizedHistory.push_back(sNormalized);

        // add the normalized version to the list box
        m_pSQLHistory->InsertEntry(sNormalized);

        // ensure that we don't exceed the history limit
        implEnsureHistoryLimit();
    }

#ifdef DBG_UTIL
    //--------------------------------------------------------------------
    const sal_Char* DirectSQLDialog::impl_CheckInvariants() const
    {
        if (m_aStatementHistory.size() != m_aNormalizedHistory.size())
            return "statement history is inconsistent!";

        if (!m_pSQLHistory)
            return "invalid listbox!";

        if (m_aStatementHistory.size() != m_pSQLHistory->GetEntryCount())
            return "invalid listbox entry count!";

        if (!m_xConnection.is())
            return "have no connection!";

        return NULL;
    }
#endif

    //--------------------------------------------------------------------
    void DirectSQLDialog::implExecuteStatement(const String& _rStatement)
    {
        CHECK_INVARIANTS("DirectSQLDialog::implExecuteStatement");

        ::osl::MutexGuard aGuard(m_aMutex);

        String sStatus;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > xResultSet;
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
                    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow > xRow( xResultSet, ::com::sun::star::uno::UNO_QUERY );
                    // work through each of the rows
                    while (xResultSet->next())
                    {
                        // initialise the output line for each row
                        String out = OUString("");
                        // work along the columns until that are none left
                        try
                        {
                            int i = 1;
                            for (;;)
                            {
                                // be dumb, treat everything as a string
                                out += xRow->getString(i) + OUString(",");
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
            sStatus = String(ModuleRes(STR_COMMAND_EXECUTED_SUCCESSFULLY));

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

    //--------------------------------------------------------------------
    void DirectSQLDialog::addStatusText(const String& _rMessage)
    {
        OUString sAppendMessage = OUString::number(m_nStatusCount++) + ": " + _rMessage + "\n\n";

        OUString sCompleteMessage = m_pStatus->GetText() + sAppendMessage;
        m_pStatus->SetText(sCompleteMessage);

        m_pStatus->SetSelection(Selection(sCompleteMessage.getLength(), sCompleteMessage.getLength()));
    }

    //--------------------------------------------------------------------
    void DirectSQLDialog::addOutputText(const String& _rMessage)
    {
        String sAppendMessage = _rMessage;
        sAppendMessage += OUString("\n");

        String sCompleteMessage = m_pOutput->GetText();
        sCompleteMessage += sAppendMessage;
        m_pOutput->SetText(sCompleteMessage);
    }

    //--------------------------------------------------------------------
    void DirectSQLDialog::executeCurrent()
    {
        CHECK_INVARIANTS("DirectSQLDialog::executeCurrent");

        String sStatement = m_pSQL->GetText();

        // execute
        implExecuteStatement(sStatement);

        // add the statement to the history
        implAddToStatementHistory(sStatement);

        m_pSQL->SetSelection(Selection());
        m_pSQL->GrabFocus();
    }

    //--------------------------------------------------------------------
    void DirectSQLDialog::switchToHistory(sal_Int32 _nHistoryPos, sal_Bool _bUpdateListBox)
    {
        CHECK_INVARIANTS("DirectSQLDialog::switchToHistory");

        if ((_nHistoryPos >= 0) && (_nHistoryPos < getHistorySize()))
        {
            // set the text in the statement editor
            String sStatement = m_aStatementHistory[_nHistoryPos];
            m_pSQL->SetText(sStatement);
            OnStatementModified(m_pSQL);

            if (_bUpdateListBox)
            {
                // selecte the normalized statement in the list box
                m_pSQLHistory->SelectEntryPos((sal_uInt16)_nHistoryPos);
                OSL_ENSURE(m_pSQLHistory->GetSelectEntry() == m_aNormalizedHistory[_nHistoryPos],
                    "DirectSQLDialog::switchToHistory: inconsistent listbox entries!");
            }

            m_pSQL->GrabFocus();
            m_pSQL->SetSelection(Selection(sStatement.Len(), sStatement.Len()));
        }
        else
            OSL_FAIL("DirectSQLDialog::switchToHistory: invalid position!");
    }

    //--------------------------------------------------------------------
    IMPL_LINK( DirectSQLDialog, OnStatementModified, void*, /*NOTINTERESTEDIN*/ )
    {
        m_pExecute->Enable(!m_pSQL->GetText().isEmpty());
        return 0L;
    }

    //--------------------------------------------------------------------
    IMPL_LINK( DirectSQLDialog, OnClose, void*, /*NOTINTERESTEDIN*/ )
    {
        EndDialog( RET_OK );
        return 0L;
    }

    //--------------------------------------------------------------------
    IMPL_LINK( DirectSQLDialog, OnExecute, void*, /*NOTINTERESTEDIN*/ )
    {
        executeCurrent();
        return 0L;
    }

    //--------------------------------------------------------------------
    IMPL_LINK( DirectSQLDialog, OnListEntrySelected, void*, /*NOTINTERESTEDIN*/ )
    {
        if (!m_pSQLHistory->IsTravelSelect())
        {
            const sal_uInt16 nSelected = m_pSQLHistory->GetSelectEntryPos();
            if (LISTBOX_ENTRY_NOTFOUND != nSelected)
                switchToHistory(nSelected, sal_False);
        }
        return 0L;
    }

//........................................................................
}   // namespace dbaui
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
