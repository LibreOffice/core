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

#include <svtools/wizardmachine.hxx>
#include <svtools/helpids.h>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <svtools/svtresid.hxx>
#include <svtools/strings.hrc>
#include <vcl/svapp.hxx>
#include <stack>

namespace svt
{
    //= WizardPageImplData
    OWizardPage::OWizardPage(vcl::Window *pParent, const OString& rID,
        const OUString& rUIXMLDescription)
        : TabPage(pParent, rID, rUIXMLDescription)
    {
    }

    OWizardPage::OWizardPage(TabPageParent pParent, const OUString& rUIXMLDescription, const OString& rID)
        : TabPage(pParent.pPage ? Application::GetDefDialogParent() : pParent.pParent.get()) //just drag this along hidden in this scenario
        , m_xBuilder(pParent.pPage ? Application::CreateBuilder(pParent.pPage, rUIXMLDescription)
                                   : Application::CreateInterimBuilder(this, rUIXMLDescription))
        , m_xContainer(m_xBuilder->weld_container(rID))
    {
    }

    OWizardPage::~OWizardPage()
    {
        disposeOnce();
    }

    void OWizardPage::dispose()
    {
        m_xBuilder.reset();
        TabPage::dispose();
    }

    void OWizardPage::initializePage()
    {
    }

    void OWizardPage::ActivatePage()
    {
        TabPage::ActivatePage();
        updateDialogTravelUI();
    }

    void OWizardPage::updateDialogTravelUI()
    {
        OWizardMachine* pWizardMachine = dynamic_cast< OWizardMachine* >( GetParent() );
        if ( pWizardMachine )
            pWizardMachine->updateTravelUI();
    }

    bool OWizardPage::canAdvance() const
    {
        return true;
    }

    bool OWizardPage::commitPage( WizardTypes::CommitPageReason )
    {
        return true;
    }

    struct WizardMachineImplData : public WizardTypes
    {
        OUString                        sTitleBase;         // the base for the title
        ::std::stack< WizardState >     aStateHistory;      // the history of all states (used for implementing "Back")

        WizardState                     nFirstUnknownPage;
            // the WizardDialog does not allow non-linear transitions (e.g. it's
            // not possible to add pages in a non-linear order), so we need some own maintenance data

        bool                            m_bAutoNextButtonState;

        bool                            m_bTravelingSuspended;

        WizardMachineImplData()
            :nFirstUnknownPage( 0 )
            ,m_bAutoNextButtonState( false )
            ,m_bTravelingSuspended( false )
        {
        }
    };

    OWizardMachine::OWizardMachine(vcl::Window* _pParent, WizardButtonFlags _nButtonFlags )
        :WizardDialog( _pParent, "WizardDialog", "svt/ui/wizarddialog.ui" )
        ,m_pFinish(nullptr)
        ,m_pCancel(nullptr)
        ,m_pNextPage(nullptr)
        ,m_pPrevPage(nullptr)
        ,m_pHelp(nullptr)
        ,m_pImpl( new WizardMachineImplData )
    {
        implConstruct( _nButtonFlags );
    }


    void OWizardMachine::implConstruct( const WizardButtonFlags _nButtonFlags )
    {
        m_pImpl->sTitleBase = GetText();

        // create the buttons according to the wizard button flags
        // the help button
        if (_nButtonFlags & WizardButtonFlags::HELP)
        {
            m_pHelp= VclPtr<HelpButton>::Create(this, WB_TABSTOP);
            m_pHelp->SetSizePixel(LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)));
            m_pHelp->Show();
            AddButton( m_pHelp, WIZARDDIALOG_BUTTON_STDOFFSET_X);
        }

        // the previous button
        if (_nButtonFlags & WizardButtonFlags::PREVIOUS)
        {
            m_pPrevPage = VclPtr<PushButton>::Create(this, WB_TABSTOP);
            m_pPrevPage->SetHelpId( HID_WIZARD_PREVIOUS );
            m_pPrevPage->SetSizePixel(LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)));
            m_pPrevPage->SetText(SvtResId(STR_WIZDLG_PREVIOUS));
            m_pPrevPage->Show();
            m_pPrevPage->set_id("previous");

            if (_nButtonFlags & WizardButtonFlags::NEXT)
                AddButton( m_pPrevPage, ( WIZARDDIALOG_BUTTON_SMALLSTDOFFSET_X) );      // half x-offset to the next button
            else
                AddButton( m_pPrevPage, WIZARDDIALOG_BUTTON_STDOFFSET_X );
            SetPrevButton( m_pPrevPage );
            m_pPrevPage->SetClickHdl( LINK( this, OWizardMachine, OnPrevPage ) );
        }

        // the next button
        if (_nButtonFlags & WizardButtonFlags::NEXT)
        {
            m_pNextPage = VclPtr<PushButton>::Create(this, WB_TABSTOP);
            m_pNextPage->SetHelpId( HID_WIZARD_NEXT );
            m_pNextPage->SetSizePixel(LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)));
            m_pNextPage->SetText(SvtResId(STR_WIZDLG_NEXT));
            m_pNextPage->Show();
            m_pNextPage->set_id("next");

            AddButton( m_pNextPage, WIZARDDIALOG_BUTTON_STDOFFSET_X );
            SetNextButton( m_pNextPage );
            m_pNextPage->SetClickHdl( LINK( this, OWizardMachine, OnNextPage ) );
        }

        // the finish button
        if (_nButtonFlags & WizardButtonFlags::FINISH)
        {
            m_pFinish = VclPtr<OKButton>::Create(this, WB_TABSTOP);
            m_pFinish->SetSizePixel(LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)));
            m_pFinish->SetText(SvtResId(STR_WIZDLG_FINISH));
            m_pFinish->Show();
            m_pFinish->set_id("finish");

            AddButton( m_pFinish, WIZARDDIALOG_BUTTON_STDOFFSET_X );
            m_pFinish->SetClickHdl( LINK( this, OWizardMachine, OnFinish ) );
        }

        // the cancel button
        if (_nButtonFlags & WizardButtonFlags::CANCEL)
        {
            m_pCancel = VclPtr<CancelButton>::Create(this, WB_TABSTOP);
            m_pCancel->SetSizePixel(LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)));
            m_pCancel->Show();

            AddButton( m_pCancel, WIZARDDIALOG_BUTTON_STDOFFSET_X );
        }
    }


    OWizardMachine::~OWizardMachine()
    {
        disposeOnce();
    }

    void OWizardMachine::dispose()
    {
        m_pFinish.disposeAndClear();
        m_pCancel.disposeAndClear();
        m_pNextPage.disposeAndClear();
        m_pPrevPage.disposeAndClear();
        m_pHelp.disposeAndClear();

        if (m_pImpl)
        {
            for (WizardState i = 0; i < m_pImpl->nFirstUnknownPage; ++i)
            {
                TabPage *pPage = GetPage(i);
                if (pPage)
                    pPage->disposeOnce();
            }
            m_pImpl.reset();
        }

        WizardDialog::dispose();
    }


    void OWizardMachine::implUpdateTitle()
    {
        OUString sCompleteTitle(m_pImpl->sTitleBase);

        // append the page title
        TabPage* pCurrentPage = GetPage(getCurrentState());
        if ( pCurrentPage && !pCurrentPage->GetText().isEmpty() )
        {
            sCompleteTitle += " - " + pCurrentPage->GetText();
        }

        SetText(sCompleteTitle);
    }


    void OWizardMachine::setTitleBase(const OUString& _rTitleBase)
    {
        m_pImpl->sTitleBase = _rTitleBase;
        implUpdateTitle();
    }


    TabPage* OWizardMachine::GetOrCreatePage( const WizardState i_nState )
    {
        if ( nullptr == GetPage( i_nState ) )
        {
            VclPtr<TabPage> pNewPage = createPage( i_nState );
            DBG_ASSERT( pNewPage, "OWizardMachine::GetOrCreatePage: invalid new page (NULL)!" );

            // fill up the page sequence of our base class (with dummies)
            while ( m_pImpl->nFirstUnknownPage < i_nState )
            {
                AddPage( nullptr );
                ++m_pImpl->nFirstUnknownPage;
            }

            if ( m_pImpl->nFirstUnknownPage == i_nState )
            {
                // encountered this page number the first time
                AddPage( pNewPage );
                ++m_pImpl->nFirstUnknownPage;
            }
            else
                // already had this page - just change it
                SetPage( i_nState, pNewPage );
        }
        return GetPage( i_nState );
    }


    void OWizardMachine::ActivatePage()
    {
        WizardDialog::ActivatePage();

        WizardState nCurrentLevel = GetCurLevel();
        GetOrCreatePage( nCurrentLevel );

        enterState( nCurrentLevel );
    }


    bool OWizardMachine::DeactivatePage()
    {
        WizardState nCurrentState = getCurrentState();
        return leaveState(nCurrentState) && WizardDialog::DeactivatePage();
    }


    void OWizardMachine::defaultButton(WizardButtonFlags _nWizardButtonFlags)
    {
        // the new default button
        PushButton* pNewDefButton = nullptr;
        if (m_pFinish && (_nWizardButtonFlags & WizardButtonFlags::FINISH))
            pNewDefButton = m_pFinish;
        if (m_pNextPage && (_nWizardButtonFlags & WizardButtonFlags::NEXT))
            pNewDefButton = m_pNextPage;
        if (m_pPrevPage && (_nWizardButtonFlags & WizardButtonFlags::PREVIOUS))
            pNewDefButton = m_pPrevPage;
        if (m_pHelp && (_nWizardButtonFlags & WizardButtonFlags::HELP))
            pNewDefButton = m_pHelp;
        if (m_pCancel && (_nWizardButtonFlags & WizardButtonFlags::CANCEL))
            pNewDefButton = m_pCancel;

        if ( pNewDefButton )
            defaultButton( pNewDefButton );
        else
            implResetDefault( this );
    }


    void OWizardMachine::implResetDefault(vcl::Window const * _pWindow)
    {
        vcl::Window* pChildLoop = _pWindow->GetWindow(GetWindowType::FirstChild);
        while (pChildLoop)
        {
            // does the window participate in the tabbing order?
            if (pChildLoop->GetStyle() & WB_DIALOGCONTROL)
                implResetDefault(pChildLoop);

            // is it a button?
            WindowType eType = pChildLoop->GetType();
            if  (   (WindowType::PUSHBUTTON == eType)
                ||  (WindowType::OKBUTTON == eType)
                ||  (WindowType::CANCELBUTTON == eType)
                ||  (WindowType::HELPBUTTON == eType)
                ||  (WindowType::IMAGEBUTTON == eType)
                ||  (WindowType::MENUBUTTON == eType)
                ||  (WindowType::MOREBUTTON == eType)
                )
            {
                pChildLoop->SetStyle(pChildLoop->GetStyle() & ~WB_DEFBUTTON);
            }

            // the next one ...
            pChildLoop = pChildLoop->GetWindow(GetWindowType::Next);
        }
    }


    void OWizardMachine::defaultButton(PushButton* _pNewDefButton)
    {
        // loop through all (direct and indirect) descendants which participate in our tabbing order, and
        // reset the WB_DEFBUTTON for every window which is a button
        implResetDefault(this);

        // set its new style
        if (_pNewDefButton)
            _pNewDefButton->SetStyle(_pNewDefButton->GetStyle() | WB_DEFBUTTON);
    }


    void OWizardMachine::enableButtons(WizardButtonFlags _nWizardButtonFlags, bool _bEnable)
    {
        if (m_pFinish && (_nWizardButtonFlags & WizardButtonFlags::FINISH))
            m_pFinish->Enable(_bEnable);
        if (m_pNextPage && (_nWizardButtonFlags & WizardButtonFlags::NEXT))
            m_pNextPage->Enable(_bEnable);
        if (m_pPrevPage && (_nWizardButtonFlags & WizardButtonFlags::PREVIOUS))
            m_pPrevPage->Enable(_bEnable);
        if (m_pHelp && (_nWizardButtonFlags & WizardButtonFlags::HELP))
            m_pHelp->Enable(_bEnable);
        if (m_pCancel && (_nWizardButtonFlags & WizardButtonFlags::CANCEL))
            m_pCancel->Enable(_bEnable);
    }


    void OWizardMachine::enterState(WizardState _nState)
    {
        // tell the page
        IWizardPageController* pController = getPageController( GetPage( _nState ) );
        OSL_ENSURE( pController, "OWizardMachine::enterState: no controller for the given page!" );
        if ( pController )
            pController->initializePage();

        if ( isAutomaticNextButtonStateEnabled() )
            enableButtons( WizardButtonFlags::NEXT, canAdvance() );

        enableButtons( WizardButtonFlags::PREVIOUS, !m_pImpl->aStateHistory.empty() );

        // set the new title - it depends on the current page (i.e. state)
        implUpdateTitle();
    }


    bool OWizardMachine::leaveState(WizardState)
    {
        // no need to ask the page here.
        // If we reach this point, we already gave the current page the chance to commit it's data,
        // and it was allowed to commit it's data

        return true;
    }


    bool OWizardMachine::onFinish()
    {
        return Finish( RET_OK );
    }


    IMPL_LINK_NOARG(OWizardMachine, OnFinish, Button*, void)
    {
        if ( isTravelingSuspended() )
            return;
        WizardTravelSuspension aTravelGuard( *this );
        if ( !prepareLeaveCurrentState( eFinish ) )
        {
            return;
        }
        onFinish();
    }


    OWizardMachine::WizardState OWizardMachine::determineNextState( WizardState _nCurrentState ) const
    {
        return _nCurrentState + 1;
    }


    bool OWizardMachine::prepareLeaveCurrentState( CommitPageReason _eReason )
    {
        IWizardPageController* pController = getPageController( GetPage( getCurrentState() ) );
        ENSURE_OR_RETURN( pController != nullptr, "OWizardMachine::prepareLeaveCurrentState: no controller for the current page!", true );
        return pController->commitPage( _eReason );
    }


    bool OWizardMachine::skipBackwardUntil( WizardState _nTargetState )
    {
        // allowed to leave the current page?
        if ( !prepareLeaveCurrentState( eTravelBackward ) )
            return false;

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
            return false;
        }
        return true;
    }


    bool OWizardMachine::skipUntil( WizardState _nTargetState )
    {
        WizardState nCurrentState = getCurrentState();

        // allowed to leave the current page?
        if ( !prepareLeaveCurrentState( nCurrentState < _nTargetState ? eTravelForward : eTravelBackward ) )
            return false;

        // don't travel directly on m_pImpl->aStateHistory, in case something goes wrong
        ::std::stack< WizardState > aTravelVirtually = m_pImpl->aStateHistory;
        ::std::stack< WizardState > aOldStateHistory = m_pImpl->aStateHistory;
        while ( nCurrentState != _nTargetState )
        {
            WizardState nNextState = determineNextState( nCurrentState );
            if ( WZS_INVALID_STATE == nNextState )
            {
                OSL_FAIL( "OWizardMachine::skipUntil: the given target state does not exist!" );
                return false;
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
            OSL_FAIL( "OWizardMachine::skipUntil: very unpolite ...." );
            m_pImpl->aStateHistory = aOldStateHistory;
            return false;
        }
        return true;
    }


    void OWizardMachine::skip()
    {
        // allowed to leave the current page?
        if ( !prepareLeaveCurrentState( eTravelForward ) )
            return;

        WizardState nCurrentState = getCurrentState();
        WizardState nNextState = determineNextState(nCurrentState);

        if (WZS_INVALID_STATE == nNextState)
            return;

        // remember the skipped state in the history
        m_pImpl->aStateHistory.push(nCurrentState);

        // get the next state
        nCurrentState = nNextState;

        // show the (n+1)th page
        if (!ShowPage(nCurrentState))
        {
            // TODO: this leaves us in a state where we have no current page and an inconsistent state history.
            // Perhaps we should rollback the skipping here ....
            OSL_FAIL("OWizardMachine::skip: very unpolite ....");
                // if somebody does a skip and then does not allow to leave ...
                // (can't be a commit error, as we've already committed the current page. So if ShowPage fails here,
                // somebody behaves really strange ...)
            return;
        }

        // all fine
    }


    bool OWizardMachine::travelNext()
    {
        // allowed to leave the current page?
        if ( !prepareLeaveCurrentState( eTravelForward ) )
            return false;

        // determine the next state to travel to
        WizardState nCurrentState = getCurrentState();
        WizardState nNextState = determineNextState(nCurrentState);
        if (WZS_INVALID_STATE == nNextState)
            return false;

        // the state history is used by the enterState method
        // all fine
        m_pImpl->aStateHistory.push(nCurrentState);
        if (!ShowPage(nNextState))
        {
            m_pImpl->aStateHistory.pop();
            return false;
        }

        return true;
    }


    bool OWizardMachine::travelPrevious()
    {
        DBG_ASSERT(!m_pImpl->aStateHistory.empty(), "OWizardMachine::travelPrevious: have no previous page!");

        // allowed to leave the current page?
        if ( !prepareLeaveCurrentState( eTravelBackward ) )
            return false;

        // the next state to switch to
        WizardState nPreviousState = m_pImpl->aStateHistory.top();

        // the state history is used by the enterState method
        m_pImpl->aStateHistory.pop();
        // show this page
        if (!ShowPage(nPreviousState))
        {
            m_pImpl->aStateHistory.push(nPreviousState);
            return false;
        }

        // all fine
        return true;
    }


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


    void OWizardMachine::enableAutomaticNextButtonState()
    {
        m_pImpl->m_bAutoNextButtonState = true;
    }


    bool OWizardMachine::isAutomaticNextButtonStateEnabled() const
    {
        return m_pImpl->m_bAutoNextButtonState;
    }


    IMPL_LINK_NOARG(OWizardMachine, OnPrevPage, Button*, void)
    {
        if ( isTravelingSuspended() )
            return;
        WizardTravelSuspension aTravelGuard( *this );
        travelPrevious();
    }


    IMPL_LINK_NOARG(OWizardMachine, OnNextPage, Button*, void)
    {
        if ( isTravelingSuspended() )
            return;
        WizardTravelSuspension aTravelGuard( *this );
        travelNext();
    }


    IWizardPageController* OWizardMachine::getPageController( TabPage* _pCurrentPage ) const
    {
        IWizardPageController* pController = dynamic_cast< IWizardPageController* >( _pCurrentPage );
        return pController;
    }


    void OWizardMachine::getStateHistory( ::std::vector< WizardState >& _out_rHistory )
    {
        ::std::stack< WizardState > aHistoryCopy( m_pImpl->aStateHistory );
        while ( !aHistoryCopy.empty() )
        {
            _out_rHistory.push_back( aHistoryCopy.top() );
            aHistoryCopy.pop();
        }
    }


    bool OWizardMachine::canAdvance() const
    {
        return WZS_INVALID_STATE != determineNextState( getCurrentState() );
    }


    void OWizardMachine::updateTravelUI()
    {
        const IWizardPageController* pController = getPageController( GetPage( getCurrentState() ) );
        OSL_ENSURE( pController != nullptr, "RoadmapWizard::updateTravelUI: no controller for the current page!" );

        bool bCanAdvance =
                ( !pController || pController->canAdvance() )   // the current page allows to advance
            &&  canAdvance();                                   // the dialog as a whole allows to advance
        enableButtons( WizardButtonFlags::NEXT, bCanAdvance );
    }


    bool OWizardMachine::isTravelingSuspended() const
    {
        return m_pImpl->m_bTravelingSuspended;
    }


    void OWizardMachine::suspendTraveling( AccessGuard )
    {
        DBG_ASSERT( !m_pImpl->m_bTravelingSuspended, "OWizardMachine::suspendTraveling: already suspended!" );
        m_pImpl->m_bTravelingSuspended = true;
    }


    void OWizardMachine::resumeTraveling( AccessGuard )
    {
        DBG_ASSERT( m_pImpl->m_bTravelingSuspended, "OWizardMachine::resumeTraveling: nothing to resume!" );
        m_pImpl->m_bTravelingSuspended = false;
    }


}   // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
