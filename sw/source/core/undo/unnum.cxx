/*************************************************************************
 *
 *  $RCSfile: unnum.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:28 $
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
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
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
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>
#endif


SV_DECL_PTRARR_DEL( _SfxPoolItems, SfxPoolItem*, 16, 16 );
SV_IMPL_PTRARR( _SfxPoolItems, SfxPoolItem* );

inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

SwUndoInsNum::SwUndoInsNum( const SwNumRule& rOldRule,
                            const SwNumRule& rNewRule )
    : SwUndo( UNDO_INSNUM ),
    aNumRule( rNewRule ), pHistory( 0 ), nLRSavePos( 0 ),
    nSttSet( ULONG_MAX ), pOldNumRule( new SwNumRule( rOldRule ))
{
    ASSERT( rOldRule.IsAutoRule(),
            "darf nur fuer AutoNumRules gerufen werden" );
}

SwUndoInsNum::SwUndoInsNum( const SwPaM& rPam, const SwNumRule& rRule )
    : SwUndo( UNDO_INSNUM ), SwUndRng( rPam ),
    aNumRule( rRule ), pHistory( 0 ), nLRSavePos( 0 ),
    nSttSet( ULONG_MAX ), pOldNumRule( 0 )
{
}

SwUndoInsNum::SwUndoInsNum( const SwPosition& rPos, const SwNumRule& rRule,
                            const String& rReplaceRule )
    : SwUndo( UNDO_INSNUM ),
    aNumRule( rRule ), pHistory( 0 ), nLRSavePos( 0 ),
    nSttSet( ULONG_MAX ), pOldNumRule( 0 ),
    sReplaceRule( rReplaceRule )
{
    // keine Selektion !!
    nEndNode = 0, nEndCntnt = USHRT_MAX;
    nSttNode = rPos.nNode.GetIndex();
    nSttCntnt = rPos.nContent.GetIndex();
}

SwUndoInsNum::~SwUndoInsNum()
{
    delete pHistory;
    delete pOldNumRule;
}

void SwUndoInsNum::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    if( nSttNode )
        SetPaM( rUndoIter );

    BOOL bUndo = rDoc.DoesUndo();
    rDoc.DoUndo( FALSE );

    if( pOldNumRule )
        rDoc.ChgNumRuleFmts( *pOldNumRule );

    if( pHistory )
    {
        SwTxtNode* pNd;
        if( ULONG_MAX != nSttSet &&
            0 != ( pNd = rDoc.GetNodes()[ nSttSet ]->GetTxtNode() ) &&
            pNd->GetNum() )
                ((SwNodeNum*)pNd->GetNum())->SetStart( TRUE );
        else
            pNd = 0;


        if( nLRSavePos )
        {
            // sofort Updaten, damit eventuell "alte" LRSpaces wieder
            // gueltig werden.
            // !!! Dafuer suche aber erstmal den richtigen NumRule - Namen!
            if( !pNd && nSttNode )
                pNd = rDoc.GetNodes()[ nSttNode ]->GetTxtNode();

            const SwNumRule* pNdRule;
            if( pNd )
                pNdRule = pNd->GetNumRule();
            else
                pNdRule = rDoc.FindNumRulePtr( aNumRule.GetName() );

            pHistory->TmpRollback( &rDoc, nLRSavePos );
            if( pNdRule )
                rDoc.UpdateNumRule( pNdRule->GetName(), ULONG_MAX );
        }
        pHistory->TmpRollback( &rDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }

    if( nSttNode )
        SetPaM( rUndoIter );
    rDoc.DoUndo( bUndo );
}


void SwUndoInsNum::Redo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();

    if( pOldNumRule )
        rDoc.ChgNumRuleFmts( aNumRule );
    else if( pHistory )
    {
        SetPaM( rUndoIter );
        if( sReplaceRule.Len() )
            rDoc.ReplaceNumRule( *rUndoIter.pAktPam->GetPoint(),
                                sReplaceRule, aNumRule.GetName() );
        else
            rDoc.SetNumRule( *rUndoIter.pAktPam, aNumRule, FALSE );
    }
}

void SwUndoInsNum::SetLRSpaceEndPos()
{
    if( pHistory )
        nLRSavePos = pHistory->Count();
}

void SwUndoInsNum::Repeat( SwUndoIter& rUndoIter )
{
    if( nSttNode )
    {
        if( !sReplaceRule.Len() )
            rUndoIter.GetDoc().SetNumRule( *rUndoIter.pAktPam, aNumRule, FALSE );
    }
    else
        rUndoIter.GetDoc().ChgNumRuleFmts( aNumRule );
}

SwHistory* SwUndoInsNum::GetHistory()
{
    if( !pHistory )
        pHistory = new SwHistory;
    return pHistory;
}

void SwUndoInsNum::SaveOldNumRule( const SwNumRule& rOld )
{
    if( !pOldNumRule )
        pOldNumRule = new SwNumRule( rOld );
}

/*  */


SwUndoDelNum::SwUndoDelNum( const SwPaM& rPam )
    : SwUndo( UNDO_DELNUM ), SwUndRng( rPam ),
    aNodeIdx( BYTE( nEndNode - nSttNode > 255 ? 255 : nEndNode - nSttNode )),
    aLevels( BYTE( nEndNode - nSttNode > 255 ? 255 : nEndNode - nSttNode ))
#ifndef NUM_RELSPACE
    , aRstLRSpaces( BYTE( nEndNode - nSttNode > 255 ? 255 : nEndNode - nSttNode ))
#endif
{
    pHistory = new SwHistory;
}


SwUndoDelNum::~SwUndoDelNum()
{
    delete pHistory;
}


void SwUndoDelNum::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    SetPaM( rUndoIter );

    BOOL bUndo = rDoc.DoesUndo();
    rDoc.DoUndo( FALSE );

    pHistory->TmpRollback( &rDoc, 0 );
    pHistory->SetTmpEnd( pHistory->Count() );

    for( USHORT n = 0; n < aNodeIdx.Count(); ++n )
    {
        SwTxtNode* pNd = rDoc.GetNodes()[ aNodeIdx[ n ] ]->GetTxtNode();
        ASSERT( pNd, "wo ist der TextNode geblieben?" );
        pNd->UpdateNum( SwNodeNum( aLevels[ n ] ));

#ifndef NUM_RELSPACE
        pNd->SetNumLSpace( aRstLRSpaces[ n ] );
#endif

        if( pNd->GetCondFmtColl() )
            pNd->ChkCondColl();
    }

    SetPaM( rUndoIter );
    rDoc.DoUndo( bUndo );
}


void SwUndoDelNum::Redo( SwUndoIter& rUndoIter )
{
    SetPaM( rUndoIter );
    rUndoIter.GetDoc().DelNumRules( *rUndoIter.pAktPam );
}


void SwUndoDelNum::Repeat( SwUndoIter& rUndoIter )
{
    SetPaM( rUndoIter );
    rUndoIter.GetDoc().DelNumRules( *rUndoIter.pAktPam );
}

void SwUndoDelNum::AddNode( const SwTxtNode& rNd, BOOL bFlag )
{
    if( rNd.GetNum() && NO_NUMBERING != rNd.GetNum()->GetLevel() )
    {
        register USHORT nIns = aNodeIdx.Count();
        aNodeIdx.Insert( rNd.GetIndex(), nIns );
        aLevels.Insert( rNd.GetNum()->GetLevel(), nIns );
#ifndef NUM_RELSPACE
        aRstLRSpaces.Insert( bFlag, nIns );
#endif
    }
}


/*  */


SwUndoMoveNum::SwUndoMoveNum( const SwPaM& rPam, long nOff, BOOL bIsOutlMv )
    : SwUndo( bIsOutlMv ? UNDO_OUTLINE_UD : UNDO_MOVENUM ),
    SwUndRng( rPam ), nOffset( nOff ),
    nNewStt( 0 )
{
    // nOffset: nach unten  =>  1
    //          nach oben   => -1
}


void SwUndoMoveNum::Undo( SwUndoIter& rUndoIter )
{
    ULONG nTmpStt = nSttNode, nTmpEnd = nEndNode;

    if( !nEndNode && USHRT_MAX == nEndCntnt )       // kein Bereich !
        nEndNode = nSttNode;

    if( nNewStt < nSttNode )        // nach vorne verschoben
        nEndNode = nEndNode - ( nSttNode - nNewStt );
    else
        nEndNode = nEndNode + ( nNewStt - nSttNode );
    nSttNode = nNewStt;

//JP 22.06.95: wird wollen die Bookmarks/Verzeichnisse behalten, oder?
//  SetPaM( rUndoIter );
//  RemoveIdxFromRange( *rUndoIter.pAktPam, TRUE );

    SetPaM( rUndoIter );
    rUndoIter.GetDoc().MoveParagraph( *rUndoIter.pAktPam, -nOffset,
                                        UNDO_OUTLINE_UD == GetId() );
    nSttNode = nTmpStt;
    nEndNode = nTmpEnd;
}


void SwUndoMoveNum::Redo( SwUndoIter& rUndoIter )
{
//JP 22.06.95: wird wollen die Bookmarks/Verzeichnisse behalten, oder?
//  SetPaM( rUndoIter );
//  RemoveIdxFromRange( *rUndoIter.pAktPam, TRUE );

    SetPaM( rUndoIter );
    rUndoIter.GetDoc().MoveParagraph( *rUndoIter.pAktPam, nOffset,
                                        UNDO_OUTLINE_UD == GetId() );
}


void SwUndoMoveNum::Repeat( SwUndoIter& rUndoIter )
{
    if( UNDO_OUTLINE_UD == GetId() )
        rUndoIter.GetDoc().MoveOutlinePara( *rUndoIter.pAktPam,
                                            0 < nOffset ? 1 : -1 );
    else
        rUndoIter.GetDoc().MoveParagraph( *rUndoIter.pAktPam, nOffset, FALSE );
}

/*  */


SwUndoNumUpDown::SwUndoNumUpDown( const SwPaM& rPam, short nOff )
    : SwUndo( UNDO_NUMUPDOWN ), SwUndRng( rPam ), nOffset( nOff )
{
    // nOffset: Down    =>  1
    //          Up      => -1
}


void SwUndoNumUpDown::Undo( SwUndoIter& rUndoIter )
{
    SetPaM( rUndoIter );
    rUndoIter.GetDoc().NumUpDown( *rUndoIter.pAktPam, 1 != nOffset );
}


void SwUndoNumUpDown::Redo( SwUndoIter& rUndoIter )
{
    SetPaM( rUndoIter );
    rUndoIter.GetDoc().NumUpDown( *rUndoIter.pAktPam, 1 == nOffset );
}


void SwUndoNumUpDown::Repeat( SwUndoIter& rUndoIter )
{
    rUndoIter.GetDoc().NumUpDown( *rUndoIter.pAktPam, 1 == nOffset );
}

/*  */


SwUndoNumOrNoNum::SwUndoNumOrNoNum( const SwNodeIndex& rIdx, BOOL bDelete,
                                    BOOL bOutln )
    : SwUndo( UNDO_NUMORNONUM ), nIdx( rIdx.GetIndex() ), bDel( bDelete ),
    bOutline( bOutln )
{
}


void SwUndoNumOrNoNum::Undo( SwUndoIter& rUndoIter )
{
    SwNodeIndex aIdx( rUndoIter.GetDoc().GetNodes(), nIdx );
    rUndoIter.GetDoc().NumOrNoNum( aIdx, !bDel, bOutline );
}


void SwUndoNumOrNoNum::Redo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    SwNodeIndex aIdx( rDoc.GetNodes(), nIdx );
    rDoc.NumOrNoNum( aIdx, bDel, bOutline );
}


void SwUndoNumOrNoNum::Repeat( SwUndoIter& rUndoIter )
{
    rUndoIter.GetDoc().NumOrNoNum( rUndoIter.pAktPam->GetPoint()->nNode,
                                    bDel, bOutline );
}

/*  */

SwUndoNumRuleStart::SwUndoNumRuleStart( const SwPosition& rPos, BOOL bFlg )
    : SwUndo( UNDO_SETNUMRULESTART ),
    nIdx( rPos.nNode.GetIndex() ), bFlag( bFlg ), bSetSttValue( FALSE ),
    nNewStt( USHRT_MAX ), nOldStt( USHRT_MAX )
{
}

SwUndoNumRuleStart::SwUndoNumRuleStart( const SwPosition& rPos, USHORT nStt )
    : SwUndo( UNDO_SETNUMRULESTART ),
    nIdx( rPos.nNode.GetIndex() ), bSetSttValue( TRUE ),
    nNewStt( nStt ), nOldStt( USHRT_MAX )
{
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( pTxtNd && pTxtNd->GetNum() )
        nOldStt = pTxtNd->GetNum()->GetSetValue();
}


void SwUndoNumRuleStart::Undo( SwUndoIter& rUndoIter )
{
    SwPosition aPos( *rUndoIter.GetDoc().GetNodes()[ nIdx ] );
    if( bSetSttValue )
        rUndoIter.GetDoc().SetNodeNumStart( aPos, nOldStt );
    else
        rUndoIter.GetDoc().SetNumRuleStart( aPos, !bFlag );
}


void SwUndoNumRuleStart::Redo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();

    SwPosition aPos( *rDoc.GetNodes()[ nIdx ] );
    if( bSetSttValue )
        rDoc.SetNodeNumStart( aPos, nNewStt );
    else
        rDoc.SetNumRuleStart( aPos, bFlag );
}


void SwUndoNumRuleStart::Repeat( SwUndoIter& rUndoIter )
{
    if( bSetSttValue )
        rUndoIter.GetDoc().SetNodeNumStart( *rUndoIter.pAktPam->GetPoint(), nNewStt );
    else
        rUndoIter.GetDoc().SetNumRuleStart( *rUndoIter.pAktPam->GetPoint(), bFlag );
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/unnum.cxx,v 1.1.1.1 2000-09-19 00:08:28 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.29  2000/09/18 16:04:29  willem.vandorp
      OpenOffice header added.

      Revision 1.28  1999/03/15 22:29:20  JP
      Task #63049#: Numerierung mit rel. Einzuegen


      Rev 1.27   15 Mar 1999 23:29:20   JP
   Task #63049#: Numerierung mit rel. Einzuegen

      Rev 1.26   11 Nov 1998 10:41:50   JP
   Task #59308#: NoNum auch bei Outlines setzen

      Rev 1.25   09 Nov 1998 17:34:22   JP
   Bug #57903#: Flag fuers umsetzen der LRSpaces mit sichern

      Rev 1.24   08 Apr 1998 15:37:16   JP
   Bug #49140#: UndoInsNum - LRSpaces sichern

      Rev 1.23   02 Apr 1998 15:13:32   JP
   Redo: Undo-Flag wird schon von der EditShell abgeschaltet

      Rev 1.22   26 Jan 1998 10:44:30   JP
   neu: eigenen Startwert fuer die akt. Numerierung am Node setzen

      Rev 1.21   23 Jan 1998 17:01:58   JP
   SwUndoOutlineUpDown entfernt, wird durch MoveParagraph ersetzt

      Rev 1.20   09 Dec 1997 16:49:50   JP
   neu: ReplaceNumRule fuer MakeByExample fuer NumerierungsVorlagen

      Rev 1.19   19 Nov 1997 09:59:22   JP
   Undo fuer Set-/ChgNumRule implementiert

      Rev 1.18   18 Nov 1997 16:34:12   JP
   neues UndoObject, UndoDelNum an neue Num. angepasst

      Rev 1.17   17 Nov 1997 09:46:24   JP
   Umstellung Numerierung

      Rev 1.16   03 Nov 1997 13:06:20   MA
   precomp entfernt

      Rev 1.15   09 Oct 1997 15:45:38   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.14   11 Jun 1997 10:45:02   JP
   pure virtual Repeat wurde zur virtual Methode, Segment Pragma entfernt

      Rev 1.13   11 Mar 1997 16:21:58   AMA
   New: Absaetze verschieben durch Strg + CursorUp/Down (auch ausserhalb von Num.)

      Rev 1.12   04 Feb 1997 18:54:32   JP
   SwUndoInsNum: bei keiner Selektion EndNode auf StartNode setzen

      Rev 1.11   13 Dec 1996 14:58:24   JP
   Bug #34497#: auf TextNode abpruefen

      Rev 1.10   12 Dec 1996 13:10:40   JP
   Bug #34429#: CTOR-UndoInNum: Selektion auf Anfang/Ende der Nodes setzen

      Rev 1.9   29 Oct 1996 14:54:44   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.8   26 Jul 1996 09:24:02   JP
   SetNumRules - neues Flag fuers setzen des abs. Abstandes

      Rev 1.7   11 Apr 1996 17:14:52   sdo
   GCC, C40_INSERT

      Rev 1.6   26 Mar 1996 15:26:14   JP
   neu: SwUndoNumOrNoNum - Number an/aus schalten

      Rev 1.5   26 Feb 1996 20:49:40   JP
   UndoMoveNum: es muss kein Bereich vorhanden sein!

      Rev 1.4   24 Nov 1995 17:14:00   OM
   PCH->PRECOMPILED

      Rev 1.3   17 Nov 1995 10:21:52   MA
   Segmentierung

      Rev 1.2   27 Jul 1995 08:45:42   mk
   an SCC4.0.1a angepasst (MDA)

      Rev 1.1   22 Jun 1995 19:33:30   JP
   virt. Methode GetUndoRange vernichtet, Objecte rufen jetzt die Basis-Klasse

      Rev 1.0   03 May 1995 14:17:52   JP
   Initial revision.

*************************************************************************/


