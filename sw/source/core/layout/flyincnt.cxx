/*************************************************************************
 *
 *  $RCSfile: flyincnt.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:09:20 $
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
// OD 2004-01-19 #110582#
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
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
    SwTwips nRel = pFmt->GetVertOrient().GetPos();
    // OD 2004-05-27 #i26791# - member <aRelPos> moved to <SwAnchoredObject>
    Point aRelPos;
    if( pAnch && pAnch->IsVertical() )
        aRelPos.X() = pAnch->IsReverse() ? nRel : -nRel;
    else
        aRelPos.Y() = nRel;
    SetCurrRelPos( aRelPos );
}

SwFlyInCntFrm::~SwFlyInCntFrm()
{
    //und Tschuess.
    if ( !GetFmt()->GetDoc()->IsInDtor() && GetAnchorFrm() )
    {
        SwRect aTmp( GetObjRectWithSpaces() );
        SwFlyInCntFrm::NotifyBackground( FindPageFrm(), aTmp, PREP_FLY_LEAVE );
    }
}

// --> OD 2004-06-29 #i28701#
TYPEINIT1(SwFlyInCntFrm,SwFlyFrm);
// <--
/*************************************************************************
|*
|*  SwFlyInCntFrm::SetRefPoint(),
|*
|*  Ersterstellung      MA 01. Dec. 92
|*  Letzte Aenderung    MA 06. Aug. 95
|*
|*************************************************************************/
void SwFlyInCntFrm::SetRefPoint( const Point& rPoint,
                                 const Point& rRelAttr,
                                 const Point& rRelPos )
{
    // OD 2004-05-27 #i26791# - member <aRelPos> moved to <SwAnchoredObject>
    ASSERT( rPoint != aRef || rRelAttr != GetCurrRelPos(), "SetRefPoint: no change" );
    SwFlyNotify *pNotify = NULL;
    // No notify at a locked fly frame, if a fly frame is locked, there's
    // already a SwFlyNotify object on the stack (MakeAll).
    if( !IsLocked() )
        pNotify = new SwFlyNotify( this );
    aRef = rPoint;
    SetCurrRelPos( rRelAttr );
    SWRECTFN( GetAnchorFrm() )
    (Frm().*fnRect->fnSetPos)( rPoint + rRelPos );
    if( pNotify )
    {
        InvalidatePage();
        bValidPos = FALSE;
        bInvalid  = TRUE;
        Calc();
        delete pNotify;
    }
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

    if ( bCallPrepare && GetAnchorFrm() )
        AnchorFrm()->Prepare( PREP_FLY_ATTR_CHG, GetFmt() );
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
// OD 2004-03-23 #i26791#
//void SwFlyInCntFrm::MakeFlyPos()
void SwFlyInCntFrm::MakeObjPos()
{
    if ( !bValidPos )
    {
        if ( !GetAnchorFrm()->IsTxtFrm() || !((SwTxtFrm*)GetAnchorFrm())->IsLocked() )
            ::DeepCalc( GetAnchorFrm() );
        if( GetAnchorFrm()->IsTxtFrm() )
            ((SwTxtFrm*)GetAnchorFrm())->GetFormatted();
        bValidPos = TRUE;
        SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
        const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
        //Und ggf. noch die aktuellen Werte im Format updaten, dabei darf
        //zu diesem Zeitpunkt natuerlich kein Modify verschickt werden.
        SWRECTFN( GetAnchorFrm() )
        SwTwips nOld = rVert.GetPos();
        SwTwips nAct = bVert ? -GetCurrRelPos().X() : GetCurrRelPos().Y();
        if( bRev )
            nAct = -nAct;
        if( nAct != nOld )
        {
            SwFmtVertOrient aVert( rVert );
            aVert.SetPos( nAct );
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
        AnchorFrm()->Prepare( PREP_FLY_ATTR_CHG );
    else
        AnchorFrm()->Prepare( eHint, (void*)&rRect );
}

/*************************************************************************
|*
|*  SwFlyInCntFrm::GetRelPos()
|*
|*  Ersterstellung      MA 04. Dec. 92
|*  Letzte Aenderung    MA 04. Dec. 92
|*
|*************************************************************************/
const Point SwFlyInCntFrm::GetRelPos() const
{
    Calc();
    return GetCurrRelPos();
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
    // OD 2004-01-19 #110582#
    if ( !GetFmt()->GetDoc()->IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) )
    {
        return;
    }

    if ( !GetAnchorFrm() || IsLocked() || IsColLocked() || !FindPageFrm() )
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
/*
            // This is also done in the Format function, so I think
            // this code is not necessary anymore:
            long nOldWidth = aFrm.Width();
            const Size aRelSize( CalcRel( rFrmSz ) );
            aFrm.Width( aRelSize.Width() );

            if ( aFrm.Width() > nOldWidth )
                //Damit sich der Inhalt anpasst
                aFrm.Height( aRelSize.Height() );
*/
        }

        if ( !bValidPrtArea )
            MakePrtArea( rAttrs );

        if ( !bValidSize )
            Format( &rAttrs );

        if ( !bValidPos )
        {
            // OD 2004-03-23 #i26791#
            //MakeFlyPos();
            MakeObjPos();
        }

/* #111909# objects anchored as characters may exceed right margin!
        if ( bValidPos && bValidSize )
        {
            SwFrm* pFrm = AnchorFrm();
            if ( Frm().Left() == (pFrm->Frm().Left()+pFrm->Prt().Left()) &&
                 Frm().Width() > pFrm->Prt().Width() )
            {
                Frm().Width( pFrm->Prt().Width() );
                bValidPrtArea = FALSE;
                bWidthClipped = TRUE;
            }
        }
 */
    }
    Unlock();
}
