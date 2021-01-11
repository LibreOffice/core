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
#include <tools/stream.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/settings.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <sal/log.hxx>
#include <bitmaps.hlst>
#include <cursor_hotspots.hxx>

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
    mxRGBSpace( CGColorSpaceCreateWithName(kCGColorSpaceSRGB) ),
    mxGraySpace( CGColorSpaceCreateWithName(kCGColorSpaceGenericGrayGamma2_2) ),
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

namespace {

NSImage* load_icon_by_name(const OUString& rIconName)
{
    OUString sIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
    OUString sUILang = Application::GetSettings().GetUILanguageTag().getBcp47();
    auto xMemStm = ImageTree::get().getImageStream(rIconName, sIconTheme, sUILang);
    if (!xMemStm)
        return nullptr;

    auto data = xMemStm->GetData();
    auto length = xMemStm->TellEnd();
    NSData * byteData = [NSData dataWithBytes:data length:length];
    NSBitmapImageRep * imageRep = [NSBitmapImageRep imageRepWithData:byteData];
    NSSize imageSize = NSMakeSize(CGImageGetWidth([imageRep CGImage]), CGImageGetHeight([imageRep CGImage]));

    NSImage * image = [[NSImage alloc] initWithSize:imageSize];
    [image addRepresentation:imageRep];
    return image;
}

}

#define MAKE_CURSOR( vcl_name, name, name2 ) \
    case vcl_name: \
        aHotSpot = NSPoint{name##curs_x_hot, name##curs_y_hot}; \
        aIconName = name2; \
        break

NSCursor* SalData::getCursor( PointerStyle i_eStyle )
{
    NSCursor* pCurs = maCursors[ i_eStyle ];
    if( pCurs != INVALID_CURSOR_PTR )
        return pCurs;

    NSPoint aHotSpot;
    OUString aIconName;

    switch( i_eStyle )
    {
        // TODO
        MAKE_CURSOR( PointerStyle::Wait, wait_, RID_CURSOR_WAIT );
        MAKE_CURSOR( PointerStyle::NWSize, nwsize_, RID_CURSOR_NWSIZE );
        MAKE_CURSOR( PointerStyle::NESize, nesize_, RID_CURSOR_NESIZE );
        MAKE_CURSOR( PointerStyle::SWSize, swsize_, RID_CURSOR_SWSIZE );
        MAKE_CURSOR( PointerStyle::SESize, sesize_, RID_CURSOR_SESIZE );
        MAKE_CURSOR( PointerStyle::WindowNWSize, window_nwsize_, RID_CURSOR_WINDOW_NWSIZE );
        MAKE_CURSOR( PointerStyle::WindowNESize, window_nesize_, RID_CURSOR_WINDOW_NESIZE );
        MAKE_CURSOR( PointerStyle::WindowSWSize, window_swsize_, RID_CURSOR_WINDOW_SWSIZE );
        MAKE_CURSOR( PointerStyle::WindowSESize, window_sesize_, RID_CURSOR_WINDOW_SESIZE );

        MAKE_CURSOR( PointerStyle::Help, help_, RID_CURSOR_HELP );
        MAKE_CURSOR( PointerStyle::Pen, pen_, RID_CURSOR_PEN );
        MAKE_CURSOR( PointerStyle::Null, null, RID_CURSOR_NULL );
        MAKE_CURSOR( PointerStyle::Magnify, magnify_, RID_CURSOR_MAGNIFY );
        MAKE_CURSOR( PointerStyle::Fill, fill_, RID_CURSOR_FILL );
        MAKE_CURSOR( PointerStyle::MoveData, movedata_, RID_CURSOR_MOVE_DATA );
        MAKE_CURSOR( PointerStyle::CopyData, copydata_, RID_CURSOR_COPY_DATA );
        MAKE_CURSOR( PointerStyle::MoveFile, movefile_, RID_CURSOR_MOVE_FILE );
        MAKE_CURSOR( PointerStyle::CopyFile, copyfile_, RID_CURSOR_COPY_FILE );
        MAKE_CURSOR( PointerStyle::MoveFiles, movefiles_, RID_CURSOR_MOVE_FILES );
        MAKE_CURSOR( PointerStyle::CopyFiles, copyfiles_, RID_CURSOR_COPY_FILES );
        MAKE_CURSOR( PointerStyle::NotAllowed, nodrop_, RID_CURSOR_NOT_ALLOWED );
        MAKE_CURSOR( PointerStyle::Rotate, rotate_, RID_CURSOR_ROTATE );
        MAKE_CURSOR( PointerStyle::HShear, hshear_, RID_CURSOR_H_SHEAR );
        MAKE_CURSOR( PointerStyle::VShear, vshear_, RID_CURSOR_V_SHEAR );
        MAKE_CURSOR( PointerStyle::DrawLine, drawline_, RID_CURSOR_DRAW_LINE );
        MAKE_CURSOR( PointerStyle::DrawRect, drawrect_, RID_CURSOR_DRAW_RECT );
        MAKE_CURSOR( PointerStyle::DrawPolygon, drawpolygon_, RID_CURSOR_DRAW_POLYGON );
        MAKE_CURSOR( PointerStyle::DrawBezier, drawbezier_, RID_CURSOR_DRAW_BEZIER );
        MAKE_CURSOR( PointerStyle::DrawArc, drawarc_, RID_CURSOR_DRAW_ARC );
        MAKE_CURSOR( PointerStyle::DrawPie, drawpie_, RID_CURSOR_DRAW_PIE );
        MAKE_CURSOR( PointerStyle::DrawCircleCut, drawcirclecut_, RID_CURSOR_DRAW_CIRCLE_CUT );
        MAKE_CURSOR( PointerStyle::DrawEllipse, drawellipse_, RID_CURSOR_DRAW_ELLIPSE );
        MAKE_CURSOR( PointerStyle::DrawConnect, drawconnect_, RID_CURSOR_DRAW_CONNECT );
        MAKE_CURSOR( PointerStyle::DrawText, drawtext_, RID_CURSOR_DRAW_TEXT );
        MAKE_CURSOR( PointerStyle::Mirror, mirror_, RID_CURSOR_MIRROR );
        MAKE_CURSOR( PointerStyle::Crook, crook_, RID_CURSOR_CROOK );
        MAKE_CURSOR( PointerStyle::Crop, crop_, RID_CURSOR_CROP );
        MAKE_CURSOR( PointerStyle::MovePoint, movepoint_, RID_CURSOR_MOVE_POINT );
        MAKE_CURSOR( PointerStyle::MoveBezierWeight, movebezierweight_, RID_CURSOR_MOVE_BEZIER_WEIGHT );
        MAKE_CURSOR( PointerStyle::DrawFreehand, drawfreehand_, RID_CURSOR_DRAW_FREEHAND );
        MAKE_CURSOR( PointerStyle::DrawCaption, drawcaption_, RID_CURSOR_DRAW_CAPTION );
        MAKE_CURSOR( PointerStyle::LinkData, linkdata_, RID_CURSOR_LINK_DATA );
        MAKE_CURSOR( PointerStyle::MoveDataLink, movedlnk_, RID_CURSOR_MOVE_DATA_LINK );
        MAKE_CURSOR( PointerStyle::CopyDataLink, copydlnk_, RID_CURSOR_COPY_DATA_LINK );
        MAKE_CURSOR( PointerStyle::LinkFile, linkfile_, RID_CURSOR_LINK_FILE );
        MAKE_CURSOR( PointerStyle::MoveFileLink, moveflnk_, RID_CURSOR_MOVE_FILE_LINK );
        MAKE_CURSOR( PointerStyle::CopyFileLink, copyflnk_, RID_CURSOR_COPY_FILE_LINK );
        MAKE_CURSOR( PointerStyle::Chart, chart_, RID_CURSOR_CHART );
        MAKE_CURSOR( PointerStyle::Detective, detective_, RID_CURSOR_DETECTIVE );
        MAKE_CURSOR( PointerStyle::PivotCol, pivotcol_, RID_CURSOR_PIVOT_COLUMN );
        MAKE_CURSOR( PointerStyle::PivotRow, pivotrow_, RID_CURSOR_PIVOT_ROW );
        MAKE_CURSOR( PointerStyle::PivotField, pivotfld_, RID_CURSOR_PIVOT_FIELD );
        MAKE_CURSOR( PointerStyle::PivotDelete, pivotdel_, RID_CURSOR_PIVOT_DELETE );
        MAKE_CURSOR( PointerStyle::Chain, chain_, RID_CURSOR_CHAIN );
        MAKE_CURSOR( PointerStyle::ChainNotAllowed, chainnot_, RID_CURSOR_CHAIN_NOT_ALLOWED );
        MAKE_CURSOR( PointerStyle::AutoScrollN, asn_, RID_CURSOR_AUTOSCROLL_N );
        MAKE_CURSOR( PointerStyle::AutoScrollS, ass_, RID_CURSOR_AUTOSCROLL_S );
        MAKE_CURSOR( PointerStyle::AutoScrollW, asw_, RID_CURSOR_AUTOSCROLL_W );
        MAKE_CURSOR( PointerStyle::AutoScrollE, ase_, RID_CURSOR_AUTOSCROLL_E );
        MAKE_CURSOR( PointerStyle::AutoScrollNW, asnw_, RID_CURSOR_AUTOSCROLL_NW );
        MAKE_CURSOR( PointerStyle::AutoScrollNE, asne_, RID_CURSOR_AUTOSCROLL_NE );
        MAKE_CURSOR( PointerStyle::AutoScrollSW, assw_, RID_CURSOR_AUTOSCROLL_SW );
        MAKE_CURSOR( PointerStyle::AutoScrollSE, asse_, RID_CURSOR_AUTOSCROLL_SE );
        MAKE_CURSOR( PointerStyle::AutoScrollNS, asns_, RID_CURSOR_AUTOSCROLL_NS );
        MAKE_CURSOR( PointerStyle::AutoScrollWE, aswe_, RID_CURSOR_AUTOSCROLL_WE );
        MAKE_CURSOR( PointerStyle::AutoScrollNSWE, asnswe_, RID_CURSOR_AUTOSCROLL_NSWE );
        MAKE_CURSOR( PointerStyle::TextVertical, vertcurs_, RID_CURSOR_TEXT_VERTICAL );

        // #i32329#
        MAKE_CURSOR( PointerStyle::TabSelectS, tblsels_, RID_CURSOR_TAB_SELECT_S );
        MAKE_CURSOR( PointerStyle::TabSelectE, tblsele_, RID_CURSOR_TAB_SELECT_E );
        MAKE_CURSOR( PointerStyle::TabSelectSE, tblselse_, RID_CURSOR_TAB_SELECT_SE );
        MAKE_CURSOR( PointerStyle::TabSelectW, tblselw_, RID_CURSOR_TAB_SELECT_W );
        MAKE_CURSOR( PointerStyle::TabSelectSW, tblselsw_, RID_CURSOR_TAB_SELECT_SW );

        MAKE_CURSOR( PointerStyle::HideWhitespace, hidewhitespace_, RID_CURSOR_HIDE_WHITESPACE );
        MAKE_CURSOR( PointerStyle::ShowWhitespace, showwhitespace_, RID_CURSOR_SHOW_WHITESPACE );

    default:
        SAL_WARN( "vcl", "pointer style " << static_cast<sal_Int32>(i_eStyle) << "not implemented" );
        assert( false && "pointer style not implemented" );
        break;
    }

    NSImage* theImage = load_icon_by_name(aIconName);
    assert ([theImage size].width == 256 || [theImage size].width == 128 || [theImage size].width == 32);
    if ([theImage size].width == 256 || [theImage size].width == 128)
    {
        // If we have a 256x256 or 128x128 image, generate scaled versions of it.
        // This will result in macOS picking a reasonably sized image for different screen dpi.
        NSSize cursorSize = NSMakeSize(32,32);
        NSImage *multiResImage = [[NSImage alloc] initWithSize:cursorSize];
        for (int scale = 1; scale <= 4; scale++) {
            NSAffineTransform *xform = [[NSAffineTransform alloc] init];
            [xform scaleBy:scale];
            id hints = @{ NSImageHintCTM: xform };
            CGImageRef rasterCGImage = [theImage CGImageForProposedRect:nullptr context:nil hints:hints];
            NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithCGImage:rasterCGImage];
            [rep setSize:cursorSize];
            [multiResImage addRepresentation:rep];
        }
        pCurs = [[NSCursor alloc] initWithImage: multiResImage hotSpot: aHotSpot];
    }
    else
        pCurs = [[NSCursor alloc] initWithImage: theImage hotSpot: aHotSpot];

    maCursors[ i_eStyle ] = pCurs;
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
