/*************************************************************************
 *
 *  $RCSfile: frame.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:02:41 $
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
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_CLOSEVETOEXCEPTION_HPP_
#include <com/sun/star/util/CloseVetoException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
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
#include "sfxuno.hxx"
#include "msgpool.hxx"
#include "objshimp.hxx"

#ifdef DBG_UTIL
#include "frmhtmlw.hxx"
#endif

static SfxFrameArr_Impl* pFramesArr_Impl=0;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;

TYPEINIT1(SfxFrame, SfxListener);
TYPEINIT1_AUTOFACTORY(SfxFrameItem, SfxPoolItem);
TYPEINIT1(SfxUsrAnyItem, SfxPoolItem);

void SfxFrame::SetLoadCancelable_Impl( SfxCancellable* pCbl )
{
    delete pImp->pLoadCancellable;
    pImp->pLoadCancellable = pCbl;
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

SfxFrame::~SfxFrame()
{
    pFramesArr_Impl->Remove( pFramesArr_Impl->GetPos( this ) );

    if ( pParentFrame )
    {
        pParentFrame->RemoveChildFrame_Impl( this );
        pParentFrame = 0;
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
    BOOL bRet = FALSE;
    if ( !pImp->bClosing )
    {
        pImp->bClosing = sal_True;
        CancelTransfers();

        // now close frame; it will be deleted if this call is successful, so don't use any members after that!
        bRet = TRUE;
        try
        {
            Reference< XCloseable > xCloseable  ( pImp->xFrame, UNO_QUERY );
            if ( (!GetCurrentDocument() || !GetCurrentDocument()->Get_Impl()->bDisposing ) && xCloseable.is())
                xCloseable->close(sal_True);
            else if ( pImp->xFrame.is() )
            {
                Reference < XFrame > xFrame = pImp->xFrame;
                xFrame->setComponent( Reference < com::sun::star::awt::XWindow >(), Reference < XController >() );
                xFrame->dispose();
            }
            else
                bRet = DoClose_Impl();
        }
        catch( ::com::sun::star::util::CloseVetoException& )
        {
            pImp->bClosing = sal_False;
            bRet = FALSE;
        }
        catch( ::com::sun::star::lang::DisposedException& )
        {
        }
    }

    return bRet;
}

sal_Bool SfxFrame::DoClose_Impl()
{
    sal_Bool bRet = sal_True;
    SfxBindings* pBindings = NULL;
    if ( pImp->pCurrentViewFrame )
        pBindings = &pImp->pCurrentViewFrame->GetBindings();

    // Bei internen Tasks m"ussen Controller und Tools abger"aumt werden
    if ( pImp->pWorkWin )
        pImp->pWorkWin->DeleteControllers_Impl();

    if ( pImp->pCurrentViewFrame )
        bRet = pImp->pCurrentViewFrame->Close();

    if ( pImp->bOwnsBindings )
        DELETEZ( pBindings );

    bRet = Close();
    DBG_ASSERT( bRet, "Impossible state: frame closes, but controller refuses!");
    return bRet;
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

SfxFrame* SfxFrame::SearchChildrenForName_Impl( const String& rName, sal_Bool bDeep ) const
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

void SfxFrame::SetIsClosing_Impl()
{
    pImp->bClosing = TRUE;
}

void SfxFrame::DocumentInserted( SfxObjectShell* pDoc )
{
}

sal_uInt16 SfxFrame::GetChildFrameCount() const
{
    return pChildArr ? pChildArr->Count() : 0;
}

sal_Bool SfxFrame::InsertDocument( SfxObjectShell *pDoc )
{
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
        SfxObjectShell* pObj = GetCurrentDocument();
        if( pObj ) //&& !( pObj->Get_Impl()->nLoadedFlags & SFX_LOADED_ALL ))
        {
            SfxViewFrame* pFrm;
            for( pFrm = SfxViewFrame::GetFirst( pObj );
                 pFrm && pFrm->GetFrame() == this;
                 pFrm = SfxViewFrame::GetNext( *pFrm, pObj ) );
            // Keine anderer Frame mehr auf Doc -> Cancel
            if( !pFrm )
            {
                pObj->CancelTransfers();
                GetCurrentDocument()->Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
            }
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

void SfxFrame::CopyHistory_Impl( SfxFrame *pFrame ) const
{
}


void SfxFrame::ClearHistory()
{
}

sal_Bool SfxFrame::ExecuteHistoryMenu_Impl( sal_uInt16 nWhich, const Rectangle& rRect, sal_uInt16 nFlags )
{
    return sal_True;
}

sal_Bool SfxFrame::Browse( sal_Bool bForward, sal_uInt16 nSteps, sal_Bool bNewFrame )
{
    return FALSE;
}

void SfxFrame::UpdatePickEntries()
{
}

void SfxFrame::UpdatePickEntries( const ::com::sun::star::uno::Any& rValue )
{
}

void SfxFrame::UpdateUndoHistory_Impl( SfxObjectShell *pDocSh, const String* pNew, const String* pTitle )
{
}

void SfxFrame::UpdateCurrentHistory_Impl( SfxObjectShell *pDocSh, const String* pNew  )
{
}

void SfxFrame::UpdateHistory( SfxObjectShell *pDocSh, const String* pNew  )
{
}

void SfxFrame::UpdateHistory( const ::rtl::OUString& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs, const ::rtl::OUString& rTitle  )
{
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

    const SfxFilter* pFilter = pMed->GetOrigFilter();
    String aFilter;
    if ( pFilter )
        aFilter = pFilter->GetFilterName();

    SFX_ITEMSET_ARG( pItemSet, pRefererItem, SfxStringItem, SID_REFERER, sal_False);
    SFX_ITEMSET_ARG( pItemSet, pOptionsItem, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False);
    SFX_ITEMSET_ARG( pItemSet, pTitle1Item, SfxStringItem, SID_DOCINFO_TITLE, sal_False);

    SfxItemSet *pSet = GetDescriptor()->GetArgs();

    // Alle alten Items l"oschen
    pSet->ClearItem();

    if ( pRefererItem )
        pSet->Put( *pRefererItem );
    else
        pSet->Put( SfxStringItem( SID_REFERER, String() ) );

    if ( pOptionsItem )
        pSet->Put( *pOptionsItem );

    if ( pTitle1Item )
        pSet->Put( *pTitle1Item );

    pSet->Put( SfxStringItem( SID_FILTER_NAME, aFilter ));
}

//-------------------------------------------------------------------------

sal_Bool SfxFrame::CanBrowseBackward() const
{
    return FALSE;
}

//-------------------------------------------------------------------------

sal_Bool SfxFrame::CanBrowseForward() const
{
    return FALSE;
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
    return pImp->pLoadEnv && pImp->pLoadEnv->GetObjectShell() != GetCurrentDocument();
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
        if ( INetURLObject( GetDescriptor()->GetActualURL() ) != INetURLObject( pMedium->GetOrigURL() ) )
            return sal_True;

        // Falls die Filter nicht uebereinstimmen
        SFX_ITEMSET_ARG( pNew, pFilterItem, SfxStringItem, SID_FILTER_NAME, sal_False );
        if( pMedium->GetOrigFilter() && pFilterItem && pFilterItem->GetValue() != pMedium->GetOrigFilter()->GetFilterName() )
            return sal_True;

        return sal_False;
    }

    return (GetDescriptor()->GetActualURL().GetMainURL( INetURLObject::NO_DECODE ).Len() != 0);
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

SfxFrameItem::SfxFrameItem( sal_uInt16 nWhich, SfxViewFrame *p )
    : SfxPoolItem( nWhich ), pFrame( p ? p->GetFrame() : NULL )
{
    wFrame = pFrame;
}

SfxFrameItem::SfxFrameItem( sal_uInt16 nWhich, SfxFrame *p ):
    SfxPoolItem( nWhich ),
    pFrame( p ), wFrame( p )
{
}

SfxFrameItem::SfxFrameItem( SfxFrame *p ):
    SfxPoolItem( 0 ),
    pFrame( p ), wFrame( p )
{
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

sal_Bool SfxFrameItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE ) const
{
    if ( wFrame )
    {
        rVal <<= wFrame->GetFrameInterface();
        return TRUE;
    }

    return FALSE;
}

sal_Bool SfxFrameItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE )
{
    Reference < XFrame > xFrame;
    if ( (rVal >>= xFrame) && xFrame.is() )
    {
        SfxFrame* pFr = SfxFrame::GetFirst();
        while ( pFr )
        {
            if ( pFr->GetFrameInterface() == xFrame )
            {
                wFrame = pFrame = pFr;
                return TRUE;
            }

            pFr = SfxFrame::GetNext( *pFr );
        }
        return TRUE;
    }

    return FALSE;
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
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder;
    if ( !rFrame.is() && GetCurrentViewFrame() )
        GetCurrentViewFrame()->GetBindings().SetRecorder_Impl( xRecorder );
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
        Reference < ::com::sun::star::awt::XTopWindow > xTopWindow( pImp->xFrame->getContainerWindow(), UNO_QUERY );
        if ( xTopWindow.is() )
            xTopWindow->toFront();
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
        long nDeltaX = rBorder.Left() + rBorder.Right();
        if ( aSize.Width() > nDeltaX )
            aSize.Width() -= nDeltaX;
        else
            aSize.Width() = 0;

        long nDeltaY = rBorder.Top() + rBorder.Bottom();
        if ( aSize.Height() > nDeltaY )
            aSize.Height() -= nDeltaY;
        else
            aSize.Height() = 0;

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

void SfxFrame::Resize()
{
    if ( IsClosing_Impl() )
        return;

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

const SfxPoolItem* SfxFrame::LoadDocumentSynchron( SfxItemSet& aSet )
{
    aSet.Put( SfxFrameItem( SID_DOCFRAME, this ) );
    aSet.ClearItem( SID_TARGETNAME );
    return SFX_APP()->GetDispatcher_Impl()->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, aSet );
}

void SfxFrame::CloseDocument_Impl()
{
    Reference < XFrame > xFrame( pImp->xFrame );
    Window* pContainer = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
    pContainer->SetText( Application::GetDisplayName() );

    Window* pWindow = new Window( pContainer, WB_BORDER );
    pWindow->Show();
    pWindow->SetBackground( Wallpaper( pWindow->GetSettings().GetStyleSettings().GetFaceColor() ) );
    xFrame->setComponent(  VCLUnoHelper::GetInterface( pWindow ), Reference < XController >() );
    String aMenuRes( RTL_CONSTASCII_USTRINGPARAM( "private:resource/" ));
    aMenuRes += String::CreateFromInt32(RID_DEFAULTMENU);

    URL aURL;
    aURL.Complete = aMenuRes;

    Reference< XURLTransformer >  xTrans ( ::comphelper::getProcessServiceFactory()->createInstance(
                        ::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer") ), UNO_QUERY );
    if( xTrans.is() )
    {
        // Datei laden
        xTrans->parseStrict( aURL );

        Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
        if ( xProv.is() )
        {
            Reference< XDispatch >  aDisp = xProv->queryDispatch( aURL,  ::rtl::OUString::createFromAscii("_menubar"), 12 );
            if ( aDisp.is() )
                aDisp->dispatch( aURL, Sequence<com::sun::star::beans::PropertyValue>() );
        }
    }
}

