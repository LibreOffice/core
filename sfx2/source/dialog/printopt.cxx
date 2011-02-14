/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#ifndef GCC
#endif
#include <vcl/msgbox.hxx>
#include <unotools/printwarningoptions.hxx>
#include <svtools/printoptions.hxx>
#include <svl/flagitem.hxx>


#include "printopt.hrc"
#include "dialog.hrc"
#include "sfx2/sfxresid.hxx"
#include <sfx2/viewsh.hxx>
#include <sfx2/printopt.hxx>

// -----------
// - statics -
// -----------

static sal_uInt16   aDPIArray[] = { 72, 96, 150, 200, 300, 600 };
static sal_Bool     bOutputForPrinter = sal_True;

#define DPI_COUNT (sizeof(aDPIArray)/sizeof(aDPIArray[0 ]))

// --------------------------------
// - SfxCommonPrintOptionsTabPage -
// --------------------------------

SfxCommonPrintOptionsTabPage::SfxCommonPrintOptionsTabPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage( pParent, SfxResId( TP_COMMONPRINTOPTIONS ), rSet ),

    aReduceGB( this, SfxResId( GB_REDUCE ) ),
    aOutputTypeFT( this, SfxResId( FT_OUTPUTTYPE ) ),
    aPrinterOutputRB( this, SfxResId( RB_PRINTEROUTPUT ) ),
    aPrintFileOutputRB( this, SfxResId( RB_PRINTFILEOUTPUT ) ),
    aOutputGB( this, SfxResId( GB_OUTPUT ) ),
    aReduceTransparencyCB( this, SfxResId( CB_REDUCETRANSPARENCY ) ),
    aReduceTransparencyAutoRB( this, SfxResId( RB_REDUCETRANSPARENCY_AUTO ) ),
    aReduceTransparencyNoneRB( this, SfxResId( RB_REDUCETRANSPARENCY_NONE ) ),
    aReduceGradientsCB( this, SfxResId( CB_REDUCEGRADIENTS ) ),
    aReduceGradientsStripesRB( this, SfxResId( RB_REDUCEGRADIENTS_STRIPES ) ),
    aReduceGradientsColorRB( this, SfxResId( RB_REDUCEGRADIENTS_COLOR ) ),
    aReduceGradientsStepCountNF( this, SfxResId( NF_REDUCEGRADIENTS_STEPCOUNT ) ),
    aReduceBitmapsCB( this, SfxResId( CB_REDUCEBITMAPS ) ),
    aReduceBitmapsOptimalRB( this, SfxResId( RB_REDUCEBITMAPS_OPTIMAL ) ),
    aReduceBitmapsNormalRB( this, SfxResId( RB_REDUCEBITMAPS_NORMAL ) ),
    aReduceBitmapsResolutionRB( this, SfxResId( RB_REDUCEBITMAPS_RESOLUTION ) ),
    aReduceBitmapsResolutionLB( this, SfxResId( LB_REDUCEBITMAPS_RESOLUTION ) ),
    aReduceBitmapsTransparencyCB( this, SfxResId( CB_REDUCEBITMAPS_TRANSPARENCY ) ),
    aConvertToGreyscalesCB( this, SfxResId( CB_CONVERTTOGREYSCALES ) ),
    aWarnGB( this, SfxResId( GB_PRINT_WARN ) ),
    aPaperSizeCB( this, SfxResId( CB_PAPERSIZE ) ),
    aPaperOrientationCB( this, SfxResId( CB_PAPERORIENTATION ) ),
    aTransparencyCB( this, SfxResId( CB_TRANSPARENCY ) )
{
    FreeResource();

    aOutputGB.SetStyle( aOutputGB.GetStyle() | WB_NOLABEL );

    if( bOutputForPrinter )
    {
        aPrinterOutputRB.Check( sal_True );
        aOutputGB.SetText( OutputDevice::GetNonMnemonicString( aPrinterOutputRB.GetText() ) );
    }
    else
    {
        aPrintFileOutputRB.Check( sal_True );
        aOutputGB.SetText( OutputDevice::GetNonMnemonicString( aPrintFileOutputRB.GetText() ) );
    }

    aPrinterOutputRB.SetToggleHdl( LINK( this, SfxCommonPrintOptionsTabPage, ToggleOutputPrinterRBHdl ) );
    aPrintFileOutputRB.SetToggleHdl( LINK( this, SfxCommonPrintOptionsTabPage, ToggleOutputPrintFileRBHdl ) );

    aReduceTransparencyCB.SetClickHdl( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceTransparencyCBHdl ) );
    aReduceGradientsCB.SetClickHdl( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceGradientsCBHdl ) );
    aReduceBitmapsCB.SetClickHdl( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceBitmapsCBHdl ) );

    aReduceGradientsStripesRB.SetToggleHdl( LINK( this, SfxCommonPrintOptionsTabPage, ToggleReduceGradientsStripesRBHdl ) );
    aReduceBitmapsResolutionRB.SetToggleHdl( LINK( this, SfxCommonPrintOptionsTabPage, ToggleReduceBitmapsResolutionRBHdl ) );

    // --> PB 2008-06-09 #i89164#
    // calculate dynamically the width of radiobutton and listbox
    const long nOffset = 10;
    Size aOldSize = aReduceBitmapsResolutionRB.GetSizePixel();
    Size aNewSize = aReduceBitmapsResolutionRB.GetOptimalSize( WINDOWSIZE_PREFERRED );
    aNewSize.Width() += nOffset;
    aNewSize.Height() = aOldSize.Height();
    long nDelta = aOldSize.Width() - aNewSize.Width();
    aReduceBitmapsResolutionRB.SetSizePixel( aNewSize );
    Point aPos = aReduceBitmapsResolutionLB.GetPosPixel();
    aPos.X() -= nDelta;
    aOldSize = aReduceBitmapsResolutionLB.GetSizePixel();
    aNewSize = aReduceBitmapsResolutionLB.GetOptimalSize( WINDOWSIZE_PREFERRED );
    aNewSize.Width() += nOffset;
    aNewSize.Height() = aOldSize.Height();
    aReduceBitmapsResolutionLB.SetPosSizePixel( aPos, aNewSize );
    // <--
}

// -----------------------------------------------------------------------------

SfxCommonPrintOptionsTabPage::~SfxCommonPrintOptionsTabPage()
{
}

// -----------------------------------------------------------------------------

SfxTabPage* SfxCommonPrintOptionsTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return( new SfxCommonPrintOptionsTabPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------------

Window* SfxCommonPrintOptionsTabPage::GetParentLabeledBy( const Window* pWindow ) const
{
    if ( pWindow == (Window *)&aReduceGradientsStepCountNF )
        return (Window *)&aReduceGradientsStripesRB;
    else if ( pWindow == (Window *)&aReduceBitmapsResolutionLB )
        return (Window *)&aReduceBitmapsResolutionRB;
    else
        return SfxTabPage::GetParentLabeledBy( pWindow );
}

Window* SfxCommonPrintOptionsTabPage::GetParentLabelFor( const Window* pWindow ) const
{
    if ( pWindow == (Window *)&aReduceGradientsStripesRB )
        return (Window *)&aReduceGradientsStepCountNF;
    else if ( pWindow == (Window *)&aReduceBitmapsResolutionRB )
        return (Window *)&aReduceBitmapsResolutionLB;
    else
        return SfxTabPage::GetParentLabelFor( pWindow );
}

// -----------------------------------------------------------------------------

sal_Bool SfxCommonPrintOptionsTabPage::FillItemSet( SfxItemSet& /*rSet*/ )
{
    SvtPrintWarningOptions  aWarnOptions;
    SvtPrinterOptions       aPrinterOptions;
    SvtPrintFileOptions     aPrintFileOptions;
    sal_Bool                    bModified = sal_False;


    if( aPaperSizeCB.IsChecked() != aPaperSizeCB.GetSavedValue())
        aWarnOptions.SetPaperSize(aPaperSizeCB.IsChecked());
    if( aPaperOrientationCB.IsChecked() != aPaperOrientationCB.GetSavedValue() )
        aWarnOptions.SetPaperOrientation(aPaperOrientationCB.IsChecked());

    if( aTransparencyCB.IsChecked() != aTransparencyCB.GetSavedValue() )
        aWarnOptions.SetTransparency( aTransparencyCB.IsChecked() );

    ImplSaveControls( aPrinterOutputRB.IsChecked() ? &maPrinterOptions : &maPrintFileOptions );

    aPrinterOptions.SetPrinterOptions( maPrinterOptions );
    aPrintFileOptions.SetPrinterOptions( maPrintFileOptions );

    return bModified;
}

// -----------------------------------------------------------------------------

void SfxCommonPrintOptionsTabPage::Reset( const SfxItemSet& /*rSet*/ )
{
    SvtPrintWarningOptions  aWarnOptions;
    SvtPrinterOptions       aPrinterOptions;
    SvtPrintFileOptions     aPrintFileOptions;

    aPaperSizeCB.Check( aWarnOptions.IsPaperSize() );
    aPaperOrientationCB.Check( aWarnOptions.IsPaperOrientation() );

    aTransparencyCB.Check( aWarnOptions.IsTransparency() );

    aPaperSizeCB.SaveValue();
    aPaperOrientationCB.SaveValue();
    aTransparencyCB.SaveValue();

    aPrinterOptions.GetPrinterOptions( maPrinterOptions );
    aPrintFileOptions.GetPrinterOptions( maPrintFileOptions );

    ImplUpdateControls( aPrinterOutputRB.IsChecked() ? &maPrinterOptions : &maPrintFileOptions );

    // --> OD 2008-06-25 #i63982#
    ImplSetAccessibleNames();
    // <--
}

// -----------------------------------------------------------------------------

int SfxCommonPrintOptionsTabPage::DeactivatePage( SfxItemSet* pItemSet )
{
    if( pItemSet )
        FillItemSet( *pItemSet );

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------------

void SfxCommonPrintOptionsTabPage::ImplUpdateControls( const PrinterOptions* pCurrentOptions )
{
    aReduceTransparencyCB.Check( pCurrentOptions->IsReduceTransparency() );

    if( pCurrentOptions->GetReducedTransparencyMode() == PRINTER_TRANSPARENCY_AUTO )
        aReduceTransparencyAutoRB.Check( sal_True );
    else
        aReduceTransparencyNoneRB.Check( sal_True );

    aReduceGradientsCB.Check( pCurrentOptions->IsReduceGradients() );

    if( pCurrentOptions->GetReducedGradientMode() == PRINTER_GRADIENT_STRIPES )
        aReduceGradientsStripesRB.Check( sal_True );
    else
        aReduceGradientsColorRB.Check( sal_True );

    aReduceGradientsStepCountNF.SetValue( pCurrentOptions->GetReducedGradientStepCount() );

    aReduceBitmapsCB.Check( pCurrentOptions->IsReduceBitmaps() );

    if( pCurrentOptions->GetReducedBitmapMode() == PRINTER_BITMAP_OPTIMAL )
        aReduceBitmapsOptimalRB.Check( sal_True );
    else if( pCurrentOptions->GetReducedBitmapMode() == PRINTER_BITMAP_NORMAL )
        aReduceBitmapsNormalRB.Check( sal_True );
    else
        aReduceBitmapsResolutionRB.Check( sal_True );

    const sal_uInt16 nDPI = pCurrentOptions->GetReducedBitmapResolution();

    if( nDPI < aDPIArray[ 0 ] )
        aReduceBitmapsResolutionLB.SelectEntryPos( 0 );
    else
    {
        for( long i = ( DPI_COUNT - 1 ); i >= 0; i-- )
        {
            if( nDPI >= aDPIArray[ i ] )
            {
                aReduceBitmapsResolutionLB.SelectEntryPos( (sal_uInt16) i );
                i = -1;
            }
        }
    }

    aReduceBitmapsResolutionLB.SetText( aReduceBitmapsResolutionLB.GetEntry( aReduceBitmapsResolutionLB.GetSelectEntryPos() ) );

    aReduceBitmapsTransparencyCB.Check( pCurrentOptions->IsReducedBitmapIncludesTransparency() );
    aConvertToGreyscalesCB.Check( pCurrentOptions->IsConvertToGreyscales() );

    ClickReduceTransparencyCBHdl( &aReduceTransparencyCB );
    ClickReduceGradientsCBHdl( &aReduceGradientsCB );
    ClickReduceBitmapsCBHdl( &aReduceBitmapsCB );
}

// -----------------------------------------------------------------------------

void SfxCommonPrintOptionsTabPage::ImplSetAccessibleNames()
{
    static const String cSeparator = String::CreateFromAscii( " - " );

    String sReduceText = aReduceGB.GetDisplayText();
    sReduceText += cSeparator;

    String sAccessibleName = sReduceText;
    sAccessibleName += aPrinterOutputRB.GetDisplayText();
    aPrinterOutputRB.SetAccessibleName( sAccessibleName );

    sAccessibleName = sReduceText;
    sAccessibleName += aPrintFileOutputRB.GetDisplayText();
    aPrintFileOutputRB.SetAccessibleName( sAccessibleName );

    String sOutputText = sReduceText;
    sOutputText += aOutputGB.GetDisplayText();
    sOutputText += cSeparator;

    sAccessibleName = sOutputText;
    sAccessibleName += aReduceTransparencyCB.GetDisplayText();
    aReduceTransparencyCB.SetAccessibleName( sAccessibleName );

    String sTransparencyText = aReduceTransparencyCB.GetAccessibleName();
    sTransparencyText += cSeparator;

    sAccessibleName = sTransparencyText;
    sAccessibleName += aReduceTransparencyAutoRB.GetDisplayText();
    aReduceTransparencyAutoRB.SetAccessibleName( sAccessibleName );

    sAccessibleName = sTransparencyText;
    sAccessibleName += aReduceTransparencyNoneRB.GetDisplayText();
    aReduceTransparencyNoneRB.SetAccessibleName( sAccessibleName );

    sAccessibleName = sOutputText;
    sAccessibleName += aReduceGradientsCB.GetDisplayText();
    aReduceGradientsCB.SetAccessibleName( sAccessibleName );

    String sGradientText = aReduceGradientsCB.GetAccessibleName();
    sGradientText += cSeparator;

    sAccessibleName = sGradientText;
    sAccessibleName += aReduceGradientsStripesRB.GetDisplayText();
    aReduceGradientsStripesRB.SetAccessibleName( sAccessibleName );

    sAccessibleName = aReduceGradientsStripesRB.GetAccessibleName();
    aReduceGradientsStepCountNF.SetAccessibleName( sAccessibleName );

    sAccessibleName = sGradientText;
    sAccessibleName += aReduceGradientsColorRB.GetDisplayText();
    aReduceGradientsColorRB.SetAccessibleName( sAccessibleName );

    sAccessibleName = sOutputText;
    sAccessibleName += aReduceBitmapsCB.GetDisplayText();
    aReduceBitmapsCB.SetAccessibleName( sAccessibleName );

    String sBitmapText = aReduceBitmapsCB.GetAccessibleName();
    sBitmapText += cSeparator;

    sAccessibleName = sBitmapText;
    sAccessibleName += aReduceBitmapsOptimalRB.GetDisplayText();
    aReduceBitmapsOptimalRB.SetAccessibleName( sAccessibleName );

    sAccessibleName = sBitmapText;
    sAccessibleName += aReduceBitmapsNormalRB.GetDisplayText();
    aReduceBitmapsNormalRB.SetAccessibleName( sAccessibleName );

    sAccessibleName = sBitmapText;
    sAccessibleName += aReduceBitmapsResolutionRB.GetDisplayText();
    aReduceBitmapsResolutionRB.SetAccessibleName( sAccessibleName );

    sAccessibleName = aReduceBitmapsResolutionRB.GetAccessibleName();
    aReduceBitmapsResolutionLB.SetAccessibleName( sAccessibleName );

    sAccessibleName = sBitmapText;
    sAccessibleName += aReduceBitmapsTransparencyCB.GetDisplayText();
    aReduceBitmapsTransparencyCB.SetAccessibleName( sAccessibleName );

    sAccessibleName = sOutputText;
    sAccessibleName += aConvertToGreyscalesCB.GetDisplayText();
    aConvertToGreyscalesCB.SetAccessibleName( sAccessibleName );

    String sWarnText = aWarnGB.GetDisplayText();
    sWarnText += cSeparator;

    sAccessibleName = sWarnText;
    sAccessibleName += aPaperSizeCB.GetDisplayText();
    aPaperSizeCB.SetAccessibleName( sAccessibleName );

    sAccessibleName = sWarnText;
    sAccessibleName += aPaperOrientationCB.GetDisplayText();
    aPaperOrientationCB.SetAccessibleName( sAccessibleName );

    sAccessibleName = sWarnText;
    sAccessibleName += aTransparencyCB.GetDisplayText();
    aTransparencyCB.SetAccessibleName( sAccessibleName );
}

// -----------------------------------------------------------------------------

void SfxCommonPrintOptionsTabPage::ImplSaveControls( PrinterOptions* pCurrentOptions )
{
    pCurrentOptions->SetReduceTransparency( aReduceTransparencyCB.IsChecked() );
    pCurrentOptions->SetReducedTransparencyMode( aReduceTransparencyAutoRB.IsChecked() ? PRINTER_TRANSPARENCY_AUTO : PRINTER_TRANSPARENCY_NONE );
    pCurrentOptions->SetReduceGradients( aReduceGradientsCB.IsChecked() );
    pCurrentOptions->SetReducedGradientMode( aReduceGradientsStripesRB.IsChecked() ? PRINTER_GRADIENT_STRIPES : PRINTER_GRADIENT_COLOR  );
    pCurrentOptions->SetReducedGradientStepCount( (sal_uInt16) aReduceGradientsStepCountNF.GetValue() );
    pCurrentOptions->SetReduceBitmaps( aReduceBitmapsCB.IsChecked() );
    pCurrentOptions->SetReducedBitmapMode( aReduceBitmapsOptimalRB.IsChecked() ? PRINTER_BITMAP_OPTIMAL :
                                           ( aReduceBitmapsNormalRB.IsChecked() ? PRINTER_BITMAP_NORMAL : PRINTER_BITMAP_RESOLUTION ) );
    pCurrentOptions->SetReducedBitmapResolution( aDPIArray[ Min( (sal_uInt16) aReduceBitmapsResolutionLB.GetSelectEntryPos(),
                                                            (sal_uInt16)( sizeof( aDPIArray ) / sizeof( aDPIArray[ 0 ] ) - 1 ) ) ] );
    pCurrentOptions->SetReducedBitmapIncludesTransparency( aReduceBitmapsTransparencyCB.IsChecked() );
    pCurrentOptions->SetConvertToGreyscales( aConvertToGreyscalesCB.IsChecked() );
}

// -----------------------------------------------------------------------------

IMPL_LINK( SfxCommonPrintOptionsTabPage, ClickReduceTransparencyCBHdl, CheckBox*, pBox )
{
    (void)pBox; //unused
    const sal_Bool bReduceTransparency = aReduceTransparencyCB.IsChecked();

    aReduceTransparencyAutoRB.Enable( bReduceTransparency );
    aReduceTransparencyNoneRB.Enable( bReduceTransparency );

    aTransparencyCB.Enable( !bReduceTransparency );

    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SfxCommonPrintOptionsTabPage, ClickReduceGradientsCBHdl, CheckBox*, pBox )
{
    (void)pBox; //unused
    const sal_Bool bEnable = aReduceGradientsCB.IsChecked();

    aReduceGradientsStripesRB.Enable( bEnable );
    aReduceGradientsColorRB.Enable( bEnable );
    aReduceGradientsStepCountNF.Enable( bEnable );

    ToggleReduceGradientsStripesRBHdl( &aReduceGradientsStripesRB );

    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SfxCommonPrintOptionsTabPage, ClickReduceBitmapsCBHdl, CheckBox*, pBox )
{
    (void)pBox; //unused
    const sal_Bool bEnable = aReduceBitmapsCB.IsChecked();

    aReduceBitmapsOptimalRB.Enable( bEnable );
    aReduceBitmapsNormalRB.Enable( bEnable );
    aReduceBitmapsResolutionRB.Enable( bEnable );
    aReduceBitmapsTransparencyCB.Enable( bEnable );
    aReduceBitmapsResolutionLB.Enable( bEnable );

    ToggleReduceBitmapsResolutionRBHdl( &aReduceBitmapsResolutionRB );

    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleReduceGradientsStripesRBHdl, RadioButton*, pButton )
{
    (void)pButton; //unused
    const sal_Bool bEnable = aReduceGradientsCB.IsChecked() && aReduceGradientsStripesRB.IsChecked();

    aReduceGradientsStepCountNF.Enable( bEnable );

    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleReduceBitmapsResolutionRBHdl, RadioButton*, pButton )
{
    (void)pButton; //unused
    const sal_Bool bEnable = aReduceBitmapsCB.IsChecked() && aReduceBitmapsResolutionRB.IsChecked();

    aReduceBitmapsResolutionLB.Enable( bEnable );

    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleOutputPrinterRBHdl, RadioButton*, pButton )
{
    if( pButton->IsChecked() )
    {
        aOutputGB.SetText( OutputDevice::GetNonMnemonicString( pButton->GetText() ) );
        ImplUpdateControls( &maPrinterOptions );
        bOutputForPrinter = sal_True;
        // --> OD 2008-06-25 #i63982#
        ImplSetAccessibleNames();
        // <--
    }
    else
        ImplSaveControls( &maPrinterOptions );

    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleOutputPrintFileRBHdl, RadioButton*, pButton )
{
    if( pButton->IsChecked() )
    {
        aOutputGB.SetText( OutputDevice::GetNonMnemonicString( pButton->GetText() ) );
        ImplUpdateControls( &maPrintFileOptions );
        bOutputForPrinter = sal_False;
        // --> OD 2008-06-25 #i63982#
        ImplSetAccessibleNames();
        // <--
    }
    else
        ImplSaveControls( &maPrintFileOptions );

    return 0;
}

// -------------------------------
// - TransparencyPrintWarningBox -
// -------------------------------

TransparencyPrintWarningBox::TransparencyPrintWarningBox( Window* pParent ) :
    ModalDialog( pParent, SfxResId( RID_WARN_PRINTTRANSPARENCY ) ),
    aWarnFI     ( this, SfxResId( FI_PRINTTRANSWARN ) ),
    aWarnFT     ( this, SfxResId( FT_PRINTTRANSWARN ) ),
    aYesBtn     ( this, SfxResId( BTN_PRINTTRANS_YES ) ),
    aNoBtn      ( this, SfxResId( BTN_PRINTTRANS_NO ) ),
    aCancelBtn  ( this, SfxResId( BTN_PRINTTRANS_CANCEL ) ),
    aNoWarnCB   ( this, SfxResId( CBX_NOPRINTTRANSWARN ) )
{
    FreeResource();

    aWarnFT.SetStyle( aWarnFT.GetStyle() | WB_INFO );
    aWarnFI.SetImage( WarningBox::GetStandardImage() );

    aNoBtn.SetClickHdl( LINK( this, TransparencyPrintWarningBox, ClickNoBtn ) );
}

// -----------------------------------------------------------------------------

TransparencyPrintWarningBox::~TransparencyPrintWarningBox()
{
}

// -----------------------------------------------------------------------------

IMPL_LINK( TransparencyPrintWarningBox, ClickNoBtn, PushButton*, pButton )
{
    (void)pButton; //unused
    EndDialog( RET_NO );
    return 0;
}
