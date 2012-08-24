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

#include "csvtablebox.hxx"
#include <vcl/lstbox.hxx>

// ause
#include "editutil.hxx"

// ============================================================================

ScCsvTableBox::ScCsvTableBox( Window* pParent, const ResId& rResId ) :
    ScCsvControl( pParent, maData, rResId ),
    maRuler( *this ),
    maGrid( *this ),
    maHScroll( this, WB_HORZ | WB_DRAG ),
    maVScroll( this, WB_VERT | WB_DRAG ),
    maScrollBox( this )
{
    mbFixedMode = false;
    mnFixedWidth = 1;

    maHScroll.EnableRTL( false ); // RTL
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
    maGrid.Init();
}

void ScCsvTableBox::InitControls()
{
    maGrid.UpdateLayoutData();

    long nScrollBarSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    Size aWinSize = CalcOutputSize( GetSizePixel() );
    long nDataWidth = aWinSize.Width() - nScrollBarSize;
    long nDataHeight = aWinSize.Height() - nScrollBarSize;

    maData.mnWinWidth = nDataWidth;
    maData.mnWinHeight = nDataHeight;

    if( mbFixedMode )
    {
        // ruler sets height internally
        maRuler.setPosSizePixel( 0, 0, nDataWidth, 0 );
        sal_Int32 nY = maRuler.GetSizePixel().Height();
        maData.mnWinHeight -= nY;
        maGrid.setPosSizePixel( 0, nY, nDataWidth, maData.mnWinHeight );
    }
    else
        maGrid.setPosSizePixel( 0, 0, nDataWidth, nDataHeight );
    maGrid.Show();
    maRuler.Show( mbFixedMode );

    // scrollbars always visible
    maHScroll.setPosSizePixel( 0, nDataHeight, nDataWidth, nScrollBarSize );
    InitHScrollBar();
    maHScroll.Show();

    // scrollbars always visible
    maVScroll.setPosSizePixel( nDataWidth, 0, nScrollBarSize, nDataHeight );
    InitVScrollBar();
    maVScroll.Show();

    bool bScrBox = maHScroll.IsVisible() && maVScroll.IsVisible();
    if( bScrBox )
        maScrollBox.setPosSizePixel( nDataWidth, nDataHeight, nScrollBarSize, nScrollBarSize );
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
        const rtl::OUString* pTextLines, const String& rSepChars,
        sal_Unicode cTextSep, bool bMergeSep )
{
    // assuming that pTextLines is a string array with size CSV_PREVIEW_LINES
    // -> will be dynamic sometime
    DisableRepaint();
    sal_Int32 nEndLine = GetFirstVisLine() + CSV_PREVIEW_LINES;
    const rtl::OUString* pString = pTextLines;
    for( sal_Int32 nLine = GetFirstVisLine(); nLine < nEndLine; ++nLine, ++pString )
    {
        if( mbFixedMode )
            maGrid.ImplSetTextLineFix( nLine, *pString );
        else
            maGrid.ImplSetTextLineSep( nLine, *pString, rSepChars, cTextSep, bMergeSep );
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
    OSL_ENSURE( pCtrl, "ScCsvTableBox::CsvCmdHdl - missing sender" );

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
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::InsertSplit - invalid call" );
            if( maRuler.GetSplitCount() + 1 < sal::static_int_cast<sal_uInt32>(CSV_MAXCOLCOUNT) )
            {
                maRuler.InsertSplit( nParam1 );
                maGrid.InsertSplit( nParam1 );
            }
        break;
        case CSVCMD_REMOVESPLIT:
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::RemoveSplit - invalid call" );
            maRuler.RemoveSplit( nParam1 );
            maGrid.RemoveSplit( nParam1 );
        break;
        case CSVCMD_TOGGLESPLIT:
            Execute( maRuler.HasSplit( nParam1 ) ? CSVCMD_REMOVESPLIT : CSVCMD_INSERTSPLIT, nParam1 );
        break;
        case CSVCMD_MOVESPLIT:
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::MoveSplit - invalid call" );
            maRuler.MoveSplit( nParam1, nParam2 );
            maGrid.MoveSplit( nParam1, nParam2 );
        break;
        case CSVCMD_REMOVEALLSPLITS:
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::RemoveAllSplits - invalid call" );
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
            maData.mnPosCount = Max( nParam1, sal_Int32( 1 ) );
            ImplSetPosOffset( GetFirstVisPos() );
        break;
        case CSVCMD_SETPOSOFFSET:
            ImplSetPosOffset( nParam1 );
        break;
        case CSVCMD_SETHDRWIDTH:
            maData.mnHdrWidth = Max( nParam1, sal_Int32( 0 ) );
            ImplSetPosOffset( GetFirstVisPos() );
        break;
        case CSVCMD_SETCHARWIDTH:
            maData.mnCharWidth = Max( nParam1, sal_Int32( 1 ) );
            ImplSetPosOffset( GetFirstVisPos() );
        break;
        case CSVCMD_SETLINECOUNT:
            maData.mnLineCount = Max( nParam1, sal_Int32( 1 ) );
            ImplSetLineOffset( GetFirstVisLine() );
        break;
        case CSVCMD_SETLINEOFFSET:
            ImplSetLineOffset( nParam1 );
        break;
        case CSVCMD_SETHDRHEIGHT:
            maData.mnHdrHeight = Max( nParam1, sal_Int32( 0 ) );
            ImplSetLineOffset( GetFirstVisLine() );
        break;
        case CSVCMD_SETLINEHEIGHT:
            maData.mnLineHeight = Max( nParam1, sal_Int32( 1 ) );
            ImplSetLineOffset( GetFirstVisLine() );
        break;
        case CSVCMD_MOVERULERCURSOR:
            maData.mnPosCursor = IsVisibleSplitPos( nParam1 ) ? nParam1 : CSV_POS_INVALID;
        break;
        case CSVCMD_MOVEGRIDCURSOR:
            maData.mnColCursor = ((0 <= nParam1) && (nParam1 < GetPosCount())) ? nParam1 : CSV_POS_INVALID;
        break;
        default:
        {
            // added to avoid warnings
        }
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
    OSL_ENSURE( pScrollBar, "ScCsvTableBox::ScrollHdl - missing sender" );

    if( pScrollBar == &maHScroll )
        Execute( CSVCMD_SETPOSOFFSET, pScrollBar->GetThumbPos() );
    else if( pScrollBar == &maVScroll )
        Execute( CSVCMD_SETLINEOFFSET, pScrollBar->GetThumbPos() );

    return 0;
}

IMPL_LINK( ScCsvTableBox, ScrollEndHdl, ScrollBar*, pScrollBar )
{
    OSL_ENSURE( pScrollBar, "ScCsvTableBox::ScrollEndHdl - missing sender" );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
