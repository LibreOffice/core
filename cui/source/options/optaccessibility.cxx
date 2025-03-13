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

#include "optaccessibility.hxx"
#include <strings.hrc>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <officecfg/Office/Common.hxx>
#include <unotools/resmgr.hxx>
#include <dialmgr.hxx>

namespace
{
    // <Option ID, <AccessibilityIssueID, TranslateId>>
    constexpr std::pair<OUString, std::pair<sfx::AccessibilityIssueID, TranslateId>> options_list[] {
        { u"DocumentTitle"_ustr, { sfx::AccessibilityIssueID::DOCUMENT_TITLE, STR_DOCUMENT_TITLE } },
        { u"DocumentLanguage"_ustr, { sfx::AccessibilityIssueID::DOCUMENT_LANGUAGE, STR_DOCUMENT_DEFAULT_LANGUAGE } },
        { u"DocumentBackground"_ustr, { sfx::AccessibilityIssueID::DOCUMENT_BACKGROUND, STR_AVOID_BACKGROUND_IMAGES } },
        { u"DocumentStyleLanguage"_ustr, { sfx::AccessibilityIssueID::STYLE_LANGUAGE, STR_STYLE_NO_LANGUAGE } },
        { u"LinkedGraphic"_ustr, { sfx::AccessibilityIssueID::LINKED_GRAPHIC, STR_LINKED_GRAPHIC } },
        { u"NoAltOleObj"_ustr, { sfx::AccessibilityIssueID::NO_ALT_OLE, STR_NO_ALT_OLE } },
        { u"NoAltGraphicObj"_ustr, { sfx::AccessibilityIssueID::NO_ALT_GRAPHIC, STR_NO_ALT_GRAPHIC } },
        { u"NoAltShapeObj"_ustr, { sfx::AccessibilityIssueID::NO_ALT_SHAPE, STR_NO_ALT_SHAPE } },
        { u"TableMergeSplit"_ustr, { sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT, STR_TABLE_MERGE_SPLIT } },
        { u"TextNewLines"_ustr, { sfx::AccessibilityIssueID::TEXT_NEW_LINES, STR_AVOID_NEWLINES_SPACE } },
        { u"TextSpaces"_ustr, { sfx::AccessibilityIssueID::TEXT_SPACES, STR_AVOID_SPACES_SPACE } },
        { u"TextTabs"_ustr, { sfx::AccessibilityIssueID::TEXT_TABS, STR_AVOID_TABS_FORMATTING } },
        { u"TextEmptyNums"_ustr, { sfx::AccessibilityIssueID::TEXT_EMPTY_NUM_PARA, STR_AVOID_EMPTY_NUM_PARA } },
        { u"DirectFormattings"_ustr, { sfx::AccessibilityIssueID::DIRECT_FORMATTING, STR_TEXT_FORMATTING_CONVEYS_MEAN } },
        { u"TableFormattings"_ustr, { sfx::AccessibilityIssueID::TABLE_FORMATTING, STR_TABLE_FORMATTING } },
        { u"HyperlinkText"_ustr, { sfx::AccessibilityIssueID::HYPERLINK_IS_TEXT, STR_HYPERLINK_TEXT_IS_LINK } },
        { u"HyperlinkShort"_ustr, { sfx::AccessibilityIssueID::HYPERLINK_SHORT, STR_HYPERLINK_TEXT_IS_SHORT } },
        { u"HyperlinkNoName"_ustr, { sfx::AccessibilityIssueID::HYPERLINK_NO_NAME, STR_HYPERLINK_NO_NAME } },
        { u"LinkInHeaderOrFooter"_ustr, { sfx::AccessibilityIssueID::LINK_IN_HEADER_FOOTER, STR_LINK_TEXT_IS_NOT_NESTED } },
        { u"FakeFootnotes"_ustr, { sfx::AccessibilityIssueID::FAKE_FOOTNOTE, STR_AVOID_FAKE_FOOTNOTES } },
        { u"FakeCaptions"_ustr, { sfx::AccessibilityIssueID::FAKE_CAPTION, STR_AVOID_FAKE_CAPTIONS } },
        { u"ManualNumbering"_ustr, { sfx::AccessibilityIssueID::MANUAL_NUMBERING, STR_FAKE_NUMBERING } },
        { u"TextContrast"_ustr, { sfx::AccessibilityIssueID::TEXT_CONTRAST, STR_TEXT_CONTRAST } },
        { u"TextBlinking"_ustr, { sfx::AccessibilityIssueID::TEXT_BLINKING, STR_TEXT_BLINKING } },
        { u"HeadingNotInOrder"_ustr, { sfx::AccessibilityIssueID::HEADINGS_NOT_IN_ORDER, STR_HEADINGS_NOT_IN_ORDER } },
        { u"NonInteractiveForms"_ustr, { sfx::AccessibilityIssueID::NON_INTERACTIVE_FORMS, STR_NON_INTERACTIVE_FORMS } },
        { u"Floatingtext"_ustr, { sfx::AccessibilityIssueID::FLOATING_TEXT, STR_FLOATING_TEXT } },
        { u"HeadingTable"_ustr, { sfx::AccessibilityIssueID::HEADING_IN_TABLE, STR_HEADING_IN_TABLE } },
        { u"HeadingStart"_ustr, { sfx::AccessibilityIssueID::HEADING_START, STR_HEADING_START } },
        { u"HeadingOrder"_ustr, { sfx::AccessibilityIssueID::HEADING_ORDER, STR_HEADING_ORDER } },
        { u"ContentControl"_ustr, { sfx::AccessibilityIssueID::CONTENT_CONTROL, STR_CONTENT_CONTROL_IN_HEADER } },
        { u"AvoidFootnotes"_ustr, { sfx::AccessibilityIssueID::AVOID_FOOTNOTES, STR_AVOID_FOOTNOTES } },
        { u"AvoidEndnotes"_ustr, { sfx::AccessibilityIssueID::AVOID_ENDNOTES, STR_AVOID_ENDNOTES } },
        { u"FontWorks"_ustr, { sfx::AccessibilityIssueID::FONTWORKS, STR_FONTWORKS } },
    };
}

SvxAccessibilityOptionsTabPage::SvxAccessibilityOptionsTabPage(weld::Container* pPage, weld::DialogController* pController,
    const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/optaccessibilitypage.ui"_ustr, u"OptAccessibilityPage"_ustr, &rSet)
    , m_xTextSelectionInReadonly(m_xBuilder->weld_check_button(u"textselinreadonly"_ustr))
    , m_xTextSelectionInReadonlyImg(m_xBuilder->weld_widget(u"locktextselinreadonly"_ustr))
    , m_xAnimatedGraphics(m_xBuilder->weld_combo_box(u"animatedgraphicenabled"_ustr))
    , m_xAnimatedGraphicsImg(m_xBuilder->weld_widget(u"lockanimatedgraphic"_ustr))
    , m_xAnimatedGraphicsLabel(m_xBuilder->weld_label(u"animatedgraphiclabel"_ustr))
    , m_xAnimatedOthers(m_xBuilder->weld_combo_box(u"animatedothersenabled"_ustr))
    , m_xAnimatedOthersImg(m_xBuilder->weld_widget(u"lockanimatedothers"_ustr))
    , m_xAnimatedOthersLabel(m_xBuilder->weld_label(u"animatedotherslabel"_ustr))
    , m_xAnimatedTexts(m_xBuilder->weld_combo_box(u"animatedtextenabled"_ustr))
    , m_xAnimatedTextsImg(m_xBuilder->weld_widget(u"lockanimatedtext"_ustr))
    , m_xAnimatedTextsLabel(m_xBuilder->weld_label(u"animatedtextlabel"_ustr))
    , m_xHighContrast(m_xBuilder->weld_combo_box(u"highcontrast"_ustr))
    , m_xHighContrastImg(m_xBuilder->weld_widget(u"lockhighcontrast"_ustr))
    , m_xHighContrastLabel(m_xBuilder->weld_label(u"label13"_ustr))
    , m_xAutomaticFontColor(m_xBuilder->weld_check_button(u"autofontcolor"_ustr))
    , m_xAutomaticFontColorImg(m_xBuilder->weld_widget(u"lockautofontcolor"_ustr))
    , m_xPagePreviews(m_xBuilder->weld_check_button(u"systempagepreviewcolor"_ustr))
    , m_xPagePreviewsImg(m_xBuilder->weld_widget(u"locksystempagepreviewcolor"_ustr))
    , m_xOptionsLB(m_xBuilder->weld_tree_view(u"options"_ustr))
    , m_xDefaultPB(m_xBuilder->weld_button(u"default"_ustr))
{

    m_xOptionsLB->enable_toggle_buttons(weld::ColumnToggleType::Check);

    auto pos = m_xOptionsLB->make_iterator();
    for (const auto& [compatId, a11yId] : options_list)
    {
        m_xOptionsLB->append(pos.get());
        m_xOptionsLB->set_id(*pos, compatId);
        m_xOptionsLB->set_text(*pos, CuiResId(a11yId.second), 0);
    }

    m_xDefaultPB->connect_clicked(LINK(this, SvxAccessibilityOptionsTabPage, UseAsDefaultHdl));
}

SvxAccessibilityOptionsTabPage::~SvxAccessibilityOptionsTabPage()
{
}

std::unique_ptr<SfxTabPage> SvxAccessibilityOptionsTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SvxAccessibilityOptionsTabPage>(pPage, pController, *rAttrSet);
}

OUString SvxAccessibilityOptionsTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr, u"label2"_ustr, u"label13"_ustr,
                          u"animationframelabel"_ustr, u"animatedgraphiclabel"_ustr, u"animatedtextlabel"_ustr,
                          u"animatedotherslabel"_ustr, u"label11"_ustr };

    for (const auto& label : labels)
    {
        if (const auto pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"textselinreadonly"_ustr,
                               u"autofontcolor"_ustr,     u"systempagepreviewcolor"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SvxAccessibilityOptionsTabPage::FillItemSet( SfxItemSet* )
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch( comphelper::ConfigurationChanges::create() );
    if ( !officecfg::Office::Common::Accessibility::IsForPagePreviews::isReadOnly() )
        officecfg::Office::Common::Accessibility::IsForPagePreviews::set(m_xPagePreviews->get_active(), batch);
    if ( !officecfg::Office::Common::Accessibility::AllowAnimatedGraphic::isReadOnly() )
        officecfg::Office::Common::Accessibility::AllowAnimatedGraphic::set(m_xAnimatedGraphics->get_active(), batch);
    if ( !officecfg::Office::Common::Accessibility::AllowAnimatedOthers::isReadOnly() )
        officecfg::Office::Common::Accessibility::AllowAnimatedOthers::set(m_xAnimatedOthers->get_active(), batch);
    if ( !officecfg::Office::Common::Accessibility::AllowAnimatedText::isReadOnly() )
        officecfg::Office::Common::Accessibility::AllowAnimatedText::set(m_xAnimatedTexts->get_active(), batch);
    if ( !officecfg::Office::Common::Accessibility::IsAutomaticFontColor::isReadOnly() )
        officecfg::Office::Common::Accessibility::IsAutomaticFontColor::set(m_xAutomaticFontColor->get_active(), batch);
    if ( !officecfg::Office::Common::Accessibility::IsSelectionInReadonly::isReadOnly() )
        officecfg::Office::Common::Accessibility::IsSelectionInReadonly::set(m_xTextSelectionInReadonly->get_active(), batch);
    if ( !officecfg::Office::Common::Accessibility::HighContrast::isReadOnly() )
        officecfg::Office::Common::Accessibility::HighContrast::set(m_xHighContrast->get_active(), batch);

    const int nCount = m_xOptionsLB->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        OUString option = m_xOptionsLB->get_id(i);
        const auto& aIssues = std::find_if(
            std::begin(options_list), std::end(options_list),
            [option](const auto& p) { return p.first == option; })->second;
        TriState const current = m_xOptionsLB->get_toggle(i);
        TriState saved = m_aSavedOptions[aIssues.first];
        if (current != saved)
        {
            bool const bChecked(current != TRISTATE_FALSE);
            switch (aIssues.first)
            {
                case sfx::AccessibilityIssueID::DOCUMENT_TITLE:
                    officecfg::Office::Common::AccessibilityIssues::DocumentTitle::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::DOCUMENT_LANGUAGE:
                    officecfg::Office::Common::AccessibilityIssues::DocumentLanguage::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::DOCUMENT_BACKGROUND:
                    officecfg::Office::Common::AccessibilityIssues::DocumentBackground::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::STYLE_LANGUAGE:
                    officecfg::Office::Common::AccessibilityIssues::DocumentStyleLanguage::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::LINKED_GRAPHIC:
                    officecfg::Office::Common::AccessibilityIssues::LinkedGraphic::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::NO_ALT_OLE:
                    officecfg::Office::Common::AccessibilityIssues::NoAltOleObj::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::NO_ALT_GRAPHIC:
                    officecfg::Office::Common::AccessibilityIssues::NoAltGraphicObj::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::NO_ALT_SHAPE:
                    officecfg::Office::Common::AccessibilityIssues::NoAltShapeObj::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::TEXT_NEW_LINES:
                    officecfg::Office::Common::AccessibilityIssues::TextNewLines::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::TEXT_SPACES:
                    officecfg::Office::Common::AccessibilityIssues::TextSpaces::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::TEXT_TABS:
                    officecfg::Office::Common::AccessibilityIssues::TextTabs::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::TEXT_EMPTY_NUM_PARA:
                    officecfg::Office::Common::AccessibilityIssues::TextEmptyNums::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::DIRECT_FORMATTING:
                    officecfg::Office::Common::AccessibilityIssues::DirectFormattings::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::TABLE_FORMATTING:
                    officecfg::Office::Common::AccessibilityIssues::TableFormattings::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT:
                    officecfg::Office::Common::AccessibilityIssues::TableMergeSplit::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::HYPERLINK_IS_TEXT:
                    officecfg::Office::Common::AccessibilityIssues::HyperlinkText::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::HYPERLINK_SHORT:
                    officecfg::Office::Common::AccessibilityIssues::HyperlinkShort::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::HYPERLINK_NO_NAME:
                    officecfg::Office::Common::AccessibilityIssues::HyperlinkNoName::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::LINK_IN_HEADER_FOOTER:
                    officecfg::Office::Common::AccessibilityIssues::LinkInHeaderOrFooter::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::FAKE_FOOTNOTE:
                    officecfg::Office::Common::AccessibilityIssues::FakeFootnotes::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::FAKE_CAPTION:
                    officecfg::Office::Common::AccessibilityIssues::FakeCaptions::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::MANUAL_NUMBERING:
                    officecfg::Office::Common::AccessibilityIssues::ManualNumbering::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::TEXT_CONTRAST:
                    officecfg::Office::Common::AccessibilityIssues::TextContrast::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::TEXT_BLINKING:
                    officecfg::Office::Common::AccessibilityIssues::TextBlinking::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::HEADINGS_NOT_IN_ORDER:
                    officecfg::Office::Common::AccessibilityIssues::HeadingNotInOrder::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::NON_INTERACTIVE_FORMS:
                    officecfg::Office::Common::AccessibilityIssues::NonInteractiveForms::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::FLOATING_TEXT:
                    officecfg::Office::Common::AccessibilityIssues::Floatingtext::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::HEADING_IN_TABLE:
                    officecfg::Office::Common::AccessibilityIssues::HeadingTable::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::HEADING_START:
                    officecfg::Office::Common::AccessibilityIssues::HeadingStart::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::HEADING_ORDER:
                    officecfg::Office::Common::AccessibilityIssues::HeadingOrder::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::CONTENT_CONTROL:
                    officecfg::Office::Common::AccessibilityIssues::ContentControl::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::AVOID_FOOTNOTES:
                    officecfg::Office::Common::AccessibilityIssues::AvoidFootnotes::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::AVOID_ENDNOTES:
                    officecfg::Office::Common::AccessibilityIssues::AvoidEndnotes::set(bChecked, batch);
                    break;

                case sfx::AccessibilityIssueID::FONTWORKS:
                    officecfg::Office::Common::AccessibilityIssues::FontWorks::set(bChecked, batch);
                    break;

                default:
                    break;
            }
        }
    }
    batch->commit();

    return false;
}

void SvxAccessibilityOptionsTabPage::Reset( const SfxItemSet* )
{
    m_xPagePreviews->set_active( officecfg::Office::Common::Accessibility::IsForPagePreviews::get() );
    if (officecfg::Office::Common::Accessibility::IsForPagePreviews::isReadOnly())
    {
        m_xPagePreviews->set_sensitive(false);
        m_xPagePreviewsImg->set_visible(true);
    }

    m_xAnimatedGraphics->set_active( officecfg::Office::Common::Accessibility::AllowAnimatedGraphic::get() );
    if (officecfg::Office::Common::Accessibility::AllowAnimatedGraphic::isReadOnly())
    {
        m_xAnimatedGraphics->set_sensitive(false);
        m_xAnimatedGraphicsLabel->set_sensitive(false);
        m_xAnimatedGraphicsImg->set_visible(true);
    }

    m_xAnimatedOthers->set_active( officecfg::Office::Common::Accessibility::AllowAnimatedOthers::get() );
    if (officecfg::Office::Common::Accessibility::AllowAnimatedOthers::isReadOnly())
    {
        m_xAnimatedOthers->set_sensitive(false);
        m_xAnimatedOthersLabel->set_sensitive(false);
        m_xAnimatedOthersImg->set_visible(true);
    }

    m_xAnimatedTexts->set_active( officecfg::Office::Common::Accessibility::AllowAnimatedText::get() );
    if (officecfg::Office::Common::Accessibility::AllowAnimatedText::isReadOnly())
    {
        m_xAnimatedTexts->set_sensitive(false);
        m_xAnimatedTextsLabel->set_sensitive(false);
        m_xAnimatedTextsImg->set_visible(true);
    }

    m_xAutomaticFontColor->set_active( officecfg::Office::Common::Accessibility::IsAutomaticFontColor::get() );
    if (officecfg::Office::Common::Accessibility::IsAutomaticFontColor::isReadOnly())
    {
        m_xAutomaticFontColor->set_sensitive(false);
        m_xAutomaticFontColorImg->set_visible(true);
    }

    m_xTextSelectionInReadonly->set_active( officecfg::Office::Common::Accessibility::IsSelectionInReadonly::get() );
    if (officecfg::Office::Common::Accessibility::IsSelectionInReadonly::isReadOnly())
    {
        m_xTextSelectionInReadonly->set_sensitive(false);
        m_xTextSelectionInReadonlyImg->set_visible(true);
    }

    m_xHighContrast->set_active( officecfg::Office::Common::Accessibility::HighContrast::get() );
    if (officecfg::Office::Common::Accessibility::HighContrast::isReadOnly())
    {
        m_xHighContrast->set_sensitive(false);
        m_xHighContrastLabel->set_sensitive(false);
        m_xHighContrastImg->set_visible(true);
    }

    m_aSavedOptions.clear();
    const sal_Int32 nCount = m_xOptionsLB->n_children();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        OUString option = m_xOptionsLB->get_id(i);
        const auto& aIssues = std::find_if(
            std::begin(options_list), std::end(options_list),
            [option](const auto& p) { return p.first == option; })->second;
        bool bChecked = true;
        switch (aIssues.first)
        {
            case sfx::AccessibilityIssueID::DOCUMENT_TITLE:
                bChecked = officecfg::Office::Common::AccessibilityIssues::DocumentTitle::get();
                break;

            case sfx::AccessibilityIssueID::DOCUMENT_LANGUAGE:
                bChecked = officecfg::Office::Common::AccessibilityIssues::DocumentLanguage::get();
                break;

            case sfx::AccessibilityIssueID::DOCUMENT_BACKGROUND:
                bChecked = officecfg::Office::Common::AccessibilityIssues::DocumentBackground::get();
                break;

            case sfx::AccessibilityIssueID::STYLE_LANGUAGE:
                bChecked = officecfg::Office::Common::AccessibilityIssues::DocumentStyleLanguage::get();
                break;

            case sfx::AccessibilityIssueID::LINKED_GRAPHIC:
                bChecked = officecfg::Office::Common::AccessibilityIssues::LinkedGraphic::get();
                break;

            case sfx::AccessibilityIssueID::NO_ALT_OLE:
                bChecked = officecfg::Office::Common::AccessibilityIssues::NoAltOleObj::get();
                break;

            case sfx::AccessibilityIssueID::NO_ALT_GRAPHIC:
                bChecked = officecfg::Office::Common::AccessibilityIssues::NoAltGraphicObj::get();
                break;

            case sfx::AccessibilityIssueID::NO_ALT_SHAPE:
                bChecked = officecfg::Office::Common::AccessibilityIssues::NoAltShapeObj::get();
                break;

            case sfx::AccessibilityIssueID::TEXT_NEW_LINES:
                bChecked = officecfg::Office::Common::AccessibilityIssues::TextNewLines::get();
                break;

            case sfx::AccessibilityIssueID::TEXT_SPACES:
                bChecked = officecfg::Office::Common::AccessibilityIssues::TextSpaces::get();
                break;

            case sfx::AccessibilityIssueID::TEXT_TABS:
                bChecked = officecfg::Office::Common::AccessibilityIssues::TextTabs::get();
                break;

            case sfx::AccessibilityIssueID::TEXT_EMPTY_NUM_PARA:
                bChecked = officecfg::Office::Common::AccessibilityIssues::TextEmptyNums::get();
                break;

            case sfx::AccessibilityIssueID::DIRECT_FORMATTING:
                bChecked = officecfg::Office::Common::AccessibilityIssues::DirectFormattings::get();
                break;

            case sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT:
                bChecked = officecfg::Office::Common::AccessibilityIssues::TableMergeSplit::get();
                break;

            case sfx::AccessibilityIssueID::TABLE_FORMATTING:
                bChecked = officecfg::Office::Common::AccessibilityIssues::TableFormattings::get();
                break;

            case sfx::AccessibilityIssueID::HYPERLINK_IS_TEXT:
                bChecked = officecfg::Office::Common::AccessibilityIssues::HyperlinkText::get();
                break;

            case sfx::AccessibilityIssueID::HYPERLINK_SHORT:
                bChecked = officecfg::Office::Common::AccessibilityIssues::HyperlinkShort::get();
                break;

            case sfx::AccessibilityIssueID::HYPERLINK_NO_NAME:
                bChecked = officecfg::Office::Common::AccessibilityIssues::HyperlinkNoName::get();
                break;

            case sfx::AccessibilityIssueID::LINK_IN_HEADER_FOOTER:
                bChecked = officecfg::Office::Common::AccessibilityIssues::LinkInHeaderOrFooter::get();
                break;

            case sfx::AccessibilityIssueID::FAKE_FOOTNOTE:
                bChecked = officecfg::Office::Common::AccessibilityIssues::FakeFootnotes::get();
                break;

            case sfx::AccessibilityIssueID::FAKE_CAPTION:
                bChecked = officecfg::Office::Common::AccessibilityIssues::FakeCaptions::get();
                break;

            case sfx::AccessibilityIssueID::MANUAL_NUMBERING:
                bChecked = officecfg::Office::Common::AccessibilityIssues::ManualNumbering::get();
                break;

            case sfx::AccessibilityIssueID::TEXT_CONTRAST:
                bChecked = officecfg::Office::Common::AccessibilityIssues::TextContrast::get();
                break;

            case sfx::AccessibilityIssueID::TEXT_BLINKING:
                bChecked = officecfg::Office::Common::AccessibilityIssues::TextBlinking::get();
                break;

            case sfx::AccessibilityIssueID::HEADINGS_NOT_IN_ORDER:
                bChecked = officecfg::Office::Common::AccessibilityIssues::HeadingNotInOrder::get();
                break;

            case sfx::AccessibilityIssueID::NON_INTERACTIVE_FORMS:
                bChecked = officecfg::Office::Common::AccessibilityIssues::NonInteractiveForms::get();
                break;

            case sfx::AccessibilityIssueID::FLOATING_TEXT:
                bChecked = officecfg::Office::Common::AccessibilityIssues::Floatingtext::get();
                break;

            case sfx::AccessibilityIssueID::HEADING_IN_TABLE:
                bChecked = officecfg::Office::Common::AccessibilityIssues::HeadingTable::get();
                break;

            case sfx::AccessibilityIssueID::HEADING_START:
                bChecked = officecfg::Office::Common::AccessibilityIssues::HeadingStart::get();
                break;

            case sfx::AccessibilityIssueID::HEADING_ORDER:
                bChecked = officecfg::Office::Common::AccessibilityIssues::HeadingOrder::get();
                break;

            case sfx::AccessibilityIssueID::CONTENT_CONTROL:
                bChecked = officecfg::Office::Common::AccessibilityIssues::ContentControl::get();
                break;

            case sfx::AccessibilityIssueID::AVOID_FOOTNOTES:
                bChecked = officecfg::Office::Common::AccessibilityIssues::AvoidFootnotes::get();
                break;

            case sfx::AccessibilityIssueID::AVOID_ENDNOTES:
                bChecked = officecfg::Office::Common::AccessibilityIssues::AvoidEndnotes::get();
                break;

            case sfx::AccessibilityIssueID::FONTWORKS:
                bChecked = officecfg::Office::Common::AccessibilityIssues::FontWorks::get();
                break;

            default:
                break;
        }

        TriState value = bChecked ? TRISTATE_TRUE : TRISTATE_FALSE;
        m_xOptionsLB->set_toggle(i, value);
        m_aSavedOptions[aIssues.first] = value;
    }
}

IMPL_LINK_NOARG(SvxAccessibilityOptionsTabPage, UseAsDefaultHdl, weld::Button&, void)
{
    const int nCount = m_xOptionsLB->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        m_xOptionsLB->set_toggle(i, TRISTATE_TRUE);
    }
    m_aSavedOptions.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
