/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <config_features.h>

#include "osx/saldata.hxx"
#include "osx/salnsmenu.h"
#include "osx/salinst.h"

#import "apple_remote/RemoteMainController.h"

oslThreadKey SalData::s_aAutoReleaseKey = 0;

static void SAL_CALL releasePool( void* pPool )
{
    if( pPool )
        [(NSAutoreleasePool*)pPool release];
}

SalData::SalData()
:
    mpTimerProc( NULL ),
    mpFirstInstance( NULL ),
    mpFirstObject( NULL ),
    mpFirstVD( NULL ),
    mpFirstPrinter( NULL ),
    mpFontList( NULL ),
    mpStatusItem( nil ),
    mxRGBSpace( CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB) ),
    mxGraySpace( CGColorSpaceCreateWithName(kCGColorSpaceGenericGray) ),
    maCursors( POINTER_COUNT, INVALID_CURSOR_PTR ),
    mbIsScrollbarDoubleMax( false ),
#if !HAVE_FEATURE_MACOSX_SANDBOX
    mpAppleRemoteMainController( NULL ),
#endif
    mpDockIconClickHandler( nil ),
    mnDPIX( 0 ),
    mnDPIY( 0 )
{
    if( s_aAutoReleaseKey == 0 )
        s_aAutoReleaseKey = osl_createThreadKey( releasePool );
}

SalData::~SalData()
{
    CGColorSpaceRelease( mxRGBSpace );
    CGColorSpaceRelease( mxGraySpace );
    for( unsigned int i = 0; i < maCursors.size(); i++ )
    {
        NSCursor* pCurs = maCursors[i];
        if( pCurs && pCurs != INVALID_CURSOR_PTR )
            [pCurs release];
    }
    if( s_aAutoReleaseKey )
    {
        
        NSAutoreleasePool* pPool = nil;
        pPool = reinterpret_cast<NSAutoreleasePool*>( osl_getThreadKeyData( s_aAutoReleaseKey ) );
        if( pPool )
        {
            osl_setThreadKeyData( s_aAutoReleaseKey, NULL );
            [pPool release];
        }

        osl_destroyThreadKey( s_aAutoReleaseKey );
        s_aAutoReleaseKey = 0;
    }
#if !HAVE_FEATURE_MACOSX_SANDBOX
    if ( mpAppleRemoteMainController )
        [mpAppleRemoteMainController release];
#endif
}

void SalData::ensureThreadAutoreleasePool()
{
    NSAutoreleasePool* pPool = nil;
    if( s_aAutoReleaseKey )
    {
        pPool = reinterpret_cast<NSAutoreleasePool*>( osl_getThreadKeyData( s_aAutoReleaseKey ) );
        if( ! pPool )
        {
            pPool = [[NSAutoreleasePool alloc] init];
            osl_setThreadKeyData( s_aAutoReleaseKey, pPool );
        }
    }
    else
    {
        OSL_FAIL( "no autorelease key" );
    }
}

struct curs_ent
{
    const char*         pBaseName;
    const NSPoint       aHotSpot;
}
const aCursorTab[ POINTER_COUNT ] =
{
{ NULL, { 0, 0 } }, 
{ "nullptr", { 16, 16 } }, 
{ "hourglass", { 15, 15 } }, 
{ NULL, { 0, 0 } }, 
{ "help", { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ "nwsesize", { 15, 15 } }, 
{ "neswsize", { 15, 15 } }, 
{ "neswsize", { 15, 15 } }, 
{ "nwsesize", { 15, 15 } }, 
{ NULL, { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ "nwsesize", { 15, 15 } }, 
{ "neswsize", { 15, 15 } }, 
{ "neswsize", { 15, 15 } }, 
{ "nwsesize", { 15, 15 } }, 
{ NULL, { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ NULL, { 0, 0 } }, 
{ "pen", { 3, 27 } }, 
{ "magnify", { 12, 13 } }, 
{ "fill", { 10, 22 } }, 
{ "rotate", { 15, 15 } }, 
{ "hshear", { 15, 15 } }, 
{ "vshear", { 15, 15 } }, 
{ "mirror", { 14, 12 } }, 
{ "crook", { 15, 14 } }, 
{ "crop", { 9, 9 } }, 
{ "movept", { 0, 0 } }, 
{ "movebw", { 0, 0 } }, 
{ "movedata", { 0, 0 } }, 
{ "copydata", { 0, 0 } }, 
{ "linkdata", { 0, 0 } }, 
{ "movedlnk", { 0, 0 } }, 
{ "copydlnk", { 0, 0 } }, 
{ "movef", { 8, 8 } }, 
{ "copyf", { 8, 8 } }, 
{ "linkf", { 8, 8 } }, 
{ "moveflnk", { 8, 8 } }, 
{ "copyflnk", { 8, 8 } }, 
{ "movef2", { 7, 8 } }, 
{ "copyf2", { 7, 8 } }, 
{ "notallow", { 15, 15 } }, 
{ "dline", { 8, 8 } }, 
{ "drect", { 8, 8 } }, 
{ "dpolygon", { 8, 8 } }, 
{ "dbezier", { 8, 8 } }, 
{ "darc", { 8, 8 } }, 
{ "dpie", { 8, 8 } }, 
{ "dcirccut", { 8, 8 } }, 
{ "dellipse", { 8, 8 } }, 
{ "dfree", { 8, 8 } }, 
{ "dconnect", { 8, 8 } }, 
{ "dtext", { 8, 8 } }, 
{ "dcapt", { 8, 8 } }, 
{ "chart", { 15, 16 } }, 
{ "detectiv", { 12, 13 } }, 
{ "pivotcol", { 7, 5 } }, 
{ "pivotrow", { 8, 7 } }, 
{ "pivotfld", { 8, 7 } }, 
{ "chain", { 0, 2 } }, 
{ "chainnot", { 2, 2 } }, 
{ "timemove", { 16, 16 } }, 
{ "timesize", { 16, 17 } }, 
{ "asn", { 16, 12 } }, 
{ "ass", { 15, 19 } }, 
{ "asw", { 12, 15 } }, 
{ "ase", { 19, 16 } }, 
{ "asnw", { 10, 10 } }, 
{ "asne", { 21, 10 } }, 
{ "assw", { 21, 21 } }, 
{ "asse", { 21, 21 } }, 
{ "asns", { 15, 15 } }, 
{ "aswe", { 15, 15 } }, 
{ "asnswe", { 15, 15 } }, 
{ "airbrush", { 5, 22 } }, 
{ "vtext", { 15, 15 } }, 
{ "pivotdel", { 18, 15 } }, 
{ "tblsels", { 15, 30 } }, 
{ "tblsele", { 30, 16 } }, 
{ "tblselse", { 30, 30 } }, 
{ "tblselw", { 1, 16 } }, 
{ "tblselsw", { 1, 30 } }, 
{ "pntbrsh", { 9, 16 } }  
};

NSCursor* SalData::getCursor( PointerStyle i_eStyle )
{
    if( i_eStyle >= POINTER_COUNT )
        return nil;

    NSCursor* pCurs = maCursors[ i_eStyle ];
    if( pCurs == INVALID_CURSOR_PTR )
    {
        pCurs = nil;
        if( aCursorTab[ i_eStyle ].pBaseName )
        {
            NSPoint aHotSpot = aCursorTab[ i_eStyle ].aHotSpot;
            CFStringRef pCursorName =
                CFStringCreateWithCStringNoCopy(
                    kCFAllocatorDefault,
                    aCursorTab[ i_eStyle ].pBaseName,
                    kCFStringEncodingASCII,
                    kCFAllocatorNull );
            CFBundleRef hMain = CFBundleGetMainBundle();
            CFURLRef hURL = CFBundleCopyResourceURL( hMain, pCursorName, CFSTR("png"), CFSTR("cursors") );
            if( hURL )
            {
                pCurs = [[NSCursor alloc] initWithImage: [[NSImage alloc] initWithContentsOfURL: (NSURL*)hURL] hotSpot: aHotSpot];
                CFRelease( hURL );
            }
            CFRelease( pCursorName );
        }
        maCursors[ i_eStyle ] = pCurs;
    }
    return pCurs;
}

NSStatusItem* SalData::getStatusItem()
{
    SalData* pData = GetSalData();
    if( ! pData->mpStatusItem )
    {
        NSStatusBar* pStatBar =[NSStatusBar systemStatusBar];
        if( pStatBar )
        {
            pData->mpStatusItem = [pStatBar statusItemWithLength: NSVariableStatusItemLength];
            [pData->mpStatusItem retain];
            OOStatusItemView* pView = [[OOStatusItemView alloc] init];
            [pData->mpStatusItem setView: pView ];
            [pView display];
        }
    }
    return pData->mpStatusItem;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
