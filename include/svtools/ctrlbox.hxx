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

#ifndef INCLUDED_SVTOOLS_CTRLBOX_HXX
#define INCLUDED_SVTOOLS_CTRLBOX_HXX

#include <svtools/svtdllapi.h>

#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include <vcl/field.hxx>

#include <com/sun/star/table/BorderLineStyle.hpp>

class FontList;
class ImplColorListData;
class ImpLineListData;

typedef ::std::vector< ImplColorListData*    > ImpColorList;
typedef ::std::vector< ImpLineListData*      > ImpLineList;
typedef ::std::vector< vcl::FontInfo         > ImplFontList;

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

For DontKnow, the FontStyleBox should be filled with OUString(),
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

class SVT_DLLPUBLIC ColorListBox : public ListBox
{
    ImpColorList*   pColorList; // separate liste, in case of user data are required from outside
    Size            aImageSize;

    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit();
    SVT_DLLPRIVATE void         ImplDestroyColorEntries();

public:
                    ColorListBox( vcl::Window* pParent,
                                  WinBits nWinStyle = WB_BORDER );
                    ColorListBox( vcl::Window* pParent, const ResId& rResId );
    virtual         ~ColorListBox();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) SAL_OVERRIDE;

    using ListBox::InsertEntry;
    virtual sal_Int32   InsertEntry( const OUString& rStr,
                                 sal_Int32  nPos = LISTBOX_APPEND );
    virtual sal_Int32   InsertEntry( const Color& rColor, const OUString& rStr,
                                 sal_Int32  nPos = LISTBOX_APPEND );
    void            InsertAutomaticEntryColor(const Color &rAutoColorValue);
    bool            IsAutomaticSelected() { return !GetSelectEntryPos(); }
    using ListBox::RemoveEntry;
    virtual void    RemoveEntry( sal_Int32  nPos );
    virtual void    Clear();
    void            CopyEntries( const ColorListBox& rBox );

    using ListBox::GetEntryPos;
    virtual sal_Int32   GetEntryPos( const Color& rColor ) const;
    virtual Color   GetEntryColor( sal_Int32  nPos ) const;
    Size            GetImageSize() const { return aImageSize; }

    void            SelectEntry( const OUString& rStr, bool bSelect = true )
                        { ListBox::SelectEntry( rStr, bSelect ); }
    void            SelectEntry( const Color& rColor, bool bSelect = true );
    Color           GetSelectEntryColor( sal_Int32  nSelIndex = 0 ) const;
    bool            IsEntrySelected(const OUString& rStr ) const
    {
        return ListBox::IsEntrySelected(rStr);
    }

    bool            IsEntrySelected(const Color& rColor) const
    {
        sal_Int32  nPos = GetEntryPos( rColor );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            return IsEntryPosSelected( nPos );
        else
            return false;
    }

private:
    // declared as private because some compilers would generate the default functions
                    ColorListBox( const ColorListBox& );
    ColorListBox&   operator =( const ColorListBox& );

    void            SetEntryData( sal_Int32  nPos, void* pNewData );
    void*           GetEntryData( sal_Int32  nPos ) const;
};

inline void ColorListBox::SelectEntry( const Color& rColor, bool bSelect )
{
    sal_Int32  nPos = GetEntryPos( rColor );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        ListBox::SelectEntryPos( nPos, bSelect );
}

inline Color ColorListBox::GetSelectEntryColor( sal_Int32  nSelIndex ) const
{
    sal_Int32  nPos = GetSelectEntryPos( nSelIndex );
    Color aColor;
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        aColor = GetEntryColor( nPos );
    return aColor;
}

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

inline Color sameColor( Color rMain )
{
    return rMain;
}

inline Color sameDistColor( Color /*rMain*/, Color rDefault )
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
    bool            UpdatePaintLineColor( void );       // returns sal_True if maPaintCol has changed
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    void            UpdateEntries( long nOldWidth );
    sal_Int32       GetStylePos( sal_Int32  nListPos, long nWidth );

public:
    typedef Color (*ColorFunc)(Color);
    typedef Color (*ColorDistFunc)(Color, Color);

                    LineListBox( vcl::Window* pParent, WinBits nWinStyle = WB_BORDER );
    virtual         ~LineListBox();

    /** Set the width in Twips */
    void            SetWidth( long nWidth );
    long            GetWidth() const { return m_nWidth; }
    void            SetNone( const OUString& sNone );

    using ListBox::InsertEntry;
    virtual sal_Int32   InsertEntry( const OUString& rStr, sal_Int32  nPos = LISTBOX_APPEND );
    /** Insert a listbox entry with all widths in Twips. */
    void            InsertEntry( BorderWidthImpl aWidthImpl,
                        sal_uInt16 nStyle, long nMinWidth = 0,
                        ColorFunc pColor1Fn = &sameColor,
                        ColorFunc pColor2Fn = &sameColor,
                        ColorDistFunc pColorDistFn = &sameDistColor );

    using ListBox::RemoveEntry;
    virtual void    RemoveEntry( sal_Int32  nPos );
    virtual void    Clear();

    using ListBox::GetEntryPos;
    virtual sal_Int32  GetEntryPos( sal_uInt16 nStyle = com::sun::star::table::BorderLineStyle::SOLID ) const;
    sal_uInt16          GetEntryStyle( sal_Int32  nPos ) const;

    void            SelectEntry( const OUString& rStr, bool bSelect = true ) { ListBox::SelectEntry( rStr, bSelect ); }
    void SelectEntry( sal_uInt16 nStyle = com::sun::star::table::BorderLineStyle::SOLID, bool bSelect = true );
    sal_uInt16          GetSelectEntryStyle( sal_Int32  nSelIndex = 0 ) const;
    bool            IsEntrySelected(const OUString& rStr) const
    {
        return ListBox::IsEntrySelected(rStr);
    }
    bool IsEntrySelected( sal_uInt16 nStyle = com::sun::star::table::BorderLineStyle::SOLID ) const
    {
        sal_Int32  nPos = GetEntryPos( nStyle );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            return IsEntryPosSelected( nPos );
        else
            return false;
    }

    inline void     SetUnit( FieldUnit eNewUnit ) { eUnit = eNewUnit; }
    inline FieldUnit    GetUnit() const { return eUnit; }
    inline void     SetSourceUnit( FieldUnit eNewUnit ) { eSourceUnit = eNewUnit; }
    inline FieldUnit    GetSourceUnit() const { return eSourceUnit; }

    void            SetColor( const Color& rColor );
    Color           GetColor() const { return aColor; }

protected:

    inline const Color&    GetPaintColor( void ) const;
    Color   GetColorLine1( sal_Int32  nPos = 0 );
    Color   GetColorLine2( sal_Int32  nPos = 0 );
    Color   GetColorDist( sal_Int32  nPos = 0 );

private:
    // declared as private because some compilers would generate the default methods
                    LineListBox( const LineListBox& );
    LineListBox&    operator =( const LineListBox& );
    void            SetEntryData( sal_Int32  nPos, void* pNewData );
    void*           GetEntryData( sal_Int32  nPos ) const;
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

class SVT_DLLPUBLIC FontNameBox : public ComboBox
{
private:
    ImplFontList*   mpFontList;
    bool            mbWYSIWYG;
    OUString        maFontMRUEntriesFile;

    SVT_DLLPRIVATE void         ImplCalcUserItemSize();
    SVT_DLLPRIVATE void         ImplDestroyFontList();

protected:
    void            LoadMRUEntries( const OUString& aFontMRUEntriesFile, sal_Unicode cSep = ';' );
    void            SaveMRUEntries( const OUString& aFontMRUEntriesFile, sal_Unicode cSep = ';' ) const;
public:
                    FontNameBox( vcl::Window* pParent,
                                 WinBits nWinStyle = WB_SORT );
    virtual         ~FontNameBox();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) SAL_OVERRIDE;

    void            Fill( const FontList* pList );

    void            EnableWYSIWYG( bool bEnable = true );
    bool            IsWYSIWYGEnabled() const { return mbWYSIWYG; }

private:
    void            InitFontMRUEntriesFile();

    // declared as private because some compilers would generate the default functions
                    FontNameBox( const FontNameBox& );
    FontNameBox&    operator =( const FontNameBox& );
};

class SVT_DLLPUBLIC FontStyleBox : public ComboBox
{
    OUString        aLastStyle;

    Size            aOptimalSize;

private:
    using ComboBox::SetText;
public:
    FontStyleBox( vcl::Window* pParent, WinBits nBits );
    virtual         ~FontStyleBox();

    virtual void    Select() SAL_OVERRIDE;
    virtual void    LoseFocus() SAL_OVERRIDE;
    virtual void    Modify() SAL_OVERRIDE;
    virtual Size    GetOptimalSize() const SAL_OVERRIDE;

    void            SetText( const OUString& rText ) SAL_OVERRIDE;
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

class SVT_DLLPUBLIC FontSizeBox : public MetricBox
{
    vcl::FontInfo       aFontInfo;
    const FontList* pFontList;
    sal_uInt16          nRelMin;
    sal_uInt16          nRelMax;
    sal_uInt16          nRelStep;
    short           nPtRelMin;
    short           nPtRelMax;
    short           nPtRelStep;
    bool            bRelativeMode:1,
                    bRelative:1,
                    bPtRelative:1,
                    bStdSize:1;

    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit();

protected:
    virtual OUString CreateFieldText( sal_Int64 nValue ) const SAL_OVERRIDE;

public:
                    FontSizeBox( vcl::Window* pParent, WinBits nWinStyle = 0 );
    virtual         ~FontSizeBox();

    void            Reformat() SAL_OVERRIDE;
    void            Modify() SAL_OVERRIDE;

    void            Fill( const vcl::FontInfo* pInfo, const FontList* pList );

    void            EnableRelativeMode( sal_uInt16 nMin = 50, sal_uInt16 nMax = 150,
                                        sal_uInt16 nStep = 5 );
    void            EnablePtRelativeMode( short nMin = -200, short nMax = 200,
                                          short nStep = 10 );
    bool            IsRelativeMode() const { return bRelativeMode; }
    void            SetRelative( bool bRelative = false );
    bool            IsRelative() const { return bRelative; }
    void            SetPtRelative( bool bPtRel = true )
                        { bPtRelative = bPtRel; SetRelative( true ); }
    bool            IsPtRelative() const { return bPtRelative; }

    virtual void    SetValue( sal_Int64 nNewValue, FieldUnit eInUnit ) SAL_OVERRIDE;
    virtual void    SetValue( sal_Int64 nNewValue  ) SAL_OVERRIDE;
    virtual sal_Int64   GetValue( FieldUnit eOutUnit ) const SAL_OVERRIDE;
    virtual sal_Int64   GetValue() const SAL_OVERRIDE;

private:
    // declared as private because some compilers would generate the default functions
                    FontSizeBox( const FontSizeBox& );
    FontSizeBox&    operator =( const FontSizeBox& );
};

#endif // INCLUDED_SVTOOLS_CTRLBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
