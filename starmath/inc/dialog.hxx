/*************************************************************************
 *
 *  $RCSfile: dialog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tl $ $Date: 2000-10-16 10:58:22 $
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
#ifndef DIALOG_HXX
#define DIALOG_HXX

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _SVX_OPTGENRL_HXX //autogen
#include <svx/optgenrl.hxx>
#endif
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif
#ifndef _SV_SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _SV_CTRL_HXX //autogen
#include <vcl/ctrl.hxx>
#endif
#ifndef _SV_MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _CTRLBOX_HXX //autogen
#include <svtools/ctrlbox.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif

#ifndef UTILITY_HXX
#include "utility.hxx"
#endif
#ifndef FORMAT_HXX
#include "format.hxx"
#endif
#ifndef SYMBOL_HXX
#include "symbol.hxx"
#endif


#define CATEGORY_NONE   0xFFFF

/**************************************************************************/

const XubString GetFontStyleName(const Font &rFont);
void            SetFontStyle(const XubString &rStyleName, Font &rFont);

/**************************************************************************/

class SmAboutDialog : public ModalDialog
{
    FixedText   aFixedText1;
    FixedText   aFixedText2;
    FixedText   aFixedText3;
    FixedText   aFixedText4;
    FixedText   aFixedText5;
    FixedText   aFixedText6;
    FixedText   aReleaseText;
    FixedBitmap aFixedBitmap1;
    OKButton    aOKButton1;

public:
    SmAboutDialog(Window *pParent, BOOL bFreeRes = TRUE);
};

/**************************************************************************/

class SmPrintOptionsTabPage : public SfxTabPage
{
    GroupBox        aGroupBox1;
    CheckBox        aTitle;
    CheckBox        aText;
    CheckBox        aFrame;
    GroupBox        aGroupBox3;
    CheckBox        aNoRightSpaces;
    GroupBox        aGroupBox2;
    RadioButton     aSizeNormal;
    RadioButton     aSizeScaled;
    RadioButton     aSizeZoomed;
    MetricField     aZoom;

    DECL_LINK(SizeButtonClickHdl, Button *);

    virtual BOOL    FillItemSet(SfxItemSet& rSet);
    virtual void    Reset(const SfxItemSet& rSet);

public:
    static SfxTabPage* Create(Window *pWindow, const SfxItemSet &rSet);

    SmPrintOptionsTabPage(Window *pParent, const SfxItemSet &rOptions);
};


#ifdef NEVER
class SmGeneralTabPage : public SvxGeneralTabPage
{
public :
    static SfxTabPage* Create(Window* pWindow, const SfxItemSet& rSet);

    SmGeneralTabPage (Window *pParent, const SfxItemSet &rAttrSet) :
        SvxGeneralTabPage (pParent, rAttrSet)
    {
    }
};
#endif //NEVER

/**************************************************************************/

#ifdef NEVER
class SmExtraOptionsTabPage : public SfxTabPage
{
    CheckBox   aAutoRedraw;
    FixedText  aFixedText1;
    SmInfoText aSymbolFile;
    PushButton aBrowseButton;

    DECL_LINK(SymClickHdl, PushButton*);

    virtual BOOL FillItemSet(SfxItemSet& rOutSet);
    virtual void Reset(const SfxItemSet& rOutSet);

public:
    SmExtraOptionsTabPage(const SfxItemSet& rInSet, Window *pParent, BOOL bFreeRes = TRUE);

    static SfxTabPage* Create(Window* pWindow, const SfxItemSet& rSet);
};
#endif //NEVER

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
    GroupBox        aGroupBox1;
    GroupBox        aGroupBox2;

    Font    Face;

    DECL_LINK(FontSelectHdl, ComboBox *);
    DECL_LINK(FontModifyHdl, ComboBox *);
    DECL_LINK(AttrChangeHdl, CheckBox *);

public:
    SmFontDialog(Window * pParent, BOOL bFreeRes = TRUE);

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
    GroupBox        aGroupBox1;
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
    GroupBox            aGroupBox1;
    GroupBox            aGroupBox2;
    OKButton            aOKButton1;
    CancelButton        aCancelButton1;
    MenuButton          aMenuButton;
    PushButton          aDefaultButton;

    DECL_LINK(MenuSelectHdl, Menu *);
    DECL_LINK(DefaultButtonClickHdl, Button *);

public:
    SmFontTypeDialog(Window *pParent, BOOL bFreeRes = TRUE);

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat) const;
};

/**************************************************************************/

#define NOCATEGORIES    10

class SmCategoryDesc : public Resource
{
    XubString       Name;
    XubString      *Strings[4];
    Bitmap     *Graphics[4];
    USHORT      Minimum[4];
    USHORT      Maximum[4];
    USHORT      Value[4];

public:
    SmCategoryDesc(const ResId &rResId, USHORT nCategoryIdx);
    ~SmCategoryDesc();

    const XubString &   GetName() const                 { return Name; }
    const XubString *   GetString(USHORT Index) const   { return Strings[Index];  }
    const Bitmap *  GetGraphic(USHORT Index) const  { return Graphics[Index]; }
    USHORT          GetMinimum(USHORT Index)        { return Minimum[Index]; }
    USHORT          GetMaximum(USHORT Index)        { return Maximum[Index]; }
    USHORT          GetValue(USHORT Index) const    { return Value[Index]; }
    void            SetValue(USHORT Index, USHORT nVal) { Value[Index] = nVal;}
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
    GroupBox        aGroupBox;

    SmCategoryDesc *Categories[NOCATEGORIES];
    USHORT          nActiveCategory;
    BOOL            bScaleAllBrackets;

    DECL_LINK(GetFocusHdl, Control *);
    DECL_LINK(MenuSelectHdl, Menu *);
    DECL_LINK(DefaultButtonClickHdl, Button *);
    DECL_LINK(CheckBoxClickHdl, CheckBox *);

    void    SetHelpId(MetricField &rField, ULONG nHelpId);
    void    SetCategory(USHORT Category);

public:
    SmDistanceDialog(Window *pParent, BOOL bFreeRes = TRUE);
    ~SmDistanceDialog();

    void ReadFrom(const SmFormat &rFormat);
    void WriteTo (SmFormat &rFormat) /*const*/;
};


/**************************************************************************/


class SmAlignDialog : public ModalDialog
{
    RadioButton  aLeft;
    RadioButton  aCenter;
    RadioButton  aRight;
    GroupBox     aGroupBox1;
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
    SmSymSet    aSymbolSet;
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

    void    SetSymbolSet(const SmSymSet& rSymbolSet);

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
    GroupBox        aGrpBox;
    FixedText       aSymbolName;
    SmShowSymbol    aSymbolDisplay;
    PushButton      aCloseBtn;
    PushButton      aEditBtn;
    PushButton      aGetBtn;

    SmSymSetManager    &rSymSetMgr;
    const SmSymSet     *pSymSet;

    DECL_LINK(SymbolSetChangeHdl, ListBox *);
    DECL_LINK(SymbolChangeHdl, SmShowSymbolSet *);
    DECL_LINK(SymbolDblClickHdl, SmShowSymbolSet *);
    DECL_LINK(CloseClickHdl, Button *);
    DECL_LINK(EditClickHdl, Button *);
    DECL_LINK(GetClickHdl, Button *);

    void            FillSymbolSets(BOOL bDeleteText = TRUE);
    void            SetSymbolSetManager(SmSymSetManager &rMgr);
    const SmSym    *GetSymbol() const;

public:
    SmSymbolDialog(Window * pParent, SmSymSetManager &rSymSetMgr, BOOL bFreeRes = TRUE);

    BOOL    SelectSymbolSet(const XubString &rSymbolSetName);
    void    SelectSymbol(USHORT nSymbolNo);
};

/**************************************************************************/

class SmShowCharset : public Control
{
    Link    aSelectHdlLink;
    Link    aDblClickHdlLink;
    USHORT  nLen;
    USHORT  nRows, nColumns;
    xub_Unicode aChar;

    virtual void    Paint(const Rectangle&);
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    KeyInput(const KeyEvent& rKEvt);

public:
    SmShowCharset(Window *pParent, const ResId& rResId);

    void    SetSelectHdl(const Link& rLink)
    {
        aSelectHdlLink = rLink;
    }

    void    SetDblClickHdl(const Link& rLink)
    {
        aDblClickHdlLink = rLink;
    }

    void    SetFont(const Font &rFont);
    void    SelectChar(xub_Unicode aChar);
    xub_Unicode GetSelectChar() const { return aChar; }
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
    FixedText     aOldSymbolText;
    ComboBox      aOldSymbols;
    FixedText     aOldSymbolSetText;
    ComboBox      aOldSymbolSets;
    SmShowCharset aCharsetDisplay;
    FixedText     aSymbolText;
    ComboBox      aSymbols;
    FixedText     aSymbolSetText;
    ComboBox      aSymbolSets;
    FixedText     aFontText;
    ListBox       aFonts;
    FixedText     aStyleText;
    FontStyleBox  aStyles;
    FixedText     aOldSymbolName;
    SmShowChar    aOldSymbolDisplay;
    FixedText     aOldSymbolSetName;
    FixedText     aSymbolName;
    SmShowChar    aSymbolDisplay;
    FixedText     aSymbolSetName;
    OKButton      aOkBtn;
    CancelButton  aCancelBtn;
    PushButton    aAddBtn;
    PushButton    aChangeBtn;
    PushButton    aDeleteBtn;
    FixedImage    aRightArrow;

    SmSymSetManager     aSymSetMgrCopy,
                       &rSymSetMgr;
    const SmSym        *pOrigSymbol;

    FontList           *pFontList;

    DECL_LINK(OldSymbolChangeHdl, ComboBox *);
    DECL_LINK(OldSymbolSetChangeHdl, ComboBox *);
    DECL_LINK(ModifyHdl, ComboBox *);
    DECL_LINK(FontChangeHdl, ListBox *);
    DECL_LINK(StyleChangeHdl, ComboBox *);
    DECL_LINK(CharSelectHdl, SmShowCharset *);
    DECL_LINK(AddClickHdl, Button *);
    DECL_LINK(ChangeClickHdl, Button *);
    DECL_LINK(DeleteClickHdl, Button *);

    void    FillSymbols(ComboBox &rComboBox, BOOL bDeleteText = TRUE);
    void    FillSymbolSets(ComboBox &rComboBox, BOOL bDeleteText = TRUE);
    void    FillFonts(BOOL bDeleteText = TRUE);
    void    FillStyles(BOOL bDeleteText = TRUE);

    void    SetSymbolSetManager(const SmSymSetManager &rMgr);
    void    SetFont(const XubString &rFontName, const XubString &rStyleName);
    void    SetOrigSymbol(const SmSym *pSymbol, const XubString &rSymbolSetName);
    void    UpdateButtons();

    BOOL    SelectSymbolSet(ComboBox &rComboBox, const XubString &rSymbolSetName,
                            BOOL bDeleteText);
    BOOL    SelectSymbol(ComboBox &rComboBox, const XubString &rSymbolName,
                            BOOL bDeleteText);
    BOOL    SelectFont(const XubString &rFontName, BOOL bApplyFont);
    BOOL    SelectStyle(const XubString &rStyleName, BOOL bApplyFont);


    SmSymSet              *GetSymbolSet(const ComboBox &rComboBox);
    inline const SmSymSet *GetSymbolSet(const ComboBox &rComboBox) const;
    SmSym                 *GetSymbol(const ComboBox &rComboBox);
    inline const SmSym    *GetSymbol(const ComboBox &rComboBox) const;

public:
    SmSymDefineDialog(Window *pParent, SmSymSetManager &rMgr, BOOL bFreeRes = TRUE);
    ~SmSymDefineDialog();

    // Dialog
    virtual short   Execute();

    inline BOOL SelectOldSymbolSet(const XubString &rSymbolSetName);
    inline BOOL SelectOldSymbol(const XubString &rSymbolName);
    inline BOOL SelectSymbolSet(const XubString &rSymbolSetName);
    inline BOOL SelectSymbol(const XubString &rSymbolName);
    BOOL        SelectFont(const XubString &rFontName)   { return SelectFont(rFontName, TRUE); }
    BOOL        SelectStyle(const XubString &rStyleName) { return SelectStyle(rStyleName, TRUE); };
    void        SelectChar(xub_Unicode cChar);
};

inline const SmSymSet * SmSymDefineDialog::GetSymbolSet(const ComboBox &rComboBox) const
{
    return ((SmSymDefineDialog *) this)->GetSymbolSet(rComboBox);
}

inline const SmSym * SmSymDefineDialog::GetSymbol(const ComboBox &rComboBox) const
{
    return ((SmSymDefineDialog *) this)->GetSymbol(rComboBox);
}

inline BOOL SmSymDefineDialog::SelectOldSymbolSet(const XubString &rSymbolSetName)
{
    return SelectSymbolSet(aOldSymbolSets, rSymbolSetName, FALSE);
}

inline BOOL SmSymDefineDialog::SelectOldSymbol(const XubString &rSymbolName)
{
    return SelectSymbol(aOldSymbols, rSymbolName, FALSE);
}

inline BOOL SmSymDefineDialog::SelectSymbolSet(const XubString &rSymbolSetName)
{
    return SelectSymbolSet(aSymbolSets, rSymbolSetName, FALSE);
}

inline BOOL SmSymDefineDialog::SelectSymbol(const XubString &rSymbolName)
{
    return SelectSymbol(aSymbols, rSymbolName, FALSE);
}


#endif

