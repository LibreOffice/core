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


#include <tools/debug.hxx>
#include <tools/json_writer.hxx>
#include <osl/diagnose.h>
#include <vcl/event.hxx>

#include <strings.hrc>
#include <svdata.hxx>
#include <wizdlg.hxx>

#include <vector>

#include "wizimpldata.hxx"
#include <uiobject-internal.hxx>

#define WIZARDDIALOG_BUTTON_OFFSET_Y        6
#define WIZARDDIALOG_BUTTON_DLGOFFSET_X     6
#define WIZARDDIALOG_BUTTON_STDOFFSET_X     6
#define WIZARDDIALOG_BUTTON_SMALLSTDOFFSET_X 3
#define WIZARDDIALOG_VIEW_DLGOFFSET_X       6
#define WIZARDDIALOG_VIEW_DLGOFFSET_Y       6

namespace vcl
{
    sal_Int32 RoadmapWizardImpl::getStateIndexInPath( WizardTypes::WizardState _nState, const WizardPath& _rPath )
    {
        sal_Int32 nStateIndexInPath = 0;
        for (auto const& path : _rPath)
        {
            if (path == _nState)
                return nStateIndexInPath;
            ++nStateIndexInPath;
        }
        return -1;
    }

    sal_Int32 RoadmapWizardImpl::getStateIndexInPath( WizardTypes::WizardState _nState, PathId _nPathId )
    {
        sal_Int32 nStateIndexInPath = -1;
        Paths::const_iterator aPathPos = aPaths.find( _nPathId );
        if ( aPathPos != aPaths.end( ) )
            nStateIndexInPath = getStateIndexInPath( _nState, aPathPos->second );
        return nStateIndexInPath;
    }

    sal_Int32 RoadmapWizardImpl::getFirstDifferentIndex( const WizardPath& _rLHS, const WizardPath& _rRHS )
    {
        sal_Int32 nMinLength = ::std::min( _rLHS.size(), _rRHS.size() );
        for ( sal_Int32 nCheck = 0; nCheck < nMinLength; ++nCheck )
        {
            if ( _rLHS[ nCheck ] != _rRHS[ nCheck ] )
                return nCheck;
        }
        return nMinLength;
    }

    //= RoadmapWizard
    RoadmapWizard::RoadmapWizard(vcl::Window* pParent, WinBits nStyle, InitFlag eFlag)
        : Dialog(pParent, nStyle, eFlag)
        , maWizardLayoutIdle("vcl RoadmapWizard maWizardLayoutIdle")
        , m_pFinish(nullptr)
        , m_pCancel(nullptr)
        , m_pNextPage(nullptr)
        , m_pPrevPage(nullptr)
        , m_pHelp(nullptr)
        , m_xWizardImpl(new WizardMachineImplData)
        , m_xRoadmapImpl(new RoadmapWizardImpl)
    {
        mpFirstPage     = nullptr;
        mpFirstBtn      = nullptr;
        mpCurTabPage    = nullptr;
        mpPrevBtn       = nullptr;
        mpNextBtn       = nullptr;
        mpViewWindow    = nullptr;
        mnCurLevel      = 0;
        mbEmptyViewMargin =  false;
        mnLeftAlignCount = 0;

        maWizardLayoutIdle.SetPriority(TaskPriority::RESIZE);
        maWizardLayoutIdle.SetInvokeHandler( LINK( this, RoadmapWizard, ImplHandleWizardLayoutTimerHdl ) );

        implConstruct();

        SetLeftAlignedButtonCount( 1 );
        mbEmptyViewMargin = true;

        m_xRoadmapImpl->pRoadmap.disposeAndReset( VclPtr<ORoadmap>::Create( this, WB_TABSTOP ) );
        m_xRoadmapImpl->pRoadmap->SetText( VclResId( STR_WIZDLG_ROADMAP_TITLE ) );
        m_xRoadmapImpl->pRoadmap->SetPosPixel( Point( 0, 0 ) );
        m_xRoadmapImpl->pRoadmap->SetItemSelectHdl( LINK( this, RoadmapWizard, OnRoadmapItemSelected ) );

        Size aRoadmapSize = LogicToPixel(Size(85, 0), MapMode(MapUnit::MapAppFont));
        aRoadmapSize.setHeight( GetSizePixel().Height() );
        m_xRoadmapImpl->pRoadmap->SetSizePixel( aRoadmapSize );

        mpViewWindow = m_xRoadmapImpl->pRoadmap;
        m_xRoadmapImpl->pRoadmap->Show();
    }

    void RoadmapWizard::ShowRoadmap(bool bShow)
    {
        m_xRoadmapImpl->pRoadmap->Show(bShow);
        CalcAndSetSize();
    }

    RoadmapWizard::~RoadmapWizard()
    {
        disposeOnce();
    }

    void RoadmapWizard::dispose()
    {
        m_xRoadmapImpl.reset();

        m_pFinish.disposeAndClear();
        m_pCancel.disposeAndClear();
        m_pNextPage.disposeAndClear();
        m_pPrevPage.disposeAndClear();
        m_pHelp.disposeAndClear();

        if (m_xWizardImpl)
        {
            for (WizardTypes::WizardState i = 0; i < m_xWizardImpl->nFirstUnknownPage; ++i)
            {
                TabPage *pPage = GetPage(i);
                if (pPage)
                    pPage->disposeOnce();
            }
            m_xWizardImpl.reset();
        }

        maWizardLayoutIdle.Stop();

        // Remove all buttons
        while ( mpFirstBtn )
            RemoveButton( mpFirstBtn->mpButton );

        // Remove all pages
        while ( mpFirstPage )
            RemovePage( mpFirstPage->mpPage );

        mpCurTabPage.clear();
        mpPrevBtn.clear();
        mpNextBtn.clear();
        mpViewWindow.clear();
        Dialog::dispose();
    }

    void RoadmapWizard::SetRoadmapHelpId( const OUString& _rId )
    {
        m_xRoadmapImpl->pRoadmap->SetHelpId( _rId );
    }

    void RoadmapWizard::SetRoadmapBitmap(const BitmapEx& rBmp)
    {
        m_xRoadmapImpl->pRoadmap->SetRoadmapBitmap(rBmp);
    }

    void RoadmapWizard::SetLeftAlignedButtonCount( sal_Int16 _nCount )
    {
        mnLeftAlignCount = _nCount;
    }

    void RoadmapWizard::ImplCalcSize( Size& rSize )
    {
        // calculate ButtonBar height and width
        tools::Long                nMaxHeight = 0;
        tools::Long                nBarWidth = WIZARDDIALOG_BUTTON_DLGOFFSET_X * 2 + LogicalCoordinateToPixel(6);
        ImplWizButtonData*  pBtnData = mpFirstBtn;
        while (pBtnData)
        {
            auto nBtnHeight = pBtnData->mpButton->GetSizePixel().Height();
            auto nBtnWidth = pBtnData->mpButton->GetSizePixel().Width();
            if (pBtnData->mpButton->IsVisible())
            {
                nBarWidth += nBtnWidth;
                nBarWidth += pBtnData->mnOffset;
            }
            if ( nBtnHeight > nMaxHeight )
                nMaxHeight = nBtnHeight;
            pBtnData = pBtnData->mpNext;
        }
        if ( nMaxHeight )
            nMaxHeight += WIZARDDIALOG_BUTTON_OFFSET_Y*2;
        rSize.AdjustHeight(nMaxHeight);

        // add in the view window size
        if ( mpViewWindow && mpViewWindow->IsVisible() )
        {
            Size aViewSize = mpViewWindow->GetSizePixel();
            // align left
            rSize.AdjustWidth(aViewSize.Width() );
        }

        if (nBarWidth > rSize.Width())
            rSize.setWidth(nBarWidth);
    }

    void RoadmapWizard::queue_resize(StateChangedType /*eReason*/)
    {
        if (maWizardLayoutIdle.IsActive())
            return;
        if (IsInClose())
            return;
        maWizardLayoutIdle.Start();
    }

    IMPL_LINK_NOARG(RoadmapWizard, ImplHandleWizardLayoutTimerHdl, Timer*, void)
    {
        ImplPosCtrls();
        ImplPosTabPage();
    }

    void RoadmapWizard::ImplPosCtrls()
    {
        Size    aDlgSize = GetOutputSizePixel();
        tools::Long    nBtnWidth = 0;
        tools::Long    nMaxHeight = 0;
        tools::Long    nOffY = aDlgSize.Height();

        ImplWizButtonData* pBtnData = mpFirstBtn;
        int j = 0;
        while ( pBtnData )
        {
            if (j >= mnLeftAlignCount)
            {
                Size aBtnSize = pBtnData->mpButton->GetSizePixel();
                tools::Long nBtnHeight = aBtnSize.Height();
                if ( nBtnHeight > nMaxHeight )
                    nMaxHeight = nBtnHeight;
                nBtnWidth += aBtnSize.Width();
                nBtnWidth += pBtnData->mnOffset;
            }
            pBtnData = pBtnData->mpNext;
            j++;
        }

        if ( nMaxHeight )
        {
            tools::Long nOffX = aDlgSize.Width()-nBtnWidth-WIZARDDIALOG_BUTTON_DLGOFFSET_X;
            tools::Long nOffLeftAlignX = LogicalCoordinateToPixel(6);
            nOffY -= WIZARDDIALOG_BUTTON_OFFSET_Y+nMaxHeight;

            pBtnData = mpFirstBtn;
            int i = 0;
            while ( pBtnData )
            {
                Size aBtnSize = pBtnData->mpButton->GetSizePixel();
                if (i >= mnLeftAlignCount)
                {
                    Point aPos( nOffX, nOffY+((nMaxHeight-aBtnSize.Height())/2) );
                    pBtnData->mpButton->SetPosPixel( aPos );
                    nOffX += aBtnSize.Width();
                    nOffX += pBtnData->mnOffset;
                }
                else
                {
                    Point aPos( nOffLeftAlignX, nOffY+((nMaxHeight-aBtnSize.Height())/2) );
                    pBtnData->mpButton->SetPosPixel( aPos );
                    nOffLeftAlignX += aBtnSize.Width();
                    nOffLeftAlignX += pBtnData->mnOffset;
                }

                pBtnData = pBtnData->mpNext;
                i++;
            }

            nOffY -= WIZARDDIALOG_BUTTON_OFFSET_Y;
        }

        if ( !(mpViewWindow && mpViewWindow->IsVisible()) )
            return;

        tools::Long    nViewOffX = 0;
        tools::Long    nViewOffY = 0;
        tools::Long    nViewWidth = 0;
        tools::Long    nViewHeight = 0;
        tools::Long    nDlgHeight = nOffY;
        PosSizeFlags nViewPosFlags = PosSizeFlags::Pos;
        // align left
        {
            if ( mbEmptyViewMargin )
            {
                nViewOffX       = 0;
                nViewOffY       = 0;
                nViewHeight     = nDlgHeight;
            }
            else
            {
                nViewOffX       = WIZARDDIALOG_VIEW_DLGOFFSET_X;
                nViewOffY       = WIZARDDIALOG_VIEW_DLGOFFSET_Y;
                nViewHeight     = nDlgHeight-(WIZARDDIALOG_VIEW_DLGOFFSET_Y*2);
            }
            nViewPosFlags  |= PosSizeFlags::Height;
        }
        mpViewWindow->setPosSizePixel( nViewOffX, nViewOffY,
                                      nViewWidth, nViewHeight,
                                      nViewPosFlags );
    }

    tools::Long RoadmapWizard::LogicalCoordinateToPixel(int iCoordinate) const
    {
        Size aLocSize = LogicToPixel(Size(iCoordinate, 0), MapMode(MapUnit::MapAppFont));
        int iPixelCoordinate =  aLocSize.Width();
        return iPixelCoordinate;
    }

    void RoadmapWizard::ImplPosTabPage()
    {
        if ( !mpCurTabPage )
            return;

        if ( !IsInInitShow() )
        {
            // #100199# - On Unix initial size is equal to screen size, on Windows
            // it's 0,0. One cannot calculate the size unless dialog is visible.
            if ( !IsReallyVisible() )
                return;
        }

        // calculate height of ButtonBar
        tools::Long                nMaxHeight = 0;
        ImplWizButtonData*  pBtnData = mpFirstBtn;
        while ( pBtnData )
        {
            tools::Long nBtnHeight = pBtnData->mpButton->GetSizePixel().Height();
            if ( nBtnHeight > nMaxHeight )
                nMaxHeight = nBtnHeight;
            pBtnData = pBtnData->mpNext;
        }
        if ( nMaxHeight )
            nMaxHeight += WIZARDDIALOG_BUTTON_OFFSET_Y*2;

        // position TabPage
        Size aDlgSize = GetOutputSizePixel();
        aDlgSize.AdjustHeight( -nMaxHeight );
        tools::Long nOffX = 0;
        tools::Long nOffY = 0;
        if ( mpViewWindow && mpViewWindow->IsVisible() )
        {
            Size aViewSize = mpViewWindow->GetSizePixel();
            // align left
            tools::Long nViewOffset = mbEmptyViewMargin ? 0 : WIZARDDIALOG_VIEW_DLGOFFSET_X;
            nOffX += aViewSize.Width() + nViewOffset;
            aDlgSize.AdjustWidth( -nOffX );
        }
        Point aPos( nOffX, nOffY );
        mpCurTabPage->SetPosSizePixel( aPos, aDlgSize );
    }

    void RoadmapWizard::ImplShowTabPage( TabPage* pTabPage )
    {
        if ( mpCurTabPage == pTabPage )
            return;

        TabPage* pOldTabPage = mpCurTabPage;

        mpCurTabPage = pTabPage;
        if ( pTabPage )
        {
            ImplPosTabPage();
            pTabPage->Show();
        }

        if ( pOldTabPage )
            pOldTabPage->Hide();
    }

    TabPage* RoadmapWizard::ImplGetPage( sal_uInt16 nLevel ) const
    {
        sal_uInt16              nTempLevel = 0;
        ImplWizPageData*    pPageData = mpFirstPage;
        while ( pPageData )
        {
            if ( (nTempLevel == nLevel) || !pPageData->mpNext )
                break;

            nTempLevel++;
            pPageData = pPageData->mpNext;
        }

        if ( pPageData )
            return pPageData->mpPage;
        return nullptr;
    }

    void RoadmapWizard::AddButtonResponse( Button* pButton, int response)
    {
        m_xRoadmapImpl->maResponses[pButton] = response;
    }

    void RoadmapWizard::implConstruct()
    {
        m_xWizardImpl->sTitleBase = GetText();

        // create buttons
        // the help button
        m_pHelp= VclPtr<HelpButton>::Create(this, WB_TABSTOP);
        m_pHelp->SetSizePixel(LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)));
        m_pHelp->Show();
        m_pHelp->set_id(u"help"_ustr);
        AddButtonResponse(m_pHelp, RET_HELP);
        AddButton( m_pHelp, WIZARDDIALOG_BUTTON_STDOFFSET_X);

        // the previous button
        m_pPrevPage = VclPtr<PushButton>::Create(this, WB_TABSTOP);
        m_pPrevPage->SetHelpId( HID_WIZARD_PREVIOUS );
        m_pPrevPage->SetSizePixel(LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)));
        m_pPrevPage->SetText(VclResId(STR_WIZDLG_PREVIOUS));
        m_pPrevPage->Show();
        m_pPrevPage->set_id(u"previous"_ustr);
        AddButton( m_pPrevPage, ( WIZARDDIALOG_BUTTON_SMALLSTDOFFSET_X) );      // half x-offset to the next button
        mpPrevBtn = m_pPrevPage;
        m_pPrevPage->SetClickHdl( LINK( this, RoadmapWizard, OnPrevPage ) );

        // the next button
        m_pNextPage = VclPtr<PushButton>::Create(this, WB_TABSTOP);
        m_pNextPage->SetHelpId( HID_WIZARD_NEXT );
        m_pNextPage->SetSizePixel(LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)));
        m_pNextPage->SetText(VclResId(STR_WIZDLG_NEXT));
        m_pNextPage->Show();
        m_pNextPage->set_id(u"next"_ustr);
        AddButton( m_pNextPage, WIZARDDIALOG_BUTTON_STDOFFSET_X );
        mpNextBtn = m_pNextPage;
        m_pNextPage->SetClickHdl( LINK( this, RoadmapWizard, OnNextPage ) );

        // the finish button
        m_pFinish = VclPtr<OKButton>::Create(this, WB_TABSTOP);
        m_pFinish->SetSizePixel(LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)));
        m_pFinish->SetText(VclResId(STR_WIZDLG_FINISH));
        m_pFinish->Show();
        m_pFinish->set_id(u"finish"_ustr);
        AddButton( m_pFinish, WIZARDDIALOG_BUTTON_STDOFFSET_X );
        m_pFinish->SetClickHdl( LINK( this, RoadmapWizard, OnFinish ) );

        // the cancel button
        m_pCancel = VclPtr<CancelButton>::Create(this, WB_TABSTOP);
        m_pCancel->SetSizePixel(LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)));
        m_pCancel->Show();
        AddButton( m_pCancel, WIZARDDIALOG_BUTTON_STDOFFSET_X );
    }

    void RoadmapWizard::Resize()
    {
        if ( IsReallyShown() && !IsInInitShow() )
        {
            ImplPosCtrls();
            ImplPosTabPage();
        }

        Dialog::Resize();
    }

    void RoadmapWizard::CalcAndSetSize()
    {
        Size aDlgSize = GetPageSizePixel();
        if ( !aDlgSize.Width() || !aDlgSize.Height() )
        {
            ImplWizPageData*  pPageData = mpFirstPage;
            while ( pPageData )
            {
                if ( pPageData->mpPage )
                {
                    Size aPageSize = pPageData->mpPage->GetSizePixel();
                    if ( aPageSize.Width() > aDlgSize.Width() )
                        aDlgSize.setWidth( aPageSize.Width() );
                    if ( aPageSize.Height() > aDlgSize.Height() )
                        aDlgSize.setHeight( aPageSize.Height() );
                }

                pPageData = pPageData->mpNext;
            }
        }
        ImplCalcSize( aDlgSize );
        SetMinOutputSizePixel( aDlgSize );
        SetOutputSizePixel( aDlgSize );
    }

    void RoadmapWizard::StateChanged( StateChangedType nType )
    {
        if ( nType == StateChangedType::InitShow )
        {
            if ( IsDefaultSize() )
            {
                CalcAndSetSize();
            }

            ImplPosCtrls();
            ImplPosTabPage();
            ImplShowTabPage( ImplGetPage( mnCurLevel ) );
        }

        Dialog::StateChanged( nType );
    }

    bool RoadmapWizard::EventNotify( NotifyEvent& rNEvt )
    {
        if ( (rNEvt.GetType() == NotifyEventType::KEYINPUT) && mpPrevBtn && mpNextBtn )
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
            vcl::KeyCode aKeyCode = pKEvt->GetKeyCode();
            sal_uInt16 nKeyCode = aKeyCode.GetCode();

            if ( aKeyCode.IsMod1() )
            {
                if ( aKeyCode.IsShift() || (nKeyCode == KEY_PAGEUP) )
                {
                    if ( (nKeyCode == KEY_TAB) || (nKeyCode == KEY_PAGEUP) )
                    {
                        if ( mpPrevBtn->IsVisible() &&
                            mpPrevBtn->IsEnabled() && mpPrevBtn->IsInputEnabled() )
                        {
                            mpPrevBtn->SetPressed( true );
                            mpPrevBtn->SetPressed( false );
                            mpPrevBtn->Click();
                        }
                        return true;
                    }
                }
                else
                {
                    if ( (nKeyCode == KEY_TAB) || (nKeyCode == KEY_PAGEDOWN) )
                    {
                        if ( mpNextBtn->IsVisible() &&
                            mpNextBtn->IsEnabled() && mpNextBtn->IsInputEnabled() )
                        {
                            mpNextBtn->SetPressed( true );
                            mpNextBtn->SetPressed( false );
                            mpNextBtn->Click();
                        }
                        return true;
                    }
                }
            }
        }

        return Dialog::EventNotify( rNEvt );
    }

    void RoadmapWizard::ShowPage(sal_uInt16 nLevel)
    {
        mnCurLevel = nLevel;

        // synchronize the roadmap
        implUpdateRoadmap();
        m_xRoadmapImpl->pRoadmap->SelectRoadmapItemByID(getCurrentState());

        ImplShowTabPage( ImplGetPage( mnCurLevel ) );
    }

    void RoadmapWizard::Finish( tools::Long nResult )
    {
        if ( IsInExecute() )
            EndDialog( nResult );
        else if ( GetStyle() & WB_CLOSEABLE )
            Close();
    }

    void RoadmapWizard::AddPage( TabPage* pPage )
    {
        ImplWizPageData* pNewPageData = new ImplWizPageData;
        pNewPageData->mpNext    = nullptr;
        pNewPageData->mpPage    = pPage;

        if ( !mpFirstPage )
            mpFirstPage = pNewPageData;
        else
        {
            pPage->Hide();
            ImplWizPageData* pPageData = mpFirstPage;
            while ( pPageData->mpNext )
                pPageData = pPageData->mpNext;
            pPageData->mpNext = pNewPageData;
        }
    }

    void RoadmapWizard::RemovePage( TabPage* pPage )
    {
        ImplWizPageData*  pPrevPageData = nullptr;
        ImplWizPageData*  pPageData = mpFirstPage;
        while ( pPageData )
        {
            if ( pPageData->mpPage == pPage )
            {
                if ( pPrevPageData )
                    pPrevPageData->mpNext = pPageData->mpNext;
                else
                    mpFirstPage = pPageData->mpNext;
                if ( pPage == mpCurTabPage )
                    mpCurTabPage = nullptr;
                delete pPageData;
                return;
            }

            pPrevPageData = pPageData;
            pPageData = pPageData->mpNext;
        }

        OSL_FAIL( "RoadmapWizard::RemovePage() - Page not in list" );
    }

    void RoadmapWizard::SetPage( sal_uInt16 nLevel, TabPage* pPage )
    {
        sal_uInt16              nTempLevel = 0;
        ImplWizPageData*    pPageData = mpFirstPage;
        while ( pPageData )
        {
            if ( (nTempLevel == nLevel) || !pPageData->mpNext )
                break;

            nTempLevel++;
            pPageData = pPageData->mpNext;
        }

        if ( pPageData )
        {
            if ( pPageData->mpPage == mpCurTabPage )
                mpCurTabPage = nullptr;
            pPageData->mpPage = pPage;
        }
    }

    TabPage* RoadmapWizard::GetPage( sal_uInt16 nLevel ) const
    {
        sal_uInt16 nTempLevel = 0;

        for (ImplWizPageData* pPageData = mpFirstPage; pPageData;
             pPageData = pPageData->mpNext)
        {
            if ( nTempLevel == nLevel )
                return pPageData->mpPage;
            nTempLevel++;
        }

        return nullptr;
    }

    void RoadmapWizard::AddButton( Button* pButton, tools::Long nOffset )
    {
        ImplWizButtonData* pNewBtnData = new ImplWizButtonData;
        pNewBtnData->mpNext     = nullptr;
        pNewBtnData->mpButton   = pButton;
        pNewBtnData->mnOffset   = nOffset;

        if ( !mpFirstBtn )
            mpFirstBtn = pNewBtnData;
        else
        {
            ImplWizButtonData* pBtnData = mpFirstBtn;
            while ( pBtnData->mpNext )
                pBtnData = pBtnData->mpNext;
            pBtnData->mpNext = pNewBtnData;
        }
    }

    void RoadmapWizard::RemoveButton( Button* pButton )
    {
        ImplWizButtonData*  pPrevBtnData = nullptr;
        ImplWizButtonData*  pBtnData = mpFirstBtn;
        while ( pBtnData )
        {
            if ( pBtnData->mpButton == pButton )
            {
                if ( pPrevBtnData )
                    pPrevBtnData->mpNext = pBtnData->mpNext;
                else
                    mpFirstBtn = pBtnData->mpNext;
                delete pBtnData;
                return;
            }

            pPrevBtnData = pBtnData;
            pBtnData = pBtnData->mpNext;
        }

        OSL_FAIL( "RoadmapWizard::RemoveButton() - Button not in list" );
    }

    IMPL_LINK_NOARG(RoadmapWizard, OnFinish, Button*, void)
    {
        if ( isTravelingSuspended() )
            return;
        RoadmapWizardTravelSuspension aTravelGuard( *this );
        Finish( RET_OK );
    }

    void RoadmapWizard::skipBackwardUntil(WizardTypes::WizardState _nTargetState)
    {
        // don't travel directly on m_xWizardImpl->aStateHistory, in case something goes wrong
        std::stack< WizardTypes::WizardState > aTravelVirtually = m_xWizardImpl->aStateHistory;

        WizardTypes::WizardState nCurrentRollbackState = getCurrentState();
        while ( nCurrentRollbackState != _nTargetState )
        {
            assert(!aTravelVirtually.empty() && "RoadmapWizard::skipBackwardUntil: this target state does not exist in the history!");
            nCurrentRollbackState = aTravelVirtually.top();
            aTravelVirtually.pop();
        }
        m_xWizardImpl->aStateHistory = std::move(aTravelVirtually);
        ShowPage(_nTargetState);
    }

    void RoadmapWizard::skipUntil(WizardTypes::WizardState _nTargetState)
    {
        WizardTypes::WizardState nCurrentState = getCurrentState();

        // don't travel directly on m_xWizardImpl->aStateHistory, in case something goes wrong
        std::stack< WizardTypes::WizardState > aTravelVirtually = m_xWizardImpl->aStateHistory;
        while ( nCurrentState != _nTargetState )
        {
            WizardTypes::WizardState nNextState = determineNextState( nCurrentState );
            assert(nNextState != WZS_INVALID_STATE && "RoadmapWizard::skipUntil: the given target state does not exist!");

            // remember the skipped state in the history
            aTravelVirtually.push( nCurrentState );

            // get the next state
            nCurrentState = nNextState;
        }
        m_xWizardImpl->aStateHistory = std::move(aTravelVirtually);
        // show the target page
        ShowPage(nCurrentState);
    }

    void RoadmapWizard::travelNext()
    {
        // determine the next state to travel to
        WizardTypes::WizardState nCurrentState = getCurrentState();
        WizardTypes::WizardState nNextState = determineNextState(nCurrentState);
        if (WZS_INVALID_STATE == nNextState)
            return;

        // the state history is used by the enterState method
        // all fine
        m_xWizardImpl->aStateHistory.push(nCurrentState);
        ShowPage(nNextState);
    }

    void RoadmapWizard::travelPrevious()
    {
        DBG_ASSERT(!m_xWizardImpl->aStateHistory.empty(), "RoadmapWizard::travelPrevious: have no previous page!");

        // the next state to switch to
        WizardTypes::WizardState nPreviousState = m_xWizardImpl->aStateHistory.top();

        // the state history is used by the enterState method
        m_xWizardImpl->aStateHistory.pop();
        // show this page
        ShowPage(nPreviousState);
    }

    void  RoadmapWizard::removePageFromHistory( WizardTypes::WizardState nToRemove )
    {

        std::stack< WizardTypes::WizardState > aTemp;
        while(!m_xWizardImpl->aStateHistory.empty())
        {
            WizardTypes::WizardState nPreviousState = m_xWizardImpl->aStateHistory.top();
            m_xWizardImpl->aStateHistory.pop();
            if(nPreviousState != nToRemove)
                aTemp.push( nPreviousState );
            else
                break;
        }
        while(!aTemp.empty())
        {
            m_xWizardImpl->aStateHistory.push( aTemp.top() );
            aTemp.pop();
        }
    }

    IMPL_LINK_NOARG(RoadmapWizard, OnPrevPage, Button*, void)
    {
        if ( isTravelingSuspended() )
            return;
        RoadmapWizardTravelSuspension aTravelGuard( *this );
        travelPrevious();
    }

    IMPL_LINK_NOARG(RoadmapWizard, OnNextPage, Button*, void)
    {
        if ( isTravelingSuspended() )
            return;
        RoadmapWizardTravelSuspension aTravelGuard( *this );
        travelNext();
    }

    bool RoadmapWizard::isTravelingSuspended() const
    {
        return m_xWizardImpl->m_bTravelingSuspended;
    }

    void RoadmapWizard::suspendTraveling( AccessGuard )
    {
        DBG_ASSERT( !m_xWizardImpl->m_bTravelingSuspended, "RoadmapWizard::suspendTraveling: already suspended!" );
        m_xWizardImpl->m_bTravelingSuspended = true;
    }

    void RoadmapWizard::resumeTraveling( AccessGuard )
    {
        DBG_ASSERT( m_xWizardImpl->m_bTravelingSuspended, "RoadmapWizard::resumeTraveling: nothing to resume!" );
        m_xWizardImpl->m_bTravelingSuspended = false;
    }

    void RoadmapWizard::implUpdateRoadmap( )
    {
        DBG_ASSERT( m_xRoadmapImpl->aPaths.find( m_xRoadmapImpl->nActivePath ) != m_xRoadmapImpl->aPaths.end(),
            "RoadmapWizard::implUpdateRoadmap: there is no such path!" );
        const WizardPath& rActivePath( m_xRoadmapImpl->aPaths[ m_xRoadmapImpl->nActivePath ] );

        sal_Int32 nCurrentStatePathIndex = RoadmapWizardImpl::getStateIndexInPath( getCurrentState(), rActivePath );
        if (nCurrentStatePathIndex < 0)
            return;
        assert(nCurrentStatePathIndex >= 0 && o3tl::make_unsigned(nCurrentStatePathIndex) < rActivePath.size());

        // determine up to which index (in the new path) we have to display the items
        RoadmapTypes::ItemIndex nUpperStepBoundary = static_cast<RoadmapTypes::ItemIndex>(rActivePath.size());
        bool bIncompletePath = false;
        if ( !m_xRoadmapImpl->bActivePathIsDefinite )
        {
            for (auto const& path : m_xRoadmapImpl->aPaths)
            {
                if ( path.first == m_xRoadmapImpl->nActivePath )
                    // it's the path we are just activating -> no need to check anything
                    continue;
                // the index from which on both paths differ
                sal_Int32 nDivergenceIndex = RoadmapWizardImpl::getFirstDifferentIndex( rActivePath, path.second );
                if ( nDivergenceIndex <= nCurrentStatePathIndex )
                    // they differ in an index which we have already left behind us
                    // -> this is no conflict anymore
                    continue;

                // the path conflicts with our new path -> don't activate the
                // *complete* new path, but only up to the step which is unambiguous
                nUpperStepBoundary = nDivergenceIndex;
                bIncompletePath = true;
            }
        }

        // now, we have to remove all items after nCurrentStatePathIndex, and insert the items from the active
        // path, up to (excluding) nUpperStepBoundary
        RoadmapTypes::ItemIndex nLoopUntil = ::std::max( nUpperStepBoundary, m_xRoadmapImpl->pRoadmap->GetItemCount() );
        for ( RoadmapTypes::ItemIndex nItemIndex = nCurrentStatePathIndex; nItemIndex < nLoopUntil; ++nItemIndex )
        {
            bool bExistentItem = ( nItemIndex < m_xRoadmapImpl->pRoadmap->GetItemCount() );
            bool bNeedItem = ( nItemIndex < nUpperStepBoundary );

            bool bInsertItem = false;
            if ( bExistentItem )
            {
                if ( !bNeedItem )
                {
                    while ( nItemIndex < m_xRoadmapImpl->pRoadmap->GetItemCount() )
                        m_xRoadmapImpl->pRoadmap->DeleteRoadmapItem( nItemIndex );
                    break;
                }
                else
                {
                    // there is an item with this index in the roadmap - does it match what is requested by
                    // the respective state in the active path?
                    RoadmapTypes::ItemId nPresentItemId = m_xRoadmapImpl->pRoadmap->GetItemID( nItemIndex );
                    WizardTypes::WizardState nRequiredState = rActivePath[ nItemIndex ];
                    if ( nPresentItemId != nRequiredState )
                    {
                        m_xRoadmapImpl->pRoadmap->DeleteRoadmapItem( nItemIndex );
                        bInsertItem = true;
                    }
                }
            }
            else
            {
                DBG_ASSERT( bNeedItem, "RoadmapWizard::implUpdateRoadmap: ehm - none needed, none present - why did the loop not terminate?" );
                bInsertItem = bNeedItem;
            }

            WizardTypes::WizardState nState( rActivePath[ nItemIndex ] );
            if ( bInsertItem )
            {
                m_xRoadmapImpl->pRoadmap->InsertRoadmapItem(
                    nItemIndex,
                    getStateDisplayName( nState ),
                    nState,
                    true
                );
            }

            const bool bEnable = m_xRoadmapImpl->aDisabledStates.find( nState ) == m_xRoadmapImpl->aDisabledStates.end();
            m_xRoadmapImpl->pRoadmap->EnableRoadmapItem( m_xRoadmapImpl->pRoadmap->GetItemID( nItemIndex ), bEnable );
        }

        m_xRoadmapImpl->pRoadmap->SetRoadmapComplete( !bIncompletePath );
    }

    WizardTypes::WizardState RoadmapWizard::determineNextState( WizardTypes::WizardState _nCurrentState ) const
    {
        sal_Int32 nCurrentStatePathIndex = -1;

        Paths::const_iterator aActivePathPos = m_xRoadmapImpl->aPaths.find( m_xRoadmapImpl->nActivePath );
        if ( aActivePathPos != m_xRoadmapImpl->aPaths.end() )
            nCurrentStatePathIndex = RoadmapWizardImpl::getStateIndexInPath( _nCurrentState, aActivePathPos->second );

        DBG_ASSERT( nCurrentStatePathIndex != -1, "RoadmapWizard::determineNextState: ehm - how can we travel if there is no (valid) active path?" );
        if (nCurrentStatePathIndex < 0)
            return WZS_INVALID_STATE;
        assert(nCurrentStatePathIndex >= 0 && o3tl::make_unsigned(nCurrentStatePathIndex) < aActivePathPos->second.size());

        sal_Int32 nNextStateIndex = nCurrentStatePathIndex + 1;

        while   (   ( nNextStateIndex < static_cast<sal_Int32>(aActivePathPos->second.size()) )
                &&  ( m_xRoadmapImpl->aDisabledStates.find( aActivePathPos->second[ nNextStateIndex ] ) != m_xRoadmapImpl->aDisabledStates.end() )
                )
        {
            ++nNextStateIndex;
        }

        if ( nNextStateIndex >= static_cast<sal_Int32>(aActivePathPos->second.size()) )
            // there is no next state in the current path (at least none which is enabled)
            return WZS_INVALID_STATE;

        return aActivePathPos->second[ nNextStateIndex ];
    }


    IMPL_LINK_NOARG(RoadmapWizard, OnRoadmapItemSelected, LinkParamNone*, void)
    {
        RoadmapTypes::ItemId nCurItemId = m_xRoadmapImpl->pRoadmap->GetCurrentRoadmapItemID();
        if ( nCurItemId == getCurrentState() )
            // nothing to do
            return;

        if ( isTravelingSuspended() )
            return;

        RoadmapWizardTravelSuspension aTravelGuard( *this );

        sal_Int32 nCurrentIndex = m_xRoadmapImpl->getStateIndexInPath( getCurrentState(), m_xRoadmapImpl->nActivePath );
        sal_Int32 nNewIndex     = m_xRoadmapImpl->getStateIndexInPath( nCurItemId, m_xRoadmapImpl->nActivePath );

        DBG_ASSERT( ( nCurrentIndex != -1 ) && ( nNewIndex != -1 ),
            "RoadmapWizard::OnRoadmapItemSelected: something's wrong here!" );
        if ( ( nCurrentIndex == -1 ) || ( nNewIndex == -1 ) )
        {
            return;
        }

        if ( nNewIndex > nCurrentIndex )
        {
            skipUntil(static_cast<WizardTypes::WizardState>(nCurItemId));
            WizardTypes::WizardState nTemp = static_cast<WizardTypes::WizardState>(nCurItemId);
            while( nTemp )
            {
                if( m_xRoadmapImpl->aDisabledStates.find( --nTemp ) != m_xRoadmapImpl->aDisabledStates.end() )
                    removePageFromHistory( nTemp );
            }
        }
        else
            skipBackwardUntil(static_cast<WizardTypes::WizardState>(nCurItemId));
    }

    OUString RoadmapWizard::getStateDisplayName( WizardTypes::WizardState /* _nState */)
    {
        SAL_WARN("vcl", "RoadmapWizard::getStateDisplayName: no name available for this state!");
        return OUString();
    }

    void RoadmapWizard::InsertRoadmapItem(int nItemIndex, const OUString& rText, int nItemId, bool bEnable)
    {
        m_xRoadmapImpl->pRoadmap->InsertRoadmapItem(nItemIndex, rText, nItemId, bEnable);
    }

    void RoadmapWizard::SelectRoadmapItemByID(int nItemId, bool bGrabFocus)
    {
        m_xRoadmapImpl->pRoadmap->SelectRoadmapItemByID(nItemId, bGrabFocus);
    }

    void RoadmapWizard::DeleteRoadmapItems()
    {
        while (m_xRoadmapImpl->pRoadmap->GetItemCount())
            m_xRoadmapImpl->pRoadmap->DeleteRoadmapItem(0);
    }

    void RoadmapWizard::SetItemSelectHdl( const Link<LinkParamNone*,void>& _rHdl )
    {
        m_xRoadmapImpl->pRoadmap->SetItemSelectHdl(_rHdl);
    }

    int RoadmapWizard::GetCurrentRoadmapItemID() const
    {
        return m_xRoadmapImpl->pRoadmap->GetCurrentRoadmapItemID();
    }

    FactoryFunction RoadmapWizard::GetUITestFactory() const
    {
        return RoadmapWizardUIObject::create;
    }

    namespace
    {
        bool isButton(WindowType eType)
        {
            return eType == WindowType::PUSHBUTTON || eType == WindowType::OKBUTTON
                || eType == WindowType::CANCELBUTTON || eType == WindowType::HELPBUTTON;
        }
    }

    void RoadmapWizard::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
    {
        rJsonWriter.put("id", get_id());
        rJsonWriter.put("type", "dialog");
        rJsonWriter.put("title", GetText());

        OUString sDialogId = GetHelpId();
        sal_Int32 nStartPos = sDialogId.lastIndexOf('/');
        nStartPos = nStartPos >= 0 ? nStartPos + 1 : 0;
        rJsonWriter.put("dialogid", sDialogId.copy(nStartPos));
        {
            auto aResponses = rJsonWriter.startArray("responses");
            for (const auto& rResponse : m_xRoadmapImpl->maResponses)
            {
                auto aResponse = rJsonWriter.startStruct();
                rJsonWriter.put("id", rResponse.first->get_id());
                rJsonWriter.put("response", rResponse.second);
            }
        }

        vcl::Window* pFocusControl = GetFirstControlForFocus();
        if (pFocusControl)
            rJsonWriter.put("init_focus_id", pFocusControl->get_id());

        {
            auto childrenNode = rJsonWriter.startArray("children");

            auto containerNode = rJsonWriter.startStruct();
            rJsonWriter.put("id", "container");
            rJsonWriter.put("type", "container");
            rJsonWriter.put("vertical", true);

            {
                auto containerChildrenNode = rJsonWriter.startArray("children");

                // tabpages
                for (int i = 0; i < GetChildCount(); i++)
                {
                    vcl::Window* pChild = GetChild(i);

                    if (!isButton(pChild->GetType()) && pChild != mpViewWindow)
                    {
                        auto childNode = rJsonWriter.startStruct();
                        pChild->DumpAsPropertyTree(rJsonWriter);
                    }
                }

                // buttons
                {
                    auto buttonsNode = rJsonWriter.startStruct();
                    rJsonWriter.put("id", "buttons");
                    rJsonWriter.put("type", "buttonbox");
                    rJsonWriter.put("layoutstyle", "end");
                    {
                        auto buttonsChildrenNode = rJsonWriter.startArray("children");
                        for (int i = 0; i < GetChildCount(); i++)
                        {
                            vcl::Window* pChild = GetChild(i);

                            if (isButton(pChild->GetType()))
                            {
                                auto childNode = rJsonWriter.startStruct();
                                pChild->DumpAsPropertyTree(rJsonWriter);
                            }
                        }
                    }
                }
            }
        }
    }

}   // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
