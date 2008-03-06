/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wizardmachine.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:25:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif
#include <svtools/helpid.hrc>
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVTOOLS_SVTDATA_HXX
#include <svtools/svtdata.hxx>
#endif
#ifndef _SVTOOLS_HRC
#include <svtools/svtools.hrc>
#endif

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= WizardPageImplData
    //=====================================================================
    struct WizardPageImplData
    {
        WizardPageImplData()
        {
        }
    };

    //=====================================================================
    //= OWizardPage
    //=====================================================================
    //---------------------------------------------------------------------
    OWizardPage::OWizardPage( Window* _pParent, WinBits _nStyle )
        :TabPage( _pParent, _nStyle )
        ,m_pImpl( new WizardPageImplData )
    {
    }

    //---------------------------------------------------------------------
    OWizardPage::OWizardPage( Window* _pParent, const ResId& _rResId )
        :TabPage( _pParent, _rResId )
        ,m_pImpl( new WizardPageImplData )
    {
    }

    //---------------------------------------------------------------------
    OWizardPage::~OWizardPage()
    {
        delete m_pImpl;
    }

    //---------------------------------------------------------------------
    void OWizardPage::initializePage()
    {
    }

    //---------------------------------------------------------------------
    void OWizardPage::ActivatePage()
    {
        TabPage::ActivatePage();
        updateDialogTravelUI();
    }

    //---------------------------------------------------------------------
    void OWizardPage::updateDialogTravelUI()
    {
        OWizardMachine* pWizardMachine = dynamic_cast< OWizardMachine* >( GetParent() );
        OSL_ENSURE( pWizardMachine, "OWizardPage::updateDialogTravelUI: where am I?" );
        if ( pWizardMachine )
            pWizardMachine->updateTravelUI();
    }

    //---------------------------------------------------------------------
    bool OWizardPage::canAdvance() const
    {
        return true;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardPage::commitPage( CommitPageReason )
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

        WizardState                     nFirstUnknownPage;
            // the WizardDialog does not allow non-linear transitions (e.g. it's
            // not possible to add pages in a non-linear order), so we need some own maintainance data

        sal_Bool                        m_bAutoNextButtonState;

        bool                            m_bTravelingSuspended;

        WizardMachineImplData()
            :nFirstUnknownPage( 0 )
            ,m_bAutoNextButtonState( sal_False )
            ,m_bTravelingSuspended( false )
        {
        }
    };

    long OWizardMachine::calcRightHelpOffset(sal_uInt32 _nButtonFlags)
    {
        sal_Int32 nMask = 1;
        sal_Int32 nRightAlignedButtonCount = -1;
        for (unsigned int i = 0; i < 8*sizeof(_nButtonFlags); i++ )
        {
            if( ( _nButtonFlags & nMask ) != 0 )
                nRightAlignedButtonCount++;
            nMask <<= 1;
        }
        Size aSize = GetPageSizePixel();
        sal_Int32 nTotButtonWidth = nRightAlignedButtonCount * LogicalCoordinateToPixel(50);
        sal_Int32 nTotRightButtonSpaceOffset = (nRightAlignedButtonCount) * WIZARDDIALOG_BUTTON_STDOFFSET_X;
        if ((_nButtonFlags & WZB_NEXT) && (_nButtonFlags & WZB_NEXT))
            nTotRightButtonSpaceOffset = (nTotRightButtonSpaceOffset - WIZARDDIALOG_BUTTON_STDOFFSET_X) + WIZARDDIALOG_BUTTON_SMALLSTDOFFSET_X;
        return aSize.Width() - nTotButtonWidth - nTotRightButtonSpaceOffset;
    }

    //=====================================================================
    //= OWizardMachine
    //=====================================================================
    //---------------------------------------------------------------------
    OWizardMachine::OWizardMachine(Window* _pParent, const ResId& _rRes, sal_uInt32 _nButtonFlags )
        :WizardDialog( _pParent, _rRes )
        ,m_pFinish(NULL)
        ,m_pCancel(NULL)
        ,m_pNextPage(NULL)
        ,m_pPrevPage(NULL)
        ,m_pHelp(NULL)
        ,m_pImpl( new WizardMachineImplData )
    {
        m_pImpl->sTitleBase = GetText();

        // create the buttons according to the wizard button flags
        // the help button
        if (_nButtonFlags & WZB_HELP)
        {
            m_pHelp= new HelpButton(this, WB_TABSTOP);
            m_pHelp->SetSizePixel( LogicToPixel( Size( 50, 14 ), MAP_APPFONT ) );
            m_pHelp->Show();
            AddButton( m_pHelp, WIZARDDIALOG_BUTTON_STDOFFSET_X);
        }

        // the previous button
        if (_nButtonFlags & WZB_PREVIOUS)
        {
            m_pPrevPage = new PushButton(this, WB_TABSTOP);
            m_pPrevPage->SetSmartHelpId( SmartId(HID_WIZARD_PREVIOUS) );
            m_pPrevPage->SetSizePixel( LogicToPixel( Size( 50, 14 ), MAP_APPFONT ) );
            m_pPrevPage->SetText(String(SvtResId(STR_WIZDLG_PREVIOUS)));
            m_pPrevPage->Show();

            if (_nButtonFlags & WZB_NEXT)
                AddButton( m_pPrevPage, ( WIZARDDIALOG_BUTTON_SMALLSTDOFFSET_X) );      // half x-offset to the next button
            else
                AddButton( m_pPrevPage, WIZARDDIALOG_BUTTON_STDOFFSET_X );
            SetPrevButton( m_pPrevPage );
            m_pPrevPage->SetClickHdl( LINK( this, OWizardMachine, OnPrevPage ) );
        }

        // the next button
        if (_nButtonFlags & WZB_NEXT)
        {
            m_pNextPage = new PushButton(this, WB_TABSTOP);
            m_pNextPage->SetSmartHelpId( SmartId(HID_WIZARD_NEXT) );
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

        // the cancel button
        if (_nButtonFlags & WZB_CANCEL)
        {
            m_pCancel = new CancelButton(this, WB_TABSTOP);
            m_pCancel->SetSizePixel( LogicToPixel( Size( 50, 14 ), MAP_APPFONT ) );
            m_pCancel->Show();

            AddButton( m_pCancel, WIZARDDIALOG_BUTTON_STDOFFSET_X );
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

        // append the page title
        TabPage* pCurrentPage = GetPage(getCurrentState());
        if ( pCurrentPage && pCurrentPage->GetText().Len() )
        {
            sCompleteTitle += String::CreateFromAscii(" - ");
            sCompleteTitle += pCurrentPage->GetText();
        }

        SetText(sCompleteTitle);
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
            _pNewDefButton->SetStyle(_pNewDefButton->GetStyle() | WB_DEFBUTTON);
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

        if ( isAutomaticNextButtonStateEnabled() )
            enableButtons( WZB_NEXT, canAdvance() );

        enableButtons( WZB_PREVIOUS, !m_pImpl->aStateHistory.empty() );

        // set the new title - it depends on the current page (i.e. state)
        implUpdateTitle();
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::leaveState(WizardState)
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
    IMPL_LINK(OWizardMachine, OnFinish, PushButton*, EMPTYARG)
    {
        if ( isTravelingSuspended() )
            return 0;
        WizardTravelSuspension aTravelGuard( *this );
        if ( !prepareLeaveCurrentState( eFinish ) )
        {
            return 0L;
        }
        long nRet = onFinish( RET_OK );
        return nRet;
    }

    //---------------------------------------------------------------------
    OWizardMachine::WizardState OWizardMachine::determineNextState( WizardState _nCurrentState ) const
    {
        return _nCurrentState + 1;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::prepareLeaveCurrentState( CommitPageReason _eReason )
    {
        IWizardPage* pCurrentPage = getWizardPage(GetPage(getCurrentState()));
        if ( pCurrentPage )
            return pCurrentPage->commitPage( _eReason );
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
        ::std::stack< WizardState > aOldStateHistory = m_pImpl->aStateHistory;

        WizardState nCurrentRollbackState = getCurrentState();
        while ( nCurrentRollbackState != _nTargetState )
        {
            DBG_ASSERT( !aTravelVirtually.empty(), "OWizardMachine::skipBackwardUntil: this target state does not exist in the history!" );
            nCurrentRollbackState = aTravelVirtually.top();
            aTravelVirtually.pop();
        }
        m_pImpl->aStateHistory = aTravelVirtually;
        if ( !ShowPage( _nTargetState ) )
        {
            m_pImpl->aStateHistory = aOldStateHistory;
            return sal_False;
        }
        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OWizardMachine::skipUntil( WizardState _nTargetState )
    {
        WizardState nCurrentState = getCurrentState();

        // alowed to leave the current page?
        if ( !prepareLeaveCurrentState( nCurrentState < _nTargetState ? eTravelForward : eTravelBackward ) )
            return sal_False;

        // don't travel directly on m_pImpl->aStateHistory, in case something goes wrong
        ::std::stack< WizardState > aTravelVirtually = m_pImpl->aStateHistory;
        ::std::stack< WizardState > aOldStateHistory = m_pImpl->aStateHistory;
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
        m_pImpl->aStateHistory = aTravelVirtually;
        // show the target page
        if ( !ShowPage( nCurrentState ) )
        {
            // argh! prepareLeaveCurrentPage succeeded, determineNextState succeeded,
            // but ShowPage doesn't? Somebody behaves very strange here ....
            DBG_ERROR( "OWizardMachine::skipUntil: very unpolite ...." );
            m_pImpl->aStateHistory = aOldStateHistory;
            return sal_False;
        }
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
    void  OWizardMachine::removePageFromHistory( WizardState nToRemove )
    {

        ::std::stack< WizardState > aTemp;
        while(!m_pImpl->aStateHistory.empty())
        {
            WizardState nPreviousState = m_pImpl->aStateHistory.top();
            m_pImpl->aStateHistory.pop();
            if(nPreviousState != nToRemove)
                aTemp.push( nPreviousState );
            else
                break;
        }
        while(!aTemp.empty())
        {
            m_pImpl->aStateHistory.push( aTemp.top() );
            aTemp.pop();
        }
    }

    //---------------------------------------------------------------------
    void OWizardMachine::enableAutomaticNextButtonState( bool _bEnable )
    {
        m_pImpl->m_bAutoNextButtonState = _bEnable;
    }

    //---------------------------------------------------------------------
    bool OWizardMachine::isAutomaticNextButtonStateEnabled() const
    {
        return m_pImpl->m_bAutoNextButtonState;
    }

    //---------------------------------------------------------------------
    IMPL_LINK(OWizardMachine, OnPrevPage, PushButton*, EMPTYARG)
    {
        if ( isTravelingSuspended() )
            return 0;
        WizardTravelSuspension aTravelGuard( *this );
        sal_Int32 nRet = travelPrevious();
        return nRet;
    }

    //---------------------------------------------------------------------
    IMPL_LINK(OWizardMachine, OnNextPage, PushButton*, EMPTYARG)
    {
        if ( isTravelingSuspended() )
            return 0;
        WizardTravelSuspension aTravelGuard( *this );
        sal_Int32 nRet = travelNext();
        return nRet;
    }

    //---------------------------------------------------------------------
    IWizardPage* OWizardMachine::getWizardPage(TabPage* _pCurrentPage) const
    {
        OWizardPage* pPage = dynamic_cast< OWizardPage* >( _pCurrentPage );
        return pPage;
    }

    //---------------------------------------------------------------------
    bool OWizardMachine::canAdvance() const
    {
        return WZS_INVALID_STATE != determineNextState( getCurrentState() );
    }

    //---------------------------------------------------------------------
    void OWizardMachine::updateTravelUI()
    {
        OWizardPage* pPage = dynamic_cast< OWizardPage* >( GetPage( getCurrentState() ) );

        bool bCanAdvance =
                ( !pPage || pPage->canAdvance() )   // the current page allows to advance
            &&  canAdvance();                       // the dialog as a whole allows to advance
        enableButtons( WZB_NEXT, bCanAdvance );
    }

    //---------------------------------------------------------------------
    bool OWizardMachine::isTravelingSuspended() const
    {
        return m_pImpl->m_bTravelingSuspended;
    }

    //---------------------------------------------------------------------
    void OWizardMachine::suspendTraveling( AccessGuard )
    {
        DBG_ASSERT( !m_pImpl->m_bTravelingSuspended, "OWizardMachine::suspendTraveling: already suspended!" );
       m_pImpl->m_bTravelingSuspended = true;
    }

    //---------------------------------------------------------------------
    void OWizardMachine::resumeTraveling( AccessGuard )
    {
        DBG_ASSERT( m_pImpl->m_bTravelingSuspended, "OWizardMachine::resumeTraveling: nothing to resume!" );
       m_pImpl->m_bTravelingSuspended = false;
    }

//.........................................................................
}   // namespace svt
//.........................................................................
