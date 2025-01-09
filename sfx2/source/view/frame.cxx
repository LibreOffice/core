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

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <tools/svborder.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <appdata.hxx>
#include <sfx2/app.hxx>
#include <sfx2/event.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/frmdescr.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include "impframe.hxx"
#include <utility>
#include <workwin.hxx>
#include <sfx2/ipclient.hxx>
#include <vector>

using namespace com::sun::star;

static std::vector<SfxFrame*> gaFramesArr_Impl;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;

SfxPoolItem* SfxUnoAnyItem::CreateDefault()
{
    return new SfxUnoAnyItem(0, uno::Any());
}

SfxPoolItem* SfxUnoFrameItem::CreateDefault()
{
    return new SfxUnoFrameItem();
}
void SfxFrame::Construct_Impl()
{
    m_pImpl.reset(new SfxFrame_Impl);
    gaFramesArr_Impl.push_back( this );
}


SfxFrame::~SfxFrame()
{
    RemoveTopFrame_Impl( this );
    m_pWindow.disposeAndClear();

    auto it = std::find( gaFramesArr_Impl.begin(), gaFramesArr_Impl.end(), this );
    if ( it != gaFramesArr_Impl.end() )
        gaFramesArr_Impl.erase( it );

    delete m_pImpl->pDescr;
}

bool SfxFrame::DoClose()
{
    // Actually, one more PrepareClose is still needed!
    bool bRet = false;
    if ( !m_pImpl->bClosing )
    {
        m_pImpl->bClosing = true;
        CancelTransfers();

        // now close frame; it will be deleted if this call is successful, so don't use any members after that!
        bRet = true;
        try
        {
            Reference< XCloseable > xCloseable  ( m_pImpl->xFrame, UNO_QUERY );
            if (xCloseable.is())
                xCloseable->close(true);
            else if ( m_pImpl->xFrame.is() )
            {
                Reference < XFrame > xFrame = m_pImpl->xFrame;
                xFrame->setComponent( Reference < css::awt::XWindow >(), Reference < XController >() );
                xFrame->dispose();
            }
            else
                DoClose_Impl();
        }
        catch( css::util::CloseVetoException& )
        {
            m_pImpl->bClosing = false;
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
    if ( m_pImpl->pCurrentViewFrame )
        pBindings = &m_pImpl->pCurrentViewFrame->GetBindings();

    // For internal tasks Controllers and Tools must be cleared
    if ( m_pImpl->pWorkWin )
        m_pImpl->pWorkWin->DeleteControllers_Impl();

    if ( m_pImpl->pCurrentViewFrame )
        m_pImpl->pCurrentViewFrame->Close();

    if ( m_pImpl->bOwnsBindings )
    {
        delete pBindings;
        pBindings = nullptr;
    }

    delete this;
}

bool SfxFrame::DocIsModified_Impl()
{
    return m_pImpl->pCurrentViewFrame && m_pImpl->pCurrentViewFrame->GetObjectShell() &&
            m_pImpl->pCurrentViewFrame->GetObjectShell()->IsModified();
}

bool SfxFrame::PrepareClose_Impl( bool bUI )
{
    bool bRet = true;

    // prevent recursive calls
    if( !m_pImpl->bPrepClosing )
    {
        m_pImpl->bPrepClosing = true;

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

        m_pImpl->bPrepClosing = false;
    }

    if ( bRet && m_pImpl->pWorkWin )
        // if closing was accepted by the component the UI subframes must be asked also
        bRet = m_pImpl->pWorkWin->PrepareClose_Impl();

    return bRet;
}


bool SfxFrame::IsClosing_Impl() const
{
    return m_pImpl->bClosing;
}

void SfxFrame::SetIsClosing_Impl()
{
    m_pImpl->bClosing = true;
}

void SfxFrame::CancelTransfers()
{
    if( m_pImpl->bInCancelTransfers )
        return;

    m_pImpl->bInCancelTransfers = true;
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
        m_pImpl->bInCancelTransfers = false;
}

SfxViewFrame* SfxFrame::GetCurrentViewFrame() const
{
    return m_pImpl->pCurrentViewFrame;
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
    return m_pImpl->pCurrentViewFrame ?
            m_pImpl->pCurrentViewFrame->GetObjectShell() :
            nullptr;
}

void SfxFrame::SetCurrentViewFrame_Impl( SfxViewFrame *pFrame )
{
    m_pImpl->pCurrentViewFrame = pFrame;
}

bool SfxFrame::GetHasTitle() const
{
    return m_pImpl->mbHasTitle;
}

void SfxFrame::SetHasTitle( bool n )
{
    m_pImpl->mbHasTitle = n;
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
    GetDescriptor()->SetActualURL();

    // Mark FileOpen parameter
    SfxItemSet& rItemSet = pMed->GetItemSet();

    const std::shared_ptr<const SfxFilter>& pFilter = pMed->GetFilter();
    OUString aFilter;
    if ( pFilter )
        aFilter = pFilter->GetFilterName();

    const SfxStringItem* pRefererItem = rItemSet.GetItem<SfxStringItem>(SID_REFERER, false);
    const SfxStringItem* pOptionsItem = rItemSet.GetItem<SfxStringItem>(SID_FILE_FILTEROPTIONS, false);
    const SfxStringItem* pTitle1Item = rItemSet.GetItem<SfxStringItem>(SID_DOCINFO_TITLE, false);

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

    if ( !m_pImpl->pDescr )
    {
        DBG_ASSERT( true, "No TopLevel-Frame, but no Descriptor!" );
        m_pImpl->pDescr = new SfxFrameDescriptor;
        if ( GetCurrentDocument() )
            m_pImpl->pDescr->SetURL( GetCurrentDocument()->GetMedium()->GetOrigURL() );
    }
    return m_pImpl->pDescr;
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

bool SfxFrameItem::operator==( const SfxPoolItem &rItem ) const
{
     return SfxPoolItem::operator==(rItem) &&
        static_cast<const SfxFrameItem&>(rItem).pFrame == pFrame &&
        static_cast<const SfxFrameItem&>(rItem).wFrame == wFrame;
}

SfxFrameItem* SfxFrameItem::Clone( SfxItemPool *) const
{
    // Every ::Clone implementation *needs* to also ensure that
    // the WhichID gets set/copied at the created Item (!)
    SfxFrameItem* pNew = new SfxFrameItem(Which(), wFrame);
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


SfxUnoAnyItem::SfxUnoAnyItem( sal_uInt16 nWhichId, const css::uno::Any& rAny )
    : SfxPoolItem( nWhichId )
{
    aValue = rAny;
}

bool SfxUnoAnyItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem)); (void)rItem;
    return false;
}

SfxUnoAnyItem* SfxUnoAnyItem::Clone( SfxItemPool *) const
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
    : SfxPoolItem( 0)
{
}

SfxUnoFrameItem::SfxUnoFrameItem( sal_uInt16 nWhichId, css::uno::Reference< css::frame::XFrame > i_xFrame )
    : SfxPoolItem( nWhichId )
    , m_xFrame(std::move( i_xFrame ))
{
}

bool SfxUnoFrameItem::operator==( const SfxPoolItem& i_rItem ) const
{
    return SfxPoolItem::operator==(i_rItem) &&
        static_cast< const SfxUnoFrameItem& >( i_rItem ).m_xFrame == m_xFrame;
}

SfxUnoFrameItem* SfxUnoFrameItem::Clone( SfxItemPool* ) const
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
    if ( m_pImpl->pCurrentViewFrame && m_pImpl->pCurrentViewFrame->GetViewShell() )
        return m_pImpl->pCurrentViewFrame->GetViewShell()->GetController();
    else
        return css::uno::Reference< css::frame::XController > ();
}

const css::uno::Reference< css::frame::XFrame >&  SfxFrame::GetFrameInterface() const
{
    return m_pImpl->xFrame;
}

void SfxFrame::SetFrameInterface_Impl( const css::uno::Reference< css::frame::XFrame >& rFrame )
{
    m_pImpl->xFrame = rFrame;
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
        m_pImpl->xFrame->getContainerWindow()->setVisible( true );
        Reference < css::awt::XTopWindow > xTopWindow( m_pImpl->xFrame->getContainerWindow(), UNO_QUERY );
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
    m_pImpl->bOwnsBindings = bSet;
}

bool SfxFrame::OwnsBindings_Impl() const
{
    return m_pImpl->bOwnsBindings;
}

void SfxFrame::SetToolSpaceBorderPixel_Impl( const SvBorder& rBorder )
{
    m_pImpl->aBorder = rBorder;
    SfxViewFrame *pF = GetCurrentViewFrame();
    if ( !pF )
        return;

    Point aPos ( rBorder.Left(), rBorder.Top() );
    Size aSize( GetWindow().GetOutputSizePixel() );
    tools::Long nDeltaX = rBorder.Left() + rBorder.Right();
    if ( aSize.Width() > nDeltaX )
        aSize.AdjustWidth( -nDeltaX );
    else
        aSize.setWidth( 0 );

    tools::Long nDeltaY = rBorder.Top() + rBorder.Bottom();
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
    return m_pImpl->pWorkWin;
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
            TOOLS_WARN_EXCEPTION( "sfx.view", "SfxFrame::CreateWorkWindow_Impl: Exception caught. Please try to submit a reproducible bug!");
        }
    }

    m_pImpl->pWorkWin = new SfxWorkWindow( &pFrame->GetWindow(), this, pFrame );
}

void SfxFrame::GrabFocusOnComponent_Impl()
{
    if ( m_pImpl->bReleasingComponent )
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
    m_pImpl->bReleasingComponent = true;
}

bool SfxFrame::IsInPlace() const
{
    return m_pImpl->bInPlace;
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
                SfxObjectShell* pDoc
                    = SfxObjectShell::GetShellFromComponent(pClient->GetObject()->getComponent());
                SfxViewFrame* pFrame = SfxViewFrame::GetFirst(pDoc);
                pWork = pFrame ? pFrame->GetFrame().GetWorkWindow_Impl() : nullptr;
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
            SetToolSpaceBorderPixel_Impl( m_pImpl->aBorder );
        }
    }
    else if ( m_pImpl->pCurrentViewFrame )
    {
        m_pImpl->pCurrentViewFrame->GetWindow().SetSizePixel( GetWindow().GetOutputSizePixel() );
    }

}

SfxFrame* SfxFrame::GetFirst()
{
    return gaFramesArr_Impl.empty() ? nullptr : gaFramesArr_Impl.front();
}

SfxFrame* SfxFrame::GetNext( SfxFrame& rFrame )
{
    auto it = std::find( gaFramesArr_Impl.begin(), gaFramesArr_Impl.end(), &rFrame );
    if ( it != gaFramesArr_Impl.end() && (++it) != gaFramesArr_Impl.end() )
        return *it;
    else
        return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
