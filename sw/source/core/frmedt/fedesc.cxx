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

#include <fesh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <txtfrm.hxx>
#include <notxtfrm.hxx>
#include <pam.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <tabfrm.hxx>
#include <SwStyleNameMapper.hxx>
#include <ndtxt.hxx>
#include <osl/diagnose.h>

size_t SwFEShell::GetPageDescCnt() const
{
    return GetDoc()->GetPageDescCnt();
}

void SwFEShell::ChgCurPageDesc( const SwPageDesc& rDesc )
{
#if OSL_DEBUG_LEVEL > 0
    // SS does not change PageDesc, but only sets the attribute.
    // The Pagedesc should be available in the document
    bool bFound = false;
    for ( size_t nTst = 0; nTst < GetPageDescCnt(); ++nTst )
        if ( &rDesc == &GetPageDesc( nTst ) )
            bFound = true;
    OSL_ENSURE( bFound, "ChgCurPageDesc with invalid descriptor." );
#endif

    StartAllAction();

    SwPageFrame *pPage = GetCurrFrame()->FindPageFrame();
    const SwFrame *pFlow = nullptr;
    ::std::optional<sal_uInt16> oPageNumOffset;

    OSL_ENSURE( !GetCursor()->HasMark(), "ChgCurPageDesc only without selection!");

    CurrShell aCurr( this );
    while ( pPage )
    {
        pFlow = pPage->FindFirstBodyContent();
        if ( pFlow )
        {
            if ( pFlow->IsInTab() )
                pFlow = pFlow->FindTabFrame();
            const SwFormatPageDesc& rPgDesc = pFlow->GetPageDescItem();
            if( rPgDesc.GetPageDesc() )
            {
                // we found the culprit
                oPageNumOffset = rPgDesc.GetNumOffset();
                break;
            }
        }
        pPage = static_cast<SwPageFrame*>( pPage->GetPrev() );
    }
    if ( !pPage )
    {
        pPage = static_cast<SwPageFrame*>(GetLayout()->Lower());
        pFlow = pPage->FindFirstBodyContent();
        if ( !pFlow )
        {
            pPage   = static_cast<SwPageFrame*>(pPage->GetNext());
            pFlow = pPage->FindFirstBodyContent();
            OSL_ENSURE( pFlow, "Document without content?!?" );
        }
    }

    // use pagenumber
    SwFormatPageDesc aNew( &rDesc );
    aNew.SetNumOffset( oPageNumOffset );

    if ( pFlow->IsInTab() )
        GetDoc()->SetAttr( aNew, *const_cast<SwFormat*>(static_cast<SwFormat const *>(pFlow->FindTabFrame()->GetFormat())) );
    else
    {
        assert(pFlow->IsContentFrame());
        SwPaM aPaM( pFlow->IsTextFrame()
            ? *static_cast<SwTextFrame const*>(pFlow)->GetTextNodeFirst() // first, for PAGEDESC
            : *static_cast<const SwNoTextFrame*>(pFlow)->GetNode() );
        GetDoc()->getIDocumentContentOperations().InsertPoolItem(
                aPaM, aNew, SetAttrMode::DEFAULT, GetLayout());
    }
    EndAllActionAndCall();
}

void SwFEShell::ChgPageDesc( size_t i, const SwPageDesc &rChged )
{
    StartAllAction();
    CurrShell aCurr( this );
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
        sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( rName, SwGetPoolIdFromName::PageDesc );
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
        const SwPageFrame* pPage =
            static_cast<const SwPageFrame*>( GetLayout()->Lower() );
        if( pPage )
        {
            while( pPage->GetNext() && rPt.Y() > pPage->getFrameArea().Bottom() )
                pPage = static_cast<const SwPageFrame*>( pPage->GetNext() );
            SwDoc *pMyDoc = GetDoc();
            size_t nPos;
            if (pMyDoc->ContainsPageDesc( pPage->GetPageDesc(), &nPos ) )
                return nPos;
        }
    }
    return 0;
}

size_t SwFEShell::GetCurPageDesc( const bool bCalcFrame ) const
{
    const SwFrame *pFrame = GetCurrFrame( bCalcFrame );
    if ( pFrame )
    {
        const SwPageFrame *pPage = pFrame->FindPageFrame();
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
    const SwFrame* pMkFrame, *pPtFrame;
    const SwPageDesc* pFnd, *pRetDesc = reinterpret_cast<SwPageDesc*>(sal_IntPtr(-1));
    const Point aNulPt;
    std::pair<Point, bool> const tmp(aNulPt, false);

    for(const SwPaM& rPaM : GetCursor()->GetRingContainer())
    {

        if( nullptr != (pCNd = rPaM.GetContentNode() ) &&
            nullptr != (pPtFrame = pCNd->getLayoutFrame(GetLayout(), nullptr, &tmp)))
            pPtFrame = pPtFrame->FindPageFrame();
        else
            pPtFrame = nullptr;

        if( rPaM.HasMark() &&
            nullptr != (pCNd = rPaM.GetContentNode( false ) ) &&
            nullptr != (pMkFrame = pCNd->getLayoutFrame(GetLayout(), nullptr, &tmp)))
            pMkFrame = pMkFrame->FindPageFrame();
        else
            pMkFrame = pPtFrame;

        if( !pMkFrame || !pPtFrame )
            pFnd = nullptr;
        else if( pMkFrame == pPtFrame )
            pFnd = static_cast<const SwPageFrame*>(pMkFrame)->GetPageDesc();
        else
        {
            // swap pointer if PtFrame before MkFrame
            if( static_cast<const SwPageFrame*>(pMkFrame)->GetPhyPageNum() >
                static_cast<const SwPageFrame*>(pPtFrame)->GetPhyPageNum() )
            {
                const SwFrame* pTmp = pMkFrame; pMkFrame = pPtFrame; pPtFrame = pTmp;
            }

            // now check from MkFrame to PtFrame for equal PageDescs
            pFnd = static_cast<const SwPageFrame*>(pMkFrame)->GetPageDesc();
            while( pFnd && pMkFrame != pPtFrame )
            {
                pMkFrame = pMkFrame->GetNext();
                if( !pMkFrame || pFnd != static_cast<const SwPageFrame*>(pMkFrame)->GetPageDesc() )
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
