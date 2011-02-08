/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_sw.hxx"

#include <hintids.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#include <osl/thread.hxx>
#include <salhelper/condition.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/objsh.hxx>
#include <editeng/boxitem.hxx>
#include <svx/svxids.hrc>       // fuer die EventIds
#include <sfx2/linkmgr.hxx>
#include <svtools/soerr.hxx>
#include <fmtfsize.hxx>
#include <fmtanchr.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
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

BOOL SetGrfFlySize( const Size& rGrfSz, const Size& rFrmSz, SwGrfNode* pGrfNd );

TYPEINIT1( SwBaseLink, ::sfx2::SvBaseLink );

SV_IMPL_REF( SwServerObject )

void lcl_CallModify( SwGrfNode& rGrfNd, SfxPoolItem& rItem )
{
    //call fist all not SwNoTxtFrames, then the SwNoTxtFrames.
    //              The reason is, that in the SwNoTxtFrames the Graphic
    //              after a Paint will be swapped out! So all other "behind"
    //              them havent't a loaded Graphic.
    rGrfNd.LockModify();

    SwClientIter aIter( rGrfNd );
    for( int n = 0; n < 2; ++n )
    {
        SwClient * pLast = aIter.GoStart();
        if( pLast )     // konnte zum Anfang gesprungen werden ??
        {
            do {
                if( (0 == n) ^ ( 0 != pLast->ISA( SwCntntFrm )) )
                    pLast->Modify( &rItem, &rItem );
            } while( 0 != ( pLast = aIter++ ));
        }
    }
    rGrfNd.UnlockModify();
}


void SwBaseLink::DataChanged( const String& rMimeType,
                            const uno::Any & rValue )
{
    if( !pCntntNode )
    {
        OSL_ENSURE(!this, "DataChanged ohne ContentNode" );
        return ;
    }

    SwDoc* pDoc = pCntntNode->GetDoc();
    if( pDoc->IsInDtor() || ChkNoDataFlag() || bIgnoreDataChanged )
    {
        bIgnoreDataChanged = FALSE;
        return ;
    }

    ULONG nFmt = SotExchange::GetFormatIdFromMimeType( rMimeType );

    if( pCntntNode->IsNoTxtNode() &&
        nFmt == sfx2::LinkManager::RegisterStatusInfoId() )
    {
        // nur eine Statusaenderung - Events bedienen ?
        ::rtl::OUString sState;
        if( rValue.hasValue() && ( rValue >>= sState ))
        {
            USHORT nEvent = 0;
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
        return;         // das wars!
    }

    BOOL bUpdate = FALSE;
    BOOL bGraphicArrived = FALSE;
    BOOL bGraphicPieceArrived = FALSE;
    BOOL bDontNotify = FALSE;
    Size aGrfSz, aFrmFmtSz;

    if( pCntntNode->IsGrfNode() )
    {
        GraphicObject& rGrfObj = ((SwGrfNode*)pCntntNode)->GetGrfObj();

        bDontNotify = ((SwGrfNode*)pCntntNode)->IsFrameInPaint();

        bGraphicArrived = GetObj()->IsDataComplete();
        bGraphicPieceArrived = GetObj()->IsPending();
        ((SwGrfNode*)pCntntNode)->SetGrafikArrived( bGraphicArrived );

        Graphic aGrf;
        if( sfx2::LinkManager::GetGraphicFromAny( rMimeType, rValue, aGrf ) &&
            ( GRAPHIC_DEFAULT != aGrf.GetType() ||
              GRAPHIC_DEFAULT != rGrfObj.GetType() ) )
        {
            aGrfSz = ::GetGraphicSizeTwip( aGrf, 0 );
            if( static_cast< const SwGrfNode * >( pCntntNode )->IsChgTwipSizeFromPixel() )
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
            Size aSz( ((SwGrfNode*)pCntntNode)->GetTwipSize() );

            if( bGraphicPieceArrived && GRAPHIC_DEFAULT != aGrf.GetType() &&
                ( !aSz.Width() || !aSz.Height() ) )
            {
                // wenn nur ein Teil ankommt, aber die Groesse nicht
                // gesetzt ist, dann muss "unten" der Teil von
                // bGraphicArrived durchlaufen werden!
                // (ansonten wird die Grafik in deft. Size gepaintet)
                bGraphicArrived = TRUE;
                bGraphicPieceArrived = FALSE;
            }

            rGrfObj.SetGraphic( aGrf, rGrfObj.GetLink() );
            bUpdate = TRUE;

            // Bug 33999: damit der Node den Transparent-Status
            //      richtig gesetzt hat, ohne auf die Grafik
            //      zugreifen zu muessen (sonst erfolgt ein SwapIn!).
            if( bGraphicArrived )
            {
                // immer mit der korrekten Grafik-Size arbeiten
                if( aGrfSz.Height() && aGrfSz.Width() &&
                    aSz.Height() && aSz.Width() &&
                    aGrfSz != aSz )
                    ((SwGrfNode*)pCntntNode)->SetTwipSize( aGrfSz );
            }
        }
        if ( bUpdate && !bGraphicArrived && !bGraphicPieceArrived )
            ((SwGrfNode*)pCntntNode)->SetTwipSize( Size(0,0) );
    }
    else if( pCntntNode->IsOLENode() )
        bUpdate = TRUE;

    ViewShell *pSh = 0;
    SwEditShell* pESh = pDoc->GetEditShell( &pSh );

    if ( bUpdate && bGraphicPieceArrived && !(bSwapIn || bDontNotify) )
    {
        //Hint ohne Actions verschicken, loest direktes Paint aus.
        if ( (!pSh || !pSh->ActionPend()) && (!pESh || !pESh->ActionPend()) )
        {
            SwMsgPoolItem aMsgHint( RES_GRAPHIC_PIECE_ARRIVED );
            pCntntNode->Modify( &aMsgHint, &aMsgHint );
            bUpdate = FALSE;
        }
    }

    static BOOL bInNotifyLinks = FALSE;
    if( bUpdate && !bDontNotify && (!bSwapIn || bGraphicArrived) &&
        !bInNotifyLinks)
    {
        BOOL bLockView = FALSE;
        if( pSh )
        {
            bLockView = pSh->IsViewLocked();
            pSh->LockView( TRUE );
        }

        if( pESh )
            pESh->StartAllAction();
        else if( pSh )
            pSh->StartAction();

        SwMsgPoolItem aMsgHint( static_cast<USHORT>(
            bGraphicArrived ? RES_GRAPHIC_ARRIVED : RES_UPDATE_ATTR ) );

        if ( bGraphicArrived )
        {
            //Alle benachrichtigen, die am gleichen Link horchen.
            bInNotifyLinks = TRUE;

            const ::sfx2::SvBaseLinks& rLnks = pDoc->GetLinkManager().GetLinks();
            for( USHORT n = rLnks.Count(); n; )
            {
                ::sfx2::SvBaseLink* pLnk = &(*rLnks[ --n ]);
                if( pLnk && OBJECT_CLIENT_GRF == pLnk->GetObjType() &&
                    pLnk->ISA( SwBaseLink ) && pLnk->GetObj() == GetObj() )
                {
                    SwBaseLink* pBLink = (SwBaseLink*)pLnk;
                    SwGrfNode* pGrfNd = (SwGrfNode*)pBLink->pCntntNode;

                    if( pBLink != this &&
                        ( !bSwapIn ||
                            GRAPHIC_DEFAULT == pGrfNd->GetGrfObj().GetType()))
                    {
                        pBLink->bIgnoreDataChanged = FALSE;
                        pBLink->DataChanged( rMimeType, rValue );
                        pBLink->bIgnoreDataChanged = TRUE;

                        pGrfNd->SetGrafikArrived( ((SwGrfNode*)pCntntNode)->
                                                    IsGrafikArrived() );

                        // Fly der Grafik anpassen !
                        if( !::SetGrfFlySize( aGrfSz, aFrmFmtSz, pGrfNd ) )
                            ::lcl_CallModify( *pGrfNd, aMsgHint );
                    }
                    else if( pBLink == this &&
                            !::SetGrfFlySize( aGrfSz, aFrmFmtSz, pGrfNd ) )
                        // Fly der Grafik anpassen !
                        ::lcl_CallModify( *pGrfNd, aMsgHint );
                }
            }

            bInNotifyLinks = FALSE;
        }
        else
        {
            pCntntNode->Modify( &aMsgHint, &aMsgHint );
        }


        if( pESh )
        {
            const BOOL bEndActionByVirDev = pESh->IsEndActionByVirDev();
            pESh->SetEndActionByVirDev( TRUE );
            pESh->EndAllAction();
            pESh->SetEndActionByVirDev( bEndActionByVirDev );
        }
        else if( pSh )
            pSh->EndAction();

        if( pSh && !bLockView )
            pSh->LockView( FALSE );
    }
}

BOOL SetGrfFlySize( const Size& rGrfSz, const Size& rFrmSz, SwGrfNode* pGrfNd )
{
    BOOL bRet = FALSE;
    ViewShell *pSh;
    CurrShell *pCurr = 0;
    if ( pGrfNd->GetDoc()->GetEditShell( &pSh ) )
        pCurr = new CurrShell( pSh );

    Size aSz = pGrfNd->GetTwipSize();
    if ( !(aSz.Width() && aSz.Height()) &&
            rGrfSz.Width() && rGrfSz.Height() )
    {
        SwFrmFmt* pFmt;
        if( pGrfNd->IsChgTwipSize() &&
            0 != (pFmt = pGrfNd->GetFlyFmt()) )
        {
            Size aCalcSz( aSz );
            if ( !aSz.Height() && aSz.Width() )
                //passende Hoehe ausrechnen.
                aCalcSz.Height() = rFrmSz.Height() *
                        aSz.Width() / rFrmSz.Width();
            else if ( !aSz.Width() && aSz.Height() )
                //passende Breite ausrechnen
                aCalcSz.Width() = rFrmSz.Width() *
                        aSz.Height() / rFrmSz.Height();
            else
                //Hoehe und Breite uebernehmen
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
                bRet = TRUE;
            }

            if( !aSz.Width() )
            {
                // Wenn die Grafik in einer Tabelle verankert ist, muess
                // die Tabellen-Spalten neu berechnet werden
                const SwDoc *pDoc = pGrfNd->GetDoc();
                const SwPosition* pAPos = pFmt->GetAnchor().GetCntntAnchor();
                SwNode *pANd;
                SwTableNode *pTblNd;
                if( pAPos &&
                    0 != (pANd = pDoc->GetNodes()[pAPos->nNode]) &&
                    0 != (pTblNd = pANd->FindTableNode()) )
                {
                    const BOOL bLastGrf = !pTblNd->GetTable().DecGrfsThatResize();
                    SwHTMLTableLayout *pLayout =
                        pTblNd->GetTable().GetHTMLTableLayout();
                    if( pLayout )
                    {
                        const USHORT nBrowseWidth =
                                    pLayout->GetBrowseWidthByTable( *pDoc );
                        if ( nBrowseWidth )
                        {
                            pLayout->Resize( nBrowseWidth, TRUE, TRUE,
                                             bLastGrf ? HTMLTABLE_RESIZE_NOW
                                                      : 500 );
                        }
                    }
                }
            }
        }

        // SetTwipSize skaliert ggf. eine ImageMap neu und
        // braucht dazu das Frame-Format
        pGrfNd->SetTwipSize( rGrfSz );
    }

    delete pCurr;

    return bRet;
}


BOOL SwBaseLink::SwapIn( BOOL bWaitForData, BOOL bNativFormat )
{
    bSwapIn = TRUE;

    BOOL bRes;

    if( !GetObj() && ( bNativFormat || ( !IsSynchron() && bWaitForData ) ))
    {
        AddNextRef();
        _GetRealObject();
        ReleaseRef();
    }

#if OSL_DEBUG_LEVEL > 1
    {
        String sGrfNm;
        if(GetLinkManager())
            GetLinkManager()->GetDisplayNames( this, 0, &sGrfNm, 0, 0 );
        int x = 0;
        ++x;
    }
#endif

    if( GetObj() )
    {
        String aMimeType( SotExchange::GetFormatMimeType( GetContentType() ));

        uno::Any aValue;
        GetObj()->GetData( aValue, aMimeType, !IsSynchron() && bWaitForData );

        if( bWaitForData && !GetObj() )
        {
            OSL_ENSURE( !this, "das SvxFileObject wurde in einem GetData geloescht!" );
            bRes = FALSE;
        }
        else if( 0 != ( bRes = aValue.hasValue() ) )
        {
            //JP 14.04.99: Bug 64820 - das Flag muss beim SwapIn natuerlich
            //              zurueckgesetzt werden. Die Daten sollen ja neu
            //              uebernommen werden
            bIgnoreDataChanged = FALSE;
            DataChanged( aMimeType, aValue );
        }
    }
    else if( !IsSynchron() && bWaitForData )
    {
        SetSynchron( TRUE );
        bRes = Update();
        SetSynchron( FALSE );
    }
    else
        bRes = Update();

    bSwapIn = FALSE;

    return bRes;
}

void SwBaseLink::Closed()
{
    if( pCntntNode && !pCntntNode->GetDoc()->IsInDtor() )
    {
        // wir heben die Verbindung auf
        if( pCntntNode->IsGrfNode() )
            ((SwGrfNode*)pCntntNode)->ReleaseLink();
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

    OSL_ENSURE( !this, "GetAnchor nicht ueberlagert" );
    return 0;
}

BOOL SwBaseLink::IsRecursion( const SwBaseLink* pChkLnk ) const
{
    SwServerObjectRef aRef( (SwServerObject*)GetObj() );
    if( aRef.Is() )
    {
        // es ist ein ServerObject, also frage nach allen darin
        // befindlichen Links, ob wir darin enthalten sind. Ansonsten
        // handelt es sich um eine Rekursion.
        return aRef->IsLinkInServer( pChkLnk );
    }
    return FALSE;
}

BOOL SwBaseLink::IsInRange( ULONG, ULONG, xub_StrLen, xub_StrLen ) const
{
    // Grafik oder OLE-Links nicht,
    // Felder oder Sections haben eigene Ableitung!
    return FALSE;
}

SwBaseLink::~SwBaseLink()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
