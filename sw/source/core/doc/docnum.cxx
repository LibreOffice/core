/*************************************************************************
 *
 *  $RCSfile: docnum.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 14:55:26 $
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
#ifndef _SVX_ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif

#include <dbgoutsw.hxx>

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

        pNumRuleTbl->Insert(pOutlineRule, pNumRuleTbl->Count()); // #115901#
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
            if (pTxtNd)
            {
                if( NO_NUMBERING == pTxtNd->GetTxtColl()->GetOutlineLevel() )
                {
                    pTxtNd->UpdateNum( aNoNum );
                    rArr.Remove( n-- );
                }
                else
                {
                    SwPaM aPam(*pTxtNd);
                    SwNumRuleItem aItem(pOutlineRule->GetName());

                    Insert(aPam, aItem);
                }
            }
        }
    }

    // suche alle Nodes, die neu aufgenommen werden muessen !!
    // (eigentlich koennte das auch per Modify am die Nodes propagiert
    // werden !! )
    ULONG nStt = GetNodes().GetEndOfContent().StartOfSectionIndex();
    USHORT n;

    for( n = 0; n < pTxtFmtCollTbl->Count(); ++n )
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
                if( ! IsNum(pColl->GetOutlineLevel()) )
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
        if( ( 1 << (GetRealLevel(pNd->GetTxtColl()->GetOutlineLevel()))
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

    /* --> #111107# */
    /* Find the last occupied level (backward). */
    for (n = MAXLEVEL - 1; n > 0; n--)
    {
        if (aCollArr[n] != 0)
            break;
    }

    /* If an occupied level is found, choose next level (which IS
       unoccupied) until a valid level is found. If no occupied level
       was found n is 0 and aCollArr[0] is 0. In this case no demoting
       is possible. */
    if (aCollArr[n] != 0)
    {
        while (n < MAXLEVEL - 1)
        {
            n++;

            SwTxtFmtColl *aTmpColl =
                GetTxtCollFromPool(RES_POOLCOLL_HEADLINE1 + n);

            if (aTmpColl->GetOutlineLevel() == n)
            {
                aCollArr[n] = aTmpColl;

                break;
            }
        }
    }

    /* Find the first occupied level (forward). */
    for (n = 0; n < MAXLEVEL - 1; n++)
    {
        if (aCollArr[n] != 0)
            break;
    }

    /* If an occupied level is found, choose previous level (which IS
       unoccupied) until a valid level is found. If no occupied level
       was found n is MAXLEVEL - 1 and aCollArr[MAXLEVEL - 1] is 0. In
       this case no demoting is possible. */
    if (aCollArr[n] != 0)
    {
        while (n > 0)
        {
            n--;

            SwTxtFmtColl *aTmpColl =
                GetTxtCollFromPool(RES_POOLCOLL_HEADLINE1 + n);

            if (aTmpColl->GetOutlineLevel() == n)
            {
                aCollArr[n] = aTmpColl;

                break;
            }
        }
    }
    /* <-- #111107# */

    /* --> #i13747#

       Build a move table that states from which level an outline will

  be moved to which other level. */

    /* the move table

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
        if (aCollArr[n] != NULL)
        {
            USHORT m = n;
            int nCount = nNum;

            while (nCount > 0 && m + nStep >= 0 && m + nStep < MAXLEVEL)
            {
                m += nStep;

                if (aCollArr[m] != NULL)
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
       outline levels occuring in the document there has to be a valid
       target outline level implied by aMoveArr. */
    bool bMoveApplicable = true;
    for (n = nSttPos; n < nEndPos; n++)
    {
        SwTxtNode* pTxtNd = rOutlNds[ n ]->GetTxtNode();
        SwTxtFmtColl* pColl = pTxtNd->GetTxtColl();
        int nLevel = pColl->GetOutlineLevel();

        if (aMoveArr[nLevel] == -1)
            bMoveApplicable = false;
    }

    if (! bMoveApplicable )
        return FALSE;

    /* <-- #i13747 # */
    if( DoesUndo() )
    {
        ClearRedo();
        AppendUndo( new SwUndoOutlineLeftRight( rPam, nOffset ) );
    }

    // 2. allen Nodes die neue Vorlage zuweisen

    n = nSttPos;
    while( n < nEndPos)
    {
        SwTxtNode* pTxtNd = rOutlNds[ n ]->GetTxtNode();
        SwTxtFmtColl* pColl = pTxtNd->GetTxtColl();

        ASSERT(pColl->GetOutlineLevel() < MAXLEVEL,
               "non outline node in outline nodes?");

        int nLevel = pColl->GetOutlineLevel();

        ASSERT(aMoveArr[nLevel] >= 0,
               "move table: current TxtColl not found when building table!");


        if (nLevel < MAXLEVEL && aMoveArr[nLevel] >= 0)
        {
            pColl = aCollArr[ aMoveArr[nLevel] ];

            if (pColl != NULL)
                pColl = (SwTxtFmtColl*)pTxtNd->ChgFmtColl( pColl );
        }

        n++;
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
    for( USHORT n = 0; n < rOutlNds.Count(); ++n )
    {
        SwTxtNode* pTxtNd = rOutlNds[ n ]->GetTxtNode();
        String sTxt( pTxtNd->GetExpandTxt() );
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
        // #i4533# without this check all parts delimited by a dot are treated as outline numbers
        if(!ByteString(sNum, gsl_getSystemTextEncoding()).IsNumericAscii())
            nPos = STRING_NOTFOUND;
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
    //search in the existing outline nodes for the required outline num array
    for( ; nPos < rOutlNds.Count(); ++nPos )
    {
        pNd = rOutlNds[ nPos ]->GetTxtNode();
        BYTE nLvl = pNd->GetTxtColl()->GetOutlineLevel();
        if( nLvl == nLevel - 1 && 0 != pNd->GetNum())
        {
            // check for the outline num
            const SwNodeNum* pNdNum = pNd->GetNum() ;
            const USHORT* pLevelVal = pNdNum->GetLevelVal();
            //now compare with the one searched for
            bool bEqual = true;
            for( BYTE n = 0; (n < nLevel) && bEqual; ++n )
            {
                bEqual = pLevelVal[n] == nLevelVal[n];
            }
            if(bEqual)
            {
                break;
            }
        }
    }
    if( nPos >= rOutlNds.Count() )
        nPos = USHRT_MAX;
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
            String sExpandedText = pNd->GetExpandTxt();
            //#i4533# leading numbers followed by a dot have been remove while
            //searching for the outline position
            //to compensate this they must be removed from the paragraphs text content, too
            USHORT nPos = 0;
            String sTempNum;
            while(sExpandedText.Len() && (sTempNum = sExpandedText.GetToken(0, '.', nPos)).Len() &&
                    STRING_NOTFOUND != nPos &&
                    ByteString(sTempNum, gsl_getSystemTextEncoding()).IsNumericAscii())
            {
                sExpandedText.Erase(0, nPos);
                nPos = 0;
            }

            if( !sExpandedText.Equals( sName ) )
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

void SwNumRuleInfo::MakeList( SwDoc& rDoc, BOOL bOutline )
{
    // -> #111955#
    if (bOutline)
    {
        const SwOutlineNodes & rOutlineNodes = rDoc.GetNodes().GetOutLineNds();

        for (int i = 0; i < rOutlineNodes.Count(); i++)
        {
            SwTxtNode & aNode = *((SwTxtNode *) rOutlineNodes[i]);

            AddNode(aNode);
        }
    }
    // <- #111955#
    else
    {
        SwModify* pMod;
        const SfxPoolItem* pItem;
        USHORT i, nMaxItems = rDoc.GetAttrPool().GetItemCount
            ( RES_PARATR_NUMRULE);
        for( i = 0; i < nMaxItems; ++i )
        {
            pItem = rDoc.GetAttrPool().GetItem( RES_PARATR_NUMRULE, i );
            if( 0 != pItem)
            {
                pMod = (SwModify*)((SwNumRuleItem*)pItem)->GetDefinedIn();
                if (0 != pMod &&
                    ((SwNumRuleItem*)pItem)->GetValue().Len() &&
                    ((SwNumRuleItem*)pItem)->GetValue() == rName )
                {
                    if( pMod->IsA( TYPE( SwFmt )) )
                        pMod->GetInfo( *this );
                    else
                    {
                        SwTxtNode* pModTxtNode = (SwTxtNode*)pMod;

                        // #115901#
                        if( pModTxtNode->GetNodes().IsDocNodes())
                        {
                            AddNode( *pModTxtNode );
                        }
                    }
                }
            }
        }
    }
}


void lcl_ChgNumRule( SwDoc& rDoc, const SwNumRule& rRule, SwHistory* pHist,
                        SwNumRuleInfo* pRuleInfo = 0 )
{
    SwNumRule* pOld = rDoc.FindNumRulePtr( rRule.GetName() );
    ASSERT( pOld, "ohne die alte NumRule geht gar nichts" );

    USHORT nChkLevel = 0, nChgFmtLevel = 0, nMask = 1;
    BYTE n;

    for( n = 0; n < MAXLEVEL; ++n, nMask <<= 1 )
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
        if( pTxtNd->GetNum() &&
            ( nLvl = (pTxtNd->GetNum()->GetRealLevel()) ) < MAXLEVEL )
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
                        sal_Bool bSetAbsLSpace, sal_Bool bCalledFromShell )
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
    {
        pNew = (*pNumRuleTbl)[ MakeNumRule( rRule.GetName(), &rRule ) ];

        /* #109308# ATTENTION THIS IS NOW PARTLY WRONG! SEE #111078#.

            If called from a shell propagate an existing
            adjust item at the beginning am rPam into the new
            numbering rule. */
        if (bCalledFromShell)
        {
            SwCntntNode * pCntntNode = rPam.GetCntntNode();

            if (pCntntNode)
            {
                SwAttrSet & rAttrSet = pCntntNode->GetSwAttrSet();

                /* #111078# Do not propagate the adjustment but set
                    the adjustment according to the text direction of
                    the paragraph. */

                SvxFrameDirection aDir = (SvxFrameDirection)
                    rAttrSet.GetFrmDir().GetValue();

                switch (aDir)
                {
                case FRMDIR_HORI_LEFT_TOP:
                    pNew->SetNumAdjust(SVX_ADJUST_LEFT);

                    break;

                case FRMDIR_HORI_RIGHT_TOP:
                    pNew->SetNumAdjust(SVX_ADJUST_RIGHT);

                    break;

                default:
                    break;
                }
            }
        }

    }
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
                        : pTxtNd->GetNum()->GetRealLevel();
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

void SwDoc::ReplaceNumRule(const SwPaM & rPaM, const SwNumRule & rNumRule)
{
    if (DoesUndo())
        StartUndo(UNDO_START);

    ULONG nStt = rPaM.Start()->nNode.GetIndex();
    ULONG nEnd = rPaM.End()->nNode.GetIndex();

    for (ULONG n = nStt; n <= nEnd; n++)
    {
        SwTxtNode * pCNd = GetNodes()[n]->GetTxtNode();

        if (pCNd && NULL != pCNd->GetNumRule())
        {
            SwPaM aPam(*pCNd);

            Insert(aPam, SwNumRuleItem(rNumRule.GetName()));
        }
    }

    if (DoesUndo())
        EndUndo(UNDO_START);
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

// #106897#
void SwDoc::ChgNumRuleFmts( const SwNumRule& rRule, const String * pName )
{
    // #106897#
    SwNumRule* pRule = FindNumRulePtr( pName ? *pName : rRule.GetName() );
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

        if (aUpd.GetList().Count() > 0)    // #106897#
        {
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
                            : pTxtNd->GetNum()->GetRealLevel();
            if( nLvl < MAXLEVEL && (nChgFmtLevel & ( 1 << nLvl )) &&
                pOldRule->IsRuleLSpace( *pTxtNd ) )
                    pTxtNd->SetNumLSpace( TRUE );
#endif

                pTxtNd->SwCntntNode::SetAttr( aRule );
                pTxtNd->NumRuleChgd();
            }
            EndUndo( UNDO_END );
            SetModified();

            bRet = TRUE;     // #106897#
        }
    }

    return bRet;
}

BOOL SwDoc::NoNum( const SwPaM& rPam )
{

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
            aNum.SetNoNum( TRUE );
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
                  (( nLvl = pTNd->GetNum()->GetRealLevel() ) <
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

            if (0 != pTNd->GetNumRule())
                pTNd->UpdateNum( SwNodeNum( 0 ));
            else
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
    else if( !IsNum(nNumber))
    {
        SetNoNum(&nNumber, FALSE);
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
                  nSrchNum = GetRealLevel(nTmpNum);

    SwNodeIndex aIdx( rPos.nNode );
    if( !IsNum(nTmpNum) )
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
                    if( !( ! IsNum(nTmpNum) &&
                         (GetRealLevel(nTmpNum) >= nSrchNum )) )
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

// -> #i23731#
const SwNumRule *  SwDoc::SearchNumRule(SwPosition & rPos,
                                        BOOL bForward,
                                        BOOL bNum,
                                        BOOL bOutline,
                                        int nNonEmptyAllowed)
{
    const SwNumRule * pResult = NULL;
    SwTxtNode * pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    SwNode * pStartFromNode = pTxtNd;

    if (pTxtNd)
    {
        SwNodeIndex aIdx(rPos.nNode);

        const SwNode * pNode = NULL;
        do
        {
            if (bForward)
                aIdx++;
            else
                aIdx--;

            if (aIdx.GetNode().IsTxtNode())
            {
                pTxtNd = aIdx.GetNode().GetTxtNode();

                const SwNumRule * pNumRule = pTxtNd->GetNumRule();
                if (pNumRule)
                {
                    if (pNumRule->IsOutlineRule() == bOutline && // #115901#
                        (bNum && pNumRule->Get(0).IsEnumeration() ||
                         !bNum && pNumRule->Get(0).IsItemize())) // #i22362#, #i29560#
                        pResult = pTxtNd->GetNumRule();

                    break;
                }
                else if (pTxtNd->Len() > 0 || NULL != pTxtNd->GetNumRule())
                {
                    if (nNonEmptyAllowed == 0)
                        break;

                    nNonEmptyAllowed--;

                    if (nNonEmptyAllowed < 0)
                        nNonEmptyAllowed = -1;
                }
            }

            pNode = &aIdx.GetNode();
        }
        while (! (pNode == aNodes.DocumentSectionStartNode(pStartFromNode) ||
                  pNode == aNodes.DocumentSectionEndNode(pStartFromNode)));
    }

    return pResult;
}
// <- #i23731#

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

    // -> #115901# outline nodes are promoted or demoted differently
    bool bOnlyOutline = true;
    bool bOnlyNonOutline = true;
    for (ULONG n = nStt; n <= nEnd; n++)
    {
        SwTxtNode * pTxtNd = GetNodes()[n]->GetTxtNode();

        if (pTxtNd)
        {
            SwNumRule * pRule = pTxtNd->GetNumRule();

            if (pRule)
            {
                if (pRule->IsOutlineRule())
                    bOnlyNonOutline = false;
                else
                    bOnlyOutline = false;
            }
        }
    }
    // <- #115901#

    BOOL bRet = TRUE;
    char nDiff = bDown ? 1 : -1;

    // ->#115901#
    if (bOnlyOutline)
        bRet = OutlineUpDown(rPam, nDiff);
    else if (bOnlyNonOutline)
    {

        const SfxPoolItem* pItem;
        const String* pName;

        /* --> #i24560#

        Only promote or demote if all selected paragraphs are
        promotable resp. demotable.

        */
        for (ULONG nTmp = nStt; nTmp <= nEnd; ++nTmp)
        {
            SwTxtNode* pTNd = GetNodes()[ nTmp ]->GetTxtNode();
            if( pTNd)
            {
                pItem = pTNd->GetNoCondAttr(RES_PARATR_NUMRULE, TRUE );

                if (0 != pItem)
                {
                    pName = &((SwNumRuleItem*)pItem)->GetValue();

                    if (pName->Len())
                    {
                        BYTE nLevel = pTNd->GetNum()->GetRealLevel();
                        if( (-1 == nDiff && 0 >= nLevel) ||
                            (1 == nDiff && MAXLEVEL - 1 <= nLevel))
                            bRet = FALSE;
                    }
                }
            }
        }

        if( bRet )
        {
            /* <-- #i24560# */
            if( DoesUndo() )
            {
                ClearRedo();
                AppendUndo( new SwUndoNumUpDown( rPam, nDiff ) );
            }

            String sNumRule;

            for(ULONG nTmp = nStt; nTmp <= nEnd; ++nTmp )
            {
                SwTxtNode* pTNd = GetNodes()[ nTmp ]->GetTxtNode();
                if( pTNd)
                {
                    pItem = pTNd->GetNoCondAttr(RES_PARATR_NUMRULE, TRUE );

                    if (0 != pItem)
                    {
                        pName = &((SwNumRuleItem*)pItem)->GetValue();

                        if (pName->Len())
                        {
                            BYTE nLevel = pTNd->GetNum()->GetLevel();
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
                        }
                    }
                }
            }

            UpdateNumRule();
            SetModified();
        }
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

            /* #101076# When copying to a non-content node Copy will
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
                BOOL bMoved = aInsPam.Move(fnMoveBackward);
                ASSERT(bMoved, "No content node found!");

                if (bMoved)
                {
                    /* Append the new node after the content node
                       found. The new position to insert the moved
                       paragraph at is before the inserted
                       paragraph. */
                    AppendTxtNode(*aInsPam.GetPoint());
                    aInsPos = *aInsPam.GetPoint();
                }
            }

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
            checkRedlining(eOld);
            if( DoesUndo() )
            {
//JP 06.01.98: MUSS noch optimiert werden!!!
SetRedlineMode( REDLINE_ON | REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE );
                AppendUndo( new SwUndoRedlineDelete( aPam, UNDO_DELETE ));
            }

            SwRedline* pNewRedline = new SwRedline( REDLINE_DELETE, aPam );

            // #101654# prevent assertion from aPam's target being deleted
            // (Alternatively, one could just let aPam go out of scope, but
            //  that requires touching a lot of code.)
            aPam.GetBound(TRUE).nContent.Assign( NULL, 0 );
            aPam.GetBound(FALSE).nContent.Assign( NULL, 0 );

            AppendRedline( pNewRedline );

//JP 06.01.98: MUSS noch optimiert werden!!!
SetRedlineMode( eOld );
            EndUndo( UNDO_END );
            SetModified();

            return TRUE;
        }
    }

    if( !pOwnRedl && !IsIgnoreRedline() && GetRedlineTbl().Count() )
    {
        SwPaM aTemp(aIdx);
        SplitRedline(aTemp);
    }

    ULONG nRedlSttNd(0), nRedlEndNd(0);
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

// #115901#: bOutline removed
BOOL SwDoc::NumOrNoNum( const SwNodeIndex& rIdx, BOOL bDel )
{
    BOOL bRet = FALSE;
    const SwNodeNum* pNum = NULL;
    const SfxPoolItem* pItem = NULL;
    const SwNumRule* pRule = NULL;
    SwTxtNode* pTNd = rIdx.GetNode().GetTxtNode();

    BOOL bTmp = FALSE;
    if (pTNd)
    {
        BOOL bTmp = FALSE;
        USHORT nOldLevel = 0;

        pItem = pTNd->GetNoCondAttr(RES_PARATR_NUMRULE,TRUE);

        if (pItem && ((SwNumRuleItem*)pItem)->GetValue().Len() )
        {
            pNum = pTNd->GetNum();

            if (pNum)
            {
                pRule = FindNumRulePtr(((SwNumRuleItem*)pItem)->
                                        GetValue());
            }
        }
        if (pRule)
        {
            if (bDel)
                bTmp = pNum->IsNum(); // #i29560#
            else
                bTmp = !pNum->IsNum(); // #i29560#

            nOldLevel = pNum->GetLevel();
        }

        if (bTmp &&
            SVX_NUM_NUMBER_NONE != pRule->Get(GetRealLevel(pNum->GetLevel())).
            GetNumberingType())
        {
            SwNodeNum aNum( *pNum );

            if( bDel )
                aNum.SetNoNum(TRUE); // #i29560#
            else
                aNum.SetNoNum(FALSE); // #i29560#

            if( DoesUndo() )
            {
                ClearRedo();
                AppendUndo( new SwUndoNumOrNoNum( rIdx, *pNum, aNum ) );
            }

            pTNd->UpdateNum( aNum );
#ifndef NUM_RELSPACE
            pTNd->SetNumLSpace( TRUE );
#endif

            ((SwNumRule*)pRule)->SetInvalidRule( TRUE );
            UpdateNumRule( pRule->GetName(), rIdx.GetIndex() );

            bRet = TRUE;
            SetModified();
        }

        // -> #115901#
        if (! bRet && bDel && pNum && !IsNum(nOldLevel)) // #i29560#
        {
            SwPaM aPam(*pTNd);

            DelNumRules(aPam);

            bRet = TRUE;
        }
        // <- #115901#
    }

    return bRet;
}

SwNumRule* SwDoc::GetCurrNumRule( const SwPosition& rPos ) const
{
    SwNumRule* pRet = 0;
    if (SwTxtNode* pTNd = rPos.nNode.GetNode().GetTxtNode())
        pRet = pTNd->GetNumRule();
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

    USHORT nNum(0), nTmp, nFlagSize = ( pNumRuleTbl->Count() / 8 ) +2;
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
    USHORT n;

    for( n = 0; n < pNumRuleTbl->Count(); ++n )
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
    delete [] pSetFlags;
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

// -> #111955#
/**
   Reset numbering to start values.

   @param rNum                   the numbering to reset
   @param rNumRule               the numbering rule to take the start values
                                 from
   @param nStartLevel            the level to start
   @param bInitializedLevels     array to mark reset levels in

   All values in rNum that have a level >= nStartLevel are set to
   their start level. The start level is looked up in rNumRule. For
   all levels >= nStartLevel the accroding entry in bInitializedLevels
   is set to true.
*/
void lcl_NodeNumReset(SwNodeNum & rNum, const SwNumRule & rNumRule,
                      int nStartLevel, bool * bInitializedLevels)
{
    for (int i = nStartLevel; i < MAXLEVEL; i++)
    {
        rNum.GetLevelVal()[i] = rNumRule.Get(i).GetStart();
        bInitializedLevels[i] = true;
    }
}

/**
   Update numbering for all nodes that have a certain numbering rule.

   @param rName           name of the numbering rule to search for
   @param nUpdatePos      document position to start at
 */
void SwDoc::UpdateNumRule( const String& rName, ULONG nUpdatePos )
{
    SwNumRule * pRule = FindNumRulePtr(rName);
    ASSERT(pRule, "numrule not found");
    if (pRule == NULL)
        return;

    UpdateNumRule(*pRule, nUpdatePos);
}

// #115901#: bOutline removed
/**
   Update numbering for all nodes that have a certain numbering rule.

   @param rRule           numbering rule to search for
   @param nUpdatePos      document position to start at
 */
void SwDoc::UpdateNumRule( SwNumRule & rRule, ULONG nUpdatePos)
{
    /* If old numbering is activated use the old algorithm. */
    if (IsOldNumbering())
    {
        UpdateNumRuleOld(rRule, nUpdatePos);

        return;
    }

    /* Get all paragraphs with the given numbering rule from the
       document. */
    SwNumRuleInfo aNumRuleInfo(rRule.GetName());
    aNumRuleInfo.MakeList(*this);  // #115901#

    /* If there are no matching paragraphs we are done. */
    if (aNumRuleInfo.GetList().Count() == 0)
        return;

    /* ULONG_MAX -> process all paragraphs found */
    if (nUpdatePos == ULONG_MAX)
        nUpdatePos = 0;
    else /* nUpdatePos is still the position in the document. Convert
            to position in the list of found paragraphs. */
        aNumRuleInfo.GetList().SearchKey(nUpdatePos, &nUpdatePos);

    // #115901#
    if (nUpdatePos >= aNumRuleInfo.GetList().Count())
    {
        nUpdatePos = 0;
    }

    /* Temporal numbering holding the values to be changed in the
       current node*/
    SwNodeNum aNum(0);
    /* Array for initialized levels.
    bInitializedLevels[i] == true -> level i is initialized.
    */
    bool bInitializedLevels[MAXLEVEL];
    /* flag for initialized continuous numbering. */
    bool bInitialized = false;
    /* counter for continuous numbering */
    int nCount = 0;

    /* If all paragraphs found are to be processed initialize all
       levels with their start values.*/
    if (nUpdatePos == 0)
    {
        for (int i = 0; i < MAXLEVEL; i++)
            bInitializedLevels[i] = false;

        lcl_NodeNumReset(aNum, rRule, 0, bInitializedLevels);

        nCount = rRule.Get(0).GetStart();
        bInitialized = true;
    }
    else /* If we start at a certain paragraph fill aNum/nCount with
            values from that paragraph. Mark all levels as initalized,
            including continuous numbering. */
    {
        for (int i = 0; i < MAXLEVEL; i++)
            bInitializedLevels[i] = true;

        aNum = *aNumRuleInfo.GetList().GetObject(nUpdatePos)->
            GetNum(); // #115901#

        nCount = aNum.GetLevelVal()[aNum.GetRealLevel()];
        bInitialized = true;
    }

    /* The old level is the level of the first node to process. */
    const SwNodeNum * pNum = aNumRuleInfo.GetList().GetObject(nUpdatePos)->
        GetNum();
    BYTE nOldLevel = pNum ? pNum->GetLevel() : 0;

    /* Iterate over all nodes to process. */
    while(nUpdatePos < aNumRuleInfo.GetList().Count())
    {
        /* Get the current node. */
        SwTxtNode * pTxtNode = aNumRuleInfo.GetList().GetObject(nUpdatePos);

        if (pTxtNode->MayBeNumbered())
        {
            /* If the current node has a conditional paragraph style,
               ensure the current node gets the resulting style. */
            if( RES_CONDTXTFMTCOLL == pTxtNode->GetFmtColl()->Which() )
                pTxtNode->ChkCondColl();

            /* Get old numbering of the current node. */
            const SwNodeNum * pOldNum = pTxtNode->GetNum(); // #115901#

            // #111955# fixed loop due to not increasing nUpdatePos when
            // pOldNum == 0
            if (pOldNum && pOldNum->IsNum()) // #i29560#
            {
                BYTE nLevel = pOldNum->GetRealLevel();

                /* If numbering restarts at the current node ...*/
                if (pOldNum->IsStart())
                {
                    /* Fill aNum with start values of current level and
                       subsequent levels. */
                    lcl_NodeNumReset(aNum, rRule, nLevel, bInitializedLevels);

                    /* Fill nCount with start values for continuous
                       numbering. */
                    nCount = rRule.Get(0).GetStart();
                    bInitialized = true;
                }
                /* If numbering restarts with a specific value ...*/
                else if (pOldNum->HasSetValue())
                {
                    /* Fill aNum with start values of current level and
                       subsequent levels. This also marks the current level as
                       initialized. Set specific value for current level. */
                    lcl_NodeNumReset(aNum, rRule, nLevel, bInitializedLevels);
                    aNum.GetLevelVal()[nLevel] = pOldNum->GetSetValue();

                    /* Fill nCount with the specific start value. */
                    nCount = pOldNum->GetSetValue();
                    bInitialized = true;
                }
                /* If the current node is the first in a row of nodes of its
                   level initialize the subsequent levels. */
                else if (nOldLevel != nLevel)
                {
                    lcl_NodeNumReset(aNum, rRule, nLevel + 1,
                                     bInitializedLevels);
                }
                nOldLevel = nLevel;

                int i;

                /* All previous levels are initialized. */
                for (i = nLevel - 1; i >= 0; i--)
                    bInitializedLevels[i] = false;

                /* If the level of the current node is initialized do not
                   increase the value for this level. Mark this level as
                   uninitialized. */
                if (bInitializedLevels[nLevel])
                    bInitializedLevels[nLevel] = false;
                else /* If the level of the current node was not initialized
                        increase the value for this level. */
                    aNum.GetLevelVal()[nLevel]++;

                /* If the continuous numbering was initialized. mark it as
                   uninitialized and do not increase the value for the
                   continuous numbering. */
                if (bInitialized)
                    bInitialized = false;
                else /* If the continuous numbering was not initialized,
                        increase its value. */
                    nCount++;

                /* If the current numbering is continuous set the value for
                   the current level to the value of the continous
                   numbering. Mark the current numbering as
                   continous. Otherwise mark the current numbering as
                   non-continuous. */
                if (rRule.IsContinusNum())
                {
                    aNum.GetLevelVal()[nLevel] = nCount;
                    aNum.SetContinuousNum();
                }
                else
                {
                    aNum.SetContinuousNum(FALSE);
                }

                /* bChanged = true -> the current node's numbering has
                   to be altered. */
                bool bChanged = false;

                /* the numbering to replace the current numbering */
                SwNodeNum aTmpNum = *pOldNum;

                if (aNum.IsContinuousNum() != aTmpNum.IsContinuousNum())
                {
                    /* Continuous numbering has been activated or deactivated
                       -> change the numbering of the current node. */
                    aTmpNum.SetContinuousNum(aNum.IsContinuousNum());
                    aTmpNum.GetLevelVal()[nLevel] = nCount;
                    bChanged = true;
                }

                /* For each level synchronize the local numbering (aNum)
                   with the numbering to be set in the current node
                   (aTmpNum). If there are differences propagate the value
                   fom aNum to aTmpNum. In this case the numbering of the
                   current node. */
                for (i = 0; i < MAXLEVEL; i++)
                {
                    if (aTmpNum.GetLevelVal()[i] != aNum.GetLevelVal()[i])
                    {
                        aTmpNum.GetLevelVal()[i] = aNum.GetLevelVal()[i];
                        bChanged = true;
                    }
                }

                /* If the current node's numbering is to be changed change the
                   according normal or outline numbering. */
                if (bChanged)
                {
                    // #115901#
                    pTxtNode->UpdateNum(aTmpNum);
                }
            }
            else if (! pOldNum) // #i29560#
            {
                ASSERT(0, "No number!");
            }
        }

        nUpdatePos++;
    }
}

// pre-SRC680-numbering
void SwDoc::UpdateNumRuleOld( SwNumRule & rRule, ULONG nUpdPos )
{
    SwNumRuleInfo aUpd( rRule.GetName() );
    aUpd.MakeList( *this );

    if( ULONG_MAX == nUpdPos )
        nUpdPos = 0;
    else
        aUpd.GetList().SearchKey( nUpdPos, &nUpdPos );

    SwNumRule* pRule = &rRule;

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
                if( ! pPrevNdNum->IsNum() )
                {
                    // OD 10.12.2002 #106111# - use correct search level
                    BYTE nSrchLvl = GetRealLevel( pStt->GetNum()->GetLevel() );
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
                            if( nSrchLvl > pPrevNdNum->GetRealLevel())
                            {
                                pPrevNdNum = 0;
                                break;
                            }
                            // gleiche Ebene und kein NO_NUMLEVEL
                            if( nSrchLvl == pPrevNdNum->GetRealLevel()
                                && pPrevNdNum->IsNum())
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
            // OD 10.12.2002 #106111# - sublevels have to be restarted.
            for ( int nSubLvl = GetRealLevel( aNum.GetLevel() ) + 1; nSubLvl < MAXLEVEL; ++nSubLvl)
                nInitLevels |= ( 1 << nSubLvl );
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
                    // OD 10.12.2002 #106111# - correct reset of level numbers
                    for ( int nSubLvl = nLevel; nSubLvl < MAXLEVEL; ++nSubLvl)
                        aNum.GetLevelVal()[ nSubLvl ] = 0;
                    if( pRule->IsContinusNum() )
                    {
                        nNumVal = pRule->Get( 0 ).GetStart();
                        nInitLevels |= 1;
                    }
                    else
                        nInitLevels |= ( 1 << GetRealLevel( nLevel ));
                }
                else if( USHRT_MAX != pStt->GetNum()->GetSetValue() )
                {
                    aNum.SetSetValue( nNumVal = pStt->GetNum()->GetSetValue() );
                    // OD 10.12.2002 #106111# - init <nInitLevels> for continues
                    // numbering.
                    if( pRule->IsContinusNum() )
                        nInitLevels |= 1;
                }
            }

            if( ! IsNum(nLevel ))       // NoNum mit Ebene
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
            else //if( NO_NUM != nLevel )
            {
                ASSERT(NO_NUM != nLevel, "NO_NUM?");

                // beim Format mit Bitmap die Graphicen schon mal anfordern
                const SwNumFmt* pNumFmt = pRule->GetNumFmt( GetRealLevel( nLevel ));
                if (! (pNumFmt && SVX_NUM_NUMBER_NONE == pNumFmt->GetNumberingType()))
                {
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
                        // OD 10.12.2002 #106111# - reset <nInitLevels>
                        nInitLevels &= ~1;
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
                        {
                            const SwNumFmt * pTmpNumFmt = pRule->GetNumFmt(nLevel);

                            if (pTmpNumFmt &&
                                SVX_NUM_NUMBER_NONE !=
                                pTmpNumFmt->GetNumberingType())
                                aNum.GetLevelVal()[ nLevel ]++;
                        }
                    }
                    nInitLevels &= ~( 1 << nLevel );
                    aNum.SetLevel( nLevel );

                    // OD 10.12.2002 #106111# - reset numbers of all sublevels and
                    // note in <nInitLevels> that numbering of all sublevels have
                    // to be restarted.
                    for ( int nSubLvl = nLevel+1; nSubLvl < MAXLEVEL; ++nSubLvl)
                    {
                        aNum.GetLevelVal()[ nSubLvl ] = 0;
                        nInitLevels |= ( 1 << nSubLvl );
                    }

                    pStt->UpdateNum( aNum );
                }
            }

//FEATURE::CONDCOLL
            if( RES_CONDTXTFMTCOLL == pStt->GetFmtColl()->Which() )
                pStt->ChkCondColl();
            else if( !pOutlNd && NO_NUMBERING !=
                    ((SwTxtFmtColl*)pStt->GetFmtColl())->GetOutlineLevel() )
                pOutlNd = pStt;

//FEATURE::CONDCOLL

#ifndef NUM_RELSPACE
            // hat sich eine Level - Aenderung ergeben, so setze jetzt die
            // gueltigen Einzuege
            if (( nLevel != nNdOldLvl || pStt->IsSetNumLSpace())
                && GetRealLevel( nLevel ) < MAXLEVEL )
            {
                SvxLRSpaceItem aLR( ((SvxLRSpaceItem&)pStt->SwCntntNode::GetAttr(
                                    RES_LR_SPACE )) );

                const SwNumFmt& rNFmt = pRule->Get( GetRealLevel( nLevel ));

                // ohne Nummer immer ohne FirstLineOffset!!!!
                short nFOfst = rNFmt.GetFirstLineOffset();
                if( ! IsNum(nLevel)) nFOfst = 0;
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
#if 0
        if( pOutlNd )
            GetNodes().UpdtOutlineIdx( *pOutlNd );
#endif
    }

    ASSERT( pRule, "die NumRule sollte schon vorhanden sein!" );
    if( pRule )
        pRule->SetInvalidRule( FALSE );
}
// <- #111955#

// -> #i27615#
void SwDoc::SetMarkedNumLevel(SwNumRule & rNumRule, BYTE nLevel, BOOL nValue)
{
    /*
       - Set new marked level.
       - Notify all affected nodes of the changed mark.
    */

    SwBitArray aChangedLevels = rNumRule.SetLevelMarked(nLevel, nValue);

    SwNumRuleInfo aInfo(rNumRule.GetName());

    aInfo.MakeList(*this, FALSE);

    for (int i = 0; i < aInfo.GetList().Count(); i++)
    {
        SwTxtNode * pTxtNd = aInfo.GetList().GetObject(i);
        const SwNodeNum * pNdNum = pTxtNd->GetNum();

        if (pNdNum && aChangedLevels.Get(pNdNum->GetRealLevel()))
            pTxtNd->NumRuleChgd();
    }
}

void SwDoc::SetMarkedNumLevel(const String & sNumRule, BYTE nLevel,
                              BOOL bValue)
{
    SwNumRule * pNumRule = FindNumRulePtr(sNumRule);

    if (pNumRule)
        SetMarkedNumLevel(*pNumRule, nLevel, bValue);
}
// <- #i27615#

// #i23726#
BOOL SwDoc::IsFirstOfNumRule(SwPosition & rPos)
{
    BOOL bResult = FALSE;
    SwTxtNode * pTxtNode = rPos.nNode.GetNode().GetTxtNode();

    if (pTxtNode)
    {
        SwNumRule * pNumRule = pTxtNode->GetNumRule();

        if (pNumRule)
        {
            SwNumRuleInfo aNumInfo(pNumRule->GetName());
            aNumInfo.MakeList(*this);

            if (aNumInfo.GetList().Count() > 0 &&
                aNumInfo.GetList().GetObject(0) == pTxtNode)
                bResult = TRUE;
        }
    }

    return bResult;
}

// #i23726#
void SwDoc::IndentNumRule(SwPosition & rPos, short nAmount)
{
    if (SwTxtNode * pTxtNode = rPos.nNode.GetNode().GetTxtNode())
    {
        if (SwNumRule * pNumRule = pTxtNode->GetNumRule())
        {
            pNumRule->Indent(nAmount);
            UpdateNumRule();
        }
    }
}

// #106897#
sal_Bool SwDoc::RenameNumRule(const String & rOldName, const String & rNewName)
{
    sal_Bool bResult = sal_False;
    SwNumRule * pOldRule = FindNumRulePtr(rOldName);

    if (pOldRule)
    {
        MakeNumRule(rNewName, pOldRule);

        SwPosition aPos = SwPosition(SwNodeIndex(*aNodes[0]));
        bResult = ReplaceNumRule(aPos, rOldName, rNewName);

        if (bResult)
            bResult = DelNumRule(rOldName);
    }

    return bResult;
}

