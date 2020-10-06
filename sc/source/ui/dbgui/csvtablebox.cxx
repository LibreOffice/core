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
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <AccessibleCsvControl.hxx>

ScCsvTableBox::ScCsvTableBox(weld::Builder& rBuilder)
    : mxRuler(new ScCsvRuler(maData, this))
    , mxGrid(new ScCsvGrid(maData, rBuilder.weld_menu("popup"), this))
    , mxScroll(rBuilder.weld_scrolled_window("scrolledwindow", true))
    , mxRulerWeld(new weld::CustomWeld(rBuilder, "csvruler", *mxRuler))
    , mxGridWeld(new weld::CustomWeld(rBuilder, "csvgrid", *mxGrid))
{
    Size aSize(mxScroll->get_approximate_digit_width() * 67,
               mxScroll->get_text_height() * 10);
    // this needs to be larger than the ScCsvGrid initial size to get it
    // to stretch to fit, see ScCsvGrid::SetDrawingArea
    mxScroll->set_size_request(aSize.Width(), aSize.Height());

    mbFixedMode = false;
    mnFixedWidth = 1;

    Link<ScCsvControl&,void> aLink = LINK( this, ScCsvTableBox, CsvCmdHdl );
    mxRuler->SetCmdHdl( aLink );
    mxGrid->SetCmdHdl( aLink );

    mxScroll->connect_hadjustment_changed(LINK(this, ScCsvTableBox, HScrollHdl));
    mxScroll->connect_vadjustment_changed(LINK(this, ScCsvTableBox, VScrollHdl));

    maEndScrollIdle.SetPriority(TaskPriority::LOWEST);
    maEndScrollIdle.SetInvokeHandler(LINK(this,ScCsvTableBox,ScrollEndHdl));

    InitControls();
}

ScCsvTableBox::~ScCsvTableBox()
{
}

// common table box handling --------------------------------------------------

void ScCsvTableBox::SetSeparatorsMode()
{
    if( !mbFixedMode )
        return;

    // rescue data for fixed width mode
    mnFixedWidth = mxGrid->GetPosCount();
    maFixColStates = mxGrid->GetColumnStates();
    // switch to separators mode
    mbFixedMode = false;
    // reset and reinitialize controls
    mxGrid->DisableRepaint();
    mxGrid->Execute( CSVCMD_SETLINEOFFSET, 0 );
    mxGrid->Execute( CSVCMD_SETPOSCOUNT, 1 );
    mxGrid->Execute( CSVCMD_NEWCELLTEXTS );
    mxGrid->SetColumnStates( maSepColStates );
    InitControls();
    mxGrid->EnableRepaint();
}

void ScCsvTableBox::SetFixedWidthMode()
{
    if( mbFixedMode )
        return;

    // rescue data for separators mode
    maSepColStates = mxGrid->GetColumnStates();
    // switch to fixed width mode
    mbFixedMode = true;
    // reset and reinitialize controls
    mxGrid->DisableRepaint();
    mxGrid->Execute( CSVCMD_SETLINEOFFSET, 0 );
    mxGrid->Execute( CSVCMD_SETPOSCOUNT, mnFixedWidth );
    mxGrid->SetSplits( mxRuler->GetSplits() );
    mxGrid->SetColumnStates( maFixColStates );
    InitControls();
    mxGrid->EnableRepaint();
}

void ScCsvTableBox::Init()
{
    mxGrid->Init();
}

void ScCsvTableBox::InitControls()
{
    mxGrid->UpdateLayoutData();

    mxGrid->Show();
    if (mbFixedMode)
        mxRuler->Show();
    else
        mxRuler->Hide();

    Size aWinSize = mxGrid->GetOutputSizePixel();
    maData.mnWinWidth = aWinSize.Width();
    maData.mnWinHeight = aWinSize.Height();

    // scrollbars always visible
    InitHScrollBar();

    // scrollbars always visible
    InitVScrollBar();

    // let the controls self-adjust to visible area
    mxGrid->Execute( CSVCMD_SETPOSOFFSET, mxGrid->GetFirstVisPos() );
    mxGrid->Execute( CSVCMD_SETLINEOFFSET, mxGrid->GetFirstVisLine() );
}

void ScCsvTableBox::InitHScrollBar()
{
    int nLower = 0;
    int nValue = mxGrid->GetFirstVisPos();
    int nUpper = mxGrid->GetPosCount() + 2;
    int nPageSize = mxGrid->GetVisPosCount();

    // Undo scrollbar RTL
    if (AllSettings::GetLayoutRTL())
        nValue = nUpper - (nValue - nLower + nPageSize);

    mxScroll->hadjustment_configure(nValue, nLower, nUpper,
                                    1, mxGrid->GetVisPosCount() * 3 / 4,
                                    nPageSize);
}

void ScCsvTableBox::InitVScrollBar()
{
    mxScroll->vadjustment_configure(mxGrid->GetFirstVisLine(), 0, mxGrid->GetLineCount() + 1,
                                    1, mxGrid->GetVisLineCount() - 2,
                                    mxGrid->GetVisLineCount());
}

void ScCsvTableBox::MakePosVisible( sal_Int32 nPos )
{
    if( (0 <= nPos) && (nPos < mxGrid->GetPosCount()) )
    {
        if( nPos - CSV_SCROLL_DIST + 1 <= mxGrid->GetFirstVisPos() )
            mxGrid->Execute( CSVCMD_SETPOSOFFSET, nPos - CSV_SCROLL_DIST );
        else if( nPos + CSV_SCROLL_DIST >= mxGrid->GetLastVisPos() )
            mxGrid->Execute( CSVCMD_SETPOSOFFSET, nPos - mxGrid->GetVisPosCount() + CSV_SCROLL_DIST );
    }
}

// cell contents --------------------------------------------------------------

void ScCsvTableBox::SetUniStrings(
        const OUString* pTextLines, const OUString& rSepChars,
        sal_Unicode cTextSep, bool bMergeSep, bool bRemoveSpace )
{
    // assuming that pTextLines is a string array with size CSV_PREVIEW_LINES
    // -> will be dynamic sometime
    mxGrid->DisableRepaint();
    sal_Int32 nEndLine = mxGrid->GetFirstVisLine() + CSV_PREVIEW_LINES;
    const OUString* pString = pTextLines;
    for( sal_Int32 nLine = mxGrid->GetFirstVisLine(); nLine < nEndLine; ++nLine, ++pString )
    {
        if( mbFixedMode )
            mxGrid->ImplSetTextLineFix( nLine, *pString );
        else
            mxGrid->ImplSetTextLineSep( nLine, *pString, rSepChars, cTextSep, bMergeSep, bRemoveSpace );
    }
    mxGrid->EnableRepaint();
}

// column settings ------------------------------------------------------------

void ScCsvTableBox::InitTypes(const weld::ComboBox& rListBox)
{
    const sal_Int32 nTypeCount = rListBox.get_count();
    std::vector<OUString> aTypeNames( nTypeCount );
    for( sal_Int32 nIndex = 0; nIndex < nTypeCount; ++nIndex )
        aTypeNames[ nIndex ] = rListBox.get_text( nIndex );
    mxGrid->SetTypeNames( aTypeNames );
}

void ScCsvTableBox::FillColumnData( ScAsciiOptions& rOptions ) const
{
    if( mbFixedMode )
        mxGrid->FillColumnDataFix( rOptions );
    else
        mxGrid->FillColumnDataSep( rOptions );
}

// event handling -------------------------------------------------------------

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
            if( !mxGrid->IsNoRepaint() )
            {
                mxGrid->Invalidate();
                mxRuler->Invalidate();
                InitHScrollBar();
                InitVScrollBar();
            }
        break;
        case CSVCMD_MAKEPOSVISIBLE:
            MakePosVisible( nParam1 );
        break;

        case CSVCMD_NEWCELLTEXTS:
            if( mbFixedMode )
                mxGrid->Execute( CSVCMD_UPDATECELLTEXTS );
            else
            {
                mxGrid->DisableRepaint();
                ScCsvColStateVec aStates( mxGrid->GetColumnStates() );
                sal_Int32 nPos = mxGrid->GetFirstVisPos();
                mxGrid->Execute( CSVCMD_SETPOSCOUNT, 1 );
                mxGrid->Execute( CSVCMD_UPDATECELLTEXTS );
                mxGrid->Execute( CSVCMD_SETPOSOFFSET, nPos );
                mxGrid->SetColumnStates( aStates );
                mxGrid->EnableRepaint();
            }
        break;
        case CSVCMD_UPDATECELLTEXTS:
            maUpdateTextHdl.Call( *this );
        break;
        case CSVCMD_SETCOLUMNTYPE:
            mxGrid->SetSelColumnType( nParam1 );
        break;
        case CSVCMD_EXPORTCOLUMNTYPE:
            maColTypeHdl.Call( *this );
        break;
        case CSVCMD_SETFIRSTIMPORTLINE:
            mxGrid->SetFirstImportedLine( nParam1 );
        break;

        case CSVCMD_INSERTSPLIT:
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::InsertSplit - invalid call" );
            if( mxRuler->GetSplitCount() + 1 < sal::static_int_cast<sal_uInt32>(CSV_MAXCOLCOUNT) )
            {
                mxRuler->InsertSplit( nParam1 );
                mxGrid->InsertSplit( nParam1 );
            }
        break;
        case CSVCMD_REMOVESPLIT:
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::RemoveSplit - invalid call" );
            mxRuler->RemoveSplit( nParam1 );
            mxGrid->RemoveSplit( nParam1 );
        break;
        case CSVCMD_TOGGLESPLIT:
            mxGrid->Execute( mxRuler->HasSplit( nParam1 ) ? CSVCMD_REMOVESPLIT : CSVCMD_INSERTSPLIT, nParam1 );
        break;
        case CSVCMD_MOVESPLIT:
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::MoveSplit - invalid call" );
            mxRuler->MoveSplit( nParam1, nParam2 );
            mxGrid->MoveSplit( nParam1, nParam2 );
        break;
        case CSVCMD_REMOVEALLSPLITS:
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::RemoveAllSplits - invalid call" );
            mxRuler->RemoveAllSplits();
            mxGrid->RemoveAllSplits();
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
            ImplSetPosOffset( mxGrid->GetFirstVisPos() );
        break;
        case CSVCMD_SETPOSOFFSET:
            ImplSetPosOffset( nParam1 );
        break;
        case CSVCMD_SETHDRWIDTH:
            maData.mnHdrWidth = std::max( nParam1, sal_Int32( 0 ) );
            ImplSetPosOffset( mxGrid->GetFirstVisPos() );
        break;
        case CSVCMD_SETCHARWIDTH:
            maData.mnCharWidth = std::max( nParam1, sal_Int32( 1 ) );
            ImplSetPosOffset( mxGrid->GetFirstVisPos() );
        break;
        case CSVCMD_SETLINECOUNT:
            maData.mnLineCount = std::max( nParam1, sal_Int32( 1 ) );
            ImplSetLineOffset( mxGrid->GetFirstVisLine() );
        break;
        case CSVCMD_SETLINEOFFSET:
            ImplSetLineOffset( nParam1 );
        break;
        case CSVCMD_SETHDRHEIGHT:
            maData.mnHdrHeight = std::max( nParam1, sal_Int32( 0 ) );
            ImplSetLineOffset( mxGrid->GetFirstVisLine() );
        break;
        case CSVCMD_SETLINEHEIGHT:
            maData.mnLineHeight = std::max( nParam1, sal_Int32( 1 ) );
            ImplSetLineOffset( mxGrid->GetFirstVisLine() );
        break;
        case CSVCMD_MOVERULERCURSOR:
            maData.mnPosCursor = mxGrid->IsVisibleSplitPos( nParam1 ) ? nParam1 : CSV_POS_INVALID;
        break;
        case CSVCMD_MOVEGRIDCURSOR:
            maData.mnColCursor = ((0 <= nParam1) && (nParam1 < mxGrid->GetPosCount())) ? nParam1 : CSV_POS_INVALID;
        break;
        default:
        {
            // added to avoid warnings
        }
    }

    if( maData != aOldData )
    {
        mxGrid->DisableRepaint();
        mxRuler->ApplyLayout( aOldData );
        mxGrid->ApplyLayout( aOldData );
        mxGrid->EnableRepaint();
    }
}

IMPL_LINK(ScCsvTableBox, HScrollHdl, weld::ScrolledWindow&, rScroll, void)
{
    int nLower = 0;
    int nValue = rScroll.hadjustment_get_value();
    int nUpper = mxGrid->GetPosCount() + 2;
    int nPageSize = mxGrid->GetVisPosCount();

    // Undo scrollbar RTL
    if (AllSettings::GetLayoutRTL())
        nValue = nUpper - (nValue - nLower + nPageSize);

    mxGrid->Execute(CSVCMD_SETPOSOFFSET, nValue);
    maEndScrollIdle.Start();
}

IMPL_LINK(ScCsvTableBox, VScrollHdl, weld::ScrolledWindow&, rScroll, void)
{
    mxGrid->Execute(CSVCMD_SETLINEOFFSET, rScroll.vadjustment_get_value());
}

IMPL_LINK_NOARG(ScCsvTableBox, ScrollEndHdl, Timer*, void)
{
    if( mxGrid->GetRulerCursorPos() != CSV_POS_INVALID )
        mxGrid->Execute( CSVCMD_MOVERULERCURSOR, mxRuler->GetNoScrollPos( mxGrid->GetRulerCursorPos() ) );
    if( mxGrid->GetGridCursorPos() != CSV_POS_INVALID )
        mxGrid->Execute( CSVCMD_MOVEGRIDCURSOR, mxGrid->GetNoScrollCol( mxGrid->GetGridCursorPos() ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
