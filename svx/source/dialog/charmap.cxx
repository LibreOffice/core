/*************************************************************************
 *
 *  $RCSfile: charmap.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hdu $ $Date: 2000-11-22 10:27:56 $
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

// include ---------------------------------------------------------------

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SOUND_HXX
#include <vcl/sound.hxx>
#endif
#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#pragma hdrstop

#include <rtl/textenc.h>
#include <ucsubset.hxx>

#include "dialogs.hrc"
#include "charmap.hrc"

#include "charmap.hxx"
#include "dialmgr.hxx"

// class SvxShowCharSet --------------------------------------------------

long SvxShowCharSet::nSelectedIndex = 0;

#define SBWIDTH 16

SvxShowCharSet::SvxShowCharSet( Window* pParent, const ResId& rResId ) :
    Control( pParent, rResId ),
    aVscrollSB( this, WB_VERT)
{
    if( nSelectedIndex > LastInMap() )
        nSelectedIndex = LastInMap();
    if( nSelectedIndex < FirstInMap() )
        nSelectedIndex = FirstInMap();

    aOrigSize = GetOutputSizePixel();
    aOrigPos = GetPosPixel();

    SetStyle( GetStyle() | WB_CLIPCHILDREN );
    aVscrollSB.SetScrollHdl( LINK( this, SvxShowCharSet, VscrollHdl ) );
    aVscrollSB.EnableDrag( TRUE);
    // other aVscroll settings depend on selected font => see SetFont

    bDrag = FALSE;
    InitSettings( TRUE, TRUE );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::GetFocus()
{
    Control::GetFocus();
    SelectIndex( nSelectedIndex, TRUE );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::LoseFocus()
{
    Control::LoseFocus();
    SelectIndex( nSelectedIndex, TRUE );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
        InitSettings( TRUE, FALSE );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        InitSettings( FALSE, TRUE );

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS )
      && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings( TRUE, TRUE );
    else
        Control::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.GetClicks() == 1 )
        {
            GrabFocus();
            bDrag = TRUE;
            CaptureMouse();

            int index = PixelToMapIndex( rMEvt.GetPosPixel());
            SelectIndex( index);
        }

        if ( !(rMEvt.GetClicks() % 2) )
            aDoubleClkHdl.Call( this );
    }
}

// -----------------------------------------------------------------------

void SvxShowCharSet::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( bDrag && rMEvt.IsLeft() )
    {
        // released mouse over character map
        if ( Rectangle(Point(), GetOutputSize()).IsInside(rMEvt.GetPosPixel()))
            aSelectHdl.Call( this );
        ReleaseMouse();
        bDrag = FALSE;
    }
}

// -----------------------------------------------------------------------

void SvxShowCharSet::MouseMove( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && bDrag )
    {
        Point aPos  = rMEvt.GetPosPixel();
        Size  aSize = GetSizePixel();

        if ( aPos.X() < 0 )
            aPos.X() = 0;
        else if ( aPos.X() > aSize.Width()-5 )
            aPos.X() = aSize.Width()-5;
        if ( aPos.Y() < 0 )
            aPos.Y() = 0;
        else if ( aPos.Y() > aSize.Height()-5 )
            aPos.Y() = aSize.Height()-5;

        int index = PixelToMapIndex( aPos);
        SelectIndex( index );
    }
}

// -----------------------------------------------------------------------

void SvxShowCharSet::Command( const CommandEvent& rCEvt )
{
    if( !HandleScrollCommand( rCEvt, 0, &aVscrollSB ) )
        Control::Command( rCEvt );
}

// -----------------------------------------------------------------------
// TODO: LastInMap(), UnicodeToMapIndex() and MapIndexToUnicode() should be in Font,
// we let them mature here though because it is currently the only use

#define FIRST_UNICODE   sal_Unicode(0x0020)     // ASCII space code
#define LAST_UNICODE    sal_Unicode(0xFFFF)     // end of UCS2
#define FIRST_SURROGATE sal_Unicode(0xD800)
#define LAST_SURROGATE  sal_Unicode(0xDFFF)
#define COUNT_SURROGATE (LAST_SURROGATE - FIRST_SURROGATE + 1);

int SvxShowCharSet::FirstInMap( void) const
{
    return 0;
}

inline int SvxShowCharSet::LastInMap( void) const
{
    if( GetFont().GetCharSet() == RTL_TEXTENCODING_SYMBOL)
        return (0x0FF - 0x020);
    return (LAST_UNICODE - FIRST_UNICODE) - COUNT_SURROGATE;
}

inline int SvxShowCharSet::UnicodeToMapIndex( sal_Unicode c) const
{
    //TODO: should depend on font and encoding, change when Font can provide info
    int nMapIndex = c - FIRST_UNICODE;
    if( c > LAST_SURROGATE)         // skip surrogate unicode subset
        nMapIndex -= COUNT_SURROGATE;
    if( nMapIndex < FirstInMap() || nMapIndex > LastInMap())
        nMapIndex = FirstInMap();
    return nMapIndex;
}

inline sal_Unicode SvxShowCharSet::MapIndexToUnicode( int index) const
{
    //TODO: should depend on font and encoding, change when Font can provide info
    sal_Unicode c = FIRST_UNICODE + index;
    if( c >= FIRST_SURROGATE)       // skip surrogate unicode subset
        c += COUNT_SURROGATE;
    return c;
}

// -----------------------------------------------------------------------

inline int SvxShowCharSet::FirstInView( void) const
{
    int nIndex = FirstInMap();
    if( aVscrollSB.IsVisible())
        nIndex += aVscrollSB.GetThumbPos() * COLUMN_COUNT;
    return nIndex;
}

inline int SvxShowCharSet::LastInView( void) const
{
    int nIndex = FirstInView();
    nIndex += ROW_COUNT * COLUMN_COUNT - 1;
    return Min( nIndex, LastInMap());
}

inline Point SvxShowCharSet::MapIndexToPixel( int index) const
{
    int base = FirstInView();
    int x = ((index - base) % COLUMN_COUNT) * nX;
    int y = ((index - base) / COLUMN_COUNT) * nY;
    return Point( x, y);
}

int SvxShowCharSet::PixelToMapIndex( const Point point) const
{
    int base = FirstInView();
    return (base + (point.X()/nX) + (point.Y()/nY) * COLUMN_COUNT);
}

// -----------------------------------------------------------------------

void SvxShowCharSet::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aCode = rKEvt.GetKeyCode();

    if ( aCode.GetModifier() )
    {
        Control::KeyInput( rKEvt );
        return;
    }

    sal_Unicode cChar = (sal_Unicode)rKEvt.GetCharCode();
    int mapIndex = UnicodeToMapIndex( cChar);
    if ( mapIndex>FirstInMap() && mapIndex<=LastInMap())
    {
        SelectIndex( mapIndex );
        aPreSelectHdl.Call( this );
        return;
    }

    int tmpSelected = nSelectedIndex;

    switch ( aCode.GetCode() )
    {
        case KEY_SPACE:
            aSelectHdl.Call( this );
            break;
        case KEY_LEFT:
            --tmpSelected;
            break;
        case KEY_RIGHT:
            ++tmpSelected;
            break;
        case KEY_UP:
            tmpSelected -= COLUMN_COUNT;
            break;
        case KEY_DOWN:
            tmpSelected += COLUMN_COUNT;
            break;
        case KEY_PAGEUP:
            tmpSelected -= ROW_COUNT * COLUMN_COUNT;
            break;
        case KEY_PAGEDOWN:
            tmpSelected += ROW_COUNT * COLUMN_COUNT;
            break;
        case KEY_HOME:
            tmpSelected = FirstInMap();
            break;
        case KEY_END:
            tmpSelected = LastInMap();
            break;
        default:
            Control::KeyInput( rKEvt );
            tmpSelected = FirstInMap() - 1; // mark as invalid
    }

    if ( tmpSelected >= FirstInMap() && tmpSelected <= LastInMap() ) {
        SelectIndex( tmpSelected, TRUE);
        aPreSelectHdl.Call( this );
    }
}

// -----------------------------------------------------------------------

void SvxShowCharSet::Paint( const Rectangle& )
{
    DrawChars_Impl( FirstInView(), LastInView());
}

// -----------------------------------------------------------------------

void SvxShowCharSet::DrawChars_Impl( int n1, int n2)
{
    if( n1 > LastInView() || n2 < FirstInView())
        return;

    Size aOutputSize = GetOutputSizePixel();
    if( aVscrollSB.IsVisible())
        aOutputSize.setWidth( aOutputSize.Width() - SBWIDTH);

    int i;
    for ( i = 1; i < COLUMN_COUNT; ++i)
        DrawLine( Point( nX * i, 0 ), Point( nX * i, aOutputSize.Height() ) );
    for ( i = 1; i < ROW_COUNT; ++i )
        DrawLine( Point( 0, nY * i ), Point( aOutputSize.Width(), nY * i ) );

    for ( i = n1; i <= n2; ++i)
    {
        Point pix = MapIndexToPixel( i);
        int x = pix.X();
        int y = pix.Y();

        if ( i == nSelectedIndex && HasFocus() )
        {
            const StyleSettings& rStyleSettings =
                Application::GetSettings().GetStyleSettings();

            Color aLineCol = GetLineColor();
            Color aFillCol = GetFillColor();
            SetLineColor();
            SetFillColor( rStyleSettings.GetFaceColor() );
            DrawRect( Rectangle( Point( x+1, y+1), Size( nX-1, nY-1)));
            SetLineColor( rStyleSettings.GetLightColor() );
            DrawLine( Point( x+1, y+1 ), Point( x+nX-1, y+1));
            DrawLine( Point( x+1, y+1 ), Point( x+1, y+nY-1));
            SetLineColor( rStyleSettings.GetShadowColor() );
            DrawLine( Point( x+1, y+nY-1), Point( x+nX-1, y+nY-1));
            DrawLine( Point( x+nX-1, y+nY-1), Point( x+nX-1, y+1));
            SetLineColor( aLineCol );
            SetFillColor( aFillCol );
        }

        String aCharStr( MapIndexToUnicode( i ) );
        x += ( nX - GetTextWidth(aCharStr) ) / 2;
        y += ( nY - GetTextHeight() ) / 2;
        DrawText( Point( x, y ), aCharStr );
    }
}

// -----------------------------------------------------------------------

void SvxShowCharSet::InitSettings( BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if ( bForeground )
    {
        Color aTextColor = rStyleSettings.GetWindowTextColor();

        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( rStyleSettings.GetWindowColor() );
    }

    Invalidate();
}

// -----------------------------------------------------------------------

sal_Unicode SvxShowCharSet::GetSelectCharacter() const
{
    return MapIndexToUnicode( nSelectedIndex );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::SetFont( const Font& rFont )
{
    sal_Unicode cSelectedChar = MapIndexToUnicode( nSelectedIndex );
    if( nSelectedIndex < FirstInView() || nSelectedIndex > LastInView() )
        cSelectedChar = MapIndexToUnicode( FirstInView() );

    Font aFont = rFont;

    aFont.SetWeight( WEIGHT_LIGHT );
    aFont.SetSize( Size( 0, 12 ) );
    aFont.SetTransparent( TRUE );
    Control::SetFont( aFont );

    // hide scrollbar when there is nothing to scroll
    BOOL bNeedVscroll = ( LastInMap()-FirstInMap()+1 > ROW_COUNT*COLUMN_COUNT );

    nX = (aOrigSize.Width() - (bNeedVscroll ? SBWIDTH : 0)) / COLUMN_COUNT;
    nY = aOrigSize.Height() / ROW_COUNT;

    if( bNeedVscroll)
    {
        aVscrollSB.SetPosSizePixel( nX * COLUMN_COUNT, 0, SBWIDTH, nY * ROW_COUNT);
        aVscrollSB.SetRangeMin( 0 );
        int nLastRow = (LastInMap() - FirstInMap() + COLUMN_COUNT) / COLUMN_COUNT;
        aVscrollSB.SetRangeMax( nLastRow);
        aVscrollSB.SetPageSize( ROW_COUNT-1);
        aVscrollSB.SetVisibleSize( ROW_COUNT);
    }

    // rearrange CharSet element in sync with nX- and nY-multiples
    Size aNewSize( nX * COLUMN_COUNT + (bNeedVscroll ? SBWIDTH : 0), nY * ROW_COUNT);
    Point aNewPos = aOrigPos + Point( (aOrigSize.Width() - aNewSize.Width()) / 2, 0);
    SetPosPixel( aNewPos);
    SetOutputSizePixel( aNewSize);

    int nMapIndex = UnicodeToMapIndex( cSelectedChar );
    SelectIndex( nMapIndex );
    aVscrollSB.Show( bNeedVscroll );

    Invalidate();
}

// -----------------------------------------------------------------------

void SvxShowCharSet::SelectIndex( int nNewIndex, BOOL bFocus )
{
    if ( (nSelectedIndex == nNewIndex) && !bFocus )
        return;

    if( nNewIndex < FirstInView()) {
        // need to scroll up to see selected item
        int nNewPos = aVscrollSB.GetThumbPos();
        nNewPos -= (FirstInView() - nNewIndex + COLUMN_COUNT-1) / COLUMN_COUNT;
        aVscrollSB.SetThumbPos( nNewPos);
        nSelectedIndex = nNewIndex;
        Invalidate();
    }
    else if( nNewIndex > LastInView()) {
        // need to scroll down to see selected item
        int nNewPos = aVscrollSB.GetThumbPos();
        nNewPos += (nNewIndex - LastInView() + COLUMN_COUNT) / COLUMN_COUNT;
        aVscrollSB.SetThumbPos( nNewPos);
        nSelectedIndex = nNewIndex;
        Invalidate();
    } else {
        // remove highlighted view
        Color aLineCol = GetLineColor();
        Color aFillCol = GetFillColor();
        SetLineColor();
        SetFillColor( GetBackground().GetColor() );

        Point aOldPixel = MapIndexToPixel( nSelectedIndex);
        aOldPixel.Move( +1, +1);
        DrawRect( Rectangle( aOldPixel, Size( nX-1, nY-1)));
        SetLineColor( aLineCol );
        SetFillColor( aFillCol );

        int nOldIndex = nSelectedIndex;
        nSelectedIndex = nNewIndex;
        DrawChars_Impl( nOldIndex, nOldIndex );
        DrawChars_Impl( nNewIndex, nNewIndex );
    }

    aHighHdl.Call( this );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::SelectCharacter( sal_Unicode cNew, BOOL bFocus )
{
    int nMapIndex = UnicodeToMapIndex( cNew);
    SelectIndex( nMapIndex, bFocus);
    if( !bFocus) {
        // move selected item to top row if unfocused
        aVscrollSB.SetThumbPos( (nMapIndex - FirstInMap()) / COLUMN_COUNT);
        Invalidate();
    }
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxShowCharSet, VscrollHdl, ScrollBar *, EMPTYARG )
{
    if( nSelectedIndex < FirstInView() || nSelectedIndex > LastInView())
        SelectIndex( FirstInView());

    Invalidate();
    return 0;
}
IMPL_LINK_INLINE_END( SvxShowCharSet, VscrollHdl, ScrollBar *, EMPTYARG )

// -----------------------------------------------------------------------

SvxShowCharSet::~SvxShowCharSet()
{
}

// class SvxShowText -----------------------------------------------------

SvxShowText::SvxShowText( Window* pParent, const ResId& rResId, BOOL _bCenter) :

    Control( pParent, rResId ),
    bCenter( _bCenter)
{
}

// -----------------------------------------------------------------------

void SvxShowText::Paint( const Rectangle& )
{
    if ( bCenter )
    {
        String aText = GetText();
        DrawText( Point( (GetOutputSizePixel().Width() - GetTextWidth(aText))/2, nY),
                  aText );
    }
    else
        DrawText( Point( 2, nY ), GetText() );
}

// -----------------------------------------------------------------------

void SvxShowText::SetFont( const Font& rFont )
{
    Invalidate();

    long nWinHeight = GetOutputSizePixel().Height();
    Font aFont = rFont;
    aFont.SetWeight( WEIGHT_NORMAL );
    aFont.SetSize( Size( 0, nWinHeight-6 ) );
    aFont.SetTransparent( TRUE );
    Control::SetFont( aFont );
    nY = ( nWinHeight - GetTextHeight() ) / 2;
}

// -----------------------------------------------------------------------

void SvxShowText::SetText( const String& rText )
{
    Control::SetText( rText );
    Invalidate();
}

// -----------------------------------------------------------------------

SvxShowText::~SvxShowText()
{
}

// class SvxCharacterMap -------------------------------------------------

SvxCharacterMap::SvxCharacterMap( Window* pParent, BOOL bOne ) :

    SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_CHARMAP ) ),

    aShowSet        ( this, ResId( CT_SHOWSET ) ),
    aFontText       ( this, ResId( FT_FONT ) ),
    aFontLB         ( this, ResId( LB_FONT ) ),
    aSubsetText     ( this, ResId( FT_SUBSET ) ),
    aSubsetLB       ( this, ResId( LB_SUBSET ) ),
    aSymbolText     ( this, ResId( FT_SYMBOLE ) ),
    aShowText       ( this, ResId( CT_SHOWTEXT ) ),
    aShowChar       ( this, ResId( CT_SHOWCHAR ), TRUE ),
    aCharCodeText   ( this, ResId( FT_CHARCODE ) ),
    aOKBtn          ( this, ResId( BTN_CHAR_OK ) ),
    aCancelBtn      ( this, ResId( BTN_CHAR_CANCEL ) ),
    aHelpBtn        ( this, ResId( BTN_CHAR_HELP ) ),
    aDeleteBtn      ( this, ResId( BTN_DELETE ) ),
    pSubsetMap( 0)

{
    FreeResource();

    aFont = GetFont();
    aFont.SetTransparent( TRUE );
    aFont.SetFamily( FAMILY_DONTKNOW );
    aFont.SetPitch( PITCH_DONTKNOW );
    aFont.SetCharSet( RTL_TEXTENCODING_DONTKNOW );

    SvxCharacterMap::bOne = bOne;

    if ( bOne )
    {
        Size aDlgSize = GetSizePixel();
        SetSizePixel( Size( aDlgSize.Width(),
                               aDlgSize.Height()-aShowText.GetSizePixel().Height() ) );
        aSymbolText.Hide();
        aShowText.Hide();
        aDeleteBtn.Hide();
    }

    String aDefStr( aFont.GetName() );
    String aLastName;
    xub_StrLen i;
    xub_StrLen nCount = GetDevFontCount();
    for ( i = 0; i < nCount; i++ )
    {
        String aFontName( GetDevFont( i ).GetName() );
        if ( aFontName != aLastName )
        {
            aLastName = aFontName;
            USHORT nPos = aFontLB.InsertEntry( aFontName );
            aFontLB.SetEntryData( nPos, (void*)(ULONG)i );
        }
    }
    // the font may not be in the list =>
    // try to find a font name token in list and select found font,
    // else select topmost entry
    FASTBOOL bFound = (aFontLB.GetEntryPos( aDefStr ) == LISTBOX_ENTRY_NOTFOUND );
    if( !bFound )
    {
        for ( i = 0; i < aDefStr.GetTokenCount(); ++i )
        {
            String aToken = aDefStr.GetToken(i);

            if ( aFontLB.GetEntryPos( aToken ) != LISTBOX_ENTRY_NOTFOUND )
            {
                aDefStr = aToken;
                bFound = TRUE;
                break;
            }
        }
    }

    if ( bFound )
        aFontLB.SelectEntry( aDefStr );
    else if ( aFontLB.GetEntryCount() )
        aFontLB.SelectEntryPos(0);
    FontSelectHdl( &aFontLB );

    aOKBtn.SetClickHdl( LINK( this, SvxCharacterMap, OKHdl ) );
    aFontLB.SetSelectHdl( LINK( this, SvxCharacterMap, FontSelectHdl ) );
    aSubsetLB.SetSelectHdl( LINK( this, SvxCharacterMap, SubsetSelectHdl ) );
    aShowSet.SetDoubleClickHdl( LINK( this, SvxCharacterMap, CharDoubleClickHdl ) );
    aShowSet.SetSelectHdl( LINK( this, SvxCharacterMap, CharSelectHdl ) );
    aShowSet.SetHighlightHdl( LINK( this, SvxCharacterMap, CharHighlightHdl ) );
    aShowSet.SetPreSelectHdl( LINK( this, SvxCharacterMap, CharPreSelectHdl ) );
    aDeleteBtn.SetClickHdl( LINK( this, SvxCharacterMap, DeleteHdl ) );

    SetChar( FIRST_UNICODE );
    aOKBtn.Disable();
}

// -----------------------------------------------------------------------

void SvxCharacterMap::SetFont( const Font& rFont )
{
    SetCharFont( rFont );
}

// -----------------------------------------------------------------------

void SvxCharacterMap::DisableFontSelection()
{
    aFontText.Disable();
    aFontLB.Disable();
}

// -----------------------------------------------------------------------

void SvxCharacterMap::SetCharFont( const Font& rFont )
{
    //Font ersteinmal ermitteln lassen, damit auch auch Fonts mit dem
    //Namen "Times New Roman;Times" auf der jeweiligen Plattform richtig
    //funktionieren.
    Font aTmp( GetFontMetric( rFont ) );

    if ( aFontLB.GetEntryPos( aTmp.GetName() ) == LISTBOX_ENTRY_NOTFOUND )
        return;

    aFontLB.SelectEntry( aTmp.GetName() );
    aFont = aTmp;
    FontSelectHdl( &aFontLB );

    // for compatibility reasons
    ModalDialog::SetFont( aFont );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharacterMap, OKHdl, OKButton *, EMPTYARG )
{
    String aStr = aShowText.GetText();

    if ( !aStr.Len() )
    {
        if ( aShowSet.GetSelectCharacter() > 0 )
            aStr = aShowSet.GetSelectCharacter();
        aShowText.SetText( aStr );
    }
    EndDialog( TRUE );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharacterMap, FontSelectHdl, ListBox *, EMPTYARG )
{
    USHORT nPos = aFontLB.GetSelectEntryPos(),
           nFont = (USHORT)(ULONG)aFontLB.GetEntryData( nPos );
    aFont = GetDevFont( nFont );

    // notify children using this font
    aShowSet.SetFont( aFont );
    aShowText.SetFont( aFont );
    aShowChar.SetFont( aFont );

    // setup unicode subset listbar with font specific subsets,
    // hide unicode subset listbar for symbol fonts
    // TODO: get info from the Font once it provides it
    if( pSubsetMap)
        delete pSubsetMap;
    pSubsetMap = new SubsetMap;

    BOOL bNeedSubset = (aFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL);
    if( bNeedSubset) {
        // update subset listbox for new font's unicode subsets
        aSubsetLB.Clear();
        const Subset* s = 0;
        // TODO: is it worth to improve stupid linear search?
        for( int i = 0; (s = pSubsetMap->GetSubsetByIndex( i)) != 0; ++i) {
            USHORT nPos = aSubsetLB.InsertEntry( s->GetName());
            aSubsetLB.SetEntryData( nPos, (void*)s );
            // subset must live at least as long as the selected font !!!
            if( i == 0)
                aSubsetLB.SelectEntryPos( nPos);
        }
        if( aSubsetLB.GetEntryCount() <= 1)
            bNeedSubset = FALSE;
    }
    aSubsetText.Show( bNeedSubset);
    aSubsetLB.Show( bNeedSubset);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharacterMap, SubsetSelectHdl, ListBox *, EMPTYARG )
{
    USHORT nPos = aSubsetLB.GetSelectEntryPos();
    const Subset* subset = reinterpret_cast<const Subset*> (aSubsetLB.GetEntryData(nPos));
    if( subset) {
        sal_Unicode c = subset->GetRangeMin();
        aShowSet.SelectCharacter( c);
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxCharacterMap, CharDoubleClickHdl, Control *, EMPTYARG )
{
    EndDialog( TRUE );
    return 0;
}
IMPL_LINK_INLINE_END( SvxCharacterMap, CharDoubleClickHdl, Control *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharacterMap, CharSelectHdl, Control *, EMPTYARG )
{
    if ( !bOne )
    {
        String aText = aShowText.GetText();

        if ( aText.Len() == CHARMAP_MAXLEN )
            Sound::Beep( SOUND_WARNING );
        else
        {
            if ( aShowSet.GetSelectCharacter() > 0 )
                aText += aShowSet.GetSelectCharacter();
            aShowText.SetText( aText );
        }
    }
    aOKBtn.Enable();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharacterMap, CharHighlightHdl, Control *, EMPTYARG )
{
    String aTemp;
    sal_Unicode c = aShowSet.GetSelectCharacter();
    sal_Bool bSelect = ( c > 0 );
    if ( bSelect ) {
        aTemp = c;
        const Subset* subset = pSubsetMap->GetSubsetByUnicode( c);
        if( subset)
            aSubsetLB.SelectEntry( subset->GetName());
    }
    aShowChar.SetText( aTemp );
    aShowChar.Update();
    if ( bSelect )
        aTemp = String::CreateFromInt32( (USHORT)c );
    aCharCodeText.SetText( aTemp );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxCharacterMap, CharPreSelectHdl, Control *, EMPTYARG )
{
    aOKBtn.Enable();
    return 0;
}
IMPL_LINK_INLINE_END( SvxCharacterMap, CharPreSelectHdl, Control *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxCharacterMap, DeleteHdl, PushButton *, EMPTYARG )
{
    aShowText.SetText( String() );
    aOKBtn.Disable();
    return 0;
}
IMPL_LINK_INLINE_END( SvxCharacterMap, DeleteHdl, PushButton *, EMPTYARG )

// -----------------------------------------------------------------------

SvxCharacterMap::~SvxCharacterMap()
{
}


// class SubsetMap -------------------------------------------------------
// TODO: should be moved into Font Attributes stuff
// we let it mature here though because it is currently the only use

SubsetMap::SubsetMap( /* TODO */ ) :
    Resource( ResId( SVX_RES(RID_SUBSETMAP)))
{
    InitList();
    FreeResource();
}

SubsetMap::~SubsetMap()
{
    for( int i = nSubsets; --i >= 0;)
        delete aSubsets[ i];
    delete[] aSubsets;
}

const Subset* SubsetMap::GetSubsetByIndex( int index) const
{
    if( (index >= 0) && (index < nSubsets))
        return aSubsets[ index];
    return 0;
}

const Subset* SubsetMap::GetSubsetByUnicode( sal_Unicode c) const
{
    const Subset* s = 0;
    for( int i = 0; (s = GetSubsetByIndex( i)) != 0; ++i)
        if( (c >= s->GetRangeMin()) && (c <= s->GetRangeMax()))
            return s;
    return 0;
}

inline Subset::Subset( sal_Unicode _min, sal_Unicode _max, int resId)
:   rangeMin(_min), rangeMax(_max), rangeName( ResId(resId))
{}

void SubsetMap::InitList( void)
{
    aSubsets = new const Subset*[ RID_SUBSET_COUNT];
    int i = 0;
    // TODO: eventually merge or split unicode subranges
    //       a "native writer" should decide for his subsets
    aSubsets[ i++] = new Subset( 0x0041, 0x007A, RID_SUBSETSTR_BASIC_LATIN);
    aSubsets[ i++] = new Subset( 0x00C0, 0x00FF, RID_SUBSETSTR_LATIN_1);
    aSubsets[ i++] = new Subset( 0x0100, 0x017F, RID_SUBSETSTR_LATIN_EXTENDED_A);
    aSubsets[ i++] = new Subset( 0x0180, 0x024F, RID_SUBSETSTR_LATIN_EXTENDED_B);
    aSubsets[ i++] = new Subset( 0x0250, 0x02AF, RID_SUBSETSTR_IPA_EXTENSIONS);
    aSubsets[ i++] = new Subset( 0x02B0, 0x02FF, RID_SUBSETSTR_SPACING_MODIFIERS);
    aSubsets[ i++] = new Subset( 0x0300, 0x036F, RID_SUBSETSTR_COMB_DIACRITICAL);
    aSubsets[ i++] = new Subset( 0x0370, 0x03FF, RID_SUBSETSTR_BASIC_GREEK);
//  aSubsets[ i++] = new Subset( 0x03D0, 0x03F3, RID_SUBSETSTR_GREEK_SYMS_COPTIC);
    aSubsets[ i++] = new Subset( 0x0400, 0x04FF, RID_SUBSETSTR_CYRILLIC);
    aSubsets[ i++] = new Subset( 0x0530, 0x058F, RID_SUBSETSTR_ARMENIAN);
    aSubsets[ i++] = new Subset( 0x0590, 0x05FF, RID_SUBSETSTR_BASIC_HEBREW);
//  aSubsets[ i++] = new Subset( 0x0591, 0x05C4, RID_SUBSETSTR_HEBREW_EXTENDED);
    aSubsets[ i++] = new Subset( 0x0600, 0x06FF, RID_SUBSETSTR_BASIC_ARABIC);
//  aSubsets[ i++] = new Subset( 0x0660, 0x06FF, RID_SUBSETSTR_ARABIC_EXTENDED);
    aSubsets[ i++] = new Subset( 0x0700, 0x074F, RID_SUBSETSTR_SYRIAC);
    aSubsets[ i++] = new Subset( 0x0780, 0x07BF, RID_SUBSETSTR_THAANA);

    aSubsets[ i++] = new Subset( 0x0900, 0x097F, RID_SUBSETSTR_DEVANAGARI);
    aSubsets[ i++] = new Subset( 0x0980, 0x09FF, RID_SUBSETSTR_BENGALI);
    aSubsets[ i++] = new Subset( 0x0A00, 0x0A7F, RID_SUBSETSTR_GURMUKHI);
    aSubsets[ i++] = new Subset( 0x0A80, 0x0AFF, RID_SUBSETSTR_GUJARATI);
    aSubsets[ i++] = new Subset( 0x0B00, 0x0B7F, RID_SUBSETSTR_ORIYA);
    aSubsets[ i++] = new Subset( 0x0B80, 0x0BFF, RID_SUBSETSTR_TAMIL);
    aSubsets[ i++] = new Subset( 0x0C00, 0x0C7F, RID_SUBSETSTR_TELUGU);
    aSubsets[ i++] = new Subset( 0x0C80, 0x0CFF, RID_SUBSETSTR_KANNADA);
    aSubsets[ i++] = new Subset( 0x0D00, 0x0D7F, RID_SUBSETSTR_MALAYALAM);
    aSubsets[ i++] = new Subset( 0x0D80, 0x0DFF, RID_SUBSETSTR_SINHALA);
    aSubsets[ i++] = new Subset( 0x0E00, 0x0E7F, RID_SUBSETSTR_THAI);
    aSubsets[ i++] = new Subset( 0x0E80, 0x0EFF, RID_SUBSETSTR_LAO);
    aSubsets[ i++] = new Subset( 0x0F00, 0x0FBF, RID_SUBSETSTR_TIBETAN);
    aSubsets[ i++] = new Subset( 0x1000, 0x109F, RID_SUBSETSTR_MYANMAR);
    aSubsets[ i++] = new Subset( 0x10A0, 0x10FF, RID_SUBSETSTR_BASIC_GEORGIAN);
//  aSubsets[ i++] = new Subset( 0x10A0, 0x10C5, RID_SUBSETSTR_GEORGIAN_EXTENDED);

    aSubsets[ i++] = new Subset( 0x1100, 0x11FF, RID_SUBSETSTR_HANGUL_JAMO);
    aSubsets[ i++] = new Subset( 0x1200, 0x137F, RID_SUBSETSTR_ETHIOPIC);
    aSubsets[ i++] = new Subset( 0x13A0, 0x13FF, RID_SUBSETSTR_CHEROKEE);
    aSubsets[ i++] = new Subset( 0x1400, 0x167F, RID_SUBSETSTR_CANADIAN_ABORIGINAL);
    aSubsets[ i++] = new Subset( 0x1680, 0x169F, RID_SUBSETSTR_OGHAM);
    aSubsets[ i++] = new Subset( 0x16A0, 0x16F0, RID_SUBSETSTR_RUNIC);

    aSubsets[ i++] = new Subset( 0x1780, 0x17FF, RID_SUBSETSTR_KHMER);
    aSubsets[ i++] = new Subset( 0x1800, 0x18AF, RID_SUBSETSTR_MONGOLIAN);
    aSubsets[ i++] = new Subset( 0x1E00, 0x1EFF, RID_SUBSETSTR_LATIN_EXTENDED_ADDS);
    aSubsets[ i++] = new Subset( 0x1F00, 0x1FFF, RID_SUBSETSTR_GREEK_EXTENDED);

    aSubsets[ i++] = new Subset( 0x2000, 0x206F, RID_SUBSETSTR_GENERAL_PUNCTUATION);
    aSubsets[ i++] = new Subset( 0x2070, 0x209F, RID_SUBSETSTR_SUB_SUPER_SCRIPTS);
    aSubsets[ i++] = new Subset( 0x20A0, 0x20CF, RID_SUBSETSTR_CURRENCY_SYMBOLS);
    aSubsets[ i++] = new Subset( 0x20D0, 0x20FF, RID_SUBSETSTR_COMB_DIACRITIC_SYMS);
    aSubsets[ i++] = new Subset( 0x2100, 0x214F, RID_SUBSETSTR_LETTERLIKE_SYMBOLS);
    aSubsets[ i++] = new Subset( 0x2150, 0x218F, RID_SUBSETSTR_NUMBER_FORMS);
    aSubsets[ i++] = new Subset( 0x2190, 0x21FF, RID_SUBSETSTR_ARROWS);
    aSubsets[ i++] = new Subset( 0x2200, 0x22FF, RID_SUBSETSTR_MATH_OPERATORS);
    aSubsets[ i++] = new Subset( 0x2300, 0x23FF, RID_SUBSETSTR_MISC_TECHNICAL);
    aSubsets[ i++] = new Subset( 0x2400, 0x243F, RID_SUBSETSTR_CONTROL_PICTURES);
    aSubsets[ i++] = new Subset( 0x2440, 0x245F, RID_SUBSETSTR_OPTICAL_CHAR_REC);
    aSubsets[ i++] = new Subset( 0x2460, 0x24FF, RID_SUBSETSTR_ENCLOSED_ALPHANUM);
    aSubsets[ i++] = new Subset( 0x2500, 0x257F, RID_SUBSETSTR_BOX_DRAWING);
    aSubsets[ i++] = new Subset( 0x2580, 0x259F, RID_SUBSETSTR_BLOCK_ELEMENTS);
    aSubsets[ i++] = new Subset( 0x25A0, 0x25FF, RID_SUBSETSTR_GEOMETRIC_SHAPES);
    aSubsets[ i++] = new Subset( 0x2600, 0x26FF, RID_SUBSETSTR_MISC_DINGBATS);
    aSubsets[ i++] = new Subset( 0x2700, 0x27BF, RID_SUBSETSTR_DINGBATS);

    aSubsets[ i++] = new Subset( 0x3000, 0x303F, RID_SUBSETSTR_CJK_SYMS_PUNCTUATION);
    aSubsets[ i++] = new Subset( 0x3040, 0x309F, RID_SUBSETSTR_HIRAGANA);
    aSubsets[ i++] = new Subset( 0x30A0, 0x30FF, RID_SUBSETSTR_KATAKANA);
    aSubsets[ i++] = new Subset( 0x3100, 0x312F, RID_SUBSETSTR_BOPOMOFO);
    aSubsets[ i++] = new Subset( 0x3130, 0x318F, RID_SUBSETSTR_HANGUL_COMPAT_JAMO);
    aSubsets[ i++] = new Subset( 0x3190, 0x31FF, RID_SUBSETSTR_CJK_MISC);
    aSubsets[ i++] = new Subset( 0x3200, 0x32FF, RID_SUBSETSTR_ENCLOSED_CJK_LETTERS);
    aSubsets[ i++] = new Subset( 0x3300, 0x33FF, RID_SUBSETSTR_CJK_COMPATIBILITY);

    aSubsets[ i++] = new Subset( 0x3400, 0x4DFF, RID_SUBSETSTR_CJK_EXT_A_UNIFIED_IDGRAPH);
    aSubsets[ i++] = new Subset( 0x4E00, 0x9FA5, RID_SUBSETSTR_CJK_UNIFIED_IDGRAPH);
    aSubsets[ i++] = new Subset( 0xA000, 0xA4CF, RID_SUBSETSTR_YI);
    aSubsets[ i++] = new Subset( 0xAC00, 0xB097, RID_SUBSETSTR_HANGUL_GA);
    aSubsets[ i++] = new Subset( 0xB098, 0xB2E3, RID_SUBSETSTR_HANGUL_NA);
    aSubsets[ i++] = new Subset( 0xB2E4, 0xB77B, RID_SUBSETSTR_HANGUL_DA);
    aSubsets[ i++] = new Subset( 0xB77C, 0xB9C7, RID_SUBSETSTR_HANGUL_RA);
    aSubsets[ i++] = new Subset( 0xB9C8, 0xBC13, RID_SUBSETSTR_HANGUL_MA);
    aSubsets[ i++] = new Subset( 0xBC14, 0xC0AB, RID_SUBSETSTR_HANGUL_BA);
    aSubsets[ i++] = new Subset( 0xC0AC, 0xC543, RID_SUBSETSTR_HANGUL_SA);
    aSubsets[ i++] = new Subset( 0xC544, 0xC78F, RID_SUBSETSTR_HANGUL_AH);
    aSubsets[ i++] = new Subset( 0xC790, 0xCC27, RID_SUBSETSTR_HANGUL_JA);
    aSubsets[ i++] = new Subset( 0xCC28, 0xCE73, RID_SUBSETSTR_HANGUL_CHA);
    aSubsets[ i++] = new Subset( 0xCE74, 0xD0BF, RID_SUBSETSTR_HANGUL_KA);
    aSubsets[ i++] = new Subset( 0xD0C0, 0xD30B, RID_SUBSETSTR_HANGUL_TA);
    aSubsets[ i++] = new Subset( 0xD30C, 0xD557, RID_SUBSETSTR_HANGUL_PA);
    aSubsets[ i++] = new Subset( 0xD558, 0xD7A3, RID_SUBSETSTR_HANGUL_HA);
//  aSubsets[ i++] = new Subset( 0xAC00, 0xD7A3, RID_SUBSETSTR_HANGUL);

//  aSubsets[ i++] = new Subset( 0xD800, 0xDFFF, RID_SUBSETSTR_SURROGATE);
    aSubsets[ i++] = new Subset( 0xE000, 0xF8FF, RID_SUBSETSTR_PRIVATE_USE_AREA);
    aSubsets[ i++] = new Subset( 0xF900, 0xFAFF, RID_SUBSETSTR_CJK_COMPAT_IDGRAPHS);
    aSubsets[ i++] = new Subset( 0xFB00, 0xFB4F, RID_SUBSETSTR_ALPHA_PRESENTATION);
    aSubsets[ i++] = new Subset( 0xFB50, 0xFDFF, RID_SUBSETSTR_ARABIC_PRESENT_A);
    aSubsets[ i++] = new Subset( 0xFE20, 0xFE2F, RID_SUBSETSTR_COMBINING_HALF_MARKS);
    aSubsets[ i++] = new Subset( 0xFE30, 0xFE4F, RID_SUBSETSTR_CJK_COMPAT_FORMS);
    aSubsets[ i++] = new Subset( 0xFE50, 0xFE6F, RID_SUBSETSTR_SMALL_FORM_VARIANTS);
    aSubsets[ i++] = new Subset( 0xFE70, 0xFEFF, RID_SUBSETSTR_ARABIC_PRESENT_B);
    aSubsets[ i++] = new Subset( 0xFF00, 0xFFEF, RID_SUBSETSTR_HALFW_FULLW_FORMS);
    aSubsets[ i++] = new Subset( 0xFFF0, 0xFFFF, RID_SUBSETSTR_SPECIALS);

    nSubsets = i;
    DBG_ASSERT( (nSubsets < RID_SUBSET_COUNT), "RID_SUBSET_COUNT too small");
    DBG_ASSERT( (2*nSubsets > RID_SUBSET_COUNT), "RID_SUBSET_COUNT way to big");
}
