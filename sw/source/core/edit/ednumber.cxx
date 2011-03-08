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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <hintids.hxx>
#include <editsh.hxx>
#include <edimp.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <swundo.hxx>
#include <numrule.hxx>

SV_IMPL_VARARR_SORT( _SwPamRanges, SwPamRange )


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

    USHORT nPos = 0;
    const SwPamRange* pTmp;
    if( Count() && Seek_Entry( aRg, &nPos ))        // suche Insert Position
    {
        // ist der im Array stehende kleiner ??
        if( ( pTmp = GetData()+ nPos )->nEnd < aRg.nEnd )
        {
            aRg.nEnd = pTmp->nEnd;
            Remove( nPos, 1 );      // zusammenfassen
        }
        else
            return;     // ende, weil schon alle zusammengefasst waren
    }

    BOOL bEnde;
    do {
        bEnde = TRUE;

        // mit dem Vorgaenger zusammenfassen ??
        if( nPos > 0 )
        {
            if( ( pTmp = GetData()+( nPos-1 ))->nEnd == aRg.nStart
                || pTmp->nEnd+1 == aRg.nStart )
            {
                aRg.nStart = pTmp->nStart;
                bEnde = FALSE;
                Remove( --nPos, 1 );        // zusammenfassen
            }
            // SSelection im Bereich ??
            else if( pTmp->nStart <= aRg.nStart && aRg.nEnd <= pTmp->nEnd )
                return;
        }
            // mit dem Nachfolger zusammenfassen ??
        if( nPos < Count() )
        {
            if( ( pTmp = GetData() + nPos )->nStart == aRg.nEnd ||
                pTmp->nStart == aRg.nEnd+1 )
            {
                aRg.nEnd = pTmp->nEnd;
                bEnde = FALSE;
                Remove( nPos, 1 );      // zusammenfassen
            }

            // SSelection im Bereich ??
            else if( pTmp->nStart <= aRg.nStart && aRg.nEnd <= pTmp->nEnd )
                return;
        }
    } while( !bEnde );

    _SwPamRanges::Insert( aRg );
}



SwPaM& SwPamRanges::SetPam( USHORT nArrPos, SwPaM& rPam )
{
    OSL_ASSERT( nArrPos < Count() );
    const SwPamRange& rTmp = *(GetData() + nArrPos );
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

BOOL SwEditShell::NoNum()
{
    BOOL bRet = TRUE;
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         // Mehrfachselektion ?
    {
        GetDoc()->StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
            bRet = bRet && GetDoc()->NoNum( aRangeArr.SetPam( n, aPam ));
        GetDoc()->EndUndo( UNDO_END, NULL );
    }
    else
        bRet = GetDoc()->NoNum( *pCrsr );

    EndAllAction();
    return bRet;
}
// Loeschen, Splitten der Aufzaehlungsliste

// -> #i29560#
BOOL SwEditShell::HasNumber() const
{
    BOOL bResult = FALSE;

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
            bResult = FALSE;
        }
    }

    return bResult;
}

BOOL SwEditShell::HasBullet() const
{
    BOOL bResult = FALSE;

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
        GetDoc()->StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
        {
            GetDoc()->DelNumRules( aRangeArr.SetPam( n, aPam ) );
        }
        GetDoc()->EndUndo( UNDO_END, NULL );
    }
    else
        GetDoc()->DelNumRules( *pCrsr );

    // rufe das AttrChangeNotify auf der UI-Seite. Sollte eigentlich
    // ueberfluessig sein, aber VB hatte darueber eine Bugrep.
    CallChgLnk();

    // cursor can not be anymore in front of a label,
    // because numbering/bullet is deleted.
    SetInFrontOfLabel( FALSE );

    GetDoc()->SetModified();
    EndAllAction();
}

// Hoch-/Runterstufen


BOOL SwEditShell::NumUpDown( BOOL bDown )
{
    StartAllAction();

    BOOL bRet = TRUE;
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() == pCrsr )         // keine Mehrfachselektion ?
        bRet = GetDoc()->NumUpDown( *pCrsr, bDown );
    else
    {
        GetDoc()->StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
            bRet = bRet && GetDoc()->NumUpDown( aRangeArr.SetPam( n, aPam ), bDown );
        GetDoc()->EndUndo( UNDO_END, NULL );
    }
    GetDoc()->SetModified();

    // --> FME 2005-09-19 #i54693# Update marked numbering levels
    if ( IsInFrontOfLabel() )
        UpdateMarkedListLevel();
    // <--

    CallChgLnk();

    EndAllAction();
    return bRet;
}
// -> #i23726#
BOOL SwEditShell::IsFirstOfNumRule() const
{
    BOOL bResult = FALSE;

    SwPaM * pCrsr = GetCrsr();
    if (pCrsr->GetNext() == pCrsr)
    {
        bResult = IsFirstOfNumRule(*pCrsr);
    }

    return bResult;
}

BOOL SwEditShell::IsFirstOfNumRule(const SwPaM & rPaM) const
{
    BOOL bResult = FALSE;

    SwPosition aPos(*rPaM.GetPoint());
    bResult = GetDoc()->IsFirstOfNumRule(aPos);

    return bResult;
}
// <- #i23726#

// -> #i23725#
// --> OD 2008-06-09 #i90078#
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
        // --> OD 2008-06-09 #i90078#
        aRule.ChangeIndent( nDiff );
        // <--

        // no start of new list
        SetCurNumRule( aRule, false );
    }

    EndAllAction();
}

// --> OD 2008-06-09 #i90078#
// Adjust method name
void SwEditShell::SetIndent(short nIndent, const SwPosition & rPos)
// <--
{
    StartAllAction();

    SwNumRule *pCurNumRule = GetDoc()->GetCurrNumRule(rPos);

    if (pCurNumRule)
    {
        SwPaM aPaM(rPos);
        SwTxtNode * pTxtNode = aPaM.GetNode()->GetTxtNode();

        // --> OD 2008-06-09 #i90078#

        SwNumRule aRule(*pCurNumRule);

        if ( IsFirstOfNumRule() )
        {
            aRule.SetIndentOfFirstListLevelAndChangeOthers( nIndent );
        }
        else if ( pTxtNode->GetActualListLevel() >= 0  )
        {
            aRule.SetIndent( nIndent,
                             static_cast<USHORT>(pTxtNode->GetActualListLevel()) );
        }
        // <--

        // --> OD 2005-02-18 #i42921# - 3rd parameter = false in order to
        // suppress setting of num rule at <aPaM>.
        // do not apply any list
        GetDoc()->SetNumRule( aPaM, aRule, false, String(), sal_False );
        // <--
    }

    EndAllAction();
}
// <- #i23725#

BOOL SwEditShell::MoveParagraph( long nOffset )
{
    StartAllAction();

    SwPaM *pCrsr = GetCrsr();
    if( !pCrsr->HasMark() )
    {
        // sorge dafuer, das Bound1 und Bound2 im gleichen Node stehen
        pCrsr->SetMark();
        pCrsr->DeleteMark();
    }

    BOOL bRet = GetDoc()->MoveParagraph( *pCrsr, nOffset );

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
    GetDoc()->GotoNextNum( *aCrsr.GetPoint(), FALSE,
                            &rUpper, &rLower );
}

BOOL SwEditShell::MoveNumParas( BOOL bUpperLower, BOOL bUpperLeft )
{
    StartAllAction();

    // auf alle Selektionen ??
    SwPaM* pCrsr = GetCrsr();
    SwPaM aCrsr( *pCrsr->Start() );
    aCrsr.SetMark();

    if( pCrsr->HasMark() )
        *aCrsr.GetPoint() = *pCrsr->End();

    BOOL bRet = FALSE;
    BYTE nUpperLevel, nLowerLevel;
    if( GetDoc()->GotoNextNum( *aCrsr.GetPoint(), FALSE,
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
                if( GetDoc()->GotoPrevNum( aPos, FALSE ) )
                    nOffset = aPos.nNode.GetIndex() -
                            aCrsr.GetMark()->nNode.GetIndex();
                else
                {
                    ULONG nStt = aPos.nNode.GetIndex(), nIdx = nStt - 1;
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
                const SwNumRule* pOrig = aCrsr.GetNode(FALSE)->GetTxtNode()->GetNumRule();
                if( aCrsr.GetNode()->IsTxtNode() &&
                    pOrig == aCrsr.GetNode()->GetTxtNode()->GetNumRule() )
                {
                    ULONG nStt = aCrsr.GetPoint()->nNode.GetIndex(), nIdx = nStt+1;

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
                        // --> OD 2005-11-14 #i57856#
                        else
                        {
                            break;
                        }
                        // <--
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

BOOL SwEditShell::OutlineUpDown( short nOffset )
{
    StartAllAction();

    BOOL bRet = TRUE;
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() == pCrsr )         // keine Mehrfachselektion ?
        bRet = GetDoc()->OutlineUpDown( *pCrsr, nOffset );
    else
    {
        GetDoc()->StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
            bRet = bRet && GetDoc()->OutlineUpDown(
                                    aRangeArr.SetPam( n, aPam ), nOffset );
        GetDoc()->EndUndo( UNDO_END, NULL );
    }
    GetDoc()->SetModified();
    EndAllAction();
    return bRet;
}


BOOL SwEditShell::MoveOutlinePara( short nOffset )
{
    StartAllAction();
    BOOL bRet = GetDoc()->MoveOutlinePara( *GetCrsr(), nOffset );
    EndAllAction();
    return bRet;
}

// Outlines and SubOutline are ReadOnly?
BOOL SwEditShell::IsProtectedOutlinePara() const
{
    BOOL bRet = FALSE;
    const SwNode& rNd = GetCrsr()->Start()->nNode.GetNode();
    if( rNd.IsTxtNode() )
    {
        const SwOutlineNodes& rOutlNd = GetDoc()->GetNodes().GetOutLineNds();
        SwNodePtr pNd = (SwNodePtr)&rNd;
        BOOL bFirst = TRUE;
        USHORT nPos;
        int nLvl(0);
        if( !rOutlNd.Seek_Entry( pNd, &nPos ) && nPos )
            --nPos;

        for( ; nPos < rOutlNd.Count(); ++nPos )
        {
            SwNodePtr pTmpNd = rOutlNd[ nPos ];

            int nTmpLvl = pTmpNd->GetTxtNode()->GetAttrOutlineLevel();

            OSL_ENSURE( nTmpLvl >= 0 && nTmpLvl <= MAXLEVEL,
                    "<SwEditShell::IsProtectedOutlinePara()>" );

            if( bFirst )
            {
                nLvl = nTmpLvl;
                bFirst = FALSE;
            }
            else if( nLvl >= nTmpLvl )
                break;

            if( pTmpNd->IsProtect() )
            {
                bRet = TRUE;
                break;
            }
        }
    }
#if OSL_DEBUG_LEVEL > 1
    else
    {
        OSL_ENSURE(!this, "Cursor not on an outline node" );
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
BOOL lcl_IsOutlineMoveAndCopyable( const SwDoc* pDoc, USHORT nIdx, bool bCopy )
{
    const SwNodes& rNds = pDoc->GetNodes();
    const SwNode* pNd = rNds.GetOutLineNds()[ nIdx ];
    return pNd->GetIndex() >= rNds.GetEndOfExtras().GetIndex() &&   // 1) body
            !pNd->FindTableNode() &&                                // 2) table
            ( bCopy || !pNd->IsProtect() );                         // 3) write
}

BOOL SwEditShell::IsOutlineMovable( USHORT nIdx ) const
{
    return lcl_IsOutlineMoveAndCopyable( GetDoc(), nIdx, false );
}

BOOL SwEditShell::IsOutlineCopyable( USHORT nIdx ) const
{
    return lcl_IsOutlineMoveAndCopyable( GetDoc(), nIdx, true );
}


BOOL SwEditShell::NumOrNoNum( BOOL bNumOn, BOOL bChkStart )
{
    BOOL bRet = FALSE;
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

BOOL SwEditShell::IsNoNum( BOOL bChkStart ) const
{
    // ein Backspace im Absatz ohne Nummer wird zum Delete
    BOOL bResult = FALSE;
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

BYTE SwEditShell::GetNumLevel() const
{
    // gebe die akt. Ebene zurueck, auf der sich der Point vom Cursor befindet
    BYTE nLevel = MAXLEVEL;     //end,zhaojianwei

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
            nLevel = static_cast<BYTE>( nListLevelOfTxtNode );
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

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         // Mehrfachselektion ?
    {
        GetDoc()->StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
          {
            aRangeArr.SetPam( n, aPam );
            GetDoc()->SetNumRule( aPam, rRule,
                                  bCreateNewList, sContinuedListId,
                                  sal_True, bResetIndentAttrs );
            GetDoc()->SetCounted( aPam, true );
          }
        GetDoc()->EndUndo( UNDO_END, NULL );
    }
    else
    {
        GetDoc()->StartUndo( UNDO_START, NULL );

        GetDoc()->SetNumRule( *pCrsr, rRule,
                              bCreateNewList, sContinuedListId,
                              sal_True, bResetIndentAttrs );
        GetDoc()->SetCounted( *pCrsr, true );

        GetDoc()->EndUndo( UNDO_END, NULL );
    }

    EndAllAction();
}

String SwEditShell::GetUniqueNumRuleName( const String* pChkStr, BOOL bAutoNum ) const
{
    return GetDoc()->GetUniqueNumRuleName( pChkStr, bAutoNum );
}

void SwEditShell::ChgNumRuleFmts( const SwNumRule& rRule )
{
    StartAllAction();
    GetDoc()->ChgNumRuleFmts( rRule );
    EndAllAction();
}

BOOL SwEditShell::ReplaceNumRule( const String& rOldRule, const String& rNewRule )
{
    StartAllAction();
    BOOL bRet = GetDoc()->ReplaceNumRule( *GetCrsr()->GetPoint(), rOldRule, rNewRule );
    EndAllAction();
    return bRet;
}

void SwEditShell::SetNumRuleStart( BOOL bFlag )
{
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         // Mehrfachselektion ?
    {
        GetDoc()->StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
            GetDoc()->SetNumRuleStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), bFlag );
        GetDoc()->EndUndo( UNDO_END, NULL );
    }
    else
        GetDoc()->SetNumRuleStart( *pCrsr->GetPoint(), bFlag );

    EndAllAction();
}

BOOL SwEditShell::IsNumRuleStart() const
{
    BOOL bResult = FALSE;
    const SwTxtNode* pTxtNd = GetCrsr()->GetNode()->GetTxtNode();
    if( pTxtNd )
        bResult = pTxtNd->IsListRestart() ? TRUE : FALSE;
    return bResult;
}

void SwEditShell::SetNodeNumStart( USHORT nStt )
{
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         // Mehrfachselektion ?
    {
        GetDoc()->StartUndo( UNDO_START, NULL );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
            GetDoc()->SetNodeNumStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), nStt );
        GetDoc()->EndUndo( UNDO_END, NULL );
    }
    else
        GetDoc()->SetNodeNumStart( *pCrsr->GetPoint(), nStt );

    EndAllAction();
}

USHORT SwEditShell::GetNodeNumStart() const
{
    const SwTxtNode* pTxtNd = GetCrsr()->GetNode()->GetTxtNode();
    // correction: check, if list restart value is set at text node and
    // use new method <SwTxtNode::GetAttrListRestartValue()>.
    // return USHRT_MAX, if no list restart value is found.
    if ( pTxtNd && pTxtNd->HasAttrListRestartValue() )
    {
        return static_cast<USHORT>(pTxtNd->GetAttrListRestartValue());
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
