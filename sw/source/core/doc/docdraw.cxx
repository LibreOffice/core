/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
using namespace css::linguistic2;

/** local method to determine positioning and alignment attributes for a drawing
 *  object, which is newly connected to the layout.
 *
 * Used for a newly formed group object <SwDoc::GroupSelection(..)>
 * and the members of a destroyed group <SwDoc::UnGroupSelection(..)>
 */
static void lcl_AdjustPositioningAttr( SwDrawFrameFormat* _pFrameFormat,
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
                !pAnchorFrm->IsTextFrm() ||
                !static_cast<const SwTextFrm*>(pAnchorFrm)->IsFollow(),
                "<lcl_AdjustPositioningAttr(..)> - anchor frame is a follow." );
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
                static_cast<const SvxFrameDirectionItem*>(&(_pFrameFormat->GetAttrSet().GetPool()->GetDefaultItem( RES_FRAMEDIR )));
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

    _pFrameFormat->SetFormatAttr( SwFormatHoriOrient( nHoriRelPos, text::HoriOrientation::NONE, text::RelOrientation::FRAME ) );
    _pFrameFormat->SetFormatAttr( SwFormatVertOrient( nVertRelPos, text::VertOrientation::NONE, text::RelOrientation::FRAME ) );
    // #i44334#, #i44681# - positioning attributes already set
    _pFrameFormat->PosAttrSet();
    // #i34750# - keep current object rectangle for  drawing
    // objects. The object rectangle is used on events from the drawing layer
    // to adjust the positioning attributes - see <SwDrawContact::_Changed(..)>.
    {
        const SwAnchoredObject* pAnchoredObj = pContact->GetAnchoredObj( &_rSdrObj );
        if ( dynamic_cast<const SwAnchoredDrawObject*>( pAnchoredObj) !=  nullptr )
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
        SwDrawFrameFormat *pFormat = 0L;

        // Revoke anchor attribute.
        SwDrawContact *pMyContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
        const SwFormatAnchor aAnch( pMyContact->GetFormat()->GetAnchor() );

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

            pFormat = static_cast<SwDrawFrameFormat*>(pContact->GetFormat());
            // Deletes itself!
            pContact->Changed(*pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
            pObj->SetUserCall( 0 );

            if( pUndo )
                pUndo->AddObj( i, pFormat, pObj );
            else
                DelFrameFormat( pFormat );

            // #i45952# - re-introduce position normalization of group member
            // objects, because its anchor position is cleared, when they are
            // grouped.
            Point aAnchorPos( pObj->GetAnchorPos() );
            pObj->NbcSetAnchorPos( Point( 0, 0 ) );
            pObj->NbcMove( Size( aAnchorPos.getX(), aAnchorPos.getY() ) );
        }

        pFormat = MakeDrawFrameFormat( OUString("DrawObject"),
                                GetDfltFrameFormat() );
        pFormat->SetFormatAttr( aAnch );
        // #i36010# - set layout direction of the position
        pFormat->SetPositionLayoutDir(
            text::PositionLayoutDir::PositionInLayoutDirOfAnchor );

        rDrawView.GroupMarked();
        OSL_ENSURE( rMrkList.GetMarkCount() == 1, "GroupMarked more or none groups." );

        SdrObject* pNewGroupObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
        pNewContact = new SwDrawContact( pFormat, pNewGroupObj );
        // #i35635#
        pNewContact->MoveObjToVisibleLayer( pNewGroupObj );
        pNewContact->ConnectToLayout();
        // #i53320# - No adjustment of the positioning and alignment
        // attributes, if group members aren't positioned yet.
        if ( !bGroupMembersNotPositioned )
        {
            // #i26791# - Adjust positioning and alignment attributes.
            lcl_AdjustPositioningAttr( pFormat, *pNewGroupObj );
        }

        if( pUndo )
        {
            pUndo->SetGroupFormat( pFormat );
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
    std::vector< std::pair< SwDrawFrameFormat*, SdrObject* > >* pFormatsAndObjs( 0L );
    const size_t nMarkCount( rMrkList.GetMarkCount() );
    if ( nMarkCount )
    {
        pFormatsAndObjs = new std::vector< std::pair< SwDrawFrameFormat*, SdrObject* > >[nMarkCount];
        SdrObject *pMyObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
        if( !pMyObj->GetUpGroup() )
        {
            OUString sDrwFormatNm("DrawObject");
            for ( size_t i = 0; i < nMarkCount; ++i )
            {
                SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
                if ( dynamic_cast<const SdrObjGroup*>(pObj) !=  nullptr )
                {
                    SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
                    SwFormatAnchor aAnch( pContact->GetFormat()->GetAnchor() );
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
                        SwDrawFrameFormat *pFormat = MakeDrawFrameFormat( sDrwFormatNm,
                                                            GetDfltFrameFormat() );
                        pFormat->SetFormatAttr( aAnch );
                        // #i36010# - set layout direction of the position
                        pFormat->SetPositionLayoutDir(
                            text::PositionLayoutDir::PositionInLayoutDirOfAnchor );
                        pFormatsAndObjs[i].push_back( std::pair< SwDrawFrameFormat*, SdrObject* >( pFormat, pSubObj ) );

                        if( bUndo )
                            pUndo->AddObj( static_cast<sal_uInt16>(i2), pFormat );
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

        while ( pFormatsAndObjs[i].size() > 0 )
        {
            SwDrawFrameFormat* pFormat( pFormatsAndObjs[i].back().first );
            SdrObject* pObj( pFormatsAndObjs[i].back().second );
            pFormatsAndObjs[i].pop_back();

            SwDrawContact* pContact = new SwDrawContact( pFormat, pObj );
            pContact->MoveObjToVisibleLayer( pObj );
            pContact->ConnectToLayout();
            lcl_AdjustPositioningAttr( pFormat, *pObj );

            if ( bUndo )
            {
                pUndo->AddFormatAndObj( pFormat, pObj );
            }
        }
    }
    delete [] pFormatsAndObjs;
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
            if( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr )
            {
                SwFlyFrameFormat* pFrameFormat =
                    static_cast<SwVirtFlyDrawObj*>(pObj)->GetFlyFrm()->GetFormat();
                if( pFrameFormat )
                {
                    getIDocumentLayoutAccess().DelLayoutFormat( pFrameFormat );
                    bDelMarked = false;
                }
            }
        }

        for( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            if( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) ==  nullptr )
            {
                SwDrawContact *pC = static_cast<SwDrawContact*>(GetUserCall(pObj));
                SwDrawFrameFormat *pFrameFormat = static_cast<SwDrawFrameFormat*>(pC->GetFormat());
                if( pFrameFormat &&
                    FLY_AS_CHAR == pFrameFormat->GetAnchor().GetAnchorId() )
                {
                    rDrawView.MarkObj( pObj, rDrawView.Imp().GetPageView(), true );
                    --i;
                    getIDocumentLayoutAccess().DelLayoutFormat( pFrameFormat );
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
                        SwDrawFrameFormat *pFormat = static_cast<SwDrawFrameFormat*>(pContact->GetFormat());
                        // before delete of selection is performed, marked
                        // <SwDrawVirtObj>-objects have to be replaced by its
                        // reference objects.  Thus, assert, if a
                        // <SwDrawVirt>-object is found in the mark list.
                        if ( dynamic_cast<const SwDrawVirtObj*>( pObj) !=  nullptr )
                        {
                            OSL_FAIL( "<SwDrawVirtObj> is still marked for delete. application will crash!" );
                        }
                        // Deletes itself!
                        pContact->Changed(*pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
                        pObj->SetUserCall( 0 );

                        if( pUndo )
                            pUndo->AddObj( i, pFormat, rMark );
                        else
                            DelFrameFormat( pFormat );
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

_ZSortFly::_ZSortFly( const SwFrameFormat* pFrameFormat, const SwFormatAnchor* pFlyAn,
                      sal_uInt32 nArrOrdNum )
    : pFormat( pFrameFormat ), pAnchor( pFlyAn ), nOrdNum( nArrOrdNum )
{
    // #i11176#
    // This also needs to work when no layout exists. Thus, for
    // FlyFrames an alternative method is used now in that case.
    if( RES_FLYFRMFMT == pFormat->Which() )
    {
        if( pFormat->getIDocumentLayoutAccess().GetCurrentViewShell() )
        {
            // See if there is an SdrObject for it
            SwFlyFrm* pFly = SwIterator<SwFlyFrm,SwFormat>( *pFrameFormat ).First();
            if( pFly )
                nOrdNum = pFly->GetVirtDrawObj()->GetOrdNum();
        }
        else
        {
            // See if there is an SdrObject for it
            SwFlyDrawContact* pContact = SwIterator<SwFlyDrawContact,SwFormat>( *pFrameFormat ).First();
            if( pContact )
                nOrdNum = pContact->GetMaster()->GetOrdNum();
        }
    }
    else if( RES_DRAWFRMFMT == pFormat->Which() )
    {
        // See if there is an SdrObject for it
        SwDrawContact* pContact = SwIterator<SwDrawContact,SwFormat>( *pFrameFormat ).First();
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
IMPL_LINK_TYPED(SwDoc, CalcFieldValueHdl, EditFieldInfo*, pInfo, void)
{
    if (!pInfo)
        return;

    const SvxFieldItem& rField = pInfo->GetField();
    const SvxFieldData* pField = rField.GetField();

    if (pField && dynamic_cast<const SvxDateField*>( pField) !=  nullptr)
    {
        // Date field
        pInfo->SetRepresentation(
            static_cast<const SvxDateField*>( pField)->GetFormatted(
                    *GetNumberFormatter(), LANGUAGE_SYSTEM) );
    }
    else if (pField && dynamic_cast<const SvxURLField*>( pField) !=  nullptr)
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

        sal_uInt16 nChrFormat;

        if (IsVisitedURL(static_cast<const SvxURLField*>(pField)->GetURL()))
            nChrFormat = RES_POOLCHR_INET_VISIT;
        else
            nChrFormat = RES_POOLCHR_INET_NORMAL;

        SwFormat *pFormat = getIDocumentStylePoolAccess().GetCharFormatFromPool(nChrFormat);

        Color aColor(COL_LIGHTBLUE);
        if (pFormat)
            aColor = pFormat->GetColor().GetValue();

        pInfo->SetTextColor(aColor);
    }
    else if (pField && dynamic_cast<const SdrMeasureField*>( pField) !=  nullptr)
    {
        // Measure field
        pInfo->ClearFieldColor();
    }
    else if ( pField && dynamic_cast<const SvxExtTimeField*>( pField) !=  nullptr)
    {
        // Time field
        pInfo->SetRepresentation(
            static_cast<const SvxExtTimeField*>( pField)->GetFormatted(
                    *GetNumberFormatter(), LANGUAGE_SYSTEM) );
    }
    else
    {
        OSL_FAIL("unknown field command");
        pInfo->SetRepresentation( OUString( '?' ) );
    }
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
