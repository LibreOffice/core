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
#include "pam.hxx"
#include "swtable.hxx"
#include "frame.hxx"
#include "pagefrm.hxx"
#include "flyfrm.hxx"
#include "viewsh.hxx"
#include "doc.hxx"
#include "viewimp.hxx"
#include "dflyobj.hxx"
#include "frmtool.hxx"
#include "dcontact.hxx"
#include <editeng/brkitem.hxx>
#include <editeng/keepitem.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <fmtpdsc.hxx>
#include <editeng/ulspitem.hxx>
#include <tgrditem.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <editeng/pgrditem.hxx>
#include <paratr.hxx>

#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "tabfrm.hxx"
#include "pagedesc.hxx"
#include "layact.hxx"
#include "fmtornt.hxx"
#include "flyfrms.hxx"
#include "sectfrm.hxx"
#include "section.hxx"
#include "dbg_lay.hxx"
#include "lineinfo.hxx"
// OD 2004-03-02 #106629#
#include <fmtclbl.hxx>
// --> OD 2004-06-23 #i28701#
#include <sortedobjs.hxx>
#include <layouter.hxx>
// <--
// --> OD 2004-10-15 #i26945#
#include <fmtfollowtextflow.hxx>
// <--

sal_Bool SwFlowFrm::bMoveBwdJump = sal_False;


/*************************************************************************
|*
|*  SwFlowFrm::SwFlowFrm()
|*
|*************************************************************************/


SwFlowFrm::SwFlowFrm( SwFrm &rFrm ) :
    rThis( rFrm ),
    pFollow( 0 )
{
    bLockJoin = bIsFollow = bCntntLock = bOwnFtnNum =
        bFtnLock = bFlyLock = sal_False;
}


/*************************************************************************
|*
|*  SwFlowFrm::IsFollowLocked()
|*     return sal_True if any follow has the JoinLocked flag
|*
|*************************************************************************/

sal_Bool SwFlowFrm::HasLockedFollow() const
{
    const SwFlowFrm* pFrm = GetFollow();
    while( pFrm )
    {
        if( pFrm->IsJoinLocked() )
            return sal_True;
        pFrm = pFrm->GetFollow();
    }
    return sal_False;
}

/*************************************************************************
|*
|*  SwFlowFrm::IsKeepFwdMoveAllowed()
|*
|*************************************************************************/


sal_Bool SwFlowFrm::IsKeepFwdMoveAllowed()
{
    //Wenn der Vorgaenger das KeepAttribut traegt und auch dessen
    //Vorgaenger usw. bis zum ersten der Kette und fuer diesen das
    //IsFwdMoveAllowed ein sal_False liefert, so ist das Moven eben nicht erlaubt.
    SwFrm *pFrm = &rThis;
    if ( !pFrm->IsInFtn() )
        do
        {   if ( pFrm->GetAttrSet()->GetKeep().GetValue() )
                pFrm = pFrm->GetIndPrev();
            else
                return sal_True;
        } while ( pFrm );

                  //Siehe IsFwdMoveAllowed()
    sal_Bool bRet = sal_False;
    if ( pFrm && pFrm->GetIndPrev() )
        bRet = sal_True;
    return bRet;
}

/*************************************************************************
|*
|*    SwFlowFrm::CheckKeep()
|*
|*************************************************************************/


void SwFlowFrm::CheckKeep()
{
    //Den 'letzten' Vorgaenger mit KeepAttribut anstossen, denn
    //die ganze Truppe koennte zuruckrutschen.
    SwFrm *pPre = rThis.GetIndPrev();
    if( pPre->IsSctFrm() )
    {
        SwFrm *pLast = ((SwSectionFrm*)pPre)->FindLastCntnt();
        if( pLast && pLast->FindSctFrm() == pPre )
            pPre = pLast;
        else
            return;
    }
    SwFrm* pTmp;
    sal_Bool bKeep;
    while ( sal_True == (bKeep = pPre->GetAttrSet()->GetKeep().GetValue()) &&
            0 != ( pTmp = pPre->GetIndPrev() ) )
    {
        if( pTmp->IsSctFrm() )
        {
            SwFrm *pLast = ((SwSectionFrm*)pTmp)->FindLastCntnt();
            if( pLast && pLast->FindSctFrm() == pTmp )
                pTmp = pLast;
            else
                break;
        }
        pPre = pTmp;
    }
    if ( bKeep )
        pPre->InvalidatePos();
}

/*************************************************************************
|*
|*  SwFlowFrm::IsKeep()
|*
|*************************************************************************/

sal_Bool SwFlowFrm::IsKeep( const SwAttrSet& rAttrs, bool bCheckIfLastRowShouldKeep ) const
{
    // 1. The keep attribute is ignored inside footnotes
    // 2. For compatibility reasons, the keep attribute is
    //    ignored for frames inside table cells
    // 3. If bBreakCheck is set to true, this function only checks
    //    if there are any break after attributes set at rAttrs
    //    or break before attributes set for the next content (or next table)
    sal_Bool bKeep = bCheckIfLastRowShouldKeep ||
                 (  !rThis.IsInFtn() &&
                    ( !rThis.IsInTab() || rThis.IsTabFrm() ) &&
                    rAttrs.GetKeep().GetValue() );

    OSL_ENSURE( !bCheckIfLastRowShouldKeep || rThis.IsTabFrm(),
            "IsKeep with bCheckIfLastRowShouldKeep should only be used for tabfrms" );

    // Ignore keep attribute if there are break situations:
    if ( bKeep )
    {
        switch ( rAttrs.GetBreak().GetBreak() )
        {
            case SVX_BREAK_COLUMN_AFTER:
            case SVX_BREAK_COLUMN_BOTH:
            case SVX_BREAK_PAGE_AFTER:
            case SVX_BREAK_PAGE_BOTH:
            {
                bKeep = sal_False;
            }
            default: break;
        }
        if ( bKeep )
        {
            SwFrm *pNxt;
            if( 0 != (pNxt = rThis.FindNextCnt()) &&
                (!pFollow || pNxt != pFollow->GetFrm()))
            {
                // --> FME 2006-05-15 #135914#
                // The last row of a table only keeps with the next content
                // it they are in the same section:
                if ( bCheckIfLastRowShouldKeep )
                {
                    const SwSection* pThisSection = 0;
                    const SwSection* pNextSection = 0;
                    const SwSectionFrm* pThisSectionFrm = rThis.FindSctFrm();
                    const SwSectionFrm* pNextSectionFrm = pNxt->FindSctFrm();

                    if ( pThisSectionFrm )
                        pThisSection = pThisSectionFrm->GetSection();

                    if ( pNextSectionFrm )
                        pNextSection = pNextSectionFrm->GetSection();

                    if ( pThisSection != pNextSection )
                        bKeep = sal_False;
                }
                // <--

                if ( bKeep )
                {
                    const SwAttrSet* pSet = NULL;

                    if ( pNxt->IsInTab() )
                    {
                        SwTabFrm* pTab = pNxt->FindTabFrm();
                        if ( ! rThis.IsInTab() || rThis.FindTabFrm() != pTab )
                            pSet = &pTab->GetFmt()->GetAttrSet();
                    }

                    if ( ! pSet )
                        pSet = pNxt->GetAttrSet();

                    OSL_ENSURE( pSet, "No AttrSet to check keep attribute" );

                    if ( pSet->GetPageDesc().GetPageDesc() )
                        bKeep = sal_False;
                    else switch ( pSet->GetBreak().GetBreak() )
                    {
                        case SVX_BREAK_COLUMN_BEFORE:
                        case SVX_BREAK_COLUMN_BOTH:
                        case SVX_BREAK_PAGE_BEFORE:
                        case SVX_BREAK_PAGE_BOTH:
                            bKeep = sal_False;
                        default: break;
                    }
                }
            }
        }
    }
    return bKeep;
}

/*************************************************************************
|*
|*  SwFlowFrm::BwdMoveNecessary()
|*
|*************************************************************************/


sal_uInt8 SwFlowFrm::BwdMoveNecessary( const SwPageFrm *pPage, const SwRect &rRect )
{
    // Der return-Wert entscheidet mit,
    // ob auf Zurueckgeflossen werden muss, (3)
    // ob das gute alte WouldFit gerufen werden kann (0, 1)
    // oder ob ein Umhaengen und eine Probeformatierung sinnvoll ist (2)
    // dabei bedeutet Bit 1, dass Objekte an mir selbst verankert sind
    // und Bit 2, dass ich anderen Objekten ausweichen muss.

    //Wenn ein SurroundObj, dass einen Umfluss wuenscht mit dem Rect ueberlappt
    //ist der Fluss notwendig (weil die Verhaeltnisse nicht geschaetzt werden
    //koennen), es kann allerdings ggf. eine TestFormatierung stattfinden.
    //Wenn das SurroundObj ein Fly ist und ich selbst ein Lower bin oder der Fly
    //Lower von mir ist, so spielt er keine Rolle.
    //Wenn das SurroundObj in einem zeichengebunden Fly verankert ist, und ich
    //selbst nicht Lower dieses Zeichengebundenen Flys bin, so spielt der Fly
    //keine Rolle.
    //#32639# Wenn das Objekt bei mir verankert ist kann ich es
    //vernachlaessigen, weil es hoechstwahrscheinlich (!?) mitfliesst,
    //eine TestFormatierung ist dann allerdings nicht erlaubt!
    sal_uInt8 nRet = 0;
    SwFlowFrm *pTmp = this;
    do
    {   // Wenn an uns oder einem Follow Objekte haengen, so
        // kann keine ProbeFormatierung stattfinden, da absatzgebundene
        // nicht richtig beruecksichtigt wuerden und zeichengebundene sollten
        // gar nicht zur Probe formatiert werden.
        if( pTmp->GetFrm()->GetDrawObjs() )
            nRet = 1;
        pTmp = pTmp->GetFollow();
    } while ( !nRet && pTmp );
    if ( pPage->GetSortedObjs() )
    {
        // --> OD 2004-07-01 #i28701# - new type <SwSortedObjs>
        const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
        sal_uLong nIndex = ULONG_MAX;
        for ( sal_uInt16 i = 0; nRet < 3 && i < rObjs.Count(); ++i )
        {
            // --> OD 2004-07-01 #i28701# - consider changed type of
            // <SwSortedObjs> entries.
            SwAnchoredObject* pObj = rObjs[i];
            const SwFrmFmt& rFmt = pObj->GetFrmFmt();
            const SwRect aRect( pObj->GetObjRect() );
            if ( aRect.IsOver( rRect ) &&
                 rFmt.GetSurround().GetSurround() != SURROUND_THROUGHT )
            {
                if( rThis.IsLayoutFrm() && //Fly Lower von This?
                    Is_Lower_Of( &rThis, pObj->GetDrawObj() ) )
                    continue;
                if( pObj->ISA(SwFlyFrm) )
                {
                    const SwFlyFrm *pFly = static_cast<const SwFlyFrm*>(pObj);
                    if ( pFly->IsAnLower( &rThis ) )//This Lower vom Fly?
                        continue;
                }

                const SwFrm* pAnchor = pObj->GetAnchorFrm();
                if ( pAnchor == &rThis )
                {
                    nRet |= 1;
                    continue;
                }

                //Nicht wenn das Objekt im Textfluss hinter mir verankert ist,
                //denn dann weiche ich ihm nicht aus.
                if ( ::IsFrmInSameKontext( pAnchor, &rThis ) )
                {
                    if ( rFmt.GetAnchor().GetAnchorId() == FLY_AT_PARA )
                    {
                        // Den Index des anderen erhalten wir immer ueber das Ankerattr.
                        sal_uLong nTmpIndex = rFmt.GetAnchor().GetCntntAnchor()->nNode.GetIndex();
                        // Jetzt wird noch ueberprueft, ob der aktuelle Absatz vor dem
                        // Anker des verdraengenden Objekts im Text steht, dann wird
                        // nicht ausgewichen.
                        // Der Index wird moeglichst ueber einen SwFmtAnchor ermittelt,
                        // da sonst recht teuer.
                        if( ULONG_MAX == nIndex )
                        {
                            const SwNode *pNode;
                            if ( rThis.IsCntntFrm() )
                                pNode = ((SwCntntFrm&)rThis).GetNode();
                            else if( rThis.IsSctFrm() )
                                pNode = ((SwSectionFmt*)((SwSectionFrm&)rThis).
                                        GetFmt())->GetSectionNode();
                            else
                            {
                                OSL_ENSURE( rThis.IsTabFrm(), "new FowFrm?" );
                                pNode = ((SwTabFrm&)rThis).GetTable()->
                                    GetTabSortBoxes()[0]->GetSttNd()->FindTableNode();
                            }
                            nIndex = pNode->GetIndex();
                        }
                        if( nIndex < nTmpIndex )
                            continue;
                    }
                }
                else
                    continue;

                nRet |= 2;
            }
        }
    }
    return nRet;
}

/*************************************************************************
|*
|*  SwFlowFrm::CutTree(), PasteTree(), MoveSubTree()
|*
|*  Beschreibung        Eine Spezialisierte Form des Cut() und Paste(), die
|*      eine ganze Kette umhaengt (naehmlich this und folgende). Dabei werden
|*      nur minimale Operationen und Benachrichtigungen ausgefuehrt.
|*
|*************************************************************************/


SwLayoutFrm *SwFlowFrm::CutTree( SwFrm *pStart )
{
    //Der Start und alle Nachbarn werden ausgeschnitten, sie werden aneinander-
    //gereiht und ein Henkel auf den ersten wird zurueckgeliefert.
    //Zurueckbleibende werden geeignet invalidiert.

    SwLayoutFrm *pLay = pStart->GetUpper();
    if ( pLay->IsInFtn() )
        pLay = pLay->FindFtnFrm();

    // --> OD 2006-05-08 #i58846#
    // <pPrepare( PREP_QUOVADIS )> only for frames in footnotes
    if( pStart->IsInFtn() )
    {
        SwFrm* pTmp = pStart->GetIndPrev();
        if( pTmp )
            pTmp->Prepare( PREP_QUOVADIS );
    }
    // <--

    //Nur fix auschneiden und zwar so, dass klare Verhaeltnisse bei den
    //Verlassenen herrschen. Die Pointer der ausgeschnittenen Kette zeigen
    //noch wer weiss wo hin.
    if ( pStart == pStart->GetUpper()->Lower() )
        pStart->GetUpper()->pLower = 0;
    if ( pStart->GetPrev() )
    {
        pStart->GetPrev()->pNext = 0;
        pStart->pPrev = 0;
    }

    if ( pLay->IsFtnFrm() )
    {
        if ( !pLay->Lower() && !pLay->IsColLocked() &&
             !((SwFtnFrm*)pLay)->IsBackMoveLocked() )
        {
            pLay->Cut();
            delete pLay;
        }
        else
        {
            sal_Bool bUnlock = !((SwFtnFrm*)pLay)->IsBackMoveLocked();
            ((SwFtnFrm*)pLay)->LockBackMove();
            pLay->InvalidateSize();
            pLay->Calc();
            SwCntntFrm *pCnt = pLay->ContainsCntnt();
            while ( pCnt && pLay->IsAnLower( pCnt ) )
            {
                //Kann sein, dass der CntFrm gelockt ist, wir wollen hier nicht
                //in eine endlose Seitenwanderung hineinlaufen und rufen das
                //Calc garnicht erst!
                OSL_ENSURE( pCnt->IsTxtFrm(), "Die Graphic ist gelandet." );
                if ( ((SwTxtFrm*)pCnt)->IsLocked() ||
                     ((SwTxtFrm*)pCnt)->GetFollow() == pStart )
                    break;
                pCnt->Calc();
                pCnt = pCnt->GetNextCntntFrm();
            }
            if( bUnlock )
                ((SwFtnFrm*)pLay)->UnlockBackMove();
        }
        pLay = 0;
    }
    return pLay;
}



sal_Bool SwFlowFrm::PasteTree( SwFrm *pStart, SwLayoutFrm *pParent, SwFrm *pSibling,
                           SwFrm *pOldParent )
{
    //returnt sal_True wenn in der Kette ein LayoutFrm steht.
    sal_Bool bRet = sal_False;

    //Die mit pStart beginnende Kette wird vor den Sibling unter den Parent
    //gehaengt. Fuer geeignete Invalidierung wird ebenfalls gesorgt.

    //Ich bekomme eine fertige Kette. Der Anfang der Kette muss verpointert
    //werden, dann alle Upper fuer die Kette und schliesslich dass Ende.
    //Unterwegs werden alle geeignet invalidiert.
    if ( pSibling )
    {
        if ( 0 != (pStart->pPrev = pSibling->GetPrev()) )
            pStart->GetPrev()->pNext = pStart;
        else
            pParent->pLower = pStart;
        pSibling->_InvalidatePos();
        pSibling->_InvalidatePrt();
    }
    else
    {
        if ( 0 == (pStart->pPrev = pParent->Lower()) )
            pParent->pLower = pStart;
        else
        //Modified for #i100782#,04/03/2009
        //If the pParent has more than 1 child nodes, former design will
        //ignore them directly without any collection work. It will make some
        //dangling pointers. This lead the crash...
        //The new design will find the last child of pParent in loop way, and
        //add the pStart after the last child.
        //  pParent->Lower()->pNext = pStart;
        {
            SwFrm* pTemp = pParent->pLower;
            while (pTemp)
            {
                if (pTemp->pNext)
                    pTemp = pTemp->pNext;
                else
                {
                    pStart->pPrev = pTemp;
                    pTemp->pNext = pStart;
                    break;
                }
            }
        }
        //End modification for #i100782#,04/03/2009

        // #i27145#
        if ( pParent->IsSctFrm() )
        {
            // We have no sibling because pParent is a section frame and
            // has just been created to contain some content. The printing
            // area of the frame behind pParent has to be invalidated, so
            // that the correct distance between pParent and the next frame
            // can be calculated.
            pParent->InvalidateNextPrtArea();
        }
    }
    SwFrm *pFloat = pStart;
    SwFrm *pLst = 0;
    SWRECTFN( pParent )
    SwTwips nGrowVal = 0;
    do
    {   pFloat->pUpper = pParent;
        pFloat->_InvalidateAll();
        pFloat->CheckDirChange();

        //Ich bin Freund des TxtFrm und darf deshalb so einiges. Das mit
        //dem CacheIdx scheint etwas riskant!
        if ( pFloat->IsTxtFrm() )
        {
            if ( ((SwTxtFrm*)pFloat)->GetCacheIdx() != USHRT_MAX )
                ((SwTxtFrm*)pFloat)->Init();    //Ich bin sein Freund.
        }
        else
            bRet = sal_True;

        nGrowVal += (pFloat->Frm().*fnRect->fnGetHeight)();
        if ( pFloat->GetNext() )
            pFloat = pFloat->GetNext();
        else
        {
            pLst = pFloat;
            pFloat = 0;
        }
    } while ( pFloat );

    if ( pSibling )
    {
        pLst->pNext = pSibling;
        pSibling->pPrev = pLst;
        if( pSibling->IsInFtn() )
        {
            if( pSibling->IsSctFrm() )
                pSibling = ((SwSectionFrm*)pSibling)->ContainsAny();
            if( pSibling )
                pSibling->Prepare( PREP_ERGOSUM );
        }
    }
    if ( nGrowVal )
    {
        if ( pOldParent && pOldParent->IsBodyFrm() ) //Fuer variable Seitenhoehe beim Browsen
            pOldParent->Shrink( nGrowVal );
        pParent->Grow( nGrowVal );
    }

    if ( pParent->IsFtnFrm() )
        ((SwFtnFrm*)pParent)->InvalidateNxtFtnCnts( pParent->FindPageFrm() );
    return bRet;
}



void SwFlowFrm::MoveSubTree( SwLayoutFrm* pParent, SwFrm* pSibling )
{
    OSL_ENSURE( pParent, "Kein Parent uebergeben." );
    OSL_ENSURE( rThis.GetUpper(), "Wo kommen wir denn her?" );

    //Sparsamer benachrichtigen wenn eine Action laeuft.
    ViewShell *pSh = rThis.GetShell();
    const SwViewImp *pImp = pSh ? pSh->Imp() : 0;
    const sal_Bool bComplete = pImp && pImp->IsAction() && pImp->GetLayAction().IsComplete();

    if ( !bComplete )
    {
        SwFrm *pPre = rThis.GetIndPrev();
        if ( pPre )
        {
            pPre->SetRetouche();
            // --> OD 2004-11-23 #115759# - follow-up of #i26250#
            // invalidate printing area of previous frame, if it's in a table
            if ( pPre->GetUpper()->IsInTab() )
            {
                pPre->_InvalidatePrt();
            }
            // <--
            pPre->InvalidatePage();
        }
        else
        {   rThis.GetUpper()->SetCompletePaint();
            rThis.GetUpper()->InvalidatePage();
        }
    }

    SwPageFrm *pOldPage = rThis.FindPageFrm();

    SwLayoutFrm *pOldParent = CutTree( &rThis );
    const sal_Bool bInvaLay = PasteTree( &rThis, pParent, pSibling, pOldParent );

    // Wenn durch das Cut&Paste ein leerer SectionFrm entstanden ist, sollte
    // dieser automatisch verschwinden.
    SwSectionFrm *pSct;
    // --> OD 2006-01-04 #126020# - adjust check for empty section
    // --> OD 2006-02-01 #130797# - correct fix #126020#
    if ( pOldParent && !pOldParent->Lower() &&
         ( pOldParent->IsInSct() &&
           !(pSct = pOldParent->FindSctFrm())->ContainsCntnt() &&
           !pSct->ContainsAny( true ) ) )
    // <--
    {
            pSct->DelEmpty( sal_False );
    }

    // In einem spaltigen Bereich rufen wir lieber kein Calc "von unten"
    if( !rThis.IsInSct() &&
        ( !rThis.IsInTab() || ( rThis.IsTabFrm() && !rThis.GetUpper()->IsInTab() ) ) )
        rThis.GetUpper()->Calc();
    else if( rThis.GetUpper()->IsSctFrm() )
    {
        SwSectionFrm* pTmpSct = (SwSectionFrm*)rThis.GetUpper();
        sal_Bool bOld = pTmpSct->IsCntntLocked();
        pTmpSct->SetCntntLock( sal_True );
        pTmpSct->Calc();
        if( !bOld )
            pTmpSct->SetCntntLock( sal_False );
    }
    SwPageFrm *pPage = rThis.FindPageFrm();

    if ( pOldPage != pPage )
    {
        rThis.InvalidatePage( pPage );
        if ( rThis.IsLayoutFrm() )
        {
            SwCntntFrm *pCnt = ((SwLayoutFrm*)&rThis)->ContainsCntnt();
            if ( pCnt )
                pCnt->InvalidatePage( pPage );
        }
        else if ( pSh && pSh->GetDoc()->GetLineNumberInfo().IsRestartEachPage()
                  && pPage->FindFirstBodyCntnt() == &rThis )
        {
            rThis._InvalidateLineNum();
        }
    }
    if ( bInvaLay || (pSibling && pSibling->IsLayoutFrm()) )
        rThis.GetUpper()->InvalidatePage( pPage );
}

/*************************************************************************
|*
|*  SwFlowFrm::IsAnFollow()
|*
|*************************************************************************/


sal_Bool SwFlowFrm::IsAnFollow( const SwFlowFrm *pAssumed ) const
{
    const SwFlowFrm *pFoll = this;
    do
    {   if ( pAssumed == pFoll )
            return sal_True;
        pFoll = pFoll->GetFollow();
    } while ( pFoll );
    return sal_False;
}


/*************************************************************************
|*
|*  SwFlowFrm::FindMaster()
|*
|*************************************************************************/

SwTxtFrm* SwCntntFrm::FindMaster() const
{
    OSL_ENSURE( IsFollow(), "SwCntntFrm::FindMaster(): !IsFollow" );

    const SwCntntFrm* pCnt = GetPrevCntntFrm();

    while ( pCnt )
    {
        if ( pCnt->HasFollow() && pCnt->GetFollow() == this )
        {
            OSL_ENSURE( pCnt->IsTxtFrm(), "NoTxtFrm with follow found" );
            return (SwTxtFrm*)pCnt;
        }
        pCnt = pCnt->GetPrevCntntFrm();
    }

    OSL_ENSURE( sal_False, "Follow ist lost in Space." );
    return 0;
}

SwSectionFrm* SwSectionFrm::FindMaster() const
{
    OSL_ENSURE( IsFollow(), "SwSectionFrm::FindMaster(): !IsFollow" );

    SwClientIter aIter( *pSection->GetFmt() );
    SwClient *pLast = aIter.GoStart();

    while ( pLast )
    {
        if ( pLast->ISA( SwFrm ) )
        {
            OSL_ENSURE( ((SwFrm*)pLast)->IsSctFrm(),
                    "Non-section frame registered in section format" );
            SwSectionFrm* pSect = (SwSectionFrm*)pLast;
            if( pSect->GetFollow() == this )
                return pSect;
        }
        pLast = aIter++;
    }

    OSL_ENSURE( sal_False, "Follow ist lost in Space." );
    return 0;
}

SwTabFrm* SwTabFrm::FindMaster( bool bFirstMaster ) const
{
    OSL_ENSURE( IsFollow(), "SwTabFrm::FindMaster(): !IsFollow" );

    SwClientIter aIter( *GetTable()->GetFrmFmt() );
    SwClient* pLast = aIter.GoStart();

    while ( pLast )
    {
        if ( pLast->ISA( SwFrm ) )
        {
            OSL_ENSURE( ((SwFrm*)pLast)->IsTabFrm(),
                    "Non-table frame registered in table format" );
            SwTabFrm* pTab = (SwTabFrm*)pLast;

            if ( bFirstMaster )
            {
                //
                // Optimization. This makes code like this obsolete:
                // while ( pTab->IsFollow() )
                //     pTab = pTab->FindMaster();
                //
                if ( !pTab->IsFollow() )
                {
                    SwTabFrm* pNxt = pTab;
                    while ( pNxt )
                    {
                        if ( pNxt->GetFollow() == this )
                            return pTab;
                        pNxt = pNxt->GetFollow();
                    }
                }
            }
            else
            {
                if ( pTab->GetFollow() == this )
                    return pTab;
            }
        }
        pLast = aIter++;
    }

    OSL_ENSURE( sal_False, "Follow ist lost in Space." );
    return 0;
}

/*************************************************************************
|*
|*  SwFrm::GetLeaf()
|*
|*  Beschreibung        Liefert das naechste/vorhergehende LayoutBlatt,
|*      das _nicht_ unterhalb von this liegt (oder gar this selbst ist).
|*      Ausserdem muss dieses LayoutBlatt im gleichen Textfluss wie
|*      pAnch Ausgangsfrm liegen (Body, Ftn)
|*
|*************************************************************************/


const SwLayoutFrm *SwFrm::GetLeaf( MakePageType eMakePage, sal_Bool bFwd,
                                   const SwFrm *pAnch ) const
{
    //Ohne Fluss kein genuss...
    if ( !(IsInDocBody() || IsInFtn() || IsInFly()) )
        return 0;

    const SwFrm *pLeaf = this;
    sal_Bool bFound = sal_False;

    do
    {   pLeaf = ((SwFrm*)pLeaf)->GetLeaf( eMakePage, bFwd );

        if ( pLeaf &&
            (!IsLayoutFrm() || !((SwLayoutFrm*)this)->IsAnLower( pLeaf )))
        {
            if ( pAnch->IsInDocBody() == pLeaf->IsInDocBody() &&
                 pAnch->IsInFtn()     == pLeaf->IsInFtn() )
            {
                bFound = sal_True;
            }
        }
    } while ( !bFound && pLeaf );

    return (const SwLayoutFrm*)pLeaf;
}

/*************************************************************************
|*
|*  SwFrm::GetLeaf()
|*
|*  Beschreibung        Ruft Get[Next|Prev]Leaf
|*
|*************************************************************************/


SwLayoutFrm *SwFrm::GetLeaf( MakePageType eMakePage, sal_Bool bFwd )
{
    if ( IsInFtn() )
        return bFwd ? GetNextFtnLeaf( eMakePage ) : GetPrevFtnLeaf( eMakePage );

    // --> OD 2005-08-16 #i53323#
    // A frame could be inside a table AND inside a section.
    // Thus, it has to be determined, which is the first parent.
    bool bInTab( IsInTab() );
    bool bInSct( IsInSct() );
    if ( bInTab && bInSct )
    {
        const SwFrm* pUpperFrm( GetUpper() );
        while ( pUpperFrm )
        {
            if ( pUpperFrm->IsTabFrm() )
            {
                // the table is the first.
                bInSct = false;
                break;
            }
            else if ( pUpperFrm->IsSctFrm() )
            {
                // the section is the first.
                bInTab = false;
                break;
            }

            pUpperFrm = pUpperFrm->GetUpper();
        }
    }

    if ( bInTab && ( !IsTabFrm() || GetUpper()->IsCellFrm() ) ) // TABLE IN TABLE
        return bFwd ? GetNextCellLeaf( eMakePage ) : GetPrevCellLeaf( eMakePage );

    if ( bInSct )
        return bFwd ? GetNextSctLeaf( eMakePage ) : GetPrevSctLeaf( eMakePage );
    // <--

    return bFwd ? GetNextLeaf( eMakePage ) : GetPrevLeaf( eMakePage );
}



sal_Bool SwFrm::WrongPageDesc( SwPageFrm* pNew )
{
    //Jetzt wirds leider etwas kompliziert:
    //Ich bringe ich evtl. selbst
    //einen Pagedesc mit; der der Folgeseite muss dann damit
    //uebereinstimmen.
    //Anderfalls muss ich mir etwas genauer ansehen wo der
    //Folgepagedesc herkam.
    //Wenn die Folgeseite selbst schon sagt, dass ihr
    //Pagedesc nicht stimmt so kann ich das Teil bedenkenlos
    //auswechseln.
    //Wenn die Seite meint, dass ihr Pagedesc stimmt, so heisst
    //das leider noch immer nicht, dass ich damit etwas anfangen
    //kann: Wenn der erste BodyCntnt einen PageDesc oder einen
    //PageBreak wuenscht, so muss ich ebenfalls eine neue
    //Seite einfuegen; es sein denn die gewuenschte Seite ist
    //die richtige.
    //Wenn ich einen neue Seite eingefuegt habe, so fangen die
    //Probleme leider erst an, denn wahrscheinlich wird die dann
    //folgende Seite verkehrt gewesen und ausgewechselt worden
    //sein. Das hat zur Folge, dass ich zwar eine neue (und
    //jetzt richtige) Seite habe, die Bedingungen zum auswechseln
    //aber leider noch immer stimmen.
    //Ausweg: Vorlaeufiger Versuch, nur einmal eine neue Seite
    //einsetzen (Leerseiten werden noetigenfalls bereits von
    //InsertPage() eingefuegt.
    const SwFmtPageDesc &rFmtDesc = GetAttrSet()->GetPageDesc();

    //Mein Pagedesc zaehlt nicht, wenn ich ein Follow bin!
    SwPageDesc *pDesc = 0;
    sal_uInt16 nTmp = 0;
    SwFlowFrm *pFlow = SwFlowFrm::CastFlowFrm( this );
    if ( !pFlow || !pFlow->IsFollow() )
    {
        pDesc = (SwPageDesc*)rFmtDesc.GetPageDesc();
        if( pDesc )
        {
            if( !pDesc->GetRightFmt() )
                nTmp = 2;
            else if( !pDesc->GetLeftFmt() )
                nTmp = 1;
            else if( rFmtDesc.GetNumOffset() )
                nTmp = rFmtDesc.GetNumOffset();
        }
    }

    //Bringt der Cntnt einen Pagedesc mit oder muss zaehlt die
    //virtuelle Seitennummer des neuen Layoutleafs?
    // Bei Follows zaehlt der PageDesc nicht
    const sal_Bool bOdd = nTmp ? ( nTmp % 2 ? sal_True : sal_False )
                           : pNew->OnRightPage();
    if ( !pDesc )
        pDesc = pNew->FindPageDesc();
    const SwFlowFrm *pNewFlow = pNew->FindFirstBodyCntnt();
    // Haben wir uns selbst gefunden?
    if( pNewFlow == pFlow )
        pNewFlow = NULL;
    if ( pNewFlow && pNewFlow->GetFrm()->IsInTab() )
        pNewFlow = pNewFlow->GetFrm()->FindTabFrm();
    const SwPageDesc *pNewDesc= ( pNewFlow && !pNewFlow->IsFollow() )
            ? pNewFlow->GetFrm()->GetAttrSet()->GetPageDesc().GetPageDesc():0;

    return ( pNew->GetPageDesc() != pDesc ||   //  own desc ?
        pNew->GetFmt() != (bOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt()) ||
        ( pNewDesc && pNewDesc == pDesc ) );
}


/*************************************************************************
|*
|*  SwFrm::GetNextLeaf()
|*
|*  Beschreibung        Liefert das naechste LayoutBlatt in den das
|*      Frame gemoved werden kann.
|*
|*************************************************************************/

SwLayoutFrm *SwFrm::GetNextLeaf( MakePageType eMakePage )
{
    OSL_ENSURE( !IsInFtn(), "GetNextLeaf(), don't call me for Ftn." );
    OSL_ENSURE( !IsInSct(), "GetNextLeaf(), don't call me for Sections." );

    const sal_Bool bBody = IsInDocBody();       //Wenn ich aus dem DocBody komme
                                            //Will ich auch im Body landen.

    // Bei Flys macht es keinen Sinn, Seiten einzufuegen, wir wollen lediglich
     // die Verkettung absuchen.
    if( IsInFly() )
        eMakePage = MAKEPAGE_NONE;

    //Bei Tabellen gleich den grossen Sprung wagen, ein einfaches GetNext...
    //wuerde die erste Zellen und in der Folge alle weiteren Zellen nacheinander
    //abklappern....
    SwLayoutFrm *pLayLeaf = 0;
    if ( IsTabFrm() )
    {
        SwCntntFrm* pTmp = ((SwTabFrm*)this)->FindLastCntnt();
        if ( pTmp )
            pLayLeaf = pTmp->GetUpper();
    }
    if ( !pLayLeaf )
        pLayLeaf = GetNextLayoutLeaf();

    SwLayoutFrm *pOldLayLeaf = 0;           //Damit bei neu erzeugten Seiten
                                            //nicht wieder vom Anfang gesucht
                                            //wird.
    sal_Bool bNewPg = sal_False;    //nur einmal eine neue Seite einfuegen.

    while ( sal_True )
    {
        if ( pLayLeaf )
        {
            //Es gibt noch einen weiteren LayoutFrm, mal sehen,
            //ob er bereit ist mich aufzunehmen.
            //Dazu braucht er nur von der gleichen Art wie mein Ausgangspunkt
            //sein (DocBody bzw. Footnote.)
            if ( pLayLeaf->FindPageFrm()->IsFtnPage() )
            {   //Wenn ich bei den Endnotenseiten angelangt bin hat sichs.
                pLayLeaf = 0;
                continue;
            }
            if ( (bBody && !pLayLeaf->IsInDocBody()) || pLayLeaf->IsInTab()
                 || pLayLeaf->IsInSct() )
            {
                //Er will mich nicht; neuer Versuch, neues Glueck
                pOldLayLeaf = pLayLeaf;
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
                continue;
            }
            //Er will mich, also ist er der gesuchte und ich bin fertig.
            //Bei einem Seitenwechsel kann es allerdings noch sein, dass
            //Der Seitentyp nicht der gewuenschte ist, in diesem Fall muessen
            //wir eine Seite des richtigen Typs einfuegen.

            if( !IsFlowFrm() && ( eMakePage == MAKEPAGE_NONE ||
                eMakePage==MAKEPAGE_APPEND || eMakePage==MAKEPAGE_NOSECTION ) )
                return pLayLeaf;

            SwPageFrm *pNew = pLayLeaf->FindPageFrm();
            // #111704# The pagedesc check does not make sense for frames in fly frames
            if ( pNew != FindPageFrm() && !bNewPg && !IsInFly() &&
                 // --> FME 2005-05-10 #i46683#
                 // Do not consider page descriptions in browse mode (since
                 // MoveBwd ignored them)
                 !pNew->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
                 // <--
            {
                if( WrongPageDesc( pNew ) )
                {
                    SwFtnContFrm *pCont = pNew->FindFtnCont();
                    if( pCont )
                    {
                        // Falls die Referenz der ersten Fussnote dieser Seite
                        // vor der Seite liegt, fuegen wir lieber keine neue Seite
                        // ein (Bug #55620#)
                        SwFtnFrm *pFtn = (SwFtnFrm*)pCont->Lower();
                        if( pFtn && pFtn->GetRef() )
                        {
                            const sal_uInt16 nRefNum = pNew->GetPhyPageNum();
                            if( pFtn->GetRef()->GetPhyPageNum() < nRefNum )
                                break;
                        }
                    }
                    //Erwischt, die folgende Seite ist verkehrt, also
                    //muss eine neue eingefuegt werden.
                    if ( eMakePage == MAKEPAGE_INSERT )
                    {
                        bNewPg = sal_True;

                        SwPageFrm *pPg = pOldLayLeaf ?
                                    pOldLayLeaf->FindPageFrm() : 0;
                        if ( pPg && pPg->IsEmptyPage() )
                            //Nicht hinter, sondern vor der EmptyPage einfuegen.
                            pPg = (SwPageFrm*)pPg->GetPrev();

                        if ( !pPg || pPg == pNew )
                            pPg = FindPageFrm();

                        InsertPage( pPg, sal_False );
                        pLayLeaf = GetNextLayoutLeaf();
                        pOldLayLeaf = 0;
                        continue;
                    }
                    else
                        pLayLeaf = 0;
                }
            }
            break;
        }
        else
        {
            //Es gibt keinen passenden weiteren LayoutFrm, also muss eine
            //neue Seite her.
            if ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT )
            {
                InsertPage(
                    pOldLayLeaf ? pOldLayLeaf->FindPageFrm() : FindPageFrm(),
                    sal_False );

                //und nochmal das ganze
                pLayLeaf = pOldLayLeaf ? pOldLayLeaf : GetNextLayoutLeaf();
            }
            else
                break;
        }
    }
    return pLayLeaf;
}

/*************************************************************************
|*
|*  SwFrm::GetPrevLeaf()
|*
|*  Beschreibung        Liefert das vorhergehende LayoutBlatt in das der
|*      Frame gemoved werden kann.
|*
|*************************************************************************/


SwLayoutFrm *SwFrm::GetPrevLeaf( MakePageType )
{
    OSL_ENSURE( !IsInFtn(), "GetPrevLeaf(), don't call me for Ftn." );

    const sal_Bool bBody = IsInDocBody();       //Wenn ich aus dem DocBody komme
                                            //will ich auch im Body landen.
    const sal_Bool bFly  = IsInFly();

    SwLayoutFrm *pLayLeaf = GetPrevLayoutLeaf();
    SwLayoutFrm *pPrevLeaf = 0;

    while ( pLayLeaf )
    {
        if ( pLayLeaf->IsInTab() ||     //In Tabellen geht's niemals hinein.
             pLayLeaf->IsInSct() )      //In Bereiche natuerlich auch nicht!
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
        else if ( bBody && pLayLeaf->IsInDocBody() )
        {
            if ( pLayLeaf->Lower() )
                break;
            pPrevLeaf = pLayLeaf;
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
            if ( pLayLeaf )
                SwFlowFrm::SetMoveBwdJump( sal_True );
        }
        else if ( bFly )
            break;  //Cntnts in Flys sollte jedes Layout-Blatt recht sein.
        else
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
    }
    return pLayLeaf ? pLayLeaf : pPrevLeaf;
}

/*************************************************************************
|*
|*  SwFlowFrm::IsPrevObjMove()
|*
|*************************************************************************/


sal_Bool SwFlowFrm::IsPrevObjMove() const
{
    //sal_True der FlowFrm soll auf einen Rahmen des Vorgaengers Ruecksicht nehmen
    //     und fuer diesen ggf. Umbrechen.

    //!!!!!!!!!!!Hack!!!!!!!!!!!
    if ( rThis.GetUpper()->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
        return sal_False;

    SwFrm *pPre = rThis.FindPrev();

    if ( pPre && pPre->GetDrawObjs() )
    {
        OSL_ENSURE( SwFlowFrm::CastFlowFrm( pPre ), "new flowfrm?" );
        if( SwFlowFrm::CastFlowFrm( pPre )->IsAnFollow( this ) )
            return sal_False;
        SwLayoutFrm* pPreUp = pPre->GetUpper();
        // Wenn der Upper ein SectionFrm oder die Spalte eines SectionFrms ist,
        // duerfen wir aus diesem durchaus heraushaengen,
        // es muss stattdessen der Upper des SectionFrms beruecksichtigt werden.
        if( pPreUp->IsInSct() )
        {
            if( pPreUp->IsSctFrm() )
                pPreUp = pPreUp->GetUpper();
            else if( pPreUp->IsColBodyFrm() &&
                     pPreUp->GetUpper()->GetUpper()->IsSctFrm() )
                pPreUp = pPreUp->GetUpper()->GetUpper()->GetUpper();
        }
        // --> OD 2004-10-15 #i26945# - re-factoring:
        // use <GetVertPosOrientFrm()> to determine, if object has followed the
        // text flow to the next layout frame
        for ( sal_uInt16 i = 0; i < pPre->GetDrawObjs()->Count(); ++i )
        {
            // --> OD 2004-07-01 #i28701# - consider changed type of
            // <SwSortedObjs> entries.
            const SwAnchoredObject* pObj = (*pPre->GetDrawObjs())[i];
            // OD 2004-01-20 #110582# - do not consider hidden objects
            // --> OD 2004-10-15 #i26945# - do not consider object, which
            // doesn't follow the text flow.
            if ( pObj->GetFrmFmt().GetDoc()->IsVisibleLayerId(
                                            pObj->GetDrawObj()->GetLayer() ) &&
                 pObj->GetFrmFmt().GetFollowTextFlow().GetValue() )
            // <--
            {
                const SwLayoutFrm* pVertPosOrientFrm = pObj->GetVertPosOrientFrm();
                if ( pVertPosOrientFrm &&
                     pPreUp != pVertPosOrientFrm &&
                     !pPreUp->IsAnLower( pVertPosOrientFrm ) )
                {
                    return sal_True;
                }
            }
        }
        // <--
    }
    return sal_False;
}

/*************************************************************************
|*
|*  sal_Bool SwFlowFrm::IsPageBreak()
|*
|*  Beschreibung        Wenn vor dem Frm ein harter Seitenumbruch steht UND
|*      es einen Vorgaenger auf der gleichen Seite gibt, wird sal_True
|*      zurueckgeliefert (es muss ein PageBreak erzeugt werden) sal_False sonst.
|*      Wenn in bAct sal_True uebergeben wird, gibt die Funktion dann sal_True
|*      zurueck, wenn ein PageBreak besteht.
|*      Fuer Follows wird der harte Seitenumbruch natuerlich nicht
|*      ausgewertet.
|*      Der Seitenumbruch steht im eigenen FrmFmt (BEFORE) oder im FrmFmt
|*      des Vorgaengers (AFTER). Wenn es keinen Vorgaenger auf der Seite
|*      gibt ist jede weitere Ueberlegung ueberfluessig.
|*      Ein Seitenumbruch (oder der Bedarf) liegt auch dann vor, wenn
|*      im FrmFmt ein PageDesc angegeben wird.
|*      Die Implementierung arbeitet zuaechst nur auf CntntFrms!
|*      -->Fuer LayoutFrms ist die Definition des Vorgaengers unklar.
|*
|*************************************************************************/


sal_Bool SwFlowFrm::IsPageBreak( sal_Bool bAct ) const
{
    if ( !IsFollow() && rThis.IsInDocBody() &&
         ( !rThis.IsInTab() || ( rThis.IsTabFrm() && !rThis.GetUpper()->IsInTab() ) ) && // i66968
         !rThis.GetUpper()->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
    {
        const SwAttrSet *pSet = rThis.GetAttrSet();

        //Vorgaenger ermitteln
        const SwFrm *pPrev = rThis.FindPrev();
        while ( pPrev && ( !pPrev->IsInDocBody() ||
                ( pPrev->IsTxtFrm() && ((SwTxtFrm*)pPrev)->IsHiddenNow() ) ) )
            pPrev = pPrev->FindPrev();

        if ( pPrev )
        {
            OSL_ENSURE( pPrev->IsInDocBody(), "IsPageBreak: Not in DocBody?" );
            if ( bAct )
            {   if ( rThis.FindPageFrm() == pPrev->FindPageFrm() )
                    return sal_False;
            }
            else
            {   if ( rThis.FindPageFrm() != pPrev->FindPageFrm() )
                    return sal_False;
            }

            const SvxBreak eBreak = pSet->GetBreak().GetBreak();
            if ( eBreak == SVX_BREAK_PAGE_BEFORE || eBreak == SVX_BREAK_PAGE_BOTH )
                return sal_True;
            else
            {
                const SvxBreak &ePrB = pPrev->GetAttrSet()->GetBreak().GetBreak();
                if ( ePrB == SVX_BREAK_PAGE_AFTER ||
                     ePrB == SVX_BREAK_PAGE_BOTH  ||
                     pSet->GetPageDesc().GetPageDesc() )
                    return sal_True;
            }
        }
    }
    return sal_False;
}

/*************************************************************************
|*
|*  sal_Bool SwFlowFrm::IsColBreak()
|*
|*  Beschreibung        Wenn vor dem Frm ein harter Spaltenumbruch steht UND
|*      es einen Vorgaenger in der gleichen Spalte gibt, wird sal_True
|*      zurueckgeliefert (es muss ein PageBreak erzeugt werden) sal_False sonst.
|*      Wenn in bAct sal_True uebergeben wird, gibt die Funktion dann sal_True
|*      zurueck, wenn ein ColBreak besteht.
|*      Fuer Follows wird der harte Spaltenumbruch natuerlich nicht
|*      ausgewertet.
|*      Der Spaltenumbruch steht im eigenen FrmFmt (BEFORE) oder im FrmFmt
|*      des Vorgaengers (AFTER). Wenn es keinen Vorgaenger in der Spalte
|*      gibt ist jede weitere Ueberlegung ueberfluessig.
|*      Die Implementierung arbeitet zuaechst nur auf CntntFrms!
|*      -->Fuer LayoutFrms ist die Definition des Vorgaengers unklar.
|*
|*************************************************************************/

sal_Bool SwFlowFrm::IsColBreak( sal_Bool bAct ) const
{
    if ( !IsFollow() && (rThis.IsMoveable() || bAct) )
    {
        const SwFrm *pCol = rThis.FindColFrm();
        if ( pCol )
        {
            //Vorgaenger ermitteln
            const SwFrm *pPrev = rThis.FindPrev();
            while( pPrev && ( ( !pPrev->IsInDocBody() && !rThis.IsInFly() ) ||
                   ( pPrev->IsTxtFrm() && ((SwTxtFrm*)pPrev)->IsHiddenNow() ) ) )
                    pPrev = pPrev->FindPrev();

            if ( pPrev )
            {
                if ( bAct )
                {   if ( pCol == pPrev->FindColFrm() )
                        return sal_False;
                }
                else
                {   if ( pCol != pPrev->FindColFrm() )
                        return sal_False;
                }

                const SvxBreak eBreak = rThis.GetAttrSet()->GetBreak().GetBreak();
                if ( eBreak == SVX_BREAK_COLUMN_BEFORE ||
                     eBreak == SVX_BREAK_COLUMN_BOTH )
                    return sal_True;
                else
                {
                    const SvxBreak &ePrB = pPrev->GetAttrSet()->GetBreak().GetBreak();
                    if ( ePrB == SVX_BREAK_COLUMN_AFTER ||
                         ePrB == SVX_BREAK_COLUMN_BOTH )
                        return sal_True;
                }
            }
        }
    }
    return sal_False;
}

sal_Bool SwFlowFrm::HasParaSpaceAtPages( sal_Bool bSct ) const
{
    if( rThis.IsInSct() )
    {
        const SwFrm* pTmp = rThis.GetUpper();
        while( pTmp )
        {
            if( pTmp->IsCellFrm() || pTmp->IsFlyFrm() ||
                pTmp->IsFooterFrm() || pTmp->IsHeaderFrm() ||
                ( pTmp->IsFtnFrm() && !((SwFtnFrm*)pTmp)->GetMaster() ) )
                return sal_True;
            if( pTmp->IsPageFrm() )
                return ( pTmp->GetPrev() && !IsPageBreak(sal_True) ) ? sal_False : sal_True;
            if( pTmp->IsColumnFrm() && pTmp->GetPrev() )
                return IsColBreak( sal_True );
            if( pTmp->IsSctFrm() && ( !bSct || pTmp->GetPrev() ) )
                return sal_False;
            pTmp = pTmp->GetUpper();
        }
        OSL_ENSURE( sal_False, "HasParaSpaceAtPages: Where's my page?" );
        return sal_False;
    }
    if( !rThis.IsInDocBody() || ( rThis.IsInTab() && !rThis.IsTabFrm()) ||
        IsPageBreak( sal_True ) || ( rThis.FindColFrm() && IsColBreak( sal_True ) ) )
        return sal_True;
    const SwFrm* pTmp = rThis.FindColFrm();
    if( pTmp )
    {
        if( pTmp->GetPrev() )
            return sal_False;
    }
    else
        pTmp = &rThis;
    pTmp = pTmp->FindPageFrm();
    return pTmp && !pTmp->GetPrev();
}

/** helper method to determine previous frame for calculation of the
    upper space

    OD 2004-03-10 #i11860#

    @author OD
*/
const SwFrm* SwFlowFrm::_GetPrevFrmForUpperSpaceCalc( const SwFrm* _pProposedPrevFrm ) const
{
    const SwFrm* pPrevFrm = _pProposedPrevFrm
                            ? _pProposedPrevFrm
                            : rThis.GetPrev();

    // Skip hidden paragraphs and empty sections
    while ( pPrevFrm &&
            ( ( pPrevFrm->IsTxtFrm() &&
                static_cast<const SwTxtFrm*>(pPrevFrm)->IsHiddenNow() ) ||
              ( pPrevFrm->IsSctFrm() &&
                !static_cast<const SwSectionFrm*>(pPrevFrm)->GetSection() ) ) )
    {
        pPrevFrm = pPrevFrm->GetPrev();
    }

    // Special case: no direct previous frame is found but frame is in footnote
    // Search for a previous frame in previous footnote,
    // if frame isn't in a section, which is also in the footnote
    if ( !pPrevFrm && rThis.IsInFtn() &&
         ( rThis.IsSctFrm() ||
           !rThis.IsInSct() || !rThis.FindSctFrm()->IsInFtn() ) )
    {
        const SwFtnFrm* pPrevFtnFrm =
                static_cast<const SwFtnFrm*>(rThis.FindFtnFrm()->GetPrev());
        if ( pPrevFtnFrm )
        {
            pPrevFrm = pPrevFtnFrm->GetLastLower();

            // Skip hidden paragraphs and empty sections
            while ( pPrevFrm &&
                    ( ( pPrevFrm->IsTxtFrm() &&
                        static_cast<const SwTxtFrm*>(pPrevFrm)->IsHiddenNow() ) ||
                      ( pPrevFrm->IsSctFrm() &&
                        !static_cast<const SwSectionFrm*>(pPrevFrm)->GetSection() ) ) )
            {
                pPrevFrm = pPrevFrm->GetPrev();
            }
        }
    }
    // Special case: found previous frame is a section
    // Search for the last content in the section
    if( pPrevFrm && pPrevFrm->IsSctFrm() )
    {
        const SwSectionFrm* pPrevSectFrm =
                                    static_cast<const SwSectionFrm*>(pPrevFrm);
        pPrevFrm = pPrevSectFrm->FindLastCntnt();
        // If the last content is in a table _inside_ the section,
        // take the table herself.
        // OD 2004-02-18 #106629# - correction:
        // Check directly, if table is inside table, instead of indirectly
        // by checking, if section isn't inside a table
        if ( pPrevFrm && pPrevFrm->IsInTab() )
        {
            const SwTabFrm* pTableFrm = pPrevFrm->FindTabFrm();
            if ( pPrevSectFrm->IsAnLower( pTableFrm ) )
            {
                pPrevFrm = pTableFrm;
            }
        }
        // OD 2004-02-18 #106629# correction: skip hidden text frames
        while ( pPrevFrm &&
                pPrevFrm->IsTxtFrm() &&
                static_cast<const SwTxtFrm*>(pPrevFrm)->IsHiddenNow() )
        {
            pPrevFrm = pPrevFrm->GetPrev();
        }
    }

    return pPrevFrm;
}

// OD 2004-03-12 #i11860# - add 3rd parameter <_bConsiderGrid>
SwTwips SwFlowFrm::CalcUpperSpace( const SwBorderAttrs *pAttrs,
                                   const SwFrm* pPr,
                                   const bool _bConsiderGrid ) const
{
    // OD 2004-03-10 #i11860# - use new method <GetPrevFrmForUpperSpaceCalc(..)>
    const SwFrm* pPrevFrm = _GetPrevFrmForUpperSpaceCalc( pPr );

    SwBorderAttrAccess *pAccess;
    SwFrm* pOwn;
    if( !pAttrs )
    {
        if( rThis.IsSctFrm() )
        {
            SwSectionFrm* pFoll = &((SwSectionFrm&)rThis);
            do
                pOwn = pFoll->ContainsAny();
            while( !pOwn && 0 != ( pFoll = pFoll->GetFollow() ) );
            if( !pOwn )
                return 0;
        }
        else
            pOwn = &rThis;
        pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), pOwn );
        pAttrs = pAccess->Get();
    }
    else
    {
        pAccess = NULL;
        pOwn = &rThis;
    }
    SwTwips nUpper = 0;
    // OD 06.01.2004 #i11859#
    {
        const IDocumentSettingAccess* pIDSA = rThis.GetUpper()->GetFmt()->getIDocumentSettingAccess();
        const bool bUseFormerLineSpacing = pIDSA->get(IDocumentSettingAccess::OLD_LINE_SPACING);
        if( pPrevFrm )
        {
            // OD 2004-03-10 #i11860# - use new method to determine needed spacing
            // values of found previous frame and use these values.
            SwTwips nPrevLowerSpace = 0;
            SwTwips nPrevLineSpacing = 0;
            // --> OD 2009-08-28 #i102458#
            bool bPrevLineSpacingPorportional = false;
            GetSpacingValuesOfFrm( (*pPrevFrm),
                                   nPrevLowerSpace, nPrevLineSpacing,
                                   bPrevLineSpacingPorportional );
            // <--
            if( pIDSA->get(IDocumentSettingAccess::PARA_SPACE_MAX) )
            {
                nUpper = nPrevLowerSpace + pAttrs->GetULSpace().GetUpper();
                SwTwips nAdd = nPrevLineSpacing;
                // OD 07.01.2004 #i11859# - consideration of the line spacing
                //      for the upper spacing of a text frame
                if ( bUseFormerLineSpacing )
                {
                    // former consideration
                    if ( pOwn->IsTxtFrm() )
                    {
                        nAdd = Max( nAdd, static_cast<SwTxtFrm&>(rThis).GetLineSpace() );
                    }
                    nUpper += nAdd;
                }
                else
                {
                    // new consideration:
                    //      Only the proportional line spacing of the previous
                    //      text frame is considered for the upper spacing and
                    //      the line spacing values are add up instead of
                    //      building its maximum.
                    if ( pOwn->IsTxtFrm() )
                    {
                        // --> OD 2009-08-28 #i102458#
                        // Correction:
                        // A proportional line spacing of the previous text frame
                        // is added up to a own leading line spacing.
                        // Otherwise, the maximum of the leading line spacing
                        // of the previous text frame and the own leading line
                        // spacing is built.
                        if ( bPrevLineSpacingPorportional )
                        {
                            nAdd += static_cast<SwTxtFrm&>(rThis).GetLineSpace( true );
                        }
                        else
                        {
                            nAdd = Max( nAdd, static_cast<SwTxtFrm&>(rThis).GetLineSpace( true ) );
                        }
                        // <--
                    }
                    nUpper += nAdd;
                }
            }
            else
            {
                nUpper = Max( static_cast<long>(nPrevLowerSpace),
                              static_cast<long>(pAttrs->GetULSpace().GetUpper()) );
                // OD 07.01.2004 #i11859# - consideration of the line spacing
                //      for the upper spacing of a text frame
                if ( bUseFormerLineSpacing )
                {
                    // former consideration
                    if ( pOwn->IsTxtFrm() )
                        nUpper = Max( nUpper, ((SwTxtFrm*)pOwn)->GetLineSpace() );
                    if ( nPrevLineSpacing != 0 )
                    {
                        nUpper = Max( nUpper, nPrevLineSpacing );
                    }
                }
                else
                {
                    // new consideration:
                    //      Only the proportional line spacing of the previous
                    //      text frame is considered for the upper spacing and
                    //      the line spacing values are add up and added to
                    //      the paragraph spacing instead of building the
                    //      maximum of the line spacings and the paragraph spacing.
                    SwTwips nAdd = nPrevLineSpacing;
                    if ( pOwn->IsTxtFrm() )
                    {
                        // --> OD 2009-08-28 #i102458#
                        // Correction:
                        // A proportional line spacing of the previous text frame
                        // is added up to a own leading line spacing.
                        // Otherwise, the maximum of the leading line spacing
                        // of the previous text frame and the own leading line
                        // spacing is built.
                        if ( bPrevLineSpacingPorportional )
                        {
                            nAdd += static_cast<SwTxtFrm&>(rThis).GetLineSpace( true );
                        }
                        else
                        {
                            nAdd = Max( nAdd, static_cast<SwTxtFrm&>(rThis).GetLineSpace( true ) );
                        }
                        // <--
                    }
                    nUpper += nAdd;
                }
            }
        }
        else if ( pIDSA->get(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES) &&
                  CastFlowFrm( pOwn )->HasParaSpaceAtPages( rThis.IsSctFrm() ) )
        {
            nUpper = pAttrs->GetULSpace().GetUpper();
        }
    }

    // OD 2004-02-26 #i25029# - pass previous frame <pPrevFrm>
    // to method <GetTopLine(..)>, if parameter <pPr> is set.
    // Note: parameter <pPr> is set, if method is called from <SwTxtFrm::WouldFit(..)>
    nUpper += pAttrs->GetTopLine( rThis, (pPr ? pPrevFrm : 0L) );

    // OD 2004-03-12 #i11860# - consider value of new parameter <_bConsiderGrid>
    // and use new method <GetUpperSpaceAmountConsideredForPageGrid(..)>

    //consider grid in square page mode
    if ( _bConsiderGrid && rThis.GetUpper()->GetFmt()->GetDoc()->IsSquaredPageMode() )
    {
        nUpper += _GetUpperSpaceAmountConsideredForPageGrid( nUpper );
    }

    delete pAccess;
    return nUpper;
}

/** method to detemine the upper space amount, which is considered for
    the page grid

    OD 2004-03-12 #i11860#
    Precondition: Position of frame is valid.

    @author OD
*/
SwTwips SwFlowFrm::_GetUpperSpaceAmountConsideredForPageGrid(
                            const SwTwips _nUpperSpaceWithoutGrid ) const
{
    SwTwips nUpperSpaceAmountConsideredForPageGrid = 0;

    if ( rThis.IsInDocBody() && rThis.GetAttrSet()->GetParaGrid().GetValue() )
    {
        const SwPageFrm* pPageFrm = rThis.FindPageFrm();
        GETGRID( pPageFrm )
        if( pGrid )
        {
            const SwFrm* pBodyFrm = pPageFrm->FindBodyCont();
            if ( pBodyFrm )
            {
                const long nGridLineHeight =
                        pGrid->GetBaseHeight() + pGrid->GetRubyHeight();

                SWRECTFN( (&rThis) )
                const SwTwips nBodyPrtTop = (pBodyFrm->*fnRect->fnGetPrtTop)();
                const SwTwips nProposedPrtTop =
                        (*fnRect->fnYInc)( (rThis.Frm().*fnRect->fnGetTop)(),
                                           _nUpperSpaceWithoutGrid );

                const SwTwips nSpaceAbovePrtTop =
                        (*fnRect->fnYDiff)( nProposedPrtTop, nBodyPrtTop );
                const SwTwips nSpaceOfCompleteLinesAbove =
                        nGridLineHeight * ( nSpaceAbovePrtTop / nGridLineHeight );
                SwTwips nNewPrtTop =
                        (*fnRect->fnYInc)( nBodyPrtTop, nSpaceOfCompleteLinesAbove );
                if ( (*fnRect->fnYDiff)( nProposedPrtTop, nNewPrtTop ) > 0 )
                {
                    nNewPrtTop = (*fnRect->fnYInc)( nNewPrtTop, nGridLineHeight );
                }

                const SwTwips nNewUpperSpace =
                        (*fnRect->fnYDiff)( nNewPrtTop,
                                            (rThis.Frm().*fnRect->fnGetTop)() );

                nUpperSpaceAmountConsideredForPageGrid =
                        nNewUpperSpace - _nUpperSpaceWithoutGrid;

                OSL_ENSURE( nUpperSpaceAmountConsideredForPageGrid >= 0,
                        "<SwFlowFrm::GetUpperSpaceAmountConsideredForPageGrid(..)> - negative space considered for page grid!" );
            }
        }
    }
    return nUpperSpaceAmountConsideredForPageGrid;
}

/** method to determent the upper space amount, which is considered for
    the previous frame

    OD 2004-03-11 #i11860#

    @author OD
*/
SwTwips SwFlowFrm::_GetUpperSpaceAmountConsideredForPrevFrm() const
{
    SwTwips nUpperSpaceAmountOfPrevFrm = 0;

    const SwFrm* pPrevFrm = _GetPrevFrmForUpperSpaceCalc();
    if ( pPrevFrm )
    {
        SwTwips nPrevLowerSpace = 0;
        SwTwips nPrevLineSpacing = 0;
        // --> OD 2009-08-28 #i102458#
        bool bDummy = false;
        GetSpacingValuesOfFrm( (*pPrevFrm), nPrevLowerSpace, nPrevLineSpacing, bDummy );
        // <--
        if ( nPrevLowerSpace > 0 || nPrevLineSpacing > 0 )
        {
            const IDocumentSettingAccess* pIDSA = rThis.GetUpper()->GetFmt()->getIDocumentSettingAccess();
            if (  pIDSA->get(IDocumentSettingAccess::PARA_SPACE_MAX) ||
                 !pIDSA->get(IDocumentSettingAccess::OLD_LINE_SPACING) )
            {
                nUpperSpaceAmountOfPrevFrm = nPrevLowerSpace + nPrevLineSpacing;
            }
            else
            {
                nUpperSpaceAmountOfPrevFrm = Max( nPrevLowerSpace, nPrevLineSpacing );
            }
        }
    }

    return nUpperSpaceAmountOfPrevFrm;
}

/** method to determine the upper space amount, which is considered for
    the previous frame and the page grid, if option 'Use former object
    positioning' is OFF

    OD 2004-03-18 #i11860#

    @author OD
*/
SwTwips SwFlowFrm::GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() const
{
    SwTwips nUpperSpaceAmountConsideredForPrevFrmAndPageGrid = 0;

    if ( !rThis.GetUpper()->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::USE_FORMER_OBJECT_POS) )
    {
        nUpperSpaceAmountConsideredForPrevFrmAndPageGrid =
            _GetUpperSpaceAmountConsideredForPrevFrm() +
            _GetUpperSpaceAmountConsideredForPageGrid( CalcUpperSpace( 0L, 0L, false ) );
    }

    return nUpperSpaceAmountConsideredForPrevFrmAndPageGrid;
}

/** calculation of lower space

    OD 2004-03-02 #106629#

    @author OD
*/
SwTwips SwFlowFrm::CalcLowerSpace( const SwBorderAttrs* _pAttrs ) const
{
    SwTwips nLowerSpace = 0;

    SwBorderAttrAccess* pAttrAccess = 0L;
    if ( !_pAttrs )
    {
        pAttrAccess = new SwBorderAttrAccess( SwFrm::GetCache(), &rThis );
        _pAttrs = pAttrAccess->Get();
    }

    sal_Bool bCommonBorder = sal_True;
    if ( rThis.IsInSct() && rThis.GetUpper()->IsColBodyFrm() )
    {
        const SwSectionFrm* pSectFrm = rThis.FindSctFrm();
        bCommonBorder = pSectFrm->GetFmt()->GetBalancedColumns().GetValue();
    }
    nLowerSpace = bCommonBorder ?
                  _pAttrs->GetBottomLine( rThis ) :
                  _pAttrs->CalcBottomLine();

    // --> OD 2004-07-16 #i26250#
    // - correct consideration of table frames
    // - use new method <CalcAddLowerSpaceAsLastInTableCell(..)>
    if ( ( ( rThis.IsTabFrm() && rThis.GetUpper()->IsInTab() ) ||
           // --> OD 2004-11-16 #115759# - no lower spacing, if frame has a follow
           ( rThis.IsInTab() && !GetFollow() ) ) &&
           // <--
         !rThis.GetIndNext() )
    {
        nLowerSpace += CalcAddLowerSpaceAsLastInTableCell( _pAttrs );
    }
    // <--

    delete pAttrAccess;

    return nLowerSpace;
}

/** calculation of the additional space to be considered, if flow frame
    is the last inside a table cell

    OD 2004-07-16 #i26250#

    @author OD
*/
SwTwips SwFlowFrm::CalcAddLowerSpaceAsLastInTableCell(
                                            const SwBorderAttrs* _pAttrs ) const
{
    SwTwips nAdditionalLowerSpace = 0;

    if ( rThis.GetUpper()->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS) )
    {
        const SwFrm* pFrm = &rThis;
        if ( pFrm->IsSctFrm() )
        {
            const SwSectionFrm* pSectFrm = static_cast<const SwSectionFrm*>(pFrm);
            pFrm = pSectFrm->FindLastCntnt();
            if ( pFrm && pFrm->IsInTab() )
            {
                const SwTabFrm* pTableFrm = pFrm->FindTabFrm();
                if ( pSectFrm->IsAnLower( pTableFrm ) )
                {
                    pFrm = pTableFrm;
                }
            }
        }

        SwBorderAttrAccess* pAttrAccess = 0L;
        if ( !_pAttrs || pFrm != &rThis )
        {
            pAttrAccess = new SwBorderAttrAccess( SwFrm::GetCache(), pFrm );
            _pAttrs = pAttrAccess->Get();
        }

        nAdditionalLowerSpace += _pAttrs->GetULSpace().GetLower();

        delete pAttrAccess;
    }

    return nAdditionalLowerSpace;
}

/*************************************************************************
|*
|*  sal_Bool SwFlowFrm::CheckMoveFwd()
|*
|*  Beschreibung        Moved den Frm vorwaerts wenn es durch die aktuellen
|*      Bedingungen und Attribute notwendig erscheint.
|*
|*************************************************************************/


sal_Bool SwFlowFrm::CheckMoveFwd( sal_Bool &rbMakePage, sal_Bool bKeep, sal_Bool )
{
    const SwFrm* pNxt = rThis.GetIndNext();

    if ( bKeep && //!bMovedBwd &&
         ( !pNxt || ( pNxt->IsTxtFrm() && ((SwTxtFrm*)pNxt)->IsEmptyMaster() ) ) &&
         ( 0 != (pNxt = rThis.FindNext()) ) && IsKeepFwdMoveAllowed() )
    {
        if( pNxt->IsSctFrm() )
        {   // Nicht auf leere SectionFrms hereinfallen
            const SwFrm* pTmp = NULL;
            while( pNxt && pNxt->IsSctFrm() &&
                   ( !((SwSectionFrm*)pNxt)->GetSection() ||
                     0 == ( pTmp = ((SwSectionFrm*)pNxt)->ContainsAny() ) ) )
            {
                pNxt = pNxt->FindNext();
                pTmp = NULL;
            }
            if( pTmp )
                pNxt = pTmp; // the content of the next notempty sectionfrm
        }
        if( pNxt && pNxt->GetValidPosFlag() )
        {
            sal_Bool bMove = sal_False;
            const SwSectionFrm *pSct = rThis.FindSctFrm();
            if( pSct && !pSct->GetValidSizeFlag() )
            {
                const SwSectionFrm* pNxtSct = pNxt->FindSctFrm();
                if( pNxtSct && pSct->IsAnFollow( pNxtSct ) )
                    bMove = sal_True;
            }
            else
                bMove = sal_True;
            if( bMove )
            {
                //Keep together with the following frame
                MoveFwd( rbMakePage, sal_False );
                return sal_True;
            }
        }
    }

    sal_Bool bMovedFwd = sal_False;

    if ( rThis.GetIndPrev() )
    {
        if ( IsPrevObjMove() ) //Auf Objekte des Prev Ruecksicht nehmen?
        {
            bMovedFwd = sal_True;
            if ( !MoveFwd( rbMakePage, sal_False ) )
                rbMakePage = sal_False;
        }
        else
        {
            if ( IsPageBreak( sal_False ) )
            {
                while ( MoveFwd( rbMakePage, sal_True ) )
                        /* do nothing */;
                rbMakePage = sal_False;
                bMovedFwd = sal_True;
            }
            else if ( IsColBreak ( sal_False ) )
            {
                const SwPageFrm *pPage = rThis.FindPageFrm();
                SwFrm *pCol = rThis.FindColFrm();
                do
                {   MoveFwd( rbMakePage, sal_False );
                    SwFrm *pTmp = rThis.FindColFrm();
                    if( pTmp != pCol )
                    {
                        bMovedFwd = sal_True;
                        pCol = pTmp;
                    }
                    else
                        break;
                } while ( IsColBreak( sal_False ) );
                if ( pPage != rThis.FindPageFrm() )
                    rbMakePage = sal_False;
            }
        }
    }
    return bMovedFwd;
}

/*************************************************************************
|*
|*  sal_Bool SwFlowFrm::MoveFwd()
|*
|*  Beschreibung        Returnwert sagt, ob der Frm die Seite gewechselt hat.
|*
|*************************************************************************/


sal_Bool SwFlowFrm::MoveFwd( sal_Bool bMakePage, sal_Bool bPageBreak, sal_Bool bMoveAlways )
{
//!!!!MoveFtnCntFwd muss ggf. mitgepflegt werden.
    SwFtnBossFrm *pOldBoss = rThis.FindFtnBossFrm();
    if ( rThis.IsInFtn() )
        return ((SwCntntFrm&)rThis).MoveFtnCntFwd( bMakePage, pOldBoss );

    if( !IsFwdMoveAllowed() && !bMoveAlways )
    {
        sal_Bool bNoFwd = sal_True;
        if( rThis.IsInSct() )
        {
            SwFtnBossFrm* pBoss = rThis.FindFtnBossFrm();
            bNoFwd = !pBoss->IsInSct() || ( !pBoss->Lower()->GetNext() &&
                     !pBoss->GetPrev() );
        }

        // Allow the MoveFwd even if we do not have an IndPrev in these cases:
        if ( rThis.IsInTab() &&
            ( !rThis.IsTabFrm() ||
                ( rThis.GetUpper()->IsInTab() &&
                  rThis.GetUpper()->FindTabFrm()->IsFwdMoveAllowed() ) ) &&
             0 != const_cast<SwFrm&>(rThis).GetNextCellLeaf( MAKEPAGE_NONE ) )
        {
            bNoFwd = sal_False;
        }

        if( bNoFwd )
        {
            //Fuer PageBreak ist das Moven erlaubt, wenn der Frm nicht
            //bereits der erste der Seite ist.
            if ( !bPageBreak )
                return sal_False;

            const SwFrm *pCol = rThis.FindColFrm();
            if ( !pCol || !pCol->GetPrev() )
                return sal_False;
        }
    }

    sal_Bool bSamePage = sal_True;
    SwLayoutFrm *pNewUpper =
            rThis.GetLeaf( bMakePage ? MAKEPAGE_INSERT : MAKEPAGE_NONE, sal_True );

    if ( pNewUpper )
    {
        PROTOCOL_ENTER( &rThis, PROT_MOVE_FWD, 0, 0 );
        SwPageFrm *pOldPage = pOldBoss->FindPageFrm();
        //Wir moven uns und alle direkten Nachfolger vor den ersten
        //CntntFrm unterhalb des neuen Uppers.

        // Wenn unser NewUpper in einem SectionFrm liegt, muessen wir
        // verhindern, dass sich dieser im Calc selbst zerstoert
        SwSectionFrm* pSect = pNewUpper->FindSctFrm();
        sal_Bool bUnlock = sal_False;
        if( pSect )
        {
            // Wenn wir nur innerhalb unseres SectionFrms die Spalte wechseln,
            // rufen wir lieber kein Calc, sonst wird noch der SectionFrm
            // formatiert, der wiederum uns ruft etc.
            if( pSect != rThis.FindSctFrm() )
            {
                bUnlock = !pSect->IsColLocked();
                pSect->ColLock();
                pNewUpper->Calc();
                if( bUnlock )
                    pSect->ColUnlock();
            }
        }
        // Do not calculate split cell frames.
        else if ( !pNewUpper->IsCellFrm() || ((SwLayoutFrm*)pNewUpper)->Lower() )
            pNewUpper->Calc();

        SwFtnBossFrm *pNewBoss = pNewUpper->FindFtnBossFrm();
        sal_Bool bBossChg = pNewBoss != pOldBoss;
        pNewBoss = pNewBoss->FindFtnBossFrm( sal_True );
        pOldBoss = pOldBoss->FindFtnBossFrm( sal_True );
        SwPageFrm* pNewPage = pOldPage;

        // First, we move the footnotes.
        sal_Bool bFtnMoved = sal_False;

        // --> FME 2004-07-15 #i26831#
        // If pSect has just been created, the printing area of pSect has
        // been calculated based on the first content of its follow.
        // In this case we prefer to call a SimpleFormat for this new
        // section after we inserted the contents. Otherwise the section
        // frame will invalidate its lowers, if its printing area changes
        // in SwSectionFrm::Format, which can cause loops.
        const bool bForceSimpleFormat = pSect && pSect->HasFollow() &&
                                       !pSect->ContainsAny();
        // <--

        if ( pNewBoss != pOldBoss )
        {
            pNewPage = pNewBoss->FindPageFrm();
            bSamePage = pNewPage == pOldPage;
            //Damit die Fussnoten nicht auf dumme Gedanken kommen
            //setzen wir hier die Deadline.
            SWRECTFN( pOldBoss )
            SwSaveFtnHeight aHeight( pOldBoss,
                (pOldBoss->Frm().*fnRect->fnGetBottom)() );
            SwCntntFrm* pStart = rThis.IsCntntFrm() ?
                (SwCntntFrm*)&rThis : ((SwLayoutFrm&)rThis).ContainsCntnt();
            OSL_ENSURE( pStart || ( rThis.IsTabFrm() && !((SwTabFrm&)rThis).Lower() ),
                    "MoveFwd: Missing Content" );
            SwLayoutFrm* pBody = pStart ? ( pStart->IsTxtFrm() ?
                (SwLayoutFrm*)((SwTxtFrm*)pStart)->FindBodyFrm() : 0 ) : 0;
            if( pBody )
                bFtnMoved = pBody->MoveLowerFtns( pStart, pOldBoss, pNewBoss,
                                                  sal_False);
        }
        // Bei SectionFrms ist es moeglich, dass wir selbst durch pNewUpper->Calc()
        // bewegt wurden, z. B. in den pNewUpper.
        // MoveSubTree bzw. PasteTree ist auf so etwas nicht vorbereitet.
        if( pNewUpper != rThis.GetUpper() )
        {
            // --> FME 2004-04-19 #i27145#
            SwSectionFrm* pOldSct = 0;
            if ( rThis.GetUpper()->IsSctFrm() )
            {
                pOldSct = static_cast<SwSectionFrm*>(rThis.GetUpper());
            }
            // <--

            MoveSubTree( pNewUpper, pNewUpper->Lower() );

            // --> FME 2004-04-19 #i27145#
            if ( pOldSct && pOldSct->GetSection() )
            {
                // Prevent loops by setting the new height at
                // the section frame if footnotes have been moved.
                // Otherwise the call of SwLayNotify::~SwLayNotify() for
                // the (invalid) section frame will invalidate the first
                // lower of its follow, because it grows due to the removed
                // footnotes.
                // Note: If pOldSct has become empty during MoveSubTree, it
                // has already been scheduled for removal. No SimpleFormat
                // for these.
                pOldSct->SimpleFormat();
            }
            // <--

            // --> FME 2004-07-15 #i26831#
            if ( bForceSimpleFormat )
            {
                pSect->SimpleFormat();
            }
            // <--

            if ( bFtnMoved && !bSamePage )
            {
                pOldPage->UpdateFtnNum();
                pNewPage->UpdateFtnNum();
            }

            if( bBossChg )
            {
                rThis.Prepare( PREP_BOSS_CHGD, 0, sal_False );
                if( !bSamePage )
                {
                    ViewShell *pSh = rThis.GetShell();
                    if ( pSh && !pSh->Imp()->IsUpdateExpFlds() )
                        pSh->GetDoc()->SetNewFldLst(true);  //Wird von CalcLayout() hinterher erledigt!

                    pNewPage->InvalidateSpelling();
                    pNewPage->InvalidateSmartTags();    // SMARTTAGS
                    pNewPage->InvalidateAutoCompleteWords();
                    pNewPage->InvalidateWordCount();
                }
            }
        }
        // OD 30.10.2002 #97265# - no <CheckPageDesc(..)> in online layout
        if ( !pNewPage->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
        {
            // --> OD 2009-12-31 #i106452#
            // check page description not only in situation with sections.
            if ( !bSamePage &&
                 ( rThis.GetAttrSet()->GetPageDesc().GetPageDesc() ||
                   pOldPage->GetPageDesc()->GetFollow() != pNewPage->GetPageDesc() ) )
            {
                SwFrm::CheckPageDescs( pNewPage, sal_False );
            }
            // <--
        }
    }
    return bSamePage;
}


/*************************************************************************
|*
|*  sal_Bool SwFlowFrm::MoveBwd()
|*
|*  Beschreibung        Returnwert sagt, ob der Frm die Seite wechseln soll.
|*                      Sollte von abgeleiteten Klassen gerufen werden.
|*                      Das moven selbst muessen die abgeleiteten uebernehmen.
|*
|*************************************************************************/

sal_Bool SwFlowFrm::MoveBwd( sal_Bool &rbReformat )
{
    SwFlowFrm::SetMoveBwdJump( sal_False );

    SwFtnFrm* pFtn = rThis.FindFtnFrm();
    if ( pFtn && pFtn->IsBackMoveLocked() )
        return sal_False;

    // --> OD 2004-11-29 #115759# - text frames, which are directly inside
    // tables aren't allowed to move backward.
    if ( rThis.IsTxtFrm() && rThis.IsInTab() )
    {
        const SwLayoutFrm* pUpperFrm = rThis.GetUpper();
        while ( pUpperFrm )
        {
            if ( pUpperFrm->IsTabFrm() )
            {
                return sal_False;
            }
            else if ( pUpperFrm->IsColumnFrm() && pUpperFrm->IsInSct() )
            {
                break;
            }
            pUpperFrm = pUpperFrm->GetUpper();
        }
    }
    // <--

    SwFtnBossFrm * pOldBoss = rThis.FindFtnBossFrm();
    SwPageFrm * const pOldPage = pOldBoss->FindPageFrm();
    SwLayoutFrm *pNewUpper = 0;
    sal_Bool bCheckPageDescs = sal_False;
    bool bCheckPageDescOfNextPage = false;

    if ( pFtn )
    {
        //Wenn die Fussnote bereits auf der gleichen Seite/Spalte wie die Referenz
        //steht, ist nix mit zurueckfliessen. Die breaks brauche fuer die
        //Fussnoten nicht geprueft zu werden.

        // --> FME 2004-11-15 #i37084# FindLastCntnt does not necessarily
        // have to have a result != 0
        SwFrm* pRef = 0;
        const bool bEndnote = pFtn->GetAttr()->GetFtn().IsEndNote();
        if( bEndnote && pFtn->IsInSct() )
        {
            SwSectionFrm* pSect = pFtn->FindSctFrm();
            if( pSect->IsEndnAtEnd() )
                pRef = pSect->FindLastCntnt( FINDMODE_LASTCNT );
        }
        if( !pRef )
            pRef = pFtn->GetRef();
        // <--

        OSL_ENSURE( pRef, "MoveBwd: Endnote for an empty section?" );

        if( !bEndnote )
            pOldBoss = pOldBoss->FindFtnBossFrm( sal_True );
        SwFtnBossFrm *pRefBoss = pRef->FindFtnBossFrm( !bEndnote );
        if ( pOldBoss != pRefBoss &&
             // OD 08.11.2002 #104840# - use <SwLayoutFrm::IsBefore(..)>
             ( !bEndnote ||
               pRefBoss->IsBefore( pOldBoss ) )
           )
            pNewUpper = rThis.GetLeaf( MAKEPAGE_FTN, sal_False );
    }
    else if ( IsPageBreak( sal_True ) ) //PageBreak zu beachten?
    {
        //Wenn auf der vorhergehenden Seite kein Frm im Body steht,
        //so ist das Zurueckfliessen trotz Pagebreak sinnvoll
        //(sonst: leere Seite).
        //Natuerlich muessen Leereseiten geflissentlich uebersehen werden!
        const SwFrm *pFlow = &rThis;
        do
        {
            pFlow = pFlow->FindPrev();
        } while ( pFlow &&
                  ( pFlow->FindPageFrm() == pOldPage ||
                    !pFlow->IsInDocBody() ) );
        if ( pFlow )
        {
            long nDiff = pOldPage->GetPhyPageNum() - pFlow->GetPhyPageNum();
            if ( nDiff > 1 )
            {
                if ( ((SwPageFrm*)pOldPage->GetPrev())->IsEmptyPage() )
                    nDiff -= 1;
                if ( nDiff > 1 )
                {
                    pNewUpper = rThis.GetLeaf( MAKEPAGE_NONE, sal_False );
                    // --> OD 2006-05-08 #i53139#
                    // Now <pNewUpper> is a previous layout frame, which contains
                    // content. But the new upper layout frame has to be the next one.
                    // Thus, hack for issue i14206 no longer needed, but fix for issue 114442
                    // --> OD 2006-05-17 #136024# - correct fix for i53139:
                    // Check for wrong page description before using next new upper.
                    // --> OD 2006-06-06 #i66051# - further correction of fix for i53139
                    // Check for correct type of new next upper layout frame
                    // --> OD 2006-06-08 #136538# - another correction of fix for i53139
                    // Assumption, that in all cases <pNewUpper> is a previous
                    // layout frame, which contains content, is wrong.
                    // --> OD 2006-07-05 #136538# - another correction of fix for i53139
                    // Beside type check, check also, if proposed new next upper
                    // frame is inside the same frame types.
                    // --> OD 2007-01-10 #i73194# - and yet another correction
                    // of fix for i53139:
                    // Assure that the new next upper layout frame doesn't
                    // equal the current one.
                    // E.g.: content is on page 3, on page 2 is only a 'ghost'
                    // section and on page 1 is normal content. Method <FindPrev(..)>
                    // will find the last content of page 1, but <GetLeaf(..)>
                    // returns new upper on page 2.
                    if ( pNewUpper->Lower() )
                    {
                        SwLayoutFrm* pNewNextUpper = pNewUpper->GetLeaf( MAKEPAGE_NONE, sal_True );
                        if ( pNewNextUpper &&
                             pNewNextUpper != rThis.GetUpper() &&
                             pNewNextUpper->GetType() == pNewUpper->GetType() &&
                             pNewNextUpper->IsInDocBody() == pNewUpper->IsInDocBody() &&
                             pNewNextUpper->IsInFtn() == pNewUpper->IsInFtn() &&
                             pNewNextUpper->IsInTab() == pNewUpper->IsInTab() &&
                             pNewNextUpper->IsInSct() == pNewUpper->IsInSct() &&
                             !rThis.WrongPageDesc( pNewNextUpper->FindPageFrm() ) )
                        {
                            pNewUpper = pNewNextUpper;
                            bCheckPageDescOfNextPage = true;
                        }
                    }
                    // <--

                    bCheckPageDescs = sal_True;
                }
            }
        }
    }
    else if ( IsColBreak( sal_True ) )
    {
        //Wenn in der vorhergehenden Spalte kein CntntFrm steht, so ist
        //das Zurueckfliessen trotz ColumnBreak sinnvoll
        //(sonst: leere Spalte).
        if( rThis.IsInSct() )
        {
            pNewUpper = rThis.GetLeaf( MAKEPAGE_NONE, sal_False );
            if( pNewUpper && !SwFlowFrm::IsMoveBwdJump() &&
                ( pNewUpper->ContainsCntnt() ||
                  ( ( !pNewUpper->IsColBodyFrm() ||
                      !pNewUpper->GetUpper()->GetPrev() ) &&
                    !pNewUpper->FindSctFrm()->GetPrev() ) ) )
            {
                pNewUpper = 0;
            }
            // --> OD 2006-05-08 #i53139#
            // --> OD 2006-09-11 #i69409# - check <pNewUpper>
            // --> OD 2006-11-02 #i71065# - check <SwFlowFrm::IsMoveBwdJump()>
            else if ( pNewUpper && !SwFlowFrm::IsMoveBwdJump() )
            // <--
            {
                // Now <pNewUpper> is a previous layout frame, which
                // contains content. But the new upper layout frame
                // has to be the next one.
                // --> OD 2006-05-17 #136024# - correct fix for i53139
                // Check for wrong page description before using next new upper.
                // --> OD 2006-06-06 #i66051# - further correction of fix for i53139
                // Check for correct type of new next upper layout frame
                // --> OD 2006-07-05 #136538# - another correction of fix for i53139
                // Beside type check, check also, if proposed new next upper
                // frame is inside the same frame types.
                SwLayoutFrm* pNewNextUpper = pNewUpper->GetLeaf( MAKEPAGE_NOSECTION, sal_True );
                if ( pNewNextUpper &&
                     pNewNextUpper->GetType() == pNewUpper->GetType() &&
                     pNewNextUpper->IsInDocBody() == pNewUpper->IsInDocBody() &&
                     pNewNextUpper->IsInFtn() == pNewUpper->IsInFtn() &&
                     pNewNextUpper->IsInTab() == pNewUpper->IsInTab() &&
                     pNewNextUpper->IsInSct() == pNewUpper->IsInSct() &&
                     !rThis.WrongPageDesc( pNewNextUpper->FindPageFrm() ) )
                {
                    pNewUpper = pNewNextUpper;
                }
            }
            // <--
        }
        else
        {
            const SwFrm *pCol = rThis.FindColFrm();
            sal_Bool bGoOn = sal_True;
            sal_Bool bJump = sal_False;
            do
            {
                if ( pCol->GetPrev() )
                    pCol = pCol->GetPrev();
                else
                {
                    bGoOn = sal_False;
                    pCol = rThis.GetLeaf( MAKEPAGE_NONE, sal_False );
                }
                if ( pCol )
                {
                    // ColumnFrms jetzt mit BodyFrm
                    SwLayoutFrm* pColBody = pCol->IsColumnFrm() ?
                        (SwLayoutFrm*)((SwLayoutFrm*)pCol)->Lower() :
                        (SwLayoutFrm*)pCol;
                    if ( pColBody->ContainsCntnt() )
                    {
                        bGoOn = sal_False; // Hier gibt's Inhalt, wir akzeptieren diese
                        // nur, wenn GetLeaf() das MoveBwdJump-Flag gesetzt hat.
                        if( SwFlowFrm::IsMoveBwdJump() )
                        {
                            pNewUpper = pColBody;
                            // --> OD 2006-05-08 #i53139#
                            // Now <pNewUpper> is a previous layout frame, which
                            // contains content. But the new upper layout frame
                            // has to be the next one.
                            // --> OD 2006-05-17 #136024# - correct fix for i53139
                            // Check for wrong page description before using next new upper.
                            // --> OD 2006-06-06 #i66051# - further correction of fix for i53139
                            // Check for correct type of new next upper layout frame
                            // --> OD 2006-07-05 #136538# - another correction of fix for i53139
                            // Beside type check, check also, if proposed new next upper
                            // frame is inside the same frame types.
                            // --> OD 2006-11-02 #i71065#
                            // Check that the proposed new next upper layout
                            // frame isn't the current one.
                            SwLayoutFrm* pNewNextUpper = pNewUpper->GetLeaf( MAKEPAGE_NONE, sal_True );
                            if ( pNewNextUpper &&
                                 pNewNextUpper != rThis.GetUpper() &&
                                 pNewNextUpper->GetType() == pNewUpper->GetType() &&
                                 pNewNextUpper->IsInDocBody() == pNewUpper->IsInDocBody() &&
                                 pNewNextUpper->IsInFtn() == pNewUpper->IsInFtn() &&
                                 pNewNextUpper->IsInTab() == pNewUpper->IsInTab() &&
                                 pNewNextUpper->IsInSct() == pNewUpper->IsInSct() &&
                                 !rThis.WrongPageDesc( pNewNextUpper->FindPageFrm() ) )
                            {
                                pNewUpper = pNewNextUpper;
                            }
                            // <--
                        }
                    }
                    else
                    {
                        if( pNewUpper ) // Wir hatten schon eine leere Spalte, haben
                            bJump = sal_True;   // also eine uebersprungen
                        pNewUpper = pColBody;  // Diese leere Spalte kommt in Frage,
                                               // trotzdem weitersuchen
                    }
                }
            } while( bGoOn );
            if( bJump )
                SwFlowFrm::SetMoveBwdJump( sal_True );
        }
    }
    else //Keine Breaks also kann ich zurueckfliessen
        pNewUpper = rThis.GetLeaf( MAKEPAGE_NONE, sal_False );

    // --> OD 2004-06-23 #i27801# - no move backward of 'master' text frame,
    // if - due to its object positioning - it isn't allowed to be on the new page frame
    // --> OD 2005-03-07 #i44049# - add another condition for not moving backward:
    // If one of its objects has restarted the layout process, moving backward
    // isn't sensible either.
    // --> OD 2005-04-19 #i47697# - refine condition made for issue i44049:
    // - allow move backward as long as the anchored object is only temporarily
    //   positions considering its wrapping style.
    if ( pNewUpper &&
         rThis.IsTxtFrm() && !IsFollow() )
    {
        sal_uInt32 nToPageNum( 0L );
        const bool bMoveFwdByObjPos = SwLayouter::FrmMovedFwdByObjPos(
                                                *(pOldPage->GetFmt()->GetDoc()),
                                                static_cast<SwTxtFrm&>(rThis),
                                                nToPageNum );
        if ( bMoveFwdByObjPos &&
             pNewUpper->FindPageFrm()->GetPhyPageNum() < nToPageNum )
        {
            pNewUpper = 0;
        }
        // --> OD 2005-03-07 #i44049# - check, if one of its anchored objects
        // has restarted the layout process.
        else if ( rThis.GetDrawObjs() )
        {
            sal_uInt32 i = 0;
            for ( ; i < rThis.GetDrawObjs()->Count(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*rThis.GetDrawObjs())[i];
                // --> OD 2005-04-19 #i47697# - refine condition - see above
                if ( pAnchoredObj->RestartLayoutProcess() &&
                     !pAnchoredObj->IsTmpConsiderWrapInfluence() )
                // <--
                {
                    pNewUpper = 0;
                    break;
                }
            }
        }
        // <--
    }
    // <--

    //Fuer Follows ist das zurueckfliessen nur dann erlaubt wenn in der
    //neuen Umgebung kein Nachbar existiert (denn dieses waere der Master).
    //(6677)Wenn allerdings leere Blaetter uebersprungen wurden wird doch gemoved.
    if ( pNewUpper && IsFollow() && pNewUpper->Lower() )
    {
        // --> OD 2007-09-05 #i79774#, #b6596954#
        // neglect empty sections in proposed new upper frame
        bool bProposedNewUpperContainsOnlyEmptySections( true );
        {
            const SwFrm* pLower( pNewUpper->Lower() );
            while ( pLower )
            {
                if ( pLower->IsSctFrm() &&
                     !dynamic_cast<const SwSectionFrm*>(pLower)->GetSection() )
                {
                    pLower = pLower->GetNext();
                    continue;
                }
                else
                {
                    bProposedNewUpperContainsOnlyEmptySections = false;
                    break;
                }
            }
        }
        if ( !bProposedNewUpperContainsOnlyEmptySections )
        {
            if ( SwFlowFrm::IsMoveBwdJump() )
            {
                //Nicht hinter den Master sondern in das naechstfolgende leere
                //Blatt moven.
                SwFrm *pFrm = pNewUpper->Lower();
                while ( pFrm->GetNext() )
                    pFrm = pFrm->GetNext();
                pNewUpper = pFrm->GetLeaf( MAKEPAGE_INSERT, sal_True );
                if( pNewUpper == rThis.GetUpper() ) //Landen wir wieder an der gleichen Stelle?
                    pNewUpper = NULL;           //dann eruebrigt sich das Moven
            }
            else
                pNewUpper = 0;
        }
        // <--
    }
    if ( pNewUpper && !ShouldBwdMoved( pNewUpper, sal_True, rbReformat ) )
    {
        if( !pNewUpper->Lower() )
        {
            if( pNewUpper->IsFtnContFrm() )
            {
                pNewUpper->Cut();
                delete pNewUpper;
            }
            else
            {
                SwSectionFrm* pSectFrm = pNewUpper->FindSctFrm();
                // --> OD 2006-01-04 #126020# - adjust check for empty section
                // --> OD 2006-02-01 #130797# - correct fix #126020#
                if ( pSectFrm && !pSectFrm->IsColLocked() &&
                     !pSectFrm->ContainsCntnt() && !pSectFrm->ContainsAny( true ) )
                // <--
                {
                    pSectFrm->DelEmpty( sal_True );
                    delete pSectFrm;
                    rThis.bValidPos = sal_True;
                }
            }
        }
        pNewUpper = 0;
    }

    // OD 2004-05-26 #i21478# - don't move backward, if flow frame wants to
    // keep with next frame and next frame is locked.
    // --> OD 2004-12-08 #i38232# - If next frame is a table, do *not* check,
    // if it's locked.
    if ( pNewUpper && !IsFollow() &&
         rThis.GetAttrSet()->GetKeep().GetValue() && rThis.GetIndNext() )
    {
        SwFrm* pIndNext = rThis.GetIndNext();
        // --> OD 2004-12-08 #i38232#
        if ( !pIndNext->IsTabFrm() )
        {
            // get first content of section, while empty sections are skipped
            while ( pIndNext && pIndNext->IsSctFrm() )
            {
                if( static_cast<SwSectionFrm*>(pIndNext)->GetSection() )
                {
                    SwFrm* pTmp = static_cast<SwSectionFrm*>(pIndNext)->ContainsAny();
                    if ( pTmp )
                    {
                        pIndNext = pTmp;
                        break;
                    }
                }
                pIndNext = pIndNext->GetIndNext();
            }
            OSL_ENSURE( !pIndNext || pIndNext->ISA(SwTxtFrm),
                    "<SwFlowFrm::MovedBwd(..)> - incorrect next found." );
            if ( pIndNext && pIndNext->IsFlowFrm() &&
                 SwFlowFrm::CastFlowFrm(pIndNext)->IsJoinLocked() )
            {
                pNewUpper = 0L;
            }
        }
        // <--
    }

    // --> OD 2006-05-10 #i65250#
    // layout loop control for flowing content again and again moving
    // backward under the same layout condition.
    if ( pNewUpper && !IsFollow() &&
         pNewUpper != rThis.GetUpper() &&
         SwLayouter::MoveBwdSuppressed( *(pOldPage->GetFmt()->GetDoc()),
                                        *this, *pNewUpper ) )
    {
        SwLayoutFrm* pNextNewUpper = pNewUpper->GetLeaf(
                                    ( !rThis.IsSctFrm() && rThis.IsInSct() )
                                    ? MAKEPAGE_NOSECTION
                                    : MAKEPAGE_NONE,
                                    sal_True );
        // --> OD 2007-01-10 #i73194# - make code robust
        OSL_ENSURE( pNextNewUpper, "<SwFlowFrm::MoveBwd(..)> - missing next new upper" );
        if ( pNextNewUpper &&
             ( pNextNewUpper == rThis.GetUpper() ||
               pNextNewUpper->GetType() != rThis.GetUpper()->GetType() ) )
        // <--
        {
            pNewUpper = 0L;
#if OSL_DEBUG_LEVEL > 1
            OSL_ENSURE( false,
                    "<SwFlowFrm::MoveBwd(..)> - layout loop control for layout action <Move Backward> applied!" );
#endif
        }
    }
    // <--

    OSL_ENSURE( pNewUpper != rThis.GetUpper(),
            "<SwFlowFrm::MoveBwd(..)> - moving backward to the current upper frame!? -> Please inform OD." );
    if ( pNewUpper )
    {
        PROTOCOL_ENTER( &rThis, PROT_MOVE_BWD, 0, 0 );
        if ( pNewUpper->IsFtnContFrm() )
        {
            //Kann sein, dass ich einen Container bekam.
            SwFtnFrm *pOld = rThis.FindFtnFrm();
            SwFtnFrm *pNew = new SwFtnFrm( pOld->GetFmt(),
                                           pOld->GetRef(), pOld->GetAttr() );
            if ( pOld->GetMaster() )
            {
                pNew->SetMaster( pOld->GetMaster() );
                pOld->GetMaster()->SetFollow( pNew );
            }
            pNew->SetFollow( pOld );
            pOld->SetMaster( pNew );
            pNew->Paste( pNewUpper );
            pNewUpper = pNew;
        }
        if( pNewUpper->IsFtnFrm() && rThis.IsInSct() )
        {
            SwSectionFrm* pSct = rThis.FindSctFrm();
            //Wenn wir in einem Bereich in einer Fussnote stecken, muss im
            //neuen Upper ggf. ein SwSectionFrm angelegt werden
            if( pSct->IsInFtn() )
            {
                SwFrm* pTmp = pNewUpper->Lower();
                if( pTmp )
                {
                    while( pTmp->GetNext() )
                        pTmp = pTmp->GetNext();
                    if( !pTmp->IsSctFrm() ||
                        ((SwSectionFrm*)pTmp)->GetFollow() != pSct )
                        pTmp = NULL;
                }
                if( pTmp )
                    pNewUpper = (SwSectionFrm*)pTmp;
                else
                {
                    pSct = new SwSectionFrm( *pSct, sal_True );
                    pSct->Paste( pNewUpper );
                    pSct->Init();
                    pNewUpper = pSct;
                    pSct->SimpleFormat();
                }
            }
        }
        sal_Bool bUnlock = sal_False;
        sal_Bool bFollow = sal_False;
        //Section locken, sonst kann sie bei Fluss des einzigen Cntnt etwa
        //von zweiter in die erste Spalte zerstoert werden.
        SwSectionFrm* pSect = pNewUpper->FindSctFrm();
        if( pSect )
        {
            bUnlock = !pSect->IsColLocked();
            pSect->ColLock();
            bFollow = pSect->HasFollow();
        }
        pNewUpper->Calc();
        rThis.Cut();
        // --> OD 2005-02-23 #b6229852#
        // optimization: format section, if its size is invalidated and if it's
        // the new parent of moved backward frame.
        bool bFormatSect( false );
        // <--
        if( bUnlock )
        {
            pSect->ColUnlock();
            if( pSect->HasFollow() != bFollow )
            {
                pSect->InvalidateSize();
                // --> OD 2005-02-23 #b6229852# - optimization
                if ( pSect == pNewUpper )
                    bFormatSect = true;
                // <--
            }
        }

        rThis.Paste( pNewUpper );
        // --> OD 2005-02-23 #b6229852# - optimization
        if ( bFormatSect )
            pSect->Calc();
        // <--

        SwPageFrm *pNewPage = rThis.FindPageFrm();
        if( pNewPage != pOldPage )
        {
            rThis.Prepare( PREP_BOSS_CHGD, (const void*)pOldPage, sal_False );
            ViewShell *pSh = rThis.GetShell();
            if ( pSh && !pSh->Imp()->IsUpdateExpFlds() )
                pSh->GetDoc()->SetNewFldLst(true);  //Wird von CalcLayout() hinterher eledigt!

            pNewPage->InvalidateSpelling();
            pNewPage->InvalidateSmartTags();    // SMARTTAGS
            pNewPage->InvalidateAutoCompleteWords();
            pNewPage->InvalidateWordCount();

            // OD 30.10.2002 #97265# - no <CheckPageDesc(..)> in online layout
            if ( !pNewPage->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
            {
                if ( bCheckPageDescs && pNewPage->GetNext() )
                {
                    SwPageFrm* pStartPage = bCheckPageDescOfNextPage ?
                                            pNewPage :
                                            (SwPageFrm*)pNewPage->GetNext();
                    SwFrm::CheckPageDescs( pStartPage, sal_False);
                }
                else if ( rThis.GetAttrSet()->GetPageDesc().GetPageDesc() )
                {
                    //Erste Seite wird etwa durch Ausblenden eines Bereiches leer
                    SwFrm::CheckPageDescs( (SwPageFrm*)pNewPage, sal_False);
                }
            }
        }
    }
    return pNewUpper != 0;
}

/*************************************************************************
|*
|*  SwFlowFrm::CastFlowFrm
|*
|*************************************************************************/

SwFlowFrm *SwFlowFrm::CastFlowFrm( SwFrm *pFrm )
{
    if ( pFrm->IsCntntFrm() )
        return (SwCntntFrm*)pFrm;
    if ( pFrm->IsTabFrm() )
        return (SwTabFrm*)pFrm;
    if ( pFrm->IsSctFrm() )
        return (SwSectionFrm*)pFrm;
    return 0;
}

const SwFlowFrm *SwFlowFrm::CastFlowFrm( const SwFrm *pFrm )
{
    if ( pFrm->IsCntntFrm() )
        return (SwCntntFrm*)pFrm;
    if ( pFrm->IsTabFrm() )
        return (SwTabFrm*)pFrm;
    if ( pFrm->IsSctFrm() )
        return (SwSectionFrm*)pFrm;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
