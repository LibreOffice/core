/*************************************************************************
 *
 *  $RCSfile: docfmt.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-10-20 09:51:28 $
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

#define _ZFORLIST_DECLARE_TABLE
#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_USHORTS

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SO2REF_HXX //autogen
#include <so3/so2ref.hxx>
#endif
#ifndef SO2_DECL_SVLINKNAME_DEFINED
#define SO2_DECL_SVLINKNAME_DEFINED
SO2_DECL_REF(SvLinkName)
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _OFA_MISCCFG_HXX
#include <sfx2/misccfg.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _WORDSEL_HXX //autogen
#include <svtools/wordsel.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#define _ZFORLIST_DECLARE_TABLE
#include <svtools/zforlist.hxx>
#endif

#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
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
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>            // fuer SwHyphenBug (in SetDefault)
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>         // Fuer Sonderbehandlung in InsFrmFmt
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>           // Undo-Attr
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>           // servieren: Veraenderungen erkennen
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _REFFLD_HXX //autogen
#include <reffld.hxx>
#endif
#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;


SV_IMPL_PTRARR(SwFrmFmts,SwFrmFmtPtr)
SV_IMPL_PTRARR(SwCharFmts,SwCharFmtPtr)

//Spezifische Frameformate (Rahmen)
SV_IMPL_PTRARR(SwSpzFrmFmts,SwFrmFmtPtr)

/*
 * interne Funktionen
 */

BOOL SetTxtFmtCollNext( const SwTxtFmtCollPtr& rpTxtColl, void* pArgs )
{
    SwTxtFmtColl *pDel = (SwTxtFmtColl*) pArgs;
    if ( &rpTxtColl->GetNextTxtFmtColl() == pDel )
    {
        rpTxtColl->SetNextTxtFmtColl( *rpTxtColl );
    }
    return TRUE;
}

/*
 * Zuruecksetzen der harten Formatierung fuer Text
 */

// Uebergabeparameter fuer _Rst und lcl_SetTxtFmtColl
struct ParaRstFmt
{
    SwFmtColl* pFmtColl;
    SwHistory* pHistory;
    const SwPosition *pSttNd, *pEndNd;
    const SfxItemSet* pDelSet;
    USHORT nWhich;
    BOOL bReset, bResetAll, bInclRefToxMark;

    ParaRstFmt( const SwPosition* pStt, const SwPosition* pEnd,
            SwHistory* pHst, USHORT nWhch = 0, const SfxItemSet* pSet = 0 )
        : pSttNd( pStt ), pEndNd( pEnd ), pHistory( pHst ), nWhich( nWhch ),
            pDelSet( pSet ), bResetAll( TRUE ), pFmtColl( 0 ),
            bInclRefToxMark( FALSE )
    {}

    ParaRstFmt( SwHistory* pHst )
        : pSttNd( 0 ), pEndNd( 0 ), pHistory( pHst ), nWhich( 0 ),
        pDelSet( 0 ), bResetAll( TRUE ), pFmtColl( 0 ),
        bInclRefToxMark( FALSE )
    {}
};

/* in pArgs steht die ChrFmtTablle vom Dokument
 * (wird bei Selectionen am Start/Ende und bei keiner SSelection benoetigt)
 */

BOOL lcl_RstTxtAttr( const SwNodePtr& rpNd, void* pArgs )
{
    ParaRstFmt* pPara = (ParaRstFmt*)pArgs;
    SwTxtNode * pTxtNode = (SwTxtNode*)rpNd->GetTxtNode();
    if( pTxtNode && pTxtNode->GetpSwpHints() )
    {
        SwIndex aSt( pTxtNode, 0 );
        USHORT nEnd = pTxtNode->Len();

        if( &pPara->pSttNd->nNode.GetNode() == pTxtNode &&
            pPara->pSttNd->nContent.GetIndex() )
            aSt = pPara->pSttNd->nContent.GetIndex();

        if( &pPara->pEndNd->nNode.GetNode() == rpNd )
            nEnd = pPara->pEndNd->nContent.GetIndex();

        if( pPara->pHistory )
        {
            // fuers Undo alle Attribute sichern
            SwRegHistory aRHst( *pTxtNode, pPara->pHistory );
            pTxtNode->GetpSwpHints()->Register( &aRHst );
            pTxtNode->RstAttr( aSt, nEnd - aSt.GetIndex(), pPara->nWhich,
                                pPara->pDelSet, pPara->bInclRefToxMark );
            if( pTxtNode->GetpSwpHints() )
                pTxtNode->GetpSwpHints()->DeRegister();
        }
        else
            pTxtNode->RstAttr( aSt, nEnd - aSt.GetIndex(), pPara->nWhich,
                                pPara->pDelSet, pPara->bInclRefToxMark );
    }
    return TRUE;
}

BOOL lcl_RstAttr( const SwNodePtr& rpNd, void* pArgs )
{
    ParaRstFmt* pPara = (ParaRstFmt*)pArgs;
    SwCntntNode* pNode = (SwCntntNode*)rpNd->GetCntntNode();
    if( pNode && pNode->GetpSwAttrSet() )
    {
        // das erhalten der Break-Attribute und der NumRule kommt nicht ins Undo
        BOOL bLocked = pNode->IsModifyLocked();
        pNode->LockModify();
        SwDoc* pDoc = pNode->GetDoc();

        SfxItemSet aSet( pDoc->GetAttrPool(),
                            RES_PAGEDESC, RES_BREAK,
                            RES_PARATR_NUMRULE, RES_PARATR_NUMRULE,
                            RES_LR_SPACE, RES_LR_SPACE,
                            0 );
        SwAttrSet* pSet = pNode->GetpSwAttrSet();

        USHORT __READONLY_DATA aSavIds[ 3 ] = { RES_PAGEDESC, RES_BREAK,
                                                RES_PARATR_NUMRULE };

        const SfxPoolItem* pItem;
        for( USHORT n = 0; n < 3; ++n )
            if( SFX_ITEM_SET == pSet->GetItemState( aSavIds[n], FALSE, &pItem )
                && ( RES_PARATR_NUMRULE != aSavIds[n] ||
                     ((SwNumRuleItem*)pItem)->GetValue().Len() ))
            {
                aSet.Put( *pItem );
                pSet->ClearItem( aSavIds[n] );
            }

        if( !bLocked )
            pNode->UnlockModify();

        if( pPara )
        {
            SwRegHistory aRegH( pNode, *pNode, pPara->pHistory );

            if( pPara->pDelSet && pPara->pDelSet->Count() )
            {
                SfxItemIter aIter( *pPara->pDelSet );
                pItem = aIter.FirstItem();
                while( TRUE )
                {
                    pNode->ResetAttr( pItem->Which() );
                    if( aIter.IsAtEnd() )
                        break;
                    pItem = aIter.NextItem();
                }
            }
            else if( pPara->bResetAll )
                pNode->ResetAllAttr();
            else
                pNode->ResetAttr( RES_PARATR_BEGIN, POOLATTR_END - 1 );
        }
        else
            pNode->ResetAllAttr();

        if( aSet.Count() )
        {
            pNode->LockModify();
            pNode->SetAttr( aSet );

            if( !bLocked )
                pNode->UnlockModify();
        }
    }
    return TRUE;
}

void SwDoc::RstTxtAttr(const SwPaM &rRg, BOOL bInclRefToxMark )
{
    SwHistory* pHst = 0;
    SwDataChanged aTmp( rRg, 0 );
    if( DoesUndo() )
    {
        ClearRedo();
        SwUndoRstAttr* pUndo = new SwUndoRstAttr( rRg, RES_CHRFMT );
        pHst = pUndo->GetHistory();
        AppendUndo( pUndo );
    }
    const SwPosition *pStt = rRg.Start(), *pEnd = rRg.End();
    ParaRstFmt aPara( pStt, pEnd, pHst );
    aPara.bInclRefToxMark = bInclRefToxMark;
    GetNodes().ForEach( pStt->nNode.GetIndex(), pEnd->nNode.GetIndex()+1,
                        lcl_RstTxtAttr, &aPara );
    SetModified();
}

void SwDoc::ResetAttr( const SwPaM &rRg, BOOL bTxtAttr,
                        const SvUShortsSort* pAttrs )
{
    SwPaM* pPam = (SwPaM*)&rRg;
    BOOL bStopAttr = FALSE;
    if( !bTxtAttr && pAttrs && pAttrs->Count() &&
        RES_TXTATR_END > (*pAttrs)[ 0 ] )
        bTxtAttr = TRUE;

    if( !rRg.HasMark() )
    {
        SwTxtNode* pTxtNd = rRg.GetPoint()->nNode.GetNode().GetTxtNode();
        if( !pTxtNd )
            return ;

        pPam = new SwPaM( *rRg.GetPoint() );

        SwIndex& rSt = pPam->GetPoint()->nContent;
        USHORT nMkPos, nPtPos = rSt.GetIndex();
        const String& rStr = pTxtNd->GetTxt();

        // JP 22.08.96: Sonderfall: steht der Crsr in einem URL-Attribut
        //              dann wird dessen Bereich genommen
        const SwTxtAttr* pURLAttr;
        if( pTxtNd->HasHints() &&
            0 != ( pURLAttr = pTxtNd->GetTxtAttr( rSt, RES_TXTATR_INETFMT ))
            && pURLAttr->GetINetFmt().GetValue().Len() )
        {
            nMkPos = *pURLAttr->GetStart();
            nPtPos = *pURLAttr->GetEnd();
        }
        else if( WordSelection::IsInWord( rStr, nPtPos ) &&
            !WordSelection::IsStartWord( rStr, nPtPos ) &&
            !WordSelection::IsEndWord( rStr, nPtPos ) &&
            USHRT_MAX != ( nMkPos = WordSelection::GoStartWord( rStr, nPtPos )) &&
            USHRT_MAX != ( nPtPos = WordSelection::GoEndWord( rStr, nPtPos )) )
            ;
        else
        {
            nPtPos = nMkPos = rSt.GetIndex();
            if( bTxtAttr )
                pTxtNd->DontExpandFmt( rSt, TRUE );
        }

        rSt = nMkPos;
        pPam->SetMark();
        pPam->GetPoint()->nContent = nPtPos;
    }

    SwDataChanged aTmp( *pPam, 0 );
    SwHistory* pHst = 0;
    if( DoesUndo() )
    {
        ClearRedo();
        SwUndoRstAttr* pUndo = new SwUndoRstAttr( rRg,
                    bTxtAttr ? RES_CONDTXTFMTCOLL : RES_TXTFMTCOLL );
        if( pAttrs && pAttrs->Count() )
            pUndo->SetAttrs( *pAttrs );
        pHst = pUndo->GetHistory();
        AppendUndo( pUndo );
    }

    const SwPosition *pStt = pPam->Start(), *pEnd = pPam->End();
    ParaRstFmt aPara( pStt, pEnd, pHst );

    SfxItemSet aDelSet( GetAttrPool(), aTxtNodeSetRange );
    if( pAttrs && pAttrs->Count() )
    {
        for( USHORT n = pAttrs->Count(); n; )
            if( POOLATTR_END > (*pAttrs)[ --n ] )
                aDelSet.Put( *GetDfltAttr( (*pAttrs)[ n ] ));

        if( aDelSet.Count() )
            aPara.pDelSet = &aDelSet;
    }

    BOOL bAdd = TRUE;
    SwNodeIndex aTmpStt( pStt->nNode );
    SwNodeIndex aTmpEnd( pEnd->nNode );
    if( pStt->nContent.GetIndex() )     // nur ein Teil
    {
        // dann spaeter aufsetzen und alle CharFmtAttr -> TxtFmtAttr
        SwTxtNode* pTNd = aTmpStt.GetNode().GetTxtNode();
        if( pTNd && pTNd->GetpSwAttrSet() && pTNd->GetpSwAttrSet()->Count() )
        {
            SfxItemIter aIter( *pTNd->GetpSwAttrSet() );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            while( TRUE )
            {
                if( IsInRange( aCharFmtSetRange, pItem->Which() ))
                {
                    SwTxtAttr* pTAttr = pTNd->MakeTxtAttr( *pItem, 0,
                                                    pTNd->GetTxt().Len() );
                    if( !pTNd->pSwpHints )
                        pTNd->pSwpHints = new SwpHints;
                    pTNd->pSwpHints->SwpHintsArr::Insert( pTAttr );

                    if( pHst )
                    {
                        SwRegHistory aRegH( pTNd, *pTNd, pHst );
                        pTNd->ResetAttr( pItem->Which() );
                        pHst->Add( pTAttr, aTmpStt.GetIndex(), TRUE );
                    }
                    else
                        pTNd->ResetAttr( pItem->Which() );
                }
                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }
        }

        aTmpStt++;
    }
    if( pEnd->nContent.GetIndex() == pEnd->nNode.GetNode().GetCntntNode()->Len() )
        // dann spaeter aufsetzen und alle CharFmtAttr -> TxtFmtAttr
        aTmpEnd++, bAdd = FALSE;
    else if( pStt->nNode != pEnd->nNode || !pStt->nContent.GetIndex() )
    {
        SwTxtNode* pTNd = aTmpEnd.GetNode().GetTxtNode();
        if( pTNd && pTNd->GetpSwAttrSet() && pTNd->GetpSwAttrSet()->Count() )
        {
            SfxItemIter aIter( *pTNd->GetpSwAttrSet() );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            while( TRUE )
            {
                if( IsInRange( aCharFmtSetRange, pItem->Which() ))
                {
                    SwTxtAttr* pTAttr = pTNd->MakeTxtAttr( *pItem, 0,
                                                pTNd->GetTxt().Len() );
                    if( !pTNd->pSwpHints )
                        pTNd->pSwpHints = new SwpHints;
                    pTNd->pSwpHints->SwpHintsArr::Insert( pTAttr );
                    if( pHst )
                    {
                        SwRegHistory aRegH( pTNd, *pTNd, pHst );
                        pTNd->ResetAttr( pItem->Which() );
                        pHst->Add( pTAttr, aTmpEnd.GetIndex(), TRUE );
                    }
                    else
                        pTNd->ResetAttr( pItem->Which() );
                }
                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }
        }
    }

    if( aTmpStt < aTmpEnd )
        GetNodes().ForEach( pStt->nNode, aTmpEnd, lcl_RstAttr, &aPara );
    else if( !rRg.HasMark() )
    {
        aPara.bResetAll = FALSE;
        ::lcl_RstAttr( aNodes[ pStt->nNode ], &aPara );
        aPara.bResetAll = TRUE;
    }

    if( bTxtAttr )
    {
        if( bAdd )
            aTmpEnd++;
        GetNodes().ForEach( pStt->nNode, aTmpEnd, lcl_RstTxtAttr, &aPara );
    }

    if( pPam != &rRg )
        delete pPam;

    SetModified();
}



// Einfuegen der Hints nach Inhaltsformen;
// wird in SwDoc::Insert(..., SwFmtHint &rHt) benutzt

BOOL InsAttr( SwDoc *pDoc, const SwPaM &rRg, const SfxItemSet& rChgSet,
                USHORT nFlags, SwUndoAttr* pUndo )
{
    // teil die Sets auf (fuer Selektion in Nodes)
    SfxItemSet aCharSet( pDoc->GetAttrPool(),
                        RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                        RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
                        RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
                        RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
                        0 );
    SfxItemSet aOtherSet( pDoc->GetAttrPool(),
                        RES_PARATR_BEGIN, RES_PARATR_END-1,
                        RES_FRMATR_BEGIN, RES_FRMATR_END-1,
                        RES_GRFATR_BEGIN, RES_GRFATR_END-1,
                        0 );

    aCharSet.Put( rChgSet );
    aOtherSet.Put( rChgSet );

    SwHistory* pHistory = pUndo ? pUndo->GetHistory() : 0;
    BOOL bRet = FALSE;
    const SwPosition *pStt = rRg.Start(), *pEnd = rRg.End();
    SwCntntNode* pNode = pStt->nNode.GetNode().GetCntntNode();

    if( pNode && pNode->IsTxtNode() )
    {
        const SwIndex& rSt = pStt->nContent;

        // Attribute ohne Ende haben keinen Bereich
        {
            SfxItemSet aTxtSet( pDoc->GetAttrPool(),
                        RES_TXTATR_NOEND_BEGIN, RES_TXTATR_NOEND_END-1 );
            aTxtSet.Put( rChgSet );
            if( aTxtSet.Count() )
            {
                SwRegHistory( (SwTxtNode*)pNode, aTxtSet, rSt.GetIndex(),
                                rSt.GetIndex(), nFlags, pHistory );
                bRet = TRUE;

                if( pDoc->IsRedlineOn() || (!pDoc->IsIgnoreRedline() &&
                    pDoc->GetRedlineTbl().Count() ))
                {
                    SwPaM aPam( pStt->nNode, pStt->nContent.GetIndex()-1,
                                pStt->nNode, pStt->nContent.GetIndex() );

                    if( pUndo )
                        pUndo->SaveRedlineData( aPam, TRUE );

                    if( pDoc->IsRedlineOn() )
                        pDoc->AppendRedline( new SwRedline( REDLINE_INSERT, aPam ));
                    else
                        pDoc->SplitRedline( aPam );
                }
            }
        }

        // TextAttribute mit Ende expandieren nie ihren Bereich
        {
            // CharFnt wird gesondert behandelt !!!
            // JP 22.08.96: URL-Attribute auch!!
            SfxItemSet aTxtSet( pDoc->GetAttrPool(),
                                RES_TXTATR_DUMMY4,
                                // RES_TXTATR_REFMARK,
                                RES_TXTATR_TOXMARK );

            aTxtSet.Put( rChgSet );
            if( aTxtSet.Count() )
            {
                USHORT nInsCnt = rSt.GetIndex();
                USHORT nEnd = pStt->nNode == pEnd->nNode
                                ? pEnd->nContent.GetIndex()
                                : pNode->Len();
                SwRegHistory( (SwTxtNode*)pNode, aTxtSet, nInsCnt,
                                nEnd, nFlags, pHistory );
                bRet = TRUE;

                if( pDoc->IsRedlineOn() || (!pDoc->IsIgnoreRedline() &&
                     pDoc->GetRedlineTbl().Count() ) )
                {
                    // wurde Text-Inhalt eingefuegt? (RefMark/TOXMarks ohne Ende)
                    BOOL bTxtIns = nInsCnt != rSt.GetIndex();
                    // wurde Inhalt eingefuegt oder ueber die Selektion gesetzt?
                    SwPaM aPam( pStt->nNode, bTxtIns ? nInsCnt + 1 : nEnd,
                                pStt->nNode, nInsCnt );
                    if( pUndo )
                        pUndo->SaveRedlineData( aPam, bTxtIns );

                    if( pDoc->IsRedlineOn() )
                        pDoc->AppendRedline( new SwRedline( bTxtIns
                                ? REDLINE_INSERT : REDLINE_FORMAT, aPam ));
                    else if( bTxtIns )
                        pDoc->SplitRedline( aPam );
                }
            }
        }
    }

    // bei PageDesc's, die am Node gesetzt werden, muss immer das
    // Auto-Flag gesetzt werden!!
    const SvxLRSpaceItem* pLRSpace = 0;
    if( aOtherSet.Count() )
    {
        SwTableNode* pTblNd;
        const SwFmtPageDesc* pDesc;
        if( SFX_ITEM_SET == aOtherSet.GetItemState( RES_PAGEDESC,
                        FALSE, (const SfxPoolItem**)&pDesc ))
        {
            if( pNode )
            {
                // Auto-Flag setzen, nur in Vorlagen ist ohne Auto !
                SwFmtPageDesc aNew( *pDesc );
                // Bug 38479: AutoFlag wird jetzt in der WrtShell gesetzt
                // aNew.SetAuto();

                // Tabellen kennen jetzt auch Umbrueche
                if( 0 != ( pTblNd = pNode->FindTableNode() ) )
                {
                    // dann am Tabellen Format setzen
                    SwFrmFmt* pFmt = pTblNd->GetTable().GetFrmFmt();
                    SwRegHistory aRegH( pFmt, *pTblNd, pHistory );
                    pFmt->SetAttr( aNew );
                }
                else
                {
                    SwRegHistory aRegH( pNode, *pNode, pHistory );
                    pNode->SetAttr( aNew );
                }
            }
            aOtherSet.ClearItem( RES_PAGEDESC );
            if( !aOtherSet.Count() )
                return TRUE;
        }

        // Tabellen kennen jetzt auch Umbrueche
        const SvxFmtBreakItem* pBreak;
        if( pNode && 0 != (pTblNd = pNode->FindTableNode() ) &&
            SFX_ITEM_SET == aOtherSet.GetItemState( RES_BREAK,
                        FALSE, (const SfxPoolItem**)&pBreak ) )
        {
            // dann am Tabellen Format setzen
            SwFrmFmt* pFmt = pTblNd->GetTable().GetFrmFmt();
            SwRegHistory aRegH( pFmt, *pTblNd, pHistory );
            pFmt->SetAttr( *pBreak );

            aOtherSet.ClearItem( RES_BREAK );
            if( !aOtherSet.Count() )
                return TRUE;
        }

        // fuer Sonderbehandlung von LR-Space innerhalb einer Numerierung !!!
        aOtherSet.GetItemState( RES_LR_SPACE, FALSE,
                                (const SfxPoolItem**)&pLRSpace );

        {
            // wenns eine PoolNumRule ist, diese ggfs. anlegen
            const SwNumRuleItem* pRule;
            USHORT nPoolId;
            if( SFX_ITEM_SET == aOtherSet.GetItemState( RES_PARATR_NUMRULE,
                                FALSE, (const SfxPoolItem**)&pRule ) &&
                !pDoc->FindNumRulePtr( pRule->GetValue() ) &&
                USHRT_MAX != (nPoolId = pDoc->GetPoolId( pRule->GetValue(),
                                GET_POOLID_NUMRULE )) )
                pDoc->GetNumRuleFromPool( nPoolId );
        }

    }

    if( !rRg.HasMark() )        // kein Bereich
    {
        if( !pNode )
            return bRet;

        if( pNode->IsTxtNode() && aCharSet.Count() )
        {
            SwTxtNode* pTxtNd = (SwTxtNode*)pNode;
            const SwIndex& rSt = pStt->nContent;
            USHORT nMkPos, nPtPos = rSt.GetIndex();
            const String& rStr = pTxtNd->GetTxt();

            // JP 22.08.96: Sonderfall: steht der Crsr in einem URL-Attribut
            //              dann wird dessen Bereich genommen
            const SwTxtAttr* pURLAttr;
            if( pTxtNd->HasHints() &&
                0 != ( pURLAttr = pTxtNd->GetTxtAttr( rSt, RES_TXTATR_INETFMT ))
                && pURLAttr->GetINetFmt().GetValue().Len() )
            {
                nMkPos = *pURLAttr->GetStart();
                nPtPos = *pURLAttr->GetEnd();
            }
            else if( WordSelection::IsInWord( rStr, nPtPos ) &&
                    !WordSelection::IsStartWord( rStr, nPtPos ) &&
                    !WordSelection::IsEndWord( rStr, nPtPos ) &&
                USHRT_MAX != ( nMkPos = WordSelection::GoStartWord( rStr, nPtPos )) &&
                USHRT_MAX != ( nPtPos = WordSelection::GoEndWord( rStr, nPtPos )) )
                ;
            else
                nPtPos = nMkPos = rSt.GetIndex();

            // erstmal die zu ueberschreibenden Attribute aus dem
            // SwpHintsArray entfernen, wenn die Selektion den gesamten
            // Absatz umspannt. (Diese Attribute werden als FormatAttr.
            // eingefuegt und verdraengen nie die TextAttr.!)
            if( !(nFlags & SETATTR_DONTREPLACE ) &&
                pTxtNd->HasHints() && !nMkPos && nPtPos == rStr.Len() )
            {
                SwIndex aSt( pTxtNd );
                if( pHistory )
                {
                    // fuers Undo alle Attribute sichern
                    SwRegHistory aRHst( *pTxtNd, pHistory );
                    pTxtNd->GetpSwpHints()->Register( &aRHst );
                    pTxtNd->RstAttr( aSt, nPtPos, 0, &aCharSet );
                    if( pTxtNd->GetpSwpHints() )
                        pTxtNd->GetpSwpHints()->DeRegister();
                }
                else
                    pTxtNd->RstAttr( aSt, nPtPos, 0, &aCharSet );
            }

            // eintragen des Attributes im Node erledigt die SwRegHistory !!
            SwRegHistory( (SwTxtNode*)pNode, aCharSet,
                            nMkPos, nPtPos, nFlags, pHistory );
            bRet = TRUE;

            if( pDoc->IsRedlineOn() )
            {
                SwPaM aPam( *pNode, nMkPos, *pNode, nPtPos );

                if( pUndo )
                    pUndo->SaveRedlineData( aPam, FALSE );
                pDoc->AppendRedline( new SwRedline( REDLINE_FORMAT, aPam ));
            }
        }
        if( aOtherSet.Count() )
        {
            SwRegHistory aRegH( pNode, *pNode, pHistory );
            pNode->SetAttr( aOtherSet );
            bRet = TRUE;
        }
        return bRet;
    }

    if( pDoc->IsRedlineOn() && aCharSet.Count() )
    {
        if( pUndo )
            pUndo->SaveRedlineData( rRg, FALSE );
        pDoc->AppendRedline( new SwRedline( REDLINE_FORMAT, rRg ));
    }

    /* jetzt wenn Bereich */
    ULONG nNodes = 0;

    SwNodeIndex aSt( pDoc->GetNodes() );
    SwNodeIndex aEnd( pDoc->GetNodes() );
    SwIndex aCntEnd( pEnd->nContent );

    if( pNode )
    {
        USHORT nLen = pNode->Len();
        if( pStt->nNode != pEnd->nNode )
            aCntEnd.Assign( pNode, nLen );

        if( pStt->nContent.GetIndex() != 0 || aCntEnd.GetIndex() != nLen )
        {
            // eintragen des Attributes im Node erledigt die SwRegHistory !!
            if( pNode->IsTxtNode() && aCharSet.Count() )
            {
                SwRegHistory( (SwTxtNode*)pNode, aCharSet,
                                pStt->nContent.GetIndex(), aCntEnd.GetIndex(),
                                nFlags, pHistory );
            }

            if( aOtherSet.Count() )
            {
                SwRegHistory aRegH( pNode, *pNode, pHistory );
                pNode->SetAttr( aOtherSet );
            }

            // lediglich Selektion in einem Node.
            if( pStt->nNode == pEnd->nNode )
                return TRUE;
            ++nNodes;
            aSt.Assign( pStt->nNode.GetNode(), +1 );
        }
        else
            aSt = pStt->nNode;
        aCntEnd = pEnd->nContent; // aEnd wurde veraendert !!
    }
    else
        aSt.Assign( pStt->nNode.GetNode(), +1 );

    // aSt zeigt jetzt auf den ersten vollen Node

    /*
     * die Selektion umfasst mehr als einen Node
     */
    if( pStt->nNode < pEnd->nNode )
    {
        pNode = pEnd->nNode.GetNode().GetCntntNode();
        if(pNode)
        {
            USHORT nLen = pNode->Len();
            if( aCntEnd.GetIndex() != nLen )
            {
            // eintragen des Attributes im Node erledigt die SwRegHistory !!
                if( pNode->IsTxtNode() && aCharSet.Count() )
                {
                    SwRegHistory( (SwTxtNode*)pNode, aCharSet,
                                    0, aCntEnd.GetIndex(), nFlags, pHistory );
                }

                if( aOtherSet.Count() )
                {
                    SwRegHistory aRegH( pNode, *pNode, pHistory );
                    pNode->SetAttr( aOtherSet );
                }

                ++nNodes;
                aEnd = pEnd->nNode;
            }
            else
                aEnd.Assign( pEnd->nNode.GetNode(), +1 );
        }
        else
            aEnd = pEnd->nNode;
    }
    else
        aEnd.Assign( pEnd->nNode.GetNode(), +1 );

    // aEnd zeigt jetzt HINTER den letzten voll Node

    /* Bearbeitung der vollstaendig selektierten Nodes. */
// alle Attribute aus dem Set zuruecksetzen !!
    if( aCharSet.Count() && !( SETATTR_DONTREPLACE & nFlags ) )
    {

        ParaRstFmt aPara( pStt, pEnd, pHistory, 0, &aCharSet );
        pDoc->GetNodes().ForEach( aSt, aEnd, lcl_RstTxtAttr, &aPara );
    }

#ifdef USED
//JP 30.10.96: siehe unten
    // sollte ueber mehrere Nodes das SwFmtChrFmt gesetzt werden ??
    const SfxPoolItem* pChrFmtItem = 0;
    aCharSet.GetItemState( RES_TXTATR_CHARFMT, FALSE, &pChrFmtItem );
#endif
    BOOL bCreateSwpHints =
        SFX_ITEM_SET == aCharSet.GetItemState( RES_TXTATR_CHARFMT, FALSE ) ||
        SFX_ITEM_SET == aCharSet.GetItemState( RES_TXTATR_INETFMT, FALSE );

    for(; aSt < aEnd; aSt++ )
    {
        pNode = aSt.GetNode().GetCntntNode();
        if( !pNode )
            continue;

        SwTxtNode* pTNd = pNode->GetTxtNode();
        if( pHistory )
        {
            SwRegHistory aRegH( pNode, *pNode, pHistory );
            SwpHints *pSwpHints;

#ifdef USED
//JP 30.10.96: Das loeschen der Zeichen erledigt schon das SwpHints-Array
//              Warum dann hier doppelt?
//              !!  Ausserdem ist die Sonderbehandlung fuer die
//              !!  Zeichenvorlage/INetAttribut falsch

            // loesche alle Text-Attribute, die durch den Set "ersetzt" werden
            if( pTNd && 0 != ( pSwpHints = pTNd->GetpSwpHints() ) &&
                pSwpHints->Count() )
            {
                pSwpHints->Register( &aRegH );

                for( USHORT n = pSwpHints->Count(); n;  )
                {
                    SwTxtAttr* pAttr = pSwpHints->GetHt( --n );
                    if( !pAttr->GetEnd() || RES_CHRATR_END <= pAttr->Which() )
                        continue;
                    if( pChrFmtItem || SFX_ITEM_SET ==
                                aCharSet.GetItemState( pAttr->Which() ) )
                    {
                        pTNd->Delete( pAttr, TRUE );
                        if( !pTNd->GetpSwpHints() )
                        {
                            pSwpHints = 0;
                            break;
                        }
                    }
                }
                if( pSwpHints )
                    pSwpHints->DeRegister();
            }
#endif
            if( pTNd && aCharSet.Count() )
            {
                pSwpHints = bCreateSwpHints ? &pTNd->GetOrCreateSwpHints()
                                            : pTNd->GetpSwpHints();
                if( pSwpHints )
                    pSwpHints->Register( &aRegH );

                pTNd->SetAttr( aCharSet, 0, pTNd->GetTxt().Len(), nFlags );
                if( pSwpHints )
                    pSwpHints->DeRegister();
            }
            if( aOtherSet.Count() )
                pNode->SetAttr( aOtherSet );
        }
        else
        {
            if( pTNd && aCharSet.Count() )
                pTNd->SetAttr( aCharSet, 0, pTNd->GetTxt().Len(), nFlags );
            if( aOtherSet.Count() )
                pNode->SetAttr( aOtherSet );
        }
        ++nNodes;
    }

    return nNodes != 0;
}


BOOL SwDoc::Insert( const SwPaM &rRg, const SfxPoolItem &rHt, USHORT nFlags )
{
    SwDataChanged aTmp( rRg, 0 );
    BOOL bRet;
    SwUndoAttr* pUndoAttr = 0;
    if( DoesUndo() )
    {
        ClearRedo();
        pUndoAttr = new SwUndoAttr( rRg, rHt, nFlags );
    }

    SfxItemSet aSet( GetAttrPool(), rHt.Which(), rHt.Which() );
    aSet.Put( rHt );
    bRet = InsAttr( this, rRg, aSet, nFlags, pUndoAttr );

/*  if( INSATTR_DONTEXPAND & nFlags )
    {
        USHORT nWhich = rHt.Which();
        const SwPosition* pPos = rRg.End();
        SwTxtNode* pTxtNd = GetNodes()[ pPos->nNode ]->GetTxtNode();
        SwpHints* pHts;
        if( pTxtNd && 0 != ( pHts = pTxtNd->GetpSwpHints()) )
        {
            USHORT nPos = pHts->GetEndCount();
            while( nPos )
            {
                SwTxtAttr *pTmp = pHts->GetEnd( --nPos );
                USHORT *pEnd = pTmp->GetEnd();
                if( !pEnd || *pEnd > nEnd )
                    continue;
                if( nEnd != *pEnd )
                    nPos = 0;
                else if( nWhich == pTmp->Which() )
                    pTmp->SetDontExpand( TRUE );
            }
        }
    }
*/
    if( DoesUndo() )
        AppendUndo( pUndoAttr );

    if( bRet )
        SetModified();
    return bRet;
}

BOOL SwDoc::Insert( const SwPaM &rRg, const SfxItemSet &rSet, USHORT nFlags )
{
    SwDataChanged aTmp( rRg, 0 );
    SwUndoAttr* pUndoAttr = 0;
    if( DoesUndo() )
    {
        ClearRedo();
        pUndoAttr = new SwUndoAttr( rRg, rSet );
    }

    BOOL bRet = InsAttr( this, rRg, rSet, nFlags, pUndoAttr );

    if( DoesUndo() )
        AppendUndo( pUndoAttr );

    if( bRet )
        SetModified();
    return bRet;
}


    // Setze das Attribut im angegebenen Format. Ist Undo aktiv, wird
    // das alte in die Undo-History aufgenommen
void SwDoc::SetAttr( const SfxPoolItem& rAttr, SwFmt& rFmt )
{
    SfxItemSet aSet( GetAttrPool(), rAttr.Which(), rAttr.Which() );
    aSet.Put( rAttr );
    SetAttr( aSet, rFmt );
}


    // Setze das Attribut im angegebenen Format. Ist Undo aktiv, wird
    // das alte in die Undo-History aufgenommen
void SwDoc::SetAttr( const SfxItemSet& rSet, SwFmt& rFmt )
{
    if( DoesUndo() )
    {
        ClearRedo();
        _UndoFmtAttr aTmp( rFmt );
        rFmt.SetAttr( rSet );
        if( aTmp.pUndo )
            AppendUndo( aTmp.pUndo );
    }
    else
        rFmt.SetAttr( rSet );
    SetModified();
}

int lcl_SetNewDefTabStops( SwTwips nOldWidth, SwTwips nNewWidth,
                                SvxTabStopItem& rChgTabStop )
{
    // dann aender bei allen TabStop die default's auf den neuen Wert
    // !!! Achtung: hier wird immer auf dem PoolAttribut gearbeitet,
    //              damit nicht in allen Sets die gleiche Berechnung
    //              auf dem gleichen TabStop (gepoolt!) vorgenommen
    //              wird. Als Modify wird ein FmtChg verschickt.

    USHORT nOldCnt = rChgTabStop.Count();
    if( !nOldCnt || nOldWidth == nNewWidth )
        return FALSE;

    // suche den Anfang der Defaults
    SvxTabStop* pTabs = ((SvxTabStop*)rChgTabStop.GetStart())
                        + (nOldCnt-1);
    for( USHORT n = nOldCnt; n ; --n, --pTabs )
        if( SVX_TAB_ADJUST_DEFAULT != pTabs->GetAdjustment() )
            break;
    ++n;
    if( n < nOldCnt )   // die DefTabStops loeschen
        rChgTabStop.Remove( n, nOldCnt - n );
    return TRUE;
}

    // Setze das Attribut als neues default Attribut in diesem Dokument.
    // Ist Undo aktiv, wird das alte in die Undo-History aufgenommen
void SwDoc::SetDefault( const SfxPoolItem& rAttr )
{
    SfxItemSet aSet( GetAttrPool(), rAttr.Which(), rAttr.Which() );
    aSet.Put( rAttr );
    SetDefault( aSet );
}

void SwDoc::SetDefault( const SfxItemSet& rSet )
{
    if( !rSet.Count() )
        return;

    SwModify aCallMod( 0 );
    SwAttrSet aOld( GetAttrPool(), rSet.GetRanges() ),
            aNew( GetAttrPool(), rSet.GetRanges() );
    SfxItemIter aIter( rSet );
    register USHORT nWhich;
    const SfxPoolItem* pItem = aIter.GetCurItem();
    while( TRUE )
    {
        nWhich = pItem->Which();
        aOld.Put( GetAttrPool().GetDefaultItem( nWhich ) );
        GetAttrPool().SetPoolDefaultItem( *pItem );
        aNew.Put( GetAttrPool().GetDefaultItem( nWhich ) );

        if( RES_CHRATR_BEGIN <= nWhich && RES_TXTATR_END > nWhich )
        {
            aCallMod.Add( pDfltTxtFmtColl );
            aCallMod.Add( pDfltCharFmt );
        }
        else if( RES_PARATR_BEGIN <= nWhich && RES_PARATR_END > nWhich )
            aCallMod.Add( pDfltTxtFmtColl );
        else if( RES_GRFATR_BEGIN <= nWhich && RES_GRFATR_END > nWhich )
            aCallMod.Add( pDfltGrfFmtColl );
        else if( RES_FRMATR_BEGIN <= nWhich && RES_FRMATR_END > nWhich )
        {
            aCallMod.Add( pDfltGrfFmtColl );
            aCallMod.Add( pDfltTxtFmtColl );
            aCallMod.Add( pDfltFrmFmt );
        }
        else if( RES_BOXATR_BEGIN <= nWhich && RES_BOXATR_END > nWhich )
            aCallMod.Add( pDfltFrmFmt );
        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    }

    if( aNew.Count() && aCallMod.GetDepends() )
    {
        if( DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoDefaultAttr( aOld ) );
        }

        const SfxPoolItem* pItem;
        if( aNew.GetItemState( RES_PARATR_TABSTOP, FALSE, &pItem ) &&
            ((SvxTabStopItem*)pItem)->Count() )
        {
            // TabStop-Aenderungen behandeln wir erstmal anders:
            // dann aender bei allen TabStop die dafault's auf den neuen Wert
            // !!! Achtung: hier wird immer auf dem PoolAttribut gearbeitet,
            //              damit nicht in allen Sets die gleiche Berechnung
            //              auf dem gleichen TabStop (gepoolt!) vorgenommen
            //              wird. Als Modify wird ein FmtChg verschickt.
            SwTwips nNewWidth = (*(SvxTabStopItem*)pItem)[ 0 ].GetTabPos(),
                    nOldWidth = ((SvxTabStopItem&)aOld.Get(RES_PARATR_TABSTOP))[ 0 ].GetTabPos();

            int bChg = FALSE;
            USHORT nMaxItems = GetAttrPool().GetItemCount( RES_PARATR_TABSTOP );
            for( USHORT n = 0; n < nMaxItems; ++n )
                if( 0 != (pItem = GetAttrPool().GetItem( RES_PARATR_TABSTOP, n ) ))
                    bChg |= lcl_SetNewDefTabStops( nOldWidth, nNewWidth,
                                                *(SvxTabStopItem*)pItem );

            aNew.ClearItem( RES_PARATR_TABSTOP );
            aOld.ClearItem( RES_PARATR_TABSTOP );
            if( bChg )
            {
                SwFmtChg aChgFmt( pDfltCharFmt );
                // dann sage mal den Frames bescheid
                aCallMod.Modify( &aChgFmt, &aChgFmt );
            }
        }
    }

    if( aNew.Count() && aCallMod.GetDepends() )
    {
        SwAttrSetChg aChgOld( aOld, aOld );
        SwAttrSetChg aChgNew( aNew, aNew );
        aCallMod.Modify( &aChgOld, &aChgNew );      // alle veraenderten werden verschickt
    }

    // und die default-Formate wieder beim Object austragen
    SwClient* pDep;
    while( 0 != ( pDep = (SwClient*)aCallMod.GetDepends()) )
        aCallMod.Remove( pDep );

    SetModified();
}

    // Erfrage das Default Attribut in diesem Dokument.
const SfxPoolItem& SwDoc::GetDefault( USHORT nFmtHint ) const
{
    return GetAttrPool().GetDefaultItem( nFmtHint );
}

/*
 * Loeschen der Formate
 */
void SwDoc::DelCharFmt(USHORT nFmt)
{
    pCharFmtTbl->DeleteAndDestroy(nFmt);
    SetModified();
}

void SwDoc::DelCharFmt( SwCharFmt *pFmt )
{
    USHORT nFmt = pCharFmtTbl->GetPos( pFmt );
    ASSERT( USHRT_MAX != nFmt, "Fmt not found," );
    DelCharFmt( nFmt );
}

void SwDoc::DelFrmFmt( SwFrmFmt *pFmt )
{
    if( pFmt->ISA( SwTableBoxFmt ) || pFmt->ISA( SwTableLineFmt ))
    {
        ASSERT( !this, "Format steht nicht mehr im DocArray, "
                       "kann per delete geloescht werden" );
        delete pFmt;
    }
    else
    {
        //Das Format muss in einem der beiden Arrays stehen, in welchem
        //werden wir schon merken.
        USHORT nPos;
        if ( USHRT_MAX != ( nPos = pFrmFmtTbl->GetPos( pFmt )) )
            pFrmFmtTbl->DeleteAndDestroy( nPos );
        else
        {
            nPos = GetSpzFrmFmts()->GetPos( pFmt );
            ASSERT( nPos != USHRT_MAX, "FrmFmt not found." );
            if( USHRT_MAX != nPos )
                GetSpzFrmFmts()->DeleteAndDestroy( nPos );
        }
    }
}

void SwDoc::DelTblFrmFmt( SwTableFmt *pFmt )
{
    USHORT nPos = pTblFrmFmtTbl->GetPos( pFmt );
    ASSERT( USHRT_MAX != nPos, "Fmt not found," );
    pTblFrmFmtTbl->DeleteAndDestroy( nPos );
}

/*
 * Erzeugen der Formate
 */
SwFlyFrmFmt *SwDoc::MakeFlyFrmFmt( const String &rFmtName,
                                    SwFrmFmt *pDerivedFrom )
{
    SwFlyFrmFmt *pFmt = new SwFlyFrmFmt( GetAttrPool(), rFmtName, pDerivedFrom );
    GetSpzFrmFmts()->Insert(pFmt, GetSpzFrmFmts()->Count());
    SetModified();
    return pFmt;
}

SwDrawFrmFmt *SwDoc::MakeDrawFrmFmt( const String &rFmtName,
                                     SwFrmFmt *pDerivedFrom )
{
    SwDrawFrmFmt *pFmt = new SwDrawFrmFmt( GetAttrPool(), rFmtName, pDerivedFrom);
    GetSpzFrmFmts()->Insert(pFmt,GetSpzFrmFmts()->Count());
    SetModified();
    return pFmt;
}


USHORT SwDoc::GetTblFrmFmtCount(BOOL bUsed) const
{
    USHORT nCount = pTblFrmFmtTbl->Count();
    if(bUsed)
    {
        SwAutoFmtGetDocNode aGetHt( &aNodes );
        for ( USHORT i = nCount; i; )
        {
            if((*pTblFrmFmtTbl)[--i]->GetInfo( aGetHt ))
                --nCount;
        }
    }
    return nCount;
}


SwFrmFmt& SwDoc::GetTblFrmFmt(USHORT nFmt, BOOL bUsed ) const
{
    USHORT nRemoved = 0;
    if(bUsed)
    {
        SwAutoFmtGetDocNode aGetHt( &aNodes );
        for ( USHORT i = 0; i <= nFmt; i++ )
        {
            while ( (*pTblFrmFmtTbl)[ i + nRemoved]->GetInfo( aGetHt ))
            {
                nRemoved++;
            }
        }
    }
    return *((*pTblFrmFmtTbl)[nRemoved + nFmt]);
}

SwTableFmt* SwDoc::MakeTblFrmFmt( const String &rFmtName,
                                    SwFrmFmt *pDerivedFrom )
{
    SwTableFmt* pFmt = new SwTableFmt( GetAttrPool(), rFmtName, pDerivedFrom );
    pTblFrmFmtTbl->Insert( pFmt, pTblFrmFmtTbl->Count() );
    SetModified();
    return pFmt;
}

SwFrmFmt *SwDoc::MakeFrmFmt(const String &rFmtName,
                            SwFrmFmt *pDerivedFrom)
{
    SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(), rFmtName, pDerivedFrom );
    pFrmFmtTbl->Insert( pFmt, pFrmFmtTbl->Count());
    SetModified();
    return pFmt;
}

SwCharFmt *SwDoc::MakeCharFmt( const String &rFmtName,
                                SwCharFmt *pDerivedFrom)
{
    SwCharFmt *pFmt = new SwCharFmt( GetAttrPool(), rFmtName, pDerivedFrom );
    pCharFmtTbl->Insert( pFmt, pCharFmtTbl->Count() );
    pFmt->SetAuto( FALSE );
    SetModified();
    return pFmt;
}


/*
 * Erzeugen der FormatCollections
 */
// TXT

SwTxtFmtColl* SwDoc::MakeTxtFmtColl( const String &rFmtName,
                                     SwTxtFmtColl *pDerivedFrom)
{
    SwTxtFmtColl *pFmtColl = new SwTxtFmtColl( GetAttrPool(), rFmtName,
                                                pDerivedFrom );
    pTxtFmtCollTbl->Insert(pFmtColl, pTxtFmtCollTbl->Count());
    pFmtColl->SetAuto( FALSE );
    SetModified();
    return pFmtColl;
}

//FEATURE::CONDCOLL
SwConditionTxtFmtColl* SwDoc::MakeCondTxtFmtColl( const String &rFmtName,
                                                SwTxtFmtColl *pDerivedFrom )
{
    SwConditionTxtFmtColl*pFmtColl = new SwConditionTxtFmtColl( GetAttrPool(),
                                                    rFmtName, pDerivedFrom );
    pTxtFmtCollTbl->Insert(pFmtColl, pTxtFmtCollTbl->Count());
    pFmtColl->SetAuto( FALSE );
    SetModified();
    return pFmtColl;
}
//FEATURE::CONDCOLL

// GRF

SwGrfFmtColl* SwDoc::MakeGrfFmtColl( const String &rFmtName,
                                     SwGrfFmtColl *pDerivedFrom )
{
    SwGrfFmtColl *pFmtColl = new SwGrfFmtColl( GetAttrPool(), rFmtName,
                                                pDerivedFrom );
    pGrfFmtCollTbl->Insert( pFmtColl, pGrfFmtCollTbl->Count() );
    pFmtColl->SetAuto( FALSE );
    SetModified();
    return pFmtColl;
}

void SwDoc::DelTxtFmtColl(USHORT nFmtColl)
{
    ASSERT( nFmtColl, "Remove fuer Coll 0." );

    // Wer hat die zu loeschende als Next
    SwTxtFmtColl *pDel = (*pTxtFmtCollTbl)[nFmtColl];
    if( pDfltTxtFmtColl == pDel )
        return;     // default nie loeschen !!
    // Die FmtColl austragen
    pTxtFmtCollTbl->Remove(nFmtColl);
    // Next korrigieren
    pTxtFmtCollTbl->ForEach( 1, pTxtFmtCollTbl->Count(),
                            &SetTxtFmtCollNext, pDel );
    delete pDel;
    SetModified();
}

void SwDoc::DelTxtFmtColl( SwTxtFmtColl *pColl )
{
    USHORT nFmt = pTxtFmtCollTbl->GetPos( pColl );
    ASSERT( USHRT_MAX != nFmt, "Collection not found," );
    DelTxtFmtColl( nFmt );
}

void SwDoc::DelGrfFmtColl(USHORT nFmtColl)
{
    ASSERT( nFmtColl, "Remove fuer Coll 0." );

    // Wer hat die zu loeschende als Next
    SwGrfFmtColl *pDel = (*pGrfFmtCollTbl)[nFmtColl];
    if( pDfltGrfFmtColl == pDel )
        return;     // default nie loeschen !!
    // Die FmtColl austragen
    pGrfFmtCollTbl->Remove(nFmtColl);
    delete pDel;
    SetModified();
}

void SwDoc::DelGrfFmtColl( SwGrfFmtColl *pColl )
{
    USHORT nFmt = pGrfFmtCollTbl->GetPos( pColl );
    ASSERT( USHRT_MAX != nFmt, "Collection not found," );
    DelGrfFmtColl( nFmt );
}

BOOL lcl_SetTxtFmtColl( const SwNodePtr& rpNode, void* pArgs )
{
    // ParaSetFmtColl * pPara = (ParaSetFmtColl*)pArgs;
    SwCntntNode* pCNd = (SwCntntNode*)rpNode->GetTxtNode();
    if( pCNd )
    {
        ParaRstFmt* pPara = (ParaRstFmt*)pArgs;
        if ( pPara->bReset )
            lcl_RstAttr( pCNd, pPara );

        // erst in die History aufnehmen, damit ggfs. alte Daten
        // gesichert werden koennen
        if( pPara->pHistory )
            pPara->pHistory->Add( pCNd->GetFmtColl(), pCNd->GetIndex(),
                                    ND_TEXTNODE );

        pCNd->ChgFmtColl( pPara->pFmtColl );
        pPara->nWhich++;
    }
    return TRUE;
}

BOOL SwDoc::SetTxtFmtColl(const SwPaM &rRg, SwTxtFmtColl *pFmt, BOOL bReset)
{
    SwDataChanged aTmp( rRg, 0 );
    const SwPosition *pStt = rRg.Start(), *pEnd = rRg.End();
    SwHistory* pHst = 0;
    BOOL bRet = TRUE;
    if( DoesUndo() )
    {
        ClearRedo();
        SwUndoFmtColl* pUndo = new SwUndoFmtColl( rRg, pFmt );
        pHst = pUndo->GetHistory();
        AppendUndo( pUndo );
    }

    if( rRg.HasMark() )
    {
        ParaRstFmt aPara( pStt, pEnd, pHst );
        aPara.pFmtColl = pFmt;
        aPara.bReset = bReset;
        GetNodes().ForEach( pStt->nNode.GetIndex(), pEnd->nNode.GetIndex()+1,
                            lcl_SetTxtFmtColl, &aPara );
        if( !aPara.nWhich )
            bRet = FALSE;           // keinen gueltigen Node gefunden
    }
    else
    {
        // ein enzelner Node:
        SwCntntNode* pCNd = rRg.GetPoint()->nNode.GetNode().GetCntntNode();
        if( pCNd )
        {
            if( bReset && pCNd->GetpSwAttrSet() )
            {
                ParaRstFmt aPara( pHst );
                aPara.pFmtColl = pFmt;
                lcl_RstAttr( pCNd, &aPara );
            }

            // erst in die History aufnehmen, damit ggfs. alte Daten
            // gesichert werden koennen
            if( pHst )
                pHst->Add( pCNd->GetFmtColl(), pCNd->GetIndex(), ND_TEXTNODE );
            pCNd->ChgFmtColl( pFmt );
        }
        else
            bRet = FALSE;
    }

    if( bRet )
        SetModified();
    return bRet;
}


// ---- Kopiere die Formate in sich selbst (SwDoc) ----------------------

SwFmt* SwDoc::CopyFmt( const SwFmt& rFmt,
                        const SvPtrarr& rFmtArr,
                        FNCopyFmt fnCopyFmt, const SwFmt& rDfltFmt )
{
    //  kein-Autoformat || default Format || Collection-Format
    // dann suche danach.
    if( !rFmt.IsAuto() || !rFmt.GetRegisteredIn() )
        for( USHORT n = 0; n < rFmtArr.Count(); n++ )
        {
            // ist die Vorlage schon im Doc vorhanden ??
            if( ((SwFmt*)rFmtArr[n])->GetName().Equals( rFmt.GetName() ))
                return (SwFmt*)rFmtArr[n];
        }

    // suche erstmal nach dem "Parent"
    SwFmt* pParent = (SwFmt*)&rDfltFmt;
    if( rFmt.DerivedFrom() && pParent != rFmt.DerivedFrom() )
        pParent = CopyFmt( *rFmt.DerivedFrom(), rFmtArr,
                                fnCopyFmt, rDfltFmt );

    // erzeuge das Format und kopiere die Attribute
    SwFmt* pNewFmt = (this->*fnCopyFmt)( rFmt.GetName(), pParent );
    pNewFmt->SetAuto( rFmt.IsAuto() );
    pNewFmt->CopyAttrs( rFmt, TRUE );           // kopiere Attribute

    pNewFmt->SetPoolFmtId( rFmt.GetPoolFmtId() );
    pNewFmt->SetPoolHelpId( rFmt.GetPoolHelpId() );

    // HelpFile-Id immer auf dflt setzen !!
    pNewFmt->SetPoolHlpFileId( UCHAR_MAX );

    return pNewFmt;
}

// ---- kopiere das Frame-Format --------
SwFrmFmt* SwDoc::CopyFrmFmt( const SwFrmFmt& rFmt )
{
    return (SwFrmFmt*)CopyFmt( rFmt, *GetFrmFmts(),
                                (FNCopyFmt)&SwDoc::MakeFrmFmt,
                                *GetDfltFrmFmt() );
}

// ---- kopiere das Char-Format --------
SwCharFmt* SwDoc::CopyCharFmt( const SwCharFmt& rFmt )
{
    return (SwCharFmt*)CopyFmt( rFmt, *GetCharFmts(),
                                (FNCopyFmt)&SwDoc::MakeCharFmt,
                                *GetDfltCharFmt() );
}


// --- Kopiere TextNodes ----

SwTxtFmtColl* SwDoc::CopyTxtColl( const SwTxtFmtColl& rColl )
{
    SwTxtFmtColl* pNewColl = FindTxtFmtCollByName( rColl.GetName() );
    if( pNewColl )
        return pNewColl;

    // suche erstmal nach dem "Parent"
    SwTxtFmtColl* pParent = pDfltTxtFmtColl;
    if( pParent != rColl.DerivedFrom() )
        pParent = CopyTxtColl( *(SwTxtFmtColl*)rColl.DerivedFrom() );


//FEATURE::CONDCOLL
    if( RES_CONDTXTFMTCOLL == rColl.Which() )
    {
        pNewColl = new SwConditionTxtFmtColl( GetAttrPool(), rColl.GetName(),
                                                pParent);
        pTxtFmtCollTbl->Insert( pNewColl, pTxtFmtCollTbl->Count() );
        pNewColl->SetAuto( FALSE );
        SetModified();

        // Kopiere noch die Bedingungen
        ((SwConditionTxtFmtColl*)pNewColl)->SetConditions(
                            ((SwConditionTxtFmtColl&)rColl).GetCondColls() );
    }
    else
//FEATURE::CONDCOLL
        pNewColl = MakeTxtFmtColl( rColl.GetName(), pParent );

    // kopiere jetzt noch die Auto-Formate oder kopiere die Attribute
    pNewColl->CopyAttrs( rColl, TRUE );

    // setze noch den Outline-Level
    if( NO_NUMBERING != rColl.GetOutlineLevel() )
        pNewColl->SetOutlineLevel( rColl.GetOutlineLevel() );

    pNewColl->SetPoolFmtId( rColl.GetPoolFmtId() );
    pNewColl->SetPoolHelpId( rColl.GetPoolHelpId() );

    // HelpFile-Id immer auf dflt setzen !!
    pNewColl->SetPoolHlpFileId( UCHAR_MAX );

    if( &rColl.GetNextTxtFmtColl() != &rColl )
        pNewColl->SetNextTxtFmtColl( *CopyTxtColl( rColl.GetNextTxtFmtColl() ));

    // ggfs. die NumRule erzeugen
    if( this != rColl.GetDoc() )
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pNewColl->GetItemState( RES_PARATR_NUMRULE,
            FALSE, &pItem ))
        {
            const SwNumRule* pRule;
            const String& rName = ((SwNumRuleItem*)pItem)->GetValue();
            if( rName.Len() &&
                0 != ( pRule = rColl.GetDoc()->FindNumRulePtr( rName )) &&
                !pRule->IsAutoRule() )
            {
                SwNumRule* pDestRule = FindNumRulePtr( rName );
                if( pDestRule )
                    pDestRule->SetInvalidRule( TRUE );
                else
                    MakeNumRule( rName, pRule );
            }
        }
    }
    return pNewColl;
}

// --- Kopiere GrafikNodes ----

SwGrfFmtColl* SwDoc::CopyGrfColl( const SwGrfFmtColl& rColl )
{
    SwGrfFmtColl* pNewColl = FindGrfFmtCollByName( rColl.GetName() );
    if( pNewColl )
        return pNewColl;

    // suche erstmal nach dem "Parent"
    SwGrfFmtColl* pParent = pDfltGrfFmtColl;
    if( pParent != rColl.DerivedFrom() )
        pParent = CopyGrfColl( *(SwGrfFmtColl*)rColl.DerivedFrom() );

    // falls nicht, so kopiere sie
    pNewColl = MakeGrfFmtColl( rColl.GetName(), pParent );

    // noch die Attribute kopieren
    pNewColl->CopyAttrs( rColl );

    pNewColl->SetPoolFmtId( rColl.GetPoolFmtId() );
    pNewColl->SetPoolHelpId( rColl.GetPoolHelpId() );

    // HelpFile-Id immer auf dflt setzen !!
    pNewColl->SetPoolHlpFileId( UCHAR_MAX );

    return pNewColl;
}

SwPageDesc* lcl_FindPageDesc( const SwPageDescs& rArr, const String& rName )
{
    for( USHORT n = rArr.Count(); n; )
    {
        SwPageDesc* pDesc = rArr[ --n ];
        if( pDesc->GetName() == rName )
            return pDesc;
    }
    return 0;
}

void SwDoc::CopyFmtArr( const SvPtrarr& rSourceArr,
                        SvPtrarr& rDestArr,
                        FNCopyFmt fnCopyFmt,
                        SwFmt& rDfltFmt )
{
    USHORT nSrc;
    SwFmt* pSrc, *pDest;

    // 1. Schritt alle Formate anlegen (das 0. ueberspringen - Default!)
    for( nSrc = rSourceArr.Count(); nSrc > 1; )
    {
        pSrc = (SwFmt*)rSourceArr[ --nSrc ];
        if( pSrc->IsDefault() || pSrc->IsAuto() )
            continue;

        if( 0 == FindFmtByName( rDestArr, pSrc->GetName() ) )
        {
            if( RES_CONDTXTFMTCOLL == pSrc->Which() )
                MakeCondTxtFmtColl( pSrc->GetName(), (SwTxtFmtColl*)&rDfltFmt );
            else
                (this->*fnCopyFmt)( pSrc->GetName(), &rDfltFmt );
        }
    }

    // 2. Schritt alle Attribute kopieren, richtige Parents setzen
    for( nSrc = rSourceArr.Count(); nSrc > 1; )
    {
        pSrc = (SwFmt*)rSourceArr[ --nSrc ];
        if( pSrc->IsDefault() || pSrc->IsAuto() )
            continue;

        pDest = FindFmtByName( rDestArr, pSrc->GetName() );
        pDest->SetAuto( FALSE );
//      pDest->ResetAllAttr();
//      pDest->CopyAttrs( *pSrc, TRUE );            // kopiere Attribute
//JP 19.02.96: ist so wohl optimaler - loest ggfs. kein Modify aus!
        pDest->DelDiffs( *pSrc );
        pDest->SetAttr( pSrc->GetAttrSet() );       // kopiere Attribute

        //JP 18.08.98: Bug 55115 - PageDescAttribute in diesem Fall doch
        //              kopieren
        const SfxPoolItem* pItem;
        if( &GetAttrPool() != pSrc->GetAttrSet().GetPool() &&
            SFX_ITEM_SET == pSrc->GetAttrSet().GetItemState(
            RES_PAGEDESC, FALSE, &pItem ) &&
            ((SwFmtPageDesc*)pItem)->GetPageDesc() )
        {
            SwFmtPageDesc aDesc( *(SwFmtPageDesc*)pItem );
            const String& rNm = aDesc.GetPageDesc()->GetName();
            SwPageDesc* pDesc = ::lcl_FindPageDesc( aPageDescs, rNm );
            if( !pDesc )
                pDesc = aPageDescs[ MakePageDesc( rNm ) ];
            pDesc->Add( &aDesc );
            pDest->SetAttr( aDesc );
        }

        pDest->SetPoolFmtId( pSrc->GetPoolFmtId() );
        pDest->SetPoolHelpId( pSrc->GetPoolHelpId() );

        // HelpFile-Id immer auf dflt setzen !!
        pDest->SetPoolHlpFileId( UCHAR_MAX );

        if( pSrc->DerivedFrom() )
            pDest->SetDerivedFrom( FindFmtByName( rDestArr,
                                        pSrc->DerivedFrom()->GetName() ) );
        if( RES_TXTFMTCOLL == pSrc->Which() ||
            RES_CONDTXTFMTCOLL == pSrc->Which() )
        {
            SwTxtFmtColl* pSrcColl = (SwTxtFmtColl*)pSrc,
                        * pDstColl = (SwTxtFmtColl*)pDest;
            if( &pSrcColl->GetNextTxtFmtColl() != pSrcColl )
                pDstColl->SetNextTxtFmtColl( *(SwTxtFmtColl*)FindFmtByName(
                    rDestArr, pSrcColl->GetNextTxtFmtColl().GetName() ) );

            // setze noch den Outline-Level
            if( NO_NUMBERING != pSrcColl->GetOutlineLevel() )
                pDstColl->SetOutlineLevel( pSrcColl->GetOutlineLevel() );

//FEATURE::CONDCOLL
            if( RES_CONDTXTFMTCOLL == pSrc->Which() )
                // Kopiere noch die Bedingungen
                // aber erst die alten loeschen!
                ((SwConditionTxtFmtColl*)pDstColl)->SetConditions(
                            ((SwConditionTxtFmtColl*)pSrc)->GetCondColls() );
//FEATURE::CONDCOLL
        }
    }
}

void SwDoc::_CopyPageDescHeaderFooter( BOOL bCpyHeader,
                                const SwFrmFmt& rSrcFmt, SwFrmFmt& rDestFmt )
{
    // jetzt noch Header-/Footer-Attribute richtig behandeln
    // Contenten Nodes Dokumentuebergreifend kopieren!
    USHORT nAttr = bCpyHeader ? RES_HEADER : RES_FOOTER;
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET != rSrcFmt.GetAttrSet().GetItemState( nAttr, FALSE, &pItem ))
        return ;

    // Im Header steht noch der Verweis auf das Format aus dem
    // anderen Document!!
    SfxPoolItem* pNewItem = pItem->Clone();

    SwFrmFmt* pOldFmt;
    if( bCpyHeader )
         pOldFmt = ((SwFmtHeader*)pNewItem)->GetHeaderFmt();
    else
         pOldFmt = ((SwFmtFooter*)pNewItem)->GetFooterFmt();

    if( pOldFmt )
    {
        SwFrmFmt* pNewFmt = new SwFrmFmt( GetAttrPool(), "CpyDesc",
                                            GetDfltFrmFmt() );
        pNewFmt->CopyAttrs( *pOldFmt, TRUE );

        if( SFX_ITEM_SET == pNewFmt->GetAttrSet().GetItemState(
            RES_CNTNT, FALSE, &pItem ))
        {
            SwFmtCntnt* pCntnt = (SwFmtCntnt*)pItem;
            if( pCntnt->GetCntntIdx() )
            {
                SwNodeIndex aTmpIdx( GetNodes().GetEndOfAutotext() );
                const SwNodes& rSrcNds = rSrcFmt.GetDoc()->GetNodes();
                SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmpIdx,
                                                bCpyHeader
                                                    ? SwHeaderStartNode
                                                    : SwFooterStartNode );
                const SwNode& rCSttNd = pCntnt->GetCntntIdx()->GetNode();
                SwNodeRange aRg( rCSttNd, 0, *rCSttNd.EndOfSectionNode() );
                aTmpIdx = *pSttNd->EndOfSectionNode();
                rSrcNds._Copy( aRg, aTmpIdx );
                aTmpIdx = *pSttNd;
                rSrcFmt.GetDoc()->_CopyFlyInFly( aRg, aTmpIdx );
                pNewFmt->SetAttr( SwFmtCntnt( pSttNd ));
            }
            else
                pNewFmt->ResetAttr( RES_CNTNT );
        }
        if( bCpyHeader )
            pNewFmt->Add( (SwFmtHeader*)pNewItem );
        else
            pNewFmt->Add( (SwFmtFooter*)pNewItem );
        rDestFmt.SetAttr( *pNewItem );
    }
    delete pNewItem;
}

void SwDoc::CopyPageDesc( const SwPageDesc& rSrcDesc, SwPageDesc& rDstDesc,
                            BOOL bCopyPoolIds )
{
    FASTBOOL bNotifyLayout = FALSE;

    rDstDesc.SetLandscape( rSrcDesc.GetLandscape() );
    rDstDesc.SetNumType( rSrcDesc.GetNumType() );
    if( rDstDesc.ReadUseOn() != rSrcDesc.ReadUseOn() )
    {
        rDstDesc.WriteUseOn( rSrcDesc.ReadUseOn() );
        bNotifyLayout = TRUE;
    }

    if( bCopyPoolIds )
    {
        rDstDesc.SetPoolFmtId( rSrcDesc.GetPoolFmtId() );
        rDstDesc.SetPoolHelpId( rSrcDesc.GetPoolHelpId() );
        // HelpFile-Id immer auf dflt setzen !!
        rDstDesc.SetPoolHlpFileId( UCHAR_MAX );
    }

    if( rSrcDesc.GetFollow() != &rSrcDesc )
    {
        SwPageDesc* pFollow = ::lcl_FindPageDesc( aPageDescs,
                                    rSrcDesc.GetFollow()->GetName() );
        if( !pFollow )
        {
            // dann mal kopieren
            USHORT nPos = MakePageDesc( rSrcDesc.GetFollow()->GetName() );
            pFollow = aPageDescs[ nPos ];
            CopyPageDesc( *rSrcDesc.GetFollow(), *pFollow );
        }
        rDstDesc.SetFollow( pFollow );
        bNotifyLayout = TRUE;
    }

    // die Header/Footer-Attribute werden gesondert kopiert, die Content-
    // Sections muessen vollstaendig mitgenommen werden!
    {
        SfxItemSet aAttrSet( rSrcDesc.GetMaster().GetAttrSet() );
        aAttrSet.ClearItem( RES_HEADER );
        aAttrSet.ClearItem( RES_FOOTER );

        rDstDesc.GetMaster().DelDiffs( aAttrSet );
        rDstDesc.GetMaster().SetAttr( aAttrSet );

        aAttrSet.ClearItem();
        aAttrSet.Put( rSrcDesc.GetLeft().GetAttrSet() );
        aAttrSet.ClearItem( RES_HEADER );
        aAttrSet.ClearItem( RES_FOOTER );

        rDstDesc.GetLeft().DelDiffs( aAttrSet );
        rDstDesc.GetLeft().SetAttr( aAttrSet );
    }

    CopyHeader( rSrcDesc.GetMaster(), rDstDesc.GetMaster() );
    CopyFooter( rSrcDesc.GetMaster(), rDstDesc.GetMaster() );
    if( !rDstDesc.IsHeaderShared() )
        CopyHeader( rSrcDesc.GetLeft(), rDstDesc.GetLeft() );
    else
        rDstDesc.GetLeft().SetAttr( rDstDesc.GetMaster().GetHeader() );

    if( !rDstDesc.IsFooterShared() )
        CopyFooter( rSrcDesc.GetLeft(), rDstDesc.GetLeft() );
    else
        rDstDesc.GetLeft().SetAttr( rDstDesc.GetMaster().GetFooter() );

    if( bNotifyLayout && GetRootFrm() )
        //Layot benachrichtigen!
        GetRootFrm()->CheckPageDescs( (SwPageFrm*)GetRootFrm()->Lower() );

    //Wenn sich FussnotenInfo veraendert, so werden die Seiten
    //angetriggert.
    if( !(rDstDesc.GetFtnInfo() == rSrcDesc.GetFtnInfo()) )
    {
        rDstDesc.SetFtnInfo( rSrcDesc.GetFtnInfo() );
        SwMsgPoolItem  aInfo( RES_PAGEDESC_FTNINFO );
        {
            SwClientIter aIter( rDstDesc.GetMaster() );
            for( SwClient* pLast = aIter.First(TYPE(SwFrm)); pLast;
                    pLast = aIter.Next() )
                pLast->Modify( &aInfo, 0 );
        }
        {
            SwClientIter aIter( rDstDesc.GetLeft() );
            for( SwClient* pLast = aIter.First(TYPE(SwFrm)); pLast;
                    pLast = aIter.Next() )
                pLast->Modify( &aInfo, 0 );
        }
    }
}

void SwDoc::ReplaceStyles( SwDoc& rSource )
{
    BOOL bIsUndo = DoesUndo();
    DoUndo( FALSE );

    CopyFmtArr( *rSource.pCharFmtTbl, *pCharFmtTbl,
                (FNCopyFmt)&SwDoc::MakeCharFmt, *pDfltCharFmt );
    CopyFmtArr( *rSource.pFrmFmtTbl, *pFrmFmtTbl,
                (FNCopyFmt)&SwDoc::MakeFrmFmt, *pDfltFrmFmt );
    CopyFmtArr( *rSource.pTxtFmtCollTbl, *pTxtFmtCollTbl,
                (FNCopyFmt)&SwDoc::MakeTxtFmtColl, *pDfltTxtFmtColl );

    // und jetzt noch die Seiten-Vorlagen
    USHORT nCnt = rSource.aPageDescs.Count();
    if( nCnt )
    {
        // ein anderes Doc -> Numberformatter muessen gemergt werden
        SwTblNumFmtMerge aTNFM( rSource, *this );

        // 1. Schritt alle Formate anlegen (das 0. ueberspringen - Default!)
        while( nCnt )
        {
            SwPageDesc *pSrc = rSource.aPageDescs[ --nCnt ];
            if( 0 == ::lcl_FindPageDesc( aPageDescs, pSrc->GetName() ) )
                MakePageDesc( pSrc->GetName() );
        }

        // 2. Schritt alle Attribute kopieren, richtige Parents setzen
        for( nCnt = rSource.aPageDescs.Count(); nCnt; )
        {
            SwPageDesc *pSrc = rSource.aPageDescs[ --nCnt ];
            CopyPageDesc( *pSrc, *::lcl_FindPageDesc( aPageDescs, pSrc->GetName() ));
        }
    }

    //JP 08.04.99: und dann sind da noch die Numerierungs-Vorlagen
    nCnt = rSource.GetNumRuleTbl().Count();
    if( nCnt )
    {
        const SwNumRuleTbl& rArr = rSource.GetNumRuleTbl();
        for( USHORT n = 0; n < nCnt; ++n )
        {
            const SwNumRule& rR = *rArr[ n ];
            if( !rR.IsAutoRule() )
            {
                SwNumRule* pNew = FindNumRulePtr( rR.GetName());
                if( pNew )
                    pNew->CopyNumRule( this, rR );
                else
                    MakeNumRule( rR.GetName(), &rR );
            }
        }
    }

    if( bIsUndo )
    {
        // es wurde am Nodes-Array gedreht!
        ClearRedo();
        DelAllUndoObj();
    }

    SetModified();
    DoUndo( bIsUndo );
}

SwFmt* SwDoc::FindFmtByName( const SvPtrarr& rFmtArr,
                                    const String& rName ) const
{
    SwFmt* pFnd = 0;
    for( USHORT n = 0; n < rFmtArr.Count(); n++ )
    {
        // ist die Vorlage schon im Doc vorhanden ??
        if( ((SwFmt*)rFmtArr[n])->GetName() == rName )
        {
            pFnd = (SwFmt*)rFmtArr[n];
            break;
        }
    }
    return pFnd;
}

void SwDoc::MoveLeftMargin( const SwPaM& rPam, BOOL bRight, BOOL bModulus )
{
    SwHistory* pHistory = 0;
    if( DoesUndo() )
    {
        ClearRedo();
        SwUndoMoveLeftMargin* pUndo = new SwUndoMoveLeftMargin( rPam, bRight,
                                                                bModulus );
        pHistory = pUndo->GetHistory();
        AppendUndo( pUndo );
    }

    const SvxTabStopItem& rTabItem = (SvxTabStopItem&)GetDefault( RES_PARATR_TABSTOP );
    USHORT nDefDist = rTabItem.Count() ? rTabItem[0].GetTabPos() : 1134;
    const SwPosition &rStt = *rPam.Start(), &rEnd = *rPam.End();
    SwNodeIndex aIdx( rStt.nNode );
    while( aIdx <= rEnd.nNode )
    {
        SwTxtNode* pTNd = aIdx.GetNode().GetTxtNode();
        if( pTNd )
        {
            SvxLRSpaceItem aLS( (SvxLRSpaceItem&)pTNd->SwCntntNode::GetAttr(
                                                        RES_LR_SPACE ) );
            USHORT nNext = aLS.GetTxtLeft();
            if( bModulus )
                nNext = ( nNext / nDefDist ) * nDefDist;

            if( bRight )
                nNext += nDefDist;
            else if( nNext )
            {
                if( !bModulus && nDefDist > nNext )
                    nNext = 0;
                else
                    nNext -= nDefDist;
            }
            aLS.SetTxtLeft( nNext );

            SwRegHistory aRegH( pTNd, *pTNd, pHistory );
            pTNd->SwCntntNode::SetAttr( aLS );
        }
        aIdx++;
    }
    SetModified();
}

BOOL SwDoc::DontExpandFmt( const SwPosition& rPos, BOOL bFlag )
{
    BOOL bRet = FALSE;
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( pTxtNd )
    {
        bRet = pTxtNd->DontExpandFmt( rPos.nContent, bFlag );
        if( bRet && DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoDontExpandFmt( rPos ));
        }
    }
    return bRet;
}

SwTableBoxFmt* SwDoc::MakeTableBoxFmt()
{
    SwTableBoxFmt* pFmt = new SwTableBoxFmt( GetAttrPool(), aEmptyStr,
                                                pDfltFrmFmt );
    SetModified();
    return pFmt;
}

SwTableLineFmt* SwDoc::MakeTableLineFmt()
{
    SwTableLineFmt* pFmt = new SwTableLineFmt( GetAttrPool(), aEmptyStr,
                                                pDfltFrmFmt );
    SetModified();
    return pFmt;
}

void SwDoc::_CreateNumberFormatter()
{
    ASSERT( !pNumberFormatter, "ist doch schon vorhanden" );


    LanguageType eLang = LANGUAGE_SYSTEM; //System::GetLanguage();
/*              ((const SvxLanguageItem&)GetAttrPool().
                    GetDefaultItem( RES_CHRATR_LANGUAGE )).GetLanguage();
*/
    Reference< XMultiServiceFactory > xMSF = ::utl::getProcessServiceFactory();
    pNumberFormatter = new SvNumberFormatter( xMSF, eLang );
    pNumberFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_FORMAT_INTL );
    pNumberFormatter->SetYear2000(SFX_APP()->GetMiscConfig()->GetYear2000());

}

SwTblNumFmtMerge::SwTblNumFmtMerge( const SwDoc& rSrc, SwDoc& rDest )
    : pNFmt( 0 )
{
    // ein anderes Doc -> Numberformatter muessen gemergt werden
    SvNumberFormatter* pN;
    if( &rSrc != &rDest && 0 != ( pN = ((SwDoc&)rSrc).GetNumberFormatter( FALSE ) ))
        ( pNFmt = rDest.GetNumberFormatter( TRUE ))->MergeFormatter( *pN );

    if( &rSrc != &rDest )
        ((SwGetRefFieldType*)rSrc.GetSysFldType( RES_GETREFFLD ))->
            MergeWithOtherDoc( rDest );
}

SwTblNumFmtMerge::~SwTblNumFmtMerge()
{
    if( pNFmt )
        pNFmt->ClearMergeTable();
}


void SwDoc::SetTxtFmtCollByAutoFmt( const SwPosition& rPos, USHORT nPoolId,
                                    const SfxItemSet* pSet )
{
    SwPaM aPam( rPos );
    SwTxtNode* pTNd = rPos.nNode.GetNode().GetTxtNode();

    if( bIsAutoFmtRedline && pTNd )
    {
        // dann das Redline Object anlegen
        const SwTxtFmtColl& rColl = *pTNd->GetTxtColl();
        SwRedline* pRedl = new SwRedline( REDLINE_FMTCOLL, aPam );
        pRedl->SetMark();

        // interressant sind nur die Items, die vom Set NICHT wieder
        // in den Node gesetzt werden. Also muss man die Differenz nehmen
        SwRedlineExtraData_FmtColl aExtraData( rColl.GetName(),
                                                rColl.GetPoolFmtId() );
        if( pSet && pTNd->GetpSwAttrSet() )
        {
            SfxItemSet aTmp( *pTNd->GetpSwAttrSet() );
            aTmp.Differentiate( *pSet );
            // das Adjust Item behalten wir extra
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pTNd->GetpSwAttrSet()->GetItemState(
                    RES_PARATR_ADJUST, FALSE, &pItem ))
                aTmp.Put( *pItem );
            aExtraData.SetItemSet( aTmp );
        }
        pRedl->SetExtraData( &aExtraData );

// !!!!!!!!! Undo fehlt noch !!!!!!!!!!!!!!!!!!
        AppendRedline( pRedl );
    }

    SetTxtFmtColl( aPam, GetTxtCollFromPool( nPoolId ) );

    if( pSet && pTNd && pSet->Count() )
    {
        aPam.SetMark();
        aPam.GetMark()->nContent.Assign( pTNd, pTNd->GetTxt().Len() );
        Insert( aPam, *pSet );
    }
}

void SwDoc::SetFmtItemByAutoFmt( const SwPaM& rPam, const SfxPoolItem& rItem )
{
    SwTxtNode* pTNd = rPam.GetPoint()->nNode.GetNode().GetTxtNode();

    SwRedlineMode eOld = GetRedlineMode();

    if( bIsAutoFmtRedline && pTNd )
    {
        // dann das Redline Object anlegen
        SwRedline* pRedl = new SwRedline( REDLINE_FORMAT, rPam );
        if( !pRedl->HasMark() )
            pRedl->SetMark();

        // interressant sind nur die Items, die vom Set NICHT wieder
        // in den Node gesetzt werden. Also muss man die Differenz nehmen
        SwRedlineExtraData_Format aExtraData( rItem.Which() );

/*
        if( pSet && pTNd->GetpSwAttrSet() )
        {
            SfxItemSet aTmp( *pTNd->GetpSwAttrSet() );
            aTmp.Differentiate( *pSet );
            // das Adjust Item behalten wir extra
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pTNd->GetpSwAttrSet()->GetItemState(
                    RES_PARATR_ADJUST, FALSE, &pItem ))
                aTmp.Put( *pItem );
            aExtraData.SetItemSet( aTmp );
        }
*/
        pRedl->SetExtraData( &aExtraData );

// !!!!!!!!! Undo fehlt noch !!!!!!!!!!!!!!!!!!
        AppendRedline( pRedl );

        SetRedlineMode_intern( eOld | REDLINE_IGNORE );
    }

    Insert( rPam, rItem, SETATTR_DONTEXPAND );
    SetRedlineMode_intern( eOld );
}

