/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <comcore.hrc>
#include <undo.hrc>
#include <vector>


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


























SwUndoDelete::SwUndoDelete(
    SwPaM& rPam,
    sal_Bool bFullPara,
    sal_Bool bCalledByTblCpy )
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
    bGroup( sal_False ),
    bBackSp( sal_False ),
    bJoinNext( sal_False ),
    bTblDelLastNd( sal_False ),
    bDelFullPara( bFullPara ),
    bResetPgDesc( sal_False ),
    bResetPgBrk( sal_False ),
    bFromTableCopy( bCalledByTblCpy )
{
    
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

    
    const SwPosition *pStt = rPam.Start(),
                    *pEnd = rPam.GetPoint() == pStt
                        ? rPam.GetMark()
                        : rPam.GetPoint();

    
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

    bool bMoveNds = *pStt == *pEnd      
                ? false
                : ( SaveCntnt( pStt, pEnd, pSttTxtNd, pEndTxtNd ) || bFromTableCopy );

    if( pSttTxtNd && pEndTxtNd && pSttTxtNd != pEndTxtNd )
    {
        
        pHistory->Add( pSttTxtNd->GetTxtColl(),pStt->nNode.GetIndex(), ND_TEXTNODE );
        pHistory->Add( pEndTxtNd->GetTxtColl(),pEnd->nNode.GetIndex(), ND_TEXTNODE );

        if( !bJoinNext )        
        {
            
            
            
            if( pEndTxtNd->HasSwAttrSet() )
            {
                SwRegHistory aRegHist( *pEndTxtNd, pHistory );
                if( SFX_ITEM_SET == pEndTxtNd->GetpSwAttrSet()->GetItemState(
                        RES_BREAK, false ) )
                    pEndTxtNd->ResetAttr( RES_BREAK );
                if( pEndTxtNd->HasSwAttrSet() &&
                    SFX_ITEM_SET == pEndTxtNd->GetpSwAttrSet()->GetItemState(
                        RES_PAGEDESC, false ) )
                    pEndTxtNd->ResetAttr( RES_PAGEDESC );
            }
        }
    }

    
    if( pEnd == rPam.GetPoint() && ( !bFullPara || pSttTxtNd || pEndTxtNd ) )
        rPam.Exchange();

    if( !pSttTxtNd && !pEndTxtNd )
        rPam.GetPoint()->nNode--;
    rPam.DeleteMark();          

    if( !pEndTxtNd )
        nEndCntnt = 0;
    if( !pSttTxtNd )
        nSttCntnt = 0;

    if( bMoveNds )      
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
                aRg.aEnd++; 
        }
        SwNode* pTmpNd;
        
        if( bJoinNext || bFullPara )
        {
            
            
            while( aRg.aEnd.GetIndex() + 2  < rDocNds.Count() &&
                ( (pTmpNd = rDocNds[ aRg.aEnd.GetIndex()+1 ])->IsEndNode() &&
                pTmpNd->StartOfSectionNode()->IsSectionNode() &&
                pTmpNd->StartOfSectionNode()->GetIndex() >= aRg.aStart.GetIndex() ) )
                aRg.aEnd++;
            nReplaceDummy = aRg.aEnd.GetIndex() + nNdDiff - nEndNode;
            if( nReplaceDummy )
            {   
                aRg.aEnd++;
                if( pEndTxtNd )
                {
                    
                    
                    
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

        
        nNode = rNds.GetEndOfContent().GetIndex();
        rDocNds._MoveNodes( aRg, rNds, SwNodeIndex( rNds.GetEndOfContent() ));
        pMvStt = new SwNodeIndex( rNds, nNode );
        
        nNode = rNds.GetEndOfContent().GetIndex() - nNode;

        if( pSttTxtNd && pEndTxtNd )
        {
            
            nSectDiff = aRg.aEnd.GetIndex() - aRg.aStart.GetIndex();
            
            
            
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
        nNode = 0;      

    
    
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

    
    if( pHistory && !pHistory->Count() )
        DELETEZ( pHistory );
}

sal_Bool SwUndoDelete::SaveCntnt( const SwPosition* pStt, const SwPosition* pEnd,
                    SwTxtNode* pSttTxtNd, SwTxtNode* pEndTxtNd )
{
    sal_uLong nNdIdx = pStt->nNode.GetIndex();
    
    if( pSttTxtNd )
    {
        bool bOneNode = nSttNode == nEndNode;
        sal_Int32 nLen = bOneNode ? nEndCntnt - nSttCntnt
                                : pSttTxtNd->GetTxt().getLength() - nSttCntnt;
        SwRegHistory aRHst( *pSttTxtNd, pHistory );
        
        
        pHistory->CopyAttr( pSttTxtNd->GetpSwpHints(), nNdIdx,
                            0, pSttTxtNd->GetTxt().getLength(), true );
        if( !bOneNode && pSttTxtNd->HasSwAttrSet() )
                pHistory->CopyFmtAttr( *pSttTxtNd->GetpSwAttrSet(), nNdIdx );

        
        nLen = ((bOneNode)
                    ? pEnd->nContent.GetIndex()
                    : pSttTxtNd->GetTxt().getLength())
            - pStt->nContent.GetIndex();

        
        
        pSttStr = new OUString( pSttTxtNd->GetTxt().copy(nSttCntnt, nLen));
        pSttTxtNd->EraseText( pStt->nContent, nLen );
        if( pSttTxtNd->GetpSwpHints() )
            pSttTxtNd->GetpSwpHints()->DeRegister();

        
        bool emptied( !pSttStr->isEmpty() && !pSttTxtNd->Len() );
        if (!bOneNode || emptied) 
        {
            m_pMetadataUndoStart = (emptied)
                ? pSttTxtNd->CreateUndoForDelete()
                : pSttTxtNd->CreateUndo();
        }

        if( bOneNode )
            return sal_False;           
    }

    
    if( pEndTxtNd )
    {
        SwIndex aEndIdx( pEndTxtNd );
        nNdIdx = pEnd->nNode.GetIndex();
        SwRegHistory aRHst( *pEndTxtNd, pHistory );

        
        
        pHistory->CopyAttr( pEndTxtNd->GetpSwpHints(), nNdIdx, 0,
                            pEndTxtNd->GetTxt().getLength(), true );

        if( pEndTxtNd->HasSwAttrSet() )
            pHistory->CopyFmtAttr( *pEndTxtNd->GetpSwAttrSet(), nNdIdx );

        
        
        pEndStr = new OUString( pEndTxtNd->GetTxt().copy( 0,
                                    pEnd->nContent.GetIndex() ));
        pEndTxtNd->EraseText( aEndIdx, pEnd->nContent.GetIndex() );
        if( pEndTxtNd->GetpSwpHints() )
            pEndTxtNd->GetpSwpHints()->DeRegister();

        
        bool emptied = !pEndStr->isEmpty() && !pEndTxtNd->Len();

        m_pMetadataUndoEnd = (emptied)
            ? pEndTxtNd->CreateUndoForDelete()
            : pEndTxtNd->CreateUndo();
    }

    
    if( ( pSttTxtNd || pEndTxtNd ) && nSttNode + 1 == nEndNode )
        return sal_False;           

    return sal_True;                
}

sal_Bool SwUndoDelete::CanGrouping( SwDoc* pDoc, const SwPaM& rDelPam )
{
    
    if( pSttStr ? pSttStr->isEmpty() || pEndStr : sal_True )
        return sal_False;

    
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

    
    SwTxtNode * pDelTxtNd = pStt->nNode.GetNode().GetTxtNode();
    if( !pDelTxtNd ) return sal_False;

    sal_Int32 nUChrPos = bBackSp ? 0 : pSttStr->getLength()-1;
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

    
    if( bBackSp )
        nSttCntnt--;    
    else
    {
        nEndCntnt++;    
        nUChrPos++;
    }
    (*pSttStr) = pSttStr->replaceAt( nUChrPos, 0, OUString(cDelChar) );
    pDelTxtNd->EraseText( pStt->nContent, 1 );

    bGroup = sal_True;
    return sal_True;
}

SwUndoDelete::~SwUndoDelete()
{
    delete pSttStr;
    delete pEndStr;
    if( pMvStt )        
    {
        
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

static OUString lcl_DenotedPortion(OUString rStr, sal_Int32 nStart, sal_Int32 nEnd)
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

    {   
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
                    pInsNd = 0;         
            }
        }
        else
            pInsNd = 0;         

        bool bNodeMove = 0 != nNode;

        if( pEndStr )
        {
            
            SwTxtNode* pTxtNd = aPos.nNode.GetNode().GetTxtNode();

            if( pTxtNd && pTxtNd->HasSwAttrSet() )
                pTxtNd->ResetAllAttr();

            if( pTxtNd && pTxtNd->GetpSwpHints() )
                pTxtNd->ClearSwpHintsArr( true );

            if( pSttStr && !bFromTableCopy )
            {
                sal_uLong nOldIdx = aPos.nNode.GetIndex();
                pDoc->SplitNode( aPos, false );
                
                
                
                
                if( bBackSp )
                    lcl_ReAnchorAtCntntFlyFrames( *pDoc->GetSpzFrmFmts(), aPos, nOldIdx );
                pTxtNd = aPos.nNode.GetNode().GetTxtNode();
            }
            if( pTxtNd )
            {
                OUString const ins( pTxtNd->InsertText(*pEndStr, aPos.nContent,
                        IDocumentContentOperations::INS_NOHINTEXPAND) );
                assert(ins.getLength() == pEndStr->getLength()); 
                (void) ins;
                
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
            
            
            if (pTxtNd != NULL)
            {
                if( pTxtNd->HasSwAttrSet() && bNodeMove && !pEndStr )
                    pTxtNd->ResetAllAttr();

                if( pTxtNd->GetpSwpHints() )
                    pTxtNd->ClearSwpHintsArr( true );

                
                
                aPos.nContent.Assign( pTxtNd, nSttCntnt );
                OUString const ins( pTxtNd->InsertText(*pSttStr, aPos.nContent,
                        IDocumentContentOperations::INS_NOHINTEXPAND) );
                assert(ins.getLength() == pSttStr->getLength()); 
                (void) ins;
                
                pTxtNd->RestoreMetadata(m_pMetadataUndoStart);
            }
        }

        if( pHistory )
        {
            pHistory->TmpRollback( pDoc, nSetPos, false );
            if( nSetPos )       
            {
                
                if( nSetPos < pHistory->Count() )
                {
                    
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

        if( !bJoinNext )           
            rPam.Exchange();
    }

    if( pHistory )      
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

        
        if( !rPam.Move( fnMoveBackward, fnGoCntnt ) &&
            !rPam.Move( fnMoveForward, fnGoCntnt ) )
            rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), 0 );
    }
    else if( bDelFullPara )
    {
        
        
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


void SwUndoDelete::SetTableName(const OUString & rName)
{
    sTableName = rName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
