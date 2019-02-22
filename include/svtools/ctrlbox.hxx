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
#include <vcl/metric.hxx>
#include <vcl/field.hxx>
#include <vcl/weld.hxx>

#include <memory>

namespace weld { class CustomWeld; }

class VirtualDevice;
class BorderWidthImpl;
class FontList;
class ImpLineListData;
enum class SvxBorderLineStyle : sal_Int16;

typedef ::std::vector< FontMetric         > ImplFontList;

/*************************************************************************

class LineListBox

Description

Allows selection of line styles and sizes. Note that before first insert,
units and window size need to be set. Supported units are typographic point
(pt) and millimeters (mm). For SourceUnit, pt, mm and twips are supported.
All scalar numbers in 1/100 of the corresponding unit.

Line1 is the outer, Line2 the inner line, Distance is the distance between
these two lines. If Line2 == 0, only Line1 will be shown. Defaults for
source and target unit are FieldUnit::POINT.

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

For DontKnow the FontSizeBox should be filled FontMetric(), so it will
contain an list with the standard sizes. Th currently shown size
probably needs to be reset by the application.

See also

FontList; FontNameBox; FontStyleBox; FontSizeMenu

*************************************************************************/

inline Color sameColor( Color rMain )
{
    return rMain;
}

inline Color sameDistColor( Color /*rMain*/, Color rDefault )
{
    return rDefault;
}

class SVT_DLLPUBLIC LineListBox final : public ListBox
{
public:
    typedef Color (*ColorFunc)(Color);
    typedef Color (*ColorDistFunc)(Color, Color);

                    LineListBox( vcl::Window* pParent, WinBits nWinStyle = WB_BORDER );
    virtual         ~LineListBox() override;
    virtual void    dispose() override;

    /** Set the width in Twips */
    void            SetWidth( long nWidth );
    long            GetWidth() const { return m_nWidth; }
    void            SetNone( const OUString& sNone );

    using ListBox::InsertEntry;
    /** Insert a listbox entry with all widths in Twips. */
    void            InsertEntry(const BorderWidthImpl& rWidthImpl,
                        SvxBorderLineStyle nStyle, long nMinWidth = 0,
                        ColorFunc pColor1Fn = &sameColor,
                        ColorFunc pColor2Fn = &sameColor,
                        ColorDistFunc pColorDistFn = &sameDistColor);

    SvxBorderLineStyle GetEntryStyle( sal_Int32 nPos ) const;

    SvxBorderLineStyle GetSelectEntryStyle() const;

    void            SetSourceUnit( FieldUnit eNewUnit ) { eSourceUnit = eNewUnit; }

    const Color&    GetColor() const { return aColor; }

private:

    SVT_DLLPRIVATE void         ImpGetLine( long nLine1, long nLine2, long nDistance,
                                    Color nColor1, Color nColor2, Color nColorDist,
                                    SvxBorderLineStyle nStyle, BitmapEx& rBmp );
    using Window::ImplInit;
    void            UpdatePaintLineColor();       // returns sal_True if maPaintCol has changed
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    void            UpdateEntries( long nOldWidth );
    sal_Int32       GetStylePos( sal_Int32  nListPos, long nWidth );

    inline const Color&    GetPaintColor() const;
    Color   GetColorLine1( sal_Int32  nPos );
    Color   GetColorLine2( sal_Int32  nPos );
    Color   GetColorDist( sal_Int32  nPos );

                    LineListBox( const LineListBox& ) = delete;
    LineListBox&    operator =( const LineListBox& ) = delete;

    std::vector<std::unique_ptr<ImpLineListData>> m_vLineList;
    long            m_nWidth;
    OUString        m_sNone;
    ScopedVclPtr<VirtualDevice>   aVirDev;
    Size            aTxtSize;
    Color const     aColor;
    Color           maPaintCol;
    FieldUnit       eSourceUnit;
};

const Color& LineListBox::GetPaintColor() const
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

class SvtValueSet;

class SVT_DLLPUBLIC SvtLineListBox
{
public:
    typedef Color (*ColorFunc)(Color);
    typedef Color (*ColorDistFunc)(Color, Color);

    SvtLineListBox(std::unique_ptr<weld::MenuButton> pControl);
    ~SvtLineListBox();

    /** Set the width in Twips */
    void SetWidth(long nWidth)
    {
        m_nWidth = nWidth;
        UpdateEntries();
        UpdatePreview();
    }

    long GetWidth() const { return m_nWidth; }

    /** Insert a listbox entry with all widths in Twips. */
    void            InsertEntry(const BorderWidthImpl& rWidthImpl,
                        SvxBorderLineStyle nStyle, long nMinWidth = 0,
                        ColorFunc pColor1Fn = &sameColor,
                        ColorFunc pColor2Fn = &sameColor,
                        ColorDistFunc pColorDistFn = &sameDistColor);

    void            SelectEntry( SvxBorderLineStyle nStyle );
    SvxBorderLineStyle GetSelectEntryStyle() const;

    void            SetSourceUnit( FieldUnit eNewUnit ) { eSourceUnit = eNewUnit; }

    void            SetColor( const Color& rColor )
    {
        aColor = rColor;
        UpdateEntries();
        UpdatePreview();
    }

    const Color&    GetColor() const { return aColor; }

    void            SetSelectHdl(const Link<SvtLineListBox&,void>& rLink) { maSelectHdl = rLink; }

    void            set_sensitive(bool bSensitive) { m_xControl->set_sensitive(bSensitive); }

private:

    SVT_DLLPRIVATE void         ImpGetLine( long nLine1, long nLine2, long nDistance,
                                    Color nColor1, Color nColor2, Color nColorDist,
                                    SvxBorderLineStyle nStyle, BitmapEx& rBmp );

    void            UpdatePaintLineColor();       // returns sal_True if maPaintCol has changed
    DECL_LINK(StyleUpdated, weld::Widget&, void);
    DECL_LINK(ValueSelectHdl, SvtValueSet*, void);
    DECL_LINK(FocusHdl, weld::Widget&, void);
    DECL_LINK(NoneHdl, weld::Button&, void);

    void            UpdateEntries();
    sal_Int32       GetStylePos(sal_Int32 nListPos);

    const Color&    GetPaintColor() const
    {
        return maPaintCol;
    }
    Color   GetColorLine1( sal_Int32  nPos );
    Color   GetColorLine2( sal_Int32  nPos );
    Color   GetColorDist( sal_Int32  nPos );

    void UpdatePreview();

                    SvtLineListBox( const SvtLineListBox& ) = delete;
    SvtLineListBox&    operator =( const SvtLineListBox& ) = delete;

    std::unique_ptr<weld::MenuButton> m_xControl;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Widget> m_xTopLevel;
    std::unique_ptr<weld::Button> m_xNoneButton;
    std::unique_ptr<SvtValueSet> m_xLineSet;
    std::unique_ptr<weld::CustomWeld> m_xLineSetWin;

    std::vector<std::unique_ptr<ImpLineListData>> m_vLineList;
    long            m_nWidth;
    ScopedVclPtr<VirtualDevice>   aVirDev;
    Color           aColor;
    Color           maPaintCol;
    FieldUnit       eSourceUnit;
    Link<SvtLineListBox&,void> maSelectHdl;
};

class SVT_DLLPUBLIC SvtCalendarBox
{
public:
    SvtCalendarBox(std::unique_ptr<weld::MenuButton> pControl);
    ~SvtCalendarBox();

    weld::MenuButton& get_button() { return *m_xControl; }

    void set_date(const Date& rDate);
    Date get_date() const { return m_xCalendar->get_date(); }

    void set_sensitive(bool bSensitive) { m_xControl->set_sensitive(bSensitive); }
    bool get_sensitive() const { return m_xControl->get_sensitive(); }
    void grab_focus() { m_xControl->grab_focus(); }
private:
    DECL_LINK(SelectHdl, weld::Calendar&, void);
    DECL_LINK(ActivateHdl, weld::Calendar&, void);

    std::unique_ptr<weld::MenuButton> m_xControl;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Widget> m_xTopLevel;
    std::unique_ptr<weld::Calendar> m_xCalendar;
};

class SVT_DLLPUBLIC FontNameBox : public ComboBox
{
private:
    std::unique_ptr<ImplFontList> mpFontList;
    bool            mbWYSIWYG;
    OUString        maFontMRUEntriesFile;

    SVT_DLLPRIVATE void         ImplCalcUserItemSize();
    SVT_DLLPRIVATE void         ImplDestroyFontList();

protected:
    void            LoadMRUEntries( const OUString& aFontMRUEntriesFile );
    void            SaveMRUEntries( const OUString& aFontMRUEntriesFile ) const;
public:
                    FontNameBox( vcl::Window* pParent,
                                 WinBits nWinStyle );
    virtual         ~FontNameBox() override;
    virtual void    dispose() override;

    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) override;

    void            Fill( const FontList* pList );

    void            EnableWYSIWYG( bool bEnable );

private:
    void            InitFontMRUEntriesFile();

                    FontNameBox( const FontNameBox& ) = delete;
    FontNameBox&    operator =( const FontNameBox& ) = delete;
};

class SVT_DLLPUBLIC FontStyleBox : public ComboBox
{
    Size            aOptimalSize;

public:
    FontStyleBox( vcl::Window* pParent, WinBits nBits );

    virtual void    Modify() override;
    virtual Size    GetOptimalSize() const override;

private:
                    FontStyleBox( const FontStyleBox& ) = delete;
    FontStyleBox&   operator =( const FontStyleBox& ) = delete;
};

class SVT_DLLPUBLIC SvtFontStyleBox
{
    std::unique_ptr<weld::ComboBox> m_xComboBox;
public:
    SvtFontStyleBox(std::unique_ptr<weld::ComboBox> p);

    void Fill(const OUString& rName, const FontList* pList);

    void connect_changed(const Link<weld::ComboBox&, void>& rLink) { m_xComboBox->connect_changed(rLink); }
    OUString get_active_text() const { return m_xComboBox->get_active_text(); }
    void set_active_text(const OUString& rText) { m_xComboBox->set_active_text(rText); }
    void append_text(const OUString& rStr) { m_xComboBox->append_text(rStr); }
    void set_sensitive(bool bSensitive) { m_xComboBox->set_sensitive(bSensitive); }
    void save_value() { m_xComboBox->save_value(); }
    OUString const& get_saved_value() const { return m_xComboBox->get_saved_value(); }
    int get_count() const { return m_xComboBox->get_count(); }
    int find_text(const OUString& rStr) const { return m_xComboBox->find_text(rStr); }
private:
    SvtFontStyleBox(const SvtFontStyleBox& ) = delete;
    SvtFontStyleBox& operator=(const SvtFontStyleBox&) = delete;
};

class SVT_DLLPUBLIC FontSizeBox : public MetricBox
{
    bool             bStdSize:1;

    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit();

protected:
    virtual sal_Int64 GetValueFromStringUnit(const OUString& rStr, FieldUnit eOutUnit) const override;

public:
                    FontSizeBox( vcl::Window* pParent, WinBits nWinStyle );

    void            Reformat() override;

    void            Fill( const FontMetric* pFontMetric, const FontList* pList );

    virtual void    SetValue( sal_Int64 nNewValue, FieldUnit eInUnit ) override;
    virtual void    SetValue( sal_Int64 nNewValue  ) override;

private:
                    FontSizeBox( const FontSizeBox& ) = delete;
    FontSizeBox&    operator =( const FontSizeBox& ) = delete;
};

class SVT_DLLPUBLIC SvtFontSizeBox
{
    FontMetric      aFontMetric;
    const FontList* pFontList;
    int             nSavedValue;
    int             nMin;
    int             nMax;
    FieldUnit       eUnit;
    sal_uInt16      nDecimalDigits;
    sal_uInt16      nRelMin;
    sal_uInt16      nRelMax;
    sal_uInt16      nRelStep;
    short           nPtRelMin;
    short           nPtRelMax;
    short           nPtRelStep;
    bool            bRelativeMode:1,
                    bRelative:1,
                    bPtRelative:1,
                    bStdSize:1;
    Link<weld::ComboBox&, void> m_aChangeHdl;
    std::unique_ptr<weld::ComboBox> m_xComboBox;

    sal_uInt16 GetDecimalDigits() const { return nDecimalDigits; }
    void SetDecimalDigits(sal_uInt16 nDigits) { nDecimalDigits = nDigits; }
    FieldUnit GetUnit() const { return eUnit; }
    void SetUnit(FieldUnit _eUnit) { eUnit = _eUnit; }
    void SetRange(int nNewMin, int nNewMax) { nMin = nNewMin; nMax = nNewMax; }
    void SetValue(int nNewValue, FieldUnit eInUnit);

    void InsertValue(int i);

    OUString format_number(int nValue) const;

    DECL_LINK(ModifyHdl, weld::ComboBox&, void);
    DECL_LINK(ReformatHdl, weld::Widget&, void);
public:
    SvtFontSizeBox(std::unique_ptr<weld::ComboBox> p);

    void Fill(const FontMetric* pFontMetric, const FontList* pList);

    void EnableRelativeMode(sal_uInt16 nMin, sal_uInt16 nMax, sal_uInt16 nStep = 5);
    void EnablePtRelativeMode(short nMin, short nMax, short nStep = 10);
    bool IsRelativeMode() const { return bRelativeMode; }
    void SetRelative( bool bRelative );
    bool IsRelative() const { return bRelative; }
    void SetPtRelative( bool bPtRel )
    {
        bPtRelative = bPtRel;
        SetRelative(true);
    }
    bool IsPtRelative() const { return bPtRelative; }

    void connect_changed(const Link<weld::ComboBox&, void>& rLink) { m_aChangeHdl = rLink; }
    OUString get_active_text() const { return m_xComboBox->get_active_text(); }
    void set_active_text(const OUString& rText) { m_xComboBox->set_active_text(rText); }
    void set_sensitive(bool bSensitive) { m_xComboBox->set_sensitive(bSensitive); }
    int get_value() const;
    void set_value(int nValue);
    void save_value() { nSavedValue = get_value(); }
    int get_saved_value() const { return nSavedValue; }

private:
    SvtFontSizeBox(const SvtFontSizeBox&) = delete;
    SvtFontSizeBox& operator=(const SvtFontSizeBox&) = delete;
};


#endif // INCLUDED_SVTOOLS_CTRLBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
