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

#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <ftninfo.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <redline.hxx>
#include <ftnidx.hxx>
#include <ndtxt.hxx>
#include <ndindex.hxx>
#include <section.hxx>
#include <fmtftntx.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>

namespace sw {

bool IsFootnoteDeleted(IDocumentRedlineAccess const& rIDRA,
        SwTextFootnote const& rTextFootnote)
{
    SwRedlineTable::size_type tmp;
    SwPosition const pos(const_cast<SwTextNode&>(rTextFootnote.GetTextNode()),
            rTextFootnote.GetStart());
    SwRangeRedline const*const pRedline(rIDRA.GetRedline(pos, &tmp));
    return (pRedline
        && pRedline->GetType() == RedlineType::Delete
        && *pRedline->GetPoint() != *pRedline->GetMark());
}

}

using sw::IsFootnoteDeleted;

bool CompareSwFootnoteIdxs::operator()(SwTextFootnote* const& lhs, SwTextFootnote* const& rhs) const
{
    sal_uLong nIdxLHS = SwTextFootnote_GetIndex( lhs );
    sal_uLong nIdxRHS = SwTextFootnote_GetIndex( rhs );
    return ( nIdxLHS == nIdxRHS && lhs->GetStart() < rhs->GetStart() ) || nIdxLHS < nIdxRHS;
}

void SwFootnoteIdxs::UpdateFootnote( const SwNodeIndex& rStt )
{
    if( empty() )
        return;

    // Get the NodesArray using the first foot note's StartIndex
    SwDoc& rDoc = rStt.GetNode().GetDoc();
    if( rDoc.IsInReading() )
        return ;
    SwTextFootnote* pTextFootnote;

    const SwEndNoteInfo& rEndInfo = rDoc.GetEndNoteInfo();
    const SwFootnoteInfo& rFootnoteInfo = rDoc.GetFootnoteInfo();
    IDocumentRedlineAccess const& rIDRA(rDoc.getIDocumentRedlineAccess());

    // For normal foot notes we treat per-chapter and per-document numbering
    // separately. For Endnotes we only have per-document numbering.
    if( FTNNUM_CHAPTER == rFootnoteInfo.m_eNum )
    {
        SwRootFrame const* pLayout(nullptr);
        o3tl::sorted_vector<SwRootFrame*> layouts = rDoc.GetAllLayouts();
        // sw_redlinehide: here we need to know if there's *any* layout with
        // IsHideRedlines(), because then the hidden-numbers have to be updated
        for (SwRootFrame const* pTmp : layouts)
        {
            if (pTmp->IsHideRedlines())
            {
                pLayout = pTmp;
            }
        }

        const SwOutlineNodes& rOutlNds = rDoc.GetNodes().GetOutLineNds();
        const SwNode *pChapterStartHidden(&rDoc.GetNodes().GetEndOfExtras());
        sal_uLong nChapterStart(pChapterStartHidden->GetIndex());
        sal_uLong nChapterEnd(rDoc.GetNodes().GetEndOfContent().GetIndex());
        sal_uLong nChapterEndHidden(nChapterEnd);
        if( !rOutlNds.empty() )
        {
            // Find the Chapter's start, which contains rStt
            size_t n = 0;

            for( ; n < rOutlNds.size(); ++n )
                if( rOutlNds[ n ]->GetIndex() > rStt.GetIndex() )
                    break;      // found it!
                else if ( rOutlNds[ n ]->GetTextNode()->GetAttrOutlineLevel() == 1 )
                {
                    nChapterStart = rOutlNds[ n ]->GetIndex();
                    if (!pLayout || sw::IsParaPropsNode(*pLayout, *rOutlNds[n]->GetTextNode()))
                    {
                        pChapterStartHidden = rOutlNds[ n ];
                    }
                }
            // now find the end of the range
            for( ; n < rOutlNds.size(); ++n )
                if ( rOutlNds[ n ]->GetTextNode()->GetAttrOutlineLevel() == 1 )
                {
                    nChapterEnd = rOutlNds[ n ]->GetIndex();
                    break;
                }

            // continue to find end of hidden-chapter
            for ( ; n < rOutlNds.size(); ++n)
            {
                if (rOutlNds[n]->GetTextNode()->GetAttrOutlineLevel() == 1
                    && (!pLayout || sw::IsParaPropsNode(*pLayout, *rOutlNds[n]->GetTextNode())))
                {
                    nChapterEndHidden = rOutlNds[n]->GetIndex();
                    break;
                }
            }
        }

        size_t nPos = 0;
        size_t nFootnoteNo = 1;
        size_t nFootnoteNoHidden = 1;
        if (SeekEntry( *pChapterStartHidden, &nPos ) && nPos)
        {
            // Step forward until the Index is not the same anymore
            const SwNode* pCmpNd = &rStt.GetNode();
            while( nPos && pCmpNd == &((*this)[ --nPos ]->GetTextNode()) )
                ;
            ++nPos;
        }

        if( nPos == size() )       // nothing found
            return;

        if( rOutlNds.empty() )
        {
            nFootnoteNo = nPos+1;
            if (nPos)
            {
                nFootnoteNoHidden = (*this)[nPos - 1]->GetFootnote().GetNumberRLHidden() + 1;
            }
        }

        for( ; nPos < size(); ++nPos )
        {
            pTextFootnote = (*this)[ nPos ];
            sal_uLong const nNode(pTextFootnote->GetTextNode().GetIndex());
            if (nChapterEndHidden <= nNode)
                break;

            const SwFormatFootnote &rFootnote = pTextFootnote->GetFootnote();
            if( rFootnote.GetNumStr().isEmpty() && !rFootnote.IsEndNote() &&
                !SwUpdFootnoteEndNtAtEnd::FindSectNdWithEndAttr( *pTextFootnote ))
            {
                pTextFootnote->SetNumber(
                    (nChapterStart <= nNode && nNode < nChapterEnd)
                        ? rFootnoteInfo.m_nFootnoteOffset + nFootnoteNo
                        : rFootnote.GetNumber(),
                    rFootnoteInfo.m_nFootnoteOffset + nFootnoteNoHidden,
                    rFootnote.GetNumStr() );
                if (nChapterStart <= nNode && nNode < nChapterEnd)
                {
                    ++nFootnoteNo;
                }
                if (!IsFootnoteDeleted(rIDRA, *pTextFootnote))
                {
                    ++nFootnoteNoHidden;
                }
            }
        }
    }

    SwUpdFootnoteEndNtAtEnd aNumArr;

    // unless we have per-document numbering, only look at endnotes here
    const bool bEndNoteOnly = FTNNUM_DOC != rFootnoteInfo.m_eNum;

    size_t nPos;
    size_t nFootnoteNo = 1;
    size_t nEndNo = 1;
    size_t nFootnoteNoHidden = 1;
    size_t nEndNoHidden = 1;
    sal_uLong nUpdNdIdx = rStt.GetIndex();
    for( nPos = 0; nPos < size(); ++nPos )
    {
        pTextFootnote = (*this)[ nPos ];
        if( nUpdNdIdx <= pTextFootnote->GetTextNode().GetIndex() )
            break;

        const SwFormatFootnote &rFootnote = pTextFootnote->GetFootnote();
        if( rFootnote.GetNumStr().isEmpty() )
        {
            if (!aNumArr.ChkNumber(rIDRA, *pTextFootnote).first)
            {
                if( pTextFootnote->GetFootnote().IsEndNote() )
                {
                    nEndNo++;
                    if (!IsFootnoteDeleted(rIDRA, *pTextFootnote))
                    {
                        ++nEndNoHidden;
                    }
                }
                else
                {
                    nFootnoteNo++;
                    if (!IsFootnoteDeleted(rIDRA, *pTextFootnote))
                    {
                        ++nFootnoteNoHidden;
                    }
                }
            }
        }
    }

    // Set the array number for all footnotes starting from nPos
    for( ; nPos < size(); ++nPos )
    {
        pTextFootnote = (*this)[ nPos ];
        const SwFormatFootnote &rFootnote = pTextFootnote->GetFootnote();
        if( rFootnote.GetNumStr().isEmpty() )
        {
            std::pair<sal_uInt16, sal_uInt16> nSectNo = aNumArr.ChkNumber(rIDRA, *pTextFootnote);
            if (!nSectNo.first && (rFootnote.IsEndNote() || !bEndNoteOnly))
            {
                if (rFootnote.IsEndNote())
                {
                    nSectNo.first = rEndInfo.m_nFootnoteOffset + nEndNo;
                    ++nEndNo;
                    nSectNo.second = rEndInfo.m_nFootnoteOffset + nEndNoHidden;
                    if (!IsFootnoteDeleted(rIDRA, *pTextFootnote))
                    {
                        ++nEndNoHidden;
                    }
                }
                else
                {
                    nSectNo.first = rFootnoteInfo.m_nFootnoteOffset + nFootnoteNo;
                    ++nFootnoteNo;
                    nSectNo.second = rFootnoteInfo.m_nFootnoteOffset + nFootnoteNoHidden;
                    if (!IsFootnoteDeleted(rIDRA, *pTextFootnote))
                    {
                        ++nFootnoteNoHidden;
                    }
                }
            }

            if (nSectNo.first)
            {
                pTextFootnote->SetNumber(nSectNo.first, nSectNo.second, rFootnote.GetNumStr());
            }
        }
    }
}

void SwFootnoteIdxs::UpdateAllFootnote()
{
    if( empty() )
        return;

    // Get the NodesArray via the StartIndex of the first Footnote
    SwDoc& rDoc = const_cast<SwDoc&>((*this)[ 0 ]->GetTextNode().GetDoc());
    SwTextFootnote* pTextFootnote;
    const SwEndNoteInfo& rEndInfo = rDoc.GetEndNoteInfo();
    const SwFootnoteInfo& rFootnoteInfo = rDoc.GetFootnoteInfo();
    IDocumentRedlineAccess const& rIDRA(rDoc.getIDocumentRedlineAccess());

    SwUpdFootnoteEndNtAtEnd aNumArr;

    SwRootFrame const* pLayout = rDoc.getIDocumentLayoutAccess().GetCurrentLayout();
    o3tl::sorted_vector<SwRootFrame*> aAllLayouts = rDoc.GetAllLayouts();
    // For normal Footnotes per-chapter and per-document numbering are treated separately.
    // For Endnotes we only have document-wise numbering.
    if( FTNNUM_CHAPTER == rFootnoteInfo.m_eNum )
    {
        // sw_redlinehide: here we need to know if there's *any* layout with
        // IsHideRedlines(), because then the hidden-numbers have to be updated
        for (SwRootFrame const* pTmp : aAllLayouts)
        {
            if (pTmp->IsHideRedlines())
            {
                pLayout = pTmp;
            }
        }

        const SwOutlineNodes& rOutlNds = rDoc.GetNodes().GetOutLineNds();
        sal_uInt16 nNo = 1;     // Number for the Footnotes
        sal_uInt16 nNoNo = 1;
        size_t nFootnoteIdx = 0;     // Index into theFootnoteIdx array
        for( size_t n = 0; n < rOutlNds.size(); ++n )
        {
            if ( rOutlNds[ n ]->GetTextNode()->GetAttrOutlineLevel() == 1 )
            {
                sal_uLong nCapStt = rOutlNds[ n ]->GetIndex();  // Start of a new chapter
                for( ; nFootnoteIdx < size(); ++nFootnoteIdx )
                {
                    pTextFootnote = (*this)[ nFootnoteIdx ];
                    if( pTextFootnote->GetTextNode().GetIndex() >= nCapStt )
                        break;

                    // Endnotes are per-document only
                    const SwFormatFootnote &rFootnote = pTextFootnote->GetFootnote();
                    if( !rFootnote.IsEndNote() && rFootnote.GetNumStr().isEmpty() &&
                        !SwUpdFootnoteEndNtAtEnd::FindSectNdWithEndAttr( *pTextFootnote ))
                    {
                        pTextFootnote->SetNumber(
                            rFootnoteInfo.m_nFootnoteOffset + nNo,
                            rFootnoteInfo.m_nFootnoteOffset + nNoNo,
                            rFootnote.GetNumStr() );
                        ++nNo;
                        if (!IsFootnoteDeleted(rIDRA, *pTextFootnote))
                        {
                            ++nNoNo;
                        }
                    }
                }
                if( nFootnoteIdx >= size() )
                    break;          // ok, everything is updated
                nNo = 1;
                // sw_redlinehide: this means the numbers are layout dependent in chapter case
                if (!pLayout || sw::IsParaPropsNode(*pLayout, *rOutlNds[ n ]->GetTextNode()))
                {
                    nNoNo = 1;
                }
            }
        }

        for (nNo = 1, nNoNo = 1; nFootnoteIdx < size(); ++nFootnoteIdx)
        {
            // Endnotes are per-document
            pTextFootnote = (*this)[ nFootnoteIdx ];
            const SwFormatFootnote &rFootnote = pTextFootnote->GetFootnote();
            if( !rFootnote.IsEndNote() && rFootnote.GetNumStr().isEmpty() &&
                !SwUpdFootnoteEndNtAtEnd::FindSectNdWithEndAttr( *pTextFootnote ))
            {
                pTextFootnote->SetNumber(
                        rFootnoteInfo.m_nFootnoteOffset + nNo,
                        rFootnoteInfo.m_nFootnoteOffset + nNoNo,
                        rFootnote.GetNumStr() );
                ++nNo;
                if (!IsFootnoteDeleted(rIDRA, *pTextFootnote))
                {
                    ++nNoNo;
                }
            }
        }
    }

    // We use bool here, so that we also iterate through the Endnotes with a chapter setting.
    const bool bEndNoteOnly = FTNNUM_DOC != rFootnoteInfo.m_eNum;
    sal_uInt16 nFootnoteNo = 1;
    sal_uInt16 nEndnoteNo = 1;
    sal_uInt16 nFootnoteNoHidden = 1;
    sal_uInt16 nEndnoteNoHidden = 1;
    for( size_t nPos = 0; nPos < size(); ++nPos )
    {
        pTextFootnote = (*this)[ nPos ];
        const SwFormatFootnote &rFootnote = pTextFootnote->GetFootnote();
        if( rFootnote.GetNumStr().isEmpty() )
        {
            std::pair<sal_uInt16, sal_uInt16> nSectNo = aNumArr.ChkNumber(rIDRA, *pTextFootnote);
            if (!nSectNo.first && (rFootnote.IsEndNote() || !bEndNoteOnly))
            {
                if (rFootnote.IsEndNote())
                {
                    nSectNo.first = rEndInfo.m_nFootnoteOffset + nEndnoteNo;
                    ++nEndnoteNo;
                    nSectNo.second = rEndInfo.m_nFootnoteOffset + nEndnoteNoHidden;
                    if (!IsFootnoteDeleted(rIDRA, *pTextFootnote))
                    {
                        ++nEndnoteNoHidden;
                    }
                }
                else
                {
                    nSectNo.first = rFootnoteInfo.m_nFootnoteOffset + nFootnoteNo;
                    ++nFootnoteNo;
                    nSectNo.second = rFootnoteInfo.m_nFootnoteOffset + nFootnoteNoHidden;
                    if (!IsFootnoteDeleted(rIDRA, *pTextFootnote))
                    {
                        ++nFootnoteNoHidden;
                    }
                }
            }

            if (nSectNo.first)
            {
                pTextFootnote->SetNumber(nSectNo.first, nSectNo.second, rFootnote.GetNumStr());
            }
        }
    }

    if (pLayout && FTNNUM_PAGE == rFootnoteInfo.m_eNum)
        for( auto aLayout : aAllLayouts )
            aLayout->UpdateFootnoteNums();
}

SwTextFootnote* SwFootnoteIdxs::SeekEntry( const SwNodeIndex& rPos, size_t* pFndPos ) const
{
    sal_uLong nIdx = rPos.GetIndex();

    size_t nO = size();
    size_t nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            const size_t nM = nU + ( nO - nU ) / 2;
            sal_uLong nNdIdx = SwTextFootnote_GetIndex( (*this)[ nM ] );
            if( nNdIdx == nIdx )
            {
                if( pFndPos )
                    *pFndPos = nM;
                return (*this)[ nM ];
            }
            else if( nNdIdx < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pFndPos )
                    *pFndPos = nU;
                return nullptr;
            }
            else
                nO = nM - 1;
        }
    }
    if( pFndPos )
        *pFndPos = nU;
    return nullptr;
}

const SwSectionNode* SwUpdFootnoteEndNtAtEnd::FindSectNdWithEndAttr(
                const SwTextFootnote& rTextFootnote )
{
    sal_uInt16 nWh = rTextFootnote.GetFootnote().IsEndNote() ?
                        sal_uInt16(RES_END_AT_TXTEND) : sal_uInt16(RES_FTN_AT_TXTEND);
    const SwSectionNode* pNd = rTextFootnote.GetTextNode().FindSectionNode();
    while( pNd )
    {
        sal_uInt16 nVal = static_cast<const SwFormatFootnoteEndAtTextEnd&>(pNd->GetSection().GetFormat()->
                 GetFormatAttr( nWh )).GetValue();
        if( FTNEND_ATTXTEND_OWNNUMSEQ == nVal || FTNEND_ATTXTEND_OWNNUMANDFMT == nVal )
            break;
        pNd = pNd->StartOfSectionNode()->FindSectionNode();
    }

    return pNd;
}

std::pair<sal_uInt16, sal_uInt16> SwUpdFootnoteEndNtAtEnd::GetNumber(
        IDocumentRedlineAccess const& rIDRA,
        const SwTextFootnote& rTextFootnote,
                                    const SwSectionNode& rNd )
{
    std::pair<sal_uInt16, sal_uInt16> nRet(0, 0);
    sal_uInt16 nWh;
    std::vector<const SwSectionNode*>* pArr;
    std::vector<std::pair<sal_uInt16, sal_uInt16>> *pNum;
    if( rTextFootnote.GetFootnote().IsEndNote() )
    {
        pArr = &m_aEndSections;
        pNum = &m_aEndNumbers;
        nWh = RES_END_AT_TXTEND;
    }
    else
    {
        pArr = &m_aFootnoteSections;
        pNum = &m_aFootnoteNumbers;
        nWh = RES_FTN_AT_TXTEND;
    }

    for( size_t n = pArr->size(); n; )
        if( (*pArr)[ --n ] == &rNd )
        {
            nRet.first = ++((*pNum)[ n ].first);
            if (!IsFootnoteDeleted(rIDRA, rTextFootnote))
            {
                ++((*pNum)[ n ].second);
            }
            nRet.second = ((*pNum)[ n ].second);
            break;
        }

    if (!nRet.first)
    {
        pArr->push_back( &rNd );
        sal_uInt16 const tmp = static_cast<const SwFormatFootnoteEndAtTextEnd&>(
                rNd.GetSection().GetFormat()->
                                GetFormatAttr( nWh )).GetOffset();
        nRet.first = tmp + 1;
        nRet.second = tmp + 1;
        pNum->push_back( nRet );
    }
    return nRet;
}

std::pair<sal_uInt16, sal_uInt16> SwUpdFootnoteEndNtAtEnd::ChkNumber(
        IDocumentRedlineAccess const& rIDRA,
        const SwTextFootnote& rTextFootnote)
{
    const SwSectionNode* pSectNd = FindSectNdWithEndAttr( rTextFootnote );
    return pSectNd
            ? GetNumber(rIDRA, rTextFootnote, *pSectNd)
            : std::pair<sal_uInt16, sal_uInt16>(0, 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
