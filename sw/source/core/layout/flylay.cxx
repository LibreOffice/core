/*************************************************************************
 *
 *  $RCSfile: flylay.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 11:47:26 $
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

#ifdef ACCESSIBLE_LAYOUT
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#endif

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
    if ( !GetAnchor() || IsLocked() || IsColLocked() )
        return;
    if( !GetPage() && GetAnchor() && GetAnchor()->IsInFly() )
    {
        SwFlyFrm* pFly = GetAnchor()->FindFlyFrm();
        SwPageFrm *pPage = pFly ? pFly->FindPageFrm() : NULL;
        if( pPage )
            pPage->SwPageFrm::AppendFly( this );
    }
    if( !GetPage() )
        return;

    Lock(); //Der Vorhang faellt

    //uebernimmt im DTor die Benachrichtigung
    const SwFlyNotify aNotify( this );

    if ( IsClipped() )
        bValidPos = bValidSize = bHeightClipped = bWidthClipped = FALSE;

    while ( !bValidPos || !bValidSize || !bValidPrtArea || bFormatHeightOnly )
    {
#ifdef VERTICAL_LAYOUT
       SWRECTFN( this )
#endif
        const SwFmtFrmSize *pSz;
        {   //Zusaetzlicher Scope, damit aAccess vor dem Check zerstoert wird!

            SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            pSz = &rAttrs.GetAttrSet().GetFrmSize();

            //Nur einstellen wenn das Flag gesetzt ist!!
            if ( !bValidSize )
            {
                bValidPrtArea = FALSE;
                const Size aTmp( CalcRel( *pSz ) );
                const SwTwips nMin = MINFLY + rAttrs.CalcLeftLine()+rAttrs.CalcRightLine();
                long nDiff = bVert ? aTmp.Height() : aTmp.Width();
                if( nDiff < nMin )
                    nDiff = nMin;
                nDiff -= (aFrm.*fnRect->fnGetWidth)();
                if( nDiff )
                {
                    (aFrm.*fnRect->fnAddRight)( nDiff );
                    bValidPos = FALSE;
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
#ifdef VERTICAL_LAYOUT
                const Point aOldPos( (Frm().*fnRect->fnGetPos)() );
                MakeFlyPos();
                if( aOldPos == (Frm().*fnRect->fnGetPos)() )
#else
                const Point aOldPos( Frm().Pos() );
                MakeFlyPos();
                if( aOldPos == Frm().Pos() )
#endif
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

#ifdef VERTICAL_LAYOUT
#ifndef PRODUCT
    SWRECTFN( this )
    ASSERT( bHeightClipped || ( (Frm().*fnRect->fnGetHeight)() > 0 &&
            (Prt().*fnRect->fnGetHeight)() > 0),
            "SwFlyFreeFrm::Format(), flipping Fly." );

#endif
#else
    ASSERT( bHeightClipped || (Frm().Height() > 0 && Prt().Height() > 0),
            "SwFlyFreeFrm::Format(), flipping Fly." );
#endif
}

/** determines, if direct environment of fly frame has 'auto' size

    OD 07.08.2003 #i17297#, #111066#, #111070#
    start with anchor frame and search via <GetUpper()> for a header, footer,
    row or fly frame stopping at page frame.
    return <true>, if such a frame is found and it has 'auto' size.
    otherwise <false> is returned.

    @author OD

    @return boolean indicating, that direct environment has 'auto' size
*/
bool SwFlyFreeFrm::HasEnvironmentAutoSize() const
{
    bool bRetVal = false;

    const SwFrm* pToBeCheckedFrm = GetAnchor();
    while ( pToBeCheckedFrm &&
            !pToBeCheckedFrm->IsPageFrm() )
    {
        if ( pToBeCheckedFrm->IsHeaderFrm() ||
             pToBeCheckedFrm->IsFooterFrm() ||
             pToBeCheckedFrm->IsRowFrm() ||
             pToBeCheckedFrm->IsFlyFrm() )
        {
            bRetVal = ATT_FIX_SIZE !=
                      pToBeCheckedFrm->GetAttrSet()->GetFrmSize().GetSizeType();
            break;
        }
        else
        {
            pToBeCheckedFrm = pToBeCheckedFrm->GetUpper();
        }
    }

    return bRetVal;
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
    //Zuerst gibt der Fly seine Position auf. Danach wird er zunaechst
    //formatiert. Erst wenn er auch durch die Aufgabe der Position nicht
    //passt wird die Breite oder Hoehe aufgegeben - der Rahmen wird soweit
    //wie notwendig zusammengequetscht.

    const SwVirtFlyDrawObj *pObj = GetVirtDrawObj();
    SwRect aClip, aTmpStretch;
    ::CalcClipRect( pObj, aClip, TRUE );
    ::CalcClipRect( pObj, aTmpStretch, FALSE );
    aClip._Intersection( aTmpStretch );

    const long nBot = Frm().Top() + Frm().Height();
    const long nRig = Frm().Left() + Frm().Width();
    const long nClipBot = aClip.Top() + aClip.Height();
    const long nClipRig = aClip.Left() + aClip.Width();

    const FASTBOOL bBot = nBot > nClipBot;
    const FASTBOOL bRig = nRig > nClipRig;
    if ( bBot || bRig )
    {
        FASTBOOL bAgain = FALSE;
        if ( bBot && !GetDrawObjs() && !GetAnchor()->IsInTab() )
        {
            SwFrm* pHeader = FindFooterOrHeader();
            // In a header, correction of the position is no good idea.
            // If the fly moves, some paragraphs has to be formatted, this
            // could cause a change of the height of the headerframe,
            // now the flyframe can change its position and so on ...
            if( !pHeader || !pHeader->IsHeaderFrm() )
            {
                const long nOld = Frm().Top();
                Frm().Pos().Y() = Max( aClip.Top(), nClipBot - Frm().Height() );
                if ( Frm().Top() != nOld )
                    bAgain = TRUE;
                bHeightClipped = TRUE;
            }
        }
        if ( bRig )
        {
            const long nOld = Frm().Left();
            Frm().Pos().X() = Max( aClip.Left(), nClipRig - Frm().Width() );
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
                nDiff -= aFrmRect.Top(); //nDiff ist die verfuegbare Strecke.
                nDiff = aFrmRect.Height() - nDiff;
                aFrmRect.Height( aFrmRect.Height() - nDiff );
                bHeightClipped = TRUE;
            }
            if ( bRig )
            {
                long nDiff = nClipRig;
                nDiff -= aFrmRect.Left();//nDiff ist die verfuegbare Strecke.
                nDiff = aFrmRect.Width() - nDiff;
                aFrmRect.Width( aFrmRect.Width() - nDiff );
                bWidthClipped = TRUE;
            }

            // OD 06.08.2003 #i17297#, #111066#, #111070# - no proportional
            // scaling of graphics in environments, which determines its size
            // by its content ('auto' size). Otherwise layout loops can occur and
            // layout sizes of the environment can be incorrect.
            // Such environment are:
            // (1) header and footer frames with 'auto' size
            // (2) table row frames with 'auto' size
            // (3) fly frames with 'auto' size
            // Note: section frames seems to be not critical - didn't found
            //       any critical layout situation so far.
            if ( Lower() && Lower()->IsNoTxtFrm() &&
                 ( static_cast<SwCntntFrm*>(Lower())->GetNode()->GetOLENode() ||
                   !HasEnvironmentAutoSize() ) )
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
                {
                    aFrmRect.Height( aFrmRect.Width() * aOldSize.Height() /
                                     aOldSize.Width() );
                    bHeightClipped = TRUE;
                }
                //Hoehe angepasst? - Breite dann proportional verkleinern
                else if( aFrmRect.Height() != aOldSize.Height() )
                {
                    aFrmRect.Width( aFrmRect.Height() * aOldSize.Width() /
                                    aOldSize.Height() );
                    bWidthClipped = TRUE;
                }

                // OD 07.08.2003 #i17297#, #111066#, #111070# - reactivate change
                // of size attribute for fly frames containing an ole object.
                if ( static_cast<SwCntntFrm*>(Lower())->GetNode()->GetOLENode() &&
                     ( bWidthClipped || bHeightClipped ) )
                {
                    SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
                    pFmt->LockModify();
                    SwFmtFrmSize aFrmSize( rSz );
                    aFrmSize.SetWidth( aFrmRect.Width() );
                    aFrmSize.SetHeight( aFrmRect.Height() );
                    pFmt->SetAttr( aFrmSize );
                    pFmt->UnlockModify();
                }
            }

            //Jetzt die Einstellungen am Frm vornehmen, bei Spalten werden
            //die neuen Werte in die Attribute eingetragen, weil es sonst
            //ziemlich fiese Oszillationen gibt.
            const long nPrtHeightDiff = Frm().Height() - Prt().Height();
            const long nPrtWidthDiff  = Frm().Width()  - Prt().Width();
            Frm().Height( aFrmRect.Height() );
            Frm().Width ( Max( long(MINLAY), aFrmRect.Width() ) );
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
        InvalidateFlyInCnt();
    else
    {
        InvalidateFlyCntnt();

        if ( !pSortedObjs )
            pSortedObjs = new SwSortDrawObjs();
        if ( !pSortedObjs->Insert( pObj ) )
            ASSERT( FALSE, "Fly nicht in Sorted eingetragen." );

        ((SwFlyFreeFrm*)pNew)->SetPage( this );
        pNew->InvalidatePage( this );

#ifdef ACCESSIBLE_LAYOUT
        // Notify accessible layout. That's required at this place for
        // frames only where the anchor is moved. Creation of new frames
        // is additionally handled by the SwFrmNotify class.
        if( GetUpper() &&
            static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
             static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
        {
            static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                      ->AddAccessibleFrm( pNew );
        }
#endif

    }

    if( pNew->GetDrawObjs() )
    {
        SwDrawObjs &rObjs = *pNew->GetDrawObjs();
        for ( USHORT i = 0; i < rObjs.Count(); ++i )
        {
            SdrObject *pO = rObjs[i];
            if( pO->IsWriterFlyFrame() )
            {
                SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                if( pFly->IsFlyFreeFrm() && !((SwFlyFreeFrm*)pFly)->GetPage() )
                    SwPageFrm::AppendFly( pFly );
            }
        }
    }

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

#ifdef ACCESSIBLE_LAYOUT
    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrmNotify class.
    if( GetUpper() &&
        static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->DisposeAccessibleFrm( pToRemove, sal_True );
    }
#endif

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

#ifdef ACCESSIBLE_LAYOUT
    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrmNotify class.
    if( GetUpper() &&
        static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->DisposeAccessibleFrm( pToMove, sal_True );
    }
#endif

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

#ifdef ACCESSIBLE_LAYOUT
    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrmNotify class.
    if( GetUpper() &&
        static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->AddAccessibleFrm( pToMove );
    }
#endif
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

// OD 20.05.2003 #108784# - adding 'virtual' drawing object to page frame
void SwPageFrm::AppendVirtDrawObj( SwDrawContact* _pDrawContact,
                                   SwDrawVirtObj* _pDrawVirtObj )
{
    if ( GetUpper() )
    {
        ((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
    }

    ASSERT( _pDrawVirtObj->GetAnchorFrm(), "virtual draw contact without anchor" );
    SwFlyFrm *pFly = _pDrawVirtObj->GetAnchorFrm()->FindFlyFrm();
    if ( pFly && _pDrawVirtObj->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() )
    {
        UINT32 nNewNum = pFly->GetVirtDrawObj()->GetOrdNumDirect() + 1;
        if ( _pDrawVirtObj->GetPage() )
            _pDrawVirtObj->GetPage()->SetObjectOrdNum( _pDrawVirtObj->GetOrdNumDirect(), nNewNum);
        else
            _pDrawVirtObj->SetOrdNum( nNewNum );
    }

    if ( FLY_IN_CNTNT == _pDrawContact->GetFmt()->GetAnchor().GetAnchorId() )
    {
        return;
    }

    if ( !pSortedObjs )
    {
        pSortedObjs = new SwSortDrawObjs();
    }
    if ( !pSortedObjs->Insert( _pDrawVirtObj ) )
    {
#ifndef PRODUCT
        USHORT nIdx;
        ASSERT( pSortedObjs->Seek_Entry( _pDrawVirtObj, &nIdx ),
                "Fly nicht in Sorted eingetragen." );
#endif
    }
    _pDrawVirtObj->SetPageFrm( this );
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

// OD 20.05.2003 #108784# - remove 'virtual' drawing object from page frame.
void SwPageFrm::RemoveVirtDrawObj( SwDrawContact* _pDrawContact,
                                   SwDrawVirtObj* _pDrawVirtObj )
{
    if ( pSortedObjs )
    {
        pSortedObjs->Remove( _pDrawVirtObj );
        if ( !pSortedObjs->Count() )
        {
            DELETEZ( pSortedObjs );
        }
        if ( GetUpper() )
        {
            if ( FLY_IN_CNTNT != _pDrawContact->GetFmt()->GetAnchor().GetAnchorId() )
            {
                ((SwRootFrm*)GetUpper())->SetSuperfluous();
                InvalidatePage();
            }
            ((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
        }
    }
    _pDrawVirtObj->SetPageFrm( 0 );
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
            SWRECTFN( pClip )

            //Vertikales clipping: Top und Bottom, ggf. an PrtArea
            const SwFmtVertOrient &rV = pFly->GetFmt()->GetVertOrient();
            if( rV.GetVertOrient() != VERT_NONE &&
                rV.GetRelationOrient() == PRTAREA )
            {
                (rRect.*fnRect->fnSetTop)( (pClip->*fnRect->fnGetPrtTop)() );
                (rRect.*fnRect->fnSetBottom)( (pClip->*fnRect->fnGetPrtBottom)() );
            }
            //Horizontales clipping: Left und Right, ggf. an PrtArea
            const SwFmtHoriOrient &rH = pFly->GetFmt()->GetHoriOrient();
            if( rH.GetHoriOrient() != HORI_NONE &&
                rH.GetRelationOrient() == PRTAREA )
            {
                (rRect.*fnRect->fnSetLeft)( (pClip->*fnRect->fnGetPrtLeft)() );
                (rRect.*fnRect->fnSetRight)((pClip->*fnRect->fnGetPrtRight)());
            }
        }
        else if( pFly->IsFlyAtCntFrm() )
        {
            const SwFrm *pClip = pFly->GetAnchor();
            SWRECTFN( pClip )
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
                    pUp = 0;
                }
            }
            if ( pUp )
            {
                if ( !pUp->IsFooterFrm() && ( !pUp->IsFlyFrm() ||
                     (!pUp->Lower() || !pUp->Lower()->IsColumnFrm()) ) )
                    pUp->Calc();
                if ( pUp->GetType() & FRM_BODY )
                {
                    const SwPageFrm *pPg;
                    if ( pUp->GetUpper() != (pPg = pFly->FindPageFrm()) )
                        pUp = pPg->FindBodyCont();
                    rRect = pUp->GetUpper()->Frm();
                    (rRect.*fnRect->fnSetTop)( (pUp->*fnRect->fnGetPrtTop)() );
                    (rRect.*fnRect->fnSetBottom)((pUp->*fnRect->fnGetPrtBottom)());
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
                        (rRect.*fnRect->fnSetBottom)(
                                    (pTab->GetUpper()->*fnRect->fnGetPrtBottom)() );
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
            const SwFrm *pUp = pFly->GetAnchor()->GetUpper();
            if( !pUp->IsFooterFrm() )
                pUp->Calc();
            SWRECTFN( pFly->GetAnchor() )
            while( pUp->IsColumnFrm() || pUp->IsSctFrm() || pUp->IsColBodyFrm())
                pUp = pUp->GetUpper();
            rRect = pUp->Frm();
            if( !pUp->IsBodyFrm() )
            {
                rRect += pUp->Prt().Pos();
                rRect.SSize( pUp->Prt().SSize() );
                if ( pUp->IsCellFrm() )
                {
                    const SwFrm *pTab = pUp->FindTabFrm();
                    (rRect.*fnRect->fnSetBottom)(
                                    (pTab->GetUpper()->*fnRect->fnGetPrtBottom)() );
                }
            }
            long nHeight = (9*(rRect.*fnRect->fnGetHeight)())/10;
            long nTop;
            const SwFmt *pFmt = ((SwContact*)GetUserCall(pSdrObj))->GetFmt();
            const SvxULSpaceItem &rUL = pFmt->GetULSpace();
            if( bMove )
            {
                nTop = bVert ? ((SwFlyInCntFrm*)pFly)->GetRefPoint().X() :
                               ((SwFlyInCntFrm*)pFly)->GetRefPoint().Y();
                nTop = (*fnRect->fnYInc)( nTop, -nHeight );
                long nWidth = (pFly->Frm().*fnRect->fnGetWidth)();
                (rRect.*fnRect->fnSetLeftAndWidth)( bVert ?
                            ((SwFlyInCntFrm*)pFly)->GetRefPoint().Y() :
                            ((SwFlyInCntFrm*)pFly)->GetRefPoint().X(), nWidth );
                nHeight = 2*nHeight - rUL.GetLower() - rUL.GetUpper();
            }
            else
            {
                nTop = (*fnRect->fnYInc)( (pFly->Frm().*fnRect->fnGetBottom)(),
                                           rUL.GetLower() - nHeight );
                nHeight = 2*nHeight - (pFly->Frm().*fnRect->fnGetHeight)()
                          - rUL.GetLower() - rUL.GetUpper();
            }
            (rRect.*fnRect->fnSetTopAndHeight)( nTop, nHeight );
        }
    }
    else
    {
        const SwDrawContact *pC = (const SwDrawContact*)GetUserCall(pSdrObj);
        const SwFrmFmt  *pFmt = (const SwFrmFmt*)pC->GetFmt();
        const SwFmtAnchor &rAnch = pFmt->GetAnchor();
        if ( FLY_IN_CNTNT == rAnch.GetAnchorId() )
        {
            const SwFrm *pFrm = pC->GetAnchor();
            if( !pFrm )
            {
                ((SwDrawContact*)pC)->ConnectToLayout();
                pFrm = pC->GetAnchor();
            }
            const SwFrm *pUp = pFrm->GetUpper();
            if( !pUp->IsFooterFrm() )
                pUp->Calc();
            rRect = pUp->Prt();
            rRect += pUp->Frm().Pos();
            SWRECTFN( pFrm )
            long nHeight = (9*(rRect.*fnRect->fnGetHeight)())/10;
            long nTop;
            const SvxULSpaceItem &rUL = pFmt->GetULSpace();
            SwRect aSnapRect( pSdrObj->GetSnapRect() );
            long nTmpH = 0;
            if( bMove )
            {
                nTop = (*fnRect->fnYInc)( bVert ? pSdrObj->GetAnchorPos().X() :
                                       pSdrObj->GetAnchorPos().Y(), -nHeight );
                long nWidth = (aSnapRect.*fnRect->fnGetWidth)();
                (rRect.*fnRect->fnSetLeftAndWidth)( bVert ?
                            pSdrObj->GetAnchorPos().Y() :
                            pSdrObj->GetAnchorPos().X(), nWidth );
            }
            else
            {
                nTop = (*fnRect->fnYInc)( (aSnapRect.*fnRect->fnGetTop)(),
                                          rUL.GetLower() + nTmpH - nHeight );
                nTmpH = bVert ? pSdrObj->GetBoundRect().GetWidth() :
                                pSdrObj->GetBoundRect().GetHeight();
            }
            nHeight = 2*nHeight - nTmpH - rUL.GetLower() - rUL.GetUpper();
            (rRect.*fnRect->fnSetTopAndHeight)( nTop, nHeight );
        }
        else
        {
            // OD 23.06.2003 #108784# - restrict clip rectangle for drawing
            // objects in header/footer to the page frame.
            const SwFrm* pAnchorFrm = 0L;
            if ( pSdrObj->ISA(SwDrawVirtObj) )
            {
                pAnchorFrm = static_cast<const SwDrawVirtObj*>(pSdrObj)->GetAnchorFrm();
            }
            else
            {
                pAnchorFrm = pC->GetAnchor();
            }
            if ( pAnchorFrm && pAnchorFrm->FindFooterOrHeader() )
            {
                // clip frame is the page frame the header/footer is on.
                const SwFrm* pClipFrm = pAnchorFrm->FindPageFrm();
                rRect = pClipFrm->Frm();
            }
            else
            {
                bRet = FALSE;
            }
        }
    }
    return bRet;
}
