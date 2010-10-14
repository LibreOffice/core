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
#ifndef DIALOG_HXX
#define DIALOG_HXX

#include <vcl/image.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/image.hxx>
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

const XubString GetFontStyleName(const Font &rFont);
void            SetFontStyle(const XubString &rStyleName, Font &rFont);

/**************************************************************************/

class SmPrintOptionsTabPage : public SfxTabPage
{
    FixedLine       aFixedLine1;
    CheckBox        aTitle;
    CheckBox        aText;
    CheckBox        aFrame;
    FixedLine       aFixedLine2;
    RadioButton     aSizeNormal;
    RadioButton     aSizeScaled;
    RadioButton     aSizeZoomed;
    MetricField     aZoom;
    FixedLine       aFixedLine3;
    CheckBox        aNoRightSpaces;

    DECL_LINK(SizeButtonClickHdl, Button *);

    virtual BOOL    FillItemSet(SfxItemSet& rSet);
    virtual void    Reset(const SfxItemSet& rSet);

public:
    static SfxTabPage* Create(Window *pWindow, const SfxItemSet &rSet);

    SmPrintOptionsTabPage(Window *pParent, const SfxItemSet &rOptions);
};

/**************************************************************************/

class SmShowFont : public Control
{
    virtual void    Paint(const Rectangle&);

public:
    SmShowFont(Window *pParent, const ResId& rResId) :
        Control(pParent, rResId)
    {
    }

    void SetFont(const Font& rFont);
};

class SmFontDialog : public ModalDialog
{
    FixedText       aFixedText1;
    ComboBox        aFontBox;
    CheckBox        aBoldCheckBox;
    CheckBox        aItalicCheckBox;
    OKButton        aOKButton1;
    CancelButton    aCancelButton1;
    SmShowFont      aShowFont;
    FixedText       aFixedText2;

    Font    Face;

    DECL_LINK(FontSelectHdl, ComboBox *);
    DECL_LINK(FontModifyHdl, ComboBox *);
    DECL_LINK(AttrChangeHdl, CheckBox *);

    void            InitColor_Impl();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    SmFontDialog(Window * pParent, OutputDevice *pFntListDevice, BOOL bHideCheckboxes, BOOL bFreeRes = TRUE);

    const Font& GetFont() const { return Face; }
    void        SetFont(const Font &rFont);
};

/**************************************************************************/

class SmFontSizeDialog : public ModalDialog
{
    FixedText       aFixedText1;
    MetricField     aBaseSize;
    FixedText       aFixedText4;
    MetricField     aTextSize;
    FixedText       aFixedText5;
    MetricField     aIndexSize;
    FixedText       aFixedText6;
    MetricField     aFunctionSize;
    FixedText       aFixedText7;
    MetricField     aOperatorSize;
    FixedText       aFixedText8;
    MetricField     aBorderSize;
    FixedLine       aFixedLine1;
    OKButton        aOKButton1;
    CancelButton    aCancelButton1;
    PushButton      aDefaultButton;

    DECL_LINK(DefaultButtonClickHdl, Button *);

public:
    SmFontSizeDialog(Window *pParent, BOOL bFreeRes = TRUE);

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat) const;
};

/**************************************************************************/

class SmFontTypeDialog : public ModalDialog
{
    FixedText           aFixedText1;
    SmFontPickListBox   aVariableFont;
    FixedText           aFixedText2;
    SmFontPickListBox   aFunctionFont;
    FixedText           aFixedText3;
    SmFontPickListBox   aNumberFont;
    FixedText           aFixedText4;
    SmFontPickListBox   aTextFont;
    FixedText           aFixedText5;
    SmFontPickListBox   aSerifFont;
    FixedText           aFixedText6;
    SmFontPickListBox   aSansFont;
    FixedText           aFixedText7;
    SmFontPickListBox   aFixedFont;
    FixedLine           aFixedLine1;
    FixedLine           aFixedLine2;
    OKButton            aOKButton1;
    CancelButton        aCancelButton1;
    MenuButton          aMenuButton;
    PushButton          aDefaultButton;

    OutputDevice       *pFontListDev;

    DECL_LINK(MenuSelectHdl, Menu *);
    DECL_LINK(DefaultButtonClickHdl, Button *);

public:
    SmFontTypeDialog(Window *pParent, OutputDevice *pFntListDevice, BOOL bFreeRes = TRUE);

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat) const;
};

/**************************************************************************/

#define NOCATEGORIES    10

class SmCategoryDesc : public Resource
{
    XubString       Name;
    XubString      *Strings[4];
    Bitmap         *Graphics[4];    /* regular bitmaps */
    Bitmap         *GraphicsH[4];   /* high contrast bitmaps */
    USHORT          Minimum[4];
    USHORT          Maximum[4];
    USHORT          Value[4];
    BOOL            bIsHighContrast;

public:
    SmCategoryDesc(const ResId &rResId, USHORT nCategoryIdx);
    ~SmCategoryDesc();

    const XubString &   GetName() const                 { return Name; }
    const XubString *   GetString(USHORT Index) const   { return Strings[Index];  }
    USHORT          GetMinimum(USHORT Index)        { return Minimum[Index]; }
    USHORT          GetMaximum(USHORT Index)        { return Maximum[Index]; }
    USHORT          GetValue(USHORT Index) const    { return Value[Index]; }
    void            SetValue(USHORT Index, USHORT nVal) { Value[Index] = nVal;}

    void            SetHighContrast( BOOL bVal )    { bIsHighContrast = bVal; }
    const Bitmap *  GetGraphic(USHORT Index) const
    {
        return bIsHighContrast ? GraphicsH[Index] : Graphics[Index];
    }
};


class SmDistanceDialog : public ModalDialog
{
    FixedText       aFixedText1;
    MetricField     aMetricField1;
    FixedText       aFixedText2;
    MetricField     aMetricField2;
    FixedText       aFixedText3;
    MetricField     aMetricField3;
    CheckBox        aCheckBox1;
    FixedText       aFixedText4;
    MetricField     aMetricField4;
    OKButton        aOKButton1;
    CancelButton    aCancelButton1;
    MenuButton      aMenuButton;
    PushButton      aDefaultButton;
    FixedBitmap     aBitmap;
    FixedLine       aFixedLine;

    SmCategoryDesc *Categories[NOCATEGORIES];
    USHORT          nActiveCategory;
    BOOL            bScaleAllBrackets;

    DECL_LINK(GetFocusHdl, Control *);
    DECL_LINK(MenuSelectHdl, Menu *);
    DECL_LINK(DefaultButtonClickHdl, Button *);
    DECL_LINK(CheckBoxClickHdl, CheckBox *);

    using   Window::SetHelpId;
    void    SetHelpId(MetricField &rField, ULONG nHelpId);
    void    SetCategory(USHORT Category);

    void    ApplyImages();

public:
    SmDistanceDialog(Window *pParent, BOOL bFreeRes = TRUE);
    ~SmDistanceDialog();

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat) /*const*/;

    // Window
    virtual void    DataChanged( const DataChangedEvent &rEvt );
};


/**************************************************************************/


class SmAlignDialog : public ModalDialog
{
    RadioButton  aLeft;
    RadioButton  aCenter;
    RadioButton  aRight;
    FixedLine    aFixedLine1;
    OKButton     aOKButton1;
    CancelButton aCancelButton1;
    PushButton   aDefaultButton;

    DECL_LINK(DefaultButtonClickHdl, Button *);

public:
    SmAlignDialog(Window *pParent, BOOL bFreeRes = TRUE);

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat) const;
};

/**************************************************************************/

class SmShowSymbolSet : public Control
{
    SymbolPtrVec_t aSymbolSet;
    ScrollBar   aVScrollBar;
    Size        aOutputSize;
    Link        aSelectHdlLink;
    Link        aDblClickHdlLink;
    USHORT      nLen;
    USHORT      nRows, nColumns;
    USHORT      nSelectSymbol;

    virtual void    Paint(const Rectangle&);
    virtual void    MouseButtonDown(const MouseEvent& rMEvt);
    virtual void    KeyInput(const KeyEvent& rKEvt);

    DECL_LINK( ScrollHdl, ScrollBar* );

public:
    SmShowSymbolSet(Window *pParent, const ResId& rResId);

    void    SetSymbolSet(const SymbolPtrVec_t& rSymbolSet);

    void    SelectSymbol(USHORT nSymbol);
    USHORT  GetSelectSymbol() const { return nSelectSymbol; }

    void SetSelectHdl(const Link& rLink)   { aSelectHdlLink = rLink; }
    void SetDblClickHdl(const Link& rLink) { aDblClickHdlLink = rLink; }
};

////////////////////////////////////////////////////////////////////////////////

class SmShowSymbol : public Control
{
    Link  aDblClickHdlLink;

    virtual void    Paint(const Rectangle&);
    virtual void    MouseButtonDown(const MouseEvent& rMEvt);

public:
    SmShowSymbol(Window *pParent, const ResId& rResId) :
        Control(pParent, rResId) {}

    void    SetSymbol(const SmSym *pSymbol);
    void    SetDblClickHdl(const Link &rLink) { aDblClickHdlLink = rLink; }
};

////////////////////////////////////////////////////////////////////////////////

class SmSymDefineDialog;

class SmSymbolDialog : public ModalDialog
{
    FixedText       aSymbolSetText;
    ListBox         aSymbolSets;
    SmShowSymbolSet aSymbolSetDisplay;
    FixedText       aSymbolName;
    SmShowSymbol    aSymbolDisplay;
    PushButton      aGetBtn;
    PushButton      aCloseBtn;
    PushButton      aEditBtn;

    SmViewShell        &rViewSh;
    SmSymbolManager    &rSymbolMgr;

    String              aSymbolSetName;
    SymbolPtrVec_t      aSymbolSet;

    OutputDevice       *pFontListDev;

    DECL_LINK(SymbolSetChangeHdl, ListBox *);
    DECL_LINK(SymbolChangeHdl, SmShowSymbolSet *);
    DECL_LINK(SymbolDblClickHdl, SmShowSymbolSet *);
    DECL_LINK(CloseClickHdl, Button *);
    DECL_LINK(EditClickHdl, Button *);
    DECL_LINK(GetClickHdl, Button *);

    void            FillSymbolSets(BOOL bDeleteText = TRUE);
    void            SetSymbolSetManager(SmSymbolManager &rMgr);
    const SmSym    *GetSymbol() const;
    void            InitColor_Impl();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    SmSymbolDialog(Window * pParent, OutputDevice *pFntListDevice,
            SmSymbolManager &rSymbolMgr, SmViewShell &rViewShell, BOOL bFreeRes = TRUE);
    virtual ~SmSymbolDialog();

    BOOL    SelectSymbolSet(const XubString &rSymbolSetName);
    void    SelectSymbol(USHORT nSymbolPos);
    USHORT  GetSelectedSymbol() const   { return aSymbolSetDisplay.GetSelectSymbol(); }
};

////////////////////////////////////////////////////////////////////////////////

class SmShowChar : public Control
{
    virtual void    Paint(const Rectangle&);

public:
    SmShowChar(Window *pParent, const ResId& rResId)
    : Control(pParent, rResId)
    {
    }

    void    SetChar(xub_Unicode aChar);
    void    SetFont(const Font &rFont);
};

////////////////////////////////////////////////////////////////////////////////

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
    CancelButton    aCancelBtn;
    PushButton      aAddBtn;
    PushButton      aChangeBtn;
    PushButton      aDeleteBtn;
    FixedImage      aRightArrow;
    Image           aRigthArrow_Im;
    Image           aRigthArrow_Im_HC;     // hi-contrast version

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
    DECL_LINK(CharHighlightHdl, Control* pControl);
    DECL_LINK(AddClickHdl, Button *);
    DECL_LINK(ChangeClickHdl, Button *);
    DECL_LINK(DeleteClickHdl, Button *);

    void    FillSymbols(ComboBox &rComboBox, BOOL bDeleteText = TRUE);
    void    FillSymbolSets(ComboBox &rComboBox, BOOL bDeleteText = TRUE);
    void    FillFonts(BOOL bDeleteText = TRUE);
    void    FillStyles(BOOL bDeleteText = TRUE);

    void    SetSymbolSetManager(const SmSymbolManager &rMgr);
    void    SetFont(const XubString &rFontName, const XubString &rStyleName);
    void    SetOrigSymbol(const SmSym *pSymbol, const XubString &rSymbolSetName);
    void    UpdateButtons();

    BOOL    SelectSymbolSet(ComboBox &rComboBox, const XubString &rSymbolSetName,
                            BOOL bDeleteText);
    BOOL    SelectSymbol(ComboBox &rComboBox, const XubString &rSymbolName,
                            BOOL bDeleteText);
    BOOL    SelectFont(const XubString &rFontName, BOOL bApplyFont);
    BOOL    SelectStyle(const XubString &rStyleName, BOOL bApplyFont);

    SmSym       * GetSymbol(const ComboBox &rComboBox);
    const SmSym * GetSymbol(const ComboBox &rComboBox) const
    {
        return ((SmSymDefineDialog *) this)->GetSymbol(rComboBox);
    }

    void            InitColor_Impl();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    SmSymDefineDialog(Window *pParent, OutputDevice *pFntListDevice, SmSymbolManager &rMgr, BOOL bFreeRes = TRUE);
    ~SmSymDefineDialog();

    using OutputDevice::SetFont;

    // Dialog
    virtual short   Execute();

    BOOL SelectOldSymbolSet(const XubString &rSymbolSetName)
    {
        return SelectSymbolSet(aOldSymbolSets, rSymbolSetName, FALSE);
    }

    BOOL SelectOldSymbol(const XubString &rSymbolName)
    {
        return SelectSymbol(aOldSymbols, rSymbolName, FALSE);
    }

    BOOL SelectSymbolSet(const XubString &rSymbolSetName)
    {
        return SelectSymbolSet(aSymbolSets, rSymbolSetName, FALSE);
    }

    BOOL SelectSymbol(const XubString &rSymbolName)
    {
        return SelectSymbol(aSymbols, rSymbolName, FALSE);
    }

    BOOL        SelectFont(const XubString &rFontName)   { return SelectFont(rFontName, TRUE); }
    BOOL        SelectStyle(const XubString &rStyleName) { return SelectStyle(rStyleName, TRUE); };
    void        SelectChar(xub_Unicode cChar);
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
