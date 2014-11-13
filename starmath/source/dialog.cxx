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

#include "tools/rcid.h"
#include <comphelper/string.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/app.hxx>
#include <vcl/builder.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/printer.hxx>
#include <vcl/help.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/settings.hxx>
#include <vcl/wall.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfx.hrc>
#include <osl/diagnose.h>
#include <svx/ucsubset.hxx>


#include "dialog.hxx"
#include "starmath.hrc"
#include "config.hxx"
#include "smmod.hxx"
#include "symbol.hxx"
#include "view.hxx"
#include "document.hxx"
#include "unomodel.hxx"


// Since it's better to set/query the FontStyle via its attributes rather
// than via the StyleName we create a way to translate
// Attribute <-> StyleName

class SmFontStyles
{
    OUString aNormal;
    OUString aBold;
    OUString aItalic;
    OUString aBoldItalic;
    OUString aEmpty;

public:
    SmFontStyles();

    sal_uInt16          GetCount() const    { return 4; }
    const OUString&  GetStyleName( const vcl::Font &rFont ) const;
    const OUString&  GetStyleName( sal_uInt16 nIdx ) const;
};


SmFontStyles::SmFontStyles() :
    aNormal ( ResId( RID_FONTREGULAR, *SM_MOD()->GetResMgr() ) ),
    aBold   ( ResId( RID_FONTBOLD,    *SM_MOD()->GetResMgr() ) ),
    aItalic ( ResId( RID_FONTITALIC,  *SM_MOD()->GetResMgr() ) )
{

    aBoldItalic = aBold;
    aBoldItalic += ", ";
    aBoldItalic += aItalic;
}


const OUString& SmFontStyles::GetStyleName( const vcl::Font &rFont ) const
{
    //! compare also SmSpecialNode::Prepare
    bool bBold   = IsBold( rFont ),
         bItalic = IsItalic( rFont );

    if (bBold && bItalic)
        return aBoldItalic;
    else if (bItalic)
        return aItalic;
    else if (bBold)
        return aBold;
    return aNormal;
}


const OUString& SmFontStyles::GetStyleName( sal_uInt16 nIdx ) const
{
    // 0 = "normal",  1 = "italic",
    // 2 = "bold",    3 = "bold italic"

#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE( nIdx < GetCount(), "index out of range" );
#endif
    switch (nIdx)
    {
        case 0 : return aNormal;
        case 1 : return aItalic;
        case 2 : return aBold;
        case 3 : return aBoldItalic;
    }
    return aEmpty;
}


const SmFontStyles & GetFontStyles()
{
    static const SmFontStyles aImpl;
    return aImpl;
}



void SetFontStyle(const OUString &rStyleName, vcl::Font &rFont)
{
    // Find index related to StyleName. For an empty StyleName it's assumed to be
    // 0 (neither bold nor italic).
    sal_uInt16  nIndex = 0;
    if (!rStyleName.isEmpty())
    {
        sal_uInt16 i;
        const SmFontStyles &rStyles = GetFontStyles();
        for (i = 0;  i < rStyles.GetCount(); ++i)
            if (rStyleName == rStyles.GetStyleName(i))
                break;
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE(i < rStyles.GetCount(), "style-name unknown");
#endif
        nIndex = i;
    }

    rFont.SetItalic((nIndex & 0x1) ? ITALIC_NORMAL : ITALIC_NONE);
    rFont.SetWeight((nIndex & 0x2) ? WEIGHT_BOLD : WEIGHT_NORMAL);
}


/**************************************************************************/

IMPL_LINK_INLINE_START( SmPrintOptionsTabPage, SizeButtonClickHdl, Button *, EMPTYARG/*pButton*/ )
{
    m_pZoom->Enable(m_pSizeZoomed->IsChecked());
    return 0;
}
IMPL_LINK_INLINE_END( SmPrintOptionsTabPage, SizeButtonClickHdl, Button *, pButton )


SmPrintOptionsTabPage::SmPrintOptionsTabPage(vcl::Window *pParent, const SfxItemSet &rOptions)
    : SfxTabPage(pParent, "SmathSettings", "modules/smath/ui/smathsettings.ui", &rOptions)
{
    get( m_pTitle,               "title");
    get( m_pText,                "text");
    get( m_pFrame,               "frame");
    get( m_pSizeNormal,          "sizenormal");
    get( m_pSizeScaled,          "sizescaled");
    get( m_pSizeZoomed,          "sizezoomed");
    get( m_pZoom,                "zoom");
    get( m_pNoRightSpaces,       "norightspaces");
    get( m_pSaveOnlyUsedSymbols, "saveonlyusedsymbols");

    m_pSizeNormal->SetClickHdl(LINK(this, SmPrintOptionsTabPage, SizeButtonClickHdl));
    m_pSizeScaled->SetClickHdl(LINK(this, SmPrintOptionsTabPage, SizeButtonClickHdl));
    m_pSizeZoomed->SetClickHdl(LINK(this, SmPrintOptionsTabPage, SizeButtonClickHdl));

    Reset(&rOptions);
}


bool SmPrintOptionsTabPage::FillItemSet(SfxItemSet* rSet)
{
    sal_uInt16  nPrintSize;
    if (m_pSizeNormal->IsChecked())
        nPrintSize = PRINT_SIZE_NORMAL;
    else if (m_pSizeScaled->IsChecked())
        nPrintSize = PRINT_SIZE_SCALED;
    else
        nPrintSize = PRINT_SIZE_ZOOMED;

    rSet->Put(SfxUInt16Item(GetWhich(SID_PRINTSIZE), (sal_uInt16) nPrintSize));
    rSet->Put(SfxUInt16Item(GetWhich(SID_PRINTZOOM), (sal_uInt16) m_pZoom->GetValue()));
    rSet->Put(SfxBoolItem(GetWhich(SID_PRINTTITLE), m_pTitle->IsChecked()));
    rSet->Put(SfxBoolItem(GetWhich(SID_PRINTTEXT), m_pText->IsChecked()));
    rSet->Put(SfxBoolItem(GetWhich(SID_PRINTFRAME), m_pFrame->IsChecked()));
    rSet->Put(SfxBoolItem(GetWhich(SID_NO_RIGHT_SPACES), m_pNoRightSpaces->IsChecked()));
    rSet->Put(SfxBoolItem(GetWhich(SID_SAVE_ONLY_USED_SYMBOLS), m_pSaveOnlyUsedSymbols->IsChecked()));

    return true;
}


void SmPrintOptionsTabPage::Reset(const SfxItemSet* rSet)
{
    SmPrintSize ePrintSize = (SmPrintSize)((const SfxUInt16Item &)rSet->Get(GetWhich(SID_PRINTSIZE))).GetValue();

    m_pSizeNormal->Check(ePrintSize == PRINT_SIZE_NORMAL);
    m_pSizeScaled->Check(ePrintSize == PRINT_SIZE_SCALED);
    m_pSizeZoomed->Check(ePrintSize == PRINT_SIZE_ZOOMED);

    m_pZoom->Enable(m_pSizeZoomed->IsChecked());

    m_pZoom->SetValue(((const SfxUInt16Item &)rSet->Get(GetWhich(SID_PRINTZOOM))).GetValue());

    m_pTitle->Check(((const SfxBoolItem &)rSet->Get(GetWhich(SID_PRINTTITLE))).GetValue());
    m_pText->Check(((const SfxBoolItem &)rSet->Get(GetWhich(SID_PRINTTEXT))).GetValue());
    m_pFrame->Check(((const SfxBoolItem &)rSet->Get(GetWhich(SID_PRINTFRAME))).GetValue());
    m_pNoRightSpaces->Check(((const SfxBoolItem &)rSet->Get(GetWhich(SID_NO_RIGHT_SPACES))).GetValue());
    m_pSaveOnlyUsedSymbols->Check(((const SfxBoolItem &)rSet->Get(GetWhich(SID_SAVE_ONLY_USED_SYMBOLS))).GetValue());
}


SfxTabPage* SmPrintOptionsTabPage::Create(vcl::Window* pWindow, const SfxItemSet& rSet)
{
    return (new SmPrintOptionsTabPage(pWindow, rSet));
}

/**************************************************************************/


void SmShowFont::Paint(const Rectangle& rRect )
{
    Window::Paint( rRect );

    OUString   Text (GetFont().GetName());
    Size    TextSize(GetTextWidth(Text), GetTextHeight());

    DrawText(Point((GetOutputSize().Width()  - TextSize.Width())  / 2,
                   (GetOutputSize().Height() - TextSize.Height()) / 2), Text);
}

extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeSmShowFont(vcl::Window* pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = 0;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;

    return new SmShowFont(pParent, nWinStyle);
}

Size SmShowFont::GetOptimalSize() const
{
    return LogicToPixel(Size(111 , 31), MapMode(MAP_APPFONT));
}

void SmShowFont::SetFont(const vcl::Font& rFont)
{
    Color aTxtColor( GetTextColor() );
    vcl::Font aFont (rFont);

    Invalidate();
    aFont.SetSize(Size(0, 24));
    aFont.SetAlign(ALIGN_TOP);
    Window::SetFont(aFont);

    // keep old text color (new font may have different color)
    SetTextColor( aTxtColor );
}

IMPL_LINK_INLINE_START( SmFontDialog, FontSelectHdl, ComboBox *, pComboBox )
{
    Face.SetName(pComboBox->GetText());
    m_pShowFont->SetFont(Face);
    return 0;
}
IMPL_LINK_INLINE_END( SmFontDialog, FontSelectHdl, ComboBox *, pComboBox )


IMPL_LINK( SmFontDialog, FontModifyHdl, ComboBox *, pComboBox )
{
    // if font is available in list then use it
    sal_Int32 nPos = pComboBox->GetEntryPos( pComboBox->GetText() );
    if (COMBOBOX_ENTRY_NOTFOUND != nPos)
    {
        FontSelectHdl( pComboBox );
    }
    return 0;
}


IMPL_LINK( SmFontDialog, AttrChangeHdl, CheckBox *, EMPTYARG /*pCheckBox*/ )
{
    if (m_pBoldCheckBox->IsChecked())
        Face.SetWeight(FontWeight(WEIGHT_BOLD));
    else
        Face.SetWeight(FontWeight(WEIGHT_NORMAL));

    if (m_pItalicCheckBox->IsChecked())
        Face.SetItalic(ITALIC_NORMAL);
    else
        Face.SetItalic(ITALIC_NONE);

    m_pShowFont->SetFont(Face);
    return 0;
}


void SmFontDialog::SetFont(const vcl::Font &rFont)
{
    Face = rFont;

    m_pFontBox->SetText( Face.GetName() );
    m_pBoldCheckBox->Check( IsBold( Face ) );
    m_pItalicCheckBox->Check( IsItalic( Face ) );

    m_pShowFont->SetFont(Face);
}

SmFontDialog::SmFontDialog(vcl::Window * pParent, OutputDevice *pFntListDevice, bool bHideCheckboxes)
    : ModalDialog(pParent, "FontDialog", "modules/smath/ui/fontdialog.ui")
{
    get(m_pFontBox, "font");
    m_pFontBox->set_height_request(8 * m_pFontBox->GetTextHeight());
    get(m_pAttrFrame, "attrframe");
    get(m_pBoldCheckBox, "bold");
    get(m_pItalicCheckBox, "italic");
    get(m_pShowFont, "preview");

    {
        WaitObject aWait( this );

        FontList aFontList( pFntListDevice );

        sal_uInt16  nCount = aFontList.GetFontNameCount();
        for (sal_uInt16 i = 0;  i < nCount; ++i)
            m_pFontBox->InsertEntry( aFontList.GetFontName(i).GetName() );

        Face.SetSize(Size(0, 24));
        Face.SetWeight(WEIGHT_NORMAL);
        Face.SetItalic(ITALIC_NONE);
        Face.SetFamily(FAMILY_DONTKNOW);
        Face.SetPitch(PITCH_DONTKNOW);
        Face.SetCharSet(RTL_TEXTENCODING_DONTKNOW);
        Face.SetTransparent(true);

        InitColor_Impl();

        // preview like controls should have a 2D look
        m_pShowFont->SetBorderStyle( WindowBorderStyle::MONO );
    }

    m_pFontBox->SetSelectHdl(LINK(this, SmFontDialog, FontSelectHdl));
    m_pFontBox->SetModifyHdl(LINK(this, SmFontDialog, FontModifyHdl));
    m_pBoldCheckBox->SetClickHdl(LINK(this, SmFontDialog, AttrChangeHdl));
    m_pItalicCheckBox->SetClickHdl(LINK(this, SmFontDialog, AttrChangeHdl));

    if (bHideCheckboxes)
    {
        m_pBoldCheckBox->Check( false );
        m_pBoldCheckBox->Enable( false );
        m_pItalicCheckBox->Check( false );
        m_pItalicCheckBox->Enable( false );
        m_pAttrFrame->Show(false);
    }
}

namespace
{
    void getColors(vcl::Window &rRef, ColorData &rBgCol, ColorData &rTxtCol)
    {
        const StyleSettings &rS = rRef.GetSettings().GetStyleSettings();
        if (rS.GetHighContrastMode())
        {
            rBgCol  = rS.GetFieldColor().GetColor();
            rTxtCol = rS.GetFieldTextColor().GetColor();
        }
        else
        {
            rBgCol  = COL_WHITE;
            rTxtCol = COL_BLACK;
        }
    }
}

void SmFontDialog::InitColor_Impl()
{
    ColorData nBgCol, nTxtCol;
    getColors(*this, nBgCol, nTxtCol);

    Color aTmpColor( nBgCol );
    Wallpaper aWall( aTmpColor );
    Color aTxtColor( nTxtCol );
    m_pShowFont->SetBackground( aWall );
    m_pShowFont->SetTextColor( aTxtColor );
}

void SmFontDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DATACHANGED_SETTINGS  &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            InitColor_Impl();

    ModalDialog::DataChanged( rDCEvt );
}

class SaveDefaultsQuery : public MessageDialog
{
public:
    SaveDefaultsQuery(vcl::Window *pParent)
        : MessageDialog(pParent, "SaveDefaultsDialog",
            "modules/smath/ui/savedefaultsdialog.ui")
    {
    }
};

IMPL_LINK( SmFontSizeDialog, DefaultButtonClickHdl, Button *, EMPTYARG /*pButton*/ )
{
    if (SaveDefaultsQuery(this).Execute() == RET_YES)
    {
        SmModule *pp = SM_MOD();
        SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
        WriteTo( aFmt );
        pp->GetConfig()->SetStandardFormat( aFmt );
    }
    return 0;
}

SmFontSizeDialog::SmFontSizeDialog(vcl::Window * pParent)
    : ModalDialog(pParent, "FontSizeDialog", "modules/smath/ui/fontsizedialog.ui")
{
    get(m_pTextSize, "spinB_text");
    get(m_pIndexSize, "spinB_index");
    get(m_pFunctionSize, "spinB_function");
    get(m_pOperatorSize, "spinB_operator");
    get(m_pBorderSize, "spinB_limit");
    get(m_pBaseSize, "spinB_baseSize");
    get(m_pDefaultButton, "default");

    m_pDefaultButton->SetClickHdl(LINK(this, SmFontSizeDialog, DefaultButtonClickHdl));
}


void SmFontSizeDialog::ReadFrom(const SmFormat &rFormat)
{
    //! aufpassen: richtig runden!
    m_pBaseSize->SetValue( SmRoundFraction(
        Sm100th_mmToPts( rFormat.GetBaseSize().Height() ) ) );

    m_pTextSize->SetValue( rFormat.GetRelSize(SIZ_TEXT) );
    m_pIndexSize->SetValue( rFormat.GetRelSize(SIZ_INDEX) );
    m_pFunctionSize->SetValue( rFormat.GetRelSize(SIZ_FUNCTION) );
    m_pOperatorSize->SetValue( rFormat.GetRelSize(SIZ_OPERATOR) );
    m_pBorderSize->SetValue( rFormat.GetRelSize(SIZ_LIMITS) );
}


void SmFontSizeDialog::WriteTo(SmFormat &rFormat) const
{
    rFormat.SetBaseSize( Size(0, SmPtsTo100th_mm( static_cast< long >(m_pBaseSize->GetValue()))) );

    rFormat.SetRelSize(SIZ_TEXT,     (sal_uInt16) m_pTextSize->GetValue());
    rFormat.SetRelSize(SIZ_INDEX,    (sal_uInt16) m_pIndexSize->GetValue());
    rFormat.SetRelSize(SIZ_FUNCTION, (sal_uInt16) m_pFunctionSize->GetValue());
    rFormat.SetRelSize(SIZ_OPERATOR, (sal_uInt16) m_pOperatorSize->GetValue());
    rFormat.SetRelSize(SIZ_LIMITS,   (sal_uInt16) m_pBorderSize->GetValue());

    const Size aTmp (rFormat.GetBaseSize());
    for (sal_uInt16  i = FNT_BEGIN;  i <= FNT_END;  i++)
        rFormat.SetFontSize(i, aTmp);

    rFormat.RequestApplyChanges();
}


/**************************************************************************/


IMPL_LINK( SmFontTypeDialog, MenuSelectHdl, Menu *, pMenu )
{
    SmFontPickListBox *pActiveListBox;

    bool bHideCheckboxes = false;
    switch (pMenu->GetCurItemId())
    {
        case 1: pActiveListBox = m_pVariableFont; break;
        case 2: pActiveListBox = m_pFunctionFont; break;
        case 3: pActiveListBox = m_pNumberFont;   break;
        case 4: pActiveListBox = m_pTextFont;     break;
        case 5: pActiveListBox = m_pSerifFont; bHideCheckboxes = true;   break;
        case 6: pActiveListBox = m_pSansFont;  bHideCheckboxes = true;   break;
        case 7: pActiveListBox = m_pFixedFont; bHideCheckboxes = true;   break;
        default:pActiveListBox = NULL;
    }

    if (pActiveListBox)
    {
        SmFontDialog *pFontDialog = new SmFontDialog(this, pFontListDev, bHideCheckboxes);

        pActiveListBox->WriteTo(*pFontDialog);
        if (pFontDialog->Execute() == RET_OK)
            pActiveListBox->ReadFrom(*pFontDialog);
        delete pFontDialog;
    }
    return 0;
}


IMPL_LINK_INLINE_START( SmFontTypeDialog, DefaultButtonClickHdl, Button *, EMPTYARG /*pButton*/ )
{
    if (SaveDefaultsQuery(this).Execute() == RET_YES)
    {
        SmModule *pp = SM_MOD();
        SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
        WriteTo( aFmt );
        pp->GetConfig()->SetStandardFormat( aFmt, true );
    }
    return 0;
}
IMPL_LINK_INLINE_END( SmFontTypeDialog, DefaultButtonClickHdl, Button *, pButton )

SmFontTypeDialog::SmFontTypeDialog(vcl::Window * pParent, OutputDevice *pFntListDevice)
    : ModalDialog(pParent, "FontsDialog", "modules/smath/ui/fonttypedialog.ui"),
    pFontListDev    (pFntListDevice)
{
    get(m_pVariableFont, "variableCB");
    get(m_pFunctionFont, "functionCB");
    get(m_pNumberFont, "numberCB");
    get(m_pTextFont, "textCB");
    get(m_pSerifFont, "serifCB");
    get(m_pSansFont, "sansCB");
    get(m_pFixedFont, "fixedCB");
    get(m_pMenuButton, "modify");
    get(m_pDefaultButton, "default");

    m_pDefaultButton->SetClickHdl(LINK(this, SmFontTypeDialog, DefaultButtonClickHdl));

    m_pMenuButton->GetPopupMenu()->SetSelectHdl(LINK(this, SmFontTypeDialog, MenuSelectHdl));
}

void SmFontTypeDialog::ReadFrom(const SmFormat &rFormat)
{
    SmModule *pp = SM_MOD();

    *m_pVariableFont = pp->GetConfig()->GetFontPickList(FNT_VARIABLE);
    *m_pFunctionFont = pp->GetConfig()->GetFontPickList(FNT_FUNCTION);
    *m_pNumberFont   = pp->GetConfig()->GetFontPickList(FNT_NUMBER);
    *m_pTextFont     = pp->GetConfig()->GetFontPickList(FNT_TEXT);
    *m_pSerifFont    = pp->GetConfig()->GetFontPickList(FNT_SERIF);
    *m_pSansFont     = pp->GetConfig()->GetFontPickList(FNT_SANS);
    *m_pFixedFont    = pp->GetConfig()->GetFontPickList(FNT_FIXED);

    m_pVariableFont->Insert( rFormat.GetFont(FNT_VARIABLE) );
    m_pFunctionFont->Insert( rFormat.GetFont(FNT_FUNCTION) );
    m_pNumberFont->Insert( rFormat.GetFont(FNT_NUMBER) );
    m_pTextFont->Insert( rFormat.GetFont(FNT_TEXT) );
    m_pSerifFont->Insert( rFormat.GetFont(FNT_SERIF) );
    m_pSansFont->Insert( rFormat.GetFont(FNT_SANS) );
    m_pFixedFont->Insert( rFormat.GetFont(FNT_FIXED) );
}


void SmFontTypeDialog::WriteTo(SmFormat &rFormat) const
{
    SmModule *pp = SM_MOD();

    pp->GetConfig()->GetFontPickList(FNT_VARIABLE) = *m_pVariableFont;
    pp->GetConfig()->GetFontPickList(FNT_FUNCTION) = *m_pFunctionFont;
    pp->GetConfig()->GetFontPickList(FNT_NUMBER)   = *m_pNumberFont;
    pp->GetConfig()->GetFontPickList(FNT_TEXT)     = *m_pTextFont;
    pp->GetConfig()->GetFontPickList(FNT_SERIF)    = *m_pSerifFont;
    pp->GetConfig()->GetFontPickList(FNT_SANS)     = *m_pSansFont;
    pp->GetConfig()->GetFontPickList(FNT_FIXED)    = *m_pFixedFont;

    rFormat.SetFont( FNT_VARIABLE, m_pVariableFont->Get(0) );
    rFormat.SetFont( FNT_FUNCTION, m_pFunctionFont->Get(0) );
    rFormat.SetFont( FNT_NUMBER,   m_pNumberFont->Get(0) );
    rFormat.SetFont( FNT_TEXT,     m_pTextFont->Get(0) );
    rFormat.SetFont( FNT_SERIF,    m_pSerifFont->Get(0) );
    rFormat.SetFont( FNT_SANS,     m_pSansFont->Get(0) );
    rFormat.SetFont( FNT_FIXED,    m_pFixedFont->Get(0) );

    rFormat.RequestApplyChanges();
}

/**************************************************************************/

struct FieldMinMax
{
    sal_uInt16 nMin, nMax;
};

// Data for min and max values of the 4 metric fields
// for each of the 10 categories
static const FieldMinMax pMinMaxData[10][4] =
{
    // 0
    {{ 0, 200 },    { 0, 200 },     { 0, 100 },     { 0, 0 }},
    // 1
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 2
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 3
    {{ 0, 100 },    { 1, 100 },     { 0, 0 },       { 0, 0 }},
    // 4
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 5
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 100 }},
    // 6
    {{ 0, 300 },    { 0, 300 },     { 0, 0 },       { 0, 0 }},
    // 7
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 8
    {{ 0, 100 },    { 0, 100 },     { 0, 0 },       { 0, 0 }},
    // 9
    {{ 0, 10000 },  { 0, 10000 },   { 0, 10000 },   { 0, 10000 }}
};

SmCategoryDesc::SmCategoryDesc(VclBuilderContainer& rBuilder, sal_uInt16 nCategoryIdx)
{
    ++nCategoryIdx;
    FixedText* pTitle = rBuilder.get<FixedText>(OString::number(nCategoryIdx)+"title");
    if (pTitle)
    {
        Name = pTitle->GetText();
    }
    for (int i = 0; i < 4; ++i)
    {
        FixedText* pLabel = rBuilder.get<FixedText>(OString::number(nCategoryIdx)+"label"+OString::number(i+1));

        if (pLabel)
        {
            Strings  [i] = new OUString(pLabel->GetText());
            FixedImage* pImage = rBuilder.get<FixedImage>(OString::number(nCategoryIdx)+"image"+OString::number(i+1));
            Graphics [i] = new Image(pImage->GetImage());
        }
        else
        {
            Strings  [i] = 0;
            Graphics [i] = 0;
        }

        const FieldMinMax& rMinMax = pMinMaxData[ nCategoryIdx-1 ][i];
        Value[i] = Minimum[i] = rMinMax.nMin;
        Maximum[i] = rMinMax.nMax;
    }
}

SmCategoryDesc::~SmCategoryDesc()
{
    for (int i = 0; i < 4; ++i)
    {
        delete Strings  [i];
        delete Graphics [i];
    }
}

/**************************************************************************/

IMPL_LINK( SmDistanceDialog, GetFocusHdl, Control *, pControl )
{
    if (Categories[nActiveCategory])
    {
        sal_uInt16  i;

        if (pControl == m_pMetricField1)
            i = 0;
        else if (pControl == m_pMetricField2)
            i = 1;
        else if (pControl == m_pMetricField3)
            i = 2;
        else if (pControl == m_pMetricField4)
            i = 3;
        else
            return 0;
        m_pBitmap->SetImage(*(Categories[nActiveCategory]->GetGraphic(i)));
    }
    return 0;
}

IMPL_LINK( SmDistanceDialog, MenuSelectHdl, Menu *, pMenu )
{
    SetCategory(pMenu->GetCurItemId() - 1);
    return 0;
}


IMPL_LINK( SmDistanceDialog, DefaultButtonClickHdl, Button *, EMPTYARG /*pButton*/ )
{
    if (SaveDefaultsQuery(this).Execute() == RET_YES)
    {
        SmModule *pp = SM_MOD();
        SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
        WriteTo( aFmt );
        pp->GetConfig()->SetStandardFormat( aFmt );
    }
    return 0;
}

IMPL_LINK( SmDistanceDialog, CheckBoxClickHdl, CheckBox *, pCheckBox )
{
    if (pCheckBox == m_pCheckBox1)
    {
        m_pCheckBox1->Toggle();

        bool bChecked = m_pCheckBox1->IsChecked();
        m_pFixedText4->Enable( bChecked );
        m_pMetricField4->Enable( bChecked );
    }
    return 0;
}


void SmDistanceDialog::SetHelpId(MetricField &rField, const OString& sHelpId)
{
    const OUString aEmptyText;

    rField.SetHelpId(sHelpId);
    rField.SetHelpText(aEmptyText);

    // since MetricField inherits from SpinField which has a sub Edit field
    // (which is actually the one we modify) we have to set the help-id
    // for it too.
    Edit *pSubEdit = rField.GetSubEdit();
    if (pSubEdit)
    {
        pSubEdit->SetHelpId(sHelpId);
        pSubEdit->SetHelpText(aEmptyText);
    }
}


void SmDistanceDialog::SetCategory(sal_uInt16 nCategory)
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(/*0 <= nCategory  &&*/  nCategory < NOCATEGORIES,
        "Sm: wrong category number in SmDistanceDialog");
#endif

    // array to convert category- and metricfield-number in help ids.
    // 0 is used in case of unused combinations.
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(NOCATEGORIES == 10, "Sm : array doesn't fit into the number of categories");
#endif
    static const char * aCatMf2Hid[10][4] =
    {
        { HID_SMA_DEFAULT_DIST,         HID_SMA_LINE_DIST,          HID_SMA_ROOT_DIST, 0 },
        { HID_SMA_SUP_DIST,             HID_SMA_SUB_DIST ,          0, 0 },
        { HID_SMA_NUMERATOR_DIST,       HID_SMA_DENOMINATOR_DIST,   0, 0 },
        { HID_SMA_FRACLINE_EXCWIDTH,    HID_SMA_FRACLINE_LINEWIDTH, 0, 0 },
        { HID_SMA_UPPERLIMIT_DIST,      HID_SMA_LOWERLIMIT_DIST,    0, 0 },
        { HID_SMA_BRACKET_EXCHEIGHT,    HID_SMA_BRACKET_DIST,       0, HID_SMA_BRACKET_EXCHEIGHT2 },
        { HID_SMA_MATRIXROW_DIST,       HID_SMA_MATRIXCOL_DIST,     0, 0 },
        { HID_SMA_ATTRIBUT_DIST,        HID_SMA_INTERATTRIBUT_DIST, 0, 0 },
        { HID_SMA_OPERATOR_EXCHEIGHT,   HID_SMA_OPERATOR_DIST,      0, 0 },
        { HID_SMA_LEFTBORDER_DIST,      HID_SMA_RIGHTBORDER_DIST,   HID_SMA_UPPERBORDER_DIST, HID_SMA_LOWERBORDER_DIST }
    };

    // array to help iterate over the controls
    vcl::Window * const  aWin[4][2] =
    {
        { m_pFixedText1, m_pMetricField1 },
        { m_pFixedText2, m_pMetricField2 },
        { m_pFixedText3, m_pMetricField3 },
        { m_pFixedText4, m_pMetricField4 }
    };

    SmCategoryDesc *pCat;

    // remember the (maybe new) settings of the active SmCategoryDesc
    // before switching to the new one
    if (nActiveCategory != CATEGORY_NONE)
    {
        pCat = Categories[nActiveCategory];
        pCat->SetValue(0, (sal_uInt16) m_pMetricField1->GetValue());
        pCat->SetValue(1, (sal_uInt16) m_pMetricField2->GetValue());
        pCat->SetValue(2, (sal_uInt16) m_pMetricField3->GetValue());
        pCat->SetValue(3, (sal_uInt16) m_pMetricField4->GetValue());

        if (nActiveCategory == 5)
            bScaleAllBrackets = m_pCheckBox1->IsChecked();

        m_pMenuButton->GetPopupMenu()->CheckItem(nActiveCategory + 1, false);
    }

    // activation/deactivation of the associated controls depending on the chosen category
    bool  bActive;
    for (sal_uInt16 i = 0;  i < 4;  i++)
    {
        FixedText   *pFT = (FixedText * const)   aWin[i][0];
        MetricField *pMF = (MetricField * const) aWin[i][1];

        // To determine which Controls should be active, the existence
        // of an associated HelpID is checked
        bActive = aCatMf2Hid[nCategory][i] != 0;

        pFT->Show(bActive);
        pFT->Enable(bActive);
        pMF->Show(bActive);
        pMF->Enable(bActive);

        // set measurement unit and number of decimal places
        FieldUnit  eUnit;
        sal_uInt16     nDigits;
        if (nCategory < 9)
        {
            eUnit = FUNIT_PERCENT;
            nDigits = 0;
        }
        else
        {
            eUnit   = FUNIT_100TH_MM;
            nDigits = 2;
        }
        pMF->SetUnit(eUnit);            // changes the value
        pMF->SetDecimalDigits(nDigits);

        if (bActive)
        {
            pCat = Categories[nCategory];
            pFT->SetText(*pCat->GetString(i));

            pMF->SetMin(pCat->GetMinimum(i));
            pMF->SetMax(pCat->GetMaximum(i));
            pMF->SetValue(pCat->GetValue(i));

            SetHelpId(*pMF, aCatMf2Hid[nCategory][i]);
        }
    }
    // activate the CheckBox and the associated MetricField if we're dealing with the brackets menu
    bActive = nCategory == 5;
    m_pCheckBox1->Show(bActive);
    m_pCheckBox1->Enable(bActive);
    if (bActive)
    {
        m_pCheckBox1->Check( bScaleAllBrackets );

        bool bChecked = m_pCheckBox1->IsChecked();
        m_pFixedText4->Enable( bChecked );
        m_pMetricField4->Enable( bChecked );
    }

    m_pMenuButton->GetPopupMenu()->CheckItem(nCategory + 1, true);
    m_pFrame->set_label(Categories[nCategory]->GetName());

    nActiveCategory = nCategory;

    m_pMetricField1->GrabFocus();
    Invalidate();
    Update();
}


SmDistanceDialog::SmDistanceDialog(vcl::Window *pParent)
    : ModalDialog(pParent, "SpacingDialog",
        "modules/smath/ui/spacingdialog.ui")
{
    get(m_pFrame, "template");
    get(m_pFixedText1, "label1");
    get(m_pMetricField1, "spinbutton1");
    get(m_pFixedText2, "label2");
    get(m_pMetricField2, "spinbutton2");
    get(m_pFixedText3, "label3");
    get(m_pMetricField3, "spinbutton3");
    get(m_pCheckBox1, "checkbutton");
    get(m_pFixedText4, "label4");
    get(m_pMetricField4, "spinbutton4");
    get(m_pMenuButton, "category");
    get(m_pDefaultButton, "default");
    get(m_pBitmap, "image");

    for (sal_uInt16 i = 0; i < NOCATEGORIES; ++i)
        Categories[i] = new SmCategoryDesc(*this, i);
    nActiveCategory   = CATEGORY_NONE;
    bScaleAllBrackets = false;

    // preview like controls should have a 2D look
    m_pBitmap->SetBorderStyle( WindowBorderStyle::MONO );

    m_pMetricField1->SetGetFocusHdl(LINK(this, SmDistanceDialog, GetFocusHdl));
    m_pMetricField2->SetGetFocusHdl(LINK(this, SmDistanceDialog, GetFocusHdl));
    m_pMetricField3->SetGetFocusHdl(LINK(this, SmDistanceDialog, GetFocusHdl));
    m_pMetricField4->SetGetFocusHdl(LINK(this, SmDistanceDialog, GetFocusHdl));
    m_pCheckBox1->SetClickHdl(LINK(this, SmDistanceDialog, CheckBoxClickHdl));

    m_pMenuButton->GetPopupMenu()->SetSelectHdl(LINK(this, SmDistanceDialog, MenuSelectHdl));

    m_pDefaultButton->SetClickHdl(LINK(this, SmDistanceDialog, DefaultButtonClickHdl));
}


SmDistanceDialog::~SmDistanceDialog()
{
    for (int i = 0; i < NOCATEGORIES; i++)
        DELETEZ(Categories[i]);
}

void SmDistanceDialog::DataChanged( const DataChangedEvent &rEvt )
{
    ModalDialog::DataChanged( rEvt );
}

void SmDistanceDialog::ReadFrom(const SmFormat &rFormat)
{
    Categories[0]->SetValue(0, rFormat.GetDistance(DIS_HORIZONTAL));
    Categories[0]->SetValue(1, rFormat.GetDistance(DIS_VERTICAL));
    Categories[0]->SetValue(2, rFormat.GetDistance(DIS_ROOT));
    Categories[1]->SetValue(0, rFormat.GetDistance(DIS_SUPERSCRIPT));
    Categories[1]->SetValue(1, rFormat.GetDistance(DIS_SUBSCRIPT));
    Categories[2]->SetValue(0, rFormat.GetDistance(DIS_NUMERATOR));
    Categories[2]->SetValue(1, rFormat.GetDistance(DIS_DENOMINATOR));
    Categories[3]->SetValue(0, rFormat.GetDistance(DIS_FRACTION));
    Categories[3]->SetValue(1, rFormat.GetDistance(DIS_STROKEWIDTH));
    Categories[4]->SetValue(0, rFormat.GetDistance(DIS_UPPERLIMIT));
    Categories[4]->SetValue(1, rFormat.GetDistance(DIS_LOWERLIMIT));
    Categories[5]->SetValue(0, rFormat.GetDistance(DIS_BRACKETSIZE));
    Categories[5]->SetValue(1, rFormat.GetDistance(DIS_BRACKETSPACE));
    Categories[5]->SetValue(3, rFormat.GetDistance(DIS_NORMALBRACKETSIZE));
    Categories[6]->SetValue(0, rFormat.GetDistance(DIS_MATRIXROW));
    Categories[6]->SetValue(1, rFormat.GetDistance(DIS_MATRIXCOL));
    Categories[7]->SetValue(0, rFormat.GetDistance(DIS_ORNAMENTSIZE));
    Categories[7]->SetValue(1, rFormat.GetDistance(DIS_ORNAMENTSPACE));
    Categories[8]->SetValue(0, rFormat.GetDistance(DIS_OPERATORSIZE));
    Categories[8]->SetValue(1, rFormat.GetDistance(DIS_OPERATORSPACE));
    Categories[9]->SetValue(0, rFormat.GetDistance(DIS_LEFTSPACE));
    Categories[9]->SetValue(1, rFormat.GetDistance(DIS_RIGHTSPACE));
    Categories[9]->SetValue(2, rFormat.GetDistance(DIS_TOPSPACE));
    Categories[9]->SetValue(3, rFormat.GetDistance(DIS_BOTTOMSPACE));

    bScaleAllBrackets = rFormat.IsScaleNormalBrackets();

    // force update (even of category 0) by setting nActiveCategory to a
    // non-existent category number
    nActiveCategory = CATEGORY_NONE;
    SetCategory(0);
}


void SmDistanceDialog::WriteTo(SmFormat &rFormat) /*const*/
{
    // TODO can they actually be different?
    // if that's not the case 'const' could be used above!
    SetCategory(nActiveCategory);

    rFormat.SetDistance( DIS_HORIZONTAL,        Categories[0]->GetValue(0) );
    rFormat.SetDistance( DIS_VERTICAL,          Categories[0]->GetValue(1) );
    rFormat.SetDistance( DIS_ROOT,              Categories[0]->GetValue(2) );
    rFormat.SetDistance( DIS_SUPERSCRIPT,       Categories[1]->GetValue(0) );
    rFormat.SetDistance( DIS_SUBSCRIPT,         Categories[1]->GetValue(1) );
    rFormat.SetDistance( DIS_NUMERATOR,         Categories[2]->GetValue(0) );
    rFormat.SetDistance( DIS_DENOMINATOR,       Categories[2]->GetValue(1) );
    rFormat.SetDistance( DIS_FRACTION,          Categories[3]->GetValue(0) );
    rFormat.SetDistance( DIS_STROKEWIDTH,       Categories[3]->GetValue(1) );
    rFormat.SetDistance( DIS_UPPERLIMIT,        Categories[4]->GetValue(0) );
    rFormat.SetDistance( DIS_LOWERLIMIT,        Categories[4]->GetValue(1) );
    rFormat.SetDistance( DIS_BRACKETSIZE,       Categories[5]->GetValue(0) );
    rFormat.SetDistance( DIS_BRACKETSPACE,      Categories[5]->GetValue(1) );
    rFormat.SetDistance( DIS_MATRIXROW,         Categories[6]->GetValue(0) );
    rFormat.SetDistance( DIS_MATRIXCOL,         Categories[6]->GetValue(1) );
    rFormat.SetDistance( DIS_ORNAMENTSIZE,      Categories[7]->GetValue(0) );
    rFormat.SetDistance( DIS_ORNAMENTSPACE,     Categories[7]->GetValue(1) );
    rFormat.SetDistance( DIS_OPERATORSIZE,      Categories[8]->GetValue(0) );
    rFormat.SetDistance( DIS_OPERATORSPACE,     Categories[8]->GetValue(1) );
    rFormat.SetDistance( DIS_LEFTSPACE,         Categories[9]->GetValue(0) );
    rFormat.SetDistance( DIS_RIGHTSPACE,        Categories[9]->GetValue(1) );
    rFormat.SetDistance( DIS_TOPSPACE,          Categories[9]->GetValue(2) );
    rFormat.SetDistance( DIS_BOTTOMSPACE,       Categories[9]->GetValue(3) );
    rFormat.SetDistance( DIS_NORMALBRACKETSIZE, Categories[5]->GetValue(3) );

    rFormat.SetScaleNormalBrackets( bScaleAllBrackets );

    rFormat.RequestApplyChanges();
}

IMPL_LINK( SmAlignDialog, DefaultButtonClickHdl, Button *, EMPTYARG /*pButton*/ )
{
    if (SaveDefaultsQuery(this).Execute() == RET_YES)
    {
        SmModule *pp = SM_MOD();
        SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
        WriteTo( aFmt );
        pp->GetConfig()->SetStandardFormat( aFmt );
    }
    return 0;
}

SmAlignDialog::SmAlignDialog(vcl::Window * pParent)
    : ModalDialog(pParent, "AlignmentDialog",
        "modules/smath/ui/alignmentdialog.ui")
{
    get(m_pLeft, "left");
    get(m_pCenter, "center");
    get(m_pRight, "right");
    get(m_pDefaultButton, "default");
    m_pDefaultButton->SetClickHdl(LINK(this, SmAlignDialog, DefaultButtonClickHdl));
}


void SmAlignDialog::ReadFrom(const SmFormat &rFormat)
{
    switch (rFormat.GetHorAlign())
    {
        case AlignLeft:
            m_pLeft->Check(true);
            m_pCenter->Check(false);
            m_pRight->Check(false);
            break;

        case AlignCenter:
            m_pLeft->Check(false);
            m_pCenter->Check(true);
            m_pRight->Check(false);
            break;

        case AlignRight:
            m_pLeft->Check(false);
            m_pCenter->Check(false);
            m_pRight->Check(true);
            break;
    }
}


void SmAlignDialog::WriteTo(SmFormat &rFormat) const
{
    if (m_pLeft->IsChecked())
        rFormat.SetHorAlign(AlignLeft);
    else if (m_pRight->IsChecked())
        rFormat.SetHorAlign(AlignRight);
    else
        rFormat.SetHorAlign(AlignCenter);

    rFormat.RequestApplyChanges();
}


SmShowSymbolSetWindow::SmShowSymbolSetWindow(vcl::Window *pParent, WinBits nStyle)
    : Control(pParent, nStyle)
    , m_pVScrollBar(0)
    , nLen(0)
    , nRows(0)
    , nColumns(0)
    , nXOffset(0)
    , nYOffset(0)
    , nSelectSymbol(SYMBOL_NONE)
{
    ColorData nBgCol, nTxtCol;
    getColors(*this, nBgCol, nTxtCol);

    Color aTmpColor( nBgCol );
    Wallpaper aWall( aTmpColor );
    Color aTxtColor( nTxtCol );
    SetBackground( aWall );
    SetTextColor( aTxtColor );
}

Point SmShowSymbolSetWindow::OffsetPoint(const Point &rPoint) const
{
    return Point(rPoint.X() + nXOffset, rPoint.Y() + nYOffset);
}

void SmShowSymbolSetWindow::Paint(const Rectangle&)
{
    Push(PushFlags::MAPMODE);

    // set MapUnit for which 'nLen' has been calculated
    SetMapMode(MapMode(MAP_PIXEL));

    sal_uInt16 v        = sal::static_int_cast< sal_uInt16 >((m_pVScrollBar->GetThumbPos() * nColumns));
    size_t nSymbols = aSymbolSet.size();

    Color aTxtColor( GetTextColor() );
    for (sal_uInt16 i = v; i < nSymbols ; i++)
    {
        SmSym     aSymbol (*aSymbolSet[i]);
        vcl::Font aFont   (aSymbol.GetFace());
        aFont.SetAlign(ALIGN_TOP);

        // taking a FontSize which is a bit smaller (compared to nLen) in order to have a buffer
        // (hopefully enough for left and right, too)
        aFont.SetSize(Size(0, nLen - (nLen / 3)));
        SetFont(aFont);
        // keep text color
        SetTextColor( aTxtColor );

        int   nIV   = i - v;
        sal_UCS4 cChar = aSymbol.GetCharacter();
        OUString aText(&cChar, 1);
        Size  aSize( GetTextWidth( aText ), GetTextHeight());

        Point aPoint((nIV % nColumns) * nLen + (nLen - aSize.Width()) / 2,
                       (nIV / nColumns) * nLen + (nLen - aSize.Height()) / 2);

        DrawText(OffsetPoint(aPoint), aText);
    }

    if (nSelectSymbol != SYMBOL_NONE)
    {
        Point aPoint(((nSelectSymbol - v) % nColumns) * nLen,
                                 ((nSelectSymbol - v) / nColumns) * nLen);

        Invert(Rectangle(OffsetPoint(aPoint), Size(nLen, nLen)));

    }

    Pop();
}


void SmShowSymbolSetWindow::MouseButtonDown(const MouseEvent& rMEvt)
{
    GrabFocus();

    Size aOutputSize(nColumns * nLen, nRows * nLen);
    Point aPoint(rMEvt.GetPosPixel());
    aPoint.X() -= nXOffset;
    aPoint.Y() -= nYOffset;

    if (rMEvt.IsLeft() && Rectangle(Point(0, 0), aOutputSize).IsInside(rMEvt.GetPosPixel()))
    {
        long nPos = (aPoint.Y() / nLen) * nColumns + (aPoint.X() / nLen) +
                      m_pVScrollBar->GetThumbPos() * nColumns;
        SelectSymbol( sal::static_int_cast< sal_uInt16 >(nPos) );

        aSelectHdlLink.Call(this);

        if (rMEvt.GetClicks() > 1)
            aDblClickHdlLink.Call(this);
    }
}


void SmShowSymbolSetWindow::KeyInput(const KeyEvent& rKEvt)
{
    sal_uInt16 n = nSelectSymbol;

    if (n != SYMBOL_NONE)
    {
        switch (rKEvt.GetKeyCode().GetCode())
        {
            case KEY_DOWN:      n = n + nColumns;   break;
            case KEY_UP:        n = n - nColumns;   break;
            case KEY_LEFT:      n -= 1; break;
            case KEY_RIGHT:     n += 1; break;
            case KEY_HOME:      n  = 0; break;
            case KEY_END:       n  = static_cast< sal_uInt16 >(aSymbolSet.size() - 1);   break;
            case KEY_PAGEUP:    n -= nColumns * nRows;  break;
            case KEY_PAGEDOWN:  n += nColumns * nRows;  break;

            default:
                Control::KeyInput(rKEvt);
                return;
        }
    }
    else
        n = 0;

    if (n >= aSymbolSet.size())
        n = nSelectSymbol;

    // adjust scrollbar
    if ((n < (sal_uInt16) (m_pVScrollBar->GetThumbPos() * nColumns)) ||
        (n >= (sal_uInt16) ((m_pVScrollBar->GetThumbPos() + nRows) * nColumns)))
    {
        m_pVScrollBar->SetThumbPos(n / nColumns);
        Invalidate();
        Update();
    }

    SelectSymbol(n);
    aSelectHdlLink.Call(this);
}

void SmShowSymbolSetWindow::setScrollbar(ScrollBar *pVScrollBar)
{
    m_pVScrollBar = pVScrollBar;
    m_pVScrollBar->Enable(false);
    m_pVScrollBar->Show();
    m_pVScrollBar->SetScrollHdl(LINK(this, SmShowSymbolSetWindow, ScrollHdl));
}

SmShowSymbolSet::SmShowSymbolSet(vcl::Window *pParent)
    : VclHBox(pParent, false, 6)
    , aSymbolWindow(this, WB_TABSTOP)
    , aVScrollBar(this, WinBits(WB_VSCROLL))
{
    aSymbolWindow.set_hexpand(true);
    aSymbolWindow.set_vexpand(true);
    aSymbolWindow.setScrollbar(&aVScrollBar);
    aSymbolWindow.calccols();
    aSymbolWindow.Show();
}

extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeSmShowSymbolSet(vcl::Window *pParent, VclBuilder::stringmap &)
{
    return new SmShowSymbolSet(pParent);
}

void SmShowSymbolSetWindow::calccols()
{
    // Height of 16pt in pixels (matching 'aOutputSize')
    nLen = LogicToPixel(Size(0, 16), MapMode(MAP_POINT)).Height();

    Size aOutputSize = GetOutputSizePixel();

    nColumns = aOutputSize.Width() / nLen;
    if (nColumns > 2  && nColumns % 2 != 0)
        --nColumns;
    nRows = aOutputSize.Height() / nLen;
    nColumns = std::max<long>(1, nColumns);
    nRows = std::max<long>(1, nRows);

    nXOffset = (aOutputSize.Width() - (nColumns * nLen)) / 2;
    nYOffset = (aOutputSize.Height() - (nRows * nLen)) / 2;

    SetScrollBarRange();
}

Size SmShowSymbolSetWindow::GetOptimalSize() const
{
    vcl::Window *pParent = GetParent();
    return Size(pParent->approximate_char_width() * 24, pParent->GetTextHeight() * 8);
}

void SmShowSymbolSetWindow::SetSymbolSet(const SymbolPtrVec_t& rSymbolSet)
{
    aSymbolSet = rSymbolSet;

    SetScrollBarRange();
}

void SmShowSymbolSetWindow::SetScrollBarRange()
{
    if (aSymbolSet.size() > static_cast<size_t>(nColumns * nRows))
    {
        m_pVScrollBar->SetRange(Range(0, ((aSymbolSet.size() + (nColumns - 1)) / nColumns) - nRows));
        m_pVScrollBar->Enable(true);
    }
    else
    {
        m_pVScrollBar->SetRange(Range(0,0));
        m_pVScrollBar->Enable (false);
    }

    Invalidate();
}

void SmShowSymbolSetWindow::SelectSymbol(sal_uInt16 nSymbol)
{
    int v = (int) (m_pVScrollBar->GetThumbPos() * nColumns);

    if (nSelectSymbol != SYMBOL_NONE)
        Invalidate(Rectangle(OffsetPoint(Point(((nSelectSymbol - v) % nColumns) * nLen,
                                   ((nSelectSymbol - v) / nColumns) * nLen)),
                             Size(nLen, nLen)));

    if (nSymbol < aSymbolSet.size())
        nSelectSymbol = nSymbol;

    if (aSymbolSet.empty())
        nSelectSymbol = SYMBOL_NONE;

    if (nSelectSymbol != SYMBOL_NONE)
        Invalidate(Rectangle(OffsetPoint(Point(((nSelectSymbol - v) % nColumns) * nLen,
                                   ((nSelectSymbol - v) / nColumns) * nLen)),
                             Size(nLen, nLen)));

    Update();
}

void SmShowSymbolSetWindow::Resize()
{
    Control::Resize();
    calccols();
}

IMPL_LINK( SmShowSymbolSetWindow, ScrollHdl, ScrollBar*, EMPTYARG /*pScrollBar*/)
{
    Invalidate();
    return 0;
}

extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeSmShowSymbol(vcl::Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = 0;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;

    return new SmShowSymbol(pParent, nWinStyle);
}

void SmShowSymbol::Resize()
{
    Control::Resize();
    Invalidate();
}

void SmShowSymbol::setFontSize(vcl::Font &rFont) const
{
    rFont.SetSize(Size(0, GetOutputSize().Height() - GetOutputSize().Height() / 3));
}

void SmShowSymbol::Paint(const Rectangle &rRect)
{
    Control::Paint( rRect );

    vcl::Font aFont(GetFont());
    setFontSize(aFont);
    SetFont(aFont);

    const OUString &rText = GetText();
    Size            aTextSize(GetTextWidth(rText), GetTextHeight());

    DrawText(Point((GetOutputSize().Width()  - aTextSize.Width())  / 2,
                   (GetOutputSize().Height() * 7/10)), rText);
}


void SmShowSymbol::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (rMEvt.GetClicks() > 1)
        aDblClickHdlLink.Call(this);
    else
        Control::MouseButtonDown (rMEvt);
}


void SmShowSymbol::SetSymbol(const SmSym *pSymbol)
{
    if (pSymbol)
    {
        vcl::Font aFont (pSymbol->GetFace());
        setFontSize(aFont);
        aFont.SetAlign(ALIGN_BASELINE);
        SetFont(aFont);

        sal_UCS4 cChar = pSymbol->GetCharacter();
        OUString aText(&cChar, 1);
        SetText( aText );
    }

    // 'Invalidate' fills the background with the background color.
    // If a NULL pointer has been passed that's already enough to clear the display
    Invalidate();
}




void SmSymbolDialog::FillSymbolSets(bool bDeleteText)
    // populate the entries of possible SymbolsSets in the dialog with
    // current values of the SymbolSet manager but selects none of those
{
    m_pSymbolSets->Clear();
    if (bDeleteText)
        m_pSymbolSets->SetNoSelection();

    std::set< OUString >  aSybolSetNames( rSymbolMgr.GetSymbolSetNames() );
    std::set< OUString >::const_iterator aIt( aSybolSetNames.begin() );
    for ( ; aIt != aSybolSetNames.end(); ++aIt)
        m_pSymbolSets->InsertEntry( *aIt );
}


IMPL_LINK_NOARG( SmSymbolDialog, SymbolSetChangeHdl )
{
    SelectSymbolSet(m_pSymbolSets->GetSelectEntry());
    return 0;
}


IMPL_LINK_NOARG( SmSymbolDialog, SymbolChangeHdl )
{
    SelectSymbol(m_pSymbolSetDisplay->GetSelectSymbol());
    return 0;
}

IMPL_LINK_NOARG(SmSymbolDialog, EditClickHdl)
{
    SmSymDefineDialog *pDialog = new SmSymDefineDialog(this, pFontListDev, rSymbolMgr);

    // set current symbol and SymbolSet for the new dialog
    const OUString  aSymSetName (m_pSymbolSets->GetSelectEntry()),
                    aSymName    (m_pSymbolName->GetText());
    pDialog->SelectOldSymbolSet(aSymSetName);
    pDialog->SelectOldSymbol(aSymName);
    pDialog->SelectSymbolSet(aSymSetName);
    pDialog->SelectSymbol(aSymName);

    // remember old SymbolSet
    OUString  aOldSymbolSet (m_pSymbolSets->GetSelectEntry());

    sal_uInt16 nSymPos = GetSelectedSymbol();

    // adapt dialog to data of the SymbolSet manager, which might have changed
    if (pDialog->Execute() == RET_OK  &&  rSymbolMgr.IsModified())
    {
        rSymbolMgr.Save();
        FillSymbolSets();
    }

    // if the old SymbolSet doesn't exist anymore, go to the first one SymbolSet (if one exists)
    if (!SelectSymbolSet(aOldSymbolSet) && m_pSymbolSets->GetEntryCount() > 0)
        SelectSymbolSet(m_pSymbolSets->GetEntry(0));
    else
    {
        // just update display of current symbol set
        assert(aSymSetName == aSymSetName); //unexpected change in symbol set name
        aSymbolSet = rSymbolMgr.GetSymbolSet( aSymbolSetName );
        m_pSymbolSetDisplay->SetSymbolSet( aSymbolSet );
    }

    if (nSymPos >= aSymbolSet.size())
        nSymPos = static_cast< sal_uInt16 >(aSymbolSet.size()) - 1;
    SelectSymbol( nSymPos );

    delete pDialog;
    return 0;
}


IMPL_LINK_NOARG( SmSymbolDialog, SymbolDblClickHdl )
{
    GetClickHdl(m_pGetBtn);
    EndDialog(RET_OK);
    return 0;
}


IMPL_LINK_NOARG( SmSymbolDialog, GetClickHdl )
{
    const SmSym *pSym = GetSymbol();
    if (pSym)
    {
        OUStringBuffer aText;
        aText.append('%').append(pSym->GetName()).append(' ');

        rViewSh.GetViewFrame()->GetDispatcher()->Execute(
                SID_INSERTSYMBOL, SfxCallMode::RECORD,
                new SfxStringItem(SID_INSERTSYMBOL, aText.makeStringAndClear()), 0L);
    }

    return 0;
}


SmSymbolDialog::SmSymbolDialog(vcl::Window *pParent, OutputDevice *pFntListDevice,
        SmSymbolManager &rMgr, SmViewShell &rViewShell)
    : ModalDialog(pParent, "CatalogDialog",
        "modules/smath/ui/catalogdialog.ui")

    ,

    rViewSh             (rViewShell),
    rSymbolMgr          (rMgr),
    pFontListDev        (pFntListDevice)
{
    get(m_pSymbolSets, "symbolset");
    m_pSymbolSets->SetStyle(m_pSymbolSets->GetStyle()|WB_SORT);
    get(m_pSymbolName, "symbolname");
    get(m_pGetBtn, "insert");
    get(m_pEditBtn, "edit");
    get(m_pSymbolSetDisplay, "symbolsetdisplay");
    get(m_pSymbolDisplay, "preview");

    aSymbolSetName.clear();
    aSymbolSet.clear();
    FillSymbolSets();
    if (m_pSymbolSets->GetEntryCount() > 0)
        SelectSymbolSet(m_pSymbolSets->GetEntry(0));

    InitColor_Impl();

    // preview like controls should have a 2D look
    m_pSymbolDisplay->SetBorderStyle( WindowBorderStyle::MONO );

    m_pSymbolSets->SetSelectHdl(LINK(this, SmSymbolDialog, SymbolSetChangeHdl));
    m_pSymbolSetDisplay->SetSelectHdl(LINK(this, SmSymbolDialog, SymbolChangeHdl));
    m_pSymbolSetDisplay->SetDblClickHdl(LINK(this, SmSymbolDialog, SymbolDblClickHdl));
    m_pSymbolDisplay->SetDblClickHdl(LINK(this, SmSymbolDialog, SymbolDblClickHdl));
    m_pEditBtn->SetClickHdl(LINK(this, SmSymbolDialog, EditClickHdl));
    m_pGetBtn->SetClickHdl(LINK(this, SmSymbolDialog, GetClickHdl));
}


void SmSymbolDialog::InitColor_Impl()
{
    ColorData nBgCol, nTxtCol;
    getColors(*this, nBgCol, nTxtCol);

    Color aTmpColor( nBgCol );
    Wallpaper aWall( aTmpColor );
    Color aTxtColor( nTxtCol );
    m_pSymbolDisplay->SetBackground( aWall );
    m_pSymbolDisplay->SetTextColor( aTxtColor );
    m_pSymbolSetDisplay->SetBackground( aWall );
    m_pSymbolSetDisplay->SetTextColor( aTxtColor );
}


void SmSymbolDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DATACHANGED_SETTINGS  &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            InitColor_Impl();

    ModalDialog::DataChanged( rDCEvt );
}


bool SmSymbolDialog::SelectSymbolSet(const OUString &rSymbolSetName)
{
    bool    bRet = false;
    sal_Int32  nPos = m_pSymbolSets->GetEntryPos(rSymbolSetName);

    aSymbolSetName.clear();
    aSymbolSet.clear();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        m_pSymbolSets->SelectEntryPos(nPos);

        aSymbolSetName  = rSymbolSetName;
        aSymbolSet      = rSymbolMgr.GetSymbolSet( aSymbolSetName );

        // sort symbols by Unicode position (useful for displaying Greek characters alphabetically)
        std::sort( aSymbolSet.begin(), aSymbolSet.end(), lt_SmSymPtr() );

        m_pSymbolSetDisplay->SetSymbolSet( aSymbolSet );
        if (aSymbolSet.size() > 0)
            SelectSymbol(0);

        bRet = true;
    }
    else
        m_pSymbolSets->SetNoSelection();

    return bRet;
}


void SmSymbolDialog::SelectSymbol(sal_uInt16 nSymbolNo)
{
    const SmSym *pSym = NULL;
    if (!aSymbolSetName.isEmpty()  &&  nSymbolNo < static_cast< sal_uInt16 >(aSymbolSet.size()))
        pSym = aSymbolSet[ nSymbolNo ];

    m_pSymbolSetDisplay->SelectSymbol(nSymbolNo);
    m_pSymbolDisplay->SetSymbol(pSym);
    m_pSymbolName->SetText(pSym ? pSym->GetName() : OUString());
}


const SmSym * SmSymbolDialog::GetSymbol() const
{
    sal_uInt16 nSymbolNo = m_pSymbolSetDisplay->GetSelectSymbol();
    bool bValid = !aSymbolSetName.isEmpty()  &&  nSymbolNo < static_cast< sal_uInt16 >(aSymbolSet.size());
    return bValid ? aSymbolSet[ nSymbolNo ] : NULL;
}



extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeSmShowChar(vcl::Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = 0;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;

    return new SmShowChar(pParent, nWinStyle);
}

void SmShowChar::Paint(const Rectangle &rRect)
{
    Control::Paint( rRect );

    OUString aText( GetText() );
    if (!aText.isEmpty())
    {
#if OSL_DEBUG_LEVEL > 1
        sal_Int32 nPos = 0;
        sal_UCS4 cChar = aText.iterateCodePoints( &nPos );
        (void) cChar;
#endif
        Size aTextSize(GetTextWidth(aText), GetTextHeight());

        DrawText(Point((GetOutputSize().Width()  - aTextSize.Width())  / 2,
                       (GetOutputSize().Height() * 7/10)), aText);
    }
}


void SmShowChar::SetSymbol( const SmSym *pSym )
{
    if (pSym)
        SetSymbol( pSym->GetCharacter(), pSym->GetFace() );
}


void SmShowChar::SetSymbol( sal_UCS4 cChar, const vcl::Font &rFont )
{
    vcl::Font aFont( rFont );
    aFont.SetSize( Size(0, GetOutputSize().Height() - GetOutputSize().Height() / 3) );
    aFont.SetAlign(ALIGN_BASELINE);
    SetFont(aFont);
    aFont.SetTransparent(true);

    OUString aText(&cChar, 1);
    SetText( aText );

    Invalidate();
}

void SmShowChar::Resize()
{
    Control::Resize();
    const OUString &rText = GetText();
    if (rText.isEmpty())
        return;
    sal_Int32 nStrIndex = 0;
    sal_UCS4 cChar = rText.iterateCodePoints(&nStrIndex);
    SetSymbol(cChar, GetFont()); //force recalculation of size
}

void SmSymDefineDialog::FillSymbols(ComboBox &rComboBox, bool bDeleteText)
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(&rComboBox == pOldSymbols  ||  &rComboBox == pSymbols,
        "Sm : wrong ComboBox");
#endif

    rComboBox.Clear();
    if (bDeleteText)
        rComboBox.SetText(OUString());

    ComboBox &rBox = &rComboBox == pOldSymbols ? *pOldSymbolSets : *pSymbolSets;
    SymbolPtrVec_t aSymSet( aSymbolMgrCopy.GetSymbolSet( rBox.GetText() ) );
    for (size_t i = 0;  i < aSymSet.size();  ++i)
        rComboBox.InsertEntry( aSymSet[i]->GetName() );
}


void SmSymDefineDialog::FillSymbolSets(ComboBox &rComboBox, bool bDeleteText)
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(&rComboBox == pOldSymbolSets  ||  &rComboBox == pSymbolSets,
        "Sm : falsche ComboBox");
#endif

    rComboBox.Clear();
    if (bDeleteText)
        rComboBox.SetText(OUString());

    const std::set< OUString >  aSymbolSetNames( aSymbolMgrCopy.GetSymbolSetNames() );
    std::set< OUString >::const_iterator aIt( aSymbolSetNames.begin() );
    for ( ;  aIt != aSymbolSetNames.end();  ++aIt)
        rComboBox.InsertEntry( *aIt );
}


void SmSymDefineDialog::FillFonts(bool bDelete)
{
    pFonts->Clear();
    if (bDelete)
        pFonts->SetNoSelection();

    // Include all fonts of FontList into the font list.
    // If there are duplicates, only include one entry of each font since the style will be
    // already selected using the FontStyleBox.
    if (pFontList)
    {
        sal_uInt16  nCount = pFontList->GetFontNameCount();
        for (sal_uInt16 i = 0;  i < nCount;  i++)
            pFonts->InsertEntry( pFontList->GetFontName(i).GetName() );
    }
}


void SmSymDefineDialog::FillStyles(bool bDeleteText)
{
    pStyles->Clear();
    if (bDeleteText)
        pStyles->SetText(OUString());

    OUString aText (pFonts->GetSelectEntry());
    if (!aText.isEmpty())
    {
        // use own StyleNames
        const SmFontStyles &rStyles = GetFontStyles();
        for (sal_uInt16 i = 0;  i < rStyles.GetCount();  i++)
            pStyles->InsertEntry( rStyles.GetStyleName(i) );

#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE(pStyles->GetEntryCount() > 0, "Sm : no styles available");
#endif
        pStyles->SetText( pStyles->GetEntry(0) );
    }
}


SmSym * SmSymDefineDialog::GetSymbol(const ComboBox &rComboBox)
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(&rComboBox == pOldSymbols  ||  &rComboBox == pSymbols,
        "Sm : wrong combobox");
#endif
    return aSymbolMgrCopy.GetSymbolByName(rComboBox.GetText());
}


IMPL_LINK( SmSymDefineDialog, OldSymbolChangeHdl, ComboBox *, EMPTYARG pComboBox )
{
    (void) pComboBox;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pComboBox == pOldSymbols, "Sm : wrong argument");
#endif
    SelectSymbol(*pOldSymbols, pOldSymbols->GetText(), false);
    return 0;
}


IMPL_LINK( SmSymDefineDialog, OldSymbolSetChangeHdl, ComboBox *, EMPTYARG pComboBox )
{
    (void) pComboBox;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pComboBox == pOldSymbolSets, "Sm : wrong argument");
#endif
    SelectSymbolSet(*pOldSymbolSets, pOldSymbolSets->GetText(), false);
    return 0;
}


IMPL_LINK( SmSymDefineDialog, ModifyHdl, ComboBox *, pComboBox )
{
    // remember cursor position for later restoring of it
    Selection  aSelection (pComboBox->GetSelection());

    if (pComboBox == pSymbols)
        SelectSymbol(*pSymbols, pSymbols->GetText(), false);
    else if (pComboBox == pSymbolSets)
        SelectSymbolSet(*pSymbolSets, pSymbolSets->GetText(), false);
    else if (pComboBox == pOldSymbols)
        // allow only names from the list
        SelectSymbol(*pOldSymbols, pOldSymbols->GetText(), true);
    else if (pComboBox == pOldSymbolSets)
        // allow only names from the list
        SelectSymbolSet(*pOldSymbolSets, pOldSymbolSets->GetText(), true);
    else if (pComboBox == pStyles)
        // allow only names from the list (that's the case here anyway)
        SelectStyle(pStyles->GetText(), true);
    else
        SAL_WARN("starmath", "wrong combobox argument");

    pComboBox->SetSelection(aSelection);

    UpdateButtons();

    return 0;
}


IMPL_LINK( SmSymDefineDialog, FontChangeHdl, ListBox *, EMPTYARG pListBox )
{
    (void) pListBox;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pListBox == pFonts, "Sm : wrong argument");
#endif

    SelectFont(pFonts->GetSelectEntry());
    return 0;
}


IMPL_LINK( SmSymDefineDialog, SubsetChangeHdl, ListBox *, EMPTYARG pListBox )
{
    (void) pListBox;
    sal_Int32 nPos = pFontsSubsetLB->GetSelectEntryPos();
    if (LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        const Subset* pSubset = reinterpret_cast<const Subset*> (pFontsSubsetLB->GetEntryData( nPos ));
        if (pSubset)
        {
            pCharsetDisplay->SelectCharacter( pSubset->GetRangeMin() );
        }
    }
    return 0;
}


IMPL_LINK( SmSymDefineDialog, StyleChangeHdl, ComboBox *, EMPTYARG pComboBox )
{
    (void) pComboBox;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pComboBox == pStyles, "Sm : falsches Argument");
#endif

    SelectStyle(pStyles->GetText());
    return 0;
}


IMPL_LINK_NOARG(SmSymDefineDialog, CharHighlightHdl)
{
   sal_UCS4 cChar = pCharsetDisplay->GetSelectCharacter();

#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE( pSubsetMap, "SubsetMap missing" );
#endif
    if (pSubsetMap)
    {
        const Subset* pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if (pSubset)
            pFontsSubsetLB->SelectEntry( pSubset->GetName() );
        else
            pFontsSubsetLB->SetNoSelection();
    }

    pSymbolDisplay->SetSymbol( cChar, pCharsetDisplay->GetFont() );

    UpdateButtons();

    // display Unicode position as symbol name while iterating over characters
    const OUString aHex(OUString::number(cChar, 16 ).toAsciiUpperCase());
    const OUString aPattern( (aHex.getLength() > 4) ? OUString("Ux000000") : OUString("Ux0000") );
    OUString aUnicodePos( aPattern.copy( 0, aPattern.getLength() - aHex.getLength() ) );
    aUnicodePos += aHex;
    pSymbols->SetText( aUnicodePos );
    pSymbolName->SetText( aUnicodePos );

    return 0;
}


IMPL_LINK( SmSymDefineDialog, AddClickHdl, Button *, EMPTYARG pButton )
{
    (void) pButton;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pButton == pAddBtn, "Sm : wrong argument");
    OSL_ENSURE(pAddBtn->IsEnabled(), "Sm : requirements met ??");
#endif

    // add symbol
    const SmSym aNewSymbol( pSymbols->GetText(), pCharsetDisplay->GetFont(),
            pCharsetDisplay->GetSelectCharacter(), pSymbolSets->GetText() );
    //OSL_ENSURE( aSymbolMgrCopy.GetSymbolByName(aTmpSymbolName) == NULL, "symbol already exists" );
    aSymbolMgrCopy.AddOrReplaceSymbol( aNewSymbol );

    // update display of new symbol
    pSymbolDisplay->SetSymbol( &aNewSymbol );
    pSymbolName->SetText( aNewSymbol.GetName() );
    pSymbolSetName->SetText( aNewSymbol.GetSymbolSetName() );

    // update list box entries
    FillSymbolSets(*pOldSymbolSets, false);
    FillSymbolSets(*pSymbolSets,    false);
    FillSymbols(*pOldSymbols ,false);
    FillSymbols(*pSymbols    ,false);

    UpdateButtons();

    return 0;
}


IMPL_LINK( SmSymDefineDialog, ChangeClickHdl, Button *, EMPTYARG pButton )
{
    (void) pButton;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pButton == pChangeBtn, "Sm : wrong argument");
    OSL_ENSURE(pChangeBtn->IsEnabled(), "Sm : requirements met ??");
#endif

    // get new Sybol to use
    //! get font from symbol-disp lay since charset-display does not keep
    //! the bold attribute.
    const SmSym aNewSymbol( pSymbols->GetText(), pCharsetDisplay->GetFont(),
            pCharsetDisplay->GetSelectCharacter(), pSymbolSets->GetText() );

    // remove old symbol if the name was changed then add new one
    const bool bNameChanged       = pOldSymbols->GetText() != pSymbols->GetText();
    if (bNameChanged)
        aSymbolMgrCopy.RemoveSymbol( pOldSymbols->GetText() );
    aSymbolMgrCopy.AddOrReplaceSymbol( aNewSymbol, true );

    // clear display for original symbol if necessary
    if (bNameChanged)
        SetOrigSymbol(NULL, OUString());

    // update display of new symbol
    pSymbolDisplay->SetSymbol( &aNewSymbol );
    pSymbolName->SetText( aNewSymbol.GetName() );
    pSymbolSetName->SetText( aNewSymbol.GetSymbolSetName() );

    // update list box entries
    FillSymbolSets(*pOldSymbolSets, false);
    FillSymbolSets(*pSymbolSets,    false);
    FillSymbols(*pOldSymbols ,false);
    FillSymbols(*pSymbols    ,false);

    UpdateButtons();

    return 0;
}


IMPL_LINK( SmSymDefineDialog, DeleteClickHdl, Button *, EMPTYARG pButton )
{
    (void) pButton;
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(pButton == pDeleteBtn, "Sm : wrong argument");
    OSL_ENSURE(pDeleteBtn->IsEnabled(), "Sm : requirements met ??");
#endif

    if (pOrigSymbol)
    {
        aSymbolMgrCopy.RemoveSymbol( pOrigSymbol->GetName() );

        // clear display for original symbol
        SetOrigSymbol(NULL, OUString());

        // update list box entries
        FillSymbolSets(*pOldSymbolSets, false);
        FillSymbolSets(*pSymbolSets,    false);
        FillSymbols(*pOldSymbols ,false);
        FillSymbols(*pSymbols    ,false);
    }

    UpdateButtons();

    return 0;
}


void SmSymDefineDialog::UpdateButtons()
{
    bool  bAdd    = false,
          bChange = false,
          bDelete = false;
    OUString aTmpSymbolName    (pSymbols->GetText()),
              aTmpSymbolSetName (pSymbolSets->GetText());

    if (aTmpSymbolName.getLength() > 0  &&  aTmpSymbolSetName.getLength() > 0)
    {
        // are all settings equal?
        //! (Font-, Style- und SymbolSet name comparison is not case sensitive)
        bool bEqual = pOrigSymbol
                    && aTmpSymbolSetName.equalsIgnoreAsciiCase(pOldSymbolSetName->GetText())
                    && aTmpSymbolName.equals(pOrigSymbol->GetName())
                    && pFonts->GetSelectEntry().equalsIgnoreAsciiCase(
                            pOrigSymbol->GetFace().GetName())
                    && pStyles->GetText().equalsIgnoreAsciiCase(
                            GetFontStyles().GetStyleName(pOrigSymbol->GetFace()))
                    && pCharsetDisplay->GetSelectCharacter() == pOrigSymbol->GetCharacter();

        // only add it if there isn't already a symbol with the same name
        bAdd    = aSymbolMgrCopy.GetSymbolByName(aTmpSymbolName) == NULL;

        // only delete it if all settings are equal
        bDelete = pOrigSymbol != NULL;

        // only change it if the old symbol exists and the new one is different
        bChange = pOrigSymbol && !bEqual;
    }

    pAddBtn   ->Enable(bAdd);
    pChangeBtn->Enable(bChange);
    pDeleteBtn->Enable(bDelete);
}

SmSymDefineDialog::SmSymDefineDialog(vcl::Window * pParent,
        OutputDevice *pFntListDevice, SmSymbolManager &rMgr) :
    ModalDialog         (pParent, "EditSymbols", "modules/smath/ui/symdefinedialog.ui"),
    rSymbolMgr          (rMgr),
    pSubsetMap          (NULL),
    pFontList           (NULL)
{
    get(pOldSymbols, "oldSymbols");
    get(pOldSymbolSets, "oldSymbolSets");
    get(pCharsetDisplay, "charsetDisplay");
    get(pSymbols, "symbols");
    get(pSymbolSets, "symbolSets");
    get(pFonts, "fonts");
    get(pFontsSubsetLB, "fontsSubsetLB");
    get(pStyles, "styles");
    get(pOldSymbolName, "oldSymbolName");
    get(pOldSymbolDisplay, "oldSymbolDisplay");
    get(pOldSymbolSetName, "oldSymbolSetName");
    get(pSymbolName, "symbolName");
    get(pSymbolDisplay, "symbolDisplay");
    get(pSymbolSetName, "symbolSetName");
    get(pAddBtn, "add");
    get(pChangeBtn, "modify");
    get(pDeleteBtn, "delete");

    pFontList = new FontList( pFntListDevice );

    pOrigSymbol = 0;

    // auto completion is troublesome since that symbols character also gets automatically selected in the
    // display and if the user previously selected a character to define/redefine that one this is bad
   pOldSymbols->EnableAutocomplete( false, true );
   pSymbols->EnableAutocomplete( false, true );

    FillFonts();
    if (pFonts->GetEntryCount() > 0)
        SelectFont(pFonts->GetEntry(0));

    InitColor_Impl();

    SetSymbolSetManager(rSymbolMgr);

    pOldSymbols     ->SetSelectHdl(LINK(this, SmSymDefineDialog, OldSymbolChangeHdl));
    pOldSymbolSets  ->SetSelectHdl(LINK(this, SmSymDefineDialog, OldSymbolSetChangeHdl));
    pSymbolSets     ->SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    pOldSymbolSets  ->SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    pSymbols        ->SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    pOldSymbols     ->SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    pStyles         ->SetModifyHdl(LINK(this, SmSymDefineDialog, ModifyHdl));
    pFonts          ->SetSelectHdl(LINK(this, SmSymDefineDialog, FontChangeHdl));
    pFontsSubsetLB  ->SetSelectHdl(LINK(this, SmSymDefineDialog, SubsetChangeHdl));
    pStyles         ->SetSelectHdl(LINK(this, SmSymDefineDialog, StyleChangeHdl));
    pAddBtn         ->SetClickHdl (LINK(this, SmSymDefineDialog, AddClickHdl));
    pChangeBtn      ->SetClickHdl (LINK(this, SmSymDefineDialog, ChangeClickHdl));
    pDeleteBtn      ->SetClickHdl (LINK(this, SmSymDefineDialog, DeleteClickHdl));
    pCharsetDisplay ->SetHighlightHdl( LINK( this, SmSymDefineDialog, CharHighlightHdl ) );

    // preview like controls should have a 2D look
    pOldSymbolDisplay->SetBorderStyle( WindowBorderStyle::MONO );
    pSymbolDisplay   ->SetBorderStyle( WindowBorderStyle::MONO );
}


SmSymDefineDialog::~SmSymDefineDialog()
{
    delete pSubsetMap;
    delete pOrigSymbol;
}

void SmSymDefineDialog::InitColor_Impl()
{
#if OSL_DEBUG_LEVEL > 1
    Color aBC( GetDisplayBackground().GetColor() );
#endif
    ColorData   nBgCol  = COL_WHITE,
                nTxtCol = COL_BLACK;
    bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
    if (bHighContrast)
    {
        const StyleSettings &rS = GetSettings().GetStyleSettings();
        nBgCol  = rS.GetFieldColor().GetColor();
        nTxtCol = rS.GetFieldTextColor().GetColor();
    }

    Color aTmpColor( nBgCol );
    Wallpaper aWall( aTmpColor );
    Color aTxtColor( nTxtCol );
    pCharsetDisplay  ->SetBackground( aWall );
    pCharsetDisplay  ->SetTextColor( aTxtColor );
    pOldSymbolDisplay->SetBackground( aWall );
    pOldSymbolDisplay->SetTextColor( aTxtColor );
    pSymbolDisplay   ->SetBackground( aWall );
    pSymbolDisplay   ->SetTextColor( aTxtColor );
}


void SmSymDefineDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DATACHANGED_SETTINGS  &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            InitColor_Impl();

    ModalDialog::DataChanged( rDCEvt );
}


short SmSymDefineDialog::Execute()
{
    short nResult = ModalDialog::Execute();

    // apply changes if dialog was closed by clicking OK
    if (aSymbolMgrCopy.IsModified()  &&  nResult == RET_OK)
        rSymbolMgr = aSymbolMgrCopy;

    return nResult;
}


void SmSymDefineDialog::SetSymbolSetManager(const SmSymbolManager &rMgr)
{
    aSymbolMgrCopy = rMgr;

    // Set the modified flag of the copy to false so that
    // we can check later on if anything has been changed
    aSymbolMgrCopy.SetModified(false);

    FillSymbolSets(*pOldSymbolSets);
    if (pOldSymbolSets->GetEntryCount() > 0)
        SelectSymbolSet(pOldSymbolSets->GetEntry(0));
    FillSymbolSets(*pSymbolSets);
    if (pSymbolSets->GetEntryCount() > 0)
        SelectSymbolSet(pSymbolSets->GetEntry(0));
    FillSymbols(*pOldSymbols);
    if (pOldSymbols->GetEntryCount() > 0)
        SelectSymbol(pOldSymbols->GetEntry(0));
    FillSymbols(*pSymbols);
    if (pSymbols->GetEntryCount() > 0)
        SelectSymbol(pSymbols->GetEntry(0));

    UpdateButtons();
}


bool SmSymDefineDialog::SelectSymbolSet(ComboBox &rComboBox,
        const OUString &rSymbolSetName, bool bDeleteText)
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(&rComboBox == pOldSymbolSets  ||  &rComboBox == pSymbolSets,
        "Sm : wrong ComboBox");
#endif

    // trim SymbolName (no leading and trailing blanks)
    OUString  aNormName (rSymbolSetName);
    aNormName = comphelper::string::stripStart(aNormName, ' ');
    aNormName = comphelper::string::stripEnd(aNormName, ' ');
    // and remove possible deviations within the input
    rComboBox.SetText(aNormName);

    bool   bRet = false;
    sal_Int32 nPos = rComboBox.GetEntryPos(aNormName);

    if (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        rComboBox.SetText(rComboBox.GetEntry(nPos));
        bRet = true;
    }
    else if (bDeleteText)
        rComboBox.SetText(OUString());

    bool  bIsOld = &rComboBox == pOldSymbolSets;

    // setting the SymbolSet name at the associated display
    FixedText &rFT = bIsOld ? *pOldSymbolSetName : *pSymbolSetName;
    rFT.SetText(rComboBox.GetText());

    // set the symbol name which belongs to the SymbolSet at the associated combobox
    ComboBox  &rCB = bIsOld ? *pOldSymbols : *pSymbols;
    FillSymbols(rCB, false);

    // display a valid respectively no symbol when changing the SymbolSets
    if (bIsOld)
    {
        OUString  aTmpOldSymbolName;
        if (pOldSymbols->GetEntryCount() > 0)
            aTmpOldSymbolName = pOldSymbols->GetEntry(0);
        SelectSymbol(*pOldSymbols, aTmpOldSymbolName, true);
    }

    UpdateButtons();

    return bRet;
}


void SmSymDefineDialog::SetOrigSymbol(const SmSym *pSymbol,
                                      const OUString &rSymbolSetName)
{
    // clear old symbol
    delete pOrigSymbol;
    pOrigSymbol = 0;

    OUString   aSymName,
                aSymSetName;
    if (pSymbol)
    {
        // set new symbol
        pOrigSymbol = new SmSym( *pSymbol );

        aSymName    = pSymbol->GetName();
        aSymSetName = rSymbolSetName;
        pOldSymbolDisplay->SetSymbol( pSymbol );
    }
    else
    {   // delete displayed symbols
        pOldSymbolDisplay->SetText(OUString());
        pOldSymbolDisplay->Invalidate();
    }
    pOldSymbolName->SetText(aSymName);
    pOldSymbolSetName->SetText(aSymSetName);
}


bool SmSymDefineDialog::SelectSymbol(ComboBox &rComboBox,
        const OUString &rSymbolName, bool bDeleteText)
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE(&rComboBox == pOldSymbols  ||  &rComboBox == pSymbols,
        "Sm : wrong ComboBox");
#endif

    // trim SymbolName (no blanks)
    OUString  aNormName(comphelper::string::remove(rSymbolName, ' '));
    // and remove possible deviations within the input
    rComboBox.SetText(aNormName);

    bool   bRet = false;
    sal_Int32 nPos = rComboBox.GetEntryPos(aNormName);

    bool  bIsOld = &rComboBox == pOldSymbols;

    if (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        rComboBox.SetText(rComboBox.GetEntry(nPos));

        if (!bIsOld)
        {
            const SmSym *pSymbol = GetSymbol(*pSymbols);
            if (pSymbol)
            {
                // choose font and style accordingly
                const vcl::Font &rFont = pSymbol->GetFace();
                SelectFont(rFont.GetName(), false);
                SelectStyle(GetFontStyles().GetStyleName(rFont), false);

                // Since setting the Font via the Style name of the SymbolFonts doesn't
                // work really well (e.g. it can be empty even though the font itself is
                // bold or italic) we're manually setting the Font with respect to the Symbol
                pCharsetDisplay->SetFont(rFont);
                pSymbolDisplay->SetFont(rFont);

                // select associated character
                SelectChar(pSymbol->GetCharacter());

                // since SelectChar will also set the unicode point as text in the
                // symbols box, we have to set the symbol name again to get that one displayed
                pSymbols->SetText( pSymbol->GetName() );
            }
        }

        bRet = true;
    }
    else if (bDeleteText)
        rComboBox.SetText(OUString());

    if (bIsOld)
    {
        // if there's a change of the old symbol, show only the available ones, otherwise show none
        const SmSym *pOldSymbol = NULL;
        OUString     aTmpOldSymbolSetName;
        if (nPos != COMBOBOX_ENTRY_NOTFOUND)
        {
            pOldSymbol        = aSymbolMgrCopy.GetSymbolByName(aNormName);
            aTmpOldSymbolSetName = pOldSymbolSets->GetText();
        }
        SetOrigSymbol(pOldSymbol, aTmpOldSymbolSetName);
    }
    else
        pSymbolName->SetText(rComboBox.GetText());

    UpdateButtons();

    return bRet;
}


void SmSymDefineDialog::SetFont(const OUString &rFontName, const OUString &rStyleName)
{
    // get Font (FontInfo) matching name and style
    vcl::FontInfo aFI;
    if (pFontList)
        aFI = pFontList->Get(rFontName, WEIGHT_NORMAL, ITALIC_NONE);
    SetFontStyle(rStyleName, aFI);

    pCharsetDisplay->SetFont(aFI);
    pSymbolDisplay->SetFont(aFI);

    // update subset listbox for new font's unicode subsets
    FontCharMapPtr pFontCharMap;
    pCharsetDisplay->GetFontCharMap( pFontCharMap );
    if (pSubsetMap)
        delete pSubsetMap;
    pSubsetMap = new SubsetMap( pFontCharMap );

    pFontsSubsetLB->Clear();
    bool bFirst = true;
    const Subset* pSubset;
    while( NULL != (pSubset = pSubsetMap->GetNextSubset( bFirst )) )
    {
        sal_uInt16 nPos = pFontsSubsetLB->InsertEntry( pSubset->GetName());
        pFontsSubsetLB->SetEntryData( nPos, (void *) pSubset );
        // subset must live at least as long as the selected font !!!
        if( bFirst )
            pFontsSubsetLB->SelectEntryPos( nPos );
        bFirst = false;
    }
    if( bFirst )
        pFontsSubsetLB->SetNoSelection();
    pFontsSubsetLB->Enable( !bFirst );

    pFontCharMap = 0;
}


bool SmSymDefineDialog::SelectFont(const OUString &rFontName, bool bApplyFont)
{
    bool   bRet = false;
    sal_Int32 nPos = pFonts->GetEntryPos(rFontName);

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        pFonts->SelectEntryPos(nPos);
        if (pStyles->GetEntryCount() > 0)
            SelectStyle(pStyles->GetEntry(0));
        if (bApplyFont)
        {
            SetFont(pFonts->GetSelectEntry(), pStyles->GetText());
        bRet = true;
            pSymbolDisplay->SetSymbol( pCharsetDisplay->GetSelectCharacter(), pCharsetDisplay->GetFont() );
        }
        bRet = true;
    }
    else
        pFonts->SetNoSelection();
    FillStyles();

    UpdateButtons();

    return bRet;
}


bool SmSymDefineDialog::SelectStyle(const OUString &rStyleName, bool bApplyFont)
{
    bool   bRet = false;
    sal_Int32 nPos = pStyles->GetEntryPos(rStyleName);

    // if the style is not available take the first available one (if existent)
    if (nPos == COMBOBOX_ENTRY_NOTFOUND  &&  pStyles->GetEntryCount() > 0)
        nPos = 0;

    if (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        pStyles->SetText(pStyles->GetEntry(nPos));
        if (bApplyFont)
        {
            SetFont(pFonts->GetSelectEntry(), pStyles->GetText());
        bRet = true;
            pSymbolDisplay->SetSymbol( pCharsetDisplay->GetSelectCharacter(), pCharsetDisplay->GetFont() );
        }
        bRet = true;
    }
    else
        pStyles->SetText(OUString());

    UpdateButtons();

    return bRet;
}


void SmSymDefineDialog::SelectChar(sal_Unicode cChar)
{
    pCharsetDisplay->SelectCharacter( cChar );
    pSymbolDisplay->SetSymbol( cChar, pCharsetDisplay->GetFont() );

    UpdateButtons();
}


/**************************************************************************/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
