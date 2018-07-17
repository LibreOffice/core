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

#include <hintids.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#include <osl/thread.hxx>
#include <salhelper/condition.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/objsh.hxx>
#include <editeng/boxitem.hxx>
#include <svx/svxids.hrc>
#include <sfx2/linkmgr.hxx>
#include <sfx2/event.hxx>
#include <svtools/soerr.hxx>
#include <fmtfsize.hxx>
#include <fmtanchr.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <pam.hxx>
#include <editsh.hxx>
#include <swtable.hxx>
#include <docary.hxx>
#include <swevent.hxx>
#include <swbaslnk.hxx>
#include <swserv.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <hints.hxx>
#include <tabfrm.hxx>
#include <cntfrm.hxx>
#include <htmltbl.hxx>
#include <calbck.hxx>
#include <memory>

using namespace com::sun::star;

static bool SetGrfFlySize( const Size& rGrfSz, SwGrfNode* pGrfNd, const Size &rOrigGrfSize );


static void lcl_CallModify( SwGrfNode& rGrfNd, SfxPoolItem& rItem )
{
    //call first all not SwNoTextFrames, then the SwNoTextFrames.
    //              The reason is, that in the SwNoTextFrames the Graphic
    //              after a Paint will be swapped out! So all other "behind"
    //              them haven't a loaded Graphic.
    rGrfNd.LockModify();
    {
        SwIterator<SwClient,SwGrfNode> aIter(rGrfNd);
        for(SwClient* pLast = aIter.First(); pLast; pLast = aIter.Next())
            if(dynamic_cast<const SwContentFrame*>( pLast) ==  nullptr)
                pLast->ModifyNotification(&rItem, &rItem);
    }
    {
        SwIterator<SwContentFrame,SwGrfNode> aIter(rGrfNd);
        for(SwClient* pLast = aIter.First(); pLast; pLast = aIter.Next())
            pLast->ModifyNotification(&rItem, &rItem);
    }
    rGrfNd.UnlockModify();
}

::sfx2::SvBaseLink::UpdateResult SwBaseLink::DataChanged(
    const OUString& rMimeType, const uno::Any & rValue )
{
    if( !m_pContentNode )
    {
        OSL_ENSURE(false, "DataChanged without ContentNode" );
        return ERROR_GENERAL;
    }

    SwDoc* pDoc = m_pContentNode->GetDoc();
    if( pDoc->IsInDtor() || ChkNoDataFlag() || m_bIgnoreDataChanged )
    {
        m_bIgnoreDataChanged = false;
        return SUCCESS;
    }

    SotClipboardFormatId nFormat = SotExchange::GetFormatIdFromMimeType( rMimeType );

    if( m_pContentNode->IsNoTextNode() &&
        nFormat == sfx2::LinkManager::RegisterStatusInfoId() )
    {
        // Only a status change - serve Events?
        OUString sState;

        if( rValue.hasValue() && ( rValue >>= sState ))
        {
            SvMacroItemId nEvent = SvMacroItemId::NONE;
            switch( sState.toInt32() )
            {
            case sfx2::LinkManager::STATE_LOAD_OK:      nEvent = SvMacroItemId::OnImageLoadDone;  break;
            case sfx2::LinkManager::STATE_LOAD_ERROR:   nEvent = SvMacroItemId::OnImageLoadError; break;
            case sfx2::LinkManager::STATE_LOAD_ABORT:   nEvent = SvMacroItemId::OnImageLoadCancel; break;
            }

            SwFrameFormat* pFormat;
            if( nEvent != SvMacroItemId::NONE && nullptr != ( pFormat = m_pContentNode->GetFlyFormat() ))
            {
                SwCallMouseEvent aCallEvent;
                aCallEvent.Set( EVENT_OBJECT_IMAGE, pFormat );
                pDoc->CallEvent( nEvent, aCallEvent );
            }
        }
        return SUCCESS; // That's it!
    }

    bool bUpdate = false;
    bool bFrameInPaint = false;
    Size aGrfSz, aOldSz;

    SwGrfNode* pSwGrfNode = nullptr;

    if (m_pContentNode->IsGrfNode())
    {
        pSwGrfNode = m_pContentNode->GetGrfNode();
        assert(pSwGrfNode && "Error, pSwGrfNode expected when node answers IsGrfNode() with true (!)");
        aOldSz = pSwGrfNode->GetTwipSize();
        const GraphicObject& rGrfObj = pSwGrfNode->GetGrfObj();

        bFrameInPaint = pSwGrfNode->IsFrameInPaint();

        pSwGrfNode->SetGraphicArrived(true);

        Graphic aGrf;

        OUString sReferer;
        SfxObjectShell * sh = pDoc->GetPersist();
        if (sh != nullptr && sh->HasName())
        {
            sReferer = sh->GetMedium()->GetName();
        }

        if( sfx2::LinkManager::GetGraphicFromAny(rMimeType, rValue, sReferer, aGrf) &&
            ( GraphicType::Default != aGrf.GetType() ||
              GraphicType::Default != rGrfObj.GetType() ) )
        {
            aGrfSz = ::GetGraphicSizeTwip( aGrf, nullptr );

            pSwGrfNode->SetGraphic(aGrf, "");
            bUpdate = true;

            // Always use the correct graphic size
            if( aGrfSz.Height() && aGrfSz.Width() &&
                aOldSz.Height() && aOldSz.Width() &&
                aGrfSz != aOldSz )
            {
                pSwGrfNode->SetTwipSize(aGrfSz);
                aOldSz = aGrfSz;
            }
        }
    }
    else if( m_pContentNode->IsOLENode() )
        bUpdate = true;

    if ( !bUpdate || bFrameInPaint )
        return SUCCESS;

    if (bUpdate)
    {
        if (!SetGrfFlySize(aGrfSz, pSwGrfNode, aOldSz))
        {
            SwMsgPoolItem aMsgHint(RES_GRAPHIC_ARRIVED);
            lcl_CallModify(*pSwGrfNode, aMsgHint);
            return SUCCESS;
        }
    }

    return SUCCESS;
}

static bool SetGrfFlySize( const Size& rGrfSz, SwGrfNode* pGrfNd, const Size& rOrigGrfSize )
{
    bool bRet = false;
    SwViewShell *pSh = pGrfNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    std::unique_ptr<CurrShell> pCurr;
    if ( pGrfNd->GetDoc()->GetEditShell() )
        pCurr.reset(new CurrShell( pSh ));

    Size aSz = rOrigGrfSize;
    if ( !(aSz.Width() && aSz.Height()) &&
            rGrfSz.Width() && rGrfSz.Height() )
    {
        SwFrameFormat* pFormat = nullptr;
        if (pGrfNd->IsChgTwipSize())
            pFormat = pGrfNd->GetFlyFormat();
        if (nullptr != pFormat)
        {
            Size aCalcSz( aSz );
            if ( !aSz.Height() && aSz.Width() )
                // Calculate the right height
                aCalcSz.setHeight( rGrfSz.Height() *
                        aSz.Width() / rGrfSz.Width() );
            else if ( !aSz.Width() && aSz.Height() )
                // Calculate the right width
                aCalcSz.setWidth( rGrfSz.Width() *
                        aSz.Height() / rGrfSz.Height() );
            else
                // Take over height and width
                aCalcSz = rGrfSz;

            const SvxBoxItem     &rBox = pFormat->GetBox();
            aCalcSz.AdjustWidth(rBox.CalcLineSpace(SvxBoxItemLine::LEFT) +
                               rBox.CalcLineSpace(SvxBoxItemLine::RIGHT) );
            aCalcSz.AdjustHeight(rBox.CalcLineSpace(SvxBoxItemLine::TOP) +
                               rBox.CalcLineSpace(SvxBoxItemLine::BOTTOM) );
            const SwFormatFrameSize& rOldAttr = pFormat->GetFrameSize();
            if( rOldAttr.GetSize() != aCalcSz )
            {
                SwFormatFrameSize aAttr( rOldAttr  );
                aAttr.SetSize( aCalcSz );
                pFormat->SetFormatAttr( aAttr );
                bRet = true;
            }

            if( !aSz.Width() )
            {
                // If the graphic is anchored in a table, we need to recalculate
                // the table rows
                const SwDoc *pDoc = pGrfNd->GetDoc();
                const SwPosition* pAPos = pFormat->GetAnchor().GetContentAnchor();
                SwNode *pANd;
                SwTableNode *pTableNd;
                if( pAPos &&
                    nullptr != (pANd = & pAPos->nNode.GetNode()) &&
                    nullptr != (pTableNd = pANd->FindTableNode()) )
                {
                    const bool bLastGrf = !pTableNd->GetTable().DecGrfsThatResize();
                    SwHTMLTableLayout *pLayout =
                        pTableNd->GetTable().GetHTMLTableLayout();
                    if( pLayout )
                    {
                        const sal_uInt16 nBrowseWidth =
                                    pLayout->GetBrowseWidthByTable( *pDoc );
                        if ( nBrowseWidth )
                        {
                            pLayout->Resize( nBrowseWidth, true, true,
                                             bLastGrf ? HTMLTABLE_RESIZE_NOW
                                                      : 500 );
                        }
                    }
                }
            }
        }

        // SetTwipSize rescales an ImageMap if needed for which
        // it requires the Frame Format
        pGrfNd->SetTwipSize( rGrfSz );
    }

    return bRet;
}

bool SwBaseLink::SwapIn( bool bWaitForData, bool bNativFormat )
{
    m_bInSwapIn = true;

    if( !GetObj() && ( bNativFormat || ( !IsSynchron() && bWaitForData ) ))
    {
        AddNextRef();
        GetRealObject_();
        ReleaseRef();
    }

    bool bRes = false;

    if( GetObj() )
    {
        OUString aMimeType( SotExchange::GetFormatMimeType( GetContentType() ));
        uno::Any aValue;
        (void)GetObj()->GetData( aValue, aMimeType, !IsSynchron() && bWaitForData );

        if( bWaitForData && !GetObj() )
        {
            OSL_ENSURE( false, "The SvxFileObject was deleted in a GetData!" );
        }
        else
        {
            bRes = aValue.hasValue();
            if ( bRes )
            {
                // The Flag needs to be reset on a SwapIn, because
                // we want to reapply the data.
                m_bIgnoreDataChanged = false;
                DataChanged( aMimeType, aValue );
            }
        }
    }
    else if( !IsSynchron() && bWaitForData )
    {
        SetSynchron( true );
        bRes = Update();
        SetSynchron( false );
    }
    else
        bRes = Update();

    m_bInSwapIn = false;
    return bRes;
}

void SwBaseLink::Closed()
{
    if( m_pContentNode && !m_pContentNode->GetDoc()->IsInDtor() )
    {
        // Delete the connection
        if( m_pContentNode->IsGrfNode() )
            static_cast<SwGrfNode*>(m_pContentNode)->ReleaseLink();
    }
    SvBaseLink::Closed();
}

const SwNode* SwBaseLink::GetAnchor() const
{
    if (m_pContentNode)
    {
        SwFrameFormat *const pFormat = m_pContentNode->GetFlyFormat();
        if (pFormat)
        {
            const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
            SwPosition const*const pAPos = rAnchor.GetContentAnchor();
            if (pAPos &&
                ((RndStdIds::FLY_AS_CHAR == rAnchor.GetAnchorId()) ||
                 (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
                 (RndStdIds::FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
                 (RndStdIds::FLY_AT_PARA == rAnchor.GetAnchorId())))
            {
                    return &pAPos->nNode.GetNode();
            }
            return nullptr;
        }
    }

    OSL_ENSURE( false, "GetAnchor is not shadowed" );
    return nullptr;
}

bool SwBaseLink::IsRecursion( const SwBaseLink* pChkLnk ) const
{
    tools::SvRef<SwServerObject> aRef( static_cast<SwServerObject*>(GetObj()) );
    if( aRef.is() )
    {
        // As it's a ServerObject, we query all contained Links
        // if we are contained in them. Else we have a recursion.
        return aRef->IsLinkInServer( pChkLnk );
    }
    return false;
}

bool SwBaseLink::IsInRange( sal_uLong, sal_uLong, sal_Int32, sal_Int32 ) const
{
    // Not Graphic or OLE Links
    // Fields or Sections have their own derivation!
    return false;
}

SwBaseLink::~SwBaseLink()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
