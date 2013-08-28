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

#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <vcl/menu.hxx>
#include <svl/intitem.hxx>
#include <vcl/splitwin.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/msgbox.hxx>

// due to pTopFrames
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
TYPEINIT1_AUTOFACTORY(SfxUnoFrameItem, SfxPoolItem);

SvCompatWeakHdl* SfxFrame::GetHdl()
{
    return pImp->GetHdl();
}

//--------------------------------------------------------------------
void SfxFrame::Construct_Impl()
{
    pImp = new SfxFrame_Impl( this );
    if ( !pFramesArr_Impl )
        pFramesArr_Impl = new SfxFrameArr_Impl;
    pFramesArr_Impl->push_back( this );
}

//--------------------------------------------------------------------

SfxFrame::~SfxFrame()
{
    RemoveTopFrame_Impl( this );
    DELETEZ( pWindow );

    SfxFrameArr_Impl::iterator it = std::find( pFramesArr_Impl->begin(), pFramesArr_Impl->end(), this );
    if ( it != pFramesArr_Impl->end() )
        pFramesArr_Impl->erase( it );

    if ( pParentFrame )
    {
        pParentFrame->RemoveChildFrame_Impl( this );
        pParentFrame = 0;
    }

    delete pImp->pDescr;

    if ( pChildArr )
    {
        DBG_ASSERT( pChildArr->empty(), "Children are not removed!" );
        delete pChildArr;
    }

    delete pImp;
}

sal_Bool SfxFrame::DoClose()
{
    // Actually, one more PrepareClose is still needed!
    sal_Bool bRet = sal_False;
    if ( !pImp->bClosing )
    {
        pImp->bClosing = sal_True;
        CancelTransfers();

        // now close frame; it will be deleted if this call is successful, so don't use any members after that!
        bRet = sal_True;
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
            bRet = sal_False;
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

    // For internal tasks Controllers and Tools must be cleared
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

sal_Bool SfxFrame::DocIsModified_Impl()
{
    if ( pImp->pCurrentViewFrame && pImp->pCurrentViewFrame->GetObjectShell() &&
            pImp->pCurrentViewFrame->GetObjectShell()->IsModified() )
        return sal_True;
    for( sal_uInt16 nPos = GetChildFrameCount(); nPos--; )
        if( (*pChildArr)[ nPos ]->DocIsModified_Impl() )
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
                bOther = ( &pFrame->GetFrame() != this );
            }

            SFX_APP()->NotifyEvent( SfxViewEventHint(SFX_EVENT_PREPARECLOSEVIEW, GlobalEventConfig::GetEventName( STR_EVENT_PREPARECLOSEVIEW ), pCur, GetController() ) );

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
            for( sal_uInt16 nPos = GetChildFrameCount(); nRet == RET_OK && nPos--; )
                nRet = (*pChildArr)[ nPos ]->PrepareClose_Impl( bUI, bForBrowsing );
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
    if ( pChildArr && pChildArr->size() > nPos )
    {
        DBG_ASSERT( nPos < pChildArr->size(), "Wrong Index!");
        return (*pChildArr)[nPos];
    }

    return 0L;
}

void SfxFrame::RemoveChildFrame_Impl( SfxFrame* pFrame )
{
    DBG_ASSERT( pChildArr, "Unknown Frame!");
    SfxFrameArr_Impl::iterator it = std::find( pChildArr->begin(), pChildArr->end(), pFrame );
    if ( it != pChildArr->end() )
        pChildArr->erase( it );
};

SfxFrame& SfxFrame::GetTopFrame() const
{
    const SfxFrame* pParent = this;
    while ( pParent->pParentFrame )
        pParent = pParent->pParentFrame;
    return *const_cast< SfxFrame* >( pParent );
}

sal_Bool SfxFrame::IsClosing_Impl() const
{
    return pImp->bClosing;
}

void SfxFrame::SetIsClosing_Impl()
{
    pImp->bClosing = sal_True;
}

sal_uInt16 SfxFrame::GetChildFrameCount() const
{
    return pChildArr ? pChildArr->size() : 0;
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
                 pFrm && &pFrm->GetFrame() == this;
                 pFrm = SfxViewFrame::GetNext( *pFrm, pObj ) ) ;
            // No more Frame in Document -> Cancel
            if( !pFrm )
            {
                pObj->CancelTransfers();
                GetCurrentDocument()->Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
            }
        }

        // First stop multiload Frames
        sal_uInt16 nCount = GetChildFrameCount();
        for( sal_uInt16 n = 0; n<nCount; n++ )
            GetChildFrame( n )->CancelTransfers();

        //  Check if StarOne-Loader should be canceled
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
            NULL;
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

void SfxFrame::GetViewData_Impl()
{
    // Update all modifiable data between load and unload, the
    // fixed data is only processed once (after PrepareForDoc_Impl in
    // updateDescriptor) to save time.

    SfxViewFrame* pViewFrame = GetCurrentViewFrame();
    if( pViewFrame && pViewFrame->GetViewShell() )
    {
        const SfxMedium *pMed = GetCurrentDocument()->GetMedium();
        sal_Bool bReadOnly = pMed->GetOpenMode() == SFX_STREAM_READONLY;
        GetDescriptor()->SetReadOnly( bReadOnly );

        SfxItemSet *pSet = GetDescriptor()->GetArgs();
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
            // For Framesets also the data from the ChildViews hace to be processed
            sal_uInt16 nCount = pChildArr->size();
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
    // For PrepareForDoc_Impl frames, the descriptor of the updated
    // and new itemset to be initialized. All data fir restoring the view
    // are thus saved. If the document be replaced, GetViewData_Impl (so)
    // the latest information hinzugef by "added. All together then the
    // browser-history saved in. When you activate such frame pick entry
    // is complete itemsets and the descriptor in the OpenDoc sent;.
    // Here only the fixed properties identified "other adjustable, the
    // retrieved by GetViewData (saves time).

    DBG_ASSERT( pDoc, "NULL-Document inserted ?!" );

    GetParentFrame();
    const SfxMedium *pMed = pDoc->GetMedium();
    GetDescriptor()->SetActualURL( pMed->GetOrigURL() );

    SFX_ITEMSET_ARG( pMed->GetItemSet(), pItem, SfxBoolItem, SID_EDITDOC, sal_False );
    sal_Bool bEditable = ( !pItem || pItem->GetValue() );

    GetDescriptor()->SetEditable( bEditable );

    // Mark FileOpen parameter
    SfxItemSet* pItemSet = pMed->GetItemSet();

    const SfxFilter* pFilter = pMed->GetOrigFilter();
    OUString aFilter;
    if ( pFilter )
        aFilter = pFilter->GetFilterName();

    SFX_ITEMSET_ARG( pItemSet, pRefererItem, SfxStringItem, SID_REFERER, sal_False);
    SFX_ITEMSET_ARG( pItemSet, pOptionsItem, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False);
    SFX_ITEMSET_ARG( pItemSet, pTitle1Item, SfxStringItem, SID_DOCINFO_TITLE, sal_False);

    SfxItemSet *pSet = GetDescriptor()->GetArgs();

    // Delete all old Items
    pSet->ClearItem();

    if ( pRefererItem )
        pSet->Put( *pRefererItem );
    else
        pSet->Put( SfxStringItem( SID_REFERER, OUString() ) );

    if ( pOptionsItem )
        pSet->Put( *pOptionsItem );

    if ( pTitle1Item )
        pSet->Put( *pTitle1Item );

    pSet->Put( SfxStringItem( SID_FILTER_NAME, aFilter ));
}

//-------------------------------------------------------------------------

SfxFrameDescriptor* SfxFrame::GetDescriptor() const
{
    // Create a FrameDescriptor On Demand; if there is no TopLevel-Frame
    // will result in an error, as no valid link is created.

    if ( !pImp->pDescr )
    {
        DBG_ASSERT( !GetParentFrame(), "No TopLevel-Frame, but no Descriptor!" );
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
        // An empty string for 'No Target'
        rList.push_back( OUString() );
        rList.push_back( OUString( "_top" ) );
        rList.push_back( OUString( "_parent" ) );
        rList.push_back( OUString( "_blank" ) );
        rList.push_back( OUString( "_self" ) );
    }

    SfxViewFrame* pView = GetCurrentViewFrame();
    if( pView && pView->GetViewShell() && pChildArr )
    {
        sal_uInt16 nCount = pChildArr->size();
        for ( sal_uInt16 n=0; n<nCount; n++)
        {
            SfxFrame* pFrame = (*pChildArr)[n];
            pFrame->GetTargetList( rList );
        }
    }
}

//-------------------------------------------------------------------------

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
    rArr.push_back( pFrame );
}

void SfxFrame::RemoveTopFrame_Impl( SfxFrame* pFrame )
{
    SfxFrameArr_Impl& rArr = *SFX_APP()->Get_Impl()->pTopFrames;
    SfxFrameArr_Impl::iterator it = std::find( rArr.begin(), rArr.end(), pFrame );
    if ( it != rArr.end() )
        rArr.erase( it );
}

SfxFrameItem::SfxFrameItem( sal_uInt16 nWhichId, SfxViewFrame *p )
    : SfxPoolItem( nWhichId ), pFrame( p ? &p->GetFrame() : NULL )
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

OUString SfxFrameItem::GetValueText() const
{
    return OUString();
}

SfxPoolItem* SfxFrameItem::Clone( SfxItemPool *) const
{
    SfxFrameItem* pNew = new SfxFrameItem( wFrame);
    pNew->SetFramePtr_Impl( pFrame );
    return pNew;
}

bool SfxFrameItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 ) const
{
    if ( wFrame )
    {
        rVal <<= wFrame->GetFrameInterface();
        return sal_True;
    }

    return sal_False;
}

bool SfxFrameItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 )
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
                return sal_True;
            }

            pFr = SfxFrame::GetNext( *pFr );
        }
        return sal_True;
    }

    return sal_False;
}


SfxUsrAnyItem::SfxUsrAnyItem( sal_uInt16 nWhichId, const ::com::sun::star::uno::Any& rAny )
    : SfxPoolItem( nWhichId )
{
    aValue = rAny;
}

int SfxUsrAnyItem::operator==( const SfxPoolItem& /*rItem*/ ) const
{
    return sal_False;
}

SfxPoolItem* SfxUsrAnyItem::Clone( SfxItemPool *) const
{
    return new SfxUsrAnyItem( Which(), aValue );
}

bool SfxUsrAnyItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal = aValue;
    return sal_True;
}

bool SfxUsrAnyItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    aValue = rVal;
    return sal_True;
}

SfxUnoFrameItem::SfxUnoFrameItem()
    : SfxPoolItem()
    , m_xFrame()
{
}

SfxUnoFrameItem::SfxUnoFrameItem( sal_uInt16 nWhichId, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame )
    : SfxPoolItem( nWhichId )
    , m_xFrame( i_rFrame )
{
}

int SfxUnoFrameItem::operator==( const SfxPoolItem& i_rItem ) const
{
    return i_rItem.ISA( SfxUnoFrameItem ) && static_cast< const SfxUnoFrameItem& >( i_rItem ).m_xFrame == m_xFrame;
}

SfxPoolItem* SfxUnoFrameItem::Clone( SfxItemPool* ) const
{
    return new SfxUnoFrameItem( Which(), m_xFrame );
}

bool SfxUnoFrameItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= m_xFrame;
    return sal_True;
}

bool SfxUnoFrameItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    return ( rVal >>= m_xFrame );
}

SfxFrameIterator::SfxFrameIterator( const SfxFrame& rFrame, sal_Bool bRecur )
    : pFrame( &rFrame )
    , bRecursive( bRecur )
{}

SfxFrame* SfxFrameIterator::FirstFrame()
{
    // GetFirst starts the iteration at the first child frame
    return pFrame->GetChildFrame( 0 );
}

SfxFrame* SfxFrameIterator::NextFrame( SfxFrame& rPrev )
{
    // If recursion is requested testing is done first on Children.
    SfxFrame *pRet = NULL;
    if ( bRecursive )
        pRet = rPrev.GetChildFrame( 0 );
    if ( !pRet )
    {
        // In other case continue with the siblings of rPrev
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
        SfxFrameArr_Impl::iterator it = std::find( rArr.begin(), rArr.end(), &rPrev );
        if ( it != rArr.end() && (++it) != rArr.end() )
            pRet = *it;

        if ( !pRet && rPrev.pParentFrame->pParentFrame )
            pRet = NextSibling_Impl( *rPrev.pParentFrame );
    }

    return pRet;
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

void SfxFrame::SetFrameInterface_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame )
{
    pImp->xFrame = rFrame;
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder;
    if ( !rFrame.is() && GetCurrentViewFrame() )
        GetCurrentViewFrame()->GetBindings().SetRecorder_Impl( xRecorder );
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
            OSL_FAIL("SfxFrame::CreateWorkWindow_Impl: Exception cachted. Please try to submit a repoducable bug !");
        }
    }

    pImp->pWorkWin = new SfxFrameWorkWin_Impl( &pFrame->GetWindow(), this, pFrame );
}

void SfxFrame::GrabFocusOnComponent_Impl()
{
    if ( pImp->bReleasingComponent )
    {
        GetWindow().GrabFocus();
        return;
    }

    Window* pFocusWindow = &GetWindow();
    if ( GetCurrentViewFrame() && GetCurrentViewFrame()->GetViewShell() && GetCurrentViewFrame()->GetViewShell()->GetWindow() )
        pFocusWindow = GetCurrentViewFrame()->GetViewShell()->GetWindow();

    if( !pFocusWindow->HasChildPathFocus() )
        pFocusWindow->GrabFocus();
}

void SfxFrame::ReleasingComponent_Impl( sal_Bool bSet )
{
    pImp->bReleasingComponent = bSet;
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
                    pWork = SfxViewFrame::GetFirst( pDoc )->GetFrame().GetWorkWindow_Impl();
                }
            }

            if ( pWork )
            {
                pWork->ArrangeChildren_Impl();
                pWork->ShowChildren_Impl();
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
    return pFramesArr_Impl->empty() ? 0 : pFramesArr_Impl->front();
}

SfxFrame* SfxFrame::GetNext( SfxFrame& rFrame )
{
    SfxFrameArr_Impl::iterator it = std::find( pFramesArr_Impl->begin(), pFramesArr_Impl->end(), &rFrame );
    if ( it != pFramesArr_Impl->end() && (++it) != pFramesArr_Impl->end() )
        return *it;
    else
        return NULL;
}

const SfxPoolItem* SfxFrame::OpenDocumentSynchron( SfxItemSet& i_rSet, const Reference< XFrame >& i_rTargetFrame )
{
    i_rSet.Put( SfxUnoFrameItem( SID_FILLFRAME, i_rTargetFrame ) );
    i_rSet.ClearItem( SID_TARGETNAME );
    return SFX_APP()->GetDispatcher_Impl()->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, i_rSet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
