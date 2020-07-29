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
    : SfxTabPage(pPage, pController, "sfx/ui/optprintpage.ui", "OptPrintPage", &rSet)
    , m_xPrinterOutputRB(m_xBuilder->weld_radio_button("printer"))
    , m_xPrintFileOutputRB(m_xBuilder->weld_radio_button("file"))
    , m_xReduceTransparencyCB(m_xBuilder->weld_check_button("reducetrans"))
    , m_xReduceTransparencyAutoRB(m_xBuilder->weld_radio_button("reducetransauto"))
    , m_xReduceTransparencyNoneRB(m_xBuilder->weld_radio_button("reducetransnone"))
    , m_xReduceGradientsCB(m_xBuilder->weld_check_button("reducegrad"))
    , m_xReduceGradientsStripesRB(m_xBuilder->weld_radio_button("reducegradstripes"))
    , m_xReduceGradientsColorRB(m_xBuilder->weld_radio_button("reducegradcolor"))
    , m_xReduceGradientsStepCountNF(m_xBuilder->weld_spin_button("reducegradstep"))
    , m_xReduceBitmapsCB(m_xBuilder->weld_check_button("reducebitmap"))
    , m_xReduceBitmapsOptimalRB(m_xBuilder->weld_radio_button("reducebitmapoptimal"))
    , m_xReduceBitmapsNormalRB(m_xBuilder->weld_radio_button("reducebitmapnormal"))
    , m_xReduceBitmapsResolutionRB(m_xBuilder->weld_radio_button("reducebitmapresol"))
    , m_xReduceBitmapsResolutionLB(m_xBuilder->weld_combo_box("reducebitmapdpi"))
    , m_xReduceBitmapsTransparencyCB(m_xBuilder->weld_check_button("reducebitmaptrans"))
    , m_xConvertToGreyscalesCB(m_xBuilder->weld_check_button("converttogray"))
    , m_xPDFCB(m_xBuilder->weld_check_button("pdf"))
    , m_xPaperSizeCB(m_xBuilder->weld_check_button("papersize"))
    , m_xPaperOrientationCB(m_xBuilder->weld_check_button("paperorient"))
    , m_xTransparencyCB(m_xBuilder->weld_check_button("trans"))
{
#ifndef ENABLE_CUPS
    m_xPDFCB->hide();
#endif

    if( bOutputForPrinter )
    {
        m_xPrinterOutputRB->set_active(true);
    }
    else
    {
        m_xPrintFileOutputRB->set_active(true);
        m_xPDFCB->set_sensitive(false);
    }

    m_xPrinterOutputRB->connect_toggled( LINK( this, SfxCommonPrintOptionsTabPage, ToggleOutputPrinterRBHdl ) );
    m_xPrintFileOutputRB->connect_toggled( LINK( this, SfxCommonPrintOptionsTabPage, ToggleOutputPrintFileRBHdl ) );

    m_xReduceTransparencyCB->connect_clicked( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceTransparencyCBHdl ) );
    m_xReduceGradientsCB->connect_clicked( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceGradientsCBHdl ) );
    m_xReduceBitmapsCB->connect_clicked( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceBitmapsCBHdl ) );

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

bool SfxCommonPrintOptionsTabPage::FillItemSet( SfxItemSet* /*rSet*/ )
{
    SvtPrinterOptions       aPrinterOptions;
    SvtPrintFileOptions     aPrintFileOptions;

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    if( m_xPaperSizeCB->get_state_changed_from_saved())
        officecfg::Office::Common::Print::Warning::PaperSize::set(m_xPaperSizeCB->get_active(), batch);
    if( m_xPaperOrientationCB->get_state_changed_from_saved() )
        officecfg::Office::Common::Print::Warning::PaperOrientation::set(m_xPaperOrientationCB->get_active(), batch);
    if( m_xTransparencyCB->get_state_changed_from_saved() )
        officecfg::Office::Common::Print::Warning::Transparency::set(m_xTransparencyCB->get_active(), batch);

    batch->commit();

    ImplSaveControls( m_xPrinterOutputRB->get_active() ? &maPrinterOptions : &maPrintFileOptions );

    aPrinterOptions.SetPrinterOptions( maPrinterOptions );
    aPrintFileOptions.SetPrinterOptions( maPrintFileOptions );

    return false;
}

void SfxCommonPrintOptionsTabPage::Reset( const SfxItemSet* /*rSet*/ )
{
    m_xPaperSizeCB->set_active(officecfg::Office::Common::Print::Warning::PaperSize::get());
    m_xPaperOrientationCB->set_active(officecfg::Office::Common::Print::Warning::PaperOrientation::get());
    m_xTransparencyCB->set_active(officecfg::Office::Common::Print::Warning::Transparency::get());

    m_xPaperSizeCB->save_state();
    m_xPaperOrientationCB->save_state();
    m_xTransparencyCB->save_state();

    SvtBasePrintOptions::GetPrinterOptions( maPrinterOptions );
    SvtBasePrintOptions::GetPrinterOptions( maPrintFileOptions );
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

void SfxCommonPrintOptionsTabPage::ImplUpdateControls( const PrinterOptions* pCurrentOptions )
{
    m_xReduceTransparencyCB->set_active( pCurrentOptions->IsReduceTransparency() );

    if( pCurrentOptions->GetReducedTransparencyMode() == PrinterTransparencyMode::Auto )
        m_xReduceTransparencyAutoRB->set_active(true);
    else
        m_xReduceTransparencyNoneRB->set_active(true);

    m_xReduceGradientsCB->set_active( pCurrentOptions->IsReduceGradients() );

    if( pCurrentOptions->GetReducedGradientMode() == PrinterGradientMode::Stripes )
        m_xReduceGradientsStripesRB->set_active(true);
    else
        m_xReduceGradientsColorRB->set_active(true);

    m_xReduceGradientsStepCountNF->set_value(pCurrentOptions->GetReducedGradientStepCount());

    m_xReduceBitmapsCB->set_active( pCurrentOptions->IsReduceBitmaps() );

    if( pCurrentOptions->GetReducedBitmapMode() == PrinterBitmapMode::Optimal )
        m_xReduceBitmapsOptimalRB->set_active(true);
    else if( pCurrentOptions->GetReducedBitmapMode() == PrinterBitmapMode::Normal )
        m_xReduceBitmapsNormalRB->set_active(true);
    else
        m_xReduceBitmapsResolutionRB->set_active(true);

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

    m_xReduceBitmapsTransparencyCB->set_active( pCurrentOptions->IsReducedBitmapIncludesTransparency() );
    m_xConvertToGreyscalesCB->set_active( pCurrentOptions->IsConvertToGreyscales() );
    m_xPDFCB->set_active( pCurrentOptions->IsPDFAsStandardPrintJobFormat() );

    ClickReduceTransparencyCBHdl(*m_xReduceTransparencyCB);
    ClickReduceGradientsCBHdl(*m_xReduceGradientsCB);
    ClickReduceBitmapsCBHdl(*m_xReduceBitmapsCB);
}

void SfxCommonPrintOptionsTabPage::ImplSaveControls( PrinterOptions* pCurrentOptions )
{
    pCurrentOptions->SetReduceTransparency( m_xReduceTransparencyCB->get_active() );
    pCurrentOptions->SetReducedTransparencyMode( m_xReduceTransparencyAutoRB->get_active() ? PrinterTransparencyMode::Auto : PrinterTransparencyMode::NONE );
    pCurrentOptions->SetReduceGradients( m_xReduceGradientsCB->get_active() );
    pCurrentOptions->SetReducedGradientMode( m_xReduceGradientsStripesRB->get_active() ? PrinterGradientMode::Stripes : PrinterGradientMode::Color  );
    pCurrentOptions->SetReducedGradientStepCount(m_xReduceGradientsStepCountNF->get_value());
    pCurrentOptions->SetReduceBitmaps( m_xReduceBitmapsCB->get_active() );
    pCurrentOptions->SetReducedBitmapMode( m_xReduceBitmapsOptimalRB->get_active() ? PrinterBitmapMode::Optimal :
                                           ( m_xReduceBitmapsNormalRB->get_active() ? PrinterBitmapMode::Normal : PrinterBitmapMode::Resolution ) );
    pCurrentOptions->SetReducedBitmapResolution( aDPIArray[ std::min<sal_uInt16>( m_xReduceBitmapsResolutionLB->get_active(),
                                                                   SAL_N_ELEMENTS(aDPIArray) - 1 ) ] );
    pCurrentOptions->SetReducedBitmapIncludesTransparency( m_xReduceBitmapsTransparencyCB->get_active() );
    pCurrentOptions->SetConvertToGreyscales( m_xConvertToGreyscalesCB->get_active() );
    bool bOrigBackEnd = pCurrentOptions->IsPDFAsStandardPrintJobFormat();
    if (bOrigBackEnd != m_xPDFCB->get_active())
    {
        pCurrentOptions->SetPDFAsStandardPrintJobFormat( m_xPDFCB->get_active() );
        svtools::executeRestartDialog(
                comphelper::getProcessComponentContext(), nullptr,
                svtools::RESTART_REASON_PDF_AS_STANDARD_JOB_FORMAT);
    }
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ClickReduceTransparencyCBHdl, weld::Button&, void )
{
    const bool bReduceTransparency = m_xReduceTransparencyCB->get_active();

    m_xReduceTransparencyAutoRB->set_sensitive( bReduceTransparency );
    m_xReduceTransparencyNoneRB->set_sensitive( bReduceTransparency );

    m_xTransparencyCB->set_sensitive( !bReduceTransparency );
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ClickReduceGradientsCBHdl, weld::Button&, void )
{
    const bool bEnable = m_xReduceGradientsCB->get_active();

    m_xReduceGradientsStripesRB->set_sensitive( bEnable );
    m_xReduceGradientsColorRB->set_sensitive( bEnable );
    m_xReduceGradientsStepCountNF->set_sensitive( bEnable );

    ToggleReduceGradientsStripesRBHdl(*m_xReduceGradientsStripesRB);
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ClickReduceBitmapsCBHdl, weld::Button&, void )
{
    const bool bEnable = m_xReduceBitmapsCB->get_active();

    m_xReduceBitmapsOptimalRB->set_sensitive( bEnable );
    m_xReduceBitmapsNormalRB->set_sensitive( bEnable );
    m_xReduceBitmapsResolutionRB->set_sensitive( bEnable );
    m_xReduceBitmapsTransparencyCB->set_sensitive( bEnable );
    m_xReduceBitmapsResolutionLB->set_sensitive( bEnable );

    ToggleReduceBitmapsResolutionRBHdl(*m_xReduceBitmapsResolutionRB);
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ToggleReduceGradientsStripesRBHdl, weld::ToggleButton&, void )
{
    const bool bEnable = m_xReduceGradientsCB->get_active() && m_xReduceGradientsStripesRB->get_active();

    m_xReduceGradientsStepCountNF->set_sensitive(bEnable);
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ToggleReduceBitmapsResolutionRBHdl, weld::ToggleButton&, void )
{
    const bool bEnable = m_xReduceBitmapsCB->get_active() && m_xReduceBitmapsResolutionRB->get_active();

    m_xReduceBitmapsResolutionLB->set_sensitive(bEnable);
}

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleOutputPrinterRBHdl, weld::ToggleButton&, rButton, void )
{
    if (rButton.get_active())
    {
        ImplUpdateControls( &maPrinterOptions );
        bOutputForPrinter = true;
    }
    else
        ImplSaveControls( &maPrinterOptions );
}

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleOutputPrintFileRBHdl, weld::ToggleButton&, rButton, void )
{
    if (rButton.get_active())
    {
        ImplUpdateControls( &maPrintFileOptions );
        bOutputForPrinter = false;
        m_xPDFCB->set_sensitive(false);
    }
    else
    {
        ImplSaveControls( &maPrintFileOptions );
        m_xPDFCB->set_sensitive(true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
