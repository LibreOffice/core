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

#include "rtl/ustrbuf.hxx"

#include "vcl/button.hxx"

#include "osx/salsys.h"
#include "osx/saldata.hxx"
#include "osx/salinst.h"
#include "quartz/utils.h"

#include "svids.hrc"

AquaSalSystem::~AquaSalSystem()
{
}

unsigned int AquaSalSystem::GetDisplayScreenCount()
{
    NSArray* pScreens = [NSScreen screens];
    return pScreens ? [pScreens count] : 1;
}

Rectangle AquaSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    NSArray* pScreens = [NSScreen screens];
    Rectangle aRet;
    NSScreen* pScreen = nil;
    if( pScreens && nScreen < [pScreens count] )
        pScreen = [pScreens objectAtIndex: nScreen];
    else
        pScreen = [NSScreen mainScreen];

    if( pScreen )
    {
        NSRect aFrame = [pScreen frame];
        aRet = Rectangle( Point( static_cast<long int>(aFrame.origin.x), static_cast<long int>(aFrame.origin.y) ),
                          Size( static_cast<long int>(aFrame.size.width), static_cast<long int>(aFrame.size.height) ) );
    }
    return aRet;
}

static NSString* getStandardString( StandardButtonType nButtonId, bool bUseResources )
{
    OUString aText;
    if( bUseResources )
    {
        aText = Button::GetStandardText( nButtonId );
    }
    if( aText.isEmpty() ) // this is for bad cases, we might be missing the vcl resource
    {
        switch( nButtonId )
        {
        case StandardButtonType::OK:         aText = "OK";break;
        case StandardButtonType::Abort:      aText = "Abort";break;
        case StandardButtonType::Cancel:     aText = "Cancel";break;
        case StandardButtonType::Retry:      aText = "Retry";break;
        case StandardButtonType::Yes:        aText = "Yes";break;
        case StandardButtonType::No:         aText = "No";break;
        default: break;
        }
    }
    return aText.isEmpty() ? nil : CreateNSString( aText);
}

#define NO_BUTTON static_cast<StandardButtonType>(-1)

int AquaSalSystem::ShowNativeMessageBox( const OUString& rTitle,
                                        const OUString& rMessage,
                                        int nButtonCombination,
                                        int nDefaultButton, bool bUseResources)
{
    NSString* pTitle = CreateNSString( rTitle );
    NSString* pMessage = CreateNSString( rMessage );

    struct id_entry
    {
        int nCombination;
        int nDefaultButton;
        StandardButtonType nTextIds[3];
    } aButtonIds[] =
    {
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK, { StandardButtonType::OK, NO_BUTTON, NO_BUTTON } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK, { StandardButtonType::OK, StandardButtonType::Cancel, NO_BUTTON } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, { StandardButtonType::Cancel, StandardButtonType::OK, NO_BUTTON } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_ABORT, { StandardButtonType::Abort, StandardButtonType::Ignore, StandardButtonType::Retry } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY, { StandardButtonType::Retry, StandardButtonType::Ignore, StandardButtonType::Abort } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE, { StandardButtonType::Ignore, StandardButtonType::Ignore, StandardButtonType::Abort } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES, { StandardButtonType::Yes, StandardButtonType::No, StandardButtonType::Cancel } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO, { StandardButtonType::No, StandardButtonType::Yes, StandardButtonType::Cancel } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, { StandardButtonType::Cancel, StandardButtonType::Yes, StandardButtonType::No } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES, { StandardButtonType::Yes, StandardButtonType::No, NO_BUTTON } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO, { StandardButtonType::No, StandardButtonType::Yes, NO_BUTTON } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY, { StandardButtonType::Retry, StandardButtonType::Cancel, NO_BUTTON } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, { StandardButtonType::Cancel, StandardButtonType::Retry, NO_BUTTON } }
    };

    NSString* pDefText = nil;
    NSString* pAltText = nil;
    NSString* pOthText = nil;

    unsigned int nC;
    for( nC = 0; nC < sizeof(aButtonIds)/sizeof(aButtonIds[0]); nC++ )
    {
        if( aButtonIds[nC].nCombination == nButtonCombination )
        {
            if( aButtonIds[nC].nDefaultButton == nDefaultButton )
            {
                if( aButtonIds[nC].nTextIds[0] != NO_BUTTON )
                    pDefText = getStandardString(
                        aButtonIds[nC].nTextIds[0], bUseResources );
                if( aButtonIds[nC].nTextIds[1] != NO_BUTTON )
                    pAltText = getStandardString(
                        aButtonIds[nC].nTextIds[1], bUseResources );
                if( aButtonIds[nC].nTextIds[2] != NO_BUTTON )
                    pOthText = getStandardString(
                        aButtonIds[nC].nTextIds[2], bUseResources );
                break;
            }
        }
    }

    SAL_WNODEPRECATED_DECLARATIONS_PUSH //TODO: 10.10 NSRunAlertPanel
    int nResult = NSRunAlertPanel( pTitle, @"%@", pDefText, pAltText, pOthText, pMessage );
    SAL_WNODEPRECATED_DECLARATIONS_POP

    if( pTitle )
        [pTitle release];
    if( pMessage )
        [pMessage release];
    if( pDefText )
        [pDefText release];
    if( pAltText )
        [pAltText release];
    if( pOthText )
        [pOthText release];

    int nRet = 0;
    if( nC < sizeof(aButtonIds)/sizeof(aButtonIds[0]) && nResult >= 1 && nResult <= 3 )
    {
        StandardButtonType nPressed = aButtonIds[nC].nTextIds[nResult-1];
        switch( nPressed )
        {
        case StandardButtonType::No:     nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO; break;
        case StandardButtonType::Yes:    nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES; break;
        case StandardButtonType::OK:     nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK; break;
        case StandardButtonType::Cancel: nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL; break;
        case StandardButtonType::Abort:  nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_ABORT; break;
        case StandardButtonType::Retry:  nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY; break;
        case StandardButtonType::Ignore: nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE; break;
        default: break;
        }
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
