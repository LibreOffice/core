/*************************************************************************
 *
 *  $RCSfile: csvcontrol.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dr $ $Date: 2002-07-05 15:47:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif
#pragma hdrstop

// ============================================================================

#ifndef _SC_CSVCONTROL_HXX
#include "csvcontrol.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


// ============================================================================

ScCsvLayoutData::ScCsvLayoutData() :
    mnPosCount( 1 ),
    mnPosOffset( 0 ),
    mnWinWidth( 1 ),
    mnOffsetX( 0 ),
    mnCharWidth( 1 ),
    mnLineCount( 1 ),
    mnLineOffset( 0 ),
    mnWinHeight( 1 ),
    mnOffsetY( 0 ),
    mnLineHeight( 1 ),
    mnPosCursor( POS_INVALID ),
    mnColCursor( 0 ),
    mnNoRepaint( 0 )
{
}

bool ScCsvLayoutData::IsHorzEqual( const ScCsvLayoutData& rData ) const
{
    return  (mnPosCount == rData.mnPosCount) &&
            (mnPosOffset == rData.mnPosOffset) &&
            (mnOffsetX == rData.mnOffsetX) &&
            (mnCharWidth == rData.mnCharWidth);
}

bool ScCsvLayoutData::IsVertEqual( const ScCsvLayoutData& rData ) const
{
    return  (mnLineCount == rData.mnLineCount) &&
            (mnLineOffset == rData.mnLineOffset) &&
            (mnOffsetY == rData.mnOffsetY) &&
            (mnLineHeight == rData.mnLineHeight);
}

bool ScCsvLayoutData::IsCursorEqual( const ScCsvLayoutData& rData ) const
{
    return  (mnPosCursor == rData.mnPosCursor) &&
            (mnColCursor == rData.mnColCursor);
}

bool operator==( const ScCsvLayoutData& rData1, const ScCsvLayoutData& rData2 )
{
    return  rData1.IsHorzEqual( rData2 ) &&
            rData1.IsVertEqual( rData2 ) &&
            rData1.IsCursorEqual( rData2 );
}


// ============================================================================

ScCsvControl::ScCsvControl( ScCsvControl& rParent ) :
    Control( &rParent, WB_TABSTOP | WB_NODIALOGCONTROL ),
    mrData( rParent.GetLayoutData() ),
    mbValidGfx( false )
{
}

ScCsvControl::ScCsvControl( Window* pParent, const ScCsvLayoutData& rData, WinBits nStyle ) :
    Control( pParent, nStyle ),
    mrData( rData ),
    mbValidGfx( false )
{
}

ScCsvControl::ScCsvControl( Window* pParent, const ScCsvLayoutData& rData, const ResId& rResId ) :
    Control( pParent, rResId ),
    mrData( rData ),
    mbValidGfx( false )
{
}


// drawing --------------------------------------------------------------------

void ScCsvControl::Repaint( bool bInvalidate )
{
    if( bInvalidate )
        InvalidateGfx();
    if( !IsNoRepaint() )
        CommitRequest( CSVREQ_REPAINT );
}

void ScCsvControl::DisableRepaint()
{
    ++mrData.mnNoRepaint;
}

void ScCsvControl::EnableRepaint( bool bInvalidate )
{
    DBG_ASSERT( IsNoRepaint(), "ScCsvControl::EnableRepaint - invalid call" );
    --mrData.mnNoRepaint;
    Repaint( bInvalidate );
}

void ScCsvControl::ImplInvertRect( OutputDevice& rOutDev, const Rectangle& rRect )
{
    RasterOp eOldOp = rOutDev.GetRasterOp();
    rOutDev.SetRasterOp( ROP_INVERT );
    rOutDev.DrawRect( rRect );
    rOutDev.SetRasterOp( eOldOp );
}


// event handling -------------------------------------------------------------

void ScCsvControl::CommitRequest( ScCsvRequestType eType, sal_Int32 nData )
{
    maRequest.Set( eType, nData );
    maRequestHdl.Call( this );
}

void ScCsvControl::CommitEvent( ScCsvEventType eType, sal_Int32 nPos, sal_Int32 nOldPos )
{
    maEvent.Set( eType, nPos, nOldPos );
    maEventHdl.Call( this );
}

// layout helpers -------------------------------------------------------------

sal_Int32 ScCsvControl::GetVisPosCount() const
{
    return (mrData.mnWinWidth - GetOffsetX()) / GetCharWidth() + 1;
}

sal_Int32 ScCsvControl::GetMaxPosOffset() const
{
    return Max( GetPosCount() - GetVisPosCount() + 2L, 0L );
}

bool ScCsvControl::IsValidSplitPos( sal_Int32 nPos ) const
{
    return (0 < nPos) && (nPos < GetPosCount() );
}

bool ScCsvControl::IsVisibleSplitPos( sal_Int32 nPos ) const
{
    return IsValidSplitPos( nPos ) && (GetFirstVisPos() <= nPos) && (nPos <= GetLastVisPos());
}

sal_Int32 ScCsvControl::GetX( sal_Int32 nPos ) const
{
    return GetOffsetX() + (nPos - GetFirstVisPos()) * GetCharWidth();
}

sal_Int32 ScCsvControl::GetPosFromX( sal_Int32 nX ) const
{
    return (nX - GetOffsetX() + GetCharWidth() / 2) / GetCharWidth() + GetFirstVisPos();
}

sal_Int32 ScCsvControl::GetVisLineCount() const
{
    return (mrData.mnWinHeight - GetOffsetY() - 2) / GetLineHeight() + 1;
}

sal_Int32 ScCsvControl::GetLastVisLine() const
{
    return Min( GetFirstVisLine() + GetVisLineCount(), GetLineCount() ) - 1;
}

sal_Int32 ScCsvControl::GetMaxLineOffset() const
{
    return Max( GetLineCount() - GetVisLineCount() + 1L, 0L );
}

bool ScCsvControl::IsValidLine( sal_Int32 nLine ) const
{
    return (0 <= nLine) && (nLine < GetLineCount());
}

bool ScCsvControl::IsVisibleLine( sal_Int32 nLine ) const
{
    return IsValidLine( nLine ) && (GetFirstVisLine() <= nLine) && (nLine <= GetLastVisLine());
}

sal_Int32 ScCsvControl::GetY( sal_Int32 nLine ) const
{
    return GetOffsetY() + (nLine - GetFirstVisLine()) * GetLineHeight();
}


// keyboard helpers -----------------------------------------------------------

ScMoveMode ScCsvControl::GetHorzDirection( sal_uInt16 nCode, bool bHomeEnd )
{
    switch( nCode )
    {
        case KEY_LEFT:  return MOVE_PREV;
        case KEY_RIGHT: return MOVE_NEXT;
    }
    if( bHomeEnd ) switch( nCode )
    {
        case KEY_HOME:  return MOVE_FIRST;
        case KEY_END:   return MOVE_LAST;
    }
    return MOVE_NONE;
}

ScMoveMode ScCsvControl::GetVertDirection( sal_uInt16 nCode, bool bHomeEnd )
{
    switch( nCode )
    {
        case KEY_UP:        return MOVE_PREV;
        case KEY_DOWN:      return MOVE_NEXT;
        case KEY_PAGEUP:    return MOVE_PREVPAGE;
        case KEY_PAGEDOWN:  return MOVE_NEXTPAGE;
    }
    if( bHomeEnd ) switch( nCode )
    {
        case KEY_HOME:      return MOVE_FIRST;
        case KEY_END:       return MOVE_LAST;
    }
    return MOVE_NONE;
}


// ============================================================================

