/*************************************************************************
 *
 *  $RCSfile: ndtxt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:27 $
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

#include <hintids.hxx>

#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _TXTATR_HXX //autogen
#include <txtatr.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _FMTRFMRK_HXX //autogen
#include <fmtrfmrk.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _FTNIDX_HXX //autogen
#include <ftnidx.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>                  // fuer SwPosition
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _FTNFRM_HXX
#include <ftnfrm.hxx>
#endif
#ifndef _FTNBOSS_HXX
#include <ftnboss.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>                // fuer SwFmtChg in ChgTxtColl
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>             // fuer SwPageDesc
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>               // fuer SwTblField
#endif
#ifndef _SECTION_HXX
#include <section.hxx>              // fuer SwSection
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _SWCACHE_HXX
#include <swcache.hxx>
#endif
#ifndef _WRONG_HXX
#include <wrong.hxx>                // fuer die WrongList des OnlineSpellings
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif



SV_DECL_PTRARR( TmpHints, SwTxtAttr*, 0, 4 )

TYPEINIT1( SwTxtNode, SwCntntNode )

SV_DECL_PTRARR(SwpHts,SwTxtAttr*,1,1)

// Leider ist das SwpHints nicht ganz wasserdicht:
// Jeder darf an den Hints rumfummeln, ohne die Sortierreihenfolge
// und Verkettung sicherstellen zu muessen.
#ifndef PRODUCT
#define CHECK_SWPHINTS(pNd)  { if(pNd->GetpSwpHints()) \
                                  pNd->GetpSwpHints()->Check(); }
#else
#define CHECK_SWPHINTS(pNd)
#endif

SwTxtNode *SwNodes::MakeTxtNode( const SwNodeIndex & rWhere,
                                 SwTxtFmtColl *pColl,
                                 SwAttrSet* pAutoAttr )
{
    ASSERT( pColl, "Collectionpointer ist 0." );

    SwTxtNode *pNode = new SwTxtNode( rWhere, pColl, pAutoAttr );

    SwNodeIndex aIdx( *pNode );

    if( pColl && NO_NUMBERING != pColl->GetOutlineLevel() && IsDocNodes() )
        UpdateOutlineNode( *pNode, NO_NUMBERING, pColl->GetOutlineLevel() );

    //Wenn es noch kein Layout gibt oder in einer versteckten Section
    // stehen, brauchen wir uns um das MakeFrms nicht bemuehen.
    const SwSectionNode* pSectNd;
    if( !GetDoc()->GetRootFrm() ||
        ( 0 != (pSectNd = pNode->FindSectionNode()) &&
            pSectNd->GetSection().IsHiddenFlag() ))
        return pNode;

    SwNodeIndex aTmp( rWhere );
    do {
        // max. 2 Durchlaeufe:
        // 1. den Nachfolger nehmen
        // 2. den Vorgaenger

        SwNode *pNd;
        switch( ( pNd = (*this)[aTmp] )->GetNodeType() )
        {
        case ND_TABLENODE:
            ((SwTableNode*)pNd)->MakeFrms( aIdx );
            return pNode;

        case ND_SECTIONNODE:
            if( ((SwSectionNode*)pNd)->GetSection().IsHidden() ||
                ((SwSectionNode*)pNd)->IsCntntHidden() )
            {
                SwNodeIndex aTmpIdx( *pNode );
                pNd = FindPrvNxtFrmNode( aTmpIdx, pNode );
                if( !pNd )
                    return pNode;
                aTmp = *pNd;
                break;
            }
            ((SwSectionNode*)pNd)->MakeFrms( aIdx );
            return pNode;

        case ND_TEXTNODE:
        case ND_GRFNODE:
        case ND_OLENODE:
            ((SwCntntNode*)pNd)->MakeFrms( *pNode );
            return pNode;

        case ND_ENDNODE:
            if( pNd->FindStartNode()->IsSectionNode() &&
                aTmp.GetIndex() < rWhere.GetIndex() )
            {
                if( pNd->FindStartNode()->GetSectionNode()->GetSection().IsHiddenFlag())
                {
                    if( !GoPrevSection( &aTmp, TRUE, FALSE ) ||
                        aTmp.GetNode().FindTableNode() !=
                            pNode->FindTableNode() )
                        return pNode;       // schade, das wars
                }
                else
                    aTmp = *pNd->FindStartNode();
                break;
            }
            else if( pNd->FindStartNode()->IsTableNode() &&
                    aTmp.GetIndex() < rWhere.GetIndex() )
            {
                // wir stehen hinter einem TabellenNode
                aTmp = *pNd->FindStartNode();
                break;
            }
            // kein break !!!
        default:
            if( rWhere == aTmp )
                aTmp -= 2;
            else
                return pNode;
            break;
        }
    } while( TRUE );
}



// --------------------
// SwTxtNode
// --------------------

SwTxtNode::SwTxtNode( const SwNodeIndex &rWhere,
                      SwTxtFmtColl *pTxtColl,
                      SwAttrSet* pAutoAttr )
    : SwCntntNode( rWhere, ND_TEXTNODE, pTxtColl ),
      pSwpHints( 0 ), pWrong( 0 ), pNdNum( 0 ), pNdOutl( 0 )
{
    // soll eine Harte-Attributierung gesetzt werden?
    if( pAutoAttr )
        SwCntntNode::SetAttr( *pAutoAttr );

    const SfxPoolItem* pItem;
    if( GetNodes().IsDocNodes() &&
        SFX_ITEM_SET == GetSwAttrSet().GetItemState( RES_PARATR_NUMRULE,
        TRUE, &pItem ) && ((SwNumRuleItem*)pItem)->GetValue().Len() )
    {
        pNdNum = new SwNodeNum( 0 );
        SwNumRule* pRule = GetDoc()->FindNumRulePtr(
                                    ((SwNumRuleItem*)pItem)->GetValue() );
        if( pRule )
            pRule->SetInvalidRule( TRUE );
    }
}

SwTxtNode::~SwTxtNode()
{
    // delete loescht nur die Pointer, nicht die Arrayelemente!
    if( pSwpHints )
    {
        // damit Attribute die ihren Inhalt entfernen nicht doppelt
        // geloescht werden.
        SwpHints* pTmpHints = pSwpHints;
        pSwpHints = 0;

        for( register USHORT j = pTmpHints->Count(); j; )
            // erst muss das Attribut aus dem Array entfernt werden,
            // denn sonst wuerde es sich selbst loeschen (Felder) !!!!
            DestroyAttr( pTmpHints->GetHt( --j ) );

        delete pTmpHints;
    }
    delete pWrong;
    // Achtung. im Dtor von SwCntntNode kann DelFrms gerufen werden, wo
    // ggf. pWrong nochmal deletet wird, deshalb diese Zuweisung
    pWrong = NULL; // hier nicht wegoptimieren!

    delete pNdNum, pNdNum = 0;      // ggfs. wird in der BasisKlasse noch
    delete pNdOutl, pNdOutl = 0;    // darauf zugegriffen??
}

SwCntntFrm *SwTxtNode::MakeFrm()
{
    SwCntntFrm *pFrm = new SwTxtFrm(this);
    return pFrm;
}

xub_StrLen SwTxtNode::Len() const
{
    return aText.Len();
}

/*---------------------------------------------------------------------------
 * lcl_ChangeFtnRef
 *  After a split node, it's necessary to actualize the ref-pointer of the
 *  ftnfrms.
 * --------------------------------------------------------------------------*/

void lcl_ChangeFtnRef( SwTxtNode &rNode )
{
    SwpHints *pSwpHints = rNode.GetpSwpHints();
    if( pSwpHints && rNode.GetDoc()->GetRootFrm() )
    {
        SwTxtAttr* pHt;
        SwCntntFrm* pFrm = NULL;
        for( register USHORT j = pSwpHints->Count(); j; )
            if( RES_TXTATR_FTN == (pHt = pSwpHints->GetHt(--j))->Which() )
            {
                if( !pFrm )
                {
                    SwClientIter aNew( rNode );
                    pFrm = (SwCntntFrm*)aNew.First( TYPE(SwCntntFrm) );
//JP 11.07.00: the assert's shows incorrect an error when nodes are converted
//              to a table. Then no layout exist!
//                  ASSERT( pFrm, "lcl_ChangeFtnRef: No TxtFrm" );
//                  ASSERT( pFrm && !aNew.Next(),"lcl_ChangeFtnRef: Doublefault");
                    if( !pFrm )
                        return;
                }
                SwTxtFtn *pAttr = (SwTxtFtn*)pHt;
                ASSERT( pAttr->GetStartNode(), "FtnAtr ohne StartNode." );
                SwNodeIndex aIdx( *pAttr->GetStartNode(), 1 );
                SwCntntNode *pNd = aIdx.GetNode().GetCntntNode();
                if ( !pNd )
                    pNd = pFrm->GetAttrSet()->GetDoc()->
                            GetNodes().GoNextSection( &aIdx, TRUE, FALSE );
                if ( !pNd )
                    continue;
                SwClientIter aIter( *pNd );
                SwCntntFrm* pCntnt = (SwCntntFrm*)aIter.First(TYPE(SwCntntFrm));
                if( pCntnt )
                {
                    ASSERT( pCntnt->FindRootFrm() == pFrm->FindRootFrm(),
                            "lcl_ChangeFtnRef: Layout double?" );
                    SwFtnFrm *pFtn = pCntnt->FindFtnFrm();
                    if( pFtn && pFtn->GetAttr() == pAttr )
                    {
                        while( pFtn->GetMaster() )
                            pFtn = pFtn->GetMaster();
                        while ( pFtn )
                        {
                            pFtn->SetRef( pFrm );
                            pFtn = pFtn->GetFollow();
                            ((SwTxtFrm*)pFrm)->SetFtn( TRUE );
                        }
                    }
#ifndef PRODUCT
                    while( 0 != (pCntnt = (SwCntntFrm*)aIter.Next()) )
                    {
                        SwFtnFrm *pFtn = pCntnt->FindFtnFrm();
                        ASSERT( !pFtn || pFtn->GetRef() == pFrm,
                                "lcl_ChangeFtnRef: Who's that guy?" );
                    }
#endif
                }
            }
    }
}

SwCntntNode *SwTxtNode::SplitNode( const SwPosition &rPos )
{
    // lege den Node "vor" mir an
    register xub_StrLen nSplitPos = rPos.nContent.GetIndex(),
                    nTxtLen = aText.Len();
    SwTxtNode* pNode = _MakeNewTxtNode( rPos.nNode, FALSE, nSplitPos==nTxtLen );

    if( GetDepends() && aText.Len() && (nTxtLen / 2) < nSplitPos )
    {
// JP 25.04.95: Optimierung fuer SplitNode:
//              Wird am Ende vom Node gesplittet, dann verschiebe die
//              Frames vom akt. auf den neuen und erzeuge fuer den akt.
//              neue. Dadurch entfaellt das neu aufbauen vom Layout.

        LockModify();   // Benachrichtigungen abschalten

        // werden FlyFrames mit verschoben, so muessen diese nicht ihre
        // Frames zerstoeren. Im SwTxtFly::SetAnchor wird es abgefragt!
        if( pSwpHints )
        {
            if( !pNode->pSwpHints )
                pNode->pSwpHints = new SwpHints;
            pNode->pSwpHints->bInSplitNode = TRUE;
        }

        //Ersten Teil des Inhalts in den neuen Node uebertragen und
        //im alten Node loeschen.
        SwIndex aIdx( this );
        Cut( pNode, aIdx, nSplitPos );

        if( GetWrong() )
            GetWrong()->Move( 0, -nSplitPos );
        SetWrongDirty( TRUE );

        if( pNode->pSwpHints )
        {
            if ( pNode->pSwpHints->CanBeDeleted() )
            {
                delete pNode->pSwpHints;
                pNode->pSwpHints = 0;
            }
            else
                pNode->pSwpHints->bInSplitNode = FALSE;

            // alle zeichengebundenen Rahmen, die im neuen Absatz laden
            // muessen aus den alten Frame entfernt werden:
            // JP 01.10.96: alle leeren und nicht zu expandierenden
            //              Attribute loeschen
            if( pSwpHints )
            {
                SwTxtAttr* pHt;
                xub_StrLen* pEnd;
                for( register USHORT j = pSwpHints->Count(); j; )
                    if( RES_TXTATR_FLYCNT == ( pHt = pSwpHints->GetHt( --j ) )->Which()
                        && RES_DRAWFRMFMT != pHt->GetFlyCnt().GetFrmFmt()->Which() )
                        pHt->GetFlyCnt().GetFrmFmt()->DelFrms();
                    else if( pHt->DontExpand() && 0 != ( pEnd = pHt->GetEnd() )
                            && *pHt->GetStart() == *pEnd )
                    {
                        // loeschen!
                        pSwpHints->DeleteAtPos( j );
                        DestroyAttr( pHt );
                    }
            }

        }

        SwClientIter aIter( *this );
        SwClient* pLast = aIter.GoStart();
        if( pLast )
            do
            {   SwCntntFrm *pFrm = PTR_CAST( SwCntntFrm, pLast );
                if ( pFrm )
                {
                    pNode->Add( pFrm );
                    if( pFrm->IsTxtFrm() && !pFrm->IsFollow() &&
                        ((SwTxtFrm*)pFrm)->GetOfst() )
                        ((SwTxtFrm*)pFrm)->SetOfst( 0 );
                }
            } while( 0 != ( pLast = aIter++ ));

        if ( IsInCache() )
        {
            SwFrm::GetCache().Delete( this );
            SetInCache( FALSE );
        }

        UnlockModify(); // Benachrichtigungen wieder freischalten

        if( nTxtLen != nSplitPos )
        {
            // dann sage den Frames noch, das am Ende etwas "geloescht" wurde
            if( 1 == nTxtLen - nSplitPos )
            {
                SwDelChr aHint( nSplitPos );
                pNode->SwModify::Modify( 0, &aHint );
            }
            else
            {
                SwDelTxt aHint( nSplitPos, nTxtLen - nSplitPos );
                pNode->SwModify::Modify( 0, &aHint );
            }
        }
        if( pSwpHints )
            MoveTxtAttr_To_AttrSet();
        pNode->MakeFrms( *this );       // neue Frames anlegen.
        lcl_ChangeFtnRef( *this );
    }
    else
    {
        //Ersten Teil des Inhalts in den neuen Node uebertragen und
        //im alten Node loeschen.
        SwIndex aIdx( this );
        Cut( pNode, aIdx, rPos.nContent.GetIndex() );

        // JP 01.10.96: alle leeren und nicht zu expandierenden
        //              Attribute loeschen
        if( pSwpHints )
        {
            SwTxtAttr* pHt;
            xub_StrLen* pEnd;
            for( register USHORT j = pSwpHints->Count(); j; )
                if( ( pHt = pSwpHints->GetHt( --j ) )->DontExpand() &&
                    0 != ( pEnd = pHt->GetEnd() ) && *pHt->GetStart() == *pEnd )
                {
                    // loeschen!
                    pSwpHints->DeleteAtPos( j );
                    DestroyAttr( pHt );
                }
            MoveTxtAttr_To_AttrSet();
        }

        if ( GetDepends() )
            MakeFrms( *pNode );     // neue Frames anlegen.
        lcl_ChangeFtnRef( *pNode );
    }

    {
        //Hint fuer Pagedesc versenden. Das mueste eigntlich das Layout im
        //Paste der Frames selbst erledigen, aber das fuehrt dann wiederum
        //zu weiteren Folgefehlern, die mit Laufzeitkosten geloest werden
        //muesten. #56977# #55001# #56135#
        const SfxPoolItem *pItem;
        if( GetDepends() && SFX_ITEM_SET == pNode->GetSwAttrSet().
            GetItemState( RES_PAGEDESC, TRUE, &pItem ) )
            pNode->Modify( (SfxPoolItem*)pItem, (SfxPoolItem*)pItem );
    }
    return pNode;
}

void SwTxtNode::MoveTxtAttr_To_AttrSet()
{
    ASSERT( pSwpHints, "MoveTxtAttr_To_AttrSet without SwpHints?" );
    for( USHORT i = 0; pSwpHints && i < pSwpHints->Count(); ++i )
    {
        SwTxtAttr *pHt = pSwpHints->GetHt(i);

        if( *pHt->GetStart() )
            break;

        const xub_StrLen* pHtEndIdx = pHt->GetEnd();

        if( !pHtEndIdx )
            continue;

        const USHORT nWhich = pHt->Which();

        if( *pHtEndIdx < aText.Len() || nWhich == RES_TXTATR_CHARFMT
                                     || nWhich == RES_TXTATR_INETFMT )
            break;

        if( nWhich == RES_TXTATR_TOXMARK || nWhich == RES_TXTATR_REFMARK )
            continue;

        if( SwCntntNode::SetAttr( pHt->GetAttr() ) )
        {
            pSwpHints->DeleteAtPos(i);
            DestroyAttr( pHt );
            --i;
        }
    }

}

SwCntntNode *SwTxtNode::JoinNext()
{
    SwNodes& rNds = GetNodes();
    SwNodeIndex aIdx( *this );
    if( SwCntntNode::CanJoinNext( &aIdx ) )
    {
        SwDoc* pDoc = rNds.GetDoc();
        SvULongs aBkmkArr( 15, 15 );
        _SaveCntntIdx( pDoc, aIdx.GetIndex(), USHRT_MAX, aBkmkArr, SAVEFLY );
        SwTxtNode *pTxtNode = aIdx.GetNode().GetTxtNode();
        xub_StrLen nOldLen = aText.Len();
        { // wg. SwIndex
            pTxtNode->Cut( this, SwIndex(pTxtNode), pTxtNode->Len() );
        }
        // verschiebe noch alle Bookmarks/TOXMarks
        if( aBkmkArr.Count() )
            _RestoreCntntIdx( pDoc, aBkmkArr, GetIndex(), nOldLen );

        if( pTxtNode->HasAnyIndex() )
        {
            // alle Crsr/StkCrsr/UnoCrsr aus dem Loeschbereich verschieben
            pDoc->CorrAbs( aIdx, SwPosition( *this ), nOldLen, TRUE );
        }
        rNds.Delete(aIdx);
        InvalidateNumRule();
    }
    else
        ASSERT( FALSE, "kein TxtNode." );

    return this;
}

SwCntntNode *SwTxtNode::JoinPrev()
{
    SwNodes& rNds = GetNodes();
    SwNodeIndex aIdx( *this );
    if( SwCntntNode::CanJoinPrev( &aIdx ) )
    {
        SwDoc* pDoc = rNds.GetDoc();
        SvULongs aBkmkArr( 15, 15 );
        _SaveCntntIdx( pDoc, aIdx.GetIndex(), USHRT_MAX, aBkmkArr, SAVEFLY );
        SwTxtNode *pTxtNode = aIdx.GetNode().GetTxtNode();
        xub_StrLen nLen = pTxtNode->Len();
        { // wg. SwIndex
            pTxtNode->Cut( this, SwIndex( this ), SwIndex(pTxtNode), nLen );
        }
        // verschiebe noch alle Bookmarks/TOXMarks
        if( aBkmkArr.Count() )
            _RestoreCntntIdx( pDoc, aBkmkArr, GetIndex() );

        if( pTxtNode->HasAnyIndex() )
        {
            // alle Crsr/StkCrsr/UnoCrsr aus dem Loeschbereich verschieben
            pDoc->CorrAbs( aIdx, SwPosition( *this ), nLen, TRUE );
        }
        rNds.Delete(aIdx);
        InvalidateNumRule();
    }
    else
        ASSERT( FALSE, "kein TxtNode." );

    return this;
}

// erzeugt einen AttrSet mit Bereichen fuer Frame-/Para/Char-Attributen
void SwTxtNode::NewAttrSet( SwAttrPool& rPool )
{
    ASSERT( !pAttrSet, "AttrSet ist doch gesetzt" );
    pAttrSet = new SwAttrSet( rPool, aTxtNodeSetRange );
//FEATURE::CONDCOLL
//  pAttrSet->SetParent( &GetFmtColl()->GetAttrSet() );
    pAttrSet->SetParent( &GetAnyFmtColl().GetAttrSet() );
//FEATURE::CONDCOLL
}


// change the URL in the attribut - if it is a valid URL!
void lcl_CheckURLChanged( const SwFmtINetFmt& rURLAttr, const String& rText,
                            xub_StrLen nStt, xub_StrLen nEnd )
{
    if( nStt < nEnd )
    {
        xub_StrLen nS = nStt, nE = nEnd;
        String sNew( URIHelper::FindFirstURLInText( rText, nS, nE,
                                                    GetAppCharClass() ));
        if( sNew.Len() && nS == nStt && nE == nEnd )
        {
            // it is an valid URL, so set it to the URL Object
            ((SwFmtINetFmt&)rURLAttr).SetValue( rText.Copy( nS, nE - nS ));
        }
    }
}

// Ueberladen der virtuellen Update-Methode von SwIndexReg. Dadurch
// benoetigen die Text-Attribute nur xub_StrLen statt SwIndizies!
void SwTxtNode::Update( const SwIndex & aPos, xub_StrLen nLen,
                        BOOL bNegativ )
{
    SetAutoCompleteWordDirty( TRUE );

    TmpHints* pCollector = NULL;
    if( pSwpHints )
    {
        xub_StrLen nPos = aPos.GetIndex();
        xub_StrLen* pIdx;
        SwTxtAttr* pHt;
        if( bNegativ )
        {
            xub_StrLen nMax = nPos + nLen;
            for( USHORT n = 0; n < pSwpHints->Count(); ++n )
            {
                BOOL bCheckURL = FALSE, bSttBefore = FALSE;
                pHt = pSwpHints->GetHt(n);
                pIdx = pHt->GetStart();
                if( *pIdx >= nPos )
                {
                    if( *pIdx > nMax )
                         *pIdx -= nLen;
                    else
                    {
                        if( *pIdx < nMax )
                             bCheckURL = TRUE;
                         *pIdx = nPos;
                    }
                }
                else
                    bSttBefore = TRUE;

                if( 0 == (pIdx = pHt->GetEnd()) )
                    continue;

                if( *pIdx >= nPos )
                {
                    if( *pIdx > nMax )
                         *pIdx -= nLen;
                    else if( *pIdx != nPos )
                    {
                        *pIdx = nPos;
                        if( bSttBefore )
                            bCheckURL = TRUE;
                    }
                }

                if( bCheckURL && RES_TXTATR_INETFMT == pHt->Which() )
                {
                    // reset the URL in the attribut - if it is a valid URL!
                    lcl_CheckURLChanged( pHt->GetINetFmt(), aText,
                                        *pHt->GetStart(), *pHt->GetEnd() );
                }

//JP 01.10.96: fuers SplitNode sollte das Flag nicht geloescht werden!
//              pHt->SetDontExpand( FALSE );
            }
            // AMA: Durch das Loeschen koennen Attribute gleiche Start-
            //      und/oder Endwerte erhalten, die vorher echt ungleich
            //      waren. Dadurch kann die Sortierung durcheinander geraten,
            //      die bei gleichen Start/Endwerten den Pointer selbst
            //      vergleicht, also ClearDummies ...
            pSwpHints->ClearDummies( *this );
            if ( !pSwpHints->Merge( *this ) )
                ((SwpHintsArr*)pSwpHints)->Resort();
        }
        else
        {
            xub_StrLen* pEnd;
            BOOL bNoExp = FALSE;
            const USHORT coArrSz = RES_TXTATR_WITHEND_END - RES_CHRATR_BEGIN +
                                ( RES_UNKNOWNATR_END - RES_UNKNOWNATR_BEGIN );

            BOOL aDontExp[ coArrSz ];
            memset( &aDontExp, 0, coArrSz * sizeof(BOOL) );

            for( USHORT n = 0; n < pSwpHints->Count(); ++n )
            {
                BOOL bCheckURL = FALSE;
                pHt = pSwpHints->GetHt(n);
                pIdx = pHt->GetStart();
                if( *pIdx >= nPos )
                {
                    *pIdx += nLen;
                    if( 0 != ( pEnd = pHt->GetEnd() ) )
                        *pEnd += nLen;
                }
                else if( 0 != ( pEnd = pHt->GetEnd() ) && *pEnd >= nPos )
                {
                    if( *pEnd > nPos )
                    {
                        bCheckURL = TRUE;
                        *pEnd += nLen;
                    }
                    else
                    {
                        USHORT nWhPos, nWhich = pHt->Which();

                        if( RES_CHRATR_BEGIN <= nWhich &&
                            nWhich < RES_TXTATR_WITHEND_END )
                             nWhPos = nWhich - RES_CHRATR_BEGIN;
                        else if( RES_UNKNOWNATR_BEGIN <= nWhich &&
                                nWhich < RES_UNKNOWNATR_END )
                            nWhPos = nWhich - RES_UNKNOWNATR_BEGIN +
                                ( RES_TXTATR_WITHEND_END - RES_CHRATR_BEGIN );
                        else
                            continue;

                        if( aDontExp[ nWhPos ] )
                            continue;
                        BOOL bCharFmt = ( nWhich == RES_TXTATR_CHARFMT ||
                                          nWhich == RES_TXTATR_INETFMT );
                        if( pHt->DontExpand() )
                        {
                            pHt->SetDontExpand( FALSE );
                            if( bCharFmt )
                            {
                                bNoExp = TRUE;
                                aDontExp[ RES_TXTATR_CHARFMT -RES_CHRATR_BEGIN ]
                                    = TRUE;
                                aDontExp[ RES_TXTATR_INETFMT -RES_CHRATR_BEGIN ]
                                    = TRUE;
                            }
                            else
                                aDontExp[ nWhPos ] = TRUE;
                        }
                        else if( bNoExp )
                        {
                             if( !pCollector )
                                pCollector = new TmpHints;
                             USHORT nCollCnt = pCollector->Count();
                             for( USHORT i = 0; i < nCollCnt; ++i )
                             {
                                SwTxtAttr *pTmp = (*pCollector)[ i ];
                                if( nWhich == pTmp->Which() )
                                {
                                    pCollector->Remove( i );
                                    delete pTmp;
                                    break;
                                }
                             }
                             SwTxtAttr *pTmp = MakeTxtAttr( pHt->GetAttr(),
                                                nPos, nPos + nLen );
                             pCollector->C40_INSERT( SwTxtAttr, pTmp, pCollector->Count() );
                        }
                        else
                        {
                            *pEnd += nLen;
                            bCheckURL = TRUE;
                        }
                    }
                }

                if( bCheckURL && RES_TXTATR_INETFMT == pHt->Which() )
                {
                    // reset the URL in the attribut - if it is a valid URL!
                    lcl_CheckURLChanged( pHt->GetINetFmt(), aText,
                                        *pHt->GetStart(), *pHt->GetEnd() );
                }
            }
        }
    }

    SwIndexReg aTmpIdxReg;
    if( !bNegativ )
    {
        SwIndex* pIdx;
        const SwRedlineTbl& rTbl = GetDoc()->GetRedlineTbl();
        if( rTbl.Count() )
            for( USHORT i = 0; i < rTbl.Count(); ++i )
            {
                SwRedline* pRedl = rTbl[ i ];
                if( pRedl->HasMark() )
                {
                    if(( this == &pRedl->GetBound(TRUE).nNode.GetNode() ||
                         this == &pRedl->GetBound(FALSE).nNode.GetNode() ) &&
                        *pRedl->GetPoint() != *pRedl->GetMark() &&
                        aPos.GetIndex() == (pIdx = &pRedl->End()->
                            nContent)->GetIndex() )
                        pIdx->Assign( &aTmpIdxReg, pIdx->GetIndex() );
                }
                else if( this == &pRedl->GetPoint()->nNode.GetNode() &&
                        aPos.GetIndex() == (pIdx = &pRedl->GetPoint()->
                            nContent)->GetIndex() )
                {
                    pIdx->Assign( &aTmpIdxReg, pIdx->GetIndex() );
                    if( &pRedl->GetBound( TRUE ) == pRedl->GetPoint() )
                    {
                        pRedl->GetBound( FALSE ) = pRedl->GetBound( TRUE );
                        pIdx = &pRedl->GetBound( FALSE ).nContent;
                    }
                    else
                    {
                        pRedl->GetBound( TRUE ) = pRedl->GetBound( FALSE );
                        pIdx = &pRedl->GetBound( TRUE ).nContent;
                    }
                    pIdx->Assign( &aTmpIdxReg, pIdx->GetIndex() );
                }
            }

        const SwBookmarks& rBkmk = GetDoc()->GetBookmarks();
        if( rBkmk.Count() )
            for( USHORT i = 0; i < rBkmk.Count(); ++i )
            {
                SwBookmark* pBkmk = rBkmk[ i ];
                if( (this == &pBkmk->GetPos().nNode.GetNode() &&
                     aPos.GetIndex() == (pIdx = (SwIndex*)&pBkmk->GetPos().
                            nContent)->GetIndex() ) ||
                    ( pBkmk->GetOtherPos() &&
                      this == &pBkmk->GetOtherPos()->nNode.GetNode() &&
                      aPos.GetIndex() == (pIdx = (SwIndex*)&pBkmk->
                              GetOtherPos()->nContent)->GetIndex() ) )
                        pIdx->Assign( &aTmpIdxReg, pIdx->GetIndex() );
            }
    }
    SwIndexReg::Update( aPos, nLen, bNegativ );
    if( pCollector )
    {
        USHORT nCount = pCollector->Count();
        for( USHORT i = 0; i < nCount; ++i )
            pSwpHints->Insert( (*pCollector)[ i ], *this, FALSE );
        delete pCollector;
    }

    aTmpIdxReg.MoveTo( *this );
}

SwFmtColl* SwTxtNode::ChgFmtColl( SwFmtColl *pNewColl )
{
    ASSERT( pNewColl,"ChgFmtColl: Collectionpointer ist 0." );
    ASSERT( HAS_BASE( SwTxtFmtColl, pNewColl ),
                "ChgFmtColl: ist kein Text-Collectionpointer." );

    SwTxtFmtColl *pOldColl = GetTxtColl();
    if( pNewColl != pOldColl )
        SwCntntNode::ChgFmtColl( pNewColl );
    // nur wenn im normalen Nodes-Array
    if( GetNodes().IsDocNodes() )
        _ChgTxtCollUpdateNum( pOldColl, (SwTxtFmtColl*)pNewColl );
    return  pOldColl;
}

void SwTxtNode::_ChgTxtCollUpdateNum( const SwTxtFmtColl *pOldColl,
                                        const SwTxtFmtColl *pNewColl)
{
    SwDoc* pDoc = GetDoc();
    ASSERT( pDoc, "Kein Doc?" );
    // erfrage die OutlineLevel und update gegebenenfalls das Nodes-Array,
    // falls sich die Level geaendert haben !
    const BYTE nOldLevel = pOldColl ? pOldColl->GetOutlineLevel():NO_NUMBERING;
    const BYTE nNewLevel = pNewColl ? pNewColl->GetOutlineLevel():NO_NUMBERING;

    SwNodes& rNds = GetNodes();
    if( nOldLevel != nNewLevel )
    {
        delete pNdOutl, pNdOutl = 0;
        // Numerierung aufheben, falls sie aus der Vorlage kommt
        // und nicht nicht aus der neuen
        if( NO_NUMBERING != nNewLevel && pNdNum && ( !GetpSwAttrSet() ||
            SFX_ITEM_SET != GetpSwAttrSet()->GetItemState(
                RES_PARATR_NUMRULE, FALSE )) &&
            (!pNewColl || SFX_ITEM_SET != pNewColl->GetItemState(
                RES_PARATR_NUMRULE )) )
            delete pNdNum, pNdNum = 0;
        if( rNds.IsDocNodes() )
            rNds.UpdateOutlineNode( *this, nOldLevel, nNewLevel );
    }

    // Update beim Level 0 noch die Fussnoten !!
    if( (!nNewLevel || !nOldLevel) && pDoc->GetFtnIdxs().Count() &&
        FTNNUM_CHAPTER == pDoc->GetFtnInfo().eNum &&
        rNds.IsDocNodes() )
    {
        SwNodeIndex aTmpIndex( rNds, GetIndex());

        pDoc->GetFtnIdxs().UpdateFtn( aTmpIndex);
    }

//FEATURE::CONDCOLL
    if( /*pOldColl != pNewColl && pNewColl && */
        RES_CONDTXTFMTCOLL == pNewColl->Which() )
    {
        // Erfrage die akt. Condition des TextNodes:
        ChkCondColl();
    }
//FEATURE::CONDCOLL
}

// Wennn man sich genau am Ende einer Text- bzw. INetvorlage befindet,
// bekommt diese das DontExpand-Flag verpasst

BOOL SwTxtNode::DontExpandFmt( const SwIndex& rIdx, BOOL bFlag,
                                BOOL bFmtToTxtAttributes )
{
    const xub_StrLen nIdx = rIdx.GetIndex();
    if( bFmtToTxtAttributes && nIdx == aText.Len() )
        FmtToTxtAttr( this );
    if( !pSwpHints )
        return FALSE;
    const USHORT nEndCnt = pSwpHints->GetEndCount();
    USHORT nPos = nEndCnt;
    BOOL bRet = FALSE;
    while( nPos )
    {
        SwTxtAttr *pTmp = pSwpHints->GetEnd( --nPos );
        xub_StrLen *pEnd = pTmp->GetEnd();
        if( !pEnd || *pEnd > nIdx )
            continue;
        if( nIdx != *pEnd )
            nPos = 0;
        else if( bFlag != pTmp->DontExpand() && *pEnd > *pTmp->GetStart() )
        {
            bRet = TRUE;
            pSwpHints->NoteInHistory( pTmp );
            pTmp->SetDontExpand( bFlag );
        }
    }
    return bRet;
}


// gebe das vorgegebene Attribut, welches an der TextPosition (rIdx)
// gesetzt ist, zurueck. Gibt es keines, returne 0-Pointer.
// (gesetzt heisst, je nach bExpand ?
//                                    Start < rIdx <= End
//                                  : Start <= rIdx < End )

SwTxtAttr* SwTxtNode::GetTxtAttr( const SwIndex& rIdx, USHORT nWhichHt,
                                  BOOL bExpand ) const
{
    const SwTxtAttr* pRet = 0;
    const SwTxtAttr* pHt = 0;
    const xub_StrLen *pEndIdx = 0;
    const xub_StrLen nIdx = rIdx.GetIndex();
    const USHORT  nSize = pSwpHints ? pSwpHints->Count() : 0;

    for( USHORT i = 0; i < nSize; ++i )
    {
        // ist der Attribut-Anfang schon groesser als der Idx ?
        if( nIdx < *((pHt = (*pSwpHints)[i])->GetStart()) )
            break;          // beenden, kein gueltiges Attribut

        // ist es das gewuenschte Attribut ?
        if( pHt->Which() != nWhichHt )
            continue;       // nein, weiter

        pEndIdx = pHt->GetEnd();
        // liegt innerhalb des Bereiches ??
        if( !pEndIdx )
        {
            if( *pHt->GetStart() == nIdx )
            {
                pRet = pHt;
                break;
            }
        }
        else if( *pHt->GetStart() <= nIdx && nIdx <= *pEndIdx )
        {
            // Wenn bExpand gesetzt ist, wird das Verhalten bei Eingabe
            // simuliert, d.h. der Start wuede verschoben, das Ende expandiert,
            if( bExpand )
            {
                if( *pHt->GetStart() < nIdx )
                    pRet = pHt;
            }
            else
            {
                if( nIdx < *pEndIdx )
                    pRet = pHt;     // den am dichtesten liegenden
            }
        }
    }
    return (SwTxtAttr*)pRet;        // kein gueltiges Attribut gefunden !!
}

/*************************************************************************
 *                          CopyHint()
 *************************************************************************/

SwCharFmt* lcl_FindCharFmt( const SwCharFmts* pCharFmts, const XubString& rName )
{
    if( rName.Len() )
    {
        SwCharFmt* pFmt;
        USHORT nArrLen = pCharFmts->Count();
        for( USHORT i = 1; i < nArrLen; i++ )
        {
            pFmt = (*pCharFmts)[ i ];
            if( pFmt->GetName().CompareTo( rName ) == COMPARE_EQUAL )
                return pFmt;
        }
    }
    return NULL;
}

void lcl_CopyHint( const USHORT nWhich, const SwTxtAttr *pHt,
                    SwTxtAttr *pNewHt, SwDoc* pOtherDoc, SwTxtNode *pDest )
{
    ASSERT( nWhich == pHt->Which(), "Falsche Hint-Id" );
    switch( nWhich )
    {
        // Wenn wir es mit einem Fussnoten-Attribut zu tun haben,
        // muessen wir natuerlich auch den Fussnotenbereich kopieren.
        case RES_TXTATR_FTN :
            ((SwTxtFtn*)pHt)->CopyFtn( (SwTxtFtn*)pNewHt );
            break;

        // Beim Kopieren von Feldern in andere Dokumente
        // muessen die Felder bei ihren neuen Feldtypen angemeldet werden.

        // TabellenFormel muessen relativ kopiert werden.
        case RES_TXTATR_FIELD :
            {
                const SwFmtFld& rFld = pHt->GetFld();
                if( pOtherDoc )
                    ((SwTxtFld*)pHt)->CopyFld( (SwTxtFld*)pNewHt );

                // Tabellenformel ??
                if( RES_TABLEFLD == rFld.GetFld()->GetTyp()->Which()
                    && ((SwTblField*)rFld.GetFld())->IsIntrnlName() )
                {
                    // wandel die interne in eine externe Formel um
                    const SwTableNode* pDstTblNd = ((SwTxtFld*)pHt)->
                                            GetTxtNode().FindTableNode();
                    if( pDstTblNd )
                    {
                        SwTblField* pTblFld = (SwTblField*)
                                                pNewHt->GetFld().GetFld();
                        pTblFld->PtrToBoxNm( &pDstTblNd->GetTable() );
                    }
                }
            }
            break;

        case RES_TXTATR_TOXMARK :
            if( pOtherDoc && pDest && pDest->GetpSwpHints()
                && USHRT_MAX != pDest->GetpSwpHints()->GetPos( pNewHt ) )
                // Beim Kopieren von TOXMarks(Client) in andere Dokumente
                // muss der Verzeichnis (Modify) ausgetauscht werden
                ((SwTxtTOXMark*)pNewHt)->CopyTOXMark( pOtherDoc );
            break;

        case RES_TXTATR_CHARFMT :
            // Wenn wir es mit einer Zeichenvorlage zu tun haben,
            // muessen wir natuerlich auch die Formate kopieren.
            if( pDest && pDest->GetpSwpHints()
                && USHRT_MAX != pDest->GetpSwpHints()->GetPos( pNewHt ) )
            {
                SwCharFmt* pFmt = (SwCharFmt*)pHt->GetCharFmt().GetCharFmt();

                if( pFmt && pOtherDoc )
                    pFmt = pOtherDoc->CopyCharFmt( *pFmt );
                ((SwFmtCharFmt&)pNewHt->GetCharFmt()).SetCharFmt( pFmt );
            }
            break;
        case RES_TXTATR_INETFMT :
            // Wenn wir es mit benutzerdefinierten INet-Zeichenvorlagen
            // zu tun haben, muessen wir natuerlich auch die Formate kopieren.
            if( pOtherDoc && pDest && pDest->GetpSwpHints()
                && USHRT_MAX != pDest->GetpSwpHints()->GetPos( pNewHt ) )
            {
                const SwDoc* pDoc;
                if( 0!=( pDoc = ((SwTxtINetFmt*)pHt)->GetTxtNode().GetDoc() ) )
                {
                    const SwCharFmts* pCharFmts = pDoc->GetCharFmts();
                    const SwFmtINetFmt& rFmt = pHt->GetINetFmt();
                    SwCharFmt* pFmt;
                    pFmt = lcl_FindCharFmt( pCharFmts, rFmt.GetINetFmt() );
                    if( pFmt )
                        pOtherDoc->CopyCharFmt( *pFmt );
                    pFmt = lcl_FindCharFmt( pCharFmts, rFmt.GetVisitedFmt() );
                    if( pFmt )
                        pOtherDoc->CopyCharFmt( *pFmt );
                }
            }
            //JP 24.04.98: Bug 49753 - ein TextNode muss am Attribut
            //              gesetzt sein, damit die Vorlagen erzeugt
            //              werden koenne
            if( !((SwTxtINetFmt*)pNewHt)->GetpTxtNode() )
                ((SwTxtINetFmt*)pNewHt)->ChgTxtNode( pDest );

            //JP 22.10.97: Bug 44875 - Verbindung zum Format herstellen
            ((SwTxtINetFmt*)pNewHt)->GetCharFmt();
            break;
    }
}

/*************************************************************************
|*  SwTxtNode::CopyAttr()
|*  Beschreibung    kopiert Attribute an der Position nStart in pDest.
|*  BP 7.6.93:      Es werden mit Absicht nur die Attribute _mit_ EndIdx
|*                  kopiert! CopyAttr wird vornehmlich dann gerufen,
|*                  wenn Attribute fuer einen Node mit leerem String
|*                  gesetzt werden sollen.
*************************************************************************/

void SwTxtNode::CopyAttr( SwTxtNode *pDest, const xub_StrLen nTxtStartIdx,
                          const xub_StrLen nOldPos )
{
    if( pSwpHints )        // keine Attribute, keine Kekse
    {
        const xub_StrLen *pEndIdx = 0;
        const SwTxtAttr *pHt = 0;
        SwTxtAttr *pNewHt = 0;
        xub_StrLen nAttrStartIdx = 0;
        USHORT nWhich;

        SwDoc* pOtherDoc = pDest->GetDoc();
        if( pOtherDoc == GetDoc() )
            pOtherDoc = 0;

        for( USHORT i = 0; i < pSwpHints->Count(); i++ )
        {
            pHt = (*pSwpHints)[i];
            if( nTxtStartIdx < ( nAttrStartIdx = *pHt->GetStart() ) )
                break;      // ueber das Textende, da nLen == 0

            pEndIdx = pHt->GetEnd();
            if( pEndIdx )
            {
                if( ( *pEndIdx > nTxtStartIdx ||
                      ( *pEndIdx == nTxtStartIdx &&
                        nAttrStartIdx == nTxtStartIdx ) ) )
                {
                    if( RES_TXTATR_REFMARK != ( nWhich = pHt->Which()) )
                    {
                        // Attribut liegt im Bereich, also kopieren
                        if( 0 != ( pNewHt = pDest->Insert( pHt->GetAttr(),
                                                nOldPos, nOldPos ) ) )
                            lcl_CopyHint( nWhich, pHt, pNewHt, pOtherDoc, pDest );
                    }
                    else if( !pOtherDoc ? GetDoc()->IsCopyIsMove()
                                        : 0 == pOtherDoc->GetRefMark(
                                        pHt->GetRefMark().GetRefName() ) )
                        pDest->Insert( pHt->GetAttr(), nOldPos, nOldPos );
                }
            }
        }
    }

    if( this != pDest )
    {
        // Frames benachrichtigen, sonst verschwinden die Ftn-Nummern
        SwUpdateAttr aHint( nOldPos, nOldPos, 0 );
        pDest->Modify( 0, &aHint );
    }
}

/*************************************************************************
|*  SwTxtNode::Copy()
|*  Beschreibung        kopiert Zeichen und Attibute in pDest,
|*                      wird angehaengt
*************************************************************************/

void SwTxtNode::Copy( SwTxtNode *pDest, const SwIndex &rStart, xub_StrLen nLen )
{
    SwIndex aIdx( pDest, pDest->aText.Len() );
    Copy( pDest, aIdx, rStart, nLen );
}

void SwTxtNode::Copy( SwTxtNode *pDest, const SwIndex &rDestStart,
                      const SwIndex &rStart, xub_StrLen nLen)
{
    xub_StrLen nTxtStartIdx = rStart.GetIndex();
    xub_StrLen nDestStart = rDestStart.GetIndex();      // alte Pos merken

    if( !nLen )
    {
        // wurde keine Laenge angegeben, dann Kopiere die Attribute
        // an der Position rStart.
        CopyAttr( pDest, nTxtStartIdx, nDestStart );

        // harte Absatz umspannende Attribute kopieren
        if( GetpSwAttrSet() )
        {
            // alle, oder nur die CharAttribute ?
            if( nDestStart || pDest->GetpSwAttrSet() ||
                nLen != pDest->GetTxt().Len() )
            {
                SfxItemSet aCharSet( pDest->GetDoc()->GetAttrPool(),
                                    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                                    RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
                                    RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
                                    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
                                    0 );
                aCharSet.Put( *GetpSwAttrSet() );
                if( aCharSet.Count() )
                    pDest->SetAttr( aCharSet, nDestStart, nDestStart );
            }
            else
                GetpSwAttrSet()->CopyToModify( *pDest );
        }

        return;
    }

    // 1. Text kopieren
    xub_StrLen i = pDest->aText.Len() - nDestStart;
    //JP 15.02.96: Bug 25537 - Attributbehandlung am Ende fehlt! Darum
    //              ueber die InsertMethode den Text einfuegen und nicht
    //              selbst direkt
    pDest->Insert( aText.Copy( nTxtStartIdx, nLen ), rDestStart,
                                                    INS_EMPTYEXPAND );

    // um reale Groesse Updaten !
    nLen = pDest->aText.Len() - nDestStart - i;
    if( !nLen )                                 // String nicht gewachsen ??
        return;

    i = 0;
    const xub_StrLen *pEndIdx = 0;
    xub_StrLen nAttrStartIdx = 0;
    const SwTxtAttr *pHt = 0;
    SwTxtAttr *pNewHt = 0;

    SwDoc* pOtherDoc = pDest->GetDoc();
    if( pOtherDoc == GetDoc() )
        pOtherDoc = 0;

    // harte Absatz umspannende Attribute kopieren
    if( GetpSwAttrSet() )
    {
        // alle, oder nur die CharAttribute ?
        if( nDestStart || pDest->GetpSwAttrSet() ||
            nLen != pDest->GetTxt().Len() )
        {
            SfxItemSet aCharSet( pDest->GetDoc()->GetAttrPool(),
                                RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                                RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
                                RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
                                RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
                                0 );
            aCharSet.Put( *GetpSwAttrSet() );
            if( aCharSet.Count() )
                pDest->SetAttr( aCharSet, nDestStart, nDestStart + nLen );
        }
        else
            GetpSwAttrSet()->CopyToModify( *pDest );
    }


    const BOOL bUndoNodes = !pOtherDoc && GetDoc()->GetUndoNds() == &GetNodes();

    // Ende erst jetzt holen, weil beim Kopieren in sich selbst der
    // Start-Index und alle Attribute vorher aktualisiert werden.
    nTxtStartIdx = rStart.GetIndex();
    xub_StrLen nEnd = nTxtStartIdx + nLen;

    // 2. Attribute kopieren
    // durch das Attribute-Array, bis der Anfang des Geltungsbereiches
    // des Attributs hinter dem zu kopierenden Bereich liegt
    USHORT nWhich, nSize = pSwpHints ? pSwpHints->Count() : 0;
    xub_StrLen nAttrStt, nAttrEnd;

    // wird in sich selbst kopiert, dann kann beim Einfuegen ein
    // Attribut geloescht werden. Darum erst ins Tmp-Array kopieren und
    // dann erst ins eigene uebertragen.
    SwpHts aArr( 5 );

    // Del-Array fuer alle RefMarks ohne Ausdehnung
    SwpHts aRefMrkArr;

        //Achtung: kann ungueltig sein!!
    while( ( i < nSize ) &&
           ((nAttrStartIdx = *(*pSwpHints)[i]->GetStart()) < nEnd) )
    {
        pHt = (*pSwpHints)[i];
        pNewHt = 0;
        pEndIdx = pHt->GetEnd();
        nWhich = pHt->Which();

        // JP 26.04.94: REFMARK's werden nie kopiert. Hat das Refmark aber
        //              keinen Bereich umspannt, so steht im Text ein 255
        //              dieses muss entfernt werden. Trick: erst kopieren,
        //              erkennen und sammeln, nach dem kopieren Loeschen.
        //              Nimmt sein Zeichen mit ins Grab !!
        // JP 14.08.95: Duerfen RefMarks gemovt werden?
        int bCopyRefMark = RES_TXTATR_REFMARK == nWhich && ( bUndoNodes ||
                           (!pOtherDoc ? GetDoc()->IsCopyIsMove()
                                      : 0 == pOtherDoc->GetRefMark(
                                        pHt->GetRefMark().GetRefName() )));

        if( pEndIdx && RES_TXTATR_REFMARK == nWhich && !bCopyRefMark )
        {
            ++i;
            continue;
        }

        if( nAttrStartIdx < nTxtStartIdx )
        {
            // Anfang liegt vor dem Bereich
            if( pEndIdx && ( nAttrEnd = *pEndIdx ) > nTxtStartIdx )
            {
                // Attribut mit einem Bereich
                // und das Ende des Attribut liegt im Bereich
                nAttrStt = nDestStart;
                nAttrEnd = nAttrEnd > nEnd
                            ? rDestStart.GetIndex()
                            : nDestStart + nAttrEnd - nTxtStartIdx;
            }
            else
            {
                ++i;
                continue;
            }
        }
        else
        {
            // der Anfang liegt innerhalb des Bereiches
            nAttrStt = nDestStart + ( nAttrStartIdx - nTxtStartIdx );
            if( pEndIdx )
                nAttrEnd = *pEndIdx > nEnd
                            ? rDestStart.GetIndex()
                            : nDestStart + ( *pEndIdx - nTxtStartIdx );
            else
                nAttrEnd = nAttrStt;
        }

        if( pDest == this )
        {
            // die Daten kopieren
            pNewHt = MakeTxtAttr( pHt->GetAttr(), nAttrStt, nAttrEnd );

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//JP 23.04.95:  erstmal so gesondert hier behandeln. Am Besten ist es
//              aber im CopyFtn wenn die pDestFtn keinen StartNode hat,
//              sich diesen dann anlegt.
//              Aber so kurz vor der BETA besser nicht anfassen.
            if( RES_TXTATR_FTN == nWhich )
            {
                SwTxtFtn* pFtn = (SwTxtFtn*)pNewHt;
                pFtn->ChgTxtNode( this );
                pFtn->MakeNewTextSection( GetNodes() );
                lcl_CopyHint( nWhich, pHt, pFtn, 0, 0 );
                pFtn->ChgTxtNode( 0 );
            }
            else
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            {
                lcl_CopyHint( nWhich, pHt, pNewHt, 0, pDest );
            }
            aArr.C40_INSERT( SwTxtAttr, pNewHt, aArr.Count() );
        }
        else
        {
            pNewHt = pDest->Insert( pHt->GetAttr(), nAttrStt,
                                    nAttrEnd, SETATTR_NOTXTATRCHR );
            if( pNewHt )
                lcl_CopyHint( nWhich, pHt, pNewHt, pOtherDoc, pDest );
            else if( !pEndIdx )
            {
                // Attribut wurde nicht kopiert, hat seinen Inhalt mitgenommen!
                // Damit der rest aber korrekt kopiert werden kann, muss das
                // Zeichen wieder an der Position stehen. Darum hier ein
                // "Dummy-TextAttribut" einfuegen, wird am Ende wieder entfernt!
                pNewHt = pDest->Insert( SwFmtHardBlank( 0xB7 ), nAttrStt, 0
                                        /*???, INS_NOHINTEXPAND*/ );
                aRefMrkArr.C40_INSERT( SwTxtAttr, pNewHt, aRefMrkArr.Count() );
            }
        }

        if( RES_TXTATR_REFMARK == nWhich && !pEndIdx && !bCopyRefMark )
        {
            aRefMrkArr.C40_INSERT( SwTxtAttr, pNewHt, aRefMrkArr.Count() );
        }

        ++i;
    }

    // nur falls im Array Attribute stehen (kann nur beim Kopieren
    // sich selbst passieren!!)
    for( i = 0; i < aArr.Count(); ++i )
        Insert( aArr[ i ], SETATTR_NOTXTATRCHR );

    if( pDest->GetpSwpHints() )
        for( i = 0; i < aRefMrkArr.Count(); ++i )
        {
            pNewHt = aRefMrkArr[i];
            if( pNewHt->GetEnd() )
            {
                pDest->GetpSwpHints()->Delete( pNewHt );
                pDest->DestroyAttr( pNewHt );
            }
            else
            {
                const SwIndex aIdx( pDest, *pNewHt->GetStart() );
                pDest->Erase( aIdx, 1 );
            }
        }

    CHECK_SWPHINTS(this);
}


/*
 * Rudimentaeres Editieren, damit die SwDoc-Insert-Methoden
 * funktionieren.
 */

SwTxtNode& SwTxtNode::Insert( const XubString   &rStr,
                              const SwIndex &rIdx, const USHORT nMode )
{
    ASSERT( rIdx <= aText.Len(), "Array ueberindiziert." );
    ASSERT( (ULONG)aText.Len() + (ULONG)rStr.Len() <= STRING_LEN,
            "STRING_LEN ueberschritten." );
    xub_StrLen aPos = rIdx.GetIndex();
    xub_StrLen nLen = aText.Len() - aPos;
    aText.Insert( rStr, aPos );
    nLen = aText.Len() - aPos - nLen;
    if( !nLen )                         // String nicht gewachsen ??
        return *this;
    Update( rIdx, nLen );       // um reale Groesse Updaten !

    // analog zu Insert(char) in txtedt.cxx:
    // 1) bei bHintExp leere Hints an rIdx.GetIndex suchen und aufspannen
    // 2) bei bHintExp == FALSE mitgezogene Feldattribute zuruecksetzen

    register USHORT i;

    if( pSwpHints )
    {
        for( i = 0; i < pSwpHints->Count() &&
                rIdx >= *(*pSwpHints)[i]->GetStart(); ++i )
        {
            SwTxtAttr *pHt = pSwpHints->GetHt( i );
            xub_StrLen* pEndIdx = pHt->GetEnd();
            if( !pEndIdx )
                continue;

            if( rIdx == *pEndIdx )
            {
                if( nMode & INS_NOHINTEXPAND || pHt->DontExpand() )
                {
                    // bei leeren Attributen auch Start veraendern
                    if( rIdx == *pHt->GetStart() )
                        *pHt->GetStart() -= nLen;
                    *pEndIdx -= nLen;
                }
                    // leere Hints an rIdx.GetIndex ?
                else if( nMode & INS_EMPTYEXPAND &&
                        *pEndIdx == *pHt->GetStart() )
                {
                    *pHt->GetStart() -= nLen;

                    // 8484: Symbol von 0-4, Roman von 4-6,
                    //       neuer Hint: Roman 4-4

                    // - while ... die Vorgaenger ueberpruefen:
                    // wenn gleiches Ende und gleicher Which
                    // => das Ende des gefundenen zuruecksetzen
                    const USHORT nWhich = pHt->Which();
                    SwTxtAttr *pFound;
                    xub_StrLen *pFoundEnd;
                    for( USHORT j = 0; j < i; ++j )
                        if( 0 != (pFound = pSwpHints->GetHt( j )) &&
                            nWhich == pFound->Which() &&
                            0 != ( pFoundEnd = pFound->GetEnd() ) &&
                            rIdx == *pFoundEnd )
                        {
                            *pFoundEnd -= nLen;
                            const USHORT nAktHtLen = pSwpHints->Count();
                            pSwpHints->DeleteAtPos(j);
                            Insert( pFound, SETATTR_NOHINTADJUST );
                            // AMA: Sicher ist sicher, falls pFound weiter hinten
                            // einsortiert wurde, koennte sonst die neue Position
                            // j vergessen werden!
                            if ( j ) --j;
                            // falls Attribute zusammengefasst werden, steht
                            // der "Index" ins Array falsch !
                            i -= nAktHtLen - pSwpHints->Count();
                            // Insert und Delete ?
                        }

                    // ist unser Attribut ueberhaupt noch vorhanden ?
                    if( pHt == pSwpHints->GetHt( i ) )
                    {
                        const USHORT nAktLen = pSwpHints->Count();
                        pSwpHints->DeleteAtPos(i);
                        Insert( pHt, SETATTR_NOHINTADJUST );
                        if( nAktLen > pSwpHints->Count() && i )
                            --i;
                    }
                    continue;
                }
                else
                    continue;
                pSwpHints->DeleteAtPos(i);
                Insert( pHt, SETATTR_NOHINTADJUST );
            }
            if ( !(nMode & INS_NOHINTEXPAND) &&
                 rIdx == nLen && *pHt->GetStart() == rIdx.GetIndex() )
            {
                // Kein Feld, am Absatzanfang, HintExpand
                pSwpHints->DeleteAtPos(i);
                *pHt->GetStart() -= nLen;
                Insert( pHt, SETATTR_NOHINTADJUST );
            }
        }
        if ( pSwpHints->CanBeDeleted() )
            DELETEZ( pSwpHints );
    }

    if ( GetDepends() )
    {
        SwInsTxt aHint( aPos, nLen );
        SwModify::Modify( 0, &aHint );
    }

    CHECK_SWPHINTS(this);
    return *this;
}

/*************************************************************************
|*
|*  SwTxtNode::Cut()
|*
|*  Beschreibung        text.doc
|*  Ersterstellung      VB 20.03.91
|*  Letzte Aenderung    JP 11.08.94
|*
*************************************************************************/

void SwTxtNode::Cut( SwTxtNode *pDest, const SwIndex &rStart, xub_StrLen nLen )
{
    if(pDest)
    {
        SwIndex aDestStt( pDest, pDest->GetTxt().Len() );
        _Cut( pDest, aDestStt, rStart, nLen, FALSE );
    }
    else
        Erase( rStart, nLen );
}


void SwTxtNode::_Cut( SwTxtNode *pDest, const SwIndex& rDestStart,
                     const SwIndex &rStart, xub_StrLen nLen, BOOL bUpdate )
{
    if(!pDest)
    {
        Erase( rStart, nLen );
        return;
    }

    // nicht im Dokument verschieben ?
    if( GetDoc() != pDest->GetDoc() )
    {
        Copy( pDest, rDestStart, rStart, nLen);
        Erase(rStart,nLen);
        return;
    }

    if( !nLen )
    {
        // wurde keine Laenge angegeben, dann Kopiere die Attribute
        // an der Position rStart.
        CopyAttr( pDest, rStart.GetIndex(), rDestStart.GetIndex() );
        return;
    }

    xub_StrLen nTxtStartIdx = rStart.GetIndex();
    xub_StrLen nDestStart = rDestStart.GetIndex();      // alte Pos merken
    xub_StrLen nInitSize = pDest->aText.Len();

    xub_StrLen *pEndIdx = 0;
    xub_StrLen nAttrStartIdx = 0;
    SwTxtAttr *pHt = 0;
    SwTxtAttr *pNewHt = 0;

    // wird in sich selbst verschoben, muss es gesondert behandelt werden !!
    if( pDest == this )
    {
        aText.Insert( aText, nTxtStartIdx, nLen, nDestStart );
        aText.Erase( nTxtStartIdx + (nDestStart<nTxtStartIdx ? nLen : 0), nLen );

        xub_StrLen nEnd = rStart.GetIndex() + nLen;
        USHORT n;

        // dann suche mal alle Attribute zusammen, die im verschobenen
        // Bereich liegen. Diese werden in das extra Array verschoben,
        // damit sich die Indizies beim Updaten nicht veraendern !!!
        SwIndexReg aTmpRegArr;
        SwpHts aArr( 5 );

        // 2. Attribute verschieben
        // durch das Attribute-Array, bis der Anfang des Geltungsbereiches
        // des Attributs hinter dem zu verschiebenden Bereich liegt
        USHORT nAttrCnt = 0, nWhich;
        while(  pSwpHints && nAttrCnt < pSwpHints->Count() &&
                (nAttrStartIdx = *(pHt = pSwpHints->GetHt(nAttrCnt))->
                                    GetStart()) < nEnd )
        {
            pNewHt = 0;
            pEndIdx = pHt->GetEnd();

            if(nAttrStartIdx < nTxtStartIdx)
            {
                // Anfang liegt vor dem Bereich
                if( RES_TXTATR_REFMARK != ( nWhich = pHt->Which() ) &&
                    pEndIdx && *pEndIdx > nTxtStartIdx )
                {
                    // Attribut mit einem Bereich
                    // und das Ende des Attribut liegt im Bereich
                    pNewHt = MakeTxtAttr( pHt->GetAttr(), 0,
                                        *pEndIdx > nEnd
                                            ? nLen
                                            : *pEndIdx - nTxtStartIdx );
                }
            }
            else
            {
                // der Anfang liegt vollstaendig im Bereich
                if( !pEndIdx || *pEndIdx < nEnd )
                {
                    // Attribut verschieben
                    pSwpHints->Delete( pHt );
                    // die Start/End Indicies neu setzen
                    *pHt->GetStart() = nAttrStartIdx - nTxtStartIdx;
                    if( pEndIdx )
                        *pHt->GetEnd() = *pEndIdx - nTxtStartIdx;
                    aArr.C40_INSERT( SwTxtAttr, pHt, aArr.Count() );
                    continue;           // while-Schleife weiter, ohne ++ !
                }
                    // das Ende liegt dahinter
                else if( RES_TXTATR_REFMARK != ( nWhich = pHt->Which() ))
                {
                    pNewHt = MakeTxtAttr( pHt->GetAttr(),
                            nAttrStartIdx - nTxtStartIdx,
                            !pEndIdx ? 0
                                     : ( *pEndIdx > nEnd
                                            ? nLen
                                            : *pEndIdx - nTxtStartIdx ));
                }
            }
            if( pNewHt )
            {
                // die Daten kopieren
                lcl_CopyHint( nWhich, pHt, pNewHt, 0, this );
                aArr.C40_INSERT( SwTxtAttr, pNewHt, aArr.Count() );
            }
            ++nAttrCnt;
        }

        if( bUpdate )
            // Update aller Indizies
            Update( rDestStart, nLen );
#ifdef CUTNOEXPAND
        else
            // wird am Ende eingefuegt, nur die Attribut-Indizies verschieben
            if( 0 < nLen && 0 < nInitSize && pSwpHints )
            {
                // siehe nach, ob an der Einfuegeposition das Ende eines
                // Attributes stand. Ist es kein Feld, muss es expandiert werden !!!
                for( n = 0; n < pSwpHints->Count(); n++ )
                {
                    pHt = pSwpHints->GetHt(n);
                    if( 0 != ( pEndIdx = pHt->GetEnd() ) &&
                        *pEndIdx == nInitSize )
                        *pEndIdx += nLen;
                }
            }
#endif
        CHECK_SWPHINTS(this);

        Update( rStart, nLen, TRUE );

        CHECK_SWPHINTS(this);

        // dann setze die kopierten/geloeschten Attribute in den Node
        if( nDestStart <= nTxtStartIdx )
            nTxtStartIdx += nLen;
        else
            nDestStart -= nLen;

        for( n = 0; n < aArr.Count(); ++n )
        {
            pNewHt = aArr[n];
            *pNewHt->GetStart() = nDestStart + *pNewHt->GetStart();
            if( 0 != ( pEndIdx = pNewHt->GetEnd() ))
                *pEndIdx = nDestStart + *pEndIdx;
            Insert( pNewHt, SETATTR_NOTXTATRCHR );
        }
    }
    else
    {
        xub_StrLen i = nInitSize - nDestStart;
        pDest->aText.Insert( aText, nTxtStartIdx, nLen, nDestStart );
        aText.Erase( nTxtStartIdx, nLen );
        nLen = pDest->aText.Len() - nDestStart - i;  // um reale Groesse Updaten !
        if( !nLen )                 // String nicht gewachsen ??
            return;

        i = 0;

        if( bUpdate )
            // Update aller Indizies
            pDest->Update( rDestStart, nLen);
#ifdef CUTNOEXPAND
        else
            // wird am Ende eingefuegt, nur die Attribut-Indizies verschieben
            if( 0 < nLen && 0 < nInitSize && pDest->pSwpHints )
            {
                // siehe nach, ob an der Einfuegeposition das Ende eines
                // Attributes stand. Ist es kein Feld, muss es expandiert werden !!!
                for( USHORT n = 0; n < pDest->pSwpHints->Count(); n++ )
                {
                    pHt = pDest->pSwpHints->GetHt(n);
                    if( 0 != ( pEndIdx = pHt->GetEnd() ) &&
                        *pEndIdx == nInitSize )
                        *pEndIdx += nLen;
                }
            }
#endif
        CHECK_SWPHINTS(pDest);

        USHORT nEnd = rStart.GetIndex() + nLen;
        SwDoc* pOtherDoc = pDest->GetDoc();
        if( pOtherDoc == GetDoc() )
            pOtherDoc = 0;
        const BOOL bUndoNodes = !pOtherDoc && GetDoc()->GetUndoNds() == &GetNodes();

        // harte Absatz umspannende Attribute kopieren
        if( GetpSwAttrSet() )
        {
            // alle, oder nur die CharAttribute ?
            if( nInitSize || pDest->GetpSwAttrSet() ||
                nLen != pDest->GetTxt().Len() )
            {
                SfxItemSet aCharSet( pDest->GetDoc()->GetAttrPool(),
                                    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                                    RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
                                    RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
                                    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
                                    0 );
                aCharSet.Put( *GetpSwAttrSet() );
                if( aCharSet.Count() )
                    pDest->SetAttr( aCharSet, nDestStart, nDestStart + nLen );
            }
            else
                GetpSwAttrSet()->CopyToModify( *pDest );
        }

        // 2. Attribute verschieben
        // durch das Attribute-Array, bis der Anfang des Geltungsbereiches
        // des Attributs hinter dem zu verschiebenden Bereich liegt
        USHORT nAttrCnt = 0, nWhich;
        while( pSwpHints && nAttrCnt < pSwpHints->Count() &&
                ( (nAttrStartIdx = *(pHt = pSwpHints->GetHt(nAttrCnt))->
                                    GetStart()) < nEnd ) )
        {
            pNewHt = 0;
            pEndIdx = pHt->GetEnd();

            if(nAttrStartIdx < nTxtStartIdx)
            {
                // Anfang liegt vor dem Bereich
                if( ( RES_TXTATR_REFMARK != ( nWhich = pHt->Which() )
                    || bUndoNodes ) && pEndIdx && *pEndIdx > nTxtStartIdx )
                {
                    // Attribut mit einem Bereich
                    // und das Ende des Attribut liegt im Bereich
                    pNewHt = pDest->MakeTxtAttr( pHt->GetAttr(), nDestStart,
                                        nDestStart + (
                                        *pEndIdx > nEnd
                                            ? nLen
                                            : *pEndIdx - nTxtStartIdx ) );
                }
            }
            else
            {
                // der Anfang liegt vollstaendig im Bereich
                nWhich = pHt->Which();
                if( !pEndIdx || *pEndIdx < nEnd ||
                    ( !pOtherDoc && !bUndoNodes && RES_TXTATR_REFMARK
                        == nWhich ) )
                {
                    // Attribut verschieben
                    pSwpHints->Delete( pHt );
                    // die Start/End Indicies neu setzen
                    *pHt->GetStart() =
                            nDestStart + (nAttrStartIdx - nTxtStartIdx);
                    if( pEndIdx )
                        *pHt->GetEnd() = nDestStart + (
                                        *pEndIdx > nEnd
                                            ? nLen
                                            : *pEndIdx - nTxtStartIdx );
                    pDest->Insert( pHt, SETATTR_NOTXTATRCHR | SETATTR_DONTREPLACE );
                    continue;           // while-Schleife weiter, ohne ++ !
                }
                    // das Ende liegt dahinter
                else if( RES_TXTATR_REFMARK != nWhich || bUndoNodes )
                {
                    pNewHt = MakeTxtAttr( pHt->GetAttr(),
                            nDestStart + (nAttrStartIdx - nTxtStartIdx),
                            !pEndIdx ? 0
                                     : nDestStart + ( *pEndIdx > nEnd
                                            ? nLen
                                            : *pEndIdx - nTxtStartIdx ));
                }
            }
            if ( pNewHt )
            {
                if( pDest->Insert( pNewHt, SETATTR_NOTXTATRCHR | SETATTR_DONTREPLACE ))
                    lcl_CopyHint( nWhich, pHt, pNewHt, pOtherDoc, pDest );
            }
            ++nAttrCnt;
        }
        // sollten jetzt noch leere Attribute rumstehen, dann haben diese
        // eine hoehere Praezedenz. Also herausholen und das Array updaten.
        // Die dabei entstehenden leeren Hints werden von den gesicherten
        // "uebergeplaettet".   (Bug: 6977)
        if( pSwpHints && nAttrCnt < pSwpHints->Count() )
        {
            SwpHts aArr( 5 );
            for( ; nAttrCnt < pSwpHints->Count() &&
                    nEnd == *(pHt = pSwpHints->GetHt(nAttrCnt))->GetStart();
                   ++nAttrCnt )
            {
                if( 0 != ( pEndIdx = pHt->GetEnd() ) && *pEndIdx == nEnd )
                {
                    aArr.C40_INSERT( SwTxtAttr, pHt, aArr.Count() );
                    pSwpHints->Delete( pHt );
                    --nAttrCnt;
                }
            }
            Update( rStart, nLen, TRUE );

            for( nAttrCnt = 0; nAttrCnt < aArr.Count(); ++nAttrCnt )
            {
                pHt = aArr[ nAttrCnt ];
                *pHt->GetStart() = *pHt->GetEnd() = rStart.GetIndex();
                Insert( pHt );
            }
        }
        else
            Update( rStart, nLen, TRUE );

        CHECK_SWPHINTS(this);
    }

    if( pSwpHints && pSwpHints->CanBeDeleted() )
        DELETEZ( pSwpHints );

    // Frames benachrichtigen;
    SwInsTxt aInsHint( nDestStart, nLen );
    pDest->SwCntntNode::Modify( 0, &aInsHint );
    SwDelTxt aDelHint( nTxtStartIdx, nLen );
    SwCntntNode::Modify( 0, &aDelHint );
}


SwTxtNode& SwTxtNode::Erase(const SwIndex &rIdx, xub_StrLen nCount,
                            const USHORT nMode )
{
    ASSERT( rIdx <= aText.Len(), "Array ueberindiziert." );

    const xub_StrLen nCnt = STRING_LEN == nCount
                      ? aText.Len() - rIdx.GetIndex() : nCount;
    aText.Erase( rIdx.GetIndex(), nCnt );

    /* GCAttr(); alle leeren weggwerfen ist zu brutal.
     * Es duerfen nur die wegggeworfen werden,
     * die im Bereich liegen und nicht am Ende des Bereiches liegen
     */

    // Abfrage auf pSwpHints weil TextFelder und FlyFrames Text loeschen
    // (Rekursion)!!
    for( USHORT i = 0; pSwpHints && i < pSwpHints->Count(); ++i )
    {
        SwTxtAttr *pHt = pSwpHints->GetHt(i);

        const xub_StrLen nHtStt = *pHt->GetStart();

        if( nHtStt < rIdx.GetIndex() )
            continue;

        // TextFelder und FlyFrames loeschen Text (Rekursion)!!
        const xub_StrLen nEndIdx = rIdx.GetIndex() + nCnt;
        if( nHtStt > nEndIdx )
            // die Hints sind nach Ende sortiert, also ist Start
            // vom Hint groesser als EndIdx dann Abbrechen
            break;

        const xub_StrLen* pHtEndIdx = pHt->GetEnd();
        const USHORT nWhich = pHt->Which();

        if( !pHtEndIdx )
        {
            // TxtHints ohne EndIndex werden natuerlich auch geloescht:
            if( RES_TXTATR_BEGIN <= nWhich && RES_TXTATR_END > nWhich &&
                nHtStt >= rIdx.GetIndex() && nHtStt < nEndIdx )
            {
                pSwpHints->DeleteAtPos(i);
                // Damit im Dtor der TxtAttr ohne End die CH_TXTATR nicht
                // geloescht werden...
                *(pHt->GetStart()) = USHRT_MAX;
                DestroyAttr( pHt );
                --i;
            }
            continue;
        }

        if( *pHtEndIdx >= nEndIdx && !(
            !(INS_EMPTYEXPAND & nMode) && *pHtEndIdx == nEndIdx &&
            (nWhich == RES_TXTATR_TOXMARK || nWhich == RES_TXTATR_REFMARK)) )
            continue;

        pSwpHints->DeleteAtPos(i);
        DestroyAttr( pHt );
        --i;
    }

    if ( pSwpHints && pSwpHints->CanBeDeleted() )
        DELETEZ( pSwpHints );

    Update( rIdx, nCnt, TRUE );

    if( 1 == nCnt )
    {
        SwDelChr aHint( rIdx.GetIndex() );
        SwModify::Modify( 0, &aHint );
    }
    else
    {
        SwDelTxt aHint( rIdx.GetIndex(), nCnt );
        SwModify::Modify( 0, &aHint );
    }

    CHECK_SWPHINTS(this);
    return *this;
}

/***********************************************************************
#*  Class       :   SwTxtNode
#*  Methode     :   GCAttr
#*
#*  Beschreibung
#*                  text.doc
#*
#*  Datum       :   MS 28.11.90
#*  Update      :   VB 24.07.91
#***********************************************************************/

void SwTxtNode::GCAttr()
{
    if ( !pSwpHints )
        return;

    const SwTxtAttr *pHt = 0;
    const xub_StrLen *pEndIdx = 0;
    BOOL   bChanged = FALSE;
    USHORT nMin = aText.Len(),
           nMax = 0;
    BOOL bAll = nMin != 0; // Bei leeren Absaetzen werden nur die
                           // INet-Formate entfernt.

    for ( USHORT i = 0; pSwpHints && i < pSwpHints->Count(); ++i )
    {
        pHt = (*pSwpHints)[i];

        // wenn Ende und Start gleich sind --> loeschen
        pEndIdx = pHt->GetEnd();
        if( pEndIdx && (*pEndIdx == *pHt->GetStart())
            && ( bAll || pHt->Which() == RES_TXTATR_INETFMT ) )
        {
            bChanged = TRUE;
            nMin = Min( nMin, *pHt->GetStart() );
            nMax = Max( nMax, *pHt->GetEnd() );
            DestroyAttr( pSwpHints->Cut(i) );
            --i;
        }
        else
            ((SwTxtAttr*)pHt)->SetDontExpand( FALSE );
    }
    if ( pSwpHints && pSwpHints->CanBeDeleted() )
        DELETEZ( pSwpHints );

    if(bChanged)
    {
        //TxtFrm's reagieren auf aHint, andere auf aNew
        SwUpdateAttr aHint( nMin, nMax, 0 );
        SwModify::Modify( 0, &aHint );
        SwFmtChg aNew( GetTxtColl() );
        SwModify::Modify( 0, &aNew );
    }
}

const SwNodeNum* SwTxtNode::UpdateNum( const SwNodeNum& rNum )
{
    if( NO_NUMBERING == rNum.GetLevel() )       // kein Nummerierung mehr ?
    {
        if( !pNdNum )
            return 0;
        delete pNdNum, pNdNum = 0;
    }
    else
    {
        if( !pNdNum )
            pNdNum = new SwNodeNum( rNum );
        else if( !( *pNdNum == rNum ))
            *pNdNum = rNum;
    }
    NumRuleChgd();
    return pNdNum;
}

const SwNumRule* SwTxtNode::GetNumRule() const
{
    const SwNumRule* pRet = 0;
    const SfxPoolItem* pItem = GetNoCondAttr( RES_PARATR_NUMRULE, TRUE );
    if( pItem && ((SwNumRuleItem*)pItem)->GetValue().Len() )
        pRet = GetDoc()->FindNumRulePtr( ((SwNumRuleItem*)pItem)->GetValue() );
    return pRet;
}

void SwTxtNode::NumRuleChgd()
{
#ifndef NUM_RELSPACE

    // 6969: Aktualisierung der NumPortions auch bei leeren Zeilen!
    SwInsTxt aHint( 0, 0 );
    SwModify::Modify( 0, &aHint );

#else

    if( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( FALSE );
    }
    SetInSwFntCache( FALSE );

    SvxLRSpaceItem& rLR = (SvxLRSpaceItem&)GetSwAttrSet().GetLRSpace();
    SwModify::Modify( &rLR, &rLR );

#endif
}

const SwNodeNum* SwTxtNode::UpdateOutlineNum( const SwNodeNum& rNum )
{
    if( NO_NUMBERING == rNum.GetLevel() )       // kein Nummerierung mehr ?
    {
        if( !pNdOutl )
            return 0;
        delete pNdOutl, pNdOutl = 0;
    }
    else
    {
        if( !pNdOutl )
            pNdOutl = new SwNodeNum( rNum );
        else if( !( *pNdOutl == rNum ))
            *pNdOutl = rNum;
    }

    // 6969: Aktualisierung der NumPortions auch bei leeren Zeilen!
    NumRuleChgd();
    return pNdOutl;
}

SwTxtNode* SwTxtNode::_MakeNewTxtNode( const SwNodeIndex& rPos, BOOL bNext,
                                        BOOL bChgFollow )
{
    /* hartes PageBreak/PageDesc/ColumnBreak aus AUTO-Set ignorieren */
    SwAttrSet* pNewAttrSet = 0;
    if( GetpSwAttrSet() )
    {
        pNewAttrSet = new SwAttrSet( *GetpSwAttrSet() );
        SwAttrSet* pTmpSet = GetpSwAttrSet();

        if( bNext )     // der naechste erbt keine Breaks!
            pTmpSet = pNewAttrSet;

        // PageBreaks/PageDesc/ColBreak rausschmeissen.
        BOOL bRemoveFromCache = 0 != pTmpSet->ClearItem( RES_PAGEDESC );
        if( SFX_ITEM_SET == pTmpSet->GetItemState( RES_BREAK, FALSE ) )
        {
            pTmpSet->ClearItem( RES_BREAK );
            bRemoveFromCache = TRUE;
        }
        if( !bNext && bRemoveFromCache && IsInCache() )
        {
            SwFrm::GetCache().Delete( this );
            SetInCache( FALSE );
        }
    }
    SwNodes& rNds = GetNodes();

    SwTxtFmtColl* pColl = GetTxtColl();

    SwTxtNode *pNode = new SwTxtNode( rPos, pColl, pNewAttrSet );

    if( pNewAttrSet )
        delete pNewAttrSet;

    const SwNumRule* pRule = GetNumRule();
    if( pRule && rNds.IsDocNodes() )
    {
        // ist am Node eine Nummerierung gesetzt und wird dieser vor dem
        // alten eingefuegt, so kopiere die Nummer
        if( !bNext && pNdNum && NO_NUMBERING != pNdNum->GetLevel() )
        {
            if( pNode->pNdNum )
                *pNode->pNdNum = *pNdNum;
            else
                pNode->pNdNum = new SwNodeNum( *pNdNum );

            // SetValue immer auf default zurueck setzem
            pNdNum->SetSetValue( USHRT_MAX );
            if( pNdNum->IsStart() )
            {
                pNdNum->SetStart( FALSE );
                pNode->pNdNum->SetStart( TRUE );
            }

            // Ein SplitNode erzeugt !!immer!! einen neuen Level, NO_NUM
            // kann nur ueber eine entsprechende Methode erzeugt werden !!
            if( NO_NUMLEVEL & pNdNum->GetLevel() )
            {
                pNdNum->SetLevel( pNdNum->GetLevel() & ~NO_NUMLEVEL );
#ifndef NUM_RELSPACE
                SetNumLSpace( TRUE );
#endif
            }
        }
        rNds.GetDoc()->UpdateNumRule( pRule->GetName(), pNode->GetIndex() );
    }

    // jetzt kann es sein, das durch die Nummerierung dem neuen Node eine
    // Vorlage aus dem Pool zugewiesen wurde. Dann darf diese nicht
    // nochmal uebergeplaettet werden !!
    if( pColl != pNode->GetTxtColl() ||
        ( bChgFollow && pColl != GetTxtColl() ))
        return pNode;       // mehr duerfte nicht gemacht werden oder ????

    pNode->_ChgTxtCollUpdateNum( 0, pColl ); // fuer Nummerierung/Gliederung
    if( bNext || !bChgFollow )
        return pNode;

    SwTxtFmtColl *pNextColl = &pColl->GetNextTxtFmtColl();
    ChgFmtColl( pNextColl );

    return pNode;
}

SwCntntNode* SwTxtNode::AppendNode( const SwPosition & rPos )
{
    // Position hinter dem eingefuegt wird
    SwNodeIndex aIdx( rPos.nNode, 1 );
    SwTxtNode* pNew = _MakeNewTxtNode( aIdx, TRUE );
    if( GetDepends() )
        MakeFrms( *pNew );
    return pNew;
}

/*************************************************************************
 *                      SwTxtNode::GetTxtAttr
 *
 * Diese Methode liefert nur Textattribute auf der Position nIdx
 * zurueck, die kein EndIdx besitzen und denselben Which besitzen.
 * Ueblicherweise steht an dieser Position ein CH_TXTATR.
 * Bei RES_TXTATR_END entfaellt die Pruefung auf den Which-Wert.
 *************************************************************************/

SwTxtAttr *SwTxtNode::GetTxtAttr( const xub_StrLen nIdx,
                                  const USHORT nWhichHt ) const
{
    if( pSwpHints )
    {
        for( USHORT i = 0; i < pSwpHints->Count(); ++i )
        {
            SwTxtAttr *pPos = pSwpHints->GetHt(i);
            const xub_StrLen nStart = *pPos->GetStart();
            if( nIdx < nStart )
                return 0;
            if( nIdx == nStart && !pPos->GetEnd() )
            {
                if( RES_TXTATR_END == nWhichHt || nWhichHt == pPos->Which() )
                    return pPos;
                else
                    return 0;
            }
        }
    }
    return 0;
}

/*************************************************************************
 *                      SwTxtNode::GetExpandTxt
 *************************************************************************/
// Felder werden expandiert:

XubString SwTxtNode::GetNumString() const
{
    const SwNodeNum* pNum;
    const SwNumRule* pRule;
    if( (( 0 != ( pNum = GetNum() ) &&
            0 != ( pRule = GetNumRule() )) ||
            ( 0 != ( pNum = GetOutlineNum() ) &&
            0 != ( pRule = GetDoc()->GetOutlineNumRule() ) ) ) &&
        pNum->GetLevel() < MAXLEVEL &&
        pRule->Get( pNum->GetLevel() ).IsTxtFmt() )
        return pRule->MakeNumString( *pNum );
    return aEmptyStr;
}

long SwTxtNode::GetLeftMarginWithNum( BOOL bTxtLeft ) const
{
    long nOffset;
    const SwNodeNum* pNum;
    const SwNumRule* pRule;
    if( (( 0 != ( pNum = GetNum() ) &&
            0 != ( pRule = GetNumRule() )) ||
            ( 0 != ( pNum = GetOutlineNum() ) &&
            0 != ( pRule = GetDoc()->GetOutlineNumRule() ) ) ) &&
            pNum->GetLevel() < NO_NUM )
    {
        const SwNumFmt& rFmt = pRule->Get( GetRealLevel( pNum->GetLevel() ) );
        nOffset = rFmt.GetAbsLSpace();

        if( !bTxtLeft )
        {
            if( 0 > rFmt.GetFirstLineOffset() &&
                nOffset > -rFmt.GetFirstLineOffset() )
                nOffset += rFmt.GetFirstLineOffset();
            else
                nOffset = 0;
        }

        if( pRule->IsAbsSpaces() )
            nOffset -= GetSwAttrSet().GetLRSpace().GetLeft();
    }
    else
        nOffset = 0;
    return nOffset;
}

BOOL SwTxtNode::GetFirstLineOfsWithNum( short& rFLOffset ) const
{
    const SwNodeNum* pNum;
    const SwNumRule* pRule;
    if( (( 0 != ( pNum = GetNum() ) &&
            0 != ( pRule = GetNumRule() )) ||
            ( 0 != ( pNum = GetOutlineNum() ) &&
            0 != ( pRule = GetDoc()->GetOutlineNumRule() ) ) ) &&
            pNum->GetLevel() < NO_NUM )
    {
        if( NO_NUMLEVEL & pNum->GetLevel() )
            rFLOffset = 0;
        else
            rFLOffset = pRule->Get( pNum->GetLevel() ).GetFirstLineOffset();
        return TRUE;
    }
    rFLOffset = GetSwAttrSet().GetLRSpace().GetTxtFirstLineOfst();
    return FALSE;
}

void SwTxtNode::Replace0xFF( XubString& rTxt, xub_StrLen& rTxtStt,
                            xub_StrLen nEndPos, BOOL bExpandFlds ) const
{
    if( GetpSwpHints() )
    {
        sal_Unicode cSrchChr = CH_TXTATR_BREAKWORD;
        for( int nSrchIter = 0; 2 > nSrchIter; ++nSrchIter,
                                cSrchChr = CH_TXTATR_INWORD )
        {
            xub_StrLen nPos = rTxt.Search( cSrchChr );
            while( STRING_NOTFOUND != nPos && nPos < nEndPos )
            {
                const SwTxtAttr* pAttr = GetTxtAttr( rTxtStt + nPos );
                if( pAttr )
                {
                    switch( pAttr->Which() )
                    {
                    case RES_TXTATR_FIELD:
                        rTxt.Erase( nPos, 1 );
                        if( bExpandFlds )
                        {
                            const XubString aExpand( ((SwTxtFld*)pAttr)->GetFld().
                                                    GetFld()->Expand() );
                            rTxt.Insert( aExpand, nPos );
                            nPos += aExpand.Len();
                            nEndPos += aExpand.Len();
                            rTxtStt -= aExpand.Len();
                        }
                        ++rTxtStt;
                        break;
                    case RES_TXTATR_HARDBLANK:
                        rTxt.SetChar( nPos, ((SwTxtHardBlank*)pAttr)->GetChar() );
                        ++nPos;
                        ++nEndPos;
                        break;
                    case RES_TXTATR_FTN:
                        rTxt.Erase( nPos, 1 );
                        if( bExpandFlds )
                        {
                            const SwFmtFtn& rFtn = pAttr->GetFtn();
                            XubString sExpand;
                            if( rFtn.GetNumStr().Len() )
                                sExpand = rFtn.GetNumStr();
                            else if( rFtn.IsEndNote() )
                                sExpand = GetDoc()->GetEndNoteInfo().aFmt.
                                                GetNumStr( rFtn.GetNumber() );
                            else
                                sExpand = GetDoc()->GetFtnInfo().aFmt.
                                                GetNumStr( rFtn.GetNumber() );
                            rTxt.Insert( sExpand, nPos );
                            nPos += sExpand.Len();
                            nEndPos += sExpand.Len();
                            rTxtStt -= sExpand.Len();
                        }
                        ++rTxtStt;
                        break;
                    default:
                        rTxt.Erase( nPos, 1 );
                        ++rTxtStt;
                    }
                }
                else
                    ++nPos, ++nEndPos;
                nPos = rTxt.Search( cSrchChr, nPos );
            }
        }
    }
}

XubString SwTxtNode::GetExpandTxt( const xub_StrLen nIdx, const xub_StrLen nLen,
                                const BOOL bWithNum  ) const
{
    XubString aTxt( GetTxt().Copy( nIdx, nLen ) );
    xub_StrLen nTxtStt = nIdx;
    Replace0xFF( aTxt, nTxtStt, aTxt.Len(), TRUE );

    if( bWithNum )
        aTxt.Insert( GetNumString(), 0 );

    return aTxt;
}

BOOL SwTxtNode::GetExpandTxt( SwTxtNode& rDestNd, const SwIndex* pDestIdx,
                        xub_StrLen nIdx, xub_StrLen nLen, BOOL bWithNum ) const
{
    if( &rDestNd == this )
        return FALSE;

    SwIndex aDestIdx( &rDestNd, rDestNd.GetTxt().Len() );
    if( pDestIdx )
        aDestIdx = *pDestIdx;
    xub_StrLen nDestStt = aDestIdx.GetIndex();

    // Text einfuegen
    rDestNd.Insert( GetTxt().Copy( nIdx, nLen ), aDestIdx );
    nLen = aDestIdx.GetIndex() - nDestStt;

    // alle FontAttribute mit CHARSET Symbol in dem Bereich setzen
    if( pSwpHints )
    {
        xub_StrLen nInsPos = nDestStt - nIdx;
        for( USHORT i = 0; i < pSwpHints->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pSwpHints)[i];
            xub_StrLen nAttrStartIdx;
            USHORT nWhich = pHt->Which();
            if( nIdx + nLen <= ( nAttrStartIdx = *pHt->GetStart() ) )
                break;      // ueber das Textende

            const SvxFontItem* pFont;
            const xub_StrLen *pEndIdx = pHt->GetEnd();
            if( pEndIdx && *pEndIdx > nIdx && (
                ( RES_CHRATR_FONT == nWhich &&
                  RTL_TEXTENCODING_SYMBOL == (pFont = &pHt->GetFont())->GetCharSet() ) ||
                ( RES_TXTATR_CHARFMT == nWhich &&
                  RTL_TEXTENCODING_SYMBOL == (pFont = &pHt->GetCharFmt().
                      GetCharFmt()->GetFont())->GetCharSet() )))
            {
                // Attribut liegt im Bereich, also kopieren
                rDestNd.Insert( *pFont, nInsPos + nAttrStartIdx,
                                        nInsPos + *pEndIdx );
            }
            else if( !pEndIdx && nAttrStartIdx >= nIdx )
            {
                aDestIdx = nInsPos + nAttrStartIdx;
                switch( nWhich )
                {
                case RES_TXTATR_FIELD:
                    {
                        const XubString aExpand( ((SwTxtFld*)pHt)->GetFld().GetFld()->Expand() );
                        if( aExpand.Len() )
                        {
                            aDestIdx++;     // dahinter einfuegen;
                            rDestNd.Insert( aExpand, aDestIdx );
                            aDestIdx = nInsPos + nAttrStartIdx;
                            nInsPos += aExpand.Len();
                        }
                        rDestNd.Erase( aDestIdx, 1 );
                        --nInsPos;
                    }
                    break;

                case RES_TXTATR_HARDBLANK:
                    rDestNd.aText.SetChar( nInsPos + nAttrStartIdx,
                                ((SwTxtHardBlank*)pHt)->GetChar() );
                    break;

                case RES_TXTATR_FTN:
                    {
                        const SwFmtFtn& rFtn = pHt->GetFtn();
                        XubString sExpand;
                        if( rFtn.GetNumStr().Len() )
                            sExpand = rFtn.GetNumStr();
                        else if( rFtn.IsEndNote() )
                            sExpand = GetDoc()->GetEndNoteInfo().aFmt.
                                            GetNumStr( rFtn.GetNumber() );
                        else
                            sExpand = GetDoc()->GetFtnInfo().aFmt.
                                            GetNumStr( rFtn.GetNumber() );
                        if( sExpand.Len() )
                        {
                            aDestIdx++;     // dahinter einfuegen;
                            rDestNd.Insert( SvxEscapementItem(
                                    SVX_ESCAPEMENT_SUPERSCRIPT ),
                                    aDestIdx.GetIndex(),
                                    aDestIdx.GetIndex() );
                            rDestNd.Insert( sExpand, aDestIdx, INS_EMPTYEXPAND );
                            aDestIdx = nInsPos + nAttrStartIdx;
                            nInsPos += sExpand.Len();
                        }
                        rDestNd.Erase( aDestIdx, 1 );
                        --nInsPos;
                    }
                    break;

                default:
                    rDestNd.Erase( aDestIdx, 1 );
                    --nInsPos;
                }
            }
        }
    }

    if( bWithNum )
    {
        aDestIdx = nDestStt;
        rDestNd.Insert( GetNumString(), aDestIdx );
    }
    return TRUE;
}


XubString SwTxtNode::GetRedlineTxt( xub_StrLen nIdx, xub_StrLen nLen,
                                BOOL bExpandFlds, BOOL bWithNum ) const
{
    SvUShorts aRedlArr;
    const SwDoc* pDoc = GetDoc();
    USHORT nRedlPos = pDoc->GetRedlinePos( *this, REDLINE_DELETE );
    if( USHRT_MAX != nRedlPos )
    {
        // es existiert fuer den Node irgendein Redline-Delete-Object
        const ULONG nNdIdx = GetIndex();
        for( ; nRedlPos < pDoc->GetRedlineTbl().Count() ; ++nRedlPos )
        {
            const SwRedline* pTmp = pDoc->GetRedlineTbl()[ nRedlPos ];
            if( REDLINE_DELETE == pTmp->GetType() )
            {
                const SwPosition *pRStt = pTmp->Start(), *pREnd = pTmp->End();
                if( pRStt->nNode < nNdIdx )
                {
                    if( pREnd->nNode > nNdIdx )
                        // Absatz ist komplett geloescht
                        return aEmptyStr;
                    else if( pREnd->nNode == nNdIdx )
                    {
                        // von 0 bis nContent ist alles geloescht
                        aRedlArr.Insert( xub_StrLen(0), aRedlArr.Count() );
                        aRedlArr.Insert( pREnd->nContent.GetIndex(), aRedlArr.Count() );
                    }
                }
                else if( pRStt->nNode == nNdIdx )
                {
                    aRedlArr.Insert( pRStt->nContent.GetIndex(), aRedlArr.Count() );
                    if( pREnd->nNode == nNdIdx )
                        aRedlArr.Insert( pREnd->nContent.GetIndex(), aRedlArr.Count() );
                    else
                    {
                        aRedlArr.Insert( GetTxt().Len(), aRedlArr.Count() );
                        break;      // mehr kann nicht kommen
                    }
                }
                else
                    break;      // mehr kann nicht kommen
            }
        }
    }

    XubString aTxt( GetTxt().Copy( nIdx, nLen ) );

    xub_StrLen nTxtStt = nIdx, nIdxEnd = nIdx + aTxt.Len();
    for( USHORT n = 0; n < aRedlArr.Count(); n += 2 )
    {
        xub_StrLen nStt = aRedlArr[ n ], nEnd = aRedlArr[ n+1 ];
        if( ( nIdx <= nStt && nStt <= nIdxEnd ) ||
            ( nIdx <= nEnd && nEnd <= nIdxEnd ))
        {
            if( nStt < nIdx ) nStt = nIdx;
            if( nIdxEnd < nEnd ) nEnd = nIdxEnd;
            xub_StrLen nDelCnt = nEnd - nStt;
            aTxt.Erase( nStt - nTxtStt, nDelCnt );
            Replace0xFF( aTxt, nTxtStt, nStt - nTxtStt, bExpandFlds );
            nTxtStt += nDelCnt;
        }
        else if( nStt >= nIdxEnd )
            break;
    }
    Replace0xFF( aTxt, nTxtStt, aTxt.Len(), bExpandFlds );

    if( bWithNum )
        aTxt.Insert( GetNumString(), 0 );
    return aTxt;
}

/*************************************************************************
 *                      SwTxtNode::GetExpandTxt
 *************************************************************************/
// Felder werden expandiert:

void SwTxtNode::Replace( const SwIndex& rStart, xub_Unicode cCh )
{

    ASSERT( rStart.GetIndex() < aText.Len(), "ausserhalb des Strings" );
    SwTxtAttr* pHt;
    if( ( CH_TXTATR_BREAKWORD == aText.GetChar( rStart.GetIndex() ) ||
          CH_TXTATR_INWORD == aText.GetChar( rStart.GetIndex() )) &&
        0 != ( pHt = GetTxtAttr( rStart.GetIndex() ) ))
    {
        Delete( pHt );
        aText.Insert( cCh, rStart.GetIndex() );
    }
    else
        aText.SetChar( rStart.GetIndex(), cCh );

    SwDelTxt aDelHint( rStart.GetIndex(), 1 );
    SwModify::Modify( 0, &aDelHint );

    SwInsTxt aHint( rStart.GetIndex(), 1 );
    SwModify::Modify( 0, &aHint );
}


void SwTxtNode::Replace( const SwIndex& rStart, xub_StrLen nLen,
                            const XubString& rText )
{
    ASSERT( rStart.GetIndex() < aText.Len() &&
            rStart.GetIndex() + nLen <= aText.Len(),
            "ausserhalb des Strings" );
    SwTxtAttr* pHt;
    xub_StrLen nStart = rStart.GetIndex();
    xub_StrLen nEnde = nStart + nLen;
    xub_StrLen nDelLen = nLen;
    for( xub_StrLen nPos = nStart; nPos < nEnde; ++nPos )
        if( ( CH_TXTATR_BREAKWORD == aText.GetChar( nPos ) ||
              CH_TXTATR_INWORD == aText.GetChar( nPos )) &&
            0 != ( pHt = GetTxtAttr( nPos ) ))
        {
            Delete( pHt );
            --nEnde;
            --nLen;
        }

    if( nLen && rText.Len() )
    {
        // dann das 1. Zeichen ersetzen den Rest loschen und einfuegen
        // Dadurch wird die Attributierung des 1. Zeichen expandiert!
        aText.SetChar( nStart, rText.GetChar( 0 ) );

        ((SwIndex&)rStart)++;
        aText.Erase( rStart.GetIndex(), nLen - 1 );
        Update( rStart, nLen - 1, TRUE );

        XubString aTmpTxt( rText ); aTmpTxt.Erase( 0, 1 );
        aText.Insert( aTmpTxt, rStart.GetIndex() );
        Update( rStart, aTmpTxt.Len(), FALSE );
    }
    else
    {
        aText.Erase( nStart, nLen );
        Update( rStart, nLen, TRUE );

        aText.Insert( rText, nStart );
        Update( rStart, rText.Len(), FALSE );
    }
    SwDelTxt aDelHint( nStart, nDelLen );
    SwModify::Modify( 0, &aDelHint );

    SwInsTxt aHint( nStart, rText.Len() );
    SwModify::Modify( 0, &aHint );
}

void SwTxtNode::Modify( SfxPoolItem* pOldValue, SfxPoolItem* pNewValue )
{
    // Bug 24616/24617:
    //      Modify ueberladen, damit beim Loeschen von Vorlagen diese
    //      wieder richtig verwaltet werden (Outline-Numerierung!!)
    //  Bug25481:
    //      bei Nodes im Undo nie _ChgTxtCollUpdateNum rufen.
    if( pOldValue && pNewValue && RES_FMT_CHG == pOldValue->Which() &&
        pRegisteredIn == ((SwFmtChg*)pNewValue)->pChangedFmt &&
        GetNodes().IsDocNodes() )
        _ChgTxtCollUpdateNum(
                        (SwTxtFmtColl*)((SwFmtChg*)pOldValue)->pChangedFmt,
                        (SwTxtFmtColl*)((SwFmtChg*)pNewValue)->pChangedFmt );

    SwCntntNode::Modify( pOldValue, pNewValue );
}



