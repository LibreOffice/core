/*************************************************************************
 *
 *  $RCSfile: wizardmachine.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2001-02-19 16:20:00 $
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

#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include "wizardmachine.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVTOOLS_SVTDATA_HXX
#include "svtdata.hxx"
#endif
#ifndef _SVTOOLS_HRC
#include "svtools.hrc"
#endif

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= OWizardPage
    //=====================================================================
    //---------------------------------------------------------------------
    void OWizardPage::initializePage()
    {
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardPage::commitPage(COMMIT_REASON _eReason)
    {
        return sal_True;
    }

    //=====================================================================
    //= OWizardMachine
    //=====================================================================
    //---------------------------------------------------------------------
    OWizardMachine::OWizardMachine(Window* _pParent, const ResId& _rRes, sal_uInt32 _nButtonFlags)
        :WizardDialog(_pParent, _rRes)
        ,m_pFinish(NULL)
        ,m_pCancel(NULL)
        ,m_pNextPage(NULL)
        ,m_pPrevPage(NULL)
        ,m_pHelp(NULL)
        ,m_nFirstUnknownPage(0)
    {
        m_sTitleBase = GetText();

        // create the buttons according to the wizard button flags
        // the cancel button
        if (_nButtonFlags & WZB_CANCEL)
        {
            m_pCancel = new CancelButton(this, WB_TABSTOP);
            m_pCancel->SetSizePixel( LogicToPixel( Size( 50, 14 ), MAP_APPFONT ) );
            m_pCancel->Show();

            AddButton( m_pCancel, WIZARDDIALOG_BUTTON_STDOFFSET_X );
        }

        // the previous button
        if (_nButtonFlags & WZB_PREVIOUS)
        {
            m_pPrevPage = new PushButton(this, WB_TABSTOP);
            m_pPrevPage->SetSizePixel( LogicToPixel( Size( 50, 14 ), MAP_APPFONT ) );
            m_pPrevPage->SetText(String(SvtResId(STR_WIZDLG_PREVIOUS)));
            m_pPrevPage->Show();

            if (_nButtonFlags & WZB_NEXT)
                AddButton( m_pPrevPage );       // no x-offset to the next button
            else
                AddButton( m_pPrevPage, WIZARDDIALOG_BUTTON_STDOFFSET_X );
            SetPrevButton( m_pPrevPage );
            m_pPrevPage->SetClickHdl( LINK( this, OWizardMachine, OnPrevPage ) );
        }

        // the next button
        if (_nButtonFlags & WZB_NEXT)
        {
            m_pNextPage = new PushButton(this, WB_TABSTOP);
            m_pNextPage->SetSizePixel( LogicToPixel( Size( 50, 14 ), MAP_APPFONT ) );
            m_pNextPage->SetText(String(SvtResId(STR_WIZDLG_NEXT)));
            m_pNextPage->Show();

            AddButton( m_pNextPage, WIZARDDIALOG_BUTTON_STDOFFSET_X );
            SetNextButton( m_pNextPage );
            m_pNextPage->SetClickHdl( LINK( this, OWizardMachine, OnNextPage ) );
        }

        // the finish button
        if (_nButtonFlags & WZB_FINISH)
        {
            m_pFinish = new OKButton(this, WB_TABSTOP);
            m_pFinish->SetSizePixel( LogicToPixel( Size( 50, 14 ), MAP_APPFONT ) );
            m_pFinish->SetText(String(SvtResId(STR_WIZDLG_FINISH)));
            m_pFinish->Show();

            AddButton( m_pFinish, WIZARDDIALOG_BUTTON_STDOFFSET_X );
            m_pFinish->SetClickHdl( LINK( this, OWizardMachine, OnFinish ) );
        }

        // the help button
        if (_nButtonFlags & WZB_HELP)
        {
            m_pHelp= new HelpButton(this, WB_TABSTOP);
            m_pHelp->SetSizePixel( LogicToPixel( Size( 50, 14 ), MAP_APPFONT ) );
            m_pHelp->Show();

            AddButton( m_pHelp, WIZARDDIALOG_BUTTON_STDOFFSET_X );
        }

    }

    //---------------------------------------------------------------------
    OWizardMachine::~OWizardMachine()
    {
        delete m_pFinish;
        delete m_pCancel;
        delete m_pNextPage;
        delete m_pPrevPage;
        delete m_pHelp;

        for (sal_uInt16 i=0; i<m_nFirstUnknownPage; ++i)
            delete GetPage(i);
    }

    //---------------------------------------------------------------------
    void OWizardMachine::implUpdateTitle()
    {
        String sCompleteTitle(m_sTitleBase);
        OWizardPage* pCurrentPage = getPage(getCurrentState());
        if (pCurrentPage)
        {
            sCompleteTitle += String::CreateFromAscii(" - ");
            sCompleteTitle += pCurrentPage->GetText();
        }
        SetText(sCompleteTitle);
    }

    //---------------------------------------------------------------------
    void OWizardMachine::setTitleBase(const String& _rTitleBase)
    {
        m_sTitleBase = _rTitleBase;
        implUpdateTitle();
    }

    //---------------------------------------------------------------------
    void OWizardMachine::ActivatePage()
    {
        WizardDialog::ActivatePage();

        sal_uInt16 nCurrentLevel = GetCurLevel();
        if (NULL == GetPage(nCurrentLevel))
        {
            TabPage* pNewPage = createPage(nCurrentLevel);
            DBG_ASSERT(pNewPage, "OWizardMachine::ActivatePage: invalid new page (NULL)!");

            // fill up the page sequence of our base class (with dummies)
            while (m_nFirstUnknownPage < nCurrentLevel)
            {
                AddPage(NULL);
                ++m_nFirstUnknownPage;
            }

            if (m_nFirstUnknownPage == nCurrentLevel)
            {
                // encountered this page number the first time
                AddPage(pNewPage);
                ++m_nFirstUnknownPage;
            }
            else
                // already had this page - just change it
                SetPage(nCurrentLevel, pNewPage);
        }

        enterState(nCurrentLevel);
    }

    //---------------------------------------------------------------------
    long OWizardMachine::DeactivatePage()
    {
        sal_uInt16 nCurrentState = getCurrentState();
        if (!leaveState(nCurrentState) || !WizardDialog::DeactivatePage())
            return sal_False;
        return sal_True;
    }

    //---------------------------------------------------------------------
    void OWizardMachine::defaultButton(sal_uInt32 _nWizardButtonFlags)
    {
        // the new default button
        PushButton* pNewDefButton = NULL;
        if (m_pFinish && (_nWizardButtonFlags & WZB_FINISH))
            pNewDefButton = m_pFinish;
        if (m_pNextPage && (_nWizardButtonFlags & WZB_NEXT))
            pNewDefButton = m_pNextPage;
        if (m_pPrevPage && (_nWizardButtonFlags & WZB_PREVIOUS))
            pNewDefButton = m_pPrevPage;
        if (m_pHelp && (_nWizardButtonFlags & WZB_HELP))
            pNewDefButton = m_pHelp;
        if (m_pCancel && (_nWizardButtonFlags & WZB_CANCEL))
            pNewDefButton = m_pCancel;

        if (pNewDefButton)
            defaultButton(pNewDefButton);
    }

    //---------------------------------------------------------------------
    void OWizardMachine::implResetDefault(Window* _pWindow)
    {
        Window* pChildLoop = _pWindow->GetWindow(WINDOW_FIRSTCHILD);
        while (pChildLoop)
        {
            // does the window participate in the tabbing order?
            if (pChildLoop->GetStyle() & WB_DIALOGCONTROL)
                implResetDefault(pChildLoop);

            // is it a button?
            WindowType eType = pChildLoop->GetType();
            if  (   (WINDOW_BUTTON == eType)
                ||  (WINDOW_PUSHBUTTON == eType)
                ||  (WINDOW_OKBUTTON == eType)
                ||  (WINDOW_CANCELBUTTON == eType)
                ||  (WINDOW_HELPBUTTON == eType)
                ||  (WINDOW_IMAGEBUTTON == eType)
                ||  (WINDOW_MENUBUTTON == eType)
                ||  (WINDOW_MOREBUTTON == eType)
                )
            {
                pChildLoop->SetStyle(pChildLoop->GetStyle() & ~WB_DEFBUTTON);

                // give the button the focus, this is the best method to enforce it to be repainted
                sal_uInt32 nSaveFocusId = Window::SaveFocus();
                pChildLoop->GrabFocus();
                Window::EndSaveFocus(nSaveFocusId);
            }

            // the next one ...
            pChildLoop = pChildLoop->GetWindow(WINDOW_NEXT);
        }
    }

    //---------------------------------------------------------------------
    void OWizardMachine::defaultButton(PushButton* _pNewDefButton)
    {
        // loop through all (direct and indirect) descendants which participate in our tabbing order, and
        // reset the WB_DEFBUTTON for every window which is a button
        implResetDefault(this);

        // set it's new style
        if (_pNewDefButton)
        {
            _pNewDefButton->SetStyle(_pNewDefButton->GetStyle() | WB_DEFBUTTON);
            // give the button the focus, this is the best method to enforce it to be repainted
            sal_uInt32 nSaveFocusId = Window::SaveFocus();
            _pNewDefButton->GrabFocus();
            Window::EndSaveFocus(nSaveFocusId);
        }
    }

    //---------------------------------------------------------------------
    void OWizardMachine::enableButtons(sal_uInt32 _nWizardButtonFlags, sal_Bool _bEnable)
    {
        if (m_pFinish && (_nWizardButtonFlags & WZB_FINISH))
            m_pFinish->Enable(_bEnable);
        if (m_pNextPage && (_nWizardButtonFlags & WZB_NEXT))
            m_pNextPage->Enable(_bEnable);
        if (m_pPrevPage && (_nWizardButtonFlags & WZB_PREVIOUS))
            m_pPrevPage->Enable(_bEnable);
        if (m_pHelp && (_nWizardButtonFlags & WZB_HELP))
            m_pHelp->Enable(_bEnable);
        if (m_pCancel && (_nWizardButtonFlags & WZB_CANCEL))
            m_pCancel->Enable(_bEnable);
    }

    //---------------------------------------------------------------------
    void OWizardMachine::enterState(sal_uInt16 _nState)
    {
        // tell the page
        OWizardPage* pCurrentPage = getPage(_nState);
        if (pCurrentPage)
            pCurrentPage->initializePage();

        // set the new title - it depends on the current page (i.e. state)
        implUpdateTitle();
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::leaveState(sal_uInt16 _nState)
    {
        // no need to ask the page here.
        // If we reach this point, we already gave the current page the chance to commit it's data,
        // and it was allowed to commit it's data

        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::onFinish(sal_Int32 _nResult)
    {
        return Finnish(_nResult);
    }

    //---------------------------------------------------------------------
    IMPL_LINK(OWizardMachine, OnFinish, PushButton*, NOINTERESTEDIN)
    {
        if (!implCommitCurrentPage(OWizardPage::CR_FINISH))
            return 0L;

        return onFinish( RET_OK );
    }

    //---------------------------------------------------------------------
    sal_uInt16 OWizardMachine::determineNextState(sal_uInt16 _nCurrentState)
    {
        return _nCurrentState + 1;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::implCommitCurrentPage(OWizardPage::COMMIT_REASON _eReason)
    {
        OWizardPage* pCurrentPage = getPage(getCurrentState());
        if (pCurrentPage)
            return pCurrentPage->commitPage(_eReason);
        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::travelNext()
    {
        // alowed to leave the current page?
        if (!implCommitCurrentPage(OWizardPage::CR_TRAVEL_NEXT))
            return sal_False;

        // determine the next state to travel to
        sal_uInt16 nCurrentState = getCurrentState();
        sal_uInt16 nNextState = determineNextState(nCurrentState);
        if (WZS_INVALID_STATE == nNextState)
            return sal_False;

        if (!ShowPage(nNextState))
            return sal_False;

        // all fine
        m_aStateHistory.push(nCurrentState);
        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::travelPrevious()
    {
        DBG_ASSERT(m_aStateHistory.size() > 0, "OWizardMachine::travelPrevious: have no previous page!");

        // alowed to leave the current page?
        if (!implCommitCurrentPage(OWizardPage::CR_TRAVEL_PREVIOUS))
            return sal_False;

        // the next state to switch to
        sal_uInt16 nPreviousState = m_aStateHistory.top();

        // show this page
        if (!ShowPage(nPreviousState))
            return sal_False;

        // all fine
        m_aStateHistory.pop();
        return sal_True;
    }

    //---------------------------------------------------------------------
    IMPL_LINK(OWizardMachine, OnPrevPage, PushButton*, NOINTERESTEDIN)
    {
        return travelPrevious();
    }

    //---------------------------------------------------------------------
    IMPL_LINK(OWizardMachine, OnNextPage, PushButton*, NOINTERESTEDIN)
    {
        return travelNext();
    }

//.........................................................................
}   // namespace svt
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/02/15 14:08:27  fs
 *  initial checkin - a wizard dialog base class
 *
 *  Revision 1.1  2001/02/12 07:16:52  fs
 *  initial checkin - importing StarOffice 5.2 database files
 *
 *
 *  Revision 1.0 30.01.01 17:04:12  fs
 ************************************************************************/

