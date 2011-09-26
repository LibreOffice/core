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

#include <string.h>

#include "ios/saldata.hxx"
#include "ios/salobj.h"
#include "ios/salframe.h"

// =======================================================================

IosSalObject::IosSalObject( IosSalFrame* pFrame ) :
    mpFrame( pFrame ),
    mnClipX( -1 ),
    mnClipY( -1 ),
    mnClipWidth( -1 ),
    mnClipHeight( -1 ),
    mbClip( false ),
    mnX( 0 ),
    mnY( 0 ),
    mnWidth( 20 ),
    mnHeight( 20 )
{
    maSysData.nSize = sizeof( maSysData );
    maSysData.pView = NULL;

    CGRect aInitFrame = { { 0, 0 }, { 20, 20 } };
    maSysData.pView = [[UIView alloc] initWithFrame: aInitFrame];
}

// -----------------------------------------------------------------------

IosSalObject::~IosSalObject()
{
    if( maSysData.pView )
    {
        UIView *pView = maSysData.pView;
        [pView removeFromSuperview];
        [pView release];
    }
#if 0 // ???
    if( mpClipView )
    {
        [mpClipView removeFromSuperview];
        [mpClipView release];
    }
#endif
}

/*
   sadly there seems to be no way to impose clipping on a child view,
   especially a QTMovieView which seems to ignore the current context
   completely. Also there is no real way to shape a window; on Ios a
   similar effect to non-rectangular windows is achieved by using a
   non-opaque window and not painting where one wants the background
   to shine through.

   With respect to SalObject this leaves us to having an NSClipView
   containing the child view. Even a QTMovieView respects the boundaries of
   that, which gives us a clip "region" consisting of one rectangle.
   This is gives us an 80% solution only, though.
*/

// -----------------------------------------------------------------------

void IosSalObject::ResetClipRegion()
{
    mbClip = false;
    setClippedPosSize();
}

// -----------------------------------------------------------------------

sal_uInt16 IosSalObject::GetClipRegionType()
{
    return SAL_OBJECT_CLIP_INCLUDERECTS;
}

// -----------------------------------------------------------------------

void IosSalObject::BeginSetClipRegion( sal_uLong )
{
    mbClip = false;
}

// -----------------------------------------------------------------------

void IosSalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    if( mbClip )
    {
        if( nX < mnClipX )
        {
            mnClipWidth += mnClipX - nX;
            mnClipX = nX;
        }
        if( nX + nWidth > mnClipX + mnClipWidth )
            mnClipWidth = nX + nWidth - mnClipX;
        if( nY < mnClipY )
        {
            mnClipHeight += mnClipY - nY;
            mnClipY = nY;
        }
        if( nY + nHeight > mnClipY + mnClipHeight )
            mnClipHeight = nY + nHeight - mnClipY;
    }
    else
    {
        mnClipX = nX;
        mnClipY = nY;
        mnClipWidth = nWidth;
        mnClipHeight = nHeight;
        mbClip = true;
    }
}

// -----------------------------------------------------------------------

void IosSalObject::EndSetClipRegion()
{
    setClippedPosSize();
}

// -----------------------------------------------------------------------

void IosSalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
    mnX = nX;
    mnY = nY;
    mnWidth = nWidth;
    mnHeight = nHeight;
    setClippedPosSize();
}

// -----------------------------------------------------------------------

void IosSalObject::setClippedPosSize()
{
    CGRect aViewRect = { { 0, 0 }, { mnWidth, mnHeight } };
    if( maSysData.pView )
    {
        UIView *pView = maSysData.pView;
        [pView setFrame: aViewRect];
    }

    CGRect aClipViewRect = { { mnX, mnY }, { mnWidth, mnHeight } };
    CGPoint aClipPt = { 0, 0 };
    if( mbClip )
    {
        aClipViewRect.origin.x += mnClipX;
        aClipViewRect.origin.y += mnClipY;
        aClipViewRect.size.width = mnClipWidth;
        aClipViewRect.size.height = mnClipHeight;
        aClipPt.x = mnClipX;
        if( mnClipY == 0 )
            aClipPt.y = mnHeight - mnClipHeight;;
    }

    mpFrame->VCLToCocoaTouch( aClipViewRect, false );
#if 0 // ???
    [mpClipView setFrame: aClipViewRect];

    [mpClipView scrollToPoint: aClipPt];
#endif
}

// -----------------------------------------------------------------------

void IosSalObject::Show( sal_Bool /*bVisible*/ )
{
#if 0 // ???
    if( mpClipView )
        [mpClipView setHidden: (bVisible ? NO : YES)];
#endif
}

// -----------------------------------------------------------------------

void IosSalObject::Enable( sal_Bool )
{
}

// -----------------------------------------------------------------------

void IosSalObject::GrabFocus()
{
}

// -----------------------------------------------------------------------

void IosSalObject::SetBackground()
{
}

// -----------------------------------------------------------------------

void IosSalObject::SetBackground( SalColor )
{
}

// -----------------------------------------------------------------------

const SystemEnvData* IosSalObject::GetSystemData() const
{
    return &maSysData;
}

// -----------------------------------------------------------------------

void IosSalObject::InterceptChildWindowKeyDown( sal_Bool /*bIntercept*/ )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
