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
#include <docfunc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
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
#include <calbck.hxx>

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
    SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
    bool bNoGroup = ( 0 == pObj->GetUpGroup() );
    SwDrawContact* pNewContact = 0;
    if( bNoGroup )
    {
        SwDrawFrmFmt *pFmt = 0L;

        // Revoke anchor attribute.
        SwDrawContact *pMyContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
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
        for( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));

            // #i53320#
#if OSL_DEBUG_LEVEL > 0
            SwAnchoredDrawObject* pAnchoredDrawObj =
                static_cast<SwAnchoredDrawObject*>(pContact->GetAnchoredObj( pObj ));
            OSL_ENSURE( bGroupMembersNotPositioned == pAnchoredDrawObj->NotYetPositioned(),
                    "<SwDoc::GroupSelection(..)> - group members have different positioning status!" );
#endif

            pFmt = static_cast<SwDrawFrmFmt*>(pContact->GetFmt());
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
    const size_t nMarkCount( rMrkList.GetMarkCount() );
    if ( nMarkCount )
    {
        pFmtsAndObjs = new std::vector< std::pair< SwDrawFrmFmt*, SdrObject* > >[nMarkCount];
        SdrObject *pMyObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
        if( !pMyObj->GetUpGroup() )
        {
            OUString sDrwFmtNm("DrawObject");
            for ( size_t i = 0; i < nMarkCount; ++i )
            {
                SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
                if ( pObj->IsA( TYPE(SdrObjGroup) ) )
                {
                    SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
                    SwFmtAnchor aAnch( pContact->GetFmt()->GetAnchor() );
                    SdrObjList *pLst = static_cast<SdrObjGroup*>(pObj)->GetSubList();

                    SwUndoDrawUnGroup* pUndo = 0;
                    if( bUndo )
                    {
                        pUndo = new SwUndoDrawUnGroup( static_cast<SdrObjGroup*>(pObj) );
                        GetIDocumentUndoRedo().AppendUndo(pUndo);
                    }

                    for ( size_t i2 = 0; i2 < pLst->GetObjCount(); ++i2 )
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
                            pUndo->AddObj( static_cast<sal_uInt16>(i2), pFmt );
                    }
                }
            }
        }
    }
    rDrawView.UnGroupMarked();
    // creation of <SwDrawContact> instances for the former group members and
    // its connection to the Writer layout.
    for ( size_t i = 0; i < nMarkCount; ++i )
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
        bool bDelMarked = true;

        if( 1 == rMrkList.GetMarkCount() )
        {
            SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
            if( pObj->ISA(SwVirtFlyDrawObj) )
            {
                SwFlyFrmFmt* pFrmFmt =
                    static_cast<SwVirtFlyDrawObj*>(pObj)->GetFlyFrm()->GetFmt();
                if( pFrmFmt )
                {
                    getIDocumentLayoutAccess().DelLayoutFmt( pFrmFmt );
                    bDelMarked = false;
                }
            }
        }

        for( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            if( !pObj->ISA(SwVirtFlyDrawObj) )
            {
                SwDrawContact *pC = static_cast<SwDrawContact*>(GetUserCall(pObj));
                SwDrawFrmFmt *pFrmFmt = static_cast<SwDrawFrmFmt*>(pC->GetFmt());
                if( pFrmFmt &&
                    FLY_AS_CHAR == pFrmFmt->GetAnchor().GetAnchorId() )
                {
                    rDrawView.MarkObj( pObj, rDrawView.Imp().GetPageView(), true );
                    --i;
                    getIDocumentLayoutAccess().DelLayoutFmt( pFrmFmt );
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
                for( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
                {
                    const SdrMark& rMark = *rMrkList.GetMark( i );
                    pObj = rMark.GetMarkedSdrObj();
                    SwDrawContact *pContact = static_cast<SwDrawContact*>(pObj->GetUserCall());
                    if( pContact ) // of course not for grouped objects
                    {
                        SwDrawFrmFmt *pFmt = static_cast<SwDrawFrmFmt*>(pContact->GetFmt());
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
        getIDocumentState().SetModified();

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
            SwFlyFrm* pFly = SwIterator<SwFlyFrm,SwFmt>( *pFrmFmt ).First();
            if( pFly )
                nOrdNum = pFly->GetVirtDrawObj()->GetOrdNum();
        }
        else
        {
            // See if there is an SdrObject for it
            SwFlyDrawContact* pContact = SwIterator<SwFlyDrawContact,SwFmt>( *pFrmFmt ).First();
            if( pContact )
                nOrdNum = pContact->GetMaster()->GetOrdNum();
        }
    }
    else if( RES_DRAWFRMFMT == pFmt->Which() )
    {
        // See if there is an SdrObject for it
        SwDrawContact* pContact = SwIterator<SwDrawContact,SwFmt>( *pFrmFmt ).First();
        if( pContact )
            nOrdNum = pContact->GetMaster()->GetOrdNum();
    }
    else {
        OSL_ENSURE( false, "what kind of format is this?" );
    }
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
                static_cast<const SvxDateField*>( pField)->GetFormatted(
                        *GetNumberFormatter( true ), LANGUAGE_SYSTEM) );
        }
        else if (pField && pField->ISA(SvxURLField))
        {
            // URL field
            switch ( static_cast<const SvxURLField*>( pField)->GetFormat() )
            {
                case SVXURLFORMAT_APPDEFAULT: //!!! Can be set in App???
                case SVXURLFORMAT_REPR:
                {
                    pInfo->SetRepresentation(
                        static_cast<const SvxURLField*>(pField)->GetRepresentation());
                }
                break;

                case SVXURLFORMAT_URL:
                {
                    pInfo->SetRepresentation(
                        static_cast<const SvxURLField*>(pField)->GetURL());
                }
                break;
            }

            sal_uInt16 nChrFmt;

            if (IsVisitedURL(static_cast<const SvxURLField*>(pField)->GetURL()))
                nChrFmt = RES_POOLCHR_INET_VISIT;
            else
                nChrFmt = RES_POOLCHR_INET_NORMAL;

            SwFmt *pFmt = getIDocumentStylePoolAccess().GetCharFmtFromPool(nChrFmt);

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
                static_cast<const SvxExtTimeField*>( pField)->GetFormatted(
                        *GetNumberFormatter( true ), LANGUAGE_SYSTEM) );
        }
        else
        {
            OSL_FAIL("unknown field command");
            pInfo->SetRepresentation( OUString( '?' ) );
        }
    }

    return 0;
}

// #i62875#
namespace docfunc
{
    bool ExistsDrawObjs( SwDoc& p_rDoc )
    {
        bool bExistsDrawObjs( false );

        if ( p_rDoc.getIDocumentDrawModelAccess().GetDrawModel() &&
             p_rDoc.getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 ) )
        {
            const SdrPage& rSdrPage( *(p_rDoc.getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 )) );

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

        if ( p_rDoc.getIDocumentDrawModelAccess().GetDrawModel() &&
             p_rDoc.getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 ) )
        {
            const SdrPage& rSdrPage( *(p_rDoc.getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 )) );

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
