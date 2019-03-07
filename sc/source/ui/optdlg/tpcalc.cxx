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

#include <vcl/weld.hxx>
#include <svl/zforlist.hxx>

#include <globstr.hrc>
#include <scresid.hxx>
#include <docoptio.hxx>
#include <sc.hrc>
#include <officecfg/Office/Calc.hxx>

#include <tpcalc.hxx>

ScTpCalcOptions::ScTpCalcOptions(vcl::Window* pParent, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pParent, "OptCalculatePage",
        "modules/scalc/ui/optcalculatepage.ui", &rCoreAttrs)
    , pOldOptions(new ScDocOptions(
        static_cast<const ScTpCalcItem&>(rCoreAttrs.Get(
            GetWhich(SID_SCDOCOPTIONS))).GetDocOptions()))
    , pLocalOptions(new ScDocOptions)
    , nWhichCalc(GetWhich(SID_SCDOCOPTIONS))
{
    get(m_pBtnIterate, "iterate");
    get(m_pFtSteps, "stepsft");
    get(m_pEdSteps, "steps");
    get(m_pFtEps, "minchangeft");
    get(m_pEdEps, "minchange");
    get(m_pBtnDateStd, "datestd");
    get(m_pBtnDateSc10, "datesc10");
    get(m_pBtnDate1904, "date1904");
    get(m_pBtnCase, "case");
    get(m_pBtnCalc, "calc");
    get(m_pBtnMatch, "match");
    get(m_pBtnWildcards, "formulawildcards");
    get(m_pBtnRegex, "formularegex");
    get(m_pBtnLiteral, "formulaliteral");
    get(m_pBtnLookUp, "lookup");
    get(m_pBtnGeneralPrec, "generalprec");
    get(m_pFtPrec, "precft");
    get(m_pEdPrec, "prec");
    get(m_pBtnThread, "threadingenabled");
    Init();
    SetExchangeSupport();
}

ScTpCalcOptions::~ScTpCalcOptions()
{
    disposeOnce();
}

void ScTpCalcOptions::dispose()
{
    pOldOptions.reset();
    pLocalOptions.reset();
    m_pBtnIterate.clear();
    m_pFtSteps.clear();
    m_pEdSteps.clear();
    m_pFtEps.clear();
    m_pEdEps.clear();
    m_pBtnDateStd.clear();
    m_pBtnDateSc10.clear();
    m_pBtnDate1904.clear();
    m_pBtnCase.clear();
    m_pBtnCalc.clear();
    m_pBtnMatch.clear();
    m_pBtnWildcards.clear();
    m_pBtnRegex.clear();
    m_pBtnLiteral.clear();
    m_pBtnLookUp.clear();
    m_pBtnGeneralPrec.clear();
    m_pFtPrec.clear();
    m_pEdPrec.clear();
    m_pBtnThread.clear();
    SfxTabPage::dispose();
}

void ScTpCalcOptions::Init()
{
    m_pBtnIterate->SetClickHdl( LINK( this, ScTpCalcOptions, CheckClickHdl ) );
    m_pBtnGeneralPrec->SetClickHdl( LINK(this, ScTpCalcOptions, CheckClickHdl) );
    m_pBtnDateStd->SetClickHdl( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
    m_pBtnDateSc10->SetClickHdl( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
    m_pBtnDate1904->SetClickHdl( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
    m_pBtnThread->SetClickHdl( LINK( this, ScTpCalcOptions, CheckClickHdl ) );
}

VclPtr<SfxTabPage> ScTpCalcOptions::Create( TabPageParent pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<ScTpCalcOptions>::Create( pParent.pParent, *rAttrSet );
}

void ScTpCalcOptions::Reset( const SfxItemSet* /* rCoreAttrs */ )
{
    sal_uInt16  d,m;
    sal_Int16   y;

    *pLocalOptions  = *pOldOptions;

    m_pBtnCase->Check( !pLocalOptions->IsIgnoreCase() );
    m_pBtnCase->Enable( !officecfg::Office::Calc::Calculate::Other::CaseSensitive::isReadOnly() );
    m_pBtnCalc->Check( pLocalOptions->IsCalcAsShown() );
    m_pBtnCalc->Enable( !officecfg::Office::Calc::Calculate::Other::Precision::isReadOnly() );
    m_pBtnMatch->Check( pLocalOptions->IsMatchWholeCell() );
    m_pBtnMatch->Enable( !officecfg::Office::Calc::Calculate::Other::SearchCriteria::isReadOnly() );
    bool bWildcards = pLocalOptions->IsFormulaWildcardsEnabled();
    bool bRegex = pLocalOptions->IsFormulaRegexEnabled();
    // If both, Wildcards and Regex, are set then Wildcards shall take
    // precedence. This is also how other code calling Search handles it. Both
    // simultaneously couldn't be set using UI but editing the configuration.
    if (bWildcards && bRegex)
        bRegex = false;
    m_pBtnWildcards->Check( bWildcards );
    m_pBtnRegex->Check( bRegex );
    m_pBtnWildcards->Enable( !officecfg::Office::Calc::Calculate::Other::Wildcards::isReadOnly() );
    m_pBtnRegex->Enable( !officecfg::Office::Calc::Calculate::Other::RegularExpressions::isReadOnly() );
    m_pBtnLiteral->Check( !bWildcards && !bRegex );
    m_pBtnLiteral->Enable( m_pBtnWildcards->IsEnabled() || m_pBtnRegex->IsEnabled() );
    // if either regex or wildcards radio button is set and read-only, disable all three
    if ( (!m_pBtnWildcards->IsEnabled() && bWildcards) || (!m_pBtnRegex->IsEnabled() && bRegex) )
    {
        m_pBtnWildcards->Enable( false );
        m_pBtnRegex->Enable( false );
        m_pBtnLiteral->Enable( false );
    }
    m_pBtnLookUp->Check( pLocalOptions->IsLookUpColRowNames() );
    m_pBtnLookUp->Enable( !officecfg::Office::Calc::Calculate::Other::FindLabel::isReadOnly() );
    m_pBtnIterate->Check( pLocalOptions->IsIter() );
    m_pEdSteps->SetValue( pLocalOptions->GetIterCount() );
    m_pEdEps->SetValue( pLocalOptions->GetIterEps(), 6 );

    pLocalOptions->GetDate( d, m, y );

    switch ( y )
    {
        case 1899:
            m_pBtnDateStd->Check();
            break;
        case 1900:
            m_pBtnDateSc10->Check();
            break;
        case 1904:
            m_pBtnDate1904->Check();
            break;
    }

    sal_uInt16 nPrec = pLocalOptions->GetStdPrecision();
    if (nPrec == SvNumberFormatter::UNLIMITED_PRECISION)
    {
        m_pFtPrec->Disable();
        m_pEdPrec->Disable();
        m_pBtnGeneralPrec->Check(false);
    }
    else
    {
        m_pBtnGeneralPrec->Check();
        m_pFtPrec->Enable();
        m_pEdPrec->Enable();
        m_pEdPrec->SetValue(nPrec);
    }

    m_pBtnThread->Enable( !officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::isReadOnly() );
    m_pBtnThread->Check( officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::get() );

    CheckClickHdl(m_pBtnIterate);
}

bool ScTpCalcOptions::FillItemSet( SfxItemSet* rCoreAttrs )
{
    // every other options are updated in handlers
    pLocalOptions->SetIterCount( static_cast<sal_uInt16>(m_pEdSteps->GetValue()) );
    pLocalOptions->SetIgnoreCase( !m_pBtnCase->IsChecked() );
    pLocalOptions->SetCalcAsShown( m_pBtnCalc->IsChecked() );
    pLocalOptions->SetMatchWholeCell( m_pBtnMatch->IsChecked() );
    pLocalOptions->SetFormulaWildcardsEnabled( m_pBtnWildcards->IsChecked() );
    pLocalOptions->SetFormulaRegexEnabled( m_pBtnRegex->IsChecked() );
    pLocalOptions->SetLookUpColRowNames( m_pBtnLookUp->IsChecked() );

    if (m_pBtnGeneralPrec->IsChecked())
        pLocalOptions->SetStdPrecision(
            static_cast<sal_uInt16>(m_pEdPrec->GetValue()) );
    else
        pLocalOptions->SetStdPrecision( SvNumberFormatter::UNLIMITED_PRECISION );

    bool bShouldEnableThreading = m_pBtnThread->IsChecked();
    if (bShouldEnableThreading != officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::get())
    {
        std::shared_ptr<comphelper::ConfigurationChanges> xBatch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::set(bShouldEnableThreading, xBatch);
        xBatch->commit();
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
    if( m_pEdEps->GetValue( fEps ) && (fEps > 0.0) )
    {
        pLocalOptions->SetIterEps( fEps );
        nReturn = DeactivateRC::LeavePage;
    }

    if ( nReturn == DeactivateRC::KeepPage )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Warning,
                    VclButtonsType::Ok, ScResId(STR_INVALID_EPS)));
        xBox->run();

        m_pEdEps->GrabFocus();
    }
    else if ( pSetP )
        FillItemSet( pSetP );

    return nReturn;
}

// Handler:

IMPL_LINK( ScTpCalcOptions, RadioClickHdl, Button*, pBtn, void )
{
    if (pBtn == m_pBtnDateStd)
    {
        pLocalOptions->SetDate( 30, 12, 1899 );
    }
    else if (pBtn == m_pBtnDateSc10)
    {
        pLocalOptions->SetDate( 1, 1, 1900 );
    }
    else if (pBtn == m_pBtnDate1904)
    {
        pLocalOptions->SetDate( 1, 1, 1904 );
    }
}

IMPL_LINK( ScTpCalcOptions, CheckClickHdl, Button*, p, void )
{
    CheckBox* pBtn = static_cast<CheckBox*>(p);
    if (pBtn == m_pBtnGeneralPrec)
    {
        if (pBtn->IsChecked())
        {
            m_pEdPrec->Enable();
            m_pFtPrec->Enable();
        }
        else
        {
            m_pEdPrec->Disable();
            m_pFtPrec->Disable();
        }
    }
    else if (pBtn == m_pBtnIterate)
    {
        if ( pBtn->IsChecked() )
        {
            pLocalOptions->SetIter( true );
            m_pFtSteps->Enable();  m_pEdSteps->Enable();
            m_pFtEps->Enable();  m_pEdEps->Enable();
        }
        else
        {
            pLocalOptions->SetIter( false );
            m_pFtSteps->Disable(); m_pEdSteps->Disable();
            m_pFtEps->Disable(); m_pEdEps->Disable();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
