/*************************************************************************
 *
 *  $RCSfile: ednumber.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

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
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
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
        GetDoc()->StartUndo( UNDO_START );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
            bRet = bRet && GetDoc()->NoNum( aRangeArr.SetPam( n, aPam ));
        GetDoc()->EndUndo( UNDO_END );
    }
    else
        bRet = GetDoc()->NoNum( *pCrsr );

    EndAllAction();
    return bRet;
}
// Loeschen, Splitten der Aufzaehlungsliste


BOOL SwEditShell::DelNumRules()
{
    BOOL bRet = TRUE;
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         // Mehrfachselektion ?
    {
        GetDoc()->StartUndo( UNDO_START );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
            bRet = bRet && GetDoc()->DelNumRules( aRangeArr.SetPam( n, aPam ) );
        GetDoc()->EndUndo( UNDO_END );
    }
    else
        bRet = GetDoc()->DelNumRules( *pCrsr );

    // rufe das AttrChangeNotify auf der UI-Seite. Sollte eigentlich
    // ueberfluessig sein, aber VB hatte darueber eine Bugrep.
    CallChgLnk();

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
        GetDoc()->StartUndo( UNDO_START );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
            bRet = bRet && GetDoc()->NumUpDown( aRangeArr.SetPam( n, aPam ), bDown );
        GetDoc()->EndUndo( UNDO_END );
    }
    GetDoc()->SetModified();
    EndAllAction();
    return bRet;
}



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
                        ( pNd->IsEndNode() && pNd->FindStartNode()->IsSectionNode())))
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
                    while( nIdx < GetDoc()->GetNodes().Count()-1 && (
                        ( pNd = GetDoc()->GetNodes()[ nIdx ])->IsSectionNode() ||
                        ( pNd->IsEndNode() && pNd->FindStartNode()->IsSectionNode()) ||
                        ( pNd->IsTxtNode() && pOrig == ((SwTxtNode*)pNd)->GetNumRule() &&
                            ((SwTxtNode*)pNd)->GetNum() &&
                            (((SwTxtNode*)pNd)->GetNum()->GetLevel() & ~NO_NUMLEVEL) > nUpperLevel )) )
                        ++nIdx;
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

// Abfrage von Oultine Informationen:


USHORT SwEditShell::GetOutlineCnt() const
{
    return GetDoc()->GetNodes().GetOutLineNds().Count();
}


BYTE SwEditShell::GetOutlineLevel( USHORT nIdx ) const
{
    const SwNodes& rNds = GetDoc()->GetNodes();
    const SwTxtFmtColl* pColl = rNds.GetOutLineNds()[ nIdx ]->GetTxtNode()->GetTxtColl();
    return pColl->GetOutlineLevel();
}


String SwEditShell::GetOutlineText( USHORT nIdx, BOOL bWithNum ) const
{
    const SwNodes& rNds = GetDoc()->GetNodes();
    return rNds.GetOutLineNds()[ nIdx ]->GetTxtNode()->GetExpandTxt( 0, STRING_LEN, bWithNum );
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
        GetDoc()->StartUndo( UNDO_START );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
            bRet = bRet && GetDoc()->OutlineUpDown(
                                    aRangeArr.SetPam( n, aPam ), nOffset );
        GetDoc()->EndUndo( UNDO_END );
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
        BYTE nLvl;
        if( !rOutlNd.Seek_Entry( pNd, &nPos ) && nPos )
            --nPos;

        for( ; nPos < rOutlNd.Count(); ++nPos )
        {
            SwNodePtr pNd = rOutlNd[ nPos ];
            BYTE nTmpLvl = GetRealLevel( pNd->GetTxtNode()->
                                    GetTxtColl()->GetOutlineLevel() );
            if( bFirst )
            {
                nLvl = nTmpLvl;
                bFirst = FALSE;
            }
            else if( nLvl >= nTmpLvl )
                break;

            if( pNd->IsProtect() )
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

BOOL SwEditShell::IsOutlineMovable( USHORT nIdx ) const
{
    const SwNodes& rNds = GetDoc()->GetNodes();
    const SwNode* pNd = rNds.GetOutLineNds()[ nIdx ];
    return pNd->GetIndex() >= rNds.GetEndOfExtras().GetIndex() &&
            !pNd->FindTableNode() && !pNd->IsProtect();
}


BOOL SwEditShell::NumOrNoNum( BOOL bNumOn, BOOL bChkStart, BOOL bOutline )
{
    BOOL bRet = FALSE;
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() == pCrsr && !pCrsr->HasMark() &&
        ( !bChkStart || !pCrsr->GetPoint()->nContent.GetIndex()) )
    {
        StartAllAction();       // Klammern fuers Updaten !!
        bRet = GetDoc()->NumOrNoNum( pCrsr->GetPoint()->nNode, bNumOn, bOutline );
        EndAllAction();
    }
    return bRet;
}


BOOL SwEditShell::IsNoNum( BOOL bChkStart, BOOL bOutline ) const
{
    // ein Backspace im Absatz ohne Nummer wird zum Delete
    const SwTxtNode* pTxtNd;
    const SwNodeNum* pNum;
    SwPaM* pCrsr = GetCrsr();

    return pCrsr->GetNext() == pCrsr && !pCrsr->HasMark() &&
        (!bChkStart || !pCrsr->GetPoint()->nContent.GetIndex()) &&
        0 != ( pTxtNd = pCrsr->GetNode()->GetTxtNode()) &&
        ( bOutline ? (NO_NUMBERING != pTxtNd->GetTxtColl()->GetOutlineLevel() &&
                      0 != (pNum = pTxtNd->GetOutlineNum() ))
                   : (pTxtNd->GetNumRule() &&
                      0 != (pNum = pTxtNd->GetNum() ) )) &&
        0 != ( pNum->GetLevel() & NO_NUMLEVEL );
}


const SwNodeNum* SwEditShell::GetOutlineNum( USHORT nIdx ) const
{
    const SwNodes& rNds = GetDoc()->GetNodes();
    return rNds.GetOutLineNds()[ nIdx ]->GetTxtNode()->GetOutlineNum();
}


BYTE SwEditShell::GetNumLevel( BOOL* pHasChilds ) const
{
    // gebe die akt. Ebene zurueck, auf der sich der Point vom Cursor befindet
    BYTE nLevel = NO_NUMBERING;

    SwPaM* pCrsr = GetCrsr();
    const SwTxtNode* pTxtNd = pCrsr->GetNode()->GetTxtNode();
    const SwNumRule* pRule = pTxtNd->GetNumRule();
    if( pRule && pTxtNd->GetNum() )
    {
        nLevel = pTxtNd->GetNum()->GetLevel();
        if( pHasChilds )
        {
            *pHasChilds = FALSE;
            // dann teste ob die NumSection noch weitere UnterEbenen hat:
            // zuerst ueber alle TextNodes und falls da nichts gefunden
            // wurde, ueber die Formate und deren GetInfo bis zu den Nodes

            BYTE nLvl = nLevel & ~NO_NUMLEVEL;
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
                        ((SwTxtNode*)pMod)->GetNum() &&
                        nLvl < ( ((SwTxtNode*)pMod)->GetNum()->GetLevel() & ~NO_NUMLEVEL) )
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

BOOL SwEditShell::IsUsed( const SwNumRule& rRule ) const
{
    return GetDoc()->IsUsed( rRule );
}

SwNumRule* SwEditShell::GetNumRuleFromPool( USHORT nId )
{
    return GetDoc()->GetNumRuleFromPool( nId );
}

void SwEditShell::SetCurNumRule( const SwNumRule& rRule )
{
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         // Mehrfachselektion ?
    {
        GetDoc()->StartUndo( UNDO_START );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
            GetDoc()->SetNumRule( aRangeArr.SetPam( n, aPam ), rRule );
        GetDoc()->EndUndo( UNDO_END );
    }
    else
        GetDoc()->SetNumRule( *pCrsr, rRule );

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
        GetDoc()->StartUndo( UNDO_START );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
            GetDoc()->SetNumRuleStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), bFlag );
        GetDoc()->EndUndo( UNDO_END );
    }
    else
        GetDoc()->SetNumRuleStart( *pCrsr->GetPoint(), bFlag );

    EndAllAction();
}

BOOL SwEditShell::IsNumRuleStart() const
{
    const SwTxtNode* pTxtNd = GetCrsr()->GetNode()->GetTxtNode();
    if( pTxtNd && pTxtNd->GetNum() && pTxtNd->GetNumRule() )
        return pTxtNd->GetNum()->IsStart();
    return FALSE;
}

void SwEditShell::SetNodeNumStart( USHORT nStt )
{
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         // Mehrfachselektion ?
    {
        GetDoc()->StartUndo( UNDO_START );
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
            GetDoc()->SetNodeNumStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), nStt );
        GetDoc()->EndUndo( UNDO_END );
    }
    else
        GetDoc()->SetNodeNumStart( *pCrsr->GetPoint(), nStt );

    EndAllAction();
}

USHORT SwEditShell::IsNodeNumStart() const
{
    const SwTxtNode* pTxtNd = GetCrsr()->GetNode()->GetTxtNode();
    if( pTxtNd && pTxtNd->GetNum() && pTxtNd->GetNumRule() )
        return pTxtNd->GetNum()->GetSetValue();
    return USHRT_MAX;
}




