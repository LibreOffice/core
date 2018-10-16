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

static bool IsFootnoteDeleted(IDocumentRedlineAccess const& rIDRA,
        SwTextFootnote const& rTextFootnote)
{
    SwRedlineTable::size_type tmp;
    SwPosition const pos(const_cast<SwTextNode&>(rTextFootnote.GetTextNode()),
            rTextFootnote.GetStart());
    SwRangeRedline const*const pRedline(rIDRA.GetRedline(pos, &tmp));
    return (pRedline
        && pRedline->GetType() == nsRedlineType_t::REDLINE_DELETE
        && *pRedline->GetPoint() != *pRedline->GetMark());
}

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
    SwDoc* pDoc = rStt.GetNode().GetDoc();
    if( pDoc->IsInReading() )
        return ;
    SwTextFootnote* pTextFootnote;

    const SwEndNoteInfo& rEndInfo = pDoc->GetEndNoteInfo();
    const SwFootnoteInfo& rFootnoteInfo = pDoc->GetFootnoteInfo();
    IDocumentRedlineAccess const& rIDRA(pDoc->getIDocumentRedlineAccess());

    // For normal foot notes we treat per-chapter and per-document numbering
    // separately. For Endnotes we only have per-document numbering.
    if( FTNNUM_CHAPTER == rFootnoteInfo.eNum )
    {
        const SwOutlineNodes& rOutlNds = pDoc->GetNodes().GetOutLineNds();
        const SwNode* pCapStt = &pDoc->GetNodes().GetEndOfExtras();
        sal_uLong nCapEnd = pDoc->GetNodes().GetEndOfContent().GetIndex();
        if( !rOutlNds.empty() )
        {
            // Find the Chapter's start, which contains rStt
            size_t n = 0;

            for( ; n < rOutlNds.size(); ++n )
                if( rOutlNds[ n ]->GetIndex() > rStt.GetIndex() )
                    break;      // found it!
                else if ( rOutlNds[ n ]->GetTextNode()->GetAttrOutlineLevel() == 1 )
                    pCapStt = rOutlNds[ n ];    // Beginning of a new Chapter
            // now find the end of the range
            for( ; n < rOutlNds.size(); ++n )
                if ( rOutlNds[ n ]->GetTextNode()->GetAttrOutlineLevel() == 1 )
                {
                    nCapEnd = rOutlNds[ n ]->GetIndex();    // End of the found Chapter
                    break;
                }
        }

        size_t nPos = 0;
        size_t nFootnoteNo = 1;
        size_t nFootnoteNoHidden = 1;
        if( SeekEntry( *pCapStt, &nPos ) && nPos )
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
            if( pTextFootnote->GetTextNode().GetIndex() >= nCapEnd )
                break;

            const SwFormatFootnote &rFootnote = pTextFootnote->GetFootnote();
            if( rFootnote.GetNumStr().isEmpty() && !rFootnote.IsEndNote() &&
                !SwUpdFootnoteEndNtAtEnd::FindSectNdWithEndAttr( *pTextFootnote ))
            {
                pTextFootnote->SetNumber(
                    rFootnoteInfo.nFootnoteOffset + nFootnoteNo,
                    rFootnoteInfo.nFootnoteOffset + nFootnoteNoHidden,
                    rFootnote.GetNumStr() );
                ++nFootnoteNo;
                if (!IsFootnoteDeleted(rIDRA, *pTextFootnote))
                {
                    ++nFootnoteNoHidden;
                }
            }
        }
    }

    SwUpdFootnoteEndNtAtEnd aNumArr;

    // unless we have per-document numbering, only look at endnotes here
    const bool bEndNoteOnly = FTNNUM_DOC != rFootnoteInfo.eNum;

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
                    nSectNo.first = rEndInfo.nFootnoteOffset + nEndNo;
                    ++nEndNo;
                    nSectNo.second = rEndInfo.nFootnoteOffset + nEndNoHidden;
                    if (!IsFootnoteDeleted(rIDRA, *pTextFootnote))
                    {
                        ++nEndNoHidden;
                    }
                }
                else
                {
                    nSectNo.first = rFootnoteInfo.nFootnoteOffset + nFootnoteNo;
                    ++nFootnoteNo;
                    nSectNo.second = rFootnoteInfo.nFootnoteOffset + nFootnoteNoHidden;
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
    SwDoc* pDoc = const_cast<SwDoc*>((*this)[ 0 ]->GetTextNode().GetDoc());
    SwTextFootnote* pTextFootnote;
    const SwEndNoteInfo& rEndInfo = pDoc->GetEndNoteInfo();
    const SwFootnoteInfo& rFootnoteInfo = pDoc->GetFootnoteInfo();
    IDocumentRedlineAccess const& rIDRA(pDoc->getIDocumentRedlineAccess());

    SwUpdFootnoteEndNtAtEnd aNumArr;

    SwRootFrame* pTmpRoot = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    std::set<SwRootFrame*> aAllLayouts = pDoc->GetAllLayouts();
    // For normal Footnotes per-chapter and per-document numbering are treated separately.
    // For Endnotes we only have document-wise numbering.
    if( FTNNUM_CHAPTER == rFootnoteInfo.eNum )
    {
        const SwOutlineNodes& rOutlNds = pDoc->GetNodes().GetOutLineNds();
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
                            rFootnoteInfo.nFootnoteOffset + nNo,
                            rFootnoteInfo.nFootnoteOffset + nNoNo,
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
                nNoNo = 1;
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
                        rFootnoteInfo.nFootnoteOffset + nNo,
                        rFootnoteInfo.nFootnoteOffset + nNoNo,
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
    const bool bEndNoteOnly = FTNNUM_DOC != rFootnoteInfo.eNum;
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
                    nSectNo.first = rEndInfo.nFootnoteOffset + nEndnoteNo;
                    ++nEndnoteNo;
                    nSectNo.second = rEndInfo.nFootnoteOffset + nEndnoteNoHidden;
                    if (!IsFootnoteDeleted(rIDRA, *pTextFootnote))
                    {
                        ++nEndnoteNoHidden;
                    }
                }
                else
                {
                    nSectNo.first = rFootnoteInfo.nFootnoteOffset + nFootnoteNo;
                    ++nFootnoteNo;
                    nSectNo.second = rFootnoteInfo.nFootnoteOffset + nFootnoteNoHidden;
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

    if( pTmpRoot && FTNNUM_PAGE == rFootnoteInfo.eNum )
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
    sal_uInt16 nVal;
    const SwSectionNode* pNd = rTextFootnote.GetTextNode().FindSectionNode();
    while( pNd && FTNEND_ATTXTEND_OWNNUMSEQ != ( nVal =
            static_cast<const SwFormatFootnoteEndAtTextEnd&>(pNd->GetSection().GetFormat()->
            GetFormatAttr( nWh )).GetValue() ) &&
            FTNEND_ATTXTEND_OWNNUMANDFMT != nVal )
        pNd = pNd->StartOfSectionNode()->FindSectionNode();

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
        pArr = &aEndSects;
        pNum = &aEndNums;
        nWh = RES_END_AT_TXTEND;
    }
    else
    {
        pArr = &aFootnoteSects;
        pNum = &aFootnoteNums;
        nWh = RES_FTN_AT_TXTEND;
    }

    for( size_t n = pArr->size(); n; )
        if( (*pArr)[ --n ] == &rNd )
        {
            nRet.first = ++((*pNum)[ n ].first);
            nRet.second = ((*pNum)[ n ].second);
            if (!IsFootnoteDeleted(rIDRA, rTextFootnote))
            {
                ++((*pNum)[ n ].second);
            }
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
