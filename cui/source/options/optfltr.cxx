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
#include <unotools/fltrcfg.hxx>
#include <officecfg/Office/Common.hxx>
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
    : SfxTabPage(pPage, pController, "cui/ui/optfltrpage.ui", "OptFltrPage", &rSet)
    , m_xWBasicCodeCB(m_xBuilder->weld_check_button("wo_basic"))
    , m_xWBasicWbctblCB(m_xBuilder->weld_check_button("wo_exec"))
    , m_xWBasicStgCB(m_xBuilder->weld_check_button("wo_saveorig"))
    , m_xEBasicCodeCB(m_xBuilder->weld_check_button("ex_basic"))
    , m_xEBasicExectblCB(m_xBuilder->weld_check_button("ex_exec"))
    , m_xEBasicStgCB(m_xBuilder->weld_check_button("ex_saveorig"))
    , m_xPBasicCodeCB(m_xBuilder->weld_check_button("pp_basic"))
    , m_xPBasicStgCB(m_xBuilder->weld_check_button("pp_saveorig"))
{
    m_xWBasicCodeCB->connect_clicked( LINK( this, OfaMSFilterTabPage, LoadWordBasicCheckHdl_Impl ) );
    m_xEBasicCodeCB->connect_clicked( LINK( this, OfaMSFilterTabPage, LoadExcelBasicCheckHdl_Impl ) );
}

OfaMSFilterTabPage::~OfaMSFilterTabPage()
{
}

IMPL_LINK_NOARG(OfaMSFilterTabPage, LoadWordBasicCheckHdl_Impl, weld::Button&, void)
{
    m_xWBasicWbctblCB->set_sensitive(m_xWBasicCodeCB->get_active());
}

IMPL_LINK_NOARG(OfaMSFilterTabPage, LoadExcelBasicCheckHdl_Impl, weld::Button&, void)
{
    m_xEBasicExectblCB->set_sensitive(m_xEBasicCodeCB->get_active());
}

std::unique_ptr<SfxTabPage> OfaMSFilterTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                               const SfxItemSet* rAttrSet )
{
    return std::make_unique<OfaMSFilterTabPage>(pPage, pController, *rAttrSet);
}

bool OfaMSFilterTabPage::FillItemSet( SfxItemSet* )
{
    SvtFilterOptions& rOpt = SvtFilterOptions::Get();

    if( m_xWBasicCodeCB->get_state_changed_from_saved() )
        rOpt.SetLoadWordBasicCode( m_xWBasicCodeCB->get_active() );
    if( m_xWBasicWbctblCB->get_state_changed_from_saved() )
        rOpt.SetLoadWordBasicExecutable( m_xWBasicWbctblCB->get_active() );
    if( m_xWBasicStgCB->get_state_changed_from_saved() )
        rOpt.SetLoadWordBasicStorage( m_xWBasicStgCB->get_active() );

    if( m_xEBasicCodeCB->get_state_changed_from_saved())
        rOpt.SetLoadExcelBasicCode( m_xEBasicCodeCB->get_active() );
    if( m_xEBasicExectblCB->get_state_changed_from_saved())
        rOpt.SetLoadExcelBasicExecutable( m_xEBasicExectblCB->get_active() );
    if( m_xEBasicStgCB->get_state_changed_from_saved())
        rOpt.SetLoadExcelBasicStorage( m_xEBasicStgCB->get_active() );

    if( m_xPBasicCodeCB->get_state_changed_from_saved())
        rOpt.SetLoadPPointBasicCode( m_xPBasicCodeCB->get_active() );
    if( m_xPBasicStgCB->get_state_changed_from_saved())
        rOpt.SetLoadPPointBasicStorage( m_xPBasicStgCB->get_active() );

    return false;
}

void OfaMSFilterTabPage::Reset( const SfxItemSet* )
{
    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();

    m_xWBasicCodeCB->set_active( rOpt.IsLoadWordBasicCode() );
    m_xWBasicCodeCB->save_state();
    m_xWBasicWbctblCB->set_active( rOpt.IsLoadWordBasicExecutable() );
    m_xWBasicWbctblCB->save_state();
    m_xWBasicStgCB->set_active( rOpt.IsLoadWordBasicStorage() );
    m_xWBasicStgCB->save_state();
    LoadWordBasicCheckHdl_Impl( *m_xWBasicCodeCB );

    m_xEBasicCodeCB->set_active( rOpt.IsLoadExcelBasicCode() );
    m_xEBasicCodeCB->save_state();
    m_xEBasicExectblCB->set_active( rOpt.IsLoadExcelBasicExecutable() );
    m_xEBasicExectblCB->save_state();
    m_xEBasicStgCB->set_active( rOpt.IsLoadExcelBasicStorage() );
    m_xEBasicStgCB->save_state();
    LoadExcelBasicCheckHdl_Impl( *m_xEBasicCodeCB );

    m_xPBasicCodeCB->set_active( rOpt.IsLoadPPointBasicCode() );
    m_xPBasicCodeCB->save_state();
    m_xPBasicStgCB->set_active( rOpt.IsLoadPPointBasicStorage() );
    m_xPBasicStgCB->save_state();
}

OfaMSFilterTabPage2::OfaMSFilterTabPage2(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/optfltrembedpage.ui", "OptFilterPage", &rSet)
    , sChgToFromMath(CuiResId(RID_SVXSTR_CHG_MATH))
    , sChgToFromWriter(CuiResId(RID_SVXSTR_CHG_WRITER))
    , sChgToFromCalc(CuiResId(RID_SVXSTR_CHG_CALC))
    , sChgToFromImpress(CuiResId(RID_SVXSTR_CHG_IMPRESS))
    , sChgToFromSmartArt(CuiResId(RID_SVXSTR_CHG_SMARTART))
    , sChgToFromVisio(CuiResId(RID_SVXSTR_CHG_VISIO))
    , sChgToFromPDF(CuiResId(RID_SVXSTR_CHG_PDF))
    , m_xCheckLB(m_xBuilder->weld_tree_view("checklbcontainer"))
    , m_xHighlightingRB(m_xBuilder->weld_radio_button("highlighting"))
    , m_xShadingRB(m_xBuilder->weld_radio_button("shading"))
    , m_xMSOLockFileCB(m_xBuilder->weld_check_button("mso_lockfile"))
{
    std::vector<int> aWidths;
    aWidths.push_back(m_xCheckLB->get_checkbox_column_width());
    aWidths.push_back(m_xCheckLB->get_checkbox_column_width());
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

bool OfaMSFilterTabPage2::FillItemSet( SfxItemSet* )
{
    SvtFilterOptions& rOpt = SvtFilterOptions::Get();

    static struct ChkCBoxEntries{
        MSFltrPg2_CheckBoxEntries eType;
        bool (SvtFilterOptions::*FnIs)() const;
        void (SvtFilterOptions::*FnSet)( bool bFlag );
    } const aChkArr[] = {
        { MSFltrPg2_CheckBoxEntries::Math,     &SvtFilterOptions::IsMathType2Math,
                        &SvtFilterOptions::SetMathType2Math },
        { MSFltrPg2_CheckBoxEntries::Math,     &SvtFilterOptions::IsMath2MathType,
                        &SvtFilterOptions::SetMath2MathType },
        { MSFltrPg2_CheckBoxEntries::Writer,   &SvtFilterOptions::IsWinWord2Writer,
                        &SvtFilterOptions::SetWinWord2Writer },
        { MSFltrPg2_CheckBoxEntries::Writer,   &SvtFilterOptions::IsWriter2WinWord,
                        &SvtFilterOptions::SetWriter2WinWord },
        { MSFltrPg2_CheckBoxEntries::Calc,     &SvtFilterOptions::IsExcel2Calc,
                        &SvtFilterOptions::SetExcel2Calc },
        { MSFltrPg2_CheckBoxEntries::Calc,     &SvtFilterOptions::IsCalc2Excel,
                        &SvtFilterOptions::SetCalc2Excel },
        { MSFltrPg2_CheckBoxEntries::Impress,  &SvtFilterOptions::IsPowerPoint2Impress,
                        &SvtFilterOptions::SetPowerPoint2Impress },
        { MSFltrPg2_CheckBoxEntries::Impress,  &SvtFilterOptions::IsImpress2PowerPoint,
                        &SvtFilterOptions::SetImpress2PowerPoint },
        { MSFltrPg2_CheckBoxEntries::SmartArt,  &SvtFilterOptions::IsSmartArt2Shape,
                        &SvtFilterOptions::SetSmartArt2Shape },
        { MSFltrPg2_CheckBoxEntries::Visio, &SvtFilterOptions::IsVisio2Draw,
                        &SvtFilterOptions::SetVisio2Draw },
    };

    bool bFirstCol = true;
    for( const ChkCBoxEntries & rEntry : aChkArr )
    {
        // we loop through the list, alternating reading the first/second column,
        // each row appears twice in the list (except for smartart and later entries, which are
        // import only)
        sal_uInt16 nCol = bFirstCol ? 0 : 1;
        bFirstCol = !bFirstCol;
        int nEntry = GetEntry4Type(rEntry.eType);
        if (nEntry != -1)
        {
            bool bCheck = m_xCheckLB->get_toggle(nEntry, nCol);
            if( bCheck != (rOpt.*rEntry.FnIs)() )
                (rOpt.*rEntry.FnSet)( bCheck );
        }
        if (rEntry.eType == MSFltrPg2_CheckBoxEntries::SmartArt)
        {
            bFirstCol = !bFirstCol;
        }
    }
    int nPDFEntry = GetEntry4Type(MSFltrPg2_CheckBoxEntries::PDF);
    bool bPDFCheck = m_xCheckLB->get_toggle(nPDFEntry, 0);
    if (bPDFCheck != officecfg::Office::Common::Filter::Adobe::Import::PDFToDraw::get())
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Adobe::Import::PDFToDraw::set(bPDFCheck, pBatch);
        pBatch->commit();
    }

    if( m_xHighlightingRB->get_state_changed_from_saved() )
    {
        if (m_xHighlightingRB->get_active())
            rOpt.SetCharBackground2Highlighting();
        else
            rOpt.SetCharBackground2Shading();
    }

    if (m_xMSOLockFileCB->get_state_changed_from_saved())
    {
        rOpt.EnableMSOLockFileCreation(m_xMSOLockFileCB->get_active());
    }

    return true;
}

void OfaMSFilterTabPage2::Reset( const SfxItemSet* )
{
    SvtFilterOptions& rOpt = SvtFilterOptions::Get();

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

    static struct ChkCBoxEntries{
        MSFltrPg2_CheckBoxEntries eType;
        bool (SvtFilterOptions::*FnIs)() const;
    } const aChkArr[] = {
        { MSFltrPg2_CheckBoxEntries::Math,     &SvtFilterOptions::IsMathType2Math },
        { MSFltrPg2_CheckBoxEntries::Math,     &SvtFilterOptions::IsMath2MathType },
        { MSFltrPg2_CheckBoxEntries::Writer,   &SvtFilterOptions::IsWinWord2Writer },
        { MSFltrPg2_CheckBoxEntries::Writer,   &SvtFilterOptions::IsWriter2WinWord },
        { MSFltrPg2_CheckBoxEntries::Calc,     &SvtFilterOptions::IsExcel2Calc },
        { MSFltrPg2_CheckBoxEntries::Calc,     &SvtFilterOptions::IsCalc2Excel },
        { MSFltrPg2_CheckBoxEntries::Impress,  &SvtFilterOptions::IsPowerPoint2Impress },
        { MSFltrPg2_CheckBoxEntries::Impress,  &SvtFilterOptions::IsImpress2PowerPoint },
        { MSFltrPg2_CheckBoxEntries::SmartArt, &SvtFilterOptions::IsSmartArt2Shape },
        { MSFltrPg2_CheckBoxEntries::Visio,    &SvtFilterOptions::IsVisio2Draw },
        { MSFltrPg2_CheckBoxEntries::PDF,      nullptr },
    };

    bool bFirstCol = true;
    for( const ChkCBoxEntries & rArr : aChkArr )
    {
        // we loop through the list, alternating reading the first/second column,
        // each row appears twice in the list (except for smartart and later entries, which are
        // import only)
        sal_uInt16 nCol = bFirstCol ? 0 : 1;
        bFirstCol = !bFirstCol;
        int nEntry = GetEntry4Type( rArr.eType );
        if (nEntry != -1)
        {
            bool bCheck = false;
            if (rArr.eType != MSFltrPg2_CheckBoxEntries::PDF)
            {
                bCheck = (rOpt.*rArr.FnIs)();
            }
            else
            {
                bCheck = officecfg::Office::Common::Filter::Adobe::Import::PDFToDraw::get();
                nCol = 0;
            }
            m_xCheckLB->set_toggle(nEntry, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE, nCol);
        }
        if (rArr.eType == MSFltrPg2_CheckBoxEntries::SmartArt)
        {
            bFirstCol = !bFirstCol;
        }
    }
    m_xCheckLB->thaw();

    if (rOpt.IsCharBackground2Highlighting())
        m_xHighlightingRB->set_active(true);
    else
        m_xShadingRB->set_active(true);

    m_xHighlightingRB->save_state();

    m_xMSOLockFileCB->set_active(rOpt.IsMSOLockFileCreationIsEnabled());
    m_xMSOLockFileCB->save_state();
    m_xMSOLockFileCB->set_sensitive(!officecfg::Office::Common::Filter::Microsoft::Import::CreateMSOLockFiles::isReadOnly());
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
