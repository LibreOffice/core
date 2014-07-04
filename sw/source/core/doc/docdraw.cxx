/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <hintids.hxx>
#include <vcl/outdev.hxx>
#include <sfx2/printer.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/editeng.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/colritem.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdogrp.hxx>
#include <editeng/langitem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/measfld.hxx>
#include <svx/svdpool.hxx>
#include <fmtanchr.hxx>
#include <charatr.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <viewimp.hxx>
#include <swhints.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docsh.hxx>
#include <rootfrm.hxx>
#include <poolfmt.hxx>
#include <viewsh.hxx>
#include <drawdoc.hxx>
#include <UndoDraw.hxx>
#include <swundo.hxx>
#include <dcontact.hxx>
#include <dview.hxx>
#include <mvsave.hxx>
#include <flyfrm.hxx>
#include <dflyobj.hxx>
#include <svx/svdetc.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/svdpagv.hxx>
#include <txtfrm.hxx>
#include <editeng/frmdiritem.hxx>
#include <fmtornt.hxx>
#include <svx/svditer.hxx>

#include <vector>
#include <switerator.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;

/** local method to determine positioning and alignment attributes for a drawing
 *  object, which is newly connected to the layout.
 *
 * Used for a newly formed group object <SwDoc::GroupSelection(..)>
 * and the members of a destroyed group <SwDoc::UnGroupSelection(..)>
 */
static void lcl_AdjustPositioningAttr( SwDrawFrmFmt* _pFrmFmt,
                                const SdrObject& _rSdrObj )
{
    const SwContact* pContact = GetUserCall( &_rSdrObj );
    OSL_ENSURE( pContact, "<lcl_AdjustPositioningAttr(..)> - missing contact object." );

    // determine position of new group object relative to its anchor frame position
    SwTwips nHoriRelPos = 0;
    SwTwips nVertRelPos = 0;
    {
        const SwFrm* pAnchorFrm = pContact->GetAnchoredObj( &_rSdrObj )->GetAnchorFrm();
        OSL_ENSURE( !pAnchorFrm ||
                !pAnchorFrm->IsTxtFrm() ||
                !static_cast<const SwTxtFrm*>(pAnchorFrm)->IsFollow(),
                "<lcl_AdjustPositioningAttr(..)> - anchor frame is a follow. Please inform OD." );
        bool bVert = false;
        bool bR2L = false;
        // #i45952# - use anchor position of anchor frame, if it exist.
        Point aAnchorPos;
        if ( pAnchorFrm )
        {
            // #i45952#
            aAnchorPos = pAnchorFrm->GetFrmAnchorPos( ::HasWrap( &_rSdrObj ) );
            bVert = pAnchorFrm->IsVertical();
            bR2L = pAnchorFrm->IsRightToLeft();
        }
        else
        {
            // #i45952#
            aAnchorPos = _rSdrObj.GetAnchorPos();
            // If no anchor frame exist - e.g. because no layout exists - the
            // default layout direction is taken.
            const SvxFrameDirectionItem* pDirItem =
                static_cast<const SvxFrameDirectionItem*>(&(_pFrmFmt->GetAttrSet().GetPool()->GetDefaultItem( RES_FRAMEDIR )));
            switch ( pDirItem->GetValue() )
            {
                case FRMDIR_VERT_TOP_LEFT:
                {
                    // vertical from left-to-right
                    bVert = true;
                    bR2L = true;
                    OSL_FAIL( "<lcl_AdjustPositioningAttr(..)> - vertical from left-to-right not supported." );
                }
                break;
                case FRMDIR_VERT_TOP_RIGHT:
                {
                    // vertical from right-to-left
                    bVert = true;
                    bR2L = false;
                }
                break;
                case FRMDIR_HORI_RIGHT_TOP:
                {
                    // horizontal from right-to-left
                    bVert = false;
                    bR2L = true;
                }
                break;
                case FRMDIR_HORI_LEFT_TOP:
                {
                    // horizontal from left-to-right
                    bVert = false;
                    bR2L = false;
                }
                break;
            }

        }
        // use geometry of drawing object
        const SwRect aObjRect = _rSdrObj.GetSnapRect();

        if ( bVert )
        {
            if ( bR2L ) {
                  //FRMDIR_VERT_TOP_LEFT
                  nHoriRelPos = aObjRect.Left() - aAnchorPos.getX();
                  nVertRelPos = aObjRect.Top() - aAnchorPos.getY();
            } else {
                //FRMDIR_VERT_TOP_RIGHT
                nHoriRelPos = aObjRect.Top() - aAnchorPos.getY();
                nVertRelPos = aAnchorPos.getX() - aObjRect.Right();
            }
        }
        else if ( bR2L )
        {
            nHoriRelPos = aAnchorPos.getX() - aObjRect.Right();
            nVertRelPos = aObjRect.Top() - aAnchorPos.getY();
        }
        else
        {
            nHoriRelPos = aObjRect.Left() - aAnchorPos.getX();
            nVertRelPos = aObjRect.Top() - aAnchorPos.getY();
        }
    }

    _pFrmFmt->SetFmtAttr( SwFmtHoriOrient( nHoriRelPos, text::HoriOrientation::NONE, text::RelOrientation::FRAME ) );
    _pFrmFmt->SetFmtAttr( SwFmtVertOrient( nVertRelPos, text::VertOrientation::NONE, text::RelOrientation::FRAME ) );
    // #i44334#, #i44681# - positioning attributes already set
    _pFrmFmt->PosAttrSet();
    // #i34750# - keep current object rectangle for  drawing
    // objects. The object rectangle is used on events from the drawing layer
    // to adjust the positioning attributes - see <SwDrawContact::_Changed(..)>.
    {
        const SwAnchoredObject* pAnchoredObj = pContact->GetAnchoredObj( &_rSdrObj );
        if ( pAnchoredObj->ISA(SwAnchoredDrawObject) )
        {
            const SwAnchoredDrawObject* pAnchoredDrawObj =
                            static_cast<const SwAnchoredDrawObject*>(pAnchoredObj);
            const SwRect aObjRect = _rSdrObj.GetSnapRect();
            const_cast<SwAnchoredDrawObject*>(pAnchoredDrawObj)
                                        ->SetLastObjRect( aObjRect.SVRect() );
        }
    }
}

SwDrawContact* SwDoc::GroupSelection( SdrView& rDrawView )
{
    // replace marked 'virtual' drawing objects by the corresponding 'master'
    // drawing objects.
    SwDrawView::ReplaceMarkedDrawVirtObjs( rDrawView );

    const SdrMarkList &rMrkList = rDrawView.GetMarkedObjectList();
    SwDrawFrmFmt *pFmt = 0L;
    SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
    bool bNoGroup = ( 0 == pObj->GetUpGroup() );
    SwDrawContact* pNewContact = 0;
    if( bNoGroup )
    {
        // Revoke anchor attribute.
        SwDrawContact *pMyContact = (SwDrawContact*)GetUserCall(pObj);
        const SwFmtAnchor aAnch( pMyContact->GetFmt()->GetAnchor() );

        SwUndoDrawGroup *const pUndo = (!GetIDocumentUndoRedo().DoesUndo())
                                 ? 0
                                 : new SwUndoDrawGroup( (sal_uInt16)rMrkList.GetMarkCount() );

        // #i53320#
        bool bGroupMembersNotPositioned( false );
        {
            SwAnchoredDrawObject* pAnchoredDrawObj =
                static_cast<SwAnchoredDrawObject*>(pMyContact->GetAnchoredObj( pObj ));
            bGroupMembersNotPositioned = pAnchoredDrawObj->NotYetPositioned();
        }
        // Destroy ContactObjects and formats.
        for( sal_uInt16 i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);

            // #i53320#
#if OSL_DEBUG_LEVEL > 0
            SwAnchoredDrawObject* pAnchoredDrawObj =
                static_cast<SwAnchoredDrawObject*>(pContact->GetAnchoredObj( pObj ));
            OSL_ENSURE( bGroupMembersNotPositioned == pAnchoredDrawObj->NotYetPositioned(),
                    "<SwDoc::GroupSelection(..)> - group members have different positioning status!" );
#endif

            pFmt = (SwDrawFrmFmt*)pContact->GetFmt();
            // Deletes itself!
            pContact->Changed(*pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
            pObj->SetUserCall( 0 );

            if( pUndo )
                pUndo->AddObj( i, pFmt, pObj );
            else
                DelFrmFmt( pFmt );

            // #i45952# - re-introduce position normalization of group member
            // objects, because its anchor position is cleared, when they are
            // grouped.
            Point aAnchorPos( pObj->GetAnchorPos() );
            pObj->NbcSetAnchorPos( Point( 0, 0 ) );
            pObj->NbcMove( Size( aAnchorPos.getX(), aAnchorPos.getY() ) );
        }

        pFmt = MakeDrawFrmFmt( OUString("DrawObject"),
                                GetDfltFrmFmt() );
        pFmt->SetFmtAttr( aAnch );
        // #i36010# - set layout direction of the position
        pFmt->SetPositionLayoutDir(
            text::PositionLayoutDir::PositionInLayoutDirOfAnchor );

        rDrawView.GroupMarked();
        OSL_ENSURE( rMrkList.GetMarkCount() == 1, "GroupMarked more or none groups." );

        SdrObject* pNewGroupObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
        pNewContact = new SwDrawContact( pFmt, pNewGroupObj );
        // #i35635#
        pNewContact->MoveObjToVisibleLayer( pNewGroupObj );
        pNewContact->ConnectToLayout();
        // #i53320# - No adjustment of the positioning and alignment
        // attributes, if group members aren't positioned yet.
        if ( !bGroupMembersNotPositioned )
        {
            // #i26791# - Adjust positioning and alignment attributes.
            lcl_AdjustPositioningAttr( pFmt, *pNewGroupObj );
        }

        if( pUndo )
        {
            pUndo->SetGroupFmt( pFmt );
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
    }
    else
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().ClearRedo();
        }

        rDrawView.GroupMarked();
        OSL_ENSURE( rMrkList.GetMarkCount() == 1, "GroupMarked more or none groups." );
    }

    return pNewContact;
}

void SwDoc::UnGroupSelection( SdrView& rDrawView )
{
    bool const bUndo = GetIDocumentUndoRedo().DoesUndo();
    if( bUndo )
    {
        GetIDocumentUndoRedo().ClearRedo();
    }

    // replace marked 'virtual' drawing objects by the corresponding 'master'
    // drawing objects.
    SwDrawView::ReplaceMarkedDrawVirtObjs( rDrawView );

    const SdrMarkList &rMrkList = rDrawView.GetMarkedObjectList();
    std::vector< std::pair< SwDrawFrmFmt*, SdrObject* > >* pFmtsAndObjs( 0L );
    const sal_uInt32 nMarkCount( rMrkList.GetMarkCount() );
    if ( nMarkCount )
    {
        pFmtsAndObjs = new std::vector< std::pair< SwDrawFrmFmt*, SdrObject* > >[nMarkCount];
        SdrObject *pMyObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
        if( !pMyObj->GetUpGroup() )
        {
            OUString sDrwFmtNm("DrawObject");
            for ( sal_uInt16 i = 0; i < nMarkCount; ++i )
            {
                SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
                if ( pObj->IsA( TYPE(SdrObjGroup) ) )
                {
                    SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
                    SwFmtAnchor aAnch( pContact->GetFmt()->GetAnchor() );
                    SdrObjList *pLst = ((SdrObjGroup*)pObj)->GetSubList();

                    SwUndoDrawUnGroup* pUndo = 0;
                    if( bUndo )
                    {
                        pUndo = new SwUndoDrawUnGroup( (SdrObjGroup*)pObj );
                        GetIDocumentUndoRedo().AppendUndo(pUndo);
                    }

                    for ( sal_uInt16 i2 = 0; i2 < pLst->GetObjCount(); ++i2 )
                    {
                        SdrObject* pSubObj = pLst->GetObj( i2 );
                        SwDrawFrmFmt *pFmt = MakeDrawFrmFmt( sDrwFmtNm,
                                                            GetDfltFrmFmt() );
                        pFmt->SetFmtAttr( aAnch );
                        // #i36010# - set layout direction of the position
                        pFmt->SetPositionLayoutDir(
                            text::PositionLayoutDir::PositionInLayoutDirOfAnchor );
                        pFmtsAndObjs[i].push_back( std::pair< SwDrawFrmFmt*, SdrObject* >( pFmt, pSubObj ) );

                        if( bUndo )
                            pUndo->AddObj( i2, pFmt );
                    }
                }
            }
        }
    }
    rDrawView.UnGroupMarked();
    // creation of <SwDrawContact> instances for the former group members and
    // its connection to the Writer layout.
    for ( sal_uInt32 i = 0; i < nMarkCount; ++i )
    {
        SwUndoDrawUnGroupConnectToLayout* pUndo = 0;
        if( bUndo )
        {
            pUndo = new SwUndoDrawUnGroupConnectToLayout();
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        while ( pFmtsAndObjs[i].size() > 0 )
        {
            SwDrawFrmFmt* pFmt( pFmtsAndObjs[i].back().first );
            SdrObject* pObj( pFmtsAndObjs[i].back().second );
            pFmtsAndObjs[i].pop_back();

            SwDrawContact* pContact = new SwDrawContact( pFmt, pObj );
            pContact->MoveObjToVisibleLayer( pObj );
            pContact->ConnectToLayout();
            lcl_AdjustPositioningAttr( pFmt, *pObj );

            if ( bUndo )
            {
                pUndo->AddFmtAndObj( pFmt, pObj );
            }
        }
    }
    delete [] pFmtsAndObjs;
}

bool SwDoc::DeleteSelection( SwDrawView& rDrawView )
{
    bool bCallBase = false;
    const SdrMarkList &rMrkList = rDrawView.GetMarkedObjectList();
    if( rMrkList.GetMarkCount() )
    {
        GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);
        sal_uInt16 i;
        bool bDelMarked = true;

        if( 1 == rMrkList.GetMarkCount() )
        {
            SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
            if( pObj->ISA(SwVirtFlyDrawObj) )
            {
                SwFlyFrmFmt* pFrmFmt = (SwFlyFrmFmt*)
                    ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->GetFmt();
                if( pFrmFmt )
                {
                    DelLayoutFmt( pFrmFmt );
                    bDelMarked = false;
                }
            }
        }

        for( i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            if( !pObj->ISA(SwVirtFlyDrawObj) )
            {
                SwDrawContact *pC = (SwDrawContact*)GetUserCall(pObj);
                SwDrawFrmFmt *pFrmFmt = (SwDrawFrmFmt*)pC->GetFmt();
                if( pFrmFmt &&
                    FLY_AS_CHAR == pFrmFmt->GetAnchor().GetAnchorId() )
                {
                    rDrawView.MarkObj( pObj, rDrawView.Imp().GetPageView(), true );
                    --i;
                    DelLayoutFmt( pFrmFmt );
                }
            }
        }

        if( rMrkList.GetMarkCount() && bDelMarked )
        {
            SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
            if( !pObj->GetUpGroup() )
            {
                SwUndoDrawDelete *const pUndo =
                    (!GetIDocumentUndoRedo().DoesUndo())
                        ? 0
                            : new SwUndoDrawDelete( (sal_uInt16)rMrkList.GetMarkCount() );

                // Destroy ContactObjects, save formats.
                for( i = 0; i < rMrkList.GetMarkCount(); ++i )
                {
                    const SdrMark& rMark = *rMrkList.GetMark( i );
                    pObj = rMark.GetMarkedSdrObj();
                    SwDrawContact *pContact = (SwDrawContact*)pObj->GetUserCall();
                    if( pContact ) // of course not for grouped objects
                    {
                        SwDrawFrmFmt *pFmt = (SwDrawFrmFmt*)pContact->GetFmt();
                        // before delete of selection is performed, marked
                        // <SwDrawVirtObj>-objects have to be replaced by its
                        // reference objects.  Thus, assert, if a
                        // <SwDrawVirt>-object is found in the mark list.
                        if ( pObj->ISA(SwDrawVirtObj) )
                        {
                            OSL_FAIL( "<SwDrawVirtObj> is still marked for delete. application will crash!" );
                        }
                        // Deletes itself!
                        pContact->Changed(*pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
                        pObj->SetUserCall( 0 );

                        if( pUndo )
                            pUndo->AddObj( i, pFmt, rMark );
                        else
                            DelFrmFmt( pFmt );
                    }
                }

                if( pUndo )
                {
                    GetIDocumentUndoRedo().AppendUndo( pUndo );
                }
            }
            bCallBase = true;
        }
        SetModified();

        GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);
    }

    return bCallBase;
}

_ZSortFly::_ZSortFly( const SwFrmFmt* pFrmFmt, const SwFmtAnchor* pFlyAn,
                      sal_uInt32 nArrOrdNum )
    : pFmt( pFrmFmt ), pAnchor( pFlyAn ), nOrdNum( nArrOrdNum )
{
    // #i11176#
    // This also needs to work when no layout exists. Thus, for
    // FlyFrames an alternative method is used now in that case.
    if( RES_FLYFRMFMT == pFmt->Which() )
    {
        if( pFmt->getIDocumentLayoutAccess()->GetCurrentViewShell() )
        {
            // See if there is an SdrObject for it
            SwFlyFrm* pFly = SwIterator<SwFlyFrm,SwFmt>::FirstElement( *pFrmFmt );
            if( pFly )
                nOrdNum = pFly->GetVirtDrawObj()->GetOrdNum();
        }
        else
        {
            // See if there is an SdrObject for it
            SwFlyDrawContact* pContact = SwIterator<SwFlyDrawContact,SwFmt>::FirstElement( *pFrmFmt );
            if( pContact )
                nOrdNum = pContact->GetMaster()->GetOrdNum();
        }
    }
    else if( RES_DRAWFRMFMT == pFmt->Which() )
    {
        // See if there is an SdrObject for it
        SwDrawContact* pContact = SwIterator<SwDrawContact,SwFmt>::FirstElement( *pFrmFmt );
        if( pContact )
            nOrdNum = pContact->GetMaster()->GetOrdNum();
    }
    else {
        OSL_ENSURE( !this, "what is that for a format?" );
    }
}

#include <svx/sxenditm.hxx>

// Is also called by the Sw3 Reader, if there was an error when reading the
// drawing layer. If it is called by the Sw3 Reader the layer is rebuilt
// from scratch.
void SwDoc::InitDrawModel()
{
    // !! Attention: there is similar code in the Sw3 Reader (sw3imp.cxx) that
    // also has to be maintained!!
    if ( mpDrawModel )
        ReleaseDrawModel();

    // Setup DrawPool and EditEnginePool. Ownership is ours and only gets passed
    // to the Drawing.
    // The pools are destroyed in the ReleaseDrawModel.
    // for loading the drawing items. This must be loaded without RefCounts!
    SfxItemPool *pSdrPool = new SdrItemPool( &GetAttrPool() );
    // change DefaultItems for the SdrEdgeObj distance items to TWIPS.
    if(pSdrPool)
    {
        const long nDefEdgeDist = ((500 * 72) / 127); // 1/100th mm in twips
        pSdrPool->SetPoolDefaultItem(SdrEdgeNode1HorzDistItem(nDefEdgeDist));
        pSdrPool->SetPoolDefaultItem(SdrEdgeNode1VertDistItem(nDefEdgeDist));
        pSdrPool->SetPoolDefaultItem(SdrEdgeNode2HorzDistItem(nDefEdgeDist));
        pSdrPool->SetPoolDefaultItem(SdrEdgeNode2VertDistItem(nDefEdgeDist));

        // #i33700#
        // Set shadow distance defaults as PoolDefaultItems. Details see bug.
        pSdrPool->SetPoolDefaultItem(SdrShadowXDistItem((300 * 72) / 127));
        pSdrPool->SetPoolDefaultItem(SdrShadowYDistItem((300 * 72) / 127));
    }
    SfxItemPool *pEEgPool = EditEngine::CreatePool( false );
    pSdrPool->SetSecondaryPool( pEEgPool );
     if ( !GetAttrPool().GetFrozenIdRanges () )
        GetAttrPool().FreezeIdRanges();
    else
        pSdrPool->FreezeIdRanges();

    // set FontHeight pool defaults without changing static SdrEngineDefaults
    GetAttrPool().SetPoolDefaultItem(SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT ));

    SAL_INFO( "sw.doc", "before create DrawDocument" );
    // The document owns the SdrModel. We always have two layers and one page.
    mpDrawModel = new SwDrawDocument( this );

    mpDrawModel->EnableUndo( GetIDocumentUndoRedo().DoesUndo() );

    OUString sLayerNm;
    sLayerNm = "Hell";
    mnHell   = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    sLayerNm = "Heaven";
    mnHeaven = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    sLayerNm = "Controls";
    mnControls = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    // add invisible layers corresponding to the visible ones.
    {
        sLayerNm = "InvisibleHell";
        mnInvisibleHell   = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

        sLayerNm = "InvisibleHeaven";
        mnInvisibleHeaven = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

        sLayerNm = "InvisibleControls";
        mnInvisibleControls = mpDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();
    }

    SdrPage* pMasterPage = mpDrawModel->AllocPage( false );
    mpDrawModel->InsertPage( pMasterPage );
    SAL_INFO( "sw.doc", "after create DrawDocument" );
    SAL_INFO( "sw.doc", "before create Spellchecker/Hyphenator" );
    SdrOutliner& rOutliner = mpDrawModel->GetDrawOutliner();
    uno::Reference< XSpellChecker1 > xSpell = ::GetSpellChecker();
    rOutliner.SetSpeller( xSpell );
    uno::Reference<XHyphenator> xHyphenator( ::GetHyphenator() );
    rOutliner.SetHyphenator( xHyphenator );
    SAL_INFO( "sw.doc", "after create Spellchecker/Hyphenator" );
    SetCalcFieldValueHdl(&rOutliner);
    SetCalcFieldValueHdl(&mpDrawModel->GetHitTestOutliner());

    // Set the LinkManager in the model so that linked graphics can be inserted.
    // The WinWord import needs it too.
    mpDrawModel->SetLinkManager( &GetLinkManager() );
    mpDrawModel->SetAddExtLeading( get(IDocumentSettingAccess::ADD_EXT_LEADING) );

    OutputDevice* pRefDev = getReferenceDevice( false );
    if ( pRefDev )
        mpDrawModel->SetRefDevice( pRefDev );

    mpDrawModel->SetNotifyUndoActionHdl( LINK( this, SwDoc, AddDrawUndo ));
    if ( mpCurrentView )
    {
        SwViewShell* pViewSh = mpCurrentView;
        do
        {
            SwRootFrm* pRoot =  pViewSh->GetLayout();
            if( pRoot && !pRoot->GetDrawPage() )
            {
                // Disable "multiple layout" for the moment:
                // use pMasterPage instead of a new created SdrPage
                // mpDrawModel->AllocPage( FALSE );
                // mpDrawModel->InsertPage( pDrawPage );
                SdrPage* pDrawPage = pMasterPage;
                pRoot->SetDrawPage( pDrawPage );
                pDrawPage->SetSize( pRoot->Frm().SSize() );
            }
            pViewSh = (SwViewShell*)pViewSh->GetNext();
        }while( pViewSh != mpCurrentView );
    }
}

/** method to notify drawing page view about the invisible layers */
void SwDoc::NotifyInvisibleLayers( SdrPageView& _rSdrPageView )
{
    OUString sLayerNm;
    sLayerNm = "InvisibleHell";
    _rSdrPageView.SetLayerVisible( sLayerNm, false );

    sLayerNm = "InvisibleHeaven";
    _rSdrPageView.SetLayerVisible( sLayerNm, false );

    sLayerNm = "InvisibleControls";
    _rSdrPageView.SetLayerVisible( sLayerNm, false );
}

/** method to determine, if a layer ID belongs to the visible ones.

    @note If given layer ID is unknown, method asserts and returns <false>.
*/
bool SwDoc::IsVisibleLayerId( const SdrLayerID& _nLayerId ) const
{
    bool bRetVal;

    if ( _nLayerId == GetHeavenId() ||
         _nLayerId == GetHellId() ||
         _nLayerId == GetControlsId() )
    {
        bRetVal = true;
    }
    else if ( _nLayerId == GetInvisibleHeavenId() ||
              _nLayerId == GetInvisibleHellId() ||
              _nLayerId == GetInvisibleControlsId() )
    {
        bRetVal = false;
    }
    else
    {
        OSL_FAIL( "<SwDoc::IsVisibleLayerId(..)> - unknown layer ID." );
        bRetVal = false;
    }

    return bRetVal;
}

/** method to determine, if the corresponding visible layer ID for a invisible one.

    @note If given layer ID is a visible one, method returns given layer ID.
    @note If given layer ID is unknown, method returns given layer ID.
*/
SdrLayerID SwDoc::GetVisibleLayerIdByInvisibleOne( const SdrLayerID& _nInvisibleLayerId )
{
    SdrLayerID nVisibleLayerId;

    if ( _nInvisibleLayerId == GetInvisibleHeavenId() )
    {
        nVisibleLayerId = GetHeavenId();
    }
    else if ( _nInvisibleLayerId == GetInvisibleHellId() )
    {
        nVisibleLayerId = GetHellId();
    }
    else if ( _nInvisibleLayerId == GetInvisibleControlsId() )
    {
        nVisibleLayerId = GetControlsId();
    }
    else if ( _nInvisibleLayerId == GetHeavenId() ||
              _nInvisibleLayerId == GetHellId() ||
              _nInvisibleLayerId == GetControlsId() )
    {
        OSL_FAIL( "<SwDoc::GetVisibleLayerIdByInvisibleOne(..)> - given layer ID already an invisible one." );
        nVisibleLayerId = _nInvisibleLayerId;
    }
    else
    {
        OSL_FAIL( "<SwDoc::GetVisibleLayerIdByInvisibleOne(..)> - given layer ID is unknown." );
        nVisibleLayerId = _nInvisibleLayerId;
    }

    return nVisibleLayerId;
}

/** method to determine, if the corresponding invisible layer ID for a visible one.

    @note If given layer ID is a invisible one, method returns given layer ID.
    @note If given layer ID is unknown, method returns given layer ID.
*/
SdrLayerID SwDoc::GetInvisibleLayerIdByVisibleOne( const SdrLayerID& _nVisibleLayerId )
{
    SdrLayerID nInvisibleLayerId;

    if ( _nVisibleLayerId == GetHeavenId() )
    {
        nInvisibleLayerId = GetInvisibleHeavenId();
    }
    else if ( _nVisibleLayerId == GetHellId() )
    {
        nInvisibleLayerId = GetInvisibleHellId();
    }
    else if ( _nVisibleLayerId == GetControlsId() )
    {
        nInvisibleLayerId = GetInvisibleControlsId();
    }
    else if ( _nVisibleLayerId == GetInvisibleHeavenId() ||
              _nVisibleLayerId == GetInvisibleHellId() ||
              _nVisibleLayerId == GetInvisibleControlsId() )
    {
        OSL_FAIL( "<SwDoc::GetInvisibleLayerIdByVisibleOne(..)> - given layer ID already an invisible one." );
        nInvisibleLayerId = _nVisibleLayerId;
    }
    else
    {
        OSL_FAIL( "<SwDoc::GetInvisibleLayerIdByVisibleOne(..)> - given layer ID is unknown." );
        nInvisibleLayerId = _nVisibleLayerId;
    }

    return nInvisibleLayerId;
}

void SwDoc::ReleaseDrawModel()
{
    if ( mpDrawModel )
    {
        // !! Also maintain the code in the sw3io for inserting documents!!

        delete mpDrawModel; mpDrawModel = 0;
        SfxItemPool *pSdrPool = GetAttrPool().GetSecondaryPool();

        OSL_ENSURE( pSdrPool, "missing pool" );
        SfxItemPool *pEEgPool = pSdrPool->GetSecondaryPool();
        OSL_ENSURE( !pEEgPool->GetSecondaryPool(), "I don't accept additional pools");
        pSdrPool->Delete();                 // First have the items destroyed,
                                            // then destroy the chain!
        GetAttrPool().SetSecondaryPool( 0 );    // This one's a must!
        pSdrPool->SetSecondaryPool( 0 );    // That one's safer
        SfxItemPool::Free(pSdrPool);
        SfxItemPool::Free(pEEgPool);
    }
}

SdrModel* SwDoc::_MakeDrawModel()
{
    OSL_ENSURE( !mpDrawModel, "_MakeDrawModel: Why?" );
    InitDrawModel();
    if ( mpCurrentView )
    {
        SwViewShell* pTmp = mpCurrentView;
        do
        {
            pTmp->MakeDrawView();
            pTmp = (SwViewShell*) pTmp->GetNext();
        } while ( pTmp != mpCurrentView );

        // Broadcast, so that the FormShell can be connected to the DrawView
        if( GetDocShell() )
        {
            SfxSimpleHint aHnt( SW_BROADCAST_DRAWVIEWS_CREATED );
            GetDocShell()->Broadcast( aHnt );
        }
    }
    return mpDrawModel;
}

void SwDoc::DrawNotifyUndoHdl()
{
    mpDrawModel->SetNotifyUndoActionHdl( Link() );
}

/// In the Outliner, set a link to the method for field display in edit objects.
void SwDoc::SetCalcFieldValueHdl(Outliner* pOutliner)
{
    pOutliner->SetCalcFieldValueHdl(LINK(this, SwDoc, CalcFieldValueHdl));
}

/// Recognise fields/URLs in the Outliner and set how they are displayed.
IMPL_LINK(SwDoc, CalcFieldValueHdl, EditFieldInfo*, pInfo)
{
    if (pInfo)
    {
        const SvxFieldItem& rField = pInfo->GetField();
        const SvxFieldData* pField = rField.GetField();

        if (pField && pField->ISA(SvxDateField))
        {
            // Date field
            pInfo->SetRepresentation(
                ((const SvxDateField*) pField)->GetFormatted(
                        *GetNumberFormatter( true ), LANGUAGE_SYSTEM) );
        }
        else if (pField && pField->ISA(SvxURLField))
        {
            // URL field
            switch ( ((const SvxURLField*) pField)->GetFormat() )
            {
                case SVXURLFORMAT_APPDEFAULT: //!!! Can be set in App???
                case SVXURLFORMAT_REPR:
                {
                    pInfo->SetRepresentation(
                        ((const SvxURLField*)pField)->GetRepresentation());
                }
                break;

                case SVXURLFORMAT_URL:
                {
                    pInfo->SetRepresentation(
                        ((const SvxURLField*)pField)->GetURL());
                }
                break;
            }

            sal_uInt16 nChrFmt;

            if (IsVisitedURL(((const SvxURLField*)pField)->GetURL()))
                nChrFmt = RES_POOLCHR_INET_VISIT;
            else
                nChrFmt = RES_POOLCHR_INET_NORMAL;

            SwFmt *pFmt = GetCharFmtFromPool(nChrFmt);

            Color aColor(COL_LIGHTBLUE);
            if (pFmt)
                aColor = pFmt->GetColor().GetValue();

            pInfo->SetTxtColor(aColor);
        }
        else if (pField && pField->ISA(SdrMeasureField))
        {
            // Measure field
            pInfo->ClearFldColor();
        }
        else if ( pField && pField->ISA(SvxExtTimeField))
        {
            // Time field
            pInfo->SetRepresentation(
                ((const SvxExtTimeField*) pField)->GetFormatted(
                        *GetNumberFormatter( true ), LANGUAGE_SYSTEM) );
        }
        else
        {
            OSL_FAIL("unknown field command");
            pInfo->SetRepresentation( OUString( '?' ) );
        }
    }

    return(0);
}

/* TFFDI: The functions formerly declared 'inline'
 */
const SdrModel* SwDoc::GetDrawModel() const { return mpDrawModel; }
SdrModel* SwDoc::GetDrawModel() { return mpDrawModel; }
SdrLayerID SwDoc::GetHeavenId() const { return mnHeaven; }
SdrLayerID SwDoc::GetHellId() const { return mnHell; }
SdrLayerID SwDoc::GetControlsId() const { return mnControls;   }
SdrLayerID SwDoc::GetInvisibleHeavenId() const { return mnInvisibleHeaven; }
SdrLayerID SwDoc::GetInvisibleHellId() const { return mnInvisibleHell; }
SdrLayerID SwDoc::GetInvisibleControlsId() const { return mnInvisibleControls; }
SdrModel* SwDoc::GetOrCreateDrawModel() { return GetDrawModel() ? GetDrawModel() : _MakeDrawModel(); }

// #i62875#
namespace docfunc
{
    bool ExistsDrawObjs( SwDoc& p_rDoc )
    {
        bool bExistsDrawObjs( false );

        if ( p_rDoc.GetDrawModel() &&
             p_rDoc.GetDrawModel()->GetPage( 0 ) )
        {
            const SdrPage& rSdrPage( *(p_rDoc.GetDrawModel()->GetPage( 0 )) );

            SdrObjListIter aIter( rSdrPage, IM_FLAT );
            while( aIter.IsMore() )
            {
                SdrObject* pObj( aIter.Next() );
                if ( !dynamic_cast<SwVirtFlyDrawObj*>(pObj) &&
                     !dynamic_cast<SwFlyDrawObj*>(pObj) )
                {
                    bExistsDrawObjs = true;
                    break;
                }
            }
        }

        return bExistsDrawObjs;
    }

    bool AllDrawObjsOnPage( SwDoc& p_rDoc )
    {
        bool bAllDrawObjsOnPage( true );

        if ( p_rDoc.GetDrawModel() &&
             p_rDoc.GetDrawModel()->GetPage( 0 ) )
        {
            const SdrPage& rSdrPage( *(p_rDoc.GetDrawModel()->GetPage( 0 )) );

            SdrObjListIter aIter( rSdrPage, IM_FLAT );
            while( aIter.IsMore() )
            {
                SdrObject* pObj( aIter.Next() );
                if ( !dynamic_cast<SwVirtFlyDrawObj*>(pObj) &&
                     !dynamic_cast<SwFlyDrawObj*>(pObj) )
                {
                    SwDrawContact* pDrawContact =
                            dynamic_cast<SwDrawContact*>(::GetUserCall( pObj ));
                    if ( pDrawContact )
                    {
                        SwAnchoredDrawObject* pAnchoredDrawObj =
                            dynamic_cast<SwAnchoredDrawObject*>(pDrawContact->GetAnchoredObj( pObj ));

                        // error handling
                        {
                            if ( !pAnchoredDrawObj )
                            {
                                OSL_FAIL( "<docfunc::AllDrawObjsOnPage() - missing anchored draw object" );
                                bAllDrawObjsOnPage = false;
                                break;
                            }
                        }

                        if ( pAnchoredDrawObj->NotYetPositioned() )
                        {
                            // The drawing object isn't yet layouted.
                            // Thus, it isn't known, if all drawing objects are on page.
                            bAllDrawObjsOnPage = false;
                            break;
                        }
                        else if ( pAnchoredDrawObj->IsOutsidePage() )
                        {
                            bAllDrawObjsOnPage = false;
                            break;
                        }
                    }
                    else
                    {
                        // contact object of drawing object doesn't exists.
                        // Thus, the drawing object isn't yet positioned.
                        // Thus, it isn't known, if all drawing objects are on page.
                        bAllDrawObjsOnPage = false;
                        break;
                    }
                }
            }
        }

        return bAllDrawObjsOnPage;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
