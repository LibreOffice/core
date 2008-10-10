/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: frame.cxx,v $
 * $Revision: 1.55.142.1 $
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
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/container/XChild.hpp>

#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#include <svtools/intitem.hxx>
#ifndef _SPLITWIN_HXX //autogen
#include <vcl/splitwin.hxx>
#endif
#include <svtools/eitem.hxx>
#include <svtools/stritem.hxx>
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif
#include <com/sun/star/awt/PosSize.hpp>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <vcl/msgbox.hxx>

#ifndef GCC
#endif

// wg. pTopFrames
#include "appdata.hxx"
#include <sfx2/app.hxx>
#include <sfx2/event.hxx>
#include <sfx2/unoctitm.hxx>
#include <sfx2/frame.hxx>
#include "arrdecl.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/frmdescr.hxx>
#include "openflag.hxx"
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/fcontnr.hxx>
#include "impframe.hxx"
#include <sfx2/appuno.hxx>
#include <sfx2/topfrm.hxx>
#include "workwin.hxx"
#include <sfx2/sfxuno.hxx>
#include <sfx2/msgpool.hxx>
#include "objshimp.hxx"
#include <sfx2/ipclient.hxx>

#ifdef DBG_UTIL
#include <sfx2/frmhtmlw.hxx>
#endif

using namespace com::sun::star;

static SfxFrameArr_Impl* pFramesArr_Impl=0;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;

TYPEINIT1(SfxFrame, SfxListener);
TYPEINIT1_AUTOFACTORY(SfxFrameItem, SfxPoolItem);
TYPEINIT1(SfxUsrAnyItem, SfxPoolItem);

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
            pWin->Hide();
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
            pWin->Show();
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

            SFX_APP()->NotifyEvent( SfxEventHint(SFX_EVENT_PREPARECLOSEVIEW, pCur) );

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

SfxFrame* SfxFrame::SearchFrame( const String& rName, SfxMedium* /*pMedium*/ )
{
    // Weil Netscape f"uhrende BLANKS "uberliest, m"ussen wir das wohl auch
    // Sollte besser auch bei den FrameNames gemacht werden!
    String aFrameName( rName );
    aFrameName.EraseLeadingChars();

    SfxFrame *pFrame = this;
    if( !aFrameName.Len() || aFrameName.CompareIgnoreCaseToAscii("_self") == COMPARE_EQUAL )
    {
        return pFrame;
    }
    else if (  aFrameName.CompareIgnoreCaseToAscii("_smartself") == COMPARE_EQUAL )
    {
        DBG_ERROR("Not supported!");
        return pFrame;
    }
    else if ( aFrameName.CompareIgnoreCaseToAscii( GetFrameName() ) == COMPARE_EQUAL)
    {
        // Eigener Name, kein Name oder Selbstbezug
        return pFrame;
    }
    else if ( aFrameName.CompareIgnoreCaseToAscii("_parent") == COMPARE_EQUAL )
    {
        // Gesucht ist das Parent Frameset
        return pParentFrame ? pParentFrame : this;
    }
    else if ( aFrameName.CompareIgnoreCaseToAscii("_blank") == COMPARE_EQUAL )
    {
        return NULL;
    }
    else if ( aFrameName.CompareIgnoreCaseToAscii("_top") == COMPARE_EQUAL )
    {
        while ( pFrame->GetParentFrame() )
            pFrame = pFrame->GetParentFrame();
        return pFrame;
    }
    else
    {
        // Zuerst unterhalb dieses Frames absuchen
        pFrame = SearchChildrenForName_Impl( aFrameName );
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
            if ( aFrameName.CompareIgnoreCaseToAscii( pParent->GetFrameName() ) == COMPARE_EQUAL )
                return pParent;

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
                if( aFrameName.CompareIgnoreCaseToAscii( pCurFrame->GetFrameName() ) ==
                    COMPARE_EQUAL )
                    return pCurFrame;
                else
                {
                    pFrame = pCurFrame->SearchChildrenForName_Impl( aFrameName );
                    if( pFrame )
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

void SfxFrame::DocumentInserted( SfxObjectShell* /*pDoc*/ )
{
}

sal_uInt16 SfxFrame::GetChildFrameCount() const
{
    return pChildArr ? pChildArr->Count() : 0;
}

sal_Bool SfxFrame::InsertDocument( SfxObjectShell* pDoc )
{
    DocumentInserted( pDoc );
    return sal_True;
}

void SfxFrame::CancelTransfers( sal_Bool /*bCancelLoadEnv*/ )
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
                 pFrm = SfxViewFrame::GetNext( *pFrm, pObj ) ) ;
            // Keine anderer Frame mehr auf Doc -> Cancel
            if( !pFrm )
            {
                pObj->CancelTransfers();
                GetCurrentDocument()->Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
            }
        }

        // zuerst Nachladende Frames stoppen
        sal_uInt16 nCount = GetChildFrameCount();
        for( sal_uInt16 n = 0; n<nCount; n++ )
            GetChildFrame( n )->CancelTransfers();

        //  ggf. StarOne-Loader canceln
        SfxFrameWeak wFrame( this );
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

void SfxFrame::CopyHistory_Impl( SfxFrame* /*pFrame*/ ) const
{
}


void SfxFrame::ClearHistory()
{
}

sal_Bool SfxFrame::ExecuteHistoryMenu_Impl( sal_uInt16 /*nWhich*/, const Rectangle& /*rRect*/, sal_uInt16 /*nFlags*/ )
{
    return sal_True;
}

sal_Bool SfxFrame::Browse( sal_Bool /*bForward*/, sal_uInt16 /*nSteps*/, sal_Bool /*bNewFrame*/ )
{
    return FALSE;
}

void SfxFrame::UpdatePickEntries()
{
}

void SfxFrame::UpdatePickEntries( const ::com::sun::star::uno::Any& /*rValue*/ )
{
}

void SfxFrame::UpdateUndoHistory_Impl( SfxObjectShell* /*pDocSh*/, const String* /*pNew*/, const String* /*pTitle*/ )
{
}

void SfxFrame::UpdateCurrentHistory_Impl( SfxObjectShell* /*pDocSh*/, const String* /*pNew*/  )
{
}

void SfxFrame::UpdateHistory( SfxObjectShell* /*pDocSh*/, const String* /*pNew*/  )
{
}

void SfxFrame::UpdateHistory(
    const ::rtl::OUString& /*aURL*/,
    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& /*aArgs*/,
    const ::rtl::OUString& /*rTitle*/  )
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

    GetParentFrame();
    const SfxMedium *pMed = pDoc->GetMedium();
    GetDescriptor()->SetActualURL( pMed->GetOrigURL() );

    SFX_ITEMSET_ARG( pMed->GetItemSet(), pItem, SfxBoolItem, SID_EDITDOC, sal_False );
    sal_Bool bEditable = ( !pItem || pItem->GetValue() );

    GetDescriptor()->SetEditable( bEditable );

    // FileOpen-Parameter merken
    SfxItemSet* pItemSet = pMed->GetItemSet();
    String aMedName( pMed->GetName() );

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
        // Nur TopLevel-Frames verwalten ihren Descriptor selbst, bei den
        // anderen tut es das Frameset
        if ( !pParentFrame )
            delete pImp->pDescr;
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
        pImp->pDescr = new SfxFrameDescriptor;
        if ( GetCurrentDocument() )
            pImp->pDescr->SetURL( GetCurrentDocument()->GetMedium()->GetOrigURL() );
    }
    return pImp->pDescr;
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
#ifdef DEBUG
        SfxItemSet* pSet;
        pSet = pMedium->GetItemSet();
#endif
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

    return (GetDescriptor()->GetActualURL().GetMainURL( INetURLObject::NO_DECODE ).getLength() != 0);
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

SfxFrameItem::SfxFrameItem( sal_uInt16 nWhichId, SfxViewFrame *p )
    : SfxPoolItem( nWhichId ), pFrame( p ? p->GetFrame() : NULL )
{
    wFrame = pFrame;
}

SfxFrameItem::SfxFrameItem( sal_uInt16 nWhichId, SfxFrame *p ):
    SfxPoolItem( nWhichId ),
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


SfxUsrAnyItem::SfxUsrAnyItem( sal_uInt16 nWhichId, const ::com::sun::star::uno::Any& rAny )
    : SfxPoolItem( nWhichId )
{
    aValue = rAny;
}

int SfxUsrAnyItem::operator==( const SfxPoolItem& /*rItem*/ ) const
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

sal_Bool SfxUsrAnyItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ ) const
{
    rVal = aValue;
    return sal_True;
}

sal_Bool SfxUsrAnyItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ )
{
    aValue = rVal;
    return sal_True;
}

sal_Bool SfxFrame::BrowseInFrame( int /*nDelta*/ )
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

void SfxFrame::LoadFinished_Impl()
{
}

SfxFrame* SfxFrame::findFrame(const ::rtl::OUString& aTargetframename, sal_Int32 nSearchFlags)
{
    // Weil Netscape f"uhrende BLANKS "uberliest, m"ussen wir das wohl auch
    // Sollte besser auch bei den FrameNames gemacht werden!
    String aTargetName( aTargetframename );
    aTargetName.EraseLeadingChars();

    SfxFrame *pFrame = this;

    if ( !aTargetName.Len() && GetParentFrame() && GetParentFrame()->GetCurrentViewFrame()->GetViewShell()->IsImplementedAsFrameset_Impl() )
        return GetParentFrame();

    if( !aTargetName.Len() || aTargetName.CompareIgnoreCaseToAscii("_self") == COMPARE_EQUAL )
        return pFrame;

    if (  aTargetName.CompareIgnoreCaseToAscii("_smartself") == COMPARE_EQUAL )
    {
        DBG_ERROR("Not supported!");
        return pFrame;
    }

    if ( aTargetName.CompareIgnoreCaseToAscii("_parent") == COMPARE_EQUAL )
    {
        // Gesucht ist das Parent Frameset
        return pFrame->GetParentFrame();
    }
    else if ( aTargetName.CompareIgnoreCaseToAscii("_blank") == COMPARE_EQUAL )
    {
        return NULL;

    }
    else if ( aTargetName.CompareIgnoreCaseToAscii("_top") == COMPARE_EQUAL )
    {
        while ( pFrame->GetParentFrame() )
            pFrame = pFrame->GetParentFrame();
        return pFrame;
    }

    if ( ( nSearchFlags & FRAME_SEARCH_SELF ) && aTargetName.CompareIgnoreCaseToAscii( GetFrameName() ) == COMPARE_EQUAL )
    {
        // Eigener Name
        return pFrame;
    }

    if ( nSearchFlags & FRAME_SEARCH_CHILDREN )
    {
        // Zuerst unterhalb dieses Frames absuchen
        pFrame = SearchChildrenForName_Impl( aTargetName );
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
            if ( aTargetName.CompareIgnoreCaseToAscii( pParent->GetFrameName() ) == COMPARE_EQUAL )
                return pParent;

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
                if( aTargetName.CompareIgnoreCaseToAscii( pCurFrame->GetFrameName() ) == COMPARE_EQUAL )
                    return pCurFrame;
                else
                {
                    pFrame = pCurFrame->SearchChildrenForName_Impl( aTargetName );
                    if( pFrame )
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

void SfxFrame::AppearWithUpdate()
{
    Appear();
    if ( GetCurrentViewFrame() )
        GetCurrentViewFrame()->GetDispatcher()->Update_Impl( sal_True );
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
    SfxFrame* pFrame = this;

    if ( IsInPlace() )
    {
        // this makes sence only for inplace activated objects
        try
        {
            Reference < XChild > xChild( GetCurrentDocument()->GetModel(), UNO_QUERY );
            if ( xChild.is() )
            {
                Reference < XModel > xParent( xChild->getParent(), UNO_QUERY );
                if ( xParent.is() )
                {
                    Reference< XController > xParentCtrler = xParent->getCurrentController();
                    if ( xParentCtrler.is() )
                    {
                        Reference < XFrame > xFrame( xParentCtrler->getFrame() );
                        SfxFrame* pFr = SfxFrame::GetFirst();
                        while ( pFr )
                        {
                            if ( pFr->GetFrameInterface() == xFrame )
                            {
                                pFrame = pFr;
                                break;
                            }

                            pFr = SfxFrame::GetNext( *pFr );
                        }
                    }
                }
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"SfxFrame::CreateWorkWindow_Impl: Exception cachted. Please try to submit a repoducable bug !");
        }
    }

    pImp->pWorkWin = new SfxFrameWorkWin_Impl( &pFrame->GetWindow(), this, pFrame );
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

const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >  SfxFrame::GetInterceptor_Impl()
{
//(mba)/compview: hier fehlt der Zugriff auf den Interceptor !!!!!
    return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor > ();
}

void SfxFrame::ReleasingComponent_Impl( sal_Bool bSet )
{
    pImp->bReleasingComponent = bSet;
}

sal_Bool SfxFrame::LoadSfxComponent(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & /*aFrame*/,
    const ::rtl::OUString& /*aURL*/,
    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& /*aArgs*/,
    const SfxObjectFactory* /*pFact*/ )
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

sal_Bool SfxFrame::IsInPlace() const
{
    return pImp->bInPlace;
}

void SfxFrame::SetInPlace_Impl( sal_Bool bSet )
{
    pImp->bInPlace = bSet;
}

void SfxFrame::Resize()
{
    if ( IsClosing_Impl() )
        return;

    if ( OwnsBindings_Impl() )
    {
        if ( IsInPlace() )
        {
            SetToolSpaceBorderPixel_Impl( SvBorder() );
        }
        else
        {
            // check for IPClient that contains UIactive object or object that is currently UI activating
            SfxWorkWindow *pWork = GetWorkWindow_Impl();
            SfxInPlaceClient* pClient = GetCurrentViewFrame()->GetViewShell() ? GetCurrentViewFrame()->GetViewShell()->GetUIActiveIPClient_Impl() : 0;
            if ( pClient )
            {
                uno::Reference < lang::XUnoTunnel > xObj( pClient->GetObject()->getComponent(), uno::UNO_QUERY );
                uno::Sequence < sal_Int8 > aSeq( SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence() );
                sal_Int64 nHandle = (xObj.is()? xObj->getSomething( aSeq ): 0);
                if ( nHandle )
                {
                    SfxObjectShell* pDoc = reinterpret_cast< SfxObjectShell* >( sal::static_int_cast< sal_IntPtr >( nHandle ));
                    pWork = SfxViewFrame::GetFirst( pDoc )->GetFrame()->GetWorkWindow_Impl();
                }
            }

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

