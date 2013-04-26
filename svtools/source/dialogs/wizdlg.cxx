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

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/tabpage.hxx>
#include <svtools/wizdlg.hxx>

// =======================================================================

#define WIZARDDIALOG_BUTTON_OFFSET_Y        6
#define WIZARDDIALOG_BUTTON_DLGOFFSET_X     6
#define WIZARDDIALOG_VIEW_DLGOFFSET_X       6
#define WIZARDDIALOG_VIEW_DLGOFFSET_Y       6

// =======================================================================

struct ImplWizPageData
{
    ImplWizPageData*    mpNext;
    TabPage*            mpPage;
};

// -----------------------------------------------------------------------

struct ImplWizButtonData
{
    ImplWizButtonData*  mpNext;
    Button*             mpButton;
    long                mnOffset;
};

// =======================================================================

void WizardDialog::ImplInitData()
{
    mpFirstPage     = NULL;
    mpFirstBtn      = NULL;
    mpFixedLine     = NULL;
    mpCurTabPage    = NULL;
    mpPrevBtn       = NULL;
    mpNextBtn       = NULL;
    mpViewWindow    = NULL;
    mnCurLevel      = 0;
    meViewAlign     = WINDOWALIGN_LEFT;
    mbEmptyViewMargin =  false;
    mnLeftAlignCount = 0;
}

// -----------------------------------------------------------------------
void WizardDialog::SetLeftAlignedButtonCount( sal_Int16 _nCount )
{
    mnLeftAlignCount = _nCount;
}

// -----------------------------------------------------------------------

void WizardDialog::SetEmptyViewMargin()
{
    mbEmptyViewMargin = true;
}

// -----------------------------------------------------------------------

void WizardDialog::ImplCalcSize( Size& rSize )
{
    // ButtonBar-Hoehe berechnen
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
    if ( mpFixedLine && mpFixedLine->IsVisible() )
        nMaxHeight += mpFixedLine->GetSizePixel().Height();
    rSize.Height() += nMaxHeight;

    // View-Window-Groesse dazurechnen
    if ( mpViewWindow && mpViewWindow->IsVisible() )
    {
        Size aViewSize = mpViewWindow->GetSizePixel();
        if ( meViewAlign == WINDOWALIGN_TOP )
            rSize.Height() += aViewSize.Height();
        else if ( meViewAlign == WINDOWALIGN_LEFT )
            rSize.Width() += aViewSize.Width();
        else if ( meViewAlign == WINDOWALIGN_BOTTOM )
            rSize.Height() += aViewSize.Height();
        else if ( meViewAlign == WINDOWALIGN_RIGHT )
            rSize.Width() += aViewSize.Width();
    }
}

// -----------------------------------------------------------------------

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

    if ( mpFixedLine && mpFixedLine->IsVisible() )
    {
        nOffY -= mpFixedLine->GetSizePixel().Height();
        mpFixedLine->setPosSizePixel( 0, nOffY, aDlgSize.Width(), 0,
                                      WINDOW_POSSIZE_POS | WINDOW_POSSIZE_WIDTH );
    }

    if ( mpViewWindow && mpViewWindow->IsVisible() )
    {
        long    nViewOffX = 0;
        long    nViewOffY = 0;
        long    nViewWidth = 0;
        long    nViewHeight = 0;
        long    nDlgHeight = nOffY;
        sal_uInt16  nViewPosFlags = WINDOW_POSSIZE_POS;
        if ( meViewAlign == WINDOWALIGN_TOP )
        {
            nViewOffX       = WIZARDDIALOG_VIEW_DLGOFFSET_X;
            nViewOffY       = WIZARDDIALOG_VIEW_DLGOFFSET_Y;
            nViewWidth      = aDlgSize.Width()-(WIZARDDIALOG_VIEW_DLGOFFSET_X*2);
            nViewPosFlags  |= WINDOW_POSSIZE_WIDTH;
        }
        else if ( meViewAlign == WINDOWALIGN_LEFT )
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
            nViewPosFlags  |= WINDOW_POSSIZE_HEIGHT;
        }
        else if ( meViewAlign == WINDOWALIGN_BOTTOM )
        {
            nViewOffX       = WIZARDDIALOG_VIEW_DLGOFFSET_X;
            nViewOffY       = nDlgHeight-mpViewWindow->GetSizePixel().Height()-WIZARDDIALOG_VIEW_DLGOFFSET_Y;
            nViewWidth      = aDlgSize.Width()-(WIZARDDIALOG_VIEW_DLGOFFSET_X*2);
            nViewPosFlags  |= WINDOW_POSSIZE_WIDTH;
        }
        else if ( meViewAlign == WINDOWALIGN_RIGHT )
        {
            nViewOffX       = aDlgSize.Width()-mpViewWindow->GetSizePixel().Width()-WIZARDDIALOG_VIEW_DLGOFFSET_X;
            nViewOffY       = WIZARDDIALOG_VIEW_DLGOFFSET_Y;
            nViewHeight     = nDlgHeight-(WIZARDDIALOG_VIEW_DLGOFFSET_Y*2);
            nViewPosFlags  |= WINDOW_POSSIZE_HEIGHT;
        }
        mpViewWindow->setPosSizePixel( nViewOffX, nViewOffY,
                                       nViewWidth, nViewHeight,
                                       nViewPosFlags );
    }
}


long WizardDialog::LogicalCoordinateToPixel(int iCoordinate){
    Size aLocSize = LogicToPixel(Size( iCoordinate, 0 ), MAP_APPFONT );
    int iPixelCoordinate =  aLocSize.Width();
    return iPixelCoordinate;
}


// -----------------------------------------------------------------------

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

    // ButtonBar-Hoehe berechnen
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
    if ( mpFixedLine && mpFixedLine->IsVisible() )
        nMaxHeight += mpFixedLine->GetSizePixel().Height();

    // TabPage positionieren
    Size aDlgSize = GetOutputSizePixel();
    aDlgSize.Height() -= nMaxHeight;
    long nOffX = 0;
    long nOffY = 0;
    if ( mpViewWindow && mpViewWindow->IsVisible() )
    {
        Size aViewSize = mpViewWindow->GetSizePixel();
        if ( meViewAlign == WINDOWALIGN_TOP )
        {
            nOffY += aViewSize.Height()+WIZARDDIALOG_VIEW_DLGOFFSET_Y;
            aDlgSize.Height() -= aViewSize.Height()+WIZARDDIALOG_VIEW_DLGOFFSET_Y;
        }
        else if ( meViewAlign == WINDOWALIGN_LEFT )
        {
            long nViewOffset = mbEmptyViewMargin ? 0 : WIZARDDIALOG_VIEW_DLGOFFSET_X;
            nOffX += aViewSize.Width() + nViewOffset;
            aDlgSize.Width() -= nOffX;
        }
        else if ( meViewAlign == WINDOWALIGN_BOTTOM )
            aDlgSize.Height() -= aViewSize.Height()+WIZARDDIALOG_VIEW_DLGOFFSET_Y;
        else if ( meViewAlign == WINDOWALIGN_RIGHT )
            aDlgSize.Width() -= aViewSize.Width()+WIZARDDIALOG_VIEW_DLGOFFSET_X;
    }
    Point aPos( nOffX, nOffY );
    mpCurTabPage->SetPosSizePixel( aPos, aDlgSize );
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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
    return NULL;
}

// =======================================================================

WizardDialog::WizardDialog( Window* pParent, WinBits nStyle ) :
    ModalDialog( pParent, nStyle )
{
    ImplInitData();
}

// -----------------------------------------------------------------------

WizardDialog::WizardDialog( Window* pParent, const ResId& rResId ) :
    ModalDialog( pParent, rResId )
{
    ImplInitData();
}

// -----------------------------------------------------------------------

WizardDialog::~WizardDialog()
{
    if ( mpFixedLine )
        delete mpFixedLine;

    // Remove all buttons
    while ( mpFirstBtn )
        RemoveButton( mpFirstBtn->mpButton );

    // Remove all pages
    while ( mpFirstPage )
        RemovePage( mpFirstPage->mpPage );
}

// -----------------------------------------------------------------------

void WizardDialog::Resize()
{
    if ( IsReallyShown() && !IsInInitShow() )
    {
        ImplPosCtrls();
        ImplPosTabPage();
    }

    Dialog::Resize();
}

// -----------------------------------------------------------------------

void WizardDialog::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_INITSHOW )
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
                            aDlgSize.Width() = aPageSize.Width();
                        if ( aPageSize.Height() > aDlgSize.Height() )
                            aDlgSize.Height() = aPageSize.Height();
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

// -----------------------------------------------------------------------

long WizardDialog::Notify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && mpPrevBtn && mpNextBtn )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        KeyCode         aKeyCode = pKEvt->GetKeyCode();
        sal_uInt16          nKeyCode = aKeyCode.GetCode();

        if ( aKeyCode.IsMod1() )
        {
            if ( aKeyCode.IsShift() || (nKeyCode == KEY_PAGEUP) )
            {
                if ( (nKeyCode == KEY_TAB) || (nKeyCode == KEY_PAGEUP) )
                {
                    if ( mpPrevBtn->IsVisible() &&
                         mpPrevBtn->IsEnabled() && mpPrevBtn->IsInputEnabled() )
                    {
                        mpPrevBtn->SetPressed( sal_True );
                        mpPrevBtn->SetPressed( sal_False );
                        mpPrevBtn->Click();
                    }
                    return sal_True;
                }
            }
            else
            {
                if ( (nKeyCode == KEY_TAB) || (nKeyCode == KEY_PAGEDOWN) )
                {
                    if ( mpNextBtn->IsVisible() &&
                         mpNextBtn->IsEnabled() && mpNextBtn->IsInputEnabled() )
                    {
                        mpNextBtn->SetPressed( sal_True );
                        mpNextBtn->SetPressed( sal_False );
                        mpNextBtn->Click();
                    }
                    return sal_True;
                }
            }
        }
    }

    return Dialog::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void WizardDialog::ActivatePage()
{
    maActivateHdl.Call( this );
}

// -----------------------------------------------------------------------

long WizardDialog::DeactivatePage()
{
    if ( maDeactivateHdl.IsSet() )
        return maDeactivateHdl.Call( this );
    else
        return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool WizardDialog::ShowNextPage()
{
    return ShowPage( mnCurLevel+1 );
}

// -----------------------------------------------------------------------

sal_Bool WizardDialog::ShowPrevPage()
{
    if ( !mnCurLevel )
        return sal_False;
    return ShowPage( mnCurLevel-1 );
}

// -----------------------------------------------------------------------

sal_Bool WizardDialog::ShowPage( sal_uInt16 nLevel )
{
    if ( DeactivatePage() )
    {
        mnCurLevel = nLevel;
        ActivatePage();
        ImplShowTabPage( ImplGetPage( mnCurLevel ) );
        return sal_True;
    }
    else
        return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool WizardDialog::Finnish( long nResult )
{
    if ( DeactivatePage() )
    {
        if ( mpCurTabPage )
            mpCurTabPage->DeactivatePage();

        if ( IsInExecute() )
            EndDialog( nResult );
        else if ( GetStyle() & WB_CLOSEABLE )
            Close();
        return sal_True;
    }
    else
        return sal_False;
}

// -----------------------------------------------------------------------

void WizardDialog::AddPage( TabPage* pPage )
{
    ImplWizPageData* pNewPageData = new ImplWizPageData;
    pNewPageData->mpNext    = NULL;
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

// -----------------------------------------------------------------------

void WizardDialog::RemovePage( TabPage* pPage )
{
    ImplWizPageData*  pPrevPageData = NULL;
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
                mpCurTabPage = NULL;
            delete pPageData;
            return;
        }

        pPrevPageData = pPageData;
        pPageData = pPageData->mpNext;
    }

    OSL_FAIL( "WizardDialog::RemovePage() - Page not in list" );
}

// -----------------------------------------------------------------------

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
            mpCurTabPage = NULL;
        pPageData->mpPage = pPage;
    }
}

// -----------------------------------------------------------------------

TabPage* WizardDialog::GetPage( sal_uInt16 nLevel ) const
{
    sal_uInt16              nTempLevel = 0;
    ImplWizPageData*    pPageData = mpFirstPage;
    while ( pPageData )
    {
        if ( nTempLevel == nLevel )
            return pPageData->mpPage;

        nTempLevel++;
        pPageData = pPageData->mpNext;
    }

    return NULL;
}

// -----------------------------------------------------------------------

void WizardDialog::AddButton( Button* pButton, long nOffset )
{
    ImplWizButtonData* pNewBtnData = new ImplWizButtonData;
    pNewBtnData->mpNext     = NULL;
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

// -----------------------------------------------------------------------

void WizardDialog::RemoveButton( Button* pButton )
{
    ImplWizButtonData*  pPrevBtnData = NULL;
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

// -----------------------------------------------------------------------

void WizardDialog::ShowButtonFixedLine( sal_Bool bVisible )
{
    if ( !mpFixedLine )
    {
        if ( !bVisible )
            return;

        mpFixedLine = new FixedLine( this );
    }

    mpFixedLine->Show( bVisible );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
