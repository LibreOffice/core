/*************************************************************************
 *
 *  $RCSfile: csvtablebox.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2002-07-12 09:05:17 $
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

#ifndef _SC_CSVTABLEBOX_HXX
#include "csvtablebox.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif


// ============================================================================

ScCsvTableBox::ScCsvTableBox( Window* pParent ) :
    ScCsvControl( pParent, maData, WB_BORDER | WB_TABSTOP | WB_DIALOGCONTROL ),
    maRuler( *this ),
    maGrid( *this ),
    maHScroll( this, WB_HORZ | WB_DRAG ),
    maVScroll( this, WB_VERT | WB_DRAG ),
    maScrollBox( this )
{
    Init();
}

ScCsvTableBox::ScCsvTableBox( Window* pParent, const ResId& rResId ) :
    ScCsvControl( pParent, maData, rResId ),
    maRuler( *this ),
    maGrid( *this ),
    maHScroll( this, WB_HORZ | WB_DRAG ),
    maVScroll( this, WB_VERT | WB_DRAG ),
    maScrollBox( this )
{
    Init();
}


// initialization -------------------------------------------------------------

void ScCsvTableBox::Init()
{
    mbFixedMode = false;
    mnSelColType = 0;
    mnFixedWidth = 1;

    maHScroll.SetLineSize( 1 );
    maVScroll.SetLineSize( 1 );

    Link aLink = LINK( this, ScCsvTableBox, CsvRequestHdl );
    SetRequestHdl( aLink );
    maRuler.SetRequestHdl( aLink );
    maGrid.SetRequestHdl( aLink );

    aLink = LINK( this, ScCsvTableBox, CsvEventHdl );
    SetEventHdl( aLink );
    maRuler.SetEventHdl( aLink );
    maGrid.SetEventHdl( aLink );

    aLink = LINK( this, ScCsvTableBox, ScrollHdl );
    maHScroll.SetScrollHdl( aLink );
    maVScroll.SetScrollHdl( aLink );

    aLink = LINK( this, ScCsvTableBox, ScrollEndHdl );
    maHScroll.SetEndScrollHdl( aLink );
    maVScroll.SetEndScrollHdl( aLink );

    InitControls();
}

void ScCsvTableBox::InitControls()
{
    maGrid.UpdateLayoutData();

    sal_Int32 nScrollBarSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    Size aWinSize = CalcOutputSize( GetSizePixel() );
    sal_Int32 nDataWidth = aWinSize.Width() - nScrollBarSize;
    sal_Int32 nDataHeight = aWinSize.Height() - nScrollBarSize;

    maData.mnWinWidth = nDataWidth;
    maData.mnWinHeight = nDataHeight;

    if( mbFixedMode )
    {
        // ruler sets height internally
        maRuler.SetPosSizePixel( 0, 0, nDataWidth, 0 );
        sal_Int32 nY = maRuler.GetSizePixel().Height();
        maData.mnWinHeight -= nY;
        maGrid.SetPosSizePixel( 0, nY, nDataWidth, maData.mnWinHeight );
    }
    else
        maGrid.SetPosSizePixel( 0, 0, nDataWidth, nDataHeight );
    maGrid.Show();
    maRuler.Show( mbFixedMode );

    maHScroll.SetPosSizePixel( 0, nDataHeight, nDataWidth, nScrollBarSize );
    InitHScrollBar();
    maHScroll.Show();

    maVScroll.SetPosSizePixel( nDataWidth, 0, nScrollBarSize, nDataHeight );
    InitVScrollBar();
    maVScroll.Show();

    bool bScrBox = maHScroll.IsVisible() && maVScroll.IsVisible();
    if( bScrBox )
        maScrollBox.SetPosSizePixel( nDataWidth, nDataHeight, nScrollBarSize, nScrollBarSize );
    maScrollBox.Show( bScrBox );

    CommitRequest( CSVREQ_POSOFFSET, GetFirstVisPos() );
    CommitRequest( CSVREQ_LINEOFFSET, GetFirstVisLine() );
}

void ScCsvTableBox::InitHScrollBar()
{
    maHScroll.SetRange( Range( 0, GetPosCount() + 2 ) );
    maHScroll.SetVisibleSize( GetVisPosCount() );
    maHScroll.SetPageSize( GetVisPosCount() * 3 / 4 );
    maHScroll.SetThumbPos( GetFirstVisPos() );
}

void ScCsvTableBox::InitVScrollBar()
{
    maVScroll.SetRange( Range( 0, GetLineCount() + 1 ) );
    maVScroll.SetVisibleSize( GetVisLineCount() );
    maVScroll.SetPageSize( GetVisLineCount() - 2 );
    maVScroll.SetThumbPos( GetFirstVisLine() );
}

void ScCsvTableBox::InitTypes( const ListBox& rListBox )
{
    sal_uInt16 nTypeCount = rListBox.GetEntryCount();
    ScCsvStringVec aTypeNames( nTypeCount );
    for( sal_uInt16 nIndex = 0; nIndex < nTypeCount; ++nIndex )
        aTypeNames[ nIndex ] = rListBox.GetEntry( nIndex );
    maGrid.SetTypeNames( aTypeNames );
}


// control handling -----------------------------------------------------------

void ScCsvTableBox::SetSeparatorsMode()
{
    if( mbFixedMode )
    {
        // rescue data for fixed width mode
        mnFixedWidth = GetPosCount();
        maFixColTypes = maGrid.GetColumnTypes();
        // switch to separators mode
        DisableRepaint();
        mbFixedMode = false;
        // reset and reinitialize controls
        CommitRequest( CSVREQ_LINEOFFSET, 0 );
        CommitRequest( CSVREQ_POSCOUNT, 1 );
        CommitRequest( CSVREQ_NEWCELLTEXTS );
        maGrid.SetColumnTypes( maSepColTypes );
        InitControls();
        EnableRepaint();
    }
}

void ScCsvTableBox::SetFixedWidthMode()
{
    if( !mbFixedMode )
    {
        // rescue data for separators mode
        maSepColTypes = maGrid.GetColumnTypes();
        // switch to fixed width mode
        DisableRepaint();
        mbFixedMode = true;
        // reset and reinitialize controls
        CommitRequest( CSVREQ_LINEOFFSET, 0 );
        CommitRequest( CSVREQ_POSCOUNT, mnFixedWidth );
        maGrid.SetSplits( maRuler.GetSplits() );
        maGrid.SetColumnTypes( maFixColTypes );
        InitControls();
        EnableRepaint();
    }
}

void ScCsvTableBox::SetUniStrings(
        const String* pTextLines, const String& rSepChars,
        sal_Unicode cTextSep, bool bMergeSep )
{
    // assuming that pTextLines is a string array with size CSV_PREVIEW_LINES
    // -> will be dynamic sometime
    DisableRepaint();
    sal_Int32 nEndLine = GetFirstVisLine() + CSV_PREVIEW_LINES;
    const String* pString = pTextLines;
    for( sal_Int32 nLine = GetFirstVisLine(); nLine < nEndLine; ++nLine, ++pString )
    {
        if( mbFixedMode )
            maGrid.ImplSetTextLineFix( nLine, *pString );
        else
            maGrid.ImplSetTextLineSep( nLine, *pString, rSepChars, cTextSep, bMergeSep );
    }
    EnableRepaint();
}

void ScCsvTableBox::SetByteStrings(
        const ByteString* pTextLines, CharSet eCharSet,
        const String& rSepChars, sal_Unicode cTextSep, bool bMergeSep )
{
    // assuming that pTextLines is a string array with size CSV_PREVIEW_LINES
    // -> will be dynamic sometime
    DisableRepaint();
    sal_Int32 nEndLine = GetFirstVisLine() + CSV_PREVIEW_LINES;
    const ByteString* pString = pTextLines;
    for( sal_Int32 nLine = GetFirstVisLine(); nLine < nEndLine; ++nLine, ++pString )
    {
        if( mbFixedMode )
            maGrid.ImplSetTextLineFix( nLine, String( *pString, eCharSet ) );
        else
            maGrid.ImplSetTextLineSep( nLine, String( *pString, eCharSet ), rSepChars, cTextSep, bMergeSep );
    }
    EnableRepaint();
}

void ScCsvTableBox::FillColumnData( ScAsciiOptions& rOptions ) const
{
    if( mbFixedMode )
        maGrid.FillColumnDataFix( rOptions );
    else
        maGrid.FillColumnDataSep( rOptions );
}

void ScCsvTableBox::MakePosVisible( sal_Int32 nPos )
{
    if( IsValidSplitPos( nPos ) )
    {
        if( nPos - CSV_SCROLL_DIST + 1 <= GetFirstVisPos() )
            CommitRequest( CSVREQ_POSOFFSET, nPos - CSV_SCROLL_DIST );
        else if( nPos + CSV_SCROLL_DIST >= GetLastVisPos() )
            CommitRequest( CSVREQ_POSOFFSET, nPos - GetVisPosCount() + CSV_SCROLL_DIST + 1 );
    }
}


// event handling -------------------------------------------------------------

void ScCsvTableBox::Resize()
{
    ScCsvControl::Resize();
    InitControls();
}

void ScCsvTableBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        InitControls();
    ScCsvControl::DataChanged( rDCEvt );
}

IMPL_LINK( ScCsvTableBox, CsvRequestHdl, ScCsvControl*, pCtrl )
{
    DBG_ASSERT( pCtrl, "ScCsvTableBox::CsvRequestHdl - missing sender" );

    const ScCsvRequest& rReq = pCtrl->GetRequest();
    const ScCsvLayoutData aOldData( maData );
    sal_Int32 nData = rReq.GetData();

    bool bFound = true;
    switch( rReq.GetType() )
    {
        case CSVREQ_REPAINT:
            if( !IsNoRepaint() )
            {
                maGrid.ImplRedraw();
                maRuler.ImplRedraw();
            }
        break;
        case CSVREQ_NEWCELLTEXTS:
            DisableRepaint();
            if( !mbFixedMode )
                CommitRequest( CSVREQ_POSCOUNT, 1 );
            maUpdateTextHdl.Call( this );
            EnableRepaint();
        break;
        case CSVREQ_UPDATECELLTEXTS:
            maUpdateTextHdl.Call( this );
        break;
        case CSVREQ_COLUMNTYPE:
            maGrid.SetSelColumnType( nData );
        break;
        case CSVREQ_MAKEPOSVISIBLE:
            MakePosVisible( nData );
        break;
        default:
            bFound = false;
    }
    if( bFound )
        return 0;

    switch( rReq.GetType() )
    {
        case CSVREQ_POSCOUNT:
            maData.mnPosCount = Max( nData, 1L );
            ImplSetPosOffset( GetFirstVisPos() );
        break;
        case CSVREQ_POSOFFSET:
            ImplSetPosOffset( nData );
        break;
        case CSVREQ_OFFSETX:
            maData.mnOffsetX = Max( nData, 0L );
            ImplSetPosOffset( GetFirstVisPos() );
        break;
        case CSVREQ_CHARWIDTH:
            maData.mnCharWidth = Max( nData, 1L );
            ImplSetPosOffset( GetFirstVisPos() );
        break;
        case CSVREQ_LINECOUNT:
            maData.mnLineCount = Max( nData, 1L );
            ImplSetLineOffset( GetFirstVisLine() );
        break;
        case CSVREQ_LINEOFFSET:
            ImplSetLineOffset( nData );
        break;
        case CSVREQ_OFFSETY:
            maData.mnOffsetY = Max( nData, 0L );
            ImplSetLineOffset( GetFirstVisLine() );
        break;
        case CSVREQ_LINEHEIGHT:
            maData.mnLineHeight = Max( nData, 1L );
            ImplSetLineOffset( GetFirstVisLine() );
        break;
        case CSVREQ_MOVERULERCURSOR:
            maData.mnPosCursor = IsVisibleSplitPos( nData ) ? nData : POS_INVALID;
        break;
        case CSVREQ_MOVEGRIDCURSOR:
            maData.mnColCursor = ((0 <= nData) && (nData < GetPosCount())) ? nData : POS_INVALID;
        break;
    }
    if( maData != aOldData )
    {
        DisableRepaint();
        maRuler.ApplyLayout( aOldData );
        maGrid.ApplyLayout( aOldData );
        InitHScrollBar();
        InitVScrollBar();
        EnableRepaint();
    }

    return 0;
}

IMPL_LINK( ScCsvTableBox, CsvEventHdl, ScCsvControl*, pCtrl )
{
    DBG_ASSERT( pCtrl, "ScCsvTableBox::CsvEventHdl - missing sender" );

    const ScCsvEvent& rEvent = pCtrl->GetEvent();
    sal_Int32 nPos = rEvent.GetPos();
    sal_Int32 nOldPos = rEvent.GetOldPos();

    switch( rEvent.GetType() )
    {
        case RULEREVENT_INSERT:
            maGrid.InsertSplit( nPos );
        break;
        case RULEREVENT_REMOVE:
            maGrid.RemoveSplit( nPos );
        break;
        case RULEREVENT_MOVE:
            maGrid.MoveSplit( nOldPos, nPos );
        break;
        case RULEREVENT_REMOVEALL:
            maGrid.RemoveAllSplits();
        break;
        case GRIDEVENT_SELECTION:
            mnSelColType = maGrid.GetSelColumnType();
            maColSelectHdl.Call( this );
        break;
        case GRIDEVENT_COLUMNTYPE:
            mnSelColType = nPos;
            maColSelectHdl.Call( this );
        break;
    }

    return 0;
}

IMPL_LINK( ScCsvTableBox, ScrollHdl, ScrollBar*, pScrollBar )
{
    DBG_ASSERT( pScrollBar, "ScCsvTableBox::ScrollHdl - missing sender" );

    if( pScrollBar == &maHScroll )
        CommitRequest( CSVREQ_POSOFFSET, pScrollBar->GetThumbPos() );
    else if( pScrollBar == &maVScroll )
        CommitRequest( CSVREQ_LINEOFFSET, pScrollBar->GetThumbPos() );

    return 0;
}

IMPL_LINK( ScCsvTableBox, ScrollEndHdl, ScrollBar*, pScrollBar )
{
    DBG_ASSERT( pScrollBar, "ScCsvTableBox::ScrollEndHdl - missing sender" );

    if( pScrollBar == &maHScroll )
    {
        if( GetRulerCursorPos() != POS_INVALID )
            CommitRequest( CSVREQ_MOVERULERCURSOR, maRuler.GetNoScrollPos( GetRulerCursorPos() ) );
        if( GetGridCursorPos() != POS_INVALID )
            CommitRequest( CSVREQ_MOVEGRIDCURSOR, maGrid.GetNoScrollCol( GetGridCursorPos() ) );
    }

    return 0;
}


// ============================================================================

