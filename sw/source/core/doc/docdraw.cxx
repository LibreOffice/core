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
#include <IDocumentUndoRedo.hxx>
#include <docsh.hxx>
#include <rootfrm.hxx>          // So that the RootDtor is called.
#include <poolfmt.hxx>
#include <viewsh.hxx>           // for MakeDrawView
#include <drawdoc.hxx>
#include <UndoDraw.hxx>
#include <swundo.hxx>           // for the UndoIds
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
                    // vertical from left-to-right - Badaa: supported now!
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
                  nHoriRelPos = aObjRect.Left() - aAnchorPos.X();
                  nVertRelPos = aObjRect.Top() - aAnchorPos.Y();
            } else {
                //FRMDIR_VERT_TOP_RIGHT
                nHoriRelPos = aObjRect.Top() - aAnchorPos.Y();
                nVertRelPos = aAnchorPos.X() - aObjRect.Right();
            }
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
        //End of SCMS
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



/*************************************************************************
|*
|*  SwDoc::DeleteSelection()
|*
|*************************************************************************/

_ZSortFly::_ZSortFly( const SwFrmFmt* pFrmFmt, const SwFmtAnchor* pFlyAn,
                      sal_uInt32 nArrOrdNum )
    : pFmt( pFrmFmt ), pAnchor( pFlyAn ), nOrdNum( nArrOrdNum )
{
    // #i11176#
    // This also needs to work when no layout exists. Thus, for
    // FlyFrames an alternative method is used now in that case.
    if( RES_FLYFRMFMT == pFmt->Which() )
    {
        if( pFmt->getIDocumentLayoutAccess()->GetCurrentViewShell() )   //swmod 071107//swmod 071225
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
        OSL_ENSURE( !this, "was ist das fuer ein Format?" );
    }
}

/*************************************************************************/
// Is also called by the Sw3 Reader, if there was an error when reading the
// drawing layer. If it is called by the Sw3 Reader the layer is rebuilt
// from scratch.

#include <svx/sxenditm.hxx>

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
