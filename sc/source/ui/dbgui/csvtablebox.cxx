/*************************************************************************
 *
 *  $RCSfile: csvtablebox.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:52 $
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


// common table box handling --------------------------------------------------

void ScCsvTableBox::SetSeparatorsMode()
{
    if( mbFixedMode )
    {
        // rescue data for fixed width mode
        mnFixedWidth = GetPosCount();
        maFixColStates = maGrid.GetColumnStates();
        // switch to separators mode
        mbFixedMode = false;
        // reset and reinitialize controls
        DisableRepaint();
        Execute( CSVCMD_SETLINEOFFSET, 0 );
        Execute( CSVCMD_SETPOSCOUNT, 1 );
        Execute( CSVCMD_NEWCELLTEXTS );
        maGrid.SetColumnStates( maSepColStates );
        InitControls();
        EnableRepaint();
    }
}

void ScCsvTableBox::SetFixedWidthMode()
{
    if( !mbFixedMode )
    {
        // rescue data for separators mode
        maSepColStates = maGrid.GetColumnStates();
        // switch to fixed width mode
        mbFixedMode = true;
        // reset and reinitialize controls
        DisableRepaint();
        Execute( CSVCMD_SETLINEOFFSET, 0 );
        Execute( CSVCMD_SETPOSCOUNT, mnFixedWidth );
        maGrid.SetSplits( maRuler.GetSplits() );
        maGrid.SetColumnStates( maFixColStates );
        InitControls();
        EnableRepaint();
    }
}

void ScCsvTableBox::Init()
{
    mbFixedMode = false;
    mnFixedWidth = 1;

    maHScroll.EnableRTL( false ); // #107812# RTL
    maHScroll.SetLineSize( 1 );
    maVScroll.SetLineSize( 1 );

    Link aLink = LINK( this, ScCsvTableBox, CsvCmdHdl );
    SetCmdHdl( aLink );
    maRuler.SetCmdHdl( aLink );
    maGrid.SetCmdHdl( aLink );

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

    // scrollbars always visible
    maHScroll.SetPosSizePixel( 0, nDataHeight, nDataWidth, nScrollBarSize );
    InitHScrollBar();
    maHScroll.Show();

    // scrollbars always visible
    maVScroll.SetPosSizePixel( nDataWidth, 0, nScrollBarSize, nDataHeight );
    InitVScrollBar();
    maVScroll.Show();

    bool bScrBox = maHScroll.IsVisible() && maVScroll.IsVisible();
    if( bScrBox )
        maScrollBox.SetPosSizePixel( nDataWidth, nDataHeight, nScrollBarSize, nScrollBarSize );
    maScrollBox.Show( bScrBox );

    // let the controls self-adjust to visible area
    Execute( CSVCMD_SETPOSOFFSET, GetFirstVisPos() );
    Execute( CSVCMD_SETLINEOFFSET, GetFirstVisLine() );
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

void ScCsvTableBox::MakePosVisible( sal_Int32 nPos )
{
    if( (0 <= nPos) && (nPos < GetPosCount()) )
    {
        if( nPos - CSV_SCROLL_DIST + 1 <= GetFirstVisPos() )
            Execute( CSVCMD_SETPOSOFFSET, nPos - CSV_SCROLL_DIST );
        else if( nPos + CSV_SCROLL_DIST >= GetLastVisPos() )
            Execute( CSVCMD_SETPOSOFFSET, nPos - GetVisPosCount() + CSV_SCROLL_DIST );
    }
}


// cell contents --------------------------------------------------------------

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


// column settings ------------------------------------------------------------

void ScCsvTableBox::InitTypes( const ListBox& rListBox )
{
    sal_uInt16 nTypeCount = rListBox.GetEntryCount();
    StringVec aTypeNames( nTypeCount );
    for( sal_uInt16 nIndex = 0; nIndex < nTypeCount; ++nIndex )
        aTypeNames[ nIndex ] = rListBox.GetEntry( nIndex );
    maGrid.SetTypeNames( aTypeNames );
}

void ScCsvTableBox::FillColumnData( ScAsciiOptions& rOptions ) const
{
    if( mbFixedMode )
        maGrid.FillColumnDataFix( rOptions );
    else
        maGrid.FillColumnDataSep( rOptions );
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

IMPL_LINK( ScCsvTableBox, CsvCmdHdl, ScCsvControl*, pCtrl )
{
    DBG_ASSERT( pCtrl, "ScCsvTableBox::CsvCmdHdl - missing sender" );

    const ScCsvCmd& rCmd = pCtrl->GetCmd();
    ScCsvCmdType eType = rCmd.GetType();
    sal_Int32 nParam1 = rCmd.GetParam1();
    sal_Int32 nParam2 = rCmd.GetParam2();

    bool bFound = true;
    switch( eType )
    {
        case CSVCMD_REPAINT:
            if( !IsNoRepaint() )
            {
                maGrid.ImplRedraw();
                maRuler.ImplRedraw();
                InitHScrollBar();
                InitVScrollBar();
            }
        break;
        case CSVCMD_MAKEPOSVISIBLE:
            MakePosVisible( nParam1 );
        break;

        case CSVCMD_NEWCELLTEXTS:
            if( mbFixedMode )
                Execute( CSVCMD_UPDATECELLTEXTS );
            else
            {
                DisableRepaint();
                ScCsvColStateVec aStates( maGrid.GetColumnStates() );
                sal_Int32 nPos = GetFirstVisPos();
                Execute( CSVCMD_SETPOSCOUNT, 1 );
                Execute( CSVCMD_UPDATECELLTEXTS );
                Execute( CSVCMD_SETPOSOFFSET, nPos );
                maGrid.SetColumnStates( aStates );
                EnableRepaint();
            }
        break;
        case CSVCMD_UPDATECELLTEXTS:
            maUpdateTextHdl.Call( this );
        break;
        case CSVCMD_SETCOLUMNTYPE:
            maGrid.SetSelColumnType( nParam1 );
        break;
        case CSVCMD_EXPORTCOLUMNTYPE:
            maColTypeHdl.Call( this );
        break;
        case CSVCMD_SETFIRSTIMPORTLINE:
            maGrid.SetFirstImportedLine( nParam1 );
        break;

        case CSVCMD_INSERTSPLIT:
            DBG_ASSERT( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::InsertSplit - invalid call" );
            if( maRuler.GetSplitCount() + 1 < CSV_MAXCOLCOUNT )
            {
                maRuler.InsertSplit( nParam1 );
                maGrid.InsertSplit( nParam1 );
            }
        break;
        case CSVCMD_REMOVESPLIT:
            DBG_ASSERT( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::RemoveSplit - invalid call" );
            maRuler.RemoveSplit( nParam1 );
            maGrid.RemoveSplit( nParam1 );
        break;
        case CSVCMD_TOGGLESPLIT:
            Execute( maRuler.HasSplit( nParam1 ) ? CSVCMD_REMOVESPLIT : CSVCMD_INSERTSPLIT, nParam1 );
        break;
        case CSVCMD_MOVESPLIT:
            DBG_ASSERT( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::MoveSplit - invalid call" );
            maRuler.MoveSplit( nParam1, nParam2 );
            maGrid.MoveSplit( nParam1, nParam2 );
        break;
        case CSVCMD_REMOVEALLSPLITS:
            DBG_ASSERT( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::RemoveAllSplits - invalid call" );
            maRuler.RemoveAllSplits();
            maGrid.RemoveAllSplits();
        break;
        default:
            bFound = false;
    }
    if( bFound )
        return 0;

    const ScCsvLayoutData aOldData( maData );
    switch( eType )
    {
        case CSVCMD_SETPOSCOUNT:
            maData.mnPosCount = Max( nParam1, 1L );
            ImplSetPosOffset( GetFirstVisPos() );
        break;
        case CSVCMD_SETPOSOFFSET:
            ImplSetPosOffset( nParam1 );
        break;
        case CSVCMD_SETHDRWIDTH:
            maData.mnHdrWidth = Max( nParam1, 0L );
            ImplSetPosOffset( GetFirstVisPos() );
        break;
        case CSVCMD_SETCHARWIDTH:
            maData.mnCharWidth = Max( nParam1, 1L );
            ImplSetPosOffset( GetFirstVisPos() );
        break;
        case CSVCMD_SETLINECOUNT:
            maData.mnLineCount = Max( nParam1, 1L );
            ImplSetLineOffset( GetFirstVisLine() );
        break;
        case CSVCMD_SETLINEOFFSET:
            ImplSetLineOffset( nParam1 );
        break;
        case CSVCMD_SETHDRHEIGHT:
            maData.mnHdrHeight = Max( nParam1, 0L );
            ImplSetLineOffset( GetFirstVisLine() );
        break;
        case CSVCMD_SETLINEHEIGHT:
            maData.mnLineHeight = Max( nParam1, 1L );
            ImplSetLineOffset( GetFirstVisLine() );
        break;
        case CSVCMD_MOVERULERCURSOR:
            maData.mnPosCursor = IsVisibleSplitPos( nParam1 ) ? nParam1 : CSV_POS_INVALID;
        break;
        case CSVCMD_MOVEGRIDCURSOR:
            maData.mnColCursor = ((0 <= nParam1) && (nParam1 < GetPosCount())) ? nParam1 : CSV_POS_INVALID;
        break;
    }

    if( maData != aOldData )
    {
        DisableRepaint();
        maRuler.ApplyLayout( aOldData );
        maGrid.ApplyLayout( aOldData );
        EnableRepaint();
    }

    return 0;
}

IMPL_LINK( ScCsvTableBox, ScrollHdl, ScrollBar*, pScrollBar )
{
    DBG_ASSERT( pScrollBar, "ScCsvTableBox::ScrollHdl - missing sender" );

    if( pScrollBar == &maHScroll )
        Execute( CSVCMD_SETPOSOFFSET, pScrollBar->GetThumbPos() );
    else if( pScrollBar == &maVScroll )
        Execute( CSVCMD_SETLINEOFFSET, pScrollBar->GetThumbPos() );

    return 0;
}

IMPL_LINK( ScCsvTableBox, ScrollEndHdl, ScrollBar*, pScrollBar )
{
    DBG_ASSERT( pScrollBar, "ScCsvTableBox::ScrollEndHdl - missing sender" );

    if( pScrollBar == &maHScroll )
    {
        if( GetRulerCursorPos() != CSV_POS_INVALID )
            Execute( CSVCMD_MOVERULERCURSOR, maRuler.GetNoScrollPos( GetRulerCursorPos() ) );
        if( GetGridCursorPos() != CSV_POS_INVALID )
            Execute( CSVCMD_MOVEGRIDCURSOR, maGrid.GetNoScrollCol( GetGridCursorPos() ) );
    }

    return 0;
}


// accessibility --------------------------------------------------------------

ScCsvTableBox::XAccessibleRef ScCsvTableBox::CreateAccessible()
{
    // do not use the ScCsvControl mechanism, return default accessible object
    return Control::CreateAccessible();
}

ScAccessibleCsvControl* ScCsvTableBox::ImplCreateAccessible()
{
    return NULL;    // not used, see CreateAccessible()
}


// ============================================================================

