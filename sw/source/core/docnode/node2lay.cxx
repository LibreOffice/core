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


#include <switerator.hxx>
#include <calbck.hxx>
#include <node.hxx>
#include <ndindex.hxx>
#include <swtable.hxx>
#include <ftnfrm.hxx>
#include <sectfrm.hxx>
#include "frmfmt.hxx"
#include "cntfrm.hxx"
#include "tabfrm.hxx"
#include "frmtool.hxx"
#include "section.hxx"
#include "node2lay.hxx"

/* --------------------------------------------------
 * Die SwNode2LayImpl-Klasse erledigt die eigentliche Arbeit,
 * die SwNode2Layout-Klasse ist nur die der Oefffentlichkeit bekannte Schnittstelle
 * --------------------------------------------------*/
class SwNode2LayImpl
{
    SwIterator<SwFrm,SwModify>* pIter;
    SwModify* pMod;
    std::vector<SwFrm*>* pUpperFrms;// Zum Einsammeln der Upper
    sal_uLong nIndex;        // Der Index des einzufuegenden Nodes
    sal_Bool bMaster    : 1; // sal_True => nur Master , sal_False => nur Frames ohne Follow
    sal_Bool bInit      : 1; // Ist am SwClient bereits ein First()-Aufruf erfolgt?
public:
    SwNode2LayImpl( const SwNode& rNode, sal_uLong nIdx, sal_Bool bSearch );
    ~SwNode2LayImpl() { delete pIter; delete pUpperFrms; }
    SwFrm* NextFrm(); // liefert den naechsten "sinnvollen" Frame
    SwLayoutFrm* UpperFrm( SwFrm* &rpFrm, const SwNode &rNode );
    void SaveUpperFrms(); // Speichert (und lockt ggf.) die pUpper
    // Fuegt unter jeden pUpper des Arrays einen Frame ein.
    void RestoreUpperFrms( SwNodes& rNds, sal_uLong nStt, sal_uLong nEnd );

    SwFrm* GetFrm( const Point* pDocPos = 0,
                    const SwPosition *pPos = 0,
                    const sal_Bool bCalcFrm = sal_True ) const;
};

/* --------------------------------------------------
 * Hauptaufgabe des Ctor: Das richtige SwModify zu ermitteln,
 * ueber das iteriert wird.
 * Uebergibt man bSearch == sal_True, so wird der naechste Cntnt- oder TableNode
 * gesucht, der Frames besitzt ( zum Einsammeln der pUpper ), ansonsten wird
 * erwartet, das rNode bereits auf einem solchen Cntnt- oder TableNode sitzt,
 * vor oder hinter den eingefuegt werden soll.
 * --------------------------------------------------*/

SwNode* GoNextWithFrm(const SwNodes& rNodes, SwNodeIndex *pIdx)
{
    if( pIdx->GetIndex() >= rNodes.Count() - 1 )
        return 0;

    SwNodeIndex aTmp(*pIdx, +1);
    SwNode* pNd = 0;
    while( aTmp < rNodes.Count()-1 )
    {
        pNd = &aTmp.GetNode();
        bool bFound = false;
        if ( pNd->IsCntntNode() )
            bFound = ( SwIterator<SwFrm,SwCntntNode>::FirstElement(*(SwCntntNode*)pNd) != 0);
        else if ( pNd->IsTableNode() )
            bFound = ( SwIterator<SwFrm,SwFmt>::FirstElement(*((SwTableNode*)pNd)->GetTable().GetFrmFmt()) != 0 );
        else if( pNd->IsEndNode() && !pNd->StartOfSectionNode()->IsSectionNode() )
        {
            pNd = 0;
            break;
        }
        if ( bFound )
                break;
        aTmp++;
    }

    if( aTmp == rNodes.Count()-1 )
        pNd = 0;
    else if( pNd )
        (*pIdx) = aTmp;
    return pNd;
}

SwNode* GoPreviousWithFrm(SwNodeIndex *pIdx)
{
    if( !pIdx->GetIndex() )
        return 0;

    SwNodeIndex aTmp( *pIdx, -1 );
    SwNode* pNd(0);
    while( aTmp.GetIndex() )
    {
        pNd = &aTmp.GetNode();
        bool bFound = false;
        if ( pNd->IsCntntNode() )
            bFound = ( SwIterator<SwFrm,SwCntntNode>::FirstElement(*(SwCntntNode*)pNd) != 0);
        else if ( pNd->IsTableNode() )
            bFound = ( SwIterator<SwFrm,SwFmt>::FirstElement(*((SwTableNode*)pNd)->GetTable().GetFrmFmt()) != 0 );
        else if( pNd->IsStartNode() && !pNd->IsSectionNode() )
        {
            pNd = 0;
            break;
        }
        if ( bFound )
                break;
        aTmp--;
    }

    if( !aTmp.GetIndex() )
        pNd = 0;
    else if( pNd )
        (*pIdx) = aTmp;
    return pNd;
}


SwNode2LayImpl::SwNode2LayImpl( const SwNode& rNode, sal_uLong nIdx, sal_Bool bSearch )
    : pUpperFrms( NULL ), nIndex( nIdx ), bInit( sal_False )
{
    const SwNode* pNd;
    if( bSearch || rNode.IsSectionNode() )
    {
        // Suche den naechsten Cntnt/TblNode, der einen Frame besitzt,
        // damit wir uns vor/hinter ihn haengen koennen
        if( !bSearch && rNode.GetIndex() < nIndex )
        {
            SwNodeIndex aTmp( *rNode.EndOfSectionNode(), +1 );
            pNd = GoPreviousWithFrm( &aTmp );
            if( !bSearch && pNd && rNode.GetIndex() > pNd->GetIndex() )
                pNd = NULL; // Nicht ueber den Bereich hinausschiessen
            bMaster = sal_False;
        }
        else
        {
            SwNodeIndex aTmp( rNode, -1 );
            pNd = GoNextWithFrm( rNode.GetNodes(), &aTmp );
            bMaster = sal_True;
            if( !bSearch && pNd && rNode.EndOfSectionIndex() < pNd->GetIndex() )
                pNd = NULL; // Nicht ueber den Bereich hinausschiessen
        }
    }
    else
    {
        pNd = &rNode;
        bMaster = nIndex < rNode.GetIndex();
    }
    if( pNd )
    {
        if( pNd->IsCntntNode() )
            pMod = (SwModify*)pNd->GetCntntNode();
        else
        {
            OSL_ENSURE( pNd->IsTableNode(), "For Tablenodes only" );
            pMod = pNd->GetTableNode()->GetTable().GetFrmFmt();
        }
        pIter = new SwIterator<SwFrm,SwModify>( *pMod );
    }
    else
    {
        pIter = NULL;
        pMod = 0;
    }
}

/* --------------------------------------------------
 * SwNode2LayImpl::NextFrm() liefert den naechsten "sinnvollen" Frame,
 * beim ersten Aufruf wird am eigentlichen Iterator ein First gerufen,
 * danach die Next-Methode. Das Ergebnis wird auf Brauchbarkeit untersucht,
 * so werden keine Follows akzeptiert, ein Master wird beim Einsammeln der
 * pUpper und beim Einfuegen vor ihm akzeptiert. Beim Einfuegen dahinter
 * wird vom Master ausgehend der letzte Follow gesucht und zurueckgegeben.
 * Wenn der Frame innerhalb eines SectionFrms liegt, wird noch festgestellt,
 * ob statt des Frames der SectionFrm der geeignete Rueckgabewert ist, dies
 * ist der Fall, wenn der neu einzufuegende Node ausserhalb des Bereichs liegt.
 * --------------------------------------------------*/
SwFrm* SwNode2LayImpl::NextFrm()
{
    SwFrm* pRet;
    if( !pIter )
        return NULL;
    if( !bInit )
    {
         pRet = pIter->First();
         bInit = sal_True;
    }
    else
        pRet = pIter->Next();
    while( pRet )
    {
        SwFlowFrm* pFlow = SwFlowFrm::CastFlowFrm( pRet );
        OSL_ENSURE( pFlow, "Cntnt or Table expected?!" );
        // Follows sind fluechtige Gestalten, deshalb werden sie ignoriert.
        // Auch wenn wir hinter dem Frame eingefuegt werden sollen, nehmen wir
        // zunaechst den Master, hangeln uns dann aber zum letzten Follow durch.
        if( !pFlow->IsFollow() )
        {
            if( !bMaster )
            {
                while( pFlow->HasFollow() )
                    pFlow = pFlow->GetFollow();
                pRet = pFlow->GetFrm();
            }
            if( pRet->IsInSct() )
            {
                SwSectionFrm* pSct = pRet->FindSctFrm();
                // Vorsicht: Wenn wir in einer Fussnote sind, so kann diese
                // Layoutmaessig in einem spaltigen Bereich liegen, obwohl
                // sie nodemaessig ausserhalb liegt. Deshalb muss bei Fussnoten
                // ueberprueft werden, ob auch der SectionFrm in der Fussnote
                // und nicht ausserhalb liegt.
                if( !pRet->IsInFtn() || pSct->IsInFtn() )
                {
                    OSL_ENSURE( pSct && pSct->GetSection(), "Where's my section?" );
                    SwSectionNode* pNd = pSct->GetSection()->GetFmt()->GetSectionNode();
                    OSL_ENSURE( pNd, "Lost SectionNode" );
                    // Wenn der erhaltene Frame in einem Bereichsframe steht,
                    // dessen Bereich den Ausgangsnode nicht umfasst, so kehren
                    // wir mit dem SectionFrm zurueck, sonst mit dem Cntnt/TabFrm
                    if( bMaster )
                    {
                        if( pNd->GetIndex() >= nIndex )
                            pRet = pSct;
                    }
                    else if( pNd->EndOfSectionIndex() < nIndex )
                        pRet = pSct;
                }
            }
            return pRet;
        }
        pRet = pIter->Next();
    }
    return NULL;
}

void SwNode2LayImpl::SaveUpperFrms()
{
    pUpperFrms = new std::vector<SwFrm*>;
    SwFrm* pFrm;
    while( 0 != (pFrm = NextFrm()) )
    {
        SwFrm* pPrv = pFrm->GetPrev();
        pFrm = pFrm->GetUpper();
        if( pFrm )
        {
            if( pFrm->IsFtnFrm() )
                ((SwFtnFrm*)pFrm)->ColLock();
            else if( pFrm->IsInSct() )
                pFrm->FindSctFrm()->ColLock();
            if( pPrv && pPrv->IsSctFrm() )
                ((SwSectionFrm*)pPrv)->LockJoin();
            pUpperFrms->push_back( pPrv );
            pUpperFrms->push_back( pFrm );
        }
    }
    delete pIter;
    pIter = NULL;
    pMod = 0;
}

SwLayoutFrm* SwNode2LayImpl::UpperFrm( SwFrm* &rpFrm, const SwNode &rNode )
{
    rpFrm = NextFrm();
    if( !rpFrm )
        return NULL;
    SwLayoutFrm* pUpper = rpFrm->GetUpper();
    if( rpFrm->IsSctFrm() )
    {
        const SwNode* pNode = rNode.StartOfSectionNode();
        if( pNode->IsSectionNode() )
        {
            SwFrm* pFrm = bMaster ? rpFrm->FindPrev() : rpFrm->FindNext();
            if( pFrm && pFrm->IsSctFrm() )
            {
                // pFrm could be a "dummy"-section
                if( ((SwSectionFrm*)pFrm)->GetSection() &&
                    (&((SwSectionNode*)pNode)->GetSection() ==
                     ((SwSectionFrm*)pFrm)->GetSection()) )
                {
                    // #i22922# - consider columned sections
                    // 'Go down' the section frame as long as the layout frame
                    // is found, which would contain content.
                    while ( pFrm->IsLayoutFrm() &&
                            static_cast<SwLayoutFrm*>(pFrm)->Lower() &&
                            !static_cast<SwLayoutFrm*>(pFrm)->Lower()->IsFlowFrm() &&
                            static_cast<SwLayoutFrm*>(pFrm)->Lower()->IsLayoutFrm() )
                    {
                        pFrm = static_cast<SwLayoutFrm*>(pFrm)->Lower();
                    }
                    OSL_ENSURE( pFrm->IsLayoutFrm(),
                            "<SwNode2LayImpl::UpperFrm(..)> - expected upper frame isn't a layout frame." );
                    rpFrm = bMaster ? NULL
                                    : static_cast<SwLayoutFrm*>(pFrm)->Lower();
                    OSL_ENSURE( !rpFrm || rpFrm->IsFlowFrm(),
                            "<SwNode2LayImpl::UpperFrm(..)> - expected sibling isn't a flow frame." );
                    return static_cast<SwLayoutFrm*>(pFrm);
                }

                pUpper = new SwSectionFrm(((SwSectionNode*)pNode)->GetSection(), rpFrm);
                pUpper->Paste( rpFrm->GetUpper(),
                               bMaster ? rpFrm : rpFrm->GetNext() );
                static_cast<SwSectionFrm*>(pUpper)->Init();
                rpFrm = NULL;
                // 'Go down' the section frame as long as the layout frame
                // is found, which would contain content.
                while ( pUpper->Lower() &&
                        !pUpper->Lower()->IsFlowFrm() &&
                        pUpper->Lower()->IsLayoutFrm() )
                {
                    pUpper = static_cast<SwLayoutFrm*>(pUpper->Lower());
                }
                return pUpper;
            }
        }
    };
    if( !bMaster )
        rpFrm = rpFrm->GetNext();
    return pUpper;
}

void SwNode2LayImpl::RestoreUpperFrms( SwNodes& rNds, sal_uLong nStt, sal_uLong nEnd )
{
    OSL_ENSURE( pUpperFrms, "RestoreUpper without SaveUpper?" );
    SwNode* pNd;
    SwDoc *pDoc = rNds.GetDoc();
    sal_Bool bFirst = sal_True;
    for( ; nStt < nEnd; ++nStt )
    {
        SwFrm* pNew = 0;
        SwFrm* pNxt;
        SwLayoutFrm* pUp;
        if( (pNd = rNds[nStt])->IsCntntNode() )
            for( sal_uInt16 n = 0; n < pUpperFrms->size(); )
            {
                pNxt = (*pUpperFrms)[n++];
                if( bFirst && pNxt && pNxt->IsSctFrm() )
                    ((SwSectionFrm*)pNxt)->UnlockJoin();
                pUp = (SwLayoutFrm*)(*pUpperFrms)[n++];
                if( pNxt )
                    pNxt = pNxt->GetNext();
                else
                    pNxt = pUp->Lower();
                pNew = ((SwCntntNode*)pNd)->MakeFrm( pUp );
                pNew->Paste( pUp, pNxt );
                (*pUpperFrms)[n-2] = pNew;
            }
        else if( pNd->IsTableNode() )
            for( sal_uInt16 x = 0; x < pUpperFrms->size(); )
            {
                pNxt = (*pUpperFrms)[x++];
                if( bFirst && pNxt && pNxt->IsSctFrm() )
                    ((SwSectionFrm*)pNxt)->UnlockJoin();
                pUp = (SwLayoutFrm*)(*pUpperFrms)[x++];
                if( pNxt )
                    pNxt = pNxt->GetNext();
                else
                    pNxt = pUp->Lower();
                pNew = ((SwTableNode*)pNd)->MakeFrm( pUp );
                OSL_ENSURE( pNew->IsTabFrm(), "Table exspected" );
                pNew->Paste( pUp, pNxt );
                ((SwTabFrm*)pNew)->RegistFlys();
                (*pUpperFrms)[x-2] = pNew;
            }
        else if( pNd->IsSectionNode() )
        {
            nStt = pNd->EndOfSectionIndex();
            for( sal_uInt16 x = 0; x < pUpperFrms->size(); )
            {
                pNxt = (*pUpperFrms)[x++];
                if( bFirst && pNxt && pNxt->IsSctFrm() )
                    ((SwSectionFrm*)pNxt)->UnlockJoin();
                pUp = (SwLayoutFrm*)(*pUpperFrms)[x++];
                OSL_ENSURE( pUp->GetUpper() || pUp->IsFlyFrm(), "Lost Upper" );
                ::_InsertCnt( pUp, pDoc, pNd->GetIndex(), sal_False, nStt+1, pNxt );
                pNxt = pUp->GetLastLower();
                (*pUpperFrms)[x-2] = pNxt;
            }
        }
        bFirst = sal_False;
    }
    for( sal_uInt16 x = 0; x < pUpperFrms->size(); ++x )
    {
        SwFrm* pTmp = (*pUpperFrms)[++x];
        if( pTmp->IsFtnFrm() )
            ((SwFtnFrm*)pTmp)->ColUnlock();
        else if ( pTmp->IsInSct() )
        {
            SwSectionFrm* pSctFrm = pTmp->FindSctFrm();
            pSctFrm->ColUnlock();
            // #i18103# - invalidate size of section in order to
            // assure, that the section is formatted, unless it was 'Collocked'
            // from its 'collection' until its 'restoration'.
            pSctFrm->_InvalidateSize();
        }
    }
}

SwFrm* SwNode2LayImpl::GetFrm( const Point* pDocPos,
                                const SwPosition *pPos,
                                const sal_Bool bCalcFrm ) const
{
    // mba: test if change of member pIter -> pMod broke anything
    return pMod ? ::GetFrmOfModify( 0, *pMod, USHRT_MAX, pDocPos, pPos, bCalcFrm ) : 0;
}

SwNode2Layout::SwNode2Layout( const SwNode& rNd, sal_uLong nIdx )
{
    pImpl = new SwNode2LayImpl( rNd, nIdx, sal_False );
}

SwNode2Layout::SwNode2Layout( const SwNode& rNd )
{
    pImpl = new SwNode2LayImpl( rNd, rNd.GetIndex(), sal_True );
    pImpl->SaveUpperFrms();
}

void SwNode2Layout::RestoreUpperFrms( SwNodes& rNds, sal_uLong nStt, sal_uLong nEnd )
{
    OSL_ENSURE( pImpl, "RestoreUpperFrms without SaveUpperFrms" );
    pImpl->RestoreUpperFrms( rNds, nStt, nEnd );
}

SwFrm* SwNode2Layout::NextFrm()
{
    return pImpl->NextFrm();
}

SwLayoutFrm* SwNode2Layout::UpperFrm( SwFrm* &rpFrm, const SwNode &rNode )
{
    return pImpl->UpperFrm( rpFrm, rNode );
}

SwNode2Layout::~SwNode2Layout()
{
    delete pImpl;
}

SwFrm* SwNode2Layout::GetFrm( const Point* pDocPos,
                                const SwPosition *pPos,
                                const sal_Bool bCalcFrm ) const
{
    return pImpl->GetFrm( pDocPos, pPos, bCalcFrm );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
