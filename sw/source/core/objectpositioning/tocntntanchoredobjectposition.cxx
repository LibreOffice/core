/*************************************************************************
 *
 *  $RCSfile: tocntntanchoredobjectposition.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-03-08 14:02:17 $
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
#ifndef _TOCNTNTANCHOREDOBJECTPOSITION_HXX
#include <tocntntanchoredobjectposition.hxx>
#endif

#ifndef _FRAME_HXX
#include <frame.hxx>
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
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FMTSRND_HXX
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTFOLLOWTEXTFLOW_HXX
#include <fmtfollowtextflow.hxx>
#endif
#ifndef _FMTFSIZE_HXX
#include <fmtfsize.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
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
#ifndef _SVX_SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _ENVIRONMENTOFANCHOREDOBJECT
#include <environmentofanchoredobject.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif

using namespace objectpositioning;

SwToCntntAnchoredObjectPosition::SwToCntntAnchoredObjectPosition( SdrObject& _rDrawObj )
    : SwAnchoredObjectPosition ( _rDrawObj ),
      mpVertPosOrientFrm( 0 ),
      mbAnchorToChar ( false ),
      mpToCharOrientFrm( 0 ),
      mpToCharRect( 0 ),
      // OD 12.11.2003 #i22341#
      mnToCharTopOfLine( 0 )
{}

SwToCntntAnchoredObjectPosition::~SwToCntntAnchoredObjectPosition()
{}

bool SwToCntntAnchoredObjectPosition::IsAnchoredToChar() const
{
    return mbAnchorToChar;
}

const SwFrm* SwToCntntAnchoredObjectPosition::ToCharOrientFrm() const
{
    return mpToCharOrientFrm;
}

const SwRect* SwToCntntAnchoredObjectPosition::ToCharRect() const
{
    return mpToCharRect;
}

// OD 12.11.2003 #i22341#
SwTwips SwToCntntAnchoredObjectPosition::ToCharTopOfLine() const
{
    return mnToCharTopOfLine;
}

SwTxtFrm& SwToCntntAnchoredObjectPosition::GetAnchorTxtFrm() const
{
    ASSERT( GetAnchorFrm().ISA(SwTxtFrm),
            "SwToCntntAnchoredObjectPosition::GetAnchorTxtFrm() - wrong anchor frame type" );

    return static_cast<SwTxtFrm&>(GetAnchorFrm());
}

SwFlyAtCntFrm* SwToCntntAnchoredObjectPosition::GetFlyAtCntFrmOfObj() const
{
    ASSERT( !IsObjFly() || ( GetFrmOfObj() && GetFrmOfObj()->ISA(SwFlyAtCntFrm) ),
            "SwToCntntAnchoredObjectPosition::GetFlyAtCntFrmOfObj() - missing frame for object or wrong frame type" );

    SwFlyAtCntFrm* pRetFlyAtCntFrm =
        const_cast<SwFlyAtCntFrm*>(static_cast<const SwFlyAtCntFrm*>(GetFrmOfObj()));
    return pRetFlyAtCntFrm;
}

void SwToCntntAnchoredObjectPosition::CalcPosition()
{
    // get format of object
    const SwFrmFmt& rFrmFmt = GetFrmFmt();

    // declare and set <pFooter> to footer frame, if object is anchored
    // at a frame belonging to the footer.
    const SwFrm* pFooter = GetAnchorFrm().FindFooterOrHeader();
    if ( pFooter && !pFooter->IsFooterFrm() )
        pFooter = NULL;

    // declare and set <bBrowse> to true, if document is in browser mode and
    // object is anchored at the body, but not at frame belonging to a table.
    const bool bBrowse = GetAnchorFrm().IsInDocBody() &&
                         !GetAnchorFrm().IsInTab()
                            ? rFrmFmt.GetDoc()->IsBrowseMode()
                            : false;

    // declare and init <bInvalidatePage> to false, in order to invalidate
    // page size, if <bInvalidatePage> is set during the calculation of the
    // object position.
    bool bInvalidatePage = false;

    // determine left/right and its upper/lower spacing.
    const SvxLRSpaceItem &rLR = rFrmFmt.GetLRSpace();
    const SvxULSpaceItem &rUL = rFrmFmt.GetULSpace();

    // determine, if object has no surrounding.
    const SwFmtSurround& rSurround = rFrmFmt.GetSurround();
    const bool bNoSurround = rSurround.GetSurround() == SURROUND_NONE;
    const bool bWrapThrough = rSurround.GetSurround() == SURROUND_THROUGHT;

    // OD 30.09.2003 #i18732# - determine, if object has to follow the text flow
    const bool bFollowTextFlow = rFrmFmt.GetFollowTextFlow().GetValue();

    // OD 29.10.2003 #110978# - new class <SwEnvironmentOfAnchoredObject>
    SwEnvironmentOfAnchoredObject aEnvOfObj( bFollowTextFlow );

    // OD 30.09.2003 #i18732# - grow only, if object has to follow the text flow
    const bool bGrow = bFollowTextFlow &&
                       ( !GetAnchorFrm().IsInTab() ||
                         !rFrmFmt.GetFrmSize().GetHeightPercent() );

    // get text frame the object is anchored at
    const SwTxtFrm& rAnchorTxtFrm = GetAnchorTxtFrm();
    SWRECTFN( (&rAnchorTxtFrm) )

    // get frame for object
    SwFlyAtCntFrm* pFlyAtCntFrm = GetFlyAtCntFrmOfObj();
    if ( !pFlyAtCntFrm && !IsObjFly() )
    {
        ASSERT( false,
                "SwToCntntAnchoredObjectPosition::CalcPosition() - missing implementation for drawing objects" )
        return;
    }

    const SwRect aObjBoundRect( GetObject().GetCurrentBoundRect() );

    // local variable keeping the calculated relative position; initialized with
    // current relative position.
    Point aRelPos( pFlyAtCntFrm->aRelPos );

    // member varible keeping the frame the vertical position is oriented at;
    // initialized with the current one.
    mpVertPosOrientFrm = pFlyAtCntFrm->GetVertPosOrientFrm();

    SwTwips nRelDiff = 0;

    pFlyAtCntFrm->bValidPos = TRUE;

    bool bMoveable = rAnchorTxtFrm.IsMoveable();

    // determine frame the object position has to be oriented at.
    const SwFrm* pOrientFrm = &rAnchorTxtFrm;
    {
        // if object is at-character anchored, determine character-rectangle
        // and frame, position has to be oriented at.
        mbAnchorToChar = FLY_AUTO_CNTNT == rFrmFmt.GetAnchor().GetAnchorId();
        if ( mbAnchorToChar )
        {
            const SwFmtAnchor& rAnch = rFrmFmt.GetAnchor();
            if ( !pFlyAtCntFrm->maLastCharRect.Height() ||
                 !pFlyAtCntFrm->mnLastTopOfLine )
            {
                pFlyAtCntFrm->CheckCharRectAndTopOfLine();
                if ( !pFlyAtCntFrm->maLastCharRect.Height() ||
                     !pFlyAtCntFrm->mnLastTopOfLine )
                {
                    return;
                }
            }
            mpToCharRect = &(pFlyAtCntFrm->maLastCharRect);
            // OD 12.11.2003 #i22341# - get top of line, in which the anchor
            // character is.
            mnToCharTopOfLine = pFlyAtCntFrm->mnLastTopOfLine;
            pOrientFrm = &(const_cast<SwTxtFrm&>(rAnchorTxtFrm).GetFrmAtOfst(
                                rAnch.GetCntntAnchor()->nContent.GetIndex() ) );
            mpToCharOrientFrm = pOrientFrm;
        }
    }
    SWREFRESHFN( pOrientFrm )

    // determine vertical position
    {

        // determine vertical positioning and alignment attributes
        SwFmtVertOrient aVert( rFrmFmt.GetVertOrient() );
        bool bVertChgd = false;

        // OD 22.09.2003 #i18732# - determine layout frame for vertical
        // positions aligned to 'page areas'.
        const SwLayoutFrm& rPageAlignLayFrm =
                aEnvOfObj.GetVertEnvironmentLayoutFrm( *pOrientFrm, true );

        if ( aVert.GetVertOrient() != VERT_NONE )
        {
            // OD 22.09.2003 #i18732# - adjustments for follow text flow or not
            // AND vertical alignment at 'page areas'.
            SwTwips nAlignAreaHeight;
            SwTwips nAlignAreaOffset;
            _GetVertAlignmentValues( *pOrientFrm, rPageAlignLayFrm,
                                     aVert.GetRelationOrient(),
                                     nAlignAreaHeight, nAlignAreaOffset );

            // determine relative vertical position
            SwTwips nRelPosY = nAlignAreaOffset;
            SwTwips nObjHeight = (aObjBoundRect.*fnRect->fnGetHeight)();
            SwTwips nUpperSpace = bVert ? rLR.GetRight() : rUL.GetUpper();
            SwTwips nLowerSpace = bVert ? rLR.GetLeft() : rUL.GetLower();
            switch ( aVert.GetVertOrient() )
            {
                case VERT_CHAR_BOTTOM:
                {
                    if ( mbAnchorToChar )
                    {
                        // bottom (to character anchored)
                        nRelPosY += nAlignAreaHeight + nUpperSpace;
                        if ( bVert )
                            nRelPosY += nObjHeight;
                        break;
                    }
                }
                // no break here
                case VERT_TOP:
                {
                    // OD 12.11.2003 #i22341# - special case for vertical
                    // alignment at top of line
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == REL_VERT_LINE )
                    {
                        nRelPosY -= (nObjHeight + nLowerSpace);
                    }
                    else
                    {
                        nRelPosY += nUpperSpace;
                    }
                }
                break;
                // OD 14.11.2003 #i22341#
                case VERT_LINE_TOP:
                {
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == REL_VERT_LINE )
                    {
                        nRelPosY -= (nObjHeight + nLowerSpace);
                    }
                    else
                    {
                        ASSERT( false,
                                "<SwToCntntAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
                    }
                }
                break;
                case VERT_CENTER:
                {
                    nRelPosY += (nAlignAreaHeight / 2) - (nObjHeight / 2);
                }
                break;
                // OD 14.11.2003 #i22341#
                case VERT_LINE_CENTER:
                {
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == REL_VERT_LINE )
                    {
                        nRelPosY += (nAlignAreaHeight / 2) - (nObjHeight / 2);
                    }
                    else
                    {
                        ASSERT( false,
                                "<SwToCntntAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
                    }
                }
                break;
                case VERT_BOTTOM:
                {
                    if ( ( aVert.GetRelationOrient() == FRAME ||
                           aVert.GetRelationOrient() == PRTAREA ) &&
                         bNoSurround )
                    {
                        // bottom (aligned to 'paragraph areas')
                        nRelPosY += nAlignAreaHeight + nUpperSpace;
                    }
                    else
                    {
                        // OD 12.11.2003 #i22341# - special case for vertical
                        // alignment at top of line
                        if ( mbAnchorToChar &&
                             aVert.GetRelationOrient() == REL_VERT_LINE )
                        {
                            nRelPosY += nUpperSpace;
                        }
                        else
                        {
                            nRelPosY += nAlignAreaHeight -
                                        ( nObjHeight + nLowerSpace );
                        }
                    }
                }
                break;
                // OD 14.11.2003 #i22341#
                case VERT_LINE_BOTTOM:
                {
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == REL_VERT_LINE )
                    {
                        nRelPosY += nUpperSpace;
                    }
                    else
                    {
                        ASSERT( false,
                                "<SwToCntntAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
                    }
                }
                break;
            }

            // adjust relative position by distance between anchor frame and
            // the frame, the object is oriented at.
            if ( pOrientFrm != &rAnchorTxtFrm )
            {
                const SwTwips nTopOfOrient = (pOrientFrm->Frm().*fnRect->fnGetTop)();
                nRelPosY += (*fnRect->fnYDiff)( nTopOfOrient,
                                      (rAnchorTxtFrm.Frm().*fnRect->fnGetTop)());
            }

            // ??? Why saving calculated relative position
            // keep calculated relative vertical position
            if ( nRelPosY != aVert.GetPos() )
            {
                aVert.SetPos( nRelPosY );
                bVertChgd = true;
            }

            // determine absolute 'vertical' position, depending on layout-direction
            if( bVert )
                aRelPos.X() = nRelPosY;
            else
                aRelPos.Y() = nRelPosY;
        }

        // OD 29.10.2003 #110978# - determine upper of frame vertical position
        // is oriented at.
        const SwLayoutFrm* pUpperOfOrientFrm = 0L;
        if ( bFollowTextFlow )
        {
            pUpperOfOrientFrm = aVert.GetVertOrient() == VERT_NONE
                                ? rAnchorTxtFrm.GetUpper()
                                : pOrientFrm->GetUpper();
        }
        else
        {
            pUpperOfOrientFrm = pOrientFrm->GetUpper();
        }
        // ignore one-column sections.
        while ( pUpperOfOrientFrm->IsSctFrm() )
        {
            pUpperOfOrientFrm = pUpperOfOrientFrm->GetUpper();
        }

        if ( aVert.GetVertOrient() == VERT_NONE )
        {
            // local variable <nRelPosY> for calculation of relative vertical
            // distance to anchor.
            SwTwips nRelPosY = 0;
            // OD 12.11.2003 #i22341# - add special case for vertical alignment
            // at top of line.
            if ( mbAnchorToChar &&
                 ( aVert.GetRelationOrient() == REL_CHAR ||
                   aVert.GetRelationOrient() == REL_VERT_LINE ) )
            {
                if ( aVert.GetRelationOrient() == REL_CHAR )
                {
                    nRelPosY = (*fnRect->fnYDiff)(
                                    (ToCharRect()->*fnRect->fnGetBottom)(),
                                    (pOrientFrm->Frm().*fnRect->fnGetTop)() );
                }
                else
                {
                    nRelPosY = (*fnRect->fnYDiff)(
                                    ToCharTopOfLine(),
                                    (pOrientFrm->Frm().*fnRect->fnGetTop)() );
                }
                nRelPosY -= aVert.GetPos();
                if( pOrientFrm != &rAnchorTxtFrm )
                {
                    const SwTxtFrm* pTmp = &rAnchorTxtFrm;
                    SWREFRESHFN( pTmp )
                    nRelPosY -=(*fnRect->fnYDiff)((pTmp->Frm().*fnRect->fnGetTop)(),
                                    (pTmp->GetUpper()->*fnRect->fnGetPrtTop)());
                    while( pTmp != pOrientFrm )
                    {
                        SWREFRESHFN( pTmp )
                        nRelPosY += (pTmp->GetUpper()->Prt().*fnRect->fnGetHeight)();
                        pTmp = pTmp->GetFollow();
                    }
                    SWREFRESHFN( pTmp )
                }
            }
            else
            {
                nRelPosY = aVert.GetPos();
                // OD 02.10.2002 #102646# - increase <nRelPosY> by margin height,
                // if position is vertical aligned to "paragraph text area"
                if ( aVert.GetRelationOrient() == PRTAREA )
                {
                    nRelPosY += (pOrientFrm->*fnRect->fnGetTopMargin)();
                }
                // OD 22.09.2003 #i18732# - adjust <nRelPosY> by difference
                // between 'page area' and 'anchor' frame, if position is
                // vertical aligned to 'page areas'
                else if ( aVert.GetRelationOrient() == REL_PG_FRAME )
                {
                    nRelPosY += (*fnRect->fnYDiff)(
                                    (rPageAlignLayFrm.Frm().*fnRect->fnGetTop)(),
                                    (pOrientFrm->Frm().*fnRect->fnGetTop)() );
                }
                else if ( aVert.GetRelationOrient() == REL_PG_PRTAREA )
                {
                    SwRect aPgPrtRect( rPageAlignLayFrm.Frm() );
                    if ( rPageAlignLayFrm.IsPageFrm() )
                    {
                        aPgPrtRect =
                            static_cast<const SwPageFrm&>(rPageAlignLayFrm).PrtWithoutHeaderAndFooter();
                    }
                    nRelPosY += (*fnRect->fnYDiff)(
                                    (aPgPrtRect.*fnRect->fnGetTop)(),
                                    (pOrientFrm->Frm().*fnRect->fnGetTop)() );
                }

                if ( pOrientFrm != &rAnchorTxtFrm )
                {
                    const SwTwips nTopOfOrient = (pOrientFrm->Frm().*fnRect->fnGetTop)();
                    nRelPosY += (*fnRect->fnYDiff)( nTopOfOrient,
                                          (rAnchorTxtFrm.Frm().*fnRect->fnGetTop)());
                }
            }

            // <pUpperOfOrientFrm>: layout frame, at which the position has to
            //                      is oriented at
            // <nRelPosY>:          rest of the relative distance in the current
            //                      layout frame
            // <nAvail>:            space, which is available in the current
            //                      layout frame

            if( nRelPosY <= 0 )
            {
                // OD 08.09.2003 #110354# - allow negative position, but keep it
                // inside environment layout frame.
                const SwFrm& rVertEnvironLayFrm =
                    aEnvOfObj.GetVertEnvironmentLayoutFrm( *pUpperOfOrientFrm, false );
                nRelPosY = _AdjustVertRelPos( rVertEnvironLayFrm, nRelPosY );
                if( bVert )
                    aRelPos.X() = nRelPosY;
                else
                    aRelPos.Y() = nRelPosY;
            }
            else
            {
                SWREFRESHFN( (&rAnchorTxtFrm) )
                SwTwips nAvail =
                    (*fnRect->fnYDiff)( (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)(),
                                        (rAnchorTxtFrm.Frm().*fnRect->fnGetTop)() );
                const bool bInFtn = rAnchorTxtFrm.IsInFtn();
                while ( nRelPosY )
                {
                    if ( nRelPosY <= nAvail ||
                         ( bBrowse &&
                           ( const_cast<SwLayoutFrm*>(pUpperOfOrientFrm)->Grow( nRelPosY-nAvail, TRUE) ) ) ||
                         ( pUpperOfOrientFrm->IsInTab() && bGrow &&
                           ( const_cast<SwLayoutFrm*>(pUpperOfOrientFrm)->Grow( nRelPosY-nAvail, TRUE) ) ) )
                    {
                        if( bVert )
                            aRelPos.X() = rAnchorTxtFrm.Frm().Left() +
                                          rAnchorTxtFrm.Frm().Width() -
                                          pUpperOfOrientFrm->Frm().Left() -
                                          pUpperOfOrientFrm->Prt().Left() -
                                          nAvail + nRelPosY;
                        else
                            aRelPos.Y() = pUpperOfOrientFrm->Frm().Top() +
                                          pUpperOfOrientFrm->Prt().Top() +
                                          pUpperOfOrientFrm->Prt().Height() -
                                          nAvail + nRelPosY -
                                          rAnchorTxtFrm.Frm().Top();

                        if ( ( bBrowse ||
                               ( pUpperOfOrientFrm->IsInTab() && bGrow ) ) &&
                             nRelPosY - nAvail > 0 )
                        {
                            const_cast<SwLayoutFrm*>(pUpperOfOrientFrm)->Grow( nRelPosY-nAvail );
                            // Why validation of page frame???
                            /*
                            SwPageFrm* pTmp = const_cast<SwPageFrm*>(pUpperOfOrientFrm->FindPageFrm());
                            ::ValidateSz( pTmp );
                            bInvalidatePage = true;
                            */
                        }
                        nRelPosY = 0;
                    }
                    else
                    {
                        if ( bFollowTextFlow &&
                             !( aVert.GetRelationOrient() == REL_PG_FRAME ||
                                aVert.GetRelationOrient() == REL_PG_PRTAREA ) )
                        {
                            if ( bMoveable )
                            {
                                // follow the text flow
                                nRelPosY -= nAvail;
                                MakePageType eMakePage = bInFtn ? MAKEPAGE_NONE
                                                                : MAKEPAGE_APPEND;
                                const bool bInSct = pUpperOfOrientFrm->IsInSct();
                                if( bInSct )
                                    eMakePage = MAKEPAGE_NOSECTION;

                                const SwLayoutFrm* pTmp =
                                    pUpperOfOrientFrm->GetLeaf( eMakePage, TRUE, &rAnchorTxtFrm );
                                if ( pTmp &&
                                     ( !bInSct ||
                                       pUpperOfOrientFrm->FindSctFrm()->IsAnFollow( pTmp->FindSctFrm() ) ) )
                                {
                                    pUpperOfOrientFrm = pTmp;
                                    bMoveable = rAnchorTxtFrm.IsMoveable( (SwLayoutFrm*)pUpperOfOrientFrm );
                                    SWREFRESHFN( pUpperOfOrientFrm )
                                    nAvail = (pUpperOfOrientFrm->Prt().*fnRect->fnGetHeight)();
                                }
                                else
                                {
                                    // if there isn't enough space in the (colmuned)
                                    // section, leave it and set available space <nAvail>
                                    // to the space below the section.
                                    // if the new available space isn't also enough,
                                    // new pages can be created.
                                    if( bInSct )
                                    {
                                        const SwFrm* pSct = pUpperOfOrientFrm->FindSctFrm();
                                        pUpperOfOrientFrm = pSct->GetUpper();
                                        nAvail = (*fnRect->fnYDiff)(
                                                   (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)(),
                                                   (pSct->*fnRect->fnGetPrtBottom)() );
                                    }
                                    else
                                    {
                                        nRelDiff = nRelPosY;
                                        nRelPosY = 0;
                                    }
                                }
                            }
                            else
                            {
                                nRelPosY = 0;
                            }
                        }
                        else
                        {
                            // OD 06.10.2003 #i18732# - do not follow text flow respectively
                            // align at 'page areas', but stay inside given environment
                            const SwFrm& rVertEnvironLayFrm =
                                aEnvOfObj.GetVertEnvironmentLayoutFrm( *pUpperOfOrientFrm, false );
                            nRelPosY = _AdjustVertRelPos( rVertEnvironLayFrm, nRelPosY );
                            if( bVert )
                                aRelPos.X() = nRelPosY;
                            else
                                aRelPos.Y() = nRelPosY;
                            nRelPosY = 0;
                        }
                    }
                } // end of <while ( nRelPosY )>
            } // end of else <nRelPosY <= 0>
        } // end of <aVert.GetVertOrient() == VERT_NONE>
        //Damit das Teil ggf. auf die richtige Seite gestellt und in die
        //PrtArea des LayLeaf gezogen werden kann, muss hier seine
        //absolute Position berechnet werden.
        if( bVert )
        {
            pFlyAtCntFrm->Frm().Pos().X() = rAnchorTxtFrm.Frm().Left() -
                                            aObjBoundRect.Width() +
                                            rAnchorTxtFrm.Frm().Width() -
                                            aRelPos.X() + nRelDiff;
        }
        else
            pFlyAtCntFrm->Frm().Pos().Y() = rAnchorTxtFrm.Frm().Top() +
                                            ( aRelPos.Y() - nRelDiff );

        // grow environment under certain conditions
        while ( pUpperOfOrientFrm->IsSctFrm() )
            pUpperOfOrientFrm = pUpperOfOrientFrm->GetUpper();
        SwTwips nDist = (pFlyAtCntFrm->Frm().*fnRect->fnBottomDist)(
                          (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)() );
        if( nDist < 0 )
        {
            if ( ( bBrowse && rAnchorTxtFrm.IsMoveable() ) ||
                 ( rAnchorTxtFrm.IsInTab() && bGrow ) )
            {
                const_cast<SwLayoutFrm*>(pUpperOfOrientFrm)->Grow( -nDist );
                // Why validation of page frame???
                /*
                SwPageFrm* pTmp = const_cast<SwPageFrm*>(pUpperOfOrientFrm->FindPageFrm());
                ::ValidateSz( pTmp );
                bInvalidatePage = true;
                */
            }
        }

        if ( bFollowTextFlow &&
             !( aVert.GetRelationOrient() == REL_PG_FRAME ||
                aVert.GetRelationOrient() == REL_PG_PRTAREA ) )
        {
            // follow text flow
            const bool bInFtn = rAnchorTxtFrm.IsInFtn();
            nDist = (pFlyAtCntFrm->Frm().*fnRect->fnBottomDist)(
                      (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)() );
            while( bMoveable && nDist < 0 )
            {
                bool bInSct = pUpperOfOrientFrm->IsInSct();
                if ( bInSct )
                {
                    const SwLayoutFrm* pTmp = pUpperOfOrientFrm->FindSctFrm()->GetUpper();
                    nDist = (pFlyAtCntFrm->Frm().*fnRect->fnBottomDist)(
                              (pTmp->*fnRect->fnGetPrtBottom)() );
                    if( nDist < 0 )
                        pUpperOfOrientFrm = pTmp;
                    else
                        break;
                    bInSct = pUpperOfOrientFrm->IsInSct();
                }
                if ( !bInSct &&
                     (pFlyAtCntFrm->Frm().*fnRect->fnGetTop)() ==
                             (pUpperOfOrientFrm->*fnRect->fnGetPrtTop)() )
                    //Das teil passt nimmer, da hilft auch kein moven.
                    break;

                const SwLayoutFrm* pNextLay = pUpperOfOrientFrm->GetLeaf(
                                ( bInSct
                                  ? MAKEPAGE_NOSECTION
                                  : ( bInFtn ? MAKEPAGE_NONE : MAKEPAGE_APPEND ) ),
                                TRUE, &rAnchorTxtFrm );
                // OD 06.10.2003 #110978# - correction:
                // If anchor is in footnote and proposed next layout environment
                // isn't a footnote frame, object can't follow the text flow
                if ( bInFtn && pNextLay && !pNextLay->IsFtnFrm() )
                {
                    pNextLay = 0L;
                }
                if ( pNextLay )
                {
                    SWRECTFNX( pNextLay )
                    if ( !bInSct ||
                         ( pUpperOfOrientFrm->FindSctFrm()->IsAnFollow( pNextLay->FindSctFrm() ) &&
                           (pNextLay->Prt().*fnRectX->fnGetHeight)() ) )
                    {
                        if( bVertX )
                            aRelPos.X() = rAnchorTxtFrm.Frm().Left() +
                                          rAnchorTxtFrm.Frm().Width() -
                                          pNextLay->Frm().Left() -
                                          pNextLay->Prt().Left() -
                                          pNextLay->Prt().Width();
                        else
                            aRelPos.Y() = pNextLay->Frm().Top() +
                                          pNextLay->Prt().Top() -
                                          rAnchorTxtFrm.Frm().Top();
                        pUpperOfOrientFrm = pNextLay;
                        SWREFRESHFN( pUpperOfOrientFrm )
                        bMoveable = rAnchorTxtFrm.IsMoveable( (SwLayoutFrm*)pUpperOfOrientFrm );
                        if( bVertX )
                            pFlyAtCntFrm->Frm().Pos().X() =
                                            rAnchorTxtFrm.Frm().Left() +
                                            rAnchorTxtFrm.Frm().Width() -
                                            aRelPos.X() -
                                            aObjBoundRect.Width();
                        else
                            pFlyAtCntFrm->Frm().Pos().Y() =
                                            rAnchorTxtFrm.Frm().Top() +
                                            aRelPos.Y();
                        nDist = (pFlyAtCntFrm->Frm().*fnRect->fnBottomDist)(
                                  (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)() );
                    }
                }
                else if ( bInSct )
                {
                    // Wenn wir innerhalb des Bereich nicht genug Platz haben, gucken
                    // wir uns mal die Seite an.
                    const SwLayoutFrm* pTmp = pUpperOfOrientFrm->FindSctFrm()->GetUpper();
                    nDist = (pFlyAtCntFrm->Frm().*fnRect->fnBottomDist)(
                              (pTmp->*fnRect->fnGetPrtBottom)() );
                    if( nDist < 0 )
                        pUpperOfOrientFrm = pTmp;
                    else
                        break;
                }
                else
                    bMoveable = false;
            }
        }
        pFlyAtCntFrm->AssertPage();

        // save calculated vertical position
        const_cast<SwFrmFmt&>(rFrmFmt).LockModify();
        if ( bVertChgd )
            const_cast<SwFrmFmt&>(rFrmFmt).SetAttr( aVert );
        const_cast<SwFrmFmt&>(rFrmFmt).UnlockModify();

        // keep layout frame vertical position is oriented at.
        mpVertPosOrientFrm = pUpperOfOrientFrm;

    }

    // determine 'horizontal' position
    {
        // determine horizontal positioning and alignment attributes
        SwFmtHoriOrient aHori( rFrmFmt.GetHoriOrient() );
        bool bHoriChgd = false;

        // set calculated vertical position in order to determine correct
        // frame, the horizontal position is oriented at.
        if( bVert )
            pFlyAtCntFrm->Frm().Pos().X() = rAnchorTxtFrm.Frm().Left() -
                                            aObjBoundRect.Width() +
                                            rAnchorTxtFrm.Frm().Width() -
                                            aRelPos.X();
        else
            pFlyAtCntFrm->Frm().Pos().Y() = aRelPos.Y() +
                                            rAnchorTxtFrm.Frm().Top();

        // determine frame, horizontal position is oriented at.
        const SwFrm* pHoriOrientFrm = &_GetHoriVirtualAnchor( *mpVertPosOrientFrm );

        SwTwips nRelPosX = _CalcRelPosX( *pHoriOrientFrm, aEnvOfObj,
                                         aHori, rLR, rUL, bWrapThrough,
                                         ( bVert ? aRelPos.X() : aRelPos.Y() ) );

        if( bVert )
            aRelPos.Y() = nRelPosX;
        else
            aRelPos.X() = nRelPosX;

        if ( HORI_NONE != aHori.GetHoriOrient() &&
             aHori.GetPos() != nRelPosX )
        {
            aHori.SetPos( nRelPosX );
            bHoriChgd = true;
        }

        // WHY ???
        // save calculated horizontal position
        const_cast<SwFrmFmt&>(rFrmFmt).LockModify();
        if ( bHoriChgd )
            const_cast<SwFrmFmt&>(rFrmFmt).SetAttr( aHori );
        const_cast<SwFrmFmt&>(rFrmFmt).UnlockModify();
    }

    pFlyAtCntFrm->AssertPage();

    // set absolute position at object
    if( bVert )
    {
        pFlyAtCntFrm->Frm().Pos().X() = rAnchorTxtFrm.Frm().Left() +
                                        rAnchorTxtFrm.Frm().Width() -
                                        aObjBoundRect.Width() -
                                        aRelPos.X();
        pFlyAtCntFrm->Frm().Pos().Y() = rAnchorTxtFrm.Frm().Top() +
                                        aRelPos.Y();
        pFlyAtCntFrm->AssertPage();
    }
    else
    {
        pFlyAtCntFrm->Frm().Pos( aRelPos + rAnchorTxtFrm.Frm().Pos() );
    }

    // set relative position at object
    pFlyAtCntFrm->aRelPos = aRelPos;

    // invalidate page size, if needed
    if ( bInvalidatePage )
    {
        pFlyAtCntFrm->FindPageFrm()->InvalidateSize();
    }
}

/** determine frame for horizontal position

    @author OD
*/
const SwFrm& SwToCntntAnchoredObjectPosition::_GetHoriVirtualAnchor(
                                        const SwLayoutFrm& _rProposedFrm ) const
{
    const SwFrm* pHoriVirtAnchFrm = &_rProposedFrm;

    // Search for first lower content frame, which is the anchor or a follow
    // of the anchor (Note: <Anchor.IsAnFollow( Anchor )> is true)
    // If none found, <_rProposedFrm> is returned.
    const SwFrm* pFrm = _rProposedFrm.Lower();
    while ( pFrm )
    {
        if ( pFrm->IsCntntFrm() &&
             GetAnchorTxtFrm().IsAnFollow( static_cast<const SwCntntFrm*>(pFrm) ) )
        {
            pHoriVirtAnchFrm = pFrm;
            break;
        }
        pFrm = pFrm->GetNext();
    }

    return *pHoriVirtAnchFrm;
}

const SwLayoutFrm& SwToCntntAnchoredObjectPosition::GetVertPosOrientFrm() const
{
    return *mpVertPosOrientFrm;
}

// **************************************************************************
// method incl. helper methods for adjusting proposed horizontal position,
// if object has to draw aside another object.
// **************************************************************************
/** adjust calculated horizontal position in order to draw object
    aside other objects with same positioning

    @author OD
*/
SwTwips SwToCntntAnchoredObjectPosition::_AdjustHoriRelPosForDrawAside(
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
    const SwTxtFrm&      rAnchorTxtFrm = GetAnchorTxtFrm();
    const SwFlyAtCntFrm* pFlyAtCntFrm = GetFlyAtCntFrmOfObj();
    const SwRect         aObjBoundRect( GetObject().GetCurrentBoundRect() );
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
    const SwPageFrm* pObjPage = pFlyAtCntFrm->FindPageFrm();
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

    OD 11.08.2003 #110978#
    method used by <_AdjustHoriRelPosForDrawAside(..)>

    @author OD
*/
bool SwToCntntAnchoredObjectPosition::_DrawAsideFly( const SwFlyFrm* _pFly,
                                              const SwRect&   _rObjRect,
                                              const SwFrm*    _pObjContext,
                                              const ULONG     _nObjIndex,
                                              const bool      _bEvenPage,
                                              const SwHoriOrient _eHoriOrient,
                                              const SwRelationOrient _eRelOrient
                                            ) const
{
    bool bRetVal = false;

    SWRECTFN( (&GetAnchorTxtFrm()) )

    if ( _pFly->IsFlyAtCntFrm() &&
         (_pFly->Frm().*fnRect->fnBottomDist)( (_rObjRect.*fnRect->fnGetTop)() ) < 0 &&
         (_rObjRect.*fnRect->fnBottomDist)( (_pFly->Frm().*fnRect->fnGetTop)() ) < 0 &&
         ::FindKontext( _pFly->GetAnchor(), FRM_COLUMN ) == _pObjContext )
    {
        ULONG nOtherIndex =
            static_cast<const SwTxtFrm*>(_pFly->GetAnchor())->GetTxtNode()->GetIndex();
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

    OD 04.08.2003
    the different alignments of the objects determines, if one has
    to draw aside another one. Thus, the given alignment are checked
    against each other, which one has to be drawn aside the other one.
    depending on parameter _bLeft check is done for left or right
    positioning.
    method used by <_DrawAsideFly(..)>

    @author OD
*/
bool SwToCntntAnchoredObjectPosition::_Minor( SwRelationOrient _eRelOrient1,
                                              SwRelationOrient _eRelOrient2,
                                              bool             _bLeft ) const
{
    bool bRetVal;

    // draw aside order for left horizontal position
    static USHORT __READONLY_DATA aLeft[ LAST_ENUM_DUMMY ] =
        { 5, 6, 0, 1, 8, 4, 7, 2, 3 };
    // draw aside order for right horizontal position
    static USHORT __READONLY_DATA aRight[ LAST_ENUM_DUMMY ] =
        { 5, 6, 0, 8, 1, 7, 4, 2, 3 };

    // decide depending on given order, which frame has to draw aside another frame
    if( _bLeft )
        bRetVal = aLeft[ _eRelOrient1 ] >= aLeft[ _eRelOrient2 ];
    else
        bRetVal = aRight[ _eRelOrient1 ] >= aRight[ _eRelOrient2 ];

    return bRetVal;
}
