/*************************************************************************
 *
 *  $RCSfile: sectfrm.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 13:07:51 $
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

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _HINTS_HXX //autogen
#include <hints.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#include "sectfrm.hxx"
#include "section.hxx"      // SwSection
#include "frmtool.hxx"      // StackHack
#include "doc.hxx"          // SwDoc
#include "cntfrm.hxx"       // SwCntntFrm
#include "rootfrm.hxx"      // SwRootFrm
#include "pagefrm.hxx"      // SwPageFrm
#include "fmtpdsc.hxx"      // SwFmtPageDesc
#include "fmtcntnt.hxx"     // SwFmtCntnt
#include "ndindex.hxx"      // SwNodeIndex
#include "ftnidx.hxx"
#include "txtfrm.hxx"       // SwTxtFrm
#include "fmtclds.hxx"      // SwFmtCol
#include "colfrm.hxx"       // SwColumnFrm
#include "tabfrm.hxx"       // SwTabFrm
#include "flyfrm.hxx"       // SwFlyFrm
#include "ftnfrm.hxx"       // SwFtnFrm
#include "layouter.hxx"     // SwLayouter
#include "dbg_lay.hxx"
#include "viewsh.hxx"
#include "viewimp.hxx"
#include "frmsh.hxx"
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _FMTFTNTX_HXX //autogen
#include <fmtftntx.hxx>
#endif

SV_IMPL_PTRARR_SORT( SwDestroyList, SwSectionFrmPtr )

/*************************************************************************
|*
|*  SwSectionFrm::SwSectionFrm(), ~SwSectionFrm()
|*
|*  Ersterstellung      AMA 26. Nov. 97
|*  Letzte Aenderung    AMA 26. Nov. 97
|*
|*************************************************************************/
SwSectionFrm::SwSectionFrm( SwSection &rSect ) :
    SwLayoutFrm( rSect.GetFmt() ),
    SwFlowFrm( (SwFrm&)*this ),
    pSection( &rSect )
{
    nType = FRMC_SECTION;

    CalcFtnAtEndFlag();
    CalcEndAtEndFlag();
}

SwSectionFrm::SwSectionFrm( SwSectionFrm &rSect, BOOL bMaster ) :
    SwLayoutFrm( rSect.GetFmt() ),
    SwFlowFrm( (SwFrm&)*this ),
    pSection( rSect.GetSection() )
{
    bFtnAtEnd = rSect.IsFtnAtEnd();
    bEndnAtEnd = rSect.IsEndnAtEnd();
    bLockJoin = FALSE;
    nType = FRMC_SECTION;

    PROTOCOL( this, PROT_SECTION, bMaster ? ACT_CREATE_MASTER : ACT_CREATE_FOLLOW, &rSect )

    if( bMaster )
    {
        if( rSect.IsFollow() )
        {
            SwSectionFrm* pMaster = rSect.FindMaster();
            pMaster->SetFollow( this );
            bIsFollow = TRUE;
        }
        else
            rSect.bIsFollow = TRUE;
        SetFollow( &rSect );
    }
    else
    {
        bIsFollow = TRUE;
        SetFollow( rSect.GetFollow() );
        rSect.SetFollow( this );
        if( !GetFollow() )
            rSect.SimpleFormat();
        if( !rSect.IsColLocked() )
            rSect.InvalidateSize();
    }
}

// NOTE: call <SwSectionFrm::Init()> directly after creation of a new section
//       frame and its insert in the layout.
void SwSectionFrm::Init()
{
    ASSERT( GetUpper(), "SwSectionFrm::Init before insertion?!" );
    SWRECTFN( this )
    long nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
    (Frm().*fnRect->fnSetWidth)( nWidth );
    (Frm().*fnRect->fnSetHeight)( 0 );

    // #109700# LRSpace for sections
    const SvxLRSpaceItem& rLRSpace = GetFmt()->GetLRSpace();
    (Prt().*fnRect->fnSetLeft)( rLRSpace.GetLeft() );
    (Prt().*fnRect->fnSetWidth)( nWidth - rLRSpace.GetLeft() -
                                 rLRSpace.GetRight() );
    (Prt().*fnRect->fnSetHeight)( 0 );

    const SwFmtCol &rCol = GetFmt()->GetCol();
    if( ( rCol.GetNumCols() > 1 || IsAnyNoteAtEnd() ) && !IsInFtn() )
    {
        const SwFmtCol *pOld = Lower() ? &rCol : new SwFmtCol;
        ChgColumns( *pOld, rCol, IsAnyNoteAtEnd() );
        if( pOld != &rCol )
            delete pOld;
    }
}

SwSectionFrm::~SwSectionFrm()
{
    if( GetFmt() && !GetFmt()->GetDoc()->IsInDtor() )
    {
        SwRootFrm *pRootFrm = GetFmt()->GetDoc()->GetRootFrm();
        if( pRootFrm )
            pRootFrm->RemoveFromList( this );
        if( IsFollow() )
        {
            SwSectionFrm *pMaster = FindMaster();
            if( pMaster )
            {
                PROTOCOL( this, PROT_SECTION, ACT_DEL_FOLLOW, pMaster )
                pMaster->SetFollow( GetFollow() );
                // Ein Master greift sich immer den Platz bis zur Unterkante seines
                // Uppers. Wenn er keinen Follow mehr hat, kann er diesen ggf. wieder
                // freigeben, deshalb wird die Size des Masters invalidiert.
                if( !GetFollow() )
                    pMaster->InvalidateSize();
            }
        }
        else if( HasFollow() )
        {
            PROTOCOL( this, PROT_SECTION, ACT_DEL_MASTER, GetFollow() )
            GetFollow()->bIsFollow = FALSE;
        }
    }
}


/*************************************************************************
|*
|*  SwSectionFrm::DelEmpty()
|*
|*  Ersterstellung      AMA 17. Dec. 97
|*  Letzte Aenderung    AMA 17. Dec. 97
|*
|*************************************************************************/
void SwSectionFrm::DelEmpty( BOOL bRemove )
{
    if( IsColLocked() )
    {
        ASSERT( !bRemove, "Don't delete locked SectionFrms" );
        return;
    }
    SwFrm* pUp = GetUpper();
    if( pUp )
        _Cut( bRemove );
    if( IsFollow() )
    {
        SwSectionFrm *pMaster = FindMaster();
        pMaster->SetFollow( GetFollow() );
        // Ein Master greift sich immer den Platz bis zur Unterkante seines
        // Uppers. Wenn er keinen Follow mehr hat, kann er diesen ggf. wieder
        // freigeben, deshalb wird die Size des Masters invalidiert.
        if( !GetFollow() && !pMaster->IsColLocked() )
            pMaster->InvalidateSize();
        bIsFollow = FALSE;
    }
    else if( HasFollow() )
        GetFollow()->bIsFollow = FALSE;
    pFollow = NULL;
    if( pUp )
    {
        Frm().Height( 0 );
        // Wenn wir sowieso sofort zerstoert werden, brauchen/duerfen wir
        // uns gar nicht erst in die Liste eintragen
        if( bRemove )
        {   // Wenn wir bereits halbtot waren vor diesem DelEmpty, so
            // stehen wir vermutlich auch in der Liste und muessen uns
            // dort austragen
            if( !pSection )
                GetFmt()->GetDoc()->GetRootFrm()->RemoveFromList( this );
        }
        else
            GetFmt()->GetDoc()->GetRootFrm()->InsertEmptySct( this );
        pSection = NULL; // damit ist allerdings eine Reanimierung quasi ausgeschlossen
    }
}

/*************************************************************************
|*
|*  SwSectionFrm::Cut()
|*
|*  Ersterstellung      AMA 02. Dec. 97
|*  Letzte Aenderung    AMA 02. Dec. 97
|*
|*************************************************************************/
void SwSectionFrm::Cut()
{
    _Cut( TRUE );
}

void SwSectionFrm::_Cut( BOOL bRemove )
{
    ASSERT( GetUpper(), "Cut ohne Upper()." );

    PROTOCOL( this, PROT_CUT, 0, GetUpper() )

    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    SwFrm *pFrm = GetNext();
    SwFrm* pPrepFrm = NULL;
    while( pFrm && pFrm->IsSctFrm() && !((SwSectionFrm*)pFrm)->GetSection() )
        pFrm = pFrm->GetNext();
    if( pFrm )
    {   //Der alte Nachfolger hat evtl. einen Abstand zum Vorgaenger
        //berechnet der ist jetzt wo er der erste wird obsolete
        pFrm->_InvalidatePrt();
        pFrm->_InvalidatePos();
        if( pFrm->IsSctFrm() )
            pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
        if ( pFrm && pFrm->IsCntntFrm() )
        {
            pFrm->InvalidatePage( pPage );
            if( IsInFtn() && !GetIndPrev() )
                pPrepFrm = pFrm;
        }
    }
    else
    {
        InvalidateNextPos();
        //Einer muss die Retusche uebernehmen: Vorgaenger oder Upper
        if ( 0 != (pFrm = GetPrev()) )
        {   pFrm->SetRetouche();
            pFrm->Prepare( PREP_WIDOWS_ORPHANS );
            if ( pFrm->IsCntntFrm() )
                pFrm->InvalidatePage( pPage );
        }
        //Wenn ich der einzige FlowFrm in meinem Upper bin (war), so muss
        //er die Retouche uebernehmen.
        //Ausserdem kann eine Leerseite entstanden sein.
        else
        {   SwRootFrm *pRoot = (SwRootFrm*)pPage->GetUpper();
            pRoot->SetSuperfluous();
            GetUpper()->SetCompletePaint();
        }
    }
    //Erst removen, dann Upper Shrinken.
    SwLayoutFrm *pUp = GetUpper();
    if( bRemove )
    {
        Remove();
        if( pUp && !pUp->Lower() && pUp->IsFtnFrm() && !pUp->IsColLocked() &&
            pUp->GetUpper() )
        {
            pUp->Cut();
            delete pUp;
            pUp = NULL;
        }
    }
    if( pPrepFrm )
        pPrepFrm->Prepare( PREP_FTN );
    if ( pUp )
    {
        SWRECTFN( this );
        SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
        if( nFrmHeight > 0 )
        {
            if( !bRemove )
            {
                (Frm().*fnRect->fnSetHeight)( 0 );
                (Prt().*fnRect->fnSetHeight)( 0 );
            }
            pUp->Shrink( nFrmHeight );
        }
    }
}

/*************************************************************************
|*
|*  SwSectionFrm::Paste()
|*
|*  Ersterstellung      AMA 04. Dec. 97
|*  Letzte Aenderung    AMA 04. Dec. 97
|*
|*************************************************************************/

void SwSectionFrm::Paste( SwFrm* pParent, SwFrm* pSibling )
{
    ASSERT( pParent, "Kein Parent fuer Paste." );
    ASSERT( pParent->IsLayoutFrm(), "Parent ist CntntFrm." );
    ASSERT( pParent != this, "Bin selbst der Parent." );
    ASSERT( pSibling != this, "Bin mein eigener Nachbar." );
    ASSERT( !GetPrev() && !GetUpper(),
            "Bin noch irgendwo angemeldet." );

    PROTOCOL( this, PROT_PASTE, 0, GetUpper() )

    //In den Baum einhaengen.
    SwSectionFrm* pSect = pParent->FindSctFrm();

    SWRECTFN( pParent )
    if( pSect && HasToBreak( pSect ) )
    {
        if( pParent->IsColBodyFrm() ) // handelt es sich um einen spaltigen Bereich
        {
            // Falls wir zufaellig am Ende einer Spalte stehen, muss pSibling
            // auf den ersten Frame der naechsten Spalte zeigen, damit
            // der Inhalt der naechsten Spalte von InsertGroup richtig in den
            // neu angelegten pSect umgehaengt wird.
            SwColumnFrm *pCol = (SwColumnFrm*)pParent->GetUpper();
            while( !pSibling && 0 != ( pCol = (SwColumnFrm*)pCol->GetNext() ) )
                pSibling = ((SwLayoutFrm*)((SwColumnFrm*)pCol)->Lower())->Lower();
            if( pSibling )
            {
                // Schlimmer noch: alle folgenden Spalteninhalte muessen
                // an die pSibling-Kette angehaengt werden, damit sie
                // mitgenommen werden.
                SwFrm *pTmp = pSibling;
                while ( 0 != ( pCol = (SwColumnFrm*)pCol->GetNext() ) )
                {
                    while ( pTmp->GetNext() )
                        pTmp = pTmp->GetNext();
                    SwFrm* pSave = ::SaveCntnt( pCol );
                    ::RestoreCntnt( pSave, pSibling->GetUpper(), pTmp, true );
                }
            }
        }
        pParent = pSect;
        pSect = new SwSectionFrm( *((SwSectionFrm*)pParent)->GetSection() );
        // Wenn pParent in zwei Teile zerlegt wird, so muss sein Follow am
        // neuen, zweiten Teil angebracht werden.
        pSect->SetFollow( ((SwSectionFrm*)pParent)->GetFollow() );
        ((SwSectionFrm*)pParent)->SetFollow( NULL );
        if( pSect->GetFollow() )
            pParent->_InvalidateSize();

        InsertGroupBefore( pParent, pSibling, pSect );
        pSect->Init();
        (pSect->*fnRect->fnMakePos)( pSect->GetUpper(), pSect->GetPrev(), TRUE);
        if( !((SwLayoutFrm*)pParent)->Lower() )
        {
            SwSectionFrm::MoveCntntAndDelete( (SwSectionFrm*)pParent, FALSE );
            pParent = this;
        }
    }
    else
        InsertGroupBefore( pParent, pSibling, NULL );

    _InvalidateAll();
    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );

    if ( pSibling )
    {
        pSibling->_InvalidatePos();
        pSibling->_InvalidatePrt();
        if ( pSibling->IsCntntFrm() )
            pSibling->InvalidatePage( pPage );
    }

    SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
    if( nFrmHeight )
        pParent->Grow( nFrmHeight );

    if ( GetPrev() )
    {
        if ( !IsFollow() )
        {
            GetPrev()->InvalidateSize();
            if ( GetPrev()->IsCntntFrm() )
                GetPrev()->InvalidatePage( pPage );
        }
    }
}


/*************************************************************************
|*
|*  SwSectionFrm::HasToBreak()
|*
|*  Hier wird entschieden, ob der this-SectionFrm den uebergebenen
|*  (Section)Frm aufbrechen soll oder nicht.
|*  Zunaechst werden uebergeordnete Bereiche immer aufgebrochen,
|*  spaeter koennte man es einstellbar machen.
|*
|*  Ersterstellung      AMA 12. Dec. 97
|*  Letzte Aenderung    AMA 12. Dec. 97
|*
|*************************************************************************/

BOOL SwSectionFrm::HasToBreak( const SwFrm* pFrm ) const
{
    if( !pFrm->IsSctFrm() )
        return FALSE;

    SwSectionFmt *pTmp = (SwSectionFmt*)GetFmt();
//  if( !pTmp->GetSect().GetValue() )
//      return FALSE;

    const SwFrmFmt *pOtherFmt = ((SwSectionFrm*)pFrm)->GetFmt();
    do
    {
        pTmp = pTmp->GetParent();
        if( !pTmp )
            return FALSE;
        if( pTmp == pOtherFmt )
            return TRUE;
    } while( TRUE ); // ( pTmp->GetSect().GetValue() );
    return FALSE;
}

/*************************************************************************
|*
|*  SwSectionFrm::MergeNext()
|*
|*  Ersterstellung      AMA 04. Dec. 97
|*  Letzte Aenderung    AMA 04. Dec. 97
|*
|*  Verschmilzt zwei SectionFrms, falls es sich um den
|*  gleichen Bereich handelt.
|*  Notwendig kann dies sein, wenn ein (Unter-)Bereich geloescht wird, der
|*  einen anderen in zwei Teile zerlegt hatte.
|*
|*************************************************************************/

void SwSectionFrm::MergeNext( SwSectionFrm* pNxt )
{
    if( !pNxt->IsJoinLocked() && GetSection() == pNxt->GetSection() )
    {
        PROTOCOL( this, PROT_SECTION, ACT_MERGE, pNxt )

        SwFrm* pTmp = ::SaveCntnt( pNxt );
        if( pTmp )
        {
            SwFrm* pLast = Lower();
            SwLayoutFrm* pLay = this;
            if( pLast )
            {
                while( pLast->GetNext() )
                    pLast = pLast->GetNext();
                if( pLast->IsColumnFrm() )
                {   // Spalten jetzt mit BodyFrm
                    pLay = (SwLayoutFrm*)((SwLayoutFrm*)pLast)->Lower();
                    pLast = pLay->Lower();
                    if( pLast )
                        while( pLast->GetNext() )
                            pLast = pLast->GetNext();
                }
            }
            ::RestoreCntnt( pTmp, pLay, pLast, true );
        }
        SetFollow( pNxt->GetFollow() );
        pNxt->SetFollow( NULL );
        pNxt->bIsFollow = FALSE;
        pNxt->Cut();
        delete pNxt;
        InvalidateSize();
    }
}

/*************************************************************************
|*
|*  SwSectionFrm::SplitSect()
|*
|*  Ersterstellung      AMA 29. Apr. 99
|*  Letzte Aenderung    AMA 29. Apr. 99
|*
|*  Zerteilt einen SectionFrm in zwei Teile, der zweite Teil beginnt mit dem
|*  uebergebenen Frame.
|*  Benoetigt wird dies beim Einfuegen eines inneren Bereichs, weil innerhalb
|*  von Rahmen oder Tabellenzellen das MoveFwd nicht den erwuenschten Effekt
|*  haben kann.
|*
|*************************************************************************/

BOOL SwSectionFrm::SplitSect( SwFrm* pFrm, BOOL bApres )
{
    ASSERT( pFrm, "SplitSect: Why?" );
    SwFrm* pOther = bApres ? pFrm->FindNext() : pFrm->FindPrev();
    if( !pOther )
        return FALSE;
    SwSectionFrm* pSect = pOther->FindSctFrm();
    if( pSect != this )
        return FALSE;
    // Den Inhalt zur Seite stellen
    SwFrm* pSav = ::SaveCntnt( this, bApres ? pOther : pFrm );
    ASSERT( pSav, "SplitSect: What's on?" );
    if( pSav ) // Robust
    {   // Einen neuen SctFrm anlegen, nicht als Follow/Master
        SwSectionFrm* pNew = new SwSectionFrm( *pSect->GetSection() );
        pNew->InsertBehind( pSect->GetUpper(), pSect );
        pNew->Init();
        SWRECTFN( this )
        (pNew->*fnRect->fnMakePos)( NULL, pSect, TRUE );
        // OD 25.03.2003 #108339# - restore content:
        // determine layout frame for restoring content after the initialization
        // of the section frame. In the section initialization the columns are
        // created.
        {
            SwLayoutFrm* pLay = pNew;
            // Search for last layout frame, e.g. for columned sections.
            while( pLay->Lower() && pLay->Lower()->IsLayoutFrm() )
                pLay = (SwLayoutFrm*)pLay->Lower();
            ::RestoreCntnt( pSav, pLay, NULL, true );
        }
        _InvalidateSize();
        if( HasFollow() )
        {
            pNew->SetFollow( GetFollow() );
            SetFollow( NULL );
        }
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
|*
|*  SwSectionFrm::MoveCntntAndDelete()
|*
|*  Ersterstellung      AMA 29. Jan 99
|*  Letzte Aenderung    AMA 29. Jan 99
|*
|*  MoveCntnt wird zur Zerstoerung eines SectionFrms wg. Aufhebung oder
|*  Verstecken des Bereichs gerufen, um den Inhalt umzuhaengen.
|*  Wenn der SectionFrm keinen anderen aufbrach, so wird der Inhalt in
|*  den Upper bewegt. Anderfalls wird der Inhalt in den anderen SectionFrm
|*  umgehaengt, dieser muss ggf. gemergt werden.
|*
|*************************************************************************/
// Wenn ein mehrspaltiger Bereich aufgehoben wird, muessen die ContentFrms
// invalidiert werden

void lcl_InvalidateInfFlags( SwFrm* pFrm, BOOL bInva )
{
    while ( pFrm )
    {
        pFrm->InvalidateInfFlags();
        if( bInva )
        {
            pFrm->_InvalidatePos();
            pFrm->_InvalidateSize();
            pFrm->_InvalidatePrt();
        }
        if( pFrm->IsLayoutFrm() )
            lcl_InvalidateInfFlags( ((SwLayoutFrm*)pFrm)->GetLower(), FALSE );
        pFrm = pFrm->GetNext();
    }
}


//
// Works like SwCntntFrm::ImplGetNextCntntFrm, but starts with a LayoutFrm
//
SwCntntFrm* lcl_GetNextCntntFrm( const SwLayoutFrm* pLay, bool bFwd )
{
    if ( bFwd )
    {
        if ( pLay->GetNext() && pLay->GetNext()->IsCntntFrm() )
            return (SwCntntFrm*)pLay->GetNext();
    }
    else
    {
        if ( pLay->GetPrev() && pLay->GetPrev()->IsCntntFrm() )
            return (SwCntntFrm*)pLay->GetPrev();
    }

    // #100926#
    const SwFrm* pFrm = pLay;
    SwCntntFrm *pCntntFrm = 0;
    FASTBOOL bGoingUp = TRUE;
    do {
        const SwFrm *p;
        FASTBOOL bGoingFwdOrBwd = FALSE, bGoingDown = FALSE;

        bGoingDown = !bGoingUp && ( 0 !=  ( p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0 ) );
        if ( !bGoingDown )
        {
            bGoingFwdOrBwd = ( 0 != ( p = pFrm->IsFlyFrm() ?
                                          ( bFwd ? ((SwFlyFrm*)pFrm)->GetNextLink() : ((SwFlyFrm*)pFrm)->GetPrevLink() ) :
                                          ( bFwd ? pFrm->GetNext() :pFrm->GetPrev() ) ) );
            if ( !bGoingFwdOrBwd )
            {
                bGoingUp = (0 != (p = pFrm->GetUpper() ) );
                if ( !bGoingUp )
                    return 0;
            }
        }

        bGoingUp = !( bGoingFwdOrBwd || bGoingDown );

        if( !bFwd && bGoingDown && p )
            while ( p->GetNext() )
                p = p->GetNext();

        pFrm = p;
    } while ( 0 == (pCntntFrm = (pFrm->IsCntntFrm() ? (SwCntntFrm*)pFrm:0) ));

    return pCntntFrm;
}

#define FIRSTLEAF( pLayFrm ) ( ( pLayFrm->Lower() && pLayFrm->Lower()->IsColumnFrm() )\
                    ? pLayFrm->GetNextLayoutLeaf() \
                    : pLayFrm )

void SwSectionFrm::MoveCntntAndDelete( SwSectionFrm* pDel, BOOL bSave )
{
    BOOL bSize = pDel->Lower() && pDel->Lower()->IsColumnFrm();
    SwFrm* pPrv = pDel->GetPrev();
    SwLayoutFrm* pUp = pDel->GetUpper();
    // OD 27.03.2003 #i12711# - initialize local pointer variables.
    SwSectionFrm* pPrvSct = NULL;
    SwSectionFrm* pNxtSct = NULL;
    SwSectionFmt* pParent = static_cast<SwSectionFmt*>(pDel->GetFmt())->GetParent();
    if( pDel->IsInTab() && pParent )
    {
        SwTabFrm *pTab = pDel->FindTabFrm();
        // Wenn wir innerhalb einer Tabelle liegen, koennen wir nur Bereiche
        // aufgebrochen haben, die ebenfalls innerhalb liegen, nicht etwa
        // einen Bereich, der die gesamte Tabelle umfasst.
        if( pTab->IsInSct() && pParent == pTab->FindSctFrm()->GetFmt() )
            pParent = NULL;
    }
    // Wenn unser Format einen Parent besitzt, so haben wir vermutlich
    // einen anderen SectionFrm aufgebrochen, dies muss geprueft werden,
    // dazu besorgen wir uns zunaechst den vorhergehende und den nach-
    // folgenden CntntFrm, mal sehen, ob diese in SectionFrms liegen.
    // OD 27.03.2003 #i12711# - check, if previous and next section belonging
    // together and can be joined, *not* only if deleted section contains content.
    if ( pParent )
    {
        SwFrm* pPrvCntnt = lcl_GetNextCntntFrm( pDel, false );
        pPrvSct = pPrvCntnt ? pPrvCntnt->FindSctFrm() : NULL;
        SwFrm* pNxtCntnt = lcl_GetNextCntntFrm( pDel, true );
        pNxtSct = pNxtCntnt ? pNxtCntnt->FindSctFrm() : NULL;
    }
    else
    {
        pParent = NULL;
        pPrvSct = pNxtSct = NULL;
    }

    // Jetzt wird der Inhalt beseite gestellt und der Frame zerstoert
    SwFrm *pSave = bSave ? ::SaveCntnt( pDel ) : NULL;
    BOOL bOldFtn = TRUE;
    if( pSave && pUp->IsFtnFrm() )
    {
        bOldFtn = ((SwFtnFrm*)pUp)->IsColLocked();
        ((SwFtnFrm*)pUp)->ColLock();
    }
    pDel->DelEmpty( TRUE );
    delete pDel;
    if( pParent )
    {   // Hier wird die geeignete Einfuegeposition gesucht
        if( pNxtSct && pNxtSct->GetFmt() == pParent )
        {   // Hier koennen wir uns am Anfang einfuegen
            pUp = FIRSTLEAF( pNxtSct );
            pPrv = NULL;
            if( pPrvSct && !( pPrvSct->GetFmt() == pParent ) )
                pPrvSct = NULL; // damit nicht gemergt wird
        }
        else if( pPrvSct && pPrvSct->GetFmt() == pParent )
        {   // Wunderbar, hier koennen wir uns am Ende einfuegen
            pUp = pPrvSct;
            if( pUp->Lower() && pUp->Lower()->IsColumnFrm() )
            {
                pUp = (SwLayoutFrm*)pUp->Lower(); // Die erste Spalte
                while( pUp->GetNext() )
                    pUp = (SwLayoutFrm*)pUp->GetNext();
                pUp = (SwLayoutFrm*)pUp->Lower(); // Der Body der letzten Spalte
            }
            pPrv = pUp->Lower(); // damit hinter dem letzten eingefuegt wird
            if( pPrv )
                while( pPrv->GetNext() )
                    pPrv = pPrv->GetNext();
            pPrvSct = NULL; // damit nicht gemergt wird
        }
        else
        {
            if( pSave )
            {   // Folgende Situationen: Vor und hinter dem zu loeschenden Bereich
                // ist entweder die Bereichsgrenze des umfassenden Bereichs oder
                // es schliesst ein anderer (Geschwister-)Bereich direkt an, der
                // vom gleichen Parent abgeleitet ist.
                // Dann gibt es (noch) keinen Teil unseres Parents, der den Inhalt
                // aufnehmen kann,also bauen wir ihn uns.
                pPrvSct = new SwSectionFrm( *pParent->GetSection() );
                pPrvSct->InsertBehind( pUp, pPrv );
                pPrvSct->Init();
                SWRECTFN( pUp )
                (pPrvSct->*fnRect->fnMakePos)( pUp, pPrv, TRUE );
                pUp = FIRSTLEAF( pPrvSct );
                pPrv = NULL;
            }
            pPrvSct = NULL; // damit nicht gemergt wird
        }
    }
    // Der Inhalt wird eingefuegt..
    if( pSave )
    {
        lcl_InvalidateInfFlags( pSave, bSize );
        ::RestoreCntnt( pSave, pUp, pPrv, true );
        pUp->FindPageFrm()->InvalidateCntnt();
        if( !bOldFtn )
            ((SwFtnFrm*)pUp)->ColUnlock();
    }
    // jetzt koennen eventuell zwei Teile des uebergeordneten Bereich verschmelzen
    if( pPrvSct && !pPrvSct->IsJoinLocked() )
    {
        ASSERT( pNxtSct, "MoveCntnt: No Merge" );
        pPrvSct->MergeNext( pNxtSct );
    }
}

void SwSectionFrm::MakeAll()
{
    if ( IsJoinLocked() || IsColLocked() || StackHack::IsLocked() || StackHack::Count() > 50 )
        return;
    if( !pSection ) // Durch DelEmpty
    {
        ASSERT( GetFmt()->GetDoc()->GetRootFrm()->IsInDelList( this ), "SectionFrm without Section" );
        if( !bValidPos )
        {
            if( GetUpper() )
            {
                SWRECTFN( GetUpper() )
                (this->*fnRect->fnMakePos)( GetUpper(), GetPrev(), FALSE );
            }
        }
        bValidSize = bValidPos = bValidPrtArea = TRUE;
        return;
    }
    LockJoin(); //Ich lass mich nicht unterwegs vernichten.

    while( GetNext() && GetNext() == GetFollow() )
    {
        const SwFrm* pFoll = GetFollow();
        MergeNext( (SwSectionFrm*)GetNext() );
        if( pFoll == GetFollow() )
            break;
    }

    // OD 2004-03-15 #116561# - In online layout join the follows, if section
    // can grow.
    if ( GetFmt()->GetDoc()->IsBrowseMode() &&
         ( Grow( LONG_MAX, true ) > 0 ) )
    {
        while( GetFollow() )
        {
            const SwFrm* pFoll = GetFollow();
            MergeNext( GetFollow() );
            if( pFoll == GetFollow() )
                break;
        }
    }

    // Ein Bereich mit Follow nimmt allen Platz bis zur Unterkante des Uppers
    // in Anspruch. Bewegt er sich, so kann seine Groesse zu- oder abnehmen...
    if( !bValidPos && ToMaximize( FALSE ) )
        bValidSize = FALSE;

#if OSL_DEBUG_LEVEL > 1
    const SwFmtCol &rCol = GetFmt()->GetCol();
#endif
    SwLayoutFrm::MakeAll();
    UnlockJoin();
    if( pSection && IsSuperfluous() )
        DelEmpty( FALSE );
}

BOOL SwSectionFrm::ShouldBwdMoved( SwLayoutFrm *pNewUpper, BOOL bHead, BOOL &rReformat )
{
    ASSERT( FALSE, "Hups, wo ist meine Tarnkappe?" );
    return FALSE;
}

const SwSectionFmt* SwSectionFrm::_GetEndSectFmt() const
{
    const SwSectionFmt *pFmt = pSection->GetFmt();
    while( !pFmt->GetEndAtTxtEnd().IsAtEnd() )
    {
        if( pFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
            pFmt = (SwSectionFmt*)pFmt->GetRegisteredIn();
        else
            return NULL;
    }
    return pFmt;
}

void lcl_FindCntntFrm( SwCntntFrm* &rpCntntFrm, SwFtnFrm* &rpFtnFrm,
    SwFrm* pFrm, BOOL &rbChkFtn )
{
    if( pFrm )
    {
        while( pFrm->GetNext() )
            pFrm = pFrm->GetNext();
        while( !rpCntntFrm && pFrm )
        {
            if( pFrm->IsCntntFrm() )
                rpCntntFrm = (SwCntntFrm*)pFrm;
            else if( pFrm->IsLayoutFrm() )
            {
                if( pFrm->IsFtnFrm() )
                {
                    if( rbChkFtn )
                    {
                        rpFtnFrm = (SwFtnFrm*)pFrm;
                        rbChkFtn = rpFtnFrm->GetAttr()->GetFtn().IsEndNote();
                    }
                }
                else
                    lcl_FindCntntFrm( rpCntntFrm, rpFtnFrm,
                        ((SwLayoutFrm*)pFrm)->Lower(), rbChkFtn );
            }
            pFrm = pFrm->GetPrev();
        }
    }
}

SwCntntFrm *SwSectionFrm::FindLastCntnt( BYTE nMode )
{
    SwCntntFrm *pRet = NULL;
    SwFtnFrm *pFtnFrm = NULL;
    SwSectionFrm *pSect = this;
    if( nMode )
    {
        const SwSectionFmt *pFmt = IsEndnAtEnd() ? GetEndSectFmt() :
                                     pSection->GetFmt();
        do {
            while( pSect->HasFollow() )
                pSect = pSect->GetFollow();
            SwFrm* pTmp = pSect->FindNext();
            while( pTmp && pTmp->IsSctFrm() &&
                   !((SwSectionFrm*)pTmp)->GetSection() )
                pTmp = pTmp->FindNext();
            if( pTmp && pTmp->IsSctFrm() &&
                ((SwSectionFrm*)pTmp)->IsDescendantFrom( pFmt ) )
                pSect = (SwSectionFrm*)pTmp;
            else
                break;
        } while( TRUE );
    }
    BOOL bFtnFound = nMode == FINDMODE_ENDNOTE;
    do
    {
        lcl_FindCntntFrm( pRet, pFtnFrm, pSect->Lower(), bFtnFound );
        if( pRet || !pSect->IsFollow() || !nMode ||
            ( FINDMODE_MYLAST == nMode && this == pSect ) )
            break;
        pSect = pSect->FindMaster();
    } while( pSect );
    if( ( nMode == FINDMODE_ENDNOTE ) && pFtnFrm )
        pRet = pFtnFrm->ContainsCntnt();
    return pRet;
}

BOOL SwSectionFrm::CalcMinDiff( SwTwips& rMinDiff ) const
{
    if( ToMaximize( TRUE ) )
    {
        SWRECTFN( this )
        rMinDiff = (GetUpper()->*fnRect->fnGetPrtBottom)();
        rMinDiff = (Frm().*fnRect->fnBottomDist)( rMinDiff );
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
 *
 *  SwSectionFrm::CollectEndnotes(  )
 *
 *  Ersterstellung      AMA 03. Nov 99
 *  Letzte Aenderung    AMA 03. Nov 99
 *
 *  CollectEndnotes looks for endnotes in the sectionfrm and his follows,
 *  the endnotes will cut off the layout and put into the array.
 *  If the first endnote is not a master-SwFtnFrm, the whole sectionfrm
 *  contains only endnotes and it is not necessary to collect them.
 *
 *************************************************************************/

SwFtnFrm* lcl_FindEndnote( SwSectionFrm* &rpSect, BOOL &rbEmpty,
    SwLayouter *pLayouter )
{
    // if rEmpty is set, the rpSect is already searched
    SwSectionFrm* pSect = rbEmpty ? rpSect->GetFollow() : rpSect;
    while( pSect )
    {
        ASSERT( pSect->Lower() && pSect->Lower()->IsColumnFrm(),
            "InsertEndnotes: Where's my column?" );
        SwColumnFrm* pCol = (SwColumnFrm*)pSect->Lower();
        do // check all columns
        {
            SwFtnContFrm* pFtnCont = pCol->FindFtnCont();
            if( pFtnCont )
            {
                SwFtnFrm* pRet = (SwFtnFrm*)pFtnCont->Lower();
                while( pRet ) // look for endnotes
                {
                    if( pRet->GetAttr()->GetFtn().IsEndNote() )
                    {
                        if( pRet->GetMaster() )
                        {
                            if( pLayouter )
                                pLayouter->CollectEndnote( pRet );
                            else
                                return 0;
                        }
                        else
                            return pRet; // Found
                    }
                    pRet = (SwFtnFrm*)pRet->GetNext();
                }
            }
            pCol = (SwColumnFrm*)pCol->GetNext();
        } while ( pCol );
        rpSect = pSect;
        pSect = pLayouter ? pSect->GetFollow() : NULL;
        rbEmpty = TRUE;
    }
    return NULL;
}

void lcl_ColumnRefresh( SwSectionFrm* pSect, BOOL bFollow )
{
    while( pSect )
    {
        BOOL bOldLock = pSect->IsColLocked();
        pSect->ColLock();
        if( pSect->Lower() && pSect->Lower()->IsColumnFrm() )
        {
            SwColumnFrm *pCol = (SwColumnFrm*)pSect->Lower();
            do
            {   pCol->_InvalidateSize();
                pCol->_InvalidatePos();
                ((SwLayoutFrm*)pCol)->Lower()->_InvalidateSize();
                pCol->Calc();   // calculation of column and
                ((SwLayoutFrm*)pCol)->Lower()->Calc();  // body
                pCol = (SwColumnFrm*)pCol->GetNext();
            } while ( pCol );
        }
        if( !bOldLock )
            pSect->ColUnlock();
        if( bFollow )
            pSect = pSect->GetFollow();
        else
            pSect = NULL;
    }
}

void SwSectionFrm::CollectEndnotes( SwLayouter* pLayouter )
{
    ASSERT( IsColLocked(), "CollectEndnotes: You love the risk?" );
    ASSERT( Lower() && Lower()->IsColumnFrm(), "Where's my column?" );
    SwSectionFrm* pSect = this;
    SwFtnFrm* pFtn;
    BOOL bEmpty = FALSE;
    // pSect is the last sectionfrm without endnotes or the this-pointer
    // the first sectionfrm with endnotes may be destroyed, when the endnotes
    // is cutted
    while( 0 != (pFtn = lcl_FindEndnote( pSect, bEmpty, pLayouter )) )
        pLayouter->CollectEndnote( pFtn );
    if( pLayouter->HasEndnotes() )
        lcl_ColumnRefresh( this, TRUE );
}

/*************************************************************************
|*
|*  SwSectionFrm::_CheckClipping( BOOL bGrow, BOOL bMaximize )
|*
|*  Beschreibung:       Passt die Groesse an die Umgebung an.
|*      Wer einen Follow oder Fussnoten besitzt, soll bis zur Unterkante
|*      des Uppers gehen (bMaximize).
|*      Niemand darf ueber den Upper hinausgehen, ggf. darf man versuchen (bGrow)
|*      seinen Upper zu growen.
|*      Wenn die Groesse veraendert werden musste, wird der Inhalt kalkuliert.
|*
|*************************************************************************/

/// OD 18.09.2002 #100522#
/// perform calculation of content, only if height has changed.
void SwSectionFrm::_CheckClipping( BOOL bGrow, BOOL bMaximize )
{
    SWRECTFN( this )
    long nDiff;
    SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
    if( bGrow && ( !IsInFly() || !GetUpper()->IsColBodyFrm() ||
                   !FindFlyFrm()->IsLocked() ) )
    {
        nDiff = -(Frm().*fnRect->fnBottomDist)( nDeadLine );
        if( !bMaximize )
            nDiff += Undersize();
        if( nDiff > 0 )
        {
            long nAdd = GetUpper()->Grow( nDiff );
            if( bVert && !bRev )
                nDeadLine -= nAdd;
            else
                nDeadLine += nAdd;
        }
    }
    nDiff = -(Frm().*fnRect->fnBottomDist)( nDeadLine );
    SetUndersized( !bMaximize && nDiff >= 0 );
    const bool bCalc = ( IsUndersized() || bMaximize ) &&
                       ( nDiff ||
                         (Prt().*fnRect->fnGetTop)() > (Frm().*fnRect->fnGetHeight)() );
    // OD 03.11.2003 #i19737# - introduce local variable <bExtraCalc> to indicate
    // that a calculation has to be done beside the value of <bCalc>.
    bool bExtraCalc = false;
    if( !bCalc && !bGrow && IsAnyNoteAtEnd() && !IsInFtn() )
    {
        SwSectionFrm *pSect = this;
        BOOL bEmpty = FALSE;
        SwLayoutFrm* pFtn = IsEndnAtEnd() ?
            lcl_FindEndnote( pSect, bEmpty, NULL ) : NULL;
        if( pFtn )
        {
            pFtn = pFtn->FindFtnBossFrm();
            SwFrm* pTmp = FindLastCntnt( FINDMODE_LASTCNT );
            // OD 08.11.2002 #104840# - use <SwLayoutFrm::IsBefore(..)>
            if ( pTmp && pFtn->IsBefore( pTmp->FindFtnBossFrm() ) )
                bExtraCalc = true;
        }
        else if( GetFollow() && !GetFollow()->ContainsAny() )
            bExtraCalc = true;
    }
    if ( bCalc || bExtraCalc )
    {
        nDiff = (*fnRect->fnYDiff)( nDeadLine, (Frm().*fnRect->fnGetTop)() );
        if( nDiff < 0 )
        {
            nDiff = 0;
            nDeadLine = (Frm().*fnRect->fnGetTop)();
        }
        const Size aOldSz( Prt().SSize() );
        long nTop = (this->*fnRect->fnGetTopMargin)();
        (Frm().*fnRect->fnSetBottom)( nDeadLine );
        nDiff = (Frm().*fnRect->fnGetHeight)();
        if( nTop > nDiff )
            nTop = nDiff;
        (this->*fnRect->fnSetYMargins)( nTop, 0 );

        // OD 18.09.2002 #100522#
        // Determine, if height has changed.
        // Note: In vertical layout the height equals the width value.
        bool bHeightChanged = bVert ?
                            (aOldSz.Width() != Prt().Width()) :
                            (aOldSz.Height() != Prt().Height());
        // Wir haben zu guter Letzt noch einmal die Hoehe geaendert,
        // dann wird das innere Layout (Columns) kalkuliert und
        // der Inhalt ebenfalls.
        // OD 18.09.2002 #100522#
        // calculate content, only if height has changed.
        // OD 03.11.2003 #i19737# - restriction of content calculation too strong.
        // If an endnote has an incorrect position or a follow section contains
        // no content except footnotes/endnotes, the content has also been calculated.
        if ( ( bHeightChanged || bExtraCalc ) && Lower() )
        {
            if( Lower()->IsColumnFrm() )
            {
                lcl_ColumnRefresh( this, FALSE );
                ::CalcCntnt( this );
            }
            else
            {
                ChgLowersProp( aOldSz );
                if( !bMaximize && !IsCntntLocked() )
                    ::CalcCntnt( this );
            }
        }
    }
}

void SwSectionFrm::SimpleFormat()
{
    if ( IsJoinLocked() || IsColLocked() )
        return;
    // ASSERT( pFollow, "SimpleFormat: Follow required" );
    LockJoin();
    SWRECTFN( this )
    if( GetPrev() || GetUpper() )
    {
        (this->*fnRect->fnMakePos)( GetUpper(), GetPrev(), FALSE );
        bValidPos = TRUE;
    }
    SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
    // OD 22.10.2002 #97265# - call always method <lcl_ColumnRefresh(..)>, in
    // order to get calculated lowers, not only if there space left in its upper.
    if( (Frm().*fnRect->fnBottomDist)( nDeadLine ) >= 0 )
    {
        (Frm().*fnRect->fnSetBottom)( nDeadLine );
        long nHeight = (Frm().*fnRect->fnGetHeight)();
        long nTop = CalcUpperSpace();
        if( nTop > nHeight )
            nTop = nHeight;
        (this->*fnRect->fnSetYMargins)( nTop, 0 );
    }
    lcl_ColumnRefresh( this, FALSE );
    UnlockJoin();
}

/*************************************************************************
|*
|*  SwSectionFrm::Format()
|*
|*  Beschreibung:       "Formatiert" den Frame; Frm und PrtArea.
|*  Ersterstellung      AMA 03. Dec. 97
|*  Letzte Aenderung    MA 09. Oct. 98
|*
|*************************************************************************/

void SwSectionFrm::Format( const SwBorderAttrs *pAttr )
{
    if( !pSection ) // Durch DelEmpty
    {
        ASSERT( GetFmt()->GetDoc()->GetRootFrm()->IsInDelList( this ),
                 "SectionFrm without Section" );
        bValidSize = bValidPos = bValidPrtArea = TRUE;
        return;
    }
    SWRECTFN( this )
    if ( !bValidPrtArea )
    {
        PROTOCOL( this, PROT_PRTAREA, 0, 0 )
        bValidPrtArea = TRUE;
        SwTwips nUpper = CalcUpperSpace();

        // #109700# LRSpace for sections
        const SvxLRSpaceItem& rLRSpace = GetFmt()->GetLRSpace();
        (this->*fnRect->fnSetXMargins)( rLRSpace.GetLeft(), rLRSpace.GetRight() );

        if( nUpper != (this->*fnRect->fnGetTopMargin)() )
        {
            bValidSize = FALSE;
            SwFrm* pOwn = ContainsAny();
            if( pOwn )
                pOwn->_InvalidatePos();
        }
        (this->*fnRect->fnSetYMargins)( nUpper, 0 );
    }

    if ( !bValidSize )
    {
        PROTOCOL_ENTER( this, PROT_SIZE, 0, 0 )
        const long nOldHeight = (Frm().*fnRect->fnGetHeight)();
        BOOL bOldLock = IsColLocked();
        ColLock();

        bValidSize = TRUE;

        //die Groesse wird nur dann vom Inhalt bestimmt, wenn der SectFrm
        //keinen Follow hat. Anderfalls fuellt er immer den Upper bis
        //zur Unterkante aus. Fuer den Textfluss ist nicht er, sondern sein
        //Inhalt selbst verantwortlich.
        BOOL bMaximize = ToMaximize( FALSE );

        // Column widths have to be adjusted before calling _CheckClipping.
        // _CheckClipping can cause the formatting of the lower frames
        // which still have a width of 0.
        const sal_Bool bHasColumns = Lower() && Lower()->IsColumnFrm();
        if ( bHasColumns && Lower()->GetNext() )
            AdjustColumns( 0, FALSE );

        if( GetUpper() )
        {
            long nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
            (aFrm.*fnRect->fnSetWidth)( nWidth );

            // #109700# LRSpace for sections
            const SvxLRSpaceItem& rLRSpace = GetFmt()->GetLRSpace();
            (aPrt.*fnRect->fnSetWidth)( nWidth - rLRSpace.GetLeft() -
                                        rLRSpace.GetRight() );

            /// OD 15.10.2002 #103517# - allow grow in online layout
            /// Thus, set <..IsBrowseMode()> as parameter <bGrow> on calling
            /// method <_CheckClipping(..)>.
            _CheckClipping( GetFmt()->GetDoc()->IsBrowseMode(), bMaximize );
            bMaximize = ToMaximize( FALSE );
            bValidSize = TRUE;
        }

        //Breite der Spalten pruefen und ggf. einstellen.
        if ( bHasColumns && ! Lower()->GetNext() && bMaximize )
            ((SwColumnFrm*)Lower())->Lower()->Calc();

        if ( !bMaximize )
        {
            SwTwips nRemaining = (this->*fnRect->fnGetTopMargin)(), nDiff;
            SwFrm *pFrm = pLower;
            if( pFrm )
            {
                if( pFrm->IsColumnFrm() && pFrm->GetNext() )
                {
                    FormatWidthCols( *pAttr, nRemaining, MINLAY );
                    while( HasFollow() && !GetFollow()->ContainsCntnt() )
                    {
                        SwFrm* pOld = GetFollow();
                        GetFollow()->DelEmpty( FALSE );
                        if( pOld == GetFollow() )
                            break;
                    }
                    bMaximize = ToMaximize( FALSE );
                    nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)();
                }
                else
                {
                    if( pFrm->IsColumnFrm() )
                    {
                        pFrm->Calc();
                        pFrm = ((SwColumnFrm*)pFrm)->Lower();
                        pFrm->Calc();
                        pFrm = ((SwLayoutFrm*)pFrm)->Lower();
                        CalcFtnCntnt();
                    }
                    // Wenn wir in einem spaltigen Rahmen stehen und dieser
                    // gerade im FormatWidthCols ein CalcCntnt ruft, muss
                    // unser Inhalt ggf. kalkuliert werden.
                    if( pFrm && !pFrm->IsValid() && IsInFly() &&
                        FindFlyFrm()->IsColLocked() )
                        ::CalcCntnt( this );
                    nRemaining += InnerHeight();
                    bMaximize = HasFollow();
                }
            }

            nDiff = (Frm().*fnRect->fnGetHeight)() - nRemaining;
            if( nDiff < 0)
            {
                SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
                {
                    long nBottom = (Frm().*fnRect->fnGetBottom)();
                    nBottom = (*fnRect->fnYInc)( nBottom, -nDiff );
                    long nTmpDiff = (*fnRect->fnYDiff)( nBottom, nDeadLine );
                    if( nTmpDiff > 0 )
                    {
                        nTmpDiff = GetUpper()->Grow( nTmpDiff, TRUE );
                        nDeadLine = (*fnRect->fnYInc)( nDeadLine, nTmpDiff );
                        nTmpDiff = (*fnRect->fnYDiff)( nBottom, nDeadLine );
                        if( nTmpDiff > 0 )
                            nDiff += nTmpDiff;
                        if( nDiff > 0 )
                            nDiff = 0;
                    }
                }
            }
            if( nDiff )
            {
                long nTmp = nRemaining - (Frm().*fnRect->fnGetHeight)();
                long nTop = (this->*fnRect->fnGetTopMargin)();
                (Frm().*fnRect->fnAddBottom)( nTmp );
                (this->*fnRect->fnSetYMargins)( nTop, 0 );
                InvalidateNextPos();
                if( pLower && ( !pLower->IsColumnFrm() || !pLower->GetNext() ) )
                {
                    // Wenn ein einspaltiger Bereich gerade den Platz geschaffen
                    // hat, den sich die "undersized" Absaetze gewuenscht haben,
                    // muessen diese invalidiert und kalkuliert werden, damit
                    // sie diesen ausfuellen.
                    pFrm = pLower;
                    if( pFrm->IsColumnFrm() )
                    {
                        pFrm->_InvalidateSize();
                        pFrm->_InvalidatePos();
                        pFrm->Calc();
                        pFrm = ((SwColumnFrm*)pFrm)->Lower();
                        pFrm->Calc();
                        pFrm = ((SwLayoutFrm*)pFrm)->Lower();
                        CalcFtnCntnt();
                    }
                    BOOL bUnderSz = FALSE;
                    while( pFrm )
                    {
                        if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
                        {
                            pFrm->Prepare( PREP_ADJUST_FRM );
                            bUnderSz = TRUE;
                        }
                        pFrm = pFrm->GetNext();
                    }
                    if( bUnderSz && !IsCntntLocked() )
                        ::CalcCntnt( this );
                }
            }
        }

        //Unterkante des Uppers nicht ueberschreiten. Fuer Sections mit
        //Follows die Unterkante auch nicht unterschreiten.
        if ( GetUpper() )
            _CheckClipping( TRUE, bMaximize );
        if( !bOldLock )
            ColUnlock();
        long nDiff = nOldHeight - (Frm().*fnRect->fnGetHeight)();
        if( nDiff > 0 )
        {
            if( !GetNext() )
                SetRetouche(); // Dann muessen wir die Retusche selbst uebernehmen
            if( GetUpper() && !GetUpper()->IsFooterFrm() )
                GetUpper()->Shrink( nDiff PHEIGHT );
        }
        if( IsUndersized() )
            bValidPrtArea = TRUE;
    }
}

/*************************************************************************
|*
|*  SwFrm::GetNextSctLeaf()
|*
|*  Beschreibung        Liefert das naechste Layoutblatt in das der Frame
|*      gemoved werden kann.
|*      Neue Seiten werden nur dann erzeugt, wenn der Parameter TRUE ist.
|*  Ersterstellung      AMA 07. Jan. 98
|*  Letzte Aenderung    AMA 07. Jan. 98
|*
|*************************************************************************/


SwLayoutFrm *SwFrm::GetNextSctLeaf( MakePageType eMakePage )
{
    //Achtung: Geschachtelte Bereiche werden zur Zeit nicht unterstuetzt.

    PROTOCOL_ENTER( this, PROT_LEAF, ACT_NEXT_SECT, GetUpper()->FindSctFrm() )

    // Abkuerzungen fuer spaltige Bereiche, wenn wir noch nicht in der letzten Spalte sind.
    // Koennen wir in die naechste Spalte des Bereichs rutschen?
    if( IsColBodyFrm() && GetUpper()->GetNext() )
        return (SwLayoutFrm*)((SwLayoutFrm*)GetUpper()->GetNext())->Lower();
    if( GetUpper()->IsColBodyFrm() && GetUpper()->GetUpper()->GetNext() )
        return (SwLayoutFrm*)((SwLayoutFrm*)GetUpper()->GetUpper()->GetNext())->Lower();
    // Innerhalb von Bereichen in Tabellen oder Bereichen in Kopf/Fusszeilen kann
    // nur ein Spaltenwechsel erfolgen, eine der oberen Abkuerzungen haette zuschlagen muessen
    if( ( IsInTab() && !IsTabFrm() ) || FindFooterOrHeader() )
        return 0;

//MA 03. Feb. 99: Warum GetUpper()? Das knallt mit Buch.sgl weil im
//FlyAtCnt::MakeFlyPos ein Orient der SectionFrm ist und auf diesen ein
//GetLeaf gerufen wird.
//  SwSectionFrm *pSect = GetUpper()->FindSctFrm();
    SwSectionFrm *pSect = FindSctFrm();
    BOOL bWrongPage = FALSE;
    ASSERT( pSect, "GetNextSctLeaf: Missing SectionFrm" );

    // Hier eine Abkuerzung fuer Bereiche mit Follows,
    // dieser kann akzeptiert werden, wenn keine Spalten oder Seiten (ausser Dummyseiten)
    // dazwischen liegen.
    // Bei verketteten Rahmen und ind Fussnoten wuerde die Abkuerzung noch aufwendiger
    if( pSect->HasFollow() && pSect->IsInDocBody() )
    {
        if( pSect->GetFollow() == pSect->GetNext() )
        {
            SwPageFrm *pPg = pSect->GetFollow()->FindPageFrm();
            if( WrongPageDesc( pPg ) )
                bWrongPage = TRUE;
            else
                return FIRSTLEAF( pSect->GetFollow() );
        }
        else
        {
            SwFrm* pTmp;
            if( !pSect->GetUpper()->IsColBodyFrm() ||
                0 == ( pTmp = pSect->GetUpper()->GetUpper()->GetNext() ) )
                pTmp = pSect->FindPageFrm()->GetNext();
            if( pTmp ) // ist jetzt die naechste Spalte oder Seite
            {
                SwFrm* pTmpX = pTmp;
                if( pTmp->IsPageFrm() && ((SwPageFrm*)pTmp)->IsEmptyPage() )
                    pTmp = pTmp->GetNext(); // Dummyseiten ueberspringen
                SwFrm *pUp = pSect->GetFollow()->GetUpper();
                // pUp wird die Spalte, wenn der Follow in einer "nicht ersten" Spalte
                // liegt, ansonsten die Seite:
                if( !pUp->IsColBodyFrm() ||
                    !( pUp = pUp->GetUpper() )->GetPrev() )
                    pUp = pUp->FindPageFrm();
                // Jetzt muessen pUp und pTmp die gleiche Seite/Spalte sein,
                // sonst liegen Seiten oder Spalten zwischen Master und Follow.
                if( pUp == pTmp || pUp->GetNext() == pTmpX )
                {
                    SwPageFrm* pNxtPg = pUp->IsPageFrm() ?
                                        (SwPageFrm*)pUp : pUp->FindPageFrm();
                    if( WrongPageDesc( pNxtPg ) )
                        bWrongPage = TRUE;
                    else
                        return FIRSTLEAF( pSect->GetFollow() );
                }
            }
        }
    }

    // Immer im gleichen Bereich landen: Body wieder in Body etc.
    const BOOL bBody = IsInDocBody();
    const BOOL bFtnPage = FindPageFrm()->IsFtnPage();

    SwLayoutFrm *pLayLeaf;
    // Eine Abkuerzung fuer TabFrms, damit nicht alle Zellen abgehuehnert werden
    if( bWrongPage )
        pLayLeaf = 0;
    else if( IsTabFrm() )
    {
        SwCntntFrm* pTmpCnt = ((SwTabFrm*)this)->FindLastCntnt();
        pLayLeaf = pTmpCnt ? pTmpCnt->GetUpper() : 0;
    }
    else
    {
        pLayLeaf = GetNextLayoutLeaf();
        if( IsColumnFrm() )
        {
            while( pLayLeaf && ((SwColumnFrm*)this)->IsAnLower( pLayLeaf ) )
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
        }
    }

    SwLayoutFrm *pOldLayLeaf = 0;           //Damit bei neu erzeugten Seiten
                                            //nicht wieder vom Anfang gesucht
                                            //wird.

    while( TRUE )
    {
        if( pLayLeaf )
        {
            // Ein Layoutblatt wurde gefunden, mal sehen, ob er mich aufnehmen kann,
            // ob hier ein weiterer SectionFrm eingefuegt werden kann
            // oder ob wir weitersuchen muessen.
            SwPageFrm* pNxtPg = pLayLeaf->FindPageFrm();
            if ( !bFtnPage && pNxtPg->IsFtnPage() )
            {   //Wenn ich bei den Endnotenseiten angelangt bin hat sichs.
                pLayLeaf = 0;
                continue;
            }
            // Einmal InBody, immer InBody, nicht in Tabellen hinein
            // und nicht in fremde Bereiche hinein
            if ( (bBody && !pLayLeaf->IsInDocBody()) ||
                 (IsInFtn() != pLayLeaf->IsInFtn() ) ||
                 pLayLeaf->IsInTab() ||
                 ( pLayLeaf->IsInSct() && ( !pSect->HasFollow()
                   || pSect->GetFollow() != pLayLeaf->FindSctFrm() ) ) )
            {
                //Er will mich nicht; neuer Versuch, neues Glueck
                pOldLayLeaf = pLayLeaf;
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
                continue;
            }
            if( WrongPageDesc( pNxtPg ) )
            {
                if( bWrongPage )
                    break; // there's a column between me and my right page
                pLayLeaf = 0;
                bWrongPage = TRUE;
                pOldLayLeaf = 0;
                continue;
            }
        }
        //Es gibt keinen passenden weiteren LayoutFrm, also muss eine
        //neue Seite her, allerdings nuetzen uns innerhalb eines Rahmens
        //neue Seiten nichts.
        else if( !pSect->IsInFly() &&
            ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT ) )
        {
            InsertPage(pOldLayLeaf ? pOldLayLeaf->FindPageFrm() : FindPageFrm(),
                       FALSE );
            //und nochmal das ganze
            pLayLeaf = pOldLayLeaf ? pOldLayLeaf : GetNextLayoutLeaf();
            continue;
        }
        break;
    }

    if( pLayLeaf )
    {
        // Das passende Layoutblatt haben wir gefunden, wenn es dort bereits einen
        // Follow unseres Bereichs gibt, nehmen wir dessen erstes Layoutblatt,
        // andernfalls wird es Zeit, einen Bereichsfollow zu erzeugen
        SwSectionFrm* pNew;

        //Dies kann entfallen, wenn bei existierenden Follows bereits abgekuerzt wurde
        SwFrm* pFirst = pLayLeaf->Lower();
        // Auch hier muessen zum Loeschen angemeldete SectionFrms ignoriert werden
        while( pFirst && pFirst->IsSctFrm() && !((SwSectionFrm*)pFirst)->GetSection() )
            pFirst = pFirst->GetNext();
        if( pFirst && pFirst->IsSctFrm() && pSect->GetFollow() == pFirst )
            pNew = pSect->GetFollow();
        else if( MAKEPAGE_NOSECTION == eMakePage )
            return pLayLeaf;
        else
        {
            pNew = new SwSectionFrm( *pSect, FALSE );
            pNew->InsertBefore( pLayLeaf, pLayLeaf->Lower() );
            pNew->Init();
            SWRECTFN( pNew )
            (pNew->*fnRect->fnMakePos)( pLayLeaf, NULL, TRUE );

            // Wenn unser Bereichsframe einen Nachfolger hat, so muss dieser
            // umgehaengt werden hinter den neuen Follow der Bereichsframes.
            SwFrm* pTmp = pSect->GetNext();
            if( pTmp && pTmp != pSect->GetFollow() )
            {
                SwFlowFrm* pNxt;
                SwCntntFrm* pNxtCntnt = NULL;
                if( pTmp->IsCntntFrm() )
                {
                    pNxt = (SwCntntFrm*)pTmp;
                    pNxtCntnt = (SwCntntFrm*)pTmp;
                }
                else
                {
                    pNxtCntnt = ((SwLayoutFrm*)pTmp)->ContainsCntnt();
                    if( pTmp->IsSctFrm() )
                        pNxt = (SwSectionFrm*)pTmp;
                    else
                    {
                        ASSERT( pTmp->IsTabFrm(), "GetNextSctLeaf: Wrong Type" );
                        pNxt = (SwTabFrm*)pTmp;
                    }
                    while( !pNxtCntnt && 0 != ( pTmp = pTmp->GetNext() ) )
                    {
                        if( pTmp->IsCntntFrm() )
                            pNxtCntnt = (SwCntntFrm*)pTmp;
                        else
                            pNxtCntnt = ((SwLayoutFrm*)pTmp)->ContainsCntnt();
                    }
                }
                if( pNxtCntnt )
                {
                    SwFtnBossFrm* pOldBoss = pSect->FindFtnBossFrm( TRUE );
                    if( pOldBoss == pNxtCntnt->FindFtnBossFrm( TRUE ) )
                    {
                        SwSaveFtnHeight aHeight( pOldBoss,
                            pOldBoss->Frm().Top() + pOldBoss->Frm().Height() );
                        pSect->GetUpper()->MoveLowerFtns( pNxtCntnt, pOldBoss,
                                    pLayLeaf->FindFtnBossFrm( TRUE ), FALSE );
                    }
                }
                ((SwFlowFrm*)pNxt)->MoveSubTree( pLayLeaf, pNew->GetNext() );
            }
            if( pNew->GetFollow() )
                pNew->SimpleFormat();
        }
        // Das gesuchte Layoutblatt ist jetzt das erste des ermittelten SctFrms:
        pLayLeaf = FIRSTLEAF( pNew );
    }
    return pLayLeaf;
}

/*************************************************************************
|*
|*  SwFrm::GetPrevSctLeaf()
|*
|*  Beschreibung        Liefert das vorhergehende LayoutBlatt in das der
|*      Frame gemoved werden kann.
|*  Ersterstellung      AMA 07. Jan. 98
|*  Letzte Aenderung    AMA 07. Jan. 98
|*
|*************************************************************************/


SwLayoutFrm *SwFrm::GetPrevSctLeaf( MakePageType eMakeFtn )
{
    PROTOCOL_ENTER( this, PROT_LEAF, ACT_PREV_SECT, GetUpper()->FindSctFrm() )

    SwLayoutFrm* pCol;
    // ColumnFrm beinhalten jetzt stets einen BodyFrm
    if( IsColBodyFrm() )
        pCol = GetUpper();
    else if( GetUpper()->IsColBodyFrm() )
        pCol = GetUpper()->GetUpper();
    else
        pCol = NULL;
    BOOL bJump = FALSE;
    if( pCol )
    {
        if( pCol->GetPrev() )
        {
            do
            {
                pCol = (SwLayoutFrm*)pCol->GetPrev();
                // Gibt es dort Inhalt?
                if( ((SwLayoutFrm*)pCol->Lower())->Lower() )
                {
                    if( bJump )     // Haben wir eine leere Spalte uebersprungen?
                        SwFlowFrm::SetMoveBwdJump( TRUE );
                    return (SwLayoutFrm*)pCol->Lower();  // Der Spaltenbody
                }
                bJump = TRUE;
            } while( pCol->GetPrev() );

            // Hier landen wir, wenn alle Spalten leer sind,
            // pCol ist jetzt die erste Spalte, wir brauchen aber den Body:
            pCol = (SwLayoutFrm*)pCol->Lower();
        }
        else
            pCol = NULL;
    }

    if( bJump )     // Haben wir eine leere Spalte uebersprungen?
        SwFlowFrm::SetMoveBwdJump( TRUE );

    // Innerhalb von Bereichen in Tabellen oder Bereichen in Kopf/Fusszeilen kann
    // nur ein Spaltenwechsel erfolgen, eine der oberen Abkuerzungen haette
    // zuschlagen muessen, ebenso wenn der Bereich einen pPrev hat.
    // Jetzt ziehen wir sogar eine leere Spalte in Betracht...
    ASSERT( FindSctFrm(), "GetNextSctLeaf: Missing SectionFrm" );
    if( ( IsInTab() && !IsTabFrm() ) || FindFooterOrHeader() )
        return pCol;

    SwSectionFrm *pSect = FindSctFrm();
    SwFrm *pPrv;
    if( 0 != ( pPrv = pSect->GetIndPrev() ) )
    {
        // Herumlungernde, halbtote SectionFrms sollen uns nicht beirren
        while( pPrv && pPrv->IsSctFrm() && !((SwSectionFrm*)pPrv)->GetSection() )
            pPrv = pPrv->GetPrev();
        if( pPrv )
            return pCol;
    }

    const BOOL bBody = IsInDocBody();
    const BOOL bFly  = IsInFly();

    SwLayoutFrm *pLayLeaf = GetPrevLayoutLeaf();
    SwLayoutFrm *pPrevLeaf = 0;

    while ( pLayLeaf )
    {   //In Tabellen oder Bereiche geht's niemals hinein.
        if ( pLayLeaf->IsInTab() || pLayLeaf->IsInSct() )
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
        else if ( bBody && pLayLeaf->IsInDocBody() )
        {
            // If there is a pLayLeaf has a lower pLayLeaf is the frame we are looking for.
            // Exception: pLayLeaf->Lower() is a zombie section frame
            const SwFrm* pTmp = pLayLeaf->Lower();
            // OD 11.04.2003 #108824# - consider, that the zombie section frame
            // can have frame below it in the found layout leaf.
            // Thus, skipping zombie section frame, if possible.
            while ( pTmp && pTmp->IsSctFrm() &&
                    !( static_cast<const SwSectionFrm*>(pTmp)->GetSection() ) &&
                    pTmp->GetNext()
                  )
            {
                pTmp = pTmp->GetNext();
            }
            if ( pTmp &&
                 ( !pTmp->IsSctFrm() ||
                   ( static_cast<const SwSectionFrm*>(pTmp)->GetSection() )
                 )
               )
            {
                break;
            }
            pPrevLeaf = pLayLeaf;
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
            if ( pLayLeaf )
                SwFlowFrm::SetMoveBwdJump( TRUE );
        }
        else if ( bFly )
            break;  //Cntnts in Flys sollte jedes Layout-Blatt recht sein. Warum?
        else
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
    }
    if( !pLayLeaf )
    {
        if( !pPrevLeaf )
            return pCol;
        pLayLeaf = pPrevLeaf;
    }

    SwSectionFrm* pNew = NULL;
    // Zunaechst einmal an das Ende des Layoutblatts gehen
    SwFrm *pTmp = pLayLeaf->Lower();
    if( pTmp )
    {
        while( pTmp->GetNext() )
            pTmp = pTmp->GetNext();
        if( pTmp->IsSctFrm() )
        {
            // Halbtote stoeren hier nur...
            while( !((SwSectionFrm*)pTmp)->GetSection() && pTmp->GetPrev() &&
                    pTmp->GetPrev()->IsSctFrm() )
                pTmp = pTmp->GetPrev();
            if( ((SwSectionFrm*)pTmp)->GetFollow() == pSect )
                pNew = (SwSectionFrm*)pTmp;
        }
    }
    if( !pNew )
    {
        pNew = new SwSectionFrm( *pSect, TRUE );
        pNew->InsertBefore( pLayLeaf, NULL );
        pNew->Init();
        SWRECTFN( pNew )
        (pNew->*fnRect->fnMakePos)( pLayLeaf, pNew->GetPrev(), TRUE );

        pLayLeaf = FIRSTLEAF( pNew );
        if( !pNew->Lower() )    // einspaltige Bereiche formatieren
        {
            pNew->MakePos();
            pLayLeaf->Format(); // damit die PrtArea fuers MoveBwd stimmt
        }
        else
            pNew->SimpleFormat();
    }
    else
    {
        pLayLeaf = FIRSTLEAF( pNew );
        if( pLayLeaf->IsColBodyFrm() )
        {
            // In existent section columns we're looking for the last not empty
            // column.
            SwLayoutFrm *pTmp = pLayLeaf;
            while( pLayLeaf->GetUpper()->GetNext() )
            {
                pLayLeaf = (SwLayoutFrm*)((SwLayoutFrm*)pLayLeaf->GetUpper()->GetNext())->Lower();
                if( pLayLeaf->Lower() )
                    pTmp = pLayLeaf;
            }
            // If we skipped an empty column, we've to set the jump-flag
            if( pLayLeaf != pTmp )
            {
                pLayLeaf = pTmp;
                SwFlowFrm::SetMoveBwdJump( TRUE );
            }
        }
    }
    return pLayLeaf;
}

SwTwips lcl_DeadLine( const SwFrm* pFrm )
{
    const SwLayoutFrm* pUp = pFrm->GetUpper();
    while( pUp && pUp->IsInSct() )
    {
        if( pUp->IsSctFrm() )
            pUp = pUp->GetUpper();
        // Spalten jetzt mit BodyFrm
        else if( pUp->IsColBodyFrm() && pUp->GetUpper()->GetUpper()->IsSctFrm() )
            pUp = pUp->GetUpper()->GetUpper();
        else
            break;
    }
    SWRECTFN( pFrm )
    return pUp ? (pUp->*fnRect->fnGetPrtBottom)() :
                 (pFrm->Frm().*fnRect->fnGetBottom)();
}

// SwSectionFrm::Growable(..) prueft, ob der SectionFrm noch wachsen kann,
// ggf. muss die Umgebung gefragt werden

BOOL SwSectionFrm::Growable() const
{
    SWRECTFN( this )
    if( (*fnRect->fnYDiff)( lcl_DeadLine( this ),
        (Frm().*fnRect->fnGetBottom)() ) > 0 )
        return TRUE;

    return ( GetUpper() && ((SwFrm*)GetUpper())->Grow( LONG_MAX PHEIGHT, TRUE ) );
}

/*************************************************************************
|*
|*  SwSectionFrm::_Grow(), _Shrink()
|*
|*  Ersterstellung      AMA 14. Jan. 98
|*  Letzte Aenderung    AMA 14. Jan. 98
|*
|*************************************************************************/

SwTwips SwSectionFrm::_Grow( SwTwips nDist, BOOL bTst )
{
    if ( !IsColLocked() && !HasFixSize() )
    {
        SWRECTFN( this )
        long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
        if( nFrmHeight > 0 && nDist > (LONG_MAX - nFrmHeight) )
            nDist = LONG_MAX - nFrmHeight;

        if ( nDist <= 0L )
            return 0L;

        BOOL bInCalcCntnt = GetUpper() && IsInFly() && FindFlyFrm()->IsLocked();
        // OD 2004-03-15 #116561# - allow grow in online layout
        if ( !Lower() || !Lower()->IsColumnFrm() || !Lower()->GetNext() ||
             GetSection()->GetFmt()->GetBalancedColumns().GetValue() ||
             GetFmt()->GetDoc()->IsBrowseMode() )
        {
            SwTwips nGrow;
            if( IsInFtn() )
                nGrow = 0;
            else
            {
                nGrow = lcl_DeadLine( this );
                nGrow = (*fnRect->fnYDiff)( nGrow,
                                           (Frm().*fnRect->fnGetBottom)() );
            }
            SwTwips nSpace = nGrow;
            if( !bInCalcCntnt && nGrow < nDist && GetUpper() )
                nGrow += GetUpper()->Grow( LONG_MAX PHEIGHT, TRUE );

            if( nGrow > nDist )
                nGrow = nDist;
            if( nGrow <= 0 )
            {
                nGrow = 0;
                if( nDist && !bTst )
                {
                    if( bInCalcCntnt )
                        _InvalidateSize();
                    else
                        InvalidateSize();
                }
            }
            else if( !bTst )
            {
                if( bInCalcCntnt )
                    _InvalidateSize();
                else if( nSpace < nGrow &&  nDist != nSpace + GetUpper()->
                         Grow( nGrow - nSpace, FALSE ) )
                    InvalidateSize();
                else
                {
                    const SvxGraphicPosition ePos =
                        GetAttrSet()->GetBackground().GetGraphicPos();
                    if ( GPOS_RT < ePos && GPOS_TILED != ePos )
                    {
                        SetCompletePaint();
                        InvalidatePage();
                    }
                    if( GetUpper() && GetUpper()->IsHeaderFrm() )
                        GetUpper()->InvalidateSize();
                }
                (Frm().*fnRect->fnAddBottom)( nGrow );
                long nPrtHeight = (Prt().*fnRect->fnGetHeight)() + nGrow;
                (Prt().*fnRect->fnSetHeight)( nPrtHeight );

                if( Lower() && Lower()->IsColumnFrm() && Lower()->GetNext() )
                {
                    SwFrm* pTmp = Lower();
                    do
                    {
                        pTmp->_InvalidateSize();
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                    _InvalidateSize();
                }
                if( GetNext() )
                {
                    SwFrm *pFrm = GetNext();
                    while( pFrm && pFrm->IsSctFrm() && !((SwSectionFrm*)pFrm)->GetSection() )
                        pFrm = pFrm->GetNext();
                    if( pFrm )
                    {
                        if( bInCalcCntnt )
                            pFrm->_InvalidatePos();
                        else
                            pFrm->InvalidatePos();
                    }
                }
            }
            return nGrow;
        }
        if ( !bTst )
        {
            if( bInCalcCntnt )
                _InvalidateSize();
            else
                InvalidateSize();
        }
    }
    return 0L;
}

SwTwips SwSectionFrm::_Shrink( SwTwips nDist, BOOL bTst )
{
    if ( Lower() && !IsColLocked() && !HasFixSize() )
    {
        if( ToMaximize( FALSE ) )
        {
            if( !bTst )
                InvalidateSize();
        }
        else
        {
            SWRECTFN( this )
            long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
            if ( nDist > nFrmHeight )
                nDist = nFrmHeight;

            if ( Lower()->IsColumnFrm() && Lower()->GetNext() && // FtnAtEnd
                 !GetSection()->GetFmt()->GetBalancedColumns().GetValue() )
            {   //Bei Spaltigkeit ubernimmt das Format die Kontrolle ueber
                //das Wachstum (wg. des Ausgleichs).
                if ( !bTst )
                    InvalidateSize();
                return nDist;
            }
            else if( !bTst )
            {
                const SvxGraphicPosition ePos =
                    GetAttrSet()->GetBackground().GetGraphicPos();
                if ( GPOS_RT < ePos && GPOS_TILED != ePos )
                {
                    SetCompletePaint();
                    InvalidatePage();
                }
                (Frm().*fnRect->fnAddBottom)( -nDist );
                long nPrtHeight = (Prt().*fnRect->fnGetHeight)() - nDist;
                (Prt().*fnRect->fnSetHeight)( nPrtHeight );

                SwTwips nReal = 0;
                // We do not allow a section frame to shrink the its upper
                // footer frame. This is because in the calculation of a
                // footer frame, the content of the section frame is _not_
                // calculated. If there is a fly frame overlapping with the
                // footer frame, the section frame is not affected by this
                // during the calculation of the footer frame size.
                // The footer frame does not grow in its FormatSize function
                // but during the calculation of the content of the section
                // frame. The section frame grows until some of its text is
                // located on top of the fly frame. The next call of CalcCntnt
                // tries to shrink the section and here it would also shrink
                // the footer. This may not happen, because shrinking the footer
                // would cause the top of the section frame to overlap with the
                // fly frame again, this would result in a perfect loop.
                if( GetUpper() && !GetUpper()->IsFooterFrm() )
                    nReal = GetUpper()->Shrink( nDist, bTst );

                if( Lower() && Lower()->IsColumnFrm() && Lower()->GetNext() )
                {
                    SwFrm* pTmp = Lower();
                    do
                    {
                        pTmp->_InvalidateSize();
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                }
                if( GetNext() )
                {
                    SwFrm* pFrm = GetNext();
                    while( pFrm && pFrm->IsSctFrm() && !((SwSectionFrm*)pFrm)->GetSection() )
                        pFrm = pFrm->GetNext();
                    if( pFrm )
                        pFrm->InvalidatePos();
                    else
                        SetRetouche();
                }
                else
                    SetRetouche();
                return nDist;
            }
        }
    }
    return 0L;
}

/*************************************************************************
|*
|*  SwSectionFrm::MoveAllowed()
|*
|*  Ersterstellung      MA 08. Oct. 98
|*  Letzte Aenderung    MA 08. Oct. 98
|*
|*  Wann sind Frms innerhalb eines SectionFrms moveable?
|*  Wenn sie noch nicht in der letzten Spalte des SectionFrms sind,
|*  wenn es einen Follow gibt,
|*  wenn der SectionFrm nicht mehr wachsen kann, wird es komplizierter,
|*  dann kommt es darauf an, ob der SectionFrm ein naechstes Layoutblatt
|*  finden kann. In (spaltigen/verketteten) Flys wird dies via GetNextLayout
|*  geprueft, in Tabellen und in Kopf/Fusszeilen gibt es keins, im DocBody
|*  und auch im Fussnoten dagegen immer.
|*
|*  Benutzt wird diese Routine im TxtFormatter, um zu entscheiden, ob ein
|*  (Absatz-)Follow erzeugt werden darf oder ob der Absatz zusammenhalten muss.
|*
|*************************************************************************/

BOOL SwSectionFrm::MoveAllowed( const SwFrm* pFrm) const
{
    // Gibt es einen Follow oder ist der Frame nicht in der letzten Spalte?
    if( HasFollow() || ( pFrm->GetUpper()->IsColBodyFrm() &&
        pFrm->GetUpper()->GetUpper()->GetNext() ) )
        return TRUE;
    if( pFrm->IsInFtn() )
    {
        if( IsInFtn() )
        {
            if( GetUpper()->IsInSct() )
            {
                if( Growable() )
                    return FALSE;
                return GetUpper()->FindSctFrm()->MoveAllowed( this );
            }
            else
                return TRUE;
        }
        // The content of footnote inside a columned sectionfrm is moveable
        // except in the last column
        const SwLayoutFrm *pLay = pFrm->FindFtnFrm()->GetUpper()->GetUpper();
        if( pLay->IsColumnFrm() && pLay->GetNext() )
        {
            // The first paragraph in the first footnote in the first column
            // in the sectionfrm at the top of the page is not moveable,
            // if the columnbody is empty.
            BOOL bRet = FALSE;
            if( pLay->GetIndPrev() || pFrm->GetIndPrev() ||
                pFrm->FindFtnFrm()->GetPrev() )
                bRet = TRUE;
            else
            {
                SwLayoutFrm* pBody = ((SwColumnFrm*)pLay)->FindBodyCont();
                if( pBody && pBody->Lower() )
                    bRet = TRUE;
            }
            if( bRet && ( IsFtnAtEnd() || !Growable() ) )
                return TRUE;
        }
    }
    // Oder kann der Bereich noch wachsen?
    if( !IsColLocked() && Growable() )
        return FALSE;
    // Jetzt muss untersucht werden, ob es ein Layoutblatt gibt, in dem
    // ein Bereichsfollow erzeugt werden kann.
    if( IsInTab() || ( !IsInDocBody() && FindFooterOrHeader() ) )
        return FALSE; // In Tabellen/Kopf/Fusszeilen geht es nicht
    if( IsInFly() ) // Bei spaltigen oder verketteten Rahmen
        return 0 != ((SwFrm*)GetUpper())->GetNextLeaf( MAKEPAGE_NONE );
    return TRUE;
}

SwFrm* SwFrm::_GetIndPrev()
{
    SwFrm *pRet = NULL;
    ASSERT( !pPrev && IsInSct(), "Why?" );
    SwFrm* pSct = GetUpper();
    if( !pSct )
        return NULL;
    if( pSct->IsSctFrm() )
        pRet = pSct->GetIndPrev();
    else if( pSct->IsColBodyFrm() && (pSct = pSct->GetUpper()->GetUpper())->IsSctFrm() )
    {   // Wir duerfen nur den Vorgaenger des SectionFrms zurueckliefern,
        // wenn in keiner vorhergehenden Spalte mehr Inhalt ist
        SwFrm* pCol = GetUpper()->GetUpper()->GetPrev();
        while( pCol )
        {
            ASSERT( pCol->IsColumnFrm(), "GetIndPrev(): ColumnFrm expected" );
            ASSERT( pCol->GetLower() && pCol->GetLower()->IsBodyFrm(),
                    "GetIndPrev(): Where's the body?");
            if( ((SwLayoutFrm*)((SwLayoutFrm*)pCol)->Lower())->Lower() )
                return NULL;
            pCol = pCol->GetPrev();
        }
        pRet = pSct->GetIndPrev();
    }
    // Scheintote SectionFrames ueberspringen wir lieber
    while( pRet && pRet->IsSctFrm() && !((SwSectionFrm*)pRet)->GetSection() )
        pRet = pRet->GetIndPrev();
    return pRet;
}

SwFrm* SwFrm::_GetIndNext()
{
    ASSERT( !pNext && IsInSct(), "Why?" );
    SwFrm* pSct = GetUpper();
    if( !pSct )
        return NULL;
    if( pSct->IsSctFrm() )
        return pSct->GetIndNext();
    if( pSct->IsColBodyFrm() && (pSct = pSct->GetUpper()->GetUpper())->IsSctFrm() )
    {   // Wir duerfen nur den Nachfolger des SectionFrms zurueckliefern,
        // wenn in keiner folgenden Spalte mehr Inhalt ist
        SwFrm* pCol = GetUpper()->GetUpper()->GetNext();
        while( pCol )
        {
            ASSERT( pCol->IsColumnFrm(), "GetIndNext(): ColumnFrm expected" );
            ASSERT( pCol->GetLower() && pCol->GetLower()->IsBodyFrm(),
                    "GetIndNext(): Where's the body?");
            if( ((SwLayoutFrm*)((SwLayoutFrm*)pCol)->Lower())->Lower() )
                return NULL;
            pCol = pCol->GetNext();
        }
        return pSct->GetIndNext();
    }
    return NULL;
}

BOOL SwSectionFrm::IsAncestorOf( const SwSection* pSect ) const
{
    if( !pSection || !pSect )
        return FALSE;
    const SwSectionFmt *pFmt = pSect->GetFmt();
    const SwSectionFmt *pMyFmt = pSection->GetFmt();
    while( pFmt != pMyFmt )
    {
        if( pFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
            pFmt = (SwSectionFmt*)pFmt->GetRegisteredIn();
        else
            return FALSE;
    }
    return TRUE;
}

BOOL SwSectionFrm::IsDescendantFrom( const SwSectionFmt* pFmt ) const
{
    if( !pSection || !pFmt )
        return FALSE;
    const SwSectionFmt *pMyFmt = pSection->GetFmt();
    while( pFmt != pMyFmt )
    {
        if( pMyFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
            pMyFmt = (SwSectionFmt*)pMyFmt->GetRegisteredIn();
        else
            return FALSE;
    }
    return TRUE;
}

void SwSectionFrm::CalcFtnAtEndFlag()
{
    SwSectionFmt *pFmt = GetSection()->GetFmt();
    USHORT nVal = pFmt->GetFtnAtTxtEnd( FALSE ).GetValue();
    bFtnAtEnd = FTNEND_ATPGORDOCEND != nVal;
    bOwnFtnNum = FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
                 FTNEND_ATTXTEND_OWNNUMANDFMT == nVal;
    while( !bFtnAtEnd && !bOwnFtnNum )
    {
        if( pFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
            pFmt = (SwSectionFmt*)pFmt->GetRegisteredIn();
        else
            break;
        nVal = pFmt->GetFtnAtTxtEnd( FALSE ).GetValue();
        if( FTNEND_ATPGORDOCEND != nVal )
        {
            bFtnAtEnd = TRUE;
            bOwnFtnNum = bOwnFtnNum ||FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
                         FTNEND_ATTXTEND_OWNNUMANDFMT == nVal;
        }
    }
}

BOOL SwSectionFrm::IsEndnoteAtMyEnd() const
{
    return pSection->GetFmt()->GetEndAtTxtEnd( FALSE ).IsAtEnd();
}

void SwSectionFrm::CalcEndAtEndFlag()
{
    SwSectionFmt *pFmt = GetSection()->GetFmt();
    bEndnAtEnd = pFmt->GetEndAtTxtEnd( FALSE ).IsAtEnd();
    while( !bEndnAtEnd )
    {
        if( pFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
            pFmt = (SwSectionFmt*)pFmt->GetRegisteredIn();
        else
            break;
        bEndnAtEnd = pFmt->GetEndAtTxtEnd( FALSE ).IsAtEnd();
    }
}

/*************************************************************************
|*
|*  SwSectionFrm::Modify()
|*
|*  Ersterstellung      MA 08. Oct. 98
|*  Letzte Aenderung    MA 08. Oct. 98
|*
|*************************************************************************/

void SwSectionFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
{
    BYTE nInvFlags = 0;

    if( pNew && RES_ATTRSET_CHG == pNew->Which() )
    {
        SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
        SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
        SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );
        while( TRUE )
        {
            _UpdateAttr( (SfxPoolItem*)aOIter.GetCurItem(),
                         (SfxPoolItem*)aNIter.GetCurItem(), nInvFlags,
                         &aOldSet, &aNewSet );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
        if ( aOldSet.Count() || aNewSet.Count() )
            SwLayoutFrm::Modify( &aOldSet, &aNewSet );
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        if ( nInvFlags & 0x01 )
            InvalidateSize();
        if ( nInvFlags & 0x10 )
            SetCompletePaint();
    }
}

void SwSectionFrm::_UpdateAttr( SfxPoolItem *pOld, SfxPoolItem *pNew,
                            BYTE &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    BOOL bClear = TRUE;
    const USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {   // Mehrspaltigkeit in Fussnoten unterdruecken...
        case RES_FMT_CHG:
        {
            const SwFmtCol& rNewCol = GetFmt()->GetCol();
            if( !IsInFtn() )
            {
                //Dummer Fall. Bei der Zuweisung einer Vorlage k”nnen wir uns
                //nicht auf das alte Spaltenattribut verlassen. Da diese
                //wenigstens anzahlgemass fuer ChgColumns vorliegen muessen,
                //bleibt uns nur einen temporaeres Attribut zu basteln.
                SwFmtCol aCol;
                if ( Lower() && Lower()->IsColumnFrm() )
                {
                    USHORT nCol = 0;
                    SwFrm *pTmp = Lower();
                    do
                    {   ++nCol;
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                    aCol.Init( nCol, 0, 1000 );
                }
                BOOL bChgFtn = IsFtnAtEnd();
                BOOL bChgEndn = IsEndnAtEnd();
                BOOL bChgMyEndn = IsEndnoteAtMyEnd();
                CalcFtnAtEndFlag();
                CalcEndAtEndFlag();
                bChgFtn = ( bChgFtn != IsFtnAtEnd() ) ||
                          ( bChgEndn != IsEndnAtEnd() ) ||
                          ( bChgMyEndn != IsEndnoteAtMyEnd() );
                ChgColumns( aCol, rNewCol, bChgFtn );
                rInvFlags |= 0x10;
            }
            rInvFlags |= 0x01;
            bClear = FALSE;
        }
            break;

        case RES_COL:
            if( !IsInFtn() )
            {
                ChgColumns( *(const SwFmtCol*)pOld, *(const SwFmtCol*)pNew );
                rInvFlags |= 0x11;
            }
            break;

        case RES_FTN_AT_TXTEND:
            if( !IsInFtn() )
            {
                BOOL bOld = IsFtnAtEnd();
                CalcFtnAtEndFlag();
                if( bOld != IsFtnAtEnd() )
                {
                    const SwFmtCol& rNewCol = GetFmt()->GetCol();
                    ChgColumns( rNewCol, rNewCol, TRUE );
                    rInvFlags |= 0x01;
                }
            }
            break;

        case RES_END_AT_TXTEND:
            if( !IsInFtn() )
            {
                BOOL bOld = IsEndnAtEnd();
                BOOL bMyOld = IsEndnoteAtMyEnd();
                CalcEndAtEndFlag();
                if( bOld != IsEndnAtEnd() || bMyOld != IsEndnoteAtMyEnd())
                {
                    const SwFmtCol& rNewCol = GetFmt()->GetCol();
                    ChgColumns( rNewCol, rNewCol, TRUE );
                    rInvFlags |= 0x01;
                }
            }
            break;
        case RES_COLUMNBALANCE:
            rInvFlags |= 0x01;
            break;

        case RES_FRAMEDIR :
            SetDerivedR2L( sal_False );
            CheckDirChange();
            break;

        case RES_PROTECT:
            {
                ViewShell *pSh = GetShell();
                if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
                    pSh->Imp()->InvalidateAccessibleEditableState( sal_True, this );
            }
            break;

        default:
            bClear = FALSE;
    }
    if ( bClear )
    {
        if ( pOldSet || pNewSet )
        {
            if ( pOldSet )
                pOldSet->ClearItem( nWhich );
            if ( pNewSet )
                pNewSet->ClearItem( nWhich );
        }
        else
            SwLayoutFrm::Modify( pOld, pNew );
    }
}

/*-----------------09.06.99 14:58-------------------
 * SwSectionFrm::ToMaximize(..): A follow or a ftncontainer at the end of the
 * page causes a maximal Size of the sectionframe.
 * --------------------------------------------------*/

BOOL SwSectionFrm::ToMaximize( BOOL bCheckFollow ) const
{
    if( HasFollow() )
    {
        if( !bCheckFollow ) // Don't check superfluous follows
            return TRUE;
        const SwSectionFrm* pFoll = GetFollow();
        while( pFoll && pFoll->IsSuperfluous() )
            pFoll = pFoll->GetFollow();
        if( pFoll )
            return TRUE;
    }
    if( IsFtnAtEnd() )
        return FALSE;
    const SwFtnContFrm* pCont = ContainsFtnCont();
    if( !IsEndnAtEnd() )
        return 0 != pCont;
    BOOL bRet = FALSE;
    while( pCont && !bRet )
    {
        if( pCont->FindFootNote() )
            bRet = TRUE;
        else
            pCont = ContainsFtnCont( pCont );
    }
    return bRet;
}

/*-----------------09.06.99 15:07-------------------
 * BOOL SwSectionFrm::ContainsFtnCont()
 * checks every Column for FtnContFrms.
 * --------------------------------------------------*/

SwFtnContFrm* SwSectionFrm::ContainsFtnCont( const SwFtnContFrm* pCont ) const
{
    SwFtnContFrm* pRet = NULL;
    const SwLayoutFrm* pLay;
    if( pCont )
    {
        pLay = pCont->FindFtnBossFrm( 0 );
        ASSERT( IsAnLower( pLay ), "ConatainsFtnCont: Wrong FtnContainer" );
        pLay = (SwLayoutFrm*)pLay->GetNext();
    }
    else if( Lower() && Lower()->IsColumnFrm() )
        pLay = (SwLayoutFrm*)Lower();
    else
        pLay = NULL;
    while ( !pRet && pLay )
    {
        if( pLay->Lower() && pLay->Lower()->GetNext() )
        {
            ASSERT( pLay->Lower()->GetNext()->IsFtnContFrm(),
                    "ToMaximize: Unexspected Frame" );
            pRet = (SwFtnContFrm*)pLay->Lower()->GetNext();
        }
        ASSERT( !pLay->GetNext() || pLay->GetNext()->IsLayoutFrm(),
                "ToMaximize: ColFrm exspected" );
        pLay = (SwLayoutFrm*)pLay->GetNext();
    }
    return pRet;
}

void SwSectionFrm::InvalidateFtnPos()
{
    SwFtnContFrm* pCont = ContainsFtnCont( NULL );
    if( pCont )
    {
        SwFrm *pTmp = pCont->ContainsCntnt();
        if( pTmp )
            pTmp->_InvalidatePos();
    }
}

/*-----------------18.03.99 10:37-------------------
 * SwSectionFrm::Undersize() liefert den Betrag, um den der Bereich gern
 * groesser waere, wenn in ihm Undersized TxtFrms liegen, ansonsten Null.
 * Das Undersized-Flag wird ggf. korrigiert.
 * --------------------------------------------------*/

long SwSectionFrm::Undersize( BOOL bOverSize )
{
    bUndersized = FALSE;
    SWRECTFN( this )
    long nRet = InnerHeight() - (Prt().*fnRect->fnGetHeight)();
    if( nRet > 0 )
        bUndersized = TRUE;
    else if( !bOverSize )
        nRet = 0;
    return nRet;
}

/// OD 01.04.2003 #108446# - determine next frame for footnote/endnote formatting
/// before format of current one, because current one can move backward.
/// After moving backward to a previous page method <FindNext()> will return
/// the text frame presenting the first page footnote, if it exists. Thus, the
/// rest of the footnote/endnote container would not be formatted.
void SwSectionFrm::CalcFtnCntnt()
{
    SwFtnContFrm* pCont = ContainsFtnCont();
    if( pCont )
    {
        SwFrm* pFrm = pCont->ContainsAny();
        if( pFrm )
            pCont->Calc();
        while( pFrm && IsAnLower( pFrm ) )
        {
            SwFtnFrm* pFtn = pFrm->FindFtnFrm();
            if( pFtn )
                pFtn->Calc();
            // OD 01.04.2003 #108446# - determine next frame before format current frame.
            SwFrm* pNextFrm = 0;
            {
                if( pFrm->IsSctFrm() )
                {
                    pNextFrm = static_cast<SwSectionFrm*>(pFrm)->ContainsAny();
                }
                if( !pNextFrm )
                {
                    pNextFrm = pFrm->FindNext();
                }
            }
            pFrm->Calc();
            pFrm = pNextFrm;
        }
    }
}

/* -----------------09.02.99 14:26-------------------
 * Wenn ein SectionFrm leerlaeuft, z.B. weil sein Inhalt die Seite/Spalte wechselt,
 * so wird er nicht sofort zerstoert (es koennte noch jemand auf dem Stack einen Pointer
 * auf ihn halten), sondern er traegt sich in eine Liste am RootFrm ein, die spaeter
 * abgearbeitet wird (in LayAction::Action u.a.). Seine Groesse wird auf Null gesetzt und
 * sein Zeiger auf seine Section ebenfalls. Solche zum Loeschen vorgesehene SectionFrms
 * muessen vom Layout/beim Formatieren ignoriert werden.
 *
 * Mit InsertEmptySct nimmt der RootFrm einen SectionFrm in die Liste auf,
 * mit RemoveFromList kann ein SectionFrm wieder aus der Liste entfernt werden (Dtor),
 * mit DeleteEmptySct wird die Liste abgearbeitet und die SectionFrms zerstoert
 * --------------------------------------------------*/

void SwRootFrm::InsertEmptySct( SwSectionFrm* pDel )
{
    if( !pDestroy )
        pDestroy = new SwDestroyList;
    USHORT nPos;
    if( !pDestroy->Seek_Entry( pDel, &nPos ) )
        pDestroy->Insert( pDel );
}

void SwRootFrm::_DeleteEmptySct()
{
    ASSERT( pDestroy, "Keine Liste, keine Kekse" );
    while( pDestroy->Count() )
    {
        SwSectionFrm* pSect = (*pDestroy)[0];
        pDestroy->Remove( USHORT(0) );
        ASSERT( !pSect->IsColLocked() && !pSect->IsJoinLocked(),
                "DeleteEmptySct: Locked SectionFrm" );
        if( !pSect->Frm().HasArea() && !pSect->ContainsCntnt() )
        {
            SwLayoutFrm* pUp = pSect->GetUpper();
            pSect->Remove();
            delete pSect;
            if( pUp && !pUp->Lower() )
            {
                if( pUp->IsPageBodyFrm() )
                    pUp->FindRootFrm()->SetSuperfluous();
                else if( pUp->IsFtnFrm() && !pUp->IsColLocked() &&
                    pUp->GetUpper() )
                {
                    pUp->Cut();
                    delete pUp;
                }
            }
        }
        else
            ASSERT( pSect->GetSection(), "DeleteEmptySct: Halbtoter SectionFrm?!" );
    }
}

void SwRootFrm::_RemoveFromList( SwSectionFrm* pSct )
{
    ASSERT( pDestroy, "Where's my list?" );
    USHORT nPos;
    if( pDestroy->Seek_Entry( pSct, &nPos ) )
        pDestroy->Remove( nPos );
}

#ifndef PRODUCT

BOOL SwRootFrm::IsInDelList( SwSectionFrm* pSct ) const
{
    USHORT nPos;
    return ( pDestroy && pDestroy->Seek_Entry( pSct, &nPos ) );
}

#endif
