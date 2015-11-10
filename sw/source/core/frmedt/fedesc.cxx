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
#include <IDocumentContentOperations.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <tabfrm.hxx>
#include <edimp.hxx>
#include <SwStyleNameMapper.hxx>

size_t SwFEShell::GetPageDescCnt() const
{
    return GetDoc()->GetPageDescCnt();
}

void SwFEShell::ChgCurPageDesc( const SwPageDesc& rDesc )
{
#if OSL_DEBUG_LEVEL > 0
    // SS does not change PageDesc, but only sets the attibute.
    // The Pagedesc should be available in the document
    bool bFound = false;
    for ( size_t nTst = 0; nTst < GetPageDescCnt(); ++nTst )
        if ( &rDesc == &GetPageDesc( nTst ) )
            bFound = true;
    OSL_ENSURE( bFound, "ChgCurPageDesc with invalid descriptor." );
#endif

    StartAllAction();

    SwPageFrm *pPage = GetCurrFrm()->FindPageFrm();
    const SwFrm *pFlow = nullptr;
    ::boost::optional<sal_uInt16> oPageNumOffset;

    OSL_ENSURE( !GetCrsr()->HasMark(), "ChgCurPageDesc only without selection!");

    SET_CURR_SHELL( this );
    while ( pPage )
    {
        pFlow = pPage->FindFirstBodyContent();
        if ( pFlow )
        {
            if ( pFlow->IsInTab() )
                pFlow = pFlow->FindTabFrm();
            const SwFormatPageDesc& rPgDesc = pFlow->GetAttrSet()->GetPageDesc();
            if( rPgDesc.GetPageDesc() )
            {
                // wir haben ihn den Schlingel
                oPageNumOffset = rPgDesc.GetNumOffset();
                break;
            }
        }
        pPage = static_cast<SwPageFrm*>( pPage->GetPrev() );
    }
    if ( !pPage )
    {
        pPage = static_cast<SwPageFrm*>(GetLayout()->Lower());
        pFlow = pPage->FindFirstBodyContent();
        if ( !pFlow )
        {
            pPage   = static_cast<SwPageFrm*>(pPage->GetNext());
            pFlow = pPage->FindFirstBodyContent();
            OSL_ENSURE( pFlow, "Dokuemnt ohne Inhalt?!?" );
        }
    }

    // use pagenumber
    SwFormatPageDesc aNew( &rDesc );
    aNew.SetNumOffset( oPageNumOffset );

    if ( pFlow->IsInTab() )
        GetDoc()->SetAttr( aNew, *const_cast<SwFormat*>(static_cast<SwFormat const *>(pFlow->FindTabFrm()->GetFormat())) );
    else
    {
        SwPaM aPaM( *static_cast<const SwContentFrm*>(pFlow)->GetNode() );
        GetDoc()->getIDocumentContentOperations().InsertPoolItem( aPaM, aNew );
    }
    EndAllActionAndCall();
}

void SwFEShell::ChgPageDesc( size_t i, const SwPageDesc &rChged )
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

const SwPageDesc& SwFEShell::GetPageDesc( size_t i ) const
{
    return GetDoc()->GetPageDesc( i );
}

SwPageDesc* SwFEShell::FindPageDescByName( const OUString& rName,
                                            bool bGetFromPool,
                                            size_t* pPos )
{
    SwPageDesc* pDesc = GetDoc()->FindPageDesc(rName, pPos);
    if( !pDesc && bGetFromPool )
    {
        sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( rName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC );
        if( USHRT_MAX != nPoolId &&
            nullptr != (pDesc = GetDoc()->getIDocumentStylePoolAccess().GetPageDescFromPool( nPoolId ))
            && pPos )
                // appended always
            *pPos = GetDoc()->GetPageDescCnt() - 1 ;
    }
    return pDesc;
}

size_t SwFEShell::GetMousePageDesc( const Point &rPt ) const
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
            size_t nPos;
            if (pMyDoc->ContainsPageDesc( pPage->GetPageDesc(), &nPos ) )
                return nPos;
        }
    }
    return 0;
}

size_t SwFEShell::GetCurPageDesc( const bool bCalcFrm ) const
{
    const SwFrm *pFrm = GetCurrFrm( bCalcFrm );
    if ( pFrm )
    {
        const SwPageFrm *pPage = pFrm->FindPageFrm();
        if ( pPage )
        {
            size_t nPos;
            if (GetDoc()->ContainsPageDesc( pPage->GetPageDesc(), &nPos ))
                return nPos;
        }
    }
    return 0;
}

// if inside all selection only one PageDesc, return this.
// Otherwise return 0 pointer
const SwPageDesc* SwFEShell::GetSelectedPageDescs() const
{
    const SwContentNode* pCNd;
    const SwFrm* pMkFrm, *pPtFrm;
    const SwPageDesc* pFnd, *pRetDesc = reinterpret_cast<SwPageDesc*>(sal_IntPtr(-1));
    const Point aNulPt;

    for(SwPaM& rPaM : GetCrsr()->GetRingContainer())
    {

        if( nullptr != (pCNd = rPaM.GetContentNode() ) &&
            nullptr != ( pPtFrm = pCNd->getLayoutFrm( GetLayout(), &aNulPt, nullptr, false )) )
            pPtFrm = pPtFrm->FindPageFrm();
        else
            pPtFrm = nullptr;

        if( rPaM.HasMark() &&
            nullptr != (pCNd = rPaM.GetContentNode( false ) ) &&
            nullptr != ( pMkFrm = pCNd->getLayoutFrm( GetLayout(), &aNulPt, nullptr, false )) )
            pMkFrm = pMkFrm->FindPageFrm();
        else
            pMkFrm = pPtFrm;

        if( !pMkFrm || !pPtFrm )
            pFnd = nullptr;
        else if( pMkFrm == pPtFrm )
            pFnd = static_cast<const SwPageFrm*>(pMkFrm)->GetPageDesc();
        else
        {
            // swap pointer if PtFrm before MkFrm
            if( static_cast<const SwPageFrm*>(pMkFrm)->GetPhyPageNum() >
                static_cast<const SwPageFrm*>(pPtFrm)->GetPhyPageNum() )
            {
                const SwFrm* pTmp = pMkFrm; pMkFrm = pPtFrm; pPtFrm = pTmp;
            }

            // now check from MkFrm to PtFrm for equal PageDescs
            pFnd = static_cast<const SwPageFrm*>(pMkFrm)->GetPageDesc();
            while( pFnd && pMkFrm != pPtFrm )
            {
                pMkFrm = pMkFrm->GetNext();
                if( !pMkFrm || pFnd != static_cast<const SwPageFrm*>(pMkFrm)->GetPageDesc() )
                    pFnd = nullptr;
            }
        }

        if( reinterpret_cast<SwPageDesc*>(sal_IntPtr(-1)) == pRetDesc )
            pRetDesc = pFnd;
        else if( pFnd != pRetDesc )
        {
            pRetDesc = nullptr;
            break;
        }

    }

    return pRetDesc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
