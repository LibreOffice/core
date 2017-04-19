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

#include <vcl/button.hxx>

#include "osx/salsys.h"
#include "osx/saldata.hxx"
#include "osx/salinst.h"
#include "quartz/utils.h"

#include "strings.hrc"

AquaSalSystem::~AquaSalSystem()
{
}

unsigned int AquaSalSystem::GetDisplayScreenCount()
{
    NSArray* pScreens = [NSScreen screens];
    return pScreens ? [pScreens count] : 1;
}

tools::Rectangle AquaSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    NSArray* pScreens = [NSScreen screens];
    tools::Rectangle aRet;
    NSScreen* pScreen = nil;
    if( pScreens && nScreen < [pScreens count] )
        pScreen = [pScreens objectAtIndex: nScreen];
    else
        pScreen = [NSScreen mainScreen];

    if( pScreen )
    {
        NSRect aFrame = [pScreen frame];
        aRet = tools::Rectangle( Point( static_cast<long int>(aFrame.origin.x), static_cast<long int>(aFrame.origin.y) ),
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

int AquaSalSystem::ShowNativeMessageBox( const OUString& rTitle,
                                        const OUString& rMessage )
{
    NSString* pTitle = CreateNSString( rTitle );
    NSString* pMessage = CreateNSString( rMessage );

    NSString* pDefText = getStandardString( StandardButtonType::OK, false/*bUseResources*/ );

    SAL_WNODEPRECATED_DECLARATIONS_PUSH //TODO: 10.10 NSRunAlertPanel
    int nResult = NSRunAlertPanel( pTitle, @"%@", pDefText, nil, nil, pMessage );
    SAL_WNODEPRECATED_DECLARATIONS_POP

    if( pTitle )
        [pTitle release];
    if( pMessage )
        [pMessage release];
    if( pDefText )
        [pDefText release];

    int nRet = 0;
    if( nResult == 1 )
        nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK;

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
