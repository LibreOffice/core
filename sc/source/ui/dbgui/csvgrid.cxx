/*************************************************************************
 *
 *  $RCSfile: csvgrid.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dr $ $Date: 2002-07-11 15:39:47 $
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

#ifndef _SC_CSVGRID_HXX
#include "csvgrid.hxx"
#endif

#ifndef _SVX_COLORCFG_HXX
#include <svx/colorcfg.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif
#ifndef _SV_POLY_HXX
#include <vcl/poly.hxx>
#endif

#ifndef SC_SCMOD_HXX
#include "scmod.hxx"
#endif
#ifndef SC_ASCIIOPT_HXX
#include "asciiopt.hxx"
#endif
#ifndef SC_IMPEX_HXX
#include "impex.hxx"
#endif

// *** edit engine ***
#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/colritem.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/fontitem.hxx>
#include <svtools/itemset.hxx>

#ifndef SC_EDITUTIL_HXX
#include "editutil.hxx"
#endif
// *** edit engine ***


// ============================================================================

// column flags
const sal_uInt8 CSV_COLFLAG_NONE    = 0x00;
const sal_uInt8 CSV_COLFLAG_SELECT  = 0x01;     /// Column is selected.

/** Modifies the specified bits (rMask) in the bitfield rValue. */
template< typename Type >
inline void set_bits( Type& rValue, const Type& rMask, bool bSet )
{
    if( bSet ) rValue |= rMask; else rValue &= ~rMask;
}


// ============================================================================

ScCsvGrid::ScCsvGrid( ScCsvControl& rParent ) :
    ScCsvControl( rParent ),
    mrColorConfig( SC_MOD()->GetColorConfig() ),
    mpEditEngine( new ScEditEngineDefaulter( EditEngine::CreatePool(), TRUE ) ),
    maHeaderFont( GetFont() ),
    maColTypes( 1 ),
    maColFlags( 1 ),
    maTypeNames( 1 ),
    mnRecentSelCol( VEC_NOTFOUND )
{
    mpEditEngine->SetRefDevice( &maBackgrDev );

    maColFlags[ 0 ] = CSV_COLFLAG_NONE;
    SetColumnType( 0, CSV_TYPE_DEFAULT );
    maPopup.SetMenuFlags( maPopup.GetMenuFlags() | MENU_FLAG_NOAUTOMNEMONICS );

    InitColors();
    InitFonts();
    ImplClearSplits();
    StartListening( mrColorConfig );
}

ScCsvGrid::~ScCsvGrid()
{
    EndListening( mrColorConfig );
}


// initialization -------------------------------------------------------------

void ScCsvGrid::InitColors()
{
    maBackColor = Color( mrColorConfig.GetColorValue( ::svx::DOCCOLOR ).nColor );
    maGridColor = Color( mrColorConfig.GetColorValue( ::svx::CALCGRID ).nColor );
    maAppBackColor = Color( mrColorConfig.GetColorValue( ::svx::APPBACKGROUND ).nColor );

    const StyleSettings& rSett = GetSettings().GetStyleSettings();
    maTextColor = rSett.GetButtonTextColor();
    maHeaderBackColor = rSett.GetFaceColor();
    maHeaderGridColor = rSett.GetDarkShadowColor();
    maSelectColor = rSett.GetActiveColor();

    InvalidateGfx();
}

void ScCsvGrid::InitFonts()
{
    maMonoFont = OutputDevice::GetDefaultFont( DEFAULTFONT_FIXED, LANGUAGE_ENGLISH_US, 0 );
    maMonoFont.SetSize( Size( maMonoFont.GetSize().Width(), maHeaderFont.GetSize().Height() ) );

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

    // set Asian/Complex font size to width of character in Latin font
    aDefSet.Put( SvxFontHeightItem( GetCharWidth(), 100, EE_CHAR_FONTHEIGHT_CJK ) );
    aDefSet.Put( SvxFontHeightItem( GetCharWidth(), 100, EE_CHAR_FONTHEIGHT_CTL ) );

    // copy other items from default font
    const SfxPoolItem& rWeightItem = aDefSet.Get( EE_CHAR_WEIGHT );
    aDefSet.Put( rWeightItem, EE_CHAR_WEIGHT_CJK );
    aDefSet.Put( rWeightItem, EE_CHAR_WEIGHT_CTL );
    const SfxPoolItem& rItalicItem = aDefSet.Get( EE_CHAR_ITALIC );
    aDefSet.Put( rItalicItem, EE_CHAR_ITALIC_CJK );
    aDefSet.Put( rItalicItem, EE_CHAR_ITALIC_CTL );
    const SfxPoolItem& rLangItem = aDefSet.Get( EE_CHAR_LANGUAGE );
    aDefSet.Put( rLangItem, EE_CHAR_LANGUAGE_CJK );
    aDefSet.Put( rLangItem, EE_CHAR_LANGUAGE_CTL );

    mpEditEngine->SetDefaults( aDefSet );
    InvalidateGfx();
}

void ScCsvGrid::InitSizeData()
{
    maWinSize = GetSizePixel();
    maBackgrDev.SetOutputSizePixel( maWinSize );
    maGridDev.SetOutputSizePixel( maWinSize );
    InvalidateGfx();
}


// grid handling --------------------------------------------------------------

void ScCsvGrid::UpdateLayoutData()
{
    DisableRepaint();
    SetFont( maMonoFont );
    CommitRequest( CSVREQ_CHARWIDTH, GetTextWidth( String( 'X' ) ) );
    CommitRequest( CSVREQ_LINEHEIGHT, GetTextHeight() + 1 );
    SetFont( maHeaderFont );
    CommitRequest( CSVREQ_OFFSETY, GetTextHeight() + 1 );
    UpdateOffsetX();
    EnableRepaint();
}

void ScCsvGrid::UpdateOffsetX()
{
    sal_Int32 nLastLine = GetLastVisLine() + 1;
    sal_Int32 nDigits = 2;
    while( nLastLine /= 10 ) ++nDigits;
    nDigits = Max( nDigits, 3L );
    CommitRequest( CSVREQ_OFFSETX, GetTextWidth( String( '0' ) ) * nDigits );
}

void ScCsvGrid::ApplyLayout( const ScCsvLayoutData& rOldData )
{
    DisableRepaint();
    sal_uInt32 nDiff = GetLayoutData().GetDiff( rOldData );

    if( nDiff & CSV_DIFF_RULERCURSOR )
    {
        ImplEraseCursor( rOldData.mnPosCursor );
        ImplDrawCursor( GetRulerCursorPos() );
    }

    if( nDiff & CSV_DIFF_POSCOUNT )
    {
        if( GetPosCount() < rOldData.mnPosCount )
        {
            ImplClearSelection();
            CommitEvent( GRIDEVENT_SELECTION );
            maSplits.RemoveRange( GetPosCount(), rOldData.mnPosCount );
        }
        else
            maSplits.Remove( rOldData.mnPosCount );
        maSplits.Insert( GetPosCount() );
        maColFlags.resize( maSplits.Count() - 1, CSV_TYPE_DEFAULT );
        maColTypes.resize( maSplits.Count() - 1, CSV_COLFLAG_NONE );
    }

    if( nDiff & CSV_DIFF_LINEOFFSET )
    {
        CommitRequest( CSVREQ_UPDATECELLTEXTS );
        UpdateOffsetX();
    }

    if( nDiff & CSV_DIFF_POSOFFSET )
        ImplDrawHorzScrolled( rOldData.mnPosOffset );

    if( nDiff & ((CSV_DIFF_HORIZONTAL | CSV_DIFF_VERTICAL) & ~CSV_DIFF_POSOFFSET) )
        InvalidateGfx();

    EnableRepaint();
}

sal_Int32 ScCsvGrid::GetNoScrollCol( sal_Int32 nPos ) const
{
    sal_Int32 nNewPos = nPos;
    if( nNewPos != POS_INVALID )
    {
        if( nNewPos < GetFirstVisPos() + SCROLL_DIST )
        {
            sal_Int32 nScroll = (GetFirstVisPos() > 0) ? SCROLL_DIST : 0;
            nNewPos = GetFirstVisPos() + nScroll;
        }
        else if( nNewPos > GetLastVisPos() - SCROLL_DIST - 1L )
        {
            sal_Int32 nScroll = (GetFirstVisPos() < GetMaxPosOffset()) ? SCROLL_DIST : 0;
            nNewPos = GetLastVisPos() - nScroll - 1;
        }
    }
    return nNewPos;
}

bool ScCsvGrid::ImplInsertSplit( sal_Int32 nPos )
{
    sal_uInt32 nColIx = GetColumnFromPos( nPos );
    bool bRet = maSplits.Insert( nPos );
    if( bRet )
    {
        maColTypes.insert( maColTypes.begin() + nColIx, GetColumnType( nColIx ) );
        maColFlags.insert( maColFlags.begin() + nColIx, maColFlags[ nColIx ] );
    }
    return bRet;
}

bool ScCsvGrid::ImplRemoveSplit( sal_Int32 nPos )
{
    bool bRet = maSplits.Remove( nPos );
    if( bRet )
    {
        sal_uInt32 nColIx = GetColumnFromPos( nPos );
        bool bFirstSel = IsSelected( nColIx );
        bool bSecondSel = IsSelected( nColIx + 1 );
        sal_uInt32 nDelColIx = (bFirstSel || !bSecondSel) ? nColIx + 1 : nColIx;
        maColTypes.erase( maColTypes.begin() + nColIx + 1 );
        maColFlags.erase( maColFlags.begin() + nDelColIx );
    }
    return bRet;
}

void ScCsvGrid::ImplClearSplits()
{
    maSplits.Clear();
    maSplits.Insert( 0 );
    maSplits.Insert( GetPosCount() );
    maColFlags.resize( 1 );
    maColFlags[ 0 ] = CSV_TYPE_DEFAULT;
    maColTypes.resize( 1 );
    maColTypes[ 0 ] = CSV_COLFLAG_NONE;
    InvalidateGfx();
}

void ScCsvGrid::InsertSplit( sal_Int32 nPos )
{
    if( ImplInsertSplit( nPos ) )
    {
        DisableRepaint();
        CommitRequest( CSVREQ_UPDATECELLTEXTS );
        sal_uInt32 nColIx = GetColumnFromPos( nPos );
        ImplDrawColumn( nColIx - 1 );
        ImplDrawColumn( nColIx );
        ValidateGfx();  // performance: do not redraw all columns
        EnableRepaint();
        // new column is equal to old -> no selection event
    }
}

void ScCsvGrid::RemoveSplit( sal_Int32 nPos )
{
    if( ImplRemoveSplit( nPos ) )
    {
        DisableRepaint();
        CommitRequest( CSVREQ_UPDATECELLTEXTS );
        CommitEvent( GRIDEVENT_SELECTION );
        ImplDrawColumn( GetColumnFromPos( nPos ) );
        ValidateGfx();  // performance: do not redraw all columns
        EnableRepaint();
    }
}

void ScCsvGrid::MoveSplit( sal_Int32 nPos, sal_Int32 nNewPos )
{
    sal_uInt32 nColIx = GetColumnFromPos( nPos );
    if( nColIx != VEC_NOTFOUND )
    {
        DisableRepaint();
        if( (GetColumnPos( nColIx - 1 ) < nNewPos) && (nNewPos < GetColumnPos( nColIx + 1 )) )
        {
            // move a split in the range between 2 others -> keep selection state of both columns
            maSplits.Remove( nPos );
            maSplits.Insert( nNewPos );
            CommitRequest( CSVREQ_UPDATECELLTEXTS );
            ImplDrawColumn( nColIx - 1 );
            ImplDrawColumn( nColIx );
            ValidateGfx();  // performance: do not redraw all columns
        }
        else
        {
            ImplRemoveSplit( nPos );
            ImplInsertSplit( nNewPos );
            CommitEvent( GRIDEVENT_SELECTION );
            CommitRequest( CSVREQ_UPDATECELLTEXTS );
        }
        EnableRepaint();
    }
}

void ScCsvGrid::RemoveAllSplits()
{
    DisableRepaint();
    ImplClearSplits();
    CommitRequest( CSVREQ_UPDATECELLTEXTS );
    CommitEvent( GRIDEVENT_SELECTION );
    EnableRepaint();
}

void ScCsvGrid::SetSplits( const ScCsvSplits& rSplits )
{
    DisableRepaint();
    ImplClearSplits();
    sal_uInt32 nCount = rSplits.Count();
    for( sal_uInt32 nIx = 0; nIx < nCount; ++nIx )
        maSplits.Insert( rSplits[ nIx ] );
    maColTypes.clear();
    maColTypes.resize( maSplits.Count() - 1, CSV_TYPE_DEFAULT );
    maColFlags.clear();
    maColFlags.resize( maSplits.Count() - 1, CSV_COLFLAG_NONE );
    CommitRequest( CSVREQ_UPDATECELLTEXTS );
    CommitEvent( GRIDEVENT_SELECTION );
    EnableRepaint();
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
    }
    CommitRequest( CSVREQ_LINEOFFSET, nLine );
}

void ScCsvGrid::ExecutePopup( const Point& rPos )
{
    sal_uInt16 nItemId = maPopup.Execute( this, rPos );
    if( nItemId )   // 0 = cancelled
        CommitRequest( CSVREQ_COLUMNTYPE, maPopup.GetItemPos( nItemId ) );
}


// column data ----------------------------------------------------------------

sal_Int32 ScCsvGrid::GetColumnType( sal_uInt32 nColIndex ) const
{
    return IsValidColumn( nColIndex ) ? maColTypes[ nColIndex ] : CSV_TYPE_NOSELECTION;
}

void ScCsvGrid::SetColumnType( sal_uInt32 nColIndex, sal_Int32 nColType )
{
    if( IsValidColumn( nColIndex ) )
        maColTypes[ nColIndex ] = nColType;
}

const String& ScCsvGrid::GetColumnTypeName( sal_uInt32 nColIndex ) const
{
    sal_uInt32 nTypeIx = static_cast< sal_uInt32 >( GetColumnType( nColIndex ) );
    return (nTypeIx < maTypeNames.size()) ? maTypeNames[ nTypeIx ] : EMPTY_STRING;
}

void ScCsvGrid::SetColumnTypes( const ScCsvColTypeVec& rColTypes )
{
    maColTypes = rColTypes;
    maColTypes.resize( maSplits.Count() - 1, CSV_TYPE_DEFAULT );
}

sal_Int32 ScCsvGrid::GetSelColumnType() const
{
    sal_uInt32 nColIx = GetFirstSelected();
    if( nColIx == VEC_NOTFOUND )
        return CSV_TYPE_NOSELECTION;

    sal_Int32 nType = GetColumnType( nColIx );
    while( (nColIx != VEC_NOTFOUND) && (nType != CSV_TYPE_MULTI) )
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
        for( sal_uInt32 nColIx = GetFirstSelected(); nColIx != VEC_NOTFOUND; nColIx = GetNextSelected( nColIx ) )
            SetColumnType( nColIx, nType );
        Repaint( true );
        CommitEvent( GRIDEVENT_COLUMNTYPE, nType );
    }
}

void ScCsvGrid::SetTypeNames( const ScCsvStringVec& rTypeNames )
{
    DBG_ASSERT( !rTypeNames.empty(), "ScCsvGrid::SetTypeNames - vector is empty" );
    maTypeNames = rTypeNames;
    Repaint( true );

    maPopup.Clear();
    sal_uInt32 nCount = maTypeNames.size();
    sal_uInt32 nIx;
    sal_uInt16 nItemId;
    for( nIx = 0, nItemId = 1; nIx < nCount; ++nIx, ++nItemId )
        maPopup.InsertItem( nItemId, maTypeNames[ nIx ] );

    ScCsvColTypeVec::iterator aEnd = maColTypes.end();
    for( ScCsvColTypeVec::iterator aIter = maColTypes.begin(); aIter != aEnd; ++aIter )
        *aIter = CSV_TYPE_DEFAULT;
}

void ScCsvGrid::ImplSetTextLineSep(
        sal_Int32 nLine, const String& rTextLine,
        const String& rSepChars, sal_Unicode cTextSep, bool bMergeSep )
{
    sal_uInt32 nLineIx = nLine - GetFirstVisLine();
    while( maTexts.size() <= nLineIx )
        maTexts.push_back( ScCsvStringVec() );
    ScCsvStringVec& rStrVec = maTexts[ nLineIx ];
    rStrVec.clear();

    // scan for separators
    String aCellText;
    const sal_Unicode* pSepChars = rSepChars.GetBuffer();
    const sal_Unicode* pChar = rTextLine.GetBuffer();
    sal_uInt32 nColIx = 0;

    while( *pChar && (nColIx <= MAXCOL) )
    {
        // scan for next cell text
        pChar = ScImportExport::ScanNextFieldFromString( pChar, aCellText, cTextSep, pSepChars, bMergeSep );

        // update column width
        sal_Int32 nWidth = Max( CSV_MINCOLWIDTH, aCellText.Len() + 1L );
        if( IsValidColumn( nColIx ) )
        {
            // expand existing column
            sal_Int32 nDiff = nWidth - GetColumnWidth( nColIx );
            if( nDiff > 0 )
            {
                CommitRequest( CSVREQ_POSCOUNT, GetPosCount() + nDiff );
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
            CommitRequest( CSVREQ_POSCOUNT, nLastPos + nWidth );
            ImplInsertSplit( nLastPos );
        }

        if( aCellText.Len() <= CSV_MAXSTRLEN )
            rStrVec.push_back( aCellText );
        else
            rStrVec.push_back( aCellText.Copy( 0, CSV_MAXSTRLEN ) );
        ++nColIx;
    }
    InvalidateGfx();
}

void ScCsvGrid::ImplSetTextLineFix( sal_Int32 nLine, const String& rTextLine )
{
    sal_Int32 nChars = rTextLine.Len();
    if( nChars > GetPosCount() )
        CommitRequest( CSVREQ_POSCOUNT, nChars );

    sal_uInt32 nLineIx = nLine - GetFirstVisLine();
    while( maTexts.size() <= nLineIx )
        maTexts.push_back( ScCsvStringVec() );

    ScCsvStringVec& rStrVec = maTexts[ nLineIx ];
    rStrVec.clear();
    sal_uInt32 nColCount = GetColumnCount();
    xub_StrLen nStrLen = rTextLine.Len();
    xub_StrLen nStrIx = 0;
    for( sal_uInt32 nColIx = 0; (nColIx < nColCount) && (nStrIx < nStrLen); ++nColIx )
    {
        xub_StrLen nChars = static_cast< xub_StrLen >( GetColumnWidth( nColIx ) );
        rStrVec.push_back( rTextLine.Copy( nStrIx, Max( nChars, CSV_MAXSTRLEN ) ) );
        nStrIx += nChars;
    }
    InvalidateGfx();
}

sal_uInt8 lcl_GetExtColumnType( sal_Int32 nIntType )
{
    static sal_uInt8 pExtTypes[] =
        { SC_COL_STANDARD, SC_COL_TEXT, SC_COL_DMY, SC_COL_MDY, SC_COL_YMD, SC_COL_ENGLISH, SC_COL_SKIP };
    static sal_Int32 nExtTypeCount = sizeof( pExtTypes ) / sizeof( *pExtTypes );
    return pExtTypes[ ((0 <= nIntType) && (nIntType < nExtTypeCount)) ? nIntType : 0 ];
}

void ScCsvGrid::FillColumnDataSep( ScAsciiOptions& rOptions ) const
{
    ScCsvExtColPosVec aColIndexVec;
    ScCsvExtColTypeVec aColTypeVec;
    sal_uInt32 nCount = GetColumnCount();

    for( sal_uInt32 nColIx = 0; nColIx < nCount; ++nColIx )
    {
        if( GetColumnType( nColIx ) != CSV_TYPE_DEFAULT )
        {
            // 1-based column index
            aColIndexVec.push_back( static_cast< xub_StrLen >( nColIx + 1 ) );
            aColTypeVec.push_back( lcl_GetExtColumnType( GetColumnType( nColIx ) ) );
        }
    }
    rOptions.SetColumnInfo( aColIndexVec, aColTypeVec );
}

void ScCsvGrid::FillColumnDataFix( ScAsciiOptions& rOptions ) const
{
    sal_uInt32 nCount = Min( GetColumnCount(), MAXCOL + 1UL );
    ScCsvExtColPosVec aColPosVec( nCount + 1 );
    ScCsvExtColTypeVec aColTypeVec( nCount + 1 );

    for( sal_uInt32 nColIx = 0; nColIx < nCount; ++nColIx )
    {
        aColPosVec[ nColIx ] = static_cast< xub_StrLen >(
            Min( static_cast< sal_Int32 >( STRING_MAXLEN ), GetColumnPos( nColIx ) ) );
        aColTypeVec[ nColIx ] = lcl_GetExtColumnType( GetColumnType( nColIx ) );
    }
    aColPosVec[ nCount ] = STRING_MAXLEN;
    aColTypeVec[ nCount ] = SC_COL_SKIP;
    rOptions.SetColumnInfo( aColPosVec, aColTypeVec );
}


// column position ------------------------------------------------------------

sal_uInt32 ScCsvGrid::GetColumnFromPos( sal_Int32 nPos ) const
{
    return maSplits.UpperBound( nPos );
}

sal_Int32 ScCsvGrid::GetColumnWidth( sal_uInt32 nColIndex ) const
{
    return IsValidColumn( nColIndex ) ? (GetColumnPos( nColIndex + 1 ) - GetColumnPos( nColIndex )) : 0;
}

sal_uInt32 ScCsvGrid::GetFirstVisColumn() const
{
    return GetColumnFromPos( GetFirstVisPos() );
}

sal_uInt32 ScCsvGrid::GetLastVisColumn() const
{
    return GetColumnFromPos( Min( GetLastVisPos(), GetPosCount() ) - 1 );
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
    sal_Int32 nPos = (nX - GetOffsetX()) / GetCharWidth() + GetFirstVisPos();
    return (nPos >= GetFirstVisPos()) ? GetColumnFromPos( nPos ) : VEC_NOTFOUND;
}


// selection handling ---------------------------------------------------------

void ScCsvGrid::MoveCursor( sal_uInt32 nColIndex )
{
    DisableRepaint();
    if( IsValidColumn( nColIndex ) )
    {
        sal_Int32 nPosBeg = GetColumnPos( nColIndex );
        sal_Int32 nPosEnd = GetColumnPos( nColIndex + 1 );
        sal_Int32 nMinPos = Max( nPosBeg - SCROLL_DIST, 0L );
        sal_Int32 nMaxPos = Min( nPosEnd - GetVisPosCount() + SCROLL_DIST + 1L, nMinPos );
        if( nPosBeg - SCROLL_DIST + 1 <= GetFirstVisPos() )
            CommitRequest( CSVREQ_POSOFFSET, nMinPos );
        else if( nPosEnd + SCROLL_DIST >= GetLastVisPos() )
            CommitRequest( CSVREQ_POSOFFSET, nMaxPos );
    }
    CommitRequest( CSVREQ_MOVEGRIDCURSOR, GetColumnPos( nColIndex ) );
    EnableRepaint();
}

void ScCsvGrid::MoveCursorRel( ScMoveMode eDir )
{
    if( GetFocusColumn() != VEC_NOTFOUND )
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
        }
    }
}

void ScCsvGrid::ImplClearSelection()
{
    sal_uInt32 nSize = maColFlags.size();
    for( sal_uInt32 nColIx = 0; nColIx < nSize; ++nColIx )
        set_bits( maColFlags[ nColIx ], CSV_COLFLAG_SELECT, false );
    ImplDrawGridDev();
}

bool ScCsvGrid::IsSelected( sal_uInt32 nColIndex ) const
{
    return IsValidColumn( nColIndex ) && (maColFlags[ nColIndex ] & CSV_COLFLAG_SELECT);
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
    return VEC_NOTFOUND;
}

void ScCsvGrid::Select( sal_uInt32 nColIndex, bool bSelect )
{
    if( IsValidColumn( nColIndex ) )
    {
        set_bits( maColFlags[ nColIndex ], CSV_COLFLAG_SELECT, bSelect );
        ImplDrawColumnSelection( nColIndex );
        Repaint();
        CommitEvent( GRIDEVENT_SELECTION );
        if( bSelect )
            mnRecentSelCol = nColIndex;
    }
}

void ScCsvGrid::ToggleSelect( sal_uInt32 nColIndex )
{
    Select( nColIndex, !IsSelected( nColIndex ) );
}

void ScCsvGrid::SelectRange( sal_uInt32 nColIndex1, sal_uInt32 nColIndex2, bool bSelect )
{
    if( nColIndex1 == VEC_NOTFOUND )
        Select( nColIndex2 );
    else if( nColIndex2 == VEC_NOTFOUND )
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
            set_bits( maColFlags[ nColIx ], CSV_COLFLAG_SELECT, bSelect );
            ImplDrawColumnSelection( nColIx );
        }
        Repaint();
        CommitEvent( GRIDEVENT_SELECTION );
        if( bSelect )
            mnRecentSelCol = nColIndex1;
    }
}

void ScCsvGrid::SelectAll()
{
    SelectRange( 0, maColFlags.size() - 1 );
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
    CommitRequest( CSVREQ_MOVEGRIDCURSOR, GetColumnPos( nColIndex ) );
}


// event handling -------------------------------------------------------------

void ScCsvGrid::Resize()
{
    ScCsvControl::Resize();
    InitSizeData();
    CommitRequest( CSVREQ_UPDATECELLTEXTS );
}

void ScCsvGrid::GetFocus()
{
    ScCsvControl::GetFocus();
    CommitRequest( CSVREQ_MOVEGRIDCURSOR, GetNoScrollCol( GetGridCursorPos() ) );
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
        if( aPos.X() < GetOffsetX() )
        {
            if( aPos.Y() <= GetOffsetY() )
                SelectAll();
        }
        else if( IsValidColumn( nColIx ) )
        {
            DoSelectAction( nColIx, rMEvt.GetModifier() );
            mnMTCurrCol = nColIx;
            mbMTSelecting = IsSelected( nColIx );
            StartTracking( STARTTRACK_BUTTONREPEAT );
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

        sal_Int32 nPos = GetPosFromX( rMEvt.GetPosPixel().X() );
        // on mouse tracking: keep position valid
        nPos = Max( Min( nPos, GetPosCount() - 1L ), 1L );
        CommitRequest( CSVREQ_MAKEPOSVISIBLE, nPos );

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
    const KeyCode& rKCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = rKCode.GetCode();
    bool bShift = rKCode.IsShift() == TRUE;
    bool bMod1 = rKCode.IsMod1() == TRUE;

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
                sal_uInt16 nType = nCode - KEY_1;
                if( nType < maTypeNames.size() )
                    CommitRequest( CSVREQ_COLUMNTYPE, nType );
            }
        }
        else if( (nCode == KEY_CONTEXTMENU) && !rKCode.GetModifier() )
        {
            if( !HasSelection() )
                Select( GetFocusColumn() );
            ExecutePopup( Point( GetColumnX( GetFocusColumn() ), GetHeight() / 2 ) );
        }
    }

    if( (rKCode.GetGroup() != KEYGROUP_CURSOR) && (nCode != KEY_CONTEXTMENU) )
        ScCsvControl::KeyInput( rKEvt );
}

void ScCsvGrid::Command( const CommandEvent& rCEvt )
{
    switch( rCEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
            Point aPos( rCEvt.GetMousePosPixel() );
            sal_uInt32 nColIx = GetColumnFromX( aPos.X() );
            if( IsValidColumn( nColIx ) && !IsSelected( nColIx ) )
                MouseButtonDown( MouseEvent( aPos, 1, MOUSE_SIMPLECLICK, MOUSE_LEFT ) );
            if( HasSelection() )
                ExecutePopup( aPos );
        }
        break;
        case COMMAND_WHEEL:
        {
            Point aPoint;
            Rectangle aRect( aPoint, maWinSize );
            if( aRect.IsInside( rCEvt.GetMousePosPixel() ) )
            {
                const CommandWheelData* pData = rCEvt.GetWheelData();
                if( pData && (pData->GetMode() == COMMAND_WHEEL_SCROLL) && !pData->IsHorz() )
                    CommitRequest( CSVREQ_LINEOFFSET, GetFirstVisLine() - pData->GetNotchDelta() );
            }
        }
        break;
        default:
            ScCsvControl::Command( rCEvt );
    }
}

void ScCsvGrid::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        InitColors();
        InitFonts();
        UpdateLayoutData();
        CommitRequest( CSVREQ_UPDATECELLTEXTS );
    }
    ScCsvControl::DataChanged( rDCEvt );
}

void ScCsvGrid::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( rHint.ISA( SfxSimpleHint ) &&
        (static_cast< const SfxSimpleHint& >( rHint ).GetId() == SFX_HINT_COLORS_CHANGED) )
    {
        InitColors();
        Repaint();
    }
}


// painting -------------------------------------------------------------------

void ScCsvGrid::Paint( const Rectangle& )
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
        DrawOutDev( Point(), maWinSize, Point(), maWinSize, maGridDev );
        ImplDrawTrackingRect( GetFocusColumn() );
    }
}

void ScCsvGrid::ImplSetColumnClipRegion( OutputDevice& rOutDev, sal_uInt32 nColIndex )
{
    rOutDev.SetClipRegion( Region( Rectangle(
        Max( GetColumnX( nColIndex ), GetOffsetX() ) + 1, 0,
        GetColumnX( nColIndex + 1 ), GetHeight() - 1 ) ) );
}

void ScCsvGrid::ImplDrawColumnHeader( OutputDevice& rOutDev, sal_uInt32 nColIndex, Color aFillColor )
{
    sal_Int32 nX1 = GetColumnX( nColIndex ) + 1;
    sal_Int32 nX2 = GetColumnX( nColIndex + 1 );

    rOutDev.SetLineColor();
    rOutDev.SetFillColor( aFillColor );
    rOutDev.DrawRect( Rectangle( nX1, 0, nX2, GetOffsetY() ) );

    rOutDev.SetFont( maHeaderFont );
    rOutDev.SetTextColor( maTextColor );
    rOutDev.SetTextFillColor();
    rOutDev.DrawText( Point( nX1 + 1, 0 ), GetColumnTypeName( nColIndex ) );

    rOutDev.SetLineColor( maHeaderGridColor );
    rOutDev.DrawLine( Point( nX1, GetOffsetY() ), Point( nX2, GetOffsetY() ) );
    rOutDev.DrawLine( Point( nX2, 0 ), Point( nX2, GetOffsetY() ) );
}

void ScCsvGrid::ImplDrawColumnBackgr( sal_uInt32 nColIndex )
{
    if( !IsVisibleColumn( nColIndex ) )
        return;

    ImplSetColumnClipRegion( maBackgrDev, nColIndex );

    sal_Int32 nX1 = GetColumnX( nColIndex ) + 1;
    sal_Int32 nX2 = GetColumnX( nColIndex + 1 );
    sal_Int32 nY2 = GetY( GetLastVisLine() + 1 );

    // data cells
    maBackgrDev.SetLineColor();
    maBackgrDev.SetFillColor( maBackColor );
    Rectangle aRect( nX1, GetOffsetY(), nX2, nY2 );
    maBackgrDev.DrawRect( aRect );

    // cell texts
    mpEditEngine->SetDefaultItem( SvxColorItem( maTextColor, EE_CHAR_COLOR ) );
    sal_Int32 nLineCount = Min( GetLastVisLine() - GetFirstVisLine() + 1UL, maTexts.size() );
    for( sal_Int32 nLine = 0; nLine < nLineCount; ++nLine )
    {
        ScCsvStringVec& rStrVec = maTexts[ nLine ];
        if( nColIndex < rStrVec.size() )
        {
            mpEditEngine->SetText( rStrVec[ nColIndex ] );
            mpEditEngine->Draw( &maBackgrDev, Point( nX1, GetY( GetFirstVisLine() + nLine ) ) );
        }
    }

    maBackgrDev.SetLineColor( maGridColor );
    maBackgrDev.DrawGrid( aRect, Size( 1, GetLineHeight() ), GRID_HORZLINES );
    maBackgrDev.DrawLine( Point( nX2, GetOffsetY() ), Point( nX2, nY2 ) );

    // header
    ImplDrawColumnHeader( maBackgrDev, nColIndex, maHeaderBackColor );

    maBackgrDev.SetClipRegion();
}

void ScCsvGrid::ImplDrawRowHeaders()
{
    maBackgrDev.SetLineColor();
    maBackgrDev.SetFillColor( maAppBackColor );
    Rectangle aRect( 0, 0, GetOffsetX(), GetHeight() - 1 );
    maBackgrDev.DrawRect( aRect );

    maBackgrDev.SetFillColor( maHeaderBackColor );
    aRect.Bottom() = GetY( GetLastVisLine() + 1 );
    maBackgrDev.DrawRect( aRect );

    // line numbers
    maBackgrDev.SetFont( maHeaderFont );
    maBackgrDev.SetTextColor( maTextColor );
    maBackgrDev.SetTextFillColor();
    sal_Int32 nLastLine = GetLastVisLine();
    for( sal_Int32 nLine = GetFirstVisLine(); nLine <= nLastLine; ++nLine )
    {
        String aText( String::CreateFromInt32( nLine + 1 ) );
        sal_Int32 nX = (GetOffsetX() - maBackgrDev.GetTextWidth( aText )) / 2;
        maBackgrDev.DrawText( Point( nX, GetY( nLine ) + 1 ), aText );
    }

    // grid
    maBackgrDev.SetLineColor( maHeaderGridColor );
    maBackgrDev.DrawLine( aRect.TopRight(), aRect.BottomRight() );
    aRect.Top() = GetOffsetY();
    maBackgrDev.DrawGrid( aRect, Size( 1, GetLineHeight() ), GRID_HORZLINES );
}

void ScCsvGrid::ImplDrawBackgrDev()
{
    maBackgrDev.SetLineColor();
    maBackgrDev.SetFillColor( maAppBackColor );
    maBackgrDev.DrawRect( Rectangle( Point( GetOffsetX() + 1, 0 ), maWinSize ) );

    sal_uInt32 nLastCol = GetLastVisColumn();
    for( sal_uInt32 nColIx = GetFirstVisColumn(); nColIx <= nLastCol; ++nColIx )
        ImplDrawColumnBackgr( nColIx );

    ImplDrawRowHeaders();
}

void ScCsvGrid::ImplDrawColumnSelection( sal_uInt32 nColIndex )
{
    ImplEraseCursor( GetRulerCursorPos() );
    ImplSetColumnClipRegion( maGridDev, nColIndex );
    maGridDev.DrawOutDev( Point(), maWinSize, Point(), maWinSize, maBackgrDev );

    if( IsSelected( nColIndex ) )
    {
        sal_Int32 nX1 = GetColumnX( nColIndex ) + 1;
        sal_Int32 nX2 = GetColumnX( nColIndex + 1 );

        // header
        Rectangle aRect( nX1, 0, nX2, GetOffsetY() );
        maGridDev.SetLineColor();
        if( maHeaderBackColor.IsDark() )
            // redraw with light gray background in dark mode
            ImplDrawColumnHeader( maGridDev, nColIndex, COL_LIGHTGRAY );
        else
        {
            // use transparent active color
            maGridDev.SetFillColor( maSelectColor );
            maGridDev.DrawTransparent( PolyPolygon( Polygon( aRect ) ), CSV_HDR_TRANSPARENCY );
        }

        // column selection
        aRect = Rectangle( nX1, GetOffsetY() + 1, nX2, GetY( GetLastVisLine() + 1 ) - 1 );
        ImplInvertRect( maGridDev, aRect );
    }

    maGridDev.SetClipRegion();
    ImplDrawCursor( GetRulerCursorPos() );
}

void ScCsvGrid::ImplDrawGridDev()
{
    maGridDev.DrawOutDev( Point(), maWinSize, Point(), maWinSize, maBackgrDev );
    sal_uInt32 nLastCol = GetLastVisColumn();
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
    if( Abs( nPos - nOldPos ) > GetVisPosCount() / 2 )
        InvalidateGfx();
    if( !IsValidGfx() || (nPos == nOldPos) )
        return;

    Point aSrc, aDest;
    sal_uInt32 nFirstColIx, nLastColIx;
    if( nPos < nOldPos )
    {
        aSrc = Point( GetOffsetX() + 1, 0 );
        aDest = Point( GetOffsetX() + GetCharWidth() * (nOldPos - nPos) + 1, 0 );
        nFirstColIx = GetColumnFromPos( nPos );
        nLastColIx = GetColumnFromPos( nOldPos );
    }
    else
    {
        aSrc = Point( GetOffsetX() + GetCharWidth() * (nPos - nOldPos) + 1, 0 );
        aDest = Point( GetOffsetX() + 1, 0 );
        nFirstColIx = GetColumnFromPos( Min( nOldPos + GetVisPosCount(), GetPosCount() ) - 1 );
        nLastColIx = GetColumnFromPos( Min( nPos + GetVisPosCount(), GetPosCount() ) - 1 );
    }

    ImplEraseCursor( GetRulerCursorPos() + (nPos - nOldPos) );
    maBackgrDev.CopyArea( aDest, aSrc, maWinSize );
    maGridDev.CopyArea( aDest, aSrc, maWinSize );
    ImplDrawCursor( GetRulerCursorPos() );
    for( sal_uInt32 nColIx = nFirstColIx; nColIx <= nLastColIx; ++nColIx )
        ImplDrawColumn( nColIx );

    sal_Int32 nLastX = GetX( GetPosCount() ) + 1;
    if( nLastX < GetWidth() )
    {
        Rectangle aRect( nLastX, 0, GetWidth() - 1, GetHeight() - 1 );
        maBackgrDev.SetLineColor();
        maBackgrDev.SetFillColor( maAppBackColor );
        maBackgrDev.DrawRect( aRect );
        maGridDev.SetLineColor();
        maGridDev.SetFillColor( maAppBackColor );
        maGridDev.DrawRect( aRect );
    }
}

void ScCsvGrid::ImplDrawCursor( sal_Int32 nPos )
{
    if( IsVisibleSplitPos( nPos ) )
    {
        sal_Int32 nX = GetX( nPos ) - 1;
        Rectangle aRect( Point( nX, 0 ), Size( 3, GetOffsetY() ) );
        ImplInvertRect( maGridDev, aRect );
        aRect.Top() = GetOffsetY() + 1;
        aRect.Bottom() = GetY( GetLastVisLine() + 1 );
        ImplInvertRect( maGridDev, aRect );
    }
}

void ScCsvGrid::ImplDrawTrackingRect( sal_uInt32 nColIndex )
{
    if( HasFocus() && IsVisibleColumn( nColIndex ) )
    {
        sal_Int32 nX1 = Max( GetColumnX( nColIndex ), GetOffsetX() ) + 1;
        sal_Int32 nX2 = Min( GetColumnX( nColIndex + 1 ), GetWidth() ) - 1;
        sal_Int32 nY2 = Min( GetY( GetLastVisLine() + 1 ), GetHeight() ) - 1;
        InvertTracking( Rectangle( nX1, 0, nX2, nY2 ), SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
    }
}


// ============================================================================

