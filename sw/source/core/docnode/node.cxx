/*************************************************************************
 *
 *  $RCSfile: node.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 16:36:58 $
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

#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HDL_
#include <com/sun/star/i18n/CharacterIteratorMode.hdl>
#endif

#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FTNFRM_HXX //autogen
#include <ftnfrm.hxx>
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
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWCACHE_HXX
#include <swcache.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _LAYFRM_HXX
#include <layfrm.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>  // SwTabFrm
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _FMTFTN_HXX
#include <fmtftn.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _PAGEFRM_HXX //autogen
#include <pagefrm.hxx>
#endif
#ifndef _NODE2LAY_HXX
#include <node2lay.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif

using namespace ::com::sun::star::i18n;

TYPEINIT2( SwCntntNode, SwModify, SwIndexReg )

/*******************************************************************
|*
|*  SwNode::GetSectionLevel
|*
|*  Beschreibung
|*      Die Funktion liefert den Sectionlevel an der durch
|*      aIndex bezeichneten Position.
|*
|*      Die Logik ist wie folgt:   ( S -> Start, E -> End, C -> CntntNode)
|*          Level   0       E
|*                  1   S  E
|*                  2    SC
|*
|*      alle EndNodes der GrundSection haben den Level 0
|*      alle StartNodes der GrundSection haben den Level 1
|*
|*  Ersterstellung
|*      VER0100 vb 901214
|*
|*  Aenderung:  JP  11.08.93
|*      keine Rekursion mehr !!
|*
*******************************************************************/


USHORT SwNode::GetSectionLevel() const
{
    // EndNode einer Grund-Section ?? diese sind immer 0 !!
    if( IsEndNode() && 0 == pStartOfSection->StartOfSectionIndex() )
        return 0;

    USHORT nLevel;
    const SwNode* pNode = IsStartNode() ? this : pStartOfSection;
    for( nLevel = 1; 0 != pNode->StartOfSectionIndex(); ++nLevel )
        pNode = pNode->pStartOfSection;
    return IsEndNode() ? nLevel-1 : nLevel;
}

/*******************************************************************
|*
|*  SwNode::SwNode
|*
|*  Beschreibung
|*      Konstruktor; dieser fuegt einen Node in das Array rNodes
|*      an der Position rWhere ein. Dieser bekommt als
|*      theEndOfSection den EndOfSection-Index des Nodes
|*      unmittelbar vor ihm. Falls er sich an der Position 0
|*      innerhalb des variablen Arrays befindet, wird
|*      theEndOfSection 0 (der neue selbst).
|*
|*  Parameter
|*      IN
|*      rNodes bezeichnet das variable Array, in das der Node
|*      eingefuegt werden soll
|*      IN
|*      rWhere bezeichnet die Position innerhalb dieses Arrays,
|*      an der der Node eingefuegt werden soll
|*
|*  Ersterstellung
|*      VER0100 vb 901214
|*
|*  Stand
|*      VER0100 vb 901214
|*
*******************************************************************/


SwNode::SwNode( const SwNodeIndex &rWhere, const BYTE nNdType )
    : pStartOfSection( 0 ), nNodeType( nNdType )
{
    bWrongDirty = bACmplWrdDirty = TRUE;
    bSetNumLSpace = bIgnoreDontExpand = FALSE;
    nAFmtNumLvl = 0;

    SwNodes& rNodes = (SwNodes&)rWhere.GetNodes();
    SwNode* pInsNd = this;      // der MAC kann this nicht einfuegen !!
    if( rWhere.GetIndex() )
    {
        SwNode* pNd = rNodes[ rWhere.GetIndex() -1 ];
        rNodes.Insert( pInsNd, rWhere );
        if( 0 == ( pStartOfSection = pNd->GetStartNode()) )
        {
            pStartOfSection = pNd->pStartOfSection;
            if( pNd->GetEndNode() )     // EndNode ? Section ueberspringen!
            {
                pNd = pStartOfSection;
                pStartOfSection = pNd->pStartOfSection;
            }
        }
    }
    else
    {
        rNodes.Insert( pInsNd, rWhere );
        pStartOfSection = (SwStartNode*)this;
    }
}

SwNode::SwNode( SwNodes& rNodes, ULONG nPos, const BYTE nNdType )
    : pStartOfSection( 0 ), nNodeType( nNdType )
{
    bWrongDirty = bACmplWrdDirty = TRUE;
    bSetNumLSpace = bIgnoreDontExpand = FALSE;
    nAFmtNumLvl = 0;

    SwNode* pInsNd = this;      // der MAC kann this nicht einfuegen !!
    if( nPos )
    {
        SwNode* pNd = rNodes[ nPos - 1 ];
        rNodes.Insert( pInsNd, nPos );
        if( 0 == ( pStartOfSection = pNd->GetStartNode()) )
        {
            pStartOfSection = pNd->pStartOfSection;
            if( pNd->GetEndNode() )     // EndNode ? Section ueberspringen!
            {
                pNd = pStartOfSection;
                pStartOfSection = pNd->pStartOfSection;
            }
        }
    }
    else
    {
        rNodes.Insert( pInsNd, nPos );
        pStartOfSection = (SwStartNode*)this;
    }
}

SwNode::~SwNode()
{
}

// suche den TabellenNode, in dem dieser steht. Wenn in keiner
// Tabelle wird 0 returnt.


SwTableNode* SwNode::FindTableNode()
{
    if( IsTableNode() )
        return GetTableNode();
    SwStartNode* pTmp = pStartOfSection;
    while( !pTmp->IsTableNode() && pTmp->GetIndex() )
#if defined( ALPHA ) && defined( UNX )
        pTmp = ((SwNode*)pTmp)->pStartOfSection;
#else
        pTmp = pTmp->pStartOfSection;
#endif
    return pTmp->GetTableNode();
}


// liegt der Node im Sichtbarenbereich der Shell ?
BOOL SwNode::IsVisible( ViewShell* pSh ) const
{
    BOOL bRet = FALSE;
    const SwCntntNode* pNd;

    if( ND_STARTNODE & nNodeType )
    {
        SwNodeIndex aIdx( *this );
        pNd = GetNodes().GoNext( &aIdx );
    }
    else if( ND_ENDNODE & nNodeType )
    {
        SwNodeIndex aIdx( *EndOfSectionNode() );
        pNd = GetNodes().GoPrevious( &aIdx );
    }
    else
        pNd = GetCntntNode();

    const SwFrm* pFrm;
    if( pNd && 0 != ( pFrm = pNd->GetFrm( 0, 0, FALSE ) ) )
    {
        if( !pSh )
            // dann die Shell vom Doc besorgen:
            GetDoc()->GetEditShell( &pSh );

        if( pSh )
        {
            if ( pFrm->IsInTab() )
                pFrm = pFrm->FindTabFrm();

            if( !pFrm->IsValid() )
                do
                {   pFrm = pFrm->FindPrev();
                } while ( pFrm && !pFrm->IsValid() );

            if( !pFrm || pSh->VisArea().IsOver( pFrm->Frm() ) )
                bRet = TRUE;
        }
    }

    return bRet;
}

BOOL SwNode::IsInProtectSect() const
{
    const SwNode* pNd = ND_SECTIONNODE == nNodeType ? pStartOfSection : this;
    const SwSectionNode* pSectNd = pNd->FindSectionNode();
    return pSectNd && pSectNd->GetSection().IsProtectFlag();
}

    // befindet sich der Node in irgendetwas geschuetzten ?
    // (Bereich/Rahmen/Tabellenzellen/... incl. des Ankers bei
    //  Rahmen/Fussnoten/..)
BOOL SwNode::IsProtect() const
{
    const SwNode* pNd = ND_SECTIONNODE == nNodeType ? pStartOfSection : this;
    const SwStartNode* pSttNd = pNd->FindSectionNode();
    if( pSttNd && ((SwSectionNode*)pSttNd)->GetSection().IsProtectFlag() )
        return TRUE;

    if( 0 != ( pSttNd = FindTableBoxStartNode() ) )
    {
        SwCntntFrm* pCFrm;
        if( IsCntntNode() && 0 != (pCFrm = ((SwCntntNode*)this)->GetFrm() ))
            return pCFrm->IsProtected();

        const SwTableBox* pBox = pSttNd->FindTableNode()->GetTable().
                                        GetTblBox( pSttNd->GetIndex() );
        if( pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
            return TRUE;
    }

    SwFrmFmt* pFlyFmt = GetFlyFmt();
    if( pFlyFmt )
    {
        if( pFlyFmt->GetProtect().IsCntntProtected() )
            return TRUE;
        const SwFmtAnchor& rAnchor = pFlyFmt->GetAnchor();
        return rAnchor.GetCntntAnchor()
                ? rAnchor.GetCntntAnchor()->nNode.GetNode().IsProtect()
                : FALSE;
    }

    if( 0 != ( pSttNd = FindFootnoteStartNode() ) )
    {
        const SwTxtFtn* pTFtn = GetDoc()->GetFtnIdxs().SeekEntry(
                                SwNodeIndex( *pSttNd ) );
        if( pTFtn )
            return pTFtn->GetTxtNode().IsProtect();
    }

    return FALSE;
}

    // suche den PageDesc, mit dem dieser Node formatiert ist. Wenn das
    // Layout vorhanden ist wird ueber das gesucht, ansonsten gibt es nur
    // die harte Tour ueber die Nodes nach vorne suchen!!
const SwPageDesc* SwNode::FindPageDesc( BOOL bCalcLay ) const
{
    const SwPageDesc* pPgDesc = 0;

    if( GetNodes().IsDocNodes() )
    {
        const SwCntntNode* pNd;
        if( ND_STARTNODE & nNodeType )
        {
            SwNodeIndex aIdx( *this );
            pNd = GetNodes().GoNext( &aIdx );
        }
        else if( ND_ENDNODE & nNodeType )
        {
            SwNodeIndex aIdx( *EndOfSectionNode() );
            pNd = GetNodes().GoPrevious( &aIdx );
        }
        else
        {
            pNd = GetCntntNode();
            if( pNd )
                pPgDesc = ((SwFmtPageDesc&)pNd->GetAttr( RES_PAGEDESC )).GetPageDesc();
        }

        // geht es uebers Layout?
        if( !pPgDesc )
        {
            const SwFrm* pFrm;
            const SwPageFrm* pPage;
            if( pNd && 0 != ( pFrm = pNd->GetFrm( 0, 0, bCalcLay ) ) &&
                0 != ( pPage = pFrm->FindPageFrm() ) )
            {
                pPgDesc = pPage->GetPageDesc();
            }
        }

        if( !pPgDesc )
        {
            // dann also uebers Nodes-Array
            const SwDoc* pDoc = GetDoc();
            const SwNode* pNd = this;
            const SwStartNode* pSttNd;
            if( pNd->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() &&
                0 != ( pSttNd = pNd->FindFlyStartNode() ) )
            {
                // dann erstmal den richtigen Anker finden
                const SwFrmFmt* pFmt = 0;
                const SwSpzFrmFmts& rFmts = *pDoc->GetSpzFrmFmts();
                for( USHORT n = 0; n < rFmts.Count(); ++n )
                {
                    SwFrmFmt* pFrmFmt = rFmts[ n ];
                    const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
                    if( rCntnt.GetCntntIdx() &&
                        &rCntnt.GetCntntIdx()->GetNode() == (SwNode*)pSttNd )
                    {
                        pFmt = pFrmFmt;
                        break;
                    }
                }

                if( pFmt )
                {
                    const SwFmtAnchor* pAnchor = &pFmt->GetAnchor();
                    if( FLY_PAGE != pAnchor->GetAnchorId() &&
                        pAnchor->GetCntntAnchor() )
                    {
                        pNd = &pAnchor->GetCntntAnchor()->nNode.GetNode();
                        const SwNode* pFlyNd = pNd->FindFlyStartNode();
                        while( pFlyNd )
                        {
                            // dann ueber den Anker nach oben "hangeln"
                            for( n = 0; n < rFmts.Count(); ++n )
                            {
                                const SwFrmFmt* pFrmFmt = rFmts[ n ];
                                const SwNodeIndex* pIdx = pFrmFmt->GetCntnt().
                                                            GetCntntIdx();
                                if( pIdx && pFlyNd == &pIdx->GetNode() )
                                {
                                    if( pFmt == pFrmFmt )
                                    {
                                        pNd = pFlyNd;
                                        pFlyNd = 0;
                                        break;
                                    }
                                    pAnchor = &pFrmFmt->GetAnchor();
                                    if( FLY_PAGE == pAnchor->GetAnchorId() ||
                                        !pAnchor->GetCntntAnchor() )
                                    {
                                        pFlyNd = 0;
                                        break;
                                    }

                                    pFlyNd = pAnchor->GetCntntAnchor()->nNode.
                                            GetNode().FindFlyStartNode();
                                    break;
                                }
                            }
                            if( n >= rFmts.Count() )
                            {
                                ASSERT( !this, "Fly-Section aber kein Format gefunden" );
                                return FALSE;
                            }
                        }
                    }
                }
                // in pNd sollte jetzt der richtige Anker Node stehen oder
                // immer noch der this
            }

            if( pNd->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() )
            {
                if( pNd->GetIndex() > GetNodes().GetEndOfAutotext().GetIndex() )
                {
                    pPgDesc = &pDoc->GetPageDesc( 0 );
                    pNd = 0;
                }
                else
                {
                    // suche den Body Textnode
                    if( 0 != ( pSttNd = pNd->FindHeaderStartNode() ) ||
                        0 != ( pSttNd = pNd->FindFooterStartNode() ))
                    {
                        // dann in den PageDescs diesen StartNode suchen
                        USHORT nId;
                        UseOnPage eAskUse;
                        if( SwHeaderStartNode == pSttNd->GetStartNodeType())
                        {
                            nId = RES_HEADER;
                            eAskUse = PD_HEADERSHARE;
                        }
                        else
                        {
                            nId = RES_FOOTER;
                            eAskUse = PD_FOOTERSHARE;
                        }

                        for( USHORT n = pDoc->GetPageDescCnt(); n && !pPgDesc; )
                        {
                            const SwPageDesc& rPgDsc = pDoc->GetPageDesc( --n );
                            const SwFrmFmt* pFmt = &rPgDsc.GetMaster();
                            int nStt = 0, nLast = 1;
                            if( !( eAskUse & rPgDsc.ReadUseOn() )) ++nLast;

                            for( ; nStt < nLast; ++nStt, pFmt = &rPgDsc.GetLeft() )
                            {
                                const SwFmtHeader& rHdFt = (SwFmtHeader&)
                                                        pFmt->GetAttr( nId );
                                if( rHdFt.GetHeaderFmt() )
                                {
                                    const SwFmtCntnt& rCntnt =
                                        rHdFt.GetHeaderFmt()->GetCntnt();
                                    if( rCntnt.GetCntntIdx() &&
                                        &rCntnt.GetCntntIdx()->GetNode() ==
                                        (SwNode*)pSttNd )
                                    {
                                        pPgDesc = &rPgDsc;
                                        break;
                                    }
                                }
                            }
                        }

                        if( !pPgDesc )
                            pPgDesc = &pDoc->GetPageDesc( 0 );
                        pNd = 0;
                    }
                    else if( 0 != ( pSttNd = pNd->FindFootnoteStartNode() ))
                    {
                        // der Anker kann nur im Bodytext sein
                        const SwTxtFtn* pTxtFtn;
                        const SwFtnIdxs& rFtnArr = pDoc->GetFtnIdxs();
                        for( USHORT n = 0; n < rFtnArr.Count(); ++n )
                            if( 0 != ( pTxtFtn = rFtnArr[ n ])->GetStartNode() &&
                                (SwNode*)pSttNd ==
                                &pTxtFtn->GetStartNode()->GetNode() )
                            {
                                pNd = &pTxtFtn->GetTxtNode();
                                break;
                            }
                    }
                    else
                    {
                        // kann jetzt nur noch ein Seitengebundener Fly sein
                        // oder irgendetwas neueres.
                        // Hier koennen wir nur noch den Standard returnen
                        ASSERT( pNd->FindFlyStartNode(),
                                "wo befindet sich dieser Node?" );

                        pPgDesc = &pDoc->GetPageDesc( 0 );
                        pNd = 0;
                    }
                }
            }

            if( pNd )
            {
                SwFindNearestNode aInfo( *pNd );
                // dann ueber alle Nodes aller PageDesc
                const SfxPoolItem* pItem;
                USHORT i, nMaxItems = pDoc->GetAttrPool().GetItemCount( RES_PAGEDESC );
                for( i = 0; i < nMaxItems; ++i )
                    if( 0 != (pItem = pDoc->GetAttrPool().GetItem( RES_PAGEDESC, i ) ) &&
                        ((SwFmtPageDesc*)pItem)->GetDefinedIn() )
                    {
                        const SwModify* pMod = ((SwFmtPageDesc*)pItem)->GetDefinedIn();
                        if( pMod->ISA( SwCntntNode ) )
                            aInfo.CheckNode( *(SwCntntNode*)pMod );
                        else if( pMod->ISA( SwFmt ))
                            ((SwFmt*)pMod)->GetInfo( aInfo );
                    }

                if( 0 != ( pNd = aInfo.GetFoundNode() ))
                {
                    if( pNd->IsCntntNode() )
                        pPgDesc = ((SwFmtPageDesc&)pNd->GetCntntNode()->
                                    GetAttr( RES_PAGEDESC )).GetPageDesc();
                    else if( pNd->IsTableNode() )
                        pPgDesc = pNd->GetTableNode()->GetTable().
                                GetFrmFmt()->GetPageDesc().GetPageDesc();
                    else if( pNd->IsSectionNode() )
                        pPgDesc = pNd->GetSectionNode()->GetSection().
                                GetFmt()->GetPageDesc().GetPageDesc();
                }
                if( !pPgDesc )
                    pPgDesc = &pDoc->GetPageDesc( 0 );
            }
        }
    }
    return pPgDesc;
}


    // falls der Node in einem Fly steht, dann wird das entsprechende Format
    // returnt
SwFrmFmt* SwNode::GetFlyFmt() const
{
    SwFrmFmt* pRet = 0;
    const SwNode* pSttNd = FindFlyStartNode();
    if( pSttNd )
    {
        if( IsCntntNode() )
        {
            SwClientIter aIter( *(SwCntntNode*)this );
            SwClient* pCli = aIter.First( TYPE( SwCntntFrm ));
            if( pCli )
                pRet = ((SwCntntFrm*)pCli)->FindFlyFrm()->GetFmt();
        }
        if( !pRet )
        {
            // dann gibts noch harten steinigen Weg uebers Dokument:
            const SwSpzFrmFmts& rFrmFmtTbl = *GetDoc()->GetSpzFrmFmts();
            for( USHORT n = 0; n < rFrmFmtTbl.Count(); ++n )
            {
                SwFrmFmt* pFmt = rFrmFmtTbl[n];
                const SwFmtCntnt& rCntnt = pFmt->GetCntnt();
                if( rCntnt.GetCntntIdx() &&
                    &rCntnt.GetCntntIdx()->GetNode() == pSttNd )
                {
                    pRet = pFmt;
                    break;
                }
            }
        }
    }
    return pRet;
}

const SwFrmFmt* SwNode::GetFrmFmt() const
{
    const SwFrmFmt* pRet = 0;
    const SwNode* pNd = FindStartNode();
    ASSERT( pNd, "SwNode::GetFrmFmt(): Missing StartNode" );
    if( pNd->IsSectionNode() )
        pRet = ((SwSectionNode*)pNd)->GetSection().GetFmt();
    else if( pNd->IsTableNode() )
        pRet = ((SwTableNode*)pNd)->GetTable().GetFrmFmt();
    else switch( ((SwStartNode*)pNd)->GetStartNodeType() )
    {
        case SwNormalStartNode: break;
        case SwTableBoxStartNode: pRet = pNd->FindTableNode()->GetTable().GetTblBox(
                                         pNd->GetIndex() )->GetFrmFmt(); break;
        case SwFlyStartNode: pRet = GetFlyFmt(); break;
        case SwFootnoteStartNode: pRet = GetDoc()->GetDfltFrmFmt(); break;
        case SwHeaderStartNode: break;
        case SwFooterStartNode: break;
        default: ASSERT( FALSE, "GetFrmFmt(): New StartNodeType?" );
    }
    return pRet;
}

SwTableBox* SwNode::GetTblBox() const
{
    SwTableBox* pBox = 0;
    const SwNode* pSttNd = FindTableBoxStartNode();
    if( pSttNd )
        pBox = (SwTableBox*)pSttNd->FindTableNode()->GetTable().GetTblBox(
                                                    pSttNd->GetIndex() );
    return pBox;
}

SwStartNode* SwNode::FindSttNodeByType( SwStartNodeType eTyp )
{
    SwStartNode* pTmp = IsStartNode() ? (SwStartNode*)this : pStartOfSection;

    while( eTyp != pTmp->GetStartNodeType() && pTmp->GetIndex() )
#if defined( ALPHA ) && defined( UNX )
        pTmp = ((SwNode*)pTmp)->pStartOfSection;
#else
        pTmp = pTmp->pStartOfSection;
#endif
    return eTyp == pTmp->GetStartNodeType() ? pTmp : 0;
}

const SwTxtNode* SwNode::FindOutlineNodeOfLevel( BYTE nLvl ) const
{
    const SwTxtNode* pRet = 0;
    const SwOutlineNodes& rONds = GetNodes().GetOutLineNds();
    if( MAXLEVEL > nLvl && rONds.Count() )
    {
        USHORT nPos;
        SwNode* pNd = (SwNode*)this;
        BOOL bCheckFirst = FALSE;
        if( !rONds.Seek_Entry( pNd, &nPos ))
        {
            if( nPos )
                nPos = nPos-1;
            else
                bCheckFirst = TRUE;
        }

        if( bCheckFirst )
        {
            // der 1.GliederungsNode liegt hinter dem Fragenden. Dann
            // teste mal, ob dieser auf der gleichen Seite steht. Wenn
            // nicht, ist das ein ungueltiger. Bug 61865
            pRet = rONds[0]->GetTxtNode();

            const SwCntntNode* pCNd = GetCntntNode();

            Point aPt( 0, 0 );
            const SwFrm* pFrm = pRet->GetFrm( &aPt, 0, FALSE ),
                       * pMyFrm = pCNd ? pCNd->GetFrm( &aPt, 0, FALSE ) : 0;
            const SwPageFrm* pPgFrm = pFrm ? pFrm->FindPageFrm() : 0;
            if( pPgFrm && pMyFrm &&
                pPgFrm->Frm().Top() > pMyFrm->Frm().Top() )
            {
                // der Fragende liegt vor der Seite, also ist er ungueltig
                pRet = 0;
            }
        }
        else
        {
            // oder ans Feld und von dort holen !!
            while( nPos && nLvl < ( pRet = rONds[nPos]->GetTxtNode() )
                    ->GetTxtColl()->GetOutlineLevel() )
                --nPos;

            if( !nPos )     // bei 0 gesondert holen !!
                pRet = rONds[0]->GetTxtNode();
        }
    }
    return pRet;
}

// is the node the first and/or last node of a section?
// This information is used for the export filters. Our layout never have a
// distance before or after if the node is the first or last in a section.

inline IsValidNextPrevNd( const SwNode& rNd )
{
    return ND_TABLENODE == rNd.GetNodeType() ||
           ( ND_CONTENTNODE & rNd.GetNodeType() ) ||
            ( ND_ENDNODE == rNd.GetNodeType() && rNd.StartOfSectionNode() &&
            ND_TABLENODE == rNd.StartOfSectionNode()->GetNodeType() );
}

BYTE SwNode::HasPrevNextLayNode() const
{
    BYTE nRet = 0;
    if( IsValidNextPrevNd( *this ))
    {
        SwNodeIndex aIdx( *this, -1 );
        if( IsValidNextPrevNd( aIdx.GetNode() ))
            nRet |= ND_HAS_PREV_LAYNODE;
        aIdx += 2;
        if( IsValidNextPrevNd( aIdx.GetNode() ))
            nRet |= ND_HAS_NEXT_LAYNODE;
    }
    return nRet;
}

/*******************************************************************
|*
|*  SwNode::StartOfSection
|*
|*  Beschreibung
|*      Die Funktion liefert die StartOfSection des Nodes.
|*
|*  Parameter
|*      IN
|*      rNodes bezeichnet das variable Array, in dem sich der Node
|*      befindet
|*  Ersterstellung
|*      VER0100 vb 901214
|*
|*  Stand
|*      VER0100 vb 901214
|*
*******************************************************************/


SwStartNode::SwStartNode( const SwNodeIndex &rWhere, const BYTE nNdType,
                            SwStartNodeType eSttNd )
    : SwNode( rWhere, nNdType ), eSttNdTyp( eSttNd )
{
    // erstmal temporaer, bis der EndNode eingefuegt wird.
    pEndOfSection = (SwEndNode*)this;
}

SwStartNode::SwStartNode( SwNodes& rNodes, ULONG nPos )
    : SwNode( rNodes, nPos, ND_STARTNODE ), eSttNdTyp( SwNormalStartNode )
{
    // erstmal temporaer, bis der EndNode eingefuegt wird.
    pEndOfSection = (SwEndNode*)this;
}


void SwStartNode::CheckSectionCondColl() const
{
//FEATURE::CONDCOLL
    SwNodeIndex aIdx( *this );
    ULONG nEndIdx = EndOfSectionIndex();
    const SwNodes& rNds = GetNodes();
    SwCntntNode* pCNd;
    while( 0 != ( pCNd = rNds.GoNext( &aIdx )) && pCNd->GetIndex() < nEndIdx )
        pCNd->ChkCondColl();
//FEATURE::CONDCOLL
}

/*******************************************************************
|*
|*  SwEndNode::SwEndNode
|*
|*  Beschreibung
|*      Konstruktor; dieser fuegt einen Node in das Array rNodes
|*      an der Position aWhere ein. Der
|*      theStartOfSection-Pointer wird entsprechend gesetzt,
|*      und der EndOfSection-Pointer des zugehoerigen
|*      Startnodes -- durch rStartOfSection bezeichnet --
|*      wird auf diesen Node gesetzt.
|*
|*  Parameter
|*      IN
|*      rNodes bezeichnet das variable Array, in das der Node
|*      eingefuegt werden soll
|*      IN
|*      aWhere bezeichnet die Position innerhalb dieses Arrays,
|*      an der der Node eingefuegt werden soll
|*      !!!!!!!!!!!!
|*      Es wird eine Kopie uebergeben!
|*
|*  Ersterstellung
|*      VER0100 vb 901214
|*
|*  Stand
|*      VER0100 vb 901214
|*
*******************************************************************/


SwEndNode::SwEndNode( const SwNodeIndex &rWhere, SwStartNode& rSttNd )
    : SwNode( rWhere, ND_ENDNODE )
{
    pStartOfSection = &rSttNd;
    pStartOfSection->pEndOfSection = this;
}

SwEndNode::SwEndNode( SwNodes& rNds, ULONG nPos, SwStartNode& rSttNd )
    : SwNode( rNds, nPos, ND_ENDNODE )
{
    pStartOfSection = &rSttNd;
    pStartOfSection->pEndOfSection = this;
}



// --------------------
// SwCntntNode
// --------------------


SwCntntNode::SwCntntNode( const SwNodeIndex &rWhere, const BYTE nNdType,
                            SwFmtColl *pColl )
    : SwNode( rWhere, nNdType ),
    pAttrSet( 0 ),
    pCondColl( 0 ),
    SwModify( pColl )    // CrsrsShell, FrameFmt
#ifdef OLD_INDEX
    ,SwIndexReg(2)
#endif
{
}


SwCntntNode::~SwCntntNode()
{
    // Die Basisklasse SwClient vom SwFrm nimmt sich aus
    // der Abhaengikeitsliste raus!
    // Daher muessen alle Frames in der Abhaengigkeitsliste geloescht werden.
    if( GetDepends() )
        DelFrms();

    if( pAttrSet )
        delete pAttrSet;
    if( pCondColl )
        delete pCondColl;
}


void SwCntntNode::Modify( SfxPoolItem* pOldValue, SfxPoolItem* pNewValue )
{
    USHORT nWhich = pOldValue ? pOldValue->Which() :
                    pNewValue ? pNewValue->Which() : 0 ;
    BOOL bNumRuleSet = FALSE;
    String sNumRule, sOldNumRule;
    const SfxPoolItem* pItem;

    switch( nWhich )
    {
    case RES_OBJECTDYING :
        {
            SwFmt * pFmt = (SwFmt *) ((SwPtrMsgPoolItem *)pNewValue)->pObject;

            // nicht umhaengen wenn dieses das oberste Format ist !!
            if( pRegisteredIn == pFmt )
            {
                if( pFmt->GetRegisteredIn() )
                {
                    // wenn Parent, dann im neuen Parent wieder anmelden
                    ((SwModify*)pFmt->GetRegisteredIn())->Add( this );
                    if ( pAttrSet )
                        pAttrSet->SetParent(
                                &((SwFmt*)GetRegisteredIn())->GetAttrSet() );
                }
                else
                {
                    // sonst auf jeden Fall beim sterbenden abmelden
                    ((SwModify*)GetRegisteredIn())->Remove( this );
                    if ( pAttrSet )
                        pAttrSet->SetParent( 0 );
                }
            }
        }
        break;


    case RES_FMT_CHG:
        // falls mein Format Parent umgesetzt wird, dann melde ich
        // meinen Attrset beim Neuen an.

        // sein eigenes Modify ueberspringen !!
        if( pAttrSet &&
            ((SwFmtChg*)pNewValue)->pChangedFmt == GetRegisteredIn() )
        {
            // den Set an den neuen Parent haengen
            pAttrSet->SetParent( GetRegisteredIn() ?
                &((SwFmt*)GetRegisteredIn())->GetAttrSet() : 0 );
        }
        if( GetNodes().IsDocNodes() && IsTxtNode() )
        {
            if( 0 != ( pItem = GetNoCondAttr( RES_PARATR_NUMRULE, TRUE )))
            {
                bNumRuleSet = TRUE;
                sNumRule = ((SwNumRuleItem*)pItem)->GetValue();
            }
            sOldNumRule = ((SwFmtChg*)pOldValue)->pChangedFmt->GetNumRule().GetValue();
        }
        break;
//FEATURE::CONDCOLL
    case RES_CONDCOLL_CONDCHG:
        if( ((SwCondCollCondChg*)pNewValue)->pChangedFmt == GetRegisteredIn() &&
            &GetNodes() == &GetDoc()->GetNodes() )
        {
            ChkCondColl();
        }
        return ;    // nicht an die Basisklasse / Frames weitergeben
//FEATURE::CONDCOLL

    case RES_ATTRSET_CHG:
        if( GetNodes().IsDocNodes() && IsTxtNode() )
        {
            if( SFX_ITEM_SET == ((SwAttrSetChg*)pNewValue)->GetChgSet()->GetItemState(
                RES_PARATR_NUMRULE, FALSE, &pItem ))
            {
                bNumRuleSet = TRUE;
                sNumRule = ((SwNumRuleItem*)pItem)->GetValue();
            }
            if( SFX_ITEM_SET == ((SwAttrSetChg*)pOldValue)->GetChgSet()->GetItemState(
                RES_PARATR_NUMRULE, FALSE, &pItem ))
                sOldNumRule = ((SwNumRuleItem*)pItem)->GetValue();
        }
        break;

    case RES_PARATR_NUMRULE:
        if( GetNodes().IsDocNodes() && IsTxtNode() )
        {
            if( pNewValue )
            {
                bNumRuleSet = TRUE;
                sNumRule = ((SwNumRuleItem*)pNewValue)->GetValue();
            }
            if( pOldValue )
                sOldNumRule = ((SwNumRuleItem*)pOldValue)->GetValue();
        }
        break;
    }

    if( bNumRuleSet )
    {
        if( sNumRule.Len() )
        {
            if( !((SwTxtNode*)this)->GetNum() )
                ((SwTxtNode*)this)->UpdateNum( SwNodeNum(0) );
#ifndef NUM_RELSPACE
            SetNumLSpace( TRUE );
#endif
            SwNumRule* pRule = GetDoc()->FindNumRulePtr( sNumRule );
            if( !pRule )
            {
                USHORT nPoolId = GetDoc()->GetPoolId( sNumRule, GET_POOLID_NUMRULE );
                if( USHRT_MAX != nPoolId )
                    pRule = GetDoc()->GetNumRuleFromPool( nPoolId );
            }
            if( pRule )
                pRule->SetInvalidRule( TRUE );
        }
        else if( ((SwTxtNode*)this)->GetNum() )
        {
            ((SwTxtNode*)this)->UpdateNum( SwNodeNum(NO_NUMBERING) );
#ifndef NUM_RELSPACE
            SetNumLSpace( TRUE );
#endif
        }
    }
    if( sOldNumRule.Len() && sNumRule != sOldNumRule )
    {
        SwNumRule* pRule = GetDoc()->FindNumRulePtr( sOldNumRule );
        if( pRule )
            pRule->SetInvalidRule( TRUE );
    }

    SwModify::Modify( pOldValue, pNewValue );
}

BOOL SwCntntNode::InvalidateNumRule()
{
    SwNumRule* pRule = 0;
    const SfxPoolItem* pItem;
    if( GetNodes().IsDocNodes() &&
        0 != ( pItem = GetNoCondAttr( RES_PARATR_NUMRULE, TRUE )) &&
        ((SwNumRuleItem*)pItem)->GetValue().Len() &&
        0 != (pRule = GetDoc()->FindNumRulePtr(
                                ((SwNumRuleItem*)pItem)->GetValue() ) ) )
    {
        pRule->SetInvalidRule( TRUE );
    }
    return 0 != pRule;
}


SwCntntFrm *SwCntntNode::GetFrm( const Point* pPoint,
                                const SwPosition *pPos,
                                const BOOL bCalcFrm ) const
{
    return (SwCntntFrm*) ::GetFrmOfModify( *(SwModify*)this, FRM_CNTNT,
                                            pPoint, pPos, bCalcFrm );
}

SwRect SwCntntNode::FindLayoutRect( const BOOL bPrtArea, const Point* pPoint,
                                    const BOOL bCalcFrm ) const
{
    SwRect aRet;
    SwCntntFrm* pFrm = (SwCntntFrm*)::GetFrmOfModify( *(SwModify*)this,
                                            FRM_CNTNT, pPoint, 0, bCalcFrm );
    if( pFrm )
        aRet = bPrtArea ? pFrm->Prt() : pFrm->Frm();
    return aRet;
}

SwRect SwCntntNode::FindPageFrmRect( const BOOL bPrtArea, const Point* pPoint,
                                    const BOOL bCalcFrm ) const
{
    SwRect aRet;
    SwFrm* pFrm = ::GetFrmOfModify( *(SwModify*)this,
                                            FRM_CNTNT, pPoint, 0, bCalcFrm );
    if( pFrm && 0 != ( pFrm = pFrm->FindPageFrm() ))
        aRet = bPrtArea ? pFrm->Prt() : pFrm->Frm();
    return aRet;
}

xub_StrLen SwCntntNode::Len() const { return 0; }



SwFmtColl *SwCntntNode::ChgFmtColl( SwFmtColl *pNewColl )
{
    ASSERT( pNewColl, Collectionpointer ist 0. );
    SwFmtColl *pOldColl = GetFmtColl();
    if( pNewColl != pOldColl )
    {
        pNewColl->Add( this );

        // setze den Parent von unseren Auto-Attributen auf die neue
        // Collection:
        if( pAttrSet )
            pAttrSet->SetParent( &pNewColl->GetAttrSet() );

//FEATURE::CONDCOLL
        // HACK: hier muss die entsprechend der neuen Vorlage die Bedingungen
        //      neu ueberprueft werden!
        if( TRUE /*pNewColl */ )
        {
            SetCondFmtColl( 0 );
        }
//FEATURE::CONDCOLL

        if( !IsModifyLocked() )
        {
            SwFmtChg aTmp1( pOldColl );
            SwFmtChg aTmp2( pNewColl );
//          SwModify::Modify( &aTmp1, &aTmp2 );
            // damit alles was im Modify passiert hier nicht noch impl.
            // werden muss
            SwCntntNode::Modify( &aTmp1, &aTmp2 );
        }
    }
    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( FALSE );
    }
    return pOldColl;
}


BOOL SwCntntNode::GoNext(SwIndex * pIdx) const
{
    if( pIdx->GetIndex() < Len() )
    {
        if( !IsTxtNode() )
        {
            (*pIdx)++;
            return TRUE;
        }

        if( pBreakIt->xBreak.is() )
        {
            const SwTxtNode& rTNd = *GetTxtNode();
            xub_StrLen nPos = pIdx->GetIndex();
            sal_Int32 nDone = 0;
            nPos = pBreakIt->xBreak->nextCharacters( rTNd.GetTxt(), nPos,
                            pBreakIt->GetLocale( rTNd.GetLang( nPos ) ),
                            CharacterIteratorMode::SKIPCONTROLCHARACTER,
                            1, nDone );
            if( 1 == nDone )
            {
                *pIdx = nPos;
                return TRUE;
            }
        }
    }
    return FALSE;
}


BOOL SwCntntNode::GoPrevious(SwIndex * pIdx) const
{
    if( pIdx->GetIndex() > 0 )
    {
        if( !IsTxtNode() )
        {
            (*pIdx)--;
            return TRUE;
        }

        if( pBreakIt->xBreak.is() )
        {
            const SwTxtNode& rTNd = *GetTxtNode();
            xub_StrLen nPos = pIdx->GetIndex();
            sal_Int32 nDone = 0;
            nPos = pBreakIt->xBreak->previousCharacters( rTNd.GetTxt(), nPos,
                            pBreakIt->GetLocale( rTNd.GetLang( nPos ) ),
                            CharacterIteratorMode::SKIPCONTROLCHARACTER,
                            1, nDone );
            if( 1 == nDone )
            {
                *pIdx = nPos;
                return TRUE;
            }
        }
    }
    return FALSE;
}


/*
 * Methode erzeugt fuer den vorhergehenden Node alle Ansichten vom
 * Dokument. Die erzeugten Contentframes werden in das entsprechende
 * Layout gehaengt.
 */


void SwCntntNode::MakeFrms( SwCntntNode& rNode )
{
    ASSERT( &rNode != this,
            "Kein Contentnode oder Copy-Node und neuer Node identisch." );

    if( !GetDepends() || &rNode == this )   // gibt es ueberhaupt Frames ??
        return;

    SwFrm *pFrm, *pNew;
    SwLayoutFrm *pUpper;
    // Frames anlegen fuer Nodes, die vor oder hinter der Tabelle stehen ??
    ASSERT( FindTableNode() == rNode.FindTableNode(), "Table confusion" )

    SwNode2Layout aNode2Layout( *this, rNode.GetIndex() );

    while( 0 != (pUpper = aNode2Layout.UpperFrm( pFrm, rNode )) )
    {
        pNew = rNode.MakeFrm();
        pNew->Paste( pUpper, pFrm );
    }
}

/*
 * Methode loescht fuer den Node alle Ansichten vom
 * Dokument. Die Contentframes werden aus dem entsprechenden
 * Layout ausgehaengt.
 */


void SwCntntNode::DelFrms()
{
    if( !GetDepends() )
        return;

    SwClientIter aIter( *this );
    SwCntntFrm *pFrm;

    for( pFrm = (SwCntntFrm*)aIter.First( TYPE(SwCntntFrm)); pFrm;
         pFrm = (SwCntntFrm*)aIter.Next() )
    {
        pFrm->SetFollow( 0 );//Damit er nicht auf dumme Gedanken kommt.
                                //Andernfalls kann es sein, dass ein Follow
                                //vor seinem Master zerstoert wird, der Master
                                //greift dann ueber den ungueltigen
                                //Follow-Pointer auf fremdes Memory zu.
                                //Die Kette darf hier zerknauscht werden, weil
                                //sowieso alle zerstoert werden.
        if( pFrm->GetUpper() && pFrm->IsInFtn() && !pFrm->GetIndNext() &&
            !pFrm->GetIndPrev() )
        {
            SwFtnFrm *pFtn = pFrm->FindFtnFrm();
            ASSERT( pFtn, "You promised a FtnFrm?" );
            if( !pFtn->GetFollow() && !pFtn->GetMaster() && pFtn->GetRef() &&
                pFtn->GetRef()->IsFollow() )
            {
                ASSERT( pFtn->GetRef()->IsTxtFrm(), "NoTxtFrm has Footnote?" );
                ((SwTxtFrm*)pFtn->GetRef()->FindMaster())->Prepare( PREP_FTN_GONE );
            }
        }
        pFrm->Cut();
        delete pFrm;
    }
    if( IsTxtNode() )
    {
        ((SwTxtNode*)this)->SetWrong( NULL );
        SetWrongDirty( TRUE );
        SetAutoCompleteWordDirty( TRUE );
    }
}


SwCntntNode *SwCntntNode::JoinNext()
{
    return this;
}


SwCntntNode *SwCntntNode::JoinPrev()
{
    return this;
}



    // erfrage vom Modify Informationen
BOOL SwCntntNode::GetInfo( SfxPoolItem& rInfo ) const
{
    const SwNumRuleItem* pItem;
    switch( rInfo.Which() )
    {
    case RES_AUTOFMT_DOCNODE:
        if( &GetNodes() == ((SwAutoFmtGetDocNode&)rInfo).pNodes )
        {
            ((SwAutoFmtGetDocNode&)rInfo).pCntntNode = this;
            return FALSE;
        }
        break;
    case RES_GETNUMNODES:
        if( IsTxtNode() && 0 != ( pItem = (SwNumRuleItem*)GetNoCondAttr(
            RES_PARATR_NUMRULE, TRUE )) &&
            pItem->GetValue().Len() &&
            pItem->GetValue() == ((SwNumRuleInfo&)rInfo).GetName() )
        {
            ((SwNumRuleInfo&)rInfo).AddNode( *(SwTxtNode*)this );
        }
        return TRUE;

    case RES_GETLOWERNUMLEVEL:
        if( IsTxtNode() && ((SwTxtNode*)this)->GetNum() &&
            0 != ( pItem = (SwNumRuleItem*)GetNoCondAttr(
            RES_PARATR_NUMRULE, TRUE )) && pItem->GetValue().Len() &&
            pItem->GetValue() == ((SwNRuleLowerLevel&)rInfo).GetName() &&
            (((SwTxtNode*)this)->GetNum()->GetLevel() & ~NO_NUMLEVEL)
                > ((SwNRuleLowerLevel&)rInfo).GetLevel() )
        {
            return FALSE;
        }
        break;

    case RES_FINDNEARESTNODE:
        if( ((SwFmtPageDesc&)GetAttr( RES_PAGEDESC )).GetPageDesc() )
            ((SwFindNearestNode&)rInfo).CheckNode( *this );
        return TRUE;

    case RES_CONTENT_VISIBLE:
        {
            ((SwPtrMsgPoolItem&)rInfo).pObject =
                SwClientIter( *(SwCntntNode*)this ).First( TYPE(SwFrm) );
        }
        return FALSE;
    }

    return SwModify::GetInfo( rInfo );
}


    // setze ein Attribut
BOOL SwCntntNode::SetAttr(const SfxPoolItem& rAttr )
{
    if( !pAttrSet )         // lasse von den entsprechenden Nodes die
        NewAttrSet( GetDoc()->GetAttrPool() );      // AttrSets anlegen

    ASSERT( pAttrSet, "warum wurde kein AttrSet angelegt?" );

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( FALSE );
    }

    BOOL bRet = FALSE;
    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if( IsModifyLocked() ||
        ( !GetDepends() &&  RES_PARATR_NUMRULE != rAttr.Which() ))
    {
        if( 0 != ( bRet = (0 != pAttrSet->Put( rAttr )) ))
            // einige Sonderbehandlungen fuer Attribute
            pAttrSet->SetModifyAtAttr( this );
    }
    else
    {
        SwAttrSet aOld( *pAttrSet->GetPool(), pAttrSet->GetRanges() ),
                    aNew( *pAttrSet->GetPool(), pAttrSet->GetRanges() );
        if( 0 != ( bRet = pAttrSet->Put_BC( rAttr, &aOld, &aNew ) ))
        {
            // einige Sonderbehandlungen fuer Attribute
            pAttrSet->SetModifyAtAttr( this );

            SwAttrSetChg aChgOld( *pAttrSet, aOld );
            SwAttrSetChg aChgNew( *pAttrSet, aNew );
            Modify( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
        }
    }
    return bRet;
}


BOOL SwCntntNode::SetAttr( const SfxItemSet& rSet )
{
    if( !pAttrSet )         // lasse von den entsprechenden Nodes die
        NewAttrSet( GetDoc()->GetAttrPool() );      // AttrSets anlegen

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( FALSE );
    }

    BOOL bRet = FALSE;

    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if( IsModifyLocked() || ( !GetDepends() &&
        SFX_ITEM_SET != rSet.GetItemState( RES_PARATR_NUMRULE, FALSE )) )
    {
        // einige Sonderbehandlungen fuer Attribute
        if( 0 != (bRet = (0 != pAttrSet->Put( rSet ))) )
            pAttrSet->SetModifyAtAttr( this );
    }
    else
    {
        SwAttrSet aOld( *pAttrSet->GetPool(), pAttrSet->GetRanges() ),
                    aNew( *pAttrSet->GetPool(), pAttrSet->GetRanges() );
        if( 0 != (bRet = pAttrSet->Put_BC( rSet, &aOld, &aNew )) )
        {
            // einige Sonderbehandlungen fuer Attribute
            pAttrSet->SetModifyAtAttr( this );
            SwAttrSetChg aChgOld( *pAttrSet, aOld );
            SwAttrSetChg aChgNew( *pAttrSet, aNew );
            Modify( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
        }
    }
    return bRet;
}

// Nimmt den Hint mit nWhich aus dem Delta-Array


BOOL SwCntntNode::ResetAttr( USHORT nWhich1, USHORT nWhich2 )
{
    if( !pAttrSet )
        return FALSE;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( FALSE );
    }

    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if( IsModifyLocked() )
    {
        USHORT nDel = (!nWhich2 || nWhich2 < nWhich1)
                ? pAttrSet->ClearItem( nWhich1 )
                : pAttrSet->ClearItem_BC( nWhich1, nWhich2 );

        if( !pAttrSet->Count() )    // leer, dann loeschen
            DELETEZ( pAttrSet );
        return 0 != nDel;
    }

    // sollte kein gueltiger Bereich definiert sein ?
    if( !nWhich2 || nWhich2 < nWhich1 )
        nWhich2 = nWhich1;      // dann setze auf 1. Id, nur dieses Item

    SwAttrSet aOld( *pAttrSet->GetPool(), pAttrSet->GetRanges() ),
                aNew( *pAttrSet->GetPool(), pAttrSet->GetRanges() );
    BOOL bRet = 0 != pAttrSet->ClearItem_BC( nWhich1, nWhich2, &aOld, &aNew );

    if( bRet )
    {
        SwAttrSetChg aChgOld( *pAttrSet, aOld );
        SwAttrSetChg aChgNew( *pAttrSet, aNew );
        Modify( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt

        if( !pAttrSet->Count() )    // leer, dann loeschen
            DELETEZ( pAttrSet );
    }
    return bRet;
}
BOOL SwCntntNode::ResetAttr( const SvUShorts& rWhichArr )
{
    if( !pAttrSet )
        return FALSE;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( FALSE );
    }

    // wenn Modify gelockt ist, werden keine Modifies verschickt
    USHORT nDel = 0;
    if( IsModifyLocked() )
    {
        for( USHORT n = 0, nEnd = rWhichArr.Count(); n < nEnd; ++n )
            if( pAttrSet->ClearItem( rWhichArr[ n ] ))
                ++nDel;
    }
    else
    {
        SwAttrSet aOld( *pAttrSet->GetPool(), pAttrSet->GetRanges() ),
                    aNew( *pAttrSet->GetPool(), pAttrSet->GetRanges() );

        for( USHORT n = 0, nEnd = rWhichArr.Count(); n < nEnd; ++n )
            if( pAttrSet->ClearItem( rWhichArr[ n ] ))
                ++nDel;

        if( nDel )
        {
            SwAttrSetChg aChgOld( *pAttrSet, aOld );
            SwAttrSetChg aChgNew( *pAttrSet, aNew );
            Modify( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
        }
    }
    if( !pAttrSet->Count() )    // leer, dann loeschen
        DELETEZ( pAttrSet );
    return 0 != nDel ;
}


USHORT SwCntntNode::ResetAllAttr()
{
    if( !pAttrSet )
        return 0;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( FALSE );
    }

    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if( IsModifyLocked() )
    {
        USHORT nDel = pAttrSet->ClearItem( 0 );
        if( !pAttrSet->Count() )    // leer, dann loeschen
            DELETEZ( pAttrSet );
        return nDel;
    }

    SwAttrSet aOld( *pAttrSet->GetPool(), pAttrSet->GetRanges() ),
                aNew( *pAttrSet->GetPool(), pAttrSet->GetRanges() );
    BOOL bRet = 0 != pAttrSet->ClearItem_BC( 0, &aOld, &aNew );

    if( bRet )
    {
        SwAttrSetChg aChgOld( *pAttrSet, aOld );
        SwAttrSetChg aChgNew( *pAttrSet, aNew );
        Modify( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt

        if( !pAttrSet->Count() )    // leer, dann loeschen
            DELETEZ( pAttrSet );
    }
    return aNew.Count();
}


BOOL SwCntntNode::GetAttr( SfxItemSet& rSet, BOOL bInParent ) const
{
    if( rSet.Count() )
        rSet.ClearItem();

    const SwAttrSet& rAttrSet = GetSwAttrSet();
    if( bInParent )
        return rSet.Set( rAttrSet, TRUE ) ? TRUE : FALSE;

    rSet.Put( rAttrSet );
    return rSet.Count() ? TRUE : FALSE;
}

const SfxPoolItem* SwCntntNode::GetNoCondAttr( USHORT nWhich,
                                                BOOL bInParents ) const
{
    const SfxPoolItem* pFnd = 0;
    if( pCondColl && pCondColl->GetRegisteredIn() )
    {
        if( !pAttrSet || ( SFX_ITEM_SET != pAttrSet->GetItemState(
                    nWhich, FALSE, &pFnd ) && bInParents ))
            ((SwFmt*)GetRegisteredIn())->GetItemState( nWhich, bInParents, &pFnd );
    }
    else
        GetSwAttrSet().GetItemState( nWhich, bInParents, &pFnd );
    return pFnd;
}

    // koennen 2 Nodes zusammengefasst werden ?
    // in pIdx kann die 2. Position returnt werden.
int SwCntntNode::CanJoinNext( SwNodeIndex* pIdx ) const
{
    const SwNodes& rNds = GetNodes();
    BYTE nNdType = GetNodeType();
    SwNodeIndex aIdx( *this, 1 );

    const SwNode* pNd = this;
    while( aIdx < rNds.Count()-1 &&
        (( pNd = &aIdx.GetNode())->IsSectionNode() ||
            ( pNd->IsEndNode() && pNd->FindStartNode()->IsSectionNode() )))
        aIdx++;

    if( pNd->GetNodeType() != nNdType || rNds.Count()-1 == aIdx.GetIndex() )
        return FALSE;
    if( pIdx )
        *pIdx = aIdx;
    return TRUE;
}


    // koennen 2 Nodes zusammengefasst werden ?
    // in pIdx kann die 2. Position returnt werden.
int SwCntntNode::CanJoinPrev( SwNodeIndex* pIdx ) const
{
    const SwNodes& rNds = GetNodes();
    BYTE nNdType = GetNodeType();
    SwNodeIndex aIdx( *this, -1 );

    const SwNode* pNd = this;
    while( aIdx.GetIndex() &&
        (( pNd = &aIdx.GetNode())->IsSectionNode() ||
            ( pNd->IsEndNode() && pNd->FindStartNode()->IsSectionNode() )))
        aIdx--;

    if( pNd->GetNodeType() != nNdType || 0 == aIdx.GetIndex() )
        return FALSE;
    if( pIdx )
        *pIdx = aIdx;
    return TRUE;
}


//FEATURE::CONDCOLL


void SwCntntNode::SetCondFmtColl( SwFmtColl* pColl )
{
    if( (!pColl && pCondColl) || ( pColl && !pCondColl ) ||
        ( pColl && pColl != pCondColl->GetRegisteredIn() ) )
    {
        SwFmtColl* pOldColl = GetCondFmtColl();
        delete pCondColl;
        if( pColl )
            pCondColl = new SwDepend( this, pColl );
        else
            pCondColl = 0;

        if( pAttrSet )
        {
// Attrset beibehalten oder loeschen??
// 13.04.99: Bisher wurden er geloescht, jetzt wird er beibehalten.
//           #64637#: Beim Laden eines Dokuments wird die bedingte
//           Vorlage nach dem Laden der harten Attribute gesetzt. Deshalb
//           wurden die harten Attribute geloescht.

            pAttrSet->SetParent( &GetAnyFmtColl().GetAttrSet() );
// steht im docfmt.cxx
//extern BOOL lcl_RstAttr( const SwNodePtr&, void* );
//          lcl_RstAttr( this, 0 );
//          if( pAttrSet && !pAttrSet->Count() )
//              delete pAttrSet, pAttrSet = 0;
        }

        if( !IsModifyLocked() )
        {
            SwFmtChg aTmp1( pOldColl ? pOldColl : GetFmtColl() );
            SwFmtChg aTmp2( pColl ? pColl : GetFmtColl() );
            SwModify::Modify( &aTmp1, &aTmp2 );
        }
        if( IsInCache() )
        {
            SwFrm::GetCache().Delete( this );
            SetInCache( FALSE );
        }
    }
}


BOOL SwCntntNode::IsAnyCondition( SwCollCondition& rTmp ) const
{
    const SwNodes& rNds = GetNodes();
    {
        int nCond = 0;
        const SwStartNode* pSttNd = FindStartNode();
        while( pSttNd )
        {
            switch( pSttNd->GetNodeType() )
            {
            case ND_TABLENODE:      nCond = PARA_IN_TABLEBODY; break;
            case ND_SECTIONNODE:    nCond = PARA_IN_SECTION; break;

            default:
                switch( pSttNd->GetStartNodeType() )
                {
                case SwTableBoxStartNode:
                    {
                        nCond = PARA_IN_TABLEBODY;
                        const SwTableNode* pTblNd = pSttNd->FindTableNode();
                        const SwTableBox* pBox;
                        if( pTblNd && 0 != ( pBox = pTblNd->GetTable().
                            GetTblBox( pSttNd->GetIndex() ) ) &&
                            pBox->IsInHeadline( &pTblNd->GetTable() ) )
                            nCond = PARA_IN_TABLEHEAD;
                    }
                    break;
                case SwFlyStartNode:        nCond = PARA_IN_FRAME; break;
                case SwFootnoteStartNode:
                    {
                        nCond = PARA_IN_FOOTENOTE;
                        const SwFtnIdxs& rFtnArr = rNds.GetDoc()->GetFtnIdxs();
                        const SwTxtFtn* pTxtFtn;
                        const SwNode* pSrchNd = pSttNd;

                        for( USHORT n = 0; n < rFtnArr.Count(); ++n )
                            if( 0 != ( pTxtFtn = rFtnArr[ n ])->GetStartNode() &&
                                pSrchNd == &pTxtFtn->GetStartNode()->GetNode() )
                            {
                                if( pTxtFtn->GetFtn().IsEndNote() )
                                    nCond = PARA_IN_ENDNOTE;
                                break;
                            }
                    }
                    break;
                case SwHeaderStartNode:     nCond = PARA_IN_HEADER; break;
                case SwFooterStartNode:     nCond = PARA_IN_FOOTER; break;
                }
            }

            if( nCond )
            {
                rTmp.SetCondition( (Master_CollConditions)nCond, 0 );
                return TRUE;
            }
            pSttNd = pSttNd->GetIndex()
                        ? pSttNd->FindStartNode()
                        : 0;
        }
    }

    {
        USHORT nPos;
        const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
        if( rOutlNds.Count() )
        {
            if( !rOutlNds.Seek_Entry( (SwCntntNode*)this, &nPos ) && nPos )
                --nPos;
            if( nPos < rOutlNds.Count() &&
                rOutlNds[ nPos ]->GetIndex() < GetIndex() )
            {
                SwTxtNode* pOutlNd = rOutlNds[ nPos ]->GetTxtNode();

                if( pOutlNd->GetOutlineNum() && !pOutlNd->GetNumRule() )
                {
                    rTmp.SetCondition( PARA_IN_OUTLINE,
                                    pOutlNd->GetOutlineNum()->GetLevel() );
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}


void SwCntntNode::ChkCondColl()
{
    // zur Sicherheit abfragen
    if( RES_CONDTXTFMTCOLL == GetFmtColl()->Which() )
    {
        SwCollCondition aTmp( 0, 0, 0 );
        const SwCollCondition* pCColl;

        if( IsAnyCondition( aTmp ) && 0 != ( pCColl =
                ((SwConditionTxtFmtColl*)GetFmtColl())->HasCondition( aTmp )))
            SetCondFmtColl( pCColl->GetTxtFmtColl() );
        else
        {
            if( IsTxtNode() && ((SwTxtNode*)this)->GetNumRule() &&
                    ((SwTxtNode*)this)->GetNum() )
            {
                // steht in einer Numerierung
                // welcher Level?
                aTmp.SetCondition( PARA_IN_LIST,
                                ((SwTxtNode*)this)->GetNum()->GetLevel() );
                pCColl = ((SwConditionTxtFmtColl*)GetFmtColl())->
                                HasCondition( aTmp );
            }
            else
                pCColl = 0;

            if( pCColl )
                SetCondFmtColl( pCColl->GetTxtFmtColl() );
            else if( pCondColl )
                SetCondFmtColl( 0 );
        }
    }
}

//FEATURE::CONDCOLL
// Metoden aus Node.hxx - erst hier ist der TxtNode bekannt !!
// os: nur fuer ICC, da der zum optimieren zu dumm ist
#ifdef ICC
SwTxtNode   *SwNode::GetTxtNode()
{
     return ND_TEXTNODE == nNodeType ? (SwTxtNode*)this : 0;
}
const SwTxtNode   *SwNode::GetTxtNode() const
{
     return ND_TEXTNODE == nNodeType ? (const SwTxtNode*)this : 0;
}
#endif




