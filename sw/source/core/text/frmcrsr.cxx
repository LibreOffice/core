/*************************************************************************
 *
 *  $RCSfile: frmcrsr.cxx,v $
 *
 *  $Revision: 1.37 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:14:52 $
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


#pragma hdrstop

#include "ndtxt.hxx"        // GetNode()
#include "pam.hxx"          // SwPosition
#include "frmtool.hxx"
#include "doc.hxx"
#include "viewopt.hxx"
#include "paratr.hxx"
#include "pagefrm.hxx"
#include "colfrm.hxx"
#include "txttypes.hxx"

#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif

#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif

#ifndef _SVX_TSPTITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif

#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif

// OD 2004-03-18 #114789#
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif

#ifndef _PORMULTI_HXX
#include <pormulti.hxx>     // SwMultiPortion
#endif

#include <unicode/ubidi.h>

#include "frmsh.hxx"
#include "txtcfg.hxx"
#include "txtfrm.hxx"       // SwTxtFrm
#include "inftxt.hxx"       // SwTxtSizeInfo
#include "itrtxt.hxx"       // SwTxtCursor
#include "crstate.hxx"      // SwTxtCursor
#include "viewsh.hxx"       // InvalidateWindows
#include "swfntcch.hxx"     // SwFontAccess
#include "flyfrm.hxx"
// OD 2004-05-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif

#if OSL_DEBUG_LEVEL > 1
#include "txtpaint.hxx"
#endif

#define MIN_OFFSET_STEP 10

/*
 * 1170-SurvivalKit: Wie gelangt man hinter das letzte Zeichen der Zeile.
 * - RightMargin verzichtet auf den Positionsausgleich mit -1
 * - GetCharRect liefert bei MV_RIGHTMARGIN ein GetEndCharRect
 * - GetEndCharRect setzt bRightMargin auf sal_True
 * - SwTxtCursor::bRightMargin wird per CharCrsrToLine auf sal_False gesetzt
 */

/*************************************************************************
 *                      GetAdjFrmAtPos()
 *************************************************************************/

SwTxtFrm *GetAdjFrmAtPos( SwTxtFrm *pFrm, const SwPosition &rPos,
                          const sal_Bool bRightMargin, const sal_Bool bNoScroll = TRUE )
{
    // 8810: vgl. 1170, RightMargin in der letzten Masterzeile...
    const xub_StrLen nOffset = rPos.nContent.GetIndex();
    SwTxtFrm *pFrmAtPos = pFrm;
    if( !bNoScroll || pFrm->GetFollow() )
    {
        pFrmAtPos = pFrm->GetFrmAtPos( rPos );
        if( nOffset < pFrmAtPos->GetOfst() &&
            !pFrmAtPos->IsFollow() )
        {
            xub_StrLen nNew = nOffset;
            if( nNew < MIN_OFFSET_STEP )
                nNew = 0;
            else
                nNew -= MIN_OFFSET_STEP;
            lcl_ChangeOffset( pFrmAtPos, nNew );
        }
    }
    while( pFrm != pFrmAtPos )
    {
        pFrm = pFrmAtPos;
        pFrm->GetFormatted();
        pFrmAtPos = (SwTxtFrm*)pFrm->GetFrmAtPos( rPos );
    }

    if( nOffset && bRightMargin )
    {
        while( pFrmAtPos && pFrmAtPos->GetOfst() == nOffset &&
               pFrmAtPos->IsFollow() )
        {
            pFrmAtPos->GetFormatted();
            pFrmAtPos = pFrmAtPos->FindMaster();
        }
        ASSERT( pFrmAtPos, "+GetCharRect: no frame with my rightmargin" );
    }
    return pFrmAtPos ? pFrmAtPos : pFrm;
}

sal_Bool lcl_ChangeOffset( SwTxtFrm* pFrm, xub_StrLen nNew )
{
    // In Bereichen und ausserhalb von Flies wird nicht mehr gescrollt.
    ASSERT( !pFrm->IsFollow(), "Illegal Scrolling by Follow!" );
    if( pFrm->GetOfst() != nNew && !pFrm->IsInSct() )
    {
        SwFlyFrm *pFly = pFrm->FindFlyFrm();
        // Vorsicht, wenn z.B. bei einem spaltigen Rahmen die Groesse noch invalide ist,
        // duerfen wir nicht mal eben herumscrollen
        if ( ( pFly && pFly->IsValid() &&
             !pFly->GetNextLink() && !pFly->GetPrevLink() ) ||
             ( !pFly && pFrm->IsInTab() ) )
        {
            ViewShell* pVsh = pFrm->GetShell();
            if( pVsh )
            {
                if( pVsh->GetNext() != pVsh ||
                    ( pFrm->GetDrawObjs() && pFrm->GetDrawObjs()->Count() ) )
                {
                    if( !pFrm->GetOfst() )
                        return sal_False;
                    nNew = 0;
                }
                pFrm->SetOfst( nNew );
                pFrm->SetPara( 0 );
                pFrm->GetFormatted();
                if( pFrm->Frm().HasArea() )
                    pFrm->GetShell()->InvalidateWindows( pFrm->Frm() );
                return sal_True;
            }
        }
    }
    return sal_False;
}

/*************************************************************************
 *                      GetFrmAtOfst(), GetFrmAtPos()
 *************************************************************************/

// OD 07.10.2003 #110978#
SwTxtFrm& SwTxtFrm::GetFrmAtOfst( const xub_StrLen nWhere )
{
    SwTxtFrm* pRet = this;
    while( pRet->HasFollow() && nWhere >= pRet->GetFollow()->GetOfst() )
        pRet = pRet->GetFollow();
    return *pRet;
}

SwTxtFrm *SwTxtFrm::GetFrmAtPos( const SwPosition &rPos )
{
    SwTxtFrm *pFoll = (SwTxtFrm*)this;
    while( pFoll->GetFollow() )
    {
        if( rPos.nContent.GetIndex() > pFoll->GetFollow()->GetOfst() )
            pFoll = pFoll->GetFollow();
        else
        {
            if( rPos.nContent.GetIndex() == pFoll->GetFollow()->GetOfst()
                 && !SwTxtCursor::IsRightMargin() )
                 pFoll = pFoll->GetFollow();
            else
                break;
        }
    }
    return pFoll;
}

/*************************************************************************
 *                      SwTxtFrm::GetCharRect()
 *************************************************************************/

/*
 * GetCharRect() findet die Characterzelle des Characters, dass
 * durch aPos beschrieben wird. GetCrsrOfst() findet den
 * umgekehrten Weg: Von einer Dokumentkoordinate zu einem Pam.
 * Beide sind virtuell in der Framebasisklasse und werden deshalb
 * immer angezogen.
 */

sal_Bool SwTxtFrm::GetCharRect( SwRect& rOrig, const SwPosition &rPos,
                            SwCrsrMoveState *pCMS ) const
{
    ASSERT( ! IsVertical() || ! IsSwapped(),"SwTxtFrm::GetCharRect with swapped frame" );

    if( IsLocked() || IsHiddenNow() )
        return sal_False;

    //Erstmal den richtigen Frm finden, dabei muss beachtet werden, dass:
    //- die gecachten Informationen verworfen sein koennen (GetPara() == 0)
    //- das ein Follow gemeint sein kann
    //- das die Kette der Follows dynamisch waechst; der in den wir
    //  schliesslich gelangen muss aber Formatiert sein.

    // opt: reading ahead erspart uns ein GetAdjFrmAtPos
    const sal_Bool bRightMargin = pCMS && ( MV_RIGHTMARGIN == pCMS->eState );
    const sal_Bool bNoScroll = pCMS && pCMS->bNoScroll;
    SwTxtFrm *pFrm = GetAdjFrmAtPos( (SwTxtFrm*)this, rPos, bRightMargin,
                                     bNoScroll );
    pFrm->GetFormatted();
    const SwFrm* pTmpFrm = (SwFrm*)pFrm->GetUpper();

    SWRECTFN ( pFrm )
    const SwTwips nUpperMaxY = (pTmpFrm->*fnRect->fnGetPrtBottom)();
    const SwTwips nFrmMaxY = (pFrm->*fnRect->fnGetPrtBottom)();

    // nMaxY is an absolute value
    SwTwips nMaxY = bVert ?
                    Max( nFrmMaxY, nUpperMaxY ) :
                    Min( nFrmMaxY, nUpperMaxY );

    sal_Bool bRet = sal_False;

    if ( pFrm->IsEmpty() || ! (pFrm->Prt().*fnRect->fnGetHeight)() )
    {
        Point aPnt1 = pFrm->Frm().Pos() + pFrm->Prt().Pos();
        SwTxtNode* pTxtNd = ((SwTxtFrm*)this)->GetTxtNode();
        short nFirstOffset;
        pTxtNd->GetFirstLineOfsWithNum( nFirstOffset );

        Point aPnt2;
        if ( bVert )
        {
            if( nFirstOffset > 0 )
                aPnt1.Y() += nFirstOffset;

            if ( aPnt1.X() < nMaxY )
                aPnt1.X() = nMaxY;
            aPnt2.X() = aPnt1.X() + pFrm->Prt().Width();
            aPnt2.Y() = aPnt1.Y();
            if( aPnt2.X() < nMaxY )
                aPnt2.X() = nMaxY;
        }
        else
        {
            if( nFirstOffset > 0 )
                aPnt1.X() += nFirstOffset;

            if( aPnt1.Y() > nMaxY )
                aPnt1.Y() = nMaxY;
            aPnt2.X() = aPnt1.X();
            aPnt2.Y() = aPnt1.Y() + pFrm->Prt().Height();
            if( aPnt2.Y() > nMaxY )
                aPnt2.Y() = nMaxY;
        }

        rOrig = SwRect( aPnt1, aPnt2 );

        if ( pCMS )
        {
            pCMS->aRealHeight.X() = 0;
            pCMS->aRealHeight.Y() = bVert ? -rOrig.Width() : rOrig.Height();
        }

#ifdef BIDI
        if ( pFrm->IsRightToLeft() )
            pFrm->SwitchLTRtoRTL( rOrig );
#endif

        bRet = sal_True;
    }
    else
    {
        if( !pFrm->HasPara() )
            return sal_False;

        SwFrmSwapper aSwapper( pFrm, sal_True );
        if ( bVert )
            nMaxY = pFrm->SwitchVerticalToHorizontal( nMaxY );

        sal_Bool bGoOn = sal_True;
        xub_StrLen nOffset = rPos.nContent.GetIndex();
        xub_StrLen nNextOfst;

        do
        {
            {
                SwTxtSizeInfo aInf( pFrm );
                SwTxtCursor  aLine( pFrm, &aInf );
                nNextOfst = aLine.GetEnd();
                // Siehe Kommentar in AdjustFrm
                // 1170: das letzte Zeichen der Zeile mitnehmen?
                bRet = bRightMargin ? aLine.GetEndCharRect( &rOrig, nOffset, pCMS, nMaxY )
                                : aLine.GetCharRect( &rOrig, nOffset, pCMS, nMaxY );
            }

#ifdef BIDI
            if ( pFrm->IsRightToLeft() )
                pFrm->SwitchLTRtoRTL( rOrig );
#endif
            if ( bVert )
                pFrm->SwitchHorizontalToVertical( rOrig );

            if( pFrm->IsUndersized() && pCMS && !pFrm->GetNext() &&
                (rOrig.*fnRect->fnGetBottom)() == nUpperMaxY &&
                pFrm->GetOfst() < nOffset &&
                !pFrm->IsFollow() && !bNoScroll &&
                pFrm->GetTxtNode()->GetTxt().Len() != nNextOfst )
                bGoOn = lcl_ChangeOffset( pFrm, nNextOfst );
            else
                bGoOn = sal_False;
        } while ( bGoOn );

        if ( pCMS )
        {
#ifdef BIDI
            if ( pFrm->IsRightToLeft() )
            {
                if( pCMS->b2Lines && pCMS->p2Lines)
                {
                    pFrm->SwitchLTRtoRTL( pCMS->p2Lines->aLine );
                    pFrm->SwitchLTRtoRTL( pCMS->p2Lines->aPortion );
                }
            }
#endif

            if ( bVert )
            {
                if ( pCMS->bRealHeight )
                {
                    pCMS->aRealHeight.Y() = -pCMS->aRealHeight.Y();
                    if ( pCMS->aRealHeight.Y() < 0 )
                    {
                        // writing direction is from top to bottom
                        pCMS->aRealHeight.X() =  ( rOrig.Width() -
                                                    pCMS->aRealHeight.X() +
                                                    pCMS->aRealHeight.Y() );
                    }
                }
                if( pCMS->b2Lines && pCMS->p2Lines)
                {
                    pFrm->SwitchHorizontalToVertical( pCMS->p2Lines->aLine );
                    pFrm->SwitchHorizontalToVertical( pCMS->p2Lines->aPortion );
                }
            }

        }
    }
    if( bRet )
    {
        SwPageFrm *pPage = pFrm->FindPageFrm();
        ASSERT( pPage, "Text esaped from page?" );
        const SwTwips nOrigTop = (rOrig.*fnRect->fnGetTop)();
        const SwTwips nPageTop = (pPage->Frm().*fnRect->fnGetTop)();
        const SwTwips nPageBott = (pPage->Frm().*fnRect->fnGetBottom)();

        // Following situation: if the frame is in an invalid sectionframe,
        // it's possible that the frame is outside the page. If we restrict
        // the cursor position to the page area, we enforce the formatting
        // of the page, of the section frame and the frame himself.
        if( (*fnRect->fnYDiff)( nPageTop, nOrigTop ) > 0 )
            (rOrig.*fnRect->fnSetTop)( nPageTop );

        if ( (*fnRect->fnYDiff)( nOrigTop, nPageBott ) > 0 )
            (rOrig.*fnRect->fnSetTop)( nPageBott );
    }

    return bRet;
}

/*************************************************************************
 *                      SwTxtFrm::GetAutoPos()
 *************************************************************************/

/*
 * GetAutoPos() findet die Characterzelle des Characters, dass
 * durch aPos beschrieben wird und wird von autopositionierten Rahmen genutzt.
 */

sal_Bool SwTxtFrm::GetAutoPos( SwRect& rOrig, const SwPosition &rPos ) const
{
    if( IsHiddenNow() )
        return sal_False;

    xub_StrLen nOffset = rPos.nContent.GetIndex();
    SwTxtFrm* pFrm = &(const_cast<SwTxtFrm*>(this)->GetFrmAtOfst( nOffset ));

    pFrm->GetFormatted();
    const SwFrm* pTmpFrm = (SwFrm*)pFrm->GetUpper();

    SWRECTFN( pTmpFrm )
    SwTwips nUpperMaxY = (pTmpFrm->*fnRect->fnGetPrtBottom)();

    // nMaxY is in absolute value
    SwTwips nMaxY = bVert ?
                    Max( (pFrm->*fnRect->fnGetPrtBottom)(), nUpperMaxY ) :
                    Min( (pFrm->*fnRect->fnGetPrtBottom)(), nUpperMaxY );

    if ( pFrm->IsEmpty() || ! (pFrm->Prt().*fnRect->fnGetHeight)() )
    {
        Point aPnt1 = pFrm->Frm().Pos() + pFrm->Prt().Pos();
        Point aPnt2;
        if ( bVert )
        {
            if ( aPnt1.X() < nMaxY )
                aPnt1.X() = nMaxY;
            aPnt2.X() = aPnt1.X() + pFrm->Prt().Width();
            aPnt2.Y() = aPnt1.Y();
            if( aPnt2.X() < nMaxY )
                aPnt2.X() = nMaxY;
        }
        else
        {
            if( aPnt1.Y() > nMaxY )
                aPnt1.Y() = nMaxY;
            aPnt2.X() = aPnt1.X();
            aPnt2.Y() = aPnt1.Y() + pFrm->Prt().Height();
            if( aPnt2.Y() > nMaxY )
                aPnt2.Y() = nMaxY;
        }
        rOrig = SwRect( aPnt1, aPnt2 );
        return sal_True;
    }
    else
    {
        if( !pFrm->HasPara() )
            return sal_False;

        SwFrmSwapper aSwapper( pFrm, sal_True );
        if ( bVert )
            nMaxY = pFrm->SwitchVerticalToHorizontal( nMaxY );

        SwTxtSizeInfo aInf( pFrm );
        SwTxtCursor aLine( pFrm, &aInf );
        SwCrsrMoveState aTmpState( MV_SETONLYTEXT );
        aTmpState.bRealHeight = TRUE;
        if( aLine.GetCharRect( &rOrig, nOffset, &aTmpState, nMaxY ) )
        {
            if( aTmpState.aRealHeight.X() >= 0 )
            {
                rOrig.Pos().Y() += aTmpState.aRealHeight.X();
                rOrig.Height( aTmpState.aRealHeight.Y() );
            }

            if ( pFrm->IsRightToLeft() )
                pFrm->SwitchLTRtoRTL( rOrig );

            if ( bVert )
                pFrm->SwitchHorizontalToVertical( rOrig );

            return sal_True;
        }
        return sal_False;
    }
}

/** determine top of line for given position in the text frame

    OD 11.11.2003 #i22341#
    OD 2004-03-18 #114789# - corrections:
    - Top of first paragraph line is the top of the printing area of the text frame
    - If a proportional line spacing is applied use top of anchor character as
      top of the line.

    @author OD
*/
bool SwTxtFrm::GetTopOfLine( SwTwips& _onTopOfLine,
                             const SwPosition& _rPos ) const
{
    bool bRet = true;

    // get position offset
    xub_StrLen nOffset = _rPos.nContent.GetIndex();

    if ( GetTxt().Len() < nOffset )
    {
        bRet = false;
    }
    else
    {
        SWRECTFN( this )
        if ( IsEmpty() || !(Prt().*fnRect->fnGetHeight)() )
        {
            // OD 2004-03-18 #i11860# - consider upper space amount considered
            // for previous frame and the page grid.
            _onTopOfLine = (this->*fnRect->fnGetPrtTop)();
        }
        else
        {
            // determine formatted text frame that contains the requested position
            SwTxtFrm* pFrm = &(const_cast<SwTxtFrm*>(this)->GetFrmAtOfst( nOffset ));
            pFrm->GetFormatted();
            SWREFRESHFN( pFrm )
            // OD 2004-03-18 #114789# - If proportional line spacing is applied
            // to the text frame, the top of the anchor character is also the
            // top of the line.
            // Otherwise the line layout determines the top of the line
            const SvxLineSpacingItem& rSpace = GetAttrSet()->GetLineSpacing();
            if ( rSpace.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
            {
                SwRect aCharRect;
                if ( GetAutoPos( aCharRect, _rPos ) )
                {
                    _onTopOfLine = (aCharRect.*fnRect->fnGetTop)();
                }
                else
                {
                    bRet = false;
                }
            }
            else
            {
                // assure that text frame is in a horizontal layout
                SwFrmSwapper aSwapper( pFrm, sal_True );
                // determine text line that contains the requested position
                SwTxtSizeInfo aInf( pFrm );
                SwTxtCursor aLine( pFrm, &aInf );
                aLine.CharCrsrToLine( nOffset );
                // determine top of line
                _onTopOfLine = aLine.Y();
                if ( bVert )
                {
                    _onTopOfLine = pFrm->SwitchHorizontalToVertical( _onTopOfLine );
                }
            }
        }
    }

    return bRet;
}

/*************************************************************************
 *                      SwTxtFrm::_GetCrsrOfst()
 *************************************************************************/

// Minimaler Abstand von nichtleeren Zeilen etwas weniger als 2 cm
#define FILL_MIN_DIST 1100

struct SwFillData
{
    SwRect aFrm;
    const SwCrsrMoveState *pCMS;
    SwPosition* pPos;
    const Point& rPoint;
    SwTwips nLineWidth;
    sal_Bool bFirstLine : 1;
    sal_Bool bInner     : 1;
    sal_Bool bColumn    : 1;
    sal_Bool bEmpty     : 1;
    SwFillData( const SwCrsrMoveState *pC, SwPosition* pP, const SwRect& rR,
        const Point& rPt ) : aFrm( rR ), pCMS( pC ), pPos( pP ), rPoint( rPt ),
        nLineWidth( 0 ), bFirstLine( sal_True ), bInner( sal_False ), bColumn( sal_False ),
        bEmpty( sal_True ){}
    const SwFillMode Mode() const { return pCMS->pFill->eMode; }
    const long X() const { return rPoint.X(); }
    const long Y() const { return rPoint.Y(); }
    const long Left() const { return aFrm.Left(); }
    const long Right() const { return aFrm.Right(); }
    const long Bottom() const { return aFrm.Bottom(); }
    SwRect& Frm() { return aFrm; }
    SwFillCrsrPos &Fill() const { return *pCMS->pFill; }
    void SetTab( MSHORT nNew ) { pCMS->pFill->nTabCnt = nNew; }
    void SetSpace( MSHORT nNew ) { pCMS->pFill->nSpaceCnt = nNew; }
    void SetOrient( const SwHoriOrient eNew ){ pCMS->pFill->eOrient = eNew; }
};

sal_Bool SwTxtFrm::_GetCrsrOfst(SwPosition* pPos, const Point& rPoint,
                    const sal_Bool bChgFrm, SwCrsrMoveState* pCMS ) const
{
    // 8804: _GetCrsrOfst wird vom GetCrsrOfst und GetKeyCrsrOfst gerufen.
    // In keinem Fall nur ein return sal_False.

    if( IsLocked() || IsHiddenNow() )
        return sal_False;

    ((SwTxtFrm*)this)->GetFormatted();

    Point aOldPoint( rPoint );

    if ( IsVertical() )
    {
        SwitchVerticalToHorizontal( (Point&)rPoint );
        ((SwTxtFrm*)this)->SwapWidthAndHeight();
    }

#ifdef BIDI
    if ( IsRightToLeft() )
        SwitchRTLtoLTR( (Point&)rPoint );
#endif

    SwFillData *pFillData = ( pCMS && pCMS->pFill ) ?
                        new SwFillData( pCMS, pPos, Frm(), rPoint ) : NULL;

    if ( IsEmpty() )
    {
        SwTxtNode* pTxtNd = ((SwTxtFrm*)this)->GetTxtNode();
        pPos->nNode = *pTxtNd;
        pPos->nContent.Assign( pTxtNd, 0 );
        if( pCMS && pCMS->bFieldInfo )
        {
            SwTwips nDiff = rPoint.X() - Frm().Left() - Prt().Left();
            if( nDiff > 50 || nDiff < 0 )
                ((SwCrsrMoveState*)pCMS)->bPosCorr = sal_True;
        }
    }
    else
    {
        SwTxtSizeInfo aInf( (SwTxtFrm*)this );
        SwTxtCursor  aLine( ((SwTxtFrm*)this), &aInf );

        // Siehe Kommentar in AdjustFrm()
        SwTwips nMaxY = Frm().Top() + Prt().Top() + Prt().Height();
        aLine.TwipsToLine( rPoint.Y() );
        while( aLine.Y() + aLine.GetLineHeight() > nMaxY )
        {
            DBG_LOOP;
            if( !aLine.Prev() )
                break;
        }

        if( aLine.GetDropLines() >= aLine.GetLineNr() && 1 != aLine.GetLineNr()
            && rPoint.X() < aLine.FirstLeft() + aLine.GetDropLeft() )
            while( aLine.GetLineNr() > 1 )
                aLine.Prev();

        xub_StrLen nOffset = aLine.GetCrsrOfst( pPos, rPoint, bChgFrm, pCMS );

        if( pCMS && pCMS->eState == MV_NONE && aLine.GetEnd() == nOffset )
            ((SwCrsrMoveState*)pCMS)->eState = MV_RIGHTMARGIN;

    // 6776: pPos ist ein reiner IN-Parameter, der nicht ausgewertet werden darf.
    // Das pIter->GetCrsrOfst returnt aus einer Verschachtelung mit STRING_LEN.
    // Wenn SwTxtIter::GetCrsrOfst von sich aus weitere GetCrsrOfst
    // ruft, so aendert sich nNode der Position. In solchen Faellen
    // darf pPos nicht berechnet werden.
        if( STRING_LEN != nOffset )
        {
#ifdef USED
            // 8626: bei Up/Down darf diese Zeile nicht verlassen werden.
            if( pCMS && MV_UPDOWN == pCMS->eState )
            {
                const xub_StrLen nEnd = aLine.GetEnd();
                if( nOffset >= nEnd && nEnd )
                {
                    // Man muss hinter das letzte Zeichen kommen duerfen?!
                    nOffset = nEnd - 1;                 // UnitUp-Korrektur
                }
                else
                    if( nOffset < aLine.GetStart() )
                        nOffset = aLine.GetStart(); // UnitDown-Korrektur
            }
#endif
            SwTxtNode* pTxtNd = ((SwTxtFrm*)this)->GetTxtNode();
            pPos->nNode = *pTxtNd;
            pPos->nContent.Assign( pTxtNd, nOffset );
            if( pFillData )
            {
                if( pTxtNd->GetTxt().Len() > nOffset ||
                    rPoint.Y() < Frm().Top() )
                    pFillData->bInner = sal_True;
                pFillData->bFirstLine = aLine.GetLineNr() < 2;
                if( pTxtNd->GetTxt().Len() )
                {
                    pFillData->bEmpty = sal_False;
                    pFillData->nLineWidth = aLine.GetCurr()->Width();
                }
            }
        }
    }
    sal_Bool bChgFillData = sal_False;
    if( pFillData && FindPageFrm()->Frm().IsInside( aOldPoint ) )
    {
        FillCrsrPos( *pFillData );
        bChgFillData = sal_True;
    }

    if ( IsVertical() )
    {
        if ( bChgFillData )
            SwitchHorizontalToVertical( pFillData->Fill().aCrsr.Pos() );
        ((SwTxtFrm*)this)->SwapWidthAndHeight();
    }

    if ( IsRightToLeft() && bChgFillData )
    {
            SwitchLTRtoRTL( pFillData->Fill().aCrsr.Pos() );
            const SwHoriOrient eOrient = pFillData->pCMS->pFill->eOrient;

            if ( HORI_LEFT == eOrient )
                pFillData->SetOrient( HORI_RIGHT );
            else if ( HORI_RIGHT == eOrient )
                pFillData->SetOrient( HORI_LEFT );
    }

    (Point&)rPoint = aOldPoint;
    delete pFillData;

    return sal_True;
}

/*************************************************************************
 *                 virtual SwTxtFrm::GetCrsrOfst()
 *************************************************************************/

sal_Bool SwTxtFrm::GetCrsrOfst(SwPosition* pPos, Point& rPoint,
                               SwCrsrMoveState* pCMS ) const
{
    MSHORT nChgFrm = 2;
    if( pCMS )
    {
        if( MV_UPDOWN == pCMS->eState )
            nChgFrm = 0;
        else if( MV_SETONLYTEXT == pCMS->eState ||
                 MV_TBLSEL == pCMS->eState )
            nChgFrm = 1;
    }
    return _GetCrsrOfst( pPos, rPoint, nChgFrm != 0, pCMS );
}

/*************************************************************************
 *                      SwTxtFrm::LeftMargin()
 *************************************************************************/

/*
 * Layout-orientierte Cursorbewegungen
 */

/*
 * an den Zeilenanfang
 */

sal_Bool SwTxtFrm::LeftMargin(SwPaM *pPam) const
{
    if( ((const SwNode*)pPam->GetNode()) != GetNode() )
        pPam->GetPoint()->nNode = *((SwTxtFrm*)this)->GetTxtNode();

    SwTxtFrm *pFrm = GetAdjFrmAtPos( (SwTxtFrm*)this, *pPam->GetPoint(),
                                     SwTxtCursor::IsRightMargin() );
    pFrm->GetFormatted();
    xub_StrLen nIndx;
    if ( pFrm->IsEmpty() )
        nIndx = 0;
    else
    {
        SwTxtSizeInfo aInf( pFrm );
        SwTxtCursor  aLine( pFrm, &aInf );

        aLine.CharCrsrToLine(pPam->GetPoint()->nContent.GetIndex());
        nIndx = aLine.GetStart();
        if( pFrm->GetOfst() && !pFrm->IsFollow() && !aLine.GetPrev() )
        {
            lcl_ChangeOffset( pFrm, 0 );
            nIndx = 0;
        }
    }
    pPam->GetPoint()->nContent = SwIndex( pFrm->GetTxtNode(), nIndx );
    SwTxtCursor::SetRightMargin( sal_False );
    return sal_True;
}

/*************************************************************************
 *                      SwTxtFrm::RightMargin()
 *************************************************************************/

/*
 * An das Zeilenende:Das ist die Position vor dem letzten
 * Character in der Zeile. Ausnahme: In der letzten Zeile soll
 * der Cursor auch hinter dem letzten Character stehen koennen,
 * um Text anhaengen zu koennen.
 *
 */

sal_Bool SwTxtFrm::RightMargin(SwPaM *pPam, sal_Bool bAPI) const
{
    if( ((const SwNode*)pPam->GetNode()) != GetNode() )
        pPam->GetPoint()->nNode = *((SwTxtFrm*)this)->GetTxtNode();

    SwTxtFrm *pFrm = GetAdjFrmAtPos( (SwTxtFrm*)this, *pPam->GetPoint(),
                                     SwTxtCursor::IsRightMargin() );
    pFrm->GetFormatted();
    xub_StrLen nRightMargin;
    if ( IsEmpty() )
        nRightMargin = 0;
    else
    {
        SwTxtSizeInfo aInf( pFrm );
        SwTxtCursor  aLine( pFrm, &aInf );

        aLine.CharCrsrToLine(pPam->GetPoint()->nContent.GetIndex());
        nRightMargin = aLine.GetStart() + aLine.GetCurr()->GetLen();

        // Harte Zeilenumbrueche lassen wir hinter uns.
        if( aLine.GetCurr()->GetLen() &&
            CH_BREAK == aInf.GetTxt().GetChar( nRightMargin - 1 ) )
            --nRightMargin;
        if( !bAPI && (aLine.GetNext() || pFrm->GetFollow()) )
        {
            while( nRightMargin > aLine.GetStart() &&
                ' ' == aInf.GetTxt().GetChar( nRightMargin - 1 ) )
                --nRightMargin;
        }
    }
    pPam->GetPoint()->nContent = SwIndex( pFrm->GetTxtNode(), nRightMargin );
    SwTxtCursor::SetRightMargin( !bAPI );
    return sal_True;
}

/*************************************************************************
 *                      SwTxtFrm::_UnitUp()
 *************************************************************************/

//Die beiden folgenden Methoden versuchen zunaechst den Crsr in die
//nachste/folgende Zeile zu setzen. Gibt es im Frame keine vorhergehende/
//folgende Zeile, so wird der Aufruf an die Basisklasse weitergeleitet.
//Die Horizontale Ausrichtung des Crsr wird hinterher von der CrsrShell
//vorgenommen.

class SwSetToRightMargin
{
    sal_Bool bRight;
public:
    inline SwSetToRightMargin() : bRight( sal_False ) { }
    inline ~SwSetToRightMargin() { SwTxtCursor::SetRightMargin( bRight ); }
    inline void SetRight( const sal_Bool bNew ) { bRight = bNew; }
};

sal_Bool SwTxtFrm::_UnitUp( SwPaM *pPam, const SwTwips nOffset,
                            sal_Bool bSetInReadOnly ) const
{
    // 8626: Im Notfall den RightMargin setzen.
    SwSetToRightMargin aSet;

    if( IsInTab() &&
        pPam->GetNode( sal_True )->StartOfSectionNode() !=
        pPam->GetNode( sal_False )->StartOfSectionNode() )
    {
        //Wenn der PaM in unterschiedlichen Boxen sitzt, so handelt es sich um
        //eine Tabellenselektion; diese wird von der Basisklasse abgearbeitet.
        return SwCntntFrm::UnitUp( pPam, nOffset, bSetInReadOnly );
    }

    ((SwTxtFrm*)this)->GetFormatted();
    const xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();
    SwRect aCharBox;

    if( !IsEmpty() && !IsHiddenNow() )
    {
        xub_StrLen nFormat = STRING_LEN;
        do
        {
            if( nFormat != STRING_LEN && !IsFollow() )
                lcl_ChangeOffset( ((SwTxtFrm*)this), nFormat );

            SwTxtSizeInfo aInf( (SwTxtFrm*)this );
            SwTxtCursor  aLine( ((SwTxtFrm*)this), &aInf );

            // 8116: Flys ohne Umlauf und IsDummy(); hier wegoptimiert
            if( nPos )
                aLine.CharCrsrToLine( nPos );
            else
                aLine.Top();

            const SwLineLayout *pPrevLine = aLine.GetPrevLine();
            const xub_StrLen nStart = aLine.GetStart();
            SwRect aCharBox;
            aLine.GetCharRect( &aCharBox, nPos );

            sal_Bool bSecondOfDouble = ( aInf.IsMulti() && ! aInf.IsFirstMulti() );
            sal_Bool bPrevLine = ( pPrevLine && pPrevLine != aLine.GetCurr() );

            if( !pPrevLine && !bSecondOfDouble && GetOfst() && !IsFollow() )
            {
                nFormat = GetOfst();
                xub_StrLen nDiff = aLine.GetLength();
                if( !nDiff )
                    nDiff = MIN_OFFSET_STEP;
                if( nFormat > nDiff )
                    nFormat -= nDiff;
                else
                    nFormat = 0;
                continue;
            }

            // we select the target line for the cursor, in case we are in a
            // double line portion, prev line = curr line
            if( bPrevLine && !bSecondOfDouble )
            {
                aLine.PrevLine();
                while ( aLine.GetStart() == nStart &&
                        0 != ( pPrevLine = aLine.GetPrevLine() ) &&
                        pPrevLine != aLine.GetCurr() )
                    aLine.PrevLine();
            }

            if ( bPrevLine || bSecondOfDouble )
            {
                aCharBox.SSize().Width() /= 2;
                aCharBox.Pos().X() = aCharBox.Pos().X() - 150;

                // siehe Kommentar in SwTxtFrm::GetCrsrOfst()
#ifndef PRODUCT
                const ULONG nOldNode = pPam->GetPoint()->nNode.GetIndex();
#endif
                // Der Node soll nicht gewechselt werden
                xub_StrLen nOfst = aLine.GetCrsrOfst( pPam->GetPoint(),
                                                    aCharBox.Pos(), sal_False );
                ASSERT( nOldNode == pPam->GetPoint()->nNode.GetIndex(),
                        "SwTxtFrm::UnitUp: illegal node change" )

                // 7684: Wir stellen sicher, dass wir uns nach oben bewegen.
                if( nOfst >= nStart && nStart && !bSecondOfDouble )
                {
                    // nOfst = nStart - 1;
                    nOfst = nStart;
                    aSet.SetRight( sal_True );
                }
                pPam->GetPoint()->nContent =
                      SwIndex( ((SwTxtFrm*)this)->GetTxtNode(), nOfst );
                return sal_True;
            }

            if ( IsFollow() )
            {
                aLine.GetCharRect( &aCharBox, nPos );
                aCharBox.SSize().Width() /= 2;
            }
            break;
        } while ( sal_True );
    }
    /* Wenn this ein Follow ist und ein Prev miszlang, so
     * muessen wir in die letzte Zeile des Master ... und der sind wir.
     * Oder wir sind ein Follow mit Follow, dann muessen wir uns den
     * Master extra besorgen...
     */
    if ( IsFollow() )
    {
        const SwTxtFrm *pPrev = FindMaster();
        xub_StrLen nOffs = GetOfst();
        if( pPrev )
        {
            ViewShell *pSh = GetShell();
            sal_Bool bProtectedAllowed = pSh && pSh->GetViewOptions()->IsCursorInProtectedArea();
            const SwTxtFrm *pPrevPrev = pPrev;
            // Hier werden geschuetzte Frames und Frame ohne Inhalt ausgelassen
            while( pPrevPrev && ( pPrevPrev->GetOfst() == nOffs ||
                   ( !bProtectedAllowed && pPrevPrev->IsProtected() ) ) )
            {
                pPrev = pPrevPrev;
                nOffs = pPrev->GetOfst();
                if ( pPrevPrev->IsFollow() )
                    pPrevPrev = pPrev->FindMaster();
                else
                    pPrevPrev = NULL;
            }
            if ( !pPrevPrev )
                return pPrev->SwCntntFrm::UnitUp( pPam, nOffset, bSetInReadOnly );
            aCharBox.Pos().Y() = pPrevPrev->Frm().Bottom() - 1;
            return pPrevPrev->GetKeyCrsrOfst( pPam->GetPoint(), aCharBox.Pos() );
        }
    }
    return SwCntntFrm::UnitUp( pPam, nOffset, bSetInReadOnly );
}

//
// Used for Bidi. nPos is the logical position in the string, bLeft indicates
// if left arrow or right arrow was pressed. The return values are:
// nPos: the new visual position
// bLeft: whether the break iterator has to add or subtract from the
//          current position
void lcl_VisualMoveRecursion( const SwLineLayout& rCurrLine, xub_StrLen nIdx,
                              xub_StrLen& nPos, sal_Bool& bRight,
                              BYTE& nCrsrLevel, BYTE nDefaultDir )
{
    const SwLinePortion* pPor = rCurrLine.GetFirstPortion();
    const SwLinePortion* pLast = 0;

    // what's the current portion
    while ( pPor && nIdx + pPor->GetLen() <= nPos )
    {
        nIdx += pPor->GetLen();
        pLast = pPor;
        pPor = pPor->GetPortion();
    }

    if ( bRight )
    {
        sal_Bool bRecurse = pPor && pPor->IsMultiPortion() &&
                           ((SwMultiPortion*)pPor)->IsBidi();

        // 1. special case: at beginning of bidi portion
        if ( bRecurse && nIdx == nPos )
        {
            nPos += pPor->GetLen();

            // leave bidi portion
            if ( nCrsrLevel != nDefaultDir )
            {
                bRecurse = sal_False;
            }
            else
                // special case:
                // buffer: abcXYZ123 in LTR paragraph
                // view:   abc123ZYX
                // cursor is between c and X in the buffer and cursor level = 0
                nCrsrLevel++;
        }

        // 2. special case: at beginning of portion after bidi portion
        else if ( pLast && pLast->IsMultiPortion() &&
                 ((SwMultiPortion*)pLast)->IsBidi() && nIdx == nPos )
        {
            // enter bidi portion
            if ( nCrsrLevel != nDefaultDir )
            {
                bRecurse = sal_True;
                nIdx -= pLast->GetLen();
                pPor = pLast;
            }
        }

        // Recursion
        if ( bRecurse )
        {
            const SwLineLayout& rLine = ((SwMultiPortion*)pPor)->GetRoot();
            xub_StrLen nTmpPos = nPos - nIdx;
            sal_Bool bTmpForward = ! bRight;
            BYTE nTmpCrsrLevel = nCrsrLevel;
            lcl_VisualMoveRecursion( rLine, 0, nTmpPos, bTmpForward,
                                     nTmpCrsrLevel, nDefaultDir + 1 );

            nPos = nTmpPos + nIdx;
            bRight = bTmpForward;
            nCrsrLevel = nTmpCrsrLevel;
        }

        // go forward
        else
        {
            bRight = sal_True;
            nCrsrLevel = nDefaultDir;
        }

    }
    else
    {
        sal_Bool bRecurse = pPor && pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->IsBidi();

        // 1. special case: at beginning of bidi portion
        if ( bRecurse && nIdx == nPos )
        {
            // leave bidi portion
            if ( nCrsrLevel == nDefaultDir )
            {
                bRecurse = sal_False;
            }
        }

        // 2. special case: at beginning of portion after bidi portion
        else if ( pLast && pLast->IsMultiPortion() &&
                 ((SwMultiPortion*)pLast)->IsBidi() && nIdx == nPos )
        {
            nPos -= pLast->GetLen();

            // enter bidi portion
            if ( nCrsrLevel % 2 == nDefaultDir % 2 )
            {
                bRecurse = sal_True;
                nIdx -= pLast->GetLen();
                pPor = pLast;

                // special case:
                // buffer: abcXYZ123 in LTR paragraph
                // view:   abc123ZYX
                // cursor is behind 3 in the buffer and cursor level = 2
                if ( nDefaultDir + 2 == nCrsrLevel )
                    nPos += pLast->GetLen();
            }
        }

        // go forward
        if ( bRecurse )
        {
            const SwLineLayout& rLine = ((SwMultiPortion*)pPor)->GetRoot();
            xub_StrLen nTmpPos = nPos - nIdx;
            sal_Bool bTmpForward = ! bRight;
            BYTE nTmpCrsrLevel = nCrsrLevel;
            lcl_VisualMoveRecursion( rLine, 0, nTmpPos, bTmpForward,
                                     nTmpCrsrLevel, nDefaultDir + 1 );

            // special case:
            // buffer: abcXYZ123 in LTR paragraph
            // view:   abc123ZYX
            // cursor is between Z and 1 in the buffer and cursor level = 2
            if ( nTmpPos == pPor->GetLen() && nTmpCrsrLevel == nDefaultDir + 1 )
            {
                nTmpPos -= pPor->GetLen();
                nTmpCrsrLevel = nDefaultDir;
                bTmpForward = ! bTmpForward;
            }

            nPos = nTmpPos + nIdx;
            bRight = bTmpForward;
            nCrsrLevel = nTmpCrsrLevel;
        }

        // go backward
        else
        {
            bRight = sal_False;
            nCrsrLevel = nDefaultDir;
        }
    }
}

void SwTxtFrm::PrepareVisualMove( xub_StrLen& nPos, BYTE& nCrsrLevel,
                                  sal_Bool& bForward, sal_Bool bInsertCrsr )
{
    if( IsEmpty() || IsHiddenNow() )
        return;

    ((SwTxtFrm*)this)->GetFormatted();

    SwTxtSizeInfo aInf( (SwTxtFrm*)this );
    SwTxtCursor  aLine( ((SwTxtFrm*)this), &aInf );

    if( nPos )
        aLine.CharCrsrToLine( nPos );
    else
        aLine.Top();

    const SwLineLayout* pLine = aLine.GetCurr();
    const xub_StrLen nStt = aLine.GetStart();
    const xub_StrLen nLen = pLine->GetLen();

    // We have to distinguish between an insert and overwrite cursor:
    // The insert cursor position depends on the cursor level:
    // buffer:  abcXYZdef in LTR paragraph
    // display: abcZYXdef
    // If cursor is between c and X in the buffer and cursor level is 0,
    // the cursor blinks between c and Z and -> sets the cursor between Z and Y.
    // If the cursor level is 1, the cursor blinks between X and d and
    // -> sets the cursor between d and e.
    // The overwrite cursor simply travels to the next visual character.
    if ( bInsertCrsr )
    {
        lcl_VisualMoveRecursion( *pLine, nStt, nPos, bForward,
                                 nCrsrLevel, IsRightToLeft() ? 1 : 0 );
        return;
    }

    const BYTE nDefaultDir = IsRightToLeft() ? UBIDI_RTL : UBIDI_LTR;
    const sal_Bool bVisualRight = ( nDefaultDir == UBIDI_LTR && bForward ) ||
                                  ( nDefaultDir == UBIDI_RTL && ! bForward );

    //
    // Bidi functions from icu 2.0
    //
    const sal_Unicode* pLineString = GetTxtNode()->GetTxt().GetBuffer();
    pLine += nStt;

    UErrorCode nError = U_ZERO_ERROR;
    UBiDi* pBidi = ubidi_openSized( nLen, 0, &nError );
    ubidi_setPara( pBidi, pLineString, nLen, nDefaultDir, NULL, &nError );

    xub_StrLen nTmpPos;
    sal_Bool bOutOfBounds = sal_False;

    if ( nPos < nStt + nLen )
    {
        nTmpPos = (xub_StrLen)ubidi_getVisualIndex( pBidi, nPos, &nError );

        // visual indices are always LTR aligned
        if ( bVisualRight )
        {
            if ( nTmpPos + 1 < nStt + nLen )
                ++nTmpPos;
            else
            {
                nPos = nDefaultDir == UBIDI_RTL ? 0 : nStt + nLen;
                bOutOfBounds = sal_True;
            }
        }
        else
        {
            if ( nTmpPos )
                --nTmpPos;
            else
            {
                nPos = nDefaultDir == UBIDI_RTL ? nStt + nLen : 0;
                bOutOfBounds = sal_True;
            }
        }
    }
    else
    {
        nTmpPos = nDefaultDir == UBIDI_LTR ? nPos - 1 : 0;
    }

    if ( ! bOutOfBounds )
    {
        nPos = (xub_StrLen)ubidi_getLogicalIndex( pBidi, nTmpPos, &nError );

        if ( bForward )
        {
            if ( nPos )
                --nPos;
            else
            {
                ++nPos;
                bForward = ! bForward;
            }
        }
        else
            ++nPos;
    }

    ubidi_close( pBidi );
}

/*************************************************************************
 *                      SwTxtFrm::_UnitDown()
 *************************************************************************/

sal_Bool SwTxtFrm::_UnitDown(SwPaM *pPam, const SwTwips nOffset,
                            sal_Bool bSetInReadOnly ) const
{

    if ( IsInTab() &&
        pPam->GetNode( sal_True )->StartOfSectionNode() !=
        pPam->GetNode( sal_False )->StartOfSectionNode() )
    {
        //Wenn der PaM in unterschiedlichen Boxen sitzt, so handelt es sich um
        //eine Tabellenselektion; diese wird von der Basisklasse abgearbeitet.
        return SwCntntFrm::UnitDown( pPam, nOffset, bSetInReadOnly );
    }
    ((SwTxtFrm*)this)->GetFormatted();
    const xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();
    SwRect aCharBox;
    const SwCntntFrm *pFollow;

    if ( IsVertical() )
        ((SwTxtFrm*)this)->SwapWidthAndHeight();

    if ( !IsEmpty() && !IsHiddenNow() )
    {
        xub_StrLen nFormat = STRING_LEN;
        do
        {
            if( nFormat != STRING_LEN && !IsFollow() &&
                !lcl_ChangeOffset( ((SwTxtFrm*)this), nFormat ) )
                break;

            SwTxtSizeInfo aInf( (SwTxtFrm*)this );
            SwTxtCursor  aLine( ((SwTxtFrm*)this), &aInf );
            nFormat = aLine.GetEnd();

            aLine.CharCrsrToLine( nPos );

            const SwLineLayout* pNextLine = aLine.GetNextLine();
            const xub_StrLen nStart = aLine.GetStart();
            SwRect aCharBox;
            aLine.GetCharRect( &aCharBox, nPos );

            sal_Bool bFirstOfDouble = ( aInf.IsMulti() && aInf.IsFirstMulti() );

            if( pNextLine || bFirstOfDouble )
            {
                aCharBox.SSize().Width() /= 2;
#ifndef PRODUCT
                // siehe Kommentar in SwTxtFrm::GetCrsrOfst()
                const ULONG nOldNode = pPam->GetPoint()->nNode.GetIndex();
#endif
                if ( pNextLine && ! bFirstOfDouble )
                    aLine.NextLine();

                xub_StrLen nOfst = aLine.GetCrsrOfst( pPam->GetPoint(),
                                 aCharBox.Pos(), sal_False );
                ASSERT( nOldNode == pPam->GetPoint()->nNode.GetIndex(),
                    "SwTxtFrm::UnitDown: illegal node change" )

                // 7684: Wir stellen sicher, dass wir uns nach unten bewegen.
                if( nOfst <= nStart && ! bFirstOfDouble )
                    nOfst = nStart + 1;
                pPam->GetPoint()->nContent =
                      SwIndex( ((SwTxtFrm*)this)->GetTxtNode(), nOfst );

                if ( IsVertical() )
                    ((SwTxtFrm*)this)->SwapWidthAndHeight();

                return sal_True;
            }
            if( 0 != ( pFollow = GetFollow() ) )
            {   // geschuetzte Follows auslassen
                const SwCntntFrm* pTmp = pFollow;
                ViewShell *pSh = GetShell();
                if( !pSh || !pSh->GetViewOptions()->IsCursorInProtectedArea() )
                {
                    while( pFollow && pFollow->IsProtected() )
                    {
                        pTmp = pFollow;
                        pFollow = pFollow->GetFollow();
                    }
                }
                if( !pFollow ) // nur noch geschuetzte
                {
                    if ( IsVertical() )
                        ((SwTxtFrm*)this)->SwapWidthAndHeight();
                    return pTmp->SwCntntFrm::UnitDown( pPam, nOffset, bSetInReadOnly );
                }

                aLine.GetCharRect( &aCharBox, nPos );
                aCharBox.SSize().Width() /= 2;
            }
            else if( !IsFollow() )
            {
                xub_StrLen nTmpLen = aInf.GetTxt().Len();
                if( aLine.GetEnd() < nTmpLen )
                {
                    if( nFormat <= GetOfst() )
                    {
                        nFormat = Min( xub_StrLen( GetOfst() + MIN_OFFSET_STEP ),
                                       nTmpLen );
                        if( nFormat <= GetOfst() )
                            break;
                    }
                    continue;
                }
            }
            break;
        } while( sal_True );
    }
    else
        pFollow = GetFollow();

    if ( IsVertical() )
        ((SwTxtFrm*)this)->SwapWidthAndHeight();

    // Bei Follows schlagen wir eine Abkuerzung
    if( pFollow )
    {
        aCharBox.Pos().Y() = pFollow->Frm().Top() + 1;
        return ((SwTxtFrm*)pFollow)->GetKeyCrsrOfst( pPam->GetPoint(),
                                                     aCharBox.Pos() );
    }
    return SwCntntFrm::UnitDown( pPam, nOffset, bSetInReadOnly );
}

/*************************************************************************
 *                   virtual SwTxtFrm::UnitUp()
 *************************************************************************/

sal_Bool SwTxtFrm::UnitUp(SwPaM *pPam, const SwTwips nOffset,
                            sal_Bool bSetInReadOnly ) const
{
    /* Im CrsrSh::Up() wird CntntNode::GetFrm() gerufen.
     * Dies liefert _immer_ den Master zurueck.
     * Um das Cursortravelling nicht zu belasten, korrigieren wir
     * hier im SwTxtFrm.
     * Wir ermittelt UnitUp fuer pFrm, pFrm ist entweder ein Master (=this)
     * oder ein Follow (!=this)
     */
    const SwTxtFrm *pFrm = GetAdjFrmAtPos( (SwTxtFrm*)this, *(pPam->GetPoint()),
                                           SwTxtCursor::IsRightMargin() );
    const sal_Bool bRet = pFrm->_UnitUp( pPam, nOffset, bSetInReadOnly );

    // 8626: kein SwTxtCursor::SetRightMargin( sal_False );
    // statt dessen steht ein SwSetToRightMargin im _UnitUp
    return bRet;
}

/*************************************************************************
 *                   virtual SwTxtFrm::UnitDown()
 *************************************************************************/

sal_Bool SwTxtFrm::UnitDown(SwPaM *pPam, const SwTwips nOffset,
                            sal_Bool bSetInReadOnly ) const
{
    const SwTxtFrm *pFrm = GetAdjFrmAtPos((SwTxtFrm*)this, *(pPam->GetPoint()),
                                           SwTxtCursor::IsRightMargin() );
    const sal_Bool bRet = pFrm->_UnitDown( pPam, nOffset, bSetInReadOnly );
    SwTxtCursor::SetRightMargin( sal_False );
    return bRet;
}

void SwTxtFrm::FillCrsrPos( SwFillData& rFill ) const
{
    if( !rFill.bColumn && GetUpper()->IsColBodyFrm() ) // ColumnFrms jetzt mit BodyFrm
    {
        const SwColumnFrm* pTmp =
            (SwColumnFrm*)GetUpper()->GetUpper()->GetUpper()->Lower(); // die 1. Spalte
        // der erste SwFrm im BodyFrm der ersten Spalte
        const SwFrm* pFrm = ((SwLayoutFrm*)pTmp->Lower())->Lower();
        MSHORT nNextCol = 0;
        // In welcher Spalte landen wir?
        while( rFill.X() > pTmp->Frm().Right() && pTmp->GetNext() )
        {
            pTmp = (SwColumnFrm*)pTmp->GetNext();
            if( ((SwLayoutFrm*)pTmp->Lower())->Lower() ) // ColumnFrms jetzt mit BodyFrm
            {
                pFrm = ((SwLayoutFrm*)pTmp->Lower())->Lower();
                nNextCol = 0;
            }
            else
                ++nNextCol; // leere Spalten erfordern Spaltenumbrueche
        }
        if( pTmp != GetUpper()->GetUpper() ) // Sind wir in einer anderen Spalte gelandet?
        {
            if( !pFrm )
                return;
            if( nNextCol )
            {
                while( pFrm->GetNext() )
                    pFrm = pFrm->GetNext();
            }
            else
            {
                while( pFrm->GetNext() && pFrm->Frm().Bottom() < rFill.Y() )
                    pFrm = pFrm->GetNext();
            }
            // Kein Fuellen, wenn als letzter Frame in der anvisierten
            // Spalte kein Absatz, sondern z.B. eine Tabelle steht
            if( pFrm->IsTxtFrm() )
            {
                rFill.Fill().nColumnCnt = nNextCol;
                rFill.bColumn = sal_True;
                if( rFill.pPos )
                {
                    SwTxtNode* pTxtNd = ((SwTxtFrm*)pFrm)->GetTxtNode();
                    rFill.pPos->nNode = *pTxtNd;
                    rFill.pPos->nContent.Assign( pTxtNd, pTxtNd->GetTxt().Len() );
                }
                if( nNextCol )
                {
                    rFill.aFrm = pTmp->Prt();
                    rFill.aFrm += pTmp->Frm().Pos();
                }
                else
                    rFill.aFrm = pFrm->Frm();
                ((SwTxtFrm*)pFrm)->FillCrsrPos( rFill );
            }
            return;
        }
    }
    sal_Bool bFill = sal_True;
    SwFont *pFnt;
    SwTxtFmtColl* pColl = GetTxtNode()->GetTxtColl();
    MSHORT nFirst = GetTxtNode()->GetSwAttrSet().GetULSpace().GetLower();
    SwTwips nDiff = rFill.Y() - Frm().Bottom();
    if( nDiff < nFirst )
        nDiff = -1;
    else
        pColl = &pColl->GetNextTxtFmtColl();
    SwAttrSet aSet( ((SwDoc*)GetTxtNode()->GetDoc())->GetAttrPool(), aTxtFmtCollSetRange );
    const SwAttrSet* pSet = &pColl->GetAttrSet();
    ViewShell *pSh = GetShell();
    if( GetTxtNode()->HasSwAttrSet() )
    {
        aSet.Put( *GetTxtNode()->GetpSwAttrSet() );
        aSet.SetParent( pSet );
        pSet = &aSet;
        pFnt = new SwFont( pSet, GetNode()->GetDoc() );
    }
    else
    {
        SwFontAccess aFontAccess( pColl, pSh );
        pFnt = new SwFont( *aFontAccess.Get()->GetFont() );
        pFnt->ChkMagic( pSh, pFnt->GetActual() );
    }
    OutputDevice* pOut = pSh->GetOut();
    if ( !GetTxtNode()->GetDoc()->IsBrowseMode() ||
            ( pSh->GetViewOptions()->IsPrtFormat() ) )
        pOut = &GetTxtNode()->GetDoc()->GetRefDev();

    pFnt->SetFntChg( sal_True );
    pFnt->ChgPhysFnt( pSh, *pOut );

    SwTwips nLineHeight = pFnt->GetHeight( pSh, *pOut );

    if( nLineHeight )
    {
        const SvxULSpaceItem &rUL = pSet->GetULSpace();
        SwTwips nDist = Max( rUL.GetLower(), rUL.GetUpper() );
        if( rFill.Fill().nColumnCnt )
        {
            rFill.aFrm.Height( nLineHeight );
            nDiff = rFill.Y() - rFill.Bottom();
            nFirst = 0;
        }
        else if( nDist < nFirst )
            nFirst -= (USHORT)nDist;
        else
            nFirst = 0;
        nDist = Max( nDist, long( GetLineSpace() ) );
        nDist += nLineHeight;
        nDiff -= nFirst;

        if( nDiff > 0 )
        {
            nDiff /= nDist;
            rFill.Fill().nParaCnt = nDiff + 1;
            rFill.nLineWidth = 0;
            rFill.bInner = sal_False;
            rFill.bEmpty = sal_True;
            rFill.SetOrient( HORI_LEFT );
        }
        else
            nDiff = -1;
        if( rFill.bInner )
            bFill = sal_False;
        else
        {
            const SvxTabStopItem &rRuler = pSet->GetTabStops();
            const SvxLRSpaceItem &rLRSpace = pSet->GetLRSpace();

            SwRect &rRect = rFill.Fill().aCrsr;
            rRect.Top( rFill.Bottom() + (nDiff+1) * nDist - nLineHeight );
            if( nFirst && nDiff > -1 )
                rRect.Top( rRect.Top() + nFirst );
            rRect.Height( nLineHeight );
            SwTwips nLeft = rFill.Left() + rLRSpace.GetLeft() +
                            GetTxtNode()->GetLeftMarginWithNum( sal_False );
            SwTwips nRight = rFill.Right() - rLRSpace.GetRight();
            SwTwips nCenter = ( nLeft + nRight ) / 2;
            rRect.Left( nLeft );
            if( FILL_MARGIN == rFill.Mode() )
            {
                if( rFill.bEmpty )
                {
                    rFill.SetOrient( HORI_LEFT );
                    if( rFill.X() < nCenter )
                    {
                        if( rFill.X() > ( nLeft + 2 * nCenter ) / 3 )
                        {
                            rFill.SetOrient( HORI_CENTER );
                            rRect.Left( nCenter );
                        }
                    }
                    else if( rFill.X() > ( nRight + 2 * nCenter ) / 3 )
                    {
                        rFill.SetOrient( HORI_RIGHT );
                        rRect.Left( nRight );
                    }
                    else
                    {
                        rFill.SetOrient( HORI_CENTER );
                        rRect.Left( nCenter );
                    }
                }
                else
                    bFill = sal_False;
            }
            else
            {
                SwTwips nSpace;
                if( FILL_TAB != rFill.Mode() )
                {
static sal_Char __READONLY_DATA sDoubleSpace[] = "  ";
                    const XubString aTmp( sDoubleSpace, RTL_TEXTENCODING_MS_1252 );

                    SwDrawTextInfo aDrawInf( pSh, *pOut, 0, aTmp, 0, 2 );
                    nSpace = pFnt->_GetTxtSize( aDrawInf ).Width()/2;
                }
                if( rFill.X() >= nRight )
                {
                    if( FILL_INDENT != rFill.Mode() && ( rFill.bEmpty ||
                        rFill.X() > rFill.nLineWidth + FILL_MIN_DIST ) )
                    {
                        rFill.SetOrient( HORI_RIGHT );
                        rRect.Left( nRight );
                    }
                    else
                        bFill = sal_False;
                }
                else if( FILL_INDENT == rFill.Mode() )
                {
                    SwTwips nIndent = rFill.X();
                    if( !rFill.bEmpty || nIndent > nRight )
                        bFill = sal_False;
                    else
                    {
                        nIndent -= rFill.Left();
                        if( nIndent >= 0 && nSpace )
                        {
                            nIndent /= nSpace;
                            nIndent *= nSpace;
                            rFill.SetTab( MSHORT( nIndent ) );
                            rRect.Left( nIndent + rFill.Left() );
                        }
                        else
                            bFill = sal_False;
                    }
                }
                else if( rFill.X() > nLeft )
                {
                    SwTwips nTxtLeft = rFill.Left() + rLRSpace.GetTxtLeft() +
                                    GetTxtNode()->GetLeftMarginWithNum( sal_True );
                    rFill.nLineWidth += rFill.bFirstLine ? nLeft : nTxtLeft;
                    SwTwips nLeftTab = nLeft;
                    SwTwips nRightTab = nLeft;
                    MSHORT nSpaceCnt = 0;
                    MSHORT nTabCnt = 0;
                    MSHORT nIdx = 0;
                    do
                    {
                        nLeftTab = nRightTab;
                        if( nIdx < rRuler.Count() )
                        {
                            const SvxTabStop &rTabStop = rRuler.operator[](nIdx);
                            nRightTab = nTxtLeft + rTabStop.GetTabPos();
                            if( nLeftTab < nTxtLeft && nRightTab > nTxtLeft )
                                nRightTab = nTxtLeft;
                            else
                                ++nIdx;
                            if( nRightTab > rFill.nLineWidth )
                                ++nTabCnt;
                        }
                        else
                        {
                            const SvxTabStopItem& rTab =
                                (const SvxTabStopItem &)pSet->
                                GetPool()->GetDefaultItem( RES_PARATR_TABSTOP );
                            MSHORT nDefTabDist = (MSHORT)rTab.GetStart()->GetTabPos();
                            nRightTab = nLeftTab - nTxtLeft;
                            nRightTab /= nDefTabDist;
                            nRightTab = nRightTab * nDefTabDist + nTxtLeft;
                            while ( nRightTab <= nLeftTab )
                                nRightTab += nDefTabDist;
                            if( nRightTab > rFill.nLineWidth )
                                ++nTabCnt;
                            while ( nRightTab < rFill.X() )
                            {
                                nRightTab += nDefTabDist;
                                if( nRightTab > rFill.nLineWidth )
                                    ++nTabCnt;
                            }
                            if( nLeftTab < nRightTab - nDefTabDist )
                                nLeftTab = nRightTab - nDefTabDist;
                        }
                        if( nRightTab > nRight )
                            nRightTab = nRight;
                    }
                    while( rFill.X() > nRightTab );
                    --nTabCnt;
                    if( FILL_TAB != rFill.Mode() )
                    {
                        if( nSpace > 0 )
                        {
                            if( !nTabCnt )
                                nLeftTab = rFill.nLineWidth;
                            while( nLeftTab < rFill.X() )
                            {
                                nLeftTab += nSpace;
                                ++nSpaceCnt;
                            }
                            if( nSpaceCnt )
                            {
                                nLeftTab -= nSpace;
                                --nSpaceCnt;
                            }
                            if( rFill.X() - nLeftTab > nRightTab - rFill.X() )
                            {
                                nSpaceCnt = 0;
                                ++nTabCnt;
                                rRect.Left( nRightTab );
                            }
                            else
                            {
                                if( rFill.X() - nLeftTab > nSpace/2 )
                                {
                                    ++nSpaceCnt;
                                    rRect.Left( nLeftTab + nSpace );
                                }
                                else
                                    rRect.Left( nLeftTab );
                            }
                        }
                        else if( rFill.X() - nLeftTab < nRightTab - rFill.X() )
                            rRect.Left( nLeftTab );
                        else
                        {
                            if( nRightTab >= nRight )
                            {
                                rFill.SetOrient( HORI_RIGHT );
                                rRect.Left( nRight );
                                nTabCnt = 0;
                                nSpaceCnt = 0;
                            }
                            else
                            {
                                rRect.Left( nRightTab );
                                ++nTabCnt;
                            }
                        }
                    }
                    else
                    {
                        if( rFill.X() - nLeftTab < nRightTab - rFill.X() )
                            rRect.Left( nLeftTab );
                        else
                        {
                            if( nRightTab >= nRight )
                            {
                                rFill.SetOrient( HORI_RIGHT );
                                rRect.Left( nRight );
                                nTabCnt = 0;
                                nSpaceCnt = 0;
                            }
                            else
                            {
                                rRect.Left( nRightTab );
                                ++nTabCnt;
                            }
                        }
                    }
                    rFill.SetTab( nTabCnt );
                    rFill.SetSpace( nSpaceCnt );
                    if( bFill )
                    {
                        if( Abs( rFill.X() - nCenter ) <=
                            Abs( rFill.X() - rRect.Left() ) )
                        {
                            rFill.SetOrient( HORI_CENTER );
                            rFill.SetTab( 0 );
                            rFill.SetSpace( 0 );
                            rRect.Left( nCenter );
                        }
                        if( !rFill.bEmpty )
                            rFill.nLineWidth += FILL_MIN_DIST;
                        if( rRect.Left() < rFill.nLineWidth )
                            bFill = sal_False;
                    }
                }
            }
            // Gehen wir ueber die Unterkante der Seite/Spalte etc. hinaus?
            const SwFrm* pUp = GetUpper();
            if( pUp->IsInSct() )
            {
                if( pUp->IsSctFrm() )
                    pUp = pUp->GetUpper();
                else if( pUp->IsColBodyFrm() &&
                         pUp->GetUpper()->GetUpper()->IsSctFrm() )
                    pUp = pUp->GetUpper()->GetUpper()->GetUpper();
            }
            SWRECTFN( this )
            SwTwips nLimit = (pUp->*fnRect->fnGetPrtBottom)();
            SwTwips nRectBottom = rRect.Bottom();
            if ( bVert )
                nRectBottom = SwitchHorizontalToVertical( nRectBottom );

            if( (*fnRect->fnYDiff)( nLimit, nRectBottom ) < 0 )
                bFill = sal_False;
            else
                rRect.Width( 1 );
        }
    }
    else
        bFill = sal_False;
    ((SwCrsrMoveState*)rFill.pCMS)->bFillRet = bFill;
    delete pFnt;
}
