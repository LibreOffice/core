/*************************************************************************
 *
 *  $RCSfile: charmap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hdu $ $Date: 2000-10-17 17:12:59 $
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

#include "dialogs.hrc"
#include "charmap.hrc"

#include "charmap.hxx"
#include "dialmgr.hxx"

#include <rtl/textenc.h>

// class SvxShowCharSet --------------------------------------------------

#define SBWIDTH 16

SvxShowCharSet::SvxShowCharSet( Window* pParent, const ResId& rResId ) :

    Control( pParent, rResId ),
    aVscrollSB( this, WB_VERT),
    selectedIndex( FirstInMap())
{
    origSize = GetOutputSizePixel();
    origPos = GetPosPixel();

    SetStyle( GetStyle() | WB_CLIPCHILDREN);
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
    SelectIndex( selectedIndex, TRUE );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::LoseFocus()
{
    Control::LoseFocus();
    SelectIndex( selectedIndex, TRUE );
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
#define LAST_UNICODE    sal_Unicode(0xFFFF)     // end of UCS-16

inline int SvxShowCharSet::FirstInMap( void) const
{
    return 0;
}

inline int SvxShowCharSet::LastInMap( void) const
{
    if( GetFont().GetCharSet() == RTL_TEXTENCODING_SYMBOL)
        return (0x0FF - 0x020);
    return (LAST_UNICODE - FIRST_UNICODE);
}

inline int SvxShowCharSet::FirstInView( void) const
{
    int idx = FirstInMap();
    if( aVscrollSB.IsVisible())
        idx += aVscrollSB.GetThumbPos() * COLUMN_COUNT;
    return idx;
}

inline int SvxShowCharSet::LastInView( void) const
{
    int idx = FirstInView();
    idx += ROW_COUNT * COLUMN_COUNT - 1;
    return Min( idx, LastInMap());
}

inline int SvxShowCharSet::UnicodeToMapIndex( sal_Unicode c) const
{
    //TODO: should depend on font and encoding, change when Font can provide info
    int mapIndex = c - FIRST_UNICODE;
    if( mapIndex < FirstInMap() || mapIndex > LastInMap())
        mapIndex = FirstInMap();
    return mapIndex;
}

inline Point SvxShowCharSet::MapIndexToPixel( int index) const
{
    int base = FirstInView();
    int x = ((index - base) % COLUMN_COUNT) * nX;
    int y = ((index - base) / COLUMN_COUNT) * nY;
    return Point( x, y);
}

inline int SvxShowCharSet::PixelToMapIndex( const Point point) const
{
    int base = FirstInView();
    return (base + (point.X()/nX) + (point.Y()/nY) * COLUMN_COUNT);
}

inline sal_Unicode SvxShowCharSet::MapIndexToUnicode( int index) const
{
    //TODO: should depend on font and encoding, change when Font can provide info
    return (FIRST_UNICODE + index);
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

    int tmpSelected = selectedIndex;

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
    }

    if ( tmpSelected >= FirstInMap() &&tmpSelected <= LastInMap() ) {
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

        if ( i == selectedIndex && HasFocus() )
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

        String aCharStr;
        if ( i > 0 )
            aCharStr = String( MapIndexToUnicode( i) );
        else
            aCharStr.Erase();

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
    return MapIndexToUnicode( selectedIndex);
}

// -----------------------------------------------------------------------

void SvxShowCharSet::SetFont( const Font& rFont )
{
    sal_Unicode selectedChar = MapIndexToUnicode( selectedIndex);
    if( selectedIndex < FirstInView() || selectedIndex > LastInView())
        selectedChar = MapIndexToUnicode( FirstInView());

    Font aFont = rFont;

    aFont.SetWeight( WEIGHT_LIGHT );
    aFont.SetSize( Size( 0, 12 ) );
    aFont.SetTransparent( TRUE );
    Control::SetFont( aFont );

    // hide scrollbar when there is nothing to scroll
    BOOL needVscroll = (LastInMap()-FirstInMap()+1 > ROW_COUNT*COLUMN_COUNT);

    nX = (origSize.Width() - (needVscroll ? SBWIDTH : 0)) / COLUMN_COUNT;
    nY = origSize.Height() / ROW_COUNT;

    if( needVscroll) {
        aVscrollSB.SetPosSizePixel( nX * COLUMN_COUNT, 0, SBWIDTH, nY * ROW_COUNT);
        int nVisible = ROW_COUNT;
        int lastRow  = (LastInMap() - FirstInMap() + COLUMN_COUNT) / COLUMN_COUNT;
        aVscrollSB.SetRangeMin( 0);
        aVscrollSB.SetRangeMax( lastRow);
        aVscrollSB.SetVisibleSize( nVisible);
    }

    // rearrange CharSet element in accordance with nX- and nY-multiples
    Size newSize( nX * COLUMN_COUNT + (needVscroll ? SBWIDTH : 0), nY * ROW_COUNT);
    Point newPos = origPos + Point( (origSize.Width() - newSize.Width()) / 2, 0);
    SetPosPixel( newPos);
    SetOutputSizePixel( newSize);

    int mapIndex = UnicodeToMapIndex( selectedChar);
    SelectIndex( mapIndex);
    aVscrollSB.Show( needVscroll);

    Invalidate();
}

// -----------------------------------------------------------------------

void SvxShowCharSet::SelectIndex( int idxNew, BOOL bFocus )
{
    if ( (selectedIndex == idxNew) && !bFocus )
        return;

    if( idxNew<FirstInView()) {
        // need to scroll up to see selected item
        int newPos = aVscrollSB.GetThumbPos();
        newPos -= (FirstInView() - idxNew + COLUMN_COUNT-1) / COLUMN_COUNT;
        aVscrollSB.SetThumbPos( newPos);
        selectedIndex = idxNew;
        Invalidate();
    }
    else if( idxNew>LastInView()) {
        // need to scroll down to see selected item
        int newPos = aVscrollSB.GetThumbPos();
        newPos += (idxNew - LastInView() + COLUMN_COUNT) / COLUMN_COUNT;
        aVscrollSB.SetThumbPos( newPos);
        selectedIndex = idxNew;
        Invalidate();
    } else {
        // remove highlighted view
        Color aLineCol = GetLineColor();
        Color aFillCol = GetFillColor();
        SetLineColor();
        SetFillColor( GetBackground().GetColor() );

        Point pixOld = MapIndexToPixel( selectedIndex);
        pixOld.Move( +1, +1);
        DrawRect( Rectangle( pixOld, Size( nX-1, nY-1)));
        SetLineColor( aLineCol );
        SetFillColor( aFillCol );

        int idxOld = selectedIndex;
        selectedIndex = idxNew;
        DrawChars_Impl( idxOld, idxOld );
        DrawChars_Impl( idxNew, idxNew );
    }

    aHighHdl.Call( this );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::SelectCharacter( sal_Unicode cNew, BOOL bFocus )
{
    SelectIndex( UnicodeToMapIndex( cNew), bFocus);
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxShowCharSet, VscrollHdl, ScrollBar *, EMPTYARG )
{
    if( selectedIndex < FirstInView() || selectedIndex > LastInView())
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

SvxShowText::SvxShowText( Window* pParent, const ResId& rResId, BOOL bCenter) :

    Control( pParent, rResId )

{
    SvxShowText::bCenter = bCenter;
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
    aDeleteBtn      ( this, ResId( BTN_DELETE ) )

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

    xub_StrLen i;
    xub_StrLen nCount = GetDevFontCount();
    String aDefStr( aFont.GetName() );

    for ( i = 0; i < nCount; i++ )
    {
        String aFontName( GetDevFont( i ).GetName() );

        if ( aFontLB.GetEntryPos( aFontName ) == LISTBOX_ENTRY_NOTFOUND )
        {
            USHORT nPos = aFontLB.InsertEntry( aFontName );
            aFontLB.SetEntryData( nPos, (void*)(ULONG)i );
        }
    }
    FASTBOOL bFound = FALSE;
    // the font may not be in the list ->
    // try to find a font name token in list and select found font,
    // else select topmost entry
    if ( aFontLB.GetEntryPos( aDefStr ) == LISTBOX_ENTRY_NOTFOUND )
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
    else
        bFound = TRUE;

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
            aStr = String( aShowSet.GetSelectCharacter() );
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
    BOOL bNeedSubset = (aFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL);
    if( bNeedSubset) {
        // update subset listbox for new font's unicode subsets
        const SubsetMap dummyMap;   // TODO: font should provide SubsetMap
        aSubsetLB.Clear();
        const Subset* s = 0;
        // TODO: is it worth to improve stupid linear search?
        for( int i = 0; (s = dummyMap.GetSubsetByIndex( i)) != 0; ++i) {
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
    const Subset* s = reinterpret_cast<const Subset*> (aSubsetLB.GetEntryData(nPos));
    if( s) {
        sal_Unicode c = s->GetRangeMin();
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
        aTemp = String( c );
        const SubsetMap dummyMap;   // TODO: font should provide SubsetMap
        const Subset* subset = dummyMap.GetSubsetByUnicode( c);
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
// we let the it mature here though because it is currently the only use

const const Subset** SubsetMap::allSubsets = 0;
int SubsetMap::numAllSubsets = 0;

SubsetMap::SubsetMap( /*TODO*/void)
{
    if( allSubsets == 0)
        InitStatics();
}

SubsetMap::~SubsetMap( void)
{
}

const Subset* SubsetMap::GetSubsetByIndex( int index) const
{
    if( (index >= 0) && (index < numAllSubsets))
        return allSubsets[ index];
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
:   rangeMin(_min), rangeMax(_max), rangeName( ResId(SVX_RES(resId)))
{}

void SubsetMap::InitStatics( void)
{
    DBG_ASSERT( allSubsets == 0, "Unicode Subset Information double init");

    allSubsets = new const Subset*[ SVXCOUNT_CHARRANGE];
    int i = 0;
    // TODO: eventually merge or split unicode subranges
    //       a "native writer" should decide for his subsets
    allSubsets[ i++] = new Subset( 0x0041, 0x007A, RID_SVXSTR_BASIC_LATIN);
    allSubsets[ i++] = new Subset( 0x00C0, 0x00FF, RID_SVXSTR_LATIN_1);
    allSubsets[ i++] = new Subset( 0x0100, 0x017F, RID_SVXSTR_LATIN_EXTENDED_A);
    allSubsets[ i++] = new Subset( 0x0180, 0x024F, RID_SVXSTR_LATIN_EXTENDED_B);
    allSubsets[ i++] = new Subset( 0x0250, 0x02AF, RID_SVXSTR_IPA_EXTENSIONS);
    allSubsets[ i++] = new Subset( 0x02B0, 0x02FF, RID_SVXSTR_SPACING_MODIFIERS);
    allSubsets[ i++] = new Subset( 0x0300, 0x036F, RID_SVXSTR_COMB_DIACRITICAL);
    allSubsets[ i++] = new Subset( 0x0370, 0x03FF, RID_SVXSTR_BASIC_GREEK);
//  allSubsets[ i++] = new Subset( 0x03D0, 0x03F3, RID_SVXSTR_GREEK_SYMS_COPTIC);
    allSubsets[ i++] = new Subset( 0x0400, 0x04FF, RID_SVXSTR_CYRILLIC);
    allSubsets[ i++] = new Subset( 0x0530, 0x058F, RID_SVXSTR_ARMENIAN);
    allSubsets[ i++] = new Subset( 0x0590, 0x05FF, RID_SVXSTR_BASIC_HEBREW);
//  allSubsets[ i++] = new Subset( 0x0591, 0x05C4, RID_SVXSTR_HEBREW_EXTENDED);
    allSubsets[ i++] = new Subset( 0x0600, 0x06FF, RID_SVXSTR_BASIC_ARABIC);
//  allSubsets[ i++] = new Subset( 0x0660, 0x06FF, RID_SVXSTR_ARABIC_EXTENDED);
    allSubsets[ i++] = new Subset( 0x0700, 0x074F, RID_SVXSTR_SYRIAC);
    allSubsets[ i++] = new Subset( 0x0780, 0x07BF, RID_SVXSTR_THAANA);

    allSubsets[ i++] = new Subset( 0x0900, 0x097F, RID_SVXSTR_DEVANAGARI);
    allSubsets[ i++] = new Subset( 0x0980, 0x09FF, RID_SVXSTR_BENGALI);
    allSubsets[ i++] = new Subset( 0x0A00, 0x0A7F, RID_SVXSTR_GURMUKHI);
    allSubsets[ i++] = new Subset( 0x0A80, 0x0AFF, RID_SVXSTR_GUJARATI);
    allSubsets[ i++] = new Subset( 0x0B00, 0x0B7F, RID_SVXSTR_ORIYA);
    allSubsets[ i++] = new Subset( 0x0B80, 0x0BFF, RID_SVXSTR_TAMIL);
    allSubsets[ i++] = new Subset( 0x0C00, 0x0C7F, RID_SVXSTR_TELUGU);
    allSubsets[ i++] = new Subset( 0x0C80, 0x0CFF, RID_SVXSTR_KANNADA);
    allSubsets[ i++] = new Subset( 0x0D00, 0x0D7F, RID_SVXSTR_MALAYALAM);
    allSubsets[ i++] = new Subset( 0x0D80, 0x0DFF, RID_SVXSTR_SINHALA);
    allSubsets[ i++] = new Subset( 0x0E00, 0x0E7F, RID_SVXSTR_THAI);
    allSubsets[ i++] = new Subset( 0x0E80, 0x0EFF, RID_SVXSTR_LAO);
    allSubsets[ i++] = new Subset( 0x0F00, 0x0FBF, RID_SVXSTR_TIBETAN);
    allSubsets[ i++] = new Subset( 0x1000, 0x109F, RID_SVXSTR_MYANMAR);
    allSubsets[ i++] = new Subset( 0x10A0, 0x10FF, RID_SVXSTR_BASIC_GEORGIAN);
//  allSubsets[ i++] = new Subset( 0x10A0, 0x10C5, RID_SVXSTR_GEORGIAN_EXTENDED);

    allSubsets[ i++] = new Subset( 0x1100, 0x11FF, RID_SVXSTR_HANGUL_JAMO);
    allSubsets[ i++] = new Subset( 0x1200, 0x137F, RID_SVXSTR_ETHIOPIC);
    allSubsets[ i++] = new Subset( 0x13A0, 0x13FF, RID_SVXSTR_CHEROKEE);
    allSubsets[ i++] = new Subset( 0x1400, 0x167F, RID_SVXSTR_CANADIAN_ABORIGINAL);
    allSubsets[ i++] = new Subset( 0x1680, 0x169F, RID_SVXSTR_OGHAM);
    allSubsets[ i++] = new Subset( 0x16A0, 0x16F0, RID_SVXSTR_RUNIC);

    allSubsets[ i++] = new Subset( 0x1780, 0x17FF, RID_SVXSTR_KHMER);
    allSubsets[ i++] = new Subset( 0x1800, 0x18AF, RID_SVXSTR_MONGOLIAN);
    allSubsets[ i++] = new Subset( 0x1E00, 0x1EFF, RID_SVXSTR_LATIN_EXTENDED_ADDS);
    allSubsets[ i++] = new Subset( 0x1F00, 0x1FFF, RID_SVXSTR_GREEK_EXTENDED);

    allSubsets[ i++] = new Subset( 0x2000, 0x206F, RID_SVXSTR_GENERAL_PUNCTUATION);
    allSubsets[ i++] = new Subset( 0x2070, 0x209F, RID_SVXSTR_SUB_SUPER_SCRIPTS);
    allSubsets[ i++] = new Subset( 0x20A0, 0x20CF, RID_SVXSTR_CURRENCY_SYMBOLS);
    allSubsets[ i++] = new Subset( 0x20D0, 0x20FF, RID_SVXSTR_COMB_DIACRITIC_SYMS);
    allSubsets[ i++] = new Subset( 0x2100, 0x214F, RID_SVXSTR_LETTERLIKE_SYMBOLS);
    allSubsets[ i++] = new Subset( 0x2150, 0x218F, RID_SVXSTR_NUMBER_FORMS);
    allSubsets[ i++] = new Subset( 0x2190, 0x21FF, RID_SVXSTR_ARROWS);
    allSubsets[ i++] = new Subset( 0x2200, 0x22FF, RID_SVXSTR_MATH_OPERATORS);
    allSubsets[ i++] = new Subset( 0x2300, 0x23FF, RID_SVXSTR_MISC_TECHNICAL);
    allSubsets[ i++] = new Subset( 0x2400, 0x243F, RID_SVXSTR_CONTROL_PICTURES);
    allSubsets[ i++] = new Subset( 0x2440, 0x245F, RID_SVXSTR_OPTICAL_CHAR_REC);
    allSubsets[ i++] = new Subset( 0x2460, 0x24FF, RID_SVXSTR_ENCLOSED_ALPHANUM);
    allSubsets[ i++] = new Subset( 0x2500, 0x257F, RID_SVXSTR_BOX_DRAWING);
    allSubsets[ i++] = new Subset( 0x2580, 0x259F, RID_SVXSTR_BLOCK_ELEMENTS);
    allSubsets[ i++] = new Subset( 0x25A0, 0x25FF, RID_SVXSTR_GEOMETRIC_SHAPES);
    allSubsets[ i++] = new Subset( 0x2600, 0x26FF, RID_SVXSTR_MISC_DINGBATS);
    allSubsets[ i++] = new Subset( 0x2700, 0x27BF, RID_SVXSTR_DINGBATS);

    allSubsets[ i++] = new Subset( 0x3000, 0x303F, RID_SVXSTR_CJK_SYMS_PUNCTUATION);
    allSubsets[ i++] = new Subset( 0x3040, 0x309F, RID_SVXSTR_HIRAGANA);
    allSubsets[ i++] = new Subset( 0x30A0, 0x30FF, RID_SVXSTR_KATAKANA);
    allSubsets[ i++] = new Subset( 0x3100, 0x312F, RID_SVXSTR_BOPOMOFO);
    allSubsets[ i++] = new Subset( 0x3130, 0x318F, RID_SVXSTR_HANGUL_COMPAT_JAMO);
    allSubsets[ i++] = new Subset( 0x3190, 0x31FF, RID_SVXSTR_CJK_MISC);
    allSubsets[ i++] = new Subset( 0x3200, 0x32FF, RID_SVXSTR_ENCLOSED_CJK_LETTERS);
    allSubsets[ i++] = new Subset( 0x3300, 0x33FF, RID_SVXSTR_CJK_COMPATIBILITY);

    allSubsets[ i++] = new Subset( 0x3400, 0x4DFF, RID_SVXSTR_CJK_UNIFIED_IDGRAPH_EXT_A);
    allSubsets[ i++] = new Subset( 0x4E00, 0x9FA5, RID_SVXSTR_CJK_UNIFIED_IDGRAPH);
    allSubsets[ i++] = new Subset( 0xA000, 0xA4CF, RID_SVXSTR_YI);
    allSubsets[ i++] = new Subset( 0xAC00, 0xB097, RID_SVXSTR_HANGUL_GA);
    allSubsets[ i++] = new Subset( 0xB098, 0xB2E3, RID_SVXSTR_HANGUL_NA);
    allSubsets[ i++] = new Subset( 0xB2E4, 0xB77B, RID_SVXSTR_HANGUL_DA);
    allSubsets[ i++] = new Subset( 0xB77C, 0xB9C7, RID_SVXSTR_HANGUL_RA);
    allSubsets[ i++] = new Subset( 0xB9C8, 0xBC13, RID_SVXSTR_HANGUL_MA);
    allSubsets[ i++] = new Subset( 0xBC14, 0xC0AB, RID_SVXSTR_HANGUL_BA);
    allSubsets[ i++] = new Subset( 0xC0AC, 0xC543, RID_SVXSTR_HANGUL_SA);
    allSubsets[ i++] = new Subset( 0xC544, 0xC78F, RID_SVXSTR_HANGUL_AH);
    allSubsets[ i++] = new Subset( 0xC790, 0xCC27, RID_SVXSTR_HANGUL_JA);
    allSubsets[ i++] = new Subset( 0xCC28, 0xCE73, RID_SVXSTR_HANGUL_CHA);
    allSubsets[ i++] = new Subset( 0xCE74, 0xD0BF, RID_SVXSTR_HANGUL_KA);
    allSubsets[ i++] = new Subset( 0xD0C0, 0xD30B, RID_SVXSTR_HANGUL_TA);
    allSubsets[ i++] = new Subset( 0xD30C, 0xD557, RID_SVXSTR_HANGUL_PA);
    allSubsets[ i++] = new Subset( 0xD558, 0xD7A3, RID_SVXSTR_HANGUL_HA);
//  allSubsets[ i++] = new Subset( 0xAC00, 0xD7A3, RID_SVXSTR_HANGUL);

    allSubsets[ i++] = new Subset( 0xE000, 0xF8FF, RID_SVXSTR_PRIVATE_USE_AREA);
    allSubsets[ i++] = new Subset( 0xF900, 0xFAFF, RID_SVXSTR_CJK_COMPAT_IDGRAPHS);
    allSubsets[ i++] = new Subset( 0xFB00, 0xFB4F, RID_SVXSTR_ALPHA_PRESENTATION);
    allSubsets[ i++] = new Subset( 0xFB50, 0xFDFF, RID_SVXSTR_ARABIC_PRESENT_A);
    allSubsets[ i++] = new Subset( 0xFE20, 0xFE2F, RID_SVXSTR_COMBINING_HALF_MARKS);
    allSubsets[ i++] = new Subset( 0xFE30, 0xFE4F, RID_SVXSTR_CJK_COMPAT_FORMS);
    allSubsets[ i++] = new Subset( 0xFE50, 0xFE6F, RID_SVXSTR_SMALL_FORM_VARIANTS);
    allSubsets[ i++] = new Subset( 0xFE70, 0xFEFF, RID_SVXSTR_ARABIC_PRESENT_B);
    allSubsets[ i++] = new Subset( 0xFF00, 0xFFEF, RID_SVXSTR_HALFW_FULLW_FORMS);
    allSubsets[ i++] = new Subset( 0xFFF0, 0xFFFF, RID_SVXSTR_SPECIALS);

    numAllSubsets = i;
    DBG_ASSERT( (numAllSubsets < SVXCOUNT_CHARRANGE), "SVXCOUNT_CHARRANGE too small");
    DBG_ASSERT( (2*numAllSubsets > SVXCOUNT_CHARRANGE), "SVXCOUNT_CHARRANGE way to big");
}
