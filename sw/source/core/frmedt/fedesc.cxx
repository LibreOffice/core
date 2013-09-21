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
#include <fesh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <tabfrm.hxx>
#include <edimp.hxx>
#include <SwStyleNameMapper.hxx>

/*************************************************************************
|*
|*  SwFEShell::GetPageDescCnt()
|*
|*************************************************************************/

sal_uInt16 SwFEShell::GetPageDescCnt() const
{
    return GetDoc()->GetPageDescCnt();
}

/*************************************************************************
|*
|*  SwFEShell::ChgCurPageDesc()
|*
|*************************************************************************/

void SwFEShell::ChgCurPageDesc( const SwPageDesc& rDesc )
{
#if OSL_DEBUG_LEVEL > 0
    // SS does not change PageDesc, but only sets the attibute.
    // The Pagedesc should be available in the document
    bool bFound = false;
    for ( sal_uInt16 nTst = 0; nTst < GetPageDescCnt(); ++nTst )
        if ( &rDesc == &GetPageDesc( nTst ) )
            bFound = true;
    OSL_ENSURE( bFound, "ChgCurPageDesc with invalid descriptor." );
#endif

    StartAllAction();

    SwPageFrm *pPage = GetCurrFrm()->FindPageFrm();
    const SwFrm *pFlow = 0;
    sal_uInt16 nPageNmOffset = 0;

    OSL_ENSURE( !GetCrsr()->HasMark(), "ChgCurPageDesc only without selection!");

    SET_CURR_SHELL( this );
    while ( pPage )
    {
        pFlow = pPage->FindFirstBodyCntnt();
        if ( pFlow )
        {
            if ( pFlow->IsInTab() )
                pFlow = pFlow->FindTabFrm();
            const SwFmtPageDesc& rPgDesc = pFlow->GetAttrSet()->GetPageDesc();
            if( rPgDesc.GetPageDesc() )
            {
                // wir haben ihn den Schlingel
                nPageNmOffset = rPgDesc.GetNumOffset();
                break;
            }
        }
        pPage = (SwPageFrm*) pPage->GetPrev();
    }
    if ( !pPage )
    {
        pPage = (SwPageFrm*) (GetLayout()->Lower());
        pFlow = pPage->FindFirstBodyCntnt();
        if ( !pFlow )
        {
            pPage   = (SwPageFrm*)pPage->GetNext();
            pFlow = pPage->FindFirstBodyCntnt();
            OSL_ENSURE( pFlow, "Dokuemnt ohne Inhalt?!?" );
        }
    }

    // use pagenumber
    SwFmtPageDesc aNew( &rDesc );
    aNew.SetNumOffset( nPageNmOffset );

    if ( pFlow->IsInTab() )
        GetDoc()->SetAttr( aNew, *(SwFmt*)pFlow->FindTabFrm()->GetFmt() );
    else
    {
        SwPaM aPaM( *((SwCntntFrm*)pFlow)->GetNode() );
        GetDoc()->InsertPoolItem( aPaM, aNew, 0 );
    }
    EndAllActionAndCall();
}

/*************************************************************************
|*
|*  SwFEShell::ChgPageDesc()
|*
|*************************************************************************/

void SwFEShell::ChgPageDesc( sal_uInt16 i, const SwPageDesc &rChged )
{
    StartAllAction();
    SET_CURR_SHELL( this );
    //Fix i64842: because Undo has a very special way to handle header/footer content
    // we have to copy the page descriptor before calling ChgPageDesc.
    SwPageDesc aDesc( rChged );
    {
        ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());
        GetDoc()->CopyPageDesc(rChged, aDesc);
    }
    GetDoc()->ChgPageDesc( i, aDesc );
    EndAllActionAndCall();
}

/*************************************************************************
|*
|*  SwFEShell::GetPageDesc(), GetCurPageDesc()
|*
|*************************************************************************/

const SwPageDesc& SwFEShell::GetPageDesc( sal_uInt16 i ) const
{
    return GetDoc()->GetPageDesc( i );
}

SwPageDesc* SwFEShell::FindPageDescByName( const String& rName,
                                            sal_Bool bGetFromPool,
                                            sal_uInt16* pPos )
{
    SwPageDesc* pDesc = GetDoc()->FindPageDescByName( rName, pPos );
    if( !pDesc && bGetFromPool )
    {
        sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( rName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC );
        if( USHRT_MAX != nPoolId &&
            0 != (pDesc = GetDoc()->GetPageDescFromPool( nPoolId ))
            && pPos )
                // appended always
            *pPos = GetDoc()->GetPageDescCnt() - 1 ;
    }
    return pDesc;
}

sal_uInt16 SwFEShell::GetMousePageDesc( const Point &rPt ) const
{
    if( GetLayout() )
    {
        const SwPageFrm* pPage =
            static_cast<const SwPageFrm*>( GetLayout()->Lower() );
        if( pPage )
        {
            while( pPage->GetNext() && rPt.Y() > pPage->Frm().Bottom() )
                pPage = static_cast<const SwPageFrm*>( pPage->GetNext() );
            SwDoc *pMyDoc = GetDoc();
            for ( sal_uInt16 i = 0; i < GetDoc()->GetPageDescCnt(); ++i )
            {
                if ( pPage->GetPageDesc() == &pMyDoc->GetPageDesc(i) )
                    return i;
            }
        }
    }
    return 0;
}

sal_uInt16 SwFEShell::GetCurPageDesc( const sal_Bool bCalcFrm ) const
{
    const SwFrm *pFrm = GetCurrFrm( bCalcFrm );
    if ( pFrm )
    {
        const SwPageFrm *pPage = pFrm->FindPageFrm();
        if ( pPage )
        {
            SwDoc *pMyDoc = GetDoc();
            for ( sal_uInt16 i = 0; i < GetDoc()->GetPageDescCnt(); ++i )
            {
                if ( pPage->GetPageDesc() == &pMyDoc->GetPageDesc(i) )
                    return i;
            }
        }
    }
    return 0;
}

// if inside all selection only one PageDesc, return this.
// Otherwise return 0 pointer
const SwPageDesc* SwFEShell::GetSelectedPageDescs() const
{
    const SwCntntNode* pCNd;
    const SwFrm* pMkFrm, *pPtFrm;
    const SwPageDesc* pFnd, *pRetDesc = (SwPageDesc*)0xffffffff;
    const Point aNulPt;

    FOREACHPAM_START(GetCrsr())

        if( 0 != (pCNd = PCURCRSR->GetCntntNode() ) &&
            0 != ( pPtFrm = pCNd->getLayoutFrm( GetLayout(), &aNulPt, 0, sal_False )) )
            pPtFrm = pPtFrm->FindPageFrm();
        else
            pPtFrm = 0;

        if( PCURCRSR->HasMark() &&
            0 != (pCNd = PCURCRSR->GetCntntNode( sal_False ) ) &&
            0 != ( pMkFrm = pCNd->getLayoutFrm( GetLayout(), &aNulPt, 0, sal_False )) )
            pMkFrm = pMkFrm->FindPageFrm();
        else
            pMkFrm = pPtFrm;


        if( !pMkFrm || !pPtFrm )
            pFnd = 0;
        else if( pMkFrm == pPtFrm )
            pFnd = ((SwPageFrm*)pMkFrm)->GetPageDesc();
        else
        {
            // swap pointer if PtFrm before MkFrm
            if( ((SwPageFrm*)pMkFrm)->GetPhyPageNum() >
                ((SwPageFrm*)pPtFrm)->GetPhyPageNum() )
            {
                const SwFrm* pTmp = pMkFrm; pMkFrm = pPtFrm; pPtFrm = pTmp;
            }

            // now check from MkFrm to PtFrm for equal PageDescs
            pFnd = ((SwPageFrm*)pMkFrm)->GetPageDesc();
            while( pFnd && pMkFrm != pPtFrm )
            {
                pMkFrm = pMkFrm->GetNext();
                if( !pMkFrm || pFnd != ((SwPageFrm*)pMkFrm)->GetPageDesc() )
                    pFnd = 0;
            }
        }

        if( (SwPageDesc*)0xffffffff == pRetDesc )
            pRetDesc = pFnd;
        else if( pFnd != pRetDesc )
        {
            pRetDesc = 0;
            break;
        }

    FOREACHPAM_END()

    return pRetDesc;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
