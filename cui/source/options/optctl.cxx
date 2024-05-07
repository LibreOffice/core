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

#include "optctl.hxx"
#include <svl/ctloptions.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>

// class SvxCTLOptionsPage -----------------------------------------------------

IMPL_LINK_NOARG(SvxCTLOptionsPage, SequenceCheckingCB_Hdl, weld::Toggleable&, void)
{
    bool bIsSequenceChecking = m_xSequenceCheckingCB->get_active();
    m_xRestrictedCB->set_sensitive( bIsSequenceChecking );
    m_xTypeReplaceCB->set_sensitive( bIsSequenceChecking );
    // #i48117#: by default restricted and type&replace have to be switched on
    if (bIsSequenceChecking)
    {
        m_xTypeReplaceCB->set_active(true);
        m_xRestrictedCB->set_active(true);
    }
}

SvxCTLOptionsPage::SvxCTLOptionsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/optctlpage.ui"_ustr, u"OptCTLPage"_ustr, &rSet)
    , m_xSequenceCheckingCB(m_xBuilder->weld_check_button(u"sequencechecking"_ustr))
    , m_xRestrictedCB(m_xBuilder->weld_check_button(u"restricted"_ustr))
    , m_xTypeReplaceCB(m_xBuilder->weld_check_button(u"typeandreplace"_ustr))
    , m_xMovementLogicalRB(m_xBuilder->weld_radio_button(u"movementlogical"_ustr))
    , m_xMovementVisualRB(m_xBuilder->weld_radio_button(u"movementvisual"_ustr))
    , m_xNumeralsLB(m_xBuilder->weld_combo_box(u"numerals"_ustr))
{
    m_xSequenceCheckingCB->connect_toggled(LINK(this, SvxCTLOptionsPage, SequenceCheckingCB_Hdl));
}

SvxCTLOptionsPage::~SvxCTLOptionsPage()
{
}

std::unique_ptr<SfxTabPage> SvxCTLOptionsPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet )
{
    return std::make_unique<SvxCTLOptionsPage>( pPage, pController, *rAttrSet );
}

OUString SvxCTLOptionsPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr, u"label2"_ustr, u"label3"_ustr, u"label4"_ustr, u"label5"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"sequencechecking"_ustr, u"restricted"_ustr, u"typeandreplace"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    OUString radioButton[] = { u"movementlogical"_ustr, u"movementvisual"_ustr };

    for (const auto& radio : radioButton)
    {
        if (const auto& pString = m_xBuilder->weld_radio_button(radio))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SvxCTLOptionsPage::FillItemSet( SfxItemSet* )
{
    bool bModified = false;
    SvtCTLOptions aCTLOptions;

    // Sequence checking
    bool bChecked = m_xSequenceCheckingCB->get_active();
    if ( m_xSequenceCheckingCB->get_state_changed_from_saved() )
    {
        aCTLOptions.SetCTLSequenceChecking( bChecked );
        bModified = true;
    }

    bChecked = m_xRestrictedCB->get_active();
    if( m_xRestrictedCB->get_state_changed_from_saved() )
    {
        aCTLOptions.SetCTLSequenceCheckingRestricted( bChecked );
        bModified = true;
    }
    bChecked = m_xTypeReplaceCB->get_active();
    if( m_xTypeReplaceCB->get_state_changed_from_saved())
    {
        aCTLOptions.SetCTLSequenceCheckingTypeAndReplace(bChecked);
        bModified = true;
    }

    bool bLogicalChecked = m_xMovementLogicalRB->get_active();
    if ( m_xMovementLogicalRB->get_state_changed_from_saved() ||
         m_xMovementVisualRB->get_state_changed_from_saved() )
    {
        SvtCTLOptions::CursorMovement eMovement =
            bLogicalChecked ? SvtCTLOptions::MOVEMENT_LOGICAL : SvtCTLOptions::MOVEMENT_VISUAL;
        aCTLOptions.SetCTLCursorMovement( eMovement );
        bModified = true;
    }

    if (m_xNumeralsLB->get_value_changed_from_saved())
    {
        const sal_Int32 nPos = m_xNumeralsLB->get_active();
        aCTLOptions.SetCTLTextNumerals( static_cast<SvtCTLOptions::TextNumerals>(nPos) );
        bModified = true;
    }

    return bModified;
}

void SvxCTLOptionsPage::Reset( const SfxItemSet* )
{
    m_xSequenceCheckingCB->set_active( SvtCTLOptions::IsCTLSequenceChecking() );
    m_xRestrictedCB->set_active( SvtCTLOptions::IsCTLSequenceCheckingRestricted() );
    m_xTypeReplaceCB->set_active( SvtCTLOptions::IsCTLSequenceCheckingTypeAndReplace() );

    SvtCTLOptions::CursorMovement eMovement = SvtCTLOptions::GetCTLCursorMovement();
    switch ( eMovement )
    {
        case SvtCTLOptions::MOVEMENT_LOGICAL :
            m_xMovementLogicalRB->set_active(true);
            break;

        case SvtCTLOptions::MOVEMENT_VISUAL :
            m_xMovementVisualRB->set_active(true);
            break;

        default:
            SAL_WARN( "cui.options", "SvxCTLOptionsPage::Reset(): invalid movement enum" );
    }

    sal_uInt16 nPos = static_cast<sal_uInt16>(SvtCTLOptions::GetCTLTextNumerals());
    DBG_ASSERT( nPos < m_xNumeralsLB->get_count(), "SvxCTLOptionsPage::Reset(): invalid numerals enum" );
    m_xNumeralsLB->set_active(nPos);

    m_xSequenceCheckingCB->save_state();
    m_xRestrictedCB->save_state();
    m_xTypeReplaceCB->save_state();
    m_xMovementLogicalRB->save_state();
    m_xMovementVisualRB->save_state();
    m_xNumeralsLB->save_value();

    bool bIsSequenceChecking = m_xSequenceCheckingCB->get_active();
    m_xRestrictedCB->set_sensitive( bIsSequenceChecking );
    m_xTypeReplaceCB->set_sensitive( bIsSequenceChecking );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
