/*************************************************************************
 *
 *  $RCSfile: undraw.cxx,v $
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

#include <rtl/string.h>

#ifndef _RTL_MEMORY_H
#include <rtl/memory.h>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _RTL_STRING_H
#include <rtl/string.h>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDMARK_HXX //autogen wg. SdrMarkList
#include <svx/svdmark.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _TXTFLCNT_HXX //autogen
#include <txtflcnt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FRAME_HXX
#include <frame.hxx>
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
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _DVIEW_HXX
#include <dview.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _ROOTFRM_HXX //autogen wg. SwRootFrm
#include <rootfrm.hxx>
#endif
#ifndef _VIEWSH_HXX //autogen wg. ViewShell
#include <viewsh.hxx>
#endif

#pragma optimize("elg",off)

struct SwUndoGroupObjImpl
{
    SwDrawFrmFmt* pFmt;
    SdrObject* pObj;
    ULONG nNodeIdx;

    // longs statt einem Point benutzen (sonst legt der Compiler fuer
    // diese Klasse einen eigenen CTOR an!)
    long aRelPosX, aRelPosY, aAnchorPosX, aAnchorPosY;

    void SetRelPos( const Point& rPt )
                            { aRelPosX = rPt.X(), aRelPosY = rPt.Y(); }
    void SetAnchorPos(const Point& rPt )
                            { aAnchorPosX = rPt.X(), aAnchorPosY = rPt.Y(); }

    Point& GetRelPos() const            { return *(Point*)(&aRelPosX); }
    Point& GetAnchorPos() const         { return *(Point*)(&aAnchorPosX); }
};


inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

// Draw-Objecte

IMPL_LINK( SwDoc, AddDrawUndo, SdrUndoAction *, pUndo )
{
#ifdef DEBUG
    USHORT nId = pUndo->GetId();
    String sComment( pUndo->GetComment() );
#endif

    if( DoesUndo() && !IsNoDrawUndoObj() )
    {
        ClearRedo();
        const SdrMarkList* pMarkList = 0;
        ViewShell* pSh = GetRootFrm() ? GetRootFrm()->GetCurrShell() : 0;
        if( pSh && pSh->HasDrawView() )
            pMarkList = &pSh->GetDrawView()->GetMarkList();

        AppendUndo( new SwSdrUndo( pUndo, pMarkList ) );
    }
    else
        delete pUndo;
    return 0;
}

SwSdrUndo::SwSdrUndo( SdrUndoAction* pUndo, const SdrMarkList* pMrkLst )
    : SwUndo( UNDO_DRAWUNDO ), pSdrUndo( pUndo )
{
    if( pMrkLst && pMrkLst->GetMarkCount() )
        pMarkList = new SdrMarkList( *pMrkLst );
    else
        pMarkList = 0;
}

SwSdrUndo::~SwSdrUndo()
{
    delete pSdrUndo;
    delete pMarkList;
}

void SwSdrUndo::Undo( SwUndoIter& rUndoIter )
{
    pSdrUndo->Undo();
    rUndoIter.pMarkList = pMarkList;
}

void SwSdrUndo::Redo( SwUndoIter& rUndoIter )
{
    pSdrUndo->Redo();
    rUndoIter.pMarkList = pMarkList;
}

String SwSdrUndo::GetComment() const
{
    return pSdrUndo->GetComment();
}

//--------------------------------------------

void lcl_SendRemoveToUno( SwFmt& rFmt )
{
    SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, &rFmt );
    rFmt.Modify( &aMsgHint, &aMsgHint );
}

void lcl_SaveAnchor( SwFrmFmt* pFmt, ULONG& rNodePos )
{
    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
    if( FLY_AT_CNTNT == rAnchor.GetAnchorId() ||
        FLY_AUTO_CNTNT == rAnchor.GetAnchorId() ||
        FLY_AT_FLY == rAnchor.GetAnchorId() ||
        FLY_IN_CNTNT == rAnchor.GetAnchorId() )
    {
        rNodePos = rAnchor.GetCntntAnchor()->nNode.GetIndex();
        xub_StrLen nCntntPos = 0;

        if( FLY_IN_CNTNT == rAnchor.GetAnchorId() )
        {
            nCntntPos = rAnchor.GetCntntAnchor()->nContent.GetIndex();

            // TextAttribut zerstoeren
            SwTxtNode *pTxtNd = pFmt->GetDoc()->GetNodes()[ rNodePos ]->GetTxtNode();
            ASSERT( pTxtNd, "Kein Textnode gefunden" );
            SwTxtFlyCnt* pAttr = (SwTxtFlyCnt*)pTxtNd->GetTxtAttr( nCntntPos );
            // Attribut steht noch im TextNode, loeschen
            if( pAttr && pAttr->GetFlyCnt().GetFrmFmt() == pFmt )
            {
                // Pointer auf 0, nicht loeschen
                ((SwFmtFlyCnt&)pAttr->GetFlyCnt()).SetFlyFmt();
                SwIndex aIdx( pTxtNd, nCntntPos );
                pTxtNd->Erase( aIdx, 1 );
            }
        }
        else if( FLY_AUTO_CNTNT == rAnchor.GetAnchorId() )
            nCntntPos = rAnchor.GetCntntAnchor()->nContent.GetIndex();

        pFmt->SetAttr( SwFmtAnchor( rAnchor.GetAnchorId(), nCntntPos ) );
    }
}

void lcl_RestoreAnchor( SwFrmFmt* pFmt, ULONG& rNodePos )
{
    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
    if( FLY_AT_CNTNT == rAnchor.GetAnchorId() ||
        FLY_AUTO_CNTNT == rAnchor.GetAnchorId() ||
        FLY_AT_FLY == rAnchor.GetAnchorId() ||
        FLY_IN_CNTNT == rAnchor.GetAnchorId() )
    {
        xub_StrLen nCntntPos = rAnchor.GetPageNum();
        SwNodes& rNds = pFmt->GetDoc()->GetNodes();

        SwNodeIndex aIdx( rNds, rNodePos );
        SwPosition aPos( aIdx );

        SwFmtAnchor aTmp( rAnchor.GetAnchorId() );
        if( FLY_IN_CNTNT == rAnchor.GetAnchorId() ||
            FLY_AUTO_CNTNT == rAnchor.GetAnchorId() )
            aPos.nContent.Assign( aIdx.GetNode().GetCntntNode(), nCntntPos );
        aTmp.SetAnchor( &aPos );
        pFmt->SetAttr( aTmp );

        if( FLY_IN_CNTNT == rAnchor.GetAnchorId() )
        {
            SwTxtNode *pTxtNd = aIdx.GetNode().GetTxtNode();
            ASSERT( pTxtNd, "Kein Textnode gefunden" );
            pTxtNd->Insert( SwFmtFlyCnt( (SwFrmFmt*)pFmt ),
                            nCntntPos, nCntntPos );
        }
    }
}

SwUndoDrawGroup::SwUndoDrawGroup( USHORT nCnt )
    : SwUndo( UNDO_DRAWGROUP ), nSize( nCnt + 1 ), bDelFmt( TRUE )
{
    pObjArr = new SwUndoGroupObjImpl[ nSize ];
}

SwUndoDrawGroup::~SwUndoDrawGroup()
{
    if( bDelFmt )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr + 1;
        for( USHORT n = 1; n < nSize; ++n, ++pTmp )
            delete pTmp->pFmt;
    }
    else
        delete pObjArr->pFmt;       // das GroupObject-Format

    __DELETE ( nSize ) pObjArr;
}

void SwUndoDrawGroup::Undo( SwUndoIter& )
{
    bDelFmt = FALSE;

    // das Group-Object sichern
    SwDrawFrmFmt* pFmt = pObjArr->pFmt;
    SwDrawContact* pContact = (SwDrawContact*)pFmt->FindContactObj();
    SdrObject* pObj = pContact->GetMaster();
    pObjArr->pObj = pObj;
    pObjArr->SetAnchorPos( pObj->GetAnchorPos() );
    pObjArr->SetRelPos( pObj->GetRelativePos() );

    //loescht sich selbst!
    pContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetBoundRect() );
    pObj->SetUserCall( 0 );

    ::lcl_SaveAnchor( pFmt, pObjArr->nNodeIdx );

    // alle Uno-Objecte sollten sich jetzt abmelden
    ::lcl_SendRemoveToUno( *pFmt );

    // aus dem Array austragen
    SwDoc* pDoc = pFmt->GetDoc();
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();
    rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));

    for( USHORT n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        ::lcl_RestoreAnchor( rSave.pFmt, rSave.nNodeIdx );
        rFlyFmts.Insert( rSave.pFmt, rFlyFmts.Count() );

        pObj = rSave.pObj;
        Point aPos( pObj->GetRelativePos() );
        pObj->NbcSetRelativePos( rSave.GetRelPos() );
        rSave.SetRelPos( aPos );

        aPos = pObj->GetAnchorPos();
        pObj->NbcSetAnchorPos( rSave.GetAnchorPos() );
        rSave.SetAnchorPos( aPos );

        SwDrawContact *pContact = new SwDrawContact( rSave.pFmt, pObj );
        pContact->ConnectToLayout();
    }
}

void SwUndoDrawGroup::Redo( SwUndoIter& )
{
    bDelFmt = TRUE;

    // aus dem Array austragen
    SwDoc* pDoc = pObjArr->pFmt->GetDoc();
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();
    SdrObject* pObj;

    for( USHORT n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        pObj = rSave.pObj;
        Point aPos( pObj->GetRelativePos() );
        pObj->NbcSetRelativePos( rSave.GetRelPos() );
        rSave.SetRelPos( aPos );

        aPos = pObj->GetAnchorPos();
        pObj->NbcSetAnchorPos( rSave.GetAnchorPos() );
        rSave.SetAnchorPos( aPos );

        SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
        //loescht sich selbst!
        pContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetBoundRect() );
        pObj->SetUserCall( 0 );

        ::lcl_SaveAnchor( rSave.pFmt, rSave.nNodeIdx );

        // alle Uno-Objecte sollten sich jetzt abmelden
        ::lcl_SendRemoveToUno( *rSave.pFmt );

        rFlyFmts.Remove( rFlyFmts.GetPos( rSave.pFmt ));
    }

    // das Group-Object wieder einfuegen
    ::lcl_RestoreAnchor( pObjArr->pFmt, pObjArr->nNodeIdx );
    rFlyFmts.Insert( pObjArr->pFmt, rFlyFmts.Count() );

    pObjArr->pObj->NbcSetAnchorPos( pObjArr->GetAnchorPos() );
    pObjArr->pObj->NbcSetRelativePos( pObjArr->GetRelPos() );
    SwDrawContact *pContact = new SwDrawContact( pObjArr->pFmt, pObjArr->pObj );
//JP 07.07.99: no ConnectToLayout, because it recalc the ordnum new before
//              the SdrUndo calls his Redo, and so SdrUndo never find his
//              SdrObjects for the OrdNums.
//  pContact->ConnectToLayout();
}

void SwUndoDrawGroup::AddObj( USHORT nPos, SwDrawFrmFmt* pFmt, SdrObject* pObj )
{
    SwUndoGroupObjImpl& rSave = *( pObjArr + nPos + 1 );
    rSave.pObj = pObj;
    rSave.pFmt = pFmt;
    rSave.SetAnchorPos( pObj->GetAnchorPos() );
    rSave.SetRelPos( pObj->GetRelativePos() );
    ::lcl_SaveAnchor( pFmt, rSave.nNodeIdx );

    // alle Uno-Objecte sollten sich jetzt abmelden
    ::lcl_SendRemoveToUno( *pFmt );

    // aus dem Array austragen
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pFmt->GetDoc()->GetSpzFrmFmts();
    rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));
}

void SwUndoDrawGroup::SetGroupFmt( SwDrawFrmFmt* pFmt )
{
    pObjArr->pObj = 0;
    pObjArr->pFmt = pFmt;
}


// ------------------------------

SwUndoDrawUnGroup::SwUndoDrawUnGroup( SdrObjGroup* pObj )
    : SwUndo( UNDO_DRAWUNGROUP ), bDelFmt( FALSE )
{
    nSize = (USHORT)pObj->GetSubList()->GetObjCount() + 1;
    pObjArr = new SwUndoGroupObjImpl[ nSize ];

    SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
    SwDrawFrmFmt* pFmt = (SwDrawFrmFmt*)pContact->GetFmt();

    pObjArr->pObj = pObj;
    pObjArr->pFmt = pFmt;
    pObjArr->SetRelPos( pObj->GetRelativePos() );
    pObjArr->SetAnchorPos( pObj->GetAnchorPos() );

    //loescht sich selbst!
    pContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetBoundRect() );
    pObj->SetUserCall( 0 );

    ::lcl_SaveAnchor( pFmt, pObjArr->nNodeIdx );

    // alle Uno-Objecte sollten sich jetzt abmelden
    ::lcl_SendRemoveToUno( *pFmt );

    // aus dem Array austragen
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pFmt->GetDoc()->GetSpzFrmFmts();
    rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));
}

SwUndoDrawUnGroup::~SwUndoDrawUnGroup()
{
    if( bDelFmt )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr + 1;
        for( USHORT n = 1; n < nSize; ++n, ++pTmp )
            delete pTmp->pFmt;
    }
    else
        delete pObjArr->pFmt;       // das GroupObject-Format

    __DELETE ( nSize ) pObjArr;
}

void SwUndoDrawUnGroup::Undo( SwUndoIter& rIter )
{
    bDelFmt = TRUE;

    // aus dem Array austragen
    SwDoc* pDoc = &rIter.GetDoc();
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();

    for( USHORT n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        SwDrawContact* pContact = (SwDrawContact*)rSave.pFmt->FindContactObj();

        rSave.pObj = pContact->GetMaster();
        rSave.SetRelPos( rSave.pObj->GetRelativePos() );
        rSave.SetAnchorPos( rSave.pObj->GetAnchorPos() );

        //loescht sich selbst!
        pContact->Changed( *rSave.pObj, SDRUSERCALL_DELETE,
            rSave.pObj->GetBoundRect() );
        rSave.pObj->SetUserCall( 0 );

        ::lcl_SaveAnchor( rSave.pFmt, rSave.nNodeIdx );

        // alle Uno-Objecte sollten sich jetzt abmelden
        ::lcl_SendRemoveToUno( *rSave.pFmt );

        rFlyFmts.Remove( rFlyFmts.GetPos( rSave.pFmt ));
    }

    // das Group-Object wieder einfuegen
    ::lcl_RestoreAnchor( pObjArr->pFmt, pObjArr->nNodeIdx );
    rFlyFmts.Insert( pObjArr->pFmt, rFlyFmts.Count() );

    pObjArr->pObj->NbcSetRelativePos( pObjArr->GetRelPos() );
    pObjArr->pObj->NbcSetAnchorPos( pObjArr->GetAnchorPos() );

    SwDrawContact *pContact = new SwDrawContact( pObjArr->pFmt, pObjArr->pObj );
    pContact->ConnectToLayout();
}

void SwUndoDrawUnGroup::Redo( SwUndoIter& )
{
    bDelFmt = FALSE;

    // das Group-Object sichern
    SwDrawFrmFmt* pFmt = pObjArr->pFmt;
    SwDrawContact* pContact = (SwDrawContact*)pFmt->FindContactObj();

    //loescht sich selbst!
    pContact->Changed( *pObjArr->pObj, SDRUSERCALL_DELETE,
        pObjArr->pObj->GetBoundRect() );
    pObjArr->pObj->SetUserCall( 0 );

    ::lcl_SaveAnchor( pFmt, pObjArr->nNodeIdx );

    // alle Uno-Objecte sollten sich jetzt abmelden
    ::lcl_SendRemoveToUno( *pFmt );

    // aus dem Array austragen
    SwDoc* pDoc = pFmt->GetDoc();
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();
    rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));

    for( USHORT n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        ::lcl_RestoreAnchor( rSave.pFmt, rSave.nNodeIdx );
        rFlyFmts.Insert( rSave.pFmt, rFlyFmts.Count() );

        SdrObject* pObj = rSave.pObj;
        Point aPos( pObj->GetRelativePos() );
        pObj->NbcSetRelativePos( rSave.GetRelPos() );
        rSave.SetRelPos( aPos );

        aPos = pObj->GetAnchorPos();
        pObj->NbcSetAnchorPos( rSave.GetAnchorPos() );
        rSave.SetAnchorPos( aPos );

        SwDrawContact *pContact = new SwDrawContact( rSave.pFmt, rSave.pObj );
        pContact->ConnectToLayout();
    }
}

void SwUndoDrawUnGroup::AddObj( USHORT nPos, SwDrawFrmFmt* pFmt )
{
    SwUndoGroupObjImpl& rSave = *( pObjArr + nPos + 1 );
    rSave.pFmt = pFmt;
    rSave.pObj = 0;
}

//-------------------------------------

SwUndoDrawDelete::SwUndoDrawDelete( USHORT nCnt )
    : SwUndo( UNDO_DRAWDELETE ), nSize( nCnt ), bDelFmt( TRUE )
{
    pObjArr = new SwUndoGroupObjImpl[ nSize ];
}

SwUndoDrawDelete::~SwUndoDrawDelete()
{
    if ( bDelFmt )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr;
        for( USHORT n = 0; n < nSize; ++n, ++pTmp )
            delete pTmp->pFmt;
    }
    __DELETE ( nSize ) pObjArr;
}

void SwUndoDrawDelete::Undo( SwUndoIter &rIter )
{
    bDelFmt = FALSE;
    SwSpzFrmFmts& rFlyFmts = *rIter.GetDoc().GetSpzFrmFmts();
    for( USHORT n = 0; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );
        ::lcl_RestoreAnchor( rSave.pFmt, rSave.nNodeIdx );
        rFlyFmts.Insert( rSave.pFmt, rFlyFmts.Count() );
        SdrObject *pObj = rSave.pObj;
        SwDrawContact *pContact = new SwDrawContact( rSave.pFmt, pObj );
        pContact->_Changed( *pObj, SDRUSERCALL_INSERTED, NULL );
    }
}

void SwUndoDrawDelete::Redo( SwUndoIter &rIter )
{
    bDelFmt = TRUE;
    SwSpzFrmFmts& rFlyFmts = *rIter.GetDoc().GetSpzFrmFmts();
    for( USHORT n = 0; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );
        SdrObject *pObj = rSave.pObj;
        SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
        SwDrawFrmFmt *pFmt = (SwDrawFrmFmt*)pContact->GetFmt();
        //loescht sich selbst!
        pContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetBoundRect() );
        pObj->SetUserCall( 0 );

        // alle Uno-Objecte sollten sich jetzt abmelden
        ::lcl_SendRemoveToUno( *pFmt );

        rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));
        ::lcl_SaveAnchor( pFmt, rSave.nNodeIdx );
    }
}

void SwUndoDrawDelete::AddObj( USHORT nPos, SwDrawFrmFmt* pFmt, SdrObject* pObj )
{
    SwUndoGroupObjImpl& rSave = *( pObjArr + nPos );
    rSave.pObj = pObj;
    rSave.pFmt = pFmt;
    ::lcl_SaveAnchor( pFmt, rSave.nNodeIdx );

    // alle Uno-Objecte sollten sich jetzt abmelden
    ::lcl_SendRemoveToUno( *pFmt );

    // aus dem Array austragen
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pFmt->GetDoc()->GetSpzFrmFmts();
    rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/undraw.cxx,v 1.1.1.1 2000-09-19 00:08:27 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.40  2000/09/18 16:04:29  willem.vandorp
      OpenOffice header added.

      Revision 1.39  2000/05/09 10:04:13  jp
      Changes for Unicode

      Revision 1.38  1999/11/03 10:52:54  rt
      #65293# include rtl/memory.h

      Revision 1.37  1999/10/25 19:32:03  tl
      ongoing ONE_LINGU implementation

      Revision 1.36  1999/09/29 06:42:39  mh
      chg: header

      Revision 1.35  1999/07/07 17:26:20  JP
      DrawGroup:Redo: no call of ConnectToLayout, because SdrUndo dont find his objects


      Rev 1.34   07 Jul 1999 19:26:20   JP
   DrawGroup:Redo: no call of ConnectToLayout, because SdrUndo dont find his objects

      Rev 1.33   07 Jul 1999 16:16:56   JP
   Bug #57068#: now we handle the seleection of SdrObjects after undo/redo ourselves

      Rev 1.32   27 Jan 1999 18:52:32   JP
   Task #61014#: FindSdrObject/FindContactObject als Methoden vom SwFrmFmt

      Rev 1.31   07 Jul 1998 16:10:24   AMA
   Fix #52317#: GPF durch Undo-Objekte innerhalb von Gruppen

      Rev 1.30   27 Feb 1998 09:23:02   JP
   Vorm loeschen der FlyFrms aus dem SpzArray den UNO Objecten bescheid geben

      Rev 1.29   20 Nov 1997 18:32:20   MA
   includes

      Rev 1.28   03 Nov 1997 13:06:30   MA
   precomp entfernt

      Rev 1.27   09 Oct 1997 15:45:28   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.26   03 Sep 1997 10:29:40   JP
   zusaetzliches include von docary

      Rev 1.25   18 Aug 1997 10:34:54   OS
   includes

      Rev 1.24   15 Aug 1997 12:37:42   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.23   11 Jun 1997 10:43:46   JP
   pure virtual Repeat wurde zur virtual Methode, Segment Pragma entfernt

      Rev 1.22   15 Apr 1997 14:52:00   AMA
   New: Rahmengebundene Rahmen und auto.positionierte Rahmen

      Rev 1.21   04 Apr 1997 16:44:56   MH
   chg: header

      Rev 1.20   30 Jan 1997 18:18:04   AMA
   Fix: Beim Undo des Loeschens umflossener Objekte muss Bescheid gesagt werden

      Rev 1.19   29 Oct 1996 14:54:00   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.18   24 Oct 1996 12:51:14   AMA
   Chg 342: Changed has changed.

      Rev 1.17   24 Aug 1996 17:00:32   JP
   svdraw.hxx entfernt

      Rev 1.16   13 Jun 1996 16:07:38   MA
   splitt si.hxx

      Rev 1.15   10 Jun 1996 16:04:26   HJS
   clooks etc.

      Rev 1.14   29 May 1996 18:35:00   NF
   define SDR_UNDO dazu...

      Rev 1.13   23 May 1996 08:20:42   OS
   Chg 319: Delete() -> Changed()

      Rev 1.12   20 May 1996 16:02:04   AMA
   Fix: Auch beim Redo muss das Objekt ggf. im TxtFrm verankert werden.

      Rev 1.11   20 May 1996 10:44:14   JP
   Save/Restore: TextAttribut zerstoeren/anlegen

      Rev 1.10   17 May 1996 18:04:44   JP
   DrawObjecte koennen auch zeichengebunden sein

      Rev 1.9   06 Feb 1996 15:16:46   JP
   Link Umstellung 305

      Rev 1.8   03 Dec 1995 13:58:20   MA
   fix(22938): UndoDrawDelete braucht doch ein bDelFmt

      Rev 1.7   24 Nov 1995 17:14:06   OM
   PCH->PRECOMPILED

      Rev 1.6   15 Nov 1995 14:27:14   MA
   chg: Undo fuer Zeichenobjekte jetzt richtig

      Rev 1.5   13 Nov 1995 12:09:08   MA
   chg: static -> lcl_

      Rev 1.4   06 Nov 1995 13:43:58   JP
   SwUndoGroupObjImpl: 2 Points gegen 4 longs ausgetauscht, erspart den CTOR

      Rev 1.3   06 Sep 1995 15:52:40   MA
   fix: Optimierung teilw. abgeschaltet

      Rev 1.2   06 Sep 1995 12:13:22   MA
   fix: includes

      Rev 1.1   04 Sep 1995 10:18:04   MA
   fix: fuer MSC verstaendlich impl.

      Rev 1.0   22 Aug 1995 17:47:12   JP
   Initial revision.

*************************************************************************/

