/*************************************************************************
 *
 *  $RCSfile: viewprn.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:37 $
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

#ifdef OS2
#include <vcl/sysdep.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef VCL
#ifndef _PRNDLG_HXX //autogen
#include <vcl/prndlg.hxx>
#endif
#else
#ifndef _SV_PRINTDLG_HXX //autogen
#include <svtools/printdlg.hxx>
#endif
#ifndef _SV_PRNSETUP_HXX //autogen
#include <svtools/prnsetup.hxx>
#endif
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
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
#pragma hdrstop

#include "viewsh.hxx"
#include "viewimp.hxx"
#include "viewfrm.hxx"
#include "prnmon.hxx"
#include "sfxresid.hxx"
#include "request.hxx"
#include "objsh.hxx"
#include "inimgr.hxx"
#include "sfxtypes.hxx"
#include "docinf.hxx"
#include "event.hxx"

#include "view.hrc"
#include "helpid.hrc"

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
    SfxViewShell*           _pViewSh;
    PrintDialog*            _pParent;
    SfxItemSet*             _pOptions;
    BOOL                    _bModified;

private:
    DECL_LINK( Execute, void * );

public:
            SfxDialogExecutor_Impl( SfxViewShell* pViewSh,
                        PrintDialog* pParent );
            ~SfxDialogExecutor_Impl()
            { delete _pOptions; }

    Link                GetLink() const
            { return LINK( this, SfxDialogExecutor_Impl, Execute); }

    const SfxItemSet*   GetOptions() const
            { return _pOptions; }
};

//--------------------------------------------------------------------

SfxDialogExecutor_Impl::SfxDialogExecutor_Impl( SfxViewShell *pViewSh,
                        PrintDialog* pParent )
:   _pViewSh( pViewSh ),
    _pParent( pParent ),
    _pOptions( 0 )
{
}

//--------------------------------------------------------------------

IMPL_LINK( SfxDialogExecutor_Impl, Execute, void *, pVoidCaller )
{
    // Options lokal merken
    if ( !_pOptions )
    _pOptions = ((SfxPrinter*)_pParent->GetPrinter())->GetOptions().Clone();

    // Dialog ausf"uhren
    SfxPrintOptionsDialog *pDlg =
        new SfxPrintOptionsDialog( _pParent, _pViewSh, _pOptions );
    if ( pDlg->Execute() == RET_OK )
    {
        delete _pOptions;
    _pOptions = pDlg->GetOptions().Clone();
    }
    delete pDlg;
    return 0;
}

//-------------------------------------------------------------------------

BOOL UseStandardPrinter_Impl( Window *pParent, SfxPrinter *pDocPrinter )
{
    // Optionen abfragen, ob gewarnt werden soll (Doc uebersteuert App)
    BOOL bWarn = FALSE;
    SfxApplication *pSfxApp = SFX_APP();
/*
    // Das Iten gibt es gar nicht !!
    SfxItemSet aOpt( pSfxApp->GetPool(),
                     SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN );
    if ( pSfxApp->GetOptions( aOpt ) )
    {
        SFX_ITEMSET_GET( aOpt, pWarnItem, SfxBoolItem, SID_PRINTER_NOTFOUND_WARN, TRUE );
        if ( pWarnItem )
            bWarn = pWarnItem->GetValue();
    }
 */
    const SfxItemSet *pDocOptions = &pDocPrinter->GetOptions();
    if ( pDocOptions )
    {
        USHORT nWhich =
                pDocOptions->GetPool()->GetWhich(SID_PRINTER_NOTFOUND_WARN);
    const SfxBoolItem *pBoolItem = 0;
    pDocPrinter->GetOptions().GetItemState( nWhich, FALSE,
        (const SfxPoolItem**) &pBoolItem );
        if ( pBoolItem )
        bWarn = pBoolItem->GetValue();
    }

/*
    // ggf. den User fragen
    if ( bWarn )
    {
        // Geht nicht mehr ohne OrigJobSetup!
        String aTmp( SfxResId( STR_PRINTER_NOTAVAIL ) );
#if SUPD<532
        aTmp.SearchAndReplace( "$1",
                pDocPrinter->GetOrigJobSetup().GetPrinterName() );
        aTmp.SearchAndReplace( "$2", pDocPrinter->GetName() );
#endif
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
    pDocPrinter->GetOptions().GetItemState( nWhich, FALSE,
        (const SfxPoolItem**) &pFlagItem );
    bOriToDoc = pFlagItem
        ? (pFlagItem->GetValue() & SFX_PRINTER_CHG_ORIENTATION)
        : FALSE;
    bSizeToDoc = pFlagItem
        ? (pFlagItem->GetValue() & SFX_PRINTER_CHG_SIZE)
        : FALSE;
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
    if ( (aTempPrtName != aDocPrtName) ||
        (pDocPrinter->IsDefPrinter() != pNewPrinter->IsDefPrinter()) )
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
    SetPrinter( pDocPrinter, nChangedFlags );
    return pDocPrinter;
}

//-------------------------------------------------------------------------
// Unter WIN32 tritt leider das Problem auf, dass nichts gedruckt
// wird, wenn SID_PRINTDOCDIRECT auflaueft; bisher bekannte,
// einzige Abhilfe ist in diesem Fall das Abschalten der Optimierungen
// (KA 17.12.95)
#ifdef WNT
#pragma optimize ( "", off )
#endif

void SfxViewShell::ExecPrint_Impl( SfxRequest &rReq )
{
    USHORT                  nCopies=1, nPaperBin = 0, nDialogRet = RET_CANCEL;
    BOOL                    bCollate=FALSE;
    SfxPrinter*             pPrinter = 0;
    PrintDialog*            pPrintDlg = 0;
    PrinterSetupDialog*     pPrintSetupDlg = 0;
    SfxDialogExecutor_Impl* pExecutor = 0;
    PushButton*             pOptBtn = 0;
    FASTBOOL                bWarn = TRUE;
    //! FASTBOOL            bUseThread = FALSE;
    FASTBOOL                bSilent = FALSE;

    const USHORT nId = rReq.GetSlot();
    switch( nId )
    {
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    case SID_PRINTDOC:
    case SID_SETUPPRINTER:
    case SID_PRINTER_NAME:
    {
        if(!GetViewFrame()->GetObjectShell()->ISA(SfxObjectShell))
            return;

        // stiller Modus (z.B. per AppEvent)?
        SFX_REQUEST_ARG(rReq, pSilentItem, SfxBoolItem, SID_SILENT, FALSE);
        bSilent = pSilentItem ? pSilentItem->GetValue() : FALSE;

        // Drucker und Einstellungen vom Dokument holen
        SfxPrinter *pDocPrinter = GetPrinter(TRUE);

        // Printername per Parameter?
        SFX_REQUEST_ARG(rReq, pPrinterItem, SfxStringItem, SID_PRINTER_NAME, FALSE);
        if ( pPrinterItem )
        {
            // zum Namen passenden Printer erzeugen
            pPrinter = new SfxPrinter( pDocPrinter->GetOptions().Clone(),
                ((const SfxStringItem*) pPrinterItem)->GetValue() );

            // ein unknown Printer wuerde sowieso gleich nochmal umgesetzt
            if ( !pPrinter->IsOriginal() )
                DELETEZ(pPrinter);

            // Printer wurde per API umgesetzt?
            if ( SID_PRINTER_NAME == nId )
            {
                if ( pPrinter )
                    SetPrinter( pPrinter, SFX_PRINTER_PRINTER  );
                return;
            }
        }

        if ( !pPrinter )
            pPrinter = pDocPrinter;

        // kein Printer oder ung"ultig
        if( !pPrinter->IsValid() )
        {
            if ( !rReq.IsAPI() )
                ErrorBox( NULL, WB_OK | WB_DEF_OK,
                          String( SfxResId( STR_NODEFPRINTER ) ) ).Execute();
            rReq.SetReturnValue(SfxBoolItem(0,FALSE));
            return;
        }

#if 0
        // au\ser unter Windows ggf. im Thread drucken lassen
        nWhich = GetPool().GetWhich(SID_PRINTER_USETHREAD);
        pPrinter->GetOptions().GetItemState( nWhich, FALSE,
                 (const SfxPoolItem**) &pBoolItem );
        bUseThread = pBoolItem ? pBoolItem->GetValue() : TRUE;
#endif

        // Drucker nicht vorhanden?
        if ( !pPrinter->IsOriginal() && bWarn && rReq.GetArgs() &&
             !UseStandardPrinter_Impl( NULL, pPrinter ) )
        {
            rReq.SetReturnValue(SfxBoolItem(0,FALSE));
            return;
        }

        // busy?
        if( pPrinter->IsPrinting() )
        {
            if ( !rReq.IsAPI() )
                InfoBox( NULL, String( SfxResId( STR_ERROR_PRINTER_BUSY ) ) ).Execute();
            rReq.SetReturnValue(SfxBoolItem(0,FALSE));
            return;
        }

        // brauchen wir den Dialog?
        if ( !rReq.GetArgs() && !bSilent && !rReq.IsAPI() )
        {
            // Printer-Dialog braucht tempor"aren Printer
            SfxPrinter* pDlgPrinter = pPrinter->Clone();

            // Print bwz. Print-Options Dialog ausf"urhren
            nDialogRet = 0;
            if ( SID_PRINTDOC == nId )
            {
                // Print-Dialog
                pPrintDlg = CreatePrintDialog( NULL );

                // Zusaetze Button und Dialog?
                if ( pImp->bHasPrintOptions )
                {
                    pExecutor = new SfxDialogExecutor_Impl( this, pPrintDlg );
                    pPrintDlg->SetOptionsHdl( pExecutor->GetLink() );
                    pPrintDlg->ShowOptionsButton();
                }

                // Drucker setzen
                pPrintDlg->SetPrinter( pDlgPrinter );
                        ::DisableRanges( *pPrintDlg, pDlgPrinter );

                // Dialog ausf"uhren
                nDialogRet = pPrintDlg->Execute();
                if ( pExecutor && pExecutor->GetOptions() )
                {
                    if ( nDialogRet == RET_OK )
                        pDlgPrinter->SetOptions( *pExecutor->GetOptions() );
                    else
                    {
                        pPrinter->SetOptions( *pExecutor->GetOptions() );
                        SetPrinter( pPrinter, SFX_PRINTER_OPTIONS );
                    }
                }
                DELETEX(pExecutor);
            }
            else
            {
                // Printer-Setup-Dialog
                pPrintSetupDlg = new PrinterSetupDialog( NULL );
                pPrintSetupDlg->SetPrinter( pDlgPrinter );
                nDialogRet = pPrintSetupDlg->Execute();
            }

            if ( nDialogRet == RET_OK )
            {
                // "Anderungen feststellen und Drucker setzen
                pPrinter = SetPrinter_Impl( pDlgPrinter );

                if ( SID_PRINTDOC == nId )
                {
                // bei Collate mu\s die Subklasse die Kopien selbst
                // handlen, sonst kann das der Druckertreiber
                DBG_ASSERT( pPrintDlg, "der PrintDlg ist weg" );
                nCopies  = pPrintDlg->GetCopyCount();
                bCollate = pPrintDlg->IsCollateChecked();
                if( bCollate )
                    pPrinter->SetCopyCount( 1 );
                else
                    pPrinter->SetCopyCount(nCopies);
                }
            }
            else
                // Abbruch => Einstellungen werden nicht ben"otigt
                DELETEX(pDlgPrinter);

            // Printer-Setup-Dialog abr"aumen
            DELETEX(pOptBtn);
    #ifndef VCL
            if ( pPrintDlg )
                pPrintDlg->SetSetupDialog(0);
    #endif
            DELETEX(pPrintSetupDlg);

            // nur f"urs tats"achliche Drucken den Druck-Dialog behalten
            if ( SID_PRINTDOC != nId || nDialogRet == RET_CANCEL )
            {
                DELETEZ(pPrintDlg);
                if ( nDialogRet == RET_CANCEL )
                    rReq.Ignore();
                else if ( SID_PRINTER_NAME != nId )
                    rReq.SetReturnValue(SfxBoolItem(0,FALSE));
                break;
            }
        }
        else if ( rReq.GetArgs() )
        {
            // zum transportieren brauchen wir einen Dialog
            pPrintDlg = CreatePrintDialog( GetWindow() );
            pPrintDlg->SetPrinter( pPrinter );
                    ::DisableRanges( *pPrintDlg, pPrinter );

            // PrintFile
            SFX_REQUEST_ARG(rReq, pFileItem, SfxStringItem, SID_FILE_NAME, FALSE);
            if ( pFileItem )
            {
                pPrinter->EnablePrintFile(TRUE);
                pPrinter->SetPrintFile( pFileItem->GetValue() );
            }

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

            // Copy-Count
            SFX_REQUEST_ARG(rReq, pCopyItem, SfxInt16Item, SID_PRINT_COPIES, FALSE);
            if ( pCopyItem )
            {
                nCopies = pCopyItem->GetValue();
                pPrintDlg->SetCopyCount( nCopies );
            }

            // arbeitet die abgeleitete View mit einem Range?
            if ( pPagesItem ||
                 pPrintDlg->IsRangeEnabled(PRINTDIALOG_RANGE) )
            {
                // auf Range schalten
                pPrintDlg->CheckRange(PRINTDIALOG_RANGE);

                // Range-Text setzen
                if ( pPagesItem )
                    pPrintDlg->SetRangeText( pPagesItem->GetValue() );
                else
                {
                    String aRange = String::CreateFromInt32( nFrom );
                    aRange += '-';
                    aRange += String::CreateFromInt32( nTo );
                    pPrintDlg->SetRangeText( aRange );
                }
            }
            else
            {
                // auf von-bis schalten
                pPrintDlg->CheckRange(PRINTDIALOG_FROMTO);

                // von-bis eintragen
                pPrintDlg->SetFirstPage( nFrom );
                pPrintDlg->SetLastPage( nTo );
            }
        }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    //   SID_PRINTDOC:
    case SID_PRINTDOCDIRECT:
    {
        // falls der nicht zuvor eingestellt wurde
        if ( !pPrinter )
            pPrinter = GetPrinter(TRUE);
        //! ??? pPrn->SetJobSetup(pSh->GetJob());
        // . . . gfs. Druckerschacht umschalten
        //! ??? if( nPaperBin != USE_DEFAULT_PAPERBIN )
        //! ???     pPrn->SetPaperBin(nPaperBin);

            // Drucker nicht vorhanden? (bei SID_PRINTDOC wurde schon gefragt)
            if ( SID_PRINTDOCDIRECT == nId &&
                 !pPrinter->IsOriginal() && bWarn &&
                 !UseStandardPrinter_Impl( NULL, pPrinter ) )
            {
                rReq.SetReturnValue(SfxBoolItem(0,FALSE));
                return;
            }

        if( bCollate )
            pPrinter->SetCopyCount(1);
        else
            pPrinter->SetCopyCount(nCopies);

        // Drucker in Thread-Mode setzen
        // unter Windows mu\s das so, weil sonst kein Querdruck funkt,
        // unter OS/2 sollte man das nutzen - Apps kommen aber nicht klar
        // WP: 07.12.95: SV macht das jetzt richtig
        SfxIniManager *pIniMgr = SFX_INIMANAGER();
        String aPages( pIniMgr->Get(SFX_KEY_PAGEQUEUESIZE) );
        pPrinter->SetPageQueueSize( aPages.Len() ? (int) aPages.ToInt32() : 1 );

#ifdef OS2
        HACK(die PrintThreadPrio erstmal nur fuer OS/2)
        String aPrio( pIniMgr->Get(SFX_KEY_PRINTTHREADPRIO) );
        if ( aPrio.Len() && aPrio.IsNumeric() )
        Sysdepen::SetPrintThreadPrio( (USHORT) aPrio );
        String aMeta( pIniMgr->Get(SFX_KEY_METAFILEPRINT) );
        if ( aMeta.Len() && aMeta.IsNumeric() )
        Sysdepen::EnableMetafilePrint( 0 != USHORT(aMeta) );
#endif
        SfxObjectShell *pObjSh = GetObjectShell();
        SfxDocumentInfo *pInfo = &pObjSh->GetDocInfo();
        SfxStamp aOldStamp = pInfo->GetPrinted();
        // Abfrage, ob die Benutzerdaten
        // f"ur die Eigenschaften verwendet werden sollen
        String aUserName = pIniMgr->GetUserFullName();

        if ( !pInfo->IsUseUserData() )
            aUserName.Erase();

        pInfo->SetPrinted( aUserName );
        pObjSh->Broadcast( SfxDocumentInfoHint( pInfo ) );

        ErrCode nError = DoPrint( pPrinter, pPrintDlg, bSilent );
        if ( nError == PRINTER_OK )
        {
            pObjSh->FlushDocInfo();

            Invalidate( SID_PRINTDOC );
            Invalidate( SID_PRINTDOCDIRECT );
            Invalidate( SID_SETUPPRINTER );
            rReq.SetReturnValue(SfxBoolItem(0,TRUE));
            SFX_REQUEST_ARG(rReq, pAsyncItem, SfxBoolItem, SID_ASYNCHRON, FALSE);
            if ( pAsyncItem && !pAsyncItem->GetValue() )
            {
                while ( pPrinter->IsPrinting())
                    Application::Yield();
            }
        }
        else
        {
            pInfo->SetPrinted(aOldStamp);
            pObjSh->Broadcast( SfxDocumentInfoHint( pInfo ) );

            if ( nError != PRINTER_ABORT )
            {
                // Printer konnte nicht gestartet werden
                String aMsg( SfxResId( STR_NOSTARTPRINTER ) );
                if ( !rReq.IsAPI() )
                ErrorBox( NULL, WB_OK | WB_DEF_OK,  aMsg ).Execute();
                rReq.SetReturnValue(SfxBoolItem(0,FALSE));
            }
            else
            {
                // Benutzer hat abgebrochen
                rReq.Ignore();
            }
        }

        delete pPrintDlg;
        break;
    }
    }
}

// Optimierungen wieder einschalten
#ifdef WNT
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
    PrintDialog *pDlg = new PrintDialog( pParent );
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
                               BOOL bSilent )
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
        Print( *pProgress, pPrintDlg );
        pProgress->Stop();
        pProgress->DeleteOnEndPrint();
        pPrinter->EndJob();
    }
    else
    {
        // Printer konnte nicht gestartet werden
        delete pProgress;
        return ERRCODE_IO_ABORT;
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

USHORT SfxViewShell::Print( SfxProgress &rProgress, PrintDialog * )
{
    SfxObjectShell *pObjShell = GetViewFrame()->GetObjectShell();
    SFX_APP()->NotifyEvent(SfxEventHint(SFX_EVENT_PRINTDOC, pObjShell));
    return 0;
}

//--------------------------------------------------------------------

SfxPrinter* SfxViewShell::GetPrinter( BOOL bCreate )
{
    return 0;
}

//--------------------------------------------------------------------

USHORT SfxViewShell::SetPrinter( SfxPrinter *pNewPrinter, USHORT nDiffFlags )
{
    return 0;
}

//--------------------------------------------------------------------

SfxTabPage* SfxViewShell::CreatePrintOptionsPage
(
    Window*             pParent,
    const SfxItemSet&   rOptions
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


