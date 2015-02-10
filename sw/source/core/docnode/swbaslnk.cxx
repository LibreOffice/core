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

using namespace com::sun::star;

static bool SetGrfFlySize( const Size& rGrfSz, const Size& rFrmSz, SwGrfNode* pGrfNd, const Size &rOrigGrfSize );

TYPEINIT1( SwBaseLink, ::sfx2::SvBaseLink );

static void lcl_CallModify( SwGrfNode& rGrfNd, SfxPoolItem& rItem )
{
    //call fist all not SwNoTxtFrames, then the SwNoTxtFrames.
    //              The reason is, that in the SwNoTxtFrames the Graphic
    //              after a Paint will be swapped out! So all other "behind"
    //              them havent't a loaded Graphic.
    rGrfNd.LockModify();

    SwClientIter aIter( rGrfNd );   // TODO
    for( int n = 0; n < 2; ++n )
    {
        SwClient * pLast = aIter.GoStart();
        if( pLast )     // Were we able to jump to the beginning?
        {
            do {
                if( (0 == n) != pLast->ISA( SwCntntFrm ) )
                    pLast->ModifyNotification( &rItem, &rItem );
            } while( 0 != ( pLast = ++aIter ));
        }
    }
    rGrfNd.UnlockModify();
}

::sfx2::SvBaseLink::UpdateResult SwBaseLink::DataChanged(
    const OUString& rMimeType, const uno::Any & rValue )
{
    if( !pCntntNode )
    {
        OSL_ENSURE(false, "DataChanged without ContentNode" );
        return ERROR_GENERAL;
    }

    SwDoc* pDoc = pCntntNode->GetDoc();
    if( pDoc->IsInDtor() || ChkNoDataFlag() || bIgnoreDataChanged )
    {
        bIgnoreDataChanged = false;
        return SUCCESS;
    }

    sal_uLong nFmt = SotExchange::GetFormatIdFromMimeType( rMimeType );

    if( pCntntNode->IsNoTxtNode() &&
        nFmt == sfx2::LinkManager::RegisterStatusInfoId() )
    {
        // Only a status change - serve Events?
        OUString sState;

        if( rValue.hasValue() && ( rValue >>= sState ))
        {
            sal_uInt16 nEvent = 0;
            switch( sState.toInt32() )
            {
            case sfx2::LinkManager::STATE_LOAD_OK:      nEvent = SVX_EVENT_IMAGE_LOAD;  break;
            case sfx2::LinkManager::STATE_LOAD_ERROR:   nEvent = SVX_EVENT_IMAGE_ERROR; break;
            case sfx2::LinkManager::STATE_LOAD_ABORT:   nEvent = SVX_EVENT_IMAGE_ABORT; break;
            }

            SwFrmFmt* pFmt;
            if( nEvent && 0 != ( pFmt = pCntntNode->GetFlyFmt() ))
            {
                SwCallMouseEvent aCallEvent;
                aCallEvent.Set( EVENT_OBJECT_IMAGE, pFmt );
                pDoc->CallEvent( nEvent, aCallEvent );
            }
        }
        return SUCCESS; // That's it!
    }

    bool bUpdate = false;
    bool bGraphicArrived = false;
    bool bGraphicPieceArrived = false;
    bool bDontNotify = false;
    Size aGrfSz, aOldSz, aFrmFmtSz;

    SwGrfNode* pSwGrfNode = NULL;

    if (pCntntNode->IsGrfNode())
    {
        pSwGrfNode = pCntntNode->GetGrfNode();
        assert(pSwGrfNode && "Error, pSwGrfNode expected when node answers IsGrfNode() with true (!)");
        aOldSz = pSwGrfNode->GetTwipSize();
        const GraphicObject& rGrfObj = pSwGrfNode->GetGrfObj();

        bDontNotify = pSwGrfNode->IsFrameInPaint();

        bGraphicArrived = GetObj()->IsDataComplete();
        bGraphicPieceArrived = GetObj()->IsPending();
        pSwGrfNode->SetGraphicArrived( bGraphicArrived );

        Graphic aGrf;

        if( sfx2::LinkManager::GetGraphicFromAny( rMimeType, rValue, aGrf ) &&
            ( GRAPHIC_DEFAULT != aGrf.GetType() ||
              GRAPHIC_DEFAULT != rGrfObj.GetType() ) )
        {
            aGrfSz = ::GetGraphicSizeTwip( aGrf, 0 );
            if( pSwGrfNode->IsChgTwipSizeFromPixel() )
            {
                const MapMode aMapTwip( MAP_TWIP );
                aFrmFmtSz =
                    Application::GetDefaultDevice()->PixelToLogic(
                        aGrf.GetSizePixel(), aMapTwip );
            }
            else
            {
                aFrmFmtSz = aGrfSz;
            }

            if( bGraphicPieceArrived && GRAPHIC_DEFAULT != aGrf.GetType() &&
                ( !aOldSz.Width() || !aOldSz.Height() ) )
            {
                // If only a part arrives, but the size is not set
                // we need to go through bGraphicArrived down there.
                // Or else the graphic is painted at its definitive size
                bGraphicArrived = true;
                bGraphicPieceArrived = false;
            }

            pSwGrfNode->SetGraphic(aGrf, rGrfObj.GetLink());
            bUpdate = true;

            // In order for the Node to have the right transparency status
            // without having to access the graphic.
            // Or else we cause a SwapIn.
            if( bGraphicArrived )
            {
                // Always use the correct graphic size
                if( aGrfSz.Height() && aGrfSz.Width() &&
                    aOldSz.Height() && aOldSz.Width() &&
                    aGrfSz != aOldSz )
                {
                    pSwGrfNode->SetTwipSize( aGrfSz );
                    aOldSz = aGrfSz;
                }
            }
        }
        if ( bUpdate && !bGraphicArrived && !bGraphicPieceArrived )
            pSwGrfNode->SetTwipSize( Size(0,0) );
    }
    else if( pCntntNode->IsOLENode() )
        bUpdate = true;

    SwViewShell *pSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    SwEditShell* pESh = pDoc->GetEditShell();

    if ( bUpdate && bGraphicPieceArrived && !(bSwapIn || bDontNotify) )
    {
        // Send hint without Actions; triggers direct paint
        if ( (!pSh || !pSh->ActionPend()) && (!pESh || !pESh->ActionPend()) )
        {
            SwMsgPoolItem aMsgHint( RES_GRAPHIC_PIECE_ARRIVED );
            pCntntNode->ModifyNotification( &aMsgHint, &aMsgHint );
            bUpdate = false;
        }
    }

    static bool bInNotifyLinks = false;
    if( bUpdate && !bDontNotify && (!bSwapIn || bGraphicArrived) &&
        !bInNotifyLinks)
    {
        bool bLockView = false;
        if( pSh )
        {
            bLockView = pSh->IsViewLocked();
            pSh->LockView( true );
        }

        if( pESh )
            pESh->StartAllAction();
        else if( pSh )
            pSh->StartAction();

        SwMsgPoolItem aMsgHint( static_cast<sal_uInt16>(
            bGraphicArrived ? RES_GRAPHIC_ARRIVED : RES_UPDATE_ATTR ) );

        if ( bGraphicArrived )
        {
            // Notify all who are listening at the same link
            bInNotifyLinks = true;

            const ::sfx2::SvBaseLinks& rLnks = pDoc->getIDocumentLinksAdministration().GetLinkManager().GetLinks();
            for( sal_uInt16 n = rLnks.size(); n; )
            {
                ::sfx2::SvBaseLink* pLnk = &(*rLnks[ --n ]);
                if( pLnk && OBJECT_CLIENT_GRF == pLnk->GetObjType() &&
                    pLnk->ISA( SwBaseLink ) && pLnk->GetObj() == GetObj() )
                {
                    SwBaseLink* pBLink = static_cast<SwBaseLink*>(pLnk);
                    SwGrfNode* pGrfNd = static_cast<SwGrfNode*>(pBLink->pCntntNode);

                    if( pBLink != this &&
                        ( !bSwapIn ||
                            GRAPHIC_DEFAULT == pGrfNd->GetGrfObj().GetType()))
                    {
                        Size aPreArriveSize(pGrfNd->GetTwipSize());

                        pBLink->bIgnoreDataChanged = false;
                        pBLink->DataChanged( rMimeType, rValue );
                        pBLink->bIgnoreDataChanged = true;

                        pGrfNd->SetGraphicArrived( static_cast<SwGrfNode*>(pCntntNode)->
                                                    IsGraphicArrived() );

                        // Adjust the Fly's graphic
                        if (!::SetGrfFlySize(aGrfSz, aFrmFmtSz, pGrfNd, aPreArriveSize))
                            ::lcl_CallModify( *pGrfNd, aMsgHint );
                    }
                    else if (pBLink == this)
                    {
                        assert(pGrfNd == pSwGrfNode && "fdo#87083 needs a different fix");
                        if (!::SetGrfFlySize(aGrfSz, aFrmFmtSz, pGrfNd, aOldSz))
                        {
                            // Adjust the Fly's graphic
                            ::lcl_CallModify( *pGrfNd, aMsgHint );
                        }
                    }
                }
            }

            bInNotifyLinks = false;
        }
        else
        {
            pCntntNode->ModifyNotification( &aMsgHint, &aMsgHint );
        }

        if( pESh )
        {
            const bool bEndActionByVirDev = pESh->IsEndActionByVirDev();
            pESh->SetEndActionByVirDev( true );
            pESh->EndAllAction();
            pESh->SetEndActionByVirDev( bEndActionByVirDev );
        }
        else if( pSh )
            pSh->EndAction();

        if( pSh && !bLockView )
            pSh->LockView( false );
    }

    return SUCCESS;
}

static bool SetGrfFlySize( const Size& rGrfSz, const Size& rFrmSz, SwGrfNode* pGrfNd, const Size& rOrigGrfSize )
{
    bool bRet = false;
    SwViewShell *pSh = pGrfNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CurrShell *pCurr = 0;
    if ( pGrfNd->GetDoc()->GetEditShell() )
        pCurr = new CurrShell( pSh );

    Size aSz = rOrigGrfSize;
    if ( !(aSz.Width() && aSz.Height()) &&
            rGrfSz.Width() && rGrfSz.Height() )
    {
        SwFrmFmt* pFmt;
        if( pGrfNd->IsChgTwipSize() &&
            0 != (pFmt = pGrfNd->GetFlyFmt()) )
        {
            Size aCalcSz( aSz );
            if ( !aSz.Height() && aSz.Width() )
                // Calculate the right height
                aCalcSz.Height() = rFrmSz.Height() *
                        aSz.Width() / rFrmSz.Width();
            else if ( !aSz.Width() && aSz.Height() )
                // Calculate the right width
                aCalcSz.Width() = rFrmSz.Width() *
                        aSz.Height() / rFrmSz.Height();
            else
                // Take over height and width
                aCalcSz = rFrmSz;

            const SvxBoxItem     &rBox = pFmt->GetBox();
            aCalcSz.Width() += rBox.CalcLineSpace(BOX_LINE_LEFT) +
                               rBox.CalcLineSpace(BOX_LINE_RIGHT);
            aCalcSz.Height()+= rBox.CalcLineSpace(BOX_LINE_TOP) +
                               rBox.CalcLineSpace(BOX_LINE_BOTTOM);
            const SwFmtFrmSize& rOldAttr = pFmt->GetFrmSize();
            if( rOldAttr.GetSize() != aCalcSz )
            {
                SwFmtFrmSize aAttr( rOldAttr  );
                aAttr.SetSize( aCalcSz );
                pFmt->SetFmtAttr( aAttr );
                bRet = true;
            }

            if( !aSz.Width() )
            {
                // If the graphic is anchored in a table, we need to recalculate
                // the table rows
                const SwDoc *pDoc = pGrfNd->GetDoc();
                const SwPosition* pAPos = pFmt->GetAnchor().GetCntntAnchor();
                SwNode *pANd;
                SwTableNode *pTblNd;
                if( pAPos &&
                    0 != (pANd = & pAPos->nNode.GetNode()) &&
                    0 != (pTblNd = pANd->FindTableNode()) )
                {
                    const bool bLastGrf = !pTblNd->GetTable().DecGrfsThatResize();
                    SwHTMLTableLayout *pLayout =
                        pTblNd->GetTable().GetHTMLTableLayout();
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

    delete pCurr;

    return bRet;
}

bool SwBaseLink::SwapIn( bool bWaitForData, bool bNativFormat )
{
    bSwapIn = true;

    if( !GetObj() && ( bNativFormat || ( !IsSynchron() && bWaitForData ) ))
    {
        AddNextRef();
        _GetRealObject();
        ReleaseRef();
    }

#if OSL_DEBUG_LEVEL > 1
    {
        OUString sGrfNm;
        if(GetLinkManager())
            GetLinkManager()->GetDisplayNames( this, 0, &sGrfNm, 0, 0 );
        int x = 0;
        ++x;
    }
#endif

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
                bIgnoreDataChanged = false;
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

    bSwapIn = false;
    return bRes;
}

void SwBaseLink::Closed()
{
    if( pCntntNode && !pCntntNode->GetDoc()->IsInDtor() )
    {
        // Delete the connection
        if( pCntntNode->IsGrfNode() )
            static_cast<SwGrfNode*>(pCntntNode)->ReleaseLink();
    }
    SvBaseLink::Closed();
}

const SwNode* SwBaseLink::GetAnchor() const
{
    if (pCntntNode)
    {
        SwFrmFmt *const pFmt = pCntntNode->GetFlyFmt();
        if (pFmt)
        {
            const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
            SwPosition const*const pAPos = rAnchor.GetCntntAnchor();
            if (pAPos &&
                ((FLY_AS_CHAR == rAnchor.GetAnchorId()) ||
                 (FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
                 (FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
                 (FLY_AT_PARA == rAnchor.GetAnchorId())))
            {
                    return &pAPos->nNode.GetNode();
            }
            return 0;
        }
    }

    OSL_ENSURE( false, "GetAnchor is not shadowed" );
    return 0;
}

bool SwBaseLink::IsRecursion( const SwBaseLink* pChkLnk ) const
{
    tools::SvRef<SwServerObject> aRef( static_cast<SwServerObject*>(GetObj()) );
    if( aRef.Is() )
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
