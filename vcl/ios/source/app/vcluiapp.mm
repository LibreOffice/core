/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

 // MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

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
    UIEventType eType = [pEvent type];
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
        const ApplicationEvent* pAppEvent = new ApplicationEvent( String(), ApplicationAddress(),
                                                    APPEVENT_OPEN_STRING, aFileList.makeStringAndClear() );
        IosSalInstance::aAppEventList.push_back( pAppEvent );
    }
}

-(void)addFallbackMenuItem: (UIMenuItem*)pNewItem
{
    // ???
}

-(void)removeFallbackMenuItem: (UIMenuItem*)pItem
{
    // ???
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
