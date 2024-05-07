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

#include <unotools/moduleoptions.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Calc.hxx>
#include <officecfg/Office/Writer.hxx>
#include <officecfg/Office/Impress.hxx>
#include "optfltr.hxx"
#include <strings.hrc>
#include <dialmgr.hxx>

enum class MSFltrPg2_CheckBoxEntries {
    Math,
    Writer,
    Calc,
    Impress,
    SmartArt,
    Visio,
    PDF,
    InvalidCBEntry
};


OfaMSFilterTabPage::OfaMSFilterTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/optfltrpage.ui"_ustr, u"OptFltrPage"_ustr, &rSet)
    , m_xWBasicCodeCB(m_xBuilder->weld_check_button(u"wo_basic"_ustr))
    , m_xWBasicCodeImg(m_xBuilder->weld_widget(u"lockwo_basic"_ustr))
    , m_xWBasicWbctblCB(m_xBuilder->weld_check_button(u"wo_exec"_ustr))
    , m_xWBasicWbctblImg(m_xBuilder->weld_widget(u"lockwo_exec"_ustr))
    , m_xWBasicStgCB(m_xBuilder->weld_check_button(u"wo_saveorig"_ustr))
    , m_xWBasicStgImg(m_xBuilder->weld_widget(u"lockwo_saveorig"_ustr))
    , m_xEBasicCodeCB(m_xBuilder->weld_check_button(u"ex_basic"_ustr))
    , m_xEBasicCodeImg(m_xBuilder->weld_widget(u"lockex_basic"_ustr))
    , m_xEBasicExectblCB(m_xBuilder->weld_check_button(u"ex_exec"_ustr))
    , m_xEBasicExectblImg(m_xBuilder->weld_widget(u"lockex_exec"_ustr))
    , m_xEBasicStgCB(m_xBuilder->weld_check_button(u"ex_saveorig"_ustr))
    , m_xEBasicStgImg(m_xBuilder->weld_widget(u"lockex_saveorig"_ustr))
    , m_xPBasicCodeCB(m_xBuilder->weld_check_button(u"pp_basic"_ustr))
    , m_xPBasicCodeImg(m_xBuilder->weld_widget(u"lockpp_basic"_ustr))
    , m_xPBasicStgCB(m_xBuilder->weld_check_button(u"pp_saveorig"_ustr))
    , m_xPBasicStgImg(m_xBuilder->weld_widget(u"lockpp_saveorig"_ustr))
{
    m_xWBasicCodeCB->connect_toggled( LINK( this, OfaMSFilterTabPage, LoadWordBasicCheckHdl_Impl ) );
    m_xEBasicCodeCB->connect_toggled( LINK( this, OfaMSFilterTabPage, LoadExcelBasicCheckHdl_Impl ) );
}

OfaMSFilterTabPage::~OfaMSFilterTabPage()
{
}

IMPL_LINK_NOARG(OfaMSFilterTabPage, LoadWordBasicCheckHdl_Impl, weld::Toggleable&, void)
{
    m_xWBasicWbctblCB->set_sensitive(m_xWBasicCodeCB->get_active() && !officecfg::Office::Writer::Filter::Import::VBA::Executable::isReadOnly());
    m_xWBasicWbctblImg->set_visible(officecfg::Office::Writer::Filter::Import::VBA::Executable::isReadOnly());
}

IMPL_LINK_NOARG(OfaMSFilterTabPage, LoadExcelBasicCheckHdl_Impl, weld::Toggleable&, void)
{
    m_xEBasicExectblCB->set_sensitive(m_xEBasicCodeCB->get_active() && !officecfg::Office::Calc::Filter::Import::VBA::Executable::isReadOnly());
    m_xEBasicExectblImg->set_visible(officecfg::Office::Calc::Filter::Import::VBA::Executable::isReadOnly());
}

std::unique_ptr<SfxTabPage> OfaMSFilterTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                               const SfxItemSet* rAttrSet )
{
    return std::make_unique<OfaMSFilterTabPage>(pPage, pController, *rAttrSet);
}

OUString OfaMSFilterTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr, u"label2"_ustr, u"label3"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"wo_basic"_ustr, u"wo_exec"_ustr,     u"wo_saveorig"_ustr, u"ex_basic"_ustr,
                               u"ex_exec"_ustr,  u"ex_saveorig"_ustr, u"pp_basic"_ustr,    u"pp_saveorig"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool OfaMSFilterTabPage::FillItemSet( SfxItemSet* )
{
    auto batch = comphelper::ConfigurationChanges::create();

    if( m_xWBasicCodeCB->get_state_changed_from_saved() )
        officecfg::Office::Writer::Filter::Import::VBA::Load::set(m_xWBasicCodeCB->get_active(), batch);
    if( m_xWBasicWbctblCB->get_state_changed_from_saved() )
        officecfg::Office::Writer::Filter::Import::VBA::Executable::set(m_xWBasicWbctblCB->get_active(), batch);
    if( m_xWBasicStgCB->get_state_changed_from_saved() )
        officecfg::Office::Writer::Filter::Import::VBA::Save::set(m_xWBasicStgCB->get_active(), batch);

    if( m_xEBasicCodeCB->get_state_changed_from_saved())
        officecfg::Office::Calc::Filter::Import::VBA::Load::set(m_xEBasicCodeCB->get_active(), batch);
    if( m_xEBasicExectblCB->get_state_changed_from_saved())
        officecfg::Office::Calc::Filter::Import::VBA::Executable::set(m_xEBasicExectblCB->get_active(), batch);
    if( m_xEBasicStgCB->get_state_changed_from_saved())
        officecfg::Office::Calc::Filter::Import::VBA::Save::set(m_xEBasicStgCB->get_active(), batch);

    if( m_xPBasicCodeCB->get_state_changed_from_saved())
        officecfg::Office::Impress::Filter::Import::VBA::Load::set(m_xPBasicCodeCB->get_active(), batch);
    if( m_xPBasicStgCB->get_state_changed_from_saved())
        officecfg::Office::Impress::Filter::Import::VBA::Save::set(m_xPBasicStgCB->get_active(), batch);

    batch->commit();

    return false;
}

void OfaMSFilterTabPage::Reset( const SfxItemSet* )
{
    m_xWBasicCodeCB->set_active(officecfg::Office::Writer::Filter::Import::VBA::Load::get());
    m_xWBasicCodeCB->set_sensitive(!officecfg::Office::Writer::Filter::Import::VBA::Load::isReadOnly());
    m_xWBasicCodeImg->set_visible(officecfg::Office::Writer::Filter::Import::VBA::Load::isReadOnly());
    m_xWBasicCodeCB->save_state();
    m_xWBasicWbctblCB->set_active(officecfg::Office::Writer::Filter::Import::VBA::Executable::get());
    m_xWBasicWbctblCB->set_sensitive(!officecfg::Office::Writer::Filter::Import::VBA::Executable::isReadOnly());
    m_xWBasicWbctblImg->set_visible(officecfg::Office::Writer::Filter::Import::VBA::Executable::isReadOnly());
    m_xWBasicWbctblCB->save_state();
    m_xWBasicStgCB->set_active(officecfg::Office::Writer::Filter::Import::VBA::Save::get());
    m_xWBasicStgCB->set_sensitive(!officecfg::Office::Writer::Filter::Import::VBA::Save::isReadOnly());
    m_xWBasicStgImg->set_visible(officecfg::Office::Writer::Filter::Import::VBA::Save::isReadOnly());
    m_xWBasicStgCB->save_state();
    LoadWordBasicCheckHdl_Impl( *m_xWBasicCodeCB );

    m_xEBasicCodeCB->set_active(officecfg::Office::Calc::Filter::Import::VBA::Load::get());
    m_xEBasicCodeCB->set_sensitive(!officecfg::Office::Calc::Filter::Import::VBA::Load::isReadOnly());
    m_xEBasicCodeImg->set_visible(officecfg::Office::Calc::Filter::Import::VBA::Load::isReadOnly());
    m_xEBasicCodeCB->save_state();
    m_xEBasicExectblCB->set_active(officecfg::Office::Calc::Filter::Import::VBA::Executable::get());
    m_xEBasicExectblCB->set_sensitive(!officecfg::Office::Calc::Filter::Import::VBA::Executable::isReadOnly());
    m_xEBasicExectblImg->set_visible(officecfg::Office::Calc::Filter::Import::VBA::Executable::isReadOnly());
    m_xEBasicExectblCB->save_state();
    m_xEBasicStgCB->set_active(officecfg::Office::Calc::Filter::Import::VBA::Save::get());
    m_xEBasicStgCB->set_sensitive(!officecfg::Office::Calc::Filter::Import::VBA::Save::isReadOnly());
    m_xEBasicStgImg->set_visible(officecfg::Office::Calc::Filter::Import::VBA::Save::isReadOnly());
    m_xEBasicStgCB->save_state();
    LoadExcelBasicCheckHdl_Impl( *m_xEBasicCodeCB );

    m_xPBasicCodeCB->set_active(officecfg::Office::Impress::Filter::Import::VBA::Load::get());
    m_xPBasicCodeCB->set_sensitive(!officecfg::Office::Impress::Filter::Import::VBA::Load::isReadOnly());
    m_xPBasicCodeImg->set_visible(officecfg::Office::Impress::Filter::Import::VBA::Load::isReadOnly());
    m_xPBasicCodeCB->save_state();
    m_xPBasicStgCB->set_active(officecfg::Office::Impress::Filter::Import::VBA::Save::get());
    m_xPBasicStgCB->set_sensitive(!officecfg::Office::Impress::Filter::Import::VBA::Save::isReadOnly());
    m_xPBasicStgImg->set_visible(officecfg::Office::Impress::Filter::Import::VBA::Save::isReadOnly());
    m_xPBasicStgCB->save_state();
}

OfaMSFilterTabPage2::OfaMSFilterTabPage2(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/optfltrembedpage.ui"_ustr, u"OptFilterPage"_ustr, &rSet)
    , sChgToFromMath(CuiResId(RID_CUISTR_CHG_MATH))
    , sChgToFromWriter(CuiResId(RID_CUISTR_CHG_WRITER))
    , sChgToFromCalc(CuiResId(RID_CUISTR_CHG_CALC))
    , sChgToFromImpress(CuiResId(RID_CUISTR_CHG_IMPRESS))
    , sChgToFromSmartArt(CuiResId(RID_CUISTR_CHG_SMARTART))
    , sChgToFromVisio(CuiResId(RID_CUISTR_CHG_VISIO))
    , sChgToFromPDF(CuiResId(RID_CUISTR_CHG_PDF))
    , m_xCheckLB(m_xBuilder->weld_tree_view(u"checklbcontainer"_ustr))
    , m_xHighlightingFT(m_xBuilder->weld_label(u"label5"_ustr))
    , m_xHighlightingRB(m_xBuilder->weld_radio_button(u"highlighting"_ustr))
    , m_xShadingRB(m_xBuilder->weld_radio_button(u"shading"_ustr))
    , m_xShadingImg(m_xBuilder->weld_widget(u"lockbuttonbox1"_ustr))
    , m_xMSOLockFileCB(m_xBuilder->weld_check_button(u"mso_lockfile"_ustr))
    , m_xMSOLockFileImg(m_xBuilder->weld_widget(u"lockmso_lockfile"_ustr))
{
    std::vector<int> aWidths
    {
        m_xCheckLB->get_checkbox_column_width(),
        m_xCheckLB->get_checkbox_column_width()
    };
    m_xCheckLB->set_column_fixed_widths(aWidths);
}

OfaMSFilterTabPage2::~OfaMSFilterTabPage2()
{
}

std::unique_ptr<SfxTabPage> OfaMSFilterTabPage2::Create( weld::Container* pPage, weld::DialogController* pController,
                                                const SfxItemSet* rAttrSet )
{
    return std::make_unique<OfaMSFilterTabPage2>( pPage, pController, *rAttrSet );
}

OUString OfaMSFilterTabPage2::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr, u"label2"_ustr, u"label3"_ustr, u"label4"_ustr, u"label5"_ustr, u"label6"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString radioButton[] = { u"highlighting"_ustr, u"shading"_ustr };

    for (const auto& radio : radioButton)
    {
        if (const auto& pString = m_xBuilder->weld_radio_button(radio))
            sAllStrings += pString->get_label() + " ";
    }

    sAllStrings += m_xMSOLockFileCB->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

namespace
{
struct Functions
{
    bool (*FnIs)(css::uno::Reference<css::uno::XComponentContext> const &);
    bool (*FnIsReadOnly)(css::uno::Reference<css::uno::XComponentContext> const &);
    void (*FnSet)(const bool& bFlag, const std::shared_ptr<comphelper::ConfigurationChanges>&);
    template <class reg> static constexpr Functions fromReg()
    {
        return { reg::get, reg::isReadOnly, reg::set };
    }
};
struct ChkCBoxPair
{
    MSFltrPg2_CheckBoxEntries eType;
    Functions load;
    Functions save;
};
template <class regLoad, class regSave> constexpr ChkCBoxPair Pair(MSFltrPg2_CheckBoxEntries eType)
{
    return { eType, Functions::fromReg<regLoad>(), Functions::fromReg<regSave>() };
}
template <class regLoad> constexpr ChkCBoxPair Load(MSFltrPg2_CheckBoxEntries eType)
{
    return { eType, Functions::fromReg<regLoad>(), {} };
}
constexpr ChkCBoxPair aChkArr[] = {
    Pair<officecfg::Office::Common::Filter::Microsoft::Import::MathTypeToMath,
         officecfg::Office::Common::Filter::Microsoft::Export::MathToMathType>(
        MSFltrPg2_CheckBoxEntries::Math),
    Pair<officecfg::Office::Common::Filter::Microsoft::Import::WinWordToWriter,
         officecfg::Office::Common::Filter::Microsoft::Export::WriterToWinWord>(
        MSFltrPg2_CheckBoxEntries::Writer),
    Pair<officecfg::Office::Common::Filter::Microsoft::Import::ExcelToCalc,
         officecfg::Office::Common::Filter::Microsoft::Export::CalcToExcel>(
        MSFltrPg2_CheckBoxEntries::Calc),
    Pair<officecfg::Office::Common::Filter::Microsoft::Import::PowerPointToImpress,
         officecfg::Office::Common::Filter::Microsoft::Export::ImpressToPowerPoint>(
        MSFltrPg2_CheckBoxEntries::Impress),
    Load<officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes>(
        MSFltrPg2_CheckBoxEntries::SmartArt),
    Load<officecfg::Office::Common::Filter::Microsoft::Import::VisioToDraw>(
        MSFltrPg2_CheckBoxEntries::Visio),
    Load<officecfg::Office::Common::Filter::Adobe::Import::PDFToDraw>(
        MSFltrPg2_CheckBoxEntries::PDF),
};
}

bool OfaMSFilterTabPage2::FillItemSet( SfxItemSet* )
{

    auto pBatch = comphelper::ConfigurationChanges::create();
    for (const ChkCBoxPair& rEntry : aChkArr)
    {
        // we loop through the list, alternating reading the first/second column,
        // each row appears twice in the list (except for smartart and later entries, which are
        // import only)
        int nEntry = GetEntry4Type(rEntry.eType);
        if (nEntry != -1)
        {
            bool bCheck = m_xCheckLB->get_toggle(nEntry, 0);
            if (bCheck != (rEntry.load.FnIs)(css::uno::Reference<css::uno::XComponentContext>()))
                (rEntry.load.FnSet)(bCheck, pBatch);

            if (rEntry.save.FnIs)
            {
                bCheck = m_xCheckLB->get_toggle(nEntry, 1);
                if (bCheck != (rEntry.save.FnIs)(
                        css::uno::Reference<css::uno::XComponentContext>()))
                    (rEntry.save.FnSet)(bCheck, pBatch);
            }
        }
    }

    if( m_xHighlightingRB->get_state_changed_from_saved() )
    {
        officecfg::Office::Common::Filter::Microsoft::Export::CharBackgroundToHighlighting::set(
            m_xHighlightingRB->get_active(), pBatch);
    }

    if (m_xMSOLockFileCB->get_state_changed_from_saved())
    {
        officecfg::Office::Common::Filter::Microsoft::Import::CreateMSOLockFiles::set(
            m_xMSOLockFileCB->get_active(), pBatch);
    }
    pBatch->commit();

    return true;
}

void OfaMSFilterTabPage2::Reset( const SfxItemSet* )
{
    m_xCheckLB->freeze();
    m_xCheckLB->clear();

    SvtModuleOptions aModuleOpt;

    // int the same sequence as the enums of MSFltrPg2_CheckBoxEntries
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::MATH ) )
        InsertEntry( sChgToFromMath, MSFltrPg2_CheckBoxEntries::Math );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
        InsertEntry( sChgToFromWriter, MSFltrPg2_CheckBoxEntries::Writer );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
        InsertEntry( sChgToFromCalc, MSFltrPg2_CheckBoxEntries::Calc );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
        InsertEntry( sChgToFromImpress, MSFltrPg2_CheckBoxEntries::Impress );
    InsertEntry( sChgToFromSmartArt, MSFltrPg2_CheckBoxEntries::SmartArt, false );
    if (aModuleOpt.IsModuleInstalled(SvtModuleOptions::EModule::DRAW))
    {
        InsertEntry(sChgToFromVisio, MSFltrPg2_CheckBoxEntries::Visio, false);
        InsertEntry(sChgToFromPDF, MSFltrPg2_CheckBoxEntries::PDF, false);
    }

    for (const ChkCBoxPair& rArr : aChkArr)
    {
        // we loop through the list, alternating reading the first/second column,
        // each row appears twice in the list (except for smartart and later entries, which are
        // import only)
        int nEntry = GetEntry4Type( rArr.eType );
        if (nEntry != -1)
        {
            bool bCheck = (rArr.load.FnIs)(css::uno::Reference<css::uno::XComponentContext>());
            bool bReadOnly = (rArr.load.FnIsReadOnly)(
                css::uno::Reference<css::uno::XComponentContext>());
            m_xCheckLB->set_toggle(nEntry, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE, 0);
            m_xCheckLB->set_sensitive(nEntry, !bReadOnly, 0);

            if (rArr.save.FnIs)
            {
                bCheck = (rArr.save.FnIs)(css::uno::Reference<css::uno::XComponentContext>());
                bReadOnly = (rArr.save.FnIsReadOnly)(
                    css::uno::Reference<css::uno::XComponentContext>());
                m_xCheckLB->set_toggle(nEntry, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE, 1);
                m_xCheckLB->set_sensitive(nEntry, !bReadOnly, 1);
            }
        }
    }
    m_xCheckLB->thaw();

    if (officecfg::Office::Common::Filter::Microsoft::Export::CharBackgroundToHighlighting::get())
        m_xHighlightingRB->set_active(true);
    else
        m_xShadingRB->set_active(true);

    if (officecfg::Office::Common::Filter::Microsoft::Export::CharBackgroundToHighlighting::isReadOnly())
    {
        m_xHighlightingRB->set_sensitive(false);
        m_xShadingRB->set_sensitive(false);
        m_xHighlightingFT->set_sensitive(false);
        m_xShadingImg->set_visible(true);
    }

    m_xHighlightingRB->save_state();

    m_xMSOLockFileCB->set_active(officecfg::Office::Common::Filter::Microsoft::Import::CreateMSOLockFiles::get());
    m_xMSOLockFileCB->save_state();
    m_xMSOLockFileCB->set_sensitive(!officecfg::Office::Common::Filter::Microsoft::Import::CreateMSOLockFiles::isReadOnly());
    m_xMSOLockFileImg->set_visible(officecfg::Office::Common::Filter::Microsoft::Import::CreateMSOLockFiles::isReadOnly());
}

void OfaMSFilterTabPage2::InsertEntry( const OUString& _rTxt, MSFltrPg2_CheckBoxEntries _nType )
{
    InsertEntry( _rTxt, _nType, true );
}

void OfaMSFilterTabPage2::InsertEntry( const OUString& _rTxt, MSFltrPg2_CheckBoxEntries _nType,
                                       bool saveEnabled )
{
    int nPos = m_xCheckLB->n_children();
    m_xCheckLB->append();
    m_xCheckLB->set_toggle(nPos, TRISTATE_FALSE, 0);
    if (saveEnabled)
        m_xCheckLB->set_toggle(nPos, TRISTATE_FALSE, 1);
    m_xCheckLB->set_text(nPos, _rTxt, 2);
    m_xCheckLB->set_id(nPos, OUString::number(static_cast<sal_Int32>(_nType)));
}

int OfaMSFilterTabPage2::GetEntry4Type( MSFltrPg2_CheckBoxEntries _nType ) const
{
    for (int i = 0, nEntryCount = m_xCheckLB->n_children(); i < nEntryCount; ++i)
    {
        if (_nType == static_cast<MSFltrPg2_CheckBoxEntries>(m_xCheckLB->get_id(i).toInt32()))
            return i;
    }
    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
