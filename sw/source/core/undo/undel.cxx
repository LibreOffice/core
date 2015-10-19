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
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
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
#include <comcore.hrc>
#include <undo.hrc>
#include <vector>

// DELETE
/*  lcl_MakeAutoFrms has to call MakeFrms for objects bounded "AtChar"
    ( == AUTO ), if the anchor frame has be moved via _MoveNodes(..) and
    DelFrms(..)
*/
static void lcl_MakeAutoFrms( const SwFrameFormats& rSpzArr, sal_uLong nMovedIndex )
{
    if( !rSpzArr.empty() )
    {
        for( size_t n = 0; n < rSpzArr.size(); ++n )
        {
            SwFrameFormat * pFormat = rSpzArr[n];
            const SwFormatAnchor* pAnchor = &pFormat->GetAnchor();
            if (pAnchor->GetAnchorId() == FLY_AT_CHAR)
            {
                const SwPosition* pAPos = pAnchor->GetContentAnchor();
                if( pAPos && nMovedIndex == pAPos->nNode.GetIndex() )
                    pFormat->MakeFrms();
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
// 1. Deletion/recording of content indices of the selection: footnotes, fly
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
SwUndoDelete::SwUndoDelete(
    SwPaM& rPam,
    bool bFullPara,
    bool bCalledByTableCpy )
    : SwUndo(UNDO_DELETE),
    SwUndRng( rPam ),
    pMvStt( 0 ),
    pSttStr(0),
    pEndStr(0),
    pRedlData(0),
    pRedlSaveData(0),
    nNode(0),
    nNdDiff(0),
    nSectDiff(0),
    nReplaceDummy(0),
    nSetPos(0),
    bGroup( false ),
    bBackSp( false ),
    bJoinNext( false ),
    bTableDelLastNd( false ),
    // bFullPara is set e.g. if an empty paragraph before a table is deleted
    bDelFullPara( bFullPara ),
    bResetPgDesc( false ),
    bResetPgBrk( false ),
    bFromTableCopy( bCalledByTableCpy )
{

    bCacheComment = false;

    SwDoc * pDoc = rPam.GetDoc();

    if( !pDoc->getIDocumentRedlineAccess().IsIgnoreRedline() && !pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty() )
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

    // Step 1. deletion/record of content indices
    if( bDelFullPara )
    {
        OSL_ENSURE( rPam.HasMark(), "PaM ohne Mark" );
        DelContentIndex( *rPam.GetMark(), *rPam.GetPoint(),
                        DelContentType(nsDelContentType::DELCNT_ALL | nsDelContentType::DELCNT_CHKNOCNTNT) );

        ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
        _DelBookmarks(pStt->nNode, pEnd->nNode);
    }
    else
    {
        DelContentIndex( *rPam.GetMark(), *rPam.GetPoint() );
        ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
        if (nEndNode - nSttNode > 1) // check for fully selected nodes
        {
            SwNodeIndex const start(pStt->nNode, +1);
            _DelBookmarks(start, pEnd->nNode);
        }
    }

    nSetPos = pHistory ? pHistory->Count() : 0;

    // Is already anything deleted?
    nNdDiff = nSttNode - pStt->nNode.GetIndex();

    bJoinNext = !bFullPara && pEnd == rPam.GetPoint();
    bBackSp = !bFullPara && !bJoinNext;

    SwTextNode *pSttTextNd = 0, *pEndTextNd = 0;
    if( !bFullPara )
    {
        pSttTextNd = pStt->nNode.GetNode().GetTextNode();
        pEndTextNd = nSttNode == nEndNode
                    ? pSttTextNd
                    : pEnd->nNode.GetNode().GetTextNode();
    }

    bool bMoveNds = *pStt != *pEnd      // any area still existent?
                && ( SaveContent( pStt, pEnd, pSttTextNd, pEndTextNd ) || bFromTableCopy );

    if( pSttTextNd && pEndTextNd && pSttTextNd != pEndTextNd )
    {
        // two different TextNodes, thus save also the TextFormatCollection
        pHistory->Add( pSttTextNd->GetTextColl(),pStt->nNode.GetIndex(), ND_TEXTNODE );
        pHistory->Add( pEndTextNd->GetTextColl(),pEnd->nNode.GetIndex(), ND_TEXTNODE );

        if( !bJoinNext )        // Selection from bottom to top
        {
            // When using JoinPrev() all AUTO-PageBreak's will be copied
            // correctly. To restore them with UNDO, Auto-PageBreak of the
            // EndNode needs to be reset. Same for PageDesc and ColBreak.
            if( pEndTextNd->HasSwAttrSet() )
            {
                SwRegHistory aRegHist( *pEndTextNd, pHistory );
                if( SfxItemState::SET == pEndTextNd->GetpSwAttrSet()->GetItemState(
                        RES_BREAK, false ) )
                    pEndTextNd->ResetAttr( RES_BREAK );
                if( pEndTextNd->HasSwAttrSet() &&
                    SfxItemState::SET == pEndTextNd->GetpSwAttrSet()->GetItemState(
                        RES_PAGEDESC, false ) )
                    pEndTextNd->ResetAttr( RES_PAGEDESC );
            }
        }
    }

    // Move now also the PaM. The SPoint is at the beginning of a SSelection.
    if( pEnd == rPam.GetPoint() && ( !bFullPara || pSttTextNd || pEndTextNd ) )
        rPam.Exchange();

    if( !pSttTextNd && !pEndTextNd )
        --rPam.GetPoint()->nNode;
    rPam.DeleteMark();          // the SPoint is in the selection

    if( !pEndTextNd )
        nEndContent = 0;
    if( !pSttTextNd )
        nSttContent = 0;

    if( bMoveNds )      // Do Nodes exist that need to be moved?
    {
        SwNodes& rNds = pDoc->GetUndoManager().GetUndoNodes();
        SwNodes& rDocNds = pDoc->GetNodes();
        SwNodeRange aRg( rDocNds, nSttNode - nNdDiff,
                         rDocNds, nEndNode - nNdDiff );
        if( !bFullPara && !pEndTextNd &&
            &aRg.aEnd.GetNode() != &pDoc->GetNodes().GetEndOfContent() )
        {
            SwNode* pNode = aRg.aEnd.GetNode().StartOfSectionNode();
            if( pNode->GetIndex() >= nSttNode - nNdDiff )
                ++aRg.aEnd; // Deletion of a complete table
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
                ++aRg.aEnd;
            nReplaceDummy = aRg.aEnd.GetIndex() + nNdDiff - nEndNode;
            if( nReplaceDummy )
            {   // The selection has been expanded, because
                ++aRg.aEnd;
                if( pEndTextNd )
                {
                    // The end text node has to leave the (expanded) selection
                    // The dummy is needed because _MoveNodes deletes empty
                    // sections
                    ++nReplaceDummy;
                    SwNodeRange aMvRg( *pEndTextNd, 0, *pEndTextNd, 1 );
                    SwPosition aSplitPos( *pEndTextNd );
                    ::sw::UndoGuard const ug(pDoc->GetIDocumentUndoRedo());
                    pDoc->getIDocumentContentOperations().SplitNode( aSplitPos, false );
                    rDocNds._MoveNodes( aMvRg, rDocNds, aRg.aEnd );
                    --aRg.aEnd;
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
                --aRg.aStart;
            if( pSttTextNd )
            {
                nReplaceDummy = nSttNode - nNdDiff - aRg.aStart.GetIndex();
                if( nReplaceDummy )
                {
                    SwNodeRange aMvRg( *pSttTextNd, 0, *pSttTextNd, 1 );
                    SwPosition aSplitPos( *pSttTextNd );
                    ::sw::UndoGuard const ug(pDoc->GetIDocumentUndoRedo());
                    pDoc->getIDocumentContentOperations().SplitNode( aSplitPos, false );
                    rDocNds._MoveNodes( aMvRg, rDocNds, aRg.aStart );
                    --aRg.aStart;
                }
            }
        }

        if( bFromTableCopy )
        {
            if( !pEndTextNd )
            {
                if( pSttTextNd )
                    ++aRg.aStart;
                else if( !bFullPara && !aRg.aEnd.GetNode().IsContentNode() )
                    --aRg.aEnd;
            }
        }
        else if (pSttTextNd && (pEndTextNd || pSttTextNd->GetText().getLength()))
            ++aRg.aStart;

        // Step 3: Moving into UndoArray...
        nNode = rNds.GetEndOfContent().GetIndex();
        rDocNds._MoveNodes( aRg, rNds, SwNodeIndex( rNds.GetEndOfContent() ));
        pMvStt = new SwNodeIndex( rNds, nNode );
        // remember difference!
        nNode = rNds.GetEndOfContent().GetIndex() - nNode;

        if( pSttTextNd && pEndTextNd )
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
                    SwNodeRange aMvRg( *pEndTextNd, 0, *pEndTextNd, 1 );
                    rDocNds._MoveNodes( aMvRg, rDocNds, aRg.aStart );
                }
                else
                {
                    SwNodeRange aMvRg( *pSttTextNd, 0, *pSttTextNd, 1 );
                    rDocNds._MoveNodes( aMvRg, rDocNds, aRg.aEnd );
                }
            }
        }
        if( nSectDiff || nReplaceDummy )
            lcl_MakeAutoFrms( *pDoc->GetSpzFrameFormats(),
                bJoinNext ? pEndTextNd->GetIndex() : pSttTextNd->GetIndex() );
    }
    else
        nNode = 0;      // moved no node -> no difference at the end

    // Are there any Nodes that got deleted before that (FootNotes
    // have ContentNodes)?
    if( !pSttTextNd && !pEndTextNd )
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

    if( !rPam.GetNode().IsContentNode() )
        rPam.GetPoint()->nContent.Assign( 0, 0 );

    // is a history necessary here at all?
    if( pHistory && !pHistory->Count() )
        DELETEZ( pHistory );
}

bool SwUndoDelete::SaveContent( const SwPosition* pStt, const SwPosition* pEnd,
                    SwTextNode* pSttTextNd, SwTextNode* pEndTextNd )
{
    sal_uLong nNdIdx = pStt->nNode.GetIndex();
    // 1 - copy start in Start-String
    if( pSttTextNd )
    {
        bool bOneNode = nSttNode == nEndNode;
        SwRegHistory aRHst( *pSttTextNd, pHistory );
        // always save all text atttibutes because of possibly overlapping
        // areas of on/off
        pHistory->CopyAttr( pSttTextNd->GetpSwpHints(), nNdIdx,
                            0, pSttTextNd->GetText().getLength(), true );
        if( !bOneNode && pSttTextNd->HasSwAttrSet() )
                pHistory->CopyFormatAttr( *pSttTextNd->GetpSwAttrSet(), nNdIdx );

        // the length might have changed (!!Fields!!)
        sal_Int32 nLen = ((bOneNode)
                    ? pEnd->nContent.GetIndex()
                    : pSttTextNd->GetText().getLength())
            - pStt->nContent.GetIndex();

        // delete now also the text (all attribute changes are added to
        // UNDO history)
        pSttStr = new OUString( pSttTextNd->GetText().copy(nSttContent, nLen));
        pSttTextNd->EraseText( pStt->nContent, nLen );
        if( pSttTextNd->GetpSwpHints() )
            pSttTextNd->GetpSwpHints()->DeRegister();

        // METADATA: store
        bool emptied( !pSttStr->isEmpty() && !pSttTextNd->Len() );
        if (!bOneNode || emptied) // merging may overwrite xmlids...
        {
            m_pMetadataUndoStart = (emptied)
                ? pSttTextNd->CreateUndoForDelete()
                : pSttTextNd->CreateUndo();
        }

        if( bOneNode )
            return false;           // stop moving more nodes
    }

    // 2 - copy end into End-String
    if( pEndTextNd )
    {
        SwIndex aEndIdx( pEndTextNd );
        nNdIdx = pEnd->nNode.GetIndex();
        SwRegHistory aRHst( *pEndTextNd, pHistory );

        // always save all text atttibutes because of possibly overlapping
        // areas of on/off
        pHistory->CopyAttr( pEndTextNd->GetpSwpHints(), nNdIdx, 0,
                            pEndTextNd->GetText().getLength(), true );

        if( pEndTextNd->HasSwAttrSet() )
            pHistory->CopyFormatAttr( *pEndTextNd->GetpSwAttrSet(), nNdIdx );

        // delete now also the text (all attribute changes are added to
        // UNDO history)
        pEndStr = new OUString( pEndTextNd->GetText().copy( 0,
                                    pEnd->nContent.GetIndex() ));
        pEndTextNd->EraseText( aEndIdx, pEnd->nContent.GetIndex() );
        if( pEndTextNd->GetpSwpHints() )
            pEndTextNd->GetpSwpHints()->DeRegister();

        // METADATA: store
        bool emptied = !pEndStr->isEmpty() && !pEndTextNd->Len();

        m_pMetadataUndoEnd = (emptied)
            ? pEndTextNd->CreateUndoForDelete()
            : pEndTextNd->CreateUndo();
    }

    // if there are only two Nodes than we're done
    if( ( pSttTextNd || pEndTextNd ) && nSttNode + 1 == nEndNode )
        return false;           // do not move any Node

    return true;                // move Nodes lying in between
}

bool SwUndoDelete::CanGrouping( SwDoc* pDoc, const SwPaM& rDelPam )
{
    // Is Undo greater than one Node (that is Start and EndString)?
    if( !pSttStr || pSttStr->isEmpty() || pEndStr )
        return false;

    // only the deletion of single char's can be condensed
    if( nSttNode != nEndNode || ( !bGroup && nSttContent+1 != nEndContent ))
        return false;

    const SwPosition *pStt = rDelPam.Start(),
                    *pEnd = rDelPam.GetPoint() == pStt
                        ? rDelPam.GetMark()
                        : rDelPam.GetPoint();

    if( pStt->nNode != pEnd->nNode ||
        pStt->nContent.GetIndex()+1 != pEnd->nContent.GetIndex() ||
        pEnd->nNode != nSttNode )
        return false;

    // Distinguish between BackSpace and Delete because the Undo array needs to
    // be constructed differently!
    if( pEnd->nContent == nSttContent )
    {
        if( bGroup && !bBackSp ) return false;
        bBackSp = true;
    }
    else if( pStt->nContent == nSttContent )
    {
        if( bGroup && bBackSp ) return false;
        bBackSp = false;
    }
    else
        return false;

    // are both Nodes (Node/Undo array) TextNodes at all?
    SwTextNode * pDelTextNd = pStt->nNode.GetNode().GetTextNode();
    if( !pDelTextNd ) return false;

    sal_Int32 nUChrPos = bBackSp ? 0 : pSttStr->getLength()-1;
    sal_Unicode cDelChar = pDelTextNd->GetText()[ pStt->nContent.GetIndex() ];
    CharClass& rCC = GetAppCharClass();
    if( ( CH_TXTATR_BREAKWORD == cDelChar || CH_TXTATR_INWORD == cDelChar ) ||
        rCC.isLetterNumeric( OUString( cDelChar ), 0 ) !=
        rCC.isLetterNumeric( *pSttStr, nUChrPos ) )
        return false;

    {
        SwRedlineSaveDatas aTmpSav;
        const bool bSaved = FillSaveData( rDelPam, aTmpSav, false );

        bool bOk = ( !pRedlSaveData && !bSaved ) ||
                   ( pRedlSaveData && bSaved &&
                SwUndo::CanRedlineGroup( *pRedlSaveData, aTmpSav, bBackSp ));
        // aTmpSav.DeleteAndDestroyAll();
        if( !bOk )
            return false;

        pDoc->getIDocumentRedlineAccess().DeleteRedline( rDelPam, false, USHRT_MAX );
    }

    // Both 'deletes' can be consolidated, so 'move' the related character
    if( bBackSp )
        nSttContent--;    // BackSpace: add char to array!
    else
    {
        nEndContent++;    // Delete: attach char at the end
        nUChrPos++;
    }
    (*pSttStr) = pSttStr->replaceAt( nUChrPos, 0, OUString(cDelChar) );
    pDelTextNd->EraseText( pStt->nContent, 1 );

    bGroup = true;
    return true;
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
        OUString aDescr = rHistory[n]->GetDescription();

        if (!aDescr.isEmpty())
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

static OUString lcl_DenotedPortion(const OUString& rStr, sal_Int32 nStart, sal_Int32 nEnd)
{
    OUString aResult;

    if (nEnd - nStart > 0)
    {
        sal_Unicode cLast = rStr[nEnd - 1];
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
            aResult += rStr.copy(nStart, nEnd - nStart);
            aResult += SW_RESSTR(STR_END_QUOTE);
        }
    }

    return aResult;
}

OUString DenoteSpecialCharacters(const OUString & rStr)
{
    OUString aResult;

    if (!rStr.isEmpty())
    {
        bool bStart = false;
        sal_Int32 nStart = 0;
        sal_Unicode cLast = 0;

        for( sal_Int32 i = 0; i < rStr.getLength(); i++)
        {
            if (lcl_IsSpecialCharacter(rStr[i]))
            {
                if (cLast != rStr[i])
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

            cLast = rStr[i];
        }

        aResult += lcl_DenotedPortion(rStr, nStart, rStr.getLength());
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
        if (!sTableName.isEmpty())
        {

            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, SW_RESSTR(STR_START_QUOTE));
            aRewriter.AddRule(UndoArg2, sTableName);
            aRewriter.AddRule(UndoArg3, SW_RESSTR(STR_END_QUOTE));

            OUString sTmp = aRewriter.Apply(SW_RES(STR_TABLE_NAME));
            aResult.AddRule(UndoArg1, sTmp);
        }
        else
            aResult.AddRule(UndoArg1, SW_RESSTR(STR_PARAGRAPHS));
    }
    else
    {
        OUString aStr;

        if (pSttStr != NULL && pEndStr != NULL && pSttStr->isEmpty() &&
            pEndStr->isEmpty())
        {
            aStr = SW_RESSTR(STR_PARAGRAPH_UNDO);
        }
        else
        {
            OUString * pStr = NULL;
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

        aStr = ShortenString(aStr, nUndoStringLength, OUString(SW_RES(STR_LDOTS)));
        if (pHistory)
        {
            SwRewriter aRewriter = lcl_RewriterFromHistory(*pHistory);
            aStr = aRewriter.Apply(aStr);
        }

        aResult.AddRule(UndoArg1, aStr);
    }

    return aResult;
}

// Every object, anchored "AtContent" will be reanchored at rPos
static void lcl_ReAnchorAtContentFlyFrames( const SwFrameFormats& rSpzArr, SwPosition &rPos, sal_uLong nOldIdx )
{
    if( !rSpzArr.empty() )
    {
        SwFlyFrameFormat* pFormat;
        const SwFormatAnchor* pAnchor;
        const SwPosition* pAPos;
        for( size_t n = 0; n < rSpzArr.size(); ++n )
        {
            pFormat = static_cast<SwFlyFrameFormat*>(rSpzArr[n]);
            pAnchor = &pFormat->GetAnchor();
            if (pAnchor->GetAnchorId() == FLY_AT_PARA)
            {
                pAPos =  pAnchor->GetContentAnchor();
                if( pAPos && nOldIdx == pAPos->nNode.GetIndex() )
                {
                    SwFormatAnchor aAnch( *pAnchor );
                    aAnch.SetAnchor( &rPos );
                    pFormat->SetFormatAttr( aAnch );
                }
            }
        }
    }
}

void SwUndoDelete::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc& rDoc = rContext.GetDoc();

    sal_uLong nCalcStt = nSttNode - nNdDiff;

    if( nSectDiff && bBackSp )
        nCalcStt += nSectDiff;

    SwNodeIndex aIdx(rDoc.GetNodes(), nCalcStt);
    SwNode* pInsNd = &aIdx.GetNode();

    {   // code block so that SwPosition is detached when deleting a Node
        SwPosition aPos( aIdx );
        if( !bDelFullPara )
        {
            if( pInsNd->IsTableNode() )
            {
                pInsNd = rDoc.GetNodes().MakeTextNode( aIdx,
                        rDoc.GetDfltTextFormatColl() );
                --aIdx;
                aPos.nNode = aIdx;
                aPos.nContent.Assign( pInsNd->GetContentNode(), nSttContent );
            }
            else
            {
                if( pInsNd->IsContentNode() )
                    aPos.nContent.Assign( static_cast<SwContentNode*>(pInsNd), nSttContent );
                if( !bTableDelLastNd )
                    pInsNd = 0;         // do not delete Node!
            }
        }
        else
            pInsNd = 0;         // do not delete Node!

        bool bNodeMove = 0 != nNode;

        if( pEndStr )
        {
            // discard attributes since they all saved!
            SwTextNode* pTextNd = aPos.nNode.GetNode().GetTextNode();

            if( pTextNd && pTextNd->HasSwAttrSet() )
                pTextNd->ResetAllAttr();

            if( pTextNd && pTextNd->GetpSwpHints() )
                pTextNd->ClearSwpHintsArr( true );

            if( pSttStr && !bFromTableCopy )
            {
                sal_uLong nOldIdx = aPos.nNode.GetIndex();
                rDoc.getIDocumentContentOperations().SplitNode( aPos, false );
                // After the split all objects are anchored at the first
                // paragraph, but the pHistory of the fly frame formats relies
                // on anchoring at the start of the selection
                // => selection backwards needs a correction.
                if( bBackSp )
                    lcl_ReAnchorAtContentFlyFrames(*rDoc.GetSpzFrameFormats(), aPos, nOldIdx);
                pTextNd = aPos.nNode.GetNode().GetTextNode();
            }
            if( pTextNd )
            {
                OUString const ins( pTextNd->InsertText(*pEndStr, aPos.nContent,
                        SwInsertFlags::NOHINTEXPAND) );
                assert(ins.getLength() == pEndStr->getLength()); // must succeed
                (void) ins;
                // METADATA: restore
                pTextNd->RestoreMetadata(m_pMetadataUndoEnd);
            }
        }
        else if( pSttStr && bNodeMove )
        {
            SwTextNode * pNd = aPos.nNode.GetNode().GetTextNode();
            if( pNd )
            {
                if (nSttContent < pNd->GetText().getLength())
                {
                    sal_uLong nOldIdx = aPos.nNode.GetIndex();
                    rDoc.getIDocumentContentOperations().SplitNode( aPos, false );
                    if( bBackSp )
                        lcl_ReAnchorAtContentFlyFrames(*rDoc.GetSpzFrameFormats(), aPos, nOldIdx);
                }
                else
                    ++aPos.nNode;
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
            SwNodeIndex aMvIdx(rDoc.GetNodes(), nMoveIndex);
            SwNodeRange aRg( aPos.nNode, 0 - nDiff, aPos.nNode, 1 - nDiff );
            --aPos.nNode;
            if( !bJoinNext )
                pMovedNode = &aPos.nNode.GetNode();
            rDoc.GetNodes()._MoveNodes(aRg, rDoc.GetNodes(), aMvIdx);
            ++aPos.nNode;
        }

        if( bNodeMove )
        {
            SwNodeRange aRange( *pMvStt, 0, *pMvStt, nNode );
            SwNodeIndex aCopyIndex( aPos.nNode, -1 );
            rDoc.GetUndoManager().GetUndoNodes()._Copy( aRange, aPos.nNode );

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
                SwNodeIndex aMvIdx(rDoc.GetNodes(), nMoveIndex);
                SwNodeRange aRg( aPos.nNode, 0, aPos.nNode, 1 );
                pMovedNode = &aPos.nNode.GetNode();
                rDoc.GetNodes()._MoveNodes(aRg, rDoc.GetNodes(), aMvIdx);
                rDoc.GetNodes().Delete( aMvIdx);
            }
        }

        if( pMovedNode )
            lcl_MakeAutoFrms(*rDoc.GetSpzFrameFormats(), pMovedNode->GetIndex());

        if( pSttStr )
        {
            aPos.nNode = nSttNode - nNdDiff + ( bJoinNext ? 0 : nReplaceDummy );
            SwTextNode * pTextNd = aPos.nNode.GetNode().GetTextNode();
            // If more than a single Node got deleted, also all "Node"
            // attributes were saved
            if (pTextNd != NULL)
            {
                if( pTextNd->HasSwAttrSet() && bNodeMove && !pEndStr )
                    pTextNd->ResetAllAttr();

                if( pTextNd->GetpSwpHints() )
                    pTextNd->ClearSwpHintsArr( true );

                // SectionNode mode and selection from top to bottom:
                //  -> in StartNode is still the rest of the Join => delete
                aPos.nContent.Assign( pTextNd, nSttContent );
                OUString const ins( pTextNd->InsertText(*pSttStr, aPos.nContent,
                        SwInsertFlags::NOHINTEXPAND) );
                assert(ins.getLength() == pSttStr->getLength()); // must succeed
                (void) ins;
                // METADATA: restore
                pTextNd->RestoreMetadata(m_pMetadataUndoStart);
            }
        }

        if( pHistory )
        {
            pHistory->TmpRollback(&rDoc, nSetPos, false);
            if( nSetPos )       // there were Footnodes/FlyFrames
            {
                // are there others than these ones?
                if( nSetPos < pHistory->Count() )
                {
                    // if so save the attributes of the others
                    SwHistory aHstr;
                    aHstr.Move( 0, pHistory, nSetPos );
                    pHistory->Rollback(&rDoc);
                    pHistory->Move( 0, &aHstr );
                }
                else
                {
                    pHistory->Rollback(&rDoc);
                    DELETEZ( pHistory );
                }
            }
        }

        if( bResetPgDesc || bResetPgBrk )
        {
            sal_uInt16 nStt = static_cast<sal_uInt16>( bResetPgDesc ? RES_PAGEDESC : RES_BREAK );
            sal_uInt16 nEnd = static_cast<sal_uInt16>( bResetPgBrk ? RES_BREAK : RES_PAGEDESC );

            SwNode* pNode = rDoc.GetNodes()[ nEndNode + 1 ];
            if( pNode->IsContentNode() )
                static_cast<SwContentNode*>(pNode)->ResetAttr( nStt, nEnd );
            else if( pNode->IsTableNode() )
                static_cast<SwTableNode*>(pNode)->GetTable().GetFrameFormat()->ResetFormatAttr( nStt, nEnd );
        }
    }
    // delete the temporarily added Node
    if( pInsNd )
        rDoc.GetNodes().Delete( aIdx );
    if( pRedlSaveData )
        SetSaveData(rDoc, *pRedlSaveData);

    AddUndoRedoPaM(rContext, true);
}

void SwUndoDelete::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam = AddUndoRedoPaM(rContext);
    SwDoc& rDoc = *rPam.GetDoc();

    if( pRedlSaveData )
    {
        const bool bSuccess = FillSaveData(rPam, *pRedlSaveData);
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
        RemoveIdxFromRange( rPam, false );
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
            DelContentIndex( *rPam.GetMark(), *rPam.GetPoint(),
                            DelContentType(nsDelContentType::DELCNT_ALL | nsDelContentType::DELCNT_CHKNOCNTNT) );

            _DelBookmarks(rPam.GetMark()->nNode, rPam.GetPoint()->nNode);
        }
        else
            DelContentIndex( *rPam.GetMark(), *rPam.GetPoint() );
        nSetPos = pHistory ? pHistory->Count() : 0;

        pHistory->Move( nSetPos, &aHstr );
    }
    else
    {
        if( bDelFullPara )
        {
            OSL_ENSURE( rPam.HasMark(), "PaM without Mark" );
            DelContentIndex( *rPam.GetMark(), *rPam.GetPoint(),
                            DelContentType(nsDelContentType::DELCNT_ALL | nsDelContentType::DELCNT_CHKNOCNTNT) );

            _DelBookmarks( rPam.GetMark()->nNode, rPam.GetPoint()->nNode );
        }
        else
            DelContentIndex( *rPam.GetMark(), *rPam.GetPoint() );
        nSetPos = pHistory ? pHistory->Count() : 0;
    }

    if( !pSttStr && !pEndStr )
    {
        SwNodeIndex aSttIdx = ( bDelFullPara || bJoinNext )
                                    ? rPam.GetMark()->nNode
                                    : rPam.GetPoint()->nNode;
        SwTableNode* pTableNd = aSttIdx.GetNode().GetTableNode();
        if( pTableNd )
        {
            if( bTableDelLastNd )
            {
                // than add again a Node at the end
                const SwNodeIndex aTmpIdx( *pTableNd->EndOfSectionNode(), 1 );
                rDoc.GetNodes().MakeTextNode( aTmpIdx,
                        rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD ) );
            }

            SwContentNode* pNextNd = rDoc.GetNodes()[
                    pTableNd->EndOfSectionIndex()+1 ]->GetContentNode();
            if( pNextNd )
            {
                SwFrameFormat* pTableFormat = pTableNd->GetTable().GetFrameFormat();

                const SfxPoolItem *pItem;
                if( SfxItemState::SET == pTableFormat->GetItemState( RES_PAGEDESC,
                    false, &pItem ) )
                    pNextNd->SetAttr( *pItem );

                if( SfxItemState::SET == pTableFormat->GetItemState( RES_BREAK,
                    false, &pItem ) )
                    pNextNd->SetAttr( *pItem );
            }
            pTableNd->DelFrms();
        }

        // avoid asserts from ~SwIndexReg for deleted nodes
        SwPaM aTmp(*rPam.End());
        if (!aTmp.Move(fnMoveForward, fnGoNode))
        {
            *aTmp.GetPoint() = *rPam.Start();
            aTmp.Move(fnMoveBackward, fnGoNode);
        }
        assert(aTmp.GetPoint()->nNode != rPam.GetPoint()->nNode
            && aTmp.GetPoint()->nNode != rPam.GetMark()->nNode);
        ::PaMCorrAbs(rPam, *aTmp.GetPoint());

        rPam.DeleteMark();

        rDoc.GetNodes().Delete( aSttIdx, nEndNode - nSttNode );
    }
    else if( bDelFullPara )
    {
        // The Pam was incremented by one at Point (== end) to provide space
        // for UNDO. This now needs to be reverted!
        --rPam.End()->nNode;
        if( rPam.GetPoint()->nNode == rPam.GetMark()->nNode )
            *rPam.GetMark() = *rPam.GetPoint();
        rDoc.getIDocumentContentOperations().DelFullPara( rPam );
    }
    else
        rDoc.getIDocumentContentOperations().DeleteAndJoin( rPam );
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
        rPam.Move( fnMoveForward, fnGoContent );
    }
    if( bDelFullPara )
        rDoc.getIDocumentContentOperations().DelFullPara( rPam );
    else
        rDoc.getIDocumentContentOperations().DeleteAndJoin( rPam );
    rContext.m_bDeleteRepeated = true;
}

void SwUndoDelete::SetTableName(const OUString & rName)
{
    sTableName = rName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
