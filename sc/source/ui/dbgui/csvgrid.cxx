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

#include <csvgrid.hxx>

#include <algorithm>
#include <memory>

#include <svtools/colorcfg.hxx>
#include <sal/macros.h>
#include <tools/poly.hxx>
#include <scmod.hxx>
#include <asciiopt.hxx>
#include <impex.hxx>
#include <AccessibleCsvControl.hxx>

// *** edit engine ***
#include <editeng/eeitem.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>

#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/langitem.hxx>
#include <svl/itemset.hxx>
#include <editutil.hxx>
// *** edit engine ***

struct Func_SetType
{
    sal_Int32 const      mnType;
    explicit                    Func_SetType( sal_Int32 nType ) : mnType( nType ) {}
    void                 operator()( ScCsvColState& rState ) const
        { rState.mnType = mnType; }
};

struct Func_Select
{
    bool const           mbSelect;
    explicit                    Func_Select( bool bSelect ) : mbSelect( bSelect ) {}
    void                 operator()( ScCsvColState& rState ) const
        { rState.Select( mbSelect ); }
};

ScCsvGrid::ScCsvGrid( ScCsvControl& rParent ) :
    ScCsvControl( rParent ),
    mpBackgrDev( VclPtr<VirtualDevice>::Create() ),
    mpGridDev( VclPtr<VirtualDevice>::Create() ),
    mpPopup( VclPtr<PopupMenu>::Create() ),
    mpColorConfig( nullptr ),
    mpEditEngine( new ScEditEngineDefaulter( EditEngine::CreatePool(), true ) ),
    maHeaderFont( GetFont() ),
    maColStates( 1 ),
    maTypeNames( 1 ),
    mnFirstImpLine( 0 ),
    mnRecentSelCol( CSV_COLUMN_INVALID ),
    mnMTCurrCol( SAL_MAX_UINT32 ),
    mbMTSelecting( false )
{
    mpEditEngine->SetRefDevice( mpBackgrDev.get() );
    mpEditEngine->SetRefMapMode( MapMode( MapUnit::MapPixel ) );
    maEdEngSize = mpEditEngine->GetPaperSize();

    mpPopup->SetMenuFlags( mpPopup->GetMenuFlags() | MenuFlags::NoAutoMnemonics );

    EnableRTL( false ); // RTL
    InitFonts();
    ImplClearSplits();
}

ScCsvGrid::~ScCsvGrid()
{
    disposeOnce();
}

void ScCsvGrid::dispose()
{
    OSL_ENSURE(mpColorConfig, "the object hasn't been initialized properly");
    if (mpColorConfig)
        mpColorConfig->RemoveListener(this);
    mpPopup.disposeAndClear();
    mpBackgrDev.disposeAndClear();
    mpGridDev.disposeAndClear();
    ScCsvControl::dispose();
}

void
ScCsvGrid::Init()
{
    OSL_PRECOND(!mpColorConfig, "the object has already been initialized");
    mpColorConfig = &SC_MOD()->GetColorConfig();
    InitColors();
    mpColorConfig->AddListener(this);
}

// common grid handling -------------------------------------------------------

void ScCsvGrid::UpdateLayoutData()
{
    DisableRepaint();
    SetFont( maMonoFont );
    Execute( CSVCMD_SETCHARWIDTH, GetTextWidth( OUString( 'X' ) ) );
    Execute( CSVCMD_SETLINEHEIGHT, GetTextHeight() + 1 );
    SetFont( maHeaderFont );
    Execute( CSVCMD_SETHDRHEIGHT, GetTextHeight() + 1 );
    UpdateOffsetX();
    EnableRepaint();
}

void ScCsvGrid::UpdateOffsetX()
{
    sal_Int32 nLastLine = GetLastVisLine() + 1;
    sal_Int32 nDigits = 2;
    while( nLastLine /= 10 ) ++nDigits;
    nDigits = std::max( nDigits, sal_Int32( 3 ) );
    Execute( CSVCMD_SETHDRWIDTH, GetTextWidth( OUString( '0' ) ) * nDigits );
}

void ScCsvGrid::ApplyLayout( const ScCsvLayoutData& rOldData )
{
    ScCsvDiff nDiff = GetLayoutData().GetDiff( rOldData );
    if( nDiff == ScCsvDiff::Equal ) return;

    DisableRepaint();

    if( nDiff & ScCsvDiff::RulerCursor )
    {
        ImplInvertCursor( rOldData.mnPosCursor );
        ImplInvertCursor( GetRulerCursorPos() );
    }

    if( nDiff & ScCsvDiff::PosCount )
    {
        if( GetPosCount() < rOldData.mnPosCount )
        {
            SelectAll( false );
            maSplits.RemoveRange( GetPosCount(), rOldData.mnPosCount );
        }
        else
            maSplits.Remove( rOldData.mnPosCount );
        maSplits.Insert( GetPosCount() );
        maColStates.resize( maSplits.Count() - 1 );
    }

    if( nDiff & ScCsvDiff::LineOffset )
    {
        Execute( CSVCMD_UPDATECELLTEXTS );
        UpdateOffsetX();
    }

    ScCsvDiff nHVDiff = nDiff & (ScCsvDiff::HorizontalMask | ScCsvDiff::VerticalMask);
    if( nHVDiff == ScCsvDiff::PosOffset )
        ImplDrawHorzScrolled( rOldData.mnPosOffset );
    else if( nHVDiff != ScCsvDiff::Equal )
        InvalidateGfx();

    EnableRepaint();

    if( nDiff & (ScCsvDiff::PosOffset | ScCsvDiff::LineOffset) )
        AccSendVisibleEvent();
}

void ScCsvGrid::SetFirstImportedLine( sal_Int32 nLine )
{
    ImplDrawFirstLineSep( false );
    mnFirstImpLine = nLine;
    ImplDrawFirstLineSep( true );
    ImplDrawGridDev();
    Repaint();
}

sal_Int32 ScCsvGrid::GetNoScrollCol( sal_Int32 nPos ) const
{
    sal_Int32 nNewPos = nPos;
    if( nNewPos != CSV_POS_INVALID )
    {
        if( nNewPos < GetFirstVisPos() + CSV_SCROLL_DIST )
        {
            sal_Int32 nScroll = (GetFirstVisPos() > 0) ? CSV_SCROLL_DIST : 0;
            nNewPos = GetFirstVisPos() + nScroll;
        }
        else if( nNewPos > GetLastVisPos() - CSV_SCROLL_DIST - 1 )
        {
            sal_Int32 nScroll = (GetFirstVisPos() < GetMaxPosOffset()) ? CSV_SCROLL_DIST : 0;
            nNewPos = GetLastVisPos() - nScroll - 1;
        }
    }
    return nNewPos;
}

void ScCsvGrid::InitColors()
{
    OSL_PRECOND(mpColorConfig, "the object hasn't been initialized properly");
    if ( !mpColorConfig )
        return;
    maBackColor = mpColorConfig->GetColorValue( ::svtools::DOCCOLOR ).nColor;
    maGridColor = mpColorConfig->GetColorValue( ::svtools::CALCGRID ).nColor;
    maGridPBColor = mpColorConfig->GetColorValue( ::svtools::CALCPAGEBREAK ).nColor;
    maAppBackColor = mpColorConfig->GetColorValue( ::svtools::APPBACKGROUND ).nColor;
    maTextColor = mpColorConfig->GetColorValue( ::svtools::FONTCOLOR ).nColor;

    const StyleSettings& rSett = GetSettings().GetStyleSettings();
    maHeaderBackColor = rSett.GetFaceColor();
    maHeaderGridColor = rSett.GetDarkShadowColor();
    maHeaderTextColor = rSett.GetButtonTextColor();
    maSelectColor = rSett.GetActiveColor();

    InvalidateGfx();
}

void ScCsvGrid::InitFonts()
{
    maMonoFont = OutputDevice::GetDefaultFont( DefaultFontType::FIXED, LANGUAGE_ENGLISH_US, GetDefaultFontFlags::NONE );
    maMonoFont.SetFontSize( Size( maMonoFont.GetFontSize().Width(), maHeaderFont.GetFontSize().Height() ) );

    /* *** Set edit engine defaults ***
        maMonoFont for Latin script, smaller default font for Asian and Complex script. */

    // get default fonts
    SvxFontItem aLatinItem( EE_CHAR_FONTINFO );
    SvxFontItem aAsianItem( EE_CHAR_FONTINFO_CJK );
    SvxFontItem aComplexItem( EE_CHAR_FONTINFO_CTL );
    ::GetDefaultFonts( aLatinItem, aAsianItem, aComplexItem );

    // create item set for defaults
    SfxItemSet aDefSet( mpEditEngine->GetEmptyItemSet() );
    EditEngine::SetFontInfoInItemSet( aDefSet, maMonoFont );
    aDefSet.Put( aAsianItem );
    aDefSet.Put( aComplexItem );

    // set Asian/Complex font size to height of character in Latin font
    sal_uLong nFontHt = static_cast< sal_uLong >( maMonoFont.GetFontSize().Height() );
    aDefSet.Put( SvxFontHeightItem( nFontHt, 100, EE_CHAR_FONTHEIGHT_CJK ) );
    aDefSet.Put( SvxFontHeightItem( nFontHt, 100, EE_CHAR_FONTHEIGHT_CTL ) );

    // copy other items from default font
    const SfxPoolItem& rWeightItem = aDefSet.Get( EE_CHAR_WEIGHT );
    std::unique_ptr<SfxPoolItem> pNewItem(rWeightItem.Clone());
    pNewItem->SetWhich(EE_CHAR_WEIGHT_CJK);
    aDefSet.Put( *pNewItem );
    pNewItem->SetWhich(EE_CHAR_WEIGHT_CTL);
    aDefSet.Put( *pNewItem );
    const SfxPoolItem& rItalicItem = aDefSet.Get( EE_CHAR_ITALIC );
    pNewItem.reset(rItalicItem.Clone());
    pNewItem->SetWhich(EE_CHAR_ITALIC_CJK);
    aDefSet.Put( *pNewItem );
    pNewItem->SetWhich(EE_CHAR_ITALIC_CTL);
    aDefSet.Put( *pNewItem );
    const SfxPoolItem& rLangItem = aDefSet.Get( EE_CHAR_LANGUAGE );
    pNewItem.reset(rLangItem.Clone());
    pNewItem->SetWhich(EE_CHAR_LANGUAGE_CJK);
    aDefSet.Put( *pNewItem );
    pNewItem->SetWhich(EE_CHAR_LANGUAGE_CTL);
    aDefSet.Put( *pNewItem );

    mpEditEngine->SetDefaults( aDefSet );
    InvalidateGfx();
}

void ScCsvGrid::InitSizeData()
{
    maWinSize = GetSizePixel();
    mpBackgrDev->SetOutputSizePixel( maWinSize );
    mpGridDev->SetOutputSizePixel( maWinSize );
    InvalidateGfx();
}

// split handling -------------------------------------------------------------

void ScCsvGrid::InsertSplit( sal_Int32 nPos )
{
    if( ImplInsertSplit( nPos ) )
    {
        DisableRepaint();
        Execute( CSVCMD_EXPORTCOLUMNTYPE );
        Execute( CSVCMD_UPDATECELLTEXTS );
        sal_uInt32 nColIx = GetColumnFromPos( nPos );
        ImplDrawColumn( nColIx - 1 );
        ImplDrawColumn( nColIx );
        ValidateGfx();  // performance: do not redraw all columns
        EnableRepaint();
    }
}

void ScCsvGrid::RemoveSplit( sal_Int32 nPos )
{
    if( ImplRemoveSplit( nPos ) )
    {
        DisableRepaint();
        Execute( CSVCMD_EXPORTCOLUMNTYPE );
        Execute( CSVCMD_UPDATECELLTEXTS );
        ImplDrawColumn( GetColumnFromPos( nPos ) );
        ValidateGfx();  // performance: do not redraw all columns
        EnableRepaint();
    }
}

void ScCsvGrid::MoveSplit( sal_Int32 nPos, sal_Int32 nNewPos )
{
    sal_uInt32 nColIx = GetColumnFromPos( nPos );
    if( nColIx != CSV_COLUMN_INVALID )
    {
        DisableRepaint();
        if( (GetColumnPos( nColIx - 1 ) < nNewPos) && (nNewPos < GetColumnPos( nColIx + 1 )) )
        {
            // move a split in the range between 2 others -> keep selection state of both columns
            maSplits.Remove( nPos );
            maSplits.Insert( nNewPos );
            Execute( CSVCMD_UPDATECELLTEXTS );
            ImplDrawColumn( nColIx - 1 );
            ImplDrawColumn( nColIx );
            ValidateGfx();  // performance: do not redraw all columns
            AccSendTableUpdateEvent( nColIx - 1, nColIx );
        }
        else
        {
            ImplRemoveSplit( nPos );
            ImplInsertSplit( nNewPos );
            Execute( CSVCMD_EXPORTCOLUMNTYPE );
            Execute( CSVCMD_UPDATECELLTEXTS );
        }
        EnableRepaint();
    }
}

void ScCsvGrid::RemoveAllSplits()
{
    DisableRepaint();
    ImplClearSplits();
    Execute( CSVCMD_EXPORTCOLUMNTYPE );
    Execute( CSVCMD_UPDATECELLTEXTS );
    EnableRepaint();
}

void ScCsvGrid::SetSplits( const ScCsvSplits& rSplits )
{
    DisableRepaint();
    ImplClearSplits();
    sal_uInt32 nCount = rSplits.Count();
    for( sal_uInt32 nIx = 0; nIx < nCount; ++nIx )
        maSplits.Insert( rSplits[ nIx ] );
    maColStates.clear();
    maColStates.resize( maSplits.Count() - 1 );
    Execute( CSVCMD_EXPORTCOLUMNTYPE );
    Execute( CSVCMD_UPDATECELLTEXTS );
    EnableRepaint();
}

bool ScCsvGrid::ImplInsertSplit( sal_Int32 nPos )
{
    sal_uInt32 nColIx = GetColumnFromPos( nPos );
    bool bRet = (nColIx < GetColumnCount()) && maSplits.Insert( nPos );
    if( bRet )
    {
        ScCsvColState aState( GetColumnType( nColIx ) );
        aState.Select( IsSelected( nColIx ) && IsSelected( nColIx + 1 ) );
        maColStates.insert( maColStates.begin() + nColIx + 1, aState );
        AccSendInsertColumnEvent( nColIx + 1, nColIx + 1 );
        AccSendTableUpdateEvent( nColIx, nColIx );
    }
    return bRet;
}

bool ScCsvGrid::ImplRemoveSplit( sal_Int32 nPos )
{
    bool bRet = maSplits.Remove( nPos );
    if( bRet )
    {
        sal_uInt32 nColIx = GetColumnFromPos( nPos );
        bool bSel = IsSelected( nColIx ) || IsSelected( nColIx + 1 );
        maColStates.erase( maColStates.begin() + nColIx + 1 );
        maColStates[ nColIx ].Select( bSel );
        AccSendRemoveColumnEvent( nColIx + 1, nColIx + 1 );
        AccSendTableUpdateEvent( nColIx, nColIx );
    }
    return bRet;
}

void ScCsvGrid::ImplClearSplits()
{
    sal_uInt32 nColumns = GetColumnCount();
    maSplits.Clear();
    maSplits.Insert( 0 );
    maSplits.Insert( GetPosCount() );
    maColStates.resize( 1 );
    InvalidateGfx();
    AccSendRemoveColumnEvent( 1, nColumns - 1 );
}

// columns/column types -------------------------------------------------------

sal_uInt32 ScCsvGrid::GetFirstVisColumn() const
{
    return GetColumnFromPos( GetFirstVisPos() );
}

sal_uInt32 ScCsvGrid::GetLastVisColumn() const
{
    return GetColumnFromPos( std::min( GetLastVisPos(), GetPosCount() ) - 1 );
}

bool ScCsvGrid::IsValidColumn( sal_uInt32 nColIndex ) const
{
    return nColIndex < GetColumnCount();
}

bool ScCsvGrid::IsVisibleColumn( sal_uInt32 nColIndex ) const
{
    return  IsValidColumn( nColIndex ) &&
            (GetColumnPos( nColIndex ) < GetLastVisPos()) &&
            (GetFirstVisPos() < GetColumnPos( nColIndex + 1 ));
}

sal_Int32 ScCsvGrid::GetColumnX( sal_uInt32 nColIndex ) const
{
    return GetX( GetColumnPos( nColIndex ) );
}

sal_uInt32 ScCsvGrid::GetColumnFromX( sal_Int32 nX ) const
{
    sal_Int32 nPos = (nX - GetFirstX()) / GetCharWidth() + GetFirstVisPos();
    return ((GetFirstVisPos() <= nPos) && (nPos <= GetLastVisPos())) ?
        GetColumnFromPos( nPos ) : CSV_COLUMN_INVALID;
}

sal_uInt32 ScCsvGrid::GetColumnFromPos( sal_Int32 nPos ) const
{
    return maSplits.UpperBound( nPos );
}

sal_Int32 ScCsvGrid::GetColumnWidth( sal_uInt32 nColIndex ) const
{
    return IsValidColumn( nColIndex ) ? (GetColumnPos( nColIndex + 1 ) - GetColumnPos( nColIndex )) : 0;
}

void ScCsvGrid::SetColumnStates( const ScCsvColStateVec& rStates )
{
    maColStates = rStates;
    maColStates.resize( maSplits.Count() - 1 );
    Execute( CSVCMD_EXPORTCOLUMNTYPE );
    AccSendTableUpdateEvent( 0, GetColumnCount(), false );
    AccSendSelectionEvent();
}

sal_Int32 ScCsvGrid::GetColumnType( sal_uInt32 nColIndex ) const
{
    return IsValidColumn( nColIndex ) ? maColStates[ nColIndex ].mnType : CSV_TYPE_NOSELECTION;
}

void ScCsvGrid::SetColumnType( sal_uInt32 nColIndex, sal_Int32 nColType )
{
    if( IsValidColumn( nColIndex ) )
    {
        maColStates[ nColIndex ].mnType = nColType;
        AccSendTableUpdateEvent( nColIndex, nColIndex, false );
    }
}

sal_Int32 ScCsvGrid::GetSelColumnType() const
{
    sal_uInt32 nColIx = GetFirstSelected();
    if( nColIx == CSV_COLUMN_INVALID )
        return CSV_TYPE_NOSELECTION;

    sal_Int32 nType = GetColumnType( nColIx );
    while( (nColIx != CSV_COLUMN_INVALID) && (nType != CSV_TYPE_MULTI) )
    {
        if( nType != GetColumnType( nColIx ) )
            nType = CSV_TYPE_MULTI;
        nColIx = GetNextSelected( nColIx );
    }
    return nType;
}

void ScCsvGrid::SetSelColumnType( sal_Int32 nType )
{
    if( (nType != CSV_TYPE_MULTI) && (nType != CSV_TYPE_NOSELECTION) )
    {
        for( sal_uInt32 nColIx = GetFirstSelected(); nColIx != CSV_COLUMN_INVALID; nColIx = GetNextSelected( nColIx ) )
            SetColumnType( nColIx, nType );
        Repaint( true );
        Execute( CSVCMD_EXPORTCOLUMNTYPE );
    }
}

void ScCsvGrid::SetTypeNames( const std::vector<OUString>& rTypeNames )
{
    OSL_ENSURE( !rTypeNames.empty(), "ScCsvGrid::SetTypeNames - vector is empty" );
    maTypeNames = rTypeNames;
    Repaint( true );

    mpPopup->Clear();
    sal_uInt32 nCount = maTypeNames.size();
    sal_uInt32 nIx;
    sal_uInt16 nItemId;
    for( nIx = 0, nItemId = 1; nIx < nCount; ++nIx, ++nItemId )
        mpPopup->InsertItem( nItemId, maTypeNames[ nIx ] );

    ::std::for_each( maColStates.begin(), maColStates.end(), Func_SetType( CSV_TYPE_DEFAULT ) );
}

const OUString& ScCsvGrid::GetColumnTypeName( sal_uInt32 nColIndex ) const
{
    sal_uInt32 nTypeIx = static_cast< sal_uInt32 >( GetColumnType( nColIndex ) );
    return (nTypeIx < maTypeNames.size()) ? maTypeNames[ nTypeIx ] : EMPTY_OUSTRING;
}

static sal_uInt8 lcl_GetExtColumnType( sal_Int32 nIntType )
{
    static const sal_uInt8 pExtTypes[] =
        { SC_COL_STANDARD, SC_COL_TEXT, SC_COL_DMY, SC_COL_MDY, SC_COL_YMD, SC_COL_ENGLISH, SC_COL_SKIP };
    static const sal_Int32 nExtTypeCount = SAL_N_ELEMENTS(pExtTypes);
    return pExtTypes[ ((0 <= nIntType) && (nIntType < nExtTypeCount)) ? nIntType : 0 ];
}

void ScCsvGrid::FillColumnDataSep( ScAsciiOptions& rOptions ) const
{
    sal_uInt32 nCount = GetColumnCount();
    ScCsvExpDataVec aDataVec;

    for( sal_uInt32 nColIx = 0; nColIx < nCount; ++nColIx )
    {
        if( GetColumnType( nColIx ) != CSV_TYPE_DEFAULT )
            // 1-based column index
            aDataVec.emplace_back(
                static_cast< sal_Int32 >( nColIx + 1 ),
                lcl_GetExtColumnType( GetColumnType( nColIx ) ) );
    }
    rOptions.SetColumnInfo( aDataVec );
}

void ScCsvGrid::FillColumnDataFix( ScAsciiOptions& rOptions ) const
{
    sal_uInt32 nCount = std::min( GetColumnCount(), static_cast<sal_uInt32>(MAXCOLCOUNT) );
    ScCsvExpDataVec aDataVec( nCount + 1 );

    for( sal_uInt32 nColIx = 0; nColIx < nCount; ++nColIx )
    {
        ScCsvExpData& rData = aDataVec[ nColIx ];
        rData.mnIndex = GetColumnPos( nColIx );
        rData.mnType = lcl_GetExtColumnType( GetColumnType( nColIx ) );
    }
    aDataVec[ nCount ].mnIndex = SAL_MAX_INT32;
    aDataVec[ nCount ].mnType = SC_COL_SKIP;
    rOptions.SetColumnInfo( aDataVec );
}

void ScCsvGrid::ScrollVertRel( ScMoveMode eDir )
{
    sal_Int32 nLine = GetFirstVisLine();
    switch( eDir )
    {
        case MOVE_PREV:     --nLine;                        break;
        case MOVE_NEXT:     ++nLine;                        break;
        case MOVE_FIRST:    nLine = 0;                      break;
        case MOVE_LAST:     nLine = GetMaxLineOffset();     break;
        case MOVE_PREVPAGE: nLine -= GetVisLineCount() - 2; break;
        case MOVE_NEXTPAGE: nLine += GetVisLineCount() - 2; break;
        default:
        {
            // added to avoid warnings
        }
    }
    Execute( CSVCMD_SETLINEOFFSET, nLine );
}

void ScCsvGrid::ExecutePopup( const Point& rPos )
{
    sal_uInt16 nItemId = mpPopup->Execute( this, rPos );
    if( nItemId )   // 0 = cancelled
        Execute( CSVCMD_SETCOLUMNTYPE, mpPopup->GetItemPos( nItemId ) );
}

// selection handling ---------------------------------------------------------

bool ScCsvGrid::IsSelected( sal_uInt32 nColIndex ) const
{
    return IsValidColumn( nColIndex ) && maColStates[ nColIndex ].IsSelected();
}

sal_uInt32 ScCsvGrid::GetFirstSelected() const
{
    return IsSelected( 0 ) ? 0 : GetNextSelected( 0 );
}

sal_uInt32 ScCsvGrid::GetNextSelected( sal_uInt32 nFromIndex ) const
{
    sal_uInt32 nColCount = GetColumnCount();
    for( sal_uInt32 nColIx = nFromIndex + 1; nColIx < nColCount; ++nColIx )
        if( IsSelected( nColIx ) )
            return nColIx;
    return CSV_COLUMN_INVALID;
}

void ScCsvGrid::Select( sal_uInt32 nColIndex, bool bSelect )
{
    if( IsValidColumn( nColIndex ) )
    {
        maColStates[ nColIndex ].Select( bSelect );
        ImplDrawColumnSelection( nColIndex );
        Repaint();
        Execute( CSVCMD_EXPORTCOLUMNTYPE );
        if( bSelect )
            mnRecentSelCol = nColIndex;
        AccSendSelectionEvent();
    }
}

void ScCsvGrid::ToggleSelect( sal_uInt32 nColIndex )
{
    Select( nColIndex, !IsSelected( nColIndex ) );
}

void ScCsvGrid::SelectRange( sal_uInt32 nColIndex1, sal_uInt32 nColIndex2, bool bSelect )
{
    if( nColIndex1 == CSV_COLUMN_INVALID )
        Select( nColIndex2 );
    else if( nColIndex2 == CSV_COLUMN_INVALID )
        Select( nColIndex1 );
    else if( nColIndex1 > nColIndex2 )
    {
        SelectRange( nColIndex2, nColIndex1, bSelect );
        if( bSelect )
            mnRecentSelCol = nColIndex1;
    }
    else if( IsValidColumn( nColIndex1 ) && IsValidColumn( nColIndex2 ) )
    {
        for( sal_uInt32 nColIx = nColIndex1; nColIx <= nColIndex2; ++nColIx )
        {
            maColStates[ nColIx ].Select( bSelect );
            ImplDrawColumnSelection( nColIx );
        }
        Repaint();
        Execute( CSVCMD_EXPORTCOLUMNTYPE );
        if( bSelect )
            mnRecentSelCol = nColIndex1;
        AccSendSelectionEvent();
    }
}

void ScCsvGrid::SelectAll( bool bSelect )
{
    SelectRange( 0, GetColumnCount() - 1, bSelect );
}

void ScCsvGrid::MoveCursor( sal_uInt32 nColIndex )
{
    DisableRepaint();
    if( IsValidColumn( nColIndex ) )
    {
        sal_Int32 nPosBeg = GetColumnPos( nColIndex );
        sal_Int32 nPosEnd = GetColumnPos( nColIndex + 1 );
        sal_Int32 nMinPos = std::max( nPosBeg - CSV_SCROLL_DIST, sal_Int32( 0 ) );
        sal_Int32 nMaxPos = std::min( nPosEnd - GetVisPosCount() + CSV_SCROLL_DIST + sal_Int32( 1 ), nMinPos );
        if( nPosBeg - CSV_SCROLL_DIST + 1 <= GetFirstVisPos() )
            Execute( CSVCMD_SETPOSOFFSET, nMinPos );
        else if( nPosEnd + CSV_SCROLL_DIST >= GetLastVisPos() )
            Execute( CSVCMD_SETPOSOFFSET, nMaxPos );
    }
    Execute( CSVCMD_MOVEGRIDCURSOR, GetColumnPos( nColIndex ) );
    EnableRepaint();
}

void ScCsvGrid::MoveCursorRel( ScMoveMode eDir )
{
    if( GetFocusColumn() != CSV_COLUMN_INVALID )
    {
        switch( eDir )
        {
            case MOVE_FIRST:
                MoveCursor( 0 );
            break;
            case MOVE_LAST:
                MoveCursor( GetColumnCount() - 1 );
            break;
            case MOVE_PREV:
                if( GetFocusColumn() > 0 )
                    MoveCursor( GetFocusColumn() - 1 );
            break;
            case MOVE_NEXT:
                if( GetFocusColumn() < GetColumnCount() - 1 )
                    MoveCursor( GetFocusColumn() + 1 );
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
}

void ScCsvGrid::ImplClearSelection()
{
    ::std::for_each( maColStates.begin(), maColStates.end(), Func_Select( false ) );
    ImplDrawGridDev();
}

void ScCsvGrid::DoSelectAction( sal_uInt32 nColIndex, sal_uInt16 nModifier )
{
    if( !(nModifier & KEY_MOD1) )
        ImplClearSelection();
    if( nModifier & KEY_SHIFT )             // SHIFT always expands
        SelectRange( mnRecentSelCol, nColIndex );
    else if( !(nModifier & KEY_MOD1) )      // no SHIFT/CTRL always selects 1 column
        Select( nColIndex );
    else if( IsTracking() )                 // CTRL in tracking does not toggle
        Select( nColIndex, mbMTSelecting );
    else                                    // CTRL only toggles
        ToggleSelect( nColIndex );
    Execute( CSVCMD_MOVEGRIDCURSOR, GetColumnPos( nColIndex ) );
}

// cell contents --------------------------------------------------------------

void ScCsvGrid::ImplSetTextLineSep(
        sal_Int32 nLine, const OUString& rTextLine,
        const OUString& rSepChars, sal_Unicode cTextSep, bool bMergeSep, bool bRemoveSpace )
{
    if( nLine < GetFirstVisLine() ) return;

    sal_uInt32 nLineIx = nLine - GetFirstVisLine();
    while( maTexts.size() <= nLineIx )
        maTexts.emplace_back( );
    std::vector<OUString>& rStrVec = maTexts[ nLineIx ];
    rStrVec.clear();

    // scan for separators
    OUString aCellText;
    const sal_Unicode* pSepChars = rSepChars.getStr();
    const sal_Unicode* pChar = rTextLine.getStr();
    sal_uInt32 nColIx = 0;

    while( *pChar && (nColIx < sal::static_int_cast<sal_uInt32>(CSV_MAXCOLCOUNT)) )
    {
        // scan for next cell text
        bool bIsQuoted = false;
        bool bOverflowCell = false;
        pChar = ScImportExport::ScanNextFieldFromString( pChar, aCellText,
                cTextSep, pSepChars, bMergeSep, bIsQuoted, bOverflowCell, bRemoveSpace );
        /* TODO: signal overflow somewhere in UI */

        // update column width
        sal_Int32 nWidth = std::max( CSV_MINCOLWIDTH, aCellText.getLength() + 1 );
        if( IsValidColumn( nColIx ) )
        {
            // expand existing column
            sal_Int32 nDiff = nWidth - GetColumnWidth( nColIx );
            if( nDiff > 0 )
            {
                Execute( CSVCMD_SETPOSCOUNT, GetPosCount() + nDiff );
                for( sal_uInt32 nSplitIx = GetColumnCount() - 1; nSplitIx > nColIx; --nSplitIx )
                {
                    sal_Int32 nPos = maSplits[ nSplitIx ];
                    maSplits.Remove( nPos );
                    maSplits.Insert( nPos + nDiff );
                }
            }
        }
        else
        {
            // append new column
            sal_Int32 nLastPos = GetPosCount();
            Execute( CSVCMD_SETPOSCOUNT, nLastPos + nWidth );
            ImplInsertSplit( nLastPos );
        }

        if( aCellText.getLength() <= CSV_MAXSTRLEN )
            rStrVec.push_back( aCellText );
        else
            rStrVec.push_back( aCellText.copy( 0, CSV_MAXSTRLEN ) );
        ++nColIx;
    }
    InvalidateGfx();
}

void ScCsvGrid::ImplSetTextLineFix( sal_Int32 nLine, const OUString& rTextLine )
{
    if( nLine < GetFirstVisLine() ) return;

    sal_Int32 nChars = rTextLine.getLength();
    if( nChars > GetPosCount() )
        Execute( CSVCMD_SETPOSCOUNT, nChars );

    sal_uInt32 nLineIx = nLine - GetFirstVisLine();
    while( maTexts.size() <= nLineIx )
        maTexts.emplace_back( );

    std::vector<OUString>& rStrVec = maTexts[ nLineIx ];
    rStrVec.clear();
    sal_uInt32 nColCount = GetColumnCount();
    sal_Int32 nStrLen = rTextLine.getLength();
    sal_Int32 nStrIx = 0;
    for( sal_uInt32 nColIx = 0; (nColIx < nColCount) && (nStrIx < nStrLen); ++nColIx )
    {
        sal_Int32 nColWidth = GetColumnWidth( nColIx );
        sal_Int32 nLen = std::min( std::min( nColWidth, CSV_MAXSTRLEN ), nStrLen - nStrIx);
        rStrVec.push_back( rTextLine.copy( nStrIx, nLen ) );
        nStrIx = nStrIx + nColWidth;
    }
    InvalidateGfx();
}

const OUString& ScCsvGrid::GetCellText( sal_uInt32 nColIndex, sal_Int32 nLine ) const
{
    if( nLine < GetFirstVisLine() ) return EMPTY_OUSTRING;

    sal_uInt32 nLineIx = nLine - GetFirstVisLine();
    if( nLineIx >= maTexts.size() ) return EMPTY_OUSTRING;

    const std::vector<OUString>& rStrVec = maTexts[ nLineIx ];
    if( nColIndex >= rStrVec.size() ) return EMPTY_OUSTRING;

    return rStrVec[ nColIndex ];
}

// event handling -------------------------------------------------------------

void ScCsvGrid::Resize()
{
    ScCsvControl::Resize();
    InitSizeData();
    Execute( CSVCMD_UPDATECELLTEXTS );
}

void ScCsvGrid::GetFocus()
{
    ScCsvControl::GetFocus();
    Execute( CSVCMD_MOVEGRIDCURSOR, GetNoScrollCol( GetGridCursorPos() ) );
    Repaint();
}

void ScCsvGrid::LoseFocus()
{
    ScCsvControl::LoseFocus();
    Repaint();
}

void ScCsvGrid::MouseButtonDown( const MouseEvent& rMEvt )
{
    DisableRepaint();
    if( !HasFocus() )
        GrabFocus();

    Point aPos( rMEvt.GetPosPixel() );
    sal_uInt32 nColIx = GetColumnFromX( aPos.X() );

    if( rMEvt.IsLeft() )
    {
        if( (GetFirstX() > aPos.X()) || (aPos.X() > GetLastX()) )   // in header column
        {
            if( aPos.Y() <= GetHdrHeight() )
                SelectAll();
        }
        else if( IsValidColumn( nColIx ) )
        {
            DoSelectAction( nColIx, rMEvt.GetModifier() );
            mnMTCurrCol = nColIx;
            mbMTSelecting = IsSelected( nColIx );
            StartTracking( StartTrackingFlags::ButtonRepeat );
        }
    }
    EnableRepaint();
}

void ScCsvGrid::Tracking( const TrackingEvent& rTEvt )
{
    if( rTEvt.IsTrackingEnded() || rTEvt.IsTrackingRepeat() )
    {
        DisableRepaint();
        const MouseEvent& rMEvt = rTEvt.GetMouseEvent();

        sal_Int32 nPos = (rMEvt.GetPosPixel().X() - GetFirstX()) / GetCharWidth() + GetFirstVisPos();
        // on mouse tracking: keep position valid
        nPos = std::max( std::min( nPos, GetPosCount() - sal_Int32( 1 ) ), sal_Int32( 0 ) );
        Execute( CSVCMD_MAKEPOSVISIBLE, nPos );

        sal_uInt32 nColIx = GetColumnFromPos( nPos );
        if( mnMTCurrCol != nColIx )
        {
            DoSelectAction( nColIx, rMEvt.GetModifier() );
            mnMTCurrCol = nColIx;
        }
        EnableRepaint();
    }
}

void ScCsvGrid::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode& rKCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = rKCode.GetCode();
    bool bShift = rKCode.IsShift();
    bool bMod1 = rKCode.IsMod1();

    if( !rKCode.IsMod2() )
    {
        ScMoveMode eHDir = GetHorzDirection( nCode, !bMod1 );
        ScMoveMode eVDir = GetVertDirection( nCode, bMod1 );

        if( eHDir != MOVE_NONE )
        {
            DisableRepaint();
            MoveCursorRel( eHDir );
            if( !bMod1 )
                ImplClearSelection();
            if( bShift )
                SelectRange( mnRecentSelCol, GetFocusColumn() );
            else if( !bMod1 )
                Select( GetFocusColumn() );
            EnableRepaint();
        }
        else if( eVDir != MOVE_NONE )
            ScrollVertRel( eVDir );
        else if( nCode == KEY_SPACE )
        {
            if( !bMod1 )
                ImplClearSelection();
            if( bShift )
                SelectRange( mnRecentSelCol, GetFocusColumn() );
            else if( bMod1 )
                ToggleSelect( GetFocusColumn() );
            else
                Select( GetFocusColumn() );
        }
        else if( !bShift && bMod1 )
        {
            if( nCode == KEY_A )
                SelectAll();
            else if( (KEY_1 <= nCode) && (nCode <= KEY_9) )
            {
                sal_uInt32 nType = nCode - KEY_1;
                if( nType < maTypeNames.size() )
                    Execute( CSVCMD_SETCOLUMNTYPE, nType );
            }
        }
    }

    if( rKCode.GetGroup() != KEYGROUP_CURSOR )
        ScCsvControl::KeyInput( rKEvt );
}

void ScCsvGrid::Command( const CommandEvent& rCEvt )
{
    switch( rCEvt.GetCommand() )
    {
        case CommandEventId::ContextMenu:
        {
            if( rCEvt.IsMouseEvent() )
            {
                Point aPos( rCEvt.GetMousePosPixel() );
                sal_uInt32 nColIx = GetColumnFromX( aPos.X() );
                if( IsValidColumn( nColIx ) && (GetFirstX() <= aPos.X()) && (aPos.X() <= GetLastX()) )
                {
                    if( !IsSelected( nColIx ) )
                        DoSelectAction( nColIx, 0 );    // focus & select
                    ExecutePopup( aPos );
                }
            }
            else
            {
                sal_uInt32 nColIx = GetFocusColumn();
                if( !IsSelected( nColIx ) )
                    Select( nColIx );
                sal_Int32 nX1 = std::max( GetColumnX( nColIx ), GetFirstX() );
                sal_Int32 nX2 = std::min( GetColumnX( nColIx + 1 ), GetWidth() );
                ExecutePopup( Point( (nX1 + nX2) / 2, GetHeight() / 2 ) );
            }
        }
        break;
        case CommandEventId::Wheel:
        {
            tools::Rectangle aRect( Point(), maWinSize );
            if( aRect.IsInside( rCEvt.GetMousePosPixel() ) )
            {
                const CommandWheelData* pData = rCEvt.GetWheelData();
                if( pData && (pData->GetMode() == CommandWheelMode::SCROLL) && !pData->IsHorz() )
                    Execute( CSVCMD_SETLINEOFFSET, GetFirstVisLine() - pData->GetNotchDelta() );
            }
        }
        break;
        default:
            ScCsvControl::Command( rCEvt );
    }
}

void ScCsvGrid::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        InitColors();
        InitFonts();
        UpdateLayoutData();
        Execute( CSVCMD_UPDATECELLTEXTS );
    }
    ScCsvControl::DataChanged( rDCEvt );
}

void ScCsvGrid::ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints )
{
    InitColors();
    Repaint();
}

// painting -------------------------------------------------------------------

void ScCsvGrid::Paint( vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& )
{
    Repaint();
}

void ScCsvGrid::ImplRedraw()
{
    if( IsVisible() )
    {
        if( !IsValidGfx() )
        {
            ValidateGfx();
            ImplDrawBackgrDev();
            ImplDrawGridDev();
        }
        DrawOutDev( Point(), maWinSize, Point(), maWinSize, *mpGridDev );
        ImplDrawTrackingRect( GetFocusColumn() );
    }
}

EditEngine* ScCsvGrid::GetEditEngine()
{
    return mpEditEngine.get();
}

void ScCsvGrid::ImplSetColumnClipRegion( OutputDevice& rOutDev, sal_uInt32 nColIndex )
{
    rOutDev.SetClipRegion( vcl::Region( tools::Rectangle(
        std::max( GetColumnX( nColIndex ), GetFirstX() ) + 1, 0,
        std::min( GetColumnX( nColIndex + 1 ), GetLastX() ), GetHeight() - 1 ) ) );
}

void ScCsvGrid::ImplDrawColumnHeader( OutputDevice& rOutDev, sal_uInt32 nColIndex, Color aFillColor )
{
    sal_Int32 nX1 = GetColumnX( nColIndex ) + 1;
    sal_Int32 nX2 = GetColumnX( nColIndex + 1 );
    sal_Int32 nHdrHt = GetHdrHeight();

    rOutDev.SetLineColor();
    rOutDev.SetFillColor( aFillColor );
    rOutDev.DrawRect( tools::Rectangle( nX1, 0, nX2, nHdrHt ) );

    rOutDev.SetFont( maHeaderFont );
    rOutDev.SetTextColor( maHeaderTextColor );
    rOutDev.SetTextFillColor();
    rOutDev.DrawText( Point( nX1 + 1, 0 ), GetColumnTypeName( nColIndex ) );

    rOutDev.SetLineColor( maHeaderGridColor );
    rOutDev.DrawLine( Point( nX1, nHdrHt ), Point( nX2, nHdrHt ) );
    rOutDev.DrawLine( Point( nX2, 0 ), Point( nX2, nHdrHt ) );
}

void ScCsvGrid::ImplDrawCellText( const Point& rPos, const OUString& rText )
{
    OUString aPlainText( rText );
    aPlainText = aPlainText.replaceAll( "\t", " " );
    aPlainText = aPlainText.replaceAll( "\n", " " );
    mpEditEngine->SetPaperSize( maEdEngSize );

    /*  #i60296# If string contains mixed script types, the space character
        U+0020 may be drawn with a wrong width (from non-fixed-width Asian or
        Complex font). Now we draw every non-space portion separately. */
    sal_Int32 nCharIxInt {aPlainText.isEmpty() ? -1 : 0};
    while (nCharIxInt>=0)
    {
        sal_Int32 nBeginIx = nCharIxInt;
        const OUString aToken = aPlainText.getToken( 0, ' ', nCharIxInt );
        if( !aToken.isEmpty() )
        {
            sal_Int32 nX = rPos.X() + GetCharWidth() * nBeginIx;
            mpEditEngine->SetText( aToken );
            mpEditEngine->Draw( mpBackgrDev.get(), Point( nX, rPos.Y() ) );
        }
    }

    sal_Int32 nCharIx = 0;
    while( (nCharIx = rText.indexOf( '\t', nCharIx )) != -1 )
    {
        sal_Int32 nX1 = rPos.X() + GetCharWidth() * nCharIx;
        sal_Int32 nX2 = nX1 + GetCharWidth() - 2;
        sal_Int32 nY = rPos.Y() + GetLineHeight() / 2;
        Color aColor( maTextColor );
        mpBackgrDev->SetLineColor( aColor );
        mpBackgrDev->DrawLine( Point( nX1, nY ), Point( nX2, nY ) );
        mpBackgrDev->DrawLine( Point( nX2 - 2, nY - 2 ), Point( nX2, nY ) );
        mpBackgrDev->DrawLine( Point( nX2 - 2, nY + 2 ), Point( nX2, nY ) );
        ++nCharIx;
    }
    nCharIx = 0;
    while( (nCharIx = rText.indexOf( '\n', nCharIx )) != -1 )
    {
        sal_Int32 nX1 = rPos.X() + GetCharWidth() * nCharIx;
        sal_Int32 nX2 = nX1 + GetCharWidth() - 2;
        sal_Int32 nY = rPos.Y() + GetLineHeight() / 2;
        Color aColor( maTextColor );
        mpBackgrDev->SetLineColor( aColor );
        mpBackgrDev->DrawLine( Point( nX1, nY ), Point( nX2, nY ) );
        mpBackgrDev->DrawLine( Point( nX1 + 2, nY - 2 ), Point( nX1, nY ) );
        mpBackgrDev->DrawLine( Point( nX1 + 2, nY + 2 ), Point( nX1, nY ) );
        mpBackgrDev->DrawLine( Point( nX2, nY - 2 ), Point( nX2, nY ) );
        ++nCharIx;
    }
}

void ScCsvGrid::ImplDrawFirstLineSep( bool bSet )
{
    if( IsVisibleLine( mnFirstImpLine ) && (mnFirstImpLine != GetFirstVisLine() ) )
    {
        sal_Int32 nY = GetY( mnFirstImpLine );
        sal_Int32 nX = std::min( GetColumnX( GetLastVisColumn() + 1 ), GetLastX() );
        mpBackgrDev->SetLineColor( bSet ? maGridPBColor : maGridColor );
        mpBackgrDev->DrawLine( Point( GetFirstX() + 1, nY ), Point( nX, nY ) );
    }
}

void ScCsvGrid::ImplDrawColumnBackgr( sal_uInt32 nColIndex )
{
    if( !IsVisibleColumn( nColIndex ) )
        return;

    ImplSetColumnClipRegion( *mpBackgrDev, nColIndex );

    // grid
    mpBackgrDev->SetLineColor();
    mpBackgrDev->SetFillColor( maBackColor );
    sal_Int32 nX1 = GetColumnX( nColIndex ) + 1;
    sal_Int32 nX2 = GetColumnX( nColIndex + 1 );
    sal_Int32 nY2 = GetY( GetLastVisLine() + 1 );
    sal_Int32 nHdrHt = GetHdrHeight();
    tools::Rectangle aRect( nX1, nHdrHt, nX2, nY2 );
    mpBackgrDev->DrawRect( aRect );
    mpBackgrDev->SetLineColor( maGridColor );
    mpBackgrDev->DrawGrid( aRect, Size( 1, GetLineHeight() ), DrawGridFlags::HorzLines );
    mpBackgrDev->DrawLine( Point( nX2, nHdrHt ), Point( nX2, nY2 ) );
    ImplDrawFirstLineSep( true );

    // cell texts
    mpEditEngine->SetDefaultItem( SvxColorItem( maTextColor, EE_CHAR_COLOR ) );
    size_t nLineCount = ::std::min( static_cast< size_t >( GetLastVisLine() - GetFirstVisLine() + 1 ), maTexts.size() );
    // #i67432# cut string to avoid edit engine performance problems with very large strings
    sal_Int32 nFirstVisPos = ::std::max( GetColumnPos( nColIndex ), GetFirstVisPos() );
    sal_Int32 nLastVisPos = ::std::min( GetColumnPos( nColIndex + 1 ), GetLastVisPos() );
    sal_Int32 nStrPos = nFirstVisPos - GetColumnPos( nColIndex );
    sal_Int32 nStrLen = nLastVisPos - nFirstVisPos + 1;
    sal_Int32 nStrX = GetX( nFirstVisPos );
    for( size_t nLine = 0; nLine < nLineCount; ++nLine )
    {
        std::vector<OUString>& rStrVec = maTexts[ nLine ];
        if( (nColIndex < rStrVec.size()) && (rStrVec[ nColIndex ].getLength() > nStrPos) )
        {
            const OUString& rStr = rStrVec[ nColIndex ];
            OUString aText = rStr.copy( nStrPos, ::std::min( nStrLen, rStr.getLength() - nStrPos) );
            ImplDrawCellText( Point( nStrX, GetY( GetFirstVisLine() + nLine ) ), aText );
        }
    }

    // header
    ImplDrawColumnHeader( *mpBackgrDev, nColIndex, maHeaderBackColor );

    mpBackgrDev->SetClipRegion();
}

void ScCsvGrid::ImplDrawRowHeaders()
{
    mpBackgrDev->SetLineColor();
    mpBackgrDev->SetFillColor( maAppBackColor );
    Point aPoint( GetHdrX(), 0 );
    tools::Rectangle aRect( aPoint, Size( GetHdrWidth() + 1, GetHeight() ) );
    mpBackgrDev->DrawRect( aRect );

    mpBackgrDev->SetFillColor( maHeaderBackColor );
    aRect.SetBottom( GetY( GetLastVisLine() + 1 ) );
    mpBackgrDev->DrawRect( aRect );

    // line numbers
    mpBackgrDev->SetFont( maHeaderFont );
    mpBackgrDev->SetTextColor( maHeaderTextColor );
    mpBackgrDev->SetTextFillColor();
    sal_Int32 nLastLine = GetLastVisLine();
    for( sal_Int32 nLine = GetFirstVisLine(); nLine <= nLastLine; ++nLine )
    {
        OUString aText( OUString::number( nLine + 1 ) );
        sal_Int32 nX = GetHdrX() + (GetHdrWidth() - mpBackgrDev->GetTextWidth( aText )) / 2;
        mpBackgrDev->DrawText( Point( nX, GetY( nLine ) ), aText );
    }

    // grid
    mpBackgrDev->SetLineColor( maHeaderGridColor );
    if( IsRTL() )
    {
        mpBackgrDev->DrawLine( Point( 0, 0 ), Point( 0, GetHeight() - 1 ) );
        mpBackgrDev->DrawLine( aRect.TopLeft(), aRect.BottomLeft() );
    }
    else
        mpBackgrDev->DrawLine( aRect.TopRight(), aRect.BottomRight() );
    aRect.SetTop( GetHdrHeight() );
    mpBackgrDev->DrawGrid( aRect, Size( 1, GetLineHeight() ), DrawGridFlags::HorzLines );
}

void ScCsvGrid::ImplDrawBackgrDev()
{
    mpBackgrDev->SetLineColor();
    mpBackgrDev->SetFillColor( maAppBackColor );
    mpBackgrDev->DrawRect( tools::Rectangle(
        Point( GetFirstX() + 1, 0 ), Size( GetWidth() - GetHdrWidth(), GetHeight() ) ) );

    sal_uInt32 nLastCol = GetLastVisColumn();
    if (nLastCol == CSV_COLUMN_INVALID)
        return;
    for( sal_uInt32 nColIx = GetFirstVisColumn(); nColIx <= nLastCol; ++nColIx )
        ImplDrawColumnBackgr( nColIx );

    ImplDrawRowHeaders();
}

void ScCsvGrid::ImplDrawColumnSelection( sal_uInt32 nColIndex )
{
    ImplInvertCursor( GetRulerCursorPos() );
    ImplSetColumnClipRegion( *mpGridDev, nColIndex );
    mpGridDev->DrawOutDev( Point(), maWinSize, Point(), maWinSize, *mpBackgrDev );

    if( IsSelected( nColIndex ) )
    {
        sal_Int32 nX1 = GetColumnX( nColIndex ) + 1;
        sal_Int32 nX2 = GetColumnX( nColIndex + 1 );

        // header
        tools::Rectangle aRect( nX1, 0, nX2, GetHdrHeight() );
        mpGridDev->SetLineColor();
        if( maHeaderBackColor.IsDark() )
            // redraw with light gray background in dark mode
            ImplDrawColumnHeader( *mpGridDev, nColIndex, COL_LIGHTGRAY );
        else
        {
            // use transparent active color
            mpGridDev->SetFillColor( maSelectColor );
            mpGridDev->DrawTransparent( tools::PolyPolygon( tools::Polygon( aRect ) ), CSV_HDR_TRANSPARENCY );
        }

        // column selection
        aRect = tools::Rectangle( nX1, GetHdrHeight() + 1, nX2, GetY( GetLastVisLine() + 1 ) - 1 );
        ImplInvertRect( *mpGridDev, aRect );
    }

    mpGridDev->SetClipRegion();
    ImplInvertCursor( GetRulerCursorPos() );
}

void ScCsvGrid::ImplDrawGridDev()
{
    mpGridDev->DrawOutDev( Point(), maWinSize, Point(), maWinSize, *mpBackgrDev );
    sal_uInt32 nLastCol = GetLastVisColumn();
    if (nLastCol == CSV_COLUMN_INVALID)
        return;
    for( sal_uInt32 nColIx = GetFirstVisColumn(); nColIx <= nLastCol; ++nColIx )
        ImplDrawColumnSelection( nColIx );
}

void ScCsvGrid::ImplDrawColumn( sal_uInt32 nColIndex )
{
    ImplDrawColumnBackgr( nColIndex );
    ImplDrawColumnSelection( nColIndex );
}

void ScCsvGrid::ImplDrawHorzScrolled( sal_Int32 nOldPos )
{
    sal_Int32 nPos = GetFirstVisPos();
    if( !IsValidGfx() || (nPos == nOldPos) )
        return;
    if( std::abs( nPos - nOldPos ) > GetVisPosCount() / 2 )
    {
        ImplDrawBackgrDev();
        ImplDrawGridDev();
        return;
    }

    Point aSrc, aDest;
    sal_uInt32 nFirstColIx, nLastColIx;
    if( nPos < nOldPos )
    {
        aSrc = Point( GetFirstX() + 1, 0 );
        aDest = Point( GetFirstX() + GetCharWidth() * (nOldPos - nPos) + 1, 0 );
        nFirstColIx = GetColumnFromPos( nPos );
        nLastColIx = GetColumnFromPos( nOldPos );
    }
    else
    {
        aSrc = Point( GetFirstX() + GetCharWidth() * (nPos - nOldPos) + 1, 0 );
        aDest = Point( GetFirstX() + 1, 0 );
        nFirstColIx = GetColumnFromPos( std::min( nOldPos + GetVisPosCount(), GetPosCount() ) - 1 );
        nLastColIx = GetColumnFromPos( std::min( nPos + GetVisPosCount(), GetPosCount() ) - 1 );
    }

    ImplInvertCursor( GetRulerCursorPos() + (nPos - nOldPos) );
    tools::Rectangle aRectangle( GetFirstX(), 0, GetLastX(), GetHeight() - 1 );
    vcl::Region aClipReg( aRectangle );
    mpBackgrDev->SetClipRegion( aClipReg );
    mpBackgrDev->CopyArea( aDest, aSrc, maWinSize );
    mpBackgrDev->SetClipRegion();
    mpGridDev->SetClipRegion( aClipReg );
    mpGridDev->CopyArea( aDest, aSrc, maWinSize );
    mpGridDev->SetClipRegion();
    ImplInvertCursor( GetRulerCursorPos() );

    for( sal_uInt32 nColIx = nFirstColIx; nColIx <= nLastColIx; ++nColIx )
        ImplDrawColumn( nColIx );

    sal_Int32 nLastX = GetX( GetPosCount() ) + 1;
    if( nLastX <= GetLastX() )
    {
        tools::Rectangle aRect( nLastX, 0, GetLastX(), GetHeight() - 1 );
        mpBackgrDev->SetLineColor();
        mpBackgrDev->SetFillColor( maAppBackColor );
        mpBackgrDev->DrawRect( aRect );
        mpGridDev->SetLineColor();
        mpGridDev->SetFillColor( maAppBackColor );
        mpGridDev->DrawRect( aRect );
    }
}

void ScCsvGrid::ImplInvertCursor( sal_Int32 nPos )
{
    if( IsVisibleSplitPos( nPos ) )
    {
        sal_Int32 nX = GetX( nPos ) - 1;
        tools::Rectangle aRect( Point( nX, 0 ), Size( 3, GetHdrHeight() ) );
        ImplInvertRect( *mpGridDev, aRect );
        aRect.SetTop( GetHdrHeight() + 1 );
        aRect.SetBottom( GetY( GetLastVisLine() + 1 ) );
        ImplInvertRect( *mpGridDev, aRect );
    }
}

void ScCsvGrid::ImplDrawTrackingRect( sal_uInt32 nColIndex )
{
    if( HasFocus() && IsVisibleColumn( nColIndex ) )
    {
        sal_Int32 nX1 = std::max( GetColumnX( nColIndex ), GetFirstX() ) + 1;
        sal_Int32 nX2 = std::min( GetColumnX( nColIndex + 1 ) - sal_Int32( 1 ), GetLastX() );
        sal_Int32 nY2 = std::min( GetY( GetLastVisLine() + 1 ), GetHeight() ) - 1;
        InvertTracking( tools::Rectangle( nX1, 0, nX2, nY2 ), ShowTrackFlags::Small | ShowTrackFlags::TrackWindow );
    }
}

// accessibility ==============================================================

rtl::Reference<ScAccessibleCsvControl> ScCsvGrid::ImplCreateAccessible()
{
    rtl::Reference<ScAccessibleCsvControl> pControl(new ScAccessibleCsvGrid( *this ));
    pControl->Init();
    return pControl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
