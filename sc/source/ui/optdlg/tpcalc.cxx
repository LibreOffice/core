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

#undef SC_DLLIMPLEMENTATION

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svl/zforlist.hxx>

#include <globstr.hrc>
#include <scresid.hxx>
#include <docoptio.hxx>
#include <sc.hrc>
#include <officecfg/Office/Calc.hxx>
#include <svtools/restartdialog.hxx>

#include <tpcalc.hxx>

ScTpCalcOptions::ScTpCalcOptions(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pParent, "modules/scalc/ui/optcalculatepage.ui", "OptCalculatePage", &rCoreAttrs)
    , pOldOptions(new ScDocOptions(
        static_cast<const ScTpCalcItem&>(rCoreAttrs.Get(
            GetWhich(SID_SCDOCOPTIONS))).GetDocOptions()))
    , pLocalOptions(new ScDocOptions)
    , nWhichCalc(GetWhich(SID_SCDOCOPTIONS))
    , m_xBtnIterate(m_xBuilder->weld_check_button("iterate"))
    , m_xFtSteps(m_xBuilder->weld_label("stepsft"))
    , m_xEdSteps(m_xBuilder->weld_spin_button("steps"))
    , m_xFtEps(m_xBuilder->weld_label("minchangeft"))
    , m_xEdEps(new DoubleField(m_xBuilder->weld_entry("minchange")))
    , m_xBtnDateStd(m_xBuilder->weld_radio_button("datestd"))
    , m_xBtnDateSc10(m_xBuilder->weld_radio_button("datesc10"))
    , m_xBtnDate1904(m_xBuilder->weld_radio_button("date1904"))
    , m_xBtnCase(m_xBuilder->weld_check_button("case"))
    , m_xBtnCalc(m_xBuilder->weld_check_button("calc"))
    , m_xBtnMatch(m_xBuilder->weld_check_button("match"))
    , m_xBtnWildcards(m_xBuilder->weld_radio_button("formulawildcards"))
    , m_xBtnRegex(m_xBuilder->weld_radio_button("formularegex"))
    , m_xBtnLiteral(m_xBuilder->weld_radio_button("formulaliteral"))
    , m_xBtnLookUp(m_xBuilder->weld_check_button("lookup"))
    , m_xBtnGeneralPrec(m_xBuilder->weld_check_button("generalprec"))
    , m_xFtPrec(m_xBuilder->weld_label("precft"))
    , m_xEdPrec(m_xBuilder->weld_spin_button("prec"))
    , m_xBtnThread(m_xBuilder->weld_check_button("threadingenabled"))
{
    Init();
    SetExchangeSupport();
}

ScTpCalcOptions::~ScTpCalcOptions()
{
}

void ScTpCalcOptions::Init()
{
    m_xBtnIterate->connect_toggled( LINK( this, ScTpCalcOptions, CheckClickHdl ) );
    m_xBtnGeneralPrec->connect_toggled( LINK(this, ScTpCalcOptions, CheckClickHdl) );
    m_xBtnDateStd->connect_clicked( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
    m_xBtnDateSc10->connect_clicked( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
    m_xBtnDate1904->connect_clicked( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
    m_xBtnThread->connect_toggled( LINK( this, ScTpCalcOptions, CheckClickHdl ) );
}

VclPtr<SfxTabPage> ScTpCalcOptions::Create( TabPageParent pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<ScTpCalcOptions>::Create( pParent, *rAttrSet );
}

void ScTpCalcOptions::Reset( const SfxItemSet* /* rCoreAttrs */ )
{
    sal_uInt16  d,m;
    sal_Int16   y;

    *pLocalOptions  = *pOldOptions;

    m_xBtnCase->set_active( !pLocalOptions->IsIgnoreCase() );
    m_xBtnCase->set_sensitive( !officecfg::Office::Calc::Calculate::Other::CaseSensitive::isReadOnly() );
    m_xBtnCalc->set_active( pLocalOptions->IsCalcAsShown() );
    m_xBtnCalc->set_sensitive( !officecfg::Office::Calc::Calculate::Other::Precision::isReadOnly() );
    m_xBtnMatch->set_active( pLocalOptions->IsMatchWholeCell() );
    m_xBtnMatch->set_sensitive( !officecfg::Office::Calc::Calculate::Other::SearchCriteria::isReadOnly() );
    bool bWildcards = pLocalOptions->IsFormulaWildcardsEnabled();
    bool bRegex = pLocalOptions->IsFormulaRegexEnabled();
    // If both, Wildcards and Regex, are set then Wildcards shall take
    // precedence. This is also how other code calling Search handles it. Both
    // simultaneously couldn't be set using UI but editing the configuration.
    if (bWildcards && bRegex)
        bRegex = false;
    m_xBtnWildcards->set_active( bWildcards );
    m_xBtnRegex->set_active( bRegex );
    m_xBtnWildcards->set_sensitive( !officecfg::Office::Calc::Calculate::Other::Wildcards::isReadOnly() );
    m_xBtnRegex->set_sensitive( !officecfg::Office::Calc::Calculate::Other::RegularExpressions::isReadOnly() );
    m_xBtnLiteral->set_active( !bWildcards && !bRegex );
    m_xBtnLiteral->set_sensitive( m_xBtnWildcards->get_sensitive() || m_xBtnRegex->get_sensitive() );
    // if either regex or wildcards radio button is set and read-only, disable all three
    if ( (!m_xBtnWildcards->get_sensitive() && bWildcards) || (!m_xBtnRegex->get_sensitive() && bRegex) )
    {
        m_xBtnWildcards->set_sensitive( false );
        m_xBtnRegex->set_sensitive( false );
        m_xBtnLiteral->set_sensitive( false );
    }
    m_xBtnLookUp->set_active( pLocalOptions->IsLookUpColRowNames() );
    m_xBtnLookUp->set_sensitive( !officecfg::Office::Calc::Calculate::Other::FindLabel::isReadOnly() );
    m_xBtnIterate->set_active( pLocalOptions->IsIter() );
    m_xEdSteps->set_value( pLocalOptions->GetIterCount() );
    m_xEdEps->SetValue( pLocalOptions->GetIterEps(), 6 );

    pLocalOptions->GetDate( d, m, y );

    switch ( y )
    {
        case 1899:
            m_xBtnDateStd->set_active(true);
            break;
        case 1900:
            m_xBtnDateSc10->set_active(true);
            break;
        case 1904:
            m_xBtnDate1904->set_active(true);
            break;
    }

    sal_uInt16 nPrec = pLocalOptions->GetStdPrecision();
    if (nPrec == SvNumberFormatter::UNLIMITED_PRECISION)
    {
        m_xFtPrec->set_sensitive(false);
        m_xEdPrec->set_sensitive(false);
        m_xBtnGeneralPrec->set_active(false);
    }
    else
    {
        m_xBtnGeneralPrec->set_active(true);
        m_xFtPrec->set_sensitive(true);
        m_xEdPrec->set_sensitive(true);
        m_xEdPrec->set_value(nPrec);
    }

    m_xBtnThread->set_sensitive( !officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::isReadOnly() );
    m_xBtnThread->set_active( officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::get() );

    CheckClickHdl(*m_xBtnIterate);
}

bool ScTpCalcOptions::FillItemSet( SfxItemSet* rCoreAttrs )
{
    // every other options are updated in handlers
    pLocalOptions->SetIterCount( static_cast<sal_uInt16>(m_xEdSteps->get_value()) );
    pLocalOptions->SetIgnoreCase( !m_xBtnCase->get_active() );
    pLocalOptions->SetCalcAsShown( m_xBtnCalc->get_active() );
    pLocalOptions->SetMatchWholeCell( m_xBtnMatch->get_active() );
    pLocalOptions->SetFormulaWildcardsEnabled( m_xBtnWildcards->get_active() );
    pLocalOptions->SetFormulaRegexEnabled( m_xBtnRegex->get_active() );
    pLocalOptions->SetLookUpColRowNames( m_xBtnLookUp->get_active() );

    if (m_xBtnGeneralPrec->get_active())
        pLocalOptions->SetStdPrecision(
            static_cast<sal_uInt16>(m_xEdPrec->get_value()) );
    else
        pLocalOptions->SetStdPrecision( SvNumberFormatter::UNLIMITED_PRECISION );

    bool bShouldEnableThreading = m_xBtnThread->get_active();
    if (bShouldEnableThreading != officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::get())
    {
        std::shared_ptr<comphelper::ConfigurationChanges> xBatch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::set(bShouldEnableThreading, xBatch);
        xBatch->commit();
        SolarMutexGuard aGuard;
        if (svtools::executeRestartDialog(
                     comphelper::getProcessComponentContext(), GetFrameWeld(),
                     svtools::RESTART_REASON_THREADING))
            GetParentDialog()->EndDialog(RET_OK);
    }
    if ( *pLocalOptions != *pOldOptions )
    {
        rCoreAttrs->Put( ScTpCalcItem( nWhichCalc, *pLocalOptions ) );
        return true;
    }
    else
        return false;
}

DeactivateRC ScTpCalcOptions::DeactivatePage( SfxItemSet* pSetP )
{
    DeactivateRC nReturn = DeactivateRC::KeepPage;

    double fEps;
    if( m_xEdEps->GetValue( fEps ) && (fEps > 0.0) )
    {
        pLocalOptions->SetIterEps( fEps );
        nReturn = DeactivateRC::LeavePage;
    }

    if ( nReturn == DeactivateRC::KeepPage )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Warning,
                    VclButtonsType::Ok, ScResId(STR_INVALID_EPS)));
        xBox->run();

        m_xEdEps->grab_focus();
    }
    else if ( pSetP )
        FillItemSet( pSetP );

    return nReturn;
}

// Handler:

IMPL_LINK( ScTpCalcOptions, RadioClickHdl, weld::Button&, rBtn, void )
{
    if (&rBtn == m_xBtnDateStd.get())
    {
        pLocalOptions->SetDate( 30, 12, 1899 );
    }
    else if (&rBtn == m_xBtnDateSc10.get())
    {
        pLocalOptions->SetDate( 1, 1, 1900 );
    }
    else if (&rBtn == m_xBtnDate1904.get())
    {
        pLocalOptions->SetDate( 1, 1, 1904 );
    }
}

IMPL_LINK(ScTpCalcOptions, CheckClickHdl, weld::ToggleButton&, rBtn, void)
{
    if (&rBtn == m_xBtnGeneralPrec.get())
    {
        if (rBtn.get_active())
        {
            m_xEdPrec->set_sensitive(true);
            m_xFtPrec->set_sensitive(true);
        }
        else
        {
            m_xEdPrec->set_sensitive(false);
            m_xFtPrec->set_sensitive(false);
        }
    }
    else if (&rBtn == m_xBtnIterate.get())
    {
        if (rBtn.get_active())
        {
            pLocalOptions->SetIter( true );
            m_xFtSteps->set_sensitive(true);  m_xEdSteps->set_sensitive(true);
            m_xFtEps->set_sensitive(true);  m_xEdEps->set_sensitive(true);
        }
        else
        {
            pLocalOptions->SetIter( false );
            m_xFtSteps->set_sensitive(false); m_xEdSteps->set_sensitive(false);
            m_xFtEps->set_sensitive(false); m_xEdEps->set_sensitive(false);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
