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
#include <vcl/layout.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabdlg.hxx>
#include <tools/rc.h>



// =======================================================================

void TabDialog::ImplInitTabDialogData()
{
    mpFixedLine     = NULL;
    mpViewWindow    = NULL;
    meViewAlign     = WINDOWALIGN_LEFT;
    mbPosControls   = sal_True;
}

// -----------------------------------------------------------------------

void TabDialog::ImplPosControls()
{
    if (isLayoutEnabled())
        return;

    Size        aCtrlSize( IMPL_MINSIZE_BUTTON_WIDTH, IMPL_MINSIZE_BUTTON_HEIGHT );
    long        nDownCtrl = 0;
    long        nOffY = 0;
    Window*     pTabControl = NULL;

    Window* pChild = GetWindow( WINDOW_FIRSTCHILD );
    while ( pChild )
    {
        if ( pChild->IsVisible() && (pChild != mpViewWindow) )
        {
            if (pChild->GetType() == WINDOW_TABCONTROL || isContainerWindow(*pChild))
                pTabControl = pChild;
            else if ( pTabControl )
            {
                Size aOptimalSize(pChild->GetOptimalSize());
                long nTxtWidth = aOptimalSize.Width();
                if ( nTxtWidth > aCtrlSize.Width() )
                    aCtrlSize.Width() = nTxtWidth;
                long nTxtHeight = aOptimalSize.Height();
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

        if (isContainerWindow(*pTabControl))
            pTabControl->SetSizePixel(pTabControl->GetOptimalSize());

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
            sal_uInt16  nViewPosFlags = WINDOW_POSSIZE_POS;
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

            mpViewWindow->setPosSizePixel( nViewOffX, nViewOffY,
                                           nViewWidth, nViewHeight,
                                           nViewPosFlags );
        }

        // Positionierung vornehmen
        pTabControl->SetPosPixel( aTabOffset );

        // Alle anderen Children positionieren
        sal_Bool bTabCtrl   = sal_False;
        int  nLines     = 0;
        long nX;
        long nY         = aDlgSize.Height();
        long nTopX      = IMPL_DIALOG_OFFSET;

        // Unter Windows 95 werden die Buttons rechtsbuendig angeordnet
        nX = IMPL_DIALOG_OFFSET;
        long nCtrlBarWidth = ((aCtrlSize.Width()+IMPL_DIALOG_OFFSET)*nDownCtrl)-IMPL_DIALOG_OFFSET;
        if ( nCtrlBarWidth <= (aTabSize.Width()+nBtnEx) )
            nX = (aTabSize.Width()+nBtnEx) - nCtrlBarWidth + IMPL_DIALOG_OFFSET;

        Window* pChild2 = GetWindow( WINDOW_FIRSTCHILD );
        while ( pChild2 )
        {
            if ( pChild2->IsVisible() && (pChild2 != mpViewWindow) )
            {
                if ( pChild2 == pTabControl )
                    bTabCtrl = sal_True;
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

                    pChild2->SetPosSizePixel( Point( nX, nY ), aCtrlSize );
                    nX += aCtrlSize.Width()+IMPL_DIALOG_OFFSET;
                }
                else
                {
                    Size aChildSize = pChild2->GetSizePixel();
                    pChild2->SetPosPixel( Point( nTopX, (nOffY-aChildSize.Height())/2 ) );
                    nTopX += aChildSize.Width()+2;
                }
            }

            pChild2 = pChild2->GetWindow( WINDOW_NEXT );
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

    mbPosControls = sal_False;
}

// -----------------------------------------------------------------------

TabDialog::TabDialog( Window* pParent, WinBits nStyle ) :
    Dialog( WINDOW_TABDIALOG )
{
    ImplInitTabDialogData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

TabDialog::TabDialog( Window* pParent, const ResId& rResId ) :
    Dialog( WINDOW_TABDIALOG )
{
    ImplInitTabDialogData();
    rResId.SetRT( RSC_TABDIALOG );
    init(pParent, rResId);
}

TabDialog::TabDialog( Window* pParent, const OString& rID, const OUString& rUIXMLDescription ) :
    Dialog(pParent, rID, rUIXMLDescription, WINDOW_TABDIALOG)
{
    ImplInitTabDialogData();
}

// -----------------------------------------------------------------------

TabDialog::~TabDialog()
{
    delete mpFixedLine;
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
    SAL_WARN_IF(isLayoutEnabled(), "vcl.layout", "unnecessary to call TabDialog::AdjustLayout when layout enabled");

    ImplPosControls();
    queue_resize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
