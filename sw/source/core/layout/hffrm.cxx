/*************************************************************************
 *
 *  $RCSfile: hffrm.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:09:57 $
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

#include "pagefrm.hxx"
#include "viewsh.hxx"
#include "doc.hxx"
#include "errhdl.hxx"

#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#include "hffrm.hxx"
#include "txtfrm.hxx"
#include "sectfrm.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "dflyobj.hxx"
#include "frmfmt.hxx"
#include "frmsh.hxx"
#include "ndindex.hxx"
#include "hfspacingitem.hxx"
// OD 2004-05-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif

extern FASTBOOL bObjsDirect;    //frmtool.cxx

static SwTwips lcl_GetFrmMinHeight(const SwLayoutFrm & rFrm)
{
    const SwFmtFrmSize &rSz = rFrm.GetFmt()->GetFrmSize();
    SwTwips nMinHeight;

    switch (rSz.GetHeightSizeType())
    {
    case ATT_MIN_SIZE:
        nMinHeight = rSz.GetHeight();

        break;

    default:
        nMinHeight = 0;
    }


    return nMinHeight;
}


static SwTwips lcl_CalcContentHeight(SwLayoutFrm & frm)
{
    SwFrm* pFrm = frm.Lower();

    SwTwips nRemaining = 0;
    USHORT nNum = 0;
    pFrm = frm.Lower();
    while ( pFrm )
    {
        SwTwips nTmp;

        nTmp = pFrm->Frm().Height();
        nRemaining += nTmp;
        if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
        {
            nTmp = ((SwTxtFrm*)pFrm)->GetParHeight()
                - pFrm->Prt().Height();
            // Dieser TxtFrm waere gern ein bisschen groesser
            nRemaining += nTmp;
        }
        else if( pFrm->IsSctFrm() && ((SwSectionFrm*)pFrm)->IsUndersized() )
        {
            nTmp = ((SwSectionFrm*)pFrm)->Undersize();
            nRemaining += nTmp;
        }
        pFrm = pFrm->GetNext();

        nNum++;
    }

    return nRemaining;
}

static void lcl_LayoutFrmEnsureMinHeight(SwLayoutFrm & rFrm,
                                         const SwBorderAttrs * pAttrs)
{
    SwTwips nMinHeight = lcl_GetFrmMinHeight(rFrm);

    if (rFrm.Frm().Height() < nMinHeight)
    {
        rFrm.Grow(nMinHeight - rFrm.Frm().Height());
    }
}

SwHeadFootFrm::SwHeadFootFrm( SwFrmFmt * pFmt, USHORT nTypeIn)
    : SwLayoutFrm(pFmt)
{
    nType = nTypeIn;
#ifdef VERTICAL_LAYOUT
    SetDerivedVert( FALSE );
#endif

    const SwFmtCntnt &rCnt = pFmt->GetCntnt();

    ASSERT( rCnt.GetCntntIdx(), "Kein Inhalt fuer Header." );

    //Fuer Header Footer die Objekte gleich erzeugen lassen.
    FASTBOOL bOld = bObjsDirect;
    bObjsDirect = TRUE;
    ULONG nIndex = rCnt.GetCntntIdx()->GetIndex();
    ::_InsertCnt( this, pFmt->GetDoc(), ++nIndex );
    bObjsDirect = bOld;
}

void SwHeadFootFrm::FormatPrt(SwTwips & nUL, const SwBorderAttrs * pAttrs)
{
    if (GetEatSpacing())
    {
        /* The minimal height of the print area is the minimal height of the
           frame without the height needed for borders and shadow. */
        SwTwips nMinHeight = lcl_GetFrmMinHeight(*this);

        nMinHeight -= pAttrs->CalcTop();
        nMinHeight -= pAttrs->CalcBottom();

        /* If the minimal height of the print area is negative, try to
           compensate by overlapping */
        SwTwips nOverlap = 0;
        if (nMinHeight < 0)
        {
            nOverlap = -nMinHeight;
            nMinHeight = 0;
        }

        /* Calculate desired height of content. The minimal height has to be
           adhered. */
        SwTwips nHeight;

        if ( ! HasFixSize() )
            nHeight = lcl_CalcContentHeight(*this);
        else
            nHeight = nMinHeight;

        if (nHeight < nMinHeight)
            nHeight = nMinHeight;

        /* calculate initial spacing/line space */
        SwTwips nSpace, nLine;

        if (IsHeaderFrm())
        {
            nSpace = pAttrs->CalcBottom();
            nLine = pAttrs->CalcBottomLine();
        }
        else
        {
            nSpace = pAttrs->CalcTop();
            nLine = pAttrs->CalcTopLine();
        }

        /* calculate overlap and correct spacing */
        nOverlap += nHeight - nMinHeight;
        if (nOverlap < nSpace - nLine)
            nSpace -= nOverlap;
        else
            nSpace = nLine;

        /* calculate real vertical space between frame and print area */
        if (IsHeaderFrm())
            nUL = pAttrs->CalcTop() + nSpace;
        else
            nUL = pAttrs->CalcBottom() + nSpace;

        /* set print area */
        // OD 23.01.2003 #106895# - add first parameter to <SwBorderAttrs::CalcRight(..)>
        SwTwips nLR = pAttrs->CalcLeft( this ) + pAttrs->CalcRight( this );

        aPrt.Left(pAttrs->CalcLeft(this));

        if (IsHeaderFrm())
            aPrt.Top(pAttrs->CalcTop());
        else
            aPrt.Top(nSpace);

        aPrt.Width(aFrm.Width() - nLR);

        SwTwips nNewHeight;

        if (nUL < aFrm.Height())
            nNewHeight = aFrm.Height() - nUL;
        else
            nNewHeight = 0;

        aPrt.Height(nNewHeight);

    }
    else
    {
        //Position einstellen.
        aPrt.Left( pAttrs->CalcLeft( this ) );
        aPrt.Top ( pAttrs->CalcTop()  );

        //Sizes einstellen; die Groesse gibt der umgebende Frm vor, die
        //die Raender werden einfach abgezogen.
        // OD 23.01.2003 #106895# - add first parameter to <SwBorderAttrs::CalcRight(..)>
        SwTwips nLR = pAttrs->CalcLeft( this ) + pAttrs->CalcRight( this );
        aPrt.Width ( aFrm.Width() - nLR );
        aPrt.Height( aFrm.Height()- nUL );

    }

    bValidPrtArea = TRUE;
}

void SwHeadFootFrm::FormatSize(SwTwips nUL, const SwBorderAttrs * pAttrs)
{
    if ( !HasFixSize() )
    {
        if( !IsColLocked() )
        {
            bValidSize = bValidPrtArea = TRUE;

            const SwTwips nBorder = nUL;
            SwTwips nMinHeight = lcl_GetFrmMinHeight(*this);
            nMinHeight -= pAttrs->CalcTop();
            nMinHeight -= pAttrs->CalcBottom();

            if (nMinHeight < 0)
                nMinHeight = 0;

            ColLock();

            SwTwips nMaxHeight = LONG_MAX;
            SwTwips nRemaining, nOldHeight;
            Point aOldPos;

            do
            {
                nOldHeight = Prt().Height();
                SwFrm* pFrm = Lower();
                if( Frm().Pos() != aOldPos && pFrm )
                {
                    pFrm->_InvalidatePos();
                    aOldPos = Frm().Pos();
                }
                while( pFrm )
                {
                    pFrm->Calc();
                    pFrm = pFrm->GetNext();
                }
                nRemaining = 0;
                pFrm = Lower();

                while ( pFrm )
                {
                    nRemaining += pFrm->Frm().Height();

                    if( pFrm->IsTxtFrm() &&
                        ((SwTxtFrm*)pFrm)->IsUndersized() )
                        // Dieser TxtFrm waere gern ein bisschen groesser
                        nRemaining += ((SwTxtFrm*)pFrm)->GetParHeight()
                            - pFrm->Prt().Height();
                    else if( pFrm->IsSctFrm() &&
                             ((SwSectionFrm*)pFrm)->IsUndersized() )
                        nRemaining += ((SwSectionFrm*)pFrm)->Undersize();
                    pFrm = pFrm->GetNext();
                }
                if ( nRemaining < nMinHeight )
                    nRemaining = nMinHeight;

                SwTwips nDiff = nRemaining - nOldHeight;

                if( !nDiff )
                    break;
                if( nDiff < 0 )
                {
                    nMaxHeight = nOldHeight;

                    if( nRemaining <= nMinHeight )
                        nRemaining = ( nMaxHeight + nMinHeight + 1 ) / 2;
                }
                else
                {
                    if (nOldHeight > nMinHeight)
                        nMinHeight = nOldHeight;

                    if( nRemaining >= nMaxHeight )
                        nRemaining = ( nMaxHeight + nMinHeight + 1 ) / 2;
                }

                nDiff = nRemaining - nOldHeight;

                if ( nDiff )
                {
                    ColUnlock();
                    if ( nDiff > 0 )
                    {
                        if ( Grow( nDiff PHEIGHT ) )
                        {
                            pFrm = Lower();

                            while ( pFrm )
                            {
                                if( pFrm->IsTxtFrm())
                                {
                                    SwTxtFrm * pTmpFrm = (SwTxtFrm*) pFrm;
                                    if (pTmpFrm->IsUndersized() )
                                    {
                                        pTmpFrm->InvalidateSize();
                                        pTmpFrm->Prepare(PREP_ADJUST_FRM);
                                    }
                                }
                                /* #i3568# Undersized sections need to be
                                   invalidated too. */
                                else if (pFrm->IsSctFrm())
                                {
                                    SwSectionFrm * pTmpFrm =
                                        (SwSectionFrm*) pFrm;
                                    if (pTmpFrm->IsUndersized() )
                                    {
                                        pTmpFrm->InvalidateSize();
                                        pTmpFrm->Prepare(PREP_ADJUST_FRM);
                                    }
                                }
                                pFrm = pFrm->GetNext();
                            }
                        }
                    }
                    else
                        Shrink( -nDiff PHEIGHT );
                    //Schnell auf dem kurzen Dienstweg die Position updaten.

                    MakePos();
                    ColLock();
                }
                else
                    break;
                //Unterkante des Uppers nicht ueberschreiten.
                if ( GetUpper() && Frm().Height() )
                {
                    const SwTwips nDeadLine = GetUpper()->Frm().Top() +
                        GetUpper()->Prt().Bottom();
                    const SwTwips nBot = Frm().Bottom();
                    if ( nBot > nDeadLine )
                    {
                        Frm().Bottom( nDeadLine );
                        Prt().SSize().Height() = Frm().Height() - nBorder;
                    }
                }
                bValidSize = bValidPrtArea = TRUE;
            } while( nRemaining<=nMaxHeight && nOldHeight!=Prt().Height() );
            ColUnlock();
        }
        bValidSize = bValidPrtArea = TRUE;
    }
    else //if ( GetType() & 0x0018 )
    {
        do
        {
            if ( Frm().Height() != pAttrs->GetSize().Height() )
                ChgSize( Size( Frm().Width(), pAttrs->GetSize().Height()));
            bValidSize = TRUE;
            MakePos();
        } while ( !bValidSize );
    }
}

void SwHeadFootFrm::Format(const SwBorderAttrs * pAttrs)
{
    ASSERT( pAttrs, "SwFooterFrm::Format, pAttrs ist 0." );

    if ( bValidPrtArea && bValidSize )
        return;

    if ( ! GetEatSpacing() && IsHeaderFrm())
    {
        SwLayoutFrm::Format(pAttrs);
    }
    else
    {
        lcl_LayoutFrmEnsureMinHeight(*this, pAttrs);

        long nUL = pAttrs->CalcTop()  + pAttrs->CalcBottom();

        if ( !bValidPrtArea )
            FormatPrt(nUL, pAttrs);

        if ( !bValidSize )
            FormatSize(nUL, pAttrs);
    }
}

SwTwips SwHeadFootFrm::GrowFrm( SwTwips nDist, BOOL bTst,  BOOL bInfo )
{
    SwTwips nResult;

    if ( IsColLocked() )
    {
        nResult = 0;
    }
    else if (!GetEatSpacing())
    {
        nResult = SwLayoutFrm::GrowFrm(nDist, bTst, bInfo);
    }
    else
    {
        nResult = 0;

        SwBorderAttrAccess * pAccess =
            new SwBorderAttrAccess( SwFrm::GetCache(), this );
        ASSERT(pAccess, "no border attributes");

        SwBorderAttrs * pAttrs = pAccess->Get();

        /* First assume the whole amount to grow can be provided by eating
           spacing. */
        SwTwips nEat = nDist;
        SwTwips nMaxEat;

        /* calculate maximum eatable spacing */
        if (IsHeaderFrm())
            nMaxEat = aFrm.Height() - aPrt.Top() - aPrt.Height() - pAttrs->CalcBottomLine();
        else
            nMaxEat = aPrt.Top() - pAttrs->CalcTopLine();

        delete pAccess;

        if (nMaxEat < 0)
            nMaxEat = 0;

        /* If the frame is too small, eat less spacing thus letting the frame
           grow more. */
        SwTwips nMinHeight = lcl_GetFrmMinHeight(*this);
        SwTwips nFrameTooSmall = nMinHeight - Frm().Height();

        if (nFrameTooSmall > 0)
            nEat -= nFrameTooSmall;

        /* No negative eating, not eating more than allowed. */
        if (nEat < 0)
            nEat = 0;
        else if (nEat > nMaxEat)
            nEat = nMaxEat;

        // OD 10.04.2003 #108719# - Notify fly frame, if header frame
        // grows. Consider, that 'normal' grow of layout frame already notifys
        // the fly frames.
        sal_Bool bNotifyFlys = sal_False;
        if (nEat > 0)
        {
            if ( ! bTst)
            {
                if (! IsHeaderFrm())
                {
                    aPrt.Top(aPrt.Top() - nEat);
                    aPrt.Height(aPrt.Height() - nEat);
                }

                InvalidateAll();
            }

            nResult += nEat;
            // OD 14.04.2003 #108719# - trigger fly frame notify.
            if ( IsHeaderFrm() )
            {
                bNotifyFlys = sal_True;
            }
        }

        if (nDist - nEat > 0)
        {
            SwTwips nFrmGrow =
                SwLayoutFrm::GrowFrm( nDist - nEat, bTst, bInfo );

            nResult += nFrmGrow;
            if ( nFrmGrow > 0 )
            {
                bNotifyFlys = sal_False;
            }
        }

        // OD 10.04.2003 #108719# - notify fly frames, if necessary and triggered.
        if ( ( nResult > 0 ) && bNotifyFlys )
        {
            NotifyLowerObjs();
        }
    }

    if ( nResult && !bTst )
        SetCompletePaint();

    return nResult;
}

SwTwips SwHeadFootFrm::ShrinkFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
{
    SwTwips nResult;

    if ( IsColLocked() )
    {
        nResult = 0;
    }
    else if (! GetEatSpacing())
    {
        nResult = SwLayoutFrm::ShrinkFrm(nDist, bTst, bInfo);
    }
    else
    {
        nResult = 0;

        SwTwips nMinHeight = lcl_GetFrmMinHeight(*this);
        SwTwips nOldHeight = Frm().Height();
        SwTwips nRest = 0; // Amount to shrink by spitting out spacing

        if ( nOldHeight >= nMinHeight )
        {
            /* If the frame's height is bigger than its minimum height, shrink
               the frame towards its minimum height. If this is not sufficient
               to provide the shrinking requested provide the rest by spitting
               out spacing. */

            SwTwips nBiggerThanMin = nOldHeight - nMinHeight;

            if (nBiggerThanMin < nDist)
            {
                nRest = nDist - nBiggerThanMin;
            }
            /* info: declaration of nRest -> else nRest = 0 */
        }
        else
            /* The frame cannot shrink. Provide shrinking by spitting out
               spacing. */
            nRest = nDist;

        // OD 10.04.2003 #108719# - Notify fly frame, if header/footer frame
        // shrinks. Consider, that 'normal' shrink of layout frame already notifys
        // the fly frames.
        sal_Bool bNotifyFlys = sal_False;
        if (nRest > 0)
        {

            SwBorderAttrAccess * pAccess =
                new SwBorderAttrAccess( SwFrm::GetCache(), this );
            ASSERT(pAccess, "no border attributes");

            SwBorderAttrs * pAttrs = pAccess->Get();

            /* minimal height of print area */
            SwTwips nMinPrtHeight = nMinHeight
                - pAttrs->CalcTop()
                - pAttrs->CalcBottom();

            if (nMinPrtHeight < 0)
                nMinPrtHeight = 0;

            delete pAccess;

            /* assume all shrinking can be provided */
            SwTwips nShrink = nRest;

            /* calculate maximum shrinking */
            SwTwips nMaxShrink = aPrt.Height() - nMinPrtHeight;

            /* shrink no more than maximum shrinking */
            if (nShrink > nMaxShrink)
            {
                //nRest -= nShrink - nMaxShrink;
                nShrink = nMaxShrink;
            }

            if (!bTst)
            {
                if (! IsHeaderFrm() )
                {
                    aPrt.Top(aPrt.Top() + nShrink);
                    aPrt.Height(aPrt.Height() - nShrink);
                }

                InvalidateAll();
            }
            nResult += nShrink;
            // OD 14.04.2003 #108719# - trigger fly frame notify.
            if ( IsHeaderFrm() )
            {
                bNotifyFlys = sal_True;
            }
        }

        /* The shrinking not providable by spitting out spacing has to be done
           by the frame. */
        if (nDist - nRest > 0)
        {
            SwTwips nShrinkAmount = SwLayoutFrm::ShrinkFrm( nDist - nRest, bTst, bInfo );
            nResult += nShrinkAmount;
            if ( nShrinkAmount > 0 )
            {
                bNotifyFlys = sal_False;
            }
        }

        // OD 10.04.2003 #108719# - notify fly frames, if necessary.
        if ( ( nResult > 0 ) && bNotifyFlys )
        {
            NotifyLowerObjs();
        }
    }

    return nResult;
}

BOOL SwHeadFootFrm::GetEatSpacing() const
{
    const SwFrmFmt * pFmt = GetFmt();
    ASSERT(pFmt, "SwHeadFootFrm: no format?");

    if (pFmt->GetHeaderAndFooterEatSpacing().GetValue())
        return TRUE;

    return FALSE;
}


/*************************************************************************
|*
|*  SwPageFrm::PrepareHeader()
|*
|*  Beschreibung        Erzeugt oder Entfernt Header
|*  Ersterstellung      MA 04. Feb. 93
|*  Letzte Aenderung    MA 12. May. 96
|*
|*************************************************************************/


void DelFlys( SwLayoutFrm *pFrm, SwPageFrm *pPage )
{
    for ( int i = 0; pPage->GetSortedObjs() &&
                        pPage->GetSortedObjs()->Count() &&
                        i < (int)pPage->GetSortedObjs()->Count(); ++i )
    {
        SwAnchoredObject* pObj = (*pPage->GetSortedObjs())[i];
        if ( pObj->ISA(SwFlyFrm) )
        {
            SwFlyFrm* pFlyFrm = static_cast<SwFlyFrm*>(pObj);
            if ( pFrm->IsAnLower( pFlyFrm ) )
            {
                delete pFlyFrm;
                --i;
            }
        }
    }
}



void SwPageFrm::PrepareHeader()
{
    SwLayoutFrm *pLay = (SwLayoutFrm*)Lower();
    if ( !pLay )
        return;

    const SwFmtHeader &rH = ((SwFrmFmt*)pRegisteredIn)->GetHeader();

    const FASTBOOL bOn = !((SwFrmFmt*)pRegisteredIn)->GetDoc()->IsBrowseMode();

    if ( bOn && rH.IsActive() )
    {   //Header einsetzen, vorher entfernen falls vorhanden.
        ASSERT( rH.GetHeaderFmt(), "FrmFmt fuer Header nicht gefunden." );

        if ( pLay->GetFmt() == (SwFrmFmt*)rH.GetHeaderFmt() )
            return; //Der Footer ist bereits der richtige

        if ( pLay->IsHeaderFrm() )
        {   SwLayoutFrm *pDel = pLay;
            pLay = (SwLayoutFrm*)pLay->GetNext();
            ::DelFlys( pDel, this );
            pDel->Cut();
            delete pDel;
        }
        ASSERT( pLay, "Wohin mit dem Header?" );
        SwHeaderFrm *pH = new SwHeaderFrm( (SwFrmFmt*)rH.GetHeaderFmt() );
        pH->Paste( this, pLay );
        if ( GetUpper() )
            ::RegistFlys( this, pH );
    }
    else if ( pLay && pLay->IsHeaderFrm() )
    {   //Header entfernen falls vorhanden.
        ::DelFlys( pLay, this );
        pLay->Cut();
        delete pLay;
    }
}
/*************************************************************************
|*
|*  SwPageFrm::PrepareFooter()
|*
|*  Beschreibung        Erzeugt oder Entfernt Footer
|*  Ersterstellung      MA 04. Feb. 93
|*  Letzte Aenderung    MA 12. May. 96
|*
|*************************************************************************/


void SwPageFrm::PrepareFooter()
{
    SwLayoutFrm *pLay = (SwLayoutFrm*)Lower();
    if ( !pLay )
        return;

    const SwFmtFooter &rF = ((SwFrmFmt*)pRegisteredIn)->GetFooter();
    while ( pLay->GetNext() )
        pLay = (SwLayoutFrm*)pLay->GetNext();

    const FASTBOOL bOn = !((SwFrmFmt*)pRegisteredIn)->GetDoc()->IsBrowseMode();

    if ( bOn && rF.IsActive() )
    {   //Footer einsetzen, vorher entfernen falls vorhanden.
        ASSERT( rF.GetFooterFmt(), "FrmFmt fuer Footer nicht gefunden." );

        if ( pLay->GetFmt() == (SwFrmFmt*)rF.GetFooterFmt() )
            return; //Der Footer ist bereits der richtige.

        if ( pLay->IsFooterFrm() )
        {   ::DelFlys( pLay, this );
            pLay->Cut();
            delete pLay;
        }
        SwFooterFrm *pF = new SwFooterFrm( (SwFrmFmt*)rF.GetFooterFmt() );
        pF->Paste( this );
        if ( GetUpper() )
            ::RegistFlys( this, pF );
    }
    else if ( pLay && pLay->IsFooterFrm() )
    {   //Footer entfernen falls vorhanden.
        ::DelFlys( pLay, this );
        ViewShell *pSh;
        if ( pLay->GetPrev() && 0 != (pSh = GetShell()) &&
             pSh->VisArea().HasArea() )
            pSh->InvalidateWindows( pSh->VisArea() );
        pLay->Cut();
        delete pLay;
    }
}



