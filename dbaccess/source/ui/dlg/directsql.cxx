/*************************************************************************
 *
 *  $RCSfile: directsql.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:22 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBACCESS_UI_DIRECTSQL_HXX_
#include "directsql.hxx"
#endif
#ifndef _DBACCESS_UI_DIRECTSQL_HRC_
#include "directsql.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

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
    //--------------------------------------------------------------------
    DirectSQLDialog::DirectSQLDialog( Window* _pParent, const Reference< XConnection >& _rxConn )
        :ModalDialog(_pParent, ModuleRes(DLG_DIRECTSQL))
        ,m_aFrame               (this, ResId(FL_SQL))
        ,m_aSQLLabel            (this, ResId(FT_SQL))
        ,m_aSQL                 (this, ResId(ME_SQL))
        ,m_aExecute             (this, ResId(PB_EXECUTE))
        ,m_aHistoryLabel        (this, ResId(FT_HISTORY))
        ,m_pSQLHistory(new LargeEntryListBox(this, ResId(LB_HISTORY)))
        ,m_aStatusFrame         (this, ResId(FL_STATUS))
        ,m_aStatus              (this, ResId(ME_STATUS))
        ,m_aButtonSeparator     (this, ResId(FL_BUTTONS))
        ,m_aHelp                (this, ResId(PB_HELP))
        ,m_aClose               (this, ResId(PB_CLOSE))
        ,m_nHistoryLimit(20)
        ,m_nStatusCount(1)
        ,m_xConnection(_rxConn)
    {
        FreeResource();

        m_aSQL.GrabFocus();

        m_aExecute.SetClickHdl(LINK(this, DirectSQLDialog, OnExecute));
        m_aClose.SetClickHdl(LINK(this, DirectSQLDialog, OnClose));
        m_pSQLHistory->SetSelectHdl(LINK(this, DirectSQLDialog, OnListEntrySelected));
        m_pSQLHistory->SetDropDownLineCount(10);

        // add a dispose listener to the connection
        Reference< XComponent > xConnComp(m_xConnection, UNO_QUERY);
        OSL_ENSURE(xConnComp.is(), "DirectSQLDialog::DirectSQLDialog: invalid connection!");
        if (xConnComp.is())
            startComponentListening(xConnComp);

        m_aSQL.SetModifyHdl(LINK(this, DirectSQLDialog, OnStatementModified));
        OnStatementModified(&m_aSQL);
    }

    //--------------------------------------------------------------------
    DirectSQLDialog::~DirectSQLDialog()
    {
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            stopAllComponentListening();
        }
        delete m_pSQLHistory;
    }

    //--------------------------------------------------------------------
    void DirectSQLDialog::_disposing( const EventObject& _rSource )
    {
        {
            ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
            ::osl::MutexGuard aGuard(m_aMutex);

            OSL_ENSURE(Reference< XConnection >(_rSource.Source, UNO_QUERY).get() == m_xConnection.get(),
                "DirectSQLDialog::_disposing: where does this come from?");

            {
                String sMessage(ModuleRes(STR_DIRECTSQL_CONNECTIONLOST));
                ErrorBox aError(this, WB_OK, sMessage);
                aError.Execute();
            }

            PostUserEvent(LINK(this, DirectSQLDialog, OnClose));
        }
    }

    //--------------------------------------------------------------------
    void DirectSQLDialog::addHistoryEntry(const String& _rStatement)
    {
        CHECK_INVARIANTS("DirectSQLDialog::addHistoryEntry");
        implAddToStatementHistory(_rStatement);
    }

    //--------------------------------------------------------------------
    sal_Int32 DirectSQLDialog::getHistorySize() const
    {
        CHECK_INVARIANTS("DirectSQLDialog::getHistorySize");
        return m_aStatementHistory.size();
    }

    //--------------------------------------------------------------------
    void DirectSQLDialog::setHistoryLimit(sal_Int32 _nMaxEntries)
    {
        CHECK_INVARIANTS("DirectSQLDialog::setHistoryLimit");

        m_nHistoryLimit = _nMaxEntries;
        implEnsureHistoryLimit();
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
            m_pSQLHistory->RemoveEntry((USHORT)0);
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
        try
        {
            // create a statement
            Reference< XStatement > xStatement = m_xConnection->createStatement();
            OSL_ENSURE(xStatement.is(), "DirectSQLDialog::implExecuteStatement: no statement returned by the connection!");

            // execute it
            if (xStatement.is())
                xStatement->execute(_rStatement);

            // successfull
            sStatus = String(ModuleRes(STR_COMMAND_EXECUTED_SUCCESSFULLY));

            // dispose the statement
            ::comphelper::disposeComponent(xStatement);
        }
        catch(const SQLException& e)
        {
            sStatus = e.Message;
        }
        catch(const Exception&)
        {
            OSL_ENSURE(sal_False, "DirectSQLDialog::implExecuteStatement: caught an (unknown) exception!");
        }

        // add the status text
        addStatusText(sStatus);
    }

    //--------------------------------------------------------------------
    void DirectSQLDialog::addStatusText(const String& _rMessage)
    {
        String sAppendMessage = String::CreateFromInt32(m_nStatusCount++);
        sAppendMessage += String::CreateFromAscii(": ");
        sAppendMessage += _rMessage;
        sAppendMessage += String::CreateFromAscii("\n\n");

        String sCompleteMessage = m_aStatus.GetText();
        sCompleteMessage += sAppendMessage;
        m_aStatus.SetText(sCompleteMessage);

        m_aStatus.SetSelection(Selection(sCompleteMessage.Len(), sCompleteMessage.Len()));
    }

    //--------------------------------------------------------------------
    void DirectSQLDialog::executeCurrent()
    {
        CHECK_INVARIANTS("DirectSQLDialog::executeCurrent");

        String sStatement = m_aSQL.GetText();

        // execute
        implExecuteStatement(sStatement);

        // add the statement to the history
        implAddToStatementHistory(sStatement);

        m_aSQL.SetSelection(Selection());
        m_aSQL.GrabFocus();
    }

    //--------------------------------------------------------------------
    void DirectSQLDialog::switchToHistory(sal_Int32 _nHistoryPos, sal_Bool _bUpdateListBox)
    {
        CHECK_INVARIANTS("DirectSQLDialog::switchToHistory");

        if ((_nHistoryPos >= 0) && (_nHistoryPos < getHistorySize()))
        {
            // set the text in the statement editor
            String sStatement = m_aStatementHistory[_nHistoryPos];
            m_aSQL.SetText(sStatement);
            OnStatementModified(&m_aSQL);

            if (_bUpdateListBox)
            {
                // selecte the normalized statement in the list box
                m_pSQLHistory->SelectEntryPos((USHORT)_nHistoryPos);
                OSL_ENSURE(m_pSQLHistory->GetSelectEntry() == m_aNormalizedHistory[_nHistoryPos],
                    "DirectSQLDialog::switchToHistory: inconsistent listbox entries!");
            }

            m_aSQL.GrabFocus();
            m_aSQL.SetSelection(Selection(sStatement.Len(), sStatement.Len()));
        }
        else
            OSL_ENSURE(sal_False, "DirectSQLDialog::switchToHistory: invalid position!");
    }

    //--------------------------------------------------------------------
    IMPL_LINK( DirectSQLDialog, OnStatementModified, void*, NOTINTERESTEDIN )
    {
        m_aExecute.Enable(0 != m_aSQL.GetText().Len());
        return 0L;
    }

    //--------------------------------------------------------------------
    IMPL_LINK( DirectSQLDialog, OnClose, void*, NOTINTERESTEDIN )
    {
        EndDialog( RET_OK );
        return 0L;
    }

    //--------------------------------------------------------------------
    IMPL_LINK( DirectSQLDialog, OnExecute, void*, NOTINTERESTEDIN )
    {
        executeCurrent();
        return 0L;
    }

    //--------------------------------------------------------------------
    IMPL_LINK( DirectSQLDialog, OnListEntrySelected, void*, NOTINTERESTEDIN )
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

