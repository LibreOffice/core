/*************************************************************************
 *
 *  $RCSfile: viewfrm.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: mba $ $Date: 2000-10-23 12:04:49 $
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

#include "viewfrm.hxx"

#ifndef _IPENV_HXX //autogen
#include <so3/ipenv.hxx>
#endif
#ifndef _SPLITWIN_HXX //autogen
#include <vcl/splitwin.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSLSTITM_HXX //autogen
#include <svtools/slstitm.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _UNDO_HXX //autogen
#include <svtools/undo.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif
#ifndef _XCEPTION_HXX_
#include <vos/xception.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::frame;

#pragma hdrstop

#include "picklist.hxx"
#include "openflag.hxx"
#include "objshimp.hxx"
#include "viewsh.hxx"
#include "objsh.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "loadenv.hxx"
#include "arrdecl.hxx"
#include "sfxtypes.hxx"
#include "newhdl.hxx"
#include "request.hxx"
#include "docfac.hxx"
#include "ipfrm.hxx"
#include "sfxresid.hxx"
#include "cfgmgr.hxx"
#include "appbas.hxx"
#include "objitem.hxx"
#include "viewfac.hxx"
#include "stbmgr.hxx"
#include "event.hxx"
#include "fltfnc.hxx"
#include "fsetvwsh.hxx"
#include "fsetobsh.hxx"
#include "docfile.hxx"
#include "interno.hxx"
#include "module.hxx"
#include "msgpool.hxx"
#include "topfrm.hxx"
#include "urlframe.hxx"
#include "fsetvwsh.hxx"
#include "viewimp.hxx"
#include "sfxbasecontroller.hxx"
#include "sfx.hrc"
#include "view.hrc"
#include <intfrm.hxx>
#include <frmdescr.hxx>
#include "appdata.hxx"
#include "sfxuno.hxx"
#include "ucbhelp.hxx"
#include "progress.hxx"
#include "workwin.hxx"
#include "helper.hxx"
#include "tbxconf.hxx"

//-------------------------------------------------------------------------
DBG_NAME(SfxViewFrame);

#define SfxViewFrame
#include "sfxslots.hxx"

//-------------------------------------------------------------------------

SFX_IMPL_INTERFACE(SfxViewFrame,SfxShell,SfxResId(0))
{
    SFX_CHILDWINDOW_REGISTRATION( SID_PARTWIN );
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
    AsynchronLink*      pReloader;
    SfxInPlaceFrame*    pIPFrame;
    Window*             pWindow;
    SfxViewFrame*       pActiveChild;
    SfxViewFrame*       pParentViewFrame;
    SfxObjectShell*     pImportShell;
    Window*             pFocusWin;
    sal_uInt16          nDocViewNo;
    sal_uInt16          nCurViewId;
    sal_Bool            bResizeInToOut:1;
    sal_Bool            bObjLocked:1;
    sal_Bool            bRestoreView:1;
    sal_Bool            bSetViewFrameLocked:1;
    sal_Bool            bReloading:1;
    sal_Bool            bIsDowning:1;
    sal_Bool            bInCtor:1;
    sal_Bool            bModal:1;
    sal_Bool            bEnabled:1;

                        SfxViewFrame_Impl()
                        : pReloader(0 )
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
            pImp->pReloader = new AsynchronLink(
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
        if ( pParent )
        {
            SfxViewShell *pShell = pParent->GetCurrentViewFrame()->GetViewShell();
            if( pShell->IsImplementedAsFrameset_Impl() &&
                pShell->GetInterface()->GetSlot( rReq.GetSlot() ) )
            {
                // Hack f"ur Explorer: Reload wird an der ViewShell ausgef"uhrt
                pShell->ExecuteSlot( rReq );
                return;
            }
        }

        SFX_REQUEST_ARG(rReq, pBoolItem, SfxBoolItem, SID_RELOAD, sal_False);
        if ( pBoolItem && pBoolItem->GetValue() &&
            pParent && !(rReq.GetModifier() & KEY_MOD1) )
        {
            // Reload "uber UI geht immer "uber TopFrame
            GetTopViewFrame()->ExecReload_Impl( rReq );
            return;
        }

        // Components m"ussen das Reload selbst implementieren
        if ( GetFrame()->HasComponent() )
            return;

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
            if( !pSh || !pSh->HasName() ||
                !(pSh->Get_Impl()->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ))
                break;

            SFX_ITEMSET_ARG(
                pSh->GetMedium()->GetItemSet(), pItem, SfxBoolItem,
                SID_EDITDOC, sal_False );
            if ( pItem && !pItem->GetValue() )
                   break;

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
                nOpenMode = SFX_STREAM_READWRITE;

            // Parameter auswerten
            sal_Bool bReload = sal_True;
            if ( rReq.IsAPI() )
            {
                // per API steuern ob r/w oder r/o
                SFX_REQUEST_ARG(rReq, pEditItem, SfxBoolItem, SID_EDITDOC, sal_False);
                if ( pEditItem )
                    nOpenMode = pEditItem->GetValue() ? SFX_STREAM_READWRITE : SFX_STREAM_READONLY;
            }
            else
            {
                // sonst Reaload abschaltbar
                HACK(SID_EDITDOC ist hier falsch verwendet)
                SFX_REQUEST_ARG(rReq, pReloadItem, SfxBoolItem, SID_EDITDOC, sal_False);
                if ( pReloadItem )
                    bReload = pReloadItem->GetValue();
            }

            // doing
            if( pSh  )
            {
                SfxMedium* pMed = pSh->GetMedium();
                INetURLObject aPhysObj( pMed->GetPhysicalName(), INET_PROT_FILE );
                SFX_ITEMSET_ARG( pSh->GetMedium()->GetItemSet(),
                                 pVersionItem, SfxInt16Item, SID_VERSION, sal_False );

                if ( ( !bNeedsReload &&
                    ( pMed->GetURLObject().GetProtocol() == INET_PROT_FILE
                              && INetURLObject( pMed->GetName(), INET_PROT_FILE ) == aPhysObj
                            && !SfxContentHelper::IsYounger( aPhysObj.GetMainURL(),
                                                             pMed->GetURLObject().GetMainURL() )
                      || pMed->IsRemote()
                    )
                   ) || pVersionItem )
                {
                    sal_Bool bOK = sal_False;
                    if ( !pVersionItem )
                    {
                        // Umschalten ohne Reload ist moeglich
                        pSh->DoHandsOff();
                        pMed->Close();
                        pMed->GetItemSet()->ClearItem( SID_DOC_READONLY );
                        pMed->SetOpenMode( nOpenMode, pMed->IsDirect() );
                        if ( nOpenMode == SFX_STREAM_READONLY )
                            pMed->CheckOpenMode_Impl(sal_False,sal_True);
                        pMed->ReOpen();
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
                                SFX_ITEMSET_ARG( pMed->GetItemSet(), pReferer,
                                                 SfxStringItem, SID_REFERER, sal_False );
                                if ( pReferer )
                                    aSet.Put( *pReferer );
                                aSet.Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
                                if ( pVersionItem )
                                    aSet.Put( *pVersionItem );

                                if( pMed->GetFilter() )
                                {
                                    aSet.Put( SfxStringItem( SID_FILTER_NAME,
                                                             pMed->GetFilter()->GetName() ) );
                                    SFX_ITEMSET_ARG( pMed->GetItemSet(), pOptions,
                                                     SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False );
                                    if ( pOptions )
                                        aSet.Put( *pOptions );
                                }

                                //MI: im selben Frame => er macht gar nix !?!
                                //SfxFrameItem aFrameItem( SID_DOCFRAME, GetFrame() );
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
                        if( nOpenMode == SFX_STREAM_READONLY )
                            pMed->Close();

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

                if ( !bReload )
                {
                    // Es soll nicht reloaded werden
                    SfxErrorContext aEc( ERRCODE_SFX_NODOCRELOAD );
                    ErrorHandler::HandleError( ERRCODE_SFX_NODOCRELOAD );
                    rReq.SetReturnValue(
                        SfxBoolItem( rReq.GetSlot(), sal_False ) );
                    return;
                }

                // Ansonsten ( lokal und arbeiten auf Kopie ) muss gereloaded
                // werden.
            }
            SfxItemSet* pSet = pSh->GetMedium()->GetItemSet();
            pSet->Put( SfxBoolItem(
                SID_DOC_READONLY, nOpenMode != SFX_STREAM_READWRITE ) );
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
            sal_uInt32 nErr = 0;
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

            // ggf. beim ::com::sun::star::sdbcx::User nachfragen
            sal_Bool bDo = sal_True;
            SFX_REQUEST_ARG(rReq, pSilentItem, SfxBoolItem,
                            SID_SILENT, sal_False);
            if ( GetFrame()->DocIsModified_Impl() &&
                 !rReq.IsAPI() && ( !pSilentItem || !pSilentItem->GetValue() ) )
            {
                QueryBox aBox( &GetWindow(), ResId(MSG_QUERY_LASTVERSION) );
                bDo = ( RET_YES == aBox.Execute() );
            }

            if ( bDo )
            {
                SfxMedium *pMedium = xOldObj->GetMedium();

                // Frameset abziehen, bevor FramesetView evtl. verschwindet
                SFX_REQUEST_ARG(rReq, pNoCacheItem, SfxBoolItem, SID_NOCACHE, sal_False);
                String aURL = pURLItem ? pURLItem->GetValue() :
                                pMedium->GetName();

                sal_uInt16 nModifier = rReq.GetModifier();
                SfxFrameDescriptor* pDesc = NULL;
                if ( !( nModifier & KEY_SHIFT ) && !pURLItem &&
                    GetFrame()->GetDescriptor()->GetFrameSet() )
                        pDesc = GetFrame()->GetDescriptor()->Clone();

                sal_Bool bHandsOff =
                    pMedium->GetURLObject().GetProtocol() == INET_PROT_FILE;

                // Files schliessen, damit wir Reloaden koennen.
                if( bHandsOff )
                    xOldObj->DoHandsOff();

                // bestehende SfxMDIFrames f"ur dieses Doc leeren
                // eigenes Format oder R/O jetzt editierbar "offnen?
                SfxViewNotificatedFrameList_Impl aFrames;
                SfxViewFrame *pView = GetFirst(xOldObj);
                SfxObjectShellLock xNewObj;
                sal_Bool bRestoreView = ( pURLItem == NULL );
                TypeId aOldType = xOldObj->Type();

                while(pView)
                {
                    if( bHandsOff )
                        pView->GetDispatcher()->LockUI_Impl(sal_True);
                    aFrames.InsertViewFrame( pView );
                    pView->GetBindings().ENTERREGISTRATIONS();

                    // RestoreView nur wenn keine neue Datei geladen
                    // (Client-Pull-Reloading)
                    if( bHandsOff )
                        pView->ReleaseObjectShell_Impl( bRestoreView );
                    pView = bHandsOff ? (SfxTopViewFrame*) GetFirst(
                        xOldObj, TYPE(SfxTopViewFrame) ) :
                        (SfxTopViewFrame*)GetNext( *pView, xOldObj,
                                               TYPE( SfxTopViewFrame ) );
                }

                MemCache_Impl& rCache = SfxPickList_Impl::Get()->GetMemCache();
                if( !pNoCacheItem || pNoCacheItem->GetValue() )
                {
                    SfxObjectShell* pSh = xOldObj;
                    if ( pURLItem )
                        pSh = rCache.Find( aURL, String() );
                    if( pSh )
                    {
                        pSh->PrepareReload();
                        rCache.RemoveObject( pSh );
                    }
                }
                DELETEZ( xOldObj->Get_Impl()->pReloadTimer );

                // Medium mit angepa\stem Open-Mode
                StreamMode nMode = bForEdit ? SFX_STREAM_READWRITE
                    : SFX_STREAM_READONLY;

                SfxMedium *pNewMedium;
                SfxItemSet* pNewSet = 0;
                if( !pURLItem )
                {
                    pNewSet = new SfxAllItemSet( *pMedium->GetItemSet() );
                    pNewSet->ClearItem( SID_VIEW_ID );
                    pNewSet->ClearItem( SID_USER_DATA );
                }

                const SfxObjectFactory* pFactory = 0;

                // Wenn OrigURL mitkam nicht Filter verwenden, denn dann
                // kann es sich um die sba Geschichten handeln.
                SFX_ITEMSET_ARG( pNewSet, pOrigURL, SfxStringItem, SID_ORIGURL, sal_False);
                sal_Bool bUseFilter = !pOrigURL && !pURLItem;

                // Falls eine salvagede Datei vorliegt, nicht nochmals die
                // OrigURL mitschicken, denn die Tempdate ist nach Reload
                // ungueltig
                SFX_ITEMSET_ARG( pNewSet, pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);

                if( pSalvageItem && pOrigURL )
                {
                    if( !pURLItem ) aURL = pOrigURL->GetValue();
                    pNewSet->ClearItem( SID_ORIGURL );
                    pNewSet->ClearItem( SID_DOC_SALVAGE );
                }

                pNewMedium = new SfxMedium(
                    aURL, nMode, pMedium->IsDirect(), bUseFilter ? pMedium->GetFilter() : 0, pNewSet );

                pNewSet = pNewMedium->GetItemSet();
                if ( pURLItem )
                {
                    pNewSet->Put( SfxStringItem( SID_REFERER, pMedium->GetName() ) );
                }
                else
                    pNewSet->Put( SfxStringItem( SID_REFERER, String() ) );

                SFX_REQUEST_ARG( rReq, pBindingItem, SfxRefItem, SID_BINDING, sal_False);
                if( pBindingItem )
                    DBG_ERROR( "Not implemented!" );

                xOldObj->CancelTransfers();
                pNewSet->Put( SfxUInt32Item( SID_FLAGS,
                                   xOldObj->GetFlags() & (
                                       SFXOBJECTSHELL_DONTREPLACE |
                                       SFXOBJECTSHELL_DONTCLOSE ) ) );
                pNewMedium->SetUsesCache(
                    xOldObj->Get_Impl()->bReloadAvailable ||
                    pNoCacheItem && !pNoCacheItem->GetValue() );

                // eigentliches Reload
                if ( pDesc )
                    pNewSet->Put( SfxFrameDescriptorItem( pDesc, SID_FRAMEDESCRIPTOR ) );
                pNewSet->Put( SfxUInt16Item( SID_BROWSEMODE, NO_BROWSE ) );
                pNewSet->Put( SfxBoolItem( SID_RELOAD, sal_True ) );
                pNewSet->Put( SfxFrameItem ( SID_DOCFRAME, GetFrame() ) );

                LoadEnvironment_ImplRef xLoader =
                    new LoadEnvironment_Impl(
                        pNewMedium, GetFrame(),
                        LEI_DETECTFILTER | LEI_LOAD | LEI_CREATEVIEW, sal_False, sal_False );
                xOldObj->SetModified( sal_False );
                // Altes Dok nicht cachen! Gilt nicht, wenn anderes
                // Doc geladen wird.
                if( !pURLItem || pURLItem->GetValue() ==
                    xOldObj->GetMedium()->GetName() )
                    xOldObj->Get_Impl()->bForbidCaching = sal_True;
                xLoader->Start();
                while( xLoader->GetState() != LoadEnvironment_Impl::DONE )
                    Application::Yield();

                // hat reload nicht geklappt?
                xNewObj = xLoader->GetObjectShell();
                if( !xNewObj.Is() )
                {
                    pNewMedium =  xLoader->GetMedium();
                    if( pNewMedium ) pNewMedium->Close();

                    // wieder auf das alte Medium zurueck
                    const SfxFilter* pOldFilter = xOldObj->GetMedium()->GetFilter();
                    if( bHandsOff )
                        xOldObj->DoSaveCompleted( xOldObj->GetMedium() );

                    // r/o-Doc kann nicht in Editmode geschaltet werden?
                    if ( bForEdit && SID_EDITDOC == rReq.GetSlot() )
                    {
                        // dem ::com::sun::star::sdbcx::User anbieten, als Vorlage zu oeffnen
                        QueryBox aBox( &GetWindow(), SfxResId(MSG_QUERY_OPENASTEMPLATE) );
                        if ( RET_YES == aBox.Execute() )
                        {
                            SfxAllItemSet aSet( pApp->GetPool() );
                            aSet.Put( SfxStringItem( SID_FILE_NAME, pNewMedium->GetName() ) );
                            SFX_ITEMSET_ARG( pNewMedium->GetItemSet(), pOptions,
                                             SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False);
                            if ( pOptions )
                                aSet.Put( *pOptions );
                            SFX_ITEMSET_ARG( pNewMedium->GetItemSet(), pReferer,
                                             SfxStringItem, SID_REFERER, sal_False);
                            if ( pReferer )
                                aSet.Put( *pReferer );
                            aSet.Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
                            if( pNewMedium->GetFilter() )
                                aSet.Put( SfxStringItem( SID_FILTER_NAME,
                                                         pNewMedium->GetFilter()->GetName() ) );

                            //MI: im selben Frame => er macht gar nix !?!
                            //SfxFrameItem aFrameItem( SID_DOCFRAME, GetFrame() );
                            GetDispatcher()->Execute( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, aSet );
                        }
                    }
                }
                else
                    xNewObj->GetMedium()->GetItemSet()->ClearItem( SID_RELOAD );

                SfxViewFrame* pThis = (SfxViewFrame*)this;
                sal_Bool bDeleted = aFrames.C40_GETPOS( SfxViewFrame, pThis ) == USHRT_MAX;

                if( !bDeleted )
                {
/*                    if( GetFrame()->GetLoadEnvironment_Impl() == &xLoader )
                        GetFrame()->SetLoadEnvironment_Impl( 0 );*/

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
                    SfxViewFrame *pView = aFrames.GetObject( i );
                    if( !bHandsOff && this != pView   )
                        pView->ReleaseObjectShell_Impl( bRestoreView );
                    pView->SetRestoreView_Impl( bRestoreView );
                    if( pView != this || !xNewObj.Is() )
                    {
                        SfxFrame *pFrame = pView->GetFrame();
                        pFrame->InsertDocument(xNewObj.Is() ? xNewObj : xOldObj );
                    }

                    pView->GetBindings().LEAVEREGISTRATIONS();
                    pView->GetDispatcher()->LockUI_Impl( sal_False );
                }

                // als erledigt recorden
                rReq.Done( sal_True );
                rReq.SetReturnValue(SfxBoolItem(rReq.GetSlot(), sal_True));
                if( !bDeleted )
                {
                    Notify( *GetObjectShell(), SfxSimpleHint(
                        SFX_HINT_TITLECHANGED ));

                    // ReloadForEdit bei Framesets schaltet auch FramesetEditmode
                    sal_Bool bIsReadonly = GetObjectShell()->IsReadOnly();
                    if ( bIsReadonly != bWasReadonly &&
                         rReq.GetSlot() == SID_EDITDOC  && !GetFrame()->GetParentFrame() )
                    {
                        SfxBoolItem aItem( SID_EDIT_FRAMESET, !bIsReadonly );
                        GetDispatcher()->Execute( SID_EDIT_FRAMESET,
                                                  SFX_CALLMODE_RECORD, &aItem, 0L );
                    }
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

    SfxFrame *pParent = GetFrame()->GetParentFrame();
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
                if ( !pSh || !pSh->HasName() ||
                     !( pSh->Get_Impl()->nLoadedFlags &  SFX_LOADED_MAINDOCUMENT ) )
                    rSet.DisableItem( SID_EDITDOC );
                else
                {
                    SFX_ITEMSET_ARG( pSh->GetMedium()->GetItemSet(), pItem,
                                     SfxBoolItem, SID_EDITDOC, sal_False );
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

                // Wenn irgendein ChildFrame reloadable ist, wird der Slot
                // enabled, damit man CTRL-Reload machen kann
                sal_Bool bReloadAvailable = sal_False;
                SfxFrameIterator aIter( *pFrame, sal_True );
                for( SfxFrame* pNextFrame = aIter.FirstFrame();
                        pFrame;
                        pNextFrame = pNextFrame ?
                            aIter.NextFrame( *pNextFrame ) : 0 )
                {
                    SfxObjectShell *pShell = pFrame->GetCurrentDocument();
                    if( pShell && pShell->Get_Impl()->bReloadAvailable )
                    {
                        bReloadAvailable = sal_True;
                        break;
                    }
                    pFrame = pNextFrame;
                }

                if ( !pSh || !pSh->CanReload_Impl() )
                    rSet.DisableItem(nWhich);
                else
                    rSet.Put( SfxBoolItem( nWhich, bReloadAvailable));

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
    SfxUndoManager *pUndoMgr = pSh->GetUndoManager();
    sal_Bool bOK = sal_False;
    if ( pUndoMgr )
    {
        switch ( rReq.GetSlot() )
        {
            case SID_CLEARHISTORY:
                pUndoMgr->Clear();
                bOK = sal_True;
                break;

            case SID_UNDO:
                pUndoMgr->Undo(0);
                GetBindings().InvalidateAll(sal_False);
                bOK = sal_True;
                break;

            case SID_REDO:
                pUndoMgr->Redo(0);
                GetBindings().InvalidateAll(sal_False);
                bOK = sal_True;
                break;

            case SID_REPEAT:
                if ( pSh->GetRepeatTarget() )
                    pUndoMgr->Repeat( *pSh->GetRepeatTarget(), 0);
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

    SfxUndoManager *pUndoMgr = pSh->GetUndoManager();
    if ( !pUndoMgr )
    {
        // der SW hat eigenes Undo an der ::com::sun::star::sdbcx::View
        SfxWhichIter aIter( rSet );
        SfxViewShell *pViewSh = GetViewShell();
        if( !pViewSh ) return;
        for ( sal_uInt16 nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
            pViewSh->GetSlotState( nSID, 0, &rSet );
        return;
    }

    if ( pUndoMgr->GetUndoActionCount() == 0 &&
         pUndoMgr->GetRedoActionCount() == 0 &&
         pUndoMgr->GetRepeatActionCount() == 0 )
        rSet.DisableItem( SID_CLEARHISTORY );

    if ( pUndoMgr && pUndoMgr->GetUndoActionCount() )
    {
        String aTmp( SfxResId( STR_UNDO ) );
        aTmp += pUndoMgr->GetUndoActionComment(0);
        rSet.Put( SfxStringItem( SID_UNDO, aTmp ) );
    }
    else
        rSet.DisableItem( SID_UNDO );

    if ( pUndoMgr && pUndoMgr->GetRedoActionCount() )
    {
        String aTmp( SfxResId(STR_REDO) );
        aTmp += pUndoMgr->GetRedoActionComment(0);
        rSet.Put( SfxStringItem( SID_REDO, aTmp ) );
    }
    else
        rSet.DisableItem( SID_REDO );
    SfxRepeatTarget *pTarget = pSh->GetRepeatTarget();
    if ( pUndoMgr && pTarget && pUndoMgr->GetRepeatActionCount() &&
         pUndoMgr->CanRepeat(*pTarget, 0) )
    {
        String aTmp( SfxResId(STR_REPEAT) );
        aTmp += pUndoMgr->GetRepeatActionComment(*pTarget, 0);
        rSet.Put( SfxStringItem( SID_REPEAT, aTmp ) );
    }
    else
        rSet.DisableItem( SID_REPEAT );
}

//--------------------------------------------------------------------

void SfxViewFrame::PropState_Impl( SfxItemSet &rSet )
{
    GetDispatcher()->Flush();
    const sal_uInt16 *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges, "Set ohne Bereich");
    while ( *pRanges )
    {
        for ( sal_uInt16 nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich )
        {
            switch(nWhich)
            {
                case SID_SELECTION:
                    break;
                case SID_TOPWINDOW:
                    rSet.Put( SfxObjectItem( SID_TOPWINDOW, GetTopViewFrame() ) );
                    break;
                case SID_PARENTFRAME:
                    rSet.Put( SfxObjectItem( nWhich, GetParentViewFrame() ) );
                    break;
                case SID_DOCUMENT:
                    if ( GetObjectShell() )
                        rSet.Put( SfxObjectItem( SID_DOCUMENT, GetObjectShell() ) );
                    break;
                case SID_ACTIVEWINDOW:
                {
                    SfxViewFrame* pActFrame = this;
                    while ( pActFrame->GetActiveChildFrame_Impl() )
                            pActFrame = pActFrame->GetActiveChildFrame_Impl();
                    rSet.Put( SfxObjectItem( SID_ACTIVEWINDOW, pActFrame ) );
                    break;
                }
                case SID_FRAMECOUNT:
                    rSet.Put( SfxUInt16Item( SID_FRAMECOUNT, GetFrame()->GetChildFrameCount() ) );
                    break;
                case SID_ISTOP:
                    rSet.Put( SfxBoolItem( SID_ISTOP, GetFrame()->GetTopFrame() == GetFrame() ) );
                    break;
            }
        }
        ++pRanges;
    }
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

    if ( rObjSh.IsA( TYPE( SfxFrameSetObjectShell ) ) )
        GetFrame()->SetFrameType_Impl( GetFrameType() | SFXFRAME_FRAMESET );
    else
        GetFrame()->SetFrameType_Impl( GetFrameType() & ~SFXFRAME_FRAMESET );

    // Modulshell einf"ugen
    SfxModule* pModule = xObjSh->GetModule();
    if( pModule )
        pDispatcher->InsertShell_Impl( *pModule, 0 );

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

    // was so in Activate passiert w"are
    SfxObjectShell *pDocSh = GetObjectShell();
    if ( SfxViewFrame::Current() == this )
    {
        // ggf. Config-Manager aktivieren
        SfxConfigManager *pCfgMgr = rObjSh.GetConfigManager();
        if ( pCfgMgr )
            pCfgMgr->Activate( SFX_CFGMANAGER() );
    }

    if ( !rObjSh.IsLoading() )
        rObjSh.PostActivateEvent_Impl();

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
    SfxViewShell *pDyingViewSh = GetViewShell();
    pImp->aLastType = xObjSh->Type();

    HACK(MI weiss nicht wie !pSh sein kann - nach PlugIns isses aber so)
    if ( pDyingViewSh )
    {
        // Gibt es noch andere Views auf mein Doc?
        SfxViewFrame *pView = GetFirst(xObjSh);
        while( pView )
        {
            if ( pView != this )
                break;
            pView = GetNext( *pView, xObjSh );
        }

        if ( !pView )
        {
            // Ich bin die letzte ::com::sun::star::sdbcx::View
            SfxObjectFactory *pFactory = &xObjSh->GetFactory();
            if ( pFactory && pFactory->GetFlags() & SFXOBJECTSHELL_HASOPENDOC )
            {
                // Event nur bei echten Dokumenten
                xObjSh->Get_Impl()->bInCloseEvent = sal_True;
                SFX_APP()->NotifyEvent( SfxEventHint(SFX_EVENT_CLOSEDOC, xObjSh) );
                xObjSh->Get_Impl()->bInCloseEvent = sal_False;
            }
        }

        SetRestoreView_Impl( bStoreView );
        if ( bStoreView )
            pDyingViewSh->WriteUserData( GetViewData_Impl(), sal_True );

        // Falls es SubFrames gibt, m"ussen diese deleted werden, solange noch
        // die FramesetViewShell da ist, sonst gibt es Probleme.
        // Um Flackern bei den Objectbars zu vermeiden, werden die SubFrames
        // zerst"ort, solange die FrameSetView noch da ist.
        if ( pDyingViewSh->IsA( TYPE(SfxFrameSetViewShell) ) )
        {
            Window *pWindow = pDyingViewSh->GetWindow();
            if ( pWindow )
                pWindow->Hide();
            GetFrame()->CloseChildFrames();
        }

        // Jetzt alle SubShells wechhauen
        pDyingViewSh->PushSubShells_Impl( sal_False );
        sal_uInt16 nLevel = pDispatcher->GetShellLevel( *pDyingViewSh );
        if ( nLevel )
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
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Keine Shell");
#endif

    GetDispatcher()->Flush();

    if ( GetWindow().HasChildPathFocus( sal_True ) )
    {
        DBG_ASSERT( !GetActiveChildFrame_Impl(), "Wrong active child frame!" );
        GetWindow().GrabFocus();
    }

    pDyingViewSh->DisconnectClients_Impl( NULL );
    SetViewShell_Impl(0);
    delete pDyingViewSh;

    pDispatcher->Pop( *xObjSh );
    SfxModule* pModule = xObjSh->GetModule();
    if( pModule )
        pDispatcher->RemoveShell_Impl( *pModule );

    pDispatcher->Flush();
    EndListening( *xObjSh );
    SFX_NOTIFY( *xObjSh, xObjSh->Type(),
            SfxSimpleHint(SFX_HINT_TITLECHANGED),
            TYPE(SfxSimpleHint) );

    SFX_NOTIFY( *xObjSh, xObjSh->Type(),
            SfxSimpleHint(SFX_HINT_DOCCHANGED),
            TYPE(SfxSimpleHint) );

    // Damit ::com::sun::star::script::JavaScript Objekte das ::com::sun::star::script::JavaScript ueber
    // Frame->ViewFrame->DocShell->Medium bei CloseEvents besorgen koennen
    if ( 1 == xObjSh->GetOwnerLockCount() && pImp->bObjLocked )
        xObjSh->DoClose();
    SfxObjectShellRef xDyingObjSh = xObjSh;
    xObjSh.Clear();
    if( ( GetFrameType() & SFXFRAME_HASTITLE  ))
        xDyingObjSh->GetNoSet_Impl().ReleaseIndex(pImp->nDocViewNo-1);
    if ( pImp->bObjLocked )
    {
        xDyingObjSh->OwnerLock( sal_False );
        pImp->bObjLocked = sal_False;
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
        return String( DEFINE_CONST_UNICODE( "UNO-Component" ) );

    if  ( pObjSh->GetInPlaceObject() && pObjSh->GetInPlaceObject()->GetProtocol().IsEmbed() )
        // kein UpdateTitle mit Embedded-ObjectShell
        return String();

    // Bei jedem Namenswechsel einen ::com::sun::star::chaos::Anchor anlegen und GETDATA putten,
    // damit das nicht bei jeder Anforderung ans Chaos immer wieder gemacht
    // werden mu\s ( Images !! )
    const SfxMedium *pMedium = pObjSh->GetMedium();
    String aURL;
    SfxFrame *pFrm = GetFrame();
    if ( pObjSh->HasName() )
    {
        aURL = pMedium->GetURLObject().GetURLNoPass();
    }

    // Erstmal den alten Anchor ignorieren
//(dv)  if ( pImp->xAnchor.Is() )
//(dv)      EndListening( *pImp->xAnchor );

    if ( aURL != pImp->aActualURL )
        // ::com::sun::star::util::URL hat sich ge"andert
        pImp->aActualURL = aURL;

    // Selbst wenn die URL sich nicht ge"andert hat, vielleicht ja der Anchor !!
#if 0   //(dv)
    pImp->xAnchor = pMedium->GetAnchor( sal_True );
    if ( pImp->xAnchor.Is() )
    {
        // PresentationURL aktualisieren
        pImp->aActualPresentationURL = pImp->xAnchor->GetPresentationURL();

        // Am neuen (?) ::com::sun::star::chaos::Anchor horchen
        StartListening( *pImp->xAnchor );
    }
#endif  //(dv)

    // Titel des Fensters
    String aName( pObjSh->GetTitle(SFX_TITLE_CAPTION) );
    String aTitle(aName);
    String aOldTitle(aName);
    aName += ':';
    aName += String::CreateFromInt32(pImp->nDocViewNo);

    // gibt es "uberhaupt noch eine weitere ::com::sun::star::sdbcx::View?
    sal_uInt16 nViews=0;
    for ( SfxViewFrame *pView= GetFirst(pObjSh);
          pView && nViews<2;
          pView = GetNext(*pView,pObjSh) )
        if ( ( pView->GetFrameType() & SFXFRAME_HASTITLE ) &&
             !IsDowning_Impl())
            nViews++;
    if ( nViews == 2 || pImp->nDocViewNo > 1 )
        // nur dann die Nummer dranh"angen
        aTitle = aName;

    if ( pObjSh->IsReadOnly() )
        aTitle += String( SfxResId(STR_READONLY) );

    // Name des SbxObjects
    String aSbxName = pObjSh->SfxShell::GetName();
    if ( IsVisible() )
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

    DBG_ASSERT( GetFrame()->IsClosing_Impl(), "Niemals den ViewFrame vor dem Frame closen!" );

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
    SfxApplication *pSfxApp = SFX_APP();

#ifdef WIN
    pSfxApp->TestFreeResources_Impl();
#endif

    pDispatcher->DoActivate_Impl( bUI );

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
    SfxApplication *pSfxApp = SFX_APP();
    pDispatcher->DoDeactivate_Impl( bUI );

    // Wenn ich einen parent habe und dieser ist kein parent des neuen
    // ViewFrames, erh"alt er ein ParentDeactivate
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
void SfxViewFrame::InvalidateBorderImpl
(
    const SfxViewShell* pSh
)

{
    if( pSh && !nAdjustPosPixelLock )
    {
        //! Hack
        if ( ISA(SfxInPlaceFrame) )
            ((SfxInPlaceFrame*)this)->InvalidateBorderImpl( pSh );
        else if ( ISA(SfxTopViewFrame) )
            ((SfxTopViewFrame*)this)->InvalidateBorderImpl( pSh );
        else
            ((SfxInternalFrame*)this)->InvalidateBorderImpl( pSh );
    }
}

//------------------------------------------------------------------------
sal_Bool SfxViewFrame::SetBorderPixelImpl
(
    const SfxViewShell* pSh,
    const SvBorder&     rBorder
)

{
    pImp->aBorder = rBorder;
    return sal_True;
}

//------------------------------------------------------------------------
const SvBorder& SfxViewFrame::GetBorderPixelImpl
(
    const SfxViewShell* pSh
)   const

{
    return pImp->aBorder;
}

//--------------------------------------------------------------------
void SfxViewFrame::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
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
        // auf ReadOnly gesetzt, was zur"ckgenommen werden mu\s, wenn
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
                    GetDispatcher()->GetBindings()->InvalidateAll(sal_True);
                }

                break;
            }

            case SFX_EVENT_LOADFINISHED:
            {
                // Ein fertig geladenes Dokument kann das Event nicht selbst ausl"osen,
                // weil es nicht wei\s, ob schon eine ::com::sun::star::sdbcx::View erzeugt wurde
                xObjSh->PostActivateEvent_Impl();
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
#if 0   // (dv)
    else if ( &rBC == (SfxBroadcaster*) (CntAnchor*) pImp->xAnchor )
    {
        CntAnchorHint* pCHint = PTR_CAST(CntAnchorHint, &rHint);
        if ( pCHint )
        {
            CntAction eAction = pCHint->GetAction();
            switch( eAction )
            {
                case CNT_ACTION_EXCHANGED :
                {
                    SfxMedium* pMedium = GetObjectShell()->GetMedium();
                    pMedium->RefreshName_Impl();
                    Reference< XController >  xController =
                        GetFrame()->GetFrameInterface()->getController();
                    Reference< XModel >  xModel = xController->getModel();
                    if ( xModel.is() )
                        xModel->attachResource( S2U( pMedium->GetName() ), xModel->getArgs() );
                    UpdateTitle();
                    pBindings->Invalidate( SID_CURRENT_URL, sal_True, sal_False );
                    pBindings->Update( SID_CURRENT_URL );
                    break;
                }
            }
        }
    }
#endif  // (dv)
}

//------------------------------------------------------------------------
void SfxViewFrame::Construct_Impl( SfxObjectShell *pObjSh )
{
    pImp->pFrame->DocumentInserted( pObjSh );
    pImp->bInCtor = sal_True;
    pImp->pParentViewFrame = 0;
    pImp->bResizeInToOut = sal_True;
    pImp->pImportShell = 0;
    pImp->bObjLocked = sal_False;
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
    pImp->pIPFrame = 0;

    SetPool( &SFX_APP()->GetPool() );
    pDispatcher = new SfxDispatcher(this);

    xObjSh = pObjSh;
    if ( xObjSh.Is() && xObjSh->IsPreview() )
        SetQuietMode_Impl( sal_True );

    if ( pObjSh && pObjSh->IsA( TYPE( SfxFrameSetObjectShell ) ) )
        GetFrame()->SetFrameType_Impl( GetFrameType() | SFXFRAME_FRAMESET );
    else
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

    SfxNewHdl::Get()->TryAllocBuffer();
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
    return SFX_APP() ? SFX_APP()->GetViewFrame() : NULL;
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
             pFrame->IsVisible() )
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
             ( !bOnlyIfVisible || pFrame->IsVisible()) )
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
             ( !bOnlyIfVisible || pFrame->IsVisible()) )
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
        if ( !pFrame->IsVisible() )
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
void SfxViewFrame::ShowStatusText( const String& rText)
{
    SfxWorkWindow* pWorkWin = GetFrame()->GetWorkWindow_Impl();
    SfxStatusBarManager *pMgr = pWorkWin->GetStatusBarManager_Impl();
    if ( pMgr )
    {
        pMgr->GetStatusBar()->HideItems();
        pMgr->GetStatusBar()->SetText( rText );
    }
}

//--------------------------------------------------------------------
void SfxViewFrame::HideStatusText()
{
    SfxWorkWindow* pWorkWin = GetFrame()->GetWorkWindow_Impl();
    SfxStatusBarManager *pMgr = pWorkWin->GetStatusBarManager_Impl();
    if ( pMgr )
        pMgr->GetStatusBar()->ShowItems();
}


//--------------------------------------------------------------------
SfxIniManager* SfxViewFrame::GetIniManager() const
{
/*  SfxIniManager *pIniMgr = GetObjectShell()
            ? GetObjectShell()->GetFactory().GetIniManager()
            : 0;
    if ( !pIniMgr )*/ //!
        return SFX_APP()->GetAppIniManager();
//  return pIniMgr;
}

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
    if ( pVSh && !IsA(TYPE(SfxInPlaceFrame)) && !pVSh->UseObjectSize() )
        pImp->bResizeInToOut = sal_False;
}

#if SUPD<604
//--------------------------------------------------------------------
SfxViewShell* SfxViewFrame::GetViewShell() const

/*  [Beschreibung]

    Liefert, sofern existent, die zur Zeit in diesem SfxViewFrame aktive
    <SfxViewShell>. Dieser kann sich bei speziellen Multi-::com::sun::star::sdbcx::View-Frames
    von Zeit zu Zeit "„ndern. Der R"uckgabewert ist nur im aktuellen
    Stack-Frame g"ultig.
*/

{
    return SfxShell::GetViewShell();
}
#endif

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

SfxViewFrame* SfxViewFrame::GetParentViewFrame_Impl() const
{
    return pImp->pParentViewFrame;
}

//--------------------------------------------------------------------
void SfxViewFrame::ForceOuterResize_Impl(sal_Bool bOn)
{
    pImp->bResizeInToOut = !bOn;
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
            while ( !pWindow->IsSystemWindow() )
                pWindow = pWindow->GetParent();
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

    // Anchor auf gelesen setzen
    Reference < XContent > xContent( GetObjectShell()->GetMedium()->GetContent() );
    if ( xContent.is() )
    {
        Any aAny( UCB_Helper::GetProperty( xContent, WID_FLAG_IS_FOLDER ) );
        sal_Bool bIsFolder = FALSE;
        if ( !( aAny >>= bIsFolder ) || !bIsFolder )
        {
            Any aSet;
            aSet <<= sal_True;
            UCB_Helper::SetProperty( xContent, WID_IS_READ, aSet );
        }
    }

    // Frame-Window anzeigen, aber nur wenn der ViewFrame kein eigenes Window
    // hat oder wenn er keine Component enth"alt
    if ( &GetWindow() == &GetFrame()->GetWindow() || !GetFrame()->HasComponent() )
        GetWindow().Show();
}

//--------------------------------------------------------------------
sal_Bool SfxViewFrame::IsVisible() const
{
    Window *pWin = pImp->bInCtor ? 0 : &GetWindow();
    return GetFrame()->HasComponent() || pImp->bObjLocked || ( pWin && pWin->IsVisible() );
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
        if ( IsVisible() )
        {
            if ( GetViewShell() )
            {
                BOOL bPreview = FALSE;
                SfxApplication *pSfxApp = SFX_APP();
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
                if ( GetFrame()->GetFrameInterface()->isActive() || !bPreview && ( !pCurrent || bGrabFocus ) )
                {
                    pSfxApp->SetViewFrame( this );
                    if ( bGrabFocus )
                    {
                        SvInPlaceClient *pCli = GetViewShell()->GetIPClient();
                        if ( ( !pCli || !pCli->GetProtocol().IsUIActive() ) &&
                            ( !pCurrent || pCurrent->GetParentViewFrame_Impl() != this ) )
                                GetFrame()->GrabFocusOnComponent_Impl();
                    }
                }
                else
                {
                    GetBindings().SetDispatcher( GetDispatcher() );
                    GetBindings().SetActiveFrame( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > () );
                    GetDispatcher()->Update_Impl( TRUE );
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
        if ( !pViewFrame )
        {
    //!     if ( GetChildFrame(0) && GetViewShell() && GetViewShell()->IsA( TYPE(SfxFrameSetViewShell) ) )
    //!         GetDispatcher()->LockUI_Impl();
        }
        else if ( !pImp->pActiveChild )
        {
            GetDispatcher()->LockUI_Impl( sal_False );
        }

        pImp->pActiveChild = pViewFrame;

        Reference< XFramesSupplier > xFrame( GetFrame()->GetFrameInterface(), UNO_QUERY );
        Reference< XFrame >  xActive;
        if ( pViewFrame )
            xActive = pViewFrame->GetFrame()->GetFrameInterface();

        if ( xFrame.is() )  // PB: #74432# xFrame cann be NULL
            xFrame->setActiveFrame( xActive );

        if ( pViewFrame )
        {
            // Das n"achsth"ohere Frameset suchen, falls ich nicht selbst schon
            // eines enthalte( der InternalFrame k"onnte auch zu einem FloatingFrame
            // geh"oren ) und dort den Frame aktivieren, der letztlich pViewFrame
            // enth"alt.
            SfxFrame *pFrame = GetFrame();
            do
            {
                SfxURLFrame *pURLFrame = PTR_CAST( SfxURLFrame, pFrame );
                if ( pURLFrame && pURLFrame->GetFrameSet() )
                {
                    pURLFrame->GetFrameSet()->SetActiveFrame( pURLFrame );
                    break;
                }
                pFrame = pFrame->GetParentFrame();
            }
            while ( pFrame );
        }
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
    DBG_ASSERT( GetObjectShell(), "Kein Dokument!" );

    SfxObjectFactory &rDocFact = GetObjectShell()->GetFactory();

    // die Indizes der neuen und alten ::com::sun::star::sdbcx::View-Shell finden
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
                nViewId = nFoundId; // fuer nViewId == 0
            }
            else if ( !bIsIndex && nViewId == nFoundId )
               nNewNo = nNo;
        }
        if ( pImp->nCurViewId == nFoundId )
            nOldNo = nNo;
    }

    if ( nNewNo == USHRT_MAX )
    {
        // Bei unbekannter Id die Dafault-Id nehmen
        sal_uInt16 nFoundId = rDocFact.GetViewFactory(0).GetOrdinal();
        nNewNo = 0;
        nViewId = nFoundId;
        if ( pImp->nCurViewId == nFoundId )
            nOldNo = 0;
    }

    SfxViewShell *pSh = GetViewShell();

    DBG_ASSERT( !pSh || nOldNo != USHRT_MAX, "old shell id not found" );

    // existiert schon eine ViewShell?
    sal_Bool bHasFocus = sal_False;
    SfxViewShell *pOldSh = pSh;
    if ( pOldSh )
    {
        if ( !bHasFocus )
            bHasFocus = pOldSh->GetWindow() && pOldSh->GetWindow()->HasChildPathFocus( sal_True );

        // dann diese schlie\sen
        if ( !pOldSh->PrepareClose() )
            return sal_False;

        // Jetzt alle SubShells wechhauen
        pOldSh->PushSubShells_Impl( sal_False );
        sal_uInt16 nLevel = pDispatcher->GetShellLevel( *pOldSh );
        if ( nLevel )
        {
            // Es gibt SubShells
            SfxShell *pSubShell = pDispatcher->GetShell( nLevel-1 );
            if ( pSubShell == pOldSh->GetSubShell() )
                //"Echte" Subshells nicht deleten
                pDispatcher->Pop( *pSubShell, SFX_SHELL_POP_UNTIL );
            else
                pDispatcher->Pop( *pSubShell, SFX_SHELL_POP_UNTIL | SFX_SHELL_POP_DELETE );
        }
        pDispatcher->Pop( *pOldSh );
        GetBindings().Invalidate( nOldNo + SID_VIEWSHELL0 );
    }

    // Id der neuen ViewShell eintragen
    pImp->nCurViewId = nViewId;
    GetBindings().Invalidate( nNewNo + SID_VIEWSHELL0 );

    // neue ViewShell erzeugen
    SfxViewFactory &rViewFactory = rDocFact.GetViewFactory( nNewNo );
    LockAdjustPosSizePixel();

    GetBindings().ENTERREGISTRATIONS();
    pSh = rViewFactory.CreateInstance(this, pOldSh);
    Window *pEditWin = pSh->GetWindow();
    DBG_ASSERT( !pEditWin || !pEditWin->IsReallyVisible(), "don`t show your ViewShell`s Window by yourself!" );

    // neue ViewShell in Frame einsetzen
    GetDispatcher()->SetDisableFlags( 0 );
    SetViewShell_Impl(pSh);

//    if( !pSh->pImp->bControllerSet )
    {
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
            xModel->setCurrentController( xController );
        }
    }

    GetDispatcher()->Push( *pSh );
    if ( pSh->GetSubShell() )
        GetDispatcher()->Push( *pSh->GetSubShell() );
    pSh->PushSubShells_Impl();
    GetDispatcher()->Flush();

    // ggf. UserData restaurieren
    if ( pImp->bRestoreView && pImp->aViewData.Len() )
    {
        pSh->ReadUserData( pImp->aViewData, sal_True );
        pImp->bRestoreView = sal_False;
    }

    // Tools anordnen, bevor die Gr"os\se gesetzt wird
    if ( SfxViewFrame::Current() == this )
        GetDispatcher()->Update_Impl( sal_True );

    // ggf. Resize ausl"osen
    UnlockAdjustPosSizePixel();

    Window* pFrameWin = &GetWindow();
    if ( pFrameWin != &GetFrame()->GetWindow() )
        pFrameWin->Show();

    if ( GetWindow().IsReallyVisible() )
        DoAdjustPosSizePixel( pSh, Point(), GetWindow().GetOutputSizePixel() );

    if ( pEditWin && pSh->IsShowView_Impl() )
    {
        SfxFrameSetViewShell *pFrSh = PTR_CAST( SfxFrameSetViewShell, pSh );
        if ( pFrSh )
            pFrSh->GetSplitWindow()->Show();
        else
            pEditWin->Show();
        if ( bHasFocus )
            GetFrame()->GrabFocusOnComponent_Impl();
    }

    GetBindings().LEAVEREGISTRATIONS();
    delete pOldSh;
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
#ifdef BASIC_HACKS
        case SID_HELP_STRING:
        {
            SFX_REQUEST_ARG( rReq, pItem, SfxUInt16Item, SID_CONFIGITEMID, sal_False );
            if ( pItem )
            {
                sal_uInt16 nId = pItem->GetValue();
                SfxModule *pMod = GetObjectShell()->GetModule();
                SfxSlotPool* pPool = pMod ? pMod->GetSlotPool() : NULL;
                if ( !pPool )
                    pPool = &SFX_APP()->GetSlotPool();
                rReq.SetReturnValue( SfxStringItem( SID_HELP_STRING, pPool->GetSlotHelpText_Impl( nId ) ) );
            }
            break;
        }

        case SID_METHODNAME:
        {
            SFX_REQUEST_ARG( rReq, pItem, SfxUInt16Item, SID_CONFIGITEMID, sal_False );
            if ( pItem )
            {
                sal_uInt16 nId = pItem->GetValue();
                SfxModule *pMod = GetObjectShell()->GetModule();
                SfxSlotPool* pPool = pMod ? pMod->GetSlotPool() : NULL;
                if ( !pPool )
                    pPool = &SFX_APP()->GetSlotPool();
                rReq.SetReturnValue( SfxStringItem( SID_METHODNAME, pPool->GetSlotName_Impl( nId ) ) );
            }
            break;
        }

        case SID_CONFIGITEMID:
        {
            SFX_REQUEST_ARG( rReq, pItem, SfxUInt16Item, SID_CONFIGITEMID, sal_False );
            if ( pItem )
            {
                sal_uInt16 nId = pItem->GetValue();
                SfxModule *pMod = GetObjectShell()->GetModule();
/*
                SfxSlotPool* pPool = pMod ? pMod->GetSlotPool() : NULL;
                if ( !pPool )
                    pPool = &SFX_APP()->GetSlotPool();
                const SfxSlot *pSlot = pPool->GetSlot( nId );
                sal_Bool bRet = pSlot ? pSlot->IsMode( SFX_SLOT_TOOLBOXCONFIG ) : sal_False;
 */
                SFX_IMAGEMANAGER()->StartCustomize();
                Image aImage = SFX_IMAGEMANAGER()->GetImageFromModule_Impl( nId, pMod );
                sal_Bool bRet = aImage.GetSizePixel().Width() != 0;
                rReq.SetReturnValue( SfxBoolItem( SID_CONFIGITEMID, bRet ) );
            }
            break;
        }
#endif

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
                    if ( nActFrame == nFrame || pView &&
                         aName.CompareIgnoreCaseToAscii( pView->SfxShell::GetName() ) == COMPARE_EQUAL )
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
                SfxFrame *pFrame = SfxTopFrame::Create();
                SfxApplication* pApp = SFX_APP();
                SfxAllItemSet aSet( pApp->GetPool() );

                SFX_ITEMSET_ARG( pMed->GetItemSet(), pRefererItem, SfxStringItem, SID_REFERER, sal_False );
                SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE( "private:user" ) );
                if ( !pRefererItem )
                    pRefererItem = &aReferer;

                aSet.Put( SfxStringItem( SID_FILE_NAME, pMed->GetName() ) );
                aSet.Put( SfxFrameDescriptorItem( pFrame->GetDescriptor(), SID_FRAMEDESCRIPTOR ) );
                aSet.Put( SfxStringItem( SID_USER_DATA, aUserData ) );
                aSet.Put( SfxUInt16Item( SID_VIEW_ID, GetCurViewId() ) );
                aSet.Put( *pRefererItem );
                if( pMed->GetFilter() )
                    aSet.Put( SfxStringItem( SID_FILTER_NAME, pMed->GetFilter()->GetName()) );
                aSet.Put( SfxFrameItem ( SID_DOCFRAME, pFrame ) );
                GetDispatcher()->Execute( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, aSet );
            }
            else
            {
                pMed->GetItemSet()->Put( SfxStringItem( SID_USER_DATA, aUserData ) );
                pMed->GetItemSet()->Put( SfxFrameDescriptorItem(
                    GetFrame()->GetDescriptor(), SID_FRAMEDESCRIPTOR ) );
                SfxFrame *pNew = SfxTopFrame::Create( GetObjectShell(), GetCurViewId() );
            }

            rReq.Done();
            break;
        }
    }
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
                         nViewNo && !IsA( TYPE(SfxInPlaceFrame) ) )
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
                    if ( !GetViewShell()->NewWindowAllowed() && !pDocSh->HasName() )
                            rSet.DisableItem( nWhich );
                    break;
                }
            }
        }
    }
}

//-------------------------------------------------------------------------
void SfxViewFrame::GetState_Impl( SfxItemSet &rSet )
{
    if ( GetViewShell() && GetViewShell()->GetVerbs() && !ISA( SfxInPlaceFrame ) )
        rSet.Put(SfxStringListItem(SID_OBJECT));
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
String SfxViewFrame::GetHelpFile_Impl()
{
    String aHelpFileName = GetObjectShell()->GetFactory().GetHelpFile();
#ifndef TF_NEWDESKTOP
    DBG_ASSERT( GetObjectShell(), "Kein Dokument!" );
    Reference< XController >  xController = GetFrame()->GetController();
    if ( 0 == aHelpFileName.Len() && xController.is() )
    {
        if ( GetFrame()->GetFrameInterface()->findFrame(
                DEFINE_CONST_UNICODE( "StructureView" ), FrameSearchFlag::ALL ).is() )
            aHelpFileName = DEFINE_CONST_UNICODE( "desktop.hlp" );
        else
        {
            Reference< ::com::sun::star::beans::XPropertySet > xProp( xController, UNO_QUERY );
            if ( xProp.is() )
            {
                TRY
                {
                    Any aAny = xProp->getPropertyValue( ::rtl::OUString::createFromAscii( "HelpFileName" ) );
                    ::rtl::OUString sTemp ;
                    aAny >>= sTemp ;
                    aHelpFileName = String( sTemp );
                    return aHelpFileName;
                }
                CATCH_ALL()
                {
                }
                END_CATCH;
            }

            aHelpFileName = DEFINE_CONST_UNICODE( "schedule.hlp" );
        }
    }
#endif

    return aHelpFileName;
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

void SfxViewFrame::SetIPFrame_Impl( SfxInPlaceFrame *pIPFrame )
{
    pImp->pIPFrame = pIPFrame;
}

SfxInPlaceFrame* SfxViewFrame::GetIPFrame_Impl() const
{
    return pImp->pIPFrame;
}

String SfxViewFrame::GetActualPresentationURL_Impl() const
{
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
    return pImp->bModal;
}

void SfxViewFrame::Resize()
{
    Size aSize = GetWindow().GetOutputSizePixel();
    if ( aSize != pImp->aSize )
    {
        pImp->aSize = aSize;
        SfxViewShell *pShell = GetViewShell();
        if ( pShell )
        {
            if ( pShell->UseObjectSize() )
                ForceOuterResize_Impl(TRUE);
            DoAdjustPosSizePixel( pShell, Point(), GetWindow().GetOutputSizePixel() );
            if ( pShell->UseObjectSize() )
                ForceOuterResize_Impl(FALSE);

            SfxViewFrame* pActFrame = this;
            while ( pActFrame->GetActiveChildFrame_Impl() )
                pActFrame = pActFrame->GetActiveChildFrame_Impl();
            SvInPlaceClient *pCli = pActFrame->GetViewShell()->GetIPClient();
            if ( pCli && pCli->GetProtocol().IsUIActive() )
            {
                if ( !GetParentViewFrame() )
                    pCli->GetEnv()->GetIPEnv()->DoTopWinResize();
                pCli->GetEnv()->GetIPEnv()->DoDocWinResize();
            }
        }
    }
}

void SfxViewFrame::MiscExec_Impl( SfxRequest& rReq )
{
    DBG_MEMTEST();
    FASTBOOL bDone = FALSE;
    switch ( rReq.GetSlot() )
    {
        case SID_NEXTWINDOW :
        case SID_PREVWINDOW :
        {
            SfxWorkWindow *pWork = GetFrame()->GetWorkWindow_Impl();
            pWork->ActivateNextChild_Impl( rReq.GetSlot() == SID_NEXTWINDOW ? TRUE :FALSE );
            rReq.Done();
            break;
        }

        case SID_TOGGLESTATUSBAR:
        {
            SfxToolBoxConfig *pTbxCfg = SfxToolBoxConfig::GetOrCreate();

            // Parameter auswerten
            SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, rReq.GetSlot(), FALSE);
            BOOL bShow = pShowItem  ? pShowItem->GetValue()
                                    : !pTbxCfg->IsStatusBarVisible();
            pTbxCfg->SetStatusBarVisible( bShow );
            GetFrame()->GetWorkWindow_Impl()->UpdateObjectBars_Impl();
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
                    BOOL bNewFullScreenMode = pItem ? pItem->GetValue() : !pWork->IsFullScreenMode();
                    pWork->ShowFullScreenMode( bNewFullScreenMode );
                    pWork->SetMenuBarMode( bNewFullScreenMode ? MENUBAR_MODE_HIDE : MENUBAR_MODE_NORMAL );
                }
            }

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

    int bSearchedMDI = FALSE;
    int bFoundNormMDI = FALSE;
    int bFoundMiniMDI = FALSE;
    int bFoundNonDesktopMDI = FALSE;

    const USHORT *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges && *pRanges, "Set ohne Bereich");
    while ( *pRanges )
    {
        for(USHORT nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich)
        {
            switch(nWhich)
            {
                case SID_TOGGLESTATUSBAR:
                {
                    rSet.Put( SfxBoolItem( nWhich, SfxToolBoxConfig::GetOrCreate()->IsStatusBarVisible() ) );
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
    BOOL bShow = FALSE;
    BOOL bHasChild = HasChildWindow(nSID);
    bShow = pShowItem ? pShowItem->GetValue() : !bHasChild;

    // ausf"uhren
    if ( !pShowItem || bShow != bHasChild )
        ToggleChildWindow( nSID );

    GetBindings().Invalidate( nSID );
    GetDispatcher()->Update_Impl( TRUE );

    if ( bShow && ( nSID == SID_HELP_PI ) )
        GetpApp()->FocusChanged();  // Hilfe passend zum FocusWindow...

    // ggf. recorden
    if ( !rReq.IsAPI() )
        rReq.AppendItem( SfxBoolItem( nSID, bShow ) );

    rReq.Done();
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
        if ( nSID == SID_HYPERLINK_DIALOG )
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
        else if ( nSID == SID_HELP_PI )
//! (pb) what about help?
            rState.DisableItem(nSID);
        else if ( KnowsChildWindow(nSID) )
            rState.Put( SfxBoolItem( nSID, HasChildWindow(nSID) ) );
        else
            rState.DisableItem(nSID);
    }
}

void SfxViewFrame::ToolboxExec_Impl( SfxRequest &rReq )
{
    // Object-Bar-Id ermitteln
    sal_uInt16 nSID = rReq.GetSlot(), nTbxID;
    switch ( nSID )
    {
        case SID_TOGGLEFUNCTIONBAR:     nTbxID = SFX_OBJECTBAR_APPLICATION; break;
        case SID_TOGGLEOBJECTBAR:       nTbxID = SFX_OBJECTBAR_OBJECT; break;
        case SID_TOGGLETOOLBAR:         nTbxID = SFX_OBJECTBAR_TOOLS; break;
        case SID_TOGGLEMACROBAR:        nTbxID = SFX_OBJECTBAR_MACRO; break;
        case SID_TOGGLEOPTIONBAR:       nTbxID = SFX_OBJECTBAR_OPTIONS; break;
        case SID_TOGGLECOMMONTASKBAR:   nTbxID = SFX_OBJECTBAR_COMMONTASK; break;
        case SID_TOGGLENAVBAR:          nTbxID = SFX_OBJECTBAR_NAVIGATION; break;
        //case SID_TOGGLERECORDINGBAR:  nTbxID = SFX_OBJECTBAR_RECORDING; break;
        //case SID_TOGGLEFULLSCREENBAR: nTbxID = SFX_OBJECTBAR_FULLSCREEN; break;
        default:
            DBG_ERROR( "invalid ObjectBar`s SID" );
    }

    // Parameter auswerten
    SfxToolBoxConfig *pTbxConfig = SfxToolBoxConfig::GetOrCreate();
    SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, nSID, sal_False);
    sal_Bool bShow = pShowItem ? pShowItem->GetValue() : !pTbxConfig->IsToolBoxPositionVisible(nTbxID);

    // ausfuehren
    pTbxConfig->SetToolBoxPositionVisible(nTbxID, bShow);
    GetBindings().Invalidate( nSID );

    SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
    while ( pViewFrame )
    {
        // update all "final" dispatchers
        if ( !pViewFrame->GetActiveChildFrame_Impl() )
            pViewFrame->GetDispatcher()->Update_Impl(sal_True);
        pViewFrame = SfxViewFrame::GetNext(*pViewFrame);
    }

    // ggf. recorden
    if ( !rReq.IsAPI() )
        rReq.AppendItem( SfxBoolItem( nSID, bShow ) );
    rReq.Done();
}

//------------------------------------------------------------------------


void SfxViewFrame::ToolboxState_Impl( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    for ( sal_uInt16 nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        SfxToolBoxConfig *pTbxConfig = SfxToolBoxConfig::GetOrCreate();
        switch ( nSID )
        {
            case SID_TOGGLEFUNCTIONBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_APPLICATION)));
                break;

            case SID_TOGGLEOBJECTBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_OBJECT)));
                break;

            case SID_TOGGLEOPTIONBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_OPTIONS)));
                break;

            case SID_TOGGLETOOLBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_TOOLS)));
                break;

            case SID_TOGGLEMACROBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_MACRO)));
                break;

            case SID_TOGGLECOMMONTASKBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_COMMONTASK)));
                break;

            case SID_TOGGLENAVBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_NAVIGATION)));
                break;

            default:
                DBG_ERROR( "invalid ObjectBar`s SID" );
        }
    }
}

//--------------------------------------------------------------------
SfxWorkWindow* SfxViewFrame::GetWorkWindow_Impl( USHORT nId )
{
    SfxWorkWindow* pWork = 0;
    if ( IsA( TYPE(SfxInPlaceFrame) ) )
    {
        SfxShell* pShell;
        const SfxSlot* pSlot;
        if( !GetDispatcher()->GetShellAndSlot_Impl( nId, &pShell, &pSlot, FALSE, TRUE ) && GetParentViewFrame_Impl() )
            // Containerslot !
            pWork = GetParentViewFrame_Impl()->GetFrame()->GetWorkWindow_Impl();
        else
            pWork = GetFrame()->GetWorkWindow_Impl();
    }
    else
        pWork = GetFrame()->GetWorkWindow_Impl();

    return pWork;
}

void SfxViewFrame::SetChildWindow(USHORT nId, BOOL bOn)
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    if ( pWork )
        pWork->SetChildWindow_Impl( nId, bOn );
}

//--------------------------------------------------------------------

void SfxViewFrame::ToggleChildWindow(USHORT nId)
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    if ( pWork )
        pWork->ToggleChildWindow_Impl( nId );
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
        pWork->ShowChildWindow_Impl(nId, bVisible);
}

//--------------------------------------------------------------------

SfxChildWindow* SfxViewFrame::GetChildWindow(USHORT nId)
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( nId );
    return pWork ? pWork->GetChildWindow_Impl(nId) : NULL;
}

