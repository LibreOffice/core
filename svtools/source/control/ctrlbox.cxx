/*************************************************************************
 *
 *  $RCSfile: ctrlbox.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2000-10-22 17:54:23 $
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

#define _CTRLBOX_CXX

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _APP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#include <svtdata.hxx>
#include <svtools.hrc>
#include <ctrlbox.hxx>
#include <ctrltool.hxx>

#define IMGTEXTSPACE    2
#define SYMBOLFONTTEXT  "AbCdEfGhIj"
#define EXTRAFONTSIZE   5

// ========================================================================
// ColorListBox
// ========================================================================

// --------------------
// - ImplColorListData -
// --------------------

struct ImplColorListData
{
    Color       aColor;
    BOOL        bColor;

                ImplColorListData() : aColor( COL_BLACK ) { bColor = FALSE; }
                ImplColorListData( const Color& rColor ) : aColor( rColor ) { bColor = TRUE; }
};

DECLARE_LIST( ImpColorList, ImplColorListData* );

// -----------------------------------------------------------------------

void ColorListBox::ImplInit()
{
    pColorList = new ImpColorList( 256, 64 );
    aImageSize.Width()  = GetTextWidth( XubString( RTL_CONSTASCII_USTRINGPARAM( "xxx" ) ) );
    aImageSize.Height() = GetTextHeight();
    aImageSize.Height() -= 2;

    EnableUserDraw( TRUE );
    SetUserItemSize( aImageSize );
}

// -----------------------------------------------------------------------

void ColorListBox::ImplDestroyColorEntries()
{
    for ( USHORT n = (USHORT) pColorList->Count(); n; )
    {
        ImplColorListData* pData = pColorList->GetObject( --n );
        delete pData;
    }
    pColorList->Clear();
}

// -----------------------------------------------------------------------

ColorListBox::ColorListBox( Window* pParent, WinBits nWinStyle ) :
    ListBox( pParent, nWinStyle )
{
    ImplInit();
}

// -----------------------------------------------------------------------

ColorListBox::ColorListBox( Window* pParent, const ResId& rResId ) :
    ListBox( pParent, rResId )
{
    ImplInit();
}

// -----------------------------------------------------------------------

ColorListBox::~ColorListBox()
{
    ImplDestroyColorEntries();
    delete pColorList;
}

// -----------------------------------------------------------------------

USHORT ColorListBox::InsertEntry( const XubString& rStr, USHORT nPos )
{
    nPos = ListBox::InsertEntry( rStr, nPos );
    if ( nPos != LISTBOX_ERROR )
    {
        ImplColorListData* pData = new ImplColorListData;
        pColorList->Insert( pData, nPos );
    }
    return nPos;
}

// -----------------------------------------------------------------------

USHORT ColorListBox::InsertEntry( const Color& rColor, const XubString& rStr,
                                  USHORT nPos )
{
    nPos = ListBox::InsertEntry( rStr, nPos );
    if ( nPos != LISTBOX_ERROR )
    {
        ImplColorListData* pData = new ImplColorListData( rColor );
        pColorList->Insert( pData, nPos );
    }
    return nPos;
}

// -----------------------------------------------------------------------

void ColorListBox::RemoveEntry( USHORT nPos )
{
    ListBox::RemoveEntry( nPos );
    delete pColorList->Remove( nPos );
}

// -----------------------------------------------------------------------

void ColorListBox::Clear()
{
    ImplDestroyColorEntries();
    ListBox::Clear();
}

// -----------------------------------------------------------------------

void ColorListBox::CopyEntries( const ColorListBox& rBox )
{
    // Liste leeren
    ImplDestroyColorEntries();

    // Daten kopieren
    USHORT nCount = (USHORT) rBox.pColorList->Count();
    for ( USHORT n = 0; n < nCount; n++ )
    {
        ImplColorListData* pData = rBox.pColorList->GetObject( n );
        USHORT nPos = InsertEntry( rBox.GetEntry( n ), LISTBOX_APPEND );
        if ( nPos != LISTBOX_ERROR )
            pColorList->Insert( new ImplColorListData( *pData ), nPos );
    }
}

// -----------------------------------------------------------------------

USHORT ColorListBox::GetEntryPos( const Color& rColor ) const
{
    for( USHORT n = (USHORT) pColorList->Count(); n; )
    {
        ImplColorListData* pData = pColorList->GetObject( --n );
        if ( pData->bColor && ( pData->aColor == rColor ) )
        {
            return n;
        }
    }
    return LISTBOX_ENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

Color ColorListBox::GetEntryColor( USHORT nPos ) const
{
    Color aColor;
    ImplColorListData* pData = pColorList->GetObject( nPos );
    if ( pData && pData->bColor )
        aColor = pData->aColor;
    return aColor;
}

// -----------------------------------------------------------------------

void ColorListBox::UserDraw( const UserDrawEvent& rUDEvt )
{
    ImplColorListData* pData = pColorList->GetObject( rUDEvt.GetItemId() );
    if ( pData )
    {
        if ( pData->bColor )
        {
            Point aPos( rUDEvt.GetRect().TopLeft() );
            aPos.X() += 2;
            aPos.Y() += ( rUDEvt.GetRect().GetHeight() - aImageSize.Height() ) / 2;
            rUDEvt.GetDevice()->Push();
            rUDEvt.GetDevice()->SetFillColor( pData->aColor );
            rUDEvt.GetDevice()->SetLineColor( COL_BLACK );
            rUDEvt.GetDevice()->DrawRect( Rectangle( aPos, aImageSize ) );
            rUDEvt.GetDevice()->Pop();
            ListBox::DrawEntry( rUDEvt, FALSE, TRUE, FALSE );
        }
        else
        {
            ListBox::DrawEntry( rUDEvt, FALSE, TRUE, TRUE );
        }
    }
    else
    {
        ListBox::DrawEntry( rUDEvt, TRUE, TRUE, FALSE );
    }
}

// =======================================================================
// LineListBox
// =======================================================================

// -------------------
// - ImpListListData -
// -------------------

struct ImpLineListData
{
    long    nLine1;
    long    nLine2;
    long    nDistance;
};

DECLARE_LIST( ImpLineList, ImpLineListData* );

// -----------------------------------------------------------------------

void LineListBox::ImpGetLine( long nLine1, long nLine2, long nDistance,
                              Bitmap& rBmp, XubString& rStr )
{
    Size aSize = GetOutputSizePixel();
    aSize.Width() -= 20;
    aSize.Width() -= aTxtSize.Width();
    aSize.Height() = aTxtSize.Height();

    // SourceUnit nach Twips
    if ( eSourceUnit == FUNIT_POINT )
    {
        nLine1      *= 20;
        nLine2      *= 20;
        nDistance   *= 20;
    }
    else if ( eSourceUnit == FUNIT_MM )
    {
        nLine1      *= 14440;
        nLine1      /= 254;
        nLine2      *= 14440;
        nLine2      /= 254;
        nDistance   *= 14440;
        nDistance   /= 254;
    }

    // Linien malen
    aSize = aVirDev.PixelToLogic( aSize );
    long nPix = aVirDev.PixelToLogic( Size( 0, 1 ) ).Height();
    long n1 = nLine1 / 100;
    long n2 = nLine2 / 100;
    long nDist  = nDistance / 100;
    n1 += nPix-1;
    n1 -= n1%nPix;
    if ( n2 )
    {
        nDist += nPix-1;
        nDist -= nDist%nPix;
        n2    += nPix-1;
        n2    -= n2%nPix;
    }
    long nVirHeight = n1+nDist+n2;
    if ( nVirHeight > aSize.Height() )
        aSize.Height() = nVirHeight;
    // negative Breiten muss und darf man nicht painten
    if ( aSize.Width() > 0 )
    {
        Size aVirSize = aVirDev.LogicToPixel( aSize );
        if ( aVirDev.GetOutputSizePixel() != aVirSize )
            aVirDev.SetOutputSizePixel( aVirSize );
        aVirDev.SetFillColor( GetSettings().GetStyleSettings().GetFieldColor() );
        aVirDev.DrawRect( Rectangle( Point(), aSize ) );
        aVirDev.SetFillColor( aColor );
        aVirDev.DrawRect( Rectangle( 0, 0, aSize.Width(), n1-nPix ) );
        if ( n2 )
        {
            aVirDev.DrawRect( Rectangle( 0, n1+nDist,
                                         aSize.Width(), n1+nDist+n2-nPix ) );
        }
        rBmp = aVirDev.GetBitmap( Point(), Size( aSize.Width(), n1+nDist+n2 ) );
    }
    // Twips nach Unit
    if ( eUnit == FUNIT_POINT )
    {
        nLine1      /= 20;
        nLine2      /= 20;
        nDistance   /= 20;
        rStr.AssignAscii( " pt" );
    }
    else if ( eUnit == FUNIT_MM )
    {
        nLine1      *= 254;
        nLine1      /= 14400;
        nLine2      *= 254;
        nLine2      /= 14400;
        nDistance   *= 254;
        nDistance   /= 14400;
        rStr.AssignAscii( " mm" );
    }

    rStr.Insert( aIntn.GetNum( nLine1+nLine2+nDistance, 2 ), 0 );
}

// -----------------------------------------------------------------------

void LineListBox::ImplInit()
{
    aTxtSize.Width()  = GetTextWidth( XubString( RTL_CONSTASCII_USTRINGPARAM( "99,99 mm" ) ) );
    aTxtSize.Height() = GetTextHeight();
    pLineList   = new ImpLineList;
    eUnit       = FUNIT_POINT;
    eSourceUnit = FUNIT_POINT;

    aVirDev.SetLineColor();
    aVirDev.SetMapMode( MapMode( MAP_TWIP ) );
}

// -----------------------------------------------------------------------

LineListBox::LineListBox( Window* pParent, WinBits nWinStyle ) :
    ListBox( pParent, nWinStyle ),
    aIntn( Application::GetAppInternational() ),
    aColor( COL_BLACK )
{
    ImplInit();
}

// -----------------------------------------------------------------------

LineListBox::LineListBox( Window* pParent, const ResId& rResId ) :
    ListBox( pParent, rResId ),
    aIntn( Application::GetAppInternational() ),
    aColor( COL_BLACK )
{
    ImplInit();
}

// -----------------------------------------------------------------------

LineListBox::~LineListBox()
{
    ULONG n = 0;
    ULONG nCount = pLineList->Count();
    while ( n < nCount )
    {
        ImpLineListData* pData = pLineList->GetObject( n );
        if ( pData )
            delete pData;
        n++;
    }
    delete pLineList;
}

// -----------------------------------------------------------------------

USHORT LineListBox::InsertEntry( const XubString& rStr, USHORT nPos )
{
    nPos = ListBox::InsertEntry( rStr, nPos );
    if ( nPos != LISTBOX_ERROR )
        pLineList->Insert( NULL, nPos );
    return nPos;
}

// -----------------------------------------------------------------------

USHORT LineListBox::InsertEntry( long nLine1, long nLine2, long nDistance,
                                 USHORT nPos )
{
    XubString   aStr;
    Bitmap      aBmp;
    ImpGetLine( nLine1, nLine2, nDistance, aBmp, aStr );
    nPos = ListBox::InsertEntry( aStr, aBmp, nPos );
    if ( nPos != LISTBOX_ERROR )
    {
        ImpLineListData* pData  = new ImpLineListData;
        pData->nLine1    = nLine1;
        pData->nLine2    = nLine2;
        pData->nDistance = nDistance;
        pLineList->Insert( pData, nPos );
    }

    return nPos;
}

// -----------------------------------------------------------------------

void LineListBox::RemoveEntry( USHORT nPos )
{
    ListBox::RemoveEntry( nPos );
    ImpLineListData* pData = pLineList->Remove( nPos );
    if ( pData )
        delete pData;
}

// -----------------------------------------------------------------------

void LineListBox::Clear()
{
    ULONG n = 0;
    ULONG nCount = pLineList->Count();
    while ( n < nCount )
    {
        ImpLineListData* pData = pLineList->GetObject( n );
        if ( pData )
            delete pData;
        n++;
    }

    pLineList->Clear();
    ListBox::Clear();
}

// -----------------------------------------------------------------------

USHORT LineListBox::GetEntryPos( long nLine1, long nLine2,
                                 long nDistance ) const
{
    ULONG n = 0;
    ULONG nCount = pLineList->Count();
    while ( n < nCount )
    {
        ImpLineListData* pData = pLineList->GetObject( n );
        if ( pData )
        {
            if ( (pData->nLine1    == nLine1) &&
                 (pData->nLine2    == nLine2) &&
                 (pData->nDistance == nDistance) )
                return (USHORT)n;
        }

        n++;
    }

    return LISTBOX_ENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

long LineListBox::GetEntryLine1( USHORT nPos ) const
{
    ImpLineListData* pData = pLineList->GetObject( nPos );
    if ( pData )
        return pData->nLine1;
    else
        return 0;
}

// -----------------------------------------------------------------------

long LineListBox::GetEntryLine2( USHORT nPos ) const
{
    ImpLineListData* pData = pLineList->GetObject( nPos );
    if ( pData )
        return pData->nLine2;
    else
        return 0;
}

// -----------------------------------------------------------------------

long LineListBox::GetEntryDistance( USHORT nPos ) const
{
    ImpLineListData* pData = pLineList->GetObject( nPos );
    if ( pData )
        return pData->nDistance;
    else
        return 0;
}

// -----------------------------------------------------------------------

void LineListBox::SetColor( const Color& rColor )
{
    // Farben austauschen
    aColor = rColor;

    // Variablen anlegen
    ULONG n = 0;
    ULONG nCount = pLineList->Count();
    if ( !nCount )
        return;

    XubString aStr;
    Bitmap   aBmp;

    // Eintrage mit Linien austauschen
    SetUpdateMode( FALSE );
    USHORT nSelEntry = GetSelectEntryPos();
    while ( n < nCount )
    {
        ImpLineListData* pData = pLineList->GetObject( n );
        if ( pData )
        {
            // ListBox-Daten austauschen
            ListBox::RemoveEntry( (USHORT)n );
            ImpGetLine( pData->nLine1, pData->nLine2, pData->nDistance,
                        aBmp, aStr );
            ListBox::InsertEntry( aStr, aBmp, (USHORT)n );
        }

        n++;
    }

    if ( nSelEntry != LISTBOX_ENTRY_NOTFOUND )
        SelectEntryPos( nSelEntry );

    SetUpdateMode( TRUE );
    Invalidate();
}

// ===================================================================
// FontNameBox
// ===================================================================

struct ImplFontNameListData
{
    FontInfo    maInfo;
    USHORT      mnType;

                ImplFontNameListData( const FontInfo& rInfo,
                                      USHORT nType ) :
                    maInfo( rInfo ),
                    mnType( nType )
                {}
};

DECLARE_LIST( ImplFontList, ImplFontNameListData* );

// -------------------------------------------------------------------

FontNameBox::FontNameBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle ),
    maImagePrinterFont( SvtResId( RID_IMG_PRINTERFONT ) ),
    maImageBitmapFont( SvtResId( RID_IMG_BITMAPFONT ) ),
    maImageScalableFont( SvtResId( RID_IMG_SCALABLEFONT ) )
{
    mpFontList = NULL;
    mbWYSIWYG = FALSE;
    mbSymbols = FALSE;
}

// -------------------------------------------------------------------

FontNameBox::FontNameBox( Window* pParent, const ResId& rResId ) :
    ComboBox( pParent, rResId ),
    maImagePrinterFont( SvtResId( RID_IMG_PRINTERFONT ) ),
    maImageBitmapFont( SvtResId( RID_IMG_BITMAPFONT ) ),
    maImageScalableFont( SvtResId( RID_IMG_SCALABLEFONT ) )
{
    mpFontList = NULL;
    mbWYSIWYG = FALSE;
    mbSymbols = FALSE;
}

// -------------------------------------------------------------------

FontNameBox::~FontNameBox()
{
    ImplDestroyFontList();
}

// -------------------------------------------------------------------

void FontNameBox::ImplDestroyFontList()
{
    if ( mpFontList )
    {
        ImplFontNameListData* pInfo = mpFontList->First();
        while ( pInfo )
        {
            delete pInfo;
            pInfo = mpFontList->Next();
        }
        delete mpFontList;
    }
}

// -------------------------------------------------------------------

void FontNameBox::Fill( const FontList* pList )
{
    // Vorherigen Namen merken und Box loeschen
    XubString aOldText = GetText();
    Clear();

    ImplDestroyFontList();
    mpFontList = new ImplFontList;

    // Fonts eintragen
    USHORT   nFontCount = pList->GetFontNameCount();
    for ( USHORT i = 0; i < nFontCount; i++ )
    {
        const FontInfo& rFontInfo = pList->GetFontName( i );
        ULONG nIndex = InsertEntry( rFontInfo.GetName() );
        if ( nIndex != LISTBOX_ERROR )
        {
            USHORT nType = pList->GetFontNameType( i );
            ImplFontNameListData* pData = new ImplFontNameListData( rFontInfo, nType );
            mpFontList->Insert( pData, nIndex );
        }
    }

    ImplCalcUserItemSize();

    // Text wieder setzen
    if ( aOldText.Len() )
        SetText( aOldText );
}

// -------------------------------------------------------------------

void FontNameBox::EnableWYSIWYG( BOOL bEnable )
{
    if ( bEnable != mbWYSIWYG )
    {
        mbWYSIWYG = bEnable;
        EnableUserDraw( mbWYSIWYG | mbSymbols );
        ImplCalcUserItemSize();
    }
}

// -------------------------------------------------------------------

void FontNameBox::EnableSymbols( BOOL bEnable )
{
    if ( bEnable != mbSymbols )
    {
        mbSymbols = bEnable;
        EnableUserDraw( mbWYSIWYG | mbSymbols );
        ImplCalcUserItemSize();
    }
}

// -------------------------------------------------------------------

void FontNameBox::ImplCalcUserItemSize()
{
    Size aUserItemSz;
    if ( mbWYSIWYG && mpFontList )
    {
        USHORT nMaxLen = 0;
        BOOL bSymbolFont = FALSE;
        for ( USHORT n = GetEntryCount(); n; )
        {
            ImplFontNameListData* pData = mpFontList->GetObject( --n );
            XubString aFontName = pData->maInfo.GetName();
            if ( aFontName.Len() > nMaxLen )
                nMaxLen = aFontName.Len();
            if ( pData->maInfo.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
                bSymbolFont = TRUE;
        }

        // Maximale Breite schaetzen
        Size aOneCharSz( GetTextWidth( XubString( 'X' ) ), GetTextHeight() );
        Size aSz( aOneCharSz );
        aSz.Width() *= nMaxLen;
        // Nur XX% der Breite, weil die ListBox die normalen Breiten berechnet...
        aSz.Width() *= 1;
        aSz.Width() /= 10;
        if ( bSymbolFont )
            aSz.Width() += (sizeof( SYMBOLFONTTEXT )-1) * aOneCharSz.Width();
        aSz.Height() *= 14;
        aSz.Height() /= 10;
        aUserItemSz = aSz;
    }
    if ( mbSymbols )
    {
        Size aSz = maImageScalableFont.GetSizePixel();
        aUserItemSz.Width() += aSz.Width() + IMGTEXTSPACE;
        if ( aSz.Height() > aUserItemSz.Height() )
            aUserItemSz.Height() = aSz.Height();
    }
    SetUserItemSize( aUserItemSz );
}

// -------------------------------------------------------------------

void FontNameBox::UserDraw( const UserDrawEvent& rUDEvt )
{
    ImplFontNameListData*   pData = mpFontList->GetObject( rUDEvt.GetItemId() );
    const FontInfo&         rInfo = pData->maInfo;
    USHORT                  nType = pData->mnType;
    Point                   aTopLeft = rUDEvt.GetRect().TopLeft();
    long                    nX = aTopLeft.X();
    long                    nH = rUDEvt.GetRect().GetHeight();

    if ( mbSymbols )
    {
        nX += IMGTEXTSPACE;
        Image* pImg = NULL;
        if ( (nType & (FONTLIST_FONTNAMETYPE_PRINTER | FONTLIST_FONTNAMETYPE_SCREEN)) == FONTLIST_FONTNAMETYPE_PRINTER )
            pImg = &maImagePrinterFont;
        else if ( nType & FONTLIST_FONTNAMETYPE_SCALABLE )
            pImg = &maImageScalableFont;
        else
            pImg = &maImageBitmapFont;

        if ( pImg )
        {
            Point aPos( nX, aTopLeft.Y() + (nH-pImg->GetSizePixel().Height())/2 );
            rUDEvt.GetDevice()->DrawImage( aPos, *pImg );
        }

        // X immer um gleiche Breite aendern, auch wenn kein Image ausgegeben.
        nX += maImagePrinterFont.GetSizePixel().Width();
    }

    if ( mbWYSIWYG && mpFontList )
    {
        nX += IMGTEXTSPACE;
        BOOL bSymbolFont = rInfo.GetCharSet() == RTL_TEXTENCODING_SYMBOL;
        if ( bSymbolFont )
        {
            XubString aText( rInfo.GetName() );
            aText.AppendAscii( "  " );
            Point aPos( nX, aTopLeft.Y() + (nH-rUDEvt.GetDevice()->GetTextHeight())/2 );
            rUDEvt.GetDevice()->DrawText( aPos, aText );
            nX += rUDEvt.GetDevice()->GetTextWidth( aText );
        }

        Color aTextColor = rUDEvt.GetDevice()->GetTextColor();
        Font aOldFont( rUDEvt.GetDevice()->GetFont() );
        Size aSz( aOldFont.GetSize() );
        aSz.Height() += EXTRAFONTSIZE;
        Font aFont( rInfo );
        aFont.SetSize( aSz );
        rUDEvt.GetDevice()->SetFont( aFont );
        rUDEvt.GetDevice()->SetTextColor( aTextColor );
        long nTextHeight = rUDEvt.GetDevice()->GetTextHeight();
        Point aPos( nX, aTopLeft.Y() + (nH-nTextHeight)/2 );
        if ( bSymbolFont )
            rUDEvt.GetDevice()->DrawText( aPos, XubString( RTL_CONSTASCII_USTRINGPARAM( SYMBOLFONTTEXT ) ) );
        else
            rUDEvt.GetDevice()->DrawText( aPos, rInfo.GetName() );
        rUDEvt.GetDevice()->SetFont( aOldFont );
        DrawEntry( rUDEvt, FALSE, FALSE);   // Separator
    }
    else
    {
        DrawEntry( rUDEvt, TRUE, TRUE );
    }
}

// ===================================================================
// FontStyleBox
// ===================================================================

FontStyleBox::FontStyleBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
}

// -------------------------------------------------------------------

FontStyleBox::FontStyleBox( Window* pParent, const ResId& rResId ) :
    ComboBox( pParent, rResId )
{
    aLastStyle = GetText();
}

// -------------------------------------------------------------------

FontStyleBox::~FontStyleBox()
{
}

// -------------------------------------------------------------------

void FontStyleBox::Select()
{
    // Damit Text nach einem Fill erhalten bleibt
    aLastStyle = GetText();
    ComboBox::Select();
}

// -------------------------------------------------------------------

void FontStyleBox::LoseFocus()
{
    // Damit Text nach einem Fill erhalten bleibt
    aLastStyle = GetText();
    ComboBox::LoseFocus();
}

// -------------------------------------------------------------------

void FontStyleBox::Modify()
{
    CharClass aChrCls( ::comphelper::getProcessServiceFactory(),
        Application::GetSettings().GetLocale() );

    XubString       aStr = GetText();
    USHORT          nEntryCount = GetEntryCount();

    if ( GetEntryPos( aStr ) == COMBOBOX_ENTRY_NOTFOUND )
    {
        aChrCls.toUpper( aStr );
        for ( USHORT i = 0; i < nEntryCount; i++ )
        {
            XubString aEntryText = GetEntry( i );
            aChrCls.toUpper( aEntryText );

            if ( aStr == aEntryText )
            {
                SetText( GetEntry( i ) );
                break;
            }
        }
    }

    ComboBox::Modify();
}

// -------------------------------------------------------------------

void FontStyleBox::Fill( const XubString& rName, const FontList* pList )
{
    // Achtung: In dieser Methode muss ComboBox::SetText() aufgerufen werden,
    // da sonst aLastStyle ueberschrieben wird
    // Vorherige Position merken und Box loeschen
    XubString aOldText = GetText();
    USHORT   nPos = GetEntryPos( aOldText );
    Clear();

    // Existiert ein Font mit diesem Namen
    sal_Handle hFontInfo = pList->GetFirstFontInfo( rName );
    if ( hFontInfo )
    {
        BOOL        bFound = FALSE;
        BOOL        bNormal = FALSE;
        BOOL        bItalic = FALSE;
        BOOL        bBold = FALSE;
        BOOL        bBoldItalic = FALSE;
        FontInfo    aInfo;
        while ( hFontInfo )
        {
            aInfo = pList->GetFontInfo( hFontInfo );

            XubString   aStyleText = pList->GetStyleName( aInfo );
            FontWeight  eWeight = aInfo.GetWeight();
            FontItalic  eItalic = aInfo.GetItalic();
            if ( eWeight <= WEIGHT_NORMAL )
            {
                bNormal = TRUE;
                if ( eItalic != ITALIC_NONE )
                    bItalic = TRUE;
            }
            else
            {
                if ( eItalic != ITALIC_NONE )
                    bBoldItalic = TRUE;
                else
                    bBold = TRUE;
            }
            if ( aStyleText == pList->GetItalicStr() )
                bItalic = TRUE;
            else if ( aStyleText == pList->GetBoldStr() )
                bBold = TRUE;
            else if ( aStyleText == pList->GetBoldItalicStr() )
                bBoldItalic = TRUE;
            if ( !bFound && (aStyleText == aLastStyle) )
                bFound = TRUE;

            // Falls doch mal doppelte Strings kommen, dann
            // nach Moeglichkeit abfangen
            if ( GetEntryPos( aStyleText ) == LISTBOX_ENTRY_NOTFOUND )
                InsertEntry( aStyleText );

            hFontInfo = pList->GetNextFontInfo( hFontInfo );
        }

        // Bestimmte Styles als Nachbildung
        if ( bNormal )
        {
            if ( !bItalic )
            {
                if ( !bFound && (aLastStyle == pList->GetItalicStr()) )
                    bFound = TRUE;
                InsertEntry( pList->GetItalicStr() );
            }
            if ( !bBold )
            {
                if ( !bFound && (aLastStyle == pList->GetBoldStr()) )
                    bFound = TRUE;
                InsertEntry( pList->GetBoldStr() );
            }
        }
        if ( !bBoldItalic )
        {
            if ( bNormal || bItalic || bBold )
            {
                if ( !bFound && (aLastStyle == pList->GetBoldItalicStr()) )
                    bFound = TRUE;
                InsertEntry( pList->GetBoldItalicStr() );
            }
        }
        if ( aOldText.Len() )
        {
            if ( bFound )
                ComboBox::SetText( aLastStyle );
            else
            {
                if ( nPos >= GetEntryCount() )
                    ComboBox::SetText( GetEntry( 0 ) );
                else
                    ComboBox::SetText( GetEntry( nPos ) );
            }
        }
    }
    else
    {
        // Wenn Font nicht, dann Standard-Styles einfuegen
        InsertEntry( pList->GetNormalStr() );
        InsertEntry( pList->GetItalicStr() );
        InsertEntry( pList->GetBoldStr() );
        InsertEntry( pList->GetBoldItalicStr() );
        if ( aOldText.Len() )
        {
            if ( nPos > GetEntryCount() )
                ComboBox::SetText( GetEntry( 0 ) );
            else
                ComboBox::SetText( GetEntry( nPos ) );
        }
    }
}

// ===================================================================
// FontSizeBox
// ===================================================================

FontSizeBox::FontSizeBox( Window* pParent, WinBits nWinSize ) :
    MetricBox( pParent, nWinSize )
{
    ImplInit();
}

// -----------------------------------------------------------------------

FontSizeBox::FontSizeBox( Window* pParent, const ResId& rResId ) :
    MetricBox( pParent, rResId )
{
    ImplInit();
}

// -----------------------------------------------------------------------

FontSizeBox::~FontSizeBox()
{
}

// -----------------------------------------------------------------------

void FontSizeBox::ImplInit()
{
    EnableAutocomplete( FALSE );

    bRelativeMode   = FALSE;
    bPtRelative     = FALSE;
    bRelative       = FALSE;
    bStdSize        = FALSE;
    pFontList       = NULL;

    International aIntn = GetInternational();
    aIntn.SetNumTrailingZeros( FALSE );
    SetInternational( aIntn );

    SetDecimalDigits( 1 );
    SetMin( 20 );
    SetMax( 9999 );
}

// -----------------------------------------------------------------------

void FontSizeBox::Modify()
{
    MetricBox::Modify();

    if ( bRelativeMode )
    {
        XubString aStr = GetText();
        aStr.EraseLeadingChars();

        BOOL bNewMode = bRelative;
        BOOL bOldPtRelMode = bPtRelative;

        if ( bRelative )
        {
            bPtRelative = FALSE;
            const xub_Unicode* pStr = aStr.GetBuffer();
            while ( *pStr )
            {
                if ( ((*pStr < '0') || (*pStr > '9')) && (*pStr != '%') )
                {
                    if ( ('-' == *pStr || '+' == *pStr) && !bPtRelative )
                        bPtRelative = TRUE;
                    else if ( bPtRelative && 'p' == *pStr && 't' == *++pStr )
                        ;
                    else
                    {
                        bNewMode = FALSE;
                        break;
                    }
                }
                pStr++;
            }
        }
        else
        {
            if ( STRING_NOTFOUND != aStr.Search( '%' ) )
            {
                bNewMode = TRUE;
                bPtRelative = FALSE;
            }

            if ( '-' == aStr.GetChar( 0 ) || '+' == aStr.GetChar( 0 ) )
            {
                bNewMode = TRUE;
                bPtRelative = TRUE;
            }
        }

        if ( bNewMode != bRelative || bPtRelative != bOldPtRelMode )
            SetRelative( bNewMode );

        MetricBox::Modify();
    }
}

// -----------------------------------------------------------------------

void FontSizeBox::Fill( const FontInfo& rInfo, const FontList* pList )
{
    // Merken fuer Relative-Mode
    aFontInfo = rInfo;
    pFontList = pList;

    // Im Relative-Mode, muessen keine Fontgroessen gesetzt werden
    if ( bRelative )
        return;

    // Fontgroessen abfragen
    const long* pAry = pList->GetSizeAry( rInfo );

    // Wenn es das Array mit den Standardgroessen ist, muessen wir im
    // Normalfall die ListBox nicht neu fuellen
    if ( (pAry == pList->GetStdSizeAry()) && GetEntryCount() )
    {
        if ( bStdSize )
            return;
        bStdSize = TRUE;
    }
    else
        bStdSize = FALSE;

    Selection aSelection = GetSelection();
    XubString  aStr = GetText();

    Clear();

    while ( *pAry )
    {
        InsertValue( *pAry );
        pAry++;
    }

    SetText( aStr );
    SetSelection( aSelection );
}

// -----------------------------------------------------------------------

void FontSizeBox::EnableRelativeMode( USHORT nMin, USHORT nMax, USHORT nStep )
{
    bRelativeMode = TRUE;
    nRelMin       = nMin;
    nRelMax       = nMax;
    nRelStep      = nStep;
    SetUnit( FUNIT_POINT );
}

// -----------------------------------------------------------------------

void FontSizeBox::EnablePtRelativeMode( short nMin, short nMax, short nStep )
{
    bRelativeMode = TRUE;
    nPtRelMin     = nMin;
    nPtRelMax     = nMax;
    nPtRelStep    = nStep;
    SetUnit( FUNIT_POINT );
}

// -----------------------------------------------------------------------

void FontSizeBox::SetRelative( BOOL bNewRelative )
{
    if ( bRelativeMode )
    {
        Selection aSelection = GetSelection();
        XubString  aStr = GetText();
        aStr.EraseLeadingChars();

        if ( bNewRelative )
        {
            bRelative = TRUE;
            bStdSize = FALSE;

            if ( bPtRelative )
            {
                SetDecimalDigits( 1 );
                SetMin( nPtRelMin );
                SetMax( nPtRelMax );
                SetUnit( FUNIT_POINT );

                Clear();
                short i = nPtRelMin, n = 0;
                // JP 30.06.98: mehr als 100 Werte machen keinen Sinn
                while ( i <= nPtRelMax && n++ < 100 )
                {
                    InsertValue( i );
                    i += nPtRelStep;
                }
            }
            else
            {
                SetDecimalDigits( 0 );
                SetMin( nRelMin );
                SetMax( nRelMax );
                SetCustomUnitText( '%' );
                SetUnit( FUNIT_CUSTOM );

                Clear();
                USHORT i = nRelMin;
                while ( i <= nRelMax )
                {
                    InsertValue( i );
                    i += nRelStep;
                }
            }
        }
        else
        {
            bRelative = bPtRelative = FALSE;
            SetDecimalDigits( 1 );
            SetMin( 20 );
            SetMax( 9999 );
            SetUnit( FUNIT_POINT );
            if ( pFontList )
                Fill( aFontInfo, pFontList );
        }

        SetText( aStr );
        SetSelection( aSelection );
    }
}

// -----------------------------------------------------------------------

XubString FontSizeBox::CreateFieldText( long nValue ) const
{
    XubString sRet( MetricBox::CreateFieldText( nValue ) );
    if ( bRelativeMode && bPtRelative && (0 <= nValue) && sRet.Len() )
        sRet.Insert( '+', 0 );
    return sRet;
}
