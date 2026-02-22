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

/*
 This Dialog is part of GSoC 2025 "New Dialog to Edit Table Styles" project.
 The Dialog is based on the design teams proposal
 (https://design.blog.documentfoundation.org/2015/12/13/style-your-tables/)
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <editeng/adjustitem.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include <bitmaps.hlst>
#include <editeng/borderline.hxx>
#include <editeng/editids.hrc>
#include <editeng/flstitem.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/ctrltool.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svxtableitems.hxx>
#include <svx/dialog/TableStylesDlg.hxx>
#include <tools/fldunit.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/ScrolledWindow.hxx>
#include <vcl/weld/Dialog.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <vcl/weld/Toolbar.hxx>

static void lcl_GetDefaultFontHeight(OUString& sName, OUString& sSize)
{
    using namespace css;
    SfxObjectShell* pDocSh = SfxObjectShell::Current();

    if (!pDocSh)
        return;

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(pDocSh->GetModel(),
                                                                         uno::UNO_QUERY);

    if (!xStyleFamiliesSupplier.is())
        return;

    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();

    // Use CellStyles for Calc and ParagraphStyles for Writer
    if (xStyleFamilies->hasByName(u"CellStyles"_ustr))
    {
        uno::Reference<container::XNameAccess> xCellStyles;
        xStyleFamilies->getByName(u"CellStyles"_ustr) >>= xCellStyles;

        if (xCellStyles->hasByName(u"Text"_ustr))
        {
            uno::Reference<beans::XPropertySet> xProps;
            xCellStyles->getByName(u"Text"_ustr) >>= xProps;

            if (xProps.is())
            {
                OUString sFontName;
                xProps->getPropertyValue(u"CharFontName"_ustr) >>= sFontName;
                if (!sFontName.isEmpty())
                    sName = sFontName;

                float fSize = 0;
                xProps->getPropertyValue(u"CharHeight"_ustr) >>= fSize;
                if (fSize > 0)
                    sSize = OUString::number(static_cast<int>(fSize)) + u" pt"_ustr;

                return;
            }
        }
    }

    if (xStyleFamilies->hasByName(u"ParagraphStyles"_ustr))
    {
        uno::Reference<container::XNameAccess> xParaStyles;
        xStyleFamilies->getByName(u"ParagraphStyles"_ustr) >>= xParaStyles;

        if (xParaStyles->hasByName(u"Table Contents"_ustr))
        {
            uno::Reference<beans::XPropertySet> xProps;
            xParaStyles->getByName(u"Table Contents"_ustr) >>= xProps;

            if (xProps.is())
            {
                OUString sFontName;
                xProps->getPropertyValue(u"CharFontName"_ustr) >>= sFontName;
                if (!sFontName.isEmpty())
                    sName = sFontName;

                float fSize = 0;
                xProps->getPropertyValue(u"CharHeight"_ustr) >>= fSize;
                if (fSize > 0)
                    sSize = OUString::number(static_cast<int>(fSize)) + u" pt"_ustr;

                return;
            }
        }
    }
}

SvxTableStylesDlg::~SvxTableStylesDlg() = default;

SvxTableStylesDlg::SvxTableStylesDlg(weld::Window* pParent, bool bNewStyle, SvxAutoFormat& pFormat,
                                     SvxAutoFormatData& pData, bool bRTL)
    : weld::GenericDialogController(pParent, u"svx/ui/tablestylesdlg.ui"_ustr,
                                    u"TableStylesDialog"_ustr)
    , maWndPreview(bRTL)
    , mpFormat(pFormat)
    , mpOriginalData(pData)
    , mpData(*pData.MakeCopy())
    , msParentName(pData.GetParent())
    , mbNewStyle(bNewStyle)
    , mpTextStyle(m_xBuilder->weld_toolbar(u"charstyletb"_ustr))
    , mpNumberFormat(m_xBuilder->weld_toolbar(u"numbertb"_ustr))
    , mpHorAlign(m_xBuilder->weld_toolbar(u"charhoritb"_ustr))
    , mpVerAlign(m_xBuilder->weld_toolbar(u"charverttb"_ustr))
    , m_xCellPadding(m_xBuilder->weld_metric_spin_button(u"cellpaddingsb"_ustr, FieldUnit::INCH))
    , m_xLinkedWith(m_xBuilder->weld_combo_box(u"linkedwithbox"_ustr))
    , m_xBorderWidth(m_xBuilder->weld_combo_box(u"borderwidth"_ustr))
    , m_xNumberFormat(m_xBuilder->weld_container(u"number"_ustr))
    , m_xTextFont(m_xBuilder->weld_combo_box(u"charnamebox"_ustr))
    , m_xTextSize(new FontSizeBox(m_xBuilder->weld_combo_box(u"charsizebox"_ustr)))
    , m_xCellColor(new ColorListBox(m_xBuilder->weld_menu_button(u"cellcolormb"_ustr),
                                    [this] { return m_xDialog.get(); }))
    , m_xBorderColor(new ColorListBox(m_xBuilder->weld_menu_button(u"bordercolormb"_ustr),
                                      [this] { return m_xDialog.get(); }))
    , m_xTextColor(new ColorListBox(m_xBuilder->weld_menu_button(u"charcolormb"_ustr),
                                    [this] { return m_xDialog.get(); }))
    , m_xBorderStyle(new SvtLineListBox(m_xBuilder->weld_menu_button(u"borderstylemb"_ustr)))
    , m_xBorderSelector(new ValueSet(nullptr))
    , m_xBorderSelectorWin(new weld::CustomWeld(*m_xBuilder, u"borders"_ustr, *m_xBorderSelector))
    , mxWndPreview(new weld::CustomWeld(*m_xBuilder, u"previewarea"_ustr, maWndPreview))
    , m_xElementBox(m_xBuilder->weld_combo_box(u"elementbox"_ustr))
    , m_xNameEntry(m_xBuilder->weld_entry(u"basicnameentry"_ustr))
    , m_xOkayBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xResetBtn(m_xBuilder->weld_button(u"reset"_ustr))
    , m_xCancelBtn(m_xBuilder->weld_button(u"cancel"_ustr))

{
    m_xNameEntry->set_text(mpData.GetName());
    if (mpData.GetName() == "Default Style")
    {
        m_xNameEntry->set_sensitive(false);
        m_xLinkedWith->set_sensitive(false);
    }

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pFontItem;
    const FontList* pFontList = nullptr;

    pFontItem = pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST);
    pFontList = static_cast<const SvxFontListItem*>(pFontItem)->GetFontList();

    m_xTextFont->freeze();
    sal_uInt16 nFontCount = pFontList->GetFontNameCount();
    for (sal_uInt16 i = 0; i < nFontCount; ++i)
    {
        const FontMetric& rFontMetric = pFontList->GetFontName(i);
        m_xTextFont->append_text(rFontMetric.GetFamilyName());
    }
    m_xTextFont->thaw();

    OUString sFontName = "Liberation Sans", sFontSize = "12 pt";
    lcl_GetDefaultFontHeight(sFontName, sFontSize);
    m_xTextFont->set_active(m_xTextFont->find_text(sFontName));
    m_xTextSize->Fill(pFontList);
    m_xTextSize->set_active_or_entry_text(sFontSize);

    {
        m_xLinkedWith->freeze();
        OUString sFormatName = mpData.GetName();
        for (size_t i = 0; i < mpFormat.size(); i++)
        {
            OUString sName = mpFormat.GetData(i)->GetName();
            OUString sParent = mpFormat.GetData(i)->GetParent();
            if (!sFormatName.equals(sName) && !sFormatName.equals(sParent))
                m_xLinkedWith->append_text(sName);
        }
        m_xLinkedWith->thaw();
        m_xLinkedWith->set_active_text(mpData.GetParent());
    }

    m_xNameEntry->select_region(0, -1);
    m_xElementBox->set_active(0);
    m_xNumberFormat->set_visible(false);

    m_xBorderColor->SelectEntry(COL_BLACK);
    m_xCellColor->SelectEntry(COL_BLACK);
    m_xTextColor->SelectEntry(COL_BLACK);

    mxWndPreview->set_size_request(300, 200);
    maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(&mpData));

    InitLineStyles();
    InitBorderSelector();

    m_xElementBox->connect_changed(LINK(this, SvxTableStylesDlg, ElementSelHdl));
    m_xLinkedWith->connect_changed(LINK(this, SvxTableStylesDlg, LinkedWithSelHdl));
    m_xTextFont->connect_changed(LINK(this, SvxTableStylesDlg, TextFontSelHdl));
    m_xTextSize->connect_changed(LINK(this, SvxTableStylesDlg, TextSizeSelHdl));
    m_xCellColor->SetSelectHdl(LINK(this, SvxTableStylesDlg, CellColorSelHdl));
    m_xBorderColor->SetSelectHdl(LINK(this, SvxTableStylesDlg, BorderColorSelHdl));
    m_xTextColor->SetSelectHdl(LINK(this, SvxTableStylesDlg, TextColorSelHdl));
    mpTextStyle->connect_clicked(LINK(this, SvxTableStylesDlg, TextStyleHdl));
    mpHorAlign->connect_clicked(LINK(this, SvxTableStylesDlg, HorAlignHdl));
    mpVerAlign->connect_clicked(LINK(this, SvxTableStylesDlg, VerAlignHdl));
    m_xBorderWidth->connect_changed(LINK(this, SvxTableStylesDlg, BorderWidthSelHdl));
    m_xBorderStyle->SetSelectHdl(LINK(this, SvxTableStylesDlg, BorderStyleHdl));
    m_xCellPadding->connect_value_changed(LINK(this, SvxTableStylesDlg, PaddingHdl));
    m_xOkayBtn->connect_clicked(LINK(this, SvxTableStylesDlg, OkayHdl));
    m_xResetBtn->connect_clicked(LINK(this, SvxTableStylesDlg, ResetHdl));
    m_xCancelBtn->connect_clicked(LINK(this, SvxTableStylesDlg, CancelHdl));

    ElementSelHdl(*m_xElementBox);
}

void SvxTableStylesDlg::InitLineStyles()
{
    using namespace ::com::sun::star::table::BorderLineStyle;

    static struct
    {
        SvxBorderLineStyle mnStyle;
        SvtLineListBox::ColorFunc mpColor1Fn;
        SvtLineListBox::ColorFunc mpColor2Fn;
        SvtLineListBox::ColorDistFunc mpColorDistFn;
    } const aLines[]
        = { // Simple lines
            { SvxBorderLineStyle::SOLID, &sameColor, &sameColor, &sameDistColor },
            { SvxBorderLineStyle::DOTTED, &sameColor, &sameColor, &sameDistColor },
            { SvxBorderLineStyle::DASHED, &sameColor, &sameColor, &sameDistColor },
            { SvxBorderLineStyle::FINE_DASHED, &sameColor, &sameColor, &sameDistColor },
            { SvxBorderLineStyle::DASH_DOT, &sameColor, &sameColor, &sameDistColor },
            { SvxBorderLineStyle::DASH_DOT_DOT, &sameColor, &sameColor, &sameDistColor },

            // Double lines
            { SvxBorderLineStyle::DOUBLE, &sameColor, &sameColor, &sameDistColor },
            { SvxBorderLineStyle::DOUBLE_THIN, &sameColor, &sameColor, &sameDistColor },

            { SvxBorderLineStyle::EMBOSSED, &editeng::SvxBorderLine::threeDLightColor,
              &editeng::SvxBorderLine::threeDDarkColor, &sameDistColor },
            { SvxBorderLineStyle::ENGRAVED, &editeng::SvxBorderLine::threeDDarkColor,
              &editeng::SvxBorderLine::threeDLightColor, &sameDistColor },

            { SvxBorderLineStyle::OUTSET, &editeng::SvxBorderLine::lightColor,
              &editeng::SvxBorderLine::darkColor, &sameDistColor },
            { SvxBorderLineStyle::INSET, &editeng::SvxBorderLine::darkColor,
              &editeng::SvxBorderLine::lightColor, &sameDistColor }
          };

    m_xBorderStyle->SetSourceUnit(FieldUnit::TWIP);

    for (size_t i = 0; i < std::size(aLines); ++i)
    {
        m_xBorderStyle->InsertEntry(editeng::SvxBorderLine::getWidthImpl(aLines[i].mnStyle),
                                    aLines[i].mnStyle, THIN, aLines[i].mpColor1Fn,
                                    aLines[i].mpColor2Fn, aLines[i].mpColorDistFn);
    }

    m_xBorderStyle->SetWidth(THIN);
    m_xBorderStyle->SelectEntry(SvxBorderLineStyle::SOLID);
}

void SvxTableStylesDlg::InitBorderSelector()
{
    m_xBorderSelector->SetStyle(m_xBorderSelector->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER);
    m_xBorderSelector->SetColCount(4);
    m_xBorderSelector->SetLineCount(2);

    struct
    {
        OUString IconId;
        TranslateId LabelId;
    } const aBorderStyles[] = { { RID_SVXBMP_FRAME1, RID_SVXSTR_TABLE_PRESET_NONE },
                                { RID_SVXBMP_FRAME2, RID_SVXSTR_PARA_PRESET_ONLYLEFT },
                                { RID_SVXBMP_FRAME3, RID_SVXSTR_PARA_PRESET_ONLYRIGHT },
                                { RID_SVXBMP_FRAME4, RID_SVXSTR_PARA_PRESET_LEFTRIGHT },
                                { RID_SVXBMP_FRAME5, RID_SVXSTR_PARA_PRESET_ONLYTOP },
                                { RID_SVXBMP_FRAME6, RID_SVXSTR_PARA_PRESET_ONLYBOTTOM },
                                { RID_SVXBMP_FRAME7, RID_SVXSTR_PARA_PRESET_TOPBOTTOM },
                                { RID_SVXBMP_FRAME8, RID_SVXSTR_TABLE_PRESET_OUTER } };

    for (size_t i = 0; i < std::size(aBorderStyles); i++)
    {
        m_xBorderSelector->InsertItem(i + 1);
        m_xBorderSelector->SetItemImage(i + 1, Image(StockImage::Yes, aBorderStyles[i].IconId));
        m_xBorderSelector->SetItemText(i + 1, SvxResId(aBorderStyles[i].LabelId));
    }

    m_xBorderSelector->SetNoSelection();
    m_xBorderSelector->SetOptimalSize();
    m_xBorderSelector->Show();

    m_xBorderSelector->SetSelectHdl(LINK(this, SvxTableStylesDlg, BorderLayoutSelHdl));
}

SvxBoxItem SvxTableStylesDlg::GetValidBox()
{
    SvxBoxItem aBox = mpFormat.GetResolvedStyle(&mpData)->GetField(mnField)->GetBox(),
               aCurrBox = mpCurrField->GetBox();

    if (aCurrBox.GetTop() || aCurrBox.GetRight() || aCurrBox.GetBottom() || aCurrBox.GetLeft())
        return aCurrBox;
    else
        return aBox;
}

OUString SvxTableStylesDlg::GetValidFont(size_t nField)
{
    OUString sFont = mpData.GetField(nField)->GetFont().GetFamilyName();
    if (!sFont.isEmpty())
        return sFont;

    switch (nField)
    {
        case FIRST_ROW:
            return GetValidFont(ODD_ROW);
        case FIRST_COL:
            return GetValidFont(ODD_COL);
        case LAST_ROW:
        case LAST_COL:
        case EVEN_ROW:
        case EVEN_COL:
        case ODD_ROW:
        case ODD_COL:
            return GetValidFont(BODY);
        case FIRST_ROW_START_COL:
        case FIRST_ROW_END_COL:
        case FIRST_ROW_EVEN_COL:
            return GetValidFont(FIRST_ROW);
        case LAST_ROW_START_COL:
        case LAST_ROW_END_COL:
        case LAST_ROW_EVEN_COL:
            return GetValidFont(LAST_ROW);
        case BODY:
            return GetValidFont(BACKGROUND);
        default:
            return "";
    }
}

IMPL_LINK_NOARG(SvxTableStylesDlg, ElementSelHdl, weld::ComboBox&, void)
{
    mnField = m_xElementBox->get_active();
    mpCurrField = mpData.GetField(mnField);
    SvxAutoFormatDataField aField = *mpFormat.GetResolvedStyle(&mpData)->GetField(mnField);
    m_xCellColor->SelectEntry(aField.GetBackground().GetColor());

    tools::Long nWidth = THIN;
    editeng::SvxBorderLine* aBorder = nullptr;
    SvxBoxItem aBox = aField.GetBox();
    if (aBox.GetLeft() && aBox.GetRight() && aBox.GetTop() && aBox.GetBottom())
    {
        m_xBorderSelector->SelectItem(TABLE_ALL_BORDERS);
        aBorder = aBox.GetLeft();
    }
    else if (aBox.GetTop() && aBox.GetBottom())
    {
        m_xBorderSelector->SelectItem(TABLE_TOP_BOTTOM_BORDERS);
        aBorder = aBox.GetTop();
    }
    else if (aBox.GetLeft() && aBox.GetRight())
    {
        m_xBorderSelector->SelectItem(TABLE_LEFT_RIGHT_BORDERS);
        aBorder = aBox.GetLeft();
    }
    else if (aBox.GetLeft())
    {
        m_xBorderSelector->SelectItem(TABLE_LEFT_BORDER);
        aBorder = aBox.GetLeft();
    }
    else if (aBox.GetRight())
    {
        m_xBorderSelector->SelectItem(TABLE_RIGHT_BORDER);
        aBorder = aBox.GetRight();
    }
    else if (aBox.GetTop())
    {
        m_xBorderSelector->SelectItem(TABLE_TOP_BORDER);
        aBorder = aBox.GetTop();
    }
    else if (aBox.GetBottom())
    {
        m_xBorderSelector->SelectItem(TABLE_BOTTOM_BORDER);
        aBorder = aBox.GetBottom();
    }
    else
        m_xBorderSelector->SelectItem(TABLE_NO_BORDERS);

    if (aBorder)
    {
        m_xBorderColor->SelectEntry(aBorder->GetColor());
        nWidth = aBorder->GetWidth();
        m_xBorderStyle->SelectEntry(aBorder->GetBorderLineStyle());
        m_xBorderStyle->SetWidth(nWidth);
    }

    if (aBox.GetTop())
        m_xBorderColor->SelectEntry(aBox.GetTop()->GetColor());
    else if (aBox.GetRight())
        m_xBorderColor->SelectEntry(aBox.GetRight()->GetColor());
    else if (aBox.GetBottom())
        m_xBorderColor->SelectEntry(aBox.GetBottom()->GetColor());
    else if (aBox.GetLeft())
        m_xBorderColor->SelectEntry(aBox.GetLeft()->GetColor());
    else
        m_xBorderColor->SelectEntry(COL_TRANSPARENT);

    OUString sFont = GetValidFont(mnField);
    if (!sFont.isEmpty())
        m_xTextFont->set_active_text(sFont);

    m_xTextColor->SelectEntry(aField.GetColor().getColor());
    m_xCellPadding->set_value(m_xCellPadding->normalize(aBox.GetSmallestDistance()),
                              FieldUnit::TWIP);

    mpTextStyle->set_item_active("bold", aField.GetWeight().GetWeight() == WEIGHT_BOLD);
    mpTextStyle->set_item_active("italic", aField.GetPosture().GetPosture() == ITALIC_NORMAL);
    mpTextStyle->set_item_active("underline",
                                 aField.GetUnderline().GetLineStyle() == LINESTYLE_SINGLE);

    mpHorAlign->set_item_active("rightalign", false);
    mpHorAlign->set_item_active("centeralign", false);
    mpHorAlign->set_item_active("leftalign", false);
    switch (aField.GetHorJustify().GetValue())
    {
        case SvxCellHorJustify::Right:
            mpHorAlign->set_item_active("rightalign", true);
            break;
        case SvxCellHorJustify::Center:
            mpHorAlign->set_item_active("centeralign", true);
            break;
        default:
            mpHorAlign->set_item_active("leftalign", true);
            break;
    }

    mpVerAlign->set_item_active("bottomalign", false);
    mpVerAlign->set_item_active("vertcenteralign", false);
    mpVerAlign->set_item_active("topalign", false);
    switch (aField.GetVerJustify().GetValue())
    {
        case SvxCellVerJustify::Bottom:
            mpVerAlign->set_item_active("bottomalign", true);
            break;
        case SvxCellVerJustify::Center:
            mpVerAlign->set_item_active("vertcenteralign", true);
            break;
        default:
            mpVerAlign->set_item_active("topalign", true);
            break;
    }

    switch (nWidth)
    {
        case HAIRLINE:
            m_xBorderWidth->set_active(0);
            break;
        case VERY_THIN:
            m_xBorderWidth->set_active(1);
            break;
        case THIN:
            m_xBorderWidth->set_active(2);
            break;
        case THICK:
            m_xBorderWidth->set_active(4);
            break;
        case EXTRA_THICK:
            m_xBorderWidth->set_active(5);
            break;
        default:
            m_xBorderWidth->set_active(3);
            break;
    }

    if (mpCurrField->IsPropertySet(PROP_FONT))
        m_xTextFont->set_active_text(mpCurrField->GetFont().GetFamilyName());
    if (mpCurrField->IsPropertySet(PROP_HEIGHT))
    {
        // Convert twips to point
        OUString sSize = OUString::number(mpCurrField->GetHeight().GetHeight() / 20);
        m_xTextSize->set_active_or_entry_text(sSize + " pt");
    }
}

IMPL_LINK_NOARG(SvxTableStylesDlg, LinkedWithSelHdl, weld::ComboBox&, void)
{
    mpData.SetParent(m_xLinkedWith->get_active_text());
    maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(&mpData));
    ElementSelHdl(*m_xElementBox);
}

IMPL_LINK_NOARG(SvxTableStylesDlg, CellColorSelHdl, ColorListBox&, void)
{
    Color aSel = m_xCellColor->GetSelectEntryColor();
    SvxBrushItem aBrushItem = mpCurrField->GetBackground();
    aBrushItem.SetColor(aSel);
    mpCurrField->SetBackground(aBrushItem);
    mpCurrField->SetPropertyFlag(PROP_BACKGROUND);
    maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(&mpData));
}

IMPL_LINK_NOARG(SvxTableStylesDlg, BorderColorSelHdl, ColorListBox&, void)
{
    Color aSel = m_xBorderColor->GetSelectEntryColor();

    SvxBoxItem aBox = GetValidBox();
    if (aBox.GetTop())
        aBox.GetTop()->SetColor(aSel);
    if (aBox.GetBottom())
        aBox.GetBottom()->SetColor(aSel);
    if (aBox.GetLeft())
        aBox.GetLeft()->SetColor(aSel);
    if (aBox.GetRight())
        aBox.GetRight()->SetColor(aSel);

    mpCurrField->SetBox(aBox);
    mpCurrField->SetPropertyFlag(PROP_BOX);
    maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(&mpData));
}

IMPL_LINK_NOARG(SvxTableStylesDlg, TextColorSelHdl, ColorListBox&, void)
{
    Color aSel = m_xTextColor->GetSelectEntryColor();

    SvxColorItem aColorItem(aSel, SVX_TABLE_FONT_COLOR);
    mpCurrField->SetColor(aColorItem);
    mpCurrField->SetPropertyFlag(PROP_COLOR);
    maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(&mpData));
}

IMPL_LINK(SvxTableStylesDlg, TextStyleHdl, const OUString&, rStyle, void)
{
    bool bActive = mpTextStyle->get_item_active(rStyle);

    if (rStyle == "bold")
    {
        SvxWeightItem aNew(bActive ? WEIGHT_BOLD : WEIGHT_NORMAL, SVX_TABLE_FONT_WEIGHT);
        mpCurrField->SetWeight(aNew);
        mpCurrField->SetPropertyFlag(PROP_WEIGHT);
    }
    if (rStyle == "italic")
    {
        SvxPostureItem aNew(bActive ? ITALIC_NORMAL : ITALIC_NONE, SVX_TABLE_FONT_POSTURE);
        mpCurrField->SetPosture(aNew);
        mpCurrField->SetPropertyFlag(PROP_POSTURE);
    }
    if (rStyle == "underline")
    {
        SvxUnderlineItem aNew(bActive ? LINESTYLE_SINGLE : LINESTYLE_NONE,
                              SVX_TABLE_FONT_UNDERLINE);
        mpCurrField->SetUnderline(aNew);
        mpCurrField->SetPropertyFlag(PROP_UNDERLINE);
    }

    maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(&mpData));
}

IMPL_LINK(SvxTableStylesDlg, HorAlignHdl, const OUString&, rAlign, void)
{
    mpHorAlign->set_item_active("rightalign", false);
    mpHorAlign->set_item_active("centeralign", false);
    mpHorAlign->set_item_active("leftalign", false);

    SvxHorJustifyItem aNew(mpCurrField->GetHorJustify());
    SvxAdjustItem aAdjust(mpCurrField->GetAdjust());
    if (rAlign == "centeralign")
    {
        aNew.SetValue(SvxCellHorJustify::Center);
        aAdjust.SetAdjust(SvxAdjust::Center);
        mpHorAlign->set_item_active("centeralign", true);
    }
    else if (rAlign == "rightalign")
    {
        aNew.SetValue(SvxCellHorJustify::Right);
        aAdjust.SetAdjust(SvxAdjust::Right);
        mpHorAlign->set_item_active("rightalign", true);
    }
    else
    {
        aNew.SetValue(SvxCellHorJustify::Left);
        aAdjust.SetAdjust(SvxAdjust::Left);
        mpHorAlign->set_item_active("leftalign", true);
    }

    mpCurrField->SetHorJustify(aNew);
    mpCurrField->SetAdjust(aAdjust);
    mpCurrField->SetPropertyFlag(PROP_HOR_JUSTIFY);
    maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(&mpData));
}

IMPL_LINK(SvxTableStylesDlg, VerAlignHdl, const OUString&, rAlign, void)
{
    mpVerAlign->set_item_active("bottomalign", false);
    mpVerAlign->set_item_active("vertcenteralign", false);
    mpVerAlign->set_item_active("topalign", false);

    SvxVerJustifyItem aNew(mpCurrField->GetVerJustify());
    if (rAlign == "bottomalign")
    {
        aNew.SetValue(SvxCellVerJustify::Bottom);
        mpVerAlign->set_item_active("bottomalign", true);
    }
    else if (rAlign == "vertcenteralign")
    {
        aNew.SetValue(SvxCellVerJustify::Center);
        mpVerAlign->set_item_active("vertcenteralign", true);
    }
    else
    {
        aNew.SetValue(SvxCellVerJustify::Top);
        mpVerAlign->set_item_active("topalign", true);
    }

    mpCurrField->SetVerJustify(aNew);
    mpCurrField->SetPropertyFlag(PROP_VER_JUSTIFY);
}

IMPL_LINK_NOARG(SvxTableStylesDlg, BorderLayoutSelHdl, ValueSet*, void)
{
    sal_uInt16 nSelected = m_xBorderSelector->GetSelectedItemId();
    SvxBoxItem aBox = GetValidBox();
    editeng::SvxBorderLine aBorder;
    if (aBox.GetTop())
    {
        aBorder = *aBox.GetTop();
    }
    else if (aBox.GetRight())
    {
        aBorder = *aBox.GetRight();
    }
    else if (aBox.GetBottom())
    {
        aBorder = *aBox.GetBottom();
    }
    else if (aBox.GetLeft())
    {
        aBorder = *aBox.GetLeft();
    }
    else
    {
        Color aCol = m_xBorderColor->GetSelectEntryColor();
        if (aCol.IsTransparent())
            aCol = COL_BLACK;
        aBorder = editeng::SvxBorderLine(&aCol, THIN);
    }

    aBox.SetLine(nullptr, SvxBoxItemLine::TOP);
    aBox.SetLine(nullptr, SvxBoxItemLine::RIGHT);
    aBox.SetLine(nullptr, SvxBoxItemLine::BOTTOM);
    aBox.SetLine(nullptr, SvxBoxItemLine::LEFT);

    switch (nSelected)
    {
        case TABLE_NO_BORDERS:
            break;
        case TABLE_LEFT_BORDER:
            aBox.SetLine(&aBorder, SvxBoxItemLine::LEFT);
            break;
        case TABLE_RIGHT_BORDER:
            aBox.SetLine(&aBorder, SvxBoxItemLine::RIGHT);
            break;
        case TABLE_LEFT_RIGHT_BORDERS:
            aBox.SetLine(&aBorder, SvxBoxItemLine::LEFT);
            aBox.SetLine(&aBorder, SvxBoxItemLine::RIGHT);
            break;
        case TABLE_TOP_BORDER:
            aBox.SetLine(&aBorder, SvxBoxItemLine::TOP);
            break;
        case TABLE_BOTTOM_BORDER:
            aBox.SetLine(&aBorder, SvxBoxItemLine::BOTTOM);
            break;
        case TABLE_TOP_BOTTOM_BORDERS:
            aBox.SetLine(&aBorder, SvxBoxItemLine::TOP);
            aBox.SetLine(&aBorder, SvxBoxItemLine::BOTTOM);
            break;
        case TABLE_ALL_BORDERS:
            aBox.SetLine(&aBorder, SvxBoxItemLine::TOP);
            aBox.SetLine(&aBorder, SvxBoxItemLine::RIGHT);
            aBox.SetLine(&aBorder, SvxBoxItemLine::BOTTOM);
            aBox.SetLine(&aBorder, SvxBoxItemLine::LEFT);
            break;
    }

    mpCurrField->SetBox(aBox);
    m_xBorderColor->SelectEntry(aBorder.GetColor());
    mpCurrField->SetPropertyFlag(PROP_BOX);
    maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(&mpData));
}

void SvxTableStylesDlg::UpdateWidth(tools::Long nWidth)
{
    SvxBoxItem aBox = GetValidBox();
    if (aBox.GetTop())
        aBox.GetTop()->SetWidth(nWidth);
    if (aBox.GetBottom())
        aBox.GetBottom()->SetWidth(nWidth);
    if (aBox.GetLeft())
        aBox.GetLeft()->SetWidth(nWidth);
    if (aBox.GetRight())
        aBox.GetRight()->SetWidth(nWidth);

    SvxAutoFormatDataField* pCurrField = mpData.GetField(mnField);
    pCurrField->SetBox(aBox);
    pCurrField->SetPropertyFlag(PROP_BOX);
    maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(&mpData));
}

IMPL_LINK_NOARG(SvxTableStylesDlg, BorderWidthSelHdl, weld::ComboBox&, void)
{
    switch (m_xBorderWidth->get_active())
    {
        case 0:
            UpdateWidth(HAIRLINE);
            break;
        case 1:
            UpdateWidth(VERY_THIN);
            break;
        case 2:
            UpdateWidth(THIN);
            break;
        case 4:
            UpdateWidth(THICK);
            break;
        case 5:
            UpdateWidth(EXTRA_THICK);
            break;
        default:
            UpdateWidth(MEDIUM);
            break;
    }
}

IMPL_LINK_NOARG(SvxTableStylesDlg, BorderStyleHdl, SvtLineListBox&, void)
{
    SvxBoxItem aBox = GetValidBox();
    auto aBorderStyle = m_xBorderStyle->GetSelectEntryStyle();

    if (aBox.GetTop())
        aBox.GetTop()->SetBorderLineStyle(aBorderStyle);
    if (aBox.GetBottom())
        aBox.GetBottom()->SetBorderLineStyle(aBorderStyle);
    if (aBox.GetLeft())
        aBox.GetLeft()->SetBorderLineStyle(aBorderStyle);
    if (aBox.GetRight())
        aBox.GetRight()->SetBorderLineStyle(aBorderStyle);

    SvxAutoFormatDataField* pCurrField = mpData.GetField(mnField);
    pCurrField->SetBox(aBox);
    pCurrField->SetPropertyFlag(PROP_BOX);
    maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(&mpData));
}

IMPL_LINK(SvxTableStylesDlg, PaddingHdl, weld::MetricSpinButton&, rField, void)
{
    SvxBoxItem aBox = GetValidBox();
    aBox.SetAllDistances(rField.denormalize(rField.get_value(FieldUnit::TWIP)));
    mpCurrField->SetBox(aBox);
    mpCurrField->SetPropertyFlag(PROP_PADDING);
}

IMPL_LINK_NOARG(SvxTableStylesDlg, TextFontSelHdl, weld::ComboBox&, void)
{
    SvxFontItem aFont = mpCurrField->GetFont();
    aFont.SetFamilyName(m_xTextFont->get_active_text());
    mpCurrField->SetFont(aFont);
    mpCurrField->SetPropertyFlag(PROP_FONT);
    maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(&mpData));
}

IMPL_LINK_NOARG(SvxTableStylesDlg, TextSizeSelHdl, weld::ComboBox&, void)
{
    SvxFontHeightItem aFontHeight = mpCurrField->GetHeight();

    // Get the selected font size and convert to twips
    OUString sSize = m_xTextSize->get_active_text();
    if (sSize.endsWith(" pt"))
        sSize = sSize.copy(0, sSize.getLength() - 3);

    sal_uInt32 nSize = sSize.toInt32();
    if (nSize > 0)
    {
        // Convert points to twips (1 pt = 20 twips)
        sal_uInt32 nSizeTwips = nSize * 20;
        aFontHeight.SetHeight(nSizeTwips);

        mpCurrField->SetHeight(aFontHeight);
        mpCurrField->SetPropertyFlag(PROP_HEIGHT);
        maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(&mpData));
    }
}

IMPL_LINK_NOARG(SvxTableStylesDlg, OkayHdl, weld::Button&, void)
{
    OUString sName = m_xNameEntry->get_text();

    // Check if style name has changed
    if (sName != mpData.GetName()
        && (sName.indexOf('.') != -1 || sName.indexOf("-") != -1 || mpFormat.FindAutoFormat(sName)))
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
            m_xDialog.get(), VclMessageType::Error, VclButtonsType::Ok,
            SvxResId(RID_SVXSTR_INVALID_AUTOFORMAT_NAME)));
        xBox->run();

        m_xNameEntry->set_text(mpData.GetName());
        m_xNameEntry->select_region(0, -1);
    }
    else
    {
        mpOriginalData.SetName(sName);
        mpOriginalData.SetParent(mpData.GetParent());
        mpOriginalData.ResetAutoFormat(mpData);
        m_xDialog->response(RET_OK);
    }
}

IMPL_LINK_NOARG(SvxTableStylesDlg, ResetHdl, weld::Button&, void)
{
    mpData.SetParent(msParentName);
    m_xLinkedWith->set_active_text(msParentName);

    if (mbNewStyle)
        mpData.ResetAutoFormat(*mpFormat.GetData(0));
    else
        mpData.ResetAutoFormat(*mpFormat.FindAutoFormat(mpData.GetName()));

    ElementSelHdl(*m_xElementBox);
    maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(&mpData));
}

IMPL_LINK_NOARG(SvxTableStylesDlg, CancelHdl, weld::Button&, void)
{
    if (!mbNewStyle)
        mpData.ResetAutoFormat(*mpFormat.FindAutoFormat(mpData.GetName()));

    m_xDialog->response(RET_CANCEL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
