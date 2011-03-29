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

#include "aqua/saldata.hxx"
#include "aqua/salnsmenu.h"
#include "aqua/salinst.h"

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
    mxP50Space( NULL ),
    mxP50Pattern( NULL ),
    maCursors( POINTER_COUNT, INVALID_CURSOR_PTR ),
    mbIsScrollbarDoubleMax( false ),
    mnSystemVersion( VER_TIGER ),
    mpMainController( NULL ),
    mpDockIconClickHandler( nil ),
    mnDPIX( 0 ),
    mnDPIY( 0 )
{
    if( s_aAutoReleaseKey == 0 )
        s_aAutoReleaseKey = osl_createThreadKey( releasePool );
}

SalData::~SalData()
{
    CGPatternRelease( mxP50Pattern );
    CGColorSpaceRelease( mxP50Space );
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
        // release the last pool
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
    if ( mpMainController )
        [mpMainController release];
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
        DBG_ERROR( "no autorelease key" );
    }
}

void SalData::drainThreadAutoreleasePool()
{
    NSAutoreleasePool* pPool = nil;
    if( s_aAutoReleaseKey )
    {
        pPool = reinterpret_cast<NSAutoreleasePool*>( osl_getThreadKeyData( s_aAutoReleaseKey ) );
        if( pPool )
        {
            // osl_setThreadKeyData( s_aAutoReleaseKey, NULL );
            // [pPool release];
            [pPool drain];
        }
        else
        {
            pPool = [[NSAutoreleasePool alloc] init];
            osl_setThreadKeyData( s_aAutoReleaseKey, pPool );
        }
    }
    else
    {
        DBG_ERROR( "no autorelease key" );
    }
}


struct curs_ent
{
    const char*         pBaseName;
    const NSPoint       aHotSpot;
}
const aCursorTab[ POINTER_COUNT ] =
{
{ NULL, { 0, 0 } }, //POINTER_ARROW
{ "nullptr", { 16, 16 } }, //POINTER_NULL
{ "hourglass", { 15, 15 } }, //POINTER_WAIT
{ NULL, { 0, 0 } }, //POINTER_TEXT
{ "help", { 0, 0 } }, //POINTER_HELP
{ NULL, { 0, 0 } }, //POINTER_CROSS
{ NULL, { 0, 0 } }, //POINTER_MOVE
{ NULL, { 0, 0 } }, //POINTER_NSIZE
{ NULL, { 0, 0 } }, //POINTER_SSIZE
{ NULL, { 0, 0 } }, //POINTER_WSIZE
{ NULL, { 0, 0 } }, //POINTER_ESIZE
{ "nwsesize", { 15, 15 } }, //POINTER_NWSIZE
{ "neswsize", { 15, 15 } }, //POINTER_NESIZE
{ "neswsize", { 15, 15 } }, //POINTER_SWSIZE
{ "nwsesize", { 15, 15 } }, //POINTER_SESIZE
{ NULL, { 0, 0 } }, //POINTER_WINDOW_NSIZE
{ NULL, { 0, 0 } }, //POINTER_WINDOW_SSIZE
{ NULL, { 0, 0 } }, //POINTER_WINDOW_WSIZE
{ NULL, { 0, 0 } }, //POINTER_WINDOW_ESIZE
{ "nwsesize", { 15, 15 } }, //POINTER_WINDOW_NWSIZE
{ "neswsize", { 15, 15 } }, //POINTER_WINDOW_NESIZE
{ "neswsize", { 15, 15 } }, //POINTER_WINDOW_SWSIZE
{ "nwsesize", { 15, 15 } }, //POINTER_WINDOW_SESIZE
{ NULL, { 0, 0 } }, //POINTER_HSPLIT
{ NULL, { 0, 0 } }, //POINTER_VSPLIT
{ NULL, { 0, 0 } }, //POINTER_HSIZEBAR
{ NULL, { 0, 0 } }, //POINTER_VSIZEBAR
{ NULL, { 0, 0 } }, //POINTER_HAND
{ NULL, { 0, 0 } }, //POINTER_REFHAND
{ "pen", { 3, 27 } }, //POINTER_PEN
{ "magnify", { 12, 13 } }, //POINTER_MAGNIFY
{ "fill", { 10, 22 } }, //POINTER_FILL
{ "rotate", { 15, 15 } }, //POINTER_ROTATE
{ "hshear", { 15, 15 } }, //POINTER_HSHEAR
{ "vshear", { 15, 15 } }, //POINTER_VSHEAR
{ "mirror", { 14, 12 } }, //POINTER_MIRROR
{ "crook", { 15, 14 } }, //POINTER_CROOK
{ "crop", { 9, 9 } }, //POINTER_CROP
{ "movept", { 0, 0 } }, //POINTER_MOVEPOINT
{ "movebw", { 0, 0 } }, //POINTER_MOVEBEZIERWEIGHT
{ "movedata", { 0, 0 } }, //POINTER_MOVEDATA
{ "copydata", { 0, 0 } }, //POINTER_COPYDATA
{ "linkdata", { 0, 0 } }, //POINTER_LINKDATA
{ "movedlnk", { 0, 0 } }, //POINTER_MOVEDATALINK
{ "copydlnk", { 0, 0 } }, //POINTER_COPYDATALINK
{ "movef", { 8, 8 } }, //POINTER_MOVEFILE
{ "copyf", { 8, 8 } }, //POINTER_COPYFILE
{ "linkf", { 8, 8 } }, //POINTER_LINKFILE
{ "moveflnk", { 8, 8 } }, //POINTER_MOVEFILELINK
{ "copyflnk", { 8, 8 } }, //POINTER_COPYFILELINK
{ "movef2", { 7, 8 } }, //POINTER_MOVEFILES
{ "copyf2", { 7, 8 } }, //POINTER_COPYFILES
{ "notallow", { 15, 15 } }, //POINTER_NOTALLOWED
{ "dline", { 8, 8 } }, //POINTER_DRAW_LINE
{ "drect", { 8, 8 } }, //POINTER_DRAW_RECT
{ "dpolygon", { 8, 8 } }, //POINTER_DRAW_POLYGON
{ "dbezier", { 8, 8 } }, //POINTER_DRAW_BEZIER
{ "darc", { 8, 8 } }, //POINTER_DRAW_ARC
{ "dpie", { 8, 8 } }, //POINTER_DRAW_PIE
{ "dcirccut", { 8, 8 } }, //POINTER_DRAW_CIRCLECUT
{ "dellipse", { 8, 8 } }, //POINTER_DRAW_ELLIPSE
{ "dfree", { 8, 8 } }, //POINTER_DRAW_FREEHAND
{ "dconnect", { 8, 8 } }, //POINTER_DRAW_CONNECT
{ "dtext", { 8, 8 } }, //POINTER_DRAW_TEXT
{ "dcapt", { 8, 8 } }, //POINTER_DRAW_CAPTION
{ "chart", { 15, 16 } }, //POINTER_CHART
{ "detectiv", { 12, 13 } }, //POINTER_DETECTIVE
{ "pivotcol", { 7, 5 } }, //POINTER_PIVOT_COL
{ "pivotrow", { 8, 7 } }, //POINTER_PIVOT_ROW
{ "pivotfld", { 8, 7 } }, //POINTER_PIVOT_FIELD
{ "chain", { 0, 2 } }, //POINTER_CHAIN
{ "chainnot", { 2, 2 } }, //POINTER_CHAIN_NOTALLOWED
{ "timemove", { 16, 16 } }, //POINTER_TIMEEVENT_MOVE
{ "timesize", { 16, 17 } }, //POINTER_TIMEEVENT_SIZE
{ "asn", { 16, 12 } }, //POINTER_AUTOSCROLL_N
{ "ass", { 15, 19 } }, //POINTER_AUTOSCROLL_S
{ "asw", { 12, 15 } }, //POINTER_AUTOSCROLL_W
{ "ase", { 19, 16 } }, //POINTER_AUTOSCROLL_E
{ "asnw", { 10, 10 } }, //POINTER_AUTOSCROLL_NW
{ "asne", { 21, 10 } }, //POINTER_AUTOSCROLL_NE
{ "assw", { 21, 21 } }, //POINTER_AUTOSCROLL_SW
{ "asse", { 21, 21 } }, //POINTER_AUTOSCROLL_SE
{ "asns", { 15, 15 } }, //POINTER_AUTOSCROLL_NS
{ "aswe", { 15, 15 } }, //POINTER_AUTOSCROLL_WE
{ "asnswe", { 15, 15 } }, //POINTER_AUTOSCROLL_NSWE
{ "airbrush", { 5, 22 } }, //POINTER_AIRBRUSH
{ "vtext", { 15, 15 } }, //POINTER_TEXT_VERTICAL
{ "pivotdel", { 18, 15 } }, //POINTER_PIVOT_DELETE
{ "tblsels", { 15, 30 } }, //POINTER_TAB_SELECT_S
{ "tblsele", { 30, 16 } }, //POINTER_TAB_SELECT_E
{ "tblselse", { 30, 30 } }, //POINTER_TAB_SELECT_SE
{ "tblselw", { 1, 16 } }, //POINTER_TAB_SELECT_W
{ "tblselsw", { 1, 30 } }, //POINTER_TAB_SELECT_SW
{ "pntbrsh", { 9, 16 } }  //POINTER_PAINTBRUSH
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
