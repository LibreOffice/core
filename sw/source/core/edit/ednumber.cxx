/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ednumber.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:39:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _EDIMP_HXX
#include <edimp.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif

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
    ASSERT_ID( nArrPos < Count(), ERR_VAR_IDX );
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

        // --> OD 2005-10-26 #b6340308#
        // special case: outline numbered, not counted paragraph
        if ( bResult &&
             pTxtNd->GetNumRule() == GetDoc()->GetOutlineNumRule() &&
             !pTxtNd->IsCounted() )
        {
            bResult = FALSE;
        }
        // <--
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

BOOL SwEditShell::DelNumRules()
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
            bRet = bRet && GetDoc()->DelNumRules( aRangeArr.SetPam( n, aPam ) );
        GetDoc()->EndUndo( UNDO_END, NULL );
    }
    else
        bRet = GetDoc()->DelNumRules( *pCrsr );

    // rufe das AttrChangeNotify auf der UI-Seite. Sollte eigentlich
    // ueberfluessig sein, aber VB hatte darueber eine Bugrep.
    CallChgLnk();

    // --> OD 2005-10-24 #126346# - cursor can not be anymore in
    // front of a label, because numbering/bullet is deleted.
    SetInFrontOfLabel( FALSE );
    // <--

    GetDoc()->SetModified();
    EndAllAction();
    return bRet;
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
        UpdateMarkedNumLevel();
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
void SwEditShell::NumIndent(short nIndent, int nLevel, BOOL bRelative)
{
    StartAllAction();

    const SwNumRule *pCurNumRule = GetCurNumRule();
    //#120911# check if numbering rule really exists
    if (pCurNumRule)
    {
        SwNumRule aRule(*pCurNumRule);
        // --> OD 2005-02-18 #i42921# - correction:
        // consider change of synopsis: 3rd parameter has to be -1
        aRule.Indent(nIndent, nLevel, -1, bRelative);
        // <--

        SetCurNumRule(aRule);
    }

    EndAllAction();
}

void SwEditShell::NumIndent(short nIndent, const SwPosition & rPos)
{
    StartAllAction();

    SwNumRule *pCurNumRule = GetDoc()->GetCurrNumRule(rPos);

    if (pCurNumRule)
    {
        SwPaM aPaM(rPos);
        SwTxtNode * pTxtNode = aPaM.GetNode()->GetTxtNode();

        int nLevel = -1;
        int nReferenceLevel = pTxtNode->GetLevel();

        if (! IsFirstOfNumRule(aPaM))
            nLevel = nReferenceLevel;

        SwNumRule aRule(*pCurNumRule);
        aRule.Indent(nIndent, nLevel, nReferenceLevel, FALSE);

        // --> OD 2005-02-18 #i42921# - 3rd parameter = false in order to
        // suppress setting of num rule at <aPaM>.
        GetDoc()->SetNumRule( aPaM, aRule, sal_False );
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
                              ((SwTxtNode*)pNd)->GetLevel() > nUpperLevel ))
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
        BYTE nLvl(0);
        if( !rOutlNd.Seek_Entry( pNd, &nPos ) && nPos )
            --nPos;

        for( ; nPos < rOutlNd.Count(); ++nPos )
        {
            SwNodePtr pTmpNd = rOutlNd[ nPos ];
            BYTE nTmpLvl = GetRealLevel( pTmpNd->GetTxtNode()->
                                    GetTxtColl()->GetOutlineLevel() );
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
#ifndef PRODUCT
    else
    {
        ASSERT(!this, "Cursor not on an outline node" );
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


BOOL SwEditShell::NumOrNoNum( BOOL bNumOn, BOOL bChkStart ) // #115901#
{
    BOOL bRet = FALSE;
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() == pCrsr && !pCrsr->HasMark() &&
        ( !bChkStart || !pCrsr->GetPoint()->nContent.GetIndex()) )
    {
        StartAllAction();       // Klammern fuers Updaten !!
        // #115901#
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
            bResult =  ! pTxtNd->IsCounted();
        }
    }

    return bResult;
}

BYTE SwEditShell::GetNumLevel( BOOL* pHasChilds ) const
{
    // gebe die akt. Ebene zurueck, auf der sich der Point vom Cursor befindet
    BYTE nLevel = NO_NUMBERING;

    SwPaM* pCrsr = GetCrsr();
    const SwTxtNode* pTxtNd = pCrsr->GetNode()->GetTxtNode();

    // --> FME 2005-09-12 #124972# Made code robust:
    ASSERT( pTxtNd, "GetNumLevel() without text node" )
    if ( !pTxtNd )
        return nLevel;
    // <--

    const SwNumRule* pRule = pTxtNd->GetNumRule();
    if(pRule)
    {
        nLevel = static_cast<BYTE>(pTxtNd->GetLevel());
        if( pHasChilds )
        {
            *pHasChilds = FALSE;
            // dann teste ob die NumSection noch weitere UnterEbenen hat:
            // zuerst ueber alle TextNodes und falls da nichts gefunden
            // wurde, ueber die Formate und deren GetInfo bis zu den Nodes

            BYTE nLvl = GetRealLevel(nLevel);
            if( nLvl + 1 < MAXLEVEL )
            {
                const String& rRule = pRule->GetName();
                SwModify* pMod;
                const SfxPoolItem* pItem;
                USHORT i, nMaxItems = GetDoc()->GetAttrPool().GetItemCount( RES_PARATR_NUMRULE);
                for( i = 0; i < nMaxItems; ++i )
                    if( 0 != (pItem = GetDoc()->GetAttrPool().GetItem( RES_PARATR_NUMRULE, i ) ) &&
                        0 != ( pMod = (SwModify*)((SwNumRuleItem*)pItem)->GetDefinedIn()) &&
                        ((SwNumRuleItem*)pItem)->GetValue().Len() &&
                        ((SwNumRuleItem*)pItem)->GetValue() == rRule &&
                        pMod->IsA( TYPE( SwTxtNode )) &&
                        ((SwTxtNode*)pMod)->GetNodes().IsDocNodes() &&
                        nLvl < ((SwTxtNode*)pMod)->GetLevel() )
                    {
                        *pHasChilds = TRUE;
                        break;
                    }

                if( !*pHasChilds )
                {
                    SwNRuleLowerLevel aHnt( rRule, nLvl );
                    for( i = 0; i < nMaxItems; ++i )
                        if( 0 != (pItem = GetDoc()->GetAttrPool().GetItem( RES_PARATR_NUMRULE, i ) ) &&
                            0 != ( pMod = (SwModify*)((SwNumRuleItem*)pItem)->GetDefinedIn()) &&
                            ((SwNumRuleItem*)pItem)->GetValue().Len() &&
                            ((SwNumRuleItem*)pItem)->GetValue() == rRule &&
                            pMod->IsA( TYPE( SwFmt )) &&
                            !pMod->GetInfo( aHnt ))
                        {
                            *pHasChilds = TRUE;
                            break;
                        }
                }

            }
        }
    }

    return nLevel;
}

const SwNumRule* SwEditShell::GetCurNumRule() const
{
    return GetDoc()->GetCurrNumRule( *GetCrsr()->GetPoint() );
}

void SwEditShell::SetCurNumRule( const SwNumRule& rRule )
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
            GetDoc()->SetNumRule( aPam, rRule );
            GetDoc()->SetCounted( aPam, true );
          }
        GetDoc()->EndUndo( UNDO_END, NULL );
    }
    else
    {
        GetDoc()->SetNumRule( *pCrsr, rRule);
        GetDoc()->SetCounted( *pCrsr, true );
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
        bResult = pTxtNd->IsRestart() ? TRUE : FALSE;
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

USHORT SwEditShell::IsNodeNumStart() const
{
    const SwTxtNode* pTxtNd = GetCrsr()->GetNode()->GetTxtNode();
    if( pTxtNd )
        return static_cast<USHORT>(pTxtNd->GetStart());
    return FALSE;
}

/*-- 26.08.2005 14:47:17---------------------------------------------------

  -----------------------------------------------------------------------*/
const SwNumRule * SwEditShell::SearchNumRule(BOOL bForward,
                                        BOOL bNum,
                                        BOOL bOutline,
                                        int nNonEmptyAllowed)
{
    return GetDoc()->SearchNumRule(*(bForward ? GetCrsr()->End() : GetCrsr()->Start()),
                                                 bForward, bNum, bOutline, nNonEmptyAllowed);
}



