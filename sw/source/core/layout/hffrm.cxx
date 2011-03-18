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

#include "pagefrm.hxx"
#include "viewsh.hxx"
#include "doc.hxx"
#include <fmtcntnt.hxx>
#include <fmthdft.hxx>
#include <fmtfsize.hxx>
#include "hffrm.hxx"
#include "txtfrm.hxx"
#include "sectfrm.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "dflyobj.hxx"
#include "frmfmt.hxx"
#include "ndindex.hxx"
#include "hfspacingitem.hxx"
// OD 2004-05-24 #i28701#
#include <sortedobjs.hxx>
// --> OD 2005-03-03 #i43771#
#include <objectformatter.hxx>
// <--

extern sal_Bool bObjsDirect;    //frmtool.cxx

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
    sal_uInt16 nNum = 0;
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
                                         const SwBorderAttrs * )
{
    SwTwips nMinHeight = lcl_GetFrmMinHeight(rFrm);

    if (rFrm.Frm().Height() < nMinHeight)
    {
        rFrm.Grow(nMinHeight - rFrm.Frm().Height());
    }
}

SwHeadFootFrm::SwHeadFootFrm( SwFrmFmt * pFmt, sal_uInt16 nTypeIn)
    : SwLayoutFrm(pFmt)
{
    nType = nTypeIn;
    SetDerivedVert( sal_False );

    const SwFmtCntnt &rCnt = pFmt->GetCntnt();

    OSL_ENSURE( rCnt.GetCntntIdx(), "Kein Inhalt fuer Header." );

    //Fuer Header Footer die Objekte gleich erzeugen lassen.
    sal_Bool bOld = bObjsDirect;
    bObjsDirect = sal_True;
    sal_uLong nIndex = rCnt.GetCntntIdx()->GetIndex();
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

    bValidPrtArea = sal_True;
}

void SwHeadFootFrm::FormatSize(SwTwips nUL, const SwBorderAttrs * pAttrs)
{
    if ( !HasFixSize() )
    {
        if( !IsColLocked() )
        {
            bValidSize = bValidPrtArea = sal_True;

            const SwTwips nBorder = nUL;
            SwTwips nMinHeight = lcl_GetFrmMinHeight(*this);
            nMinHeight -= pAttrs->CalcTop();
            nMinHeight -= pAttrs->CalcBottom();

            if (nMinHeight < 0)
                nMinHeight = 0;

            ColLock();

            SwTwips nMaxHeight = LONG_MAX;
            SwTwips nRemaining, nOldHeight;
            // --> OD 2006-05-24 #i64301#
            // use the position of the footer printing area to control invalidation
            // of the first footer content.
            Point aOldFooterPrtPos;
            // <--

            do
            {
                nOldHeight = Prt().Height();
                SwFrm* pFrm = Lower();
                // --> OD 2006-05-24 #i64301#
                if ( pFrm &&
                     aOldFooterPrtPos != ( Frm().Pos() + Prt().Pos() ) )
                {
                    pFrm->_InvalidatePos();
                    aOldFooterPrtPos = Frm().Pos() + Prt().Pos();
                }
                // <--
                while( pFrm )
                {
                    pFrm->Calc();
                    // --> OD 2005-03-03 #i43771# - format also object anchored
                    // at the frame
                    // --> OD 2005-05-03 #i46941# - frame has to be valid.
                    // Note: frame could be invalid after calling its format,
                    //       if it's locked
                    OSL_ENSURE( StackHack::IsLocked() || !pFrm->IsTxtFrm() ||
                            pFrm->IsValid() ||
                            static_cast<SwTxtFrm*>(pFrm)->IsJoinLocked(),
                            "<SwHeadFootFrm::FormatSize(..)> - text frame invalid and not locked." );
                    if ( pFrm->IsTxtFrm() && pFrm->IsValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrm( *pFrm,
                                                                  *(pFrm->FindPageFrm()) ) )
                        {
                            // restart format with first content
                            pFrm = Lower();
                            continue;
                        }
                    }
                    // <--
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
                        if ( Grow( nDiff ) )
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
                        Shrink( -nDiff );
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
                bValidSize = bValidPrtArea = sal_True;
            } while( nRemaining<=nMaxHeight && nOldHeight!=Prt().Height() );
            ColUnlock();
        }
        bValidSize = bValidPrtArea = sal_True;
    }
    else //if ( GetType() & 0x0018 )
    {
        do
        {
            if ( Frm().Height() != pAttrs->GetSize().Height() )
                ChgSize( Size( Frm().Width(), pAttrs->GetSize().Height()));
            bValidSize = sal_True;
            MakePos();
        } while ( !bValidSize );
    }
}

void SwHeadFootFrm::Format(const SwBorderAttrs * pAttrs)
{
    OSL_ENSURE( pAttrs, "SwFooterFrm::Format, pAttrs ist 0." );

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

SwTwips SwHeadFootFrm::GrowFrm( SwTwips nDist, sal_Bool bTst,  sal_Bool bInfo )
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
        OSL_ENSURE(pAccess, "no border attributes");

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

SwTwips SwHeadFootFrm::ShrinkFrm( SwTwips nDist, sal_Bool bTst, sal_Bool bInfo )
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
            OSL_ENSURE(pAccess, "no border attributes");

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

sal_Bool SwHeadFootFrm::GetEatSpacing() const
{
    const SwFrmFmt * pFmt = GetFmt();
    OSL_ENSURE(pFmt, "SwHeadFootFrm: no format?");

    if (pFmt->GetHeaderAndFooterEatSpacing().GetValue())
        return sal_True;

    return sal_False;
}


/*************************************************************************
|*
|*  SwPageFrm::PrepareHeader()
|*
|*  Beschreibung        Erzeugt oder Entfernt Header
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

    const sal_Bool bOn = !((SwFrmFmt*)pRegisteredIn)->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE);

    if ( bOn && rH.IsActive() )
    {   //Header einsetzen, vorher entfernen falls vorhanden.
        OSL_ENSURE( rH.GetHeaderFmt(), "FrmFmt fuer Header nicht gefunden." );

        if ( pLay->GetFmt() == (SwFrmFmt*)rH.GetHeaderFmt() )
            return; //Der Footer ist bereits der richtige

        if ( pLay->IsHeaderFrm() )
        {   SwLayoutFrm *pDel = pLay;
            pLay = (SwLayoutFrm*)pLay->GetNext();
            ::DelFlys( pDel, this );
            pDel->Cut();
            delete pDel;
        }
        OSL_ENSURE( pLay, "Wohin mit dem Header?" );
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

    const sal_Bool bOn = !((SwFrmFmt*)pRegisteredIn)->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE);

    if ( bOn && rF.IsActive() )
    {   //Footer einsetzen, vorher entfernen falls vorhanden.
        OSL_ENSURE( rF.GetFooterFmt(), "FrmFmt fuer Footer nicht gefunden." );

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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
