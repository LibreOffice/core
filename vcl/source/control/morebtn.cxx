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

#include <vcl/morebtn.hxx>

#include <tools/rc.h>
#include <vector>

struct ImplMoreButtonData
{
    std::vector< VclPtr<vcl::Window> >*  mpItemList;
    OUString                             maMoreText;
    OUString                             maLessText;
};

void MoreButton::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    mpMBData     = new ImplMoreButtonData;
    mnDelta      = 0;
    meUnit       = MAP_PIXEL;
    mbState      = false;

    mpMBData->mpItemList = nullptr;

    PushButton::ImplInit( pParent, nStyle );

    mpMBData->maMoreText = Button::GetStandardText( StandardButtonType::More );
    mpMBData->maLessText = Button::GetStandardText( StandardButtonType::Less );

    ShowState();

    SetSymbolAlign(SymbolAlign::RIGHT);
    SetImageAlign(ImageAlign::Right); //Resolves: fdo#31849 ensure button remains vertically centered
    SetSmallSymbol();

    if ( ! ( nStyle & ( WB_RIGHT | WB_LEFT ) ) )
    {
        nStyle |= WB_CENTER;
        SetStyle( nStyle );
    }
}

void MoreButton::ShowState()
{
    if ( mbState )
    {
        SetSymbol( SymbolType::PAGEUP );
        SetText( mpMBData->maLessText );
    }
    else
    {
        SetSymbol( SymbolType::PAGEDOWN );
        SetText( mpMBData->maMoreText );
    }
}

MoreButton::MoreButton( vcl::Window* pParent, WinBits nStyle ) :
    PushButton( WINDOW_MOREBUTTON )
{
    ImplInit( pParent, nStyle );
}

MoreButton::~MoreButton()
{
    disposeOnce();
}

void MoreButton::dispose()
{
    delete mpMBData->mpItemList;
    delete mpMBData;
    PushButton::dispose();
}

void MoreButton::Click()
{
    vcl::Window*     pParent = GetParent();
    Size        aSize( pParent->GetSizePixel() );
    long        nDeltaPixel = LogicToPixel( Size( 0, mnDelta ), meUnit ).Height();

    // Change status
    mbState = !mbState;
    ShowState();

    // Update the windows according to the status
    if ( mbState )
    {
        // Show window
        if ( mpMBData->mpItemList ) {
            for ( size_t i = 0, n = mpMBData->mpItemList->size(); i < n; ++i ) {
                (*mpMBData->mpItemList)[ i ]->Show();
            }
        }

        // Adapt dialogbox
        Point aPos( pParent->GetPosPixel() );
        Rectangle aDeskRect( pParent->ImplGetFrameWindow()->GetDesktopRectPixel() );

        aSize.Height() += nDeltaPixel;
        if ( (aPos.Y()+aSize.Height()) > aDeskRect.Bottom() )
        {
            aPos.Y() = aDeskRect.Bottom()-aSize.Height();

            if ( aPos.Y() < aDeskRect.Top() )
                aPos.Y() = aDeskRect.Top();

            pParent->SetPosSizePixel( aPos, aSize );
        }
        else
            pParent->SetSizePixel( aSize );
    }
    else
    {
        // Adapt Dialogbox
        aSize.Height() -= nDeltaPixel;
        pParent->SetSizePixel( aSize );

        // Hide window(s) again
        if ( mpMBData->mpItemList ) {
            for ( size_t i = 0, n = mpMBData->mpItemList->size(); i < n; ++i ) {
                (*mpMBData->mpItemList)[ i ]->Hide();
            }
        }
    }
    // Call Click handler here, so that we can initialize the Controls
    PushButton::Click();
}

void MoreButton::SetText( const OUString& rText )
{
    PushButton::SetText( rText );
}

OUString MoreButton::GetText() const
{
    return PushButton::GetText();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
