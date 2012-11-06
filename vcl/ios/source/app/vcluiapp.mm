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

#include "vcl/window.hxx"
#include "vcl/svapp.hxx"
#include "vcl/cmdevt.hxx"

#include "ios/vcluiapp.h"
#include "ios/salinst.h"
#include "ios/saldata.hxx"
#include "ios/salframe.h"
#include "ios/salframeview.h"

#include "impimagetree.hxx"

@implementation VCL_UIApplication
-(void)sendEvent:(UIEvent*)pEvent
{
    [super sendEvent: pEvent];
}

-(void)sendSuperEvent:(UIEvent*)pEvent
{
    [super sendEvent: pEvent];
}

-(BOOL)application: (UIApplication*)app openFile: (NSString*)pFile
{
    (void)app;
    const rtl::OUString aFile( GetOUString( pFile ) );
    return YES;
}

-(void)application: (UIApplication*) app openFiles: (NSArray*)files
{
    (void)app;
    rtl::OUStringBuffer aFileList( 256 );

    NSEnumerator* it = [files objectEnumerator];
    NSString* pFile = nil;

    while( (pFile = [it nextObject]) != nil )
    {
        const rtl::OUString aFile( GetOUString( pFile ) );
    }

    if( aFileList.getLength() )
    {
        // we have no back channel here, we have to assume success, in which case
        // replyToOpenOrPrint does not need to be called according to documentation
        // [app replyToOpenOrPrint: NSApplicationDelegateReplySuccess];
        const ApplicationEvent* pAppEvent = new ApplicationEvent(ApplicationEvent::TYPE_OPEN, aFileList.makeStringAndClear());
        IosSalInstance::aAppEventList.push_back( pAppEvent );
    }
}

-(void)addFallbackMenuItem: (UIMenuItem*)pNewItem
{
    // ???
    (void) pNewItem;
}

-(void)removeFallbackMenuItem: (UIMenuItem*)pItem
{
    // ???
    (void) pItem;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
