/*************************************************************************
 *
 *  $RCSfile: swcrsr.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 15:41:14 $
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

#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HDL
#include <com/sun/star/i18n/WordType.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_CHARTYPE_HDL
#include <com/sun/star/i18n/CharType.hdl>
#endif


#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <svtools/ctloptions.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _SWTBLFMT_HXX //autogen
#include <swtblfmt.hxx>
#endif
#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _SCRIPTINFO_HXX
#include <scriptinfo.hxx>
#endif
#ifndef _CRSTATE_HXX
#include <crstate.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // ...Percent()
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>          // ResId fuer Statusleiste
#endif

using namespace ::com::sun::star::i18n;

static const USHORT coSrchRplcThreshold = 60000;

struct _PercentHdl
{
    SwDocShell* pDSh;
    ULONG nActPos;
    BOOL bBack, bNodeIdx;

    _PercentHdl( ULONG nStt, ULONG nEnd, SwDocShell* pSh )
        : pDSh( pSh )
    {
        nActPos = nStt;
        if( 0 != ( bBack = (nStt > nEnd )) )
        {
            ULONG n = nStt; nStt = nEnd; nEnd = n;
        }
        ::StartProgress( STR_STATSTR_SEARCH, nStt, nEnd, 0 );
    }

    _PercentHdl( const SwPaM& rPam )
        : pDSh( (SwDocShell*)rPam.GetDoc()->GetDocShell() )
    {
        ULONG nStt, nEnd;
        if( rPam.GetPoint()->nNode == rPam.GetMark()->nNode )
        {
            bNodeIdx = FALSE;
            nStt = rPam.GetMark()->nContent.GetIndex();
            nEnd = rPam.GetPoint()->nContent.GetIndex();
        }
        else
        {
            bNodeIdx = TRUE;
            nStt = rPam.GetMark()->nNode.GetIndex();
            nEnd = rPam.GetPoint()->nNode.GetIndex();
        }
        nActPos = nStt;
        if( 0 != ( bBack = (nStt > nEnd )) )
        {
            ULONG n = nStt; nStt = nEnd; nEnd = n;
        }
        ::StartProgress( STR_STATSTR_SEARCH, nStt, nEnd, pDSh );
    }

    ~_PercentHdl()                      { ::EndProgress( pDSh ); }

    void NextPos( ULONG nPos ) const
        { ::SetProgressState( bBack ? nActPos - nPos : nPos, pDSh ); }

    void NextPos( SwPosition& rPos ) const
        {
            ULONG nPos;
            if( bNodeIdx )
                nPos = rPos.nNode.GetIndex();
            else
                nPos = rPos.nContent.GetIndex();
            ::SetProgressState( bBack ? nActPos - nPos : nPos, pDSh );
        }
};

SwCursor::SwCursor( const SwPosition &rPos, SwPaM* pRing )
    : SwPaM( rPos, pRing ), pSavePos( 0 ), nCursorBidiLevel( 0 )
{
}

SwCursor::SwCursor( SwCursor& rCpy )
    : SwPaM( rCpy ), pSavePos( 0 ), nCursorBidiLevel( rCpy.nCursorBidiLevel )
{
}

SwCursor::~SwCursor()
{
    while( pSavePos )
    {
        _SwCursor_SavePos* pNext = pSavePos->pNext;
        delete pSavePos;
        pSavePos = pNext;
    }
}

SwCursor* SwCursor::Create( SwPaM* pRing ) const
{
    return new SwCursor( *GetPoint(), pRing );
}

SwCursor::operator SwTableCursor* ()        { return 0; }
SwCursor::operator SwShellCrsr* ()          { return 0; }
SwCursor::operator SwShellTableCrsr* ()     { return 0; }
SwCursor::operator SwUnoCrsr* ()            { return 0; }
SwCursor::operator SwUnoTableCrsr* ()       { return 0; }


// Sicher die aktuelle Position, damit ggfs. auf diese zurueck
// gefallen werden kann. Die SavePos Objekte werden als Stack verwaltet,
// damit das auch alles bei verschachtelten Aufrufen funktioniert.
// Das CreateNewSavePos ist virtual, damit abgeleitete Klassen vom Cursor
// gegebenenfalls eigene SaveObjecte anlegen und in den virtuellen
// Check-Routinen verwenden koennen.

void SwCursor::SaveState()
{
    _SwCursor_SavePos* pNew = CreateNewSavePos();
    pNew->pNext = pSavePos;
    pSavePos = pNew;
}

void SwCursor::RestoreState()
{
    _SwCursor_SavePos* pDel = pSavePos;
    pSavePos = pSavePos->pNext;
    delete pDel;
}

_SwCursor_SavePos* SwCursor::CreateNewSavePos() const
{
    return new _SwCursor_SavePos( *this );
}

// stelle fest, ob sich der Point ausserhalb des Content-Bereichs
// vom Nodes-Array befindet
FASTBOOL SwCursor::IsNoCntnt() const
{
    return GetPoint()->nNode.GetIndex() <
            GetDoc()->GetNodes().GetEndOfExtras().GetIndex();
}


FASTBOOL SwCursor::IsSelOvr( int eFlags )
{
    SwTableCursor* pTblCrsr = *this;
    SwDoc* pDoc = GetDoc();
    SwNodes& rNds = pDoc->GetNodes();

    BOOL bSkipOverHiddenSections, bSkipOverProtectSections;
    SwUnoCrsr* pUnoCrsr = *this;
    if( pUnoCrsr )
    {
        bSkipOverHiddenSections = pUnoCrsr->IsSkipOverHiddenSections();
        bSkipOverProtectSections = pUnoCrsr->IsSkipOverProtectSections();
    }
    else
    {
        bSkipOverHiddenSections = TRUE;
        bSkipOverProtectSections = !IsReadOnlyAvailable();
    }

    // Bereiche vom Nodes-Array ueberpruefen
    if( (SELOVER_CHECKNODESSECTION & eFlags) && pTblCrsr && HasMark() )
    {
        SwNodeIndex aOldPos( rNds, pSavePos->nNode );
        if( !CheckNodesRange( aOldPos, GetPoint()->nNode, TRUE ))
        {
            GetPoint()->nNode = aOldPos;
            GetPoint()->nContent.Assign( GetCntntNode(), pSavePos->nCntnt );
            return TRUE;
        }
    }

// neu: Bereiche ueberpruefen
// Anfang
    if( pSavePos->nNode != GetPoint()->nNode.GetIndex() &&
        //JP 28.10.97: Bug 45129 - im UI-ReadOnly ist alles erlaubt
        ( !pDoc->GetDocShell() || !pDoc->GetDocShell()->IsReadOnlyUI() ))
    {
        // teste doch mal die neuen Sections:
        SwNodeIndex& rPtIdx = GetPoint()->nNode;
        const SwSectionNode* pSectNd = rPtIdx.GetNode().FindSectionNode();
        if( pSectNd &&
            ((bSkipOverHiddenSections && pSectNd->GetSection().IsHiddenFlag() ) ||
             (bSkipOverProtectSections && pSectNd->GetSection().IsProtectFlag() )))
        {
            if( 0 == ( SELOVER_CHANGEPOS & eFlags ) )
            {
                // dann wars das schon
                RestoreSavePos();
                return TRUE;
            }

            // dann setze den Cursor auf die neue Position:
            SwNodeIndex aIdx( rPtIdx );
            xub_StrLen nCntntPos = pSavePos->nCntnt;
            int bGoNxt = pSavePos->nNode < rPtIdx.GetIndex();
            SwCntntNode* pCNd = bGoNxt
                    ? rNds.GoNextSection( &rPtIdx, bSkipOverHiddenSections, bSkipOverProtectSections)
                    : rNds.GoPrevSection( &rPtIdx, bSkipOverHiddenSections, bSkipOverProtectSections);
            if( !pCNd && ( SELOVER_ENABLEREVDIREKTION & eFlags ))
            {
                bGoNxt = !bGoNxt;
                pCNd = bGoNxt ? rNds.GoNextSection( &rPtIdx, bSkipOverHiddenSections, bSkipOverProtectSections)
                              : rNds.GoPrevSection( &rPtIdx, bSkipOverHiddenSections, bSkipOverProtectSections);
            }

            int bIsValidPos = 0 != pCNd;
            FASTBOOL bValidNodesRange = bIsValidPos &&
                                    ::CheckNodesRange( rPtIdx, aIdx, TRUE );
            if( !bValidNodesRange )
            {
                rPtIdx = pSavePos->nNode;
                if( 0 == ( pCNd = rPtIdx.GetNode().GetCntntNode() ) )
                {
                    bIsValidPos = FALSE;
                    nCntntPos = 0;
                    rPtIdx = aIdx;
                    if( 0 == ( pCNd = rPtIdx.GetNode().GetCntntNode() ) )
                    {
                        // dann auf den Anfang vom Doc
                        rPtIdx = rNds.GetEndOfExtras();
                        pCNd = rNds.GoNext( &rPtIdx );
                    }
                }
            }

            // ContentIndex noch anmelden:
            xub_StrLen nTmpPos = bIsValidPos ? (bGoNxt ? 0 : pCNd->Len()) : nCntntPos;
            GetPoint()->nContent.Assign( pCNd, nTmpPos );
            if( !bIsValidPos || !bValidNodesRange ||
                // sollten wir in einer Tabelle gelandet sein?
                IsInProtectTable( TRUE ) )
                return TRUE;
        }

        // oder sollte eine geschuetzte Section innerhalb der Selektion liegen?
        if( HasMark() && bSkipOverProtectSections)
        {
            ULONG nSttIdx = GetMark()->nNode.GetIndex(),
                  nEndIdx = GetPoint()->nNode.GetIndex();
            if( nEndIdx <= nSttIdx )
            {
                ULONG nTmp = nSttIdx;
                nSttIdx = nEndIdx;
                nEndIdx = nTmp;
            }

            const SwSectionFmts& rFmts = pDoc->GetSections();
            for( USHORT n = 0; n < rFmts.Count(); ++n )
            {
                const SwSectionFmt* pFmt = rFmts[n];
                const SvxProtectItem& rProtect = pFmt->GetProtect();
                if( rProtect.IsCntntProtected() )
                {
                    const SwFmtCntnt& rCntnt = pFmt->GetCntnt(FALSE);
                    ASSERT( rCntnt.GetCntntIdx(), "wo ist der SectionNode?" );
                    ULONG nIdx = rCntnt.GetCntntIdx()->GetIndex();
                    if( nSttIdx <= nIdx && nEndIdx >= nIdx )
                    {
                        // ist es keine gelinkte Section, dann kann sie auch
                        // nicht mitselektiert werden
                        const SwSection& rSect = *pFmt->GetSection();
                        if( CONTENT_SECTION == rSect.GetType() )
                        {
                            RestoreSavePos();
                            return TRUE;
                        }
                    }
                }
            }
        }

    }
// Ende
// neu: Bereiche ueberpruefen

    const SwNode* pNd = &GetPoint()->nNode.GetNode();
    if( pNd->IsCntntNode() && 0 == (SwUnoCrsr*)*this )
    {
        const SwCntntFrm* pFrm = ((SwCntntNode*)pNd)->GetFrm();
        if( pFrm && pFrm->IsValid() && 0 == pFrm->Frm().Height() &&
            0 != ( SELOVER_CHANGEPOS & eFlags ) )
        {
            // skip to the next / prev valida paragraph with a layout
            SwNodeIndex& rPtIdx = GetPoint()->nNode;
            int bGoNxt = pSavePos->nNode < rPtIdx.GetIndex();
            while( 0 != ( pFrm = ( bGoNxt ? pFrm->GetNextCntntFrm()
                                          : pFrm->GetPrevCntntFrm() )) &&
                    0 == pFrm->Frm().Height() )
                ;

            SwCntntNode* pCNd;
            if( pFrm && 0 != (pCNd = (SwCntntNode*)pFrm->GetNode()) )
            {
                // set this cntntNode as new position
                rPtIdx = *pCNd;
                pNd = pCNd;

                // ContentIndex noch anmelden:
                xub_StrLen nTmpPos = bGoNxt ? 0 : pCNd->Len();
                GetPoint()->nContent.Assign( pCNd, nTmpPos );

                    // sollten wir in einer Tabelle gelandet sein?
                if( IsInProtectTable( TRUE ) )
                    pFrm = 0;
            }
        }

        if( !pFrm )
        {
            DeleteMark();
            RestoreSavePos();
            return TRUE;        // ohne Frames geht gar nichts!
        }
    }

    // darf der Cursor in geschuetzen "Nodes" stehen?
    if( 0 == ( SELOVER_CHANGEPOS & eFlags ) && !IsAtValidPos() )
    {
        DeleteMark();
        RestoreSavePos();
        return TRUE;
    }

    if( !HasMark() )
        return FALSE;

    //JP 19.08.98: teste mal auf ungueltige Selektion - sprich ueber
    //              GrundSections:
    if( !::CheckNodesRange( GetMark()->nNode, GetPoint()->nNode, TRUE ))
    {
        DeleteMark();
        RestoreSavePos();
        return TRUE;        // ohne Frames geht gar nichts!
    }

    const SwTableNode* pPtNd = pNd->FindTableNode();

    if( (pNd = &GetMark()->nNode.GetNode())->IsCntntNode() &&
        !((SwCntntNode*)pNd)->GetFrm() && 0 == (SwUnoCrsr*)*this )
    {
        DeleteMark();
        RestoreSavePos();
        return TRUE;        // ohne Frames geht gar nichts!
    }

    const SwTableNode* pMrkNd = pNd->FindTableNode();

    // beide in keinem oder beide im gleichen TableNode
    if( ( !pMrkNd && !pPtNd ) || pPtNd == pMrkNd )
        return FALSE;

    // in unterschiedlichen Tabellen oder nur Mark in der Tabelle
    if( ( pPtNd && pMrkNd ) || pMrkNd )
    {                       // dann lasse das nicht zu, alte Pos zurueck
        RestoreSavePos();
        // Crsr bleibt an der alten Position
        return TRUE;
    }

    // ACHTUNG: dieses kann nicht im TableMode geschehen !!
    if( pPtNd )     // nur Point in Tabelle, dann gehe hinter/vor diese
    {
        if( SELOVER_CHANGEPOS & eFlags )
        {
            FASTBOOL bSelTop = GetPoint()->nNode.GetIndex() <
                    (( SELOVER_TOGGLE & eFlags ) ? pSavePos->nNode
                                                 : GetMark()->nNode.GetIndex());

            do {
                // in Schleife fuer Tabelle hinter Tabelle
                ULONG nSEIdx = pPtNd->EndOfSectionIndex();
                ULONG nSttEndTbl = nSEIdx + 1; // dflt. Sel. nach unten

                if( bSelTop )                               // Sel. nach oben
                    nSttEndTbl = rNds[ nSEIdx ]->StartOfSectionIndex() - 1;

                GetPoint()->nNode = nSttEndTbl;
                const SwNode* pNd = GetNode();

                if( pNd->IsSectionNode() || ( pNd->IsEndNode() &&
                    pNd->FindStartNode()->IsSectionNode() ) )
                {
                    // die lassen wir zu:
                    pNd = bSelTop
                        ? rNds.GoPrevSection( &GetPoint()->nNode,TRUE,FALSE )
                        : rNds.GoNextSection( &GetPoint()->nNode,TRUE,FALSE );

                    /* #i12312# Handle failure of Go{Prev|Next}Section */
                    if ( 0 == pNd)
                        break;

                    if( 0 != ( pPtNd = pNd->FindTableNode() ))
                        continue;
                }

                if( pNd->IsCntntNode() &&       // ist es ein ContentNode ??
                    ::CheckNodesRange( GetMark()->nNode,
                                       GetPoint()->nNode, TRUE ))
                {
                    // TABLE IN TABLE
                    const SwTableNode* pOuterTableNd = pNd->FindTableNode();
                    if ( pOuterTableNd )
                        pNd = pOuterTableNd;
                    else
                    {
                        SwCntntNode* pCNd = (SwCntntNode*)pNd;
                        xub_StrLen nTmpPos = bSelTop ? pCNd->Len() : 0;
                        GetPoint()->nContent.Assign( pCNd, nTmpPos );
                        return FALSE;
                    }
                }
                if( bSelTop
                    ? ( !pNd->IsEndNode() || 0 == ( pPtNd = pNd->FindTableNode() ))
                    : 0 == ( pPtNd = pNd->GetTableNode() ))
                    break;
            } while( TRUE );
        }

        // dann verbleibe auf der alten Position
        RestoreSavePos();
        return TRUE;        // Crsr bleibt an der alten Position
    }
    return FALSE;       // was bleibt noch ??
}

#if defined( UNX ) || defined( MAC )
#define IDX     (*pCellStt)
#else
#define IDX     aCellStt
#endif


FASTBOOL SwCursor::IsInProtectTable( FASTBOOL bMove, FASTBOOL bChgCrsr )
{
    // stehe ich in einer Tabelle ??
    SwCntntNode* pCNd = GetCntntNode();
    if( !pCNd || pSavePos->nNode == GetPoint()->nNode.GetIndex() ||
        !pCNd->FindTableNode() ||
        !pCNd->IsProtect() ||
        IsReadOnlyAvailable() )
        return FALSE;

    if( !bMove )
    {
        if( bChgCrsr )
            // dann verbleibe auf der alten Position
            RestoreSavePos();
        return TRUE;        // Crsr bleibt an der alten Position
    }

    // wir stehen in einer geschuetzten TabellenZelle
    // von Oben nach Unten Traveln ?
    if( pSavePos->nNode < GetPoint()->nNode.GetIndex() )
    {
        // suche die naechste "gueltige" Box

        // folgt nach dem EndNode der Zelle ein weiterer StartNode, dann
        // gibt es auch eine naechste Zelle
#if defined( UNX ) || defined( MAC )
        SwNodeIndex* pCellStt = new SwNodeIndex( *GetNode()->
                        FindTableBoxStartNode()->EndOfSectionNode(), 1 );
#else
        SwNodeIndex aCellStt( *GetNode()->FindTableBoxStartNode()->EndOfSectionNode(), 1 );
#endif
        FASTBOOL bProt = TRUE;
GoNextCell:
        do {
            if( !IDX.GetNode().IsStartNode() )
                break;
            IDX++;
            if( 0 == ( pCNd = IDX.GetNode().GetCntntNode() ))
                pCNd = IDX.GetNodes().GoNext( &IDX );
            if( 0 == ( bProt = pCNd->IsProtect() ))
                break;
            IDX.Assign( *pCNd->FindTableBoxStartNode()->EndOfSectionNode(), 1 );
        } while( bProt );

SetNextCrsr:
        if( !bProt )        // eine freie Zelle gefunden
        {
            GetPoint()->nNode = IDX;
#if defined( UNX ) || defined( MAC )
            delete pCellStt;
#endif
            SwCntntNode* pCNd = GetCntntNode();
            if( pCNd )
            {
                GetPoint()->nContent.Assign( pCNd, 0 );
                return FALSE;
            }
            return IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS );
        }
        // am Ende der Tabelle, also setze hinter diese
        IDX++;     // auf den naechsten Node
        SwNode* pNd;
        if( ( pNd = &IDX.GetNode())->IsEndNode() || HasMark())
        {
            // Tabelle allein in einem FlyFrame oder SSelection,
            // dann verbleibe auf der alten Position
            if( bChgCrsr )
                RestoreSavePos();
#if defined( UNX ) || defined( MAC )
            delete pCellStt;
#endif
            return TRUE;        // Crsr bleibt an der alten Position
        }
        else if( pNd->IsTableNode() && IDX++ )
            goto GoNextCell;

        bProt = FALSE;      // Index steht jetzt auf einem ContentNode
        goto SetNextCrsr;
    }

    // suche die vorherige "gueltige" Box
    {
        // liegt vor dem StartNode der Zelle ein weiterer EndNode, dann
        // gibt es auch eine vorherige Zelle
#if defined( UNX ) || defined( MAC )
        SwNodeIndex* pCellStt = new SwNodeIndex(
                    *GetNode()->FindTableBoxStartNode(), -1 );
#else
        SwNodeIndex aCellStt( *GetNode()->FindTableBoxStartNode(), -1 );
#endif
        SwNode* pNd;
        FASTBOOL bProt = TRUE;
GoPrevCell:
        do {
            if( !( pNd = &IDX.GetNode())->IsEndNode() )
                break;
            IDX.Assign( *pNd->StartOfSectionNode(), +1 );
            if( 0 == ( pCNd = IDX.GetNode().GetCntntNode() ))
                pCNd = pNd->GetNodes().GoNext( &IDX );
            if( 0 == ( bProt = pCNd->IsProtect() ))
                break;
            IDX.Assign( *pNd->FindTableBoxStartNode(), -1 );
        } while( bProt );

SetPrevCrsr:
        if( !bProt )        // eine freie Zelle gefunden
        {
            GetPoint()->nNode = IDX;
#if defined( UNX ) || defined( MAC )
            delete pCellStt;
#endif
            SwCntntNode* pCNd = GetCntntNode();
            if( pCNd )
            {
                GetPoint()->nContent.Assign( pCNd, 0 );
                return FALSE;
            }
            return IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS );
        }
        // am Start der Tabelle, also setze vor diese
        IDX--;     // auf den naechsten Node
        if( ( pNd = &IDX.GetNode())->IsStartNode() || HasMark() )
        {
            // Tabelle allein in einem FlyFrame oder Selektion,
            // dann verbleibe auf der alten Position
            if( bChgCrsr )
                RestoreSavePos();
#if defined( UNX ) || defined( MAC )
            delete pCellStt;
#endif
            return TRUE;        // Crsr bleibt an der alten Position
        }
        else if( pNd->StartOfSectionNode()->IsTableNode() && IDX-- )
            goto GoPrevCell;

        bProt = FALSE;      // Index steht jetzt auf einem ContentNode
        goto SetPrevCrsr;
    }

    ASSERT( FALSE, "sollte nie erreicht werden oder??" );
    return  FALSE;
}

// TRUE: an die Position kann der Cursor gesetzt werden
FASTBOOL SwCursor::IsAtValidPos( BOOL bPoint ) const
{
    const SwDoc* pDoc = GetDoc();
    const SwPosition* pPos = bPoint ? GetPoint() : GetMark();
    const SwNode* pNd = &pPos->nNode.GetNode();

    if( pNd->IsCntntNode() && !((SwCntntNode*)pNd)->GetFrm() &&
        0 == (const SwUnoCrsr*)*this )
        return FALSE;

        //JP 28.10.97: Bug 45129 - im UI-ReadOnly ist alles erlaubt
    if( !pDoc->GetDocShell() || !pDoc->GetDocShell()->IsReadOnlyUI() )
        return TRUE;

    BOOL bCrsrInReadOnly = IsReadOnlyAvailable();
    if( !bCrsrInReadOnly && pNd->IsProtect() )
        return FALSE;

    const SwSectionNode* pSectNd = pNd->FindSectionNode();
    if( pSectNd && (pSectNd->GetSection().IsHiddenFlag() ||
                    ( !bCrsrInReadOnly && pSectNd->GetSection().IsProtectFlag() )))
        return FALSE;

    return TRUE;
}

void SwCursor::SaveTblBoxCntnt( const SwPosition* ) {}

// setze den SRange fuer das Suchen im Dokument
SwMoveFnCollection* SwCursor::MakeFindRange( SwDocPositions nStart,
                                SwDocPositions nEnd, SwPaM* pRange ) const
{
    pRange->SetMark();
    FillFindPos( nStart, *pRange->GetMark() );
    FillFindPos( nEnd, *pRange->GetPoint() );

    // bestimme die Richtung, in der zu suchen ist
    // ( GetPoint > GetMark -> vorwaerts, sonst rueckwaerts )
    return ( DOCPOS_START == nStart || DOCPOS_OTHERSTART == nStart ||
              (DOCPOS_CURR == nStart &&
                (DOCPOS_END == nEnd || DOCPOS_OTHEREND == nEnd ) ))
                ? fnMoveForward : fnMoveBackward;
}


ULONG lcl_FindSelection( SwFindParas& rParas, SwCursor* pCurCrsr,
                        SwMoveFn fnMove, SwCursor*& pFndRing,
                        SwPaM& aRegion, FindRanges eFndRngs,
                        FASTBOOL bInReadOnly, BOOL& bCancel )
{
    SwDoc* pDoc = pCurCrsr->GetDoc();
    FASTBOOL bDoesUndo = pDoc->DoesUndo();
    int nFndRet = 0;
    ULONG nFound = 0;
    int bSrchBkwrd = fnMove == fnMoveBackward, bEnde = FALSE;
    SwPaM *pTmpCrsr = pCurCrsr, *pSaveCrsr = pCurCrsr;

    // nur beim ShellCrsr einen Prgogressbar erzeugen
    BOOL bIsUnoCrsr = 0 != (SwUnoCrsr*)*pCurCrsr;
    _PercentHdl* pPHdl = 0;
    USHORT nCrsrCnt = 0;
    if( FND_IN_SEL & eFndRngs )
    {
        while( pCurCrsr != ( pTmpCrsr = (SwPaM*)pTmpCrsr->GetNext() ))
            ++nCrsrCnt;
        if( nCrsrCnt && !bIsUnoCrsr )
            pPHdl = new _PercentHdl( 0, nCrsrCnt, pDoc->GetDocShell() );
    }

    do {
        aRegion.SetMark();
        // egal in welche Richtung, SPoint ist immer groesser als Mark,
        // wenn der Suchbereich gueltig ist !!
        SwPosition *pSttPos = aRegion.GetMark(),
                        *pEndPos = aRegion.GetPoint();
        *pSttPos = *pTmpCrsr->Start();
        *pEndPos = *pTmpCrsr->End();
        if( bSrchBkwrd )
            aRegion.Exchange();

        if( !nCrsrCnt && !pPHdl && !bIsUnoCrsr )
            pPHdl = new _PercentHdl( aRegion );

        // solange gefunden und nicht auf gleicher Position haengen bleibt
        while(  *pSttPos <= *pEndPos &&
                0 != ( nFndRet = rParas.Find( pCurCrsr, fnMove,
                                            &aRegion, bInReadOnly )) &&
                ( !pFndRing ||
                    *pFndRing->GetPoint() != *pCurCrsr->GetPoint() ||
                    *pFndRing->GetMark() != *pCurCrsr->GetMark() ))
        {
            if( !( FIND_NO_RING & nFndRet ))
            {
                // Bug 24084: Ring richtig herum aufbauen -> gleiche Mimik
                //            wie beim CreateCrsr !!!!

                SwCursor* pNew = pCurCrsr->Create( pFndRing );
                if( !pFndRing )
                    pFndRing = pNew;

                pNew->SetMark();
                *pNew->GetMark() = *pCurCrsr->GetMark();
            }

            ++nFound;

            if( !( eFndRngs & FND_IN_SELALL) )
            {
                bEnde = TRUE;
                break;
            }

            if( coSrchRplcThreshold == nFound && pDoc->DoesUndo()
                && rParas.IsReplaceMode())
            {
                short nRet = pCurCrsr->MaxReplaceArived();
                if( RET_YES == nRet )
                {
                    pDoc->DelAllUndoObj();
                    pDoc->DoUndo( FALSE );
                }
                else
                {
                    bEnde = TRUE;
                    if(RET_CANCEL == nRet)
                    {
                        bCancel = TRUE;
                        //unwind() ??
                    }
                    break;
                }
            }

            if( bSrchBkwrd )
                // bewege pEndPos vor den gefundenen Bereich
                *pEndPos = *pCurCrsr->Start();
            else
                // bewege pSttPos hinter den gefundenen Bereich
                *pSttPos = *pCurCrsr->End();

            if( *pSttPos == *pEndPos )      // im Bereich, aber am Ende
                break;                      // fertig

            if( !nCrsrCnt && !bIsUnoCrsr )
                pPHdl->NextPos( *aRegion.GetMark() );
        }

        if( bEnde || !( eFndRngs & ( FND_IN_SELALL | FND_IN_SEL )) )
            break;

        pTmpCrsr = ((SwPaM*)pTmpCrsr->GetNext());
        if( nCrsrCnt && !bIsUnoCrsr )
            pPHdl->NextPos( ++pPHdl->nActPos );

    } while( pTmpCrsr != pSaveCrsr );

    if( nFound && !pFndRing )       // falls kein Ring aufgebaut werden soll
        pFndRing = pCurCrsr->Create();

    delete pPHdl;
    pDoc->DoUndo( bDoesUndo );
    return nFound;
}


int lcl_MakeSelFwrd( const SwNode& rSttNd, const SwNode& rEndNd,
                        SwPaM& rPam, int bFirst )
{
    if( rSttNd.GetIndex() + 1 == rEndNd.GetIndex() )
        return FALSE;

    SwNodes& rNds = rPam.GetDoc()->GetNodes();
    rPam.DeleteMark();
    SwCntntNode* pCNd;
    if( !bFirst )
    {
        rPam.GetPoint()->nNode = rSttNd;
        pCNd = rNds.GoNext( &rPam.GetPoint()->nNode );
        if( !pCNd )
            return FALSE;
        pCNd->MakeStartIndex( &rPam.GetPoint()->nContent );
    }
    else if( rSttNd.GetIndex() > rPam.GetPoint()->nNode.GetIndex() ||
             rPam.GetPoint()->nNode.GetIndex() >= rEndNd.GetIndex() )
        return FALSE;       // steht nicht in dieser Section

    rPam.SetMark();
    rPam.GetPoint()->nNode = rEndNd;
    pCNd = rNds.GoPrevious( &rPam.GetPoint()->nNode );
    if( !pCNd )
        return FALSE;
    pCNd->MakeEndIndex( &rPam.GetPoint()->nContent );

    return *rPam.GetMark() < *rPam.GetPoint();
}


int lcl_MakeSelBkwrd( const SwNode& rSttNd, const SwNode& rEndNd,
                        SwPaM& rPam, int bFirst )
{
    if( rEndNd.GetIndex() + 1 == rSttNd.GetIndex() )
        return FALSE;

    SwNodes& rNds = rPam.GetDoc()->GetNodes();
    rPam.DeleteMark();
    SwCntntNode* pCNd;
    if( !bFirst )
    {
        rPam.GetPoint()->nNode = rSttNd;
        pCNd = rNds.GoPrevious( &rPam.GetPoint()->nNode );
        if( !pCNd )
            return FALSE;
        pCNd->MakeEndIndex( &rPam.GetPoint()->nContent );
    }
    else if( rEndNd.GetIndex() > rPam.GetPoint()->nNode.GetIndex() ||
             rPam.GetPoint()->nNode.GetIndex() >= rSttNd.GetIndex() )
        return FALSE;       // steht nicht in dieser Section

    rPam.SetMark();
    rPam.GetPoint()->nNode = rEndNd;
    pCNd = rNds.GoNext( &rPam.GetPoint()->nNode );
    if( !pCNd )
        return FALSE;
    pCNd->MakeStartIndex( &rPam.GetPoint()->nContent );

    return *rPam.GetPoint() < *rPam.GetMark();
}


// diese Methode "sucht" fuer alle Anwendungsfaelle, denn in SwFindParas
// steht immer die richtigen Parameter und die entsprechende Find-Methode

ULONG SwCursor::FindAll( SwFindParas& rParas,
                            SwDocPositions nStart, SwDocPositions nEnde,
                            FindRanges eFndRngs, BOOL& bCancel )
{
    bCancel = FALSE;
    SwCrsrSaveState aSaveState( *this );

    // Region erzeugen, ohne das diese in den Ring aufgenommen wird !
    SwPaM aRegion( *GetPoint() );
    SwMoveFn fnMove = MakeFindRange( nStart, nEnde, &aRegion );

    ULONG nFound = 0;
    int bMvBkwrd = fnMove == fnMoveBackward;
    FASTBOOL bInReadOnly = IsReadOnlyAvailable();

    SwCursor* pFndRing = 0;
    SwNodes& rNds = GetDoc()->GetNodes();

    // suche in Bereichen ?
    if( FND_IN_SEL & eFndRngs )
    {
        // String nicht im Bereich gefunden, dann erhalte alle Bereiche,
        // der Cursor beleibt unveraendert
        if( 0 == ( nFound = lcl_FindSelection( rParas, this, fnMove,
                                                pFndRing, aRegion, eFndRngs,
                                                bInReadOnly, bCancel ) ))
            return nFound;

        // der String wurde ein- bis mehrmals gefunden. Das steht alles
        // im neuen Crsr-Ring. Darum hebe erstmal den alten Ring auf
        while( GetNext() != this )
            delete GetNext();

        *GetPoint() = *pFndRing->GetPoint();
        SetMark();
        *GetMark() = *pFndRing->GetMark();
        pFndRing->MoveRingTo( this );
        delete pFndRing;
    }
    else if( FND_IN_OTHER & eFndRngs )
    {
        // Cursor als Kopie vom akt. und in den Ring aufnehmen
        // Verkettung zeigt immer auf den zuerst erzeugten, also vorwaerts
        SwCursor* pSav = Create( this );    // sicher den aktuellen Crsr

        // wenn schon ausserhalb vom Bodytext, suche von der Position,
        // ansonsten beginne mit der 1. GrundSection
        if( bMvBkwrd
            ? lcl_MakeSelBkwrd( rNds.GetEndOfExtras(),
                    *rNds.GetEndOfPostIts().StartOfSectionNode(),
                     *this, rNds.GetEndOfExtras().GetIndex() >=
                    GetPoint()->nNode.GetIndex() )
            : lcl_MakeSelFwrd( *rNds.GetEndOfPostIts().StartOfSectionNode(),
                    rNds.GetEndOfExtras(), *this,
                    rNds.GetEndOfExtras().GetIndex() >=
                    GetPoint()->nNode.GetIndex() ))
        {
            nFound = lcl_FindSelection( rParas, this, fnMove, pFndRing,
                                        aRegion, eFndRngs, bInReadOnly, bCancel );
        }

        if( !nFound )
        {
            // den alten wieder zurueck
            *GetPoint() = *pSav->GetPoint();
            if( pSav->HasMark() )
            {
                SetMark();
                *GetMark() = *pSav->GetMark();
            }
            else
                DeleteMark();
            return 0;
        }

        delete pSav;
        if( !( FND_IN_SELALL & eFndRngs ))
        {
            // es sollte nur einer gesucht werden, also fuege in dazu
            // egal in welche Richtung, SPoint ist immer groesser als Mark,
            // wenn der Suchbereich gueltig ist !!
            *GetPoint() = *pFndRing->GetPoint();
            SetMark();
            *GetMark() = *pFndRing->GetMark();
        }
        else
        {
            // es  wurde ein- bis mehrmals gefunden. Das steht alles
            // im neuen Crsr-Ring. Darum hebe erstmal den alten Ring auf
            while( GetNext() != this )
                delete GetNext();

            *GetPoint() = *pFndRing->GetPoint();
            SetMark();
            *GetMark() = *pFndRing->GetMark();
            pFndRing->MoveRingTo( this );
        }
        delete pFndRing;
    }
    else if( FND_IN_SELALL & eFndRngs )
    {
        SwCursor* pSav = Create( this );    // sicher den aktuellen Crsr

        const SwNode* pSttNd = ( FND_IN_BODYONLY & eFndRngs )
                            ? rNds.GetEndOfContent().StartOfSectionNode()
                            : rNds.GetEndOfPostIts().StartOfSectionNode();

        if( bMvBkwrd
            ? lcl_MakeSelBkwrd( rNds.GetEndOfContent(), *pSttNd,*this, FALSE )
            : lcl_MakeSelFwrd( *pSttNd, rNds.GetEndOfContent(), *this, FALSE ))
        {
            nFound = lcl_FindSelection( rParas, this, fnMove, pFndRing,
                                        aRegion, eFndRngs, bInReadOnly, bCancel );
        }

        if( !nFound )
        {
            // den alten wieder zurueck
            *GetPoint() = *pSav->GetPoint();
            if( pSav->HasMark() )
            {
                SetMark();
                *GetMark() = *pSav->GetMark();
            }
            else
                DeleteMark();
            return 0;
        }
        // es  wurde ein- bis mehrmals gefunden. Das steht alles
        // im neuen Crsr-Ring. Darum hebe erstmal den alten Ring auf
        delete pSav;

        while( GetNext() != this )
            delete GetNext();

        *GetPoint() = *pFndRing->GetPoint();
        SetMark();
        *GetMark() = *pFndRing->GetMark();
        pFndRing->MoveRingTo( this );
        delete pFndRing;
    }
    else
    {
        // ist ein GetMark gesetzt, dann wird bei gefundenem Object
        // der GetMark beibehalten !! Dadurch kann ein Bereich mit der Suche
        // aufgespannt werden.
        SwPosition aMarkPos( *GetMark() );
        int bMarkPos = HasMark() && !eFndRngs;

        if( 0 != (nFound = rParas.Find( this, fnMove,
                                        &aRegion, bInReadOnly ) ? 1 : 0)
            && bMarkPos )
            *GetMark() = aMarkPos;
    }

    if( nFound && SwCursor::IsSelOvr( SELOVER_TOGGLE ) )
        nFound = 0;
    return nFound;
}


void SwCursor::FillFindPos( SwDocPositions ePos, SwPosition& rPos ) const
{
    BOOL bIsStart = TRUE;
    SwCntntNode* pCNd = 0;
    SwNodes& rNds = GetDoc()->GetNodes();

    switch( ePos )
    {
    case DOCPOS_START:
        rPos.nNode = *rNds.GetEndOfContent().StartOfSectionNode();
        pCNd = rNds.GoNext( &rPos.nNode );
        break;

    case DOCPOS_END:
        rPos.nNode = rNds.GetEndOfContent();
        pCNd = rNds.GoPrevious( &rPos.nNode );
        bIsStart = FALSE;
        break;

    case DOCPOS_OTHERSTART:
        rPos.nNode = *rNds[ ULONG(0) ];
        pCNd = rNds.GoNext( &rPos.nNode );
        break;

    case DOCPOS_OTHEREND:
        rPos.nNode = *rNds.GetEndOfContent().StartOfSectionNode();
        pCNd = rNds.GoPrevious( &rPos.nNode );
        bIsStart = FALSE;
        break;

//  case DOCPOS_CURR:
    default:
        rPos = *GetPoint();
    }

    if( pCNd )
    {
        xub_StrLen nCPos = 0;
        if( !bIsStart )
            nCPos = pCNd->Len();
        rPos.nContent.Assign( pCNd, nCPos );
    }
}

short SwCursor::MaxReplaceArived()
{
    return RET_YES;
}


FASTBOOL SwCursor::IsStartWord() const
{
    return IsStartWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

FASTBOOL SwCursor::IsEndWord() const
{
    return IsEndWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

FASTBOOL SwCursor::IsInWord() const
{
    return IsEndWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

FASTBOOL SwCursor::GoStartWord()
{
    return GoStartWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

FASTBOOL SwCursor::GoEndWord()
{
    return GoEndWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

FASTBOOL SwCursor::GoNextWord()
{
    return GoNextWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

FASTBOOL SwCursor::GoPrevWord()
{
    return GoPrevWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

FASTBOOL SwCursor::SelectWord( const Point* pPt )
{
    return SelectWordWT( WordType::ANYWORD_IGNOREWHITESPACES, pPt );
}

FASTBOOL SwCursor::IsStartWordWT( sal_Int16 nWordType ) const
{
    FASTBOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->xBreak.is() )
    {
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        bRet = pBreakIt->xBreak->isBeginWord(
                            pTxtNd->GetTxt(), nPtPos,
                            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos )),
                            nWordType );
    }
    return bRet;
}

FASTBOOL SwCursor::IsEndWordWT( sal_Int16 nWordType ) const
{
    FASTBOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->xBreak.is() )
    {
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        bRet = pBreakIt->xBreak->isEndWord(
                            pTxtNd->GetTxt(), nPtPos,
                            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                            nWordType );

    }
    return bRet;
}

FASTBOOL SwCursor::IsInWordWT( sal_Int16 nWordType ) const
{
    FASTBOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->xBreak.is() )
    {
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        Boundary aBoundary = pBreakIt->xBreak->getWordBoundary(
                            pTxtNd->GetTxt(), nPtPos,
                            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                            nWordType,
                            TRUE );

        bRet = aBoundary.startPos != aBoundary.endPos &&
                aBoundary.startPos <= nPtPos &&
                    nPtPos <= aBoundary.endPos;
        if(bRet)
        {
            const CharClass& rCC = GetAppCharClass();
            bRet = rCC.isLetterNumeric( pTxtNd->GetTxt(), aBoundary.startPos );
        }
    }
    return bRet;
}

FASTBOOL SwCursor::GoStartWordWT( sal_Int16 nWordType )
{
    FASTBOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->xBreak.is() )
    {
        SwCrsrSaveState aSave( *this );
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        nPtPos = (xub_StrLen)pBreakIt->xBreak->getWordBoundary(
                            pTxtNd->GetTxt(), nPtPos,
                            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                            nWordType,
                            FALSE ).startPos;

        if( nPtPos < pTxtNd->GetTxt().Len() )
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = TRUE;
        }
    }
    return bRet;
}

FASTBOOL SwCursor::GoEndWordWT( sal_Int16 nWordType )
{
    FASTBOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->xBreak.is() )
    {
        SwCrsrSaveState aSave( *this );
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        nPtPos = (xub_StrLen)pBreakIt->xBreak->getWordBoundary(
                            pTxtNd->GetTxt(), nPtPos,
                            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                            nWordType,
                            TRUE ).endPos;

        if( nPtPos <= pTxtNd->GetTxt().Len() &&
            GetPoint()->nContent.GetIndex() != nPtPos )
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = TRUE;
        }
    }
    return bRet;
}

FASTBOOL SwCursor::GoNextWordWT( sal_Int16 nWordType )
{
    FASTBOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->xBreak.is() )
    {
        SwCrsrSaveState aSave( *this );
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();

        nPtPos = (xub_StrLen)pBreakIt->xBreak->nextWord(
                                pTxtNd->GetTxt(), nPtPos,
            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos, 1 ) ),
                    nWordType ).startPos;

        if( nPtPos < pTxtNd->GetTxt().Len() )
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = TRUE;
        }
    }
    return bRet;
}

FASTBOOL SwCursor::GoPrevWordWT( sal_Int16 nWordType )
{
    FASTBOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->xBreak.is() )
    {
        SwCrsrSaveState aSave( *this );
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        const xub_StrLen nPtStart = nPtPos;

        if( nPtPos )
            --nPtPos;
        nPtPos = (xub_StrLen)pBreakIt->xBreak->previousWord(
                                pTxtNd->GetTxt(), nPtStart,
            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos, 1 ) ),
                    nWordType ).startPos;

        if( nPtPos < pTxtNd->GetTxt().Len() )
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = TRUE;
        }
    }
    return bRet;
}

FASTBOOL SwCursor::SelectWordWT( sal_Int16 nWordType, const Point* pPt )
{
    SwCrsrSaveState aSave( *this );

    FASTBOOL bRet = FALSE;
    BOOL bForward = TRUE;
    DeleteMark();
    SwRootFrm* pLayout;
    if( pPt && 0 != (pLayout = GetDoc()->GetRootFrm()) )
    {
        // set the cursor to the layout position
        Point aPt( *pPt );
        pLayout->GetCrsrOfst( GetPoint(), aPt );
    }

    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->xBreak.is() )
    {
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        Boundary aBndry( pBreakIt->xBreak->getWordBoundary(
                            pTxtNd->GetTxt(), nPtPos,
                            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                            nWordType,
                            bForward ));

        if( aBndry.startPos != aBndry.endPos )
        {
            GetPoint()->nContent = (xub_StrLen)aBndry.endPos;
            if( !IsSelOvr() )
            {
                SetMark();
                GetMark()->nContent = (xub_StrLen)aBndry.startPos;
                if( !IsSelOvr() )
                    bRet = TRUE;
            }
        }
    }

    if( !bRet )
    {
        DeleteMark();
        RestoreSavePos();
    }
    return bRet;
}

//-----------------------------------------------------------------------------
FASTBOOL SwCursor::GoSentence( SentenceMoveType eMoveType )
{
    FASTBOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->xBreak.is() )
    {
        SwCrsrSaveState aSave( *this );
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        switch ( eMoveType )
        {
        case END_SENT:
            nPtPos = (xub_StrLen)pBreakIt->xBreak->endOfSentence(
                                    pTxtNd->GetTxt(),
                                    nPtPos, pBreakIt->GetLocale(
                                                pTxtNd->GetLang( nPtPos ) ));
            break;
        case NEXT_SENT:
            {
                String aTxt( pTxtNd->GetTxt() );
                nPtPos = (xub_StrLen)pBreakIt->xBreak->endOfSentence(
                                        aTxt,
                                        nPtPos, pBreakIt->GetLocale(
                                                    pTxtNd->GetLang( nPtPos ) ));
                while (nPtPos != (USHORT) -1 && ++nPtPos < aTxt.Len()
                       && aTxt.GetChar(nPtPos)== ' ' /*isWhiteSpace( aTxt.GetChar(nPtPos)*/ )
                    ;
                break;
            }
        case START_SENT:
            nPtPos = (xub_StrLen)pBreakIt->xBreak->beginOfSentence(
                                    pTxtNd->GetTxt(),
                                    nPtPos, pBreakIt->GetLocale(
                                            pTxtNd->GetLang( nPtPos ) ));
            break;
        case PREV_SENT:
            nPtPos = (xub_StrLen)pBreakIt->xBreak->beginOfSentence(
                                    pTxtNd->GetTxt(),
                                    nPtPos, pBreakIt->GetLocale(
                                                pTxtNd->GetLang( nPtPos ) ));
            if (nPtPos == 0)
                return FALSE;   // the previous sentence is not in this paragraph
            if (nPtPos > 0)
                nPtPos = (xub_StrLen)pBreakIt->xBreak->beginOfSentence(
                                    pTxtNd->GetTxt(),
                                    nPtPos - 1, pBreakIt->GetLocale(
                                                pTxtNd->GetLang( nPtPos ) ));
            break;
        }

        // it is allowed to place the PaM just behind the last
        // character in the text thus <= ...Len
        if( nPtPos <= pTxtNd->GetTxt().Len() )
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = TRUE;
        }
    }
    return bRet;
}


FASTBOOL SwCursor::LeftRight( BOOL bLeft, USHORT nCnt, USHORT nMode,
                              BOOL bVisualAllowed,BOOL bSkipHidden, BOOL bInsertCrsr )
{
    SwTableCursor* pTblCrsr = (SwTableCursor*)*this;
    if( pTblCrsr )
        return bLeft ? pTblCrsr->GoPrevCell( nCnt )
                     : pTblCrsr->GoNextCell( nCnt );

    // calculate cursor bidi level
    const SwCntntFrm* pSttFrm = NULL;
    SwNode& rNode = GetPoint()->nNode.GetNode();
    const BOOL bDoNotSetBidiLevel = 0 != (SwUnoCrsr*)*this;

    if ( !bDoNotSetBidiLevel )
    {
        if( rNode.IsTxtNode() )
        {
            const SwTxtNode& rTNd = *rNode.GetTxtNode();
            SwIndex& rIdx = GetPoint()->nContent;
            xub_StrLen nPos = rIdx.GetIndex();

            SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();
            if ( bVisualAllowed && rCTLOptions.IsCTLFontEnabled() &&
                 SvtCTLOptions::MOVEMENT_VISUAL ==
                 rCTLOptions.GetCTLCursorMovement() )
            {
                // for visual cursor travelling (used in bidi layout)
                // we first have to convert the logic to a visual position
                Point aPt;
                pSttFrm = rTNd.GetFrm( &aPt, GetPoint() );
                if( pSttFrm )
                {
                    BYTE nCrsrLevel = GetCrsrBidiLevel();
                    sal_Bool bForward = ! bLeft;
                    ((SwTxtFrm*)pSttFrm)->PrepareVisualMove( nPos, nCrsrLevel,
                                                             bForward, bInsertCrsr );
                    rIdx = nPos;
                    SetCrsrBidiLevel( nCrsrLevel );
                    bLeft = ! bForward;
                }
            }
            else
            {
                const SwScriptInfo* pSI = SwScriptInfo::GetScriptInfo( rTNd );
                if ( pSI )
                {
                    const xub_StrLen nMoveOverPos = bLeft ?
                                                   ( nPos ? nPos - 1 : 0 ) :
                                                    nPos;
                    SetCrsrBidiLevel( pSI->DirType( nMoveOverPos ) );
                }
            }
        }
    }

    // kann der Cursor n-mal weiterverschoben werden ?
    SwCrsrSaveState aSave( *this );
    SwMoveFn fnMove = bLeft ? fnMoveBackward : fnMoveForward;

    SwGoInDoc fnGo;
    if ( bSkipHidden )
        fnGo = CRSR_SKIP_CELLS == nMode ? fnGoCntntCellsSkipHidden : fnGoCntntSkipHidden;
    else
        fnGo = CRSR_SKIP_CELLS == nMode ? fnGoCntntCells : fnGoCntnt;

    while( nCnt && Move( fnMove, fnGo ) )
        --nCnt;

    // here come some special rules for visual cursor travelling
    if ( pSttFrm )
    {
        SwNode& rTmpNode = GetPoint()->nNode.GetNode();
        if ( &rTmpNode != &rNode && rTmpNode.IsTxtNode() )
        {
            Point aPt;
            const SwCntntFrm* pEndFrm = ((SwTxtNode&)rTmpNode).GetFrm( &aPt, GetPoint() );
            if ( pEndFrm )
            {
                if ( ! pEndFrm->IsRightToLeft() != ! pSttFrm->IsRightToLeft() )
                {
                    if ( ! bLeft )
                        pEndFrm->RightMargin( this );
                    else
                        pEndFrm->LeftMargin( this );
                }
            }
        }
    }

    return 0 == nCnt && !IsInProtectTable( TRUE ) &&
            !IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS );
}

FASTBOOL SwCursor::UpDown( BOOL bUp, USHORT nCnt,
                            Point* pPt, long nUpDownX )
{
    SwTableCursor* pTblCrsr = (SwTableCursor*)*this;
    sal_Bool bAdjustTableCrsr = sal_False;

    // vom Tabellen Crsr Point/Mark in der gleichen Box ??
    // dann stelle den Point an den Anfang der Box
    if( pTblCrsr && GetNode( TRUE )->FindStartNode() ==
                    GetNode( FALSE )->FindStartNode() )
    {
        if ( End() != GetPoint() )
            Exchange();
        bAdjustTableCrsr = sal_True;
    }

    FASTBOOL bRet = FALSE;
    Point aPt;
    if( pPt )
        aPt = *pPt;
    SwCntntFrm* pFrm = GetCntntNode()->GetFrm( &aPt, GetPoint() );

    if( pFrm )
    {
        SwCrsrSaveState aSave( *this );

        if( !pPt )
        {
            SwRect aTmpRect;
            pFrm->GetCharRect( aTmpRect, *GetPoint() );
            aPt = aTmpRect.Pos();

            nUpDownX = pFrm->IsVertical() ?
                aPt.Y() - pFrm->Frm().Top() :
                aPt.X() - pFrm->Frm().Left();
        }

        // Bei Fussnoten ist auch die Bewegung in eine andere Fussnote erlaubt.
        // aber keine Selection!!
        const FASTBOOL bChkRange = pFrm->IsInFtn() && !HasMark()
                                    ? FALSE : TRUE;
        const SwPosition aOldPos( *GetPoint() );
        BOOL bInReadOnly = IsReadOnlyAvailable();

        if ( bAdjustTableCrsr && !bUp )
        {
            // Special case: We have a table cursor but the start box
            // has more than one paragraph. If we want to go down, we have to
            // set the point to the last frame in the table box. This is
            // only necessary if we do not already have a table selection
            const SwStartNode* pTblNd = GetNode( TRUE )->FindTableBoxStartNode();
            ASSERT( pTblNd, "pTblCrsr without SwTableNode?" )

            if ( pTblNd ) // safety first
            {
                const SwNode* pEndNd = pTblNd->EndOfSectionNode();
                GetPoint()->nNode = *pEndNd;
                pTblCrsr->Move( fnMoveBackward, fnGoNode );
                   pFrm = GetCntntNode()->GetFrm( &aPt, GetPoint() );
            }
        }

        while( nCnt &&
               (bUp ? pFrm->UnitUp( this, nUpDownX, bInReadOnly )
                    : pFrm->UnitDown( this, nUpDownX, bInReadOnly ) ) &&
                CheckNodesRange( aOldPos.nNode, GetPoint()->nNode, bChkRange ))
        {
               pFrm = GetCntntNode()->GetFrm( &aPt, GetPoint() );
            --nCnt;
        }

        if( !nCnt && !IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS ) )  // die gesamte Anzahl durchlaufen ?
        {
            if( !pTblCrsr )
            {
                // dann versuche den Cursor auf die Position zu setzen,
                // auf halber Heohe vom Char-Rectangle
                pFrm = GetCntntNode()->GetFrm( &aPt, GetPoint() );
                SwCrsrMoveState eTmpState( MV_UPDOWN );
                eTmpState.bSetInReadOnly = bInReadOnly;
                SwRect aTmpRect;
                pFrm->GetCharRect( aTmpRect, *GetPoint(), &eTmpState );
#ifdef VERTICAL_LAYOUT
                if ( pFrm->IsVertical() )
                {
                    aPt.X() = aTmpRect.Center().X();
                    pFrm->Calc();
                    aPt.Y() = pFrm->Frm().Top() + nUpDownX;
                }
                else
                {
                    aPt.Y() = aTmpRect.Center().Y();
                    pFrm->Calc();
                    aPt.X() = pFrm->Frm().Left() + nUpDownX;
                }
#else
                aPt.Y() = aTmpRect.Center().Y();
                pFrm->Calc();
                aPt.X() = pFrm->Frm().Left() + nUpDownX;
#endif
                pFrm->GetCrsrOfst( GetPoint(), aPt, &eTmpState );
            }
            bRet = TRUE;
        }
        else
            *GetPoint() = aOldPos;

        // calculate cursor bidi level
        const BOOL bDoNotSetBidiLevel = 0 != (SwUnoCrsr*)*this;

        if ( ! bDoNotSetBidiLevel )
        {
            SwNode& rNode = GetPoint()->nNode.GetNode();
            if ( rNode.IsTxtNode() )
            {
                const SwScriptInfo* pSI = SwScriptInfo::GetScriptInfo( (SwTxtNode&)rNode );
                if ( pSI )
                {
                    SwIndex& rIdx = GetPoint()->nContent;
                    xub_StrLen nPos = rIdx.GetIndex();

                    if( nPos && nPos < ((SwTxtNode&)rNode).GetTxt().Len() )
                    {
                        const BYTE nCurrLevel = pSI->DirType( nPos );
                        const BYTE nPrevLevel = pSI->DirType( nPos - 1 );

                        if ( nCurrLevel % 2 != nPrevLevel % 2 )
                        {
                            // set cursor level to the lower of the two levels
                            SetCrsrBidiLevel( Min( nCurrLevel, nPrevLevel ) );
                        }
                        else
                            SetCrsrBidiLevel( nCurrLevel );
                    }
                }
            }
        }

        // #i27615#
        if (GetPoint()->nContent.GetIndex() != 0)
            SetInFrontOfLabel(FALSE);
    }

    return bRet;
}

FASTBOOL SwCursor::LeftRightMargin( BOOL bLeft, BOOL bAPI )
{
    Point aPt;
    SwCntntFrm * pFrm = GetCntntNode()->GetFrm( &aPt, GetPoint() );
    FASTBOOL bRet;

    // calculate cursor bidi level
    if ( pFrm )
        SetCrsrBidiLevel( pFrm->IsRightToLeft() ? 1 : 0 );

    // #i27615# Manage cursor in front of label.
    if (pFrm)
    {
        if (bLeft)
        {
            FASTBOOL bWasAtLeftMargin = IsAtLeftRightMargin(TRUE, bAPI);
            bRet = pFrm->LeftMargin( this );

            if (! bAPI && bWasAtLeftMargin)
                SetInFrontOfLabel(TRUE);
        }
        else
        {
            bRet = pFrm->RightMargin( this, bAPI );

            if (! bAPI )
                SetInFrontOfLabel(FALSE);
        }
    }

    return bRet;
}

FASTBOOL SwCursor::IsAtLeftRightMargin( BOOL bLeft, BOOL bAPI ) const
{
    FASTBOOL bRet = FALSE;
    Point aPt;
    SwCntntFrm * pFrm = GetCntntNode()->GetFrm( &aPt, GetPoint() );
    if( pFrm )
    {
        SwPaM aPam( *GetPoint() );
        if( !bLeft && aPam.GetPoint()->nContent.GetIndex() )
            aPam.GetPoint()->nContent--;
        bRet = (bLeft ? pFrm->LeftMargin( &aPam )
                      : pFrm->RightMargin( &aPam, bAPI ))
                && *aPam.GetPoint() == *GetPoint();
    }
    return bRet;
}

FASTBOOL SwCursor::SttEndDoc( BOOL bStt )
{
    SwCrsrSaveState aSave( *this );

    // Springe beim Selektieren nie ueber Section-Grenzen !!
    // kann der Cursor weiterverschoben werden ?
    SwMoveFn fnMove = bStt ? fnMoveBackward : fnMoveForward;
    FASTBOOL bRet = (!HasMark() || !IsNoCntnt() ) &&
                    Move( fnMove, fnGoDoc ) &&
                    !IsInProtectTable( TRUE ) &&
                    !IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS |
                                SELOVER_ENABLEREVDIREKTION );

    return bRet;
}

FASTBOOL SwCursor::GoPrevNextCell( BOOL bNext, USHORT nCnt )
{
    const SwTableNode* pTblNd = GetPoint()->nNode.GetNode().FindTableNode();
    if( !pTblNd )
        return FALSE;

    // liegt vor dem StartNode der Cell ein weiterer EndNode, dann
    // gibt es auch eine vorherige Celle
    SwCrsrSaveState aSave( *this );
    SwNodeIndex& rPtIdx = GetPoint()->nNode;
    if( bNext )
    {
        while( nCnt-- )
        {
            SwNodeIndex aCellIdx( *rPtIdx.GetNode().FindTableBoxStartNode()->
                                    EndOfSectionNode(), 1 );
            if( !aCellIdx.GetNode().IsStartNode() )
                return FALSE;
            rPtIdx = aCellIdx;
        }
    }
    else
    {
        while( nCnt-- )
        {
            SwNodeIndex aCellIdx( *rPtIdx.GetNode().FindTableBoxStartNode(),-1);
            if( !aCellIdx.GetNode().IsEndNode() )
                return FALSE;

            rPtIdx = *aCellIdx.GetNode().StartOfSectionNode();
        }
    }

    rPtIdx++;
    if( !rPtIdx.GetNode().IsCntntNode() )
        GetDoc()->GetNodes().GoNextSection( &rPtIdx, TRUE, FALSE );
    GetPoint()->nContent.Assign( GetCntntNode(), 0 );

    return !IsInProtectTable( TRUE );
}

FASTBOOL SwCursor::GotoTable( const String& rName )
{
    FASTBOOL bRet = FALSE;
    // Tabellenselektion oder ueberhaupt Selection ?
    // Das ist eine ungueltige Action !
    if( !(SwTableCursor*)*this && !HasMark() )
    {
        SwTable* pTmpTbl = SwTable::FindTable( GetDoc()->FindTblFmtByName( rName ) );
        if( pTmpTbl )
        {
            // eine Tabelle im normalen NodesArr
            SwCrsrSaveState aSave( *this );
            GetPoint()->nNode = *pTmpTbl->GetTabSortBoxes()[ 0 ]->
                                GetSttNd()->FindTableNode();
            Move( fnMoveForward, fnGoCntnt );
            bRet = !IsSelOvr();
        }
    }
    return bRet;
}

FASTBOOL SwCursor::GotoTblBox( const String& rName )
{
    FASTBOOL bRet = FALSE;
    const SwTableNode* pTblNd = GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        // erfrage die Box, mit dem Nanen
        const SwTableBox* pTblBox = pTblNd->GetTable().GetTblBox( rName );
        if( pTblBox && pTblBox->GetSttNd() &&
            ( !pTblBox->GetFrmFmt()->GetProtect().IsCntntProtected() ||
              IsReadOnlyAvailable() ) )
        {
            SwCrsrSaveState aSave( *this );
            GetPoint()->nNode = *pTblBox->GetSttNd();
            Move( fnMoveForward, fnGoCntnt );
            bRet = !IsSelOvr();
        }
    }
    return bRet;
}

FASTBOOL SwCursor::MovePara(SwWhichPara fnWhichPara, SwPosPara fnPosPara )
{
    //JP 28.8.2001: for optimization test something before
    const SwNode* pNd;
    if( fnWhichPara == fnParaCurr ||
        (( pNd = &GetPoint()->nNode.GetNode())->IsTxtNode() &&
          pNd->GetNodes()[ pNd->GetIndex() +
                    (fnWhichPara == fnParaNext ? 1 : -1 ) ]->IsTxtNode() ) )
    {
        return (*fnWhichPara)( *this, fnPosPara );
    }
    // else we must use the SaveStructure, because the next/prev is not
    // a same node type.
    SwCrsrSaveState aSave( *this );
    return (*fnWhichPara)( *this, fnPosPara ) &&
            !IsInProtectTable( TRUE ) &&
            !IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS );
}


FASTBOOL SwCursor::MoveSection( SwWhichSection fnWhichSect,
                                SwPosSection fnPosSect)
{
    SwCrsrSaveState aSave( *this );
    return (*fnWhichSect)( *this, fnPosSect ) &&
            !IsInProtectTable( TRUE ) &&
            !IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS );
}

/*
    FASTBOOL MoveTable( SwWhichTable, SwPosTable );
    FASTBOOL MoveColumn( SwWhichColumn, SwPosColumn );
    FASTBOOL MoveRegion( SwWhichRegion, SwPosRegion );
*/

void SwCursor::RestoreSavePos()     // Point auf die SavePos setzen
{
    if( pSavePos )
    {
        GetPoint()->nNode = pSavePos->nNode;
        GetPoint()->nContent.Assign( GetCntntNode(), pSavePos->nCntnt );
    }
}


/*  */

SwTableCursor::SwTableCursor( const SwPosition &rPos, SwPaM* pRing )
    : SwCursor( rPos, pRing )
{
    bParked = FALSE;
    bChg = FALSE;
    nTblPtNd = 0, nTblMkNd = 0;
    nTblPtCnt = 0, nTblMkCnt = 0;
}

SwTableCursor::~SwTableCursor() {}

SwTableCursor::operator SwTableCursor* () { return this; }

BOOL lcl_SeekEntry( const SwSelBoxes& rTmp, const SwStartNode* pSrch, USHORT& rFndPos )
{
    ULONG nIdx = pSrch->GetIndex();

    register USHORT nO = rTmp.Count(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            if( rTmp[ nM ]->GetSttNd() == pSrch )
            {
                rFndPos = nM;
                return TRUE;
            }
            else if( rTmp[ nM ]->GetSttIdx() < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
                return FALSE;
            else
                nO = nM - 1;
        }
    }
    return FALSE;
}


SwCursor* SwTableCursor::MakeBoxSels( SwCursor* pAktCrsr )
{
    if( bChg )      // ???
    {
        if( bParked )
        {
            // wieder in den Inhalt schieben
            Exchange();
            Move( fnMoveForward );
            Exchange();
            Move( fnMoveForward );
            bParked = FALSE;
        }

        bChg = FALSE;

        // temp Kopie anlegen, damit alle Boxen, fuer die schon Cursor
        // existieren, entfernt werden koennen.
        SwSelBoxes aTmp;
        aTmp.Insert( &aSelBoxes );

        //Jetzt die Alten und die neuen abgleichen.
        SwNodes& rNds = pAktCrsr->GetDoc()->GetNodes();
        USHORT nPos;
        const SwStartNode* pSttNd;
        SwPaM* pCur = pAktCrsr;
        do {
            BOOL bDel = FALSE;
            pSttNd = pCur->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
            if( !pCur->HasMark() || !pSttNd ||
                pSttNd != pCur->GetMark()->nNode.GetNode().FindTableBoxStartNode() )
                bDel = TRUE;

            else if( lcl_SeekEntry( aTmp, pSttNd, nPos ))
            {
                SwNodeIndex aIdx( *pSttNd, 1 );
                const SwNode* pNd = &aIdx.GetNode();
                if( !pNd->IsCntntNode() )
                    pNd = rNds.GoNextSection( &aIdx, TRUE, FALSE );

                SwPosition* pPos = pCur->GetMark();
                if( pNd != &pPos->nNode.GetNode() )
                    pPos->nNode = *pNd;
                pPos->nContent.Assign( (SwCntntNode*)pNd, 0 );

                aIdx.Assign( *pSttNd->EndOfSectionNode(), - 1 );
                if( !( pNd = &aIdx.GetNode())->IsCntntNode() )
                    pNd = rNds.GoPrevSection( &aIdx, TRUE, FALSE );

                pPos = pCur->GetPoint();
                if( pNd != &pPos->nNode.GetNode() )
                    pPos->nNode = *pNd;
                pPos->nContent.Assign( (SwCntntNode*)pNd, ((SwCntntNode*)pNd)->Len() );

                aTmp.Remove( nPos );
            }
            else
                bDel = TRUE;

            pCur = (SwPaM*)pCur->GetNext();
            if( bDel )
            {
                SwPaM* pDel = (SwPaM*)pCur->GetPrev();
/*
JP 20.07.98: der alte Code geht mit dem UNO-TableCrsr nicht
                if( pDel == pAktCrsr )
                {
                    if( pAktCrsr->GetNext() == pAktCrsr )
                    {
                        pAktCrsr->DeleteMark();
                        break;      // es gibt nichts mehr zu loeschen!
                    }
                    pAktCrsr = (SwCursor*)pDel->GetPrev();
                }
                delete pDel;
*/

                if( pDel == pAktCrsr )
                    pAktCrsr->DeleteMark();
                else
                    delete pDel;
            }
        } while ( pAktCrsr != pCur );

        for( nPos = 0; nPos < aTmp.Count(); ++nPos )
        {
            pSttNd = aTmp[ nPos ]->GetSttNd();

            SwNodeIndex aIdx( *pSttNd, 1 );
            const SwNode* pNd = &aIdx.GetNode();
            if( !pNd->IsCntntNode() )
                pNd = rNds.GoNextSection( &aIdx, TRUE, FALSE );

            SwPaM* pNew;
            if( pAktCrsr->GetNext() == pAktCrsr && !pAktCrsr->HasMark() )
            {
                pNew = pAktCrsr;
                pNew->GetPoint()->nNode = *pNd;
                pNew->GetPoint()->nContent.Assign( (SwCntntNode*)pNd, 0 );
            }
            else
            {
                pNew = pAktCrsr->Create( pAktCrsr );
                pNew->GetPoint()->nNode = *pNd;
                pNew->GetPoint()->nContent.Assign( (SwCntntNode*)pNd, 0 );
            }
            pNew->SetMark();

            SwPosition* pPos = pNew->GetPoint();
            pPos->nNode.Assign( *pSttNd->EndOfSectionNode(), - 1 );
            if( !( pNd = &pPos->nNode.GetNode())->IsCntntNode() )
                pNd = rNds.GoPrevSection( &pPos->nNode, TRUE, FALSE );

            pPos->nContent.Assign( (SwCntntNode*)pNd, ((SwCntntNode*)pNd)->Len() );
        }
    }
    return pAktCrsr;
}


void SwTableCursor::InsertBox( const SwTableBox& rTblBox )
{
    SwTableBox* pBox = (SwTableBox*)&rTblBox;
    aSelBoxes.Insert( pBox );
    bChg = TRUE;
}

FASTBOOL SwTableCursor::IsCrsrMovedUpdt()
{
    if( !IsCrsrMoved() )
        return FALSE;

    nTblMkNd = GetMark()->nNode.GetIndex();
    nTblPtNd = GetPoint()->nNode.GetIndex();
    nTblMkCnt = GetMark()->nContent.GetIndex();
    nTblPtCnt = GetPoint()->nContent.GetIndex();
    return TRUE;
}


// Parke den Tabellen-Cursor auf dem StartNode der Boxen.
void SwTableCursor::ParkCrsr()
{
    // Index aus dem TextNode abmelden
    SwNode* pNd = &GetPoint()->nNode.GetNode();
    if( !pNd->IsStartNode() )
        pNd = pNd->StartOfSectionNode();
    GetPoint()->nNode = *pNd;
    GetPoint()->nContent.Assign( 0, 0 );

    pNd = &GetMark()->nNode.GetNode();
    if( !pNd->IsStartNode() )
        pNd = pNd->StartOfSectionNode();
    GetMark()->nNode = *pNd;
    GetMark()->nContent.Assign( 0, 0 );

    bChg = TRUE;
    bParked = TRUE;
}


FASTBOOL SwTableCursor::HasReadOnlyBoxSel() const
{
    FASTBOOL bRet = FALSE;
    for( USHORT n = aSelBoxes.Count(); n;  )
        if( aSelBoxes[ --n ]->GetFrmFmt()->GetProtect().IsCntntProtected() )
        {
            bRet = TRUE;
            break;
        }
    return bRet;
}


