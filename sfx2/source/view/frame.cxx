/*************************************************************************
 *
 *  $RCSfile: frame.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mba $ $Date: 2000-10-23 12:23:23 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SPLITWIN_HXX //autogen
#include <vcl/splitwin.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <vcl/msgbox.hxx>

#pragma hdrstop

#include "unoctitm.hxx"
#include "frame.hxx"
#include "arrdecl.hxx"
#include "objsh.hxx"
#include "dispatch.hxx"
#include "docfile.hxx"
#include "docfilt.hxx"
#include "frmdescr.hxx"
#include "appdata.hxx"
#include "openflag.hxx"
#include "urlframe.hxx"
#include "viewsh.hxx"
#include "viewfrm.hxx"
#include "request.hxx"
#include "fcontnr.hxx"
#include "impframe.hxx"
#include "appuno.hxx"
#include "topfrm.hxx"
#include "workwin.hxx"
#include "fsetvwsh.hxx"
#include <sfxuno.hxx>
#include <msgpool.hxx>

#ifdef DBG_UTIL
#include "frmhtmlw.hxx"
#endif

static SfxFrameArr_Impl* pFramesArr_Impl=0;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;

TYPEINIT1(SfxFrame, SfxListener);
TYPEINIT1(SfxFrameItem, SfxPoolItem);
TYPEINIT1(SfxUsrAnyItem, SfxPoolItem);

void SfxFrame::SetLoadCancelable_Impl( SfxCancellable* pCbl )
{
    delete pImp->pLoadCancellable;
    pImp->pLoadCancellable = pCbl;
}

void SfxFrame::CopyHistory_Impl( SfxFrame *pFrame ) const
{
    if ( pParentFrame )
    {
        pParentFrame->CopyHistory_Impl( pFrame );
        return;
    }

    pFrame->ClearHistory();
    pFrame->pImp->pHistory = new SfxFrameHistory_Impl;
    SfxFrameHistory_Impl* pSource = pImp->pHistory;
    SfxFrameHistory_Impl* pTarget = pFrame->pImp->pHistory;

    // Zuerst die History bis jetzt kopieren
    sal_uInt32 nCurHisEntry = 0;
    if ( pSource )
    {
        // Die aktuelle Position in der UndoHistory
        nCurHisEntry = pSource->GetCurPos();
        sal_uInt32 nCount = pSource->Count();
        SfxFramePickEntry_Impl* pActEntry = pSource->GetCurObject();
        if( pActEntry )
        {
            GetTopFrame()->GetDescriptor()->GetArgs()->ClearItem( SID_VIEW_DATA );
            pActEntry->Update( GetTopFrame() );
        }

        for ( sal_uInt32 n=0; n<=nCurHisEntry; n++ )
        {
            SfxFramePickEntry_Impl *pEntry = pSource->GetObject(n);
            if ( pEntry->aName.Len() )
                pTarget->Insert( pEntry->Clone(), pTarget->Count() );
        }
    }

    pTarget->Seek( pTarget->Count() - 1 );
}


void SfxFrame::ClearHistory()
{
    SfxFrameHistory_Impl* pHistory = pImp->pHistory;
    if ( pHistory )
    {
        for ( sal_uInt32 n= pHistory->Count(); n--; )
        {
            SfxFramePickEntry_Impl *pEntry = pHistory->GetObject(n);
            delete pEntry;
        }
        delete pImp->pHistory;
        pImp->pHistory = NULL;
    }
}

SfxFrame::SfxFrame(SfxFrame* pParent):
    pParentFrame( pParent ),
    pChildArr(0),
    pUnoImp(0)
{
    pImp = new SfxFrame_Impl( this );
    if ( pParent )
        pParent->InsertChildFrame_Impl( this );
    if ( !pFramesArr_Impl )
        pFramesArr_Impl = new SfxFrameArr_Impl;
    pFramesArr_Impl->Insert( this, pFramesArr_Impl->Count() );
}

SvCompatWeakHdl* SfxFrame::GetHdl()
{
    return pImp->GetHdl();
}

SfxCancelManager* SfxFrame::GetCancelManager() const
{
    SfxFrame *pFrame = GetTopFrame();
    SfxCancelManager*& rpMgr = pFrame->pImp->pCancelMgr;
    if ( !rpMgr )
    {
        rpMgr = new SfxCancelManager( SFX_APP()->GetCancelManager() );
        pFrame->pImp->StartListening( *rpMgr );
    }
    return rpMgr;
}

//--------------------------------------------------------------------

void SfxFrame::Lock_Impl( sal_Bool bLock )
{
    SfxFrame *pParent = pParentFrame; // this kann kaputt gehen!

    if ( bLock )
    {
        // erstes Lock?
        if ( 1 == ++pImp->nLocks )
            SvFactory::IncAliveCount();
    }
    else
    {
        // letztes Lock?
        if ( !--pImp->nLocks )
        {
            if ( pImp->bCloseOnUnlock )
                DoClose();
            SvFactory::DecAliveCount();
        }
    }

    if ( pParent )
        pParent->Lock_Impl(bLock);
}

//--------------------------------------------------------------------

sal_uInt16 SfxFrame::GetLockCount_Impl() const
{
    return pImp->nLocks;
}

//--------------------------------------------------------------------

void SfxFrame::CloseOnUnlock_Impl()
{
    pImp->bCloseOnUnlock = sal_True;
}

//--------------------------------------------------------------------

SfxFrame::~SfxFrame()
{
    pFramesArr_Impl->Remove( pFramesArr_Impl->GetPos( this ) );

    if ( pParentFrame )
    {
        pParentFrame->RemoveChildFrame_Impl( this );
        pParentFrame = 0;
    }

    for ( sal_uInt32 n=0; n<pImp->aHistory.Count(); n++ )
    {
        SfxFramePickEntry_Impl *pEntry = pImp->aHistory.GetObject(n);
        delete pEntry;
    }

    if ( pImp->pHistory )
    {
        for ( sal_uInt32 n=0; n<pImp->pHistory->Count(); n++ )
        {
            SfxFramePickEntry_Impl *pEntry = pImp->pHistory->GetObject(n);
            delete pEntry;
        }
        delete pImp->pHistory;
    }

    // Nur TopLevel-Frames verwalten ihren Descriptor selbst, bei den anderen
    // tut es das Frameset
    if ( pImp->pDescr && !pImp->pDescr->GetParent() )
        delete pImp->pDescr;

    if ( pChildArr )
    {
        DBG_ASSERT( !pChildArr->Count(), "Childs nicht entfernt!" );
        delete pChildArr;
    }

    delete pImp;
}

sal_Bool SfxFrame::DoClose()
{
    // Eigentlich wird noch ein PrepareClose gebraucht !!!
    if ( !pImp->bClosing )
    {
        // wenn jetzt gelockt, dann sp"ater wiederholen
        if ( IsLocked_Impl() )
        {
            CloseOnUnlock_Impl();
            return sal_False;
        }

        pImp->bClosing = sal_True;
        CancelTransfers();

        // Bei FrameSets verhindern, da\s das Closen der Childs zu st"andigen
        // ReCalcs im SplitWindow f"uhrt; SetUpdateMode am FrameWindow wirkt
        // leider nicht auf dem MAC
        Window *pWin = NULL;
        SfxViewShell *pViewSh;
        if ( pImp->pCurrentViewFrame && 0 != ( pViewSh = pImp->pCurrentViewFrame->GetViewShell() ) )
        {
            pWin = pViewSh->GetWindow();
            if ( pWin )
            {
                if ( pWin->GetType() ==  RSC_SPLITWINDOW )
                    ((SplitWindow*)pWin)->SetUpdateMode( sal_False );
                pWin->Hide();
            }
        }

        sal_Bool bRet = sal_True;
        SfxBindings* pBindings = NULL;
        if ( pImp->pCurrentViewFrame )
            pBindings = &pImp->pCurrentViewFrame->GetBindings();

        // Bei internen Tasks m"ussen Controller und Tools abger"aumt werden
        if ( pImp->pWorkWin )
        {
            pImp->pWorkWin->DeleteControllers_Impl();
            DELETEZ( pImp->pWorkWin );
        }

        if ( pImp->pCurrentViewFrame )
            bRet = pImp->pCurrentViewFrame->Close();

        if ( pImp->bOwnsBindings )
            delete pBindings;

        // now close frame; it will be deleted if this call is successful, so don't use any members after that!
        Reference < XFrame > xFrame( pImp-> xFrame );
        bRet = Close();

        if ( bRet )
        {
            if ( xFrame.is() )
                xFrame->dispose();
        }
        else
        {
            if ( pWin )
            {
                if ( pWin->GetType() ==  RSC_SPLITWINDOW )
                {
                    ((SplitWindow*)pWin)->SetUpdateMode( sal_True );
                    ((SplitWindow*)pWin)->Show();
                }
                else
                    pWin->Show();
            }

            pImp->bClosing = sal_False;
        }

        return bRet;
    }

    return sal_True;
}

void SfxFrame::Clear_Impl()
{
    CancelTransfers();

    // Bei FrameSets verhindern, da\s das Closen der Childs zu st"andigen
    // ReCalcs im SplitWindow f"uhrt; SetUpdateMode am FrameWindow wirkt
    // leider nicht auf dem MAC
    Window *pWin = NULL;
    SfxViewShell *pViewSh;
    if ( pImp->pCurrentViewFrame &&
            0 != ( pViewSh = pImp->pCurrentViewFrame->GetViewShell() ) )
    {
        pWin = pViewSh->GetWindow();
        if ( pWin )
        {
            if ( pWin->GetType() ==  RSC_SPLITWINDOW )
                ((SplitWindow*)pWin)->SetUpdateMode( sal_False );
            pWin->Hide();
        }
    }

    sal_Bool bRet = sal_True;
    SfxBindings* pBindings = NULL;
    if ( pImp->pCurrentViewFrame )
    {
        pBindings = &pImp->pCurrentViewFrame->GetBindings();
        pImp->bClosing = sal_True;
        bRet = pImp->pCurrentViewFrame->Close();
        pImp->bClosing = sal_False;
    }

    if ( bRet )
    {
        // Bei internen Tasks m"ussen Controller und Tools abger"aumt werden
        if ( pImp->pWorkWin )
        {
            pImp->pWorkWin->DeleteControllers_Impl();
            DELETEZ( pImp->pWorkWin );
        }

        if ( pImp->bOwnsBindings )
            delete pBindings;
    }
    else
    {
        if ( pWin )
        {
            if ( pWin->GetType() ==  RSC_SPLITWINDOW )
            {
                ((SplitWindow*)pWin)->SetUpdateMode( sal_True );
                ((SplitWindow*)pWin)->Show();
            }
            else
                pWin->Show();
        }
    }
}


sal_Bool SfxFrame::DocIsModified_Impl()
{
    if ( pImp->pCurrentViewFrame && pImp->pCurrentViewFrame->GetObjectShell() &&
            pImp->pCurrentViewFrame->GetObjectShell()->IsModified() )
        return sal_True;
    for( sal_uInt16 nPos = GetChildFrameCount(); nPos--; )
        if( pChildArr->GetObject( nPos )->DocIsModified_Impl() )
            return sal_True;
    return sal_False;
}

sal_uInt16 SfxFrame::PrepareClose_Impl( sal_Bool bUI, sal_Bool bForBrowsing )
{
    sal_uInt16 nRet = RET_OK;

    // prevent recursive calls
    if( !pImp->bPrepClosing )
    {
        pImp->bPrepClosing = sal_True;

        SfxObjectShell* pCur = GetCurrentDocument() ;
        if( pCur )
        {
            // SFX components have a known behaviour
            // First check if this frame is the only view to its current document
            int bOther = sal_False;
            for ( const SfxViewFrame *pFrame = SfxViewFrame::GetFirst( pCur );
                    !bOther && pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, pCur ) )
            {
                bOther = ( pFrame->GetFrame() != this );
            }

            if ( bOther )
                // if there are other views only the current view of this frame must be asked
                nRet = GetCurrentViewFrame()->GetViewShell()->PrepareClose( bUI, bForBrowsing );
            else
                // otherwise ask the document
                nRet = pCur->PrepareClose( bUI, bForBrowsing );
        }

        if ( nRet == RET_OK )
        {
            // if this frame has child frames, ask them too
            for( USHORT nPos = GetChildFrameCount(); nRet == RET_OK && nPos--; )
                nRet = pChildArr->GetObject( nPos )->PrepareClose_Impl( bUI, bForBrowsing );
        }

        pImp->bPrepClosing = sal_False;
    }

    if ( nRet == RET_OK && pImp->pWorkWin )
        // if closing was accepted by the component the UI subframes must be asked also
        nRet = pImp->pWorkWin->PrepareClose_Impl();

    return nRet;
}

//--------------------------------------------------------------------

SfxFrame* SfxFrame::GetChildFrame( sal_uInt16 nPos ) const
{
    if ( pChildArr && pChildArr->Count() > nPos )
    {
        DBG_ASSERT( nPos < pChildArr->Count(), "Falscher Index!")
        return (*pChildArr)[nPos];
    }

    return 0L;
}

void SfxFrame::InsertChildFrame_Impl( SfxFrame* pFrame, sal_uInt16 nPos )
{
    if ( !pChildArr )
        pChildArr = new SfxFrameArr_Impl;
    pChildArr->Insert( pFrame, nPos );
    pFrame->pParentFrame = this;
}

void SfxFrame::RemoveChildFrame_Impl( sal_uInt16 nPos )
{
    DBG_ASSERT( pChildArr, "Unbekannter Frame!");
    pChildArr->Remove( nPos );
};

void SfxFrame::RemoveChildFrame_Impl( SfxFrame* pFrame )
{
    DBG_ASSERT( pChildArr, "Unbekannter Frame!");
    sal_uInt16 nPos = pChildArr->GetPos(pFrame);
    RemoveChildFrame_Impl( nPos );
};

sal_Bool SfxFrame::CloseChildFrames()
{
    sal_Bool bRet = sal_True;
    if ( pChildArr )
    {
        // Childs closen
        sal_uInt16 nCount = pChildArr->Count();
        for ( sal_uInt16 n=nCount; n>0; n--)
        {
            SfxFrame* pFrame = (*pChildArr)[n-1];
            // pFrame removed sich selbst
            bRet = pFrame->DoClose();
            if ( !bRet )
                break;
        }
    }

    return bRet;
}

SfxFrame* SfxFrame::SearchChildrenForName_Impl(
    const String& rName, sal_Bool bDeep ) const
{
    if ( pChildArr )
    {
        sal_uInt16 nCount = pChildArr->Count();
        for ( sal_uInt16 n=nCount; n>0; n--)
        {
            SfxFrame* pFrame = (*pChildArr)[n-1];
            if ( rName.CompareIgnoreCaseToAscii( pFrame->GetFrameName() ) == COMPARE_EQUAL )
                return pFrame;
            if( bDeep )
            {
                pFrame = pFrame->SearchChildrenForName_Impl( rName );
                if ( pFrame )
                    return pFrame;
            }
        }
    }

    return NULL;
}

SfxFrame* SfxFrame::GetTopFrame() const
{
    SfxFrame *pParent = (SfxFrame*) this;
    while ( pParent->pParentFrame )
        pParent = pParent->pParentFrame;
    return pParent;
}

SfxFrame* SfxFrame::SearchFrame( const String& rName, SfxMedium* pMedium )
{
    // Weil Netscape f"uhrende BLANKS "uberliest, m"ussen wir das wohl auch
    // Sollte besser auch bei den FrameNames gemacht werden!
    String aName( rName );
    aName.EraseLeadingChars();

    SfxFrame *pFrame = this;
    if( !aName.Len() || aName.CompareIgnoreCaseToAscii("_self") == COMPARE_EQUAL )
    {
        return pFrame;
    }
    else if (  aName.CompareIgnoreCaseToAscii("_smartself") == COMPARE_EQUAL )
    {
        DBG_ERROR("Not supported!");
        return pFrame;
    }
    else if ( aName.CompareIgnoreCaseToAscii( GetFrameName() ) == COMPARE_EQUAL)
    {
        // Eigener Name, kein Name oder Selbstbezug
        return pFrame;
    }
    else if ( aName.CompareIgnoreCaseToAscii("_parent") == COMPARE_EQUAL )
    {
        // Gesucht ist das Parent Frameset
        return pParentFrame ? pParentFrame : this;
    }
    else if ( aName.CompareIgnoreCaseToAscii("_blank") == COMPARE_EQUAL )
    {
        return NULL;
    }
    else if ( aName.CompareIgnoreCaseToAscii("_top") == COMPARE_EQUAL )
    {
        while ( pFrame->GetParentFrame() )
            pFrame = pFrame->GetParentFrame();
        return pFrame;
    }
    else
    {
        // Zuerst unterhalb dieses Frames absuchen
        pFrame = SearchChildrenForName_Impl( aName );
    }

    if ( !pFrame && GetParentFrame() )
    {
        // Noch nichts gefunden, nach oben weitergehen
        // Eigentlich m"u\sten Frames, die schon abgesucht wurden, aus
        // Performance-Gr"unden markiert werden
        SfxFrame *pParent = GetParentFrame();
        do
        {
            // Ist es der Parent ?
            if ( aName.CompareIgnoreCaseToAscii( pParent->GetFrameName() ) == COMPARE_EQUAL )
                return pParent;

            if ( pParent->pImp->pDescr->GetFrameSet() )
            {
                // Wenn es ein Parent Frameset gibt, dort suchen
                pFrame = pParent->SearchChildrenForName_Impl( aName );
                if ( pFrame )
                    return pFrame;
            }

            // Weiter nach oben
            pParent = pParent->GetParentFrame();
        }
        while ( pParent );
    }

    if( !pFrame )
    {
        SfxFrameArr_Impl& rArr = *SFX_APP()->Get_Impl()->pTopFrames;
        // keinen Frame im MDI-Fenster gefunden. Jetzt die anderen
        // Toplevel Frames checken.
        SfxFrame *pTop = GetTopFrame();
        for( sal_uInt16 nPos = rArr.Count(); nPos--; )
        {
             // Unser Topframe wurde bereits durchsucht
            SfxFrame* pCurFrame = rArr[ nPos ];
            if(  pCurFrame != pTop )
            {
                if( aName.CompareIgnoreCaseToAscii( pCurFrame->GetFrameName() ) ==
                    COMPARE_EQUAL )
                    return pCurFrame;
                else
                {
                    if( pFrame =
                        pCurFrame->SearchChildrenForName_Impl( aName ) )
                        return pFrame;
                }
            }
        }
    }

    return pFrame;
}

sal_Bool SfxFrame::IsClosing_Impl() const
{
    return pImp->bClosing;
}

void SfxFrame::DocumentInserted( SfxObjectShell* pDoc )
{
}

sal_uInt16 SfxFrame::GetChildFrameCount() const
{
    return pChildArr ? pChildArr->Count() : 0;
}

sal_Bool SfxFrame::InsertDocument( SfxObjectShell *pDoc )
/*  [Beschreibung]

    Mu\s von abgeleiteten Klassen gerufen werden, liefert ggf. auch sal_False!
    */

    {
        // am drucken oder so?
        if ( IsLocked_Impl() )
            return sal_False;

        // Falls die aktuelle ::com::sun::star::sdbcx::View oder die TopFrame-::com::sun::star::sdbcx::View parent eines
        // ModalDialogs ist, darf sie nicht entfernt werden
        SfxObjectShell* pCur = GetCurrentDocument();
        if ( pCur && ( pCur->IsInModalMode() || pCur->IsInPrepareClose() ) )
            return sal_False;
        DocumentInserted( pDoc );
        return sal_True;
    }

void SfxFrame::SetLoadEnvironment_Impl( LoadEnvironment_Impl* pEnv )
{
    if ( pImp->pLoadEnv )
        pImp->pLoadEnv->EndListening( *pImp );

    pImp->pLoadEnv = pEnv;

    if ( pImp->pLoadEnv && !pImp->pLoadEnv->IsListening( *pImp ) )
        pImp->pLoadEnv->StartListening( *pImp );
}

LoadEnvironment_Impl* SfxFrame::GetLoadEnvironment_Impl() const
{
    return pImp->pLoadEnv;
}

void SfxFrame::CancelTransfers( sal_Bool bCancelLoadEnv )
{
    if( !pImp->bInCancelTransfers )
    {
        pImp->bInCancelTransfers = sal_True;
#if SUPD<582
        SfxJS* pJS = GetJavaScript( sal_False );
        if( pJS )
        {
            JScriptDocumentManager * pDM = pJS->GetDocumentManager( sal_False );
            if( pDM ) pDM->StopNoneRecursiveCalls();
        }
#endif
        SfxObjectShell* pObj = GetCurrentDocument();
        if( pObj ) //&& !( pObj->Get_Impl()->nLoadedFlags & SFX_LOADED_ALL ))
        {
            SfxViewFrame* pFrm;
            for( pFrm = SfxViewFrame::GetFirst( pObj );
                 pFrm && pFrm->GetFrame() == this;
                 pFrm = SfxViewFrame::GetNext( *pFrm, pObj ) );
            // Keine anderer Frame mehr auf Doc -> Cancel
            if( !pFrm ) pObj->CancelTransfers();
        }

        // zuerst Nachladende Frames stoppen
        SfxURLFrame* pURLFrame = PTR_CAST( SfxURLFrame, this );
        if( pURLFrame )
            pURLFrame->CancelActivate_Impl();
        sal_uInt16 nCount = GetChildFrameCount();
        for( sal_uInt16 n = 0; n<nCount; n++ )
            GetChildFrame( n )->CancelTransfers();

        //  ggf. StarOne-Loader canceln
        SfxFrameWeak wFrame( this );
        if( wFrame.Is() && pImp->pLoadEnv && bCancelLoadEnv )
            pImp->pLoadEnv->CancelTransfers();

        if (wFrame.Is())
            pImp->bInCancelTransfers = sal_False;
    }
}

SfxViewFrame* SfxFrame::GetCurrentViewFrame() const
{
    return pImp->pCurrentViewFrame;
}

SfxDispatcher* SfxFrame::GetDispatcher_Impl() const
{
    if ( pImp->pCurrentViewFrame )
        return pImp->pCurrentViewFrame->GetDispatcher();
    return GetParentFrame()->GetDispatcher_Impl();
}

SfxObjectShell* SfxFrame::GetCurrentDocument() const
{
    return pImp->pCurrentViewFrame ?
            pImp->pCurrentViewFrame->GetObjectShell() :
            pImp->pCurrentObjectShell;
}

void SfxFrame::SetCurrentDocument_Impl( SfxObjectShell *pDoc )
{
    pImp->pCurrentObjectShell = pDoc;
}

void SfxFrame::SetCurrentViewFrame_Impl( SfxViewFrame *pFrame )
{
    pImp->pCurrentViewFrame = pFrame;
}

sal_uInt32 SfxFrame::GetFrameType() const
{
    return pImp->nType;
}

void SfxFrame::SetFrameType_Impl( sal_uInt32 n )
{
    pImp->nType = n;
}

sal_Bool SfxFrame::ExecuteHistoryMenu_Impl( sal_uInt16 nWhich, const Rectangle& rRect,
    sal_uInt16 nFlags )
{
    SfxFrameHistory_Impl* pHistory = pImp->pHistory;
    if ( !pHistory || !pHistory->Count() )
        return sal_False;

    PopupMenu *pMenu = new PopupMenu;
    long nPos = (long) pHistory->GetCurPos();
    if ( nWhich == SID_BROWSE_FORWARD )
    {
        for ( long n=nPos+1; n<(long)pHistory->Count(); n++ )
        {
            SfxFramePickEntry_Impl *pEntry = pHistory->GetObject( n );
            String aEntry( n-nPos-1 );
            aEntry += ' ';
            aEntry += pEntry->aName;
            pMenu->InsertItem( n+1, aEntry );
        }
    }
    else
    {
        for ( long n=nPos-1; n>=0; n-- )
        {
            SfxFramePickEntry_Impl *pEntry = pHistory->GetObject( n );
            String aEntry( nPos-n-1 );
            aEntry += ' ';
            aEntry += pEntry->aName;
            pMenu->InsertItem( n+1, aEntry );
        }
    }

    sal_uInt16 nRet = pMenu->Execute( &GetWindow(), rRect, nFlags ) - 1;
    if ( nRet != MENU_ITEM_NOTFOUND )
        Browse( nWhich == SID_BROWSE_FORWARD,
            (sal_uInt16) ( nRet > nPos ? nRet-nPos : nPos - nRet ) );
    return sal_True;
}

sal_Bool SfxFrame::Browse( sal_Bool bForward, sal_uInt16 nSteps, sal_Bool bNewFrame )
{
    SfxFrameHistory_Impl* pHistory = pImp->pHistory;
    if ( !pHistory || !pHistory->Count() )
    {
        // Die Undo-History wird immer am TopLevel-Frame gef"uhrt; hier mu\s
        // noch definiert werden, was "BackInFrame" bedeuten soll (eigene Methode)
        return sal_False;
    }

    if ( bNewFrame )
        return sal_False;

    long nPos = (long) pHistory->GetCurPos();
    long nCount = (long) pHistory->Count();
    long nNextPos = nPos + ( bForward ? (long) nSteps : - (long) nSteps );
    sal_uInt16 nMode = bForward ? BROWSE_FORWARD : BROWSE_BACKWARD;
    nMode += ( nSteps << 4 );
    if ( nSteps && nNextPos >= 0L && nNextPos < nCount )
    {
        SfxFrame *pFrame = this;
        if ( bNewFrame )
        {
            SfxFrame* pNew = SfxTopFrame::Create();

            // Die History komplett kopieren
            pNew->pImp->pHistory = new SfxFrameHistory_Impl;
            SfxFrameHistory_Impl& rTarget = *pNew->pImp->pHistory;
            sal_uInt32 nCount = pHistory->Count();
            SfxFramePickEntry_Impl* pActEntry = pHistory->GetCurObject();
            if( pActEntry )
            {
                pFrame->GetDescriptor()->GetArgs()->ClearItem( SID_VIEW_DATA );
                pActEntry->Update( pFrame );
            }

            for ( sal_uInt32 n=0; n<nCount; n++ )
            {
                SfxFramePickEntry_Impl *pEntry = pHistory->GetObject(n);
                if ( pEntry->aName.Len() )
                    rTarget.Insert( pEntry->Clone(), n );
            }

            rTarget.Seek( pHistory->GetCurPos() );
            pFrame = pNew;
        }

        SfxFramePickEntry_Impl *pEntry = pFrame->pImp->pHistory->GetObject( nNextPos );
        pFrame->ActivatePickEntry_Impl( pEntry, nMode );
        return sal_True;
    }

    return sal_False;
}

void SfxFrame::UpdatePickEntries()
{

    SfxFrame *pParent = this;
    while ( pParent->pParentFrame )
        pParent = pParent->pParentFrame;

    // ViewData clearen, damit sie neu geholt wird
    GetDescriptor()->GetArgs()->ClearItem( SID_VIEW_DATA );
    SfxFrameHistory_Impl* pParentHistory = pParent->pImp->pHistory;
    if( pParentHistory )
    {
        SfxFramePickEntry_Impl* pActEntry = pParentHistory->GetCurObject();
        if( pActEntry )
        {
            Deactivate_Impl();
            pActEntry->Update( GetTopFrame() );
        }
    }

    SfxObjectShell *pDoc = GetCurrentDocument();
    if ( !pDoc )
        return;

    SfxFrameHistory_Impl& rHistory = pImp->aHistory;
    SfxFramePickEntry_Impl* pActEntry = pImp->aHistory.GetCurObject();
    if( pActEntry )
        pActEntry->Update( this );
}

void SfxFrame::UpdatePickEntries( const ::com::sun::star::uno::Any& rValue )
{
    SfxFrame *pParent = this;
    while ( pParent->pParentFrame )
        pParent = pParent->pParentFrame;

    // ViewData setzen, nicht mehr holen
    SfxPoolItem *pItem = new SfxUsrAnyItem( SID_VIEW_DATA, rValue );
    SfxFrameHistory_Impl* pParentHistory = pParent->pImp->pHistory;
    if( pParentHistory )
    {
        SfxFramePickEntry_Impl* pActEntry = pParentHistory->GetCurObject();
        if( pActEntry )
        {
            Deactivate_Impl();
            pActEntry->Update( GetTopFrame(), pItem );
        }
    }

    SfxObjectShell *pDoc = GetCurrentDocument();
    if ( !pDoc )
        return;

    SfxFrameHistory_Impl& rHistory = pImp->aHistory;
    SfxFramePickEntry_Impl* pActEntry = pImp->aHistory.GetCurObject();
    if( pActEntry )
        pActEntry->Update( this, pItem );

    delete pItem;
}

void SfxFrame::UpdateUndoHistory_Impl( SfxObjectShell *pDocSh, const String* pNew, const String* pTitle )
{
    if (  pDocSh->GetCreateMode() != SFX_CREATE_MODE_STANDARD )
        // Keine OLE-Objekte etc.
        return;

    SfxItemSet* pSet = pDocSh->GetMedium()->GetItemSet();
    String aNew;
    if( pNew )
        aNew = *pNew;
    else
        aNew = pDocSh->GetMedium()->GetOrigURL();

    // Die Undo-History wird immer am TopLevel-Frame gef"uhrt
    // Zusaetzlich wird pro Frame eine History gefuehrt
    SfxFrame *pParent = this;
    while ( pParent->pParentFrame )
        pParent = pParent->pParentFrame;

    sal_uInt16 nBrowseMode = BROWSE_NORMAL;
    if( pSet )
    {
        SFX_ITEMSET_ARG( pSet, pBrowseMode, SfxUInt16Item, SID_BROWSEMODE, sal_False );
        if( pBrowseMode )
            nBrowseMode = pBrowseMode->GetValue();
    }

    if( nBrowseMode != NO_BROWSE )
    {
        SfxObjectShell *pDoc = GetCurrentDocument();
        SfxFrameHistory_Impl*& pParentHistory = pParent->pImp->pHistory;
        if( !pParentHistory )
        {
            pParentHistory = new SfxFrameHistory_Impl;
            nBrowseMode = BROWSE_NORMAL;
        }

        sal_uInt16 nSteps = nBrowseMode >> 4;
        nBrowseMode &= 0x000F;

        if ( nBrowseMode != BROWSE_BACKWARD && nBrowseMode != BROWSE_FORWARD )
        {
            // Nur Entry anlegen, wenn bisher leer, oder kein NoName-Dokument
            // und entweder neue DocShell oder neuer lokaler Sprung
            if ( !pDoc || pDoc->HasName() &&
                ( pDoc != pDocSh || pDoc->GetMedium()->GetOrigURL() != aNew ) )
            {
                SfxFramePickEntry_Impl* pNewEntry =
                    new SfxFramePickEntry_Impl;
                pNewEntry->Initialize( GetTopFrame(), sal_False, pDocSh, pNew, pTitle );
                pParent->pImp->AppendPickEntry( pNewEntry );
                pParentHistory->Seek( pParentHistory->Count() - 1 );

                pImp->nHasBrowser = SFX_BEAMER_OFF;
                SFX_ITEMSET_ARG( pSet, pBrowserItem, SfxUInt16Item,
                                        SID_BROWSER, sal_False);
                if ( pBrowserItem )
                    pImp->nHasBrowser = pBrowserItem->GetValue();
            }
        }
        else
        {
            pParentHistory->Seek(
                pParentHistory->GetCurPos() +
                ( nBrowseMode == BROWSE_FORWARD ? (long) nSteps : - (long) nSteps ));

            SfxFramePickEntry_Impl* pEntry = pParentHistory->GetCurObject();

            DBG_ASSERT( pEntry, "Kein History-Entry!?" );
        }

        SfxViewFrame* pViewFrame = pParent->GetCurrentViewFrame();
        if( pViewFrame )
        {
            SfxBindings& rBindings = pViewFrame->GetBindings();
            rBindings.Invalidate( SID_BROWSE_FORWARD );
            rBindings.Invalidate( SID_BROWSE_BACKWARD );
        }
    }
}

void SfxFrame::UpdateCurrentHistory_Impl( SfxObjectShell *pDocSh, const String* pNew  )
{
    if (  pDocSh->GetCreateMode() != SFX_CREATE_MODE_STANDARD )
        // Keine OLE-Objekte etc.
        return;

    String aNew;
    if( pNew )
        aNew = *pNew;
    else
        aNew = pDocSh->GetMedium()->GetOrigURL();

    // Die Undo-History wird immer am TopLevel-Frame gef"uhrt
    // Zusaetzlich wird pro Frame eine History gefuehrt
    SfxFrame *pParent = this;
    while ( pParent->pParentFrame )
        pParent = pParent->pParentFrame;

    SfxObjectShell *pDoc = GetCurrentDocument();
    sal_uInt16 nBrowseMode = BROWSE_NORMAL;
    SFX_ITEMSET_ARG( pDocSh->GetMedium()->GetItemSet(), pBrowseMode,
                     SfxUInt16Item, SID_BROWSEMODE, sal_False );
    if( pBrowseMode )
        nBrowseMode = pBrowseMode->GetValue();

    if( nBrowseMode != BROWSE_FRAME )
    {
        // Nur Entry anlegen, wenn bisher leer, oder kein NoName-Dokument
        // und entweder neue DocShell oder neuer lokaler Sprung
        if ( !pDoc || pDoc->HasName() &&
            ( pDoc != pDocSh || pDoc->GetMedium()->GetOrigURL() != aNew ) )
        {
            SfxFrameHistory_Impl& rHistory = pImp->aHistory;
            sal_uInt32 nCurHisEntry = rHistory.GetCurPos();
            SfxFramePickEntry_Impl* pNewPickEntry =
                new SfxFramePickEntry_Impl;
            pNewPickEntry->Initialize( this, sal_False, pDocSh, pNew );

            // Anzahl beschr"anken
            if ( rHistory.Count() == PICKLIST_MAXSIZE )
                delete rHistory.Remove( rHistory.Count() - 1 );

            // Eintraege ueber dem aktuellen Eintrag werden geloescht
            if ( LIST_ENTRY_NOTFOUND != nCurHisEntry )
            {
                for ( sal_uInt32 nEntry = 0; nEntry < nCurHisEntry; ++nEntry )
                    delete rHistory.Remove( 0UL );
            }

            // den neuen einf"ugen
            rHistory.Insert( pNewPickEntry, 0UL );
            rHistory.Seek( 0UL );
            nCurHisEntry = 0;
        }
    }

    if ( nBrowseMode != BROWSE_BACKWARD &&
         nBrowseMode != BROWSE_FORWARD &&
         nBrowseMode != NO_BROWSE)
    {
        if ( pParent == this )
            SfxPickList_Impl::Get()->InsertToHistory( pDocSh );
    }
}

void SfxFrame::UpdateHistory( SfxObjectShell *pDocSh, const String* pNew  )
{
    String aNew;
    if( pNew )
        aNew = *pNew;
    else
        aNew = pDocSh->GetMedium()->GetOrigURL();

    UpdateUndoHistory_Impl( pDocSh, &aNew );
    UpdateCurrentHistory_Impl( pDocSh, &aNew );
}

void SfxFrame::UpdateHistory( const ::rtl::OUString& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs, const ::rtl::OUString& rTitle  )
{
    String aStr( aURL );
    String aTitle( rTitle );
    SfxObjectShell *pDoc = GetCurrentDocument();
    if ( pDoc )
    {
        pDoc->GetMedium()->GetItemSet()->ClearItem( SID_BROWSEMODE );
        UpdateUndoHistory_Impl( pDoc, &aStr, &aTitle );
    }
}

void SfxFrame::GetViewData_Impl()
{
    // Alle zwischen Laden und Entfernen "anderbaren Daten aktualisieren; die
    // festen Daten werden nur einmal ( nach InsertDocument in UpdateDescriptor )
    // geholt, um Zeit zu sparen.
    SfxViewFrame* pViewFrame = GetCurrentViewFrame();
    if( pViewFrame && pViewFrame->GetViewShell() )
    {
        const SfxMedium *pMed = GetCurrentDocument()->GetMedium();
        sal_Bool bReadOnly = pMed->GetOpenMode() == SFX_STREAM_READONLY;
        GetDescriptor()->SetReadOnly( bReadOnly );

        String aUserData;
        pViewFrame->GetViewShell()->WriteUserData( aUserData, sal_True );
        SfxItemSet *pSet = GetDescriptor()->GetArgs();
        pSet->Put( SfxStringItem( SID_USER_DATA, aUserData ));
        sal_Bool bGetViewData = sal_False;
        if ( GetController().is() && pSet->GetItemState( SID_VIEW_DATA ) != SFX_ITEM_SET )
        {
            ::com::sun::star::uno::Any aData = GetController()->getViewData();
            pSet->Put( SfxUsrAnyItem( SID_VIEW_DATA, aData ) );
            bGetViewData = sal_True;
        }

        if ( pViewFrame->GetCurViewId() )
            pSet->Put( SfxUInt16Item( SID_VIEW_ID, pViewFrame->GetCurViewId() ) );
        if ( pChildArr )
        {
            // Bei Framesets m"ussen auch die Daten der ChildViews geholt werden
            sal_uInt16 nCount = pChildArr->Count();
            for ( sal_uInt16 n=nCount; n>0; n--)
            {
                SfxFrame* pFrame = (*pChildArr)[n-1];
                if ( bGetViewData )
                    pFrame->GetDescriptor()->GetArgs()->ClearItem( SID_VIEW_DATA );
                pFrame->GetViewData_Impl();
            }
        }
    }
}

void SfxFrame::UpdateDescriptor( SfxObjectShell *pDoc )
{
    // Beim InsertDocument wird der Descriptor des Frames aktualisiert
    // und sein ItemSet neu initialisiert. Alle Daten, die f"ur ein sp"ateres
    // Restaurieren der ::com::sun::star::sdbcx::View n"otig sind, sind damit festgehalten.
    // Soll das Dokument ersetzt werden, wird durch GetViewData_Impl (s.o.)
    // die neueste Information hinzugef"ugt. Alles zusammen wird dann in der
    // Browse-History gesichert. Beim Aktivieren eines solchen FramePickEntry
    // wird das komplette ItemSet und der Descriptor im OpenDoc mitgeschickt.
    // Hier werden nur die festen Eigenschaften gesichert; die "anderbaren werden
    // durch GetViewData geholt ( spart Zeit ).
    DBG_ASSERT( pDoc, "NULL-Document inserted ?!" );

    SfxFrame *pParent = GetParentFrame();
    const SfxMedium *pMed = pDoc->GetMedium();
    GetDescriptor()->SetActualURL( pMed->GetOrigURL() );

    SFX_ITEMSET_ARG( pMed->GetItemSet(), pItem, SfxBoolItem, SID_EDITDOC, sal_False );
    sal_Bool bEditable = ( !pItem || pItem->GetValue() );

    GetDescriptor()->SetEditable( bEditable );

    // FileOpen-Parameter merken
    SfxItemSet* pItemSet = pMed->GetItemSet();
    String aName( pMed->GetName() );
    SFX_ITEMSET_ARG( pItemSet, pPostItem, SfxStringItem, SID_POSTSTRING, sal_False );
    String aPost;
    if(  pPostItem )
        aPost = pPostItem->GetValue();

    const SfxFilter* pFilter = pMed->GetOrigFilter();
    String aFilter;
    if ( pFilter )
        aFilter = pFilter->GetName();

    SFX_ITEMSET_ARG( pItemSet, pRefererItem, SfxStringItem, SID_REFERER, sal_False);
    SFX_ITEMSET_ARG( pItemSet, pOptionsItem, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False);
    SFX_ITEMSET_ARG( pItemSet, pTitle1Item, SfxStringItem, SID_DOCINFO_TITLE, sal_False);
    SFX_ITEMSET_ARG( pItemSet, pTitle2Item, SfxStringItem, SID_FILE_LONGNAME, sal_False);

    SfxItemSet *pSet = GetDescriptor()->GetArgs();

    // Alle alten Items l"oschen
    pSet->ClearItem();

    if ( pRefererItem )
        pSet->Put( *pRefererItem );
    else
        pSet->Put( SfxStringItem( SID_REFERER, String() ) );

    if ( pPostItem )
        pSet->Put( *pPostItem );

    if ( pOptionsItem )
        pSet->Put( *pOptionsItem );

    if ( pTitle1Item )
        pSet->Put( *pTitle1Item );

    if ( pTitle2Item )
        pSet->Put( *pTitle2Item );

    pSet->Put( SfxStringItem( SID_FILTER_NAME, aFilter ));
}

//-------------------------------------------------------------------------

sal_Bool SfxFrame::CanBrowseBackward() const
{
    SfxFrameHistory_Impl* pHistory = pImp->pHistory;
    return  pHistory && pHistory->Count() && pHistory->GetCurPos() > 0;
}

//-------------------------------------------------------------------------

sal_Bool SfxFrame::CanBrowseForward() const
{
    SfxFrameHistory_Impl* pHistory = pImp->pHistory;
    return  pHistory && pHistory->Count() && pHistory->GetCurPos() <
        pHistory->Count() - 1l;
}

//-------------------------------------------------------------------------

void SfxFrame::SetDescriptor( SfxFrameDescriptor *pD )
{
    DBG_ASSERT( pD, "Kein Descriptor!" );
    DBG_ASSERT( pD != pImp->pDescr, "Descriptor ist schon gesetzt!" );

    if ( pImp->pDescr )
    {
        sal_uInt16 nPos = pImp->pDescr->GetItemPos();
        SfxFrameSetDescriptor *pSet = pImp->pDescr->GetParent();

        // Nur TopLevel-Frames verwalten ihren Descriptor selbst, bei den
        // anderen tut es das Frameset
        if ( !pParentFrame )
            delete pImp->pDescr;
        if ( pSet && !pD->GetParent() )
        {
            pSet->RemoveFrame( pImp->pDescr );
            pSet->InsertFrame( pD, nPos );
        }
    }
    pImp->pDescr = pD;
}

//-------------------------------------------------------------------------

SfxFrameDescriptor* SfxFrame::GetDescriptor() const
{
    // On Demand einen FrameDescriptor anlegen; wenn es kein TopLevel-Frame
    // ist, f"uhrt es zu Fehlern, da keine g"ulige Verkettung hergestellt wird

    if ( !pImp->pDescr )
    {
        DBG_ASSERT( !GetParentFrame(), "Kein TopLevel-Frame, aber kein Descriptor!" );
        pImp->pDescr = new SfxFrameDescriptor( NULL );
        if ( GetCurrentDocument() )
            pImp->pDescr->SetURL( GetCurrentDocument()->GetMedium()->GetOrigURL() );
    }
    return pImp->pDescr;
}

sal_Bool SfxFrame::TransferForReplaceInProgress() const
{
    return pImp->pLoadEnv && pImp->pLoadEnv->GetObjectShell() !=
        GetCurrentDocument();
}

//-------------------------------------------------------------------------

void SfxFrame::GetTargetList( TargetList& rList ) const
{
    if ( !GetParentFrame() )
    {
        // Einen Leerstring f"ur 'Kein Target'
        rList.Insert( new String() );
        rList.Insert( new String( DEFINE_CONST_UNICODE( "_top" ) ) );
        rList.Insert( new String( DEFINE_CONST_UNICODE( "_parent" ) ) );
        rList.Insert( new String( DEFINE_CONST_UNICODE( "_blank" ) ) );
        rList.Insert( new String( DEFINE_CONST_UNICODE( "_self" ) ) );
    }

    SfxViewFrame* pView = GetCurrentViewFrame();
    if( pView && pView->GetViewShell() && !pView->GetViewShell()->IsImplementedAsFrameset_Impl() && pChildArr )
    {
        sal_uInt16 nCount = pChildArr->Count();
        for ( sal_uInt16 n=0; n<nCount; n++)
        {
            SfxFrame* pFrame = (*pChildArr)[n];
            if ( pFrame->GetFrameName().Len() )
                rList.Insert( new String (pFrame->GetFrameName()) );
            pFrame->GetTargetList( rList );
        }
    }
}

//-------------------------------------------------------------------------

void SfxFrame::SetFrameId_Impl( sal_uInt16 nId )
{
    pImp->nFrameId = nId;
    if ( GetCurrentViewFrame() )
        GetCurrentViewFrame()->UpdateTitle();
}

sal_uInt16 SfxFrame::GetFrameId_Impl() const
{
    return pImp->nFrameId;
}

void SfxFrame::SetFrameIdName_Impl( const String& rName )
{
    pImp->aFrameIdName = rName;
}

//-------------------------------------------------------------------------

SfxFrame* SfxFrame::SearchFrame_Impl( sal_uInt16 nId, sal_Bool bDeep )
{
    if ( pImp->nFrameId == nId )
        return this;

    if ( pChildArr )
    {
        sal_uInt16 nCount = pChildArr->Count();
        for ( sal_uInt16 n=0; n<nCount; n++)
        {
            SfxFrame* pFrame = (*pChildArr)[n];
            if ( pFrame->pImp->nFrameId == nId )
                return pFrame;
            if ( bDeep )
            {
                pFrame = pFrame->SearchFrame_Impl( nId, sal_True );
                if ( pFrame )
                    return pFrame;
            }
        }
    }

    return NULL;
}

SfxFrame* SfxFrame::SearchFrame_Impl( const String& rName, sal_Bool bDeep )
{
    if ( pImp->aFrameIdName == rName )
        return this;

    if ( pChildArr )
    {
        sal_uInt16 nCount = pChildArr->Count();
        for ( sal_uInt16 n=0; n<nCount; n++)
        {
            SfxFrame* pFrame = (*pChildArr)[n];
            if ( pFrame->pImp->aFrameIdName == rName )
                return pFrame;
            if ( bDeep )
            {
                pFrame = pFrame->SearchFrame_Impl( rName );
                if ( pFrame )
                    return pFrame;
            }
        }
    }

    return NULL;
}

//-------------------------------------------------------------------------

SfxViewFrame* SfxFrame::ActivateChildFrame_Impl()
{
    // Irgendeinen ChildFrame suchen, der aktiviert werden kann
    BOOL bSetFocus = GetCurrentViewFrame() && GetCurrentViewFrame()->GetViewShell()->GetWindow()->HasChildPathFocus( TRUE );
    for ( sal_uInt16 n=0; n<GetChildFrameCount(); n++ )
    {
        // Wenn es ein ViewFrame ohne FrameSet ist, aktivieren
        SfxFrame *pAct = GetChildFrame(n);
        SfxViewFrame *pViewFrm = pAct->GetCurrentViewFrame();
        if ( pViewFrm && !( pAct->GetFrameType() & SFXFRAME_FRAMESET ) &&
                                pViewFrm->GetObjectShell() )
                pViewFrm->MakeActive_Impl( bSetFocus );
        else
            pViewFrm = pAct->ActivateChildFrame_Impl();
        if ( pViewFrm )
            return pViewFrm;
    }

    return NULL;
}

//-------------------------------------------------------------------------

void SfxFrame::Activate_Impl( sal_Bool bBeamerOn )

/*  [Description]

    Internal Activate-Handler, called from derived classes which supports
    Activate-Handling.
*/

{
    if ( GetParentFrame() )
        GetParentFrame()->Activate_Impl( bBeamerOn );
}

SfxBroadcaster& SfxFrame::GetBroadcaster() const
{
    return *pImp;
}

//-------------------------------------------------------------------------

void SfxFrame::Deactivate_Impl()

/*  [Description]

    Internal Deactivate-Handler, called from derived classes which supports
    Activate-Handling.
*/

{
    if( GetParentFrame() )
        GetParentFrame()->Deactivate_Impl();
}

String SfxFrame::GetContent() const
{
    SfxObjectShell *pObj = GetCurrentDocument();
    if ( !pObj || !pObj->GetMedium() )
        return String();
    else
        return pObj->GetMedium()->GetName();
}

sal_Bool SfxFrame::CheckContentForLoad_Impl()
{
    if ( GetCurrentDocument() )
    {
        SfxMedium* pMedium = GetCurrentDocument()->GetMedium();
        SfxItemSet* pSet = pMedium->GetItemSet();
        SfxItemSet* pNew = GetDescriptor()->GetArgs();

        // Falls URLs nicht uebereinstimmen
        if ( INetURLObject( GetDescriptor()->GetActualURL() ) !=
                INetURLObject( pMedium->GetOrigURL() ) )
            return sal_True;

        // Falls die Filter nicht uebereinstimmen
        SFX_ITEMSET_ARG( pNew, pFilterItem, SfxStringItem, SID_FILTER_NAME, sal_False );
        if( pMedium->GetOrigFilter() && pFilterItem &&
            pFilterItem->GetValue() != pMedium->GetOrigFilter()->GetName() )
            return sal_True;

        // Falls die Poststrings nicht uebereinstimmen
        SFX_ITEMSET_ARG( pSet, pPostItem, SfxStringItem, SID_POSTSTRING, sal_False );
        SFX_ITEMSET_ARG( pNew, pNewPostItem, SfxStringItem, SID_POSTSTRING, sal_False );
        SFX_ITEMSET_ARG( pSet, pPost2Item, SfxRefItem, SID_POSTLOCKBYTES, sal_False );
        SFX_ITEMSET_ARG( pNew, pNewPost2Item, SfxRefItem, SID_POSTLOCKBYTES, sal_False );
        if( pPost2Item || pNewPost2Item || (
            pPostItem && pNewPostItem && pNewPostItem->GetValue() != pPostItem->GetValue() ||
            !pPostItem && pNewPostItem || pPostItem && !pNewPostItem ) )
            return sal_True;

        return sal_False;
    }

    return (GetDescriptor()->GetActualURL().GetMainURL().Len() != 0);
}

sal_Bool SfxFrame::IsParent( SfxFrame *pFrame ) const
{
    SfxFrame *pParent = pParentFrame;
    while ( pParent )
    {
        if ( pParent == pFrame )
            return sal_True;
        pParent = pParent->pParentFrame;
    }

    return sal_False;
}

void SfxFrame::InsertTopFrame_Impl( SfxFrame* pFrame )
{
    SfxFrameArr_Impl& rArr = *SFX_APP()->Get_Impl()->pTopFrames;
    rArr.Insert( pFrame, rArr.Count() );
}

void SfxFrame::RemoveTopFrame_Impl( SfxFrame* pFrame )
{
    SfxFrameArr_Impl& rArr = *SFX_APP()->Get_Impl()->pTopFrames;
    rArr.Remove( rArr.GetPos( pFrame ) );
}

//========================================================================

SfxFramePickEntry_Impl::SfxFramePickEntry_Impl()
:   pDescriptor( NULL ),
    pBrowserCfg( 0 ),
    nHasBrowser( SFX_BEAMER_OFF )
{
}

//-------------------------------------------------------------------------
void SfxFrame::ActivatePickEntry_Impl( SfxFramePickEntry_Impl* pEntry,
        sal_uInt16 nMode, SfxFrameDescriptor *pDesc )
{
    SfxAllItemSet aSet( SFX_APP()->GetPool() );

    // Die vollst"andige FrameSet-Beschreibung zur Verf"ugung stellen
    if ( !pDesc )
        pDesc = pEntry->pDescriptor->Clone();

    // Parameter zusammenstellen; Achtung: ActivatePickEntry mu\s an dem Frame
    // aufgerufen werden, in den geladen werden soll!
    aSet.Put( *pEntry->pDescriptor->GetArgs() );
    aSet.Put( SfxStringItem( SID_FILE_NAME, pDesc->GetActualURL().GetMainURL() ) );
    aSet.Put( SfxFrameItem( SID_DOCFRAME, this ) );
    aSet.Put( SfxUInt16Item( SID_BROWSEMODE, nMode ));
    aSet.Put( SfxFrameDescriptorItem( pDesc, SID_FRAMEDESCRIPTOR ) );

    pImp->pCurrentViewFrame->GetDispatcher()->Execute( SID_OPENDOC, SFX_CALLMODE_RECORD, aSet );
    delete pDesc;
}

void SfxFramePickEntry_Impl::Initialize( SfxFrame* pFrameP, sal_Bool bBrowserCfg,
        const SfxObjectShell *pDocShell, const String* pURL, const String* pTitle )
{
    if ( pTitle )
        aName = *pTitle;
    else
    {
        aName = pURL ? *pURL : pDocShell->GetMedium()->GetOrigURL();
    }

    String aMark( pDocShell->GetLastMark_Impl() );
    if ( aMark.Len() )
    {
        aName += '#';
        aName += aMark;
    }

    pDescriptor = pFrameP->GetDescriptor()->Clone();
    SfxItemSet *pSet = pDescriptor->GetArgs();
    pSet->Put( SfxStringItem( SID_DOCINFO_TITLE, pDocShell->GetTitle( SFX_TITLE_CAPTION ) ) );
}

void SfxFramePickEntry_Impl::Update( SfxFrame* pFrame, const SfxPoolItem* pItem )
{
    // Zuerst den Descriptor des Frames auf den Stand bringen;
    SfxFrameDescriptor *pD = pFrame->GetDescriptor();
    SfxObjectShell *pDoc = pFrame->GetCurrentDocument();
    if ( pDoc )
    {
        pFrame->UpdateDescriptor( pDoc );
        if ( pItem )
            pFrame->GetDescriptor()->GetArgs()->Put( *pItem );
        else
            pFrame->GetViewData_Impl();
    }

    // Dann den Descriptor kopieren
    delete pDescriptor;
    pDescriptor = pFrame->GetDescriptor()->Clone();
}

SfxFramePickEntry_Impl* SfxFramePickEntry_Impl::Clone() const
{
    // Achtung: Browser-Config kann momentan nicht kopiert werden!
    SfxFramePickEntry_Impl* pEntry = new SfxFramePickEntry_Impl;
    pEntry->aName = aName;
    if ( pDescriptor )
        pEntry->pDescriptor = pDescriptor->Clone();
    return pEntry;
}

//-------------------------------------------------------------------------

SfxFramePickEntry_Impl::~SfxFramePickEntry_Impl()
{
    delete pDescriptor;
}


SfxFrameItem::SfxFrameItem( sal_uInt16 nWhich, SfxViewFrame *p )
    : SfxPoolItem( nWhich ), pFrame( p ? p->GetFrame() : NULL )
{
    wFrame = pFrame;
}

int SfxFrameItem::operator==( const SfxPoolItem &rItem ) const
{
     return ((SfxFrameItem&)rItem).pFrame == pFrame &&
         ((SfxFrameItem&)rItem).wFrame == wFrame;
}

//--------------------------------------------------------------------

String SfxFrameItem::GetValueText() const
{
    return String();
}



SfxPoolItem* SfxFrameItem::Clone( SfxItemPool *) const
{
    SfxFrameItem* pNew = new SfxFrameItem( wFrame);
    pNew->SetFramePtr_Impl( pFrame );
    return pNew;
}

SfxUsrAnyItem::SfxUsrAnyItem( sal_uInt16 nWhich, const ::com::sun::star::uno::Any& rAny )
    : SfxPoolItem( nWhich )
{
    aValue = rAny;
}

int SfxUsrAnyItem::operator==( const SfxPoolItem &rItem ) const
{
//   return rItem.ISA( SfxUsrAnyItem ) && ((SfxUsrAnyItem&)rItem).aValue == aValue;
    return sal_False;
}

String SfxUsrAnyItem::GetValueText() const
{
    return String();
}

SfxPoolItem* SfxUsrAnyItem::Clone( SfxItemPool *) const
{
    return new SfxUsrAnyItem( Which(), aValue );
}

sal_Bool SfxFrame::BrowseInFrame( int nDelta )
{
    SfxFrameHistory_Impl& rHistory = pImp->aHistory;
    int nPos = (int) rHistory.GetCurPos() - nDelta;
    if( nPos >= 0 && nPos < (int) rHistory.Count() )
    {
        rHistory.Seek( nPos );
        SfxFramePickEntry_Impl *pEntry = rHistory.GetCurObject();

        ActivatePickEntry_Impl( pEntry, BROWSE_FRAME );
        return sal_True;
    }
    return sal_False;
}

SfxFrameIterator::SfxFrameIterator( const SfxFrame& rFrame, sal_Bool bRecur )
    : pFrame( &rFrame )
    , bRecursive( bRecur )
{}

SfxFrame* SfxFrameIterator::FirstFrame()
{
    // GetFirst beginnt die Iteration beim ersten ChildFrame
    return pFrame->GetChildFrame( 0 );
}

SfxFrame* SfxFrameIterator::NextFrame( SfxFrame& rPrev )
{
    // Zuerst auf Kinder testen, wenn Rekursion erw"unscht
    SfxFrame *pRet = NULL;
    if ( bRecursive )
        pRet = rPrev.GetChildFrame( 0 );
    if ( !pRet )
    {
        // Anderenfalls mit den Geschwistern von rPrev weitermachen
        pRet = NextSibling_Impl( rPrev );
    }

    return pRet;
}


SfxFrame* SfxFrameIterator::NextSibling_Impl( SfxFrame& rPrev )
{
    SfxFrame *pRet = NULL;
    if ( &rPrev != pFrame )
    {
        SfxFrameArr_Impl& rArr = *rPrev.pParentFrame->pChildArr;
        sal_uInt16 nPos = rArr.GetPos( &rPrev );
        if ( nPos+1 < rArr.Count() )
            pRet = rArr[nPos+1];

        if ( !pRet && rPrev.pParentFrame->pParentFrame )
            pRet = NextSibling_Impl( *rPrev.pParentFrame );
    }

    return pRet;
}

void SfxFrame::SetFrameName( const String& rName )
{
    if ( pImp->pDescr )
        pImp->pDescr->SetName( rName );
    aName = rName;
    if ( pImp->pCurrentViewFrame )
        pImp->pCurrentViewFrame->UpdateTitle();
    if ( GetFrameInterface().is() )
        GetFrameInterface()->setName( rName );
}

void SfxFrame::BeamerSwitched_Impl( sal_Bool bOn )
{
}

void SfxFrame::BeamerSet_Impl()
{
}

void SfxFrame::LoadFinished_Impl()
{
    SfxURLFrame *pURL = PTR_CAST( SfxURLFrame, this );
    if ( pURL )
        pURL->LoadFinished_Impl();
}

SfxFrame* SfxFrame::findFrame(const ::rtl::OUString& aTargetframename, sal_Int32 nSearchFlags)
{
    // Weil Netscape f"uhrende BLANKS "uberliest, m"ussen wir das wohl auch
    // Sollte besser auch bei den FrameNames gemacht werden!
    String aName( aTargetframename );
    aName.EraseLeadingChars();

    SfxFrame *pFrame = this;

    if ( !aName.Len() && GetParentFrame() && GetParentFrame()->GetCurrentViewFrame()->GetViewShell()->IsImplementedAsFrameset_Impl() )
        return GetParentFrame();

    if( !aName.Len() || aName.CompareIgnoreCaseToAscii("_self") == COMPARE_EQUAL )
        return pFrame;

    if (  aName.CompareIgnoreCaseToAscii("_smartself") == COMPARE_EQUAL )
    {
        DBG_ERROR("Not supported!");
        return pFrame;
    }

    if ( aName.CompareIgnoreCaseToAscii("_parent") == COMPARE_EQUAL )
    {
        // Gesucht ist das Parent Frameset
        return pFrame->GetParentFrame();
    }
    else if ( aName.CompareIgnoreCaseToAscii("_blank") == COMPARE_EQUAL )
    {
        return NULL;

    }
    else if ( aName.CompareIgnoreCaseToAscii("_top") == COMPARE_EQUAL )
    {
        while ( pFrame->GetParentFrame() )
            pFrame = pFrame->GetParentFrame();
        return pFrame;
    }

    if ( ( nSearchFlags & FRAME_SEARCH_SELF ) && aName.CompareIgnoreCaseToAscii( GetFrameName() ) == COMPARE_EQUAL )
    {
        // Eigener Name
        return pFrame;
    }

    if ( nSearchFlags & FRAME_SEARCH_CHILDREN )
    {
        // Zuerst unterhalb dieses Frames absuchen
        pFrame = SearchChildrenForName_Impl( aName );
    }

    if ( ( nSearchFlags & FRAME_SEARCH_CHILDREN ) && !pFrame && GetParentFrame() )
    {
        // Noch nichts gefunden, nach oben weitergehen
        // Eigentlich m"u\sten Frames, die schon abgesucht wurden, aus
        // Performance-Gr"unden markiert werden
        SfxFrame *pParent = GetParentFrame();
        do
        {
            // Ist es der Parent ?
            if ( aName.CompareIgnoreCaseToAscii( pParent->GetFrameName() ) == COMPARE_EQUAL )
                return pParent;

            if ( pParent->pImp->pDescr->GetFrameSet() )
            {
                // Wenn es ein Parent Frameset gibt, dort suchen
                pFrame = pParent->SearchChildrenForName_Impl( aName );
                if ( pFrame )
                    return pFrame;
            }

            // Weiter nach oben
            pParent = pParent->GetParentFrame();
        }
        while ( pParent );
    }

    if( !pFrame )
    {
        SfxFrameArr_Impl& rArr = *SFX_APP()->Get_Impl()->pTopFrames;
        // keinen Frame im MDI-Fenster gefunden. Jetzt die anderen
        // Toplevel Frames checken.
        SfxFrame *pTop = GetTopFrame();
        for( sal_uInt16 nPos = rArr.Count(); nPos--; )
        {
             // Unser Topframe wurde bereits durchsucht
            SfxFrame* pCurFrame = rArr[ nPos ];
            if(  pCurFrame != pTop )
            {
                if( aName.CompareIgnoreCaseToAscii( pCurFrame->GetFrameName() ) == COMPARE_EQUAL )
                    return pCurFrame;
                else
                {
                    if( pFrame =
                        pCurFrame->SearchChildrenForName_Impl( aName ) )
                        return pFrame;
                }
            }
        }
    }

    return pFrame;
}

sal_Bool SfxFrame::IsTop() const
{
    return GetFrameInterface().is() ? GetFrameInterface()->isTop() : sal_False;
}

sal_Bool SfxFrame::HasComponent() const
{
    return sal_False;
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SfxFrame::GetController() const
{
    if ( pImp->pCurrentViewFrame && pImp->pCurrentViewFrame->GetViewShell() )
        return pImp->pCurrentViewFrame->GetViewShell()->GetController();
    else
        return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > ();
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >  SfxFrame::GetFrameInterface() const
{
    return pImp->xFrame;
}

void SfxFrame::SetFrameInterface_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame )
{
    pImp->xFrame = rFrame;
}

void SfxFrame::SetItemSet_Impl( const SfxItemSet* pSet )
{
    pImp->pSet = pSet;
}

const SfxItemSet* SfxFrame::GetItemSet_Impl()
{
    return pImp->pSet;
}

void SfxFrame::Appear()
{
    if ( GetCurrentViewFrame() )
    {
        GetCurrentViewFrame()->Show();
        GetWindow().Show();
        pImp->xFrame->getContainerWindow()->setVisible( sal_True );
        if ( pParentFrame )
            pParentFrame->Appear();
    }
}

sal_uInt16 SfxFrame::GetBeamerState_Impl() const
{
    return pImp->nHasBrowser;
}

void SfxFrame::SetOwnsBindings_Impl( sal_Bool bSet )
{
    pImp->bOwnsBindings = bSet;
}

sal_Bool SfxFrame::OwnsBindings_Impl() const
{
    return pImp->bOwnsBindings;
}

void SfxFrame::SetToolSpaceBorderPixel_Impl( const SvBorder& rBorder )
{
    pImp->aBorder = rBorder;
    SfxViewFrame *pF = GetCurrentViewFrame();
    if ( pF )
    {
        Point aPos ( rBorder.Left(), rBorder.Top() );
        Size aSize( GetWindow().GetOutputSizePixel() );
        aSize.Width() -= ( rBorder.Left() + rBorder.Right() );
        aSize.Height() -= ( rBorder.Top() + rBorder.Bottom() );

        if ( GetParentFrame() )
        {
            sal_Bool bHasTools = rBorder.Left() != rBorder.Right() || rBorder.Top() != rBorder.Bottom();
            pF->GetWindow().SetBorderStyle( bHasTools ? WINDOW_BORDER_NORMAL : WINDOW_BORDER_NOBORDER );
        }

        pF->GetWindow().SetPosSizePixel( aPos, aSize );
    }
}

Rectangle SfxFrame::GetTopOuterRectPixel_Impl() const
{
    Size aSize( GetWindow().GetOutputSizePixel() );
    Point aPoint;
    return ( Rectangle ( aPoint, aSize ) );
}

SfxWorkWindow* SfxFrame::GetWorkWindow_Impl() const
{
    if ( pImp->pWorkWin )
        return pImp->pWorkWin;
    else if ( pParentFrame )
        return pParentFrame->GetWorkWindow_Impl();
    else
        return NULL;
}

void SfxFrame::SetWorkWindow_Impl( SfxWorkWindow* pWorkwin )
{
    pImp->pWorkWin = pWorkwin;
}

void SfxFrame::CreateWorkWindow_Impl()
{
    pImp->pWorkWin = new SfxFrameWorkWin_Impl( &GetWindow(), this );
}

const SvBorder& SfxFrame::GetBorder_Impl() const
{
    return pImp->aBorder;
}

void SfxFrame::GrabFocusOnComponent_Impl()
{
    if ( pImp->bReleasingComponent )
    {
        GetWindow().GrabFocus();
        return;
    }

    Window* pWindow = &GetWindow();
    if ( GetCurrentViewFrame() && GetCurrentViewFrame()->GetViewShell() && GetCurrentViewFrame()->GetViewShell()->GetWindow() )
        pWindow = GetCurrentViewFrame()->GetViewShell()->GetWindow();

    if( !pWindow->HasChildPathFocus() )
        pWindow->GrabFocus();
}

void SfxFrame::ReFill_Impl( const SfxFrameSetDescriptor* pSet )
{
    SfxFrameSetViewShell *pView = PTR_CAST( SfxFrameSetViewShell, GetCurrentViewFrame()->GetViewShell() );
    if ( pView )
        pView->ReFill( pSet );
    else
    {
        SfxFrameSetDescriptor *pOld = pImp->pDescr->GetFrameSet();
        SfxFrameSetDescriptor *pNew = pSet->Clone();
        pImp->pDescr->SetFrameSet( pNew );
        for ( sal_uInt16 n=0; n<pSet->GetFrameCount(); n++ )
        {
            SfxFrameDescriptor* pD = pNew->GetFrame(n);
            SfxURLFrame* pFrame = PTR_CAST( SfxURLFrame, SearchChildrenForName_Impl( pD->GetName() ) );
            if ( pFrame )
                pFrame->Update( pD );
            if ( pD->GetFrameSet() )
                pFrame->ReFill_Impl( pD->GetFrameSet() );
        }

        delete pOld;
    }
}

sal_Bool SfxFrame::ActivateNextChildWindow_Impl( sal_Bool bForward )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp;
    ::com::sun::star::util::URL aTargetURL;
    String aCmd( DEFINE_CONST_UNICODE( ".uno:" ) );
    aCmd += String::CreateFromAscii( SFX_SLOTPOOL().GetUnoSlot( bForward ? SID_NEXTWINDOW : SID_PREVWINDOW )->GetUnoName() );
    aTargetURL.Complete = aCmd;
    Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
    xTrans->parseStrict( aTargetURL );

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >  aTmpRef( GetFrameInterface() );
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  xProv( aTmpRef, ::com::sun::star::uno::UNO_QUERY );
    if ( xProv.is() )
        xDisp = xProv->queryDispatch( aTargetURL, String(), 0 );

    if ( xDisp.is() )
    {
        // Der Controller kann es selbst
        xDisp->dispatch( aTargetURL, ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue>() );
        return sal_True;
    }
    else if ( ( pImp->nType & SFXFRAME_FRAMESET ) && pChildArr->Count() )
    {
        // ansonsten versuchen wir es mit dem SFX
        SfxViewFrame *pAct = pImp->pCurrentViewFrame->GetActiveChildFrame_Impl();
        if ( !GetWindow().HasChildPathFocus( sal_True ) )
        {
            pAct = ( bForward ? (*pChildArr)[0] : (*pChildArr)[pChildArr->Count()-1] )->GetCurrentViewFrame();
        }
        else if ( pAct )
        {
            sal_uInt16 nPos = (*pChildArr).GetPos( pAct->GetFrame() );
            if ( bForward && ++nPos<pChildArr->Count() )
            {
                pAct = (*pChildArr)[nPos]->GetCurrentViewFrame();
            }
            else if ( !bForward && nPos > 0 )
            {
                pAct = (*pChildArr)[--nPos]->GetCurrentViewFrame();
            }
            else
                pAct = NULL;
        }

        if ( pAct )
        {
            pAct->MakeActive_Impl( TRUE );
            return sal_True;
        }
    }

    return sal_False;
}

const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >  SfxFrame::GetInterceptor_Impl()
{
//(mba)/compview: hier fehlt der Zugriff auf den Interceptor !!!!!
    return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor > ();
}

void SfxFrame::ReleasingComponent_Impl( sal_Bool bSet )
{
    pImp->bReleasingComponent = bSet;
}

sal_Bool SfxFrame::LoadSfxComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & aFrame, const ::rtl::OUString& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs, const SfxObjectFactory *pFact )
{
    // get Implementation of xComponents ...
    DBG_ERROR( "Should not be called anymore!" );
    return sal_False;
}

void SfxFrame::LockFocus_Impl( sal_Bool bLock )
{
    if ( pChildArr && pChildArr->Count() )
    {
        sal_uInt16 nCount = pChildArr->Count();
        for ( sal_uInt16 n=0; n<nCount; n++ )
        {
            SfxFrame* pFrame = (*pChildArr)[n];
            pFrame->pImp->bFocusLocked = bLock;
            pFrame->LockFocus_Impl( bLock );
        }
    }
}

sal_Bool SfxFrame::IsFocusLocked_Impl() const
{
    return pImp->bFocusLocked;
}
/*
sal_Bool SfxFrame::IsPlugin_Impl() const
{
//  Reference < XPluginInstance > xPlugin ( pImp->xFrame, UNO_QUERY );
//  return xPlugin.is();

    // Set default return value if method failed.
    sal_Bool bReturn = sal_False;
    // Get Desktop to get a list of all current tasks on it.
    Reference< XTasksSupplier > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ), UNO_QUERY );
    DBG_ASSERT( !(xDesktop.is()==sal_False), "SfxFrame::IsPlugin_Impl()Can't get reference to desktop service!\n" );
    Reference< XEnumeration > xList = xDesktop->getTasks()->createEnumeration();
    while( xList->hasMoreElements() == sal_True )
    {
        Reference< XTask > xTask;
        xList->nextElement() >>= xTask;
        Reference< XPluginInstance > xPlugIn( xTask, UNO_QUERY );
        if( xPlugIn.is() == sal_True )
        {
            bReturn = sal_True;
            break;
        }
    }
    return bReturn;
}*/

void SfxFrame::Resize()
{
    if ( OwnsBindings_Impl() )
    {
        SfxWorkWindow *pWork = GetWorkWindow_Impl();
        if ( pWork )
        {
            pWork->ArrangeChilds_Impl();
            pWork->ShowChilds_Impl();
        }

        // problem in presence of UIActive object: when the window is resized, but the toolspace border
        // remains the same, setting the toolspace border at the ContainerEnvironment doesn't force a
        // resize on the IPEnvironment; without that no resize is called for the SfxViewFrame. So always
        // set the window size of the SfxViewFrame explicit.
        SetToolSpaceBorderPixel_Impl( pImp->aBorder );
    }
    else if ( pImp->pCurrentViewFrame )
    {
        pImp->pCurrentViewFrame->GetWindow().SetSizePixel( GetWindow().GetOutputSizePixel() );
    }

}

SfxFrame* SfxFrame::GetFirst()
{
    if ( !pFramesArr_Impl )
        return 0;
    return pFramesArr_Impl->Count() ? pFramesArr_Impl->GetObject(0) : 0;
}

SfxFrame* SfxFrame::GetNext( SfxFrame& rFrame )
{
    USHORT nPos = pFramesArr_Impl->GetPos( &rFrame );
    if ( nPos+1 < pFramesArr_Impl->Count() )
        return pFramesArr_Impl->GetObject(nPos+1);
    else
        return NULL;
}


