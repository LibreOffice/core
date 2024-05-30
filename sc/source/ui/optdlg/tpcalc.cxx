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
#include <svl/numformat.hxx>

#include <globstr.hrc>
#include <scresid.hxx>
#include <docoptio.hxx>
#include <sc.hrc>
#include <officecfg/Office/Calc.hxx>
#include <svtools/restartdialog.hxx>

#include <tpcalc.hxx>

ScTpCalcOptions::ScTpCalcOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pPage, pController, u"modules/scalc/ui/optcalculatepage.ui"_ustr, u"OptCalculatePage"_ustr, &rCoreAttrs)
    , pOldOptions(new ScDocOptions(
        rCoreAttrs.Get(SID_SCDOCOPTIONS).GetDocOptions()))
    , pLocalOptions(new ScDocOptions)
    , m_xBtnIterate(m_xBuilder->weld_check_button(u"iterate"_ustr))
    , m_xBtnIterateImg(m_xBuilder->weld_widget(u"lockiterate"_ustr))
    , m_xFtSteps(m_xBuilder->weld_label(u"stepsft"_ustr))
    , m_xEdSteps(m_xBuilder->weld_spin_button(u"steps"_ustr))
    , m_xEdStepsImg(m_xBuilder->weld_widget(u"locksteps"_ustr))
    , m_xFtEps(m_xBuilder->weld_label(u"minchangeft"_ustr))
    , m_xEdEps(new ScDoubleField(m_xBuilder->weld_entry(u"minchange"_ustr)))
    , m_xEdEpsImg(m_xBuilder->weld_widget(u"lockminchange"_ustr))
    , m_xBtnDateStd(m_xBuilder->weld_radio_button(u"datestd"_ustr))
    , m_xBtnDateSc10(m_xBuilder->weld_radio_button(u"datesc10"_ustr))
    , m_xBtnDate1904(m_xBuilder->weld_radio_button(u"date1904"_ustr))
    , m_xDateImg(m_xBuilder->weld_widget(u"lockdate"_ustr))
    , m_xBtnCase(m_xBuilder->weld_check_button(u"case"_ustr))
    , m_xBtnCaseImg(m_xBuilder->weld_widget(u"lockcase"_ustr))
    , m_xBtnCalc(m_xBuilder->weld_check_button(u"calc"_ustr))
    , m_xBtnCalcImg(m_xBuilder->weld_widget(u"lockcalc"_ustr))
    , m_xBtnMatch(m_xBuilder->weld_check_button(u"match"_ustr))
    , m_xBtnMatchImg(m_xBuilder->weld_widget(u"lockmatch"_ustr))
    , m_xBtnWildcards(m_xBuilder->weld_radio_button(u"formulawildcards"_ustr))
    , m_xBtnRegex(m_xBuilder->weld_radio_button(u"formularegex"_ustr))
    , m_xBtnLiteral(m_xBuilder->weld_radio_button(u"formulaliteral"_ustr))
    , m_xFormulaImg(m_xBuilder->weld_widget(u"lockformulawild"_ustr))
    , m_xBtnLookUp(m_xBuilder->weld_check_button(u"lookup"_ustr))
    , m_xBtnLookUpImg(m_xBuilder->weld_widget(u"locklookup"_ustr))
    , m_xBtnGeneralPrec(m_xBuilder->weld_check_button(u"generalprec"_ustr))
    , m_xBtnGeneralPrecImg(m_xBuilder->weld_widget(u"lockgeneralprec"_ustr))
    , m_xFtPrec(m_xBuilder->weld_label(u"precft"_ustr))
    , m_xEdPrec(m_xBuilder->weld_spin_button(u"prec"_ustr))
    , m_xEdPrecImg(m_xBuilder->weld_widget(u"lockprec"_ustr))
    , m_xBtnThread(m_xBuilder->weld_check_button(u"threadingenabled"_ustr))
    , m_xBtnThreadImg(m_xBuilder->weld_widget(u"lockthreadingenabled"_ustr))
{
    Init();
    SetExchangeSupport();

    css::uno::Reference < css::uno::XComponentContext > xContext(::comphelper::getProcessComponentContext());
    m_xReadWriteAccess = css::configuration::ReadWriteAccess::create(xContext, u"*"_ustr);
}

ScTpCalcOptions::~ScTpCalcOptions()
{
}

void ScTpCalcOptions::Init()
{
    m_xBtnIterate->connect_toggled( LINK( this, ScTpCalcOptions, CheckClickHdl ) );
    m_xBtnGeneralPrec->connect_toggled( LINK(this, ScTpCalcOptions, CheckClickHdl) );
    m_xBtnDateStd->connect_toggled( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
    m_xBtnDateSc10->connect_toggled( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
    m_xBtnDate1904->connect_toggled( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
    m_xBtnThread->connect_toggled( LINK( this, ScTpCalcOptions, CheckClickHdl ) );
}

std::unique_ptr<SfxTabPage> ScTpCalcOptions::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet )
{
    return std::make_unique<ScTpCalcOptions>( pPage, pController, *rAttrSet );
}

void ScTpCalcOptions::Reset(const SfxItemSet* rCoreAttrs)
{
    sal_uInt16  d,m;
    sal_Int16   y;

    pOldOptions.reset(new ScDocOptions(
        rCoreAttrs->Get(SID_SCDOCOPTIONS).GetDocOptions()));

    *pLocalOptions = *pOldOptions;

    bool bReadOnly = officecfg::Office::Calc::Calculate::Other::CaseSensitive::isReadOnly();
    m_xBtnCase->set_active( !pLocalOptions->IsIgnoreCase() );
    m_xBtnCase->set_sensitive(!bReadOnly);
    m_xBtnCaseImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Calculate::Other::Precision::isReadOnly();
    m_xBtnCalc->set_active( pLocalOptions->IsCalcAsShown() );
    m_xBtnCalc->set_sensitive(!bReadOnly);
    m_xBtnCalcImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Calculate::Other::SearchCriteria::isReadOnly();
    m_xBtnMatch->set_active( pLocalOptions->IsMatchWholeCell() );
    m_xBtnMatch->set_sensitive(!bReadOnly);
    m_xBtnMatchImg->set_visible(bReadOnly);

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
    bReadOnly = officecfg::Office::Calc::Calculate::Other::Wildcards::isReadOnly() ||
        officecfg::Office::Calc::Calculate::Other::RegularExpressions::isReadOnly();
    m_xFormulaImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Calculate::Other::FindLabel::isReadOnly();
    m_xBtnLookUp->set_active( pLocalOptions->IsLookUpColRowNames() );
    m_xBtnLookUp->set_sensitive(!bReadOnly);
    m_xBtnLookUpImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Calculate::IterativeReference::Iteration::isReadOnly();
    m_xBtnIterate->set_active( pLocalOptions->IsIter() );
    m_xBtnIterate->set_sensitive(!bReadOnly);
    m_xBtnIterateImg->set_visible(bReadOnly);

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

    // TODO: these option settings need to be simplified.
    bReadOnly = false;
    OUString aDateConfPath = officecfg::Office::Calc::Calculate::Other::path() + "/Date/DD";
    if (m_xReadWriteAccess->hasPropertyByHierarchicalName(aDateConfPath))
    {
        css::beans::Property aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(aDateConfPath);
        bReadOnly = (aProperty.Attributes & css::beans::PropertyAttribute::READONLY) != 0;
    }

    if (!bReadOnly)
    {
        aDateConfPath = officecfg::Office::Calc::Calculate::Other::path() + "/Date/MM";
        if (m_xReadWriteAccess->hasPropertyByHierarchicalName(aDateConfPath))
        {
            css::beans::Property aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(aDateConfPath);
            bReadOnly = (aProperty.Attributes & css::beans::PropertyAttribute::READONLY) != 0;
        }
    }

    if (!bReadOnly)
    {
        aDateConfPath = officecfg::Office::Calc::Calculate::Other::path() + "/Date/YY";
        if (m_xReadWriteAccess->hasPropertyByHierarchicalName(aDateConfPath))
        {
            css::beans::Property aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(aDateConfPath);
            bReadOnly = (aProperty.Attributes & css::beans::PropertyAttribute::READONLY) != 0;
        }
    }

    if (bReadOnly)
    {
        m_xBtnDateStd->set_sensitive(false);
        m_xBtnDateSc10->set_sensitive(false);
        m_xBtnDate1904->set_sensitive(false);
        m_xDateImg->set_visible(true);
    }

    sal_uInt16 nPrec = pLocalOptions->GetStdPrecision();
    bReadOnly = officecfg::Office::Calc::Calculate::Other::DecimalPlaces::isReadOnly();
    if (nPrec == SvNumberFormatter::UNLIMITED_PRECISION)
    {
        m_xFtPrec->set_sensitive(false);
        m_xEdPrec->set_sensitive(false);
        m_xBtnGeneralPrec->set_active(false);
        m_xEdPrec->set_value(0);
    }
    else
    {
        m_xBtnGeneralPrec->set_active(true);
        m_xFtPrec->set_sensitive(true);
        m_xEdPrec->set_sensitive(!bReadOnly);
        m_xEdPrec->set_value(nPrec);
    }
    m_xBtnGeneralPrec->set_sensitive(!bReadOnly);
    m_xBtnGeneralPrecImg->set_visible(bReadOnly);
    m_xEdPrecImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Calculate::IterativeReference::Steps::isReadOnly();
    m_xEdSteps->set_sensitive(!bReadOnly);
    m_xEdStepsImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Calculate::IterativeReference::MinimumChange::isReadOnly();
    m_xEdEps->set_sensitive(!bReadOnly);
    m_xEdEpsImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::isReadOnly();
    m_xBtnThread->set_sensitive(!bReadOnly);
    m_xBtnThreadImg->set_visible(bReadOnly);
    m_xBtnThread->set_active( officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::get() );

    CheckClickHdl(*m_xBtnIterate);
}

OUString ScTpCalcOptions::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[]
        = { u"label5"_ustr, u"label1"_ustr, u"precft"_ustr, u"label2"_ustr, u"stepsft"_ustr, u"minchangeft"_ustr, u"label4"_ustr, u"label3"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[]
        = { u"case"_ustr, u"calc"_ustr, u"match"_ustr, u"lookup"_ustr, u"generalprec"_ustr, u"iterate"_ustr, u"threadingenabled"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    OUString radioButton[] = { u"formulawildcards"_ustr, u"formularegex"_ustr, u"formulaliteral"_ustr,
                               u"datestd"_ustr,          u"datesc10"_ustr,     u"date1904"_ustr };

    for (const auto& radio : radioButton)
    {
        if (const auto& pString = m_xBuilder->weld_radio_button(radio))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
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
            GetDialogController()->response(RET_OK);
    }
    if ( *pLocalOptions != *pOldOptions )
    {
        rCoreAttrs->Put( ScTpCalcItem( SID_SCDOCOPTIONS, *pLocalOptions ) );
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

IMPL_LINK( ScTpCalcOptions, RadioClickHdl, weld::Toggleable&, rBtn, void )
{
    if (!rBtn.get_active())
        return;
    if (m_xBtnDateStd->get_active())
    {
        pLocalOptions->SetDate( 30, 12, 1899 );
    }
    else if (m_xBtnDateSc10->get_active())
    {
        pLocalOptions->SetDate( 1, 1, 1900 );
    }
    else if (m_xBtnDate1904->get_active())
    {
        pLocalOptions->SetDate( 1, 1, 1904 );
    }
}

IMPL_LINK(ScTpCalcOptions, CheckClickHdl, weld::Toggleable&, rBtn, void)
{
    if (&rBtn == m_xBtnGeneralPrec.get())
    {
        if (rBtn.get_active())
        {
            m_xEdPrec->set_sensitive(!officecfg::Office::Calc::Calculate::Other::DecimalPlaces::isReadOnly());
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
            m_xFtSteps->set_sensitive(true);  m_xEdSteps->set_sensitive(!officecfg::Office::Calc::Calculate::IterativeReference::Steps::isReadOnly());
            m_xFtEps->set_sensitive(true);  m_xEdEps->set_sensitive(!officecfg::Office::Calc::Calculate::IterativeReference::MinimumChange::isReadOnly());
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
