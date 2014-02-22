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

#include <hintids.hxx>
#include <editsh.hxx>
#include <edimp.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <swundo.hxx>
#include <numrule.hxx>

SwPamRanges::SwPamRanges( const SwPaM& rRing )
{
    const SwPaM* pTmp = &rRing;
    do {
        Insert( pTmp->GetMark()->nNode, pTmp->GetPoint()->nNode );
    } while( &rRing != ( pTmp = (const SwPaM*)pTmp->GetNext() ));
}

void SwPamRanges::Insert( const SwNodeIndex& rIdx1, const SwNodeIndex& rIdx2 )
{
    SwPamRange aRg( rIdx1.GetIndex(), rIdx2.GetIndex() );
    if( aRg.nEnd < aRg.nStart )
    {   aRg.nStart = aRg.nEnd; aRg.nEnd = rIdx1.GetIndex(); }

    _SwPamRanges::const_iterator it = lower_bound(aRg); 
    sal_uInt16 nPos = it - begin();
    if (!empty() && (it != end()) && (*it) == aRg)
    {
        
        SwPamRange const& rTmp = _SwPamRanges::operator[](nPos);
        if( rTmp.nEnd < aRg.nEnd )
        {
            aRg.nEnd = rTmp.nEnd;
            erase(begin() + nPos); 
        }
        else
            return; 
    }

    bool bEnd;
    do {
        bEnd = true;

        
        if( nPos > 0 )
        {
            SwPamRange const& rTmp = _SwPamRanges::operator[](nPos-1);
            if( rTmp.nEnd == aRg.nStart
                || rTmp.nEnd+1 == aRg.nStart )
            {
                aRg.nStart = rTmp.nStart;
                bEnd = false;
                erase( begin() + --nPos ); 
            }
            
            else if( rTmp.nStart <= aRg.nStart && aRg.nEnd <= rTmp.nEnd )
                return;
        }
        
        if( nPos < size() )
        {
            SwPamRange const& rTmp = _SwPamRanges::operator[](nPos);
            if( rTmp.nStart == aRg.nEnd ||
                rTmp.nStart == aRg.nEnd+1 )
            {
                aRg.nEnd = rTmp.nEnd;
                bEnd = false;
                erase( begin() + nPos ); 
            }

            
            else if( rTmp.nStart <= aRg.nStart && aRg.nEnd <= rTmp.nEnd )
                return;
        }
    } while( !bEnd );

    _SwPamRanges::insert( aRg );
}

SwPaM& SwPamRanges::SetPam( sal_uInt16 nArrPos, SwPaM& rPam )
{
    OSL_ASSERT( nArrPos < Count() );
    const SwPamRange& rTmp = (*this)[ nArrPos ];
    rPam.GetPoint()->nNode = rTmp.nStart;
    rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), 0 );
    rPam.SetMark();
    rPam.GetPoint()->nNode = rTmp.nEnd;
    rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), 0 );
    return rPam;
}



void SwEditShell::SetOutlineNumRule(const SwNumRule& rRule)
{
    StartAllAction();       
    GetDoc()->SetOutlineNumRule(rRule);
    EndAllAction();
}

const SwNumRule* SwEditShell::GetOutlineNumRule() const
{
    return GetDoc()->GetOutlineNumRule();
}





bool SwEditShell::NoNum()
{
    bool bRet = true;
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
            bRet = bRet && GetDoc()->NoNum( aRangeArr.SetPam( n, aPam ));
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
    else
        bRet = GetDoc()->NoNum( *pCrsr );

    EndAllAction();
    return bRet;
}

sal_Bool SwEditShell::SelectionHasNumber() const
{
    sal_Bool bResult = HasNumber();
    const SwTxtNode * pTxtNd =
        GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();
    if (!bResult && pTxtNd && pTxtNd->Len()==0 && !pTxtNd->GetNumRule()) {
        SwPamRanges aRangeArr( *GetCrsr() );
        SwPaM aPam( *GetCrsr()->GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
        {
            aRangeArr.SetPam( n, aPam );
            {
                sal_uInt32 nStt = aPam.GetPoint()->nNode.GetIndex(),
                      nEnd = aPam.GetMark()->nNode.GetIndex();
                if( nStt > nEnd )
                {
                    sal_uInt32 nTmp = nStt; nStt = nEnd; nEnd = nTmp;
                }
                for (sal_uInt32 nPos = nStt; nPos<=nEnd; nPos++)
                {
                    pTxtNd = mpDoc->GetNodes()[nPos]->GetTxtNode();
                    if (pTxtNd && pTxtNd->Len()!=0)
                    {
                        bResult = pTxtNd->HasNumber();

                        
                        if ( bResult &&
                            pTxtNd->GetNumRule() == GetDoc()->GetOutlineNumRule() &&
                            !pTxtNd->IsCountedInList() )
                        {
                            bResult = sal_False;
                        }
                        if (bResult==sal_False) {
                            break;
                        }
                    }
                }
            }
        }

    }

    return bResult;
}


sal_Bool SwEditShell::SelectionHasBullet() const
{
    sal_Bool bResult = HasBullet();
    const SwTxtNode * pTxtNd =
        GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();
    if (!bResult && pTxtNd && pTxtNd->Len()==0 && !pTxtNd->GetNumRule()) {
        SwPamRanges aRangeArr( *GetCrsr() );
        SwPaM aPam( *GetCrsr()->GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
        {
            aRangeArr.SetPam( n, aPam );
            {
                sal_uInt32 nStt = aPam.GetPoint()->nNode.GetIndex(),
                      nEnd = aPam.GetMark()->nNode.GetIndex();
                if( nStt > nEnd )
                {
                    sal_uInt32 nTmp = nStt; nStt = nEnd; nEnd = nTmp;
                }
                for (sal_uInt32 nPos = nStt; nPos<=nEnd; nPos++)
                {
                    pTxtNd = mpDoc->GetNodes()[nPos]->GetTxtNode();
                    if (pTxtNd && pTxtNd->Len()!=0)
                    {
                        bResult = pTxtNd->HasBullet();

                        if (bResult==sal_False) {
                            break;
                        }
                    }
                }
            }
        }
    }

    return bResult;
}


sal_Bool SwEditShell::HasNumber() const
{
    sal_Bool bResult = sal_False;

    const SwTxtNode * pTxtNd =
        GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();

    if (pTxtNd)
    {
        bResult = pTxtNd->HasNumber();

        
        if ( bResult &&
             pTxtNd->GetNumRule() == GetDoc()->GetOutlineNumRule() &&
             !pTxtNd->IsCountedInList() )
        {
            bResult = sal_False;
        }
    }

    return bResult;
}

sal_Bool SwEditShell::HasBullet() const
{
    sal_Bool bResult = sal_False;

    const SwTxtNode * pTxtNd =
        GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();

    if (pTxtNd)
    {
        bResult = pTxtNd->HasBullet();
    }

    return bResult;
}



void SwEditShell::DelNumRules()
{
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr ) 
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
        {
            GetDoc()->DelNumRules( aRangeArr.SetPam( n, aPam ) );
        }
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
    else
        GetDoc()->DelNumRules( *pCrsr );

    
    CallChgLnk();

    
    SetInFrontOfLabel( false );

    GetDoc()->SetModified();
    EndAllAction();
}


bool SwEditShell::NumUpDown( bool bDown )
{
    StartAllAction();

    bool bRet = true;
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() == pCrsr )         
        bRet = GetDoc()->NumUpDown( *pCrsr, bDown );
    else
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
            bRet = bRet && GetDoc()->NumUpDown( aRangeArr.SetPam( n, aPam ), bDown );
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
    GetDoc()->SetModified();

    
    if ( IsInFrontOfLabel() )
        UpdateMarkedListLevel();

    CallChgLnk();

    EndAllAction();
    return bRet;
}


bool SwEditShell::IsFirstOfNumRule() const
{
    bool bResult = false;

    SwPaM * pCrsr = GetCrsr();
    if (pCrsr->GetNext() == pCrsr)
    {
        bResult = IsFirstOfNumRule(*pCrsr);
    }

    return bResult;
}

bool SwEditShell::IsFirstOfNumRule(const SwPaM & rPaM) const
{
    SwPosition aPos(*rPaM.GetPoint());
    return (GetDoc()->IsFirstOfNumRule(aPos));
}



void SwEditShell::ChangeIndentOfAllListLevels( short nDiff )
{
    StartAllAction();

    const SwNumRule *pCurNumRule = GetCurNumRule();
    
    if (pCurNumRule)
    {
        SwNumRule aRule(*pCurNumRule);
        const SwNumFmt& aRootNumFmt(aRule.Get(0));
        if( nDiff > 0 || aRootNumFmt.GetIndentAt() + nDiff > 0) 
        {
            
            aRule.ChangeIndent( nDiff );
        }
        
        SetCurNumRule( aRule, false );
    }

    EndAllAction();
}


void SwEditShell::SetIndent(short nIndent, const SwPosition & rPos)
{
    StartAllAction();

    SwNumRule *pCurNumRule = GetDoc()->GetCurrNumRule(rPos);

    if (pCurNumRule)
    {
        SwPaM aPaM(rPos);
        SwTxtNode * pTxtNode = aPaM.GetNode()->GetTxtNode();

        SwNumRule aRule(*pCurNumRule);

        if ( IsFirstOfNumRule() )
        {
            aRule.SetIndentOfFirstListLevelAndChangeOthers( nIndent );
        }
        else if ( pTxtNode->GetActualListLevel() >= 0  )
        {
            aRule.SetIndent( nIndent,
                             static_cast<sal_uInt16>(pTxtNode->GetActualListLevel()) );
        }

        
        
        
        GetDoc()->SetNumRule( aPaM, aRule, false, OUString(), false );
    }

    EndAllAction();
}


bool SwEditShell::MoveParagraph( long nOffset )
{
    StartAllAction();

    SwPaM *pCrsr = GetCrsr();
    if( !pCrsr->HasMark() )
    {
        
        pCrsr->SetMark();
        pCrsr->DeleteMark();
    }

    bool bRet = GetDoc()->MoveParagraph( *pCrsr, nOffset );

    GetDoc()->SetModified();
    EndAllAction();
    return bRet;
}

int SwEditShell::GetCurrentParaOutlineLevel( ) const
{
    int nLevel = 0;

    SwPaM* pCrsr = GetCrsr();
    const SwTxtNode* pTxtNd = pCrsr->GetNode()->GetTxtNode();
    if( pTxtNd )
        nLevel = pTxtNd->GetAttrOutlineLevel();
    return nLevel;
}

void SwEditShell::GetCurrentOutlineLevels( sal_uInt8& rUpper, sal_uInt8& rLower )
{
    SwPaM* pCrsr = GetCrsr();
    SwPaM aCrsr( *pCrsr->Start() );
    aCrsr.SetMark();
    if( pCrsr->HasMark() )
        *aCrsr.GetPoint() = *pCrsr->End();
    GetDoc()->GotoNextNum( *aCrsr.GetPoint(), false, &rUpper, &rLower );
}

bool SwEditShell::MoveNumParas( bool bUpperLower, bool bUpperLeft )
{
    StartAllAction();

    
    SwPaM* pCrsr = GetCrsr();
    SwPaM aCrsr( *pCrsr->Start() );
    aCrsr.SetMark();

    if( pCrsr->HasMark() )
        *aCrsr.GetPoint() = *pCrsr->End();

    sal_Bool bRet = sal_False;
    sal_uInt8 nUpperLevel, nLowerLevel;
    if( GetDoc()->GotoNextNum( *aCrsr.GetPoint(), false,
                                &nUpperLevel, &nLowerLevel ))
    {
        if( bUpperLower )
        {
            
            long nOffset = 0;
            const SwNode* pNd;

            if( bUpperLeft ) 
            {
                SwPosition aPos( *aCrsr.GetMark() );
                if( GetDoc()->GotoPrevNum( aPos, false ) )
                    nOffset = aPos.nNode.GetIndex() -
                            aCrsr.GetMark()->nNode.GetIndex();
                else
                {
                    sal_uLong nStt = aPos.nNode.GetIndex(), nIdx = nStt - 1;
                    while( nIdx && (
                        ( pNd = GetDoc()->GetNodes()[ nIdx ])->IsSectionNode() ||
                        ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsSectionNode())))
                        --nIdx;
                    if( GetDoc()->GetNodes()[ nIdx ]->IsTxtNode() )
                        nOffset = nIdx - nStt;
                }
            }
            else             
            {
                const SwNumRule* pOrig = aCrsr.GetNode(false)->GetTxtNode()->GetNumRule();
                if( aCrsr.GetNode()->IsTxtNode() &&
                    pOrig == aCrsr.GetNode()->GetTxtNode()->GetNumRule() )
                {
                    sal_uLong nStt = aCrsr.GetPoint()->nNode.GetIndex(), nIdx = nStt+1;

                    while (nIdx < GetDoc()->GetNodes().Count()-1)
                    {
                        pNd = GetDoc()->GetNodes()[ nIdx ];

                        if (pNd->IsSectionNode() ||
                            ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsSectionNode()) ||
                            ( pNd->IsTxtNode() && pOrig == ((SwTxtNode*)pNd)->GetNumRule() &&
                              ((SwTxtNode*)pNd)->GetActualListLevel() > nUpperLevel ))
                        {
                            ++nIdx;
                        }
                        
                        else
                        {
                            break;
                        }
                    }

                    if( nStt == nIdx || !GetDoc()->GetNodes()[ nIdx ]->IsTxtNode() )
                        nOffset = 1;
                    else
                        nOffset = nIdx - nStt;
                }
                else
                    nOffset = 1;
            }

            if( nOffset )
            {
                aCrsr.Move( fnMoveBackward, fnGoNode );
                bRet = GetDoc()->MoveParagraph( aCrsr, nOffset );
            }
        }
        else if( bUpperLeft ? nUpperLevel : nLowerLevel+1 < MAXLEVEL )
        {
            aCrsr.Move( fnMoveBackward, fnGoNode );
            bRet = GetDoc()->NumUpDown( aCrsr, !bUpperLeft );
        }
    }

    GetDoc()->SetModified();
    EndAllAction();
    return bRet;
}

bool SwEditShell::OutlineUpDown( short nOffset )
{
    StartAllAction();

    bool bRet = true;
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() == pCrsr ) 
        bRet = GetDoc()->OutlineUpDown( *pCrsr, nOffset );
    else
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
            bRet = bRet && GetDoc()->OutlineUpDown(
                                    aRangeArr.SetPam( n, aPam ), nOffset );
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
    GetDoc()->SetModified();
    EndAllAction();
    return bRet;
}

bool SwEditShell::MoveOutlinePara( short nOffset )
{
    StartAllAction();
    bool bRet = GetDoc()->MoveOutlinePara( *GetCrsr(), nOffset );
    EndAllAction();
    return bRet;
}


sal_Bool SwEditShell::IsProtectedOutlinePara() const
{
    sal_Bool bRet = sal_False;
    const SwNode& rNd = GetCrsr()->Start()->nNode.GetNode();
    if( rNd.IsTxtNode() )
    {
        const SwOutlineNodes& rOutlNd = GetDoc()->GetNodes().GetOutLineNds();
        SwNodePtr pNd = (SwNodePtr)&rNd;
        bool bFirst = true;
        sal_uInt16 nPos;
        int nLvl(0);
        if( !rOutlNd.Seek_Entry( pNd, &nPos ) && nPos )
            --nPos;

        for( ; nPos < rOutlNd.size(); ++nPos )
        {
            SwNodePtr pTmpNd = rOutlNd[ nPos ];

            int nTmpLvl = pTmpNd->GetTxtNode()->GetAttrOutlineLevel();

            OSL_ENSURE( nTmpLvl >= 0 && nTmpLvl <= MAXLEVEL,
                    "<SwEditShell::IsProtectedOutlinePara()>" );

            if( bFirst )
            {
                nLvl = nTmpLvl;
                bFirst = false;
            }
            else if( nLvl >= nTmpLvl )
                break;

            if( pTmpNd->IsProtect() )
            {
                bRet = sal_True;
                break;
            }
        }
    }
#if OSL_DEBUG_LEVEL > 0
    else
    {
        OSL_FAIL("Cursor not on an outline node");
    }
#endif
    return bRet;
}

/** Test whether outline may be moved (bCopy == false)
 *                           or copied (bCopy == true)
 * Verify these conditions:
 * 1) outline must be within main body (and not in redline)
 * 2) outline must not be within table
 * 3) if bCopy is set, outline must not be write protected
 */
static sal_Bool lcl_IsOutlineMoveAndCopyable( const SwDoc* pDoc, sal_uInt16 nIdx, bool bCopy )
{
    const SwNodes& rNds = pDoc->GetNodes();
    const SwNode* pNd = rNds.GetOutLineNds()[ nIdx ];
    return pNd->GetIndex() >= rNds.GetEndOfExtras().GetIndex() &&   
            !pNd->FindTableNode() &&                                
            ( bCopy || !pNd->IsProtect() );                         
}

sal_Bool SwEditShell::IsOutlineMovable( sal_uInt16 nIdx ) const
{
    return lcl_IsOutlineMoveAndCopyable( GetDoc(), nIdx, false );
}

sal_Bool SwEditShell::IsOutlineCopyable( sal_uInt16 nIdx ) const
{
    return lcl_IsOutlineMoveAndCopyable( GetDoc(), nIdx, true );
}

bool SwEditShell::NumOrNoNum( sal_Bool bNumOn, bool bChkStart )
{
    bool bRet = false;
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() == pCrsr && !pCrsr->HasMark() &&
        ( !bChkStart || !pCrsr->GetPoint()->nContent.GetIndex()) )
    {
        StartAllAction();       
        bRet = GetDoc()->NumOrNoNum( pCrsr->GetPoint()->nNode, !bNumOn ); 
        EndAllAction();
    }
    return bRet;
}

sal_Bool SwEditShell::IsNoNum( sal_Bool bChkStart ) const
{
    
    sal_Bool bResult = sal_False;
    SwPaM* pCrsr = GetCrsr();

    if (pCrsr->GetNext() == pCrsr && !pCrsr->HasMark() &&
        (!bChkStart || !pCrsr->GetPoint()->nContent.GetIndex()))
    {
        const SwTxtNode* pTxtNd = pCrsr->GetNode()->GetTxtNode();

        if (pTxtNd)
        {
            bResult =  ! pTxtNd->IsCountedInList();
        }
    }

    return bResult;
}

sal_uInt8 SwEditShell::GetNumLevel() const
{
    
    sal_uInt8 nLevel = MAXLEVEL;

    SwPaM* pCrsr = GetCrsr();
    const SwTxtNode* pTxtNd = pCrsr->GetNode()->GetTxtNode();

    OSL_ENSURE( pTxtNd, "GetNumLevel() without text node" );
    if ( !pTxtNd )
        return nLevel;

    const SwNumRule* pRule = pTxtNd->GetNumRule();
    if(pRule)
    {
        const int nListLevelOfTxtNode( pTxtNd->GetActualListLevel() );
        if ( nListLevelOfTxtNode >= 0 )
        {
            nLevel = static_cast<sal_uInt8>( nListLevelOfTxtNode );
        }
    }

    return nLevel;
}

const SwNumRule* SwEditShell::GetCurNumRule() const
{
    return GetDoc()->GetCurrNumRule( *GetCrsr()->GetPoint() );
}

void SwEditShell::SetCurNumRule( const SwNumRule& rRule,
                                 const bool bCreateNewList,
                                 const OUString sContinuedListId,
                                 const bool bResetIndentAttrs )
{
    StartAllAction();

    GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         
    {
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
        {
            aRangeArr.SetPam( n, aPam );
            GetDoc()->SetNumRule( aPam, rRule,
                                  bCreateNewList, sContinuedListId,
                                  true , bResetIndentAttrs );
            GetDoc()->SetCounted( aPam, true );
        }
    }
    else
    {
        GetDoc()->SetNumRule( *pCrsr, rRule,
                              bCreateNewList, sContinuedListId,
                              true, bResetIndentAttrs );
        GetDoc()->SetCounted( *pCrsr, true );
    }
    GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );

    EndAllAction();
}

OUString SwEditShell::GetUniqueNumRuleName( const OUString* pChkStr, bool bAutoNum ) const
{
    return GetDoc()->GetUniqueNumRuleName( pChkStr, bAutoNum );
}

void SwEditShell::ChgNumRuleFmts( const SwNumRule& rRule )
{
    StartAllAction();
    GetDoc()->ChgNumRuleFmts( rRule );
    EndAllAction();
}

bool SwEditShell::ReplaceNumRule( const OUString& rOldRule, const OUString& rNewRule )
{
    StartAllAction();
    bool bRet = GetDoc()->ReplaceNumRule( *GetCrsr()->GetPoint(), rOldRule, rNewRule );
    EndAllAction();
    return bRet;
}

void SwEditShell::SetNumRuleStart( sal_Bool bFlag, SwPaM* pPaM )
{
    StartAllAction();
    SwPaM* pCrsr = pPaM ? pPaM : GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
            GetDoc()->SetNumRuleStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), bFlag );
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
    else
        GetDoc()->SetNumRuleStart( *pCrsr->GetPoint(), bFlag );

    EndAllAction();
}

sal_Bool SwEditShell::IsNumRuleStart( SwPaM* pPaM ) const
{
    sal_Bool bResult = sal_False;
    SwPaM* pCrsr = pPaM ? pPaM : GetCrsr( );
    const SwTxtNode* pTxtNd = pCrsr->GetNode()->GetTxtNode();
    if( pTxtNd )
        bResult = pTxtNd->IsListRestart() ? sal_True : sal_False;
    return bResult;
}

void SwEditShell::SetNodeNumStart( sal_uInt16 nStt, SwPaM* pPaM )
{
    StartAllAction();

    SwPaM* pCrsr = pPaM ? pPaM : GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
            GetDoc()->SetNodeNumStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), nStt );
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
    else
        GetDoc()->SetNodeNumStart( *pCrsr->GetPoint(), nStt );

    EndAllAction();
}

sal_uInt16 SwEditShell::GetNodeNumStart( SwPaM* pPaM ) const
{
    SwPaM* pCrsr = pPaM ? pPaM : GetCrsr();
    const SwTxtNode* pTxtNd = pCrsr->GetNode()->GetTxtNode();
    
    
    
    if ( pTxtNd && pTxtNd->HasAttrListRestartValue() )
    {
        return static_cast<sal_uInt16>(pTxtNd->GetAttrListRestartValue());
    }
    return USHRT_MAX;
}

const SwNumRule * SwEditShell::SearchNumRule( const bool bForward,
                                              const bool bNum,
                                              const bool bOutline,
                                              int nNonEmptyAllowed,
                                              OUString& sListId )
{
    return GetDoc()->SearchNumRule( *(bForward ? GetCrsr()->End() : GetCrsr()->Start()),
                                    bForward, bNum, bOutline, nNonEmptyAllowed,
                                    sListId );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
