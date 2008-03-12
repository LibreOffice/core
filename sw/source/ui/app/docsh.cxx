/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docsh.cxx,v $
 *
 *  $Revision: 1.78 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:44:15 $
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
 *    Foundation, Inc., 59: Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_JOBSET_HXX //autogen
#include <vcl/jobset.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif

#ifndef _SBXCLASS_HXX //autogen
#include <basic/sbx.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_MISCCFG_HXX
#include <svtools/misccfg.hxx>
#endif
#ifndef _PASSWD_HXX
#include <sfx2/passwd.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_EVENTCONF_HXX //autogen
#include <sfx2/evntconf.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX //autogen
#include <svx/flstitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVXMSBAS_HXX
#include <svx/svxmsbas.hxx>
#endif
#ifndef _SOERR_HXX
#include <svtools/soerr.hxx>
#endif
#include <sot/clsids.hxx>
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif
//#ifndef _SB_SBJSMOD_HXX //autogen
//#include <basic/sbjsmod.hxx>
//#endif

#ifndef _SWEVENT_HXX //autogen
#include <swevent.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _SWPRTOPT_HXX
#include <swprtopt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>         // fuer die aktuelle Sicht
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>        // Verbindung zur Core
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>        // Dokumenterzeugung
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _GLOBDOC_HXX
#include <globdoc.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>      // I/O
#endif
#ifndef _DOCSTYLE_HXX
#include <docstyle.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCSTAT_HXX
#include <docstat.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _PVIEW_HXX
#include <pview.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _SWBASLNK_HXX
#include <swbaslnk.hxx>
#endif
#ifndef _SRCVIEW_HXX
#include <srcview.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _SWCLI_HXX
#include <swcli.hxx>
#endif
#ifndef _TXTFTN_HXX
#include <txtftn.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif

// --> FME 2004-08-05 #i20883# Digital Signatures and Encryption
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
// <--

#ifndef _SWSWERROR_H
#include <swerror.h>        // Fehlermeldungen
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _CMDID_H
#include <cmdid.h>          //
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _APP_HRC
#include <app.hrc>
#endif
#ifndef SW_WARNPASSWORD_HXX
#include "warnpassword.hxx"
#endif

#include <cfgid.h>
#include <svtools/moduleoptions.hxx>
#include <svtools/fltrcfg.hxx>
#include <svx/htmlcfg.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/objface.hxx>
#include <comphelper/storagehelper.hxx>

#define SwDocShell
#ifndef _ITEMDEF_HXX
#include <itemdef.hxx>
#endif
#ifndef _SWSLOTS_HXX
#include <swslots.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <unomid.h>

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
    BOOL bAPICall = FALSE;
    const SfxPoolItem* pApiItem;
    const SfxItemSet* pMedSet;
    if( 0 != ( pMedSet = rMedium.GetItemSet() ) && SFX_ITEM_SET ==
            pMedSet->GetItemState( FN_API_CALL, TRUE, &pApiItem ) )
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
    SwRead pRead = SwIoSystem::GetReader( pFlt->GetUserData() );
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
            if(pSet && SFX_ITEM_SET == pSet->GetItemState(SID_PASSWORD, TRUE, &pPassItem))
                aPasswd = ((const SfxStringItem *)pPassItem)->GetValue();
        }

        if (!(*ppRdr)->CheckPasswd( aPasswd, *pRead ))
        {
            InfoBox( 0, SW_RES(MSG_ERROR_PASSWD)).Execute();
                delete *ppRdr;
            return 0;
        }
    }
    if(rMedium.IsStorage())
    {
        //SvStorageRef aStor( rMedium.GetStorage() );
        const SfxItemSet* pSet = rMedium.GetItemSet();
        const SfxPoolItem *pItem;
        if(pSet && SFX_ITEM_SET == pSet->GetItemState(SID_PASSWORD, TRUE, &pItem))
        {
            DBG_ASSERT(pItem->IsA( TYPE(SfxStringItem) ), "Fehler Parametertype");
            comphelper::OStorageHelper::SetCommonStoragePassword( rMedium.GetStorage(), ((const SfxStringItem *)pItem)->GetValue() );
        }
        // Fuer's Dokument-Einfuegen noch die FF-Version, wenn's der
        // eigene Filter ist.
        ASSERT( /*pRead != ReadSw3 || */pRead != ReadXML || pFlt->GetVersion(),
                "Am Filter ist keine FF-Version gesetzt" );
        //if( (pRead == ReadSw3 || pRead == ReadXML) && pFlt->GetVersion() )
        //    aStor->SetVersion( (long)pFlt->GetVersion() );
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
            pSet->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) )
            aOpt.ReadUserData( ((const SfxStringItem*)pItem)->GetValue() );

        if( pRead )
            pRead->GetReaderOpt().SetASCIIOpts( aOpt );
    }

    return pRead;
}

/*--------------------------------------------------------------------
    Beschreibung: Laden
 --------------------------------------------------------------------*/

BOOL SwDocShell::ConvertFrom( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::ConvertFrom" );

    SwReader* pRdr;
    SwRead pRead = StartConvertFrom(rMedium, &pRdr);
    if (!pRead)
      return FALSE; // #129881# return if no reader is found
    SotStorageRef pStg=pRead->getSotStorageRef(); // #i45333# save sot storage ref in case of recursive calls

    SwWait aWait( *this, TRUE );

        // SfxProgress unterdruecken, wenn man Embedded ist
    SW_MOD()->SetEmbeddedLoadSave(
                            SFX_CREATE_MODE_EMBEDDED == GetCreateMode() );

    pRdr->GetDoc()->set(IDocumentSettingAccess::HTML_MODE, ISA(SwWebDocShell));

    /* #106748# Restore the pool default if reading a saved document. */
    pDoc->RemoveAllFmtLanguageDependencies();

    ULONG nErr = pRdr->Read( *pRead );

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

    SW_MOD()->SetEmbeddedLoadSave( FALSE );

    SetError( nErr );
    BOOL bOk = !IsError( nErr );

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


BOOL SwDocShell::Save()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::Save" );
    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    if(pView)
        pView->GetEditWin().StopQuickHelp();
    SwWait aWait( *this, TRUE );

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

    ULONG nErr = ERR_SWG_WRITE_ERROR, nVBWarning = ERRCODE_NONE;
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
                xWrt->SetOrganizerMode( TRUE );
                SwWriter aWrt( *GetMedium(), *pDoc );
                nErr = aWrt.Write( xWrt );
                xWrt->SetOrganizerMode( FALSE );
            }
            break;

        case SFX_CREATE_MODE_EMBEDDED:
            // SfxProgress unterdruecken, wenn man Embedded ist
            SW_MOD()->SetEmbeddedLoadSave( TRUE );
            // kein break;

        case SFX_CREATE_MODE_STANDARD:
        case SFX_CREATE_MODE_PREVIEW:
        default:
            {
                if( pDoc->ContainsMSVBasic() )
                {
                    //TODO/MBA: it looks as that this code can be removed!
                    //SvxImportMSVBasic aTmp( *this, pIo->GetStorage() );
                    //aTmp.SaveOrDelMSVBAStorage( FALSE, aEmptyStr );
                    if( SvtFilterOptions::Get()->IsLoadWordBasicStorage() )
                        nVBWarning = SvxImportMSVBasic::
                                        GetSaveWarningOfMSVBAStorage( *this );
                    pDoc->SetContainsMSVBasic( FALSE );
                }

                // TabellenBox Edit beenden!
                if( pWrtShell )
                    pWrtShell->EndAllTblBoxEdit();

                WriterRef xWrt;
                ::GetXMLWriter( aEmptyStr, GetMedium()->GetBaseURL( true ), xWrt );

                BOOL bLockedView(FALSE);
                if ( pWrtShell )
                {
                    bLockedView = pWrtShell->IsViewLocked();
                    pWrtShell->LockView( TRUE );    //lock visible section
                }

                SwWriter aWrt( *GetMedium(), *pDoc );
                nErr = aWrt.Write( xWrt );

                if ( pWrtShell )
                    pWrtShell->LockView( bLockedView );
            }
            break;
        }
        SW_MOD()->SetEmbeddedLoadSave( FALSE );
    }
    SetError( nErr ? nErr : nVBWarning );

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

    SwWait aWait( *this, TRUE );
    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    if(pView)
        pView->GetEditWin().StopQuickHelp();

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

    ULONG nErr = ERR_SWG_WRITE_ERROR, nVBWarning = ERRCODE_NONE;
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
            SfxObjectShellRef xDocSh =
                new SwGlobalDocShell( SFX_CREATE_MODE_INTERNAL );
            // the global document can not be a template
            xDocSh->SetupStorage( xStor, SotStorage::GetVersion( xStor ), sal_False );
            xDocSh->DoClose();
        }

        if( pDoc->ContainsMSVBasic() )
        {
            //TODO/MBA: it looks as that this code can be removed!
            //SvxImportMSVBasic aTmp( *this, pIo->GetStorage() );
            //aTmp.SaveOrDelMSVBAStorage( FALSE, aEmptyStr );
            if( SvtFilterOptions::Get()->IsLoadWordBasicStorage() )
                nVBWarning = SvxImportMSVBasic::
                                GetSaveWarningOfMSVBAStorage( *this );
            pDoc->SetContainsMSVBasic( FALSE );
        }

        // TabellenBox Edit beenden!
        if( pWrtShell )
            pWrtShell->EndAllTblBoxEdit();

        // Modified-Flag merken und erhalten ohne den Link zu Callen
        // (fuer OLE; nach Anweisung von MM)
        BOOL bIsModified = pDoc->IsModified();
        SwUndoNoModifiedPosition aOldPos = pDoc->getUndoNoModifiedPosition();
        Link aOldOLELnk( pDoc->GetOle2Link() );
        pDoc->SetOle2Link( Link() );

            // SfxProgress unterdruecken, wenn man Embedded ist
        SW_MOD()->SetEmbeddedLoadSave(
                            SFX_CREATE_MODE_EMBEDDED == GetCreateMode() );

        WriterRef xWrt;
        ::GetXMLWriter( aEmptyStr, rMedium.GetBaseURL( true ), xWrt );

        BOOL bLockedView(FALSE);
        if ( pWrtShell )
        {
            bLockedView = pWrtShell->IsViewLocked();
            pWrtShell->LockView( TRUE );    //lock visible section
        }

        SwWriter aWrt( rMedium, *pDoc );
        nErr = aWrt.Write( xWrt );

        if ( pWrtShell )
            pWrtShell->LockView( bLockedView );

        if( bIsModified )
        {
            pDoc->SetModified();
            pDoc->setUndoNoModifiedPosition( aOldPos );
        }
        pDoc->SetOle2Link( aOldOLELnk );

        SW_MOD()->SetEmbeddedLoadSave( FALSE );
    }
    SetError( nErr ? nErr : nVBWarning );

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

BOOL SwDocShell::ConvertTo( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::ConvertTo" );
    const SfxFilter* pFlt = rMedium.GetFilter();
    if( !pFlt )
        return FALSE;

    WriterRef xWriter;
    SwIoSystem::GetWriter( pFlt->GetUserData(), rMedium.GetBaseURL( true ), xWriter );
    if( !xWriter.Is() )
    {   // Der Filter ist nicht vorhanden
        InfoBox( 0,
                 SW_RESSTR(STR_DLLNOTFOUND) ).Execute();
        return FALSE;
    }

    // if the imported word document is password protected - warn the user
    // about saving it without the password.
    if(pDoc->IsWinEncrypted())
    {
        if(!SwWarnPassword::WarningOnPassword( rMedium ))
        {
            SetError(ERRCODE_ABORT);
            return FALSE;
        }
    }

    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    if(pView)
        pView->GetEditWin().StopQuickHelp();
    ULONG nVBWarning = 0;

    if( pDoc->ContainsMSVBasic() )
    {
        BOOL bSave = pFlt->GetUserData().EqualsAscii( "CWW8" )
             && SvtFilterOptions::Get()->IsLoadWordBasicStorage();

        if ( bSave )
        {
            SvStorageRef xStg = new SotStorage( rMedium.GetOutStream(), FALSE );
            DBG_ASSERT( !xStg->GetError(), "No storage available for storing VBA macros!" );
            if ( !xStg->GetError() )
            {
                SvxImportMSVBasic aTmp( *this, *xStg );
                nVBWarning = aTmp.SaveOrDelMSVBAStorage( bSave, String::CreateFromAscii("Macros") );
                xStg->Commit();
                pDoc->SetContainsMSVBasic( TRUE );
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
                        SetError(WARN_SWG_HTML_NO_MACROS);
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
        BYTE nMyType = 0;
        if( ISA( SwWebDocShell) )
            nMyType = 1;
        else if( ISA( SwGlobalDocShell) )
            nMyType = 2;

        // gewuenschten Typ ermitteln
        BYTE nSaveType = 0;
        ULONG nSaveClipId = pFlt->GetFormat();
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
        BOOL bIsHTMLModeSave = GetDoc()->get(IDocumentSettingAccess::HTML_MODE);
        BOOL bIsGlobalDocSave = GetDoc()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT);
        BOOL bIsGlblDocSaveLinksSave = GetDoc()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS);
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
        BOOL bRet = SaveAs( rMedium );

        if( nMyType != nSaveType )
        {
            GetDoc()->set(IDocumentSettingAccess::HTML_MODE, bIsHTMLModeSave );
            GetDoc()->set(IDocumentSettingAccess::GLOBAL_DOCUMENT, bIsGlobalDocSave);
            GetDoc()->set(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS, bIsGlblDocSaveLinksSave);
        }

        if( bRet && nMyType != nSaveType )
        {
            SvGlobalName aClassName;
            String aAppName, aLongUserName, aUserName;
            SfxObjectShellRef xDocSh;
            switch( nSaveType )
            {
            case 0:
                xDocSh = new SwDocShell( SFX_CREATE_MODE_INTERNAL );
                break;
            case 1:
                xDocSh = new SwWebDocShell( SFX_CREATE_MODE_INTERNAL );
                break;
            case 2:
                xDocSh = new SwGlobalDocShell( SFX_CREATE_MODE_INTERNAL );
                break;
            }
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
                                                    TRUE, &pItem ) )
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
    ULONG nErrno;
    String aFileName( rMedium.GetName() );

    //Keine View also das ganze Dokument!
    if ( pWrtShell )
    {
        SwWait aWait( *this, TRUE );
        pWrtShell->StartAllAction();
        pWrtShell->Push();
        SwWriter aWrt( rMedium, *pWrtShell, TRUE );
        nErrno = aWrt.Write( xWriter, &aFileName );
        //JP 16.05.97: falls der SFX uns die View waehrend des speicherns
        //              entzieht
        if( pWrtShell )
        {
            pWrtShell->Pop(FALSE);
            pWrtShell->EndAllAction();
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

    SW_MOD()->SetEmbeddedLoadSave( FALSE );
    SetError( nErrno ? nErrno : nVBWarning );
    if( !rMedium.IsStorage() )
        rMedium.CloseOutStream();

    return !IsError( nErrno );
}

/*--------------------------------------------------------------------
    Beschreibung:   Haende weg
 --------------------------------------------------------------------*/


/*--------------------------------------------------------------------
    Beschreibung: ??? noch nicht zu aktivieren, muss TRUE liefern
 --------------------------------------------------------------------*/


sal_Bool SwDocShell::SaveCompleted( const uno::Reference < embed::XStorage >& xStor  )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::SaveCompleted" );
    BOOL bRet = SfxObjectShell::SaveCompleted( xStor );
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
        BOOL bResetModified = IsEnableSetModified();
        if( bResetModified )
            EnableSetModified( FALSE );

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
            EnableSetModified( TRUE );
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Draw()-Overload fuer OLE2 (Sfx)
 --------------------------------------------------------------------*/

void SwDocShell::Draw( OutputDevice* pDev, const JobSetup& rSetup,
                               USHORT nAspect )
{
    //fix #25341# Draw sollte das Modified nicht beeinflussen
    BOOL bResetModified;
    if ( TRUE == (bResetModified = IsEnableSetModified()) )
        EnableSetModified( FALSE );

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
    BOOL bWeb = 0 != PTR_CAST(SwWebDocShell, this);
    SwPrtOptions aOpts( aEmptyStr );
    ViewShell::PrtOle2( pDoc, SW_MOD()->GetUsrPref(bWeb), aOpts, pDev, aRect );
    pDev->Pop();

    if( pOrig )
    {
        pDoc->setJobsetup( *pOrig );
        delete pOrig;
    }
    if ( bResetModified )
        EnableSetModified( TRUE );
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
        pView->SetVisArea( aRect, TRUE );
    }
    else
        SfxObjectShell::SetVisArea( aRect );
}


Rectangle SwDocShell::GetVisArea( USHORT nAspect ) const
{
    if ( nAspect == ASPECT_THUMBNAIL )
    {
        //PreView: VisArea auf die erste Seite einstellen.
        SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), 1 );
        SwCntntNode* pNd = pDoc->GetNodes().GoNext( &aIdx );

        const SwRect aPageRect = pNd->FindPageFrmRect( FALSE, 0, FALSE );
        return aPageRect.SVRect();

        // Why does this have to be that complicated? I replaced this by the
        // call of FindPageFrmRect():
        /*
        //PageDesc besorgen, vom ersten Absatz oder den default.
        const SwFmtPageDesc &rDesc = pNd->GetSwAttrSet().GetPageDesc();
        const SwPageDesc* pDesc = rDesc.GetPageDesc();
        if( !pDesc )
            pDesc = &const_cast<const SwDoc *>(pDoc)->GetPageDesc( 0 );

        //Das Format wird evtl. von der virtuellen Seitennummer bestimmt.
        const USHORT nPgNum = rDesc.GetNumOffset();
        const BOOL bOdd = nPgNum % 2 ? TRUE : FALSE;
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

ULONG SwDocShell::GetMiscStatus() const
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
            SwClientIter aIter( *pType );
            SwClient* pFirst = aIter.GoStart();
            while( pFirst )
            {
                if( static_cast<SwFmtFld*>(pFirst)->GetTxtFld() &&
                    static_cast<SwFmtFld*>(pFirst)->IsFldInDoc() )
                {
                    nState |= HIDDENINFORMATION_NOTES;
                    break;
                }
                pFirst = ++aIter;
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
    USHORT  nWhich  = aIter.FirstWhich();

    while (nWhich)
    {
        switch (nWhich)
        {
        // MT: MakroChosser immer enablen, weil Neu moeglich
        // case SID_BASICCHOOSER:
        // {
        //  StarBASIC* pBasic = GetBasic();
        //  StarBASIC* pAppBasic = SFX_APP()->GetBasic();
        //  if ( !(pBasic->GetModules()->Count() ||
        //      pAppBasic->GetModules()->Count()) )
        //          rSet.DisableItem(nWhich);
        // }
        // break;
        case SID_PRINTPREVIEW:
        {
            BOOL bDisable = IsInPlaceActive();
            if ( !bDisable )
            {
                SfxViewFrame *pTmpFrm = SfxViewFrame::GetFirst(this);
                while (pTmpFrm)     // Preview suchen
                {
                    if ( PTR_CAST(SwView, pTmpFrm->GetViewShell()) &&
                         ((SwView*)pTmpFrm->GetViewShell())->GetWrtShell().getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE))
                    {
                        bDisable = TRUE;
                        break;
                    }
                    pTmpFrm = pTmpFrm->GetNext(*pTmpFrm, this);
                }
            }
            if ( bDisable )
                rSet.DisableItem( SID_PRINTPREVIEW );
            else
            {
                SfxBoolItem aBool( SID_PRINTPREVIEW, FALSE );
                if( PTR_CAST( SwPagePreView, SfxViewShell::Current()) )
                    aBool.SetValue( TRUE );
                rSet.Put( aBool );
            }
        }
        break;
        case SID_SOURCEVIEW:
        {
            if(IsLoading())
                rSet.DisableItem(nWhich);
            else
            {
                SfxViewShell* pCurrView = GetView() ? (SfxViewShell*)GetView()
                                            : SfxViewShell::Current();
                BOOL bSourceView = 0 != PTR_CAST(SwSrcView, pCurrView);
                rSet.Put(SfxBoolItem(SID_SOURCEVIEW, bSourceView));
            }
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
                SfxViewShell* pViewShell = SfxViewShell::Current();
                BOOL bDisable = 0 != PTR_CAST(SwPagePreView, pViewShell) ||
                                0 != PTR_CAST(SwSrcView, pViewShell);
                if (bDisable)
                    rSet.DisableItem( nWhich );
                else
                {
                    sal_Bool bState = GetDoc()->get(IDocumentSettingAccess::BROWSE_MODE);
                    if(FN_PRINT_LAYOUT == nWhich)
                        bState = !bState;
                    rSet.Put( SfxBoolItem( nWhich, bState));
                }
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
                const SvNumberFormatter* pFmtr = pDoc->GetNumberFormatter(FALSE);
                rSet.Put( SfxUInt16Item( nWhich,
                        static_cast< sal_uInt16 >(
                        pFmtr ? pFmtr->GetYear2000()
                              : SFX_APP()->GetMiscConfig()->GetYear2000() )));
            }
            break;
        case SID_ATTR_CHAR_FONTLIST:
        {
            rSet.Put( SvxFontListItem( pFontList, SID_ATTR_CHAR_FONTLIST ) );
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
        SetModified( (nStatus & 2) ? TRUE : FALSE );
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
//        EnableSetModified( FALSE );
//        bResetEnableSetModified = true;
//    }
    // <--
    FinishedLoading( SFX_LOADED_ALL );
//    // --> OD 2005-02-11 #i38810#
//    if ( bResetEnableSetModified )
//    {
//        EnableSetModified( TRUE );
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
    SwClientIter aIter( *(SwModify*)pDoc->GetDfltGrfFmtColl() );
    for( SwCntntNode* pNd = (SwCntntNode*)aIter.First( TYPE( SwCntntNode ) );
            pNd; pNd = (SwCntntNode*)aIter.Next() )
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

    SwClientIter aIter( *(SwModify*)pDoc->GetDfltGrfFmtColl() );
    for( SwCntntNode* pNd = (SwCntntNode*)aIter.First( TYPE( SwCntntNode ) );
            pNd; pNd = (SwCntntNode*)aIter.Next() )
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
    GetDoc()->UpdateLinks(TRUE);
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
Sequence< OUString >    SwDocShell::GetEventNames()
{
    Sequence< OUString > aRet = SfxObjectShell::GetEventNames();
    sal_Int32 nLen = aRet.getLength();
    aRet.realloc(nLen + 2);
    OUString* pNames = aRet.getArray();
    pNames[nLen++] = OUString::createFromAscii("OnMailMerge");
    pNames[nLen] = OUString::createFromAscii("OnPageCountChange");
    return aRet;
}
/*
void SwTmpPersist::FillClass( SvGlobalName * pClassName,
                            ULONG * pClipFormat,
                            String * pAppName,
                            String * pLongUserName,
                            String * pUserName,
                            sal_Int32 nFileFormat ) const
{
    pDShell->SwDocShell::FillClass( pClassName, pClipFormat, pAppName,
                                    pLongUserName, pUserName, nFileFormat );
}

BOOL SwTmpPersist::Save()
{
    if( SaveChilds() )
        return SvPersist::Save();
    return FALSE;
}

BOOL SwTmpPersist::SaveCompleted( SvStorage * pStor )
{
    if( SaveCompletedChilds( pStor ) )
        return SvPersist::SaveCompleted( pStor );
    return FALSE;
} */


