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

#include <crsrsh.hxx>
#include <doc.hxx>
#include <swcrsr.hxx>
#include <docary.hxx>
#include <fmtcntnt.hxx>
#include <viscrs.hxx>
#include <callnk.hxx>
#include <pamtyp.hxx>
#include <section.hxx>
#include <trvlreg.hxx>

bool GotoPrevRegion( SwPaM& rCurrentCursor, SwPosRegion fnPosRegion,
                        bool bInReadOnly )
{
    SwNodeIndex aIdx( rCurrentCursor.GetPoint()->nNode );
    SwSectionNode* pNd = aIdx.GetNode().FindSectionNode();
    if( pNd )
        aIdx.Assign( *pNd, - 1 );

    do {
        while( aIdx.GetIndex() &&
            nullptr == ( pNd = aIdx.GetNode().StartOfSectionNode()->GetSectionNode()) )
            --aIdx;

        if( pNd ) // is there another section node?
        {
            if( pNd->GetSection().IsHiddenFlag() ||
                ( !bInReadOnly &&
                  pNd->GetSection().IsProtectFlag() ))
            {
                // skip protected or hidden ones
                aIdx.Assign( *pNd, - 1 );
            }
            else if( fnPosRegion == fnMoveForward )
            {
                aIdx = *pNd;
                SwContentNode* pCNd = pNd->GetNodes().GoNextSection( &aIdx,
                                                true, !bInReadOnly );
                if( !pCNd )
                {
                    --aIdx;
                    continue;
                }
                rCurrentCursor.GetPoint()->nContent.Assign( pCNd, 0 );
            }
            else
            {
                aIdx = *pNd->EndOfSectionNode();
                SwContentNode* pCNd = SwNodes::GoPrevSection( &aIdx,
                                                true, !bInReadOnly );
                if( !pCNd )
                {
                    aIdx.Assign( *pNd, - 1 );
                    continue;
                }
                rCurrentCursor.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
            }
            rCurrentCursor.GetPoint()->nNode = aIdx;
            return true;
        }
    } while( pNd );
    return false;
}

bool GotoNextRegion( SwPaM& rCurrentCursor, SwPosRegion fnPosRegion,
                        bool bInReadOnly )
{
    SwNodeIndex aIdx( rCurrentCursor.GetPoint()->nNode );
    SwSectionNode* pNd = aIdx.GetNode().FindSectionNode();
    if( pNd )
        aIdx.Assign( *pNd->EndOfSectionNode(), - 1 );

    sal_uLong nEndCount = aIdx.GetNode().GetNodes().Count()-1;
    do {
        while( aIdx.GetIndex() < nEndCount &&
                nullptr == ( pNd = aIdx.GetNode().GetSectionNode()) )
            ++aIdx;

        if( pNd ) // is there another section node?
        {
            if( pNd->GetSection().IsHiddenFlag() ||
                ( !bInReadOnly &&
                  pNd->GetSection().IsProtectFlag() ))
            {
                // skip protected or hidden ones
                aIdx.Assign( *pNd->EndOfSectionNode(), +1 );
            }
            else if( fnPosRegion == fnMoveForward )
            {
                aIdx = *pNd;
                SwContentNode* pCNd = pNd->GetNodes().GoNextSection( &aIdx,
                                                true, !bInReadOnly );
                if( !pCNd )
                {
                    aIdx.Assign( *pNd->EndOfSectionNode(), +1 );
                    continue;
                }
                rCurrentCursor.GetPoint()->nContent.Assign( pCNd, 0 );
            }
            else
            {
                aIdx = *pNd->EndOfSectionNode();
                SwContentNode* pCNd = SwNodes::GoPrevSection( &aIdx,
                                                true, !bInReadOnly );
                if( !pCNd )
                {
                    ++aIdx;
                    continue;
                }
                rCurrentCursor.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
            }
            rCurrentCursor.GetPoint()->nNode = aIdx;
            return true;
        }
    } while( pNd );
    return false;
}

bool GotoCurrRegion( SwPaM& rCurrentCursor, SwPosRegion fnPosRegion,
                        bool bInReadOnly )
{
    SwSectionNode* pNd = rCurrentCursor.GetNode().FindSectionNode();
    if( !pNd )
        return false;

    SwPosition* pPos = rCurrentCursor.GetPoint();
    bool bMoveBackward = fnPosRegion == fnMoveBackward;

    SwContentNode* pCNd;
    if( bMoveBackward )
    {
        SwNodeIndex aIdx( *pNd->EndOfSectionNode() );
        pCNd = SwNodes::GoPrevSection( &aIdx, true, !bInReadOnly );
    }
    else
    {
        SwNodeIndex aIdx( *pNd );
        pCNd = pNd->GetNodes().GoNextSection( &aIdx, true, !bInReadOnly );
    }

    if( pCNd )
    {
        pPos->nNode = *pCNd;
        pPos->nContent.Assign( pCNd, bMoveBackward ? pCNd->Len() : 0 );
    }
    return nullptr != pCNd;
}

bool GotoCurrRegionAndSkip( SwPaM& rCurrentCursor, SwPosRegion fnPosRegion,
                                bool bInReadOnly )
{
    SwNode& rCurrNd = rCurrentCursor.GetNode();
    SwSectionNode* pNd = rCurrNd.FindSectionNode();
    if( !pNd )
        return false;

    SwPosition* pPos = rCurrentCursor.GetPoint();
    const sal_Int32 nCurrCnt = pPos->nContent.GetIndex();
    bool bMoveBackward = fnPosRegion == fnMoveBackward;

    do {
        SwContentNode* pCNd;
        if( bMoveBackward ) // to the end of the section
        {
            SwNodeIndex aIdx( *pNd->EndOfSectionNode() );
            pCNd = SwNodes::GoPrevSection( &aIdx, true, !bInReadOnly );
            if( !pCNd )
                return false;
            pPos->nNode = aIdx;
        }
        else
        {
            SwNodeIndex aIdx( *pNd );
            pCNd = pNd->GetNodes().GoNextSection( &aIdx, true, !bInReadOnly );
            if( !pCNd )
                return false;
            pPos->nNode = aIdx;
        }

        pPos->nContent.Assign( pCNd, bMoveBackward ? pCNd->Len() : 0 );

        if( &pPos->nNode.GetNode() != &rCurrNd ||
            pPos->nContent.GetIndex() != nCurrCnt )
            // there was a change
            return true;

        // try also the parent of this section
        SwSection* pParent = pNd->GetSection().GetParent();
        pNd = pParent ? pParent->GetFormat()->GetSectionNode() : nullptr;
    } while( pNd );
    return false;
}

bool SwCursor::MoveRegion( SwWhichRegion fnWhichRegion, SwPosRegion fnPosRegion )
{
    SwCursorSaveState aSaveState( *this );
    return !dynamic_cast<SwTableCursor*>(this) &&
            (*fnWhichRegion)( *this, fnPosRegion, IsReadOnlyAvailable()  ) &&
            !IsSelOvr() &&
            (GetPoint()->nNode.GetIndex() != m_pSavePos->nNode ||
             GetPoint()->nContent.GetIndex() != m_pSavePos->nContent);
}

bool SwCursorShell::MoveRegion( SwWhichRegion fnWhichRegion, SwPosRegion fnPosRegion )
{
    SwCallLink aLk( *this ); // watch Cursor-Moves;call Link if needed
    bool bRet = !m_pTableCursor && m_pCurrentCursor->MoveRegion( fnWhichRegion, fnPosRegion );
    if( bRet )
        UpdateCursor();
    return bRet;
}

bool SwCursor::GotoRegion( const OUString& rName )
{
    bool bRet = false;
    const SwSectionFormats& rFormats = GetDoc()->GetSections();
    for( SwSectionFormats::size_type n = rFormats.size(); n; )
    {
        const SwSectionFormat* pFormat = rFormats[ --n ];
        const SwNodeIndex* pIdx = nullptr;
        const SwSection* pSect;
        if( nullptr != ( pSect = pFormat->GetSection() ) &&
            pSect->GetSectionName() == rName &&
            nullptr != ( pIdx = pFormat->GetContent().GetContentIdx() ) &&
            pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            // area in normal nodes array
            SwCursorSaveState aSaveState( *this );

            GetPoint()->nNode = *pIdx;
            Move( fnMoveForward, fnGoContent );
            bRet = !IsSelOvr();
        }
    }
    return bRet;
}

bool SwCursorShell::GotoRegion( const OUString& rName )
{
    SwCallLink aLk( *this ); // watch Cursor-Moves;call Link if needed
    bool bRet = !m_pTableCursor && m_pCurrentCursor->GotoRegion( rName );
    if( bRet )
        UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                    SwCursorShell::READONLY );
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
