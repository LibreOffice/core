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
#include <svtools/htmlcfg.hxx>
#include <comphelper/configuration.hxx>
#include <officecfg/Office/Common.hxx>
#include "opthtml.hxx"


OfaHtmlTabPage::OfaHtmlTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/opthtmlpage.ui", "OptHtmlPage", &rSet)
    , m_xSize1NF(m_xBuilder->weld_spin_button("size1"))
    , m_xSize2NF(m_xBuilder->weld_spin_button("size2"))
    , m_xSize3NF(m_xBuilder->weld_spin_button("size3"))
    , m_xSize4NF(m_xBuilder->weld_spin_button("size4"))
    , m_xSize5NF(m_xBuilder->weld_spin_button("size5"))
    , m_xSize6NF(m_xBuilder->weld_spin_button("size6"))
    , m_xSize7NF(m_xBuilder->weld_spin_button("size7"))
    , m_xNumbersEnglishUSCB(m_xBuilder->weld_check_button("numbersenglishus"))
    , m_xUnknownTagCB(m_xBuilder->weld_check_button("unknowntag"))
    , m_xIgnoreFontNamesCB(m_xBuilder->weld_check_button("ignorefontnames"))
    , m_xStarBasicCB(m_xBuilder->weld_check_button("starbasic"))
    , m_xStarBasicWarningCB(m_xBuilder->weld_check_button("starbasicwarning"))
    , m_xPrintExtensionCB(m_xBuilder->weld_check_button("printextension"))
    , m_xSaveGrfLocalCB(m_xBuilder->weld_check_button("savegrflocal"))
    , m_xCharSetLB(new SvxTextEncodingBox(m_xBuilder->weld_combo_box("charset")))
{
    // replace placeholder with UI string from language list
    OUString aText(m_xNumbersEnglishUSCB->get_label());
    OUString aPlaceholder("%ENGLISHUSLOCALE");
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

    // initialize the characterset listbox
    m_xCharSetLB->FillWithMimeAndSelectBest();
}

OfaHtmlTabPage::~OfaHtmlTabPage()
{
}

std::unique_ptr<SfxTabPage> OfaHtmlTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                           const SfxItemSet* rAttrSet )
{
    return std::make_unique<OfaHtmlTabPage>(pPage, pController, *rAttrSet);
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

    if( m_xCharSetLB->GetSelectTextEncoding() != SvxHtmlOptions::GetTextEncoding() )
        officecfg::Office::Common::Filter::HTML::Export::Encoding::set(
            m_xCharSetLB->GetSelectTextEncoding(), xChanges );

    xChanges->commit();
    return false;
}

void OfaHtmlTabPage::Reset( const SfxItemSet* )
{
    m_xSize1NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_1::get());
    m_xSize2NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_2::get());
    m_xSize3NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_3::get());
    m_xSize4NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_4::get());
    m_xSize5NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_5::get());
    m_xSize6NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_6::get());
    m_xSize7NF->set_value(officecfg::Office::Common::Filter::HTML::Import::FontSize::Size_7::get());
    m_xNumbersEnglishUSCB->set_active(officecfg::Office::Common::Filter::HTML::Import::NumbersEnglishUS::get());
    m_xUnknownTagCB->set_active(officecfg::Office::Common::Filter::HTML::Import::UnknownTag::get());
    m_xIgnoreFontNamesCB->set_active(officecfg::Office::Common::Filter::HTML::Import::FontSetting::get());

    m_xStarBasicCB->set_active(officecfg::Office::Common::Filter::HTML::Export::Basic::get());
    m_xStarBasicWarningCB->set_active(officecfg::Office::Common::Filter::HTML::Export::Warning::get());
    m_xStarBasicWarningCB->set_sensitive(!m_xStarBasicCB->get_active());
    m_xSaveGrfLocalCB->set_active(officecfg::Office::Common::Filter::HTML::Export::LocalGraphic::get());
    m_xPrintExtensionCB->set_active(SvxHtmlOptions::IsPrintLayoutExtension());

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

    if( !SvxHtmlOptions::IsDefaultTextEncoding() &&
        m_xCharSetLB->GetSelectTextEncoding() != SvxHtmlOptions::GetTextEncoding() )
        m_xCharSetLB->SelectTextEncoding( SvxHtmlOptions::GetTextEncoding() );
}

IMPL_LINK(OfaHtmlTabPage, CheckBoxHdl_Impl, weld::Toggleable&, rBox, void)
{
    m_xStarBasicWarningCB->set_sensitive(!rBox.get_active());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
