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

#include <sal/config.h>
#include <sal/log.hxx>

#include <cassert>

#include <comphelper/string.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <vcl/combobox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <svtools/ctrltool.hxx>
#include <vcl/settings.hxx>
#include <vcl/wall.hxx>
#include <vcl/fontcharmap.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/charmap.hxx>
#include <svx/ucsubset.hxx>

#include <dialog.hxx>
#include <starmath.hrc>
#include <strings.hrc>
#include <helpids.h>
#include "cfgitem.hxx"
#include <smmod.hxx>
#include <symbol.hxx>
#include <view.hxx>

#include <algorithm>

namespace
{

void lclGetSettingColors(Color& rBackgroundColor, Color& rTextColor)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    if (rStyleSettings.GetHighContrastMode())
    {
        rBackgroundColor = rStyleSettings.GetFieldColor();
        rTextColor = rStyleSettings.GetFieldTextColor();
    }
    else
    {
        rBackgroundColor = COL_WHITE;
        rTextColor = COL_BLACK;
    }
}

} // end anonymous namespace

// Since it's better to set/query the FontStyle via its attributes rather
// than via the StyleName we create a way to translate
// Attribute <-> StyleName

class SmFontStyles
{
    OUString const aNormal;
    OUString const aBold;
    OUString const aItalic;
    OUString aBoldItalic;

public:
    SmFontStyles();

    static sal_uInt16 GetCount() { return 4; }
    const OUString& GetStyleName(const vcl::Font& rFont) const;
    const OUString& GetStyleName(sal_uInt16 nIdx) const;
};

SmFontStyles::SmFontStyles()
    : aNormal(SmResId(RID_FONTREGULAR))
    , aBold(SmResId(RID_FONTBOLD))
    , aItalic(SmResId(RID_FONTITALIC))
{
    aBoldItalic = aBold;
    aBoldItalic += ", ";
    aBoldItalic += aItalic;
}

const OUString& SmFontStyles::GetStyleName(const vcl::Font& rFont) const
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

    assert( nIdx < GetCount() );
    switch (nIdx)
    {
        case 0 : return aNormal;
        case 1 : return aItalic;
        case 2 : return aBold;
        default: /*case 3:*/ return aBoldItalic;
    }
}

static const SmFontStyles & GetFontStyles()
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
        for (i = 0;  i < SmFontStyles::GetCount(); ++i)
            if (rStyleName == rStyles.GetStyleName(i))
                break;
        assert(i < SmFontStyles::GetCount() && "style-name unknown");
        nIndex = i;
    }

    rFont.SetItalic((nIndex & 0x1) ? ITALIC_NORMAL : ITALIC_NONE);
    rFont.SetWeight((nIndex & 0x2) ? WEIGHT_BOLD : WEIGHT_NORMAL);
}

IMPL_LINK_NOARG(SmPrintOptionsTabPage, SizeButtonClickHdl, weld::ToggleButton&, void)
{
    m_xZoom->set_sensitive(m_xSizeZoomed->get_active());
}

SmPrintOptionsTabPage::SmPrintOptionsTabPage(TabPageParent pPage, const SfxItemSet& rOptions)
    : SfxTabPage(pPage, "modules/smath/ui/smathsettings.ui", "SmathSettings", &rOptions)
    , m_xTitle(m_xBuilder->weld_check_button("title"))
    , m_xText(m_xBuilder->weld_check_button("text"))
    , m_xFrame(m_xBuilder->weld_check_button("frame"))
    , m_xSizeNormal(m_xBuilder->weld_radio_button("sizenormal"))
    , m_xSizeScaled(m_xBuilder->weld_radio_button("sizescaled"))
    , m_xSizeZoomed(m_xBuilder->weld_radio_button("sizezoomed"))
    , m_xZoom(m_xBuilder->weld_metric_spin_button("zoom", FieldUnit::PERCENT))
    , m_xNoRightSpaces(m_xBuilder->weld_check_button("norightspaces"))
    , m_xSaveOnlyUsedSymbols(m_xBuilder->weld_check_button("saveonlyusedsymbols"))
    , m_xAutoCloseBrackets(m_xBuilder->weld_check_button("autoclosebrackets"))
{
    m_xSizeNormal->connect_toggled(LINK(this, SmPrintOptionsTabPage, SizeButtonClickHdl));
    m_xSizeScaled->connect_toggled(LINK(this, SmPrintOptionsTabPage, SizeButtonClickHdl));
    m_xSizeZoomed->connect_toggled(LINK(this, SmPrintOptionsTabPage, SizeButtonClickHdl));

    Reset(&rOptions);
}

SmPrintOptionsTabPage::~SmPrintOptionsTabPage()
{
}

bool SmPrintOptionsTabPage::FillItemSet(SfxItemSet* rSet)
{
    sal_uInt16  nPrintSize;
    if (m_xSizeNormal->get_active())
        nPrintSize = PRINT_SIZE_NORMAL;
    else if (m_xSizeScaled->get_active())
        nPrintSize = PRINT_SIZE_SCALED;
    else
        nPrintSize = PRINT_SIZE_ZOOMED;

    rSet->Put(SfxUInt16Item(GetWhich(SID_PRINTSIZE), nPrintSize));
    rSet->Put(SfxUInt16Item(GetWhich(SID_PRINTZOOM), sal::static_int_cast<sal_uInt16>(m_xZoom->get_value(FieldUnit::PERCENT))));
    rSet->Put(SfxBoolItem(GetWhich(SID_PRINTTITLE), m_xTitle->get_active()));
    rSet->Put(SfxBoolItem(GetWhich(SID_PRINTTEXT), m_xText->get_active()));
    rSet->Put(SfxBoolItem(GetWhich(SID_PRINTFRAME), m_xFrame->get_active()));
    rSet->Put(SfxBoolItem(GetWhich(SID_NO_RIGHT_SPACES), m_xNoRightSpaces->get_active()));
    rSet->Put(SfxBoolItem(GetWhich(SID_SAVE_ONLY_USED_SYMBOLS), m_xSaveOnlyUsedSymbols->get_active()));
    rSet->Put(SfxBoolItem(GetWhich(SID_AUTO_CLOSE_BRACKETS), m_xAutoCloseBrackets->get_active()));

    return true;
}

void SmPrintOptionsTabPage::Reset(const SfxItemSet* rSet)
{
    SmPrintSize ePrintSize = static_cast<SmPrintSize>(static_cast<const SfxUInt16Item &>(rSet->Get(GetWhich(SID_PRINTSIZE))).GetValue());

    m_xSizeNormal->set_active(ePrintSize == PRINT_SIZE_NORMAL);
    m_xSizeScaled->set_active(ePrintSize == PRINT_SIZE_SCALED);
    m_xSizeZoomed->set_active(ePrintSize == PRINT_SIZE_ZOOMED);

    m_xZoom->set_sensitive(m_xSizeZoomed->get_active());

    m_xZoom->set_value(static_cast<const SfxUInt16Item &>(rSet->Get(GetWhich(SID_PRINTZOOM))).GetValue(), FieldUnit::PERCENT);

    m_xTitle->set_active(static_cast<const SfxBoolItem &>(rSet->Get(GetWhich(SID_PRINTTITLE))).GetValue());
    m_xNoRightSpaces->set_active(static_cast<const SfxBoolItem &>(rSet->Get(GetWhich(SID_NO_RIGHT_SPACES))).GetValue());
    m_xSaveOnlyUsedSymbols->set_active(static_cast<const SfxBoolItem &>(rSet->Get(GetWhich(SID_SAVE_ONLY_USED_SYMBOLS))).GetValue());
    m_xAutoCloseBrackets->set_active(static_cast<const SfxBoolItem &>(rSet->Get(GetWhich(SID_AUTO_CLOSE_BRACKETS))).GetValue());
}

VclPtr<SfxTabPage> SmPrintOptionsTabPage::Create(TabPageParent pParent, const SfxItemSet& rSet)
{
    return VclPtr<SmPrintOptionsTabPage>::Create(pParent, rSet).get();
}

void SmShowFont::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    Color aBackColor;
    Color aTextColor;
    lclGetSettingColors(aBackColor, aTextColor);

    rRenderContext.SetBackground(Wallpaper(aBackColor));

    vcl::Font aFont(maFont);
    aFont.SetFontSize(Size(0, 24 * rRenderContext.GetDPIScaleFactor()));
    aFont.SetAlignment(ALIGN_TOP);
    rRenderContext.SetFont(aFont);
    rRenderContext.SetTextColor(aTextColor);

    OUString sText(rRenderContext.GetFont().GetFamilyName());
    Size aTextSize(rRenderContext.GetTextWidth(sText), rRenderContext.GetTextHeight());

    rRenderContext.DrawText(Point((rRenderContext.GetOutputSize().Width()  - aTextSize.Width())  / 2,
                                  (rRenderContext.GetOutputSize().Height() - aTextSize.Height()) / 2), sText);
}

void SmShowFont::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize(pDrawingArea->get_ref_device().LogicToPixel(Size(111 , 31), MapMode(MapUnit::MapAppFont)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
}

void SmShowFont::SetFont(const vcl::Font& rFont)
{
    maFont = rFont;
    Invalidate();
}

IMPL_LINK( SmFontDialog, FontSelectHdl, weld::ComboBox&, rComboBox, void )
{
    maFont.SetFamilyName(rComboBox.get_active_text());
    m_aShowFont.SetFont(maFont);
}

IMPL_LINK_NOARG(SmFontDialog, AttrChangeHdl, weld::ToggleButton&, void)
{
    if (m_xBoldCheckBox->get_active())
        maFont.SetWeight(WEIGHT_BOLD);
    else
        maFont.SetWeight(WEIGHT_NORMAL);

    if (m_xItalicCheckBox->get_active())
        maFont.SetItalic(ITALIC_NORMAL);
    else
        maFont.SetItalic(ITALIC_NONE);

    m_aShowFont.SetFont(maFont);
}

void SmFontDialog::SetFont(const vcl::Font &rFont)
{
    maFont = rFont;

    m_xFontBox->set_active_text(maFont.GetFamilyName());
    m_xBoldCheckBox->set_active(IsBold(maFont));
    m_xItalicCheckBox->set_active(IsItalic(maFont));
    m_aShowFont.SetFont(maFont);
}

SmFontDialog::SmFontDialog(weld::Window * pParent, OutputDevice *pFntListDevice, bool bHideCheckboxes)
    : GenericDialogController(pParent, "modules/smath/ui/fontdialog.ui", "FontDialog")
    , m_xFontBox(m_xBuilder->weld_entry_tree_view("fontgrid", "font", "fonts"))
    , m_xAttrFrame(m_xBuilder->weld_widget("attrframe"))
    , m_xBoldCheckBox(m_xBuilder->weld_check_button("bold"))
    , m_xItalicCheckBox(m_xBuilder->weld_check_button("italic"))
    , m_xShowFont(new weld::CustomWeld(*m_xBuilder, "preview", m_aShowFont))
{
    m_xFontBox->set_height_request_by_rows(8);

    {
        weld::WaitObject aWait(pParent);

        FontList aFontList( pFntListDevice );

        sal_uInt16  nCount = aFontList.GetFontNameCount();
        for (sal_uInt16 i = 0;  i < nCount; ++i)
        {
            m_xFontBox->append_text(aFontList.GetFontName(i).GetFamilyName());
        }
        maFont.SetFontSize(Size(0, 24));
        maFont.SetWeight(WEIGHT_NORMAL);
        maFont.SetItalic(ITALIC_NONE);
        maFont.SetFamily(FAMILY_DONTKNOW);
        maFont.SetPitch(PITCH_DONTKNOW);
        maFont.SetCharSet(RTL_TEXTENCODING_DONTKNOW);
        maFont.SetTransparent(true);
    }

    m_xFontBox->connect_changed(LINK(this, SmFontDialog, FontSelectHdl));
    m_xBoldCheckBox->connect_toggled(LINK(this, SmFontDialog, AttrChangeHdl));
    m_xItalicCheckBox->connect_toggled(LINK(this, SmFontDialog, AttrChangeHdl));

    if (bHideCheckboxes)
    {
        m_xBoldCheckBox->set_active(false);
        m_xBoldCheckBox->set_sensitive(false);
        m_xItalicCheckBox->set_active(false);
        m_xItalicCheckBox->set_sensitive(false);
        m_xAttrFrame->hide();
    }
}

SmFontDialog::~SmFontDialog()
{
}

class SaveDefaultsQuery : public weld::MessageDialogController
{
public:
    explicit SaveDefaultsQuery(weld::Widget* pParent)
        : MessageDialogController(pParent, "modules/smath/ui/savedefaultsdialog.ui",
                "SaveDefaultsDialog")
    {
    }
};

IMPL_LINK_NOARG( SmFontSizeDialog, DefaultButtonClickHdl, weld::Button&, void )
{
    SaveDefaultsQuery aQuery(m_xDialog.get());
    if (aQuery.run() == RET_YES)
    {
        SmModule *pp = SM_MOD();
        SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
        WriteTo( aFmt );
        pp->GetConfig()->SetStandardFormat( aFmt );
    }
}

SmFontSizeDialog::SmFontSizeDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/smath/ui/fontsizedialog.ui", "FontSizeDialog")
    , m_xBaseSize(m_xBuilder->weld_metric_spin_button("spinB_baseSize", FieldUnit::POINT))
    , m_xTextSize(m_xBuilder->weld_metric_spin_button("spinB_text", FieldUnit::PERCENT))
    , m_xIndexSize(m_xBuilder->weld_metric_spin_button("spinB_index", FieldUnit::PERCENT))
    , m_xFunctionSize(m_xBuilder->weld_metric_spin_button("spinB_function", FieldUnit::PERCENT))
    , m_xOperatorSize(m_xBuilder->weld_metric_spin_button("spinB_operator", FieldUnit::PERCENT))
    , m_xBorderSize(m_xBuilder->weld_metric_spin_button("spinB_limit", FieldUnit::PERCENT))
    , m_xDefaultButton(m_xBuilder->weld_button("default"))
{
    m_xDefaultButton->connect_clicked(LINK(this, SmFontSizeDialog, DefaultButtonClickHdl));
}

SmFontSizeDialog::~SmFontSizeDialog()
{
}

void SmFontSizeDialog::ReadFrom(const SmFormat &rFormat)
{
    //! watch out: round properly!
    m_xBaseSize->set_value( SmRoundFraction(
        Sm100th_mmToPts( rFormat.GetBaseSize().Height() ) ), FieldUnit::NONE );

    m_xTextSize->set_value( rFormat.GetRelSize(SIZ_TEXT), FieldUnit::NONE );
    m_xIndexSize->set_value( rFormat.GetRelSize(SIZ_INDEX), FieldUnit::NONE );
    m_xFunctionSize->set_value( rFormat.GetRelSize(SIZ_FUNCTION), FieldUnit::NONE );
    m_xOperatorSize->set_value( rFormat.GetRelSize(SIZ_OPERATOR), FieldUnit::NONE );
    m_xBorderSize->set_value( rFormat.GetRelSize(SIZ_LIMITS), FieldUnit::NONE );
}

void SmFontSizeDialog::WriteTo(SmFormat &rFormat) const
{
    rFormat.SetBaseSize( Size(0, SmPtsTo100th_mm( static_cast< long >(m_xBaseSize->get_value(FieldUnit::NONE)))) );

    rFormat.SetRelSize(SIZ_TEXT,     sal::static_int_cast<sal_uInt16>(m_xTextSize->get_value(FieldUnit::NONE)));
    rFormat.SetRelSize(SIZ_INDEX,    sal::static_int_cast<sal_uInt16>(m_xIndexSize->get_value(FieldUnit::NONE)));
    rFormat.SetRelSize(SIZ_FUNCTION, sal::static_int_cast<sal_uInt16>(m_xFunctionSize->get_value(FieldUnit::NONE)));
    rFormat.SetRelSize(SIZ_OPERATOR, sal::static_int_cast<sal_uInt16>(m_xOperatorSize->get_value(FieldUnit::NONE)));
    rFormat.SetRelSize(SIZ_LIMITS,   sal::static_int_cast<sal_uInt16>(m_xBorderSize->get_value(FieldUnit::NONE)));

    const Size aTmp (rFormat.GetBaseSize());
    for (sal_uInt16  i = FNT_BEGIN;  i <= FNT_END;  i++)
        rFormat.SetFontSize(i, aTmp);

    rFormat.RequestApplyChanges();
}

IMPL_LINK(SmFontTypeDialog, MenuSelectHdl, const OString&, rIdent, void)
{
    SmFontPickListBox *pActiveListBox;

    bool bHideCheckboxes = false;
    if (rIdent == "variables")
        pActiveListBox = m_xVariableFont.get();
    else if (rIdent == "functions")
        pActiveListBox = m_xFunctionFont.get();
    else if (rIdent == "numbers")
        pActiveListBox = m_xNumberFont.get();
    else if (rIdent == "text")
        pActiveListBox = m_xTextFont.get();
    else if (rIdent == "serif")
    {
        pActiveListBox = m_xSerifFont.get();
        bHideCheckboxes = true;
    }
    else if (rIdent == "sansserif")
    {
        pActiveListBox = m_xSansFont.get();
        bHideCheckboxes = true;
    }
    else if (rIdent == "fixedwidth")
    {
        pActiveListBox = m_xFixedFont.get();
        bHideCheckboxes = true;
    }
    else
        pActiveListBox = nullptr;

    if (pActiveListBox)
    {
        SmFontDialog aFontDialog(m_xDialog.get(), pFontListDev, bHideCheckboxes);

        pActiveListBox->WriteTo(aFontDialog);
        if (aFontDialog.run() == RET_OK)
            pActiveListBox->ReadFrom(aFontDialog);
    }
}

IMPL_LINK_NOARG(SmFontTypeDialog, DefaultButtonClickHdl, weld::Button&, void)
{
    SaveDefaultsQuery aQuery(m_xDialog.get());
    if (aQuery.run() == RET_YES)
    {
        SmModule *pp = SM_MOD();
        SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
        WriteTo( aFmt );
        pp->GetConfig()->SetStandardFormat( aFmt, true );
    }
}

SmFontTypeDialog::SmFontTypeDialog(weld::Window* pParent, OutputDevice *pFntListDevice)
    : GenericDialogController(pParent, "modules/smath/ui/fonttypedialog.ui", "FontsDialog")
    , pFontListDev(pFntListDevice)
    , m_xVariableFont(new SmFontPickListBox(m_xBuilder->weld_combo_box("variableCB")))
    , m_xFunctionFont(new SmFontPickListBox(m_xBuilder->weld_combo_box("functionCB")))
    , m_xNumberFont(new SmFontPickListBox(m_xBuilder->weld_combo_box("numberCB")))
    , m_xTextFont(new SmFontPickListBox(m_xBuilder->weld_combo_box("textCB")))
    , m_xSerifFont(new SmFontPickListBox(m_xBuilder->weld_combo_box("serifCB")))
    , m_xSansFont(new SmFontPickListBox(m_xBuilder->weld_combo_box("sansCB")))
    , m_xFixedFont(new SmFontPickListBox(m_xBuilder->weld_combo_box("fixedCB")))
    , m_xMenuButton(m_xBuilder->weld_menu_button("modify"))
    , m_xDefaultButton(m_xBuilder->weld_button("default"))
{
    m_xDefaultButton->connect_clicked(LINK(this, SmFontTypeDialog, DefaultButtonClickHdl));
    m_xMenuButton->connect_selected(LINK(this, SmFontTypeDialog, MenuSelectHdl));
}

SmFontTypeDialog::~SmFontTypeDialog()
{
}

void SmFontTypeDialog::ReadFrom(const SmFormat &rFormat)
{
    SmModule *pp = SM_MOD();

    *m_xVariableFont = pp->GetConfig()->GetFontPickList(FNT_VARIABLE);
    *m_xFunctionFont = pp->GetConfig()->GetFontPickList(FNT_FUNCTION);
    *m_xNumberFont   = pp->GetConfig()->GetFontPickList(FNT_NUMBER);
    *m_xTextFont     = pp->GetConfig()->GetFontPickList(FNT_TEXT);
    *m_xSerifFont    = pp->GetConfig()->GetFontPickList(FNT_SERIF);
    *m_xSansFont     = pp->GetConfig()->GetFontPickList(FNT_SANS);
    *m_xFixedFont    = pp->GetConfig()->GetFontPickList(FNT_FIXED);

    m_xVariableFont->Insert( rFormat.GetFont(FNT_VARIABLE) );
    m_xFunctionFont->Insert( rFormat.GetFont(FNT_FUNCTION) );
    m_xNumberFont->Insert( rFormat.GetFont(FNT_NUMBER) );
    m_xTextFont->Insert( rFormat.GetFont(FNT_TEXT) );
    m_xSerifFont->Insert( rFormat.GetFont(FNT_SERIF) );
    m_xSansFont->Insert( rFormat.GetFont(FNT_SANS) );
    m_xFixedFont->Insert( rFormat.GetFont(FNT_FIXED) );
}


void SmFontTypeDialog::WriteTo(SmFormat &rFormat) const
{
    SmModule *pp = SM_MOD();

    pp->GetConfig()->GetFontPickList(FNT_VARIABLE) = *m_xVariableFont;
    pp->GetConfig()->GetFontPickList(FNT_FUNCTION) = *m_xFunctionFont;
    pp->GetConfig()->GetFontPickList(FNT_NUMBER)   = *m_xNumberFont;
    pp->GetConfig()->GetFontPickList(FNT_TEXT)     = *m_xTextFont;
    pp->GetConfig()->GetFontPickList(FNT_SERIF)    = *m_xSerifFont;
    pp->GetConfig()->GetFontPickList(FNT_SANS)     = *m_xSansFont;
    pp->GetConfig()->GetFontPickList(FNT_FIXED)    = *m_xFixedFont;

    rFormat.SetFont( FNT_VARIABLE, m_xVariableFont->Get() );
    rFormat.SetFont( FNT_FUNCTION, m_xFunctionFont->Get() );
    rFormat.SetFont( FNT_NUMBER,   m_xNumberFont->Get() );
    rFormat.SetFont( FNT_TEXT,     m_xTextFont->Get() );
    rFormat.SetFont( FNT_SERIF,    m_xSerifFont->Get() );
    rFormat.SetFont( FNT_SANS,     m_xSansFont->Get() );
    rFormat.SetFont( FNT_FIXED,    m_xFixedFont->Get() );

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

SmCategoryDesc::SmCategoryDesc(weld::Builder& rBuilder, sal_uInt16 nCategoryIdx)
{
    ++nCategoryIdx;
    std::unique_ptr<weld::Label> xTitle(rBuilder.weld_label(OString::number(nCategoryIdx)+"title"));
    if (xTitle)
    {
        Name = xTitle->get_label();
    }
    for (int i = 0; i < 4; ++i)
    {
        std::unique_ptr<weld::Label> xLabel(rBuilder.weld_label(OString::number(nCategoryIdx)+"label"+OString::number(i+1)));

        if (xLabel)
        {
            Strings[i] = xLabel->get_label();
            Graphics[i] = rBuilder.weld_widget(OString::number(nCategoryIdx)+"image"+OString::number(i+1));
        }
        else
        {
            Strings[i].clear();
            Graphics[i].reset();
        }

        const FieldMinMax& rMinMax = pMinMaxData[ nCategoryIdx-1 ][i];
        Value[i] = Minimum[i] = rMinMax.nMin;
        Maximum[i] = rMinMax.nMax;
    }
}

SmCategoryDesc::~SmCategoryDesc()
{
}

/**************************************************************************/

IMPL_LINK( SmDistanceDialog, GetFocusHdl, weld::Widget&, rControl, void )
{
    if (!m_xCategories[nActiveCategory])
        return;

    sal_uInt16  i;

    if (&rControl == &m_xMetricField1->get_widget())
        i = 0;
    else if (&rControl == &m_xMetricField2->get_widget())
        i = 1;
    else if (&rControl == &m_xMetricField3->get_widget())
        i = 2;
    else if (&rControl == &m_xMetricField4->get_widget())
        i = 3;
    else
        return;
    if (m_pCurrentImage)
        m_pCurrentImage->hide();
    m_pCurrentImage = m_xCategories[nActiveCategory]->GetGraphic(i);
    m_pCurrentImage->show();
}

IMPL_LINK(SmDistanceDialog, MenuSelectHdl, const OString&, rId, void)
{
    assert(rId.startsWith("menuitem"));
    SetCategory(rId.replaceFirst("menuitem", "").toInt32() - 1);
}

IMPL_LINK_NOARG( SmDistanceDialog, DefaultButtonClickHdl, weld::Button&, void )
{
    SaveDefaultsQuery aQuery(m_xDialog.get());
    if (aQuery.run() == RET_YES)
    {
        SmModule *pp = SM_MOD();
        SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
        WriteTo( aFmt );
        pp->GetConfig()->SetStandardFormat( aFmt );
    }
}

IMPL_LINK( SmDistanceDialog, CheckBoxClickHdl, weld::ToggleButton&, rCheckBox, void )
{
    if (&rCheckBox == m_xCheckBox1.get())
    {
        bool bChecked = m_xCheckBox1->get_active();
        m_xFixedText4->set_sensitive( bChecked );
        m_xMetricField4->set_sensitive( bChecked );
    }
}

void SmDistanceDialog::SetCategory(sal_uInt16 nCategory)
{
    assert(nCategory < NOCATEGORIES && "Sm: wrong category number in SmDistanceDialog");

    // array to convert category- and metricfield-number in help ids.
    // 0 is used in case of unused combinations.
    assert(NOCATEGORIES == 10 && "Sm : array doesn't fit into the number of categories");
    static const char * aCatMf2Hid[10][4] =
    {
        { HID_SMA_DEFAULT_DIST,         HID_SMA_LINE_DIST,          HID_SMA_ROOT_DIST, nullptr },
        { HID_SMA_SUP_DIST,             HID_SMA_SUB_DIST ,          nullptr, nullptr },
        { HID_SMA_NUMERATOR_DIST,       HID_SMA_DENOMINATOR_DIST,   nullptr, nullptr },
        { HID_SMA_FRACLINE_EXCWIDTH,    HID_SMA_FRACLINE_LINEWIDTH, nullptr, nullptr },
        { HID_SMA_UPPERLIMIT_DIST,      HID_SMA_LOWERLIMIT_DIST,    nullptr, nullptr },
        { HID_SMA_BRACKET_EXCHEIGHT,    HID_SMA_BRACKET_DIST,       nullptr, HID_SMA_BRACKET_EXCHEIGHT2 },
        { HID_SMA_MATRIXROW_DIST,       HID_SMA_MATRIXCOL_DIST,     nullptr, nullptr },
        { HID_SMA_ATTRIBUT_DIST,        HID_SMA_INTERATTRIBUT_DIST, nullptr, nullptr },
        { HID_SMA_OPERATOR_EXCHEIGHT,   HID_SMA_OPERATOR_DIST,      nullptr, nullptr },
        { HID_SMA_LEFTBORDER_DIST,      HID_SMA_RIGHTBORDER_DIST,   HID_SMA_UPPERBORDER_DIST, HID_SMA_LOWERBORDER_DIST }
    };

    // array to help iterate over the controls
    std::pair<weld::Label*, weld::MetricSpinButton*> const aWin[4] =
    {
        { m_xFixedText1.get(), m_xMetricField1.get() },
        { m_xFixedText2.get(), m_xMetricField2.get() },
        { m_xFixedText3.get(), m_xMetricField3.get() },
        { m_xFixedText4.get(), m_xMetricField4.get() }
    };

    SmCategoryDesc *pCat;

    // remember the (maybe new) settings of the active SmCategoryDesc
    // before switching to the new one
    if (nActiveCategory != CATEGORY_NONE)
    {
        pCat = m_xCategories[nActiveCategory].get();
        pCat->SetValue(0, sal::static_int_cast<sal_uInt16>(m_xMetricField1->get_value(FieldUnit::NONE)));
        pCat->SetValue(1, sal::static_int_cast<sal_uInt16>(m_xMetricField2->get_value(FieldUnit::NONE)));
        pCat->SetValue(2, sal::static_int_cast<sal_uInt16>(m_xMetricField3->get_value(FieldUnit::NONE)));
        pCat->SetValue(3, sal::static_int_cast<sal_uInt16>(m_xMetricField4->get_value(FieldUnit::NONE)));

        if (nActiveCategory == 5)
            bScaleAllBrackets = m_xCheckBox1->get_active();

        m_xMenuButton->set_item_active("menuitem" + OString::number(nActiveCategory + 1), false);
    }

    // activation/deactivation of the associated controls depending on the chosen category
    bool  bActive;
    for (sal_uInt16 i = 0;  i < 4;  i++)
    {
        weld::Label *pFT = aWin[i].first;
        weld::MetricSpinButton *pMF = aWin[i].second;

        // To determine which Controls should be active, the existence
        // of an associated HelpID is checked
        bActive = aCatMf2Hid[nCategory][i] != nullptr;

        pFT->set_visible(bActive);
        pFT->set_sensitive(bActive);
        pMF->set_visible(bActive);
        pMF->set_sensitive(bActive);

        // set measurement unit and number of decimal places
        FieldUnit  eUnit;
        sal_uInt16     nDigits;
        if (nCategory < 9)
        {
            eUnit = FieldUnit::PERCENT;
            nDigits = 0;
        }
        else
        {
            eUnit   = FieldUnit::MM_100TH;
            nDigits = 2;
        }
        pMF->set_unit(eUnit);            // changes the value
        pMF->set_digits(nDigits);

        if (bActive)
        {
            pCat = m_xCategories[nCategory].get();
            pFT->set_label(pCat->GetString(i));

            pMF->set_range(pCat->GetMinimum(i), pCat->GetMaximum(i), FieldUnit::NONE);
            pMF->set_value(pCat->GetValue(i), FieldUnit::NONE);

            pMF->set_help_id(aCatMf2Hid[nCategory][i]);
        }
    }
    // activate the CheckBox and the associated MetricField if we're dealing with the brackets menu
    bActive = nCategory == 5;
    m_xCheckBox1->set_visible(bActive);
    m_xCheckBox1->set_sensitive(bActive);
    if (bActive)
    {
        m_xCheckBox1->set_active(bScaleAllBrackets);

        bool bChecked = m_xCheckBox1->get_active();
        m_xFixedText4->set_sensitive( bChecked );
        m_xMetricField4->set_sensitive( bChecked );
    }

    m_xMenuButton->set_item_active("menuitem" + OString::number(nCategory + 1), true);
    m_xFrame->set_label(m_xCategories[nCategory]->GetName());

    nActiveCategory = nCategory;

    m_xMetricField1->grab_focus();
}

SmDistanceDialog::SmDistanceDialog(weld::Window *pParent)
    : GenericDialogController(pParent, "modules/smath/ui/spacingdialog.ui", "SpacingDialog")
    , m_xFrame(m_xBuilder->weld_frame("template"))
    , m_xFixedText1(m_xBuilder->weld_label("label1"))
    , m_xMetricField1(m_xBuilder->weld_metric_spin_button("spinbutton1", FieldUnit::CM))
    , m_xFixedText2(m_xBuilder->weld_label("label2"))
    , m_xMetricField2(m_xBuilder->weld_metric_spin_button("spinbutton2", FieldUnit::CM))
    , m_xFixedText3(m_xBuilder->weld_label("label3"))
    , m_xMetricField3(m_xBuilder->weld_metric_spin_button("spinbutton3", FieldUnit::CM))
    , m_xCheckBox1(m_xBuilder->weld_check_button("checkbutton"))
    , m_xFixedText4(m_xBuilder->weld_label("label4"))
    , m_xMetricField4(m_xBuilder->weld_metric_spin_button("spinbutton4", FieldUnit::CM))
    , m_xMenuButton(m_xBuilder->weld_menu_button("category"))
    , m_xDefaultButton(m_xBuilder->weld_button("default"))
    , m_xBitmap(m_xBuilder->weld_widget("image"))
    , m_pCurrentImage(m_xBitmap.get())
{
    for (sal_uInt16 i = 0; i < NOCATEGORIES; ++i)
        m_xCategories[i].reset( new SmCategoryDesc(*m_xBuilder, i) );
    nActiveCategory   = CATEGORY_NONE;
    bScaleAllBrackets = false;

    m_xMetricField1->connect_focus_in(LINK(this, SmDistanceDialog, GetFocusHdl));
    m_xMetricField2->connect_focus_in(LINK(this, SmDistanceDialog, GetFocusHdl));
    m_xMetricField3->connect_focus_in(LINK(this, SmDistanceDialog, GetFocusHdl));
    m_xMetricField4->connect_focus_in(LINK(this, SmDistanceDialog, GetFocusHdl));
    m_xCheckBox1->connect_toggled(LINK(this, SmDistanceDialog, CheckBoxClickHdl));
    m_xMenuButton->connect_selected(LINK(this, SmDistanceDialog, MenuSelectHdl));
    m_xDefaultButton->connect_clicked(LINK(this, SmDistanceDialog, DefaultButtonClickHdl));

    //set the initial size, with max visible widgets visible, as preferred size
    m_xDialog->set_size_request(-1, m_xDialog->get_preferred_size().Height());
}

SmDistanceDialog::~SmDistanceDialog()
{
}

void SmDistanceDialog::ReadFrom(const SmFormat &rFormat)
{
    m_xCategories[0]->SetValue(0, rFormat.GetDistance(DIS_HORIZONTAL));
    m_xCategories[0]->SetValue(1, rFormat.GetDistance(DIS_VERTICAL));
    m_xCategories[0]->SetValue(2, rFormat.GetDistance(DIS_ROOT));
    m_xCategories[1]->SetValue(0, rFormat.GetDistance(DIS_SUPERSCRIPT));
    m_xCategories[1]->SetValue(1, rFormat.GetDistance(DIS_SUBSCRIPT));
    m_xCategories[2]->SetValue(0, rFormat.GetDistance(DIS_NUMERATOR));
    m_xCategories[2]->SetValue(1, rFormat.GetDistance(DIS_DENOMINATOR));
    m_xCategories[3]->SetValue(0, rFormat.GetDistance(DIS_FRACTION));
    m_xCategories[3]->SetValue(1, rFormat.GetDistance(DIS_STROKEWIDTH));
    m_xCategories[4]->SetValue(0, rFormat.GetDistance(DIS_UPPERLIMIT));
    m_xCategories[4]->SetValue(1, rFormat.GetDistance(DIS_LOWERLIMIT));
    m_xCategories[5]->SetValue(0, rFormat.GetDistance(DIS_BRACKETSIZE));
    m_xCategories[5]->SetValue(1, rFormat.GetDistance(DIS_BRACKETSPACE));
    m_xCategories[5]->SetValue(3, rFormat.GetDistance(DIS_NORMALBRACKETSIZE));
    m_xCategories[6]->SetValue(0, rFormat.GetDistance(DIS_MATRIXROW));
    m_xCategories[6]->SetValue(1, rFormat.GetDistance(DIS_MATRIXCOL));
    m_xCategories[7]->SetValue(0, rFormat.GetDistance(DIS_ORNAMENTSIZE));
    m_xCategories[7]->SetValue(1, rFormat.GetDistance(DIS_ORNAMENTSPACE));
    m_xCategories[8]->SetValue(0, rFormat.GetDistance(DIS_OPERATORSIZE));
    m_xCategories[8]->SetValue(1, rFormat.GetDistance(DIS_OPERATORSPACE));
    m_xCategories[9]->SetValue(0, rFormat.GetDistance(DIS_LEFTSPACE));
    m_xCategories[9]->SetValue(1, rFormat.GetDistance(DIS_RIGHTSPACE));
    m_xCategories[9]->SetValue(2, rFormat.GetDistance(DIS_TOPSPACE));
    m_xCategories[9]->SetValue(3, rFormat.GetDistance(DIS_BOTTOMSPACE));

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

    rFormat.SetDistance( DIS_HORIZONTAL,        m_xCategories[0]->GetValue(0) );
    rFormat.SetDistance( DIS_VERTICAL,          m_xCategories[0]->GetValue(1) );
    rFormat.SetDistance( DIS_ROOT,              m_xCategories[0]->GetValue(2) );
    rFormat.SetDistance( DIS_SUPERSCRIPT,       m_xCategories[1]->GetValue(0) );
    rFormat.SetDistance( DIS_SUBSCRIPT,         m_xCategories[1]->GetValue(1) );
    rFormat.SetDistance( DIS_NUMERATOR,         m_xCategories[2]->GetValue(0) );
    rFormat.SetDistance( DIS_DENOMINATOR,       m_xCategories[2]->GetValue(1) );
    rFormat.SetDistance( DIS_FRACTION,          m_xCategories[3]->GetValue(0) );
    rFormat.SetDistance( DIS_STROKEWIDTH,       m_xCategories[3]->GetValue(1) );
    rFormat.SetDistance( DIS_UPPERLIMIT,        m_xCategories[4]->GetValue(0) );
    rFormat.SetDistance( DIS_LOWERLIMIT,        m_xCategories[4]->GetValue(1) );
    rFormat.SetDistance( DIS_BRACKETSIZE,       m_xCategories[5]->GetValue(0) );
    rFormat.SetDistance( DIS_BRACKETSPACE,      m_xCategories[5]->GetValue(1) );
    rFormat.SetDistance( DIS_MATRIXROW,         m_xCategories[6]->GetValue(0) );
    rFormat.SetDistance( DIS_MATRIXCOL,         m_xCategories[6]->GetValue(1) );
    rFormat.SetDistance( DIS_ORNAMENTSIZE,      m_xCategories[7]->GetValue(0) );
    rFormat.SetDistance( DIS_ORNAMENTSPACE,     m_xCategories[7]->GetValue(1) );
    rFormat.SetDistance( DIS_OPERATORSIZE,      m_xCategories[8]->GetValue(0) );
    rFormat.SetDistance( DIS_OPERATORSPACE,     m_xCategories[8]->GetValue(1) );
    rFormat.SetDistance( DIS_LEFTSPACE,         m_xCategories[9]->GetValue(0) );
    rFormat.SetDistance( DIS_RIGHTSPACE,        m_xCategories[9]->GetValue(1) );
    rFormat.SetDistance( DIS_TOPSPACE,          m_xCategories[9]->GetValue(2) );
    rFormat.SetDistance( DIS_BOTTOMSPACE,       m_xCategories[9]->GetValue(3) );
    rFormat.SetDistance( DIS_NORMALBRACKETSIZE, m_xCategories[5]->GetValue(3) );

    rFormat.SetScaleNormalBrackets( bScaleAllBrackets );

    rFormat.RequestApplyChanges();
}

IMPL_LINK_NOARG( SmAlignDialog, DefaultButtonClickHdl, weld::Button&, void )
{
    SaveDefaultsQuery aQuery(m_xDialog.get());
    if (aQuery.run() == RET_YES)
    {
        SmModule *pp = SM_MOD();
        SmFormat aFmt( pp->GetConfig()->GetStandardFormat() );
        WriteTo( aFmt );
        pp->GetConfig()->SetStandardFormat( aFmt );
    }
}

SmAlignDialog::SmAlignDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/smath/ui/alignmentdialog.ui", "AlignmentDialog")
    , m_xLeft(m_xBuilder->weld_radio_button("left"))
    , m_xCenter(m_xBuilder->weld_radio_button("center"))
    , m_xRight(m_xBuilder->weld_radio_button("right"))
    , m_xDefaultButton(m_xBuilder->weld_button("default"))
{
    m_xDefaultButton->connect_clicked(LINK(this, SmAlignDialog, DefaultButtonClickHdl));
}

SmAlignDialog::~SmAlignDialog()
{
}

void SmAlignDialog::ReadFrom(const SmFormat &rFormat)
{
    switch (rFormat.GetHorAlign())
    {
        case SmHorAlign::Left:
            m_xLeft->set_active(true);
            break;
        case SmHorAlign::Center:
            m_xCenter->set_active(true);
            break;
        case SmHorAlign::Right:
            m_xRight->set_active(true);
            break;
    }
}

void SmAlignDialog::WriteTo(SmFormat &rFormat) const
{
    if (m_xLeft->get_active())
        rFormat.SetHorAlign(SmHorAlign::Left);
    else if (m_xRight->get_active())
        rFormat.SetHorAlign(SmHorAlign::Right);
    else
        rFormat.SetHorAlign(SmHorAlign::Center);

    rFormat.RequestApplyChanges();
}

SmShowSymbolSet::SmShowSymbolSet(std::unique_ptr<weld::ScrolledWindow> pScrolledWindow)
    : nLen(0)
    , nRows(0)
    , nColumns(0)
    , nXOffset(0)
    , nYOffset(0)
    , nSelectSymbol(SYMBOL_NONE)
    , m_xScrolledWindow(std::move(pScrolledWindow))
{
    m_xScrolledWindow->set_user_managed_scrolling();
    m_xScrolledWindow->connect_vadjustment_changed(LINK(this, SmShowSymbolSet, ScrollHdl));
}

Point SmShowSymbolSet::OffsetPoint(const Point &rPoint) const
{
    return Point(rPoint.X() + nXOffset, rPoint.Y() + nYOffset);
}

void SmShowSymbolSet::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    Size aWinSize(GetOutputSizePixel());
    if (aWinSize != m_aOldSize)
    {
        calccols(rRenderContext);
        m_aOldSize = aWinSize;
    }

    Color aBackgroundColor;
    Color aTextColor;
    lclGetSettingColors(aBackgroundColor, aTextColor);

    rRenderContext.SetBackground(Wallpaper(aBackgroundColor));
    rRenderContext.SetTextColor(aTextColor);

    rRenderContext.Push(PushFlags::MAPMODE);

    // set MapUnit for which 'nLen' has been calculated
    rRenderContext.SetMapMode(MapMode(MapUnit::MapPixel));

    sal_uInt16 v = sal::static_int_cast< sal_uInt16 >(m_xScrolledWindow->vadjustment_get_value() * nColumns);
    size_t nSymbols = aSymbolSet.size();

    Color aTxtColor(rRenderContext.GetTextColor());
    for (size_t i = v; i < nSymbols ; i++)
    {
        SmSym aSymbol(*aSymbolSet[i]);
        vcl::Font aFont(aSymbol.GetFace());
        aFont.SetAlignment(ALIGN_TOP);

        // taking a FontSize which is a bit smaller (compared to nLen) in order to have a buffer
        // (hopefully enough for left and right, too)
        aFont.SetFontSize(Size(0, nLen - (nLen / 3)));
        rRenderContext.SetFont(aFont);
        // keep text color
        rRenderContext.SetTextColor(aTxtColor);

        int nIV = i - v;
        sal_UCS4 cChar = aSymbol.GetCharacter();
        OUString aText(&cChar, 1);
        Size  aSize(rRenderContext.GetTextWidth( aText ), rRenderContext.GetTextHeight());

        Point aPoint((nIV % nColumns) * nLen + (nLen - aSize.Width()) / 2,
                     (nIV / nColumns) * nLen + (nLen - aSize.Height()) / 2);

        rRenderContext.DrawText(OffsetPoint(aPoint), aText);
    }

    if (nSelectSymbol != SYMBOL_NONE)
    {
        Point aPoint(((nSelectSymbol - v) % nColumns) * nLen,
                                 ((nSelectSymbol - v) / nColumns) * nLen);

        rRenderContext.Invert(tools::Rectangle(OffsetPoint(aPoint), Size(nLen, nLen)));

    }

    rRenderContext.Pop();
}

bool SmShowSymbolSet::MouseButtonDown(const MouseEvent& rMEvt)
{
    GrabFocus();

    Size aOutputSize(nColumns * nLen, nRows * nLen);
    aOutputSize.AdjustWidth(nXOffset );
    aOutputSize.AdjustHeight(nYOffset );
    Point aPoint(rMEvt.GetPosPixel());
    aPoint.AdjustX( -nXOffset );
    aPoint.AdjustY( -nYOffset );

    if (rMEvt.IsLeft() && tools::Rectangle(Point(0, 0), aOutputSize).IsInside(rMEvt.GetPosPixel()))
    {
        long nPos = (aPoint.Y() / nLen) * nColumns + (aPoint.X() / nLen) +
                      m_xScrolledWindow->vadjustment_get_value() * nColumns;
        SelectSymbol( sal::static_int_cast< sal_uInt16 >(nPos) );

        aSelectHdlLink.Call(*this);

        if (rMEvt.GetClicks() > 1)
            aDblClickHdlLink.Call(*this);
    }

    return true;
}

bool SmShowSymbolSet::KeyInput(const KeyEvent& rKEvt)
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
                return false;
        }
    }
    else
        n = 0;

    if (n >= aSymbolSet.size())
        n = nSelectSymbol;

    // adjust scrollbar
    if ((n < sal::static_int_cast<sal_uInt16>(m_xScrolledWindow->vadjustment_get_value() * nColumns)) ||
        (n >= sal::static_int_cast<sal_uInt16>((m_xScrolledWindow->vadjustment_get_value() + nRows) * nColumns)))
    {
        m_xScrolledWindow->vadjustment_set_value(n / nColumns);
        Invalidate();
    }

    SelectSymbol(n);
    aSelectHdlLink.Call(*this);

    return true;
}

void SmShowSymbolSet::calccols(const vcl::RenderContext& rRenderContext)
{
    // Height of 16pt in pixels (matching 'aOutputSize')
    nLen = rRenderContext.LogicToPixel(Size(0, 16), MapMode(MapUnit::MapPoint)).Height();

    Size aOutputSize(GetOutputSizePixel());

    nColumns = aOutputSize.Width() / nLen;
    nRows = aOutputSize.Height() / nLen;
    nColumns = std::max<long>(1, nColumns);
    nRows = std::max<long>(1, nRows);

    nXOffset = (aOutputSize.Width() - (nColumns * nLen)) / 2;
    nYOffset = (aOutputSize.Height() - (nRows * nLen)) / 2;

    SetScrollBarRange();
}

void SmShowSymbolSet::SetSymbolSet(const SymbolPtrVec_t& rSymbolSet)
{
    aSymbolSet = rSymbolSet;
    Invalidate();
}

void SmShowSymbolSet::SetScrollBarRange()
{
    const int nLastRow = (aSymbolSet.size() - 1 + nColumns) / nColumns;
    m_xScrolledWindow->vadjustment_configure(m_xScrolledWindow->vadjustment_get_value(), 0, nLastRow, 1, nRows - 1, nRows);
    Invalidate();
}

void SmShowSymbolSet::SelectSymbol(sal_uInt16 nSymbol)
{
    int v = m_xScrolledWindow->vadjustment_get_value() * nColumns;

    if (nSelectSymbol != SYMBOL_NONE && nColumns)
    {
        Point aPoint(OffsetPoint(Point(((nSelectSymbol - v) % nColumns) * nLen,
                                       ((nSelectSymbol - v) / nColumns) * nLen)));
        Invalidate(tools::Rectangle(aPoint, Size(nLen, nLen)));
    }

    if (nSymbol < aSymbolSet.size())
        nSelectSymbol = nSymbol;

    if (aSymbolSet.empty())
        nSelectSymbol = SYMBOL_NONE;

    if (nSelectSymbol != SYMBOL_NONE && nColumns)
    {
        Point aPoint(OffsetPoint(Point(((nSelectSymbol - v) % nColumns) * nLen,
                                       ((nSelectSymbol - v) / nColumns) * nLen)));
        Invalidate(tools::Rectangle(aPoint, Size(nLen, nLen)));
    }

    if (!nColumns)
        Invalidate();
}

IMPL_LINK_NOARG(SmShowSymbolSet, ScrollHdl, weld::ScrolledWindow&, void)
{
    Invalidate();
}

SmShowSymbol::SmShowSymbol()
{
}

void SmShowSymbol::setFontSize(vcl::Font &rFont) const
{
    Size aSize(GetOutputSizePixel());
    rFont.SetFontSize(Size(0, aSize.Height() - aSize.Height() / 3));
}

void SmShowSymbol::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    Color aBackgroundColor;
    Color aTextColor;
    lclGetSettingColors(aBackgroundColor, aTextColor);
    rRenderContext.SetBackground(Wallpaper(aBackgroundColor));
    rRenderContext.SetTextColor(aTextColor);
    rRenderContext.Erase();

    vcl::Font aFont(GetFont());
    setFontSize(aFont);
    rRenderContext.SetFont(aFont);

    const OUString &rText = GetText();
    Size aTextSize(rRenderContext.GetTextWidth(rText), rRenderContext.GetTextHeight());

    rRenderContext.DrawText(Point((rRenderContext.GetOutputSize().Width()  - aTextSize.Width())  / 2,
                                  (rRenderContext.GetOutputSize().Height() * 7 / 10)), rText);
}

bool SmShowSymbol::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (rMEvt.GetClicks() > 1)
        aDblClickHdlLink.Call(*this);
    return true;
}

void SmShowSymbol::SetSymbol(const SmSym *pSymbol)
{
    if (pSymbol)
    {
        vcl::Font aFont(pSymbol->GetFace());
        aFont.SetAlignment(ALIGN_BASELINE);
        SetFont(aFont);

        sal_UCS4 cChar = pSymbol->GetCharacter();
        OUString aText(&cChar, 1);
        SetText( aText );
    }

    Invalidate();
}

void SmSymbolDialog::FillSymbolSets()
    // populate the entries of possible SymbolsSets in the dialog with
    // current values of the SymbolSet manager but selects none of those
{
    m_xSymbolSets->clear();
    m_xSymbolSets->set_active(-1);

    std::set< OUString >  aSybolSetNames( rSymbolMgr.GetSymbolSetNames() );
    for (const auto& rSymbolSetName : aSybolSetNames)
        m_xSymbolSets->append_text(rSymbolSetName);
}

IMPL_LINK_NOARG( SmSymbolDialog, SymbolSetChangeHdl, weld::ComboBox&, void )
{
    SelectSymbolSet(m_xSymbolSets->get_active_text());
}

IMPL_LINK_NOARG( SmSymbolDialog, SymbolChangeHdl, SmShowSymbolSet&, void )
{
    SelectSymbol(m_xSymbolSetDisplay->GetSelectSymbol());
}

IMPL_LINK_NOARG(SmSymbolDialog, EditClickHdl, weld::Button&, void)
{
    SmSymDefineDialog aDialog(m_xDialog.get(), pFontListDev, rSymbolMgr);

    // set current symbol and SymbolSet for the new dialog
    const OUString  aSymSetName (m_xSymbolSets->get_active_text()),
                    aSymName    (m_xSymbolName->get_label());
    aDialog.SelectOldSymbolSet(aSymSetName);
    aDialog.SelectOldSymbol(aSymName);
    aDialog.SelectSymbolSet(aSymSetName);
    aDialog.SelectSymbol(aSymName);

    // remember old SymbolSet
    OUString  aOldSymbolSet (m_xSymbolSets->get_active_text());

    sal_uInt16 nSymPos = m_xSymbolSetDisplay->GetSelectSymbol();

    // adapt dialog to data of the SymbolSet manager, which might have changed
    if (aDialog.run() == RET_OK && rSymbolMgr.IsModified())
    {
        rSymbolMgr.Save();
        FillSymbolSets();
    }

    // if the old SymbolSet doesn't exist anymore, go to the first one SymbolSet (if one exists)
    if (!SelectSymbolSet(aOldSymbolSet) && m_xSymbolSets->get_count() > 0)
        SelectSymbolSet(m_xSymbolSets->get_text(0));
    else
    {
        // just update display of current symbol set
        assert(aSymSetName == aSymSetName); //unexpected change in symbol set name
        aSymbolSet = rSymbolMgr.GetSymbolSet( aSymbolSetName );
        m_xSymbolSetDisplay->SetSymbolSet( aSymbolSet );
    }

    if (nSymPos >= aSymbolSet.size())
        nSymPos = static_cast< sal_uInt16 >(aSymbolSet.size()) - 1;
    SelectSymbol( nSymPos );
}

IMPL_LINK_NOARG( SmSymbolDialog, SymbolDblClickHdl2, SmShowSymbolSet&, void )
{
    SymbolDblClickHdl();
}

IMPL_LINK_NOARG( SmSymbolDialog, SymbolDblClickHdl, SmShowSymbol&, void )
{
    SymbolDblClickHdl();
}

void SmSymbolDialog::SymbolDblClickHdl()
{
    GetClickHdl(*m_xGetBtn);
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SmSymbolDialog, GetClickHdl, weld::Button&, void)
{
    const SmSym *pSym = GetSymbol();
    if (pSym)
    {
        OUString aText = "%" + pSym->GetName() + " ";

        rViewSh.GetViewFrame()->GetDispatcher()->ExecuteList(
                SID_INSERTSPECIAL, SfxCallMode::RECORD,
                { new SfxStringItem(SID_INSERTSPECIAL, aText) });
    }
}

SmSymbolDialog::SmSymbolDialog(weld::Window *pParent, OutputDevice *pFntListDevice,
                               SmSymbolManager &rMgr, SmViewShell &rViewShell)
    : GenericDialogController(pParent, "modules/smath/ui/catalogdialog.ui", "CatalogDialog")
    , rViewSh(rViewShell)
    , rSymbolMgr(rMgr)
    , pFontListDev(pFntListDevice)
    , m_xSymbolSets(m_xBuilder->weld_combo_box("symbolset"))
    , m_xSymbolSetDisplay(new SmShowSymbolSet(m_xBuilder->weld_scrolled_window("scrolledwindow")))
    , m_xSymbolSetDisplayArea(new weld::CustomWeld(*m_xBuilder, "symbolsetdisplay", *m_xSymbolSetDisplay))
    , m_xSymbolName(m_xBuilder->weld_label("symbolname"))
    , m_xSymbolDisplay(new weld::CustomWeld(*m_xBuilder, "preview", m_aSymbolDisplay))
    , m_xGetBtn(m_xBuilder->weld_button("ok"))
    , m_xEditBtn(m_xBuilder->weld_button("edit"))
{
    m_xSymbolSets->make_sorted();

    aSymbolSetName.clear();
    aSymbolSet.clear();
    FillSymbolSets();
    if (m_xSymbolSets->get_count() > 0)
        SelectSymbolSet(m_xSymbolSets->get_text(0));

    m_xSymbolSets->connect_changed(LINK(this, SmSymbolDialog, SymbolSetChangeHdl));
    m_xSymbolSetDisplay->SetSelectHdl(LINK(this, SmSymbolDialog, SymbolChangeHdl));
    m_xSymbolSetDisplay->SetDblClickHdl(LINK(this, SmSymbolDialog, SymbolDblClickHdl2));
    m_aSymbolDisplay.SetDblClickHdl(LINK(this, SmSymbolDialog, SymbolDblClickHdl));
    m_xEditBtn->connect_clicked(LINK(this, SmSymbolDialog, EditClickHdl));
    m_xGetBtn->connect_clicked(LINK(this, SmSymbolDialog, GetClickHdl));
}

SmSymbolDialog::~SmSymbolDialog()
{
}

bool SmSymbolDialog::SelectSymbolSet(const OUString &rSymbolSetName)
{
    bool bRet = false;
    sal_Int32 nPos = m_xSymbolSets->find_text(rSymbolSetName);

    aSymbolSetName.clear();
    aSymbolSet.clear();
    if (nPos != -1)
    {
        m_xSymbolSets->set_active(nPos);

        aSymbolSetName  = rSymbolSetName;
        aSymbolSet      = rSymbolMgr.GetSymbolSet( aSymbolSetName );

        // sort symbols by Unicode position (useful for displaying Greek characters alphabetically)
        std::sort( aSymbolSet.begin(), aSymbolSet.end(),
                   [](const SmSym *pSym1, const SmSym *pSym2)
                   {
                       return pSym1->GetCharacter() < pSym2->GetCharacter();
                   } );

        m_xSymbolSetDisplay->SetSymbolSet( aSymbolSet );
        if (!aSymbolSet.empty())
            SelectSymbol(0);

        bRet = true;
    }
    else
        m_xSymbolSets->set_active(-1);

    return bRet;
}

void SmSymbolDialog::SelectSymbol(sal_uInt16 nSymbolNo)
{
    const SmSym *pSym = nullptr;
    if (!aSymbolSetName.isEmpty()  &&  nSymbolNo < static_cast< sal_uInt16 >(aSymbolSet.size()))
        pSym = aSymbolSet[ nSymbolNo ];

    m_xSymbolSetDisplay->SelectSymbol(nSymbolNo);
    m_aSymbolDisplay.SetSymbol(pSym);
    m_xSymbolName->set_label(pSym ? pSym->GetName() : OUString());
}

const SmSym* SmSymbolDialog::GetSymbol() const
{
    sal_uInt16 nSymbolNo = m_xSymbolSetDisplay->GetSelectSymbol();
    bool bValid = !aSymbolSetName.isEmpty()  &&  nSymbolNo < static_cast< sal_uInt16 >(aSymbolSet.size());
    return bValid ? aSymbolSet[ nSymbolNo ] : nullptr;
}

void SmShowChar::Resize()
{
    const OUString &rText = GetText();
    if (rText.isEmpty())
        return;
    sal_Int32 nStrIndex = 0;
    sal_UCS4 cChar = rText.iterateCodePoints(&nStrIndex);
    SetSymbol(cChar, GetFont()); //force recalculation of size
}

void SmShowChar::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    Color aTextCol = rRenderContext.GetTextColor();
    Color aFillCol = rRenderContext.GetFillColor();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color aWindowTextColor(rStyleSettings.GetDialogTextColor());
    const Color aWindowColor(rStyleSettings.GetWindowColor());
    rRenderContext.SetTextColor(aWindowTextColor);
    rRenderContext.SetFillColor(aWindowColor);

    Size aSize(GetOutputSizePixel());
    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), aSize));

    OUString aText(GetText());
    if (!aText.isEmpty())
    {
        vcl::Font aFont(m_aFont);
        aFont.SetAlignment(ALIGN_TOP);
        rRenderContext.SetFont(aFont);

        Size aTextSize(rRenderContext.GetTextWidth(aText), rRenderContext.GetTextHeight());

        rRenderContext.DrawText(Point((aSize.Width()  - aTextSize.Width()) / 2,
                                      (aSize.Height() - aTextSize.Height()) / 2), aText);
    }

    rRenderContext.SetTextColor(aTextCol);
    rRenderContext.SetFillColor(aFillCol);
}

void SmShowChar::SetSymbol( const SmSym *pSym )
{
    if (pSym)
        SetSymbol( pSym->GetCharacter(), pSym->GetFace() );
}


void SmShowChar::SetSymbol( sal_UCS4 cChar, const vcl::Font &rFont )
{
    vcl::Font aFont( rFont );
    Size aSize(GetOutputSizePixel());
    aFont.SetFontSize(Size(0, aSize.Height() - aSize.Height() / 3));
    aFont.SetAlignment(ALIGN_BASELINE);
    SetFont(aFont);

    OUString aText(&cChar, 1);
    SetText( aText );

    Invalidate();
}

void SmSymDefineDialog::FillSymbols(weld::ComboBox& rComboBox, bool bDeleteText)
{
    assert((&rComboBox == m_xOldSymbols.get() || &rComboBox == m_xSymbols.get()) && "Sm : wrong ComboBox");

    rComboBox.clear();
    if (bDeleteText)
        rComboBox.set_entry_text(OUString());

    weld::ComboBox& rBox = &rComboBox == m_xOldSymbols.get() ? *m_xOldSymbolSets : *m_xSymbolSets;
    SymbolPtrVec_t aSymSet(m_aSymbolMgrCopy.GetSymbolSet(rBox.get_active_text()));
    for (const SmSym* i : aSymSet)
        rComboBox.append_text(i->GetName());
}

void SmSymDefineDialog::FillSymbolSets(weld::ComboBox& rComboBox, bool bDeleteText)
{
    assert((&rComboBox == m_xOldSymbolSets.get() || &rComboBox == m_xSymbolSets.get()) && "Sm : wrong ComboBox");

    rComboBox.clear();
    if (bDeleteText)
        rComboBox.set_entry_text(OUString());

    const std::set< OUString >  aSymbolSetNames( m_aSymbolMgrCopy.GetSymbolSetNames() );
    for (const auto& rSymbolSetName : aSymbolSetNames)
        rComboBox.append_text(rSymbolSetName);
}

void SmSymDefineDialog::FillFonts()
{
    m_xFonts->clear();
    m_xFonts->set_active(-1);

    // Include all fonts of FontList into the font list.
    // If there are duplicates, only include one entry of each font since the style will be
    // already selected using the FontStyleBox.
    if (m_xFontList)
    {
        sal_uInt16  nCount = m_xFontList->GetFontNameCount();
        for (sal_uInt16 i = 0; i < nCount; ++i)
            m_xFonts->append_text(m_xFontList->GetFontName(i).GetFamilyName());
    }
}

void SmSymDefineDialog::FillStyles()
{
    m_xStyles->clear();
//    pStyles->SetText(OUString());

    OUString aText(m_xFonts->get_active_text());
    if (!aText.isEmpty())
    {
        // use own StyleNames
        const SmFontStyles &rStyles = GetFontStyles();
        for (sal_uInt16 i = 0; i < SmFontStyles::GetCount(); ++i)
            m_xStyles->append_text(rStyles.GetStyleName(i));

        assert(m_xStyles->get_count() > 0 && "Sm : no styles available");
        m_xStyles->set_active(0);
    }
}

SmSym* SmSymDefineDialog::GetSymbol(const weld::ComboBox& rComboBox)
{
    assert((&rComboBox == m_xOldSymbols.get() || &rComboBox == m_xSymbols.get()) && "Sm : wrong combobox");
    return m_aSymbolMgrCopy.GetSymbolByName(rComboBox.get_active_text());
}

IMPL_LINK(SmSymDefineDialog, OldSymbolChangeHdl, weld::ComboBox&, rComboBox, void)
{
    (void) rComboBox;
    assert(&rComboBox == m_xOldSymbols.get() && "Sm : wrong argument");
    SelectSymbol(*m_xOldSymbols, m_xOldSymbols->get_active_text(), false);
}

IMPL_LINK( SmSymDefineDialog, OldSymbolSetChangeHdl, weld::ComboBox&, rComboBox, void )
{
    (void) rComboBox;
    assert(&rComboBox == m_xOldSymbolSets.get() && "Sm : wrong argument");
    SelectSymbolSet(*m_xOldSymbolSets, m_xOldSymbolSets->get_active_text(), false);
}

IMPL_LINK(SmSymDefineDialog, ModifyHdl, weld::ComboBox&, rComboBox, void)
{
    // remember cursor position for later restoring of it
    int nStartPos, nEndPos;
    rComboBox.get_entry_selection_bounds(nStartPos, nEndPos);

    if (&rComboBox == m_xSymbols.get())
        SelectSymbol(*m_xSymbols, m_xSymbols->get_active_text(), false);
    else if (&rComboBox == m_xSymbolSets.get())
        SelectSymbolSet(*m_xSymbolSets, m_xSymbolSets->get_active_text(), false);
    else if (&rComboBox == m_xOldSymbols.get())
        // allow only names from the list
        SelectSymbol(*m_xOldSymbols, m_xOldSymbols->get_active_text(), true);
    else if (&rComboBox == m_xOldSymbolSets.get())
        // allow only names from the list
        SelectSymbolSet(*m_xOldSymbolSets, m_xOldSymbolSets->get_active_text(), true);
    else if (&rComboBox == m_xStyles.get())
        // allow only names from the list (that's the case here anyway)
        SelectStyle(m_xStyles->get_active_text(), true);
    else
        SAL_WARN("starmath", "wrong combobox argument");

    rComboBox.select_entry_region(nStartPos, nEndPos);

    UpdateButtons();
}

IMPL_LINK(SmSymDefineDialog, FontChangeHdl, weld::ComboBox&, rListBox, void)
{
    (void) rListBox;
    assert(&rListBox == m_xFonts.get() && "Sm : wrong argument");

    SelectFont(m_xFonts->get_active_text());
}

IMPL_LINK_NOARG(SmSymDefineDialog, SubsetChangeHdl, weld::ComboBox&, void)
{
    int nPos = m_xFontsSubsetLB->get_active();
    if (nPos != -1)
    {
        const Subset* pSubset = reinterpret_cast<const Subset*>(m_xFontsSubsetLB->get_active_id().toUInt64());
        if (pSubset)
        {
            m_xCharsetDisplay->SelectCharacter( pSubset->GetRangeMin() );
        }
    }
}

IMPL_LINK( SmSymDefineDialog, StyleChangeHdl, weld::ComboBox&, rComboBox, void )
{
    (void) rComboBox;
    assert(&rComboBox == m_xStyles.get() && "Sm : wrong argument");

    SelectStyle(m_xStyles->get_active_text());
}

IMPL_LINK_NOARG(SmSymDefineDialog, CharHighlightHdl, SvxShowCharSet*, void)
{
    sal_UCS4 cChar = m_xCharsetDisplay->GetSelectCharacter();

    if (m_xSubsetMap)
    {
        const Subset* pSubset = m_xSubsetMap->GetSubsetByUnicode(cChar);
        if (pSubset)
            m_xFontsSubsetLB->set_active_text(pSubset->GetName());
        else
            m_xFontsSubsetLB->set_active(-1);
    }

    m_aSymbolDisplay.SetSymbol(cChar, m_xCharsetDisplay->GetFont());

    UpdateButtons();

    // display Unicode position as symbol name while iterating over characters
    const OUString aHex(OUString::number(cChar, 16 ).toAsciiUpperCase());
    const OUString aPattern( (aHex.getLength() > 4) ? OUString("Ux000000") : OUString("Ux0000") );
    OUString aUnicodePos( aPattern.copy( 0, aPattern.getLength() - aHex.getLength() ) );
    aUnicodePos += aHex;
    m_xSymbols->set_entry_text(aUnicodePos);
    m_xSymbolName->set_label(aUnicodePos);
}

IMPL_LINK( SmSymDefineDialog, AddClickHdl, weld::Button&, rButton, void )
{
    (void) rButton;
    assert(&rButton == m_xAddBtn.get() && "Sm : wrong argument");
    assert(rButton.get_sensitive() && "Sm : requirements met ??");

    // add symbol
    const SmSym aNewSymbol(m_xSymbols->get_active_text(), m_xCharsetDisplay->GetFont(),
            m_xCharsetDisplay->GetSelectCharacter(), m_xSymbolSets->get_active_text());
    //OSL_ENSURE( m_aSymbolMgrCopy.GetSymbolByName(aTmpSymbolName) == NULL, "symbol already exists" );
    m_aSymbolMgrCopy.AddOrReplaceSymbol( aNewSymbol );

    // update display of new symbol
    m_aSymbolDisplay.SetSymbol( &aNewSymbol );
    m_xSymbolName->set_label(aNewSymbol.GetName());
    m_xSymbolSetName->set_label(aNewSymbol.GetSymbolSetName());

    // update list box entries
    FillSymbolSets(*m_xOldSymbolSets, false);
    FillSymbolSets(*m_xSymbolSets, false);
    FillSymbols(*m_xOldSymbols, false);
    FillSymbols(*m_xSymbols, false);

    UpdateButtons();
}

IMPL_LINK( SmSymDefineDialog, ChangeClickHdl, weld::Button&, rButton, void )
{
    (void) rButton;
    assert(&rButton == m_xChangeBtn.get() && "Sm : wrong argument");
    assert(m_xChangeBtn->get_sensitive() && "Sm : requirements met ??");

    // get new Sybol to use
    //! get font from symbol-disp lay since charset-display does not keep
    //! the bold attribute.
    const SmSym aNewSymbol(m_xSymbols->get_active_text(), m_xCharsetDisplay->GetFont(),
            m_xCharsetDisplay->GetSelectCharacter(), m_xSymbolSets->get_active_text());

    // remove old symbol if the name was changed then add new one
    const bool bNameChanged = m_xOldSymbols->get_active_text() != m_xSymbols->get_active_text();
    if (bNameChanged)
        m_aSymbolMgrCopy.RemoveSymbol(m_xOldSymbols->get_active_text());
    m_aSymbolMgrCopy.AddOrReplaceSymbol( aNewSymbol, true );

    // clear display for original symbol if necessary
    if (bNameChanged)
        SetOrigSymbol(nullptr, OUString());

    // update display of new symbol
    m_aSymbolDisplay.SetSymbol(&aNewSymbol);
    m_xSymbolName->set_label(aNewSymbol.GetName());
    m_xSymbolSetName->set_label(aNewSymbol.GetSymbolSetName());

    // update list box entries
    FillSymbolSets(*m_xOldSymbolSets, false);
    FillSymbolSets(*m_xSymbolSets, false);
    FillSymbols(*m_xOldSymbols, false);
    FillSymbols(*m_xSymbols, false);

    UpdateButtons();
}

IMPL_LINK(SmSymDefineDialog, DeleteClickHdl, weld::Button&, rButton, void)
{
    (void) rButton;
    assert(&rButton == m_xDeleteBtn.get() && "Sm : wrong argument");
    assert(m_xDeleteBtn->get_sensitive() && "Sm : requirements met ??");

    if (m_xOrigSymbol)
    {
        m_aSymbolMgrCopy.RemoveSymbol(m_xOrigSymbol->GetName());

        // clear display for original symbol
        SetOrigSymbol(nullptr, OUString());

        // update list box entries
        FillSymbolSets(*m_xOldSymbolSets, false);
        FillSymbolSets(*m_xSymbolSets,    false);
        FillSymbols(*m_xOldSymbols ,false);
        FillSymbols(*m_xSymbols    ,false);
    }

    UpdateButtons();
}

void SmSymDefineDialog::UpdateButtons()
{
    bool  bAdd    = false,
          bChange = false,
          bDelete = false;
    OUString aTmpSymbolName(m_xSymbols->get_active_text()),
             aTmpSymbolSetName(m_xSymbolSets->get_active_text());

    if (!aTmpSymbolName.isEmpty() && !aTmpSymbolSetName.isEmpty())
    {
        // are all settings equal?
        //! (Font-, Style- and SymbolSet name comparison is not case sensitive)
        bool bEqual = m_xOrigSymbol
                    && aTmpSymbolSetName.equalsIgnoreAsciiCase(m_xOldSymbolSetName->get_label())
                    && aTmpSymbolName == m_xOrigSymbol->GetName()
                    && m_xFonts->get_active_text().equalsIgnoreAsciiCase(
                            m_xOrigSymbol->GetFace().GetFamilyName())
                    && m_xStyles->get_active_text().equalsIgnoreAsciiCase(
                            GetFontStyles().GetStyleName(m_xOrigSymbol->GetFace()))
                    && m_xCharsetDisplay->GetSelectCharacter() == m_xOrigSymbol->GetCharacter();

        // only add it if there isn't already a symbol with the same name
        bAdd    = m_aSymbolMgrCopy.GetSymbolByName(aTmpSymbolName) == nullptr;

        // only delete it if all settings are equal
        bDelete = bool(m_xOrigSymbol);

        // only change it if the old symbol exists and the new one is different
        bChange = m_xOrigSymbol && !bEqual;
    }

    m_xAddBtn->set_sensitive(bAdd);
    m_xChangeBtn->set_sensitive(bChange);
    m_xDeleteBtn->set_sensitive(bDelete);
}

SmSymDefineDialog::SmSymDefineDialog(weld::Window* pParent, OutputDevice *pFntListDevice, SmSymbolManager &rMgr)
    : GenericDialogController(pParent, "modules/smath/ui/symdefinedialog.ui", "EditSymbols")
    , m_xVirDev(VclPtr<VirtualDevice>::Create())
    , m_rSymbolMgr(rMgr)
    , m_xFontList(new FontList(pFntListDevice))
    , m_xOldSymbols(m_xBuilder->weld_combo_box("oldSymbols"))
    , m_xOldSymbolSets(m_xBuilder->weld_combo_box("oldSymbolSets"))
    , m_xSymbols(m_xBuilder->weld_combo_box("symbols"))
    , m_xSymbolSets(m_xBuilder->weld_combo_box("symbolSets"))
    , m_xFonts(m_xBuilder->weld_combo_box("fonts"))
    , m_xFontsSubsetLB(m_xBuilder->weld_combo_box("fontsSubsetLB"))
    , m_xStyles(m_xBuilder->weld_combo_box("styles"))
    , m_xOldSymbolName(m_xBuilder->weld_label("oldSymbolName"))
    , m_xOldSymbolSetName(m_xBuilder->weld_label("oldSymbolSetName"))
    , m_xSymbolName(m_xBuilder->weld_label("symbolName"))
    , m_xSymbolSetName(m_xBuilder->weld_label("symbolSetName"))
    , m_xAddBtn(m_xBuilder->weld_button("add"))
    , m_xChangeBtn(m_xBuilder->weld_button("modify"))
    , m_xDeleteBtn(m_xBuilder->weld_button("delete"))
    , m_xOldSymbolDisplay(new weld::CustomWeld(*m_xBuilder, "oldSymbolDisplay", m_aOldSymbolDisplay))
    , m_xSymbolDisplay(new weld::CustomWeld(*m_xBuilder, "symbolDisplay", m_aSymbolDisplay))
    , m_xCharsetDisplay(new SvxShowCharSet(m_xBuilder->weld_scrolled_window("showscroll"), m_xVirDev))
    , m_xCharsetDisplayArea(new weld::CustomWeld(*m_xBuilder, "charsetDisplay", *m_xCharsetDisplay))
{
    // auto completion is troublesome since that symbols character also gets automatically selected in the
    // display and if the user previously selected a character to define/redefine that one this is bad
    m_xOldSymbols->set_entry_completion(false);
    m_xSymbols->set_entry_completion(false);

    FillFonts();
    if (m_xFonts->get_count() > 0)
        SelectFont(m_xFonts->get_text(0));

    SetSymbolSetManager(m_rSymbolMgr);

    m_xOldSymbols->connect_changed(LINK(this, SmSymDefineDialog, OldSymbolChangeHdl));
    m_xOldSymbolSets->connect_changed(LINK(this, SmSymDefineDialog, OldSymbolSetChangeHdl));
    m_xSymbolSets->connect_changed(LINK(this, SmSymDefineDialog, ModifyHdl));
    m_xOldSymbolSets->connect_changed(LINK(this, SmSymDefineDialog, ModifyHdl));
    m_xSymbols->connect_changed(LINK(this, SmSymDefineDialog, ModifyHdl));
    m_xOldSymbols->connect_changed(LINK(this, SmSymDefineDialog, ModifyHdl));
    m_xStyles->connect_changed(LINK(this, SmSymDefineDialog, ModifyHdl));
    m_xFonts->connect_changed(LINK(this, SmSymDefineDialog, FontChangeHdl));
    m_xFontsSubsetLB->connect_changed(LINK(this, SmSymDefineDialog, SubsetChangeHdl));
    m_xStyles->connect_changed(LINK(this, SmSymDefineDialog, StyleChangeHdl));
    m_xAddBtn->connect_clicked(LINK(this, SmSymDefineDialog, AddClickHdl));
    m_xChangeBtn->connect_clicked(LINK(this, SmSymDefineDialog, ChangeClickHdl));
    m_xDeleteBtn->connect_clicked(LINK(this, SmSymDefineDialog, DeleteClickHdl));
    m_xCharsetDisplay->SetHighlightHdl( LINK( this, SmSymDefineDialog, CharHighlightHdl ) );
}

SmSymDefineDialog::~SmSymDefineDialog()
{
}

short SmSymDefineDialog::run()
{
    short nResult = GenericDialogController::run();

    // apply changes if dialog was closed by clicking OK
    if (m_aSymbolMgrCopy.IsModified() && nResult == RET_OK)
        m_rSymbolMgr = m_aSymbolMgrCopy;

    return nResult;
}

void SmSymDefineDialog::SetSymbolSetManager(const SmSymbolManager &rMgr)
{
    m_aSymbolMgrCopy = rMgr;

    // Set the modified flag of the copy to false so that
    // we can check later on if anything has been changed
    m_aSymbolMgrCopy.SetModified(false);

    FillSymbolSets(*m_xOldSymbolSets);
    if (m_xOldSymbolSets->get_count() > 0)
        SelectSymbolSet(m_xOldSymbolSets->get_text(0));
    FillSymbolSets(*m_xSymbolSets);
    if (m_xSymbolSets->get_count() > 0)
        SelectSymbolSet(m_xSymbolSets->get_text(0));
    FillSymbols(*m_xOldSymbols);
    if (m_xOldSymbols->get_count() > 0)
        SelectSymbol(m_xOldSymbols->get_text(0));
    FillSymbols(*m_xSymbols);
    if (m_xSymbols->get_count() > 0)
        SelectSymbol(m_xSymbols->get_text(0));

    UpdateButtons();
}

bool SmSymDefineDialog::SelectSymbolSet(weld::ComboBox& rComboBox,
        const OUString &rSymbolSetName, bool bDeleteText)
{
    assert((&rComboBox == m_xOldSymbolSets.get() || &rComboBox == m_xSymbolSets.get()) && "Sm : wrong ComboBox");

    // trim SymbolName (no leading and trailing blanks)
    OUString  aNormName (rSymbolSetName);
    aNormName = comphelper::string::stripStart(aNormName, ' ');
    aNormName = comphelper::string::stripEnd(aNormName, ' ');
    // and remove possible deviations within the input
    rComboBox.set_entry_text(aNormName);

    bool   bRet = false;
    int nPos = rComboBox.find_text(aNormName);

    if (nPos != -1)
    {
        rComboBox.set_active(nPos);
        bRet = true;
    }
    else if (bDeleteText)
        rComboBox.set_entry_text(OUString());

    bool  bIsOld = &rComboBox == m_xOldSymbolSets.get();

    // setting the SymbolSet name at the associated display
    weld::Label& rFT = bIsOld ? *m_xOldSymbolSetName : *m_xSymbolSetName;
    rFT.set_label(rComboBox.get_active_text());

    // set the symbol name which belongs to the SymbolSet at the associated combobox
    weld::ComboBox& rCB = bIsOld ? *m_xOldSymbols : *m_xSymbols;
    FillSymbols(rCB, false);

    // display a valid respectively no symbol when changing the SymbolSets
    if (bIsOld)
    {
        OUString aTmpOldSymbolName;
        if (m_xOldSymbols->get_count() > 0)
            aTmpOldSymbolName = m_xOldSymbols->get_text(0);
        SelectSymbol(*m_xOldSymbols, aTmpOldSymbolName, true);
    }

    UpdateButtons();

    return bRet;
}

void SmSymDefineDialog::SetOrigSymbol(const SmSym *pSymbol,
                                      const OUString &rSymbolSetName)
{
    // clear old symbol
    m_xOrigSymbol.reset();

    OUString   aSymName,
                aSymSetName;
    if (pSymbol)
    {
        // set new symbol
        m_xOrigSymbol.reset(new SmSym(*pSymbol));

        aSymName    = pSymbol->GetName();
        aSymSetName = rSymbolSetName;
        m_aOldSymbolDisplay.SetSymbol( pSymbol );
    }
    else
    {   // delete displayed symbols
        m_aOldSymbolDisplay.SetText(OUString());
        m_aOldSymbolDisplay.Invalidate();
    }
    m_xOldSymbolName->set_label(aSymName);
    m_xOldSymbolSetName->set_label(aSymSetName);
}


bool SmSymDefineDialog::SelectSymbol(weld::ComboBox& rComboBox,
        const OUString &rSymbolName, bool bDeleteText)
{
    assert((&rComboBox == m_xOldSymbols.get() || &rComboBox == m_xSymbols.get()) && "Sm : wrong ComboBox");

    // trim SymbolName (no blanks)
    OUString  aNormName = rSymbolName.replaceAll(" ", "");
    // and remove possible deviations within the input
    rComboBox.set_entry_text(aNormName);

    bool   bRet = false;
    int nPos = rComboBox.find_text(aNormName);

    bool  bIsOld = &rComboBox == m_xOldSymbols.get();

    if (nPos != -1)
    {
        rComboBox.set_active(nPos);

        if (!bIsOld)
        {
            const SmSym *pSymbol = GetSymbol(*m_xSymbols);
            if (pSymbol)
            {
                // choose font and style accordingly
                const vcl::Font &rFont = pSymbol->GetFace();
                SelectFont(rFont.GetFamilyName(), false);
                SelectStyle(GetFontStyles().GetStyleName(rFont), false);

                // Since setting the Font via the Style name of the SymbolFonts doesn't
                // work really well (e.g. it can be empty even though the font itself is
                // bold or italic) we're manually setting the Font with respect to the Symbol
                m_xCharsetDisplay->SetFont(rFont);
                m_aSymbolDisplay.SetFont(rFont);

                // select associated character
                SelectChar(pSymbol->GetCharacter());

                // since SelectChar will also set the unicode point as text in the
                // symbols box, we have to set the symbol name again to get that one displayed
                m_xSymbols->set_entry_text(pSymbol->GetName());
            }
        }

        bRet = true;
    }
    else if (bDeleteText)
        rComboBox.set_entry_text(OUString());

    if (bIsOld)
    {
        // if there's a change of the old symbol, show only the available ones, otherwise show none
        const SmSym *pOldSymbol = nullptr;
        OUString     aTmpOldSymbolSetName;
        if (nPos != COMBOBOX_ENTRY_NOTFOUND)
        {
            pOldSymbol        = m_aSymbolMgrCopy.GetSymbolByName(aNormName);
            aTmpOldSymbolSetName = m_xOldSymbolSets->get_active_text();
        }
        SetOrigSymbol(pOldSymbol, aTmpOldSymbolSetName);
    }
    else
        m_xSymbolName->set_label(rComboBox.get_active_text());

    UpdateButtons();

    return bRet;
}


void SmSymDefineDialog::SetFont(const OUString &rFontName, const OUString &rStyleName)
{
    // get Font (FontInfo) matching name and style
    FontMetric aFontMetric;
    if (m_xFontList)
        aFontMetric = m_xFontList->Get(rFontName, WEIGHT_NORMAL, ITALIC_NONE);
    SetFontStyle(rStyleName, aFontMetric);

    m_xCharsetDisplay->SetFont(aFontMetric);
    m_aSymbolDisplay.SetFont(aFontMetric);

    // update subset listbox for new font's unicode subsets
    FontCharMapRef xFontCharMap = m_xCharsetDisplay->GetFontCharMap();
    m_xSubsetMap.reset(new SubsetMap( xFontCharMap ));

    m_xFontsSubsetLB->clear();
    bool bFirst = true;
    for (auto & subset : m_xSubsetMap->GetSubsetMap())
    {
        m_xFontsSubsetLB->append(OUString::number(reinterpret_cast<sal_uInt64>(&subset)), subset.GetName());
        // subset must live at least as long as the selected font !!!
        if (bFirst)
            m_xFontsSubsetLB->set_active(0);
        bFirst = false;
    }
    if (bFirst)
        m_xFontsSubsetLB->set_active(-1);
    m_xFontsSubsetLB->set_sensitive(!bFirst);
}

bool SmSymDefineDialog::SelectFont(const OUString &rFontName, bool bApplyFont)
{
    bool   bRet = false;
    int nPos = m_xFonts->find_text(rFontName);

    if (nPos != -1)
    {
        m_xFonts->set_active(nPos);
        if (m_xStyles->get_count() > 0)
            SelectStyle(m_xStyles->get_text(0));
        if (bApplyFont)
        {
            SetFont(m_xFonts->get_active_text(), m_xStyles->get_active_text());
            m_aSymbolDisplay.SetSymbol(m_xCharsetDisplay->GetSelectCharacter(), m_xCharsetDisplay->GetFont());
        }
        bRet = true;
    }
    else
        m_xFonts->set_active(-1);
    FillStyles();

    UpdateButtons();

    return bRet;
}


bool SmSymDefineDialog::SelectStyle(const OUString &rStyleName, bool bApplyFont)
{
    bool   bRet = false;
    int nPos = m_xStyles->find_text(rStyleName);

    // if the style is not available take the first available one (if existent)
    if (nPos == -1 && m_xStyles->get_count() > 0)
        nPos = 0;

    if (nPos != -1)
    {
        m_xStyles->set_active(nPos);
        if (bApplyFont)
        {
            SetFont(m_xFonts->get_active_text(), m_xStyles->get_active_text());
            m_aSymbolDisplay.SetSymbol(m_xCharsetDisplay->GetSelectCharacter(), m_xCharsetDisplay->GetFont());
        }
        bRet = true;
    }
    else
        m_xStyles->set_entry_text(OUString());

    UpdateButtons();

    return bRet;
}

void SmSymDefineDialog::SelectChar(sal_Unicode cChar)
{
    m_xCharsetDisplay->SelectCharacter( cChar );
    m_aSymbolDisplay.SetSymbol(cChar, m_xCharsetDisplay->GetFont());

    UpdateButtons();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
