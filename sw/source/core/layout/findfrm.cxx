/*************************************************************************
 *
 *  $RCSfile: findfrm.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:08:25 $
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
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "node.hxx"
#include "doc.hxx"
#include "frmtool.hxx"
#include "flyfrm.hxx"

#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _CELLFRM_HXX //autogen
#include <cellfrm.hxx>
#endif
#ifndef _ROWFRM_HXX
#include <rowfrm.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif

#include "tabfrm.hxx"
#include "sectfrm.hxx"
#include "flyfrms.hxx"
#include "ftnfrm.hxx"
#include "txtftn.hxx"
#include "fmtftn.hxx"
// OD 09.01.2004 #i11859#
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>   // SwTxtFrm
#endif

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

const SwFrm *SwLayoutFrm::ContainsAny() const
{
    //LayoutBlatt nach unten hin suchen und wenn dieses keinen Inhalt hat
    //solange die weiteren Blatter abklappern bis Inhalt gefunden oder der
    //this verlassen wird.
    // Oder bis wir einen SectionFrm oder TabFrm gefunden haben

    const SwLayoutFrm *pLayLeaf = this;
    BOOL bNoFtn = IsSctFrm();
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
#ifdef USED
            const SwCntntFrm *pCnt = pLayLeaf->ContainsCntnt();
            if( pCnt )
                return pLayLeaf;
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
#endif
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
BOOL SwLayoutFrm::IsAnLower( const SwFrm *pAssumed ) const
{
    const SwFrm *pUp = pAssumed;
    while ( pUp )
    {
        if ( pUp == this )
            return TRUE;
        if ( pUp->IsFlyFrm() )
            pUp = ((SwFlyFrm*)pUp)->GetAnchorFrm();
        else
            pUp = pUp->GetUpper();
    }
    return FALSE;
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

const SwFrm* lcl_FindLayoutFrame( const SwFrm* pFrm, bool bNext, bool bFollowCell )
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
    if ( bFwd )
        return pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0;

    const SwFrm *pLower = pFrm->GetLower();
    if ( pLower )
        while ( pLower->GetNext() )
            pLower = pLower->GetNext();
    return pLower;
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
    const SwFrm       *p;
    bool bGoingUp = !bFwd;          // false for forward, true for backward
    do {

         bool bGoingFwdOrBwd = false, bGoingDown = false;

         bGoingDown = ( !bGoingUp && ( 0 != (p = lcl_GetLower( pFrm, bFwd ) ) ) );
         if ( !bGoingDown )
         {
             // I cannot go down, because either I'm currently going up or
             // because the is no lower.
             // I'll try to go forward:
             bGoingFwdOrBwd = (0 != (p = lcl_FindLayoutFrame( pFrm, bFwd, true ) ) );
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
    FASTBOOL bGoingUp = FALSE;
    do {
        const SwFrm *p;
        FASTBOOL bGoingFwdOrBwd = FALSE, bGoingDown = FALSE;

        bGoingDown = ( !bGoingUp && ( 0 != ( p = lcl_GetLower( pFrm, true ) ) ) );
        if ( !bGoingDown )
        {
            bGoingFwdOrBwd = ( 0 != ( p = lcl_FindLayoutFrame( pFrm, bFwd, false ) ) );
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
SwRootFrm* SwFrm::FindRootFrm()
{
    // MIB: A layout frame is always registerd at a SwFrmFmt and a content
    // frame alyways at a SwCntntNode. For any other case we won't find
    // a root frame.
    // Casting the GetDep() result instead of the frame itself (that has
    // been done before) makes it save to use that method in constructors
    // and destructors.
    ASSERT( GetDep(), "frame is not registered any longer" );
    ASSERT( IsLayoutFrm() || IsCntntFrm(), "invalid frame type" );
    SwDoc *pDoc = IsLayoutFrm()
                        ? static_cast < SwFrmFmt * >( GetDep() )->GetDoc()
                        : static_cast < SwCntntNode * >( GetDep() )->GetDoc();
    return pDoc->GetRootFrm();
}

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

SwFtnBossFrm* SwFrm::FindFtnBossFrm( BOOL bFootnotes )
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
            return pSct->FindFtnBossFrm( TRUE );
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

SwSectionFrm* SwFrm::ImplFindTopSctFrm()
{
    SwFrm *pRet = 0;
    SwFrm *pTmp = this;
    do
    {   if( pTmp->IsSctFrm() )
            pRet = pTmp;
       pTmp = pTmp->GetUpper();
    } while ( pTmp && pTmp->IsInSct() );
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

const SwFtnFrm* SwFtnContFrm::FindEndNote() const
{
    const SwFtnFrm* pRet = (SwFtnFrm*)Lower();
    while( pRet && !pRet->GetAttr()->GetFtn().IsEndNote() )
        pRet = (SwFtnFrm*)pRet->GetNext();
    return pRet;
}

BOOL SwRootFrm::IsPageAtPos( const Point &rPt ) const
{
    if( !Frm().IsInside( rPt ) )
        return FALSE;
    const SwFrm* pPage = Lower();
    while( pPage && rPt.Y() > pPage->Frm().Bottom() )
        pPage = pPage->GetNext();
    return pPage && pPage->Frm().IsInside( rPt );
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

SwAttrSet* SwFrm::GetAttrSet()
{
    if ( IsCntntFrm() )
        return &((SwCntntFrm*)this)->GetNode()->GetSwAttrSet();
    else
        return (SwAttrSet*)&((SwLayoutFrm*)this)->GetFmt()->GetAttrSet();
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
    FASTBOOL bGoingUp = FALSE;
    do {
        SwFrm *p;
        FASTBOOL bGoingFwd = FALSE, bGoingDown = FALSE;
        if( !(bGoingDown = (!bGoingUp && ( 0 != (p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0)))) &&
            !(bGoingFwd = (0 != (p = ( pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->GetNextLink() : pFrm->GetNext())))) &&
            !(bGoingUp = (0 != (p = pFrm->GetUpper()))))
            return 0;
        bGoingUp = !(bGoingFwd || bGoingDown);
        pFrm = p;
    } while ( 0 == (pRet = ( ( pFrm->IsCntntFrm() || ( !bGoingUp &&
            ( pFrm->IsTabFrm() || pFrm->IsSctFrm() ) ) )? pFrm : 0 ) ) );
    return pRet;
}

SwFrm *SwFrm::_FindNext()
{
    BOOL bIgnoreTab = FALSE;
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
        bIgnoreTab = TRUE;
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
        SwFrm* pUpper = GetUpper();
        if ( pUpper->IsTabFrm() && ((SwTabFrm*)pUpper)->GetFollow() )
            return ((SwTabFrm*)pUpper)->GetFollow()->GetLower();
        else return NULL;
    }
    else
        return NULL;

    SwFrm* pRet = NULL;
    const BOOL bFtn  = pThis->IsInFtn();
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
        const BOOL bBody = pThis->IsInDocBody();
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

SwCntntFrm *SwFrm::_FindNextCnt()
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
        const BOOL bBody = pThis->IsInDocBody();
        const BOOL bFtn  = pThis->IsInFtn();
        SwCntntFrm *pNxtCnt = ((SwCntntFrm*)pThis)->GetNextCntntFrm();
        if ( pNxtCnt )
        {
            if ( bBody || bFtn )
            {
                while ( pNxtCnt )
                {
                    if ( (bBody && pNxtCnt->IsInDocBody()) ||
                         (bFtn  && pNxtCnt->IsInFtn()) )
                        return pNxtCnt;
                    pNxtCnt = pNxtCnt->GetNextCntntFrm();
                }
            }
            else if ( pThis->IsInFly() )
                return pNxtCnt;
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
                    return pNxtCnt;
            }
        }
    }
    return 0;
}

SwFrm *SwFrm::_FindPrev()
{
    BOOL bIgnoreTab = FALSE;
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
        bIgnoreTab = TRUE;
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
            const BOOL bBody = pThis->IsInDocBody();
            const BOOL bFtn  = bBody ? FALSE : pThis->IsInFtn();
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

void SwFrm::ImplInvalidateNextPos( BOOL bNoFtn )
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
|*      liefert nur TRUE, wenn der Frame _direkt_ in einem spaltigen Bereich steht,
|*      nicht etwa, wenn er in einer Tabelle steht, die in einem spaltigen Bereich ist.
|*
|*************************************************************************/

BOOL lcl_IsInColSct( const SwFrm *pUp )
{
    BOOL bRet = FALSE;
    while( pUp )
    {
        if( pUp->IsColumnFrm() )
            bRet = TRUE;
        else if( pUp->IsSctFrm() )
            return bRet;
        else if( pUp->IsTabFrm() )
            return FALSE;
        pUp = pUp->GetUpper();
    }
    return FALSE;
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
    method replaced 'old' method <BOOL IsMoveable() const>.
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
                 ( !IsCntntFrm() || ( NULL == IsInSplitTableRow() ) ) )
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

    bInfInvalid = bInfBody = bInfTab = bInfFly = bInfFtn = bInfSct = FALSE;

    SwFrm *pFrm = this;
    if( IsFtnContFrm() )
        bInfFtn = TRUE;
    do
    {   // bInfBody wird nur am Seitenbody, nicht im ColumnBody gesetzt
        if ( pFrm->IsBodyFrm() && !bInfFtn && pFrm->GetUpper()
             && pFrm->GetUpper()->IsPageFrm() )
            bInfBody = TRUE;
        else if ( pFrm->IsTabFrm() || pFrm->IsCellFrm() )
        {
            bInfTab = TRUE;
        }
        else if ( pFrm->IsFlyFrm() )
            bInfFly = TRUE;
        else if ( pFrm->IsSctFrm() )
            bInfSct = TRUE;
        else if ( pFrm->IsFtnFrm() )
            bInfFtn = TRUE;

        pFrm = pFrm->GetUpper();

    } while ( pFrm && !pFrm->IsPageFrm() ); //Oberhalb der Seite kommt nix
}

/*-----------------22.8.2001 14:30------------------
 * SwFrm::SetDirFlags( BOOL )
 * actualizes the vertical or the righttoleft-flags.
 * If the property is derived, it's from the upper or (for fly frames) from
 * the anchor. Otherwise we've to call a virtual method to check the property.
 * --------------------------------------------------*/

void SwFrm::SetDirFlags( BOOL bVert )
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
                    bInvalidVert = FALSE;
            }
        }
        else
            CheckDirection( bVert );
    }
    else
    {
        BOOL bInv = 0;
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

SwLayoutFrm* SwFrm::GetNextCellLeaf( MakePageType eMakePage )
{
    SwFrm* pTmpFrm = this;
    while ( !pTmpFrm->IsCellFrm() )
        pTmpFrm = pTmpFrm->GetUpper();

    ASSERT( pTmpFrm, "SwFrm::GetNextCellLeaf() without cell" )
    return ((SwCellFrm*)pTmpFrm)->GetFollowCell();
}

SwLayoutFrm* SwFrm::GetPrevCellLeaf( MakePageType eMakePage )
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
            SwRowFrm* pTmpRow = (SwRowFrm*)pCorrCell->Lower();
            while ( pTmpRow->GetNext() )
                pTmpRow = (SwRowFrm*)pTmpRow->GetNext();

            if ( pTmpRow->GetFollowRow() == pRow )
                pCorrRow = pTmpRow;
        }

        if ( pCorrRow )
            pRet = lcl_FindCorrespondingCellFrm( *pRow, rOrigCell, *pCorrRow, bInFollow );
    }
    else
        pRet = pCorrCell;

    return pRet;
}


SwCellFrm* SwCellFrm::GetFollowCell() const
{
    SwCellFrm* pRet = NULL;

    // find most upper row frame
    const SwFrm* pRow = GetUpper();
    while( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() )
        pRow = pRow->GetUpper();

    const SwRowFrm* pFollowRow = NULL;
    if ( !pRow->GetNext() &&
         NULL != ( pFollowRow = pRow->IsInSplitTableRow() ) )
        pRet = lcl_FindCorrespondingCellFrm( *((SwRowFrm*)pRow), *this, *pFollowRow, true );

    return pRet;
}


SwCellFrm* SwCellFrm::GetPreviousCell() const
{
    SwCellFrm* pRet = NULL;

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
                SwRowFrm* pMasterRow = (SwRowFrm*)pMaster->Lower();
                while ( pMasterRow->GetNext() )
                {
                    pMasterRow = (SwRowFrm*)pMasterRow->GetNext();
                }

                pRet = lcl_FindCorrespondingCellFrm( *((SwRowFrm*)pRow), *this, *pMasterRow, false );
            }
        }
    }

    return pRet;
}


const SwRowFrm* SwFrm::IsInSplitTableRow() const
{
    ASSERT( IsInTab(), "IsInSplitTableRow should only be called for frames in tables" )

    // find most upper row frame
    const SwFrm* pRow = this;
    while( pRow && ( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() ) )
        pRow = pRow->GetUpper();

    if ( !pRow ) return NULL;

    ASSERT( pRow->GetUpper()->IsTabFrm(), "Confusion in table layout" )

    const SwTabFrm* pTab = (SwTabFrm*)pRow->GetUpper();
    if ( pRow->GetNext() ||
        !pTab->HasFollowFlowLine() ||
        !pTab->GetFollow() )
        return NULL;

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

    SwRowFrm* pMasterRow = (SwRowFrm*)pMaster->Lower();

    while ( pMasterRow && pMasterRow->GetNext() )
        pMasterRow = (SwRowFrm*)pMasterRow->GetNext();

    return pMasterRow;
}


