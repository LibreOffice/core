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

class SubsetMap;
#define CATEGORY_NONE   0xFFFF

/**************************************************************************/

void SetFontStyle(const OUString &rStyleName, Font &rFont);

/**************************************************************************/

class SmPrintOptionsTabPage : public SfxTabPage
{
    CheckBox*       m_pTitle;
    CheckBox*       m_pText;
    CheckBox*       m_pFrame;
    RadioButton*    m_pSizeNormal;
    RadioButton*    m_pSizeScaled;
    RadioButton*    m_pSizeZoomed;
    MetricField*    m_pZoom;
    CheckBox*       m_pNoRightSpaces;
    CheckBox*       m_pSaveOnlyUsedSymbols;

    DECL_LINK(SizeButtonClickHdl, Button *);

    virtual sal_Bool    FillItemSet(SfxItemSet& rSet);
    virtual void    Reset(const SfxItemSet& rSet);

public:
    static SfxTabPage* Create(Window *pWindow, const SfxItemSet &rSet);

    SmPrintOptionsTabPage(Window *pParent, const SfxItemSet &rOptions);
};

/**************************************************************************/

class SmShowFont : public Window
{
    virtual void Paint(const Rectangle&);

public:
    SmShowFont(Window *pParent, WinBits nStyle)
        : Window(pParent, nStyle)
    {
    }
    virtual Size GetOptimalSize() const;
    void SetFont(const Font& rFont);
};

class SmFontDialog : public ModalDialog
{
    ComboBox*       m_pFontBox;
    VclContainer*   m_pAttrFrame;
    CheckBox*       m_pBoldCheckBox;
    CheckBox*       m_pItalicCheckBox;
    SmShowFont*     m_pShowFont;

    Font    Face;

    DECL_LINK(FontSelectHdl, ComboBox *);
    DECL_LINK(FontModifyHdl, ComboBox *);
    DECL_LINK(AttrChangeHdl, CheckBox *);

    void            InitColor_Impl();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    SmFontDialog(Window * pParent, OutputDevice *pFntListDevice, bool bHideCheckboxes);

    const Font& GetFont() const { return Face; }
    void        SetFont(const Font &rFont);
};

/**************************************************************************/

class SmFontSizeDialog : public ModalDialog
{
    MetricField* m_pBaseSize;
    MetricField* m_pTextSize;
    MetricField* m_pIndexSize;
    MetricField* m_pFunctionSize;
    MetricField* m_pOperatorSize;
    MetricField* m_pBorderSize;
    PushButton* m_pDefaultButton;

    DECL_LINK(DefaultButtonClickHdl, Button *);

public:
    SmFontSizeDialog(Window *pParent);

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat) const;
};

/**************************************************************************/

class SmFontTypeDialog : public ModalDialog
{
    SmFontPickListBox* m_pVariableFont;
    SmFontPickListBox* m_pFunctionFont;
    SmFontPickListBox* m_pNumberFont;
    SmFontPickListBox* m_pTextFont;
    SmFontPickListBox* m_pSerifFont;
    SmFontPickListBox* m_pSansFont;
    SmFontPickListBox* m_pFixedFont;
    MenuButton* m_pMenuButton;
    PushButton* m_pDefaultButton;

    OutputDevice       *pFontListDev;

    DECL_LINK(MenuSelectHdl, Menu *);
    DECL_LINK(DefaultButtonClickHdl, Button *);
    DECL_LINK(HelpButtonClickHdl, Button *);

public:
    SmFontTypeDialog(Window *pParent, OutputDevice *pFntListDevice);

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat) const;
};

/**************************************************************************/

#define NOCATEGORIES    10

class SmCategoryDesc
{
    OUString Name;
    OUString  *Strings[4];
    Image *Graphics[4];    /* regular bitmaps */
    sal_uInt16 Minimum[4];
    sal_uInt16 Maximum[4];
    sal_uInt16 Value[4];

public:
    SmCategoryDesc(VclBuilderContainer& rBuilder, sal_uInt16 nCategoryIdx);
    ~SmCategoryDesc();

    const OUString& GetName() const                 { return Name; }
    const OUString* GetString(sal_uInt16 Index) const   { return Strings[Index];  }
    sal_uInt16          GetMinimum(sal_uInt16 Index)        { return Minimum[Index]; }
    sal_uInt16          GetMaximum(sal_uInt16 Index)        { return Maximum[Index]; }
    sal_uInt16          GetValue(sal_uInt16 Index) const    { return Value[Index]; }
    void            SetValue(sal_uInt16 Index, sal_uInt16 nVal) { Value[Index] = nVal;}

    const Image *  GetGraphic(sal_uInt16 Index) const
    {
        return Graphics[Index];
    }
};


class SmDistanceDialog : public ModalDialog
{
    VclFrame*       m_pFrame;
    FixedText*      m_pFixedText1;
    MetricField*    m_pMetricField1;
    FixedText*      m_pFixedText2;
    MetricField*    m_pMetricField2;
    FixedText*      m_pFixedText3;
    MetricField*    m_pMetricField3;
    CheckBox*       m_pCheckBox1;
    FixedText*      m_pFixedText4;
    MetricField*    m_pMetricField4;
    MenuButton*     m_pMenuButton;
    PushButton*     m_pDefaultButton;
    FixedImage*     m_pBitmap;

    SmCategoryDesc *Categories[NOCATEGORIES];
    sal_uInt16          nActiveCategory;
    bool            bScaleAllBrackets;

    DECL_LINK(GetFocusHdl, Control *);
    DECL_LINK(MenuSelectHdl, Menu *);
    DECL_LINK(DefaultButtonClickHdl, Button *);
    DECL_LINK(CheckBoxClickHdl, CheckBox *);

    using   Window::SetHelpId;
    void    SetHelpId(MetricField &rField, const OString& sHelpId);
    void    SetCategory(sal_uInt16 Category);

public:
    SmDistanceDialog(Window *pParent);
    ~SmDistanceDialog();

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat) /*const*/;

    // Window
    virtual void    DataChanged( const DataChangedEvent &rEvt );
};


/**************************************************************************/


class SmAlignDialog : public ModalDialog
{
    RadioButton* m_pLeft;
    RadioButton* m_pCenter;
    RadioButton* m_pRight;
    PushButton*  m_pDefaultButton;

    DECL_LINK(DefaultButtonClickHdl, Button *);

public:
    SmAlignDialog(Window *pParent);

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat) const;
};

/**************************************************************************/

class SmShowSymbolSetWindow : public Control
{
    ScrollBar*  m_pVScrollBar;
    SymbolPtrVec_t aSymbolSet;
    Link        aSelectHdlLink;
    Link        aDblClickHdlLink;
    sal_uInt16  nLen;
    sal_uInt16  nRows, nColumns;
    sal_uInt16  nXOffset, nYOffset;
    sal_uInt16  nSelectSymbol;

    void SetScrollBarRange();
    Point OffsetPoint(const Point &rPoint) const;

    virtual void    Paint(const Rectangle&);
    virtual void    MouseButtonDown(const MouseEvent& rMEvt);
    virtual void    KeyInput(const KeyEvent& rKEvt);
    virtual void    Resize();
    virtual Size    GetOptimalSize() const;

    DECL_LINK( ScrollHdl, ScrollBar* );
public:
    SmShowSymbolSetWindow(Window *pParent, WinBits nStyle);
    void setScrollbar(ScrollBar *pVScrollBar);
    void calccols();
    void    SelectSymbol(sal_uInt16 nSymbol);
    sal_uInt16  GetSelectSymbol() const { return nSelectSymbol; }
    void SetSymbolSet(const SymbolPtrVec_t& rSymbolSet);
    void SetSelectHdl(const Link& rLink)   { aSelectHdlLink = rLink; }
    void SetDblClickHdl(const Link& rLink) { aDblClickHdlLink = rLink; }
};

class SmShowSymbolSet : public VclHBox
{
    SmShowSymbolSetWindow aSymbolWindow;
    ScrollBar   aVScrollBar;
    Size        aOutputSize;

public:
    SmShowSymbolSet(Window *pParent);

    void    SetSymbolSet(const SymbolPtrVec_t& rSymbolSet) { aSymbolWindow.SetSymbolSet(rSymbolSet); }

    void    SelectSymbol(sal_uInt16 nSymbol) { aSymbolWindow.SelectSymbol(nSymbol); }
    sal_uInt16  GetSelectSymbol() const { return aSymbolWindow.GetSelectSymbol(); }

    void SetSelectHdl(const Link& rLink)   { aSymbolWindow.SetSelectHdl(rLink); }
    void SetDblClickHdl(const Link& rLink) { aSymbolWindow.SetDblClickHdl(rLink); }
};



class SmShowSymbol : public Control
{
    Link  aDblClickHdlLink;

    virtual void    Paint(const Rectangle&);
    virtual void    MouseButtonDown(const MouseEvent& rMEvt);
    virtual void    Resize();

    void setFontSize(Font &rFont) const;

public:
    SmShowSymbol(Window *pParent, WinBits nStyle)
        : Control(pParent, nStyle)
    {
    }

    void    SetSymbol(const SmSym *pSymbol);
    void    SetDblClickHdl(const Link &rLink) { aDblClickHdlLink = rLink; }
};



class SmSymDefineDialog;

class SmSymbolDialog : public ModalDialog
{
    ListBox*         m_pSymbolSets;
    SmShowSymbolSet* m_pSymbolSetDisplay;
    FixedText*       m_pSymbolName;
    SmShowSymbol*    m_pSymbolDisplay;
    PushButton*      m_pGetBtn;
    PushButton*      m_pEditBtn;

    SmViewShell        &rViewSh;
    SmSymbolManager    &rSymbolMgr;

    OUString       aSymbolSetName;
    SymbolPtrVec_t      aSymbolSet;

    OutputDevice       *pFontListDev;

    DECL_LINK(SymbolSetChangeHdl, void*);
    DECL_LINK(SymbolChangeHdl, void*);
    DECL_LINK(SymbolDblClickHdl, void*);
    DECL_LINK(EditClickHdl, void*);
    DECL_LINK(GetClickHdl, void*);

    void            FillSymbolSets(bool bDeleteText = true);
    void            SetSymbolSetManager(SmSymbolManager &rMgr);
    const SmSym    *GetSymbol() const;
    void            InitColor_Impl();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    SmSymbolDialog(Window * pParent, OutputDevice *pFntListDevice,
            SmSymbolManager &rSymbolMgr, SmViewShell &rViewShell);

    bool    SelectSymbolSet(const OUString &rSymbolSetName);
    void    SelectSymbol(sal_uInt16 nSymbolPos);
    sal_uInt16  GetSelectedSymbol() const   { return m_pSymbolSetDisplay->GetSelectSymbol(); }
};



class SmShowChar : public Control
{
    virtual void    Paint(const Rectangle&);

public:
    SmShowChar(Window *pParent, const ResId& rResId)
    : Control(pParent, rResId)
    {
    }

    void    SetSymbol( const SmSym *pSym );
    void    SetSymbol( sal_UCS4 cChar, const Font &rFont );
};



class SmSymDefineDialog : public ModalDialog
{
    FixedText       aOldSymbolText;
    ComboBox        aOldSymbols;
    FixedText       aOldSymbolSetText;
    ComboBox        aOldSymbolSets;
    SvxShowCharSet  aCharsetDisplay;
    FixedText       aSymbolText;
    ComboBox        aSymbols;
    FixedText       aSymbolSetText;
    ComboBox        aSymbolSets;
    FixedText       aFontText;
    ListBox         aFonts;
    FixedText       aFontsSubsetFT;
    ListBox         aFontsSubsetLB;
    FixedText       aStyleText;
    FontStyleBox    aStyles;
    FixedText       aOldSymbolName;
    SmShowChar      aOldSymbolDisplay;
    FixedText       aOldSymbolSetName;
    FixedText       aSymbolName;
    SmShowChar      aSymbolDisplay;
    FixedText       aSymbolSetName;
    OKButton        aOkBtn;
    HelpButton      aHelpBtn;
    CancelButton    aCancelBtn;
    PushButton      aAddBtn;
    PushButton      aChangeBtn;
    PushButton      aDeleteBtn;
    FixedImage      aRightArrow;
    Image           aRightArrow_Im;

    SmSymbolManager     aSymbolMgrCopy,
                       &rSymbolMgr;
    const SmSym        *pOrigSymbol;

    const SubsetMap    *pSubsetMap;
    FontList           *pFontList;

    DECL_LINK(OldSymbolChangeHdl, ComboBox *);
    DECL_LINK(OldSymbolSetChangeHdl, ComboBox *);
    DECL_LINK(ModifyHdl, ComboBox *);
    DECL_LINK(FontChangeHdl, ListBox *);
    DECL_LINK(SubsetChangeHdl, ListBox*);
    DECL_LINK(StyleChangeHdl, ComboBox *);
    DECL_LINK(CharHighlightHdl, void *);
    DECL_LINK(AddClickHdl, Button *);
    DECL_LINK(ChangeClickHdl, Button *);
    DECL_LINK(DeleteClickHdl, Button *);
    DECL_LINK(HelpButtonClickHdl, Button *);

    void    FillSymbols(ComboBox &rComboBox, bool bDeleteText = true);
    void    FillSymbolSets(ComboBox &rComboBox, bool bDeleteText = true);
    void    FillFonts(bool bDeleteText = true);
    void    FillStyles(bool bDeleteText = true);

    void    SetSymbolSetManager(const SmSymbolManager &rMgr);
    void    SetFont(const OUString &rFontName, const OUString &rStyleName);
    void    SetOrigSymbol(const SmSym *pSymbol, const OUString &rSymbolSetName);
    void    UpdateButtons();

    bool    SelectSymbolSet(ComboBox &rComboBox, const OUString &rSymbolSetName,
                            bool bDeleteText);
    bool    SelectSymbol(ComboBox &rComboBox, const OUString &rSymbolName,
                            bool bDeleteText);
    bool    SelectFont(const OUString &rFontName, bool bApplyFont);
    bool    SelectStyle(const OUString &rStyleName, bool bApplyFont);

    SmSym       * GetSymbol(const ComboBox &rComboBox);
    const SmSym * GetSymbol(const ComboBox &rComboBox) const
    {
        return ((SmSymDefineDialog *) this)->GetSymbol(rComboBox);
    }

    void            InitColor_Impl();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    SmSymDefineDialog(Window *pParent, OutputDevice *pFntListDevice, SmSymbolManager &rMgr, bool bFreeRes = true);
    ~SmSymDefineDialog();

    using OutputDevice::SetFont;

    // Dialog
    virtual short   Execute();

    bool SelectOldSymbolSet(const OUString &rSymbolSetName)
    {
        return SelectSymbolSet(aOldSymbolSets, rSymbolSetName, false);
    }

    bool SelectOldSymbol(const OUString &rSymbolName)
    {
        return SelectSymbol(aOldSymbols, rSymbolName, false);
    }

    bool SelectSymbolSet(const OUString &rSymbolSetName)
    {
        return SelectSymbolSet(aSymbolSets, rSymbolSetName, false);
    }

    bool SelectSymbol(const OUString &rSymbolName)
    {
        return SelectSymbol(aSymbols, rSymbolName, false);
    }

    bool        SelectFont(const OUString &rFontName)   { return SelectFont(rFontName, true); }
    bool        SelectStyle(const OUString &rStyleName) { return SelectStyle(rStyleName, true); };
    void        SelectChar(sal_Unicode cChar);
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
