/*************************************************************************
 *
 *  $RCSfile: printopt.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 11:28:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PRINTWARNINGOPTIONS_HXX
#include <svtools/printwarningoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PRINTOPTIONS_HXX
#include <svtools/printoptions.hxx>
#endif
#ifndef _SFXFLAGITEM_HXX
#include <svtools/flagitem.hxx>
#endif


#include "printopt.hrc"
#include "dialog.hrc"
#include "sfxresid.hxx"
#include "viewsh.hxx"
#include "printopt.hxx"

// -----------
// - statics -
// -----------

static USHORT   aDPIArray[] = { 72, 96, 150, 200, 300, 600 };
static BOOL     bOutputForPrinter = TRUE;

#define DPI_COUNT (sizeof(aDPIArray)/sizeof(aDPIArray[0 ]))

// --------------------------------
// - SfxCommonPrintOptionsTabPage -
// --------------------------------

SfxCommonPrintOptionsTabPage::SfxCommonPrintOptionsTabPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage( pParent, SfxResId( TP_COMMONPRINTOPTIONS ), rSet ),

    aReduceGB( this, ResId( GB_REDUCE ) ),
    aOutputTypeFT( this, ResId( FT_OUTPUTTYPE ) ),
    aPrinterOutputRB( this, ResId( RB_PRINTEROUTPUT ) ),
    aPrintFileOutputRB( this, ResId( RB_PRINTFILEOUTPUT ) ),
    aOutputGB( this, ResId( GB_OUTPUT ) ),
    aReduceTransparencyCB( this, ResId( CB_REDUCETRANSPARENCY ) ),
    aReduceTransparencyAutoRB( this, ResId( RB_REDUCETRANSPARENCY_AUTO ) ),
    aReduceTransparencyNoneRB( this, ResId( RB_REDUCETRANSPARENCY_NONE ) ),
    aReduceGradientsCB( this, ResId( CB_REDUCEGRADIENTS ) ),
    aReduceGradientsStripesRB( this, ResId( RB_REDUCEGRADIENTS_STRIPES ) ),
    aReduceGradientsColorRB( this, ResId( RB_REDUCEGRADIENTS_COLOR ) ),
    aReduceGradientsStepCountNF( this, ResId( NF_REDUCEGRADIENTS_STEPCOUNT ) ),
    aReduceBitmapsCB( this, ResId( CB_REDUCEBITMAPS ) ),
    aReduceBitmapsOptimalRB( this, ResId( RB_REDUCEBITMAPS_OPTIMAL ) ),
    aReduceBitmapsNormalRB( this, ResId( RB_REDUCEBITMAPS_NORMAL ) ),
    aReduceBitmapsResolutionRB( this, ResId( RB_REDUCEBITMAPS_RESOLUTION ) ),
    aReduceBitmapsTransparencyCB( this, ResId( CB_REDUCEBITMAPS_TRANSPARENCY ) ),
    aReduceBitmapsResolutionLB( this, ResId( LB_REDUCEBITMAPS_RESOLUTION ) ),
    aConvertToGreyscalesCB( this, ResId( CB_CONVERTTOGREYSCALES ) ),
    aWarnGB( this, ResId( GB_PRINT_WARN ) ),
    aPaperSizeCB( this, ResId( CB_PAPERSIZE ) ),
    aPaperOrientationCB( this, ResId( CB_PAPERORIENTATION ) ),
    aTransparencyCB( this, ResId( CB_TRANSPARENCY ) )
{
    FreeResource();

    aOutputGB.SetStyle( aOutputGB.GetStyle() | WB_NOLABEL );

    if( bOutputForPrinter )
    {
        aPrinterOutputRB.Check( TRUE );
        aOutputGB.SetText( OutputDevice::GetNonMnemonicString( aPrinterOutputRB.GetText() ) );
    }
    else
    {
        aPrintFileOutputRB.Check( TRUE );
        aOutputGB.SetText( OutputDevice::GetNonMnemonicString( aPrintFileOutputRB.GetText() ) );
    }

    aPrinterOutputRB.SetToggleHdl( LINK( this, SfxCommonPrintOptionsTabPage, ToggleOutputPrinterRBHdl ) );
    aPrintFileOutputRB.SetToggleHdl( LINK( this, SfxCommonPrintOptionsTabPage, ToggleOutputPrintFileRBHdl ) );

    aReduceTransparencyCB.SetClickHdl( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceTransparencyCBHdl ) );
    aReduceGradientsCB.SetClickHdl( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceGradientsCBHdl ) );
    aReduceBitmapsCB.SetClickHdl( LINK( this, SfxCommonPrintOptionsTabPage, ClickReduceBitmapsCBHdl ) );

    aReduceGradientsStripesRB.SetToggleHdl( LINK( this, SfxCommonPrintOptionsTabPage, ToggleReduceGradientsStripesRBHdl ) );
    aReduceBitmapsResolutionRB.SetToggleHdl( LINK( this, SfxCommonPrintOptionsTabPage, ToggleReduceBitmapsResolutionRBHdl ) );
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

BOOL SfxCommonPrintOptionsTabPage::FillItemSet( SfxItemSet& rSet )
{
    SvtPrintWarningOptions  aWarnOptions;
    SvtPrinterOptions       aPrinterOptions;
    SvtPrintFileOptions     aPrintFileOptions;
    BOOL                    bModified = FALSE;


    if( aPaperSizeCB.IsChecked() != aPaperSizeCB.GetSavedValue() ||
        aPaperOrientationCB.IsChecked() != aPaperOrientationCB.GetSavedValue() )
    {
        USHORT nFlag = aPaperSizeCB.IsChecked() ? SFX_PRINTER_CHG_SIZE : 0;

        nFlag |= aPaperOrientationCB.IsChecked() ? SFX_PRINTER_CHG_ORIENTATION : 0;
        bModified |= ( 0 != rSet.Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, nFlag ) ) );
       }

    if( aTransparencyCB.IsChecked() != aTransparencyCB.GetSavedValue() )
        aWarnOptions.SetTransparency( aTransparencyCB.IsChecked() );

    ImplSaveControls( aPrinterOutputRB.IsChecked() ? &maPrinterOptions : &maPrintFileOptions );

    aPrinterOptions.SetPrinterOptions( maPrinterOptions );
    aPrintFileOptions.SetPrinterOptions( maPrintFileOptions );

    return bModified;
}

// -----------------------------------------------------------------------------

void SfxCommonPrintOptionsTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem*      pItem;
    SvtPrintWarningOptions  aWarnOptions;
    SvtPrinterOptions       aPrinterOptions;
    SvtPrintFileOptions     aPrintFileOptions;

    if( SFX_ITEM_SET == rSet.GetItemState( SID_PRINTER_CHANGESTODOC, FALSE, &pItem ) )
    {
        USHORT nFlag = ( (const SfxFlagItem*)pItem )->GetValue();

        aPaperSizeCB.Check( 0 != ( nFlag & SFX_PRINTER_CHG_SIZE ) );
        aPaperOrientationCB.Check( 0 != ( nFlag & SFX_PRINTER_CHG_ORIENTATION ) );
    }
    else
    {
        aPaperSizeCB.Check( aWarnOptions.IsPaperSize() );
        aPaperOrientationCB.Check( aWarnOptions.IsPaperOrientation() );
    }

    aTransparencyCB.Check( aWarnOptions.IsTransparency() );

    aPaperSizeCB.SaveValue();
    aPaperOrientationCB.SaveValue();
    aTransparencyCB.SaveValue();

    aPrinterOptions.GetPrinterOptions( maPrinterOptions );
    aPrintFileOptions.GetPrinterOptions( maPrintFileOptions );

    ImplUpdateControls( aPrinterOutputRB.IsChecked() ? &maPrinterOptions : &maPrintFileOptions );
}

// -----------------------------------------------------------------------------

int SfxCommonPrintOptionsTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if( pSet )
        FillItemSet( *pSet );

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------------

void SfxCommonPrintOptionsTabPage::ImplUpdateControls( const PrinterOptions* pCurrentOptions )
{
    aReduceTransparencyCB.Check( pCurrentOptions->IsReduceTransparency() );

    if( pCurrentOptions->GetReducedTransparencyMode() == PRINTER_TRANSPARENCY_AUTO )
        aReduceTransparencyAutoRB.Check( TRUE );
    else
        aReduceTransparencyNoneRB.Check( TRUE );

    aReduceGradientsCB.Check( pCurrentOptions->IsReduceGradients() );

    if( pCurrentOptions->GetReducedGradientMode() == PRINTER_GRADIENT_STRIPES )
        aReduceGradientsStripesRB.Check( TRUE );
    else
        aReduceGradientsColorRB.Check( TRUE );

    aReduceGradientsStepCountNF.SetValue( pCurrentOptions->GetReducedGradientStepCount() );

    aReduceBitmapsCB.Check( pCurrentOptions->IsReduceBitmaps() );

    if( pCurrentOptions->GetReducedBitmapMode() == PRINTER_BITMAP_OPTIMAL )
        aReduceBitmapsOptimalRB.Check( TRUE );
    else if( pCurrentOptions->GetReducedBitmapMode() == PRINTER_BITMAP_NORMAL )
        aReduceBitmapsNormalRB.Check( TRUE );
    else
        aReduceBitmapsResolutionRB.Check( TRUE );

    const USHORT nDPI = pCurrentOptions->GetReducedBitmapResolution();

    if( nDPI < aDPIArray[ 0 ] )
        aReduceBitmapsResolutionLB.SelectEntryPos( 0 );
    else
    {
        for( long i = ( DPI_COUNT - 1 ); i >= 0; i-- )
        {
            if( nDPI >= aDPIArray[ i ] )
            {
                aReduceBitmapsResolutionLB.SelectEntryPos( (USHORT) i );
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

void SfxCommonPrintOptionsTabPage::ImplSaveControls( PrinterOptions* pCurrentOptions )
{
    pCurrentOptions->SetReduceTransparency( aReduceTransparencyCB.IsChecked() );
    pCurrentOptions->SetReducedTransparencyMode( aReduceTransparencyAutoRB.IsChecked() ? PRINTER_TRANSPARENCY_AUTO : PRINTER_TRANSPARENCY_NONE );
    pCurrentOptions->SetReduceGradients( aReduceGradientsCB.IsChecked() );
    pCurrentOptions->SetReducedGradientMode( aReduceGradientsStripesRB.IsChecked() ? PRINTER_GRADIENT_STRIPES : PRINTER_GRADIENT_COLOR  );
    pCurrentOptions->SetReducedGradientStepCount( (USHORT) aReduceGradientsStepCountNF.GetValue() );
    pCurrentOptions->SetReduceBitmaps( aReduceBitmapsCB.IsChecked() );
    pCurrentOptions->SetReducedBitmapMode( aReduceBitmapsOptimalRB.IsChecked() ? PRINTER_BITMAP_OPTIMAL :
                                           ( aReduceBitmapsNormalRB.IsChecked() ? PRINTER_BITMAP_NORMAL : PRINTER_BITMAP_RESOLUTION ) );
    pCurrentOptions->SetReducedBitmapResolution( aDPIArray[ Min( (USHORT) aReduceBitmapsResolutionLB.GetSelectEntryPos(),
                                                            (USHORT)( sizeof( aDPIArray ) / sizeof( aDPIArray[ 0 ] ) - 1 ) ) ] );
    pCurrentOptions->SetReducedBitmapIncludesTransparency( aReduceBitmapsTransparencyCB.IsChecked() );
    pCurrentOptions->SetConvertToGreyscales( aConvertToGreyscalesCB.IsChecked() );
}

// -----------------------------------------------------------------------------

IMPL_LINK( SfxCommonPrintOptionsTabPage, ClickReduceTransparencyCBHdl, CheckBox*, pBox )
{
    const BOOL bReduceTransparency = aReduceTransparencyCB.IsChecked();

    aReduceTransparencyAutoRB.Enable( bReduceTransparency );
    aReduceTransparencyNoneRB.Enable( bReduceTransparency );

    aTransparencyCB.Enable( !bReduceTransparency );

    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SfxCommonPrintOptionsTabPage, ClickReduceGradientsCBHdl, CheckBox*, pBox )
{
    const BOOL bEnable = aReduceGradientsCB.IsChecked();

    aReduceGradientsStripesRB.Enable( bEnable );
    aReduceGradientsColorRB.Enable( bEnable );
    aReduceGradientsStepCountNF.Enable( bEnable );

    ToggleReduceGradientsStripesRBHdl( &aReduceGradientsStripesRB );

    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SfxCommonPrintOptionsTabPage, ClickReduceBitmapsCBHdl, CheckBox*, pBox )
{
    const BOOL bEnable = aReduceBitmapsCB.IsChecked();

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
    const BOOL bEnable = aReduceGradientsCB.IsChecked() && aReduceGradientsStripesRB.IsChecked();

    aReduceGradientsStepCountNF.Enable( bEnable );

    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SfxCommonPrintOptionsTabPage, ToggleReduceBitmapsResolutionRBHdl, RadioButton*, pButton )
{
    const BOOL bEnable = aReduceBitmapsCB.IsChecked() && aReduceBitmapsResolutionRB.IsChecked();

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
        bOutputForPrinter = TRUE;
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
        bOutputForPrinter = FALSE;
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
    aWarnFI     ( this, ResId( FI_PRINTTRANSWARN ) ),
    aWarnFT     ( this, ResId( FT_PRINTTRANSWARN ) ),
    aYesBtn     ( this, ResId( BTN_PRINTTRANS_YES ) ),
    aNoBtn      ( this, ResId( BTN_PRINTTRANS_NO ) ),
    aCancelBtn  ( this, ResId( BTN_PRINTTRANS_CANCEL ) ),
    aNoWarnCB   ( this, ResId( CBX_NOPRINTTRANSWARN ) )
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
    EndDialog( RET_NO );
    return 0;
}
