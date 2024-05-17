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
#include <ftninfo.hxx>
#include <ftnidx.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentListsAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <poolfmt.hxx>
#include <UndoCore.hxx>
#include <UndoRedline.hxx>
#include <UndoNumbering.hxx>
#include <swundo.hxx>
#include <SwUndoFmt.hxx>
#include <rolbck.hxx>
#include <paratr.hxx>
#include <docary.hxx>
#include <mvsave.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <redline.hxx>
#include <strings.hrc>
#include <SwNodeNum.hxx>
#include <list.hxx>
#include <calbck.hxx>
#include <editeng/lrspitem.hxx>
#include <comphelper/string.hxx>
#include <comphelper/random.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <tools/datetimeutils.hxx>

#include <map>
#include <stdlib.h>

#include <wrtsh.hxx>

namespace {
    void lcl_ResetIndentAttrs(SwDoc *pDoc, const SwPaM &rPam,
            const o3tl::sorted_vector<sal_uInt16> aResetAttrsArray,
            SwRootFrame const*const pLayout)
    {
        // #i114929#
        // On a selection setup a corresponding Point-and-Mark in order to get
        // the indentation attribute reset on all paragraphs touched by the selection
        if ( rPam.HasMark() &&
             rPam.End()->GetNode().GetTextNode() )
        {
            SwPaM aPam( rPam.Start()->GetNode(), 0,
                        rPam.End()->GetNode(), rPam.End()->GetNode().GetTextNode()->Len() );
            pDoc->ResetAttrs( aPam, false, aResetAttrsArray, true, pLayout );
        }
        else
        {
            pDoc->ResetAttrs( rPam, false, aResetAttrsArray, true, pLayout );
        }
    }

    void ExpandPamForParaPropsNodes(SwPaM& rPam, SwRootFrame const*const pLayout)
    {
        if (!pLayout)
            return;

        // ensure that selection from the Shell includes the para-props node
        // to which the attributes should be applied
        if (rPam.GetPoint()->GetNode().IsTextNode())
        {
            rPam.GetPoint()->Assign( *sw::GetParaPropsNode(*pLayout, rPam.GetPoint()->GetNode()) );
        }
        if (rPam.GetMark()->GetNode().IsTextNode())
        {
            rPam.GetMark()->Assign( *sw::GetParaPropsNode(*pLayout, rPam.GetMark()->GetNode()) );
        }
    }
}

static sal_uInt8 GetUpperLvlChg( sal_uInt8 nCurLvl, sal_uInt8 nLevel, sal_uInt16 nMask )
{
    if( 1 < nLevel )
    {
        if( nCurLvl + 1 >= nLevel )
            nCurLvl -= nLevel - 1;
        else
            nCurLvl = 0;
    }
    return static_cast<sal_uInt8>((nMask - 1) & ~(( 1 << nCurLvl ) - 1));
}

void SwDoc::SetOutlineNumRule( const SwNumRule& rRule )
{
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().StartUndo(SwUndoId::OUTLINE_EDIT, nullptr);
        if (mpOutlineRule)
        {
            GetIDocumentUndoRedo().AppendUndo(
                std::make_unique<SwUndoOutlineEdit>(*mpOutlineRule, rRule, *this));
        }
    }

    if( mpOutlineRule )
        (*mpOutlineRule) = rRule;
    else
    {
        mpOutlineRule = new SwNumRule( rRule );

        AddNumRule(mpOutlineRule); // #i36749#
    }

    mpOutlineRule->SetRuleType( OUTLINE_RULE );
    mpOutlineRule->SetName(SwNumRule::GetOutlineRuleName(), getIDocumentListsAccess());

    // assure that the outline numbering rule is an automatic rule
    mpOutlineRule->SetAutoRule( true );

    // test whether the optional CharFormats are defined in this Document
    mpOutlineRule->CheckCharFormats( *this );

    // notify text nodes, which are registered at the outline style, about the
    // changed outline style
    SwNumRule::tTextNodeList aTextNodeList;
    mpOutlineRule->GetTextNodeList( aTextNodeList );
    for ( SwTextNode* pTextNd : aTextNodeList )
    {
        pTextNd->NumRuleChgd();

        // assure that list level corresponds to outline level
        if ( pTextNd->GetTextColl()->IsAssignedToListLevelOfOutlineStyle() &&
             pTextNd->GetAttrListLevel() != pTextNd->GetTextColl()->GetAssignedOutlineStyleLevel() )
        {
            pTextNd->SetAttrListLevel( pTextNd->GetTextColl()->GetAssignedOutlineStyleLevel() );
        }
    }

    PropagateOutlineRule();
    mpOutlineRule->SetInvalidRule(true);
    UpdateNumRule();

    // update if we have foot notes && numbering by chapter
    if( !GetFootnoteIdxs().empty() && FTNNUM_CHAPTER == GetFootnoteInfo().m_eNum )
        GetFootnoteIdxs().UpdateAllFootnote();

    getIDocumentFieldsAccess().UpdateExpFields(nullptr, true);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().EndUndo(SwUndoId::OUTLINE_EDIT, nullptr);
    }

    getIDocumentState().SetModified();
}

void SwDoc::PropagateOutlineRule()
{
    SwNumRule* pMyOutlineRule = GetOutlineNumRule();
    if (!pMyOutlineRule)
        return;

    for (auto pColl : *mpTextFormatCollTable)
    {
        if(pColl->IsAssignedToListLevelOfOutlineStyle())
        {
            // Check only the list style, which is set at the paragraph style
            const SwNumRuleItem & rCollRuleItem = pColl->GetNumRule( false );

            if ( rCollRuleItem.GetValue().isEmpty() )
            {
                SwNumRuleItem aNumItem( pMyOutlineRule->GetName() );
                pColl->SetFormatAttr(aNumItem);
            }
        }
    }
}

// Increase/Decrease
bool SwDoc::OutlineUpDown(const SwPaM& rPam, short nOffset,
        SwRootFrame const*const pLayout)
{
    if( GetNodes().GetOutLineNds().empty() || !nOffset )
        return false;

    // calculate the range
    SwPaM aPam(rPam, nullptr);
    ExpandPamForParaPropsNodes(aPam, pLayout);
    const SwOutlineNodes& rOutlNds = GetNodes().GetOutLineNds();
    SwNode* const pSttNd = &aPam.Start()->GetNode();
    SwNode* const pEndNd = &aPam.End()->GetNode();
    SwOutlineNodes::size_type nSttPos, nEndPos;

    if( !rOutlNds.Seek_Entry( pSttNd, &nSttPos ) &&
        !nSttPos-- )
        // we're not in an "Outline section"
        return false;

    if( rOutlNds.Seek_Entry( pEndNd, &nEndPos ) )
        ++nEndPos;

    // We now have the wanted range in the OutlineNodes array,
    // so check now if we're not invalidating sublevels
    // (stepping over the limits)

    // Here we go:
    // 1. Create the style array:
    SwTextFormatColl* aCollArr[ MAXLEVEL ];
    memset( aCollArr, 0, sizeof( SwTextFormatColl* ) * MAXLEVEL );

    for( auto pTextFormatColl : *mpTextFormatCollTable )
    {
        if (pTextFormatColl->IsAssignedToListLevelOfOutlineStyle())
        {
            const int nLevel = pTextFormatColl->GetAssignedOutlineStyleLevel();
            aCollArr[ nLevel ] = pTextFormatColl;
        }
    }

    int n;

    /* Find the last occupied level (backward). */
    for (n = MAXLEVEL - 1; n > 0; n--)
    {
        if (aCollArr[n] != nullptr)
            break;
    }

    /* If an occupied level is found, choose next level (which IS
       unoccupied) until a valid level is found. If no occupied level
       was found n is 0 and aCollArr[0] is 0. In this case no demoting
       is possible. */
    if (aCollArr[n] != nullptr)
    {
        while (n < MAXLEVEL - 1)
        {
            n++;

            SwTextFormatColl *aTmpColl =
                getIDocumentStylePoolAccess().GetTextCollFromPool(o3tl::narrowing<sal_uInt16>(RES_POOLCOLL_HEADLINE1 + n));

            if( aTmpColl->IsAssignedToListLevelOfOutlineStyle() &&
                aTmpColl->GetAssignedOutlineStyleLevel() == n )
            {
                aCollArr[n] = aTmpColl;
                break;
            }
        }
    }

    /* Find the first occupied level (forward). */
    for (n = 0; n < MAXLEVEL - 1; n++)
    {
        if (aCollArr[n] != nullptr)
            break;
    }

    /* If an occupied level is found, choose previous level (which IS
       unoccupied) until a valid level is found. If no occupied level
       was found n is MAXLEVEL - 1 and aCollArr[MAXLEVEL - 1] is 0. In
       this case no demoting is possible. */
    if (aCollArr[n] != nullptr)
    {
        while (n > 0)
        {
            n--;

            SwTextFormatColl *aTmpColl =
                getIDocumentStylePoolAccess().GetTextCollFromPool(o3tl::narrowing<sal_uInt16>(RES_POOLCOLL_HEADLINE1 + n));

            if( aTmpColl->IsAssignedToListLevelOfOutlineStyle() &&
                aTmpColl->GetAssignedOutlineStyleLevel() == n )
            {
                aCollArr[n] = aTmpColl;
                break;
            }
        }
    }

    /* --> #i13747#

        Build a move table that states from which level to which other level
        an outline will be moved.

        the move table:
        aMoveArr[n] = m: replace aCollArr[n] with aCollArr[m]
    */
    int aMoveArr[MAXLEVEL];
    int nStep; // step size for searching in aCollArr: -1 or 1
    int nNum; // amount of steps for stepping in aCollArr

    if (nOffset < 0)
    {
        nStep = -1;
        nNum = -nOffset;
    }
    else
    {
        nStep = 1;
        nNum = nOffset;
    }

    /* traverse aCollArr */
    for (n = 0; n < MAXLEVEL; n++)
    {
        /* If outline level n has an assigned paragraph style step
           nNum steps forwards (nStep == 1) or backwards (nStep ==
           -1).  One step is to go to the next non-null entry in
           aCollArr in the selected direction. If nNum steps were
           possible write the index of the entry found to aCollArr[n],
           i.e. outline level n will be replaced by outline level
           aCollArr[n].

           If outline level n has no assigned paragraph style
           aMoveArr[n] is set to -1.
        */
        if (aCollArr[n] != nullptr)
        {
            int m = n;
            int nCount = nNum;

            while (nCount > 0 && m + nStep >= 0 && m + nStep < MAXLEVEL)
            {
                m += nStep;

                if (aCollArr[m] != nullptr)
                    nCount--;
            }

            if (nCount == 0)
                aMoveArr[n] = m;
            else
                aMoveArr[n] = -1;
        }
        else
            aMoveArr[n] = -1;
    }

    /* If moving of the outline levels is applicable, i.e. for all
       outline levels occurring in the document there has to be a valid
       target outline level implied by aMoveArr. */
    bool bMoveApplicable = true;
    for (auto i = nSttPos; i < nEndPos; ++i)
    {
        SwTextNode* pTextNd = rOutlNds[ i ]->GetTextNode();
        if (pLayout && !sw::IsParaPropsNode(*pLayout, *pTextNd))
        {
            continue;
        }
        SwTextFormatColl* pColl = pTextNd->GetTextColl();

        if( pColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            const int nLevel = pColl->GetAssignedOutlineStyleLevel();
            if (aMoveArr[nLevel] == -1)
                bMoveApplicable = false;
        }

        // Check on outline level attribute of text node, if text node is
        // not an outline via a to outline style assigned paragraph style.
        else
        {
            const int nNewOutlineLevel = pTextNd->GetAttrOutlineLevel() + nOffset;
            if ( nNewOutlineLevel < 1 || nNewOutlineLevel > MAXLEVEL )
            {
                bMoveApplicable = false;
            }
        }
    }

    if (! bMoveApplicable )
        return false;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().StartUndo(SwUndoId::OUTLINE_LR, nullptr);
        GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoOutlineLeftRight>(aPam, nOffset) );
    }

    // 2. Apply the new style to all Nodes
    for (auto i = nSttPos; i < nEndPos; ++i)
    {
        SwTextNode* pTextNd = rOutlNds[ i ]->GetTextNode();
        if (pLayout && !sw::IsParaPropsNode(*pLayout, *pTextNd))
        {
            continue;
        }
        SwTextFormatColl* pColl = pTextNd->GetTextColl();

        if( pColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            const int nLevel = pColl->GetAssignedOutlineStyleLevel();

            OSL_ENSURE(aMoveArr[nLevel] >= 0,
                "move table: current TextColl not found when building table!");

            if (nLevel < MAXLEVEL && aMoveArr[nLevel] >= 0)
            {
                pColl = aCollArr[ aMoveArr[nLevel] ];

                if (pColl != nullptr)
                    pTextNd->ChgFormatColl( pColl );
            }

        }
        else if( pTextNd->GetAttrOutlineLevel() > 0)
        {
            int nLevel = pTextNd->GetAttrOutlineLevel() + nOffset;
            if( 0 <= nLevel && nLevel <= MAXLEVEL)
                pTextNd->SetAttrOutlineLevel( nLevel );

        }
        // Undo ???
    }
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().EndUndo(SwUndoId::OUTLINE_LR, nullptr);
    }

    ChkCondColls();
    getIDocumentState().SetModified();

    return true;
}

// Move up/down
bool SwDoc::MoveOutlinePara( const SwPaM& rPam, SwOutlineNodes::difference_type nOffset )
{
    // Do not move to special sections in the nodes array
    const SwPosition& rStt = *rPam.Start(),
                    & rEnd = *rPam.End();
    if( GetNodes().GetOutLineNds().empty() || !nOffset ||
        (rStt.GetNodeIndex() < GetNodes().GetEndOfExtras().GetIndex()) ||
        (rEnd.GetNodeIndex() < GetNodes().GetEndOfExtras().GetIndex()))
    {
        return false;
    }

    SwOutlineNodes::size_type nCurrentPos = 0;
    SwNodeIndex aSttRg( rStt.GetNode() ), aEndRg( rEnd.GetNode() );

    int nOutLineLevel = MAXLEVEL;
    SwNode* pSrch = &aSttRg.GetNode();

    if( pSrch->IsTextNode())
        nOutLineLevel = static_cast<sal_uInt8>(pSrch->GetTextNode()->GetAttrOutlineLevel()-1);
    SwNode* pEndSrch = &aEndRg.GetNode();
    if( !GetNodes().GetOutLineNds().Seek_Entry( pSrch, &nCurrentPos ) )
    {
        if( !nCurrentPos )
            return false; // Promoting or demoting before the first outline => no.
        if( --nCurrentPos )
            aSttRg = *GetNodes().GetOutLineNds()[ nCurrentPos ];
        else if( 0 > nOffset )
            return false; // Promoting at the top of document?!
        else
            aSttRg = *GetNodes().GetEndOfContent().StartOfSectionNode();
    }
    SwOutlineNodes::size_type nTmpPos = 0;
    // If the given range ends at an outlined text node we have to decide if it has to be a part of
    // the moving range or not. Normally it will be a sub outline of our chapter
    // and has to be moved, too. But if the chapter ends with a table(or a section end),
    // the next text node will be chosen and this could be the next outline of the same level.
    // The criteria has to be the outline level: sub level => incorporate, same/higher level => no.
    if( GetNodes().GetOutLineNds().Seek_Entry( pEndSrch, &nTmpPos ) )
    {
        if( !pEndSrch->IsTextNode() || pEndSrch == pSrch ||
            nOutLineLevel < pEndSrch->GetTextNode()->GetAttrOutlineLevel()-1 )
            ++nTmpPos; // For sub outlines only!
    }

    aEndRg = nTmpPos < GetNodes().GetOutLineNds().size()
                    ? *GetNodes().GetOutLineNds()[ nTmpPos ]
                    : GetNodes().GetEndOfContent();
    if( nOffset >= 0 )
        nCurrentPos = nTmpPos;
    if( aEndRg == aSttRg )
    {
        OSL_FAIL( "Moving outlines: Surprising selection" );
        ++aEndRg;
    }

    const SwNode* pNd;
    // The following code corrects the range to handle sections (start/end nodes)
    // The range will be extended if the least node before the range is a start node
    // which ends inside the range => The complete section will be moved.
    // The range will be shrunk if the last position is a start node.
    // The range will be shrunk if the last node is an end node which starts before the range.
    --aSttRg;
    while( aSttRg.GetNode().IsStartNode() )
    {
        pNd = aSttRg.GetNode().EndOfSectionNode();
        if( pNd->GetIndex() >= aEndRg.GetIndex() )
            break;
        --aSttRg;
    }
    ++aSttRg;

    --aEndRg;
    while( aEndRg.GetNode().IsStartNode() )
        --aEndRg;

    while( aEndRg.GetNode().IsEndNode() )
    {
        pNd = aEndRg.GetNode().StartOfSectionNode();
        if( pNd->GetIndex() >= aSttRg.GetIndex() )
            break;
        --aEndRg;
    }
    ++aEndRg;

    // calculation of the new position
    if( nOffset < 0 && nCurrentPos < o3tl::make_unsigned(-nOffset) )
        pNd = GetNodes().GetEndOfContent().StartOfSectionNode();
    else if( nCurrentPos + nOffset >= GetNodes().GetOutLineNds().size() )
        pNd = &GetNodes().GetEndOfContent();
    else
        pNd = GetNodes().GetOutLineNds()[ nCurrentPos + nOffset ];

    SwNodeOffset nNewPos = pNd->GetIndex();

    // And now a correction of the insert position if necessary...
    SwNodeIndex aInsertPos( *pNd, -1 );
    while( aInsertPos.GetNode().IsStartNode() )
    {
        // Just before the insert position starts a section:
        // when I'm moving forward I do not want to enter the section,
        // when I'm moving backward I want to stay in the section if I'm already a part of,
        // I want to stay outside if I was outside before.
        if( nOffset < 0 )
        {
            pNd = aInsertPos.GetNode().EndOfSectionNode();
            if( pNd->GetIndex() >= aEndRg.GetIndex() )
                break;
        }
        --aInsertPos;
        --nNewPos;
    }

    if( nOffset >= 0 )
    {
        // When just before the insert position a section ends, it is okay when I'm moving backward
        // because I want to stay outside the section.
        // When moving forward I've to check if I started inside or outside the section
        // because I don't want to enter of leave such a section
        while( aInsertPos.GetNode().IsEndNode() )
        {
            pNd = aInsertPos.GetNode().StartOfSectionNode();
            if( pNd->GetIndex() >= aSttRg.GetIndex() )
                break;
            --aInsertPos;
            --nNewPos;
        }
    }
    // We do not want to move into tables (at the moment)
    ++aInsertPos;
    pNd = &aInsertPos.GetNode();
    if( pNd->IsTableNode() )
        pNd = pNd->StartOfSectionNode();
    if( pNd->FindTableNode() )
        return false;

    OSL_ENSURE( aSttRg.GetIndex() > nNewPos || nNewPos >= aEndRg.GetIndex(),
                "Position lies within Move range" );

    // If a Position inside the special nodes array sections was calculated,
    // set it to document start instead.
    // Sections or Tables at the document start will be pushed backwards.
    nNewPos = std::max( nNewPos, GetNodes().GetEndOfExtras().GetIndex() + SwNodeOffset(2) );

    SwNodeOffset nOffs = nNewPos - ( 0 < nOffset ? aEndRg.GetIndex() : aSttRg.GetIndex());
    SwPaM aPam( aSttRg, aEndRg, SwNodeOffset(0), SwNodeOffset(-1) );
    return MoveParagraph( aPam, nOffs, true );
}

static SwTextNode* lcl_FindOutlineName(const SwOutlineNodes& rOutlNds,
    SwRootFrame const*const pLayout, std::u16string_view aName, bool const bExact)
{
    SwTextNode * pExactButDeleted(nullptr);
    SwTextNode* pSavedNode = nullptr;
    for( auto pOutlNd : rOutlNds )
    {
        SwTextNode* pTextNd = pOutlNd->GetTextNode();
        const OUString sText( pTextNd->GetExpandText(pLayout) );
        if (sText.startsWith(aName))
        {
            if (sText.getLength() == sal_Int32(aName.size()))
            {
                if (pLayout && !sw::IsParaPropsNode(*pLayout, *pTextNd))
                {
                    pExactButDeleted = pTextNd;
                }
                else
                {
                    // Found "exact", set Pos to the Node
                    return pTextNd;
                }
            }
            if (!bExact && !pSavedNode
                && (!pLayout || sw::IsParaPropsNode(*pLayout, *pTextNd)))
            {
                // maybe we just found the text's first part
                pSavedNode = pTextNd;
            }
        }
    }

    return bExact ? pExactButDeleted : pSavedNode;
}

static SwTextNode* lcl_FindOutlineNum(const SwOutlineNodes& rOutlNds,
        OUString& rName, SwRootFrame const*const pLayout)
{
    // Valid numbers are (always just offsets!):
    //  ([Number]+\.)+  (as a regular expression!)
    //  (Number followed by a period, with 5 repetitions)
    //  i.e.: "1.1.", "1.", "1.1.1."
    sal_Int32 nPos = 0;
    std::u16string_view sNum = o3tl::getToken(rName, 0, '.', nPos );
    if( -1 == nPos )
        return nullptr;           // invalid number!

    sal_uInt16 nLevelVal[ MAXLEVEL ];       // numbers of all levels
    memset( nLevelVal, 0, MAXLEVEL * sizeof( nLevelVal[0] ));
    int nLevel = 0;
    std::u16string_view sName( rName );

    while( -1 != nPos )
    {
        sal_uInt16 nVal = 0;
        for( size_t n = 0; n < sNum.size(); ++n )
        {
            const sal_Unicode c {sNum[ n ]};
            if( '0' <= c && c <= '9' )
            {
                nVal *= 10;
                nVal += c - '0';
            }
            else if( nLevel )
                break;                      // "almost" valid number
            else
                return nullptr;             // invalid number!
        }

        if( MAXLEVEL > nLevel )
            nLevelVal[ nLevel++ ] = nVal;

        sName = sName.substr( nPos );
        nPos = 0;
        sNum = o3tl::getToken(sName, 0, '.', nPos );
        // #i4533# without this check all parts delimited by a dot are treated as outline numbers
        if(!comphelper::string::isdigitAsciiString(sNum))
            break;
    }
    rName = sName;      // that's the follow-up text

    // read all levels, so search the document for this outline

    // Without OutlineNodes searching doesn't pay off
    // and we save a crash
    if( rOutlNds.empty() )
        return nullptr;

    // search in the existing outline nodes for the required outline num array
    for( auto pOutlNd : rOutlNds )
    {
        SwTextNode* pNd = pOutlNd->GetTextNode();
        if ( pNd->GetAttrOutlineLevel() == nLevel )
        {
            // #i51089#, #i68289#
            // Assure, that text node has the correct numbering level. Otherwise,
            // its number vector will not fit to the searched level.
            if (pNd->GetNum(pLayout) && pNd->GetActualListLevel() == nLevel - 1)
            {
                const SwNodeNum & rNdNum = *(pNd->GetNum(pLayout));
                SwNumberTree::tNumberVector aLevelVal = rNdNum.GetNumberVector();
                // now compare with the one searched for
                bool bEqual = true;
                nLevel = std::min<int>(nLevel, MAXLEVEL);
                for( int n = 0; n < nLevel; ++n )
                {
                    if ( aLevelVal[n] != nLevelVal[n] )
                    {
                        bEqual = false;
                        break;
                    }
                }
                if (bEqual)
                    return pNd;
            }
            else
            {
                // A text node, which has an outline paragraph style applied and
                // has as hard attribute 'no numbering' set, has an outline level,
                // but no numbering tree node. Thus, consider this situation in
                // the assertion condition.
                OSL_ENSURE( !pNd->GetNumRule(),
                        "<lcl_FindOutlineNum(..)> - text node with outline level and numbering rule, but without numbering tree node. This is a serious defect" );
            }
        }
    }

    return nullptr;
}

// rName can contain a Number and/or the Text.
// First, we try to find the correct Entry via the Number.
// If it exists, we compare the Text to see if it's the right one.
// If that's not the case, we search again via the Text. If it is
// found, we got the right entry. Or else we use the one found by
// searching for the Number.
// If we don't have a Number, we search via the Text only.
bool SwDoc::GotoOutline(SwPosition& rPos, const OUString& rName, SwRootFrame const*const pLayout) const
{
    if( !rName.isEmpty() )
    {
        const SwOutlineNodes& rOutlNds = GetNodes().GetOutLineNds();

        // 1. step: via the Number:
        OUString sName( rName );
        SwTextNode* pNd = ::lcl_FindOutlineNum(rOutlNds, sName, pLayout);
        if ( pNd )
        {
            OUString sExpandedText = pNd->GetExpandText(pLayout);
            //#i4533# leading numbers followed by a dot have been remove while
            //searching for the outline position
            //to compensate this they must be removed from the paragraphs text content, too
            while(!sExpandedText.isEmpty())
            {
                sal_Int32 nPos = 0;
                std::u16string_view sTempNum = o3tl::getToken(sExpandedText, 0, '.', nPos);
                if( sTempNum.empty() || -1 == nPos ||
                    !comphelper::string::isdigitAsciiString(sTempNum))
                    break;
                sExpandedText = sExpandedText.copy(nPos);
            }

            if( sExpandedText != sName )
            {
                SwTextNode *pTmpNd = ::lcl_FindOutlineName(rOutlNds, pLayout, sName, true);
                if ( pTmpNd )             // found via the Name
                {
                    if (pLayout && !sw::IsParaPropsNode(*pLayout, *pTmpNd))
                    {   // found the correct node but it's deleted!
                        return false; // avoid fallback to inexact search
                    }
                    pNd = pTmpNd;
                }
            }
            rPos.Assign(*pNd);
            return true;
        }

        pNd = ::lcl_FindOutlineName(rOutlNds, pLayout, rName, false);
        if ( pNd )
        {
            rPos.Assign(*pNd);
            return true;
        }

        // #i68289# additional search on hyperlink URL without its outline numbering part
        if ( sName != rName )
        {
            pNd = ::lcl_FindOutlineName(rOutlNds, pLayout, sName, false);
            if ( pNd )
            {
                rPos.Assign(*pNd);
                return true;
            }
        }
    }
    return false;
}

static void lcl_ChgNumRule( SwDoc& rDoc, const SwNumRule& rRule )
{
    SwNumRule* pOld = rDoc.FindNumRulePtr( rRule.GetName() );
    if (!pOld) //we cannot proceed without the old NumRule
        return;

    sal_uInt16 nChgFormatLevel = 0;
    sal_uInt16 nMask = 1;

    for ( sal_uInt8 n = 0; n < MAXLEVEL; ++n, nMask <<= 1 )
    {
        const SwNumFormat& rOldFormat = pOld->Get( n ), &rNewFormat = rRule.Get( n );

        if ( rOldFormat != rNewFormat )
        {
            nChgFormatLevel |= nMask;
        }
        else if ( SVX_NUM_NUMBER_NONE > rNewFormat.GetNumberingType()
                  && 1 < rNewFormat.GetIncludeUpperLevels()
                  && 0 != ( nChgFormatLevel & GetUpperLvlChg( n, rNewFormat.GetIncludeUpperLevels(), nMask ) ) )
        {
            nChgFormatLevel |= nMask;
        }
    }

    if( !nChgFormatLevel )         // Nothing has been changed?
    {
        const bool bInvalidateNumRule( pOld->IsContinusNum() != rRule.IsContinusNum() );
        pOld->CheckCharFormats( rDoc );
        pOld->SetContinusNum( rRule.IsContinusNum() );

        if ( bInvalidateNumRule )
        {
            pOld->SetInvalidRule(true);
        }

        return ;
    }

    SwNumRule::tTextNodeList aTextNodeList;
    pOld->GetTextNodeList( aTextNodeList );
    sal_uInt8 nLvl( 0 );
    for ( SwTextNode* pTextNd : aTextNodeList )
    {
        nLvl = static_cast<sal_uInt8>(pTextNd->GetActualListLevel());

        if( nLvl < MAXLEVEL )
        {
            if( nChgFormatLevel & ( 1 << nLvl ))
            {
                pTextNd->NumRuleChgd();
            }
        }
    }

    for ( sal_uInt8 n = 0; n < MAXLEVEL; ++n )
        if ( nChgFormatLevel & ( 1 << n ) )
            pOld->Set( n, rRule.GetNumFormat( n ) );

    pOld->CheckCharFormats( rDoc );
    pOld->SetInvalidRule( true );
    pOld->SetContinusNum( rRule.IsContinusNum() );

    rDoc.UpdateNumRule();
}

OUString SwDoc::SetNumRule( const SwPaM& rPam,
                        const SwNumRule& rRule,
                        SetNumRuleMode eMode,
                        SwRootFrame const*const pLayout,
                        const OUString& sContinuedListId,
                        SvxTextLeftMarginItem const*const pTextLeftMarginToPropagate,
                        SvxFirstLineIndentItem const*const pFirstLineIndentToPropagate)
{
    OUString sListId;

    SwPaM aPam(rPam, nullptr);
    ExpandPamForParaPropsNodes(aPam, pLayout);

    SwUndoInsNum * pUndo = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        // Start/End for attributes!
        GetIDocumentUndoRedo().StartUndo( SwUndoId::INSNUM, nullptr );
        pUndo = new SwUndoInsNum( aPam, rRule );
        GetIDocumentUndoRedo().AppendUndo(std::unique_ptr<SwUndo>(pUndo));
    }

    SwNumRule* pNewOrChangedNumRule = FindNumRulePtr( rRule.GetName() );
    bool bNewNumRuleCreated = false;
    if ( pNewOrChangedNumRule == nullptr )
    {
        // create new numbering rule based on given one
        pNewOrChangedNumRule = ( *mpNumRuleTable )[MakeNumRule( rRule.GetName(), &rRule )];
        bNewNumRuleCreated = true;
    }
    else if ( rRule != *pNewOrChangedNumRule )
    {
        // change existing numbering rule
        if (pUndo)
        {
            pUndo->SaveOldNumRule( *pNewOrChangedNumRule );
        }
        ::lcl_ChgNumRule( *this, rRule );
        if (pUndo)
        {
            pUndo->SetLRSpaceEndPos();
        }
    }

    if (!(eMode & SetNumRuleMode::DontSetItem))
    {
        if (eMode & SetNumRuleMode::CreateNewList)
        {
            if ( bNewNumRuleCreated )
            {
                // apply list id of list, which has been created for the new list style
                sListId = pNewOrChangedNumRule->GetDefaultListId();
            }
            else
            {
                // create new list and apply its list id
                const SwList* pNewList = getIDocumentListsAccess().createList( OUString(), pNewOrChangedNumRule->GetName() );
                OSL_ENSURE( pNewList,
                        "<SwDoc::SetNumRule(..)> - could not create new list. Serious defect." );
                sListId = pNewList->GetListId();
            }
        }
        else if ( !sContinuedListId.isEmpty() )
        {
            // apply given list id
            sListId = sContinuedListId;
        }
        if (!sListId.isEmpty())
        {
            getIDocumentContentOperations().InsertPoolItem(aPam,
                SfxStringItem(RES_PARATR_LIST_ID, sListId),
                SetAttrMode::DEFAULT, pLayout);
        }
    }

    if (!aPam.HasMark())
    {
        SwTextNode * pTextNd = aPam.GetPoint()->GetNode().GetTextNode();
        // robust code: consider case that the PaM doesn't denote a text node - e.g. it denotes a graphic node
        if ( pTextNd != nullptr )
        {
            assert(!pLayout || sw::IsParaPropsNode(*pLayout, *pTextNd));
            SwNumRule * pRule = pTextNd->GetNumRule();

            if (pRule && pRule->GetName() == pNewOrChangedNumRule->GetName())
            {
                eMode |= SetNumRuleMode::DontSetItem;
                if ( !pTextNd->IsInList() )
                {
                    pTextNd->AddToList();
                }
            }
            // Only clear numbering attribute at text node, if at paragraph
            // style the new numbering rule is found.
            else if ( !pRule )
            {
                SwTextFormatColl* pColl = pTextNd->GetTextColl();
                if ( pColl )
                {
                    SwNumRule* pCollRule = FindNumRulePtr(pColl->GetNumRule().GetValue());
                    if ( pCollRule && pCollRule->GetName() == pNewOrChangedNumRule->GetName() )
                    {
                        pTextNd->ResetAttr( RES_PARATR_NUMRULE );
                        eMode |= SetNumRuleMode::DontSetItem;
                    }
                }
            }
        }
    }

    if (!(eMode & SetNumRuleMode::DontSetItem))
    {
        if (eMode & SetNumRuleMode::DontSetIfAlreadyApplied)
        {
            for (SwNodeIndex i = aPam.Start()->nNode; i <= aPam.End()->nNode; ++i)
            {
                if (SwTextNode const*const pNode = i.GetNode().GetTextNode())
                {
                    if (pNode->GetNumRule(true) != pNewOrChangedNumRule)
                    {
                        // only apply if it doesn't already have it - to
                        // avoid overriding indents from style
                        SwPaM const temp(*pNode, 0, *pNode, pNode->Len());
                        getIDocumentContentOperations().InsertPoolItem(temp,
                                SwNumRuleItem(pNewOrChangedNumRule->GetName()),
                                SetAttrMode::DEFAULT, pLayout);
                        // apply provided margins to get visually same result
                        if (pTextLeftMarginToPropagate)
                        {
                            getIDocumentContentOperations().InsertPoolItem(temp,
                                    *pTextLeftMarginToPropagate,
                                    SetAttrMode::DEFAULT, pLayout);
                        }
                        if (pFirstLineIndentToPropagate)
                        {
                            getIDocumentContentOperations().InsertPoolItem(temp,
                                    *pFirstLineIndentToPropagate,
                                    SetAttrMode::DEFAULT, pLayout);
                        }
                    }
                }
            }
        }
        else
        {
            getIDocumentContentOperations().InsertPoolItem(aPam,
                    SwNumRuleItem(pNewOrChangedNumRule->GetName()),
                    SetAttrMode::DEFAULT, pLayout);
            if (pTextLeftMarginToPropagate)
            {
                getIDocumentContentOperations().InsertPoolItem(aPam,
                        *pTextLeftMarginToPropagate,
                        SetAttrMode::DEFAULT, pLayout);
            }
            if (pFirstLineIndentToPropagate)
            {
                getIDocumentContentOperations().InsertPoolItem(aPam,
                        *pFirstLineIndentToPropagate,
                        SetAttrMode::DEFAULT, pLayout);
            }
        }
    }

    if ((eMode & SetNumRuleMode::ResetIndentAttrs)
         && pNewOrChangedNumRule->Get( 0 ).GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        const o3tl::sorted_vector<sal_uInt16> attrs{ RES_MARGIN_FIRSTLINE, RES_MARGIN_TEXTLEFT, RES_MARGIN_RIGHT };
        ::lcl_ResetIndentAttrs(this, aPam, attrs, pLayout);
    }

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().EndUndo( SwUndoId::INSNUM, nullptr );
    }

    getIDocumentState().SetModified();

    return sListId;
}

void SwDoc::SetCounted(const SwPaM & rPam, bool bCounted,
        SwRootFrame const*const pLayout)
{
    if ( bCounted )
    {
        const o3tl::sorted_vector<sal_uInt16> attrs{ RES_PARATR_LIST_ISCOUNTED };
        ::lcl_ResetIndentAttrs(this, rPam, attrs, pLayout);
    }
    else
    {
        getIDocumentContentOperations().InsertPoolItem(rPam,
                SfxBoolItem(RES_PARATR_LIST_ISCOUNTED, false),
                SetAttrMode::DEFAULT, pLayout);
    }
}

void SwDoc::SetNumRuleStart( const SwPosition& rPos, bool bFlag )
{
    SwTextNode* pTextNd = rPos.GetNode().GetTextNode();

    if (!pTextNd)
        return;

    const SwNumRule* pRule = pTextNd->GetNumRule();
    if( pRule && !bFlag != !pTextNd->IsListRestart())
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo(
                std::make_unique<SwUndoNumRuleStart>(rPos, bFlag) );
        }

        pTextNd->SetListRestart(bFlag);

        getIDocumentState().SetModified();
    }
}

void SwDoc::SetNodeNumStart( const SwPosition& rPos, sal_uInt16 nStt )
{
    SwTextNode* pTextNd = rPos.GetNode().GetTextNode();

    if (!pTextNd)
        return;

    if ( !pTextNd->HasAttrListRestartValue() ||
         pTextNd->GetAttrListRestartValue() != nStt )
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo(
                std::make_unique<SwUndoNumRuleStart>(rPos, nStt) );
        }
        pTextNd->SetAttrListRestartValue( nStt );

        getIDocumentState().SetModified();
    }
}

// We can only delete if the Rule is unused!
bool SwDoc::DelNumRule( const OUString& rName, bool bBroadcast )
{
    sal_uInt16 nPos = FindNumRule( rName );

    if (nPos == USHRT_MAX)
        return false;

    if ( (*mpNumRuleTable)[ nPos ] == GetOutlineNumRule() )
    {
        OSL_FAIL( "<SwDoc::DelNumRule(..)> - No deletion of outline list style. This is serious defect" );
        return false;
    }

    if( !IsUsed( *(*mpNumRuleTable)[ nPos ] ))
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo(
                std::make_unique<SwUndoNumruleDelete>(*(*mpNumRuleTable)[nPos], *this));
        }

        if (bBroadcast)
            BroadcastStyleOperation(rName, SfxStyleFamily::Pseudo,
                                    SfxHintId::StyleSheetErased);

        getIDocumentListsAccess().deleteListForListStyle( rName );
        getIDocumentListsAccess().deleteListsByDefaultListStyle( rName );
        // #i34097# DeleteAndDestroy deletes rName if
        // rName is directly taken from the numrule.
        const OUString aTmpName( rName );
        delete (*mpNumRuleTable)[ nPos ];
        mpNumRuleTable->erase( mpNumRuleTable->begin() + nPos );
        maNumRuleMap.erase(aTmpName);

        getIDocumentState().SetModified();
        return true;
    }
    return false;
}

void SwDoc::ChgNumRuleFormats( const SwNumRule& rRule )
{
    SwNumRule* pRule = FindNumRulePtr( rRule.GetName() );
    if( !pRule )
        return;

    SwUndoInsNum* pUndo = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoInsNum( *pRule, rRule, *this );
        pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo( std::unique_ptr<SwUndo>(pUndo) );
    }
    ::lcl_ChgNumRule( *this, rRule );
    if (pUndo)
    {
        pUndo->SetLRSpaceEndPos();
    }

    getIDocumentState().SetModified();
}

bool SwDoc::RenameNumRule(const OUString & rOldName, const OUString & rNewName,
                              bool bBroadcast)
{
    assert(!FindNumRulePtr(rNewName));

    bool bResult = false;
    SwNumRule * pNumRule = FindNumRulePtr(rOldName);

    if (pNumRule)
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo(
                std::make_unique<SwUndoNumruleRename>(rOldName, rNewName, *this));
        }

        SwNumRule::tTextNodeList aTextNodeList;
        pNumRule->GetTextNodeList( aTextNodeList );

        pNumRule->SetName( rNewName, getIDocumentListsAccess() );

        SwNumRuleItem aItem(rNewName);

        for ( SwTextNode* pTextNd : aTextNodeList )
        {
            pTextNd->SetAttr(aItem);
        }

        bResult = true;

        if (bBroadcast)
            BroadcastStyleOperation(rOldName, SfxStyleFamily::Pseudo,
                                    SfxHintId::StyleSheetModified);
    }

    return bResult;
}

void SwDoc::StopNumRuleAnimations( const OutputDevice* pOut )
{
    for( sal_uInt16 n = GetNumRuleTable().size(); n; )
    {
        SwNumRule::tTextNodeList aTextNodeList;
        GetNumRuleTable()[ --n ]->GetTextNodeList( aTextNodeList );
        for ( SwTextNode* pTNd : aTextNodeList )
        {
            SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*pTNd);
            for(SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
                if (pFrame->HasAnimation() &&
                    (!pFrame->GetMergedPara() || pFrame->GetMergedPara()->pParaPropsNode == pTNd))
                {
                    pFrame->StopAnimation( pOut );
                }
        }
    }
}

void SwDoc::ReplaceNumRule( const SwPosition& rPos,
                            const OUString& rOldRule, const OUString& rNewRule )
{
    SwNumRule *pOldRule = FindNumRulePtr( rOldRule ),
              *pNewRule = FindNumRulePtr( rNewRule );
    if( !pOldRule || !pNewRule || pOldRule == pNewRule )
        return;

    SwUndoInsNum* pUndo = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        // Start/End for attributes!
        GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );
        pUndo = new SwUndoInsNum( rPos, *pNewRule, rOldRule );
        GetIDocumentUndoRedo().AppendUndo(std::unique_ptr<SwUndo>(pUndo));
    }

    SwNumRule::tTextNodeList aTextNodeList;
    pOldRule->GetTextNodeList( aTextNodeList );
    if ( !aTextNodeList.empty() )
    {
        SwRegHistory aRegH( pUndo ? pUndo->GetHistory() : nullptr );

        const SwTextNode* pGivenTextNode = rPos.GetNode().GetTextNode();
        SwNumRuleItem aRule( rNewRule );
        for ( SwTextNode* pTextNd : aTextNodeList )
        {
            if ( pGivenTextNode &&
                 pGivenTextNode->GetListId() == pTextNd->GetListId() )
            {
                aRegH.RegisterInModify( pTextNd, *pTextNd );

                pTextNd->SetAttr( aRule );
                pTextNd->NumRuleChgd();
            }
        }
        GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
        getIDocumentState().SetModified();
    }
}

namespace
{
    struct ListStyleData
    {
        SwNumRule* pReplaceNumRule;
        bool bCreateNewList;
        OUString sListId;

        ListStyleData()
            : pReplaceNumRule( nullptr ),
              bCreateNewList( false )
        {}
    };
}

void SwDoc::MakeUniqueNumRules(const SwPaM & rPaM)
{
    OSL_ENSURE( &rPaM.GetDoc() == this, "need same doc" );

    std::map<SwNumRule *, ListStyleData> aMyNumRuleMap;

    bool bFirst = true;

    const SwNodeOffset nStt = rPaM.Start()->GetNodeIndex();
    const SwNodeOffset nEnd = rPaM.End()->GetNodeIndex();
    for (SwNodeOffset n = nStt; n <= nEnd; n++)
    {
        SwTextNode * pCNd = GetNodes()[n]->GetTextNode();

        if (pCNd)
        {
            SwNumRule * pRule = pCNd->GetNumRule();

            if (pRule && pRule->IsAutoRule() && ! pRule->IsOutlineRule())
            {
                ListStyleData aListStyleData = aMyNumRuleMap[pRule];

                if ( aListStyleData.pReplaceNumRule == nullptr )
                {
                    if (bFirst)
                    {
                        SwPosition aPos(*pCNd);
                        aListStyleData.pReplaceNumRule =
                            const_cast<SwNumRule *>
                            (SearchNumRule( aPos, false, pCNd->HasNumber(),
                                            false, 0,
                                    aListStyleData.sListId, nullptr, true ));
                    }

                    if ( aListStyleData.pReplaceNumRule == nullptr )
                    {
                        aListStyleData.pReplaceNumRule = new SwNumRule(*pRule);
                        aListStyleData.pReplaceNumRule->SetName( GetUniqueNumRuleName(), getIDocumentListsAccess() );
                        aListStyleData.bCreateNewList = true;
                    }

                    aMyNumRuleMap[pRule] = aListStyleData;
                }

                SwPaM aPam(*pCNd);

                SetNumRule( aPam,
                            *aListStyleData.pReplaceNumRule,
                            aListStyleData.bCreateNewList ? SetNumRuleMode::CreateNewList : SetNumRuleMode::Default,
                            nullptr,
                            aListStyleData.sListId );
                if ( aListStyleData.bCreateNewList )
                {
                    aListStyleData.bCreateNewList = false;
                    aListStyleData.sListId = pCNd->GetListId();
                    aMyNumRuleMap[pRule] = aListStyleData;
                }

                bFirst = false;
            }
        }
    }
}

bool SwDoc::NoNum( const SwPaM& rPam )
{

    bool bRet = getIDocumentContentOperations().SplitNode( *rPam.GetPoint(), false );
    // Do we actually use Numbering at all?
    if( bRet )
    {
        // Set NoNum and Update
        SwTextNode* pNd = rPam.GetPoint()->GetNode().GetTextNode();
        const SwNumRule* pRule = pNd->GetNumRule();
        if( pRule )
        {
            pNd->SetCountedInList(false);

            getIDocumentState().SetModified();
        }
        else
            bRet = false;   // no Numbering or just always true?
    }
    return bRet;
}

void SwDoc::DelNumRules(const SwPaM& rPam, SwRootFrame const*const pLayout)
{
    SwPaM aPam(rPam, nullptr);
    ExpandPamForParaPropsNodes(aPam, pLayout);
    SwNodeOffset nStt = aPam.Start()->GetNodeIndex();
    SwNodeOffset const nEnd = aPam.End()->GetNodeIndex();

    SwUndoDelNum* pUndo;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoDelNum( aPam );
        GetIDocumentUndoRedo().AppendUndo(std::unique_ptr<SwUndo>(pUndo));
    }
    else
        pUndo = nullptr;

    SwRegHistory aRegH( pUndo ? pUndo->GetHistory() : nullptr );

    SwNumRuleItem aEmptyRule;
    const SwNode* pOutlNd = nullptr;
    for( ; nStt <= nEnd; ++nStt )
    {
        SwTextNode* pTNd = GetNodes()[ nStt ]->GetTextNode();
        if (pLayout && pTNd)
        {
            pTNd = sw::GetParaPropsNode(*pLayout, *pTNd);
        }
        SwNumRule* pNumRuleOfTextNode = pTNd ? pTNd->GetNumRule() : nullptr;
        if ( pTNd && pNumRuleOfTextNode )
        {
            // recognize changes of attribute for undo
            aRegH.RegisterInModify( pTNd, *pTNd );

            if( pUndo )
                pUndo->AddNode( *pTNd );

            // directly set list style attribute is reset, otherwise empty
            // list style is applied
            const SfxItemSet* pAttrSet = pTNd->GetpSwAttrSet();
            if ( pAttrSet &&
                 pAttrSet->GetItemState( RES_PARATR_NUMRULE, false ) == SfxItemState::SET )
                pTNd->ResetAttr( RES_PARATR_NUMRULE );
            else
                pTNd->SetAttr( aEmptyRule );

            pTNd->ResetAttr( RES_PARATR_LIST_ID );
            pTNd->ResetAttr( RES_PARATR_LIST_LEVEL );
            pTNd->ResetAttr( RES_PARATR_LIST_ISRESTART );
            pTNd->ResetAttr( RES_PARATR_LIST_RESTARTVALUE );
            pTNd->ResetAttr( RES_PARATR_LIST_ISCOUNTED );

            if( RES_CONDTXTFMTCOLL == pTNd->GetFormatColl()->Which() )
            {
                pTNd->ChkCondColl();
            }
            else if( !pOutlNd &&
                     static_cast<SwTextFormatColl*>(pTNd->GetFormatColl())->IsAssignedToListLevelOfOutlineStyle() )
            {
                pOutlNd = pTNd;
            }
        }
    }

    // Finally, update all
    UpdateNumRule();

    if( pOutlNd )
        GetNodes().UpdateOutlineIdx( *pOutlNd );
}

void SwDoc::InvalidateNumRules()
{
    for (size_t n = 0; n < mpNumRuleTable->size(); ++n)
        (*mpNumRuleTable)[n]->SetInvalidRule(true);
}

// To the next/preceding Bullet at the same Level
static bool lcl_IsNumOk( sal_uInt8 nSrchNum, sal_uInt8& rLower, sal_uInt8& rUpper,
                    bool bOverUpper, sal_uInt8 nNumber )
{
    OSL_ENSURE( nNumber < MAXLEVEL,
            "<lcl_IsNumOk(..)> - misusage of method" );

    bool bRet = false;
    {
        if( bOverUpper ? nSrchNum == nNumber : nSrchNum >= nNumber )
            bRet = true;
        else if( nNumber > rLower )
            rLower = nNumber;
        else if( nNumber < rUpper )
            rUpper = nNumber;
    }
    return bRet;
}

static bool lcl_IsValidPrevNextNumNode( const SwNodeIndex& rIdx )
{
    bool bRet = false;
    const SwNode& rNd = rIdx.GetNode();
    switch( rNd.GetNodeType() )
    {
    case SwNodeType::End:
        bRet = SwTableBoxStartNode == rNd.StartOfSectionNode()->GetStartNodeType() ||
                rNd.StartOfSectionNode()->IsSectionNode();
        break;

    case SwNodeType::Start:
        bRet = SwTableBoxStartNode == static_cast<const SwStartNode&>(rNd).GetStartNodeType();
        break;

    case SwNodeType::Section:            // that one's valid, so proceed
        bRet = true;
        break;

    default: break;
    }
    return bRet;
}

namespace sw {

void
GotoPrevLayoutTextFrame(SwNodeIndex & rIndex, SwRootFrame const*const pLayout)
{
    if (pLayout && pLayout->HasMergedParas())
    {
        if (rIndex.GetNode().IsTextNode())
        {
            if (rIndex.GetNode().GetRedlineMergeFlag() != SwNode::Merge::None)
            {
                // not a tracked row deletion in Hide Changes mode
                if (SwContentFrame* pFrame = rIndex.GetNode().GetTextNode()->getLayoutFrame(pLayout))
                {
                    if (sw::MergedPara* pMerged = static_cast<SwTextFrame*>(pFrame)->GetMergedPara())
                    {
                        rIndex = pMerged->pFirstNode->GetIndex();
                    }
                }
            }
        }
        else if (rIndex.GetNode().IsEndNode())
        {
            if (rIndex.GetNode().GetRedlineMergeFlag() == SwNode::Merge::Hidden)
            {
                rIndex = *rIndex.GetNode().StartOfSectionNode();
                assert(rIndex.GetNode().IsTableNode());
            }
        }
    }
    --rIndex;
    if (pLayout && rIndex.GetNode().IsTextNode())
    {
        rIndex = *sw::GetParaPropsNode(*pLayout, *rIndex.GetNode().GetTextNode());
    }
}

void
GotoNextLayoutTextFrame(SwNodeIndex & rIndex, SwRootFrame const*const pLayout)
{
    if (pLayout && pLayout->HasMergedParas())
    {
        if (rIndex.GetNode().IsTextNode())
        {
            if (rIndex.GetNode().GetRedlineMergeFlag() != SwNode::Merge::None)
            {
                if (SwContentFrame* pFrame = rIndex.GetNode().GetTextNode()->getLayoutFrame(pLayout))
                {
                    if (sw::MergedPara* pMerged = static_cast<SwTextFrame*>(pFrame)->GetMergedPara())
                    {
                        rIndex = pMerged->pLastNode->GetIndex();
                    }
                }
            }
        }
        else if (rIndex.GetNode().IsTableNode())
        {
            if (rIndex.GetNode().GetRedlineMergeFlag() == SwNode::Merge::Hidden)
            {
                rIndex = *rIndex.GetNode().EndOfSectionNode();
            }
        }
    }
    ++rIndex;
    if (pLayout && rIndex.GetNode().IsTextNode())
    {
        rIndex = *sw::GetParaPropsNode(*pLayout, *rIndex.GetNode().GetTextNode());
    }
}

} // namespace sw

static bool lcl_GotoNextPrevNum( SwPosition& rPos, bool bNext,
        bool bOverUpper, sal_uInt8* pUpper, sal_uInt8* pLower,
        SwRootFrame const*const pLayout)
{
    const SwTextNode* pNd = rPos.GetNode().GetTextNode();
    if (pNd && pLayout)
    {
        pNd = sw::GetParaPropsNode(*pLayout, *pNd);
    }
    if( !pNd || nullptr == pNd->GetNumRule() )
        return false;

    sal_uInt8 nSrchNum = static_cast<sal_uInt8>(pNd->GetActualListLevel());

    SwNodeIndex aIdx( rPos.GetNode() );
    if( ! pNd->IsCountedInList() )
    {
        bool bError = false;
        do {
            sw::GotoPrevLayoutTextFrame(aIdx, pLayout);
            if( aIdx.GetNode().IsTextNode() )
            {
                pNd = aIdx.GetNode().GetTextNode();
                const SwNumRule* pRule = pNd->GetNumRule();

                if( pRule  )
                {
                    sal_uInt8 nTmpNum = static_cast<sal_uInt8>(pNd->GetActualListLevel());
                    if( pNd->IsCountedInList() || (nTmpNum < nSrchNum ) )
                        break;      // found it!
                }
                else
                    bError = true;
            }
            else
                bError = !lcl_IsValidPrevNextNumNode( aIdx );

        } while( !bError );
        if( bError )
            return false;
    }

    sal_uInt8 nLower = nSrchNum, nUpper = nSrchNum;
    bool bRet = false;

    const SwTextNode* pLast;
    if( bNext )
    {
        sw::GotoNextLayoutTextFrame(aIdx, pLayout);
        pLast = pNd;
    }
    else
    {
        sw::GotoPrevLayoutTextFrame(aIdx, pLayout);
        pLast = nullptr;
    }

    while( bNext ? ( aIdx.GetIndex() < aIdx.GetNodes().Count() - 1 )
                 : aIdx.GetIndex() != SwNodeOffset(0) )
    {
        if( aIdx.GetNode().IsTextNode() )
        {
            pNd = aIdx.GetNode().GetTextNode();
            const SwNumRule* pRule = pNd->GetNumRule();
            if( pRule )
            {
                if( ::lcl_IsNumOk( nSrchNum, nLower, nUpper, bOverUpper,
                                    static_cast<sal_uInt8>(pNd->GetActualListLevel()) ))
                {
                    rPos.Assign(aIdx);
                    bRet = true;
                    break;
                }
                else
                    pLast = pNd;
            }
            else
                break;
        }
        else if( !lcl_IsValidPrevNextNumNode( aIdx ))
            break;

        if( bNext )
            sw::GotoNextLayoutTextFrame(aIdx, pLayout);
        else
            sw::GotoPrevLayoutTextFrame(aIdx, pLayout);
    }

    if( !bRet && !bOverUpper && pLast )     // do not iterate over higher numbers, but still to the end
    {
        if( bNext )
            rPos.Assign(aIdx);
        else
            rPos.Assign( *pLast );
        bRet = true;
    }

    if( bRet )
    {
        if( pUpper )
            *pUpper = nUpper;
        if( pLower )
            *pLower = nLower;
    }
    return bRet;
}

bool SwDoc::GotoNextNum(SwPosition& rPos, SwRootFrame const*const pLayout,
        bool bOverUpper, sal_uInt8* pUpper, sal_uInt8* pLower)
{
    return ::lcl_GotoNextPrevNum(rPos, true, bOverUpper, pUpper, pLower, pLayout);
}

const SwNumRule *  SwDoc::SearchNumRule(const SwPosition & rPos,
                                        const bool bForward,
                                        const bool bNum,
                                        const bool bOutline,
                                        int nNonEmptyAllowed,
                                        OUString& sListId,
                                        SwRootFrame const* pLayout,
                                        const bool bInvestigateStartNode,
                                        SvxTextLeftMarginItem const** o_ppTextLeftMargin,
                                        SvxFirstLineIndentItem const** o_ppFirstLineIndent)
{
    const SwNumRule * pResult = nullptr;
    SwTextNode * pTextNd = rPos.GetNode().GetTextNode();
    if (pLayout)
    {
        pTextNd = sw::GetParaPropsNode(*pLayout, rPos.GetNode());
    }
    SwNode * pStartFromNode = pTextNd;

    if (pTextNd)
    {
        SwNodeIndex aIdx(rPos.GetNode());

        // - the start node has also been investigated, if requested.
        const SwNode * pNode = nullptr;
        do
        {
            if ( !bInvestigateStartNode )
            {
                if (bForward)
                    sw::GotoNextLayoutTextFrame(aIdx, pLayout);
                else
                    sw::GotoPrevLayoutTextFrame(aIdx, pLayout);
            }

            if (aIdx.GetNode().IsTextNode())
            {
                pTextNd = aIdx.GetNode().GetTextNode();

                const SwNumRule * pNumRule = pTextNd->GetNumRule();
                if (pNumRule)
                {
                    if ( ( pNumRule->IsOutlineRule() == bOutline ) &&
                         ( ( bNum && pNumRule->Get(0).IsEnumeration()) ||
                           ( !bNum && pNumRule->Get(0).IsItemize() ) ) ) // #i22362#, #i29560#
                    {
                        pResult = pNumRule;
                        // provide also the list id, to which the text node belongs.
                        sListId = pTextNd->GetListId();
                        // also get the margins that override the numrule
                        int const nListLevel{pTextNd->GetActualListLevel()};
                        if ((o_ppTextLeftMargin || o_ppFirstLineIndent)
                            && 0 <= nListLevel
                            && pNumRule->Get(o3tl::narrowing<sal_uInt16>(nListLevel))
                                .GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT)
                        {
                            ::sw::ListLevelIndents const indents{pTextNd->AreListLevelIndentsApplicable()};
                            if (!(indents & ::sw::ListLevelIndents::LeftMargin)
                                && o_ppTextLeftMargin)
                            {
                                *o_ppTextLeftMargin = &pTextNd->SwContentNode::GetAttr(RES_MARGIN_TEXTLEFT);
                            }
                            if (!(indents & ::sw::ListLevelIndents::FirstLine)
                                && o_ppFirstLineIndent)
                            {
                                *o_ppFirstLineIndent = &pTextNd->SwContentNode::GetAttr(RES_MARGIN_FIRSTLINE);
                            }
                        }
                    }

                    break;
                }
                else if (pTextNd->Len() > 0 || nullptr != pTextNd->GetNumRule())
                {
                    if (nNonEmptyAllowed == 0)
                        break;

                    nNonEmptyAllowed--;

                    if (nNonEmptyAllowed < 0)
                        nNonEmptyAllowed = -1;
                }
            }

            if ( bInvestigateStartNode )
            {
                if (bForward)
                    sw::GotoNextLayoutTextFrame(aIdx, pLayout);
                else
                    sw::GotoPrevLayoutTextFrame(aIdx, pLayout);
            }

            pNode = &aIdx.GetNode();
        }
        while (pNode != GetNodes().DocumentSectionStartNode(pStartFromNode) &&
                 pNode != GetNodes().DocumentSectionEndNode(pStartFromNode));
    }

    return pResult;
}

bool SwDoc::GotoPrevNum(SwPosition& rPos, SwRootFrame const*const pLayout,
        bool bOverUpper)
{
    return ::lcl_GotoNextPrevNum(rPos, false, bOverUpper, nullptr, nullptr, pLayout);
}

bool SwDoc::NumUpDown(const SwPaM& rPam, bool bDown, SwRootFrame const*const pLayout)
{
    SwPaM aPam(rPam, nullptr);
    ExpandPamForParaPropsNodes(aPam, pLayout);
    SwNodeOffset nStt = aPam.Start()->GetNodeIndex();
    SwNodeOffset const nEnd = aPam.End()->GetNodeIndex();

    // -> outline nodes are promoted or demoted differently
    bool bOnlyOutline = true;
    bool bOnlyNonOutline = true;
    for (SwNodeOffset n = nStt; n <= nEnd; n++)
    {
        SwTextNode * pTextNd = GetNodes()[n]->GetTextNode();

        if (pTextNd)
        {
            if (pLayout)
            {
                pTextNd = sw::GetParaPropsNode(*pLayout, *pTextNd);
            }
            SwNumRule * pRule = pTextNd->GetNumRule();

            if (pRule)
            {
                if (pRule->IsOutlineRule())
                    bOnlyNonOutline = false;
                else
                    bOnlyOutline = false;
            }
        }
    }

    bool bRet = true;
    sal_Int8 nDiff = bDown ? 1 : -1;

    if (bOnlyOutline)
        bRet = OutlineUpDown(rPam, nDiff, pLayout);
    else if (bOnlyNonOutline)
    {
        /* #i24560#
        Only promote or demote if all selected paragraphs are
        promotable resp. demotable.
        */
        for (SwNodeOffset nTmp = nStt; nTmp <= nEnd; ++nTmp)
        {
            SwTextNode* pTNd = GetNodes()[ nTmp ]->GetTextNode();

            // Make code robust: consider case that the node doesn't denote a
            // text node.
            if ( pTNd )
            {
                if (pLayout)
                {
                    pTNd = sw::GetParaPropsNode(*pLayout, *pTNd);
                }

                SwNumRule * pRule = pTNd->GetNumRule();

                if (pRule)
                {
                    sal_uInt8 nLevel = static_cast<sal_uInt8>(pTNd->GetActualListLevel());
                    if( (-1 == nDiff && 0 >= nLevel) ||
                        (1 == nDiff && MAXLEVEL - 1 <= nLevel))
                        bRet = false;
                }
            }
        }

        if( bRet )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoNumUpDown>(aPam, nDiff) );
            }

            SwTextNode* pPrev = nullptr;
            for(SwNodeOffset nTmp = nStt; nTmp <= nEnd; ++nTmp )
            {
                SwTextNode* pTNd = GetNodes()[ nTmp ]->GetTextNode();

                if( pTNd)
                {
                    if (pLayout)
                    {
                        pTNd = sw::GetParaPropsNode(*pLayout, *pTNd);
                        if (pTNd == pPrev)
                        {
                            continue;
                        }
                        pPrev = pTNd;
                    }

                    SwNumRule * pRule = pTNd->GetNumRule();

                    if (pRule)
                    {
                        sal_uInt8 nLevel = static_cast<sal_uInt8>(pTNd->GetActualListLevel());
                        nLevel = nLevel + nDiff;

                        pTNd->SetAttrListLevel(nLevel);
                    }
                }
            }

            ChkCondColls();
            getIDocumentState().SetModified();
        }
    }

    return bRet;
}

// this function doesn't contain any numbering-related code, but it is
// primarily called to move numbering-relevant paragraphs around, hence
// it will expand its selection to include full SwTextFrames.
bool SwDoc::MoveParagraph(SwPaM& rPam, SwNodeOffset nOffset, bool const bIsOutlMv)
{
    MakeAllOutlineContentTemporarilyVisible a(this);

    // sw_redlinehide: as long as a layout with Hide mode exists, only
    // move nodes that have merged frames *completely*
    SwRootFrame const* pLayout(nullptr);
    for (SwRootFrame const*const pLay : GetAllLayouts())
    {
        if (pLay->HasMergedParas())
        {
            pLayout = pLay;
        }
    }
    if (pLayout)
    {
        std::pair<SwTextNode *, SwTextNode *> nodes(
            sw::GetFirstAndLastNode(*pLayout, rPam.Start()->GetNode()));
        if (nodes.first && nodes.first != &rPam.Start()->GetNode())
        {
            assert(nodes.second);
            if (nOffset < SwNodeOffset(0))
            {
                nOffset += rPam.Start()->GetNodeIndex() - nodes.first->GetIndex();
                if (SwNodeOffset(0) <= nOffset)   // hack: there are callers that know what
                {                   // node they want; those should never need
                    nOffset = SwNodeOffset(-1);   // this; other callers just pass in -1
                }                   // and those should still move
            }
            if (!rPam.HasMark())
            {
                rPam.SetMark();
            }
            assert(nodes.first->GetIndex() < rPam.Start()->GetNodeIndex());
            rPam.Start()->Assign(*nodes.first);
        }
        nodes = sw::GetFirstAndLastNode(*pLayout, rPam.End()->GetNode());
        if (nodes.second && nodes.second != &rPam.End()->GetNode())
        {
            assert(nodes.first);
            if (SwNodeOffset(0) < nOffset)
            {
                nOffset -= nodes.second->GetIndex() - rPam.End()->GetNodeIndex();
                if (nOffset <= SwNodeOffset(0))   // hack: there are callers that know what
                {                   // node they want; those should never need
                    nOffset = SwNodeOffset(+1);   // this; other callers just pass in +1
                }                   // and those should still move
            }
            if (!rPam.HasMark())
            {
                rPam.SetMark();
            }
            assert(rPam.End()->GetNodeIndex() < nodes.second->GetIndex());
            // until end, otherwise Impl will detect overlapping redline
            rPam.End()->Assign(*nodes.second, nodes.second->GetTextNode()->Len());
        }

        if (nOffset > SwNodeOffset(0))
        {   // sw_redlinehide: avoid moving into delete redline, skip forward
            if (GetNodes().GetEndOfContent().GetIndex() <= rPam.End()->GetNodeIndex() + nOffset)
            {
                return false; // can't move
            }
            SwNode const* pNode(GetNodes()[rPam.End()->GetNodeIndex() + nOffset + 1]);
            if (   pNode->GetRedlineMergeFlag() != SwNode::Merge::None
                && pNode->GetRedlineMergeFlag() != SwNode::Merge::First)
            {
                for ( ; ; ++nOffset)
                {
                    pNode = GetNodes()[rPam.End()->GetNodeIndex() + nOffset];
                    if (pNode->IsTextNode())
                    {
                        nodes = GetFirstAndLastNode(*pLayout, *pNode->GetTextNode());
                        assert(nodes.first && nodes.second);
                        nOffset += nodes.second->GetIndex() - pNode->GetIndex();
                        // on last; will be incremented below to behind-last
                        break;
                    }
                }
            }
        }
        else
        {   // sw_redlinehide: avoid moving into delete redline, skip backward
            if (rPam.Start()->GetNodeIndex() + nOffset < SwNodeOffset(1))
            {
                return false; // can't move
            }
            SwNode const* pNode(GetNodes()[rPam.Start()->GetNodeIndex() + nOffset]);
            if (   pNode->GetRedlineMergeFlag() != SwNode::Merge::None
                && pNode->GetRedlineMergeFlag() != SwNode::Merge::First)
            {
                for ( ; ; --nOffset)
                {
                    pNode = GetNodes()[rPam.Start()->GetNodeIndex() + nOffset];
                    if (pNode->IsTextNode())
                    {
                        nodes = GetFirstAndLastNode(*pLayout, *pNode->GetTextNode());
                        assert(nodes.first && nodes.second);
                        nOffset -= pNode->GetIndex() - nodes.first->GetIndex();
                        // on first
                        break;
                    }
                }
            }
        }
    }
    return MoveParagraphImpl(rPam, nOffset, bIsOutlMv, pLayout);
}

bool SwDoc::MoveParagraphImpl(SwPaM& rPam, SwNodeOffset const nOffset,
        bool const bIsOutlMv, SwRootFrame const*const pLayout)
{
    auto [pStt, pEnd] = rPam.StartEnd(); // SwPosition*

    SwNodeOffset nStIdx = pStt->GetNodeIndex();
    SwNodeOffset nEndIdx = pEnd->GetNodeIndex();

    // Here are some sophisticated checks whether the wished PaM will be moved or not.
    // For moving outlines (bIsOutlMv) I've already done some checks, so here are two different
    // checks...
    SwNode *pTmp1;
    SwNode *pTmp2;
    if( bIsOutlMv )
    {
        // For moving chapters (outline) the following reason will deny the move:
        // if a start node is inside the moved range and its end node outside or vice versa.
        // If a start node is the first moved paragraph, its end node has to be within the moved
        // range, too (e.g. as last node).
        // If an end node is the last node of the moved range, its start node has to be a part of
        // the moved section, too.
        pTmp1 = GetNodes()[ nStIdx ];
        if( pTmp1->IsStartNode() )
        {
            // coverity[copy_paste_error : FALSE] - First is a start node
            pTmp2 = pTmp1->EndOfSectionNode();
            if( pTmp2->GetIndex() > nEndIdx )
                return false; // Its end node is behind the moved range
        }
        pTmp1 = pTmp1->StartOfSectionNode()->EndOfSectionNode();
        if( pTmp1->GetIndex() <= nEndIdx )
            return false; // End node inside but start node before moved range => no.
        pTmp1 = GetNodes()[ nEndIdx ];
        if( pTmp1->IsEndNode() )
        {   // The last one is an end node
            pTmp1 = pTmp1->StartOfSectionNode();
            if( pTmp1->GetIndex() < nStIdx )
                return false; // Its start node is before the moved range.
        }
        pTmp1 = pTmp1->StartOfSectionNode();
        if( pTmp1->GetIndex() >= nStIdx )
            return false; // A start node which ends behind the moved range => no.
    }

    SwNodeOffset nInStIdx, nInEndIdx;
    SwNodeOffset nOffs = nOffset;
    if( nOffset > SwNodeOffset(0) )
    {
        nInEndIdx = nEndIdx;
        nEndIdx += nOffset;
        ++nOffs;
    }
    else
    {
        // Impossible to move to negative index
        if( abs( nOffset ) > nStIdx)
            return false;

        nInEndIdx = nStIdx - 1;
        nStIdx += nOffset;
    }
    nInStIdx = nInEndIdx + 1;
    // The following paragraphs shall be swapped:
    // Swap [ nStIdx, nInEndIdx ] with [ nInStIdx, nEndIdx ]

    if( nEndIdx >= GetNodes().GetEndOfContent().GetIndex() )
        return false;

    if( !bIsOutlMv )
    {   // And here the restrictions for moving paragraphs other than chapters (outlines)
        // The plan is to exchange [nStIdx,nInEndIdx] and [nStartIdx,nEndIdx]
        // It will checked if the both "start" nodes as well as the both "end" notes belongs to
        // the same start-end-section. This is more restrictive than the conditions checked above.
        // E.g. a paragraph will not escape from a section or be inserted to another section.
        pTmp1 = GetNodes()[ nStIdx ]->StartOfSectionNode();
        pTmp2 = GetNodes()[ nInStIdx ]->StartOfSectionNode();
        if( pTmp1 != pTmp2 )
            return false; // "start" nodes in different sections
        pTmp1 = GetNodes()[ nEndIdx ];
        bool bIsEndNode = pTmp1->IsEndNode();
        if( !pTmp1->IsStartNode() )
        {
            pTmp1 = pTmp1->StartOfSectionNode();
            if( bIsEndNode ) // For end nodes the first start node is of course inside the range,
                pTmp1 = pTmp1->StartOfSectionNode(); // I've to check the start node of the start node.
        }
        pTmp1 = pTmp1->EndOfSectionNode();
        pTmp2 = GetNodes()[ nInEndIdx ];
        if( !pTmp2->IsStartNode() )
        {
            bIsEndNode = pTmp2->IsEndNode();
            pTmp2 = pTmp2->StartOfSectionNode();
            if( bIsEndNode )
                pTmp2 = pTmp2->StartOfSectionNode();
        }
        pTmp2 = pTmp2->EndOfSectionNode();
        if( pTmp1 != pTmp2 )
            return false; // The "end" notes are in different sections
    }

    // Test for Redlining - Can the Selection be moved at all, actually?
    if( !getIDocumentRedlineAccess().IsIgnoreRedline() )
    {
        SwRedlineTable::size_type nRedlPos = getIDocumentRedlineAccess().GetRedlinePos( pStt->GetNode(), RedlineType::Delete );
        if( SwRedlineTable::npos != nRedlPos )
        {
            SwContentNode* pCNd = pEnd->GetNode().GetContentNode();
            SwPosition aStPos( pStt->GetNode() );
            SwPosition aEndPos( pEnd->GetNode(), pCNd, pCNd ? pCNd->Len() : 1 );
            bool bCheckDel = true;

            // There is a some Redline Delete Object for the range
            for( ; nRedlPos < getIDocumentRedlineAccess().GetRedlineTable().size(); ++nRedlPos )
            {
                const SwRangeRedline* pTmp = getIDocumentRedlineAccess().GetRedlineTable()[ nRedlPos ];
                if( !bCheckDel || RedlineType::Delete == pTmp->GetType() )
                {
                    auto [pRStt, pREnd] = pTmp->StartEnd(); // SwPosition*
                    switch( ComparePosition( *pRStt, *pREnd, aStPos, aEndPos ))
                    {
                    case SwComparePosition::CollideStart:
                    case SwComparePosition::Behind:            // Pos1 comes after Pos2
                        nRedlPos = getIDocumentRedlineAccess().GetRedlineTable().size();
                        break;

                    case SwComparePosition::CollideEnd:
                    case SwComparePosition::Before:            // Pos1 comes before Pos2
                        break;
                    case SwComparePosition::Inside:            // Pos1 is completely inside Pos2
                        // that's valid, but check all following for overlapping
                        bCheckDel = false;
                        break;

                    case SwComparePosition::Outside:           // Pos2 is completely inside Pos1
                    case SwComparePosition::Equal:             // Pos1 is equal to Pos2
                    case SwComparePosition::OverlapBefore:    // Pos1 overlaps Pos2 in the beginning
                    case SwComparePosition::OverlapBehind:    // Pos1 overlaps Pos2 at the end
                        return false;
                    }
                }
            }
        }
    }

    {
        // Send DataChanged before moving. We then can detect
        // which objects are still in the range.
        // After the move they could come before/after the
        // Position.
        SwDataChanged aTmp( rPam );
    }

    SwNodeIndex aIdx( nOffset > SwNodeOffset(0) ? pEnd->GetNode() : pStt->GetNode(), nOffs );
    SwNodeRange aMvRg( pStt->GetNode(), SwNodeOffset(0), pEnd->GetNode(), SwNodeOffset(+1) );

    SwRangeRedline* pOwnRedl = nullptr;
    if( getIDocumentRedlineAccess().IsRedlineOn() )
    {
        // If the range is completely in the own Redline, we can move it!
        SwRedlineTable::size_type nRedlPos = getIDocumentRedlineAccess().GetRedlinePos( pStt->GetNode(), RedlineType::Insert );
        if( SwRedlineTable::npos != nRedlPos )
        {
            SwRangeRedline* pTmp = getIDocumentRedlineAccess().GetRedlineTable()[ nRedlPos ];
            auto [pRStt, pREnd] = pTmp->StartEnd(); // SwPosition*
            SwRangeRedline aTmpRedl( RedlineType::Insert, rPam );
            const SwContentNode* pCEndNd = pEnd->GetNode().GetContentNode();
            // Is completely in the range and is the own Redline too?
            if( aTmpRedl.IsOwnRedline( *pTmp ) &&
                (pRStt->GetNode() < pStt->GetNode() ||
                (pRStt->GetNode() == pStt->GetNode() && !pRStt->GetContentIndex()) ) &&
                (pEnd->GetNode() < pREnd->GetNode() ||
                (pEnd->GetNode() == pREnd->GetNode() &&
                 pCEndNd ? pREnd->GetContentIndex() == pCEndNd->Len()
                         : !pREnd->GetContentIndex() )) )
            {
                pOwnRedl = pTmp;
                if( nRedlPos + 1 < getIDocumentRedlineAccess().GetRedlineTable().size() )
                {
                    pTmp = getIDocumentRedlineAccess().GetRedlineTable()[ nRedlPos+1 ];
                    if( *pTmp->Start() == *pREnd )
                        // then don't!
                        pOwnRedl = nullptr;
                }

                if( pOwnRedl &&
                    ( pRStt->GetNode() > aIdx.GetNode() || aIdx > pREnd->GetNode() ||
                    // pOwnRedl doesn't start at the beginning of a node, so it's not
                    // possible to resize it to contain the line moved before it
                    ( pRStt->GetNode() == aIdx.GetNode() && pRStt->GetContentIndex() > 0 ) ) )
                {
                    // it's not in itself, so don't move it
                    pOwnRedl = nullptr;
                }
            }
        }

        if( !pOwnRedl )
        {
            GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );

            // First the Insert, then the Delete
            SwPosition aInsPos( aIdx );

            std::optional<SwPaM> oPam( std::in_place, pStt->GetNode(), 0, aMvRg.aEnd.GetNode(), 0 );

            SwPaM& rOrigPam(rPam);
            rOrigPam.DeleteMark();
            rOrigPam.GetPoint()->Assign(aIdx.GetIndex() - 1);

            bool bDelLastPara = !aInsPos.GetNode().IsContentNode();
            SwNodeOffset nOrigIdx = aIdx.GetIndex();

            /* When copying to a non-content node Copy will
               insert a paragraph before that node and insert before
               that inserted node. Copy creates an SwUndoInserts that
               does not cover the extra paragraph. Thus we insert the
               extra paragraph ourselves, _with_ correct undo
               information. */
            if (bDelLastPara)
            {
                /* aInsPos points to the non-content node. Move it to
                   the previous content node. */
                SwPaM aInsPam(aInsPos);
                const bool bMoved = aInsPam.Move(fnMoveBackward);
                OSL_ENSURE(bMoved, "No content node found!");

                if (bMoved)
                {
                    /* Append the new node after the content node
                       found. The new position to insert the moved
                       paragraph at is before the inserted
                       paragraph. */
                    getIDocumentContentOperations().AppendTextNode(*aInsPam.GetPoint());
                    aInsPos = *aInsPam.GetPoint();
                }
            }

            --aIdx; // move before insertion

            // adjust empty nodes later
            SwTextNode const*const pIsEmptyNode(nOffset < SwNodeOffset(0)
                           ? aInsPos.GetNode().GetTextNode()
                           : aIdx.GetNode().GetTextNode());
            bool bIsEmptyNode = pIsEmptyNode && pIsEmptyNode->Len() == 0;

            sal_uInt32 nMovedID = getIDocumentRedlineAccess().GetRedlineTable().getNewMovedID();
            getIDocumentContentOperations().CopyRange(*oPam, aInsPos, SwCopyFlags::CheckPosInFly,
                                                      nMovedID);

            // now delete all the delete redlines that were copied
#ifndef NDEBUG
            size_t nRedlines(getIDocumentRedlineAccess().GetRedlineTable().size());
#endif
            if (nOffset > SwNodeOffset(0))
                assert(oPam->End()->GetNodeIndex() - oPam->Start()->GetNodeIndex() + nOffset == aInsPos.GetNodeIndex() - oPam->End()->GetNodeIndex());
            else
                assert(oPam->Start()->GetNodeIndex() - oPam->End()->GetNodeIndex() + nOffset == aInsPos.GetNodeIndex() - oPam->End()->GetNodeIndex());
            SwRedlineTable::size_type i;
            getIDocumentRedlineAccess().GetRedline(*oPam->End(), &i);
            for ( ; 0 < i; --i)
            {   // iterate backwards and offset via the start nodes difference
                SwRangeRedline const*const pRedline = getIDocumentRedlineAccess().GetRedlineTable()[i - 1];
                if (*pRedline->End() < *oPam->Start())
                {
                    break;
                }
                if (pRedline->GetType() == RedlineType::Delete &&
                    // tdf#145066 skip full-paragraph deletion which was jumped over
                    // in Show Changes mode to avoid of deleting an extra row
                    *oPam->Start() <= *pRedline->Start())
                {
                    SwRangeRedline* pNewRedline;
                    {
                        SwPaM pam(*pRedline, nullptr);
                        SwNodeOffset const nCurrentOffset(
                            nOrigIdx - oPam->Start()->GetNodeIndex());
                        pam.GetPoint()->Assign(pam.GetPoint()->GetNodeIndex() + nCurrentOffset,
                                               pam.GetPoint()->GetContentIndex());
                        pam.GetMark()->Assign(pam.GetMark()->GetNodeIndex() + nCurrentOffset,
                                              pam.GetMark()->GetContentIndex());

                        pNewRedline = new SwRangeRedline( RedlineType::Delete, pam, nMovedID );
                    }
                    // note: effectively this will DeleteAndJoin the pam!
                    getIDocumentRedlineAccess().AppendRedline(pNewRedline, true);
                    assert(getIDocumentRedlineAccess().GetRedlineTable().size() <= nRedlines);
                }
            }

            if( bDelLastPara )
            {
                // We need to remove the last empty Node again
                aIdx = aInsPos.GetNode();
                SwContentNode* pCNd = SwNodes::GoPrevious( &aInsPos );
                if (pCNd)
                    aInsPos.AssignEndIndex( *pCNd );

                // All, that are in the to-be-deleted Node, need to be
                // moved to the next Node
                for(SwRangeRedline* pTmp : getIDocumentRedlineAccess().GetRedlineTable())
                {
                    SwPosition* pPos = &pTmp->GetBound();
                    if( pPos->GetNode() == aIdx.GetNode() )
                    {
                        pPos->Adjust(SwNodeOffset(1));
                    }
                    pPos = &pTmp->GetBound(false);
                    if( pPos->GetNode() == aIdx.GetNode() )
                    {
                        pPos->Adjust(SwNodeOffset(1));
                    }
                }
                CorrRel( aIdx.GetNode(), aInsPos );

                if (pCNd)
                    pCNd->JoinNext();
            }

            rOrigPam.GetPoint()->Adjust(SwNodeOffset(1));
            assert(*oPam->GetMark() < *oPam->GetPoint());
            if (oPam->GetPoint()->GetNode().IsEndNode())
            {   // ensure redline ends on content node
                oPam->GetPoint()->Adjust(SwNodeOffset(-1));
                assert(oPam->GetPoint()->GetNode().IsTextNode());
                SwTextNode *const pNode(oPam->GetPoint()->GetNode().GetTextNode());
                oPam->GetPoint()->SetContent(pNode->Len());
            }

            RedlineFlags eOld = getIDocumentRedlineAccess().GetRedlineFlags();
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                // this should no longer happen in calls from the UI but maybe via API
                SAL_WARN_IF((eOld & RedlineFlags::ShowMask) != RedlineFlags::ShowMask,
                    "sw.core", "redlines will be moved in DeleteAndJoin");

                getIDocumentRedlineAccess().SetRedlineFlags(
                   RedlineFlags::On | RedlineFlags::ShowInsert | RedlineFlags::ShowDelete );
                GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoRedlineDelete>(*oPam, SwUndoId::DELETE));
            }

            SwRangeRedline* pNewRedline = new SwRangeRedline( RedlineType::Delete, *oPam, nMovedID );

            // prevent assertion from aPam's target being deleted
            SwNodeIndex bound1(oPam->GetBound().GetNode());
            SwNodeIndex bound2(oPam->GetBound(false).GetNode());
            oPam.reset();

            getIDocumentRedlineAccess().AppendRedline( pNewRedline, true, nMovedID );

            oPam.emplace(bound1, bound2);
            sw::UpdateFramesForAddDeleteRedline(*this, *oPam);

            // avoid setting empty nodes to tracked insertion
            if ( bIsEmptyNode )
            {
                SwRedlineTable& rTable = getIDocumentRedlineAccess().GetRedlineTable();
                SwRedlineTable::size_type nRedlPosWithEmpty =
                    getIDocumentRedlineAccess().GetRedlinePos( pStt->GetNode(), RedlineType::Insert );
                if ( SwRedlineTable::npos != nRedlPosWithEmpty )
                {
                    pOwnRedl = rTable[nRedlPosWithEmpty];
                    SwPosition *pRPos = nOffset < SwNodeOffset(0) ? pOwnRedl->End() : pOwnRedl->Start();
                    SwNodeIndex aIdx2 ( pRPos->GetNode() );
                    SwTextNode const*const pEmptyNode0(aIdx2.GetNode().GetTextNode());
                    if ( nOffset < SwNodeOffset(0) )
                    {
                        // move up
                        --aIdx2;
                        SwTextNode const*const pEmptyNode(aIdx2.GetNode().GetTextNode());
                        if ( pEmptyNode && pEmptyNode->Len() == 0 )
                            pRPos->Adjust(SwNodeOffset(-1));
                    }
                    else if ( pEmptyNode0 && pEmptyNode0->Len() == 0 )
                    {
                        // move down
                        ++aIdx2;
                        SwTextNode const*const pEmptyNode(aIdx2.GetNode().GetTextNode());
                        if (pEmptyNode)
                            pRPos->Adjust(SwNodeOffset(+1));
                    }

                    // sort redlines, when the trimmed range results bad redline order
                    if ( nRedlPosWithEmpty + 1 < rTable.size() &&
                            *rTable[nRedlPosWithEmpty + 1] < *rTable[nRedlPosWithEmpty] )
                    {
                        rTable.Remove(nRedlPosWithEmpty);
                        rTable.Insert(pOwnRedl);
                    }
                }
            }

            getIDocumentRedlineAccess().SetRedlineFlags( eOld );
            GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
            getIDocumentState().SetModified();

            return true;
        }
    }

    if( !pOwnRedl && !getIDocumentRedlineAccess().IsIgnoreRedline() && !getIDocumentRedlineAccess().GetRedlineTable().empty() )
    {
        SwPaM aTemp(aIdx);
        getIDocumentRedlineAccess().SplitRedline(aTemp);
    }

    SwNodeOffset nRedlSttNd(0), nRedlEndNd(0);
    if( pOwnRedl )
    {
        const SwPosition *pRStt = pOwnRedl->Start(), *pREnd = pOwnRedl->End();
        nRedlSttNd = pRStt->GetNodeIndex();
        nRedlEndNd = pREnd->GetNodeIndex();
    }

    std::unique_ptr<SwUndoMoveNum> pUndo;
    SwNodeOffset nMoved(0);
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo.reset(new SwUndoMoveNum( rPam, nOffset, bIsOutlMv ));
        nMoved = rPam.End()->GetNodeIndex() - rPam.Start()->GetNodeIndex() + 1;
    }

    (void) pLayout; // note: move will insert between aIdx-1 and aIdx
    assert(!pLayout // check not moving *into* delete redline (caller's fault)
        || aIdx.GetNode().GetRedlineMergeFlag() == SwNode::Merge::None
        || aIdx.GetNode().GetRedlineMergeFlag() == SwNode::Merge::First);
    getIDocumentContentOperations().MoveNodeRange( aMvRg, aIdx.GetNode(), SwMoveFlags::REDLINES );

    if( pUndo )
    {
        // i57907: Under circumstances (sections at the end of a chapter)
        // the rPam.Start() is not moved to the new position.
        // But aIdx should be at the new end position and as long as the
        // number of moved paragraphs is nMoved, I know, where the new
        // position is.
        pUndo->SetStartNode( aIdx.GetIndex() - nMoved );
        GetIDocumentUndoRedo().AppendUndo(std::move(pUndo));
    }

    if( pOwnRedl )
    {
        auto [pRStt, pREnd] = pOwnRedl->StartEnd(); // SwPosition*
        if( pRStt->GetNodeIndex() != nRedlSttNd )
        {
            pRStt->Assign(nRedlSttNd);
        }
        if( pREnd->GetNodeIndex() != nRedlEndNd )
        {
            pREnd->Assign(nRedlEndNd);
            SwContentNode* pCNd = pREnd->GetNode().GetContentNode();
            if(pCNd)
                pREnd->SetContent( pCNd->Len() );
        }
    }

    getIDocumentState().SetModified();
    return true;
}

bool SwDoc::NumOrNoNum( SwNode& rIdx, bool bDel )
{
    bool bResult = false;
    SwTextNode * pTextNd = rIdx.GetTextNode();

    if (pTextNd && pTextNd->GetNumRule() != nullptr &&
        (pTextNd->HasNumber() || pTextNd->HasBullet()))
    {
        if ( !pTextNd->IsCountedInList() == !bDel)
        {
            bool bOldNum = bDel;
            bool bNewNum = !bDel;
            pTextNd->SetCountedInList(bNewNum);

            getIDocumentState().SetModified();

            bResult = true;

            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoNumOrNoNum>(rIdx, bOldNum, bNewNum));
            }
        }
        else if (bDel && pTextNd->GetNumRule(false) &&
                 pTextNd->GetActualListLevel() >= 0 &&
                 pTextNd->GetActualListLevel() < MAXLEVEL)
        {
            SwPaM aPam(*pTextNd);
            DelNumRules(aPam);

            bResult = true;
        }
    }

    return bResult;
}

SwNumRule* SwDoc::GetNumRuleAtPos(SwPosition& rPos,
        SwRootFrame const*const pLayout)
{
    SwNumRule* pRet = nullptr;
    SwTextNode* pTNd = rPos.GetNode().GetTextNode();

    if ( pTNd != nullptr )
    {
        if (pLayout && !sw::IsParaPropsNode(*pLayout, *pTNd))
        {
            pTNd = static_cast<SwTextFrame*>(pTNd->getLayoutFrame(pLayout))->GetMergedPara()->pParaPropsNode;
            rPos.Assign(*pTNd);
        }
        pRet = pTNd->GetNumRule();
    }

    return pRet;
}

sal_uInt16 SwDoc::FindNumRule( std::u16string_view rName ) const
{
    for( sal_uInt16 n = mpNumRuleTable->size(); n; )
        if( (*mpNumRuleTable)[ --n ]->GetName() == rName )
            return n;

    return USHRT_MAX;
}

SwNumRule* SwDoc::FindNumRulePtr( const OUString& rName ) const
{
    SwNumRule * pResult = maNumRuleMap[rName];

    if ( !pResult )
    {
        for (size_t n = 0; n < mpNumRuleTable->size(); ++n)
        {
            if ((*mpNumRuleTable)[n]->GetName() == rName)
            {
                pResult = (*mpNumRuleTable)[n];

                break;
            }
        }
    }

    return pResult;
}

void SwDoc::AddNumRule(SwNumRule * pRule)
{
    if ((SAL_MAX_UINT16 - 1) <= mpNumRuleTable->size())
    {
        OSL_ENSURE(false, "SwDoc::AddNumRule: table full.");
        abort(); // this should never happen on real documents
    }
    mpNumRuleTable->push_back(pRule);
    maNumRuleMap[pRule->GetName()] = pRule;
    pRule->SetNumRuleMap(&maNumRuleMap);

    getIDocumentListsAccess().createListForListStyle( pRule->GetName() );
}

sal_uInt16 SwDoc::MakeNumRule( const OUString &rName,
            const SwNumRule* pCpy,
            bool bBroadcast,
            const SvxNumberFormat::SvxNumPositionAndSpaceMode eDefaultNumberFormatPositionAndSpaceMode )
{
    SwNumRule* pNew;
    if( pCpy )
    {
        pNew = new SwNumRule( *pCpy );

        pNew->SetName( GetUniqueNumRuleName( &rName ), getIDocumentListsAccess() );

        if( pNew->GetName() != rName )
        {
            pNew->SetPoolFormatId( USHRT_MAX );
            pNew->SetPoolHelpId( USHRT_MAX );
            pNew->SetPoolHlpFileId( UCHAR_MAX );
            pNew->SetDefaultListId( OUString() );
        }
        pNew->CheckCharFormats( *this );
    }
    else
    {
        pNew = new SwNumRule( GetUniqueNumRuleName( &rName ),
                              eDefaultNumberFormatPositionAndSpaceMode );
    }

    sal_uInt16 nRet = mpNumRuleTable->size();

    AddNumRule(pNew);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoNumruleCreate>(pNew, *this));
    }

    if (bBroadcast)
        BroadcastStyleOperation(pNew->GetName(), SfxStyleFamily::Pseudo,
                                SfxHintId::StyleSheetCreated);

    return nRet;
}

OUString SwDoc::GetUniqueNumRuleName( const OUString* pChkStr, bool bAutoNum ) const
{
    // If we got pChkStr, then the caller expects that in case it's not yet
    // used, it'll be returned.
    if( IsInMailMerge() && !pChkStr )
    {
        OUString newName = "MailMergeNumRule"
            + OStringToOUString( DateTimeToOString( DateTime( DateTime::SYSTEM )), RTL_TEXTENCODING_ASCII_US )
            + OUString::number( mpNumRuleTable->size() + 1 );
        return newName;
    }

    OUString aName;
    if( bAutoNum )
    {
        static bool bHack = (getenv("LIBO_ONEWAY_STABLE_ODF_EXPORT") != nullptr);

        if (bHack)
        {
            static sal_Int64 nIdCounter = SAL_CONST_INT64(8000000000);
            aName = OUString::number(nIdCounter++);
        }
        else
        {
            unsigned int const n(comphelper::rng::uniform_uint_distribution(0,
                                    std::numeric_limits<unsigned int>::max()));
            aName = OUString::number(n);
        }
        if( pChkStr && pChkStr->isEmpty() )
            pChkStr = nullptr;
    }
    else if( pChkStr && !pChkStr->isEmpty() )
        aName = *pChkStr;
    else
    {
        pChkStr = nullptr;
        aName = SwResId( STR_NUMRULE_DEFNAME );
    }

    sal_uInt16 nNum(0), nTmp, nFlagSize = ( mpNumRuleTable->size() / 8 ) +2;
    std::unique_ptr<sal_uInt8[]> pSetFlags(new sal_uInt8[ nFlagSize ]);
    memset( pSetFlags.get(), 0, nFlagSize );

    sal_Int32 nNmLen = aName.getLength();
    if( !bAutoNum && pChkStr )
    {
        while( nNmLen-- && '0' <= aName[nNmLen] && aName[nNmLen] <= '9' )
            ; //nop

        if( ++nNmLen < aName.getLength() )
        {
            aName = aName.copy(0, nNmLen );
            pChkStr = nullptr;
        }
    }

    for( auto const & pNumRule: *mpNumRuleTable )
        if( nullptr != pNumRule )
        {
            const OUString sNm = pNumRule->GetName();
            if( sNm.startsWith( aName ) )
            {
                // Determine Number and set the Flag
                nNum = o3tl::narrowing<sal_uInt16>(o3tl::toInt32(sNm.subView( nNmLen )));
                if( nNum-- && nNum < mpNumRuleTable->size() )
                    pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
            }
            if( pChkStr && *pChkStr==sNm )
                pChkStr = nullptr;
        }

    if( !pChkStr )
    {
        // All Numbers have been flagged accordingly, so identify the right Number
        nNum = mpNumRuleTable->size();
        for( sal_uInt16 n = 0; n < nFlagSize; ++n )
        {
            nTmp = pSetFlags[ n ];
            if( 0xff != nTmp )
            {
                // identify the Number
                nNum = n * 8;
                while( nTmp & 1 )
                {
                    ++nNum;
                    nTmp >>= 1;
                }
                break;
            }
        }
    }
    if( pChkStr && !pChkStr->isEmpty() )
        return *pChkStr;
    return aName + OUString::number( ++nNum );
}

void SwDoc::UpdateNumRule()
{
    const SwNumRuleTable& rNmTable = GetNumRuleTable();
    for( size_t n = 0; n < rNmTable.size(); ++n )
        if( rNmTable[ n ]->IsInvalidRule() )
            rNmTable[ n ]->Validate(*this);
}

void SwDoc::MarkListLevel( const OUString& sListId,
                           const int nListLevel,
                           const bool bValue )
{
    SwList* pList = getIDocumentListsAccess().getListByName( sListId );

    if ( pList )
    {
        // Set new marked list level and notify all affected nodes of the changed mark.
        pList->MarkListLevel( nListLevel, bValue );
    }
}

bool SwDoc::IsFirstOfNumRuleAtPos(const SwPosition & rPos,
        SwRootFrame const& rLayout)
{
    bool bResult = false;

    const SwTextNode *const pTextNode = sw::GetParaPropsNode(rLayout, rPos.GetNode());
    if ( pTextNode != nullptr )
    {
        bResult = pTextNode->IsFirstOfNumRule(rLayout);
    }

    return bResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
