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

#include <svl/smplhint.hxx>
#include <svl/itemiter.hxx>
#include <hints.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <fmtclbl.hxx>
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
#include "viewopt.hxx"
#include "viewimp.hxx"
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brshitem.hxx>
#include <fmtftntx.hxx>
// OD 2004-05-24 #i28701#
#include <dflyobj.hxx>
#include <flyfrms.hxx>
#include <sortedobjs.hxx>

SV_IMPL_PTRARR_SORT( SwDestroyList, SwSectionFrmPtr )

/*************************************************************************
|*
|*  SwSectionFrm::SwSectionFrm(), ~SwSectionFrm()
|*
|*  Ersterstellung      AMA 26. Nov. 97
|*  Letzte Aenderung    AMA 26. Nov. 97
|*
|*************************************************************************/
SwSectionFrm::SwSectionFrm( SwSection &rSect, SwFrm* pSib ) :
    SwLayoutFrm( rSect.GetFmt(), pSib ),
    SwFlowFrm( (SwFrm&)*this ),
    pSection( &rSect )
{
    nType = FRMC_SECTION;

    CalcFtnAtEndFlag();
    CalcEndAtEndFlag();
}

SwSectionFrm::SwSectionFrm( SwSectionFrm &rSect, sal_Bool bMaster ) :
    SwLayoutFrm( rSect.GetFmt(), rSect.getRootFrm() ),
    SwFlowFrm( (SwFrm&)*this ),
    pSection( rSect.GetSection() )
{
    bFtnAtEnd = rSect.IsFtnAtEnd();
    bEndnAtEnd = rSect.IsEndnAtEnd();
    bLockJoin = sal_False;
    nType = FRMC_SECTION;

    PROTOCOL( this, PROT_SECTION, bMaster ? ACT_CREATE_MASTER : ACT_CREATE_FOLLOW, &rSect )

    if( bMaster )
    {
        if( rSect.IsFollow() )
        {
            SwSectionFrm* pMaster = rSect.FindMaster();
            pMaster->SetFollow( this );
            bIsFollow = sal_True;
        }
        else
            rSect.bIsFollow = sal_True;
        SetFollow( &rSect );
    }
    else
    {
        bIsFollow = sal_True;
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
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm )
            pRootFrm->RemoveFromList( this );   //swmod 071108//swmod 071225
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
            GetFollow()->bIsFollow = sal_False;
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
void SwSectionFrm::DelEmpty( sal_Bool bRemove )
{
    if( IsColLocked() )
    {
        ASSERT( !bRemove, "Don't delete locked SectionFrms" );
        return;
    }
    SwFrm* pUp = GetUpper();
    if( pUp )
    {
        // --> OD 2005-12-01 #i27138#
        // notify accessibility paragraphs objects about changed
        // CONTENT_FLOWS_FROM/_TO relation.
        // Relation CONTENT_FLOWS_FROM for current next paragraph will change
        // and relation CONTENT_FLOWS_TO for current previous paragraph will change.
        {
            ViewShell* pViewShell( getRootFrm()->GetCurrShell() );
            if ( pViewShell && pViewShell->GetLayout() &&
                 pViewShell->GetLayout()->IsAnyShellAccessible() )
            {
                pViewShell->InvalidateAccessibleParaFlowRelation(
                                dynamic_cast<SwTxtFrm*>(FindNextCnt( true )),
                                dynamic_cast<SwTxtFrm*>(FindPrevCnt( true )) );
            }
        }
        // <--
        _Cut( bRemove );
    }
    if( IsFollow() )
    {
        SwSectionFrm *pMaster = FindMaster();
        pMaster->SetFollow( GetFollow() );
        // Ein Master greift sich immer den Platz bis zur Unterkante seines
        // Uppers. Wenn er keinen Follow mehr hat, kann er diesen ggf. wieder
        // freigeben, deshalb wird die Size des Masters invalidiert.
        if( !GetFollow() && !pMaster->IsColLocked() )
            pMaster->InvalidateSize();
        bIsFollow = sal_False;
    }
    else if( HasFollow() )
        GetFollow()->bIsFollow = sal_False;
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
            if( !pSection && getRootFrm() )
                getRootFrm()->RemoveFromList( this );
        }
        else if( getRootFrm() )
            getRootFrm()->InsertEmptySct( this );   //swmod 071108//swmod 071225
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
    _Cut( sal_True );
}

void SwSectionFrm::_Cut( sal_Bool bRemove )
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
    // --> OD 2008-06-23 #156927#
    // Assure that parent is not inside a table frame, which is inside the found section frame.
    if ( pSect )
    {
        SwTabFrm* pTableFrm = pParent->FindTabFrm();
        if ( pTableFrm &&
             pSect->IsAnLower( pTableFrm ) )
        {
            pSect = 0;
        }
    }
    // <--

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
        pSect = new SwSectionFrm( *((SwSectionFrm*)pParent)->GetSection(), pParent );
        // Wenn pParent in zwei Teile zerlegt wird, so muss sein Follow am
        // neuen, zweiten Teil angebracht werden.
        pSect->SetFollow( ((SwSectionFrm*)pParent)->GetFollow() );
        ((SwSectionFrm*)pParent)->SetFollow( NULL );
        if( pSect->GetFollow() )
            pParent->_InvalidateSize();

        InsertGroupBefore( pParent, pSibling, pSect );
        pSect->Init();
        (pSect->*fnRect->fnMakePos)( pSect->GetUpper(), pSect->GetPrev(), sal_True);
        if( !((SwLayoutFrm*)pParent)->Lower() )
        {
            SwSectionFrm::MoveCntntAndDelete( (SwSectionFrm*)pParent, sal_False );
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

sal_Bool SwSectionFrm::HasToBreak( const SwFrm* pFrm ) const
{
    if( !pFrm->IsSctFrm() )
        return sal_False;

    SwSectionFmt *pTmp = (SwSectionFmt*)GetFmt();
//  if( !pTmp->GetSect().GetValue() )
//      return sal_False;

    const SwFrmFmt *pOtherFmt = ((SwSectionFrm*)pFrm)->GetFmt();
    do
    {
        pTmp = pTmp->GetParent();
        if( !pTmp )
            return sal_False;
        if( pTmp == pOtherFmt )
            return sal_True;
    } while( sal_True ); // ( pTmp->GetSect().GetValue() );
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
        pNxt->bIsFollow = sal_False;
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

sal_Bool SwSectionFrm::SplitSect( SwFrm* pFrm, sal_Bool bApres )
{
    ASSERT( pFrm, "SplitSect: Why?" );
    SwFrm* pOther = bApres ? pFrm->FindNext() : pFrm->FindPrev();
    if( !pOther )
        return sal_False;
    SwSectionFrm* pSect = pOther->FindSctFrm();
    if( pSect != this )
        return sal_False;
    // Den Inhalt zur Seite stellen
    SwFrm* pSav = ::SaveCntnt( this, bApres ? pOther : pFrm );
    ASSERT( pSav, "SplitSect: What's on?" );
    if( pSav ) // Robust
    {   // Einen neuen SctFrm anlegen, nicht als Follow/Master
        SwSectionFrm* pNew = new SwSectionFrm( *pSect->GetSection(), pSect );
        pNew->InsertBehind( pSect->GetUpper(), pSect );
        pNew->Init();
        SWRECTFN( this )
        (pNew->*fnRect->fnMakePos)( NULL, pSect, sal_True );
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
        return sal_True;
    }
    return sal_False;
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

void lcl_InvalidateInfFlags( SwFrm* pFrm, sal_Bool bInva )
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
            lcl_InvalidateInfFlags( ((SwLayoutFrm*)pFrm)->GetLower(), sal_False );
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
    sal_Bool bGoingUp = sal_True;
    do {
        const SwFrm *p = 0;
        sal_Bool bGoingFwdOrBwd = sal_False, bGoingDown = sal_False;

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

void SwSectionFrm::MoveCntntAndDelete( SwSectionFrm* pDel, sal_Bool bSave )
{
    sal_Bool bSize = pDel->Lower() && pDel->Lower()->IsColumnFrm();
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
    sal_Bool bOldFtn = sal_True;
    if( pSave && pUp->IsFtnFrm() )
    {
        bOldFtn = ((SwFtnFrm*)pUp)->IsColLocked();
        ((SwFtnFrm*)pUp)->ColLock();
    }
    pDel->DelEmpty( sal_True );
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
                pUp = static_cast<SwLayoutFrm*>(pUp->GetLastLower());
                // Der Body der letzten Spalte
                pUp = static_cast<SwLayoutFrm*>(pUp->Lower());
            }
            // damit hinter dem letzten eingefuegt wird
            pPrv = pUp->GetLastLower();
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
                pPrvSct = new SwSectionFrm( *pParent->GetSection(), pUp );
                pPrvSct->InsertBehind( pUp, pPrv );
                pPrvSct->Init();
                SWRECTFN( pUp )
                (pPrvSct->*fnRect->fnMakePos)( pUp, pPrv, sal_True );
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
        ASSERT( getRootFrm()->IsInDelList( this ), "SectionFrm without Section" );
        if( !bValidPos )
        {
            if( GetUpper() )
            {
                SWRECTFN( GetUpper() )
                (this->*fnRect->fnMakePos)( GetUpper(), GetPrev(), sal_False );
            }
        }
        bValidSize = bValidPos = bValidPrtArea = sal_True;
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
    const ViewShell *pSh = getRootFrm()->GetCurrShell();
    if( pSh && pSh->GetViewOptions()->getBrowseMode() &&
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
    if( !bValidPos && ToMaximize( sal_False ) )
        bValidSize = sal_False;

#if OSL_DEBUG_LEVEL > 1
    const SwFmtCol &rCol = GetFmt()->GetCol();
    (void)rCol;
#endif
    SwLayoutFrm::MakeAll();
    UnlockJoin();
    if( pSection && IsSuperfluous() )
        DelEmpty( sal_False );
}

sal_Bool SwSectionFrm::ShouldBwdMoved( SwLayoutFrm *, sal_Bool , sal_Bool & )
{
    ASSERT( sal_False, "Hups, wo ist meine Tarnkappe?" );
    return sal_False;
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
    SwFrm* pFrm, sal_Bool &rbChkFtn )
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

SwCntntFrm *SwSectionFrm::FindLastCntnt( sal_uInt8 nMode )
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
        } while( sal_True );
    }
    sal_Bool bFtnFound = nMode == FINDMODE_ENDNOTE;
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

sal_Bool SwSectionFrm::CalcMinDiff( SwTwips& rMinDiff ) const
{
    if( ToMaximize( sal_True ) )
    {
        SWRECTFN( this )
        rMinDiff = (GetUpper()->*fnRect->fnGetPrtBottom)();
        rMinDiff = (Frm().*fnRect->fnBottomDist)( rMinDiff );
        return sal_True;
    }
    return sal_False;
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

SwFtnFrm* lcl_FindEndnote( SwSectionFrm* &rpSect, sal_Bool &rbEmpty,
    SwLayouter *pLayouter )
{
    // if rEmpty is set, the rpSect is already searched
    SwSectionFrm* pSect = rbEmpty ? rpSect->GetFollow() : rpSect;
    while( pSect )
    {
       ASSERT( (pSect->Lower() && pSect->Lower()->IsColumnFrm()) || pSect->GetUpper()->IsFtnFrm(),
                "InsertEndnotes: Where's my column?" );

        // i73332: Columned section in endnote
        SwColumnFrm* pCol = 0;
        if(pSect->Lower() && pSect->Lower()->IsColumnFrm())
            pCol = (SwColumnFrm*)pSect->Lower();

        while( pCol ) // check all columns
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
        }
        rpSect = pSect;
        pSect = pLayouter ? pSect->GetFollow() : NULL;
        rbEmpty = sal_True;
    }
    return NULL;
}

void lcl_ColumnRefresh( SwSectionFrm* pSect, sal_Bool bFollow )
{
    while( pSect )
    {
        sal_Bool bOldLock = pSect->IsColLocked();
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
    // i73332: Section in footnode does not have columns!
    ASSERT( (Lower() && Lower()->IsColumnFrm()) || GetUpper()->IsFtnFrm(), "Where's my column?" );

    SwSectionFrm* pSect = this;
    SwFtnFrm* pFtn;
    sal_Bool bEmpty = sal_False;
    // pSect is the last sectionfrm without endnotes or the this-pointer
    // the first sectionfrm with endnotes may be destroyed, when the endnotes
    // is cutted
    while( 0 != (pFtn = lcl_FindEndnote( pSect, bEmpty, pLayouter )) )
        pLayouter->CollectEndnote( pFtn );
    if( pLayouter->HasEndnotes() )
        lcl_ColumnRefresh( this, sal_True );
}

/*************************************************************************
|*
|*  SwSectionFrm::_CheckClipping( sal_Bool bGrow, sal_Bool bMaximize )
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
void SwSectionFrm::_CheckClipping( sal_Bool bGrow, sal_Bool bMaximize )
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
        sal_Bool bEmpty = sal_False;
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
                lcl_ColumnRefresh( this, sal_False );
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
        // --> OD 2009-09-28 #b6882166#
        // assure notifications on position changes.
        const SwLayNotify aNotify( this );
        // <--
        (this->*fnRect->fnMakePos)( GetUpper(), GetPrev(), sal_False );
        bValidPos = sal_True;
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
    lcl_ColumnRefresh( this, sal_False );
    UnlockJoin();
}

// --> OD 2005-01-11 #i40147# - helper class to perform extra section format
// to position anchored objects and to keep the position of whose objects locked.
class ExtraFormatToPositionObjs
{
    private:
        SwSectionFrm* mpSectFrm;
        bool mbExtraFormatPerformed;

    public:
        ExtraFormatToPositionObjs( SwSectionFrm& _rSectFrm)
            : mpSectFrm( &_rSectFrm ),
              mbExtraFormatPerformed( false )
        {}

        ~ExtraFormatToPositionObjs()
        {
            if ( mbExtraFormatPerformed )
            {
                // release keep locked position of lower floating screen objects
                SwPageFrm* pPageFrm = mpSectFrm->FindPageFrm();
                SwSortedObjs* pObjs = pPageFrm ? pPageFrm->GetSortedObjs() : 0L;
                if ( pObjs )
                {
                    sal_uInt32 i = 0;
                    for ( i = 0; i < pObjs->Count(); ++i )
                    {
                        SwAnchoredObject* pAnchoredObj = (*pObjs)[i];

                        if ( mpSectFrm->IsAnLower( pAnchoredObj->GetAnchorFrm() ) )
                        {
                            pAnchoredObj->SetKeepPosLocked( false );
                        }
                    }
                }
            }
        }

        // --> OD 2008-06-20 #i81555#
        void InitObjs( SwFrm& rFrm )
        {
            SwSortedObjs* pObjs = rFrm.GetDrawObjs();
            if ( pObjs )
            {
                sal_uInt32 i = 0;
                for ( i = 0; i < pObjs->Count(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pObjs)[i];

                    pAnchoredObj->UnlockPosition();
                    pAnchoredObj->SetClearedEnvironment( false );
                }
            }
            SwLayoutFrm* pLayoutFrm = dynamic_cast<SwLayoutFrm*>(&rFrm);
            if ( pLayoutFrm != 0 )
            {
                SwFrm* pLowerFrm = pLayoutFrm->GetLower();
                while ( pLowerFrm != 0 )
                {
                    InitObjs( *pLowerFrm );

                    pLowerFrm = pLowerFrm->GetNext();
                }
            }
        }
        // <--

        void FormatSectionToPositionObjs()
        {
            // perform extra format for multi-columned section.
            if ( mpSectFrm->Lower() && mpSectFrm->Lower()->IsColumnFrm() &&
                 mpSectFrm->Lower()->GetNext() )
            {
                // grow section till bottom of printing area of upper frame
                SWRECTFN( mpSectFrm );
                SwTwips nTopMargin = (mpSectFrm->*fnRect->fnGetTopMargin)();
                Size aOldSectPrtSize( mpSectFrm->Prt().SSize() );
                SwTwips nDiff = (mpSectFrm->Frm().*fnRect->fnBottomDist)(
                                        (mpSectFrm->GetUpper()->*fnRect->fnGetPrtBottom)() );
                (mpSectFrm->Frm().*fnRect->fnAddBottom)( nDiff );
                (mpSectFrm->*fnRect->fnSetYMargins)( nTopMargin, 0 );
                // --> OD 2006-05-08 #i59789#
                // suppress formatting, if printing area of section is too narrow
                if ( (mpSectFrm->Prt().*fnRect->fnGetHeight)() <= 0 )
                {
                    return;
                }
                // <--
                mpSectFrm->ChgLowersProp( aOldSectPrtSize );

                // format column frames and its body and footnote container
                SwColumnFrm* pColFrm = static_cast<SwColumnFrm*>(mpSectFrm->Lower());
                while ( pColFrm )
                {
                    pColFrm->Calc();
                    pColFrm->Lower()->Calc();
                    if ( pColFrm->Lower()->GetNext() )
                    {
                        pColFrm->Lower()->GetNext()->Calc();
                    }

                    pColFrm = static_cast<SwColumnFrm*>(pColFrm->GetNext());
                }

                // unlock position of lower floating screen objects for the extra format
                // --> OD 2008-06-20 #i81555#
                // Section frame can already have changed the page and its content
                // can still be on the former page.
                // Thus, initialize objects via lower-relationship
                InitObjs( *mpSectFrm );
                // <--

                // format content - first with collecting its foot-/endnotes before content
                // format, second without collecting its foot-/endnotes.
                ::CalcCntnt( mpSectFrm );
                ::CalcCntnt( mpSectFrm, true );

                // keep locked position of lower floating screen objects
                SwPageFrm* pPageFrm = mpSectFrm->FindPageFrm();
                SwSortedObjs* pObjs = pPageFrm ? pPageFrm->GetSortedObjs() : 0L;
                if ( pObjs )
                {
                    sal_uInt32 i = 0;
                    for ( i = 0; i < pObjs->Count(); ++i )
                    {
                        SwAnchoredObject* pAnchoredObj = (*pObjs)[i];

                        if ( mpSectFrm->IsAnLower( pAnchoredObj->GetAnchorFrm() ) )
                        {
                            pAnchoredObj->SetKeepPosLocked( true );
                        }
                    }
                }

                mbExtraFormatPerformed = true;
            }
        }
};

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
        ASSERT( getRootFrm()->IsInDelList( this ), "SectionFrm without Section" );
        bValidSize = bValidPos = bValidPrtArea = sal_True;
        return;
    }
    SWRECTFN( this )
    if ( !bValidPrtArea )
    {
        PROTOCOL( this, PROT_PRTAREA, 0, 0 )
        bValidPrtArea = sal_True;
        SwTwips nUpper = CalcUpperSpace();

        // #109700# LRSpace for sections
        const SvxLRSpaceItem& rLRSpace = GetFmt()->GetLRSpace();
        (this->*fnRect->fnSetXMargins)( rLRSpace.GetLeft(), rLRSpace.GetRight() );

        if( nUpper != (this->*fnRect->fnGetTopMargin)() )
        {
            bValidSize = sal_False;
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
        sal_Bool bOldLock = IsColLocked();
        ColLock();

        bValidSize = sal_True;

        //die Groesse wird nur dann vom Inhalt bestimmt, wenn der SectFrm
        //keinen Follow hat. Anderfalls fuellt er immer den Upper bis
        //zur Unterkante aus. Fuer den Textfluss ist nicht er, sondern sein
        //Inhalt selbst verantwortlich.
        sal_Bool bMaximize = ToMaximize( sal_False );

        // OD 2004-05-17 #i28701# - If the wrapping style has to be considered
        // on object positioning, an extra formatting has to be performed
        // to determine the correct positions the floating screen objects.
        // --> OD 2005-01-11 #i40147#
        // use new helper class <ExtraFormatToPositionObjs>.
        // This class additionally keep the locked position of the objects
        // and releases this position lock keeping on destruction.
        ExtraFormatToPositionObjs aExtraFormatToPosObjs( *this );
        if ( !bMaximize &&
             GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) &&
             !GetFmt()->GetBalancedColumns().GetValue() )
        {
            aExtraFormatToPosObjs.FormatSectionToPositionObjs();
        }
        // <--

        // Column widths have to be adjusted before calling _CheckClipping.
        // _CheckClipping can cause the formatting of the lower frames
        // which still have a width of 0.
        const sal_Bool bHasColumns = Lower() && Lower()->IsColumnFrm();
        if ( bHasColumns && Lower()->GetNext() )
            AdjustColumns( 0, sal_False );

        if( GetUpper() )
        {
            long nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
            (aFrm.*fnRect->fnSetWidth)( nWidth );

            // #109700# LRSpace for sections
            const SvxLRSpaceItem& rLRSpace = GetFmt()->GetLRSpace();
            (aPrt.*fnRect->fnSetWidth)( nWidth - rLRSpace.GetLeft() -
                                        rLRSpace.GetRight() );

            // OD 15.10.2002 #103517# - allow grow in online layout
            // Thus, set <..IsBrowseMode()> as parameter <bGrow> on calling
            // method <_CheckClipping(..)>.
            const ViewShell *pSh = getRootFrm()->GetCurrShell();
            _CheckClipping( pSh && pSh->GetViewOptions()->getBrowseMode(), bMaximize );
            bMaximize = ToMaximize( sal_False );
            bValidSize = sal_True;
        }

        //Breite der Spalten pruefen und ggf. einstellen.
        if ( bHasColumns && ! Lower()->GetNext() && bMaximize )
            ((SwColumnFrm*)Lower())->Lower()->Calc();

        if ( !bMaximize )
        {
            SwTwips nRemaining = (this->*fnRect->fnGetTopMargin)();
            SwFrm *pFrm = pLower;
            if( pFrm )
            {
                if( pFrm->IsColumnFrm() && pFrm->GetNext() )
                {
                    // --> OD 2006-05-08 #i61435#
                    // suppress formatting, if upper frame has height <= 0
                    if ( (GetUpper()->Frm().*fnRect->fnGetHeight)() > 0 )
                    {
                        FormatWidthCols( *pAttr, nRemaining, MINLAY );
                    }
                    // <--
                    // --> OD 2006-01-04 #126020# - adjust check for empty section
                    // --> OD 2006-02-01 #130797# - correct fix #126020#
                    while( HasFollow() && !GetFollow()->ContainsCntnt() &&
                           !GetFollow()->ContainsAny( true ) )
                    // <--
                    {
                        SwFrm* pOld = GetFollow();
                        GetFollow()->DelEmpty( sal_False );
                        if( pOld == GetFollow() )
                            break;
                    }
                    bMaximize = ToMaximize( sal_False );
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

            SwTwips nDiff = (Frm().*fnRect->fnGetHeight)() - nRemaining;
            if( nDiff < 0)
            {
                SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
                {
                    long nBottom = (Frm().*fnRect->fnGetBottom)();
                    nBottom = (*fnRect->fnYInc)( nBottom, -nDiff );
                    long nTmpDiff = (*fnRect->fnYDiff)( nBottom, nDeadLine );
                    if( nTmpDiff > 0 )
                    {
                        nTmpDiff = GetUpper()->Grow( nTmpDiff, sal_True );
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
                    sal_Bool bUnderSz = sal_False;
                    while( pFrm )
                    {
                        if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
                        {
                            pFrm->Prepare( PREP_ADJUST_FRM );
                            bUnderSz = sal_True;
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
            _CheckClipping( sal_True, bMaximize );
        if( !bOldLock )
            ColUnlock();
        long nDiff = nOldHeight - (Frm().*fnRect->fnGetHeight)();
        if( nDiff > 0 )
        {
            if( !GetNext() )
                SetRetouche(); // Dann muessen wir die Retusche selbst uebernehmen
            if( GetUpper() && !GetUpper()->IsFooterFrm() )
                GetUpper()->Shrink( nDiff );
        }
        if( IsUndersized() )
            bValidPrtArea = sal_True;
    }
}

/*************************************************************************
|*
|*  SwFrm::GetNextSctLeaf()
|*
|*  Beschreibung        Liefert das naechste Layoutblatt in das der Frame
|*      gemoved werden kann.
|*      Neue Seiten werden nur dann erzeugt, wenn der Parameter sal_True ist.
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
    if( GetUpper()->IsInTab() || FindFooterOrHeader() )
        return 0;

//MA 03. Feb. 99: Warum GetUpper()? Das knallt mit Buch.sgl weil im
//FlyAtCnt::MakeFlyPos ein Orient der SectionFrm ist und auf diesen ein
//GetLeaf gerufen wird.
//  SwSectionFrm *pSect = GetUpper()->FindSctFrm();
    SwSectionFrm *pSect = FindSctFrm();
    sal_Bool bWrongPage = sal_False;
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
                bWrongPage = sal_True;
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
                        bWrongPage = sal_True;
                    else
                        return FIRSTLEAF( pSect->GetFollow() );
                }
            }
        }
    }

    // Immer im gleichen Bereich landen: Body wieder in Body etc.
    const sal_Bool bBody = IsInDocBody();
    const sal_Bool bFtnPage = FindPageFrm()->IsFtnPage();

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

    while( sal_True )
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
                bWrongPage = sal_True;
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
                       sal_False );
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
            pNew = new SwSectionFrm( *pSect, sal_False );
            pNew->InsertBefore( pLayLeaf, pLayLeaf->Lower() );
            pNew->Init();
            SWRECTFN( pNew )
            (pNew->*fnRect->fnMakePos)( pLayLeaf, NULL, sal_True );

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
                    SwFtnBossFrm* pOldBoss = pSect->FindFtnBossFrm( sal_True );
                    if( pOldBoss == pNxtCntnt->FindFtnBossFrm( sal_True ) )
                    {
                        SwSaveFtnHeight aHeight( pOldBoss,
                            pOldBoss->Frm().Top() + pOldBoss->Frm().Height() );
                        pSect->GetUpper()->MoveLowerFtns( pNxtCntnt, pOldBoss,
                                    pLayLeaf->FindFtnBossFrm( sal_True ), sal_False );
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


SwLayoutFrm *SwFrm::GetPrevSctLeaf( MakePageType )
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
    sal_Bool bJump = sal_False;
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
                        SwFlowFrm::SetMoveBwdJump( sal_True );
                    return (SwLayoutFrm*)pCol->Lower();  // Der Spaltenbody
                }
                bJump = sal_True;
            } while( pCol->GetPrev() );

            // Hier landen wir, wenn alle Spalten leer sind,
            // pCol ist jetzt die erste Spalte, wir brauchen aber den Body:
            pCol = (SwLayoutFrm*)pCol->Lower();
        }
        else
            pCol = NULL;
    }

    if( bJump )     // Haben wir eine leere Spalte uebersprungen?
        SwFlowFrm::SetMoveBwdJump( sal_True );

    // Innerhalb von Bereichen in Tabellen oder Bereichen in Kopf/Fusszeilen kann
    // nur ein Spaltenwechsel erfolgen, eine der oberen Abkuerzungen haette
    // zuschlagen muessen, ebenso wenn der Bereich einen pPrev hat.
    // Jetzt ziehen wir sogar eine leere Spalte in Betracht...
    ASSERT( FindSctFrm(), "GetNextSctLeaf: Missing SectionFrm" );
    if( ( IsInTab() && !IsTabFrm() ) || FindFooterOrHeader() )
        return pCol;

    // === IMPORTANT ===
    // Precondition, which needs to be hold, is that the <this> frame can be
    // inside a table, but then the found section frame <pSect> is also inside
    // this table.
    SwSectionFrm *pSect = FindSctFrm();

    // --> OD 2009-01-16 #i95698#
    // A table cell containing directly a section does not break - see lcl_FindSectionsInRow(..)
    // Thus, a table inside a section, which is inside another table can only
    // flow backward in the columns of its section.
    // Note: The table cell, which contains the section, can not have a master table cell.
    if ( IsTabFrm() && pSect->IsInTab() )
    {
        return pCol;
    }
    // <--

    {
        SwFrm *pPrv;
        if( 0 != ( pPrv = pSect->GetIndPrev() ) )
        {
            // Herumlungernde, halbtote SectionFrms sollen uns nicht beirren
            while( pPrv && pPrv->IsSctFrm() && !((SwSectionFrm*)pPrv)->GetSection() )
                pPrv = pPrv->GetPrev();
            if( pPrv )
                return pCol;
        }
    }

    const sal_Bool bBody = IsInDocBody();
    const sal_Bool bFly  = IsInFly();

    SwLayoutFrm *pLayLeaf = GetPrevLayoutLeaf();
    SwLayoutFrm *pPrevLeaf = 0;

    while ( pLayLeaf )
    {
        //In Tabellen oder Bereiche geht's niemals hinein.
        if ( pLayLeaf->IsInTab() || pLayLeaf->IsInSct() )
        {
            pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
        }
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
                SwFlowFrm::SetMoveBwdJump( sal_True );
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
        pNew = new SwSectionFrm( *pSect, sal_True );
        pNew->InsertBefore( pLayLeaf, NULL );
        pNew->Init();
        SWRECTFN( pNew )
        (pNew->*fnRect->fnMakePos)( pLayLeaf, pNew->GetPrev(), sal_True );

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
            SwLayoutFrm *pTmpLay = pLayLeaf;
            while( pLayLeaf->GetUpper()->GetNext() )
            {
                pLayLeaf = (SwLayoutFrm*)((SwLayoutFrm*)pLayLeaf->GetUpper()->GetNext())->Lower();
                if( pLayLeaf->Lower() )
                    pTmpLay = pLayLeaf;
            }
            // If we skipped an empty column, we've to set the jump-flag
            if( pLayLeaf != pTmpLay )
            {
                pLayLeaf = pTmpLay;
                SwFlowFrm::SetMoveBwdJump( sal_True );
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

sal_Bool SwSectionFrm::Growable() const
{
    SWRECTFN( this )
    if( (*fnRect->fnYDiff)( lcl_DeadLine( this ),
        (Frm().*fnRect->fnGetBottom)() ) > 0 )
        return sal_True;

    return ( GetUpper() && ((SwFrm*)GetUpper())->Grow( LONG_MAX, sal_True ) );
}

/*************************************************************************
|*
|*  SwSectionFrm::_Grow(), _Shrink()
|*
|*  Ersterstellung      AMA 14. Jan. 98
|*  Letzte Aenderung    AMA 14. Jan. 98
|*
|*************************************************************************/

SwTwips SwSectionFrm::_Grow( SwTwips nDist, sal_Bool bTst )
{
    if ( !IsColLocked() && !HasFixSize() )
    {
        SWRECTFN( this )
        long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
        if( nFrmHeight > 0 && nDist > (LONG_MAX - nFrmHeight) )
            nDist = LONG_MAX - nFrmHeight;

        if ( nDist <= 0L )
            return 0L;

        sal_Bool bInCalcCntnt = GetUpper() && IsInFly() && FindFlyFrm()->IsLocked();
        // OD 2004-03-15 #116561# - allow grow in online layout
        sal_Bool bGrow = !Lower() || !Lower()->IsColumnFrm() || !Lower()->GetNext() ||
             GetSection()->GetFmt()->GetBalancedColumns().GetValue();
        if( !bGrow )
        {
             const ViewShell *pSh = getRootFrm()->GetCurrShell();
             bGrow = pSh && pSh->GetViewOptions()->getBrowseMode();
        }
        if( bGrow )
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
                nGrow += GetUpper()->Grow( LONG_MAX, sal_True );

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
                         Grow( nGrow - nSpace, sal_False ) )
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
                // --> OD 2004-07-05 #i28701# - Due to the new object positioning
                // the frame on the next page/column can flow backward (e.g. it
                // was moved forward due to the positioning of its objects ).
                // Thus, invalivate this next frame, if document compatibility
                // option 'Consider wrapping style influence on object positioning' is ON.
                else if ( GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) )
                {
                    InvalidateNextPos();
                }
                // <--
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

SwTwips SwSectionFrm::_Shrink( SwTwips nDist, sal_Bool bTst )
{
    if ( Lower() && !IsColLocked() && !HasFixSize() )
    {
        if( ToMaximize( sal_False ) )
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

sal_Bool SwSectionFrm::MoveAllowed( const SwFrm* pFrm) const
{
    // Gibt es einen Follow oder ist der Frame nicht in der letzten Spalte?
    if( HasFollow() || ( pFrm->GetUpper()->IsColBodyFrm() &&
        pFrm->GetUpper()->GetUpper()->GetNext() ) )
        return sal_True;
    if( pFrm->IsInFtn() )
    {
        if( IsInFtn() )
        {
            if( GetUpper()->IsInSct() )
            {
                if( Growable() )
                    return sal_False;
                return GetUpper()->FindSctFrm()->MoveAllowed( this );
            }
            else
                return sal_True;
        }
        // The content of footnote inside a columned sectionfrm is moveable
        // except in the last column
        const SwLayoutFrm *pLay = pFrm->FindFtnFrm()->GetUpper()->GetUpper();
        if( pLay->IsColumnFrm() && pLay->GetNext() )
        {
            // The first paragraph in the first footnote in the first column
            // in the sectionfrm at the top of the page is not moveable,
            // if the columnbody is empty.
            sal_Bool bRet = sal_False;
            if( pLay->GetIndPrev() || pFrm->GetIndPrev() ||
                pFrm->FindFtnFrm()->GetPrev() )
                bRet = sal_True;
            else
            {
                SwLayoutFrm* pBody = ((SwColumnFrm*)pLay)->FindBodyCont();
                if( pBody && pBody->Lower() )
                    bRet = sal_True;
            }
            if( bRet && ( IsFtnAtEnd() || !Growable() ) )
                return sal_True;
        }
    }
    // Oder kann der Bereich noch wachsen?
    if( !IsColLocked() && Growable() )
        return sal_False;
    // Jetzt muss untersucht werden, ob es ein Layoutblatt gibt, in dem
    // ein Bereichsfollow erzeugt werden kann.
    if( IsInTab() || ( !IsInDocBody() && FindFooterOrHeader() ) )
        return sal_False; // In Tabellen/Kopf/Fusszeilen geht es nicht
    if( IsInFly() ) // Bei spaltigen oder verketteten Rahmen
        return 0 != ((SwFrm*)GetUpper())->GetNextLeaf( MAKEPAGE_NONE );
    return sal_True;
}

/** Called for a frame inside a section with no direct previous frame (or only
    previous empty section frames) the previous frame of the outer section is
    returned, if the frame is the first flowing content of this section.

    Note: For a frame inside a table frame, which is inside a section frame,
          NULL is returned.
*/
SwFrm* SwFrm::_GetIndPrev() const
{
    SwFrm *pRet = NULL;
    // --> OD 2007-09-04 #i79774#, #b659654#
    // Do not assert, if the frame has a direct previous frame, because it
    // could be an empty section frame. The caller has to assure, that the
    // frame has no direct previous frame or only empty section frames as
    // previous frames.
    ASSERT( /*!pPrev &&*/ IsInSct(), "Why?" );
    // <--
    const SwFrm* pSct = GetUpper();
    if( !pSct )
        return NULL;
    if( pSct->IsSctFrm() )
        pRet = pSct->GetIndPrev();
    else if( pSct->IsColBodyFrm() && (pSct = pSct->GetUpper()->GetUpper())->IsSctFrm() )
    {
        // Do not return the previous frame of the outer section, if in one
        // of the previous columns is content.
        const SwFrm* pCol = GetUpper()->GetUpper()->GetPrev();
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

    // skip empty section frames
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

sal_Bool SwSectionFrm::IsDescendantFrom( const SwSectionFmt* pFmt ) const
{
    if( !pSection || !pFmt )
        return sal_False;
    const SwSectionFmt *pMyFmt = pSection->GetFmt();
    while( pFmt != pMyFmt )
    {
        if( pMyFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
            pMyFmt = (SwSectionFmt*)pMyFmt->GetRegisteredIn();
        else
            return sal_False;
    }
    return sal_True;
}

void SwSectionFrm::CalcFtnAtEndFlag()
{
    SwSectionFmt *pFmt = GetSection()->GetFmt();
    sal_uInt16 nVal = pFmt->GetFtnAtTxtEnd( sal_False ).GetValue();
    bFtnAtEnd = FTNEND_ATPGORDOCEND != nVal;
    bOwnFtnNum = FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
                 FTNEND_ATTXTEND_OWNNUMANDFMT == nVal;
    while( !bFtnAtEnd && !bOwnFtnNum )
    {
        if( pFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
            pFmt = (SwSectionFmt*)pFmt->GetRegisteredIn();
        else
            break;
        nVal = pFmt->GetFtnAtTxtEnd( sal_False ).GetValue();
        if( FTNEND_ATPGORDOCEND != nVal )
        {
            bFtnAtEnd = sal_True;
            bOwnFtnNum = bOwnFtnNum ||FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
                         FTNEND_ATTXTEND_OWNNUMANDFMT == nVal;
        }
    }
}

sal_Bool SwSectionFrm::IsEndnoteAtMyEnd() const
{
    return pSection->GetFmt()->GetEndAtTxtEnd( sal_False ).IsAtEnd();
}

void SwSectionFrm::CalcEndAtEndFlag()
{
    SwSectionFmt *pFmt = GetSection()->GetFmt();
    bEndnAtEnd = pFmt->GetEndAtTxtEnd( sal_False ).IsAtEnd();
    while( !bEndnAtEnd )
    {
        if( pFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
            pFmt = (SwSectionFmt*)pFmt->GetRegisteredIn();
        else
            break;
        bEndnAtEnd = pFmt->GetEndAtTxtEnd( sal_False ).IsAtEnd();
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

void SwSectionFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    sal_uInt8 nInvFlags = 0;

    if( pNew && RES_ATTRSET_CHG == pNew->Which() )
    {
        SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
        SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
        SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );
        while( sal_True )
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

void SwSectionFrm::SwClientNotify( const SwModify& rMod, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING && &rMod == GetRegisteredIn() )
    {
        SwSectionFrm::MoveCntntAndDelete( this, sal_True );
    }
}

void SwSectionFrm::_UpdateAttr( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                            sal_uInt8 &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    sal_Bool bClear = sal_True;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {   // Mehrspaltigkeit in Fussnoten unterdruecken...
        case RES_FMT_CHG:
        {
            const SwFmtCol& rNewCol = GetFmt()->GetCol();
            if( !IsInFtn() )
            {
                //Dummer Fall. Bei der Zuweisung einer Vorlage k?nnen wir uns
                //nicht auf das alte Spaltenattribut verlassen. Da diese
                //wenigstens anzahlgemass fuer ChgColumns vorliegen muessen,
                //bleibt uns nur einen temporaeres Attribut zu basteln.
                SwFmtCol aCol;
                if ( Lower() && Lower()->IsColumnFrm() )
                {
                    sal_uInt16 nCol = 0;
                    SwFrm *pTmp = Lower();
                    do
                    {   ++nCol;
                        pTmp = pTmp->GetNext();
                    } while ( pTmp );
                    aCol.Init( nCol, 0, 1000 );
                }
                sal_Bool bChgFtn = IsFtnAtEnd();
                sal_Bool bChgEndn = IsEndnAtEnd();
                sal_Bool bChgMyEndn = IsEndnoteAtMyEnd();
                CalcFtnAtEndFlag();
                CalcEndAtEndFlag();
                bChgFtn = ( bChgFtn != IsFtnAtEnd() ) ||
                          ( bChgEndn != IsEndnAtEnd() ) ||
                          ( bChgMyEndn != IsEndnoteAtMyEnd() );
                ChgColumns( aCol, rNewCol, bChgFtn );
                rInvFlags |= 0x10;
            }
            rInvFlags |= 0x01;
            bClear = sal_False;
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
                sal_Bool bOld = IsFtnAtEnd();
                CalcFtnAtEndFlag();
                if( bOld != IsFtnAtEnd() )
                {
                    const SwFmtCol& rNewCol = GetFmt()->GetCol();
                    ChgColumns( rNewCol, rNewCol, sal_True );
                    rInvFlags |= 0x01;
                }
            }
            break;

        case RES_END_AT_TXTEND:
            if( !IsInFtn() )
            {
                sal_Bool bOld = IsEndnAtEnd();
                sal_Bool bMyOld = IsEndnoteAtMyEnd();
                CalcEndAtEndFlag();
                if( bOld != IsEndnAtEnd() || bMyOld != IsEndnoteAtMyEnd())
                {
                    const SwFmtCol& rNewCol = GetFmt()->GetCol();
                    ChgColumns( rNewCol, rNewCol, sal_True );
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
                ViewShell *pSh = getRootFrm()->GetCurrShell();
                if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
                    pSh->Imp()->InvalidateAccessibleEditableState( sal_True, this );
            }
            break;

        default:
            bClear = sal_False;
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

sal_Bool SwSectionFrm::ToMaximize( sal_Bool bCheckFollow ) const
{
    if( HasFollow() )
    {
        if( !bCheckFollow ) // Don't check superfluous follows
            return sal_True;
        const SwSectionFrm* pFoll = GetFollow();
        while( pFoll && pFoll->IsSuperfluous() )
            pFoll = pFoll->GetFollow();
        if( pFoll )
            return sal_True;
    }
    if( IsFtnAtEnd() )
        return sal_False;
    const SwFtnContFrm* pCont = ContainsFtnCont();
    if( !IsEndnAtEnd() )
        return 0 != pCont;
    sal_Bool bRet = sal_False;
    while( pCont && !bRet )
    {
        if( pCont->FindFootNote() )
            bRet = sal_True;
        else
            pCont = ContainsFtnCont( pCont );
    }
    return bRet;
}

/*-----------------09.06.99 15:07-------------------
 * sal_Bool SwSectionFrm::ContainsFtnCont()
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

long SwSectionFrm::Undersize( sal_Bool bOverSize )
{
    bUndersized = sal_False;
    SWRECTFN( this )
    long nRet = InnerHeight() - (Prt().*fnRect->fnGetHeight)();
    if( nRet > 0 )
        bUndersized = sal_True;
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
    sal_uInt16 nPos;
    if( !pDestroy->Seek_Entry( pDel, &nPos ) )
        pDestroy->Insert( pDel );
}

void SwRootFrm::_DeleteEmptySct()
{
    ASSERT( pDestroy, "Keine Liste, keine Kekse" );
    while( pDestroy->Count() )
    {
        SwSectionFrm* pSect = (*pDestroy)[0];
        pDestroy->Remove( sal_uInt16(0) );
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
                    pUp->getRootFrm()->SetSuperfluous();
                else if( pUp->IsFtnFrm() && !pUp->IsColLocked() &&
                    pUp->GetUpper() )
                {
                    pUp->Cut();
                    delete pUp;
                }
            }
        }
        else {
            ASSERT( pSect->GetSection(), "DeleteEmptySct: Halbtoter SectionFrm?!" );
        }
    }
}

void SwRootFrm::_RemoveFromList( SwSectionFrm* pSct )
{
    ASSERT( pDestroy, "Where's my list?" );
    sal_uInt16 nPos;
    if( pDestroy->Seek_Entry( pSct, &nPos ) )
        pDestroy->Remove( nPos );
}

#ifdef DBG_UTIL

sal_Bool SwRootFrm::IsInDelList( SwSectionFrm* pSct ) const
{
    sal_uInt16 nPos;
    return ( pDestroy && pDestroy->Seek_Entry( pSct, &nPos ) );
}

#endif

bool SwSectionFrm::IsBalancedSection() const
{
    bool bRet = false;
    if ( GetSection() && Lower() && Lower()->IsColumnFrm() && Lower()->GetNext() )
    {
        bRet = !GetSection()->GetFmt()->GetBalancedColumns().GetValue();
    }
    return bRet;
}

