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
#ifndef INCLUDED_STARMATH_INC_DIALOG_HXX
#define INCLUDED_STARMATH_INC_DIALOG_HXX

#include <vcl/image.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/combobox.hxx>
#include <svx/charmap.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/field.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/menu.hxx>
#include <vcl/outdev.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include "utility.hxx"
#include "format.hxx"
#include "symbol.hxx"
#include <memory>

class SubsetMap;
#define CATEGORY_NONE   0xFFFF

/**************************************************************************/

void SetFontStyle(const OUString &rStyleName, vcl::Font &rFont);

/**************************************************************************/

class SmPrintOptionsTabPage : public SfxTabPage
{
    std::unique_ptr<weld::CheckButton>      m_xTitle;
    std::unique_ptr<weld::CheckButton>      m_xText;
    std::unique_ptr<weld::CheckButton>      m_xFrame;
    std::unique_ptr<weld::RadioButton>      m_xSizeNormal;
    std::unique_ptr<weld::RadioButton>      m_xSizeScaled;
    std::unique_ptr<weld::RadioButton>      m_xSizeZoomed;
    std::unique_ptr<weld::MetricSpinButton> m_xZoom;
    std::unique_ptr<weld::CheckButton>      m_xNoRightSpaces;
    std::unique_ptr<weld::CheckButton>      m_xSaveOnlyUsedSymbols;
    std::unique_ptr<weld::CheckButton>      m_xAutoCloseBrackets;

    DECL_LINK(SizeButtonClickHdl, weld::ToggleButton&, void);

    virtual bool    FillItemSet(SfxItemSet* rSet) override;
    virtual void    Reset(const SfxItemSet* rSet) override;

public:
    static VclPtr<SfxTabPage> Create(TabPageParent pWindow, const SfxItemSet &rSet);

    SmPrintOptionsTabPage(TabPageParent pPage, const SfxItemSet &rOptions);
    virtual ~SmPrintOptionsTabPage() override;
};

/**************************************************************************/

class SmShowFont : public vcl::Window
{
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;

    vcl::Font maFont;

public:
    SmShowFont(vcl::Window *pParent, WinBits nStyle)
        : Window(pParent, nStyle)
    {
    }
    virtual Size GetOptimalSize() const override;
    void SetFont(const vcl::Font& rFont);
};

class SmFontDialog : public ModalDialog
{
    VclPtr<ComboBox>       m_pFontBox;
    VclPtr<VclContainer>   m_pAttrFrame;
    VclPtr<CheckBox>       m_pBoldCheckBox;
    VclPtr<CheckBox>       m_pItalicCheckBox;
    VclPtr<SmShowFont>     m_pShowFont;

    vcl::Font maFont;

    DECL_LINK(FontSelectHdl, ComboBox&, void);
    DECL_LINK(FontModifyHdl, Edit&, void);
    DECL_LINK(AttrChangeHdl, Button *, void);

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

public:
    SmFontDialog(vcl::Window * pParent, OutputDevice *pFntListDevice, bool bHideCheckboxes);
    virtual ~SmFontDialog() override;
    virtual void dispose() override;

    const vcl::Font& GetFont() const
    {
        return maFont;
    }
    void SetFont(const vcl::Font &rFont);
};

/**************************************************************************/

class SmFontSizeDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::MetricSpinButton> m_xBaseSize;
    std::unique_ptr<weld::MetricSpinButton> m_xTextSize;
    std::unique_ptr<weld::MetricSpinButton> m_xIndexSize;
    std::unique_ptr<weld::MetricSpinButton> m_xFunctionSize;
    std::unique_ptr<weld::MetricSpinButton> m_xOperatorSize;
    std::unique_ptr<weld::MetricSpinButton> m_xBorderSize;
    std::unique_ptr<weld::Button> m_xDefaultButton;

    DECL_LINK(DefaultButtonClickHdl, weld::Button&, void);

public:
    SmFontSizeDialog(weld::Window *pParent);
    virtual ~SmFontSizeDialog() override;

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat) const;
};

/**************************************************************************/

class SmFontTypeDialog : public ModalDialog
{
    VclPtr<SmFontPickListBox> m_pVariableFont;
    VclPtr<SmFontPickListBox> m_pFunctionFont;
    VclPtr<SmFontPickListBox> m_pNumberFont;
    VclPtr<SmFontPickListBox> m_pTextFont;
    VclPtr<SmFontPickListBox> m_pSerifFont;
    VclPtr<SmFontPickListBox> m_pSansFont;
    VclPtr<SmFontPickListBox> m_pFixedFont;
    VclPtr<MenuButton> m_pMenuButton;
    VclPtr<PushButton> m_pDefaultButton;

    VclPtr<OutputDevice> pFontListDev;

    DECL_LINK(MenuSelectHdl, Menu *, bool);
    DECL_LINK(DefaultButtonClickHdl, Button *, void);

public:
    SmFontTypeDialog(vcl::Window *pParent, OutputDevice *pFntListDevice);
    virtual ~SmFontTypeDialog() override;
    virtual void dispose() override;

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat) const;
};

/**************************************************************************/

#define NOCATEGORIES    10

class SmCategoryDesc
{
    OUString Name;
    OUString Strings[4];
    std::unique_ptr<weld::Widget> Graphics[4];    /* regular bitmaps */
    sal_uInt16 Minimum[4];
    sal_uInt16 Maximum[4];
    sal_uInt16 Value[4];

public:
    SmCategoryDesc(weld::Builder& rBuilder, sal_uInt16 nCategoryIdx);
    ~SmCategoryDesc();

    const OUString& GetName() const                 { return Name; }
    const OUString& GetString(sal_uInt16 Index) const   { return Strings[Index];  }
    sal_uInt16          GetMinimum(sal_uInt16 Index)        { return Minimum[Index]; }
    sal_uInt16          GetMaximum(sal_uInt16 Index)        { return Maximum[Index]; }
    sal_uInt16          GetValue(sal_uInt16 Index) const    { return Value[Index]; }
    void            SetValue(sal_uInt16 Index, sal_uInt16 nVal) { Value[Index] = nVal;}

    weld::Widget* GetGraphic(sal_uInt16 Index) const
    {
        return Graphics[Index].get();
    }
};

class SmDistanceDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Frame> m_xFrame;
    std::unique_ptr<weld::Label> m_xFixedText1;
    std::unique_ptr<weld::MetricSpinButton>    m_xMetricField1;
    std::unique_ptr<weld::Label> m_xFixedText2;
    std::unique_ptr<weld::MetricSpinButton>    m_xMetricField2;
    std::unique_ptr<weld::Label> m_xFixedText3;
    std::unique_ptr<weld::MetricSpinButton>    m_xMetricField3;
    std::unique_ptr<weld::CheckButton> m_xCheckBox1;
    std::unique_ptr<weld::Label> m_xFixedText4;
    std::unique_ptr<weld::MetricSpinButton> m_xMetricField4;
    std::unique_ptr<weld::MenuButton> m_xMenuButton;
    std::unique_ptr<weld::Button> m_xDefaultButton;
    std::unique_ptr<weld::Widget> m_xBitmap;

    weld::Widget* m_pCurrentImage;

    SmCategoryDesc *Categories[NOCATEGORIES];
    sal_uInt16          nActiveCategory;
    bool            bScaleAllBrackets;

    DECL_LINK(GetFocusHdl, weld::Widget&, void);
    DECL_LINK(MenuSelectHdl, const OString&, void);
    DECL_LINK(DefaultButtonClickHdl, weld::Button&, void);
    DECL_LINK(CheckBoxClickHdl, weld::ToggleButton&, void);

    void    SetCategory(sal_uInt16 Category);

public:
    SmDistanceDialog(weld::Window *pParent);
    virtual ~SmDistanceDialog() override;

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat);
};

/**************************************************************************/


class SmAlignDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::RadioButton> m_xLeft;
    std::unique_ptr<weld::RadioButton> m_xCenter;
    std::unique_ptr<weld::RadioButton> m_xRight;
    std::unique_ptr<weld::Button> m_xDefaultButton;

    DECL_LINK(DefaultButtonClickHdl, weld::Button&, void);

public:
    SmAlignDialog(weld::Window *pParent);
    virtual ~SmAlignDialog() override;

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat) const;
};

/**************************************************************************/

class SmShowSymbolSetWindow : public Control
{
    VclPtr<ScrollBar>  m_pVScrollBar;
    SymbolPtrVec_t aSymbolSet;
    Link<SmShowSymbolSetWindow&,void> aSelectHdlLink;
    Link<SmShowSymbolSetWindow&,void> aDblClickHdlLink;
    long        nLen;
    long        nRows, nColumns;
    long        nXOffset, nYOffset;
    sal_uInt16  nSelectSymbol;

    void SetScrollBarRange();
    Point OffsetPoint(const Point &rPoint) const;

    virtual void    Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    virtual void    MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void    KeyInput(const KeyEvent& rKEvt) override;
    virtual void    Resize() override;
    virtual Size    GetOptimalSize() const override;

    DECL_LINK( ScrollHdl, ScrollBar*, void );
public:
    SmShowSymbolSetWindow(vcl::Window *pParent, WinBits nStyle);
    virtual ~SmShowSymbolSetWindow() override;
    virtual void dispose() override;
    void setScrollbar(ScrollBar *pVScrollBar);
    void calccols();
    void    SelectSymbol(sal_uInt16 nSymbol);
    sal_uInt16  GetSelectSymbol() const { return nSelectSymbol; }
    void SetSymbolSet(const SymbolPtrVec_t& rSymbolSet);
    void SetSelectHdl(const Link<SmShowSymbolSetWindow&,void>& rLink) { aSelectHdlLink = rLink; }
    void SetDblClickHdl(const Link<SmShowSymbolSetWindow&,void>& rLink) { aDblClickHdlLink = rLink; }
};

class SmShowSymbolSet : public VclHBox
{
    VclPtr<SmShowSymbolSetWindow> aSymbolWindow;
    VclPtr<ScrollBar>             aVScrollBar;

public:
    SmShowSymbolSet(vcl::Window *pParent);
    virtual ~SmShowSymbolSet() override;
    virtual void dispose() override;

    void    SetSymbolSet(const SymbolPtrVec_t& rSymbolSet) { aSymbolWindow->SetSymbolSet(rSymbolSet); }

    void    SelectSymbol(sal_uInt16 nSymbol) { aSymbolWindow->SelectSymbol(nSymbol); }
    sal_uInt16  GetSelectSymbol() const { return aSymbolWindow->GetSelectSymbol(); }

    void SetSelectHdl(const Link<SmShowSymbolSetWindow&,void>& rLink) { aSymbolWindow->SetSelectHdl(rLink); }
    void SetDblClickHdl(const Link<SmShowSymbolSetWindow&,void>& rLink) { aSymbolWindow->SetDblClickHdl(rLink); }
};


class SmShowSymbol : public Control
{
    Link<SmShowSymbol&,void> aDblClickHdlLink;

    virtual void    Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    virtual void    ApplySettings(vcl::RenderContext& rRenderContext) override;
    virtual void    MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void    Resize() override;

    void setFontSize(vcl::Font &rFont) const;

public:
    SmShowSymbol(vcl::Window *pParent, WinBits nStyle)
        : Control(pParent, nStyle)
    {
    }

    void    SetSymbol(const SmSym *pSymbol);
    void    SetDblClickHdl(const Link<SmShowSymbol&,void> &rLink) { aDblClickHdlLink = rLink; }
};


class SmSymbolDialog : public ModalDialog
{
    VclPtr<ListBox>         m_pSymbolSets;
    VclPtr<SmShowSymbolSet> m_pSymbolSetDisplay;
    VclPtr<FixedText>       m_pSymbolName;
    VclPtr<SmShowSymbol>    m_pSymbolDisplay;
    VclPtr<PushButton>      m_pGetBtn;
    VclPtr<PushButton>      m_pEditBtn;

    SmViewShell        &rViewSh;
    SmSymbolManager    &rSymbolMgr;

    OUString       aSymbolSetName;
    SymbolPtrVec_t      aSymbolSet;

    VclPtr<OutputDevice> pFontListDev;

    DECL_LINK(SymbolSetChangeHdl, ListBox&, void);
    DECL_LINK(SymbolChangeHdl, SmShowSymbolSetWindow&, void);
    DECL_LINK(SymbolDblClickHdl, SmShowSymbol&, void);
    DECL_LINK(SymbolDblClickHdl2, SmShowSymbolSetWindow&, void);
    DECL_LINK(EditClickHdl, Button*, void);
    DECL_LINK(GetClickHdl, Button*, void);
    void SymbolDblClickHdl();

    void            FillSymbolSets();
    const SmSym    *GetSymbol() const;

    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

public:
    SmSymbolDialog(vcl::Window * pParent, OutputDevice *pFntListDevice,
            SmSymbolManager &rSymbolMgr, SmViewShell &rViewShell);
    virtual ~SmSymbolDialog() override;
    virtual void dispose() override;

    bool    SelectSymbolSet(const OUString &rSymbolSetName);
    void    SelectSymbol(sal_uInt16 nSymbolPos);
};


class SmShowChar
{
private:
    std::unique_ptr<weld::DrawingArea> m_xDrawingArea;
    OUString m_aText;
    vcl::Font m_aFont;
    Size m_aSize;

    DECL_LINK(DoPaint, weld::DrawingArea::draw_args, void);
    DECL_LINK(DoResize, const Size& rSize, void);

public:
    SmShowChar(weld::DrawingArea* pDrawingArea)
        : m_xDrawingArea(pDrawingArea)
    {
        m_xDrawingArea->connect_size_allocate(LINK(this, SmShowChar, DoResize));
        m_xDrawingArea->connect_draw(LINK(this, SmShowChar, DoPaint));
        m_xDrawingArea->set_size_request(m_xDrawingArea->get_approximate_digit_width() * 7,
                                         m_xDrawingArea->get_text_height() * 3);
    }

    void SetSymbol(const SmSym *pSym);
    void SetSymbol(sal_UCS4 cChar, const vcl::Font &rFont);
    void SetText(const OUString& rText) { m_aText = rText; }
    const OUString& GetText() const { return m_aText; }
    void SetFont(const vcl::Font& rFont) { m_aFont = rFont; }
    const vcl::Font& GetFont() const { return m_aFont; }

    void queue_draw() { m_xDrawingArea->queue_draw(); }
};

class SmSymDefineDialog : public weld::GenericDialogController
{
    VclPtr<VirtualDevice> m_xVirDev;
    SmSymbolManager m_aSymbolMgrCopy;
    SmSymbolManager& m_rSymbolMgr;
    std::unique_ptr<SmSym> m_xOrigSymbol;
    std::unique_ptr<SubsetMap> m_xSubsetMap;
    std::unique_ptr<FontList> m_xFontList;
    std::unique_ptr<weld::ComboBoxText> m_xOldSymbols;
    std::unique_ptr<weld::ComboBoxText> m_xOldSymbolSets;
    std::unique_ptr<weld::ComboBoxText> m_xSymbols;
    std::unique_ptr<weld::ComboBoxText> m_xSymbolSets;
    std::unique_ptr<weld::ComboBoxText> m_xFonts;
    std::unique_ptr<weld::ComboBoxText> m_xFontsSubsetLB;
    std::unique_ptr<weld::ComboBoxText> m_xStyles;
    std::unique_ptr<weld::Label> m_xOldSymbolName;
    std::unique_ptr<weld::Label> m_xOldSymbolSetName;
    std::unique_ptr<weld::Label> m_xSymbolName;
    std::unique_ptr<weld::Label> m_xSymbolSetName;
    std::unique_ptr<weld::Button> m_xAddBtn;
    std::unique_ptr<weld::Button> m_xChangeBtn;
    std::unique_ptr<weld::Button> m_xDeleteBtn;
    std::unique_ptr<SmShowChar> m_xOldSymbolDisplay;
    std::unique_ptr<SmShowChar> m_xSymbolDisplay;
    std::unique_ptr<SvxShowCharSet>  m_xCharsetDisplay;

    DECL_LINK(OldSymbolChangeHdl, weld::ComboBoxText&, void);
    DECL_LINK(OldSymbolSetChangeHdl, weld::ComboBoxText&, void);
    DECL_LINK(ModifyHdl, weld::ComboBoxText&, void);
    DECL_LINK(FontChangeHdl, weld::ComboBoxText&, void);
    DECL_LINK(SubsetChangeHdl, weld::ComboBoxText&, void);
    DECL_LINK(StyleChangeHdl, weld::ComboBoxText&, void);
    DECL_LINK(CharHighlightHdl, SvxShowCharSet*, void);
    DECL_LINK(AddClickHdl, weld::Button&, void);
    DECL_LINK(ChangeClickHdl, weld::Button&, void);
    DECL_LINK(DeleteClickHdl, weld::Button&, void);

    void    FillSymbols(weld::ComboBoxText& rComboBox, bool bDeleteText = true);
    void    FillSymbolSets(weld::ComboBoxText& rComboBox, bool bDeleteText = true);
    void    FillFonts();
    void    FillStyles();

    void    SetSymbolSetManager(const SmSymbolManager &rMgr);
    void    SetFont(const OUString &rFontName, const OUString &rStyleName);
    void    SetOrigSymbol(const SmSym *pSymbol, const OUString &rSymbolSetName);
    void    UpdateButtons();

    bool    SelectSymbolSet(weld::ComboBoxText &rComboBox, const OUString &rSymbolSetName,
                            bool bDeleteText);
    bool    SelectSymbol(weld::ComboBoxText& rComboBox, const OUString &rSymbolName,
                            bool bDeleteText);
    bool    SelectFont(const OUString &rFontName, bool bApplyFont);
    bool    SelectStyle(const OUString &rStyleName, bool bApplyFont);

    SmSym* GetSymbol(const weld::ComboBoxText& rComboBox);
    const SmSym* GetSymbol(const weld::ComboBoxText& rComboBox) const
    {
        return const_cast<SmSymDefineDialog *>(this)->GetSymbol(rComboBox);
    }

public:
    SmSymDefineDialog(weld::Window *pParent, OutputDevice *pFntListDevice, SmSymbolManager &rMgr);
    virtual ~SmSymDefineDialog() override;

    short execute();

    void SelectOldSymbolSet(const OUString &rSymbolSetName)
    {
        SelectSymbolSet(*m_xOldSymbolSets, rSymbolSetName, false);
    }

    void SelectOldSymbol(const OUString &rSymbolName)
    {
        SelectSymbol(*m_xOldSymbols, rSymbolName, false);
    }

    bool SelectSymbolSet(const OUString &rSymbolSetName)
    {
        return SelectSymbolSet(*m_xSymbolSets, rSymbolSetName, false);
    }

    bool SelectSymbol(const OUString &rSymbolName)
    {
        return SelectSymbol(*m_xSymbols, rSymbolName, false);
    }

    bool        SelectFont(const OUString &rFontName)   { return SelectFont(rFontName, true); }
    bool        SelectStyle(const OUString &rStyleName) { return SelectStyle(rStyleName, true); };
    void        SelectChar(sal_Unicode cChar);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
