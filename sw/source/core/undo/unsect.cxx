/*************************************************************************
 *
 *  $RCSfile: unsect.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:40:10 $
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

#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif

#ifndef _FMTCNTNT_HXX
#include <fmtcntnt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>           // fuer die UndoIds
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif

#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
/// OD 04.10.2002 #102894#
/// class Calc needed for calculation of the hidden condition of a section.
#ifndef _CALC_HXX
#include <calc.hxx>
#endif


inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

SfxItemSet* lcl_GetAttrSet( const SwSection& rSect )
{
    // Attribute des Formate sichern (Spalten, Farbe, ... )
    // Cntnt- und Protect- Items interessieren nicht (stehen schon in der
    // Section), muessen also entfernen werden
    SfxItemSet* pAttr = 0;
    if( rSect.GetFmt() )
    {
        USHORT nCnt = 1;
        if( rSect.IsProtect() )
            ++nCnt;

        if( nCnt < rSect.GetFmt()->GetAttrSet().Count() )
        {
            pAttr = new SfxItemSet( rSect.GetFmt()->GetAttrSet() );
            pAttr->ClearItem( RES_PROTECT );
            pAttr->ClearItem( RES_CNTNT );
            if( !pAttr->Count() )
                delete pAttr, pAttr = 0;
        }
    }
    return pAttr;
}

SwUndoInsSection::SwUndoInsSection( const SwPaM& rPam, const SwSection& rNew,
                                    const SfxItemSet* pSet )
    : SwUndo( UNDO_INSSECTION ), SwUndRng( rPam ), nSectNodePos( 0 ),
    pHistory( 0 ), pRedlData( 0 ), pAttr( 0 )
{
    if( rNew.ISA( SwTOXBaseSection ))
    {
        const SwTOXBase& rBase = (SwTOXBaseSection&)rNew;
        pSection = new SwTOXBaseSection( rBase );
    }
    else
        pSection = new SwSection( rNew.GetType(), rNew.GetName() );
    *pSection = rNew;

    SwDoc& rDoc = *(SwDoc*)rPam.GetDoc();
    if( rDoc.IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( REDLINE_INSERT,
                                        rDoc.GetRedlineAuthor() );
        SetRedlineMode( rDoc.GetRedlineMode() );
    }

    bSplitAtStt = FALSE;
    bSplitAtEnd = FALSE;
    bUpdateFtn = FALSE;

    if( pSet && pSet->Count() )
        pAttr = new SfxItemSet( *pSet );

    if( !rPam.HasMark() )
    {
        const SwCntntNode* pCNd = rPam.GetPoint()->nNode.GetNode().GetCntntNode();
        if( pCNd && pCNd->GetpSwAttrSet() && (
            !rPam.GetPoint()->nContent.GetIndex() ||
            rPam.GetPoint()->nContent.GetIndex() == pCNd->Len() ))
        {
            SfxItemSet aBrkSet( rDoc.GetAttrPool(), aBreakSetRange );
            aBrkSet.Put( *pCNd->GetpSwAttrSet() );
            if( aBrkSet.Count() )
            {
                pHistory = new SwHistory;
                pHistory->CopyFmtAttr( aBrkSet, pCNd->GetIndex() );
            }
        }
    }
}


SwUndoInsSection::~SwUndoInsSection()
{
    delete pSection;
    delete pRedlData;
    delete pAttr;

    if( pHistory )
        delete pHistory;
}



void SwUndoInsSection::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();

    RemoveIdxFromSection( rDoc, nSectNodePos );

    SwSectionNode* pNd = rDoc.GetNodes()[ nSectNodePos ]->GetSectionNode();
    ASSERT( pNd, "wo ist mein SectionNode?" );

    if( IsRedlineOn( GetRedlineMode() ))
        rDoc.DeleteRedline( *pNd );

    // lag keine Selektion vor ??
    SwNodeIndex aIdx( *pNd );
    if( ( !nEndNode && STRING_MAXLEN == nEndCntnt ) ||
        ( nSttNode == nEndNode && nSttCntnt == nEndCntnt ))
        // loesche einfach alle Nodes
        rDoc.GetNodes().Delete( aIdx, pNd->EndOfSectionIndex() -
                                        aIdx.GetIndex() );
    else
        // einfach das Format loeschen, der Rest erfolgt automatisch
        rDoc.DelSectionFmt( pNd->GetSection().GetFmt() );

    // muessen wir noch zusammenfassen ?
    if( bSplitAtStt )
        Join( rDoc, nSttNode );

    if( bSplitAtEnd )
        Join( rDoc, nEndNode );

    if( pHistory )
        pHistory->TmpRollback( &rDoc, 0, FALSE );

    if( bUpdateFtn )
        rDoc.GetFtnIdxs().UpdateFtn( aIdx );

    SetPaM( rUndoIter );
}


void SwUndoInsSection::Redo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    SetPaM( rUndoIter );

    const SwTOXBaseSection* pUpdateTOX = 0;
    if( pSection->ISA( SwTOXBaseSection ))
    {
        const SwTOXBase& rBase = *(SwTOXBaseSection*)pSection;
        pUpdateTOX = rDoc.InsertTableOf( *rUndoIter.pAktPam->GetPoint(),
                                        rBase, pAttr, TRUE );
    }
    else
        rDoc.Insert( *rUndoIter.pAktPam, *pSection, pAttr, TRUE );

    if( pHistory )
        pHistory->SetTmpEnd( pHistory->Count() );

    SwSectionNode* pSectNd = rDoc.GetNodes()[ nSectNodePos ]->GetSectionNode();
    if( pRedlData && IsRedlineOn( GetRedlineMode() ))
    {
        SwRedlineMode eOld = rDoc.GetRedlineMode();
        rDoc.SetRedlineMode_intern( eOld & ~REDLINE_IGNORE );

        SwPaM aPam( *pSectNd->EndOfSectionNode(), *pSectNd, 1 );
        rDoc.AppendRedline( new SwRedline( *pRedlData, aPam ));
        rDoc.SetRedlineMode_intern( eOld );
    }
    else if( !( REDLINE_IGNORE & GetRedlineMode() ) &&
            rDoc.GetRedlineTbl().Count() )
    {
        SwPaM aPam( *pSectNd->EndOfSectionNode(), *pSectNd, 1 );
        rDoc.SplitRedline( aPam );
    }

    if( pUpdateTOX )
    {
        // Formatierung anstossen
        SwEditShell* pESh = rDoc.GetEditShell();
        if( pESh )
            pESh->CalcLayout();

        // Seitennummern eintragen
        ((SwTOXBaseSection*)pUpdateTOX)->UpdatePageNum();
    }
}


void SwUndoInsSection::Repeat( SwUndoIter& rUndoIter )
{
    if( pSection->ISA( SwTOXBaseSection ))
    {
        const SwTOXBase& rBase = *(SwTOXBaseSection*)pSection;
        rUndoIter.GetDoc().InsertTableOf( *rUndoIter.pAktPam->GetPoint(),
                                            rBase, pAttr, TRUE );
    }
    else
        rUndoIter.GetDoc().Insert( *rUndoIter.pAktPam, *pSection, pAttr );
}


void SwUndoInsSection::Join( SwDoc& rDoc, ULONG nNode )
{
    SwNodeIndex aIdx( rDoc.GetNodes(), nNode );
    SwTxtNode* pTxtNd = aIdx.GetNode().GetTxtNode();
    ASSERT( pTxtNd, "wo ist mein TextNode?" );

    {
        RemoveIdxRel( nNode + 1, SwPosition( aIdx,
                            SwIndex( pTxtNd, pTxtNd->GetTxt().Len() )));
    }
    pTxtNd->JoinNext();

    if( pHistory )
    {
        SwIndex aCntIdx( pTxtNd, 0 );
        pTxtNd->RstAttr( aCntIdx, pTxtNd->GetTxt().Len() );
    }
}


void SwUndoInsSection::SaveSplitNode( SwTxtNode* pTxtNd, BOOL bAtStt )
{
    if( pTxtNd->GetpSwpHints() )
    {
        if( !pHistory )
            pHistory = new SwHistory;
        pHistory->CopyAttr( pTxtNd->GetpSwpHints(), pTxtNd->GetIndex(), 0,
                            pTxtNd->GetTxt().Len(), FALSE );
    }

    if( bAtStt )
        bSplitAtStt = TRUE;
    else
        bSplitAtEnd = TRUE;
}


// -----------------------------

SwUndoDelSection::SwUndoDelSection( const SwSectionFmt& rFmt )
     : SwUndo( UNDO_DELSECTION )
{
    const SwSection& rSect = *rFmt.GetSection();
    if( rSect.ISA( SwTOXBaseSection ))
    {
        const SwTOXBase& rBase = (SwTOXBaseSection&)rSect;
        pSection = new SwTOXBaseSection( rBase );
    }
    else
        pSection = new SwSection( rSect.GetType(), rSect.GetName() );
    *pSection = rSect;

    pAttr = ::lcl_GetAttrSet( rSect );

    const SwNodeIndex* pIdx = rFmt.GetCntnt().GetCntntIdx();
    nSttNd = pIdx->GetIndex();
    nEndNd = pIdx->GetNode().EndOfSectionIndex();
}


SwUndoDelSection::~SwUndoDelSection()
{
    delete pSection;
    delete pAttr;
}


void SwUndoDelSection::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();

    if( pSection->ISA( SwTOXBaseSection ))
    {
        const SwTOXBase& rBase = *(SwTOXBaseSection*)pSection;
        SwTOXBaseSection* pBaseSect =  (SwTOXBaseSection*)rDoc.InsertTableOf(
                                        nSttNd, nEndNd-2, rBase, pAttr );
    }
    else
    {
        SwNodeIndex aStt( rDoc.GetNodes(), nSttNd );
        SwNodeIndex aEnd( rDoc.GetNodes(), nEndNd-2 );
        SwSectionFmt* pFmt = rDoc.MakeSectionFmt( 0 );
        if( pAttr )
            pFmt->SetAttr( *pAttr );

        /// OD 04.10.2002 #102894#
        /// remember inserted section node for further calculations
        SwSectionNode* pInsertedSectNd =
                rDoc.GetNodes().InsertSection( aStt, *pFmt, *pSection, &aEnd );

        if( SFX_ITEM_SET == pFmt->GetItemState( RES_FTN_AT_TXTEND ) ||
            SFX_ITEM_SET == pFmt->GetItemState( RES_END_AT_TXTEND ))
        {
            rDoc.GetFtnIdxs().UpdateFtn( aStt );
        }

        /// OD 04.10.2002 #102894#
        /// consider that section is hidden by condition.
        /// If section is hidden by condition,
        /// recalculate condition and update hidden condition flag.
        /// Recalculation is necessary, because fields, on which the hide
        /// condition depends, can be changed - fields changes aren't undoable.
        /// NOTE: setting hidden condition flag also creates/deletes corresponding
        ///     frames, if the hidden condition flag changes.
        SwSection& aInsertedSect = pInsertedSectNd->GetSection();
        if ( aInsertedSect.IsHidden() &&
             aInsertedSect.GetCondition().Len() > 0 )
        {
            SwCalc aCalc( rDoc );
            rDoc.FldsToCalc( aCalc, pInsertedSectNd->GetIndex() );
            bool bRecalcCondHidden =
                    aCalc.Calculate( aInsertedSect.GetCondition() ).GetBool() ? true : false;
            aInsertedSect.SetCondHidden( bRecalcCondHidden );
        }

    }
}


void SwUndoDelSection::Redo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();

    SwSectionNode* pNd = rDoc.GetNodes()[ nSttNd ]->GetSectionNode();
    ASSERT( pNd, "wo ist mein SectionNode?" );
    // einfach das Format loeschen, der Rest erfolgt automatisch
    rDoc.DelSectionFmt( pNd->GetSection().GetFmt() );
}



SwUndoChgSection::SwUndoChgSection( const SwSectionFmt& rFmt, BOOL bOnlyAttr )
     : SwUndo( UNDO_CHGSECTION ), bOnlyAttrChgd( bOnlyAttr )
{
    const SwSection& rSect = *rFmt.GetSection();
    pSection = new SwSection( rSect.GetType(), rSect.GetName() );
    *pSection = rSect;

    pAttr = ::lcl_GetAttrSet( rSect );

    nSttNd = rFmt.GetCntnt().GetCntntIdx()->GetIndex();
}


SwUndoChgSection::~SwUndoChgSection()
{
    delete pSection;
    delete pAttr;
}


void SwUndoChgSection::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    SwSectionNode* pSectNd = rDoc.GetNodes()[ nSttNd ]->GetSectionNode();
    ASSERT( pSectNd, "wo ist mein SectionNode?" );

    SwSection& rNdSect = pSectNd->GetSection();
    SwFmt* pFmt = rNdSect.GetFmt();

    SfxItemSet* pCur = ::lcl_GetAttrSet( rNdSect );
    if( pAttr )
    {
        // das Content- und Protect-Item muss bestehen bleiben
        const SfxPoolItem* pItem;
        pAttr->Put( pFmt->GetAttr( RES_CNTNT ));
        if( SFX_ITEM_SET == pFmt->GetItemState( RES_PROTECT, TRUE, &pItem ))
            pAttr->Put( *pItem );
        pFmt->DelDiffs( *pAttr );
        pAttr->ClearItem( RES_CNTNT );
        pFmt->SetAttr( *pAttr );
        delete pAttr;
    }
    else
    {
        // dann muessen die alten entfernt werden
        pFmt->ResetAttr( RES_FRMATR_BEGIN, RES_BREAK );
        pFmt->ResetAttr( RES_HEADER, RES_OPAQUE );
        pFmt->ResetAttr( RES_SURROUND, RES_FRMATR_END-1 );
    }
    pAttr = pCur;

    if( !bOnlyAttrChgd )
    {
        BOOL bUpdate = (!rNdSect.IsLinkType() && pSection->IsLinkType() ) ||
                            ( pSection->GetLinkFileName().Len() &&
                                pSection->GetLinkFileName() !=
                                rNdSect.GetLinkFileName());

        SwSection* pTmp = new SwSection( CONTENT_SECTION, aEmptyStr );
        *pTmp = rNdSect;        // das aktuelle sichern

        rNdSect = *pSection;    // das alte setzen

        delete pSection;
        pSection = pTmp;        // das aktuelle ist jetzt das alte

        if( bUpdate )
            rNdSect.CreateLink( CREATE_UPDATE );
        else if( CONTENT_SECTION == rNdSect.GetType() && rNdSect.IsConnected() )
        {
            rNdSect.Disconnect();
            rDoc.GetLinkManager().Remove( &rNdSect.GetBaseLink() );
        }
    }
}


void SwUndoChgSection::Redo( SwUndoIter& rUndoIter )
{
    Undo( rUndoIter );
}



SwUndoChgSectPsswd::SwUndoChgSectPsswd(
                    const ::com::sun::star::uno::Sequence <sal_Int8>& rOld,
                    const SwSectionNode* pSectNd )
    : SwUndo( UNDO_CHGSECTIONPASSWD ), aPasswd( rOld )
{
    nSectNd = pSectNd ? pSectNd->GetIndex() : 0;
}


void SwUndoChgSectPsswd::Undo( SwUndoIter& rIter )
{
    SwDoc& rDoc = rIter.GetDoc();
    ::com::sun::star::uno::Sequence <sal_Int8> aCurr;
    const SwSection* pSect = 0;
    if( nSectNd )
    {
        SwSectionNode* pNd = rDoc.GetNodes()[ nSectNd ]->GetSectionNode();
        ASSERT( pNd, "where is the sectionnode?" );
        pSect = &pNd->GetSection();
        aCurr = pSect->GetPasswd();
    }
    rDoc.ChgSectionPasswd( aPasswd, *pSect );
    aPasswd = aCurr;
}


void SwUndoChgSectPsswd::Redo( SwUndoIter& rUndoIter )
{
    Undo( rUndoIter );
}



