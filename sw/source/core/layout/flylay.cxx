/*************************************************************************
 *
 *  $RCSfile: flylay.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2000-10-31 11:01:43 $
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

#include "doc.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "dview.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "flyfrm.hxx"
#include "ftnfrm.hxx"
#include "frmtool.hxx"
#include "frmfmt.hxx"
#include "errhdl.hxx"
#include "hints.hxx"
#include "pam.hxx"
#include "sectfrm.hxx"


#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif

#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#include "ndole.hxx"
#include "tabfrm.hxx"
#include "flyfrms.hxx"

/*************************************************************************
|*
|*  SwFlyFreeFrm::SwFlyFreeFrm(), ~SwFlyFreeFrm()
|*
|*  Ersterstellung      MA 03. Dec. 92
|*  Letzte Aenderung    MA 09. Apr. 99
|*
|*************************************************************************/

SwFlyFreeFrm::SwFlyFreeFrm( SwFlyFrmFmt *pFmt, SwFrm *pAnch ) :
    SwFlyFrm( pFmt, pAnch ),
    pPage( 0 )
{
}

SwFlyFreeFrm::~SwFlyFreeFrm()
{
    //und Tschuess.
    if( GetPage() )
    {
        if( GetFmt()->GetDoc()->IsInDtor() )
        {
            if ( IsFlyAtCntFrm() && GetPage()->GetSortedObjs() )
                GetPage()->GetSortedObjs()->Remove( GetVirtDrawObj() );
        }
        else
        {
            SwRect aTmp( AddSpacesToFrm() );
            SwFlyFreeFrm::NotifyBackground( GetPage(), aTmp, PREP_FLY_LEAVE );
        }
    }
}

/*************************************************************************
|*
|*  SwFlyFreeFrm::NotifyBackground()
|*
|*  Beschreibung        Benachrichtigt den Hintergrund (alle CntntFrms die
|*      gerade ueberlappt werden. Ausserdem wird das Window in einigen
|*      Faellen direkt invalidiert (vor allem dort, wo keine CntntFrms
|*      ueberlappt werden.
|*      Es werden auch die CntntFrms innerhalb von anderen Flys
|*      beruecksichtigt.
|*  Ersterstellung      MA 03. Dec. 92
|*  Letzte Aenderung    MA 26. Aug. 93
|*
|*************************************************************************/

void SwFlyFreeFrm::NotifyBackground( SwPageFrm *pPage,
                                     const SwRect& rRect, PrepareHint eHint )
{
    ::Notify_Background( GetVirtDrawObj(), pPage, rRect, eHint, TRUE );
}

/*************************************************************************
|*
|*  SwFlyFreeFrm::MakeAll()
|*
|*  Ersterstellung      MA 18. Feb. 94
|*  Letzte Aenderung    MA 03. Mar. 97
|*
|*************************************************************************/

void SwFlyFreeFrm::MakeAll()
{
    if ( !GetAnchor() || IsLocked() || IsColLocked() || !GetPage() )
        return;

    Lock(); //Der Vorhang faellt

    //uebernimmt im DTor die Benachrichtigung
    const SwFlyNotify aNotify( this );

    if ( IsClipped() )
        bValidPos = bValidSize = bHeightClipped = bWidthClipped = FALSE;

    while ( !bValidPos || !bValidSize || !bValidPrtArea || bFormatHeightOnly )
    {
        const SwFmtFrmSize *pSz;
        {   //Zusaetzlicher Scope, damit aAccess vor dem Check zerstoert wird!

            SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            pSz = &rAttrs.GetAttrSet().GetFrmSize();

            //Nur einstellen wenn das Flag gesetzt ist!!
            if ( !bValidSize )
            {
                bValidPrtArea = FALSE;
                const long nOldWidth = Frm().Width();
                const Size aTmp( CalcRel( *pSz ) );
                const SwTwips nMin = MINFLY + rAttrs.CalcLeftLine()+rAttrs.CalcRightLine();
                aFrm.Width( Max( aTmp.Width(), nMin ) );
                if ( nOldWidth != Frm().Width() )
                {
                    bValidPos = FALSE;  //Clipping kann notwendig sein.
//MA 03. Mar. 97: Wozu das? Stoert bei #37008#
//                  if ( aFrm.Width() > nOldWidth )
//                      aFrm.Height( aTmp.Height() );
                }
            }

            if ( !bValidPrtArea )
                MakePrtArea( rAttrs );

            if ( !bValidSize || bFormatHeightOnly )
            {
                bValidSize = FALSE;
                Format( &rAttrs );
                bFormatHeightOnly = FALSE;
            }

            if ( !bValidPos )
            {
                const Point aOldPos( Frm().Pos() );
                MakeFlyPos();
                if( aOldPos == Frm().Pos() )
                {
                    if( !bValidPos && GetAnchor()->IsInSct() &&
                        !GetAnchor()->FindSctFrm()->IsValid() )
                        bValidPos = TRUE;
                }
                else
                    bValidSize = FALSE;
            }
        }
        if ( bValidPos && bValidSize )
            CheckClip( *pSz );
    }
    Unlock();

    ASSERT( bHeightClipped || (Frm().Height() > 0 && Prt().Height() > 0),
            "SwFlyFreeFrm::Format(), flipping Fly." );
}

/*************************************************************************
|*
|*  SwFlyFreeFrm::CheckClip()
|*
|*  Ersterstellung      MA 21. Feb. 94
|*  Letzte Aenderung    MA 03. Mar. 97
|*
|*************************************************************************/

void SwFlyFreeFrm::CheckClip( const SwFmtFrmSize &rSz )
{
    //Jetzt ist es ggf. an der Zeit geignete Massnahmen zu ergreifen wenn
    //der Fly nicht in seine Umgebung passt.
    //Zuerst gibt der Fly seine Position auf. Dananch wird er zunaechst
    //Formatiert. Erst wenn er auch durch die Aufgabe der Position nicht
    //passt wird die Breite oder Hoehe aufgegeben - der Rahmen wird soweit
    //wie notwendig zusammengequetscht.

    const SwVirtFlyDrawObj *pObj = GetVirtDrawObj();
    SwRect aClip, aTmpStretch;
    ::CalcClipRect( pObj, aClip, TRUE );
    ::CalcClipRect( pObj, aTmpStretch, FALSE );
    aClip._Intersection( aTmpStretch );

    const long nBot = Frm().Bottom();
    const long nRig = Frm().Right();
    const long nClipBot = aClip.Bottom();
    const long nClipRig = aClip.Right();

    const FASTBOOL bBot = nBot > nClipBot;
    const FASTBOOL bRig = nRig > nClipRig;
    if ( bBot || bRig )
    {
        FASTBOOL bAgain = FALSE;
        if ( bBot )
        {
            SwFrm* pHeader = FindFooterOrHeader();
            // In a header, correction of the position is no good idea.
            // If the fly moves, some paragraphs has to be formatted, this
            // could cause a change of the heigth of the headerframe,
            // now the flyframe can change its position and so on ...
            if( !pHeader || !pHeader->IsHeaderFrm() )
            {
                const long nOld = Frm().Top();
                Frm().Pos().Y() = Max( aClip.Top(),
                    aClip.Bottom() - Frm().Height());
                if ( Frm().Top() != nOld )
                    bAgain = TRUE;
                bHeightClipped = TRUE;
            }
        }
        if ( bRig )
        {
            const long nOld = Frm().Left();
            Frm().Pos().X() = Max( aClip.Left(), aClip.Right() - Frm().Width() );
            if ( Frm().Left() != nOld )
            {
                const SwFmtHoriOrient &rH = GetFmt()->GetHoriOrient();
                // Links ausgerichtete duerfen nicht nach links verschoben werden,
                // wenn sie einem anderen ausweichen.
                if( rH.GetHoriOrient() == HORI_LEFT )
                    Frm().Pos().X() = nOld;
                else
                    bAgain = TRUE;
            }
            bWidthClipped = TRUE;
        }
        if ( bAgain )
            bValidSize = FALSE;
        else
        {
            //Wenn wir hier ankommen ragt der Frm in unerlaubte Bereiche
            //hinein, und eine Positionskorrektur ist nicht erlaubt bzw.
            //moeglich oder noetig.

            //Fuer Flys mit OLE-Objekten als Lower sorgen wir dafuer, dass
            //immer proportional Resized wird.
            Size aOldSize( Frm().SSize() );

            //Zuerst wird das FrmRect eingestellt, und dann auf den Frm
            //uebertragen.
            SwRect aFrmRect( Frm() );

            if ( bBot )
            {
                long nDiff = nClipBot;
                nDiff -= aFrmRect.Top() - 1; //nDiff ist die verfuegbare Strecke.
                nDiff = aFrmRect.Height() - nDiff;
                aFrmRect.Height( aFrmRect.Height() - nDiff );
                bHeightClipped = TRUE;
            }
            if ( bRig )
            {
                long nDiff = nClipRig;
                nDiff -= aFrmRect.Left() - 1;//nDiff ist die verfuegbare Strecke.
                nDiff = aFrmRect.Width() - nDiff;
                aFrmRect.Width( aFrmRect.Width() - nDiff );
                bWidthClipped = TRUE;
            }

            if ( Lower() && Lower()->IsNoTxtFrm() &&
                 ((SwCntntFrm*)Lower())->GetNode()->GetOLENode() )
            {
                //Wenn Breite und Hoehe angepasst wurden, so ist die
                //groessere Veraenderung massgeblich.
                if ( aFrmRect.Width() != aOldSize.Width() &&
                     aFrmRect.Height()!= aOldSize.Height() )
                {
                    if ( (aOldSize.Width() - aFrmRect.Width()) >
                         (aOldSize.Height()- aFrmRect.Height()) )
                        aFrmRect.Height( aOldSize.Height() );
                    else
                        aFrmRect.Width( aOldSize.Width() );
                }

                //Breite angepasst? - Hoehe dann proportional verkleinern
                if( aFrmRect.Width() != aOldSize.Width() )
                    aFrmRect.Height( aFrmRect.Width() * aOldSize.Height() /
                                     aOldSize.Width() );

                //Hoehe angepasst? - Breite dann proportional verkleinern
                if( aFrmRect.Height() != aOldSize.Height() )
                    aFrmRect.Width( aFrmRect.Height() * aOldSize.Width() /
                                    aOldSize.Height() );
            }

            //Jetzt die Einstellungen am Frm vornehmen, bei Spalten werden
            //die neuen Werte in die Attribute eingetragen, weil es sonst
            //ziemlich fiese Oszillationen gibt.
            const long nPrtHeightDiff = Frm().Height() - Prt().Height();
            const long nPrtWidthDiff  = Frm().Width()  - Prt().Width();
            Frm().Height( aFrmRect.Height() );
            Frm().Width ( aFrmRect.Width() );
            if ( Lower() && Lower()->IsColumnFrm() )
            {
                ColLock();  //Grow/Shrink locken.
                const Size aOldSize( Prt().SSize() );
                Prt().Height( Frm().Height() - nPrtHeightDiff );
                Prt().Width ( Frm().Width()  - nPrtWidthDiff );
                ChgLowersProp( aOldSize );
                SwFrm *pLow = Lower();
                do
                {   pLow->Calc();
                    // auch den (Column)BodyFrm mitkalkulieren
                    ((SwLayoutFrm*)pLow)->Lower()->Calc();
                    pLow = pLow->GetNext();
                } while ( pLow );
                ::CalcCntnt( this );
                ColUnlock();
/* MA 02. Sep. 96: Wenn das Attribut gesetzt wird funktionieren Flys in Flys
 * nicht  (30095 30096)
                SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
                pFmt->LockModify();
                SwFmtFrmSize aFrmSize( rSz );
                if ( bRig )
                    aFrmSize.SetWidth( Frm().Width() );
                if ( bBot )
                {
                    aFrmSize.SetSizeType( ATT_FIX_SIZE );
                    aFrmSize.SetHeight( Frm().Height() );
                    bFixHeight = TRUE;
                    bMinHeight = FALSE;
                }
                pFmt->SetAttr( aFrmSize );
                pFmt->UnlockModify();
*/
//Stattdessen:
                if ( !bValidSize && !bWidthClipped )
                    bFormatHeightOnly = bValidSize = TRUE;
            }
            else
            {
                Prt().Height( Frm().Height() - nPrtHeightDiff );
                Prt().Width ( Frm().Width()  - nPrtWidthDiff );
            }
        }
    }
}

/*************************************************************************
|*
|*  SwFlyLayFrm::SwFlyLayFrm()
|*
|*  Ersterstellung      MA 25. Aug. 92
|*  Letzte Aenderung    MA 09. Apr. 99
|*
|*************************************************************************/

SwFlyLayFrm::SwFlyLayFrm( SwFlyFrmFmt *pFmt, SwFrm *pAnch ) :
    SwFlyFreeFrm( pFmt, pAnch )
{
    bLayout = TRUE;
}

/*************************************************************************
|*
|*  SwFlyLayFrm::Modify()
|*
|*  Ersterstellung      MA 08. Feb. 93
|*  Letzte Aenderung    MA 28. Aug. 93
|*
|*************************************************************************/

void SwFlyLayFrm::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
    USHORT nWhich = pNew ? pNew->Which() : 0;

    SwFmtAnchor *pAnch = 0;
    if( RES_ATTRSET_CHG == nWhich && SFX_ITEM_SET ==
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_ANCHOR, FALSE,
            (const SfxPoolItem**)&pAnch ))
        ;       // Beim GetItemState wird der AnkerPointer gesetzt !

    else if( RES_ANCHOR == nWhich )
    {
        //Ankerwechsel, ich haenge mich selbst um.
        //Es darf sich nicht um einen Wechsel des Ankertyps handeln,
        //dies ist nur ueber die SwFEShell moeglich.
        pAnch = (SwFmtAnchor*)pNew;
    }

    if( pAnch )
    {
        ASSERT( pAnch->GetAnchorId() ==
                GetFmt()->GetAnchor().GetAnchorId(),
                "8-) Unzulaessiger Wechsel des Ankertyps." );

        //Abmelden, Seite besorgen, an den entsprechenden LayoutFrm
        //haengen.
        SwRect aOld( AddSpacesToFrm() );
        SwPageFrm *pOldPage = GetPage();
        GetAnchor()->RemoveFly( this );

        if( FLY_PAGE == pAnch->GetAnchorId() )
        {
            USHORT nPgNum = pAnch->GetPageNum();
            SwRootFrm *pRoot = FindRootFrm();
            SwPageFrm *pPage = (SwPageFrm*)pRoot->Lower();
            for ( USHORT i = 1; (i <= nPgNum) && pPage; ++i,
                                pPage = (SwPageFrm*)pPage->GetNext() )
            {
                if ( i == nPgNum )
                    pPage->PlaceFly( this, 0, pAnch );
            }
            if( !pPage )
            {
                pRoot->SetAssertFlyPages();
                pRoot->AssertFlyPages();
            }
        }
        else
        {
            SwNodeIndex aIdx( pAnch->GetCntntAnchor()->nNode );
            SwCntntFrm *pCntnt = GetFmt()->GetDoc()->GetNodes().GoNext( &aIdx )->
                         GetCntntNode()->GetFrm( 0, 0, FALSE );
            if( pCntnt )
            {
                SwFlyFrm *pTmp = pCntnt->FindFlyFrm();
                if( pTmp )
                    pTmp->AppendFly( this );
            }
        }
        if ( pOldPage && pOldPage != GetPage() )
            NotifyBackground( pOldPage, aOld, PREP_FLY_LEAVE );
        SetCompletePaint();
        InvalidateAll();
        SetNotifyBack();
    }
    else
        SwFlyFrm::Modify( pOld, pNew );
}

/*************************************************************************
|*
|*  SwPageFrm::AppendFly()
|*
|*  Ersterstellung      MA 10. Oct. 92
|*  Letzte Aenderung    MA 08. Jun. 96
|*
|*************************************************************************/

void SwPageFrm::AppendFly( SwFlyFrm *pNew )
{
    if ( !pNew->GetVirtDrawObj()->IsInserted() )
        FindRootFrm()->GetDrawPage()->InsertObject(
                (SdrObject*)pNew->GetVirtDrawObj(),
                pNew->GetVirtDrawObj()->GetReferencedObj().GetOrdNumDirect() );

    InvalidateSpelling();
    InvalidateAutoCompleteWords();

    if ( GetUpper() )
    {
        ((SwRootFrm*)GetUpper())->SetIdleFlags();
        ((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
    }

    const SdrObjectPtr pObj = pNew->GetVirtDrawObj();
    ASSERT( pNew->GetAnchor(), "Fly without Anchor" );
    SwFlyFrm *pFly = pNew->GetAnchor()->FindFlyFrm();
    if ( pFly && pObj->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() )
    {
        UINT32 nNewNum = pFly->GetVirtDrawObj()->GetOrdNumDirect() + 1;
        if ( pObj->GetPage() )
            pObj->GetPage()->SetObjectOrdNum( pObj->GetOrdNumDirect(), nNewNum);
        else
            pObj->SetOrdNum( nNewNum );
    }

    //Flys die im Cntnt sitzen beachten wir nicht weiter.
    if ( pNew->IsFlyInCntFrm() )
    {
        InvalidateFlyInCnt();
        return;
    }
    InvalidateFlyCntnt();

    if ( !pSortedObjs )
        pSortedObjs = new SwSortDrawObjs();
    if ( !pSortedObjs->Insert( pObj ) )
        ASSERT( FALSE, "Fly nicht in Sorted eingetragen." );

    ((SwFlyFreeFrm*)pNew)->SetPage( this );
    pNew->InvalidatePage( this );


    //fix(3018): Kein pNew->Calc() oder sonstiges hier.
    //Code enfernt in flylay.cxx Rev 1.51
}

/*************************************************************************
|*
|*  SwPageFrm::RemoveFly()
|*
|*  Ersterstellung      MA 10. Oct. 92
|*  Letzte Aenderung    MA 26. Aug. 96
|*
|*************************************************************************/

void SwPageFrm::RemoveFly( SwFlyFrm *pToRemove )
{
    const UINT32 nOrdNum = pToRemove->GetVirtDrawObj()->GetOrdNum();
    FindRootFrm()->GetDrawPage()->RemoveObject( nOrdNum );
    pToRemove->GetVirtDrawObj()->ReferencedObj().SetOrdNum( nOrdNum );

    if ( GetUpper() )
    {
        if ( !pToRemove->IsFlyInCntFrm() )
            ((SwRootFrm*)GetUpper())->SetSuperfluous();
        ((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
    }

    //Flys die im Cntnt sitzen beachten wir nicht weiter.
    if ( pToRemove->IsFlyInCntFrm() )
        return;

    //Collections noch nicht loeschen. Das passiert am Ende
    //der Action im RemoveSuperfluous der Seite - angestossen von gleich-
    //namiger Methode der Root.
    //Die FlyColl kann bereits weg sein, weil der DTor der Seite
    //gerade 'laeuft'
    if ( pSortedObjs )
    {
        pSortedObjs->Remove( pToRemove->GetVirtDrawObj() );
        if ( !pSortedObjs->Count() )
        {   DELETEZ( pSortedObjs );
        }
    }
    ((SwFlyFreeFrm*)pToRemove)->SetPage( 0 );
}

/*************************************************************************
|*
|*  SwPageFrm::MoveFly
|*
|*  Ersterstellung      MA 25. Jan. 97
|*  Letzte Aenderung    MA 25. Jan. 97
|*
|*************************************************************************/

void SwPageFrm::MoveFly( SwFlyFrm *pToMove, SwPageFrm *pDest )
{
    //Invalidierungen
    if ( GetUpper() )
    {
        ((SwRootFrm*)GetUpper())->SetIdleFlags();
        if ( !pToMove->IsFlyInCntFrm() && pDest->GetPhyPageNum() < GetPhyPageNum() )
            ((SwRootFrm*)GetUpper())->SetSuperfluous();
    }
    pDest->InvalidateSpelling();
    pDest->InvalidateAutoCompleteWords();

    if ( pToMove->IsFlyInCntFrm() )
    {
        pDest->InvalidateFlyInCnt();
        return;
    }

    //Die FlyColl kann bereits weg sein, weil der DTor der Seite
    //gerade 'laeuft'
    const SdrObjectPtr pObj = pToMove->GetVirtDrawObj();
    if ( pSortedObjs )
    {
        pSortedObjs->Remove( pObj );
        if ( !pSortedObjs->Count() )
        {   DELETEZ( pSortedObjs );
        }
    }

    //Anmelden
    if ( !pDest->GetSortedObjs() )
        pDest->pSortedObjs = new SwSortDrawObjs();
    if ( !pDest->GetSortedObjs()->Insert( pObj ) )
        ASSERT( FALSE, "Fly nicht in Sorted eingetragen." );

    ((SwFlyFreeFrm*)pToMove)->SetPage( pDest );
    pToMove->InvalidatePage( pDest );
    pToMove->SetNotifyBack();
    pDest->InvalidateFlyCntnt();
}

/*************************************************************************
|*
|*  SwPageFrm::AppendDrawObject(), RemoveDrawObject()
|*
|*  Ersterstellung      MA 09. Jan. 95
|*  Letzte Aenderung    MA 31. Jul. 96
|*
|*************************************************************************/

void SwPageFrm::AppendDrawObj( SwDrawContact *pNew )
{
    if ( GetUpper() )
        ((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();

    const SdrObjectPtr pObj = pNew->GetMaster();
    ASSERT( pNew->GetAnchor(), "Contact without Anchor" );
    SwFlyFrm *pFly = pNew->GetAnchor()->FindFlyFrm();
    if ( pFly && pObj->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() )
    {
        UINT32 nNewNum = pFly->GetVirtDrawObj()->GetOrdNumDirect() + 1;
        if ( pObj->GetPage() )
            pObj->GetPage()->SetObjectOrdNum( pObj->GetOrdNumDirect(), nNewNum);
        else
            pObj->SetOrdNum( nNewNum );
    }

    if ( FLY_IN_CNTNT == pNew->GetFmt()->GetAnchor().GetAnchorId() )
        return;

    if ( !pSortedObjs )
        pSortedObjs = new SwSortDrawObjs();
    if ( !pSortedObjs->Insert( pObj ) )
    {
#ifndef PRODUCT
        USHORT nIdx;
        ASSERT( pSortedObjs->Seek_Entry( pObj, &nIdx ),
                "Fly nicht in Sorted eingetragen." );
#endif
    }
    pNew->ChgPage( this );
}

void SwPageFrm::RemoveDrawObj( SwDrawContact *pToRemove )
{
    //Auch Zeichengebundene muessen hier leider durchlaufen, weil beim
    //setzen des Attributes zuerst das Attribut gesetzt und dann das Modify
    //versendet wird.

    //Die FlyColl kann bereits weg sein, weil der DTor der Seite
    //gerade 'laeuft'
    if ( pSortedObjs )
    {
        const SdrObjectPtr *pDel = pSortedObjs->GetData();
        pSortedObjs->Remove( pToRemove->GetMaster() );
        if ( !pSortedObjs->Count() )
        {
            DELETEZ( pSortedObjs );
        }
        if ( GetUpper() )
        {
            if ( FLY_IN_CNTNT != pToRemove->GetFmt()->GetAnchor().GetAnchorId() )
            {
                ((SwRootFrm*)GetUpper())->SetSuperfluous();
                InvalidatePage();
            }
            ((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
        }
    }
    pToRemove->ChgPage( 0 );
}

/*************************************************************************
|*
|*  SwPageFrm::PlaceFly
|*
|*  Ersterstellung      MA 08. Feb. 93
|*  Letzte Aenderung    MA 27. Feb. 93
|*
|*************************************************************************/

SwFrm *SwPageFrm::PlaceFly( SwFlyFrm *pFly, SwFrmFmt *pFmt,
                            const SwFmtAnchor *pAnch )
{
    //Der Fly will immer an der Seite direkt haengen.
    ASSERT( pAnch->GetAnchorId() == FLY_PAGE, "Unerwartete AnchorId." );

    //Wenn ein Fly uebergeben wurde, so benutzen wir diesen, ansonsten wird
    //mit dem Format einer erzeugt.
    if ( pFly )
        SwFrm::AppendFly( pFly );
    else
    {   ASSERT( pFmt, ":-( kein Format fuer Fly uebergeben." );
        pFly = new SwFlyLayFrm( (SwFlyFrmFmt*)pFmt, this );
        SwFrm::AppendFly( pFly );
        ::RegistFlys( this, pFly );
    }
    return pFly;
}

/*************************************************************************
|*
|*  ::CalcClipRect
|*
|*  Ersterstellung      AMA 24. Sep. 96
|*  Letzte Aenderung    MA  18. Dec. 96
|*
|*************************************************************************/

BOOL CalcClipRect( const SdrObject *pSdrObj, SwRect &rRect, BOOL bMove )
{
    BOOL bRet = TRUE;
    if ( pSdrObj->IsWriterFlyFrame() )
    {
        const SwFlyFrm *pFly = ((const SwVirtFlyDrawObj*)pSdrObj)->GetFlyFrm();
        if( pFly->IsFlyLayFrm() )
        {
#ifdef AMA_OUT_OF_FLY
            const SwFrm *pClip = pFly->GetAnchor()->FindPageFrm();
#else
            const SwFrm *pClip = pFly->GetAnchor();
#endif
            pClip->Calc();

            rRect = pClip->Frm();

            //Vertikales clipping: Top und Bottom, ggf. an PrtArea
            const SwFmtVertOrient &rV = pFly->GetFmt()->GetVertOrient();
            if( rV.GetVertOrient() != VERT_NONE &&
                rV.GetRelationOrient() == PRTAREA )
            {
                rRect.Top( pClip->Frm().Top() + pClip->Prt().Top() );
                rRect.Bottom( pClip->Frm().Top() + pClip->Prt().Bottom() );
            }
            //Horizontales clipping: Left und Right, ggf. an PrtArea
            const SwFmtHoriOrient &rH = pFly->GetFmt()->GetHoriOrient();
            if( rH.GetHoriOrient() != HORI_NONE &&
                rH.GetRelationOrient() == PRTAREA )
            {
                rRect.Left( pClip->Frm().Left() + pClip->Prt().Left() );
                rRect.Right( pClip->Frm().Left() + pClip->Prt().Right() );
            }
        }
        else if( pFly->IsFlyAtCntFrm() )
        {
            const SwFrm *pClip = pFly->GetAnchor();
            const SwLayoutFrm *pUp = pClip->GetUpper();
            const SwFrm *pCell = pUp->IsCellFrm() ? pUp : 0;
            USHORT nType = bMove ? FRM_ROOT   | FRM_FLY | FRM_HEADER |
                                   FRM_FOOTER | FRM_FTN
                                 : FRM_BODY   | FRM_FLY | FRM_HEADER |
                                   FRM_FOOTER | FRM_CELL| FRM_FTN;

            while ( !(pUp->GetType() & nType) || pUp->IsColBodyFrm() )
            {
                pUp = pUp->GetUpper();
                if ( !pCell && pUp->IsCellFrm() )
                    pCell = pUp;
            }
            if ( bMove )
            {
                if ( pUp->IsRootFrm() )
                {
                    rRect  = pUp->Prt();
                    rRect += pUp->Frm().Pos();

                    const SwPageFrm *pPg = (SwPageFrm*)pUp->Lower();
                    if( !pPg->Lower() )
                        pPg = (SwPageFrm*)pPg->GetNext();
                    const SwFrm *pBody = pPg->FindBodyCont();
                    if ( pBody->GetPrev() )
                        pBody->GetPrev()->Calc();
                    pBody->Calc();
                    rRect.Top( pBody->Frm().Top() + pBody->Prt().Top() );

                    //Den Bottom setzen wir auf den unteren Rand der letzten Seite.
                    pPg = ((SwRootFrm*)pUp)->GetLastPage();
                    if ( !pPg->Lower() )
                        pPg = (SwPageFrm*)pPg->GetPrev();
                    if ( pPg )
                    {
                        pPg->Calc();
                        pBody = pPg->FindBodyCont();
                        if ( pBody->GetPrev() )
                            pBody->GetPrev()->Calc();
                        pBody->Calc();
                        rRect.Bottom( pBody->Frm().Top() + pBody->Prt().Bottom());
                        if( pFly->GetFmt()->GetDoc()->IsBrowseMode() )
                        {
                            // Hier wird folgende Situation abgefangen: Im Browse-
                            // Modus hat man mehrere Seiten, die aktuelle Seite
                            // waechst gerade, die letzte Seite hat dies noch nicht
                            // mitbekommen, dann koennte der von der letzten Seite
                            // gelieferte nBot zu klein sein, der Fly sich also
                            // zum Clippen/Moven genoetigt sehen, obgleich genug
                            // Platz vorhanden ist.
                            const SwPageFrm* pMyPg = pClip->FindPageFrm();
                            if( pMyPg != pPg && pMyPg &&
                                pMyPg->Frm().Bottom() > pPg->Frm().Top() )
                            {
                                pBody = pMyPg->FindBodyCont();
                                if ( pBody->GetPrev() )
                                    pBody->GetPrev()->Calc();
                                pBody->Calc();
                                SwTwips nBot = pBody->Frm().Top() + pBody->Prt().Bottom();
                                if( rRect.Bottom() < nBot )
                                    rRect.Bottom( nBot );
                            }
                        }
                    }
                    pUp = 0;
                }
            }
            if ( pUp )
            {
                if ( !pUp->IsFlyFrm() ||
                     (!pUp->Lower() || !pUp->Lower()->IsColumnFrm()) )
                    pUp->Calc();
                if ( pUp->GetType() & FRM_BODY )
                {
                    const SwPageFrm *pPg;
                    if ( pUp->GetUpper() != (pPg = pFly->FindPageFrm()) )
                        pUp = pPg->FindBodyCont();
                    rRect = pUp->GetUpper()->Frm();
                    rRect.Pos().Y() = pUp->Frm().Top() + pUp->Prt().Top();
                    rRect.SSize().Height() = pUp->Prt().Height();
                }
                else
                {
                    if( ( pUp->GetType() & (FRM_FLY | FRM_FTN ) ) &&
                        !pUp->Frm().IsInside( pFly->Frm().Pos() ) )
                    {
                        if( pUp->IsFlyFrm() )
                        {
                            SwFlyFrm *pTmpFly = (SwFlyFrm*)pUp;
                            while( pTmpFly->GetNextLink() )
                            {
                                pTmpFly = pTmpFly->GetNextLink();
                                if( pTmpFly->Frm().IsInside( pFly->Frm().Pos() ) )
                                    break;
                            }
                            pUp = pTmpFly;
                        }
                        else if( pUp->IsInFtn() )
                        {
                            const SwFtnFrm *pTmp = pUp->FindFtnFrm();
                            while( pTmp->GetFollow() )
                            {
                                pTmp = pTmp->GetFollow();
                                if( pTmp->Frm().IsInside( pFly->Frm().Pos() ) )
                                    break;
                            }
                            pUp = pTmp;
                        }
                    }
                    rRect = pUp->Prt();
                    rRect.Pos() += pUp->Frm().Pos();
                    if ( pUp->GetType() & (FRM_HEADER | FRM_FOOTER) )
                    {
                        rRect.Left ( pUp->GetUpper()->Frm().Left() );
                        rRect.Width( pUp->GetUpper()->Frm().Width());
                    }
                    else if ( pUp->IsCellFrm() )                //MA_FLY_HEIGHT
                    {
                        const SwFrm *pTab = pUp->FindTabFrm();
                        long nBottom = pTab->GetUpper()->Frm().Top() +
                                       pTab->GetUpper()->Prt().Bottom();
                        rRect.Bottom( nBottom );
                    }
                }
            }
            if ( pCell )
            {
                //CellFrms koennen auch in 'unerlaubten' Bereichen stehen, dann
                //darf der Fly das auch.
                SwRect aTmp( pCell->Prt() );
                aTmp += pCell->Frm().Pos();
                rRect.Union( aTmp );
            }
        }
        else
        {
            if( bMove )
            {
                const SwFmt *pFmt = ((SwContact*)GetUserCall(pSdrObj))
                                        ->GetFmt();
                const SvxULSpaceItem &rUL = pFmt->GetULSpace();
                const SwRect &rTmp = pFly->Frm();
                long nTmpH = rUL.GetLower() + rUL.GetUpper() + rTmp.Height();
                rRect.Width( rTmp.Width() );
                rRect.Height( rTmp.Height() + nTmpH - 1 );
                rRect.Pos( ((SwFlyInCntFrm*)pFly)->GetRefPoint().X(),
                    ((SwFlyInCntFrm*)pFly)->GetRefPoint().Y() - nTmpH + 1 );
            }
            else
            {
                //Sizeable ist er bis zur Grosse der PrtArea
                // des Uppers seines Ankers.
                const SwFrm *pUp = pFly->GetAnchor()->GetUpper();
                pUp->Calc();
                rRect  = pUp->Prt();
                rRect += pUp->Frm().Pos();
            }
        }
    }
    else
    {
        const SwDrawContact *pC = (const SwDrawContact*)GetUserCall(pSdrObj);
        const SwFrmFmt  *pFmt = (const SwFrmFmt*)pC->GetFmt();
        const SwFmtAnchor &rAnch = pFmt->GetAnchor();
        if ( FLY_IN_CNTNT == rAnch.GetAnchorId() )
        {
            if( bMove )
            {
                //Verschiebbar ist das Teil nur vertikal um seinen Ankerpunkt herum.
                SwRect aSnapRect( pSdrObj->GetSnapRect() );
                const SvxULSpaceItem &rUL = pFmt->GetULSpace();
                long nTmpH = pSdrObj->GetBoundRect().GetHeight()+rUL.GetLower()+rUL.GetUpper();
                rRect.Width( aSnapRect.Width() );
                rRect.Height( aSnapRect.Height() + nTmpH - 1 );
                rRect.Pos( pSdrObj->GetAnchorPos().X(),
                           pSdrObj->GetAnchorPos().Y() - nTmpH + 1 );
            }
            else
            {
                //Sizeable ist er bis zur Grosse der PrtArea des Uppers
                //seines Ankers.
                const SwFrm *pFrm = pC->GetAnchor();
                if( !pFrm )
                {
                    ((SwDrawContact*)pC)->ConnectToLayout();
                    pFrm = pC->GetAnchor();
                }
                const SwFrm *pUp = pFrm->GetUpper();
                pUp->Calc();
                rRect = pUp->Prt();
                rRect += pUp->Frm().Pos();
            }
        }
        else
            bRet = FALSE;
    }
    return bRet;
}


