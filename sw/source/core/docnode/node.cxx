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
#include <editeng/frmdiritem.hxx>
#include <editeng/protitem.hxx>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <txtftn.hxx>
#include <ftnfrm.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <node.hxx>
#include <ndindex.hxx>
#include <numrule.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <swcache.hxx>
#include <section.hxx>
#include <cntfrm.hxx>
#include <flyfrm.hxx>
#include <txtfrm.hxx>
#include <tabfrm.hxx>  // SwTabFrm
#include <viewsh.hxx>
#include <paratr.hxx>
#include <ftnidx.hxx>
#include <fmtftn.hxx>
#include <fmthdft.hxx>
#include <frmatr.hxx>
#include <fmtautofmt.hxx>
#include <frmtool.hxx>
#include <pagefrm.hxx>
#include <node2lay.hxx>
#include <pagedesc.hxx>
#include <fmtpdsc.hxx>
#include <breakit.hxx>
#include <crsskip.hxx>
#include <SwStyleNameMapper.hxx>
#include <scriptinfo.hxx>
#include <rootfrm.hxx>
#include <istyleaccess.hxx>
#include <IDocumentListItems.hxx>
#include <switerator.hxx>
#include "ndole.hxx"

using namespace ::com::sun::star::i18n;

TYPEINIT2( SwCntntNode, SwModify, SwIndexReg )

/*
 * Some local helper functions for the attribute set handle of a content node.
 * Since the attribute set of a content node may not be modified directly,
 * we always have to create a new SwAttrSet, do the modifications, and get
 * a new handle from the style access
 */

namespace AttrSetHandleHelper
{

void GetNewAutoStyle( boost::shared_ptr<const SfxItemSet>& mrpAttrSet,
                      const SwCntntNode& rNode,
                      SwAttrSet& rNewAttrSet )
{
    const SwAttrSet* pAttrSet = static_cast<const SwAttrSet*>(mrpAttrSet.get());
    if( rNode.GetModifyAtAttr() )
        const_cast<SwAttrSet*>(pAttrSet)->SetModifyAtAttr( 0 );
    IStyleAccess& rSA = pAttrSet->GetPool()->GetDoc()->GetIStyleAccess();
    mrpAttrSet = rSA.getAutomaticStyle( rNewAttrSet, rNode.IsTxtNode() ?
                                                     IStyleAccess::AUTO_STYLE_PARA :
                                                     IStyleAccess::AUTO_STYLE_NOTXT );
    const bool bSetModifyAtAttr = ((SwAttrSet*)mrpAttrSet.get())->SetModifyAtAttr( &rNode );
    rNode.SetModifyAtAttr( bSetModifyAtAttr );
}


void SetParent( boost::shared_ptr<const SfxItemSet>& mrpAttrSet,
                const SwCntntNode& rNode,
                const SwFmt* pParentFmt,
                const SwFmt* pConditionalFmt )
{
    const SwAttrSet* pAttrSet = static_cast<const SwAttrSet*>(mrpAttrSet.get());
    OSL_ENSURE( pAttrSet, "no SwAttrSet" );
    OSL_ENSURE( pParentFmt || !pConditionalFmt, "ConditionalFmt without ParentFmt?" );

    const SwAttrSet* pParentSet = pParentFmt ? &pParentFmt->GetAttrSet() : 0;

    if ( pParentSet != pAttrSet->GetParent() )
    {
        SwAttrSet aNewSet( *pAttrSet );
        aNewSet.SetParent( pParentSet );
        aNewSet.ClearItem( RES_FRMATR_STYLE_NAME );
        aNewSet.ClearItem( RES_FRMATR_CONDITIONAL_STYLE_NAME );
        String sVal;

        if ( pParentFmt )
        {
            SwStyleNameMapper::FillProgName( pParentFmt->GetName(), sVal, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True );
            const SfxStringItem aAnyFmtColl( RES_FRMATR_STYLE_NAME, sVal );
            aNewSet.Put( aAnyFmtColl );

            if ( pConditionalFmt != pParentFmt )
                SwStyleNameMapper::FillProgName( pConditionalFmt->GetName(), sVal, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True );

            const SfxStringItem aFmtColl( RES_FRMATR_CONDITIONAL_STYLE_NAME, sVal );
            aNewSet.Put( aFmtColl );
        }

        GetNewAutoStyle( mrpAttrSet, rNode, aNewSet );
    }
}

const SfxPoolItem* Put( boost::shared_ptr<const SfxItemSet>& mrpAttrSet,
                        const SwCntntNode& rNode,
                        const SfxPoolItem& rAttr )
{
    SwAttrSet aNewSet( (SwAttrSet&)*mrpAttrSet );
    const SfxPoolItem* pRet = aNewSet.Put( rAttr );
    if ( pRet )
        GetNewAutoStyle( mrpAttrSet, rNode, aNewSet );
    return pRet;
}

int Put( boost::shared_ptr<const SfxItemSet>& mrpAttrSet, const SwCntntNode& rNode,
         const SfxItemSet& rSet )
{
    SwAttrSet aNewSet( (SwAttrSet&)*mrpAttrSet );

    // #i76273# Robust
    SfxItemSet* pStyleNames = 0;
    if ( SFX_ITEM_SET == rSet.GetItemState( RES_FRMATR_STYLE_NAME, sal_False ) )
    {
        pStyleNames = new SfxItemSet( *aNewSet.GetPool(), RES_FRMATR_STYLE_NAME, RES_FRMATR_CONDITIONAL_STYLE_NAME );
        pStyleNames->Put( aNewSet );
    }

    const int nRet = aNewSet.Put( rSet );

    // #i76273# Robust
    if ( pStyleNames )
    {
        aNewSet.Put( *pStyleNames );
        delete pStyleNames;
    }

    if ( nRet )
        GetNewAutoStyle( mrpAttrSet, rNode, aNewSet );

    return nRet;
}

int Put_BC( boost::shared_ptr<const SfxItemSet>& mrpAttrSet,
            const SwCntntNode& rNode, const SfxPoolItem& rAttr,
            SwAttrSet* pOld, SwAttrSet* pNew )
{
    SwAttrSet aNewSet( (SwAttrSet&)*mrpAttrSet );

    // for a correct broadcast, we need to do a SetModifyAtAttr with the items
    // from aNewSet. The 'regular' SetModifyAtAttr is done in GetNewAutoStyle
    if( rNode.GetModifyAtAttr() )
        aNewSet.SetModifyAtAttr( &rNode );

    const int nRet = aNewSet.Put_BC( rAttr, pOld, pNew );

    if ( nRet )
        GetNewAutoStyle( mrpAttrSet, rNode, aNewSet );

    return nRet;
}

int Put_BC( boost::shared_ptr<const SfxItemSet>& mrpAttrSet,
            const SwCntntNode& rNode, const SfxItemSet& rSet,
            SwAttrSet* pOld, SwAttrSet* pNew )
{
    SwAttrSet aNewSet( (SwAttrSet&)*mrpAttrSet );

    // #i76273# Robust
    SfxItemSet* pStyleNames = 0;
    if ( SFX_ITEM_SET == rSet.GetItemState( RES_FRMATR_STYLE_NAME, sal_False ) )
    {
        pStyleNames = new SfxItemSet( *aNewSet.GetPool(), RES_FRMATR_STYLE_NAME, RES_FRMATR_CONDITIONAL_STYLE_NAME );
        pStyleNames->Put( aNewSet );
    }

    // for a correct broadcast, we need to do a SetModifyAtAttr with the items
    // from aNewSet. The 'regular' SetModifyAtAttr is done in GetNewAutoStyle
    if( rNode.GetModifyAtAttr() )
        aNewSet.SetModifyAtAttr( &rNode );

    const int nRet = aNewSet.Put_BC( rSet, pOld, pNew );

    // #i76273# Robust
    if ( pStyleNames )
    {
        aNewSet.Put( *pStyleNames );
        delete pStyleNames;
    }

    if ( nRet )
        GetNewAutoStyle( mrpAttrSet, rNode, aNewSet );

    return nRet;
}

sal_uInt16 ClearItem_BC( boost::shared_ptr<const SfxItemSet>& mrpAttrSet,
                     const SwCntntNode& rNode, sal_uInt16 nWhich,
                     SwAttrSet* pOld, SwAttrSet* pNew )
{
    SwAttrSet aNewSet( (SwAttrSet&)*mrpAttrSet );
    if( rNode.GetModifyAtAttr() )
        aNewSet.SetModifyAtAttr( &rNode );
    const sal_uInt16 nRet = aNewSet.ClearItem_BC( nWhich, pOld, pNew );
    if ( nRet )
        GetNewAutoStyle( mrpAttrSet, rNode, aNewSet );
    return nRet;
}

sal_uInt16 ClearItem_BC( boost::shared_ptr<const SfxItemSet>& mrpAttrSet,
                     const SwCntntNode& rNode,
                     sal_uInt16 nWhich1, sal_uInt16 nWhich2,
                     SwAttrSet* pOld, SwAttrSet* pNew )
{
    SwAttrSet aNewSet( (SwAttrSet&)*mrpAttrSet );
    if( rNode.GetModifyAtAttr() )
        aNewSet.SetModifyAtAttr( &rNode );
    const sal_uInt16 nRet = aNewSet.ClearItem_BC( nWhich1, nWhich2, pOld, pNew );
    if ( nRet )
        GetNewAutoStyle( mrpAttrSet, rNode, aNewSet );
    return nRet;
}

}

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
*******************************************************************/


sal_uInt16 SwNode::GetSectionLevel() const
{
    // EndNode einer Grund-Section ?? diese sind immer 0 !!
    if( IsEndNode() && 0 == pStartOfSection->StartOfSectionIndex() )
        return 0;

    sal_uInt16 nLevel;
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
*******************************************************************/

#ifdef DBG_UTIL
long SwNode::s_nSerial = 0;
#endif

SwNode::SwNode( const SwNodeIndex &rWhere, const sal_uInt8 nNdType )
    : nNodeType( nNdType ), pStartOfSection( 0 )
{
    bSetNumLSpace = bIgnoreDontExpand = sal_False;
    nAFmtNumLvl = 0;

    SwNodes& rNodes = (SwNodes&)rWhere.GetNodes();
    SwNode* pInsNd = this;      // der MAC kann this nicht einfuegen !!
    if( rWhere.GetIndex() )
    {
        SwNode* pNd = rNodes[ rWhere.GetIndex() -1 ];
        rNodes.InsertNode( pInsNd, rWhere );
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
        rNodes.InsertNode( pInsNd, rWhere );
        pStartOfSection = (SwStartNode*)this;
    }

#ifdef DBG_UTIL
    m_nSerial = s_nSerial;
    s_nSerial++;
#endif
}

SwNode::SwNode( SwNodes& rNodes, sal_uLong nPos, const sal_uInt8 nNdType )
    : nNodeType( nNdType ), pStartOfSection( 0 )
{
    bSetNumLSpace = bIgnoreDontExpand = sal_False;
    nAFmtNumLvl = 0;

    SwNode* pInsNd = this;      // der MAC kann this nicht einfuegen !!
    if( nPos )
    {
        SwNode* pNd = rNodes[ nPos - 1 ];
        rNodes.InsertNode( pInsNd, nPos );
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
        rNodes.InsertNode( pInsNd, nPos );
        pStartOfSection = (SwStartNode*)this;
    }

#ifdef DBG_UTIL
    m_nSerial = s_nSerial;
    s_nSerial++;
#endif
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
        pTmp = pTmp->pStartOfSection;
    return pTmp->GetTableNode();
}


// liegt der Node im Sichtbarenbereich der Shell ?
sal_Bool SwNode::IsInVisibleArea( ViewShell* pSh ) const
{
    sal_Bool bRet = sal_False;
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

    if( !pSh )
        // dann die Shell vom Doc besorgen:
        GetDoc()->GetEditShell( &pSh );

    if( pSh )
    {
        const SwFrm* pFrm;
        if( pNd && 0 != ( pFrm = pNd->getLayoutFrm( pSh->GetLayout(), 0, 0, sal_False ) ) )
        {

            if ( pFrm->IsInTab() )
                pFrm = pFrm->FindTabFrm();

            if( !pFrm->IsValid() )
                do
                {   pFrm = pFrm->FindPrev();
                } while ( pFrm && !pFrm->IsValid() );

            if( !pFrm || pSh->VisArea().IsOver( pFrm->Frm() ) )
                bRet = sal_True;
        }
    }

    return bRet;
}

sal_Bool SwNode::IsInProtectSect() const
{
    const SwNode* pNd = ND_SECTIONNODE == nNodeType ? pStartOfSection : this;
    const SwSectionNode* pSectNd = pNd->FindSectionNode();
    return pSectNd && pSectNd->GetSection().IsProtectFlag();
}

    // befindet sich der Node in irgendetwas geschuetzten ?
    // (Bereich/Rahmen/Tabellenzellen/... incl. des Ankers bei
    //  Rahmen/Fussnoten/..)
sal_Bool SwNode::IsProtect() const
{
    const SwNode* pNd = ND_SECTIONNODE == nNodeType ? pStartOfSection : this;
    const SwStartNode* pSttNd = pNd->FindSectionNode();
    if( pSttNd && ((SwSectionNode*)pSttNd)->GetSection().IsProtectFlag() )
        return sal_True;

    if( 0 != ( pSttNd = FindTableBoxStartNode() ) )
    {
        SwCntntFrm* pCFrm;
        if( IsCntntNode() && 0 != (pCFrm = ((SwCntntNode*)this)->getLayoutFrm( GetDoc()->GetCurrentLayout() ) ))
            return pCFrm->IsProtected();

        const SwTableBox* pBox = pSttNd->FindTableNode()->GetTable().
                                        GetTblBox( pSttNd->GetIndex() );
        //Robust #149568
        if( pBox && pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
            return sal_True;
    }

    SwFrmFmt* pFlyFmt = GetFlyFmt();
    if( pFlyFmt )
    {
        if( pFlyFmt->GetProtect().IsCntntProtected() )
            return sal_True;
        const SwFmtAnchor& rAnchor = pFlyFmt->GetAnchor();
        return rAnchor.GetCntntAnchor()
                ? rAnchor.GetCntntAnchor()->nNode.GetNode().IsProtect()
                : sal_False;
    }

    if( 0 != ( pSttNd = FindFootnoteStartNode() ) )
    {
        const SwTxtFtn* pTFtn = GetDoc()->GetFtnIdxs().SeekEntry(
                                SwNodeIndex( *pSttNd ) );
        if( pTFtn )
            return pTFtn->GetTxtNode().IsProtect();
    }

    return sal_False;
}

    // suche den PageDesc, mit dem dieser Node formatiert ist. Wenn das
    // Layout vorhanden ist wird ueber das gesucht, ansonsten gibt es nur
    // die harte Tour ueber die Nodes nach vorne suchen!!
const SwPageDesc* SwNode::FindPageDesc( sal_Bool bCalcLay,
                                        sal_uInt32* pPgDescNdIdx ) const
{
    if ( !GetNodes().IsDocNodes() )
    {
        return 0;
    }

    const SwPageDesc* pPgDesc = 0;

    const SwCntntNode* pNode;
    if( ND_STARTNODE & nNodeType )
    {
        SwNodeIndex aIdx( *this );
        pNode = GetNodes().GoNext( &aIdx );
    }
    else if( ND_ENDNODE & nNodeType )
    {
        SwNodeIndex aIdx( *EndOfSectionNode() );
        pNode = GetNodes().GoPrevious( &aIdx );
    }
    else
    {
        pNode = GetCntntNode();
        if( pNode )
            pPgDesc = ((SwFmtPageDesc&)pNode->GetAttr( RES_PAGEDESC )).GetPageDesc();
    }

    // geht es uebers Layout?
    if( !pPgDesc )
    {
        const SwFrm* pFrm;
        const SwPageFrm* pPage;
        if( pNode && 0 != ( pFrm = pNode->getLayoutFrm( pNode->GetDoc()->GetCurrentLayout(), 0, 0, bCalcLay ) ) &&
            0 != ( pPage = pFrm->FindPageFrm() ) )
        {
            pPgDesc = pPage->GetPageDesc();
            if ( pPgDescNdIdx )
            {
                *pPgDescNdIdx = pNode->GetIndex();
            }
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
            const SwFrmFmts& rFmts = *pDoc->GetSpzFrmFmts();
            sal_uInt16 n;

            for( n = 0; n < rFmts.size(); ++n )
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
                if ((FLY_AT_PAGE != pAnchor->GetAnchorId()) &&
                    pAnchor->GetCntntAnchor() )
                {
                    pNd = &pAnchor->GetCntntAnchor()->nNode.GetNode();
                    const SwNode* pFlyNd = pNd->FindFlyStartNode();
                    while( pFlyNd )
                    {
                        // dann ueber den Anker nach oben "hangeln"
                        for( n = 0; n < rFmts.size(); ++n )
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
                                if ((FLY_AT_PAGE == pAnchor->GetAnchorId()) ||
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
                        if( n >= rFmts.size() )
                        {
                            OSL_ENSURE( !this, "Fly-Section aber kein Format gefunden" );
                            return 0;
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
                    sal_uInt16 nId;
                    UseOnPage eAskUse;
                    if( SwHeaderStartNode == pSttNd->GetStartNodeType())
                    {
                        nId = RES_HEADER;
                        eAskUse = nsUseOnPage::PD_HEADERSHARE;
                    }
                    else
                    {
                        nId = RES_FOOTER;
                        eAskUse = nsUseOnPage::PD_FOOTERSHARE;
                    }

                    for( sal_uInt16 n = pDoc->GetPageDescCnt(); n && !pPgDesc; )
                    {
                        const SwPageDesc& rPgDsc = pDoc->GetPageDesc( --n );
                        const SwFrmFmt* pFmt = &rPgDsc.GetMaster();
                        int nStt = 0, nLast = 1;
                        if( !( eAskUse & rPgDsc.ReadUseOn() )) ++nLast;

                        for( ; nStt < nLast; ++nStt, pFmt = &rPgDsc.GetLeft() )
                        {
                            const SwFmtHeader& rHdFt = (SwFmtHeader&)
                                                    pFmt->GetFmtAttr( nId );
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
                    for( sal_uInt16 n = 0; n < rFtnArr.size(); ++n )
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
                    OSL_ENSURE( pNd->FindFlyStartNode(),
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
            sal_uInt32 i, nMaxItems = pDoc->GetAttrPool().GetItemCount2( RES_PAGEDESC );
            for( i = 0; i < nMaxItems; ++i )
                if( 0 != (pItem = pDoc->GetAttrPool().GetItem2( RES_PAGEDESC, i ) ) &&
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
                if ( pPgDescNdIdx )
                {
                    *pPgDescNdIdx = pNd->GetIndex();
                }
            }
            if( !pPgDesc )
                pPgDesc = &pDoc->GetPageDesc( 0 );
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
            SwCntntFrm* pFrm = SwIterator<SwCntntFrm,SwCntntNode>::FirstElement( *(SwCntntNode*)this );
            if( pFrm )
                pRet = pFrm->FindFlyFrm()->GetFmt();
        }
        if( !pRet )
        {
            // dann gibts noch harten steinigen Weg uebers Dokument:
            const SwFrmFmts& rFrmFmtTbl = *GetDoc()->GetSpzFrmFmts();
            for( sal_uInt16 n = 0; n < rFrmFmtTbl.size(); ++n )
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
        pTmp = pTmp->pStartOfSection;
    return eTyp == pTmp->GetStartNodeType() ? pTmp : 0;
}

const SwTxtNode* SwNode::FindOutlineNodeOfLevel( sal_uInt8 nLvl ) const
{
    const SwTxtNode* pRet = 0;
    const SwOutlineNodes& rONds = GetNodes().GetOutLineNds();
    if( MAXLEVEL > nLvl && !rONds.empty() )
    {
        sal_uInt16 nPos;
        SwNode* pNd = (SwNode*)this;
        sal_Bool bCheckFirst = sal_False;
        if( !rONds.Seek_Entry( pNd, &nPos ))
        {
            if( nPos )
                nPos = nPos-1;
            else
                bCheckFirst = sal_True;
        }

        if( bCheckFirst )
        {
            // der 1.GliederungsNode liegt hinter dem Fragenden. Dann
            // teste mal, ob dieser auf der gleichen Seite steht. Wenn
            // nicht, ist das ein ungueltiger. Bug 61865
            pRet = rONds[0]->GetTxtNode();

            const SwCntntNode* pCNd = GetCntntNode();

            Point aPt( 0, 0 );
            const SwFrm* pFrm = pRet->getLayoutFrm( pRet->GetDoc()->GetCurrentLayout(), &aPt, 0, sal_False ),
                       * pMyFrm = pCNd ? pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout(), &aPt, 0, sal_False ) : 0;
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
            while( nPos &&
                   nLvl < ( pRet = rONds[nPos]->GetTxtNode() )
                    ->GetAttrOutlineLevel() - 1 )  //<-end,zhaojianwei
                --nPos;

            if( !nPos )     // bei 0 gesondert holen !!
                pRet = rONds[0]->GetTxtNode();
        }
    }
    return pRet;
}

inline sal_Bool IsValidNextPrevNd( const SwNode& rNd )
{
    return ND_TABLENODE == rNd.GetNodeType() ||
           ( ND_CONTENTNODE & rNd.GetNodeType() ) ||
            ( ND_ENDNODE == rNd.GetNodeType() && rNd.StartOfSectionNode() &&
            ND_TABLENODE == rNd.StartOfSectionNode()->GetNodeType() );
}

sal_uInt8 SwNode::HasPrevNextLayNode() const
{
    // assumption: <this> node is a node inside the document nodes array section.

    sal_uInt8 nRet = 0;
    if( IsValidNextPrevNd( *this ))
    {
        SwNodeIndex aIdx( *this, -1 );
        // #i77805# - skip section start and end nodes
        while ( aIdx.GetNode().IsSectionNode() ||
                ( aIdx.GetNode().IsEndNode() &&
                  aIdx.GetNode().StartOfSectionNode()->IsSectionNode() ) )
        {
            --aIdx;
        }
        if( IsValidNextPrevNd( aIdx.GetNode() ))
            nRet |= ND_HAS_PREV_LAYNODE;
        // #i77805# - skip section start and end nodes
        aIdx = SwNodeIndex( *this, +1 );
        while ( aIdx.GetNode().IsSectionNode() ||
                ( aIdx.GetNode().IsEndNode() &&
                  aIdx.GetNode().StartOfSectionNode()->IsSectionNode() ) )
        {
            ++aIdx;
        }
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
|*
*******************************************************************/


SwStartNode::SwStartNode( const SwNodeIndex &rWhere, const sal_uInt8 nNdType,
                            SwStartNodeType eSttNd )
    : SwNode( rWhere, nNdType ), eSttNdTyp( eSttNd )
{
    // erstmal temporaer, bis der EndNode eingefuegt wird.
    pEndOfSection = (SwEndNode*)this;
}

SwStartNode::SwStartNode( SwNodes& rNodes, sal_uLong nPos )
    : SwNode( rNodes, nPos, ND_STARTNODE ), eSttNdTyp( SwNormalStartNode )
{
    // erstmal temporaer, bis der EndNode eingefuegt wird.
    pEndOfSection = (SwEndNode*)this;
}


void SwStartNode::CheckSectionCondColl() const
{
//FEATURE::CONDCOLL
    SwNodeIndex aIdx( *this );
    sal_uLong nEndIdx = EndOfSectionIndex();
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
*******************************************************************/


SwEndNode::SwEndNode( const SwNodeIndex &rWhere, SwStartNode& rSttNd )
    : SwNode( rWhere, ND_ENDNODE )
{
    pStartOfSection = &rSttNd;
    pStartOfSection->pEndOfSection = this;
}

SwEndNode::SwEndNode( SwNodes& rNds, sal_uLong nPos, SwStartNode& rSttNd )
    : SwNode( rNds, nPos, ND_ENDNODE )
{
    pStartOfSection = &rSttNd;
    pStartOfSection->pEndOfSection = this;
}



// --------------------
// SwCntntNode
// --------------------


SwCntntNode::SwCntntNode( const SwNodeIndex &rWhere, const sal_uInt8 nNdType,
                            SwFmtColl *pColl )
    : SwModify( pColl ),     // CrsrsShell, FrameFmt,
    SwNode( rWhere, nNdType ),
    pCondColl( 0 ),
    mbSetModifyAtAttr( false )
{
}


SwCntntNode::~SwCntntNode()
{
    // Die Basisklasse SwClient vom SwFrm nimmt sich aus
    // der Abhaengikeitsliste raus!
    // Daher muessen alle Frames in der Abhaengigkeitsliste geloescht werden.
    if( GetDepends() )
        DelFrms();

    delete pCondColl;

    if ( mpAttrSet.get() && mbSetModifyAtAttr )
        ((SwAttrSet*)mpAttrSet.get())->SetModifyAtAttr( 0 );
}

void SwCntntNode::Modify( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue )
{
    sal_uInt16 nWhich = pOldValue ? pOldValue->Which() :
                    pNewValue ? pNewValue->Which() : 0 ;

    switch( nWhich )
    {
    case RES_OBJECTDYING :
        {
            SwFmt * pFmt = (SwFmt *) ((SwPtrMsgPoolItem *)pNewValue)->pObject;

            // nicht umhaengen wenn dieses das oberste Format ist !!
            if( GetRegisteredIn() == pFmt )
            {
                if( pFmt->GetRegisteredIn() )
                {
                    // wenn Parent, dann im neuen Parent wieder anmelden
                    ((SwModify*)pFmt->GetRegisteredIn())->Add( this );
                    if ( GetpSwAttrSet() )
                        AttrSetHandleHelper::SetParent( mpAttrSet, *this, GetFmtColl(), GetFmtColl() );
                }
                else
                {
                    // sonst auf jeden Fall beim sterbenden abmelden
                    ((SwModify*)GetRegisteredIn())->Remove( this );
                    if ( GetpSwAttrSet() )
                        AttrSetHandleHelper::SetParent( mpAttrSet, *this, 0, 0 );
                }
            }
        }
        break;


    case RES_FMT_CHG:
        // falls mein Format Parent umgesetzt wird, dann melde ich
        // meinen Attrset beim Neuen an.

        // sein eigenes Modify ueberspringen !!
        if( GetpSwAttrSet() &&
            ((SwFmtChg*)pNewValue)->pChangedFmt == GetRegisteredIn() )
        {
            // den Set an den neuen Parent haengen
            AttrSetHandleHelper::SetParent( mpAttrSet, *this, GetFmtColl(), GetFmtColl() );
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
            if( SFX_ITEM_SET == ((SwAttrSetChg*)pOldValue)->GetChgSet()->GetItemState(
                RES_CHRATR_HIDDEN, sal_False ) )
            {
                ((SwTxtNode*)this)->SetCalcHiddenCharFlags();
            }
        }
        break;

    case RES_UPDATE_ATTR:
        if( GetNodes().IsDocNodes() && IsTxtNode() )
        {
            const sal_uInt16 nTmp = ((SwUpdateAttr*)pNewValue)->nWhichAttr;
            if ( RES_ATTRSET_CHG == nTmp )
            {
                // anybody wants to do some optimization here?
                ((SwTxtNode*)this)->SetCalcHiddenCharFlags();
            }
        }
        break;
    }

    NotifyClients( pOldValue, pNewValue );
}

sal_Bool SwCntntNode::InvalidateNumRule()
{
    SwNumRule* pRule = 0;
    const SfxPoolItem* pItem;
    if( GetNodes().IsDocNodes() &&
        0 != ( pItem = GetNoCondAttr( RES_PARATR_NUMRULE, sal_True )) &&
        ((SwNumRuleItem*)pItem)->GetValue().Len() &&
        0 != (pRule = GetDoc()->FindNumRulePtr(
                                ((SwNumRuleItem*)pItem)->GetValue() ) ) )
    {
        pRule->SetInvalidRule( sal_True );
    }
    return 0 != pRule;
}

SwCntntFrm *SwCntntNode::getLayoutFrm( const SwRootFrm* _pRoot,
    const Point* pPoint, const SwPosition *pPos, const sal_Bool bCalcFrm ) const
{
    return (SwCntntFrm*) ::GetFrmOfModify( _pRoot, *(SwModify*)this, FRM_CNTNT,
                                            pPoint, pPos, bCalcFrm );
}

SwRect SwCntntNode::FindLayoutRect( const sal_Bool bPrtArea, const Point* pPoint,
                                    const sal_Bool bCalcFrm ) const
{
    SwRect aRet;
    SwCntntFrm* pFrm = (SwCntntFrm*)::GetFrmOfModify( 0, *(SwModify*)this,
                                            FRM_CNTNT, pPoint, 0, bCalcFrm );
    if( pFrm )
        aRet = bPrtArea ? pFrm->Prt() : pFrm->Frm();
    return aRet;
}

SwRect SwCntntNode::FindPageFrmRect( const sal_Bool bPrtArea, const Point* pPoint,
                                    const sal_Bool bCalcFrm ) const
{
    SwRect aRet;
    SwFrm* pFrm = ::GetFrmOfModify( 0, *(SwModify*)this,
                                            FRM_CNTNT, pPoint, 0, bCalcFrm );
    if( pFrm && 0 != ( pFrm = pFrm->FindPageFrm() ))
        aRet = bPrtArea ? pFrm->Prt() : pFrm->Frm();
    return aRet;
}

xub_StrLen SwCntntNode::Len() const { return 0; }



SwFmtColl *SwCntntNode::ChgFmtColl( SwFmtColl *pNewColl )
{
    OSL_ENSURE( pNewColl, "Collectionpointer ist 0." );
    SwFmtColl *pOldColl = GetFmtColl();

    if( pNewColl != pOldColl )
    {
        pNewColl->Add( this );

        // setze den Parent von unseren Auto-Attributen auf die neue
        // Collection:
        if( GetpSwAttrSet() )
            AttrSetHandleHelper::SetParent( mpAttrSet, *this, pNewColl, pNewColl );

//FEATURE::CONDCOLL
        // HACK: hier muss die entsprechend der neuen Vorlage die Bedingungen
        //      neu ueberprueft werden!
        if( sal_True /*pNewColl */ )
        {
            SetCondFmtColl( 0 );
        }
//FEATURE::CONDCOLL

        if( !IsModifyLocked() )
        {
            SwFmtChg aTmp1( pOldColl );
            SwFmtChg aTmp2( pNewColl );
            SwCntntNode::Modify( &aTmp1, &aTmp2 );
        }
    }
    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    return pOldColl;
}


sal_Bool SwCntntNode::GoNext(SwIndex * pIdx, sal_uInt16 nMode ) const
{
    sal_Bool bRet = sal_True;
    if( pIdx->GetIndex() < Len() )
    {
        if( !IsTxtNode() )
            (*pIdx)++;
        else
        {
            const SwTxtNode& rTNd = *GetTxtNode();
            xub_StrLen nPos = pIdx->GetIndex();
            if( pBreakIt->GetBreakIter().is() )
            {
                sal_Int32 nDone = 0;
                sal_uInt16 nItrMode = ( CRSR_SKIP_CELLS & nMode ) ?
                                        CharacterIteratorMode::SKIPCELL :
                                        CharacterIteratorMode::SKIPCONTROLCHARACTER;
                nPos = (xub_StrLen)pBreakIt->GetBreakIter()->nextCharacters( rTNd.GetTxt(), nPos,
                                   pBreakIt->GetLocale( rTNd.GetLang( nPos ) ),
                                   nItrMode, 1, nDone );

                // Check if nPos is inside hidden text range:
                if ( CRSR_SKIP_HIDDEN & nMode )
                {
                    xub_StrLen nHiddenStart;
                    xub_StrLen nHiddenEnd;
                    SwScriptInfo::GetBoundsOfHiddenRange( rTNd, nPos, nHiddenStart, nHiddenEnd );
                    if ( nHiddenStart != STRING_LEN && nHiddenStart != nPos )
                         nPos = nHiddenEnd;
                }

                if( 1 == nDone )
                    *pIdx = nPos;
                else
                    bRet = sal_False;
            }
            else if( nPos < rTNd.GetTxt().Len() )
                (*pIdx)++;
            else
                bRet = sal_False;
        }
    }
    else
        bRet = sal_False;
    return bRet;
}


sal_Bool SwCntntNode::GoPrevious(SwIndex * pIdx, sal_uInt16 nMode ) const
{
    sal_Bool bRet = sal_True;
    if( pIdx->GetIndex() > 0 )
    {
        if( !IsTxtNode() )
            (*pIdx)--;
        else
        {
            const SwTxtNode& rTNd = *GetTxtNode();
            xub_StrLen nPos = pIdx->GetIndex();
            if( pBreakIt->GetBreakIter().is() )
            {
                sal_Int32 nDone = 0;
                sal_uInt16 nItrMode = ( CRSR_SKIP_CELLS & nMode ) ?
                                        CharacterIteratorMode::SKIPCELL :
                                        CharacterIteratorMode::SKIPCONTROLCHARACTER;
                nPos = (xub_StrLen)pBreakIt->GetBreakIter()->previousCharacters( rTNd.GetTxt(), nPos,
                                   pBreakIt->GetLocale( rTNd.GetLang( nPos ) ),
                                   nItrMode, 1, nDone );

                // Check if nPos is inside hidden text range:
                if ( CRSR_SKIP_HIDDEN & nMode )
                {
                    xub_StrLen nHiddenStart;
                    xub_StrLen nHiddenEnd;
                    SwScriptInfo::GetBoundsOfHiddenRange( rTNd, nPos, nHiddenStart, nHiddenEnd );
                    if ( nHiddenStart != STRING_LEN  )
                         nPos = nHiddenStart;
                }

                if( 1 == nDone )
                    *pIdx = nPos;
                else
                    bRet = sal_False;
            }
            else if( nPos )
                (*pIdx)--;
            else
                bRet = sal_False;
        }
    }
    else
        bRet = sal_False;
    return bRet;
}


/*
 * Methode erzeugt fuer den vorhergehenden Node alle Ansichten vom
 * Dokument. Die erzeugten Contentframes werden in das entsprechende
 * Layout gehaengt.
 */


void SwCntntNode::MakeFrms( SwCntntNode& rNode )
{
    OSL_ENSURE( &rNode != this,
            "Kein Contentnode oder Copy-Node und neuer Node identisch." );

    if( !GetDepends() || &rNode == this )   // gibt es ueberhaupt Frames ??
        return;

    SwFrm *pFrm, *pNew;
    SwLayoutFrm *pUpper;
    // Frames anlegen fuer Nodes, die vor oder hinter der Tabelle stehen ??
    OSL_ENSURE( FindTableNode() == rNode.FindTableNode(), "Table confusion" );

    SwNode2Layout aNode2Layout( *this, rNode.GetIndex() );

    while( 0 != (pUpper = aNode2Layout.UpperFrm( pFrm, rNode )) )
    {
        pNew = rNode.MakeFrm( pUpper );
        pNew->Paste( pUpper, pFrm );
        // #i27138#
        // notify accessibility paragraphs objects about changed
        // CONTENT_FLOWS_FROM/_TO relation.
        // Relation CONTENT_FLOWS_FROM for next paragraph will change
        // and relation CONTENT_FLOWS_TO for previous paragraph will change.
        if ( pNew->IsTxtFrm() )
        {
            ViewShell* pViewShell( pNew->getRootFrm()->GetCurrShell() );
            if ( pViewShell && pViewShell->GetLayout() &&
                 pViewShell->GetLayout()->IsAnyShellAccessible() )
            {
                pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTxtFrm*>(pNew->FindNextCnt( true )),
                            dynamic_cast<SwTxtFrm*>(pNew->FindPrevCnt( true )) );
            }
        }
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

    SwCntntFrm::DelFrms(*this);
    if( IsTxtNode() )
    {
        ((SwTxtNode*)this)->SetWrong( NULL );
        ((SwTxtNode*)this)->SetWrongDirty( true );

        ((SwTxtNode*)this)->SetGrammarCheck( NULL );
        ((SwTxtNode*)this)->SetGrammarCheckDirty( true );
        // SMARTTAGS
        ((SwTxtNode*)this)->SetSmartTags( NULL );
        ((SwTxtNode*)this)->SetSmartTagDirty( true );

        ((SwTxtNode*)this)->SetWordCountDirty( true );
        ((SwTxtNode*)this)->SetAutoCompleteWordDirty( true );
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
sal_Bool SwCntntNode::GetInfo( SfxPoolItem& rInfo ) const
{
    switch( rInfo.Which() )
    {
    case RES_AUTOFMT_DOCNODE:
        if( &GetNodes() == ((SwAutoFmtGetDocNode&)rInfo).pNodes )
        {
            ((SwAutoFmtGetDocNode&)rInfo).pCntntNode = this;
            return sal_False;
        }
        break;

    case RES_FINDNEARESTNODE:
        if( ((SwFmtPageDesc&)GetAttr( RES_PAGEDESC )).GetPageDesc() )
            ((SwFindNearestNode&)rInfo).CheckNode( *this );
        return sal_True;

    case RES_CONTENT_VISIBLE:
        {
            ((SwPtrMsgPoolItem&)rInfo).pObject =
                SwIterator<SwFrm,SwCntntNode>::FirstElement(*this);
        }
        return sal_False;
    }

    return SwModify::GetInfo( rInfo );
}


    // setze ein Attribut
sal_Bool SwCntntNode::SetAttr(const SfxPoolItem& rAttr )
{
    if( !GetpSwAttrSet() )            // lasse von den entsprechenden Nodes die
        NewAttrSet( GetDoc()->GetAttrPool() );      // AttrSets anlegen

    OSL_ENSURE( GetpSwAttrSet(), "warum wurde kein AttrSet angelegt?" );

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }

    sal_Bool bRet = sal_False;
    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if( IsModifyLocked() ||
        ( !GetDepends() &&  RES_PARATR_NUMRULE != rAttr.Which() ))
    {
        bRet = 0 != AttrSetHandleHelper::Put( mpAttrSet, *this, rAttr );
    }
    else
    {
        SwAttrSet aOld( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() ),
                  aNew( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() );
        if( 0 != ( bRet = 0 != AttrSetHandleHelper::Put_BC( mpAttrSet, *this, rAttr, &aOld, &aNew ) ))
        {
            SwAttrSetChg aChgOld( *GetpSwAttrSet(), aOld );
            SwAttrSetChg aChgNew( *GetpSwAttrSet(), aNew );
            ModifyNotification( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
        }
    }
    return bRet;
}
#include <svl/itemiter.hxx>

sal_Bool SwCntntNode::SetAttr( const SfxItemSet& rSet )
{
    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }

    const SfxPoolItem* pFnd = 0;
    if( SFX_ITEM_SET == rSet.GetItemState( RES_AUTO_STYLE, sal_False, &pFnd ) )
    {
        OSL_ENSURE( rSet.Count() == 1, "SetAutoStyle mixed with other attributes?!" );
        const SwFmtAutoFmt* pTmp = static_cast<const SwFmtAutoFmt*>(pFnd);

        // If there already is an attribute set (usually containing a numbering
        // item), we have to merge the attribute of the new set into the old set:
        bool bSetParent = true;
        if ( GetpSwAttrSet() )
        {
            bSetParent = false;
            AttrSetHandleHelper::Put( mpAttrSet, *this, *pTmp->GetStyleHandle() );
        }
        else
        {
            mpAttrSet = pTmp->GetStyleHandle();
        }

        if ( bSetParent )
        {
            // If the content node has a conditional style, we have to set the
            // string item containing the correct conditional style name (the
            // style name property has already been set during the import!)
            // In case we do not have a conditional style, we make use of the
            // fact that nobody else uses the attribute set behind the handle.
            // FME 2007-07-10 #i78124# If autostyle does not have a parent,
            // the string is empty.
            const SfxPoolItem* pNameItem = 0;
            if ( 0 != GetCondFmtColl() ||
                 SFX_ITEM_SET != mpAttrSet->GetItemState( RES_FRMATR_STYLE_NAME, sal_False, &pNameItem ) ||
                 0 == static_cast<const SfxStringItem*>(pNameItem)->GetValue().Len() )
                AttrSetHandleHelper::SetParent( mpAttrSet, *this, &GetAnyFmtColl(), GetFmtColl() );
            else
                const_cast<SfxItemSet*>(mpAttrSet.get())->SetParent( &GetFmtColl()->GetAttrSet() );
        }

        return sal_True;
    }

    if( !GetpSwAttrSet() )            // lasse von den entsprechenden Nodes die
        NewAttrSet( GetDoc()->GetAttrPool() );      // AttrSets anlegen

    sal_Bool bRet = sal_False;
    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if ( IsModifyLocked() ||
         ( !GetDepends() &&
           SFX_ITEM_SET != rSet.GetItemState( RES_PARATR_NUMRULE, sal_False ) ) )
    {
        // einige Sonderbehandlungen fuer Attribute
        bRet = 0 != AttrSetHandleHelper::Put( mpAttrSet, *this, rSet );
    }
    else
    {
        SwAttrSet aOld( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() ),
                  aNew( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() );
        if( 0 != (bRet = 0 != AttrSetHandleHelper::Put_BC( mpAttrSet, *this, rSet, &aOld, &aNew )) )
        {
            // einige Sonderbehandlungen fuer Attribute
            SwAttrSetChg aChgOld( *GetpSwAttrSet(), aOld );
            SwAttrSetChg aChgNew( *GetpSwAttrSet(), aNew );
            ModifyNotification( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
        }
    }
    return bRet;
}

// Nimmt den Hint mit nWhich aus dem Delta-Array


sal_Bool SwCntntNode::ResetAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
{
    if( !GetpSwAttrSet() )
        return sal_False;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }

    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if( IsModifyLocked() )
    {
        sal_uInt16 nDel = 0;
        if ( !nWhich2 || nWhich2 < nWhich1 )
        {
            std::vector<sal_uInt16> aClearWhichIds;
            aClearWhichIds.push_back( nWhich1 );
            nDel = ClearItemsFromAttrSet( aClearWhichIds );
        }
        else
            nDel = AttrSetHandleHelper::ClearItem_BC( mpAttrSet, *this, nWhich1, nWhich2, 0, 0 );

        if( !GetpSwAttrSet()->Count() )   // leer, dann loeschen
            mpAttrSet.reset();//DELETEZ( mpAttrSet );
        return 0 != nDel;
    }

    // sollte kein gueltiger Bereich definiert sein ?
    if( !nWhich2 || nWhich2 < nWhich1 )
        nWhich2 = nWhich1;      // dann setze auf 1. Id, nur dieses Item

    SwAttrSet aOld( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() ),
              aNew( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() );
    sal_Bool bRet = 0 != AttrSetHandleHelper::ClearItem_BC( mpAttrSet, *this, nWhich1, nWhich2, &aOld, &aNew );

    if( bRet )
    {
        SwAttrSetChg aChgOld( *GetpSwAttrSet(), aOld );
        SwAttrSetChg aChgNew( *GetpSwAttrSet(), aNew );
        ModifyNotification( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt

        if( !GetpSwAttrSet()->Count() )   // leer, dann loeschen
            mpAttrSet.reset();//DELETEZ( mpAttrSet );
    }
    return bRet;
}
sal_Bool SwCntntNode::ResetAttr( const std::vector<sal_uInt16>& rWhichArr )
{
    if( !GetpSwAttrSet() )
        return sal_False;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }

    // wenn Modify gelockt ist, werden keine Modifies verschickt
    sal_uInt16 nDel = 0;
    if( IsModifyLocked() )
    {
        std::vector<sal_uInt16> aClearWhichIds(rWhichArr);
        nDel = ClearItemsFromAttrSet( aClearWhichIds );
    }
    else
    {
        SwAttrSet aOld( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() ),
                  aNew( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() );

        std::vector<sal_uInt16>::const_iterator it;
        for ( it = rWhichArr.begin(); it != rWhichArr.end(); ++it )
            if( AttrSetHandleHelper::ClearItem_BC( mpAttrSet, *this, *it, &aOld, &aNew ))
                ++nDel;

        if( nDel )
        {
            SwAttrSetChg aChgOld( *GetpSwAttrSet(), aOld );
            SwAttrSetChg aChgNew( *GetpSwAttrSet(), aNew );
            ModifyNotification( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt
        }
    }
    if( !GetpSwAttrSet()->Count() )   // leer, dann loeschen
        mpAttrSet.reset();//DELETEZ( mpAttrSet );
    return 0 != nDel ;
}


sal_uInt16 SwCntntNode::ResetAllAttr()
{
    if( !GetpSwAttrSet() )
        return 0;

    if ( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }

    // wenn Modify gelockt ist, werden keine Modifies verschickt
    if( IsModifyLocked() )
    {
        std::vector<sal_uInt16> aClearWhichIds;
        aClearWhichIds.push_back(0);
        sal_uInt16 nDel = ClearItemsFromAttrSet( aClearWhichIds );
        if( !GetpSwAttrSet()->Count() )   // leer, dann loeschen
            mpAttrSet.reset();            // DELETEZ( mpAttrSet );
        return nDel;
    }

    SwAttrSet aOld( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() ),
              aNew( *GetpSwAttrSet()->GetPool(), GetpSwAttrSet()->GetRanges() );
    sal_Bool bRet = 0 != AttrSetHandleHelper::ClearItem_BC( mpAttrSet, *this, 0, &aOld, &aNew );

    if( bRet )
    {
        SwAttrSetChg aChgOld( *GetpSwAttrSet(), aOld );
        SwAttrSetChg aChgNew( *GetpSwAttrSet(), aNew );
        ModifyNotification( &aChgOld, &aChgNew );       // alle veraenderten werden verschickt

        if( !GetpSwAttrSet()->Count() )   // leer, dann loeschen
            mpAttrSet.reset();//DELETEZ( mpAttrSet );
    }
    return aNew.Count();
}


sal_Bool SwCntntNode::GetAttr( SfxItemSet& rSet, sal_Bool bInParent ) const
{
    if( rSet.Count() )
        rSet.ClearItem();

    const SwAttrSet& rAttrSet = GetSwAttrSet();
    if( bInParent )
        return rSet.Set( rAttrSet, sal_True ) ? sal_True : sal_False;

    rSet.Put( rAttrSet );
    return rSet.Count() ? sal_True : sal_False;
}

sal_uInt16 SwCntntNode::ClearItemsFromAttrSet( const std::vector<sal_uInt16>& rWhichIds )
{
    sal_uInt16 nRet = 0;
    if ( 0 == rWhichIds.size() )
        return nRet;

    OSL_ENSURE( GetpSwAttrSet(), "no item set" );
    SwAttrSet aNewAttrSet( *GetpSwAttrSet() );
    for ( std::vector<sal_uInt16>::const_iterator aIter = rWhichIds.begin();
          aIter != rWhichIds.end();
          ++aIter )
    {
        nRet = nRet + aNewAttrSet.ClearItem( *aIter );
    }
    if ( nRet )
        AttrSetHandleHelper::GetNewAutoStyle( mpAttrSet, *this, aNewAttrSet );

    return nRet;
}

const SfxPoolItem* SwCntntNode::GetNoCondAttr( sal_uInt16 nWhich,
                                                sal_Bool bInParents ) const
{
    const SfxPoolItem* pFnd = 0;
    if( pCondColl && pCondColl->GetRegisteredIn() )
    {
        if( !GetpSwAttrSet() || ( SFX_ITEM_SET != GetpSwAttrSet()->GetItemState(
                    nWhich, sal_False, &pFnd ) && bInParents ))
            ((SwFmt*)GetRegisteredIn())->GetItemState( nWhich, bInParents, &pFnd );
    }
    // undo change of issue #i51029#
    // Note: <GetSwAttrSet()> returns <mpAttrSet>, if set, otherwise it returns
    //       the attribute set of the paragraph style, which is valid for the
    //       content node - see file <node.hxx>
    else
    {
        GetSwAttrSet().GetItemState( nWhich, bInParents, &pFnd );
    }
    return pFnd;
}

    // koennen 2 Nodes zusammengefasst werden ?
    // in pIdx kann die 2. Position returnt werden.
int SwCntntNode::CanJoinNext( SwNodeIndex* pIdx ) const
{
    const SwNodes& rNds = GetNodes();
    sal_uInt8 nNdType = GetNodeType();
    SwNodeIndex aIdx( *this, 1 );

    const SwNode* pNd = this;
    while( aIdx < rNds.Count()-1 &&
        (( pNd = &aIdx.GetNode())->IsSectionNode() ||
            ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsSectionNode() )))
        aIdx++;

    if( pNd->GetNodeType() != nNdType || rNds.Count()-1 == aIdx.GetIndex() )
        return sal_False;
    if( IsTxtNode() )
    {   // Do not merge strings if the result exceeds the allowed string length
        const SwTxtNode* pTxtNd = static_cast<const SwTxtNode*>(this);
        sal_uInt64 nSum = pTxtNd->GetTxt().Len();
        pTxtNd = static_cast<const SwTxtNode*>(pNd);
        nSum += pTxtNd->GetTxt().Len();
        if( nSum > STRING_LEN )
            return sal_False;
    }
    if( pIdx )
        *pIdx = aIdx;
    return sal_True;
}


    // koennen 2 Nodes zusammengefasst werden ?
    // in pIdx kann die 2. Position returnt werden.
int SwCntntNode::CanJoinPrev( SwNodeIndex* pIdx ) const
{
    sal_uInt8 nNdType = GetNodeType();
    SwNodeIndex aIdx( *this, -1 );

    const SwNode* pNd = this;
    while( aIdx.GetIndex() &&
        (( pNd = &aIdx.GetNode())->IsSectionNode() ||
            ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsSectionNode() )))
        aIdx--;

    if( pNd->GetNodeType() != nNdType || 0 == aIdx.GetIndex() )
        return sal_False;
    if( pIdx )
        *pIdx = aIdx;
    return sal_True;
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

        if( GetpSwAttrSet() )
        {
            AttrSetHandleHelper::SetParent( mpAttrSet, *this, &GetAnyFmtColl(), GetFmtColl() );
        }

        if( !IsModifyLocked() )
        {
            SwFmtChg aTmp1( pOldColl ? pOldColl : GetFmtColl() );
            SwFmtChg aTmp2( pColl ? pColl : GetFmtColl() );
            NotifyClients( &aTmp1, &aTmp2 );
        }
        if( IsInCache() )
        {
            SwFrm::GetCache().Delete( this );
            SetInCache( sal_False );
        }
    }
}


sal_Bool SwCntntNode::IsAnyCondition( SwCollCondition& rTmp ) const
{
    const SwNodes& rNds = GetNodes();
    {
        int nCond = 0;
        const SwStartNode* pSttNd = StartOfSectionNode();
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
                            GetTblBox( pSttNd->GetIndex() ) ) && pBox &&
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

                        for( sal_uInt16 n = 0; n < rFtnArr.size(); ++n )
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
                case SwNormalStartNode:     break;
                }
            }

            if( nCond )
            {
                rTmp.SetCondition( (Master_CollConditions)nCond, 0 );
                return sal_True;
            }
            pSttNd = pSttNd->GetIndex()
                        ? pSttNd->StartOfSectionNode()
                        : 0;
        }
    }

    {
        sal_uInt16 nPos;
        const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
        if( !rOutlNds.empty() )
        {
            if( !rOutlNds.Seek_Entry( (SwCntntNode*)this, &nPos ) && nPos )
                --nPos;
            if( nPos < rOutlNds.size() &&
                rOutlNds[ nPos ]->GetIndex() < GetIndex() )
            {
                SwTxtNode* pOutlNd = rOutlNds[ nPos ]->GetTxtNode();

                if( pOutlNd->IsOutline())
                {
                    rTmp.SetCondition( PARA_IN_OUTLINE, pOutlNd->GetAttrOutlineLevel() - 1 );
                    return sal_True;
                }
            }
        }
    }

    return sal_False;
}


void SwCntntNode::ChkCondColl()
{
    // zur Sicherheit abfragen
    if( RES_CONDTXTFMTCOLL == GetFmtColl()->Which() )
    {
        SwCollCondition aTmp( 0, 0, 0 );
        const SwCollCondition* pCColl;

        bool bDone = false;

        if( IsAnyCondition( aTmp ))
        {
            pCColl = static_cast<SwConditionTxtFmtColl*>(GetFmtColl())
                ->HasCondition( aTmp );

            if (pCColl)
            {
                SetCondFmtColl( pCColl->GetTxtFmtColl() );
                bDone = true;
            }
        }

        if (!bDone)
        {
            if( IsTxtNode() && ((SwTxtNode*)this)->GetNumRule())
            {
                // steht in einer Numerierung
                // welcher Level?
                aTmp.SetCondition( PARA_IN_LIST,
                                ((SwTxtNode*)this)->GetActualListLevel() );
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

// #i42921#
short SwCntntNode::GetTextDirection( const SwPosition& rPos,
                                     const Point* pPt ) const
{
    short nRet = -1;

    Point aPt;
    if( pPt )
        aPt = *pPt;

    // #i72024# - No format of the frame, because this can cause recursive layout actions
    SwFrm* pFrm = getLayoutFrm( GetDoc()->GetCurrentLayout(), &aPt, &rPos, sal_False );

    if ( pFrm )
    {
        if ( pFrm->IsVertical() )
        {
            if ( pFrm->IsRightToLeft() )
                nRet = FRMDIR_VERT_TOP_LEFT;
            else
                nRet = FRMDIR_VERT_TOP_RIGHT;
        }
        else
        {
            if ( pFrm->IsRightToLeft() )
                nRet = FRMDIR_HORI_RIGHT_TOP;
            else
                nRet = FRMDIR_HORI_LEFT_TOP;
        }
    }


    return nRet;
}

SwOLENodes* SwCntntNode::CreateOLENodesArray( const SwFmtColl& rColl, bool bOnlyWithInvalidSize )
{
    SwOLENodes *pNodes = 0;
    SwIterator<SwCntntNode,SwFmtColl> aIter( rColl );
    for( SwCntntNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
    {
        SwOLENode *pONd = pNd->GetOLENode();
        if ( pONd && (!bOnlyWithInvalidSize || pONd->IsOLESizeInvalid()) )
        {
            if ( !pNodes  )
                pNodes = new SwOLENodes;
            pNodes->push_back( pONd );
        }
    }

    return pNodes;
}

/*
 * Document Interface Access
 */
const IDocumentSettingAccess* SwNode::getIDocumentSettingAccess() const { return GetDoc(); }
const IDocumentDeviceAccess* SwNode::getIDocumentDeviceAccess() const { return GetDoc(); }
const IDocumentMarkAccess* SwNode::getIDocumentMarkAccess() const { return GetDoc()->getIDocumentMarkAccess(); }
const IDocumentRedlineAccess* SwNode::getIDocumentRedlineAccess() const { return GetDoc(); }
const IDocumentStylePoolAccess* SwNode::getIDocumentStylePoolAccess() const { return GetDoc(); }
const IDocumentLineNumberAccess* SwNode::getIDocumentLineNumberAccess() const { return GetDoc(); }
const IDocumentDrawModelAccess* SwNode::getIDocumentDrawModelAccess() const { return GetDoc(); }
const IDocumentLayoutAccess* SwNode::getIDocumentLayoutAccess() const { return GetDoc(); }
IDocumentLayoutAccess* SwNode::getIDocumentLayoutAccess() { return GetDoc(); }
const IDocumentLinksAdministration* SwNode::getIDocumentLinksAdministration() const { return GetDoc(); }
IDocumentLinksAdministration* SwNode::getIDocumentLinksAdministration() { return GetDoc(); }
const IDocumentFieldsAccess* SwNode::getIDocumentFieldsAccess() const { return GetDoc(); }
IDocumentFieldsAccess* SwNode::getIDocumentFieldsAccess() { return GetDoc(); }
IDocumentContentOperations* SwNode::getIDocumentContentOperations() { return GetDoc(); }
IStyleAccess& SwNode::getIDocumentStyleAccess() { return GetDoc()->GetIStyleAccess(); }
// #i83479#
IDocumentListItems& SwNode::getIDocumentListItems()
{
    return *GetDoc();
}

sal_Bool SwNode::IsInRedlines() const
{
    const SwDoc * pDoc = GetDoc();
    sal_Bool bResult = sal_False;

    if (pDoc != NULL)
        bResult = pDoc->IsInRedlines(*this);

    return bResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
