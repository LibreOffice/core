/*************************************************************************
 *
 *  $RCSfile: docfly.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-06 16:03:40 $
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

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDCAPT_HXX //autogen
#include <svx/svdocapt.hxx>
#endif
#ifndef _SVDMARK_HXX //autogen
#include <svx/svdmark.hxx>
#endif

#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _DCONTACT_HXX //autogen
#include <dcontact.hxx>
#endif

#include <ndgrf.hxx>
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _NDINDEX_HXX //autogen
#include <ndindex.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _TXTFLCNT_HXX //autogen
#include <txtflcnt.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _TXTFRM_HXX //autogen
#include <txtfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX //autogen
#include <pagefrm.hxx>
#endif
#ifndef _ROOTFRM_HXX //autogen
#include <rootfrm.hxx>
#endif
#ifndef _FLYFRMS_HXX //autogen
#include <flyfrms.hxx>
#endif
#ifndef _FRMTOOL_HXX //autogen
#include <frmtool.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif
#ifndef _TBLSEL_HXX //autogen
#include <tblsel.hxx>
#endif
#ifndef _SWUNDO_HXX //autogen
#include <swundo.hxx>
#endif
#ifndef _SWTABLE_HXX //autogen
#include <swtable.hxx>
#endif
#ifndef _CRSTATE_HXX
#include <crstate.hxx>
#endif
#ifndef _UNDOBJ_HXX //autogen
#include <undobj.hxx>
#endif
#ifndef _FMTCNCT_HXX //autogen
#include <fmtcnct.hxx>
#endif

extern USHORT GetHtmlMode( const SwDocShell* );

/*-----------------17.02.98 08:35-------------------

--------------------------------------------------*/
USHORT SwDoc::GetFlyCount(FlyCntType eType ) const
{
    const SwSpzFrmFmts& rFmts = *GetSpzFrmFmts();
    USHORT nSize = rFmts.Count();
    USHORT nCount = 0;
    const SwNodeIndex* pIdx;
    for ( USHORT i = 0; i < nSize; i++)
    {
        const SwFrmFmt* pFlyFmt = rFmts[ i ];
        if( RES_FLYFRMFMT == pFlyFmt->Which()
            && 0 != ( pIdx = pFlyFmt->GetCntnt().GetCntntIdx() )
            && pIdx->GetNodes().IsDocNodes()
            )
        {
            const SwNode* pNd = GetNodes()[ pIdx->GetIndex() + 1 ];

            switch( eType )
            {
            case FLYCNTTYPE_FRM:
                if(!pNd->IsNoTxtNode())
                    nCount++;
                break;

            case FLYCNTTYPE_GRF:
                if( pNd->IsGrfNode() )
                    nCount++;
                break;

            case FLYCNTTYPE_OLE:
                if(pNd->IsOLENode())
                    nCount++;
                break;

            default:
                nCount++;
            }
        }
    }
    return nCount;
}

/*-----------------17.02.98 08:35-------------------

--------------------------------------------------*/
SwFrmFmt* SwDoc::GetFlyNum( USHORT nIdx, FlyCntType eType )
{
    SwSpzFrmFmts& rFmts = *GetSpzFrmFmts();
    SwFrmFmt* pRetFmt = 0;
    USHORT nSize = rFmts.Count();
    const SwNodeIndex* pIdx;
    USHORT nCount = 0;
    for( USHORT i = 0; !pRetFmt && i < nSize; ++i )
    {
        SwFrmFmt* pFlyFmt = rFmts[ i ];
        if( RES_FLYFRMFMT == pFlyFmt->Which()
            && 0 != ( pIdx = pFlyFmt->GetCntnt().GetCntntIdx() )
            && pIdx->GetNodes().IsDocNodes()
            )
        {
            const SwNode* pNd = GetNodes()[ pIdx->GetIndex() + 1 ];
            switch( eType )
            {
            case FLYCNTTYPE_FRM:
                if( !pNd->IsNoTxtNode() && nIdx == nCount++)
                    pRetFmt = pFlyFmt;
                break;
            case FLYCNTTYPE_GRF:
                if(pNd->IsGrfNode() && nIdx == nCount++ )
                    pRetFmt = pFlyFmt;
                break;
            case FLYCNTTYPE_OLE:
                if(pNd->IsOLENode() && nIdx == nCount++)
                    pRetFmt = pFlyFmt;
                break;
            default:
                if(nIdx == nCount++)
                    pRetFmt = pFlyFmt;
            }
        }
    }
    return pRetFmt;
}

/*  */

/***********************************************************************
#*  Class       :  SwDoc
#*  Methode     :  SetFlyFrmAnchor
#*  Beschreibung:  Das Ankerattribut des FlyFrms aendert sich.
#*  Datum       :  MA 01. Feb. 94
#*  Update      :  JP 09.03.98
#***********************************************************************/

Point lcl_FindAnchorLayPos( SwDoc& rDoc, const SwFmtAnchor& rAnch,
                            const SwFrmFmt* pFlyFmt )
{
    Point aRet;
    if( rDoc.GetRootFrm() )
        switch( rAnch.GetAnchorId() )
        {
        case FLY_IN_CNTNT:
            if( pFlyFmt && rAnch.GetCntntAnchor() )
            {
                const SwFrm* pOld = ((SwFlyFrmFmt*)pFlyFmt)->GetFrm( &aRet, FALSE );
                if( pOld )
                    aRet = pOld->Frm().Pos();
            }
            break;

        case FLY_AT_CNTNT:
        case FLY_AUTO_CNTNT: // LAYER_IMPL
            if( rAnch.GetCntntAnchor() )
            {
                const SwPosition *pPos = rAnch.GetCntntAnchor();
                const SwCntntNode* pNd = pPos->nNode.GetNode().GetCntntNode();
                const SwFrm* pOld = pNd ? pNd->GetFrm( &aRet, 0, FALSE ) : 0;
                if( pOld )
                    aRet = pOld->Frm().Pos();
            }
            break;

        case FLY_AT_FLY: // LAYER_IMPL
            if( rAnch.GetCntntAnchor() )
            {
                const SwFlyFrmFmt* pFmt = (SwFlyFrmFmt*)rAnch.GetCntntAnchor()->
                                                nNode.GetNode().GetFlyFmt();
                const SwFrm* pOld = pFmt ? pFmt->GetFrm( &aRet, FALSE ) : 0;
                if( pOld )
                    aRet = pOld->Frm().Pos();
            }
            break;

        case FLY_PAGE:
            {
                USHORT nPgNum = rAnch.GetPageNum();
                const SwPageFrm *pPage = (SwPageFrm*)rDoc.GetRootFrm()->Lower();
                for( USHORT i = 1; (i <= nPgNum) && pPage; ++i,
                                    pPage = (const SwPageFrm*)pPage->GetNext() )
                    if( i == nPgNum )
                    {
                        aRet = pPage->Frm().Pos();
                        break;
                    }
            }
            break;
        }
    return aRet;
}

BOOL SwDoc::SetFlyFrmAnchor( SwFrmFmt& rFmt, SfxItemSet& rSet, BOOL bNewFrms )
{
    //Ankerwechsel sind fast immer in alle 'Richtungen' erlaubt.
    //Ausnahme: Absatz- bzw. Zeichengebundene Rahmen duerfen wenn sie in
    //Kopf-/Fusszeilen stehen nicht Seitengebunden werden.
    const SwFmtAnchor &rOldAnch = rFmt.GetAnchor();
    const RndStdIds nOld = rOldAnch.GetAnchorId();

    SwFmtAnchor aNewAnch( (SwFmtAnchor&)rSet.Get( RES_ANCHOR ) );
    RndStdIds nNew = aNewAnch.GetAnchorId();

    if( nOld == nNew )
        return FALSE;

    // ist der neue ein gueltiger Anker?
    if( !aNewAnch.GetCntntAnchor() && (FLY_AT_FLY == nNew ||
        FLY_AT_CNTNT == nNew || FLY_IN_CNTNT == nNew ||
        FLY_AUTO_CNTNT == nNew ))
        return FALSE;

#ifndef PRODUCT
    if( nNew == FLY_PAGE &&
        (FLY_AT_CNTNT==nOld || FLY_AUTO_CNTNT==nOld || FLY_IN_CNTNT==nOld ) &&
        IsInHeaderFooter( rOldAnch.GetCntntAnchor()->nNode ) )
        ASSERT( !this, "Unerlaubter Ankerwechsel in Head/Foot." );
#endif

    Point aOldAnchorPos( ::lcl_FindAnchorLayPos( *this, rOldAnch, &rFmt ));
    Point aNewAnchorPos( ::lcl_FindAnchorLayPos( *this, aNewAnch, 0 ));

    //Die alten Frms vernichten. Dabei werden die Views implizit gehidet und
    //doppeltes hiden waere so eine art Show!
    rFmt.DelFrms();

    if( FLY_IN_CNTNT == nOld )
    {
        //Bei InCntnt's wird es spannend: Das TxtAttribut muss vernichtet
        //werden. Leider reisst dies neben den Frms auch noch das Format mit
        //in sein Grab. Um dass zu unterbinden loesen wir vorher die
        //Verbindung zwischen Attribut und Format.
        const SwPosition *pPos = rOldAnch.GetCntntAnchor();
        SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
        ASSERT( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
        const xub_StrLen nIdx = pPos->nContent.GetIndex();
        SwTxtAttr * pHnt = pTxtNode->GetTxtAttr( nIdx, RES_TXTATR_FLYCNT );
#ifndef PRODUCT
        ASSERT( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                    "Missing FlyInCnt-Hint." );
        ASSERT( pHnt && pHnt->GetFlyCnt().GetFrmFmt() == &rFmt,
                    "Wrong TxtFlyCnt-Hint." );
#endif
        ((SwFmtFlyCnt&)pHnt->GetFlyCnt()).SetFlyFmt();

        //Die Verbindung ist geloest, jetzt muss noch das Attribut vernichtet
        //werden.
        pTxtNode->Delete( RES_TXTATR_FLYCNT, nIdx, nIdx );
    }

    //Endlich kann das Attribut gesetzt werden. Es muss das erste Attribut
    //sein; Undo depends on it!
    rFmt.SetAttr( aNewAnch );

    //Positionskorrekturen
    const SfxPoolItem* pItem;
    switch( nNew )
    {
    case FLY_IN_CNTNT:
            //Wenn keine Positionsattribute hereinkommen, dann muss dafuer
            //gesorgt werden, das keine unerlaubte automatische Ausrichtung
            //bleibt.
        {
            const SwPosition *pPos = aNewAnch.GetCntntAnchor();
            SwTxtNode *pNd = pPos->nNode.GetNode().GetTxtNode();
            ASSERT( pNd, "Crsr steht nicht auf TxtNode." );

            pNd->Insert( SwFmtFlyCnt( (SwFlyFrmFmt*)&rFmt ),
                                       pPos->nContent.GetIndex(), 0 );
        }

        if( SFX_ITEM_SET != rSet.GetItemState( RES_VERT_ORIENT, FALSE, &pItem ))
        {
            SwFmtVertOrient aOldV( rFmt.GetVertOrient() );
            BOOL bSet = TRUE;
            switch( aOldV.GetVertOrient() )
            {
            case VERT_LINE_TOP:     aOldV.SetVertOrient( VERT_TOP );   break;
            case VERT_LINE_CENTER:  aOldV.SetVertOrient( VERT_CENTER); break;
            case VERT_LINE_BOTTOM:  aOldV.SetVertOrient( VERT_BOTTOM); break;
            case VERT_NONE:         aOldV.SetVertOrient( VERT_CENTER); break;
            default:
                bSet = FALSE;
            }
            if( bSet )
                rSet.Put( aOldV );
        }
        break;

    case FLY_AT_CNTNT:
    case FLY_AUTO_CNTNT: // LAYER_IMPL
    case FLY_AT_FLY: // LAYER_IMPL
    case FLY_PAGE:
        {
            //Wenn keine Positionsattribute hereinschneien korrigieren wir
            //die Position so, dass die Dokumentkoordinaten des Flys erhalten
            //bleiben.
            //Chg: Wenn sich in den Positionsattributen lediglich die
            //Ausrichtung veraendert (FRAME vs. PRTAREA), dann wird die
            //Position ebenfalls korrigiert.
            if( SFX_ITEM_SET != rSet.GetItemState( RES_HORI_ORIENT, FALSE, &pItem ))
                pItem = 0;

            SwFmtHoriOrient aOldH( rFmt.GetHoriOrient() );

            if( HORI_NONE == aOldH.GetHoriOrient() && ( !pItem ||
                aOldH.GetPos() == ((SwFmtHoriOrient*)pItem)->GetPos() ))
            {
                SwTwips nPos = FLY_IN_CNTNT == nOld ? 0 : aOldH.GetPos();
                nPos += aOldAnchorPos.X() - aNewAnchorPos.X();

                if( pItem )
                {
                    SwFmtHoriOrient* pH = (SwFmtHoriOrient*)pItem;
                    aOldH.SetHoriOrient( pH->GetHoriOrient() );
                    aOldH.SetRelationOrient( pH->GetRelationOrient() );
                }
                aOldH.SetPos( nPos );
                rSet.Put( aOldH );
            }

            if( SFX_ITEM_SET != rSet.GetItemState( RES_VERT_ORIENT, FALSE, &pItem ))
                pItem = 0;
            SwFmtVertOrient aOldV( rFmt.GetVertOrient() );

            if( HORI_NONE == aOldV.GetVertOrient() && (!pItem ||
                aOldV.GetPos() == ((SwFmtVertOrient*)pItem)->GetPos() ) )
            {
                SwTwips nPos = FLY_IN_CNTNT == nOld ? 0 : aOldV.GetPos();
                nPos += aOldAnchorPos.Y() - aNewAnchorPos.Y();
                if( pItem )
                {
                    SwFmtVertOrient* pV = (SwFmtVertOrient*)pItem;
                    aOldV.SetVertOrient( pV->GetVertOrient() );
                    aOldV.SetRelationOrient( pV->GetRelationOrient() );
                }
                aOldV.SetPos( nPos );
                rSet.Put( aOldV );
            }
        }
        break;
    }

    if( bNewFrms )
        rFmt.MakeFrms();

    return TRUE;
}

BOOL SwDoc::SetFlyFrmAttr( SwFrmFmt& rFlyFmt, SfxItemSet& rSet )
{
    if( !rSet.Count() )
        return FALSE;

    _UndoFmtAttr* pSaveUndo = 0;
    if( DoesUndo() )
    {
        ClearRedo();
        pSaveUndo = new _UndoFmtAttr( rFlyFmt );
    }

    //Ist das Ankerattribut dabei? Falls ja ueberlassen wir die Verarbeitung
    //desselben einer Spezialmethode. Sie Returnt TRUE wenn der Fly neu
    //erzeugt werden muss (z.B. weil ein Wechsel des FlyTyps vorliegt).
    BOOL bMakeFrms = SFX_ITEM_SET == rSet.GetItemState( RES_ANCHOR, FALSE ) &&
                    SetFlyFrmAnchor( rFlyFmt, rSet, FALSE );

    const SfxPoolItem* pItem;
    SfxItemIter aIter( rSet );
    SfxItemSet aTmpSet( GetAttrPool(), aFrmFmtSetRange );
    USHORT nWhich = aIter.GetCurItem()->Which();
    do {
        switch( nWhich )
        {
        case RES_FILL_ORDER:
        case RES_BREAK:
        case RES_PAGEDESC:
        case RES_CNTNT:
        case RES_FOOTER:
            ASSERT( !this, ":-) Unbekanntes Attribut fuer Fly." );
            // kein break;
        case RES_CHAIN:
            rSet.ClearItem( nWhich );
            // kein break;
        case RES_ANCHOR:
            break;

        default:
            if( !IsInvalidItem( aIter.GetCurItem() ) && ( SFX_ITEM_SET !=
                rFlyFmt.GetAttrSet().GetItemState( nWhich, TRUE, &pItem ) ||
                *pItem != *aIter.GetCurItem() ))
                aTmpSet.Put( *aIter.GetCurItem() );
            break;
        }

        if( aIter.IsAtEnd() )
            break;

    } while( 0 != ( nWhich = aIter.NextItem()->Which() ) );

    if( aTmpSet.Count() )
        rFlyFmt.SetAttr( aTmpSet );

    if( bMakeFrms )
        rFlyFmt.MakeFrms();

    if( pSaveUndo )
    {
        if( pSaveUndo->pUndo )
            AppendUndo( pSaveUndo->pUndo );
        delete pSaveUndo;
    }

    SetModified();

    return aTmpSet.Count() || bMakeFrms;
}


/***************************************************************************
 *  Methode     :   BOOL SwDoc::SetFrmFmtToFly( SwFlyFrm&, SwFrmFmt& )
 *  Beschreibung:
 *  Erstellt    :   OK 14.04.94 15:40
 *  Aenderung   :   JP 23.04.98
 ***************************************************************************/

BOOL SwDoc::SetFrmFmtToFly( SwFrmFmt& rFmt, SwFrmFmt& rNewFmt,
                            SfxItemSet* pSet, BOOL bKeepOrient )
{
    BOOL bChgAnchor = FALSE, bFrmSz = FALSE;

    const SwFmtFrmSize aFrmSz( rFmt.GetFrmSize() );
    const SwFmtVertOrient aVert( rFmt.GetVertOrient() );
    const SwFmtHoriOrient aHori( rFmt.GetHoriOrient() );

    SwUndoSetFlyFmt* pUndo = 0;
    if( DoesUndo() )
    {
        ClearRedo();
        AppendUndo( pUndo = new SwUndoSetFlyFmt( rFmt, rNewFmt ) );
    }

    //Erstmal die Spalten setzen, sonst gibts nix als Aerger mit dem
    //Set/Reset/Abgleich usw.
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET != rNewFmt.GetAttrSet().GetItemState( RES_COL ))
        rFmt.ResetAttr( RES_COL );

    if( rFmt.DerivedFrom() != &rNewFmt )
    {
        rFmt.SetDerivedFrom( &rNewFmt );

        // 1. wenn nicht automatisch -> ignorieren, sonst -> wech
        // 2. wech damit, MB!
        if( SFX_ITEM_SET == rNewFmt.GetAttrSet().GetItemState( RES_FRM_SIZE, FALSE ))
        {
            rFmt.ResetAttr( RES_FRM_SIZE );
            bFrmSz = TRUE;
        }

        const SfxItemSet* pAsk = pSet;
        if( !pAsk ) pAsk = &rNewFmt.GetAttrSet();
        if( SFX_ITEM_SET == pAsk->GetItemState( RES_ANCHOR, FALSE, &pItem )
            && ((SwFmtAnchor*)pItem)->GetAnchorId() !=
                rFmt.GetAnchor().GetAnchorId() )
        {
            if( pUndo )
                DoUndo( FALSE );

            if( pSet )
                bChgAnchor = SetFlyFrmAnchor( rFmt, *pSet, FALSE );
            else
            {
                //JP 23.04.98: muss den FlyFmt-Range haben, denn im SetFlyFrmAnchor
                //              werden Attribute in diesen gesetzt!
                SfxItemSet aFlySet( *rNewFmt.GetAttrSet().GetPool(),
                                    rNewFmt.GetAttrSet().GetRanges() );
                aFlySet.Put( *pItem );
                bChgAnchor = SetFlyFrmAnchor( rFmt, aFlySet, FALSE );
            }

            if( pUndo )
                DoUndo( TRUE );
        }
    }

    //Hori und Vert nur dann resetten, wenn in der Vorlage eine
    //automatische Ausrichtung eingestellt ist, anderfalls den alten Wert
    //wieder hineinstopfen.
    //JP 09.06.98: beim Update der RahmenVorlage sollte der Fly NICHT
    //              seine Orientierng verlieren (diese wird nicht geupdatet!)
    if( !bKeepOrient )
    {
        const SwFmtVertOrient &rVert = rNewFmt.GetVertOrient();
        if ( VERT_NONE != rVert.GetVertOrient()  )
            rFmt.ResetAttr( RES_VERT_ORIENT );
        else
            rFmt.SetAttr( aVert );

        const SwFmtHoriOrient &rHori = rNewFmt.GetHoriOrient();
        if ( HORI_NONE != rHori.GetHoriOrient() )
            rFmt.ResetAttr( RES_HORI_ORIENT );
        else
            rFmt.SetAttr( aHori );
    }

    rFmt.ResetAttr( RES_PRINT, RES_SURROUND );
    rFmt.ResetAttr( RES_LR_SPACE, RES_UL_SPACE );
    rFmt.ResetAttr( RES_BACKGROUND, RES_COL );
    rFmt.ResetAttr( RES_URL, RES_EDIT_IN_READONLY );

    if( !bFrmSz )
        rFmt.SetAttr( aFrmSz );

    if( bChgAnchor )
        rFmt.MakeFrms();

    if( pUndo )
        rFmt.Remove( pUndo );

    SetModified();
    return bChgAnchor;
}

void SwDoc::GetGrfNms( const SwFlyFrmFmt& rFmt, String* pGrfName,
                        String* pFltName ) const
{
    SwNodeIndex aIdx( *rFmt.GetCntnt().GetCntntIdx(), 1 );
    const SwGrfNode* pGrfNd = aIdx.GetNode().GetGrfNode();
    if( pGrfNd && pGrfNd->IsLinkedFile() )
        pGrfNd->GetFileFilterNms( pGrfName, pFltName );
}

/*************************************************************************
|*
|*  SwDoc::ChgAnchor()
|*
|*  Ersterstellung      MA 10. Jan. 95
|*  Letzte Aenderung    JP 08.07.98
|*
*************************************************************************/

BOOL SwDoc::ChgAnchor( const SdrMarkList& rMrkList, int eAnchorId,
                        BOOL bSameOnly, BOOL bPosCorr )
{
    ASSERT( GetRootFrm(), "Ohne Layout geht gar nichts" );

    if( !rMrkList.GetMarkCount() ||
        rMrkList.GetMark( 0 )->GetObj()->GetUpGroup() )
        return FALSE;           // Kein Ankerwechsel innerhalb von Gruppen

    StartUndo( UNDO_INSATTR );

    BOOL bUnmark = FALSE;
    for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
        if ( !pObj->IsWriterFlyFrame() )
        {
            SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);

            const SwFrm *pOldAnch = pContact->GetAnchor();
            const SwFrm *pNewAnch = pOldAnch;

            BOOL bChanges = TRUE;
            xub_StrLen nIndx = STRING_NOTFOUND;
            SwTxtNode *pTxtNode;
            int nOld = pContact->GetFmt()->GetAnchor().GetAnchorId();
            if( !bSameOnly && FLY_IN_CNTNT == nOld )
            {
                const SwPosition *pPos =
                    pContact->GetFmt()->GetAnchor().GetCntntAnchor();
                pTxtNode = pPos->nNode.GetNode().GetTxtNode();
                ASSERT( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
                nIndx = pPos->nContent.GetIndex();
                if( !pOldAnch )
                {
                    pContact->ConnectToLayout();
                    pOldAnch = pContact->GetAnchor();
                }
                pOldAnch->Calc();
                pObj->ImpSetAnchorPos( pOldAnch->Frm().Pos() );
            }

            if ( bSameOnly )
                eAnchorId = nOld;

            bChanges = FLY_IN_CNTNT != eAnchorId;
            SwFmtAnchor  aNewAnch( (RndStdIds)eAnchorId );
            const Point aPt( pObj->GetAnchorPos() + pObj->GetRelativePos() );

            switch ( eAnchorId )
            {
            case FLY_AT_CNTNT:
            case FLY_AUTO_CNTNT:
                pNewAnch = ::FindAnchor( pOldAnch, aPt, TRUE );
                if( pNewAnch->IsProtected() )
                    pNewAnch = 0;
                else
                {
                    SwPosition aPos( *((SwCntntFrm*)pNewAnch)->GetNode() );
                    aNewAnch.SetType( (RndStdIds)eAnchorId );
                    aNewAnch.SetAnchor( &aPos );
                }
                break;

            case FLY_AT_FLY: // LAYER_IMPL
                {
                    //Ausgehend von der linken oberen Ecke des Fly den
                    //dichtesten SwFlyFrm suchen.
                    SwFrm *pTxtFrm;
                    {
                        SwCrsrMoveState aState( MV_SETONLYTEXT );
                        SwPosition aPos( GetNodes() );
                        Point aPoint( aPt );
                        aPoint.X() -= 1;
                        GetRootFrm()->GetCrsrOfst( &aPos, aPoint, &aState );
                        pTxtFrm = aPos.nNode.GetNode().
                                         GetCntntNode()->GetFrm( 0, 0, FALSE );
                    }
                    const SwFrm *pTmp = ::FindAnchor( pTxtFrm, aPt );
                    pNewAnch = pTmp->FindFlyFrm();
                    if( pNewAnch && !pNewAnch->IsProtected() )
                    {
                        const SwFrmFmt *pTmpFmt = ((SwFlyFrm*)pNewAnch)->GetFmt();
                        const SwFmtCntnt& rCntnt = pTmpFmt->GetCntnt();
                        SwPosition aPos( *rCntnt.GetCntntIdx() );
                        aNewAnch.SetAnchor( &aPos );
                        break;
                    }

                    aNewAnch.SetType( FLY_PAGE );
                    // no break
                }
            case FLY_PAGE:
                {
                    pNewAnch = GetRootFrm()->Lower();
                    while ( pNewAnch && !pNewAnch->Frm().IsInside( aPt ) )
                        pNewAnch = pNewAnch->GetNext();
                    if ( !pNewAnch )
                        continue;

                    aNewAnch.SetPageNum( ((SwPageFrm*)pNewAnch)->GetPhyPageNum());
                }
                break;
            case FLY_IN_CNTNT:
                if( bSameOnly ) // Positions/Groessenaenderung
                {
                    SwDrawFrmFmt *pFmt = (SwDrawFrmFmt*)pContact->GetFmt();
                    const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
                    Point aRelPos = pObj->GetRelativePos();
                    if ( rVert.GetPos() != aRelPos.Y() ||
                            VERT_NONE != rVert.GetVertOrient() )
                    {
                        SwFmtVertOrient aVert( rVert );
                        aVert.SetVertOrient( VERT_NONE );
                        aVert.SetPos( aRelPos.Y() );
                        SetAttr( aVert, *pFmt );
                    }
                    else
                    {
                        if( !pOldAnch )
                        {
                            pContact->ConnectToLayout();
                            pOldAnch = pContact->GetAnchor();
                        }
                        ((SwTxtFrm*)pOldAnch)->Prepare();
                    }
                }
                else            // Ankerwechsel
                {
                    pNewAnch = ::FindAnchor( pOldAnch, aPt, TRUE );
                    if( pNewAnch->IsProtected() )
                    {
                        pNewAnch = 0;
                        break;
                    }

                    bUnmark = ( 0 != i );
                    Point aPoint( aPt );
                    aPoint.X() -= 1;    // nicht im DrawObj landen!!
                    aNewAnch.SetType( FLY_IN_CNTNT );
                    SwPosition aPos( *((SwCntntFrm*)pNewAnch)->GetNode() );
                    if ( pNewAnch->Frm().IsInside( aPoint ) )
                    {
                    // es muss ein TextNode gefunden werden, denn nur dort
                    // ist ein inhaltsgebundenes DrawObjekt zu verankern
                        SwCrsrMoveState aState( MV_SETONLYTEXT );
                        GetRootFrm()->GetCrsrOfst( &aPos, aPoint, &aState );
                    }
                    else
                    {
                        SwCntntNode &rCNd = (SwCntntNode&)
                            *((SwCntntFrm*)pNewAnch)->GetNode();
                        if ( pNewAnch->Frm().Bottom() < aPt.Y() )
                            rCNd.MakeStartIndex( &aPos.nContent );
                        else
                            rCNd.MakeEndIndex( &aPos.nContent );
                    }
                    aNewAnch.SetAnchor( &aPos );
                    SetAttr( aNewAnch, *pContact->GetFmt() );
                    SwTxtNode *pNd = aPos.nNode.GetNode().GetTxtNode();
                    ASSERT( pNd, "Crsr steht nicht auf TxtNode." );

                    pNd->Insert( SwFmtFlyCnt( pContact->GetFmt() ),
                                    aPos.nContent.GetIndex(), 0 );
                }
                break;
            default:
                ASSERT( !this, "unexpected AnchorId." );
            }

            if( bChanges && pNewAnch )
            {
                SetAttr( aNewAnch, *pContact->GetFmt() );
                if( bPosCorr )
                {
                    const Point aTmpRel( aPt - pNewAnch->Frm().Pos() );
                    pObj->NbcSetRelativePos( aTmpRel );
                }
#ifndef PRODUCT
                const Point aIstA( pObj->GetAnchorPos() );
                ASSERT( aIstA == pNewAnch->Frm().Pos(),
                                "ChgAnchor: Wrong Anchor-Pos." );
#endif
            }

            if ( pNewAnch && STRING_NOTFOUND != nIndx )
            {
                //Bei InCntnt's wird es spannend: Das TxtAttribut muss vernichtet
                //werden. Leider reisst dies neben den Frms auch noch das Format mit
                //in sein Grab. Um dass zu unterbinden loesen wir vorher die
                //Verbindung zwischen Attribut und Format.
                SwTxtAttr *pHnt = pTxtNode->GetTxtAttr( nIndx, RES_TXTATR_FLYCNT );
                ((SwFmtFlyCnt&)pHnt->GetFlyCnt()).SetFlyFmt();

                //Die Verbindung ist geloest, jetzt muss noch das Attribut vernichtet
                //werden.
                pTxtNode->Delete( RES_TXTATR_FLYCNT, nIndx, nIndx );
            }
        }
    }

    EndUndo( UNDO_END );
    SetModified();

    return bUnmark;
}


/* -----------------23.07.98 13:56-------------------
 *
 * --------------------------------------------------*/
int SwDoc::Chainable( const SwFrmFmt &rSource, const SwFrmFmt &rDest )
{
    //Die Source darf noch keinen Follow haben.
    const SwFmtChain &rOldChain = rSource.GetChain();
    if ( rOldChain.GetNext() )
        return SW_CHAIN_SOURCE_CHAINED;

    //Ziel darf natuerlich nicht gleich Source sein und es
    //darf keine geschlossene Kette entstehen.
    const SwFrmFmt *pFmt = &rDest;
    do {
        if( pFmt == &rSource )
            return SW_CHAIN_SELF;
        pFmt = pFmt->GetChain().GetNext();
    } while ( pFmt );

    //Auch eine Verkettung von Innen nach aussen oder von aussen
    //nach innen ist nicht zulaessig.
    if( rDest.IsLowerOf( rSource ) || rSource .IsLowerOf( rDest ) )
        return SW_CHAIN_SELF;

    //Das Ziel darf noch keinen Master haben.
    const SwFmtChain &rChain = rDest.GetChain();
    if( rChain.GetPrev() )
        return SW_CHAIN_IS_IN_CHAIN;

    //Das Ziel muss leer sein.
    const SwNodeIndex* pCntIdx = rDest.GetCntnt().GetCntntIdx();
    if( !pCntIdx )
        return SW_CHAIN_NOT_FOUND;

    SwNodeIndex aNxtIdx( *pCntIdx, 1 );
    const SwTxtNode* pTxtNd = aNxtIdx.GetNode().GetTxtNode();
    if( !pTxtNd )
        return SW_CHAIN_NOT_FOUND;

    ULONG nFlySttNd = pCntIdx->GetIndex(), nTstSttNd;
    if( 2 != ( pCntIdx->GetNode().EndOfSectionIndex() - nFlySttNd ) ||
        pTxtNd->GetTxt().Len() )
        return SW_CHAIN_NOT_EMPTY;

    USHORT nArrLen = GetSpzFrmFmts()->Count();
    for( USHORT n = 0; n < nArrLen; ++n )
    {
        const SwFmtAnchor& rAnchor = (*GetSpzFrmFmts())[ n ]->GetAnchor();
        if ( ( rAnchor.GetAnchorId() == FLY_AT_CNTNT ||
               rAnchor.GetAnchorId() == FLY_AT_FLY ||
               rAnchor.GetAnchorId() == FLY_AUTO_CNTNT ) &&
             0 != rAnchor.GetCntntAnchor() &&
             nFlySttNd <= ( nTstSttNd =
                         rAnchor.GetCntntAnchor()->nNode.GetIndex() ) &&
             nTstSttNd < nFlySttNd + 2 )
        {
            return SW_CHAIN_NOT_EMPTY;
        }
    }

    //Auf die richtige Area muessen wir auch noch einen Blick werfen.
    //Beide Flys muessen im selben Bereich (Body, Head/Foot, Fly) sitzen
    //Wenn die Source nicht der selektierte Rahmen ist, so reicht es
    //Wenn ein passender gefunden wird (Der Wunsch kann z.B. von der API
    //kommen).

    // both in the same fly, header, footer or on the page?
    const SwFmtAnchor &rSrcAnchor = rSource.GetAnchor(),
                      &rDstAnchor = rDest.GetAnchor();
    ULONG nEndOfExtras = GetNodes().GetEndOfExtras().GetIndex();
    BOOL bAllowed = FALSE;
    if( FLY_PAGE == rSrcAnchor.GetAnchorId() )
    {
        if( FLY_PAGE == rDstAnchor.GetAnchorId() ||
            ( rDstAnchor.GetCntntAnchor() &&
              rDstAnchor.GetCntntAnchor()->nNode.GetIndex() > nEndOfExtras ))
            bAllowed = TRUE;
    }
    else if( rSrcAnchor.GetCntntAnchor() && rDstAnchor.GetCntntAnchor() )
    {
        const SwNodeIndex &rSrcIdx = rSrcAnchor.GetCntntAnchor()->nNode,
                            &rDstIdx = rDstAnchor.GetCntntAnchor()->nNode;
        const SwStartNode* pSttNd = 0;
        if( rSrcIdx == rDstIdx ||
            ( !pSttNd &&
                0 != ( pSttNd = rSrcIdx.GetNode().FindFlyStartNode() ) &&
                pSttNd == rDstIdx.GetNode().FindFlyStartNode() ) ||
            ( !pSttNd &&
                0 != ( pSttNd = rSrcIdx.GetNode().FindFooterStartNode() ) &&
                pSttNd == rDstIdx.GetNode().FindFooterStartNode() ) ||
            ( !pSttNd &&
                0 != ( pSttNd = rSrcIdx.GetNode().FindHeaderStartNode() ) &&
                pSttNd == rDstIdx.GetNode().FindHeaderStartNode() ) ||
            ( !pSttNd && rDstIdx.GetIndex() > nEndOfExtras &&
                            rSrcIdx.GetIndex() > nEndOfExtras ))
            bAllowed = TRUE;
    }

    return bAllowed ? SW_CHAIN_OK : SW_CHAIN_WRONG_AREA;
}
/* -----------------23.07.98 13:56-------------------
 *
 * --------------------------------------------------*/
int SwDoc::Chain( SwFrmFmt &rSource, const SwFrmFmt &rDest )
{
    int nErr = Chainable( rSource, rDest );
    if ( !nErr )
    {
        StartUndo( UNDO_CHAINE );

        SwFlyFrmFmt& rDestFmt = (SwFlyFrmFmt&)rDest;
        SwFlyFrm* pFly = rDestFmt.GetFrm();

        //Follow an den Master haengen.
        SwFmtChain aChain = rDestFmt.GetChain();
        aChain.SetPrev( &(SwFlyFrmFmt&)rSource );
        SetAttr( aChain, rDestFmt );

        SfxItemSet aSet( GetAttrPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                        RES_CHAIN,  RES_CHAIN, 0 );

        //Follow an den Master haengen.
        aChain.SetPrev( &(SwFlyFrmFmt&)rSource );
        SetAttr( aChain, rDestFmt );

        //Master an den Follow haengen und dafuer sorgen, dass der Master
        //eine fixierte Hoehe hat.
        aChain = rSource.GetChain();
        aChain.SetNext( &rDestFmt );
        aSet.Put( aChain );

        SwFmtFrmSize aSize( rSource.GetFrmSize() );
        if ( aSize.GetSizeType() != ATT_FIX_SIZE )
        {
            SwClientIter aIter( rSource );
            SwFlyFrm *pFly = (SwFlyFrm*)aIter.First( TYPE(SwFlyFrm) );
            if ( pFly )
                aSize.SetHeight( pFly->Frm().Height() );
            aSize.SetSizeType( ATT_FIX_SIZE );
            aSet.Put( aSize );
        }
        SetAttr( aSet, rSource );

        EndUndo( UNDO_CHAINE );
    }
    return nErr;
}
/* -----------------23.07.98 13:56-------------------
 *
 * --------------------------------------------------*/
void SwDoc::Unchain( SwFrmFmt &rFmt )
{
    SwFmtChain aChain( rFmt.GetChain() );
    if ( aChain.GetNext() )
    {
        StartUndo( UNDO_UNCHAIN );
        SwFrmFmt *pFollow = aChain.GetNext();
        aChain.SetNext( 0 );
        SetAttr( aChain, rFmt );
        aChain = pFollow->GetChain();
        aChain.SetPrev( 0 );
        SetAttr( aChain, *pFollow );
        EndUndo( UNDO_UNCHAIN );
    }
}



