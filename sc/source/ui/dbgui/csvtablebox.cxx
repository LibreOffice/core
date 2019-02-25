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

#include <csvtablebox.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/event.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/settings.hxx>
#include <AccessibleCsvControl.hxx>

ScCsvTableBox::ScCsvTableBox( vcl::Window* pParent, WinBits nBits ) :
    ScCsvControl( pParent, maData, nBits ),
    maRuler( VclPtr<ScCsvRuler>::Create(*this) ),
    maGrid( VclPtr<ScCsvGrid>::Create(*this) ),
    maHScroll( VclPtr<ScrollBar>::Create( this, WB_HORZ | WB_DRAG ) ),
    maVScroll( VclPtr<ScrollBar>::Create( this, WB_VERT | WB_DRAG ) ),
    maScrollBox( VclPtr<ScrollBarBox>::Create(this) )
{
    mbFixedMode = false;
    mnFixedWidth = 1;

    maHScroll->EnableRTL( false ); // RTL
    maHScroll->SetLineSize( 1 );
    maVScroll->SetLineSize( 1 );

    Link<ScCsvControl&,void> aLink = LINK( this, ScCsvTableBox, CsvCmdHdl );
    SetCmdHdl( aLink );
    maRuler->SetCmdHdl( aLink );
    maGrid->SetCmdHdl( aLink );

    Link<ScrollBar*,void> aLink2 = LINK( this, ScCsvTableBox, ScrollHdl );
    maHScroll->SetScrollHdl( aLink2 );
    maVScroll->SetScrollHdl( aLink2 );

    aLink2 = LINK( this, ScCsvTableBox, ScrollEndHdl );
    maHScroll->SetEndScrollHdl( aLink2 );
    maVScroll->SetEndScrollHdl( aLink2 );

    InitControls();
}

ScCsvTableBox::~ScCsvTableBox()
{
    disposeOnce();
}

void ScCsvTableBox::dispose()
{
    maRuler.disposeAndClear();
    maGrid.disposeAndClear();
    maHScroll.disposeAndClear();
    maVScroll.disposeAndClear();
    maScrollBox.disposeAndClear();
    ScCsvControl::dispose();
}

VCL_BUILDER_FACTORY_ARGS(ScCsvTableBox, WB_BORDER)

Size ScCsvTableBox::GetOptimalSize() const
{
    Size aDefault(LogicToPixel(Size(243, 82), MapMode(MapUnit::MapAppFont)));
    return aDefault;
}

// common table box handling --------------------------------------------------

void ScCsvTableBox::SetSeparatorsMode()
{
    if( mbFixedMode )
    {
        // rescue data for fixed width mode
        mnFixedWidth = GetPosCount();
        maFixColStates = maGrid->GetColumnStates();
        // switch to separators mode
        mbFixedMode = false;
        // reset and reinitialize controls
        DisableRepaint();
        Execute( CSVCMD_SETLINEOFFSET, 0 );
        Execute( CSVCMD_SETPOSCOUNT, 1 );
        Execute( CSVCMD_NEWCELLTEXTS );
        maGrid->SetColumnStates( maSepColStates );
        InitControls();
        EnableRepaint();
    }
}

void ScCsvTableBox::SetFixedWidthMode()
{
    if( !mbFixedMode )
    {
        // rescue data for separators mode
        maSepColStates = maGrid->GetColumnStates();
        // switch to fixed width mode
        mbFixedMode = true;
        // reset and reinitialize controls
        DisableRepaint();
        Execute( CSVCMD_SETLINEOFFSET, 0 );
        Execute( CSVCMD_SETPOSCOUNT, mnFixedWidth );
        maGrid->SetSplits( maRuler->GetSplits() );
        maGrid->SetColumnStates( maFixColStates );
        InitControls();
        EnableRepaint();
    }
}

void ScCsvTableBox::Init()
{
    maGrid->Init();
}

void ScCsvTableBox::InitControls()
{
    maGrid->UpdateLayoutData();

    long nScrollBarSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    Size aWinSize = CalcOutputSize( GetSizePixel() );
    long nDataWidth = aWinSize.Width() - nScrollBarSize;
    long nDataHeight = aWinSize.Height() - nScrollBarSize;

    maData.mnWinWidth = nDataWidth;
    maData.mnWinHeight = nDataHeight;

    if( mbFixedMode )
    {
        // ruler sets height internally
        maRuler->setPosSizePixel( 0, 0, nDataWidth, 0 );
        sal_Int32 nY = maRuler->GetSizePixel().Height();
        maData.mnWinHeight -= nY;
        maGrid->setPosSizePixel( 0, nY, nDataWidth, maData.mnWinHeight );
    }
    else
        maGrid->setPosSizePixel( 0, 0, nDataWidth, nDataHeight );
    maGrid->Show();
    maRuler->Show( mbFixedMode );

    // scrollbars always visible
    maHScroll->setPosSizePixel( 0, nDataHeight, nDataWidth, nScrollBarSize );
    InitHScrollBar();
    maHScroll->Show();

    // scrollbars always visible
    maVScroll->setPosSizePixel( nDataWidth, 0, nScrollBarSize, nDataHeight );
    InitVScrollBar();
    maVScroll->Show();

    bool bScrBox = maHScroll->IsVisible() && maVScroll->IsVisible();
    if( bScrBox )
        maScrollBox->setPosSizePixel( nDataWidth, nDataHeight, nScrollBarSize, nScrollBarSize );
    maScrollBox->Show( bScrBox );

    // let the controls self-adjust to visible area
    Execute( CSVCMD_SETPOSOFFSET, GetFirstVisPos() );
    Execute( CSVCMD_SETLINEOFFSET, GetFirstVisLine() );
}

void ScCsvTableBox::InitHScrollBar()
{
    maHScroll->SetRange( Range( 0, GetPosCount() + 2 ) );
    maHScroll->SetVisibleSize( GetVisPosCount() );
    maHScroll->SetPageSize( GetVisPosCount() * 3 / 4 );
    maHScroll->SetThumbPos( GetFirstVisPos() );
}

void ScCsvTableBox::InitVScrollBar()
{
    maVScroll->SetRange( Range( 0, GetLineCount() + 1 ) );
    maVScroll->SetVisibleSize( GetVisLineCount() );
    maVScroll->SetPageSize( GetVisLineCount() - 2 );
    maVScroll->SetThumbPos( GetFirstVisLine() );
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
        const OUString* pTextLines, const OUString& rSepChars,
        sal_Unicode cTextSep, bool bMergeSep, bool bRemoveSpace )
{
    // assuming that pTextLines is a string array with size CSV_PREVIEW_LINES
    // -> will be dynamic sometime
    DisableRepaint();
    sal_Int32 nEndLine = GetFirstVisLine() + CSV_PREVIEW_LINES;
    const OUString* pString = pTextLines;
    for( sal_Int32 nLine = GetFirstVisLine(); nLine < nEndLine; ++nLine, ++pString )
    {
        if( mbFixedMode )
            maGrid->ImplSetTextLineFix( nLine, *pString );
        else
            maGrid->ImplSetTextLineSep( nLine, *pString, rSepChars, cTextSep, bMergeSep, bRemoveSpace );
    }
    EnableRepaint();
}

// column settings ------------------------------------------------------------

void ScCsvTableBox::InitTypes( const ListBox& rListBox )
{
    const sal_Int32 nTypeCount = rListBox.GetEntryCount();
    std::vector<OUString> aTypeNames( nTypeCount );
    for( sal_Int32 nIndex = 0; nIndex < nTypeCount; ++nIndex )
        aTypeNames[ nIndex ] = rListBox.GetEntry( nIndex );
    maGrid->SetTypeNames( aTypeNames );
}

void ScCsvTableBox::FillColumnData( ScAsciiOptions& rOptions ) const
{
    if( mbFixedMode )
        maGrid->FillColumnDataFix( rOptions );
    else
        maGrid->FillColumnDataSep( rOptions );
}

// event handling -------------------------------------------------------------

void ScCsvTableBox::Resize()
{
    ScCsvControl::Resize();
    InitControls();
}

void ScCsvTableBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
        InitControls();
    ScCsvControl::DataChanged( rDCEvt );
}

IMPL_LINK( ScCsvTableBox, CsvCmdHdl, ScCsvControl&, rCtrl, void )
{
    const ScCsvCmd& rCmd = rCtrl.GetCmd();
    ScCsvCmdType eType = rCmd.GetType();
    sal_Int32 nParam1 = rCmd.GetParam1();
    sal_Int32 nParam2 = rCmd.GetParam2();

    bool bFound = true;
    switch( eType )
    {
        case CSVCMD_REPAINT:
            if( !IsNoRepaint() )
            {
                maGrid->ImplRedraw();
                maRuler->ImplRedraw();
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
                ScCsvColStateVec aStates( maGrid->GetColumnStates() );
                sal_Int32 nPos = GetFirstVisPos();
                Execute( CSVCMD_SETPOSCOUNT, 1 );
                Execute( CSVCMD_UPDATECELLTEXTS );
                Execute( CSVCMD_SETPOSOFFSET, nPos );
                maGrid->SetColumnStates( aStates );
                EnableRepaint();
            }
        break;
        case CSVCMD_UPDATECELLTEXTS:
            maUpdateTextHdl.Call( *this );
        break;
        case CSVCMD_SETCOLUMNTYPE:
            maGrid->SetSelColumnType( nParam1 );
        break;
        case CSVCMD_EXPORTCOLUMNTYPE:
            maColTypeHdl.Call( *this );
        break;
        case CSVCMD_SETFIRSTIMPORTLINE:
            maGrid->SetFirstImportedLine( nParam1 );
        break;

        case CSVCMD_INSERTSPLIT:
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::InsertSplit - invalid call" );
            if( maRuler->GetSplitCount() + 1 < sal::static_int_cast<sal_uInt32>(CSV_MAXCOLCOUNT) )
            {
                maRuler->InsertSplit( nParam1 );
                maGrid->InsertSplit( nParam1 );
            }
        break;
        case CSVCMD_REMOVESPLIT:
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::RemoveSplit - invalid call" );
            maRuler->RemoveSplit( nParam1 );
            maGrid->RemoveSplit( nParam1 );
        break;
        case CSVCMD_TOGGLESPLIT:
            Execute( maRuler->HasSplit( nParam1 ) ? CSVCMD_REMOVESPLIT : CSVCMD_INSERTSPLIT, nParam1 );
        break;
        case CSVCMD_MOVESPLIT:
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::MoveSplit - invalid call" );
            maRuler->MoveSplit( nParam1, nParam2 );
            maGrid->MoveSplit( nParam1, nParam2 );
        break;
        case CSVCMD_REMOVEALLSPLITS:
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::RemoveAllSplits - invalid call" );
            maRuler->RemoveAllSplits();
            maGrid->RemoveAllSplits();
        break;
        default:
            bFound = false;
    }
    if( bFound )
        return;

    const ScCsvLayoutData aOldData( maData );
    switch( eType )
    {
        case CSVCMD_SETPOSCOUNT:
            maData.mnPosCount = std::max( nParam1, sal_Int32( 1 ) );
            ImplSetPosOffset( GetFirstVisPos() );
        break;
        case CSVCMD_SETPOSOFFSET:
            ImplSetPosOffset( nParam1 );
        break;
        case CSVCMD_SETHDRWIDTH:
            maData.mnHdrWidth = std::max( nParam1, sal_Int32( 0 ) );
            ImplSetPosOffset( GetFirstVisPos() );
        break;
        case CSVCMD_SETCHARWIDTH:
            maData.mnCharWidth = std::max( nParam1, sal_Int32( 1 ) );
            ImplSetPosOffset( GetFirstVisPos() );
        break;
        case CSVCMD_SETLINECOUNT:
            maData.mnLineCount = std::max( nParam1, sal_Int32( 1 ) );
            ImplSetLineOffset( GetFirstVisLine() );
        break;
        case CSVCMD_SETLINEOFFSET:
            ImplSetLineOffset( nParam1 );
        break;
        case CSVCMD_SETHDRHEIGHT:
            maData.mnHdrHeight = std::max( nParam1, sal_Int32( 0 ) );
            ImplSetLineOffset( GetFirstVisLine() );
        break;
        case CSVCMD_SETLINEHEIGHT:
            maData.mnLineHeight = std::max( nParam1, sal_Int32( 1 ) );
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
        maRuler->ApplyLayout( aOldData );
        maGrid->ApplyLayout( aOldData );
        EnableRepaint();
    }
}

IMPL_LINK( ScCsvTableBox, ScrollHdl, ScrollBar*, pScrollBar, void )
{
    OSL_ENSURE( pScrollBar, "ScCsvTableBox::ScrollHdl - missing sender" );

    if( pScrollBar == maHScroll.get() )
        Execute( CSVCMD_SETPOSOFFSET, pScrollBar->GetThumbPos() );
    else if( pScrollBar == maVScroll.get() )
        Execute( CSVCMD_SETLINEOFFSET, pScrollBar->GetThumbPos() );
}

IMPL_LINK( ScCsvTableBox, ScrollEndHdl, ScrollBar*, pScrollBar, void )
{
    OSL_ENSURE( pScrollBar, "ScCsvTableBox::ScrollEndHdl - missing sender" );

    if( pScrollBar == maHScroll.get() )
    {
        if( GetRulerCursorPos() != CSV_POS_INVALID )
            Execute( CSVCMD_MOVERULERCURSOR, maRuler->GetNoScrollPos( GetRulerCursorPos() ) );
        if( GetGridCursorPos() != CSV_POS_INVALID )
            Execute( CSVCMD_MOVEGRIDCURSOR, maGrid->GetNoScrollCol( GetGridCursorPos() ) );
    }
}

// accessibility --------------------------------------------------------------

css::uno::Reference< css::accessibility::XAccessible > ScCsvTableBox::CreateAccessible()
{
    // do not use the ScCsvControl mechanism, return default accessible object
    return Control::CreateAccessible();
}

rtl::Reference<ScAccessibleCsvControl> ScCsvTableBox::ImplCreateAccessible()
{
    return rtl::Reference<ScAccessibleCsvControl>();    // not used, see CreateAccessible()
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
