/*************************************************************************
 *
 *  $RCSfile: hffrm.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:22 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

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

extern FASTBOOL bObjsDirect;    //frmtool.cxx

/*************************************************************************
|*
|*  SwHeaderFrm::SwHeaderFrm()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA ??
|*
|*************************************************************************/


SwHeaderFrm::SwHeaderFrm( SwFrmFmt *pFmt ):
    SwLayoutFrm( pFmt )
{
    nType = FRM_HEADER;

    const SwFmtCntnt &rCnt = pFmt->GetCntnt();

    ASSERT( rCnt.GetCntntIdx(), "Kein Inhalt fuer Header." );

    //Fuer Header Footer die Objekte gleich erzeugen lassen.
    FASTBOOL bOld = bObjsDirect;
    bObjsDirect = TRUE;
    ULONG nIndex = rCnt.GetCntntIdx()->GetIndex();
    ::_InsertCnt( this, pFmt->GetDoc(), ++nIndex );
    bObjsDirect = bOld;
}

SwTwips SwHeaderFrm::GrowFrm( SwTwips nDist, const SzPtr pPtr,
                              BOOL bTst, BOOL bInfo )
{
    SwTwips nRet = SwLayoutFrm::GrowFrm( nDist, pPtr, bTst, bInfo );
    if ( nRet && !bTst )
        SetCompletePaint();
    return nRet;
}

SwTwips SwHeaderFrm::ShrinkFrm( SwTwips nDist, const SzPtr pPtr,
                               BOOL bTst, BOOL bInfo )
{
    SwTwips nRet = SwLayoutFrm::ShrinkFrm( nDist, pPtr, bTst, bInfo );
    if ( nRet && !bTst )
        SetCompletePaint();
    return nRet;
}


/*************************************************************************
|*
|*  SwFooterFrm::SwFooterFrm()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA ??
|*
|*************************************************************************/


SwFooterFrm::SwFooterFrm( SwFrmFmt *pFmt ):
    SwLayoutFrm( pFmt )
{
    nType = FRM_FOOTER;

    const SwFmtCntnt &rCnt = pFmt->GetCntnt();

    ASSERT( rCnt.GetCntntIdx(), "Kein Inhalt fuer Footer." );

    //Fuer Header Footer die Objekte gleich erzeugen lassen.
    FASTBOOL bOld = bObjsDirect;
    bObjsDirect = TRUE;
    ULONG nIndex = rCnt.GetCntntIdx()->GetIndex();
    ::_InsertCnt( this, pFmt->GetDoc(), ++nIndex );
    bObjsDirect = bOld;
}

void SwFooterFrm::Format( const SwBorderAttrs *pAttrs )
{
    ASSERT( pAttrs, "SwFooterFrm::Format, pAttrs ist 0." );

    if ( bValidPrtArea && bValidSize )
        return;

    const USHORT nLR = pAttrs->CalcLeft( this ) + pAttrs->CalcRight();
    const USHORT nUL = pAttrs->CalcTop()  + pAttrs->CalcBottom();

    if ( !bValidPrtArea )
    {
        bValidPrtArea = TRUE;

        //Position einstellen.
        aPrt.Left( pAttrs->CalcLeft( this ) );
        aPrt.Top ( pAttrs->CalcTop()  );

        //Sizes einstellen; die Groesse gibt der umgebende Frm vor, die
        //die Raender werden einfach abgezogen.
        aPrt.Width ( aFrm.Width() - nLR );
        aPrt.Height( aFrm.Height()- nUL );
    }

    if ( !bValidSize )
    {
        const SzPtr pVarSz = pVARSIZE;
        if ( !HasFixSize( pVarSz ) )
        {
            if( !IsColLocked() )
            {
                bValidSize = bValidPrtArea = TRUE;
                const SwTwips nBorder = bVarHeight ? nUL : nLR;
                const PtPtr pVarPs = pVARPOS;
                const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
                SwTwips nMinHeight = rSz.GetSizeType() == ATT_MIN_SIZE ? rSz.GetHeight() : 0;
                ColLock();
                SwTwips nMaxHeight = LONG_MAX;
                SwTwips nRemaining, nOldHeight;
                Point aOldPos;
                do
                {
                    nOldHeight = Frm().SSize().*pVarSz;
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
                    nRemaining = nBorder;
                    pFrm = Lower();
                    while ( pFrm )
                    {   nRemaining += pFrm->Frm().SSize().*pVarSz;
                        if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
                        // Dieser TxtFrm waere gern ein bisschen groesser
                            nRemaining += ((SwTxtFrm*)pFrm)->GetParHeight()
                                        - pFrm->Prt().Height();
                        else if( pFrm->IsSctFrm() && ((SwSectionFrm*)pFrm)->IsUndersized() )
                            nRemaining += ((SwSectionFrm*)pFrm)->Undersize();
                        pFrm = pFrm->GetNext();
                    }
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
                        nMinHeight = nOldHeight;
                        if( nRemaining >= nMaxHeight )
                            nRemaining = ( nMaxHeight + nMinHeight + 1 ) / 2;
                    }
                    nDiff = nRemaining - nOldHeight;
                    if ( nDiff )
                    {
                        ColUnlock();
                        if ( nDiff > 0 )
                            Grow( nDiff, pVarSz );
                        else
                            Shrink( -nDiff, pVarSz );
                        //Schnell auf dem kurzen Dienstweg die Position updaten.
                        MakePos();
                        ColLock();
                    }
                    else
                        break;
                    //Unterkante des Uppers nicht ueberschreiten.
                    if ( GetUpper() && Frm().SSize().*pVarSz )
                    {
                        const SwTwips nDeadLine =
                            GetUpper()->Frm().Pos().*pVarPs +
                                    (bVarHeight ? GetUpper()->Prt().Bottom() :
                                                GetUpper()->Prt().Right());
                        const SwTwips nBot = bVarHeight ?
                                                    Frm().Bottom():Frm().Right();
                        if ( nBot > nDeadLine )
                        {
                            Frm().Bottom( nDeadLine );
                            Prt().SSize().Height() = Frm().SSize().Height() - nBorder;
                        }
                    }
                    bValidSize = bValidPrtArea = TRUE;
                } while( nRemaining < nMaxHeight && nOldHeight != Frm().SSize().*pVarSz );
                ColUnlock();
            }
            bValidSize = bValidPrtArea = TRUE;
        }
        else if ( GetType() & 0x0018 )
        {
            do
            {   if ( Frm().Height() != pAttrs->GetSize().Height() )
                    ChgSize( Size( Frm().Width(), pAttrs->GetSize().Height()));
                bValidSize = TRUE;
                MakePos();
            } while ( !bValidSize );
        }
        else
            bValidSize = TRUE;
    }
}

SwTwips SwFooterFrm::GrowFrm( SwTwips nDist, const SzPtr pPtr,
                              BOOL bTst, BOOL bInfo )
{
    if( IsColLocked() )
        return 0;
    return SwLayoutFrm::GrowFrm( nDist, pPtr, bTst, bInfo );
}

SwTwips SwFooterFrm::ShrinkFrm( SwTwips nDist, const SzPtr pPtr,
                              BOOL bTst, BOOL bInfo )
{
    if( IsColLocked() )
        return 0;
    return SwLayoutFrm::ShrinkFrm( nDist, pPtr, bTst, bInfo );
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
        SdrObject *pO = (*pPage->GetSortedObjs())[i];
        if ( pO->IsWriterFlyFrame() )
        {
            SwVirtFlyDrawObj *pObj = (SwVirtFlyDrawObj*)pO;
            if ( pFrm->IsAnLower( pObj->GetFlyFrm() ) )
            {
                delete pObj->GetFlyFrm();
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

    const FASTBOOL bOn = !((SwFrmFmt*)pRegisteredIn)->GetDoc()->IsBrowseMode() ||
                          ((SwFrmFmt*)pRegisteredIn)->GetDoc()->IsHeadInBrowse();

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

    const FASTBOOL bOn = !((SwFrmFmt*)pRegisteredIn)->GetDoc()->IsBrowseMode() ||
                         ((SwFrmFmt*)pRegisteredIn)->GetDoc()->IsFootInBrowse();

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



