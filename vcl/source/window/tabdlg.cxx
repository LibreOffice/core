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

#include <vcl/toolkit/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/toolkit/tabdlg.hxx>
#include <vcl/tabpage.hxx>

void TabDialog::ImplInitTabDialogData()
{
    mpFixedLine     = nullptr;
    mbPosControls   = true;
}

void TabDialog::ImplPosControls()
{
    if (isLayoutEnabled())
        return;

    Size        aCtrlSize( IMPL_MINSIZE_BUTTON_WIDTH, IMPL_MINSIZE_BUTTON_HEIGHT );
    tools::Long        nDownCtrl = 0;
    tools::Long        nOffY = 0;
    vcl::Window*     pTabControl = nullptr;

    vcl::Window* pChild = GetWindow( GetWindowType::FirstChild );
    while ( pChild )
    {
        if ( pChild->IsVisible() )
        {
            if (pChild->GetType() == WindowType::TABCONTROL || isContainerWindow(*pChild))
                pTabControl = pChild;
            else if ( pTabControl )
            {
                Size aOptimalSize(pChild->get_preferred_size());
                tools::Long nTxtWidth = aOptimalSize.Width();
                if ( nTxtWidth > aCtrlSize.Width() )
                    aCtrlSize.setWidth( nTxtWidth );
                tools::Long nTxtHeight = aOptimalSize.Height();
                if ( nTxtHeight > aCtrlSize.Height() )
                    aCtrlSize.setHeight( nTxtHeight );
                nDownCtrl++;
            }
            else
            {
                tools::Long nHeight = pChild->GetSizePixel().Height();
                if ( nHeight > nOffY )
                    nOffY = nHeight;
            }
        }

        pChild = pChild->GetWindow( GetWindowType::Next );
    }

    // do we have a TabControl ?
    if ( pTabControl )
    {
        // adapt offset for other controls by an extra distance
        if ( nOffY )
            nOffY += IMPL_DIALOG_BAR_OFFSET*2 + 2;

        Point   aTabOffset( IMPL_DIALOG_OFFSET, IMPL_DIALOG_OFFSET+nOffY );

        if (isContainerWindow(*pTabControl))
            pTabControl->SetSizePixel(pTabControl->get_preferred_size());

        Size    aTabSize = pTabControl->GetSizePixel();

        Size    aDlgSize( aTabSize.Width() + IMPL_DIALOG_OFFSET*2,
                          aTabSize.Height() + IMPL_DIALOG_OFFSET*2 + nOffY );

        // adapt positioning
        pTabControl->SetPosPixel( aTabOffset );

        // position all other Children
        bool bTabCtrl   = false;
        int  nLines     = 0;
        tools::Long nX;
        tools::Long nY         = aDlgSize.Height();
        tools::Long nTopX      = IMPL_DIALOG_OFFSET;

        // all buttons are right aligned under Windows 95
        nX = IMPL_DIALOG_OFFSET;
        tools::Long nCtrlBarWidth = ((aCtrlSize.Width()+IMPL_DIALOG_OFFSET)*nDownCtrl)-IMPL_DIALOG_OFFSET;
        if ( nCtrlBarWidth <= aTabSize.Width() )
            nX = aTabSize.Width() - nCtrlBarWidth + IMPL_DIALOG_OFFSET;

        vcl::Window* pChild2 = GetWindow( GetWindowType::FirstChild );
        while ( pChild2 )
        {
            if ( pChild2->IsVisible() )
            {
                if ( pChild2 == pTabControl )
                    bTabCtrl = true;
                else if ( bTabCtrl )
                {
                    if ( !nLines )
                        nLines = 1;

                    if ( nX+aCtrlSize.Width()-IMPL_DIALOG_OFFSET > aTabSize.Width() )
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

            pChild2 = pChild2->GetWindow( GetWindowType::Next );
        }

        aDlgSize.AdjustHeight(nLines * (aCtrlSize.Height()+IMPL_DIALOG_OFFSET) );
        SetOutputSizePixel( aDlgSize );
    }

    // store offset
    if ( nOffY )
    {
        Size aDlgSize = GetOutputSizePixel();
        if ( !mpFixedLine )
            mpFixedLine = VclPtr<FixedLine>::Create( this );
        mpFixedLine->SetPosSizePixel( Point( 0, nOffY ),
                                      Size( aDlgSize.Width(), 2 ) );
        mpFixedLine->Show();
    }

    mbPosControls = false;
}

TabDialog::TabDialog( vcl::Window* pParent, WinBits nStyle ) :
    Dialog( WindowType::TABDIALOG )
{
    ImplInitTabDialogData();
    ImplInitDialog( pParent, nStyle );
}

TabDialog::~TabDialog()
{
    disposeOnce();
}

void TabDialog::dispose()
{
    mpFixedLine.disposeAndClear();
    Dialog::dispose();
}

void TabDialog::StateChanged( StateChangedType nType )
{
    if ( nType == StateChangedType::InitShow )
    {
        // Calculate the Layout only for the initialized state
        if ( mbPosControls )
            ImplPosControls();
    }
    Dialog::StateChanged( nType );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
