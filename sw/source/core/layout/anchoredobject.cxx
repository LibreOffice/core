/*************************************************************************
 *
 *  $RCSfile: anchoredobject.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 13:06:55 $
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
#ifndef _ANCHOREDOBJECT_HXX
#include <anchoredobject.hxx>
#endif

#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _FRAME_HXX
#include <frame.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif

TYPEINIT0(SwAnchoredObject);

SwAnchoredObject::SwAnchoredObject() :
    mpDrawObj( 0L ),
    mpAnchorFrm( 0L ),
    maLastCharRect(),
    mnLastTopOfLine( 0L ),
    mpVertPosOrientFrm( 0L )
{
}

SwAnchoredObject::~SwAnchoredObject()
{
}

// =============================================================================
// accessors for member <mpDrawObj>
// =============================================================================
void SwAnchoredObject::SetDrawObj( SdrObject& _rDrawObj )
{
    mpDrawObj = &_rDrawObj;
}

const SdrObject* SwAnchoredObject::GetDrawObj() const
{
    return mpDrawObj;
}

SdrObject* SwAnchoredObject::DrawObj()
{
    return mpDrawObj;
}

// =============================================================================
// accessors for member <mpAnchorFrm>
// =============================================================================
const SwFrm* SwAnchoredObject::GetAnchorFrm() const
{
    return mpAnchorFrm;
}

SwFrm* SwAnchoredObject::AnchorFrm()
{
    return mpAnchorFrm;
}

void SwAnchoredObject::ChgAnchorFrm( SwFrm* _pNewAnchorFrm )
{
    mpAnchorFrm = _pNewAnchorFrm;

    ObjectAttachedToAnchorFrame();
}

// =============================================================================
// accessors for member <maLastCharRect>
// =============================================================================
const SwRect& SwAnchoredObject::GetLastCharRect() const
{
    return maLastCharRect;
}

SwTwips SwAnchoredObject::GetLastCharX() const
{
    return maLastCharRect.Left() - GetObjRect().Left();
}

SwTwips SwAnchoredObject::GetRelCharX( const SwFrm* pFrm ) const
{
    return maLastCharRect.Left() - pFrm->Frm().Left();
}

SwTwips SwAnchoredObject::GetRelCharY( const SwFrm* pFrm ) const
{
    return maLastCharRect.Bottom() - pFrm->Frm().Top();
}

void SwAnchoredObject::AddLastCharY( long nDiff )
{
    maLastCharRect.Pos().Y() += nDiff;
}

void SwAnchoredObject::ResetLastCharRectHeight()
{
    maLastCharRect.Height( 0 );
}
// =============================================================================
// accessors for member <mpVertPosOrientFrm>
// =============================================================================
const SwLayoutFrm* SwAnchoredObject::GetVertPosOrientFrm() const
{
    return mpVertPosOrientFrm;
}

void SwAnchoredObject::SetVertPosOrientFrm( const SwLayoutFrm& _rVertPosOrientFrm )
{
    mpVertPosOrientFrm = &_rVertPosOrientFrm;
}

// =============================================================================
// accessors for member <mnLastTopOfLine>
// =============================================================================
const SwTwips SwAnchoredObject::GetLastTopOfLine() const
{
    return mnLastTopOfLine;
}

/** check anchor character rectangle and top of line

    OD 2004-03-24 #i26791
    For to-character anchored Writer fly frames the members <maLastCharRect>
    and <maLastTopOfLine> are updated. These are checked for change and
    depending on the applied positioning, it's decided, if the Writer fly
    frame has to be invalidated.
    OD 2004-07-15 #117380#
    add parameter <_bCheckForParaPorInf>, default value <true>

    @author OD
*/
void SwAnchoredObject::CheckCharRectAndTopOfLine(
                                        const bool _bCheckForParaPorInf )
{
    if ( GetAnchorFrm() &&
         GetAnchorFrm()->IsTxtFrm() )
    {
        const SwFmtAnchor& rAnch = GetFrmFmt().GetAnchor();
        if ( rAnch.GetAnchorId() == FLY_AUTO_CNTNT &&
             rAnch.GetCntntAnchor() )
        {
            // --> OD 2004-07-14 #117380# - if requested, assure that anchor frame,
            // which contains the anchor character, has a paragraph portion information.
            // The paragraph portion information is needed to determine the
            // anchor character rectangle respectively the top of the line.
            // Thus, a format of this frame is avoided to determine the
            // paragraph portion information.
            xub_StrLen nOffset = rAnch.GetCntntAnchor()->nContent.GetIndex();
            const SwTxtFrm& aAnchorCharFrm =
                    static_cast<SwTxtFrm*>(AnchorFrm())->GetFrmAtOfst( nOffset );
            if ( !_bCheckForParaPorInf || aAnchorCharFrm.HasPara() )
            {
                _CheckCharRect( rAnch, aAnchorCharFrm );
                _CheckTopOfLine( rAnch, aAnchorCharFrm );
            }
            // <--
        }
    }
}

/** check anchor character rectangle

    OD 11.11.2003 #i22341#
    helper method for method <CheckCharRectAndTopOfLine()>
    For to-character anchored Writer fly frames the member <maLastCharRect>
    is updated. This is checked for change and depending on the applied
    positioning, it's decided, if the Writer fly frame has to be invalidated.
    OD 2004-07-14 #117380#
    improvement - add second parameter <_rAnchorCharFrm>

    @author OD
*/
void SwAnchoredObject::_CheckCharRect( const SwFmtAnchor& _rAnch,
                                       const SwTxtFrm& _rAnchorCharFrm )
{
    // determine rectangle of anchor character. If not exist, abort operation
    SwRect aCharRect;
    if ( !_rAnchorCharFrm.GetAutoPos( aCharRect, *_rAnch.GetCntntAnchor() ) )
    {
        return;
    }
    // check, if anchor character rectangle has changed
    if ( aCharRect != maLastCharRect )
    {
        // check positioning and alignment for invalidation of position
        {
            SWRECTFN( (&_rAnchorCharFrm) );
            // determine positioning and alignment
            SwFmtVertOrient aVert( GetFrmFmt().GetVertOrient() );
            SwFmtHoriOrient aHori( GetFrmFmt().GetHoriOrient() );
            // check for anchor character rectangle changes for certain
            // positionings and alignments
            // OD 07.10.2003 #110978# - add condition to invalidate position,
            // if vertical aligned at frame/page area and vertical position
            // of anchor character has changed.
            const SwRelationOrient eVertRelOrient = aVert.GetRelationOrient();
            if ( ( aHori.GetRelationOrient() == REL_CHAR &&
                   (aCharRect.*fnRect->fnGetLeft)() !=
                        (maLastCharRect.*fnRect->fnGetLeft)() ) ||
                 ( eVertRelOrient == REL_CHAR &&
                   ( (aCharRect.*fnRect->fnGetTop)() !=
                        (maLastCharRect.*fnRect->fnGetTop)() ||
                     (aCharRect.*fnRect->fnGetHeight)() !=
                        (maLastCharRect.*fnRect->fnGetHeight)() ) ) ||
                 ( ( ( eVertRelOrient == FRAME ) ||
                     ( eVertRelOrient == PRTAREA ) ||
                     ( eVertRelOrient == REL_PG_FRAME ) ||
                     ( eVertRelOrient == REL_PG_PRTAREA ) ) &&
                   ( (aCharRect.*fnRect->fnGetTop)() !=
                        (maLastCharRect.*fnRect->fnGetTop)() ) ) )
            {
                InvalidateObjPos();
            }
        }
        // keep new anchor character rectangle
        maLastCharRect = aCharRect;
    }
}

/** check top of line

    OD 11.11.2003 #i22341#
    helper method for method <CheckCharRectAndTopOfLine()>
    For to-character anchored Writer fly frames the member <mnLastTopOfLine>
    is updated. This is checked for change and depending on the applied
    positioning, it's decided, if the Writer fly frame has to be invalidated.
    OD 2004-07-14 #117380#
    improvement - add second parameter <_rAnchorCharFrm>

    @author OD
*/
void SwAnchoredObject::_CheckTopOfLine( const SwFmtAnchor& _rAnch,
                                        const SwTxtFrm& _rAnchorCharFrm )
{
    SwTwips nTopOfLine = 0L;
    if ( _rAnchorCharFrm.GetTopOfLine( nTopOfLine, *_rAnch.GetCntntAnchor() ) )
    {
        if ( nTopOfLine != mnLastTopOfLine )
        {
            // check alignment for invalidation of position
            if ( GetFrmFmt().GetVertOrient().GetRelationOrient() == REL_VERT_LINE )
            {
                InvalidateObjPos();
            }
            // keep new top of line value
            mnLastTopOfLine = nTopOfLine;
        }
    }
}

void SwAnchoredObject::SetPositioningInProgress( const bool _bPosInProgress )
{
    // empty method body
}

bool SwAnchoredObject::IsPositioningInProgress() const
{
    return false;
}

const Point SwAnchoredObject::GetCurrRelPos() const
{
    return maRelPos;
}
void SwAnchoredObject::SetCurrRelPos( Point _aRelPos )
{
    maRelPos = _aRelPos;
}

void SwAnchoredObject::ObjectAttachedToAnchorFrame()
{
    // default behaviour: nothing to do
}
