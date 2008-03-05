/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docfmt.cxx,v $
 *
 *  $Revision: 1.49 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:53:20 $
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


#define _ZFORLIST_DECLARE_TABLE
#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_USHORTS

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_MISCCFG_HXX
#include <svtools/misccfg.hxx>
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
#ifndef _ZFORLIST_HXX //autogen
#define _ZFORLIST_DECLARE_TABLE
#include <svtools/zforlist.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HDL
#include <com/sun/star/i18n/WordType.hdl>
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
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#include <fmtautofmt.hxx>
#include <istyleaccess.hxx>
#include <SwUndoFmt.hxx>

using namespace ::com::sun::star::i18n;
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
    bool bReset;
    // --> OD 2007-11-06 #i62575#
    bool bResetListAttrs;
    // <--
    bool bResetAll;
    bool bInclRefToxMark;

    ParaRstFmt( const SwPosition* pStt, const SwPosition* pEnd,
                SwHistory* pHst, USHORT nWhch = 0, const SfxItemSet* pSet = 0 )
        : pFmtColl(0),
          pHistory(pHst),
          pSttNd(pStt),
          pEndNd(pEnd),
          pDelSet(pSet),
          nWhich(nWhch),
          // --> OD 2007-11-06 #i62675#
          bReset( false ),
          bResetListAttrs( false ),
          // <--
          bResetAll( true ),
          bInclRefToxMark( false )
    {}

    ParaRstFmt( SwHistory* pHst )
        : pFmtColl(0),
          pHistory(pHst),
          pSttNd(0),
          pEndNd(0),
          pDelSet(0),
          nWhich(0),
          // --> OD 2007-11-06 #i62675#
          bReset( false ),
          bResetListAttrs( false ),
          // <--
          bResetAll( true ),
          bInclRefToxMark( false )
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
    if( pNode && pNode->HasSwAttrSet() )
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
        const SfxItemSet* pSet = pNode->GetpSwAttrSet();

        USHORT __READONLY_DATA aSavIds[ 3 ] = { RES_PAGEDESC, RES_BREAK,
                                                RES_PARATR_NUMRULE };

        // --> OD 2007-03-02 #i75027#
        // numbering attributes has to be restored, if numbering rule is restored.
        bool bRestoreNumAttrs( false );
        int nToBeRestoredNumLevel( -1 );
        bool bToBeRestoredIsRestart( false );
        SwNodeNum::tSwNumTreeNumber nToBeRestoredRestartVal( 0 );
        // <--

        const SfxPoolItem* pItem;
        std::vector<USHORT> aClearWhichIds;
        for( USHORT n = 0; n < 3; ++n )
        {
            if( SFX_ITEM_SET == pSet->GetItemState( aSavIds[ n ], FALSE, &pItem ))
            {
                BOOL bSave = FALSE;
                switch( aSavIds[ n ] )
                {
                    case RES_PAGEDESC:
                        bSave = 0 != ((SwFmtPageDesc*)pItem)->GetPageDesc();
                    break;
                    case RES_BREAK:
                        bSave = SVX_BREAK_NONE != ((SvxFmtBreakItem*)pItem)->GetBreak();
                    break;
                    case RES_PARATR_NUMRULE:
                    {
                        bSave = 0 != ((SwNumRuleItem*)pItem)->GetValue().Len();
                        // --> OD 2007-03-02 #i75027#
                        SwTxtNode* pTxtNode( dynamic_cast<SwTxtNode*>(pNode) );
                        if ( pTxtNode )
                        {
                            bRestoreNumAttrs = true;
                            nToBeRestoredNumLevel = pTxtNode->GetLevel();
                            bToBeRestoredIsRestart = pTxtNode->IsRestart();
                            nToBeRestoredRestartVal = pTxtNode->GetStart();
                        }
                        // <--
                    }
                    break;
                }
                if( bSave )
                {
                    aSet.Put( *pItem );
                    aClearWhichIds.push_back( aSavIds[n] );
                }
            }
        }
        pNode->ClearItemsFromAttrSet( aClearWhichIds );

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
            // --> OD 2007-03-02 #i75027#
            if ( bRestoreNumAttrs && dynamic_cast<SwTxtNode*>(pNode) )
            {
                SwTxtNode* pTxtNode( dynamic_cast<SwTxtNode*>(pNode) );
                pTxtNode->SetLevel( nToBeRestoredNumLevel );
                pTxtNode->SetRestart( bToBeRestoredIsRestart );
                pTxtNode->SetStart( nToBeRestoredRestartVal );
            }
            // <--

            if( !bLocked )
                pNode->UnlockModify();
        }
    }
    return TRUE;
}

void SwDoc::RstTxtAttrs(const SwPaM &rRg, BOOL bInclRefToxMark )
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
    aPara.bInclRefToxMark = ( bInclRefToxMark == TRUE );
    GetNodes().ForEach( pStt->nNode.GetIndex(), pEnd->nNode.GetIndex()+1,
                        lcl_RstTxtAttr, &aPara );
    SetModified();
}

void SwDoc::ResetAttrs( const SwPaM &rRg, BOOL bTxtAttr,
                        const SvUShortsSort* pAttrs )
{
    SwPaM* pPam = (SwPaM*)&rRg;
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
        else
        {
            Boundary aBndry;
            if( pBreakIt->xBreak.is() )
                aBndry = pBreakIt->xBreak->getWordBoundary(
                            pTxtNd->GetTxt(), nPtPos,
                            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                            WordType::ANY_WORD /*ANYWORD_IGNOREWHITESPACES*/,
                            TRUE );

            if( aBndry.startPos < nPtPos && nPtPos < aBndry.endPos )
            {
                nMkPos = (xub_StrLen)aBndry.startPos;
                nPtPos = (xub_StrLen)aBndry.endPos;
            }
            else
            {
                nPtPos = nMkPos = rSt.GetIndex();
                if( bTxtAttr )
                    pTxtNd->DontExpandFmt( rSt, TRUE );
            }
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
            static_cast<USHORT>(bTxtAttr ? RES_CONDTXTFMTCOLL : RES_TXTFMTCOLL ));
        if( pAttrs && pAttrs->Count() )
            pUndo->SetAttrs( *pAttrs );
        pHst = pUndo->GetHistory();
        AppendUndo( pUndo );
    }

    const SwPosition *pStt = pPam->Start(), *pEnd = pPam->End();
    ParaRstFmt aPara( pStt, pEnd, pHst );

    USHORT __FAR_DATA aResetableSetRange[] = {
        RES_FRMATR_BEGIN, RES_FRMATR_END-1,
        RES_CHRATR_BEGIN, RES_CHRATR_END-1,
        RES_PARATR_BEGIN, RES_PARATR_END-1,
        RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
        RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
        RES_TXTATR_CJK_RUBY, RES_TXTATR_UNKNOWN_CONTAINER,
        RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
        0
    };

    SfxItemSet aDelSet( GetAttrPool(), aResetableSetRange );
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
        if( pTNd && pTNd->HasSwAttrSet() && pTNd->GetpSwAttrSet()->Count() )
        {
            SfxItemIter aIter( *pTNd->GetpSwAttrSet() );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            SfxItemSet aCharSet( GetAttrPool(), RES_CHRATR_BEGIN, RES_CHRATR_END );

            while( TRUE )
            {
                if( IsInRange( aCharFmtSetRange, pItem->Which() ))
                {

                    if( !pTNd->pSwpHints )
                        pTNd->pSwpHints = new SwpHints;

                    aCharSet.Put( *pItem );

                    if( pHst )
                    {
                        SwRegHistory aRegH( pTNd, *pTNd, pHst );
                        pTNd->ResetAttr( pItem->Which() );
                    }
                    else
                        pTNd->ResetAttr( pItem->Which() );
                }
                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }

            if ( aCharSet.Count() )
            {
                if ( pHst )
                {
                    SwRegHistory( pTNd, aCharSet, 0, pTNd->GetTxt().Len(), nsSetAttrMode::SETATTR_NOFORMATATTR, pHst );
                }
                else
                {
                    SwTxtAttr* pNew = pTNd->MakeTxtAttr( aCharSet, 0, pTNd->GetTxt().Len() );
                    pTNd->Insert( pNew );
                }
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
        if( pTNd && pTNd->HasSwAttrSet() && pTNd->GetpSwAttrSet()->Count() )
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
        aPara.bResetAll = false ;
        ::lcl_RstAttr( &pStt->nNode.GetNode(), &aPara );
        aPara.bResetAll = true ;
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

#define DELETECHARSETS if ( bDelete ) { delete pCharSet; delete pOtherSet; }

// Einfuegen der Hints nach Inhaltsformen;
// wird in SwDoc::Insert(..., SwFmtHint &rHt) benutzt

BOOL InsAttr( SwDoc *pDoc, const SwPaM &rRg, const SfxItemSet& rChgSet,
                USHORT nFlags, SwUndoAttr* pUndo )
{
    // teil die Sets auf (fuer Selektion in Nodes)
    const SfxItemSet* pCharSet = 0;
    const SfxItemSet* pOtherSet = 0;
    bool bDelete = false;
    bool bCharAttr = false;
    bool bOtherAttr = false;

    // Check, if we can work with rChgSet or if we have to create additional SfxItemSets
    if ( 1 == rChgSet.Count() )
    {
        SfxItemIter aIter( rChgSet );
        const SfxPoolItem* pItem = aIter.FirstItem();
        const USHORT nWhich = pItem->Which();

        if ( RES_CHRATR_BEGIN <= nWhich && nWhich < RES_CHRATR_END ||
             RES_TXTATR_CHARFMT == nWhich ||
             RES_TXTATR_INETFMT == nWhich ||
             RES_TXTATR_AUTOFMT == nWhich ||
             RES_UNKNOWNATR_BEGIN <= nWhich && nWhich < RES_UNKNOWNATR_END )
        {
            pCharSet  = &rChgSet;
            bCharAttr = true;
        }

        if ( RES_PARATR_BEGIN <= nWhich && nWhich < RES_PARATR_END ||
             RES_FRMATR_BEGIN <= nWhich && nWhich < RES_FRMATR_END ||
             RES_GRFATR_BEGIN <= nWhich && nWhich < RES_GRFATR_END )
        {
            pOtherSet = &rChgSet;
            bOtherAttr = true;
        }
    }

    // Build new itemset if either
    // - rChgSet.Count() > 1 or
    // - The attribute in rChgSet does not belong to one of the above categories
    if ( !bCharAttr && !bOtherAttr )
    {
        SfxItemSet* pTmpCharItemSet = new SfxItemSet( pDoc->GetAttrPool(),
                                   RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                                   RES_TXTATR_AUTOFMT, RES_TXTATR_AUTOFMT,
                                   RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
                                   RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
                                   RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
                                   0 );

        SfxItemSet* pTmpOtherItemSet = new SfxItemSet( pDoc->GetAttrPool(),
                                    RES_PARATR_BEGIN, RES_PARATR_END-1,
                                    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
                                    RES_GRFATR_BEGIN, RES_GRFATR_END-1,
                                    0 );

        pTmpCharItemSet->Put( rChgSet );
        pTmpOtherItemSet->Put( rChgSet );

        pCharSet = pTmpCharItemSet;
        pOtherSet = pTmpOtherItemSet;

        bDelete = true;
    }

    SwHistory* pHistory = pUndo ? pUndo->GetHistory() : 0;
    BOOL bRet = FALSE;
    const SwPosition *pStt = rRg.Start(), *pEnd = rRg.End();
    SwCntntNode* pNode = pStt->nNode.GetNode().GetCntntNode();

    if( pNode && pNode->IsTxtNode() )
    {
        // -> #i27615#
        if (rRg.IsInFrontOfLabel())
        {
            SwTxtNode * pTxtNd = pNode->GetTxtNode();
            SwNumRule * pNumRule = pTxtNd->GetNumRule();

            // --> OD 2005-10-24 #126346# - make code robust:
            if ( !pNumRule )
            {
                ASSERT( false,
                        "<InsAttr(..)> - PaM in front of label, but text node has no numbering rule set. This is a serious defect, please inform OD." );
                DELETECHARSETS
                return FALSE;
            }
            // <--

            SwNumFmt aNumFmt = pNumRule->Get(static_cast<USHORT>(pTxtNd->GetLevel()));
            SwCharFmt * pCharFmt =
                pDoc->FindCharFmtByName(aNumFmt.GetCharFmtName());

            if (pCharFmt)
            {
                if (pHistory)
                    pHistory->Add(pCharFmt->GetAttrSet(), *pCharFmt);

                if ( pCharSet )
                    pCharFmt->SetAttr(*pCharSet);
            }

            DELETECHARSETS
            return TRUE;
        }
        // <- #i27615#

        const SwIndex& rSt = pStt->nContent;

        // Attribute ohne Ende haben keinen Bereich
        if ( !bCharAttr && !bOtherAttr )
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
                        pDoc->AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
                    else
                        pDoc->SplitRedline( aPam );
                }
            }
        }

        // TextAttribute mit Ende expandieren nie ihren Bereich
        if ( !bCharAttr && !bOtherAttr )
        {
            // CharFmt wird gesondert behandelt !!!
            // JP 22.08.96: URL-Attribute auch!!
            // TEST_TEMP ToDo: AutoFmt!
            SfxItemSet aTxtSet( pDoc->GetAttrPool(),
                                RES_TXTATR_REFMARK, RES_TXTATR_TOXMARK,
                                RES_TXTATR_DUMMY5, RES_TXTATR_WITHEND_END-1,
                                0 );

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
                                ? nsRedlineType_t::REDLINE_INSERT : nsRedlineType_t::REDLINE_FORMAT, aPam ), true);
                    else if( bTxtIns )
                        pDoc->SplitRedline( aPam );
                }
            }
        }
    }

    // bei PageDesc's, die am Node gesetzt werden, muss immer das
    // Auto-Flag gesetzt werden!!
    const SvxLRSpaceItem* pLRSpace = 0;
    if( pOtherSet && pOtherSet->Count() )
    {
        SwTableNode* pTblNd;
        const SwFmtPageDesc* pDesc;
        if( SFX_ITEM_SET == pOtherSet->GetItemState( RES_PAGEDESC,
                        FALSE, (const SfxPoolItem**)&pDesc ))
        {
            if( pNode )
            {
                // Auto-Flag setzen, nur in Vorlagen ist ohne Auto !
                SwFmtPageDesc aNew( *pDesc );
                // Bug 38479: AutoFlag wird jetzt in der WrtShell gesetzt
                // aNew.SetAuto();

                // Tabellen kennen jetzt auch Umbrueche
                if( 0 == (nFlags & nsSetAttrMode::SETATTR_APICALL) &&
                    0 != ( pTblNd = pNode->FindTableNode() ) )
                {
                    SwTableNode* pCurTblNd = pTblNd;
                    while ( 0 != ( pCurTblNd = pCurTblNd->StartOfSectionNode()->FindTableNode() ) )
                        pTblNd = pCurTblNd;

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

            // bOtherAttr = true means that pOtherSet == rChgSet. In this case
            // we know, that there is only one attribute in pOtherSet. We cannot
            // perform the following operations, instead we return:
            if ( bOtherAttr )
                return TRUE;

            const_cast<SfxItemSet*>(pOtherSet)->ClearItem( RES_PAGEDESC );
            if( !pOtherSet->Count() )
            {
                DELETECHARSETS
                return TRUE;
            }
        }

        // Tabellen kennen jetzt auch Umbrueche
        const SvxFmtBreakItem* pBreak;
        if( pNode && 0 == (nFlags & nsSetAttrMode::SETATTR_APICALL) &&
            0 != (pTblNd = pNode->FindTableNode() ) &&
            SFX_ITEM_SET == pOtherSet->GetItemState( RES_BREAK,
                        FALSE, (const SfxPoolItem**)&pBreak ) )
        {
            SwTableNode* pCurTblNd = pTblNd;
            while ( 0 != ( pCurTblNd = pCurTblNd->StartOfSectionNode()->FindTableNode() ) )
                pTblNd = pCurTblNd;

            // dann am Tabellen Format setzen
            SwFrmFmt* pFmt = pTblNd->GetTable().GetFrmFmt();
            SwRegHistory aRegH( pFmt, *pTblNd, pHistory );
            pFmt->SetAttr( *pBreak );

            // bOtherAttr = true means that pOtherSet == rChgSet. In this case
            // we know, that there is only one attribute in pOtherSet. We cannot
            // perform the following operations, instead we return:
            if ( bOtherAttr )
                return TRUE;

            const_cast<SfxItemSet*>(pOtherSet)->ClearItem( RES_BREAK );
            if( !pOtherSet->Count() )
            {
                DELETECHARSETS
                return TRUE;
            }
        }

        // fuer Sonderbehandlung von LR-Space innerhalb einer Numerierung !!!
        pOtherSet->GetItemState( RES_LR_SPACE, FALSE,
                                (const SfxPoolItem**)&pLRSpace );

        {
            // wenns eine PoolNumRule ist, diese ggfs. anlegen
            const SwNumRuleItem* pRule;
            USHORT nPoolId;
            if( SFX_ITEM_SET == pOtherSet->GetItemState( RES_PARATR_NUMRULE,
                                FALSE, (const SfxPoolItem**)&pRule ) &&
                !pDoc->FindNumRulePtr( pRule->GetValue() ) &&
                USHRT_MAX != (nPoolId = SwStyleNameMapper::GetPoolIdFromUIName ( pRule->GetValue(),
                                nsSwGetPoolIdFromName::GET_POOLID_NUMRULE )) )
                pDoc->GetNumRuleFromPool( nPoolId );
        }

    }

    if( !rRg.HasMark() )        // kein Bereich
    {
        if( !pNode )
        {
            DELETECHARSETS
            return bRet;
        }

        if( pNode->IsTxtNode() && pCharSet && pCharSet->Count() )
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
            else
            {
                Boundary aBndry;
                if( pBreakIt->xBreak.is() )
                    aBndry = pBreakIt->xBreak->getWordBoundary(
                                pTxtNd->GetTxt(), nPtPos,
                                pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                                WordType::ANY_WORD /*ANYWORD_IGNOREWHITESPACES*/,
                                TRUE );

                if( aBndry.startPos < nPtPos && nPtPos < aBndry.endPos )
                {
                    nMkPos = (xub_StrLen)aBndry.startPos;
                    nPtPos = (xub_StrLen)aBndry.endPos;
                }
                else
                    nPtPos = nMkPos = rSt.GetIndex();
            }

            // erstmal die zu ueberschreibenden Attribute aus dem
            // SwpHintsArray entfernen, wenn die Selektion den gesamten
            // Absatz umspannt. (Diese Attribute werden als FormatAttr.
            // eingefuegt und verdraengen nie die TextAttr.!)
            if( !(nFlags & nsSetAttrMode::SETATTR_DONTREPLACE ) &&
                pTxtNd->HasHints() && !nMkPos && nPtPos == rStr.Len() )
            {
                SwIndex aSt( pTxtNd );
                if( pHistory )
                {
                    // fuers Undo alle Attribute sichern
                    SwRegHistory aRHst( *pTxtNd, pHistory );
                    pTxtNd->GetpSwpHints()->Register( &aRHst );
                    pTxtNd->RstAttr( aSt, nPtPos, 0, pCharSet );
                    if( pTxtNd->GetpSwpHints() )
                        pTxtNd->GetpSwpHints()->DeRegister();
                }
                else
                    pTxtNd->RstAttr( aSt, nPtPos, 0, pCharSet );
            }

            // eintragen des Attributes im Node erledigt die SwRegHistory !!
            SwRegHistory( (SwTxtNode*)pNode, *pCharSet,
                            nMkPos, nPtPos, nFlags, pHistory );
            bRet = TRUE;

            if( pDoc->IsRedlineOn() )
            {
                SwPaM aPam( *pNode, nMkPos, *pNode, nPtPos );

                if( pUndo )
                    pUndo->SaveRedlineData( aPam, FALSE );
                pDoc->AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_FORMAT, aPam ), true);
            }
        }
        if( pOtherSet && pOtherSet->Count() )
        {
            SwRegHistory aRegH( pNode, *pNode, pHistory );
            pNode->SetAttr( *pOtherSet );
            bRet = TRUE;
        }

        DELETECHARSETS
        return bRet;
    }

    if( pDoc->IsRedlineOn() && pCharSet && pCharSet->Count() )
    {
        if( pUndo )
            pUndo->SaveRedlineData( rRg, FALSE );
        pDoc->AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_FORMAT, rRg ), true);
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
            if( pNode->IsTxtNode() && pCharSet && pCharSet->Count() )
            {
                SwRegHistory( (SwTxtNode*)pNode, *pCharSet,
                                pStt->nContent.GetIndex(), aCntEnd.GetIndex(),
                                nFlags, pHistory );
            }

            if( pOtherSet && pOtherSet->Count() )
            {
                SwRegHistory aRegH( pNode, *pNode, pHistory );
                pNode->SetAttr( *pOtherSet );
            }

            // lediglich Selektion in einem Node.
            if( pStt->nNode == pEnd->nNode )
            {
                DELETECHARSETS
                return TRUE;
            }
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
                if( pNode->IsTxtNode() && pCharSet && pCharSet->Count() )
                {
                    SwRegHistory( (SwTxtNode*)pNode, *pCharSet,
                                    0, aCntEnd.GetIndex(), nFlags, pHistory );
                }

                if( pOtherSet && pOtherSet->Count() )
                {
                    SwRegHistory aRegH( pNode, *pNode, pHistory );
                    pNode->SetAttr( *pOtherSet );
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
    if( pCharSet && pCharSet->Count() && !( nsSetAttrMode::SETATTR_DONTREPLACE & nFlags ) )
    {

        ParaRstFmt aPara( pStt, pEnd, pHistory, 0, pCharSet );
        pDoc->GetNodes().ForEach( aSt, aEnd, lcl_RstTxtAttr, &aPara );
    }

    BOOL bCreateSwpHints = pCharSet && (
        SFX_ITEM_SET == pCharSet->GetItemState( RES_TXTATR_CHARFMT, FALSE ) ||
        SFX_ITEM_SET == pCharSet->GetItemState( RES_TXTATR_INETFMT, FALSE ) );

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

            if( pTNd && pCharSet && pCharSet->Count() )
            {
                pSwpHints = bCreateSwpHints ? &pTNd->GetOrCreateSwpHints()
                                            : pTNd->GetpSwpHints();
                if( pSwpHints )
                    pSwpHints->Register( &aRegH );

                pTNd->SetAttr( *pCharSet, 0, pTNd->GetTxt().Len(), nFlags );
                if( pSwpHints )
                    pSwpHints->DeRegister();
            }
            if( pOtherSet && pOtherSet->Count() )
                pNode->SetAttr( *pOtherSet );
        }
        else
        {
            if( pTNd && pCharSet && pCharSet->Count() )
                pTNd->SetAttr( *pCharSet, 0, pTNd->GetTxt().Len(), nFlags );
            if( pOtherSet && pOtherSet->Count() )
                pNode->SetAttr( *pOtherSet );
        }
        ++nNodes;
    }

    DELETECHARSETS
    return nNodes != 0;
}


bool SwDoc::Insert( const SwPaM &rRg, const SfxPoolItem &rHt, USHORT nFlags )
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

    if( DoesUndo() )
        AppendUndo( pUndoAttr );

    if( bRet )
        SetModified();
    return bRet;
}

bool SwDoc::Insert( const SwPaM &rRg, const SfxItemSet &rSet, USHORT nFlags )
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

// --> OD 2008-02-12 #newlistlevelattrs#
void SwDoc::ResetAttrAtFormat( const USHORT nWhichId,
                               SwFmt& rChangedFormat )
{
    SwUndo* pUndo = 0;
    if ( DoesUndo() )
        pUndo = new SwUndoFmtResetAttr( rChangedFormat, nWhichId );

    const BOOL bAttrReset = rChangedFormat.ResetAttr( nWhichId );

    if ( bAttrReset )
    {
        if ( pUndo )
            AppendUndo( pUndo );

        SetModified();
    }
    else if ( pUndo )
        delete pUndo;
}
// <--

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
    USHORT n;

    for( n = nOldCnt; n ; --n, --pTabs )
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
    USHORT nWhich;
    const SfxPoolItem* pItem = aIter.GetCurItem();
    SfxItemPool* pSdrPool = GetAttrPool().GetSecondaryPool();
    while( TRUE )
    {
        BOOL bCheckSdrDflt = FALSE;
        nWhich = pItem->Which();
        aOld.Put( GetAttrPool().GetDefaultItem( nWhich ) );
        GetAttrPool().SetPoolDefaultItem( *pItem );
        aNew.Put( GetAttrPool().GetDefaultItem( nWhich ) );

        if( RES_CHRATR_BEGIN <= nWhich && RES_TXTATR_END > nWhich )
        {
            aCallMod.Add( pDfltTxtFmtColl );
            aCallMod.Add( pDfltCharFmt );
            bCheckSdrDflt = 0 != pSdrPool;
        }
        else if( RES_PARATR_BEGIN <= nWhich && RES_PARATR_END > nWhich )
        {
            aCallMod.Add( pDfltTxtFmtColl );
            bCheckSdrDflt = 0 != pSdrPool;
        }
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

        // copy also the defaults
        if( bCheckSdrDflt )
        {
            USHORT nEdtWhich, nSlotId;
            if( 0 != (nSlotId = GetAttrPool().GetSlotId( nWhich ) ) &&
                nSlotId != nWhich &&
                0 != (nEdtWhich = pSdrPool->GetWhich( nSlotId )) &&
                nSlotId != nEdtWhich )
            {
                SfxPoolItem* pCpy = pItem->Clone();
                pCpy->SetWhich( nEdtWhich );
                pSdrPool->SetPoolDefaultItem( *pCpy );
                delete pCpy;
            }
        }

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

        const SfxPoolItem* pTmpItem;
        if( ( SFX_ITEM_SET ==
                aNew.GetItemState( RES_PARATR_TABSTOP, FALSE, &pTmpItem ) ) &&
            ((SvxTabStopItem*)pTmpItem)->Count() )
        {
            // TabStop-Aenderungen behandeln wir erstmal anders:
            // dann aender bei allen TabStop die dafault's auf den neuen Wert
            // !!! Achtung: hier wird immer auf dem PoolAttribut gearbeitet,
            //              damit nicht in allen Sets die gleiche Berechnung
            //              auf dem gleichen TabStop (gepoolt!) vorgenommen
            //              wird. Als Modify wird ein FmtChg verschickt.
            SwTwips nNewWidth = (*(SvxTabStopItem*)pTmpItem)[ 0 ].GetTabPos(),
                    nOldWidth = ((SvxTabStopItem&)aOld.Get(RES_PARATR_TABSTOP))[ 0 ].GetTabPos();

            int bChg = FALSE;
            USHORT nMaxItems = GetAttrPool().GetItemCount( RES_PARATR_TABSTOP );
            for( USHORT n = 0; n < nMaxItems; ++n )
                if( 0 != (pTmpItem = GetAttrPool().GetItem( RES_PARATR_TABSTOP, n ) ))
                    bChg |= lcl_SetNewDefTabStops( nOldWidth, nNewWidth,
                                                *(SvxTabStopItem*)pTmpItem );

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
void SwDoc::DelCharFmt(USHORT nFmt, BOOL bBroadcast)
{
    SwCharFmt * pDel = (*pCharFmtTbl)[nFmt];

    if (bBroadcast)
        BroadcastStyleOperation(pDel->GetName(), SFX_STYLE_FAMILY_CHAR,
                                SFX_STYLESHEET_ERASED);

    if (DoesUndo())
    {
        SwUndo * pUndo =
            new SwUndoCharFmtDelete(pDel, this);

        AppendUndo(pUndo);
    }

    pCharFmtTbl->DeleteAndDestroy(nFmt);

    SetModified();
}

void SwDoc::DelCharFmt( SwCharFmt *pFmt, BOOL bBroadcast )
{
    USHORT nFmt = pCharFmtTbl->GetPos( pFmt );
    ASSERT( USHRT_MAX != nFmt, "Fmt not found," );

    DelCharFmt( nFmt, bBroadcast );
}

void SwDoc::DelFrmFmt( SwFrmFmt *pFmt, BOOL bBroadcast )
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
        {
            if (bBroadcast)
                BroadcastStyleOperation(pFmt->GetName(),
                                        SFX_STYLE_FAMILY_FRAME,
                                        SFX_STYLESHEET_ERASED);

            if (DoesUndo())
            {
                SwUndo * pUndo = new SwUndoFrmFmtDelete(pFmt, this);

                AppendUndo(pUndo);
            }

            pFrmFmtTbl->DeleteAndDestroy( nPos );
        }
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
                            SwFrmFmt *pDerivedFrom,
                            BOOL bBroadcast, BOOL bAuto)
{

    SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(), rFmtName, pDerivedFrom );

    pFmt->SetAuto(bAuto);
    pFrmFmtTbl->Insert( pFmt, pFrmFmtTbl->Count());
    SetModified();

    if (bBroadcast)
    {
        BroadcastStyleOperation(rFmtName, SFX_STYLE_FAMILY_PARA,
                                SFX_STYLESHEET_CREATED);

        if (DoesUndo())
        {
            SwUndo * pUndo = new SwUndoFrmFmtCreate(pFmt, pDerivedFrom, this);

            AppendUndo(pUndo);
        }
    }

    return pFmt;
}

SwFmt *SwDoc::_MakeFrmFmt(const String &rFmtName,
                            SwFmt *pDerivedFrom,
                            BOOL bBroadcast, BOOL bAuto)
{
    SwFrmFmt *pFrmFmt = dynamic_cast<SwFrmFmt*>(pDerivedFrom);
    pFrmFmt = MakeFrmFmt( rFmtName, pFrmFmt, bBroadcast, bAuto );
    return dynamic_cast<SwFmt*>(pFrmFmt);
}


// --> OD 2005-01-13 #i40550# - add parameter <bAuto> - not relevant
SwCharFmt *SwDoc::MakeCharFmt( const String &rFmtName,
                               SwCharFmt *pDerivedFrom,
                               BOOL bBroadcast,
                               BOOL )
// <--
{
    SwCharFmt *pFmt = new SwCharFmt( GetAttrPool(), rFmtName, pDerivedFrom );
    pCharFmtTbl->Insert( pFmt, pCharFmtTbl->Count() );
    pFmt->SetAuto( FALSE );
    SetModified();

    if (DoesUndo())
    {
        SwUndo * pUndo = new SwUndoCharFmtCreate(pFmt, pDerivedFrom, this);

        AppendUndo(pUndo);
    }

    if (bBroadcast)
    {
        BroadcastStyleOperation(rFmtName, SFX_STYLE_FAMILY_CHAR,
                                SFX_STYLESHEET_CREATED);
    }

    return pFmt;
}

SwFmt *SwDoc::_MakeCharFmt(const String &rFmtName,
                            SwFmt *pDerivedFrom,
                            BOOL bBroadcast, BOOL bAuto)
{
    SwCharFmt *pCharFmt = dynamic_cast<SwCharFmt*>(pDerivedFrom);
    pCharFmt = MakeCharFmt( rFmtName, pCharFmt, bBroadcast, bAuto );
    return dynamic_cast<SwFmt*>(pCharFmt);
}


/*
 * Erzeugen der FormatCollections
 */
// TXT
// --> OD 2005-01-13 #i40550# - add parameter <bAuto> - not relevant
SwTxtFmtColl* SwDoc::MakeTxtFmtColl( const String &rFmtName,
                                     SwTxtFmtColl *pDerivedFrom,
                                     BOOL bBroadcast,
                                     BOOL )
// <--
{
    SwTxtFmtColl *pFmtColl = new SwTxtFmtColl( GetAttrPool(), rFmtName,
                                                pDerivedFrom );
    pTxtFmtCollTbl->Insert(pFmtColl, pTxtFmtCollTbl->Count());
    pFmtColl->SetAuto( FALSE );
    SetModified();

    if (DoesUndo())
    {
        SwUndo * pUndo = new SwUndoTxtFmtCollCreate(pFmtColl, pDerivedFrom,
                                                    this);
        AppendUndo(pUndo);
    }

    if (bBroadcast)
        BroadcastStyleOperation(rFmtName, SFX_STYLE_FAMILY_PARA,
                                SFX_STYLESHEET_CREATED);

    return pFmtColl;
}

SwFmt *SwDoc::_MakeTxtFmtColl(const String &rFmtName,
                            SwFmt *pDerivedFrom,
                            BOOL bBroadcast, BOOL bAuto)
{
    SwTxtFmtColl *pTxtFmtColl = dynamic_cast<SwTxtFmtColl*>(pDerivedFrom);
    pTxtFmtColl = MakeTxtFmtColl( rFmtName, pTxtFmtColl, bBroadcast, bAuto );
    return dynamic_cast<SwFmt*>(pTxtFmtColl);
}


//FEATURE::CONDCOLL
SwConditionTxtFmtColl* SwDoc::MakeCondTxtFmtColl( const String &rFmtName,
                                                  SwTxtFmtColl *pDerivedFrom,
                                                  BOOL bBroadcast)
{
    SwConditionTxtFmtColl*pFmtColl = new SwConditionTxtFmtColl( GetAttrPool(),
                                                    rFmtName, pDerivedFrom );
    pTxtFmtCollTbl->Insert(pFmtColl, pTxtFmtCollTbl->Count());
    pFmtColl->SetAuto( FALSE );
    SetModified();

    if (bBroadcast)
        BroadcastStyleOperation(rFmtName, SFX_STYLE_FAMILY_PARA,
                                SFX_STYLESHEET_CREATED);

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

void SwDoc::DelTxtFmtColl(USHORT nFmtColl, BOOL bBroadcast)
{
    ASSERT( nFmtColl, "Remove fuer Coll 0." );

    // Wer hat die zu loeschende als Next
    SwTxtFmtColl *pDel = (*pTxtFmtCollTbl)[nFmtColl];
    if( pDfltTxtFmtColl == pDel )
        return;     // default nie loeschen !!

    if (bBroadcast)
        BroadcastStyleOperation(pDel->GetName(), SFX_STYLE_FAMILY_PARA,
                                SFX_STYLESHEET_ERASED);

    if (DoesUndo())
    {
        SwUndoTxtFmtCollDelete * pUndo =
            new SwUndoTxtFmtCollDelete(pDel, this);

        AppendUndo(pUndo);
    }

    // Die FmtColl austragen
    pTxtFmtCollTbl->Remove(nFmtColl);
    // Next korrigieren
    pTxtFmtCollTbl->ForEach( 1, pTxtFmtCollTbl->Count(),
                            &SetTxtFmtCollNext, pDel );
    delete pDel;
    SetModified();
}

void SwDoc::DelTxtFmtColl( SwTxtFmtColl *pColl, BOOL bBroadcast )
{
    USHORT nFmt = pTxtFmtCollTbl->GetPos( pColl );
    ASSERT( USHRT_MAX != nFmt, "Collection not found," );
    DelTxtFmtColl( nFmt, bBroadcast );
}

BOOL lcl_SetTxtFmtColl( const SwNodePtr& rpNode, void* pArgs )
{
    // ParaSetFmtColl * pPara = (ParaSetFmtColl*)pArgs;
    SwCntntNode* pCNd = (SwCntntNode*)rpNode->GetTxtNode();
    if( pCNd )
    {
        ParaRstFmt* pPara = (ParaRstFmt*)pArgs;

        SwTxtFmtColl* pFmt = static_cast<SwTxtFmtColl*>(pPara->pFmtColl);
        if ( pPara->bReset )
        {
            lcl_RstAttr( pCNd, pPara );

            // --> OD 2007-11-06 #i62675#
            if ( pPara->bResetListAttrs &&
                 pFmt->GetItemState( RES_PARATR_NUMRULE ) == SFX_ITEM_SET )
            {
                if ( pPara->pHistory )
                {
                    SwTxtNode* pTNd( dynamic_cast<SwTxtNode*>(pCNd) );
                    ASSERT( pTNd,
                            "<lcl_SetTxtFmtColl(..)> - text node expected -> crash" );
                    SwRegHistory aRegH( pTNd, *pTNd, pPara->pHistory );

                    pCNd->ResetAttr( RES_PARATR_NUMRULE );
                }
                else
                {
                    pCNd->ResetAttr( RES_PARATR_NUMRULE );
                }
            }
            // <--
        }

        // erst in die History aufnehmen, damit ggfs. alte Daten
        // gesichert werden koennen
        if( pPara->pHistory )
            pPara->pHistory->Add( pCNd->GetFmtColl(), pCNd->GetIndex(),
                                    ND_TEXTNODE );

        pCNd->ChgFmtColl( pFmt );

        pPara->nWhich++;
    }
    return TRUE;
}

BOOL SwDoc::SetTxtFmtColl( const SwPaM &rRg,
                           SwTxtFmtColl *pFmt,
                           bool bReset,
                           bool bResetListAttrs )
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

    ParaRstFmt aPara( pStt, pEnd, pHst );
    aPara.pFmtColl = pFmt;
    aPara.bReset = bReset;
    // --> OD 2007-11-06 #i62675#
    aPara.bResetListAttrs = bResetListAttrs;
    // <--

    GetNodes().ForEach( pStt->nNode.GetIndex(), pEnd->nNode.GetIndex()+1,
                        lcl_SetTxtFmtColl, &aPara );
    if( !aPara.nWhich )
        bRet = FALSE;           // keinen gueltigen Node gefunden

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
    // --> OD 2005-01-13 #i40550#
    SwFmt* pNewFmt = (this->*fnCopyFmt)( rFmt.GetName(), pParent, FALSE, TRUE );
    // <--
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

    return (SwFrmFmt*)CopyFmt( rFmt, *GetFrmFmts(), &SwDoc::_MakeFrmFmt,
                                *GetDfltFrmFmt() );
}

// ---- kopiere das Char-Format --------
SwCharFmt* SwDoc::CopyCharFmt( const SwCharFmt& rFmt )
{
    return (SwCharFmt*)CopyFmt( rFmt, *GetCharFmts(),
                                &SwDoc::_MakeCharFmt,
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
                // --> OD 2005-01-13 #i40550#
                (this->*fnCopyFmt)( pSrc->GetName(), &rDfltFmt, FALSE, TRUE );
                // <--
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
        pDest->SetAttr( pSrc->GetAttrSet() );      // kopiere Attribute

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
    USHORT nAttr = static_cast<USHORT>( bCpyHeader ? RES_HEADER : RES_FOOTER );
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
    BOOL bNotifyLayout = FALSE;

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
                &SwDoc::_MakeCharFmt, *pDfltCharFmt );
    CopyFmtArr( *rSource.pFrmFmtTbl, *pFrmFmtTbl,
                &SwDoc::_MakeFrmFmt, *pDfltFrmFmt );
    CopyFmtArr( *rSource.pTxtFmtCollTbl, *pTxtFmtCollTbl,
                &SwDoc::_MakeTxtFmtColl, *pDfltTxtFmtColl );

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
    USHORT nDefDist = rTabItem.Count() ?
        static_cast<USHORT>(rTabItem[0].GetTabPos()) : 1134;
    const SwPosition &rStt = *rPam.Start(), &rEnd = *rPam.End();
    SwNodeIndex aIdx( rStt.nNode );
    while( aIdx <= rEnd.nNode )
    {
        SwTxtNode* pTNd = aIdx.GetNode().GetTxtNode();
        if( pTNd )
        {
            SvxLRSpaceItem aLS( (SvxLRSpaceItem&)pTNd->SwCntntNode::GetAttr(
                                                        RES_LR_SPACE ) );
            long nNext = aLS.GetTxtLeft();
            if( bModulus )
                nNext = ( nNext / nDefDist ) * nDefDist;

            if( bRight )
                nNext += nDefDist;
            else
                nNext -= nDefDist;

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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDoc::_CreateNumberFormatter" );

    ASSERT( !pNumberFormatter, "ist doch schon vorhanden" );


    LanguageType eLang = LANGUAGE_SYSTEM; //System::GetLanguage();
/*              ((const SvxLanguageItem&)GetAttrPool().
                    GetDefaultItem( RES_CHRATR_LANGUAGE )).GetLanguage();
*/
    Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    pNumberFormatter = new SvNumberFormatter( xMSF, eLang );
    pNumberFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_FORMAT_INTL );
    pNumberFormatter->SetYear2000(static_cast<USHORT>(SFX_APP()->GetMiscConfig()->GetYear2000()));

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

    if( mbIsAutoFmtRedline && pTNd )
    {
        // dann das Redline Object anlegen
        const SwTxtFmtColl& rColl = *pTNd->GetTxtColl();
        SwRedline* pRedl = new SwRedline( nsRedlineType_t::REDLINE_FMTCOLL, aPam );
        pRedl->SetMark();

        // interressant sind nur die Items, die vom Set NICHT wieder
        // in den Node gesetzt werden. Also muss man die Differenz nehmen
        SwRedlineExtraData_FmtColl aExtraData( rColl.GetName(),
                                                rColl.GetPoolFmtId() );
        if( pSet && pTNd->HasSwAttrSet() )
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
        AppendRedline( pRedl, true );
    }

    SetTxtFmtColl( aPam, GetTxtCollFromPool( nPoolId ) );

    if( pSet && pTNd && pSet->Count() )
    {
        aPam.SetMark();
        aPam.GetMark()->nContent.Assign( pTNd, pTNd->GetTxt().Len() );
        Insert( aPam, *pSet, 0 );
    }
}

void SwDoc::SetFmtItemByAutoFmt( const SwPaM& rPam, const SfxItemSet& rSet )
{
    SwTxtNode* pTNd = rPam.GetPoint()->nNode.GetNode().GetTxtNode();

    RedlineMode_t eOld = GetRedlineMode();

    if( mbIsAutoFmtRedline && pTNd )
    {
        // dann das Redline Object anlegen
        SwRedline* pRedl = new SwRedline( nsRedlineType_t::REDLINE_FORMAT, rPam );
        if( !pRedl->HasMark() )
            pRedl->SetMark();

        // interressant sind nur die Items, die vom Set NICHT wieder
        // in den Node gesetzt werden. Also muss man die Differenz nehmen
        SwRedlineExtraData_Format aExtraData( rSet );

/*
        if( pSet && pTNd->HasSwAttrSet() )
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
        AppendRedline( pRedl, true );

        SetRedlineMode_intern( (RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));
    }

    Insert( rPam, rSet, nsSetAttrMode::SETATTR_DONTEXPAND );
    SetRedlineMode_intern( eOld );
}

void SwDoc::ChgFmt(SwFmt & rFmt, const SfxItemSet & rSet)
{
    if (DoesUndo())
    {
        // copying <rSet> to <aSet>
        SfxItemSet aSet(rSet);
        // remove from <aSet> all items, which are already set at the format
        aSet.Differentiate(rFmt.GetAttrSet());
        // <aSet> contains now all *new* items for the format

        // copying current format item set to <aOldSet>
        SfxItemSet aOldSet(rFmt.GetAttrSet());
        // insert new items into <aOldSet>
        aOldSet.Put(aSet);
        // invalidate all new items in <aOldSet> in order to clear these items,
        // if the undo action is triggered.
        {
            SfxItemIter aIter(aSet);

            const SfxPoolItem * pItem = aIter.FirstItem();
            while (pItem != NULL)
            {
                aOldSet.InvalidateItem(pItem->Which());

                pItem = aIter.NextItem();
            }
        }

        // --> OD 2007-07-11 #i56253#
        SwUndo * pUndo = new SwUndoFmtAttr(aOldSet, rSet, rFmt);
        // <--

        AppendUndo(pUndo);
    }

    rFmt.SetAttr(rSet);
}

void SwDoc::RenameFmt(SwFmt & rFmt, const String & sNewName,
                      BOOL bBroadcast)
{
    SfxStyleFamily eFamily = SFX_STYLE_FAMILY_ALL;

    if (DoesUndo())
    {
        SwUndo * pUndo = NULL;

        switch (rFmt.Which())
        {
        case RES_CHRFMT:
            pUndo = new SwUndoRenameCharFmt(rFmt.GetName(), sNewName, this);
            eFamily = SFX_STYLE_FAMILY_PARA;
            break;
        case RES_TXTFMTCOLL:
            pUndo = new SwUndoRenameFmtColl(rFmt.GetName(), sNewName, this);
            eFamily = SFX_STYLE_FAMILY_CHAR;
            break;
        case RES_FRMFMT:
            pUndo = new SwUndoRenameFrmFmt(rFmt.GetName(), sNewName, this);
            eFamily = SFX_STYLE_FAMILY_FRAME;
            break;

        default:
            break;
        }

        if (pUndo)
            AppendUndo(pUndo);
    }

    rFmt.SetName(sNewName);

    if (bBroadcast)
        BroadcastStyleOperation(sNewName, eFamily, SFX_STYLESHEET_MODIFIED);
}

// --> OD 2006-09-27 #i69627#
namespace docfunc
{
    bool HasOutlineStyleToBeWrittenAsNormalListStyle( SwDoc& rDoc )
    {
        // If a parent paragraph style of one of the parargraph styles, which
        // are assigned to the list levels of the outline style, has a list style
        // set or inherits a list style from its parent style, the outline style
        // has to be written as a normal list style to the OpenDocument file
        // format or the OpenOffice.org file format.
        bool bRet( false );

        const SwTxtFmtColls* pTxtFmtColls( rDoc.GetTxtFmtColls() );
        if ( pTxtFmtColls )
        {
            const USHORT nCount = pTxtFmtColls->Count();
            for ( USHORT i = 0; i < nCount; ++i )
            {
                SwTxtFmtColl* pTxtFmtColl = (*pTxtFmtColls)[i];

                if ( pTxtFmtColl->IsDefault() ||
                     pTxtFmtColl->GetOutlineLevel() == NO_NUMBERING )
                {
                    continue;
                }

                const SwTxtFmtColl* pParentTxtFmtColl =
                   dynamic_cast<const SwTxtFmtColl*>( pTxtFmtColl->DerivedFrom());
                if ( !pParentTxtFmtColl )
                    continue;

                // --> OD 2007-12-07 #i77708#
                // consider that explicitly no list style is set - empty string
                // at numrule item.
//                const SwNumRuleItem& rDirectItem = pParentTxtFmtColl->GetNumRule();
//                if ( rDirectItem.GetValue().Len() != 0 )
                if ( SFX_ITEM_SET == pParentTxtFmtColl->GetItemState( RES_PARATR_NUMRULE ) )
                {
                    bRet = true;
                    break;
                }
                // <--

            }

        }
        return bRet;
    }
}
// <--
