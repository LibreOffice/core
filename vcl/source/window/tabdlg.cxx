/*************************************************************************
 *
 *  $RCSfile: tabdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
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

#define _SV_TABDLG_CXX

#ifndef _SV_FIXED_HXX
#include <fixed.hxx>
#endif
#ifndef _SV_TABCTRL_HXX
#include <tabctrl.hxx>
#endif
#ifndef _SV_TABDLG_HXX
#include <tabdlg.hxx>
#endif
#ifndef _SV_RC_H
#include <rc.h>
#endif

#pragma hdrstop

// =======================================================================

void TabDialog::ImplInitData()
{
    mpFixedLine     = NULL;
    mpViewWindow    = NULL;
    meViewAlign     = WINDOWALIGN_LEFT;
    mbPosControls   = TRUE;
}

// -----------------------------------------------------------------------

void TabDialog::ImplPosControls()
{
    Size        aCtrlSize( IMPL_MINSIZE_BUTTON_WIDTH, IMPL_MINSIZE_BUTTON_HEIGHT );
    long        nDownCtrl = 0;
    long        nOffY = 0;
    TabControl* pTabControl = NULL;

    Window* pChild = GetWindow( WINDOW_FIRSTCHILD );
    while ( pChild )
    {
        if ( pChild->IsVisible() && (pChild != mpViewWindow) )
        {
            if ( pChild->GetType() == WINDOW_TABCONTROL )
                pTabControl = (TabControl*)pChild;
            else if ( pTabControl )
            {
                long nTxtWidth = pChild->GetCtrlTextWidth( pChild->GetText() );
                nTxtWidth += IMPL_EXTRA_BUTTON_WIDTH;
                if ( nTxtWidth > aCtrlSize.Width() )
                    aCtrlSize.Width() = nTxtWidth;
                long nTxtHeight = pChild->GetTextHeight();
                nTxtHeight += IMPL_EXTRA_BUTTON_HEIGHT;
                if ( nTxtHeight > aCtrlSize.Height() )
                    aCtrlSize.Height() = nTxtHeight;
                nDownCtrl++;
            }
            else
            {
                long nHeight = pChild->GetSizePixel().Height();
                if ( nHeight > nOffY )
                    nOffY = nHeight;
            }
        }

        pChild = pChild->GetWindow( WINDOW_NEXT );
    }

    // Haben wir ueberhaupt ein TabControl
    if ( pTabControl )
    {
        // Offset bei weiteren Controls um einen weiteren Abstand anpassen
        if ( nOffY )
            nOffY += IMPL_DIALOG_BAR_OFFSET*2 + 2;

        Point   aTabOffset( IMPL_DIALOG_OFFSET, IMPL_DIALOG_OFFSET+nOffY );
        Size    aTabSize = pTabControl->GetSizePixel();
        Size    aDlgSize( aTabSize.Width() + IMPL_DIALOG_OFFSET*2,
                          aTabSize.Height() + IMPL_DIALOG_OFFSET*2 + nOffY );
        long    nBtnEx = 0;

        // Preview-Fenster beruecksichtigen und die Groessen/Offsets anpassen
        if ( mpViewWindow && mpViewWindow->IsVisible() )
        {
            long    nViewOffX = 0;
            long    nViewOffY = 0;
            long    nViewWidth = 0;
            long    nViewHeight = 0;
            USHORT  nViewPosFlags = WINDOW_POSSIZE_POS;
            Size    aViewSize = mpViewWindow->GetSizePixel();
            if (  meViewAlign == WINDOWALIGN_TOP )
            {
                nViewOffX       = aTabOffset.X();
                nViewOffY       = nOffY+IMPL_DIALOG_OFFSET;
                nViewWidth      = aTabSize.Width();
                nViewPosFlags  |= WINDOW_POSSIZE_WIDTH;
                aTabOffset.Y() += aViewSize.Height()+IMPL_DIALOG_OFFSET;
                aDlgSize.Height() += aViewSize.Height()+IMPL_DIALOG_OFFSET;
            }
            else if (  meViewAlign == WINDOWALIGN_BOTTOM )
            {
                nViewOffX       = aTabOffset.X();
                nViewOffY       = aTabOffset.Y()+aTabSize.Height()+IMPL_DIALOG_OFFSET;
                nViewWidth      = aTabSize.Width();
                nViewPosFlags  |= WINDOW_POSSIZE_WIDTH;
                aDlgSize.Height() += aViewSize.Height()+IMPL_DIALOG_OFFSET;
            }
            else if (  meViewAlign == WINDOWALIGN_RIGHT )
            {
                nViewOffX       = aTabOffset.X()+aTabSize.Width()+IMPL_DIALOG_OFFSET;
                nViewOffY       = aTabOffset.Y();
                nViewHeight     = aTabSize.Height();
                nViewPosFlags  |= WINDOW_POSSIZE_HEIGHT;
                aDlgSize.Width() += aViewSize.Width()+IMPL_DIALOG_OFFSET;
                nBtnEx          = aViewSize.Width()+IMPL_DIALOG_OFFSET;
            }
            else // meViewAlign == WINDOWALIGN_LEFT
            {
                nViewOffX       = IMPL_DIALOG_OFFSET;
                nViewOffY       = aTabOffset.Y();
                nViewHeight     = aTabSize.Height();
                nViewPosFlags  |= WINDOW_POSSIZE_HEIGHT;
                aTabOffset.X() += aViewSize.Width()+IMPL_DIALOG_OFFSET;
                aDlgSize.Width() += aViewSize.Width()+IMPL_DIALOG_OFFSET;
                nBtnEx          = aViewSize.Width()+IMPL_DIALOG_OFFSET;
            }

            mpViewWindow->SetPosSizePixel( nViewOffX, nViewOffY,
                                           nViewWidth, nViewHeight,
                                           nViewPosFlags );
        }

        // Positionierung vornehmen
        pTabControl->SetPosPixel( aTabOffset );

        // Alle anderen Childs positionieren
        BOOL bTabCtrl   = FALSE;
        int  nLines     = 0;
        long nX;
        long nY         = aDlgSize.Height();
        long nTopX      = IMPL_DIALOG_OFFSET;

        // Unter Windows 95 werden die Buttons rechtsbuendig angeordnet
        nX = IMPL_DIALOG_OFFSET;
        long nCtrlBarWidth = ((aCtrlSize.Width()+IMPL_DIALOG_OFFSET)*nDownCtrl)-IMPL_DIALOG_OFFSET;
        if ( nCtrlBarWidth <= (aTabSize.Width()+nBtnEx) )
            nX = (aTabSize.Width()+nBtnEx) - nCtrlBarWidth + IMPL_DIALOG_OFFSET;

        pChild = pChild = GetWindow( WINDOW_FIRSTCHILD );
        while ( pChild )
        {
            if ( pChild->IsVisible() && (pChild != mpViewWindow) )
            {
                if ( pChild == pTabControl )
                    bTabCtrl = TRUE;
                else if ( bTabCtrl )
                {
                    if ( !nLines )
                        nLines = 1;

                    if ( nX+aCtrlSize.Width()-IMPL_DIALOG_OFFSET > (aTabSize.Width()+nBtnEx) )
                    {
                        nY += aCtrlSize.Height()+IMPL_DIALOG_OFFSET;
                        nX  = IMPL_DIALOG_OFFSET;
                        nLines++;
                    }

                    pChild->SetPosSizePixel( Point( nX, nY ), aCtrlSize );
                    nX += aCtrlSize.Width()+IMPL_DIALOG_OFFSET;
                }
                else
                {
                    Size aChildSize = pChild->GetSizePixel();
                    pChild->SetPosPixel( Point( nTopX, (nOffY-aChildSize.Height())/2 ) );
                    nTopX += aChildSize.Width()+2;
                }
            }

            pChild = pChild->GetWindow( WINDOW_NEXT );
        }

        aDlgSize.Height() += nLines * (aCtrlSize.Height()+IMPL_DIALOG_OFFSET);
        SetOutputSizePixel( aDlgSize );
    }

    // Offset merken
    if ( nOffY )
    {
        Size aDlgSize = GetOutputSizePixel();
        if ( !mpFixedLine )
            mpFixedLine = new FixedLine( this );
        mpFixedLine->SetPosSizePixel( Point( 0, nOffY ),
                                      Size( aDlgSize.Width(), 2 ) );
        mpFixedLine->Show();
    }

    mbPosControls = FALSE;
}

// -----------------------------------------------------------------------

TabDialog::TabDialog( Window* pParent, WinBits nStyle ) :
    Dialog( WINDOW_TABDIALOG )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

TabDialog::TabDialog( Window* pParent, const ResId& rResId ) :
    Dialog( WINDOW_TABDIALOG )
{
    ImplInitData();
    rResId.SetRT( RSC_TABDIALOG );
    ImplInit( pParent, ImplInitRes( rResId ) );
    ImplLoadRes( rResId );
}

// -----------------------------------------------------------------------

TabDialog::~TabDialog()
{
    if ( mpFixedLine )
        delete mpFixedLine;
}

// -----------------------------------------------------------------------

void TabDialog::Resize()
{
// !!! In the future the controls should be automaticly rearrange
// !!! if the window is resized
// !!! if ( !IsRollUp() )
// !!!      ImplPosControls();
}

// -----------------------------------------------------------------------

void TabDialog::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_INITSHOW )
    {
        // Calculate the Layout only for the initialized state
        if ( mbPosControls )
            ImplPosControls();
    }
    Dialog::StateChanged( nType );
}

// -----------------------------------------------------------------------

void TabDialog::AdjustLayout()
{
    ImplPosControls();
}
