/*************************************************************************
 *
 *  $RCSfile: anchoredobjectposition.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 13:08:33 $
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
#ifndef _ANCHOREDOBJECTPOSITION_HXX
#include <anchoredobjectposition.hxx>
#endif
#ifndef _ENVIRONMENTOFANCHOREDOBJECT
#include <environmentofanchoredobject.hxx>
#endif

#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _SVX_SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
// OD 2004-03-23 #i26791#
#ifndef _ANCHOREDOBJECT_HXX
#include <anchoredobject.hxx>
#endif
// OD 2004-03-16 #i11860#
#ifndef _DOC_HXX
#include <doc.hxx>
#endif

using namespace objectpositioning;

// **************************************************************************
// constructor, destructor, initialization
// **************************************************************************
SwAnchoredObjectPosition::SwAnchoredObjectPosition( SdrObject& _rDrawObj )
    : mrDrawObj( _rDrawObj ),
      mbIsObjFly( false ),
      mpAnchoredObj( 0 ),
      mpAnchorFrm( 0 ),
      mpContact( 0 )
{
#if OSL_DEBUG_LEVEL > 1
    // assert, if object isn't of excepted type
    const bool bObjOfExceptedType =
            mrDrawObj.ISA(SwVirtFlyDrawObj) || // object representing fly frame
            mrDrawObj.ISA(SwDrawVirtObj)    || // 'virtual' drawing object
            ( !mrDrawObj.ISA(SdrVirtObj) &&    // 'master' drawing object
              !mrDrawObj.ISA(SwFlyDrawObj) );  // - indirectly checked
    ASSERT( bObjOfExceptedType,
            "SwAnchoredObjectPosition(..) - object of unexcepted type!" );
#endif

    _GetInfoAboutObj();
}

/** determine information about object

    OD 30.07.2003 #110978#
    member <mbIsObjFly>, <mpFrmOfObj>, <mpAnchorFrm> and
    <mpContact> are set

    @author OD
*/
void SwAnchoredObjectPosition::_GetInfoAboutObj()
{
    // determine, if object represents a fly frame
    {
        mbIsObjFly = mrDrawObj.ISA(SwVirtFlyDrawObj);
    }

    // determine contact object
    {
        mpContact = static_cast<SwContact*>(GetUserCall( &mrDrawObj ));
        ASSERT( mpContact,
                "SwAnchoredObjectPosition::_GetInfoAboutObj() - missing SwContact-object." );
    }

    // determine anchored object, the object belongs to
    {
        // OD 2004-03-30 #i26791#
        mpAnchoredObj = mpContact->GetAnchoredObj( &mrDrawObj );
        ASSERT( mpAnchoredObj,
                "SwAnchoredObjectPosition::_GetInfoAboutObj() - missing anchored object." );
    }

    // determine frame, the object is anchored at
    {
        // OD 2004-03-23 #i26791#
        mpAnchorFrm = mpAnchoredObj->AnchorFrm();
        ASSERT( mpAnchorFrm,
                "SwAnchoredObjectPosition::_GetInfoAboutObj() - missing anchor frame." );
    }

    // determine format the object belongs to
    {
        if ( IsObjFly() )
        {
            mpFrmFmt = static_cast<SwVirtFlyDrawObj&>(mrDrawObj).GetFlyFrm()->GetFmt();
        }
        else
        {
            mpFrmFmt = static_cast<SwDrawContact*>(mpContact)->GetFmt();
        }

        ASSERT( mpFrmFmt,
                "<SwAnchoredObjectPosition::_GetInfoAboutObj() - missing frame format." );
    }
}

SwAnchoredObjectPosition::~SwAnchoredObjectPosition()
{}

// **************************************************************************
// accessors for object and its corresponding data/information
// **************************************************************************
SdrObject& SwAnchoredObjectPosition::GetObject() const
{
    return mrDrawObj;
}

bool SwAnchoredObjectPosition::IsObjFly() const
{
    return mbIsObjFly;
}

// OD 2004-03-23 #i26791#
SwAnchoredObject& SwAnchoredObjectPosition::GetAnchoredObj() const
{
    return *mpAnchoredObj;
}

SwFrm& SwAnchoredObjectPosition::GetAnchorFrm() const
{
    return *mpAnchorFrm;
}

SwContact& SwAnchoredObjectPosition::GetContact() const
{
    return *mpContact;
}

const SwFrmFmt& SwAnchoredObjectPosition::GetFrmFmt() const
{
    return *mpFrmFmt;
}

bool SwAnchoredObjectPosition::IsAnchoredToChar() const
{
    return false;
}

const SwFrm* SwAnchoredObjectPosition::ToCharOrientFrm() const
{
    return NULL;
}

const SwRect* SwAnchoredObjectPosition::ToCharRect() const
{
    return NULL;
}

// OD 12.11.2003 #i22341#
SwTwips SwAnchoredObjectPosition::ToCharTopOfLine() const
{
    return 0L;
}

/** helper method to determine top of a frame for the vertical
    object positioning

    OD 2004-03-11 #i11860#

    @author OD
*/
SwTwips SwAnchoredObjectPosition::_GetTopForObjPos( const SwFrm& _rFrm,
                                                    const SwRectFn& _fnRect,
                                                    const bool _bVert ) const
{
    SwTwips nTopOfFrmForObjPos = (_rFrm.Frm().*_fnRect->fnGetTop)();

    if ( _rFrm.IsTxtFrm() )
    {
        const SwTxtFrm& rTxtFrm = static_cast<const SwTxtFrm&>(_rFrm);
        if ( _bVert )
        {
            nTopOfFrmForObjPos -=
                rTxtFrm.GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
        }
        else
        {
            nTopOfFrmForObjPos +=
                rTxtFrm.GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
        }
    }

    return nTopOfFrmForObjPos;
}

void SwAnchoredObjectPosition::_GetVertAlignmentValues(
                                        const SwFrm& _rVertOrientFrm,
                                        const SwFrm& _rPageAlignLayFrm,
                                        const SwRelationOrient _eRelOrient,
                                        SwTwips&      _orAlignAreaHeight,
                                        SwTwips&      _orAlignAreaOffset ) const
{
    SwTwips nHeight = 0;
    SwTwips nOffset = 0;
    SWRECTFN( (&_rVertOrientFrm) )
    // OD 2004-03-11 #i11860# - top of <_rVertOrientFrm> for object positioning
    const SwTwips nVertOrientTop = _GetTopForObjPos( _rVertOrientFrm, fnRect, bVert );
    // OD 2004-03-11 #i11860# - upper space amount of <_rVertOrientFrm> considered
    // for previous frame
    const SwTwips nVertOrientUpperSpaceForPrevFrmAndPageGrid =
            _rVertOrientFrm.IsTxtFrm()
            ? static_cast<const SwTxtFrm&>(_rVertOrientFrm).
                        GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid()
            : 0;
    switch ( _eRelOrient )
    {
        case FRAME:
        {
            // OD 2004-03-11 #i11860# - consider upper space of previous frame
            nHeight = (_rVertOrientFrm.Frm().*fnRect->fnGetHeight)() -
                      nVertOrientUpperSpaceForPrevFrmAndPageGrid;
            nOffset = 0;
        }
        break;
        case PRTAREA:
        {
            nHeight = (_rVertOrientFrm.Prt().*fnRect->fnGetHeight)();
            // OD 2004-03-11 #i11860# - consider upper space of previous frame
            nOffset = (_rVertOrientFrm.*fnRect->fnGetTopMargin)() -
                      nVertOrientUpperSpaceForPrevFrmAndPageGrid;
            // if aligned to page in horizontal layout, consider header and
            // footer frame height appropriately.
            if( _rVertOrientFrm.IsPageFrm() && !bVert )
            {
                const SwFrm* pPrtFrm =
                        static_cast<const SwPageFrm&>(_rVertOrientFrm).Lower();
                while( pPrtFrm )
                {
                    if( pPrtFrm->IsHeaderFrm() )
                    {
                        nHeight -= pPrtFrm->Frm().Height();
                        nOffset += pPrtFrm->Frm().Height();
                    }
                    else if( pPrtFrm->IsFooterFrm() )
                    {
                        nHeight -= pPrtFrm->Frm().Height();
                    }
                    pPrtFrm = pPrtFrm->GetNext();
                }
            }
        }
        break;
        case REL_PG_FRAME:
        {
            nHeight = (_rPageAlignLayFrm.Frm().*fnRect->fnGetHeight)();
            nOffset = (*fnRect->fnYDiff)(
                        (_rPageAlignLayFrm.Frm().*fnRect->fnGetTop)(),
                        nVertOrientTop );
        }
        break;
        case REL_PG_PRTAREA:
        {
            nHeight = (_rPageAlignLayFrm.Prt().*fnRect->fnGetHeight)();
            nOffset = (_rPageAlignLayFrm.*fnRect->fnGetTopMargin)() +
                      (*fnRect->fnYDiff)(
                        (_rPageAlignLayFrm.Frm().*fnRect->fnGetTop)(),
                        nVertOrientTop );
            // if aligned to page in horizontal layout, consider header and
            // footer frame height appropriately.
            if( _rPageAlignLayFrm.IsPageFrm() && !bVert )
            {
                const SwFrm* pPrtFrm =
                        static_cast<const SwPageFrm&>(_rPageAlignLayFrm).Lower();
                while( pPrtFrm )
                {
                    if( pPrtFrm->IsHeaderFrm() )
                    {
                        nHeight -= pPrtFrm->Frm().Height();
                        nOffset += pPrtFrm->Frm().Height();
                    }
                    else if( pPrtFrm->IsFooterFrm() )
                    {
                        nHeight -= pPrtFrm->Frm().Height();
                    }
                    pPrtFrm = pPrtFrm->GetNext();
                }
            }
        }
        break;
        // OD 12.11.2003 #i22341# - vertical alignment at top of line
        case REL_VERT_LINE:
        {
            if ( IsAnchoredToChar() )
            {
                nHeight = 0;
                nOffset = (*fnRect->fnYDiff)( ToCharTopOfLine(), nVertOrientTop );
            }
            else
            {
                ASSERT( false,
                        "<SwAnchoredObjectPosition::_GetVertAlignmentValues(..)> - invalid relative alignment" );
            }
        }
        break;
        case REL_CHAR:
        {
            if ( IsAnchoredToChar() )
            {
                nHeight = (ToCharRect()->*fnRect->fnGetHeight)();
                nOffset = (*fnRect->fnYDiff)( (ToCharRect()->*fnRect->fnGetTop)(),
                                              nVertOrientTop );
            }
            else
            {
                ASSERT( false,
                        "<SwAnchoredObjectPosition::_GetVertAlignmentValues(..)> - invalid relative alignment" );
            }
        }
        break;
        // no break here, because REL_CHAR is invalid, if !mbAnchorToChar
        default:
        //case REL_PG_LEFT:     not valid for vertical alignment
        //case REL_PG_RIGHT:    not valid for vertical alignment
        //case REL_FRM_LEFT:    not valid for vertical alignment
        //case REL_FRM_RIGHT:   not valid for vertical alignment
        {
            ASSERT( false,
                    "<SwAnchoredObjectPosition::_GetVertAlignmentValues(..)> - invalid relative alignment" );
        }
    }

    _orAlignAreaHeight = nHeight;
    _orAlignAreaOffset = nOffset;
}

// --> OD 2004-06-17 #i26791# - add output parameter <_roVertOffsetToFrmAnchorPos>
SwTwips SwAnchoredObjectPosition::_GetVertRelPos(
                                    const SwFrm& _rVertOrientFrm,
                                    const SwFrm& _rPageAlignLayFrm,
                                    const SwVertOrient     _eVertOrient,
                                    const SwRelationOrient _eRelOrient,
                                    const SwTwips          _nVertPos,
                                    const SvxLRSpaceItem& _rLRSpacing,
                                    const SvxULSpaceItem& _rULSpacing,
                                    SwTwips& _roVertOffsetToFrmAnchorPos ) const
{
    SwTwips nRelPosY = 0;
    SWRECTFN( (&_rVertOrientFrm) );

    SwTwips nAlignAreaHeight;
    SwTwips nAlignAreaOffset;
    _GetVertAlignmentValues( _rVertOrientFrm, _rPageAlignLayFrm,
                             _eRelOrient, nAlignAreaHeight, nAlignAreaOffset );

    nRelPosY = nAlignAreaOffset;
    const SwRect aObjBoundRect( GetAnchoredObj().GetObjRect() );
    const SwTwips nObjHeight = (aObjBoundRect.*fnRect->fnGetHeight)();

    switch ( _eVertOrient )
    {
        case VERT_NONE:
        {
            // 'manual' vertical position
            nRelPosY += _nVertPos;
        }
        break;
        case VERT_TOP:
        {
            nRelPosY += bVert ? _rLRSpacing.GetRight() : _rULSpacing.GetUpper();
        }
        break;
        case VERT_CENTER:
        {
            nRelPosY += (nAlignAreaHeight / 2) - (nObjHeight / 2);
        }
        break;
        case VERT_BOTTOM:
        {
            nRelPosY += nAlignAreaHeight -
                       ( nObjHeight + ( bVert ? _rLRSpacing.GetLeft() : _rULSpacing.GetLower() ) );
        }
        break;
        default:
        {
            ASSERT( false,
                    "<SwAnchoredObjectPosition::_GetVertRelPos(..) - invalid vertical positioning" );
        }
    }

    // --> OD 2004-06-17 #i26791#
    _roVertOffsetToFrmAnchorPos = nAlignAreaOffset;

    return nRelPosY;
}

/** adjust calculated vertical in order to keep object inside
    'page' alignment layout frame.

    OD 2004-07-22 #i31805# - add 3rd parameter <_bCheckBottom>

    @author OD
*/
SwTwips SwAnchoredObjectPosition::_AdjustVertRelPos(
                                                const SwFrm&  _rPageAlignLayFrm,
                                                const SwTwips _nProposedRelPosY,
                                                const bool _bCheckBottom ) const
{
    SwTwips nAdjustedRelPosY = _nProposedRelPosY;

    const SwFrm& rAnchorFrm = GetAnchorFrm();
    SWRECTFN( (&rAnchorFrm) );

    const Size aObjSize( GetAnchoredObj().GetObjRect().SSize() );

    // determine the area of 'page' alignment frame, to which the vertical
    // position is restricted.
    SwRect aPgAlignArea = _rPageAlignLayFrm.Frm();
    /* OD 29.10.2003 - extension of 'page' alignment area, if 'page' alignment
                       frame is a page isn't applied yet
    {
        if ( _rPageAlignLayFrm.ISA(SwPageFrm) )
        {
            // extend 'page' area by size of object descreased by 10pt == 200 SwTwips
            if ( bVert )
            {
                SwTwips nExtend = aObjSize.Width() - 200;
                if ( nExtend > 0 )
                {
                    aPgAlignArea.Left( aPgAlignArea.Left() - nExtend );
                    aPgAlignArea.Right( aPgAlignArea.Right() + nExtend );
                }
            }
            else
            {
                SwTwips nExtend = aObjSize.Height() - 200;
                if ( nExtend > 0 )
                {
                    aPgAlignArea.Top( aPgAlignArea.Top() - nExtend );
                    aPgAlignArea.Bottom( aPgAlignArea.Bottom() + nExtend );
                }
            }
        }
    }
    */

    if ( bVert )
    {
        // OD 2004-07-22 #i31805# - consider value of <_bCheckBottom>
        if ( _bCheckBottom &&
             rAnchorFrm.Frm().Right() - nAdjustedRelPosY - aObjSize.Width() <
                aPgAlignArea.Left() )
        {
            nAdjustedRelPosY = aPgAlignArea.Left() +
                               rAnchorFrm.Frm().Right() -
                               aObjSize.Width();
        }
        if ( rAnchorFrm.Frm().Right() + nAdjustedRelPosY >
                aPgAlignArea.Right() )
        {
            nAdjustedRelPosY = aPgAlignArea.Right() -
                               rAnchorFrm.Frm().Right();
        }
    }
    else
    {
        // OD 2004-07-22 #i31805# - consider value of <_bCheckBottom>
        if ( _bCheckBottom &&
             rAnchorFrm.Frm().Top() + nAdjustedRelPosY + aObjSize.Height() >
                aPgAlignArea.Bottom() )
        {
            nAdjustedRelPosY = aPgAlignArea.Bottom() -
                               rAnchorFrm.Frm().Top() -
                               aObjSize.Height();
        }
        if ( rAnchorFrm.Frm().Top() + nAdjustedRelPosY <
                aPgAlignArea.Top() )
        {
            nAdjustedRelPosY = aPgAlignArea.Top() - rAnchorFrm.Frm().Top();
        }
    }

    return nAdjustedRelPosY;
}

/** adjust calculated horizontal in order to keep object inside
    'page' alignment layout frame.

    @author OD
*/
SwTwips SwAnchoredObjectPosition::_AdjustHoriRelPos(
                                        const SwFrm&  _rPageAlignLayFrm,
                                        const SwTwips _nProposedRelPosX ) const
{
    SwTwips nAdjustedRelPosX = _nProposedRelPosX;

    const SwFrm& rAnchorFrm = GetAnchorFrm();
    SWRECTFN( (&rAnchorFrm) );

    const Size aObjSize( GetAnchoredObj().GetObjRect().SSize() );

    if( bVert )
    {
        if ( rAnchorFrm.Frm().Top() + nAdjustedRelPosX + aObjSize.Height() >
                _rPageAlignLayFrm.Frm().Bottom() )
        {
            nAdjustedRelPosX = _rPageAlignLayFrm.Frm().Bottom() -
                               rAnchorFrm.Frm().Top() -
                               aObjSize.Height();
        }
        if ( rAnchorFrm.Frm().Top() + nAdjustedRelPosX <
                _rPageAlignLayFrm.Frm().Top() )
        {
            nAdjustedRelPosX = _rPageAlignLayFrm.Frm().Top() -
                               rAnchorFrm.Frm().Top();
        }
    }
    else
    {
        if ( rAnchorFrm.Frm().Left() + nAdjustedRelPosX + aObjSize.Width() >
                _rPageAlignLayFrm.Frm().Right() )
        {
            nAdjustedRelPosX = _rPageAlignLayFrm.Frm().Right() -
                               rAnchorFrm.Frm().Left() -
                               aObjSize.Width();
        }
        if ( rAnchorFrm.Frm().Left() + nAdjustedRelPosX <
                _rPageAlignLayFrm.Frm().Left() )
        {
            nAdjustedRelPosX = _rPageAlignLayFrm.Frm().Left() -
                               rAnchorFrm.Frm().Left();
        }
    }

    return nAdjustedRelPosX;
}

/** determine alignment value for horizontal position of object

    @author OD
*/
void SwAnchoredObjectPosition::_GetHoriAlignmentValues( const SwFrm&  _rHoriOrientFrm,
                                                        const SwFrm&  _rPageAlignLayFrm,
                                                        const SwRelationOrient _eRelOrient,
                                                        const bool    _bObjWrapThrough,
                                                        SwTwips&      _orAlignAreaWidth,
                                                        SwTwips&      _orAlignAreaOffset,
                                                        bool&         _obAlignedRelToPage ) const
{
    SwTwips nWidth = 0;
    SwTwips nOffset = 0;
    SWRECTFN( (&_rHoriOrientFrm) )
    switch ( _eRelOrient )
    {
        case PRTAREA:
        {
            nWidth = (_rHoriOrientFrm.Prt().*fnRect->fnGetWidth)();
            nOffset = (_rHoriOrientFrm.*fnRect->fnGetLeftMargin)();
            if ( _rHoriOrientFrm.IsTxtFrm() )
            {
                // consider movement of text frame left
                nOffset += static_cast<const SwTxtFrm&>(_rHoriOrientFrm).GetBaseOfstForFly( !_bObjWrapThrough );
            }
            else if ( _rHoriOrientFrm.IsPageFrm() && bVert )
            {
                // for to-page anchored objects, consider header/footer frame
                // in vertical layout
                const SwFrm* pPrtFrm =
                        static_cast<const SwPageFrm&>(_rHoriOrientFrm).Lower();
                while( pPrtFrm )
                {
                    if( pPrtFrm->IsHeaderFrm() )
                    {
                        nWidth -= pPrtFrm->Frm().Height();
                        nOffset += pPrtFrm->Frm().Height();
                    }
                    else if( pPrtFrm->IsFooterFrm() )
                    {
                        nWidth -= pPrtFrm->Frm().Height();
                    }
                    pPrtFrm = pPrtFrm->GetNext();
                }
            }
            break;
        }
        case REL_PG_LEFT:
        {
            // align at left border of page frame/fly frame/cell frame
            nWidth = (_rPageAlignLayFrm.*fnRect->fnGetLeftMargin)();
            nOffset = (*fnRect->fnXDiff)(
                      (_rPageAlignLayFrm.Frm().*fnRect->fnGetLeft)(),
                      (_rHoriOrientFrm.Frm().*fnRect->fnGetLeft)() );
            _obAlignedRelToPage = true;
        }
        break;
        case REL_PG_RIGHT:
        {
            // align at right border of page frame/fly frame/cell frame
            nWidth = (_rPageAlignLayFrm.*fnRect->fnGetRightMargin)();
            nOffset = (*fnRect->fnXDiff)(
                      (_rPageAlignLayFrm.*fnRect->fnGetPrtRight)(),
                      (_rHoriOrientFrm.Frm().*fnRect->fnGetLeft)() );
            _obAlignedRelToPage = true;
        }
        break;
        case REL_FRM_LEFT:
        {
            // align at left border of anchor frame
            nWidth = (_rHoriOrientFrm.*fnRect->fnGetLeftMargin)();
            nOffset = 0;
        }
        break;
        case REL_FRM_RIGHT:
        {
            // align at right border of anchor frame
            // OD 19.08.2003 #110978# - unify and simplify
            nWidth = (_rHoriOrientFrm.*fnRect->fnGetRightMargin)();
            //nOffset = (_rHoriOrientFrm.Frm().*fnRect->fnGetWidth)() -
            //          nWidth;
            nOffset = (_rHoriOrientFrm.Prt().*fnRect->fnGetRight)();
        }
        break;
        case REL_CHAR:
        {
            // alignment relative to character - assure, that corresponding
            // character rectangle is set.
            if ( IsAnchoredToChar() )
            {
                nWidth = 0;
                nOffset = (*fnRect->fnXDiff)(
                            (ToCharRect()->*fnRect->fnGetLeft)(),
                            (ToCharOrientFrm()->Frm().*fnRect->fnGetLeft)() );
                break;
            }
            // no break!
        }
        case REL_PG_PRTAREA:
        {
            nWidth = (_rPageAlignLayFrm.Prt().*fnRect->fnGetWidth)();
            nOffset = (*fnRect->fnXDiff)(
                        (_rPageAlignLayFrm.*fnRect->fnGetPrtLeft)(),
                        (_rHoriOrientFrm.Frm().*fnRect->fnGetLeft)() );
            if ( _rHoriOrientFrm.IsPageFrm() && bVert )
            {
                // for to-page anchored objects, consider header/footer frame
                // in vertical layout
                const SwFrm* pPrtFrm =
                        static_cast<const SwPageFrm&>(_rHoriOrientFrm).Lower();
                while( pPrtFrm )
                {
                    if( pPrtFrm->IsHeaderFrm() )
                    {
                        nWidth -= pPrtFrm->Frm().Height();
                        nOffset += pPrtFrm->Frm().Height();
                    }
                    else if( pPrtFrm->IsFooterFrm() )
                    {
                        nWidth -= pPrtFrm->Frm().Height();
                    }
                    pPrtFrm = pPrtFrm->GetNext();
                }
            }
            _obAlignedRelToPage = true;
            break;
        }
        case REL_PG_FRAME:
        {
            nWidth = (_rPageAlignLayFrm.Frm().*fnRect->fnGetWidth)();
            nOffset = (*fnRect->fnXDiff)(
                        (_rPageAlignLayFrm.Frm().*fnRect->fnGetLeft)(),
                        (_rHoriOrientFrm.Frm().*fnRect->fnGetLeft)() );
            _obAlignedRelToPage = true;
            break;
        }
        default:
        // case FRAME:
        {
            nWidth = (_rHoriOrientFrm.Frm().*fnRect->fnGetWidth)();
            nOffset = _rHoriOrientFrm.IsTxtFrm() ?
                   static_cast<const SwTxtFrm&>(_rHoriOrientFrm).GetBaseOfstForFly( !_bObjWrapThrough ) :
                   0;
            break;
        }
    }

    _orAlignAreaWidth = nWidth;
    _orAlignAreaOffset = nOffset;
}

/** toggle given horizontal orientation and relative alignment

    @author OD
*/
void SwAnchoredObjectPosition::_ToggleHoriOrientAndAlign(
                                        const bool         _bToggleLeftRight,
                                        SwHoriOrient&      _ioeHoriOrient,
                                        SwRelationOrient&  _iopeRelOrient
                                      ) const
{
    if( _bToggleLeftRight )
    {
        // toggle orientation
        switch ( _ioeHoriOrient )
        {
            case HORI_RIGHT :
                {
                    _ioeHoriOrient = HORI_LEFT;
                }
                break;
            case HORI_LEFT :
                {
                    _ioeHoriOrient = HORI_RIGHT;
                }
                break;
        }

        // toggle relative alignment
        switch ( _iopeRelOrient )
        {
            case REL_PG_RIGHT :
                {
                    _iopeRelOrient = REL_PG_LEFT;
                }
                break;
            case REL_PG_LEFT :
                {
                    _iopeRelOrient = REL_PG_RIGHT;
                }
                break;
            case REL_FRM_RIGHT :
                {
                    _iopeRelOrient = REL_FRM_LEFT;
                }
                break;
            case REL_FRM_LEFT :
                {
                    _iopeRelOrient = REL_FRM_RIGHT;
                }
                break;
        }
    }
}

/** calculate relative horizontal position

    @author OD
*/
SwTwips SwAnchoredObjectPosition::_CalcRelPosX(
                                const SwFrm& _rHoriOrientFrm,
                                const SwEnvironmentOfAnchoredObject& _rEnvOfObj,
                                const SwFmtHoriOrient& _rHoriOrient,
                                const SvxLRSpaceItem& _rLRSpacing,
                                const SvxULSpaceItem& _rULSpacing,
                                const bool _bObjWrapThrough,
                                const SwTwips _nRelPosY,
                                SwTwips& _roHoriOffsetToFrmAnchorPos
                              ) const
{
    // determine 'page' alignment layout frame
    const SwFrm& rPageAlignLayFrm =
            _rEnvOfObj.GetHoriEnvironmentLayoutFrm( _rHoriOrientFrm, true );

    const bool bEvenPage = !rPageAlignLayFrm.OnRightPage();
    const bool bToggle = _rHoriOrient.IsPosToggle() && bEvenPage;

    // determine orientation and relative alignment
    SwHoriOrient eHoriOrient = _rHoriOrient.GetHoriOrient();
    SwRelationOrient eRelOrient = _rHoriOrient.GetRelationOrient();
    // toggle orientation and relative alignment
    _ToggleHoriOrientAndAlign( bToggle, eHoriOrient, eRelOrient );

    // determine alignment parameter
    // <nWidth>:  'width' of alignment area
    // <nOffset>: offset of alignment area, relative to 'left' of anchor frame
    SwTwips nWidth = 0;
    SwTwips nOffset = 0;
    bool bAlignedRelToPage = false;
    _GetHoriAlignmentValues( _rHoriOrientFrm, rPageAlignLayFrm,
                             eRelOrient, _bObjWrapThrough,
                             nWidth, nOffset, bAlignedRelToPage );

    const SwFrm& rAnchorFrm = GetAnchorFrm();
    SWRECTFN( (&_rHoriOrientFrm) )
    SwTwips nObjWidth = (GetAnchoredObj().GetObjRect().*fnRect->fnGetWidth)();
    SwTwips nRelPosX = nOffset;
    if ( _rHoriOrient.GetHoriOrient() == HORI_NONE )
    {
        // 'manual' horizonal position
        const bool bR2L = rAnchorFrm.IsRightToLeft();
        if( IsAnchoredToChar() && REL_CHAR == eRelOrient )
        {
            if( bR2L )
                nRelPosX -= _rHoriOrient.GetPos();
            else
                nRelPosX += _rHoriOrient.GetPos();
        }
        else if ( bToggle || ( !_rHoriOrient.IsPosToggle() && bR2L ) )
        {
            // OD 04.08.2003 #110978# - correction: consider <nOffset> also for
            // toggling from left to right.
            nRelPosX += nWidth - nObjWidth - _rHoriOrient.GetPos();
        }
        else
        {
            nRelPosX += _rHoriOrient.GetPos();
        }
    }
    else if ( HORI_CENTER == eHoriOrient )
        nRelPosX += (nWidth / 2) - (nObjWidth / 2);
    else if ( HORI_RIGHT == eHoriOrient )
        nRelPosX += nWidth -
                    ( nObjWidth +
                      ( bVert ? _rULSpacing.GetLower() : _rLRSpacing.GetRight() ) );
    else
        nRelPosX += bVert ? _rULSpacing.GetUpper() : _rLRSpacing.GetLeft();

    // adjust relative position by distance between anchor frame and
    // the frame, the object is oriented at.
    if ( &rAnchorFrm != &_rHoriOrientFrm )
    {
        SwTwips nLeftOrient = (_rHoriOrientFrm.Frm().*fnRect->fnGetLeft)();
        SwTwips nLeftAnchor = (rAnchorFrm.Frm().*fnRect->fnGetLeft)();
        nRelPosX += (*fnRect->fnXDiff)( nLeftOrient, nLeftAnchor );
    }

    // adjust relative horizontal position, if object is manual horizontal
    // positioned (not 'page' aligned) and orients not at the anchor frame,
    // but it overlaps anchor frame.
    if ( _rHoriOrient.GetHoriOrient() == HORI_NONE && !bAlignedRelToPage &&
         &rAnchorFrm != &_rHoriOrientFrm )
    {
        // E.g.: consider a columned page/section with an horizontal
        //       negative positioned object.
        // OD 2004-03-23 #i26791#
        const SwRect& rObjRect = GetAnchoredObj().GetObjRect();
        if( bVert )
        {
            if( _rHoriOrientFrm.Frm().Top() > rAnchorFrm.Frm().Bottom() &&
                rObjRect.Right() > rAnchorFrm.Frm().Left() )
            {
                const SwTwips nProposedPosX = nRelPosX + rAnchorFrm.Frm().Top();
                if ( nProposedPosX < rAnchorFrm.Frm().Bottom() )
                    nRelPosX = rAnchorFrm.Frm().Height() + 1;
            }
        }
        else
        {
            if( _rHoriOrientFrm.Frm().Left() > rAnchorFrm.Frm().Right() &&
                rObjRect.Top() < rAnchorFrm.Frm().Bottom() )
            {
                // OD 04.08.2003 #110978# - correction: use <nRelPosX>
                // instead of <aRelPos.X()>
                const SwTwips nProposedPosX = nRelPosX + rAnchorFrm.Frm().Left();
                if ( nProposedPosX < rAnchorFrm.Frm().Right() )
                    nRelPosX = rAnchorFrm.Frm().Width() + 1;
            }
        }
    }
    // adjust calculated relative horizontal position, in order to
    // keep object inside 'page' alignment layout frame
    const SwFrm& rEnvironmentLayFrm =
            _rEnvOfObj.GetHoriEnvironmentLayoutFrm( _rHoriOrientFrm, false );
    nRelPosX = _AdjustHoriRelPos( rEnvironmentLayFrm, nRelPosX );

    // if object is a Writer fly frame and it's anchored to a content and
    // it is horizontal positioned left or right, but not relative to character,
    // it has to be drawn aside another object, which have the same horizontal
    // position and lay below it.
    if ( GetAnchoredObj().ISA(SwFlyFrm) &&
         ( GetContact().ObjAnchoredAtPara() || GetContact().ObjAnchoredAtChar() ) &&
         ( eHoriOrient == HORI_LEFT || eHoriOrient == HORI_RIGHT ) &&
         eRelOrient != REL_CHAR )
    {
        nRelPosX = _AdjustHoriRelPosForDrawAside( _rHoriOrientFrm,
                                                  nRelPosX, _nRelPosY,
                                                  eHoriOrient, eRelOrient,
                                                  _rLRSpacing, _rULSpacing,
                                                  bEvenPage );
    }

    // --> OD 2004-06-17 #i26791#
    _roHoriOffsetToFrmAnchorPos = nOffset;

    return nRelPosX;
}

// **************************************************************************
// method incl. helper methods for adjusting proposed horizontal position,
// if object has to draw aside another object.
// **************************************************************************
/** adjust calculated horizontal position in order to draw object
    aside other objects with same positioning

    @author OD
*/
SwTwips SwAnchoredObjectPosition::_AdjustHoriRelPosForDrawAside(
                                            const SwFrm&  _rHoriOrientFrm,
                                            const SwTwips _nProposedRelPosX,
                                            const SwTwips _nRelPosY,
                                            const SwHoriOrient _eHoriOrient,
                                            const SwRelationOrient _eRelOrient,
                                            const SvxLRSpaceItem& _rLRSpacing,
                                            const SvxULSpaceItem& _rULSpacing,
                                            const bool _bEvenPage
                                          ) const
{
    // OD 2004-03-23 #i26791#
    if ( !GetAnchorFrm().ISA(SwTxtFrm) ||
         !GetAnchoredObj().ISA(SwFlyAtCntFrm) )
    {
        ASSERT( false,
                "<SwAnchoredObjectPosition::_AdjustHoriRelPosForDrawAside(..) - usage for wrong anchor type" );
        return _nProposedRelPosX;
    }

    const SwTxtFrm& rAnchorTxtFrm = static_cast<const SwTxtFrm&>(GetAnchorFrm());
    // OD 2004-03-23 #i26791#
    const SwFlyAtCntFrm& rFlyAtCntFrm =
                        static_cast<const SwFlyAtCntFrm&>(GetAnchoredObj());
    const SwRect aObjBoundRect( GetAnchoredObj().GetObjRect() );
    SWRECTFN( (&_rHoriOrientFrm) )

    SwTwips nAdjustedRelPosX = _nProposedRelPosX;

    // determine proposed object bound rectangle
    Point aTmpPos = (rAnchorTxtFrm.Frm().*fnRect->fnGetPos)();
    if( bVert )
    {
        aTmpPos.X() -= _nRelPosY + aObjBoundRect.Width();
        aTmpPos.Y() += nAdjustedRelPosX;
    }
    else
    {
        aTmpPos.X() += nAdjustedRelPosX;
        aTmpPos.Y() += _nRelPosY;
    }
    SwRect aTmpObjRect( aTmpPos, aObjBoundRect.SSize() );

    const UINT32 nObjOrdNum = GetObject().GetOrdNum();
    const SwPageFrm* pObjPage = rFlyAtCntFrm.FindPageFrm();
    const SwFrm* pObjContext = ::FindKontext( &rAnchorTxtFrm, FRM_COLUMN );
    ULONG nObjIndex = rAnchorTxtFrm.GetTxtNode()->GetIndex();
    SwOrderIter aIter( pObjPage, TRUE );
    const SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)aIter.Bottom())->GetFlyFrm();
    while ( pFly && nObjOrdNum > pFly->GetVirtDrawObj()->GetOrdNumDirect() )
    {
        if ( _DrawAsideFly( pFly, aTmpObjRect, pObjContext, nObjIndex,
                           _bEvenPage, _eHoriOrient, _eRelOrient ) )
        {
            if( bVert )
            {
                const SvxULSpaceItem& rOtherUL = pFly->GetFmt()->GetULSpace();
                const SwTwips nOtherTop = pFly->Frm().Top() - rOtherUL.GetUpper();
                const SwTwips nOtherBot = pFly->Frm().Bottom() + rOtherUL.GetLower();
                if ( nOtherTop <= aTmpObjRect.Bottom() + _rULSpacing.GetLower() &&
                     nOtherBot >= aTmpObjRect.Top() - _rULSpacing.GetUpper() )
                {
                    if ( _eHoriOrient == HORI_LEFT )
                    {
                        SwTwips nTmp = nOtherBot + 1 + _rULSpacing.GetUpper() -
                                       rAnchorTxtFrm.Frm().Top();
                        if ( nTmp > nAdjustedRelPosX &&
                             rAnchorTxtFrm.Frm().Top() + nTmp +
                             aObjBoundRect.Height() + _rULSpacing.GetLower()
                             <= pObjPage->Frm().Height() + pObjPage->Frm().Top() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    else if ( _eHoriOrient == HORI_RIGHT )
                    {
                        SwTwips nTmp = nOtherTop - 1 - _rULSpacing.GetLower() -
                                       aObjBoundRect.Height() -
                                       rAnchorTxtFrm.Frm().Top();
                        if ( nTmp < nAdjustedRelPosX &&
                             rAnchorTxtFrm.Frm().Top() + nTmp - _rULSpacing.GetUpper()
                              >= pObjPage->Frm().Top() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    aTmpObjRect.Pos().Y() = rAnchorTxtFrm.Frm().Top() +
                                            nAdjustedRelPosX;
                }
            }
            else
            {
                const SvxLRSpaceItem& rOtherLR = pFly->GetFmt()->GetLRSpace();
                const SwTwips nOtherLeft = pFly->Frm().Left() - rOtherLR.GetLeft();
                const SwTwips nOtherRight = pFly->Frm().Right() + rOtherLR.GetRight();
                if( nOtherLeft <= aTmpObjRect.Right() + _rLRSpacing.GetRight() &&
                    nOtherRight >= aTmpObjRect.Left() - _rLRSpacing.GetLeft() )
                {
                    if ( _eHoriOrient == HORI_LEFT )
                    {
                        SwTwips nTmp = nOtherRight + 1 + _rLRSpacing.GetLeft() -
                                       rAnchorTxtFrm.Frm().Left();
                        if ( nTmp > nAdjustedRelPosX &&
                             rAnchorTxtFrm.Frm().Left() + nTmp +
                             aObjBoundRect.Width() + _rLRSpacing.GetRight()
                             <= pObjPage->Frm().Width() + pObjPage->Frm().Left() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    else if ( _eHoriOrient == HORI_RIGHT )
                    {
                        SwTwips nTmp = nOtherLeft - 1 - _rLRSpacing.GetRight() -
                                       aObjBoundRect.Width() -
                                       rAnchorTxtFrm.Frm().Left();
                        if ( nTmp < nAdjustedRelPosX &&
                             rAnchorTxtFrm.Frm().Left() + nTmp - _rLRSpacing.GetLeft()
                             >= pObjPage->Frm().Left() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    aTmpObjRect.Pos().X() = rAnchorTxtFrm.Frm().Left() +
                                            nAdjustedRelPosX;
                }
            } // end of <if (bVert)>
        } // end of <if _DrawAsideFly(..)>

        pFly = ((SwVirtFlyDrawObj*)aIter.Next())->GetFlyFrm();
    } // end of <loop on fly frames

    return nAdjustedRelPosX;
}

/** detemine, if object has to draw aside given fly frame

    method used by <_AdjustHoriRelPosForDrawAside(..)>

    @author OD
*/
bool SwAnchoredObjectPosition::_DrawAsideFly( const SwFlyFrm* _pFly,
                                              const SwRect&   _rObjRect,
                                              const SwFrm*    _pObjContext,
                                              const ULONG     _nObjIndex,
                                              const bool      _bEvenPage,
                                              const SwHoriOrient _eHoriOrient,
                                              const SwRelationOrient _eRelOrient
                                            ) const
{
    bool bRetVal = false;

    SWRECTFN( (&GetAnchorFrm()) )

    if ( _pFly->IsFlyAtCntFrm() &&
         (_pFly->Frm().*fnRect->fnBottomDist)( (_rObjRect.*fnRect->fnGetTop)() ) < 0 &&
         (_rObjRect.*fnRect->fnBottomDist)( (_pFly->Frm().*fnRect->fnGetTop)() ) < 0 &&
         ::FindKontext( _pFly->GetAnchorFrm(), FRM_COLUMN ) == _pObjContext )
    {
        ULONG nOtherIndex =
            static_cast<const SwTxtFrm*>(_pFly->GetAnchorFrm())->GetTxtNode()->GetIndex();
        if( _nObjIndex >= nOtherIndex )
        {
            const SwFmtHoriOrient& rHori = _pFly->GetFmt()->GetHoriOrient();
            SwRelationOrient eOtherRelOrient = rHori.GetRelationOrient();
            if( REL_CHAR != eOtherRelOrient )
            {
                SwHoriOrient eOtherHoriOrient = rHori.GetHoriOrient();
                _ToggleHoriOrientAndAlign( _bEvenPage && rHori.IsPosToggle(),
                                           eOtherHoriOrient,
                                           eOtherRelOrient );
                if ( eOtherHoriOrient == _eHoriOrient &&
                    _Minor( _eRelOrient, eOtherRelOrient, HORI_LEFT == _eHoriOrient ) )
                {
                    bRetVal = true;
                }
            }
        }
    }

    return bRetVal;
}

/** determine, if object has to draw aside another object

    the different alignments of the objects determines, if one has
    to draw aside another one. Thus, the given alignment are checked
    against each other, which one has to be drawn aside the other one.
    depending on parameter _bLeft check is done for left or right
    positioning.
    method used by <_DrawAsideFly(..)>

    @author OD
*/
bool SwAnchoredObjectPosition::_Minor( SwRelationOrient _eRelOrient1,
                                       SwRelationOrient _eRelOrient2,
                                       bool             _bLeft ) const
{
    bool bRetVal;

    // draw aside order for left horizontal position
    static USHORT __READONLY_DATA aLeft[ LAST_ENUM_DUMMY ] =
        { 5, 6, 0, 1, 8, 4, 7, 2, 3, 9 };
    // draw aside order for right horizontal position
    static USHORT __READONLY_DATA aRight[ LAST_ENUM_DUMMY ] =
        { 5, 6, 0, 8, 1, 7, 4, 2, 3, 9 };

    // decide depending on given order, which frame has to draw aside another frame
    if( _bLeft )
        bRetVal = aLeft[ _eRelOrient1 ] >= aLeft[ _eRelOrient2 ];
    else
        bRetVal = aRight[ _eRelOrient1 ] >= aRight[ _eRelOrient2 ];

    return bRetVal;
}
