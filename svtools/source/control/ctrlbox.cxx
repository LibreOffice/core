/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ctrlbox.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 17:13:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

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

#include <svtools/svtdata.hxx>
#include <svtools/svtools.hrc>
#include <ctrlbox.hxx>
#include <ctrltool.hxx>

#include <vcl/i18nhelp.hxx>

#define IMGTEXTSPACE    2
#define EXTRAFONTSIZE   5

static sal_Unicode aImplSymbolFontText[] = {0xF021,0xF032,0xF043,0xF054,0xF065,0xF076,0xF0B7,0xF0C8,0};
static sal_Unicode aImplStarSymbolText[] = {0x2706,0x2704,0x270D,0xE033,0x2211,0x2288,0};

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

DECLARE_LIST( ImpColorList, ImplColorListData* )

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

void ColorListBox::InsertAutomaticEntry()
{
    // insert the "Automatic"-entry always on the first position
    InsertEntry( Color( COL_AUTO ), SvtResId( STR_SVT_AUTOMATIC_COLOR ), 0 );
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
            return n;
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
            rUDEvt.GetDevice()->SetLineColor( rUDEvt.GetDevice()->GetTextColor() );
            rUDEvt.GetDevice()->DrawRect( Rectangle( aPos, aImageSize ) );
            rUDEvt.GetDevice()->Pop();
            ListBox::DrawEntry( rUDEvt, FALSE, TRUE, FALSE );
        }
        else
            ListBox::DrawEntry( rUDEvt, FALSE, TRUE, TRUE );
    }
    else
        ListBox::DrawEntry( rUDEvt, TRUE, TRUE, FALSE );
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

DECLARE_LIST( ImpLineList, ImpLineListData* )

// -----------------------------------------------------------------------

inline const Color& LineListBox::GetPaintColor( void ) const
{
    return maPaintCol;
}

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

        aVirDev.SetFillColor( GetPaintColor() );
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

    String aNum( GetSettings().GetLocaleI18nHelper().GetNum( nLine1+nLine2+nDistance, 2 ) );
    rStr.Insert( aNum, 0 );
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

    UpdatePaintLineColor();
}

// -----------------------------------------------------------------------

LineListBox::LineListBox( Window* pParent, WinBits nWinStyle ) :
    ListBox( pParent, nWinStyle ),
    aColor( COL_BLACK ),
    maPaintCol( COL_BLACK )
{
    ImplInit();
}

// -----------------------------------------------------------------------

LineListBox::LineListBox( Window* pParent, const ResId& rResId ) :
    ListBox( pParent, rResId ),
    aColor( COL_BLACK ),
    maPaintCol( COL_BLACK )
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
        ImpLineListData* pData = new ImpLineListData;
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

void LineListBox::UpdateLineColors( void )
{
    if( UpdatePaintLineColor() )
    {
        ULONG       nCount = pLineList->Count();
        if( !nCount )
            return;

        XubString   aStr;
        Bitmap      aBmp;

        // exchange entries which containing lines
        SetUpdateMode( FALSE );

        USHORT      nSelEntry = GetSelectEntryPos();
        for( ULONG n = 0 ; n < nCount ; ++n )
        {
            ImpLineListData*    pData = pLineList->GetObject( n );
            if( pData )
            {
                // exchange listbox data
                ListBox::RemoveEntry( USHORT( n ) );
                ImpGetLine( pData->nLine1, pData->nLine2, pData->nDistance, aBmp, aStr );
                ListBox::InsertEntry( aStr, aBmp, USHORT( n ) );
            }
        }

        if( nSelEntry != LISTBOX_ENTRY_NOTFOUND )
            SelectEntryPos( nSelEntry );

        SetUpdateMode( TRUE );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

BOOL LineListBox::UpdatePaintLineColor( void )
{
    BOOL                    bRet = TRUE;
    const StyleSettings&    rSettings = GetSettings().GetStyleSettings();
    Color                   aNewCol( rSettings.GetWindowColor().IsDark()? rSettings.GetLabelTextColor() : aColor );

    bRet = aNewCol != maPaintCol;

    if( bRet )
        maPaintCol = aNewCol;

    return bRet;
}

// -----------------------------------------------------------------------

void LineListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ListBox::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        UpdateLineColors();
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

DECLARE_LIST( ImplFontList, ImplFontNameListData* )

// -------------------------------------------------------------------

FontNameBox::FontNameBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    InitBitmaps();
    mpFontList = NULL;
    mbWYSIWYG = FALSE;
    mbSymbols = FALSE;
}

// -------------------------------------------------------------------

FontNameBox::FontNameBox( Window* pParent, const ResId& rResId ) :
    ComboBox( pParent, rResId )
{
    InitBitmaps();
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

void FontNameBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if( rDCEvt.GetType() == DATACHANGED_SETTINGS && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitBitmaps();
}

// -------------------------------------------------------------------

void FontNameBox::InitBitmaps( void )
{
    Color   aCol = GetSettings().GetStyleSettings().GetWindowColor();
    BOOL    bHC = aCol.IsDark();

    maImagePrinterFont = Image( SvtResId( bHC? RID_IMG_PRINTERFONT_HC : RID_IMG_PRINTERFONT ) );
    maImageBitmapFont = Image( SvtResId( bHC? RID_IMG_BITMAPFONT_HC : RID_IMG_BITMAPFONT ) );
    maImageScalableFont = Image( SvtResId( bHC? RID_IMG_SCALABLEFONT_HC : RID_IMG_SCALABLEFONT ) );
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
    // store old text and clear box
    XubString aOldText = GetText();
    Clear();

    ImplDestroyFontList();
    mpFontList = new ImplFontList;

    // insert fonts
    USHORT nFontCount = pList->GetFontNameCount();
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

    // restore text
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
        BOOL bStarSymbol = FALSE;
        for ( USHORT n = GetEntryCount(); n; )
        {
            ImplFontNameListData* pData = mpFontList->GetObject( --n );
            XubString aFontName = pData->maInfo.GetName();
            if ( aFontName.Len() > nMaxLen )
                nMaxLen = aFontName.Len();
            if ( pData->maInfo.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
                bSymbolFont = TRUE;
            // starsymbol is a unicode font, but gets WYSIWIG symbols
            if( aFontName.EqualsIgnoreCaseAscii( "starsymbol" )
            ||  aFontName.EqualsIgnoreCaseAscii( "opensymbol" ) )
                bSymbolFont = bStarSymbol = TRUE;
        }

        // guess maximimum width
        Size aOneCharSz( GetTextWidth( String( 'X' ) ), GetTextHeight() );
        Size aSz( aOneCharSz );
        aSz.Width() *= nMaxLen;
        // only XX% of width, because ListBox calculates the normal width...
        aSz.Width() *= 1;
        aSz.Width() /= 10;
        if ( bSymbolFont )
        {
            int nLength = sizeof(aImplSymbolFontText)/sizeof(aImplSymbolFontText[0]) - 1;
            int nLength2 = sizeof(aImplStarSymbolText)/sizeof(aImplStarSymbolText[0]) - 1;
            if( bStarSymbol && (nLength < nLength2) )
                nLength = nLength2;
            aSz.Width() += aOneCharSz.Width() * nLength;
        }
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

        bool bSymbolFont = (rInfo.GetCharSet() == RTL_TEXTENCODING_SYMBOL);
        // starsymbol is a unicode font, but cannot display its own name
        const bool bOpenSymbol = rInfo.GetName().EqualsIgnoreCaseAscii( "starsymbol" )
                              || rInfo.GetName().EqualsIgnoreCaseAscii( "opensymbol" );
        bSymbolFont |= bOpenSymbol;

        if( bSymbolFont )
        {
            String aText( rInfo.GetName() );
            aText.AppendAscii( "  " );
            Point aPos( nX, aTopLeft.Y() + (nH-rUDEvt.GetDevice()->GetTextHeight())/2 );
            rUDEvt.GetDevice()->DrawText( aPos, aText );
            nX += rUDEvt.GetDevice()->GetTextWidth( aText );
        }

        Color aTextColor = rUDEvt.GetDevice()->GetTextColor();
        Font aOldFont( rUDEvt.GetDevice()->GetFont() );
        Size aSize( aOldFont.GetSize() );
        aSize.Height() += EXTRAFONTSIZE;
        Font aFont( rInfo );
        aFont.SetSize( aSize );
        rUDEvt.GetDevice()->SetFont( aFont );
        rUDEvt.GetDevice()->SetTextColor( aTextColor );

        FontCharMap aFontCharMap;
        bool bHasCharMap = rUDEvt.GetDevice()->GetFontCharMap( aFontCharMap );

        String aString;
        if( !bSymbolFont )
        {
              // preview the font name
            aString = rInfo.GetName();

            // reset font if the name cannot be display in the preview font
            if( STRING_LEN != rUDEvt.GetDevice()->HasGlyphs( aFont, aString ) )
                rUDEvt.GetDevice()->SetFont( aOldFont );
        }
        else if( bHasCharMap )
        {
            // use some sample characters available in the font
            sal_Unicode aText[8];

            // start just above the PUA used by most symbol fonts
            sal_uInt32 cNewChar = 0xFF00;
#ifdef QUARTZ
            // on MacOSX there are too many non-presentable symbols above the codepoint 0x0192
            if( !bOpenSymbol )
                cNewChar = 0x0192;
#endif
            const int nMaxCount = sizeof(aText)/sizeof(*aText) - 1;
            int nSkip = aFontCharMap.GetCharCount() / nMaxCount;
            if( nSkip > 10 )
                nSkip = 10;
            else if( nSkip <= 0 )
                nSkip = 1;
            for( int i = 0; i < nMaxCount; ++i )
            {
                sal_uInt32 cOldChar = cNewChar;
                for( int j = nSkip; --j >= 0; )
                    cNewChar = aFontCharMap.GetPrevChar( cNewChar );
                if( cOldChar == cNewChar )
                    break;
                aText[ i ] = static_cast<sal_Unicode>(cNewChar); // TODO: support UCS4 samples
                aText[ i+1 ] = 0;
            }

            aString = String( aText );
        }
        else
        {
            const sal_Unicode* pText = aImplSymbolFontText;
            if( bOpenSymbol )
                pText = aImplStarSymbolText;

            aString = String( pText );
        }

        long nTextHeight = rUDEvt.GetDevice()->GetTextHeight();
        Point aPos( nX, aTopLeft.Y() + (nH-nTextHeight)/2 );
        rUDEvt.GetDevice()->DrawText( aPos, aString );

        rUDEvt.GetDevice()->SetFont( aOldFont );
        DrawEntry( rUDEvt, FALSE, FALSE);   // draw seperator
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
    // keep text over fill operation
    aLastStyle = GetText();
    ComboBox::Select();
}

// -------------------------------------------------------------------

void FontStyleBox::LoseFocus()
{
    // keep text over fill operation
    aLastStyle = GetText();
    ComboBox::LoseFocus();
}

// -------------------------------------------------------------------

void FontStyleBox::Modify()
{
    CharClass   aChrCls( ::comphelper::getProcessServiceFactory(),
                        GetSettings().GetLocale() );
    XubString   aStr = GetText();
    USHORT      nEntryCount = GetEntryCount();

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
    // note: this method must call ComboBox::SetText(),
    //   else aLastStyle will overwritten
    // store prior selection position and clear box
    XubString aOldText = GetText();
    USHORT nPos = GetEntryPos( aOldText );
    Clear();

    // does a font with this name already exist?
    sal_Handle hFontInfo = pList->GetFirstFontInfo( rName );
    if ( hFontInfo )
    {
        XubString   aStyleText;
        FontWeight  eLastWeight = WEIGHT_DONTKNOW;
        FontItalic  eLastItalic = ITALIC_NONE;
        FontWidth   eLastWidth = WIDTH_DONTKNOW;
        BOOL        bNormal = FALSE;
        BOOL        bItalic = FALSE;
        BOOL        bBold = FALSE;
        BOOL        bBoldItalic = FALSE;
        BOOL        bInsert = FALSE;
        FontInfo    aInfo;
        while ( hFontInfo )
        {
            aInfo = pList->GetFontInfo( hFontInfo );

            FontWeight  eWeight = aInfo.GetWeight();
            FontItalic  eItalic = aInfo.GetItalic();
            FontWidth   eWidth = aInfo.GetWidthType();
            // Only if the attributes are different, we insert the
            // Font to avoid double Entries in different languages
            if ( (eWeight != eLastWeight) || (eItalic != eLastItalic) ||
                 (eWidth != eLastWidth) )
            {
                if ( bInsert )
                    InsertEntry( aStyleText );

                if ( eWeight <= WEIGHT_NORMAL )
                {
                    if ( eItalic != ITALIC_NONE )
                        bItalic = TRUE;
                    else
                        bNormal = TRUE;
                }
                else
                {
                    if ( eItalic != ITALIC_NONE )
                        bBoldItalic = TRUE;
                    else
                        bBold = TRUE;
                }

                // For wrong StyleNames we replace this with the correct once
                aStyleText = pList->GetStyleName( aInfo );
                bInsert = GetEntryPos( aStyleText ) == LISTBOX_ENTRY_NOTFOUND;
                if ( !bInsert )
                {
                    aStyleText = pList->GetStyleName( eWeight, eItalic );
                    bInsert = GetEntryPos( aStyleText ) == LISTBOX_ENTRY_NOTFOUND;
                }

                eLastWeight = eWeight;
                eLastItalic = eItalic;
                eLastWidth = eWidth;
            }
            else
            {
                if ( bInsert )
                {
                    // If we have two names for the same attributes
                    // we prefer the translated standard names
                    const XubString& rAttrStyleText = pList->GetStyleName( eWeight, eItalic );
                    if ( rAttrStyleText != aStyleText )
                    {
                        XubString aTempStyleText = pList->GetStyleName( aInfo );
                        if ( rAttrStyleText == aTempStyleText )
                            aStyleText = rAttrStyleText;
                        bInsert = GetEntryPos( aStyleText ) == LISTBOX_ENTRY_NOTFOUND;
                    }
                }
            }

            if ( !bItalic && (aStyleText == pList->GetItalicStr()) )
                bItalic = TRUE;
            else if ( !bBold && (aStyleText == pList->GetBoldStr()) )
                bBold = TRUE;
            else if ( !bBoldItalic && (aStyleText == pList->GetBoldItalicStr()) )
                bBoldItalic = TRUE;

            hFontInfo = pList->GetNextFontInfo( hFontInfo );
        }

        if ( bInsert )
            InsertEntry( aStyleText );

        // Bestimmte Styles als Nachbildung
        if ( bNormal )
        {
            if ( !bItalic )
                InsertEntry( pList->GetItalicStr() );
            if ( !bBold )
                InsertEntry( pList->GetBoldStr() );
        }
        if ( !bBoldItalic )
        {
            if ( bNormal || bItalic || bBold )
                InsertEntry( pList->GetBoldItalicStr() );
        }
        if ( aOldText.Len() )
        {
            if ( GetEntryPos( aLastStyle ) != LISTBOX_ENTRY_NOTFOUND )
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

    SetShowTrailingZeros( FALSE );
    SetDecimalDigits( 1 );
    SetMin( 20 );
    SetMax( 9999 );
}

// -----------------------------------------------------------------------

void FontSizeBox::Reformat()
{
    FontSizeNames aFontSizeNames( GetSettings().GetUILanguage() );
    if ( !bRelativeMode || !aFontSizeNames.IsEmpty() )
    {
        long nNewValue = aFontSizeNames.Name2Size( GetText() );
        if ( nNewValue)
        {
            mnLastValue = nNewValue;
            return;
        }
    }

    MetricBox::Reformat();
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
    }
}

// -----------------------------------------------------------------------

void FontSizeBox::Fill( const FontInfo* pInfo, const FontList* pList )
{
    // remember for relative mode
    pFontList = pList;

    // no font sizes need to be set for relative mode
    if ( bRelative )
        return;

    // query font sizes
    const long* pTempAry;
    const long* pAry = 0;

    if( pInfo )
    {
        aFontInfo = *pInfo;
        pAry = pList->GetSizeAry( *pInfo );
    }
    else
    {
        pAry = pList->GetStdSizeAry();
    }

    // first insert font size names (for simplified/traditional chinese)
    FontSizeNames aFontSizeNames( GetSettings().GetUILanguage() );
    if ( pAry == pList->GetStdSizeAry() )
    {
        // for standard sizes we don't need to bother
        if ( bStdSize && GetEntryCount() && aFontSizeNames.IsEmpty() )
            return;
        bStdSize = TRUE;
    }
    else
        bStdSize = FALSE;

    Selection aSelection = GetSelection();
    XubString aStr = GetText();

    Clear();
    USHORT nPos = 0;

    if ( !aFontSizeNames.IsEmpty() )
    {
        if ( pAry == pList->GetStdSizeAry() )
        {
            // for scalable fonts all font size names
            ULONG nCount = aFontSizeNames.Count();
            for( ULONG i = 0; i < nCount; i++ )
            {
                String  aSizeName = aFontSizeNames.GetIndexName( i );
                long    nSize = aFontSizeNames.GetIndexSize( i );
                ComboBox::InsertEntry( aSizeName, nPos );
                ComboBox::SetEntryData( nPos, (void*)(-nSize) ); // mark as special
                nPos++;
            }
        }
        else
        {
            // for fixed size fonts only selectable font size names
            pTempAry = pAry;
            while ( *pTempAry )
            {
                String aSizeName = aFontSizeNames.Size2Name( *pTempAry );
                if ( aSizeName.Len() )
                {
                    ComboBox::InsertEntry( aSizeName, nPos );
                    ComboBox::SetEntryData( nPos, (void*)(-(*pTempAry)) ); // mark as special
                    nPos++;
                }
                pTempAry++;
            }
        }
    }

    // then insert numerical font size values
    pTempAry = pAry;
    while ( *pTempAry )
    {
        InsertValue( *pTempAry, FUNIT_NONE, nPos );
        ComboBox::SetEntryData( nPos, (void*)(*pTempAry) );
        nPos++;
        pTempAry++;
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
                // JP 30.06.98: more than 100 values are not useful
                while ( i <= nPtRelMax && n++ < 100 )
                {
                    InsertValue( i );
                    i = i + nPtRelStep;
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
                    i = i + nRelStep;
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
                Fill( &aFontInfo, pFontList );
        }

        SetText( aStr );
        SetSelection( aSelection );
    }
}

// -----------------------------------------------------------------------

XubString FontSizeBox::CreateFieldText( sal_Int64 nValue ) const
{
    XubString sRet( MetricBox::CreateFieldText( nValue ) );
    if ( bRelativeMode && bPtRelative && (0 <= nValue) && sRet.Len() )
        sRet.Insert( '+', 0 );
    return sRet;
}

// -----------------------------------------------------------------------

void FontSizeBox::SetValue( sal_Int64 nNewValue, FieldUnit eInUnit )
{
    if ( !bRelative )
    {
        sal_Int64 nTempValue = MetricField::ConvertValue( nNewValue, GetBaseValue(), GetDecimalDigits(), eInUnit, GetUnit() );
        FontSizeNames aFontSizeNames( GetSettings().GetUILanguage() );
        // conversion loses precision; however font sizes should
        // never have a problem with that
        String aName = aFontSizeNames.Size2Name( static_cast<long>(nTempValue) );
        if ( aName.Len() && (GetEntryPos( aName ) != LISTBOX_ENTRY_NOTFOUND) )
        {
            mnLastValue = nTempValue;
            SetText( aName );
            mnFieldValue = mnLastValue;
            SetEmptyFieldValueData( FALSE );
            return;
        }
    }

    MetricBox::SetValue( nNewValue, eInUnit );
}

// -----------------------------------------------------------------------

void FontSizeBox::SetValue( sal_Int64 nNewValue )
{
    SetValue( nNewValue, FUNIT_NONE );
}

// -----------------------------------------------------------------------

sal_Int64 FontSizeBox::GetValue( USHORT nPos, FieldUnit eOutUnit ) const
{
    if ( !bRelative )
    {
        sal_Int64 nComboVal = static_cast<sal_Int64>(reinterpret_cast<long>(ComboBox::GetEntryData( nPos )));
        if ( nComboVal < 0 )     // marked as special?
        {
            return MetricField::ConvertValue( -nComboVal, mnBaseValue, GetDecimalDigits(),
                                              meUnit, eOutUnit );
        }
    }

    // do normal font size processing
    sal_Int64 nRetValue = MetricBox::GetValue( nPos, eOutUnit );
    return nRetValue;
}

// -----------------------------------------------------------------------

sal_Int64 FontSizeBox::GetValue( FieldUnit eOutUnit ) const
{
    if ( !bRelative )
    {
        FontSizeNames aFontSizeNames( GetSettings().GetUILanguage() );
        sal_Int64 nValue = aFontSizeNames.Name2Size( GetText() );
        if ( nValue)
            return MetricField::ConvertValue( nValue, GetBaseValue(), GetDecimalDigits(), GetUnit(), eOutUnit );
    }

    return MetricBox::GetValue( eOutUnit );
}

// -----------------------------------------------------------------------

sal_Int64 FontSizeBox::GetValue() const
{
    // implementation not inline, because it is a virtual function
    return GetValue( FUNIT_NONE );
}

// -----------------------------------------------------------------------

void FontSizeBox::SetUserValue( sal_Int64 nNewValue, FieldUnit eInUnit )
{
    if ( !bRelative )
    {
        sal_Int64 nTempValue = MetricField::ConvertValue( nNewValue, GetBaseValue(), GetDecimalDigits(), eInUnit, GetUnit() );
        FontSizeNames aFontSizeNames( GetSettings().GetUILanguage() );
        // conversion loses precision
        // however font sizes should never have a problem with that
        String aName = aFontSizeNames.Size2Name( static_cast<long>(nTempValue) );
        if ( aName.Len() && (GetEntryPos( aName ) != LISTBOX_ENTRY_NOTFOUND) )
        {
            mnLastValue = nTempValue;
            SetText( aName );
            return;
        }
    }

    MetricBox::SetUserValue( nNewValue, eInUnit );
}

