/*************************************************************************
 *
 *  $RCSfile: edit.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mt $ $Date: 2000-11-08 09:19:34 $
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

#define _SV_EDIT_CXX

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_CURSOR_HXX
#include <cursor.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_CLIP_HXX
#include <clip.hxx>
#endif
#ifndef _SV_DRAG_HXX
#include <drag.hxx>
#endif
#ifndef _SV_TRANSFER_HXX
#include <transfer.hxx>
#endif
#ifndef _SV_SVIDS_HRC
#include <svids.hrc>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#ifndef _VCL_CMDEVT_H
#include <cmdevt.h>
#endif
#ifndef _SV_SUBEDIT_HXX
#include <subedit.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <edit.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XBREAKITERATOR_HPP_
#include <com/sun/star/text/XBreakIterator.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_CHARACTERITERATORMODE_HPP_
#include <com/sun/star/text/CharacterIteratorMode.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_WORDTYPE_HPP_
#include <com/sun/star/text/WordType.hpp>
#endif

#include <unohelp.hxx>


#pragma hdrstop

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

// - Redo
// - Bei Tracking-Cancel DefaultSelection wieder herstellen

// =======================================================================

static FncGetSpecialChars pImplFncGetSpecialChars = NULL;

// =======================================================================

#define EDIT_ALIGN_LEFT             1
#define EDIT_ALIGN_CENTER           2
#define EDIT_ALIGN_RIGHT            3

#define EDIT_DEL_LEFT               1
#define EDIT_DEL_RIGHT              2

#define EDIT_DELMODE_SIMPLE         11
#define EDIT_DELMODE_RESTOFWORD     12
#define EDIT_DELMODE_RESTOFCONTENT  13

// =======================================================================

uno::Reference < text::XBreakIterator > ImplGetBreakIterator()
{
    static uno::Reference < text::XBreakIterator > xB;
    if ( !xB.is() )
        xB = vcl::unohelper::CreateBreakIterator();
    return xB;
}

// =======================================================================

struct DDInfo
{
    Cursor          aCursor;
    xub_StrLen      nDropPos;
    BOOL            bStarterOfDD;
    BOOL            bDroppedInMe;
    BOOL            bVisCursor;

    DDInfo()
    {
        aCursor.SetStyle( CURSOR_SHADOW );
        nDropPos = 0;
        bStarterOfDD = FALSE;
        bDroppedInMe = FALSE;
        bVisCursor = FALSE;
    }
};

// =======================================================================

struct Impl_IMEInfos
{
    USHORT*     pAttribs;
    xub_StrLen  nPos;
    xub_StrLen  nLen;
    BOOL        bCursor;
    BOOL        bWasCursorOverwrite;

                Impl_IMEInfos( xub_StrLen nPos );
                ~Impl_IMEInfos();

    void        CopyAttribs( const xub_StrLen* pA, xub_StrLen nL );
    void        DestroyAttribs();
};

// -----------------------------------------------------------------------

Impl_IMEInfos::Impl_IMEInfos( xub_StrLen nP )
{
    nPos = nP;
    nLen = 0;
    bCursor = TRUE;
    pAttribs = NULL;
    bWasCursorOverwrite = FALSE;
}

// -----------------------------------------------------------------------

Impl_IMEInfos::~Impl_IMEInfos()
{
    delete pAttribs;
}

// -----------------------------------------------------------------------

void Impl_IMEInfos::CopyAttribs( const xub_StrLen* pA, xub_StrLen nL )
{
    nLen = nL;
    delete pAttribs;
    pAttribs = new USHORT[ nL ];
    memcpy( pAttribs, pA, nL*sizeof(USHORT) );
}

// -----------------------------------------------------------------------

void Impl_IMEInfos::DestroyAttribs()
{
    delete pAttribs;
    pAttribs = NULL;
}

// =======================================================================

Edit::Edit( WindowType nType ) :
    Control( nType )
{
    ImplInitData();
}

// -----------------------------------------------------------------------

Edit::Edit( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_EDIT )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

Edit::Edit( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_EDIT )
{
    ImplInitData();
    rResId.SetRT( RSC_EDIT );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );
    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

Edit::~Edit()
{
    delete mpDDInfo;
    Cursor* pCursor = GetCursor();
    if ( pCursor )
    {
        SetCursor( NULL );
        delete pCursor;
    }

    delete mpIMEInfos;

    if ( mpUpdateDataTimer )
        delete mpUpdateDataTimer;
}

// -----------------------------------------------------------------------

void Edit::ImplInitData()
{
    mpSubEdit               = NULL;
    mpUpdateDataTimer       = NULL;
    mnXOffset               = 0;
    mnAlign                 = EDIT_ALIGN_LEFT;
    mnMaxTextLen            = EDIT_NOLIMIT;
    meAutocompleteAction    = AUTOCOMPLETE_KEYINPUT;
    mbModified              = FALSE;
    mbInternModified        = FALSE;
    mbReadOnly              = FALSE;
    mbInsertMode            = TRUE;
    mbClickedInSelection    = FALSE;
    mbActivePopup           = FALSE;
    mbIsSubEdit             = FALSE;
    mbInMBDown              = FALSE;
    mpDDInfo                = NULL;
    mpIMEInfos              = NULL;
    mcEchoChar              = 0;
}

// -----------------------------------------------------------------------

void Edit::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    if ( !(nStyle & (WB_CENTER | WB_RIGHT)) )
        nStyle |= WB_LEFT;

    Control::ImplInit( pParent, nStyle, NULL );

    mbReadOnly = (nStyle & WB_READONLY) != 0;

    mnAlign = EDIT_ALIGN_LEFT;
    if ( nStyle & WB_RIGHT )
        mnAlign = EDIT_ALIGN_RIGHT;
    else if ( nStyle & WB_CENTER )
        mnAlign = EDIT_ALIGN_CENTER;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( Wallpaper( rStyleSettings.GetFieldColor() ) );
    SetFillColor( rStyleSettings.GetFieldColor() );
    SetCursor( new Cursor );

    SetPointer( Pointer( POINTER_TEXT ) );
    ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

WinBits Edit::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;

    return nStyle;
}

// -----------------------------------------------------------------------

BOOL Edit::IsCharInput( const KeyEvent& rKeyEvent )
{
    // In the future we must use new Unicode functions for this
    xub_Unicode cCharCode = rKeyEvent.GetCharCode();
    return ((cCharCode >= 32) && (cCharCode != 127) &&
            !rKeyEvent.GetKeyCode().IsControlMod());
}

// -----------------------------------------------------------------------

void Edit::ImplModified()
{
    mbModified = TRUE;
    Modify();
}

// -----------------------------------------------------------------------

void Edit::ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetFieldFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
    }

    if ( bFont || bForeground )
    {
        Color aTextColor = rStyleSettings.GetFieldTextColor();
        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if( IsControlBackground() )
        {
            SetBackground( GetControlBackground() );
            SetFillColor( GetControlBackground() );
        }
        else
        {
            SetBackground( rStyleSettings.GetFieldColor() );
            SetFillColor( rStyleSettings.GetFieldColor() );
        }
    }
}

// -----------------------------------------------------------------------

XubString Edit::ImplGetText() const
{
    if ( mcEchoChar || (GetStyle() & WB_PASSWORD) )
    {
        XubString   aText;
        xub_Unicode cEchoChar;
        if ( mcEchoChar )
            cEchoChar = mcEchoChar;
        else
            cEchoChar = '*';
        aText.Fill( maText.Len(), cEchoChar );
        return aText;
    }
    else
        return maText;
}

// -----------------------------------------------------------------------

void Edit::ImplRepaint( xub_StrLen nStart, xub_StrLen nEnd )
{
    if ( !IsReallyVisible() )
        return;

    XubString aText = ImplGetText();
    if ( nStart >= aText.Len() )
        return;

    if ( nEnd > aText.Len() )
        nEnd = aText.Len();

    Cursor* pCursor = GetCursor();
    BOOL bVisCursor = pCursor ? pCursor->IsVisible() : FALSE;
    if ( pCursor )
        pCursor->Hide();

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if ( IsEnabled() )
        ImplInitSettings( FALSE, TRUE, FALSE );
    else
        SetTextColor( rStyleSettings.GetDisableColor() );

    SetTextFillColor( IsControlBackground() ? GetControlBackground() : rStyleSettings.GetFieldColor() );

    // In der Hoehe zentrieren
    long    nH = GetOutputSize().Height();
    long    nTH = GetTextHeight();
    Point   aPos( mnXOffset, (nH-nTH)/2 );

    BOOL bDrawSelection = maSelection.Len() && ( HasFocus() || ( GetStyle() & WB_NOHIDESELECTION ) || mbActivePopup );

    if ( !bDrawSelection && !mpIMEInfos )
    {
        aPos.X() = GetTextWidth( aText, 0, nStart ) + mnXOffset;
        DrawText( aPos, aText, nStart, nEnd - nStart );
    }
    else
    {
        Selection aTmpSel( maSelection );
        aTmpSel.Justify();

        xub_StrLen nIndex = nStart;
        while ( nIndex < nEnd )
        {
            xub_StrLen nTmpEnd = nEnd;
            USHORT      nAttr = 0;
            if ( mpIMEInfos && mpIMEInfos->pAttribs )
            {
                xub_StrLen nIMEEnd = mpIMEInfos->nPos+mpIMEInfos->nLen;
                if ( (nIndex < mpIMEInfos->nPos) && (nTmpEnd > mpIMEInfos->nPos) )
                {
                    nTmpEnd = mpIMEInfos->nPos;
                }
                else if ( (nIndex >= mpIMEInfos->nPos) && (nIndex < nIMEEnd) )
                {
                    // Attributweise ausgeben...
                    nTmpEnd = nIndex + 1;
                    if ( (nIndex >= mpIMEInfos->nPos) && (nIndex < (mpIMEInfos->nPos+mpIMEInfos->nLen)) )
                    {
                        nAttr = mpIMEInfos->pAttribs[nIndex-mpIMEInfos->nPos];
                        xub_StrLen nMax = mpIMEInfos->nPos+mpIMEInfos->nLen;
                        while ( ( nTmpEnd < nMax ) && ( mpIMEInfos->pAttribs[ nTmpEnd - mpIMEInfos->nPos ] == nAttr ) )
                            nTmpEnd++;
                    }
                }
            }
            else if ( bDrawSelection )
            {
                if ( ( nIndex < aTmpSel.Min() ) && ( nTmpEnd > aTmpSel.Min() ) )
                    nTmpEnd = (xub_StrLen)aTmpSel.Min();
                else if ( ( nIndex >= aTmpSel.Min() ) && ( nIndex < aTmpSel.Max() ) && ( nTmpEnd > aTmpSel.Max() ) )
                    nTmpEnd = (xub_StrLen)aTmpSel.Max();
            }

            aPos.X() = GetTextWidth( aText, 0, nIndex ) + mnXOffset;
            ImplInitSettings( mpIMEInfos ? TRUE : FALSE, TRUE, FALSE );
            BOOL bSelected = bDrawSelection && ((xub_StrLen)aTmpSel.Min() <= nIndex ) && ((xub_StrLen)aTmpSel.Max() > nIndex );
            if ( bSelected || ( nAttr & EXTTEXTINPUT_ATTR_HIGHLIGHT) )
            {
                SetTextColor( rStyleSettings.GetHighlightTextColor() );
                SetTextFillColor( rStyleSettings.GetHighlightColor() );
            }
            else
            {
                SetTextFillColor( IsControlBackground() ? GetControlBackground() : rStyleSettings.GetFieldColor() );
            }

            if ( nAttr )
            {
                Font aFont = GetFont();
                if ( nAttr & EXTTEXTINPUT_ATTR_UNDERLINE )
                    aFont.SetUnderline( UNDERLINE_SINGLE );
                else if ( nAttr & EXTTEXTINPUT_ATTR_BOLDUNDERLINE )
                    aFont.SetUnderline( UNDERLINE_BOLD );
                else if ( nAttr & EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE )
                    aFont.SetUnderline( UNDERLINE_DOTTED );
                else if ( nAttr & EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE )
                    aFont.SetUnderline( UNDERLINE_DOTTED );
                else if ( nAttr & EXTTEXTINPUT_ATTR_GRAYWAVELINE )
                {
                    aFont.SetUnderline( UNDERLINE_WAVE );
                    SetTextLineColor( Color( COL_LIGHTGRAY ) );
                }
                SetFont( aFont );

                if ( nAttr & EXTTEXTINPUT_ATTR_REDTEXT )
                    SetTextColor( Color( COL_RED ) );
                else if ( nAttr & EXTTEXTINPUT_ATTR_HALFTONETEXT )
                    SetTextColor( Color( COL_LIGHTGRAY ) );
            }
            DrawText( aPos, aText, nIndex, nTmpEnd - nIndex );
            nIndex = nTmpEnd;
        }
    }

    if ( bVisCursor && ( !mpIMEInfos || mpIMEInfos->bCursor ) )
        pCursor->Show();
}

// -----------------------------------------------------------------------

void Edit::ImplDelete( const Selection& rSelection, BYTE nDirection, BYTE nMode )
{
    XubString aText = ImplGetText();

    // loeschen moeglich?
    if ( !rSelection.Len() &&
         (((rSelection.Min() == 0) && (nDirection == EDIT_DEL_LEFT)) ||
          ((rSelection.Max() == aText.Len()) && (nDirection == EDIT_DEL_RIGHT))) )
        return;

    long nOldWidth = GetTextWidth( aText );
    Selection aSelection( rSelection );
    aSelection.Justify();

    if ( !aSelection.Len() )
    {
        uno::Reference < text::XBreakIterator > xBI = ImplGetBreakIterator();
        if ( nDirection == EDIT_DEL_LEFT )
        {
            if ( nMode == EDIT_DELMODE_RESTOFWORD )
            {
                text::Boundary aBoundary = xBI->getWordBoundary( maText, aSelection.Min(), GetSettings().GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
                if ( aBoundary.startPos == aSelection.Min() )
                    aBoundary = xBI->previousWord( maText, aSelection.Min(), GetSettings().GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES );
                aSelection.Min() = aBoundary.startPos;
            }
            else if ( nMode == EDIT_DELMODE_RESTOFCONTENT )
               {
                aSelection.Min() = 0;
            }
            else
            {
                sal_Int32 nCount = 1;
                aSelection.Min() = xBI->previousCharacters( maText, aSelection.Min(), GetSettings().GetLocale(), text::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
            }
        }
        else
        {
            if ( nMode == EDIT_DELMODE_RESTOFWORD )
            {
                text::Boundary aBoundary = xBI->nextWord( maText, aSelection.Max(), GetSettings().GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES );
                aSelection.Max() = aBoundary.startPos;
            }
            else if ( nMode == EDIT_DELMODE_RESTOFCONTENT )
            {
                aSelection.Max() = aText.Len();
            }
            else
            {
                sal_Int32 nCount = 1;
                aSelection.Max() = xBI->nextCharacters( maText, aSelection.Max(), GetSettings().GetLocale(), text::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );;
            }
        }
    }

    maText.Erase( (xub_StrLen)aSelection.Min(), (xub_StrLen)aSelection.Len() );
    maSelection.Min() = aSelection.Min();
    maSelection.Max() = aSelection.Min();
    ImplAlignAndPaint( (xub_StrLen)aSelection.Min(), nOldWidth );
    mbInternModified = TRUE;
}

// -----------------------------------------------------------------------

void Edit::ImplInsertText( const XubString& rStr, const Selection* pNewSel )
{
    Selection aSelection( maSelection );
    aSelection.Justify();

    XubString aNewText( rStr );
    aNewText.EraseAllChars( _LF );
    aNewText.EraseAllChars( _CR );
    aNewText.SearchAndReplaceAll( '\t', ' ' );

    if ( (maText.Len() + aNewText.Len() - aSelection.Len()) > mnMaxTextLen )
        return;

    long nOldWidth = GetTextWidth( ImplGetText() );

    if ( aSelection.Len() )
        maText.Erase( (xub_StrLen)aSelection.Min(), (xub_StrLen)aSelection.Len() );
    else if ( !mbInsertMode && (aSelection.Max() < maText.Len()) )
        maText.Erase( (xub_StrLen)aSelection.Max(), 1 );

    if ( aNewText.Len() )
        maText.Insert( aNewText, (xub_StrLen)aSelection.Min() );

    if ( !pNewSel )
    {
        maSelection.Min() = aSelection.Min() + aNewText.Len();
        maSelection.Max() = maSelection.Min();
    }
    else
    {
        maSelection = *pNewSel;
        if ( maSelection.Min() > maText.Len() )
            maSelection.Min() = maText.Len();
        if ( maSelection.Max() > maText.Len() )
            maSelection.Max() = maText.Len();
    }

    ImplAlignAndPaint( (xub_StrLen)aSelection.Min(), nOldWidth );
    mbInternModified = TRUE;
}

// -----------------------------------------------------------------------

void Edit::ImplSetText( const XubString& rText, const Selection* pNewSelection )
{
    // Der Text wird dadurch geloescht das der alte Text komplett 'selektiert'
    // wird, dann InsertText, damit flackerfrei.
    if ( (rText != maText) || (pNewSelection && (*pNewSelection != maSelection)) )
    {
        maSelection.Min() = 0;
        maSelection.Max() = maText.Len();
        if ( mnXOffset || HasPaintEvent() )
        {
            mnXOffset = 0;
            maText = rText;

            if ( pNewSelection )
                ImplSetSelection( *pNewSelection, FALSE );

            if ( mnXOffset && !pNewSelection )
                maSelection.Max() = 0;

            ImplAlign();
            Invalidate();
        }
        else
            ImplInsertText( rText, pNewSelection );
    }
}

// -----------------------------------------------------------------------

void Edit::ImplClearBackground( long nXStart, long nXEnd )
{
    Point aTmpPoint;
    Rectangle aRect( aTmpPoint, GetOutputSizePixel() );
    aRect.Left() = nXStart;
    aRect.Right() = nXEnd;

    Cursor* pCursor = HasFocus() ? GetCursor() : NULL;

    if ( pCursor )
        pCursor->Hide();

    Erase( aRect );

    if ( pCursor )
        pCursor->Show();
}

// -----------------------------------------------------------------------

void Edit::ImplShowCursor( BOOL bOnlyIfVisible )
{
    if ( !IsUpdateMode() || ( bOnlyIfVisible && !IsReallyVisible() ) )
        return;

    Cursor*     pCursor = GetCursor();
    XubString   aText = ImplGetText();
    long        nTextWidth = GetTextWidth( aText, 0, (xub_StrLen)maSelection.Max() );

    long nCursorWidth = 0;
    if ( !mbInsertMode && !maSelection.Len() && (maSelection.Max() < aText.Len()) )
        nCursorWidth = GetTextWidth( aText, (xub_StrLen)maSelection.Max(), 1 );
    long nCursorPosX = nTextWidth + mnXOffset;

    // Cursor muss im sichtbaren Bereich landen:
    Size aOutSize = GetOutputSizePixel();
    if ( (nCursorPosX < 0) || (nCursorPosX >= aOutSize.Width()) )
    {
        long nOldXOffset = mnXOffset;

        if ( nCursorPosX < 0 )
        {
            mnXOffset = - nTextWidth;
            long nMaxX = 0;
            mnXOffset += aOutSize.Width() / 5;
            if ( mnXOffset > nMaxX )
                mnXOffset = nMaxX;
        }
        else
        {
            mnXOffset = aOutSize.Width() - nTextWidth;
            // Etwas mehr?
            if ( aOutSize.Width() < nTextWidth )
            {
                long nMaxNegX = aOutSize.Width() - GetTextWidth( aText );
                mnXOffset -= aOutSize.Width() / 5;
                if ( mnXOffset < nMaxNegX )  // beides negativ...
                    mnXOffset = nMaxNegX;
            }
        }

        nCursorPosX = nTextWidth + mnXOffset;
        if ( nCursorPosX == aOutSize.Width() )  // dann nicht sichtbar...
            nCursorPosX--;

        if ( mnXOffset != nOldXOffset )
            ImplRepaint();
    }

    long nTextHeight = GetTextHeight();
    long nCursorPosY = (aOutSize.Height()-nTextHeight) / 2;
    pCursor->SetPos( Point( nCursorPosX, nCursorPosY ) );
    pCursor->SetSize( Size( nCursorWidth, nTextHeight ) );
    pCursor->Show();
}

// -----------------------------------------------------------------------

void Edit::ImplAlign()
{
    if ( mnAlign == EDIT_ALIGN_LEFT )
        return;

    long nTextWidth = GetTextWidth( ImplGetText() );
    long nOutWidth = GetOutputSizePixel().Width();

    if ( mnAlign == EDIT_ALIGN_RIGHT )
    {
        long nMinXOffset = nOutWidth - nTextWidth;
        if ( mnXOffset < nMinXOffset )
            mnXOffset = nMinXOffset;
    }
    else if( mnAlign == EDIT_ALIGN_CENTER )
    {
        // Mit Abfrage schoener, wenn gescrollt, dann aber nicht zentriert im gescrollten Zustand...
//      if ( nTextWidth < nOutWidth )
            mnXOffset = (nOutWidth - nTextWidth) / 2;
    }
}


// -----------------------------------------------------------------------

void Edit::ImplAlignAndPaint( xub_StrLen nChangedFrom, long nOldWidth )
{
    long        nNewWidth = GetTextWidth( ImplGetText() );
    xub_StrLen  nPaintStart = nChangedFrom;

    ImplAlign();
    if ( mnAlign == EDIT_ALIGN_LEFT )
    {
        if ( nOldWidth > nNewWidth )
            ImplClearBackground( nNewWidth+mnXOffset, nOldWidth+mnXOffset );
    }
    else if ( mnAlign == EDIT_ALIGN_RIGHT )
    {
        nPaintStart = 0;
        ImplClearBackground( GetOutputSizePixel().Width()-Max( nOldWidth, nNewWidth )-1, mnXOffset+1 );
    }
    else // EDIT_ALIGN_CENTER
    {
        nPaintStart = 0;
        ImplClearBackground( 0, mnXOffset + 1 );
        ImplClearBackground( mnXOffset+nNewWidth-1, GetOutputSizePixel().Width() );
    }

    ImplRepaint( nPaintStart, STRING_LEN );
    ImplShowCursor();
}

// -----------------------------------------------------------------------

xub_StrLen Edit::ImplGetCharPos( const Point& rWindowPos )
{
    return GetTextBreak( ImplGetText(), rWindowPos.X() - mnXOffset );
}

// -----------------------------------------------------------------------

void Edit::ImplSetCursorPos( xub_StrLen nChar, BOOL bSelect )
{
    Selection aSelection( maSelection );
    aSelection.Max() = nChar;
    if ( !bSelect )
        aSelection.Min() = aSelection.Max();
    ImplSetSelection( aSelection );
}

// -----------------------------------------------------------------------

void Edit::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    xub_StrLen nTextLength = ReadShortRes();
    if ( nTextLength )
        SetMaxTextLen( nTextLength );
}

// -----------------------------------------------------------------------

void Edit::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( mpSubEdit )
    {
        Control::MouseButtonDown( rMEvt );
        return;
    }

    xub_StrLen nChar = ImplGetCharPos( rMEvt.GetPosPixel() );
    Selection aSelection( maSelection );
    aSelection.Justify();

    if ( rMEvt.GetClicks() < 4 )
    {
        mbClickedInSelection = FALSE;
        if ( rMEvt.GetClicks() == 3 )
            ImplSetSelection( Selection( 0, 0xFFFF ) );
        else if ( rMEvt.GetClicks() == 2 )
        {
            uno::Reference < text::XBreakIterator > xBI = ImplGetBreakIterator();
             text::Boundary aBoundary = xBI->getWordBoundary( maText, aSelection.Max(), GetSettings().GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
            ImplSetSelection( Selection( aBoundary.startPos, aBoundary.endPos ) );
        }
        else if ( !rMEvt.IsShift() && HasFocus() && aSelection.IsInside( nChar ) )
            mbClickedInSelection = TRUE;
        else if ( rMEvt.IsLeft() )
            ImplSetCursorPos( nChar, rMEvt.IsShift() );

        if ( !mbClickedInSelection && rMEvt.IsLeft() && ( rMEvt.GetClicks() == 1 ) )
            StartTracking( STARTTRACK_SCROLLREPEAT );
    }

    mbInMBDown = TRUE;  // Dann im GetFocus nicht alles selektieren
    GrabFocus();
    mbInMBDown = FALSE;
}

// -----------------------------------------------------------------------

void Edit::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( mbClickedInSelection && rMEvt.IsLeft() )
    {
        xub_StrLen nChar = ImplGetCharPos( rMEvt.GetPosPixel() );
        ImplSetCursorPos( nChar, FALSE );
        mbClickedInSelection = FALSE;
    }
}

// -----------------------------------------------------------------------

void Edit::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( mbClickedInSelection )
        {
            xub_StrLen nChar = ImplGetCharPos( rTEvt.GetMouseEvent().GetPosPixel() );
            ImplSetCursorPos( nChar, FALSE );
            mbClickedInSelection = FALSE;
        }
    }
    else
    {
        if( !mbClickedInSelection )
        {
            xub_StrLen nChar = ImplGetCharPos( rTEvt.GetMouseEvent().GetPosPixel() );
            ImplSetCursorPos( nChar, TRUE );
        }
    }
}

// -----------------------------------------------------------------------

BOOL Edit::ImplHandleKeyEvent( const KeyEvent& rKEvt )
{
    BOOL        bDone = FALSE;
    USHORT      nCode = rKEvt.GetKeyCode().GetCode();
    KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();

    mbInternModified = FALSE;

    if ( eFunc != KEYFUNC_DONTKNOW )
    {
        switch ( eFunc )
        {
            case KEYFUNC_CUT:
            {
                if ( !mbReadOnly && maSelection.Len() && !(GetStyle() & WB_PASSWORD) )
                {
                    Cut();
                    ImplModified();
                    bDone = TRUE;
                }
            }
            break;

            case KEYFUNC_COPY:
            {
                if ( !(GetStyle() & WB_PASSWORD) )
                {
                    Copy();
                    bDone = TRUE;
                }
            }
            break;

            case KEYFUNC_PASTE:
            {
                if ( !mbReadOnly )
                {
                    Paste();
                    bDone = TRUE;
                }
            }
            break;

            case KEYFUNC_UNDO:
            {
                if ( !mbReadOnly )
                {
                    Undo();
                    bDone = TRUE;
                }
            }
            break;

            default: // wird dann evtl. unten bearbeitet.
                eFunc = KEYFUNC_DONTKNOW;
        }
    }

    if ( eFunc == KEYFUNC_DONTKNOW )
    {
        switch ( nCode )
        {
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_HOME:
            case KEY_END:
            {
                if ( !rKEvt.GetKeyCode().IsMod2() )
                {
                    uno::Reference < text::XBreakIterator > xBI = ImplGetBreakIterator();

                    Selection aSel( maSelection );
                    BOOL bWord = rKEvt.GetKeyCode().IsMod1();
                    // Range wird in ImplSetSelection geprueft...
                    if ( ( nCode == KEY_LEFT ) && aSel.Max() )
                    {
                        if ( bWord )
                        {
                            text::Boundary aBoundary = xBI->getWordBoundary( maText, aSel.Max(), GetSettings().GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
                            if ( aBoundary.startPos == aSel.Max() )
                                aBoundary = xBI->previousWord( maText, aSel.Max(), GetSettings().GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES );
                            aSel.Max() = aBoundary.startPos;
                        }
                        else
                        {
                            sal_Int32 nCount = 1;
                            aSel.Max() = xBI->previousCharacters( maText, aSel.Max(), GetSettings().GetLocale(), text::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
                        }
                    }
                    else if ( ( nCode == KEY_RIGHT ) && ( aSel.Max() < maText.Len() ) )
                    {
                        if ( bWord )
                           {
                            text::Boundary aBoundary = xBI->nextWord( maText, aSel.Max(), GetSettings().GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES );
                            aSel.Max() = aBoundary.startPos;
                        }
                        else
                        {
                            sal_Int32 nCount = 1;
                            aSel.Max() = xBI->nextCharacters( maText, aSel.Max(), GetSettings().GetLocale(), text::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
                        }
                    }
                    else if ( nCode == KEY_HOME )
                        aSel.Max() = 0;
                    else if ( nCode == KEY_END )
                        aSel.Max() = 0xFFFF;

                    if ( !rKEvt.GetKeyCode().IsShift() )
                        aSel.Min() = aSel.Max();

                    ImplSetSelection( aSel );

                    if ( (nCode == KEY_END) && maAutocompleteHdl.IsSet() && !rKEvt.GetKeyCode().GetModifier() )
                    {
                        if ( (maSelection.Min() == maSelection.Max()) && (maSelection.Min() == maText.Len()) )
                        {
                            meAutocompleteAction = AUTOCOMPLETE_KEYINPUT;
                            maAutocompleteHdl.Call( this );
                        }
                    }

                    bDone = TRUE;
                }
            }
            break;

            case KEY_BACKSPACE:
            case KEY_DELETE:
            {
                if ( !mbReadOnly && !rKEvt.GetKeyCode().IsMod2() )
                {
                    BYTE nDel = (nCode == KEY_DELETE) ? EDIT_DEL_RIGHT : EDIT_DEL_LEFT;
                    BYTE nMode = rKEvt.GetKeyCode().IsMod1() ? EDIT_DELMODE_RESTOFWORD : EDIT_DELMODE_SIMPLE;
                    if ( (nMode == EDIT_DELMODE_RESTOFWORD) && rKEvt.GetKeyCode().IsShift() )
                        nMode = EDIT_DELMODE_RESTOFCONTENT;
                    xub_StrLen nOldLen = maText.Len();
                    ImplDelete( maSelection, nDel, nMode );
                    if ( maText.Len() != nOldLen )
                        ImplModified();
                    bDone = TRUE;
                }
            }
            break;

            case KEY_INSERT:
            {
                if ( !mbReadOnly && !rKEvt.GetKeyCode().IsMod2() )
                {
                    SetInsertMode( !mbInsertMode );
                    bDone = TRUE;
                }
            }
            break;

            case KEY_TAB:
            {
                if ( !mbReadOnly && maAutocompleteHdl.IsSet() &&
                     maSelection.Min() && (maSelection.Min() == maText.Len()) &&
                     !rKEvt.GetKeyCode().IsMod1() && !rKEvt.GetKeyCode().IsMod2() )
                {
                    // Kein Autocomplete wenn alles Selektiert oder Edit leer, weil dann
                    // keine vernuenftige Tab-Steuerung!
                    if ( rKEvt.GetKeyCode().IsShift() )
                        meAutocompleteAction = AUTOCOMPLETE_TABBACKWARD;
                    else
                        meAutocompleteAction = AUTOCOMPLETE_TABFORWARD;

                    maAutocompleteHdl.Call( this );

                    // Wurde nichts veraendert, dann TAB fuer DialogControl
                    if ( GetSelection().Len() )
                        bDone = TRUE;
                }
            }
            break;

            default:
            {
                if ( IsCharInput( rKEvt ) )
                {
                    bDone = TRUE;   // Auch bei ReadOnly die Zeichen schlucken.
                    if ( !mbReadOnly )
                    {
                        ImplInsertText( rKEvt.GetCharCode() );
                        if ( maAutocompleteHdl.IsSet() )
                        {
                            if ( (maSelection.Min() == maSelection.Max()) && (maSelection.Min() == maText.Len()) )
                            {
                                meAutocompleteAction = AUTOCOMPLETE_KEYINPUT;
                                maAutocompleteHdl.Call( this );
                            }
                        }
                    }
                }
            }
        }
    }

    if ( !bDone && rKEvt.GetKeyCode().IsMod1() )
    {
        if ( nCode == KEY_A )
        {
            ImplSetSelection( Selection( 0, maText.Len() ) );
            bDone = TRUE;
        }
        else if ( rKEvt.GetKeyCode().IsShift() && (nCode == KEY_S) )
        {
            if ( pImplFncGetSpecialChars )
            {
                Selection aSaveSel = GetSelection();    // Falls jemand in Get/LoseFocus die Selektion verbiegt, z.B. URL-Zeile...
                XubString aChars = pImplFncGetSpecialChars( this, GetFont() );
                SetSelection( aSaveSel );
                if ( aChars.Len() )
                {
                    ImplInsertText( aChars );
                    ImplModified();
                }
                bDone = TRUE;
            }
        }
    }

    if ( mbInternModified )
        ImplModified();

    return bDone;
}

// -----------------------------------------------------------------------

void Edit::KeyInput( const KeyEvent& rKEvt )
{
    if ( mpSubEdit || !ImplHandleKeyEvent( rKEvt ) )
        Control::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void Edit::Paint( const Rectangle& )
{
    if ( !mpSubEdit )
        ImplRepaint();
}

// -----------------------------------------------------------------------

void Edit::Resize()
{
    if ( !mpSubEdit && IsReallyVisible() )
    {
        // Wegen vertikaler Zentrierung...
        mnXOffset = 0;
        ImplAlign();
        Invalidate();
        ImplShowCursor();
    }
}

// -----------------------------------------------------------------------

void Edit::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags )
{
    ImplInitSettings( TRUE, TRUE, TRUE );

    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );
    Font aFont = GetDrawPixelFont( pDev );
    OutDevType eOutDevType = pDev->GetOutDevType();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    pDev->SetTextFillColor();

    // Border/Background
    pDev->SetLineColor();
    pDev->SetFillColor();
    BOOL bBorder = !(nFlags & WINDOW_DRAW_NOBORDER ) && (GetStyle() & WB_BORDER);
    BOOL bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && IsControlBackground();
    if ( bBorder || bBackground )
    {
        Rectangle aRect( aPos, aSize );
        if ( bBorder )
        {
            DecorationView aDecoView( pDev );
            aRect = aDecoView.DrawFrame( aRect, FRAME_DRAW_DOUBLEIN );
        }
        if ( bBackground )
        {
            pDev->SetFillColor( GetControlBackground() );
            pDev->DrawRect( aRect );
        }
    }

    // Inhalt
    if ( ( nFlags & WINDOW_DRAW_MONO ) || ( eOutDevType == OUTDEV_PRINTER ) )
        pDev->SetTextColor( Color( COL_BLACK ) );
    else
    {
        if ( !(nFlags & WINDOW_DRAW_NODISABLE ) && !IsEnabled() )
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            pDev->SetTextColor( rStyleSettings.GetDisableColor() );
        }
        else
        {
            pDev->SetTextColor( GetTextColor() );
        }
    }

    XubString   aText = ImplGetText();
    long        nTextHeight = pDev->GetTextHeight();
    long        nTextWidth = pDev->GetTextWidth( aText );
    long        nOnePixel = GetDrawPixel( pDev, 1 );
    long        nOffX = 3*nOnePixel;
    long        nOffY = (aSize.Height() - nTextHeight) / 2;

    if ( GetStyle() & WB_CENTER )
    {
        aPos.X() += (aSize.Width() - nTextWidth) / 2;
        nOffX = 0;
    }
    else if ( GetStyle() & WB_RIGHT )
    {
        aPos.X() += aSize.Width() - nTextWidth;
        nOffX = -nOffX;
    }

    // Clipping?
    if ( (nOffY < 0) ||
         ((nOffY+nTextHeight) > aSize.Height()) ||
         ((nOffX+nTextWidth) > aSize.Width()) )
    {
        Rectangle aClip( aPos, aSize );
        if ( nTextHeight > aSize.Height() )
            aClip.Bottom() += nTextHeight-aSize.Height()+1;  // Damit HP-Drucker nicht 'weg-optimieren'
        pDev->IntersectClipRegion( aClip );
    }

    pDev->DrawText( Point( aPos.X() + nOffX, aPos.Y() + nOffY ), aText );
    pDev->Pop();
}

// -----------------------------------------------------------------------

void Edit::GetFocus()
{
    if ( mpSubEdit )
        mpSubEdit->ImplGrabFocus( GetGetFocusFlags() );
    else if ( !mbActivePopup )
    {
        maUndoText = maText;

        ULONG nSelOptions = GetSettings().GetStyleSettings().GetSelectionOptions();
        if ( !( GetStyle() & (WB_NOHIDESELECTION|WB_READONLY) )
                && ( GetGetFocusFlags() & (GETFOCUS_INIT|GETFOCUS_TAB|GETFOCUS_CURSOR|GETFOCUS_MNEMONIC) ) )
        {
            if ( nSelOptions & SELECTION_OPTION_SHOWFIRST )
            {
                maSelection.Min() = maText.Len();
                maSelection.Max() = 0;
            }
            else
            {
                maSelection.Min() = 0;
                maSelection.Max() = maText.Len();
            }
        }

        ImplShowCursor();

        if ( maSelection.Len() )
        {
            // Selektion malen
            if ( !HasPaintEvent() )
                ImplRepaint();
            else
                Invalidate();
        }

        SetInputContext( InputContext( GetFont(), !IsReadOnly() ? INPUTCONTEXT_TEXT|INPUTCONTEXT_EXTTEXTINPUT : 0 ) );
    }

    Control::GetFocus();
}

// -----------------------------------------------------------------------

void Edit::LoseFocus()
{
    if ( !mpSubEdit )
    {
        if ( !mbActivePopup && !( GetStyle() & WB_NOHIDESELECTION ) && maSelection.Len() )
            ImplRepaint();    // Selektion malen
    }

    Control::LoseFocus();
}

// -----------------------------------------------------------------------

void Edit::Command( const CommandEvent& rCEvt )
{
    if ( (rCEvt.GetCommand() == COMMAND_STARTDRAG) &&
         !IsTracking() && maSelection.Len() &&
         !(GetStyle() & WB_PASSWORD) &&
         (!mpDDInfo || mpDDInfo->bStarterOfDD == FALSE) ) // Kein Mehrfach D&D
    {
        Selection aSel( maSelection );
        aSel.Justify();

        // Nur wenn Maus in der Selektion...
        xub_StrLen nChar = ImplGetCharPos( rCEvt.GetMousePosPixel() );
        if ( (nChar >= aSel.Min()) && (nChar < aSel.Max()) )
        {
            if ( !mpDDInfo )
                mpDDInfo = new DDInfo;

            mpDDInfo->bStarterOfDD = TRUE;

            Region* pDDRegion = NULL;
            Cursor* pCursor = GetCursor();
            if ( pCursor )
                pCursor->Hide();

            SotDataObjectRef xData = new StringDataObject( GetSelected() );
            if ( IsTracking() )
                EndTracking();  // Vor D&D Tracking ausschalten
            DropAction aAction = DragManager::ExecuteDrag( xData, DRAG_ALL );

            if ( aAction == DROP_MOVE )
            {
                if ( mpDDInfo->bDroppedInMe )
                {
                    if ( aSel.Max() > mpDDInfo->nDropPos )
                    {
                        long nLen = aSel.Len();
                        aSel.Min() += nLen;
                        aSel.Max() += nLen;
                    }
                }
                ImplDelete( aSel, EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
                ImplModified();
            }

            ImplHideDDCursor();
            delete mpDDInfo;
            mpDDInfo = 0;
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        PopupMenu* pPopup = Edit::CreatePopupMenu();
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        if ( rStyleSettings.GetOptions() & STYLE_OPTION_HIDEDISABLED )
            pPopup->SetMenuFlags( MENU_FLAG_HIDEDISABLEDENTRIES );

        if ( !maSelection.Len() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_CUT, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_COPY, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_DELETE, FALSE );
        }

        if ( IsReadOnly() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_CUT, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_PASTE, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_DELETE, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_INSERTSYMBOL, FALSE );
        }
        else
        {
            // Paste nur, wenn Text im Clipboard
            if ( !VclClipboard::HasFormat( FORMAT_STRING ) )
                pPopup->EnableItem( SV_MENU_EDIT_PASTE, FALSE );
        }

        if ( maUndoText == maText )
            pPopup->EnableItem( SV_MENU_EDIT_UNDO, FALSE );
        if ( ( maSelection.Min() == 0 ) && ( maSelection.Max() == maText.Len() ) )
            pPopup->EnableItem( SV_MENU_EDIT_SELECTALL, FALSE );
        if ( !pImplFncGetSpecialChars )
        {
            USHORT nPos = pPopup->GetItemPos( SV_MENU_EDIT_INSERTSYMBOL );
            pPopup->RemoveItem( nPos );
            pPopup->RemoveItem( nPos-1 );
        }

        mbActivePopup = TRUE;
        Selection aSaveSel = GetSelection();    // Falls jemand in Get/LoseFocus die Selektion verbiegt, z.B. URL-Zeile...
        Point aPos = rCEvt.GetMousePosPixel();
        if ( !rCEvt.IsMouseEvent() )
        {
            // !!! Irgendwann einmal Menu zentriert in der Selektion anzeigen !!!
            Size aSize = GetOutputSizePixel();
            aPos = Point( aSize.Width()/2, aSize.Height()/2 );
        }
        USHORT n = pPopup->Execute( this, aPos );
        Edit::DeletePopupMenu( pPopup );
        SetSelection( aSaveSel );
        switch ( n )
        {
            case SV_MENU_EDIT_UNDO:
                Undo();
                ImplModified();
                break;
            case SV_MENU_EDIT_CUT:
                Cut();
                ImplModified();
                break;
            case SV_MENU_EDIT_COPY:
                Copy();
                break;
            case SV_MENU_EDIT_PASTE:
                Paste();
                ImplModified();
                break;
            case SV_MENU_EDIT_DELETE:
                DeleteSelected();
                ImplModified();
                break;
            case SV_MENU_EDIT_SELECTALL:
                ImplSetSelection( Selection( 0, maText.Len() ) );
                break;
            case SV_MENU_EDIT_INSERTSYMBOL:
                {
                    XubString aChars = pImplFncGetSpecialChars( this, GetFont() );
                    SetSelection( aSaveSel );
                    if ( aChars.Len() )
                    {
                        ImplInsertText( aChars );
                        ImplModified();
                    }
                }
                break;
        }
        mbActivePopup = FALSE;
    }
    else if ( rCEvt.GetCommand() == COMMAND_VOICE )
    {
        const CommandVoiceData* pData = rCEvt.GetVoiceData();
        if ( pData->GetType() == VOICECOMMANDTYPE_DICTATION )
        {
            switch ( pData->GetCommand() )
            {
                case DICTATIONCOMMAND_UNKNOWN:
                {
                    ReplaceSelected( pData->GetText() );
                }
                break;
                case DICTATIONCOMMAND_LEFT:
                {
                    ImplHandleKeyEvent( KeyEvent( 0, KeyCode( KEY_LEFT, KEY_MOD1  ) ) );
                }
                break;
                case DICTATIONCOMMAND_RIGHT:
                {
                    ImplHandleKeyEvent( KeyEvent( 0, KeyCode( KEY_RIGHT, KEY_MOD1  ) ) );
                }
                break;
                case DICTATIONCOMMAND_UNDO:
                {
                    Undo();
                }
                break;
                case DICTATIONCOMMAND_DEL:
                {
                    ImplHandleKeyEvent( KeyEvent( 0, KeyCode( KEY_LEFT, KEY_MOD1|KEY_SHIFT  ) ) );
                    DeleteSelected();
                }
                break;
            }
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_STARTEXTTEXTINPUT )
    {
        DeleteSelected();
        delete mpIMEInfos;
        mpIMEInfos = new Impl_IMEInfos( (xub_StrLen)maSelection.Max() );
        mpIMEInfos->bWasCursorOverwrite = !IsInsertMode();
    }
    else if ( rCEvt.GetCommand() == COMMAND_ENDEXTTEXTINPUT )
    {
        BOOL bInsertMode = !mpIMEInfos->bWasCursorOverwrite;
        delete mpIMEInfos;
        mpIMEInfos = NULL;
        // Font wieder ohne Attribute einstellen, wird jetzt im Repaint nicht
        // mehr neu initialisiert
        ImplInitSettings( TRUE, FALSE, FALSE );

        SetInsertMode( bInsertMode );
    }
    else if ( rCEvt.GetCommand() == COMMAND_EXTTEXTINPUT )
    {
        const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();

        maText.Erase( mpIMEInfos->nPos, mpIMEInfos->nLen );
        maText.Insert( pData->GetText(), mpIMEInfos->nPos );
        ImplModified();

        if ( pData->GetTextAttr() )
        {
            mpIMEInfos->CopyAttribs( pData->GetTextAttr(), pData->GetText().Len() );
            mpIMEInfos->bCursor = pData->IsCursorVisible();
        }
        else
        {
            mpIMEInfos->DestroyAttribs();
        }

        Invalidate();   // Erstmal einfach zum Testen
        xub_StrLen nCursorPos = mpIMEInfos->nPos + pData->GetCursorPos();
        SetSelection( Selection( nCursorPos, nCursorPos ) );
        SetInsertMode( !pData->IsCursorOverwrite() );
    }
    else if ( rCEvt.GetCommand() == COMMAND_CURSORPOS )
    {
        if ( mpIMEInfos )
        {
            xub_StrLen nCursorPos = GetSelection().Max();
            SetCursorRect( NULL, GetTextWidth(
                maText, nCursorPos, mpIMEInfos->nPos+mpIMEInfos->nLen-nCursorPos ) );
        }
    }
    else
        Control::Command( rCEvt );
}

// -----------------------------------------------------------------------

void Edit::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_INITSHOW )
    {
        if ( !mpSubEdit )
        {
            mnXOffset = 0;  // Falls vorher GrabFocus, als Groesse noch falsch.
            ImplAlign();
            if ( !mpSubEdit )
                ImplShowCursor( FALSE );
        }
    }
    else if ( nType == STATE_CHANGE_ENABLE )
    {
        if ( !mpSubEdit )
        {
            // Es aendert sich nur die Textfarbe...
            ImplRepaint( 0, 0xFFFF );
        }
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        WinBits nStyle = ImplInitStyle( GetStyle() );
        SetStyle( nStyle );

        USHORT nOldAlign = mnAlign;
        mnAlign = EDIT_ALIGN_LEFT;
        if ( nStyle & WB_RIGHT )
            mnAlign = EDIT_ALIGN_RIGHT;
        else if ( nStyle & WB_CENTER )
            mnAlign = EDIT_ALIGN_CENTER;
        if ( maText.Len() && ( mnAlign != nOldAlign ) )
        {
            ImplAlign();
            Invalidate();
        }

    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( TRUE, FALSE, FALSE );
            ImplShowCursor( TRUE );
            Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( TRUE, FALSE, FALSE );
            ImplShowCursor();
            Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( FALSE, TRUE, FALSE );
            Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( FALSE, FALSE, TRUE );
            Invalidate();
        }
    }

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

void Edit::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( TRUE, TRUE, TRUE );
            ImplShowCursor( TRUE );
            Invalidate();
        }
    }

    Control::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

void Edit::ImplShowDDCursor()
{
    if ( !mpDDInfo->bVisCursor )
    {
        long nTextWidth = GetTextWidth( maText, 0, mpDDInfo->nDropPos );
        long nTextHeight = GetTextHeight();
        Rectangle aCursorRect( Point( nTextWidth + mnXOffset, (GetOutputSize().Height()-nTextHeight)/2 ), Size( 2, nTextHeight ) );
        mpDDInfo->aCursor.SetWindow( this );
        mpDDInfo->aCursor.SetPos( aCursorRect.TopLeft() );
        mpDDInfo->aCursor.SetSize( aCursorRect.GetSize() );
        mpDDInfo->aCursor.Show();
        mpDDInfo->bVisCursor = TRUE;
    }
}

// -----------------------------------------------------------------------

void Edit::ImplHideDDCursor()
{
    if ( mpDDInfo && mpDDInfo->bVisCursor )
    {
        mpDDInfo->aCursor.Hide();
        mpDDInfo->bVisCursor = FALSE;
    }
}

// -----------------------------------------------------------------------

BOOL Edit::QueryDrop( DropEvent& rDEvt )
{
    if ( rDEvt.IsLeaveWindow() )
    {
        ImplHideDDCursor();
        return FALSE;
    }

    // Daten holen
    SotDataObjectRef xDataObj = ((DropEvent&)rDEvt).GetData();
    BOOL bString = xDataObj.Is() && NULL != xDataObj->GetTypeList().Get( FORMAT_STRING );

    BOOL bDrop = FALSE;
    if ( !mbReadOnly && bString &&
        ( ( rDEvt.GetAction() == DROP_COPY ) || ( rDEvt.GetAction() == DROP_MOVE ) || ( rDEvt.GetAction() == DROP_LINK ) ) )
    {
        if ( !mpDDInfo )
            mpDDInfo = new DDInfo;

        Point aMousePos( rDEvt.GetPosPixel() );

        xub_StrLen nPrevDropPos = mpDDInfo->nDropPos;
        mpDDInfo->nDropPos = ImplGetCharPos( aMousePos );

        Size aOutSize = GetOutputSizePixel();
        if ( ( aMousePos.X() < 0 ) || ( aMousePos.X() > aOutSize.Width() ) )
        {
            // Scrollen ?
        }

        Selection aSel( maSelection );
        aSel.Justify();

        // Nicht in Selektion droppen:
        if ( aSel.IsInside( mpDDInfo->nDropPos ) )
        {
            ImplHideDDCursor();
            return FALSE;
        }

        // Alten Cursor wegzeichnen...
        if ( !mpDDInfo->bVisCursor || ( nPrevDropPos != mpDDInfo->nDropPos ) )
        {
            ImplHideDDCursor();
            ImplShowDDCursor();
        }
        bDrop = TRUE;
    }

    return bDrop;
}

// -----------------------------------------------------------------------

BOOL Edit::Drop( const DropEvent& rDEvt )
{
    mbInternModified = FALSE;

    BOOL bDone = TRUE;
    if ( !mbReadOnly && mpDDInfo )
    {
        ImplHideDDCursor();
        Point aMousePos( rDEvt.GetPosPixel() );

        Selection aSel( maSelection );
        aSel.Justify();

        if ( aSel.Len() && !mpDDInfo->bStarterOfDD )
            ImplDelete( aSel, EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );

        mpDDInfo->bDroppedInMe = TRUE;

        aSel.Min() = mpDDInfo->nDropPos;
        aSel.Max() = mpDDInfo->nDropPos;
        ImplSetSelection( aSel );

        // Daten holen
        SotDataObjectRef xDataObj = ((DropEvent&)rDEvt).GetData();
        SvData aData( FORMAT_STRING );
        if( xDataObj->GetData( &aData ) )
        {
            XubString aStr;
            aData.GetData( aStr );
            ImplInsertText( aStr );
        }

        if ( !mpDDInfo->bStarterOfDD )
        {
            delete mpDDInfo;
            mpDDInfo = 0;
        }
    }

    if ( mbInternModified )
        ImplModified();

    return bDone;
}

// -----------------------------------------------------------------------

void Edit::Modify()
{
        if ( mbIsSubEdit )
    {
        ((Edit*)GetParent())->Modify();
    }
    else
    {
        if ( mpUpdateDataTimer )
            mpUpdateDataTimer->Start();

        maModifyHdl.Call( this );
    }
}

// -----------------------------------------------------------------------

void Edit::UpdateData()
{
    maUpdateDataHdl.Call( this );
}

// -----------------------------------------------------------------------

IMPL_LINK( Edit, ImplUpdateDataHdl, Timer*, EMPTYARG )
{
    UpdateData();
    return 0;
}

// -----------------------------------------------------------------------

void Edit::EnableUpdateData( ULONG nTimeout )
{
    if ( !nTimeout )
        DisableUpdateData();
    else
    {
        if ( !mpUpdateDataTimer )
        {
            mpUpdateDataTimer = new Timer;
            mpUpdateDataTimer->SetTimeoutHdl( LINK( this, Edit, ImplUpdateDataHdl ) );
        }

        mpUpdateDataTimer->SetTimeout( nTimeout );
    }
}

// -----------------------------------------------------------------------

void Edit::SetEchoChar( xub_Unicode c )
{
    mcEchoChar = c;
    if ( mpSubEdit )
        mpSubEdit->SetEchoChar( c );
}

// -----------------------------------------------------------------------

void Edit::SetReadOnly( BOOL bReadOnly )
{
    if ( mbReadOnly != bReadOnly )
    {
        mbReadOnly = bReadOnly;
        if ( mpSubEdit )
            mpSubEdit->SetReadOnly( bReadOnly );

        StateChanged( STATE_CHANGE_READONLY );
    }
}

// -----------------------------------------------------------------------

void Edit::SetAutocompleteHdl( const Link& rHdl )
{
    maAutocompleteHdl = rHdl;
    if ( mpSubEdit )
        mpSubEdit->SetAutocompleteHdl( rHdl );
}

// -----------------------------------------------------------------------

void Edit::SetInsertMode( BOOL bInsert )
{
    if ( bInsert != mbInsertMode )
    {
        mbInsertMode = bInsert;
        if ( mpSubEdit )
            mpSubEdit->SetInsertMode( bInsert );
        else
            ImplShowCursor();
    }
}

// -----------------------------------------------------------------------

BOOL Edit::IsInsertMode() const
{
    if ( mpSubEdit )
        return mpSubEdit->IsInsertMode();
    else
        return mbInsertMode;
}

// -----------------------------------------------------------------------

void Edit::SetMaxTextLen( xub_StrLen nMaxLen )
{
    mnMaxTextLen = nMaxLen ? nMaxLen : EDIT_NOLIMIT;

    if ( mpSubEdit )
        mpSubEdit->SetMaxTextLen( nMaxLen );
    else
    {
        if ( maText.Len() > nMaxLen )
            ImplDelete( Selection( nMaxLen, maText.Len() ), EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
    }
}

// -----------------------------------------------------------------------

void Edit::SetSelection( const Selection& rSelection )
{
    // Wenn von aussen z.B. im MouseButtonDown die Selektion geaendert wird,
    // soll nicht gleich ein Tracking() zuschlagen und die Selektion aendern.
    if ( IsTracking() )
        EndTracking();
    else if ( mpSubEdit && mpSubEdit->IsTracking() )
        mpSubEdit->EndTracking();

    ImplSetSelection( rSelection );
}

// -----------------------------------------------------------------------

void Edit::ImplSetSelection( const Selection& rSelection, BOOL bPaint )
{
    if ( mpSubEdit )
        mpSubEdit->ImplSetSelection( rSelection );
    else
    {
        if ( rSelection != maSelection )
        {
            Selection aOld( maSelection );
            Selection aNew( rSelection );

            if ( aNew.Min() > maText.Len() )
                aNew.Min() = maText.Len();
            if ( aNew.Max() > maText.Len() )
                aNew.Max() = maText.Len();
            if ( aNew.Min() < 0 )
                aNew.Min() = 0;
            if ( aNew.Max() < 0 )
                aNew.Max() = 0;

            if ( aNew != maSelection )
            {
                maSelection = aNew;

                if ( bPaint && ( aOld.Len() || aNew.Len() ) )
                {
                    aOld.Justify();
                    aNew.Justify();
                    xub_StrLen nStart = (xub_StrLen)Min( aOld.Min(), aNew.Min() );
                    xub_StrLen nEnd = (xub_StrLen)Max( aOld.Max(), aNew.Max() );
                    ImplRepaint( nStart, nEnd );
                }
                ImplShowCursor();
            }
        }
    }
}

// -----------------------------------------------------------------------

const Selection& Edit::GetSelection() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetSelection();
    else
        return maSelection;
}

// -----------------------------------------------------------------------

void Edit::ReplaceSelected( const XubString& rStr )
{
    if ( mpSubEdit )
        mpSubEdit->ReplaceSelected( rStr );
    else
        ImplInsertText( rStr );
}

// -----------------------------------------------------------------------

void Edit::DeleteSelected()
{
    if ( mpSubEdit )
        mpSubEdit->DeleteSelected();
    else
    {
        if ( maSelection.Len() )
            ImplDelete( maSelection, EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
    }
}

// -----------------------------------------------------------------------

XubString Edit::GetSelected() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetSelected();
    else
    {
        Selection aSelection( maSelection );
        aSelection.Justify();
        return maText.Copy( (xub_StrLen)aSelection.Min(), (xub_StrLen)aSelection.Len() );
    }
}

// -----------------------------------------------------------------------

void Edit::Cut()
{
    if ( !(GetStyle() & WB_PASSWORD ) )
    {
        Copy();
        ReplaceSelected( ImplGetSVEmptyStr() );
    }
}

// -----------------------------------------------------------------------

void Edit::Copy()
{
    if ( !(GetStyle() & WB_PASSWORD ) )
    {
        SotDataObjectRef xData = new StringDataObject( GetSelected() );
        VclClipboard::Copy( xData );
    }
}

// -----------------------------------------------------------------------

void Edit::Paste()
{
    // Daten holen
    SotDataObjectRef xDataObj = VclClipboard::Paste();
    SvData aData( FORMAT_STRING );
    if( xDataObj.Is() && xDataObj->GetData( &aData ) )
    {
        XubString aStr;
        aData.GetData( aStr );
        ReplaceSelected( aStr );
    }
}

// -----------------------------------------------------------------------

void Edit::Undo()
{
    if ( mpSubEdit )
        mpSubEdit->Undo();
    else
    {
        XubString aText( maText );
        ImplDelete( Selection( 0, aText.Len() ), EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
        ImplInsertText( maUndoText );
        ImplSetSelection( Selection( 0, maUndoText.Len() ) );
        maUndoText = aText;
    }
}

// -----------------------------------------------------------------------

void Edit::SetText( const XubString& rStr )
{
    if ( mpSubEdit )
        mpSubEdit->SetText( rStr );     // Nicht direkt ImplSetText, falls SetText ueberladen
    else
    {
        Selection aNewSel( 0, 0 );  // Damit nicht gescrollt wird
        ImplSetText( rStr, &aNewSel );
    }
}

// -----------------------------------------------------------------------

void Edit::SetText( const XubString& rStr, const Selection& rSelection )
{
    if ( mpSubEdit )
        mpSubEdit->SetText( rStr, rSelection );
    else
        ImplSetText( rStr, &rSelection );
}

// -----------------------------------------------------------------------

XubString Edit::GetText() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetText();
    else
        return maText;
}

// -----------------------------------------------------------------------

void Edit::SetModifyFlag()
{
    if ( mpSubEdit )
        mpSubEdit->mbModified = TRUE;
    else
        mbModified = TRUE;
}

// -----------------------------------------------------------------------

void Edit::ClearModifyFlag()
{
    if ( mpSubEdit )
        mpSubEdit->mbModified = FALSE;
    else
        mbModified = FALSE;
}

// -----------------------------------------------------------------------

void Edit::SetSubEdit( Edit* pEdit )
{
    mpSubEdit = pEdit;
    if ( mpSubEdit )
    {
        SetPointer( POINTER_ARROW );    // Nur das SubEdit hat den BEAM...
        mpSubEdit->mbIsSubEdit = TRUE;
    }
}

// -----------------------------------------------------------------------

Size Edit::CalcMinimumSize() const
{
    Size aSz( GetTextWidth( GetText() ), GetTextHeight() );
    aSz = CalcWindowSize( aSz );
    return aSz;
}

// -----------------------------------------------------------------------

Size Edit::CalcSize( xub_StrLen nChars ) const
{
    // Breite fuer n Zeichen, unabhaengig vom Inhalt.
    // Funktioniert nur bei FixedFont richtig, sonst Mittelwert.
    Size aSz( GetTextWidth( XubString( 'x' ) ), GetTextHeight() );
    aSz.Width() *= nChars;
    aSz = CalcWindowSize( aSz );
    return aSz;
}

// -----------------------------------------------------------------------

xub_StrLen Edit::GetMaxVisChars() const
{
    const Window* pW = mpSubEdit ? mpSubEdit : this;
    long nOutWidth = pW->GetOutputSizePixel().Width();
    long nCharWidth = GetTextWidth( XubString( 'x' ) );
    return nCharWidth ? (xub_StrLen)(nOutWidth/nCharWidth) : 0;
}

// -----------------------------------------------------------------------

void Edit::SetGetSpecialCharsFunction( FncGetSpecialChars fn )
{
    pImplFncGetSpecialChars = fn;
}

// -----------------------------------------------------------------------

FncGetSpecialChars Edit::GetGetSpecialCharsFunction()
{
    return pImplFncGetSpecialChars;
}

// -----------------------------------------------------------------------

PopupMenu* Edit::CreatePopupMenu()
{
    PopupMenu* pPopup = new PopupMenu( ResId( SV_RESID_MENU_EDIT, ImplGetResMgr() ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_UNDO, KeyCode( KEYFUNC_UNDO ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_CUT, KeyCode( KEYFUNC_CUT ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_COPY, KeyCode( KEYFUNC_COPY ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_PASTE, KeyCode( KEYFUNC_PASTE ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_DELETE, KeyCode( KEYFUNC_DELETE ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_SELECTALL, KeyCode( KEY_A, FALSE, TRUE, FALSE ) );
    pPopup->SetAccelKey( SV_MENU_EDIT_INSERTSYMBOL, KeyCode( KEY_S, TRUE, TRUE, FALSE ) );
    return pPopup;
}

// -----------------------------------------------------------------------

void Edit::DeletePopupMenu( PopupMenu* pMenu )
{
    delete pMenu;
}

ImplSubEdit::ImplSubEdit( Edit* pParent, WinBits nStyle ) :
    Edit( pParent, nStyle )
{
    pParent->SetSubEdit( this );
}

// -----------------------------------------------------------------------

void ImplSubEdit::Modify()
{
    GetParent()->Modify();
}
