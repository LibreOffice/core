/*************************************************************************
 *
 *  $RCSfile: docnum.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2001-02-23 12:45:12 $
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

#ifndef _DATE_HXX //autogen
#include <tools/date.hxx>
#endif
#ifndef _TIME_HXX //autogen
#include <tools/time.hxx>
#endif
#ifndef _TOOLS_RESID_HXX //autogen
#include <tools/resid.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif

#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _FTNIDX_HXX //autogen
#include <ftnidx.hxx>
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
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>       // pTOXBaseRing
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _PAMTYP_HXX
#include <pamtyp.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif


inline BYTE GetUpperLvlChg( BYTE nCurLvl, BYTE nLevel, USHORT nMask )
{
    if( 1 < nLevel )
    {
        if( nCurLvl + 1 >= nLevel )
            nCurLvl -= nLevel - 1;
        else
            nCurLvl = 0;
    }
    return (nMask - 1) & ~(( 1 << nCurLvl ) - 1);
}

void SwDoc::SetOutlineNumRule( const SwNumRule& rRule )
{
    USHORT nChkLevel = 0, nChgFmtLevel = 0;
    if( pOutlineRule )
    {
        USHORT nMask = 1;
        for( BYTE n = 0; n < MAXLEVEL; ++n, nMask <<= 1 )
        {
            const SwNumFmt& rOld = pOutlineRule->Get( n ),
                          & rNew = rRule.Get( n );
            if( rOld != rNew )
            {
                nChgFmtLevel |= nMask;
                if( rOld.GetAbsLSpace() != rNew.GetAbsLSpace() ||
                    rOld.GetFirstLineOffset() != rNew.GetFirstLineOffset() )
                    nChkLevel |= nMask;
            }
            else if( SVX_NUM_NUMBER_NONE > rNew.GetNumberingType() && 1 < rNew.GetIncludeUpperLevels() &&
                    0 != (nChgFmtLevel & GetUpperLvlChg( n,
                                            rNew.GetIncludeUpperLevels(), nMask )) )
                nChgFmtLevel |= nMask;
        }
        (*pOutlineRule) = rRule;
    }
    else
    {
        nChgFmtLevel = nChkLevel = 0xffff;
        pOutlineRule = new SwNumRule( rRule );
    }
    pOutlineRule->SetRuleType( OUTLINE_RULE );
    pOutlineRule->SetName( String::CreateFromAscii(
                                        SwNumRule::GetOutlineRuleName() ));

    // teste ob die evt. gesetzen CharFormate in diesem Document
    // definiert sind
    pOutlineRule->CheckCharFmts( this );

    // losche aus dem Array alle Nodes, die ohne Outline Nummerierung sind
    SwOutlineNodes& rArr = (SwOutlineNodes&)GetNodes().GetOutLineNds();
    {
        SwNodeNum aNoNum( NO_NUMBERING );
        for( USHORT n = 0; n < rArr.Count(); ++n )
        {
            SwTxtNode* pTxtNd = rArr[n]->GetTxtNode();
            if( pTxtNd && NO_NUMBERING == pTxtNd->GetTxtColl()->GetOutlineLevel() )
            {
                pTxtNd->UpdateOutlineNum( aNoNum );
                rArr.Remove( n-- );
            }
        }
    }

    // suche alle Nodes, die neu aufgenommen werden muessen !!
    // (eigentlich koennte das auch per Modify am die Nodes propagiert
    // werden !! )
    ULONG nStt = GetNodes().GetEndOfContent().StartOfSectionIndex();
    for( USHORT n = 0; n < pTxtFmtCollTbl->Count(); ++n )
    {
        SwTxtFmtColl* pColl = (*pTxtFmtCollTbl)[ n ];
        BYTE nLevel = pColl->GetOutlineLevel();
        if( NO_NUMBERING != nLevel )
        {
#ifndef NUM_RELSPACE
            // JP 08.07.98: Einzuege aus der Outline uebernehmen.
            //              ??Aber nur wenn sie veraendert wurden??
            if( ( nLevel = GetRealLevel( nLevel )) < MAXLEVEL
                /*&& 0 != (nChkLevel & (1 << nLevel ))*/ )
            {
                SvxLRSpaceItem aLR( (SvxLRSpaceItem&)pColl->GetAttr( RES_LR_SPACE ) );
                const SwNumFmt& rNFmt = pOutlineRule->Get( nLevel );

                // ohne Nummer immer ohne FirstLineOffset!!!!
                short nFOfst;
                if( pColl->GetOutlineLevel() & NO_NUMLEVEL )
                    nFOfst = 0;
                else
                    nFOfst = rNFmt.GetFirstLineOffset();

                if( aLR.GetTxtLeft() != rNFmt.GetAbsLSpace() ||
                    aLR.GetTxtFirstLineOfst() != nFOfst )
                {
                    aLR.SetTxtFirstLineOfstValue( nFOfst );
                    aLR.SetTxtLeft( rNFmt.GetAbsLSpace() );

                    pColl->SetAttr( aLR );
                }
            }
#endif
            SwClientIter aIter( *pColl );
            for( SwTxtNode* pNd = (SwTxtNode*)aIter.First( TYPE( SwTxtNode ));
                    pNd; pNd = (SwTxtNode*)aIter.Next() )
                if( pNd->GetNodes().IsDocNodes() && nStt < pNd->GetIndex() )
                    rArr.Insert( pNd );
        }
    }

    for( n = 0; n < rArr.Count(); ++n )
    {
        SwTxtNode* pNd = rArr[ n ]->GetTxtNode();
        ASSERT( pNd, "was ist das fuer ein Node?" );
        if( ( 1 << (pNd->GetTxtColl()->GetOutlineLevel() & ~NO_NUMLEVEL )
            & nChgFmtLevel ))
            pNd->NumRuleChgd();
    }
    GetNodes().UpdateOutlineNodes();        // update der Nummern

    // gibt es Fussnoten && gilt Kapitelweises Nummerieren, dann updaten
    if( GetFtnIdxs().Count() && FTNNUM_CHAPTER == GetFtnInfo().eNum )
        GetFtnIdxs().UpdateAllFtn();

    UpdateExpFlds();

    SetModified();
}



    // Hoch-/Runterstufen
BOOL SwDoc::OutlineUpDown( const SwPaM& rPam, short nOffset )
{
    if( !GetNodes().GetOutLineNds().Count() || !nOffset )
        return FALSE;

    // den Bereich feststellen
    const SwOutlineNodes& rOutlNds = GetNodes().GetOutLineNds();
    const SwNodePtr pSttNd = (SwNodePtr)&rPam.Start()->nNode.GetNode();
    const SwNodePtr pEndNd = (SwNodePtr)&rPam.End()->nNode.GetNode();
    USHORT nSttPos, nEndPos;

    if( !rOutlNds.Seek_Entry( pSttNd, &nSttPos ) &&
        !nSttPos-- )
        // wir stehen in keiner "Outline-Section"
        return FALSE;

    if( rOutlNds.Seek_Entry( pEndNd, &nEndPos ) )
        ++nEndPos;


    // jetzt haben wir unseren Bereich im OutlineNodes-Array
    // dann prufe ersmal, ob nicht unterebenen aufgehoben werden
    // (Stufung ueber die Grenzen)
    register USHORT n;
    if( 0 <= nOffset )          // nach unten
    {
        for( n = nSttPos; n < nEndPos; ++n )
            if( rOutlNds[ n ]->GetTxtNode()->GetTxtColl()->
                GetOutlineLevel() + nOffset >= MAXLEVEL )
                return FALSE;
    }
    else
        for( n = nSttPos; n < nEndPos; ++n )
            if( rOutlNds[ n ]->GetTxtNode()->GetTxtColl()->
                GetOutlineLevel() < -nOffset )
                return FALSE;

    // so, dann koennen wir:
    // 1. Vorlagen-Array anlegen
    SwTxtFmtColl* aCollArr[ MAXLEVEL ];
    memset( aCollArr, 0, sizeof( SwTxtFmtColl* ) * MAXLEVEL );

    for( n = 0; n < pTxtFmtCollTbl->Count(); ++n )
    {
        BYTE nLevel = (*pTxtFmtCollTbl)[ n ]->GetOutlineLevel();
        if( nLevel < MAXLEVEL )
            aCollArr[ nLevel ] = (*pTxtFmtCollTbl)[ n ];
    }
    for( n = 0; n < MAXLEVEL; ++n )
        if( !aCollArr[ n ] )
            aCollArr[ n ] = GetTxtCollFromPool( RES_POOLCOLL_HEADLINE1 + n );

    if( DoesUndo() )
    {
        ClearRedo();
        AppendUndo( new SwUndoOutlineLeftRight( rPam, nOffset ) );
    }

    // 2. allen Nodes die neue Vorlage zuweisen
    for( n = nSttPos; n < nEndPos; ++n )
    {
        SwTxtNode* pTxtNd = rOutlNds[ n ]->GetTxtNode();
        SwTxtFmtColl* pColl = pTxtNd->GetTxtColl();
        pColl = aCollArr[ pColl->GetOutlineLevel() + nOffset ];
        pColl = (SwTxtFmtColl*)pTxtNd->ChgFmtColl( pColl );
        // Undo ???
    }

    SetModified();
    return TRUE;
}



    // Hoch-/Runter - Verschieben !
BOOL SwDoc::MoveOutlinePara( const SwPaM& rPam, short nOffset )
{
    // kein Verschiebung in den Sonderbereichen
    const SwPosition& rStt = *rPam.Start(),
                    & rEnd = &rStt == rPam.GetPoint() ? *rPam.GetMark()
                                                      : *rPam.GetPoint();
    if( !GetNodes().GetOutLineNds().Count() || !nOffset ||
        rStt.nNode.GetIndex() < aNodes.GetEndOfExtras().GetIndex() ||
        rEnd.nNode.GetIndex() < aNodes.GetEndOfExtras().GetIndex() )
        return FALSE;

    USHORT nAktPos = 0;
    SwNodeIndex aSttRg( rStt.nNode ), aEndRg( rEnd.nNode );

    // nach vorne  -> vom Start erfragen
    // nach hinten -> vom End erfragen
    if( 0 > nOffset )       // nach vorne
    {
        SwNode* pSrch = &aSttRg.GetNode();
        if( !GetNodes().GetOutLineNds().Seek_Entry( pSrch, &nAktPos ) )
        {
            if( !nAktPos-- )
                // wir stehen in keiner "Outline-Section"
                return FALSE;

            // dann sollten wir den Start korrigieren !!!
            aSttRg = *GetNodes().GetOutLineNds()[ nAktPos ];
        }

        pSrch = &aEndRg.GetNode();
        USHORT nTmp;
        if( GetNodes().GetOutLineNds().Seek_Entry( pSrch, &nTmp ))
            // wenn die Selection in einem OutlineNode endet, diesen noch
            // mitnehmen
            aEndRg++;
        else
        {
            aEndRg = nTmp < GetNodes().GetOutLineNds().Count()
                            ? *GetNodes().GetOutLineNds()[ nTmp ]
                            : GetNodes().GetEndOfContent();
        }

        if( aEndRg == aSttRg )
            // kein Bereich, dann machen wir uns einen
            aEndRg++;
    }
    // nach hinten
    else
    {
        SwNode* pSrch = &aEndRg.GetNode();
        if( GetNodes().GetOutLineNds().Seek_Entry( pSrch, &nAktPos ) )
            ++nAktPos;

        aEndRg = nAktPos < GetNodes().GetOutLineNds().Count()
                        ? *GetNodes().GetOutLineNds()[ nAktPos ]
                        : GetNodes().GetEndOfContent();

        pSrch = &aSttRg.GetNode();
        USHORT nTmp;
        if( !GetNodes().GetOutLineNds().Seek_Entry( pSrch, &nTmp ) )
        {
            if( --nTmp )
                aSttRg = *GetNodes().GetOutLineNds()[ nTmp ];
            else
                aSttRg = *GetNodes().GetEndOfContent().StartOfSectionNode();
        }
    }
    // neue Position errechnen:
    const SwNode* pNd;
    if( nOffset < 0 && nAktPos < USHORT(-nOffset) )
        pNd = GetNodes().GetEndOfContent().StartOfSectionNode();
    else if( nAktPos + nOffset >= GetNodes().GetOutLineNds().Count() )
        pNd = &GetNodes().GetEndOfContent();
    else
        pNd = GetNodes().GetOutLineNds()[ nAktPos + nOffset ];

    ULONG nNewPos = pNd->GetIndex();

    ASSERT( aSttRg.GetIndex() > nNewPos || nNewPos >= aEndRg.GetIndex(),
                "Position liegt im MoveBereich" );

    // wurde ein Position in den Sonderbereichen errechnet, dann
    // setze die Position auf den Dokumentanfang.
    // Sollten da Bereiche oder Tabellen stehen, so werden sie nach
    // hinten verschoben.
    nNewPos = Max( nNewPos, aNodes.GetEndOfExtras().GetIndex() + 2 );

    long nOffs = nNewPos - ( 0 < nOffset ? aEndRg.GetIndex() : aSttRg.GetIndex());
    SwPaM aPam( aSttRg, aEndRg, 0, -1 );
    return MoveParagraph( aPam, nOffs, TRUE );
}


USHORT lcl_FindOutlineName( const SwNodes& rNds, const String& rName,
                            BOOL bExact )
{
    USHORT nSavePos = USHRT_MAX;
    const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
    String sName( INetURLObject::createFragment( rName ) );
    for( USHORT n = 0; n < rOutlNds.Count(); ++n )
    {
        SwTxtNode* pTxtNd = rOutlNds[ n ]->GetTxtNode();
        String sTxt( INetURLObject::createFragment( pTxtNd->GetExpandTxt() ));
        if( sTxt.Equals( rName ) )
        {
            // "exact" gefunden, setze Pos auf den Node
            nSavePos = n;
            break;
        }
        else if( !bExact && USHRT_MAX == nSavePos &&
                    COMPARE_EQUAL == sTxt.CompareTo( rName, rName.Len()) )
        {
            // dann vielleicht nur den den 1.Teil vom Text gefunden
            nSavePos = n;
        }
    }

    return nSavePos;
}



USHORT lcl_FindOutlineNum( const SwNodes& rNds, String& rName )
{
    // Gueltig Nummern sind (immer nur Offsets!!!):
    //  ([Nummer]+\.)+  (als regulaerer Ausdruck!)
    //  (Nummer gefolgt von Punkt, zum 5 Wiederholungen)
    //  also: "1.1.", "1.", "1.1.1."
    xub_StrLen nPos = 0;
    String sNum = rName.GetToken( 0, '.', nPos );
    if( STRING_NOTFOUND == nPos )
        return USHRT_MAX;           // ungueltige Nummer!!!

    USHORT nLevelVal[ MAXLEVEL ];       // Nummern aller Levels
    memset( nLevelVal, 0, MAXLEVEL * sizeof( nLevelVal[0] ));
    BYTE nLevel = 0;
    String sName( rName );

    while( STRING_NOTFOUND != nPos )
    {
        USHORT nVal = 0;
        sal_Unicode c;
        for( USHORT n = 0; n < sNum.Len(); ++n )
            if( '0' <= ( c = sNum.GetChar( n )) && c <= '9' )
            {
                nVal *= 10;  nVal += c - '0';
            }
            else if( nLevel )
                break;                      // "fast" gueltige Nummer
            else
                return USHRT_MAX;           // ungueltige Nummer!!!

        if( MAXLEVEL > nLevel )
            nLevelVal[ nLevel++ ] = nVal;

        sName.Erase( 0, nPos );
        nPos = 0;
        sNum = sName.GetToken( 0, '.', nPos );
    }

    rName = sName;      // das ist der nachfolgende Text.

    // alle Levels gelesen, dann suche mal im Document nach dieser
    // Gliederung:
    const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
    // OS: ohne OutlineNodes lohnt die Suche nicht
    // und man spart sich einen Absturz #42958#
    if(!rOutlNds.Count())
        return USHRT_MAX;
    SwTxtNode* pNd;
    nPos = 0;
    BOOL bNextPos = FALSE;
    for( BYTE n = 0; n < nLevel; ++n )
    {
        USHORT nOff = nLevelVal[ n ];
        if( nOff )
        {
            USHORT nLastPos = nPos;
            if( bNextPos )
                ++nPos;
            bNextPos = FALSE;
            for( ; nPos < rOutlNds.Count(); ++nPos )
            {
                pNd = rOutlNds[ nPos ]->GetTxtNode();
                BYTE nLvl = pNd->GetTxtColl()->GetOutlineLevel();
                if( nLvl == n )
                {
                    nLastPos = nPos;
                    if( !--nOff )
                    {
                        bNextPos = TRUE; // auf jedenfall mit dem nachsten weiter
                        break;
                    }
                }
                else if( nLvl < n )     // nicht ueber den Level hinaus
                {
                    nPos = nLastPos;
                    break;
                }
            }
            if( nPos >= rOutlNds.Count() )
            {
                if( !n )                // auf oberster Ebene muss die
                    return USHRT_MAX;   // Nummer gefunden werden
                nPos = nLastPos;
            }
        }
        else if( nPos+1 < rOutlNds.Count() )
        {
            pNd = rOutlNds[ nPos+1 ]->GetTxtNode();
            if( n < pNd->GetTxtColl()->GetOutlineLevel() )
                ++nPos, bNextPos = FALSE;
        }
    }

    // jetzt sollte im nPos die richtige Position fuer den OutlineLevel
    // stehen:
    return nPos;
}

    // zu diesem Gliederungspunkt


    // JP 13.06.96:
    // im Namen kann eine Nummer oder/und der Text stehen.
    // zuerst wird ueber die Nummer versucht den richtigen Eintrag zu finden.
    // Gibt es diesen, dann wird ueber den Text verglichen, od es der
    // gewuenschte ist. Ist das nicht der Fall, wird noch mal nur ueber den
    // Text gesucht. Wird dieser gefunden ist es der Eintrag. Ansonsten der,
    // der ueber die Nummer gefunden wurde.
    // Ist keine Nummer angegeben, dann nur den Text suchen.

BOOL SwDoc::GotoOutline( SwPosition& rPos, const String& rName ) const
{
    if( rName.Len() )
    {
        const SwOutlineNodes& rOutlNds = GetNodes().GetOutLineNds();

        // 1. Schritt: ueber die Nummer:
        String sName( rName );
        USHORT nFndPos = ::lcl_FindOutlineNum( GetNodes(), sName );
        if( USHRT_MAX != nFndPos )
        {
            SwTxtNode* pNd = rOutlNds[ nFndPos ]->GetTxtNode();
            String sTxt( pNd->GetExpandTxt() );
            if( sTxt.Equals( sName ) )
            {
                USHORT nTmp = ::lcl_FindOutlineName( GetNodes(), sName, TRUE );
                if( USHRT_MAX != nTmp )             // ueber den Namen gefunden
                {
                    nFndPos = nTmp;
                    pNd = rOutlNds[ nFndPos ]->GetTxtNode();
                }
            }
            rPos.nNode = *pNd;
            rPos.nContent.Assign( pNd, 0 );
            return TRUE;
        }

        nFndPos = ::lcl_FindOutlineName( GetNodes(), rName, FALSE );
        if( USHRT_MAX != nFndPos )
        {
            SwTxtNode* pNd = rOutlNds[ nFndPos ]->GetTxtNode();
            rPos.nNode = *pNd;
            rPos.nContent.Assign( pNd, 0 );
            return TRUE;
        }
    }
    return FALSE;
}

void SwDoc::SetOutlineLSpace( BYTE nLevel, short nFirstLnOfst, USHORT nLSpace )
{
    if( MAXLEVEL >= nLevel )
    {
        const SwNumFmt& rNFmt = pOutlineRule->Get( nLevel );
        if( nLSpace != rNFmt.GetAbsLSpace() ||
            nFirstLnOfst != rNFmt.GetFirstLineOffset() )
        {
            SwNumFmt aFmt( rNFmt );
            aFmt.SetAbsLSpace( nLSpace );
            aFmt.SetFirstLineOffset( nFirstLnOfst );
            pOutlineRule->Set( nLevel, aFmt );
        }
    }
}

/*  */

// --- Nummerierung -----------------------------------------

void SwNumRuleInfo::MakeList( SwDoc& rDoc )
{
    SwModify* pMod;
    const SfxPoolItem* pItem;
    USHORT i, nMaxItems = rDoc.GetAttrPool().GetItemCount( RES_PARATR_NUMRULE);
    for( i = 0; i < nMaxItems; ++i )
        if( 0 != (pItem = rDoc.GetAttrPool().GetItem( RES_PARATR_NUMRULE, i ) ) &&
            0 != ( pMod = (SwModify*)((SwNumRuleItem*)pItem)->GetDefinedIn()) &&
            ((SwNumRuleItem*)pItem)->GetValue().Len() &&
            ((SwNumRuleItem*)pItem)->GetValue() == rName )
        {
            if( pMod->IsA( TYPE( SwFmt )) )
                pMod->GetInfo( *this );
            else if( ((SwTxtNode*)pMod)->GetNodes().IsDocNodes() )
                AddNode( *(SwTxtNode*)pMod );
        }
}


void lcl_ChgNumRule( SwDoc& rDoc, const SwNumRule& rRule, SwHistory* pHist,
                        SwNumRuleInfo* pRuleInfo = 0 )
{
    SwNumRule* pOld = rDoc.FindNumRulePtr( rRule.GetName() );
    ASSERT( pOld, "ohne die alte NumRule geht gar nichts" );

    USHORT nChkLevel = 0, nChgFmtLevel = 0, nMask = 1;

    for( BYTE n = 0; n < MAXLEVEL; ++n, nMask <<= 1 )
    {
        const SwNumFmt& rOldFmt = pOld->Get( n ),
                      & rNewFmt = rRule.Get( n );

        if( rOldFmt != rNewFmt )
        {
            nChgFmtLevel |= nMask;
            if( rOldFmt.GetAbsLSpace() != rNewFmt.GetAbsLSpace() ||
                rOldFmt.GetFirstLineOffset() != rNewFmt.GetFirstLineOffset() )
                nChkLevel |= nMask;
        }
        else if( SVX_NUM_NUMBER_NONE > rNewFmt.GetNumberingType() && 1 < rNewFmt.GetIncludeUpperLevels() &&
                0 != (nChgFmtLevel & GetUpperLvlChg( n, rNewFmt.GetIncludeUpperLevels(),nMask )) )
            nChgFmtLevel |= nMask;
    }

    if( !nChgFmtLevel )         // es wurde nichts veraendert?
    {
        pOld->CheckCharFmts( &rDoc );
        pOld->SetContinusNum( rRule.IsContinusNum() );
        return ;
    }

    SwNumRuleInfo* pUpd;
    if( !pRuleInfo )
    {
        pUpd = new SwNumRuleInfo( rRule.GetName() );
        pUpd->MakeList( rDoc );
    }
    else
        pUpd = pRuleInfo;

    BYTE nLvl;
    for( ULONG nFirst = 0, nLast = pUpd->GetList().Count();
        nFirst < nLast; ++nFirst )
    {
        SwTxtNode* pTxtNd = pUpd->GetList().GetObject( nFirst );
        if( pTxtNd->GetNum() && ( nLvl = (~NO_NUMLEVEL &
            pTxtNd->GetNum()->GetLevel() ) ) < MAXLEVEL )
        {
            if( nChgFmtLevel & ( 1 << nLvl ))
            {
                pTxtNd->NumRuleChgd();

#ifndef NUM_RELSPACE
                if( nChkLevel && (nChkLevel & ( 1 << nLvl )) &&
                    pOld->IsRuleLSpace( *pTxtNd ) )
                    pTxtNd->SetNumLSpace( TRUE );

                if( pHist )
                {
                    const SfxPoolItem& rItem =
                                pTxtNd->SwCntntNode::GetAttr( RES_LR_SPACE );
                    pHist->Add( &rItem, &rItem, pTxtNd->GetIndex() );
                }
#endif
            }
        }
    }

    for( n = 0; n < MAXLEVEL; ++n )
        if( nChgFmtLevel & ( 1 << n ))
            pOld->Set( n, rRule.GetNumFmt( n ));

    pOld->CheckCharFmts( &rDoc );
    pOld->SetInvalidRule( TRUE );
    pOld->SetContinusNum( rRule.IsContinusNum() );

    if( !pRuleInfo )
        delete pUpd;
}

void SwDoc::SetNumRule( const SwPaM& rPam, const SwNumRule& rRule,
                        BOOL bSetAbsLSpace )
{
    SwUndoInsNum* pUndo;
    if( DoesUndo() )
    {
        ClearRedo();
        StartUndo( UNDO_START );        // Klammerung fuer die Attribute!
        AppendUndo( pUndo = new SwUndoInsNum( rPam, rRule ) );
    }
    else
        pUndo = 0;

    ULONG nPamPos = rPam.Start()->nNode.GetIndex();
    BOOL bSetItem = TRUE;
    SwNumRule* pNew = FindNumRulePtr( rRule.GetName() );
    if( !pNew )
        pNew = (*pNumRuleTbl)[ MakeNumRule( rRule.GetName(), &rRule ) ];
    else if( rRule.IsAutoRule() && !(*pNew == rRule) )
    {
        // die Rule existiert schon, wurde aber veraendert. Dann muss hier
        // entschieden werden, in wie weit eine neue erzeugt wird oder
        // die aktuelle geupdatet wird.

        // neue erzeugen wenn:
        //  - der Pam einen Bereich hat
        //  - Nodes existieren, die mit der Rule neu Starten

        if( rPam.HasMark() )
            pNew = (*pNumRuleTbl)[ MakeNumRule( rRule.GetName(), &rRule ) ];
        else
        {
            SwNumRuleInfo aUpd( rRule.GetName() );
            aUpd.MakeList( *this );

            // Position suchen und bestimme ob ein Node davor oder dahinter
            // einen Start erzwingt
            SwTxtNode* pTxtNd;
            ULONG nFndPos, nFirst, nLast;

            if( TABLE_ENTRY_NOTFOUND != aUpd.GetList().SearchKey( nPamPos, &nFndPos ))
                ++nFndPos;

            for( nLast = nFndPos; nLast < aUpd.GetList().Count(); ++nLast )
                if( ( pTxtNd = aUpd.GetList().GetObject( nLast ))->GetNum() &&
                    pTxtNd->GetNum()->IsStart() )
                    break;
            for( nFirst = nFndPos; nFirst; )
                if( ( pTxtNd = aUpd.GetList().GetObject( --nFirst ))->GetNum() &&
                    pTxtNd->GetNum()->IsStart() )
                    break;

            bSetItem = FALSE;
            nPamPos = ULONG_MAX;
            if( !pNew->IsAutoRule() || nFirst || nLast != aUpd.GetList().Count() )
            {
                // dann neue Numerierung ueber diesen Bereich
                // definieren und den Start am Anfang/Ende zurueck setzen
                pTxtNd = aUpd.GetList().GetObject( nFirst );
                if( pTxtNd->GetNum()->IsStart() )
                {
                    ((SwNodeNum*)pTxtNd->GetNum())->SetStart( FALSE );
                    if( pUndo )
                        pUndo->SetSttNum( pTxtNd->GetIndex() );
                }

                SwHistory* pHist = pUndo ? pUndo->GetHistory() : 0;
#ifndef NUM_RELSPACE
                if( pHist )
                {
                    // die LRSpaces muessen zuerst gesichert werden
                    for( ULONG n = nFirst; n < nLast; ++n )
                    {
                        pTxtNd = aUpd.GetList().GetObject( n );
                        SfxPoolItem* pItem = (SfxPoolItem*)
                            &pTxtNd->SwCntntNode::GetAttr( RES_LR_SPACE );
                        pHist->Add( pItem, pItem, pTxtNd->GetIndex() );
                    }
                    pUndo->SetLRSpaceEndPos();
                }
#endif
                SwRegHistory aRegH( pHist );
                SwNumRule* pOld = pNew;
                pNew = (*pNumRuleTbl)[ MakeNumRule( rRule.GetName(), &rRule ) ];

                USHORT nChgFmtLevel = 0;
                for( BYTE n = 0; n < MAXLEVEL; ++n )
                {
                    const SwNumFmt& rOldFmt = pOld->Get( n ),
                                  & rNewFmt = rRule.Get( n );

                    if( rOldFmt.GetAbsLSpace() != rNewFmt.GetAbsLSpace() ||
                        rOldFmt.GetFirstLineOffset() != rNewFmt.GetFirstLineOffset() )
                        nChgFmtLevel |= ( 1 << n );
                }

                SwNumRuleItem aRule( pNew->GetName() );
                for( ; nFirst < nLast; ++nFirst )
                {
                    pTxtNd = aUpd.GetList().GetObject( nFirst );

                    aRegH.RegisterInModify( pTxtNd, *pTxtNd );

#ifndef NUM_RELSPACE
                    BYTE nLvl = !pTxtNd->GetNum() ? NO_NUMBERING
                                    : pTxtNd->GetNum()->GetLevel() & ~NO_NUMLEVEL;
                    if( nLvl < MAXLEVEL && (nChgFmtLevel & ( 1 << nLvl )) &&
                        pOld->IsRuleLSpace( *pTxtNd ) )
                            pTxtNd->SetNumLSpace( TRUE );
#endif

                    pTxtNd->SwCntntNode::SetAttr( aRule );
                    pTxtNd->NumRuleChgd();
                }
            }
            else
            {
                // dann nur die Rule Updaten
                SwHistory* pHist = 0;
                if( pUndo )
                {
                    pUndo->SaveOldNumRule( *pNew );
                    pHist = pUndo->GetHistory();
                }
                ::lcl_ChgNumRule( *this, rRule, pHist, &aUpd );
                if( pUndo )
                    pUndo->SetLRSpaceEndPos();
            }
        }
    }

    if( bSetItem )
    {
#ifndef NUM_RELSPACE
        if( pUndo )
        {
            SwHistory* pHist = pUndo->GetHistory();
            SwCntntNode* pCNd;
            for( ULONG n = nPamPos, nEndPos = rPam.End()->nNode.GetIndex();
                    n <= nEndPos; ++n )
                if( 0 != ( pCNd = GetNodes()[ n ]->GetCntntNode() ))
                {
                    const SfxPoolItem& rItem = pCNd->GetAttr( RES_LR_SPACE );
                    pHist->Add( &rItem, &rItem, n );
                }
            pUndo->SetLRSpaceEndPos();
        }
#endif
        Insert( rPam, SwNumRuleItem( pNew->GetName() ) );
    }
    UpdateNumRule( pNew->GetName(), nPamPos );

    EndUndo( UNDO_END );

    SetModified();
}

void SwDoc::SetNumRuleStart( const SwPosition& rPos, BOOL bFlag )
{
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    const SwNumRule* pRule;
    if( pTxtNd && pTxtNd->GetNum() && 0 != ( pRule = pTxtNd->GetNumRule() )
        && bFlag != pTxtNd->GetNum()->IsStart())
    {
        if( DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoNumRuleStart( rPos, bFlag ));
        }
        SwNodeNum aNum( *pTxtNd->GetNum() );
        aNum.SetStart( bFlag );
        pTxtNd->UpdateNum( aNum );
        UpdateNumRule( pRule->GetName(),
                        bFlag ? rPos.nNode.GetIndex() : ULONG_MAX );
        SetModified();
    }
}

void SwDoc::SetNodeNumStart( const SwPosition& rPos, USHORT nStt )
{
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    const SwNumRule* pRule;
    if( pTxtNd && pTxtNd->GetNum() && 0 != ( pRule = pTxtNd->GetNumRule() )
        && nStt != pTxtNd->GetNum()->GetSetValue() )
    {
        if( DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoNumRuleStart( rPos, nStt ));
        }
        SwNodeNum aNum( *pTxtNd->GetNum() );
        aNum.SetSetValue( nStt );
        pTxtNd->UpdateNum( aNum );
        UpdateNumRule( pRule->GetName(), USHRT_MAX != nStt
                            ? rPos.nNode.GetIndex() : ULONG_MAX );
        SetModified();
    }
}

    // loeschen geht nur, wenn die Rule niemand benutzt!
BOOL SwDoc::DelNumRule( const String& rName )
{
    USHORT nPos = FindNumRule( rName );
    if( USHRT_MAX != nPos && !IsUsed( *(*pNumRuleTbl)[ nPos ] ))
    {
        pNumRuleTbl->DeleteAndDestroy( nPos );
        SetModified();
        return TRUE;
    }
    return FALSE;
}

void SwDoc::ChgNumRuleFmts( const SwNumRule& rRule )
{
    SwNumRule* pRule = FindNumRulePtr( rRule.GetName() );
    if( pRule )
    {
        SwUndoInsNum* pUndo = 0;
        SwHistory* pHistory = 0;
        if( DoesUndo() && pRule->IsAutoRule() )
        {
            ClearRedo();
            pUndo = new SwUndoInsNum( *pRule, rRule );
            pHistory = pUndo->GetHistory();
            AppendUndo( pUndo );
        }
        ::lcl_ChgNumRule( *this, rRule, pHistory );

        if( pUndo )
            pUndo->SetLRSpaceEndPos();

        SetModified();
    }
}

void SwDoc::StopNumRuleAnimations( OutputDevice* pOut )
{
    for( USHORT n = GetNumRuleTbl().Count(); n; )
    {
        SwNumRuleInfo aUpd( GetNumRuleTbl()[ --n ]->GetName() );
        aUpd.MakeList( *this );

        for( ULONG nFirst = 0, nLast = aUpd.GetList().Count();
                nFirst < nLast; ++nFirst )
        {
            SwTxtNode* pTNd = aUpd.GetList().GetObject( nFirst );

            SwClientIter aIter( *pTNd );
            for( SwFrm* pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) );
                    pFrm; pFrm = (SwFrm*)aIter.Next() )
                if( ((SwTxtFrm*)pFrm)->HasAnimation() )
                    ((SwTxtFrm*)pFrm)->StopAnimation( pOut );
        }
    }
}

BOOL SwDoc::ReplaceNumRule( const SwPosition& rPos,
                            const String& rOldRule, const String& rNewRule )
{
    BOOL bRet = FALSE;
    SwNumRule *pOldRule = FindNumRulePtr( rOldRule ),
              *pNewRule = FindNumRulePtr( rNewRule );
    if( pOldRule && pNewRule && pOldRule != pNewRule )
    {
        SwUndoInsNum* pUndo = 0;
        if( DoesUndo() )
        {
            ClearRedo();
            StartUndo( UNDO_START );        // Klammerung fuer die Attribute!
            AppendUndo( pUndo = new SwUndoInsNum( rPos, *pNewRule, rOldRule ) );
        }

        SwNumRuleInfo aUpd( rOldRule );
        aUpd.MakeList( *this );

        // Position suchen und bestimme ob ein Node davor oder dahinter
        // einen Start erzwingt
        SwTxtNode* pTxtNd;
        ULONG nFndPos, nFirst, nLast;

        if( TABLE_ENTRY_NOTFOUND != aUpd.GetList().SearchKey(
            rPos.nNode.GetIndex(), &nFndPos ))
            ++nFndPos;

        for( nLast = nFndPos; nLast < aUpd.GetList().Count(); ++nLast )
            if( ( pTxtNd = aUpd.GetList().GetObject( nLast ))->GetNum() &&
                pTxtNd->GetNum()->IsStart() )
                break;
        for( nFirst = nFndPos; nFirst; )
            if( ( pTxtNd = aUpd.GetList().GetObject( --nFirst ))->GetNum() &&
                pTxtNd->GetNum()->IsStart() )
                break;

        // dann neue Numerierung ueber diesen Bereich
        // definieren und den Start am Anfang/Ende zurueck setzen
        pTxtNd = aUpd.GetList().GetObject( nFirst );
        if( pTxtNd->GetNum()->IsStart() )
        {
            ((SwNodeNum*)pTxtNd->GetNum())->SetStart( FALSE );
            if( pUndo )
                pUndo->SetSttNum( pTxtNd->GetIndex() );
        }

        SwRegHistory aRegH( pUndo ? pUndo->GetHistory() : 0 );
        USHORT nChgFmtLevel = 0;
        for( BYTE n = 0; n < MAXLEVEL; ++n )
        {
            const SwNumFmt& rOldFmt = pOldRule->Get( n ),
                            & rNewFmt = pNewRule->Get( n );

            if( rOldFmt.GetAbsLSpace() != rNewFmt.GetAbsLSpace() ||
                rOldFmt.GetFirstLineOffset() != rNewFmt.GetFirstLineOffset() )
                nChgFmtLevel |= ( 1 << n );
        }

        SwNumRuleItem aRule( rNewRule );
        for( ; nFirst < nLast; ++nFirst )
        {
            pTxtNd = aUpd.GetList().GetObject( nFirst );

            aRegH.RegisterInModify( pTxtNd, *pTxtNd );

#ifndef NUM_RELSPACE
            BYTE nLvl = !pTxtNd->GetNum() ? NO_NUMBERING
                            : pTxtNd->GetNum()->GetLevel() & ~NO_NUMLEVEL;
            if( nLvl < MAXLEVEL && (nChgFmtLevel & ( 1 << nLvl )) &&
                pOldRule->IsRuleLSpace( *pTxtNd ) )
                    pTxtNd->SetNumLSpace( TRUE );
#endif

            pTxtNd->SwCntntNode::SetAttr( aRule );
            pTxtNd->NumRuleChgd();
        }
        EndUndo( UNDO_END );
        SetModified();
    }
    return bRet;
}

BOOL SwDoc::NoNum( const SwPaM& rPam )
{
#if 0
// Vielleich koennte man die Funktion auch so definieren:
// den Bereich auf nicht Nummeriert setzen - sprich NO_NUMLEVEL.
//
    ULONG nStt = rPam.GetPoint()->nNode.GetIndex(),
            nEnd = rPam.GetMark()->nNode.GetIndex();
    if( nStt > nEnd )
    {
        ULONG nTmp = nStt; nStt = nEnd; nEnd = nTmp;
    }

    const SfxPoolItem* pItem;
    const String* pName;
    String sNumRule;
    for( ; nStt <= nEnd; ++nStt )
    {
        SwTxtNode* pTNd = GetNodes()[ nStt ]->GetTxtNode();
        if( pTNd && 0 != ( pItem = pTNd->GetNoCondAttr(
            RES_PARATR_NUMRULE, TRUE ) ) &&
            ( pName = &((SwNumRuleItem*)pItem)->GetValue())->Len() )
        {
            SwNodeNum aNum;
            if( pTNd->GetNum() )
                aNum = *pTNd->GetNum();
            aNum.SetLevel( aNum.GetLevel() | NO_NUMLEVEL );
            pTNd->UpdateNum( aNum );

            if( *pName != sNumRule )
            {
                sNumRule = *pName;
                SwNumRule* pRule = FindNumRulePtr( *pName );
                pRule->SetInvalidRule( TRUE );
            }
        }
    }

    // dann noch alle Updaten
    UpdateNumRule();

    // irgendetwas wurde geupdatet
    return 0 != n;
#else

    BOOL bRet = SplitNode( *rPam.GetPoint() );
    // ist ueberhaupt Nummerierung im Spiel ?
    if( bRet )
    {
        // NoNum setzen und Upaten
        const SwNodeIndex& rIdx = rPam.GetPoint()->nNode;
        SwTxtNode* pNd = rIdx.GetNode().GetTxtNode();
        const SwNodeNum* pNum = pNd->GetNum();
        const SwNumRule* pRule = pNd->GetNumRule();
        if( pNum && pRule )
        {
            SwNodeNum aNum( *pNum );
            aNum.SetLevel( aNum.GetLevel() | NO_NUMLEVEL );
            pNd->UpdateNum( aNum );
#ifndef NUM_RELSPACE
            pNd->SetNumLSpace( TRUE );
#endif
            UpdateNumRule( pRule->GetName(), rIdx.GetIndex() );
            SetModified();
        }
        else
            bRet = FALSE;   // keine Nummerierung , ?? oder immer TRUE ??
    }
    return bRet;

#endif
}

BOOL SwDoc::DelNumRules( const SwPaM& rPam )
{
    ULONG nStt = rPam.GetPoint()->nNode.GetIndex(),
            nEnd = rPam.GetMark()->nNode.GetIndex();
    if( nStt > nEnd )
    {
        ULONG nTmp = nStt; nStt = nEnd; nEnd = nTmp;
    }

    SwUndoDelNum* pUndo;
    if( DoesUndo() )
    {
        ClearRedo();
        AppendUndo( pUndo = new SwUndoDelNum( rPam ) );
    }
    else
        pUndo = 0;

    SwRegHistory aRegH( pUndo ? pUndo->GetHistory() : 0 );

    SwNumRuleItem aEmptyRule( aEmptyStr );
    SvxLRSpaceItem aLRSpace;
    String sNumRule;
    const SfxPoolItem* pItem;
    const String* pName;
    SwNumRule* pRule;
    const SwNode* pOutlNd = 0;
    for( ; nStt <= nEnd; ++nStt )
    {
        SwTxtNode* pTNd = GetNodes()[ nStt ]->GetTxtNode();
        if( pTNd && 0 != ( pItem = pTNd->GetNoCondAttr(
            RES_PARATR_NUMRULE, TRUE ) ) &&
            ( pName = &((SwNumRuleItem*)pItem)->GetValue())->Len() )
        {
            if( *pName != sNumRule )
            {
                sNumRule = *pName;
                pRule = FindNumRulePtr( *pName );
                pRule->SetInvalidRule( TRUE );
            }

            // fuers Undo - Attribut Aenderungen merken
            aRegH.RegisterInModify( pTNd, *pTNd );

            BOOL bResetNumRule = FALSE;
            const SwAttrSet* pAttrSet = pTNd->GetpSwAttrSet();
#ifndef NUM_RELSPACE
            BYTE nLvl;
            const SvxLRSpaceItem* pLRSpace;
            if( pAttrSet && SFX_ITEM_SET == pAttrSet->GetItemState(
                RES_LR_SPACE, FALSE, (const SfxPoolItem**)&pLRSpace ) &&
                ( !pTNd->GetNum() ||
                  (( nLvl = (~NO_NUMLEVEL & pTNd->GetNum()->GetLevel() )) <
                    MAXLEVEL && ( pRule->Get( nLvl ).GetAbsLSpace()
                                            == pLRSpace->GetTxtLeft() )) ))
            {
                if( pLRSpace->GetRight() )
                {
                    aLRSpace.SetRight( pLRSpace->GetRight() );
                    pTNd->SwCntntNode::SetAttr( aLRSpace );
                }
                else
                {
                    pTNd->SwCntntNode::ResetAttr( RES_LR_SPACE );
                    pAttrSet = pTNd->GetpSwAttrSet();
                }
                bResetNumRule = TRUE;
            }
#endif
            if( pUndo )
                pUndo->AddNode( *pTNd, bResetNumRule );

            // kommt die Rule vom Node, dann Reseten, sonst auf leer setzen
            if( pAttrSet && SFX_ITEM_SET == pAttrSet->GetItemState(
                RES_PARATR_NUMRULE, FALSE ))
                pTNd->SwCntntNode::ResetAttr( RES_PARATR_NUMRULE );
            else
                pTNd->SwCntntNode::SetAttr( aEmptyRule );

            pTNd->UpdateNum( SwNodeNum( NO_NUMBERING ));

            if( RES_CONDTXTFMTCOLL == pTNd->GetFmtColl()->Which() )
                pTNd->ChkCondColl();
            else if( !pOutlNd && NO_NUMBERING !=
                    ((SwTxtFmtColl*)pTNd->GetFmtColl())->GetOutlineLevel() )
                pOutlNd = pTNd;
        }
    }

    // dann noch alle Updaten
    UpdateNumRule();

    if( pOutlNd )
        GetNodes().UpdtOutlineIdx( *pOutlNd );

    // irgendetwas wurde geupdatet
    return 0 != sNumRule.Len();
}


    // zum naechsten/vorhergehenden Punkt auf gleicher Ebene

BOOL lcl_IsNumOk( BYTE nSrchNum, BYTE& rLower, BYTE& rUpper,
                    BOOL bOverUpper, BYTE nNumber )
{
    register BOOL bRet = FALSE;
    if( nNumber < MAXLEVEL )            // keine Nummerierung ueberspringen
    {
        if( bOverUpper ? nSrchNum == nNumber : nSrchNum >= nNumber )
            bRet = TRUE;
        else if( nNumber > rLower )
            rLower = nNumber;
        else if( nNumber < rUpper )
            rUpper = nNumber;
    }
    else if( nNumber & NO_NUMLEVEL )
    {
        nNumber &= ~NO_NUMLEVEL;
        if( bOverUpper ? FALSE : nSrchNum > nNumber )
            bRet = TRUE;
        else if( nNumber > rLower )
            rLower = nNumber;
        else if( nNumber < rUpper )
            rUpper = nNumber;
    }
    return bRet;
}

BOOL lcl_IsValidPrevNextNumNode( const SwNodeIndex& rIdx )
{
    BOOL bRet = FALSE;
    const SwNode& rNd = rIdx.GetNode();
    switch( rNd.GetNodeType() )
    {
    case ND_ENDNODE:
        bRet = SwTableBoxStartNode == rNd.FindStartNode()->GetStartNodeType() ||
                rNd.FindStartNode()->IsSectionNode();
        break;

    case ND_STARTNODE:
        bRet = SwTableBoxStartNode == ((SwStartNode&)rNd).GetStartNodeType();
        break;

    case ND_SECTIONNODE:            // der ist erlaubt, also weiter
        bRet = TRUE;
        break;
    }
    return bRet;
}

BOOL lcl_GotoNextPrevNum( SwPosition& rPos, BOOL bNext,
                            BOOL bOverUpper, BYTE* pUpper, BYTE* pLower )
{
    const SwTxtNode* pNd = rPos.nNode.GetNode().GetTxtNode();
    const SwNumRule* pRule;
    if( !pNd || 0 == ( pRule = pNd->GetNumRule() ) || !pNd->GetNum() )
        return FALSE;

    register BYTE nTmpNum = pNd->GetNum()->GetLevel(),
                  nSrchNum = nTmpNum & ~NO_NUMLEVEL;

    SwNodeIndex aIdx( rPos.nNode );
    if( nTmpNum & NO_NUMLEVEL )
    {
        // falls gerade mal NO_NUMLEVEL an ist, so such den vorherigen Node
        // mit Nummerierung
        BOOL bError = FALSE;
        do {
            aIdx--;
            if( aIdx.GetNode().IsTxtNode() )
            {
                pNd = aIdx.GetNode().GetTxtNode();
                if( pNd->GetNum() && pRule == pNd->GetNumRule() )
                {
                    nTmpNum = pNd->GetNum()->GetLevel();
                    if( !( nTmpNum & NO_NUMLEVEL &&
                         (( nTmpNum & ~NO_NUMLEVEL ) >= nSrchNum )) )
                        break;      // gefunden
                }
                else
                    bError = TRUE;
            }
            else
                bError = !lcl_IsValidPrevNextNumNode( aIdx );

        } while( !bError );
        if( bError )
            return FALSE;
    }

    BYTE nLower = nSrchNum, nUpper = nSrchNum;
    BOOL bRet = FALSE;

    const SwTxtNode* pLast;
    if( bNext )
        aIdx++, pLast = pNd;
    else
        aIdx--, pLast = 0;

    while( bNext ? ( aIdx.GetIndex() < aIdx.GetNodes().Count() - 1 )
                 : aIdx.GetIndex() )
    {
        if( aIdx.GetNode().IsTxtNode() )
        {
            pNd = aIdx.GetNode().GetTxtNode();
            if( pNd->GetNum() && pRule == pNd->GetNumRule() )
            {
                if( ::lcl_IsNumOk( nSrchNum, nLower, nUpper, bOverUpper,
                                    pNd->GetNum()->GetLevel() ))
                {
                    rPos.nNode = aIdx;
                    rPos.nContent.Assign( (SwTxtNode*)pNd, 0 );
                    bRet = TRUE;
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
            aIdx++;
        else
            aIdx--;
    }

    if( !bRet && !bOverUpper && pLast )     // nicht ueber hoehere Nummmern, aber bis Ende
    {
        if( bNext )
        {
            rPos.nNode = aIdx;
            if( aIdx.GetNode().IsCntntNode() )
                rPos.nContent.Assign( aIdx.GetNode().GetCntntNode(), 0 );
        }
        else
        {
            rPos.nNode.Assign( *pLast );
            rPos.nContent.Assign( (SwTxtNode*)pLast, 0 );
        }
        bRet = TRUE;
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

BOOL SwDoc::GotoNextNum( SwPosition& rPos, BOOL bOverUpper,
                            BYTE* pUpper, BYTE* pLower  )
{
   return ::lcl_GotoNextPrevNum( rPos, TRUE, bOverUpper, pUpper, pLower );
}



BOOL SwDoc::GotoPrevNum( SwPosition& rPos, BOOL bOverUpper,
                            BYTE* pUpper, BYTE* pLower  )
{
   return ::lcl_GotoNextPrevNum( rPos, FALSE, bOverUpper, pUpper, pLower );
}

BOOL SwDoc::NumUpDown( const SwPaM& rPam, BOOL bDown )
{
    ULONG nStt = rPam.GetPoint()->nNode.GetIndex(),
            nEnd = rPam.GetMark()->nNode.GetIndex();
    if( nStt > nEnd )
    {
        ULONG nTmp = nStt; nStt = nEnd; nEnd = nTmp;
    }

    char nDiff = bDown ? 1 : -1;
    if( DoesUndo() )
    {
        ClearRedo();
        AppendUndo( new SwUndoNumUpDown( rPam, nDiff ) );
    }

    BOOL bRet = FALSE;
    String sNumRule;
    const SfxPoolItem* pItem;
    const String* pName;
    for( ; nStt <= nEnd; ++nStt )
    {
        SwTxtNode* pTNd = GetNodes()[ nStt ]->GetTxtNode();
        if( pTNd && 0 != ( pItem = pTNd->GetNoCondAttr(
            RES_PARATR_NUMRULE, TRUE ) ) &&
            ( pName = &((SwNumRuleItem*)pItem)->GetValue())->Len() )
        {
            BYTE nLevel = pTNd->GetNum()->GetLevel();
            if( ( -1 == nDiff && 0 < ( nLevel & ~NO_NUMLEVEL )) ||
                ( 1 == nDiff && MAXLEVEL - 1 > ( nLevel & ~NO_NUMLEVEL ) ) )
            {
                nLevel += nDiff;
                SwNodeNum aNum( *pTNd->GetNum() );
                aNum.SetLevel( nLevel );

                pTNd->UpdateNum( aNum );
#ifndef NUM_RELSPACE
                pTNd->SetNumLSpace( TRUE );
#endif
                if( *pName != sNumRule )
                {
                    sNumRule = *pName;
                    SwNumRule* pRule = FindNumRulePtr( *pName );
                    pRule->SetInvalidRule( TRUE );
                }
                bRet = TRUE;
            }
        }
    }

    if( bRet )
    {
        UpdateNumRule();
        SetModified();
    }
    return bRet;
}

BOOL SwDoc::MoveParagraph( const SwPaM& rPam, long nOffset, BOOL bIsOutlMv )
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    ULONG nStIdx = pStt->nNode.GetIndex();
    ULONG nEndIdx = pEnd->nNode.GetIndex();
    ULONG nInStIdx, nInEndIdx;
    long nOffs = nOffset;
    if( nOffset > 0 )
    {
        nInEndIdx = nEndIdx;
        nEndIdx += nOffset;
        ++nOffs;
    }
    else
    {
        nInEndIdx = nStIdx - 1;
        nStIdx += nOffset;
    }
    nInStIdx = nInEndIdx + 1;
    // Folgende Absatzbloecke sollen vertauscht werden:
    // [ nStIdx, nInEndIdx ] mit [ nInStIdx, nEndIdx ]

    SwNode *pStartNd = GetNodes()[ nStIdx ];
    SwNode *pEndNd = GetNodes()[ nEndIdx ];
    if( !pStartNd->IsCntntNode() || !pEndNd->IsCntntNode() )
        return FALSE;

    pStartNd = pStartNd->FindStartNode();
    pEndNd = pEndNd->FindStartNode();
    // Es muss sich alles in einem Bereich abspielen
    if( pStartNd != pEndNd )
        return FALSE;

    // auf Redlining testen - darf die Selektion ueberhaupt verschoben
    // werden?
    if( !IsIgnoreRedline() )
    {
        USHORT nRedlPos = GetRedlinePos( pStt->nNode.GetNode(), REDLINE_DELETE );
        if( USHRT_MAX != nRedlPos )
        {
            SwPosition aStPos( *pStt ), aEndPos( *pEnd );
            aStPos.nContent = 0;
            SwCntntNode* pCNd = pEnd->nNode.GetNode().GetCntntNode();
            aEndPos.nContent = pCNd ? pCNd->Len() : 1;
            BOOL bCheckDel = TRUE;

            // es existiert fuer den Bereich irgendein Redline-Delete-Object
            for( ; nRedlPos < GetRedlineTbl().Count(); ++nRedlPos )
            {
                const SwRedline* pTmp = GetRedlineTbl()[ nRedlPos ];
                if( !bCheckDel || REDLINE_DELETE == pTmp->GetType() )
                {
                    const SwPosition *pRStt = pTmp->Start(), *pREnd = pTmp->End();
                    switch( ComparePosition( *pRStt, *pREnd, aStPos, aEndPos ))
                    {
                    case POS_COLLIDE_START:
                    case POS_BEHIND:            // Pos1 liegt hinter Pos2
                        nRedlPos = GetRedlineTbl().Count();
                        break;

                    case POS_COLLIDE_END:
                    case POS_BEFORE:            // Pos1 liegt vor Pos2
                        break;
                    case POS_INSIDE:            // Pos1 liegt vollstaendig in Pos2
                        // ist erlaubt, aber checke dann alle nachfolgenden
                        // auf Ueberlappungen
                        bCheckDel = FALSE;
                        break;

                    case POS_OUTSIDE:           // Pos2 liegt vollstaendig in Pos1
                    case POS_EQUAL:             // Pos1 ist genauso gross wie Pos2
                    case POS_OVERLAP_BEFORE:    // Pos1 ueberlappt Pos2 am Anfang
                    case POS_OVERLAP_BEHIND:    // Pos1 ueberlappt Pos2 am Ende
                        return FALSE;
                    }
                }
            }
        }
    }

    {
        // DataChanged vorm verschieben verschicken, dann bekommt
        // man noch mit, welche Objecte sich im Bereich befinden.
        // Danach koennen sie vor/hinter der Position befinden.
        SwDataChanged aTmp( rPam, 0 );
    }

    SwNodeIndex aIdx( nOffset > 0 ? pEnd->nNode : pStt->nNode, nOffs );
    SwNodeRange aMvRg( pStt->nNode, 0, pEnd->nNode, +1 );

    SwRedline* pOwnRedl = 0;
    if( IsRedlineOn() )
    {
        // wenn der Bereich komplett im eigenen Redline liegt, kann es
        // verschoben werden!
        USHORT nRedlPos = GetRedlinePos( pStt->nNode.GetNode(), REDLINE_INSERT );
        if( USHRT_MAX != nRedlPos )
        {
            SwRedline* pTmp = GetRedlineTbl()[ nRedlPos ];
            const SwPosition *pRStt = pTmp->Start(), *pREnd = pTmp->End();
            SwRedline aTmpRedl( REDLINE_INSERT, rPam );
            const SwCntntNode* pCEndNd = pEnd->nNode.GetNode().GetCntntNode();
            // liegt komplett im Bereich, und ist auch der eigene Redline?
            if( aTmpRedl.IsOwnRedline( *pTmp ) &&
                (pRStt->nNode < pStt->nNode ||
                (pRStt->nNode == pStt->nNode && !pRStt->nContent.GetIndex()) ) &&
                (pEnd->nNode < pREnd->nNode ||
                (pEnd->nNode == pREnd->nNode &&
                 pCEndNd ? pREnd->nContent.GetIndex() == pCEndNd->Len()
                         : !pREnd->nContent.GetIndex() )) )
            {
                pOwnRedl = pTmp;
                if( nRedlPos + 1 < GetRedlineTbl().Count() )
                {
                    pTmp = GetRedlineTbl()[ nRedlPos+1 ];
                    if( *pTmp->Start() == *pREnd )
                        // dann doch nicht!
                        pOwnRedl = 0;
                }

                if( pOwnRedl &&
                    !( pRStt->nNode <= aIdx && aIdx <= pREnd->nNode ))
                {
                    // nicht in sich selbst, dann auch nicht moven
                    pOwnRedl = 0;
                }
            }
        }

        if( !pOwnRedl )
        {
            StartUndo( UNDO_START );

            // zuerst das Insert, dann das Loeschen
            SwPosition aInsPos( aIdx );
            aInsPos.nContent.Assign( aIdx.GetNode().GetCntntNode(), 0 );

            SwPaM aPam( pStt->nNode, aMvRg.aEnd );

            SwPaM& rOrigPam = (SwPaM&)rPam;
            rOrigPam.DeleteMark();
            rOrigPam.GetPoint()->nNode = aIdx.GetIndex() - 1;

            BOOL bDelLastPara = !aInsPos.nNode.GetNode().IsCntntNode();

            Copy( aPam, aInsPos );
            if( bDelLastPara )
            {
                // dann muss der letzte leere Node wieder entfernt werden
                aIdx = aInsPos.nNode;
                SwCntntNode* pCNd = GetNodes().GoPrevious( &aInsPos.nNode );
                xub_StrLen nCLen = 0; if( pCNd ) nCLen = pCNd->Len();
                aInsPos.nContent.Assign( pCNd, nCLen );

                // alle die im zu loeschenden Node stehen, mussen auf den
                // naechsten umgestezt werden
                SwPosition* pPos;
                for( USHORT n = 0; n < GetRedlineTbl().Count(); ++n )
                {
                    SwRedline* pTmp = GetRedlineTbl()[ n ];
                    if( ( pPos = &pTmp->GetBound(TRUE))->nNode == aIdx )
                    {
                        pPos->nNode++;
                        pPos->nContent.Assign( pPos->nNode.GetNode().GetCntntNode(),0);
                    }
                    if( ( pPos = &pTmp->GetBound(FALSE))->nNode == aIdx )
                    {
                        pPos->nNode++;
                        pPos->nContent.Assign( pPos->nNode.GetNode().GetCntntNode(),0);
                    }
                }
                CorrRel( aIdx, aInsPos, 0, FALSE );

                pCNd->JoinNext();
            }

            rOrigPam.GetPoint()->nNode++;
            rOrigPam.GetPoint()->nContent.Assign( rOrigPam.GetCntntNode(), 0 );

            SwRedlineMode eOld = GetRedlineMode();
            if( DoesUndo() )
            {
//JP 06.01.98: MUSS noch optimiert werden!!!
SetRedlineMode( REDLINE_ON | REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE );
                AppendUndo( new SwUndoRedlineDelete( aPam, UNDO_DELETE ));
            }
            AppendRedline( new SwRedline( REDLINE_DELETE, aPam ));

//JP 06.01.98: MUSS noch optimiert werden!!!
SetRedlineMode( eOld );
            EndUndo( UNDO_END );
            SetModified();
            return TRUE;
        }
    }

    if( !pOwnRedl && !IsIgnoreRedline() && GetRedlineTbl().Count() )
    {
        SplitRedline( SwPaM( aIdx ));
    }

    ULONG nRedlSttNd, nRedlEndNd;
    if( pOwnRedl )
    {
        const SwPosition *pRStt = pOwnRedl->Start(), *pREnd = pOwnRedl->End();
        nRedlSttNd = pRStt->nNode.GetIndex();
        nRedlEndNd = pREnd->nNode.GetIndex();
    }

    SwUndoMoveNum* pUndo = 0;
    if( DoesUndo() )
        pUndo = new SwUndoMoveNum( rPam, nOffset, bIsOutlMv );


    Move( aMvRg, aIdx, DOC_MOVEREDLINES );

    if( pUndo )
    {
        ClearRedo();
        pUndo->SetStartNode( rPam.Start()->nNode.GetIndex() );
        AppendUndo( pUndo );
    }

    if( pOwnRedl )
    {
        SwPosition *pRStt = pOwnRedl->Start(), *pREnd = pOwnRedl->End();
        if( pRStt->nNode.GetIndex() != nRedlSttNd )
        {
            pRStt->nNode = nRedlSttNd;
            pRStt->nContent.Assign( pRStt->nNode.GetNode().GetCntntNode(),0);
        }
        if( pREnd->nNode.GetIndex() != nRedlEndNd )
        {
            pREnd->nNode = nRedlEndNd;
            SwCntntNode* pCNd = pREnd->nNode.GetNode().GetCntntNode();
            xub_StrLen nL = 0; if( pCNd ) nL = pCNd->Len();
            pREnd->nContent.Assign( pCNd, nL );
        }
    }

    SetModified();
    return TRUE;
}

BOOL SwDoc::NumOrNoNum( const SwNodeIndex& rIdx, BOOL bDel, BOOL bOutline )
{
    BOOL bRet = FALSE;
    const SwNodeNum* pNum;
    const SfxPoolItem* pItem;
    const SwNumRule* pRule;
    SwTxtNode* pTNd = rIdx.GetNode().GetTxtNode();

    if( pTNd &&
        ( bOutline
            ? (NO_NUMBERING != pTNd->GetTxtColl()->GetOutlineLevel() &&
               0 != ( pNum = pTNd->GetOutlineNum()) &&
               0 != ( pRule = GetOutlineNumRule()) )
            : ( 0 != (pItem = pTNd->GetNoCondAttr(RES_PARATR_NUMRULE,TRUE)) &&
                ((SwNumRuleItem*)pItem)->GetValue().Len() &&
                0 != ( pNum = pTNd->GetNum()) &&
                0 != ( pRule = FindNumRulePtr(
                    ((SwNumRuleItem*)pItem)->GetValue() )) )) &&
        (bDel ? 0 != ( pNum->GetLevel() & NO_NUMLEVEL )
             : 0 == ( pNum->GetLevel() & NO_NUMLEVEL ) ) &&
        SVX_NUM_NUMBER_NONE != pRule->Get( GetRealLevel(pNum->GetLevel()) ).GetNumberingType() )
    {
        if( DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoNumOrNoNum( rIdx, bDel, bOutline ) );
        }
        SwNodeNum aNum( *pNum );
        if( bDel )
            aNum.SetLevel( aNum.GetLevel() & ~NO_NUMLEVEL );
        else
            aNum.SetLevel( aNum.GetLevel() | NO_NUMLEVEL );

        if( bOutline )
        {
            pTNd->UpdateOutlineNum( aNum );
            GetNodes().UpdtOutlineIdx( *pTNd );
        }
        else
        {
            pTNd->UpdateNum( aNum );
#ifndef NUM_RELSPACE
            pTNd->SetNumLSpace( TRUE );
#endif
            ((SwNumRule*)pRule)->SetInvalidRule( TRUE );
            UpdateNumRule( pRule->GetName(), rIdx.GetIndex() );
        }
        bRet = TRUE;
        SetModified();
    }
    return bRet;
}

SwNumRule* SwDoc::GetCurrNumRule( const SwPosition& rPos ) const
{
    SwNumRule* pRet = 0;
    const SfxPoolItem* pItem;
    SwTxtNode* pTNd = rPos.nNode.GetNode().GetTxtNode();

    if( pTNd && 0 != ( pItem = pTNd->GetNoCondAttr( RES_PARATR_NUMRULE, TRUE ) ) &&
        ((SwNumRuleItem*)pItem)->GetValue().Len() )
        pRet = FindNumRulePtr( ((SwNumRuleItem*)pItem)->GetValue() );
    return pRet;
}

USHORT SwDoc::FindNumRule( const String& rName ) const
{
    for( USHORT n = pNumRuleTbl->Count(); n; )
        if( (*pNumRuleTbl)[ --n ]->GetName() == rName )
            return n;
/*
//JP 20.11.97: sollte man im Find neue Rule anlegen??
                erstmal nicht
    USHORT nPoolId = GetPoolId( rName, GET_POOLID_NUMRULE );
    if( USHRT_MAX != nPoolId )
    {
        SwDoc* pThis = (SwDoc*)this;
        SwNumRule* pR = pThis->GetNumRuleFromPool( nPoolId );
        for( n = pNumRuleTbl->Count(); n; )
            if( (*pNumRuleTbl)[ --n ] == pR )
                return n;
    }
*/
    return USHRT_MAX;
}

SwNumRule* SwDoc::FindNumRulePtr( const String& rName ) const
{
    for( USHORT n = pNumRuleTbl->Count(); n; )
        if( (*pNumRuleTbl)[ --n ]->GetName() == rName )
            return (*pNumRuleTbl)[ n ];

/*
//JP 20.11.97: sollte man im Find neue Rule anlegen??
                erstmal nicht
    USHORT nPoolId = GetPoolId( rName, GET_POOLID_NUMRULE );
    if( USHRT_MAX != nPoolId )
    {
        SwDoc* pThis = (SwDoc*)this;
        return pThis->GetNumRuleFromPool( nPoolId );
    }
*/
    return 0;
}

USHORT SwDoc::MakeNumRule( const String &rName, const SwNumRule* pCpy )
{
    SwNumRule* pNew;
    if( pCpy )
    {
        pNew = new SwNumRule( *pCpy );
        pNew->SetName( GetUniqueNumRuleName( &rName ));
        if( pNew->GetName() != rName )
        {
            pNew->SetPoolFmtId( USHRT_MAX );
            pNew->SetPoolHelpId( USHRT_MAX );
            pNew->SetPoolHlpFileId( UCHAR_MAX );
        }
        pNew->CheckCharFmts( this );
    }
    else
        pNew = new SwNumRule( GetUniqueNumRuleName( &rName ) );
    USHORT nRet = pNumRuleTbl->Count();
    pNumRuleTbl->Insert( pNew, nRet );
    return nRet;
}

String SwDoc::GetUniqueNumRuleName( const String* pChkStr, BOOL bAutoNum ) const
{
    String aName;
    if( bAutoNum )
    {
        long n = Time().GetTime();
        n += Date().GetDate();
        aName = String::CreateFromInt32( n );
        if( pChkStr && !pChkStr->Len() )
            pChkStr = 0;
    }
    else if( pChkStr && pChkStr->Len() )
        aName = *pChkStr;
    else
    {
        pChkStr = 0;
        aName = SW_RESSTR( STR_NUMRULE_DEFNAME );
    }

    USHORT nNum, nTmp, nFlagSize = ( pNumRuleTbl->Count() / 8 ) +2;
    BYTE* pSetFlags = new BYTE[ nFlagSize ];
    memset( pSetFlags, 0, nFlagSize );

    xub_StrLen nNmLen = aName.Len();
    if( !bAutoNum && pChkStr )
    {
        while( nNmLen-- && '0' <= aName.GetChar( nNmLen ) &&
                           '9' >= aName.GetChar( nNmLen ) )
            ; //nop

        if( ++nNmLen < aName.Len() )
        {
            aName.Erase( nNmLen );
            pChkStr = 0;
        }
    }

    const SwNumRule* pNumRule;
    for( USHORT n = 0; n < pNumRuleTbl->Count(); ++n )
        if( 0 != ( pNumRule = (*pNumRuleTbl)[ n ] ) )
        {
            const String& rNm = pNumRule->GetName();
            if( rNm.Match( aName ) == nNmLen )
            {
                // Nummer bestimmen und das Flag setzen
                nNum = rNm.Copy( nNmLen ).ToInt32();
                if( nNum-- && nNum < pNumRuleTbl->Count() )
                    pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
            }
            if( pChkStr && pChkStr->Equals( rNm ) )
                pChkStr = 0;
        }

    if( !pChkStr )
    {
        // alle Nummern entsprechend geflag, also bestimme die richtige Nummer
        nNum = pNumRuleTbl->Count();
        for( n = 0; n < nFlagSize; ++n )
            if( 0xff != ( nTmp = pSetFlags[ n ] ))
            {
                // also die Nummer bestimmen
                nNum = n * 8;
                while( nTmp & 1 )
                    ++nNum, nTmp >>= 1;
                break;
            }

    }
    __DELETE( nFlagSize ) pSetFlags;
    if( pChkStr && pChkStr->Len() )
        return *pChkStr;
    return aName += String::CreateFromInt32( ++nNum );
}

const SwNode* lcl_FindBaseNode( const SwNode& rNd )
{
    const SwNodes& rNds = rNd.GetNodes();
    ULONG nNdIdx = rNd.GetIndex();
    if( nNdIdx > rNds.GetEndOfExtras().GetIndex() )
        return rNds.GetEndOfContent().FindStartNode();

    const SwNode* pSttNd = rNds[ ULONG(0) ]->FindStartNode();
    const SwNode* pNd = rNd.FindStartNode();
    while( pSttNd != pNd->FindStartNode()->FindStartNode() )
        pNd = pNd->FindStartNode();
    return pNd;
}


void SwDoc::UpdateNumRule()
{
    SwNumRuleTbl& rNmTbl = GetNumRuleTbl();
    for( USHORT n = 0; n < rNmTbl.Count(); ++n )
        if( rNmTbl[ n ]->IsInvalidRule() )
            UpdateNumRule( rNmTbl[ n ]->GetName(), ULONG_MAX );
}

void SwDoc::UpdateNumRule( const String& rName, ULONG nUpdPos )
{
    SwNumRuleInfo aUpd( rName );
    aUpd.MakeList( *this );

    if( ULONG_MAX == nUpdPos )
        nUpdPos = 0;
    else
        aUpd.GetList().SearchKey( nUpdPos, &nUpdPos );

    SwNumRule* pRule = FindNumRulePtr( rName );
    if( nUpdPos < aUpd.GetList().Count() )
    {
        USHORT nInitLevels = USHRT_MAX; // Bitfeld fuer die Levels!
                                        // TRUE: starte mit NumFmt Start
        USHORT nNumVal = 0;
        SwNodeNum aNum( 0 );

        if( pRule->IsContinusNum() )
            nNumVal = pRule->Get( 0 ).GetStart();

        SwTxtNode* pStt = aUpd.GetList().GetObject( nUpdPos );
        SwTxtNode* pPrev = nUpdPos ? aUpd.GetList().GetObject( nUpdPos-1 ) : 0;
        const SwNode* pBaseNd = lcl_FindBaseNode( *pStt );
        if( pPrev  && lcl_FindBaseNode( *pPrev ) == pBaseNd )
        {
            if( pPrev->GetNum() )
            {
                const SwNodeNum* pPrevNdNum = pPrev->GetNum();
                if( pPrevNdNum->GetLevel() & NO_NUMLEVEL )
                {
                    BYTE nSrchLvl = GetRealLevel( pPrevNdNum->GetLevel() );
                    pPrevNdNum = 0;
                    ULONG nArrPos = nUpdPos-1;
                    while( nArrPos-- )
                    {
                        pPrev = aUpd.GetList().GetObject( nArrPos );
                        if( lcl_FindBaseNode( *pPrev ) != pBaseNd )
                            break;

                        if( 0 != ( pPrevNdNum = pPrev->GetNum() ))
                        {
                            // uebergeordnete Ebene
                            if( nSrchLvl > (pPrevNdNum->GetLevel() &~ NO_NUMLEVEL))
                            {
                                pPrevNdNum = 0;
                                break;
                            }
                            // gleiche Ebene und kein NO_NUMLEVEL
                            if( nSrchLvl == (pPrevNdNum->GetLevel() &~ NO_NUMLEVEL)
                                && !( pPrevNdNum->GetLevel() & NO_NUMLEVEL ))
                                break;

                            pPrevNdNum = 0;
                        }
                    }
                }

                if( pPrevNdNum )
                {
                    aNum = *pPrevNdNum;
                    aNum.SetStart( FALSE );
                    aNum.SetSetValue( USHRT_MAX );
                }
            }
            nInitLevels = 0;
            nNumVal = aNum.GetLevelVal()[ GetRealLevel( aNum.GetLevel() ) ];
        }

        const SwNode* pOutlNd = 0;
        for( ; nUpdPos < aUpd.GetList().Count(); ++nUpdPos )
        {
            pStt = aUpd.GetList().GetObject( nUpdPos );

            const SwNode* pTmpBaseNd = lcl_FindBaseNode( *pStt );
            if( pTmpBaseNd != pBaseNd )
            {
                aNum.SetLevel( 0 );
                memset( aNum.GetLevelVal(), 0,
                        (MAXLEVEL) * sizeof( aNum.GetLevelVal()[0]) );
                pBaseNd = pTmpBaseNd;
            }

            BYTE nLevel = aNum.GetLevel();
            BYTE nNdOldLvl = MAXLEVEL;
            if( pStt->GetNum() )
            {
                if( NO_NUMBERING != pStt->GetNum()->GetLevel() )
                    nNdOldLvl = nLevel = pStt->GetNum()->GetLevel();
                if( pStt->GetNum()->IsStart() )
                {
                    aNum.SetStart( TRUE );
                    memset( aNum.GetLevelVal(), 0,
                            (MAXLEVEL) * sizeof( aNum.GetLevelVal()[0]) );
                    if( pRule->IsContinusNum() )
                    {
                        nNumVal = pRule->Get( 0 ).GetStart();
                        nInitLevels |= 1;
                    }
                    else
                        nInitLevels |= ( 1 << GetRealLevel( nLevel ));
                }
                else if( USHRT_MAX != pStt->GetNum()->GetSetValue() )
                    aNum.SetSetValue( nNumVal = pStt->GetNum()->GetSetValue() );
            }

            if( NO_NUMLEVEL & nLevel )      // NoNum mit Ebene
            {
                BYTE nPrevLvl = GetRealLevel( aNum.GetLevel() ),
                    nCurrLvl = GetRealLevel( nLevel );

                if( nPrevLvl < nCurrLvl )
                {
                    if( !(nInitLevels & ( 1 << nPrevLvl )) )
                        ++nPrevLvl;
                    for( ; nPrevLvl < nCurrLvl; ++nPrevLvl )
                        nInitLevels |= ( 1 << nPrevLvl );
                }

                aNum.SetLevel( nLevel );
                pStt->UpdateNum( aNum );
            }
            else if( NO_NUM != nLevel )
            {
                // beim Format mit Bitmap die Graphicen schon mal anfordern
                const SwNumFmt* pNumFmt = pRule->GetNumFmt( GetRealLevel( nLevel ));
                if( pNumFmt && SVX_NUM_BITMAP == pNumFmt->GetNumberingType() )
                    pNumFmt->GetGraphic();

                if( pRule->IsContinusNum() )
                {
                    if( !(nInitLevels & 1) &&
                        !( pNumFmt && (SVX_NUM_CHAR_SPECIAL == pNumFmt->GetNumberingType() ||
                                       SVX_NUM_BITMAP == pNumFmt->GetNumberingType() ||
                                       SVX_NUM_NUMBER_NONE == pNumFmt->GetNumberingType() )))
                        ++nNumVal;
                    aNum.GetLevelVal()[ nLevel ] = nNumVal;
                }
                else
                {
                    BYTE nPrevLvl = GetRealLevel( aNum.GetLevel() );
                    if( nPrevLvl < nLevel  )
                    {
                        // Erfrage wie geloescht werden soll:
                        // z.B von Stufe 0 -> 1: 1 -> 0.1 ; wenn nStart = 1
                        //  aber Stufe 1 -> 2: 1.1 -> 1.1.1 !!, nur 0.1 -> 0.0.1
                        if( !(nInitLevels & ( 1 << nPrevLvl )) )
                            ++nPrevLvl;

                        for( int ii = nPrevLvl; ii < nLevel; ++ii )
                        {
                            nInitLevels &= ~( 1 << ii );
                            aNum.GetLevelVal()[ ii ] =
                                        pRule->Get( ii ).GetStart();
                        }
                        aNum.GetLevelVal()[ nLevel ] =
                            USHRT_MAX == aNum.GetSetValue()
                                ? pRule->Get( nLevel ).GetStart()
                                : aNum.GetSetValue();
                    }
                    else if( USHRT_MAX != aNum.GetSetValue() )
                        aNum.GetLevelVal()[ nLevel ] = aNum.GetSetValue();
                    else if( nInitLevels & ( 1 << nLevel ))
                        aNum.GetLevelVal()[ nLevel ] =
                                        pRule->Get( nLevel ).GetStart();
                    else
                        aNum.GetLevelVal()[ nLevel ]++;
                }
                nInitLevels &= ~( 1 << nLevel );
                aNum.SetLevel( nLevel );

                pStt->UpdateNum( aNum );
            }

//FEATURE::CONDCOLL
            BOOL bCheck = TRUE;
            if( RES_CONDTXTFMTCOLL == pStt->GetFmtColl()->Which() )
            {
//              SwFmtColl* pChgColl = pStt->GetCondFmtColl();
                pStt->ChkCondColl();
/*
//JP 19.11.97:
// setzen der bedingten Vorlage aendert nichts an den Einzuegen, die bleiben
// als harte vorhanden
                if( pStt->GetCondFmtColl() )
                {
                    // es gab eine Aenderung -> harte Einzuege entfernen
                    if( pChgColl != pStt->GetCondFmtColl() )
                        pStt->ResetAttr( RES_LR_SPACE );
                    bCheck = FALSE;
                }
*/
            }
            else if( !pOutlNd && NO_NUMBERING !=
                    ((SwTxtFmtColl*)pStt->GetFmtColl())->GetOutlineLevel() )
                pOutlNd = pStt;

//FEATURE::CONDCOLL

#ifndef NUM_RELSPACE
            // hat sich eine Level - Aenderung ergeben, so setze jetzt die
            // gueltigen Einzuege
            if( bCheck && ( nLevel != nNdOldLvl || pStt->IsSetNumLSpace())
                && GetRealLevel( nLevel ) < MAXLEVEL )
            {
                SvxLRSpaceItem aLR( ((SvxLRSpaceItem&)pStt->SwCntntNode::GetAttr(
                                    RES_LR_SPACE )) );

                const SwNumFmt& rNFmt = pRule->Get( GetRealLevel( nLevel ));

                // ohne Nummer immer ohne FirstLineOffset!!!!
                short nFOfst = rNFmt.GetFirstLineOffset();
                if( nLevel & NO_NUMLEVEL ) nFOfst = 0;
                aLR.SetTxtFirstLineOfstValue( nFOfst );
                aLR.SetTxtLeft( rNFmt.GetAbsLSpace() );

                pStt->SwCntntNode::SetAttr( aLR );
            }
            // Flag immer loeschen!
            pStt->SetNumLSpace( FALSE );
#endif
            aNum.SetStart( FALSE );
            aNum.SetSetValue( USHRT_MAX );
        }
        if( pOutlNd )
            GetNodes().UpdtOutlineIdx( *pOutlNd );
    }

    ASSERT( pRule, "die NumRule sollte schon vorhanden sein!" );
    if( pRule )
        pRule->SetInvalidRule( FALSE );
}



