/*************************************************************************
 *
 *  $RCSfile: wizdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:58 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SVT_WIZDLG_CXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _VCL_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _VCL_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _VCL_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif

#ifndef _SVT_WIZDLG_HXX
#include <wizdlg.hxx>
#endif

// =======================================================================

#define WIZARDDIALOG_BUTTON_OFFSET_Y        6
#define WIZARDDIALOG_BUTTON_DLGOFFSET_X     6
#define WIZARDDIALOG_LINE_DLGOFFSET_X       6
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
    while ( pBtnData )
    {
        Size aBtnSize = pBtnData->mpButton->GetSizePixel();
        long nBtnHeight = aBtnSize.Height();
        if ( nBtnHeight > nMaxHeight )
            nMaxHeight = nBtnHeight;
        nBtnWidth += aBtnSize.Width();
        nBtnWidth += pBtnData->mnOffset;
        pBtnData = pBtnData->mpNext;
    }

    if ( nMaxHeight )
    {
        long nOffX = aDlgSize.Width()-nBtnWidth-WIZARDDIALOG_BUTTON_DLGOFFSET_X;
        nOffY -= WIZARDDIALOG_BUTTON_OFFSET_Y+nMaxHeight;

        pBtnData = mpFirstBtn;
        while ( pBtnData )
        {
            Size aBtnSize = pBtnData->mpButton->GetSizePixel();
            Point aPos( nOffX, nOffY+((nMaxHeight-aBtnSize.Height())/2) );
            pBtnData->mpButton->SetPosPixel( aPos );
            nOffX += aBtnSize.Width();
            nOffX += pBtnData->mnOffset;
            pBtnData = pBtnData->mpNext;
        }

        nOffY -= WIZARDDIALOG_BUTTON_OFFSET_Y;
    }

    if ( mpFixedLine && mpFixedLine->IsVisible() )
    {
        nOffY -= mpFixedLine->GetSizePixel().Height();
        mpFixedLine->SetPosSizePixel( WIZARDDIALOG_LINE_DLGOFFSET_X, nOffY,
                                      aDlgSize.Width()-(WIZARDDIALOG_LINE_DLGOFFSET_X*2), 0,
                                      WINDOW_POSSIZE_POS | WINDOW_POSSIZE_WIDTH );
    }

    if ( mpViewWindow && mpViewWindow->IsVisible() )
    {
        long    nViewOffX = 0;
        long    nViewOffY = 0;
        long    nViewWidth = 0;
        long    nViewHeight = 0;
        long    nDlgHeight = nOffY;
        USHORT  nViewPosFlags = WINDOW_POSSIZE_POS;
        if ( meViewAlign == WINDOWALIGN_TOP )
        {
            nViewOffX       = WIZARDDIALOG_VIEW_DLGOFFSET_X;
            nViewOffY       = WIZARDDIALOG_VIEW_DLGOFFSET_Y;
            nViewWidth      = aDlgSize.Width()-(WIZARDDIALOG_VIEW_DLGOFFSET_X*2);
            nViewPosFlags  |= WINDOW_POSSIZE_WIDTH;
        }
        else if ( meViewAlign == WINDOWALIGN_LEFT )
        {
            nViewOffX       = WIZARDDIALOG_VIEW_DLGOFFSET_X;
            nViewOffY       = WIZARDDIALOG_VIEW_DLGOFFSET_Y;
            nViewHeight     = nDlgHeight-(WIZARDDIALOG_VIEW_DLGOFFSET_Y*2);
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
        mpViewWindow->SetPosSizePixel( nViewOffX, nViewOffY,
                                       nViewWidth, nViewHeight,
                                       nViewPosFlags );
    }
}

// -----------------------------------------------------------------------

void WizardDialog::ImplPosTabPage()
{
    if ( !mpCurTabPage )
        return;

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
            nOffX += aViewSize.Width()+WIZARDDIALOG_VIEW_DLGOFFSET_X;
            aDlgSize.Width() -= aViewSize.Width()+WIZARDDIALOG_VIEW_DLGOFFSET_X;
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

TabPage* WizardDialog::ImplGetPage( USHORT nLevel ) const
{
    USHORT              nTempLevel = 0;
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
        USHORT          nKeyCode = aKeyCode.GetCode();

        if ( aKeyCode.IsMod1() )
        {
            if ( aKeyCode.IsShift() || (nKeyCode == KEY_PAGEUP) )
            {
                if ( (nKeyCode == KEY_TAB) || (nKeyCode == KEY_PAGEUP) )
                {
                    if ( mpPrevBtn->IsVisible() &&
                         mpPrevBtn->IsEnabled() && mpPrevBtn->IsInputEnabled() )
                    {
                        mpPrevBtn->SetPressed( TRUE );
                        mpPrevBtn->SetPressed( FALSE );
                        mpPrevBtn->Click();
                    }
                    return TRUE;
                }
            }
            else
            {
                if ( (nKeyCode == KEY_TAB) || (nKeyCode == KEY_PAGEDOWN) )
                {
                    if ( mpNextBtn->IsVisible() &&
                         mpNextBtn->IsEnabled() && mpNextBtn->IsInputEnabled() )
                    {
                        mpNextBtn->SetPressed( TRUE );
                        mpNextBtn->SetPressed( FALSE );
                        mpNextBtn->Click();
                    }
                    return TRUE;
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
        return TRUE;
}

// -----------------------------------------------------------------------

BOOL WizardDialog::ShowNextPage()
{
    return ShowPage( mnCurLevel+1 );
}

// -----------------------------------------------------------------------

BOOL WizardDialog::ShowPrevPage()
{
    if ( !mnCurLevel )
        return FALSE;
    return ShowPage( mnCurLevel-1 );
}

// -----------------------------------------------------------------------

BOOL WizardDialog::ShowPage( USHORT nLevel )
{
    if ( DeactivatePage() )
    {
        mnCurLevel = nLevel;
        ActivatePage();
        ImplShowTabPage( ImplGetPage( mnCurLevel ) );
        return TRUE;
    }
    else
        return FALSE;
}

// -----------------------------------------------------------------------

BOOL WizardDialog::Finnish( long nResult )
{
    if ( DeactivatePage() )
    {
        if ( mpCurTabPage )
            mpCurTabPage->DeactivatePage();

        if ( IsInExecute() )
            EndDialog( nResult );
        else if ( GetStyle() & WB_CLOSEABLE )
            Close();
        return TRUE;
    }
    else
        return FALSE;
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

    DBG_ERROR( "WizardDialog::RemovePage() - Page not in list" );
}

// -----------------------------------------------------------------------

void WizardDialog::SetPage( USHORT nLevel, TabPage* pPage )
{
    USHORT              nTempLevel = 0;
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

TabPage* WizardDialog::GetPage( USHORT nLevel ) const
{
    USHORT              nTempLevel = 0;
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

    DBG_ERROR( "WizardDialog::RemoveButton() - Button not in list" );
}

// -----------------------------------------------------------------------

void WizardDialog::ShowButtonFixedLine( BOOL bVisible )
{
    if ( !mpFixedLine )
    {
        if ( !bVisible )
            return;

        mpFixedLine = new FixedLine( this );
    }

    mpFixedLine->Show( bVisible );
}

// -----------------------------------------------------------------------

BOOL WizardDialog::IsButtonFixedLineVisible()
{
    return (mpFixedLine && mpFixedLine->IsVisible());
}
