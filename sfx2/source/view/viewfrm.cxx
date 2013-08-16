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

#include <stdio.h>

#include <sfx2/infobar.hxx>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchRecorder.hpp>
#include <com/sun/star/frame/DispatchRecorderSupplier.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/ui/UIElementFactoryManager.hpp>

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
#include <vcl/msgbox.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/ehdl.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrames.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel2.hpp>
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

#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
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
#include <comphelper/storagehelper.hxx>
#include <svtools/asynclink.hxx>
#include <svl/sharecontrolfile.hxx>
#include <svtools/svtools.hrc>
#include <svtools/svtresid.hxx>
#include <framework/framelistanalyzer.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/configuration.hxx>
#include <officecfg/Office/Common.hxx>

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
#include "appdata.hxx"
#include <sfx2/taskpane.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include "openflag.hxx"
#include "objshimp.hxx"
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include "arrdecl.hxx"
#include "sfxtypes.hxx"
#include <sfx2/request.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/ipclient.hxx>
#include "sfx2/sfxresid.hxx"
#include "appbas.hxx"
#include <sfx2/objitem.hxx>
#include "sfx2/viewfac.hxx"
#include <sfx2/event.hxx>
#include "fltfnc.hxx"
#include <sfx2/docfile.hxx>
#include <sfx2/module.hxx>
#include <sfx2/msgpool.hxx>
#include "viewimp.hxx"
#include <sfx2/sfxbasecontroller.hxx>
#include <sfx2/sfx.hrc>
#include "view.hrc"
#include <sfx2/frmdescr.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/progress.hxx>
#include "workwin.hxx"
#include "helper.hxx"
#include "sfx2/minfitem.hxx"
#include "../appl/app.hrc"
#include "impviewframe.hxx"

//-------------------------------------------------------------------------
DBG_NAME(SfxViewFrame)

#define SfxViewFrame
#include "sfxslots.hxx"
#undef SfxViewFrame

//-------------------------------------------------------------------------

SFX_IMPL_INTERFACE(SfxViewFrame,SfxShell,SfxResId(0))
{
    SFX_CHILDWINDOW_REGISTRATION( SID_BROWSER );
    SFX_CHILDWINDOW_REGISTRATION( SID_RECORDING_FLOATWINDOW );
#if HAVE_FEATURE_DESKTOP_GUI_ELEMENTS
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_FULLSCREEN | SFX_VISIBILITY_FULLSCREEN, SfxResId(RID_FULLSCREENTOOLBOX) );
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_APPLICATION | SFX_VISIBILITY_STANDARD, SfxResId(RID_ENVTOOLBOX) );
#endif
}

TYPEINIT2(SfxViewFrame,SfxShell,SfxListener);
TYPEINIT1(SfxViewFrameItem, SfxPoolItem);

//-------------------------------------------------------------------------
namespace
{
    bool moduleHasToolPanels( SfxViewFrame_Impl& i_rViewFrameImpl )
    {
        if ( !i_rViewFrameImpl.aHasToolPanels )
        {
            i_rViewFrameImpl.aHasToolPanels.reset( ::sfx2::ModuleTaskPane::ModuleHasToolPanels(
                i_rViewFrameImpl.rFrame.GetFrameInterface() ) );
        }
        return *i_rViewFrameImpl.aHasToolPanels;
    }
}

//-------------------------------------------------------------------------
static sal_Bool AskPasswordToModify_Impl( const uno::Reference< task::XInteractionHandler >& xHandler, const OUString& aPath, const SfxFilter* pFilter, sal_uInt32 nPasswordHash, const uno::Sequence< beans::PropertyValue > aInfo )
{
    // TODO/LATER: In future the info should replace the direct hash completely
    sal_Bool bResult = ( !nPasswordHash && !aInfo.getLength() );

    OSL_ENSURE( pFilter && ( pFilter->GetFilterFlags() & SFX_FILTER_PASSWORDTOMODIFY ), "PasswordToModify feature is active for a filter that does not support it!" );

    if ( pFilter && xHandler.is() )
    {
        sal_Bool bCancel = sal_False;
        sal_Bool bFirstTime = sal_True;

        while ( !bResult && !bCancel )
        {
            sal_Bool bMSType = !pFilter->IsOwnFormat();

            ::rtl::Reference< ::comphelper::DocPasswordRequest > pPasswordRequest(
                 new ::comphelper::DocPasswordRequest(
                 bMSType ? ::comphelper::DocPasswordRequestType_MS : ::comphelper::DocPasswordRequestType_STANDARD,
                 bFirstTime ? ::com::sun::star::task::PasswordRequestMode_PASSWORD_ENTER : ::com::sun::star::task::PasswordRequestMode_PASSWORD_REENTER,
                 aPath,
                 sal_True ) );

            uno::Reference< com::sun::star::task::XInteractionRequest > rRequest( pPasswordRequest.get() );
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
                    bResult = ( SfxMedium::CreatePasswordToModifyHash( pPasswordRequest->getPasswordToModify(), OUString( "com.sun.star.text.TextDocument"  ).equals( pFilter->GetServiceName() ) ) == nPasswordHash );
                }
            }
            else
                bCancel = sal_True;

            bFirstTime = sal_False;
        }
    }

    return bResult;
}

//-------------------------------------------------------------------------
void SfxViewFrame::SetDowning_Impl()
{
    pImp->bIsDowning = sal_True;
}

//-------------------------------------------------------------------------
sal_Bool SfxViewFrame::IsDowning_Impl() const
{
    return pImp->bIsDowning;
}


//--------------------------------------------------------------------
class SfxViewNotificatedFrameList_Impl :
    public SfxListener, public SfxViewFrameArr_Impl
{
public:

    void InsertViewFrame( SfxViewFrame* pFrame )
    {
        StartListening( *pFrame );
        push_back( pFrame );
    }
    void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

//-------------------------------------------------------------------------
void SfxViewNotificatedFrameList_Impl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.IsA(TYPE(SfxSimpleHint)) )
    {
        switch( ( (SfxSimpleHint&) rHint ).GetId() )
        {
            case SFX_HINT_DYING:
                SfxViewFrame* pFrame = (SfxViewFrame*) &rBC;
                if( pFrame )
                {
                    iterator it = std::find( begin(), end(), pFrame );
                    if( it != end() )
                        erase( it );
                }
                break;
        }
    }
}

//-------------------------------------------------------------------------

long ReloadDecouple_Impl( void* pObj, void* pArg )
{
    ((SfxViewFrame*) pObj)->ExecReload_Impl( *(SfxRequest*)pArg );
    return 0;
}

void SfxViewFrame::ExecReload_Impl( SfxRequest& rReq )
{
    SfxFrame *pParent = GetFrame().GetParentFrame();
    if ( rReq.GetSlot() == SID_RELOAD )
    {
        // When CTRL-Reload, reload the active Frame
        SfxViewFrame* pActFrame = this;
        while ( pActFrame )
            pActFrame = pActFrame->GetActiveChildFrame_Impl();

        if ( pActFrame )
        {
            sal_uInt16 nModifier = rReq.GetModifier();
            if ( nModifier & KEY_MOD1 )
            {
                pActFrame->ExecReload_Impl( rReq );
                return;
            }
        }

        // If only a reload of the graphics for one or more child frames
        // should be made
        SfxFrame& rFrame = GetFrame();
        if ( pParent == &rFrame && rFrame.GetChildFrameCount() )
        {
            sal_Bool bReloadAvailable = sal_False;
            SfxFrameIterator aIter( rFrame, sal_False );
            SfxFrame *pChild = aIter.FirstFrame();
            while ( pChild )
            {
                SfxFrame *pNext = aIter.NextFrame( *pChild );
                SfxObjectShell *pShell = pChild->GetCurrentDocument();
                if( pShell && pShell->Get_Impl()->bReloadAvailable )
                {
                    bReloadAvailable = sal_True;
                    pChild->GetCurrentViewFrame()->ExecuteSlot( rReq );
                }
                pChild = pNext;
            }

            // The top level frame itself has no graphics!
            if ( bReloadAvailable )
                return;
        }
    }
    else
    {
        // When CTRL-Edit, edit the TopFrame.
        sal_uInt16 nModifier = rReq.GetModifier();

        if ( ( nModifier & KEY_MOD1 ) && pParent )
        {
            SfxViewFrame *pTop = GetTopViewFrame();
            pTop->ExecReload_Impl( rReq );
            return;
        }
    }

    SfxObjectShell* pSh = GetObjectShell();
    switch ( rReq.GetSlot() )
    {
        case SID_EDITDOC:
        {
            if ( GetFrame().HasComponent() )
                break;

            // Due to Double occupancy in toolboxes (with or without Ctrl),
            // it is also possible that the slot is enabled, but Ctrl-click
            // despite this is not!
            if( !pSh || !pSh->HasName() || !(pSh->Get_Impl()->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ))
                break;

            SfxMedium* pMed = pSh->GetMedium();

            SFX_ITEMSET_ARG( pSh->GetMedium()->GetItemSet(), pItem, SfxBoolItem, SID_VIEWONLY, sal_False );
            if ( pItem && pItem->GetValue() )
            {
                SfxApplication* pApp = SFX_APP();
                SfxAllItemSet aSet( pApp->GetPool() );
                aSet.Put( SfxStringItem( SID_FILE_NAME, pMed->GetURLObject().GetMainURL(INetURLObject::NO_DECODE) ) );
                aSet.Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
                aSet.Put( SfxStringItem( SID_TARGETNAME, OUString("_blank") ) );
                SFX_ITEMSET_ARG( pMed->GetItemSet(), pReferer, SfxStringItem, SID_REFERER, sal_False );
                if ( pReferer )
                    aSet.Put( *pReferer );
                SFX_ITEMSET_ARG( pSh->GetMedium()->GetItemSet(), pVersionItem, SfxInt16Item, SID_VERSION, sal_False );
                if ( pVersionItem )
                    aSet.Put( *pVersionItem );

                if( pMed->GetFilter() )
                {
                    aSet.Put( SfxStringItem( SID_FILTER_NAME, pMed->GetFilter()->GetFilterName() ) );
                    SFX_ITEMSET_ARG( pMed->GetItemSet(), pOptions, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False );
                    if ( pOptions )
                        aSet.Put( *pOptions );
                }

                GetDispatcher()->Execute( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, aSet );
                return;
            }

            sal_uInt16 nOpenMode;
            sal_Bool bNeedsReload = sal_False;
            if ( !pSh->IsReadOnly() )
            {
                // Save and reload Readonly
                if( pSh->IsModified() )
                {
                    if ( pSh->PrepareClose() )
                    {
                        // the storing could let the medium be changed
                        pMed = pSh->GetMedium();
                        bNeedsReload = sal_True;
                    }
                    else
                    {
                        rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), sal_False ) );
                        return;
                    }
                }
                nOpenMode = SFX_STREAM_READONLY;
                pSh->SetReadOnlyUI(true);
            }
            else
            {
                if ( pSh->IsReadOnlyMedium()
                  && ( pSh->GetModifyPasswordHash() || pSh->GetModifyPasswordInfo().getLength() )
                  && !pSh->IsModifyPasswordEntered() )
                {
                    OUString aDocumentName = INetURLObject( pMed->GetOrigURL() ).GetMainURL( INetURLObject::DECODE_WITH_CHARSET );
                    if( !AskPasswordToModify_Impl( pMed->GetInteractionHandler(), aDocumentName, pMed->GetOrigFilter(), pSh->GetModifyPasswordHash(), pSh->GetModifyPasswordInfo() ) )
                    {
                        // this is a read-only document, if it has "Password to modify"
                        // the user should enter password before he can edit the document
                        rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), sal_False ) );
                        return;
                    }

                    pSh->SetModifyPasswordEntered();
                }

                nOpenMode = pSh->IsOriginallyReadOnlyMedium() ? SFX_STREAM_READONLY : SFX_STREAM_READWRITE;

                // if only the view was in the readonly mode then there is no need to do the reload
                if ( !pSh->IsReadOnlyMedium() )
                {
                    // SetReadOnlyUI causes recomputation of window title, using
                    // open mode among other things, so call SetOpenMode before
                    // SetReadOnlyUI:
                    pMed->SetOpenMode( nOpenMode );
                    pSh->SetReadOnlyUI( sal_False );
                    return;
                }

                pSh->SetReadOnlyUI( sal_False );
            }

            if ( rReq.IsAPI() )
            {
                // Control through API if r/w or r/o
                SFX_REQUEST_ARG(rReq, pEditItem, SfxBoolItem, SID_EDITDOC, sal_False);
                if ( pEditItem )
                    nOpenMode = pEditItem->GetValue() ? SFX_STREAM_READWRITE : SFX_STREAM_READONLY;
            }

            // doing

            OUString aTemp;
            utl::LocalFileHelper::ConvertPhysicalNameToURL( pMed->GetPhysicalName(), aTemp );
            INetURLObject aPhysObj( aTemp );
            SFX_ITEMSET_ARG( pSh->GetMedium()->GetItemSet(),
                             pVersionItem, SfxInt16Item, SID_VERSION, sal_False );

            INetURLObject aMedObj( pMed->GetName() );

            // the logic below is following, if the document seems not to need to be reloaded and the physical name is different
            // to the logical one, then on file system it can be checked that the copy is still newer than the original and no document reload is required
            if ( ( !bNeedsReload && ( (aMedObj.GetProtocol() == INET_PROT_FILE &&
                    aMedObj.getFSysPath(INetURLObject::FSYS_DETECT) != aPhysObj.getFSysPath(INetURLObject::FSYS_DETECT) &&
                    !::utl::UCBContentHelper::IsYounger( aMedObj.GetMainURL( INetURLObject::NO_DECODE ), aPhysObj.GetMainURL( INetURLObject::NO_DECODE ) ))
                  || pMed->IsRemote() ) )
               || pVersionItem )
            {
                sal_Bool bOK = sal_False;
                if ( !pVersionItem )
                {
                    sal_Bool bHasStorage = pMed->HasStorage_Impl();
                    // switching edit mode could be possible without reload
                    if ( bHasStorage && pMed->GetStorage() == pSh->GetStorage() )
                    {
                        // TODO/LATER: faster creation of copy
                        if ( !pSh->ConnectTmpStorage_Impl( pMed->GetStorage(), pMed ) )
                            return;
                    }

                    pMed->CloseAndRelease();
                    pMed->GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, !( nOpenMode & STREAM_WRITE ) ) );
                    pMed->SetOpenMode( nOpenMode );

                    pMed->CompleteReOpen();
                    if ( nOpenMode & STREAM_WRITE )
                        pMed->LockOrigFileOnDemand( sal_False, sal_True );

                    // LockOrigFileOnDemand might set the readonly flag itself, it should be set back
                    pMed->GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, !( nOpenMode & STREAM_WRITE ) ) );

                    if ( !pMed->GetErrorCode() )
                        bOK = sal_True;
                }

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
                    rReq.Done( sal_False );

                    if ( nOpenMode == SFX_STREAM_READWRITE && !rReq.IsAPI() )
                    {
                        // ::com::sun::star::sdbcx::User offering to open it as a template
                        QueryBox aBox( &GetWindow(), SfxResId(MSG_QUERY_OPENASTEMPLATE) );
                        if ( RET_YES == aBox.Execute() )
                        {
                            SfxApplication* pApp = SFX_APP();
                            SfxAllItemSet aSet( pApp->GetPool() );
                            aSet.Put( SfxStringItem( SID_FILE_NAME, pMed->GetName() ) );
                            SFX_ITEMSET_ARG( pMed->GetItemSet(), pReferer, SfxStringItem, SID_REFERER, sal_False );
                            if ( pReferer )
                                aSet.Put( *pReferer );
                            aSet.Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
                            if ( pVersionItem )
                                aSet.Put( *pVersionItem );

                            if( pMed->GetFilter() )
                            {
                                aSet.Put( SfxStringItem( SID_FILTER_NAME, pMed->GetFilter()->GetFilterName() ) );
                                SFX_ITEMSET_ARG( pMed->GetItemSet(), pOptions,
                                                 SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False );
                                if ( pOptions )
                                    aSet.Put( *pOptions );
                            }

                            GetDispatcher()->Execute( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, aSet );
                            return;
                        }
                        else
                            nErr = 0;
                    }

                    ErrorHandler::HandleError( nErr );
                    rReq.SetReturnValue(
                        SfxBoolItem( rReq.GetSlot(), sal_False ) );
                    return;
                }
                else
                {
                    pSh->DoSaveCompleted( pMed );
                    pSh->Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );
                    rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), sal_True ) );
                    rReq.Done( sal_True );
                    return;
                }
            }

            rReq.AppendItem( SfxBoolItem( SID_FORCERELOAD, sal_True) );
            rReq.AppendItem( SfxBoolItem( SID_SILENT, sal_True ));
        }

        case SID_RELOAD:
        {
            // Due to Double occupancy in toolboxes (with or without Ctrl),
            // it is also possible that the slot is enabled, but Ctrl-click
            // despite this is not!
            if ( !pSh || !pSh->CanReload_Impl() )
                break;
            SfxApplication* pApp = SFX_APP();
            SFX_REQUEST_ARG(rReq, pForceReloadItem, SfxBoolItem,
                            SID_FORCERELOAD, sal_False);
            if(  pForceReloadItem && !pForceReloadItem->GetValue() &&
                !pSh->GetMedium()->IsExpired() )
                return;
            if( pImp->bReloading || pSh->IsInModalMode() )
                return;

            // AutoLoad is prohibited if possible
            SFX_REQUEST_ARG(rReq, pAutoLoadItem, SfxBoolItem, SID_AUTOLOAD, sal_False);
            if ( pAutoLoadItem && pAutoLoadItem->GetValue() &&
                 GetFrame().IsAutoLoadLocked_Impl() )
                return;

            SfxObjectShellLock xOldObj( pSh );
            pImp->bReloading = sal_True;
            SFX_REQUEST_ARG(rReq, pURLItem, SfxStringItem,
                            SID_FILE_NAME, sal_False);
            // Open as editable?
            sal_Bool bForEdit = !pSh->IsReadOnly();

            // If possible ask the User
            sal_Bool bDo = ( GetViewShell()->PrepareClose() != sal_False );
            SFX_REQUEST_ARG(rReq, pSilentItem, SfxBoolItem, SID_SILENT, sal_False);
            if ( bDo && GetFrame().DocIsModified_Impl() &&
                 !rReq.IsAPI() && ( !pSilentItem || !pSilentItem->GetValue() ) )
            {
                QueryBox aBox( &GetWindow(), SfxResId(MSG_QUERY_LASTVERSION) );
                bDo = ( RET_YES == aBox.Execute() );
            }

            if ( bDo )
            {
                SfxMedium *pMedium = xOldObj->GetMedium();

                // Remove Frameset before the FramesetView may disappear
                String aURL;
                if (pURLItem)
                    aURL = pURLItem->GetValue();
                else
                    aURL = pMedium->GetName();

                sal_Bool bHandsOff =
                    ( pMedium->GetURLObject().GetProtocol() == INET_PROT_FILE && !xOldObj->IsDocShared() );

                // Emty existing SfxMDIFrames for this Document
                // in native format or R/O, open it now for editing?
                SfxObjectShellLock xNewObj;

                // collect the views of the document
                // TODO: when UNO ViewFactories are available for SFX-based documents, the below code should
                // be UNOized, too
                typedef ::std::pair< Reference< XFrame >, sal_uInt16 >  ViewDescriptor;
                ::std::list< ViewDescriptor > aViewFrames;
                SfxViewFrame *pView = GetFirst( xOldObj );
                while ( pView )
                {
                    Reference< XFrame > xFrame( pView->GetFrame().GetFrameInterface() );
                    OSL_ENSURE( xFrame.is(), "SfxViewFrame::ExecReload_Impl: no XFrame?!" );
                    aViewFrames.push_back( ViewDescriptor( xFrame, pView->GetCurViewId() ) );

                    pView = GetNext( *pView, xOldObj );
                }

                DELETEZ( xOldObj->Get_Impl()->pReloadTimer );

                SfxItemSet* pNewSet = 0;
                const SfxFilter *pFilter = pMedium->GetFilter();
                if( pURLItem )
                {
                    pNewSet = new SfxAllItemSet( pApp->GetPool() );
                    pNewSet->Put( *pURLItem );

                    // Filter Detection
                    SfxMedium aMedium( pURLItem->GetValue(), SFX_STREAM_READWRITE );
                    SfxFilterMatcher().GuessFilter( aMedium, &pFilter );
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

                    if ( pSh->IsOriginallyReadOnlyMedium() )
                        // edit mode is switched or reload of readonly document
                        pNewSet->Put( SfxBoolItem( SID_DOC_READONLY, true ) );
                    else
                        // Reload of file opened for writing
                        pNewSet->ClearItem( SID_DOC_READONLY );
                }

                // If a salvaged file is present, do not enclose the OrigURL
                // again, since the Tempdate is invalid after reload.
                SFX_ITEMSET_ARG( pNewSet, pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
                if( pSalvageItem )
                {
                    aURL = pSalvageItem->GetValue();
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
                    pNewSet->Put( SfxStringItem( SID_REFERER, String() ) );

                xOldObj->CancelTransfers();


                if ( pSilentItem && pSilentItem->GetValue() )
                    pNewSet->Put( SfxBoolItem( SID_SILENT, sal_True ) );

                SFX_ITEMSET_ARG(pNewSet, pInteractionItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, sal_False);
                SFX_ITEMSET_ARG(pNewSet, pMacroExecItem  , SfxUInt16Item, SID_MACROEXECMODE     , sal_False);
                SFX_ITEMSET_ARG(pNewSet, pDocTemplateItem, SfxUInt16Item, SID_UPDATEDOCMODE     , sal_False);

                if (!pInteractionItem)
                {
                    Reference < task::XInteractionHandler2 > xHdl = task::InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), 0 );
                    if (xHdl.is())
                        pNewSet->Put( SfxUnoAnyItem(SID_INTERACTIONHANDLER,::com::sun::star::uno::makeAny(xHdl)) );
                }

                if (!pMacroExecItem)
                    pNewSet->Put( SfxUInt16Item(SID_MACROEXECMODE,::com::sun::star::document::MacroExecMode::USE_CONFIG) );
                if (!pDocTemplateItem)
                    pNewSet->Put( SfxUInt16Item(SID_UPDATEDOCMODE,::com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG) );

                xOldObj->SetModified( sal_False );
                // Do not chache the old Document! Is invalid when loading
                // another document.

                SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSavedOptions, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False);
                SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSavedReferer, SfxStringItem, SID_REFERER, sal_False);

                sal_Bool bHasStorage = pMedium->HasStorage_Impl();
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

                xNewObj = SfxObjectShell::CreateObject( pFilter->GetServiceName(), SFX_CREATE_MODE_STANDARD );

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
                    xNewObj = 0;
                }

                DELETEZ( pNewSet );

                if( !xNewObj.Is() )
                {
                    if( bHandsOff )
                    {
                        // back to old medium
                        pMedium->ReOpen();
                        pMedium->LockOrigFileOnDemand( sal_False, sal_True );

                        xOldObj->DoSaveCompleted( pMedium );
                    }

                    // r/o-Doc couldn't be switched to writing mode
                    if ( bForEdit && SID_EDITDOC == rReq.GetSlot() )
                    {
                        // ask user for opening as template
                        QueryBox aBox( &GetWindow(), SfxResId(MSG_QUERY_OPENASTEMPLATE) );
                        if ( RET_YES == aBox.Execute() )
                        {
                            SfxAllItemSet aSet( pApp->GetPool() );
                            aSet.Put( SfxStringItem( SID_FILE_NAME, pMedium->GetName() ) );
                            aSet.Put( SfxStringItem( SID_TARGETNAME, OUString("_blank") ) );
                            if ( pSavedOptions )
                                aSet.Put( *pSavedOptions );
                            if ( pSavedReferer )
                                aSet.Put( *pSavedReferer );
                            aSet.Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
                            if( pFilter )
                                aSet.Put( SfxStringItem( SID_FILTER_NAME, pFilter->GetFilterName() ) );
                            GetDispatcher()->Execute( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, aSet );
                        }
                    }
                }
                else
                {
                    if ( xNewObj->GetModifyPasswordHash() && xNewObj->GetModifyPasswordHash() != xOldObj->GetModifyPasswordHash() )
                    {
                        xNewObj->SetModifyPasswordEntered( sal_False );
                        xNewObj->SetReadOnly();
                    }
                    else if ( rReq.GetSlot() == SID_EDITDOC )
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
                        while ( !aViewFrames.empty() )
                        {
                            LoadViewIntoFrame_Impl( *xNewObj, aViewFrames.front().first, aLoadArgs, aViewFrames.front().second, false );
                            aViewFrames.pop_front();
                        }
                    }
                    catch( const Exception& )
                    {
                        // close the remaining frames
                        // Don't catch exceptions herein, if this fails, then we're left in an indetermined state, and
                        // crashing is better than trying to proceed
                        while ( !aViewFrames.empty() )
                        {
                            Reference< util::XCloseable > xClose( aViewFrames.front().first, UNO_QUERY_THROW );
                            xClose->close( sal_True );
                            aViewFrames.pop_front();
                        }
                    }

                    // Propagate document closure.
                    SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_CLOSEDOC, GlobalEventConfig::GetEventName( STR_EVENT_CLOSEDOC ), xOldObj ) );
                }

                // Record as done
                rReq.Done( sal_True );
                rReq.SetReturnValue(SfxBoolItem(rReq.GetSlot(), sal_True));
                return;
            }
            else
            {
                // Record as not done
                rReq.Done();
                rReq.SetReturnValue(SfxBoolItem(rReq.GetSlot(), sal_False));
                pImp->bReloading = sal_False;
                return;
            }
        }
    }
}

//-------------------------------------------------------------------------
void SfxViewFrame::StateReload_Impl( SfxItemSet& rSet )
{
    SfxObjectShell* pSh = GetObjectShell();
    if ( !pSh )
        // I'm just on reload and am yielding myself ...
        return;

    GetFrame().GetParentFrame();
    SfxWhichIter aIter( rSet );
    for ( sal_uInt16 nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich() )
    {
        if ( GetFrame().HasComponent() )
        {
            // If the component is not self-dispatched, then
            // it makes no sense!
            rSet.DisableItem( nWhich );
            continue;
        }

        switch ( nWhich )
        {
            case SID_EDITDOC:
            {
                if ( !pSh || !pSh->HasName() || !( pSh->Get_Impl()->nLoadedFlags &  SFX_LOADED_MAINDOCUMENT )
                  || pSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
                    rSet.DisableItem( SID_EDITDOC );
                else
                {
                    SFX_ITEMSET_ARG( pSh->GetMedium()->GetItemSet(), pItem, SfxBoolItem, SID_EDITDOC, sal_False );
                    if ( pItem && !pItem->GetValue() )
                        rSet.DisableItem( SID_EDITDOC );
                    else
                        rSet.Put( SfxBoolItem( nWhich, !pSh->IsReadOnly() ) );
                }
                break;
            }

            case SID_RELOAD:
            {
                SfxFrame* pFrame = &GetTopFrame();

                if ( !pSh || !pSh->CanReload_Impl() || pSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
                    rSet.DisableItem(nWhich);
                else
                {
                    // If any ChildFrame is reloadable, the slot is enabled,
                    // so you can perfom CTRL-Reload
                    sal_Bool bReloadAvailable = sal_False;
                    SfxFrameIterator aFrameIter( *pFrame, sal_True );
                    for( SfxFrame* pNextFrame = aFrameIter.FirstFrame();
                            pFrame;
                            pNextFrame = pNextFrame ?
                                aFrameIter.NextFrame( *pNextFrame ) : 0 )
                    {
                        SfxObjectShell *pShell = pFrame->GetCurrentDocument();
                        if( pShell && pShell->Get_Impl()->bReloadAvailable )
                        {
                            bReloadAvailable = sal_True;
                            break;
                        }
                        pFrame = pNextFrame;
                    }

                    rSet.Put( SfxBoolItem( nWhich, bReloadAvailable));
                }

                break;
            }
        }
    }
}


//--------------------------------------------------------------------
void SfxViewFrame::ExecHistory_Impl( SfxRequest &rReq )
{
    // Is there an Undo-Manager on the top Shell?
    SfxShell *pSh = GetDispatcher()->GetShell(0);
    ::svl::IUndoManager* pShUndoMgr = pSh->GetUndoManager();
    sal_Bool bOK = sal_False;
    if ( pShUndoMgr )
    {
        switch ( rReq.GetSlot() )
        {
            case SID_CLEARHISTORY:
                pShUndoMgr->Clear();
                bOK = sal_True;
                break;

            case SID_UNDO:
                pShUndoMgr->Undo();
                GetBindings().InvalidateAll(sal_False);
                bOK = sal_True;
                break;

            case SID_REDO:
                pShUndoMgr->Redo();
                GetBindings().InvalidateAll(sal_False);
                bOK = sal_True;
                break;

            case SID_REPEAT:
                if ( pSh->GetRepeatTarget() )
                    pShUndoMgr->Repeat( *pSh->GetRepeatTarget() );
                bOK = sal_True;
                break;
        }
    }
    else if ( GetViewShell() )
    {
        // The SW has its own undo in the View
        const SfxPoolItem *pRet = GetViewShell()->ExecuteSlot( rReq );
        if ( pRet )
            bOK = ((SfxBoolItem*)pRet)->GetValue();
    }

    rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bOK ) );
    rReq.Done();
}

//--------------------------------------------------------------------
void SfxViewFrame::StateHistory_Impl( SfxItemSet &rSet )
{
    // Search for Undo-Manager
    SfxShell *pSh = GetDispatcher()->GetShell(0);
    if ( !pSh )
        // I'm just on reload and am yielding myself ...
        return;

    ::svl::IUndoManager *pShUndoMgr = pSh->GetUndoManager();
    if ( !pShUndoMgr )
    {
        // The SW has its own undo in the View
        SfxWhichIter aIter( rSet );
        SfxViewShell *pViewSh = GetViewShell();
        if( !pViewSh ) return;
        for ( sal_uInt16 nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
            pViewSh->GetSlotState( nSID, 0, &rSet );
        return;
    }

    if ( pShUndoMgr->GetUndoActionCount() == 0 &&
         pShUndoMgr->GetRedoActionCount() == 0 &&
         pShUndoMgr->GetRepeatActionCount() == 0 )
        rSet.DisableItem( SID_CLEARHISTORY );

    if ( pShUndoMgr && pShUndoMgr->GetUndoActionCount() )
    {
        String aTmp(SvtResId(STR_UNDO).toString());
        aTmp+= pShUndoMgr->GetUndoActionComment(0);
        rSet.Put( SfxStringItem( SID_UNDO, aTmp ) );
    }
    else
        rSet.DisableItem( SID_UNDO );

    if ( pShUndoMgr && pShUndoMgr->GetRedoActionCount() )
    {
        String aTmp(SvtResId(STR_REDO).toString());
        aTmp += pShUndoMgr->GetRedoActionComment(0);
        rSet.Put( SfxStringItem( SID_REDO, aTmp ) );
    }
    else
        rSet.DisableItem( SID_REDO );
    SfxRepeatTarget *pTarget = pSh->GetRepeatTarget();
    if ( pShUndoMgr && pTarget && pShUndoMgr->GetRepeatActionCount() &&
         pShUndoMgr->CanRepeat(*pTarget) )
    {
        String aTmp(SvtResId(STR_REPEAT).toString());
        aTmp += pShUndoMgr->GetRepeatActionComment(*pTarget);
        rSet.Put( SfxStringItem( SID_REPEAT, aTmp ) );
    }
    else
        rSet.DisableItem( SID_REPEAT );
}

//--------------------------------------------------------------------
void SfxViewFrame::PopShellAndSubShells_Impl( SfxViewShell& i_rViewShell )
{
    i_rViewShell.PopSubShells_Impl();
    sal_uInt16 nLevel = pDispatcher->GetShellLevel( i_rViewShell );
    if ( nLevel != USHRT_MAX )
    {
        if ( nLevel )
        {
            // more sub shells on the stack, which were not affected by PopSubShells_Impl
            SfxShell *pSubShell = pDispatcher->GetShell( nLevel-1 );
            if ( pSubShell == i_rViewShell.GetSubShell() )
                // "real" sub shells will be deleted elsewhere
                pDispatcher->Pop( *pSubShell, SFX_SHELL_POP_UNTIL );
            else
                pDispatcher->Pop( *pSubShell, SFX_SHELL_POP_UNTIL | SFX_SHELL_POP_DELETE );
        }
        pDispatcher->Pop( i_rViewShell );
        pDispatcher->Flush();
    }

}

//--------------------------------------------------------------------
void SfxViewFrame::ReleaseObjectShell_Impl()

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
{
    DBG_CHKTHIS(SfxViewFrame, 0);
    DBG_ASSERT( xObjSh.Is(), "no SfxObjectShell to release!" );

    GetFrame().ReleasingComponent_Impl( sal_True );
    if ( GetWindow().HasChildPathFocus( sal_True ) )
    {
        DBG_ASSERT( !GetActiveChildFrame_Impl(), "Wrong active child frame!" );
        GetWindow().GrabFocus();
    }

    SfxViewShell *pDyingViewSh = GetViewShell();
    if ( pDyingViewSh )
    {
        PopShellAndSubShells_Impl( *pDyingViewSh );
        pDyingViewSh->DisconnectAllClients();
        SetViewShell_Impl(0);
        delete pDyingViewSh;
    }
#ifdef DBG_UTIL
    else
        OSL_FAIL("No Shell");
#endif

    if ( xObjSh.Is() )
    {
         pImp->aLastType = xObjSh->Type();
        pDispatcher->Pop( *xObjSh );
        SfxModule* pModule = xObjSh->GetModule();
        if( pModule )
            pDispatcher->RemoveShell_Impl( *pModule );
        pDispatcher->Flush();
        EndListening( *xObjSh );

        Notify( *xObjSh, SfxSimpleHint(SFX_HINT_TITLECHANGED) );
        Notify( *xObjSh, SfxSimpleHint(SFX_HINT_DOCCHANGED) );

        if ( 1 == xObjSh->GetOwnerLockCount() && pImp->bObjLocked && xObjSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
            xObjSh->DoClose();
        SfxObjectShellRef xDyingObjSh = xObjSh;
        xObjSh.Clear();
        if( ( GetFrameType() & SFXFRAME_HASTITLE ) && pImp->nDocViewNo )
            xDyingObjSh->GetNoSet_Impl().ReleaseIndex(pImp->nDocViewNo-1);
        if ( pImp->bObjLocked )
        {
            xDyingObjSh->OwnerLock( sal_False );
            pImp->bObjLocked = sal_False;
        }
    }

    GetDispatcher()->SetDisableFlags( 0 );
}

//--------------------------------------------------------------------
sal_Bool SfxViewFrame::Close()
{
    DBG_CHKTHIS(SfxViewFrame, 0);

    DBG_ASSERT( GetFrame().IsClosing_Impl() || !GetFrame().GetFrameInterface().is(), "ViewFrame closed too early!" );

    // If no saving have been made up until now, then embedded Objects should
    // not be saved automatically anymore.
    if ( GetViewShell() )
        GetViewShell()->DiscardClients_Impl();
    Broadcast( SfxSimpleHint( SFX_HINT_DYING ) );

    if (SfxViewFrame::Current() == this)
        SfxViewFrame::SetViewFrame( NULL );

    // Since the Dispatcher is emptied, it can not be used in any reasnable
    // manner, thus it is better to let the dispatcher be.
    GetDispatcher()->Lock(sal_True);
    delete this;

    return sal_True;
}

//--------------------------------------------------------------------

void SfxViewFrame::DoActivate( sal_Bool bUI, SfxViewFrame* pOldFrame )
{
    DBG_CHKTHIS(SfxViewFrame, 0);
    SFX_APP();

    pDispatcher->DoActivate_Impl( bUI, pOldFrame );

    // If this ViewFrame has got a parent and this is not a parent of the
    // old ViewFrames, it gets a ParentActivate.
    if ( bUI )
    {
        SfxViewFrame *pFrame = GetParentViewFrame();
        while ( pFrame )
        {
            if ( !pOldFrame || !pOldFrame->GetFrame().IsParent( &pFrame->GetFrame() ) )
                pFrame->pDispatcher->DoParentActivate_Impl();
            pFrame = pFrame->GetParentViewFrame();
        }
    }
}

//--------------------------------------------------------------------
void SfxViewFrame::DoDeactivate(sal_Bool bUI, SfxViewFrame* pNewFrame )
{
    DBG_CHKTHIS(SfxViewFrame, 0);
    SFX_APP();
    pDispatcher->DoDeactivate_Impl( bUI, pNewFrame );

    // If this ViewFrame has got a parent and this is not a parent of the
    // new ViewFrames, it gets a ParentDeactivate.
    if ( bUI )
    {
        SfxViewFrame *pFrame = GetParentViewFrame();
        while ( pFrame )
        {
            if ( !pNewFrame || !pNewFrame->GetFrame().IsParent( &pFrame->GetFrame() ) )
                pFrame->pDispatcher->DoParentDeactivate_Impl();
            pFrame = pFrame->GetParentViewFrame();
        }
    }
}

//------------------------------------------------------------------------
void SfxViewFrame::InvalidateBorderImpl( const SfxViewShell* pSh )
{
    if( pSh && !nAdjustPosPixelLock )
    {
        if ( GetViewShell() && GetWindow().IsVisible() )
        {
            if ( GetFrame().IsInPlace() )
            {
                return;
            }

            DoAdjustPosSizePixel( (SfxViewShell *) GetViewShell(), Point(),
                                            GetWindow().GetOutputSizePixel() );
        }
    }
}

//------------------------------------------------------------------------
sal_Bool SfxViewFrame::SetBorderPixelImpl
(
    const SfxViewShell* pVSh,
    const SvBorder&     rBorder
)

{
    pImp->aBorder = rBorder;

    if ( IsResizeInToOut_Impl() && !GetFrame().IsInPlace() )
    {
        Size aSize = pVSh->GetWindow()->GetOutputSizePixel();
        if ( aSize.Width() && aSize.Height() )
        {
            aSize.Width() += rBorder.Left() + rBorder.Right();
            aSize.Height() += rBorder.Top() + rBorder.Bottom();

            Size aOldSize = GetWindow().GetOutputSizePixel();
            GetWindow().SetOutputSizePixel( aSize );
            Window* pParent = &GetWindow();
            while ( pParent->GetParent() )
                pParent = pParent->GetParent();
            Size aOuterSize = pParent->GetOutputSizePixel();
            aOuterSize.Width() += ( aSize.Width() - aOldSize.Width() );
            aOuterSize.Height() += ( aSize.Height() - aOldSize.Height() );
            pParent->SetOutputSizePixel( aOuterSize );
        }
    }
    else
    {
        Point aPoint;
        Rectangle aEditArea( aPoint, GetWindow().GetOutputSizePixel() );
        aEditArea.Left() += rBorder.Left();
        aEditArea.Right() -= rBorder.Right();
        aEditArea.Top() += rBorder.Top();
        aEditArea.Bottom() -= rBorder.Bottom();
        pVSh->GetWindow()->SetPosSizePixel( aEditArea.TopLeft(), aEditArea.GetSize() );
    }

    return sal_True;
}

//------------------------------------------------------------------------
const SvBorder& SfxViewFrame::GetBorderPixelImpl
(
    const SfxViewShell* /*pSh*/
)   const

{
    return pImp->aBorder;
}

//--------------------------------------------------------------------
void SfxViewFrame::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    {DBG_CHKTHIS(SfxViewFrame, 0);}

    if( IsDowning_Impl())
        return;

    // we know only SimpleHints
    if ( rHint.IsA(TYPE(SfxSimpleHint)) )
    {
        switch( ( (SfxSimpleHint&) rHint ).GetId() )
        {
            case SFX_HINT_MODECHANGED:
            {
                UpdateTitle();

                if ( !xObjSh.Is() )
                    break;

                // Switch r/o?
                SfxBindings& rBind = GetBindings();
                rBind.Invalidate( SID_RELOAD );
                SfxDispatcher *pDispat = GetDispatcher();
                sal_Bool bWasReadOnly = pDispat->GetReadOnly_Impl();
                sal_Bool bIsReadOnly = xObjSh->IsReadOnly();
                if ( !bWasReadOnly != !bIsReadOnly )
                {
                    // Then also TITLE_CHANGED
                    UpdateTitle();
                    rBind.Invalidate( SID_FILE_NAME );
                    rBind.Invalidate( SID_DOCINFO_TITLE );
                    rBind.Invalidate( SID_EDITDOC );

                    pDispat->GetBindings()->InvalidateAll(sal_True);
                    pDispat->SetReadOnly_Impl( bIsReadOnly );

                    // Only force and Dispatcher-Update, if it is done next
                    // anyway, otherwise flickering or GPF is possibel since
                    // the Writer for example prefers in Resize preform some
                    // actions which has a SetReadOnlyUI in Dispatcher as a
                    // result!

                    if ( pDispat->IsUpdated_Impl() )
                        pDispat->Update_Impl(sal_True);
                }

                Enable( !xObjSh->IsInModalMode() );
                break;
            }

            case SFX_HINT_TITLECHANGED:
            {
                UpdateTitle();
                SfxBindings& rBind = GetBindings();
                rBind.Invalidate( SID_FILE_NAME );
                rBind.Invalidate( SID_DOCINFO_TITLE );
                rBind.Invalidate( SID_EDITDOC );
                rBind.Invalidate( SID_RELOAD );
                break;
            }

            case SFX_HINT_DEINITIALIZING:
                GetFrame().DoClose();
                break;
            case SFX_HINT_DYING:
                // when the Object is being deleted, destroy the view too
                if ( xObjSh.Is() )
                    ReleaseObjectShell_Impl();
                else
                    GetFrame().DoClose();
                break;

        }
    }
    else if ( rHint.IsA(TYPE(SfxEventHint)) )
    {
        // When the Document is loaded asynchronously, was the Dispatcher
        // set as ReadOnly, to what must be returned when the document itself
        // is not read only, and the loading is finished.
        switch ( ((SfxEventHint&)rHint).GetEventId() )
        {
            case SFX_EVENT_MODIFYCHANGED:
            {
                SfxBindings& rBind = GetBindings();
                rBind.Invalidate( SID_DOC_MODIFIED );
                rBind.Invalidate( SID_SAVEDOC );
                rBind.Invalidate( SID_RELOAD );
                rBind.Invalidate( SID_EDITDOC );
                break;
            }

            case SFX_EVENT_OPENDOC:
            case SFX_EVENT_CREATEDOC:
            {
                if ( !xObjSh.Is() )
                    break;

                SfxBindings& rBind = GetBindings();
                rBind.Invalidate( SID_RELOAD );
                rBind.Invalidate( SID_EDITDOC );
                if ( !xObjSh->IsReadOnly() )
                {
                    // In contrast to above (TITLE_CHANGED) does the UI not
                    // have to be updated because it was not obstructed

                    // #i21560# InvalidateAll() causes the assertion
                    // 'SfxBindings::Invalidate while in update" when
                    // the sfx slot SID_BASICIDE_APPEAR is executed
                    // via API from another thread (Java).
                    // According to MBA this call is not necessary anymore,
                    // because each document has its own SfxBindings.
                    //GetDispatcher()->GetBindings()->InvalidateAll(sal_True);
                }

                break;
            }

            case SFX_EVENT_TOGGLEFULLSCREENMODE:
            {
                if ( GetFrame().OwnsBindings_Impl() )
                    GetBindings().GetDispatcher_Impl()->Update_Impl( sal_True );
                break;
            }
        }
    }
}

//------------------------------------------------------------------------
void SfxViewFrame::Construct_Impl( SfxObjectShell *pObjSh )
{
    pImp->bResizeInToOut = sal_True;
    pImp->bDontOverwriteResizeInToOut = sal_False;
    pImp->bObjLocked = sal_False;
    pImp->pFocusWin = 0;
    pImp->pActiveChild = NULL;
    pImp->nCurViewId = 0;
    pImp->bReloading = sal_False;
    pImp->bIsDowning = sal_False;
    pImp->bModal = sal_False;
    pImp->bEnabled = sal_True;
    pImp->nDocViewNo = 0;
    pImp->aMargin = Size( -1, -1 );
    pImp->pWindow = 0;

    SetPool( &SFX_APP()->GetPool() );
    pDispatcher = new SfxDispatcher(this);
    if ( !GetBindings().GetDispatcher() )
        GetBindings().SetDispatcher( pDispatcher );

    xObjSh = pObjSh;
    if ( xObjSh.Is() && xObjSh->IsPreview() )
        SetQuietMode_Impl( sal_True );

    if ( pObjSh )
    {
        pDispatcher->Push( *SFX_APP() );
        SfxModule* pModule = xObjSh->GetModule();
        if( pModule )
            pDispatcher->Push( *pModule );
        pDispatcher->Push( *this );
        pDispatcher->Push( *pObjSh );
        pDispatcher->Flush();
        StartListening( *pObjSh );
        pObjSh->ViewAssigned();
        Notify( *pObjSh, SfxSimpleHint(SFX_HINT_TITLECHANGED) );
        Notify( *pObjSh, SfxSimpleHint(SFX_HINT_DOCCHANGED) );
        pDispatcher->SetReadOnly_Impl( pObjSh->IsReadOnly() );
    }
    else
    {
        pDispatcher->Push( *SFX_APP() );
        pDispatcher->Push( *this );
        pDispatcher->Flush();
    }

    SfxViewFrameArr_Impl &rViewArr = SFX_APP()->GetViewFrames_Impl();
    rViewArr.push_back( this );
}

SfxViewFrame::SfxViewFrame
(
    SfxFrame&           rFrame,
    SfxObjectShell*     pObjShell
)

/*  [Description]

    Constructor of SfxViewFrame for a <SfxObjectShell> from the Resource.
    The 'nViewId' to the created <SfxViewShell> can be returned.
    (default is the SfxViewShell-Subclass that was registered first).
*/

    : pImp( new SfxViewFrame_Impl( rFrame ) )
    , pDispatcher(0)
    , pBindings( new SfxBindings )
    , nAdjustPosPixelLock( 0 )
{
    DBG_CTOR( SfxViewFrame, NULL );

    rFrame.SetCurrentViewFrame_Impl( this );
    rFrame.SetFrameType_Impl( GetFrameType() | SFXFRAME_HASTITLE );
    Construct_Impl( pObjShell );

    pImp->pWindow = new SfxFrameViewWindow_Impl( this, rFrame.GetWindow() );
    pImp->pWindow->SetSizePixel( rFrame.GetWindow().GetOutputSizePixel() );
    rFrame.SetOwnsBindings_Impl( sal_True );
    rFrame.CreateWorkWindow_Impl();
}

//------------------------------------------------------------------------
SfxViewFrame::~SfxViewFrame()
{
    DBG_DTOR(SfxViewFrame, 0);

    SetDowning_Impl();

    if ( SfxViewFrame::Current() == this )
        SfxViewFrame::SetViewFrame( NULL );

    ReleaseObjectShell_Impl();

    if ( GetFrame().OwnsBindings_Impl() )
        // The Bindings delete the Frame!
        KillDispatcher_Impl();

    delete pImp->pWindow;

    if ( GetFrame().GetCurrentViewFrame() == this )
        GetFrame().SetCurrentViewFrame_Impl( NULL );

    // Unregister from the Frame List.
    SfxApplication *pSfxApp = SFX_APP();
    SfxViewFrameArr_Impl &rFrames = pSfxApp->GetViewFrames_Impl();
    SfxViewFrameArr_Impl::iterator it = std::find( rFrames.begin(), rFrames.end(), this );
    rFrames.erase( it );

    // Delete Member
    KillDispatcher_Impl();

    delete pImp;
}

//------------------------------------------------------------------------
void SfxViewFrame::KillDispatcher_Impl()

// Remove and delete the Dispatcher.

{
    DBG_CHKTHIS(SfxViewFrame, 0);

    SfxModule* pModule = xObjSh.Is() ? xObjSh->GetModule() : 0;
    if ( xObjSh.Is() )
        ReleaseObjectShell_Impl();
    if ( pDispatcher )
    {
        if( pModule )
            pDispatcher->Pop( *pModule, SFX_SHELL_POP_UNTIL );
        else
            pDispatcher->Pop( *this );
        DELETEZ(pDispatcher);
    }
}

//------------------------------------------------------------------------
SfxViewFrame* SfxViewFrame::Current()
{
    return SfxApplication::Get() ? SFX_APP()->Get_Impl()->pViewFrame : NULL;
}

//--------------------------------------------------------------------
// returns the first window of spec. type viewing the specified doc.
SfxViewFrame* SfxViewFrame::GetFirst
(
    const SfxObjectShell*   pDoc,
    sal_Bool                    bOnlyIfVisible
)
{
    SfxApplication *pSfxApp = SFX_APP();
    SfxViewFrameArr_Impl &rFrames = pSfxApp->GetViewFrames_Impl();

    // search for a SfxDocument of the specified type
    for ( sal_uInt16 nPos = 0; nPos < rFrames.size(); ++nPos )
    {
        SfxViewFrame *pFrame = rFrames[nPos];
        if  (   ( !pDoc || pDoc == pFrame->GetObjectShell() )
            &&  ( !bOnlyIfVisible || pFrame->IsVisible() )
            )
            return pFrame;
    }

    return 0;
}
//--------------------------------------------------------------------

// returns thenext window of spec. type viewing the specified doc.
SfxViewFrame* SfxViewFrame::GetNext
(
    const SfxViewFrame&     rPrev,
    const SfxObjectShell*   pDoc,
    sal_Bool                    bOnlyIfVisible
)
{
    SfxApplication *pSfxApp = SFX_APP();
    SfxViewFrameArr_Impl &rFrames = pSfxApp->GetViewFrames_Impl();

    // refind the specified predecessor
    sal_uInt16 nPos;
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
    return 0;
}

//--------------------------------------------------------------------
SfxProgress* SfxViewFrame::GetProgress() const
{
    SfxObjectShell *pObjSh = GetObjectShell();
    return pObjSh ? pObjSh->GetProgress() : 0;
}

//--------------------------------------------------------------------
void SfxViewFrame::DoAdjustPosSizePixel //! divide on Inner.../Outer...
(
    SfxViewShell*   pSh,
    const Point&    rPos,
    const Size&     rSize
)
{
    DBG_CHKTHIS(SfxViewFrame, 0);

    // Components do not use this Method!
    if( pSh && pSh->GetWindow() && !nAdjustPosPixelLock )
    {
        nAdjustPosPixelLock++;
        if ( pImp->bResizeInToOut )
            pSh->InnerResizePixel( rPos, rSize );
        else
            pSh->OuterResizePixel( rPos, rSize );
        nAdjustPosPixelLock--;
    }
}

//========================================================================

int SfxViewFrameItem::operator==( const SfxPoolItem &rItem ) const
{
     return PTR_CAST(SfxViewFrameItem, &rItem)->pFrame== pFrame;
}

//--------------------------------------------------------------------
OUString SfxViewFrameItem::GetValueText() const
{
    return OUString();
}

//--------------------------------------------------------------------
SfxPoolItem* SfxViewFrameItem::Clone( SfxItemPool *) const
{
    return new SfxViewFrameItem( pFrame);
}

//--------------------------------------------------------------------
void SfxViewFrame::SetViewShell_Impl( SfxViewShell *pVSh )

/*  [Description]

    Internal Method to set the current <SfxViewShell> Instance,
    that is active int this SfxViewFrame at the moment.
*/

{
    SfxShell::SetViewShell_Impl( pVSh );

    // Hack: InPlaceMode
    if ( pVSh )
        pImp->bResizeInToOut = sal_False;
}

//--------------------------------------------------------------------
/*  [Description]

    The ParentViewFrame of the Containers ViewFrame in the internal InPlace
*/

//TODO/LATER: is it still necessary? is there a replacement for GetParentViewFrame_Impl?
SfxViewFrame* SfxViewFrame::GetParentViewFrame_Impl() const
{
    return NULL;
}

//--------------------------------------------------------------------
void SfxViewFrame::ForceOuterResize_Impl(sal_Bool bOn)
{
    if ( !pImp->bDontOverwriteResizeInToOut )
        pImp->bResizeInToOut = !bOn;
}

//--------------------------------------------------------------------
sal_Bool SfxViewFrame::IsResizeInToOut_Impl() const
{
    return pImp->bResizeInToOut;
}

//--------------------------------------------------------------------
void SfxViewFrame::GetDocNumber_Impl()
{
    DBG_ASSERT( GetObjectShell(), "No Document!" );
    GetObjectShell()->SetNamedVisibility_Impl();
    pImp->nDocViewNo = GetObjectShell()->GetNoSet_Impl().GetFreeIndex()+1;
}

//--------------------------------------------------------------------

void SfxViewFrame::Enable( sal_Bool bEnable )
{
    if ( bEnable != pImp->bEnabled )
    {
        pImp->bEnabled = bEnable;

        // e.g. InPlace-Frames have a parent...
        SfxViewFrame *pParent = GetParentViewFrame_Impl();
        if ( pParent )
        {
            pParent->Enable( bEnable );
        }
        else
        {
            Window *pWindow = &GetFrame().GetTopFrame().GetWindow();
            if ( !bEnable )
                pImp->bWindowWasEnabled = pWindow->IsInputEnabled();
            if ( !bEnable || pImp->bWindowWasEnabled )
                pWindow->EnableInput( bEnable, sal_True );
        }

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
                pViewSh->ShowCursor(sal_False);
        }
    }
}

//--------------------------------------------------------------------
void SfxViewFrame::Show()

/*  [Description]

    This method makes the Frame-Window visible and before transmitts the
    window name. In addition, the document is held. In general one can never
    show the window directly!
*/

{
    // First lock the objectShell so that UpdateTitle() is valid:
    // IsVisible() == sal_True (:#)
    if ( xObjSh.Is() )
    {
        xObjSh->GetMedium()->GetItemSet()->ClearItem( SID_HIDDEN );
        if ( !pImp->bObjLocked )
            LockObjectShell_Impl( sal_True );

        // Adjust Doc-Shell titel nummer, get unique view-no
        if ( 0 == pImp->nDocViewNo  )
        {
            GetDocNumber_Impl();
            UpdateTitle();
        }
    }
    else
        UpdateTitle();

    // Display Frame-window, but only if the ViewFrame has no window of its
    // own or if it does not contain a Component
    if ( &GetWindow() == &GetFrame().GetWindow() || !GetFrame().HasComponent() )
        GetWindow().Show();
    GetFrame().GetWindow().Show();
}

//--------------------------------------------------------------------
sal_Bool SfxViewFrame::IsVisible() const
{
    return pImp->bObjLocked;
}

//--------------------------------------------------------------------
void SfxViewFrame::LockObjectShell_Impl( sal_Bool bLock )
{
    DBG_ASSERT( pImp->bObjLocked != bLock, "Wrong Locked status!" );

    DBG_ASSERT( GetObjectShell(), "No Document!" );
    GetObjectShell()->OwnerLock(bLock);
    pImp->bObjLocked = bLock;
}

//--------------------------------------------------------------------
void SfxViewFrame::MakeActive_Impl( sal_Bool bGrabFocus )
{
    if ( GetViewShell() && !GetFrame().IsClosing_Impl() )
    {
        if ( IsVisible() )
        {
            if ( GetViewShell() )
            {
                sal_Bool bPreview = sal_False;
                if ( GetObjectShell()->IsPreview() )
                {
                    bPreview = sal_True;
                }
                else
                {
                    SfxViewFrame* pParent = GetParentViewFrame();
                    if ( pParent )
                        pParent->SetActiveChildFrame_Impl( this );
                }

                SfxViewFrame* pCurrent = SfxViewFrame::Current();
                css::uno::Reference< css::frame::XFrame > xFrame = GetFrame().GetFrameInterface();
                if ( !bPreview )
                {
                    SetViewFrame( this );
                    GetBindings().SetActiveFrame( css::uno::Reference< css::frame::XFrame >() );
                    uno::Reference< frame::XFramesSupplier > xSupp( xFrame, uno::UNO_QUERY );
                    if ( xSupp.is() )
                        xSupp->setActiveFrame( uno::Reference < frame::XFrame >() );

                    css::uno::Reference< css::awt::XWindow > xContainerWindow = xFrame->getContainerWindow();
                    Window* pWindow = VCLUnoHelper::GetWindow(xContainerWindow);
                    if (pWindow && pWindow->HasChildPathFocus() && bGrabFocus)
                    {
                        SfxInPlaceClient *pCli = GetViewShell()->GetUIActiveClient();
                        if ( ( !pCli || !pCli->IsObjectUIActive() ) &&
                            ( !pCurrent || pCurrent->GetParentViewFrame_Impl() != this ) )
                                GetFrame().GrabFocusOnComponent_Impl();
                    }
                }
                else
                {
                    GetBindings().SetDispatcher( GetDispatcher() );
                    GetBindings().SetActiveFrame( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > () );
                    GetDispatcher()->Update_Impl( sal_False );
                }
            }
        }
    }
}

//-------------------------------------------------------------------------

void SfxViewFrame::SetQuietMode_Impl( sal_Bool bOn )
{
    GetDispatcher()->SetQuietMode_Impl( bOn );
}

//-------------------------------------------------------------------------

SfxObjectShell* SfxViewFrame::GetObjectShell()
{
    return xObjSh;
}

const Size& SfxViewFrame::GetMargin_Impl() const
{
    return pImp->aMargin;
}

void SfxViewFrame::SetActiveChildFrame_Impl( SfxViewFrame *pViewFrame )
{
    if ( pViewFrame != pImp->pActiveChild )
    {
        pImp->pActiveChild = pViewFrame;

        Reference< XFramesSupplier > xFrame( GetFrame().GetFrameInterface(), UNO_QUERY );
        Reference< XFrame >  xActive;
        if ( pViewFrame )
            xActive = pViewFrame->GetFrame().GetFrameInterface();

        if ( xFrame.is() )      // xFrame can be NULL
            xFrame->setActiveFrame( xActive );
    }
}

SfxViewFrame* SfxViewFrame::GetActiveChildFrame_Impl() const
{
    SfxViewFrame *pViewFrame = pImp->pActiveChild;
    return pViewFrame;
}

//--------------------------------------------------------------------
SfxViewFrame* SfxViewFrame::LoadViewIntoFrame_Impl_NoThrow( const SfxObjectShell& i_rDoc, const Reference< XFrame >& i_rFrame,
                                                   const sal_uInt16 i_nViewId, const bool i_bHidden )
{
    Reference< XFrame > xFrame( i_rFrame );
    bool bOwnFrame = false;
    SfxViewShell* pSuccessView = NULL;
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
                    ::framework::FrameListAnalyzer aAnalyzer( xDesktop, Reference< XFrame >(), ::framework::FrameListAnalyzer::E_BACKINGCOMPONENT );

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
            xContainerWindow->setVisible( sal_True );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
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
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    return NULL;
}

//--------------------------------------------------------------------
SfxViewShell* SfxViewFrame::LoadViewIntoFrame_Impl( const SfxObjectShell& i_rDoc, const Reference< XFrame >& i_rFrame,
                                           const Sequence< PropertyValue >& i_rLoadArgs, const sal_uInt16 i_nViewId,
                                           const bool i_bHidden )
{
    Reference< XModel > xDocument( i_rDoc.GetModel(), UNO_SET_THROW );

    ::comphelper::NamedValueCollection aTransformLoadArgs( i_rLoadArgs.getLength() ? i_rLoadArgs : xDocument->getArgs() );
    aTransformLoadArgs.put( "Model", xDocument );
    if ( i_nViewId )
        aTransformLoadArgs.put( "ViewId", sal_Int16( i_nViewId ) );
    if ( i_bHidden )
        aTransformLoadArgs.put( "Hidden", i_bHidden );
    else
        aTransformLoadArgs.remove( "Hidden" );

    OUString sURL( "private:object"  );
    if ( sURL.isEmpty() )
        sURL = i_rDoc.GetFactory().GetFactoryURL();

    Reference< XComponentLoader > xLoader( i_rFrame, UNO_QUERY_THROW );
    xLoader->loadComponentFromURL( sURL, OUString("_self"), 0,
        aTransformLoadArgs.getPropertyValues() );

    SfxViewShell* pViewShell = SfxViewShell::Get( i_rFrame->getController() );
    ENSURE_OR_THROW( pViewShell,
        "SfxViewFrame::LoadViewIntoFrame_Impl: loading an SFX doc into a frame resulted in a non-SFX view - quite impossible" );
    return pViewShell;
}

//--------------------------------------------------------------------

SfxViewFrame* SfxViewFrame::LoadHiddenDocument( SfxObjectShell& i_rDoc, const sal_uInt16 i_nViewId )
{
    return LoadViewIntoFrame_Impl_NoThrow( i_rDoc, Reference< XFrame >(), i_nViewId, true );
}

//--------------------------------------------------------------------

SfxViewFrame* SfxViewFrame::LoadDocument( SfxObjectShell& i_rDoc, const sal_uInt16 i_nViewId )
{
    return LoadViewIntoFrame_Impl_NoThrow( i_rDoc, Reference< XFrame >(), i_nViewId, false );
}

//--------------------------------------------------------------------

SfxViewFrame* SfxViewFrame::LoadDocumentIntoFrame( SfxObjectShell& i_rDoc, const Reference< XFrame >& i_rTargetFrame, const sal_uInt16 i_nViewId )
{
    return LoadViewIntoFrame_Impl_NoThrow( i_rDoc, i_rTargetFrame, i_nViewId, false );
}

//--------------------------------------------------------------------

SfxViewFrame* SfxViewFrame::LoadDocumentIntoFrame( SfxObjectShell& i_rDoc, const SfxFrameItem* i_pFrameItem, const sal_uInt16 i_nViewId )
{
    return LoadViewIntoFrame_Impl_NoThrow( i_rDoc, i_pFrameItem && i_pFrameItem->GetFrame() ? i_pFrameItem->GetFrame()->GetFrameInterface() : NULL, i_nViewId, false );
}

//--------------------------------------------------------------------
SfxViewFrame* SfxViewFrame::DisplayNewDocument( SfxObjectShell& i_rDoc, const SfxRequest& i_rCreateDocRequest, const sal_uInt16 i_nViewId )
{
    SFX_REQUEST_ARG( i_rCreateDocRequest, pFrameItem, SfxUnoFrameItem, SID_FILLFRAME, sal_False );
    SFX_REQUEST_ARG( i_rCreateDocRequest, pHiddenItem, SfxBoolItem, SID_HIDDEN, sal_False );

    return LoadViewIntoFrame_Impl_NoThrow(
        i_rDoc,
        pFrameItem ? pFrameItem->GetFrame() : NULL,
        i_nViewId,
        pHiddenItem ? pHiddenItem->GetValue() : false
    );
}

//--------------------------------------------------------------------

SfxViewFrame* SfxViewFrame::Get( const Reference< XController>& i_rController, const SfxObjectShell* i_pDoc )
{
    if ( !i_rController.is() )
        return NULL;

    const SfxObjectShell* pDoc = i_pDoc;
    if ( !pDoc )
    {
        Reference< XModel > xDocument( i_rController->getModel() );
        for (   pDoc = SfxObjectShell::GetFirst( 0, false );
                pDoc;
                pDoc = SfxObjectShell::GetNext( *pDoc, 0, false )
            )
        {
            if ( pDoc->GetModel() == xDocument )
                break;
        }
    }

    SfxViewFrame* pViewFrame = NULL;
    for (   pViewFrame = SfxViewFrame::GetFirst( pDoc, sal_False );
            pViewFrame;
            pViewFrame = SfxViewFrame::GetNext( *pViewFrame, pDoc, sal_False )
        )
    {
        if ( pViewFrame->GetViewShell()->GetController() == i_rController )
            break;
    }

    return pViewFrame;
}

//--------------------------------------------------------------------

void SfxViewFrame::SaveCurrentViewData_Impl( const sal_uInt16 i_nNewViewId )
{
    SfxViewShell* pCurrentShell = GetViewShell();
    ENSURE_OR_RETURN_VOID( pCurrentShell != NULL, "SfxViewFrame::SaveCurrentViewData_Impl: no current view shell -> no current view data!" );

    // determine the logical (API) view name
    const SfxObjectFactory& rDocFactory( pCurrentShell->GetObjectShell()->GetFactory() );
    const sal_uInt16 nCurViewNo = rDocFactory.GetViewNo_Impl( GetCurViewId(), 0 );
    const String sCurrentViewName = rDocFactory.GetViewFactory( nCurViewNo ).GetAPIViewName();
    const sal_uInt16 nNewViewNo = rDocFactory.GetViewNo_Impl( i_nNewViewId, 0 );
    const String sNewViewName = rDocFactory.GetViewFactory( nNewViewNo ).GetAPIViewName();
    if ( ( sCurrentViewName.Len() == 0 ) || ( sNewViewName.Len() == 0 ) )
    {
        // can't say anything about the view, the respective application did not yet migrate its code to
        // named view factories => bail out
        OSL_FAIL( "SfxViewFrame::SaveCurrentViewData_Impl: views without API names? Shouldn't happen anymore?" );
        return;
    }
    OSL_ENSURE( !sNewViewName.Equals( sCurrentViewName ), "SfxViewFrame::SaveCurrentViewData_Impl: suspicious: new and old view name are identical!" );

    // save the view data only when we're moving from a non-print-preview to the print-preview view
    if ( !sNewViewName.EqualsAscii( "PrintPreview" ) )
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
            OUString sViewId( aCurViewData.getOrDefault( "ViewId", OUString() ) );
            if ( sViewId.isEmpty() )
                continue;

            const SfxViewFactory* pViewFactory = rDocFactory.GetViewFactoryByViewName( sViewId );
            if ( pViewFactory == NULL )
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
        DBG_UNHANDLED_EXCEPTION();
    }
}

//--------------------------------------------------------------------

sal_Bool SfxViewFrame::SwitchToViewShell_Impl
(
    sal_uInt16  nViewIdOrNo,    /*  > 0
                                Registration-Id of the View, to which the
                                the method should switch, for example the one
                                that will be created.

                                == 0
                                First use the Default view. */

    sal_Bool    bIsIndex        /*  sal_True
                                'nViewIdOrNo' is no Registration-Id instead
                                an Index of <SfxViewFrame> in <SfxObjectShell>.
                                */
)

/*  [Description]

    Internal Method for switching to another <SfxViewShell> subclass,
    which should be created in this SfxMDIFrame. If no SfxViewShell exist
    in this SfxMDIFrame, then one will first be created.


    [Return Value]

    sal_Bool                        sal_True
                                requested SfxViewShell was created and a
                                possibly existing one deleted

                                sal_False
                                SfxViewShell requested could not be created,
                                the existing SfxViewShell thus continue to exist
*/

{
    try
    {
        ENSURE_OR_THROW( GetObjectShell() != NULL, "not possible without a document" );

        // if we already have a view shell, remove it
        SfxViewShell* pOldSh = GetViewShell();
        OSL_PRECOND( pOldSh, "SfxViewFrame::SwitchToViewShell_Impl: that's called *switch* (not for *initial-load*) for a reason" );
        if ( pOldSh )
        {
            // ask whether it can be closed
            if ( !pOldSh->PrepareClose( sal_True ) )
                return sal_False;

            // remove sub shells from Dispatcher before switching to new ViewShell
            PopShellAndSubShells_Impl( *pOldSh );
        }

        GetBindings().ENTERREGISTRATIONS();
        LockAdjustPosSizePixel();

        // ID of the new view
        SfxObjectFactory& rDocFact = GetObjectShell()->GetFactory();
        const sal_uInt16 nViewId = ( bIsIndex || !nViewIdOrNo ) ? rDocFact.GetViewFactory( nViewIdOrNo ).GetOrdinal() : nViewIdOrNo;

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
            DoAdjustPosSizePixel( pNewSh, Point(), GetWindow().GetOutputSizePixel() );

        GetBindings().LEAVEREGISTRATIONS();
        delete pOldSh;
    }
    catch ( const com::sun::star::uno::Exception& )
    {
        // the SfxCode is not able to cope with exceptions thrown while creating views
        // the code will crash in the stack unwinding procedure, so we shouldn't let exceptions go through here
        DBG_UNHANDLED_EXCEPTION();
        return sal_False;
    }

    DBG_ASSERT( SFX_APP()->GetViewFrames_Impl().size() == SFX_APP()->GetViewShells_Impl().size(), "Inconsistent view arrays!" );
    return sal_True;
}

//-------------------------------------------------------------------------
void SfxViewFrame::SetCurViewId_Impl( const sal_uInt16 i_nID )
{
    pImp->nCurViewId = i_nID;
}

//-------------------------------------------------------------------------
sal_uInt16 SfxViewFrame::GetCurViewId() const
{
    return pImp->nCurViewId;
}

//-------------------------------------------------------------------------
void SfxViewFrame::ExecView_Impl
(
    SfxRequest& rReq        // The executable <SfxRequest>
)

/*  [Description]

    Internal method to run the slot for the <SfxShell> Subclass in the
    SfxViewFrame <SVIDL> described slots.
*/

{
    DBG_CHKTHIS(SfxViewFrame, 0);

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
            const SfxPoolItem *pItem = 0;
            if  (   rReq.GetArgs()
                &&  SFX_ITEM_SET == rReq.GetArgs()->GetItemState( SID_VIEWSHELL, sal_False, &pItem )
                )
            {
                const sal_uInt16 nViewId = static_cast< const SfxUInt16Item* >( pItem )->GetValue();
                sal_Bool bSuccess = SwitchToViewShell_Impl( nViewId );
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
            sal_Bool bSuccess = SwitchToViewShell_Impl( nViewNo, sal_True );
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

            // Get ViewData of FrameSets recursivly.
            GetFrame().GetViewData_Impl();
            SfxMedium* pMed = GetObjectShell()->GetMedium();

            // do not open the new window hidden
            pMed->GetItemSet()->ClearItem( SID_HIDDEN );

            // the view ID (optional arg. TODO: this is currently not supported in the slot definition ...)
            SFX_REQUEST_ARG( rReq, pViewIdItem, SfxUInt16Item, SID_VIEW_ID, sal_False );
            const sal_uInt16 nViewId = pViewIdItem ? pViewIdItem->GetValue() : GetCurViewId();

            Reference < XFrame > xFrame;
            // the frame (optional arg. TODO: this is currently not supported in the slot definition ...)
            SFX_REQUEST_ARG( rReq, pFrameItem, SfxUnoFrameItem, SID_FILLFRAME, sal_False );
            if ( pFrameItem )
                xFrame = pFrameItem->GetFrame();

            LoadViewIntoFrame_Impl_NoThrow( *GetObjectShell(), xFrame, nViewId, false );

            rReq.Done();
            break;
        }

        case SID_OBJECT:
        {
            SFX_REQUEST_ARG( rReq, pItem, SfxUInt16Item, SID_OBJECT, sal_False );

            SfxViewShell *pViewShell = GetViewShell();
            if ( pViewShell && pItem )
            {
                pViewShell->DoVerb( pItem->GetValue() );
                rReq.Done();
                break;;
            }
        }
    }
}

//-------------------------------------------------------------------------
/* TODO as96863:
        This method try to collect information about the count of currently open documents.
        But the algorithm is implemented very simple ...
        E.g. hidden documents should be ignored here ... but they are counted.
        TODO: export special helper "framework::FrameListAnalyzer" within the framework module
        and use it here.
*/
sal_Bool impl_maxOpenDocCountReached()
{
    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    boost::optional<sal_Int32> x(officecfg::Office::Common::Misc::MaxOpenDocuments::get(xContext));
    // NIL means: count of allowed documents = infinite !
    if (!x)
        return sal_False;
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

//-------------------------------------------------------------------------
void SfxViewFrame::StateView_Impl
(
    SfxItemSet&     rSet            /*  empty <SfxItemSet> with <Which-Ranges>,
                                        which describes the Slot Ids */
)

/*  [Description]

    This internal methode returns in 'rSet' the Status for the  <SfxShell>
    Subclass SfxViewFrame in the <SVIDL> described <Slots>.

    Thus exactly those Slots-IDs that are recognized as beeing invalid by Sfx
    are included as Which-ranges in 'rSet'. If there exists a mapping for
    single slot-IDs of the <SfxItemPool> set in the shell, then the respective
    Which-IDs are used so that items can be replaced directly with a working
    Core::sun::com::star::script::Engine of the Which-IDs if possible. .
*/

{
    DBG_CHKTHIS(SfxViewFrame, 0);

    SfxObjectShell *pDocSh = GetObjectShell();

    if ( !pDocSh )
        // I'm just on reload and am yielding myself ...
        return;

    const sal_uInt16 *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges, "Set with no Range");
    while ( *pRanges )
    {
        for ( sal_uInt16 nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich )
        {
            switch(nWhich)
            {
                case SID_VIEWSHELL:
                {
                    rSet.Put( SfxUInt16Item( nWhich, pImp->nCurViewId ) );
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
                            nWhich, pImp->nCurViewId == rViewFactory.GetOrdinal() ) );
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

//-------------------------------------------------------------------------
void SfxViewFrame::ToTop()
{
    GetFrame().Appear();
}

//-------------------------------------------------------------------------
SfxViewFrame* SfxViewFrame::GetParentViewFrame() const
/*  [Description]

    The ParentViewFrame is the ViewFrame of the ParentFrames.
*/
{
    SfxFrame *pFrame = GetFrame().GetParentFrame();
    return pFrame ? pFrame->GetCurrentViewFrame() : NULL;
}

//-------------------------------------------------------------------------
SfxFrame& SfxViewFrame::GetFrame() const
/*  [Description]

    GetFrame returns the Frame, in which the ViewFrame is located.
*/
{
    return pImp->rFrame;
}

//-------------------------------------------------------------------------
SfxViewFrame* SfxViewFrame::GetTopViewFrame() const
{
    return GetFrame().GetTopFrame().GetCurrentViewFrame();
}

Window& SfxViewFrame::GetWindow() const
{
    return pImp->pWindow ? *pImp->pWindow : GetFrame().GetWindow();
}

sal_Bool SfxViewFrame::DoClose()
{
    return GetFrame().DoClose();
}

String SfxViewFrame::GetActualPresentationURL_Impl() const
{
    if ( xObjSh.Is() )
        return xObjSh->GetMedium()->GetName();
    return String();
}

void SfxViewFrame::SetModalMode( sal_Bool bModal )
{
    pImp->bModal = bModal;
    if ( xObjSh.Is() )
    {
        for ( SfxViewFrame* pFrame = SfxViewFrame::GetFirst( xObjSh );
              !bModal && pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, xObjSh ) )
            bModal = pFrame->pImp->bModal;
        xObjSh->SetModalMode_Impl( bModal );
    }
}

sal_Bool SfxViewFrame::IsInModalMode() const
{
    return pImp->bModal || GetFrame().GetWindow().IsInModalMode();
}

void SfxViewFrame::Resize( sal_Bool bForce )
{
    Size aSize = GetWindow().GetOutputSizePixel();
    if ( bForce || aSize != pImp->aSize )
    {
        pImp->aSize = aSize;
        SfxViewShell *pShell = GetViewShell();
        if ( pShell )
        {
            if ( GetFrame().IsInPlace() )
            {
                Point aPoint = GetWindow().GetPosPixel();
                DoAdjustPosSizePixel( pShell, aPoint, aSize );
            }
            else
            {
                DoAdjustPosSizePixel( pShell, Point(), aSize );
            }
        }
    }
}

#define LINE_SEP 0x0A

void CutLines( OUString& rStr, sal_Int32 nStartLine, sal_Int32 nLines, sal_Bool bEraseTrailingEmptyLines )
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

    DBG_ASSERTWARNING( nStartPos != STRING_NOTFOUND, "CutLines: Start row not found!" );

    if ( nStartPos != -1 )
    {
        sal_Int32 nEndPos = nStartPos;
        for ( sal_Int32 i = 0; i < nLines; i++ )
            nEndPos = rStr.indexOf( LINE_SEP, nEndPos+1 );

        if ( nEndPos == -1 ) // Can happen at the last row.
            nEndPos = rStr.getLength();
        else
            nEndPos++;

        OUString aEndStr = rStr.copy( nEndPos );
        rStr = rStr.copy( 0, nStartPos );
        rStr += aEndStr;
    }
    if ( bEraseTrailingEmptyLines )
    {
        sal_Int32 n = nStartPos;
        sal_Int32 nLen = rStr.getLength();
        while ( ( n < nLen ) && ( rStr.getStr()[ n ] == LINE_SEP ) )
            n++;

        if ( n > nStartPos )
        {
            OUString aEndStr = rStr.copy( n );
            rStr = rStr.copy( 0, nStartPos );
            rStr += aEndStr;
        }
    }
}

/*
    add new recorded dispatch macro script into the application global basic
    lib container. It generates a new unique id for it and insert the macro
    by using this number as name for the modul
 */
void SfxViewFrame::AddDispatchMacroToBasic_Impl( const OUString& sMacro )
{
#ifdef DISABLE_SCRIPTING
    (void) sMacro;
#else
    if ( sMacro.isEmpty() )
        return;

    SfxApplication* pSfxApp = SFX_APP();
    SfxRequest aReq( SID_BASICCHOOSER, SFX_CALLMODE_SYNCHRON, pSfxApp->GetPool() );
    aReq.AppendItem( SfxBoolItem(SID_RECORDMACRO,sal_True) );
    const SfxPoolItem* pRet = SFX_APP()->ExecuteSlot( aReq );
    String aScriptURL;
    if ( pRet )
        aScriptURL = ((SfxStringItem*)pRet)->GetValue();
    if ( aScriptURL.Len() )
    {
        // parse scriptURL
        String aLibName;
        String aModuleName;
        String aMacroName;
        String aLocation;
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        Reference< com::sun::star::uri::XUriReferenceFactory > xFactory =
            com::sun::star::uri::UriReferenceFactory::create( xContext );
        Reference< com::sun::star::uri::XVndSunStarScriptUrl > xUrl( xFactory->parse( aScriptURL ), UNO_QUERY );
        if ( xUrl.is() )
        {
            // get name
            OUString aName = xUrl->getName();
            sal_Unicode cTok = '.';
            sal_Int32 nIndex = 0;
            aLibName = aName.getToken( 0, cTok, nIndex );
            if ( nIndex != -1 )
                aModuleName = aName.getToken( 0, cTok, nIndex );
            if ( nIndex != -1 )
                aMacroName = aName.getToken( 0, cTok, nIndex );

            // get location
            OUString aLocKey("location");
            if ( xUrl->hasParameter( aLocKey ) )
                aLocation = xUrl->getParameter( aLocKey );
        }

        BasicManager* pBasMgr = 0;
        if ( aLocation.EqualsIgnoreCaseAscii( "application" ) )
        {
            // application basic
            pBasMgr = pSfxApp->GetBasicManager();
        }
        else if ( aLocation.EqualsIgnoreCaseAscii( "document" ) )
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
                if ( pModule )
                {
                    SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Find( aMacroName, SbxCLASS_METHOD );
                    aOUSource = pModule->GetSource32();
                    sal_uInt16 nStart, nEnd;
                    pMethod->GetLineRange( nStart, nEnd );
                    sal_uIntPtr nlStart = nStart;
                    sal_uIntPtr nlEnd = nEnd;
                    CutLines( aOUSource, nlStart-1, nlEnd-nlStart+1, sal_True );
                }
            }
        }

        // open lib container and break operation if it couldn't be opened
        com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer > xLibCont;
        if ( aLocation.EqualsIgnoreCaseAscii( "application" ) )
        {
            xLibCont = SFX_APP()->GetBasicContainer();
        }
        else if ( aLocation.EqualsIgnoreCaseAscii( "document" ) )
        {
            xLibCont = GetObjectShell()->GetBasicContainer();
        }

        if(!xLibCont.is())
        {
            SAL_WARN( "sfx.view", "couldn't get access to the basic lib container. Adding of macro isn't possible." );
            return;
        }

        // get LibraryContainer
        com::sun::star::uno::Any aTemp;
        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xRoot(
                xLibCont,
                com::sun::star::uno::UNO_QUERY);

        OUString sLib( aLibName );
        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xLib;
        if(xRoot->hasByName(sLib))
        {
            // library must be loaded
            aTemp = xRoot->getByName(sLib);
            xLibCont->loadLibrary(sLib);
            aTemp >>= xLib;
        }
        else
        {
            xLib = com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >(
                        xLibCont->createLibrary(sLib),
                        com::sun::star::uno::UNO_QUERY);
        }

        // pack the macro as direct usable "sub" routine
        OUString sCode;
        OUStringBuffer sRoutine(10000);
        OUString sMacroName( aMacroName );
        sal_Bool bReplace = sal_False;

        // get module
        OUString sModule( aModuleName );
        if(xLib->hasByName(sModule))
        {
            if ( !aOUSource.isEmpty() )
            {
                sRoutine.append( aOUSource );
            }
            else
            {
                aTemp = xLib->getByName(sModule);
                aTemp >>= sCode;
                sRoutine.append( sCode );
            }

            bReplace = sal_True;
        }

        // append new method
        sRoutine.appendAscii(RTL_CONSTASCII_STRINGPARAM("\nsub "));
        sRoutine.append(sMacroName);
        sRoutine.appendAscii(RTL_CONSTASCII_STRINGPARAM("\n"));
        sRoutine.append(sMacro);
        sRoutine.appendAscii(RTL_CONSTASCII_STRINGPARAM("\nend sub\n"));

        // create the modul inside the library and insert the macro routine
        aTemp <<= sRoutine.makeStringAndClear();
        if ( bReplace )
        {
            com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > xModulCont(
                xLib,
                com::sun::star::uno::UNO_QUERY);
            xModulCont->replaceByName(sModule,aTemp);
        }
        else
        {
            com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > xModulCont(
                xLib,
                com::sun::star::uno::UNO_QUERY);
            xModulCont->insertByName(sModule,aTemp);
        }

        // #i17355# update the Basic IDE
        for ( SfxViewShell* pViewShell = SfxViewShell::GetFirst(); pViewShell; pViewShell = SfxViewShell::GetNext( *pViewShell ) )
        {
            if ( pViewShell->GetName().EqualsAscii( "BasicIDE" ) )
            {
                SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();
                SfxDispatcher* pDispat = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
                if ( pDispat )
                {
                    SfxMacroInfoItem aInfoItem( SID_BASICIDE_ARG_MACROINFO, pBasMgr, aLibName, aModuleName, String(), String() );
                    pDispat->Execute( SID_BASICIDE_UPDATEMODULESOURCE, SFX_CALLMODE_SYNCHRON, &aInfoItem, 0L );
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
            OUString sProperty("DispatchRecorderSupplier");
            com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xFrame(
                    GetFrame().GetFrameInterface(),
                    com::sun::star::uno::UNO_QUERY);

            com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xSet(xFrame,com::sun::star::uno::UNO_QUERY);
            com::sun::star::uno::Any aProp = xSet->getPropertyValue(sProperty);
            com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorderSupplier > xSupplier;
            aProp >>= xSupplier;
            com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder;
            if (xSupplier.is())
                xRecorder = xSupplier->getDispatchRecorder();

            bool bIsRecording = xRecorder.is();
            SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, SID_RECORDMACRO, sal_False);
            if ( pItem && pItem->GetValue() == bIsRecording )
                return;

            if ( xRecorder.is() )
            {
                // disable active recording
                aProp <<= com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorderSupplier >();
                xSet->setPropertyValue(sProperty,aProp);

                SFX_REQUEST_ARG( rReq, pRecordItem, SfxBoolItem, FN_PARAM_1, sal_False);
                if ( !pRecordItem || !pRecordItem->GetValue() )
                    // insert script into basic library container of application
                    AddDispatchMacroToBasic_Impl(xRecorder->getRecordedMacro());

                xRecorder->endRecording();
                xRecorder = NULL;
                GetBindings().SetRecorder_Impl( xRecorder );

                SetChildWindow( SID_RECORDING_FLOATWINDOW, sal_False );
                if ( rReq.GetSlot() != SID_RECORDMACRO )
                    GetBindings().Invalidate( SID_RECORDMACRO );
            }
            else if ( rReq.GetSlot() == SID_RECORDMACRO )
            {
                // enable recording
                com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > xContext(
                        ::comphelper::getProcessComponentContext());

                xRecorder = com::sun::star::frame::DispatchRecorder::create( xContext );

                xSupplier = com::sun::star::frame::DispatchRecorderSupplier::create( xContext );

                xSupplier->setDispatchRecorder(xRecorder);
                xRecorder->startRecording(xFrame);
                aProp <<= xSupplier;
                xSet->setPropertyValue(sProperty,aProp);
                GetBindings().SetRecorder_Impl( xRecorder );
                SetChildWindow( SID_RECORDING_FLOATWINDOW, sal_True );
            }

            rReq.Done();
            break;
        }

        case SID_TOGGLESTATUSBAR:
        {
            com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xFrame(
                    GetFrame().GetFrameInterface(),
                    com::sun::star::uno::UNO_QUERY);

            Reference< com::sun::star::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
            Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
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
                OUString aStatusbarResString( "private:resource/statusbar/statusbar" );
                // Evaluate parameter.
                SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, rReq.GetSlot(), sal_False);
                sal_Bool bShow( sal_True );
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
            SFX_REQUEST_ARG(rReq, pItem, SfxBoolItem, rReq.GetSlot(), sal_False);
            SfxViewFrame *pTop = GetTopViewFrame();
            if ( pTop )
            {
                WorkWindow* pWork = (WorkWindow*) pTop->GetFrame().GetTopWindow_Impl();
                if ( pWork )
                {
                    com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xFrame(
                            GetFrame().GetFrameInterface(),
                            com::sun::star::uno::UNO_QUERY);

                    Reference< ::com::sun::star::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
                    Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
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

                    sal_Bool bNewFullScreenMode = pItem ? pItem->GetValue() : !pWork->IsFullScreenMode();
                    if ( bNewFullScreenMode != pWork->IsFullScreenMode() )
                    {
                        Reference< ::com::sun::star::beans::XPropertySet > xLMPropSet( xLayoutManager, UNO_QUERY );
                        if ( xLMPropSet.is() )
                        {
                            try
                            {
                                xLMPropSet->setPropertyValue(
                                    OUString( "HideCurrentUI" ),
                                    makeAny( bNewFullScreenMode ));
                            }
                            catch ( ::com::sun::star::beans::UnknownPropertyException& )
                            {
                            }
                        }
                        pWork->ShowFullScreenMode( bNewFullScreenMode );
                        pWork->SetMenuBarMode( bNewFullScreenMode ? MENUBAR_MODE_HIDE : MENUBAR_MODE_NORMAL );
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

            GetDispatcher()->Update_Impl( sal_True );
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
                    // Get the ContainerFrame, when internal InPlace.
                    SfxViewFrame *pFrame = this;
                    if ( pFrame->GetParentViewFrame_Impl() )
                        pFrame = pFrame->GetParentViewFrame_Impl();
                    rSet.Put( SfxStringItem( nWhich, pFrame->GetActualPresentationURL_Impl() ) );
                    break;
                }

                case SID_RECORDMACRO :
                {
                    SvtMiscOptions aMiscOptions;
                    const char* pName = GetObjectShell()->GetFactory().GetShortName();
                    if ( !aMiscOptions.IsMacroRecorderMode() ||
                         ( strcmp(pName,"swriter") && strcmp(pName,"scalc") ) )
                    {
                        rSet.DisableItem( nWhich );
                        rSet.Put(SfxVisibilityItem(nWhich, sal_False));
                        break;
                    }

                    OUString sProperty("DispatchRecorderSupplier");
                    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xSet(
                            GetFrame().GetFrameInterface(),
                            com::sun::star::uno::UNO_QUERY);

                    com::sun::star::uno::Any aProp = xSet->getPropertyValue(sProperty);
                    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorderSupplier > xSupplier;
                    if ( aProp >>= xSupplier )
                        rSet.Put( SfxBoolItem( nWhich, xSupplier.is() ) );
                    else
                        rSet.DisableItem( nWhich );
                    break;
                }

                case SID_STOP_RECORDING :
                {
                    SvtMiscOptions aMiscOptions;
                    const char* pName = GetObjectShell()->GetFactory().GetShortName();
                    if ( !aMiscOptions.IsMacroRecorderMode() ||
                         ( strcmp(pName,"swriter") && strcmp(pName,"scalc") ) )
                    {
                        rSet.DisableItem( nWhich );
                        break;
                    }

                    OUString sProperty("DispatchRecorderSupplier");
                    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xSet(
                            GetFrame().GetFrameInterface(),
                            com::sun::star::uno::UNO_QUERY);

                    com::sun::star::uno::Any aProp = xSet->getPropertyValue(sProperty);
                    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorderSupplier > xSupplier;
                    if ( !(aProp >>= xSupplier) || !xSupplier.is() )
                        rSet.DisableItem( nWhich );
                    break;
                }

                case SID_TOGGLESTATUSBAR:
                {
                    com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
                    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xSet(
                            GetFrame().GetFrameInterface(),
                            com::sun::star::uno::UNO_QUERY);
                    com::sun::star::uno::Any aProp = xSet->getPropertyValue(
                        OUString( "LayoutManager" ) );

                    if ( !( aProp >>= xLayoutManager ))
                        rSet.Put( SfxBoolItem( nWhich, sal_False ));
                    else
                    {
                        OUString aStatusbarResString( "private:resource/statusbar/statusbar" );
                        sal_Bool bShow = xLayoutManager->isElementVisible( aStatusbarResString );
                        rSet.Put( SfxBoolItem( nWhich, bShow ));
                    }
                    break;
                }

                case SID_WIN_FULLSCREEN:
                {
                    SfxViewFrame* pTop = GetTopViewFrame();
                    if ( pTop )
                    {
                        WorkWindow* pWork = (WorkWindow*) pTop->GetFrame().GetTopWindow_Impl();
                        if ( pWork )
                        {
                            rSet.Put( SfxBoolItem( nWhich, pWork->IsFullScreenMode() ) );
                            break;
                        }
                    }

                    rSet.DisableItem( nWhich );
                    break;
                }

                case SID_FORMATMENUSTATE :
                {
                    OSL_FAIL("Outdated slot!");
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

void SfxViewFrame::ChildWindowExecute( SfxRequest &rReq )

/*  [Description]

    This method can be included in the Execute method for the on- and off-
    switching of ChildWindows, to implement this and API-bindings.

    Simply include as 'ExecuteMethod' in the IDL.
*/

{
    // Evaluate Parameter
    sal_uInt16 nSID = rReq.GetSlot();

    SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, nSID, sal_False);
    if ( nSID == SID_VIEW_DATA_SOURCE_BROWSER )
    {
        if (!SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SDATABASE))
            return;
        Reference < XFrame > xFrame = GetFrame().GetTopFrame().GetFrameInterface();
        Reference < XFrame > xBeamer( xFrame->findFrame( "_beamer", FrameSearchFlag::CHILDREN ) );
        sal_Bool bShow = sal_False;
        sal_Bool bHasChild = xBeamer.is();
        bShow = pShowItem ? pShowItem->GetValue() : !bHasChild;
        if ( pShowItem )
        {
            if( bShow == bHasChild )
                return;
        }
        else
            rReq.AppendItem( SfxBoolItem( nSID, bShow ) );

        if ( !bShow )
        {
            SetChildWindow( SID_BROWSER, sal_False );
        }
        else
        {
            ::com::sun::star::util::URL aTargetURL;
            aTargetURL.Complete = OUString(".component:DB/DataSourceBrowser");
            Reference < ::com::sun::star::util::XURLTransformer > xTrans(
                    ::com::sun::star::util::URLTransformer::create(
                         ::comphelper::getProcessComponentContext() ) );
            xTrans->parseStrict( aTargetURL );

            Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
            Reference < ::com::sun::star::frame::XDispatch > xDisp;
            if ( xProv.is() )
                xDisp = xProv->queryDispatch( aTargetURL, OUString("_beamer"), 31 );
            if ( xDisp.is() )
            {
                Sequence < ::com::sun::star::beans::PropertyValue > aArgs(1);
                ::com::sun::star::beans::PropertyValue* pArg = aArgs.getArray();
                pArg[0].Name = OUString("Referer");
                pArg[0].Value <<= OUString("private:user");
                xDisp->dispatch( aTargetURL, aArgs );
            }
        }

        rReq.Done();
        return;
    }

    sal_Bool bShow = sal_False;
    sal_Bool bHasChild = HasChildWindow(nSID);
    bShow = pShowItem ? pShowItem->GetValue() : !bHasChild;

    // Perform action.
    if ( !pShowItem || bShow != bHasChild )
        ToggleChildWindow( nSID );

    GetBindings().Invalidate( nSID );
    GetDispatcher()->Update_Impl( sal_True );

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

//--------------------------------------------------------------------

void SfxViewFrame::ChildWindowState( SfxItemSet& rState )

/*  [Description]

    This method can be used in the state method for the on and off-state
    of child-windows, in order to implement this.

    Just register the IDL as 'StateMethod'.
*/

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
            const SfxPoolItem* pDummy = NULL;
            SfxItemState eState = GetDispatcher()->QueryState( SID_HYPERLINK_SETLINK, pDummy );
            if ( SFX_ITEM_DISABLED == eState )
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
            Reference < XFrame > xFrame = GetFrame().GetTopFrame().GetFrameInterface()->
                            findFrame( "_beamer", FrameSearchFlag::CHILDREN );
            if ( !xFrame.is() )
                rState.DisableItem( nSID );
            else if ( KnowsChildWindow(nSID) )
                rState.Put( SfxBoolItem( nSID, HasChildWindow(nSID) ) );
        }
        else if ( nSID == SID_TASKPANE )
        {
            if ( !KnowsChildWindow( nSID ) )
            {
                OSL_FAIL( "SID_TASKPANE state requested, but no task pane child window exists for this ID!" );
                rState.DisableItem( nSID );
            }
            else if ( !moduleHasToolPanels( *pImp ) )
            {
                rState.Put( SfxVisibilityItem( nSID, sal_False ) );
            }
            else
            {
                rState.Put( SfxBoolItem( nSID, HasChildWindow( nSID ) ) );
            }
        }
        else if ( nSID == SID_SIDEBAR )
        {
            if ( !IsSidebarEnabled() )
            {
                rState.DisableItem( nSID );
                rState.Put( SfxVisibilityItem( nSID, sal_False ) );
            }
            if  ( !KnowsChildWindow( nSID ) )
            {
                OSL_ENSURE( false, "SID_TASKPANE state requested, but no task pane child window exists for this ID!" );
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

//--------------------------------------------------------------------
SfxWorkWindow* SfxViewFrame::GetWorkWindow_Impl( sal_uInt16 /*nId*/ )
{
    SfxWorkWindow* pWork = 0;
    pWork = GetFrame().GetWorkWindow_Impl();
    return pWork;
}

void SfxViewFrame::SetChildWindow(sal_uInt16 nId, sal_Bool bOn, sal_Bool bSetFocus )
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    if ( pWork )
        pWork->SetChildWindow_Impl( nId, bOn, bSetFocus );
}

//--------------------------------------------------------------------

void SfxViewFrame::ToggleChildWindow(sal_uInt16 nId)
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    if ( pWork )
        pWork->ToggleChildWindow_Impl( nId, sal_True );
}

//--------------------------------------------------------------------

sal_Bool SfxViewFrame::HasChildWindow( sal_uInt16 nId )
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    return pWork ? pWork->HasChildWindow_Impl(nId) : sal_False;
}

//--------------------------------------------------------------------

sal_Bool SfxViewFrame::KnowsChildWindow( sal_uInt16 nId )
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    return pWork ? pWork->KnowsChildWindow_Impl(nId) : sal_False;
}

//--------------------------------------------------------------------

void SfxViewFrame::ShowChildWindow( sal_uInt16 nId, sal_Bool bVisible )
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    if ( pWork )
    {
        GetDispatcher()->Update_Impl(sal_True);
        pWork->ShowChildWindow_Impl(nId, bVisible, sal_True );
    }
}

//--------------------------------------------------------------------

SfxChildWindow* SfxViewFrame::GetChildWindow(sal_uInt16 nId)
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    return pWork ? pWork->GetChildWindow_Impl(nId) : NULL;
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
    SFX_APP()->SetViewFrame_Impl( pFrame );
}

// ---------------------------------------------------------------------------------------------------------------------
void SfxViewFrame::ActivateToolPanel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame, const OUString& i_rPanelURL )
{
    SolarMutexGuard aGuard;

    // look up the SfxFrame for the given XFrame
    SfxFrame* pFrame = NULL;
    for ( pFrame = SfxFrame::GetFirst(); pFrame; pFrame = SfxFrame::GetNext( *pFrame ) )
    {
        if ( pFrame->GetFrameInterface() == i_rFrame )
            break;
    }
    SfxViewFrame* pViewFrame = pFrame ? pFrame->GetCurrentViewFrame() : NULL;
    ENSURE_OR_RETURN_VOID( pViewFrame != NULL, "SfxViewFrame::ActivateToolPanel: did not find an SfxFrame for the given XFrame!" );

    pViewFrame->ActivateToolPanel_Impl( i_rPanelURL );
}

// ---------------------------------------------------------------------------------------------------------------------
void SfxViewFrame::ActivateToolPanel_Impl( const OUString& i_rPanelURL )
{
    // ensure the task pane is visible
    ENSURE_OR_RETURN_VOID( KnowsChildWindow( SID_TASKPANE ), "SfxViewFrame::ActivateToolPanel: this frame/module does not allow for a task pane!" );
    if ( !HasChildWindow( SID_TASKPANE ) )
        ToggleChildWindow( SID_TASKPANE );

    SfxChildWindow* pTaskPaneChildWindow = GetChildWindow( SID_TASKPANE );
    ENSURE_OR_RETURN_VOID( pTaskPaneChildWindow, "SfxViewFrame::ActivateToolPanel_Impl: just switched it on, but it is not there!" );

    ::sfx2::ITaskPaneToolPanelAccess* pPanelAccess = dynamic_cast< ::sfx2::ITaskPaneToolPanelAccess* >( pTaskPaneChildWindow );
    ENSURE_OR_RETURN_VOID( pPanelAccess, "SfxViewFrame::ActivateToolPanel_Impl: task pane child window does not implement a required interface!" );
    pPanelAccess->ActivateToolPanel( i_rPanelURL );
}

void SfxViewFrame::AppendInfoBar( const OUString& sId, const OUString& sMessage, std::vector< PushButton* > aButtons )
{
    const sal_uInt16 nId = SfxInfoBarContainerChild::GetChildWindowId();

    // Make sure the InfoBar container is visible
    if ( !HasChildWindow( nId ) )
        ToggleChildWindow( nId );
    SfxChildWindow* pChild = GetChildWindow( nId );
    if ( pChild )
    {
        SfxInfoBarContainerWindow* pInfoBars = ( SfxInfoBarContainerWindow* )pChild->GetWindow();
        pInfoBars->appendInfoBar( sId, sMessage, aButtons );
        ShowChildWindow( nId );
    }
}

void SfxViewFrame::RemoveInfoBar( const OUString& sId )
{
    const sal_uInt16 nId = SfxInfoBarContainerChild::GetChildWindowId();

    // Make sure the InfoBar container is visible
    if ( !HasChildWindow( nId ) )
        ToggleChildWindow( nId );
    SfxChildWindow* pChild = GetChildWindow( nId );
    if ( pChild )
    {
        SfxInfoBarContainerWindow* pInfoBars = ( SfxInfoBarContainerWindow* )pChild->GetWindow();
        SfxInfoBarWindow* pInfoBar = pInfoBars->getInfoBar( sId );
        pInfoBars->removeInfoBar( pInfoBar );
        ShowChildWindow( nId );
    }
}

bool SfxViewFrame::IsSidebarEnabled()
{
    static bool bInitialized = false;
    static bool bEnabled = false;

    // read the setting once at start, and that's what we
    // stick with for now.
    if (!bInitialized)
    {
        bInitialized = true;
        css::uno::Reference< css::uno::XComponentContext > xContext;
        xContext = ::comphelper::getProcessComponentContext();
        try {
            bEnabled = officecfg::Office::Common::Misc::ExperimentalSidebar::get( xContext );
        } catch (const uno::Exception &) {
            SAL_WARN("sfx.view", "don't have experimental sidebar option installed");
        }

        // rip out the services from framework/ for good measure
        try
        {
            uno::Reference< ui::XUIElementFactoryManager > xUIElementFactory = ui::UIElementFactoryManager::create( xContext );
            if( !bEnabled )
            {
                xUIElementFactory->deregisterFactory( "toolpanel", "ScPanelFactory", "" );
                xUIElementFactory->deregisterFactory( "toolpanel", "SwPanelFactory", "" );
                xUIElementFactory->deregisterFactory( "toolpanel", "SvxPanelFactory", "" );
                xUIElementFactory->deregisterFactory( "toolpanel", "SdPanelFactory", "" );
            }
            else
            {
                xUIElementFactory->deregisterFactory( "toolpanel", "DrawingFramework",
                                                      "com.sun.star.presentation.PresentationDocument" );
            }
        }
        catch ( const uno::Exception &e )
        {
            SAL_WARN( "sfx.view", "Exception de-registering sidebar factories " << e.Message );
        }
    }

    return bEnabled;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
