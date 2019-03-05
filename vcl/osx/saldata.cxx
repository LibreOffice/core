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

#include <config_features.h>

#include <osl/diagnose.h>
#include <osx/saldata.hxx>
#include <osx/salnsmenu.h>
#include <osx/salinst.h>
#include <o3tl/enumarray.hxx>

#import "apple_remote/RemoteMainController.h"

oslThreadKey SalData::s_aAutoReleaseKey = nullptr;

static void releasePool( void* pPool )
{
    if( pPool )
        [static_cast<NSAutoreleasePool*>(pPool) release];
}

SalData::SalData()
:
    mpTimerProc( nullptr ),
    mpInstance( nullptr ),
    mpFirstObject( nullptr ),
    mpFirstVD( nullptr ),
    mpFirstPrinter( nullptr ),
    mpFontList( nullptr ),
    mpStatusItem( nil ),
    mxRGBSpace( CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB) ),
    mxGraySpace( CGColorSpaceCreateWithName(kCGColorSpaceGenericGray) ),
    maCursors(),
    mbIsScrollbarDoubleMax( false ),
#if !HAVE_FEATURE_MACOSX_SANDBOX
    mpAppleRemoteMainController( nullptr ),
#endif
    mpDockIconClickHandler( nil ),
    mnDPIX( 0 ),
    mnDPIY( 0 )
{
    SetSalData(this);
    maCursors.fill( INVALID_CURSOR_PTR );
    if( s_aAutoReleaseKey == nullptr )
        s_aAutoReleaseKey = osl_createThreadKey( releasePool );
}

SalData::~SalData()
{
    CGColorSpaceRelease( mxRGBSpace );
    CGColorSpaceRelease( mxGraySpace );
    for( NSCursor* pCurs : maCursors )
    {
        if( pCurs && pCurs != INVALID_CURSOR_PTR )
            [pCurs release];
    }
    if( s_aAutoReleaseKey )
    {
        // release the last pool
        NSAutoreleasePool* pPool = reinterpret_cast<NSAutoreleasePool*>( osl_getThreadKeyData( s_aAutoReleaseKey ) );
        if( pPool )
        {
            osl_setThreadKeyData( s_aAutoReleaseKey, nullptr );
            [pPool release];
        }

        osl_destroyThreadKey( s_aAutoReleaseKey );
        s_aAutoReleaseKey = nullptr;
    }
#if !HAVE_FEATURE_MACOSX_SANDBOX
    if ( mpAppleRemoteMainController )
        [mpAppleRemoteMainController release];
#endif

    if( mpStatusItem )
    {
        [mpStatusItem release];
        mpStatusItem = nil;
    }
    SetSalData( nullptr );
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
};

const o3tl::enumarray<PointerStyle, curs_ent> aCursorTab =
{
curs_ent{ nullptr, { 0, 0 } }, //PointerStyle::Arrow
{ "nullptr", { 16, 16 } }, //PointerStyle::Null
{ "hourglass", { 15, 15 } }, //PointerStyle::Wait
{ nullptr, { 0, 0 } }, //PointerStyle::Text
{ "help", { 0, 0 } }, //PointerStyle::Help
{ nullptr, { 0, 0 } }, //PointerStyle::Cross
{ nullptr, { 0, 0 } }, //PointerStyle::Move
{ nullptr, { 0, 0 } }, //PointerStyle::NSize
{ nullptr, { 0, 0 } }, //PointerStyle::SSize
{ nullptr, { 0, 0 } }, //PointerStyle::WSize
{ nullptr, { 0, 0 } }, //PointerStyle::ESize
{ "nwsesize", { 15, 15 } }, //PointerStyle::NWSize
{ "neswsize", { 15, 15 } }, //PointerStyle::NESize
{ "neswsize", { 15, 15 } }, //PointerStyle::SWSize
{ "nwsesize", { 15, 15 } }, //PointerStyle::SESize
{ nullptr, { 0, 0 } }, //PointerStyle::WindowNSize
{ nullptr, { 0, 0 } }, //PointerStyle::WindowSSize
{ nullptr, { 0, 0 } }, //PointerStyle::WindowWSize
{ nullptr, { 0, 0 } }, //PointerStyle::WindowESize
{ "nwsesize", { 15, 15 } }, //PointerStyle::WindowNWSize
{ "neswsize", { 15, 15 } }, //PointerStyle::WindowNESize
{ "neswsize", { 15, 15 } }, //PointerStyle::WindowSWSize
{ "nwsesize", { 15, 15 } }, //PointerStyle::WindowSESize
{ nullptr, { 0, 0 } }, //PointerStyle::HSplit
{ nullptr, { 0, 0 } }, //PointerStyle::VSplit
{ nullptr, { 0, 0 } }, //PointerStyle::HSizeBar
{ nullptr, { 0, 0 } }, //PointerStyle::VSizeBar
{ nullptr, { 0, 0 } }, //PointerStyle::Hand
{ nullptr, { 0, 0 } }, //PointerStyle::RefHand
{ "pen", { 3, 27 } }, //PointerStyle::Pen
{ "magnify", { 12, 13 } }, //PointerStyle::Magnify
{ "fill", { 10, 22 } }, //PointerStyle::Fill
{ "rotate", { 15, 15 } }, //PointerStyle::Rotate
{ "hshear", { 15, 15 } }, //PointerStyle::HShear
{ "vshear", { 15, 15 } }, //PointerStyle::VShear
{ "mirror", { 14, 12 } }, //PointerStyle::Mirror
{ "crook", { 15, 14 } }, //PointerStyle::Crook
{ "crop", { 9, 9 } }, //PointerStyle::Crop
{ "movept", { 0, 0 } }, //PointerStyle::MovePoint
{ "movebw", { 0, 0 } }, //PointerStyle::MoveBezierWeight
{ "movedata", { 0, 0 } }, //PointerStyle::MoveData
{ "copydata", { 0, 0 } }, //PointerStyle::CopyData
{ "linkdata", { 0, 0 } }, //PointerStyle::LinkData
{ "movedlnk", { 0, 0 } }, //PointerStyle::MoveDataLink
{ "copydlnk", { 0, 0 } }, //PointerStyle::CopyDataLink
{ "movef", { 8, 8 } }, //PointerStyle::MoveFile
{ "copyf", { 8, 8 } }, //PointerStyle::CopyFile
{ "linkf", { 8, 8 } }, //PointerStyle::LinkFile
{ "moveflnk", { 8, 8 } }, //PointerStyle::MoveFileLink
{ "copyflnk", { 8, 8 } }, //PointerStyle::CopyFileLink
{ "movef2", { 7, 8 } }, //PointerStyle::MoveFiles
{ "copyf2", { 7, 8 } }, //PointerStyle::CopyFiles
{ "notallow", { 15, 15 } }, //PointerStyle::NotAllowed
{ "dline", { 8, 8 } }, //PointerStyle::DrawLine
{ "drect", { 8, 8 } }, //PointerStyle::DrawRect
{ "dpolygon", { 8, 8 } }, //PointerStyle::DrawPolygon
{ "dbezier", { 8, 8 } }, //PointerStyle::DrawBezier
{ "darc", { 8, 8 } }, //PointerStyle::DrawArc
{ "dpie", { 8, 8 } }, //PointerStyle::DrawPie
{ "dcirccut", { 8, 8 } }, //PointerStyle::DrawCircleCut
{ "dellipse", { 8, 8 } }, //PointerStyle::DrawEllipse
{ "dfree", { 8, 8 } }, //PointerStyle::DrawFreehand
{ "dconnect", { 8, 8 } }, //PointerStyle::DrawConnect
{ "dtext", { 8, 8 } }, //PointerStyle::DrawText
{ "dcapt", { 8, 8 } }, //PointerStyle::DrawCaption
{ "chart", { 15, 16 } }, //PointerStyle::Chart
{ "detectiv", { 12, 13 } }, //PointerStyle::Detective
{ "pivotcol", { 7, 5 } }, //PointerStyle::PivotCol
{ "pivotrow", { 8, 7 } }, //PointerStyle::PivotRow
{ "pivotfld", { 8, 7 } }, //PointerStyle::PivotField
{ "chain", { 0, 2 } }, //PointerStyle::Chain
{ "chainnot", { 2, 2 } }, //PointerStyle::ChainNotAllowed
{ "asn", { 16, 12 } }, //PointerStyle::AutoScrollN
{ "ass", { 15, 19 } }, //PointerStyle::AutoScrollS
{ "asw", { 12, 15 } }, //PointerStyle::AutoScrollW
{ "ase", { 19, 16 } }, //PointerStyle::AutoScrollE
{ "asnw", { 10, 10 } }, //PointerStyle::AutoScrollNW
{ "asne", { 21, 10 } }, //PointerStyle::AutoScrollNE
{ "assw", { 21, 21 } }, //PointerStyle::AutoScrollSW
{ "asse", { 21, 21 } }, //PointerStyle::AutoScrollSE
{ "asns", { 15, 15 } }, //PointerStyle::AutoScrollNS
{ "aswe", { 15, 15 } }, //PointerStyle::AutoScrollWE
{ "asnswe", { 15, 15 } }, //PointerStyle::AutoScrollNSWE
{ "vtext", { 15, 15 } }, //PointerStyle::TextVertical
{ "pivotdel", { 18, 15 } }, //PointerStyle::PivotDelete
{ "tblsels", { 15, 30 } }, //PointerStyle::TabSelectS
{ "tblsele", { 30, 16 } }, //PointerStyle::TabSelectE
{ "tblselse", { 30, 30 } }, //PointerStyle::TabSelectSE
{ "tblselw", { 1, 16 } }, //PointerStyle::TabSelectW
{ "tblselsw", { 1, 30 } }, //PointerStyle::TabSelectSW
{ "wshide", { 16, 16 } }, //PointerStyle::HideWhitespace
{ "wsshow", { 16, 16 } } //PointerStyle::ShowWhitespace
};

NSCursor* SalData::getCursor( PointerStyle i_eStyle )
{
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
                pCurs = [[NSCursor alloc] initWithImage: [[NSImage alloc] initWithContentsOfURL: const_cast<NSURL*>(reinterpret_cast<NSURL const *>(hURL))] hotSpot: aHotSpot];
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
SAL_WNODEPRECATED_DECLARATIONS_PUSH
                // "'setView:' is deprecated: first deprecated in macOS 10.14 - Use the standard
                // button property instead"
            [pData->mpStatusItem setView: pView ];
SAL_WNODEPRECATED_DECLARATIONS_POP
            [pView display];
        }
    }
    return pData->mpStatusItem;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
