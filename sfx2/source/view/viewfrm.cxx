/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewfrm.cxx,v $
 * $Revision: 1.136.8.1 $
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
#include "precompiled_sfx2.hxx"

#include <stdio.h>

#include <sfx2/viewfrm.hxx>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _SPLITWIN_HXX //autogen
#include <vcl/splitwin.hxx>
#endif
#include <svtools/moduleoptions.hxx>
#include <svtools/intitem.hxx>
#include <svtools/stritem.hxx>
#include <svtools/eitem.hxx>
#include <svtools/slstitm.hxx>
#include <svtools/whiter.hxx>
#include <svtools/undo.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrames.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <rtl/ustrbuf.hxx>

#include <unotools/localfilehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/configurationhelper.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>

#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbx.hxx>
#include <comphelper/storagehelper.hxx>
#include <svtools/asynclink.hxx>
#include <svtools/sharecontrolfile.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
namespace css = ::com::sun::star;

#ifndef GCC
#endif

// wg. ViewFrame::Current
#include "appdata.hxx"
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
#include "sfxresid.hxx"
#include "appbas.hxx"
#include <sfx2/objitem.hxx>
#include "viewfac.hxx"
#include <sfx2/event.hxx>
#include "fltfnc.hxx"
#include <sfx2/docfile.hxx>
#include <sfx2/module.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/topfrm.hxx>
#include "viewimp.hxx"
#include <sfx2/sfxbasecontroller.hxx>
#include <sfx2/sfx.hrc>
#include "view.hrc"
#include <sfx2/frmdescr.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/progress.hxx>
#include "workwin.hxx"
#include "helper.hxx"
#include "macro.hxx"
#include "minfitem.hxx"
#include "../appl/app.hrc"
//-------------------------------------------------------------------------
DBG_NAME(SfxViewFrame)

#define SfxViewFrame
#include "sfxslots.hxx"

//-------------------------------------------------------------------------

SFX_IMPL_INTERFACE(SfxViewFrame,SfxShell,SfxResId(0))
{
    SFX_CHILDWINDOW_REGISTRATION( SID_BROWSER );
    SFX_CHILDWINDOW_REGISTRATION( SID_RECORDING_FLOATWINDOW );

    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_FULLSCREEN | SFX_VISIBILITY_FULLSCREEN, SfxResId(RID_FULLSCREENTOOLBOX) );
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_APPLICATION | SFX_VISIBILITY_STANDARD, SfxResId(RID_ENVTOOLBOX) );
}

TYPEINIT2(SfxViewFrame,SfxShell,SfxListener);
TYPEINIT1(SfxViewFrameItem, SfxPoolItem);

//=========================================================================

struct SfxViewFrame_Impl
{
    SvBorder            aBorder;
    Size                aMargin;
    Size                aSize;
    String              aViewData;
    String              aFrameTitle;
    TypeId              aLastType;
    String              aActualURL;
    String              aActualPresentationURL;
    SfxFrame*           pFrame;
    SfxCancelManager*   pCancelMgr;
    svtools::AsynchronLink* pReloader;
    //SfxInPlaceFrame*  pIPFrame;
    Window*             pWindow;
    SfxViewFrame*       pActiveChild;
    SfxViewFrame*       pParentViewFrame;
    SfxObjectShell*     pImportShell;
    Window*             pFocusWin;
    SfxMacro*           pMacro;
    sal_uInt16          nDocViewNo;
    sal_uInt16          nCurViewId;
    sal_Bool            bResizeInToOut:1;
    sal_Bool            bDontOverwriteResizeInToOut:1;
    sal_Bool            bObjLocked:1;
    sal_Bool            bRestoreView:1;
    sal_Bool            bSetViewFrameLocked:1;
    sal_Bool            bReloading:1;
    sal_Bool            bIsDowning:1;
    sal_Bool            bInCtor:1;
    sal_Bool            bModal:1;
    sal_Bool            bEnabled:1;
    sal_Bool            bEventFlag:1;
    sal_Bool            bWindowWasEnabled:1;

                        SfxViewFrame_Impl()
                        : pReloader(0 )
                        , pMacro( 0 )
                        , bWindowWasEnabled(sal_True)
                        {}

                        ~SfxViewFrame_Impl()
                        {
                            delete pReloader;
                            delete pCancelMgr;
                        }
};

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


//-------------------------------------------------------------------------
void SfxViewFrame::SetSetViewFrameAllowed_Impl( sal_Bool bSet )
{
    pImp->bSetViewFrameLocked = !bSet;
};

//-------------------------------------------------------------------------
sal_Bool SfxViewFrame::IsSetViewFrameAllowed_Impl() const
{
    return !pImp->bSetViewFrameLocked;
}

//-------------------------------------------------------------------------
void SfxViewFrame::SetImportingObjectShell_Impl( SfxObjectShell* pSh )
{
    pImp->pImportShell = pSh;
}

//--------------------------------------------------------------------
SfxObjectShell* SfxViewFrame::GetImportingObjectShell_Impl() const
{
    return pImp->pImportShell;
}


class SfxViewNotificatedFrameList_Impl :
    public SfxListener, public SfxViewFrameArr_Impl
{
public:

    void InsertViewFrame( SfxViewFrame* pFrame )
    {
        StartListening( *pFrame );
        C40_INSERT( SfxViewFrame, pFrame, Count() );
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
                    sal_uInt16 nPos = C40_GETPOS( SfxViewFrame, pFrame );
                    if( nPos != USHRT_MAX )
                        Remove( nPos );
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

void SfxViewFrame::ExecReload_Impl( SfxRequest& rReq, sal_Bool bAsync )
{
    if( bAsync )
    {
        if( !pImp->pReloader )
            pImp->pReloader = new svtools::AsynchronLink(
                Link( this, ReloadDecouple_Impl ) );
        pImp->pReloader->Call( new SfxRequest( rReq ) );
    }
    else ExecReload_Impl( rReq );
}

void SfxViewFrame::ExecReload_Impl( SfxRequest& rReq )
{
    SfxFrame *pParent = GetFrame()->GetParentFrame();
    if ( rReq.GetSlot() == SID_RELOAD )
    {
        // Bei CTRL-Reload den aktiven Frame reloaden
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

        // Wenn nur ein Reload der Graphiken eines oder mehrerer ChildFrames
        // gemacht werden soll
        SfxFrame *pFrame = GetFrame();
        if ( pParent == pFrame && pFrame->GetChildFrameCount() )
        {
            sal_Bool bReloadAvailable = sal_False;
            SfxFrameIterator aIter( *pFrame, sal_False );
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

            // Der TopLevel-Frame selbst het keine Graphiken!
            if ( bReloadAvailable )
                return;
        }
    }
    else
    {
        // Bei CTRL-Edit den TopFrame bearbeiten
        sal_uInt16 nModifier = rReq.GetModifier();

        if ( ( nModifier & KEY_MOD1 ) && pParent )
        {
            SfxViewFrame *pTop = GetTopViewFrame();
            pTop->ExecReload_Impl( rReq );
            return;
        }
    }

    SfxObjectShell* pSh = GetObjectShell();
    sal_Bool bWasReadonly = pSh->IsReadOnly();

    switch ( rReq.GetSlot() )
    {
        case SID_EDITDOC:
        {
            if ( GetFrame()->HasComponent() )
                break;

            // Wg. Doppeltbelegung in Toolboxen (mit/ohne Ctrl) ist es auch
            // m"oglich, da\s der Slot zwar enabled ist, aber Ctrl-Click
            // trotzdem nicht geht!
            if( !pSh || !pSh->HasName() || !(pSh->Get_Impl()->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ))
                break;

            SFX_ITEMSET_ARG( pSh->GetMedium()->GetItemSet(), pItem, SfxBoolItem, SID_VIEWONLY, sal_False );
            if ( pItem && pItem->GetValue() )
            {
                SfxMedium* pMed = pSh->GetMedium();
                SfxApplication* pApp = SFX_APP();
                SfxAllItemSet aSet( pApp->GetPool() );
                aSet.Put( SfxStringItem( SID_FILE_NAME, pMed->GetURLObject().GetMainURL(INetURLObject::NO_DECODE) ) );
                aSet.Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
                aSet.Put( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii("_blank") ) );
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
                // Speichern und Readonly Reloaden
                if( pSh->IsModified() )
                {
                    if ( !pSh->PrepareClose() )
                    {
                        rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), sal_False ) );
                        return;
                    }
                    else bNeedsReload = sal_True;
                }
                nOpenMode = SFX_STREAM_READONLY;
            }
            else
            {
                nOpenMode = SFX_STREAM_READWRITE;
                pSh->SetReadOnlyUI( sal_False );

                // if only the view was in the readonly mode then there is no need to do the reload
                if ( !pSh->IsReadOnly() )
                    return;
            }

            // Parameter auswerten
            // sal_Bool bReload = sal_True;
            if ( rReq.IsAPI() )
            {
                // per API steuern ob r/w oder r/o
                SFX_REQUEST_ARG(rReq, pEditItem, SfxBoolItem, SID_EDITDOC, sal_False);
                if ( pEditItem )
                    nOpenMode = pEditItem->GetValue() ? SFX_STREAM_READWRITE : SFX_STREAM_READONLY;
            }

            // doing
            if( pSh  )
            {
                SfxMedium* pMed = pSh->GetMedium();
                String aTemp;
                utl::LocalFileHelper::ConvertPhysicalNameToURL( pMed->GetPhysicalName(), aTemp );
                INetURLObject aPhysObj( aTemp );
                SFX_ITEMSET_ARG( pSh->GetMedium()->GetItemSet(),
                                 pVersionItem, SfxInt16Item, SID_VERSION, sal_False );

                INetURLObject aMedObj( pMed->GetName() );

                // the logic below is following, if the document seems not to need to be reloaded and the physical name is different
                // to the logical one, then on file system it can be checked that the copy is still newer than the original and no document reload is required
                if ( ( !bNeedsReload && ( aMedObj.GetProtocol() == INET_PROT_FILE &&
                        aMedObj.getFSysPath(INetURLObject::FSYS_DETECT) != aPhysObj.getFSysPath(INetURLObject::FSYS_DETECT) &&
                        SfxContentHelper::IsYounger( aPhysObj.GetMainURL( INetURLObject::NO_DECODE ), aMedObj.GetMainURL( INetURLObject::NO_DECODE ) )
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
                        pMed->SetOpenMode( nOpenMode, pMed->IsDirect() );

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
                            pMed->SetOpenMode( SFX_STREAM_READONLY, pMed->IsDirect() );
                            pMed->ReOpen();
                            pSh->DoSaveCompleted( pMed );
                        }

                        // r/o-Doc kann nicht in Editmode geschaltet werden?
                        rReq.Done( sal_False );

                        SFX_REQUEST_ARG( rReq, pFSetItem, SfxBoolItem, SID_EDIT_FRAMESET, sal_False);
                        if ( nOpenMode == SFX_STREAM_READWRITE && !rReq.IsAPI() )
                        {
                            // dem ::com::sun::star::sdbcx::User anbieten, als Vorlage zu oeffnen
                            QueryBox aBox( &GetWindow(), SfxResId(MSG_QUERY_OPENASTEMPLATE) );
                            if ( !pFSetItem && RET_YES == aBox.Execute() )
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
                        // if( nOpenMode == SFX_STREAM_READONLY )
                        //    pMed->Close();

                        // ReloadForEdit bei Framesets schaltet auch FramesetEditmode
                        sal_Bool bIsReadonly = GetObjectShell()->IsReadOnly();
                        if ( bIsReadonly != bWasReadonly && !GetFrame()->GetParentFrame() )
                        {
                            SfxBoolItem aItem( SID_EDIT_FRAMESET, !bIsReadonly );
                            GetDispatcher()->Execute( SID_EDIT_FRAMESET,
                                    SFX_CALLMODE_RECORD, &aItem, 0L );
                        }
                        return;
                    }
                }

                /*
                if ( !bReload )
                {
                    // Es soll nicht reloaded werden
                    SfxErrorContext aEc( ERRCODE_SFX_NODOCRELOAD );
                    ErrorHandler::HandleError( ERRCODE_SFX_NODOCRELOAD );
                    rReq.SetReturnValue(
                        SfxBoolItem( rReq.GetSlot(), sal_False ) );
                    return;
                }
                */
                // Ansonsten ( lokal und arbeiten auf Kopie ) muss gereloaded
                // werden.
            }

            rReq.AppendItem( SfxBoolItem( SID_FORCERELOAD, sal_True) );
            rReq.AppendItem( SfxBoolItem( SID_SILENT, sal_True ));
        }

        case SID_RELOAD:
        {
            // Wg. Doppeltbelegung in Toolboxen (mit/ohne Ctrl) ist es auch
            // m"oglich, da\s der Slot zwar enabled ist, aber Ctrl-Click
            // trotzdem nicht geht!
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

            // AutoLoad ist ggf. verboten
            SFX_REQUEST_ARG(rReq, pAutoLoadItem, SfxBoolItem, SID_AUTOLOAD, sal_False);
            if ( pAutoLoadItem && pAutoLoadItem->GetValue() &&
                 GetFrame()->IsAutoLoadLocked_Impl() )
                return;

            SfxObjectShellLock xOldObj( pSh );
            pImp->bReloading = sal_True;
            SFX_REQUEST_ARG(rReq, pURLItem, SfxStringItem,
                            SID_FILE_NAME, sal_False);
            // editierbar "offnen?
            sal_Bool bForEdit = !pSh->IsReadOnly();
            if ( rReq.GetSlot() == SID_EDITDOC )
                bForEdit = !bForEdit;

            // ggf. beim User nachfragen
            sal_Bool bDo = ( GetViewShell()->PrepareClose() != FALSE );
            SFX_REQUEST_ARG(rReq, pSilentItem, SfxBoolItem, SID_SILENT, sal_False);
            if ( bDo && GetFrame()->DocIsModified_Impl() &&
                 !rReq.IsAPI() && ( !pSilentItem || !pSilentItem->GetValue() ) )
            {
                QueryBox aBox( &GetWindow(), SfxResId(MSG_QUERY_LASTVERSION) );
                bDo = ( RET_YES == aBox.Execute() );
            }

            if ( bDo )
            {
                SfxMedium *pMedium = xOldObj->GetMedium();

                // Frameset abziehen, bevor FramesetView evtl. verschwindet
                String aURL = pURLItem ? pURLItem->GetValue() :
                                pMedium->GetName();

                sal_Bool bHandsOff =
                    ( pMedium->GetURLObject().GetProtocol() == INET_PROT_FILE && !xOldObj->IsDocShared() );

                // bestehende SfxMDIFrames f"ur dieses Doc leeren
                // eigenes Format oder R/O jetzt editierbar "offnen?
                SfxViewNotificatedFrameList_Impl aFrames;
                SfxObjectShellLock xNewObj;
                sal_Bool bRestoreView = ( pURLItem == NULL );
                TypeId aOldType = xOldObj->Type();

                SfxViewFrame *pView = GetFirst(xOldObj);
                while(pView)
                {
                    if( bHandsOff )
                        pView->GetDispatcher()->LockUI_Impl(sal_True);
                    aFrames.InsertViewFrame( pView );
                    pView->GetBindings().ENTERREGISTRATIONS();

                    // RestoreView nur wenn keine neue Datei geladen
                    // (Client-Pull-Reloading)
                    pView = /*bHandsOff ? (SfxTopViewFrame*) GetFirst(
                        xOldObj, TYPE(SfxTopViewFrame) ) :*/
                        (SfxTopViewFrame*)GetNext( *pView, xOldObj,
                                               TYPE( SfxTopViewFrame ) );
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
                    pNewSet->ClearItem( SID_USER_DATA );
                    pNewSet->ClearItem( SID_STREAM );
                    pNewSet->ClearItem( SID_INPUTSTREAM );
                    pNewSet->Put( SfxStringItem( SID_FILTER_NAME, pMedium->GetFilter()->GetName() ) );

                    // let the current security settings be checked again
                    pNewSet->Put( SfxUInt16Item( SID_MACROEXECMODE, document::MacroExecMode::USE_CONFIG ) );

                    if ( rReq.GetSlot() == SID_EDITDOC || !bForEdit )
                        // edit mode is switched or reload of readonly document
                        pNewSet->Put( SfxBoolItem( SID_DOC_READONLY, !bForEdit ) );
                    else
                        // Reload of file opened for writing
                        pNewSet->ClearItem( SID_DOC_READONLY );
                }

                // Falls eine salvagede Datei vorliegt, nicht nochmals die
                // OrigURL mitschicken, denn die Tempdate ist nach Reload
                // ungueltig
                SFX_ITEMSET_ARG( pNewSet, pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
                if( pSalvageItem )
                {
                    aURL = pSalvageItem->GetValue();
                    pNewSet->ClearItem( SID_DOC_SALVAGE );
                }

                // TODO/LATER: Temporary solution, the SfxMedium must know the original URL as aLogicName
                //             SfxMedium::Transfer_Impl() will be vorbidden then.
                if ( xOldObj->IsDocShared() )
                    pNewSet->Put( SfxStringItem( SID_FILE_NAME, xOldObj->GetSharedFileURL() ) );

                //pNewMedium = new SfxMedium( aURL, nMode, pMedium->IsDirect(), bUseFilter ? pMedium->GetFilter() : 0, pNewSet );
                //pNewSet = pNewMedium->GetItemSet();
                if ( pURLItem )
                    pNewSet->Put( SfxStringItem( SID_REFERER, pMedium->GetName() ) );
                else
                    pNewSet->Put( SfxStringItem( SID_REFERER, String() ) );

                xOldObj->CancelTransfers();

                // eigentliches Reload
                //pNewSet->Put( SfxFrameItem ( SID_DOCFRAME, GetFrame() ) );
                //pNewSet->Put( SfxBoolItem( SID_SILENT, sal_True ) );

                SFX_ITEMSET_ARG(pNewSet, pInteractionItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, FALSE);
                SFX_ITEMSET_ARG(pNewSet, pMacroExecItem  , SfxUInt16Item, SID_MACROEXECMODE     , FALSE);
                SFX_ITEMSET_ARG(pNewSet, pDocTemplateItem, SfxUInt16Item, SID_UPDATEDOCMODE     , FALSE);

                if (!pInteractionItem)
                {
                    Reference < ::com::sun::star::task::XInteractionHandler > xHdl( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.comp.uui.UUIInteractionHandler")), UNO_QUERY );
                    if (xHdl.is())
                        pNewSet->Put( SfxUnoAnyItem(SID_INTERACTIONHANDLER,::com::sun::star::uno::makeAny(xHdl)) );
                }

                if (!pMacroExecItem)
                    pNewSet->Put( SfxUInt16Item(SID_MACROEXECMODE,::com::sun::star::document::MacroExecMode::USE_CONFIG) );
                if (!pDocTemplateItem)
                    pNewSet->Put( SfxUInt16Item(SID_UPDATEDOCMODE,::com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG) );

                xOldObj->SetModified( sal_False );
                // Altes Dok nicht cachen! Gilt nicht, wenn anderes
                // Doc geladen wird.

                SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSavedOptions, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False);
                SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSavedReferer, SfxStringItem, SID_REFERER, sal_False);

                if( !pURLItem || pURLItem->GetValue() == xOldObj->GetMedium()->GetName() )
                    xOldObj->Get_Impl()->bForbidCaching = sal_True;

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
                try
                {
                    uno::Sequence < beans::PropertyValue > aProps;
                    TransformItems( SID_OPENDOC, *pNewSet, aProps );
                    uno::Reference < frame::XLoadable > xLoad( xNewObj->GetModel(), uno::UNO_QUERY );
                    xLoad->load( aProps );
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
                            aSet.Put( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii("_blank") ) );
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
                    else
                    {
                        // an error handling should be done here?!
                        // if ( !pSilentItem || !pSilentItem->GetValue() )
                        //    ErrorHandler::HandleError( nLoadError );
                    }
                }
                else
                {
                    if ( xNewObj->IsDocShared() )
                    {
                        // the file is shared but the closing can chang the sharing control file
                        xOldObj->DoNotCleanShareControlFile();
                    }

                    xNewObj->GetMedium()->GetItemSet()->ClearItem( SID_RELOAD );
                    UpdateDocument_Impl();
                }

                SfxViewFrame* pThis = (SfxViewFrame*)this;
                sal_Bool bDeleted = aFrames.C40_GETPOS( SfxViewFrame, pThis ) == USHRT_MAX;

                if( !bDeleted )
                {
                    GetBindings().Invalidate( SID_RELOAD );
                    pImp->bReloading = sal_False;
                }

                // neues Doc in die bestehenden SfxMDIFrames einsetzen; wenn
                // das Reload geklappt hat, mu\s in diesem Frame kein Dokument
                // eingesetzt werden, weil das schon vom LoadEnvironment
                // gemacht wurde
                if ( xNewObj.Is() && xNewObj->Type() != aOldType )
                    // RestoreView nur, wenn gleicher Dokumenttyp
                    bRestoreView = sal_False;

                const sal_uInt16 nCount = aFrames.Count();
                for(sal_uInt16 i = 0; i < nCount; ++i)
                {
                    SfxViewFrame *pCurrView = aFrames.GetObject( i );
                    if ( xNewObj.Is() )
                    {
                        //if( /*!bHandsOff &&*/ this != pView   )
                        pCurrView->ReleaseObjectShell_Impl( bRestoreView );
                        pCurrView->SetRestoreView_Impl( bRestoreView );
                        //if( pView != this || !xNewObj.Is() )
                        {
                            SfxFrame *pFrame = pCurrView->GetFrame();
                            pFrame->InsertDocument(xNewObj.Is() ? xNewObj : xOldObj );
                        }
                    }

                    pCurrView->GetBindings().LEAVEREGISTRATIONS();
                    pCurrView->GetDispatcher()->LockUI_Impl( sal_False );
                }

                if ( xNewObj.Is() )
                {
                    // Propagate document closure.
                    SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_CLOSEDOC, xOldObj ) );
                }

                // als erledigt recorden
                rReq.Done( sal_True );
                rReq.SetReturnValue(SfxBoolItem(rReq.GetSlot(), sal_True));
                if( !bDeleted )
                {
                    Notify( *GetObjectShell(), SfxSimpleHint(
                        SFX_HINT_TITLECHANGED ));
                }
                return;
            }
            else
            {
                // als nicht erledigt recorden
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
        // Ich bin gerade am Reloaden und Yielde so vor mich hin ...
        return;

    GetFrame()->GetParentFrame();
    SfxWhichIter aIter( rSet );
    for ( sal_uInt16 nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich() )
    {
        if ( GetFrame()->HasComponent() )
        {
            // Wenn die Komponente es nicht selbst dispatched, dann
            // macht es auch keinen Sinn!
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
                SfxFrame* pFrame = GetTopFrame();
                SfxViewFrame *pView = pFrame->GetCurrentViewFrame();
                if ( pView && pView->GetViewShell() &&
                    pView->GetViewShell()->IsImplementedAsFrameset_Impl() &&
                    pView->GetViewShell()->GetInterface()->GetSlot( nWhich ) )
                {
                    // Hack f"ur Explorer: Reload wird an der ViewShell ausgef"uhrt
                    pView->GetViewShell()->GetSlotState( nWhich, 0, &rSet );
                    break;
                }

                if ( !pSh || !pSh->CanReload_Impl() || pSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
                    rSet.DisableItem(nWhich);
                else
                {
                    // Wenn irgendein ChildFrame reloadable ist, wird der Slot
                    // enabled, damit man CTRL-Reload machen kann
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
    // gibt es an der obersten Shell einen Undo-Manager?
    SfxShell *pSh = GetDispatcher()->GetShell(0);
    SfxUndoManager* pShUndoMgr = pSh->GetUndoManager();
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
                pShUndoMgr->Undo(0);
                GetBindings().InvalidateAll(sal_False);
                bOK = sal_True;
                break;

            case SID_REDO:
                pShUndoMgr->Redo(0);
                GetBindings().InvalidateAll(sal_False);
                bOK = sal_True;
                break;

            case SID_REPEAT:
                if ( pSh->GetRepeatTarget() )
                    pShUndoMgr->Repeat( *pSh->GetRepeatTarget(), 0);
                bOK = sal_True;
                break;
        }
    }
    else if ( GetViewShell() )
    {
        // der SW hat eigenes Undo an der ::com::sun::star::sdbcx::View
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
    // Undo-Manager suchen
    SfxShell *pSh = GetDispatcher()->GetShell(0);
    if ( !pSh )
        // Ich bin gerade am Reloaden und Yielde so vor mich hin ...
        return;

    SfxUndoManager *pShUndoMgr = pSh->GetUndoManager();
    if ( !pShUndoMgr )
    {
        // der SW hat eigenes Undo an der ::com::sun::star::sdbcx::View
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
        String aTmp( SfxResId( STR_UNDO ) );
        aTmp += pShUndoMgr->GetUndoActionComment(0);
        rSet.Put( SfxStringItem( SID_UNDO, aTmp ) );
    }
    else
        rSet.DisableItem( SID_UNDO );

    if ( pShUndoMgr && pShUndoMgr->GetRedoActionCount() )
    {
        String aTmp( SfxResId(STR_REDO) );
        aTmp += pShUndoMgr->GetRedoActionComment(0);
        rSet.Put( SfxStringItem( SID_REDO, aTmp ) );
    }
    else
        rSet.DisableItem( SID_REDO );
    SfxRepeatTarget *pTarget = pSh->GetRepeatTarget();
    if ( pShUndoMgr && pTarget && pShUndoMgr->GetRepeatActionCount() &&
         pShUndoMgr->CanRepeat(*pTarget, 0) )
    {
        String aTmp( SfxResId(STR_REPEAT) );
        aTmp += pShUndoMgr->GetRepeatActionComment(*pTarget, 0);
        rSet.Put( SfxStringItem( SID_REPEAT, aTmp ) );
    }
    else
        rSet.DisableItem( SID_REPEAT );
}

//--------------------------------------------------------------------
void SfxViewFrame::SetObjectShell_Impl
(
    SfxObjectShell& rObjSh, // eine initialisierte SfxObjectShell,
    FASTBOOL        bDefaultView    // sal_True: nicht restaurieren
)

/*  [Beschreibung]

    Diese Methode setzt eine <SfxObjectShell> in den SfxViewFrame ein.

    Zuvor mu\s die vorherige SfxObjectShell, insofern schein eine gesetzt
    wurde, mit der Methode ReleaseObjectShell() entfernt worden sein. Somit
    kann durch Aufruf von ReleaseObjectShell() und SetObjectShell() die
    SfxObjectShell ausgetauscht werden.


    [Querverweise]

    <SfxViewFrame::ReleaseObjectShell()>
*/

{
    DBG_CHKTHIS(SfxViewFrame, 0);
    DBG_ASSERT( !xObjSh.Is(), "old feature used: only one Object per View!" );

    GetFrame()->ReleasingComponent_Impl( sal_False );

    // Doc einsetzen
    xObjSh = &rObjSh;
    if ( xObjSh.Is() && xObjSh->IsPreview() )
        SetQuietMode_Impl( sal_True );

    GetFrame()->SetFrameType_Impl( GetFrameType() & ~SFXFRAME_FRAMESET );

    // Modulshell einf"ugen
    SfxModule* pModule = xObjSh->GetModule();
    if( pModule )
        pDispatcher->InsertShell_Impl( *pModule, 1 );

    pDispatcher->Push( rObjSh );
    pDispatcher->Flush();
    StartListening( rObjSh );

    rObjSh.ViewAssigned();
    pDispatcher->SetReadOnly_Impl( rObjSh.IsReadOnly() );

    const SfxMedium *pMedium = GetObjectShell()->GetMedium();
    SFX_ITEMSET_ARG(
        pMedium->GetItemSet(), pHiddenItem, SfxBoolItem, SID_HIDDEN, sal_False );
    if ( !pHiddenItem || !pHiddenItem->GetValue() )
    {
        LockObjectShell_Impl(sal_True);
        GetDocNumber_Impl();
    }

    // ::com::sun::star::sdbcx::View erzeugen
    if ( bDefaultView )
        SetRestoreView_Impl( sal_False );

// So darf man es nicht machen, da LaodWindows hierueber laeuft. Kann meiner Meinung nach
// auch nur beim Reload mit Dokumenttypwechsel passieren.
/*    if ( xObjSh->Type() != pImp->aLastType )
        SetRestoreView_Impl( sal_False ); */

    SwitchToViewShell_Impl( !IsRestoreView_Impl() ? (sal_uInt16) 0 : GetCurViewId() );
    rObjSh.PostActivateEvent_Impl( this );
    if ( Current() == this )
        SFX_APP()->NotifyEvent(SfxEventHint(SFX_EVENT_ACTIVATEDOC, &rObjSh ) );

    Notify( rObjSh, SfxSimpleHint(SFX_HINT_TITLECHANGED) );
    Notify( rObjSh, SfxSimpleHint(SFX_HINT_DOCCHANGED) );

    // Zur Sicherheit, aber eigentlich sollte jetzt nichts mehr passieren
    // ( kein erzwungenes Update )
    if ( SfxViewFrame::Current() == this )
        GetDispatcher()->Update_Impl();
}

//--------------------------------------------------------------------
void SfxViewFrame::ReleaseObjectShell_Impl( sal_Bool bStoreView )

/*  [Beschreibung]

    Diese Methode entleert den SfxViewFrame, d.h. nimmt die <SfxObjectShell>
    vom Dispatcher und beendet seine <SfxListener>-Beziehung zu dieser
    SfxObjectShell (wodurch sie sich ggf. selbst zerst"ort).

    Somit kann durch Aufruf von ReleaseObjectShell() und SetObjectShell()
    die SfxObjectShell ausgetauscht werden.

    Zwischen RealeaseObjectShell() und SetObjectShell() darf die Kontrolle
    nicht an das ::com::sun::star::chaos::System abgegeben werden.


    [Querverweise]

    <SfxViewFrame::SetObjectShell(SfxObjectShell&)>
*/
{
    DBG_CHKTHIS(SfxViewFrame, 0);
    DBG_ASSERT( xObjSh.Is(), "no SfxObjectShell to release!" );

    GetFrame()->ReleasingComponent_Impl( sal_True );
    if ( GetWindow().HasChildPathFocus( sal_True ) )
    {
        DBG_ASSERT( !GetActiveChildFrame_Impl(), "Wrong active child frame!" );
        GetWindow().GrabFocus();
    }

    SfxViewShell *pDyingViewSh = GetViewShell();
    if ( pDyingViewSh )
    {
        SetRestoreView_Impl( bStoreView );
        if ( bStoreView )
            pDyingViewSh->WriteUserData( GetViewData_Impl(), sal_True );

        // Jetzt alle SubShells wechhauen
        pDyingViewSh->PushSubShells_Impl( sal_False );
        sal_uInt16 nLevel = pDispatcher->GetShellLevel( *pDyingViewSh );
        if ( nLevel && nLevel != USHRT_MAX )
        {
            // Es gibt immer nocht SubShells
            SfxShell *pSubShell = pDispatcher->GetShell( nLevel-1 );
            if ( pSubShell == pDyingViewSh->GetSubShell() )
                //"Echte" Subshells nicht deleten
                pDispatcher->Pop( *pSubShell, SFX_SHELL_POP_UNTIL );
            else
                pDispatcher->Pop( *pSubShell, SFX_SHELL_POP_UNTIL | SFX_SHELL_POP_DELETE );
        }
        pDispatcher->Pop( *pDyingViewSh );
        pDispatcher->Flush();
        pDyingViewSh->DisconnectAllClients();
        SetViewShell_Impl(0);
        delete pDyingViewSh;
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Keine Shell");
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

//-------------------------------------------------------------------------

String SfxViewFrame::UpdateTitle()

/*  [Beschreibung]

    Mit dieser Methode kann der SfxMDIFrame gezwungen werden, sich sofort
    den neuen Titel vom der <SfxObjectShell> zu besorgen.

    [Anmerkung]

    Dies ist z.B. dann notwendig, wenn man der SfxObjectShell als SfxListener
    zuh"ort und dort auf den <SfxSimpleHint> SFX_HINT_TITLECHANGED reagieren
    m"ochte, um dann die Titel seiner Views abzufragen. Diese Views (SfxMDIFrames)
    jedoch sind ebenfalls SfxListener und da die Reihenfolge der Benachrichtigung
    nicht feststeht, mu\s deren Titel-Update vorab erzwungen werden.


    [Beispiel]

    void SwDocShell::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
    {
        if ( rHint.IsA(TYPE(SfxSimpleHint)) )
        {
            switch( ( (SfxSimpleHint&) rHint ).GetId() )
            {
                case SFX_HINT_TITLECHANGED:
                    for ( SfxMDIFrame *pMDI = (SfxMDIFrame*)
                                SfxViewFrame::GetFirst(this, TYPE(SfxMDIFrame));
                          pMDI;
                          pMDI = (SfxMDIFrame*)
                                SfxViewFrame::GetNext(this, TYPE(SfxMDIFrame));
                    {
                        pMDI->UpdateTitle();
                        ... pMDI->GetName() ...
                    }
                    break;
                ...
            }
        }
    }
*/

{
    SfxObjectShell *pObjSh = GetObjectShell();
    if ( !pObjSh )
        return String();

//    if  ( pObjSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
//        // kein UpdateTitle mit Embedded-ObjectShell
//        return String();

    const SfxMedium *pMedium = pObjSh->GetMedium();
    String aURL;
    GetFrame();  // -Wall required??
    if ( pObjSh->HasName() )
    {
        INetURLObject aTmp( pMedium->GetName() );
        aURL = aTmp.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
    }

    if ( aURL != pImp->aActualURL )
        // URL hat sich ge"andert
        pImp->aActualURL = aURL;

    // gibt es noch eine weitere View?
    sal_uInt16 nViews=0;
    for ( SfxViewFrame *pView= GetFirst(pObjSh);
          pView && nViews<2;
          pView = GetNext(*pView,pObjSh) )
        if ( ( pView->GetFrameType() & SFXFRAME_HASTITLE ) &&
             !IsDowning_Impl())
            nViews++;

    // Titel des Fensters
    String aTitle;
    if ( nViews == 2 || pImp->nDocViewNo > 1 )
        // dann die Nummer dranh"angen
        aTitle = pObjSh->UpdateTitle( NULL, pImp->nDocViewNo );
    else
        aTitle = pObjSh->UpdateTitle();

    // Name des SbxObjects
    String aSbxName = pObjSh->SfxShell::GetName();
    if ( IsVisible_Impl() )
    {
        aSbxName += ':';
        aSbxName += String::CreateFromInt32(pImp->nDocViewNo);
    }

    SetName( aSbxName );
    pImp->aFrameTitle = aTitle;
    GetBindings().Invalidate( SID_FRAMETITLE );
    GetBindings().Invalidate( SID_CURRENT_URL );
    return aTitle;
}


//--------------------------------------------------------------------
sal_Bool SfxViewFrame::Close()
{
    DBG_CHKTHIS(SfxViewFrame, 0);

    DBG_ASSERT( GetFrame()->IsClosing_Impl() || !GetFrame()->GetFrameInterface().is(), "ViewFrame closed too early!" );

    // Wenn bis jetzt noch nicht gespeichert wurde, sollen eingebettete Objekte
    // auch nicht mehr automatisch gespeichert werden!
    if ( GetViewShell() )
        GetViewShell()->DiscardClients_Impl();
    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    return sal_True;
}

//--------------------------------------------------------------------

void SfxViewFrame::DoActivate( sal_Bool bUI, SfxViewFrame* pOldFrame )
{
    DBG_CHKTHIS(SfxViewFrame, 0);
    SFX_APP();

#ifdef WIN
    pSfxApp->TestFreeResources_Impl();
#endif

    pDispatcher->DoActivate_Impl( bUI, pOldFrame );

    // Wenn ich einen parent habe und dieser ist kein parent des alten
    // ViewFrames, erh"alt er ein ParentActivate
    if ( bUI )
    {
/*
        SfxMedium* pMed = GetObjectShell() ? GetObjectShell()->GetMedium() : NULL;
        if( pMed )
        {
            SFX_ITEMSET_ARG(
                pMed->GetItemSet(), pInterceptorItem, SfxSlotInterceptorItem,
                SID_INTERCEPTOR, sal_False );
            if( pInterceptorItem )
            {
                SfxSlotInterceptor* pInter = pInterceptorItem->GetValue();
                if( !pInter->GetBindings() )
                    pInter->SetBindings( &GetBindings() );
                pInter->Activate( sal_True );
            }
        }
 */
        SfxViewFrame *pFrame = GetParentViewFrame();
        while ( pFrame )
        {
            if ( !pOldFrame || !pOldFrame->GetFrame()->IsParent( pFrame->GetFrame() ) )
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

    // Wenn ich einen parent habe und dieser ist kein parent des neuen
    // ViewFrames, erh"alt er ein ParentDeactivate
    if ( bUI )
    {
//        if ( GetFrame()->GetWorkWindow_Impl() )
//            GetFrame()->GetWorkWindow_Impl()->SaveStatus_Impl();
/*
        SfxMedium* pMed = GetObjectShell() ? GetObjectShell()->GetMedium() : NULL;
        if( pMed )
        {
            SFX_ITEMSET_ARG(
                pMed->GetItemSet(), pInterceptorItem, SfxSlotInterceptorItem,
                SID_INTERCEPTOR, sal_False );
            if( pInterceptorItem )
                pInterceptorItem->GetValue()->Activate( sal_False );
        }
*/
        SfxViewFrame *pFrame = GetParentViewFrame();
        while ( pFrame )
        {
            if ( !pNewFrame || !pNewFrame->GetFrame()->IsParent( pFrame->GetFrame() ) )
                pFrame->pDispatcher->DoParentDeactivate_Impl();
            pFrame = pFrame->GetParentViewFrame();
        }
    }
#ifdef WIN
    pSfxApp->TestFreeResources_Impl();
#endif
}

//------------------------------------------------------------------------
void SfxViewFrame::InvalidateBorderImpl( const SfxViewShell* pSh )
{
    if( pSh && !nAdjustPosPixelLock )
    {
        if ( GetViewShell() && GetWindow().IsVisible() )
        {
            if ( GetFrame()->IsInPlace() )
            {
                /*
                Size aSize( GetViewShell()->GetWindow()->GetSizePixel() );

                //Size aBorderSz( pEnv->GetBorderWin()->GetHatchBorderPixel() );
                Point aOfs; //( aBorderSz.Width(), aBorderSz.Height() );

                DoAdjustPosSizePixel( GetViewShell(), aOfs, aSize );*/
                return;
            }

            if ( GetViewShell()->UseObjectSize() )
            {
                // Zun"achst die Gr"o\se des MDI-Fensters berechnen

                DoAdjustPosSizePixel( GetViewShell(), Point(),
                                GetViewShell()->GetWindow()->GetSizePixel() );

                // Da nach einem InnerResize die Position des EditFensters und
                // damit auch der Tools nocht stimmt, mu\s nun noch einmal von
                // au\sen resized werden !

                ForceOuterResize_Impl(sal_True);
            }

            DoAdjustPosSizePixel( (SfxViewShell *) GetViewShell(), Point(),
                                            GetWindow().GetOutputSizePixel() );
            if ( GetViewShell()->UseObjectSize() )
                ForceOuterResize_Impl(sal_False);
        }
    }
}

//------------------------------------------------------------------------
sal_Bool SfxViewFrame::SetBorderPixelImpl
(
    const SfxViewShell* /*pSh*/,
    const SvBorder&     rBorder
)

{
    pImp->aBorder = rBorder;
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
    if ( !xObjSh.Is() )
        return;

    if ( rHint.IsA(TYPE(SfxSimpleHint)) )
    {
        switch( ( (SfxSimpleHint&) rHint ).GetId() )
        {
            case SFX_HINT_MODECHANGED:
            {
                // r/o Umschaltung?
                SfxDispatcher *pDispat = GetDispatcher();
                sal_Bool bWasReadOnly = pDispat->GetReadOnly_Impl();
                sal_Bool bIsReadOnly = xObjSh->IsReadOnly();
                if ( !bWasReadOnly != !bIsReadOnly )
                {
                    // Dann auch TITLE_CHANGED
                    UpdateTitle();
                    GetBindings().Invalidate( SID_FILE_NAME );
                    GetBindings().Invalidate( SID_DOCINFO_TITLE );

                    pDispat->GetBindings()->InvalidateAll(sal_True);
                    pDispat->SetReadOnly_Impl( bIsReadOnly );

                    // Dispatcher-Update nur erzwingen, wenn es nicht sowieso
                    // demn"achst kommt, anderenfalls ist Zappelei oder gar
                    // GPF m"oglich, da Writer z.B. gerne mal im Resize irgendwelche
                    // Aktionen t"atigt, die ein SetReadOnlyUI am Dispatcher zur
                    // Folge haben!
                    if ( pDispat->IsUpdated_Impl() )
                        pDispat->Update_Impl(sal_True);
                }

                Enable( !xObjSh->IsInModalMode() );
                break;
            }

            case SFX_HINT_TITLECHANGED:
            {
                UpdateTitle();
                GetBindings().Invalidate( SID_FILE_NAME );
                GetBindings().Invalidate( SID_DOCINFO_TITLE );
                break;
            }

            case SFX_HINT_DYING:
            case SFX_HINT_DEINITIALIZING:
                // when the Object is being deleted, destroy the view too
                if ( xObjSh.Is() )
                    ReleaseObjectShell_Impl();
                else
                    GetFrame()->DoClose();
                break;

        }
    }
    else if ( rHint.IsA(TYPE(SfxEventHint)) )
    {
        // Wenn das Document asynchron geladen wurde, wurde der Dispatcher
        // auf ReadOnly gesetzt, was zur"?ckgenommen werden mu\s, wenn
        // das Document selbst nicht ReadOnly ist und das Laden fertig ist.
        switch ( ((SfxEventHint&)rHint).GetEventId() )
        {
            case SFX_EVENT_MODIFYCHANGED:
            {
                SfxBindings& rBind = GetBindings();
                rBind.Invalidate( SID_DOC_MODIFIED );
                rBind.Invalidate( SID_SAVEDOC );
                rBind.Invalidate( SID_RELOAD );
                break;
            }

            case SFX_EVENT_OPENDOC:
            case SFX_EVENT_CREATEDOC:
            {
                if ( !xObjSh->IsReadOnly() )
                {
                    // Im Gegensatz zu oben (TITLE_CHANGED) mu\s das UI nicht
                    // upgedated werden, da es nicht gehidet war!

                    // #i21560# InvalidateAll() causes the assertion
                    // 'SfxBindings::Invalidate while in update" when
                    // the sfx slot SID_BASICIDE_APPEAR is executed
                    // via API from another thread (Java).
                    // According to MBA this call is not necessary anymore,
                    // because each document has its own SfxBindings.
                    //
                    //GetDispatcher()->GetBindings()->InvalidateAll(sal_True);
                }

                break;
            }

            case SFX_EVENT_TOGGLEFULLSCREENMODE:
            {
                if ( GetFrame()->OwnsBindings_Impl() )
                    GetBindings().GetDispatcher_Impl()->Update_Impl( sal_True );
                break;
            }
        }
    }
}

//------------------------------------------------------------------------
void SfxViewFrame::Construct_Impl( SfxObjectShell *pObjSh )
{
    pImp->pFrame->DocumentInserted( pObjSh );
    pImp->bInCtor = sal_True;
    pImp->pParentViewFrame = 0;
    pImp->bResizeInToOut = sal_True;
    pImp->bDontOverwriteResizeInToOut = sal_False;
    pImp->pImportShell = 0;
    pImp->bObjLocked = sal_False;
    pImp->bEventFlag = sal_True;
    pImp->pFocusWin = 0;
    pImp->pActiveChild = NULL;
    pImp->bRestoreView = sal_False;
    pImp->nCurViewId = 0;
    pImp->bSetViewFrameLocked = sal_False;
    pImp->bReloading = sal_False;
    pImp->bIsDowning = sal_False;
    pImp->bModal = sal_False;
    pImp->bEnabled = sal_True;
    pImp->nDocViewNo = 0;
    pImp->aMargin = Size( -1, -1 );
    pImp->pCancelMgr = 0;
    pImp->pWindow = 0;

    SetPool( &SFX_APP()->GetPool() );
    pDispatcher = new SfxDispatcher(this);
    if ( !GetBindings().GetDispatcher() )
        GetBindings().SetDispatcher( pDispatcher );

    xObjSh = pObjSh;
    if ( xObjSh.Is() && xObjSh->IsPreview() )
        SetQuietMode_Impl( sal_True );

    GetFrame()->SetFrameType_Impl( GetFrameType() & ~SFXFRAME_FRAMESET );
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

    SfxViewFrame *pThis = this; // wegen der kranken Array-Syntax
    SfxViewFrameArr_Impl &rViewArr = SFX_APP()->GetViewFrames_Impl();
    rViewArr.C40_INSERT(SfxViewFrame, pThis, rViewArr.Count() );
    pImp->bInCtor = sal_False;
}

//------------------------------------------------------------------------
SfxViewFrame::SfxViewFrame( SfxObjectShell &rObjShell, SfxBindings &rBindings,
                            SfxFrame* pParent, sal_uInt32 nType )
:
    pImp( new SfxViewFrame_Impl ),
    pDispatcher(0),
    pBindings(&rBindings),
    nAdjustPosPixelLock( 0 )
{
    DBG_CTOR(SfxViewFrame, 0);

    SetFrame_Impl( pParent );
    pImp->pFrame->SetCurrentViewFrame_Impl( this );
    GetFrame()->SetFrameType_Impl( GetFrameType() | nType );
    Construct_Impl( &rObjShell );
}

//------------------------------------------------------------------------
SfxViewFrame::SfxViewFrame(const SfxViewFrame &rCopy, SfxBindings &rBindings,
    SfxFrame *pFrame )
:
    pImp( new SfxViewFrame_Impl ),
    pDispatcher(0),
    pBindings(&rBindings),
    nAdjustPosPixelLock( 0 )
{
    DBG_CTOR(SfxViewFrame, 0);

    SetFrame_Impl( pFrame );
    pImp->pFrame->SetCurrentViewFrame_Impl( this );
    GetFrame()->SetFrameType_Impl( rCopy.GetFrameType() );
    Construct_Impl( rCopy.GetObjectShell() );
}

SfxViewFrame::SfxViewFrame( SfxBindings& rBindings, SfxFrame *pFrame,
        SfxObjectShell *pDoc, sal_uInt32 nType )
    : pImp( new SfxViewFrame_Impl )
    , pDispatcher(0)
    , pBindings(&rBindings)
    , nAdjustPosPixelLock( 0 )
{
    DBG_CTOR(SfxViewFrame, 0);

    SetFrame_Impl( pFrame );
    pImp->pFrame->SetCurrentViewFrame_Impl( this );
    GetFrame()->SetFrameType_Impl( GetFrameType() | nType );
    Construct_Impl( pDoc);
}

//------------------------------------------------------------------------
SfxViewFrame::~SfxViewFrame()
{
    DBG_DTOR(SfxViewFrame, 0);

    if ( GetFrame() && GetFrame()->GetCurrentViewFrame() == this )
        GetFrame()->SetCurrentViewFrame_Impl( NULL );

    SfxObjectShell* pSh = pImp->pImportShell;
    if( pSh )
        pSh->AbortImport();

    // von Frame-Liste abmelden
    SfxApplication *pSfxApp = SFX_APP();
    SfxViewFrameArr_Impl &rFrames = pSfxApp->GetViewFrames_Impl();
    const SfxViewFrame *pThis = this;
    rFrames.Remove( rFrames.GetPos(pThis) );

    // Member l"oschen
    KillDispatcher_Impl();

    delete pImp;
}

//------------------------------------------------------------------------
void SfxViewFrame::KillDispatcher_Impl()

// Dispatcher abr"aumen und l"oschen

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
    return SfxApplication::Is_Impl() ? SFX_APP()->Get_Impl()->pViewFrame : NULL;
}

//--------------------------------------------------------------------
sal_uInt16 SfxViewFrame::Count(TypeId aType)

/*  [Beschreibung]

    Liefert die Anzahl der sichtbaren <SfxViewFrame>-Instanzen vom Typ
    'aType' bzw. aller sichtbaren, falls 'aType==0' (default).
*/

{
    SfxApplication *pSfxApp = SFX_APP();
    SfxViewFrameArr_Impl& rFrames = pSfxApp->GetViewFrames_Impl();
    const sal_uInt16 nCount = rFrames.Count();
    sal_uInt16 nFound = 0;
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        SfxViewFrame *pFrame = rFrames[i];
        if ( ( !aType || pFrame->IsA(aType) ) &&
             pFrame->IsVisible_Impl() )
            ++nFound;
    }
    return nFound;
}

//--------------------------------------------------------------------
// returns the first window of spec. type viewing the specified doc.
SfxViewFrame* SfxViewFrame::GetFirst
(
    const SfxObjectShell*   pDoc,
    TypeId                  aType,
    sal_Bool                    bOnlyIfVisible
)
{
    SfxApplication *pSfxApp = SFX_APP();
    SfxViewFrameArr_Impl &rFrames = pSfxApp->GetViewFrames_Impl();

    // search for a SfxDocument of the specified type
    for ( sal_uInt16 nPos = 0; nPos < rFrames.Count(); ++nPos )
    {
        SfxViewFrame *pFrame = rFrames.GetObject(nPos);
        if ( ( !pDoc || pDoc == pFrame->GetObjectShell() ) &&
             ( !aType || pFrame->IsA(aType) ) &&
             ( !bOnlyIfVisible || pFrame->IsVisible_Impl()) )
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
    TypeId                  aType,
    sal_Bool                    bOnlyIfVisible
)
{
    SfxApplication *pSfxApp = SFX_APP();
    SfxViewFrameArr_Impl &rFrames = pSfxApp->GetViewFrames_Impl();

    // refind the specified predecessor
    sal_uInt16 nPos;
    for ( nPos = 0; nPos < rFrames.Count(); ++nPos )
        if ( rFrames.GetObject(nPos) == &rPrev )
            break;

    // search for a Frame of the specified type
    for ( ++nPos; nPos < rFrames.Count(); ++nPos )
    {
        SfxViewFrame *pFrame = rFrames.GetObject(nPos);
        if ( ( !pDoc || pDoc == pFrame->GetObjectShell() ) &&
             ( !aType || pFrame->IsA(aType) ) &&
             ( !bOnlyIfVisible || pFrame->IsVisible_Impl()) )
            return pFrame;
    }
    return 0;
}

void SfxViewFrame::CloseHiddenFrames_Impl()
{
    SfxApplication *pSfxApp = SFX_APP();
    SfxViewFrameArr_Impl &rFrames = pSfxApp->GetViewFrames_Impl();
    for ( sal_uInt16 nPos=0; nPos<rFrames.Count(); )
    {
        SfxViewFrame *pFrame = rFrames.GetObject(nPos);
        if ( !pFrame->IsVisible_Impl() )
            pFrame->DoClose();
        else
            nPos++;
    }
}

//--------------------------------------------------------------------
SfxProgress* SfxViewFrame::GetProgress() const
{
    SfxObjectShell *pObjSh = GetObjectShell();
    return pObjSh ? pObjSh->GetProgress() : 0;
}

//--------------------------------------------------------------------
void SfxViewFrame::ShowStatusText( const String& /*rText*/)
{
/* OBSOLETE: If this is used, framework/uielement/progressbarwrapper.[h|c]xx &
             framework/uielement/statusindicatorinterfacewrapper.[h|c]xx must be
             extended to support a new interface to support ShowStatusText/HideStatusText
    SfxWorkWindow* pWorkWin = GetFrame()->GetWorkWindow_Impl();
    SfxStatusBarManager *pMgr = pWorkWin->GetStatusBarManager_Impl();
    if ( pMgr )
    {
        pMgr->GetStatusBar()->HideItems();
        pMgr->GetStatusBar()->SetText( rText );
    }
*/
}

//--------------------------------------------------------------------
void SfxViewFrame::HideStatusText()
{
/* OBSOLETE: If this is used, framework/uielement/progressbarwrapper.[h|c]xx &
             framework/uielement/statusindicatorinterfacewrapper.[h|c]xx must be
             extended to support a new interface to support ShowStatusText/HideStatusText
    SfxWorkWindow* pWorkWin = GetFrame()->GetWorkWindow_Impl();
    SfxStatusBarManager *pMgr = pWorkWin->GetStatusBarManager_Impl();
    if ( pMgr )
        pMgr->GetStatusBar()->ShowItems();
*/
}


//--------------------------------------------------------------------
#ifdef ENABLE_INIMANAGER//MUSTINI
SfxIniManager* SfxViewFrame::GetIniManager() const
{
/*  SfxIniManager *pIniMgr = GetObjectShell()
            ? GetObjectShell()->GetFactory().GetIniManager()
            : 0;
    if ( !pIniMgr )*/ //!
        return SFX_APP()->GetAppIniManager();
//  return pIniMgr;
}
#endif

//--------------------------------------------------------------------
void SfxViewFrame::DoAdjustPosSizePixel //! teilen in Inner.../Outer...
(
    SfxViewShell*   pSh,
    const Point&    rPos,
    const Size&     rSize
)
{
    DBG_CHKTHIS(SfxViewFrame, 0);

    // Components benutzen diese Methode nicht!
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
String SfxViewFrameItem::GetValueText() const
{
    return String();
}

//--------------------------------------------------------------------
SfxPoolItem* SfxViewFrameItem::Clone( SfxItemPool *) const
{
    return new SfxViewFrameItem( pFrame);
}

//--------------------------------------------------------------------
void SfxViewFrame::SetViewShell_Impl( SfxViewShell *pVSh )

/*  [Beschreibung]

    Interne Methode zum setzen der jeweils aktuellen <SfxViewShell>-Instanz,
    die in diesem SfxViewFrame aktiv ist.
*/

{
    SfxShell::SetViewShell_Impl( pVSh );

    // Hack: InPlaceMode
    if ( pVSh && !pVSh->UseObjectSize() )
        pImp->bResizeInToOut = sal_False;
}

//--------------------------------------------------------------------
/*
    Beschreibung:
    Der ParentViewFrame ist der ViewFrame des Containers bei internem InPlace
*/

void SfxViewFrame::SetParentViewFrame_Impl(SfxViewFrame *pFrame)
{
    pImp->pParentViewFrame = pFrame;
}

//--------------------------------------------------------------------
/*
    Beschreibung:
    Der ParentViewFrame ist der ViewFrame des Containers bei internem InPlace
*/

//TODO/LATER: is it still necessary? is there a replacement for GetParentViewFrame_Impl?
SfxViewFrame* SfxViewFrame::GetParentViewFrame_Impl() const
{
    return pImp->pParentViewFrame;
}

//--------------------------------------------------------------------
void SfxViewFrame::ForceOuterResize_Impl(sal_Bool bOn)
{
    if ( !pImp->bDontOverwriteResizeInToOut )
        pImp->bResizeInToOut = !bOn;
}

void SfxViewFrame::ForceInnerResize_Impl(sal_Bool bOn)
{
    pImp->bDontOverwriteResizeInToOut = bOn;
}

//--------------------------------------------------------------------
sal_Bool SfxViewFrame::IsResizeInToOut_Impl() const
{
    return pImp->bResizeInToOut;
}
//--------------------------------------------------------------------
void SfxViewFrame::DoAdjustPosSize( SfxViewShell *pSh,
                                const Point rPos, const Size &rSize )
{
    DBG_CHKTHIS(SfxViewFrame, 0);
    if( pSh && !nAdjustPosPixelLock && pSh->UseObjectSize())
    {
        Window *pWindow = pSh->GetWindow();
        Point aPos = pWindow->LogicToPixel(rPos);
        Size aSize = pWindow->LogicToPixel(rSize);
        DoAdjustPosSizePixel(pSh, aPos, aSize);
    }
}

//--------------------------------------------------------------------
void SfxViewFrame::GetDocNumber_Impl()
{
    DBG_ASSERT( GetObjectShell(), "Kein Dokument!" );
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
            Window *pWindow = &GetFrame()->GetTopFrame()->GetWindow();
            if ( !bEnable )
                pImp->bWindowWasEnabled = pWindow->IsInputEnabled();
            if ( !bEnable || pImp->bWindowWasEnabled )
                pWindow->EnableInput( bEnable, TRUE );
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
/*
        if ( !bEnable )
            GetBindings().ENTERREGISTRATIONS();
        GetDispatcher()->Lock( !bEnable );
        if ( bEnable )
            GetBindings().LEAVEREGISTRATIONS();
*/
    }
}

//--------------------------------------------------------------------
void SfxViewFrame::Show()

/*  [Beschreibung]

    Diese Methode macht das Frame-Window sichtbar und ermittelt vorher
    den Fenstername. Au\serdem wird das Dokument festgehalten. Man darf
    i.d.R. nie das Window direkt showen!
*/

{
    // zuerst locken damit in UpdateTitle() gilt: IsVisible() == sal_True (:#)
    if ( xObjSh.Is() )
    {
        xObjSh->GetMedium()->GetItemSet()->ClearItem( SID_HIDDEN );
        if ( !pImp->bObjLocked )
            LockObjectShell_Impl( sal_True );

        // Doc-Shell Titel-Nummer anpassen, get unique view-no
        if ( 0 == pImp->nDocViewNo && !(GetFrameType() & SFXFRAME_PLUGIN ) )
        {
            GetDocNumber_Impl();
            UpdateTitle();
        }
    }
    else
        UpdateTitle();

    // Frame-Window anzeigen, aber nur wenn der ViewFrame kein eigenes Window
    // hat oder wenn er keine Component enth"alt
    if ( &GetWindow() == &GetFrame()->GetWindow() || !GetFrame()->HasComponent() )
        GetWindow().Show();
    GetFrame()->GetWindow().Show();

/*    SfxViewFrame* pCurrent = SfxViewFrame::Current();
    if ( GetFrame()->GetFrameInterface()->isActive() &&
            pCurrent != this &&
            ( !pCurrent || pCurrent->GetParentViewFrame_Impl() != this ) &&
            !GetActiveChildFrame_Impl() )
        MakeActive_Impl( FALSE );*/
    if ( xObjSh.Is() && xObjSh->Get_Impl()->bHiddenLockedByAPI )
    {
        xObjSh->Get_Impl()->bHiddenLockedByAPI = FALSE;
        xObjSh->OwnerLock(FALSE);
    }
}

//--------------------------------------------------------------------
sal_Bool SfxViewFrame::IsVisible_Impl() const
{
    //Window *pWin = pImp->bInCtor ? 0 : &GetWindow();
    //return GetFrame()->HasComponent() || pImp->bObjLocked || ( pWin && pWin->IsVisible() );
    return pImp->bObjLocked;
}

//--------------------------------------------------------------------
void SfxViewFrame::Hide()
{
    GetWindow().Hide();
    if ( pImp->bObjLocked )
        LockObjectShell_Impl( sal_False );
}

//--------------------------------------------------------------------
void SfxViewFrame::LockObjectShell_Impl( sal_Bool bLock )
{
    DBG_ASSERT( pImp->bObjLocked != bLock, "Falscher Locked-Status!" );

    DBG_ASSERT( GetObjectShell(), "Kein Dokument!" );
    GetObjectShell()->OwnerLock(bLock);
    pImp->bObjLocked = bLock;
}

//--------------------------------------------------------------------
void SfxViewFrame::MakeActive_Impl( BOOL bGrabFocus )
{
    if ( GetViewShell() && !GetFrame()->IsClosing_Impl() )
    {
        if ( IsVisible_Impl() )
        {
            if ( GetViewShell() )
            {
                BOOL bPreview = FALSE;
                if ( GetObjectShell()->IsPreview() )
                {
                    bPreview = TRUE;
                }
                else
                {
                    SfxViewFrame* pParent = GetParentViewFrame();
                    if ( pParent )
                        pParent->SetActiveChildFrame_Impl( this );
                }

                SfxViewFrame* pCurrent = SfxViewFrame::Current();
                css::uno::Reference< css::frame::XFrame > xFrame = GetFrame()->GetFrameInterface();
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
                                GetFrame()->GrabFocusOnComponent_Impl();
                    }
                }
                else
                {
                    GetBindings().SetDispatcher( GetDispatcher() );
                    GetBindings().SetActiveFrame( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > () );
                    GetDispatcher()->Update_Impl( FALSE );
                }
            }
        }
    }
}

//--------------------------------------------------------------------

SfxViewShell* SfxViewFrame::CreateView_Impl( sal_uInt16 nViewId )

/*  [Beschreibung]

    Erzeugt eine SfxViewShell f"ur diesen SfxViewFrame. Wird auch aus
    <SfxObjectShell::LoadWindows_Impl()>
    gerufen.
*/

{
    DBG_ASSERT( GetObjectShell(), "Kein Dokument!" );

    LockAdjustPosSizePixel();

    // passende ::com::sun::star::sdbcx::View-Factory suchen
    SfxObjectFactory &rDocFact = GetObjectShell()->GetFactory();
    sal_uInt16 nNewNo = nViewId ? USHRT_MAX : 0;
    for ( sal_uInt16 nNo = 0;
          nNo < rDocFact.GetViewFactoryCount();
          ++nNo )
    {
        sal_uInt16 nFoundId = rDocFact.GetViewFactory(nNo).GetOrdinal();
        if ( nNewNo == USHRT_MAX )
        {
            if ( nViewId == nFoundId )
                nNewNo = nNo;
        }
    }

    // per Factory erzeugen
    GetBindings().ENTERREGISTRATIONS();
    SfxViewFactory &rViewFactory = rDocFact.GetViewFactory( nNewNo );
    SfxViewShell *pViewShell = rViewFactory.CreateInstance(this, 0);
    SetViewShell_Impl(pViewShell);
    UnlockAdjustPosSizePixel();

    if ( GetWindow().IsReallyVisible() )
        DoAdjustPosSizePixel(pViewShell, Point(), GetWindow().GetOutputSizePixel());

    // erste jetzt anzeigen (ausser wenn MTs BASIC-IDE mal wieder keins hat)
    Window *pViewWin = pViewShell->GetWindow();
    if ( pViewWin && pViewShell->IsShowView_Impl() )
        pViewWin->Show();

    // Dispatcher
    GetDispatcher()->Push( *pViewShell );
    if ( pViewShell->GetSubShell() )
        GetDispatcher()->Push( *pViewShell->GetSubShell() );
    pViewShell->PushSubShells_Impl();

#if defined SFX_HINT_VIEWCREATED
    GetObjectShell()->Broadcast( SfxSimpleHint( SFX_HINT_VIEWCREATED ) );
#endif

    GetBindings().LEAVEREGISTRATIONS();

    DBG_ASSERT( SFX_APP()->GetViewFrames_Impl().Count() == SFX_APP()->GetViewShells_Impl().Count(), "Inconsistent view arrays!" );
    return pViewShell;
}

//-------------------------------------------------------------------------
SfxViewFrame* SfxViewFrame::SearchViewFrame( SfxViewFrame *pViewFrame,
    const String& rName )
{
    if ( !pViewFrame )
        pViewFrame = SfxViewFrame::Current();
    if ( !pViewFrame )
        return NULL;
    SfxFrame *pFrame = pViewFrame->GetFrame()->SearchFrame( rName );
    if ( !pFrame )
        return NULL;

    // Der Frame ist selbst ein ViewFrame oder enth"alt einen
/*
    pViewFrame = PTR_CAST( SfxViewFrame, pFrame );
    if ( !pViewFrame && pFrame->GetChildFrameCount() )
        pViewFrame = PTR_CAST( SfxViewFrame, pFrame->GetChildFrame(0) );
*/
    return pFrame->GetCurrentViewFrame();
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

void SfxViewFrame::SetMargin_Impl( const Size& rMargin )
{
    pImp->aMargin = rMargin;
}

const Size& SfxViewFrame::GetMargin_Impl() const
{
    return pImp->aMargin;
}

void SfxViewFrame::SetActiveChildFrame_Impl( SfxViewFrame *pViewFrame )
{
    if ( pViewFrame != pImp->pActiveChild )
    {
        if ( !pImp->pActiveChild )
            GetDispatcher()->LockUI_Impl( sal_False );

        pImp->pActiveChild = pViewFrame;

        Reference< XFramesSupplier > xFrame( GetFrame()->GetFrameInterface(), UNO_QUERY );
        Reference< XFrame >  xActive;
        if ( pViewFrame )
            xActive = pViewFrame->GetFrame()->GetFrameInterface();

        if ( xFrame.is() )  // PB: #74432# xFrame cann be NULL
            xFrame->setActiveFrame( xActive );
    }
}

SfxViewFrame* SfxViewFrame::GetActiveChildFrame_Impl() const
{
    SfxViewFrame *pViewFrame = pImp->pActiveChild;
/*
    if ( !pViewFrame )
    {
        // Wenn es keinen aktiven ChildFrame gibt, irgendeinen nehmen
        for ( sal_uInt16 n=0; n<GetChildFrameCount(); n++ )
        {
            pViewFrame =
                PTR_CAST( SfxViewFrame, GetChildFrame(n)->GetChildFrame(0) );
            if ( pViewFrame )
                break;
        }
    }

    pImp->pActiveChild = pViewFrame;
*/
    return pViewFrame;
}

//--------------------------------------------------------------------

sal_Bool SfxViewFrame::SwitchToViewShell_Impl
(
    sal_uInt16  nViewId,        /*  > 0
                                Registrierungs-Id der ::com::sun::star::sdbcx::View, auf die umge-
                                schaltet werden soll, bzw. die erstmalig
                                erzeugt werden soll.

                                == 0
                                Es soll die Default-::com::sun::star::sdbcx::View verwendet werden. */

    sal_Bool    bIsIndex        /*  sal_True
                                'nViewId' ist keine Registrations-Id sondern
                                ein ::com::sun::star::sdbcx::Index in die f"ur die in diesem
                                <SfxViewFrame> dargestellte <SfxObjectShell>.
                                */
)

/*  [Beschreibung]

    Interne Methode zum Umschalten auf eine andere <SfxViewShell>-Subklasse,
    die in diesem SfxMDIFrame erzeugt werden soll. Existiert noch
    keine SfxViewShell in diesem SfxMDIFrame, so wird erstmalig eine
    erzeugt.


    [R"uckgabewert]

    sal_Bool                        sal_True
                                die angeforderte SfxViewShell wurde erzeugt
                                und eine ggf. bestehende gel"oscht

                                sal_False
                                die angeforderte SfxViewShell konnte nicht
                                erzeugt werden, die bestehende SfxViewShell
                                existiert daher weiterhin
*/

{
    try{
    DBG_ASSERT( GetObjectShell(), "Kein Dokument!" );

    SfxObjectFactory &rDocFact = GetObjectShell()->GetFactory();

    // find index of old and new ViewShell
    sal_uInt16 nOldNo = USHRT_MAX, nNewNo = USHRT_MAX;
    bIsIndex |= 0 == nViewId;
    for ( sal_uInt16 nNo = 0; nNo < rDocFact.GetViewFactoryCount(); ++nNo )
    {
        sal_uInt16 nFoundId = rDocFact.GetViewFactory(nNo).GetOrdinal();
        if ( nNewNo == USHRT_MAX )
        {
            if ( bIsIndex && nViewId == nNo )
            {
                nNewNo = nNo;
                nViewId = nFoundId; // for nViewId == 0
            }
            else if ( !bIsIndex && nViewId == nFoundId )
               nNewNo = nNo;
        }
        if ( pImp->nCurViewId == nFoundId )
            nOldNo = nNo;
    }

    if ( nNewNo == USHRT_MAX )
    {
        // unknown ID -> fall back to default
        sal_uInt16 nFoundId = rDocFact.GetViewFactory(0).GetOrdinal();
        nNewNo = 0;
        nViewId = nFoundId;
        if ( pImp->nCurViewId == nFoundId )
            nOldNo = 0;
    }

    SfxViewShell *pSh = GetViewShell();

    DBG_ASSERT( !pSh || nOldNo != USHRT_MAX, "old shell id not found" );

    // does a ViewShell exist already?
    SfxViewShell *pOldSh = pSh;
    if ( pOldSh )
    {
        // ask wether it can be closed
        if ( !pOldSh->PrepareClose() )
            return sal_False;

        // remove SubShells from Dispatcher before switching to new ViewShell
        pOldSh->PushSubShells_Impl( sal_False );
        sal_uInt16 nLevel = pDispatcher->GetShellLevel( *pOldSh );
        if ( nLevel )
        {
            SfxShell *pSubShell = pDispatcher->GetShell( nLevel-1 );
            if ( pSubShell == pOldSh->GetSubShell() )
                //"real" SubShells are not deleted
                pDispatcher->Pop( *pSubShell, SFX_SHELL_POP_UNTIL );
            else
                // SubShells only known to Dispatcher must be deleted
                pDispatcher->Pop( *pSubShell, SFX_SHELL_POP_UNTIL | SFX_SHELL_POP_DELETE );
        }

        pDispatcher->Pop( *pOldSh );
        GetBindings().Invalidate( nOldNo + SID_VIEWSHELL0 );
    }

    // remember ViewID
    pImp->nCurViewId = nViewId;
    GetBindings().Invalidate( nNewNo + SID_VIEWSHELL0 );

    // create new ViewShell
    SfxViewFactory &rViewFactory = rDocFact.GetViewFactory( nNewNo );
    LockAdjustPosSizePixel();

    GetBindings().ENTERREGISTRATIONS();
    pSh = rViewFactory.CreateInstance(this, pOldSh);

    Window *pEditWin = pSh->GetWindow();
    DBG_ASSERT( !pEditWin || !pEditWin->IsReallyVisible(), "don`t show your ViewShell`s Window by yourself!" );

    // by setting the ViewShell it is prevented that disposing the Controller will destroy this ViewFrame also
    GetDispatcher()->SetDisableFlags( 0 );
    SetViewShell_Impl(pSh);

    Reference < ::com::sun::star::awt::XWindow > xWindow(
        GetFrame()->GetWindow().GetComponentInterface(), UNO_QUERY );
    Reference < XFrame > xFrame( GetFrame()->GetFrameInterface() );
    if ( !pSh->GetController().is() )
        pSh->SetController( new SfxBaseController( pSh ) );
    Reference < XController > xController( pSh->GetController() );
    xFrame->setComponent( xWindow, xController );

    xController->attachFrame( xFrame );
    Reference < XModel > xModel( GetObjectShell()->GetModel() );
    if ( xModel.is() )
    {
        xController->attachModel( xModel );
        xModel->connectController( xController );
        xModel->setCurrentController( xController );
    }

    GetDispatcher()->Push( *pSh );
    if ( pSh->GetSubShell() )
        GetDispatcher()->Push( *pSh->GetSubShell() );
    pSh->PushSubShells_Impl();
    GetDispatcher()->Flush();

    if ( pImp->bRestoreView && pImp->aViewData.Len() )
    {
        // restore view data if required
        pSh->ReadUserData( pImp->aViewData, sal_True );
        pImp->bRestoreView = sal_False;
    }

    // create UI elements before size is set
    if ( SfxViewFrame::Current() == this )
        GetDispatcher()->Update_Impl( sal_True );

    // allow resize events to be processed
    UnlockAdjustPosSizePixel();

    Window* pFrameWin = &GetWindow();
    if ( pFrameWin != &GetFrame()->GetWindow() )
        pFrameWin->Show();

    if ( GetWindow().IsReallyVisible() )
        DoAdjustPosSizePixel( pSh, Point(), GetWindow().GetOutputSizePixel() );

    if ( pEditWin && pSh->IsShowView_Impl() )
        pEditWin->Show();

    GetBindings().LEAVEREGISTRATIONS();
    delete pOldSh;
    }
    catch ( com::sun::star::uno::Exception& )
    {
        // the SfxCode is not able to cope with exceptions thrown while creating views
        // the code will crash in the stack unwinding procedure, so we shouldn't let exceptions go through here
        DBG_ERROR("Exception in SwitchToViewShell_Impl - urgent issue. Please contact development!");
    }

    DBG_ASSERT( SFX_APP()->GetViewFrames_Impl().Count() == SFX_APP()->GetViewShells_Impl().Count(), "Inconsistent view arrays!" );
    return sal_True;
}
//--------------------------------------------------------------------

void SfxViewFrame::SetViewData_Impl( sal_uInt16 nViewId, const String &rViewData )
{
    pImp->bRestoreView = sal_True;
    pImp->nCurViewId = nViewId;
    pImp->aViewData = rViewData;
}

//-------------------------------------------------------------------------
String& SfxViewFrame::GetViewData_Impl()
{
    return pImp->aViewData;
}

//-------------------------------------------------------------------------
sal_Bool SfxViewFrame::IsRestoreView_Impl() const
{
    return pImp->bRestoreView;
}

//-------------------------------------------------------------------------
void SfxViewFrame::SetRestoreView_Impl( sal_Bool bOn )
{
    pImp->bRestoreView = bOn;
}

//-------------------------------------------------------------------------
sal_uInt16 SfxViewFrame::GetCurViewId() const
{
    return pImp->nCurViewId;
}

//-------------------------------------------------------------------------
void SfxViewFrame::ExecView_Impl
(
    SfxRequest& rReq        // der auszuf"uhrende <SfxRequest>
)

/*  [Beschreibung]

    Interne Methode zum Ausf"uhren der f"ur die <SfxShell> Subklasse
    SfxViewFrame in der <SVIDL> beschriebenen Slots.
*/

{
    DBG_CHKTHIS(SfxViewFrame, 0);

    // Wenn gerade die Shells ausgetauscht werden...
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

        case SID_FILLFRAME:
        {
            // Bei Mail etc. k"onnen die Frames nicht angesprochen werden
            SfxFrame *pParent = GetFrame()->GetParentFrame();
            if ( pParent && pParent->GetCurrentViewFrame()->
                    GetViewShell()->IsImplementedAsFrameset_Impl() )
                break;

            SfxViewFrame *pRet = NULL;
            SFX_REQUEST_ARG(
                rReq, pItem, SfxStringItem, SID_FILLFRAME, sal_False );
            if ( pItem )
            {
                String aName( pItem->GetValue() );
                sal_uInt16 nFrame = (sal_uInt16) aName.ToInt32();
                if ( nFrame == 0 )
                    nFrame = USHRT_MAX;

                SfxFrameIterator aIter( *GetFrame(), sal_False );
                SfxFrame *pFrame = aIter.FirstFrame();
                sal_uInt16 nActFrame = 1;
                while ( pFrame )
                {
                    SfxViewFrame *pView = pFrame->GetCurrentViewFrame();
                    if (
                        nActFrame == nFrame ||
                        (
                         pView &&
                         aName.CompareIgnoreCaseToAscii( pView->SfxShell::GetName() ) == COMPARE_EQUAL
                        )
                       )
                    {
                        pRet = pView;
                        break;
                    }

                    pFrame = aIter.NextFrame( *pFrame );
                    nActFrame++;
                }
            }

            rReq.SetReturnValue( SfxObjectItem( SID_DOCFRAME, pRet ) );
            break;
        }

        case SID_VIEWSHELL:
        {
            const SfxPoolItem *pItem = 0;
            if ( rReq.GetArgs() &&
                SFX_ITEM_SET == rReq.GetArgs()->GetItemState( SID_VIEWSHELL, sal_False, &pItem ) )
                rReq.SetReturnValue( SfxBoolItem(0, SwitchToViewShell_Impl(
                    (sal_uInt16)((const SfxUInt16Item*) pItem)->GetValue()) ));
            break;
        }

        case SID_VIEWSHELL0:
        case SID_VIEWSHELL1:
        case SID_VIEWSHELL2:
        case SID_VIEWSHELL3:
        case SID_VIEWSHELL4:
        {
            rReq.SetReturnValue( SfxBoolItem(0,
                SwitchToViewShell_Impl( rReq.GetSlot() - SID_VIEWSHELL0, sal_True ) ) );
            break;
        }

        case SID_NEWWINDOW:
        {
            // Hack. demnaechst virtuelle Funktion
            if ( !GetViewShell()->NewWindowAllowed() && !GetObjectShell()->HasName() )
                return;

            // ViewData bei FrameSets rekursiv holen
            GetFrame()->GetViewData_Impl();
            SfxMedium *pMed = GetObjectShell()->GetMedium();
            String aUserData;
            GetViewShell()->WriteUserData( aUserData, sal_True );
            if ( !GetViewShell()->NewWindowAllowed() )
            {
                SFX_REQUEST_ARG( rReq, pFrameItem, SfxUnoAnyItem, SID_FILLFRAME, sal_False );
                SfxFrame *pFrame = NULL;
                Reference < XFrame > xFrame;
                if ( pFrameItem )
                {
                    pFrameItem->GetValue() >>= xFrame;
                    pFrame = SfxTopFrame::Create( xFrame );
                }
                else
                    pFrame = SfxTopFrame::Create();

                SfxAllItemSet aSet( SFX_APP()->GetPool() );
                SFX_REQUEST_ARG( rReq, pHiddenItem, SfxBoolItem, SID_HIDDEN, sal_False );
                if ( pHiddenItem )
                    aSet.Put( *pHiddenItem );

                SFX_ITEMSET_ARG( pMed->GetItemSet(), pRefererItem, SfxStringItem, SID_REFERER, sal_False );
                SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE( "private:user" ) );
                if ( !pRefererItem )
                    pRefererItem = &aReferer;

                aSet.Put( SfxStringItem( SID_FILE_NAME, pMed->GetName() ) );
                aSet.Put( SfxStringItem( SID_USER_DATA, aUserData ) );
                aSet.Put( SfxUInt16Item( SID_VIEW_ID, GetCurViewId() ) );
                aSet.Put( *pRefererItem );
                if( pMed->GetFilter() )
                    aSet.Put( SfxStringItem( SID_FILTER_NAME, pMed->GetFilter()->GetFilterName()) );
                aSet.Put( SfxFrameItem ( SID_DOCFRAME, pFrame ) );
                if ( xFrame.is() )
                    GetDispatcher()->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, aSet );
                else
                    GetDispatcher()->Execute( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, aSet );
            }
            else
            {
                pMed->GetItemSet()->Put( SfxStringItem( SID_USER_DATA, aUserData ) );

                BOOL bHidden = FALSE;
                SFX_REQUEST_ARG( rReq, pHiddenItem, SfxBoolItem, SID_HIDDEN, sal_False );
                if ( pHiddenItem )
                    bHidden = pHiddenItem->GetValue();
                SFX_REQUEST_ARG( rReq, pFrameItem, SfxUnoAnyItem, SID_FILLFRAME, sal_False );
                if ( pFrameItem )
                {
                    Reference < XFrame > xFrame;
                    pFrameItem->GetValue() >>= xFrame;
                    SfxFrame* pFrame = SfxTopFrame::Create( xFrame );
                    pMed->GetItemSet()->ClearItem( SID_HIDDEN );
                    pFrame->InsertDocument( GetObjectShell() );
                    if ( !bHidden )
                        xFrame->getContainerWindow()->setVisible( sal_True );
                }
                else
                {
                    SfxAllItemSet aSet( GetPool() );
                    aSet.Put( SfxBoolItem( SID_OPEN_NEW_VIEW, TRUE ) );
                    SfxFrame* pFrame = SfxTopFrame::Create( GetObjectShell(), GetCurViewId(), bHidden, &aSet );
                    if ( bHidden )
                        pFrame->GetCurrentViewFrame()->LockObjectShell_Impl( TRUE );
                }
            }

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
        This method try to collect informations about the count of currently open documents.
        But the algorithm is implemented very simple ...
        E.g. hidden documents should be ignored here ... but they are counted.
        TODO: export special helper "framework::FrameListAnalyzer" within the framework module
        and use it here.
*/
sal_Bool impl_maxOpenDocCountReached()
{
    static ::rtl::OUString SERVICE_DESKTOP = ::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop");

    try
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
        css::uno::Any aVal = ::comphelper::ConfigurationHelper::readDirectKey(
                                xSMGR,
                                ::rtl::OUString::createFromAscii("org.openoffice.Office.Common/"),
                                ::rtl::OUString::createFromAscii("Misc"),
                                ::rtl::OUString::createFromAscii("MaxOpenDocuments"),
                                ::comphelper::ConfigurationHelper::E_READONLY);

        // NIL means: count of allowed documents = infinite !
        if ( ! aVal.hasValue())
            return sal_False;

        sal_Int32 nOpenDocs = 0;
        sal_Int32 nMaxDocs  = 0;
        aVal >>= nMaxDocs;

        css::uno::Reference< css::frame::XFramesSupplier >  xDesktop(xSMGR->createInstance(SERVICE_DESKTOP), css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XIndexAccess > xCont   (xDesktop->getFrames()                 , css::uno::UNO_QUERY_THROW);

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
                if (xFrame->getName().equalsAscii("OFFICE_HELP_TASK"))
                    continue;

                // b) count all other frames
                ++nOpenDocs;
            }
            catch(const css::uno::Exception&)
                // A IndexOutOfBoundException can happen in multithreaded environments,
                // where any other thread can change this container !
                { continue; }
        }

        return (nOpenDocs >= nMaxDocs);
    }
    catch(const css::uno::Exception&)
        {}

    // Any internal error is no reason to stop opening documents !
    // Limitation of opening documents is a special "nice to  have" feature.
    // Otherwhise it can happen, that NO document will be opened ...
    return sal_False;
}

//-------------------------------------------------------------------------
void SfxViewFrame::StateView_Impl
(
    SfxItemSet&     rSet            /*  leeres <SfxItemSet> mit <Which-Ranges>,
                                        welche die Ids der zu erfragenden
                                        Slots beschreiben. */
)

/*  [Beschreibung]

    Diese interne Methode liefert in 'rSet' die Status der f"ur die
    <SfxShell> Subklasse SfxViewFrame in der <SVIDL> beschriebenen <Slots>.

    In 'rSet' sind dabei genau die vom SFx als ung"ultig erkannten
    Slot-Ids als Which-ranges enthalten. Falls der an dieser Shell gesetzte
    <SfxItemPool> f"ur einzelne Slot-Ids ein Mapping hat, werden die
    entsprechenden Which-Ids verwendet, so da\s Items ggf. direkt mit
    einer mit Which-Ids arbeitenden Core-::com::sun::star::script::Engine ausgetauscht werden
    k"onnen.
*/

{
    DBG_CHKTHIS(SfxViewFrame, 0);

    SfxObjectShell *pDocSh = GetObjectShell();

    if ( !pDocSh )
        // Ich bin gerade am Reloaden und Yielde so vor mich hin ...
        return;

    const sal_uInt16 *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges, "Set ohne Bereich");
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
                case SID_FRAMETITLE:
                {
                    if( GetFrameType() & SFXFRAME_HASTITLE )
                        rSet.Put( SfxStringItem(
                            SID_FRAMETITLE, pImp->aFrameTitle) );
                    else
                        rSet.DisableItem( nWhich );
                    break;
                }

                case SID_NEWWINDOW:
                {
                    if ( !GetViewShell()->NewWindowAllowed() /* && !pDocSh->HasName() */ )
                            rSet.DisableItem( nWhich );
                    else
                    {
                        if (impl_maxOpenDocCountReached())
                            rSet.DisableItem( nWhich );
                    }
                    break;
                }
            }
        }
    }
}

//-------------------------------------------------------------------------
void SfxViewFrame::GetState_Impl( SfxItemSet &rSet )
{
    if ( GetViewShell() && GetViewShell()->GetVerbs().getLength() && !GetObjectShell()->IsInPlaceActive() )
    {
        uno::Any aAny;
        aAny <<= GetViewShell()->GetVerbs();
        rSet.Put( SfxUnoAnyItem( USHORT( SID_OBJECT ), aAny ) );
    }
    else
        rSet.DisableItem( SID_OBJECT );
}

//-------------------------------------------------------------------------
void SfxViewFrame::ToTop()
{
    GetFrame()->Appear();
}

//-------------------------------------------------------------------------
SfxViewFrame* SfxViewFrame::GetParentViewFrame() const
/*
    Beschreibung:
    Der ParentViewFrame ist der ViewFrame des ParentFrames
*/
{
    SfxFrame *pFrame = GetFrame()->GetParentFrame();
    return pFrame ? pFrame->GetCurrentViewFrame() : NULL;
}

//-------------------------------------------------------------------------
SfxFrame* SfxViewFrame::GetFrame() const
/*
    Beschreibung:
    GetFrame liefert den Frame, in dem sich der ViewFrame befindet
*/
{
    return pImp->pFrame;
}

//-------------------------------------------------------------------------
void SfxViewFrame::SetFrame_Impl( SfxFrame *pFrame )
{
    pImp->pFrame = pFrame;
}

//-------------------------------------------------------------------------
SfxViewFrame* SfxViewFrame::GetTopViewFrame() const
{
    return GetFrame()->GetTopFrame()->GetCurrentViewFrame();
}

//-------------------------------------------------------------------------

SfxCancelManager *SfxViewFrame::GetCancelManager() const

/*  <H3>Description</H3>

    Returns a pointer to the <SfxCancelManager> of the top-view-frame of
    this view-frame.
*/

{
    return GetTopViewFrame()->GetFrame()->GetCancelManager();
}

void SfxViewFrame::SetWindow_Impl( Window *pWin )
{
    pImp->pWindow = pWin;
}

Window& SfxViewFrame::GetWindow() const
{
    return pImp->pWindow ? *pImp->pWindow : GetFrame()->GetWindow();
}

sal_Bool SfxViewFrame::DoClose()
{
    return GetFrame()->DoClose();
}

String SfxViewFrame::GetActualPresentationURL_Impl() const
{
    if ( xObjSh.Is() )
        return xObjSh->GetMedium()->GetName();
    return pImp->aActualPresentationURL;
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

BOOL SfxViewFrame::IsInModalMode() const
{
    return pImp->bModal || GetFrame()->GetWindow().IsInModalMode();
}

void SfxViewFrame::Resize( BOOL bForce )
{
    Size aSize = GetWindow().GetOutputSizePixel();
    if ( bForce || aSize != pImp->aSize )
    {
        pImp->aSize = aSize;
        SfxViewShell *pShell = GetViewShell();
        if ( pShell )
        {
            if ( GetFrame()->IsInPlace() )
            {
                Point aPoint = GetWindow().GetPosPixel();
                DoAdjustPosSizePixel( pShell, aPoint, aSize );
            }
            else
            {
                if ( pShell->UseObjectSize() )
                    ForceOuterResize_Impl(TRUE);
                DoAdjustPosSizePixel( pShell, Point(), aSize );
                if ( pShell->UseObjectSize() )
                    ForceOuterResize_Impl(FALSE);
            }
        }
    }
}

#define LINE_SEP 0x0A

void CutLines( ::rtl::OUString& rStr, sal_Int32 nStartLine, sal_Int32 nLines, BOOL bEraseTrailingEmptyLines )
{
    sal_Int32 nStartPos = 0;
    sal_Int32 nEndPos = 0;
    sal_Int32 nLine = 0;
    while ( nLine < nStartLine )
    {
        nStartPos = rStr.indexOf( LINE_SEP, nStartPos );
        if( nStartPos == -1 )
            break;
        nStartPos++;    // nicht das \n.
        nLine++;
    }

    DBG_ASSERTWARNING( nStartPos != STRING_NOTFOUND, "CutLines: Startzeile nicht gefunden!" );

    if ( nStartPos != -1 )
    {
        nEndPos = nStartPos;
        for ( sal_Int32 i = 0; i < nLines; i++ )
            nEndPos = rStr.indexOf( LINE_SEP, nEndPos+1 );

        if ( nEndPos == -1 ) // kann bei letzter Zeile passieren
            nEndPos = rStr.getLength();
        else
            nEndPos++;

        ::rtl::OUString aEndStr = rStr.copy( nEndPos );
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
            ::rtl::OUString aEndStr = rStr.copy( n );
            rStr = rStr.copy( 0, nStartPos );
            rStr += aEndStr;
        }
    }
}

/*
    add new recorded dispatch macro script into the application global basic lib container
    It generates a new unique id for it and insert the macro by using this number as name for
    the modul
 */
void SfxViewFrame::AddDispatchMacroToBasic_Impl( const ::rtl::OUString& sMacro )
{
    /*
    // get lib and modul name from dialog
    SfxModule *pMod = GetObjectShell()->GetModule();
    SfxRequest aReq( SID_BASICCHOOSER, SFX_CALLMODE_SYNCHRON, pMod->GetPool() );
    const SfxPoolItem* pRet = pMod->ExecuteSlot( aReq );
    if ( pRet )
        ::rtl::OUString = ((SfxStringItem*)pRet)->GetValue();
    */
    if ( !sMacro.getLength() )
        return;

    SfxApplication* pSfxApp = SFX_APP();
    SfxRequest aReq( SID_BASICCHOOSER, SFX_CALLMODE_SYNCHRON, pSfxApp->GetPool() );
    aReq.AppendItem( SfxBoolItem(SID_RECORDMACRO,TRUE) );
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
        Reference< XMultiServiceFactory > xSMgr = ::comphelper::getProcessServiceFactory();
        Reference< com::sun::star::uri::XUriReferenceFactory > xFactory( xSMgr->createInstance(
            ::rtl::OUString::createFromAscii( "com.sun.star.uri.UriReferenceFactory" ) ), UNO_QUERY );
        if ( xFactory.is() )
        {
            Reference< com::sun::star::uri::XVndSunStarScriptUrl > xUrl( xFactory->parse( aScriptURL ), UNO_QUERY );
            if ( xUrl.is() )
            {
                // get name
                ::rtl::OUString aName = xUrl->getName();
                sal_Unicode cTok = '.';
                sal_Int32 nIndex = 0;
                aLibName = aName.getToken( 0, cTok, nIndex );
                if ( nIndex != -1 )
                    aModuleName = aName.getToken( 0, cTok, nIndex );
                if ( nIndex != -1 )
                    aMacroName = aName.getToken( 0, cTok, nIndex );

                // get location
                ::rtl::OUString aLocKey = ::rtl::OUString::createFromAscii( "location" );
                if ( xUrl->hasParameter( aLocKey ) )
                    aLocation = xUrl->getParameter( aLocKey );
            }
        }

        pSfxApp->EnterBasicCall();

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

        ::rtl::OUString aOUSource;
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
                    USHORT nStart, nEnd;
                    pMethod->GetLineRange( nStart, nEnd );
                    ULONG nlStart = nStart;
                    ULONG nlEnd = nEnd;
                    CutLines( aOUSource, nlStart-1, nlEnd-nlStart+1, TRUE );
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
            DBG_ERRORFILE("couldn't get access to the basic lib container. Adding of macro isn't possible.");
            return;
        }

        // get LibraryContainer
        com::sun::star::uno::Any aTemp;
        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xRoot(
                xLibCont,
                com::sun::star::uno::UNO_QUERY);

        ::rtl::OUString sLib( aLibName );
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
        ::rtl::OUString sCode;
        ::rtl::OUStringBuffer sRoutine(10000);
        ::rtl::OUString sMacroName( aMacroName );
        BOOL bReplace = FALSE;

        // get module
        ::rtl::OUString sModule( aModuleName );
        if(xLib->hasByName(sModule))
        {
            if ( aOUSource.getLength() )
            {
                sRoutine.append( aOUSource );
            }
            else
            {
                aTemp = xLib->getByName(sModule);
                aTemp >>= sCode;
                sRoutine.append( sCode );
            }

            bReplace = TRUE;
        }

        // append new method
        sRoutine.appendAscii("\nsub "     );
        sRoutine.append     (sMacroName   );
        sRoutine.appendAscii("\n"         );
        sRoutine.append     (sMacro       );
        sRoutine.appendAscii("\nend sub\n");

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

        pSfxApp->LeaveBasicCall();
    }
    else
    {
        // add code for "session only" macro
    }

    /*
    FILE* pFile = fopen( "macro.bas", "a" );
    fprintf( pFile, "%s", ::rtl::OUStringToOString(sBuffer.makeStringAndClear(),RTL_TEXTENCODING_UTF8).getStr() );
    fclose ( pFile );
    */
}

void SfxViewFrame::MiscExec_Impl( SfxRequest& rReq )
{
    DBG_MEMTEST();
    FASTBOOL bDone = FALSE;
    switch ( rReq.GetSlot() )
    {
        case SID_STOP_RECORDING :
        case SID_RECORDMACRO :
        {
            // try to find any active recorder on this frame
            ::rtl::OUString sProperty = rtl::OUString::createFromAscii("DispatchRecorderSupplier");
            com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xFrame(
                    GetFrame()->GetFrameInterface(),
                    com::sun::star::uno::UNO_QUERY);

            com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xSet(xFrame,com::sun::star::uno::UNO_QUERY);
            com::sun::star::uno::Any aProp = xSet->getPropertyValue(sProperty);
            com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorderSupplier > xSupplier;
            aProp >>= xSupplier;
            com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder;
            if (xSupplier.is())
                xRecorder = xSupplier->getDispatchRecorder();

            BOOL bIsRecording = xRecorder.is();
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

                SetChildWindow( SID_RECORDING_FLOATWINDOW, FALSE );
                if ( rReq.GetSlot() != SID_RECORDMACRO )
                    GetBindings().Invalidate( SID_RECORDMACRO );
            }
            else if ( rReq.GetSlot() == SID_RECORDMACRO )
            {
                // enable recording
                com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xFactory(
                        ::comphelper::getProcessServiceFactory(),
                        com::sun::star::uno::UNO_QUERY);

                xRecorder = com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder >(
                        xFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.frame.DispatchRecorder")),
                        com::sun::star::uno::UNO_QUERY);

                xSupplier = com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorderSupplier >(
                        xFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.frame.DispatchRecorderSupplier")),
                        com::sun::star::uno::UNO_QUERY);

                xSupplier->setDispatchRecorder(xRecorder);
                xRecorder->startRecording(xFrame);
                aProp <<= xSupplier;
                xSet->setPropertyValue(sProperty,aProp);
                GetBindings().SetRecorder_Impl( xRecorder );
                SetChildWindow( SID_RECORDING_FLOATWINDOW, TRUE );
            }

            rReq.Done();
            break;
        }

        case SID_TOGGLESTATUSBAR:
        {
            com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xFrame(
                    GetFrame()->GetFrameInterface(),
                    com::sun::star::uno::UNO_QUERY);

            Reference< com::sun::star::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
            Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
            if ( xPropSet.is() )
            {
                try
                {
                    Any aValue = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
                    aValue >>= xLayoutManager;
                }
                catch ( Exception& )
                {
                }
            }

            if ( xLayoutManager.is() )
            {
                rtl::OUString aStatusbarResString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/statusbar/statusbar" ));
                // Parameter auswerten
                SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, rReq.GetSlot(), FALSE);
                BOOL bShow( TRUE );
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
            SFX_REQUEST_ARG(rReq, pItem, SfxBoolItem, rReq.GetSlot(), FALSE);
            SfxTopViewFrame *pTop= PTR_CAST( SfxTopViewFrame, GetTopViewFrame() );
            if ( pTop )
            {
                WorkWindow* pWork = (WorkWindow*) pTop->GetTopFrame_Impl()->GetTopWindow_Impl();
                if ( pWork )
                {
                    com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xFrame(
                            GetFrame()->GetFrameInterface(),
                            com::sun::star::uno::UNO_QUERY);

                    Reference< ::com::sun::star::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
                    Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
                    if ( xPropSet.is() )
                    {
                        try
                        {
                            Any aValue = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
                            aValue >>= xLayoutManager;
                        }
                        catch ( Exception& )
                        {
                        }
                    }

                    BOOL bNewFullScreenMode = pItem ? pItem->GetValue() : !pWork->IsFullScreenMode();
                    if ( bNewFullScreenMode != pWork->IsFullScreenMode() )
                    {
                        Reference< ::com::sun::star::beans::XPropertySet > xLMPropSet( xLayoutManager, UNO_QUERY );
                        if ( xLMPropSet.is() )
                        {
                            try
                            {
                                xLMPropSet->setPropertyValue(
                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HideCurrentUI" )),
                                    makeAny( bNewFullScreenMode ));
                            }
                            catch ( ::com::sun::star::beans::UnknownPropertyException& )
                            {
                            }
                        }
                        pWork->ShowFullScreenMode( bNewFullScreenMode );
                        pWork->SetMenuBarMode( bNewFullScreenMode ? MENUBAR_MODE_HIDE : MENUBAR_MODE_NORMAL );
                        GetFrame()->GetWorkWindow_Impl()->SetFullScreen_Impl( bNewFullScreenMode );
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

            GetDispatcher()->Update_Impl( TRUE );
            break;
        }
    }

    if ( bDone )
        rReq.Done();
}

void SfxViewFrame::MiscState_Impl(SfxItemSet &rSet)
{
    DBG_MEMTEST();

    const USHORT *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges && *pRanges, "Set ohne Bereich");
    while ( *pRanges )
    {
        for(USHORT nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich)
        {
            switch(nWhich)
            {
                case SID_CURRENT_URL:
                {
                    // Bei internem InPlace den ContainerFrame nehmen
                    SfxViewFrame *pFrame = this;
                    if ( pFrame->GetParentViewFrame_Impl() )
                        pFrame = pFrame->GetParentViewFrame_Impl();
                    rSet.Put( SfxStringItem( nWhich, pFrame->GetActualPresentationURL_Impl() ) );
                    break;
                }

                case SID_RECORDMACRO :
                {
                    const char* pName = GetObjectShell()->GetFactory().GetShortName();
                    if (  strcmp(pName,"swriter") && strcmp(pName,"scalc") )
                    {
                        rSet.DisableItem( nWhich );
                        break;
                    }

                    ::rtl::OUString sProperty = rtl::OUString::createFromAscii("DispatchRecorderSupplier");
                    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xSet(
                            GetFrame()->GetFrameInterface(),
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
                    const char* pName = GetObjectShell()->GetFactory().GetShortName();
                    if (  strcmp(pName,"swriter") && strcmp(pName,"scalc") )
                    {
                        rSet.DisableItem( nWhich );
                        break;
                    }

                    ::rtl::OUString sProperty = rtl::OUString::createFromAscii("DispatchRecorderSupplier");
                    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xSet(
                            GetFrame()->GetFrameInterface(),
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
                            GetFrame()->GetFrameInterface(),
                            com::sun::star::uno::UNO_QUERY);
                    com::sun::star::uno::Any aProp = xSet->getPropertyValue(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )) );

                    if ( !( aProp >>= xLayoutManager ))
                        rSet.Put( SfxBoolItem( nWhich, FALSE ));
                    else
                    {
                        rtl::OUString aStatusbarResString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/statusbar/statusbar" ));
                        BOOL bShow = xLayoutManager->isElementVisible( aStatusbarResString );
                        rSet.Put( SfxBoolItem( nWhich, bShow ));
                    }
                    break;
                }

                case SID_WIN_FULLSCREEN:
                {
                    SfxTopViewFrame *pTop= PTR_CAST( SfxTopViewFrame, GetTopViewFrame() );
                    if ( pTop )
                    {
                        WorkWindow* pWork = (WorkWindow*) pTop->GetTopFrame_Impl()->GetTopWindow_Impl();
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
                    DBG_ERROR("Outdated slot!");
                    rSet.DisableItem( nWhich );
                    break;
                }

                default:
                    //! DBG_ASSERT(FALSE, "Falscher Server fuer GetState");
                    break;
            }
        }

        ++pRanges;
    }
}

void SfxViewFrame::ChildWindowExecute( SfxRequest &rReq )

/*  [Beschreibung]

    Diese Methode kann in der Execute-Methode f"ur das ein- und ausschalten
    von Child-Windows eingesetzt werden, um dieses inkl. API-Anbindung zu
    implementieren.

    Einfach in der IDL als 'ExecuteMethod' eintragen.
*/

{
    // Parameter auswerten
    USHORT nSID = rReq.GetSlot();

    SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, nSID, FALSE);
    if ( nSID == SID_VIEW_DATA_SOURCE_BROWSER )
    {
        if (!SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SDATABASE))
            return;
        Reference < XFrame > xFrame = GetFrame()->GetTopFrame()->GetFrameInterface();
        Reference < XFrame > xBeamer( xFrame->findFrame( DEFINE_CONST_UNICODE("_beamer"), FrameSearchFlag::CHILDREN ) );
        BOOL bShow = FALSE;
        BOOL bHasChild = xBeamer.is();
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
            SetChildWindow( SID_BROWSER, FALSE );
        }
        else
        {
            ::com::sun::star::util::URL aTargetURL;
            aTargetURL.Complete = ::rtl::OUString::createFromAscii(".component:DB/DataSourceBrowser");
            Reference < ::com::sun::star::util::XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
            xTrans->parseStrict( aTargetURL );

            Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
            Reference < ::com::sun::star::frame::XDispatch > xDisp;
            if ( xProv.is() )
                xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString::createFromAscii("_beamer"), 31 );
            if ( xDisp.is() )
            {
                Sequence < ::com::sun::star::beans::PropertyValue > aArgs(1);
                ::com::sun::star::beans::PropertyValue* pArg = aArgs.getArray();
                pArg[0].Name = rtl::OUString::createFromAscii("Referer");
                pArg[0].Value <<= ::rtl::OUString::createFromAscii("private:user");
                xDisp->dispatch( aTargetURL, aArgs );
            }
        }

        rReq.Done();
        return;
    }

    BOOL bShow = FALSE;
    BOOL bHasChild = HasChildWindow(nSID);
    bShow = pShowItem ? pShowItem->GetValue() : !bHasChild;

    // ausf"uhren
    if ( !pShowItem || bShow != bHasChild )
        ToggleChildWindow( nSID );

    GetBindings().Invalidate( nSID );
    GetDispatcher()->Update_Impl( TRUE );

    // ggf. recorden
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

/*  [Beschreibung]

    Diese Methode kann in der Status-Methode f"ur das Ein- und Ausschalt-
    Zustand von Child-Windows eingesetzt werden, um dieses zu implementieren.

    Einfach in der IDL als 'StateMethod' eintragen.
*/

{
    SfxWhichIter aIter( rState );
    for ( USHORT nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
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
            Reference < XFrame > xFrame = GetFrame()->GetTopFrame()->GetFrameInterface()->
                            findFrame( DEFINE_CONST_UNICODE("_beamer"), FrameSearchFlag::CHILDREN );
            if ( !xFrame.is() )
                rState.DisableItem( nSID );
            else if ( KnowsChildWindow(nSID) )
                rState.Put( SfxBoolItem( nSID, HasChildWindow(nSID) ) );
        }
        else if ( KnowsChildWindow(nSID) )
            rState.Put( SfxBoolItem( nSID, HasChildWindow(nSID) ) );
        else
            rState.DisableItem(nSID);
    }
}

//--------------------------------------------------------------------
SfxWorkWindow* SfxViewFrame::GetWorkWindow_Impl( USHORT /*nId*/ )
{
    SfxWorkWindow* pWork = 0;
    pWork = GetFrame()->GetWorkWindow_Impl();
    return pWork;
}

/*
void SfxViewFrame::SetChildWindow(USHORT nId, BOOL bOn)
{
    SetChildWindow( nId, bOn, TRUE );
}*/

void SfxViewFrame::SetChildWindow(USHORT nId, BOOL bOn, BOOL bSetFocus )
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    if ( pWork )
        pWork->SetChildWindow_Impl( nId, bOn, bSetFocus );
}

//--------------------------------------------------------------------

void SfxViewFrame::ToggleChildWindow(USHORT nId)
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    if ( pWork )
        pWork->ToggleChildWindow_Impl( nId, TRUE );
}

//--------------------------------------------------------------------

BOOL SfxViewFrame::HasChildWindow( USHORT nId )
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    return pWork ? pWork->HasChildWindow_Impl(nId) : FALSE;
}

//--------------------------------------------------------------------

BOOL SfxViewFrame::KnowsChildWindow( USHORT nId )
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    return pWork ? pWork->KnowsChildWindow_Impl(nId) : FALSE;
}

//--------------------------------------------------------------------

void SfxViewFrame::ShowChildWindow( USHORT nId, BOOL bVisible )
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    if ( pWork )
    {
        GetDispatcher()->Update_Impl(sal_True);
        pWork->ShowChildWindow_Impl(nId, bVisible, TRUE );
    }
}

//--------------------------------------------------------------------

SfxChildWindow* SfxViewFrame::GetChildWindow(USHORT nId)
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    return pWork ? pWork->GetChildWindow_Impl(nId) : NULL;
}

SfxMacro* SfxViewFrame::GetRecordingMacro_Impl()
{
    return pImp->pMacro;
}

void SfxViewFrame::UpdateDocument_Impl()
{
    SfxObjectShell* pDoc = GetObjectShell();
    if ( pDoc->IsLoadingFinished() )
        pDoc->CheckSecurityOnLoading_Impl();

    // check if document depends on a template
    pDoc->UpdateFromTemplate_Impl();
}

BOOL SfxViewFrame::ClearEventFlag_Impl()
{
    if ( pImp->bEventFlag )
    {
        pImp->bEventFlag = FALSE;
        return TRUE;
    }
    else
        return FALSE;
}

SfxViewFrame* SfxViewFrame::CreateViewFrame( SfxObjectShell& rDoc, sal_uInt16 nViewId, sal_Bool bHidden )
{
    SfxItemSet *pSet = rDoc.GetMedium()->GetItemSet();
    if ( nViewId )
        pSet->Put( SfxUInt16Item( SID_VIEW_ID, nViewId ) );
    if ( bHidden )
        pSet->Put( SfxBoolItem( SID_HIDDEN, sal_True ) );

    SfxFrame *pFrame = SfxTopFrame::Create( &rDoc, 0, bHidden );
    return pFrame->GetCurrentViewFrame();
}

void SfxViewFrame::SetViewFrame( SfxViewFrame* pFrame )
{
    SFX_APP()->SetViewFrame_Impl( pFrame );
}
