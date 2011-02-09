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
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "node.hxx"
#include "doc.hxx"
#include "frmtool.hxx"
#include "flyfrm.hxx"
#include <frmfmt.hxx>
#include <cellfrm.hxx>
#include <rowfrm.hxx>
#include <swtable.hxx>

#include "tabfrm.hxx"
#include "sectfrm.hxx"
#include "flyfrms.hxx"
#include "ftnfrm.hxx"
#include "txtftn.hxx"
#include "fmtftn.hxx"
#include <txtfrm.hxx>   // SwTxtFrm
#include <switerator.hxx>

/*************************************************************************
|*
|*  FindBodyCont, FindLastBodyCntnt()
|*
|*  Beschreibung        Sucht den ersten/letzten CntntFrm im BodyText unterhalb
|*      der Seite.
|*  Ersterstellung      MA 15. Feb. 93
|*  Letzte Aenderung    MA 18. Apr. 94
|*
|*************************************************************************/
SwLayoutFrm *SwFtnBossFrm::FindBodyCont()
{
    SwFrm *pLay = Lower();
    while ( pLay && !pLay->IsBodyFrm() )
        pLay = pLay->GetNext();
    return (SwLayoutFrm*)pLay;
}

SwCntntFrm *SwPageFrm::FindLastBodyCntnt()
{
    SwCntntFrm *pRet = FindFirstBodyCntnt();
    SwCntntFrm *pNxt = pRet;
    while ( pNxt && pNxt->IsInDocBody() && IsAnLower( pNxt ) )
    {   pRet = pNxt;
        pNxt = pNxt->FindNextCnt();
    }
    return pRet;
}

/*************************************************************************
|*
|*  SwLayoutFrm::ContainsCntnt
|*
|*  Beschreibung            Prueft, ob der Frame irgendwo in seiner
|*          untergeordneten Struktur einen oder mehrere CntntFrm's enthaelt;
|*          Falls ja wird der erste gefundene CntntFrm zurueckgegeben.
|*
|*  Ersterstellung      MA 13. May. 92
|*  Letzte Aenderung    MA 20. Apr. 94
|*
|*************************************************************************/

const SwCntntFrm *SwLayoutFrm::ContainsCntnt() const
{
    //LayoutBlatt nach unten hin suchen und wenn dieses keinen Inhalt hat
    //solange die weiteren Blatter abklappern bis Inhalt gefunden oder der
    //this verlassen wird.
    //Sections: Cntnt neben Sections wuerde so nicht gefunden (leere Section
    //direct neben CntntFrm), deshalb muss fuer diese Aufwendiger rekursiv gesucht
    //werden.

    const SwLayoutFrm *pLayLeaf = this;
    do
    {
        while ( (!pLayLeaf->IsSctFrm() || pLayLeaf == this ) &&
                pLayLeaf->Lower() && pLayLeaf->Lower()->IsLayoutFrm() )
            pLayLeaf = (SwLayoutFrm*)pLayLeaf->Lower();

        if( pLayLeaf->IsSctFrm() && pLayLeaf != this )
        {
            const SwCntntFrm *pCnt = pLayLeaf->ContainsCntnt();
            if( pCnt )
                return pCnt;
            if( pLayLeaf->GetNext() )
            {
                if( pLayLeaf->GetNext()->IsLayoutFrm() )
                {
                    pLayLeaf = (SwLayoutFrm*)pLayLeaf->GetNext();
                    continue;
                }
                else
                    return (SwCntntFrm*)pLayLeaf->GetNext();
            }
        }
        else if ( pLayLeaf->Lower() )
            return (SwCntntFrm*)pLayLeaf->Lower();

        pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
        if( !IsAnLower( pLayLeaf) )
            return 0;
    } while( pLayLeaf );
    return 0;
}

/*************************************************************************
|*
|*  SwLayoutFrm::FirstCell
|*
|*  Beschreibung    ruft zunaechst ContainsAny auf, um in die innerste Zelle
|*                  hineinzukommen. Dort hangelt es sich wieder hoch zum
|*                  ersten SwCellFrm, seit es SectionFrms gibt, reicht kein
|*                  ContainsCntnt()->GetUpper() mehr...
|*  Ersterstellung      AMA 17. Mar. 99
|*  Letzte Aenderung    AMA 17. Mar. 99
|*
|*************************************************************************/

const SwCellFrm *SwLayoutFrm::FirstCell() const
{
    const SwFrm* pCnt = ContainsAny();
    while( pCnt && !pCnt->IsCellFrm() )
        pCnt = pCnt->GetUpper();
    return (const SwCellFrm*)pCnt;
}

/*************************************************************************
|*
|*  SwLayoutFrm::ContainsAny
|*
|*  Beschreibung wie ContainsCntnt, nur dass nicht nur CntntFrms, sondern auch
|*          Bereiche und Tabellen zurueckgegeben werden.
|*  Ersterstellung      AMA 10. Mar. 99
|*  Letzte Aenderung    AMA 10. Mar. 99
|*
|*************************************************************************/

// --> OD 2006-02-01 #130797#
// New parameter <_bInvestigateFtnForSections> controls investigation of
// content of footnotes for sections.
const SwFrm *SwLayoutFrm::ContainsAny( const bool _bInvestigateFtnForSections ) const
{
    //LayoutBlatt nach unten hin suchen und wenn dieses keinen Inhalt hat
    //solange die weiteren Blatter abklappern bis Inhalt gefunden oder der
    //this verlassen wird.
    // Oder bis wir einen SectionFrm oder TabFrm gefunden haben

    const SwLayoutFrm *pLayLeaf = this;
    // --> OD 2006-02-01 #130797#
    const bool bNoFtn = IsSctFrm() && !_bInvestigateFtnForSections;
    // <--
    do
    {
        while ( ( (!pLayLeaf->IsSctFrm() && !pLayLeaf->IsTabFrm())
                 || pLayLeaf == this ) &&
                pLayLeaf->Lower() && pLayLeaf->Lower()->IsLayoutFrm() )
            pLayLeaf = (SwLayoutFrm*)pLayLeaf->Lower();

        if( ( pLayLeaf->IsTabFrm() || pLayLeaf->IsSctFrm() )
            && pLayLeaf != this )
        {
            // Wir liefern jetzt auch "geloeschte" SectionFrms zurueck,
            // damit diese beim SaveCntnt und RestoreCntnt mitgepflegt werden.
            return pLayLeaf;
        }
        else if ( pLayLeaf->Lower() )
            return (SwCntntFrm*)pLayLeaf->Lower();

        pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
        if( bNoFtn && pLayLeaf && pLayLeaf->IsInFtn() )
        {
            do
            {
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
            } while( pLayLeaf && pLayLeaf->IsInFtn() );
        }
        if( !IsAnLower( pLayLeaf) )
            return 0;
    } while( pLayLeaf );
    return 0;
}


/*************************************************************************
|*
|*  SwFrm::GetLower()
|*
|*  Ersterstellung      MA 27. Jul. 92
|*  Letzte Aenderung    MA 09. Oct. 97
|*
|*************************************************************************/
const SwFrm* SwFrm::GetLower() const
{
    return IsLayoutFrm() ? ((SwLayoutFrm*)this)->Lower() : 0;
}

SwFrm* SwFrm::GetLower()
{
    return IsLayoutFrm() ? ((SwLayoutFrm*)this)->Lower() : 0;
}

/*************************************************************************
|*
|*  SwLayoutFrm::IsAnLower()
|*
|*  Ersterstellung      MA 18. Mar. 93
|*  Letzte Aenderung    MA 18. Mar. 93
|*
|*************************************************************************/
sal_Bool SwLayoutFrm::IsAnLower( const SwFrm *pAssumed ) const
{
    const SwFrm *pUp = pAssumed;
    while ( pUp )
    {
        if ( pUp == this )
            return sal_True;
        if ( pUp->IsFlyFrm() )
            pUp = ((SwFlyFrm*)pUp)->GetAnchorFrm();
        else
            pUp = pUp->GetUpper();
    }
    return sal_False;
}

/** method to check relative position of layout frame to
    a given layout frame.

    OD 08.11.2002 - refactoring of pseudo-local method <lcl_Apres(..)> in
    <txtftn.cxx> for #104840#.

    @param _aCheckRefLayFrm
    constant reference of an instance of class <SwLayoutFrm> which
    is used as the reference for the relative position check.

    @author OD

    @return true, if <this> is positioned before the layout frame <p>
*/
bool SwLayoutFrm::IsBefore( const SwLayoutFrm* _pCheckRefLayFrm ) const
{
    ASSERT( !IsRootFrm() , "<IsBefore> called at a <SwRootFrm>.");
    ASSERT( !_pCheckRefLayFrm->IsRootFrm() , "<IsBefore> called with a <SwRootFrm>.");

    bool bReturn;

    // check, if on different pages
    const SwPageFrm *pMyPage = FindPageFrm();
    const SwPageFrm *pCheckRefPage = _pCheckRefLayFrm->FindPageFrm();
    if( pMyPage != pCheckRefPage )
    {
        // being on different page as check reference
        bReturn = pMyPage->GetPhyPageNum() < pCheckRefPage->GetPhyPageNum();
    }
    else
    {
        // being on same page as check reference
        // --> search my supreme parent <pUp>, which doesn't contain check reference.
        const SwLayoutFrm* pUp = this;
        while ( pUp->GetUpper() &&
                !pUp->GetUpper()->IsAnLower( _pCheckRefLayFrm )
              )
            pUp = pUp->GetUpper();
        if( !pUp->GetUpper() )
        {
            // can occur, if <this> is a fly frm
            bReturn = false;
        }
        else
        {
            // travel through the next's of <pUp> and check if one of these
            // contain the check reference.
            SwLayoutFrm* pUpNext = (SwLayoutFrm*)pUp->GetNext();
            while ( pUpNext &&
                    !pUpNext->IsAnLower( _pCheckRefLayFrm ) )
            {
                pUpNext = (SwLayoutFrm*)pUpNext->GetNext();
            }
            bReturn = pUpNext != 0;
        }
    }

    return bReturn;
}

//
// Local helper functions for GetNextLayoutLeaf
//

const SwFrm* lcl_FindLayoutFrame( const SwFrm* pFrm, bool bNext )
{
    const SwFrm* pRet = 0;
    if ( pFrm->IsFlyFrm() )
        pRet = bNext ? ((SwFlyFrm*)pFrm)->GetNextLink() : ((SwFlyFrm*)pFrm)->GetPrevLink();
    else
        pRet = bNext ? pFrm->GetNext() : pFrm->GetPrev();

    return pRet;
}

const SwFrm* lcl_GetLower( const SwFrm* pFrm, bool bFwd )
{
    if ( !pFrm->IsLayoutFrm() )
        return 0;

    return bFwd ?
           static_cast<const SwLayoutFrm*>(pFrm)->Lower() :
           static_cast<const SwLayoutFrm*>(pFrm)->GetLastLower();
}

/*************************************************************************
|*
|*  SwFrm::ImplGetNextLayoutLeaf
|*
|* Finds the next layout leaf. This is a layout frame, which does not
 * have a lower which is a LayoutFrame. That means, pLower can be 0 or a
 * content frame.
 *
 * However, pLower may be a TabFrm
 *
|*************************************************************************/

const SwLayoutFrm *SwFrm::ImplGetNextLayoutLeaf( bool bFwd ) const
{
    const SwFrm       *pFrm = this;
    const SwLayoutFrm *pLayoutFrm = 0;
    const SwFrm       *p = 0;
    bool bGoingUp = !bFwd;          // false for forward, true for backward
    do {

         bool bGoingFwdOrBwd = false, bGoingDown = false;

         bGoingDown = ( !bGoingUp && ( 0 != (p = lcl_GetLower( pFrm, bFwd ) ) ) );
         if ( !bGoingDown )
         {
             // I cannot go down, because either I'm currently going up or
             // because the is no lower.
             // I'll try to go forward:
             bGoingFwdOrBwd = (0 != (p = lcl_FindLayoutFrame( pFrm, bFwd ) ) );
             if ( !bGoingFwdOrBwd )
             {
                 // I cannot go forward, because there is no next frame.
                 // I'll try to go up:
                 bGoingUp = (0 != (p = pFrm->GetUpper() ) );
                 if ( !bGoingUp )
                 {
                    // I cannot go up, because there is no upper frame.
                    return 0;
                 }
             }
         }

        // If I could not go down or forward, I'll have to go up
        bGoingUp = !bGoingFwdOrBwd && !bGoingDown;

        pFrm = p;
        p = lcl_GetLower( pFrm, true );

    } while( ( p && !p->IsFlowFrm() ) ||
             pFrm == this ||
             0 == ( pLayoutFrm = pFrm->IsLayoutFrm() ? (SwLayoutFrm*)pFrm : 0 ) ||
             pLayoutFrm->IsAnLower( this ) );

    return pLayoutFrm;
}



/*************************************************************************
|*
|*    SwFrm::ImplGetNextCntntFrm( bool )
|*
|*      Rueckwaertswandern im Baum: Den untergeordneten Frm greifen,
|*      wenn es einen gibt und nicht gerade zuvor um eine Ebene
|*      aufgestiegen wurde (das wuerde zu einem endlosen Auf und Ab
|*      fuehren!). Damit wird sichergestellt, dass beim
|*      Rueckwaertswandern alle Unterbaeume durchsucht werden. Wenn
|*      abgestiegen wurde, wird zuerst an das Ende der Kette gegangen,
|*      weil im weiteren ja vom letzten Frm innerhalb eines anderen
|*      Frms rueckwaerts gegangen wird.
|*      Vorwaetzwander funktioniert analog.
|*
|*    Ersterstellung    ??
|*    Letzte Aenderung  MA 30. Oct. 97
|*
|*************************************************************************/

// Achtung: Fixes in ImplGetNextCntntFrm() muessen moeglicherweise auch in
// die weiter oben stehende Methode lcl_NextFrm(..) eingepflegt werden
const SwCntntFrm* SwCntntFrm::ImplGetNextCntntFrm( bool bFwd ) const
{
    const SwFrm *pFrm = this;
    // #100926#
    SwCntntFrm *pCntntFrm = 0;
    sal_Bool bGoingUp = sal_False;
    do {
        const SwFrm *p = 0;
        sal_Bool bGoingFwdOrBwd = sal_False, bGoingDown = sal_False;

        bGoingDown = ( !bGoingUp && ( 0 != ( p = lcl_GetLower( pFrm, true ) ) ) );
        if ( !bGoingDown )
        {
            bGoingFwdOrBwd = ( 0 != ( p = lcl_FindLayoutFrame( pFrm, bFwd ) ) );
            if ( !bGoingFwdOrBwd )
            {
                bGoingUp = ( 0 != ( p = pFrm->GetUpper() ) );
                if ( !bGoingUp )
                {
                    return 0;
                }
            }
        }

        bGoingUp = !(bGoingFwdOrBwd || bGoingDown);

        if ( !bFwd )
        {
            if( bGoingDown && p )
                while ( p->GetNext() )
                    p = p->GetNext();
        }

        pFrm = p;
    } while ( 0 == (pCntntFrm = (pFrm->IsCntntFrm() ? (SwCntntFrm*)pFrm:0) ));

    return pCntntFrm;
}




/*************************************************************************
|*
|*  SwFrm::FindRootFrm(), FindTabFrm(), FindFtnFrm(), FindFlyFrm(),
|*         FindPageFrm(), FindColFrm()
|*
|*  Ersterstellung      ??
|*  Letzte Aenderung    MA 05. Sep. 93
|*
|*************************************************************************/
SwPageFrm* SwFrm::FindPageFrm()
{
    SwFrm *pRet = this;
    while ( pRet && !pRet->IsPageFrm() )
    {
        if ( pRet->GetUpper() )
            pRet = pRet->GetUpper();
        else if ( pRet->IsFlyFrm() )
        {
            // --> OD 2004-06-30 #i28701# - use new method <GetPageFrm()>
            if ( static_cast<SwFlyFrm*>(pRet)->GetPageFrm() )
                pRet = static_cast<SwFlyFrm*>(pRet)->GetPageFrm();
            else
                pRet = static_cast<SwFlyFrm*>(pRet)->AnchorFrm();
        }
        else
            return 0;
    }
    return (SwPageFrm*)pRet;
}

SwFtnBossFrm* SwFrm::FindFtnBossFrm( sal_Bool bFootnotes )
{
    SwFrm *pRet = this;
    // Innerhalb einer Tabelle gibt es keine Fussnotenbosse, auch spaltige
    // Bereiche enthalten dort keine Fussnotentexte
    if( pRet->IsInTab() )
        pRet = pRet->FindTabFrm();
    while ( pRet && !pRet->IsFtnBossFrm() )
    {
        if ( pRet->GetUpper() )
            pRet = pRet->GetUpper();
        else if ( pRet->IsFlyFrm() )
        {
            // --> OD 2004-06-30 #i28701# - use new method <GetPageFrm()>
            if ( static_cast<SwFlyFrm*>(pRet)->GetPageFrm() )
                pRet = static_cast<SwFlyFrm*>(pRet)->GetPageFrm();
            else
                pRet = static_cast<SwFlyFrm*>(pRet)->AnchorFrm();
        }
        else
            return 0;
    }
    if( bFootnotes && pRet && pRet->IsColumnFrm() &&
        !pRet->GetNext() && !pRet->GetPrev() )
    {
        SwSectionFrm* pSct = pRet->FindSctFrm();
        ASSERT( pSct, "FindFtnBossFrm: Single column outside section?" );
        if( !pSct->IsFtnAtEnd() )
            return pSct->FindFtnBossFrm( sal_True );
    }
    return (SwFtnBossFrm*)pRet;
}

SwTabFrm* SwFrm::ImplFindTabFrm()
{
    SwFrm *pRet = this;
    while ( !pRet->IsTabFrm() )
    {
        pRet = pRet->GetUpper();
        if ( !pRet )
            return 0;
    }
    return (SwTabFrm*)pRet;
}

SwSectionFrm* SwFrm::ImplFindSctFrm()
{
    SwFrm *pRet = this;
    while ( !pRet->IsSctFrm() )
    {
        pRet = pRet->GetUpper();
        if ( !pRet )
            return 0;
    }
    return (SwSectionFrm*)pRet;
}

SwFtnFrm *SwFrm::ImplFindFtnFrm()
{
    SwFrm *pRet = this;
    while ( !pRet->IsFtnFrm() )
    {
        pRet = pRet->GetUpper();
        if ( !pRet )
            return 0;
    }
    return (SwFtnFrm*)pRet;
}

SwFlyFrm *SwFrm::ImplFindFlyFrm()
{
    const SwFrm *pRet = this;
    do
    {
        if ( pRet->IsFlyFrm() )
            return (SwFlyFrm*)pRet;
        else
            pRet = pRet->GetUpper();
    } while ( pRet );
    return 0;
}

SwFrm *SwFrm::FindColFrm()
{
    SwFrm *pFrm = this;
    do
    {   pFrm = pFrm->GetUpper();
    } while ( pFrm && !pFrm->IsColumnFrm() );
    return pFrm;
}

SwFrm* SwFrm::FindFooterOrHeader()
{
    SwFrm* pRet = this;
    do
    {   if ( pRet->GetType() & 0x0018 ) //Header und Footer
            return pRet;
        else if ( pRet->GetUpper() )
            pRet = pRet->GetUpper();
        else if ( pRet->IsFlyFrm() )
            pRet = ((SwFlyFrm*)pRet)->AnchorFrm();
        else
            return 0;
    } while ( pRet );
    return pRet;
}

const SwFtnFrm* SwFtnContFrm::FindFootNote() const
{
    const SwFtnFrm* pRet = (SwFtnFrm*)Lower();
    if( pRet && !pRet->GetAttr()->GetFtn().IsEndNote() )
        return pRet;
    return NULL;
}

const SwPageFrm* SwRootFrm::GetPageAtPos( const Point& rPt, const Size* pSize, bool bExtend ) const
{
    const SwPageFrm* pRet = 0;

    SwRect aRect;
    if ( pSize )
    {
        aRect.Pos()  = rPt;
        aRect.SSize() = *pSize;
    }

    const SwFrm* pPage = Lower();

    if ( !bExtend )
    {
        if( !Frm().IsInside( rPt ) )
            return 0;

        // skip pages above point:
        while( pPage && rPt.Y() > pPage->Frm().Bottom() )
            pPage = pPage->GetNext();
    }

    ASSERT( GetPageNum() <= maPageRects.size(), "number of pages differes from page rect array size" )
    sal_uInt16 nPageIdx = 0;

    while ( pPage && !pRet )
    {
        const SwRect& rBoundRect = bExtend ? maPageRects[ nPageIdx++ ] : pPage->Frm();

        if ( (!pSize && rBoundRect.IsInside(rPt)) ||
              (pSize && rBoundRect.IsOver(aRect)) )
        {
            pRet = static_cast<const SwPageFrm*>(pPage);
        }

        pPage = pPage->GetNext();
    }

    return pRet;
}

/*************************************************************************
|*
|*  SwFrmFrm::GetAttrSet()
|*
|*  Ersterstellung      MA 02. Aug. 93
|*  Letzte Aenderung    MA 02. Aug. 93
|*
|*************************************************************************/
const SwAttrSet* SwFrm::GetAttrSet() const
{
    if ( IsCntntFrm() )
        return &((const SwCntntFrm*)this)->GetNode()->GetSwAttrSet();
    else
        return &((const SwLayoutFrm*)this)->GetFmt()->GetAttrSet();
}

/*************************************************************************
|*
|*  SwFrm::_FindNext(), _FindPrev(), InvalidateNextPos()
|*         _FindNextCnt() geht in Tabellen und Bereiche hineinund liefert
|*         nur SwCntntFrms.
|*
|*  Beschreibung        Invalidiert die Position des Naechsten Frames.
|*      Dies ist der direkte Nachfolger, oder bei CntntFrm's der naechste
|*      CntntFrm der im gleichen Fluss liegt wie ich:
|*      - Body,
|*      - Fussnoten,
|*      - Bei Kopf-/Fussbereichen ist die Benachrichtigung nur innerhalb des
|*        Bereiches weiterzuleiten.
|*      - dito fuer Flys.
|*      - Cntnts in Tabs halten sich ausschliesslich innerhalb ihrer Zelle
|*        auf.
|*      - Tabellen verhalten sich prinzipiell analog zu den Cntnts
|*      - Bereiche ebenfalls
|*  Ersterstellung      AK 14-Feb-1991
|*  Letzte Aenderung    AMA 10. Mar. 99
|*
|*************************************************************************/

// Diese Hilfsfunktion ist ein Aequivalent zur ImplGetNextCntntFrm()-Methode,
// sie liefert allerdings neben ContentFrames auch TabFrms und SectionFrms.
SwFrm* lcl_NextFrm( SwFrm* pFrm )
{
    SwFrm *pRet = 0;
    sal_Bool bGoingUp = sal_False;
    do {
        SwFrm *p = 0;

        sal_Bool bGoingFwd = sal_False;
        sal_Bool bGoingDown = (!bGoingUp && ( 0 != (p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0)));

        if( !bGoingDown )
        {
            bGoingFwd = (0 != (p = ( pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->GetNextLink() : pFrm->GetNext())));
            if ( !bGoingFwd )
            {
                bGoingUp = (0 != (p = pFrm->GetUpper()));
                if ( !bGoingUp )
                {
                    return 0;
                }
            }
        }
        bGoingUp = !(bGoingFwd || bGoingDown);
        pFrm = p;
    } while ( 0 == (pRet = ( ( pFrm->IsCntntFrm() || ( !bGoingUp &&
            ( pFrm->IsTabFrm() || pFrm->IsSctFrm() ) ) )? pFrm : 0 ) ) );
    return pRet;
}

SwFrm *SwFrm::_FindNext()
{
    sal_Bool bIgnoreTab = sal_False;
    SwFrm *pThis = this;

    if ( IsTabFrm() )
    {
        //Der letzte Cntnt der Tabelle wird
        //gegriffen und dessen Nachfolger geliefert. Um die Spezialbeh.
        //Fuer Tabellen (s.u.) auszuschalten wird bIgnoreTab gesetzt.
        if ( ((SwTabFrm*)this)->GetFollow() )
            return ((SwTabFrm*)this)->GetFollow();

        pThis = ((SwTabFrm*)this)->FindLastCntnt();
        if ( !pThis )
            pThis = this;
        bIgnoreTab = sal_True;
    }
    else if ( IsSctFrm() )
    {
        //Der letzte Cntnt des Bereichs wird gegriffen und dessen Nachfolger
        // geliefert.
        if ( ((SwSectionFrm*)this)->GetFollow() )
            return ((SwSectionFrm*)this)->GetFollow();

        pThis = ((SwSectionFrm*)this)->FindLastCntnt();
        if ( !pThis )
            pThis = this;
    }
    else if ( IsCntntFrm() )
    {
        if( ((SwCntntFrm*)this)->GetFollow() )
            return ((SwCntntFrm*)this)->GetFollow();
    }
    else if ( IsRowFrm() )
    {
        SwFrm* pMyUpper = GetUpper();
        if ( pMyUpper->IsTabFrm() && ((SwTabFrm*)pMyUpper)->GetFollow() )
            return ((SwTabFrm*)pMyUpper)->GetFollow()->GetLower();
        else return NULL;
    }
    else
        return NULL;

    SwFrm* pRet = NULL;
    const sal_Bool bFtn  = pThis->IsInFtn();
    if ( !bIgnoreTab && pThis->IsInTab() )
    {
        SwLayoutFrm *pUp = pThis->GetUpper();
        while ( !pUp->IsCellFrm() )
            pUp = pUp->GetUpper();
        ASSERT( pUp, "Cntnt in Tabelle aber nicht in Zelle." );
        SwFrm* pNxt = ((SwCellFrm*)pUp)->GetFollowCell();
        if ( pNxt )
            pNxt = ((SwCellFrm*)pNxt)->ContainsCntnt();
        if ( !pNxt )
        {
            pNxt = lcl_NextFrm( pThis );
            if ( pUp->IsAnLower( pNxt ) )
                pRet = pNxt;
        }
        else
            pRet = pNxt;
    }
    else
    {
        const sal_Bool bBody = pThis->IsInDocBody();
        SwFrm *pNxtCnt = lcl_NextFrm( pThis );
        if ( pNxtCnt )
        {
            if ( bBody || bFtn )
            {
                while ( pNxtCnt )
                {
                    // OD 02.04.2003 #108446# - check for endnote, only if found
                    // next content isn't contained in a section, that collect its
                    // endnotes at its end.
                    bool bEndn = IsInSct() && !IsSctFrm() &&
                                 ( !pNxtCnt->IsInSct() ||
                                   !pNxtCnt->FindSctFrm()->IsEndnAtEnd()
                                 );
                    if ( ( bBody && pNxtCnt->IsInDocBody() ) ||
                         ( pNxtCnt->IsInFtn() &&
                           ( bFtn ||
                             ( bEndn && pNxtCnt->FindFtnFrm()->GetAttr()->GetFtn().IsEndNote() )
                           )
                         )
                       )
                    {
                        pRet = pNxtCnt->IsInTab() ? pNxtCnt->FindTabFrm()
                                                    : (SwFrm*)pNxtCnt;
                        break;
                    }
                    pNxtCnt = lcl_NextFrm( pNxtCnt );
                }
            }
            else if ( pThis->IsInFly() )
            {
                pRet = pNxtCnt->IsInTab() ? pNxtCnt->FindTabFrm()
                                            : (SwFrm*)pNxtCnt;
            }
            else    //Fuss-/oder Kopfbereich
            {
                const SwFrm *pUp = pThis->GetUpper();
                const SwFrm *pCntUp = pNxtCnt->GetUpper();
                while ( pUp && pUp->GetUpper() &&
                        !pUp->IsHeaderFrm() && !pUp->IsFooterFrm() )
                    pUp = pUp->GetUpper();
                while ( pCntUp && pCntUp->GetUpper() &&
                        !pCntUp->IsHeaderFrm() && !pCntUp->IsFooterFrm() )
                    pCntUp = pCntUp->GetUpper();
                if ( pCntUp == pUp )
                {
                    pRet = pNxtCnt->IsInTab() ? pNxtCnt->FindTabFrm()
                                                : (SwFrm*)pNxtCnt;
                }
            }
        }
    }
    if( pRet && pRet->IsInSct() )
    {
        SwSectionFrm* pSct = pRet->FindSctFrm();
        //Fussnoten in spaltigen Rahmen duerfen nicht den Bereich
        //liefern, der die Fussnoten umfasst
        if( !pSct->IsAnLower( this ) &&
            (!bFtn || pSct->IsInFtn() ) )
            return pSct;
    }
    return pRet;
}

// --> OD 2005-12-01 #i27138# - add parameter <_bInSameFtn>
SwCntntFrm *SwFrm::_FindNextCnt( const bool _bInSameFtn )
{
    SwFrm *pThis = this;

    if ( IsTabFrm() )
    {
        if ( ((SwTabFrm*)this)->GetFollow() )
        {
            pThis = ((SwTabFrm*)this)->GetFollow()->ContainsCntnt();
            if( pThis )
                return (SwCntntFrm*)pThis;
        }
        pThis = ((SwTabFrm*)this)->FindLastCntnt();
        if ( !pThis )
            return 0;
    }
    else if ( IsSctFrm() )
    {
        if ( ((SwSectionFrm*)this)->GetFollow() )
        {
            pThis = ((SwSectionFrm*)this)->GetFollow()->ContainsCntnt();
            if( pThis )
                return (SwCntntFrm*)pThis;
        }
        pThis = ((SwSectionFrm*)this)->FindLastCntnt();
        if ( !pThis )
            return 0;
    }
    else if ( IsCntntFrm() && ((SwCntntFrm*)this)->GetFollow() )
        return ((SwCntntFrm*)this)->GetFollow();

    if ( pThis->IsCntntFrm() )
    {
        const sal_Bool bBody = pThis->IsInDocBody();
        const sal_Bool bFtn  = pThis->IsInFtn();
        SwCntntFrm *pNxtCnt = ((SwCntntFrm*)pThis)->GetNextCntntFrm();
        if ( pNxtCnt )
        {
            // --> OD 2005-12-01 #i27138#
            if ( bBody || ( bFtn && !_bInSameFtn ) )
            // <--
            {
                // handling for environments 'footnotes' and 'document body frames':
                while ( pNxtCnt )
                {
                    if ( (bBody && pNxtCnt->IsInDocBody()) ||
                         (bFtn  && pNxtCnt->IsInFtn()) )
                        return pNxtCnt;
                    pNxtCnt = pNxtCnt->GetNextCntntFrm();
                }
            }
            // --> OD 2005-12-01 #i27138#
            else if ( bFtn && _bInSameFtn )
            {
                // handling for environments 'each footnote':
                // Assure that found next content frame belongs to the same footnotes
                const SwFtnFrm* pFtnFrmOfNext( pNxtCnt->FindFtnFrm() );
                const SwFtnFrm* pFtnFrmOfCurr( pThis->FindFtnFrm() );
                ASSERT( pFtnFrmOfCurr,
                        "<SwFrm::_FindNextCnt() - unknown layout situation: current frame has to have an upper footnote frame." );
                if ( pFtnFrmOfNext == pFtnFrmOfCurr )
                {
                    return pNxtCnt;
                }
                else if ( pFtnFrmOfCurr->GetFollow() )
                {
                    // next content frame has to be the first content frame
                    // in the follow footnote, which contains a content frame.
                    SwFtnFrm* pFollowFtnFrmOfCurr(
                                        const_cast<SwFtnFrm*>(pFtnFrmOfCurr) );
                    pNxtCnt = 0L;
                    do {
                        pFollowFtnFrmOfCurr = pFollowFtnFrmOfCurr->GetFollow();
                        pNxtCnt = pFollowFtnFrmOfCurr->ContainsCntnt();
                    } while ( !pNxtCnt && pFollowFtnFrmOfCurr->GetFollow() );
                    return pNxtCnt;
                }
                else
                {
                    // current content frame is the last content frame in the
                    // footnote - no next content frame exists.
                    return 0L;
                }
            }
            // <--
            else if ( pThis->IsInFly() )
                // handling for environments 'unlinked fly frame' and
                // 'group of linked fly frames':
                return pNxtCnt;
            else
            {
                // handling for environments 'page header' and 'page footer':
                const SwFrm *pUp = pThis->GetUpper();
                const SwFrm *pCntUp = pNxtCnt->GetUpper();
                while ( pUp && pUp->GetUpper() &&
                        !pUp->IsHeaderFrm() && !pUp->IsFooterFrm() )
                    pUp = pUp->GetUpper();
                while ( pCntUp && pCntUp->GetUpper() &&
                        !pCntUp->IsHeaderFrm() && !pCntUp->IsFooterFrm() )
                    pCntUp = pCntUp->GetUpper();
                if ( pCntUp == pUp )
                    return pNxtCnt;
            }
        }
    }
    return 0;
}

/** method to determine previous content frame in the same environment
    for a flow frame (content frame, table frame, section frame)

    OD 2005-11-30 #i27138#

    @author OD
*/
SwCntntFrm* SwFrm::_FindPrevCnt( const bool _bInSameFtn )
{
    if ( !IsFlowFrm() )
    {
        // nothing to do, if current frame isn't a flow frame.
        return 0L;
    }

    SwCntntFrm* pPrevCntntFrm( 0L );

    // Because method <SwCntntFrm::GetPrevCntntFrm()> is used to travel
    // through the layout, a content frame, at which the travel starts, is needed.
    SwCntntFrm* pCurrCntntFrm = dynamic_cast<SwCntntFrm*>(this);

    // perform shortcut, if current frame is a follow, and
    // determine <pCurrCntntFrm>, if current frame is a table or section frame
    if ( pCurrCntntFrm && pCurrCntntFrm->IsFollow() )
    {
        // previous content frame is its master content frame
        pPrevCntntFrm = pCurrCntntFrm->FindMaster();
    }
    else if ( IsTabFrm() )
    {
        SwTabFrm* pTabFrm( static_cast<SwTabFrm*>(this) );
        if ( pTabFrm->IsFollow() )
        {
            // previous content frame is the last content of its master table frame
            pPrevCntntFrm = pTabFrm->FindMaster()->FindLastCntnt();
        }
        else
        {
            // start content frame for the search is the first content frame of
            // the table frame.
            pCurrCntntFrm = pTabFrm->ContainsCntnt();
        }
    }
    else if ( IsSctFrm() )
    {
        SwSectionFrm* pSectFrm( static_cast<SwSectionFrm*>(this) );
        if ( pSectFrm->IsFollow() )
        {
            // previous content frame is the last content of its master section frame
            pPrevCntntFrm = pSectFrm->FindMaster()->FindLastCntnt();
        }
        else
        {
            // start content frame for the search is the first content frame of
            // the section frame.
            pCurrCntntFrm = pSectFrm->ContainsCntnt();
        }
    }

    // search for next content frame, depending on the environment, in which
    // the current frame is in.
    if ( !pPrevCntntFrm && pCurrCntntFrm )
    {
        pPrevCntntFrm = pCurrCntntFrm->GetPrevCntntFrm();
        if ( pPrevCntntFrm )
        {
            if ( pCurrCntntFrm->IsInFly() )
            {
                // handling for environments 'unlinked fly frame' and
                // 'group of linked fly frames':
                // Nothing to do, <pPrevCntntFrm> is the one
            }
            else
            {
                const bool bInDocBody = pCurrCntntFrm->IsInDocBody();
                const bool bInFtn  = pCurrCntntFrm->IsInFtn();
                if ( bInDocBody || ( bInFtn && !_bInSameFtn ) )
                {
                    // handling for environments 'footnotes' and 'document body frames':
                    // Assure that found previous frame is also in one of these
                    // environments. Otherwise, travel further
                    while ( pPrevCntntFrm )
                    {
                        if ( ( bInDocBody && pPrevCntntFrm->IsInDocBody() ) ||
                             ( bInFtn && pPrevCntntFrm->IsInFtn() ) )
                        {
                            break;
                        }
                        pPrevCntntFrm = pPrevCntntFrm->GetPrevCntntFrm();
                    }
                }
                else if ( bInFtn && _bInSameFtn )
                {
                    // handling for environments 'each footnote':
                    // Assure that found next content frame belongs to the same footnotes
                    const SwFtnFrm* pFtnFrmOfPrev( pPrevCntntFrm->FindFtnFrm() );
                    const SwFtnFrm* pFtnFrmOfCurr( pCurrCntntFrm->FindFtnFrm() );
                    if ( pFtnFrmOfPrev != pFtnFrmOfCurr )
                    {
                        if ( pFtnFrmOfCurr->GetMaster() )
                        {
                            SwFtnFrm* pMasterFtnFrmOfCurr(
                                        const_cast<SwFtnFrm*>(pFtnFrmOfCurr) );
                            pPrevCntntFrm = 0L;
                            // --> OD 2007-07-05 #146872#
                            // correct wrong loop-condition
                            do {
                                pMasterFtnFrmOfCurr = pMasterFtnFrmOfCurr->GetMaster();
                                pPrevCntntFrm = pMasterFtnFrmOfCurr->FindLastCntnt();
                            } while ( !pPrevCntntFrm &&
                                      pMasterFtnFrmOfCurr->GetMaster() );
                            // <--
                        }
                        else
                        {
                            // current content frame is the first content in the
                            // footnote - no previous content exists.
                            pPrevCntntFrm = 0L;;
                        }
                    }
                }
                else
                {
                    // handling for environments 'page header' and 'page footer':
                    // Assure that found previous frame is also in the same
                    // page header respectively page footer as <pCurrCntntFrm>
                    // Note: At this point its clear, that <pCurrCntntFrm> has
                    //       to be inside a page header or page footer and that
                    //       neither <pCurrCntntFrm> nor <pPrevCntntFrm> are
                    //       inside a fly frame.
                    //       Thus, method <FindFooterOrHeader()> can be used.
                    ASSERT( pCurrCntntFrm->FindFooterOrHeader(),
                            "<SwFrm::_FindPrevCnt()> - unknown layout situation: current frame should be in page header or page footer" );
                    ASSERT( !pPrevCntntFrm->IsInFly(),
                            "<SwFrm::_FindPrevCnt()> - unknown layout situation: found previous frame should *not* be inside a fly frame." );
                    if ( pPrevCntntFrm->FindFooterOrHeader() !=
                                            pCurrCntntFrm->FindFooterOrHeader() )
                    {
                        pPrevCntntFrm = 0L;
                    }
                }
            }
        }
    }

    return pPrevCntntFrm;
}

SwFrm *SwFrm::_FindPrev()
{
    sal_Bool bIgnoreTab = sal_False;
    SwFrm *pThis = this;

    if ( IsTabFrm() )
    {
        //Der erste Cntnt der Tabelle wird
        //gegriffen und dessen Vorgaenger geliefert. Um die Spezialbeh.
        //Fuer Tabellen (s.u.) auszuschalten wird bIgnoreTab gesetzt.
        if ( ((SwTabFrm*)this)->IsFollow() )
            return ((SwTabFrm*)this)->FindMaster();
        else
            pThis = ((SwTabFrm*)this)->ContainsCntnt();
        bIgnoreTab = sal_True;
    }

    if ( pThis && pThis->IsCntntFrm() )
    {
        SwCntntFrm *pPrvCnt = ((SwCntntFrm*)pThis)->GetPrevCntntFrm();
        if( !pPrvCnt )
            return 0;
        if ( !bIgnoreTab && pThis->IsInTab() )
        {
            SwLayoutFrm *pUp = pThis->GetUpper();
            while ( !pUp->IsCellFrm() )
                pUp = pUp->GetUpper();
            ASSERT( pUp, "Cntnt in Tabelle aber nicht in Zelle." );
            if ( pUp->IsAnLower( pPrvCnt ) )
                return pPrvCnt;
        }
        else
        {
            SwFrm* pRet;
            const sal_Bool bBody = pThis->IsInDocBody();
            const sal_Bool bFtn  = bBody ? sal_False : pThis->IsInFtn();
            if ( bBody || bFtn )
            {
                while ( pPrvCnt )
                {
                    if ( (bBody && pPrvCnt->IsInDocBody()) ||
                            (bFtn   && pPrvCnt->IsInFtn()) )
                    {
                        pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrm()
                                                  : (SwFrm*)pPrvCnt;
                        return pRet;
                    }
                    pPrvCnt = pPrvCnt->GetPrevCntntFrm();
                }
            }
            else if ( pThis->IsInFly() )
            {
                pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrm()
                                            : (SwFrm*)pPrvCnt;
                return pRet;
            }
            else    //Fuss-/oder Kopfbereich oder Fly
            {
                const SwFrm *pUp = pThis->GetUpper();
                const SwFrm *pCntUp = pPrvCnt->GetUpper();
                while ( pUp && pUp->GetUpper() &&
                        !pUp->IsHeaderFrm() && !pUp->IsFooterFrm() )
                    pUp = pUp->GetUpper();
                while ( pCntUp && pCntUp->GetUpper() )
                    pCntUp = pCntUp->GetUpper();
                if ( pCntUp == pUp )
                {
                    pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrm()
                                                : (SwFrm*)pPrvCnt;
                    return pRet;
                }
            }
        }
    }
    return 0;
}

void SwFrm::ImplInvalidateNextPos( sal_Bool bNoFtn )
{
    SwFrm *pFrm;
    if ( 0 != (pFrm = _FindNext()) )
    {
        if( pFrm->IsSctFrm() )
        {
            while( pFrm && pFrm->IsSctFrm() )
            {
                if( ((SwSectionFrm*)pFrm)->GetSection() )
                {
                    SwFrm* pTmp = ((SwSectionFrm*)pFrm)->ContainsAny();
                    if( pTmp )
                        pTmp->InvalidatePos();
                    else if( !bNoFtn )
                        ((SwSectionFrm*)pFrm)->InvalidateFtnPos();
                    if( !IsInSct() || FindSctFrm()->GetFollow() != pFrm )
                        pFrm->InvalidatePos();
                    return;
                }
                pFrm = pFrm->FindNext();
            }
            if( pFrm )
            {
                if ( pFrm->IsSctFrm())
                { // Damit der Inhalt eines Bereichs die Chance erhaelt,
                  // die Seite zu wechseln, muss er ebenfalls invalidiert werden.
                    SwFrm* pTmp = ((SwSectionFrm*)pFrm)->ContainsAny();
                    if( pTmp )
                        pTmp->InvalidatePos();
                    if( !IsInSct() || FindSctFrm()->GetFollow() != pFrm )
                        pFrm->InvalidatePos();
                }
                else
                    pFrm->InvalidatePos();
            }
        }
        else
            pFrm->InvalidatePos();
    }
}

/** method to invalidate printing area of next frame

    OD 09.01.2004 #i11859#

    @author OD

    FME 2004-04-19 #i27145# Moved function from SwTxtFrm to SwFrm
*/
void SwFrm::InvalidateNextPrtArea()
{
    // determine next frame
    SwFrm* pNextFrm = FindNext();
    // skip empty section frames and hidden text frames
    {
        while ( pNextFrm &&
                ( ( pNextFrm->IsSctFrm() &&
                    !static_cast<SwSectionFrm*>(pNextFrm)->GetSection() ) ||
                  ( pNextFrm->IsTxtFrm() &&
                    static_cast<SwTxtFrm*>(pNextFrm)->IsHiddenNow() ) ) )
        {
            pNextFrm = pNextFrm->FindNext();
        }
    }

    // Invalidate printing area of found next frame
    if ( pNextFrm )
    {
        if ( pNextFrm->IsSctFrm() )
        {
            // Invalidate printing area of found section frame, if
            // (1) this text frame isn't in a section OR
            // (2) found section frame isn't a follow of the section frame this
            //     text frame is in.
            if ( !IsInSct() || FindSctFrm()->GetFollow() != pNextFrm )
            {
                pNextFrm->InvalidatePrt();
            }

            // Invalidate printing area of first content in found section.
            SwFrm* pFstCntntOfSctFrm =
                    static_cast<SwSectionFrm*>(pNextFrm)->ContainsAny();
            if ( pFstCntntOfSctFrm )
            {
                pFstCntntOfSctFrm->InvalidatePrt();
            }
        }
        else
        {
            pNextFrm->InvalidatePrt();
        }
    }
}

/*************************************************************************
|*
|*    lcl_IsInColSect()
|*      liefert nur sal_True, wenn der Frame _direkt_ in einem spaltigen Bereich steht,
|*      nicht etwa, wenn er in einer Tabelle steht, die in einem spaltigen Bereich ist.
|*
|*************************************************************************/

sal_Bool lcl_IsInColSct( const SwFrm *pUp )
{
    sal_Bool bRet = sal_False;
    while( pUp )
    {
        if( pUp->IsColumnFrm() )
            bRet = sal_True;
        else if( pUp->IsSctFrm() )
            return bRet;
        else if( pUp->IsTabFrm() )
            return sal_False;
        pUp = pUp->GetUpper();
    }
    return sal_False;
}

/*************************************************************************
|*
|*    SwFrm::IsMoveable();
|*
|*    Ersterstellung    MA 09. Mar. 93
|*    Letzte Aenderung  MA 05. May. 95
|*
|*************************************************************************/
/** determine, if frame is moveable in given environment

    OD 08.08.2003 #110978#
    method replaced 'old' method <sal_Bool IsMoveable() const>.
    Determines, if frame is moveable in given environment. if no environment
    is given (parameter _pLayoutFrm == 0L), the movability in the actual
    environment (<this->GetUpper()) is checked.

    @author OD
*/

bool SwFrm::IsMoveable( const SwLayoutFrm* _pLayoutFrm ) const
{
    bool bRetVal = false;

    if ( !_pLayoutFrm )
    {
        _pLayoutFrm = GetUpper();
    }

    if ( _pLayoutFrm && IsFlowFrm() )
    {
        if ( _pLayoutFrm->IsInSct() && lcl_IsInColSct( _pLayoutFrm ) )
        {
            bRetVal = true;
        }
        else if ( _pLayoutFrm->IsInFly() ||
                  _pLayoutFrm->IsInDocBody() ||
                  _pLayoutFrm->IsInFtn() )
        {
            if ( _pLayoutFrm->IsInTab() && !IsTabFrm() &&
                 ( !IsCntntFrm() || !const_cast<SwFrm*>(this)->GetNextCellLeaf( MAKEPAGE_NONE ) ) )
            {
                bRetVal = false;
            }
            else
            {
                if ( _pLayoutFrm->IsInFly() )
                {
                    // if fly frame has a follow (next linked fly frame),
                    // frame is moveable.
                    if ( const_cast<SwLayoutFrm*>(_pLayoutFrm)->FindFlyFrm()->GetNextLink() )
                    {
                        bRetVal = true;
                    }
                    else
                    {
                        // if environment is columned, frame is moveable, if
                        // it isn't in last column.
                        // search for column frame
                        const SwFrm* pCol = _pLayoutFrm;
                        while ( pCol && !pCol->IsColumnFrm() )
                        {
                            pCol = pCol->GetUpper();
                        }
                        // frame is moveable, if found column frame isn't last one.
                        if ( pCol && pCol->GetNext() )
                        {
                            bRetVal = true;
                        }
                    }
                }
                else
                {
                    bRetVal = true;
                }
            }
        }
    }

    return bRetVal;
}

/*************************************************************************
|*
|*    SwFrm::SetInfFlags();
|*
|*    Ersterstellung    MA 05. Apr. 94
|*    Letzte Aenderung  MA 05. Apr. 94
|*
|*************************************************************************/
void SwFrm::SetInfFlags()
{
    if ( !IsFlyFrm() && !GetUpper() ) //noch nicht gepastet, keine Informationen
        return;                       //lieferbar

    bInfInvalid = bInfBody = bInfTab = bInfFly = bInfFtn = bInfSct = sal_False;

    SwFrm *pFrm = this;
    if( IsFtnContFrm() )
        bInfFtn = sal_True;
    do
    {   // bInfBody wird nur am Seitenbody, nicht im ColumnBody gesetzt
        if ( pFrm->IsBodyFrm() && !bInfFtn && pFrm->GetUpper()
             && pFrm->GetUpper()->IsPageFrm() )
            bInfBody = sal_True;
        else if ( pFrm->IsTabFrm() || pFrm->IsCellFrm() )
        {
            bInfTab = sal_True;
        }
        else if ( pFrm->IsFlyFrm() )
            bInfFly = sal_True;
        else if ( pFrm->IsSctFrm() )
            bInfSct = sal_True;
        else if ( pFrm->IsFtnFrm() )
            bInfFtn = sal_True;

        pFrm = pFrm->GetUpper();

    } while ( pFrm && !pFrm->IsPageFrm() ); //Oberhalb der Seite kommt nix
}

/*-----------------22.8.2001 14:30------------------
 * SwFrm::SetDirFlags( sal_Bool )
 * actualizes the vertical or the righttoleft-flags.
 * If the property is derived, it's from the upper or (for fly frames) from
 * the anchor. Otherwise we've to call a virtual method to check the property.
 * --------------------------------------------------*/

void SwFrm::SetDirFlags( sal_Bool bVert )
{
    if( bVert )
    {
        // OD 2004-01-21 #114969# - if derived, valid vertical flag only if
        // vertical flag of upper/anchor is valid.
        if( bDerivedVert )
        {
            const SwFrm* pAsk = IsFlyFrm() ?
                          ((SwFlyFrm*)this)->GetAnchorFrm() : GetUpper();

            ASSERT( pAsk != this, "Autsch! Stack overflow is about to happen" )

            if( pAsk )
            {
                bVertical = pAsk->IsVertical() ? 1 : 0;
                bReverse  = pAsk->IsReverse()  ? 1 : 0;
                if ( !pAsk->bInvalidVert )
                    bInvalidVert = sal_False;
            }
        }
        else
            CheckDirection( bVert );
    }
    else
    {
        sal_Bool bInv = 0;
        if( !bDerivedR2L ) // CheckDirection is able to set bDerivedR2L!
            CheckDirection( bVert );
        if( bDerivedR2L )
        {
            const SwFrm* pAsk = IsFlyFrm() ?
                          ((SwFlyFrm*)this)->GetAnchorFrm() : GetUpper();

            ASSERT( pAsk != this, "Autsch! Stack overflow is about to happen" )

            if( pAsk )
                bRightToLeft = pAsk->IsRightToLeft() ? 1 : 0;
            if( !pAsk || pAsk->bInvalidR2L )
                bInv = bInvalidR2L;
        }
        bInvalidR2L = bInv;
    }
}

SwLayoutFrm* SwFrm::GetNextCellLeaf( MakePageType )
{
    SwFrm* pTmpFrm = this;
    while ( !pTmpFrm->IsCellFrm() )
        pTmpFrm = pTmpFrm->GetUpper();

    ASSERT( pTmpFrm, "SwFrm::GetNextCellLeaf() without cell" )
    return ((SwCellFrm*)pTmpFrm)->GetFollowCell();
}

SwLayoutFrm* SwFrm::GetPrevCellLeaf( MakePageType )
{
    SwFrm* pTmpFrm = this;
    while ( !pTmpFrm->IsCellFrm() )
        pTmpFrm = pTmpFrm->GetUpper();

    ASSERT( pTmpFrm, "SwFrm::GetNextPreviousLeaf() without cell" )
    return ((SwCellFrm*)pTmpFrm)->GetPreviousCell();
}

SwCellFrm* lcl_FindCorrespondingCellFrm( const SwRowFrm& rOrigRow,
                                         const SwCellFrm& rOrigCell,
                                         const SwRowFrm& rCorrRow,
                                         bool bInFollow )
{
    SwCellFrm* pRet = NULL;
    SwCellFrm* pCell = (SwCellFrm*)rOrigRow.Lower();
    SwCellFrm* pCorrCell = (SwCellFrm*)rCorrRow.Lower();

    while ( pCell != &rOrigCell && !pCell->IsAnLower( &rOrigCell ) )
    {
        pCell = (SwCellFrm*)pCell->GetNext();
        pCorrCell = (SwCellFrm*)pCorrCell->GetNext();
    }

    ASSERT( pCell && pCorrCell, "lcl_FindCorrespondingCellFrm does not work" )

    if ( pCell != &rOrigCell )
    {
        // rOrigCell must be a lower of pCell. We need to recurse into the rows:
        ASSERT( pCell->Lower() && pCell->Lower()->IsRowFrm(),
                "lcl_FindCorrespondingCellFrm does not work" )

        SwRowFrm* pRow = (SwRowFrm*)pCell->Lower();
        while ( !pRow->IsAnLower( &rOrigCell ) )
            pRow = (SwRowFrm*)pRow->GetNext();

        SwRowFrm* pCorrRow = 0;
        if ( bInFollow )
            pCorrRow = pRow->GetFollowRow();
        else
        {
            SwRowFrm* pTmpRow = static_cast<SwRowFrm*>(pCorrCell->GetLastLower());

            if ( pTmpRow && pTmpRow->GetFollowRow() == pRow )
                pCorrRow = pTmpRow;
        }

        if ( pCorrRow )
            pRet = lcl_FindCorrespondingCellFrm( *pRow, rOrigCell, *pCorrRow, bInFollow );
    }
    else
        pRet = pCorrCell;

    return pRet;
}

// VERSION OF GetFollowCell() that assumes that we always have a follow flow line:
SwCellFrm* SwCellFrm::GetFollowCell() const
{
    SwCellFrm* pRet = NULL;

    // NEW TABLES
    // Covered cells do not have follow cells!
    const long nRowSpan = GetLayoutRowSpan();
    if ( nRowSpan < 1 )
        return NULL;

    // find most upper row frame
    const SwFrm* pRow = GetUpper();
    while( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() )
        pRow = pRow->GetUpper();

    if ( !pRow )
        return NULL;

    const SwTabFrm* pTabFrm = static_cast<const SwTabFrm*>( pRow->GetUpper() );
    if ( !pRow || !pTabFrm->GetFollow() || !pTabFrm->HasFollowFlowLine() )
        return NULL;

    const SwCellFrm* pThisCell = this;

    // Get last cell of the current table frame that belongs to the rowspan:
    if ( nRowSpan > 1 )
    {
        // optimization: Will end of row span be in last row or exceed row?
        long nMax = 0;
        while ( pRow->GetNext() && ++nMax < nRowSpan )
            pRow = pRow->GetNext();

        if ( !pRow->GetNext() )
        {
            pThisCell = &pThisCell->FindStartEndOfRowSpanCell( false, true );
            pRow = pThisCell->GetUpper();
        }
    }

    const SwRowFrm* pFollowRow = NULL;
    if ( !pRow->GetNext() &&
         NULL != ( pFollowRow = pRow->IsInSplitTableRow() ) &&
         ( !pFollowRow->IsRowSpanLine() || nRowSpan > 1 ) )
         pRet = lcl_FindCorrespondingCellFrm( *((SwRowFrm*)pRow), *pThisCell, *pFollowRow, true );

    return pRet;
}

// VERSION OF GetPreviousCell() THAT ASSUMES THAT WE ALWAYS HAVE A FFL
SwCellFrm* SwCellFrm::GetPreviousCell() const
{
    SwCellFrm* pRet = NULL;

    // NEW TABLES
    // Covered cells do not have previous cells!
    if ( GetLayoutRowSpan() < 1 )
        return NULL;

    // find most upper row frame
    const SwFrm* pRow = GetUpper();
    while( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() )
        pRow = pRow->GetUpper();

    ASSERT( pRow->GetUpper() && pRow->GetUpper()->IsTabFrm(), "GetPreviousCell without Table" );

    SwTabFrm* pTab = (SwTabFrm*)pRow->GetUpper();

    if ( pTab->IsFollow() )
    {
        const SwFrm* pTmp = pTab->GetFirstNonHeadlineRow();
        const bool bIsInFirstLine = ( pTmp == pRow );

        if ( bIsInFirstLine )
        {
            SwTabFrm *pMaster = (SwTabFrm*)pTab->FindMaster();
            if ( pMaster && pMaster->HasFollowFlowLine() )
            {
                SwRowFrm* pMasterRow = static_cast<SwRowFrm*>(pMaster->GetLastLower());
                if ( pMasterRow )
                    pRet = lcl_FindCorrespondingCellFrm( *((SwRowFrm*)pRow), *this, *pMasterRow, false );
                if ( pRet && pRet->GetTabBox()->getRowSpan() < 1 )
                    pRet = &const_cast<SwCellFrm&>(pRet->FindStartEndOfRowSpanCell( true, true ));
            }
        }
    }

    return pRet;
}

// --> NEW TABLES
const SwCellFrm& SwCellFrm::FindStartEndOfRowSpanCell( bool bStart, bool bCurrentTableOnly ) const
{
    const SwCellFrm* pRet = 0;

    const SwTabFrm* pTableFrm = dynamic_cast<const SwTabFrm*>(GetUpper()->GetUpper());

    if ( !bStart && pTableFrm->IsFollow() && pTableFrm->IsInHeadline( *this ) )
        return *this;

    ASSERT( pTableFrm &&
            (  bStart && GetTabBox()->getRowSpan() < 1 ||
              !bStart && GetLayoutRowSpan() > 1 ),
            "SwCellFrm::FindStartRowSpanCell: No rowspan, no table, no cookies" )

    if ( pTableFrm )
    {
        const SwTable* pTable = pTableFrm->GetTable();

        sal_uInt16 nMax = USHRT_MAX;
        if ( bCurrentTableOnly )
        {
            const SwFrm* pCurrentRow = GetUpper();
            const bool bDoNotEnterHeadline = bStart && pTableFrm->IsFollow() &&
                                        !pTableFrm->IsInHeadline( *pCurrentRow );

            // check how many rows we are allowed to go up or down until we reach the end of
            // the current table frame:
            nMax = 0;
            while ( bStart ? pCurrentRow->GetPrev() : pCurrentRow->GetNext() )
            {
                if ( bStart )
                {
                    // do not enter a repeated headline:
                    if ( bDoNotEnterHeadline && pTableFrm->IsFollow() &&
                         pTableFrm->IsInHeadline( *pCurrentRow->GetPrev() ) )
                        break;

                    pCurrentRow = pCurrentRow->GetPrev();
                }
                else
                    pCurrentRow = pCurrentRow->GetNext();

                ++nMax;
            }
        }

        // By passing the nMax value for Find*OfRowSpan (in case of bCurrentTableOnly
        // is set) we assure that we find a rMasterBox that has a SwCellFrm in
        // the current table frame:
        const SwTableBox& rMasterBox = bStart ?
                                       GetTabBox()->FindStartOfRowSpan( *pTable, nMax ) :
                                       GetTabBox()->FindEndOfRowSpan( *pTable, nMax );

        SwIterator<SwCellFrm,SwFmt> aIter( *rMasterBox.GetFrmFmt() );

        for ( SwCellFrm* pMasterCell = aIter.First(); pMasterCell; pMasterCell = aIter.Next() )
        {
            if ( pMasterCell->GetTabBox() == &rMasterBox )
            {
                const SwTabFrm* pMasterTable = static_cast<const SwTabFrm*>(pMasterCell->GetUpper()->GetUpper());

                if ( bCurrentTableOnly )
                {
                    if ( pMasterTable == pTableFrm )
                    {
                        pRet = pMasterCell;
                        break;
                    }
                }
                else
                {
                    if ( pMasterTable == pTableFrm ||
                         (  (bStart && pMasterTable->IsAnFollow(pTableFrm)) ||
                           (!bStart && pTableFrm->IsAnFollow(pMasterTable)) ) )
                    {
                        pRet = pMasterCell;
                        break;
                    }
                }
            }
        }
    }

    ASSERT( pRet, "SwCellFrm::FindStartRowSpanCell: No result" )

    return *pRet;
}
// <-- NEW TABLES

const SwRowFrm* SwFrm::IsInSplitTableRow() const
{
    ASSERT( IsInTab(), "IsInSplitTableRow should only be called for frames in tables" )

    const SwFrm* pRow = this;

    // find most upper row frame
    while( pRow && ( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() ) )
        pRow = pRow->GetUpper();

    if ( !pRow ) return NULL;

    ASSERT( pRow->GetUpper()->IsTabFrm(), "Confusion in table layout" )

    const SwTabFrm* pTab = (SwTabFrm*)pRow->GetUpper();
    // --> OD 2006-06-28 #b6443897#
    // If most upper row frame is a headline row, the current frame
    // can't be in a splitted table row. Thus, add corresponding condition.
    if ( pRow->GetNext() ||
         pTab->GetTable()->IsHeadline(
                    *(static_cast<const SwRowFrm*>(pRow)->GetTabLine()) ) ||
         !pTab->HasFollowFlowLine() ||
         !pTab->GetFollow() )
        return NULL;
    // <--

    // skip headline
    const SwRowFrm* pFollowRow = pTab->GetFollow()->GetFirstNonHeadlineRow();

    ASSERT( pFollowRow, "SwFrm::IsInSplitTableRow() does not work" )

    return pFollowRow;
}

const SwRowFrm* SwFrm::IsInFollowFlowRow() const
{
    ASSERT( IsInTab(), "IsInSplitTableRow should only be called for frames in tables" )

    // find most upper row frame
    const SwFrm* pRow = this;
    while( pRow && ( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() ) )
        pRow = pRow->GetUpper();

    if ( !pRow ) return NULL;

    ASSERT( pRow->GetUpper()->IsTabFrm(), "Confusion in table layout" )

    const SwTabFrm* pTab = (SwTabFrm*)pRow->GetUpper();

    const SwTabFrm* pMaster = pTab->IsFollow() ? pTab->FindMaster() : 0;

    if ( !pMaster || !pMaster->HasFollowFlowLine() )
        return NULL;

    const SwFrm* pTmp = pTab->GetFirstNonHeadlineRow();
    const bool bIsInFirstLine = ( pTmp == pRow );

    if ( !bIsInFirstLine )
        return NULL;

    const SwRowFrm* pMasterRow = static_cast<const SwRowFrm*>(pMaster->GetLastLower());
    return pMasterRow;
}

bool SwFrm::IsInBalancedSection() const
{
    bool bRet = false;

    if ( IsInSct() )
    {
        const SwSectionFrm* pSectionFrm = FindSctFrm();
        if ( pSectionFrm )
            bRet = pSectionFrm->IsBalancedSection();
    }
    return bRet;
}

/*
 * SwLayoutFrm::GetLastLower()
 */
const SwFrm* SwLayoutFrm::GetLastLower() const
{
    const SwFrm* pRet = Lower();
    if ( !pRet )
        return 0;
    while ( pRet->GetNext() )
        pRet = pRet->GetNext();
    return pRet;
}
