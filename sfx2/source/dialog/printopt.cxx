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
#include <vcl/msgbox.hxx>
#include <unotools/printwarningoptions.hxx>
#include <svtools/printoptions.hxx>
#include <svtools/restartdialog.hxx>
#include <svl/flagitem.hxx>

#include <comphelper/processfactory.hxx>

#include <sfx2/sfxresid.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/printopt.hxx>

static sal_uInt16   aDPIArray[] = { 72, 96, 150, 200, 300, 600 };
static sal_Bool     bOutputForPrinter = sal_True;

#define DPI_COUNT (sizeof(aDPIArray)/sizeof(aDPIArray[0 ]))

SfxCommonPrintOptionsTabPage::SfxCommonPrintOptionsTabPage( Window* pParent, const SfxItemSet& rSet )
    : SfxTabPage(pParent, "OptPrintPage", "sfx/ui/optprintpage.ui", rSet)
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
        m_pPrinterOutputRB->Check( true );
    }
    else
    {
        m_pPrintFileOutputRB->Check( true );
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
}

SfxTabPage* SfxCommonPrintOptionsTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return( new SfxCommonPrintOptionsTabPage( pParent, rAttrSet ) );
}

Window* SfxCommonPrintOptionsTabPage::GetParentLabeledBy( const Window* pWindow ) const
{
    if ( pWindow == (Window *)m_pReduceGradientsStepCountNF )
        return m_pReduceGradientsStripesRB;
    else if ( pWindow == (Window *)m_pReduceBitmapsResolutionLB )
        return m_pReduceBitmapsResolutionRB;
    else
        return SfxTabPage::GetParentLabeledBy( pWindow );
}

Window* SfxCommonPrintOptionsTabPage::GetParentLabelFor( const Window* pWindow ) const
{
    if ( pWindow == (Window *)m_pReduceGradientsStripesRB )
        return m_pReduceGradientsStepCountNF;
    else if ( pWindow == (Window *)m_pReduceBitmapsResolutionRB )
        return m_pReduceBitmapsResolutionLB;
    else
        return SfxTabPage::GetParentLabelFor( pWindow );
}

bool SfxCommonPrintOptionsTabPage::FillItemSet( SfxItemSet& /*rSet*/ )
{
    SvtPrintWarningOptions  aWarnOptions;
    SvtPrinterOptions       aPrinterOptions;
    SvtPrintFileOptions     aPrintFileOptions;
    bool                    bModified = false;


    if( TriState(m_pPaperSizeCB->IsChecked()) != m_pPaperSizeCB->GetSavedValue())
        aWarnOptions.SetPaperSize(m_pPaperSizeCB->IsChecked());
    if( TriState(m_pPaperOrientationCB->IsChecked()) != m_pPaperOrientationCB->GetSavedValue() )
        aWarnOptions.SetPaperOrientation(m_pPaperOrientationCB->IsChecked());

    if( TriState(m_pTransparencyCB->IsChecked()) != m_pTransparencyCB->GetSavedValue() )
        aWarnOptions.SetTransparency( m_pTransparencyCB->IsChecked() );

    ImplSaveControls( m_pPrinterOutputRB->IsChecked() ? &maPrinterOptions : &maPrintFileOptions );

    aPrinterOptions.SetPrinterOptions( maPrinterOptions );
    aPrintFileOptions.SetPrinterOptions( maPrintFileOptions );

    return bModified;
}

void SfxCommonPrintOptionsTabPage::Reset( const SfxItemSet& /*rSet*/ )
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

    ImplUpdateControls( m_pPrinterOutputRB->IsChecked() ? &maPrinterOptions : &maPrintFileOptions );
}

int SfxCommonPrintOptionsTabPage::DeactivatePage( SfxItemSet* pItemSet )
{
    if( pItemSet )
        FillItemSet( *pItemSet );

    return LEAVE_PAGE;
}

void SfxCommonPrintOptionsTabPage::ImplUpdateControls( const PrinterOptions* pCurrentOptions )
{
    m_pReduceTransparencyCB->Check( pCurrentOptions->IsReduceTransparency() );

    if( pCurrentOptions->GetReducedTransparencyMode() == PRINTER_TRANSPARENCY_AUTO )
        m_pReduceTransparencyAutoRB->Check( true );
    else
        m_pReduceTransparencyNoneRB->Check( true );

    m_pReduceGradientsCB->Check( pCurrentOptions->IsReduceGradients() );

    if( pCurrentOptions->GetReducedGradientMode() == PRINTER_GRADIENT_STRIPES )
        m_pReduceGradientsStripesRB->Check( true );
    else
        m_pReduceGradientsColorRB->Check( true );

    m_pReduceGradientsStepCountNF->SetValue( pCurrentOptions->GetReducedGradientStepCount() );

    m_pReduceBitmapsCB->Check( pCurrentOptions->IsReduceBitmaps() );

    if( pCurrentOptions->GetReducedBitmapMode() == PRINTER_BITMAP_OPTIMAL )
        m_pReduceBitmapsOptimalRB->Check( true );
    else if( pCurrentOptions->GetReducedBitmapMode() == PRINTER_BITMAP_NORMAL )
        m_pReduceBitmapsNormalRB->Check( true );
    else
        m_pReduceBitmapsResolutionRB->Check( true );

    const sal_uInt16 nDPI = pCurrentOptions->GetReducedBitmapResolution();

    if( nDPI < aDPIArray[ 0 ] )
        m_pReduceBitmapsResolutionLB->SelectEntryPos( 0 );
    else
    {
        for( long i = ( DPI_COUNT - 1 ); i >= 0; i-- )
        {
            if( nDPI >= aDPIArray[ i ] )
            {
                m_pReduceBitmapsResolutionLB->SelectEntryPos( (sal_uInt16) i );
                i = -1;
            }
        }
    }

    m_pReduceBitmapsResolutionLB->SetText( m_pReduceBitmapsResolutionLB->GetEntry( m_pReduceBitmapsResolutionLB->GetSelectEntryPos() ) );

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
    pCurrentOptions->SetReducedTransparencyMode( m_pReduceTransparencyAutoRB->IsChecked() ? PRINTER_TRANSPARENCY_AUTO : PRINTER_TRANSPARENCY_NONE );
    pCurrentOptions->SetReduceGradients( m_pReduceGradientsCB->IsChecked() );
    pCurrentOptions->SetReducedGradientMode( m_pReduceGradientsStripesRB->IsChecked() ? PRINTER_GRADIENT_STRIPES : PRINTER_GRADIENT_COLOR  );
    pCurrentOptions->SetReducedGradientStepCount( (sal_uInt16) m_pReduceGradientsStepCountNF->GetValue() );
    pCurrentOptions->SetReduceBitmaps( m_pReduceBitmapsCB->IsChecked() );
    pCurrentOptions->SetReducedBitmapMode( m_pReduceBitmapsOptimalRB->IsChecked() ? PRINTER_BITMAP_OPTIMAL :
                                           ( m_pReduceBitmapsNormalRB->IsChecked() ? PRINTER_BITMAP_NORMAL : PRINTER_BITMAP_RESOLUTION ) );
    pCurrentOptions->SetReducedBitmapResolution( aDPIArray[ std::min( (sal_uInt16) m_pReduceBitmapsResolutionLB->GetSelectEntryPos(),
                                                            (sal_uInt16)( (sizeof (aDPIArray) / sizeof (aDPIArray[0])) - 1 ) ) ] );
    pCurrentOptions->SetReducedBitmapIncludesTransparency( m_pReduceBitmapsTransparencyCB->IsChecked() );
    pCurrentOptions->SetConvertToGreyscales( m_pConvertToGreyscalesCB->IsChecked() );
    bool bOrigBackEnd = pCurrentOptions->IsPDFAsStandardPrintJobFormat();
    if (bOrigBackEnd != m_pPDFCB->IsChecked())
    {
        pCurrentOptions->SetPDFAsStandardPrintJobFormat( m_pPDFCB->IsChecked() );
            svtools::executeRestartDialog(
                comphelper::getProcessComponentContext(), 0,
                svtools::RESTART_REASON_PDF_AS_STANDARD_JOB_FORMAT);
    }
}

IMPL_LINK( SfxCommonPrintOptionsTabPage, ClickReduceTransparencyCBHdl, CheckBox*, pBox )
{
    (void)pBox; //unused
    const sal_Bool bReduceTransparency = m_pReduceTransparencyCB->IsChecked();

    m_pReduceTransparencyAutoRB->Enable( bReduceTransparency );
    m_pReduceTransparencyNoneRB->Enable( bReduceTransparency );

    m_pTransparencyCB->Enable( !bReduceTransparency );

    return 0;
}

IMPL_LINK( SfxCommonPrintOptionsTabPage, ClickReduceGradientsCBHdl, CheckBox*, pBox )
{
    (void)pBox; //unused
    const sal_Bool bEnable = m_pReduceGradientsCB->IsChecked();

    m_pReduceGradientsStripesRB->Enable( bEnable );
    m_pReduceGradientsColorRB->Enable( bEnable );
    m_pReduceGradientsStepCountNF->Enable( bEnable );

    ToggleReduceGradientsStripesRBHdl(m_pReduceGradientsStripesRB);

    return 0;
}

IMPL_LINK( SfxCommonPrintOptionsTabPage, ClickReduceBitmapsCBHdl, CheckBox*, pBox )
{
    (void)pBox; //unused
    const sal_Bool bEnable = m_pReduceBitmapsCB->IsChecked();

    m_pReduceBitmapsOptimalRB->Enable( bEnable );
    m_pReduceBitmapsNormalRB->Enable( bEnable );
    m_pReduceBitmapsResolutionRB->Enable( bEnable );
    m_pReduceBitmapsTransparencyCB->Enable( bEnable );
    m_pReduceBitmapsResolutionLB->Enable( bEnable );

    ToggleReduceBitmapsResolutionRBHdl(m_pReduceBitmapsResolutionRB);

    return 0;
}

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleReduceGradientsStripesRBHdl, RadioButton*, pButton )
{
    (void)pButton; //unused
    const sal_Bool bEnable = m_pReduceGradientsCB->IsChecked() && m_pReduceGradientsStripesRB->IsChecked();

    m_pReduceGradientsStepCountNF->Enable( bEnable );

    return 0;
}

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleReduceBitmapsResolutionRBHdl, RadioButton*, pButton )
{
    (void)pButton; //unused
    const sal_Bool bEnable = m_pReduceBitmapsCB->IsChecked() && m_pReduceBitmapsResolutionRB->IsChecked();

    m_pReduceBitmapsResolutionLB->Enable( bEnable );

    return 0;
}

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleOutputPrinterRBHdl, RadioButton*, pButton )
{
    if( pButton->IsChecked() )
    {
        ImplUpdateControls( &maPrinterOptions );
        bOutputForPrinter = sal_True;
    }
    else
        ImplSaveControls( &maPrinterOptions );

    return 0;
}

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleOutputPrintFileRBHdl, RadioButton*, pButton )
{
    if( pButton->IsChecked() )
    {
        ImplUpdateControls( &maPrintFileOptions );
        bOutputForPrinter = sal_False;
        m_pPDFCB->Disable();
    }
    else
    {
        ImplSaveControls( &maPrintFileOptions );
        m_pPDFCB->Enable();
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
