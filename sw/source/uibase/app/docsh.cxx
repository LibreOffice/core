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

#include <config_features.h>

#include <hintids.hxx>
#include <vcl/layout.hxx>
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
#include <docfunc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentState.hxx>
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

SFX_IMPL_SUPERCLASS_INTERFACE(SwDocShell, SfxObjectShell)

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
    if( 0 != ( pMedSet = rMedium.GetItemSet() ) && SfxItemState::SET ==
            pMedSet->GetItemState( FN_API_CALL, true, &pApiItem ) )
            bAPICall = static_cast<const SfxBoolItem*>(pApiItem)->GetValue();

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
                    : new SwReader( rMedium, aFileName, mpDoc );
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
            if(pSet && SfxItemState::SET == pSet->GetItemState(SID_PASSWORD, true, &pPassItem))
                aPasswd = static_cast<const SfxStringItem *>(pPassItem)->GetValue();
        }

        if (!(*ppRdr)->CheckPasswd( aPasswd, *pRead ))
        {
            MessageDialog( 0, SW_RES(STR_ERROR_PASSWD), VCL_MESSAGE_INFO).Execute();
                delete *ppRdr;
            return 0;
        }
    }

    // #i30171# set the UpdateDocMode at the SwDocShell
    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, false);
    mnUpdateDocMode = pUpdateDocItem ? pUpdateDocItem->GetValue() : document::UpdateDocMode::NO_UPDATE;

    if (!pFlt->GetDefaultTemplate().isEmpty())
        pRead->SetTemplateName( pFlt->GetDefaultTemplate() );

    if( pRead == ReadAscii && 0 != rMedium.GetInStream() &&
        pFlt->GetUserData() == FILTER_TEXT_DLG )
    {
        SwAsciiOptions aOpt;
        const SfxItemSet* pSet;
        const SfxPoolItem* pItem;
        if( 0 != ( pSet = rMedium.GetItemSet() ) && SfxItemState::SET ==
            pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
            aOpt.ReadUserData( static_cast<const SfxStringItem*>(pItem)->GetValue() );

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

    mpDoc->setDocAccTitle(OUString());
    SfxViewFrame* pFrame1 = SfxViewFrame::GetFirst( this );
    if (pFrame1)
    {
        vcl::Window* pWindow = &pFrame1->GetWindow();
        if ( pWindow )
        {
            vcl::Window* pSysWin = pWindow->GetSystemWindow();
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

    pRdr->GetDoc()->getIDocumentSettingAccess().set(IDocumentSettingAccess::HTML_MODE, ISA(SwWebDocShell));

    // Restore the pool default if reading a saved document.
    mpDoc->RemoveAllFmtLanguageDependencies();

    sal_uLong nErr = pRdr->Read( *pRead );

    // Maybe put away one old Doc
    if ( mpDoc != pRdr->GetDoc() )
    {
        RemoveLink();
        mpDoc = pRdr->GetDoc();

        AddLink();

        if ( !mxBasePool.is() )
            mxBasePool = new SwDocStyleSheetPool( *mpDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
    }

    UpdateFontList();
    InitDrawModelAndDocShell(this, mpDoc ? mpDoc->getIDocumentDrawModelAccess().GetDrawModel() : 0);

    delete pRdr;

    SW_MOD()->SetEmbeddedLoadSave( false );

    SetError( nErr, OUString(  OSL_LOG_PREFIX  ) );
    bool bOk = !IsError( nErr );

    if ( bOk && !mpDoc->IsInLoadAsynchron() )
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
    if(mpView)
        mpView->GetEditWin().StopQuickHelp();
    SwWait aWait( *this, true );

    CalcLayoutForOLEObjects();  // format for OLE objets
    // #i62875#
    // reset compatibility flag <DoNotCaptureDrawObjsOnPage>, if possible
    if ( mpWrtShell && mpDoc &&
         mpDoc->getIDocumentSettingAccess().get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
         docfunc::AllDrawObjsOnPage( *mpDoc ) )
    {
        mpDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
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
                SwWriter aWrt( *GetMedium(), *mpDoc );
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
                if( mpDoc->ContainsMSVBasic() )
                {
                    if( SvtFilterOptions::Get().IsLoadWordBasicStorage() )
                        nVBWarning = GetSaveWarningOfMSVBAStorage( (SfxObjectShell&) (*this) );
                    mpDoc->SetContainsMSVBasic( false );
                }

                // End TableBox Edit!
                if( mpWrtShell )
                    mpWrtShell->EndAllTblBoxEdit();

                WriterRef xWrt;
                ::GetXMLWriter( aEmptyOUStr, GetMedium()->GetBaseURL( true ), xWrt );

                bool bLockedView(false);
                if ( mpWrtShell )
                {
                    bLockedView = mpWrtShell->IsViewLocked();
                    mpWrtShell->LockView( true );    //lock visible section
                }

                SwWriter aWrt( *GetMedium(), *mpDoc );
                nErr = aWrt.Write( xWrt );

                if ( mpWrtShell )
                    mpWrtShell->LockView( bLockedView );
            }
            break;
        }
        SW_MOD()->SetEmbeddedLoadSave( false );
    }
    SetError( nErr ? nErr : nVBWarning, OUString(  OSL_LOG_PREFIX  ) );

    SfxViewFrame* pFrm = mpWrtShell ? mpWrtShell->GetView().GetViewFrame() : 0;
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
    if(mpView)
        mpView->GetEditWin().StopQuickHelp();

    //#i91811# mod if we have an active margin window, write back the text
    if ( mpView &&
         mpView->GetPostItMgr() &&
         mpView->GetPostItMgr()->HasActiveSidebarWin() )
    {
        mpView->GetPostItMgr()->UpdateDataOnActiveSidebarWin();
    }

    if( mpDoc->getIDocumentSettingAccess().get(IDocumentSettingAccess::GLOBAL_DOCUMENT) &&
        !mpDoc->getIDocumentSettingAccess().get(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS) )
        RemoveOLEObjects();

    {
        // Task 75666 - is the Document imported by our Microsoft-Filters?
        const SfxFilter* pOldFilter = GetMedium()->GetFilter();
        if( pOldFilter &&
            ( pOldFilter->GetUserData() == FILTER_WW8 ||
              pOldFilter->GetUserData() == "CWW6" ||
              pOldFilter->GetUserData() == "WW6" ) )
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
    if ( mpWrtShell &&
         mpDoc->getIDocumentSettingAccess().get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
         docfunc::AllDrawObjsOnPage( *mpDoc ) )
    {
        mpDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
    }

    sal_uLong nErr = ERR_SWG_WRITE_ERROR, nVBWarning = ERRCODE_NONE;
    uno::Reference < embed::XStorage > xStor = rMedium.GetOutputStorage();
    if( SfxObjectShell::SaveAs( rMedium ) )
    {
        if( GetDoc()->getIDocumentSettingAccess().get(IDocumentSettingAccess::GLOBAL_DOCUMENT) && !ISA( SwGlobalDocShell ) )
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

        if( mpDoc->ContainsMSVBasic() )
        {
            if( SvtFilterOptions::Get().IsLoadWordBasicStorage() )
                nVBWarning = GetSaveWarningOfMSVBAStorage( (SfxObjectShell&) *this );
            mpDoc->SetContainsMSVBasic( false );
        }

        // End TableBox Edit!
        if( mpWrtShell )
            mpWrtShell->EndAllTblBoxEdit();

        // Remember and preserve Modified-Flag without calling the Link
        // (for OLE; after Statement from MM)
        bool bIsModified = mpDoc->getIDocumentState().IsModified();
        mpDoc->GetIDocumentUndoRedo().LockUndoNoModifiedPosition();
        Link aOldOLELnk( mpDoc->GetOle2Link() );
        mpDoc->SetOle2Link( Link() );

            // Suppress SfxProgress when we are Embedded
        SW_MOD()->SetEmbeddedLoadSave(
                            SFX_CREATE_MODE_EMBEDDED == GetCreateMode() );

        WriterRef xWrt;
        ::GetXMLWriter( aEmptyOUStr, rMedium.GetBaseURL( true ), xWrt );

        bool bLockedView(false);
        if ( mpWrtShell )
        {
            bLockedView = mpWrtShell->IsViewLocked();
            mpWrtShell->LockView( true );    //lock visible section
        }

        SwWriter aWrt( rMedium, *mpDoc );
        nErr = aWrt.Write( xWrt );

        if (mpWrtShell)
            mpWrtShell->LockView( bLockedView );

        if( bIsModified )
        {
            mpDoc->getIDocumentState().SetModified();
            mpDoc->GetIDocumentUndoRedo().UnLockUndoNoModifiedPosition();
        }
        mpDoc->SetOle2Link( aOldOLELnk );

        SW_MOD()->SetEmbeddedLoadSave( false );

        // Increase RSID
        mpDoc->setRsid( mpDoc->getRsid() );
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
    if(mpView)
        mpView->GetEditWin().StopQuickHelp();

    //#i91811# mod if we have an active margin window, write back the text
    if ( mpView &&
         mpView->GetPostItMgr() &&
         mpView->GetPostItMgr()->HasActiveSidebarWin() )
    {
        mpView->GetPostItMgr()->UpdateDataOnActiveSidebarWin();
    }

    sal_uLong nVBWarning = 0;

    if( mpDoc->ContainsMSVBasic() )
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
                mpDoc->SetContainsMSVBasic( true );
            }
        }
    }

    // End TableBox Edit!
    if( mpWrtShell )
        mpWrtShell->EndAllTblBoxEdit();

    if( pFlt->GetUserData() == "HTML" )
    {
#if HAVE_FEATURE_SCRIPTING
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
    mpDoc->getIDocumentStatistics().UpdateDocStat( false, true );

    CalcLayoutForOLEObjects();  // format for OLE objets
    // #i62875#
    // reset compatibility flag <DoNotCaptureDrawObjsOnPage>, if possible
    if ( mpWrtShell &&
         mpDoc->getIDocumentSettingAccess().get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
         docfunc::AllDrawObjsOnPage( *mpDoc ) )
    {
        mpDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
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
                 SOT_FORMATSTR_ID_STARWRITERGLOB_8_TEMPLATE == nSaveClipId ||
                 SOT_FORMATSTR_ID_STARWRITERGLOB_60 == nSaveClipId ||
                 SOT_FORMATSTR_ID_STARWRITERGLOB_50 == nSaveClipId ||
                 SOT_FORMATSTR_ID_STARWRITERGLOB_40 == nSaveClipId )
            nSaveType = 2;

        // Change Flags of the Document accordingly
        bool bIsHTMLModeSave = GetDoc()->getIDocumentSettingAccess().get(IDocumentSettingAccess::HTML_MODE);
        bool bIsGlobalDocSave = GetDoc()->getIDocumentSettingAccess().get(IDocumentSettingAccess::GLOBAL_DOCUMENT);
        bool bIsGlblDocSaveLinksSave = GetDoc()->getIDocumentSettingAccess().get(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS);
        if( nMyType != nSaveType )
        {
            GetDoc()->getIDocumentSettingAccess().set(IDocumentSettingAccess::HTML_MODE, 1 == nSaveType);
            GetDoc()->getIDocumentSettingAccess().set(IDocumentSettingAccess::GLOBAL_DOCUMENT, 2 == nSaveType);
            if( 2 != nSaveType )
                GetDoc()->getIDocumentSettingAccess().set(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS, false);
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
            GetDoc()->getIDocumentSettingAccess().set(IDocumentSettingAccess::HTML_MODE, bIsHTMLModeSave );
            GetDoc()->getIDocumentSettingAccess().set(IDocumentSettingAccess::GLOBAL_DOCUMENT, bIsGlobalDocSave);
            GetDoc()->getIDocumentSettingAccess().set(IDocumentSettingAccess::GLOBAL_DOCUMENT_SAVE_LINKS, bIsGlblDocSaveLinksSave);
        }

        return bRet;
    }

    if( pFlt->GetUserData() == FILTER_TEXT_DLG &&
        ( mpWrtShell || !::lcl_GetSourceView( this ) ))
    {
        SwAsciiOptions aOpt;
        OUString sItemOpt;
        const SfxItemSet* pSet;
        const SfxPoolItem* pItem;
        if( 0 != ( pSet = rMedium.GetItemSet() ) )
        {
            if( SfxItemState::SET == pSet->GetItemState( SID_FILE_FILTEROPTIONS,
                                                    true, &pItem ) )
                sItemOpt = static_cast<const SfxStringItem*>(pItem)->GetValue();
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
    if ( mpWrtShell && !Application::IsHeadlessModeEnabled() )
    {
        SwWait aWait( *this, true );
        // #i106906#
        const bool bFormerLockView = mpWrtShell->IsViewLocked();
        mpWrtShell->LockView( true );
        mpWrtShell->StartAllAction();
        mpWrtShell->Push();
        SwWriter aWrt( rMedium, *mpWrtShell, true );
        nErrno = aWrt.Write( xWriter, &aFileName );
        //JP 16.05.97: In case the SFX revokes the View while saving
        if( mpWrtShell )
        {
            mpWrtShell->Pop(false);
            mpWrtShell->EndAllAction();
            // #i106906#
            mpWrtShell->LockView( bFormerLockView );
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
            SwWriter aWrt( rMedium, *mpDoc );
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
            mpDoc->getIDocumentState().SetModified();
        else
            mpDoc->getIDocumentState().ResetModified();
    }

    if (mpOLEChildList)
    {
        bool bResetModified = IsEnableSetModified();
        if( bResetModified )
            EnableSetModified( false );

        uno::Sequence < OUString > aNames = mpOLEChildList->GetObjectNames();
        for( sal_Int32 n = aNames.getLength(); n; n-- )
        {
            if ( !mpOLEChildList->MoveEmbeddedObject( aNames[n-1], GetEmbeddedObjectContainer() ) )
            {
                OSL_FAIL("Copying of objects didn't work!" );
            }
        }

        DELETEZ(mpOLEChildList);
        if( bResetModified )
            EnableSetModified( true );
    }
    return bRet;
}

// Draw()-Override for OLE2 (Sfx)
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
        pOrig = const_cast<JobSetup*>(mpDoc->getIDocumentDeviceAccess().getJobsetup());
        if( pOrig )         // then we copy that
            pOrig = new JobSetup( *pOrig );
        mpDoc->getIDocumentDeviceAccess().setJobsetup( rSetup );
    }

    Rectangle aRect( nAspect == ASPECT_THUMBNAIL ?
            GetVisArea( nAspect ) : GetVisArea( ASPECT_CONTENT ) );

    pDev->Push();
    pDev->SetFillColor();
    pDev->SetLineColor();
    pDev->SetBackground();
    const bool bWeb = this->ISA(SwWebDocShell);
    SwPrintData aOpts;
    SwViewShell::PrtOle2( mpDoc, SW_MOD()->GetUsrPref(bWeb), aOpts, pDev, aRect );
    pDev->Pop();

    if( pOrig )
    {
        mpDoc->getIDocumentDeviceAccess().setJobsetup( *pOrig );
        delete pOrig;
    }
    if ( bResetModified )
        EnableSetModified( true );
}

void SwDocShell::SetVisArea( const Rectangle &rRect )
{
    Rectangle aRect( rRect );
    if (mpView)
    {
        Size aSz( mpView->GetDocSz() );
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
        mpView->SetVisArea( aRect, true );
    }
    else
        SfxObjectShell::SetVisArea( aRect );
}

Rectangle SwDocShell::GetVisArea( sal_uInt16 nAspect ) const
{
    if ( nAspect == ASPECT_THUMBNAIL )
    {
        // Preview: set VisArea to the first page.
        SwNodeIndex aIdx( mpDoc->GetNodes().GetEndOfExtras(), 1 );
        SwCntntNode* pNd = mpDoc->GetNodes().GoNext( &aIdx );

        const SwRect aPageRect = pNd->FindPageFrmRect( false, 0, false );
        return aPageRect.SVRect();
    }
    return SfxObjectShell::GetVisArea( nAspect );
}

Printer *SwDocShell::GetDocumentPrinter()
{
    return mpDoc->getIDocumentDeviceAccess().getPrinter( false );
}

OutputDevice* SwDocShell::GetDocumentRefDev()
{
    return mpDoc->getIDocumentDeviceAccess().getReferenceDevice( false );
}

void SwDocShell::OnDocumentPrinterChanged( Printer * pNewPrinter )
{
    if ( pNewPrinter )
        GetDoc()->getIDocumentDeviceAccess().setJobsetup( pNewPrinter->GetJobSetup() );
    else
        GetDoc()->getIDocumentDeviceAccess().setPrinter( 0, true, true );
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
        if ( !GetDoc()->getIDocumentRedlineAccess().GetRedlineTbl().empty() )
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
                         static_cast<SwView*>(pTmpFrm->GetViewShell())->GetWrtShell().GetViewOptions()->getBrowseMode() )
                    {
                        bDisable = true;
                        break;
                    }
                    pTmpFrm = SfxViewFrame::GetNext(*pTmpFrm, this);
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
                bool bState = GetDoc()->getIDocumentSettingAccess().get(IDocumentSettingAccess::BROWSE_MODE);
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
                const SvNumberFormatter* pFmtr = mpDoc->GetNumberFormatter(false);
                rSet.Put( SfxUInt16Item( nWhich,
                        static_cast< sal_uInt16 >(
                        pFmtr ? pFmtr->GetYear2000()
                              : ::utl::MiscCfg().GetYear2000() )));
            }
            break;
        case SID_ATTR_CHAR_FONTLIST:
        {
            rSet.Put( SvxFontListItem( mpFontList, SID_ATTR_CHAR_FONTLIST ) );
        }
        break;
        case SID_MAIL_PREPAREEXPORT:
        {
            //check if linked content or possibly hidden content is available
            //mpDoc->UpdateFlds( NULL, false );
            sfx2::LinkManager& rLnkMgr = mpDoc->getIDocumentLinksAdministration().GetLinkManager();
            const ::sfx2::SvBaseLinks& rLnks = rLnkMgr.GetLinks();
            bool bRet = false;
            if( !rLnks.empty() )
                bRet = true;
            else
            {
                //sections with hidden flag, hidden character attribute, hidden paragraph/text or conditional text fields
                bRet = mpDoc->HasInvisibleContent();
            }
            rSet.Put( SfxBoolItem( nWhich, bRet ) );
        }
        break;

        default: OSL_ENSURE(false,"You cannot get here!");

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
    sal_IntPtr nStatus = reinterpret_cast<sal_IntPtr>(p);
    if( IsEnableSetModified() )
        SetModified( (nStatus & 2) != 0 );
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
    if ( 0 != (mpView = pVw) )
        mpWrtShell = &mpView->GetWrtShell();
    else
        mpWrtShell = 0;
}

void SwDocShell::PrepareReload()
{
    ::DelAllGrfCacheEntries( mpDoc );
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
    const bool bHasDocToStayModified( mpDoc->getIDocumentState().IsModified() && mpDoc->getIDocumentLinksAdministration().LinksUpdated() );

    FinishedLoading( SFX_LOADED_ALL );
    SfxViewFrame* pVFrame = SfxViewFrame::GetFirst(this);
    if(pVFrame)
    {
        SfxViewShell* pShell = pVFrame->GetViewShell();
        if(PTR_CAST(SwSrcView, pShell))
            static_cast<SwSrcView*>(pShell)->Load(this);
    }

    // #i38810#
    if ( bHasDocToStayModified && !mpDoc->getIDocumentState().IsModified() )
    {
        mpDoc->getIDocumentState().SetModified();
    }
}

// a Transfer is cancelled (is called from SFX)
void SwDocShell::CancelTransfers()
{
    // Cancel all links from LinkManager
    aFinishedTimer.Stop();
    mpDoc->getIDocumentLinksAdministration().GetLinkManager().CancelTransfers();
    SfxObjectShell::CancelTransfers();
}

SwFEShell* SwDocShell::GetFEShell()
{
    return mpWrtShell;
}

void SwDocShell::RemoveOLEObjects()
{
    SwIterator<SwCntntNode,SwFmtColl> aIter( *mpDoc->GetDfltGrfFmtColl() );
    for( SwCntntNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
    {
        SwOLENode* pOLENd = pNd->GetOLENode();
        if( pOLENd && ( pOLENd->IsOLEObjectDeleted() ||
                        pOLENd->IsInGlobalDocSection() ) )
        {
            if( !mpOLEChildList )
                mpOLEChildList = new comphelper::EmbeddedObjectContainer;

            OUString aObjName = pOLENd->GetOLEObj().GetCurrentPersistName();
            GetEmbeddedObjectContainer().MoveEmbeddedObject( aObjName, *mpOLEChildList );
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
    if( !mpWrtShell )
        return;

    SwIterator<SwCntntNode,SwFmtColl> aIter( *mpDoc->GetDfltGrfFmtColl() );
    for( SwCntntNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
    {
        SwOLENode* pOLENd = pNd->GetOLENode();
        if( pOLENd && pOLENd->IsOLESizeInvalid() )
        {
            mpWrtShell->CalcLayout();
            break;
        }
    }
}

// #i42634# Overwrites SfxObjectShell::UpdateLinks
// This new function is necessary to trigger update of links in docs
// read by the binary filter:
void SwDocShell::UpdateLinks()
{
    GetDoc()->getIDocumentLinksAdministration().UpdateLinks(true);
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
    if (mpDoc)
    {
        mpDoc->setDocAccTitle( rTitle );
    }
}

const OUString SwDocShell::getDocAccTitle() const
{
    OUString sRet;
    if (mpDoc)
    {
        sRet = mpDoc->getDocAccTitle();
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
    return mpDoc ? &mpDoc->GetXmlIdRegistry() : 0;
}

bool SwDocShell::IsChangeRecording() const
{
    return (mpWrtShell->GetRedlineMode() & nsRedlineMode_t::REDLINE_ON) != 0;
}

bool SwDocShell::HasChangeRecordProtection() const
{
    return mpWrtShell->getIDocumentRedlineAccess()->GetRedlinePassword().getLength() > 0;
}

void SwDocShell::SetChangeRecording( bool bActivate )
{
    sal_uInt16 nOn = bActivate ? nsRedlineMode_t::REDLINE_ON : 0;
    sal_uInt16 nMode = mpWrtShell->GetRedlineMode();
    mpWrtShell->SetRedlineModeAndCheckInsMode( (nMode & ~nsRedlineMode_t::REDLINE_ON) | nOn);
}

bool SwDocShell::SetProtectionPassword( const OUString &rNewPassword )
{
    const SfxAllItemSet aSet( GetPool() );
    const SfxItemSet*   pArgs = &aSet;
    const SfxPoolItem*  pItem = NULL;

    IDocumentRedlineAccess* pIDRA = mpWrtShell->getIDocumentRedlineAccess();
    Sequence< sal_Int8 > aPasswd = pIDRA->GetRedlinePassword();
    if (pArgs && SfxItemState::SET == pArgs->GetItemState( FN_REDLINE_PROTECT, false, &pItem )
        && static_cast<const SfxBoolItem*>(pItem)->GetValue() == (aPasswd.getLength() > 0))
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

    IDocumentRedlineAccess* pIDRA = mpWrtShell->getIDocumentRedlineAccess();
    Sequence< sal_Int8 > aPasswdHash( pIDRA->GetRedlinePassword() );
    if (pArgs && SfxItemState::SET == pArgs->GetItemState( FN_REDLINE_PROTECT, false, &pItem )
        && static_cast<const SfxBoolItem*>(pItem)->GetValue() == (aPasswdHash.getLength() != 0))
        return false;
    rPasswordHash = aPasswdHash;
    bRes = true;

    return bRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
