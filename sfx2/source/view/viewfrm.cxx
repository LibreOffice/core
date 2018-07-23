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
#include <osl/file.hxx>
#include <sfx2/infobar.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/classificationhelper.hxx>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchRecorder.hpp>
#include <com/sun/star/frame/DispatchRecorderSupplier.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Setup.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/splitwin.hxx>
#include <unotools/moduleoptions.hxx>
#include <svl/intitem.hxx>
#include <svl/visitem.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/slstitm.hxx>
#include <svl/whiter.hxx>
#include <svl/undo.hxx>
#include <vcl/weld.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/ehdl.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <rtl/ustrbuf.hxx>

#include <unotools/ucbhelper.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/docpasswordrequest.hxx>
#include <comphelper/docpasswordhelper.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>

#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbx.hxx>
#include <svtools/asynclink.hxx>
#include <svl/sharecontrolfile.hxx>
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <framework/framelistanalyzer.hxx>
#include <shellimpl.hxx>

#include <boost/optional.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using ::com::sun::star::awt::XWindow;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::document::XViewDataSupplier;
using ::com::sun::star::container::XIndexContainer;

// Due to ViewFrame::Current
#include <appdata.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <openflag.hxx>
#include <objshimp.hxx>
#include <openuriexternally.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <arrdecl.hxx>
#include <sfxtypes.hxx>
#include <sfx2/request.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/ipclient.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/viewfac.hxx>
#include <sfx2/event.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/module.hxx>
#include <sfx2/msgpool.hxx>
#include "viewimp.hxx"
#include <sfx2/sfxbasecontroller.hxx>
#include <sfx2/frmdescr.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <workwin.hxx>
#include <helper.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/strings.hrc>
#include "impviewframe.hxx"
#include <vcl/svapp.hxx>

#define ShellClass_SfxViewFrame
#include <sfxslots.hxx>

SFX_IMPL_SUPERCLASS_INTERFACE(SfxViewFrame,SfxShell)

void SfxViewFrame::InitInterface_Impl()
{
    GetStaticInterface()->RegisterChildWindow(SID_BROWSER);
    GetStaticInterface()->RegisterChildWindow(SID_RECORDING_FLOATWINDOW);
#if HAVE_FEATURE_DESKTOP
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_FULLSCREEN, SfxVisibilityFlags::FullScreen, ToolbarId::FullScreenToolbox);
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_APPLICATION, SfxVisibilityFlags::Standard, ToolbarId::EnvToolbox);
#endif
}

namespace {
/// Asks the user if editing a read-only document is really wanted.
class SfxEditDocumentDialog : public weld::MessageDialogController
{
private:
    std::unique_ptr<weld::Button> m_xEditDocument;
    std::unique_ptr<weld::Button> m_xCancel;

public:
    SfxEditDocumentDialog(weld::Widget* pParent);
};

SfxEditDocumentDialog::SfxEditDocumentDialog(weld::Widget* pParent)
    : MessageDialogController(pParent, "sfx/ui/editdocumentdialog.ui",
            "EditDocumentDialog")
    , m_xEditDocument(m_xBuilder->weld_button("edit"))
    , m_xCancel(m_xBuilder->weld_button("cancel"))
{
}

class SfxQueryOpenAsTemplate
{
private:
    std::unique_ptr<weld::MessageDialog> m_xQueryBox;
public:
    SfxQueryOpenAsTemplate(weld::Window* pParent, bool bAllowIgnoreLock)
        : m_xQueryBox(Application::CreateMessageDialog(pParent, VclMessageType::Question, VclButtonsType::NONE,
                      SfxResId(bAllowIgnoreLock ? STR_QUERY_OPENASTEMPLATE_ALLOW_IGNORE : STR_QUERY_OPENASTEMPLATE)))
    {
        m_xQueryBox->add_button(SfxResId(STR_QUERY_OPENASTEMPLATE_OPENCOPY_BTN), RET_YES);
        if (bAllowIgnoreLock)
            m_xQueryBox->add_button(SfxResId(STR_QUERY_OPENASTEMPLATE_OPEN_BTN), RET_IGNORE);
        m_xQueryBox->add_button(Button::GetStandardText( StandardButtonType::Cancel ), RET_CANCEL);
        m_xQueryBox->set_default_response(RET_YES);
    }
    short run() { return m_xQueryBox->run(); }
};

/// Is this read-only object shell opened via .uno:SignPDF?
bool IsSignPDF(const SfxObjectShellRef& xObjSh)
{
    if (!xObjSh.is())
        return false;

    SfxMedium* pMedium = xObjSh->GetMedium();
    if (pMedium && !pMedium->IsOriginallyReadOnly())
    {
        std::shared_ptr<const SfxFilter> pFilter = pMedium->GetFilter();
        if (pFilter && pFilter->GetName() == "draw_pdf_import")
            return true;
    }

    return false;
}

bool AskPasswordToModify_Impl( const uno::Reference< task::XInteractionHandler >& xHandler, const OUString& aPath, const std::shared_ptr<const SfxFilter>& pFilter, sal_uInt32 nPasswordHash, const uno::Sequence< beans::PropertyValue >& aInfo )
{
    // TODO/LATER: In future the info should replace the direct hash completely
    bool bResult = ( !nPasswordHash && !aInfo.getLength() );

    SAL_WARN_IF( !(pFilter && ( pFilter->GetFilterFlags() & SfxFilterFlags::PASSWORDTOMODIFY )), "sfx.view",
                       "PasswordToModify feature is active for a filter that does not support it!");

    if ( pFilter && xHandler.is() )
    {
        bool bCancel = false;
        bool bFirstTime = true;

        while ( !bResult && !bCancel )
        {
            bool bMSType = !pFilter->IsOwnFormat();

            ::rtl::Reference< ::comphelper::DocPasswordRequest > pPasswordRequest(
                 new ::comphelper::DocPasswordRequest(
                 bMSType ? ::comphelper::DocPasswordRequestType::MS : ::comphelper::DocPasswordRequestType::Standard,
                 bFirstTime ? css::task::PasswordRequestMode_PASSWORD_ENTER : css::task::PasswordRequestMode_PASSWORD_REENTER,
                 aPath,
                 true ) );

            uno::Reference< css::task::XInteractionRequest > rRequest( pPasswordRequest.get() );
            xHandler->handle( rRequest );

            if ( pPasswordRequest->isPassword() )
            {
                if ( aInfo.getLength() )
                {
                    bResult = ::comphelper::DocPasswordHelper::IsModifyPasswordCorrect( pPasswordRequest->getPasswordToModify(), aInfo );
                }
                else
                {
                    // the binary format
                    bResult = ( SfxMedium::CreatePasswordToModifyHash( pPasswordRequest->getPasswordToModify(), pFilter->GetServiceName()=="com.sun.star.text.TextDocument" ) == nPasswordHash );
                }
            }
            else
                bCancel = true;

            bFirstTime = false;
        }
    }

    return bResult;
}
}

void SfxViewFrame::ExecReload_Impl( SfxRequest& rReq )
{
    SfxObjectShell* pSh = GetObjectShell();
    switch ( rReq.GetSlot() )
    {
        case SID_EDITDOC:
        case SID_READONLYDOC:
        {
            // Due to Double occupancy in toolboxes (with or without Ctrl),
            // it is also possible that the slot is enabled, but Ctrl-click
            // despite this is not!
            if( !pSh || !pSh->HasName() || !(pSh->Get_Impl()->nLoadedFlags & SfxLoadedFlags::MAINDOCUMENT ))
                break;

            // Only change read-only UI and remove info bar when we succeed
            struct ReadOnlyUIGuard
            {
                SfxViewFrame* m_pFrame;
                SfxObjectShell* m_pSh;
                SfxMedium* m_pMed = nullptr;
                bool m_bSetRO;
                ReadOnlyUIGuard(SfxViewFrame* pFrame, SfxObjectShell* p_Sh)
                    : m_pFrame(pFrame), m_pSh(p_Sh), m_bSetRO(p_Sh->IsReadOnlyUI())
                {}
                ~ReadOnlyUIGuard() COVERITY_NOEXCEPT_FALSE
                {
                    if (m_bSetRO != m_pSh->IsReadOnlyUI())
                    {
                        m_pSh->SetReadOnlyUI(m_bSetRO);
                        if (!m_bSetRO)
                            m_pFrame->RemoveInfoBar("readonly");
                        if (m_pMed)
                        {
                            // tdf#116066: DoSaveCompleted should be called after SetReadOnlyUI
                            m_pSh->DoSaveCompleted(m_pMed);
                            m_pSh->Broadcast(SfxHint(SfxHintId::ModeChanged));
                        }
                    }
                }
            } aReadOnlyUIGuard(this, pSh);

            SfxMedium* pMed = pSh->GetMedium();

            const SfxBoolItem* pItem = SfxItemSet::GetItem<SfxBoolItem>(pSh->GetMedium()->GetItemSet(), SID_VIEWONLY, false);
            if ( pItem && pItem->GetValue() )
            {
                SfxApplication* pApp = SfxGetpApp();
                SfxAllItemSet aSet( pApp->GetPool() );
                aSet.Put( SfxStringItem( SID_FILE_NAME, pMed->GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::NONE) ) );
                aSet.Put( SfxBoolItem( SID_TEMPLATE, true ) );
                aSet.Put( SfxStringItem( SID_TARGETNAME, "_blank" ) );
                const SfxStringItem* pReferer = SfxItemSet::GetItem<SfxStringItem>(pMed->GetItemSet(), SID_REFERER, false);
                if ( pReferer )
                    aSet.Put( *pReferer );
                const SfxInt16Item* pVersionItem = SfxItemSet::GetItem<SfxInt16Item>(pSh->GetMedium()->GetItemSet(), SID_VERSION, false);
                if ( pVersionItem )
                    aSet.Put( *pVersionItem );

                if( pMed->GetFilter() )
                {
                    aSet.Put( SfxStringItem( SID_FILTER_NAME, pMed->GetFilter()->GetFilterName() ) );
                    const SfxStringItem* pOptions = SfxItemSet::GetItem<SfxStringItem>(pMed->GetItemSet(), SID_FILE_FILTEROPTIONS, false);
                    if ( pOptions )
                        aSet.Put( *pOptions );
                }

                GetDispatcher()->Execute( SID_OPENDOC, SfxCallMode::ASYNCHRON, aSet );
                return;
            }

            StreamMode nOpenMode;
            bool bNeedsReload = false;
            if ( !pSh->IsReadOnly() )
            {
                // Save and reload Readonly
                if( pSh->IsModified() )
                {
                    if ( pSh->PrepareClose() )
                    {
                        // the storing could let the medium be changed
                        pMed = pSh->GetMedium();
                        bNeedsReload = true;
                    }
                    else
                    {
                        rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), false ) );
                        return;
                    }
                }
                nOpenMode = SFX_STREAM_READONLY;
                aReadOnlyUIGuard.m_bSetRO = true;
            }
            else
            {
                if ( pSh->IsReadOnlyMedium()
                  && ( pSh->GetModifyPasswordHash() || pSh->GetModifyPasswordInfo().getLength() )
                  && !pSh->IsModifyPasswordEntered() )
                {
                    const OUString aDocumentName = INetURLObject( pMed->GetOrigURL() ).GetMainURL( INetURLObject::DecodeMechanism::WithCharset );
                    if( !AskPasswordToModify_Impl( pMed->GetInteractionHandler(), aDocumentName, pMed->GetOrigFilter(), pSh->GetModifyPasswordHash(), pSh->GetModifyPasswordInfo() ) )
                    {
                        // this is a read-only document, if it has "Password to modify"
                        // the user should enter password before he can edit the document
                        rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), false ) );
                        return;
                    }

                    pSh->SetModifyPasswordEntered();
                }

                nOpenMode = pSh->IsOriginallyReadOnlyMedium() ? SFX_STREAM_READONLY : SFX_STREAM_READWRITE;
                aReadOnlyUIGuard.m_bSetRO = false;

                // if only the view was in the readonly mode then there is no need to do the reload
                if ( !pSh->IsReadOnlyMedium() )
                {
                    // SetReadOnlyUI causes recomputation of window title, using
                    // open mode among other things, so call SetOpenMode before
                    // SetReadOnlyUI:
                    pMed->SetOpenMode( nOpenMode );
                    return;
                }
            }

            if ( rReq.IsAPI() )
            {
                // Control through API if r/w or r/o
                const SfxBoolItem* pEditItem = rReq.GetArg<SfxBoolItem>(SID_EDITDOC);
                if ( pEditItem )
                    nOpenMode = pEditItem->GetValue() ? SFX_STREAM_READWRITE : SFX_STREAM_READONLY;
            }

            // doing

            OUString sTemp;
            osl::FileBase::getFileURLFromSystemPath( pMed->GetPhysicalName(), sTemp );
            INetURLObject aPhysObj( sTemp );
            const SfxInt16Item* pVersionItem = SfxItemSet::GetItem<SfxInt16Item>(pSh->GetMedium()->GetItemSet(), SID_VERSION, false);

            INetURLObject aMedObj( pMed->GetName() );

            // -> tdf#82744
            // the logic below is following:
            // if the document seems not to need to be reloaded
            //     and the physical name is different to the logical one,
            // then on file system it can be checked that the copy is still newer than the original and no document reload is required.
            // Did some semplification to enhance readability of the 'if' expression
            //
            // when the 'http/https' protocol is active, the bool bPhysObjIsYounger relies upon the getlastmodified Property of a WebDAV resource.
            // Said property should be implemented, but sometimes it's not.
            // implemented. On this case the reload activated here will not work properly.
            // TODO: change the check age method for WebDAV to etag (entity-tag) property value, need some rethinking, since the
            // etag tells that the cache representation (e.g. in LO) is different from the one on the server,
            // but tells nothing about the age
            // Details at this link: http://tools.ietf.org/html/rfc4918#section-15, section 15.7
            bool bPhysObjIsYounger = ::utl::UCBContentHelper::IsYounger( aMedObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                                                                         aPhysObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
            bool bIsWebDAV = aMedObj.isAnyKnownWebDAVScheme();

            if ( ( !bNeedsReload && ( ( aMedObj.GetProtocol() == INetProtocol::File &&
                                        aMedObj.getFSysPath( FSysStyle::Detect ) != aPhysObj.getFSysPath( FSysStyle::Detect ) &&
                                        !bPhysObjIsYounger )
                                      || ( bIsWebDAV && !bPhysObjIsYounger )
                                      || ( pMed->IsRemote() && !bIsWebDAV ) ) )
                 || pVersionItem )
            // <- tdf#82744
            {
                bool bOK = false;
                bool bRetryIgnoringLock = false;
                bool bOpenTemplate = false;
                do {
                    if ( !pVersionItem )
                    {
                        if (bRetryIgnoringLock)
                            pMed->ResetError();

                        bool bHasStorage = pMed->HasStorage_Impl();
                        // switching edit mode could be possible without reload
                        if ( bHasStorage && pMed->GetStorage() == pSh->GetStorage() )
                        {
                            // TODO/LATER: faster creation of copy
                            if ( !pSh->ConnectTmpStorage_Impl( pMed->GetStorage(), pMed ) )
                                return;
                        }

                        pMed->CloseAndRelease();
                        pMed->GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, !( nOpenMode & StreamMode::WRITE ) ) );
                        pMed->SetOpenMode( nOpenMode );

                        pMed->CompleteReOpen();
                        if ( nOpenMode & StreamMode::WRITE )
                        {
                             auto eResult = pMed->LockOrigFileOnDemand( true, true, bRetryIgnoringLock );
                             bRetryIgnoringLock = eResult == SfxMedium::LockFileResult::FailedLockFile;
                        }

                        // LockOrigFileOnDemand might set the readonly flag itself, it should be set back
                        pMed->GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, !( nOpenMode & StreamMode::WRITE ) ) );

                        if ( !pMed->GetErrorCode() )
                            bOK = true;
                    }

                    if( !bOK )
                    {
                        if (nOpenMode == SFX_STREAM_READWRITE && !rReq.IsAPI())
                        {
                            // css::sdbcx::User offering to open it as a template
                            SfxQueryOpenAsTemplate aBox(GetWindow().GetFrameWeld(), bRetryIgnoringLock);

                            short nUserAnswer = aBox.run();
                            bOpenTemplate = RET_YES == nUserAnswer;
                            // Always reset this here to avoid infinite loop
                            bRetryIgnoringLock = RET_IGNORE == nUserAnswer;
                        }
                        else
                            bRetryIgnoringLock = false;
                    }
                }
                while ( !bOK && bRetryIgnoringLock );

                if( !bOK )
                {
                    ErrCode nErr = pMed->GetErrorCode();
                    if ( pVersionItem )
                        nErr = ERRCODE_IO_ACCESSDENIED;
                    else
                    {
                        pMed->ResetError();
                        pMed->SetOpenMode( SFX_STREAM_READONLY );
                        pMed->ReOpen();
                        pSh->DoSaveCompleted( pMed );
                    }

                    // Readonly document can not be switched to edit mode?
                    rReq.Done();

                    if ( nOpenMode == SFX_STREAM_READWRITE && !rReq.IsAPI() )
                    {
                        if ( bOpenTemplate )
                        {
                            SfxApplication* pApp = SfxGetpApp();
                            SfxAllItemSet aSet( pApp->GetPool() );
                            aSet.Put( SfxStringItem( SID_FILE_NAME, pMed->GetName() ) );
                            const SfxStringItem* pReferer = SfxItemSet::GetItem<SfxStringItem>(pMed->GetItemSet(), SID_REFERER, false);
                            if ( pReferer )
                                aSet.Put( *pReferer );
                            aSet.Put( SfxBoolItem( SID_TEMPLATE, true ) );
                            if ( pVersionItem )
                                aSet.Put( *pVersionItem );

                            if( pMed->GetFilter() )
                            {
                                aSet.Put( SfxStringItem( SID_FILTER_NAME, pMed->GetFilter()->GetFilterName() ) );
                                const SfxStringItem* pOptions = SfxItemSet::GetItem<SfxStringItem>(pMed->GetItemSet(), SID_FILE_FILTEROPTIONS, false);
                                if ( pOptions )
                                    aSet.Put( *pOptions );
                            }

                            GetDispatcher()->Execute( SID_OPENDOC, SfxCallMode::ASYNCHRON, aSet );
                            return;
                        }

                        nErr = ERRCODE_NONE;
                    }

                    // Keep the read-only UI
                    aReadOnlyUIGuard.m_bSetRO = true;

                    ErrorHandler::HandleError( nErr );
                    rReq.SetReturnValue(
                        SfxBoolItem( rReq.GetSlot(), false ) );
                    return;
                }
                else
                {
                    aReadOnlyUIGuard.m_pMed = pMed;
                    rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), true ) );
                    rReq.Done( true );
                    return;
                }
            }

            rReq.AppendItem( SfxBoolItem( SID_FORCERELOAD, true) );
            rReq.AppendItem( SfxBoolItem( SID_SILENT, true ));

            SAL_FALLTHROUGH; //TODO ???
        }

        case SID_RELOAD:
        {
            // Due to Double occupancy in toolboxes (with or without Ctrl),
            // it is also possible that the slot is enabled, but Ctrl-click
            // despite this is not!
            if ( !pSh || !pSh->CanReload_Impl() )
                break;
            SfxApplication* pApp = SfxGetpApp();
            const SfxBoolItem* pForceReloadItem = rReq.GetArg<SfxBoolItem>(SID_FORCERELOAD);
            if(  pForceReloadItem && !pForceReloadItem->GetValue() &&
                !pSh->GetMedium()->IsExpired() )
                return;
            if( m_pImpl->bReloading || pSh->IsInModalMode() )
                return;

            // AutoLoad is prohibited if possible
            const SfxBoolItem* pAutoLoadItem = rReq.GetArg<SfxBoolItem>(SID_AUTOLOAD);
            if ( pAutoLoadItem && pAutoLoadItem->GetValue() &&
                 GetFrame().IsAutoLoadLocked_Impl() )
                return;

            SfxObjectShellLock xOldObj( pSh );
            m_pImpl->bReloading = true;
            const SfxStringItem* pURLItem = rReq.GetArg<SfxStringItem>(SID_FILE_NAME);
            // Open as editable?
            bool bForEdit = !pSh->IsReadOnly();

            // If possible ask the User
            bool bDo = GetViewShell()->PrepareClose();
            const SfxBoolItem* pSilentItem = rReq.GetArg<SfxBoolItem>(SID_SILENT);
            if ( bDo && GetFrame().DocIsModified_Impl() &&
                 !rReq.IsAPI() && ( !pSilentItem || !pSilentItem->GetValue() ) )
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetWindow().GetFrameWeld(),
                                                                         VclMessageType::Question, VclButtonsType::YesNo,
                                                                         SfxResId(STR_QUERY_LASTVERSION)));
                bDo = RET_YES == xBox->run();
            }

            if ( bDo )
            {
                SfxMedium *pMedium = xOldObj->GetMedium();

                bool bHandsOff =
                    ( pMedium->GetURLObject().GetProtocol() == INetProtocol::File && !xOldObj->IsDocShared() );

                // Empty existing SfxMDIFrames for this Document
                // in native format or R/O, open it now for editing?
                SfxObjectShellLock xNewObj;

                // collect the views of the document
                // TODO: when UNO ViewFactories are available for SFX-based documents, the below code should
                // be UNOized, too
                typedef ::std::pair< Reference< XFrame >, SfxInterfaceId >  ViewDescriptor;
                ::std::vector< ViewDescriptor > aViewFrames;
                SfxViewFrame *pView = GetFirst( xOldObj );
                while ( pView )
                {
                    Reference< XFrame > xFrame( pView->GetFrame().GetFrameInterface() );
                    SAL_WARN_IF( !xFrame.is(), "sfx.view", "SfxViewFrame::ExecReload_Impl: no XFrame?!");
                    aViewFrames.emplace_back( xFrame, pView->GetCurViewId() );

                    pView = GetNext( *pView, xOldObj );
                }

                DELETEZ( xOldObj->Get_Impl()->pReloadTimer );

                SfxItemSet* pNewSet = nullptr;
                std::shared_ptr<const SfxFilter> pFilter = pMedium->GetFilter();
                if( pURLItem )
                {
                    pNewSet = new SfxAllItemSet( pApp->GetPool() );
                    pNewSet->Put( *pURLItem );

                    // Filter Detection
                    OUString referer;
                    const SfxStringItem* refererItem = rReq.GetArg<SfxStringItem>(SID_REFERER);
                    if (refererItem != nullptr) {
                        referer = refererItem->GetValue();
                    }
                    SfxMedium aMedium( pURLItem->GetValue(), referer, SFX_STREAM_READWRITE );
                    SfxFilterMatcher().GuessFilter( aMedium, pFilter );
                    if ( pFilter )
                        pNewSet->Put( SfxStringItem( SID_FILTER_NAME, pFilter->GetName() ) );
                    pNewSet->Put( *aMedium.GetItemSet() );
                }
                else
                {
                    pNewSet = new SfxAllItemSet( *pMedium->GetItemSet() );
                    pNewSet->ClearItem( SID_VIEW_ID );
                    pNewSet->ClearItem( SID_STREAM );
                    pNewSet->ClearItem( SID_INPUTSTREAM );
                    pNewSet->Put( SfxStringItem( SID_FILTER_NAME, pMedium->GetFilter()->GetName() ) );

                    // let the current security settings be checked again
                    pNewSet->Put( SfxUInt16Item( SID_MACROEXECMODE, document::MacroExecMode::USE_CONFIG ) );

                    if ( pSh->IsOriginallyReadOnlyMedium()
                         || pSh->IsOriginallyLoadedReadOnlyMedium() )
                        // edit mode is switched or reload of readonly document
                        pNewSet->Put( SfxBoolItem( SID_DOC_READONLY, true ) );
                    else
                        // Reload of file opened for writing
                        pNewSet->ClearItem( SID_DOC_READONLY );
                }

                // If a salvaged file is present, do not enclose the OrigURL
                // again, since the Template is invalid after reload.
                const SfxStringItem* pSalvageItem = SfxItemSet::GetItem<SfxStringItem>(pNewSet, SID_DOC_SALVAGE, false);
                if( pSalvageItem )
                {
                    pNewSet->ClearItem( SID_DOC_SALVAGE );
                }

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                // TODO/LATER: Temporary solution, the SfxMedium must know the original URL as aLogicName
                //             SfxMedium::Transfer_Impl() will be forbidden then.
                if ( xOldObj->IsDocShared() )
                    pNewSet->Put( SfxStringItem( SID_FILE_NAME, xOldObj->GetSharedFileURL() ) );
#endif
                if ( pURLItem )
                    pNewSet->Put( SfxStringItem( SID_REFERER, pMedium->GetName() ) );
                else
                    pNewSet->Put( SfxStringItem( SID_REFERER, OUString() ) );

                xOldObj->CancelTransfers();


                if ( pSilentItem && pSilentItem->GetValue() )
                    pNewSet->Put( SfxBoolItem( SID_SILENT, true ) );

                const SfxUnoAnyItem* pInteractionItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pNewSet, SID_INTERACTIONHANDLER, false);
                const SfxUInt16Item* pMacroExecItem = SfxItemSet::GetItem<SfxUInt16Item>(pNewSet, SID_MACROEXECMODE, false);
                const SfxUInt16Item* pDocTemplateItem = SfxItemSet::GetItem<SfxUInt16Item>(pNewSet, SID_UPDATEDOCMODE, false);

                if (!pInteractionItem)
                {
                    Reference < task::XInteractionHandler2 > xHdl = task::InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), nullptr );
                    if (xHdl.is())
                        pNewSet->Put( SfxUnoAnyItem(SID_INTERACTIONHANDLER,css::uno::makeAny(xHdl)) );
                }

                if (!pMacroExecItem)
                    pNewSet->Put( SfxUInt16Item(SID_MACROEXECMODE,css::document::MacroExecMode::USE_CONFIG) );
                if (!pDocTemplateItem)
                    pNewSet->Put( SfxUInt16Item(SID_UPDATEDOCMODE,css::document::UpdateDocMode::ACCORDING_TO_CONFIG) );

                xOldObj->SetModified( false );
                // Do not cache the old Document! Is invalid when loading
                // another document.

                const SfxStringItem* pSavedOptions = SfxItemSet::GetItem<SfxStringItem>(pMedium->GetItemSet(), SID_FILE_FILTEROPTIONS, false);
                const SfxStringItem* pSavedReferer = SfxItemSet::GetItem<SfxStringItem>(pMedium->GetItemSet(), SID_REFERER, false);

                bool bHasStorage = pMedium->HasStorage_Impl();
                if( bHandsOff )
                {
                    if ( bHasStorage && pMedium->GetStorage() == xOldObj->GetStorage() )
                    {
                        // TODO/LATER: faster creation of copy
                        if ( !xOldObj->ConnectTmpStorage_Impl( pMedium->GetStorage(), pMedium ) )
                            return;
                    }

                    pMedium->CloseAndRelease();
                }

                xNewObj = SfxObjectShell::CreateObject( pFilter->GetServiceName() );

                if ( xOldObj->IsModifyPasswordEntered() )
                    xNewObj->SetModifyPasswordEntered();

                uno::Sequence < beans::PropertyValue > aLoadArgs;
                TransformItems( SID_OPENDOC, *pNewSet, aLoadArgs );
                try
                {
                    uno::Reference < frame::XLoadable > xLoad( xNewObj->GetModel(), uno::UNO_QUERY );
                    xLoad->load( aLoadArgs );
                }
                catch ( uno::Exception& )
                {
                    xNewObj->DoClose();
                    xNewObj = nullptr;
                }

                DELETEZ( pNewSet );

                if( !xNewObj.Is() )
                {
                    if( bHandsOff )
                    {
                        // back to old medium
                        pMedium->ReOpen();
                        pMedium->LockOrigFileOnDemand( false, true );

                        xOldObj->DoSaveCompleted( pMedium );
                    }

                    // r/o-Doc couldn't be switched to writing mode
                    if ( bForEdit && ( SID_EDITDOC == rReq.GetSlot() || SID_READONLYDOC == rReq.GetSlot() ) )
                    {
                        // ask user for opening as template
                        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetWindow().GetFrameWeld(),
                                                                                 VclMessageType::Question, VclButtonsType::YesNo,
                                                                                 SfxResId(STR_QUERY_OPENASTEMPLATE)));
                        if (RET_YES == xBox->run())
                        {
                            SfxAllItemSet aSet( pApp->GetPool() );
                            aSet.Put( SfxStringItem( SID_FILE_NAME, pMedium->GetName() ) );
                            aSet.Put( SfxStringItem( SID_TARGETNAME, "_blank" ) );
                            if ( pSavedOptions )
                                aSet.Put( *pSavedOptions );
                            if ( pSavedReferer )
                                aSet.Put( *pSavedReferer );
                            aSet.Put( SfxBoolItem( SID_TEMPLATE, true ) );
                            if( pFilter )
                                aSet.Put( SfxStringItem( SID_FILTER_NAME, pFilter->GetFilterName() ) );
                            GetDispatcher()->Execute( SID_OPENDOC, SfxCallMode::ASYNCHRON, aSet );
                        }
                    }
                }
                else
                {
                    if ( xNewObj->GetModifyPasswordHash() && xNewObj->GetModifyPasswordHash() != xOldObj->GetModifyPasswordHash() )
                    {
                        xNewObj->SetModifyPasswordEntered( false );
                        xNewObj->SetReadOnly();
                    }
                    else if ( rReq.GetSlot() == SID_EDITDOC || rReq.GetSlot() == SID_READONLYDOC )
                    {
                        xNewObj->SetReadOnlyUI( !bForEdit );
                    }

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                    if ( xNewObj->IsDocShared() )
                    {
                        // the file is shared but the closing can change the sharing control file
                        xOldObj->DoNotCleanShareControlFile();
                    }
#endif
                    // the Reload and Silent items were only temporary, remove them
                    xNewObj->GetMedium()->GetItemSet()->ClearItem( SID_RELOAD );
                    xNewObj->GetMedium()->GetItemSet()->ClearItem( SID_SILENT );
                    TransformItems( SID_OPENDOC, *xNewObj->GetMedium()->GetItemSet(), aLoadArgs );

                    UpdateDocument_Impl();

                    try
                    {
                        for (auto const& viewFrame : aViewFrames)
                        {
                            LoadViewIntoFrame_Impl( *xNewObj, viewFrame.first, aLoadArgs, viewFrame.second, false );
                        }
                        aViewFrames.clear();
                    }
                    catch( const Exception& )
                    {
                        // close the remaining frames
                        // Don't catch exceptions herein, if this fails, then we're left in an indetermined state, and
                        // crashing is better than trying to proceed
                        for (auto const& viewFrame : aViewFrames)
                        {
                            Reference< util::XCloseable > xClose( viewFrame.first, UNO_QUERY_THROW );
                            xClose->close( true );
                        }
                        aViewFrames.clear();
                    }

                    // Propagate document closure.
                    SfxGetpApp()->NotifyEvent( SfxEventHint( SfxEventHintId::CloseDoc, GlobalEventConfig::GetEventName( GlobalEventId::CLOSEDOC ), xOldObj ) );
                }

                // Record as done
                rReq.Done( true );
                rReq.SetReturnValue(SfxBoolItem(rReq.GetSlot(), true));
                return;
            }
            else
            {
                // Record as not done
                rReq.Done();
                rReq.SetReturnValue(SfxBoolItem(rReq.GetSlot(), false));
                m_pImpl->bReloading = false;
                return;
            }
        }
    }
}

void SfxViewFrame::StateReload_Impl( SfxItemSet& rSet )
{
    SfxObjectShell* pSh = GetObjectShell();
    if ( !pSh )
    {
        // I'm just on reload and am yielding myself ...
        return;
    }

    SfxWhichIter aIter( rSet );
    for ( sal_uInt16 nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich() )
    {
        switch ( nWhich )
        {
            case SID_EDITDOC:
            case SID_READONLYDOC:
            {
                const SfxViewShell *pVSh;
                const SfxShell *pFSh;
                if ( !pSh->HasName() ||
                     !( pSh->Get_Impl()->nLoadedFlags &  SfxLoadedFlags::MAINDOCUMENT ) ||
                     ( pSh->GetCreateMode() == SfxObjectCreateMode::EMBEDDED &&
                       ( !(pVSh = pSh->GetViewShell())  ||
                         !(pFSh = pVSh->GetFormShell()) ||
                         !pFSh->IsDesignMode())))
                    rSet.DisableItem( nWhich );
                else
                {
                    const SfxBoolItem* pItem = SfxItemSet::GetItem<SfxBoolItem>(pSh->GetMedium()->GetItemSet(), SID_EDITDOC, false);
                    if ( pItem && !pItem->GetValue() )
                        rSet.DisableItem( nWhich );
                    else
                    {
                        if (nWhich==SID_EDITDOC)
                            rSet.Put( SfxBoolItem( nWhich, !pSh->IsReadOnly() ) );
                        else if (nWhich==SID_READONLYDOC)
                            rSet.Put( SfxBoolItem( nWhich, pSh->IsReadOnly() ) );
                    }
                }
                break;
            }

            case SID_RELOAD:
            {
                if ( !pSh->CanReload_Impl() || pSh->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
                    rSet.DisableItem(nWhich);
                else
                {
                    // If any ChildFrame is reloadable, the slot is enabled,
                    // so you can perform CTRL-Reload
                    rSet.Put( SfxBoolItem( nWhich, false));
                }

                break;
            }
        }
    }
}

void SfxViewFrame::ExecHistory_Impl( SfxRequest &rReq )
{
    // Is there an Undo-Manager on the top Shell?
    SfxShell *pSh = GetDispatcher()->GetShell(0);
    SfxUndoManager* pShUndoMgr = pSh->GetUndoManager();
    bool bOK = false;
    if ( pShUndoMgr )
    {
        switch ( rReq.GetSlot() )
        {
            case SID_CLEARHISTORY:
                pShUndoMgr->Clear();
                bOK = true;
                break;

            case SID_UNDO:
                pShUndoMgr->Undo();
                GetBindings().InvalidateAll(false);
                bOK = true;
                break;

            case SID_REDO:
                pShUndoMgr->Redo();
                GetBindings().InvalidateAll(false);
                bOK = true;
                break;

            case SID_REPEAT:
                if ( pSh->GetRepeatTarget() )
                    pShUndoMgr->Repeat( *pSh->GetRepeatTarget() );
                bOK = true;
                break;
        }
    }
    else if ( GetViewShell() )
    {
        // The SW has its own undo in the View
        const SfxPoolItem *pRet = GetViewShell()->ExecuteSlot( rReq );
        if ( pRet )
            bOK = static_cast<const SfxBoolItem*>(pRet)->GetValue();
    }

    rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bOK ) );
    rReq.Done();
}

void SfxViewFrame::StateHistory_Impl( SfxItemSet &rSet )
{
    // Search for Undo-Manager
    SfxShell *pSh = GetDispatcher()->GetShell(0);
    if ( !pSh )
        // I'm just on reload and am yielding myself ...
        return;

    SfxUndoManager *pShUndoMgr = pSh->GetUndoManager();
    if ( !pShUndoMgr )
    {
        // The SW has its own undo in the View
        SfxWhichIter aIter( rSet );
        SfxViewShell *pViewSh = GetViewShell();
        if( !pViewSh ) return;
        for ( sal_uInt16 nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
            pViewSh->GetSlotState( nSID, nullptr, &rSet );
        return;
    }

    if ( pShUndoMgr->GetUndoActionCount() == 0 &&
         pShUndoMgr->GetRedoActionCount() == 0 &&
         pShUndoMgr->GetRepeatActionCount() == 0 )
        rSet.DisableItem( SID_CLEARHISTORY );

    if ( pShUndoMgr && pShUndoMgr->GetUndoActionCount() )
    {
        const SfxUndoAction* pAction = pShUndoMgr->GetUndoAction();
        SfxViewShell *pViewSh = GetViewShell();
        if (pViewSh && pAction->GetViewShellId() != pViewSh->GetViewShellId())
        {
            rSet.Put(SfxUInt32Item(SID_UNDO, static_cast<sal_uInt32>(SID_REPAIRPACKAGE)));
        }
        else
        {
            rSet.Put( SfxStringItem( SID_UNDO, SvtResId(STR_UNDO)+pShUndoMgr->GetUndoActionComment() ) );
        }
    }
    else
        rSet.DisableItem( SID_UNDO );

    if ( pShUndoMgr && pShUndoMgr->GetRedoActionCount() )
    {
        const SfxUndoAction* pAction = pShUndoMgr->GetRedoAction();
        SfxViewShell *pViewSh = GetViewShell();
        if (pViewSh && pAction->GetViewShellId() != pViewSh->GetViewShellId())
        {
            rSet.Put(SfxUInt32Item(SID_REDO, static_cast<sal_uInt32>(SID_REPAIRPACKAGE)));
        }
        else
        {
            rSet.Put( SfxStringItem( SID_REDO, SvtResId(STR_REDO)+pShUndoMgr->GetRedoActionComment() ) );
        }
    }
    else
        rSet.DisableItem( SID_REDO );

    SfxRepeatTarget *pTarget = pSh->GetRepeatTarget();
    if ( pShUndoMgr && pTarget && pShUndoMgr->GetRepeatActionCount() && pShUndoMgr->CanRepeat(*pTarget) )
        rSet.Put( SfxStringItem( SID_REPEAT, SvtResId(STR_REPEAT)+pShUndoMgr->GetRepeatActionComment(*pTarget) ) );
    else
        rSet.DisableItem( SID_REPEAT );
}

void SfxViewFrame::PopShellAndSubShells_Impl( SfxViewShell& i_rViewShell )
{
    i_rViewShell.PopSubShells_Impl();
    sal_uInt16 nLevel = m_pDispatcher->GetShellLevel( i_rViewShell );
    if ( nLevel != USHRT_MAX )
    {
        if ( nLevel )
        {
            // more sub shells on the stack, which were not affected by PopSubShells_Impl
            SfxShell *pSubShell = m_pDispatcher->GetShell( nLevel-1 );
            m_pDispatcher->Pop( *pSubShell, SfxDispatcherPopFlags::POP_UNTIL | SfxDispatcherPopFlags::POP_DELETE );
        }
        m_pDispatcher->Pop( i_rViewShell );
        m_pDispatcher->Flush();
    }

}

/*  [Description]

    This method empties the SfxViewFrame, i.e. takes the <SfxObjectShell>
    from the dispatcher and ends its <SfxListener> Relationship to this
    SfxObjectShell (by which they may even destroy themselves).

    Thus, by invoking ReleaseObjectShell() and  SetObjectShell() the
    SfxObjectShell can be replaced.

    Between RealeaseObjectShell() and SetObjectShell() can the control not
    be handed over to the system.

    [Cross-reference]

    <SfxViewFrame::SetObjectShell(SfxObjectShell&)>
*/
void SfxViewFrame::ReleaseObjectShell_Impl()
{
    DBG_ASSERT( m_xObjSh.is(), "no SfxObjectShell to release!" );

    GetFrame().ReleasingComponent_Impl();
    if ( GetWindow().HasChildPathFocus( true ) )
    {
        GetWindow().GrabFocus();
    }

    SfxViewShell *pDyingViewSh = GetViewShell();
    if ( pDyingViewSh )
    {
        PopShellAndSubShells_Impl( *pDyingViewSh );
        pDyingViewSh->DisconnectAllClients();
        SetViewShell_Impl(nullptr);
        delete pDyingViewSh;
    }
#ifdef DBG_UTIL
    else
        OSL_FAIL("No Shell");
#endif

    if ( m_xObjSh.is() )
    {
        m_pDispatcher->Pop( *m_xObjSh );
        SfxModule* pModule = m_xObjSh->GetModule();
        if( pModule )
            m_pDispatcher->RemoveShell_Impl( *pModule );
        m_pDispatcher->Flush();
        EndListening( *m_xObjSh );

        Notify( *m_xObjSh, SfxHint(SfxHintId::TitleChanged) );
        Notify( *m_xObjSh, SfxHint(SfxHintId::DocChanged) );

        if ( 1 == m_xObjSh->GetOwnerLockCount() && m_pImpl->bObjLocked && m_xObjSh->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
            m_xObjSh->DoClose();
        SfxObjectShellRef xDyingObjSh = m_xObjSh;
        m_xObjSh.clear();
        if( GetHasTitle() && m_pImpl->nDocViewNo )
            xDyingObjSh->GetNoSet_Impl().ReleaseIndex(m_pImpl->nDocViewNo-1);
        if ( m_pImpl->bObjLocked )
        {
            xDyingObjSh->OwnerLock( false );
            m_pImpl->bObjLocked = false;
        }
    }

    GetDispatcher()->SetDisableFlags( SfxDisableFlags::NONE );
}

bool SfxViewFrame::Close()
{

    DBG_ASSERT( GetFrame().IsClosing_Impl() || !GetFrame().GetFrameInterface().is(), "ViewFrame closed too early!" );

    // If no saving have been made up until now, then embedded Objects should
    // not be saved automatically anymore.
    if ( GetViewShell() )
        GetViewShell()->DisconnectAllClients();
    Broadcast( SfxHint( SfxHintId::Dying ) );

    if (SfxViewFrame::Current() == this)
        SfxViewFrame::SetViewFrame( nullptr );

    // Since the Dispatcher is emptied, it can not be used in any reasonable
    // manner, thus it is better to let the dispatcher be.
    GetDispatcher()->Lock(true);
    delete this;

    return true;
}

void SfxViewFrame::DoActivate( bool bUI )
{
    m_pDispatcher->DoActivate_Impl( bUI );
}

void SfxViewFrame::DoDeactivate(bool bUI, SfxViewFrame const * pNewFrame )
{
    m_pDispatcher->DoDeactivate_Impl( bUI, pNewFrame );
}

void SfxViewFrame::InvalidateBorderImpl( const SfxViewShell* pSh )
{
    if( pSh && !m_nAdjustPosPixelLock )
    {
        if ( GetViewShell() && GetWindow().IsVisible() )
        {
            if ( GetFrame().IsInPlace() )
            {
                return;
            }

            DoAdjustPosSizePixel( GetViewShell(), Point(),
                                            GetWindow().GetOutputSizePixel(),
                                            false );
        }
    }
}

void SfxViewFrame::SetBorderPixelImpl
(
    const SfxViewShell* pVSh,
    const SvBorder&     rBorder
)

{
    m_pImpl->aBorder = rBorder;

    if ( m_pImpl->bResizeInToOut && !GetFrame().IsInPlace() )
    {
        Size aSize = pVSh->GetWindow()->GetOutputSizePixel();
        if ( aSize.Width() && aSize.Height() )
        {
            aSize.AdjustWidth(rBorder.Left() + rBorder.Right() );
            aSize.AdjustHeight(rBorder.Top() + rBorder.Bottom() );

            Size aOldSize = GetWindow().GetOutputSizePixel();
            GetWindow().SetOutputSizePixel( aSize );
            vcl::Window* pParent = &GetWindow();
            while ( pParent->GetParent() )
                pParent = pParent->GetParent();
            Size aOuterSize = pParent->GetOutputSizePixel();
            aOuterSize.AdjustWidth( aSize.Width() - aOldSize.Width() );
            aOuterSize.AdjustHeight( aSize.Height() - aOldSize.Height() );
            pParent->SetOutputSizePixel( aOuterSize );
        }
    }
    else
    {
        tools::Rectangle aEditArea( Point(), GetWindow().GetOutputSizePixel() );
        aEditArea.AdjustLeft(rBorder.Left() );
        aEditArea.AdjustRight( -(rBorder.Right()) );
        aEditArea.AdjustTop(rBorder.Top() );
        aEditArea.AdjustBottom( -(rBorder.Bottom()) );
        pVSh->GetWindow()->SetPosSizePixel( aEditArea.TopLeft(), aEditArea.GetSize() );
    }
}

const SvBorder& SfxViewFrame::GetBorderPixelImpl() const
{
    return m_pImpl->aBorder;
}

void SfxViewFrame::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if(m_pImpl->bIsDowning)
        return;

    // we know only SfxEventHint or simple SfxHint
    if (const SfxEventHint* pEventHint = dynamic_cast<const SfxEventHint*>(&rHint))
    {
        // When the Document is loaded asynchronously, was the Dispatcher
        // set as ReadOnly, to what must be returned when the document itself
        // is not read only, and the loading is finished.
        switch ( pEventHint->GetEventId() )
        {
            case SfxEventHintId::ModifyChanged:
            {
                SfxBindings& rBind = GetBindings();
                rBind.Invalidate( SID_DOC_MODIFIED );
                rBind.Invalidate( SID_RELOAD );
                rBind.Invalidate( SID_EDITDOC );
                break;
            }

            case SfxEventHintId::OpenDoc:
            case SfxEventHintId::CreateDoc:
            {
                if ( !m_xObjSh.is() )
                    break;

                SfxBindings& rBind = GetBindings();
                rBind.Invalidate( SID_RELOAD );
                rBind.Invalidate( SID_EDITDOC );

                // inform about the community involvement
                const sal_Int64 nLastShown = officecfg::Setup::Product::LastTimeGetInvolvedShown::get();
                const sal_Int64 nNow = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                const sal_Int64 nPeriodSec(60 * 60 * 24 * 30); // 30 days in seconds
                bool bUpdateLastTimeGetInvolvedShown = false;

                if (nLastShown == 0)
                    bUpdateLastTimeGetInvolvedShown = true;
                else if (nPeriodSec < nNow && nLastShown < nNow - nPeriodSec)
                {
                    bUpdateLastTimeGetInvolvedShown = true;

                    VclPtr<SfxInfoBarWindow> pInfoBar = AppendInfoBar("getinvolved", SfxResId(STR_GET_INVOLVED_TEXT), InfoBarType::Info);

                    VclPtrInstance<PushButton> xGetInvolvedButton(&GetWindow());
                    xGetInvolvedButton->SetText(SfxResId(STR_GET_INVOLVED_BUTTON));
                    xGetInvolvedButton->SetSizePixel(xGetInvolvedButton->GetOptimalSize());
                    xGetInvolvedButton->SetClickHdl(LINK(this, SfxViewFrame, GetInvolvedHandler));
                    pInfoBar->addButton(xGetInvolvedButton);
                }

                if (bUpdateLastTimeGetInvolvedShown)
                {
                    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
                    officecfg::Setup::Product::LastTimeGetInvolvedShown::set(nNow, batch);
                    batch->commit();
                }

                // read-only infobar if necessary
                const SfxViewShell *pVSh;
                const SfxShell *pFSh;
                if ( m_xObjSh->IsReadOnly() &&
                    ! m_xObjSh->IsSecurityOptOpenReadOnly() &&
                    ( m_xObjSh->GetCreateMode() != SfxObjectCreateMode::EMBEDDED ||
                        (( pVSh = m_xObjSh->GetViewShell()) && (pFSh = pVSh->GetFormShell()) && pFSh->IsDesignMode())))
                {
                    bool bSignPDF = IsSignPDF(m_xObjSh);

                    auto pInfoBar = AppendInfoBar("readonly", SfxResId(bSignPDF ? STR_READONLY_PDF : STR_READONLY_DOCUMENT), InfoBarType::Info);
                    if (pInfoBar)
                    {
                        if (bSignPDF)
                        {
                            // SID_SIGNPDF opened a read-write PDF
                            // read-only for signing purposes.
                            VclPtrInstance<PushButton> xSignButton(&GetWindow());
                            xSignButton->SetText(SfxResId(STR_READONLY_SIGN));
                            xSignButton->SetSizePixel(xSignButton->GetOptimalSize());
                            xSignButton->SetClickHdl(LINK(this, SfxViewFrame, SignDocumentHandler));
                            pInfoBar->addButton(xSignButton);
                        }

                        VclPtrInstance<PushButton> xBtn(&GetWindow());
                        xBtn->SetText(SfxResId(STR_READONLY_EDIT));
                        xBtn->SetSizePixel(xBtn->GetOptimalSize());
                        xBtn->SetClickHdl(LINK(this, SfxViewFrame, SwitchReadOnlyHandler));
                        pInfoBar->addButton(xBtn);
                    }
                }

                if (SfxClassificationHelper::IsClassified(m_xObjSh->getDocProperties()))
                {
                    // Document has BAILS properties, display an infobar accordingly.
                    SfxClassificationHelper aHelper(m_xObjSh->getDocProperties());
                    aHelper.UpdateInfobar(*this);
                }

                break;
            }
            default: break;
        }
    }
    else
    {
        switch( rHint.GetId() )
        {
            case SfxHintId::ModeChanged:
            {
                UpdateTitle();

                if ( !m_xObjSh.is() )
                    break;

                // Switch r/o?
                SfxBindings& rBind = GetBindings();
                rBind.Invalidate( SID_RELOAD );
                SfxDispatcher *pDispat = GetDispatcher();
                bool bWasReadOnly = pDispat->GetReadOnly_Impl();
                bool bIsReadOnly = m_xObjSh->IsReadOnly();
                if ( bWasReadOnly != bIsReadOnly )
                {
                    // Then also TITLE_CHANGED
                    UpdateTitle();
                    rBind.Invalidate( SID_FILE_NAME );
                    rBind.Invalidate( SID_DOCINFO_TITLE );
                    rBind.Invalidate( SID_EDITDOC );

                    pDispat->GetBindings()->InvalidateAll(true);
                    pDispat->SetReadOnly_Impl( bIsReadOnly );

                    // Only force and Dispatcher-Update, if it is done next
                    // anyway, otherwise flickering or GPF is possibel since
                    // the Writer for example prefers in Resize perform some
                    // actions which has a SetReadOnlyUI in Dispatcher as a
                    // result!

                    if ( pDispat->IsUpdated_Impl() )
                        pDispat->Update_Impl(true);
                }

                Enable( !m_xObjSh->IsInModalMode() );
                break;
            }

            case SfxHintId::TitleChanged:
            {
                UpdateTitle();
                SfxBindings& rBind = GetBindings();
                rBind.Invalidate( SID_FILE_NAME );
                rBind.Invalidate( SID_DOCINFO_TITLE );
                rBind.Invalidate( SID_EDITDOC );
                rBind.Invalidate( SID_RELOAD );
                break;
            }

            case SfxHintId::DocumentRepair:
            {
                GetBindings().Invalidate( SID_DOC_REPAIR );
                break;
            }

            case SfxHintId::Deinitializing:
            {
                if (GetWindow().GetLOKNotifier())
                    GetWindow().ReleaseLOKNotifier();

                GetFrame().DoClose();
                break;
            }
            case SfxHintId::Dying:
                // when the Object is being deleted, destroy the view too
                if ( m_xObjSh.is() )
                    ReleaseObjectShell_Impl();
                else
                    GetFrame().DoClose();
                break;
            default: break;
        }
    }
}

IMPL_LINK_NOARG(SfxViewFrame, GetInvolvedHandler, Button*, void)
{
    try
    {
        sfx2::openUriExternally("https://hub.libreoffice.org/joinus", false);
    }
    catch (const Exception&)
    {
    }
}

IMPL_LINK(SfxViewFrame, SwitchReadOnlyHandler, Button*, pButton, void)
{
    if (m_xObjSh.is() && IsSignPDF(m_xObjSh))
    {
        SfxEditDocumentDialog aDialog(pButton->GetFrameWeld());
        if (aDialog.run() != RET_OK)
            return;
    }
    GetDispatcher()->Execute(SID_EDITDOC);
}

IMPL_LINK_NOARG(SfxViewFrame, SignDocumentHandler, Button*, void)
{
    GetDispatcher()->Execute(SID_SIGNATURE);
}

void SfxViewFrame::Construct_Impl( SfxObjectShell *pObjSh )
{
    m_pImpl->bResizeInToOut = true;
    m_pImpl->bObjLocked = false;
    m_pImpl->pFocusWin = nullptr;
    m_pImpl->nCurViewId = SFX_INTERFACE_NONE;
    m_pImpl->bReloading = false;
    m_pImpl->bIsDowning = false;
    m_pImpl->bModal = false;
    m_pImpl->bEnabled = true;
    m_pImpl->nDocViewNo = 0;
    m_pImpl->aMargin = Size( -1, -1 );
    m_pImpl->pWindow = nullptr;

    SetPool( &SfxGetpApp()->GetPool() );
    m_pDispatcher.reset( new SfxDispatcher(this) );
    if ( !GetBindings().GetDispatcher() )
        GetBindings().SetDispatcher( m_pDispatcher.get() );

    m_xObjSh = pObjSh;
    if ( m_xObjSh.is() && m_xObjSh->IsPreview() )
        GetDispatcher()->SetQuietMode_Impl( true );

    if ( pObjSh )
    {
        m_pDispatcher->Push( *SfxGetpApp() );
        SfxModule* pModule = m_xObjSh->GetModule();
        if( pModule )
            m_pDispatcher->Push( *pModule );
        m_pDispatcher->Push( *this );
        m_pDispatcher->Push( *pObjSh );
        m_pDispatcher->Flush();
        StartListening( *pObjSh );
        Notify( *pObjSh, SfxHint(SfxHintId::TitleChanged) );
        Notify( *pObjSh, SfxHint(SfxHintId::DocChanged) );
        m_pDispatcher->SetReadOnly_Impl( pObjSh->IsReadOnly() );
    }
    else
    {
        m_pDispatcher->Push( *SfxGetpApp() );
        m_pDispatcher->Push( *this );
        m_pDispatcher->Flush();
    }

    SfxViewFrameArr_Impl &rViewArr = SfxGetpApp()->GetViewFrames_Impl();
    rViewArr.push_back( this );
}

/*  [Description]

    Constructor of SfxViewFrame for a <SfxObjectShell> from the Resource.
    The 'nViewId' to the created <SfxViewShell> can be returned.
    (default is the SfxViewShell-Subclass that was registered first).
*/
SfxViewFrame::SfxViewFrame
(
    SfxFrame&           rFrame,
    SfxObjectShell*     pObjShell
)
    : m_pImpl( new SfxViewFrame_Impl( rFrame ) )
    , m_pDispatcher(nullptr)
    , m_pBindings( new SfxBindings )
    , m_nAdjustPosPixelLock( 0 )
{

    rFrame.SetCurrentViewFrame_Impl( this );
    rFrame.SetHasTitle( true );
    Construct_Impl( pObjShell );

    m_pImpl->pWindow = VclPtr<SfxFrameViewWindow_Impl>::Create( this, rFrame.GetWindow() );
    m_pImpl->pWindow->SetSizePixel( rFrame.GetWindow().GetOutputSizePixel() );
    rFrame.SetOwnsBindings_Impl( true );
    rFrame.CreateWorkWindow_Impl();
}

SfxViewFrame::~SfxViewFrame()
{
    m_pImpl->bIsDowning = true;

    if ( SfxViewFrame::Current() == this )
        SfxViewFrame::SetViewFrame( nullptr );

    ReleaseObjectShell_Impl();

    if ( GetFrame().OwnsBindings_Impl() )
        // The Bindings delete the Frame!
        KillDispatcher_Impl();

    m_pImpl->pWindow.disposeAndClear();
    m_pImpl->pFocusWin.clear();

    if ( GetFrame().GetCurrentViewFrame() == this )
        GetFrame().SetCurrentViewFrame_Impl( nullptr );

    // Unregister from the Frame List.
    SfxApplication *pSfxApp = SfxApplication::Get();
    if (pSfxApp)
    {
        SfxViewFrameArr_Impl &rFrames = pSfxApp->GetViewFrames_Impl();
        SfxViewFrameArr_Impl::iterator it = std::find( rFrames.begin(), rFrames.end(), this );
        rFrames.erase( it );
    }

    // Delete Member
    KillDispatcher_Impl();
}

// Remove and delete the Dispatcher.
void SfxViewFrame::KillDispatcher_Impl()
{

    SfxModule* pModule = m_xObjSh.is() ? m_xObjSh->GetModule() : nullptr;
    if ( m_xObjSh.is() )
        ReleaseObjectShell_Impl();
    if ( m_pDispatcher )
    {
        if( pModule )
            m_pDispatcher->Pop( *pModule, SfxDispatcherPopFlags::POP_UNTIL );
        else
            m_pDispatcher->Pop( *this );
        m_pDispatcher.reset();
    }
}

SfxViewFrame* SfxViewFrame::Current()
{
    SfxApplication* pApp = SfxApplication::Get();
    return pApp ? pApp->Get_Impl()->pViewFrame : nullptr;
}

// returns the first window of spec. type viewing the specified doc.
SfxViewFrame* SfxViewFrame::GetFirst
(
    const SfxObjectShell*   pDoc,
    bool                    bOnlyIfVisible
)
{
    SfxApplication *pSfxApp = SfxApplication::Get();
    if (!pSfxApp)
        return nullptr;

    SfxViewFrameArr_Impl &rFrames = pSfxApp->GetViewFrames_Impl();

    // search for a SfxDocument of the specified type
    for (SfxViewFrame* pFrame : rFrames)
    {
        if  (   ( !pDoc || pDoc == pFrame->GetObjectShell() )
            &&  ( !bOnlyIfVisible || pFrame->IsVisible() )
            )
            return pFrame;
    }

    return nullptr;
}

// returns the next window of spec. type viewing the specified doc.
SfxViewFrame* SfxViewFrame::GetNext
(
    const SfxViewFrame&     rPrev,
    const SfxObjectShell*   pDoc,
    bool                    bOnlyIfVisible
)
{
    SfxApplication *pSfxApp = SfxApplication::Get();
    if (!pSfxApp)
        return nullptr;

    SfxViewFrameArr_Impl &rFrames = pSfxApp->GetViewFrames_Impl();

    // refind the specified predecessor
    size_t nPos;
    for ( nPos = 0; nPos < rFrames.size(); ++nPos )
        if ( rFrames[nPos] == &rPrev )
            break;

    // search for a Frame of the specified type
    for ( ++nPos; nPos < rFrames.size(); ++nPos )
    {
        SfxViewFrame *pFrame = rFrames[nPos];
        if  (   ( !pDoc || pDoc == pFrame->GetObjectShell() )
            &&  ( !bOnlyIfVisible || pFrame->IsVisible() )
            )
            return pFrame;
    }
    return nullptr;
}

SfxProgress* SfxViewFrame::GetProgress() const
{
    SfxObjectShell *pObjSh = m_xObjSh.get();
    return pObjSh ? pObjSh->GetProgress() : nullptr;
}

void SfxViewFrame::DoAdjustPosSizePixel //! divide on Inner.../Outer...
(
    SfxViewShell*   pSh,
    const Point&    rPos,
    const Size&     rSize,
    bool inplaceEditModeChange
)
{

    // Components do not use this Method!
    if( pSh && pSh->GetWindow() && !m_nAdjustPosPixelLock )
    {
        m_nAdjustPosPixelLock++;
        if ( m_pImpl->bResizeInToOut )
            pSh->InnerResizePixel( rPos, rSize, inplaceEditModeChange );
        else
            pSh->OuterResizePixel( rPos, rSize );
        m_nAdjustPosPixelLock--;
    }
}

bool SfxViewFrameItem::operator==( const SfxPoolItem &rItem ) const
{
    return dynamic_cast<const SfxViewFrameItem&>(rItem).pFrame == pFrame;
}

SfxPoolItem* SfxViewFrameItem::Clone( SfxItemPool *) const
{
    return new SfxViewFrameItem( *this );
}

void SfxViewFrame::SetViewShell_Impl( SfxViewShell *pVSh )
/*  [Description]

    Internal Method to set the current <SfxViewShell> Instance,
    that is active int this SfxViewFrame at the moment.
*/
{
    SfxShell::SetViewShell_Impl( pVSh );

    // Hack: InPlaceMode
    if ( pVSh )
        m_pImpl->bResizeInToOut = false;
}

void SfxViewFrame::ForceOuterResize_Impl()
{
    m_pImpl->bResizeInToOut = true;
}

void SfxViewFrame::GetDocNumber_Impl()
{
    DBG_ASSERT( GetObjectShell(), "No Document!" );
    GetObjectShell()->SetNamedVisibility_Impl();
    m_pImpl->nDocViewNo = GetObjectShell()->GetNoSet_Impl().GetFreeIndex()+1;
}

void SfxViewFrame::Enable( bool bEnable )
{
    if ( bEnable != m_pImpl->bEnabled )
    {
        m_pImpl->bEnabled = bEnable;

        vcl::Window *pWindow = &GetFrame().GetWindow();
        if ( !bEnable )
            m_pImpl->bWindowWasEnabled = pWindow->IsInputEnabled();
        if ( !bEnable || m_pImpl->bWindowWasEnabled )
            pWindow->EnableInput( bEnable );

        // cursor and focus
        SfxViewShell* pViewSh = GetViewShell();
        if ( bEnable )
        {
            // show cursor
            if ( pViewSh )
                pViewSh->ShowCursor();
        }
        else
        {
            // hide cursor
            if ( pViewSh )
                pViewSh->ShowCursor(false);
        }
    }
}

/*  [Description]

    This method makes the Frame-Window visible and before transmits the
    window name. In addition, the document is held. In general one can never
    show the window directly!
*/
void SfxViewFrame::Show()
{
    // First lock the objectShell so that UpdateTitle() is valid:
    // IsVisible() == true (:#)
    if ( m_xObjSh.is() )
    {
        m_xObjSh->GetMedium()->GetItemSet()->ClearItem( SID_HIDDEN );
        if ( !m_pImpl->bObjLocked )
            LockObjectShell_Impl();

        // Adjust Doc-Shell title number, get unique view-no
        if ( 0 == m_pImpl->nDocViewNo  )
        {
            GetDocNumber_Impl();
            UpdateTitle();
        }
    }
    else
        UpdateTitle();

    // Display Frame-window, but only if the ViewFrame has no window of its
    // own or if it does not contain a Component
    GetWindow().Show();
    GetFrame().GetWindow().Show();
}


bool SfxViewFrame::IsVisible() const
{
    return m_pImpl->bObjLocked;
}


void SfxViewFrame::LockObjectShell_Impl()
{
    DBG_ASSERT( !m_pImpl->bObjLocked, "Wrong Locked status!" );

    DBG_ASSERT( GetObjectShell(), "No Document!" );
    GetObjectShell()->OwnerLock(true);
    m_pImpl->bObjLocked = true;
}


void SfxViewFrame::MakeActive_Impl( bool bGrabFocus )
{
    if ( GetViewShell() && !GetFrame().IsClosing_Impl() )
    {
        if ( IsVisible() )
        {
            if ( GetViewShell() )
            {
                bool bPreview = false;
                if ( GetObjectShell()->IsPreview() )
                {
                    bPreview = true;
                }

                css::uno::Reference< css::frame::XFrame > xFrame = GetFrame().GetFrameInterface();
                if ( !bPreview )
                {
                    SetViewFrame( this );
                    GetBindings().SetActiveFrame( css::uno::Reference< css::frame::XFrame >() );
                    uno::Reference< frame::XFramesSupplier > xSupp( xFrame, uno::UNO_QUERY );
                    if ( xSupp.is() )
                        xSupp->setActiveFrame( uno::Reference < frame::XFrame >() );

                    css::uno::Reference< css::awt::XWindow > xContainerWindow = xFrame->getContainerWindow();
                    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xContainerWindow);
                    if (pWindow && pWindow->HasChildPathFocus() && bGrabFocus)
                    {
                        SfxInPlaceClient *pCli = GetViewShell()->GetUIActiveClient();
                        if ( !pCli || !pCli->IsObjectUIActive() )
                                GetFrame().GrabFocusOnComponent_Impl();
                    }
                }
                else
                {
                    GetBindings().SetDispatcher( GetDispatcher() );
                    GetBindings().SetActiveFrame( css::uno::Reference< css::frame::XFrame > () );
                    GetDispatcher()->Update_Impl();
                }
            }
        }
    }
}

SfxObjectShell* SfxViewFrame::GetObjectShell()
{
    return m_xObjSh.get();
}

const Size& SfxViewFrame::GetMargin_Impl() const
{
    return m_pImpl->aMargin;
}

SfxViewFrame* SfxViewFrame::LoadViewIntoFrame_Impl_NoThrow( const SfxObjectShell& i_rDoc, const Reference< XFrame >& i_rFrame,
                                                   const SfxInterfaceId i_nViewId, const bool i_bHidden )
{
    Reference< XFrame > xFrame( i_rFrame );
    bool bOwnFrame = false;
    SfxViewShell* pSuccessView = nullptr;
    try
    {
        if ( !xFrame.is() )
        {
            Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );

            if ( !i_bHidden )
            {
                try
                {
                    // if there is a backing component, use it
                    ::framework::FrameListAnalyzer aAnalyzer( xDesktop, Reference< XFrame >(), FrameAnalyzerFlags::BackingComponent );

                    if ( aAnalyzer.m_xBackingComponent.is() )
                        xFrame = aAnalyzer.m_xBackingComponent;
                }
                catch( uno::Exception& )
                {}
            }

            if ( !xFrame.is() )
                xFrame.set( xDesktop->findFrame( "_blank", 0 ), UNO_SET_THROW );

            bOwnFrame = true;
        }

        pSuccessView = LoadViewIntoFrame_Impl(
            i_rDoc,
            xFrame,
            Sequence< PropertyValue >(),    // means "reuse existing model's args"
            i_nViewId,
            i_bHidden
        );

        if ( bOwnFrame && !i_bHidden )
        {
            // ensure the frame/window is visible
            Reference< XWindow > xContainerWindow( xFrame->getContainerWindow(), UNO_SET_THROW );
            xContainerWindow->setVisible( true );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("sfx.view");
    }

    if ( pSuccessView )
        return pSuccessView->GetViewFrame();

    if ( bOwnFrame )
    {
        try
        {
            xFrame->dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("sfx.view");
        }
    }

    return nullptr;
}

SfxViewShell* SfxViewFrame::LoadViewIntoFrame_Impl( const SfxObjectShell& i_rDoc, const Reference< XFrame >& i_rFrame,
                                           const Sequence< PropertyValue >& i_rLoadArgs, const SfxInterfaceId i_nViewId,
                                           const bool i_bHidden )
{
    Reference< XModel > xDocument( i_rDoc.GetModel(), UNO_SET_THROW );

    ::comphelper::NamedValueCollection aTransformLoadArgs( i_rLoadArgs.getLength() ? i_rLoadArgs : xDocument->getArgs() );
    aTransformLoadArgs.put( "Model", xDocument );
    if ( i_nViewId )
        aTransformLoadArgs.put( "ViewId", sal_uInt16( i_nViewId ) );
    if ( i_bHidden )
        aTransformLoadArgs.put( "Hidden", i_bHidden );
    else
        aTransformLoadArgs.remove( "Hidden" );

    Reference< XComponentLoader > xLoader( i_rFrame, UNO_QUERY_THROW );
    xLoader->loadComponentFromURL( "private:object", "_self", 0,
        aTransformLoadArgs.getPropertyValues() );

    SfxViewShell* pViewShell = SfxViewShell::Get( i_rFrame->getController() );
    ENSURE_OR_THROW( pViewShell,
        "SfxViewFrame::LoadViewIntoFrame_Impl: loading an SFX doc into a frame resulted in a non-SFX view - quite impossible" );
    return pViewShell;
}

SfxViewFrame* SfxViewFrame::LoadHiddenDocument( SfxObjectShell const & i_rDoc, SfxInterfaceId i_nViewId )
{
    return LoadViewIntoFrame_Impl_NoThrow( i_rDoc, Reference< XFrame >(), i_nViewId, true );
}

SfxViewFrame* SfxViewFrame::LoadDocument( SfxObjectShell const & i_rDoc, SfxInterfaceId i_nViewId )
{
    return LoadViewIntoFrame_Impl_NoThrow( i_rDoc, Reference< XFrame >(), i_nViewId, false );
}

SfxViewFrame* SfxViewFrame::LoadDocumentIntoFrame( SfxObjectShell const & i_rDoc, const Reference< XFrame >& i_rTargetFrame )
{
    return LoadViewIntoFrame_Impl_NoThrow( i_rDoc, i_rTargetFrame, SFX_INTERFACE_NONE, false );
}

SfxViewFrame* SfxViewFrame::LoadDocumentIntoFrame( SfxObjectShell const & i_rDoc, const SfxFrameItem* i_pFrameItem, SfxInterfaceId i_nViewId )
{
    return LoadViewIntoFrame_Impl_NoThrow( i_rDoc, i_pFrameItem && i_pFrameItem->GetFrame() ? i_pFrameItem->GetFrame()->GetFrameInterface() : nullptr, i_nViewId, false );
}

SfxViewFrame* SfxViewFrame::DisplayNewDocument( SfxObjectShell const & i_rDoc, const SfxRequest& i_rCreateDocRequest )
{
    const SfxUnoFrameItem* pFrameItem = i_rCreateDocRequest.GetArg<SfxUnoFrameItem>(SID_FILLFRAME);
    const SfxBoolItem* pHiddenItem = i_rCreateDocRequest.GetArg<SfxBoolItem>(SID_HIDDEN);

    return LoadViewIntoFrame_Impl_NoThrow(
        i_rDoc,
        pFrameItem ? pFrameItem->GetFrame() : nullptr,
        SFX_INTERFACE_NONE,
        pHiddenItem && pHiddenItem->GetValue()
    );
}

SfxViewFrame* SfxViewFrame::Get( const Reference< XController>& i_rController, const SfxObjectShell* i_pDoc )
{
    if ( !i_rController.is() )
        return nullptr;

    const SfxObjectShell* pDoc = i_pDoc;
    if ( !pDoc )
    {
        Reference< XModel > xDocument( i_rController->getModel() );
        for (   pDoc = SfxObjectShell::GetFirst( nullptr, false );
                pDoc;
                pDoc = SfxObjectShell::GetNext( *pDoc, nullptr, false )
            )
        {
            if ( pDoc->GetModel() == xDocument )
                break;
        }
    }

    SfxViewFrame* pViewFrame = nullptr;
    for (   pViewFrame = SfxViewFrame::GetFirst( pDoc, false );
            pViewFrame;
            pViewFrame = SfxViewFrame::GetNext( *pViewFrame, pDoc, false )
        )
    {
        if ( pViewFrame->GetViewShell()->GetController() == i_rController )
            break;
    }

    return pViewFrame;
}

void SfxViewFrame::SaveCurrentViewData_Impl( const SfxInterfaceId i_nNewViewId )
{
    SfxViewShell* pCurrentShell = GetViewShell();
    ENSURE_OR_RETURN_VOID( pCurrentShell != nullptr, "SfxViewFrame::SaveCurrentViewData_Impl: no current view shell -> no current view data!" );

    // determine the logical (API) view name
    const SfxObjectFactory& rDocFactory( pCurrentShell->GetObjectShell()->GetFactory() );
    const sal_uInt16 nCurViewNo = rDocFactory.GetViewNo_Impl( GetCurViewId(), 0 );
    const OUString sCurrentViewName = rDocFactory.GetViewFactory( nCurViewNo ).GetAPIViewName();
    const sal_uInt16 nNewViewNo = rDocFactory.GetViewNo_Impl( i_nNewViewId, 0 );
    const OUString sNewViewName = rDocFactory.GetViewFactory( nNewViewNo ).GetAPIViewName();
    if ( sCurrentViewName.isEmpty() || sNewViewName.isEmpty() )
    {
        // can't say anything about the view, the respective application did not yet migrate its code to
        // named view factories => bail out
        OSL_FAIL( "SfxViewFrame::SaveCurrentViewData_Impl: views without API names? Shouldn't happen anymore?" );
        return;
    }
    SAL_WARN_IF(sNewViewName == sCurrentViewName, "sfx.view", "SfxViewFrame::SaveCurrentViewData_Impl: suspicious: new and old view name are identical!");

    // save the view data only when we're moving from a non-print-preview to the print-preview view
    if ( sNewViewName != "PrintPreview" )
        return;

    // retrieve the view data from the view
    Sequence< PropertyValue > aViewData;
    pCurrentShell->WriteUserDataSequence( aViewData );

    try
    {
        // retrieve view data (for *all* views) from the model
        const Reference< XController > xController( pCurrentShell->GetController(), UNO_SET_THROW );
        const Reference< XViewDataSupplier > xViewDataSupplier( xController->getModel(), UNO_QUERY_THROW );
        const Reference< XIndexContainer > xViewData( xViewDataSupplier->getViewData(), UNO_QUERY_THROW );

        // look up the one view data item which corresponds to our current view, and remove it
        const sal_Int32 nCount = xViewData->getCount();
        for ( sal_Int32 i=0; i<nCount; ++i )
        {
            const ::comphelper::NamedValueCollection aCurViewData( xViewData->getByIndex(i) );
            const OUString sViewId( aCurViewData.getOrDefault( "ViewId", OUString() ) );
            if ( sViewId.isEmpty() )
                continue;

            const SfxViewFactory* pViewFactory = rDocFactory.GetViewFactoryByViewName( sViewId );
            if ( pViewFactory == nullptr )
                continue;

            if ( pViewFactory->GetOrdinal() == GetCurViewId() )
            {
                xViewData->removeByIndex(i);
                break;
            }
        }

        // then replace it with the most recent view data we just obtained
        xViewData->insertByIndex( 0, makeAny( aViewData ) );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("sfx.view");
    }
}

/*  [Description]

    Internal Method for switching to another <SfxViewShell> subclass,
    which should be created in this SfxMDIFrame. If no SfxViewShell exist
    in this SfxMDIFrame, then one will first be created.


    [Return Value]

    bool                        true
                                requested SfxViewShell was created and a
                                possibly existing one deleted

                                false
                                SfxViewShell requested could not be created,
                                the existing SfxViewShell thus continue to exist
*/
bool SfxViewFrame::SwitchToViewShell_Impl
(
    sal_uInt16  nViewIdOrNo,    /*  > 0
                                Registration-Id of the View, to which the
                                method should switch, for example the one
                                that will be created.

                                == 0
                                First use the Default view. */

    bool        bIsIndex        /*  true
                                'nViewIdOrNo' is no Registration-Id instead
                                an Index of <SfxViewFrame> in <SfxObjectShell>.
                                */
)
{
    try
    {
        ENSURE_OR_THROW( GetObjectShell() != nullptr, "not possible without a document" );

        // if we already have a view shell, remove it
        SfxViewShell* pOldSh = GetViewShell();
        OSL_PRECOND( pOldSh, "SfxViewFrame::SwitchToViewShell_Impl: that's called *switch* (not for *initial-load*) for a reason" );
        if ( pOldSh )
        {
            // ask whether it can be closed
            if ( !pOldSh->PrepareClose() )
                return false;

            // remove sub shells from Dispatcher before switching to new ViewShell
            PopShellAndSubShells_Impl( *pOldSh );
        }

        GetBindings().ENTERREGISTRATIONS();
        LockAdjustPosSizePixel();

        // ID of the new view
        SfxObjectFactory& rDocFact = GetObjectShell()->GetFactory();
        const SfxInterfaceId nViewId = ( bIsIndex || !nViewIdOrNo ) ? rDocFact.GetViewFactory( nViewIdOrNo ).GetOrdinal() : SfxInterfaceId(nViewIdOrNo);

        // save the view data of the old view, so it can be restored later on (when needed)
        SaveCurrentViewData_Impl( nViewId );

        // create and load new ViewShell
        SfxViewShell* pNewSh = LoadViewIntoFrame_Impl(
            *GetObjectShell(),
            GetFrame().GetFrameInterface(),
            Sequence< PropertyValue >(),    // means "reuse existing model's args"
            nViewId,
            false
        );

        // allow resize events to be processed
        UnlockAdjustPosSizePixel();

        if ( GetWindow().IsReallyVisible() )
            DoAdjustPosSizePixel( pNewSh, Point(), GetWindow().GetOutputSizePixel(), false );

        GetBindings().LEAVEREGISTRATIONS();
        delete pOldSh;
    }
    catch ( const css::uno::Exception& )
    {
        // the SfxCode is not able to cope with exceptions thrown while creating views
        // the code will crash in the stack unwinding procedure, so we shouldn't let exceptions go through here
        DBG_UNHANDLED_EXCEPTION("sfx.view");
        return false;
    }

    DBG_ASSERT( SfxGetpApp()->GetViewFrames_Impl().size() == SfxGetpApp()->GetViewShells_Impl().size(), "Inconsistent view arrays!" );
    return true;
}

void SfxViewFrame::SetCurViewId_Impl( const SfxInterfaceId i_nID )
{
    m_pImpl->nCurViewId = i_nID;
}

SfxInterfaceId SfxViewFrame::GetCurViewId() const
{
    return m_pImpl->nCurViewId;
}

/*  [Description]

    Internal method to run the slot for the <SfxShell> Subclass in the
    SfxViewFrame <SVIDL> described slots.
*/
void SfxViewFrame::ExecView_Impl
(
    SfxRequest& rReq        // The executable <SfxRequest>
)
{

    // If the Shells are just being replaced...
    if ( !GetObjectShell() || !GetViewShell() )
        return;

    switch ( rReq.GetSlot() )
    {
        case SID_TERMINATE_INPLACEACTIVATION :
        {
            SfxInPlaceClient* pClient = GetViewShell()->GetUIActiveClient();
            if ( pClient )
                pClient->DeactivateObject();
            break;
        }

        case SID_VIEWSHELL:
        {
            const SfxPoolItem *pItem = nullptr;
            if  (   rReq.GetArgs()
                &&  SfxItemState::SET == rReq.GetArgs()->GetItemState( SID_VIEWSHELL, false, &pItem )
                )
            {
                const sal_uInt16 nViewId = static_cast< const SfxUInt16Item* >( pItem )->GetValue();
                bool bSuccess = SwitchToViewShell_Impl( nViewId );
                rReq.SetReturnValue( SfxBoolItem( 0, bSuccess ) );
            }
            break;
        }

        case SID_VIEWSHELL0:
        case SID_VIEWSHELL1:
        case SID_VIEWSHELL2:
        case SID_VIEWSHELL3:
        case SID_VIEWSHELL4:
        {
            const sal_uInt16 nViewNo = rReq.GetSlot() - SID_VIEWSHELL0;
            bool bSuccess = SwitchToViewShell_Impl( nViewNo, true );
            rReq.SetReturnValue( SfxBoolItem( 0, bSuccess ) );
            break;
        }

        case SID_NEWWINDOW:
        {
            // Hack. at the moment a virtual Function
            if ( !GetViewShell()->NewWindowAllowed() )
            {
                OSL_FAIL( "You should have disabled the 'Window/New Window' slot!" );
                return;
            }

            // Get ViewData of FrameSets recursively.
            GetFrame().GetViewData_Impl();
            SfxMedium* pMed = GetObjectShell()->GetMedium();

            // do not open the new window hidden
            pMed->GetItemSet()->ClearItem( SID_HIDDEN );

            // the view ID (optional arg. TODO: this is currently not supported in the slot definition ...)
            const SfxUInt16Item* pViewIdItem = rReq.GetArg<SfxUInt16Item>(SID_VIEW_ID);
            const SfxInterfaceId nViewId = pViewIdItem ? SfxInterfaceId(pViewIdItem->GetValue()) : GetCurViewId();

            Reference < XFrame > xFrame;
            // the frame (optional arg. TODO: this is currently not supported in the slot definition ...)
            const SfxUnoFrameItem* pFrameItem = rReq.GetArg<SfxUnoFrameItem>(SID_FILLFRAME);
            if ( pFrameItem )
                xFrame = pFrameItem->GetFrame();

            LoadViewIntoFrame_Impl_NoThrow( *GetObjectShell(), xFrame, nViewId, false );

            rReq.Done();
            break;
        }

        case SID_OBJECT:
        {
            const SfxInt16Item* pItem = rReq.GetArg<SfxInt16Item>(SID_OBJECT);

            SfxViewShell *pViewShell = GetViewShell();
            if ( pViewShell && pItem )
            {
                pViewShell->DoVerb( pItem->GetValue() );
                rReq.Done();
                break;
            }
        }
    }
}

/* TODO as96863:
        This method try to collect information about the count of currently open documents.
        But the algorithm is implemented very simple ...
        E.g. hidden documents should be ignored here ... but they are counted.
        TODO: export special helper "framework::FrameListAnalyzer" within the framework module
        and use it here.
*/
bool impl_maxOpenDocCountReached()
{
    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    boost::optional<sal_Int32> x(officecfg::Office::Common::Misc::MaxOpenDocuments::get(xContext));
    // NIL means: count of allowed documents = infinite !
    if (!x)
        return false;
    sal_Int32 nMaxDocs(x.get());
    sal_Int32 nOpenDocs = 0;

    css::uno::Reference< css::frame::XDesktop2 >  xDesktop = css::frame::Desktop::create(xContext);
    css::uno::Reference< css::container::XIndexAccess > xCont(xDesktop->getFrames(), css::uno::UNO_QUERY_THROW);

    sal_Int32 c = xCont->getCount();
    sal_Int32 i = 0;

    for (i=0; i<c; ++i)
    {
        try
        {
            css::uno::Reference< css::frame::XFrame > xFrame;
            xCont->getByIndex(i) >>= xFrame;
            if ( ! xFrame.is())
                continue;

            // a) do not count the help window
            if ( xFrame->getName() == "OFFICE_HELP_TASK" )
                continue;

            // b) count all other frames
            ++nOpenDocs;
        }
        catch(const css::uno::Exception&)
            // A IndexOutOfBoundException can happen in multithreaded
            // environments, where any other thread can change this
            // container !
            { continue; }
    }

    return (nOpenDocs >= nMaxDocs);
}

/*  [Description]

    This internal method returns in 'rSet' the Status for the  <SfxShell>
    Subclass SfxViewFrame in the <SVIDL> described <Slots>.

    Thus exactly those Slots-IDs that are recognized as being invalid by Sfx
    are included as Which-ranges in 'rSet'. If there exists a mapping for
    single slot-IDs of the <SfxItemPool> set in the shell, then the respective
    Which-IDs are used so that items can be replaced directly with a working
    Core::sun::com::star::script::Engine of the Which-IDs if possible. .
*/
void SfxViewFrame::StateView_Impl
(
    SfxItemSet&     rSet            /*  empty <SfxItemSet> with <Which-Ranges>,
                                        which describes the Slot Ids */
)
{

    SfxObjectShell *pDocSh = GetObjectShell();

    if ( !pDocSh )
        // I'm just on reload and am yielding myself ...
        return;

    const sal_uInt16 *pRanges = rSet.GetRanges();
    assert(pRanges && "Set with no Range");
    while ( *pRanges )
    {
        sal_uInt16 nStartWhich = *pRanges++;
        sal_uInt16 nEndWhich = *pRanges++;
        for ( sal_uInt16 nWhich = nStartWhich; nWhich <= nEndWhich; ++nWhich )
        {
            switch(nWhich)
            {
                case SID_VIEWSHELL:
                {
                    rSet.Put( SfxUInt16Item( nWhich, sal_uInt16(m_pImpl->nCurViewId )) );
                    break;
                }

                case SID_VIEWSHELL0:
                case SID_VIEWSHELL1:
                case SID_VIEWSHELL2:
                case SID_VIEWSHELL3:
                case SID_VIEWSHELL4:
                {
                    sal_uInt16 nViewNo = nWhich - SID_VIEWSHELL0;
                    if ( GetObjectShell()->GetFactory().GetViewFactoryCount() >
                         nViewNo && !GetObjectShell()->IsInPlaceActive() )
                    {
                        SfxViewFactory &rViewFactory =
                            GetObjectShell()->GetFactory().GetViewFactory(nViewNo);
                        rSet.Put( SfxBoolItem(
                            nWhich, m_pImpl->nCurViewId == rViewFactory.GetOrdinal() ) );
                    }
                    else
                        rSet.DisableItem( nWhich );
                    break;
                }

                case SID_NEWWINDOW:
                {
                    if  (   !GetViewShell()->NewWindowAllowed()
                        ||  impl_maxOpenDocCountReached()
                        )
                        rSet.DisableItem( nWhich );
                    break;
                }
            }
        }
    }
}


void SfxViewFrame::ToTop()
{
    GetFrame().Appear();
}


/*  [Description]

    GetFrame returns the Frame, in which the ViewFrame is located.
*/
SfxFrame& SfxViewFrame::GetFrame() const
{
    return m_pImpl->rFrame;
}

SfxViewFrame* SfxViewFrame::GetTopViewFrame() const
{
    return GetFrame().GetCurrentViewFrame();
}

vcl::Window& SfxViewFrame::GetWindow() const
{
    return m_pImpl->pWindow ? *m_pImpl->pWindow : GetFrame().GetWindow();
}

bool SfxViewFrame::DoClose()
{
    return GetFrame().DoClose();
}

OUString SfxViewFrame::GetActualPresentationURL_Impl() const
{
    if ( m_xObjSh.is() )
        return m_xObjSh->GetMedium()->GetName();
    return OUString();
}

void SfxViewFrame::SetModalMode( bool bModal )
{
    // no real modality for LOK
    if (comphelper::LibreOfficeKit::isActive())
        return;

    m_pImpl->bModal = bModal;
    if ( m_xObjSh.is() )
    {
        for ( SfxViewFrame* pFrame = SfxViewFrame::GetFirst( m_xObjSh.get() );
              !bModal && pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, m_xObjSh.get() ) )
            bModal = pFrame->m_pImpl->bModal;
        m_xObjSh->SetModalMode_Impl( bModal );
    }
}

bool SfxViewFrame::IsInModalMode() const
{
    return m_pImpl->bModal || GetFrame().GetWindow().IsInModalMode();
}

void SfxViewFrame::Resize( bool bForce )
{
    Size aSize = GetWindow().GetOutputSizePixel();
    if ( bForce || aSize != m_pImpl->aSize )
    {
        m_pImpl->aSize = aSize;
        SfxViewShell *pShell = GetViewShell();
        if ( pShell )
        {
            if ( GetFrame().IsInPlace() )
            {
                Point aPoint = GetWindow().GetPosPixel();
                DoAdjustPosSizePixel( pShell, aPoint, aSize, true );
            }
            else
            {
                DoAdjustPosSizePixel( pShell, Point(), aSize, false );
            }
        }
    }
}

#define LINE_SEP 0x0A

void CutLines( OUString& rStr, sal_Int32 nStartLine, sal_Int32 nLines )
{
    sal_Int32 nStartPos = 0;
    sal_Int32 nLine = 0;
    while ( nLine < nStartLine )
    {
        nStartPos = rStr.indexOf( LINE_SEP, nStartPos );
        if( nStartPos == -1 )
            break;
        nStartPos++;    // not the \n.
        nLine++;
    }

    SAL_WARN_IF(nStartPos == -1, "sfx.view", "CutLines: Start row not found!");

    if ( nStartPos != -1 )
    {
        sal_Int32 nEndPos = nStartPos;
        for ( sal_Int32 i = 0; i < nLines; i++ )
            nEndPos = rStr.indexOf( LINE_SEP, nEndPos+1 );

        if ( nEndPos == -1 ) // Can happen at the last row.
            nEndPos = rStr.getLength();
        else
            nEndPos++;

        rStr = rStr.copy( 0, nStartPos ) + rStr.copy( nEndPos );
    }
    // erase trailing lines
    if ( nStartPos != -1 )
    {
        sal_Int32 n = nStartPos;
        sal_Int32 nLen = rStr.getLength();
        while ( ( n < nLen ) && ( rStr[ n ] == LINE_SEP ) )
            n++;

        if ( n > nStartPos )
            rStr = rStr.copy( 0, nStartPos ) + rStr.copy( n );
    }
}

/*
    add new recorded dispatch macro script into the application global basic
    lib container. It generates a new unique id for it and insert the macro
    by using this number as name for the module
 */
void SfxViewFrame::AddDispatchMacroToBasic_Impl( const OUString& sMacro )
{
#if !HAVE_FEATURE_SCRIPTING
    (void) sMacro;
#else
    if ( sMacro.isEmpty() )
        return;

    SfxApplication* pSfxApp = SfxGetpApp();
    SfxRequest aReq( SID_BASICCHOOSER, SfxCallMode::SYNCHRON, pSfxApp->GetPool() );
    aReq.AppendItem( SfxBoolItem(SID_RECORDMACRO,true) );
    const SfxPoolItem* pRet = SfxGetpApp()->ExecuteSlot( aReq );
    OUString aScriptURL;
    if ( pRet )
        aScriptURL = static_cast<const SfxStringItem*>(pRet)->GetValue();
    if ( !aScriptURL.isEmpty() )
    {
        // parse scriptURL
        OUString aLibName;
        OUString aModuleName;
        OUString aMacroName;
        OUString aLocation;
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        Reference< css::uri::XUriReferenceFactory > xFactory =
            css::uri::UriReferenceFactory::create( xContext );
        Reference< css::uri::XVndSunStarScriptUrl > xUrl( xFactory->parse( aScriptURL ), UNO_QUERY );
        if ( xUrl.is() )
        {
            // get name
            const OUString aName = xUrl->getName();
            const sal_Unicode cTok = '.';
            sal_Int32 nIndex = 0;
            aLibName = aName.getToken( 0, cTok, nIndex );
            if ( nIndex != -1 )
                aModuleName = aName.getToken( 0, cTok, nIndex );
            if ( nIndex != -1 )
                aMacroName = aName.getToken( 0, cTok, nIndex );

            // get location
            aLocation = xUrl->getParameter( "location" );
        }

        BasicManager* pBasMgr = nullptr;
        if ( aLocation.equalsIgnoreAsciiCase( "application" ) )
        {
            // application basic
            pBasMgr = SfxApplication::GetBasicManager();
        }
        else if ( aLocation.equalsIgnoreAsciiCase( "document" ) )
        {
            pBasMgr = GetObjectShell()->GetBasicManager();
        }

        OUString aOUSource;
        if ( pBasMgr)
        {
            StarBASIC* pBasic = pBasMgr->GetLib( aLibName );
            if ( pBasic )
            {
                SbModule* pModule = pBasic->FindModule( aModuleName );
                SbMethod* pMethod = pModule ? pModule->FindMethod(aMacroName, SbxClassType::Method) : nullptr;
                if (pMethod)
                {
                    aOUSource = pModule->GetSource32();
                    sal_uInt16 nStart, nEnd;
                    pMethod->GetLineRange( nStart, nEnd );
                    sal_uIntPtr nlStart = nStart;
                    sal_uIntPtr nlEnd = nEnd;
                    CutLines( aOUSource, nlStart-1, nlEnd-nlStart+1 );
                }
            }
        }

        // open lib container and break operation if it couldn't be opened
        css::uno::Reference< css::script::XLibraryContainer > xLibCont;
        if ( aLocation.equalsIgnoreAsciiCase( "application" ) )
        {
            xLibCont = SfxGetpApp()->GetBasicContainer();
        }
        else if ( aLocation.equalsIgnoreAsciiCase( "document" ) )
        {
            xLibCont = GetObjectShell()->GetBasicContainer();
        }

        if(!xLibCont.is())
        {
            SAL_WARN("sfx.view", "couldn't get access to the basic lib container. Adding of macro isn't possible.");
            return;
        }

        // get LibraryContainer
        css::uno::Any aTemp;
        css::uno::Reference< css::container::XNameAccess > xRoot(
                xLibCont,
                css::uno::UNO_QUERY);

        css::uno::Reference< css::container::XNameAccess > xLib;
        if(xRoot->hasByName(aLibName))
        {
            // library must be loaded
            aTemp = xRoot->getByName(aLibName);
            xLibCont->loadLibrary(aLibName);
            aTemp >>= xLib;
        }
        else
        {
            xLib.set( xLibCont->createLibrary(aLibName), css::uno::UNO_QUERY);
        }

        // pack the macro as direct usable "sub" routine
        OUStringBuffer sRoutine(10000);
        bool bReplace = false;

        // get module
        if(xLib->hasByName(aModuleName))
        {
            if ( !aOUSource.isEmpty() )
            {
                sRoutine.append( aOUSource );
            }
            else
            {
                OUString sCode;
                aTemp = xLib->getByName(aModuleName);
                aTemp >>= sCode;
                sRoutine.append( sCode );
            }

            bReplace = true;
        }

        // append new method
        sRoutine.append( "\nsub " );
        sRoutine.append(aMacroName);
        sRoutine.append( "\n" );
        sRoutine.append(sMacro);
        sRoutine.append( "\nend sub\n" );

        // create the module inside the library and insert the macro routine
        aTemp <<= sRoutine.makeStringAndClear();
        if ( bReplace )
        {
            css::uno::Reference< css::container::XNameContainer > xModulCont(
                xLib,
                css::uno::UNO_QUERY);
            xModulCont->replaceByName(aModuleName,aTemp);
        }
        else
        {
            css::uno::Reference< css::container::XNameContainer > xModulCont(
                xLib,
                css::uno::UNO_QUERY);
            xModulCont->insertByName(aModuleName,aTemp);
        }

        // #i17355# update the Basic IDE
        for ( SfxViewShell* pViewShell = SfxViewShell::GetFirst(); pViewShell; pViewShell = SfxViewShell::GetNext( *pViewShell ) )
        {
            if ( pViewShell->GetName() == "BasicIDE" )
            {
                SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();
                SfxDispatcher* pDispat = pViewFrame ? pViewFrame->GetDispatcher() : nullptr;
                if ( pDispat )
                {
                    SfxMacroInfoItem aInfoItem( SID_BASICIDE_ARG_MACROINFO, pBasMgr, aLibName, aModuleName, OUString(), OUString() );
                    pDispat->ExecuteList(SID_BASICIDE_UPDATEMODULESOURCE,
                            SfxCallMode::SYNCHRON, { &aInfoItem });
                }
            }
        }
    }
    else
    {
        // add code for "session only" macro
    }
#endif
}

void SfxViewFrame::MiscExec_Impl( SfxRequest& rReq )
{
    switch ( rReq.GetSlot() )
    {
        case SID_STOP_RECORDING :
        case SID_RECORDMACRO :
        {
            // try to find any active recorder on this frame
            const OUString sProperty("DispatchRecorderSupplier");
            css::uno::Reference< css::frame::XFrame > xFrame(
                    GetFrame().GetFrameInterface(),
                    css::uno::UNO_QUERY);

            css::uno::Reference< css::beans::XPropertySet > xSet(xFrame,css::uno::UNO_QUERY);
            css::uno::Any aProp = xSet->getPropertyValue(sProperty);
            css::uno::Reference< css::frame::XDispatchRecorderSupplier > xSupplier;
            aProp >>= xSupplier;
            css::uno::Reference< css::frame::XDispatchRecorder > xRecorder;
            if (xSupplier.is())
                xRecorder = xSupplier->getDispatchRecorder();

            bool bIsRecording = xRecorder.is();
            const SfxBoolItem* pItem = rReq.GetArg<SfxBoolItem>(SID_RECORDMACRO);
            if ( pItem && pItem->GetValue() == bIsRecording )
                return;

            if ( xRecorder.is() )
            {
                // disable active recording
                aProp <<= css::uno::Reference< css::frame::XDispatchRecorderSupplier >();
                xSet->setPropertyValue(sProperty,aProp);

                const SfxBoolItem* pRecordItem = rReq.GetArg<SfxBoolItem>(FN_PARAM_1);
                if ( !pRecordItem || !pRecordItem->GetValue() )
                    // insert script into basic library container of application
                    AddDispatchMacroToBasic_Impl(xRecorder->getRecordedMacro());

                xRecorder->endRecording();
                xRecorder = nullptr;
                GetBindings().SetRecorder_Impl( xRecorder );

                SetChildWindow( SID_RECORDING_FLOATWINDOW, false );
                if ( rReq.GetSlot() != SID_RECORDMACRO )
                    GetBindings().Invalidate( SID_RECORDMACRO );
            }
            else if ( rReq.GetSlot() == SID_RECORDMACRO )
            {
                // enable recording
                css::uno::Reference< css::uno::XComponentContext > xContext(
                        ::comphelper::getProcessComponentContext());

                xRecorder = css::frame::DispatchRecorder::create( xContext );

                xSupplier = css::frame::DispatchRecorderSupplier::create( xContext );

                xSupplier->setDispatchRecorder(xRecorder);
                xRecorder->startRecording(xFrame);
                aProp <<= xSupplier;
                xSet->setPropertyValue(sProperty,aProp);
                GetBindings().SetRecorder_Impl( xRecorder );
                SetChildWindow( SID_RECORDING_FLOATWINDOW, true );
            }

            rReq.Done();
            break;
        }

        case SID_TOGGLESTATUSBAR:
        {
            css::uno::Reference< css::frame::XFrame > xFrame(
                    GetFrame().GetFrameInterface(),
                    css::uno::UNO_QUERY);

            Reference< css::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
            Reference< css::frame::XLayoutManager > xLayoutManager;
            if ( xPropSet.is() )
            {
                try
                {
                    Any aValue = xPropSet->getPropertyValue("LayoutManager");
                    aValue >>= xLayoutManager;
                }
                catch ( Exception& )
                {
                }
            }

            if ( xLayoutManager.is() )
            {
                const OUString aStatusbarResString( "private:resource/statusbar/statusbar" );
                // Evaluate parameter.
                const SfxBoolItem* pShowItem = rReq.GetArg<SfxBoolItem>(rReq.GetSlot());
                bool bShow( true );
                if ( !pShowItem )
                    bShow = xLayoutManager->isElementVisible( aStatusbarResString );
                else
                    bShow = pShowItem->GetValue();

                if ( bShow )
                {
                    xLayoutManager->createElement( aStatusbarResString );
                    xLayoutManager->showElement( aStatusbarResString );
                }
                else
                    xLayoutManager->hideElement( aStatusbarResString );

                if ( !pShowItem )
                    rReq.AppendItem( SfxBoolItem( SID_TOGGLESTATUSBAR, bShow ) );
            }
            rReq.Done();
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_WIN_FULLSCREEN:
        {
            const SfxBoolItem* pItem = rReq.GetArg<SfxBoolItem>(rReq.GetSlot());
            SfxViewFrame *pTop = GetTopViewFrame();
            if ( pTop )
            {
                WorkWindow* pWork = static_cast<WorkWindow*>( pTop->GetFrame().GetTopWindow_Impl() );
                if ( pWork )
                {
                    css::uno::Reference< css::frame::XFrame > xFrame(
                            GetFrame().GetFrameInterface(),
                            css::uno::UNO_QUERY);

                    Reference< css::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
                    Reference< css::frame::XLayoutManager > xLayoutManager;
                    if ( xPropSet.is() )
                    {
                        try
                        {
                            Any aValue = xPropSet->getPropertyValue("LayoutManager");
                            aValue >>= xLayoutManager;
                        }
                        catch ( Exception& )
                        {
                        }
                    }

                    bool bNewFullScreenMode = pItem ? pItem->GetValue() : !pWork->IsFullScreenMode();
                    if ( bNewFullScreenMode != pWork->IsFullScreenMode() )
                    {
                        Reference< css::beans::XPropertySet > xLMPropSet( xLayoutManager, UNO_QUERY );
                        if ( xLMPropSet.is() )
                        {
                            try
                            {
                                xLMPropSet->setPropertyValue(
                                    "HideCurrentUI",
                                    makeAny( bNewFullScreenMode ));
                            }
                            catch ( css::beans::UnknownPropertyException& )
                            {
                            }
                        }
                        pWork->ShowFullScreenMode( bNewFullScreenMode );
                        pWork->SetMenuBarMode( bNewFullScreenMode ? MenuBarMode::Hide : MenuBarMode::Normal );
                        GetFrame().GetWorkWindow_Impl()->SetFullScreen_Impl( bNewFullScreenMode );
                        if ( !pItem )
                            rReq.AppendItem( SfxBoolItem( SID_WIN_FULLSCREEN, bNewFullScreenMode ) );
                        rReq.Done();
                    }
                    else
                        rReq.Ignore();
                }
            }
            else
                rReq.Ignore();

            GetDispatcher()->Update_Impl( true );
            break;
        }
    }
}

void SfxViewFrame::MiscState_Impl(SfxItemSet &rSet)
{
    const sal_uInt16 *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges && *pRanges, "Set without range");
    while ( *pRanges )
    {
        for(sal_uInt16 nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich)
        {
            switch(nWhich)
            {
                case SID_CURRENT_URL:
                {
                    rSet.Put( SfxStringItem( nWhich, GetActualPresentationURL_Impl() ) );
                    break;
                }

                case SID_RECORDMACRO :
                {
                    SvtMiscOptions aMiscOptions;
                    const OUString& sName{GetObjectShell()->GetFactory().GetFactoryName()};
                    if ( !aMiscOptions.IsMacroRecorderMode() ||
                         ( sName!="swriter" && sName!="scalc" ) )
                    {
                        rSet.DisableItem( nWhich );
                        rSet.Put(SfxVisibilityItem(nWhich, false));
                        break;
                    }

                    css::uno::Reference< css::beans::XPropertySet > xSet(
                            GetFrame().GetFrameInterface(),
                            css::uno::UNO_QUERY);

                    css::uno::Any aProp = xSet->getPropertyValue("DispatchRecorderSupplier");
                    css::uno::Reference< css::frame::XDispatchRecorderSupplier > xSupplier;
                    if ( aProp >>= xSupplier )
                        rSet.Put( SfxBoolItem( nWhich, xSupplier.is() ) );
                    else
                        rSet.DisableItem( nWhich );
                    break;
                }

                case SID_STOP_RECORDING :
                {
                    SvtMiscOptions aMiscOptions;
                    const OUString& sName{GetObjectShell()->GetFactory().GetFactoryName()};
                    if ( !aMiscOptions.IsMacroRecorderMode() ||
                         ( sName!="swriter" && sName!="scalc" ) )
                    {
                        rSet.DisableItem( nWhich );
                        break;
                    }

                    css::uno::Reference< css::beans::XPropertySet > xSet(
                            GetFrame().GetFrameInterface(),
                            css::uno::UNO_QUERY);

                    css::uno::Any aProp = xSet->getPropertyValue("DispatchRecorderSupplier");
                    css::uno::Reference< css::frame::XDispatchRecorderSupplier > xSupplier;
                    if ( !(aProp >>= xSupplier) || !xSupplier.is() )
                        rSet.DisableItem( nWhich );
                    break;
                }

                case SID_TOGGLESTATUSBAR:
                {
                    css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
                    css::uno::Reference< css::beans::XPropertySet > xSet(
                            GetFrame().GetFrameInterface(),
                            css::uno::UNO_QUERY);
                    css::uno::Any aProp = xSet->getPropertyValue( "LayoutManager" );

                    if ( !( aProp >>= xLayoutManager ))
                        rSet.Put( SfxBoolItem( nWhich, false ));
                    else
                    {
                        bool bShow = xLayoutManager->isElementVisible( "private:resource/statusbar/statusbar" );
                        rSet.Put( SfxBoolItem( nWhich, bShow ));
                    }
                    break;
                }

                case SID_WIN_FULLSCREEN:
                {
                    SfxViewFrame* pTop = GetTopViewFrame();
                    if ( pTop )
                    {
                        WorkWindow* pWork = static_cast<WorkWindow*>( pTop->GetFrame().GetTopWindow_Impl() );
                        if ( pWork )
                        {
                            rSet.Put( SfxBoolItem( nWhich, pWork->IsFullScreenMode() ) );
                            break;
                        }
                    }

                    rSet.DisableItem( nWhich );
                    break;
                }

                default:
                    break;
            }
        }

        ++pRanges;
    }
}

/*  [Description]

    This method can be included in the Execute method for the on- and off-
    switching of ChildWindows, to implement this and API-bindings.

    Simply include as 'ExecuteMethod' in the IDL.
*/
void SfxViewFrame::ChildWindowExecute( SfxRequest &rReq )
{
    // Evaluate Parameter
    sal_uInt16 nSID = rReq.GetSlot();

    const SfxBoolItem* pShowItem = rReq.GetArg<SfxBoolItem>(nSID);
    if ( nSID == SID_VIEW_DATA_SOURCE_BROWSER )
    {
        if (!SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::EModule::DATABASE))
            return;
        Reference < XFrame > xFrame = GetFrame().GetFrameInterface();
        Reference < XFrame > xBeamer( xFrame->findFrame( "_beamer", FrameSearchFlag::CHILDREN ) );
        bool bHasChild = xBeamer.is();
        bool bShow = pShowItem ? pShowItem->GetValue() : !bHasChild;
        if ( pShowItem )
        {
            if( bShow == bHasChild )
                return;
        }
        else
            rReq.AppendItem( SfxBoolItem( nSID, bShow ) );

        if ( !bShow )
        {
            SetChildWindow( SID_BROWSER, false );
        }
        else
        {
            css::util::URL aTargetURL;
            aTargetURL.Complete = ".component:DB/DataSourceBrowser";
            Reference < css::util::XURLTransformer > xTrans(
                    css::util::URLTransformer::create(
                         ::comphelper::getProcessComponentContext() ) );
            xTrans->parseStrict( aTargetURL );

            Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
            Reference < css::frame::XDispatch > xDisp;
            if ( xProv.is() )
                xDisp = xProv->queryDispatch( aTargetURL, "_beamer", 31 );
            if ( xDisp.is() )
            {
                Sequence < css::beans::PropertyValue > aArgs(1);
                css::beans::PropertyValue* pArg = aArgs.getArray();
                pArg[0].Name = "Referer";
                pArg[0].Value <<= OUString("private:user");
                xDisp->dispatch( aTargetURL, aArgs );
            }
        }

        rReq.Done();
        return;
    }
    if (nSID == SID_STYLE_DESIGNER)
    {
        // First make sure that the sidebar is visible
        ShowChildWindow(SID_SIDEBAR);

        ::sfx2::sidebar::Sidebar::ShowPanel("StyleListPanel",
                                            GetFrame().GetFrameInterface(), true);
        rReq.Done();
        return;
    }

    bool bHasChild = HasChildWindow(nSID);
    bool bShow = pShowItem ? pShowItem->GetValue() : !bHasChild;
    GetDispatcher()->Update_Impl( true );

    // Perform action.
    if ( !pShowItem || bShow != bHasChild )
        ToggleChildWindow( nSID );

    GetBindings().Invalidate( nSID );

    // Record if possible.
    if ( nSID == SID_HYPERLINK_DIALOG || nSID == SID_SEARCH_DLG )
    {
        rReq.Ignore();
    }
    else
    {
        rReq.AppendItem( SfxBoolItem( nSID, bShow ) );
        rReq.Done();
    }
}

/*  [Description]

    This method can be used in the state method for the on and off-state
    of child-windows, in order to implement this.

    Just register the IDL as 'StateMethod'.
*/
void SfxViewFrame::ChildWindowState( SfxItemSet& rState )
{
    SfxWhichIter aIter( rState );
    for ( sal_uInt16 nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        if ( nSID == SID_VIEW_DATA_SOURCE_BROWSER )
        {
            rState.Put( SfxBoolItem( nSID, HasChildWindow( SID_BROWSER ) ) );
        }
        else if ( nSID == SID_HYPERLINK_DIALOG )
        {
            const SfxPoolItem* pDummy = nullptr;
            SfxItemState eState = GetDispatcher()->QueryState( SID_HYPERLINK_SETLINK, pDummy );
            if ( SfxItemState::DISABLED == eState )
                rState.DisableItem(nSID);
            else
            {
                if ( KnowsChildWindow(nSID) )
                    rState.Put( SfxBoolItem( nSID, HasChildWindow(nSID)) );
                else
                    rState.DisableItem(nSID);
            }
        }
        else if ( nSID == SID_BROWSER )
        {
            Reference < XFrame > xFrame = GetFrame().GetFrameInterface()->
                            findFrame( "_beamer", FrameSearchFlag::CHILDREN );
            if ( !xFrame.is() )
                rState.DisableItem( nSID );
            else if ( KnowsChildWindow(nSID) )
                rState.Put( SfxBoolItem( nSID, HasChildWindow(nSID) ) );
        }
        else if ( nSID == SID_SIDEBAR )
        {
            if  ( !KnowsChildWindow( nSID ) )
            {
                SAL_WARN("sfx.view", "SID_SIDEBAR state requested, but no task pane child window exists for this ID!");
                rState.DisableItem( nSID );
            }
            else
            {
                rState.Put( SfxBoolItem( nSID, HasChildWindow( nSID ) ) );
            }
        }
        else if ( KnowsChildWindow(nSID) )
            rState.Put( SfxBoolItem( nSID, HasChildWindow(nSID) ) );
        else
            rState.DisableItem(nSID);
    }
}

SfxWorkWindow* SfxViewFrame::GetWorkWindow_Impl()
{
    SfxWorkWindow* pWork = GetFrame().GetWorkWindow_Impl();
    return pWork;
}

void SfxViewFrame::SetChildWindow(sal_uInt16 nId, bool bOn, bool bSetFocus )
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl();
    if ( pWork )
        pWork->SetChildWindow_Impl( nId, bOn, bSetFocus );
}

void SfxViewFrame::ToggleChildWindow(sal_uInt16 nId)
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl();
    if ( pWork )
        pWork->ToggleChildWindow_Impl( nId, true );
}

bool SfxViewFrame::HasChildWindow( sal_uInt16 nId )
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl();
    return pWork && pWork->HasChildWindow_Impl(nId);
}

bool SfxViewFrame::KnowsChildWindow( sal_uInt16 nId )
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl();
    return pWork && pWork->KnowsChildWindow_Impl(nId);
}

void SfxViewFrame::ShowChildWindow( sal_uInt16 nId, bool bVisible )
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl();
    if ( pWork )
    {
        GetDispatcher()->Update_Impl(true);
        pWork->ShowChildWindow_Impl(nId, bVisible, true );
    }
}

SfxChildWindow* SfxViewFrame::GetChildWindow(sal_uInt16 nId)
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl();
    return pWork ? pWork->GetChildWindow_Impl(nId) : nullptr;
}

void SfxViewFrame::UpdateDocument_Impl()
{
    SfxObjectShell* pDoc = GetObjectShell();
    if ( pDoc->IsLoadingFinished() )
        pDoc->CheckSecurityOnLoading_Impl();

    // check if document depends on a template
    pDoc->UpdateFromTemplate_Impl();
}

void SfxViewFrame::SetViewFrame( SfxViewFrame* pFrame )
{
    SfxGetpApp()->SetViewFrame_Impl( pFrame );
}

VclPtr<SfxInfoBarWindow> SfxViewFrame::AppendInfoBar(const OUString& sId,
                                               const OUString& sMessage,
                                               InfoBarType aInfoBarType)
{
    SfxChildWindow* pChild = GetChildWindow(SfxInfoBarContainerChild::GetChildWindowId());
    if (!pChild)
        return nullptr;

    SfxInfoBarContainerWindow* pInfoBarContainer = static_cast<SfxInfoBarContainerWindow*>(pChild->GetWindow());
    auto pInfoBar = pInfoBarContainer->appendInfoBar(sId, sMessage, aInfoBarType, WB_LEFT | WB_VCENTER);
    ShowChildWindow(SfxInfoBarContainerChild::GetChildWindowId());
    return pInfoBar;
}

void SfxViewFrame::UpdateInfoBar( const OUString& sId,
                           const OUString& sMessage,
                           InfoBarType eType )
{
    const sal_uInt16 nId = SfxInfoBarContainerChild::GetChildWindowId();

    // Make sure the InfoBar container is visible
    if (!HasChildWindow(nId))
        ToggleChildWindow(nId);

    SfxChildWindow* pChild = GetChildWindow(nId);
    if (pChild)
    {
        SfxInfoBarContainerWindow* pInfoBarContainer = static_cast<SfxInfoBarContainerWindow*>(pChild->GetWindow());
        auto pInfoBar = pInfoBarContainer->getInfoBar(sId);

        if (pInfoBar)
             pInfoBar->Update(sMessage, eType);
    }
}

void SfxViewFrame::RemoveInfoBar( const OUString& sId )
{
    const sal_uInt16 nId = SfxInfoBarContainerChild::GetChildWindowId();

    // Make sure the InfoBar container is visible
    if (!HasChildWindow(nId))
        ToggleChildWindow(nId);

    SfxChildWindow* pChild = GetChildWindow(nId);
    if (pChild)
    {
        SfxInfoBarContainerWindow* pInfoBarContainer = static_cast<SfxInfoBarContainerWindow*>(pChild->GetWindow());
        auto pInfoBar = pInfoBarContainer->getInfoBar(sId);
        pInfoBarContainer->removeInfoBar(pInfoBar);
        ShowChildWindow(nId);
    }
}

bool SfxViewFrame::HasInfoBarWithID( const OUString& sId )
{
    const sal_uInt16 nId = SfxInfoBarContainerChild::GetChildWindowId();

    SfxChildWindow* pChild = GetChildWindow(nId);
    if (pChild)
    {
        SfxInfoBarContainerWindow* pInfoBarContainer = static_cast<SfxInfoBarContainerWindow*>(pChild->GetWindow());
        return pInfoBarContainer->hasInfoBarWithID(sId);
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
