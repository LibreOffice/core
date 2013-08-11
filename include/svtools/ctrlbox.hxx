/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

typedef ::std::vector< ImplColorListData*    > ImpColorList;
typedef ::std::vector< ImpLineListData*      > ImpLineList;
typedef ::std::vector< FontInfo              > ImplFontList;

// FIXME: STYLE_* duplicate values from editeng::SvxBorderStyle,
// which in turn duplicates values from com::sun::star::table::BorderLineStyle:
// this needs cleaning up on master
#define STYLE_SOLID                ( ( sal_uInt16 ) 0 )
#define STYLE_DOTTED               ( ( sal_uInt16 ) 1 )
#define STYLE_DASHED               ( ( sal_uInt16 ) 2 )
#define STYLE_FINE_DASHED          ( ( sal_uInt16 ) 14 )
#define STYLE_NONE                 ( ( sal_uInt16 ) -1)

#define CHANGE_LINE1               ( ( sal_uInt16 ) 1 )
#define CHANGE_LINE2               ( ( sal_uInt16 ) 2 )
#define CHANGE_DIST                ( ( sal_uInt16 ) 4 )
#define ADAPT_DIST                 ( ( sal_uInt16 ) 8 )



/*************************************************************************

Description
============

class ColorListBox

Description

Allows color selection

--------------------------------------------------------------------------

class LineListBox

Description

Allows selection of line styles and sizes. Not that before first insert,
units and window size need to be set. Supported units are typographic point
(pt) and millimeters (mm). For SourceUnit, pt, mm and twips are supported.
All scalar numbers in 1/100 of the corresponding unit.

Line1 is the outer, Line2 the inner line, Distance is the distance between
these two lines. If Line2 == 0, only Line1 will be shown. Defaults for
source and target unit are FUNIT_POINT.

SetColor() sets the line color.

Remarks

Contrary to a simple ListBox, user-specific data are not supported.
If UpdateMode is disabled, no data should be read, no selections
should be set, and the return code shall be ignore, as in these are
not defined in this mode. Also the bit WinBit WB_SORT may not be set.

--------------------------------------------------------------------------

class FontNameBox

Description

Allows selection of fonts. The ListBox will be filled using Fill parameter,
which is pointer to an FontList object.

Calling EnableWYSIWYG() enables rendering the font name in the currently
selected font.

See also

FontList; FontStyleBox; FontSizeBox; FontNameMenu

--------------------------------------------------------------------------

class FontStyleBox

Description

Allows select of FontStyle's. The parameter Fill points to a list
of available font styles for the font.

Reproduced styles are always added - this could change in future, as
potentially not all applications [Draw,Equation,FontWork] can properly
handle synthetic fonts. On filling, the previous name will be retained
if possible.

For DontKnow, the FontStyleBox should be filled with String(),
so it will contain a list with the default attributes. The currently
shown style probably needs to be reset by the application.

See also

FontList; FontNameBox; FontSizeBox;

--------------------------------------------------------------------------

class FontSizeBox

Description

Allows selection of font sizes. The values are retrieved via GetValue()
and set via SetValue(). The Fill parameter fills the ListBox with the
available sizes for the passed font.

All sizes are in 1/10 typographic point (pt).

The passed FontList must be retained until the next fill call.

Additionally it supports an relative mod, which allows entering
percentage values. This, eg., can be useful for template dialogs.
This mode can only be enabled, but not disabled again.

For DontKnow the FontSizeBox should be filled FontInfo(), so it will
contain an list with the standard sizes. Th currently shown size
probably needs to be reset by the application.

See also

FontList; FontNameBox; FontStyleBox; FontSizeMenu

*************************************************************************/

// ----------------
// - ColorListBox -
// ----------------

class SVT_DLLPUBLIC ColorListBox : public ListBox
{
    ImpColorList*   pColorList; // separate liste, in case of user data are required from outside
    Size            aImageSize;

    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit();
    SVT_DLLPRIVATE void         ImplDestroyColorEntries();

public:
                    ColorListBox( Window* pParent,
                                  WinBits nWinStyle = WB_BORDER );
                    ColorListBox( Window* pParent, const ResId& rResId );
    virtual         ~ColorListBox();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    using ListBox::InsertEntry;
    virtual sal_uInt16  InsertEntry( const OUString& rStr,
                                 sal_uInt16 nPos = LISTBOX_APPEND );
    virtual sal_uInt16  InsertEntry( const Color& rColor, const OUString& rStr,
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

    void            SelectEntry( const OUString& rStr, sal_Bool bSelect = sal_True )
                        { ListBox::SelectEntry( rStr, bSelect ); }
    void            SelectEntry( const Color& rColor, sal_Bool bSelect = sal_True );
    Color           GetSelectEntryColor( sal_uInt16 nSelIndex = 0 ) const;
    bool            IsEntrySelected(const OUString& rStr ) const
    {
        return ListBox::IsEntrySelected(rStr);
    }

    bool            IsEntrySelected(const Color& rColor) const
    {
        sal_uInt16 nPos = GetEntryPos( rColor );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            return IsEntryPosSelected( nPos );
        else
            return sal_False;
    }

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
    OUString        m_sNone;

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
    virtual         ~LineListBox();

    /** Set the width in Twips */
    void            SetWidth( long nWidth );
    long            GetWidth() const { return m_nWidth; }
    void            SetNone( const OUString& sNone );

    using ListBox::InsertEntry;
    virtual sal_uInt16  InsertEntry( const OUString& rStr, sal_uInt16 nPos = LISTBOX_APPEND );
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

    void            SelectEntry( const OUString& rStr, sal_Bool bSelect = sal_True ) { ListBox::SelectEntry( rStr, bSelect ); }
    void            SelectEntry( sal_uInt16 nStyle = STYLE_SOLID, sal_Bool bSelect = sal_True );
    sal_uInt16          GetSelectEntryStyle( sal_uInt16 nSelIndex = 0 ) const;
    bool            IsEntrySelected(const OUString& rStr) const
    {
        return ListBox::IsEntrySelected(rStr);
    }
    bool            IsEntrySelected(sal_uInt16 nStyle = STYLE_SOLID) const
    {
        sal_uInt16 nPos = GetEntryPos( nStyle );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            return IsEntryPosSelected( nPos );
        else
            return sal_False;
    }

    inline void     SetUnit( FieldUnit eNewUnit ) { eUnit = eNewUnit; }
    inline FieldUnit    GetUnit() const { return eUnit; }
    inline void     SetSourceUnit( FieldUnit eNewUnit ) { eSourceUnit = eNewUnit; }
    inline FieldUnit    GetSourceUnit() const { return eSourceUnit; }

    void            SetColor( const Color& rColor );
    Color           GetColor() const { return aColor; }

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

inline void LineListBox::SetColor( const Color& rColor )
{
    aColor = rColor;

    UpdateEntries( m_nWidth );
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

inline void LineListBox::SetNone( const OUString& sNone )
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
    sal_Bool        mbWYSIWYG;
    String          maFontMRUEntriesFile;

    SVT_DLLPRIVATE void         ImplCalcUserItemSize();
    SVT_DLLPRIVATE void         ImplDestroyFontList();

protected:
    void            LoadMRUEntries( const String& aFontMRUEntriesFile, sal_Unicode cSep = ';' );
    void            SaveMRUEntries( const String& aFontMRUEntriesFile, sal_Unicode cSep = ';' ) const;
public:
                    FontNameBox( Window* pParent,
                                 WinBits nWinStyle = WB_SORT );
    virtual         ~FontNameBox();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    void            Fill( const FontList* pList );

    void            EnableWYSIWYG( sal_Bool bEnable = sal_True );
    sal_Bool            IsWYSIWYGEnabled() const { return mbWYSIWYG; }

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
    OUString        aLastStyle;

private:
    using ComboBox::SetText;
public:
                    FontStyleBox( Window* pParent, const ResId& rResId );
                    FontStyleBox( Window* pParent, WinBits nBits );
    virtual         ~FontStyleBox();

    virtual void    Select();
    virtual void    LoseFocus();
    virtual void    Modify();

    void            SetText( const OUString& rText );
    void            Fill( const OUString& rName, const FontList* pList );

private:
    // declared as private because some compilers would generate the default functions
                    FontStyleBox( const FontStyleBox& );
    FontStyleBox&   operator =( const FontStyleBox& );
};

inline void FontStyleBox::SetText( const OUString& rText )
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

    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit();

protected:
    virtual OUString CreateFieldText( sal_Int64 nValue ) const;

public:
                    FontSizeBox( Window* pParent, WinBits nWinStyle = 0 );
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
    sal_Int64           GetValue( sal_uInt16 nPos, FieldUnit eOutUnit ) const;

private:
    // declared as private because some compilers would generate the default functions
                    FontSizeBox( const FontSizeBox& );
    FontSizeBox&    operator =( const FontSizeBox& );
};

#endif  // _CTRLBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
