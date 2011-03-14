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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <UndoDraw.hxx>

#include <rtl/string.h>
#include <rtl/memory.h>

#include <svx/svdogrp.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmark.hxx>

#include <hintids.hxx>
#include <hints.hxx>
#include <fmtanchr.hxx>
#include <fmtflcnt.hxx>
#include <txtflcnt.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <frame.hxx>
#include <swundo.hxx>           // fuer die UndoIds
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <dcontact.hxx>
#include <dview.hxx>
#include <rootfrm.hxx>
#include <viewsh.hxx>


struct SwUndoGroupObjImpl
{
    SwDrawFrmFmt* pFmt;
    SdrObject* pObj;
    sal_uLong nNodeIdx;

    // OD 2004-04-15 #i26791# - keeping the anchor and the relative position
    // of drawing objects no longer needed
};


// Draw-Objecte

IMPL_LINK( SwDoc, AddDrawUndo, SdrUndoAction *, pUndo )
{
#if OSL_DEBUG_LEVEL > 1
    sal_uInt16 nId = pUndo->GetId();
    (void)nId;
    String sComment( pUndo->GetComment() );
#endif

    if (GetIDocumentUndoRedo().DoesUndo() &&
        GetIDocumentUndoRedo().DoesDrawUndo())
    {
        const SdrMarkList* pMarkList = 0;
        ViewShell* pSh = GetRootFrm() ? GetRootFrm()->GetCurrShell() : 0;
        if( pSh && pSh->HasDrawView() )
            pMarkList = &pSh->GetDrawView()->GetMarkedObjectList();

        GetIDocumentUndoRedo().AppendUndo( new SwSdrUndo(pUndo, pMarkList) );
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

void SwSdrUndo::UndoImpl(::sw::UndoRedoContext & rContext)
{
    pSdrUndo->Undo();
    rContext.SetSelections(0, pMarkList);
}

void SwSdrUndo::RedoImpl(::sw::UndoRedoContext & rContext)
{
    pSdrUndo->Redo();
    rContext.SetSelections(0, pMarkList);
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

void lcl_SaveAnchor( SwFrmFmt* pFmt, sal_uLong& rNodePos )
{
    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
    if ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
        (FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
        (FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
        (FLY_AS_CHAR == rAnchor.GetAnchorId()))
    {
        rNodePos = rAnchor.GetCntntAnchor()->nNode.GetIndex();
        xub_StrLen nCntntPos = 0;

        if (FLY_AS_CHAR == rAnchor.GetAnchorId())
        {
            nCntntPos = rAnchor.GetCntntAnchor()->nContent.GetIndex();

            // TextAttribut zerstoeren
            SwTxtNode *pTxtNd = pFmt->GetDoc()->GetNodes()[ rNodePos ]->GetTxtNode();
            OSL_ENSURE( pTxtNd, "Kein Textnode gefunden" );
            SwTxtFlyCnt* pAttr = static_cast<SwTxtFlyCnt*>(
                pTxtNd->GetTxtAttrForCharAt( nCntntPos, RES_TXTATR_FLYCNT ));
            // Attribut steht noch im TextNode, loeschen
            if( pAttr && pAttr->GetFlyCnt().GetFrmFmt() == pFmt )
            {
                // Pointer auf 0, nicht loeschen
                ((SwFmtFlyCnt&)pAttr->GetFlyCnt()).SetFlyFmt();
                SwIndex aIdx( pTxtNd, nCntntPos );
                pTxtNd->EraseText( aIdx, 1 );
            }
        }
        else if (FLY_AT_CHAR == rAnchor.GetAnchorId())
        {
            nCntntPos = rAnchor.GetCntntAnchor()->nContent.GetIndex();
        }

        pFmt->SetFmtAttr( SwFmtAnchor( rAnchor.GetAnchorId(), nCntntPos ) );
    }
}

void lcl_RestoreAnchor( SwFrmFmt* pFmt, sal_uLong& rNodePos )
{
    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
    if ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
        (FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
        (FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
        (FLY_AS_CHAR == rAnchor.GetAnchorId()))
    {
        xub_StrLen nCntntPos = rAnchor.GetPageNum();
        SwNodes& rNds = pFmt->GetDoc()->GetNodes();

        SwNodeIndex aIdx( rNds, rNodePos );
        SwPosition aPos( aIdx );

        SwFmtAnchor aTmp( rAnchor.GetAnchorId() );
        if ((FLY_AS_CHAR == rAnchor.GetAnchorId()) ||
            (FLY_AT_CHAR == rAnchor.GetAnchorId()))
        {
            aPos.nContent.Assign( aIdx.GetNode().GetCntntNode(), nCntntPos );
        }
        aTmp.SetAnchor( &aPos );
        pFmt->SetFmtAttr( aTmp );

        if (FLY_AS_CHAR == rAnchor.GetAnchorId())
        {
            SwTxtNode *pTxtNd = aIdx.GetNode().GetTxtNode();
            OSL_ENSURE( pTxtNd, "no Text Node" );
            SwFmtFlyCnt aFmt( pFmt );
            pTxtNd->InsertItem( aFmt, nCntntPos, nCntntPos );
        }
    }
}

SwUndoDrawGroup::SwUndoDrawGroup( sal_uInt16 nCnt )
    : SwUndo( UNDO_DRAWGROUP ), nSize( nCnt + 1 ), bDelFmt( sal_True )
{
    pObjArr = new SwUndoGroupObjImpl[ nSize ];
}

SwUndoDrawGroup::~SwUndoDrawGroup()
{
    if( bDelFmt )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr + 1;
        for( sal_uInt16 n = 1; n < nSize; ++n, ++pTmp )
            delete pTmp->pFmt;
    }
    else
        delete pObjArr->pFmt;       // das GroupObject-Format

    delete [] pObjArr;
}

void SwUndoDrawGroup::UndoImpl(::sw::UndoRedoContext &)
{
    bDelFmt = sal_False;

    // das Group-Object sichern
    SwDrawFrmFmt* pFmt = pObjArr->pFmt;
    SwDrawContact* pDrawContact = (SwDrawContact*)pFmt->FindContactObj();
    SdrObject* pObj = pDrawContact->GetMaster();
    pObjArr->pObj = pObj;

    //loescht sich selbst!
    pDrawContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
    pObj->SetUserCall( 0 );

    ::lcl_SaveAnchor( pFmt, pObjArr->nNodeIdx );

    // alle Uno-Objecte sollten sich jetzt abmelden
    ::lcl_SendRemoveToUno( *pFmt );

    // aus dem Array austragen
    SwDoc* pDoc = pFmt->GetDoc();
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();
    rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));

    for( sal_uInt16 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        ::lcl_RestoreAnchor( rSave.pFmt, rSave.nNodeIdx );
        rFlyFmts.Insert( rSave.pFmt, rFlyFmts.Count() );

        pObj = rSave.pObj;

        SwDrawContact *pContact = new SwDrawContact( rSave.pFmt, pObj );
        pContact->ConnectToLayout();
        // --> OD 2005-03-22 #i45718# - follow-up of #i35635#
        // move object to visible layer
        pContact->MoveObjToVisibleLayer( pObj );
        // <--
        // --> OD 2005-05-10 #i45952# - notify that position attributes
        // are already set
        OSL_ENSURE( rSave.pFmt->ISA(SwDrawFrmFmt),
                "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object" );
        if ( rSave.pFmt->ISA(SwDrawFrmFmt) )
        {
            static_cast<SwDrawFrmFmt*>(rSave.pFmt)->PosAttrSet();
        }
        // <--
    }
}

void SwUndoDrawGroup::RedoImpl(::sw::UndoRedoContext &)
{
    bDelFmt = sal_True;

    // aus dem Array austragen
    SwDoc* pDoc = pObjArr->pFmt->GetDoc();
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();
    SdrObject* pObj;

    for( sal_uInt16 n = 1; n < nSize; ++n )
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
    // --> OD 2005-03-22 #i45718# - follow-up of #i35635#
    // move object to visible layer
    pContact->MoveObjToVisibleLayer( pObjArr->pObj );
    // <--
    // --> OD 2005-05-10 #i45952# - notify that position attributes
    // are already set
    OSL_ENSURE( pObjArr->pFmt->ISA(SwDrawFrmFmt),
            "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object" );
    if ( pObjArr->pFmt->ISA(SwDrawFrmFmt) )
    {
        static_cast<SwDrawFrmFmt*>(pObjArr->pFmt)->PosAttrSet();
    }
    // <--
}

void SwUndoDrawGroup::AddObj( sal_uInt16 nPos, SwDrawFrmFmt* pFmt, SdrObject* pObj )
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
    : SwUndo( UNDO_DRAWUNGROUP ), bDelFmt( sal_False )
{
    nSize = (sal_uInt16)pObj->GetSubList()->GetObjCount() + 1;
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
        for( sal_uInt16 n = 1; n < nSize; ++n, ++pTmp )
            delete pTmp->pFmt;
    }
    else
        delete pObjArr->pFmt;       // das GroupObject-Format

    delete [] pObjArr;
}

void SwUndoDrawUnGroup::UndoImpl(::sw::UndoRedoContext & rContext)
{
    bDelFmt = sal_True;

    SwDoc *const pDoc = & rContext.GetDoc();
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();

    // remove from array
    for( sal_uInt16 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        // --> OD 2006-11-01 #130889# - taken over by <SwUndoDrawUnGroupConnectToLayout>
//        SwDrawContact* pContact = (SwDrawContact*)rSave.pFmt->FindContactObj();

//        rSave.pObj = pContact->GetMaster();

//        //loescht sich selbst!
//        pContact->Changed( *rSave.pObj, SDRUSERCALL_DELETE,
//                           rSave.pObj->GetLastBoundRect() );
//        rSave.pObj->SetUserCall( 0 );
        // <--

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
    // --> OD 2005-03-22 #i45718# - follow-up of #i35635#
    // move object to visible layer
    pContact->MoveObjToVisibleLayer( pObjArr->pObj );
    // <--
    // --> OD 2005-05-10 #i45952# - notify that position attributes
    // are already set
    OSL_ENSURE( pObjArr->pFmt->ISA(SwDrawFrmFmt),
            "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object" );
    if ( pObjArr->pFmt->ISA(SwDrawFrmFmt) )
    {
        static_cast<SwDrawFrmFmt*>(pObjArr->pFmt)->PosAttrSet();
    }
    // <--
}

void SwUndoDrawUnGroup::RedoImpl(::sw::UndoRedoContext &)
{
    bDelFmt = sal_False;

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

    for( sal_uInt16 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        ::lcl_RestoreAnchor( rSave.pFmt, rSave.nNodeIdx );
        rFlyFmts.Insert( rSave.pFmt, rFlyFmts.Count() );

        // --> OD 2006-11-01 #130889# - taken over by <SwUndoDrawUnGroupConnectToLayout>
//        SdrObject* pObj = rSave.pObj;

//        SwDrawContact *pContact = new SwDrawContact( rSave.pFmt, rSave.pObj );
//        pContact->ConnectToLayout();
//        // --> OD 2005-03-22 #i45718# - follow-up of #i35635#
//        // move object to visible layer
//        pContact->MoveObjToVisibleLayer( rSave.pObj );
//        // <--
        // <--
        // --> OD 2005-05-10 #i45952# - notify that position attributes
        // are already set
        OSL_ENSURE( rSave.pFmt->ISA(SwDrawFrmFmt),
                "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object" );
        if ( rSave.pFmt->ISA(SwDrawFrmFmt) )
        {
            static_cast<SwDrawFrmFmt*>(rSave.pFmt)->PosAttrSet();
        }
        // <--
    }
}

void SwUndoDrawUnGroup::AddObj( sal_uInt16 nPos, SwDrawFrmFmt* pFmt )
{
    SwUndoGroupObjImpl& rSave = *( pObjArr + nPos + 1 );
    rSave.pFmt = pFmt;
    rSave.pObj = 0;
}

//-------------------------------------
// --> OD 2006-11-01 #130889#
SwUndoDrawUnGroupConnectToLayout::SwUndoDrawUnGroupConnectToLayout()
    : SwUndo( UNDO_DRAWUNGROUP )
{
}

SwUndoDrawUnGroupConnectToLayout::~SwUndoDrawUnGroupConnectToLayout()
{
}

void
SwUndoDrawUnGroupConnectToLayout::UndoImpl(::sw::UndoRedoContext &)
{
    for ( std::vector< SdrObject >::size_type i = 0;
          i < aDrawFmtsAndObjs.size(); ++i )
    {
        SdrObject* pObj( aDrawFmtsAndObjs[i].second );
        SwDrawContact* pDrawContact( dynamic_cast<SwDrawContact*>(pObj->GetUserCall()) );
        OSL_ENSURE( pDrawContact,
                "<SwUndoDrawUnGroupConnectToLayout::Undo(..)> -- missing SwDrawContact instance" );
        if ( pDrawContact )
        {
            // deletion of instance <pDrawContact> and thus disconnection from
            // the Writer layout.
            pDrawContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
            pObj->SetUserCall( 0 );
        }
    }
}

void
SwUndoDrawUnGroupConnectToLayout::RedoImpl(::sw::UndoRedoContext &)
{
    for ( std::vector< std::pair< SwDrawFrmFmt*, SdrObject* > >::size_type i = 0;
          i < aDrawFmtsAndObjs.size(); ++i )
    {
        SwDrawFrmFmt* pFmt( aDrawFmtsAndObjs[i].first );
        SdrObject* pObj( aDrawFmtsAndObjs[i].second );
        SwDrawContact *pContact = new SwDrawContact( pFmt, pObj );
        pContact->ConnectToLayout();
        pContact->MoveObjToVisibleLayer( pObj );
    }
}

void SwUndoDrawUnGroupConnectToLayout::AddFmtAndObj( SwDrawFrmFmt* pDrawFrmFmt,
                                                     SdrObject* pDrawObject )
{
    aDrawFmtsAndObjs.push_back(
            std::pair< SwDrawFrmFmt*, SdrObject* >( pDrawFrmFmt, pDrawObject ) );
}
// <--

//-------------------------------------

SwUndoDrawDelete::SwUndoDrawDelete( sal_uInt16 nCnt )
    : SwUndo( UNDO_DRAWDELETE ), nSize( nCnt ), bDelFmt( sal_True )
{
    pObjArr = new SwUndoGroupObjImpl[ nSize ];
    pMarkLst = new SdrMarkList();
}

SwUndoDrawDelete::~SwUndoDrawDelete()
{
    if( bDelFmt )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr;
        for( sal_uInt16 n = 0; n < pMarkLst->GetMarkCount(); ++n, ++pTmp )
            delete pTmp->pFmt;
    }
    delete [] pObjArr;
    delete pMarkLst;
}

void SwUndoDrawDelete::UndoImpl(::sw::UndoRedoContext & rContext)
{
    bDelFmt = sal_False;
    SwSpzFrmFmts & rFlyFmts = *rContext.GetDoc().GetSpzFrmFmts();
    for( sal_uInt16 n = 0; n < pMarkLst->GetMarkCount(); ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );
        ::lcl_RestoreAnchor( rSave.pFmt, rSave.nNodeIdx );
        rFlyFmts.Insert( rSave.pFmt, rFlyFmts.Count() );
        SdrObject *pObj = rSave.pObj;
        SwDrawContact *pContact = new SwDrawContact( rSave.pFmt, pObj );
        pContact->_Changed( *pObj, SDRUSERCALL_INSERTED, NULL );
        // --> OD 2005-03-22 #i45718# - follow-up of #i35635#
        // move object to visible layer
        pContact->MoveObjToVisibleLayer( pObj );
        // <--
        // --> OD 2005-05-10 #i45952# - notify that position attributes
        // are already set
        OSL_ENSURE( rSave.pFmt->ISA(SwDrawFrmFmt),
                "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object" );
        if ( rSave.pFmt->ISA(SwDrawFrmFmt) )
        {
            static_cast<SwDrawFrmFmt*>(rSave.pFmt)->PosAttrSet();
        }
        // <--
    }
    rContext.SetSelections(0, pMarkLst);
}

void SwUndoDrawDelete::RedoImpl(::sw::UndoRedoContext & rContext)
{
    bDelFmt = sal_True;
    SwSpzFrmFmts & rFlyFmts = *rContext.GetDoc().GetSpzFrmFmts();
    for( sal_uInt16 n = 0; n < pMarkLst->GetMarkCount(); ++n )
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

void SwUndoDrawDelete::AddObj( sal_uInt16 , SwDrawFrmFmt* pFmt,
                                const SdrMark& rMark )
{
    SwUndoGroupObjImpl& rSave = *( pObjArr + pMarkLst->GetMarkCount() );
    rSave.pObj = rMark.GetMarkedSdrObj();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
