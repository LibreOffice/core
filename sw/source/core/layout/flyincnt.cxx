/*************************************************************************
 *
 *  $RCSfile: flyincnt.cxx,v $
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

#include "cntfrm.hxx"
#include "doc.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "frmfmt.hxx"
#include "hints.hxx"

#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#include "txtfrm.hxx"       //fuer IsLocked()
#include "flyfrms.hxx"


#if !defined( PRODUCT ) && defined( WNT ) && !defined( ALPHA )
USHORT DiesIstEinDummyFuerLibMist = 0;
#endif

//aus FlyCnt.cxx
void DeepCalc( const SwFrm *pFrm );

/*************************************************************************
|*
|*  SwFlyInCntFrm::SwFlyInCntFrm(), ~SwFlyInCntFrm()
|*
|*  Ersterstellung      MA 01. Dec. 92
|*  Letzte Aenderung    MA 09. Apr. 99
|*
|*************************************************************************/
SwFlyInCntFrm::SwFlyInCntFrm( SwFlyFrmFmt *pFmt, SwFrm *pAnch ) :
    SwFlyFrm( pFmt, pAnch )
{
    bInCnt = bInvalidLayout = bInvalidCntnt = TRUE;
    const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
    aRelPos.Y() = rVert.GetPos();
}

SwFlyInCntFrm::~SwFlyInCntFrm()
{
    //und Tschuess.
    if ( !GetFmt()->GetDoc()->IsInDtor() && GetAnchor() )
    {
        SwRect aTmp( AddSpacesToFrm() );
        SwFlyInCntFrm::NotifyBackground( FindPageFrm(), aTmp, PREP_FLY_LEAVE );
    }
}

/*************************************************************************
|*
|*  SwFlyInCntFrm::SetRefPoint(),
|*
|*  Ersterstellung      MA 01. Dec. 92
|*  Letzte Aenderung    MA 06. Aug. 95
|*
|*************************************************************************/
void SwFlyInCntFrm::SetRefPoint( const Point& rPoint, const Point& rRelPos )
{
    ASSERT( rPoint != aRef || rRelPos != aRelPos, "SetRefPoint: no change" );
    const SwFlyNotify aNotify( this );
    aRef = rPoint;
    aRelPos = rRelPos;
    Frm().Pos( rPoint + rRelPos );
/*
    //Kein InvalidatePos hier, denn das wuerde dem Cntnt ein Prepare
    //senden - dieser hat uns aber gerade gerufen.
    //Da der Frm aber durchaus sein Position wechseln kann, muss hier
    //der von ihm abdeckte Window-Bereich invalidiert werden damit keine
    //Reste stehenbleiben.
    //Fix: Nicht fuer PreView-Shells, dort ist es nicht notwendig und
    //fuehrt zu fiesen Problemen (Der Absatz wird nur formatiert weil
    //er gepaintet wird und der Cache uebergelaufen ist, beim Paint durch
    //das Invalidate wird der Absatz formatiert weil...)
    if ( Frm().HasArea() && GetShell()->ISA(SwCrsrShell) )
        GetShell()->InvalidateWindows( Frm() );
*/
    InvalidatePage();
    bValidPos = FALSE;
    bInvalid  = TRUE;
    Calc();
}

/*************************************************************************
|*
|*  SwFlyInCntFrm::Modify()
|*
|*  Ersterstellung      MA 16. Dec. 92
|*  Letzte Aenderung    MA 02. Sep. 93
|*
|*************************************************************************/
void SwFlyInCntFrm::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
    BOOL bCallPrepare = FALSE;
    USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    if( RES_ATTRSET_CHG == nWhich )
    {
        if( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->
            GetItemState( RES_SURROUND, FALSE ) ||
            SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->
            GetItemState( RES_FRMMACRO, FALSE ) )
        {
            SwAttrSetChg aOld( *(SwAttrSetChg*)pOld );
            SwAttrSetChg aNew( *(SwAttrSetChg*)pNew );

            aOld.ClearItem( RES_SURROUND );
            aNew.ClearItem( RES_SURROUND );
            aOld.ClearItem( RES_FRMMACRO );
            aNew.ClearItem( RES_FRMMACRO );
            if( aNew.Count() )
            {
                SwFlyFrm::Modify( &aOld, &aNew );
                bCallPrepare = TRUE;
            }
        }
        else if( ((SwAttrSetChg*)pNew)->GetChgSet()->Count())
        {
            SwFlyFrm::Modify( pOld, pNew );
            bCallPrepare = TRUE;
        }
    }
    else if( nWhich != RES_SURROUND && RES_FRMMACRO != nWhich )
    {
        SwFlyFrm::Modify( pOld, pNew );
        bCallPrepare = TRUE;
    }

    if ( bCallPrepare && GetAnchor() )
        GetAnchor()->Prepare( PREP_FLY_ATTR_CHG, GetFmt() );
}
/*************************************************************************
|*
|*  SwFlyInCntFrm::Format()
|*
|*  Beschreibung:       Hier wird der Inhalt initial mit Formatiert.
|*  Ersterstellung      MA 16. Dec. 92
|*  Letzte Aenderung    MA 19. May. 93
|*
|*************************************************************************/
void SwFlyInCntFrm::Format( const SwBorderAttrs *pAttrs )
{
    if ( !Frm().Height() )
    {
        Lock(); //nicht hintenherum den Anker formatieren.
        SwCntntFrm *pCntnt = ContainsCntnt();
        while ( pCntnt )
        {   pCntnt->Calc();
            pCntnt = pCntnt->GetNextCntntFrm();
        }
        Unlock();
    }
    SwFlyFrm::Format( pAttrs );
}
/*************************************************************************
|*
|*  SwFlyInCntFrm::MakeFlyPos()
|*
|*  Beschreibung        Im Unterschied zu anderen Frms wird hier nur die
|*      die RelPos berechnet. Die absolute Position wird ausschliesslich
|*      per SetAbsPos errechnet.
|*  Ersterstellung      MA 03. Dec. 92
|*  Letzte Aenderung    MA 12. Apr. 96
|*
|*************************************************************************/
void SwFlyInCntFrm::MakeFlyPos()
{
    if ( !bValidPos )
    {
        if ( !GetAnchor()->IsTxtFrm() || !((SwTxtFrm*)GetAnchor())->IsLocked() )
            ::DeepCalc( GetAnchor() );
        if( GetAnchor()->IsTxtFrm() )
            ((SwTxtFrm*)GetAnchor())->GetFormatted();
        bValidPos = TRUE;
        SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
        const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
        //Und ggf. noch die aktuellen Werte im Format updaten, dabei darf
        //zu diesem Zeitpunkt natuerlich kein Modify verschickt werden.
        if ( rVert.GetPos() != aRelPos.Y() )
        {
            SwFmtVertOrient aVert( rVert );
            aVert.SetPos( aRelPos.Y() );
            pFmt->LockModify();
            pFmt->SetAttr( aVert );
            pFmt->UnlockModify();
        }
    }
}

/*************************************************************************
|*
|*  SwFlyInCntFrm::NotifyBackground()
|*
|*  Ersterstellung      MA 03. Dec. 92
|*  Letzte Aenderung    MA 26. Aug. 93
|*
|*************************************************************************/
void SwFlyInCntFrm::NotifyBackground( SwPageFrm *, const SwRect& rRect,
                                       PrepareHint eHint)
{
    if ( eHint == PREP_FLY_ATTR_CHG )
        GetAnchor()->Prepare( PREP_FLY_ATTR_CHG );
    else
        GetAnchor()->Prepare( eHint, (void*)&rRect );
}

/*************************************************************************
|*
|*  SwFlyInCntFrm::GetRelPos()
|*
|*  Ersterstellung      MA 04. Dec. 92
|*  Letzte Aenderung    MA 04. Dec. 92
|*
|*************************************************************************/
const Point &SwFlyInCntFrm::GetRelPos() const
{
    Calc();
    return GetCurRelPos();
}

/*************************************************************************
|*
|*  SwFlyInCntFrm::RegistFlys()
|*
|*  Ersterstellung      MA 26. Nov. 93
|*  Letzte Aenderung    MA 26. Nov. 93
|*
|*************************************************************************/
void SwFlyInCntFrm::RegistFlys()
{
    // vgl. SwRowFrm::RegistFlys()
    SwPageFrm *pPage = FindPageFrm();
    ASSERT( pPage, "Flys ohne Seite anmelden?" );
    ::RegistFlys( pPage, this );
}

/*************************************************************************
|*
|*  SwFlyInCntFrm::MakeAll()
|*
|*  Ersterstellung      MA 18. Feb. 94
|*  Letzte Aenderung    MA 13. Jun. 96
|*
|*************************************************************************/
void SwFlyInCntFrm::MakeAll()
{
    if ( !GetAnchor() || IsLocked() || IsColLocked() || !FindPageFrm() )
        return;

    Lock(); //Der Vorhang faellt

        //uebernimmt im DTor die Benachrichtigung
    const SwFlyNotify aNotify( this );
    SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();
    const Size &rSz = rAttrs.GetSize();
    const SwFmtFrmSize &rFrmSz = GetFmt()->GetFrmSize();

    if ( IsClipped() )
        bValidSize = bHeightClipped = bWidthClipped = FALSE;

    while ( !bValidPos || !bValidSize || !bValidPrtArea )
    {
        //Nur einstellen wenn das Flag gesetzt ist!!
        if ( !bValidSize )
        {
            bValidPrtArea = FALSE;
            long nOldWidth = aFrm.Width();
            aFrm.Width( CalcRel( rFrmSz ).Width() );

            if ( aFrm.Width() > nOldWidth )
                //Damit sich der Inhalt anpasst
                aFrm.Height( CalcRel( rFrmSz ).Height() );
        }

        if ( !bValidPrtArea )
            MakePrtArea( rAttrs );

        if ( !bValidSize )
            Format( &rAttrs );

        if ( !bValidPos )
            MakeFlyPos();

        if ( bValidPos && bValidSize )
        {
            SwFrm *pFrm = GetAnchor();
            if (
//MA 03. Apr. 96 fix(26652), Das trifft uns bestimmt nocheinmal
//          !pFrm->IsMoveable() &&
                 Frm().Left() == (pFrm->Frm().Left()+pFrm->Prt().Left()) &&
                 Frm().Width() > pFrm->Prt().Width() )
            {
                Frm().Width( pFrm->Prt().Width() );
                bValidPrtArea = FALSE;
                bWidthClipped = TRUE;
            }
        }
    }
    Unlock();
}

