/*************************************************************************
 *
 *  $RCSfile: wizardmachine.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:37:20 $
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
#ifndef SVTOOLS_WIZARDHEADER_HXX
#include "wizardheader.hxx"
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#define HEADER_SIZE_Y   30

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= WizardPageImplData
    //=====================================================================
    struct WizardPageImplData
    {
        WizardHeader*       pHeader;

        WizardPageImplData()
            :pHeader( NULL )
        {
        }
    };

    //=====================================================================
    //= OWizardPage
    //=====================================================================
    //---------------------------------------------------------------------
    OWizardPage::OWizardPage( OWizardMachine* _pParent, WinBits _nStyle )
        :TabPage( _pParent, _nStyle )
        ,m_pImpl( new WizardPageImplData )
    {
    }

    //---------------------------------------------------------------------
    OWizardPage::OWizardPage( OWizardMachine* _pParent, const ResId& _rResId )
        :TabPage( _pParent, _rResId )
        ,m_pImpl( new WizardPageImplData )
    {
    }

    //---------------------------------------------------------------------
    OWizardPage::~OWizardPage()
    {
        delete m_pImpl->pHeader;
        delete m_pImpl;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardPage::isHeaderEnabled( ) const
    {
        return NULL != m_pImpl->pHeader;
    }

    //---------------------------------------------------------------------
    void OWizardPage::enableHeader( const Bitmap& _rBitmap, sal_Int32 _nPixelHeight, IWizardPage::GrantAccess )
    {
        //.................................................................
        // create the header control
        m_pImpl->pHeader = new WizardHeader( this );

        // move it to the upper left corner
        m_pImpl->pHeader->SetPosPixel( Point( 0, 0 ) );

        // size it: as wide as we are, and 30 APPFONT units high
        Size aHeaderSize( GetSizePixel().Width(), _nPixelHeight );
        m_pImpl->pHeader->SetSizePixel( aHeaderSize );

        // set bitmap / text of the control
        m_pImpl->pHeader->setHeaderBitmap( _rBitmap );
        m_pImpl->pHeader->setHeaderText( GetText() );

        // show
        m_pImpl->pHeader->Show( );

        //.................................................................
        // move all the other controls down
        Point aChildPos;
        // loop through the children
        Window* pChildLoop = GetWindow( WINDOW_FIRSTCHILD );
        while ( pChildLoop )
        {
            if ( pChildLoop != m_pImpl->pHeader )
            {   // it's not the header itself
                aChildPos = pChildLoop->GetPosPixel();
                aChildPos.Y() += aHeaderSize.Height();
                pChildLoop->SetPosPixel( aChildPos );
            }

            pChildLoop = pChildLoop->GetWindow( WINDOW_NEXT );
        }
    }

    //---------------------------------------------------------------------
    void OWizardPage::setHeaderText( const String& _rHeaderText )
    {
        if ( !isHeaderEnabled() )
        {
            DBG_ERROR( "OWizardPage::setHeaderText: have no header!" );
            return;
        }
        m_pImpl->pHeader->setHeaderText( _rHeaderText );
    }

    //---------------------------------------------------------------------
    void OWizardPage::initializePage()
    {
    }

    //---------------------------------------------------------------------
    void OWizardPage::ActivatePage()
    {
        TabPage::ActivatePage();
        implCheckNextButton();
    }

    //---------------------------------------------------------------------
    void OWizardPage::implCheckNextButton()
    {
        static_cast< OWizardMachine* >(GetParent())->enableButtons(WZB_NEXT, determineNextButtonState());
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardPage::determineNextButtonState()
    {
        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardPage::commitPage(IWizardPage::COMMIT_REASON _eReason)
    {
        return sal_True;
    }

    //=====================================================================
    //= WizardMachineImplData
    //=====================================================================
    struct WizardMachineImplData : public WizardTypes
    {
        String                          sTitleBase;         // the base for the title
        ::std::stack< WizardState >     aStateHistory;      // the history of all states (used for implementing "Back")
        Bitmap                          aHeaderBitmap;      // the bitmap to use for the page header

        sal_Int32                       nHeaderHeight;      // the height (in pixels) of the page header

        WizardState                     nFirstUnknownPage;
            // the WizardDialog does not allow non-linear transitions (e.g. it's
            // not possible to add pages in a non-linear order), so we need some own maintainance data

        sal_Bool                        bUsingHeaders;      // do we use page headers?
        sal_Bool                        m_bCheckButtonStates;

        WizardMachineImplData()
            :nFirstUnknownPage( 0 )
            ,bUsingHeaders( sal_False )
        {
        }
    };

    //=====================================================================
    //= OWizardMachine
    //=====================================================================
    //---------------------------------------------------------------------
    OWizardMachine::OWizardMachine(Window* _pParent, const ResId& _rRes, sal_uInt32 _nButtonFlags,sal_Bool _bCheckButtonStates)
        :WizardDialog(_pParent, _rRes)
        ,m_pFinish(NULL)
        ,m_pCancel(NULL)
        ,m_pNextPage(NULL)
        ,m_pPrevPage(NULL)
        ,m_pHelp(NULL)
        ,m_pImpl( new WizardMachineImplData )
    {
        m_pImpl->sTitleBase = GetText();
        m_pImpl->m_bCheckButtonStates = _bCheckButtonStates;

        // create the buttons according to the wizard button flags
        // the help button
        if (_nButtonFlags & WZB_HELP)
        {
            m_pHelp= new HelpButton(this, WB_TABSTOP);
            m_pHelp->SetSizePixel( LogicToPixel( Size( 50, 14 ), MAP_APPFONT ) );
            m_pHelp->Show();

            AddButton( m_pHelp, WIZARDDIALOG_BUTTON_STDOFFSET_X );
        }

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

    }

    //---------------------------------------------------------------------
    OWizardMachine::~OWizardMachine()
    {
        delete m_pFinish;
        delete m_pCancel;
        delete m_pNextPage;
        delete m_pPrevPage;
        delete m_pHelp;

        for (WizardState i=0; i<m_pImpl->nFirstUnknownPage; ++i)
            delete GetPage(i);

        delete m_pImpl;
    }

    //---------------------------------------------------------------------
    void OWizardMachine::implUpdateTitle()
    {
        String sCompleteTitle(m_pImpl->sTitleBase);
        if ( !m_pImpl->bUsingHeaders )
        {   // append the page title only if we're not using headers - in this case, the title
            // would be part of the header
            TabPage* pCurrentPage = GetPage(getCurrentState());
            if ( pCurrentPage && pCurrentPage->GetText().Len() )
            {
                sCompleteTitle += String::CreateFromAscii(" - ");
                sCompleteTitle += pCurrentPage->GetText();
            }
        }
        SetText(sCompleteTitle);
    }

    //---------------------------------------------------------------------
    void OWizardMachine::enableHeader( const Bitmap& _rBitmap, sal_Int32 _nPixelHeight )
    {
        if ( m_pImpl->bUsingHeaders )
            // nothing to do
            return;

        // is the height defaulted?
        if ( -1 == _nPixelHeight )
        {
            _nPixelHeight = LogicToPixel( Size( 0, 30 ), MAP_APPFONT ).Height();
        }

        m_pImpl->bUsingHeaders = sal_True;
        m_pImpl->aHeaderBitmap = _rBitmap;
        m_pImpl->nHeaderHeight = _nPixelHeight;

#ifdef DBG_UTIL
        for (WizardState i=0; i<m_pImpl->nFirstUnknownPage; ++i)
        {
            DBG_ASSERT( NULL == GetPage( i ), "OWizardMachine::enableHeader: there already are pages!" );
            // this method has not to be called if there already have been created any pages
        }
#endif
    }

    //---------------------------------------------------------------------
    const String& OWizardMachine::getTitleBase() const
    {
        return m_pImpl->sTitleBase;
    }

    //---------------------------------------------------------------------
    void OWizardMachine::setTitleBase(const String& _rTitleBase)
    {
        m_pImpl->sTitleBase = _rTitleBase;
        implUpdateTitle();
    }

    //---------------------------------------------------------------------
    void OWizardMachine::ActivatePage()
    {
        WizardDialog::ActivatePage();

        WizardState nCurrentLevel = GetCurLevel();
        if (NULL == GetPage(nCurrentLevel))
        {
            TabPage* pNewPage = createPage(nCurrentLevel);
            DBG_ASSERT(pNewPage, "OWizardMachine::ActivatePage: invalid new page (NULL)!");

            // announce our header bitmap to the page
            if ( m_pImpl->bUsingHeaders )
            {
                IWizardPage* pHeader = getWizardPage(pNewPage);
                if ( pHeader )
                    pHeader->enableHeader( m_pImpl->aHeaderBitmap, m_pImpl->nHeaderHeight, IWizardPage::GrantAccess() );
            }

            // fill up the page sequence of our base class (with dummies)
            while (m_pImpl->nFirstUnknownPage < nCurrentLevel)
            {
                AddPage(NULL);
                ++m_pImpl->nFirstUnknownPage;
            }

            if (m_pImpl->nFirstUnknownPage == nCurrentLevel)
            {
                // encountered this page number the first time
                AddPage(pNewPage);
                ++m_pImpl->nFirstUnknownPage;
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
        WizardState nCurrentState = getCurrentState();
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
    void OWizardMachine::enterState(WizardState _nState)
    {
        // tell the page
        IWizardPage* pCurrentPage = getWizardPage(GetPage(_nState));
        if ( pCurrentPage )
            pCurrentPage->initializePage();

        if ( m_pImpl->m_bCheckButtonStates )
            enableButtons(WZB_NEXT,WZS_INVALID_STATE != determineNextState(_nState) );
        enableButtons(WZB_PREVIOUS,!m_pImpl->aStateHistory.empty());

        // set the new title - it depends on the current page (i.e. state)
        implUpdateTitle();
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::leaveState(WizardState _nState)
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
        if ( !prepareLeaveCurrentState( eFinish ) )
            return 0L;

        return onFinish( RET_OK );
    }

    //---------------------------------------------------------------------
    OWizardMachine::WizardState OWizardMachine::determineNextState( WizardState _nCurrentState )
    {
        return _nCurrentState + 1;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::prepareLeaveCurrentState( CommitPageReason _eReason )
    {
        IWizardPage* pCurrentPage = getWizardPage(GetPage(getCurrentState()));
        if ( pCurrentPage )
            return pCurrentPage->commitPage( ( IWizardPage::COMMIT_REASON )_eReason );
        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::skipBackwardUntil( WizardState _nTargetState )
    {
        // alowed to leave the current page?
        if ( !prepareLeaveCurrentState( eTravelBackward ) )
            return sal_False;

        // don't travel directly on m_pImpl->aStateHistory, in case something goes wrong
        ::std::stack< WizardState > aTravelVirtually = m_pImpl->aStateHistory;

        WizardState nCurrentRollbackState = getCurrentState();
        while ( nCurrentRollbackState != _nTargetState )
        {
            DBG_ASSERT( !aTravelVirtually.empty(), "OWizardMachine::skipBackwardUntil: this target state does not exist in the history!" );
            nCurrentRollbackState = aTravelVirtually.top();
            aTravelVirtually.pop();
        }

        if ( !ShowPage( _nTargetState ) )
            return sal_False;

        m_pImpl->aStateHistory = aTravelVirtually;
        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::skipUntil( WizardState _nTargetState )
    {
        // alowed to leave the current page?
        if ( !prepareLeaveCurrentState( eTravelBackward ) )
            return sal_False;

        // don't travel directly on m_pImpl->aStateHistory, in case something goes wrong
        ::std::stack< WizardState > aTravelVirtually = m_pImpl->aStateHistory;

        WizardState nCurrentState = getCurrentState();
        while ( nCurrentState != _nTargetState )
        {
            WizardState nNextState = determineNextState( nCurrentState );
            if ( WZS_INVALID_STATE == nNextState )
            {
                DBG_ERROR( "OWizardMachine::skipUntil: the given target state does not exist!" );
                return sal_False;
            }

            // remember the skipped state in the history
            aTravelVirtually.push( nCurrentState );

            // get the next state
            nCurrentState = nNextState;
        }

        // show the target page
        if ( !ShowPage( nCurrentState ) )
        {
            // argh! prepareLeaveCurrentPage succeeded, determineNextState succeeded,
            // but ShowPage doesn't? Somebody behaves very strange here ....
            DBG_ERROR( "OWizardMachine::skipUntil: very unpolite ...." );
            return sal_False;
        }

        m_pImpl->aStateHistory = aTravelVirtually;
        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::skip(sal_Int32 _nSteps)
    {
        DBG_ASSERT(_nSteps > 0, "OWizardMachine::skip: invalid number of steps!");
        // alowed to leave the current page?
        if ( !prepareLeaveCurrentState( eTravelForward ) )
            return sal_False;

        WizardState nCurrentState = getCurrentState();
        WizardState nNextState = determineNextState(nCurrentState);
        // loop _nSteps steps
        while (_nSteps-- > 0)
        {
            if (WZS_INVALID_STATE == nNextState)
                return sal_False;

            // remember the skipped state in the history
            m_pImpl->aStateHistory.push(nCurrentState);

            // get the next state
            nCurrentState = nNextState;
            nNextState = determineNextState(nCurrentState);
        }

        // show the (n+1)th page
        if (!ShowPage(nCurrentState))
        {
            // TODO: this leaves us in a state where we have no current page and an inconsistent state history.
            // Perhaps we should rollback the skipping here ....
            DBG_ERROR("OWizardMachine::skip: very unpolite ....");
                // if somebody does a skip and then does not allow to leave ...
                // (can't be a commit error, as we've already committed the current page. So if ShowPage fails here,
                // somebody behaves really strange ...)
            return sal_False;
        }

        // all fine
        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::travelNext()
    {
        // allowed to leave the current page?
        if ( !prepareLeaveCurrentState( eTravelForward ) )
            return sal_False;

        // determine the next state to travel to
        WizardState nCurrentState = getCurrentState();
        WizardState nNextState = determineNextState(nCurrentState);
        if (WZS_INVALID_STATE == nNextState)
            return sal_False;

        // the state history is used by the enterState method
        // all fine
        m_pImpl->aStateHistory.push(nCurrentState);
        if (!ShowPage(nNextState))
        {
            m_pImpl->aStateHistory.pop();
            return sal_False;
        }

        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::travelPrevious()
    {
        DBG_ASSERT(m_pImpl->aStateHistory.size() > 0, "OWizardMachine::travelPrevious: have no previous page!");

        // alowed to leave the current page?
        if ( !prepareLeaveCurrentState( eTravelBackward ) )
            return sal_False;

        // the next state to switch to
        WizardState nPreviousState = m_pImpl->aStateHistory.top();

        // the state history is used by the enterState method
        m_pImpl->aStateHistory.pop();
        // show this page
        if (!ShowPage(nPreviousState))
        {
            m_pImpl->aStateHistory.push(nPreviousState);
            return sal_False;
        }

        // all fine
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
    //---------------------------------------------------------------------
    IWizardPage* OWizardMachine::getWizardPage(TabPage* _pCurrentPage) const
    {
        OWizardPage* pPage = static_cast<OWizardPage*>(_pCurrentPage);
        return pPage;
    }

//.........................................................................
}   // namespace svt
//.........................................................................

