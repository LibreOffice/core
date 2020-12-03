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

#include <officecfg/Office/Common.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <vcl/jobset.hxx>
#include <svl/whiter.hxx>
#include <svl/zforlist.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/PasswordHelper.hxx>
#include <unotools/moduleoptions.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/flstitem.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/classids.hxx>
#include <basic/sbmod.hxx>
#include <node.hxx>
#include <swwait.hxx>
#include <printdata.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <PostItMgr.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
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
#include <pview.hxx>
#include <srcview.hxx>
#include <ndindex.hxx>
#include <ndole.hxx>
#include <txtftn.hxx>
#include <ftnidx.hxx>
#include <fldbas.hxx>
#include <docary.hxx>
#include <swerror.h>
#include <cmdid.h>
#include <strings.hrc>

#include <unotools/fltrcfg.hxx>
#include <svtools/htmlcfg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>

#define ShellClass_SwDocShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>

#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/VndSunStarPkgUrlReferenceFactory.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <ooo/vba/XSinkCaller.hpp>

#include <unotextrange.hxx>

#include <dbmgr.hxx>
#include <iodetect.hxx>

#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::container;

SFX_IMPL_SUPERCLASS_INTERFACE(SwDocShell, SfxObjectShell)

void SwDocShell::InitInterface_Impl()
{
}


SFX_IMPL_OBJECTFACTORY(SwDocShell, SvGlobalName(SO3_SW_CLASSID), "swriter"  )

bool SwDocShell::InsertGeneratedStream(SfxMedium & rMedium,
        uno::Reference<text::XTextRange> const& xInsertPosition)
{
    SwUnoInternalPaM aPam(*GetDoc()); // must have doc since called from SwView
    if (!::sw::XTextRangeToSwPaM(aPam, xInsertPosition))
        return false;
    // similar to SwView::InsertMedium
    SwReaderPtr pReader;
    Reader *const pRead = StartConvertFrom(rMedium, pReader, nullptr, &aPam);
    if (!pRead)
        return false;
    ErrCode const nError = pReader->Read(*pRead);
    return ERRCODE_NONE == nError;
}

// Prepare loading
Reader* SwDocShell::StartConvertFrom(SfxMedium& rMedium, SwReaderPtr& rpRdr,
                                    SwCursorShell const *pCursorShell,
                                    SwPaM* pPaM )
{
    bool bAPICall = false;
    const SfxPoolItem* pApiItem;
    const SfxItemSet* pMedSet;
    if( nullptr != ( pMedSet = rMedium.GetItemSet() ) && SfxItemState::SET ==
            pMedSet->GetItemState( FN_API_CALL, true, &pApiItem ) )
            bAPICall = static_cast<const SfxBoolItem*>(pApiItem)->GetValue();

    std::shared_ptr<const SfxFilter> pFlt = rMedium.GetFilter();
    if( !pFlt )
    {
        if(!bAPICall)
        {
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(nullptr,
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          SwResId(STR_CANTOPEN)));
            xInfoBox->run();
        }
        return nullptr;
    }
    OUString aFileName( rMedium.GetName() );
    Reader* pRead = SwReaderWriter::GetReader( pFlt->GetUserData() );
    if( !pRead )
        return nullptr;

    if( rMedium.IsStorage()
        ? SwReaderType::Storage & pRead->GetReaderType()
        : SwReaderType::Stream & pRead->GetReaderType() )
    {
        if (pPaM)
            rpRdr.reset(new SwReader( rMedium, aFileName, *pPaM ));
        else if (pCursorShell)
            rpRdr.reset(new SwReader( rMedium, aFileName, *pCursorShell->GetCursor() ));
        else
            rpRdr.reset(new SwReader( rMedium, aFileName, m_xDoc.get() ));
    }
    else
        return nullptr;

    // #i30171# set the UpdateDocMode at the SwDocShell
    const SfxUInt16Item* pUpdateDocItem = SfxItemSet::GetItem<SfxUInt16Item>(rMedium.GetItemSet(), SID_UPDATEDOCMODE, false);
    m_nUpdateDocMode = pUpdateDocItem ? pUpdateDocItem->GetValue() : document::UpdateDocMode::NO_UPDATE;

    if (!pFlt->GetDefaultTemplate().isEmpty())
        pRead->SetTemplateName( pFlt->GetDefaultTemplate() );

    if( pRead == ReadAscii && nullptr != rMedium.GetInStream() &&
        pFlt->GetUserData() == FILTER_TEXT_DLG )
    {
        SwAsciiOptions aOpt;
        const SfxItemSet* pSet;
        const SfxPoolItem* pItem;
        if( nullptr != ( pSet = rMedium.GetItemSet() ) && SfxItemState::SET ==
            pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
            aOpt.ReadUserData( static_cast<const SfxStringItem*>(pItem)->GetValue() );

        pRead->GetReaderOpt().SetASCIIOpts( aOpt );
    }

    return pRead;
}

// Loading
bool SwDocShell::ConvertFrom( SfxMedium& rMedium )
{
    SwReaderPtr pRdr;
    Reader* pRead = StartConvertFrom(rMedium, pRdr);
    if (!pRead)
      return false; // #129881# return if no reader is found
    tools::SvRef<SotStorage> pStg=pRead->getSotStorageRef(); // #i45333# save sot storage ref in case of recursive calls

    m_xDoc->setDocAccTitle(OUString());
    if (const auto pFrame1 = SfxViewFrame::GetFirst(this))
    {
        if (auto pSysWin = pFrame1->GetWindow().GetSystemWindow())
        {
            pSysWin->SetAccessibleName(OUString());
        }
    }
    SwWait aWait( *this, true );

        // Suppress SfxProgress, when we are Embedded
    SW_MOD()->SetEmbeddedLoadSave(
                            SfxObjectCreateMode::EMBEDDED == GetCreateMode() );

    pRdr->GetDoc().getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, dynamic_cast< const SwWebDocShell *>( this ) !=  nullptr);

    // Restore the pool default if reading a saved document.
    m_xDoc->RemoveAllFormatLanguageDependencies();

    ErrCode nErr = pRdr->Read( *pRead );

    // Maybe put away one old Doc
    if (m_xDoc.get() != &pRdr->GetDoc())
    {
        RemoveLink();
        m_xDoc = &pRdr->GetDoc();

        AddLink();

        if (!m_xBasePool.is())
            m_xBasePool = new SwDocStyleSheetPool( *m_xDoc, SfxObjectCreateMode::ORGANIZER == GetCreateMode() );
    }

    UpdateFontList();
    InitDrawModelAndDocShell(this, m_xDoc ? m_xDoc->getIDocumentDrawModelAccess().GetDrawModel() : nullptr);

    pRdr.reset();

    SW_MOD()->SetEmbeddedLoadSave( false );

    SetError(nErr);
    bool bOk = !nErr.IsError();

    if (bOk && !m_xDoc->IsInLoadAsynchron())
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
    if (m_pView)
        m_pView->GetEditWin().StopQuickHelp();
    SwWait aWait( *this, true );

    CalcLayoutForOLEObjects();  // format for OLE objects
    // #i62875#
    // reset compatibility flag <DoNotCaptureDrawObjsOnPage>, if possible
    if (m_pWrtShell && m_xDoc &&
        m_xDoc->getIDocumentSettingAccess().get(DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
        docfunc::AllDrawObjsOnPage(*m_xDoc))
    {
        m_xDoc->getIDocumentSettingAccess().set(DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
    }

    ErrCode nErr = ERR_SWG_WRITE_ERROR, nVBWarning = ERRCODE_NONE;
    if( SfxObjectShell::Save() )
    {
        switch( GetCreateMode() )
        {
        case SfxObjectCreateMode::INTERNAL:
            nErr = ERRCODE_NONE;
            break;

        case SfxObjectCreateMode::ORGANIZER:
            {
                WriterRef xWrt;
                ::GetXMLWriter(OUString(), GetMedium()->GetBaseURL(true), xWrt);
                xWrt->SetOrganizerMode( true );
                SwWriter aWrt( *GetMedium(), *m_xDoc );
                nErr = aWrt.Write( xWrt );
                xWrt->SetOrganizerMode( false );
            }
            break;

        case SfxObjectCreateMode::EMBEDDED:
            // Suppress SfxProgress, if we are Embedded
            SW_MOD()->SetEmbeddedLoadSave( true );
            [[fallthrough]];

        case SfxObjectCreateMode::STANDARD:
        default:
            {
                if (m_xDoc->ContainsMSVBasic())
                {
                    if( SvtFilterOptions::Get().IsLoadWordBasicStorage() )
                        nVBWarning = GetSaveWarningOfMSVBAStorage( static_cast<SfxObjectShell&>(*this) );
                    m_xDoc->SetContainsMSVBasic( false );
                }

                // End TableBox Edit!
                if (m_pWrtShell)
                    m_pWrtShell->EndAllTableBoxEdit();

                WriterRef xWrt;
                ::GetXMLWriter(OUString(), GetMedium()->GetBaseURL(true), xWrt);

                bool bLockedView(false);
                if (m_pWrtShell)
                {
                    bLockedView = m_pWrtShell->IsViewLocked();
                    m_pWrtShell->LockView( true );    //lock visible section
                }

                SwWriter aWrt( *GetMedium(), *m_xDoc );
                nErr = aWrt.Write( xWrt );

                if (m_pWrtShell)
                    m_pWrtShell->LockView( bLockedView );
            }
            break;
        }
        SW_MOD()->SetEmbeddedLoadSave( false );
    }
    SetError(nErr ? nErr : nVBWarning);

    SfxViewFrame *const pFrame =
        m_pWrtShell ? m_pWrtShell->GetView().GetViewFrame() : nullptr;
    if( pFrame )
    {
        pFrame->GetBindings().SetState(SfxBoolItem(SID_DOC_MODIFIED, false));
    }
    return !nErr.IsError();
}

SwDocShell::LockAllViewsGuard_Impl::LockAllViewsGuard_Impl(SwViewShell* pViewShell)
{
    if (!pViewShell)
        return;
    for (SwViewShell& rShell : pViewShell->GetRingContainer())
    {
        if (!rShell.IsViewLocked())
        {
            m_aViewWasUnLocked.push_back(&rShell);
            rShell.LockView(true);
        }
    }
}

SwDocShell::LockAllViewsGuard_Impl::~LockAllViewsGuard_Impl()
{
    for (SwViewShell* pShell : m_aViewWasUnLocked)
        pShell->LockView(false);
}

std::unique_ptr<SfxObjectShell::LockAllViewsGuard> SwDocShell::LockAllViews()
{
    return std::make_unique<LockAllViewsGuard_Impl>(GetEditShell());
}

// Save using the Defaultformat
bool SwDocShell::SaveAs( SfxMedium& rMedium )
{
    SwWait aWait( *this, true );
    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    if (m_pView)
        m_pView->GetEditWin().StopQuickHelp();

    //#i91811# mod if we have an active margin window, write back the text
    if (m_pView &&
        m_pView->GetPostItMgr() &&
        m_pView->GetPostItMgr()->HasActiveSidebarWin())
    {
        m_pView->GetPostItMgr()->UpdateDataOnActiveSidebarWin();
    }

    if (m_xDoc->getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT) &&
        !m_xDoc->getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT_SAVE_LINKS))
        RemoveOLEObjects();

    if (GetMedium())
    {
        // Task 75666 - is the Document imported by our Microsoft-Filters?
        std::shared_ptr<const SfxFilter> pOldFilter = GetMedium()->GetFilter();
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

    CalcLayoutForOLEObjects();  // format for OLE objects

    const bool bURLChanged = GetMedium() && GetMedium()->GetURLObject() != rMedium.GetURLObject();
    const SwDBManager* const pMgr = m_xDoc->GetDBManager();
    const bool bHasEmbedded = pMgr && !pMgr->getEmbeddedName().isEmpty();
    bool bSaveDS = bHasEmbedded && bURLChanged;
    if (bSaveDS)
    {
        // Don't save data source in case a temporary is being saved for preview in MM wizard
        if (const SfxBoolItem* pNoEmbDS
            = SfxItemSet::GetItem(rMedium.GetItemSet(), SID_NO_EMBEDDED_DS, false))
            bSaveDS = !pNoEmbDS->GetValue();
    }
    if (bSaveDS)
    {
        // We have an embedded data source definition, need to re-store it,
        // otherwise relative references will break when the new file is in a
        // different directory.

        OUString aURL(GetMedium()->GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::NONE));
        if (aURL.isEmpty())
        {
            // No old URL - is this a new document created from a template with embedded DS?
            // Try to get the template URL to reconstruct the embedded data source URL
            const css::beans::PropertyValues& rArgs = GetMedium()->GetArgs();
            const auto aURLIter = std::find_if(rArgs.begin(), rArgs.end(),
                                               [](const auto& v) { return v.Name == "URL"; });
            if (aURLIter != rArgs.end())
                aURLIter->Value >>= aURL;
        }

        if (!aURL.isEmpty())
        {
            auto xContext(comphelper::getProcessComponentContext());
            auto xUri = css::uri::UriReferenceFactory::create(xContext)->parse(aURL);
            assert(xUri.is());
            xUri = css::uri::VndSunStarPkgUrlReferenceFactory::create(xContext)
                       ->createVndSunStarPkgUrlReference(xUri);
            assert(xUri.is());
            aURL = xUri->getUriReference() + "/"
                   + INetURLObject::encode(pMgr->getEmbeddedName(), INetURLObject::PART_FPATH,
                                           INetURLObject::EncodeMechanism::All);

            bool bCopyTo = GetCreateMode() == SfxObjectCreateMode::EMBEDDED;
            if (!bCopyTo)
            {
                if (const SfxBoolItem* pSaveToItem
                    = SfxItemSet::GetItem(rMedium.GetItemSet(), SID_SAVETO, false))
                    bCopyTo = pSaveToItem->GetValue();
            }

            auto xDatabaseContext = sdb::DatabaseContext::create(xContext);
            uno::Reference<sdb::XDocumentDataSource> xDataSource(xDatabaseContext->getByName(aURL),
                                                                 uno::UNO_QUERY);
            if (xDataSource)
            {
                uno::Reference<frame::XStorable> xStorable(xDataSource->getDatabaseDocument(),
                                                           uno::UNO_QUERY);
                SwDBManager::StoreEmbeddedDataSource(xStorable, rMedium.GetOutputStorage(),
                                                     pMgr->getEmbeddedName(), rMedium.GetName(),
                                                     bCopyTo);
            }
        }
    }

    // #i62875#
    // reset compatibility flag <DoNotCaptureDrawObjsOnPage>, if possible
    if (m_pWrtShell &&
        m_xDoc->getIDocumentSettingAccess().get(DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
        docfunc::AllDrawObjsOnPage(*m_xDoc))
    {
        m_xDoc->getIDocumentSettingAccess().set(DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
    }

    ErrCode nErr = ERR_SWG_WRITE_ERROR, nVBWarning = ERRCODE_NONE;
    uno::Reference < embed::XStorage > xStor = rMedium.GetOutputStorage();
    if( SfxObjectShell::SaveAs( rMedium ) )
    {
        if( GetDoc()->getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT) && dynamic_cast< const SwGlobalDocShell *>( this ) ==  nullptr )
        {
            // The document is closed explicitly, but using SfxObjectShellLock is still more correct here
            SfxObjectShellLock xDocSh =
                new SwGlobalDocShell( SfxObjectCreateMode::INTERNAL );
            // the global document can not be a template
            xDocSh->SetupStorage( xStor, SotStorage::GetVersion( xStor ), false );
            xDocSh->DoClose();
        }

        if (m_xDoc->ContainsMSVBasic())
        {
            if( SvtFilterOptions::Get().IsLoadWordBasicStorage() )
                nVBWarning = GetSaveWarningOfMSVBAStorage( static_cast<SfxObjectShell&>(*this) );
            m_xDoc->SetContainsMSVBasic( false );
        }

        if (m_pWrtShell)
        {
            // End TableBox Edit!
            m_pWrtShell->EndAllTableBoxEdit();

            // Remove invalid signatures.
            m_pWrtShell->ValidateAllParagraphSignatures(false);

            m_pWrtShell->ClassifyDocPerHighestParagraphClass();
        }

        // Remember and preserve Modified-Flag without calling the Link
        // (for OLE; after Statement from MM)
        const bool bIsModified = m_xDoc->getIDocumentState().IsModified();
        m_xDoc->GetIDocumentUndoRedo().LockUndoNoModifiedPosition();
        Link<bool,void> aOldOLELnk( m_xDoc->GetOle2Link() );
        m_xDoc->SetOle2Link( Link<bool,void>() );

            // Suppress SfxProgress when we are Embedded
        SW_MOD()->SetEmbeddedLoadSave(
                            SfxObjectCreateMode::EMBEDDED == GetCreateMode() );

        WriterRef xWrt;
        ::GetXMLWriter(OUString(), rMedium.GetBaseURL(true), xWrt);

        bool bLockedView(false);
        if (m_pWrtShell)
        {
            bLockedView = m_pWrtShell->IsViewLocked();
            m_pWrtShell->LockView( true );    //lock visible section
        }

        SwWriter aWrt( rMedium, *m_xDoc );
        nErr = aWrt.Write( xWrt );

        if (m_pWrtShell)
            m_pWrtShell->LockView( bLockedView );

        if( bIsModified )
        {
            m_xDoc->getIDocumentState().SetModified();
            m_xDoc->GetIDocumentUndoRedo().UnLockUndoNoModifiedPosition();
        }
        m_xDoc->SetOle2Link( aOldOLELnk );

        SW_MOD()->SetEmbeddedLoadSave( false );

        // Increase RSID
        m_xDoc->setRsid( m_xDoc->getRsid() );

        m_xDoc->cleanupUnoCursorTable();
    }
    SetError(nErr ? nErr : nVBWarning);

    return !nErr.IsError();
}

// Save all Formats
static SwSrcView* lcl_GetSourceView( SwDocShell const * pSh )
{
    // are we in SourceView?
    SfxViewFrame* pVFrame = SfxViewFrame::GetFirst( pSh );
    SfxViewShell* pViewShell = pVFrame ? pVFrame->GetViewShell() : nullptr;
    return dynamic_cast<SwSrcView*>( pViewShell );
}

bool SwDocShell::ConvertTo( SfxMedium& rMedium )
{
    std::shared_ptr<const SfxFilter> pFlt = rMedium.GetFilter();
    if( !pFlt )
        return false;

    WriterRef xWriter;
    SwReaderWriter::GetWriter( pFlt->GetUserData(), rMedium.GetBaseURL( true ), xWriter );
    if( !xWriter.is() )
    {   // Filter not available
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(nullptr,
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      SwResId(STR_DLLNOTFOUND)));
        xInfoBox->run();
        return false;
    }

    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    if (m_pView)
        m_pView->GetEditWin().StopQuickHelp();

    //#i91811# mod if we have an active margin window, write back the text
    if (m_pView &&
        m_pView->GetPostItMgr() &&
        m_pView->GetPostItMgr()->HasActiveSidebarWin())
    {
        m_pView->GetPostItMgr()->UpdateDataOnActiveSidebarWin();
    }

    ErrCode nVBWarning = ERRCODE_NONE;

    if (m_xDoc->ContainsMSVBasic())
    {
        bool bSave = pFlt->GetUserData() == "CWW8"
             && SvtFilterOptions::Get().IsLoadWordBasicStorage();

        if ( bSave )
        {
            tools::SvRef<SotStorage> xStg = new SotStorage( rMedium.GetOutStream(), false );
            OSL_ENSURE( !xStg->GetError(), "No storage available for storing VBA macros!" );
            if ( !xStg->GetError() )
            {
                nVBWarning = SaveOrDelMSVBAStorage( static_cast<SfxObjectShell&>(*this), *xStg, bSave, "Macros" );
                xStg->Commit();
                m_xDoc->SetContainsMSVBasic( true );
            }
        }
    }

    // End TableBox Edit!
    if (m_pWrtShell)
        m_pWrtShell->EndAllTableBoxEdit();

    if( pFlt->GetUserData() == "HTML" )
    {
#if HAVE_FEATURE_SCRIPTING
        SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
        if( !rHtmlOpt.IsStarBasic() && rHtmlOpt.IsStarBasicWarning() && HasBasic() )
        {
            uno::Reference< XLibraryContainer > xLibCont = GetBasicContainer();
            uno::Reference< XNameAccess > xLib;
            const Sequence<OUString> aNames = xLibCont->getElementNames();
            for(const OUString& rName : aNames)
            {
                Any aLib = xLibCont->getByName(rName);
                aLib >>= xLib;
                if(xLib.is())
                {
                    Sequence<OUString> aModNames = xLib->getElementNames();
                    if(aModNames.hasElements())
                    {
                        SetError(WARN_SWG_HTML_NO_MACROS);
                        break;
                    }
                }
            }
        }
#endif
    }

    // #i76360# Update document statistics
    if ( !rMedium.IsSkipImages() )
        m_xDoc->getIDocumentStatistics().UpdateDocStat( false, true );

    CalcLayoutForOLEObjects();  // format for OLE objects
    // #i62875#
    // reset compatibility flag <DoNotCaptureDrawObjsOnPage>, if possible
    if (m_pWrtShell &&
        m_xDoc->getIDocumentSettingAccess().get(DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
        docfunc::AllDrawObjsOnPage(*m_xDoc))
    {
        m_xDoc->getIDocumentSettingAccess().set(DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
    }

    if( xWriter->IsStgWriter() &&
        ( pFlt->GetUserData() == FILTER_XML ||
          pFlt->GetUserData() == FILTER_XMLV ||
          pFlt->GetUserData() == FILTER_XMLVW ) )
    {
        // determine the own Type
        sal_uInt8 nMyType = 0;
        if( dynamic_cast< const SwWebDocShell *>( this ) !=  nullptr )
            nMyType = 1;
        else if( dynamic_cast< const SwGlobalDocShell *>( this ) !=  nullptr )
            nMyType = 2;

        // determine the desired Type
        sal_uInt8 nSaveType = 0;
        SotClipboardFormatId nSaveClipId = pFlt->GetFormat();
        if( SotClipboardFormatId::STARWRITERWEB_8 == nSaveClipId ||
            SotClipboardFormatId::STARWRITERWEB_60 == nSaveClipId ||
            SotClipboardFormatId::STARWRITERWEB_50 == nSaveClipId ||
            SotClipboardFormatId::STARWRITERWEB_40 == nSaveClipId )
            nSaveType = 1;
        else if( SotClipboardFormatId::STARWRITERGLOB_8 == nSaveClipId ||
                 SotClipboardFormatId::STARWRITERGLOB_8_TEMPLATE == nSaveClipId ||
                 SotClipboardFormatId::STARWRITERGLOB_60 == nSaveClipId ||
                 SotClipboardFormatId::STARWRITERGLOB_50 == nSaveClipId ||
                 SotClipboardFormatId::STARWRITERGLOB_40 == nSaveClipId )
            nSaveType = 2;

        // Change Flags of the Document accordingly
        bool bIsHTMLModeSave = GetDoc()->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE);
        bool bIsGlobalDocSave = GetDoc()->getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT);
        bool bIsGlblDocSaveLinksSave = GetDoc()->getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT_SAVE_LINKS);
        if( nMyType != nSaveType )
        {
            GetDoc()->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, 1 == nSaveType);
            GetDoc()->getIDocumentSettingAccess().set(DocumentSettingId::GLOBAL_DOCUMENT, 2 == nSaveType);
            if( 2 != nSaveType )
                GetDoc()->getIDocumentSettingAccess().set(DocumentSettingId::GLOBAL_DOCUMENT_SAVE_LINKS, false);
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
                    xSet->setPropertyValue("MediaType", uno::makeAny( SotExchange::GetFormatMimeType( nSaveClipId ) ) );
            }
            catch (const uno::Exception&)
            {
            }
        }

        // Now normally save the Document
        bool bRet = SaveAs( rMedium );

        if( nMyType != nSaveType )
        {
            GetDoc()->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, bIsHTMLModeSave );
            GetDoc()->getIDocumentSettingAccess().set(DocumentSettingId::GLOBAL_DOCUMENT, bIsGlobalDocSave);
            GetDoc()->getIDocumentSettingAccess().set(DocumentSettingId::GLOBAL_DOCUMENT_SAVE_LINKS, bIsGlblDocSaveLinksSave);
        }

        return bRet;
    }

    if( pFlt->GetUserData() == FILTER_TEXT_DLG &&
        (m_pWrtShell || !::lcl_GetSourceView(this)))
    {
        SwAsciiOptions aOpt;
        OUString sItemOpt;
        const SfxItemSet* pSet = rMedium.GetItemSet();
        if( nullptr != pSet )
        {
            const SfxPoolItem* pItem;
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
                            SfxObjectCreateMode::EMBEDDED == GetCreateMode());

    // Span Context in order to suppress the Selection's View
    ErrCode nErrno;
    const OUString aFileName( rMedium.GetName() );

    // No View, so the whole Document!
    if (m_pWrtShell && !Application::IsHeadlessModeEnabled())
    {
        SwWait aWait( *this, true );
        // #i106906#
        const bool bFormerLockView = m_pWrtShell->IsViewLocked();
        m_pWrtShell->LockView( true );
        m_pWrtShell->StartAllAction();
        m_pWrtShell->Push();
        SwWriter aWrt( rMedium, *m_pWrtShell, true );
        nErrno = aWrt.Write( xWriter, &aFileName );
        //JP 16.05.97: In case the SFX revokes the View while saving
        if (m_pWrtShell)
        {
            m_pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
            m_pWrtShell->EndAllAction();
            // #i106906#
            m_pWrtShell->LockView( bFormerLockView );
        }
    }
    else
    {
        // are we in SourceView?
        SwSrcView* pSrcView = ::lcl_GetSourceView( this );
        if( pSrcView )
        {
            pSrcView->SaveContentTo(rMedium);
            nErrno = ERRCODE_NONE;
        }
        else
        {
            SwWriter aWrt( rMedium, *m_xDoc );
            nErrno = aWrt.Write( xWriter, &aFileName );
        }
    }

    SW_MOD()->SetEmbeddedLoadSave( false );
    SetError(nErrno ? nErrno : nVBWarning);
    if( !rMedium.IsStorage() )
        rMedium.CloseOutStream();

    return ! nErrno.IsError();
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
            m_xDoc->getIDocumentState().SetModified();
        else
            m_xDoc->getIDocumentState().ResetModified();
    }

    if (m_pOLEChildList)
    {
        bool bResetModified = IsEnableSetModified();
        if( bResetModified )
            EnableSetModified( false );

        uno::Sequence < OUString > aNames = m_pOLEChildList->GetObjectNames();
        for( sal_Int32 n = aNames.getLength(); n; n-- )
        {
            if (!m_pOLEChildList->MoveEmbeddedObject(aNames[n-1], GetEmbeddedObjectContainer()))
            {
                OSL_FAIL("Copying of objects didn't work!" );
            }
        }

        m_pOLEChildList.reset();
        if( bResetModified )
            EnableSetModified();
    }
    return bRet;
}

// Draw()-Override for OLE2 (Sfx)
void SwDocShell::Draw( OutputDevice* pDev, const JobSetup& rSetup,
                               sal_uInt16 nAspect )
{
    //fix #25341# Draw should not affect the Modified
    bool bResetModified = IsEnableSetModified();
    if ( bResetModified )
        EnableSetModified( false );

    // When there is a JobSetup connected to the Document, we copy it to
    // reconnect it after PrtOle2. We don't use an empty JobSetup because
    // that would only lead to questionable results after expensive
    // reformatting (Preview!)
    std::unique_ptr<JobSetup> pOrig;
    if ( !rSetup.GetPrinterName().isEmpty() && ASPECT_THUMBNAIL != nAspect )
    {
        const JobSetup* pCurrentJobSetup = m_xDoc->getIDocumentDeviceAccess().getJobsetup();
        if( pCurrentJobSetup )         // then we copy that
            pOrig.reset(new JobSetup( *pCurrentJobSetup ));
        m_xDoc->getIDocumentDeviceAccess().setJobsetup( rSetup );
    }

    tools::Rectangle aRect( nAspect == ASPECT_THUMBNAIL ?
            GetVisArea( nAspect ) : GetVisArea( ASPECT_CONTENT ) );

    pDev->Push();
    pDev->SetFillColor();
    pDev->SetLineColor();
    pDev->SetBackground();
    const bool bWeb = dynamic_cast< const SwWebDocShell *>( this ) !=  nullptr;
    SwPrintData aOpts;
    SwViewShell::PrtOle2(m_xDoc.get(), SW_MOD()->GetUsrPref(bWeb), aOpts, *pDev, aRect);
    pDev->Pop();

    if( pOrig )
    {
        m_xDoc->getIDocumentDeviceAccess().setJobsetup( *pOrig );
    }
    if ( bResetModified )
        EnableSetModified();
}

void SwDocShell::SetVisArea( const tools::Rectangle &rRect )
{
    tools::Rectangle aRect( rRect );
    if (m_pView)
    {
        Size aSz( m_pView->GetDocSz() );
        aSz.AdjustWidth(DOCUMENTBORDER ); aSz.AdjustHeight(DOCUMENTBORDER );
        tools::Long nMoveX = 0, nMoveY = 0;
        if ( aRect.Right() > aSz.Width() )
            nMoveX = aSz.Width() - aRect.Right();
        if ( aRect.Bottom() > aSz.Height() )
            nMoveY = aSz.Height() - aRect.Bottom();
        aRect.Move( nMoveX, nMoveY );
        nMoveX = aRect.Left() < 0 ? -aRect.Left() : 0;
        nMoveY = aRect.Top()  < 0 ? -aRect.Top()  : 0;
        aRect.Move( nMoveX, nMoveY );

        // Calls SfxInPlaceObject::SetVisArea()!
        m_pView->SetVisArea( aRect );
    }
    else
        SfxObjectShell::SetVisArea( aRect );
}

tools::Rectangle SwDocShell::GetVisArea( sal_uInt16 nAspect ) const
{
    if ( nAspect == ASPECT_THUMBNAIL )
    {
        // Preview: set VisArea to the first page.
        SwNodeIndex aIdx( m_xDoc->GetNodes().GetEndOfExtras(), 1 );
        SwContentNode* pNd = m_xDoc->GetNodes().GoNext( &aIdx );

        const SwRect aPageRect = pNd->FindPageFrameRect();
        if (aPageRect.IsEmpty())
            return tools::Rectangle();
        tools::Rectangle aRect(aPageRect.SVRect());

        // tdf#81219 sanitize - nobody is interested in a thumbnail where's
        // nothing visible
        if (aRect.GetHeight() > 2*aRect.GetWidth())
            aRect.SetSize(Size(aRect.GetWidth(), 2*aRect.GetWidth()));
        else if (aRect.GetWidth() > 2*aRect.GetHeight())
            aRect.SetSize(Size(2*aRect.GetHeight(), aRect.GetHeight()));

        return aRect;
    }
    return SfxObjectShell::GetVisArea( nAspect );
}

Printer *SwDocShell::GetDocumentPrinter()
{
    return m_xDoc->getIDocumentDeviceAccess().getPrinter( false );
}

OutputDevice* SwDocShell::GetDocumentRefDev()
{
    return m_xDoc->getIDocumentDeviceAccess().getReferenceDevice( false );
}

void SwDocShell::OnDocumentPrinterChanged( Printer * pNewPrinter )
{
    if ( pNewPrinter )
        GetDoc()->getIDocumentDeviceAccess().setJobsetup( pNewPrinter->GetJobSetup() );
    else
        GetDoc()->getIDocumentDeviceAccess().setPrinter( nullptr, true, true );
}

// #i20883# Digital Signatures and Encryption
HiddenInformation SwDocShell::GetHiddenInformationState( HiddenInformation nStates )
{
    // get global state like HiddenInformation::DOCUMENTVERSIONS
    HiddenInformation nState = SfxObjectShell::GetHiddenInformationState( nStates );

    if ( nStates & HiddenInformation::RECORDEDCHANGES )
    {
        if ( !GetDoc()->getIDocumentRedlineAccess().GetRedlineTable().empty() )
            nState |= HiddenInformation::RECORDEDCHANGES;
    }
    if ( nStates & HiddenInformation::NOTES )
    {
        OSL_ENSURE( GetWrtShell(), "No SwWrtShell, no information" );
        if(GetWrtShell() && GetWrtShell()->GetFieldType(SwFieldIds::Postit, OUString())->HasHiddenInformationNotes())
                nState |= HiddenInformation::NOTES;
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
                SfxViewFrame *pTmpFrame = SfxViewFrame::GetFirst(this);
                while (pTmpFrame)     // Look for Preview
                {
                    if ( dynamic_cast<SwView*>( pTmpFrame->GetViewShell() ) &&
                         static_cast<SwView*>(pTmpFrame->GetViewShell())->GetWrtShell().GetViewOptions()->getBrowseMode() )
                    {
                        bDisable = true;
                        break;
                    }
                    pTmpFrame = SfxViewFrame::GetNext(*pTmpFrame, this);
                }
            }
            // End of disabled "multiple layout"
            if ( bDisable )
                rSet.DisableItem( SID_PRINTPREVIEW );
            else
            {
                SfxBoolItem aBool( SID_PRINTPREVIEW, false );
                if( dynamic_cast<SwPagePreview*>( SfxViewShell::Current())  )
                    aBool.SetValue( true );
                rSet.Put( aBool );
            }
        }
        break;
        case SID_AUTO_CORRECT_DLG:
            if ( comphelper::LibreOfficeKit::isActive() )
                rSet.DisableItem( SID_AUTO_CORRECT_DLG );
        break;
        case SID_SOURCEVIEW:
        {
            SfxViewShell* pCurrView = GetView() ? static_cast<SfxViewShell*>(GetView())
                                        : SfxViewShell::Current();
            bool bSourceView = dynamic_cast<SwSrcView*>( pCurrView ) !=  nullptr;
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
                if (!aMOpt.IsImpress() || GetObjectShell()->isExportLocked())
                    rSet.DisableItem( nWhich );
            }
            [[fallthrough]];
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
                bool bState = GetDoc()->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE);
                if(FN_PRINT_LAYOUT == nWhich)
                    bState = !bState;
                rSet.Put( SfxBoolItem( nWhich, bState));
            }
            break;

        case FN_NEW_GLOBAL_DOC:
            if (dynamic_cast<const SwGlobalDocShell*>(this) != nullptr
                || GetObjectShell()->isExportLocked())
                rSet.DisableItem( nWhich );
            break;

        case FN_NEW_HTML_DOC:
            if (dynamic_cast<const SwWebDocShell*>(this) != nullptr
                || GetObjectShell()->isExportLocked())
                rSet.DisableItem( nWhich );
            break;

        case FN_OPEN_FILE:
            if( dynamic_cast< const SwWebDocShell *>( this ) !=  nullptr )
                rSet.DisableItem( nWhich );
            break;

        case SID_ATTR_YEAR2000:
            {
                const SvNumberFormatter* pFormatr = m_xDoc->GetNumberFormatter(false);
                rSet.Put( SfxUInt16Item( nWhich,
                        static_cast< sal_uInt16 >(
                        pFormatr ? pFormatr->GetYear2000()
                              : officecfg::Office::Common::DateFormat::TwoDigitYear::get())) );
            }
            break;
        case SID_ATTR_CHAR_FONTLIST:
        {
            rSet.Put( SvxFontListItem(m_pFontList.get(), SID_ATTR_CHAR_FONTLIST) );
        }
        break;
        case SID_MAIL_PREPAREEXPORT:
        {
            //check if linked content or possibly hidden content is available
            //m_xDoc->UpdateFields( NULL, false );
            sfx2::LinkManager& rLnkMgr = m_xDoc->getIDocumentLinksAdministration().GetLinkManager();
            const ::sfx2::SvBaseLinks& rLnks = rLnkMgr.GetLinks();
            bool bRet = false;
            if( !rLnks.empty() )
                bRet = true;
            else
            {
                //sections with hidden flag, hidden character attribute, hidden paragraph/text or conditional text fields
                bRet = m_xDoc->HasInvisibleContent();
            }
            rSet.Put( SfxBoolItem( nWhich, bRet ) );
        }
        break;
        case SID_NOTEBOOKBAR:
        {
            SfxViewShell* pViewShell = GetView()? GetView(): SfxViewShell::Current();
            bool bVisible = sfx2::SfxNotebookBar::StateMethod(pViewShell->GetViewFrame()->GetBindings(),
                                                              "modules/swriter/ui/");
            rSet.Put( SfxBoolItem( SID_NOTEBOOKBAR, bVisible ) );
        }
        break;
        case FN_REDLINE_ACCEPT_ALL:
        case FN_REDLINE_REJECT_ALL:
        {
            if (GetDoc()->getIDocumentRedlineAccess().GetRedlineTable().empty() ||
                HasChangeRecordProtection()) // tdf#128229 Disable Accept / Reject all if redlines are password protected
                rSet.DisableItem(nWhich);
        }
        break;

        default: OSL_ENSURE(false,"You cannot get here!");

        }
        nWhich = aIter.NextWhich();
    }
}

// OLE-Hdls
IMPL_LINK( SwDocShell, Ole2ModifiedHdl, bool, bNewStatus, void )
{
    if( IsEnableSetModified() )
        SetModified( bNewStatus );
}

// return Pool here, because virtual
SfxStyleSheetBasePool*  SwDocShell::GetStyleSheetPool()
{
    return m_xBasePool.get();
}

sfx2::StyleManager* SwDocShell::GetStyleManager()
{
    return m_pStyleManager.get();
}

void SwDocShell::SetView(SwView* pVw)
{
    SetViewShell_Impl(pVw);
    m_pView = pVw;
    if (m_pView)
    {
        m_pWrtShell = &m_pView->GetWrtShell();

        // Set view-specific redline author.
        const OUString& rRedlineAuthor = m_pView->GetRedlineAuthor();
        if (!rRedlineAuthor.isEmpty())
            SW_MOD()->SetRedlineAuthor(m_pView->GetRedlineAuthor());
    }
    else
        m_pWrtShell = nullptr;
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
    // Thus, manual modify the document, if it's modified and its links are updated
    // before <FinishedLoading(..)> is called.
    const bool bHasDocToStayModified( m_xDoc->getIDocumentState().IsModified() && m_xDoc->getIDocumentLinksAdministration().LinksUpdated() );

    FinishedLoading();
    SfxViewFrame* pVFrame = SfxViewFrame::GetFirst(this);
    if(pVFrame)
    {
        SfxViewShell* pShell = pVFrame->GetViewShell();
        if(auto pSrcView = dynamic_cast<SwSrcView*>( pShell) )
            pSrcView->Load(this);
    }

    // #i38810#
    if ( bHasDocToStayModified && !m_xDoc->getIDocumentState().IsModified() )
    {
        m_xDoc->getIDocumentState().SetModified();
    }
}

// a Transfer is cancelled (is called from SFX)
void SwDocShell::CancelTransfers()
{
    // Cancel all links from LinkManager
    m_xDoc->getIDocumentLinksAdministration().GetLinkManager().CancelTransfers();
    SfxObjectShell::CancelTransfers();
}

SwEditShell * SwDocShell::GetEditShell()
{
    return m_pWrtShell;
}

SwFEShell* SwDocShell::GetFEShell()
{
    return m_pWrtShell;
}

void SwDocShell::RemoveOLEObjects()
{
    SwIterator<SwContentNode,SwFormatColl> aIter( *m_xDoc->GetDfltGrfFormatColl() );
    for( SwContentNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
    {
        SwOLENode* pOLENd = pNd->GetOLENode();
        if( pOLENd && ( pOLENd->IsOLEObjectDeleted() ||
                        pOLENd->IsInGlobalDocSection() ) )
        {
            if (!m_pOLEChildList)
                m_pOLEChildList.reset( new comphelper::EmbeddedObjectContainer );

            OUString aObjName = pOLENd->GetOLEObj().GetCurrentPersistName();
            GetEmbeddedObjectContainer().MoveEmbeddedObject( aObjName, *m_pOLEChildList );
        }
    }
}

// When a document is loaded, SwDoc::PrtOLENotify is called to update
// the sizes of math objects. However, for objects that do not have a
// SwFrame at this time, only a flag is set (bIsOLESizeInvalid) and the
// size change takes place later, while calculating the layout in the
// idle handler. If this document is saved now, it is saved with invalid
// sizes. For this reason, the layout has to be calculated before a document is
// saved, but of course only id there are OLE objects with bOLESizeInvalid set.
void SwDocShell::CalcLayoutForOLEObjects()
{
    if (!m_pWrtShell)
        return;

    if (m_pView && m_pView->GetIPClient())
    {
        // We have an active OLE edit: allow link updates, so an up to date replacement graphic can
        // be created.
        comphelper::EmbeddedObjectContainer& rEmbeddedObjectContainer = getEmbeddedObjectContainer();
        rEmbeddedObjectContainer.setUserAllowsLinkUpdate(true);
    }

    SwIterator<SwContentNode,SwFormatColl> aIter( *m_xDoc->GetDfltGrfFormatColl() );
    for( SwContentNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
    {
        SwOLENode* pOLENd = pNd->GetOLENode();
        if( pOLENd && pOLENd->IsOLESizeInvalid() )
        {
            m_pWrtShell->CalcLayout();
            break;
        }
    }
}

// #i42634# Overwrites SfxObjectShell::UpdateLinks
// This new function is necessary to trigger update of links in docs
// read by the binary filter:
void SwDocShell::UpdateLinks()
{
    GetDoc()->getIDocumentLinksAdministration().UpdateLinks();
    // #i50703# Update footnote numbers
    SwTextFootnote::SetUniqueSeqRefNo( *GetDoc() );
    SwNodeIndex aTmp( GetDoc()->GetNodes() );
    GetDoc()->GetFootnoteIdxs().UpdateFootnote( aTmp );
}

uno::Reference< frame::XController >
                                SwDocShell::GetController()
{
    css::uno::Reference< css::frame::XController > aRet;
    // #i82346# No view in page preview
    if ( GetView() )
        aRet = GetView()->GetController();
    return aRet;
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
sal_Int32 const s_nEvents(SAL_N_ELEMENTS(s_EventNames));

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
    return m_xDoc ? &m_xDoc->GetXmlIdRegistry() : nullptr;
}

bool SwDocShell::IsChangeRecording() const
{
    if (!m_pWrtShell)
        return false;
    return bool(m_pWrtShell->GetRedlineFlags() & RedlineFlags::On);
}

bool SwDocShell::HasChangeRecordProtection() const
{
    if (!m_pWrtShell)
        return false;
    return m_pWrtShell->getIDocumentRedlineAccess().GetRedlinePassword().hasElements();
}

void SwDocShell::SetChangeRecording( bool bActivate, bool bLockAllViews )
{
    RedlineFlags nOn = bActivate ? RedlineFlags::On : RedlineFlags::NONE;
    RedlineFlags nMode = m_pWrtShell->GetRedlineFlags();
    if (bLockAllViews)
    {
        // tdf#107870: prevent jumping to cursor
        auto aViewGuard(LockAllViews());
        m_pWrtShell->SetRedlineFlagsAndCheckInsMode( (nMode & ~RedlineFlags::On) | nOn );
    }
    else
    {
        m_pWrtShell->SetRedlineFlagsAndCheckInsMode( (nMode & ~RedlineFlags::On) | nOn );
    }
}

void SwDocShell::SetProtectionPassword( const OUString &rNewPassword )
{
    const SfxAllItemSet aSet( GetPool() );
    const SfxPoolItem*  pItem = nullptr;

    IDocumentRedlineAccess& rIDRA = m_pWrtShell->getIDocumentRedlineAccess();
    Sequence< sal_Int8 > aPasswd = rIDRA.GetRedlinePassword();
    if (SfxItemState::SET == aSet.GetItemState(FN_REDLINE_PROTECT, false, &pItem)
        && static_cast<const SfxBoolItem*>(pItem)->GetValue() == aPasswd.hasElements())
        return;

    if (!rNewPassword.isEmpty())
    {
        // when password protection is applied change tracking must always be active
        SetChangeRecording( true );

        Sequence< sal_Int8 > aNewPasswd;
        SvPasswordHelper::GetHashPassword( aNewPasswd, rNewPassword );
        rIDRA.SetRedlinePassword( aNewPasswd );
    }
    else
    {
        rIDRA.SetRedlinePassword( Sequence< sal_Int8 >() );
    }
}

bool SwDocShell::GetProtectionHash( /*out*/ css::uno::Sequence< sal_Int8 > &rPasswordHash )
{
    bool bRes = false;

    const SfxAllItemSet aSet( GetPool() );
    const SfxPoolItem*  pItem = nullptr;

    IDocumentRedlineAccess& rIDRA = m_pWrtShell->getIDocumentRedlineAccess();
    const Sequence< sal_Int8 >& aPasswdHash( rIDRA.GetRedlinePassword() );
    if (SfxItemState::SET == aSet.GetItemState(FN_REDLINE_PROTECT, false, &pItem)
        && static_cast<const SfxBoolItem*>(pItem)->GetValue() == aPasswdHash.hasElements())
        return false;
    rPasswordHash = aPasswdHash;
    bRes = true;

    return bRes;
}

void SwDocShell::RegisterAutomationDocumentEventsCaller(css::uno::Reference< ooo::vba::XSinkCaller > const& xCaller)
{
    mxAutomationDocumentEventsCaller = xCaller;
}

void SwDocShell::CallAutomationDocumentEventSinks(const OUString& Method, css::uno::Sequence< css::uno::Any >& Arguments)
{
    if (mxAutomationDocumentEventsCaller.is())
        mxAutomationDocumentEventsCaller->CallSinks(Method, Arguments);
}

void SwDocShell::RegisterAutomationDocumentObject(css::uno::Reference< ooo::vba::word::XDocument > const& xDocument)
{
    mxAutomationDocumentObject = xDocument;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
