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

#include <UndoDelete.hxx>
#include <hintids.hxx>
#include <unotools/charclass.hxx>
#include <editeng/formatbreakitem.hxx>
#include <fmtpdsc.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <doc.hxx>
#include <UndoManager.hxx>
#include <swtable.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <poolfmt.hxx>
#include <mvsave.hxx>
#include <redline.hxx>
#include <docary.hxx>
#include <sfx2/app.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <comcore.hrc> // #111827#
#include <undo.hrc>
#include <vector>

// DELETE
/*  lcl_MakeAutoFrms has to call MakeFrms for objects bounded "AtChar"
    ( == AUTO ), if the anchor frame has be moved via _MoveNodes(..) and
    DelFrms(..)
*/
static void lcl_MakeAutoFrms( const SwFrmFmts& rSpzArr, sal_uLong nMovedIndex )
{
    if( !rSpzArr.empty() )
    {
        SwFlyFrmFmt* pFmt;
        const SwFmtAnchor* pAnchor;
        for( sal_uInt16 n = 0; n < rSpzArr.size(); ++n )
        {
            pFmt = (SwFlyFrmFmt*)rSpzArr[n];
            pAnchor = &pFmt->GetAnchor();
            if (pAnchor->GetAnchorId() == FLY_AT_CHAR)
            {
                const SwPosition* pAPos = pAnchor->GetCntntAnchor();
                if( pAPos && nMovedIndex == pAPos->nNode.GetIndex() )
                    pFmt->MakeFrms();
            }
        }
    }
}

// SwUndoDelete has to perform a deletion and to record anything that is needed
// to restore the situation before the deletion. Unfortunately a part of the
// deletion will be done after calling this Ctor, this has to be kept in mind!
// In this Ctor only the complete paragraphs will be deleted, the joining of
// the first and last paragraph of the selection will be handled outside this
// function.
// Here are the main steps of the function:
// 1. Deletion/recording of content indizes of the selection: footnotes, fly
//    frames and bookmarks
// Step 1 could shift all nodes by deletion of footnotes => nNdDiff will be set.
// 2. If the paragraph where the selection ends, is the last content of a
//    section so that this section becomes empty when the paragraphs will be
//    joined we have to do some smart actions ;-) The paragraph will be moved
//    outside the section and replaced by a dummy text node, the complete
//    section will be deleted in step 3. The difference between replacement
//    dummy and original is nReplacementDummy.
// 3. Moving complete selected nodes into the UndoArray. Before this happens the
//    selection has to be extended if there are sections which would become
//    empty otherwise. BTW: sections will be moved into the UndoArray if they
//    are complete part of the selection. Sections starting or ending outside
//    of the selection will not be removed from the DocNodeArray even they got
//    a "dummy"-copy in the UndoArray.
// 4. We have to anticipate the joining of the two paragraphs if the start
//    paragraph is inside a section and the end paragraph not. Then we have to
//    move the paragraph into this section and to record this in nSectDiff.
SwUndoDelete::SwUndoDelete( SwPaM& rPam, sal_Bool bFullPara, sal_Bool bCalledByTblCpy )
    : SwUndo(UNDO_DELETE), SwUndRng( rPam ),
    pMvStt( 0 ), pSttStr(0), pEndStr(0), pRedlData(0), pRedlSaveData(0),
    nNode(0), nNdDiff(0), nSectDiff(0), nReplaceDummy(0), nSetPos(0),
    bGroup( sal_False ), bBackSp( sal_False ), bJoinNext( sal_False ), bTblDelLastNd( sal_False ),
    bDelFullPara( bFullPara ), bResetPgDesc( sal_False ), bResetPgBrk( sal_False ),
    bFromTableCopy( bCalledByTblCpy )
{
    // bFullPara is set e.g. if an empty paragraph before a table is deleted
    bDelFullPara = bFullPara;

    bCacheComment = false;

    SwDoc * pDoc = rPam.GetDoc();

    if( !pDoc->IsIgnoreRedline() && !pDoc->GetRedlineTbl().empty() )
    {
        pRedlSaveData = new SwRedlineSaveDatas;
        if( !FillSaveData( rPam, *pRedlSaveData ))
            delete pRedlSaveData, pRedlSaveData = 0;
    }

    if( !pHistory )
        pHistory = new SwHistory;

    // delete all footnotes for now
    const SwPosition *pStt = rPam.Start(),
                    *pEnd = rPam.GetPoint() == pStt
                        ? rPam.GetMark()
                        : rPam.GetPoint();

    // Step 1. deletion/record of content indizes
    if( bDelFullPara )
    {
        OSL_ENSURE( rPam.HasMark(), "PaM ohne Mark" );
        DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint(),
                        DelCntntType(nsDelCntntType::DELCNT_ALL | nsDelCntntType::DELCNT_CHKNOCNTNT) );

        ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
        _DelBookmarks(pStt->nNode, pEnd->nNode);
    }
    else
        DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint() );

    nSetPos = pHistory ? pHistory->Count() : 0;

    // Is already anything deleted?
    nNdDiff = nSttNode - pStt->nNode.GetIndex();

    bJoinNext = !bFullPara && pEnd == rPam.GetPoint();
    bBackSp = !bFullPara && !bJoinNext;

    SwTxtNode *pSttTxtNd = 0, *pEndTxtNd = 0;
    if( !bFullPara )
    {
        pSttTxtNd = pStt->nNode.GetNode().GetTxtNode();
        pEndTxtNd = nSttNode == nEndNode
                    ? pSttTxtNd
                    : pEnd->nNode.GetNode().GetTxtNode();
    }

    bool bMoveNds = *pStt == *pEnd      // any area still existent?
                ? false
                : ( SaveCntnt( pStt, pEnd, pSttTxtNd, pEndTxtNd ) || bFromTableCopy );

    if( pSttTxtNd && pEndTxtNd && pSttTxtNd != pEndTxtNd )
    {
        // two different TextNodes, thus save also the TextFormatCollection
        pHistory->Add( pSttTxtNd->GetTxtColl(),pStt->nNode.GetIndex(), ND_TEXTNODE );
        pHistory->Add( pEndTxtNd->GetTxtColl(),pEnd->nNode.GetIndex(), ND_TEXTNODE );

        if( !bJoinNext )        // Selection from bottom to top
        {
            // When using JoinPrev() all AUTO-PageBreak's will be copied
            // correctly. To restore them with UNDO, Auto-PageBreak of the
            // EndNode needs to be reset. Same for PageDesc and ColBreak.
            if( pEndTxtNd->HasSwAttrSet() )
            {
                SwRegHistory aRegHist( *pEndTxtNd, pHistory );
                if( SFX_ITEM_SET == pEndTxtNd->GetpSwAttrSet()->GetItemState(
                        RES_BREAK, sal_False ) )
                    pEndTxtNd->ResetAttr( RES_BREAK );
                if( pEndTxtNd->HasSwAttrSet() &&
                    SFX_ITEM_SET == pEndTxtNd->GetpSwAttrSet()->GetItemState(
                        RES_PAGEDESC, sal_False ) )
                    pEndTxtNd->ResetAttr( RES_PAGEDESC );
            }
        }
    }

    // Move now also the PaM. The SPoint is at the beginning of a SSelection.
    if( pEnd == rPam.GetPoint() && ( !bFullPara || pSttTxtNd || pEndTxtNd ) )
        rPam.Exchange();

    if( !pSttTxtNd && !pEndTxtNd )
        rPam.GetPoint()->nNode--;
    rPam.DeleteMark();          // the SPoint is in the selection

    if( !pEndTxtNd )
        nEndCntnt = 0;
    if( !pSttTxtNd )
        nSttCntnt = 0;

    if( bMoveNds )      // Do Nodes exist that need to be moved?
    {
        SwNodes& rNds = pDoc->GetUndoManager().GetUndoNodes();
        SwNodes& rDocNds = pDoc->GetNodes();
        SwNodeRange aRg( rDocNds, nSttNode - nNdDiff,
                         rDocNds, nEndNode - nNdDiff );
        if( !bFullPara && !pEndTxtNd &&
            &aRg.aEnd.GetNode() != &pDoc->GetNodes().GetEndOfContent() )
        {
            SwNode* pNode = aRg.aEnd.GetNode().StartOfSectionNode();
            if( pNode->GetIndex() >= nSttNode - nNdDiff )
                aRg.aEnd++; // Deletion of a complete table
        }
        SwNode* pTmpNd;
        // Step 2: Expand selection if necessary
        if( bJoinNext || bFullPara )
        {
            // If all content of a section will be moved into Undo, the section
            // itself should be moved completely.
            while( aRg.aEnd.GetIndex() + 2  < rDocNds.Count() &&
                ( (pTmpNd = rDocNds[ aRg.aEnd.GetIndex()+1 ])->IsEndNode() &&
                pTmpNd->StartOfSectionNode()->IsSectionNode() &&
                pTmpNd->StartOfSectionNode()->GetIndex() >= aRg.aStart.GetIndex() ) )
                aRg.aEnd++;
            nReplaceDummy = aRg.aEnd.GetIndex() + nNdDiff - nEndNode;
            if( nReplaceDummy )
            {   // The selection has been expanded, because
                aRg.aEnd++;
                if( pEndTxtNd )
                {
                    // The end text node has to leave the (expanded) selection
                    // The dummy is needed because _MoveNodes deletes empty
                    // sections
                    ++nReplaceDummy;
                    SwNodeRange aMvRg( *pEndTxtNd, 0, *pEndTxtNd, 1 );
                    SwPosition aSplitPos( *pEndTxtNd );
                    ::sw::UndoGuard const ug(pDoc->GetIDocumentUndoRedo());
                    pDoc->SplitNode( aSplitPos, false );
                    rDocNds._MoveNodes( aMvRg, rDocNds, aRg.aEnd, sal_True );
                    aRg.aEnd--;
                }
                else
                    nReplaceDummy = 0;
            }
        }
        if( bBackSp || bFullPara )
        {
            // See above, the selection has to be expanded if there are "nearly
            // empty" sections and a replacement dummy has to be set if needed.
            while( 1 < aRg.aStart.GetIndex() &&
                ( (pTmpNd = rDocNds[ aRg.aStart.GetIndex()-1 ])->IsSectionNode() &&
                pTmpNd->EndOfSectionIndex() < aRg.aEnd.GetIndex() ) )
                aRg.aStart--;
            if( pSttTxtNd )
            {
                nReplaceDummy = nSttNode - nNdDiff - aRg.aStart.GetIndex();
                if( nReplaceDummy )
                {
                    SwNodeRange aMvRg( *pSttTxtNd, 0, *pSttTxtNd, 1 );
                    SwPosition aSplitPos( *pSttTxtNd );
                    ::sw::UndoGuard const ug(pDoc->GetIDocumentUndoRedo());
                    pDoc->SplitNode( aSplitPos, false );
                    rDocNds._MoveNodes( aMvRg, rDocNds, aRg.aStart, sal_True );
                    aRg.aStart--;
                }
            }
        }

        if( bFromTableCopy )
        {
            if( !pEndTxtNd )
            {
                if( pSttTxtNd )
                    aRg.aStart++;
                else if( !bFullPara && !aRg.aEnd.GetNode().IsCntntNode() )
                    aRg.aEnd--;
            }
        }
        else if (pSttTxtNd && (pEndTxtNd || pSttTxtNd->GetTxt().getLength()))
            aRg.aStart++;

        // Step 3: Moving into UndoArray...
        nNode = rNds.GetEndOfContent().GetIndex();
        rDocNds._MoveNodes( aRg, rNds, SwNodeIndex( rNds.GetEndOfContent() ));
        pMvStt = new SwNodeIndex( rNds, nNode );
        // remember difference!
        nNode = rNds.GetEndOfContent().GetIndex() - nNode;

        if( pSttTxtNd && pEndTxtNd )
        {
            //Step 4: Moving around sections
            nSectDiff = aRg.aEnd.GetIndex() - aRg.aStart.GetIndex();
            // nSect is the number of sections which starts(ends) between start
            // and end node of the selection. The "loser" paragraph has to be
            // moved into the section(s) of the "winner" paragraph
            if( nSectDiff )
            {
                if( bJoinNext )
                {
                    SwNodeRange aMvRg( *pEndTxtNd, 0, *pEndTxtNd, 1 );
                    rDocNds._MoveNodes( aMvRg, rDocNds, aRg.aStart, sal_True );
                }
                else
                {
                    SwNodeRange aMvRg( *pSttTxtNd, 0, *pSttTxtNd, 1 );
                    rDocNds._MoveNodes( aMvRg, rDocNds, aRg.aEnd, sal_True );
                }
            }
        }
        if( nSectDiff || nReplaceDummy )
            lcl_MakeAutoFrms( *pDoc->GetSpzFrmFmts(),
                bJoinNext ? pEndTxtNd->GetIndex() : pSttTxtNd->GetIndex() );
    }
    else
        nNode = 0;      // moved no node -> no difference at the end

    // Are there any Nodes that got deleted before that (FootNotes
    // have ContentNodes)?
    if( !pSttTxtNd && !pEndTxtNd )
    {
        nNdDiff = nSttNode - rPam.GetPoint()->nNode.GetIndex() - (bFullPara ? 0 : 1);
        rPam.Move( fnMoveForward, fnGoNode );
    }
    else
    {
        nNdDiff = nSttNode;
        if( nSectDiff && bBackSp )
            nNdDiff += nSectDiff;
        nNdDiff -= rPam.GetPoint()->nNode.GetIndex();
    }

    if( !rPam.GetNode()->IsCntntNode() )
        rPam.GetPoint()->nContent.Assign( 0, 0 );

    // is a history necessary here at all?
    if( pHistory && !pHistory->Count() )
        DELETEZ( pHistory );
}

sal_Bool SwUndoDelete::SaveCntnt( const SwPosition* pStt, const SwPosition* pEnd,
                    SwTxtNode* pSttTxtNd, SwTxtNode* pEndTxtNd )
{
    sal_uLong nNdIdx = pStt->nNode.GetIndex();
    // 1 - copy start in Start-String
    if( pSttTxtNd )
    {
        bool bOneNode = nSttNode == nEndNode;
        xub_StrLen nLen = bOneNode ? nEndCntnt - nSttCntnt
                                : pSttTxtNd->GetTxt().getLength() - nSttCntnt;
        SwRegHistory aRHst( *pSttTxtNd, pHistory );
        // always save all text atttibutes because of possibly overlapping
        // areas of on/off
        pHistory->CopyAttr( pSttTxtNd->GetpSwpHints(), nNdIdx,
                            0, pSttTxtNd->GetTxt().getLength(), true );
        if( !bOneNode && pSttTxtNd->HasSwAttrSet() )
                pHistory->CopyFmtAttr( *pSttTxtNd->GetpSwAttrSet(), nNdIdx );

        // the length might have changed (!!Fields!!)
        nLen = ((bOneNode)
                    ? pEnd->nContent.GetIndex()
                    : pSttTxtNd->GetTxt().getLength())
            - pStt->nContent.GetIndex();

        // delete now also the text (all attribute changes are added to
        // UNDO history)
        pSttStr = new String( pSttTxtNd->GetTxt().copy(nSttCntnt, nLen));
        pSttTxtNd->EraseText( pStt->nContent, nLen );
        if( pSttTxtNd->GetpSwpHints() )
            pSttTxtNd->GetpSwpHints()->DeRegister();

        // METADATA: store
        bool emptied( pSttStr->Len() && !pSttTxtNd->Len() );
        if (!bOneNode || emptied) // merging may overwrite xmlids...
        {
            m_pMetadataUndoStart = (emptied)
                ? pSttTxtNd->CreateUndoForDelete()
                : pSttTxtNd->CreateUndo();
        }

        if( bOneNode )
            return sal_False;           // stop moving more nodes
    }

    // 2 - copy end into End-String
    if( pEndTxtNd )
    {
        SwIndex aEndIdx( pEndTxtNd );
        nNdIdx = pEnd->nNode.GetIndex();
        SwRegHistory aRHst( *pEndTxtNd, pHistory );

        // always save all text atttibutes because of possibly overlapping
        // areas of on/off
        pHistory->CopyAttr( pEndTxtNd->GetpSwpHints(), nNdIdx, 0,
                            pEndTxtNd->GetTxt().getLength(), true );

        if( pEndTxtNd->HasSwAttrSet() )
            pHistory->CopyFmtAttr( *pEndTxtNd->GetpSwAttrSet(), nNdIdx );

        // delete now also the text (all attribute changes are added to
        // UNDO history)
        pEndStr = new String( pEndTxtNd->GetTxt().copy( 0,
                                    pEnd->nContent.GetIndex() ));
        pEndTxtNd->EraseText( aEndIdx, pEnd->nContent.GetIndex() );
        if( pEndTxtNd->GetpSwpHints() )
            pEndTxtNd->GetpSwpHints()->DeRegister();

        // METADATA: store
        bool emptied( pEndStr->Len() && !pEndTxtNd->Len() );

        m_pMetadataUndoEnd = (emptied)
            ? pEndTxtNd->CreateUndoForDelete()
            : pEndTxtNd->CreateUndo();
    }

    // if there are only two Nodes than we're done
    if( ( pSttTxtNd || pEndTxtNd ) && nSttNode + 1 == nEndNode )
        return sal_False;           // do not move any Node

    return sal_True;                // move Nodes lying in between
}

sal_Bool SwUndoDelete::CanGrouping( SwDoc* pDoc, const SwPaM& rDelPam )
{
    // Is Undo greater than one Node (that is Start and EndString)?
    if( pSttStr ? !pSttStr->Len() || pEndStr : sal_True )
        return sal_False;

    // only the deletion of single char's can be condensed
    if( nSttNode != nEndNode || ( !bGroup && nSttCntnt+1 != nEndCntnt ))
        return sal_False;

    const SwPosition *pStt = rDelPam.Start(),
                    *pEnd = rDelPam.GetPoint() == pStt
                        ? rDelPam.GetMark()
                        : rDelPam.GetPoint();

    if( pStt->nNode != pEnd->nNode ||
        pStt->nContent.GetIndex()+1 != pEnd->nContent.GetIndex() ||
        pEnd->nNode != nSttNode )
        return sal_False;

    // Distinguish between BackSpace and Delete because the Undo array needs to
    // be constructed differently!
    if( pEnd->nContent == nSttCntnt )
    {
        if( bGroup && !bBackSp ) return sal_False;
        bBackSp = sal_True;
    }
    else if( pStt->nContent == nSttCntnt )
    {
        if( bGroup && bBackSp ) return sal_False;
        bBackSp = sal_False;
    }
    else
        return sal_False;

    // are both Nodes (Node/Undo array) TextNodes at all?
    SwTxtNode * pDelTxtNd = pStt->nNode.GetNode().GetTxtNode();
    if( !pDelTxtNd ) return sal_False;

    xub_StrLen nUChrPos = bBackSp ? 0 : pSttStr->Len()-1;
    sal_Unicode cDelChar = pDelTxtNd->GetTxt()[ pStt->nContent.GetIndex() ];
    CharClass& rCC = GetAppCharClass();
    if( ( CH_TXTATR_BREAKWORD == cDelChar || CH_TXTATR_INWORD == cDelChar ) ||
        rCC.isLetterNumeric( OUString( cDelChar ), 0 ) !=
        rCC.isLetterNumeric( *pSttStr, nUChrPos ) )
        return sal_False;

    {
        SwRedlineSaveDatas* pTmpSav = new SwRedlineSaveDatas;
        if( !FillSaveData( rDelPam, *pTmpSav, sal_False ))
            delete pTmpSav, pTmpSav = 0;

        bool bOk = ( !pRedlSaveData && !pTmpSav ) ||
                   ( pRedlSaveData && pTmpSav &&
                SwUndo::CanRedlineGroup( *pRedlSaveData, *pTmpSav, bBackSp ));
        delete pTmpSav;
        if( !bOk )
            return sal_False;

        pDoc->DeleteRedline( rDelPam, false, USHRT_MAX );
    }

    // Both 'deletes' can be consolidated, so 'move' the related character
    if( bBackSp )
        nSttCntnt--;    // BackSpace: add char to array!
    else
    {
        nEndCntnt++;    // Delete: attach char at the end
        nUChrPos++;
    }
    pSttStr->Insert( cDelChar, nUChrPos );
    pDelTxtNd->EraseText( pStt->nContent, 1 );

    bGroup = sal_True;
    return sal_True;
}

SwUndoDelete::~SwUndoDelete()
{
    delete pSttStr;
    delete pEndStr;
    if( pMvStt )        // Delete also the selection from UndoNodes array
    {
        // Insert saves content in IconSection
        pMvStt->GetNode().GetNodes().Delete( *pMvStt, nNode );
        delete pMvStt;
    }
    delete pRedlData;
    delete pRedlSaveData;
}

static SwRewriter lcl_RewriterFromHistory(SwHistory & rHistory)
{
    SwRewriter aRewriter;

    bool bDone = false;

    for ( sal_uInt16 n = 0; n < rHistory.Count(); n++)
    {
        String aDescr = rHistory[n]->GetDescription();

        if (aDescr.Len() > 0)
        {
            aRewriter.AddRule(UndoArg2, aDescr);

            bDone = true;
            break;
        }
    }

    if (! bDone)
    {
        aRewriter.AddRule(UndoArg2, SW_RESSTR(STR_FIELD));
    }

    return aRewriter;
}

static bool lcl_IsSpecialCharacter(sal_Unicode nChar)
{
    switch (nChar)
    {
    case CH_TXTATR_BREAKWORD:
    case CH_TXTATR_INWORD:
    case CH_TXTATR_TAB:
    case CH_TXTATR_NEWLINE:
        return true;

    default:
        break;
    }

    return false;
}

static String lcl_DenotedPortion(String rStr, xub_StrLen nStart,
                                 xub_StrLen nEnd)
{
    String aResult;

    if (nEnd - nStart > 0)
    {
        sal_Unicode cLast = rStr.GetChar(nEnd - 1);
        if (lcl_IsSpecialCharacter(cLast))
        {
            switch(cLast)
            {
            case CH_TXTATR_TAB:
                aResult = SW_RESSTR(STR_UNDO_TABS);

                break;
            case CH_TXTATR_NEWLINE:
                aResult = SW_RESSTR(STR_UNDO_NLS);

                break;

            case CH_TXTATR_INWORD:
            case CH_TXTATR_BREAKWORD:
                aResult = SwRewriter::GetPlaceHolder(UndoArg2);
                break;

            }
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1,
                              OUString::number(nEnd - nStart));
            aResult = aRewriter.Apply(aResult);
        }
        else
        {
            aResult = SW_RESSTR(STR_START_QUOTE);
            aResult += rStr.Copy(nStart, nEnd - nStart);
            aResult += SW_RESSTR(STR_END_QUOTE);
        }
    }

    return aResult;
}

String DenoteSpecialCharacters(const String & rStr)
{
    String aResult;

    if (rStr.Len() > 0)
    {
        bool bStart = false;
        xub_StrLen nStart = 0;
        sal_Unicode cLast = 0;

        for (xub_StrLen i = 0; i < rStr.Len(); i++)
        {
            if (lcl_IsSpecialCharacter(rStr.GetChar(i)))
            {
                if (cLast != rStr.GetChar(i))
                    bStart = true;

            }
            else
            {
                if (lcl_IsSpecialCharacter(cLast))
                    bStart = true;
            }

            if (bStart)
            {
                aResult += lcl_DenotedPortion(rStr, nStart, i);

                nStart = i;
                bStart = false;
            }

            cLast = rStr.GetChar(i);
        }

        aResult += lcl_DenotedPortion(rStr, nStart, rStr.Len());
    }
    else
        aResult = SwRewriter::GetPlaceHolder(UndoArg2);

    return aResult;
}

SwRewriter SwUndoDelete::GetRewriter() const
{
    SwRewriter aResult;

    if (nNode != 0)
    {
        if (sTableName.Len() > 0)
        {

            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, SW_RESSTR(STR_START_QUOTE));
            aRewriter.AddRule(UndoArg2, sTableName);
            aRewriter.AddRule(UndoArg3, SW_RESSTR(STR_END_QUOTE));

            String sTmp = aRewriter.Apply(SW_RES(STR_TABLE_NAME));
            aResult.AddRule(UndoArg1, sTmp);
        }
        else
            aResult.AddRule(UndoArg1, SW_RESSTR(STR_PARAGRAPHS));
    }
    else
    {
        String aStr;

        if (pSttStr != NULL && pEndStr != NULL && pSttStr->Len() == 0 &&
            pEndStr->Len() == 0)
        {
            aStr = SW_RESSTR(STR_PARAGRAPH_UNDO);
        }
        else
        {
            String * pStr = NULL;
            if (pSttStr != NULL)
                pStr = pSttStr;
            else if (pEndStr != NULL)
                pStr = pEndStr;

            if (pStr != NULL)
            {
                aStr = DenoteSpecialCharacters(*pStr);
            }
            else
            {
                aStr = SwRewriter::GetPlaceHolder(UndoArg2);
            }
        }

        aStr = ShortenString(aStr, nUndoStringLength, String(SW_RES(STR_LDOTS)));
        if (pHistory)
        {
            SwRewriter aRewriter = lcl_RewriterFromHistory(*pHistory);
            aStr = aRewriter.Apply(aStr);
        }

        aResult.AddRule(UndoArg1, aStr);
    }

    return aResult;
}

// Every object, anchored "AtCntnt" will be reanchored at rPos
static void lcl_ReAnchorAtCntntFlyFrames( const SwFrmFmts& rSpzArr, SwPosition &rPos, sal_uLong nOldIdx )
{
    if( !rSpzArr.empty() )
    {
        SwFlyFrmFmt* pFmt;
        const SwFmtAnchor* pAnchor;
        const SwPosition* pAPos;
        for( sal_uInt16 n = 0; n < rSpzArr.size(); ++n )
        {
            pFmt = (SwFlyFrmFmt*)rSpzArr[n];
            pAnchor = &pFmt->GetAnchor();
            if (pAnchor->GetAnchorId() == FLY_AT_PARA)
            {
                pAPos =  pAnchor->GetCntntAnchor();
                if( pAPos && nOldIdx == pAPos->nNode.GetIndex() )
                {
                    SwFmtAnchor aAnch( *pAnchor );
                    aAnch.SetAnchor( &rPos );
                    pFmt->SetFmtAttr( aAnch );
                }
            }
        }
    }
}

void SwUndoDelete::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();

    sal_uLong nCalcStt = nSttNode - nNdDiff;

    if( nSectDiff && bBackSp )
        nCalcStt += nSectDiff;

    SwNodeIndex aIdx( pDoc->GetNodes(), nCalcStt );
    SwNode* pInsNd = &aIdx.GetNode();

    {   // code block so that SwPosition is detached when deleting a Node
        SwPosition aPos( aIdx );
        if( !bDelFullPara )
        {
            if( pInsNd->IsTableNode() )
            {
                pInsNd = pDoc->GetNodes().MakeTxtNode( aIdx,
                        (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl() );
                aIdx--;
                aPos.nNode = aIdx;
                aPos.nContent.Assign( pInsNd->GetCntntNode(), nSttCntnt );
            }
            else
            {
                if( pInsNd->IsCntntNode() )
                    aPos.nContent.Assign( (SwCntntNode*)pInsNd, nSttCntnt );
                if( !bTblDelLastNd )
                    pInsNd = 0;         // do not delete Node!
            }
        }
        else
            pInsNd = 0;         // do not delete Node!

        bool bNodeMove = 0 != nNode;

        if( pEndStr )
        {
            // discard attributes since they all saved!
            SwTxtNode* pTxtNd = aPos.nNode.GetNode().GetTxtNode();

            if( pTxtNd && pTxtNd->HasSwAttrSet() )
                pTxtNd->ResetAllAttr();

            if( pTxtNd && pTxtNd->GetpSwpHints() )
                pTxtNd->ClearSwpHintsArr( true );

            if( pSttStr && !bFromTableCopy )
            {
                sal_uLong nOldIdx = aPos.nNode.GetIndex();
                pDoc->SplitNode( aPos, false );
                // After the split all objects are anchored at the first
                // paragraph, but the pHistory of the fly frame formats relies
                // on anchoring at the start of the selection
                // => selection backwards needs a correction.
                if( bBackSp )
                    lcl_ReAnchorAtCntntFlyFrames( *pDoc->GetSpzFrmFmts(), aPos, nOldIdx );
                pTxtNd = aPos.nNode.GetNode().GetTxtNode();
            }
            if( pTxtNd )
            {
                OUString const ins( pTxtNd->InsertText(*pEndStr, aPos.nContent,
                        IDocumentContentOperations::INS_NOHINTEXPAND) );
                assert(ins.getLength() == pEndStr->Len()); // must succeed
                // METADATA: restore
                pTxtNd->RestoreMetadata(m_pMetadataUndoEnd);
            }
        }
        else if( pSttStr && bNodeMove )
        {
            SwTxtNode * pNd = aPos.nNode.GetNode().GetTxtNode();
            if( pNd )
            {
                if (nSttCntnt < pNd->GetTxt().getLength())
                {
                    sal_uLong nOldIdx = aPos.nNode.GetIndex();
                    pDoc->SplitNode( aPos, false );
                    if( bBackSp )
                        lcl_ReAnchorAtCntntFlyFrames( *pDoc->GetSpzFrmFmts(), aPos, nOldIdx );
                }
                else
                    aPos.nNode++;
            }
        }
        SwNode* pMovedNode = NULL;
        if( nSectDiff )
        {
            sal_uLong nMoveIndex = aPos.nNode.GetIndex();
            int nDiff = 0;
            if( bJoinNext )
            {
                nMoveIndex += nSectDiff + 1;
                pMovedNode = &aPos.nNode.GetNode();
            }
            else
            {
                nMoveIndex -= nSectDiff + 1;
                ++nDiff;
            }
            SwNodeIndex aMvIdx( pDoc->GetNodes(), nMoveIndex );
            SwNodeRange aRg( aPos.nNode, 0 - nDiff, aPos.nNode, 1 - nDiff );
            aPos.nNode--;
            if( !bJoinNext )
                pMovedNode = &aPos.nNode.GetNode();
            pDoc->GetNodes()._MoveNodes( aRg, pDoc->GetNodes(), aMvIdx, sal_True );
            aPos.nNode++;
        }

        if( bNodeMove )
        {
            SwNodeRange aRange( *pMvStt, 0, *pMvStt, nNode );
            SwNodeIndex aCopyIndex( aPos.nNode, -1 );
            pDoc->GetUndoManager().GetUndoNodes()._Copy( aRange, aPos.nNode );

            if( nReplaceDummy )
            {
                sal_uLong nMoveIndex;
                if( bJoinNext )
                {
                    nMoveIndex = nEndNode - nNdDiff;
                    aPos.nNode = nMoveIndex + nReplaceDummy;
                }
                else
                {
                    aPos = SwPosition( aCopyIndex );
                    nMoveIndex = aPos.nNode.GetIndex() + nReplaceDummy + 1;
                }
                SwNodeIndex aMvIdx( pDoc->GetNodes(), nMoveIndex );
                SwNodeRange aRg( aPos.nNode, 0, aPos.nNode, 1 );
                pMovedNode = &aPos.nNode.GetNode();
                pDoc->GetNodes()._MoveNodes( aRg, pDoc->GetNodes(), aMvIdx, sal_True );
                pDoc->GetNodes().Delete( aMvIdx, 1 );
            }
        }

        if( pMovedNode )
            lcl_MakeAutoFrms( *pDoc->GetSpzFrmFmts(), pMovedNode->GetIndex() );

        if( pSttStr )
        {
            aPos.nNode = nSttNode - nNdDiff + ( bJoinNext ? 0 : nReplaceDummy );
            SwTxtNode * pTxtNd = aPos.nNode.GetNode().GetTxtNode();
            // If more than a single Node got deleted, also all "Node"
            // attributes were saved
            if (pTxtNd != NULL)
            {
                if( pTxtNd->HasSwAttrSet() && bNodeMove && !pEndStr )
                    pTxtNd->ResetAllAttr();

                if( pTxtNd->GetpSwpHints() )
                    pTxtNd->ClearSwpHintsArr( true );

                // SectionNode mode and selection from top to bottom:
                //  -> in StartNode is still the rest of the Join => delete
                aPos.nContent.Assign( pTxtNd, nSttCntnt );
                OUString const ins( pTxtNd->InsertText(*pSttStr, aPos.nContent,
                        IDocumentContentOperations::INS_NOHINTEXPAND) );
                assert(ins.getLength() == pSttStr->Len()); // must succeed
                // METADATA: restore
                pTxtNd->RestoreMetadata(m_pMetadataUndoStart);
            }
        }

        if( pHistory )
        {
            pHistory->TmpRollback( pDoc, nSetPos, false );
            if( nSetPos )       // there were Footnodes/FlyFrames
            {
                // are there others than these ones?
                if( nSetPos < pHistory->Count() )
                {
                    // if so save the attributes of the others
                    SwHistory aHstr;
                    aHstr.Move( 0, pHistory, nSetPos );
                    pHistory->Rollback( pDoc );
                    pHistory->Move( 0, &aHstr );
                }
                else
                {
                    pHistory->Rollback( pDoc );
                    DELETEZ( pHistory );
                }
            }
        }

        if( bResetPgDesc || bResetPgBrk )
        {
            sal_uInt16 nStt = static_cast<sal_uInt16>( bResetPgDesc ? RES_PAGEDESC : RES_BREAK );
            sal_uInt16 nEnd = static_cast<sal_uInt16>( bResetPgBrk ? RES_BREAK : RES_PAGEDESC );

            SwNode* pNode = pDoc->GetNodes()[ nEndNode + 1 ];
            if( pNode->IsCntntNode() )
                ((SwCntntNode*)pNode)->ResetAttr( nStt, nEnd );
            else if( pNode->IsTableNode() )
                ((SwTableNode*)pNode)->GetTable().GetFrmFmt()->ResetFmtAttr( nStt, nEnd );
        }
    }
    // delete the temporarily added Node
    if( pInsNd )
        pDoc->GetNodes().Delete( aIdx, 1 );
    if( pRedlSaveData )
        SetSaveData( *pDoc, *pRedlSaveData );

    AddUndoRedoPaM(rContext, true);
}

void SwUndoDelete::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam = AddUndoRedoPaM(rContext);
    SwDoc& rDoc = *rPam.GetDoc();

    if( pRedlSaveData )
    {
        bool bSuccess = FillSaveData(rPam, *pRedlSaveData, sal_True);
        OSL_ENSURE(bSuccess,
            "SwUndoDelete::Redo: used to have redline data, but now none?");
        if (!bSuccess)
        {
            delete pRedlSaveData, pRedlSaveData = 0;
        }
    }

    if( !bDelFullPara )
    {
        SwUndRng aTmpRng( rPam );
        RemoveIdxFromRange( rPam, sal_False );
        aTmpRng.SetPaM( rPam );

        if( !bJoinNext )           // then restore selection from bottom to top
            rPam.Exchange();
    }

    if( pHistory )      // are the attributes saved?
    {
        pHistory->SetTmpEnd( pHistory->Count() );
        SwHistory aHstr;
        aHstr.Move( 0, pHistory );

        if( bDelFullPara )
        {
            OSL_ENSURE( rPam.HasMark(), "PaM without Mark" );
            DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint(),
                            DelCntntType(nsDelCntntType::DELCNT_ALL | nsDelCntntType::DELCNT_CHKNOCNTNT) );

            _DelBookmarks(rPam.GetMark()->nNode, rPam.GetPoint()->nNode);
        }
        else
            DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint() );
        nSetPos = pHistory ? pHistory->Count() : 0;

        pHistory->Move( nSetPos, &aHstr );
    }
    else
    {
        if( bDelFullPara )
        {
            OSL_ENSURE( rPam.HasMark(), "PaM without Mark" );
            DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint(),
                            DelCntntType(nsDelCntntType::DELCNT_ALL | nsDelCntntType::DELCNT_CHKNOCNTNT) );

            _DelBookmarks( rPam.GetMark()->nNode, rPam.GetPoint()->nNode );
        }
        else
            DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint() );
        nSetPos = pHistory ? pHistory->Count() : 0;
    }

    if( !pSttStr && !pEndStr )
    {
        SwNodeIndex aSttIdx = ( bDelFullPara || bJoinNext )
                                    ? rPam.GetMark()->nNode
                                    : rPam.GetPoint()->nNode;
        SwTableNode* pTblNd = aSttIdx.GetNode().GetTableNode();
        if( pTblNd )
        {
            if( bTblDelLastNd )
            {
                // than add again a Node at the end
                const SwNodeIndex aTmpIdx( *pTblNd->EndOfSectionNode(), 1 );
                rDoc.GetNodes().MakeTxtNode( aTmpIdx,
                        rDoc.GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );
            }

            SwCntntNode* pNextNd = rDoc.GetNodes()[
                    pTblNd->EndOfSectionIndex()+1 ]->GetCntntNode();
            if( pNextNd )
            {
                SwFrmFmt* pTableFmt = pTblNd->GetTable().GetFrmFmt();

                const SfxPoolItem *pItem;
                if( SFX_ITEM_SET == pTableFmt->GetItemState( RES_PAGEDESC,
                    sal_False, &pItem ) )
                    pNextNd->SetAttr( *pItem );

                if( SFX_ITEM_SET == pTableFmt->GetItemState( RES_BREAK,
                    sal_False, &pItem ) )
                    pNextNd->SetAttr( *pItem );
            }
            pTblNd->DelFrms();
        }

        rPam.SetMark();
        rPam.DeleteMark();

        rDoc.GetNodes().Delete( aSttIdx, nEndNode - nSttNode );

        // always set the cursor into a ContentNode!
        if( !rPam.Move( fnMoveBackward, fnGoCntnt ) &&
            !rPam.Move( fnMoveForward, fnGoCntnt ) )
            rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), 0 );
    }
    else if( bDelFullPara )
    {
        // The Pam was incremented by one at Point (== end) to provide space
        // for UNDO. This now needs to be reverted!
        rPam.End()->nNode--;
        if( rPam.GetPoint()->nNode == rPam.GetMark()->nNode )
            *rPam.GetMark() = *rPam.GetPoint();
        rDoc.DelFullPara( rPam );
    }
    else
        rDoc.DeleteAndJoin( rPam );
}

void SwUndoDelete::RepeatImpl(::sw::RepeatContext & rContext)
{
    // this action does not seem idempotent,
    // so make sure it is only executed once on repeat
    if (rContext.m_bDeleteRepeated)
        return;

    SwPaM & rPam = rContext.GetRepeatPaM();
    SwDoc& rDoc = *rPam.GetDoc();
    ::sw::GroupUndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
    if( !rPam.HasMark() )
    {
        rPam.SetMark();
        rPam.Move( fnMoveForward, fnGoCntnt );
    }
    if( bDelFullPara )
        rDoc.DelFullPara( rPam );
    else
        rDoc.DeleteAndJoin( rPam );
    rContext.m_bDeleteRepeated = true;
}


void SwUndoDelete::SetTableName(const String & rName)
{
    sTableName = rName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
