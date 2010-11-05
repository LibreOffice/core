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
#include "precompiled_svtools.hxx"
#include <tools/debug.hxx>
#ifndef _SV_APP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_PRINT_HXX
#include <vcl/print.hxx>
#endif
#include <vcl/msgbox.hxx>
#include <vcl/jobset.hxx>
#include <tools/urlobj.hxx>

#include "printdlg.hrc"
#include "controldims.hrc"
#include <svtools/prnsetup.hxx>
#include <svtools/printdlg.hxx>
#include <svtools/svtdata.hxx>
#include <filedlg.hxx>
#include "svl/pickerhelper.hxx"
#ifndef _SVT_HELPID_HRC
#include <svtools/helpid.hrc>
#endif
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

using rtl::OUString;
using namespace com::sun::star;

struct SvtPrinterImpl
{
    Printer*        m_pTempPrinter;
    sal_Bool        m_bHelpDisabled;
    PrintSheetRange m_eSheetRange;

    SvtPrinterImpl() :
        m_pTempPrinter( NULL ), m_bHelpDisabled( sal_False ), m_eSheetRange( PRINTSHEETS_ALL ) {}
    ~SvtPrinterImpl() { delete m_pTempPrinter; }
};

#define TEMPPRINTER()   mpPrinterImpl->m_pTempPrinter

// =======================================================================

PrintDialog::PrintDialog( Window* pWindow, bool bWithSheetsAndCells ) :
    ModalDialog     ( pWindow, SvtResId( DLG_SVT_PRNDLG_PRINTDLG ) ),
    maFlPrinter     ( this, SvtResId( FL_PRINTER ) ),
    maFtName        ( this, SvtResId( FT_NAME ) ),
    maLbName        ( this, SvtResId( LB_NAMES ) ),
    maBtnProperties ( this, SvtResId( BTN_PROPERTIES ) ),
    maFtStatus      ( this, SvtResId( FT_STATUS ) ),
    maFiStatus      ( this, SvtResId( FI_STATUS ) ),
    maFtType        ( this, SvtResId( FT_TYPE ) ),
    maFiType        ( this, SvtResId( FI_TYPE ) ),
    maFtLocation    ( this, SvtResId( FT_LOCATION ) ),
    maFiLocation    ( this, SvtResId( FI_LOCATION ) ),
    maFtComment     ( this, SvtResId( FT_COMMENT ) ),
    maFiComment     ( this, SvtResId( FI_COMMENT ) ),
    maCbxFilePrint  ( this, SvtResId( CBX_FILEPRINT ) ),
    maFiPrintFile   ( this, SvtResId( FI_PRINTFILE ) ),
    maFiFaxNo       ( this, SvtResId( FI_FAXNO ) ),
    maEdtFaxNo      ( this, SvtResId( EDT_FAXNO ) ),
    maFlPrint       ( this, SvtResId( FL_PRINT ) ),
    maRbtAllSheets  ( this, SvtResId( RBT_ALL_SHEETS ) ),
    maRbtSelectedSheets ( this, SvtResId( RBT_SELECTED_SHEETS ) ),
    maRbtSelectedCells  ( this, SvtResId( RBT_SELECTED_CELLS ) ),
    maFlPrintRange      ( this, SvtResId( FL_PRINTRANGE ) ),
    maRbtAll            ( this, SvtResId( RBT_ALL ) ),
    maRbtPages          ( this, SvtResId( RBT_PAGES ) ),
    maRbtSelection      ( this, SvtResId( RBT_SELECTION ) ),
    maEdtPages          ( this, SvtResId( EDT_PAGES ) ),
    maFlSepCopiesRange  ( this, SvtResId( FL_SEPCOPIESRANGE ) ),
    maFlCopies          ( this, SvtResId( FL_COPIES ) ),
    maFtCopies          ( this, SvtResId( FT_COPIES ) ),
    maNumCopies         ( this, SvtResId( NUM_COPIES ) ),
    maImgCollate        ( this, SvtResId( IMG_COLLATE ) ),
    maImgNotCollate     ( this, SvtResId( IMG_NOT_COLLATE ) ),
    maCbxCollate        ( this, SvtResId( CBX_COLLATE ) ),
    maFlSepButtonLine   ( this, SvtResId( FL_SEPBUTTONLINE ) ),
    maBtnOptions        ( this, SvtResId( BTN_OPTIONS ) ),
    maBtnOK             ( this, SvtResId( BTN_OK ) ),
    maBtnCancel         ( this, SvtResId( BTN_CANCEL ) ),
    maBtnHelp           ( this, SvtResId( BTN_HELP ) ),
    mbWithSheetsAndCells( bWithSheetsAndCells ),
    maAllFilterStr      (       SvtResId( STR_ALLFILTER ) )

{
    FreeResource();

    mpPrinter       = NULL;
    mpPrinterImpl   = new SvtPrinterImpl;
    mnCopyCount     = 1;
    mnFirstPage     = 0;
    mnLastPage      = 0;
    mnMinPage       = 1;
    mnMaxPage       = 65535;
    meCheckRange    = PRINTDIALOG_ALL;
    mbAll           = sal_True;
    mbSelection     = sal_False;
    mbFromTo        = sal_False;
    mbRange         = sal_False;
    mbCollate       = sal_True;
    mbCollateCheck  = sal_True;
    mbOptions       = sal_False;

    maStatusTimer.SetTimeout( IMPL_PRINTDLG_STATUS_UPDATE );
    maStatusTimer.SetTimeoutHdl( LINK( this, PrintDialog, ImplStatusHdl ) );
    maBtnProperties.SetClickHdl( LINK( this, PrintDialog, ImplPropertiesHdl ) );
    maLbName.SetSelectHdl( LINK( this, PrintDialog, ImplChangePrinterHdl ) );

    maFiPrintFile.SetStyle( maFiPrintFile.GetStyle() | WB_PATHELLIPSIS );

    Link aLink( LINK( this, PrintDialog, ImplModifyControlHdl ) );
    maCbxFilePrint.SetClickHdl( aLink );
    maRbtAll.SetClickHdl( aLink );
    maRbtPages.SetClickHdl( aLink );
    maRbtSelection.SetClickHdl( aLink );
    maEdtPages.SetModifyHdl( aLink );
    maNumCopies.SetModifyHdl( aLink );
    maCbxCollate.SetClickHdl( aLink );
    maBtnOptions.SetClickHdl( aLink );
    maEdtFaxNo.SetModifyHdl( aLink );
    maBtnOK.SetClickHdl( aLink );

    maRbtAll.Check();
    ImplSetImages();
}

// -----------------------------------------------------------------------

PrintDialog::~PrintDialog()
{
    ImplFreePrnDlgListBox( &maLbName, sal_False );
    delete mpPrinterImpl;
}

// -----------------------------------------------------------------------

void PrintDialog::ImplSetImages()
{
    if( ! GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        maImgCollate.SetModeImage( Image( SvtResId( RID_IMG_PRNDLG_COLLATE ) ), BMP_COLOR_NORMAL );
        maImgNotCollate.SetModeImage( Image( SvtResId( RID_IMG_PRNDLG_NOCOLLATE ) ), BMP_COLOR_NORMAL );
    }
    else
    {
        maImgCollate.SetModeImage( Image( SvtResId( RID_IMG_PRNDLG_COLLATE_HC ) ), BMP_COLOR_HIGHCONTRAST );
        maImgNotCollate.SetModeImage( Image( SvtResId( RID_IMG_PRNDLG_NOCOLLATE_HC ) ), BMP_COLOR_HIGHCONTRAST );
    }
}

// -----------------------------------------------------------------------

void PrintDialog::ImplSetInfo()
{
    const QueueInfo* pInfo = Printer::GetQueueInfo( maLbName.GetSelectEntry(), true );
    if ( pInfo )
    {
        maFiType.SetText( pInfo->GetDriver() );
        maFiLocation.SetText( pInfo->GetLocation() );
        maFiComment.SetText( pInfo->GetComment() );
        maFiStatus.SetText( ImplPrnDlgGetStatusText( *pInfo ) );
    }
    else
    {
        XubString aTempStr;
        maFiType.SetText( aTempStr );
        maFiLocation.SetText( aTempStr );
        maFiComment.SetText( aTempStr );
        maFiStatus.SetText( aTempStr );
    }

#ifdef UNX
    if( pInfo && pInfo->GetLocation().EqualsAscii( "fax_queue" ) )
    {
        maFiPrintFile.Show( sal_False );
        maCbxFilePrint.Show( sal_False );
        maFiFaxNo.Show( sal_True );
        maEdtFaxNo.Show( sal_True );
        Printer* pPrinter = TEMPPRINTER() ? TEMPPRINTER() : mpPrinter;
        maEdtFaxNo.SetText( pPrinter->GetJobValue( String::CreateFromAscii( "FAX#" ) ) );

        Size aFTSize = maFiFaxNo.GetSizePixel();
        long nTextWidth = maFiFaxNo.GetCtrlTextWidth( maFiFaxNo.GetText() ) + 10;
        if ( aFTSize.Width() < nTextWidth )
        {
            long nDelta = nTextWidth - aFTSize.Width();
            aFTSize.Width() = aFTSize.Width() + nDelta;
            maFiFaxNo.SetSizePixel( aFTSize );
            Size aEdtSize = maEdtFaxNo.GetSizePixel();
            aEdtSize.Width() = aEdtSize.Width() - nDelta;
            Point aEdtPos = maEdtFaxNo.GetPosPixel();
            aEdtPos.X() = aEdtPos.X() + nDelta;
            maEdtFaxNo.SetPosSizePixel( aEdtPos, aEdtSize );
        }
    }
    else
#endif
    {
        maFiPrintFile.Show( sal_True );
        maCbxFilePrint.Show( sal_True );
        maFiFaxNo.Show( sal_False );
        maEdtFaxNo.Show( sal_False );
    }

}

// -----------------------------------------------------------------------

void PrintDialog::ImplCheckOK()
{
    // Ueberprueft, ob der OK-Button enabled ist
    sal_Bool bEnable = sal_True;

    if ( bEnable && maRbtPages.IsChecked() )
        bEnable = maEdtPages.GetText().Len() > 0;

    if ( bEnable )
    {
        if ( TEMPPRINTER() )
            bEnable = TEMPPRINTER()->IsValid();
        else
            bEnable = mpPrinter->IsValid();
    }

    maBtnOK.Enable( bEnable );
}

// -----------------------------------------------------------------------

void PrintDialog::ImplInitControls()
{
    // Alles
    if ( mbAll )
    {
        maRbtAll.Enable();
        if( meCheckRange == PRINTDIALOG_ALL )
            maRbtAll.Check( sal_True );
    }
    else
        maRbtAll.Enable( sal_False );

    // Selektion
    if ( mbSelection )
    {
        maRbtSelection.Enable();
        if ( meCheckRange == PRINTDIALOG_SELECTION )
            maRbtSelection.Check( sal_True );
    }
    else
        maRbtSelection.Enable( sal_False );

    // Seiten
    if ( mbRange )
    {
        maRbtPages.Enable();
        maEdtPages.Show();
        maEdtPages.SetText( maRangeText );

        if( ( meCheckRange == PRINTDIALOG_FROMTO ) ||
            ( meCheckRange == PRINTDIALOG_RANGE ) )
        {
            maRbtPages.Check( sal_True );
            maEdtPages.Enable();
        }
        else
            maEdtPages.Enable( sal_False );
    }
    else
    {
        maRbtPages.Enable( sal_False );
        maEdtPages.Hide();
    }

    // Anzahl Kopien
    maNumCopies.SetValue( mnCopyCount );

    // Sortierung
    maCbxCollate.Enable( mbCollate );
    maCbxCollate.Check( mbCollateCheck );

    // Zusaetze-Button
    if ( mbOptions )
        maBtnOptions.Show();

    if ( !mbWithSheetsAndCells )
    {
        Size aMarginSize =
            LogicToPixel( Size( RSC_SP_CTRL_GROUP_X, RSC_SP_CTRL_GROUP_Y ), MAP_APPFONT );
        long nTempPos = maImgCollate.GetPosPixel().Y() +
            maImgCollate.GetSizePixel().Height() +  aMarginSize.Height();
        long nDelta1 = maFlPrintRange.GetPosPixel().Y() - maFlPrint.GetPosPixel().Y();
        long nDelta2 = maFlSepButtonLine.GetPosPixel().Y() - nTempPos;

        maFlPrint.Hide();
        maRbtAllSheets.Hide();
        maRbtSelectedSheets.Hide();
        maRbtSelectedCells.Hide();
        maRbtSelection.Show();

        Size aNewSize = GetSizePixel();
        aNewSize.Height() -= nDelta2;
        SetSizePixel( aNewSize );
        aNewSize = maFlSepCopiesRange.GetSizePixel();
        aNewSize.Height() -= nDelta2;
        maFlSepCopiesRange.SetSizePixel( aNewSize );

        long nDelta = nDelta1;
        Window* pControls[] = { &maFlPrintRange, &maRbtAll,
                                &maRbtPages, &maEdtPages, &maRbtSelection, NULL,
                                &maFlSepButtonLine, &maBtnOptions, &maBtnOK,
                                &maBtnCancel, &maBtnHelp };
        Window** pCtrl = pControls;
        const sal_Int32 nCount = sizeof( pControls ) / sizeof( pControls[0] );
        for ( sal_Int32 i = 0; i < nCount; ++i, ++pCtrl )
        {
            if ( NULL == *pCtrl )
            {
                nDelta = nDelta2;
                continue;
            }
            Point aNewPos = (*pCtrl)->GetPosPixel();
            aNewPos.Y() -= nDelta;
            (*pCtrl)->SetPosPixel( aNewPos );
        }
    }
}

// -----------------------------------------------------------------------

void PrintDialog::ImplFillDialogData()
{
    if ( maRbtAll.IsChecked() )
        meCheckRange = PRINTDIALOG_ALL;
    else if( maRbtSelection.IsChecked() )
        meCheckRange = PRINTDIALOG_SELECTION;
    else
    {
        meCheckRange = PRINTDIALOG_RANGE;
        maRangeText = maEdtPages.GetText();
    }

    mnCopyCount = (sal_uInt16) maNumCopies.GetValue();
    mbCollateCheck = maCbxCollate.IsChecked();

    // In Datei drucken
    if ( maCbxFilePrint.IsChecked() )
        mpPrinter->SetPrintFile( maFiPrintFile.GetText() );
    mpPrinter->EnablePrintFile( maCbxFilePrint.IsChecked() );
}

// -----------------------------------------------------------------------

IMPL_LINK( PrintDialog, ImplStatusHdl, Timer*, EMPTYARG )
{
    QueueInfo aInfo;
    ImplPrnDlgUpdateQueueInfo( &maLbName, aInfo );
    maFiStatus.SetText( ImplPrnDlgGetStatusText( aInfo ) );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( PrintDialog, ImplPropertiesHdl, void*, EMPTYARG )
{
    if ( !TEMPPRINTER() )
        TEMPPRINTER() = new Printer( mpPrinter->GetJobSetup() );
    TEMPPRINTER()->Setup( this );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( PrintDialog, ImplChangePrinterHdl, void*, EMPTYARG )
{
    TEMPPRINTER() = ImplPrnDlgListBoxSelect( &maLbName, &maBtnProperties,
                                             mpPrinter, TEMPPRINTER() );
    ImplSetInfo();
    ImplCheckOK(); // Check if "OK" button can be enabled now!

    return 0;
}

// -----------------------------------------------------------------------

bool PrintDialog::ImplGetFilename()
{
    uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    static ::rtl::OUString aOldFile;
    if( xFactory.is() )
    {
        uno::Sequence< uno::Any > aTempl( 1 );
        aTempl.getArray()[0] <<= ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION;
        uno::Reference< ui::dialogs::XFilePicker > xFilePicker(
            xFactory->createInstanceWithArguments(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FilePicker" ) ),
                aTempl ), uno::UNO_QUERY );
        DBG_ASSERT( xFilePicker.is(), "could not get FilePicker service" );

        uno::Reference< ui::dialogs::XFilterManager > xFilterMgr( xFilePicker, uno::UNO_QUERY );
        if( xFilePicker.is() && xFilterMgr.is() )
        {
            try
            {
#ifdef UNX
                // add PostScript and PDF
                Printer* pPrinter = TEMPPRINTER() ? TEMPPRINTER() : mpPrinter;
                bool bPS = true, bPDF = true;
                if( pPrinter )
                {
                    if( pPrinter->GetCapabilities( PRINTER_CAPABILITIES_PDF ) )
                        bPS = false;
                    else
                        bPDF = false;
                }
                if( bPS )
                    xFilterMgr->appendFilter( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PostScript" ) ), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*.ps" ) ) );
                if( bPDF )
                    xFilterMgr->appendFilter( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Portable Document Format" ) ), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*.pdf" ) ) );
#elif defined WNT
                xFilterMgr->appendFilter( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*.PRN" ) ), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*.prn" ) ) );
#endif
                // add arbitrary files
                xFilterMgr->appendFilter( maAllFilterStr, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*.*" ) ) );
            }
            catch( lang::IllegalArgumentException rExc )
            {
                DBG_ERRORFILE( "caught IllegalArgumentException when registering filter\n" );
            }

            if( aOldFile.getLength() )
            {
                INetURLObject aUrl( aOldFile, INET_PROT_FILE );
                xFilePicker->setDefaultName( aUrl.GetLastName() );
                aUrl.CutLastName();
                xFilePicker->setDisplayDirectory( aUrl.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
            }

            if( xFilePicker->execute() == ui::dialogs::ExecutableDialogResults::OK )
            {
                uno::Sequence< ::rtl::OUString > aPathSeq( xFilePicker->getFiles() );
                INetURLObject aObj( aPathSeq[0] );
                maFiPrintFile.SetText( aOldFile = aObj.PathToFileName() );
                return true;
            }
            return false;
        }
    }

    // something went awry, lets try the old fashioned dialogue
    Window* pDlgParent = IsReallyVisible() ? this : GetParent();
    FileDialog aDlg( pDlgParent, WB_STDDIALOG | WB_SAVEAS );
#ifdef WNT
    aDlg.AddFilter( String( RTL_CONSTASCII_USTRINGPARAM( "*.prn" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "*.prn" ) ) );
    aDlg.SetDefaultExt( String( RTL_CONSTASCII_USTRINGPARAM( "prn" ) ) );
#elif defined UNX
    aDlg.AddFilter( String( RTL_CONSTASCII_USTRINGPARAM( "PostScript" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "*.ps" ) ) );
    aDlg.SetDefaultExt( String( RTL_CONSTASCII_USTRINGPARAM( "ps" ) ) );
#endif

    if( aOldFile.getLength() )
        aDlg.SetPath( aOldFile );

    if( aDlg.Execute() )
    {
        String aTargetFile = aDlg.GetPath();
        maFiPrintFile.SetText( aOldFile = aTargetFile );
        return true;
    }

    return false;
}

// -----------------------------------------------------------------------

IMPL_LINK( PrintDialog, ImplModifyControlHdl, void*, p )
{
    // Radiobuttons (Umfang)
    if ( !p || (p == &maRbtAll) || (p == &maRbtPages) || (p == &maRbtSelection) )
    {
        sal_Bool bCheck = maRbtPages.IsChecked();
        maEdtPages.Enable( bCheck );
        if ( p == &maRbtPages )
            maEdtPages.GrabFocus();
        ImplCheckOK();
    }

    // Edit-Felder (Seiten)
    if ( p == &maEdtPages )
        ImplCheckOK();

    if( p == &maEdtFaxNo )
    {
        Printer* pPrinter = TEMPPRINTER() ? TEMPPRINTER() : mpPrinter;
        pPrinter->SetJobValue( String::CreateFromAscii( "FAX#" ), maEdtFaxNo.GetText() );
    }

    // Anzahl Kopien
    sal_Bool bNumCopies = sal_False;

    if ( !p || p == &maNumCopies )
    {
        if ( p )
            bNumCopies = sal_True;
        //sal_Bool bCopies = maNumCopies.GetValue() > 1;
        maCbxCollate.Enable( mbCollate );

        /*if ( !bCopies )
            maCbxCollate.Check( sal_False );
        else*/
        if ( mbCollateCheck )
            maCbxCollate.Check( sal_True );
    }

    // Sortieren
    if ( !p || p == &maCbxCollate || bNumCopies )
    {
        sal_Bool bCheck = maCbxCollate.IsChecked();

        if ( !bNumCopies )
            mbCollateCheck = maCbxCollate.IsChecked();

        if( bCheck )
        {
            maImgCollate.Show();
            maImgNotCollate.Hide();
        }
        else
        {
            maImgCollate.Hide();
            maImgNotCollate.Show();
        }
    }

    // Zus"atze
    if ( p == &maBtnOptions )
        ClickOptionsHdl();

    if( p == &maBtnOK )
    {
        EndDialog( maCbxFilePrint.IsChecked() ? ImplGetFilename() : sal_True );
    }

    return 0;
}

// -----------------------------------------------------------------------

long PrintDialog::ClickOptionsHdl()
{
    if ( maOptionsHdlLink.IsSet() )
        return maOptionsHdlLink.Call( this );
    else
        return sal_True;
}

// -----------------------------------------------------------------------

long PrintDialog::OK()
{
    if ( maOKHdlLink.IsSet() )
        return maOKHdlLink.Call( this );
    else
        return sal_True;
}

// -----------------------------------------------------------------------

void PrintDialog::EnableSheetRange( bool bEnable, PrintSheetRange eRange )
{
    if ( mbWithSheetsAndCells )
    {
        switch ( eRange )
        {
            case PRINTSHEETS_ALL :
                maRbtAllSheets.Enable( bEnable != false );
                break;
            case PRINTSHEETS_SELECTED_SHEETS :
                maRbtSelectedSheets.Enable( bEnable != false );
                break;
            case PRINTSHEETS_SELECTED_CELLS :
                maRbtSelectedCells.Enable( bEnable != false );
                break;
            default:
                DBG_ERRORFILE( "PrintDialog::EnableSheetRange(): invalid range" );
        }
    }
}

// -----------------------------------------------------------------------

bool PrintDialog::IsSheetRangeEnabled( PrintSheetRange eRange ) const
{
    if ( !mbWithSheetsAndCells )
        return false;

    bool bRet = false;
    switch ( eRange )
    {
        case PRINTSHEETS_ALL :
            bRet = maRbtAllSheets.IsEnabled() != sal_False;
            break;
        case PRINTSHEETS_SELECTED_SHEETS :
            bRet = maRbtSelectedSheets.IsEnabled() != sal_False;
            break;
        case PRINTSHEETS_SELECTED_CELLS :
            bRet = maRbtSelectedCells.IsEnabled() != sal_False;
            break;
        default:
            DBG_ERRORFILE( "PrintDialog::IsSheetRangeEnabled(): invalid range" );
    }
    return bRet;
}

// -----------------------------------------------------------------------

void PrintDialog::CheckSheetRange( PrintSheetRange eRange )
{
    if ( mbWithSheetsAndCells )
    {
        switch ( eRange )
        {
            case PRINTSHEETS_ALL :
                maRbtAllSheets.Check();
                break;
            case PRINTSHEETS_SELECTED_SHEETS :
                maRbtSelectedSheets.Check();
                break;
            case PRINTSHEETS_SELECTED_CELLS :
                maRbtSelectedCells.Check();
                break;
            default:
                DBG_ERRORFILE( "PrintDialog::CheckSheetRange(): invalid range" );
        }
    }
}

// -----------------------------------------------------------------------

PrintSheetRange PrintDialog::GetCheckedSheetRange() const
{
    PrintSheetRange eRange = PRINTSHEETS_ALL;
    if ( mbWithSheetsAndCells )
    {
        if ( maRbtSelectedSheets.IsChecked() )
            eRange = PRINTSHEETS_SELECTED_SHEETS;
        else if ( maRbtSelectedCells.IsChecked() )
            eRange = PRINTSHEETS_SELECTED_CELLS;
    }
    return eRange;
}

// -----------------------------------------------------------------------

bool PrintDialog::IsSheetRangeChecked( PrintSheetRange eRange ) const
{
    if ( !mbWithSheetsAndCells )
        return false;

    bool bRet = false;
    switch ( eRange )
    {
        case PRINTSHEETS_ALL :
            bRet = maRbtAllSheets.IsChecked() != sal_False;
            break;
        case PRINTSHEETS_SELECTED_SHEETS :
            bRet = maRbtSelectedSheets.IsChecked() != sal_False;
            break;
        case PRINTSHEETS_SELECTED_CELLS :
            bRet = maRbtSelectedCells.IsChecked() != sal_False;
            break;
        default:
            DBG_ERRORFILE( "PrintDialog::IsSheetRangeChecked(): invalid range" );
    }
    return bRet;
}

// -----------------------------------------------------------------------

long PrintDialog::Notify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_GETFOCUS) && IsReallyVisible() )
        ImplStatusHdl( &maStatusTimer );
    else if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if ( rNEvt.GetKeyEvent()->GetKeyCode().GetCode() == KEY_F1 && mpPrinterImpl->m_bHelpDisabled )
            return 1; // do nothing
    }

    return ModalDialog::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void PrintDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DATACHANGED_PRINTER )
    {
        TEMPPRINTER() = ImplPrnDlgUpdatePrinter( mpPrinter, TEMPPRINTER() );
        Printer* pPrn;
        if ( TEMPPRINTER() )
            pPrn = TEMPPRINTER();
        else
            pPrn = mpPrinter;
        ImplFillPrnDlgListBox( pPrn, &maLbName, &maBtnProperties );
        ImplSetInfo();
        ImplCheckOK();
    }
    else if ( rDCEvt.GetType() == DATACHANGED_SETTINGS )
        ImplSetImages();

    ModalDialog::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

short PrintDialog::Execute()
{
    if ( !mpPrinter || mpPrinter->IsPrinting() || mpPrinter->IsJobActive() )
    {
        DBG_ERRORFILE( "PrinterSetupDialog::Execute() - No Printer or printer is printing" );
        return sal_False;
    }

    // check if the printer brings up its own dialog
    // in that case leave the work to that dialog
    if( mpPrinter->GetCapabilities( PRINTER_CAPABILITIES_EXTERNALDIALOG ) )
        return sal_True;

    Printer::updatePrinters();

    // Controls initialisieren
    ImplFillPrnDlgListBox( mpPrinter, &maLbName, &maBtnProperties );
    ImplSetInfo();
    maStatusTimer.Start();
    ImplInitControls();
    maNumCopies.GrabFocus();
    maNumCopies.SetSelection( Selection( 0, maNumCopies.GetText().Len() ) );
    ImplModifyControlHdl( NULL );

    // Dialog starten
    short nRet = ModalDialog::Execute();

    // Wenn Dialog mit OK beendet wurde, dann die Daten updaten
    if( nRet == sal_True )
    {
        if ( TEMPPRINTER() )
            mpPrinter->SetPrinterProps( TEMPPRINTER() );
        ImplFillDialogData();
    }

    maStatusTimer.Stop();

    return nRet;
}

// -----------------------------------------------------------------------

void PrintDialog::DisableHelp()
{
    mpPrinterImpl->m_bHelpDisabled = sal_True;
    maBtnHelp.Disable();
}

