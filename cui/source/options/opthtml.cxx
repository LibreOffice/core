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

#include <svtools/langtab.hxx>
#include <comphelper/configuration.hxx>
#include <officecfg/Office/Common.hxx>
#include "opthtml.hxx"


OfaHtmlTabPage::OfaHtmlTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/opthtmlpage.ui"_ustr, u"OptHtmlPage"_ustr, &rSet)
    , m_xSize1NF(m_xBuilder->weld_spin_button(u"size1"_ustr))
    , m_xSize1Img(m_xBuilder->weld_widget(u"locksize1"_ustr))
    , m_xSize2NF(m_xBuilder->weld_spin_button(u"size2"_ustr))
    , m_xSize2Img(m_xBuilder->weld_widget(u"locksize2"_ustr))
    , m_xSize3NF(m_xBuilder->weld_spin_button(u"size3"_ustr))
    , m_xSize3Img(m_xBuilder->weld_widget(u"locksize3"_ustr))
    , m_xSize4NF(m_xBuilder->weld_spin_button(u"size4"_ustr))
    , m_xSize4Img(m_xBuilder->weld_widget(u"locksize4"_ustr))
    , m_xSize5NF(m_xBuilder->weld_spin_button(u"size5"_ustr))
    , m_xSize5Img(m_xBuilder->weld_widget(u"locksize5"_ustr))
    , m_xSize6NF(m_xBuilder->weld_spin_button(u"size6"_ustr))
    , m_xSize6Img(m_xBuilder->weld_widget(u"locksize6"_ustr))
    , m_xSize7NF(m_xBuilder->weld_spin_button(u"size7"_ustr))
    , m_xSize7Img(m_xBuilder->weld_widget(u"locksize7"_ustr))
    , m_xNumbersEnglishUSCB(m_xBuilder->weld_check_button(u"numbersenglishus"_ustr))
    , m_xNumbersEnglishUSImg(m_xBuilder->weld_widget(u"locknumbersenglishus"_ustr))
    , m_xUnknownTagCB(m_xBuilder->weld_check_button(u"unknowntag"_ustr))
    , m_xUnknownTagImg(m_xBuilder->weld_widget(u"lockunknowntag"_ustr))
    , m_xIgnoreFontNamesCB(m_xBuilder->weld_check_button(u"ignorefontnames"_ustr))
    , m_xIgnoreFontNamesImg(m_xBuilder->weld_widget(u"lockignorefontnames"_ustr))
    , m_xStarBasicCB(m_xBuilder->weld_check_button(u"starbasic"_ustr))
    , m_xStarBasicImg(m_xBuilder->weld_widget(u"lockstarbasic"_ustr))
    , m_xStarBasicWarningCB(m_xBuilder->weld_check_button(u"starbasicwarning"_ustr))
    , m_xStarBasicWarningImg(m_xBuilder->weld_widget(u"lockstarbasicwarning"_ustr))
    , m_xPrintExtensionCB(m_xBuilder->weld_check_button(u"printextension"_ustr))
    , m_xPrintExtensionImg(m_xBuilder->weld_widget(u"lockprintextension"_ustr))
    , m_xSaveGrfLocalCB(m_xBuilder->weld_check_button(u"savegrflocal"_ustr))
    , m_xSaveGrfLocalImg(m_xBuilder->weld_widget(u"locksavegrflocal"_ustr))
{
    // replace placeholder with UI string from language list
    OUString aText(m_xNumbersEnglishUSCB->get_label());
    OUString aPlaceholder(u"%ENGLISHUSLOCALE"_ustr);
    sal_Int32 nPos;
    if ((nPos = aText.indexOf( aPlaceholder)) != -1)
    {
        const OUString& rStr = SvtLanguageTable::GetLanguageString( LANGUAGE_ENGLISH_US);
        if (!rStr.isEmpty())
        {
            aText = aText.replaceAt( nPos, aPlaceholder.getLength(), rStr);
            m_xNumbersEnglishUSCB->set_label( aText);
        }
    }

    m_xStarBasicCB->connect_toggled(LINK(this, OfaHtmlTabPage, CheckBoxHdl_Impl));
}

OfaHtmlTabPage::~OfaHtmlTabPage()
{
}

std::unique_ptr<SfxTabPage> OfaHtmlTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                           const SfxItemSet* rAttrSet )
{
    return std::make_unique<OfaHtmlTabPage>(pPage, pController, *rAttrSet);
}

OUString OfaHtmlTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr,  u"label2"_ustr,  u"label3"_ustr,  u"size1FT"_ustr, u"size2FT"_ustr,
                          u"size3FT"_ustr, u"size4FT"_ustr, u"size5FT"_ustr, u"size6FT"_ustr, u"size7FT"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"numbersenglishus"_ustr, u"unknowntag"_ustr,     u"ignorefontnames"_ustr, u"starbasic"_ustr,
                               u"starbasicwarning"_ustr, u"printextension"_ustr, u"savegrflocal"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool OfaHtmlTabPage::FillItemSet( SfxItemSet* )
{
    std::shared_ptr<comphelper::ConfigurationChanges> xChanges = comphelper::ConfigurationChanges::create();
    if(m_xSize1NF->get_value_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_1::set(
            static_cast<sal_uInt16>(m_xSize1NF->get_value()), xChanges);
    if(m_xSize2NF->get_value_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_2::set(
            static_cast<sal_uInt16>(m_xSize2NF->get_value()), xChanges);
    if(m_xSize3NF->get_value_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_3::set(
            static_cast<sal_uInt16>(m_xSize3NF->get_value()), xChanges);
    if(m_xSize4NF->get_value_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_4::set(
            static_cast<sal_uInt16>(m_xSize4NF->get_value()), xChanges);
    if(m_xSize5NF->get_value_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_5::set(
            static_cast<sal_uInt16>(m_xSize5NF->get_value()), xChanges);
    if(m_xSize6NF->get_value_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_6::set(
            static_cast<sal_uInt16>(m_xSize6NF->get_value()), xChanges);
    if(m_xSize7NF->get_value_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_7::set(
            static_cast<sal_uInt16>(m_xSize7NF->get_value()), xChanges);

    if(m_xNumbersEnglishUSCB->get_state_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Import::NumbersEnglishUS::set(
            m_xNumbersEnglishUSCB->get_active(), xChanges);

    if(m_xUnknownTagCB->get_state_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Import::UnknownTag::set(
            m_xUnknownTagCB->get_active(), xChanges);

    if(m_xIgnoreFontNamesCB->get_state_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Import::FontSetting::set(
            m_xIgnoreFontNamesCB->get_active(), xChanges);

    if(m_xStarBasicCB->get_state_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Export::Basic::set(
            m_xStarBasicCB->get_active(), xChanges);

    if(m_xStarBasicWarningCB->get_state_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Export::Warning::set(
            m_xStarBasicWarningCB->get_active(), xChanges);

    if(m_xSaveGrfLocalCB->get_state_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Export::LocalGraphic::set(
            m_xSaveGrfLocalCB->get_active(), xChanges);

    if(m_xPrintExtensionCB->get_state_changed_from_saved())
        officecfg::Office::Common::Filter::HTML::Export::PrintLayout::set(
            m_xPrintExtensionCB->get_active(), xChanges);

    xChanges->commit();
    return false;
}

void OfaHtmlTabPage::Reset( const SfxItemSet* )
{
    m_xSize1NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_1::get());
    if (officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_1::isReadOnly())
    {
        m_xSize1NF->set_sensitive(false);
        m_xSize1Img->set_visible(true);
    }

    m_xSize2NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_2::get());
    if (officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_2::isReadOnly())
    {
        m_xSize2NF->set_sensitive(false);
        m_xSize2Img->set_visible(true);
    }

    m_xSize3NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_3::get());
    if (officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_3::isReadOnly())
    {
        m_xSize3NF->set_sensitive(false);
        m_xSize3Img->set_visible(true);
    }

    m_xSize4NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_4::get());
    if (officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_4::isReadOnly())
    {
        m_xSize4NF->set_sensitive(false);
        m_xSize4Img->set_visible(true);
    }

    m_xSize5NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_5::get());
    if (officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_5::isReadOnly())
    {
        m_xSize5NF->set_sensitive(false);
        m_xSize5Img->set_visible(true);
    }

    m_xSize6NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_6::get());
    if (officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_6::isReadOnly())
    {
        m_xSize6NF->set_sensitive(false);
        m_xSize6Img->set_visible(true);
    }

    m_xSize7NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_7::get());
    if (officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_7::isReadOnly())
    {
        m_xSize7NF->set_sensitive(false);
        m_xSize7Img->set_visible(true);
    }

    m_xNumbersEnglishUSCB->set_active(officecfg::Office::Common::Filter::HTML::Import::NumbersEnglishUS::get());
    if (officecfg::Office::Common::Filter::HTML::Import::NumbersEnglishUS::isReadOnly())
    {
        m_xNumbersEnglishUSCB->set_sensitive(false);
        m_xNumbersEnglishUSImg->set_visible(true);
    }

    m_xUnknownTagCB->set_active(officecfg::Office::Common::Filter::HTML::Import::UnknownTag::get());
    if (officecfg::Office::Common::Filter::HTML::Import::UnknownTag::isReadOnly())
    {
        m_xUnknownTagCB->set_sensitive(false);
        m_xUnknownTagImg->set_visible(true);
    }

    m_xIgnoreFontNamesCB->set_active(officecfg::Office::Common::Filter::HTML::Import::FontSetting::get());
    if (officecfg::Office::Common::Filter::HTML::Import::FontSetting::isReadOnly())
    {
        m_xIgnoreFontNamesCB->set_sensitive(false);
        m_xIgnoreFontNamesImg->set_visible(true);
    }

    m_xStarBasicCB->set_active(officecfg::Office::Common::Filter::HTML::Export::Basic::get());
    if (officecfg::Office::Common::Filter::HTML::Export::Basic::isReadOnly())
    {
        m_xStarBasicCB->set_sensitive(false);
        m_xStarBasicImg->set_visible(true);
    }

    m_xStarBasicWarningCB->set_active(officecfg::Office::Common::Filter::HTML::Export::Warning::get());
    m_xStarBasicWarningCB->set_sensitive(!m_xStarBasicCB->get_active() && !officecfg::Office::Common::Filter::HTML::Export::Warning::isReadOnly());
    m_xStarBasicWarningImg->set_visible(officecfg::Office::Common::Filter::HTML::Export::Warning::isReadOnly());

    m_xSaveGrfLocalCB->set_active(officecfg::Office::Common::Filter::HTML::Export::LocalGraphic::get());
    if (officecfg::Office::Common::Filter::HTML::Export::LocalGraphic::isReadOnly())
    {
        m_xSaveGrfLocalCB->set_sensitive(false);
        m_xSaveGrfLocalImg->set_visible(true);
    }

    m_xPrintExtensionCB->set_active(officecfg::Office::Common::Filter::HTML::Export::PrintLayout::get());
    if (officecfg::Office::Common::Filter::HTML::Export::PrintLayout::isReadOnly())
    {
        m_xPrintExtensionCB->set_sensitive(false);
        m_xPrintExtensionImg->set_visible(true);
    }

    m_xPrintExtensionCB->save_state();
    m_xStarBasicCB->save_state();
    m_xStarBasicWarningCB->save_state();
    m_xSaveGrfLocalCB->save_state();
    m_xSize1NF->save_value();
    m_xSize2NF->save_value();
    m_xSize3NF->save_value();
    m_xSize4NF->save_value();
    m_xSize5NF->save_value();
    m_xSize6NF->save_value();
    m_xSize7NF->save_value();
    m_xNumbersEnglishUSCB->save_state();
    m_xUnknownTagCB->save_state();
    m_xIgnoreFontNamesCB->save_state();
}

IMPL_LINK(OfaHtmlTabPage, CheckBoxHdl_Impl, weld::Toggleable&, rBox, void)
{
    m_xStarBasicWarningCB->set_sensitive(!rBox.get_active());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
