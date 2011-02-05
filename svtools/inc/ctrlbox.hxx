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

#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include <vcl/field.hxx>

class ImplFontList;
class FontList;
class ImplColorListData;
class ImpLineListData;

typedef ::std::vector< ImplColorListData* > ImpColorList;
typedef ::std::vector< ImpLineListData* > ImpLineList;

#define STYLE_SOLID     ( ( USHORT ) 0 )
#define STYLE_DOTTED    ( ( USHORT ) 1 )
#define STYLE_DASHED    ( ( USHORT ) 2 )

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

FontList; FontNameBox; FontSizeBox; FontStyleMenu

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
    virtual USHORT  InsertEntry( const XubString& rStr,
                                 USHORT nPos = LISTBOX_APPEND );
    virtual USHORT  InsertEntry( const Color& rColor, const XubString& rStr,
                                 USHORT nPos = LISTBOX_APPEND );
    void            InsertAutomaticEntry();
    using ListBox::RemoveEntry;
    virtual void    RemoveEntry( USHORT nPos );
    virtual void    Clear();
    void            CopyEntries( const ColorListBox& rBox );

    using ListBox::GetEntryPos;
    virtual USHORT  GetEntryPos( const Color& rColor ) const;
    virtual Color   GetEntryColor( USHORT nPos ) const;
    Size            GetImageSize() const { return aImageSize; }

    void            SelectEntry( const XubString& rStr, BOOL bSelect = TRUE )
                        { ListBox::SelectEntry( rStr, bSelect ); }
    void            SelectEntry( const Color& rColor, BOOL bSelect = TRUE );
    XubString       GetSelectEntry( USHORT nSelIndex = 0 ) const
                        { return ListBox::GetSelectEntry( nSelIndex ); }
    Color           GetSelectEntryColor( USHORT nSelIndex = 0 ) const;
    BOOL            IsEntrySelected( const XubString& rStr ) const
                        { return ListBox::IsEntrySelected( rStr ); }

    BOOL            IsEntrySelected( const Color& rColor ) const;

private:
    // declared as private because some compilers would generate the default functions
                    ColorListBox( const ColorListBox& );
    ColorListBox&   operator =( const ColorListBox& );

    void            SetEntryData( USHORT nPos, void* pNewData );
    void*           GetEntryData( USHORT nPos ) const;
};

inline void ColorListBox::SelectEntry( const Color& rColor, BOOL bSelect )
{
    USHORT nPos = GetEntryPos( rColor );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        ListBox::SelectEntryPos( nPos, bSelect );
}

inline BOOL ColorListBox::IsEntrySelected( const Color& rColor ) const
{
    USHORT nPos = GetEntryPos( rColor );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return IsEntryPosSelected( nPos );
    else
        return FALSE;
}

inline Color ColorListBox::GetSelectEntryColor( USHORT nSelIndex ) const
{
    USHORT nPos = GetSelectEntryPos( nSelIndex );
    Color aColor;
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        aColor = GetEntryColor( nPos );
    return aColor;
}

// ---------------
// - LineListBox -
// ---------------

class SVT_DLLPUBLIC LineListBox : public ListBox
{
    ImpLineList*    pLineList;
    VirtualDevice   aVirDev;
    Size            aTxtSize;
    Color           aColor;
    Color           maPaintCol;
    FieldUnit       eUnit;
    FieldUnit       eSourceUnit;

    SVT_DLLPRIVATE void         ImpGetLine( long nLine1, long nLine2, long nDistance,
                                    USHORT nStyle, Bitmap& rBmp, XubString& rStr );
    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit();
    void            UpdateLineColors( void );
    BOOL            UpdatePaintLineColor( void );       // returns TRUE if maPaintCol has changed
    inline const Color& GetPaintColor( void ) const;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
                    LineListBox( Window* pParent, WinBits nWinStyle = WB_BORDER );
                    LineListBox( Window* pParent, const ResId& rResId );
    virtual         ~LineListBox();

    using ListBox::InsertEntry;
    virtual USHORT  InsertEntry( const XubString& rStr, USHORT nPos = LISTBOX_APPEND );
    virtual USHORT  InsertEntry( long nLine1, long nLine2 = 0, long nDistance = 0,
                                USHORT nStyle = STYLE_SOLID, USHORT nPos = LISTBOX_APPEND );
    using ListBox::RemoveEntry;
    virtual void    RemoveEntry( USHORT nPos );
    virtual void    Clear();

    using ListBox::GetEntryPos;
    USHORT          GetEntryPos( long nLine1, long nLine2 = 0, long nDistance = 0,
                                 USHORT nStyle = STYLE_SOLID ) const;
    long            GetEntryLine1( USHORT nPos ) const;
    long            GetEntryLine2( USHORT nPos ) const;
    long            GetEntryDistance( USHORT nPos ) const;
    USHORT          GetEntryStyle( USHORT nPos ) const;

    inline void     SelectEntry( const XubString& rStr, BOOL bSelect = TRUE ) { ListBox::SelectEntry( rStr, bSelect ); }
    void            SelectEntry( long nLine1, long nLine2 = 0, long nDistance = 0,
                                 USHORT nStyle = STYLE_SOLID, BOOL bSelect = TRUE );
    long            GetSelectEntryLine1( USHORT nSelIndex = 0 ) const;
    long            GetSelectEntryLine2( USHORT nSelIndex = 0 ) const;
    long            GetSelectEntryDistance( USHORT nSelIndex = 0 ) const;
    USHORT          GetSelectEntryStyle( USHORT nSelIndex = 0 ) const;
    inline BOOL     IsEntrySelected( const XubString& rStr ) const { return ListBox::IsEntrySelected( rStr ); }
    BOOL            IsEntrySelected( long nLine1, long nLine2 = 0, long nDistance = 0,
                                     USHORT nStyle1 = STYLE_SOLID ) const;

    inline void     SetUnit( FieldUnit eNewUnit ) { eUnit = eNewUnit; }
    inline FieldUnit    GetUnit() const { return eUnit; }
    inline void     SetSourceUnit( FieldUnit eNewUnit ) { eSourceUnit = eNewUnit; }
    inline FieldUnit    GetSourceUnit() const { return eSourceUnit; }

    void            SetColor( const Color& rColor );
    inline Color    GetColor( void ) const;

private:
    // declared as private because some compilers would generate the default methods
                    LineListBox( const LineListBox& );
    LineListBox&    operator =( const LineListBox& );
    void            SetEntryData( USHORT nPos, void* pNewData );
    void*           GetEntryData( USHORT nPos ) const;
};

inline void LineListBox::SelectEntry( long nLine1, long nLine2, long nDistance, USHORT nStyle, BOOL bSelect )
{
    USHORT nPos = GetEntryPos( nLine1, nLine2, nDistance, nStyle );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        ListBox::SelectEntryPos( nPos, bSelect );
}

inline long LineListBox::GetSelectEntryLine1( USHORT nSelIndex ) const
{
    USHORT nPos = GetSelectEntryPos( nSelIndex );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return GetEntryLine1( nPos );
    else
        return 0;
}

inline long LineListBox::GetSelectEntryLine2( USHORT nSelIndex ) const
{
    USHORT nPos = GetSelectEntryPos( nSelIndex );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return GetEntryLine2( nPos );
    else
        return 0;
}

inline long LineListBox::GetSelectEntryDistance( USHORT nSelIndex ) const
{
    USHORT nPos = GetSelectEntryPos( nSelIndex );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return GetEntryDistance( nPos );
    else
        return 0;
}

inline USHORT LineListBox::GetSelectEntryStyle( USHORT nSelIndex ) const
{
    USHORT nStyle = STYLE_SOLID;
    USHORT nPos = GetSelectEntryPos( nSelIndex );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nStyle = GetEntryStyle( nPos );

    return nStyle;
}

inline BOOL LineListBox::IsEntrySelected( long nLine1, long nLine2, long nDistance, USHORT nStyle ) const
{
    USHORT nPos = GetEntryPos( nLine1, nLine2, nDistance, nStyle );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return IsEntryPosSelected( nPos );
    else
        return FALSE;
}

inline void LineListBox::SetColor( const Color& rColor )
{
    aColor = rColor;

    UpdateLineColors();
}

inline Color LineListBox::GetColor( void ) const
{
    return aColor;
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
    BOOL            mbWYSIWYG;
    BOOL            mbSymbols;

#ifdef _CTRLBOX_CXX
    SVT_DLLPRIVATE void         ImplCalcUserItemSize();
    SVT_DLLPRIVATE void         ImplDestroyFontList();
#endif

    void            InitBitmaps( void );
protected:
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
public:
                    FontNameBox( Window* pParent,
                                 WinBits nWinStyle = WB_SORT );
                    FontNameBox( Window* pParent, const ResId& rResId );
    virtual         ~FontNameBox();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    void            Fill( const FontList* pList );

    void            EnableWYSIWYG( BOOL bEnable = TRUE );
    BOOL            IsWYSIWYGEnabled() const { return mbWYSIWYG; }

    void            EnableSymbols( BOOL bEnable = TRUE );
    BOOL            IsSymbolsEnabled() const { return mbSymbols; }

private:
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
                    FontStyleBox( Window* pParent, WinBits nWinStyle = 0 );
                    FontStyleBox( Window* pParent, const ResId& rResId );
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
    USHORT          nRelMin;
    USHORT          nRelMax;
    USHORT          nRelStep;
    short           nPtRelMin;
    short           nPtRelMax;
    short           nPtRelStep;
    BOOL            bRelativeMode:1,
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

    void            EnableRelativeMode( USHORT nMin = 50, USHORT nMax = 150,
                                        USHORT nStep = 5 );
    void            EnablePtRelativeMode( short nMin = -200, short nMax = 200,
                                          short nStep = 10 );
    BOOL            IsRelativeMode() const { return bRelativeMode; }
    void            SetRelative( BOOL bRelative = FALSE );
    BOOL            IsRelative() const { return bRelative; }
    void            SetPtRelative( BOOL bPtRel = TRUE )
                        { bPtRelative = bPtRel; SetRelative( TRUE ); }
    BOOL            IsPtRelative() const { return bPtRelative; }

    virtual void    SetValue( sal_Int64 nNewValue, FieldUnit eInUnit );
    virtual void    SetValue( sal_Int64 nNewValue  );
    virtual sal_Int64   GetValue( FieldUnit eOutUnit ) const;
    virtual sal_Int64   GetValue() const;
    sal_Int64           GetValue( USHORT nPos, FieldUnit eOutUnit ) const;
    void            SetUserValue( sal_Int64 nNewValue, FieldUnit eInUnit );
    void            SetUserValue( sal_Int64 nNewValue ) { SetUserValue( nNewValue, FUNIT_NONE ); }

private:
    // declared as private because some compilers would generate the default functions
                    FontSizeBox( const FontSizeBox& );
    FontSizeBox&    operator =( const FontSizeBox& );
};

#endif  // _CTRLBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
