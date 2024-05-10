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


#include <sal/macros.h>
#include <officecfg/Office/Common.hxx>
#include <svtools/printoptions.hxx>
#include <svtools/restartdialog.hxx>

#include <comphelper/processfactory.hxx>

#include <sfx2/printopt.hxx>

static sal_uInt16   aDPIArray[] = { 72, 96, 150, 200, 300, 600 };
static bool     bOutputForPrinter = true;

#define DPI_COUNT SAL_N_ELEMENTS(aDPIArray)

SfxCommonPrintOptionsTabPage::SfxCommonPrintOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"sfx/ui/optprintpage.ui"_ustr, u"OptPrintPage"_ustr, &rSet)
    , m_xPrinterOutputRB(m_xBuilder->weld_radio_button(u"printer"_ustr))
    , m_xPrintFileOutputRB(m_xBuilder->weld_radio_button(u"file"_ustr))
    , m_xReduceTransparencyCB(m_xBuilder->weld_check_button(u"reducetrans"_ustr))
    , m_xReduceTransparencyImg(m_xBuilder->weld_widget(u"lockreducetrans"_ustr))
    , m_xReduceTransparencyAutoRB(m_xBuilder->weld_radio_button(u"reducetransauto"_ustr))
    , m_xReduceTransparencyNoneRB(m_xBuilder->weld_radio_button(u"reducetransnone"_ustr))
    , m_xReduceTransparencyModeImg(m_xBuilder->weld_widget(u"lockreducetransmode"_ustr))
    , m_xReduceGradientsCB(m_xBuilder->weld_check_button(u"reducegrad"_ustr))
    , m_xReduceGradientsImg(m_xBuilder->weld_widget(u"lockreducegrad"_ustr))
    , m_xReduceGradientsStripesRB(m_xBuilder->weld_radio_button(u"reducegradstripes"_ustr))
    , m_xReduceGradientsColorRB(m_xBuilder->weld_radio_button(u"reducegradcolor"_ustr))
    , m_xReduceGradientsModeImg(m_xBuilder->weld_widget(u"lockreducegradmode"_ustr))
    , m_xReduceGradientsStepCountNF(m_xBuilder->weld_spin_button(u"reducegradstep"_ustr))
    , m_xReduceBitmapsCB(m_xBuilder->weld_check_button(u"reducebitmap"_ustr))
    , m_xReduceBitmapsImg(m_xBuilder->weld_widget(u"lockreducebitmap"_ustr))
    , m_xReduceBitmapsOptimalRB(m_xBuilder->weld_radio_button(u"reducebitmapoptimal"_ustr))
    , m_xReduceBitmapsNormalRB(m_xBuilder->weld_radio_button(u"reducebitmapnormal"_ustr))
    , m_xReduceBitmapsResolutionRB(m_xBuilder->weld_radio_button(u"reducebitmapresol"_ustr))
    , m_xReduceBitmapsModeImg(m_xBuilder->weld_widget(u"lockreducebitmapmode"_ustr))
    , m_xReduceBitmapsResolutionLB(m_xBuilder->weld_combo_box(u"reducebitmapdpi"_ustr))
    , m_xReduceBitmapsTransparencyCB(m_xBuilder->weld_check_button(u"reducebitmaptrans"_ustr))
    , m_xReduceBitmapsTransparencyImg(m_xBuilder->weld_widget(u"lockreducebitmaptrans"_ustr))
    , m_xConvertToGreyscalesCB(m_xBuilder->weld_check_button(u"converttogray"_ustr))
    , m_xConvertToGreyscalesImg(m_xBuilder->weld_widget(u"lockconverttogray"_ustr))
    , m_xPaperSizeCB(m_xBuilder->weld_check_button(u"papersize"_ustr))
    , m_xPaperSizeImg(m_xBuilder->weld_widget(u"lockpapersize"_ustr))
    , m_xPaperOrientationCB(m_xBuilder->weld_check_button(u"paperorient"_ustr))
    , m_xPaperOrientationImg(m_xBuilder->weld_widget(u"lockpaperorient"_ustr))
    , m_xTransparencyCB(m_xBuilder->weld_check_button(u"trans"_ustr))
    , m_xTransparencyImg(m_xBuilder->weld_widget(u"locktrans"_ustr))
{
    if (bOutputForPrinter)
        m_xPrinterOutputRB->set_active(true);
    else
        m_xPrintFileOutputRB->set_active(true);

    m_xPrinterOutputRB->connect_toggled( LINK( this, SfxCommonPrintOptionsTabPage, ToggleOutputPrinterRBHdl ) );
    m_xPrintFileOutputRB->connect_toggled( LINK( this, SfxCommonPrintOptionsTabPage, ToggleOutputPrintFileRBHdl ) );

    m_xReduceTransparencyCB->connect_toggled( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceTransparencyCBHdl ) );
    m_xReduceGradientsCB->connect_toggled( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceGradientsCBHdl ) );
    m_xReduceBitmapsCB->connect_toggled( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceBitmapsCBHdl ) );

    m_xReduceGradientsStripesRB->connect_toggled( LINK( this, SfxCommonPrintOptionsTabPage, ToggleReduceGradientsStripesRBHdl ) );
    m_xReduceBitmapsResolutionRB->connect_toggled( LINK( this, SfxCommonPrintOptionsTabPage, ToggleReduceBitmapsResolutionRBHdl ) );
}

SfxCommonPrintOptionsTabPage::~SfxCommonPrintOptionsTabPage()
{
}

std::unique_ptr<SfxTabPage> SfxCommonPrintOptionsTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SfxCommonPrintOptionsTabPage>(pPage, pController, *rAttrSet);
}

OUString SfxCommonPrintOptionsTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label4"_ustr, u"label6"_ustr, u"label2"_ustr, u"label3"_ustr, u"label1"_ustr, u"label5"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"converttogray"_ustr, u"reducebitmaptrans"_ustr, u"reducebitmap"_ustr, u"reducetrans"_ustr,
                               u"papersize"_ustr,     u"paperorient"_ustr,       u"trans"_ustr,        u"reducegrad"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    OUString radioButton[] = { u"printer"_ustr,
                               u"file"_ustr,
                               u"reducebitmapresol"_ustr,
                               u"reducebitmapnormal"_ustr,
                               u"reducebitmapoptimal"_ustr,
                               u"reducetransauto"_ustr,
                               u"reducetransnone"_ustr,
                               u"reducegradstripes"_ustr,
                               u"reducegradcolor"_ustr };

    for (const auto& radio : radioButton)
    {
        if (const auto& pString = m_xBuilder->weld_radio_button(radio))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SfxCommonPrintOptionsTabPage::FillItemSet( SfxItemSet* /*rSet*/ )
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    if( m_xPaperSizeCB->get_state_changed_from_saved())
        officecfg::Office::Common::Print::Warning::PaperSize::set(m_xPaperSizeCB->get_active(), batch);
    if( m_xPaperOrientationCB->get_state_changed_from_saved() )
        officecfg::Office::Common::Print::Warning::PaperOrientation::set(m_xPaperOrientationCB->get_active(), batch);
    if( m_xTransparencyCB->get_state_changed_from_saved() )
        officecfg::Office::Common::Print::Warning::Transparency::set(m_xTransparencyCB->get_active(), batch);

    batch->commit();

    ImplSaveControls( m_xPrinterOutputRB->get_active() ? &maPrinterOptions : &maPrintFileOptions );

    svtools::SetPrinterOptions(maPrinterOptions, /*bFile*/false);
    svtools::SetPrinterOptions(maPrintFileOptions, /*bFile*/true);

    return false;
}

void SfxCommonPrintOptionsTabPage::Reset( const SfxItemSet* /*rSet*/ )
{
    m_xPaperSizeCB->set_active(officecfg::Office::Common::Print::Warning::PaperSize::get());
    m_xPaperSizeCB->set_sensitive(!officecfg::Office::Common::Print::Warning::PaperSize::isReadOnly());
    m_xPaperSizeImg->set_visible(officecfg::Office::Common::Print::Warning::PaperSize::isReadOnly());

    m_xPaperOrientationCB->set_active(officecfg::Office::Common::Print::Warning::PaperOrientation::get());
    m_xPaperOrientationCB->set_sensitive(!officecfg::Office::Common::Print::Warning::PaperOrientation::isReadOnly());
    m_xPaperOrientationImg->set_visible(officecfg::Office::Common::Print::Warning::PaperOrientation::isReadOnly());

    m_xTransparencyCB->set_active(officecfg::Office::Common::Print::Warning::Transparency::get());
    m_xTransparencyCB->set_sensitive(!officecfg::Office::Common::Print::Warning::Transparency::isReadOnly());
    m_xTransparencyImg->set_visible(officecfg::Office::Common::Print::Warning::Transparency::isReadOnly());

    m_xPaperSizeCB->save_state();
    m_xPaperOrientationCB->save_state();
    m_xTransparencyCB->save_state();

    svtools::GetPrinterOptions( maPrinterOptions, /*bFile*/false );
    svtools::GetPrinterOptions( maPrintFileOptions, /*bFile*/true );
    if(m_xPrintFileOutputRB->get_active()){
       m_xPrinterOutputRB->set_active(true);
    }

    ImplUpdateControls( m_xPrinterOutputRB->get_active() ? &maPrinterOptions : &maPrintFileOptions );
}

DeactivateRC SfxCommonPrintOptionsTabPage::DeactivatePage( SfxItemSet* pItemSet )
{
    if( pItemSet )
        FillItemSet( pItemSet );

    return DeactivateRC::LeavePage;
}

void SfxCommonPrintOptionsTabPage::ImplUpdateControls( const vcl::printer::Options* pCurrentOptions )
{
    bool bEnable = true;

    bEnable = !officecfg::Office::Common::Print::Option::File::ReduceTransparency::isReadOnly();
    m_xReduceTransparencyCB->set_active( pCurrentOptions->IsReduceTransparency() );
    m_xReduceTransparencyCB->set_sensitive(bEnable);
    m_xReduceTransparencyImg->set_visible(!bEnable);

    if( pCurrentOptions->GetReducedTransparencyMode() == vcl::printer::TransparencyMode::Auto )
        m_xReduceTransparencyAutoRB->set_active(true);
    else
        m_xReduceTransparencyNoneRB->set_active(true);

    bEnable = !officecfg::Office::Common::Print::Option::File::ReducedTransparencyMode::isReadOnly();
    m_xReduceTransparencyAutoRB->set_sensitive(bEnable);
    m_xReduceTransparencyNoneRB->set_sensitive(bEnable);
    m_xReduceTransparencyModeImg->set_visible(!bEnable);

    bEnable = !officecfg::Office::Common::Print::Option::Printer::ReduceGradients::isReadOnly();
    m_xReduceGradientsCB->set_active( pCurrentOptions->IsReduceGradients() );
    m_xReduceGradientsCB->set_sensitive(bEnable);
    m_xReduceGradientsImg->set_visible(!bEnable);

    if( pCurrentOptions->GetReducedGradientMode() == vcl::printer::GradientMode::Stripes )
        m_xReduceGradientsStripesRB->set_active(true);
    else
        m_xReduceGradientsColorRB->set_active(true);

    bEnable = !officecfg::Office::Common::Print::Option::Printer::ReducedGradientMode::isReadOnly();
    m_xReduceGradientsStripesRB->set_sensitive(bEnable);
    m_xReduceGradientsColorRB->set_sensitive(bEnable);
    m_xReduceGradientsModeImg->set_visible(!bEnable);

    bEnable = !officecfg::Office::Common::Print::Option::Printer::ReducedGradientStepCount::isReadOnly();
    m_xReduceGradientsStepCountNF->set_value(pCurrentOptions->GetReducedGradientStepCount());
    m_xReduceGradientsStepCountNF->set_sensitive(bEnable);

    bEnable = !officecfg::Office::Common::Print::Option::Printer::ReduceBitmaps::isReadOnly();
    m_xReduceBitmapsCB->set_active( pCurrentOptions->IsReduceBitmaps() );
    m_xReduceBitmapsCB->set_sensitive(bEnable);
    m_xReduceBitmapsImg->set_visible(!bEnable);

    if( pCurrentOptions->GetReducedBitmapMode() == vcl::printer::BitmapMode::Optimal )
        m_xReduceBitmapsOptimalRB->set_active(true);
    else if( pCurrentOptions->GetReducedBitmapMode() == vcl::printer::BitmapMode::Normal )
        m_xReduceBitmapsNormalRB->set_active(true);
    else
        m_xReduceBitmapsResolutionRB->set_active(true);

    bEnable = !officecfg::Office::Common::Print::Option::Printer::ReducedBitmapMode::isReadOnly();
    m_xReduceBitmapsOptimalRB->set_sensitive(bEnable);
    m_xReduceBitmapsNormalRB->set_sensitive(bEnable);
    m_xReduceBitmapsResolutionRB->set_sensitive(bEnable);
    m_xReduceBitmapsModeImg->set_visible(!bEnable);

    const sal_uInt16 nDPI = pCurrentOptions->GetReducedBitmapResolution();

    if( nDPI < aDPIArray[ 0 ] )
        m_xReduceBitmapsResolutionLB->set_active(0);
    else
    {
        for( int i = DPI_COUNT - 1; i >= 0; i-- )
        {
            if( nDPI >= aDPIArray[ i ] )
            {
                m_xReduceBitmapsResolutionLB->set_active(i);
                i = -1;
            }
        }
    }
    bEnable = !officecfg::Office::Common::Print::Option::File::ReducedBitmapResolution::isReadOnly();
    m_xReduceBitmapsResolutionLB->set_sensitive(bEnable);

    bEnable = !officecfg::Office::Common::Print::Option::Printer::ReducedBitmapIncludesTransparency::isReadOnly();
    m_xReduceBitmapsTransparencyCB->set_active( pCurrentOptions->IsReducedBitmapIncludesTransparency() );
    m_xReduceBitmapsTransparencyCB->set_sensitive(bEnable);
    m_xReduceBitmapsTransparencyImg->set_visible(!bEnable);

    bEnable = !officecfg::Office::Common::Print::Option::Printer::ConvertToGreyscales::isReadOnly();
    m_xConvertToGreyscalesCB->set_active( pCurrentOptions->IsConvertToGreyscales() );
    m_xConvertToGreyscalesCB->set_sensitive(bEnable);
    m_xConvertToGreyscalesImg->set_visible(!bEnable);

    ClickReduceTransparencyCBHdl(*m_xReduceTransparencyCB);
    ClickReduceGradientsCBHdl(*m_xReduceGradientsCB);
    ClickReduceBitmapsCBHdl(*m_xReduceBitmapsCB);
}

void SfxCommonPrintOptionsTabPage::ImplSaveControls( vcl::printer::Options* pCurrentOptions )
{
    pCurrentOptions->SetReduceTransparency( m_xReduceTransparencyCB->get_active() );
    pCurrentOptions->SetReducedTransparencyMode( m_xReduceTransparencyAutoRB->get_active() ? vcl::printer::TransparencyMode::Auto : vcl::printer::TransparencyMode::NONE );
    pCurrentOptions->SetReduceGradients( m_xReduceGradientsCB->get_active() );
    pCurrentOptions->SetReducedGradientMode( m_xReduceGradientsStripesRB->get_active() ? vcl::printer::GradientMode::Stripes : vcl::printer::GradientMode::Color  );
    pCurrentOptions->SetReducedGradientStepCount(m_xReduceGradientsStepCountNF->get_value());
    pCurrentOptions->SetReduceBitmaps( m_xReduceBitmapsCB->get_active() );
    pCurrentOptions->SetReducedBitmapMode( m_xReduceBitmapsOptimalRB->get_active() ? vcl::printer::BitmapMode::Optimal :
                                           ( m_xReduceBitmapsNormalRB->get_active() ? vcl::printer::BitmapMode::Normal : vcl::printer::BitmapMode::Resolution ) );
    pCurrentOptions->SetReducedBitmapResolution( aDPIArray[ std::min<sal_uInt16>( m_xReduceBitmapsResolutionLB->get_active(),
                                                                   SAL_N_ELEMENTS(aDPIArray) - 1 ) ] );
    pCurrentOptions->SetReducedBitmapIncludesTransparency( m_xReduceBitmapsTransparencyCB->get_active() );
    pCurrentOptions->SetConvertToGreyscales( m_xConvertToGreyscalesCB->get_active() );
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ClickReduceTransparencyCBHdl, weld::Toggleable&, void )
{
    const bool bReduceTransparency = m_xReduceTransparencyCB->get_active();
    bool bReadOnly = officecfg::Office::Common::Print::Option::File::ReducedTransparencyMode::isReadOnly();

    m_xReduceTransparencyAutoRB->set_sensitive( bReduceTransparency && !bReadOnly );
    m_xReduceTransparencyNoneRB->set_sensitive( bReduceTransparency && !bReadOnly );

    bReadOnly = officecfg::Office::Common::Print::Warning::Transparency::isReadOnly();
    m_xTransparencyCB->set_sensitive( !bReduceTransparency && !bReadOnly);
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ClickReduceGradientsCBHdl, weld::Toggleable&, void )
{
    const bool bEnable = m_xReduceGradientsCB->get_active();
    bool bReadOnly = officecfg::Office::Common::Print::Option::Printer::ReducedGradientMode::isReadOnly();

    m_xReduceGradientsStripesRB->set_sensitive( bEnable && !bReadOnly );
    m_xReduceGradientsColorRB->set_sensitive( bEnable && !bReadOnly );

    bReadOnly = officecfg::Office::Common::Print::Option::Printer::ReducedGradientStepCount::isReadOnly();
    m_xReduceGradientsStepCountNF->set_sensitive( bEnable && !bReadOnly );

    ToggleReduceGradientsStripesRBHdl(*m_xReduceGradientsStripesRB);
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ClickReduceBitmapsCBHdl, weld::Toggleable&, void )
{
    const bool bEnable = m_xReduceBitmapsCB->get_active();
    bool bReadOnly = officecfg::Office::Common::Print::Option::Printer::ReducedBitmapMode::isReadOnly();

    m_xReduceBitmapsOptimalRB->set_sensitive( bEnable && !bReadOnly);
    m_xReduceBitmapsNormalRB->set_sensitive( bEnable && !bReadOnly);
    m_xReduceBitmapsResolutionRB->set_sensitive( bEnable && !bReadOnly );
    m_xReduceBitmapsTransparencyCB->set_sensitive( bEnable &&
        !officecfg::Office::Common::Print::Option::Printer::ReducedBitmapIncludesTransparency::isReadOnly() );
    m_xReduceBitmapsResolutionLB->set_sensitive( bEnable &&
        !officecfg::Office::Common::Print::Option::File::ReducedBitmapResolution::isReadOnly() );

    ToggleReduceBitmapsResolutionRBHdl(*m_xReduceBitmapsResolutionRB);
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ToggleReduceGradientsStripesRBHdl, weld::Toggleable&, void )
{
    const bool bEnable = m_xReduceGradientsCB->get_active() && m_xReduceGradientsStripesRB->get_active();

    m_xReduceGradientsStepCountNF->set_sensitive(bEnable);
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ToggleReduceBitmapsResolutionRBHdl, weld::Toggleable&, void )
{
    const bool bEnable = m_xReduceBitmapsCB->get_active() && m_xReduceBitmapsResolutionRB->get_active();

    m_xReduceBitmapsResolutionLB->set_sensitive(bEnable);
}

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleOutputPrinterRBHdl, weld::Toggleable&, rButton, void )
{
    if (rButton.get_active())
    {
        ImplUpdateControls( &maPrinterOptions );
        bOutputForPrinter = true;
    }
    else
        ImplSaveControls( &maPrinterOptions );
}

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleOutputPrintFileRBHdl, weld::Toggleable&, rButton, void )
{
    if (rButton.get_active())
    {
        ImplUpdateControls( &maPrintFileOptions );
        bOutputForPrinter = false;
    }
    else
    {
        ImplSaveControls( &maPrintFileOptions );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
