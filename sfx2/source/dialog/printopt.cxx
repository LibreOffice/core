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
#include <unotools/printwarningoptions.hxx>
#include <svtools/printoptions.hxx>
#include <svtools/restartdialog.hxx>
#include <svl/flagitem.hxx>

#include <comphelper/processfactory.hxx>

#include <sfx2/viewsh.hxx>
#include <sfx2/printopt.hxx>

static sal_uInt16   aDPIArray[] = { 72, 96, 150, 200, 300, 600 };
static bool     bOutputForPrinter = true;

#define DPI_COUNT SAL_N_ELEMENTS(aDPIArray)

SfxCommonPrintOptionsTabPage::SfxCommonPrintOptionsTabPage( vcl::Window* pParent, const SfxItemSet& rSet )
    : SfxTabPage(pParent, "OptPrintPage", "sfx/ui/optprintpage.ui", &rSet)
{
    get(m_pPrinterOutputRB, "printer");
    get(m_pPrintFileOutputRB, "file");
    get(m_pReduceTransparencyCB, "reducetrans");
    get(m_pReduceTransparencyAutoRB, "reducetransauto");
    get(m_pReduceTransparencyNoneRB, "reducetransnone");
    get(m_pReduceGradientsCB, "reducegrad");
    get(m_pReduceGradientsStripesRB, "reducegradstripes");
    get(m_pReduceGradientsColorRB, "reducegradcolor");
    get(m_pReduceGradientsStepCountNF, "reducegradstep");
    get(m_pReduceBitmapsCB, "reducebitmap");
    get(m_pReduceBitmapsOptimalRB, "reducebitmapoptimal");
    get(m_pReduceBitmapsNormalRB, "reducebitmapnormal");
    get(m_pReduceBitmapsResolutionRB, "reducebitmapresol");
    get(m_pReduceBitmapsResolutionLB, "reducebitmapdpi");
    get(m_pReduceBitmapsTransparencyCB, "reducebitmaptrans");
    get(m_pConvertToGreyscalesCB, "converttogray");
    get(m_pPDFCB, "pdf");
    get(m_pPaperSizeCB, "papersize");
    get(m_pPaperOrientationCB, "paperorient");
    get(m_pTransparencyCB, "trans");

#ifndef ENABLE_CUPS
    m_pPDFCB->Hide();
#endif

    if( bOutputForPrinter )
    {
        m_pPrinterOutputRB->Check();
    }
    else
    {
        m_pPrintFileOutputRB->Check();
        m_pPDFCB->Disable();
    }

    m_pPrinterOutputRB->SetToggleHdl( LINK( this, SfxCommonPrintOptionsTabPage, ToggleOutputPrinterRBHdl ) );
    m_pPrintFileOutputRB->SetToggleHdl( LINK( this, SfxCommonPrintOptionsTabPage, ToggleOutputPrintFileRBHdl ) );

    m_pReduceTransparencyCB->SetClickHdl( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceTransparencyCBHdl ) );
    m_pReduceGradientsCB->SetClickHdl( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceGradientsCBHdl ) );
    m_pReduceBitmapsCB->SetClickHdl( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceBitmapsCBHdl ) );

    m_pReduceGradientsStripesRB->SetToggleHdl( LINK( this, SfxCommonPrintOptionsTabPage, ToggleReduceGradientsStripesRBHdl ) );
    m_pReduceBitmapsResolutionRB->SetToggleHdl( LINK( this, SfxCommonPrintOptionsTabPage, ToggleReduceBitmapsResolutionRBHdl ) );
}

SfxCommonPrintOptionsTabPage::~SfxCommonPrintOptionsTabPage()
{
    disposeOnce();
}

void SfxCommonPrintOptionsTabPage::dispose()
{
    m_pPrinterOutputRB.clear();
    m_pPrintFileOutputRB.clear();
    m_pReduceTransparencyCB.clear();
    m_pReduceTransparencyAutoRB.clear();
    m_pReduceTransparencyNoneRB.clear();
    m_pReduceGradientsCB.clear();
    m_pReduceGradientsStripesRB.clear();
    m_pReduceGradientsColorRB.clear();
    m_pReduceGradientsStepCountNF.clear();
    m_pReduceBitmapsCB.clear();
    m_pReduceBitmapsOptimalRB.clear();
    m_pReduceBitmapsNormalRB.clear();
    m_pReduceBitmapsResolutionRB.clear();
    m_pReduceBitmapsResolutionLB.clear();
    m_pReduceBitmapsTransparencyCB.clear();
    m_pConvertToGreyscalesCB.clear();
    m_pPDFCB.clear();
    m_pPaperSizeCB.clear();
    m_pPaperOrientationCB.clear();
    m_pTransparencyCB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SfxCommonPrintOptionsTabPage::Create( TabPageParent pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SfxCommonPrintOptionsTabPage>::Create( pParent.pParent, *rAttrSet );
}

vcl::Window* SfxCommonPrintOptionsTabPage::GetParentLabeledBy( const vcl::Window* pWindow ) const
{
    if ( pWindow == static_cast<vcl::Window *>(m_pReduceGradientsStepCountNF) )
        return m_pReduceGradientsStripesRB;
    else if ( pWindow == static_cast<vcl::Window *>(m_pReduceBitmapsResolutionLB) )
        return m_pReduceBitmapsResolutionRB;
    else
        return SfxTabPage::GetParentLabeledBy( pWindow );
}

vcl::Window* SfxCommonPrintOptionsTabPage::GetParentLabelFor( const vcl::Window* pWindow ) const
{
    if ( pWindow == static_cast<vcl::Window *>(m_pReduceGradientsStripesRB) )
        return m_pReduceGradientsStepCountNF;
    else if ( pWindow == static_cast<vcl::Window *>(m_pReduceBitmapsResolutionRB) )
        return m_pReduceBitmapsResolutionLB;
    else
        return SfxTabPage::GetParentLabelFor( pWindow );
}

bool SfxCommonPrintOptionsTabPage::FillItemSet( SfxItemSet* /*rSet*/ )
{
    SvtPrintWarningOptions  aWarnOptions;
    SvtPrinterOptions       aPrinterOptions;
    SvtPrintFileOptions     aPrintFileOptions;


    if( m_pPaperSizeCB->IsValueChangedFromSaved())
        aWarnOptions.SetPaperSize(m_pPaperSizeCB->IsChecked());
    if( m_pPaperOrientationCB->IsValueChangedFromSaved() )
        aWarnOptions.SetPaperOrientation(m_pPaperOrientationCB->IsChecked());

    if( m_pTransparencyCB->IsValueChangedFromSaved() )
        aWarnOptions.SetTransparency( m_pTransparencyCB->IsChecked() );

    ImplSaveControls( m_pPrinterOutputRB->IsChecked() ? &maPrinterOptions : &maPrintFileOptions );

    aPrinterOptions.SetPrinterOptions( maPrinterOptions );
    aPrintFileOptions.SetPrinterOptions( maPrintFileOptions );

    return false;
}

void SfxCommonPrintOptionsTabPage::Reset( const SfxItemSet* /*rSet*/ )
{
    SvtPrintWarningOptions  aWarnOptions;
    SvtPrinterOptions       aPrinterOptions;
    SvtPrintFileOptions     aPrintFileOptions;

    m_pPaperSizeCB->Check( aWarnOptions.IsPaperSize() );
    m_pPaperOrientationCB->Check( aWarnOptions.IsPaperOrientation() );

    m_pTransparencyCB->Check( aWarnOptions.IsTransparency() );

    m_pPaperSizeCB->SaveValue();
    m_pPaperOrientationCB->SaveValue();
    m_pTransparencyCB->SaveValue();

    aPrinterOptions.GetPrinterOptions( maPrinterOptions );
    aPrintFileOptions.GetPrinterOptions( maPrintFileOptions );
    if(m_pPrintFileOutputRB->IsChecked()){
       m_pPrinterOutputRB->Check();
    }

    ImplUpdateControls( m_pPrinterOutputRB->IsChecked() ? &maPrinterOptions : &maPrintFileOptions );
}

DeactivateRC SfxCommonPrintOptionsTabPage::DeactivatePage( SfxItemSet* pItemSet )
{
    if( pItemSet )
        FillItemSet( pItemSet );

    return DeactivateRC::LeavePage;
}

void SfxCommonPrintOptionsTabPage::ImplUpdateControls( const PrinterOptions* pCurrentOptions )
{
    m_pReduceTransparencyCB->Check( pCurrentOptions->IsReduceTransparency() );

    if( pCurrentOptions->GetReducedTransparencyMode() == PrinterTransparencyMode::Auto )
        m_pReduceTransparencyAutoRB->Check();
    else
        m_pReduceTransparencyNoneRB->Check( );

    m_pReduceGradientsCB->Check( pCurrentOptions->IsReduceGradients() );

    if( pCurrentOptions->GetReducedGradientMode() == PrinterGradientMode::Stripes )
        m_pReduceGradientsStripesRB->Check();
    else
        m_pReduceGradientsColorRB->Check();

    m_pReduceGradientsStepCountNF->SetValue( pCurrentOptions->GetReducedGradientStepCount() );

    m_pReduceBitmapsCB->Check( pCurrentOptions->IsReduceBitmaps() );

    if( pCurrentOptions->GetReducedBitmapMode() == PrinterBitmapMode::Optimal )
        m_pReduceBitmapsOptimalRB->Check();
    else if( pCurrentOptions->GetReducedBitmapMode() == PrinterBitmapMode::Normal )
        m_pReduceBitmapsNormalRB->Check();
    else
        m_pReduceBitmapsResolutionRB->Check();

    const sal_uInt16 nDPI = pCurrentOptions->GetReducedBitmapResolution();

    if( nDPI < aDPIArray[ 0 ] )
        m_pReduceBitmapsResolutionLB->SelectEntryPos( 0 );
    else
    {
        for( long i = DPI_COUNT - 1; i >= 0; i-- )
        {
            if( nDPI >= aDPIArray[ i ] )
            {
                m_pReduceBitmapsResolutionLB->SelectEntryPos( static_cast<sal_uInt16>(i) );
                i = -1;
            }
        }
    }

    m_pReduceBitmapsResolutionLB->SetText( m_pReduceBitmapsResolutionLB->GetSelectedEntry() );

    m_pReduceBitmapsTransparencyCB->Check( pCurrentOptions->IsReducedBitmapIncludesTransparency() );
    m_pConvertToGreyscalesCB->Check( pCurrentOptions->IsConvertToGreyscales() );
    m_pPDFCB->Check( pCurrentOptions->IsPDFAsStandardPrintJobFormat() );

    ClickReduceTransparencyCBHdl(m_pReduceTransparencyCB);
    ClickReduceGradientsCBHdl(m_pReduceGradientsCB);
    ClickReduceBitmapsCBHdl(m_pReduceBitmapsCB);
}

void SfxCommonPrintOptionsTabPage::ImplSaveControls( PrinterOptions* pCurrentOptions )
{
    pCurrentOptions->SetReduceTransparency( m_pReduceTransparencyCB->IsChecked() );
    pCurrentOptions->SetReducedTransparencyMode( m_pReduceTransparencyAutoRB->IsChecked() ? PrinterTransparencyMode::Auto : PrinterTransparencyMode::NONE );
    pCurrentOptions->SetReduceGradients( m_pReduceGradientsCB->IsChecked() );
    pCurrentOptions->SetReducedGradientMode( m_pReduceGradientsStripesRB->IsChecked() ? PrinterGradientMode::Stripes : PrinterGradientMode::Color  );
    pCurrentOptions->SetReducedGradientStepCount( static_cast<sal_uInt16>(m_pReduceGradientsStepCountNF->GetValue()) );
    pCurrentOptions->SetReduceBitmaps( m_pReduceBitmapsCB->IsChecked() );
    pCurrentOptions->SetReducedBitmapMode( m_pReduceBitmapsOptimalRB->IsChecked() ? PrinterBitmapMode::Optimal :
                                           ( m_pReduceBitmapsNormalRB->IsChecked() ? PrinterBitmapMode::Normal : PrinterBitmapMode::Resolution ) );
    pCurrentOptions->SetReducedBitmapResolution( aDPIArray[ std::min<sal_uInt16>( m_pReduceBitmapsResolutionLB->GetSelectedEntryPos(),
                                                                   SAL_N_ELEMENTS(aDPIArray) - 1 ) ] );
    pCurrentOptions->SetReducedBitmapIncludesTransparency( m_pReduceBitmapsTransparencyCB->IsChecked() );
    pCurrentOptions->SetConvertToGreyscales( m_pConvertToGreyscalesCB->IsChecked() );
    bool bOrigBackEnd = pCurrentOptions->IsPDFAsStandardPrintJobFormat();
    if (bOrigBackEnd != m_pPDFCB->IsChecked())
    {
        pCurrentOptions->SetPDFAsStandardPrintJobFormat( m_pPDFCB->IsChecked() );
        svtools::executeRestartDialog(
                comphelper::getProcessComponentContext(), nullptr,
                svtools::RESTART_REASON_PDF_AS_STANDARD_JOB_FORMAT);
    }
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ClickReduceTransparencyCBHdl, Button*, void )
{
    const bool bReduceTransparency = m_pReduceTransparencyCB->IsChecked();

    m_pReduceTransparencyAutoRB->Enable( bReduceTransparency );
    m_pReduceTransparencyNoneRB->Enable( bReduceTransparency );

    m_pTransparencyCB->Enable( !bReduceTransparency );
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ClickReduceGradientsCBHdl, Button*, void )
{
    const bool bEnable = m_pReduceGradientsCB->IsChecked();

    m_pReduceGradientsStripesRB->Enable( bEnable );
    m_pReduceGradientsColorRB->Enable( bEnable );
    m_pReduceGradientsStepCountNF->Enable( bEnable );

    ToggleReduceGradientsStripesRBHdl(*m_pReduceGradientsStripesRB);
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ClickReduceBitmapsCBHdl, Button*, void )
{
    const bool bEnable = m_pReduceBitmapsCB->IsChecked();

    m_pReduceBitmapsOptimalRB->Enable( bEnable );
    m_pReduceBitmapsNormalRB->Enable( bEnable );
    m_pReduceBitmapsResolutionRB->Enable( bEnable );
    m_pReduceBitmapsTransparencyCB->Enable( bEnable );
    m_pReduceBitmapsResolutionLB->Enable( bEnable );

    ToggleReduceBitmapsResolutionRBHdl(*m_pReduceBitmapsResolutionRB);
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ToggleReduceGradientsStripesRBHdl, RadioButton&, void )
{
    const bool bEnable = m_pReduceGradientsCB->IsChecked() && m_pReduceGradientsStripesRB->IsChecked();

    m_pReduceGradientsStepCountNF->Enable( bEnable );
}

IMPL_LINK_NOARG( SfxCommonPrintOptionsTabPage, ToggleReduceBitmapsResolutionRBHdl, RadioButton&, void )
{
    const bool bEnable = m_pReduceBitmapsCB->IsChecked() && m_pReduceBitmapsResolutionRB->IsChecked();

    m_pReduceBitmapsResolutionLB->Enable( bEnable );
}

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleOutputPrinterRBHdl, RadioButton&, rButton, void )
{
    if( rButton.IsChecked() )
    {
        ImplUpdateControls( &maPrinterOptions );
        bOutputForPrinter = true;
    }
    else
        ImplSaveControls( &maPrinterOptions );
}

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleOutputPrintFileRBHdl, RadioButton&, rButton, void )
{
    if( rButton.IsChecked() )
    {
        ImplUpdateControls( &maPrintFileOptions );
        bOutputForPrinter = false;
        m_pPDFCB->Disable();
    }
    else
    {
        ImplSaveControls( &maPrintFileOptions );
        m_pPDFCB->Enable();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
