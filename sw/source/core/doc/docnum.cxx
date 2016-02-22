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
#include <rtl/random.h>
#include <tools/resid.hxx>
#include <editeng/lrspitem.hxx>
#include <ftninfo.hxx>
#include <ftnidx.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentListsAccess.hxx>
#include <DocumentRedlineManager.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <doctxm.hxx>
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
#include <pamtyp.hxx>
#include <redline.hxx>
#include <comcore.hrc>
#include <editeng/adjustitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <frmatr.hxx>
#include <SwStyleNameMapper.hxx>
#include <SwNodeNum.hxx>
#include <list.hxx>
#include <calbck.hxx>
#include <comphelper/string.hxx>
#include <tools/datetimeutils.hxx>

#include <cstdlib>
#include <map>

namespace {
    static void lcl_ResetIndentAttrs(SwDoc *pDoc, const SwPaM &rPam, sal_uInt16 marker )
    {
        std::set<sal_uInt16> aResetAttrsArray;
        aResetAttrsArray.insert( marker );
        // #i114929#
        // On a selection setup a corresponding Point-and-Mark in order to get
        // the indentation attribute reset on all paragraphs touched by the selection
        if ( rPam.HasMark() &&
             rPam.End()->nNode.GetNode().GetTextNode() )
        {
            SwPaM aPam( rPam.Start()->nNode,
                        rPam.End()->nNode );
            aPam.Start()->nContent = 0;
            aPam.End()->nContent = rPam.End()->nNode.GetNode().GetTextNode()->Len();
            pDoc->ResetAttrs( aPam, false, aResetAttrsArray );
        }
        else
        {
            pDoc->ResetAttrs( rPam, false, aResetAttrsArray );
        }
    }
}

#include <stdlib.h>

inline sal_uInt8 GetUpperLvlChg( sal_uInt8 nCurLvl, sal_uInt8 nLevel, sal_uInt16 nMask )
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
    mpOutlineRule->CheckCharFormats( this );

    // notify text nodes, which are registered at the outline style, about the
    // changed outline style
    SwNumRule::tTextNodeList aTextNodeList;
    mpOutlineRule->GetTextNodeList( aTextNodeList );
    for ( SwNumRule::tTextNodeList::iterator aIter = aTextNodeList.begin();
          aIter != aTextNodeList.end(); ++aIter )
    {
        SwTextNode* pTextNd = *aIter;
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
    if( !GetFootnoteIdxs().empty() && FTNNUM_CHAPTER == GetFootnoteInfo().eNum )
        GetFootnoteIdxs().UpdateAllFootnote();

    getIDocumentFieldsAccess().UpdateExpFields(nullptr, true);

    getIDocumentState().SetModified();
}

void SwDoc::PropagateOutlineRule()
{
    for (auto pColl : *mpTextFormatCollTable)
    {
        if(pColl->IsAssignedToListLevelOfOutlineStyle())
        {
            // Check only the list style, which is set at the paragraph style
            const SwNumRuleItem & rCollRuleItem = pColl->GetNumRule( false );

            if ( rCollRuleItem.GetValue().isEmpty() )
            {
                SwNumRule * pMyOutlineRule = GetOutlineNumRule();

                if (pMyOutlineRule)
                {
                    SwNumRuleItem aNumItem( pMyOutlineRule->GetName() );

                    pColl->SetFormatAttr(aNumItem);
                }
            }
        }
    }
}

// Increase/Decrease
bool SwDoc::OutlineUpDown( const SwPaM& rPam, short nOffset )
{
    if( GetNodes().GetOutLineNds().empty() || !nOffset )
        return false;

    // calculate the range
    const SwOutlineNodes& rOutlNds = GetNodes().GetOutLineNds();
    const SwNodePtr pSttNd = &rPam.Start()->nNode.GetNode();
    const SwNodePtr pEndNd = &rPam.End()->nNode.GetNode();
    sal_uInt16 nSttPos, nEndPos;

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
                getIDocumentStylePoolAccess().GetTextCollFromPool(static_cast<sal_uInt16>(RES_POOLCOLL_HEADLINE1 + n));

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
                getIDocumentStylePoolAccess().GetTextCollFromPool(static_cast<sal_uInt16>(RES_POOLCOLL_HEADLINE1 + n));

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
        GetIDocumentUndoRedo().StartUndo(UNDO_OUTLINE_LR, nullptr);
        SwUndo *const pUndoOLR( new SwUndoOutlineLeftRight( rPam, nOffset ) );
        GetIDocumentUndoRedo().AppendUndo(pUndoOLR);
    }

    // 2. Apply the new style to all Nodes
    for (auto i = nSttPos; i < nEndPos; ++i)
    {
        SwTextNode* pTextNd = rOutlNds[ i ]->GetTextNode();
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
        GetIDocumentUndoRedo().EndUndo(UNDO_OUTLINE_LR, nullptr);
    }

    ChkCondColls();
    getIDocumentState().SetModified();

    return true;
}

// Move up/down
bool SwDoc::MoveOutlinePara( const SwPaM& rPam, short nOffset )
{
    // Do not move to special sections in the nodes array
    const SwPosition& rStt = *rPam.Start(),
                    & rEnd = &rStt == rPam.GetPoint() ? *rPam.GetMark()
                                                      : *rPam.GetPoint();
    if( GetNodes().GetOutLineNds().empty() || !nOffset ||
        (rStt.nNode.GetIndex() < GetNodes().GetEndOfExtras().GetIndex()) ||
        (rEnd.nNode.GetIndex() < GetNodes().GetEndOfExtras().GetIndex()))
    {
        return false;
    }

    sal_uInt16 nAktPos = 0;
    SwNodeIndex aSttRg( rStt.nNode ), aEndRg( rEnd.nNode );

    int nOutLineLevel = MAXLEVEL;
    SwNode* pSrch = &aSttRg.GetNode();

   if( pSrch->IsTextNode())
        nOutLineLevel = static_cast<sal_uInt8>(pSrch->GetTextNode()->GetAttrOutlineLevel()-1);
    SwNode* pEndSrch = &aEndRg.GetNode();
    if( !GetNodes().GetOutLineNds().Seek_Entry( pSrch, &nAktPos ) )
    {
        if( !nAktPos )
            return false; // Promoting or demoting before the first outline => no.
        if( --nAktPos )
            aSttRg = *GetNodes().GetOutLineNds()[ nAktPos ];
        else if( 0 > nOffset )
            return false; // Promoting at the top of document?!
        else
            aSttRg = *GetNodes().GetEndOfContent().StartOfSectionNode();
    }
    sal_uInt16 nTmpPos = 0;
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
        nAktPos = nTmpPos;
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
    if( nOffset < 0 && nAktPos < sal_uInt16(-nOffset) )
        pNd = GetNodes().GetEndOfContent().StartOfSectionNode();
    else if( nAktPos + nOffset >= (sal_uInt16)GetNodes().GetOutLineNds().size() )
        pNd = &GetNodes().GetEndOfContent();
    else
        pNd = GetNodes().GetOutLineNds()[ nAktPos + nOffset ];

    sal_uLong nNewPos = pNd->GetIndex();

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
    nNewPos = std::max( nNewPos, GetNodes().GetEndOfExtras().GetIndex() + 2 );

    long nOffs = nNewPos - ( 0 < nOffset ? aEndRg.GetIndex() : aSttRg.GetIndex());
    SwPaM aPam( aSttRg, aEndRg, 0, -1 );
    return MoveParagraph( aPam, nOffs, true );
}

static SwTextNode* lcl_FindOutlineName( const SwOutlineNodes& rOutlNds, const OUString& rName,
                            bool bExact )
{
    SwTextNode* pSavedNode = nullptr;
    for( auto pOutlNd : rOutlNds )
    {
        SwTextNode* pTextNd = pOutlNd->GetTextNode();
        const OUString sText( pTextNd->GetExpandText() );
        if (sText.startsWith(rName))
        {
            if (sText.getLength() == rName.getLength())
            {
                // Found "exact", set Pos to the Node
                return pTextNd;
            }
            if( !bExact && !pSavedNode )
            {
                // maybe we just found the text's first part
                pSavedNode = pTextNd;
            }
        }
    }

    return pSavedNode;
}

static SwTextNode* lcl_FindOutlineNum( const SwOutlineNodes& rOutlNds, OUString& rName )
{
    // Valid numbers are (always just offsets!):
    //  ([Number]+\.)+  (as a regular expression!)
    //  (Number follwed by a period, with 5 repetitions)
    //  i.e.: "1.1.", "1.", "1.1.1."
    sal_Int32 nPos = 0;
    OUString sNum = rName.getToken( 0, '.', nPos );
    if( -1 == nPos )
        return nullptr;           // invalid number!

    sal_uInt16 nLevelVal[ MAXLEVEL ];       // numbers of all levels
    memset( nLevelVal, 0, MAXLEVEL * sizeof( nLevelVal[0] ));
    int nLevel = 0;
    OUString sName( rName );

    while( -1 != nPos )
    {
        sal_uInt16 nVal = 0;
        for( sal_Int32 n = 0; n < sNum.getLength(); ++n )
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

        sName = sName.copy( nPos );
        nPos = 0;
        sNum = sName.getToken( 0, '.', nPos );
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
            if ( pNd->GetNum() && pNd->GetActualListLevel() == nLevel - 1 )
            {
                const SwNodeNum & rNdNum = *(pNd->GetNum());
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

// Add this bullet point:

// A Name can contain a Number and/or the Text.

// First, we try to find the correct Entry via the Number.
// If it exists, we compare the Text, to see if it's the right one.
// If that's not the case, we search again via the Text. If it is
// found, we got the right entry. Or else we use the one found by
// searching for the Number.
// If we don't have a Number, we search via the Text only.
bool SwDoc::GotoOutline( SwPosition& rPos, const OUString& rName ) const
{
    if( !rName.isEmpty() )
    {
        const SwOutlineNodes& rOutlNds = GetNodes().GetOutLineNds();

        // 1. step: via the Number:
        OUString sName( rName );
        SwTextNode* pNd = ::lcl_FindOutlineNum( rOutlNds, sName );
        if ( pNd )
        {
            OUString sExpandedText = pNd->GetExpandText();
            //#i4533# leading numbers followed by a dot have been remove while
            //searching for the outline position
            //to compensate this they must be removed from the paragraphs text content, too
            sal_Int32 nPos = 0;
            OUString sTempNum;
            while(!sExpandedText.isEmpty() && !(sTempNum = sExpandedText.getToken(0, '.', nPos)).isEmpty() &&
                    -1 != nPos &&
                    comphelper::string::isdigitAsciiString(sTempNum))
            {
                sExpandedText = sExpandedText.copy(nPos);
                nPos = 0;
            }

            if( sExpandedText != sName )
            {
                SwTextNode *pTmpNd = ::lcl_FindOutlineName( rOutlNds, sName, true );
                if ( pTmpNd )             // found via the Name
                {
                    pNd = pTmpNd;
                }
            }
            rPos.nNode = *pNd;
            rPos.nContent.Assign( pNd, 0 );
            return true;
        }

        pNd = ::lcl_FindOutlineName( rOutlNds, rName, false );
        if ( pNd )
        {
            rPos.nNode = *pNd;
            rPos.nContent.Assign( pNd, 0 );
            return true;
        }

        // #i68289# additional search on hyperlink URL without its outline numbering part
        if ( sName != rName )
        {
            pNd = ::lcl_FindOutlineName( rOutlNds, sName, false );
            if ( pNd )
            {
                rPos.nNode = *pNd;
                rPos.nContent.Assign( pNd, 0 );
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
        pOld->CheckCharFormats( &rDoc );
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
    for ( SwNumRule::tTextNodeList::iterator aIter = aTextNodeList.begin();
          aIter != aTextNodeList.end(); ++aIter )
    {
        SwTextNode* pTextNd = *aIter;
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

    pOld->CheckCharFormats( &rDoc );
    pOld->SetInvalidRule( true );
    pOld->SetContinusNum( rRule.IsContinusNum() );

    rDoc.UpdateNumRule();
}

OUString SwDoc::SetNumRule( const SwPaM& rPam,
                        const SwNumRule& rRule,
                        const bool bCreateNewList,
                        const OUString& sContinuedListId,
                        bool bSetItem,
                        const bool bResetIndentAttrs )
{
    OUString sListId;

    SwUndoInsNum * pUndo = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        // Start/End for attributes!
        GetIDocumentUndoRedo().StartUndo( UNDO_INSNUM, nullptr );
        pUndo = new SwUndoInsNum( rPam, rRule );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
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

    if ( bSetItem )
    {
        if ( bCreateNewList )
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
            getIDocumentContentOperations().InsertPoolItem( rPam, SfxStringItem( RES_PARATR_LIST_ID, sListId ) );
        }
    }

    if ( !rPam.HasMark() )
    {
        SwTextNode * pTextNd = rPam.GetPoint()->nNode.GetNode().GetTextNode();
        // robust code: consider case that the PaM doesn't denote a text node - e.g. it denotes a graphic node
        if ( pTextNd != nullptr )
        {
            SwNumRule * pRule = pTextNd->GetNumRule();

            if (pRule && pRule->GetName() == pNewOrChangedNumRule->GetName())
            {
                bSetItem = false;
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
                        bSetItem = false;
                    }
                }
            }
        }
    }

    if ( bSetItem )
    {
        getIDocumentContentOperations().InsertPoolItem( rPam, SwNumRuleItem( pNewOrChangedNumRule->GetName() ) );
    }

    if ( bResetIndentAttrs
         && pNewOrChangedNumRule->Get( 0 ).GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        ::lcl_ResetIndentAttrs(this, rPam, RES_LR_SPACE);
    }

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().EndUndo( UNDO_INSNUM, nullptr );
    }

    getIDocumentState().SetModified();

    return sListId;
}

void SwDoc::SetCounted(const SwPaM & rPam, bool bCounted)
{
    if ( bCounted )
    {
        ::lcl_ResetIndentAttrs(this, rPam, RES_PARATR_LIST_ISCOUNTED);
    }
    else
    {
        getIDocumentContentOperations().InsertPoolItem( rPam, SfxBoolItem( RES_PARATR_LIST_ISCOUNTED, false ) );
    }
}

void SwDoc::SetNumRuleStart( const SwPosition& rPos, bool bFlag )
{
    SwTextNode* pTextNd = rPos.nNode.GetNode().GetTextNode();

    if (pTextNd)
    {
        const SwNumRule* pRule = pTextNd->GetNumRule();
        if( pRule && !bFlag != !pTextNd->IsListRestart())
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo *const pUndo( new SwUndoNumRuleStart(rPos, bFlag) );
                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }

            pTextNd->SetListRestart(bFlag);

            getIDocumentState().SetModified();
        }
    }
}

void SwDoc::SetNodeNumStart( const SwPosition& rPos, sal_uInt16 nStt )
{
    SwTextNode* pTextNd = rPos.nNode.GetNode().GetTextNode();

    if (pTextNd)
    {
        if ( !pTextNd->HasAttrListRestartValue() ||
             pTextNd->GetAttrListRestartValue() != nStt )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo *const pUndo( new SwUndoNumRuleStart(rPos, nStt) );
                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }
            pTextNd->SetAttrListRestartValue( nStt );

            getIDocumentState().SetModified();
        }
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
            SwUndo * pUndo =
                new SwUndoNumruleDelete(*(*mpNumRuleTable)[nPos], this);
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        if (bBroadcast)
            BroadcastStyleOperation(rName, SFX_STYLE_FAMILY_PSEUDO,
                                    SfxStyleSheetHintId::ERASED);

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
    if( pRule )
    {
        SwUndoInsNum* pUndo = nullptr;
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            pUndo = new SwUndoInsNum( *pRule, rRule );
            pUndo->GetHistory();
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
        ::lcl_ChgNumRule( *this, rRule );
        if (pUndo)
        {
            pUndo->SetLRSpaceEndPos();
        }

        getIDocumentState().SetModified();
    }
}

bool SwDoc::RenameNumRule(const OUString & rOldName, const OUString & rNewName,
                              bool bBroadcast)
{
    bool bResult = false;
    SwNumRule * pNumRule = FindNumRulePtr(rOldName);

    if (pNumRule)
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            SwUndo * pUndo = new SwUndoNumruleRename(rOldName, rNewName, this);
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        SwNumRule::tTextNodeList aTextNodeList;
        pNumRule->GetTextNodeList( aTextNodeList );

        pNumRule->SetName( rNewName, getIDocumentListsAccess() );

        SwNumRuleItem aItem(rNewName);

        for ( SwNumRule::tTextNodeList::iterator aIter = aTextNodeList.begin();
              aIter != aTextNodeList.end(); ++aIter )
        {
            SwTextNode * pTextNd = *aIter;
            pTextNd->SetAttr(aItem);
        }

        bResult = true;

        if (bBroadcast)
            BroadcastStyleOperation(rOldName, SFX_STYLE_FAMILY_PSEUDO,
                                    SfxStyleSheetHintId::MODIFIED);
    }

    return bResult;
}

void SwDoc::StopNumRuleAnimations( OutputDevice* pOut )
{
    for( sal_uInt16 n = GetNumRuleTable().size(); n; )
    {
        SwNumRule::tTextNodeList aTextNodeList;
        GetNumRuleTable()[ --n ]->GetTextNodeList( aTextNodeList );
        for ( SwNumRule::tTextNodeList::iterator aTextNodeIter = aTextNodeList.begin();
              aTextNodeIter != aTextNodeList.end(); ++aTextNodeIter )
        {
            SwTextNode* pTNd = *aTextNodeIter;
            SwIterator<SwTextFrame,SwTextNode> aIter(*pTNd);
            for(SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
                if( pFrame->HasAnimation() )
                    pFrame->StopAnimation( pOut );
        }
    }
}

bool SwDoc::ReplaceNumRule( const SwPosition& rPos,
                            const OUString& rOldRule, const OUString& rNewRule )
{
    bool bRet = false;
    SwNumRule *pOldRule = FindNumRulePtr( rOldRule ),
              *pNewRule = FindNumRulePtr( rNewRule );
    if( pOldRule && pNewRule && pOldRule != pNewRule )
    {
        SwUndoInsNum* pUndo = nullptr;
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            // Start/End for attributes!
            GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );
            pUndo = new SwUndoInsNum( rPos, *pNewRule, rOldRule );
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        SwNumRule::tTextNodeList aTextNodeList;
        pOldRule->GetTextNodeList( aTextNodeList );
        if ( aTextNodeList.size() > 0 )
        {
            SwRegHistory aRegH( pUndo ? pUndo->GetHistory() : nullptr );
            sal_uInt16 nChgFormatLevel = 0;
            for( sal_uInt8 n = 0; n < MAXLEVEL; ++n )
            {
                const SwNumFormat& rOldFormat = pOldRule->Get( n ),
                    & rNewFormat = pNewRule->Get( n );

                if( rOldFormat.GetAbsLSpace() != rNewFormat.GetAbsLSpace() ||
                    rOldFormat.GetFirstLineOffset() != rNewFormat.GetFirstLineOffset() )
                    nChgFormatLevel |= ( 1 << n );
            }

            const SwTextNode* pGivenTextNode = rPos.nNode.GetNode().GetTextNode();
            SwNumRuleItem aRule( rNewRule );
            for ( SwNumRule::tTextNodeList::iterator aIter = aTextNodeList.begin();
                  aIter != aTextNodeList.end(); ++aIter )
            {
                SwTextNode* pTextNd = *aIter;

                if ( pGivenTextNode &&
                     pGivenTextNode->GetListId() == pTextNd->GetListId() )
                {
                    aRegH.RegisterInModify( pTextNd, *pTextNd );

                    pTextNd->SetAttr( aRule );
                    pTextNd->NumRuleChgd();
                }
            }
            GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );
            getIDocumentState().SetModified();

            bRet = true;
        }
    }

    return bRet;
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
              bCreateNewList( false ),
              sListId()
        {}
    };
}

void SwDoc::MakeUniqueNumRules(const SwPaM & rPaM)
{
    OSL_ENSURE( rPaM.GetDoc() == this, "need same doc" );

    ::std::map<SwNumRule *, ListStyleData> aMyNumRuleMap;

    bool bFirst = true;

    const sal_uLong nStt = rPaM.Start()->nNode.GetIndex();
    const sal_uLong nEnd = rPaM.End()->nNode.GetIndex();
    for (sal_uLong n = nStt; n <= nEnd; n++)
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
                                            aListStyleData.sListId, true ));
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
                            aListStyleData.bCreateNewList,
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
        // Set NoNum and Upate
        const SwNodeIndex& rIdx = rPam.GetPoint()->nNode;
        SwTextNode* pNd = rIdx.GetNode().GetTextNode();
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

void SwDoc::DelNumRules( const SwPaM& rPam )
{
    sal_uLong nStt = rPam.GetPoint()->nNode.GetIndex(),
            nEnd = rPam.GetMark()->nNode.GetIndex();
    if( nStt > nEnd )
    {
        sal_uLong nTmp = nStt; nStt = nEnd; nEnd = nTmp;
    }

    SwUndoDelNum* pUndo;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoDelNum( rPam );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }
    else
        pUndo = nullptr;

    SwRegHistory aRegH( pUndo ? pUndo->GetHistory() : nullptr );

    SwNumRuleItem aEmptyRule( aEmptyOUStr );
    const SwNode* pOutlNd = nullptr;
    for( ; nStt <= nEnd; ++nStt )
    {
        SwTextNode* pTNd = GetNodes()[ nStt ]->GetTextNode();
        SwNumRule* pNumRuleOfTextNode = pTNd ? pTNd->GetNumRule() : nullptr;
        if ( pTNd && pNumRuleOfTextNode )
        {
            // recognize changes of attribute for undo
            aRegH.RegisterInModify( pTNd, *pTNd );

            if( pUndo )
                pUndo->AddNode( *pTNd, false );

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
    case ND_ENDNODE:
        bRet = SwTableBoxStartNode == rNd.StartOfSectionNode()->GetStartNodeType() ||
                rNd.StartOfSectionNode()->IsSectionNode();
        break;

    case ND_STARTNODE:
        bRet = SwTableBoxStartNode == static_cast<const SwStartNode&>(rNd).GetStartNodeType();
        break;

    case ND_SECTIONNODE:            // that one's valid, so proceed
        bRet = true;
        break;
    }
    return bRet;
}

static bool lcl_GotoNextPrevNum( SwPosition& rPos, bool bNext,
                            bool bOverUpper, sal_uInt8* pUpper, sal_uInt8* pLower )
{
    const SwTextNode* pNd = rPos.nNode.GetNode().GetTextNode();
    if( !pNd || nullptr == pNd->GetNumRule() )
        return false;

    sal_uInt8 nSrchNum = static_cast<sal_uInt8>(pNd->GetActualListLevel());

    SwNodeIndex aIdx( rPos.nNode );
    if( ! pNd->IsCountedInList() )
    {
        // If NO_NUMLEVEL is switched on, we search the preceding Node with Numbering
        bool bError = false;
        do {
            --aIdx;
            if( aIdx.GetNode().IsTextNode() )
            {
                pNd = aIdx.GetNode().GetTextNode();
                const SwNumRule* pRule = pNd->GetNumRule();

                sal_uInt8 nTmpNum;

                if( pRule  )
                {
                    nTmpNum = static_cast<sal_uInt8>(pNd->GetActualListLevel());
                    if( !( ! pNd->IsCountedInList() &&
                         (nTmpNum >= nSrchNum )) )
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
        ++aIdx;
        pLast = pNd;
    }
    else
    {
        --aIdx;
        pLast = nullptr;
    }

    while( bNext ? ( aIdx.GetIndex() < aIdx.GetNodes().Count() - 1 )
                 : aIdx.GetIndex() != 0 )
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
                    rPos.nNode = aIdx;
                    rPos.nContent.Assign( const_cast<SwTextNode*>(pNd), 0 );
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
            ++aIdx;
        else
            --aIdx;
    }

    if( !bRet && !bOverUpper && pLast )     // do not iterate over higher numbers, but still to the end
    {
        if( bNext )
        {
            rPos.nNode = aIdx;
            if( aIdx.GetNode().IsContentNode() )
                rPos.nContent.Assign( aIdx.GetNode().GetContentNode(), 0 );
        }
        else
        {
            rPos.nNode.Assign( *pLast );
            rPos.nContent.Assign( const_cast<SwTextNode*>(pLast), 0 );
        }
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

bool SwDoc::GotoNextNum( SwPosition& rPos, bool bOverUpper,
                            sal_uInt8* pUpper, sal_uInt8* pLower  )
{
    return ::lcl_GotoNextPrevNum( rPos, true, bOverUpper, pUpper, pLower );
}

const SwNumRule *  SwDoc::SearchNumRule(const SwPosition & rPos,
                                        const bool bForward,
                                        const bool bNum,
                                        const bool bOutline,
                                        int nNonEmptyAllowed,
                                        OUString& sListId,
                                        const bool bInvestigateStartNode)
{
    const SwNumRule * pResult = nullptr;
    SwTextNode * pTextNd = rPos.nNode.GetNode().GetTextNode();
    SwNode * pStartFromNode = pTextNd;

    if (pTextNd)
    {
        SwNodeIndex aIdx(rPos.nNode);

        // - the start node has also been investigated, if requested.
        const SwNode * pNode = nullptr;
        do
        {
            if ( !bInvestigateStartNode )
            {
                if (bForward)
                    ++aIdx;
                else
                    --aIdx;
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
                        pResult = pTextNd->GetNumRule();
                        // provide also the list id, to which the text node belongs.
                        sListId = pTextNd->GetListId();
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
                    ++aIdx;
                else
                    --aIdx;
            }

            pNode = &aIdx.GetNode();
        }
        while (!(pNode == GetNodes().DocumentSectionStartNode(pStartFromNode) ||
                 pNode == GetNodes().DocumentSectionEndNode(pStartFromNode)));
    }

    return pResult;
}

bool SwDoc::GotoPrevNum( SwPosition& rPos, bool bOverUpper,
                            sal_uInt8* pUpper, sal_uInt8* pLower  )
{
    return ::lcl_GotoNextPrevNum( rPos, false, bOverUpper, pUpper, pLower );
}

bool SwDoc::NumUpDown( const SwPaM& rPam, bool bDown )
{
    sal_uLong nStt = rPam.GetPoint()->nNode.GetIndex(),
            nEnd = rPam.GetMark()->nNode.GetIndex();
    if( nStt > nEnd )
    {
        sal_uLong nTmp = nStt; nStt = nEnd; nEnd = nTmp;
    }

    // -> outline nodes are promoted or demoted differently
    bool bOnlyOutline = true;
    bool bOnlyNonOutline = true;
    for (sal_uLong n = nStt; n <= nEnd; n++)
    {
        SwTextNode * pTextNd = GetNodes()[n]->GetTextNode();

        if (pTextNd)
        {
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
        bRet = OutlineUpDown(rPam, nDiff);
    else if (bOnlyNonOutline)
    {
        /* #i24560#
        Only promote or demote if all selected paragraphs are
        promotable resp. demotable.
        */
        for (sal_uLong nTmp = nStt; nTmp <= nEnd; ++nTmp)
        {
            SwTextNode* pTNd = GetNodes()[ nTmp ]->GetTextNode();

            // Make code robust: consider case that the node doesn't denote a
            // text node.
            if ( pTNd )
            {
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
                SwUndo *const pUndo( new SwUndoNumUpDown(rPam, nDiff) );
                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }

            for(sal_uLong nTmp = nStt; nTmp <= nEnd; ++nTmp )
            {
                SwTextNode* pTNd = GetNodes()[ nTmp ]->GetTextNode();

                if( pTNd)
                {
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

bool SwDoc::MoveParagraph( const SwPaM& rPam, long nOffset, bool bIsOutlMv )
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    sal_uLong nStIdx = pStt->nNode.GetIndex();
    sal_uLong nEndIdx = pEnd->nNode.GetIndex();

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
        {   // First is a start node
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

    sal_uLong nInStIdx, nInEndIdx;
    long nOffs = nOffset;
    if( nOffset > 0 )
    {
        nInEndIdx = nEndIdx;
        nEndIdx += nOffset;
        ++nOffs;
    }
    else
    {
        // Impossible to move to negative index
        if( sal_uLong(std::abs( nOffset )) > nStIdx)
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
        sal_uInt16 nRedlPos = getIDocumentRedlineAccess().GetRedlinePos( pStt->nNode.GetNode(), nsRedlineType_t::REDLINE_DELETE );
        if( USHRT_MAX != nRedlPos )
        {
            SwPosition aStPos( *pStt ), aEndPos( *pEnd );
            aStPos.nContent = 0;
            SwContentNode* pCNd = pEnd->nNode.GetNode().GetContentNode();
            aEndPos.nContent = pCNd ? pCNd->Len() : 1;
            bool bCheckDel = true;

            // There is a some Redline Delete Object for the range
            for( ; nRedlPos < getIDocumentRedlineAccess().GetRedlineTable().size(); ++nRedlPos )
            {
                const SwRangeRedline* pTmp = getIDocumentRedlineAccess().GetRedlineTable()[ nRedlPos ];
                if( !bCheckDel || nsRedlineType_t::REDLINE_DELETE == pTmp->GetType() )
                {
                    const SwPosition *pRStt = pTmp->Start(), *pREnd = pTmp->End();
                    switch( ComparePosition( *pRStt, *pREnd, aStPos, aEndPos ))
                    {
                    case POS_COLLIDE_START:
                    case POS_BEHIND:            // Pos1 comes after Pos2
                        nRedlPos = getIDocumentRedlineAccess().GetRedlineTable().size();
                        break;

                    case POS_COLLIDE_END:
                    case POS_BEFORE:            // Pos1 comes before Pos2
                        break;
                    case POS_INSIDE:            // Pos1 is completely inside Pos2
                        // that's valid, but check all following for overlapping
                        bCheckDel = false;
                        break;

                    case POS_OUTSIDE:           // Pos2 is completely inside Pos1
                    case POS_EQUAL:             // Pos1 is equal to Pos2
                    case POS_OVERLAP_BEFORE:    // Pos1 overlaps Pos2 in the beginning
                    case POS_OVERLAP_BEHIND:    // Pos1 overlaps Pos2 at the end
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

    SwNodeIndex aIdx( nOffset > 0 ? pEnd->nNode : pStt->nNode, nOffs );
    SwNodeRange aMvRg( pStt->nNode, 0, pEnd->nNode, +1 );

    SwRangeRedline* pOwnRedl = nullptr;
    if( getIDocumentRedlineAccess().IsRedlineOn() )
    {
        // If the range is completely in the own Redline, we can move it!
        sal_uInt16 nRedlPos = getIDocumentRedlineAccess().GetRedlinePos( pStt->nNode.GetNode(), nsRedlineType_t::REDLINE_INSERT );
        if( USHRT_MAX != nRedlPos )
        {
            SwRangeRedline* pTmp = getIDocumentRedlineAccess().GetRedlineTable()[ nRedlPos ];
            const SwPosition *pRStt = pTmp->Start(), *pREnd = pTmp->End();
            SwRangeRedline aTmpRedl( nsRedlineType_t::REDLINE_INSERT, rPam );
            const SwContentNode* pCEndNd = pEnd->nNode.GetNode().GetContentNode();
            // Is completely in the range and is the own Redline too?
            if( aTmpRedl.IsOwnRedline( *pTmp ) &&
                (pRStt->nNode < pStt->nNode ||
                (pRStt->nNode == pStt->nNode && !pRStt->nContent.GetIndex()) ) &&
                (pEnd->nNode < pREnd->nNode ||
                (pEnd->nNode == pREnd->nNode &&
                 pCEndNd ? pREnd->nContent.GetIndex() == pCEndNd->Len()
                         : !pREnd->nContent.GetIndex() )) )
            {
                pOwnRedl = pTmp;
                if( nRedlPos + 1 < (sal_uInt16)getIDocumentRedlineAccess().GetRedlineTable().size() )
                {
                    pTmp = getIDocumentRedlineAccess().GetRedlineTable()[ nRedlPos+1 ];
                    if( *pTmp->Start() == *pREnd )
                        // then don't!
                        pOwnRedl = nullptr;
                }

                if( pOwnRedl &&
                    !( pRStt->nNode <= aIdx && aIdx <= pREnd->nNode ))
                {
                    // it's not in itself, so don't move it
                    pOwnRedl = nullptr;
                }
            }
        }

        if( !pOwnRedl )
        {
            GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );

            // First the Insert, then the Delete
            SwPosition aInsPos( aIdx );
            aInsPos.nContent.Assign( aIdx.GetNode().GetContentNode(), 0 );

            SwPaM aPam( pStt->nNode, aMvRg.aEnd );

            SwPaM& rOrigPam = (SwPaM&)rPam;
            rOrigPam.DeleteMark();
            rOrigPam.GetPoint()->nNode = aIdx.GetIndex() - 1;

            bool bDelLastPara = !aInsPos.nNode.GetNode().IsContentNode();

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

            getIDocumentContentOperations().CopyRange( aPam, aInsPos, /*bCopyAll=*/false, /*bCheckPos=*/true );
            if( bDelLastPara )
            {
                // We need to remove the last empty Node again
                aIdx = aInsPos.nNode;
                SwContentNode* pCNd = SwNodes::GoPrevious( &aInsPos.nNode );
                aInsPos.nContent.Assign( pCNd, pCNd ? pCNd->Len() : 0 );

                // All, that are in the to-be-deleted Node, need to be
                // moved to the next Node
                for( size_t n = 0; n < getIDocumentRedlineAccess().GetRedlineTable().size(); ++n )
                {
                    SwRangeRedline* pTmp = getIDocumentRedlineAccess().GetRedlineTable()[ n ];

                    SwPosition* pPos;
                    if( ( pPos = &pTmp->GetBound())->nNode == aIdx )
                    {
                        ++pPos->nNode;
                        pPos->nContent.Assign( pPos->nNode.GetNode().GetContentNode(),0);
                    }
                    if( ( pPos = &pTmp->GetBound(false))->nNode == aIdx )
                    {
                        ++pPos->nNode;
                        pPos->nContent.Assign( pPos->nNode.GetNode().GetContentNode(),0);
                    }
                }
                CorrRel( aIdx, aInsPos );

                if (pCNd)
                    pCNd->JoinNext();
            }

            ++rOrigPam.GetPoint()->nNode;
            rOrigPam.GetPoint()->nContent.Assign( rOrigPam.GetContentNode(), 0 );

            RedlineMode_t eOld = getIDocumentRedlineAccess().GetRedlineMode();
            GetDocumentRedlineManager().checkRedlining(eOld);
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                // Still NEEDS to be optimized (even after 14 years)
                getIDocumentRedlineAccess().SetRedlineMode(
                   (RedlineMode_t)(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE));
                SwUndo *const pUndo(new SwUndoRedlineDelete(aPam, UNDO_DELETE));
                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }

            SwRangeRedline* pNewRedline = new SwRangeRedline( nsRedlineType_t::REDLINE_DELETE, aPam );

            // prevent assertion from aPam's target being deleted
            // (Alternatively, one could just let aPam go out of scope, but
            // that requires touching a lot of code.)
            aPam.GetBound().nContent.Assign( nullptr, 0 );
            aPam.GetBound(false).nContent.Assign( nullptr, 0 );

            getIDocumentRedlineAccess().AppendRedline( pNewRedline, true );

            // Still NEEDS to be optimized!
            getIDocumentRedlineAccess().SetRedlineMode( eOld );
            GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );
            getIDocumentState().SetModified();

            return true;
        }
    }

    if( !pOwnRedl && !getIDocumentRedlineAccess().IsIgnoreRedline() && !getIDocumentRedlineAccess().GetRedlineTable().empty() )
    {
        SwPaM aTemp(aIdx);
        getIDocumentRedlineAccess().SplitRedline(aTemp);
    }

    sal_uLong nRedlSttNd(0), nRedlEndNd(0);
    if( pOwnRedl )
    {
        const SwPosition *pRStt = pOwnRedl->Start(), *pREnd = pOwnRedl->End();
        nRedlSttNd = pRStt->nNode.GetIndex();
        nRedlEndNd = pREnd->nNode.GetIndex();
    }

    SwUndoMoveNum* pUndo = nullptr;
    sal_uLong nMoved = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoMoveNum( rPam, nOffset, bIsOutlMv );
        nMoved = rPam.End()->nNode.GetIndex() - rPam.Start()->nNode.GetIndex() + 1;
    }

    getIDocumentContentOperations().MoveNodeRange( aMvRg, aIdx, SwMoveFlags::REDLINES );

    if( pUndo )
    {
        // i57907: Under circumstances (sections at the end of a chapter)
        // the rPam.Start() is not moved to the new position.
        // But aIdx should be at the new end position and as long as the
        // number of moved paragraphs is nMoved, I know, where the new
        // position is.
        pUndo->SetStartNode( aIdx.GetIndex() - nMoved );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if( pOwnRedl )
    {
        SwPosition *pRStt = pOwnRedl->Start(), *pREnd = pOwnRedl->End();
        if( pRStt->nNode.GetIndex() != nRedlSttNd )
        {
            pRStt->nNode = nRedlSttNd;
            pRStt->nContent.Assign( pRStt->nNode.GetNode().GetContentNode(),0);
        }
        if( pREnd->nNode.GetIndex() != nRedlEndNd )
        {
            pREnd->nNode = nRedlEndNd;
            SwContentNode* pCNd = pREnd->nNode.GetNode().GetContentNode();
            pREnd->nContent.Assign( pCNd, pCNd ? pCNd->Len() : 0 );
        }
    }

    getIDocumentState().SetModified();
    return true;
}

bool SwDoc::NumOrNoNum( const SwNodeIndex& rIdx, bool bDel )
{
    bool bResult = false;
    SwTextNode * pTextNd = rIdx.GetNode().GetTextNode();

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
                SwUndoNumOrNoNum * pUndo =
                    new SwUndoNumOrNoNum(rIdx, bOldNum, bNewNum);

                GetIDocumentUndoRedo().AppendUndo(pUndo);
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

SwNumRule* SwDoc::GetNumRuleAtPos( const SwPosition& rPos )
{
    SwNumRule* pRet = nullptr;
    SwTextNode* pTNd = rPos.nNode.GetNode().GetTextNode();

    if ( pTNd != nullptr )
    {
        pRet = pTNd->GetNumRule();
    }

    return pRet;
}

sal_uInt16 SwDoc::FindNumRule( const OUString& rName ) const
{
    for( sal_uInt16 n = mpNumRuleTable->size(); n; )
        if( (*mpNumRuleTable)[ --n ]->GetName() == rName )
            return n;

    return USHRT_MAX;
}

SwNumRule* SwDoc::FindNumRulePtr( const OUString& rName ) const
{
    SwNumRule * pResult = nullptr;

    pResult = maNumRuleMap[rName];

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
        pNew->CheckCharFormats( this );
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
        SwUndo * pUndo = new SwUndoNumruleCreate(pNew, this);
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if (bBroadcast)
        BroadcastStyleOperation(pNew->GetName(), SFX_STYLE_FAMILY_PSEUDO,
                                SfxStyleSheetHintId::CREATED);

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
            static rtlRandomPool s_RandomPool( rtl_random_createPool() );
            sal_Int64 n;
            rtl_random_getBytes( s_RandomPool, &n, sizeof(n) );
            aName = OUString::number( (n < 0 ? -n : n) );
        }
        if( pChkStr && pChkStr->isEmpty() )
            pChkStr = nullptr;
    }
    else if( pChkStr && !pChkStr->isEmpty() )
        aName = *pChkStr;
    else
    {
        pChkStr = nullptr;
        aName = SW_RESSTR( STR_NUMRULE_DEFNAME );
    }

    sal_uInt16 nNum(0), nTmp, nFlagSize = ( mpNumRuleTable->size() / 8 ) +2;
    sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
    memset( pSetFlags, 0, nFlagSize );

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

    const SwNumRule* pNumRule;
    sal_uInt16 n;

    for( n = 0; n < mpNumRuleTable->size(); ++n )
        if( nullptr != ( pNumRule = (*mpNumRuleTable)[ n ] ) )
        {
            const OUString sNm = pNumRule->GetName();
            if( sNm.startsWith( aName ) )
            {
                // Determine Number and set the Flag
                nNum = (sal_uInt16)sNm.copy( nNmLen ).toInt32();
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
        for( n = 0; n < nFlagSize; ++n )
            if( 0xff != ( nTmp = pSetFlags[ n ] ))
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
    delete [] pSetFlags;
    if( pChkStr && !pChkStr->isEmpty() )
        return *pChkStr;
    return aName + OUString::number( ++nNum );
}

void SwDoc::UpdateNumRule()
{
    const SwNumRuleTable& rNmTable = GetNumRuleTable();
    for( size_t n = 0; n < rNmTable.size(); ++n )
        if( rNmTable[ n ]->IsInvalidRule() )
            rNmTable[ n ]->Validate();
}

void SwDoc::MarkListLevel( const OUString& sListId,
                           const int nListLevel,
                           const bool bValue )
{
    SwList* pList = getIDocumentListsAccess().getListByName( sListId );

    if ( pList )
    {
        MarkListLevel( *pList, nListLevel, bValue );
    }
}

void SwDoc::MarkListLevel( SwList& rList,
                           const int nListLevel,
                           const bool bValue )
{
    // Set new marked list level and notify all affected nodes of the changed mark.
    rList.MarkListLevel( nListLevel, bValue );
}

bool SwDoc::IsFirstOfNumRuleAtPos( const SwPosition & rPos )
{
    bool bResult = false;

    const SwTextNode* pTextNode = rPos.nNode.GetNode().GetTextNode();
    if ( pTextNode != nullptr )
    {
        bResult = pTextNode->IsFirstOfNumRule();
    }

    return bResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
