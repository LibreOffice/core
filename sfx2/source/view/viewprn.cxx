/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewprn.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:56:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <com/sun/star/document/XDocumentProperties.hpp>

#ifndef _COM_SUN_STAR_VIEW_PRINTABLESTATE_HPP_
#include <com/sun/star/view/PrintableState.hpp>
#endif

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_PRINTDLG_HXX //autogen
#include <svtools/printdlg.hxx>
#endif
#ifndef _SV_PRNSETUP_HXX //autogen
#include <svtools/prnsetup.hxx>
#endif
#ifndef _SFXFLAGITEM_HXX //autogen
#include <svtools/flagitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#include <svtools/useroptions.hxx>
#include <svtools/printwarningoptions.hxx>
#include <tools/datetime.hxx>

#include <sfx2/viewsh.hxx>
#include "viewimp.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/prnmon.hxx>
#include "sfxresid.hxx"
#include <sfx2/request.hxx>
#include <sfx2/objsh.hxx>
#include "sfxtypes.hxx"
#include <sfx2/event.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>

#include "view.hrc"
#include "helpid.hrc"

TYPEINIT1(SfxPrintingHint, SfxHint);

// -----------------------------------------------------------------------

void DisableRanges( PrintDialog& rDlg, SfxPrinter* pPrinter )

/*      [Beschreibung]

    Mit dieser Funktion werden die nicht verf"ugbaren Ranges
    vom Printer zum PrintDialog geforwarded.
*/

{
    if ( !pPrinter )
        return;

    if ( !pPrinter->IsRangeEnabled( PRINTDIALOG_ALL ) )
        rDlg.DisableRange( PRINTDIALOG_ALL );
    if ( !pPrinter->IsRangeEnabled( PRINTDIALOG_SELECTION ) )
        rDlg.DisableRange( PRINTDIALOG_SELECTION );
    if ( !pPrinter->IsRangeEnabled( PRINTDIALOG_FROMTO ) )
        rDlg.DisableRange( PRINTDIALOG_FROMTO );
    if ( !pPrinter->IsRangeEnabled( PRINTDIALOG_RANGE ) )
        rDlg.DisableRange( PRINTDIALOG_RANGE );
}

//====================================================================

class SfxDialogExecutor_Impl

/*  [Beschreibung]

    Eine Instanz dieser Klasse wird f"ur die Laufzeit des Printer-Dialogs
    erzeugt, um im dessen Click-Handler f"ur die Zus"atze den per
    virtueller Methode von der abgeleiteten SfxViewShell erzeugten
    Print-Options-Dialog zu erzeugen und die dort eingestellten Optionen
    als SfxItemSet zu zwischenzuspeichern.
*/

{
private:
    SfxViewShell*           _pViewSh;
    PrintDialog*            _pPrintParent;
    PrinterSetupDialog*     _pSetupParent;
    SfxItemSet*             _pOptions;
    sal_Bool                _bModified;
    sal_Bool                _bHelpDisabled;

    DECL_LINK( Execute, void * );

public:
            SfxDialogExecutor_Impl( SfxViewShell* pViewSh, PrintDialog* pParent );
            SfxDialogExecutor_Impl( SfxViewShell* pViewSh, PrinterSetupDialog* pParent );
            ~SfxDialogExecutor_Impl() { delete _pOptions; }

    Link                GetLink() const { return LINK( this, SfxDialogExecutor_Impl, Execute); }
    const SfxItemSet*   GetOptions() const { return _pOptions; }
    void                DisableHelp() { _bHelpDisabled = sal_True; }
};

//--------------------------------------------------------------------

SfxDialogExecutor_Impl::SfxDialogExecutor_Impl( SfxViewShell* pViewSh, PrintDialog* pParent ) :

    _pViewSh        ( pViewSh ),
    _pPrintParent   ( pParent ),
    _pSetupParent   ( NULL ),
    _pOptions       ( NULL ),
    _bModified      ( sal_False ),
    _bHelpDisabled  ( sal_False )

{
}

SfxDialogExecutor_Impl::SfxDialogExecutor_Impl( SfxViewShell* pViewSh, PrinterSetupDialog* pParent ) :

    _pViewSh        ( pViewSh ),
    _pPrintParent   ( NULL ),
    _pSetupParent   ( pParent ),
    _pOptions       ( NULL ),
    _bModified      ( sal_False ),
    _bHelpDisabled  ( sal_False )

{
}

//--------------------------------------------------------------------

IMPL_LINK( SfxDialogExecutor_Impl, Execute, void *, EMPTYARG )
{
    // Options lokal merken
    if ( !_pOptions )
    {
        DBG_ASSERT( _pPrintParent || _pSetupParent, "no dialog parent" );
        if( _pPrintParent )
            _pOptions = ( (SfxPrinter*)_pPrintParent->GetPrinter() )->GetOptions().Clone();
        else if( _pSetupParent )
            _pOptions = ( (SfxPrinter*)_pSetupParent->GetPrinter() )->GetOptions().Clone();
    }

    if ( _pOptions && _pPrintParent && _pPrintParent->IsSheetRangeAvailable() )
    {
        SfxItemState eState = _pOptions->GetItemState( SID_PRINT_SELECTEDSHEET );
        if ( eState != SFX_ITEM_UNKNOWN )
        {
            PrintSheetRange eRange = _pPrintParent->GetCheckedSheetRange();
            BOOL bValue = ( PRINTSHEETS_ALL != eRange );
            _pOptions->Put( SfxBoolItem( SID_PRINT_SELECTEDSHEET, bValue ) );
        }
    }

    // Dialog ausf"uhren
    SfxPrintOptionsDialog* pDlg = new SfxPrintOptionsDialog( _pPrintParent ? static_cast<Window*>(_pPrintParent)
                                                                           : static_cast<Window*>(_pSetupParent),
                                                             _pViewSh, _pOptions );
    if ( _bHelpDisabled )
        pDlg->DisableHelp();
    if ( pDlg->Execute() == RET_OK )
    {
        delete _pOptions;
        _pOptions = pDlg->GetOptions().Clone();

        if ( _pOptions && _pPrintParent && _pPrintParent->IsSheetRangeAvailable() )
        {
            const SfxPoolItem* pItem;
            if ( SFX_ITEM_SET == _pOptions->GetItemState( SID_PRINT_SELECTEDSHEET, FALSE , &pItem ) )
            {
                _pPrintParent->CheckSheetRange( ( (const SfxBoolItem*)pItem )->GetValue()
                    ? PRINTSHEETS_SELECTED_SHEETS : PRINTSHEETS_ALL );
            }
        }
    }
    delete pDlg;

    return 0;
}

//-------------------------------------------------------------------------

BOOL UseStandardPrinter_Impl( Window* /*pParent*/, SfxPrinter* pDocPrinter )
{
    // Optionen abfragen, ob gewarnt werden soll (Doc uebersteuert App)
    BOOL bWarn = FALSE;
    const SfxItemSet *pDocOptions = &pDocPrinter->GetOptions();
    if ( pDocOptions )
    {
        USHORT nWhich = pDocOptions->GetPool()->GetWhich(SID_PRINTER_NOTFOUND_WARN);
        const SfxBoolItem* pBoolItem = NULL;
        pDocPrinter->GetOptions().GetItemState( nWhich, FALSE, (const SfxPoolItem**) &pBoolItem );
        if ( pBoolItem )
            bWarn = pBoolItem->GetValue();
    }
/*
    // ggf. den User fragen
    if ( bWarn )
    {
        // Geht nicht mehr ohne OrigJobSetup!
        String aTmp( SfxResId( STR_PRINTER_NOTAVAIL ) );
        QueryBox aBox( pParent, WB_OK_CANCEL | WB_DEF_OK, aTmp );
        return RET_OK == aBox.Execute();
    }
*/
    // nicht gewarnt => einfach so den StandardDrucker nehmen
    return TRUE;
}
//-------------------------------------------------------------------------

SfxPrinter* SfxViewShell::SetPrinter_Impl( SfxPrinter *pNewPrinter )

/*  Interne Methode zum Setzen der Unterschiede von 'pNewPrinter' zum
    aktuellen Printer. pNewPrinter wird entweder "ubernommen oder gel"oscht.
*/

{
    // aktuellen Printer holen
    SfxPrinter *pDocPrinter = GetPrinter();

    // Printer-Options auswerten
    FASTBOOL bOriToDoc = FALSE;
    FASTBOOL bSizeToDoc = FALSE;
    if ( &pDocPrinter->GetOptions() )
    {
        USHORT nWhich = GetPool().GetWhich(SID_PRINTER_CHANGESTODOC);
        const SfxFlagItem *pFlagItem = 0;
        pDocPrinter->GetOptions().GetItemState( nWhich, FALSE, (const SfxPoolItem**) &pFlagItem );
        bOriToDoc = pFlagItem ? (pFlagItem->GetValue() & SFX_PRINTER_CHG_ORIENTATION) : FALSE;
        bSizeToDoc = pFlagItem ? (pFlagItem->GetValue() & SFX_PRINTER_CHG_SIZE) : FALSE;
    }

    // vorheriges Format und Size feststellen
    Orientation eOldOri = pDocPrinter->GetOrientation();
    Size aOldPgSz = pDocPrinter->GetPaperSizePixel();

    // neues Format und Size feststellen
    Orientation eNewOri = pNewPrinter->GetOrientation();
    Size aNewPgSz = pNewPrinter->GetPaperSizePixel();

    // "Anderungen am Seitenformat feststellen
    BOOL bOriChg = (eOldOri != eNewOri) && bOriToDoc;
    BOOL bPgSzChg = ( aOldPgSz.Height() !=
            ( bOriChg ? aNewPgSz.Width() : aNewPgSz.Height() ) ||
            aOldPgSz.Width() !=
            ( bOriChg ? aNewPgSz.Height() : aNewPgSz.Width() ) ) &&
            bSizeToDoc;

    // Message und Flags f"ur Seitenformat-"Anderung zusammenstellen
    String aMsg;
    USHORT nNewOpt=0;
    if( bOriChg && bPgSzChg )
    {
        aMsg = String(SfxResId(STR_PRINT_NEWORISIZE));
        nNewOpt = SFX_PRINTER_CHG_ORIENTATION | SFX_PRINTER_CHG_SIZE;
    }
    else if (bOriChg )
    {
        aMsg = String(SfxResId(STR_PRINT_NEWORI));
        nNewOpt = SFX_PRINTER_CHG_ORIENTATION;
    }
    else if (bPgSzChg)
    {
        aMsg = String(SfxResId(STR_PRINT_NEWSIZE));
        nNewOpt = SFX_PRINTER_CHG_SIZE;
    }

    // in dieser Variable sammeln, was sich so ge"aendert hat
    USHORT nChangedFlags = 0;

    // ggf. Nachfrage, ob Seitenformat vom Drucker "ubernommen werden soll
    if ( ( bOriChg  || bPgSzChg ) &&
        RET_YES == QueryBox(0, WB_YES_NO | WB_DEF_OK, aMsg).Execute() )
    // Flags mit "Anderungen f"ur <SetPrinter(SfxPrinter*)> mitpflegen
    nChangedFlags |= nNewOpt;

    // fuer den MAC sein "temporary of class String" im naechsten if()
    String aTempPrtName = pNewPrinter->GetName();
    String aDocPrtName = pDocPrinter->GetName();

    // Wurde der Drucker gewechselt oder von Default auf Specific
    // oder umgekehrt geaendert?
    if ( (aTempPrtName != aDocPrtName) || (pDocPrinter->IsDefPrinter() != pNewPrinter->IsDefPrinter()) )
    {
        // neuen Printer "ubernehmen
        // pNewPrinter->SetOrigJobSetup( pNewPrinter->GetJobSetup() );
        nChangedFlags |= SFX_PRINTER_PRINTER|SFX_PRINTER_JOBSETUP;
        pDocPrinter = pNewPrinter;
    }
    else
    {
        // Extra-Optionen vergleichen
        if ( ! (pNewPrinter->GetOptions() == pDocPrinter->GetOptions()) )
        {
            // Options haben sich geaendert
            pDocPrinter->SetOptions( pNewPrinter->GetOptions() );
            nChangedFlags |= SFX_PRINTER_OPTIONS;
        }

        // JobSetups vergleichen
        JobSetup aNewJobSetup = pNewPrinter->GetJobSetup();
        JobSetup aOldJobSetup = pDocPrinter->GetJobSetup();
        if ( aNewJobSetup != aOldJobSetup )
        {
            // JobSetup hat sich geaendert (=> App mu\s neu formatieren)
            // pDocPrinter->SetOrigJobSetup( aNewJobSetup );
            nChangedFlags |= SFX_PRINTER_JOBSETUP;
        }

        // alten, ver"anderten Printer behalten
        pDocPrinter->SetPrinterProps( pNewPrinter );
        delete pNewPrinter;
    }

    if ( 0 != nChangedFlags )
        // SetPrinter will delete the old printer if it changes
        SetPrinter( pDocPrinter, nChangedFlags );
    return pDocPrinter;
}

//-------------------------------------------------------------------------
// Unter WIN32 tritt leider das Problem auf, dass nichts gedruckt
// wird, wenn SID_PRINTDOCDIRECT auflaueft; bisher bekannte,
// einzige Abhilfe ist in diesem Fall das Abschalten der Optimierungen
// (KA 17.12.95)
#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

class SfxPrintGuard_Impl
{
    SfxObjectShell* m_pObjectShell;
    sal_Bool        m_bOrigStatus;
    sal_Bool        m_bNeedsChange;

public:
    SfxPrintGuard_Impl( SfxObjectShell* pObjectShell )
    : m_pObjectShell( pObjectShell )
    , m_bOrigStatus( sal_False )
    , m_bNeedsChange( sal_False )
    {
        if ( m_pObjectShell )
        {
            m_bOrigStatus = m_pObjectShell->IsEnableSetModified();

            // check configuration: shall update of printing information in DocInfo set the document to "modified"?
            if ( m_bOrigStatus && !SvtPrintWarningOptions().IsModifyDocumentOnPrintingAllowed() )
            {
                m_pObjectShell->EnableSetModified( sal_False );
                m_bNeedsChange = sal_True;
            }
        }
    }

    ~SfxPrintGuard_Impl()
    {
        if ( m_pObjectShell && m_bNeedsChange )
            m_pObjectShell->EnableSetModified( m_bOrigStatus );
    }
};

void SfxViewShell::ExecPrint_Impl( SfxRequest &rReq )
{
    USHORT                  nCopies=1;
    USHORT                  nDialogRet = RET_CANCEL;
    BOOL                    bCollate=FALSE;
    SfxPrinter*             pPrinter = 0;
    PrintDialog*            pPrintDlg = 0;
    SfxDialogExecutor_Impl* pExecutor = 0;
    bool                    bSilent = false;
    BOOL bIsAPI = rReq.GetArgs() && rReq.GetArgs()->Count();

    const USHORT nId = rReq.GetSlot();
    switch( nId )
    {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        case SID_PRINTDOC:
        case SID_SETUPPRINTER:
        case SID_PRINTER_NAME :
        {
            // quiet mode (AppEvent, API call)
            SFX_REQUEST_ARG(rReq, pSilentItem, SfxBoolItem, SID_SILENT, FALSE);
            bSilent = pSilentItem && pSilentItem->GetValue();

            // get printer and printer settings from the document
            SfxPrinter *pDocPrinter = GetPrinter(TRUE);

            // look for printer in parameters
            SFX_REQUEST_ARG( rReq, pPrinterItem, SfxStringItem, SID_PRINTER_NAME, FALSE );
            if ( pPrinterItem )
            {
                // use PrinterName parameter to create a printer
                pPrinter = new SfxPrinter( pDocPrinter->GetOptions().Clone(), ((const SfxStringItem*) pPrinterItem)->GetValue() );

                // if printer is unknown, it can't be used - now printer from document will be used
                if ( !pPrinter->IsOriginal() )
                    DELETEZ(pPrinter);
            }

            if ( SID_PRINTER_NAME == nId )
            {
                // just set a recorded printer name
                if ( pPrinter )
                    SetPrinter( pPrinter, SFX_PRINTER_PRINTER  );
                return;
            }

            // no PrinterName parameter in ItemSet or the PrinterName points to an unknown printer
            if ( !pPrinter )
                // use default printer from document
                pPrinter = pDocPrinter;

            if( !pPrinter || !pPrinter->IsValid() )
            {
                // no valid printer either in ItemSet or at the document
                if ( bSilent )
                {
                    rReq.SetReturnValue(SfxBoolItem(0,FALSE));
                    return;
                }
                else
                    ErrorBox( NULL, WB_OK | WB_DEF_OK, String( SfxResId( STR_NODEFPRINTER ) ) ).Execute();
            }

            if ( !pPrinter->IsOriginal() && rReq.GetArgs() && !UseStandardPrinter_Impl( NULL, pPrinter ) )
            {
                // printer is not available, but standard printer should not be used
                rReq.SetReturnValue(SfxBoolItem(0,FALSE));
                return;
            }

            if( pPrinter->IsPrinting() )
            {
                // if printer is busy, abort printing
                if ( !bSilent )
                    InfoBox( NULL, String( SfxResId( STR_ERROR_PRINTER_BUSY ) ) ).Execute();
                rReq.SetReturnValue(SfxBoolItem(0,FALSE));
                return;
            }

            // the print dialog shouldn't use a help button if it is called from the help window
            // (pressing help button would exchange the current page inside the help document that is going to be printed!)
            String aHelpFilterName( DEFINE_CONST_UNICODE("writer_web_HTML_help") );
            SfxMedium* pMedium = GetViewFrame()->GetObjectShell()->GetMedium();
            const SfxFilter* pFilter = pMedium ? pMedium->GetFilter() : NULL;
            sal_Bool bPrintOnHelp = ( pFilter && pFilter->GetFilterName() == aHelpFilterName );

            SfxObjectShell* pDoc = NULL;
            if ( SID_PRINTDOC == nId )
                pDoc = GetObjectShell();

            // Let the document stay nonmodified during the printing if the configuration says to do so
            SfxPrintGuard_Impl aGuard( pDoc );

            // if no arguments are given, retrieve them from a dialog
            if ( !bIsAPI )
            {
                // PrinterDialog needs a temporary printer
                SfxPrinter* pDlgPrinter = pPrinter->Clone();
                nDialogRet = 0;
                if ( SID_PRINTDOC == nId )
                {
                    bool bDetectHidden = ( !bSilent && !bPrintOnHelp && pDoc );
                    if ( !bDetectHidden
                        || pDoc->QueryHiddenInformation( WhenPrinting, NULL ) == RET_YES )
                    {
                        // execute PrintDialog
                        pPrintDlg = CreatePrintDialog( NULL );
                        if ( bPrintOnHelp )
                            pPrintDlg->DisableHelp();

                        if ( pImp->bHasPrintOptions )
                        {
                            // additional controls for dialog
                            pExecutor = new SfxDialogExecutor_Impl( this, pPrintDlg );
                            if ( bPrintOnHelp )
                                pExecutor->DisableHelp();
                            pPrintDlg->SetOptionsHdl( pExecutor->GetLink() );
                            pPrintDlg->ShowOptionsButton();
                        }

                        // set printer on dialog and execute
                        pPrintDlg->SetPrinter( pDlgPrinter );
                        ::DisableRanges( *pPrintDlg, pDlgPrinter );
                        nDialogRet = pPrintDlg->Execute();
                        if ( pExecutor && pExecutor->GetOptions() )
                        {
                            if ( nDialogRet == RET_OK )
                                // remark: have to be recorded if possible!
                                pDlgPrinter->SetOptions( *pExecutor->GetOptions() );
                            else
                            {
                                pPrinter->SetOptions( *pExecutor->GetOptions() );
                                SetPrinter( pPrinter, SFX_PRINTER_OPTIONS );
                            }
                        }

                        DELETEZ( pExecutor );
                    }
                }
                else
                {
                    // execute PrinterSetupDialog
                    PrinterSetupDialog* pPrintSetupDlg = new PrinterSetupDialog( GetWindow() );

                    if ( pImp->bHasPrintOptions )
                    {
                        // additional controls for dialog
                        pExecutor = new SfxDialogExecutor_Impl( this, pPrintSetupDlg );
                        if ( bPrintOnHelp )
                            pExecutor->DisableHelp();
                        pPrintSetupDlg->SetOptionsHdl( pExecutor->GetLink() );
                    }

                    pPrintSetupDlg->SetPrinter( pDlgPrinter );
                    nDialogRet = pPrintSetupDlg->Execute();

                    if ( pExecutor && pExecutor->GetOptions() )
                    {
                        if ( nDialogRet == RET_OK )
                            // remark: have to be recorded if possible!
                            pDlgPrinter->SetOptions( *pExecutor->GetOptions() );
                        else
                        {
                            pPrinter->SetOptions( *pExecutor->GetOptions() );
                            SetPrinter( pPrinter, SFX_PRINTER_OPTIONS );
                        }
                    }

                    DELETEZ( pPrintSetupDlg );

                    // no recording of PrinterSetup except printer name (is printer dependent)
                    rReq.Ignore();
                }

                if ( nDialogRet == RET_OK )
                {
                    if ( pPrinter->GetName() != pDlgPrinter->GetName() )
                    {
                        // user has changed the printer -> macro recording
                        SfxRequest aReq( GetViewFrame(), SID_PRINTER_NAME );
                        aReq.AppendItem( SfxStringItem( SID_PRINTER_NAME, pDlgPrinter->GetName() ) );
                        aReq.Done();
                    }

                    // take the changes made in the dialog
                    pPrinter = SetPrinter_Impl( pDlgPrinter );

                    // forget new printer, it was taken over (as pPrinter) or deleted
                    pDlgPrinter = NULL;

                    /* Now lets reset the Dialog printer, since its freed */
                    if (pPrintDlg)
                        pPrintDlg->SetPrinter (pPrinter);

                    if ( SID_PRINTDOC == nId )
                    {
                        nCopies  = pPrintDlg->GetCopyCount();
                        bCollate = pPrintDlg->IsCollateChecked();
                    }
                    else
                        break;
                }
                else
                {
                    // PrinterDialog is used to transfer information on printing,
                    // so it will only be deleted here if dialog was cancelled
                    DELETEZ( pDlgPrinter );
                    DELETEZ( pPrintDlg );
                    rReq.Ignore();
                    if ( SID_PRINTDOC == nId )
                        rReq.SetReturnValue(SfxBoolItem(0,FALSE));
                    break;
                }

                // recording
                rReq.AppendItem( SfxBoolItem( SID_PRINT_COLLATE, bCollate ) );
                rReq.AppendItem( SfxInt16Item( SID_PRINT_COPIES, (INT16) pPrintDlg->GetCopyCount() ) );
                if ( pPrinter->IsPrintFileEnabled() )
                    rReq.AppendItem( SfxStringItem( SID_FILE_NAME, pPrinter->GetPrintFile() ) );
                if ( pPrintDlg->IsRangeChecked(PRINTDIALOG_SELECTION) )
                    rReq.AppendItem( SfxBoolItem( SID_SELECTION, TRUE ) );
                else if ( pPrintDlg->IsRangeChecked(PRINTDIALOG_RANGE) )
                    rReq.AppendItem( SfxStringItem( SID_PRINT_PAGES, pPrintDlg->GetRangeText() ) );
                else if ( pPrintDlg->IsRangeChecked(PRINTDIALOG_FROMTO) )
                {
                    // currently this doesn't seem to work -> return values of dialog are always 0
                    // seems to be encoded as range string like "1-3"
                    rReq.AppendItem( SfxInt16Item( SID_PRINT_FIRST_PAGE, (INT16) pPrintDlg->GetFirstPage() ) );
                    rReq.AppendItem( SfxInt16Item( SID_PRINT_LAST_PAGE, (INT16) pPrintDlg->GetLastPage() ) );
                }
            }
            else if ( rReq.GetArgs() )
            {
                if ( SID_PRINTDOC != nId )
                {
                    DBG_ERROR("Wrong slotid!");
                    break;
                }

                // PrinterDialog is used to transfer information on printing
                pPrintDlg = CreatePrintDialog( GetWindow() );
                if ( bPrintOnHelp )
                    pPrintDlg->DisableHelp();
                pPrintDlg->SetPrinter( pPrinter );
                ::DisableRanges( *pPrintDlg, pPrinter );

                // PrintToFile requested?
                SFX_REQUEST_ARG(rReq, pFileItem, SfxStringItem, SID_FILE_NAME, FALSE);
                if ( pFileItem )
                {
                    pPrinter->EnablePrintFile(TRUE);
                    pPrinter->SetPrintFile( pFileItem->GetValue() );
                }

                // Collate
                SFX_REQUEST_ARG(rReq, pCollateItem, SfxBoolItem, SID_PRINT_COLLATE, FALSE);
                if ( pCollateItem )
                    bCollate = pCollateItem->GetValue();

                // Selection
                SFX_REQUEST_ARG(rReq, pSelectItem, SfxBoolItem, SID_SELECTION, FALSE);

                // Pages (as String)
                SFX_REQUEST_ARG(rReq, pPagesItem, SfxStringItem, SID_PRINT_PAGES, FALSE);

                // FirstPage
                SFX_REQUEST_ARG(rReq, pFirstPgItem, SfxInt16Item, SID_PRINT_FIRST_PAGE, FALSE);
                USHORT nFrom = 1;
                if ( pFirstPgItem )
                    nFrom = pFirstPgItem->GetValue();

                // LastPage
                SFX_REQUEST_ARG(rReq, pLastPgItem, SfxInt16Item, SID_PRINT_LAST_PAGE, FALSE);
                USHORT nTo = 9999;
                if ( pLastPgItem )
                    nTo = pLastPgItem->GetValue();

                // CopyCount
                SFX_REQUEST_ARG(rReq, pCopyItem, SfxInt16Item, SID_PRINT_COPIES, FALSE);
                if ( pCopyItem )
                {
                    nCopies = pCopyItem->GetValue();
                    pPrintDlg->SetCopyCount( nCopies );
                }

                // does the view support ranges?
                if ( pSelectItem && pSelectItem->GetValue() )
                {
                    // print selection only
                    pPrintDlg->CheckRange(PRINTDIALOG_SELECTION);
                }
                else if ( pPagesItem )
                {
                    // get range text from parameter
                    // enable ranges
                    pPrintDlg->CheckRange(PRINTDIALOG_RANGE);
                    pPrintDlg->SetRangeText( pPagesItem->GetValue() );
                }
                else if ( pPrintDlg->IsRangeEnabled(PRINTDIALOG_RANGE) )
                {
                    // enable ranges
                    // construct range text from page range
                    pPrintDlg->CheckRange(PRINTDIALOG_RANGE);
                    String aRange = String::CreateFromInt32( nFrom );
                    aRange += '-';
                    aRange += String::CreateFromInt32( nTo );
                    pPrintDlg->SetRangeText( aRange );
                }
                else
                {
                    // print page rage
                    pPrintDlg->CheckRange(PRINTDIALOG_FROMTO);
                    pPrintDlg->SetFirstPage( nFrom );
                    pPrintDlg->SetLastPage( nTo );
                }
            }

            // intentionally no break for SID_PRINTDOC
            // printing now proceeds like SID_PRINTDOCDIRECT
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        case SID_PRINTDOCDIRECT:
        {
            if ( SID_PRINTDOCDIRECT == nId )
            {
                SfxObjectShell* pDoc = GetObjectShell();
                bool bDetectHidden = ( !bSilent && pDoc );
                if ( bDetectHidden && pDoc->QueryHiddenInformation( WhenPrinting, NULL ) != RET_YES )
                    return;

                // if no printer was selected before
                if ( !pPrinter )
                    pPrinter = GetPrinter(TRUE);

                if( !pPrinter->IsValid() )
                {
                    // redirect slot to call the print dialog if the document's printer is not valid!
                    rReq.SetSlot( SID_PRINTDOC );
                    ExecPrint_Impl( rReq );
                    return;
                }

                if( pPrinter->IsOriginal() && pPrinter->GetName() != Printer::GetDefaultPrinterName() )
                {
                    // redirect slot to call the print dialog
                    // if the document's printer is available but not system default
                    rReq.SetSlot( SID_PRINTDOC );
                    ExecPrint_Impl( rReq );
                    return;
                }

                pPrinter->SetNextJobIsQuick();
            }

            // if "Collate" was checked, the SfxPrinter must handle the CopyCount itself,
            // usually this is handled by the printer driver
            if( bCollate )
                // set printer to default, handle multiple copies explicitly
                pPrinter->SetCopyCount( 1 );
            else
                pPrinter->SetCopyCount( nCopies );

            // enable background printing
            pPrinter->SetPageQueueSize( 1 );

            // refresh document info
            using namespace ::com::sun::star;
            SfxObjectShell *pObjSh = GetObjectShell();
            uno::Reference<document::XDocumentProperties> xDocProps(
                pObjSh->getDocProperties());
            ::rtl::OUString aLastPrintedBy = xDocProps->getPrintedBy();
            util::DateTime aLastPrinted = xDocProps->getPrintDate();

            // Let the document stay nonmodified during the printing if the configuration says to do so
            SfxPrintGuard_Impl aGuard( pObjSh );

            xDocProps->setPrintedBy( GetObjectShell()->IsUseUserData()
                ? ::rtl::OUString( SvtUserOptions().GetFullName() )
                : ::rtl::OUString() );
            ::DateTime now;
            xDocProps->setPrintDate( util::DateTime(
                now.Get100Sec(), now.GetSec(), now.GetMin(), now.GetHour(),
                now.GetDay(), now.GetMonth(), now.GetYear() ) );

            GetObjectShell()->Broadcast( SfxPrintingHint( -1, pPrintDlg, pPrinter ) );
            ErrCode nError = DoPrint( pPrinter, pPrintDlg, bSilent, bIsAPI );
            if ( nError == PRINTER_OK )
            {
                Invalidate( SID_PRINTDOC );
                Invalidate( SID_PRINTDOCDIRECT );
                Invalidate( SID_SETUPPRINTER );
                rReq.SetReturnValue(SfxBoolItem(0,TRUE));

                SFX_REQUEST_ARG(rReq, pAsyncItem, SfxBoolItem, SID_ASYNCHRON, FALSE);
                if ( pAsyncItem && !pAsyncItem->GetValue() )
                {
                    // synchronous execution wanted - wait for end of printing
                    while ( pPrinter->IsPrinting())
                        Application::Yield();
                }

                rReq.Done();
            }
            else
            {
                // printing not succesful, reset DocInfo
                xDocProps->setPrintedBy(aLastPrintedBy);
                xDocProps->setPrintDate(aLastPrinted);

                if ( nError != PRINTER_ABORT )
                {
                    // "real" problem (not simply printing cancelled by user)
                    String aMsg( SfxResId( STR_NOSTARTPRINTER ) );
                    if ( !bIsAPI )
                        ErrorBox( NULL, WB_OK | WB_DEF_OK,  aMsg ).Execute();
                    rReq.SetReturnValue(SfxBoolItem(0,FALSE));
                }

                rReq.Ignore();
            }

            delete pPrintDlg;
            break;
        }
    }
}

// Optimierungen wieder einschalten
#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif

//--------------------------------------------------------------------

PrintDialog* SfxViewShell::CreatePrintDialog( Window* pParent )

/*  [Beschreibung]

    Diese Methode kann "uberladen werden, um einen speziellen PrintDialog
    zu erzeugen. Dies ist z.B. notwendig wenn spezielle <StarView> Features
    wie drucken von Seitenbereichen.
*/

{
    PrintDialog *pDlg = new PrintDialog( pParent, false );
    pDlg->SetFirstPage( 1 );
    pDlg->SetLastPage( 9999 );
    pDlg->EnableCollate();
    return pDlg;
}

//--------------------------------------------------------------------

void SfxViewShell::PreparePrint( PrintDialog * )
{
}

//--------------------------------------------------------------------


ErrCode SfxViewShell::DoPrint( SfxPrinter *pPrinter,
                               PrintDialog *pPrintDlg,
                               BOOL bSilent, BOOL bIsAPI )
{
    // Printer-Dialogbox waehrend des Ausdrucks mu\s schon vor
    // StartJob erzeugt werden, da SV bei einem Quit-Event h"angt
    SfxPrintProgress *pProgress = new SfxPrintProgress( this, !bSilent );
    SfxPrinter *pDocPrinter = GetPrinter(TRUE);
    if ( !pPrinter )
        pPrinter = pDocPrinter;
    else if ( pDocPrinter != pPrinter )
    {
        pProgress->RestoreOnEndPrint( pDocPrinter->Clone() );
        SetPrinter( pPrinter, SFX_PRINTER_PRINTER );
    }
    pProgress->SetWaitMode(FALSE);

    // Drucker starten
    PreparePrint( pPrintDlg );
    SfxObjectShell *pObjShell = GetViewFrame()->GetObjectShell();
    if ( pPrinter->StartJob(pObjShell->GetTitle(0)) )
    {
        // Drucken
        Print( *pProgress, bIsAPI, pPrintDlg );
        pProgress->Stop();
        pProgress->DeleteOnEndPrint();
        pPrinter->EndJob();
    }
    else
    {
        // Printer konnte nicht gestartet werden
        delete pProgress;
    }

    return pPrinter->GetError();
}

//--------------------------------------------------------------------

BOOL SfxViewShell::IsPrinterLocked() const
{
    return pImp->nPrinterLocks > 0;
}

//--------------------------------------------------------------------

void SfxViewShell::LockPrinter( BOOL bLock)
{
    BOOL bChanged = FALSE;
    if ( bLock )
        bChanged = 1 == ++pImp->nPrinterLocks;
    else
        bChanged = 0 == --pImp->nPrinterLocks;

    if ( bChanged )
    {
        Invalidate( SID_PRINTDOC );
        Invalidate( SID_PRINTDOCDIRECT );
        Invalidate( SID_SETUPPRINTER );
    }
}

//--------------------------------------------------------------------

USHORT SfxViewShell::Print( SfxProgress& /*rProgress*/, BOOL /*bIsAPI*/, PrintDialog* /*pDlg*/ )
{
    return 0;
}

//--------------------------------------------------------------------

SfxPrinter* SfxViewShell::GetPrinter( BOOL /*bCreate*/ )
{
    return 0;
}

//--------------------------------------------------------------------

USHORT SfxViewShell::SetPrinter( SfxPrinter* /*pNewPrinter*/, USHORT /*nDiffFlags*/, bool )
{
    return 0;
}

//--------------------------------------------------------------------

SfxTabPage* SfxViewShell::CreatePrintOptionsPage
(
    Window*             /*pParent*/,
    const SfxItemSet&   /*rOptions*/
)

/*  [Beschreibung]

    Diese Factory-Methode wird vom SFx verwendet, um die TabPage mit den
    Print-Optionen, welche "uber das <SfxItemSet> am <SfxPrinter>
    transportiert werden, zu erzeugen.

    Abgeleitete Klassen k"onnen diese Methode also "uberladen um die zu
    ihren SfxPrinter passenden Einstellungen vorzunehmen. Dieses sollte
    genau die <SfxTabPage> sein, die auch unter Extras/Einstellungen
    verwendet wird.

    Die Basisimplementierung liefert einen 0-Pointer.
*/

{
    return 0;
}


