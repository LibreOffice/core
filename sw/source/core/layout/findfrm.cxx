/*************************************************************************
 *
 *  $RCSfile: findfrm.cxx,v $
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
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "node.hxx"
#include "doc.hxx"
#include "frmtool.hxx"
#include "flyfrm.hxx"

#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#include "tabfrm.hxx"
#include "sectfrm.hxx"
#include "flyfrms.hxx"
#include "ftnfrm.hxx"
#include "txtftn.hxx"
#include "fmtftn.hxx"

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
            pUp = ((SwFlyFrm*)pUp)->GetAnchor();
        else
            pUp = pUp->GetUpper();
    }
    return FALSE;
}

/*************************************************************************
|*
|*  SwFrm::GetPrevLayoutLeaf()
|*
|*  Beschreibung        Findet das vorhergehende Layout-Blatt. Ein Layout-
|*      Blatt ist ein LayoutFrm, der keinen LayoutFrm in seinen Unterbaum hat;
|*      genau gesagt, darf pLower kein LayoutFrm sein.
|*      Anders ausgedrueckt: pLower darf 0 sein oder auf einen CntntFrm
|*      zeigen.
|*      pLower darf allerdings auf einen TabFrm zeigen, denn diese stehen
|*      direkt neben den CntntFrms.
|*  Ersterstellung      MA 29. May. 92
|*  Letzte Aenderung    MA 30. Oct. 97
|*
|*************************************************************************/
const SwFrm * MA_FASTCALL lcl_LastLower( const SwFrm *pFrm )
{
    const SwFrm *pLower = pFrm->GetLower();
    if ( pLower )
        while ( pLower->GetNext() )
            pLower = pLower->GetNext();
    return pLower;
}

const SwLayoutFrm *SwFrm::GetPrevLayoutLeaf() const
{
    const SwFrm       *pFrm = this;
    const SwLayoutFrm *pLayoutFrm = 0;
    const SwFrm       *p;
    FASTBOOL bGoingUp = TRUE;
    do {
        FASTBOOL bGoingBwd = FALSE, bGoingDown = FALSE;
        if( !(bGoingDown = (!bGoingUp && ( 0 != (p = ::lcl_LastLower( pFrm ))))) &&
            !(bGoingBwd = (0 != (p = pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->GetPrevLink()
                                                      : pFrm->GetPrev()))) &&
            !(bGoingUp = (0 != (p = pFrm->GetUpper()))))
            return 0;
        bGoingUp = !( bGoingBwd || bGoingDown );
        pFrm = p;
        p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0;
    } while( (p && !p->IsFlowFrm()) ||
             pFrm == this ||
             0 == (pLayoutFrm = pFrm->IsLayoutFrm() ? (SwLayoutFrm*)pFrm:0) ||
             pLayoutFrm->IsAnLower( this ) );

    return pLayoutFrm;
}
/*************************************************************************
|*
|*  SwFrm::GetNextLayoutLeaf
|*
|*  Beschreibung        Findet das naechste Layout-Blatt. Ein Layout-Blatt
|*          ist ein LayoutFrm, der kein LayoutFrm in seinen Unterbaum hat;
|*          genau gesagt, darf pLower kein LayoutFrm sein.
|*          Anders ausgedrueckt: pLower darf 0 sein oder auf einen CntntFrm
|*          zeigen.
|*          pLower darf allerdings auf einen TabFrm zeigen, denn diese stehen
|*          direkt neben den CntntFrms.
|*  Ersterstellung      MA 13. May. 92
|*  Letzte Aenderung    MA 30. Oct. 97
|*
|*************************************************************************/
const SwLayoutFrm *SwFrm::GetNextLayoutLeaf() const
{
    const SwFrm       *pFrm = this;
    const SwLayoutFrm *pLayoutFrm = 0;
    const SwFrm       *p;
    FASTBOOL bGoingUp = FALSE;
    do {
        FASTBOOL bGoingFwd = FALSE, bGoingDown = FALSE;
        if( !(bGoingDown = (!bGoingUp && ( 0 !=
            (p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0)))) &&
            !(bGoingFwd = (0 != (p = pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->GetNextLink()
                                                      : pFrm->GetNext()))) &&
            !(bGoingUp = (0 != (p = pFrm->GetUpper()))))
            return 0;
        bGoingUp = !( bGoingFwd || bGoingDown );
        pFrm = p;
        p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0;
    } while( (p && !p->IsFlowFrm()) ||
             pFrm == this ||
             0 == (pLayoutFrm = pFrm->IsLayoutFrm() ? (SwLayoutFrm*)pFrm:0 ) ||
             pLayoutFrm->IsAnLower( this ) );

    return pLayoutFrm;
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
    SwDoc *pDoc = IsLayoutFrm() ? ((SwLayoutFrm*)this)->GetFmt()->GetDoc()
                                : ((SwCntntFrm*)this)->GetNode()->GetDoc();
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
            if ( ((SwFlyFrm*)pRet)->IsFlyFreeFrm() )
                pRet = ((SwFlyFreeFrm*)pRet)->GetPage();
            else
                pRet = ((SwFlyFrm*)pRet)->GetAnchor();
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
            if ( ((SwFlyFrm*)pRet)->IsFlyFreeFrm() )
                pRet = ((SwFlyFreeFrm*)pRet)->GetPage();
            else
                pRet = ((SwFlyFrm*)pRet)->GetAnchor();
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
            pRet = ((SwFlyFrm*)pRet)->GetAnchor();
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
        if( !(bGoingDown = (!bGoingUp && ( 0 !=
             (p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0)))) &&
            !(bGoingFwd = (0 != (p = pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->GetNextLink()
                                                      : pFrm->GetNext()))) &&
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
        SwFrm *pNxt = lcl_NextFrm( pThis );
        if ( pUp->IsAnLower( pNxt ) )
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
                    BOOL bEndn = IsInSct() && !IsSctFrm();
                    if ( ( bBody && pNxtCnt->IsInDocBody() ) ||
                         ( pNxtCnt->IsInFtn() && ( bFtn || ( bEndn && pNxtCnt->
                           FindFtnFrm()->GetAttr()->GetFtn().IsEndNote() ) ) ) )
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
        pThis = ((SwTabFrm*)this)->ContainsCntnt();
        bIgnoreTab = TRUE;
    }

    if ( pThis->IsCntntFrm() )
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
                        pTmp->_InvalidatePos();
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
                        pTmp->_InvalidatePos();
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
BOOL SwFrm::IsMoveable() const
{
    if ( IsFlowFrm() )
    {
        if( IsInSct() && lcl_IsInColSct( GetUpper() ) )
            return TRUE;
        if( IsInFly() || IsInDocBody() || IsInFtn() || IsInSct() )
        {
            if ( IsInTab() && !IsTabFrm() )
                return FALSE;
            BOOL bRet = TRUE;
            if ( IsInFly() )
            {
                //Wenn der Fly noch einen Follow hat ist der Inhalt auf jeden
                //Fall moveable
                if ( !((SwFlyFrm*)FindFlyFrm())->GetNextLink() )
                {
                    //Fuer Inhalt innerhab von Spaltigen Rahmen ist nur der Inhalt
                    //der letzten Spalte nicht moveable.
                    const SwFrm *pCol = GetUpper();
                    while ( pCol && !pCol->IsColumnFrm() )
                        pCol = pCol->GetUpper();
                    if ( !pCol || (pCol && !pCol->GetNext()) )
                        bRet = FALSE;
                }
            }
            return bRet;
        }
    }
    return FALSE;
}

/*************************************************************************
|*
|*    SwFrm::ImplGetNextCntntFrm(), ImplGetPrevCntntFrm()
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
const SwCntntFrm *SwCntntFrm::ImplGetNextCntntFrm() const
{
    const SwFrm *pFrm = this;
    const SwCntntFrm *pCntntFrm = 0;
    FASTBOOL bGoingUp = FALSE;
    do {
        const SwFrm *p;
        FASTBOOL bGoingFwd = FALSE, bGoingDown = FALSE;
        if( !(bGoingDown = (!bGoingUp && ( 0 !=
             (p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0)))) &&
            !(bGoingFwd = (0 != (p = pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->GetNextLink()
                                                      : pFrm->GetNext()))) &&
            !(bGoingUp = (0 != (p = pFrm->GetUpper()))))
            return 0;
        bGoingUp = !(bGoingFwd || bGoingDown);
        pFrm = p;
    } while ( 0 == (pCntntFrm = (pFrm->IsCntntFrm() ? (SwCntntFrm*)pFrm:0) ));
    return pCntntFrm;

}

const SwCntntFrm *SwCntntFrm::ImplGetPrevCntntFrm() const
{
    const SwFrm *pFrm = this;
    const SwCntntFrm *pCntntFrm = 0;
    FASTBOOL bGoingUp = FALSE;
    do {
        const SwFrm *p;
        FASTBOOL bGoingBack = FALSE, bGoingDown = FALSE;
        if( !(bGoingDown = (!bGoingUp && (0 !=
             (p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0)))) &&
            !(bGoingBack = (0 != (p = pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->GetPrevLink()
                                                       : pFrm->GetPrev()))) &&
            !(bGoingUp = (0 != (p = pFrm->GetUpper()))))
            return 0;
        bGoingUp = !(bGoingBack || bGoingDown);
        if( bGoingDown && p )
            while ( p->GetNext() )
                p = p->GetNext();
        pFrm = p;
    } while ( 0 == (pCntntFrm = (pFrm->IsCntntFrm() ? (SwCntntFrm*)pFrm:0) ));
    return pCntntFrm;
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
#ifndef PRODUCT
    BOOL bIsInTab = FALSE;
#endif

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
#ifndef PRODUCT
            if( pFrm->IsTabFrm() )
            {
                ASSERT( !bIsInTab, "Table in table: Not implemented." );
                bIsInTab = TRUE;
            }
#endif
            bInfTab = TRUE;
        }
        else if ( pFrm->IsFlyFrm() )
            bInfFly = TRUE;
        else if ( pFrm->IsSctFrm() )
            bInfSct = TRUE;
        else if ( pFrm->IsFtnFrm() )
            bInfFtn = TRUE;

//MA: 06. Apr. 94, oberhalb eines Fly geht es nicht weiter!
//      if ( pFrm->IsFlyFrm() )
//          pFrm = ((SwFlyFrm*)pFrm)->GetAnchor();
//      else
            pFrm = pFrm->GetUpper();

    } while ( pFrm && !pFrm->IsPageFrm() ); //Oberhalb der Seite kommt nix
}

/*************************************************************************
|*
|*    SwFrm::HasFixSize()
|*
|*    Ersterstellung    MA 08. Apr. 94
|*    Letzte Aenderung  MA 08. Apr. 94
|*
|*************************************************************************/
BOOL SwFrm::HasFixSize( const SzPtr pSize ) const
{
    if ( pFIXSIZE == pSize )
        return TRUE;

    SzPtr pVar = pVARSIZE;
    return ((pSize == pHeight) && bFixHeight && pVar == pHeight) ||
           ((pSize == pWidth) && bFixWidth && pVar == pWidth);
}


