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

#include <osl/diagnose.h>
#include <vcl/event.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/tabpage.hxx>
#include <svtools/wizdlg.hxx>


#define WIZARDDIALOG_BUTTON_OFFSET_Y        6
#define WIZARDDIALOG_BUTTON_DLGOFFSET_X     6
#define WIZARDDIALOG_VIEW_DLGOFFSET_X       6
#define WIZARDDIALOG_VIEW_DLGOFFSET_Y       6


struct ImplWizPageData
{
    ImplWizPageData*    mpNext;
    VclPtr<TabPage>     mpPage;
};


struct ImplWizButtonData
{
    ImplWizButtonData*  mpNext;
    VclPtr<Button>      mpButton;
    long                mnOffset;
};


void WizardDialog::ImplInitData()
{
    mpFirstPage     = nullptr;
    mpFirstBtn      = nullptr;
    mpCurTabPage    = nullptr;
    mpPrevBtn       = nullptr;
    mpNextBtn       = nullptr;
    mpViewWindow    = nullptr;
    mnCurLevel      = 0;
    meViewAlign     = WindowAlign::Left;
    mbEmptyViewMargin =  false;
    mnLeftAlignCount = 0;

    maWizardLayoutIdle.SetPriority(TaskPriority::RESIZE);
    maWizardLayoutIdle.SetInvokeHandler( LINK( this, WizardDialog, ImplHandleWizardLayoutTimerHdl ) );
}


void WizardDialog::SetLeftAlignedButtonCount( sal_Int16 _nCount )
{
    mnLeftAlignCount = _nCount;
}


void WizardDialog::SetEmptyViewMargin()
{
    mbEmptyViewMargin = true;
}


void WizardDialog::ImplCalcSize( Size& rSize )
{
    // calculate ButtonBar height
    long                nMaxHeight = 0;
    ImplWizButtonData*  pBtnData = mpFirstBtn;
    while ( pBtnData )
    {
        long nBtnHeight = pBtnData->mpButton->GetSizePixel().Height();
        if ( nBtnHeight > nMaxHeight )
            nMaxHeight = nBtnHeight;
        pBtnData = pBtnData->mpNext;
    }
    if ( nMaxHeight )
        nMaxHeight += WIZARDDIALOG_BUTTON_OFFSET_Y*2;
    rSize.AdjustHeight(nMaxHeight );

    // add in the view window size
    if ( mpViewWindow && mpViewWindow->IsVisible() )
    {
        Size aViewSize = mpViewWindow->GetSizePixel();
        if ( meViewAlign == WindowAlign::Top )
            rSize.AdjustHeight(aViewSize.Height() );
        else if ( meViewAlign == WindowAlign::Left )
            rSize.AdjustWidth(aViewSize.Width() );
        else if ( meViewAlign == WindowAlign::Bottom )
            rSize.AdjustHeight(aViewSize.Height() );
        else if ( meViewAlign == WindowAlign::Right )
            rSize.AdjustWidth(aViewSize.Width() );
    }
}

void WizardDialog::queue_resize(StateChangedType /*eReason*/)
{
    if (maWizardLayoutIdle.IsActive())
        return;
    if (IsInClose())
        return;
    maWizardLayoutIdle.Start();
}

IMPL_LINK_NOARG( WizardDialog, ImplHandleWizardLayoutTimerHdl, Timer*, void )
{
    ImplPosCtrls();
    ImplPosTabPage();
}

void WizardDialog::ImplPosCtrls()
{
    Size    aDlgSize = GetOutputSizePixel();
    long    nBtnWidth = 0;
    long    nMaxHeight = 0;
    long    nOffY = aDlgSize.Height();

    ImplWizButtonData* pBtnData = mpFirstBtn;
    int j = 0;
    while ( pBtnData )
    {
        if (j >= mnLeftAlignCount)
        {
            Size aBtnSize = pBtnData->mpButton->GetSizePixel();
            long nBtnHeight = aBtnSize.Height();
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
        long nOffX = aDlgSize.Width()-nBtnWidth-WIZARDDIALOG_BUTTON_DLGOFFSET_X;
        long nOffLeftAlignX = LogicalCoordinateToPixel(6);
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

    long    nViewOffX = 0;
    long    nViewOffY = 0;
    long    nViewWidth = 0;
    long    nViewHeight = 0;
    long    nDlgHeight = nOffY;
    PosSizeFlags nViewPosFlags = PosSizeFlags::Pos;
    if ( meViewAlign == WindowAlign::Top )
    {
        nViewOffX       = WIZARDDIALOG_VIEW_DLGOFFSET_X;
        nViewOffY       = WIZARDDIALOG_VIEW_DLGOFFSET_Y;
        nViewWidth      = aDlgSize.Width()-(WIZARDDIALOG_VIEW_DLGOFFSET_X*2);
        nViewPosFlags  |= PosSizeFlags::Width;
    }
    else if ( meViewAlign == WindowAlign::Left )
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
    else if ( meViewAlign == WindowAlign::Bottom )
    {
        nViewOffX       = WIZARDDIALOG_VIEW_DLGOFFSET_X;
        nViewOffY       = nDlgHeight-mpViewWindow->GetSizePixel().Height()-WIZARDDIALOG_VIEW_DLGOFFSET_Y;
        nViewWidth      = aDlgSize.Width()-(WIZARDDIALOG_VIEW_DLGOFFSET_X*2);
        nViewPosFlags  |= PosSizeFlags::Width;
    }
    else if ( meViewAlign == WindowAlign::Right )
    {
        nViewOffX       = aDlgSize.Width()-mpViewWindow->GetSizePixel().Width()-WIZARDDIALOG_VIEW_DLGOFFSET_X;
        nViewOffY       = WIZARDDIALOG_VIEW_DLGOFFSET_Y;
        nViewHeight     = nDlgHeight-(WIZARDDIALOG_VIEW_DLGOFFSET_Y*2);
        nViewPosFlags  |= PosSizeFlags::Height;
    }
    mpViewWindow->setPosSizePixel( nViewOffX, nViewOffY,
                                   nViewWidth, nViewHeight,
                                   nViewPosFlags );
}


long WizardDialog::LogicalCoordinateToPixel(int iCoordinate){
    Size aLocSize = LogicToPixel(Size(iCoordinate, 0), MapMode(MapUnit::MapAppFont));
    int iPixelCoordinate =  aLocSize.Width();
    return iPixelCoordinate;
}


void WizardDialog::ImplPosTabPage()
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
    long                nMaxHeight = 0;
    ImplWizButtonData*  pBtnData = mpFirstBtn;
    while ( pBtnData )
    {
        long nBtnHeight = pBtnData->mpButton->GetSizePixel().Height();
        if ( nBtnHeight > nMaxHeight )
            nMaxHeight = nBtnHeight;
        pBtnData = pBtnData->mpNext;
    }
    if ( nMaxHeight )
        nMaxHeight += WIZARDDIALOG_BUTTON_OFFSET_Y*2;

    // position TabPage
    Size aDlgSize = GetOutputSizePixel();
    aDlgSize.AdjustHeight( -nMaxHeight );
    long nOffX = 0;
    long nOffY = 0;
    if ( mpViewWindow && mpViewWindow->IsVisible() )
    {
        Size aViewSize = mpViewWindow->GetSizePixel();
        if ( meViewAlign == WindowAlign::Top )
        {
            nOffY += aViewSize.Height()+WIZARDDIALOG_VIEW_DLGOFFSET_Y;
            aDlgSize.AdjustHeight( -(aViewSize.Height()+WIZARDDIALOG_VIEW_DLGOFFSET_Y) );
        }
        else if ( meViewAlign == WindowAlign::Left )
        {
            long nViewOffset = mbEmptyViewMargin ? 0 : WIZARDDIALOG_VIEW_DLGOFFSET_X;
            nOffX += aViewSize.Width() + nViewOffset;
            aDlgSize.AdjustWidth( -nOffX );
        }
        else if ( meViewAlign == WindowAlign::Bottom )
            aDlgSize.AdjustHeight( -(aViewSize.Height()+WIZARDDIALOG_VIEW_DLGOFFSET_Y) );
        else if ( meViewAlign == WindowAlign::Right )
            aDlgSize.AdjustWidth( -(aViewSize.Width()+WIZARDDIALOG_VIEW_DLGOFFSET_X) );
    }
    Point aPos( nOffX, nOffY );
    mpCurTabPage->SetPosSizePixel( aPos, aDlgSize );
}


void WizardDialog::ImplShowTabPage( TabPage* pTabPage )
{
    if ( mpCurTabPage == pTabPage )
        return;

    TabPage* pOldTabPage = mpCurTabPage;
    if ( pOldTabPage )
        pOldTabPage->DeactivatePage();

    mpCurTabPage = pTabPage;
    if ( pTabPage )
    {
        ImplPosTabPage();
        pTabPage->ActivatePage();
        pTabPage->Show();
    }

    if ( pOldTabPage )
        pOldTabPage->Hide();
}


TabPage* WizardDialog::ImplGetPage( sal_uInt16 nLevel ) const
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

WizardDialog::WizardDialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription ) :
    ModalDialog( pParent, rID, rUIXMLDescription )
{
    ImplInitData();
}

WizardDialog::~WizardDialog()
{
    disposeOnce();
}

void WizardDialog::dispose()
{
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
    ModalDialog::dispose();
}


void WizardDialog::Resize()
{
    if ( IsReallyShown() && !IsInInitShow() )
    {
        ImplPosCtrls();
        ImplPosTabPage();
    }

    Dialog::Resize();
}


void WizardDialog::StateChanged( StateChangedType nType )
{
    if ( nType == StateChangedType::InitShow )
    {
        if ( IsDefaultSize() )
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
            SetOutputSizePixel( aDlgSize );
        }

        ImplPosCtrls();
        ImplPosTabPage();
        ImplShowTabPage( ImplGetPage( mnCurLevel ) );
    }

    Dialog::StateChanged( nType );
}


bool WizardDialog::EventNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && mpPrevBtn && mpNextBtn )
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


void WizardDialog::ActivatePage()
{
    maActivateHdl.Call( this );
}


bool WizardDialog::DeactivatePage()
{
    return true;
}


bool WizardDialog::ShowNextPage()
{
    return ShowPage( mnCurLevel+1 );
}


bool WizardDialog::ShowPrevPage()
{
    if ( !mnCurLevel )
        return false;
    return ShowPage( mnCurLevel-1 );
}


bool WizardDialog::ShowPage( sal_uInt16 nLevel )
{
    if ( DeactivatePage() )
    {
        mnCurLevel = nLevel;
        ActivatePage();
        ImplShowTabPage( ImplGetPage( mnCurLevel ) );
        return true;
    }
    else
        return false;
}


bool WizardDialog::Finish( long nResult )
{
    if ( DeactivatePage() )
    {
        if ( mpCurTabPage )
            mpCurTabPage->DeactivatePage();

        if ( IsInExecute() )
            EndDialog( nResult );
        else if ( GetStyle() & WB_CLOSEABLE )
            Close();
        return true;
    }
    else
        return false;
}


void WizardDialog::AddPage( TabPage* pPage )
{
    ImplWizPageData* pNewPageData = new ImplWizPageData;
    pNewPageData->mpNext    = nullptr;
    pNewPageData->mpPage    = pPage;

    if ( !mpFirstPage )
        mpFirstPage = pNewPageData;
    else
    {
        ImplWizPageData* pPageData = mpFirstPage;
        while ( pPageData->mpNext )
            pPageData = pPageData->mpNext;
        pPageData->mpNext = pNewPageData;
    }
}


void WizardDialog::RemovePage( TabPage* pPage )
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

    OSL_FAIL( "WizardDialog::RemovePage() - Page not in list" );
}


void WizardDialog::SetPage( sal_uInt16 nLevel, TabPage* pPage )
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


TabPage* WizardDialog::GetPage( sal_uInt16 nLevel ) const
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


void WizardDialog::AddButton( Button* pButton, long nOffset )
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


void WizardDialog::RemoveButton( Button* pButton )
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

    OSL_FAIL( "WizardDialog::RemoveButton() - Button not in list" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
