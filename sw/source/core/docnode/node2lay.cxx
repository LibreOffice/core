/*************************************************************************
 *
 *  $RCSfile: node2lay.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:17 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _CALBCK_HXX
#include <calbck.hxx>   // SwClientIter
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _FTNFRM_HXX
#include <ftnfrm.hxx>
#endif
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>
#endif
#include "frmfmt.hxx"
#include "cntfrm.hxx"
#include "tabfrm.hxx"
#include "frmtool.hxx"
#include "section.hxx"
#include "node2lay.hxx"


/* -----------------25.02.99 10:31-------------------
 * Die SwNode2LayImpl-Klasse erledigt die eigentliche Arbeit,
 * die SwNode2Layout-Klasse ist nur die der Oefffentlichkeit bekannte Schnittstelle
 * --------------------------------------------------*/
class SwNode2LayImpl
{
    SwClientIter *pIter; // Der eigentliche Iterator
    SvPtrarr *pUpperFrms;// Zum Einsammeln der Upper
    ULONG nIndex;        // Der Index des einzufuegenden Nodes
    BOOL bMaster    : 1; // TRUE => nur Master , FALSE => nur Frames ohne Follow
    BOOL bInit      : 1; // Ist am SwClient bereits ein First()-Aufruf erfolgt?
public:
    SwNode2LayImpl( const SwNode& rNode, ULONG nIdx, BOOL bSearch );
    ~SwNode2LayImpl() { delete pIter; delete pUpperFrms; }
    SwFrm* NextFrm(); // liefert den naechsten "sinnvollen" Frame
    SwLayoutFrm* UpperFrm( SwFrm* &rpFrm, const SwNode &rNode );
    void SaveUpperFrms(); // Speichert (und lockt ggf.) die pUpper
    // Fuegt unter jeden pUpper des Arrays einen Frame ein.
    void RestoreUpperFrms( SwNodes& rNds, ULONG nStt, ULONG nEnd );

    SwFrm* GetFrm( const Point* pDocPos = 0,
                    const SwPosition *pPos = 0,
                    const BOOL bCalcFrm = TRUE ) const;
};

/* -----------------25.02.99 10:38-------------------
 * Hauptaufgabe des Ctor: Das richtige SwModify zu ermitteln,
 * ueber das iteriert wird.
 * Uebergibt man bSearch == TRUE, so wird der naechste Cntnt- oder TableNode
 * gesucht, der Frames besitzt ( zum Einsammeln der pUpper ), ansonsten wird
 * erwartet, das rNode bereits auf einem solchen Cntnt- oder TableNode sitzt,
 * vor oder hinter den eingefuegt werden soll.
 * --------------------------------------------------*/

SwNode2LayImpl::SwNode2LayImpl( const SwNode& rNode, ULONG nIdx, BOOL bSearch )
    : pUpperFrms( NULL ), nIndex( nIdx ), bInit( FALSE )
{
    const SwNode* pNd;
    if( bSearch || rNode.IsSectionNode() )
    {
        // Suche den naechsten Cntnt/TblNode, der einen Frame besitzt,
        // damit wir uns vor/hinter ihn haengen koennen
        if( !bSearch && rNode.GetIndex() < nIndex )
        {
            SwNodeIndex aTmp( *rNode.EndOfSectionNode(), +1 );
            pNd = rNode.GetNodes().GoPreviousWithFrm( &aTmp );
            if( !bSearch && pNd && rNode.GetIndex() > pNd->GetIndex() )
                pNd = NULL; // Nicht ueber den Bereich hinausschiessen
            bMaster = FALSE;
        }
        else
        {
            SwNodeIndex aTmp( rNode, -1 );
            pNd = rNode.GetNodes().GoNextWithFrm( &aTmp );
            bMaster = TRUE;
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
        SwModify *pMod;
        if( pNd->IsCntntNode() )
            pMod = (SwModify*)pNd->GetCntntNode();
        else
        {
            ASSERT( pNd->IsTableNode(), "For Tablenodes only" );
            pMod = pNd->GetTableNode()->GetTable().GetFrmFmt();
        }
        pIter = new SwClientIter( *pMod );
    }
    else
        pIter = NULL;
}

/* -----------------25.02.99 10:41-------------------
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
        return FALSE;
    if( !bInit )
    {
         pRet = (SwFrm*)pIter->First(TYPE(SwFrm));
         bInit = TRUE;
    }
    else
        pRet = (SwFrm*)pIter->Next();
    while( pRet )
    {
        SwFlowFrm* pFlow = SwFlowFrm::CastFlowFrm( pRet );
        ASSERT( pFlow, "Cntnt or Table expected?!" );
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
                    ASSERT( pSct && pSct->GetSection(), "Where's my section?" );
                    SwSectionNode* pNd = pSct->GetSection()->GetFmt()->GetSectionNode();
                    ASSERT( pNd, "Lost SectionNode" );
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
        pRet = (SwFrm*)pIter->Next();
    }
    return NULL;
}

void SwNode2LayImpl::SaveUpperFrms()
{
    pUpperFrms = new SvPtrarr( 0, 20 );
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
            pUpperFrms->Insert( (void*)pPrv, pUpperFrms->Count() );
            pUpperFrms->Insert( (void*)pFrm, pUpperFrms->Count() );
        }
    }
    delete pIter;
    pIter = NULL;
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
                if( ((SwSectionNode*)pNode)->GetSection() ==
                    *((SwSectionFrm*)pFrm)->GetSection() )
                {
                    rpFrm = bMaster ? NULL : ((SwLayoutFrm*)pFrm)->Lower();
                    return ((SwLayoutFrm*)pFrm);
                }
                pUpper =new SwSectionFrm(((SwSectionNode*)pNode)->GetSection());
                pUpper->Paste( rpFrm->GetUpper(),
                               bMaster ? rpFrm : rpFrm->GetNext() );
                rpFrm = NULL;
                return pUpper;
            }
        }
    };
    if( !bMaster )
        rpFrm = rpFrm->GetNext();
    return pUpper;
}

void SwNode2LayImpl::RestoreUpperFrms( SwNodes& rNds, ULONG nStt, ULONG nEnd )
{
    ASSERT( pUpperFrms, "RestoreUpper without SaveUpper?" )
    SwNode* pNd;
    SwDoc *pDoc = rNds.GetDoc();
    BOOL bFirst = TRUE;
    for( ; nStt < nEnd; ++nStt )
    {
        SwFrm* pNew = 0;
        SwFrm* pNxt;
        SwLayoutFrm* pUp;
        if( (pNd = rNds[nStt])->IsCntntNode() )
            for( USHORT n = 0; n < pUpperFrms->Count(); )
            {
                pNxt = (SwFrm*)(*pUpperFrms)[n++];
                if( bFirst && pNxt && pNxt->IsSctFrm() )
                    ((SwSectionFrm*)pNxt)->UnlockJoin();
                pUp = (SwLayoutFrm*)(*pUpperFrms)[n++];
                if( pNxt )
                    pNxt = pNxt->GetNext();
                else
                    pNxt = pUp->Lower();
                pNew = ((SwCntntNode*)pNd)->MakeFrm();
                pNew->Paste( pUp, pNxt );
                (*pUpperFrms)[n-2] = pNew;
            }
        else if( pNd->IsTableNode() )
            for( USHORT x = 0; x < pUpperFrms->Count(); )
            {
                pNxt = (SwFrm*)(*pUpperFrms)[x++];
                if( bFirst && pNxt && pNxt->IsSctFrm() )
                    ((SwSectionFrm*)pNxt)->UnlockJoin();
                pUp = (SwLayoutFrm*)(*pUpperFrms)[x++];
                if( pNxt )
                    pNxt = pNxt->GetNext();
                else
                    pNxt = pUp->Lower();
                pNew = ((SwTableNode*)pNd)->MakeFrm();
                ASSERT( pNew->IsTabFrm(), "Table exspected" );
                pNew->Paste( pUp, pNxt );
                ((SwTabFrm*)pNew)->RegistFlys();
                (*pUpperFrms)[x-2] = pNew;
            }
        else if( pNd->IsSectionNode() )
        {
            nStt = pNd->EndOfSectionIndex();
            for( USHORT x = 0; x < pUpperFrms->Count(); )
            {
                pNxt = (SwFrm*)(*pUpperFrms)[x++];
                if( bFirst && pNxt && pNxt->IsSctFrm() )
                    ((SwSectionFrm*)pNxt)->UnlockJoin();
                pUp = (SwLayoutFrm*)(*pUpperFrms)[x++];
                ASSERT( pUp->GetUpper() || pUp->IsFlyFrm(), "Lost Upper" );
                ::_InsertCnt( pUp, pDoc, pNd->GetIndex(), FALSE, nStt+1, pNxt );
                pNxt = pUp->Lower();
                if( pNxt )
                    while( pNxt->GetNext() )
                        pNxt = pNxt->GetNext();
                (*pUpperFrms)[x-2] = pNxt;
            }
        }
        bFirst = FALSE;
    }
    for( USHORT x = 0; x < pUpperFrms->Count(); ++x )
    {
        SwFrm* pTmp = (SwFrm*)(*pUpperFrms)[++x];
        if( pTmp->IsFtnFrm() )
            ((SwFtnFrm*)pTmp)->ColUnlock();
        else if( pTmp->IsInSct() )
            pTmp->FindSctFrm()->ColUnlock();
    }
}

SwFrm* SwNode2LayImpl::GetFrm( const Point* pDocPos,
                                const SwPosition *pPos,
                                const BOOL bCalcFrm ) const
{
    return pIter ? ::GetFrmOfModify( pIter->GetModify(), USHRT_MAX,
                                        pDocPos, pPos, bCalcFrm )
                 : 0;
}

SwNode2Layout::SwNode2Layout( const SwNode& rNd, ULONG nIdx )
{
    pImpl = new SwNode2LayImpl( rNd, nIdx, FALSE );
}

SwNode2Layout::SwNode2Layout( const SwNode& rNd )
{
    pImpl = new SwNode2LayImpl( rNd, rNd.GetIndex(), TRUE );
    pImpl->SaveUpperFrms();
}

void SwNode2Layout::RestoreUpperFrms( SwNodes& rNds, ULONG nStt, ULONG nEnd )
{
    ASSERT( pImpl, "RestoreUpperFrms without SaveUpperFrms" );
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
                                const BOOL bCalcFrm ) const
{
    return pImpl->GetFrm( pDocPos, pPos, bCalcFrm );
}


