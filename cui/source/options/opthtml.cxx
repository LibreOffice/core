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
#include "opthtml.hxx"
#include <sal/macros.h>


OfaHtmlTabPage::OfaHtmlTabPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "cui/ui/opthtmlpage.ui", "OptHtmlPage", &rSet)
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
    , m_xCharSetLB(new TextEncodingBox(m_xBuilder->weld_combo_box("charset")))
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

VclPtr<SfxTabPage> OfaHtmlTabPage::Create( TabPageParent pParent,
                                           const SfxItemSet* rAttrSet )
{
    return VclPtr<OfaHtmlTabPage>::Create(pParent, *rAttrSet);
}

bool OfaHtmlTabPage::FillItemSet( SfxItemSet* )
{
    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    if(m_xSize1NF->get_value_changed_from_saved())
        rHtmlOpt.SetFontSize(0, static_cast<sal_uInt16>(m_xSize1NF->get_value()));
    if(m_xSize2NF->get_value_changed_from_saved())
        rHtmlOpt.SetFontSize(1, static_cast<sal_uInt16>(m_xSize2NF->get_value()));
    if(m_xSize3NF->get_value_changed_from_saved())
        rHtmlOpt.SetFontSize(2, static_cast<sal_uInt16>(m_xSize3NF->get_value()));
    if(m_xSize4NF->get_value_changed_from_saved())
        rHtmlOpt.SetFontSize(3, static_cast<sal_uInt16>(m_xSize4NF->get_value()));
    if(m_xSize5NF->get_value_changed_from_saved())
        rHtmlOpt.SetFontSize(4, static_cast<sal_uInt16>(m_xSize5NF->get_value()));
    if(m_xSize6NF->get_value_changed_from_saved())
        rHtmlOpt.SetFontSize(5, static_cast<sal_uInt16>(m_xSize6NF->get_value()));
    if(m_xSize7NF->get_value_changed_from_saved())
        rHtmlOpt.SetFontSize(6, static_cast<sal_uInt16>(m_xSize7NF->get_value()));

    if(m_xNumbersEnglishUSCB->get_state_changed_from_saved())
        rHtmlOpt.SetNumbersEnglishUS(m_xNumbersEnglishUSCB->get_active());

    if(m_xUnknownTagCB->get_state_changed_from_saved())
        rHtmlOpt.SetImportUnknown(m_xUnknownTagCB->get_active());

    if(m_xIgnoreFontNamesCB->get_state_changed_from_saved())
        rHtmlOpt.SetIgnoreFontFamily(m_xIgnoreFontNamesCB->get_active());

    if(m_xStarBasicCB->get_state_changed_from_saved())
        rHtmlOpt.SetStarBasic(m_xStarBasicCB->get_active());

    if(m_xStarBasicWarningCB->get_state_changed_from_saved())
        rHtmlOpt.SetStarBasicWarning(m_xStarBasicWarningCB->get_active());

    if(m_xSaveGrfLocalCB->get_state_changed_from_saved())
        rHtmlOpt.SetSaveGraphicsLocal(m_xSaveGrfLocalCB->get_active());

    if(m_xPrintExtensionCB->get_state_changed_from_saved())
        rHtmlOpt.SetPrintLayoutExtension(m_xPrintExtensionCB->get_active());

    if( m_xCharSetLB->GetSelectTextEncoding() != rHtmlOpt.GetTextEncoding() )
        rHtmlOpt.SetTextEncoding( m_xCharSetLB->GetSelectTextEncoding() );

    return false;
}

void OfaHtmlTabPage::Reset( const SfxItemSet* )
{
    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    m_xSize1NF->set_value(rHtmlOpt.GetFontSize(0));
    m_xSize2NF->set_value(rHtmlOpt.GetFontSize(1));
    m_xSize3NF->set_value(rHtmlOpt.GetFontSize(2));
    m_xSize4NF->set_value(rHtmlOpt.GetFontSize(3));
    m_xSize5NF->set_value(rHtmlOpt.GetFontSize(4));
    m_xSize6NF->set_value(rHtmlOpt.GetFontSize(5));
    m_xSize7NF->set_value(rHtmlOpt.GetFontSize(6));
    m_xNumbersEnglishUSCB->set_active(rHtmlOpt.IsNumbersEnglishUS());
    m_xUnknownTagCB->set_active(rHtmlOpt.IsImportUnknown());
    m_xIgnoreFontNamesCB->set_active(rHtmlOpt.IsIgnoreFontFamily());

    m_xStarBasicCB->set_active(rHtmlOpt.IsStarBasic());
    m_xStarBasicWarningCB->set_active(rHtmlOpt.IsStarBasicWarning());
    m_xStarBasicWarningCB->set_sensitive(!m_xStarBasicCB->get_active());
    m_xSaveGrfLocalCB->set_active(rHtmlOpt.IsSaveGraphicsLocal());
    m_xPrintExtensionCB->set_active(rHtmlOpt.IsPrintLayoutExtension());

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

    if( !rHtmlOpt.IsDefaultTextEncoding() &&
        m_xCharSetLB->GetSelectTextEncoding() != rHtmlOpt.GetTextEncoding() )
        m_xCharSetLB->SelectTextEncoding( rHtmlOpt.GetTextEncoding() );
}

IMPL_LINK(OfaHtmlTabPage, CheckBoxHdl_Impl, weld::ToggleButton&, rBox, void)
{
    m_xStarBasicWarningCB->set_sensitive(!rBox.get_active());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
