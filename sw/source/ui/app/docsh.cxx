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
#include "precompiled_sw.hxx"

#include <hintids.hxx>
#include <rtl/logfile.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/jobset.hxx>
#include <tools/urlobj.hxx>
#include <svl/whiter.hxx>
#include <svl/zforlist.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/PasswordHelper.hxx>
#include <editeng/adjitem.hxx>
#include <basic/sbx.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/misccfg.hxx>
#include <sfx2/request.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/linkmgr.hxx>
#include <svl/srchitem.hxx>
#include <editeng/flstitem.hxx>
#include <svx/htmlmode.hxx>
#include <svtools/soerr.hxx>
#include <sot/clsids.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <swevent.hxx>
#include <fmtpdsc.hxx>
#include <fmtfsize.hxx>
#include <fmtfld.hxx>
#include <node.hxx>
#include <swwait.hxx>
#include <printdata.hxx>
#include <frmatr.hxx>
#include <view.hxx>         // fuer die aktuelle Sicht
#include <edtwin.hxx>
#include <PostItMgr.hxx>
#include <wrtsh.hxx>        // Verbindung zur Core
#include <docsh.hxx>        // Dokumenterzeugung
#include <basesh.hxx>
#include <viewopt.hxx>
#include <wdocsh.hxx>
#include <swmodule.hxx>
#include <globdoc.hxx>
#include <usrpref.hxx>
#include <shellio.hxx>      // I/O
#include <docstyle.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docstat.hxx>
#include <pagedesc.hxx>
#include <pview.hxx>
#include <mdiexp.hxx>
#include <swbaslnk.hxx>
#include <srcview.hxx>
#include <ndindex.hxx>
#include <ndole.hxx>
#include <swcli.hxx>
#include <txtftn.hxx>
#include <ftnidx.hxx>
#include <fldbas.hxx>
#include <docary.hxx>
#include <swerror.h>        // Fehlermeldungen
#include <helpid.h>
#include <cmdid.h>
#include <globals.hrc>
#include <app.hrc>

#include <cfgid.h>
#include <unotools/moduleoptions.hxx>
#include <unotools/fltrcfg.hxx>
#include <svtools/htmlcfg.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/objface.hxx>
#include <comphelper/storagehelper.hxx>

#define SwDocShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <unomid.h>

#include <sfx2/Metadatable.hxx>
#include <switerator.hxx>

using rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::container;


SFX_IMPL_INTERFACE( SwDocShell, SfxObjectShell, SW_RES(0) )
{
    SFX_CHILDWINDOW_REGISTRATION( SID_HYPERLINK_INSERT );
}

TYPEINIT2(SwDocShell, SfxObjectShell, SfxListener);

//-------------------------------------------------------------------------
SFX_IMPL_OBJECTFACTORY(SwDocShell, SvGlobalName(SO3_SW_CLASSID), SFXOBJECTSHELL_STD_NORMAL|SFXOBJECTSHELL_HASMENU, "swriter"  )

/*--------------------------------------------------------------------
    Beschreibung: Laden vorbereiten
 --------------------------------------------------------------------*/


Reader* SwDocShell::StartConvertFrom(SfxMedium& rMedium, SwReader** ppRdr,
                                    SwCrsrShell *pCrsrShell,
                                    SwPaM* pPaM )
{
    sal_Bool bAPICall = sal_False;
    const SfxPoolItem* pApiItem;
    const SfxItemSet* pMedSet;
    if( 0 != ( pMedSet = rMedium.GetItemSet() ) && SFX_ITEM_SET ==
            pMedSet->GetItemState( FN_API_CALL, sal_True, &pApiItem ) )
            bAPICall = ((const SfxBoolItem*)pApiItem)->GetValue();

    const SfxFilter* pFlt = rMedium.GetFilter();
    if( !pFlt )
    {
        if(!bAPICall)
        {
            InfoBox( 0, SW_RESSTR(STR_CANTOPEN)).Execute();
        }
        return 0;
    }
    String aFileName( rMedium.GetName() );
    SwRead pRead = SwReaderWriter::GetReader( pFlt->GetUserData() );
    if( !pRead )
        return 0;

    if( rMedium.IsStorage()
        ? SW_STORAGE_READER & pRead->GetReaderType()
        : SW_STREAM_READER & pRead->GetReaderType() )
    {
        *ppRdr = pPaM ? new SwReader( rMedium, aFileName, *pPaM ) :
            pCrsrShell ?
                new SwReader( rMedium, aFileName, *pCrsrShell->GetCrsr() )
                    : new SwReader( rMedium, aFileName, pDoc );
    }
    else
        return 0;

    // PassWord Checken
    String aPasswd;
    if ((*ppRdr)->NeedsPasswd( *pRead ))
    {
        if(!bAPICall)
        {
            SfxPasswordDialog* pPasswdDlg =
                    new SfxPasswordDialog( 0 );
                if(RET_OK == pPasswdDlg->Execute())
                    aPasswd = pPasswdDlg->GetPassword();
        }
        else
        {
            const SfxItemSet* pSet = rMedium.GetItemSet();
            const SfxPoolItem *pPassItem;
            if(pSet && SFX_ITEM_SET == pSet->GetItemState(SID_PASSWORD, sal_True, &pPassItem))
                aPasswd = ((const SfxStringItem *)pPassItem)->GetValue();
        }

        if (!(*ppRdr)->CheckPasswd( aPasswd, *pRead ))
        {
            InfoBox( 0, SW_RES(MSG_ERROR_PASSWD)).Execute();
                delete *ppRdr;
            return 0;
        }
    }

    // #i30171# set the UpdateDocMode at the SwDocShell
    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
    nUpdateDocMode = pUpdateDocItem ? pUpdateDocItem->GetValue() : document::UpdateDocMode::NO_UPDATE;

    if( pFlt->GetDefaultTemplate().Len() )
        pRead->SetTemplateName( pFlt->GetDefaultTemplate() );

    if( pRead == ReadAscii && 0 != rMedium.GetInStream() &&
        pFlt->GetUserData().EqualsAscii( FILTER_TEXT_DLG ) )
    {
        SwAsciiOptions aOpt;
        const SfxItemSet* pSet;
        const SfxPoolItem* pItem;
        if( 0 != ( pSet = rMedium.GetItemSet() ) && SFX_ITEM_SET ==
            pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
            aOpt.ReadUserData( ((const SfxStringItem*)pItem)->GetValue() );

        if( pRead )
            pRead->GetReaderOpt().SetASCIIOpts( aOpt );
    }

    return pRead;
}

/*--------------------------------------------------------------------
    Beschreibung: Laden
 --------------------------------------------------------------------*/

sal_Bool SwDocShell::ConvertFrom( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::ConvertFrom" );

    SwReader* pRdr;
    SwRead pRead = StartConvertFrom(rMedium, &pRdr);
    if (!pRead)
      return sal_False; // #129881# return if no reader is found
    SotStorageRef pStg=pRead->getSotStorageRef(); // #i45333# save sot storage ref in case of recursive calls

    SwWait aWait( *this, sal_True );

        // SfxProgress unterdruecken, wenn man Embedded ist
    SW_MOD()->SetEmbeddedLoadSave(
                            SFX_CREATE_MODE_EMBEDDED == GetCreateMode() );

    pRdr->GetDoc()->set(IDocumentSettingAccess::HTML_MODE, ISA(SwWebDocShell));

    /* #106748# Restore the pool default if reading a saved document. */
    pDoc->RemoveAllFmtLanguageDependencies();

    sal_uLong nErr = pRdr->Read( *pRead );

    // Evtl. ein altes Doc weg
    if ( pDoc != pRdr->GetDoc() )
    {
        if( pDoc )
            RemoveLink();
        pDoc = pRdr->GetDoc();

        AddLink();

        if ( !mxBasePool.is() )
            mxBasePool = new SwDocStyleSheetPool( *pDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
    }

    UpdateFontList();
    InitDraw();

    delete pRdr;

    SW_MOD()->SetEmbeddedLoadSave( sal_False );

    SetError( nErr, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
    sal_Bool bOk = !IsError( nErr );

    // --> OD 2006-11-07 #i59688#
//    // StartFinishedLoading rufen. Nicht bei asynchronen Filtern!
//    // Diese muessen das selbst rufen!
//    if( bOk && !pDoc->IsInLoadAsynchron() )
//        StartLoadFinishedTimer();
    if ( bOk && !pDoc->IsInLoadAsynchron() )
    {
        LoadingFinished();
    }
    // <--

    pRead->setSotStorageRef(pStg); // #i45333# save sot storage ref in case of recursive calls

    return bOk;
}

/*--------------------------------------------------------------------
    Beschreibung: Sichern des Default-Formats, Stg vorhanden
 --------------------------------------------------------------------*/


sal_Bool SwDocShell::Save()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::Save" );
    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    if(pView)
        pView->GetEditWin().StopQuickHelp();
    SwWait aWait( *this, sal_True );

    CalcLayoutForOLEObjects();  // format for OLE objets
    // --> OD 2006-03-17 #i62875#
    // reset compatibility flag <DoNotCaptureDrawObjsOnPage>, if possible
    if ( pWrtShell && pDoc &&
         pDoc->get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
         docfunc::AllDrawObjsOnPage( *pDoc ) )
    {
        pDoc->set(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
    }
    // <--

    sal_uLong nErr = ERR_SWG_WRITE_ERROR, nVBWarning = ERRCODE_NONE;
    if( SfxObjectShell::Save() )
    {
        switch( GetCreateMode() )
        {
        case SFX_CREATE_MODE_INTERNAL:
            nErr = 0;
            break;

        case SFX_CREATE_MODE_ORGANIZER:
            {
                WriterRef xWrt;
                ::GetXMLWriter( aEmptyStr, GetMedium()->GetBaseURL( true ), xWrt );
                xWrt->SetOrganizerMode( sal_True );
                SwWriter aWrt( *GetMedium(), *pDoc );
                nErr = aWrt.Write( xWrt );
                xWrt->SetOrganizerMode( sal_False );
            }
            break;

        case SFX_CREATE_MODE_EMBEDDED:
            // SfxProgress unterdruecken, wenn man Embedded ist
            SW_MOD()->SetEmbeddedLoadSave( sal_True );
            // kein break;

        case SFX_CREATE_MODE_STANDARD:
        case SFX_CREATE_MODE_PREVIEW:
        default:
            {
                if( pDoc->ContainsMSVBasic() )
                {
                    //TODO/MBA: it looks as that this code can be removed!
                    //SvxImportMSVBasic aTmp( *this, pIo->GetStorage() );
                    //aTmp.SaveOrDelMSVBAStorage( sal_False, aEmptyStr );
                    if( SvtFilterOptions::Get()->IsLoadWordBasicStorage() )
                        nVBWarning = GetSaveWarningOfMSVBAStorage( (SfxObjectShell&) (*this) );
                    pDoc->SetContainsMSVBasic( sal_False );
                }

                // TabellenBox Edit beenden!
                if( pWrtShell )
                    pWrtShell->EndAllTblBoxEdit();

                WriterRef xWrt;
                ::GetXMLWriter( aEmptyStr, GetMedium()->GetBaseURL( true ), xWrt );

                sal_Bool bLockedView(sal_False);
                if ( pWrtShell )
                {
                    bLockedView = pWrtShell->IsViewLocked();
                    pWrtShell->LockView( sal_True );    //lock visible section
                }

                SwWriter aWrt( *GetMedium(), *pDoc );
                nErr = aWrt.Write( xWrt );

                if ( pWrtShell )
                    pWrtShell->LockView( bLockedView );
            }
            break;
        }
        SW_MOD()->SetEmbeddedLoadSave( sal_False );
    }
    SetError( nErr ? nErr : nVBWarning, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

    SfxViewFrame* pFrm = pWrtShell ? pWrtShell->GetView().GetViewFrame() : 0;
    if( pFrm )
    {
        pFrm->GetBindings().SetState( SfxStringItem( SID_DOC_MODIFIED, ' ' ));
    }
    return !IsError( nErr );
}

/*--------------------------------------------------------------------
    Beschreibung: Sichern im Defaultformat
 --------------------------------------------------------------------*/


sal_Bool SwDocShell::SaveAs( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::SaveAs" );

    SwWait aWait( *this, sal_True );
    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    if(pView)
        pView->GetEditWin().StopQuickHelp();

    //#i91811# mod if we have an active margin window, write back the text
    if ( pView &&
         pView->GetPostItMgr() &&
         pView->GetPostItMgr()->HasActiveSidebarWin() )
    {
        pView->GetPostItMgr()->UpdateDataOnActiveSidebarWin();
    }

    if( pDoc->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) &&
        !pDoc->get(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS) )
        RemoveOLEObjects();

    {
        // Task 75666 - is the Document imported by our Microsoft-Filters?
        const SfxFilter* pOldFilter = GetMedium()->GetFilter();
        if( pOldFilter &&
            ( pOldFilter->GetUserData().EqualsAscii( FILTER_WW8 ) ||
              pOldFilter->GetUserData().EqualsAscii( "CWW6" ) ||
              pOldFilter->GetUserData().EqualsAscii( "WW6" ) ||
              pOldFilter->GetUserData().EqualsAscii( "WW1" ) ))
        {
            // when saving it in our own fileformat, then remove the template
            // name from the docinfo.
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                GetModel(), uno::UNO_QUERY_THROW);
            uno::Reference<document::XDocumentProperties> xDocProps
                = xDPS->getDocumentProperties();
            xDocProps->setTemplateName(::rtl::OUString::createFromAscii(""));
            xDocProps->setTemplateURL(::rtl::OUString::createFromAscii(""));
            xDocProps->setTemplateDate(::util::DateTime());
        }
    }

    CalcLayoutForOLEObjects();  // format for OLE objets
    // --> OD 2006-03-17 #i62875#
    // reset compatibility flag <DoNotCaptureDrawObjsOnPage>, if possible
    if ( pWrtShell && pDoc &&
         pDoc->get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
         docfunc::AllDrawObjsOnPage( *pDoc ) )
    {
        pDoc->set(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
    }
    // <--

    sal_uLong nErr = ERR_SWG_WRITE_ERROR, nVBWarning = ERRCODE_NONE;
    uno::Reference < embed::XStorage > xStor = rMedium.GetOutputStorage();
    if( SfxObjectShell::SaveAs( rMedium ) )
    {
        if( GetDoc()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) && !ISA( SwGlobalDocShell ) )
        {
            // This is to set the correct class id if SaveAs is
            // called from SwDoc::SplitDoc to save a normal doc as
            // global doc. In this case, SaveAs is called at a
            // normal doc shell, therefore, SfxInplaceObject::SaveAs
            // will set the wrong class id.
            SvGlobalName aClassName;
            String aAppName, aLongUserName, aUserName;

            // The document is closed explicitly, but using SfxObjectShellLock is still more correct here
            SfxObjectShellLock xDocSh =
                new SwGlobalDocShell( SFX_CREATE_MODE_INTERNAL );
            // the global document can not be a template
            xDocSh->SetupStorage( xStor, SotStorage::GetVersion( xStor ), sal_False );
            xDocSh->DoClose();
        }

        if( pDoc->ContainsMSVBasic() )
        {
            //TODO/MBA: it looks as that this code can be removed!
            //SvxImportMSVBasic aTmp( *this, pIo->GetStorage() );
            //aTmp.SaveOrDelMSVBAStorage( sal_False, aEmptyStr );
            if( SvtFilterOptions::Get()->IsLoadWordBasicStorage() )
                nVBWarning = GetSaveWarningOfMSVBAStorage( (SfxObjectShell&) *this );
            pDoc->SetContainsMSVBasic( sal_False );
        }

        // TabellenBox Edit beenden!
        if( pWrtShell )
            pWrtShell->EndAllTblBoxEdit();

        // Modified-Flag merken und erhalten ohne den Link zu Callen
        // (fuer OLE; nach Anweisung von MM)
        sal_Bool bIsModified = pDoc->IsModified();
        pDoc->GetIDocumentUndoRedo().LockUndoNoModifiedPosition();
        Link aOldOLELnk( pDoc->GetOle2Link() );
        pDoc->SetOle2Link( Link() );

            // SfxProgress unterdruecken, wenn man Embedded ist
        SW_MOD()->SetEmbeddedLoadSave(
                            SFX_CREATE_MODE_EMBEDDED == GetCreateMode() );

        WriterRef xWrt;
        ::GetXMLWriter( aEmptyStr, rMedium.GetBaseURL( true ), xWrt );

        sal_Bool bLockedView(sal_False);
        if ( pWrtShell )
        {
            bLockedView = pWrtShell->IsViewLocked();
            pWrtShell->LockView( sal_True );    //lock visible section
        }

        SwWriter aWrt( rMedium, *pDoc );
        nErr = aWrt.Write( xWrt );

        if ( pWrtShell )
            pWrtShell->LockView( bLockedView );

        if( bIsModified )
        {
            pDoc->SetModified();
            pDoc->GetIDocumentUndoRedo().UnLockUndoNoModifiedPosition();
        }
        pDoc->SetOle2Link( aOldOLELnk );

        SW_MOD()->SetEmbeddedLoadSave( sal_False );
    }
    SetError( nErr ? nErr : nVBWarning, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

    return !IsError( nErr );
}

/*--------------------------------------------------------------------
    Beschreibung: Sichern aller Formate
 --------------------------------------------------------------------*/
SwSrcView* lcl_GetSourceView( SwDocShell* pSh )
{
    // sind wir in der SourceView?
    SfxViewFrame* pVFrame = SfxViewFrame::GetFirst( pSh );
    SfxViewShell* pViewShell = pVFrame ? pVFrame->GetViewShell() : 0;
    return PTR_CAST( SwSrcView, pViewShell);
}

sal_Bool SwDocShell::ConvertTo( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::ConvertTo" );
    const SfxFilter* pFlt = rMedium.GetFilter();
    if( !pFlt )
        return sal_False;

    WriterRef xWriter;
    SwReaderWriter::GetWriter( pFlt->GetUserData(), rMedium.GetBaseURL( true ), xWriter );
    if( !xWriter.Is() )
    {   // Der Filter ist nicht vorhanden
        InfoBox( 0,
                 SW_RESSTR(STR_DLLNOTFOUND) ).Execute();
        return sal_False;
    }

    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    if(pView)
        pView->GetEditWin().StopQuickHelp();

    //#i91811# mod if we have an active margin window, write back the text
    if ( pView &&
         pView->GetPostItMgr() &&
         pView->GetPostItMgr()->HasActiveSidebarWin() )
    {
        pView->GetPostItMgr()->UpdateDataOnActiveSidebarWin();
    }

    sal_uLong nVBWarning = 0;

    if( pDoc->ContainsMSVBasic() )
    {
        sal_Bool bSave = pFlt->GetUserData().EqualsAscii( "CWW8" )
             && SvtFilterOptions::Get()->IsLoadWordBasicStorage();

        if ( bSave )
        {
            SvStorageRef xStg = new SotStorage( rMedium.GetOutStream(), sal_False );
            DBG_ASSERT( !xStg->GetError(), "No storage available for storing VBA macros!" );
            if ( !xStg->GetError() )
            {
                nVBWarning = SaveOrDelMSVBAStorage( (SfxObjectShell&) *this, *xStg, bSave, String::CreateFromAscii("Macros") );
                xStg->Commit();
                pDoc->SetContainsMSVBasic( sal_True );
            }
        }
    }

    // TabellenBox Edit beenden!
    if( pWrtShell )
        pWrtShell->EndAllTblBoxEdit();

    if( pFlt->GetUserData().EqualsAscii( "HTML") )
    {
        SvxHtmlOptions* pHtmlOpt = SvxHtmlOptions::Get();
        if( !pHtmlOpt->IsStarBasic() && pHtmlOpt->IsStarBasicWarning() && HasBasic() )
        {
            uno::Reference< XLibraryContainer > xLibCont(GetBasicContainer(), UNO_QUERY);
            uno::Reference< XNameAccess > xLib;
            Sequence<rtl::OUString> aNames = xLibCont->getElementNames();
            const rtl::OUString* pNames = aNames.getConstArray();
            for(sal_Int32 nLib = 0; nLib < aNames.getLength(); nLib++)
            {
                Any aLib = xLibCont->getByName(pNames[nLib]);
                aLib >>= xLib;
                if(xLib.is())
                {
                    Sequence<rtl::OUString> aModNames = xLib->getElementNames();
                    if(aModNames.getLength())
                    {
                        SetError(WARN_SWG_HTML_NO_MACROS, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
                        break;
                    }
                }
            }
        }
        UpdateDocInfoForSave();
    }

    // --> FME 2007-5-7 #i76360# Update document statistics
    SwDocStat aDocStat( pDoc->GetDocStat() );;
    pDoc->UpdateDocStat( aDocStat );
    // <--
    CalcLayoutForOLEObjects();  // format for OLE objets
    // --> OD 2006-03-17 #i62875#
    // reset compatibility flag <DoNotCaptureDrawObjsOnPage>, if possible
    if ( pWrtShell && pDoc &&
         pDoc->get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
         docfunc::AllDrawObjsOnPage( *pDoc ) )
    {
        pDoc->set(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
    }
    // <--

    if( xWriter->IsStgWriter() &&
        ( /*xWriter->IsSw3Writer() ||*/
          pFlt->GetUserData().EqualsAscii( FILTER_XML ) ||
           pFlt->GetUserData().EqualsAscii( FILTER_XMLV ) ||
           pFlt->GetUserData().EqualsAscii( FILTER_XMLVW ) ) )
    {
        // eigenen Typ ermitteln
        sal_uInt8 nMyType = 0;
        if( ISA( SwWebDocShell) )
            nMyType = 1;
        else if( ISA( SwGlobalDocShell) )
            nMyType = 2;

        // gewuenschten Typ ermitteln
        sal_uInt8 nSaveType = 0;
        sal_uLong nSaveClipId = pFlt->GetFormat();
        if( SOT_FORMATSTR_ID_STARWRITERWEB_8 == nSaveClipId ||
            SOT_FORMATSTR_ID_STARWRITERWEB_60 == nSaveClipId ||
            SOT_FORMATSTR_ID_STARWRITERWEB_50 == nSaveClipId ||
            SOT_FORMATSTR_ID_STARWRITERWEB_40 == nSaveClipId )
            nSaveType = 1;
        else if( SOT_FORMATSTR_ID_STARWRITERGLOB_8 == nSaveClipId ||
                 SOT_FORMATSTR_ID_STARWRITERGLOB_60 == nSaveClipId ||
                 SOT_FORMATSTR_ID_STARWRITERGLOB_50 == nSaveClipId ||
                 SOT_FORMATSTR_ID_STARWRITERGLOB_40 == nSaveClipId )
            nSaveType = 2;

        // Flags am Dokument entsprechend umsetzen
        sal_Bool bIsHTMLModeSave = GetDoc()->get(IDocumentSettingAccess::HTML_MODE);
        sal_Bool bIsGlobalDocSave = GetDoc()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT);
        sal_Bool bIsGlblDocSaveLinksSave = GetDoc()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS);
        if( nMyType != nSaveType )
        {
            GetDoc()->set(IDocumentSettingAccess::HTML_MODE, 1 == nSaveType);
            GetDoc()->set(IDocumentSettingAccess::GLOBAL_DOCUMENT, 2 == nSaveType);
            if( 2 != nSaveType )
                GetDoc()->set(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS, false);
        }

        // if the target format is storage based, then the output storage must be already created
        if ( rMedium.IsStorage() )
        {
            // set MediaType on target storage
            // (MediaType will be queried during SaveAs)
            try
            {
                // TODO/MBA: testing
                uno::Reference < beans::XPropertySet > xSet( rMedium.GetStorage(), uno::UNO_QUERY );
                if ( xSet.is() )
                    xSet->setPropertyValue( ::rtl::OUString::createFromAscii("MediaType"), uno::makeAny( ::rtl::OUString( SotExchange::GetFormatMimeType( nSaveClipId ) ) ) );
            }
            catch ( uno::Exception& )
            {
            }
        }

        // Jetzt das Dokument normal speichern
        sal_Bool bRet = SaveAs( rMedium );

        if( nMyType != nSaveType )
        {
            GetDoc()->set(IDocumentSettingAccess::HTML_MODE, bIsHTMLModeSave );
            GetDoc()->set(IDocumentSettingAccess::GLOBAL_DOCUMENT, bIsGlobalDocSave);
            GetDoc()->set(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS, bIsGlblDocSaveLinksSave);
        }

        return bRet;
    }

    if( pFlt->GetUserData().EqualsAscii( FILTER_TEXT_DLG ) &&
        ( pWrtShell || !::lcl_GetSourceView( this ) ))
    {
        SwAsciiOptions aOpt;
        String sItemOpt;
        const SfxItemSet* pSet;
        const SfxPoolItem* pItem;
        if( 0 != ( pSet = rMedium.GetItemSet() ) )
        {
            if( SFX_ITEM_SET == pSet->GetItemState( SID_FILE_FILTEROPTIONS,
                                                    sal_True, &pItem ) )
                sItemOpt = ((const SfxStringItem*)pItem)->GetValue();
        }
        if(sItemOpt.Len())
            aOpt.ReadUserData( sItemOpt );

        xWriter->SetAsciiOptions( aOpt );
    }

        // SfxProgress unterdruecken, wenn man Embedded ist
    SW_MOD()->SetEmbeddedLoadSave(
                            SFX_CREATE_MODE_EMBEDDED == GetCreateMode());

    // Kontext aufspannen, um die Anzeige der Selektion zu unterbinden
    sal_uLong nErrno;
    String aFileName( rMedium.GetName() );

    //Keine View also das ganze Dokument!
    if ( pWrtShell )
    {
        SwWait aWait( *this, sal_True );
        // --> OD 2009-12-31 #i106906#
        const sal_Bool bFormerLockView = pWrtShell->IsViewLocked();
        pWrtShell->LockView( sal_True );
        // <--
        pWrtShell->StartAllAction();
        pWrtShell->Push();
        SwWriter aWrt( rMedium, *pWrtShell, sal_True );
        nErrno = aWrt.Write( xWriter, &aFileName );
        //JP 16.05.97: falls der SFX uns die View waehrend des speicherns
        //              entzieht
        if( pWrtShell )
        {
            pWrtShell->Pop(sal_False);
            pWrtShell->EndAllAction();
            // --> OD 2009-12-31 #i106906#
            pWrtShell->LockView( bFormerLockView );
            // <--
        }
    }
    else
    {
        // sind wir in der SourceView?
        SwSrcView* pSrcView = ::lcl_GetSourceView( this );
        if( pSrcView )
        {
            pSrcView->SaveContentTo(rMedium);
            nErrno = 0;
        }
        else
        {
            SwWriter aWrt( rMedium, *pDoc );
            nErrno = aWrt.Write( xWriter, &aFileName );
        }
    }

    SW_MOD()->SetEmbeddedLoadSave( sal_False );
    SetError( nErrno ? nErrno : nVBWarning, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
    if( !rMedium.IsStorage() )
        rMedium.CloseOutStream();

    return !IsError( nErrno );
}

/*--------------------------------------------------------------------
    Beschreibung:   Haende weg
 --------------------------------------------------------------------*/


/*--------------------------------------------------------------------
    Beschreibung: ??? noch nicht zu aktivieren, muss sal_True liefern
 --------------------------------------------------------------------*/


sal_Bool SwDocShell::SaveCompleted( const uno::Reference < embed::XStorage >& xStor  )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::SaveCompleted" );
    sal_Bool bRet = SfxObjectShell::SaveCompleted( xStor );
    if( bRet )
    {
        // erst hier entscheiden, ob das Speichern geklappt hat oder nicht
        if( IsModified() )
            pDoc->SetModified();
        else
            pDoc->ResetModified();
    }

    if( pOLEChildList )
    {
        sal_Bool bResetModified = IsEnableSetModified();
        if( bResetModified )
            EnableSetModified( sal_False );

        uno::Sequence < rtl::OUString > aNames = pOLEChildList->GetObjectNames();
        for( sal_Int32 n = aNames.getLength(); n; n-- )
        {
            if ( !pOLEChildList->MoveEmbeddedObject( aNames[n-1], GetEmbeddedObjectContainer() ) )
            {
                DBG_ERROR( "Copying of objects didn't work!" );
            }

            //SvPersist* pPersist = this;
            //SvInfoObjectRef aRef( pInfList->GetObject( --n ));
            //pPersist->Move( &aRef, aRef->GetStorageName() );
        }

        DELETEZ( pOLEChildList );
        if( bResetModified )
            EnableSetModified( sal_True );
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Draw()-Overload fuer OLE2 (Sfx)
 --------------------------------------------------------------------*/

void SwDocShell::Draw( OutputDevice* pDev, const JobSetup& rSetup,
                               sal_uInt16 nAspect )
{
    //fix #25341# Draw sollte das Modified nicht beeinflussen
    sal_Bool bResetModified;
    if ( sal_True == (bResetModified = IsEnableSetModified()) )
        EnableSetModified( sal_False );

    //sollte am Document ein JobSetup haengen, dann kopieren wir uns diesen,
    //um nach dem PrtOle2 diesen wieder am Doc zu verankern.
    //Einen leeren JobSetup setzen wir nicht ein, denn der wuerde nur zu
    //fragwuerdigem Ergebnis nach teurer Neuformatierung fuehren (Preview!)
    JobSetup *pOrig = 0;
    if ( rSetup.GetPrinterName().Len() && ASPECT_THUMBNAIL != nAspect )
    {
        pOrig = const_cast<JobSetup*>(pDoc->getJobsetup());
        if( pOrig )         // dann kopieren wir uns den
            pOrig = new JobSetup( *pOrig );
        pDoc->setJobsetup( rSetup );
    }

    Rectangle aRect( nAspect == ASPECT_THUMBNAIL ?
            GetVisArea( nAspect ) : GetVisArea( ASPECT_CONTENT ) );

    pDev->Push();
    pDev->SetFillColor();
    pDev->SetLineColor();
    pDev->SetBackground();
    sal_Bool bWeb = 0 != PTR_CAST(SwWebDocShell, this);
    SwPrintData aOpts;
    ViewShell::PrtOle2( pDoc, SW_MOD()->GetUsrPref(bWeb), aOpts, pDev, aRect );
    pDev->Pop();

    if( pOrig )
    {
        pDoc->setJobsetup( *pOrig );
        delete pOrig;
    }
    if ( bResetModified )
        EnableSetModified( sal_True );
}


void SwDocShell::SetVisArea( const Rectangle &rRect )
{
    Rectangle aRect( rRect );
    if ( pView )
    {
        Size aSz( pView->GetDocSz() );
        aSz.Width() += DOCUMENTBORDER; aSz.Height() += DOCUMENTBORDER;
        long nMoveX = 0, nMoveY = 0;
        if ( aRect.Right() > aSz.Width() )
            nMoveX = aSz.Width() - aRect.Right();
        if ( aRect.Bottom() > aSz.Height() )
            nMoveY = aSz.Height() - aRect.Bottom();
        aRect.Move( nMoveX, nMoveY );
        nMoveX = aRect.Left() < 0 ? -aRect.Left() : 0;
        nMoveY = aRect.Top()  < 0 ? -aRect.Top()  : 0;
        aRect.Move( nMoveX, nMoveY );

        //Ruft das SfxInPlaceObject::SetVisArea()!
        pView->SetVisArea( aRect, sal_True );
    }
    else
        SfxObjectShell::SetVisArea( aRect );
}


Rectangle SwDocShell::GetVisArea( sal_uInt16 nAspect ) const
{
    if ( nAspect == ASPECT_THUMBNAIL )
    {
        //PreView: VisArea auf die erste Seite einstellen.
        SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), 1 );
        SwCntntNode* pNd = pDoc->GetNodes().GoNext( &aIdx );

        const SwRect aPageRect = pNd->FindPageFrmRect( sal_False, 0, sal_False );
        return aPageRect.SVRect();
<<<<<<< local
=======

        // Why does this have to be that complicated? I replaced this by the
        // call of FindPageFrmRect():
        /*
        //PageDesc besorgen, vom ersten Absatz oder den default.
        const SwFmtPageDesc &rDesc = pNd->GetSwAttrSet().GetPageDesc();
        const SwPageDesc* pDesc = rDesc.GetPageDesc();
        if( !pDesc )
            pDesc = &const_cast<const SwDoc *>(pDoc)->GetPageDesc( 0 );

        //Das Format wird evtl. von der virtuellen Seitennummer bestimmt.
        const sal_uInt16 nPgNum = rDesc.GetNumOffset();
        const sal_Bool bOdd = nPgNum % 2 ? sal_True : sal_False;
        const SwFrmFmt *pFmt = bOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt();
        if ( !pFmt ) //#40568#
            pFmt = bOdd ? pDesc->GetLeftFmt() : pDesc->GetRightFmt();

        if ( pFmt->GetFrmSize().GetWidth() == LONG_MAX )
            //Jetzt wird es aber Zeit fuer die Initialisierung
            pDoc->getPrinter( true );

        const SwFmtFrmSize& rFrmSz = pFmt->GetFrmSize();
        const Size aSz( rFrmSz.GetWidth(), rFrmSz.GetHeight() );
        const Point aPt( DOCUMENTBORDER, DOCUMENTBORDER );
        const Rectangle aRect( aPt, aSz );
        return aRect;*/
>>>>>>> other
    }
    return SfxObjectShell::GetVisArea( nAspect );
}

Printer *SwDocShell::GetDocumentPrinter()
{
    return pDoc->getPrinter( false );
}

OutputDevice* SwDocShell::GetDocumentRefDev()
{
    return pDoc->getReferenceDevice( false );
}

void SwDocShell::OnDocumentPrinterChanged( Printer * pNewPrinter )
{
    if ( pNewPrinter )
        GetDoc()->setJobsetup( pNewPrinter->GetJobSetup() );
    else
        GetDoc()->setPrinter( 0, true, true );
}

sal_uLong SwDocShell::GetMiscStatus() const
{
    return SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE;
}

// --> FME 2004-08-05 #i20883# Digital Signatures and Encryption
sal_uInt16 SwDocShell::GetHiddenInformationState( sal_uInt16 nStates )
{
    // get global state like HIDDENINFORMATION_DOCUMENTVERSIONS
    sal_uInt16 nState = SfxObjectShell::GetHiddenInformationState( nStates );

    if ( nStates & HIDDENINFORMATION_RECORDEDCHANGES )
    {
        if ( GetDoc()->GetRedlineTbl().Count() )
            nState |= HIDDENINFORMATION_RECORDEDCHANGES;
    }
    if ( nStates & HIDDENINFORMATION_NOTES )
    {
        ASSERT( GetWrtShell(), "No SwWrtShell, no information" )
        if ( GetWrtShell() )
        {
            SwFieldType* pType = GetWrtShell()->GetFldType( RES_POSTITFLD, aEmptyStr );
            SwIterator<SwFmtFld,SwFieldType> aIter( *pType );
            SwFmtFld* pFirst = aIter.First();
            while( pFirst )
            {
                if( pFirst->GetTxtFld() && pFirst->IsFldInDoc() )
                {
                    nState |= HIDDENINFORMATION_NOTES;
                    break;
                }
                pFirst = aIter.Next();
            }
        }
    }

    return nState;
}
// <--


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwDocShell::GetState(SfxItemSet& rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16  nWhich  = aIter.FirstWhich();

    while (nWhich)
    {
        switch (nWhich)
        {
        case SID_PRINTPREVIEW:
        {
<<<<<<< local
            BOOL bDisable = IsInPlaceActive();
            // Disable "multiple layout"
=======
            sal_Bool bDisable = IsInPlaceActive();
>>>>>>> other
            if ( !bDisable )
            {
                SfxViewFrame *pTmpFrm = SfxViewFrame::GetFirst(this);
                while (pTmpFrm)     // Preview suchen
                {
                    if ( PTR_CAST(SwView, pTmpFrm->GetViewShell()) &&
                         ((SwView*)pTmpFrm->GetViewShell())->GetWrtShell().GetViewOptions()->getBrowseMode() )
                    {
                        bDisable = sal_True;
                        break;
                    }
                    pTmpFrm = pTmpFrm->GetNext(*pTmpFrm, this);
                }
            }
            // End of disabled "multiple layout"
            if ( bDisable )
                rSet.DisableItem( SID_PRINTPREVIEW );
            else
            {
                SfxBoolItem aBool( SID_PRINTPREVIEW, sal_False );
                if( PTR_CAST( SwPagePreView, SfxViewShell::Current()) )
                    aBool.SetValue( sal_True );
                rSet.Put( aBool );
            }
        }
        break;
        case SID_SOURCEVIEW:
        {
            SfxViewShell* pCurrView = GetView() ? (SfxViewShell*)GetView()
                                        : SfxViewShell::Current();
            sal_Bool bSourceView = 0 != PTR_CAST(SwSrcView, pCurrView);
            rSet.Put(SfxBoolItem(SID_SOURCEVIEW, bSourceView));
        }
        break;
        case SID_HTML_MODE:
            rSet.Put(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(this)));
        break;

        case FN_ABSTRACT_STARIMPRESS:
        case FN_OUTLINE_TO_IMPRESS:
            {
                SvtModuleOptions aMOpt;
                if ( !aMOpt.IsImpress() )
                    rSet.DisableItem( nWhich );
            }
            /* no break here */
        case FN_ABSTRACT_NEWDOC:
        case FN_OUTLINE_TO_CLIPBOARD:
            {
                if ( !GetDoc()->GetNodes().GetOutLineNds().Count() )
                    rSet.DisableItem( nWhich );
            }
            break;
        case SID_BROWSER_MODE:
        case FN_PRINT_LAYOUT:
            {
                sal_Bool bState = GetDoc()->get(IDocumentSettingAccess::BROWSE_MODE);
                if(FN_PRINT_LAYOUT == nWhich)
                    bState = !bState;
                rSet.Put( SfxBoolItem( nWhich, bState));
            }
            break;

        case FN_NEW_GLOBAL_DOC:
            if ( ISA(SwGlobalDocShell) )
                rSet.DisableItem( nWhich );
            break;

        case FN_NEW_HTML_DOC:
            if( ISA( SwWebDocShell ) )
                rSet.DisableItem( nWhich );
            break;

        case SID_ATTR_YEAR2000:
            {
                const SvNumberFormatter* pFmtr = pDoc->GetNumberFormatter(sal_False);
                rSet.Put( SfxUInt16Item( nWhich,
                        static_cast< sal_uInt16 >(
                        pFmtr ? pFmtr->GetYear2000()
                              : ::utl::MiscCfg().GetYear2000() )));
            }
            break;
        case SID_ATTR_CHAR_FONTLIST:
        {
            rSet.Put( SvxFontListItem( pFontList, SID_ATTR_CHAR_FONTLIST ) );
        }
        break;
        case SID_MAIL_PREPAREEXPORT:
        {
            //check if linked content or possibly hidden content is available
            //pDoc->UpdateFlds( NULL, false );
            sfx2::LinkManager& rLnkMgr = pDoc->GetLinkManager();
            const ::sfx2::SvBaseLinks& rLnks = rLnkMgr.GetLinks();
            sal_Bool bRet = sal_False;
            if( rLnks.Count() )
                bRet = sal_True;
            else
            {
                //sections with hidden flag, hidden character attribute, hidden paragraph/text or conditional text fields
                bRet = pDoc->HasInvisibleContent();
            }
            rSet.Put( SfxBoolItem( nWhich, bRet ) );
        }
        break;

        default: DBG_ASSERT(!this,"Hier darfst Du nicht hinein!");

        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   OLE-Hdls
 --------------------------------------------------------------------*/


IMPL_LINK( SwDocShell, Ole2ModifiedHdl, void *, p )
{
    // vom Doc wird der Status mitgegeben (siehe doc.cxx)
    //  Bit 0:  -> alter Zustand
    //  Bit 1:  -> neuer Zustand
    long nStatus = (long)p;
    if( IsEnableSetModified() )
        SetModified( (nStatus & 2) ? sal_True : sal_False );
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:   Pool returnen Hier weil virtuelll
 --------------------------------------------------------------------*/


SfxStyleSheetBasePool*  SwDocShell::GetStyleSheetPool()
{
    return mxBasePool.get();
}


void SwDocShell::SetView(SwView* pVw)
{
    if ( 0 != (pView = pVw) )
        pWrtShell = &pView->GetWrtShell();
    else
        pWrtShell = 0;
}


void SwDocShell::PrepareReload()
{
    ::DelAllGrfCacheEntries( pDoc );
}

// --> OD 2006-11-07 #i59688#
// linked graphics are now loaded on demand.
// Thus, loading of linked graphics no longer needed and necessary for
// the load of document being finished.
void SwDocShell::LoadingFinished()
{
    // --> OD 2007-10-08 #i38810#
    // Original fix fails after integration of cws xmlsec11:
    // interface <SfxObjectShell::EnableSetModified(..)> no longer works, because
    // <SfxObjectShell::FinishedLoading(..)> doesn't care about its status and
    // enables the document modification again.
    // Thus, manuell modify the document, if its modified and its links are updated
    // before <FinishedLoading(..)> is called.
    const bool bHasDocToStayModified( pDoc->IsModified() && pDoc->LinksUpdated() );
//    // --> OD 2005-02-11 #i38810# - disable method <SetModified(..)>, if document
//    // has stay in modified state, due to the update of its links during load.
//    bool bResetEnableSetModified(false);
//    if ( IsEnableSetModified() &&
//         pDoc->IsModified() && pDoc->LinksUpdated() )
//    {
//        EnableSetModified( sal_False );
//        bResetEnableSetModified = true;
//    }
    // <--
    FinishedLoading( SFX_LOADED_ALL );
//    // --> OD 2005-02-11 #i38810#
//    if ( bResetEnableSetModified )
//    {
//        EnableSetModified( sal_True );
//    }
//    // <--
    SfxViewFrame* pVFrame = SfxViewFrame::GetFirst(this);
    if(pVFrame)
    {
        SfxViewShell* pShell = pVFrame->GetViewShell();
        if(PTR_CAST(SwSrcView, pShell))
            ((SwSrcView*)pShell)->Load(this);
    }

    // --> OD 2007-10-08 #i38810#
    if ( bHasDocToStayModified && !pDoc->IsModified() )
    {
        pDoc->SetModified();
    }
    // <--
}

// eine Uebertragung wird abgebrochen (wird aus dem SFX gerufen)
void SwDocShell::CancelTransfers()
{
    // alle Links vom LinkManager Canceln
    aFinishedTimer.Stop();
    pDoc->GetLinkManager().CancelTransfers();
    SfxObjectShell::CancelTransfers();
}

SwFEShell* SwDocShell::GetFEShell()
{
    return pWrtShell;
}

void SwDocShell::RemoveOLEObjects()
{
    SwIterator<SwCntntNode,SwFmtColl> aIter( *pDoc->GetDfltGrfFmtColl() );
    for( SwCntntNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
    {
        SwOLENode* pOLENd = pNd->GetOLENode();
        if( pOLENd && ( pOLENd->IsOLEObjectDeleted() ||
                        pOLENd->IsInGlobalDocSection() ) )
        {
            if( !pOLEChildList )
                pOLEChildList = new comphelper::EmbeddedObjectContainer;

            ::rtl::OUString aObjName = pOLENd->GetOLEObj().GetCurrentPersistName();
            GetEmbeddedObjectContainer().MoveEmbeddedObject( aObjName, *pOLEChildList );
        }
    }
}

// When a document is loaded, SwDoc::PrtOLENotify is called to update
// the sizes of math objects. However, for objects that do not have a
// SwFrm at this time, only a flag is set (bIsOLESizeInvalid) and the
// size change takes place later, while calculating the layout in the
// idle handler. If this document is saved now, it is saved with invalid
// sizes. For this reason, the layout has to be calculated before a document is
// saved, but of course only id there are OLE objects with bOLESizeInvalid set.
void SwDocShell::CalcLayoutForOLEObjects()
{
    if( !pWrtShell )
        return;

    SwIterator<SwCntntNode,SwFmtColl> aIter( *pDoc->GetDfltGrfFmtColl() );
    for( SwCntntNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
    {
        SwOLENode* pOLENd = pNd->GetOLENode();
        if( pOLENd && pOLENd->IsOLESizeInvalid() )
        {
            pWrtShell->CalcLayout();
            break;
        }
    }
}


// --> FME 2005-02-25 #i42634# Overwrites SfxObjectShell::UpdateLinks
// This new function is necessary to trigger update of links in docs
// read by the binary filter:
void SwDocShell::UpdateLinks()
{
    GetDoc()->UpdateLinks(sal_True);
    // --> FME 2005-07-27 #i50703# Update footnote numbers
    SwTxtFtn::SetUniqueSeqRefNo( *GetDoc() );
    SwNodeIndex aTmp( GetDoc()->GetNodes() );
    GetDoc()->GetFtnIdxs().UpdateFtn( aTmp );
    // <--
}

uno::Reference< frame::XController >
                                SwDocShell::GetController()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > aRet;
    // --> FME 2007-10-15 #i82346# No view in page preview
    if ( GetView() )
    // <--
        aRet = GetView()->GetController();
    return aRet;
}

/* -----------------------------12.02.01 12:08--------------------------------

 ---------------------------------------------------------------------------*/
static const char* pEventNames[] =
{
    "OnPageCountChange",
    "OnMailMerge",
    "OnMailMergeFinished",
    "OnFieldMerge",
    "OnFieldMergeFinished",
    "OnLayoutFinished"
};

Sequence< OUString >    SwDocShell::GetEventNames()
{
    Sequence< OUString > aRet = SfxObjectShell::GetEventNames();
    sal_Int32 nLen = aRet.getLength();
    aRet.realloc(nLen + 6);
    OUString* pNames = aRet.getArray();
    pNames[nLen++] = GetEventName(0);
    pNames[nLen++] = GetEventName(1);
    pNames[nLen++] = GetEventName(2);
    pNames[nLen++] = GetEventName(3);
    pNames[nLen++] = GetEventName(4);
    pNames[nLen]   = GetEventName(5);

    return aRet;
}

static sal_Int32 nEvents=13;

rtl::OUString SwDocShell::GetEventName( sal_Int32 nIndex )
{
    if ( nIndex<nEvents )
        return ::rtl::OUString::createFromAscii(pEventNames[nIndex]);
    return rtl::OUString();
}

const ::sfx2::IXmlIdRegistry* SwDocShell::GetXmlIdRegistry() const
{
    return pDoc ? &pDoc->GetXmlIdRegistry() : 0;
}


bool SwDocShell::IsChangeRecording() const
{
    return (pWrtShell->GetRedlineMode() & nsRedlineMode_t::REDLINE_ON) != 0;
}


bool SwDocShell::HasChangeRecordProtection() const
{
    return pWrtShell->getIDocumentRedlineAccess()->GetRedlinePassword().getLength() > 0;
}


void SwDocShell::SetChangeRecording( bool bActivate )
{
    sal_uInt16 nOn = bActivate ? nsRedlineMode_t::REDLINE_ON : 0;
    sal_uInt16 nMode = pWrtShell->GetRedlineMode();
    pWrtShell->SetRedlineModeAndCheckInsMode( (nMode & ~nsRedlineMode_t::REDLINE_ON) | nOn);
}


bool SwDocShell::SetProtectionPassword( const String &rNewPassword )
{
    const SfxAllItemSet aSet( GetPool() );
    const SfxItemSet*   pArgs = &aSet;
    const SfxPoolItem*  pItem = NULL;

    IDocumentRedlineAccess* pIDRA = pWrtShell->getIDocumentRedlineAccess();
    Sequence< sal_Int8 > aPasswd = pIDRA->GetRedlinePassword();
    if (pArgs && SFX_ITEM_SET == pArgs->GetItemState( FN_REDLINE_PROTECT, sal_False, &pItem )
        && ((SfxBoolItem*)pItem)->GetValue() == (aPasswd.getLength() > 0))
        return false;

    bool bRes = false;

    if (rNewPassword.Len())
    {
        // when password protection is applied change tracking must always be active
        SetChangeRecording( true );

        Sequence< sal_Int8 > aNewPasswd;
        SvPasswordHelper::GetHashPassword( aNewPasswd, rNewPassword );
        pIDRA->SetRedlinePassword( aNewPasswd );
        bRes = true;
    }
    else
    {
        pIDRA->SetRedlinePassword( Sequence< sal_Int8 >() );
        bRes = true;
    }

    return bRes;
}


bool SwDocShell::GetProtectionHash( /*out*/ ::com::sun::star::uno::Sequence< sal_Int8 > &rPasswordHash )
{
    bool bRes = false;

    const SfxAllItemSet aSet( GetPool() );
    const SfxItemSet*   pArgs = &aSet;
    const SfxPoolItem*  pItem = NULL;

    IDocumentRedlineAccess* pIDRA = pWrtShell->getIDocumentRedlineAccess();
    Sequence< sal_Int8 > aPasswdHash( pIDRA->GetRedlinePassword() );
    if (pArgs && SFX_ITEM_SET == pArgs->GetItemState( FN_REDLINE_PROTECT, sal_False, &pItem )
        && ((SfxBoolItem*)pItem)->GetValue() == (aPasswdHash.getLength() != 0))
        return false;
    rPasswordHash = aPasswdHash;
    bRes = true;

    return bRes;
}


