/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <hintids.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/jobset.hxx>
#include <svl/whiter.hxx>
#include <svl/zforlist.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/PasswordHelper.hxx>
#include <editeng/adjustitem.hxx>
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
#include <sfx2/htmlmode.hxx>
#include <svtools/soerr.hxx>
#include <comphelper/classids.hxx>
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
#include <view.hxx>
#include <edtwin.hxx>
#include <PostItMgr.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <basesh.hxx>
#include <viewopt.hxx>
#include <wdocsh.hxx>
#include <swmodule.hxx>
#include <globdoc.hxx>
#include <usrpref.hxx>
#include <shellio.hxx>
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
#include <swerror.h>
#include <helpid.h>
#include <cmdid.h>
#include <globals.hrc>
#include <app.hrc>

#include <cfgid.h>
#include <unotools/fltrcfg.hxx>
#include <svtools/htmlcfg.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>
#include <comphelper/storagehelper.hxx>

#define SwDocShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <unomid.h>
#include <unotextrange.hxx>

#include <sfx2/Metadatable.hxx>
#include <switerator.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::container;

SFX_IMPL_INTERFACE(SwDocShell, SfxObjectShell, SW_RES(0) )

void SwDocShell::InitInterface_Impl()
{
}

TYPEINIT2(SwDocShell, SfxObjectShell, SfxListener);

SFX_IMPL_OBJECTFACTORY(SwDocShell, SvGlobalName(SO3_SW_CLASSID), SFXOBJECTSHELL_STD_NORMAL|SFXOBJECTSHELL_HASMENU, "swriter"  )

bool SwDocShell::InsertGeneratedStream(SfxMedium & rMedium,
        uno::Reference<text::XTextRange> const& xInsertPosition)
{
    SwUnoInternalPaM aPam(*GetDoc()); // must have doc since called from SwView
    if (!::sw::XTextRangeToSwPaM(aPam, xInsertPosition))
        return false;
    // similar to SwView::InsertMedium
    SwReader *pReader(0);
    Reader *const pRead = StartConvertFrom(rMedium, &pReader, 0, &aPam);
    if (!pRead)
        return false;
    sal_uLong const nError = pReader->Read(*pRead);
    delete pReader;
    return 0 == nError;
}

// Prepare loading
Reader* SwDocShell::StartConvertFrom(SfxMedium& rMedium, SwReader** ppRdr,
                                    SwCrsrShell *pCrsrShell,
                                    SwPaM* pPaM )
{
    bool bAPICall = false;
    const SfxPoolItem* pApiItem;
    const SfxItemSet* pMedSet;
    if( 0 != ( pMedSet = rMedium.GetItemSet() ) && SFX_ITEM_SET ==
            pMedSet->GetItemState( FN_API_CALL, true, &pApiItem ) )
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
    OUString aFileName( rMedium.GetName() );
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

    // Check password
    OUString aPasswd;
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
            if(pSet && SFX_ITEM_SET == pSet->GetItemState(SID_PASSWORD, true, &pPassItem))
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
    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, false);
    nUpdateDocMode = pUpdateDocItem ? pUpdateDocItem->GetValue() : document::UpdateDocMode::NO_UPDATE;

    if (!pFlt->GetDefaultTemplate().isEmpty())
        pRead->SetTemplateName( pFlt->GetDefaultTemplate() );

    if( pRead == ReadAscii && 0 != rMedium.GetInStream() &&
        pFlt->GetUserData() == FILTER_TEXT_DLG )
    {
        SwAsciiOptions aOpt;
        const SfxItemSet* pSet;
        const SfxPoolItem* pItem;
        if( 0 != ( pSet = rMedium.GetItemSet() ) && SFX_ITEM_SET ==
            pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
            aOpt.ReadUserData( ((const SfxStringItem*)pItem)->GetValue() );

        if( pRead )
            pRead->GetReaderOpt().SetASCIIOpts( aOpt );
    }

    return pRead;
}

// Loading
bool SwDocShell::ConvertFrom( SfxMedium& rMedium )
{
    SwReader* pRdr;
    SwRead pRead = StartConvertFrom(rMedium, &pRdr);
    if (!pRead)
      return false; // #129881# return if no reader is found
    SotStorageRef pStg=pRead->getSotStorageRef(); // #i45333# save sot storage ref in case of recursive calls

    pDoc->setDocAccTitle(OUString());
    SfxViewFrame* pFrame1 = SfxViewFrame::GetFirst( this );
    if (pFrame1)
    {
        Window* pWindow = &pFrame1->GetWindow();
        if ( pWindow )
        {
            Window* pSysWin = pWindow->GetSystemWindow();
            if ( pSysWin )
            {
                pSysWin->SetAccessibleName(OUString());
            }
        }
    }
    SwWait aWait( *this, true );

        // Suppress SfxProgress, when we are Embedded
    SW_MOD()->SetEmbeddedLoadSave(
                            SFX_CREATE_MODE_EMBEDDED == GetCreateMode() );

    pRdr->GetDoc()->set(IDocumentSettingAccess::HTML_MODE, ISA(SwWebDocShell));

    /* #106748# Restore the pool default if reading a saved document. */
    pDoc->RemoveAllFmtLanguageDependencies();

    sal_uLong nErr = pRdr->Read( *pRead );

    // Maybe put away one old Doc
    if ( pDoc != pRdr->GetDoc() )
    {
        RemoveLink();
        pDoc = pRdr->GetDoc();

        AddLink();

        if ( !mxBasePool.is() )
            mxBasePool = new SwDocStyleSheetPool( *pDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
    }

    UpdateFontList();
    InitDraw();

    delete pRdr;

    SW_MOD()->SetEmbeddedLoadSave( false );

    SetError( nErr, OUString(  OSL_LOG_PREFIX  ) );
    bool bOk = !IsError( nErr );

    if ( bOk && !pDoc->IsInLoadAsynchron() )
    {
        LoadingFinished();
    }

    pRead->setSotStorageRef(pStg); // #i45333# save sot storage ref in case of recursive calls

    return bOk;
}

// Saving the Default-Format, Stg present
bool SwDocShell::Save()
{
    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    if(pView)
        pView->GetEditWin().StopQuickHelp();
    SwWait aWait( *this, true );

    CalcLayoutForOLEObjects();  // format for OLE objets
    // #i62875#
    // reset compatibility flag <DoNotCaptureDrawObjsOnPage>, if possible
    if ( pWrtShell && pDoc &&
         pDoc->get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
         docfunc::AllDrawObjsOnPage( *pDoc ) )
    {
        pDoc->set(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
    }

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
                ::GetXMLWriter( aEmptyOUStr, GetMedium()->GetBaseURL( true ), xWrt );
                xWrt->SetOrganizerMode( true );
                SwWriter aWrt( *GetMedium(), *pDoc );
                nErr = aWrt.Write( xWrt );
                xWrt->SetOrganizerMode( false );
            }
            break;

        case SFX_CREATE_MODE_EMBEDDED:
            // Suppress SfxProgress, if we are Embedded
            SW_MOD()->SetEmbeddedLoadSave( true );
            // no break;

        case SFX_CREATE_MODE_STANDARD:
        case SFX_CREATE_MODE_PREVIEW:
        default:
            {
                if( pDoc->ContainsMSVBasic() )
                {
                    if( SvtFilterOptions::Get().IsLoadWordBasicStorage() )
                        nVBWarning = GetSaveWarningOfMSVBAStorage( (SfxObjectShell&) (*this) );
                    pDoc->SetContainsMSVBasic( false );
                }

                // End TableBox Edit!
                if( pWrtShell )
                    pWrtShell->EndAllTblBoxEdit();

                WriterRef xWrt;
                ::GetXMLWriter( aEmptyOUStr, GetMedium()->GetBaseURL( true ), xWrt );

                bool bLockedView(false);
                if ( pWrtShell )
                {
                    bLockedView = pWrtShell->IsViewLocked();
                    pWrtShell->LockView( true );    //lock visible section
                }

                SwWriter aWrt( *GetMedium(), *pDoc );
                nErr = aWrt.Write( xWrt );

                if ( pWrtShell )
                    pWrtShell->LockView( bLockedView );
            }
            break;
        }
        SW_MOD()->SetEmbeddedLoadSave( false );
    }
    SetError( nErr ? nErr : nVBWarning, OUString(  OSL_LOG_PREFIX  ) );

    SfxViewFrame* pFrm = pWrtShell ? pWrtShell->GetView().GetViewFrame() : 0;
    if( pFrm )
    {
        pFrm->GetBindings().SetState(SfxBoolItem(SID_DOC_MODIFIED, false));
    }
    return !IsError( nErr );
}

// Save using the Defaultformat
bool SwDocShell::SaveAs( SfxMedium& rMedium )
{
    SwWait aWait( *this, true );
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
            ( pOldFilter->GetUserData() == FILTER_WW8 ||
              pOldFilter->GetUserData() == "CWW6" ||
              pOldFilter->GetUserData() == "WW6" ||
              pOldFilter->GetUserData() == "WW1" ) )
        {
            // when saving it in our own fileformat, then remove the template
            // name from the docinfo.
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                GetModel(), uno::UNO_QUERY_THROW);
            uno::Reference<document::XDocumentProperties> xDocProps
                = xDPS->getDocumentProperties();
            xDocProps->setTemplateName(OUString());
            xDocProps->setTemplateURL(OUString());
            xDocProps->setTemplateDate(::util::DateTime());
        }
    }

    CalcLayoutForOLEObjects();  // format for OLE objets
    // #i62875#
    // reset compatibility flag <DoNotCaptureDrawObjsOnPage>, if possible
    if ( pWrtShell &&
         pDoc->get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
         docfunc::AllDrawObjsOnPage( *pDoc ) )
    {
        pDoc->set(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
    }

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
            // The document is closed explicitly, but using SfxObjectShellLock is still more correct here
            SfxObjectShellLock xDocSh =
                new SwGlobalDocShell( SFX_CREATE_MODE_INTERNAL );
            // the global document can not be a template
            xDocSh->SetupStorage( xStor, SotStorage::GetVersion( xStor ), false );
            xDocSh->DoClose();
        }

        if( pDoc->ContainsMSVBasic() )
        {
            if( SvtFilterOptions::Get().IsLoadWordBasicStorage() )
                nVBWarning = GetSaveWarningOfMSVBAStorage( (SfxObjectShell&) *this );
            pDoc->SetContainsMSVBasic( false );
        }

        // End TableBox Edit!
        if( pWrtShell )
            pWrtShell->EndAllTblBoxEdit();

        // Remember and preserve Modified-Flag without calling the Link
        // (for OLE; after Statement from MM)
        bool bIsModified = pDoc->IsModified();
        pDoc->GetIDocumentUndoRedo().LockUndoNoModifiedPosition();
        Link aOldOLELnk( pDoc->GetOle2Link() );
        pDoc->SetOle2Link( Link() );

            // Suppress SfxProgress when we are Embedded
        SW_MOD()->SetEmbeddedLoadSave(
                            SFX_CREATE_MODE_EMBEDDED == GetCreateMode() );

        WriterRef xWrt;
        ::GetXMLWriter( aEmptyOUStr, rMedium.GetBaseURL( true ), xWrt );

        bool bLockedView(false);
        if ( pWrtShell )
        {
            bLockedView = pWrtShell->IsViewLocked();
            pWrtShell->LockView( true );    //lock visible section
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

        SW_MOD()->SetEmbeddedLoadSave( false );

        // Increase RSID
        pDoc->setRsid( pDoc->getRsid() );
    }
    SetError( nErr ? nErr : nVBWarning, OUString(  OSL_LOG_PREFIX  ) );

    return !IsError( nErr );
}

// Save all Formats
static SwSrcView* lcl_GetSourceView( SwDocShell* pSh )
{
    // are we in SourceView?
    SfxViewFrame* pVFrame = SfxViewFrame::GetFirst( pSh );
    SfxViewShell* pViewShell = pVFrame ? pVFrame->GetViewShell() : 0;
    return PTR_CAST( SwSrcView, pViewShell);
}

bool SwDocShell::ConvertTo( SfxMedium& rMedium )
{
    const SfxFilter* pFlt = rMedium.GetFilter();
    if( !pFlt )
        return false;

    WriterRef xWriter;
    SwReaderWriter::GetWriter( pFlt->GetUserData(), rMedium.GetBaseURL( true ), xWriter );
    if( !xWriter.Is() )
    {   // Filter not available
        InfoBox( 0,
                 SW_RESSTR(STR_DLLNOTFOUND) ).Execute();
        return false;
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
        bool bSave = pFlt->GetUserData() == "CWW8"
             && SvtFilterOptions::Get().IsLoadWordBasicStorage();

        if ( bSave )
        {
            SvStorageRef xStg = new SotStorage( rMedium.GetOutStream(), false );
            OSL_ENSURE( !xStg->GetError(), "No storage available for storing VBA macros!" );
            if ( !xStg->GetError() )
            {
                nVBWarning = SaveOrDelMSVBAStorage( (SfxObjectShell&) *this, *xStg, bSave, OUString("Macros") );
                xStg->Commit();
                pDoc->SetContainsMSVBasic( true );
            }
        }
    }

    // End TableBox Edit!
    if( pWrtShell )
        pWrtShell->EndAllTblBoxEdit();

    if( pFlt->GetUserData() == "HTML" )
    {
#ifndef DISABLE_SCRIPTING
        SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
        if( !rHtmlOpt.IsStarBasic() && rHtmlOpt.IsStarBasicWarning() && HasBasic() )
        {
            uno::Reference< XLibraryContainer > xLibCont(GetBasicContainer(), UNO_QUERY);
            uno::Reference< XNameAccess > xLib;
            Sequence<OUString> aNames = xLibCont->getElementNames();
            const OUString* pNames = aNames.getConstArray();
            for(sal_Int32 nLib = 0; nLib < aNames.getLength(); nLib++)
            {
                Any aLib = xLibCont->getByName(pNames[nLib]);
                aLib >>= xLib;
                if(xLib.is())
                {
                    Sequence<OUString> aModNames = xLib->getElementNames();
                    if(aModNames.getLength())
                    {
                        SetError(WARN_SWG_HTML_NO_MACROS, OUString(  OSL_LOG_PREFIX  ) );
                        break;
                    }
                }
            }
        }
#endif
    }

    // #i76360# Update document statistics
    pDoc->UpdateDocStat();

    CalcLayoutForOLEObjects();  // format for OLE objets
    // #i62875#
    // reset compatibility flag <DoNotCaptureDrawObjsOnPage>, if possible
    if ( pWrtShell &&
         pDoc->get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
         docfunc::AllDrawObjsOnPage( *pDoc ) )
    {
        pDoc->set(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
    }

    if( xWriter->IsStgWriter() &&
        ( pFlt->GetUserData() == FILTER_XML ||
          pFlt->GetUserData() == FILTER_XMLV ||
          pFlt->GetUserData() == FILTER_XMLVW ) )
    {
        // determine the own Type
        sal_uInt8 nMyType = 0;
        if( ISA( SwWebDocShell) )
            nMyType = 1;
        else if( ISA( SwGlobalDocShell) )
            nMyType = 2;

        // determine the desired Type
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

        // Change Flags of the Document accordingly
        bool bIsHTMLModeSave = GetDoc()->get(IDocumentSettingAccess::HTML_MODE);
        bool bIsGlobalDocSave = GetDoc()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT);
        bool bIsGlblDocSaveLinksSave = GetDoc()->get(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS);
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
                    xSet->setPropertyValue("MediaType", uno::makeAny( OUString( SotExchange::GetFormatMimeType( nSaveClipId ) ) ) );
            }
            catch (const uno::Exception&)
            {
            }
        }

        // Now normally save the Document
        bool bRet = SaveAs( rMedium );

        if( nMyType != nSaveType )
        {
            GetDoc()->set(IDocumentSettingAccess::HTML_MODE, bIsHTMLModeSave );
            GetDoc()->set(IDocumentSettingAccess::GLOBAL_DOCUMENT, bIsGlobalDocSave);
            GetDoc()->set(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS, bIsGlblDocSaveLinksSave);
        }

        return bRet;
    }

    if( pFlt->GetUserData() == FILTER_TEXT_DLG &&
        ( pWrtShell || !::lcl_GetSourceView( this ) ))
    {
        SwAsciiOptions aOpt;
        OUString sItemOpt;
        const SfxItemSet* pSet;
        const SfxPoolItem* pItem;
        if( 0 != ( pSet = rMedium.GetItemSet() ) )
        {
            if( SFX_ITEM_SET == pSet->GetItemState( SID_FILE_FILTEROPTIONS,
                                                    true, &pItem ) )
                sItemOpt = ((const SfxStringItem*)pItem)->GetValue();
        }
        if(!sItemOpt.isEmpty())
            aOpt.ReadUserData( sItemOpt );

        xWriter->SetAsciiOptions( aOpt );
    }

        // Suppress SfxProgress when we are Embedded
    SW_MOD()->SetEmbeddedLoadSave(
                            SFX_CREATE_MODE_EMBEDDED == GetCreateMode());

    // Span Context in order to suppress the Selection's View
    sal_uLong nErrno;
    const OUString aFileName( rMedium.GetName() );

    // No View, so the whole Document!
    if ( pWrtShell && !Application::IsHeadlessModeEnabled() )
    {
        SwWait aWait( *this, true );
        // #i106906#
        const bool bFormerLockView = pWrtShell->IsViewLocked();
        pWrtShell->LockView( true );
        pWrtShell->StartAllAction();
        pWrtShell->Push();
        SwWriter aWrt( rMedium, *pWrtShell, true );
        nErrno = aWrt.Write( xWriter, &aFileName );
        //JP 16.05.97: In case the SFX revokes the View while saving
        if( pWrtShell )
        {
            pWrtShell->Pop(false);
            pWrtShell->EndAllAction();
            // #i106906#
            pWrtShell->LockView( bFormerLockView );
        }
    }
    else
    {
        // are we in SourceView?
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

    SW_MOD()->SetEmbeddedLoadSave( false );
    SetError( nErrno ? nErrno : nVBWarning, OUString(  OSL_LOG_PREFIX  ) );
    if( !rMedium.IsStorage() )
        rMedium.CloseOutStream();

    return !IsError( nErrno );
}

// Hands off
// do not yet activate, must deliver TRUE
bool SwDocShell::SaveCompleted( const uno::Reference < embed::XStorage >& xStor  )
{
    bool bRet = SfxObjectShell::SaveCompleted( xStor );
    if( bRet )
    {
        // Do not decide until here, whether Saving was successful or not
        if( IsModified() )
            pDoc->SetModified();
        else
            pDoc->ResetModified();
    }

    if( pOLEChildList )
    {
        bool bResetModified = IsEnableSetModified();
        if( bResetModified )
            EnableSetModified( false );

        uno::Sequence < OUString > aNames = pOLEChildList->GetObjectNames();
        for( sal_Int32 n = aNames.getLength(); n; n-- )
        {
            if ( !pOLEChildList->MoveEmbeddedObject( aNames[n-1], GetEmbeddedObjectContainer() ) )
            {
                OSL_FAIL("Copying of objects didn't work!" );
            }
        }

        DELETEZ( pOLEChildList );
        if( bResetModified )
            EnableSetModified( true );
    }
    return bRet;
}

// Draw()-Overload for OLE2 (Sfx)
void SwDocShell::Draw( OutputDevice* pDev, const JobSetup& rSetup,
                               sal_uInt16 nAspect )
{
    //fix #25341# Draw should not affect the Modified
    bool bResetModified;
    if ( (bResetModified = IsEnableSetModified()) )
        EnableSetModified( false );

    // When there is a JobSetup connected to the Document, we copy it to
    // reconnect it after PrtOle2. We don't use an empty JobSetup because
    // that would only lead to questionable results after expensive
    // reformatting (Preview!)
    JobSetup *pOrig = 0;
    if ( !rSetup.GetPrinterName().isEmpty() && ASPECT_THUMBNAIL != nAspect )
    {
        pOrig = const_cast<JobSetup*>(pDoc->getJobsetup());
        if( pOrig )         // then we copy that
            pOrig = new JobSetup( *pOrig );
        pDoc->setJobsetup( rSetup );
    }

    Rectangle aRect( nAspect == ASPECT_THUMBNAIL ?
            GetVisArea( nAspect ) : GetVisArea( ASPECT_CONTENT ) );

    pDev->Push();
    pDev->SetFillColor();
    pDev->SetLineColor();
    pDev->SetBackground();
    const bool bWeb = this->ISA(SwWebDocShell);
    SwPrintData aOpts;
    SwViewShell::PrtOle2( pDoc, SW_MOD()->GetUsrPref(bWeb), aOpts, pDev, aRect );
    pDev->Pop();

    if( pOrig )
    {
        pDoc->setJobsetup( *pOrig );
        delete pOrig;
    }
    if ( bResetModified )
        EnableSetModified( true );
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

        // Calls SfxInPlaceObject::SetVisArea()!
        pView->SetVisArea( aRect, true );
    }
    else
        SfxObjectShell::SetVisArea( aRect );
}

Rectangle SwDocShell::GetVisArea( sal_uInt16 nAspect ) const
{
    if ( nAspect == ASPECT_THUMBNAIL )
    {
        // Preview: set VisArea to the first page.
        SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), 1 );
        SwCntntNode* pNd = pDoc->GetNodes().GoNext( &aIdx );

        const SwRect aPageRect = pNd->FindPageFrmRect( false, 0, false );
        return aPageRect.SVRect();
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

// #i20883# Digital Signatures and Encryption
sal_uInt16 SwDocShell::GetHiddenInformationState( sal_uInt16 nStates )
{
    // get global state like HIDDENINFORMATION_DOCUMENTVERSIONS
    sal_uInt16 nState = SfxObjectShell::GetHiddenInformationState( nStates );

    if ( nStates & HIDDENINFORMATION_RECORDEDCHANGES )
    {
        if ( !GetDoc()->GetRedlineTbl().empty() )
            nState |= HIDDENINFORMATION_RECORDEDCHANGES;
    }
    if ( nStates & HIDDENINFORMATION_NOTES )
    {
        OSL_ENSURE( GetWrtShell(), "No SwWrtShell, no information" );
        if ( GetWrtShell() )
        {
            SwFieldType* pType = GetWrtShell()->GetFldType( RES_POSTITFLD, aEmptyOUStr );
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
            bool bDisable = IsInPlaceActive();
            // Disable "multiple layout"
            if ( !bDisable )
            {
                SfxViewFrame *pTmpFrm = SfxViewFrame::GetFirst(this);
                while (pTmpFrm)     // Look for Preview
                {
                    if ( PTR_CAST(SwView, pTmpFrm->GetViewShell()) &&
                         ((SwView*)pTmpFrm->GetViewShell())->GetWrtShell().GetViewOptions()->getBrowseMode() )
                    {
                        bDisable = true;
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
                SfxBoolItem aBool( SID_PRINTPREVIEW, false );
                if( PTR_CAST( SwPagePreview, SfxViewShell::Current()) )
                    aBool.SetValue( true );
                rSet.Put( aBool );
            }
        }
        break;
        case SID_SOURCEVIEW:
        {
            SfxViewShell* pCurrView = GetView() ? (SfxViewShell*)GetView()
                                        : SfxViewShell::Current();
            bool bSourceView = 0 != PTR_CAST(SwSrcView, pCurrView);
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
                if ( GetDoc()->GetNodes().GetOutLineNds().empty() )
                    rSet.DisableItem( nWhich );
            }
            break;
        case SID_BROWSER_MODE:
        case FN_PRINT_LAYOUT:
            {
                bool bState = GetDoc()->get(IDocumentSettingAccess::BROWSE_MODE);
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

        case FN_OPEN_FILE:
            if( ISA( SwWebDocShell ) )
                rSet.DisableItem( nWhich );
            break;

        case SID_ATTR_YEAR2000:
            {
                const SvNumberFormatter* pFmtr = pDoc->GetNumberFormatter(false);
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
            bool bRet = false;
            if( !rLnks.empty() )
                bRet = true;
            else
            {
                //sections with hidden flag, hidden character attribute, hidden paragraph/text or conditional text fields
                bRet = pDoc->HasInvisibleContent();
            }
            rSet.Put( SfxBoolItem( nWhich, bRet ) );
        }
        break;

        default: OSL_ENSURE(!this,"You cannot get here!");

        }
        nWhich = aIter.NextWhich();
    }
}

// OLE-Hdls
IMPL_LINK( SwDocShell, Ole2ModifiedHdl, void *, p )
{
    // the Status is handed over from Doc (see doc.cxx)
    //  Bit 0:  -> old state
    //  Bit 1:  -> new state
    sal_IntPtr nStatus = (sal_IntPtr)p;
    if( IsEnableSetModified() )
        SetModified( (nStatus & 2) ? sal_True : sal_False );
    return 0;
}

// return Pool here, because virtual
SfxStyleSheetBasePool*  SwDocShell::GetStyleSheetPool()
{
    return mxBasePool.get();
}

void SwDocShell::SetView(SwView* pVw)
{
    SetViewShell_Impl(pVw);
    if ( 0 != (pView = pVw) )
        pWrtShell = &pView->GetWrtShell();
    else
        pWrtShell = 0;
}

void SwDocShell::PrepareReload()
{
    ::DelAllGrfCacheEntries( pDoc );
}

// #i59688#
// linked graphics are now loaded on demand.
// Thus, loading of linked graphics no longer needed and necessary for
// the load of document being finished.
void SwDocShell::LoadingFinished()
{
    // #i38810#
    // Original fix fails after integration of cws xmlsec11:
    // interface <SfxObjectShell::EnableSetModified(..)> no longer works, because
    // <SfxObjectShell::FinishedLoading(..)> doesn't care about its status and
    // enables the document modification again.
    // Thus, manuell modify the document, if its modified and its links are updated
    // before <FinishedLoading(..)> is called.
    const bool bHasDocToStayModified( pDoc->IsModified() && pDoc->LinksUpdated() );

    FinishedLoading( SFX_LOADED_ALL );
    SfxViewFrame* pVFrame = SfxViewFrame::GetFirst(this);
    if(pVFrame)
    {
        SfxViewShell* pShell = pVFrame->GetViewShell();
        if(PTR_CAST(SwSrcView, pShell))
            ((SwSrcView*)pShell)->Load(this);
    }

    // #i38810#
    if ( bHasDocToStayModified && !pDoc->IsModified() )
    {
        pDoc->SetModified();
    }
}

// a Transfer is cancelled (is called from SFX)
void SwDocShell::CancelTransfers()
{
    // Cancel all links from LinkManager
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

            OUString aObjName = pOLENd->GetOLEObj().GetCurrentPersistName();
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

// #i42634# Overwrites SfxObjectShell::UpdateLinks
// This new function is necessary to trigger update of links in docs
// read by the binary filter:
void SwDocShell::UpdateLinks()
{
    GetDoc()->UpdateLinks(true);
    // #i50703# Update footnote numbers
    SwTxtFtn::SetUniqueSeqRefNo( *GetDoc() );
    SwNodeIndex aTmp( GetDoc()->GetNodes() );
    GetDoc()->GetFtnIdxs().UpdateFtn( aTmp );
}

uno::Reference< frame::XController >
                                SwDocShell::GetController()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > aRet;
    // #i82346# No view in page preview
    if ( GetView() )
        aRet = GetView()->GetController();
    return aRet;
}

void SwDocShell::setDocAccTitle( const OUString& rTitle )
{
    if (pDoc)
    {
        pDoc->setDocAccTitle( rTitle );
    }
}

const OUString SwDocShell::getDocAccTitle() const
{
    OUString sRet;
    if (pDoc)
    {
        sRet = pDoc->getDocAccTitle();
    }

    return sRet;
}

static const char* s_EventNames[] =
{
    "OnPageCountChange",
    "OnMailMerge",
    "OnMailMergeFinished",
    "OnFieldMerge",
    "OnFieldMergeFinished",
    "OnLayoutFinished"
};
static sal_Int32 const s_nEvents(sizeof(s_EventNames)/sizeof(s_EventNames[0]));

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

OUString SwDocShell::GetEventName( sal_Int32 nIndex )
{
    if (nIndex < s_nEvents)
    {
        return OUString::createFromAscii(s_EventNames[nIndex]);
    }
    return OUString();
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

bool SwDocShell::SetProtectionPassword( const OUString &rNewPassword )
{
    const SfxAllItemSet aSet( GetPool() );
    const SfxItemSet*   pArgs = &aSet;
    const SfxPoolItem*  pItem = NULL;

    IDocumentRedlineAccess* pIDRA = pWrtShell->getIDocumentRedlineAccess();
    Sequence< sal_Int8 > aPasswd = pIDRA->GetRedlinePassword();
    if (pArgs && SFX_ITEM_SET == pArgs->GetItemState( FN_REDLINE_PROTECT, false, &pItem )
        && ((SfxBoolItem*)pItem)->GetValue() == (aPasswd.getLength() > 0))
        return false;

    bool bRes = false;

    if (!rNewPassword.isEmpty())
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
    if (pArgs && SFX_ITEM_SET == pArgs->GetItemState( FN_REDLINE_PROTECT, false, &pItem )
        && ((SfxBoolItem*)pItem)->GetValue() == (aPasswdHash.getLength() != 0))
        return false;
    rPasswordHash = aPasswdHash;
    bRes = true;

    return bRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
