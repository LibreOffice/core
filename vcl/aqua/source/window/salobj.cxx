/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salobj.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:49:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <string.h>

#include "saldata.hxx"
#include "salobj.h"
#include "salframe.h"

// get QTMovieView
#include "premac.h"
#include <QTKit/QTMovieView.h>
#include "postmac.h"

// =======================================================================

AquaSalObject::AquaSalObject( AquaSalFrame* pFrame ) :
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

    NSRect aInitFrame = { { 0, 0 }, { 20, 20 } };
    mpClipView = [[NSClipView alloc] initWithFrame: aInitFrame ];
    if( mpClipView )
    {
        [mpFrame->getView() addSubview: mpClipView];
        [mpClipView setHidden: YES];
    }
    maSysData.pView = [[QTMovieView alloc] initWithFrame: aInitFrame];
    if( maSysData.pView )
    {
        if( mpClipView )
            [mpClipView setDocumentView: maSysData.pView];
    }
}

// -----------------------------------------------------------------------

AquaSalObject::~AquaSalObject()
{
    if( maSysData.pView )
    {
        [maSysData.pView removeFromSuperview];
        [maSysData.pView release];
    }
    if( mpClipView )
    {
        [mpClipView removeFromSuperview];
        [mpClipView release];
    }
}

/*
   sadly there seems to be no way to impose clipping on a child view,
   especially a QTMovieView which seems to ignore the current context
   completely. Also there is no real way to shape a window; on Aqua a
   similar effect to non-rectangular windows is achieved by using a
   non-opaque window and not painting where one wants the background
   to shine through.

   With respect to SalObject this leaves us to having an NSClipView
   containing the child view. Even a QTMovieView respects the boundaries of
   that, which gives us a clip "region" consisting of one rectangle.
   This is gives us an 80% solution only, though.
*/

// -----------------------------------------------------------------------

void AquaSalObject::ResetClipRegion()
{
    mbClip = false;
    setClippedPosSize();
}

// -----------------------------------------------------------------------

USHORT AquaSalObject::GetClipRegionType()
{
    return SAL_OBJECT_CLIP_INCLUDERECTS;
}

// -----------------------------------------------------------------------

void AquaSalObject::BeginSetClipRegion( ULONG nRectCount )
{
    mbClip = false;
}

// -----------------------------------------------------------------------

void AquaSalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
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

void AquaSalObject::EndSetClipRegion()
{
    setClippedPosSize();
}

// -----------------------------------------------------------------------

void AquaSalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
    mnX = nX;
    mnY = nY;
    mnWidth = nWidth;
    mnHeight = nHeight;
    setClippedPosSize();
}

// -----------------------------------------------------------------------

void AquaSalObject::setClippedPosSize()
{
    NSRect aViewRect = { { 0, 0 }, { mnWidth, mnHeight } };
    if( maSysData.pView )
        [maSysData.pView setFrame: aViewRect];

    NSRect aClipViewRect = { { mnX, mnY }, { mnWidth, mnHeight } };
    NSPoint aClipPt = { 0, 0 };
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

    mpFrame->VCLToCocoa( aClipViewRect, false );
    [mpClipView setFrame: aClipViewRect];

    [mpClipView scrollToPoint: aClipPt];
}

// -----------------------------------------------------------------------

void AquaSalObject::Show( BOOL bVisible )
{
    if( mpClipView )
        [mpClipView setHidden: (bVisible ? NO : YES)];
}

// -----------------------------------------------------------------------

void AquaSalObject::Enable( BOOL bEnable )
{
}

// -----------------------------------------------------------------------

void AquaSalObject::GrabFocus()
{
}

// -----------------------------------------------------------------------

void AquaSalObject::SetBackground()
{
}

// -----------------------------------------------------------------------

void AquaSalObject::SetBackground( SalColor nSalColor )
{
}

// -----------------------------------------------------------------------

const SystemEnvData* AquaSalObject::GetSystemData() const
{
    return &maSysData;
}

