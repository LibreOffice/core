/*************************************************************************
 *
 *  $RCSfile: ctrlbox.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: th $ $Date: 2001-03-09 15:42:06 $
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

#ifndef _CTRLBOX_HXX
#define _CTRLBOX_HXX

#ifndef _INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _FIELD_HXX
#include <vcl/field.hxx>
#endif

class ImplFontList;
class ImpColorList;
class ImpLineList;
class FontList;

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

class ColorListBox : public ListBox
{
    ImpColorList*   pColorList; // Separate Liste, falls UserDaten von aussen verwendet werden.
    Size            aImageSize;

#ifdef _CTRLBOX_CXX
    void            ImplInit();
    void            ImplDestroyColorEntries();
#endif

public:
                    ColorListBox( Window* pParent,
                                  WinBits nWinStyle = WB_BORDER );
                    ColorListBox( Window* pParent, const ResId& rResId );
    virtual         ~ColorListBox();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    USHORT          InsertEntry( const XubString& rStr,
                                 USHORT nPos = LISTBOX_APPEND );
    USHORT          InsertEntry( const Color& rColor, const XubString& rStr,
                                 USHORT nPos = LISTBOX_APPEND );
    void            RemoveEntry( USHORT nPos );
    void            Clear();
    void            CopyEntries( const ColorListBox& rBox );

    USHORT          GetEntryPos( const XubString& rStr ) const
                        { return ListBox::GetEntryPos( rStr ); }

    USHORT          GetEntryPos( const Color& rColor ) const;
    Color           GetEntryColor( USHORT nPos ) const;
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
    USHORT          GetEntryPos( const void* pData ) const;
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

class LineListBox : public ListBox
{
    ImpLineList*    pLineList;
    VirtualDevice   aVirDev;
    International   aIntn;
    Size            aTxtSize;
    Color           aColor;
    FieldUnit       eUnit;
    FieldUnit       eSourceUnit;

#ifdef _CTRLBOX_CXX
    void            ImpGetLine( long nLine1, long nLine2, long nDistance,
                                Bitmap& rBmp, XubString& rStr );
    void            ImplInit();
#endif

public:
                    LineListBox( Window* pParent,
                                 WinBits nWinStyle = WB_BORDER );
                    LineListBox( Window* pParent, const ResId& rResId );
    virtual         ~LineListBox();

    USHORT          InsertEntry( const XubString& rStr,
                                 USHORT nPos = LISTBOX_APPEND );
    USHORT          InsertEntry( long nLine1, long nLine2 = 0,
                                 long nDistance = 0,
                                 USHORT nPos = LISTBOX_APPEND );
    void            RemoveEntry( USHORT nPos );
    void            Clear();

    USHORT          GetEntryPos( const XubString& rStr ) const
                        { return ListBox::GetEntryPos( rStr ); }
    USHORT          GetEntryPos( long nLine1, long nLine2 = 0,
                                 long nDistance = 0 ) const;
    long            GetEntryLine1( USHORT nPos ) const;
    long            GetEntryLine2( USHORT nPos ) const;
    long            GetEntryDistance( USHORT nPos ) const;

    void            SelectEntry( const XubString& rStr, BOOL bSelect = TRUE )
                        { ListBox::SelectEntry( rStr, bSelect ); }
    void            SelectEntry( long nLine1, long nLine2 = 0,
                                 long nDistance = 0, BOOL bSelect = TRUE );
    long            GetSelectEntryLine1( USHORT nSelIndex = 0 ) const;
    long            GetSelectEntryLine2( USHORT nSelIndex = 0 ) const;
    long            GetSelectEntryDistance( USHORT nSelIndex = 0 ) const;
    BOOL            IsEntrySelected( const XubString& rStr ) const
                        { return ListBox::IsEntrySelected( rStr ); }
    BOOL            IsEntrySelected( long nLine1, long nLine2 = 0,
                                     long nDistance = 0 ) const;

    void            SetUnit( FieldUnit eNewUnit ) { eUnit = eNewUnit; }
    FieldUnit       GetUnit() const { return eUnit; }
    void            SetSourceUnit( FieldUnit eNewUnit ) { eSourceUnit = eNewUnit; }
    FieldUnit       GetSourceUnit() const { return eSourceUnit; }

    void            SetColor( const Color& rColor );
    Color           GetColor() const { return aColor; }

private:
    // declared as private because some compilers would generate the default functions
                    LineListBox( const LineListBox& );
    LineListBox&    operator =( const LineListBox& );
    USHORT          GetEntryPos( const void* pData ) const;
    void            SetEntryData( USHORT nPos, void* pNewData );
    void*           GetEntryData( USHORT nPos ) const;
};

inline void LineListBox::SelectEntry( long nLine1, long nLine2,
                                      long nDistance, BOOL bSelect )
{
    USHORT nPos = GetEntryPos( nLine1, nLine2, nDistance );
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

inline BOOL LineListBox::IsEntrySelected( long nLine1, long nLine2,
                                          long nDistance ) const
{
    USHORT nPos = GetEntryPos( nLine1, nLine2, nDistance );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return IsEntryPosSelected( nPos );
    else
        return FALSE;
}

// ---------------
// - FontNameBox -
// ---------------

class FontNameBox : public ComboBox
{
private:
    ImplFontList*   mpFontList;
    Image           maImagePrinterFont;
    Image           maImageBitmapFont;
    Image           maImageScalableFont;
    BOOL            mbWYSIWYG;
    BOOL            mbSymbols;

#ifdef _CTRLBOX_CXX
    void            ImplCalcUserItemSize();
    void            ImplDestroyFontList();
#endif

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

class FontStyleBox : public ComboBox
{
    XubString       aLastStyle;

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

class FontSizeBox : public MetricBox
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
    void            ImplInit();
#endif

protected:
    virtual XubString CreateFieldText( long nValue ) const;

public:
                    FontSizeBox( Window* pParent, WinBits nWinStyle = 0 );
                    FontSizeBox( Window* pParent, const ResId& rResId );
    virtual         ~FontSizeBox();

    void            Reformat();
    void            Modify();

    void            Fill( const FontInfo& rInfo, const FontList* pList );

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

    virtual void    SetValue( long nNewValue, FieldUnit eInUnit );
    virtual void    SetValue( long nNewValue  );
    virtual long    GetValue( FieldUnit eOutUnit ) const;
    virtual long    GetValue() const;
    long            GetValue( USHORT nPos, FieldUnit eOutUnit ) const;
    void            SetUserValue( long nNewValue, FieldUnit eInUnit );
    void            SetUserValue( long nNewValue ) { SetUserValue( nNewValue, FUNIT_NONE ); }

private:
    // declared as private because some compilers would generate the default functions
                    FontSizeBox( const FontSizeBox& );
    FontSizeBox&    operator =( const FontSizeBox& );
};

#endif  // _CTRLBOX_HXX
