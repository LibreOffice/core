/*************************************************************************
 *
 *  $RCSfile: charmap.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:07 $
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

// class SvxShowCharSet --------------------------------------------------

SvxShowCharSet::SvxShowCharSet( Window* pParent, const ResId& rResId ) :

    Control( pParent, rResId )

{
    Size aOutputSize = GetOutputSizePixel();
    c  = 32;
    nX = aOutputSize.Width()  / COLUMN_COUNT;
    nY = aOutputSize.Height() / ROW_COUNT;
    bDrag = FALSE;
    SetOutputSizePixel( Size( nX * COLUMN_COUNT, nY * ROW_COUNT ) );
    InitSettings( TRUE, TRUE );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::GetFocus()
{
    Control::GetFocus();
    SelectCharacter( c, TRUE );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::LoseFocus()
{
    Control::LoseFocus();
    SelectCharacter( c, TRUE );
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
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
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

            long n = (rMEvt.GetPosPixel().Y() / nY) * COLUMN_COUNT +
                     (rMEvt.GetPosPixel().X() / nX) + 32;
            SelectCharacter( (sal_Unicode)Min( n, 255L ) );
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
        // Mouse ueber der Char-Auswahl losgelassen
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

        long n = (aPos.Y() / nY) * COLUMN_COUNT +
                   (aPos.X() / nX) + 32;
        SelectCharacter( (sal_Unicode)Min( n, 255L ) );
    }
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

    if ( cChar > 32 )
    {
        SelectCharacter( cChar );
        aPreSelectHdl.Call( this );
        return;
    }

    FASTBOOL bPreSelect = TRUE;

    switch ( aCode.GetCode() )
    {
        case KEY_SPACE:
            aSelectHdl.Call( this );
            break;
        case KEY_LEFT:
            if ( c > 32 )
                SelectCharacter( c-1 );
            break;
        case KEY_RIGHT:
            if ( c < 255 )
                SelectCharacter( c+1 );
            break;
        case KEY_UP:
            if ( c-COLUMN_COUNT >= 32 )
                SelectCharacter( c-COLUMN_COUNT );
            break;
        case KEY_DOWN:
            if ( c+COLUMN_COUNT <= 255 )
                SelectCharacter( c+COLUMN_COUNT );
            break;
        case KEY_HOME:
            SelectCharacter( 32 );
            break;
        case KEY_END:
            SelectCharacter( 255 );
            break;
        default:
            bPreSelect = FALSE;
            Control::KeyInput( rKEvt );
    }

    if ( bPreSelect )
        aPreSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::Paint( const Rectangle& )
{
    DrawChars_Impl( 32, 256 );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::DrawChars_Impl( USHORT n1, USHORT n2 )
{
    USHORT  i;
    long    x, y;
    Size    aOutputSize;
    String  aCharStr;

    aOutputSize = GetOutputSizePixel();

    for ( i = 1; i < COLUMN_COUNT; i++ )
        DrawLine( Point( nX * i, 0 ), Point( nX * i, aOutputSize.Height() ) );
    for ( i = 1; i < ROW_COUNT; i++ )
        DrawLine( Point( 0, nY * i ), Point( aOutputSize.Width(), nY * i ) );

    for ( i = n1; i < n2; i++ )
    {
        x = ((i-32) % COLUMN_COUNT) * nX;
        y = ((i-32) / COLUMN_COUNT) * nY;

        if ( c == sal_Unicode(i) && HasFocus() )
        {
            const StyleSettings& rStyleSettings =
                Application::GetSettings().GetStyleSettings();

            Color aLineCol = GetLineColor();
            Color aFillCol = GetFillColor();
            SetLineColor();
            SetFillColor( rStyleSettings.GetFaceColor() );
            DrawRect( Rectangle( Point( x + 1, y + 1),
                                 Size( nX - 1, nY - 1 ) ) );
            SetLineColor( rStyleSettings.GetLightColor() );
            DrawLine( Point( x + 1, y + 1 ), Point( x + nX - 1, y + 1 ) );
            DrawLine( Point( x + 1, y + 1 ), Point( x + 1, y + nY - 1 ) );
            SetLineColor( rStyleSettings.GetShadowColor() );
            DrawLine( Point( x + 1, y + nY - 1 ),
                      Point( x + nX - 1, y + nY - 1 ) );
            DrawLine( Point( x + nX - 1, y + nY - 1 ),
                      Point( x + nX - 1, y + 1 ) );
            SetLineColor( aLineCol );
            SetFillColor( aFillCol );
        }

        if ( i > 0 )
            aCharStr = String( (sal_Unicode)i );
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

void SvxShowCharSet::SetFont( const Font& rFont )
{
    Font aFont = rFont;

    Invalidate();

    aFont.SetWeight( WEIGHT_LIGHT );
    aFont.SetSize( Size( 0, 12 ) );
    aFont.SetTransparent( TRUE );
    Control::SetFont( aFont );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::SelectCharacter( sal_Unicode cNew, BOOL bFocus )
{
    if ( (c == cNew) && !bFocus )
        return;

    USHORT n = c;

    // Highlight-Darstellung entfernen
    long x = ((n-32) % COLUMN_COUNT) * nX;
    long y = ((n-32) / COLUMN_COUNT) * nY;

    Color aLineCol = GetLineColor();
    Color aFillCol = GetFillColor();
    SetLineColor();
    SetFillColor( GetBackground().GetColor() );
    DrawRect( Rectangle( Point( x + 1, y + 1), Size( nX - 1, nY - 1 ) ) );
    SetLineColor( aLineCol );
    SetFillColor( aFillCol );

    c = cNew;

    DrawChars_Impl( n, n+1 );

    n = c;
    DrawChars_Impl( n, n+1 );

    aHighHdl.Call( this );
}

// -----------------------------------------------------------------------

SvxShowCharSet::~SvxShowCharSet()
{
}

// class SvxShowText -----------------------------------------------------

SvxShowText::SvxShowText(Window* pParent, const ResId& rResId, BOOL bCenter) :

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
        DrawText( Point( (GetOutputSizePixel().Width() - GetTextWidth(aText)) / 2,
                         nY ),
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

    // SystemFont erst mal einstellen
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
    // ggf. ist der Name des Fonts in der Liste nicht enthalten ->
    // dann versuchen einen Token des Namens zu selektieren oder sonst
    // den ersten Eintrag.
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
    aShowSet.SetDoubleClickHdl( LINK( this, SvxCharacterMap, CharDoubleClickHdl ) );
    aShowSet.SetSelectHdl( LINK( this, SvxCharacterMap, CharSelectHdl ) );
    aShowSet.SetHighlightHdl( LINK( this, SvxCharacterMap, CharHighlightHdl ) );
    aShowSet.SetPreSelectHdl( LINK( this, SvxCharacterMap, CharPreSelectHdl ) );
    aDeleteBtn.SetClickHdl( LINK( this, SvxCharacterMap, DeleteHdl ) );

    SetChar( 32 );
    aOKBtn.Disable();
}

// -----------------------------------------------------------------------

void SvxCharacterMap::SetFont( const Font& rFont )
{
    SetCharFont( rFont );
}

// -----------------------------------------------------------------------

/*  [Beschreibung]

    Diese Methode wird gerufen, um die Font-Auswahl zu disablen.
*/

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

    // aus Kompatiblitaetsgruenden noch drin
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
    aShowSet.SetFont( aFont );
    aShowText.SetFont( aFont );
    aShowChar.SetFont( aFont );
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
    sal_Bool bSelect = ( aShowSet.GetSelectCharacter() > 0 );
    if ( bSelect )
        aTemp = String( aShowSet.GetSelectCharacter() );
    aShowChar.SetText( aTemp );
    aShowChar.Update();
    if ( bSelect )
        aTemp = String::CreateFromInt32( (USHORT)aShowSet.GetSelectCharacter() );
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


