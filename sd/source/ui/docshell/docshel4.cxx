/*************************************************************************
 *
 *  $RCSfile: docshel4.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: pw $ $Date: 2000-10-27 14:29:25 $
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

#ifndef _SFX_PROGRESS_HXX
#include <sfx2/progress.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#define ITEMID_FONTLIST     SID_ATTR_CHAR_FONTLIST
#ifndef _SVX_FLSTITEM_HXX
#include <svx/flstitem.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXFLAGITEM_HXX //autogen
#include <svtools/flagitem.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _DISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SFXSTBMGR_HXX //autogen
#include <sfx2/stbmgr.hxx>
#endif
#ifndef _SFX_ECODE_HXX //autogen
#include <svtools/sfxecode.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifdef TF_STARONE
#include "unomodel.hxx"
#endif
#ifndef _OFF_APP_HXX
#include <offmgr/app.hxx>
#endif
#ifndef _OFA_FLTRCFG_HXX
#include <offmgr/fltrcfg.hxx>
#endif
#ifndef _SVXMSBAS_HXX
#include <svx/svxmsbas.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif

#pragma hdrstop

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"

#include "strmname.h"
#include "frmview.hxx"
#include "docshell.hxx"
#include "optsitem.hxx"
#include "sdoutl.hxx"
#include "sdattr.hxx"
#include "drawdoc.hxx"
#include "viewshel.hxx"
#include "app.hxx"
#include "sdview.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "drviewsh.hxx"
#include "sdwindow.hxx"
#include "new_foil.hxx"
#include "sdmod.hxx"
#include "outlnvsh.hxx"

#ifndef SO2_DECL_SVSTORAGESTREAM_DEFINED
#define SO2_DECL_SVSTORAGESTREAM_DEFINED
SO2_DECL_REF(SvStorageStream)
#endif

#define POOL_BUFFER_SIZE        (USHORT)32768
#define BASIC_BUFFER_SIZE       (USHORT)8192
#define DOCUMENT_BUFFER_SIZE    (USHORT)32768

/*************************************************************************
|*
|* SfxPrinter ggf. erzeugen und zurueckgeben
|*
\************************************************************************/

SfxPrinter* SdDrawDocShell::GetPrinter(BOOL bCreate)
{
    if (bCreate && !pPrinter)
    {
        // ItemSet mit speziellem Poolbereich anlegen
        SfxItemSet* pSet = new SfxItemSet( GetPool(),
                            SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                            ATTR_OPTIONS_PRINT,         ATTR_OPTIONS_PRINT,
                            0 );
        // PrintOptionsSet setzen
        SdOptionsPrintItem aPrintItem( ATTR_OPTIONS_PRINT,
                            SD_MOD()->GetSdOptions(pDoc->GetDocumentType()));
        SfxFlagItem aFlagItem( SID_PRINTER_CHANGESTODOC );
        USHORT      nFlags = 0;

        nFlags =  (aPrintItem.IsWarningSize() ? SFX_PRINTER_CHG_SIZE : 0) |
                (aPrintItem.IsWarningOrientation() ? SFX_PRINTER_CHG_ORIENTATION : 0);
        aFlagItem.SetValue( nFlags );

        pSet->Put( aPrintItem );
        pSet->Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aPrintItem.IsWarningPrinter() ) );
        pSet->Put( aFlagItem );

        pPrinter = new SfxPrinter(pSet);
        bOwnPrinter = TRUE;

        // Ausgabequalitaet setzen
        UINT16 nQuality = aPrintItem.GetOutputQuality();

        ULONG nMode = DRAWMODE_DEFAULT;
        if( nQuality == 1 )
            nMode = DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_BLACKTEXT |
                    DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT;
        else if( nQuality == 2 )
            nMode = DRAWMODE_BLACKLINE | DRAWMODE_BLACKTEXT | DRAWMODE_WHITEFILL |
                    DRAWMODE_GRAYBITMAP | DRAWMODE_WHITEGRADIENT;

        pPrinter->SetDrawMode( nMode );

        MapMode aMM (pPrinter->GetMapMode());
        aMM.SetMapUnit(MAP_100TH_MM);
        pPrinter->SetMapMode(aMM);

        // der Drawing Engine den neuen Printer als Referenz-Device setzen
        if (pDoc)
        {
            pDoc->SetRefDevice(pPrinter);

            SdOutliner* pOutl = pDoc->GetOutliner(FALSE);

            if (pOutl)
                pOutl->SetRefDevice(pPrinter);

            SdOutliner* pInternalOutl = pDoc->GetInternalOutliner(FALSE);

            if (pInternalOutl)
                pInternalOutl->SetRefDevice(pPrinter);
        }
    }
    return pPrinter;
}

/*************************************************************************
|*
|* neuen SfxPrinter setzen (Eigentuemeruebergang)
|*
\************************************************************************/

void SdDrawDocShell::SetPrinter(SfxPrinter *pNewPrinter)
{
    if ( pViewShell )
    {
        SdView* pView = pViewShell->GetView();
        if ( pView->IsTextEdit() )
            pView->EndTextEdit();
    }

    if ( pPrinter && bOwnPrinter && (pPrinter != pNewPrinter) )
    {
        delete pPrinter;
    }

    pPrinter = pNewPrinter;
    bOwnPrinter = TRUE;

    delete pFontList;
    pFontList = new FontList( GetPrinter(TRUE), Application::GetDefaultDevice(), FALSE );
    SvxFontListItem aFontListItem( pFontList );
    PutItem( aFontListItem );

    // der Drawing Engine den neuen Printer als Referenz-Device setzen
    if (pDoc)
    {
        pDoc->SetRefDevice(pPrinter);

        SdOutliner* pOutl = pDoc->GetOutliner(FALSE);

        if (pOutl)
            pOutl->SetRefDevice(pPrinter);

        SdOutliner* pInternalOutl = pDoc->GetInternalOutliner(FALSE);

        if (pInternalOutl)
            pInternalOutl->SetRefDevice(pPrinter);
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/
Printer* SdDrawDocShell::GetDocumentPrinter()
{
    return GetPrinter(FALSE);
}

/*************************************************************************
|*
|*
|*
\************************************************************************/
void SdDrawDocShell::OnDocumentPrinterChanged(Printer* pNewPrinter)
{
//  if (pPrinter->IsA(SfxPrinter))
    {
        // Da kein RTTI verfuegbar, wird hart gecasted (...)
        SetPrinter((SfxPrinter*) pNewPrinter);

        // Printer gehoert dem Container
        bOwnPrinter = FALSE;
    }
}


/*************************************************************************
|*
|* InitNew, (Dokument wird neu erzeugt): Streams oeffnen
|*
\************************************************************************/

BOOL SdDrawDocShell::InitNew( SvStorage * pStor )
{
    BOOL bRet = FALSE;

    bRet = SfxInPlaceObject::InitNew( pStor );

    Rectangle aVisArea( Point(0, 0), Size(14100, 10000) );
    SetVisArea(aVisArea);

    if (bRet)
    {
        if( !pDoc )
        {
            pDoc = new SdDrawDocument(eDocType, this);
            SetModel(new SdXImpressDocument(this));
            SetPool( &pDoc->GetItemPool() );
            pUndoManager = new SfxUndoManager;
            UpdateTablePointers();
            SetStyleFamily(5);       //CL: eigentlich SFX_STYLE_FAMILY_PSEUDO
        }

        if( !bSdDataObj )
            pDoc->NewOrLoadCompleted(NEW_DOC);  // otherwise calling
                                                // NewOrLoadCompleted(NEW_LOADED) in
                                                // SdDrawDocument::AllocModel()

    }
    return bRet;
}

/*************************************************************************
|*
|* Load: Pools und Dokument laden
|*
\************************************************************************/

BOOL SdDrawDocShell::Load( SvStorage * pStor )
{
    BOOL bRet = FALSE;

    ULONG nStorFmt = pStor->GetFormat();

    if ( nStorFmt == SOT_FORMATSTR_ID_STARIMPRESS_50 ||
         nStorFmt == SOT_FORMATSTR_ID_STARDRAW_50    ||
         nStorFmt == SOT_FORMATSTR_ID_STARDRAW_40    ||
         nStorFmt == SOT_FORMATSTR_ID_STARDRAW )
    {
        StreamMode aStreamMode = STREAM_STD_READWRITE;

        pDoc = new SdDrawDocument(eDocType, this);
        SetModel(new SdXImpressDocument(this));

        SfxItemSet* pSet = GetMedium()->GetItemSet();

        if (pSet                                                 &&
            SFX_ITEM_SET == pSet->GetItemState(SID_PREVIEW)      &&
            ((SfxBoolItem&) (pSet->Get(SID_PREVIEW))).GetValue())
        {
            // Bei einer Template-Preview (Vorlagen-Dialog) sollen nur die ersten
            // drei Seiten geladen werden (Handzettel-, 1.Standard- und 1.Notizseite)
            pDoc->SetStarDrawPreviewMode(TRUE);
        }

        SetPool( &pDoc->GetItemPool() );
        pUndoManager = new SfxUndoManager;
        SetStyleFamily(5);       //CL: eigentlich SFX_STYLE_FAMILY_PSEUDO

        if (GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
            pProgress = NULL;
        else
        {
            if( mpSpecialProgress )
                pProgress = mpSpecialProgress;
            else
                pProgress = new SfxProgress( this, String( SdResId( STR_OPEN_DOCUMENT )), 100 );
        }

        if( pProgress )
        {
            if( mpSpecialProgress )
                pDoc->SetIOProgressHdl( *mpSpecialProgressHdl );
            else
            {
                pDoc->SetIOProgressHdl( LINK( this, SdDrawDocShell, IOProgressHdl ) );

                pProgress->SetState( 0, 100 );
            }
        }

        bRet = SfxInPlaceObject::Load(pStor);

        BOOL bIsEmptyStreams = FALSE;

        if (pStor->IsStream(pSfxStyleSheets))
        {
            SvStorageStreamRef aPoolStm = pStor->OpenStream(pSfxStyleSheets,
                                                            aStreamMode);
            bRet = aPoolStm->GetError() == 0;

            if (!bRet)
            {
                // Stream konnte nicht geoeffnet werden
                // Kann der Stream denn als read geoeffnet werden?
                aPoolStm.Clear();
                aStreamMode = STREAM_STD_READ;
                aPoolStm = pStor->OpenStream(pSfxStyleSheets, aStreamMode);
                bRet = aPoolStm->GetError() == 0;
            }

            // Pool laden
            if (bRet)
            {
                aPoolStm->SetVersion(pStor->GetVersion());
                GetPool().SetFileFormatVersion(pStor->GetVersion());
                aPoolStm->SetBufferSize(POOL_BUFFER_SIZE);
                GetPool().Load(*aPoolStm);
            }
            bRet = aPoolStm->GetError() == 0;
            DBG_ASSERT(bRet, "Fehler beim Laden des Item-Pools");

            // StyleSheet-Pool laden
            if (bRet)
            {
                SfxStyleSheetPool* pSSSP = (SfxStyleSheetPool*)GetStyleSheetPool();
                GetStyleSheetPool()->Load(*aPoolStm);
                aPoolStm->SetBufferSize(0);
                bRet = aPoolStm->GetError() == 0;
                DBG_ASSERT(bRet, "Fehler beim Laden des StyleSheet-Pools");
            }

            if ( !bRet || aPoolStm->GetErrorCode() )
                SetError( aPoolStm->GetErrorCode() );
        }
        else
        {
            // wenn der Stream gar nicht vorhanden ist, gilt das trotzdem als
            // Erfolg (fuer das Laden ungeaenderter OLE-Objekte)
            bIsEmptyStreams = TRUE;
            bRet = TRUE;
        }

        // Model der Drawing Engine laden
        if (bRet)
        {
            // altes oder neues Format enthalten?
            BOOL bAcceptableStreamFound = FALSE;
            SvStorageStreamRef aDocStm;
            if (pStor->IsStream(pStarDrawDoc))
            {
                bAcceptableStreamFound = TRUE;
                aDocStm = pStor->OpenStream(pStarDrawDoc, aStreamMode);
            }
            else if (pStor->IsStream(pStarDrawDoc3))
            {
                bAcceptableStreamFound = TRUE;
                aDocStm = pStor->OpenStream(pStarDrawDoc3, aStreamMode);
            }

            if (bAcceptableStreamFound)
            {
                aDocStm->SetVersion(pStor->GetVersion());
                bRet = aDocStm->GetError() == 0;
                if(bRet)
                {
                    aDocStm->SetBufferSize(DOCUMENT_BUFFER_SIZE);
                    aDocStm->SetKey( pStor->GetKey() ); // Passwort setzen
                    pDoc->SetModelStorage( pStor );
                    *aDocStm >> *pDoc;
                    pDoc->SetModelStorage( NULL );
                    bRet = aDocStm->GetError() == 0;

                    if (!bRet)
                    {
                        if (pStor->GetKey().Len() == 0)
                            SetError(ERRCODE_SFX_DOLOADFAILED);  // kein Passwort gesetzt --> Datei marode
                        else
                            SetError(ERRCODE_SFX_WRONGPASSWORD); // Passwort gesetzt --> war wohl falsch
                    }
                    else if ( aDocStm->GetErrorCode() )
                        SetError( aDocStm->GetErrorCode() );

                    aDocStm->SetBufferSize(0);
                }
                bIsEmptyStreams = FALSE;
            }
            // wenn der Stream gar nicht vorhanden ist, gilt das trotzdem als
            // Erfolg (fuer das Laden ungeaenderter OLE-Objekte)
        }

        if (bRet && pDoc)
        {
            if (bIsEmptyStreams == TRUE)
            {
                pDoc->NewOrLoadCompleted(NEW_DOC);
            }
            else
            {
                pDoc->NewOrLoadCompleted(DOC_LOADED);
            }
        }

        if( pProgress && !mpSpecialProgress )
        {
            delete pProgress;
            pProgress = NULL;
        }

        if (bRet)
        {
            UpdateTablePointers();

            if (GetCreateMode() == SFX_CREATE_MODE_EMBEDDED &&
                SfxInPlaceObject::GetVisArea().IsEmpty())
            {
                // Leere VisArea: auf Seitengroesse setzen
                SdPage* pPage = pDoc->GetSdPage(0, PK_STANDARD);

                if (pPage)
                    SetVisArea(Rectangle(pPage->GetAllObjBoundRect()));
            }

            FinishedLoading(SFX_LOADED_ALL);
        }
    }
    else
    {
        pStor->SetError(SVSTREAM_WRONGVERSION);
    }

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdDrawDocShell::HandsOff()
{
    SfxInPlaceObject::HandsOff();
    pDocStor = NULL;
}

/*************************************************************************
|*
|* Save: Pools und Dokument in die offenen Streams schreiben
|*
\************************************************************************/

BOOL SdDrawDocShell::Save()
{
    BOOL bRet;

    // Late-Init muss fertig sein
    pDoc->StopWorkStartupDelay();

    if (GetCreateMode() == SFX_CREATE_MODE_STANDARD)
    {
        // Normal bearbeitet -> keine VisArea
        SvInPlaceObject::SetVisArea( Rectangle() );
    }

    if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
        bRet=SfxInPlaceObject::Save();
    else
        bRet=TRUE;

    if (GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        pProgress = NULL;
    else
        pProgress = new SfxProgress( this, String(SdResId( STR_SAVE_DOCUMENT )), 100 );

    if( pProgress )
    {
        pDoc->SetIOProgressHdl( LINK( this, SdDrawDocShell, IOProgressHdl ) );

        pProgress->SetState( 0, 100 );
    }

    // komprimiert/native speichern?
    SvtSaveOptions                          aOptions;
    const SvtSaveOptions::SaveGraphicsMode  eSaveMode( aOptions.GetSaveGraphicsMode() );
    const BOOL                              bSaveCompressed = ( SvtSaveOptions::SaveGraphicsCompressed == eSaveMode );
    const BOOL                              bSaveNative = ( SvtSaveOptions::SaveGraphicsOriginal == eSaveMode );

    pDoc->SetSaveCompressed( bSaveCompressed );
    pDoc->SetSaveNative( bSaveNative );

    if( bRet )
    {
        pDoc->PrepareStore();

        SvStorage* pStor = GetStorage();
        SvStorageStreamRef aPoolStm = pStor->OpenStream(pSfxStyleSheets, STREAM_READ | STREAM_WRITE | STREAM_TRUNC);
        aPoolStm->SetVersion(pStor->GetVersion());

        if ( !aPoolStm->GetError() )
        {
            aPoolStm->SetSize(0);
            aPoolStm->SetBufferSize(POOL_BUFFER_SIZE);
            const long nVersion = pStor->GetVersion();
            GetPool().SetFileFormatVersion( nVersion );
            const USHORT nOldComprMode = aPoolStm->GetCompressMode();
            USHORT       nNewComprMode = nOldComprMode;

            if( SOFFICE_FILEFORMAT_40 <= nVersion )
            {
                if( bSaveCompressed )
                    nNewComprMode |= COMPRESSMODE_ZBITMAP;

                if( bSaveNative )
                    nNewComprMode |= COMPRESSMODE_NATIVE;

                aPoolStm->SetCompressMode( nNewComprMode );
            }

            GetPool().Store(*aPoolStm);

            // der StyleSheetPool benutzt beim Speichern intern First()/Next(),
            // setzt aber nicht die Suchmasken zurueck, darum machen
            // wir das selbst
            GetStyleSheetPool()->SetSearchMask(SFX_STYLE_FAMILY_ALL);
            // FALSE = auch unbenutzte Vorlagen speichern
            GetStyleSheetPool()->Store(*aPoolStm, FALSE);
            aPoolStm->SetBufferSize(0);

            if( nOldComprMode != nNewComprMode )
                aPoolStm->SetCompressMode( nOldComprMode );
        }
        else bRet = FALSE;
        bRet = bRet && (aPoolStm->GetError() == 0);
        DBG_ASSERT(bRet, "Fehler beim Schreiben der Pools");
        if ( !bRet || aPoolStm->GetErrorCode() )
            SetError( aPoolStm->GetErrorCode() );

        if ( GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
        {
            // Hat der Dokument-Stream noch den alten Namen? Wenn ja, wird er
            // umbenannt, denn es wird in jedem Fall das neue Format
            // geschrieben.
            if (pStor->IsStream(pStarDrawDoc))
            {
                BOOL bOK = pStor->Rename(pStarDrawDoc, pStarDrawDoc3);
                DBG_ASSERT(bOK, "Umbenennung des Streams gescheitert");
            }

            SvStorageStreamRef aDocStm = pStor->OpenStream(pStarDrawDoc3, STREAM_READ | STREAM_WRITE | STREAM_TRUNC);
            aDocStm->SetVersion(pStor->GetVersion());
            if ( !aDocStm->GetError() )
            {
                aDocStm->SetSize(0);
                aDocStm->SetBufferSize(DOCUMENT_BUFFER_SIZE);
                aDocStm->SetKey( pStor->GetKey() ); // Passwort setzen
                *aDocStm << *pDoc;
                aDocStm->SetBufferSize(0);
            }
            else bRet = FALSE;
            if (bRet)
                bRet = aDocStm->GetError() == 0;
            DBG_ASSERT(bRet, "Fehler beim Schreiben des Models");
            if ( !bRet || aDocStm->GetErrorCode() )
                SetError( aDocStm->GetErrorCode() );
        }

        if (bRet && GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
            AddXMLAsZipToTheStorage( *pStor );

    }

    if( pProgress )
    {
        delete pProgress;
        pProgress = NULL;
    }

    return bRet;
}

/*************************************************************************
|*
|* SaveAs: Pools und Dokument in den angegebenen Storage sichern
|*
\************************************************************************/

BOOL SdDrawDocShell::SaveAs( SvStorage * pStor )
{
    // Late-Init muss fertig sein
    pDoc->StopWorkStartupDelay();

    if (GetCreateMode() == SFX_CREATE_MODE_STANDARD)
    {
        // Normal bearbeitet -> keine VisArea
        SvInPlaceObject::SetVisArea( Rectangle() );
    }

    UINT32 nVBWarning = ERRCODE_NONE;
    BOOL bRet = SfxInPlaceObject::SaveAs(pStor);
    if ( bRet )
    {
        OfficeApplication* pApplication = OFF_APP();
        if ( pApplication )
        {
            OfaFilterOptions* pBasOpt = pApplication->GetFilterOptions();
            if ( pBasOpt && pBasOpt->IsLoadPPointBasicStorage() )
                nVBWarning = SvxImportMSVBasic::GetSaveWarningOfMSVBAStorage( *this );
        }
    }
    // komprimiert/native speichern?
    SvtSaveOptions                          aOptions;
    const SvtSaveOptions::SaveGraphicsMode  eSaveMode( aOptions.GetSaveGraphicsMode() );
    const BOOL                              bSaveCompressed = ( SvtSaveOptions::SaveGraphicsCompressed == eSaveMode );
    const BOOL                              bSaveNative = ( SvtSaveOptions::SaveGraphicsOriginal == eSaveMode );

    pDoc->SetSaveCompressed( bSaveCompressed );
    pDoc->SetSaveNative( bSaveNative );

    if (GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        pProgress = NULL;
    else
    {
        if( mpSpecialProgress )
            pProgress = mpSpecialProgress;
        else
            pProgress = new SfxProgress( this, String(SdResId( STR_SAVE_DOCUMENT )), 100 );
    }

    if( pProgress )
    {
        if( mpSpecialProgress )
            pDoc->SetIOProgressHdl( *mpSpecialProgressHdl );
        else
        {
            pDoc->SetIOProgressHdl( LINK( this, SdDrawDocShell, IOProgressHdl ) );

            pProgress->SetState( 0, 100 );
        }
    }

    if (bRet)
    {
        pDoc->PrepareStore();
        SvStorageStreamRef aStm = pStor->OpenStream(pSfxStyleSheets, STREAM_READ | STREAM_WRITE | STREAM_TRUNC);
        aStm->SetVersion(pStor->GetVersion());

        if( !aStm->GetError() )
        {
            aStm->SetBufferSize(POOL_BUFFER_SIZE);
            const long nVersion = pStor->GetVersion();
            GetPool().SetFileFormatVersion( nVersion );
            const USHORT nOldComprMode = aStm->GetCompressMode();
            USHORT       nNewComprMode = nOldComprMode;

            if( SOFFICE_FILEFORMAT_40 <= nVersion )
            {
                if( bSaveCompressed )
                    nNewComprMode |= COMPRESSMODE_ZBITMAP;

                if( bSaveNative )
                    nNewComprMode |= COMPRESSMODE_NATIVE;

                aStm->SetCompressMode( nNewComprMode );
            }

            GetPool().Store(*aStm);

            // der StyleSheetPool benutzt beim Speichern intern First()/Next(),
            // setzt aber nicht die Suchmasken zurueck, darum machen
            // wir das selbst
            GetStyleSheetPool()->SetSearchMask(SFX_STYLE_FAMILY_ALL);
            // FALSE = auch unbenutzte Vorlagen speichern
            GetStyleSheetPool()->Store(*aStm, FALSE);
            aStm->SetBufferSize(0);

            if( nOldComprMode != nNewComprMode )
                aStm->SetCompressMode( nOldComprMode );
        }
        else bRet = FALSE;
        if (bRet)
            bRet = aStm->GetError() == 0;
        DBG_ASSERT(bRet, "Fehler beim Schreiben der Pools");
        if ( !bRet || aStm->GetErrorCode() )
            SetError( aStm->GetErrorCode() );

        if ( GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
        {
            aStm = pStor->OpenStream(pStarDrawDoc3, STREAM_READ | STREAM_WRITE | STREAM_TRUNC);
            aStm->SetVersion(pStor->GetVersion());
            if ( !aStm->GetError() )
            {
                aStm->SetBufferSize(DOCUMENT_BUFFER_SIZE);
                aStm->SetKey( pStor->GetKey() ); // Passwort setzen
                *aStm << *pDoc;
                aStm->SetBufferSize(0);
            }
            else bRet = FALSE;
            if (bRet)
                bRet = aStm->GetError() == 0;
            DBG_ASSERT(bRet, "Fehler beim Schreiben des Models");
            if ( !bRet || aStm->GetErrorCode() )
                SetError( aStm->GetErrorCode() );
        }
    }

    if (bRet && GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
        AddXMLAsZipToTheStorage( *pStor );

    if( pProgress && !mpSpecialProgress )
    {
        delete pProgress;
        pProgress = NULL;
    }
    if ( GetError() == ERRCODE_NONE )
        SetError( nVBWarning );
    return bRet;
}

/*************************************************************************
|*
|* SaveCompleted: die eigenen Streams wieder oeffnen, damit kein anderer
|*                                sie "besetzt"
|*
\************************************************************************/

BOOL SdDrawDocShell::SaveCompleted( SvStorage * pStor )
{
    BOOL bRet = FALSE;

    if ( SfxInPlaceObject::SaveCompleted(pStor) )
    {
        pDoc->NbcSetChanged(FALSE); // Nbc.. damit keine Rekursion auftritt

        if (pViewShell && pViewShell->ISA(SdOutlineViewShell))
        {
            ((SdOutlineView*) pViewShell->GetView())->GetOutliner()->ClearModifyFlag();
        }

        bRet = TRUE;

        // Damit der Navigator nach dem Speichern updaten kann!
        ( ( pViewShell && pViewShell->GetViewFrame() ) ?
          pViewShell->GetViewFrame() : SfxViewFrame::Current() )->
          GetBindings().Invalidate( SID_NAVIGATOR_STATE, TRUE, FALSE );
    }
    return bRet;
}


/*************************************************************************
|*
|* LoadFrom: Inhalte fuer Organizer laden
|*
\************************************************************************/

BOOL SdDrawDocShell::LoadFrom(SvStorage* pStor)
{
    BOOL bRet = FALSE;

    BOOL bRet1 = SfxObjectShell::LoadFrom(pStor);
    BOOL bRet2 = TRUE;
    WaitObject* pWait = NULL;

    if( pViewShell )
        pWait = new WaitObject( (Window*) pViewShell->GetActiveWindow() );

    // da trotz eines erfolgten InitNew() noch LoadFrom() gerufen werden kann,
    // muessen die Vorlagen hier geloescht werden
    GetStyleSheetPool()->Clear();

    // Pool und StyleSheet Pool laden
    SvStorageStreamRef aPoolStm = pStor->OpenStream(pSfxStyleSheets);
    aPoolStm->SetVersion(pStor->GetVersion());
    bRet2 = aPoolStm->GetError() == 0;
    if (bRet2)
    {
        aPoolStm->SetBufferSize(POOL_BUFFER_SIZE);
        GetPool().SetFileFormatVersion(pStor->GetVersion());
        GetPool().Load(*aPoolStm);
        bRet2 = aPoolStm->GetError() == 0;
        DBG_ASSERT(bRet2, "Fehler beim Laden des Item-Pools");
    }

    if (bRet2)
    {
        GetStyleSheetPool()->Load(*aPoolStm);
        bRet2 = aPoolStm->GetError() == 0;
        aPoolStm->SetBufferSize(0);
        DBG_ASSERT(bRet2, "Fehler beim Laden des StyleSheet-Pools");
    }

    delete pWait;

    bRet = bRet1 || bRet2;
    return bRet;
}

/*************************************************************************
|*
|* Referenz auf Dokument
|*
\************************************************************************/

SdDrawDocument* SdDrawDocShell::GetDoc()
{
    return pDoc;
}

/*************************************************************************
|*
|* Referenz auf Dokument
|*
\************************************************************************/

SfxStyleSheetBasePool* SdDrawDocShell::GetStyleSheetPool()
{
    return( (SfxStyleSheetBasePool*) pDoc->GetStyleSheetPool() );
}

/*************************************************************************
|*
|* IOPProgressHdl()
|*
\************************************************************************/

IMPL_LINK( SdDrawDocShell, IOProgressHdl, USHORT*, pPercent )
{
    if( pProgress )
        pProgress->SetState( *pPercent );

    return 0;
}

/*************************************************************************
|*
|* Sprung zu Bookmark
|*
\************************************************************************/

BOOL SdDrawDocShell::GotoBookmark(const String& rBookmark)
{
    BOOL bFound = FALSE;

    if (pViewShell && pViewShell->ISA(SdDrawViewShell))
    {
        SdDrawViewShell* pDrViewSh = (SdDrawViewShell*) pViewShell;

        String aBookmark( rBookmark );

        if( rBookmark.Len() && rBookmark.GetChar( 0 ) == sal_Unicode('#') )
            aBookmark = rBookmark.Copy( 1 );

        // Ist das Bookmark eine Seite?
        USHORT nPgNum = pDoc->GetPageByName(aBookmark);
        SdrObject* pObj = NULL;

        if (nPgNum == SDRPAGE_NOTFOUND)
        {
            // Ist das Bookmark ein Objekt?
            pObj = pDoc->GetObj(aBookmark);

            if (pObj)
            {
                nPgNum = pObj->GetPage()->GetPageNum();
            }
        }

        if (nPgNum != SDRPAGE_NOTFOUND)
        {
            /**********************************************************
            * Zur Seite springen
            **********************************************************/
            bFound = TRUE;
            SdPage* pPage = (SdPage*) pDoc->GetPage(nPgNum);

            PageKind eNewPageKind = pPage->GetPageKind();

            if (eNewPageKind != pDrViewSh->GetPageKind())
            {
                // Arbeitsbereich wechseln
                GetFrameView()->SetPageKind(eNewPageKind);
                ( ( pViewShell && pViewShell->GetViewFrame() ) ?
                  pViewShell->GetViewFrame() : SfxViewFrame::Current() )->
                  GetDispatcher()->Execute( SID_VIEWSHELL0, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );

                // Die aktuelle ViewShell hat sich geaendert!
                pDrViewSh = (SdDrawViewShell*) pViewShell;
            }

            EditMode eNewEditMode = EM_PAGE;

            if (pPage->IsMasterPage())
            {
                eNewEditMode = EM_MASTERPAGE;
            }

            if (eNewEditMode != pDrViewSh->GetEditMode())
            {
                // EditMode setzen
                pDrViewSh->ChangeEditMode(eNewEditMode, FALSE);
            }

            // Zur Seite springen
            USHORT nSdPgNum = (nPgNum - 1) / 2;
            pDrViewSh->SwitchPage(nSdPgNum);

            if (pObj)
            {
                // Objekt einblenden und selektieren
                pDrViewSh->MakeVisible(pObj->GetLogicRect(),
                                       *pDrViewSh->GetActiveWindow());
                pDrViewSh->GetView()->UnmarkAll();
                pDrViewSh->GetView()->MarkObj(pObj, pDrViewSh->GetView()->GetPageViewPvNum(0), FALSE);
            }
        }

        SfxBindings& rBindings = ( ( pViewShell && pViewShell->GetViewFrame() ) ?
                                 pViewShell->GetViewFrame() : SfxViewFrame::Current() )->GetBindings();

        rBindings.Invalidate(SID_NAVIGATOR_STATE, TRUE, FALSE);
        rBindings.Invalidate(SID_NAVIGATOR_PAGENAME);
    }

    return (bFound);
}

/*************************************************************************
|*
|* SaveAsOwnFormat: wenn es eine Dokumentvorlage werden soll,
|*
\************************************************************************/

BOOL SdDrawDocShell::SaveAsOwnFormat( SfxMedium& rMedium )
{
    const SfxFilter* pFilter = rMedium.GetFilter();

    if (pFilter->IsOwnTemplateFormat())
    {
        // jetzt die StarDraw-Spezialitaeten:
        // die Layoutvorlagen der ersten Seite werden mit dem jetzt
        // bekannten Layoutnamen versehen, die Layoutnamen der betroffenen
        // Masterpages und Seiten werden gesetzt;
        // alle Textobjekte der betroffenen Standard-, Notiz- und
        // Masterpages werden ueber die Namensaenderung informiert
        String aLayoutName(((SfxStringItem&)(rMedium.GetItemSet()->Get(SID_DOCTEMPLATE))).GetValue());
        String aOldPageLayoutName = pDoc->GetSdPage(0, PK_STANDARD)->GetLayoutName();
        pDoc->RenameLayoutTemplate(aOldPageLayoutName, aLayoutName);
    }

    return SfxObjectShell::SaveAsOwnFormat(rMedium);
}



/*************************************************************************
|*
|* FillClass
|*
\************************************************************************/

void SdDrawDocShell::FillClass(SvGlobalName* pClassName,
                                        ULONG*  pFormat,
                                        String* pAppName,
                                        String* pFullTypeName,
                                        String* pShortTypeName,
                                        long    nFileFormat) const
{
    SfxInPlaceObject::FillClass(pClassName, pFormat, pAppName, pFullTypeName,
                                pShortTypeName, nFileFormat);

    if (nFileFormat == SOFFICE_FILEFORMAT_31)
    {
        *pClassName     = SvGlobalName(SO3_SIMPRESS_CLASSID_30);
        *pFormat        = SOT_FORMATSTR_ID_STARDRAW;
        *pAppName       = String(RTL_CONSTASCII_USTRINGPARAM("Sdraw 3.1"));
        *pFullTypeName  = String(SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_31));;
        *pShortTypeName = String(SdResId(STR_IMPRESS_DOCUMENT));
    }
    else if (nFileFormat == SOFFICE_FILEFORMAT_40)
    {
        *pClassName     = SvGlobalName(SO3_SIMPRESS_CLASSID_40);
        *pFormat        = SOT_FORMATSTR_ID_STARDRAW_40;
        *pFullTypeName  = String(SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_40));
        *pShortTypeName = String(SdResId(STR_IMPRESS_DOCUMENT));
    }
    else if (nFileFormat == SOFFICE_FILEFORMAT_NOW)
    {
        if (eDocType == DOCUMENT_TYPE_DRAW)
        {
            *pFullTypeName  = String(SdResId(STR_GRAPHIC_DOCUMENT_FULLTYPE_50));
            *pShortTypeName = String(SdResId(STR_GRAPHIC_DOCUMENT));
        }
        else
        {
            *pFullTypeName  = String(SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_50));
            *pShortTypeName = String(SdResId(STR_IMPRESS_DOCUMENT));
        }
    }
}




