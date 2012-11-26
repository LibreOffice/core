/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <UndoDraw.hxx>

#include <rtl/string.h>
#include <rtl/memory.h>

#include <rtl/string.h>
#include <svx/svdogrp.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdpage.hxx>
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
        SdrObjectVector aSelection;
        ViewShell* pSh = GetCurrentViewShell();
        if( pSh && pSh->HasDrawView() )
        {
            aSelection = pSh->GetDrawView()->getSelectedSdrObjectVectorFromSdrMarkView();
        }

        GetIDocumentUndoRedo().AppendUndo( new SwSdrUndo(pUndo, aSelection) );
    }
    else
        delete pUndo;
    return 0;
}

SwSdrUndo::SwSdrUndo( SdrUndoAction* pUndo, const SdrObjectVector& rSelectedSdrObjectVector )
:   SwUndo(UNDO_DRAWUNDO),
    pSdrUndo(pUndo),
    maSelectedSdrObjectVector(rSelectedSdrObjectVector)
{
}

SwSdrUndo::~SwSdrUndo()
{
    delete pSdrUndo;
    maSelectedSdrObjectVector.clear();
}

void SwSdrUndo::UndoImpl(::sw::UndoRedoContext & rContext)
{
    pSdrUndo->Undo();
    rContext.SetSelections(0, maSelectedSdrObjectVector);
}

void SwSdrUndo::RedoImpl(::sw::UndoRedoContext & rContext)
{
    pSdrUndo->Redo();
    rContext.SetSelections(0, maSelectedSdrObjectVector);
}

String SwSdrUndo::GetComment() const
{
    return pSdrUndo->GetComment();
}

//--------------------------------------------

void lcl_SendRemoveToUno( SwFmt& rFmt )
{
    SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, &rFmt );
    rFmt.ModifyNotification( &aMsgHint, &aMsgHint );
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

            // destroy TextAttribute
            SwTxtNode *pTxtNd = pFmt->GetDoc()->GetNodes()[ rNodePos ]->GetTxtNode();
            ASSERT( pTxtNd, "No text node found!" );
            SwTxtFlyCnt* pAttr = static_cast<SwTxtFlyCnt*>(
                pTxtNd->GetTxtAttrForCharAt( nCntntPos, RES_TXTATR_FLYCNT ));
            // attribute still in text node, delete
            if( pAttr && pAttr->GetFlyCnt().GetFrmFmt() == pFmt )
            {
                // just set pointer to 0, don't delete
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
            ASSERT( pTxtNd, "no Text Node" );
            SwFmtFlyCnt aFmt( pFmt );
            pTxtNd->InsertItem( aFmt, nCntntPos, nCntntPos );
        }
    }
}

SwUndoDrawGroup::SwUndoDrawGroup( sal_uInt32 nCnt )
    : SwUndo( UNDO_DRAWGROUP ), nSize( nCnt + 1 ), bDelFmt( true )
{
    pObjArr = new SwUndoGroupObjImpl[ nSize ];
}

SwUndoDrawGroup::~SwUndoDrawGroup()
{
    if( bDelFmt )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr + 1;
        for( sal_uInt32 n = 1; n < nSize; ++n, ++pTmp )
            delete pTmp->pFmt;
    }
    else
        delete pObjArr->pFmt;

    delete [] pObjArr;
}

void SwUndoDrawGroup::UndoImpl(::sw::UndoRedoContext &)
{
    bDelFmt = false;

    // save group object
    SwDrawFrmFmt* pFmt = pObjArr->pFmt;
    SwDrawContact* pDrawContact = (SwDrawContact*)pFmt->FindContactObj();
    SdrObject* pObj = pDrawContact->GetMaster();
    pObjArr->pObj = pObj;

    // object will destroy itself
    pDrawContact->HandleChanged(*pObj, HINT_SDROBJECTDYING);
    resetConnectionToSdrObject(pObj);

    ::lcl_SaveAnchor( pFmt, pObjArr->nNodeIdx );

    // notify UNO objects to decouple
    ::lcl_SendRemoveToUno( *pFmt );

    // remove from array
    SwDoc* pDoc = pFmt->GetDoc();
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();
    rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));

    for( sal_uInt32 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        ::lcl_RestoreAnchor( rSave.pFmt, rSave.nNodeIdx );
        rFlyFmts.Insert( rSave.pFmt, rFlyFmts.Count() );

        pObj = rSave.pObj;

        SwDrawContact *pContact = new SwDrawContact( rSave.pFmt, pObj );
        pContact->ConnectToLayout();
        // #i45718# - follow-up of #i35635# move object to visible layer
        pContact->MoveObjToVisibleLayer( pObj );
        // #i45952# - notify that position attributes are already set
        ASSERT( dynamic_cast< SwDrawFrmFmt* >(rSave.pFmt),
                "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object" );
        if ( dynamic_cast< SwDrawFrmFmt* >(rSave.pFmt) )
        {
            static_cast<SwDrawFrmFmt*>(rSave.pFmt)->PosAttrSet();
        }
    }
}

void SwUndoDrawGroup::RedoImpl(::sw::UndoRedoContext &)
{
    bDelFmt = true;

    // remove from array
    SwDoc* pDoc = pObjArr->pFmt->GetDoc();
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();
    SdrObject* pObj;

    for( sal_uInt32 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        pObj = rSave.pObj;

        SwDrawContact *pContact = (SwDrawContact*)findConnectionToSdrObject(pObj);

        // object will destroy itself
        pContact->HandleChanged(*pObj, HINT_SDROBJECTDYING);
        resetConnectionToSdrObject(pObj);

        ::lcl_SaveAnchor( rSave.pFmt, rSave.nNodeIdx );

        // notify UNO objects to decouple
        ::lcl_SendRemoveToUno( *rSave.pFmt );

        rFlyFmts.Remove( rFlyFmts.GetPos( rSave.pFmt ));
    }

    // re-insert group object
    ::lcl_RestoreAnchor( pObjArr->pFmt, pObjArr->nNodeIdx );
    rFlyFmts.Insert( pObjArr->pFmt, rFlyFmts.Count() );

    SwDrawContact *pContact = new SwDrawContact( pObjArr->pFmt, pObjArr->pObj );
    // #i26791# - correction: connect object to layout
    pContact->ConnectToLayout();
    // #i45718# - follow-up of #i35635# move object to visible layer
    pContact->MoveObjToVisibleLayer( pObjArr->pObj );
    // #i45952# - notify that position attributes are already set
    ASSERT( dynamic_cast< SwDrawFrmFmt* >(pObjArr->pFmt),
            "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object" );
    if ( dynamic_cast< SwDrawFrmFmt* >(pObjArr->pFmt) )
    {
        static_cast<SwDrawFrmFmt*>(pObjArr->pFmt)->PosAttrSet();
    }
}

void SwUndoDrawGroup::AddObj( sal_uInt32 nPos, SwDrawFrmFmt* pFmt, SdrObject* pObj )
{
    SwUndoGroupObjImpl& rSave = *( pObjArr + nPos + 1 );
    rSave.pObj = pObj;
    rSave.pFmt = pFmt;
    ::lcl_SaveAnchor( pFmt, rSave.nNodeIdx );

       // notify UNO objects to decouple
    ::lcl_SendRemoveToUno( *pFmt );

    // remove from array
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
    : SwUndo( UNDO_DRAWUNGROUP ), bDelFmt( false )
{
    nSize = pObj->getChildrenOfSdrObject()->GetObjCount() + 1;
    pObjArr = new SwUndoGroupObjImpl[ nSize ];

    SwDrawContact *pContact = (SwDrawContact*)findConnectionToSdrObject(pObj);
    SwDrawFrmFmt* pFmt = (SwDrawFrmFmt*)pContact->GetFmt();

    pObjArr->pObj = pObj;
    pObjArr->pFmt = pFmt;

    // object will destroy itself
    pContact->HandleChanged(*pObj, HINT_SDROBJECTDYING);
    resetConnectionToSdrObject(pObj);

    ::lcl_SaveAnchor( pFmt, pObjArr->nNodeIdx );

       // notify UNO objects to decouple
    ::lcl_SendRemoveToUno( *pFmt );

    // remove from array
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pFmt->GetDoc()->GetSpzFrmFmts();
    rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));
}

SwUndoDrawUnGroup::~SwUndoDrawUnGroup()
{
    if( bDelFmt )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr + 1;
        for( sal_uInt32 n = 1; n < nSize; ++n, ++pTmp )
            delete pTmp->pFmt;
    }
    else
        delete pObjArr->pFmt;

    delete [] pObjArr;
}

void SwUndoDrawUnGroup::UndoImpl(::sw::UndoRedoContext & rContext)
{
    bDelFmt = true;

    SwDoc *const pDoc = & rContext.GetDoc();
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();

    // remove from array
    for( sal_uInt32 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        ::lcl_SaveAnchor( rSave.pFmt, rSave.nNodeIdx );

           // notify UNO objects to decouple
        ::lcl_SendRemoveToUno( *rSave.pFmt );

        rFlyFmts.Remove( rFlyFmts.GetPos( rSave.pFmt ));
    }

    // re-insert group object
    ::lcl_RestoreAnchor( pObjArr->pFmt, pObjArr->nNodeIdx );
    rFlyFmts.Insert( pObjArr->pFmt, rFlyFmts.Count() );

    SwDrawContact *pContact = new SwDrawContact( pObjArr->pFmt, pObjArr->pObj );
    pContact->ConnectToLayout();
    // #i45718# - follow-up of #i35635# move object to visible layer
    pContact->MoveObjToVisibleLayer( pObjArr->pObj );
    // #i45952# - notify that position attributes are already set
    ASSERT( dynamic_cast< SwDrawFrmFmt* >(pObjArr->pFmt),
            "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object" );
    if ( dynamic_cast< SwDrawFrmFmt* >(pObjArr->pFmt) )
    {
        static_cast<SwDrawFrmFmt*>(pObjArr->pFmt)->PosAttrSet();
    }
}

void SwUndoDrawUnGroup::RedoImpl(::sw::UndoRedoContext &)
{
    bDelFmt = false;

    // save group object
    SwDrawFrmFmt* pFmt = pObjArr->pFmt;
    SwDrawContact* pContact = (SwDrawContact*)pFmt->FindContactObj();

        // object will destroy itself
    pContact->HandleChanged(*pObjArr->pObj, HINT_SDROBJECTDYING);
    resetConnectionToSdrObject(pObjArr->pObj);

    ::lcl_SaveAnchor( pFmt, pObjArr->nNodeIdx );

       // notify UNO objects to decouple
    ::lcl_SendRemoveToUno( *pFmt );

    // remove from array
    SwDoc* pDoc = pFmt->GetDoc();
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();
    rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));

    for( sal_uInt32 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        ::lcl_RestoreAnchor( rSave.pFmt, rSave.nNodeIdx );
        rFlyFmts.Insert( rSave.pFmt, rFlyFmts.Count() );

        // #i45952# - notify that position attributes are already set
        ASSERT( dynamic_cast< SwDrawFrmFmt* >(rSave.pFmt),
                "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object" );
        if ( dynamic_cast< SwDrawFrmFmt* >(rSave.pFmt) )
        {
            static_cast<SwDrawFrmFmt*>(rSave.pFmt)->PosAttrSet();
        }
    }
}

void SwUndoDrawUnGroup::AddObj( sal_uInt32 nPos, SwDrawFrmFmt* pFmt )
{
    SwUndoGroupObjImpl& rSave = *( pObjArr + nPos + 1 );
    rSave.pFmt = pFmt;
    rSave.pObj = 0;
}

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
    for ( SdrObjectVector::size_type i = 0; i < aDrawFmtsAndObjs.size(); ++i )
    {
        SdrObject* pObj( aDrawFmtsAndObjs[i].second );
        SwDrawContact* pDrawContact( dynamic_cast<SwDrawContact*>(findConnectionToSdrObjectDirect(pObj)) );
        ASSERT( pDrawContact,
                "<SwUndoDrawUnGroupConnectToLayout::Undo(..)> -- missing SwDrawContact instance" );
        if ( pDrawContact )
        {
            // deletion of instance <pDrawContact> and thus disconnection from
            // the Writer layout.
            pDrawContact->HandleChanged(*pObj, HINT_SDROBJECTDYING);
            resetConnectionToSdrObject(pObj);
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

//-------------------------------------

SwUndoDrawDelete::SwUndoDrawDelete( sal_uInt32 nCnt )
:   SwUndo( UNDO_DRAWDELETE ),
    pObjArr(0),
    maSdrObjectVector(),
    nSize( nCnt ),
    bDelFmt( true )
{
    pObjArr = new SwUndoGroupObjImpl[ nSize ];
}

SwUndoDrawDelete::~SwUndoDrawDelete()
{
    if( bDelFmt )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr;
        for( sal_uInt32 n = 0; n < maSdrObjectVector.size(); ++n, ++pTmp )
            delete pTmp->pFmt;
    }
    delete [] pObjArr;
    maSdrObjectVector.clear();
}

void SwUndoDrawDelete::UndoImpl(::sw::UndoRedoContext & rContext)
{
    bDelFmt = false;
    SwSpzFrmFmts & rFlyFmts = *rContext.GetDoc().GetSpzFrmFmts();
    for( sal_uInt32 n = 0; n < maSdrObjectVector.size(); ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );
        ::lcl_RestoreAnchor( rSave.pFmt, rSave.nNodeIdx );
        rFlyFmts.Insert( rSave.pFmt, rFlyFmts.Count() );
        SdrObject *pObj = rSave.pObj;
        SwDrawContact *pContact = new SwDrawContact( rSave.pFmt, pObj );
        pContact->_Changed( *pObj, HINT_OBJCHG_RESIZE, false );
        // #i45718# - follow-up of #i35635# move object to visible layer
        pContact->MoveObjToVisibleLayer( pObj );
        // #i45952# - notify that position attributes are already set
        ASSERT( dynamic_cast< SwDrawFrmFmt* >(rSave.pFmt),
                "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object" );
        if ( dynamic_cast< SwDrawFrmFmt* >(rSave.pFmt) )
        {
            static_cast<SwDrawFrmFmt*>(rSave.pFmt)->PosAttrSet();
        }
        // <--
    }
    rContext.SetSelections(0, maSdrObjectVector);
}

void SwUndoDrawDelete::RedoImpl(::sw::UndoRedoContext & rContext)
{
    bDelFmt = true;
    SwSpzFrmFmts & rFlyFmts = *rContext.GetDoc().GetSpzFrmFmts();
    for( sal_uInt32 n = 0; n < maSdrObjectVector.size(); ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );
        SdrObject *pObj = rSave.pObj;
        SwDrawContact *pContact = (SwDrawContact*)findConnectionToSdrObject(pObj);
        SwDrawFrmFmt *pFmt = (SwDrawFrmFmt*)pContact->GetFmt();
        // object will destroy itself
        pContact->HandleChanged(*pObj, HINT_SDROBJECTDYING);
        resetConnectionToSdrObject(pObj);

           // notify UNO objects to decouple
        ::lcl_SendRemoveToUno( *pFmt );

        rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));
        ::lcl_SaveAnchor( pFmt, rSave.nNodeIdx );
    }
}

void SwUndoDrawDelete::AddObj( sal_uInt32 , SwDrawFrmFmt* pFmt, const SdrObject& rSdrObject )
{
    SwUndoGroupObjImpl& rSave = *( pObjArr + maSdrObjectVector.size() );
    rSave.pObj = const_cast< SdrObject* >(&rSdrObject);
    rSave.pFmt = pFmt;
    ::lcl_SaveAnchor( pFmt, rSave.nNodeIdx );

       // notify UNO objects to decouple
    ::lcl_SendRemoveToUno( *pFmt );

    // remove from array
    SwDoc* pDoc = pFmt->GetDoc();
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();
    rFlyFmts.Remove( rFlyFmts.GetPos( pFmt ));

    maSdrObjectVector.push_back( const_cast< SdrObject* >(&rSdrObject) );
}

