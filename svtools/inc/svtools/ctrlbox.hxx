/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CTRLBOX_HXX
#define _CTRLBOX_HXX

#include "svtools/svtdllapi.h"

#include <tools/string.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include <vcl/field.hxx>

class FontList;
class ImplColorListData;
class ImpLineListData;
class ImplFontNameListData;

typedef ::std::vector< ImplColorListData*    > ImpColorList;
typedef ::std::vector< ImpLineListData*      > ImpLineList;
typedef ::std::vector< ImplFontNameListData* > ImplFontList;

// FIXME: STYLE_* duplicate values from editeng::SvxBorderStyle,
// which in turn duplicates values from com::sun::star::table::BorderLineStyle:
// this needs cleaning up on master
#define STYLE_SOLID                ( ( sal_uInt16 ) 0 )
#define STYLE_DOTTED               ( ( sal_uInt16 ) 1 )
#define STYLE_DASHED               ( ( sal_uInt16 ) 2 )
#define STYLE_NONE                 ( ( sal_uInt16 ) -1)

#define CHANGE_LINE1               ( ( sal_uInt16 ) 1 )
#define CHANGE_LINE2               ( ( sal_uInt16 ) 2 )
#define CHANGE_DIST                ( ( sal_uInt16 ) 4 )
#define ADAPT_DIST                 ( ( sal_uInt16 ) 8 )



/*************************************************************************

Beschreibung
============

class ColorListBox

Beschreibung

Erlaubt die Auswahl von Farben.

--------------------------------------------------------------------------

class LineListBox

Beschreibung

Erlaubt die Auswahl von Linien-Styles und Groessen. Es ist darauf zu achten,
das vor dem ersten Insert die Units und die Fesntergroesse gesetzt sein
muessen. An Unit wird Point und mm unterstuetzt und als SourceUnit Point,
mm und Twips. Alle Angaben muessen in 100teln der jeweiligen Einheit
vorliegen.

Line1 ist aeussere, Line2 die innere und Distance die Distanz zwischen
den beiden Linien. Wenn Line2 = 0 ist, wird nur Line1 angezeigt. Als
Default sind sowohl Source als auch Ziel-Unit FUNIT_POINT.

Mit SetColor() kann die Linienfarbe eingestellt werden.

Anmerkungen und Ausnahmen

Gegenueber einer normalen ListBox, koennen keine User-Daten gesetzt
werden. Ausserdem sollte wenn der UpdateMode ausgeschaltet ist, keine
Daten abgefragt oder die Selektion gesetzt werden, da in diesem Zustand
die Daten nicht definiert sind. Wenn der UpdateMode ausgeschaltet ist,
sollte der Rueckgabewert bei Insert nicht ausgewertet werden, da er keine
Bedeutung hat. Ausserdem darf nicht das WinBit WB_SORT gesetzt sein.

--------------------------------------------------------------------------

class FontNameBox

Beschreibung

Erlaubt die Auswahl von Fonts. Die ListBox wird mit Fill gefuellt, wo
ein Pointer auf eine FontList uebergeben werden muss.

Mit EnableWYSIWYG() kann man einstellen, das die Fontnamen in Ihrer Schrift
angezeigt werden und mit EnableSymbols() kann eingestellt werden, das
vor dem Namen ueber ein Symbol angezeigt wird, ob es sich um eine
Drucker oder Bildschirmschrift handelt.

Querverweise

FontList; FontStyleBox; FontSizeBox; FontNameMenu

--------------------------------------------------------------------------

class FontStyleBox

Beschreibung

Erlaubt die Auswahl eines FontStyles. Mit Fill wird die ListBox mit
den Styles zum uebergebenen Font gefuellt. Nachgebildete Styles werden
immer mit eingefuegt (kann sich aber noch aendern, da vielleicht
nicht alle Applikationen [StarDraw,Formel,FontWork] mit Syntetic-Fonts
umgehen koennen). Bei Fill bleibt vorherige Name soweit wie moeglich
erhalten.

Fuer DontKnow sollte die FontStyleBox mit String() gefuellt werden.
Dann enthaellt die Liste die Standardattribute. Der Style, der gerade
angezeigt wird, muss gegebenenfalls noch vom Programm zurueckgesetzt werden.

Querverweise

FontList; FontNameBox; FontSizeBox;

--------------------------------------------------------------------------

class FontSizeBox

Beschreibung

Erlaubt die Auswahl von Fontgroessen. Werte werden ueber GetValue()
abgefragt und ueber SetValue() gesetzt. Fill fuellt die ListBox mit den
Groessen zum uebergebenen Font. Alle Groessen werden in 10tel Point
angegeben. Die FontListe, die bei Fill uebergeben wird, muss bis zum
naechsten Fill-Aufruf erhalten bleiben.

Zusaetzlich erlaubt die FontSizeBox noch einen Relative-Mode. Dieser
dient dazu, Prozent-Werte eingeben zu koennen. Dies kann zum Beispiel
nuetzlich sein, wenn man die Box in einem Vorlagen-Dialog anbietet.
Dieser Modus ist nur anschaltbar, jedoch nicht wieder abschaltbar.

Fuer DontKnow sollte die FontSizeBox mit FontInfo() gefuellt werden.
Dann enthaellt die Liste die Standardgroessen. Die Groesse, die gerade
angezeigt wird, muss gegebenenfalls noch vom Programm zurueckgesetzt werden.

Querverweise

FontList; FontNameBox; FontStyleBox; FontSizeMenu

*************************************************************************/

// ----------------
// - ColorListBox -
// ----------------

class SVT_DLLPUBLIC ColorListBox : public ListBox
{
    ImpColorList*   pColorList; // Separate Liste, falls UserDaten von aussen verwendet werden.
    Size            aImageSize;

#ifdef _CTRLBOX_CXX
    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit();
    SVT_DLLPRIVATE void         ImplDestroyColorEntries();
#endif
public:
                    ColorListBox( Window* pParent,
                                  WinBits nWinStyle = WB_BORDER );
                    ColorListBox( Window* pParent, const ResId& rResId );
    virtual         ~ColorListBox();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    using ListBox::InsertEntry;
    virtual sal_uInt16  InsertEntry( const XubString& rStr,
                                 sal_uInt16 nPos = LISTBOX_APPEND );
    virtual sal_uInt16  InsertEntry( const Color& rColor, const XubString& rStr,
                                 sal_uInt16 nPos = LISTBOX_APPEND );
    void            InsertAutomaticEntryColor(const Color &rAutoColorValue);
    bool            IsAutomaticSelected() { return !GetSelectEntryPos(); }
    using ListBox::RemoveEntry;
    virtual void    RemoveEntry( sal_uInt16 nPos );
    virtual void    Clear();
    void            CopyEntries( const ColorListBox& rBox );

    using ListBox::GetEntryPos;
    virtual sal_uInt16  GetEntryPos( const Color& rColor ) const;
    virtual Color   GetEntryColor( sal_uInt16 nPos ) const;
    Size            GetImageSize() const { return aImageSize; }

    void            SelectEntry( const XubString& rStr, sal_Bool bSelect = sal_True )
                        { ListBox::SelectEntry( rStr, bSelect ); }
    void            SelectEntry( const Color& rColor, sal_Bool bSelect = sal_True );
    XubString       GetSelectEntry( sal_uInt16 nSelIndex = 0 ) const
                        { return ListBox::GetSelectEntry( nSelIndex ); }
    Color           GetSelectEntryColor( sal_uInt16 nSelIndex = 0 ) const;
    sal_Bool            IsEntrySelected( const XubString& rStr ) const
                        { return ListBox::IsEntrySelected( rStr ); }

    sal_Bool            IsEntrySelected( const Color& rColor ) const;

private:
    // declared as private because some compilers would generate the default functions
                    ColorListBox( const ColorListBox& );
    ColorListBox&   operator =( const ColorListBox& );

    void            SetEntryData( sal_uInt16 nPos, void* pNewData );
    void*           GetEntryData( sal_uInt16 nPos ) const;
};

inline void ColorListBox::SelectEntry( const Color& rColor, sal_Bool bSelect )
{
    sal_uInt16 nPos = GetEntryPos( rColor );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        ListBox::SelectEntryPos( nPos, bSelect );
}

inline sal_Bool ColorListBox::IsEntrySelected( const Color& rColor ) const
{
    sal_uInt16 nPos = GetEntryPos( rColor );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return IsEntryPosSelected( nPos );
    else
        return sal_False;
}

inline Color ColorListBox::GetSelectEntryColor( sal_uInt16 nSelIndex ) const
{
    sal_uInt16 nPos = GetSelectEntryPos( nSelIndex );
    Color aColor;
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        aColor = GetEntryColor( nPos );
    return aColor;
}

// ---------------
// - LineListBox -
// ---------------

/**
    Class computing border widths shared between Line style listbox and the
    SvxBorderLine implementation.

    This class doesn't know anything about units: it all depends on the different
    values set. A border is composed of 2 lines separated by a gap. The computed
    widths are the ones of each line and the gap and they can either be fix or vary.

    The #m_nflags member will define which widths will vary (value 0 means that all
    widths are fixed). The available flags are:
     - CHANGE_LINE1
     - CHANGE_LINE2
     - CHANGE_DIST

    For each line, the rate member is used as a multiplication factor is the width
    isn't fixed. Otherwise it is the width in the unit expected by the client code.
 */
class SVT_DLLPUBLIC BorderWidthImpl
{
    sal_uInt16 m_nFlags;
    double m_nRate1;
    double m_nRate2;
    double m_nRateGap;

public:

    BorderWidthImpl( sal_uInt16 nFlags = CHANGE_LINE1, double nRate1 = 0.0,
            double nRate2 = 0.0, double nRateGap = 0.0 );

    BorderWidthImpl& operator= ( const BorderWidthImpl& r );
    bool operator== ( const BorderWidthImpl& r ) const;

    long GetLine1 ( long nWidth ) const;
    long GetLine2( long nWidth ) const;
    long GetGap( long nWidth ) const;

    long GuessWidth( long nLine1, long nLine2, long nGap );

    bool IsEmpty( ) const { return (0 == m_nRate1) && (0 == m_nRate2); }
    bool IsDouble( ) const { return (0 != m_nRate1) && (0 != m_nRate2);  }
};

SVT_DLLPUBLIC inline Color sameColor( Color rMain )
{
    return rMain;
}

SVT_DLLPUBLIC inline Color sameDistColor( Color /*rMain*/, Color rDefault )
{
    return rDefault;
}


class SVT_DLLPUBLIC LineListBox : public ListBox
{
    ImpLineList*    pLineList;
    long            m_nWidth;
    XubString       m_sNone;

    VirtualDevice   aVirDev;
    Size            aTxtSize;
    Color           aColor;
    Color           maPaintCol;
    FieldUnit       eUnit;
    FieldUnit       eSourceUnit;

    SVT_DLLPRIVATE void         ImpGetLine( long nLine1, long nLine2, long nDistance,
                                    Color nColor1, Color nColor2, Color nColorDist,
                                    sal_uInt16 nStyle, Bitmap& rBmp );
    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit();
    sal_Bool        UpdatePaintLineColor( void );       // returns sal_True if maPaintCol has changed
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void            UpdateEntries( long nOldWidth );
    sal_uInt16           GetStylePos( sal_uInt16 nListPos, long nWidth );

public:
                    LineListBox( Window* pParent, WinBits nWinStyle = WB_BORDER );
                    LineListBox( Window* pParent, const ResId& rResId );
    virtual         ~LineListBox();

    /** Set the width in Twips */
    void            SetWidth( long nWidth );
    void            SetNone( const XubString& sNone );

    using ListBox::InsertEntry;
    virtual sal_uInt16  InsertEntry( const XubString& rStr, sal_uInt16 nPos = LISTBOX_APPEND );
    /** Insert a listbox entry with all widths in Twips. */
    void            InsertEntry( BorderWidthImpl aWidthImpl,
                        sal_uInt16 nStyle, long nMinWidth = 0,
                        Color (*pColor1Fn)(Color) = &sameColor,
                        Color (*pColor2Fn)( Color ) = &sameColor,
                        Color (*pColorDistFn)( Color, Color ) = &sameDistColor );

    using ListBox::RemoveEntry;
    virtual void    RemoveEntry( sal_uInt16 nPos );
    virtual void    Clear();

    using ListBox::GetEntryPos;
    virtual sal_uInt16  GetEntryPos( sal_uInt16 nStyle = STYLE_SOLID ) const;
    sal_uInt16          GetEntryStyle( sal_uInt16 nPos ) const;

    void            SelectEntry( const XubString& rStr, sal_Bool bSelect = sal_True ) { ListBox::SelectEntry( rStr, bSelect ); }
    void            SelectEntry( sal_uInt16 nStyle = STYLE_SOLID, sal_Bool bSelect = sal_True );
    sal_uInt16          GetSelectEntryStyle( sal_uInt16 nSelIndex = 0 ) const;
    inline sal_Bool     IsEntrySelected( const XubString& rStr ) const { return ListBox::IsEntrySelected( rStr ); }
    sal_Bool            IsEntrySelected( sal_uInt16 nStyle1 = STYLE_SOLID ) const;

    inline void     SetUnit( FieldUnit eNewUnit ) { eUnit = eNewUnit; }
    inline FieldUnit    GetUnit() const { return eUnit; }
    inline void     SetSourceUnit( FieldUnit eNewUnit ) { eSourceUnit = eNewUnit; }
    inline FieldUnit    GetSourceUnit() const { return eSourceUnit; }

    void            SetColor( const Color& rColor );
    inline Color    GetColor( void ) const;

protected:

    inline const Color&    GetPaintColor( void ) const;
    Color   GetColorLine1( sal_uInt16 nPos = 0 );
    Color   GetColorLine2( sal_uInt16 nPos = 0 );
    Color   GetColorDist( sal_uInt16 nPos = 0 );

private:
    // declared as private because some compilers would generate the default methods
                    LineListBox( const LineListBox& );
    LineListBox&    operator =( const LineListBox& );
    void            SetEntryData( sal_uInt16 nPos, void* pNewData );
    void*           GetEntryData( sal_uInt16 nPos ) const;
};

inline sal_Bool LineListBox::IsEntrySelected( sal_uInt16 nStyle ) const
{
    sal_uInt16 nPos = GetEntryPos( nStyle );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return IsEntryPosSelected( nPos );
    else
        return sal_False;
}

inline void LineListBox::SetColor( const Color& rColor )
{
    aColor = rColor;

    UpdateEntries( m_nWidth );
}

inline Color LineListBox::GetColor( void ) const
{
    return aColor;
}

const Color& LineListBox::GetPaintColor( void ) const
{
    return maPaintCol;
}

inline void LineListBox::SetWidth( long nWidth )
{
    long nOldWidth = m_nWidth;
    m_nWidth = nWidth;
    UpdateEntries( nOldWidth );
}

inline void LineListBox::SetNone( const XubString& sNone )
{
    m_sNone = sNone;
}

// ---------------
// - FontNameBox -
// ---------------

class SVT_DLLPUBLIC FontNameBox : public ComboBox
{
private:
    ImplFontList*   mpFontList;
    Image           maImagePrinterFont;
    Image           maImageBitmapFont;
    Image           maImageScalableFont;
    sal_Bool        mbWYSIWYG;
    sal_Bool        mbSymbols;
    String         maFontMRUEntriesFile;

#ifdef _CTRLBOX_CXX
    SVT_DLLPRIVATE void         ImplCalcUserItemSize();
    SVT_DLLPRIVATE void         ImplDestroyFontList();
#endif

    void            InitBitmaps( void );
protected:
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    void            LoadMRUEntries( const String& aFontMRUEntriesFile, xub_Unicode cSep = ';' );
    void            SaveMRUEntries( const String& aFontMRUEntriesFile, xub_Unicode cSep = ';' ) const;
public:
                    FontNameBox( Window* pParent,
                                 WinBits nWinStyle = WB_SORT );
                    FontNameBox( Window* pParent, const ResId& rResId );
    virtual         ~FontNameBox();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    void            Fill( const FontList* pList );

    void            EnableWYSIWYG( sal_Bool bEnable = sal_True );
    sal_Bool            IsWYSIWYGEnabled() const { return mbWYSIWYG; }

    void            EnableSymbols( sal_Bool bEnable = sal_True );
    sal_Bool            IsSymbolsEnabled() const { return mbSymbols; }

private:
    void            InitFontMRUEntriesFile();

    // declared as private because some compilers would generate the default functions
                    FontNameBox( const FontNameBox& );
    FontNameBox&    operator =( const FontNameBox& );
};

// ----------------
// - FontStyleBox -
// ----------------

class SVT_DLLPUBLIC FontStyleBox : public ComboBox
{
    XubString       aLastStyle;

private:
    using ComboBox::SetText;
public:
                    FontStyleBox( Window* pParent, const ResId& rResId );
                    FontStyleBox( Window* pParent, WinBits nBits );
    virtual         ~FontStyleBox();

    virtual void    Select();
    virtual void    LoseFocus();
    virtual void    Modify();

    void            SetText( const XubString& rText );
    void            Fill( const XubString& rName, const FontList* pList );

private:
    // declared as private because some compilers would generate the default functions
                    FontStyleBox( const FontStyleBox& );
    FontStyleBox&   operator =( const FontStyleBox& );
};

inline void FontStyleBox::SetText( const XubString& rText )
{
    aLastStyle = rText;
    ComboBox::SetText( rText );
}

// ---------------
// - FontSizeBox -
// ---------------

class SVT_DLLPUBLIC FontSizeBox : public MetricBox
{
    FontInfo        aFontInfo;
    const FontList* pFontList;
    sal_uInt16          nRelMin;
    sal_uInt16          nRelMax;
    sal_uInt16          nRelStep;
    short           nPtRelMin;
    short           nPtRelMax;
    short           nPtRelStep;
    sal_Bool            bRelativeMode:1,
                    bRelative:1,
                    bPtRelative:1,
                    bStdSize:1;

#ifdef _CTRLBOX_CXX
    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit();
#endif

protected:
    virtual XubString CreateFieldText( sal_Int64 nValue ) const;

public:
                    FontSizeBox( Window* pParent, WinBits nWinStyle = 0 );
                    FontSizeBox( Window* pParent, const ResId& rResId );
    virtual         ~FontSizeBox();

    void            Reformat();
    void            Modify();

    void            Fill( const FontInfo* pInfo, const FontList* pList );

    void            EnableRelativeMode( sal_uInt16 nMin = 50, sal_uInt16 nMax = 150,
                                        sal_uInt16 nStep = 5 );
    void            EnablePtRelativeMode( short nMin = -200, short nMax = 200,
                                          short nStep = 10 );
    sal_Bool            IsRelativeMode() const { return bRelativeMode; }
    void            SetRelative( sal_Bool bRelative = sal_False );
    sal_Bool            IsRelative() const { return bRelative; }
    void            SetPtRelative( sal_Bool bPtRel = sal_True )
                        { bPtRelative = bPtRel; SetRelative( sal_True ); }
    sal_Bool            IsPtRelative() const { return bPtRelative; }

    virtual void    SetValue( sal_Int64 nNewValue, FieldUnit eInUnit );
    virtual void    SetValue( sal_Int64 nNewValue  );
    virtual sal_Int64   GetValue( FieldUnit eOutUnit ) const;
    virtual sal_Int64   GetValue() const;

private:
    // declared as private because some compilers would generate the default functions
                    FontSizeBox( const FontSizeBox& );
    FontSizeBox&    operator =( const FontSizeBox& );
};

#endif  // _CTRLBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
