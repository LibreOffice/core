/*************************************************************************
 *
 *  $RCSfile: printdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pl $ $Date: 2000-09-27 12:55:32 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_APP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_PRINT_HXX
#include <vcl/print.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _VCL_FILEDLG_HXX
#include <vcl/filedlg.hxx>
#endif
#ifndef _SVTOOLS_FILEDLG_HXX
#include <filedlg.hxx>
#endif
#ifndef _SV_JOBSET_HXX
#include <vcl/jobset.hxx>
#endif
#include <tools/urlobj.hxx>
#pragma hdrstop

#include "printdlg.hrc"
#include <prnsetup.hxx>
#include <printdlg.hxx>
#include <svtdata.hxx>

// =======================================================================

PrintDialog::PrintDialog( Window* pWindow ) :
    ModalDialog     ( pWindow, SvtResId( DLG_SVT_PRNDLG_PRINTDLG ) ),
    maGrpPrinter    ( this, SvtResId( GRP_PRINTER ) ),
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
    maBtnBrowse     ( this, SvtResId( BTN_BROWSE ) ),
    maGrpPrintRange ( this, SvtResId( GRP_PRINTRANGE ) ),
    maRbtAll        ( this, SvtResId( RBT_ALL ) ),
    maRbtPages      ( this, SvtResId( RBT_PAGES ) ),
    maRbtSelection  ( this, SvtResId( RBT_SELECTION ) ),
    maEdtPages      ( this, SvtResId( EDT_PAGES ) ),
    maGrpCopies     ( this, SvtResId( GRP_COPIES ) ),
    maFtCopies      ( this, SvtResId( FT_COPIES ) ),
    maNumCopies     ( this, SvtResId( NUM_COPIES ) ),
    maImgCollate    ( this, SvtResId( IMG_COLLATE ) ),
    maImgNotCollate ( this, SvtResId( IMG_NOT_COLLATE ) ),
    maCbxCollate    ( this, SvtResId( CBX_COLLATE ) ),
    maBtnOptions    ( this, SvtResId( BTN_OPTIONS ) ),
    maBtnHelp       ( this, SvtResId( BTN_HELP ) ),
    maBtnOK         ( this, SvtResId( BTN_OK ) ),
    maBtnCancel     ( this, SvtResId( BTN_CANCEL ) ),
    maFiFaxNo       ( this, SvtResId( FI_FAXNO ) ),
    maEdtFaxNo      ( this, SvtResId( EDT_FAXNO ) )
{
    FreeResource();

    mpPrinter       = NULL;
    mpTempPrinter   = NULL;
    mnCopyCount     = 1;
    mnFirstPage     = 0;
    mnLastPage      = 0;
    mnMinPage       = 1;
    mnMaxPage       = 65535;
    meCheckRange    = PRINTDIALOG_ALL;
    mbAll           = TRUE;
    mbSelection     = FALSE;
    mbFromTo        = FALSE;
    mbRange         = FALSE;
    mbCollate       = FALSE;
    mbCollateCheck  = FALSE;
    mbOptions       = FALSE;

    maStatusTimer.SetTimeout( IMPL_PRINTDLG_STATUS_UPDATE );
    maStatusTimer.SetTimeoutHdl( LINK( this, PrintDialog, ImplStatusHdl ) );
    maBtnProperties.SetClickHdl( LINK( this, PrintDialog, ImplPropertiesHdl ) );
    maLbName.SetSelectHdl( LINK( this, PrintDialog, ImplChangePrinterHdl ) );
    maBtnBrowse.SetClickHdl( LINK( this, PrintDialog, ImplBrowseHdl ) );

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

    maRbtAll.Check();

    // Because we have no local print Dialog, or in the other case
    // the print client and print server must handle this with a
    // temporaery file and copy the file after printing to the
    // destionation file name.
    if ( Application::IsRemoteServer() )
        maCbxFilePrint.Enable( FALSE );
}

// -----------------------------------------------------------------------

PrintDialog::~PrintDialog()
{
    ImplFreePrnDlgListBox( &maLbName, FALSE );
    delete mpTempPrinter;
}

// -----------------------------------------------------------------------

void PrintDialog::ImplSetInfo()
{
    const QueueInfo* pInfo = (QueueInfo*)(maLbName.GetEntryData( maLbName.GetSelectEntryPos() ));
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
        maFiPrintFile.Show( FALSE );
        maCbxFilePrint.Show( FALSE );
        maBtnBrowse.Show( FALSE );
        maFiFaxNo.Show( TRUE );
        maEdtFaxNo.Show( TRUE );
        Printer* pPrinter = mpTempPrinter ? mpTempPrinter : mpPrinter;
        maEdtFaxNo.SetText( pPrinter->GetJobValue( String::CreateFromAscii( "FAX#" ) ) );
    }
    else
#endif
    {
        maFiPrintFile.Show( TRUE );
        maCbxFilePrint.Show( TRUE );
        maBtnBrowse.Show( TRUE );
        maFiFaxNo.Show( FALSE );
        maEdtFaxNo.Show( FALSE );
    }

}

// -----------------------------------------------------------------------

void PrintDialog::ImplCheckOK()
{
    // Ueberprueft, ob der OK-Button enabled ist
    BOOL bEnable = TRUE;

    if ( maCbxFilePrint.IsChecked() )
        bEnable = maFiPrintFile.GetText().Len() > 0;

    if ( bEnable && maRbtPages.IsChecked() )
        bEnable = maEdtPages.GetText().Len() > 0;

    if ( bEnable )
    {
        if ( mpTempPrinter )
            bEnable = mpTempPrinter->IsValid();
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
            maRbtAll.Check( TRUE );
    }
    else
        maRbtAll.Enable( FALSE );

    // Selektion
    if ( mbSelection )
    {
        maRbtSelection.Enable();
        if ( meCheckRange == PRINTDIALOG_SELECTION )
            maRbtSelection.Check( TRUE );
    }
    else
        maRbtSelection.Enable( FALSE );

    // Seiten
    if ( mbRange )
    {
        maRbtPages.Enable();
        maEdtPages.Show();
        maEdtPages.SetText( maRangeText );

        if( ( meCheckRange == PRINTDIALOG_FROMTO ) ||
            ( meCheckRange == PRINTDIALOG_RANGE ) )
        {
            maRbtPages.Check( TRUE );
            maEdtPages.Enable();
        }
        else
            maEdtPages.Enable( FALSE );
    }
    else
    {
        maRbtPages.Enable( FALSE );
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

    mnCopyCount = (USHORT) maNumCopies.GetValue();
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
    if ( !mpTempPrinter )
        mpTempPrinter = new Printer( mpPrinter->GetJobSetup() );
    mpTempPrinter->Setup();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( PrintDialog, ImplChangePrinterHdl, void*, EMPTYARG )
{
    mpTempPrinter = ImplPrnDlgListBoxSelect( &maLbName, &maBtnProperties,
                                             mpPrinter, mpTempPrinter );
    ImplSetInfo();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( PrintDialog, ImplBrowseHdl, void*, EMPTYARG )
{
    VclFileDialog* pVclFileDlg = GetpApp()->CreateFileDialog( this, WB_SAVEAS );
    if ( pVclFileDlg )
    {
        short nRet = pVclFileDlg->Execute();

        if ( nRet == RET_OK )
        {
            INetURLObject aObj( pVclFileDlg->GetPath() );
            maFiPrintFile.SetText( aObj.PathToFileName() );
            ImplCheckOK();
        }

        delete pVclFileDlg;
    }
    else
    {
        FileDialog* pFileDlg = new FileDialog( this, WB_SAVEAS );
        short nRet = pFileDlg->Execute();

        if ( nRet == RET_OK )
        {
            maFiPrintFile.SetText( pFileDlg->GetPath() );
            ImplCheckOK();
        }

        delete pFileDlg;
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( PrintDialog, ImplModifyControlHdl, void*, p )
{
    // Drucken in Datei
    if ( !p || (p == &maCbxFilePrint) )
    {
        BOOL bCheck = maCbxFilePrint.IsChecked();
        if ( bCheck && !maFiPrintFile.GetText().Len() )
            ImplBrowseHdl( &maBtnBrowse );
        maFiPrintFile.Enable( bCheck );
        maBtnBrowse.Enable( bCheck );
        ImplCheckOK();
    }

    // Radiobuttons (Umfang)
    if ( !p || (p == &maRbtAll) || (p == &maRbtPages) || (p == &maRbtSelection) )
    {
        BOOL bCheck = maRbtPages.IsChecked();
        maEdtPages.Enable( bCheck );
        if ( p == &maRbtPages )
            maEdtPages.GrabFocus();
        ImplCheckOK();
    }

    // Edit-Felder (Dateiname, Seiten)
    if ( p == &maEdtPages )
        ImplCheckOK();

    if( p == &maEdtFaxNo )
    {
        Printer* pPrinter = mpTempPrinter ? mpTempPrinter : mpPrinter;
        pPrinter->SetJobValue( String::CreateFromAscii( "FAX#" ), maEdtFaxNo.GetText() );
    }

    // Anzahl Kopien
    BOOL bNumCopies = FALSE;

    if ( !p || p == &maNumCopies )
    {
        if ( p )
            bNumCopies = TRUE;
        BOOL bCopies = maNumCopies.GetValue() > 1;
        maCbxCollate.Enable( bCopies && mbCollate );

        if ( !bCopies )
            maCbxCollate.Check( FALSE );
        else if ( mbCollateCheck )
            maCbxCollate.Check( TRUE );
    }

    // Sortieren
    if ( !p || p == &maCbxCollate || bNumCopies )
    {
        BOOL bCheck = maCbxCollate.IsChecked();

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

    return 0;
}

// -----------------------------------------------------------------------

long PrintDialog::ClickOptionsHdl()
{
    if ( maOptionsHdlLink.IsSet() )
        return maOptionsHdlLink.Call( this );
    else
        return TRUE;
}

// -----------------------------------------------------------------------

long PrintDialog::OK()
{
    if ( maOKHdlLink.IsSet() )
        return maOKHdlLink.Call( this );
    else
        return TRUE;
}

// -----------------------------------------------------------------------

long PrintDialog::Notify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_GETFOCUS) && IsReallyVisible() )
        ImplStatusHdl( &maStatusTimer );

    return ModalDialog::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void PrintDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( rDCEvt.GetType() == DATACHANGED_PRINTER )
    {
        mpTempPrinter = ImplPrnDlgUpdatePrinter( mpPrinter, mpTempPrinter );
        Printer* pPrn;
        if ( mpTempPrinter )
            pPrn = mpTempPrinter;
        else
            pPrn = mpPrinter;
        ImplFillPrnDlgListBox( pPrn, &maLbName, &maBtnProperties );
        ImplSetInfo();
        ImplCheckOK();
    }

    ModalDialog::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

short PrintDialog::Execute()
{
    if ( !mpPrinter || mpPrinter->IsPrinting() || mpPrinter->IsJobActive() )
    {
        DBG_ERRORFILE( "PrinterSetupDialog::Execute() - No Printer or printer is printing" );
        return FALSE;
    }

    // Controls initialisieren
    ImplFillPrnDlgListBox( mpPrinter, &maLbName, &maBtnProperties );
    ImplSetInfo();
    maStatusTimer.Start();
    ImplInitControls();
    ImplModifyControlHdl( NULL );

    // Dialog starten
    short nRet = ModalDialog::Execute();

    // Wenn Dialog mit OK beendet wurde, dann die Daten updaten
    if( nRet == TRUE )
    {
        if ( mpTempPrinter )
            mpPrinter->SetPrinterProps( mpTempPrinter );
        ImplFillDialogData();
    }

    maStatusTimer.Stop();

    return nRet;
}


