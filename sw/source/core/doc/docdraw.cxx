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
#include <hintids.hxx>
#include <rtl/logfile.hxx>
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
#include <docsh.hxx>
#include <rootfrm.hxx>  //Damit der RootDtor gerufen wird.
#include <poolfmt.hxx>
#include <viewsh.hxx>           // fuer MakeDrawView
#include <drawdoc.hxx>
#include <undobj.hxx>
#include <swundo.hxx>           // fuer die UndoIds
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;


SV_IMPL_VARARR_SORT( _ZSortFlys, _ZSortFly )

/*************************************************************************
|*
|*  SwDoc::GroupSelection / SwDoc::UnGroupSelection
|*
|*************************************************************************/
// local method to determine positioning and
// alignment attributes for a drawing object, which is newly connected to
// the layout. Used for a newly formed group object <SwDoc::GroupSelection(..)>
// and the members of a destroyed group <SwDoc::UnGroupSelection(..)>
void lcl_AdjustPositioningAttr( SwDrawFrmFmt* _pFrmFmt,
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
                    // vertical from left-to-right - not supported yet
                    bVert = true;
                    bR2L = true;
                    OSL_ENSURE( false,
                            "<lcl_AdjustPositioningAttr(..)> - vertical from left-to-right not supported." );
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
            nHoriRelPos = aObjRect.Top() - aAnchorPos.Y();
            nVertRelPos = aAnchorPos.X() - aObjRect.Right();
        }
        else if ( bR2L )
        {
            nHoriRelPos = aAnchorPos.X() - aObjRect.Right();
            nVertRelPos = aObjRect.Top() - aAnchorPos.Y();
        }
        else
        {
            nHoriRelPos = aObjRect.Left() - aAnchorPos.X();
            nVertRelPos = aObjRect.Top() - aAnchorPos.Y();
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
    BOOL bNoGroup = ( 0 == pObj->GetUpGroup() );
    SwDrawContact* pNewContact = 0;
    if( bNoGroup )
    {
        //Ankerattribut aufheben.
        SwDrawContact *pMyContact = (SwDrawContact*)GetUserCall(pObj);
        const SwFmtAnchor aAnch( pMyContact->GetFmt()->GetAnchor() );

        SwUndoDrawGroup* pUndo = !DoesUndo()
                                 ? 0
                                 : new SwUndoDrawGroup( (USHORT)rMrkList.GetMarkCount() );

        // #i53320#
        bool bGroupMembersNotPositioned( false );
        {
            SwAnchoredDrawObject* pAnchoredDrawObj =
                static_cast<SwAnchoredDrawObject*>(pMyContact->GetAnchoredObj( pObj ));
            bGroupMembersNotPositioned = pAnchoredDrawObj->NotYetPositioned();
        }
        //ContactObjekte und Formate vernichten.
        for( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);

            // #i53320#
#if OSL_DEBUG_LEVEL > 1
            SwAnchoredDrawObject* pAnchoredDrawObj =
                static_cast<SwAnchoredDrawObject*>(pContact->GetAnchoredObj( pObj ));
            OSL_ENSURE( bGroupMembersNotPositioned == pAnchoredDrawObj->NotYetPositioned(),
                    "<SwDoc::GroupSelection(..)> - group members have different positioning status!" );
#endif

            pFmt = (SwDrawFrmFmt*)pContact->GetFmt();
            //loescht sich selbst!
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
            pObj->NbcMove( Size( aAnchorPos.X(), aAnchorPos.Y() ) );
        }

        pFmt = MakeDrawFrmFmt( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "DrawObject" )),
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
            ClearRedo();
            AppendUndo( pUndo );
        }
    }
    else
    {
        if ( DoesUndo() )
            ClearRedo();

        rDrawView.GroupMarked();
        OSL_ENSURE( rMrkList.GetMarkCount() == 1, "GroupMarked more or none groups." );
    }

    return pNewContact;
}


void SwDoc::UnGroupSelection( SdrView& rDrawView )
{
    const int bUndo = DoesUndo();
    if( bUndo )
        ClearRedo();

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
            String sDrwFmtNm( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM("DrawObject" )));
            for ( USHORT i = 0; i < nMarkCount; ++i )
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
                        AppendUndo( pUndo );
                    }

                    for ( USHORT i2 = 0; i2 < pLst->GetObjCount(); ++i2 )
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
            AppendUndo( pUndo );
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

/*************************************************************************
|*
|*  SwDoc::DeleteSelection()
|*
|*************************************************************************/

BOOL SwDoc::DeleteSelection( SwDrawView& rDrawView )
{
    BOOL bCallBase = FALSE;
    const SdrMarkList &rMrkList = rDrawView.GetMarkedObjectList();
    if( rMrkList.GetMarkCount() )
    {
        StartUndo(UNDO_EMPTY, NULL);
        USHORT i;
        BOOL bDelMarked = TRUE;

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
                    bDelMarked = FALSE;
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
                    rDrawView.MarkObj( pObj, rDrawView.Imp().GetPageView(), TRUE );
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
                SwUndoDrawDelete* pUndo = !DoesUndo() ? 0
                            : new SwUndoDrawDelete( (USHORT)rMrkList.GetMarkCount() );

                //ContactObjekte vernichten, Formate sicherstellen.
                for( i = 0; i < rMrkList.GetMarkCount(); ++i )
                {
                    const SdrMark& rMark = *rMrkList.GetMark( i );
                    pObj = rMark.GetMarkedSdrObj();
                    SwDrawContact *pContact = (SwDrawContact*)pObj->GetUserCall();
                    if( pContact ) // natuerlich nicht bei gruppierten Objekten
                    {
                        SwDrawFrmFmt *pFmt = (SwDrawFrmFmt*)pContact->GetFmt();
                        // before delete of selection is performed, marked
                        // <SwDrawVirtObj>-objects have to be replaced by its
                        // reference objects.  Thus, assert, if a
                        // <SwDrawVirt>-object is found in the mark list.
                        if ( pObj->ISA(SwDrawVirtObj) )
                        {
                            OSL_ENSURE( false,
                                    "<SwDrawVirtObj> is still marked for delete. application will crash!" );
                        }
                        //loescht sich selbst!
                        pContact->Changed(*pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
                        pObj->SetUserCall( 0 );

                        if( pUndo )
                            pUndo->AddObj( i, pFmt, rMark );
                        else
                            DelFrmFmt( pFmt );
                    }
                }

                if( pUndo )
                    AppendUndo( pUndo );
            }
            bCallBase = TRUE;
        }
        SetModified();

        EndUndo(UNDO_EMPTY, NULL);
    }

    return bCallBase;
}

/*************************************************************************
|*
|*  SwDoc::DeleteSelection()
|*
|*************************************************************************/

_ZSortFly::_ZSortFly( const SwFrmFmt* pFrmFmt, const SwFmtAnchor* pFlyAn,
                      UINT32 nArrOrdNum )
    : pFmt( pFrmFmt ), pAnchor( pFlyAn ), nOrdNum( nArrOrdNum )
{
    // #i11176#
    // This also needs to work when no layout exists. Thus, for
    // FlyFrames an alternative method is used now in that case.
    SwClientIter aIter( (SwFmt&)*pFmt );

    if( RES_FLYFRMFMT == pFmt->Which() )
    {
        if( pFmt->getIDocumentLayoutAccess()->GetRootFrm() )
        {
            // Schauen, ob es ein SdrObject dafuer gibt
            if( aIter.First( TYPE( SwFlyFrm) ) )
                nOrdNum = ((SwFlyFrm*)aIter())->GetVirtDrawObj()->GetOrdNum();
        }
        else
        {
            // Schauen, ob es ein SdrObject dafuer gibt
            if( aIter.First( TYPE(SwFlyDrawContact) ) )
                nOrdNum = ((SwFlyDrawContact*)aIter())->GetMaster()->GetOrdNum();
        }
    }
    else if( RES_DRAWFRMFMT == pFmt->Which() )
    {
        // Schauen, ob es ein SdrObject dafuer gibt
        if( aIter.First( TYPE(SwDrawContact) ) )
            nOrdNum = ((SwDrawContact*)aIter())->GetMaster()->GetOrdNum();
    }
    else {
        OSL_ENSURE( !this, "was ist das fuer ein Format?" );
    }
}

/*************************************************************************/
// Wird auch vom Sw3-Reader gerufen, wenn ein Fehler beim Einlesen
// des Drawing Layers auftrat. In diesem Fall wird der Layer komplett
// neu aufgebaut.

#include <svx/sxenditm.hxx>

void SwDoc::InitDrawModel()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDoc::InitDrawModel" );

    //!!Achtung im sw3-Reader (sw3imp.cxx) gibt es aehnlichen Code, der
    //mitgepfelgt werden muss.
    if ( pDrawModel )
        ReleaseDrawModel();

    //DrawPool und EditEnginePool anlegen, diese gehoeren uns und werden
    //dem Drawing nur mitgegeben. Im ReleaseDrawModel werden die Pools wieder
    //zerstoert.
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
    SfxItemPool *pEEgPool = EditEngine::CreatePool( FALSE );
    pSdrPool->SetSecondaryPool( pEEgPool );
     if ( !GetAttrPool().GetFrozenIdRanges () )
        GetAttrPool().FreezeIdRanges();
    else
        pSdrPool->FreezeIdRanges();

    // set FontHeight pool defaults without changing static SdrEngineDefaults
     GetAttrPool().SetPoolDefaultItem(SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT ));

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "before create DrawDocument" );
    //Das SdrModel gehoert dem Dokument, wir haben immer zwei Layer und eine
    //Seite.
    pDrawModel = new SwDrawDocument( this );

    pDrawModel->EnableUndo( DoesUndo() );

    String sLayerNm;
    sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Hell" ));
    nHell   = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Heaven" ));
    nHeaven = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Controls" ));
    nControls = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

    // add invisible layers corresponding to the visible ones.
    {
        sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleHell" ));
        nInvisibleHell   = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

        sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleHeaven" ));
        nInvisibleHeaven = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();

        sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleControls" ));
        nInvisibleControls = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();
    }

    pDrawModel->InsertPage( pDrawModel->AllocPage( FALSE ) );
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "after create DrawDocument" );

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "before create Spellchecker/Hyphenator" );
    SdrOutliner& rOutliner = pDrawModel->GetDrawOutliner();
    uno::Reference< XSpellChecker1 > xSpell = ::GetSpellChecker();
    rOutliner.SetSpeller( xSpell );
    uno::Reference<XHyphenator> xHyphenator( ::GetHyphenator() );
    rOutliner.SetHyphenator( xHyphenator );
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "after create Spellchecker/Hyphenator" );

    SetCalcFieldValueHdl(&rOutliner);
    SetCalcFieldValueHdl(&pDrawModel->GetHitTestOutliner());

    //JP 16.07.98: Bug 50193 - Linkmanager am Model setzen, damit
    //          dort ggfs. verlinkte Grafiken eingefuegt werden koennen
    //JP 28.01.99: der WinWord Import benoetigt ihn auch
    pDrawModel->SetLinkManager( &GetLinkManager() );
    pDrawModel->SetAddExtLeading( get(IDocumentSettingAccess::ADD_EXT_LEADING) );

    OutputDevice* pRefDev = getReferenceDevice( false );
    if ( pRefDev )
        pDrawModel->SetRefDevice( pRefDev );

    pDrawModel->SetNotifyUndoActionHdl( LINK( this, SwDoc, AddDrawUndo ));
    if ( pLayout )
    {
        pLayout->SetDrawPage( pDrawModel->GetPage( 0 ) );
        pLayout->GetDrawPage()->SetSize( pLayout->Frm().SSize() );
    }
}

/** method to notify drawing page view about the invisible layers */
void SwDoc::NotifyInvisibleLayers( SdrPageView& _rSdrPageView )
{
    String sLayerNm;
    sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleHell" ));
    _rSdrPageView.SetLayerVisible( sLayerNm, FALSE );

    sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleHeaven" ));
    _rSdrPageView.SetLayerVisible( sLayerNm, FALSE );

    sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleControls" ));
    _rSdrPageView.SetLayerVisible( sLayerNm, FALSE );
}

/** method to determine, if a layer ID belongs to the visible ones.

    Note: If given layer ID is unknown, method asserts and returns <false>.
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
        OSL_ENSURE( false, "<SwDoc::IsVisibleLayerId(..)> - unknown layer ID." );
        bRetVal = false;
    }

    return bRetVal;
}

/** method to determine, if the corresponding visible layer ID for a invisible one.

    Note: If given layer ID is a visible one, method returns given layer ID.
    Note: If given layer ID is unknown, method returns given layer ID.
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
        OSL_ENSURE( false, "<SwDoc::GetVisibleLayerIdByInvisibleOne(..)> - given layer ID already an invisible one." );
        nVisibleLayerId = _nInvisibleLayerId;
    }
    else
    {
        OSL_ENSURE( false, "<SwDoc::GetVisibleLayerIdByInvisibleOne(..)> - given layer ID is unknown." );
        nVisibleLayerId = _nInvisibleLayerId;
    }

    return nVisibleLayerId;
}

/** method to determine, if the corresponding invisible layer ID for a visible one.

    Note: If given layer ID is a invisible one, method returns given layer ID.
    Note: If given layer ID is unknown, method returns given layer ID.
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
        OSL_ENSURE( false, "<SwDoc::GetInvisibleLayerIdByVisibleOne(..)> - given layer ID already an invisible one." );
        nInvisibleLayerId = _nVisibleLayerId;
    }
    else
    {
        OSL_ENSURE( false, "<SwDoc::GetInvisibleLayerIdByVisibleOne(..)> - given layer ID is unknown." );
        nInvisibleLayerId = _nVisibleLayerId;
    }

    return nInvisibleLayerId;
}

/*************************************************************************/


void SwDoc::ReleaseDrawModel()
{
    if ( pDrawModel )
    {
        //!!Den code im sw3io fuer Einfuegen Dokument mitpflegen!!

        delete pDrawModel; pDrawModel = 0;
        SfxItemPool *pSdrPool = GetAttrPool().GetSecondaryPool();

        OSL_ENSURE( pSdrPool, "missing Pool" );
        SfxItemPool *pEEgPool = pSdrPool->GetSecondaryPool();
        OSL_ENSURE( !pEEgPool->GetSecondaryPool(), "i don't accept additional pools");
        pSdrPool->Delete();                 //Erst die Items vernichten lassen,
                                            //dann erst die Verkettung loesen
        GetAttrPool().SetSecondaryPool( 0 );    //Der ist ein muss!
        pSdrPool->SetSecondaryPool( 0 );    //Der ist sicherer
        SfxItemPool::Free(pSdrPool);
        SfxItemPool::Free(pEEgPool);
    }
}

/*************************************************************************/


SdrModel* SwDoc::_MakeDrawModel()
{
    OSL_ENSURE( !pDrawModel, "_MakeDrawModel: Why?" );
    InitDrawModel();
    if ( pLayout && pLayout->GetCurrShell() )
    {
        ViewShell* pTmp = pLayout->GetCurrShell();
        do
        {
            pTmp->MakeDrawView();
            pTmp = (ViewShell*) pTmp->GetNext();
        } while ( pTmp != pLayout->GetCurrShell() );

        //Broadcast, damit die FormShell mit der DrawView verbunden werden kann
        if( GetDocShell() )
        {
            SfxSimpleHint aHnt( SW_BROADCAST_DRAWVIEWS_CREATED );
            GetDocShell()->Broadcast( aHnt );
        }
    }
    return pDrawModel;
}

/*************************************************************************/

void SwDoc::DrawNotifyUndoHdl()
{
    pDrawModel->SetNotifyUndoActionHdl( Link() );
}

/*************************************************************************
*
* Am Outliner Link auf Methode fuer Felddarstellung in Editobjekten setzen
*
*************************************************************************/

void SwDoc::SetCalcFieldValueHdl(Outliner* pOutliner)
{
    pOutliner->SetCalcFieldValueHdl(LINK(this, SwDoc, CalcFieldValueHdl));
}

/*************************************************************************
|*
|* Felder bzw URLs im Outliner erkennen und Darstellung festlegen
|*
\************************************************************************/

IMPL_LINK(SwDoc, CalcFieldValueHdl, EditFieldInfo*, pInfo)
{
    if (pInfo)
    {
        const SvxFieldItem& rField = pInfo->GetField();
        const SvxFieldData* pField = rField.GetField();

        if (pField && pField->ISA(SvxDateField))
        {
            /******************************************************************
            * Date-Field
            ******************************************************************/
            pInfo->SetRepresentation(
                ((const SvxDateField*) pField)->GetFormatted(
                        *GetNumberFormatter( TRUE ), LANGUAGE_SYSTEM) );
        }
        else if (pField && pField->ISA(SvxURLField))
        {
            /******************************************************************
            * URL-Field
            ******************************************************************/

            switch ( ((const SvxURLField*) pField)->GetFormat() )
            {
                case SVXURLFORMAT_APPDEFAULT: //!!! einstellbar an App???
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

            USHORT nChrFmt;

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
            /******************************************************************
            * Measure-Field
            ******************************************************************/
            pInfo->ClearFldColor();
        }
        else if ( pField && pField->ISA(SvxExtTimeField))
        {
            /******************************************************************
            * Time-Field
            ******************************************************************/
            pInfo->SetRepresentation(
                ((const SvxExtTimeField*) pField)->GetFormatted(
                        *GetNumberFormatter( TRUE ), LANGUAGE_SYSTEM) );
        }
        else
        {
            OSL_FAIL("unbekannter Feldbefehl");
            pInfo->SetRepresentation( String( '?' ) );
        }
    }

    return(0);
}

/* TFFDI: The functions formerly declared 'inline'
 */
const SdrModel* SwDoc::GetDrawModel() const { return pDrawModel; }
SdrModel* SwDoc::GetDrawModel() { return pDrawModel; }
SdrLayerID SwDoc::GetHeavenId() const { return nHeaven; }
SdrLayerID SwDoc::GetHellId() const { return nHell; }
SdrLayerID SwDoc::GetControlsId() const { return nControls;   }
SdrLayerID SwDoc::GetInvisibleHeavenId() const { return nInvisibleHeaven; }
SdrLayerID SwDoc::GetInvisibleHellId() const { return nInvisibleHell; }
SdrLayerID SwDoc::GetInvisibleControlsId() const { return nInvisibleControls; }
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
                                OSL_ENSURE( false,
                                        "<docfunc::AllDrawObjsOnPage() - missing anchored draw object" );
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
