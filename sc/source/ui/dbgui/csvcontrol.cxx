/*************************************************************************
 *
 *  $RCSfile: csvcontrol.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:51 $
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
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SC_ACCESSIBLECSVCONTROL_HXX
#include "AccessibleCsvControl.hxx"
#endif


// ============================================================================

ScCsvLayoutData::ScCsvLayoutData() :
    mnPosCount( 1 ),
    mnPosOffset( 0 ),
    mnWinWidth( 1 ),
    mnHdrWidth( 0 ),
    mnCharWidth( 1 ),
    mnLineCount( 1 ),
    mnLineOffset( 0 ),
    mnWinHeight( 1 ),
    mnHdrHeight( 0 ),
    mnLineHeight( 1 ),
    mnPosCursor( CSV_POS_INVALID ),
    mnColCursor( 0 ),
    mnNoRepaint( 0 ),
    mbAppRTL( !!Application::GetSettings().GetLayoutRTL() )
{
}

ScCsvDiff ScCsvLayoutData::GetDiff( const ScCsvLayoutData& rData ) const
{
    ScCsvDiff nRet = CSV_DIFF_EQUAL;
    if( mnPosCount != rData.mnPosCount )        nRet |= CSV_DIFF_POSCOUNT;
    if( mnPosOffset != rData.mnPosOffset )      nRet |= CSV_DIFF_POSOFFSET;
    if( mnHdrWidth != rData.mnHdrWidth )        nRet |= CSV_DIFF_HDRWIDTH;
    if( mnCharWidth != rData.mnCharWidth )      nRet |= CSV_DIFF_CHARWIDTH;
    if( mnLineCount != rData.mnLineCount )      nRet |= CSV_DIFF_LINECOUNT;
    if( mnLineOffset != rData.mnLineOffset )    nRet |= CSV_DIFF_LINEOFFSET;
    if( mnHdrHeight != rData.mnHdrHeight )      nRet |= CSV_DIFF_HDRHEIGHT;
    if( mnLineHeight != rData.mnLineHeight )    nRet |= CSV_DIFF_LINEHEIGHT;
    if( mnPosCursor != rData.mnPosCursor )      nRet |= CSV_DIFF_RULERCURSOR;
    if( mnColCursor != rData.mnColCursor )      nRet |= CSV_DIFF_GRIDCURSOR;
    return nRet;
}


// ============================================================================

ScCsvControl::ScCsvControl( ScCsvControl& rParent ) :
    Control( &rParent, WB_TABSTOP | WB_NODIALOGCONTROL ),
    mrData( rParent.GetLayoutData() ),
    mpAccessible( NULL ),
    mbValidGfx( false )
{
}

ScCsvControl::ScCsvControl( Window* pParent, const ScCsvLayoutData& rData, WinBits nStyle ) :
    Control( pParent, nStyle ),
    mrData( rData ),
    mpAccessible( NULL ),
    mbValidGfx( false )
{
}

ScCsvControl::ScCsvControl( Window* pParent, const ScCsvLayoutData& rData, const ResId& rResId ) :
    Control( pParent, rResId ),
    mrData( rData ),
    mpAccessible( NULL ),
    mbValidGfx( false )
{
}

ScCsvControl::~ScCsvControl()
{
    if( mpAccessible )
        mpAccessible->dispose();
}


// event handling -------------------------------------------------------------

void ScCsvControl::GetFocus()
{
    Control::GetFocus();
    AccSendFocusEvent( true );
}

void ScCsvControl::LoseFocus()
{
    Control::LoseFocus();
    AccSendFocusEvent( false );
}

void ScCsvControl::AccSendFocusEvent( bool bFocused )
{
    if( mpAccessible )
        mpAccessible->SendFocusEvent( bFocused );
}

void ScCsvControl::AccSendCaretEvent()
{
    if( mpAccessible )
        mpAccessible->SendCaretEvent();
}

void ScCsvControl::AccSendVisibleEvent()
{
    if( mpAccessible )
        mpAccessible->SendVisibleEvent();
}

void ScCsvControl::AccSendSelectionEvent()
{
    if( mpAccessible )
        mpAccessible->SendSelectionEvent();
}

void ScCsvControl::AccSendTableUpdateEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn, bool bAllRows )
{
    if( mpAccessible )
        mpAccessible->SendTableUpdateEvent( nFirstColumn, nLastColumn, bAllRows );
}

void ScCsvControl::AccSendInsertColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn )
{
    if( mpAccessible )
        mpAccessible->SendInsertColumnEvent( nFirstColumn, nLastColumn );
}

void ScCsvControl::AccSendRemoveColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn )
{
    if( mpAccessible )
        mpAccessible->SendRemoveColumnEvent( nFirstColumn, nLastColumn );
}


// repaint helpers ------------------------------------------------------------

void ScCsvControl::Repaint( bool bInvalidate )
{
    if( bInvalidate )
        InvalidateGfx();
    if( !IsNoRepaint() )
        Execute( CSVCMD_REPAINT );
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


// command handling -----------------------------------------------------------

void ScCsvControl::Execute( ScCsvCmdType eType, sal_Int32 nParam1, sal_Int32 nParam2 )
{
    maCmd.Set( eType, nParam1, nParam2 );
    maCmdHdl.Call( this );
}


// layout helpers -------------------------------------------------------------

sal_Int32 ScCsvControl::GetVisPosCount() const
{
    return (mrData.mnWinWidth - GetHdrWidth()) / GetCharWidth();
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

sal_Int32 ScCsvControl::GetHdrX() const
{
    return IsRTL() ? (mrData.mnWinWidth - GetHdrWidth()) : 0;
}

sal_Int32 ScCsvControl::GetFirstX() const
{
    return IsRTL() ? 0 : GetHdrWidth();
}

sal_Int32 ScCsvControl::GetLastX() const
{
    return mrData.mnWinWidth - (IsRTL() ? GetHdrWidth() : 0) - 1;
}

sal_Int32 ScCsvControl::GetX( sal_Int32 nPos ) const
{
    return GetFirstX() + (nPos - GetFirstVisPos()) * GetCharWidth();
}

sal_Int32 ScCsvControl::GetPosFromX( sal_Int32 nX ) const
{
    return (nX - GetFirstX() + GetCharWidth() / 2) / GetCharWidth() + GetFirstVisPos();
}

sal_Int32 ScCsvControl::GetVisLineCount() const
{
    return (mrData.mnWinHeight - GetHdrHeight() - 2) / GetLineHeight() + 1;
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
    return GetHdrHeight() + (nLine - GetFirstVisLine()) * GetLineHeight();
}

sal_Int32 ScCsvControl::GetLineFromY( sal_Int32 nY ) const
{
    return (nY - GetHdrHeight()) / GetLineHeight() + GetFirstVisLine();
}


// static helpers -------------------------------------------------------------

void ScCsvControl::ImplInvertRect( OutputDevice& rOutDev, const Rectangle& rRect )
{
    RasterOp eOldOp = rOutDev.GetRasterOp();
    rOutDev.SetRasterOp( ROP_INVERT );
    rOutDev.DrawRect( rRect );
    rOutDev.SetRasterOp( eOldOp );
}

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


// accessibility --------------------------------------------------------------

ScCsvControl::XAccessibleRef ScCsvControl::CreateAccessible()
{
    mpAccessible = ImplCreateAccessible();
    mxAccessible = mpAccessible;
    return mxAccessible;
}


// ============================================================================

