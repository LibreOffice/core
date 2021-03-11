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

#include "menuwindow.hxx"
#include "menuitemlist.hxx"

#include <vcl/help.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

static sal_uLong ImplChangeTipTimeout( sal_uLong nTimeout, vcl::Window *pWindow )
{
    AllSettings aAllSettings( pWindow->GetSettings() );
    HelpSettings aHelpSettings( aAllSettings.GetHelpSettings() );
    sal_uLong nRet = aHelpSettings.GetTipTimeout();
    aHelpSettings.SetTipTimeout( nTimeout );
    aAllSettings.SetHelpSettings( aHelpSettings );
    pWindow->SetSettings( aAllSettings );
    return nRet;
}

bool MenuWindow::ImplHandleHelpEvent(vcl::Window* pMenuWindow, Menu const * pMenu, sal_uInt16 nHighlightedItem,
        const HelpEvent& rHEvt, const tools::Rectangle &rHighlightRect)
{
    if( ! pMenu )
        return false;

    bool bDone = false;
    sal_uInt16 nId = 0;

    if ( nHighlightedItem != ITEMPOS_INVALID )
    {
        MenuItemData* pItemData = pMenu->GetItemList()->GetDataFromPos( nHighlightedItem );
        if ( pItemData )
            nId = pItemData->nId;
    }

    if ( ( rHEvt.GetMode() & HelpEventMode::BALLOON ) && pMenuWindow )
    {
        Point aPos;
        if( rHEvt.KeyboardActivated() )
            aPos = rHighlightRect.Center();
        else
            aPos = rHEvt.GetMousePosPixel();

        tools::Rectangle aRect( aPos, Size() );
        if (!pMenu->GetHelpText(nId).isEmpty())
            Help::ShowBalloon( pMenuWindow, aPos, aRect, pMenu->GetHelpText( nId ) );
        else
        {
            // give user a chance to read the full filename
            sal_uLong oldTimeout=ImplChangeTipTimeout( 60000, pMenuWindow );
            // call always, even when strlen==0 to correctly remove tip
            Help::ShowQuickHelp( pMenuWindow, aRect, pMenu->GetTipHelpText( nId ) );
            ImplChangeTipTimeout( oldTimeout, pMenuWindow );
        }
        bDone = true;
    }
    else if ( ( rHEvt.GetMode() &HelpEventMode::QUICK ) && pMenuWindow )
    {
        Point aPos = rHEvt.GetMousePosPixel();
        tools::Rectangle aRect( aPos, Size() );
        // give user a chance to read the full filename
        sal_uLong oldTimeout=ImplChangeTipTimeout( 60000, pMenuWindow );
        // call always, even when strlen==0 to correctly remove tip
        Help::ShowQuickHelp( pMenuWindow, aRect, pMenu->GetTipHelpText( nId ) );
        ImplChangeTipTimeout( oldTimeout, pMenuWindow );
        bDone = true;
    }
    else if ( rHEvt.GetMode() & HelpEventMode::CONTEXT )
    {
        // is help in the application selected
        Help* pHelp = Application::GetHelp();
        if ( pHelp )
        {
            // is an id available, then call help with the id, otherwise
            // use help-index
            OUString aCommand = pMenu->GetItemCommand( nId );
            OString aHelpId(  pMenu->GetHelpId( nId ) );
            if( aHelpId.isEmpty() )
                aHelpId = OOO_HELP_INDEX;

            if ( !aCommand.isEmpty() )
                pHelp->Start(aCommand);
            else
                pHelp->Start(OStringToOUString(aHelpId, RTL_TEXTENCODING_UTF8));
        }
        bDone = true;
    }
    return bDone;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
