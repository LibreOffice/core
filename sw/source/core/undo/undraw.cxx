/*************************************************************************
 *
 *  $RCSfile: undraw.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:48:35 $
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

    // OD 2004-04-15 #i26791# - keeping the anchor and the relative position
    // of drawing objects no longer needed
};


inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

// Draw-Objecte

IMPL_LINK( SwDoc, AddDrawUndo, SdrUndoAction *, pUndo )
{
#if OSL_DEBUG_LEVEL > 1
    USHORT nId = pUndo->GetId();
    String sComment( pUndo->GetComment() );
#endif

    if( DoesUndo() && !IsNoDrawUndoObj() )
    {
        ClearRedo();
        const SdrMarkList* pMarkList = 0;
        ViewShell* pSh = GetRootFrm() ? GetRootFrm()->GetCurrShell() : 0;
        if( pSh && pSh->HasDrawView() )
            pMarkList = &pSh->GetDrawView()->GetMarkedObjectList();

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

    delete [] pObjArr;
}

void SwUndoDrawGroup::Undo( SwUndoIter& )
{
    bDelFmt = FALSE;

    // das Group-Object sichern
    SwDrawFrmFmt* pFmt = pObjArr->pFmt;
    SwDrawContact* pContact = (SwDrawContact*)pFmt->FindContactObj();
    SdrObject* pObj = pContact->GetMaster();
    pObjArr->pObj = pObj;

    //loescht sich selbst!
    pContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
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

        SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
        //loescht sich selbst!
        pContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
        pObj->SetUserCall( 0 );

        ::lcl_SaveAnchor( rSave.pFmt, rSave.nNodeIdx );

        // alle Uno-Objecte sollten sich jetzt abmelden
        ::lcl_SendRemoveToUno( *rSave.pFmt );

        rFlyFmts.Remove( rFlyFmts.GetPos( rSave.pFmt ));
    }

    // das Group-Object wieder einfuegen
    ::lcl_RestoreAnchor( pObjArr->pFmt, pObjArr->nNodeIdx );
    rFlyFmts.Insert( pObjArr->pFmt, rFlyFmts.Count() );

    SwDrawContact *pContact = new SwDrawContact( pObjArr->pFmt, pObjArr->pObj );
    // OD 2004-04-15 #i26791# - correction: connect object to layout
    pContact->ConnectToLayout();
}

void SwUndoDrawGroup::AddObj( USHORT nPos, SwDrawFrmFmt* pFmt, SdrObject* pObj )
{
    SwUndoGroupObjImpl& rSave = *( pObjArr + nPos + 1 );
    rSave.pObj = pObj;
    rSave.pFmt = pFmt;
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

    //loescht sich selbst!
    pContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
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

    delete [] pObjArr;
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

        //loescht sich selbst!
        pContact->Changed( *rSave.pObj, SDRUSERCALL_DELETE,
            rSave.pObj->GetLastBoundRect() );
        rSave.pObj->SetUserCall( 0 );

        ::lcl_SaveAnchor( rSave.pFmt, rSave.nNodeIdx );

        // alle Uno-Objecte sollten sich jetzt abmelden
        ::lcl_SendRemoveToUno( *rSave.pFmt );

        rFlyFmts.Remove( rFlyFmts.GetPos( rSave.pFmt ));
    }

    // das Group-Object wieder einfuegen
    ::lcl_RestoreAnchor( pObjArr->pFmt, pObjArr->nNodeIdx );
    rFlyFmts.Insert( pObjArr->pFmt, rFlyFmts.Count() );

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
        pObjArr->pObj->GetLastBoundRect() );
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
    pMarkLst = new SdrMarkList();
}

SwUndoDrawDelete::~SwUndoDrawDelete()
{
    if( bDelFmt )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr;
        for( USHORT n = 0; n < pMarkLst->GetMarkCount(); ++n, ++pTmp )
            delete pTmp->pFmt;
    }
    delete [] pObjArr;
    delete pMarkLst;
}

void SwUndoDrawDelete::Undo( SwUndoIter &rIter )
{
    bDelFmt = FALSE;
    SwSpzFrmFmts& rFlyFmts = *rIter.GetDoc().GetSpzFrmFmts();
    for( USHORT n = 0; n < pMarkLst->GetMarkCount(); ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );
        ::lcl_RestoreAnchor( rSave.pFmt, rSave.nNodeIdx );
        rFlyFmts.Insert( rSave.pFmt, rFlyFmts.Count() );
        SdrObject *pObj = rSave.pObj;
        SwDrawContact *pContact = new SwDrawContact( rSave.pFmt, pObj );
        pContact->_Changed( *pObj, SDRUSERCALL_INSERTED, NULL );
    }
    rIter.pMarkList = pMarkLst;
}

void SwUndoDrawDelete::Redo( SwUndoIter &rIter )
{
    bDelFmt = TRUE;
    SwSpzFrmFmts& rFlyFmts = *rIter.GetDoc().GetSpzFrmFmts();
    for( USHORT n = 0; n < pMarkLst->GetMarkCount(); ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );
        SdrObject *pObj = rSave.pObj;
        SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
        SwDrawFrmFmt *pFmt = (SwDrawFrmFmt*)pContact->GetFmt();
        //loescht sich selbst!
        pContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
        pObj->SetUserCall( 0 );

        // alle Uno-Objecte sollten sich jetzt abmelden
        ::lcl_SendRemoveToUno( *pFmt );

        rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));
        ::lcl_SaveAnchor( pFmt, rSave.nNodeIdx );
    }
}

void SwUndoDrawDelete::AddObj( USHORT , SwDrawFrmFmt* pFmt,
                                const SdrMark& rMark )
{
    SwUndoGroupObjImpl& rSave = *( pObjArr + pMarkLst->GetMarkCount() );
    rSave.pObj = rMark.GetObj();
    rSave.pFmt = pFmt;
    ::lcl_SaveAnchor( pFmt, rSave.nNodeIdx );

    // alle Uno-Objecte sollten sich jetzt abmelden
    ::lcl_SendRemoveToUno( *pFmt );

    // aus dem Array austragen
    SwDoc* pDoc = pFmt->GetDoc();
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();
    rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));

    pMarkLst->InsertEntry( rMark );
}

