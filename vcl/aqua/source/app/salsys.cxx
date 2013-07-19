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

#include "aqua/salsys.h"
#include "aqua/saldata.hxx"
#include "aqua/salinst.h"
#include "quartz/utils.h"

#include "svids.hrc"


// =======================================================================

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

OUString AquaSalSystem::GetDisplayScreenName( unsigned int nScreen )
{
   NSArray* pScreens = [NSScreen screens];
   OUString aRet;
   if( nScreen < [pScreens count] )
   {
        ResMgr* pMgr = ImplGetResMgr();
        if( pMgr )
        {
            OUString aScreenName(ResId(SV_MAC_SCREENNNAME, *pMgr).toString());
            aRet = aScreenName.replaceAll("%d", OUString::number(nScreen));
        }
   }
   return aRet;
}

static NSString* getStandardString( int nButtonId, bool bUseResources )
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
        case BUTTON_OK:         aText = OUString( "OK" );break;
        case BUTTON_ABORT:      aText = OUString( "Abort" );break;
        case BUTTON_CANCEL:     aText = OUString( "Cancel" );break;
        case BUTTON_RETRY:      aText = OUString( "Retry" );break;
        case BUTTON_YES:        aText = OUString( "Yes" );break;
        case BUTTON_NO :        aText = OUString( "No" );break;
        }
    }
    return aText.isEmpty() ? nil : CreateNSString( aText);
}

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
        int nTextIds[3];
    } aButtonIds[] =
    {
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK, { BUTTON_OK, -1, -1 } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK, { BUTTON_OK, BUTTON_CANCEL, -1 } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, { BUTTON_CANCEL, BUTTON_OK, -1 } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_ABORT, { BUTTON_ABORT, BUTTON_IGNORE, BUTTON_RETRY } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY, { BUTTON_RETRY, BUTTON_IGNORE, BUTTON_ABORT } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE, { BUTTON_IGNORE, BUTTON_IGNORE, BUTTON_ABORT } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES, { BUTTON_YES, BUTTON_NO, BUTTON_CANCEL } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO, { BUTTON_NO, BUTTON_YES, BUTTON_CANCEL } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, { BUTTON_CANCEL, BUTTON_YES, BUTTON_NO } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES, { BUTTON_YES, BUTTON_NO, -1 } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO, { BUTTON_NO, BUTTON_YES, -1 } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY, { BUTTON_RETRY, BUTTON_CANCEL, -1 } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, { BUTTON_CANCEL, BUTTON_RETRY, -1 } }
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
                if( aButtonIds[nC].nTextIds[0] != -1 )
                    pDefText = getStandardString(
                        aButtonIds[nC].nTextIds[0], bUseResources );
                if( aButtonIds[nC].nTextIds[1] != -1 )
                    pAltText = getStandardString(
                        aButtonIds[nC].nTextIds[1], bUseResources );
                if( aButtonIds[nC].nTextIds[2] != -1 )
                    pOthText = getStandardString(
                        aButtonIds[nC].nTextIds[2], bUseResources );
                break;
            }
        }
    }


    int nResult = NSRunAlertPanel( pTitle, pMessage, pDefText, pAltText, pOthText );

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
        int nPressed = aButtonIds[nC].nTextIds[nResult-1];
        switch( nPressed )
        {
        case BUTTON_NO:     nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO; break;
        case BUTTON_YES:    nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES; break;
        case BUTTON_OK:     nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK; break;
        case BUTTON_CANCEL: nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL; break;
        case BUTTON_ABORT:  nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_ABORT; break;
        case BUTTON_RETRY:  nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY; break;
        case BUTTON_IGNORE: nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE; break;
        }
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
