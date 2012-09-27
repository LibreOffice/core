/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

    _SwPamRanges::const_iterator it = lower_bound(aRg); //search Insert Position
    sal_uInt16 nPos = it - begin();
    if (!empty() && (it != end()) && (*it) == aRg)
    {
        // is the one in the Array smaller?
        SwPamRange const& rTmp = _SwPamRanges::operator[](nPos);
        if( rTmp.nEnd < aRg.nEnd )
        {
            aRg.nEnd = rTmp.nEnd;
            erase(begin() + nPos); // combine
        }
        else
            return; // done, because by precondition everything is combined
    }

    sal_Bool bEnde;
    do {
        bEnde = sal_True;

        // combine with predecessor?
        if( nPos > 0 )
        {
            SwPamRange const& rTmp = _SwPamRanges::operator[](nPos-1);
            if( rTmp.nEnd == aRg.nStart
                || rTmp.nEnd+1 == aRg.nStart )
            {
                aRg.nStart = rTmp.nStart;
                bEnde = sal_False;
                erase( begin() + --nPos ); // combine
            }
            // range contained in rTmp?
            else if( rTmp.nStart <= aRg.nStart && aRg.nEnd <= rTmp.nEnd )
                return;
        }
        // combine with successor?
        if( nPos < size() )
        {
            SwPamRange const& rTmp = _SwPamRanges::operator[](nPos);
            if( rTmp.nStart == aRg.nEnd ||
                rTmp.nStart == aRg.nEnd+1 )
            {
                aRg.nEnd = rTmp.nEnd;
                bEnde = sal_False;
                erase( begin() + nPos ); // combine
            }

            // range contained in rTmp?
            else if( rTmp.nStart <= aRg.nStart && aRg.nEnd <= rTmp.nEnd )
                return;
        }
    } while( !bEnde );

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



// Numerierung Outline Regelwerk


void SwEditShell::SetOutlineNumRule(const SwNumRule& rRule)
{
    StartAllAction();       // Klammern fuers Updaten !!
    GetDoc()->SetOutlineNumRule(rRule);
    EndAllAction();
}


const SwNumRule* SwEditShell::GetOutlineNumRule() const
{
    return GetDoc()->GetOutlineNumRule();
}

// setzt, wenn noch keine Numerierung, sonst wird geaendert
// arbeitet mit alten und neuen Regeln, nur Differenzen aktualisieren

// Absaetze ohne Numerierung, aber mit Einzuegen

sal_Bool SwEditShell::NoNum()
{
    sal_Bool bRet = sal_True;
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         // Mehrfachselektion ?
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
// Loeschen, Splitten der Aufzaehlungsliste

// -> #i29560#
sal_Bool SwEditShell::HasNumber() const
{
    sal_Bool bResult = sal_False;

    const SwTxtNode * pTxtNd =
        GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();

    if (pTxtNd)
    {
        bResult = pTxtNd->HasNumber();

        // special case: outline numbered, not counted paragraph
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
// <- #i29560#

void SwEditShell::DelNumRules()
{
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         // Mehrfachselektion ?
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

    // rufe das AttrChangeNotify auf der UI-Seite. Sollte eigentlich
    // ueberfluessig sein, aber VB hatte darueber eine Bugrep.
    CallChgLnk();

    // cursor can not be anymore in front of a label,
    // because numbering/bullet is deleted.
    SetInFrontOfLabel( false );

    GetDoc()->SetModified();
    EndAllAction();
}

// Hoch-/Runterstufen


sal_Bool SwEditShell::NumUpDown( sal_Bool bDown )
{
    StartAllAction();

    sal_Bool bRet = sal_True;
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() == pCrsr )         // keine Mehrfachselektion ?
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

    // #i54693# Update marked numbering levels
    if ( IsInFrontOfLabel() )
        UpdateMarkedListLevel();

    CallChgLnk();

    EndAllAction();
    return bRet;
}
// -> #i23726#
sal_Bool SwEditShell::IsFirstOfNumRule() const
{
    sal_Bool bResult = sal_False;

    SwPaM * pCrsr = GetCrsr();
    if (pCrsr->GetNext() == pCrsr)
    {
        bResult = IsFirstOfNumRule(*pCrsr);
    }

    return bResult;
}

sal_Bool SwEditShell::IsFirstOfNumRule(const SwPaM & rPaM) const
{
    sal_Bool bResult = sal_False;

    SwPosition aPos(*rPaM.GetPoint());
    bResult = GetDoc()->IsFirstOfNumRule(aPos);

    return bResult;
}
// <- #i23726#

// -> #i23725#
// #i90078#
// Remove unused default parameter <nLevel> and <bRelative>.
// Adjust method name and parameter name
void SwEditShell::ChangeIndentOfAllListLevels( short nDiff )
{
    StartAllAction();

    const SwNumRule *pCurNumRule = GetCurNumRule();
    //#120911# check if numbering rule really exists
    if (pCurNumRule)
    {
        SwNumRule aRule(*pCurNumRule);
        // #i90078#
        aRule.ChangeIndent( nDiff );

        // no start of new list
        SetCurNumRule( aRule, false );
    }

    EndAllAction();
}

// #i90078#
// Adjust method name
void SwEditShell::SetIndent(short nIndent, const SwPosition & rPos)
{
    StartAllAction();

    SwNumRule *pCurNumRule = GetDoc()->GetCurrNumRule(rPos);

    if (pCurNumRule)
    {
        SwPaM aPaM(rPos);
        SwTxtNode * pTxtNode = aPaM.GetNode()->GetTxtNode();

        // #i90078#

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

        // #i42921# - 3rd parameter = false in order to
        // suppress setting of num rule at <aPaM>.
        // do not apply any list
        GetDoc()->SetNumRule( aPaM, aRule, false, String(), sal_False );
    }

    EndAllAction();
}
// <- #i23725#

sal_Bool SwEditShell::MoveParagraph( long nOffset )
{
    StartAllAction();

    SwPaM *pCrsr = GetCrsr();
    if( !pCrsr->HasMark() )
    {
        // sorge dafuer, das Bound1 und Bound2 im gleichen Node stehen
        pCrsr->SetMark();
        pCrsr->DeleteMark();
    }

    sal_Bool bRet = GetDoc()->MoveParagraph( *pCrsr, nOffset );

    GetDoc()->SetModified();
    EndAllAction();
    return bRet;
}

//#outline level add by zhaojianwei
int SwEditShell::GetCurrentParaOutlineLevel( ) const
{
    int nLevel = 0;

    SwPaM* pCrsr = GetCrsr();
    const SwTxtNode* pTxtNd = pCrsr->GetNode()->GetTxtNode();
    if( pTxtNd )
        nLevel = pTxtNd->GetAttrOutlineLevel();
    return nLevel;
}
//<-end,zhaojianwei

void SwEditShell::GetCurrentOutlineLevels( sal_uInt8& rUpper, sal_uInt8& rLower )
{
    SwPaM* pCrsr = GetCrsr();
    SwPaM aCrsr( *pCrsr->Start() );
    aCrsr.SetMark();
    if( pCrsr->HasMark() )
        *aCrsr.GetPoint() = *pCrsr->End();
    GetDoc()->GotoNextNum( *aCrsr.GetPoint(), sal_False,
                            &rUpper, &rLower );
}

sal_Bool SwEditShell::MoveNumParas( sal_Bool bUpperLower, sal_Bool bUpperLeft )
{
    StartAllAction();

    // auf alle Selektionen ??
    SwPaM* pCrsr = GetCrsr();
    SwPaM aCrsr( *pCrsr->Start() );
    aCrsr.SetMark();

    if( pCrsr->HasMark() )
        *aCrsr.GetPoint() = *pCrsr->End();

    sal_Bool bRet = sal_False;
    sal_uInt8 nUpperLevel, nLowerLevel;
    if( GetDoc()->GotoNextNum( *aCrsr.GetPoint(), sal_False,
                                &nUpperLevel, &nLowerLevel ))
    {
        if( bUpperLower )
        {
            // ueber die naechste Nummerierung
            long nOffset = 0;
            const SwNode* pNd;

            if( bUpperLeft )        // verschiebe nach oben
            {
                SwPosition aPos( *aCrsr.GetMark() );
                if( GetDoc()->GotoPrevNum( aPos, sal_False ) )
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
            else                    // verschiebe nach unten
            {
                const SwNumRule* pOrig = aCrsr.GetNode(sal_False)->GetTxtNode()->GetNumRule();
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
                        // #i57856#
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

sal_Bool SwEditShell::OutlineUpDown( short nOffset )
{
    StartAllAction();

    sal_Bool bRet = sal_True;
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() == pCrsr )         // keine Mehrfachselektion ?
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


sal_Bool SwEditShell::MoveOutlinePara( short nOffset )
{
    StartAllAction();
    sal_Bool bRet = GetDoc()->MoveOutlinePara( *GetCrsr(), nOffset );
    EndAllAction();
    return bRet;
}

// Outlines and SubOutline are ReadOnly?
sal_Bool SwEditShell::IsProtectedOutlinePara() const
{
    sal_Bool bRet = sal_False;
    const SwNode& rNd = GetCrsr()->Start()->nNode.GetNode();
    if( rNd.IsTxtNode() )
    {
        const SwOutlineNodes& rOutlNd = GetDoc()->GetNodes().GetOutLineNds();
        SwNodePtr pNd = (SwNodePtr)&rNd;
        sal_Bool bFirst = sal_True;
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
                bFirst = sal_False;
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
sal_Bool lcl_IsOutlineMoveAndCopyable( const SwDoc* pDoc, sal_uInt16 nIdx, bool bCopy )
{
    const SwNodes& rNds = pDoc->GetNodes();
    const SwNode* pNd = rNds.GetOutLineNds()[ nIdx ];
    return pNd->GetIndex() >= rNds.GetEndOfExtras().GetIndex() &&   // 1) body
            !pNd->FindTableNode() &&                                // 2) table
            ( bCopy || !pNd->IsProtect() );                         // 3) write
}

sal_Bool SwEditShell::IsOutlineMovable( sal_uInt16 nIdx ) const
{
    return lcl_IsOutlineMoveAndCopyable( GetDoc(), nIdx, false );
}

sal_Bool SwEditShell::IsOutlineCopyable( sal_uInt16 nIdx ) const
{
    return lcl_IsOutlineMoveAndCopyable( GetDoc(), nIdx, true );
}


sal_Bool SwEditShell::NumOrNoNum( sal_Bool bNumOn, sal_Bool bChkStart )
{
    sal_Bool bRet = sal_False;
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() == pCrsr && !pCrsr->HasMark() &&
        ( !bChkStart || !pCrsr->GetPoint()->nContent.GetIndex()) )
    {
        StartAllAction();       // Klammern fuers Updaten !!
        bRet = GetDoc()->NumOrNoNum( pCrsr->GetPoint()->nNode, !bNumOn ); // #i29560#
        EndAllAction();
    }
    return bRet;
}

sal_Bool SwEditShell::IsNoNum( sal_Bool bChkStart ) const
{
    // ein Backspace im Absatz ohne Nummer wird zum Delete
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
    // gebe die akt. Ebene zurueck, auf der sich der Point vom Cursor befindet
    sal_uInt8 nLevel = MAXLEVEL;        //end,zhaojianwei

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
                                 const String sContinuedListId,
                                 const bool bResetIndentAttrs )
{
    StartAllAction();

    GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         // Mehrfachselektion ?
    {
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
          {
            aRangeArr.SetPam( n, aPam );
            GetDoc()->SetNumRule( aPam, rRule,
                                  bCreateNewList, sContinuedListId,
                                  sal_True, bResetIndentAttrs );
            GetDoc()->SetCounted( aPam, true );
          }
    }
    else
    {
        GetDoc()->SetNumRule( *pCrsr, rRule,
                              bCreateNewList, sContinuedListId,
                              sal_True, bResetIndentAttrs );
        GetDoc()->SetCounted( *pCrsr, true );
    }
    GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );

    EndAllAction();
}

String SwEditShell::GetUniqueNumRuleName( const String* pChkStr, sal_Bool bAutoNum ) const
{
    return GetDoc()->GetUniqueNumRuleName( pChkStr, bAutoNum );
}

void SwEditShell::ChgNumRuleFmts( const SwNumRule& rRule )
{
    StartAllAction();
    GetDoc()->ChgNumRuleFmts( rRule );
    EndAllAction();
}

sal_Bool SwEditShell::ReplaceNumRule( const String& rOldRule, const String& rNewRule )
{
    StartAllAction();
    sal_Bool bRet = GetDoc()->ReplaceNumRule( *GetCrsr()->GetPoint(), rOldRule, rNewRule );
    EndAllAction();
    return bRet;
}

void SwEditShell::SetNumRuleStart( sal_Bool bFlag, SwPaM* pPaM )
{
    StartAllAction();
    SwPaM* pCrsr = pPaM ? pPaM : GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         // Mehrfachselektion ?
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
    if( pCrsr->GetNext() != pCrsr )         // Mehrfachselektion ?
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
    // correction: check, if list restart value is set at text node and
    // use new method <SwTxtNode::GetAttrListRestartValue()>.
    // return USHRT_MAX, if no list restart value is found.
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
                                              String& sListId )
{
    return GetDoc()->SearchNumRule( *(bForward ? GetCrsr()->End() : GetCrsr()->Start()),
                                    bForward, bNum, bOutline, nNonEmptyAllowed,
                                    sListId );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
