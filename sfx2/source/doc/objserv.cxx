/*************************************************************************
 *
 *  $RCSfile: objserv.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: mba $ $Date: 2001-03-09 17:55:57 $
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
#include <so3/svstor.hxx>

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#if SUPD<613//MUSTINI
    #ifndef _SFX_INIMGR_HXX //autogen
    #include <inimgr.hxx>
    #endif
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#if SUPD<613//MUSTINI
    #ifndef _SFXINIMGR_HXX //autogen
    #include <svtools/iniman.hxx>
    #endif
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#include <vcl/wrkwin.hxx>
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#include <svtools/sbx.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/useroptions.hxx>

#pragma hdrstop

#include "sfxresid.hxx"
#include "event.hxx"
#include "request.hxx"
#include "printer.hxx"
#include "viewsh.hxx"
#include "doctdlg.hxx"
#include "docfilt.hxx"
#include "docfile.hxx"
#include "docinf.hxx"
#include "dispatch.hxx"
#include "dinfdlg.hxx"
#include "objitem.hxx"
#include "objsh.hxx"
#include "objshimp.hxx"
#include "sfxtypes.hxx"
#include "interno.hxx"
#include "module.hxx"
#include "topfrm.hxx"
#include "versdlg.hxx"
#include "doc.hrc"
#include "docfac.hxx"

//====================================================================

BOOL ShallSetBaseURL_Impl( SfxMedium &rMed );

#define SfxObjectShell
#include "sfxslots.hxx"

AsynchronLink* pPendingCloser = 0;

//=========================================================================

SFX_IMPL_INTERFACE(SfxObjectShell,SfxShell,SfxResId(0))
{
}

long SfxObjectShellClose_Impl( void* pObj, void* pArg )
{
    SfxObjectShell *pObjSh = (SfxObjectShell*) pArg;
    if ( pObjSh->Get_Impl()->bHiddenLockedByAPI )
    {
        pObjSh->Get_Impl()->bHiddenLockedByAPI = FALSE;
        pObjSh->OwnerLock(FALSE);
    }
    else if ( !pObjSh->Get_Impl()->bClosing )
        // GCC stuerzt ab, wenn schon im dtor, also vorher Flag abfragen
        pObjSh->DoClose();
    return 0;
}

//=========================================================================

void SfxObjectShell::PrintExec_Impl(SfxRequest &rReq)
{
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst(this);
    if ( pFrame )
    {
        rReq.SetSlot( SID_PRINTDOC );
        pFrame->GetViewShell()->ExecuteSlot(rReq);
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::PrintState_Impl(SfxItemSet &rSet)
{
    FASTBOOL bPrinting = FALSE;
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst(this, TYPE(SfxTopViewFrame));
    if ( pFrame )
    {
        SfxPrinter *pPrinter = pFrame->GetViewShell()->GetPrinter();
        bPrinting = pPrinter && pPrinter->IsPrinting();
    }
    rSet.Put( SfxBoolItem( SID_PRINTOUT, bPrinting ) );
}

//--------------------------------------------------------------------
void SfxObjectShell::ExecFile_Impl(SfxRequest &rReq)
{
    {DBG_CHKTHIS(SfxObjectShell, 0);}

    pImp->bSetStandardName=FALSE;
    USHORT nId = rReq.GetSlot();
    if ( !GetMedium() && nId != SID_CLOSEDOC )
        return;

    if( nId == SID_SAVEDOC || nId  == SID_UPDATEDOC )
    {
        // Embedded?
        SfxInPlaceObject *pObj=GetInPlaceObject();
        if( pObj && pObj->GetProtocol().IsEmbed() )
        {
            BOOL bRet = pObj->GetClient()->SaveObject();
            rReq.SetReturnValue( SfxBoolItem(0, bRet) );
            rReq.Done();
            return;
        }

        // not-modified => nichts tun, kein BASIC-Laufzeitfehler (API)
        if ( !IsModified() )
        {
            rReq.SetReturnValue( SfxBoolItem(0, FALSE) );
            rReq.Done();
            return;
        }
    }

    // API-Call => suppress dialogs
    SfxBoolResetter aSilentReset( pImp->bSilent );
    if ( rReq.IsAPI() )
        pImp->bSilent = TRUE;

    const SfxFilter *pCurFilter = GetMedium()->GetFilter();
    const SfxFilter *pDefFilter = GetFactory().GetFilter(0);
    if ( nId == SID_SAVEDOC &&
         pCurFilter && !pCurFilter->CanExport() &&
         pDefFilter && pDefFilter->IsInternal() )
        nId = SID_SAVEASDOC;

    // interaktiv speichern via (nicht-Default) Filter?
    if ( !rReq.IsAPI() && GetMedium()->GetFilter() && HasName() &&
         (nId == SID_SAVEDOC || nId == SID_UPDATEDOC) )
    {
        // aktuellen und Default-Filter besorgen

        // Filter kann nicht exportieren und Default-Filter ist verf"ugbar?
        if ( !pCurFilter->CanExport() &&
             !pDefFilter->IsInternal() )
        {
            // fragen, ob im default-Format gespeichert werden soll
            String aWarn(SfxResId(STR_QUERY_MUSTOWNFORMAT));
            aWarn = SearchAndReplace( aWarn, DEFINE_CONST_UNICODE( "$(FORMAT)" ),
                        GetMedium()->GetFilter()->GetFilterName() );
            aWarn = SearchAndReplace( aWarn, DEFINE_CONST_UNICODE( "$(OWNFORMAT)" ),
                        GetFactory().GetFilter(0)->GetFilterName() );
            QueryBox aWarnBox(0,WB_OK_CANCEL|WB_DEF_OK,aWarn);
            if ( aWarnBox.Execute() == RET_OK )
            {
                // ja: Save-As in eigenem Foramt
                rReq.SetSlot(nId = SID_SAVEASDOC);
                pImp->bSetStandardName=TRUE;
            }
            else
            {
                // nein: Abbruch
                rReq.Ignore();
                return;
            }
        }
        else
        {
            // fremdes Format mit m"oglichem Verlust (aber nicht per API) wenn noch nicht gewarnt und anschließend im
            // alien format gespeichert wurde
            if ( !( pCurFilter->IsOwnFormat() && pCurFilter->GetVersion() == SOFFICE_FILEFORMAT_CURRENT || ( pCurFilter->GetFilterFlags() & SFX_FILTER_SILENTEXPORT ) )
                 && ( !pImp->bDidWarnFormat || !pImp->bDidDangerousSave ) )
            {
                // Default-Format verf"ugbar?
                if ( !pDefFilter->IsInternal() && pCurFilter != pDefFilter )
                {
                    // fragen, ob im default-Format gespeichert werden soll
                    String aWarn(SfxResId(STR_QUERY_SAVEOWNFORMAT));
                    aWarn = SearchAndReplace( aWarn, DEFINE_CONST_UNICODE( "$(FORMAT)" ),
                                GetMedium()->GetFilter()->GetFilterName());
                    aWarn = SearchAndReplace( aWarn, DEFINE_CONST_UNICODE( "$(OWNFORMAT)" ),
                                GetFactory().GetFilter(0)->GetFilterName());

                    SfxViewFrame *pFrame = SfxObjectShell::Current() == this ?
                        SfxViewFrame::Current() : SfxViewFrame::GetFirst( this );
                    while ( pFrame && (pFrame->GetFrameType() & SFXFRAME_SERVER ) )
                        pFrame = SfxViewFrame::GetNext( *pFrame, this );

                    if ( pFrame )
                    {
                        SfxFrame* pTop = pFrame->GetTopFrame();
                        SFX_APP()->SetViewFrame( pTop->GetCurrentViewFrame() );
                        pFrame->GetFrame()->Appear();

                        QueryBox aWarnBox(&pFrame->GetWindow(),WB_YES_NO_CANCEL|WB_DEF_YES,aWarn);
                        switch(aWarnBox.Execute())
                        {
                              case RET_YES:
                            {
                                // ja: in Save-As umsetzen
                                rReq.SetSlot(nId = SID_SAVEASDOC);
                                SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pPassItem, SfxStringItem, SID_PASSWORD, FALSE );
                                if ( pPassItem )
                                    rReq.AppendItem( *pPassItem );
                                pImp->bSetStandardName = TRUE;
                                break;
                            }

                              case RET_CANCEL:
                                // nein: Abbruch
                                rReq.Ignore();
                                return;
                        }

                        pImp->bDidWarnFormat=TRUE;
                    }
                }
            }
        }
    }

    // Speichern eines namenslosen oder readonly Dokumentes
    BOOL bMediumRO = IsReadOnlyMedium();
    if ( ( nId == SID_SAVEDOC || nId == SID_UPDATEDOC ) &&
         ( !HasName() || bMediumRO ) )
    {
        if ( rReq.IsAPI() )
        {
            // FALSE zur"uckliefern
            rReq.SetReturnValue( SfxBoolItem( 0, FALSE ) );
            rReq.Done();
            return;
        }
        else
        {
            // in SaveAs umwandlen
            rReq.SetSlot(nId = SID_SAVEASDOC);
            if ( SFX_APP()->IsPlugin() && !HasName() )
            {
                SFX_REQUEST_ARG( rReq, pWarnItem, SfxBoolItem, SID_FAIL_ON_WARNING, FALSE);
                if ( pWarnItem && pWarnItem->GetValue() )
                {
                    INetURLObject aObj( SvtPathOptions().GetWorkPath() );
                    aObj.insertName( GetTitle(), false, INetURLObject::LAST_SEGMENT, true, INetURLObject::ENCODE_ALL );
                    const SfxFilter* pFilter = GetFactory().GetFilter(0);
                    String aExtension( pFilter->GetDefaultExtension().Copy(2) );
                    aObj.setExtension( aExtension, INetURLObject::LAST_SEGMENT, true, INetURLObject::ENCODE_ALL );
                    rReq.AppendItem( SfxStringItem( SID_FILE_NAME, aObj.GetMainURL() ) );
                    rReq.AppendItem( SfxBoolItem( SID_RENAME, TRUE ) );
                }
            }
        }
    }

    switch(nId)
    {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_VERSION:
        {
            SfxViewFrame* pFrame = GetFrame();
            if ( !pFrame )
                pFrame = SfxViewFrame::GetFirst( this );
            if ( !pFrame )
                return;

            if ( !rReq.IsAPI() )
            {
                if ( pFrame->GetFrame()->GetParentFrame() )
                {
                    pFrame->GetTopViewFrame()->GetObjectShell()->ExecuteSlot( rReq );
                    return;
                }
            }

            if ( !IsOwnStorageFormat_Impl( *GetMedium() ) )
                return;

            SfxVersionDialog *pDlg = new SfxVersionDialog( pFrame, NULL );
            pDlg->Execute();
            delete pDlg;
            rReq.Done();
            break;
        }

        case SID_LOAD_LIBRARY:
        case SID_UNLOAD_LIBRARY:
        case SID_REMOVE_LIBRARY:
        case SID_ADD_LIBRARY:
        {
            // Diese Funktionen sind nur f"ur Aufrufe aus dem Basic gedacht
            SfxApplication *pApp = SFX_APP();
            if ( pApp->IsInBasicCall() )
                pApp->BasicLibExec_Impl( rReq, GetBasicManager() );
            break;
        }

        case SID_SAVEDOC:
        {
            //!! detaillierte Auswertung eines Fehlercodes
            SfxObjectShellRef xLock( this );
            SfxErrorContext aEc(ERRCTX_SFX_SAVEDOC,GetTitle());
            SFX_APP()->NotifyEvent(SfxEventHint(SFX_EVENT_SAVEDOC,this));

            BOOL bOk = Save_Impl( rReq.GetArgs() );

            ULONG lErr=GetErrorCode();
            if( !lErr && !bOk )
                lErr=ERRCODE_IO_GENERAL;

            if ( lErr && bOk )
            {
                SFX_REQUEST_ARG( rReq, pWarnItem, SfxBoolItem, SID_FAIL_ON_WARNING, FALSE);
                if ( pWarnItem && pWarnItem->GetValue() )
                    bOk = FALSE;
            }

            if( !ErrorHandler::HandleError( lErr ) )
                SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_SAVEFINISHED, this ) );
            ResetError();

            rReq.SetReturnValue( SfxBoolItem(0, bOk) );
            if ( bOk )
                SFX_APP()->NotifyEvent(SfxEventHint(SFX_EVENT_SAVEDOCDONE,this));
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_UPDATEDOC:
        {
            return;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_DOCINFO:
        {
            SFX_REQUEST_ARG(rReq, pDocInfItem, SfxDocumentInfoItem, SID_DOCINFO, FALSE);

            // keine Parameter vorhanden?
            if ( !pDocInfItem )
            {
                // Dialog ausf"uhren
                SfxDocumentInfo *pOldInfo = new SfxDocumentInfo;
                if ( pImp->pDocInfo )
                    // r/o-flag korrigieren falls es zu frueh gesetzt wurde
                    pImp->pDocInfo->SetReadOnly( IsReadOnly() );
                *pOldInfo = GetDocInfo();
                DocInfoDlg_Impl( GetDocInfo() );

                // ge"andert?
                if( !(*pOldInfo == GetDocInfo()) )
                {
                    // Dokument gilt als ver"andert
                    FlushDocInfo();
                   rReq.Done();
                }
                else
                    rReq.Ignore();

                delete pOldInfo;
            }
            else
            {
                // DocInfo aus Parameter anwenden
                GetDocInfo() = (*pDocInfItem)();
                FlushDocInfo();
            }

            return;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_SAVEASURL:
        case SID_SAVEASDOC:
        {
            //!! detaillierte Auswertung eines Fehlercodes
            SfxObjectShellRef xLock( this );
            SfxErrorContext aEc(ERRCTX_SFX_SAVEASDOC,GetTitle());
            SFX_APP()->NotifyEvent(SfxEventHint(SFX_EVENT_SAVEASDOC,this));

            // Bei Calls "uber StarOne OverWrite-Status checken
            SFX_REQUEST_ARG( rReq, pOverwriteItem, SfxBoolItem, SID_OVERWRITE, FALSE );
            if ( pOverwriteItem )
            {
                // because there is no "exist" function, the overwrite handling is done in the SfxMedium
                SFX_REQUEST_ARG( rReq, pItem, SfxStringItem, SID_FILE_NAME, FALSE );
                if ( !pItem )
                    // In diesem Falle mu\s ein Dateiname mitkommen
                    SetError( ERRCODE_IO_INVALIDPARAMETER );
            }

            BOOL bWasReadonly = IsReadOnly();
            BOOL bOk = SaveAs_Impl(nId == SID_SAVEASURL,  &rReq);
            ULONG lErr=GetErrorCode();
            if ( !lErr && !bOk )
                lErr=ERRCODE_IO_GENERAL;

            if ( lErr && bOk )
            {
                SFX_REQUEST_ARG( rReq, pWarnItem, SfxBoolItem, SID_FAIL_ON_WARNING, FALSE );
                if ( pWarnItem && pWarnItem->GetValue() )
                    bOk = FALSE;
            }

            if ( lErr!=ERRCODE_IO_ABORT )
                ErrorHandler::HandleError(lErr);

            ResetError();
            rReq.SetReturnValue( SfxBoolItem(0, bOk) );
            if ( bOk )
            {
                SFX_APP()->NotifyEvent(SfxEventHint(SFX_EVENT_SAVEASDOCDONE,this));

                // Daten am Medium updaten
                SfxItemSet *pSet = GetMedium()->GetItemSet();
                pSet->ClearItem( SID_POSTSTRING );
                pSet->ClearItem( SID_POSTLOCKBYTES );
                pSet->ClearItem( SID_TEMPLATE );
                pSet->ClearItem( SID_DOC_READONLY );
                pSet->ClearItem( SID_CONTENTTYPE );
                pSet->ClearItem( SID_CHARSET );
                pSet->ClearItem( SID_FILTER_NAME );
                pSet->ClearItem( SID_OPTIONS );
                pSet->ClearItem( SID_FILE_FILTEROPTIONS );
                pSet->ClearItem( SID_VERSION );
                SFX_REQUEST_ARG( rReq, pFilterItem, SfxStringItem, SID_FILTER_NAME, FALSE );
                if ( pFilterItem )
                    pSet->Put( *pFilterItem );
                SFX_REQUEST_ARG( rReq, pOptionsItem, SfxStringItem, SID_OPTIONS, FALSE );
                if ( pOptionsItem )
                    pSet->Put( *pOptionsItem );
                SFX_REQUEST_ARG( rReq, pFilterOptItem, SfxStringItem, SID_FILE_FILTEROPTIONS, FALSE );
                if ( pFilterOptItem )
                    pSet->Put( *pFilterOptItem );
                if ( bWasReadonly )
                    Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );
            }

            Invalidate();
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_CLOSEDOC:
        {
            if ( !rReq.IsAPI() )
            {
                SfxViewFrame *pFrame = GetFrame();
                if ( pFrame && pFrame->GetFrame()->GetParentFrame() )
                {
                    // Wenn SID_CLOSEDOC "uber Menue etc. ausgef"uhrt wird, das
                    // aktuelle Dokument aber in einem Frame liegt, soll eigentlich
                    // das FrameSetDocument geclosed werden
                    pFrame->GetTopViewFrame()->GetObjectShell()->ExecuteSlot( rReq );
                    return;
                }

                BOOL bInFrameSet = FALSE;
                pFrame = SfxViewFrame::GetFirst( this );
                while ( pFrame )
                {
                    if ( pFrame->GetFrame()->GetParentFrame() )
                    {
                        // Auf dieses Dokument existiert noch eine Sicht, die
                        // in einem FrameSet liegt; diese darf nat"urlich nicht
                        // geclosed werden
                        bInFrameSet = TRUE;
                    }

                    pFrame = SfxViewFrame::GetNext( *pFrame, this );
                }

                if ( bInFrameSet )
                {
                    // Alle Sichten, die nicht in einem FrameSet liegen, closen
                    pFrame = SfxViewFrame::GetFirst( this );
                    while ( pFrame )
                    {
                        if ( !pFrame->GetFrame()->GetParentFrame() )
                            pFrame->GetFrame()->DoClose();
                        pFrame = SfxViewFrame::GetNext( *pFrame, this );
                    }
                }
            }

            // Parameter auswerten
            SFX_REQUEST_ARG(rReq, pSaveItem, SfxBoolItem, SID_CLOSEDOC_SAVE, FALSE);
            SFX_REQUEST_ARG(rReq, pNameItem, SfxStringItem, SID_CLOSEDOC_FILENAME, FALSE);
            if ( pSaveItem )
            {
                if ( pSaveItem->GetValue() )
                {
                    if ( !pNameItem )
                    {
                        SbxBase::SetError( SbxERR_WRONG_ARGS );
                        rReq.Ignore();
                        return;
                    }
                    SfxAllItemSet aArgs( GetPool() );
                    SfxStringItem aTmpItem( SID_FILE_NAME, pNameItem->GetValue() );
                    aArgs.Put( aTmpItem, aTmpItem.Which() );
                    SfxRequest aSaveAsReq( SID_SAVEASDOC, SFX_CALLMODE_API, aArgs );
                    ExecFile_Impl( aSaveAsReq );
                    if ( !aSaveAsReq.IsDone() )
                    {
                        rReq.Ignore();
                        return;
                    }
                }
                else
                    SetModified(FALSE);
            }
            else if ( rReq.IsAPI() )
            {
                SbxBase::SetError( SbxERR_WRONG_ARGS );
                rReq.Ignore();
                return;
            }

            // Benutzer bricht ab?
            if ( !PrepareClose( !rReq.IsAPI() ) )
            {
                rReq.SetReturnValue( SfxBoolItem(0, FALSE) );
                rReq.Done();
                return;
            }

            ULONG lErr = GetErrorCode();
            ErrorHandler::HandleError(lErr);

            rReq.SetReturnValue( SfxBoolItem(0, TRUE) );
            rReq.Done();
            rReq.ReleaseArgs(); // da der Pool in Close zerst"ort wird
            if ( rReq.IsAPI() )
                // falls Handler eines Controls dies ruft, sonst GPF nach return
            {
                if( !pImp->pCloser )
                {

                    if ( pPendingCloser )
                        pPendingCloser->ForcePendingCall();
                    pImp->pCloser = new AsynchronLink(
                        Link( 0, SfxObjectShellClose_Impl ) );
                    pImp->pCloser->Call( this );
                    pPendingCloser = pImp->pCloser;
                }
            }
            else
                DoClose();
            return;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_DOCTEMPLATE:
        {
            // speichern als Dokumentvorlagen
            SfxDocumentTemplateDlg *pDlg = 0;
            SfxErrorContext aEc(ERRCTX_SFX_DOCTEMPLATE,GetTitle());
            SfxDocumentTemplates *pTemplates =  new SfxDocumentTemplates;
            pTemplates->Construct();

            if ( !rReq.GetArgs() )
            {
                pDlg = new SfxDocumentTemplateDlg(0, pTemplates);
                if ( RET_OK == pDlg->Execute() && pDlg->GetTemplateName().Len())
                {
                    rReq.AppendItem(SfxStringItem(
                        SID_FILE_NAME,  pTemplates->GetTemplatePath(
                            pDlg->GetRegion(),
                            pDlg->GetTemplateName())));

                    rReq.AppendItem(SfxStringItem(
                        SID_TEMPLATE_NAME, pDlg->GetTemplateName()));
                    rReq.AppendItem(SfxUInt16Item(
                        SID_TEMPLATE_REGION, pDlg->GetRegion()));
                }
                else
                {
                    delete pDlg;
                    rReq.Ignore();
                    return;
                }
            }

            // Region und Name aus Parameter holen
            SFX_REQUEST_ARG(rReq, pRegionItem, SfxStringItem, SID_TEMPLATE_REGIONNAME, FALSE);
            SFX_REQUEST_ARG(rReq, pNameItem, SfxStringItem, SID_TEMPLATE_NAME, FALSE);
            SFX_REQUEST_ARG(rReq, pRegionNrItem, SfxUInt16Item, SID_TEMPLATE_REGION, FALSE);
            if ( (!pRegionItem && !pRegionNrItem ) || !pNameItem )
            {
                DBG_ASSERT( rReq.IsAPI(), "non-API call without Arguments" );
                SbxBase::SetError( SbxERR_WRONG_ARGS );
                rReq.Ignore();
                return;
            }
            String aTemplateName = pNameItem->GetValue();

            // Region-Nr besorgen
            USHORT nRegion;
            if( pRegionItem )
            {
                // Region-Name finden (eigentlich nicht unbedingt eindeutig)
                nRegion = pTemplates->GetRegionNo( pRegionItem->GetValue() );
                if ( nRegion == USHRT_MAX )
                {
                    SbxBase::SetError( ERRCODE_IO_INVALIDPARAMETER );
                    rReq.Ignore();
                    return;
                }
            }
            if ( pRegionNrItem )
                nRegion = pRegionNrItem->GetValue();

            // kein File-Name angegeben?
            if ( SFX_ITEM_SET != rReq.GetArgs()->GetItemState( SID_FILE_NAME ) )
            {
                // TemplatePath nicht angebgeben => aus Region+Name ermitteln
                // Dateiname zusammenbauen lassen
                String aTemplPath = pTemplates->GetTemplatePath(
                    nRegion, aTemplateName );
                rReq.AppendItem(
                    SfxStringItem(SID_FILE_NAME, aTemplPath) );
                rReq.AppendItem( SfxStringItem(SID_FILE_NAME, aTemplPath) );
            }


            // Dateiname
            SFX_REQUEST_ARG(rReq, pFileItem, SfxStringItem, SID_FILE_NAME, FALSE);
            const String aFileName(((const SfxStringItem *)pFileItem)->GetValue());

            // Filter
            const SfxFilter* pFilter;
            const SfxObjectFactory& rFactory = GetFactory();
            USHORT nFilterCount = rFactory.GetFilterCount();
            int n;
            for( n=0; n<nFilterCount; n++)
            {
                pFilter = rFactory.GetFilter( n );
                if( pFilter && pFilter->IsOwnFormat() &&
                    pFilter->IsOwnTemplateFormat() )
                    break;
            }
            DBG_ASSERT( n < nFilterCount && pFilter, "Template Filter nicht gefunden" );
            if( !pFilter || n == nFilterCount )
                pFilter = rFactory.GetFilter(0);

            // Medium zusammenbauen
            SfxItemSet* pSet = new SfxAllItemSet( *rReq.GetArgs() );
            pSet->Put( SfxStringItem( SID_DOCTEMPLATE, aTemplateName ) );
            SfxMedium aMedium( aFileName, STREAM_STD_READWRITE | STREAM_TRUNC, FALSE, pFilter, pSet);

            // als Vorlage speichern
            BOOL bModified = IsModified();
            BOOL bHasTemplateConfig = HasTemplateConfig();
            SetTemplateConfig( FALSE );
            BOOL bOK = FALSE;
            const String aOldURL( INetURLObject::GetBaseURL() );
            if( ShallSetBaseURL_Impl( aMedium ) )
                INetURLObject::SetBaseURL(
                    aMedium.GetURLObject().GetMainURL() );
            else
                INetURLObject::SetBaseURL( String() );

            aMedium.CreateTempFileNoCopy();

            // Because we can't save into a storage directly ( only using tempfile ), we must save the DocInfo first, then
            // we can call SaveTo_Impl and Commit
            if ( pFilter->UsesStorage() )
            {
                SfxDocumentInfo *pInfo = new SfxDocumentInfo;
                pInfo->CopyUserData(GetDocInfo());
                pInfo->SetTitle( aTemplateName );
#if SUPD<613//MUSTINI
                pInfo->SetChanged( SfxStamp(SFX_INIMANAGER()->GetUserFullName()));
#else
                pInfo->SetChanged( SfxStamp(SvtUserOptions().GetFullName()));
#endif
                SvStorageRef aRef = aMedium.GetStorage();
                if ( aRef.Is() )
                {
                    pInfo->SetTime(0L);
                    pInfo->Save(aRef);
                }

                delete pInfo;
            }

            if ( SaveTo_Impl(aMedium) )
            {
                bOK = TRUE;
                aMedium.Commit();
                pTemplates->NewTemplate( nRegion, aTemplateName, aFileName );
            }

            INetURLObject::SetBaseURL( aOldURL );

            DELETEX(pDlg);

            SetError(aMedium.GetErrorCode());
            ULONG lErr=GetErrorCode();
            if(!lErr && !bOK)
                lErr=ERRCODE_IO_GENERAL;
            ErrorHandler::HandleError(lErr);
            ResetError();
            delete pTemplates;
            HACK(warum qualifiziert? SfxObjectShell::) DoSaveCompleted();
            SetTemplateConfig( bHasTemplateConfig );
            SetModified(bModified);
            rReq.SetReturnValue( SfxBoolItem( 0, bOK ) );
            if ( !bOK )
                return;
            break;
        }
    }

    // Picklisten-Eintrag verhindern
    if ( rReq.IsAPI() )
        GetMedium()->SetUpdatePickList( FALSE );
    else if ( rReq.GetArgs() )
    {
        SFX_ITEMSET_GET( *rReq.GetArgs(), pPicklistItem, SfxBoolItem, SID_PICKLIST, FALSE );
        if ( pPicklistItem )
            GetMedium()->SetUpdatePickList( pPicklistItem->GetValue() );
    }

    // Ignore()-Zweige haben schon returnt
    rReq.Done();
}

//--------------------------------------------------------------------

void SfxObjectShell::GetState_Impl(SfxItemSet &rSet)
{
    DBG_CHKTHIS(SfxObjectShell, 0);
    SfxWhichIter aIter( rSet );
    SfxInPlaceObject *pObj=GetInPlaceObject();
    for ( USHORT nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich() )
    {
        switch ( nWhich )
        {
            case SID_DOCTEMPLATE :
            {
                if ( !GetFactory().GetTemplateFilter() )
                    rSet.DisableItem( nWhich );
                break;
            }

            case SID_VERSION:
                {
                    SfxObjectShell *pDoc = this;
                    SfxViewFrame* pFrame = GetFrame();
                    if ( !pFrame )
                        pFrame = SfxViewFrame::GetFirst( this );
                    if ( pFrame  )
                    {
                        if ( pFrame->GetFrame()->GetParentFrame() )
                        {
                            pFrame = pFrame->GetTopViewFrame();
                            pDoc = pFrame->GetObjectShell();
                        }
                    }

                    if ( !pFrame || !pDoc->HasName() ||
                        !IsOwnStorageFormat_Impl( *pDoc->GetMedium() ) ||
                        pDoc->GetMedium()->GetStorage()->GetVersion() < SOFFICE_FILEFORMAT_50 )
                        rSet.DisableItem( nWhich );
                    break;
                }
            case SID_SAVEDOC:
            case SID_UPDATEDOC:
                if (pObj && pObj->GetProtocol().IsEmbed())
                {
                    String aEntry (SfxResId(STR_UPDATEDOC));
                    aEntry += ' ';
                    aEntry += GetInPlaceObject()->GetDocumentName();
                    rSet.Put(SfxStringItem(nWhich, aEntry));
                }
                else
                {
                    BOOL bMediumRO = IsReadOnlyMedium();
                    if ( !bMediumRO && GetMedium() && IsModified() )
                        rSet.Put(SfxStringItem(
                            nWhich, String(SfxResId(STR_SAVEDOC))));
                    else
                        rSet.DisableItem(nWhich);
                }
                break;

            case SID_DOCINFO:
                if ( 0 != ( pImp->eFlags & SFXOBJECTSHELL_NODOCINFO ) )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxDocumentInfoItem( GetTitle(), GetDocInfo() ) );
                break;

            case SID_CLOSEDOC:
            {
                SfxObjectShell *pDoc = this;
                SfxViewFrame *pFrame = GetFrame();
                if ( pFrame && pFrame->GetFrame()->GetParentFrame() )
                {
                    // Wenn SID_CLOSEDOC "uber Menue etc. ausgef"uhrt wird, das
                    // aktuelle Dokument aber in einem Frame liegt, soll eigentlich
                    // das FrameSetDocument geclosed werden
                    pDoc = pFrame->GetTopViewFrame()->GetObjectShell();
                }

                if ( pDoc->GetFlags() & SFXOBJECTSHELL_DONTCLOSE )
                    rSet.DisableItem(nWhich);
                else if ( pObj && pObj->GetProtocol().IsEmbed() )
                {
                    String aEntry (SfxResId(STR_CLOSEDOC_ANDRETURN));
                    aEntry += pObj->GetDocumentName();
                    rSet.Put( SfxStringItem(nWhich, aEntry) );
                }
                else
                    rSet.Put(SfxStringItem(nWhich, String(SfxResId(STR_CLOSEDOC))));
                break;
            }

            case SID_SAVEASDOC:
            {
                if( ( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ) != SFX_LOADED_MAINDOCUMENT )
                {
                    rSet.DisableItem( nWhich );
                    break;
                }

                if ( !GetMedium() )
                    rSet.DisableItem(nWhich);
                else if ( pObj && pObj->GetProtocol().IsEmbed() )
                    rSet.Put( SfxStringItem( nWhich, String( SfxResId( STR_SAVECOPYDOC ) ) ) );
                else
                    rSet.Put( SfxStringItem( nWhich, String( SfxResId( STR_SAVEASDOC ) ) ) );
                break;
            }

            case SID_DOC_MODIFIED:
            {
                rSet.Put( SfxStringItem( SID_DOC_MODIFIED, IsModified() ? '*' : ' ' ) );
                break;
            }

            case SID_MODIFIED:
            {
                rSet.Put( SfxBoolItem( SID_MODIFIED, IsModified() ) );
                break;
            }

            case SID_DOCINFO_TITLE:
            {
                rSet.Put( SfxStringItem(
                    SID_DOCINFO_TITLE, GetDocInfo().GetTitle() ) );
                break;
            }
            case SID_FILE_NAME:
            {
                if( GetMedium() && HasName() )
                    rSet.Put( SfxStringItem(
                        SID_FILE_NAME, GetMedium()->GetName() ) );
                break;
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::ExecProps_Impl(SfxRequest &rReq)
{
    switch ( rReq.GetSlot() )
    {
        case SID_MODIFIED:
        {
            SetModified( ( (SfxBoolItem&) rReq.GetArgs()->Get(SID_MODIFIED)).GetValue() );
            rReq.Done();
            break;
        }

        case SID_DOCTITLE:
            SetTitle( ( (SfxStringItem&) rReq.GetArgs()->Get(SID_DOCTITLE)).GetValue() );
            rReq.Done();
            break;

        case SID_ON_CREATEDOC:
        case SID_ON_OPENDOC:
        case SID_ON_PREPARECLOSEDOC:
        case SID_ON_CLOSEDOC:
        case SID_ON_SAVEDOC:
        case SID_ON_SAVEASDOC:
        case SID_ON_ACTIVATEDOC:
        case SID_ON_DEACTIVATEDOC:
        case SID_ON_PRINTDOC:
        case SID_ON_SAVEDOCDONE:
        case SID_ON_SAVEASDOCDONE:
            SFX_APP()->EventExec_Impl( rReq, this );
            break;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_PLAYMACRO:
        {
            SFX_APP()->PlayMacro_Impl( rReq, GetBasic() );
            break;
        }

        case SID_DOCINFO_AUTHOR :
        {
            String aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            SfxStamp aStamp( GetDocInfo().GetCreated() );
            aStamp.SetName( aStr );
            GetDocInfo().SetCreated( aStamp );
            break;
        }

        case SID_DOCINFO_COMMENTS :
        {
            String aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            GetDocInfo().SetComment( aStr );
            break;
        }

        case SID_DOCINFO_KEYWORDS :
        {
            String aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            GetDocInfo().SetKeywords( aStr );
            break;
        }
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::StateProps_Impl(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    for ( USHORT nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        switch ( nSID )
        {
            case SID_DOCINFO_AUTHOR :
            {
                String aStr = GetDocInfo().GetCreated().GetName();
                rSet.Put( SfxStringItem( nSID, aStr ) );
                break;
            }

            case SID_DOCINFO_COMMENTS :
            {
                String aStr = GetDocInfo().GetComment();
                rSet.Put( SfxStringItem( nSID, aStr ) );
                break;
            }

            case SID_DOCINFO_KEYWORDS :
            {
                String aStr = GetDocInfo().GetKeywords();
                rSet.Put( SfxStringItem( nSID, aStr ) );
                break;
            }

            case SID_DOCPATH:
            {
                DBG_ERROR( "Not supported anymore!" );
                break;
            }

            case SID_DOCFULLNAME:
            {
                rSet.Put( SfxStringItem( SID_DOCFULLNAME, GetTitle(SFX_TITLE_FULLNAME) ) );
                break;
            }

            case SID_DOCTITLE:
            {
                rSet.Put( SfxStringItem( SID_DOCTITLE, GetTitle() ) );
                break;
            }

            case SID_DOC_READONLY:
            {
                rSet.Put( SfxBoolItem( SID_DOC_READONLY, IsReadOnly() ) );
                break;
            }

            case SID_DOC_SAVED:
            {
                rSet.Put( SfxBoolItem( SID_DOC_SAVED, !IsModified() ) );
                break;
            }

            case SID_CLOSING:
            {
                rSet.Put( SfxBoolItem( SID_CLOSING, Get_Impl()->bInCloseEvent ) );
                break;
            }

            case SID_THISDOCUMENT:
            {
                rSet.Put( SfxObjectItem( SID_THISDOCUMENT, this ) );
                break;
            }

            case SID_THISWINDOW:
            {
                rSet.Put( SfxObjectItem( SID_THISWINDOW, SfxViewFrame::GetFirst(this) ) );
                break;
            }

            case SID_ON_CREATEDOC:
            case SID_ON_OPENDOC:
            case SID_ON_PREPARECLOSEDOC:
            case SID_ON_CLOSEDOC:
            case SID_ON_SAVEDOC:
            case SID_ON_SAVEASDOC:
            case SID_ON_ACTIVATEDOC:
            case SID_ON_DEACTIVATEDOC:
            case SID_ON_PRINTDOC:
            case SID_ON_SAVEDOCDONE:
            case SID_ON_SAVEASDOCDONE:
                SFX_APP()->EventState_Impl( nSID, rSet, this );
                break;

            case SID_DOC_LOADING:
                rSet.Put( SfxBoolItem( nSID, SFX_LOADED_MAINDOCUMENT !=
                            ( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ) ) );
                break;

            case SID_IMG_LOADING:
                rSet.Put( SfxBoolItem( nSID, SFX_LOADED_IMAGES !=
                            ( pImp->nLoadedFlags & SFX_LOADED_IMAGES ) ) );
                break;

            case SID_ACTIVEMODULE:
            {
                SfxShell *pSh = GetModule();
                if ( !pSh )
                    pSh = this;
                rSet.Put( SfxObjectItem( SID_ACTIVEMODULE, pSh ) );
                break;
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::ExecView_Impl(SfxRequest &rReq)
{
    switch ( rReq.GetSlot() )
    {
        case SID_ACTIVATE:
        {
            SfxViewFrame *pFrame =
                    SfxViewFrame::GetFirst( this, TYPE(SfxTopViewFrame), TRUE );
            if ( pFrame )
                pFrame->GetFrame()->Appear();
            rReq.SetReturnValue( SfxObjectItem( 0, pFrame ) );
            rReq.Done();
            break;
        }
        case SID_NEWWINDOWFOREDIT:
        {
            SfxViewFrame* pFrame = SfxViewFrame::Current();
            if( pFrame->GetObjectShell() == this &&
                ( pFrame->GetFrameType() & SFXFRAME_HASTITLE ) )
                pFrame->ExecuteSlot( rReq );
            else
            {
                String aFileName( GetObjectShell()->GetMedium()->GetName() );
                if ( aFileName.Len() )
                {
                    SfxStringItem aName( SID_FILE_NAME, aFileName );
                    SfxBoolItem aCreateView( SID_OPEN_NEW_VIEW, TRUE );
                    SFX_APP()->GetAppDispatcher_Impl()->Execute(
                        SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, &aName,
                        &aCreateView, 0L);
                }
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::StateView_Impl(SfxItemSet &rSet)
{
}

