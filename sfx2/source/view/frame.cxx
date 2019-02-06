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
#include <tools/globname.hxx>
#include <com/sun/star/awt/PosSize.hpp>

#include <appdata.hxx>
#include <sfx2/app.hxx>
#include <sfx2/event.hxx>
#include <sfx2/unoctitm.hxx>
#include <sfx2/frame.hxx>
#include <arrdecl.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/frmdescr.hxx>
#include <openflag.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/fcontnr.hxx>
#include "impframe.hxx"
#include <workwin.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/msgpool.hxx>
#include <objshimp.hxx>
#include <sfx2/ipclient.hxx>
#include <childwinimpl.hxx>
#include <vector>

#ifdef DBG_UTIL
#include <sfx2/frmhtmlw.hxx>
#endif

using namespace com::sun::star;

static std::vector<SfxFrame*>* pFramesArr_Impl=nullptr;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;

SfxPoolItem* SfxUnoAnyItem::CreateDefault()
{
    return new SfxUnoAnyItem();
}

SfxPoolItem* SfxUnoFrameItem::CreateDefault()
{
    return new SfxUnoFrameItem();
}

void SfxFrame::Construct_Impl()
{
    pImpl.reset(new SfxFrame_Impl);
    if ( !pFramesArr_Impl )
        pFramesArr_Impl = new std::vector<SfxFrame*>;
    pFramesArr_Impl->push_back( this );
}


SfxFrame::~SfxFrame()
{
    RemoveTopFrame_Impl( this );
    pWindow.disposeAndClear();

    auto it = std::find( pFramesArr_Impl->begin(), pFramesArr_Impl->end(), this );
    if ( it != pFramesArr_Impl->end() )
        pFramesArr_Impl->erase( it );

    delete pImpl->pDescr;
}

bool SfxFrame::DoClose()
{
    // Actually, one more PrepareClose is still needed!
    bool bRet = false;
    if ( !pImpl->bClosing )
    {
        pImpl->bClosing = true;
        CancelTransfers();

        // now close frame; it will be deleted if this call is successful, so don't use any members after that!
        bRet = true;
        try
        {
            Reference< XCloseable > xCloseable  ( pImpl->xFrame, UNO_QUERY );
            if (xCloseable.is())
                xCloseable->close(true);
            else if ( pImpl->xFrame.is() )
            {
                Reference < XFrame > xFrame = pImpl->xFrame;
                xFrame->setComponent( Reference < css::awt::XWindow >(), Reference < XController >() );
                xFrame->dispose();
            }
            else
                DoClose_Impl();
        }
        catch( css::util::CloseVetoException& )
        {
            pImpl->bClosing = false;
            bRet = false;
        }
        catch( css::lang::DisposedException& )
        {
        }
    }

    return bRet;
}

void SfxFrame::DoClose_Impl()
{
    SfxBindings* pBindings = nullptr;
    if ( pImpl->pCurrentViewFrame )
        pBindings = &pImpl->pCurrentViewFrame->GetBindings();

    // For internal tasks Controllers and Tools must be cleared
    if ( pImpl->pWorkWin )
        pImpl->pWorkWin->DeleteControllers_Impl();

    if ( pImpl->pCurrentViewFrame )
        pImpl->pCurrentViewFrame->Close();

    if ( pImpl->bOwnsBindings )
        DELETEZ( pBindings );

    Close();
}

bool SfxFrame::DocIsModified_Impl()
{
    return pImpl->pCurrentViewFrame && pImpl->pCurrentViewFrame->GetObjectShell() &&
            pImpl->pCurrentViewFrame->GetObjectShell()->IsModified();
}

bool SfxFrame::PrepareClose_Impl( bool bUI )
{
    bool bRet = true;

    // prevent recursive calls
    if( !pImpl->bPrepClosing )
    {
        pImpl->bPrepClosing = true;

        SfxObjectShell* pCur = GetCurrentDocument() ;
        if( pCur )
        {
            // SFX components have a known behaviour
            // First check if this frame is the only view to its current document
            bool bOther = false;
            for ( const SfxViewFrame *pFrame = SfxViewFrame::GetFirst( pCur );
                    !bOther && pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, pCur ) )
            {
                bOther = ( &pFrame->GetFrame() != this );
            }

            SfxGetpApp()->NotifyEvent( SfxViewEventHint(SfxEventHintId::PrepareCloseView, GlobalEventConfig::GetEventName( GlobalEventId::PREPARECLOSEVIEW ), pCur, GetController() ) );

            if ( bOther )
                // if there are other views only the current view of this frame must be asked
                bRet = GetCurrentViewFrame()->GetViewShell()->PrepareClose( bUI );
            else
                // otherwise ask the document
                bRet = pCur->PrepareClose( bUI );
        }

        pImpl->bPrepClosing = false;
    }

    if ( bRet && pImpl->pWorkWin )
        // if closing was accepted by the component the UI subframes must be asked also
        bRet = pImpl->pWorkWin->PrepareClose_Impl();

    return bRet;
}


bool SfxFrame::IsClosing_Impl() const
{
    return pImpl->bClosing;
}

void SfxFrame::SetIsClosing_Impl()
{
    pImpl->bClosing = true;
}

void SfxFrame::CancelTransfers()
{
    if( pImpl->bInCancelTransfers )
        return;

    pImpl->bInCancelTransfers = true;
    SfxObjectShell* pObj = GetCurrentDocument();
    if( pObj ) //&& !( pObj->Get_Impl()->nLoadedFlags & SfxLoadedFlags::ALL ))
    {
        SfxViewFrame* pFrm;
        for( pFrm = SfxViewFrame::GetFirst( pObj );
             pFrm && &pFrm->GetFrame() == this;
             pFrm = SfxViewFrame::GetNext( *pFrm, pObj ) ) ;
        // No more Frame in Document -> Cancel
        if( !pFrm )
        {
            pObj->CancelTransfers();
            GetCurrentDocument()->Broadcast( SfxHint(SfxHintId::TitleChanged) );
        }
    }

    //  Check if StarOne-Loader should be canceled
    SfxFrameWeakRef wFrame( this );
    if (wFrame.is())
        pImpl->bInCancelTransfers = false;
}

SfxViewFrame* SfxFrame::GetCurrentViewFrame() const
{
    return pImpl->pCurrentViewFrame;
}

bool SfxFrame::IsAutoLoadLocked_Impl() const
{
    // Its own Document is locked?
    const SfxObjectShell* pObjSh = GetCurrentDocument();
    if ( !pObjSh || !pObjSh->IsAutoLoadLocked() )
        return false;

    // otherwise allow AutoLoad
    return true;
}

SfxObjectShell* SfxFrame::GetCurrentDocument() const
{
    return pImpl->pCurrentViewFrame ?
            pImpl->pCurrentViewFrame->GetObjectShell() :
            nullptr;
}

void SfxFrame::SetCurrentViewFrame_Impl( SfxViewFrame *pFrame )
{
    pImpl->pCurrentViewFrame = pFrame;
}

bool SfxFrame::GetHasTitle() const
{
    return pImpl->mbHasTitle;
}

void SfxFrame::SetHasTitle( bool n )
{
    pImpl->mbHasTitle = n;
}

void SfxFrame::GetViewData_Impl()
{
    // Update all modifiable data between load and unload, the
    // fixed data is only processed once (after PrepareForDoc_Impl in
    // updateDescriptor) to save time.

    SfxViewFrame* pViewFrame = GetCurrentViewFrame();
    if( pViewFrame && pViewFrame->GetViewShell() )
    {
        SfxItemSet *pSet = GetDescriptor()->GetArgs();
        if ( GetController().is() && pSet->GetItemState( SID_VIEW_DATA ) != SfxItemState::SET )
        {
            css::uno::Any aData = GetController()->getViewData();
            pSet->Put( SfxUnoAnyItem( SID_VIEW_DATA, aData ) );
        }

        if ( pViewFrame->GetCurViewId() )
            pSet->Put( SfxUInt16Item( SID_VIEW_ID, static_cast<sal_uInt16>(pViewFrame->GetCurViewId()) ) );
    }
}

void SfxFrame::UpdateDescriptor( SfxObjectShell const *pDoc )
{
    // For PrepareForDoc_Impl frames, the descriptor of the updated
    // and new itemset to be initialized. All data fir restoring the view
    // are thus saved. If the document be replaced, GetViewData_Impl (so)
    // the latest information hinzugef by "added. All together then the
    // browser-history saved in. When you activate such frame pick entry
    // is complete itemsets and the descriptor in the OpenDoc sent;.
    // Here only the fixed properties identified "other adjustable, the
    // retrieved by GetViewData (saves time).

    assert(pDoc && "NULL-Document inserted ?!");

    const SfxMedium *pMed = pDoc->GetMedium();
    GetDescriptor()->SetActualURL( pMed->GetOrigURL() );

    // Mark FileOpen parameter
    SfxItemSet* pItemSet = pMed->GetItemSet();

    const std::shared_ptr<const SfxFilter>& pFilter = pMed->GetFilter();
    OUString aFilter;
    if ( pFilter )
        aFilter = pFilter->GetFilterName();

    const SfxStringItem* pRefererItem = SfxItemSet::GetItem<SfxStringItem>(pItemSet, SID_REFERER, false);
    const SfxStringItem* pOptionsItem = SfxItemSet::GetItem<SfxStringItem>(pItemSet, SID_FILE_FILTEROPTIONS, false);
    const SfxStringItem* pTitle1Item = SfxItemSet::GetItem<SfxStringItem>(pItemSet, SID_DOCINFO_TITLE, false);

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


SfxFrameDescriptor* SfxFrame::GetDescriptor() const
{
    // Create a FrameDescriptor On Demand; if there is no TopLevel-Frame
    // will result in an error, as no valid link is created.

    if ( !pImpl->pDescr )
    {
        DBG_ASSERT( true, "No TopLevel-Frame, but no Descriptor!" );
        pImpl->pDescr = new SfxFrameDescriptor;
        if ( GetCurrentDocument() )
            pImpl->pDescr->SetURL( GetCurrentDocument()->GetMedium()->GetOrigURL() );
    }
    return pImpl->pDescr;
}

void SfxFrame::GetDefaultTargetList(TargetList& rList)
{
    // An empty string for 'No Target'
    rList.emplace_back( );
    rList.emplace_back( "_top" );
    rList.emplace_back( "_parent" );
    rList.emplace_back( "_blank" );
    rList.emplace_back( "_self" );
}

void SfxFrame::InsertTopFrame_Impl( SfxFrame* pFrame )
{
    auto& rArr = SfxGetpApp()->Get_Impl()->vTopFrames;
    rArr.push_back( pFrame );
}

void SfxFrame::RemoveTopFrame_Impl( SfxFrame* pFrame )
{
    auto& rArr = SfxGetpApp()->Get_Impl()->vTopFrames;
    auto it = std::find( rArr.begin(), rArr.end(), pFrame );
    if ( it != rArr.end() )
        rArr.erase( it );
}

SfxFrameItem::SfxFrameItem( sal_uInt16 nWhichId, SfxViewFrame const *p )
    : SfxPoolItem( nWhichId ), pFrame( p ? &p->GetFrame() : nullptr )
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

bool SfxFrameItem::operator==( const SfxPoolItem &rItem ) const
{
     return static_cast<const SfxFrameItem&>(rItem).pFrame == pFrame &&
         static_cast<const SfxFrameItem&>(rItem).wFrame == wFrame;
}

SfxPoolItem* SfxFrameItem::Clone( SfxItemPool *) const
{
    SfxFrameItem* pNew = new SfxFrameItem( wFrame);
    pNew->pFrame = pFrame;
    return pNew;
}

bool SfxFrameItem::QueryValue( css::uno::Any& rVal, sal_uInt8 ) const
{
    if ( wFrame )
    {
        rVal <<= wFrame->GetFrameInterface();
        return true;
    }

    return false;
}

bool SfxFrameItem::PutValue( const css::uno::Any& rVal, sal_uInt8 )
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
                return true;
            }

            pFr = SfxFrame::GetNext( *pFr );
        }
        return true;
    }

    return false;
}

SfxUnoAnyItem::SfxUnoAnyItem()
    : SfxPoolItem( 0 )
{
}

SfxUnoAnyItem::SfxUnoAnyItem( sal_uInt16 nWhichId, const css::uno::Any& rAny )
    : SfxPoolItem( nWhichId )
{
    aValue = rAny;
}

bool SfxUnoAnyItem::operator==( const SfxPoolItem& /*rItem*/ ) const
{
    return false;
}

SfxPoolItem* SfxUnoAnyItem::Clone( SfxItemPool *) const
{
    return new SfxUnoAnyItem( *this );
}

bool SfxUnoAnyItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal = aValue;
    return true;
}

bool SfxUnoAnyItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    aValue = rVal;
    return true;
}

SfxUnoFrameItem::SfxUnoFrameItem()
    : SfxPoolItem()
    , m_xFrame()
{
}

SfxUnoFrameItem::SfxUnoFrameItem( sal_uInt16 nWhichId, const css::uno::Reference< css::frame::XFrame >& i_rFrame )
    : SfxPoolItem( nWhichId )
    , m_xFrame( i_rFrame )
{
}

bool SfxUnoFrameItem::operator==( const SfxPoolItem& i_rItem ) const
{
    return dynamic_cast< const SfxUnoFrameItem* >(&i_rItem)  !=  nullptr && static_cast< const SfxUnoFrameItem& >( i_rItem ).m_xFrame == m_xFrame;
}

SfxPoolItem* SfxUnoFrameItem::Clone( SfxItemPool* ) const
{
    return new SfxUnoFrameItem( *this );
}

bool SfxUnoFrameItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= m_xFrame;
    return true;
}

bool SfxUnoFrameItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    return ( rVal >>= m_xFrame );
}

css::uno::Reference< css::frame::XController > SfxFrame::GetController() const
{
    if ( pImpl->pCurrentViewFrame && pImpl->pCurrentViewFrame->GetViewShell() )
        return pImpl->pCurrentViewFrame->GetViewShell()->GetController();
    else
        return css::uno::Reference< css::frame::XController > ();
}

const css::uno::Reference< css::frame::XFrame >&  SfxFrame::GetFrameInterface() const
{
    return pImpl->xFrame;
}

void SfxFrame::SetFrameInterface_Impl( const css::uno::Reference< css::frame::XFrame >& rFrame )
{
    pImpl->xFrame = rFrame;
    css::uno::Reference< css::frame::XDispatchRecorder > xRecorder;
    if ( !rFrame.is() && GetCurrentViewFrame() )
        GetCurrentViewFrame()->GetBindings().SetRecorder_Impl( xRecorder );
}

void SfxFrame::Appear()
{
    if ( GetCurrentViewFrame() )
    {
        GetCurrentViewFrame()->Show();
        GetWindow().Show();
        pImpl->xFrame->getContainerWindow()->setVisible( true );
        Reference < css::awt::XTopWindow > xTopWindow( pImpl->xFrame->getContainerWindow(), UNO_QUERY );
        if ( xTopWindow.is() )
            xTopWindow->toFront();
    }
}

void SfxFrame::AppearWithUpdate()
{
    Appear();
    if ( GetCurrentViewFrame() )
        GetCurrentViewFrame()->GetDispatcher()->Update_Impl( true );
}

void SfxFrame::SetOwnsBindings_Impl( bool bSet )
{
    pImpl->bOwnsBindings = bSet;
}

bool SfxFrame::OwnsBindings_Impl() const
{
    return pImpl->bOwnsBindings;
}

void SfxFrame::SetToolSpaceBorderPixel_Impl( const SvBorder& rBorder )
{
    pImpl->aBorder = rBorder;
    SfxViewFrame *pF = GetCurrentViewFrame();
    if ( !pF )
        return;

    Point aPos ( rBorder.Left(), rBorder.Top() );
    Size aSize( GetWindow().GetOutputSizePixel() );
    long nDeltaX = rBorder.Left() + rBorder.Right();
    if ( aSize.Width() > nDeltaX )
        aSize.AdjustWidth( -nDeltaX );
    else
        aSize.setWidth( 0 );

    long nDeltaY = rBorder.Top() + rBorder.Bottom();
    if ( aSize.Height() > nDeltaY )
        aSize.AdjustHeight( -nDeltaY );
    else
        aSize.setHeight( 0 );

    pF->GetWindow().SetPosSizePixel( aPos, aSize );
}

tools::Rectangle SfxFrame::GetTopOuterRectPixel_Impl() const
{
    Size aSize( GetWindow().GetOutputSizePixel() );
    return tools::Rectangle( Point(), aSize );
}

SfxWorkWindow* SfxFrame::GetWorkWindow_Impl() const
{
    if ( pImpl->pWorkWin )
        return pImpl->pWorkWin;
    else
        return nullptr;
}

void SfxFrame::CreateWorkWindow_Impl()
{
    SfxFrame* pFrame = this;

    if ( IsInPlace() )
    {
        // this makes sense only for inplace activated objects
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
            OSL_FAIL("SfxFrame::CreateWorkWindow_Impl: Exception caught. Please try to submit a reproducible bug!");
        }
    }

    pImpl->pWorkWin = new SfxWorkWindow( &pFrame->GetWindow(), this, pFrame );
}

void SfxFrame::GrabFocusOnComponent_Impl()
{
    if ( pImpl->bReleasingComponent )
    {
        GetWindow().GrabFocus();
        return;
    }

    vcl::Window* pFocusWindow = &GetWindow();
    if ( GetCurrentViewFrame() && GetCurrentViewFrame()->GetViewShell() && GetCurrentViewFrame()->GetViewShell()->GetWindow() )
        pFocusWindow = GetCurrentViewFrame()->GetViewShell()->GetWindow();

    if( !pFocusWindow->HasChildPathFocus() )
        pFocusWindow->GrabFocus();
}

void SfxFrame::ReleasingComponent_Impl()
{
    pImpl->bReleasingComponent = true;
}

bool SfxFrame::IsInPlace() const
{
    return pImpl->bInPlace;
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
            SfxInPlaceClient* pClient = GetCurrentViewFrame()->GetViewShell() ? GetCurrentViewFrame()->GetViewShell()->GetUIActiveIPClient_Impl() : nullptr;
            if ( pClient )
            {
                uno::Reference < lang::XUnoTunnel > xObj( pClient->GetObject()->getComponent(), uno::UNO_QUERY );
                uno::Sequence < sal_Int8 > aSeq( SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence() );
                sal_Int64 nHandle = (xObj.is()? xObj->getSomething( aSeq ): 0);
                if ( nHandle )
                {
                    SfxObjectShell* pDoc = reinterpret_cast< SfxObjectShell* >( sal::static_int_cast< sal_IntPtr >( nHandle ));
                    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( pDoc );
                    pWork = pFrame ? pFrame->GetFrame().GetWorkWindow_Impl() : nullptr;
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
            SetToolSpaceBorderPixel_Impl( pImpl->aBorder );
        }
    }
    else if ( pImpl->pCurrentViewFrame )
    {
        pImpl->pCurrentViewFrame->GetWindow().SetSizePixel( GetWindow().GetOutputSizePixel() );
    }

}

SfxFrame* SfxFrame::GetFirst()
{
    if ( !pFramesArr_Impl )
        return nullptr;
    return pFramesArr_Impl->empty() ? nullptr : pFramesArr_Impl->front();
}

SfxFrame* SfxFrame::GetNext( SfxFrame& rFrame )
{
    auto it = std::find( pFramesArr_Impl->begin(), pFramesArr_Impl->end(), &rFrame );
    if ( it != pFramesArr_Impl->end() && (++it) != pFramesArr_Impl->end() )
        return *it;
    else
        return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
