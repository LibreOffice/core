/*************************************************************************
 *
 *  $RCSfile: wsfrm.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:23 $
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

#ifndef _PSTM_HXX
#include <tools/pstm.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif

#include "pagefrm.hxx"
#include "section.hxx"      // SwSection
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "viewsh.hxx"
#include "viewimp.hxx"
#include "doc.hxx"
#include "fesh.hxx"
#include "docsh.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "frmfmt.hxx"
#include "ftninfo.hxx"
#include "dflyobj.hxx"
#include "hints.hxx"
#include "errhdl.hxx"
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif

#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef _FMTFORDR_HXX //autogen
#include <fmtfordr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif

#include "ftnfrm.hxx"
#include "tabfrm.hxx"
#include "swtable.hxx"
#include "htmltbl.hxx"
#include "flyfrms.hxx"
#include "frmsh.hxx"
#include "sectfrm.hxx"
#include "fmtclds.hxx"
#include "txtfrm.hxx"
#include "dbg_lay.hxx"

#ifdef DEBUG

static void CheckRootSize( SwFrm *pRoot )
{
    SwTwips nHeight = 0;
    const SwFrm *pPage = pRoot->GetLower();
    while ( pPage )
    {
        if ( pPage->GetPrev() && pPage->GetPrev()->GetLower() )
            nHeight += DOCUMENTBORDER/2;
        nHeight += pPage->Frm().Height();
        pPage = pPage->GetNext();
    }
    ASSERT( nHeight == pRoot->Frm().Height(), ":-) Roothoehe verschaetzt.");
}
#define CHECKROOTSIZE( pRoot ) ::CheckRootSize( pRoot );
#else
#define CHECKROOTSIZE( pRoot )
#endif


/*************************************************************************
|*
|*  SwFrm::SwFrm()
|*
|*  Ersterstellung      AK 12-Feb-1991
|*  Letzte Aenderung    MA 05. Apr. 94
|*
|*************************************************************************/


SwFrm::SwFrm( SwModify *pMod ) :
    SwClient( pMod ),
    pPrev( 0 ),
    pNext( 0 ),
    pUpper( 0 ),
    pDrawObjs( 0 )
#ifndef PRODUCT
    , nFrmId( SwFrm::nLastFrmId++ )
#endif
{
#ifndef PRODUCT
#ifdef DEBUG
    static USHORT nStopAt = USHRT_MAX;
    if ( nFrmId == nStopAt )
    {
        int bla = 5;
    }
#endif
#endif

    ASSERT( pMod, "Kein Frameformat uebergeben." );
    bValidPos = bValidPrtArea = bValidSize = bValidLineNum = bRetouche =
    bFixHeight = bFixWidth = bColLocked = FALSE;
    bCompletePaint = bInfInvalid = bVarHeight = TRUE;
}

/*************************************************************************
|*
|*  SwFrm::Modify()
|*
|*  Ersterstellung      AK 01-Mar-1991
|*  Letzte Aenderung    MA 20. Jun. 96
|*
|*************************************************************************/
void SwFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
{
    BYTE nInvFlags = 0;

    if( pNew && RES_ATTRSET_CHG == pNew->Which() )
    {
        SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
        SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
        while( TRUE )
        {
            _UpdateAttr( (SfxPoolItem*)aOIter.GetCurItem(),
                         (SfxPoolItem*)aNIter.GetCurItem(), nInvFlags );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        SwPageFrm *pPage = FindPageFrm();
        InvalidatePage( pPage );
        if ( nInvFlags & 0x01 )
        {
            _InvalidatePrt();
            if( !GetPrev() && IsTabFrm() && IsInSct() )
                FindSctFrm()->_InvalidatePrt();
        }
        if ( nInvFlags & 0x02 )
            _InvalidateSize();
        if ( nInvFlags & 0x04 )
            _InvalidatePos();
        if ( nInvFlags & 0x08 )
            SetCompletePaint();
        SwFrm *pNxt;
        if ( nInvFlags & 0x30 && 0 != (pNxt = GetNext()) )
        {
            pNxt->InvalidatePage( pPage );
            if ( nInvFlags & 0x10 )
                pNxt->_InvalidatePos();
            if ( nInvFlags & 0x20 )
                pNxt->SetCompletePaint();
        }
    }
}

void SwFrm::_UpdateAttr( SfxPoolItem *pOld, SfxPoolItem *pNew,
                         BYTE &rInvFlags )
{
    USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_BOX:
        case RES_SHADOW:
            Prepare( PREP_FIXSIZE_CHG );
            // hier kein break !
        case RES_LR_SPACE:
        case RES_UL_SPACE:
            rInvFlags |= 0x0B;
            break;

        case RES_BACKGROUND:
            rInvFlags |= 0x28;
            break;

        case RES_KEEP:
            rInvFlags |= 0x04;
            break;

        case RES_FRM_SIZE:
            ReinitializeFrmSizeAttrFlags();
            rInvFlags |= 0x13;
            break;

        case RES_FMT_CHG:
            rInvFlags |= 0x0F;
            break;

        case RES_COL:
            ASSERT( FALSE, "Spalten fuer neuen FrmTyp?" );
            break;

        default:
            /* do Nothing */;
    }
}

/*************************************************************************
|*
|*    SwFrm::Prepare()
|*    Ersterstellung    MA 13. Apr. 93
|*    Letzte Aenderung  MA 26. Jun. 96
|*
|*************************************************************************/
void SwFrm::Prepare( const PrepareHint, const void *, BOOL )
{
    /* Do nothing */
}

/*************************************************************************
|*
|*    SwFrm::InvalidatePage()
|*    Beschreibung:     Invalidiert die Seite, in der der Frm gerade steht.
|*      Je nachdem ob es ein Layout, Cntnt oder FlyFrm ist wird die Seite
|*      entsprechend Invalidiert.
|*    Ersterstellung    MA 22. Jul. 92
|*    Letzte Aenderung  MA 14. Oct. 94
|*
|*************************************************************************/
void SwFrm::InvalidatePage( const SwPageFrm *pPage ) const
{
#if defined(DEBUG) && !defined(PRODUCT)
    static USHORT nStop = 0;
    if ( nStop == GetFrmId() )
    {
        int bla = 5;
    }
#endif

    if ( !pPage )
        pPage = FindPageFrm();

    if ( pPage && pPage->GetUpper() )
    {
        if ( pPage->GetFmt()->GetDoc()->IsInDtor() )
            return;

        SwRootFrm *pRoot = (SwRootFrm*)pPage->GetUpper();
        const SwFlyFrm *pFly = FindFlyFrm();
        if ( IsCntntFrm() )
        {
            if ( pRoot->IsTurboAllowed() )
            {
                // JP 21.09.95: wenn sich der ContentFrame 2 mal eintragen
                //              will, kann es doch eine TurboAction bleiben.
                //  ODER????
                if ( !pRoot->GetTurbo() || this == pRoot->GetTurbo() )
                    pRoot->SetTurbo( (const SwCntntFrm*)this );
                else
                {
                    pRoot->DisallowTurbo();
                    //Die Seite des Turbo koennte eine andere als die meinige
                    //sein, deshalb muss sie invalidiert werden.
                    const SwFrm *pTmp = pRoot->GetTurbo();
                    pRoot->ResetTurbo();
                    pTmp->InvalidatePage();
                }
            }
            if ( !pRoot->GetTurbo() )
            {
                if ( pFly )
                {   if( !pFly->IsLocked() )
                    {
                        if ( pFly->IsFlyInCntFrm() )
                        {   pPage->InvalidateFlyInCnt();
                            ((SwFlyInCntFrm*)pFly)->InvalidateCntnt();
                            pFly->GetAnchor()->InvalidatePage();
                        }
                        else
                            pPage->InvalidateFlyCntnt();
                    }
                }
                else
                    pPage->InvalidateCntnt();
            }
        }
        else
        {
            pRoot->DisallowTurbo();
            if ( pFly )
            {   if( !pFly->IsLocked() )
                {
                    if ( pFly->IsFlyInCntFrm() )
                    {   pPage->InvalidateFlyInCnt();
                        ((SwFlyInCntFrm*)pFly)->InvalidateLayout();
                        pFly->GetAnchor()->InvalidatePage();
                    }
                    else
                        pPage->InvalidateFlyLayout();
                }
            }
            else
                pPage->InvalidateLayout();

            if ( pRoot->GetTurbo() )
            {   const SwFrm *pTmp = pRoot->GetTurbo();
                pRoot->ResetTurbo();
                pTmp->InvalidatePage();
            }
        }
        pRoot->SetIdleFlags();
    }
}

/*************************************************************************
|*
|*  SwFrm::ChgSize()
|*
|*  Ersterstellung      AK 15-Feb-1991
|*  Letzte Aenderung    MA 18. Nov. 98
|*
|*************************************************************************/
void SwFrm::ChgSize( const Size& aNewSize )
{
    bFixHeight = bVarHeight;
    bFixWidth  = !bVarHeight;

    const Size aOldSize( Frm().SSize() );
    if ( aNewSize == aOldSize )
        return;

    const SzPtr pVar = pVARSIZE;
    const SzPtr pFix = pFIXSIZE;

    aFrm.SSize().*pFix = aNewSize.*pFix;

    if ( GetUpper() )
    {
        long nDiff = aNewSize.*pVar - aFrm.SSize().*pVar;
        if( nDiff )
        {
            if ( GetUpper()->IsFtnBossFrm() && HasFixSize( pVar ) &&
                 NA_GROW_SHRINK !=
                 ((SwFtnBossFrm*)GetUpper())->NeighbourhoodAdjustment( this ) )
            {
                aFrm.SSize().*pVar = aNewSize.*pVar;
                SwTwips nReal = ((SwLayoutFrm*)this)->AdjustNeighbourhood( nDiff );
                if ( nReal != nDiff )
                    aFrm.SSize().*pVar -= nDiff - nReal;
            }
            else
            {
                if ( nDiff > 0 )
                    Grow( nDiff, pVar );
                else
                    Shrink( -nDiff, pVar );
                // Auch wenn das Grow/Shrink noch nicht die gewuenschte Breite eingestellt hat,
                // wie z.B. beim Aufruf durch ChgColumns, um die Spaltenbreiten einzustellen,
                // wird die Breite jetzt gesetzt.
                aFrm.SSize().*pVar = aNewSize.*pVar;
            }
        }
    }
    else
        aFrm.SSize().*pVar = aNewSize.*pVar;

    if ( Frm().SSize() != aOldSize )
    {
        SwPageFrm *pPage = FindPageFrm();
        if ( GetNext() )
        {
            GetNext()->_InvalidatePos();
            GetNext()->InvalidatePage( pPage );
        }
        if ( IsLayoutFrm() && ((SwLayoutFrm*)this)->Lower() )
            ((SwLayoutFrm*)this)->Lower()->_InvalidateSize();
        _InvalidatePrt();
        _InvalidateSize();
        InvalidatePage( pPage );
        if ( GetUpper() )
            GetUpper()->_InvalidateSize();
    }
}

/*************************************************************************
|*
|*  SwFrm::InsertBefore()
|*
|*  Beschreibung        SwFrm wird in eine bestehende Struktur eingefuegt
|*                      Eingefuegt wird unterhalb des Parent und entweder
|*                      vor pBehind oder am Ende der Kette wenn pBehind
|*                      leer ist.
|*  Letzte Aenderung    MA 06. Aug. 99
|*
|*************************************************************************/
void SwFrm::InsertBefore( SwLayoutFrm* pParent, SwFrm* pBehind )
{
    ASSERT( pParent, "Kein Parent fuer Insert." );
    ASSERT( (!pBehind || (pBehind && pParent == pBehind->GetUpper())),
            "Framebaum inkonsistent." );

    pUpper = pParent;
    pNext = pBehind;
    if( pBehind )
    {   //Einfuegen vor pBehind.
        if( 0 != (pPrev = pBehind->pPrev) )
            pPrev->pNext = this;
        else
            pUpper->pLower = this;
        pBehind->pPrev = this;
    }
    else
    {   //Einfuegen am Ende, oder als ersten Node im Unterbaum
        pPrev = pUpper->Lower();
        if ( pPrev )
        {
            while( pPrev->pNext )
                pPrev = pPrev->pNext;
            pPrev->pNext = this;
        }
        else
            pUpper->pLower = this;
    }
}

/*************************************************************************
|*
|*  SwFrm::InsertBehind()
|*
|*  Beschreibung        SwFrm wird in eine bestehende Struktur eingefuegt
|*                      Eingefuegt wird unterhalb des Parent und entweder
|*                      hinter pBefore oder am Anfang der Kette wenn pBefore
|*                      leer ist.
|*  Letzte Aenderung    MA 06. Aug. 99
|*
|*************************************************************************/
void SwFrm::InsertBehind( SwLayoutFrm *pParent, SwFrm *pBefore )
{
    ASSERT( pParent, "Kein Parent fuer Insert." );
    ASSERT( (!pBefore || (pBefore && pParent == pBefore->GetUpper())),
            "Framebaum inkonsistent." );

    pUpper = pParent;
    pPrev = pBefore;
    if ( pBefore )
    {
        //Einfuegen hinter pBefore
        if ( 0 != (pNext = pBefore->pNext) )
            pNext->pPrev = this;
        pBefore->pNext = this;
    }
    else
    {
        //Einfuegen am Anfang der Kette
        pNext = pParent->Lower();
        if ( pParent->Lower() )
            pParent->Lower()->pPrev = this;
        pParent->pLower = this;
    }
}

/*************************************************************************
|*
|*  SwFrm::InsertGroup()
|*
|*  Beschreibung        Eine Kette von SwFrms wird in eine bestehende Struktur
|*                      eingefuegt
|*  Letzte Aenderung    AMA 9. Dec. 97
|*
|*  Bisher wird dies genutzt, um einen SectionFrame, der ggf. schon Geschwister
|*  mit sich bringt, in eine bestehende Struktur einzufuegen.
|*
|*  Wenn man den dritten Parameter als NULL uebergibt, entspricht
|*  diese Methode dem SwFrm::InsertBefore(..), nur eben mit Geschwistern.
|*
|*  Wenn man einen dritten Parameter uebergibt, passiert folgendes:
|*  this wird pNext von pParent,
|*  pSct wird pNext vom Letzten der this-Kette,
|*  pBehind wird vom pParent an den pSct umgehaengt.
|*  Dies dient dazu: ein SectionFrm (this) wird nicht als
|*  Kind an einen anderen SectionFrm (pParent) gehaengt, sondern pParent
|*  wird in zwei Geschwister aufgespalten (pParent+pSct) und this dazwischen
|*  eingebaut.
|*
|*************************************************************************/
void SwFrm::InsertGroupBefore( SwFrm* pParent, SwFrm* pBehind, SwFrm* pSct )
{
    ASSERT( pParent, "Kein Parent fuer Insert." );
    ASSERT( (!pBehind || (pBehind && ( pParent == pBehind->GetUpper())
            || ( pParent->IsSctFrm() && pBehind->GetUpper()->IsColBodyFrm() ) ) ),
            "Framebaum inkonsistent." );
    if( pSct )
    {
        pUpper = pParent->GetUpper();
        SwFrm *pLast = this;
        while( pLast->GetNext() )
        {
            pLast = pLast->GetNext();
            pLast->pUpper = GetUpper();
        }
        if( pBehind )
        {
            pLast->pNext = pSct;
            pSct->pPrev = pLast;
            pSct->pNext = pParent->GetNext();
        }
        else
        {
            pLast->pNext = pParent->GetNext();
            if( pLast->GetNext() )
                pLast->GetNext()->pPrev = pLast;
        }
        pParent->pNext = this;
        pPrev = pParent;
        if( pSct->GetNext() )
            pSct->GetNext()->pPrev = pSct;
        while( pLast->GetNext() )
        {
            pLast = pLast->GetNext();
            pLast->pUpper = GetUpper();
        }
        if( pBehind )
        {   //Einfuegen vor pBehind.
            if( pBehind->GetPrev() )
                pBehind->GetPrev()->pNext = NULL;
            else
                pBehind->GetUpper()->pLower = NULL;
            pBehind->pPrev = NULL;
            SwLayoutFrm* pTmp = (SwLayoutFrm*)pSct;
            if( pTmp->Lower() )
            {
                ASSERT( pTmp->Lower()->IsColumnFrm(), "InsertGrp: Used SectionFrm" );
                pTmp = (SwLayoutFrm*)((SwLayoutFrm*)pTmp->Lower())->Lower();
                ASSERT( pTmp, "InsertGrp: Missing ColBody" );
            }
            pBehind->pUpper = pTmp;
            pBehind->GetUpper()->pLower = pBehind;
            pLast = pBehind->GetNext();
            while ( pLast )
            {
                pLast->pUpper = pBehind->GetUpper();
                pLast = pLast->GetNext();
            };
        }
        else
        {
            ASSERT( pSct->IsSctFrm(), "InsertGroup: For SectionFrms only" );
            delete ((SwSectionFrm*)pSct);
        }
    }
    else
    {
        pUpper = (SwLayoutFrm*)pParent;
        SwFrm *pLast = this;
        while( pLast->GetNext() )
        {
            pLast = pLast->GetNext();
            pLast->pUpper = GetUpper();
        }
        pLast->pNext = pBehind;
        if( pBehind )
        {   //Einfuegen vor pBehind.
            if( 0 != (pPrev = pBehind->pPrev) )
                pPrev->pNext = this;
            else
                pUpper->pLower = this;
            pBehind->pPrev = pLast;
        }
        else
        {   //Einfuegen am Ende, oder des ersten Nodes im Unterbaum
            pPrev = pUpper->Lower();
            if ( pPrev )
            {
                while( pPrev->pNext )
                    pPrev = pPrev->pNext;
                pPrev->pNext = this;
            }
            else
                pUpper->pLower = this;
        }
    }
}

/*************************************************************************
|*
|*  SwFrm::Remove()
|*
|*  Ersterstellung      AK 01-Mar-1991
|*  Letzte Aenderung    MA 07. Dec. 95
|*
|*************************************************************************/
void SwFrm::Remove()
{
    ASSERT( pUpper, "Removen ohne Upper?" );

    if( pPrev )
        // einer aus der Mitte wird removed
        pPrev->pNext = pNext;
    else
    {   // der erste in einer Folge wird removed
        ASSERT( pUpper->pLower == this, "Layout inkonsistent." );
        pUpper->pLower = pNext;
    }
    if( pNext )
        pNext->pPrev = pPrev;

    // Verbindung kappen.
    pNext  = pPrev  = 0;
    pUpper = 0;
}
/*************************************************************************
|*
|*  SwCntntFrm::Paste()
|*
|*  Ersterstellung      MA 23. Feb. 94
|*  Letzte Aenderung    MA 09. Sep. 98
|*
|*************************************************************************/
void SwCntntFrm::Paste( SwFrm* pParent, SwFrm* pSibling)
{
    ASSERT( pParent, "Kein Parent fuer Paste." );
    ASSERT( pParent->IsLayoutFrm(), "Parent ist CntntFrm." );
    ASSERT( pParent != this, "Bin selbst der Parent." );
    ASSERT( pSibling != this, "Bin mein eigener Nachbar." );
    ASSERT( !GetPrev() && !GetNext() && !GetUpper(),
            "Bin noch irgendwo angemeldet." );

    //In den Baum einhaengen.
    InsertBefore( (SwLayoutFrm*)pParent, pSibling );

    SwPageFrm *pPage = FindPageFrm();
    _InvalidateAll();
    InvalidatePage( pPage );

    if( pPage )
    {
        pPage->InvalidateSpelling();
        pPage->InvalidateAutoCompleteWords();
    }

    if ( GetNext() )
    {
        SwFrm* pNxt = GetNext();
        pNxt->_InvalidatePrt();
        pNxt->_InvalidatePos();
        pNxt->InvalidatePage( pPage );
        if( pNxt->IsSctFrm() )
            pNxt = ((SwSectionFrm*)pNxt)->ContainsCntnt();
        if( pNxt && pNxt->IsTxtFrm() && pNxt->IsInFtn() )
            pNxt->Prepare( PREP_FTN, 0, FALSE );
    }

    if ( Frm().Height() )
        pParent->Grow( Frm().Height(), pHeight );

    if ( Frm().Width() != pParent->Prt().Width() )
        Prepare( PREP_FIXSIZE_CHG );

    if ( GetPrev() )
    {
        if ( IsFollow() )
            //Ich bin jetzt direkter Nachfolger meines Masters geworden
            ((SwCntntFrm*)GetPrev())->Prepare( PREP_FOLLOW_FOLLOWS );
        else
        {
            if ( GetPrev()->Frm().Height() !=
                 GetPrev()->Prt().Height() + GetPrev()->Prt().Top() )
                //Umrandung zu beruecksichtigen?
                GetPrev()->_InvalidatePrt();
            GetPrev()->InvalidatePage( pPage );
        }
    }
    if ( IsInFtn() )
    {
        SwFrm* pFrm = GetIndPrev();
        if( pFrm && pFrm->IsSctFrm() )
            pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
        if( pFrm )
            pFrm->Prepare( PREP_QUOVADIS, 0, FALSE );
        if( !GetNext() )
        {
            pFrm = FindFtnFrm()->GetNext();
            if( pFrm && 0 != (pFrm=((SwLayoutFrm*)pFrm)->ContainsAny()) )
                pFrm->_InvalidatePrt();
        }
    }

    _InvalidateLineNum();
    SwFrm *pNxt = FindNextCnt();
    if ( pNxt  )
    {
        while ( pNxt && pNxt->IsInTab() )
        {
            if( 0 != (pNxt = pNxt->FindTabFrm()) )
                pNxt = pNxt->FindNextCnt();
        }
        if ( pNxt )
        {
            pNxt->_InvalidateLineNum();
            if ( pNxt != GetNext() )
                pNxt->InvalidatePage();
        }
    }
}

/*************************************************************************
|*
|*  SwCntntFrm::Cut()
|*
|*  Ersterstellung      AK 14-Feb-1991
|*  Letzte Aenderung    MA 09. Sep. 98
|*
|*************************************************************************/
void SwCntntFrm::Cut()
{
    ASSERT( GetUpper(), "Cut ohne Upper()." );

    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    SwFrm *pFrm = GetIndPrev();
    if( pFrm )
    {
        if( pFrm->IsSctFrm() )
            pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
        if ( pFrm && pFrm->IsCntntFrm() )
        {
            pFrm->_InvalidatePrt();
            if( IsInFtn() )
                pFrm->Prepare( PREP_QUOVADIS, 0, FALSE );
        }
    }

    SwFrm *pNxt = FindNextCnt();
    if ( pNxt )
    {
        while ( pNxt && pNxt->IsInTab() )
        {
            if( 0 != (pNxt = pNxt->FindTabFrm()) )
                pNxt = pNxt->FindNextCnt();
        }
        if ( pNxt )
        {
            pNxt->_InvalidateLineNum();
            if ( pNxt != GetNext() )
                pNxt->InvalidatePage();
        }
    }

    if( 0 != (pFrm = GetIndNext()) )
    {   //Der alte Nachfolger hat evtl. einen Abstand zum Vorgaenger
        //berechnet, der ist jetzt, wo er der erste wird obsolet bzw. anders.
        pFrm->_InvalidatePrt();
        pFrm->_InvalidatePos();
        pFrm->InvalidatePage( pPage );
        if( pFrm->IsSctFrm() )
        {
            pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
            if( pFrm )
            {
                pFrm->_InvalidatePrt();
                pFrm->_InvalidatePos();
                pFrm->InvalidatePage( pPage );
            }
        }
        if( pFrm && IsInFtn() )
            pFrm->Prepare( PREP_ERGOSUM, 0, FALSE );
        if( IsInSct() && !GetPrev() )
        {
            SwSectionFrm* pSct = FindSctFrm();
            if( !pSct->IsFollow() )
            {
                pSct->_InvalidatePrt();
                pSct->InvalidatePage( pPage );
            }
        }
    }
    else
    {
        InvalidateNextPos();
        //Einer muss die Retusche uebernehmen: Vorgaenger oder Upper
        if ( 0 != (pFrm = GetPrev()) )
        {   pFrm->SetRetouche();
            pFrm->Prepare( PREP_WIDOWS_ORPHANS );
            pFrm->_InvalidatePos();
            pFrm->InvalidatePage( pPage );
        }
        //Wenn ich der einzige CntntFrm in meinem Upper bin (war), so muss
        //er die Retouche uebernehmen.
        //Ausserdem kann eine Leerseite entstanden sein.
        else
        {   SwRootFrm *pRoot = FindRootFrm();
            if ( pRoot )
            {
                pRoot->SetSuperfluous();
                GetUpper()->SetCompletePaint();
                GetUpper()->InvalidatePage( pPage );
            }
            if( IsInSct() )
            {
                SwSectionFrm* pSct = FindSctFrm();
                if( !pSct->IsFollow() )
                {
                    pSct->_InvalidatePrt();
                    pSct->InvalidatePage( pPage );
                }
            }
        }
    }
    //Erst removen, dann Upper Shrinken.
    SwLayoutFrm *pUp = GetUpper();
    Remove();
    if ( pUp )
    {
        SwSectionFrm *pSct;
        if ( !pUp->Lower() && ( ( pUp->IsFtnFrm() && !pUp->IsColLocked() )
            || ( pUp->IsInSct() && !(pSct = pUp->FindSctFrm())->ContainsCntnt() ) ) )
        {
            if ( pUp->GetUpper() )
            {
                if( pUp->IsFtnFrm() )
                {
                    if( pUp->GetNext() && !pUp->GetPrev() )
                    {
                        SwFrm* pTmp = ((SwLayoutFrm*)pUp->GetNext())->ContainsAny();
                        if( pTmp )
                            pTmp->_InvalidatePrt();
                    }
                    pUp->Cut();
                    delete pUp;
                }
                else
                {
                    if( pSct->IsColLocked() || !pSct->IsInFtn() )
                    {
                        pSct->DelEmpty( FALSE );
                    // Wenn ein gelockter Bereich nicht geloescht werden darf,
                    // so ist zumindest seine Groesse durch das Entfernen seines
                    // letzten Contents ungueltig geworden.
                        pSct->_InvalidateSize();
                    }
                    else
                    {
                        pSct->DelEmpty( TRUE );
                        delete pSct;
                    }
                }
            }
        }
        else if ( Frm().Height() )
            pUp->Shrink( Frm().Height(), pHeight );
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::Paste()
|*
|*  Ersterstellung      MA 23. Feb. 94
|*  Letzte Aenderung    MA 23. Feb. 94
|*
|*************************************************************************/
void SwLayoutFrm::Paste( SwFrm* pParent, SwFrm* pSibling)
{
    ASSERT( pParent, "Kein Parent fuer Paste." );
    ASSERT( pParent->IsLayoutFrm(), "Parent ist CntntFrm." );
    ASSERT( pParent != this, "Bin selbst der Parent." );
    ASSERT( pSibling != this, "Bin mein eigener Nachbar." );
    ASSERT( !GetPrev() && !GetNext() && !GetUpper(),
            "Bin noch irgendwo angemeldet." );

    //In den Baum einhaengen.
    InsertBefore( (SwLayoutFrm*)pParent, pSibling );

    //ggf. die Memberpointer korrigieren.
    const SwFmtFillOrder &rFill =
                        ((SwLayoutFrm*)pParent)->GetFmt()->GetFillOrder();
    if ( rFill.GetFillOrder() == ATT_BOTTOM_UP ||
         rFill.GetFillOrder() == ATT_TOP_DOWN )
        bVarHeight = TRUE;
    else
        bVarHeight = FALSE;
    const SzPtr pFix = pFIXSIZE;
    const SzPtr pVar = pVARSIZE;
    if( Frm().SSize().*pFix != pParent->Prt().SSize().*pFix )
        _InvalidateSize();
    _InvalidatePos();
    const SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    SwFrm *pFrm;
    if( !IsColumnFrm() )
    {
        if( 0 != ( pFrm = GetIndNext() ) )
        {
            pFrm->_InvalidatePos();
            if( IsInFtn() )
            {
                if( pFrm->IsSctFrm() )
                    pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
                if( pFrm )
                    pFrm->Prepare( PREP_ERGOSUM, 0, FALSE );
            }
        }
        if ( IsInFtn() && 0 != ( pFrm = GetIndPrev() ) )
        {
            if( pFrm->IsSctFrm() )
                pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
            if( pFrm )
                pFrm->Prepare( PREP_QUOVADIS, 0, FALSE );
        }
    }

    if ( Frm().SSize().*pVar )
    {
        // AdjustNeighbourhood wird jetzt auch in Spalten aufgerufen,
        // die sich nicht in Rahmen befinden
        BYTE nAdjust = GetUpper()->IsFtnBossFrm() ?
                ((SwFtnBossFrm*)GetUpper())->NeighbourhoodAdjustment( this )
                : NA_GROW_SHRINK;
        SwTwips nGrow = Frm().SSize().*pVar;
        if( NA_ONLY_ADJUST == nAdjust )
            AdjustNeighbourhood( nGrow );
        else
        {
            SwTwips nReal = 0;
            if( NA_ADJUST_GROW == nAdjust )
                nReal = AdjustNeighbourhood( nGrow );
            if( nReal < nGrow )
                nReal += pParent->Grow( nGrow - nReal, pVar );
            if( NA_GROW_ADJUST == nAdjust && nReal < nGrow )
                AdjustNeighbourhood( nGrow - nReal );
        }
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::Cut()
|*
|*  Ersterstellung      MA 23. Feb. 94
|*  Letzte Aenderung    MA 23. Feb. 94
|*
|*************************************************************************/
void SwLayoutFrm::Cut()
{
    if ( GetNext() )
        GetNext()->_InvalidatePos();

    const SzPtr pVar = pVARSIZE;
    SwTwips nShrink = Frm().SSize().*pVar;

    //Erst removen, dann Upper Shrinken.
    SwLayoutFrm *pUp = GetUpper();

    // AdjustNeighbourhood wird jetzt auch in Spalten aufgerufen,
    // die sich nicht in Rahmen befinden

    // Remove must not be called before a AdjustNeighbourhood, but it has to
    // be called before the upper-shrink-call, if the upper-shrink takes care
    // of his content
    if ( pUp && nShrink )
    {
        if( pUp->IsFtnBossFrm() )
        {
            BYTE nAdjust= ((SwFtnBossFrm*)pUp)->NeighbourhoodAdjustment( this );
            if( NA_ONLY_ADJUST == nAdjust )
                AdjustNeighbourhood( -nShrink );
            else
            {
                SwTwips nReal = 0;
                if( NA_ADJUST_GROW == nAdjust )
                    nReal = -AdjustNeighbourhood( -nShrink );
                if( nReal < nShrink )
                {
                    SwTwips nOldHeight = Frm().Height();
                    Frm().Height( 0 );
                    nReal += pUp->Shrink( nShrink - nReal, pVar );
                    Frm().Height( nOldHeight );
                }
                if( NA_GROW_ADJUST == nAdjust && nReal < nShrink )
                    AdjustNeighbourhood( nReal - nShrink );
            }
            Remove();
        }
        else
        {
            Remove();
            pUp->Shrink( nShrink, pVar );
        }
    }
    else
        Remove();

    if( pUp && !pUp->Lower() )
    {
        pUp->SetCompletePaint();
        pUp->InvalidatePage();
    }
}

/*************************************************************************
|*
|*  SwFrm::Grow()
|*
|*  Ersterstellung      AK 19-Feb-1991
|*  Letzte Aenderung    MA 05. May. 94
|*
|*************************************************************************/
SwTwips SwFrm::Grow( SwTwips nDist, const SzPtr pDirection,
                     BOOL bTst, BOOL bInfo )
{
    ASSERT( nDist >= 0, "Negatives Wachstum?" );

    PROTOCOL_ENTER( this, bTst ? PROT_GROW_TST : PROT_GROW, 0, &nDist )

    if ( nDist )
    {
        if ( Prt().SSize().*pDirection > 0 &&
             nDist > (LONG_MAX - Prt().SSize().*pDirection) )
            nDist = LONG_MAX - Prt().SSize().*pDirection;

        if ( IsFlyFrm() )
            return ((SwFlyFrm*)this)->_Grow( nDist, pDirection, bTst );
        else if( IsSctFrm() )
            return ((SwSectionFrm*)this)->_Grow( nDist, pDirection, bTst );
        else
        {
            const SwTwips nReal = GrowFrm( nDist, pDirection, bTst, bInfo );
            if( !bTst )
                Prt().SSize().*pDirection += IsCntntFrm() ? nDist : nReal;
            return nReal;
        }
    }
    return 0L;
}
/*************************************************************************
|*
|*  SwFrm::Shrink()
|*
|*  Ersterstellung      AK 14-Feb-1991
|*  Letzte Aenderung    MA 05. May. 94
|*
|*************************************************************************/
SwTwips SwFrm::Shrink( SwTwips nDist, const SzPtr pDirection,
                       BOOL bTst, BOOL bInfo )
{
    ASSERT( nDist >= 0, "Negative Verkleinerung?" );

    PROTOCOL_ENTER( this, bTst ? PROT_SHRINK_TST : PROT_SHRINK, 0, &nDist )

    if ( nDist )
    {
        if ( IsFlyFrm() )
            return ((SwFlyFrm*)this)->_Shrink( nDist, pDirection, bTst );
        else if( IsSctFrm() )
            return ((SwSectionFrm*)this)->_Shrink( nDist, pDirection, bTst );
        else
        {
            SwTwips nReal = Frm().SSize().*pDirection;
            ShrinkFrm( nDist, pDirection, bTst, bInfo );
            nReal -= Frm().SSize().*pDirection;
            if( !bTst )
                Prt().SSize().*pDirection -= IsCntntFrm() ? nDist : nReal;
            return nReal;
        }
    }
    return 0L;
}
/*************************************************************************
|*
|*  SwFrm::AdjustNeighbourhood()
|*
|*  Beschreibung        Wenn sich die Groesse eines Frm's direkt unterhalb
|*      eines Fussnotenbosses (Seite/Spalte) veraendert hat, so muss dieser
|*      "Normalisiert" werden.
|*      Es gibt dort immer einen Frame, der den "maximal moeglichen" Raum
|*      einnimmt (der Frame, der den Body.Text enhaelt) und keinen oder
|*      mehrere Frames die den Platz einnehmen den sie halt brauchen
|*      (Kopf-/Fussbereich, Fussnoten).
|*      Hat sich einer der Frames veraendert, so muss der Body-Text-Frame
|*      entsprechen wachsen oder schrumpfen; unabhaegig davon, dass er fix ist.
|*      !! Ist es moeglich dies allgemeiner zu loesen, also nicht auf die
|*      Seite beschraenkt und nicht auf einen Speziellen Frame, der den
|*      maximalen Platz einnimmt (gesteuert ueber Attribut FrmSize)? Probleme:
|*      Was ist wenn mehrere Frames nebeneinander stehen, die den maximalen
|*      Platz einnehmen?
|*      Wie wird der Maximale Platz berechnet?
|*      Wie klein duerfen diese Frames werden?
|*
|*      Es wird auf jeden Fall nur so viel Platz genehmigt, dass ein
|*      Minimalwert fuer die Hoehe des Bodys nicht unterschritten wird.
|*
|*  Parameter: nDiff ist der Betrag, um den Platz geschaffen werden muss
|*
|*  Ersterstellung      MA 07. May. 92
|*  Letzte Aenderung    AMA 02. Nov. 98
|*
|*************************************************************************/
SwTwips SwFrm::AdjustNeighbourhood( SwTwips nDiff, BOOL bTst )
{
    PROTOCOL_ENTER( this, PROT_ADJUSTN, 0, &nDiff );

    if ( !nDiff || !GetUpper()->IsFtnBossFrm() ) // nur innerhalb von Seiten/Spalten
        return 0L;

    FASTBOOL bBrowse = GetUpper()->GetFmt()->GetDoc()->IsBrowseMode();

    //Der (Page)Body veraendert sich nur im BrowseMode, aber nicht wenn er
    //Spalten enthaelt.
    if ( IsPageBodyFrm() && (!bBrowse ||
          (((SwLayoutFrm*)this)->Lower() &&
           ((SwLayoutFrm*)this)->Lower()->IsColumnFrm())) )
        return 0L;

    //In der BrowseView kann der PageFrm selbst ersteinmal einiges von den
    //Wuenschen abfangen.
    long nBrowseAdd = 0;
    if ( bBrowse && GetUpper()->IsPageFrm() ) // nur (Page)BodyFrms
    {
        ViewShell *pSh = GetShell();
        SwLayoutFrm *pUp = GetUpper();
        long nChg;
        const long nUpPrtBottom = pUp->Frm().Height() -
                                  pUp->Prt().Height() - pUp->Prt().Top();
        SwRect aInva( pUp->Frm() );
        if ( pSh )
        {
            aInva.Pos().X() = pSh->VisArea().Left();
            aInva.Width( pSh->VisArea().Width() );
        }
        if ( nDiff > 0 )
        {
            nChg = BROWSE_HEIGHT - pUp->Frm().Height();
            nChg = Min( nDiff, nChg );

            if ( !IsBodyFrm() )
            {
                SetCompletePaint();
                if ( !pSh || pSh->VisArea().Height() >= pUp->Frm().Height() )
                {
                    //Ersteinmal den Body verkleinern. Der waechst dann schon
                    //wieder.
                    SwFrm *pBody = ((SwFtnBossFrm*)pUp)->FindBodyCont();
                    const long nTmp = nChg - pBody->Prt().Height();
                    if ( !bTst )
                    {
                        pBody->Frm().Height(Max( 0L, pBody->Frm().Height() - nChg ));
                        pBody->_InvalidatePrt();
                        pBody->_InvalidateSize();
                        if ( pBody->GetNext() )
                            pBody->GetNext()->_InvalidatePos();
                        if ( !IsHeaderFrm() )
                            pBody->SetCompletePaint();
                    }
                    nChg = nTmp <= 0 ? 0 : nTmp;
                }
            }

            const long nTmp = nUpPrtBottom + 20;
            aInva.Top( aInva.Bottom() - nTmp );
            aInva.Height( nChg + nTmp );
        }
        else
        {
            //Die Seite kann bis auf 0 schrumpfen. Die erste Seite bleibt
            //mindestens so gross wie die VisArea.
            nChg = nDiff;
            long nInvaAdd = 0;
            if ( pSh && !pUp->GetPrev() &&
                 pUp->Frm().Height() + nDiff < pSh->VisArea().Height() )
            {
                //Das heisst aber wiederum trotzdem, das wir geeignet invalidieren
                //muessen.
                nChg = pSh->VisArea().Height() - pUp->Frm().Height();
                nInvaAdd = -(nDiff - nChg);
            }

            //Invalidieren inklusive unterem Rand.
            long nBorder = nUpPrtBottom + 20;
            nBorder -= nChg;
            aInva.Top( aInva.Bottom() - (nBorder+nInvaAdd) );
            if ( !IsBodyFrm() )
            {
                SetCompletePaint();
                if ( !IsHeaderFrm() )
                    ((SwFtnBossFrm*)pUp)->FindBodyCont()->SetCompletePaint();
            }
            //Wegen der Rahmen die Seite invalidieren. Dadurch wird die Seite
            //wieder entsprechend gross wenn ein Rahmen nicht passt. Das
            //funktioniert anderfalls nur zufaellig fuer absatzgebundene Rahmen
            //(NotifyFlys).
            pUp->InvalidateSize();
        }
        if ( !bTst )
        {
            //Unabhaengig von nChg
            if ( pSh && aInva.HasArea() && pUp->GetUpper() )
                pSh->InvalidateWindows( aInva );
        }
        if ( !bTst && nChg )
        {
            const SwRect aOldRect( pUp->Frm() );
            pUp->Frm().SSize().Height() += nChg;
            pUp->Prt().SSize().Height() += nChg;
            if ( pSh )
                pSh->Imp()->SetFirstVisPageInvalid();

            if ( GetNext() )
                GetNext()->_InvalidatePos();

            //Ggf. noch ein Repaint ausloesen.
            const SvxGraphicPosition ePos = pUp->GetFmt()->GetBackground().GetGraphicPos();
            if ( ePos != GPOS_NONE && ePos != GPOS_TILED )
                pSh->InvalidateWindows( pUp->Frm() );

            if ( pUp->GetUpper() )
            {
                if ( pUp->GetNext() )
                    pUp->GetNext()->InvalidatePos();

                //Mies aber wahr: im Notify am ViewImp wird evtl. ein Calc
                //auf die Seite und deren Lower gerufen. Die Werte sollten
                //unverandert bleiben, weil der Aufrufer bereits fuer die
                //Anpassung von Frm und Prt sorgen wird.
                const long nOldFrmHeight = Frm().Height();
                const long nOldPrtHeight = Prt().Height();
                const BOOL bOldComplete = IsCompletePaint();
                if ( IsBodyFrm() )
                    Prt().SSize().Height() = nOldFrmHeight;
                ((SwPageFrm*)pUp)->AdjustRootSize( CHG_CHGPAGE, &aOldRect );
                Frm().SSize().Height() = nOldFrmHeight;
                Prt().SSize().Height() = nOldPrtHeight;
                bCompletePaint = bOldComplete;
            }
            if ( !IsBodyFrm() )
                pUp->_InvalidateSize();
            InvalidatePage( (SwPageFrm*)pUp );
        }
        nDiff -= nChg;
        if ( !nDiff )
            return nChg;
        else
            nBrowseAdd = nChg;
    }

    const SwFtnBossFrm *pBoss = (SwFtnBossFrm*)GetUpper();

    SwTwips nReal = 0,
            nAdd  = 0;
    SwFrm *pFrm = 0;
    const SzPtr pVar = pVARSIZE;

    if( IsBodyFrm() )
    {
        if( IsInSct() )
        {
            SwSectionFrm *pSect = FindSctFrm();
            if( nDiff > 0 && pSect->IsEndnAtEnd() && GetNext() &&
                GetNext()->IsFtnContFrm() )
            {
                SwFtnContFrm* pCont = (SwFtnContFrm*)GetNext();
                SwTwips nMinH = 0;
                SwFtnFrm* pFtn = (SwFtnFrm*)pCont->Lower();
                BOOL bFtn = FALSE;
                while( pFtn )
                {
                    if( !pFtn->GetAttr()->GetFtn().IsEndNote() )
                    {
                        nMinH += pFtn->Frm().Height();
                        bFtn = TRUE;
                    }
                    pFtn = (SwFtnFrm*)pFtn->GetNext();
                }
                if( bFtn )
                    nMinH += pCont->Prt().Top();
                nReal = pCont->Frm().Height() - nMinH;
                if( nReal > nDiff )
                    nReal = nDiff;
                if( nReal > 0 )
                    pFrm = GetNext();
                else
                    nReal = 0;
            }
            if( !bTst && !pSect->IsColLocked() )
                pSect->InvalidateSize();
        }
        if( !pFrm )
            return nBrowseAdd;
    }
    else
    {
        const BOOL bFtnPage = pBoss->IsPageFrm() && ((SwPageFrm*)pBoss)->IsFtnPage();
        if ( bFtnPage && !IsFtnContFrm() )
            pFrm = (SwFrm*)pBoss->FindFtnCont();
        if ( !pFrm )
            pFrm = (SwFrm*)pBoss->FindBodyCont();

        if ( !pFrm )
            return 0;

        //Wenn ich keinen finde eruebrigt sich alles weitere.
        nReal = pFrm->Frm().SSize().*pVar - nDiff < 0 ?
                                            pFrm->Frm().SSize().*pVar : nDiff;
        if( !bFtnPage )
        {
            //Minimalgrenze beachten!
            if( nReal )
            {
                const SwTwips nMax = pBoss->GetVarSpace();
                if ( nReal > nMax )
                    nReal = nMax;
            }
            if( !IsFtnContFrm() && nDiff > nReal &&
                pFrm->GetNext() && pFrm->GetNext()->IsFtnContFrm() )
            {
                //Wenn der Body nicht genuegend her gibt, kann ich noch mal
                //schauen ob es eine Fussnote gibt, falls ja kann dieser
                //entsprechend viel gemopst werden.
                const SwTwips nAddMax = pFrm->GetNext()->Frm().Height();
                nAdd = nDiff - nReal;
                if ( nAdd > nAddMax )
                    nAdd = nAddMax;
                if ( !bTst )
                {
                    pFrm->GetNext()->Frm().SSize().*pVar -= nAdd;
                    pFrm->GetNext()->InvalidatePrt();
                    if ( pFrm->GetNext()->GetNext() )
                        pFrm->GetNext()->GetNext()->_InvalidatePos();
                }
            }
        }
    }

    if ( !bTst && nReal )
    {
        pFrm->Frm().SSize().*pVar -= nReal;
        pFrm->InvalidatePrt();
        if ( pFrm->GetNext() )
            pFrm->GetNext()->_InvalidatePos();
        if( nReal < 0 && pFrm->IsInSct() )
        {
            SwLayoutFrm* pUp = pFrm->GetUpper();
            if( pUp && 0 != ( pUp = pUp->GetUpper() ) && pUp->IsSctFrm() &&
                !pUp->IsColLocked() )
                pUp->InvalidateSize();
        }
        if( ( IsHeaderFrm() || IsFooterFrm() ) && pBoss->GetDrawObjs() )
        {
            const SwDrawObjs &rObjs = *pBoss->GetDrawObjs();
            ASSERT( pBoss->IsPageFrm(), "Header/Footer out of page?" );
            SwPageFrm *pPage = (SwPageFrm*)pBoss;
            for ( USHORT i = 0; i < rObjs.Count(); ++i )
            {
                SdrObject *pObj = rObjs[i];
                if ( pObj->IsWriterFlyFrame() )
                {
                    SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
                    ASSERT( !pFly->IsFlyInCntFrm(), "FlyInCnt at Page?" );
                    const SwFmtVertOrient &rVert =
                                        pFly->GetFmt()->GetVertOrient();
                   // Wann muss invalidiert werden?
                   // Wenn ein Rahmen am SeitenTextBereich ausgerichtet ist,
                   // muss bei Aenderung des Headers ein TOP, MIDDLE oder NONE,
                   // bei Aenderung des Footers ein BOTTOM oder MIDDLE
                   // ausgerichteter Rahmen seine Position neu berechnen.
                    if( ( rVert.GetRelationOrient() == PRTAREA ||
                          rVert.GetRelationOrient() == REL_PG_PRTAREA ) &&
                        ((IsHeaderFrm() && rVert.GetVertOrient()!=VERT_BOTTOM) ||
                         (IsFooterFrm() && rVert.GetVertOrient()!=VERT_NONE &&
                          rVert.GetVertOrient() != VERT_TOP)) )
                    {
                        pFly->_InvalidatePos();
                        pFly->_Invalidate();
                    }
                }
            }
        }
    }
    return (nBrowseAdd + nReal + nAdd);
}

/*************************************************************************
|*
|*  SwFrm::ImplInvalidateSize(), ImplInvalidatePrt(), ImplInvalidatePos(),
|*         ImplInvalidateLineNum()
|*
|*  Ersterstellung      MA 15. Oct. 92
|*  Letzte Aenderung    MA 24. Mar. 94
|*
|*************************************************************************/
void SwFrm::ImplInvalidateSize()
{
    bValidSize = FALSE;
    if ( IsFlyFrm() )
        ((SwFlyFrm*)this)->_Invalidate();
    else
        InvalidatePage();
}

void SwFrm::ImplInvalidatePrt()
{
    bValidPrtArea = FALSE;
    if ( IsFlyFrm() )
        ((SwFlyFrm*)this)->_Invalidate();
    else
        InvalidatePage();
}

void SwFrm::ImplInvalidatePos()
{
    bValidPos = FALSE;
    if ( IsFlyFrm() )
        ((SwFlyFrm*)this)->_Invalidate();
    else
        InvalidatePage();
}

void SwFrm::ImplInvalidateLineNum()
{
    bValidLineNum = FALSE;
    ASSERT( IsTxtFrm(), "line numbers are implemented for text only" );
    InvalidatePage();
}

/*************************************************************************
|*
|*  SwFrm::ReinitializeFrmSizeAttrFlags
|*
|*  Ersterstellung      MA 15. Oct. 96
|*  Letzte Aenderung    MA 15. Oct. 96
|*
|*************************************************************************/
void SwFrm::ReinitializeFrmSizeAttrFlags()
{
    const SwFmtFrmSize &rFmtSize = GetAttrSet()->GetFrmSize();
    if ( ATT_VAR_SIZE == rFmtSize.GetSizeType() ||
         ATT_MIN_SIZE == rFmtSize.GetSizeType())
    {
        bFixHeight = bFixWidth = FALSE;
        if ( GetType() & (FRM_HEADER | FRM_FOOTER | FRM_ROW) )
        {
            SwFrm *pFrm = ((SwLayoutFrm*)this)->Lower();
            while ( pFrm )
            {   pFrm->_InvalidateSize();
                pFrm->_InvalidatePrt();
                pFrm = pFrm->GetNext();
            }
            SwCntntFrm *pCnt = ((SwLayoutFrm*)this)->ContainsCntnt();
            pCnt->InvalidatePage();
            do
            {   pCnt->Prepare( PREP_ADJUST_FRM );
                pCnt->_InvalidateSize();
                pCnt = pCnt->GetNextCntntFrm();
            } while ( ((SwLayoutFrm*)this)->IsAnLower( pCnt ) );
        }
    }
    else if ( rFmtSize.GetSizeType() == ATT_FIX_SIZE )
    {   if ( bVarHeight ) {
            bFixHeight = FALSE;
            ChgSize( Size( Frm().Width(), rFmtSize.GetHeight()));
        }
        else {
            bFixWidth = TRUE;
            ChgSize( Size( rFmtSize.GetWidth(), Frm().Height()));
        }
    }
}

/*************************************************************************
|*
|*  SwCntntFrm::GrowFrm()
|*
|*  Ersterstellung      MA 30. Jul. 92
|*  Letzte Aenderung    MA 25. Mar. 99
|*
|*************************************************************************/
SwTwips SwCntntFrm::GrowFrm( SwTwips nDist, const SzPtr pDirection,
                             BOOL bTst, BOOL bInfo )
{
    ASSERT( pDirection == pHeight, "VarSize eines Cntnt ist Breite !?!" );

    if ( Frm().SSize().Height() > 0 &&
         nDist > (LONG_MAX - Frm().SSize().Height()) )
        nDist = LONG_MAX - Frm().SSize().Height();

    //Abkuerzung, damit der Growbetrag fuer den Upper nicht umstaendlich
    //ermittelt werden muss. Die Abfrage korrespondiert mit derjenigen im
    //SwLayoutFrm::GrowFrm() (wo das Grow fuer alle direkten Upper von CntntFrms
    //durchlaufen sollte) und trifft genau dann zu, wenn das Grow auf den Upper
    //sowieso abgeleht wuerde.
    const FASTBOOL bBrowse = GetUpper()->GetFmt()->GetDoc()->IsBrowseMode();
    const USHORT nType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse mit Body
    if( !(GetUpper()->GetType() & nType) &&
        GetUpper()->HasFixSize( pDirection ) )
    {
        if ( !bTst )
        {
            Frm().SSize().Height() += nDist;
            if ( GetNext() )
                GetNext()->InvalidatePos();
        }
        return 0;
    }

    //Der Upper wird nur soweit wie notwendig gegrowed. In nReal wird erstmal
    //die bereits zur Verfuegung stehende Strecke bereitgestellt.
    SwTwips nReal = 0;

    //Es bleibt hier beim scheinbar unguenstigen Algorithmus des
    //Aufzaddierens der Einzelwerte. Alles andere birgt Risiken die jetzt
    //(04. Jan. 94) nicht vertretbar sind. Die Aufwendigsten und
    //Haeufigsten Faelle sind oben abgefangen und somit nicht mehr relevant.
    nReal = GetUpper()->Prt().Height();
    SwFrm *pFrm = GetUpper()->Lower();
    while ( pFrm && nReal > 0 )         //MA, #63881# ">0": Genug ist Genug
    {   nReal -= pFrm->Frm().Height();
        pFrm = pFrm->GetNext();
    }

    if ( !bTst )
    {
        //Cntnts werden immer auf den gewuenschten Wert gebracht.
        long nOld = Frm().Height();
        Frm().SSize().Height() += nDist;
        if ( nOld && IsInTab() )
        {
            SwTabFrm *pTab = FindTabFrm();
            if ( pTab->GetTable()->GetHTMLTableLayout() &&
                 !pTab->IsJoinLocked() &&
                 !pTab->GetFmt()->GetDoc()->GetDocShell()->IsReadOnly() )
            {
                pTab->InvalidatePos();
                pTab->SetResizeHTMLTable();
            }
        }
    }

    //Upper nur growen wenn notwendig.
    if ( nReal < nDist )
        nReal = GetUpper() ? GetUpper()->Grow( nDist - (nReal > 0 ? nReal : 0),
                                               pDirection, bTst, bInfo ) : 0;
    else
        nReal = nDist;

    if ( !bTst && GetNext() )
        GetNext()->InvalidatePos();

    return nReal;
}
/*************************************************************************
|*
|*  SwCntntFrm::ShrinkFrm()
|*
|*  Ersterstellung      MA 30. Jul. 92
|*  Letzte Aenderung    MA 05. May. 94
|*
|*************************************************************************/
SwTwips SwCntntFrm::ShrinkFrm( SwTwips nDist, const SzPtr pDirection,
                               BOOL bTst, BOOL bInfo )
{
    ASSERT( nDist >= 0, "nDist < 0" );
    ASSERT( nDist <= Frm().SSize().*pDirection,
            "nDist > als aktuelle Grosse." );
    ASSERT( pDirection == pHeight, "VarSize eines Cntnt ist Breite !?!" );

    if ( !bTst )
    {
        SwTwips nRstHeight = GetUpper() ? GetUpper()->Frm().Top()
                            + GetUpper()->Prt().Top()
                            + GetUpper()->Prt().Height()
                            - Frm().Top() - Frm().Height() : 0;
        if( nRstHeight < 0 )
            nRstHeight = nDist + nRstHeight;
        else
            nRstHeight = nDist;
        Frm().SSize().Height() -= nDist;
        nDist = nRstHeight;
        if ( IsInTab() )
        {
            SwTabFrm *pTab = FindTabFrm();
            if ( pTab->GetTable()->GetHTMLTableLayout() &&
                 !pTab->IsJoinLocked() &&
                 !pTab->GetFmt()->GetDoc()->GetDocShell()->IsReadOnly() )
            {
                pTab->InvalidatePos();
                pTab->SetResizeHTMLTable();
            }
        }
    }

    const SwTwips nReal = GetUpper() && nDist > 0 ?
            GetUpper()->Shrink( nDist, pDirection, bTst, bInfo ) : 0;

    if ( !bTst )
    {
        //Die Position des naechsten Frm's veraendert sich auf jeden Fall.
        InvalidateNextPos();

        //Wenn ich keinen Nachfolger habe, so muss ich mich eben selbst um
        //die Retusche kuemmern.
        if ( !GetNext() )
            SetRetouche();
    }
    return nReal;
}

/*************************************************************************
|*
|*    SwCntntFrm::Modify()
|*
|*    Beschreibung
|*    Ersterstellung    AK 05-Mar-1991
|*    Letzte Aenderung  MA 13. Oct. 95
|*
|*************************************************************************/
void SwCntntFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
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
            SwFrm::Modify( &aOldSet, &aNewSet );
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        SwPageFrm *pPage = FindPageFrm();
        InvalidatePage( pPage );
        if ( nInvFlags & 0x01 )
            SetCompletePaint();
        if ( nInvFlags & 0x02 )
            _InvalidatePos();
        if ( nInvFlags & 0x04 )
            _InvalidateSize();
        if ( nInvFlags & 0x88 )
        {
            if( IsInSct() && !GetPrev() )
            {
                SwSectionFrm *pSect = FindSctFrm();
                if( pSect->ContainsAny() == this )
                {
                    pSect->_InvalidatePrt();
                    pSect->InvalidatePage( pPage );
                }
            }
            _InvalidatePrt();
        }
        SwFrm *pTmp;
        if ( 0 != (pTmp = GetIndNext()) && nInvFlags & 0x10)
        {
            pTmp->_InvalidatePrt();
            pTmp->InvalidatePage( pPage );
        }
        if ( nInvFlags & 0x80 && pTmp )
            pTmp->SetCompletePaint();
        if ( nInvFlags & 0x20 && 0 != (pTmp = GetPrev()) )
        {
            pTmp->_InvalidatePrt();
            pTmp->InvalidatePage( pPage );
        }
        if ( nInvFlags & 0x40 )
            InvalidateNextPos();
    }
}

void SwCntntFrm::_UpdateAttr( SfxPoolItem* pOld, SfxPoolItem* pNew,
                              BYTE &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    BOOL bClear = TRUE;
    USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch ( nWhich )
    {
        case RES_FMT_CHG:
            rInvFlags = 0xFF;
            /* kein break hier */

        case RES_PAGEDESC:                      //Attributaenderung (an/aus)
            if ( IsInDocBody() && !IsInTab() )
            {
                rInvFlags |= 0x02;
                SwPageFrm *pPage = FindPageFrm();
                if ( !GetPrev() )
                    CheckPageDescs( pPage );
                if ( pPage && GetAttrSet()->GetPageDesc().GetNumOffset() )
                    ((SwRootFrm*)pPage->GetUpper())->SetVirtPageNum( TRUE );
                SwDocPosUpdate aMsgHnt( pPage->Frm().Top() );
                pPage->GetFmt()->GetDoc()->UpdatePageFlds( &aMsgHnt );
            }
            break;

        case RES_UL_SPACE:
            {
                if( IsInFtn() && !GetIndNext() )
                {
                    SwFrm* pNxt = FindNext();
                    if( pNxt )
                    {
                        SwPageFrm* pPg = pNxt->FindPageFrm();
                        pNxt->InvalidatePage( pPg );
                        pNxt->_InvalidatePrt();
                        if( pNxt->IsSctFrm() )
                        {
                            SwFrm* pCnt = ((SwSectionFrm*)pNxt)->ContainsAny();
                            if( pCnt )
                            {
                                pCnt->_InvalidatePrt();
                                pCnt->InvalidatePage( pPg );
                            }
                        }
                        pNxt->SetCompletePaint();
                    }
                }
                Prepare( PREP_UL_SPACE );   //TxtFrm muss Zeilenabst. korrigieren.
                rInvFlags |= 0x80;
                /* kein Break hier */
            }
        case RES_LR_SPACE:
        case RES_BOX:
        case RES_SHADOW:
            Prepare( PREP_FIXSIZE_CHG );
            SwFrm::Modify( pOld, pNew );
            rInvFlags |= 0x30;
            break;

        case RES_BREAK:
            {
                rInvFlags |= 0x42;
                if( GetAttrSet()->GetDoc()->IsParaSpaceMax() ||
                    GetAttrSet()->GetDoc()->IsParaSpaceMaxAtPages() )
                {
                    rInvFlags |= 0x1;
                    SwFrm* pNxt = FindNext();
                    if( pNxt )
                    {
                        SwPageFrm* pPg = pNxt->FindPageFrm();
                        pNxt->InvalidatePage( pPg );
                        pNxt->_InvalidatePrt();
                        if( pNxt->IsSctFrm() )
                        {
                            SwFrm* pCnt = ((SwSectionFrm*)pNxt)->ContainsAny();
                            if( pCnt )
                            {
                                pCnt->_InvalidatePrt();
                                pCnt->InvalidatePage( pPg );
                            }
                        }
                        pNxt->SetCompletePaint();
                    }
                }
            }
            break;

        case RES_PARATR_TABSTOP:
        case RES_CHRATR_PROPORTIONALFONTSIZE:
        case RES_CHRATR_SHADOWED:
        case RES_CHRATR_AUTOKERN:
        case RES_CHRATR_UNDERLINE:
        case RES_CHRATR_KERNING:
        case RES_CHRATR_FONT:
        case RES_CHRATR_FONTSIZE:
        case RES_CHRATR_ESCAPEMENT:
        case RES_CHRATR_CONTOUR:
            rInvFlags |= 0x01;
            break;


        case RES_FRM_SIZE:
            rInvFlags |= 0x01;
            /* no break here */

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
            SwFrm::Modify( pOld, pNew );
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::SwLayoutFrm()
|*
|*  Ersterstellung      AK 14-Feb-1991
|*  Letzte Aenderung    MA 12. May. 95
|*
|*************************************************************************/
SwLayoutFrm::SwLayoutFrm( SwFrmFmt* pFmt ):
    SwFrm( pFmt ),
    pLower( 0 )
{
    nType = FRM_LAYOUT;

    const SwFmtFrmSize &rFmtSize = pFmt->GetFrmSize();
    if ( rFmtSize.GetSizeType() == ATT_FIX_SIZE )
        bFixHeight = TRUE;
}

/*-----------------10.06.99 09:42-------------------
 * SwLayoutFrm::InnerHeight()
 * --------------------------------------------------*/

SwTwips SwLayoutFrm::InnerHeight() const
{
    if( !Lower() )
        return 0;
    SwTwips nRet = 0;
    const SwFrm* pCnt = Lower();
    if( pCnt->IsColumnFrm() || pCnt->IsCellFrm() )
    {
        do
        {
            SwTwips nTmp = ((SwLayoutFrm*)pCnt)->InnerHeight();
            if( pCnt->GetValidPrtAreaFlag() )
                nTmp += pCnt->Frm().Height() - pCnt->Prt().Height();
            if( nRet < nTmp )
                nRet = nTmp;
            pCnt = pCnt->GetNext();
        } while ( pCnt );
    }
    else
    {
        do
        {
            nRet += pCnt->Frm().Height();
            if( pCnt->IsCntntFrm() && ((SwTxtFrm*)pCnt)->IsUndersized() )
                nRet += ((SwTxtFrm*)pCnt)->GetParHeight() - pCnt->Prt().Height();
            if( pCnt->IsLayoutFrm() && !pCnt->IsTabFrm() )
                nRet += ((SwLayoutFrm*)pCnt)->InnerHeight()-pCnt->Prt().Height();
            pCnt = pCnt->GetNext();
        } while( pCnt );

    }
    return nRet;
}

/*************************************************************************
|*
|*  SwLayoutFrm::GrowFrm()
|*
|*  Ersterstellung      MA 30. Jul. 92
|*  Letzte Aenderung    MA 23. Sep. 96
|*
|*************************************************************************/
SwTwips SwLayoutFrm::GrowFrm( SwTwips nDist, const SzPtr pDirection,
                              BOOL bTst, BOOL bInfo )
{
    //CellFrms geben das Grow auf jedenfall mal an ihren Upper weiter, sie
    //zwar in jedem Fall in beiden Richtungen Fix, aber vielleicht schafft
    //die Row ja den benoetigten Platz.
    //Auch ColumnFrms koennen es auf einen Versuch ankommen lassen,
    const FASTBOOL bBrowse = GetFmt()->GetDoc()->IsBrowseMode();
    const USHORT nType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse mit Body
    if( !(GetType() & nType) && HasFixSize( pDirection ) )
        return 0;

    if ( Frm().SSize().*pDirection > 0 &&
         nDist > (LONG_MAX - Frm().SSize().*pDirection) )
        nDist = LONG_MAX - Frm().SSize().*pDirection;

    SwTwips nMin = 0; //Bis zur Upper-Groesse kann ich auf jedenfall auffuellen.
    if ( GetUpper() )
    {
        //Wenn die VarSize nicht der Direction entspricht, so ist
        //es eben die FixSize; ich brauche dann nicht die Kette
        //zu messen sondern muss lediglich den Upper betrachen.
        if ( pDirection == (bVarHeight ? pHeight : pWidth) )
        {
            SwFrm *pFrm = GetUpper()->Lower();
            while ( pFrm )
            {   nMin += pFrm->Frm().SSize().*pDirection;
                pFrm = pFrm->GetNext();
            }
        }
        else
            nMin = Frm().SSize().*pDirection;
        nMin = GetUpper()->Prt().SSize().*pDirection - nMin;
        if ( nMin < 0 )
            nMin = 0;
    }

    if ( !bTst )
        Frm().SSize().*pDirection += nDist;

    //Natuerlich braucht der Upper nur soweit wie wirklich notwendig
    //ge'grow'ed zu werden.
    SwTwips nReal = nDist - nMin;

    if ( nReal > 0 )
    {
        if ( GetUpper() )
        {   // AdjustNeighbourhood jetzt auch in Spalten (aber nicht in Rahmen)
            BYTE nAdjust = GetUpper()->IsFtnBossFrm() ?
                ((SwFtnBossFrm*)GetUpper())->NeighbourhoodAdjustment( this )
                : NA_GROW_SHRINK;
            if( NA_ONLY_ADJUST == nAdjust )
                nReal = AdjustNeighbourhood( nReal, bTst );
            else
            {
                SwTwips nGrow = 0;
                if( NA_ADJUST_GROW == nAdjust )
                    nReal += AdjustNeighbourhood( nReal - nGrow, bTst );
                if( nGrow < nReal )
                    nGrow += GetUpper()->Grow( nReal - nGrow, pDirection,
                                               bTst, bInfo );
                if( NA_GROW_ADJUST == nAdjust && nGrow < nReal )
                    nReal += AdjustNeighbourhood( nReal - nGrow, bTst );
                if ( IsFtnFrm() && (nGrow != nReal) && GetNext() )
                {
                    //Fussnoten koennen ihre Nachfolger verdraengen.
                    SwTwips nSpace = bTst ? 0 : -nDist;
                    const SwFrm *pFrm = GetUpper()->Lower();
                    do
                    {   nSpace += pFrm->Frm().Height();
                        pFrm = pFrm->GetNext();
                    } while ( pFrm != GetNext() );
                    nSpace = GetUpper()->Prt().Height() - nSpace;
                    if ( nSpace < 0 )
                        nSpace = 0;
                    nSpace += nGrow;
                    if ( nReal > nSpace )
                        nReal = nSpace;
                    if ( nReal && !bTst )
                        ((SwFtnFrm*)this)->InvalidateNxtFtnCnts( FindPageFrm() );
                }
                else
                    nReal = nGrow;
            }
        }
        else
            nReal = 0;

        nReal += nMin;
    }
    else
        nReal = nDist;

    if ( !bTst )
    {
        if ( (nReal != nDist) &&
             (!IsCellFrm() || (IsCellFrm() && (pDirection == pWidth))) )
            //Den masslosen Wunsch koennen wir leider nur teilweise erfuellen,
            //Zellen werden bei Veraenderungen der Hoehe direkt von der Row
            //angepasst.
            Frm().SSize().*pDirection -= (nDist - nReal);

        if ( nReal )
        {
            SwPageFrm *pPage = FindPageFrm();
            if ( GetNext() )
            {
                GetNext()->_InvalidatePos();
                if ( GetNext()->IsCntntFrm() )
                    GetNext()->InvalidatePage( pPage );
            }
            if ( !IsPageBodyFrm() )
            {
                _InvalidateAll();
                InvalidatePage( pPage );
            }
            if ( !(GetType() & 0x1823) ) //Tab, Row, FtnCont, Root, Page
                NotifyFlys();

            if( IsCellFrm() )
                InvaPercentLowers();

            const SvxGraphicPosition ePos = GetFmt()->GetBackground().GetGraphicPos();
            if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
                SetCompletePaint();
        }
    }
    return nReal;
}
/*************************************************************************
|*
|*  SwLayoutFrm::ShrinkFrm()
|*
|*  Ersterstellung      MA 30. Jul. 92
|*  Letzte Aenderung    MA 25. Mar. 99
|*
|*************************************************************************/
SwTwips SwLayoutFrm::ShrinkFrm( SwTwips nDist, const SzPtr pDirection,
                                BOOL bTst, BOOL bInfo )
{
    //CellFrms geben das Shrink auf jedenfall mal an ihren Upper weiter,
    //sie sind zwar in beiden Richtungen fix, aber Upper wirds schon richten.
    //Auch ColumnFrms koennen es auf einen Versuch ankommen lassen,
    const FASTBOOL bBrowse = GetFmt()->GetDoc()->IsBrowseMode();
    const USHORT nType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse mit Body
    if( !(GetType() & nType) && HasFixSize( pDirection ) )
        return 0;

    ASSERT( nDist >= 0, "nDist < 0" );

    if ( nDist > Frm().SSize().*pDirection )
        nDist = Frm().SSize().*pDirection;

    //Es gilt eine unuebersichtliche Situation zu vermeiden:
    //Der Inhalt kann nach dem Shrink noch immer groesser als der Frm
    //bzw. seine PrtArea sein; in diesem Fall wird das Shrink auf den
    //Inhalt beschraenkt bzw. abgelehnt wenn ein Wachtstum notwendig waere.
    SwTwips nMin = 0;
    if ( Lower() )
    {
        if ( pDirection == (Lower()->bVarHeight ? pHeight : pWidth) )
        {   const SwFrm *pFrm = Lower();
            const long nTmp = Prt().SSize().*pDirection; //MA #63881#: Genug ist Genug
            while ( pFrm && nMin < nTmp )
            {   nMin += pFrm->Frm().SSize().*pDirection;
                pFrm = pFrm->GetNext();
            }
        }
    }
    SwTwips nReal = nDist;
    if ( nReal > (Prt().SSize().*pDirection - nMin) )
        nReal = Prt().SSize().*pDirection - nMin;
    if ( nReal <= 0 )
        return nDist;

    SwTwips nRealDist = nReal;
    if ( !bTst )
        Frm().SSize().*pDirection -= nReal;

    BYTE nAdjust = GetUpper() && GetUpper()->IsFtnBossFrm() ?
                   ((SwFtnBossFrm*)GetUpper())->NeighbourhoodAdjustment( this )
                   : NA_GROW_SHRINK;

    // AdjustNeighbourhood auch in Spalten (aber nicht in Rahmen)
    if( NA_ONLY_ADJUST == nAdjust )
    {
        if ( IsPageBodyFrm() && !bBrowse )
            nReal = nDist;
        else
        {   nReal = AdjustNeighbourhood( -nReal, bTst );
            nReal *= -1;
            if ( !bTst && IsBodyFrm() && nReal < nRealDist )
                Frm().SSize().*pDirection += nRealDist - nReal;
        }
    }
    else if ( (IsCellFrm() && !(pDirection == pHeight)) || IsColumnFrm() || IsColBodyFrm() )
    {   //Columns und auch ColumnBodies schrumpfen nur soweit wie es der Upper mitmacht.
        //Zellen auch, in der Hoehe werden sie aber direkt vom Upper
        //zurechtgewiesen, brauchen sich also hier nicht zu korrigieren.
        SwTwips nTmp = GetUpper()->Shrink( nReal, pDirection, bTst, bInfo );
        if ( nTmp != nReal )
        {
            Frm().SSize().*pDirection += nReal - nTmp;
            nReal = nTmp;
        }
    }
    else
    {
        SwTwips nShrink = nReal;
        nReal = GetUpper() ?
                    GetUpper()->Shrink( nShrink, pDirection, bTst, bInfo ) : 0;
        if( ( NA_GROW_ADJUST == nAdjust || NA_ADJUST_GROW == nAdjust )
            && nReal < nShrink )
            AdjustNeighbourhood( nReal - nShrink );
    }

    if ( !bTst && (IsCellFrm() || IsColumnFrm() ? nReal : nRealDist) )
    {
        SwPageFrm *pPage = FindPageFrm();
        if ( GetNext() )
        {
            GetNext()->_InvalidatePos();
            if ( GetNext()->IsCntntFrm() )
                GetNext()->InvalidatePage( pPage );
            if ( IsTabFrm() )
                ((SwTabFrm*)this)->SetComplete();
        }
        else
        {   if ( IsRetoucheFrm() )
                SetRetouche();
            if ( IsTabFrm() )
            {
                if( IsTabFrm() )
                    ((SwTabFrm*)this)->SetComplete();
                if ( Lower() )  //Kann auch im Join stehen und leer sein!
                    InvalidateNextPos();
            }
        }
        if ( !IsBodyFrm() )
        {
            _InvalidateAll();
            InvalidatePage( pPage );
            const SvxGraphicPosition ePos = GetFmt()->GetBackground().GetGraphicPos();
            if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
                SetCompletePaint();
        }

        if ( !(GetType() & 0x1823) ) //Tab, Row, FtnCont, Root, Page
            NotifyFlys();

        if( IsCellFrm() )
            InvaPercentLowers();

        if( IsFtnFrm() && !((SwFtnFrm*)this)->GetAttr()->GetFtn().IsEndNote() &&
            ( GetFmt()->GetDoc()->GetFtnInfo().ePos != FTNPOS_CHAPTER ||
              ( IsInSct() && FindSctFrm()->IsFtnAtEnd() ) ) )
        {

            SwCntntFrm *pCnt = ((SwFtnFrm*)this)->GetRef();
            ASSERT( pCnt, "Ftn ohne Ref." );
            if ( pCnt->IsFollow() )
            {   // Wenn wir sowieso schon in einer anderen Spalte/Seite sitzen
                // als der Frame mit der Referenz, dann brauchen wir nicht
                // auch noch seinen Master zu invalidieren.
                SwFrm *pTmp = pCnt->FindFtnBossFrm(TRUE) == FindFtnBossFrm(TRUE)
                              ?  pCnt->FindMaster()->GetFrm() : pCnt;
                pTmp->Prepare( PREP_ADJUST_FRM );
                pTmp->InvalidateSize();
            }
            else
                pCnt->InvalidatePos();
        }
    }
    return nReal;
}
/*************************************************************************
|*
|*  SwLayoutFrm::ChgLowersProp()
|*
|*  Beschreibung        Aendert die Grosse der direkt untergeordneten Frm's
|*      die eine Fixe Groesse haben, proportional zur Groessenaenderung der
|*      PrtArea des Frm's.
|*      Die Variablen Frm's werden auch proportional angepasst; sie werden
|*      sich schon wieder zurechtwachsen/-schrumpfen.
|*  Ersterstellung      MA 11.03.92
|*  Letzte Aenderung    AMA 2. Nov. 98
|*
|*************************************************************************/
void SwLayoutFrm::ChgLowersProp( const Size& rOldSize )
{
    if ( IsRootFrm() || !Lower() )
        return;

    BOOL bInvaCntnt = TRUE; //Einmal die Seite benachrichtigen.
    SwFrm *pFrm = Lower();
    const BOOL bHeightChgd = rOldSize.Height() != Prt().Height();
    const BOOL bWidthChgd  = rOldSize.Width()  != Prt().Width();

    //Abkuerzung fuer Body-Inhalt
    if( IsBodyFrm() && IsInDocBody() && !Lower()->IsColumnFrm() && !bWidthChgd
        && ( !IsInSct() || !FindSctFrm()->IsColLocked() ) )
    {
        SwPageFrm *pPage = FindPageFrm();
        if( pFrm )
        {
            do
            {
                if( pFrm->IsSctFrm() &&((SwSectionFrm*)pFrm)->_ToMaximize() )
                {
                    pFrm->_InvalidateSize();
                    pFrm->InvalidatePage( pPage );
                }
                if( pFrm->GetNext() )
                    pFrm = pFrm->GetNext();
                else
                    break;
            } while( TRUE );
            while( pFrm->IsSctFrm() && !((SwSectionFrm*)pFrm)->GetSection() &&
                   pFrm->GetPrev() )
                pFrm = pFrm->GetPrev();
            if( pFrm->IsSctFrm() )
                pFrm = ((SwSectionFrm*)pFrm)->GetSection() &&
                       !((SwSectionFrm*)pFrm)->ToMaximize( FALSE ) ?
                       ((SwSectionFrm*)pFrm)->FindLastCntnt() : NULL;
        }
        if ( pFrm )
        {
            if ( pFrm->IsInTab() )
                pFrm = pFrm->FindTabFrm();
            if ( rOldSize.Height() < Prt().Height() )
            {
                //Wenn der Body gewachsen ist, genuegt es den auf den letzten
                //und den darauf folgenden geeignet zu invalidieren.
                pFrm->_InvalidateAll();
                pFrm->InvalidatePage( pPage );
                if( !pFrm->IsFlowFrm() ||
                    !SwFlowFrm::CastFlowFrm( pFrm )->HasFollow() )
                    pFrm->InvalidateNextPos( TRUE );
                if ( pFrm->IsTxtFrm() )
                    ((SwCntntFrm*)pFrm)->Prepare( PREP_ADJUST_FRM );
                if ( pFrm->IsInSct() )
                {
                    pFrm = pFrm->FindSctFrm();
                    if( IsAnLower( pFrm ) )
                    {
                        pFrm->_InvalidateSize();
                        pFrm->InvalidatePage( pPage );
                    }
                }
            }
            else
            {
                //Anderfalls werden alle herausragenden in ihrer Position
                //invalidiert und nur der letzte noch (teilweise) passende
                //Adjustiert.
                SwTwips nBot = Frm().Top() + Prt().Bottom();
                while ( pFrm->GetPrev() && pFrm->Frm().Top() > nBot )
                {
                    pFrm->_InvalidateAll();
                    pFrm->InvalidatePage( pPage );
                    pFrm = pFrm->GetPrev();
                }
                if ( pFrm )
                {
                    pFrm->_InvalidateSize();
                    pFrm->InvalidatePage( pPage );
                    if ( pFrm->IsTxtFrm() )
                        ((SwCntntFrm*)pFrm)->Prepare( PREP_ADJUST_FRM );
                    if ( pFrm->IsInSct() )
                    {
                        pFrm = pFrm->FindSctFrm();
                        if( IsAnLower( pFrm ) )
                        {
                            pFrm->_InvalidateSize();
                            pFrm->InvalidatePage( pPage );
                        }
                    }
                }
            }
        }
        return;
    }

    BOOL  bFixChgd, bVarChgd;
    if ( pFrm->bVarHeight )
    {
        bFixChgd = bWidthChgd;
        bVarChgd = bHeightChgd;
    }
    else
    {
        bFixChgd = bHeightChgd;
        bVarChgd = bWidthChgd;
    }
    while ( pFrm )
    {   //TxtFrms werden invalidiert, andere werden nur proportional angepasst.
        if ( pFrm->IsTxtFrm() )
        {
            if ( bFixChgd )
                ((SwCntntFrm*)pFrm)->Prepare( PREP_FIXSIZE_CHG );
            if ( bVarChgd )
                ((SwCntntFrm*)pFrm)->Prepare( PREP_ADJUST_FRM );
        }
        else if ( !(pFrm->GetType() & (FRM_TAB|FRM_ROW|FRM_CELL|FRM_SECTION)) )
        {
            //Der Frame wird Proportional angepasst.
            //Die FixSize des Lowers wird direkt an die neue Groesse
            //angepasst, so werden Rundungsfehler vermieden.

            //Neue Breite
            if ( bWidthChgd )
            {
                if ( pFrm->bVarHeight )
                    pFrm->Frm().Width( Prt().Width() );
                else if ( (pFrm->GetType() & FRM_COLUMN) && rOldSize.Width() )
                    pFrm->Frm().Width( (pFrm->Frm().Width() * Prt().Width()) /
                                       rOldSize.Width() );
            }
            //Neue Hoehe
            if ( bHeightChgd )
            {
                if ( !pFrm->bVarHeight )
                    pFrm->Frm().Height( Prt().Height() );
                else if ( (pFrm->GetType() & FRM_COLUMN) && rOldSize.Height() )
                    pFrm->Frm().Height( (pFrm->Frm().Height() * Prt().Height()) /
                                        rOldSize.Height() );
            }
        }
        pFrm->_InvalidateAll();
        if ( bInvaCntnt && pFrm->IsCntntFrm() )
            pFrm->InvalidatePage();

        if ( !pFrm->GetNext() && pFrm->IsRetoucheFrm() )
        {
            //Wenn ein Wachstum stattgefunden hat, und die untergeordneten
            //zur Retouche faehig sind (derzeit Tab, Section und Cntnt), so
            //trigger ich sie an.
            if ( rOldSize.Height() < Prt().SSize().Height() ||
                 rOldSize.Width() < Prt().SSize().Width() )
                pFrm->SetRetouche();
        }
        pFrm = pFrm->GetNext();
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::Format()
|*
|*  Beschreibung:       "Formatiert" den Frame; Frm und PrtArea.
|*                      Die Fixsize wird hier nicht eingestellt.
|*  Ersterstellung      MA 28. Jul. 92
|*  Letzte Aenderung    MA 21. Mar. 95
|*
|*************************************************************************/
void SwLayoutFrm::Format( const SwBorderAttrs *pAttrs )
{
    ASSERT( pAttrs, "LayoutFrm::Format, pAttrs ist 0." );

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
            const SwTwips nBorder = bVarHeight ? nUL : nLR;
            const PtPtr pVarPs = pVARPOS;
            const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
            SwTwips nMinHeight = rSz.GetSizeType() == ATT_MIN_SIZE ? rSz.GetHeight() : 0;
            do
            {   bValidSize = TRUE;

                //Die Groesse in der VarSize wird durch den Inhalt plus den
                //Raendern bestimmt.
                SwTwips nRemaining = 0;
                SwFrm *pFrm = Lower();
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
                nRemaining += nBorder;
                nRemaining = Max( nRemaining, nMinHeight );
                const SwTwips nDiff = nRemaining - Frm().SSize().*pVarSz;
                const Point aOldPos = Frm().Pos();
                if ( nDiff )
                {
                    if ( nDiff > 0 )
                        Grow( nDiff, pVarSz );
                    else
                        Shrink( -nDiff, pVarSz );
                    //Schnell auf dem kurzen Dienstweg die Position updaten.
                    MakePos();
                }
                //Unterkante des Uppers nicht ueberschreiten.
                if ( GetUpper() && Frm().SSize().*pVarSz )
                {
                    const SwTwips nDeadLine = GetUpper()->Frm().Top() +
                        GetUpper()->Prt().Top() + GetUpper()->Prt().Height();
                    const SwTwips nBot = Frm().Top() + Frm().Height();
                    if ( nBot > nDeadLine )
                    {
                        Frm().Height( nDeadLine - Frm().Top() );
                        Prt().SSize().Height() = Frm().SSize().Height() - nBorder;
                        if ( Frm().Pos() == aOldPos )
                            bValidSize = bValidPrtArea = TRUE;
                    }
                }
            } while ( !bValidSize );
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

/*************************************************************************
|*
|*  SwLayoutFrm::InvalidatePercentLowers()
|*
|*  Ersterstellung      MA 13. Jun. 96
|*  Letzte Aenderung    MA 13. Jun. 96
|*
|*************************************************************************/
static void InvaPercentFlys( SwFrm *pFrm )
{
    ASSERT( pFrm->GetDrawObjs(), "Can't find any Objects" );
    for ( USHORT i = 0; i < pFrm->GetDrawObjs()->Count(); ++i )
    {
        SdrObject *pO = (*pFrm->GetDrawObjs())[i];
        if ( pO->IsWriterFlyFrame() )
        {
            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
            const SwFmtFrmSize &rSz = pFly->GetFmt()->GetFrmSize();
            if ( rSz.GetWidthPercent() || rSz.GetHeightPercent() )
                pFly->InvalidateSize();
        }
    }
}

void SwLayoutFrm::InvaPercentLowers()
{
    if ( GetDrawObjs() )
        ::InvaPercentFlys( this );

    SwFrm *pFrm = ContainsCntnt();
    if ( pFrm )
        do
        {
            if ( pFrm->IsInTab() && !IsTabFrm() )
            {
                SwFrm *pTmp = pFrm->FindTabFrm();
                ASSERT( pTmp, "Where's my TabFrm?" );
                if( IsAnLower( pTmp ) )
                    pFrm = pTmp;
            }

            if ( pFrm->IsTabFrm() )
            {
                const SwFmtFrmSize &rSz = ((SwLayoutFrm*)pFrm)->GetFmt()->GetFrmSize();
                if ( rSz.GetWidthPercent() || rSz.GetHeightPercent() )
                    pFrm->InvalidatePrt();
            }
            else if ( pFrm->GetDrawObjs() )
                ::InvaPercentFlys( pFrm );
            pFrm = pFrm->FindNextCnt();
        } while ( pFrm && IsAnLower( pFrm ) ) ;
}

/*************************************************************************
|*
|*  SwLayoutFrm::CalcRel()
|*
|*  Ersterstellung      MA 13. Jun. 96
|*  Letzte Aenderung    MA 10. Oct. 96
|*
|*************************************************************************/
long SwLayoutFrm::CalcRel( const SwFmtFrmSize &rSz, BOOL bWidth ) const
{
    ASSERT( bWidth, "NonFlys, CalcRel: width only" );

    long nRet     = rSz.GetWidth(),
         nPercent = rSz.GetWidthPercent();

    if ( nPercent )
    {
        const SwFrm *pRel = GetUpper();
        long nRel = LONG_MAX;
        const ViewShell *pSh = GetShell();
        if ( pRel->IsPageBodyFrm() && GetFmt()->GetDoc()->IsBrowseMode() &&
             pSh && pSh->VisArea().Width())
        {
            nRel = pSh->VisArea().Width();
            const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
            nRel -= 2*aBorder.Width();
            long nDiff = nRel - pRel->Prt().Width();
            if ( nDiff > 0 )
                nRel -= nDiff;
        }
        nRel = Min( nRel, pRel->Prt().Width() );
        nRet = nRel * nPercent / 100;
    }
    return nRet;
}

long MA_FASTCALL lcl_CalcMinColDiff( SwLayoutFrm *pLayFrm )
{
    long nDiff = 0, nFirstDiff = 0;
    SwLayoutFrm *pCol = (SwLayoutFrm*)pLayFrm->Lower();
    ASSERT( pCol, "Where's the columnframe?" );
    SwFrm *pFrm = pCol->Lower();
    do
    {   if ( pFrm && pFrm->IsTxtFrm() )
        {
            const long nTmp = ((SwTxtFrm*)pFrm)->FirstLineHeight();
            if ( nTmp != USHRT_MAX )
            {
                if ( pCol == pLayFrm->Lower() )
                    nFirstDiff = nTmp;
                else
                    nDiff = nDiff ? Min( nDiff, nTmp ) : nTmp;
            }
        }
        //Leere Spalten ueberspringen!
        pCol = (SwLayoutFrm*)pCol->GetNext();
        while ( pCol && 0 == (pFrm = pCol->Lower()) )
            pCol = (SwLayoutFrm*)pCol->GetNext();

    } while ( pFrm && pCol );

    return nDiff ? nDiff : nFirstDiff ? nFirstDiff : 240;
}

BOOL lcl_IsFlyHeightClipped( SwLayoutFrm *pLay )
{
    SwFrm *pFrm = pLay->ContainsCntnt();
    while ( pFrm )
    {   if ( pFrm->IsInTab() )
            pFrm = pFrm->FindTabFrm();

        if ( pFrm->GetDrawObjs() )
        {
            USHORT nCnt = pFrm->GetDrawObjs()->Count();
            for ( USHORT i = 0; i < nCnt; ++i )
            {
                SdrObject *pO = (*pFrm->GetDrawObjs())[i];
                if ( pO->IsWriterFlyFrame() &&
                     ((SwVirtFlyDrawObj*)pO)->GetFlyFrm()->IsHeightClipped() )
                    return TRUE;
            }
        }
        pFrm = pFrm->FindNextCnt();
    }
    return FALSE;
}

void SwLayoutFrm::FormatWidthCols( const SwBorderAttrs &rAttrs,
    const SwTwips nBorder, const SwTwips nMinHeight )
{
    //Wenn Spalten im Spiel sind, so wird die Groesse an der
    //letzten Spalte ausgerichtet.
    //1. Inhalt formatieren.
    //2. Hoehe der letzten Spalte ermitteln, wenn diese zu
    //   zu gross ist muss der Fly wachsen.
    //   Der Betrag um den der Fly waechst ist aber nicht etwa
    //   der Betrag des Ueberhangs, denn wir muessen davon
    //   ausgehen, dass etwas Masse zurueckfliesst und so
    //   zusaetzlicher Platz geschaffen wird.
    //   Im Ersten Ansatz ist der Betrag um den gewachsen wird
    //   der Ueberhang geteilt durch die Spaltenanzahl oder
    //   der Ueberhang selbst wenn er kleiner als die Spalten-
    //   anzahl ist.
    //3. Weiter mit 1. bis zur Stabilitaet.

    const SwFmtCol &rCol = rAttrs.GetAttrSet().GetCol();
    const USHORT nNumCols = rCol.GetNumCols();

    FASTBOOL bEnd = FALSE;
    FASTBOOL bBackLock = FALSE;
    SwViewImp *pImp = GetShell() ? GetShell()->Imp() : 0;
    {
        // Zugrunde liegender Algorithmus
        // Es wird versucht, eine optimale Hoehe fuer die Spalten zu finden.
        // nMinimum beginnt mit der uebergebenen Mindesthoehe und wird dann als
        // Maximum der Hoehen gepflegt, bei denen noch Spalteninhalt aus einer
        // Spalte herausragt.
        // nMaximum beginnt bei LONG_MAX und wird als Minimum der Hoehen gepflegt,
        // bei denen der Inhalt gepasst hat.
        // Bei spaltigen Bereichen beginnt nMaximum bei dem maximalen Wert, den
        // die Umgebung vorgibt, dies kann natuerlich ein Wert sein, bei dem noch
        // Inhalt heraushaengt.
        // Es werden die Spalten formatiert, wenn Inhalt heraushaengt, wird nMinimum
        // ggf. angepasst, dann wird gewachsen, mindestens um nMinDiff, aber nicht ueber
        // ein groesseres nMaximum hinaus. Wenn kein Inhalt heraushaengt, sondern
        // noch Luft in einer Spalte ist, schrumpfen wir entsprechend, mindestens um
        // nMinDiff, aber nicht unter das nMinimum.
        // Abgebrochen wird, wenn kein Inhalt mehr heraushaengt und das Minimum sich auf
        // weniger als ein MinDiff dem Maximum angenaehert hat oder das von der
        // Umgebung vorgegebene Maximum erreicht ist und trotzdem Inhalt heraus-
        // haengt.

        // Kritik an der Implementation
        // 1. Es kann theoretisch Situationen geben, in denen der Inhalt in einer geringeren
        // Hoehe passt und in einer groesseren Hoehe nicht passt. Damit der Code robust
        // gegen solche Verhaeltnisse ist, sind ein paar Abfragen bezgl. Minimum und Maximum
        // drin, die wahrscheinlich niemals zuschlagen koennen.
        // 2. Es wird fuer das Schrumpfen das gleiche nMinDiff benutzt wie fuer das Wachstum,
        // das nMinDiff ist allerdings mehr oder weniger die kleinste erste Zeilenhoehe und
        // als Mindestwert fuer das Schrumpfen nicht unbedingt optimal.

        long nMinimum = nMinHeight;
        long nMaximum;
        BOOL bNoBalance = FALSE;
        if( IsSctFrm() )
        {
            nMaximum = GetUpper()->Frm().Top() + GetUpper()->Prt().Top()
                       + GetUpper()->Prt().Height() - Frm().Top() - nBorder;
            nMaximum += GetUpper()->Grow( LONG_MAX, pHeight, TRUE );
            if( nMaximum < nMinimum )
            {
                if( nMaximum < 0 )
                    nMinimum = nMaximum = 0;
                else
                    nMinimum = nMaximum;
            }
            if( nMaximum > BROWSE_HEIGHT )
                nMaximum = BROWSE_HEIGHT;

            bNoBalance = ((SwSectionFrm*)this)->GetSection()->GetFmt()->
                         GetBalancedColumns().GetValue();
            SwFrm* pAny = ContainsAny();
            if( bNoBalance ||
                ( !Frm().Height() && pAny ) ) // presumably a fresh one
            {
                Frm().Height( nMaximum );
                if( Prt().Top() > Frm().Height() )
                    Prt().Pos().Y() = Frm().Height();
                Prt().Height( nMaximum - Prt().Top() );
            }
            if( !pAny && !((SwSectionFrm*)this)->IsFtnLock() )
            {
                SwFtnContFrm* pFtnCont = ((SwSectionFrm*)this)->ContainsFtnCont();
                if( pFtnCont )
                {
                    SwFrm* pFtnAny = pFtnCont->ContainsAny();
                    if( pFtnAny && pFtnAny->IsValid() )
                    {
                        bBackLock = TRUE;
                        ((SwSectionFrm*)this)->SetFtnLock( TRUE );
                    }
                }
            }
        }
        else
            nMaximum = LONG_MAX;
        do
        {
            //Kann eine Weile dauern, deshalb hier auf Waitcrsr pruefen.
            if ( pImp )
                pImp->CheckWaitCrsr();

            bValidSize = TRUE;
            //Erstmal die Spalten formatieren, das entlastet den
            //Stack ein wenig.
            //Bei der Gelegenheit stellen wir auch gleich mal die
            //Breiten und Hoehen der Spalten ein (so sie denn falsch sind).
            SwLayoutFrm *pCol = (SwLayoutFrm*)Lower();
            SwTwips nAvail = Prt().Width();
            for ( USHORT i = 0; i < nNumCols; ++i )
            {
                SwTwips nWidth = rCol.CalcColWidth( i, USHORT(Prt().Width()));
                if ( i == (nNumCols - 1) ) //Dem Letzten geben wir wie
                    nWidth = nAvail;       //immer den Rest.
                if ( pCol->Frm().Width() != nWidth )
                {
                    pCol->Frm().Width( nWidth );
                    pCol->_InvalidatePrt();
                    if ( pCol->GetNext() )
                        pCol->GetNext()->_InvalidatePos();
                }
                if ( pCol->Frm().Height() != Prt().Height() )
                {
                    pCol->Frm().Height( Prt().Height() );
                    pCol->_InvalidatePrt();
                }
                pCol->Calc();
                // ColumnFrms besitzen jetzt einen BodyFrm, der auch kalkuliert werden will
                pCol->Lower()->Calc();
                if( pCol->Lower()->GetNext() )
                    pCol->Lower()->GetNext()->Calc();  // SwFtnCont
                pCol = (SwLayoutFrm*)pCol->GetNext();
                nAvail -= nWidth;
            }

            ::CalcCntnt( this );

            pCol = (SwLayoutFrm*)Lower();
            ASSERT( pCol && pCol->GetNext(), ":-( Spalten auf Urlaub?");
            // bMinDiff wird gesetzt, wenn es keine leere Spalte gibt
            BOOL bMinDiff = TRUE;
            while ( bMinDiff && pCol && pCol->GetNext() )
            {   // Zwischen Spalte und Inhalt ist jetzt noch der BodyFrm gekommen
                bMinDiff = 0 != ((SwLayoutFrm*)pCol->Lower())->Lower();
                pCol = (SwLayoutFrm*)pCol->GetNext();
            }
            pCol = (SwLayoutFrm*)Lower();
            SwFrm *pLow;
            SwTwips nDiff = 0;
            SwTwips nMaxFree = 0;
            SwTwips nAllFree = LONG_MAX;
            // bFoundLower wird gesetzt, wenn es mind. eine nichtleere Spalte gibt
            BOOL bFoundLower = FALSE;
            while( pCol )
            {
                SwLayoutFrm* pLay = (SwLayoutFrm*)pCol->Lower();
                SwTwips nInnerHeight = pLay->Frm().Height() - pLay->Prt().Height();
                if( pLay->Lower() )
                {
                    bFoundLower = TRUE;
                    nInnerHeight += pLay->InnerHeight();
                }
                else if( nInnerHeight < 0 )
                    nInnerHeight = 0;

                if( pLay->GetNext() )
                {
                    bFoundLower = TRUE;
                    pLay = (SwLayoutFrm*)pLay->GetNext();
                    ASSERT( pLay->IsFtnContFrm(),"FtnContainer exspected" );
                    nInnerHeight += pLay->InnerHeight();
                    nInnerHeight += pLay->Frm().Height() - pLay->Prt().Height();
                }
                nInnerHeight -= pCol->Prt().Height();
                if( nInnerHeight > nDiff )
                {
                    nDiff = nInnerHeight;
                    nAllFree = 0;
                }
                else
                {
                    if( nMaxFree < -nInnerHeight )
                        nMaxFree = -nInnerHeight;
                    if( nAllFree > -nInnerHeight )
                        nAllFree = -nInnerHeight;
                }
                pCol = (SwLayoutFrm*)pCol->GetNext();
            }

            if ( bFoundLower || ( IsSctFrm() && ((SwSectionFrm*)this)->HasFollow() ) )
            {
                SwTwips nMinDiff = ::lcl_CalcMinColDiff( this );
                // Hier wird entschieden, ob wir wachsen muessen, naemlich wenn
                // ein Spalteninhalt (nDiff) oder ein Fly herausragt.
                // Bei spaltigen Bereichen wird beruecksichtigt, dass mit dem
                // Besitz eines nichtleeren Follows die Groesse festgelegt ist.
                if ( nDiff || ::lcl_IsFlyHeightClipped( this ) ||
                     ( IsSctFrm() && ((SwSectionFrm*)this)->CalcMinDiff( nMinDiff ) ) )
                {
                    // Das Minimum darf nicht kleiner sein als unsere PrtHeight,
                    // solange noch etwas herausragt.
                    if( nMinimum < Prt().Height() )
                        nMinimum = Prt().Height();
                    // Es muss sichergestellt sein, dass das Maximum nicht kleiner
                    // als die PrtHeight ist, wenn noch etwas herausragt
                    if( nMaximum < Prt().Height() )
                        nMaximum = Prt().Height();  // Robust, aber kann das ueberhaupt eintreten?
                    if( !nDiff ) // wenn nur Flys herausragen, wachsen wir um nMinDiff
                        nDiff = nMinDiff;
                    // Wenn wir um mehr als nMinDiff wachsen wollen, wird dies auf die
                    // Spalten verteilt
                    if ( Abs(nDiff - nMinDiff) > nNumCols && nDiff > (long)nNumCols )
                        nDiff /= nNumCols;

                    if ( bMinDiff )
                    {   // Wenn es keinen leeren Spalten gibt, wollen wir mind. um nMinDiff
                        // wachsen. Sonderfall: Wenn wir kleiner als die minimale Frmhoehe
                        // sind und die PrtHeight kleiner als nMinDiff ist, wachsen wir so,
                        // dass die PrtHeight hinterher genau nMinDiff ist.
                        if ( Frm().Height() > nMinHeight || Prt().Height() >= nMinDiff )
                            nDiff = Max( nDiff, nMinDiff );
                        else if( nDiff < nMinDiff )
                            nDiff = nMinDiff - Prt().Height() + 1;
                    }
                    // nMaximum ist eine Groesse, in der der Inhalt gepasst hat,
                    // oder der von der Umgebung vorgegebene Wert, deshalb
                    // brauchen wir nicht ueber diesen Wrt hinauswachsen.
                    if( nDiff + Prt().Height() > nMaximum )
                        nDiff = nMaximum - Prt().Height();
                }
                else if( nMaximum > nMinimum ) // Wir passen, haben wir auch noch Spielraum?
                {
                    if ( nMaximum < Prt().Height() )
                        nDiff = nMaximum - Prt().Height(); // wir sind ueber eine funktionierende
                        // Hoehe hinausgewachsen und schrumpfen wieder auf diese zurueck,
                        // aber kann das ueberhaupt eintreten?
                    else
                    {   // Wir haben ein neues Maximum, eine Groesse, fuer die der Inhalt passt.
                        nMaximum = Prt().Height();
                        // Wenn der Freiraum in den Spalten groesser ist als nMinDiff und wir
                        // nicht dadurch wieder unter das Minimum rutschen, wollen wir ein wenig
                        // Luft herauslassen.
                        if( !bNoBalance && ( nMaxFree >= nMinDiff && (!nAllFree
                            || nMinimum < Prt().Height() - nMinDiff ) ) )
                        {
                            nMaxFree /= nNumCols; // auf die Spalten verteilen
                            nDiff = nMaxFree < nMinDiff ? -nMinDiff : -nMaxFree; // mind. nMinDiff
                            if( Prt().Height() + nDiff <= nMinimum ) // Unter das Minimum?
                                nDiff = ( nMinimum - nMaximum ) / 2; // dann lieber die Mitte
                        }
                        else if( nAllFree )
                        {
                            nDiff = -nAllFree;
                            if( Prt().Height() + nDiff <= nMinimum ) // Less than minimum?
                                nDiff = ( nMinimum - nMaximum ) / 2; // Take the center
                        }
                    }
                }
                if( nDiff ) // jetzt wird geschrumpft oder gewachsen..
                {
                    Size aOldSz( Prt().SSize() );
                    Prt().Height( Prt().Height() + nDiff );
                    Frm().Height( Prt().Height() + nBorder);
                    ChgLowersProp( aOldSz );
                    NotifyFlys();

                    //Es muss geeignet invalidiert werden, damit
                    //sich die Frms huebsch ausbalancieren
                    //- Der jeweils erste ab der zweiten Spalte bekommt
                    //  ein InvalidatePos();
                    pCol = (SwLayoutFrm*)Lower()->GetNext();
                    while ( pCol )
                    {
                        pLow = pCol->Lower();
                        if ( pLow )
                            pLow->_InvalidatePos();
                        pCol = (SwLayoutFrm*)pCol->GetNext();
                    }
                    if( IsSctFrm() && ((SwSectionFrm*)this)->HasFollow() )
                    {
                        // Wenn wir einen Follow erzeugt haben, muessen wir
                        // seinem Inhalt die Chance geben, im CalcCntnt
                        // zurueckzufliessen
                        SwCntntFrm* pTmpCntnt =
                            ((SwSectionFrm*)this)->GetFollow()->ContainsCntnt();
                        if( pTmpCntnt )
                            pTmpCntnt->_InvalidatePos();
                    }
                }
                else
                    bEnd = TRUE;
            }
            else
                bEnd = TRUE;

        } while ( !bEnd || !bValidSize );
    }
    ::CalcCntnt( this );
    if( IsSctFrm() )
    {
        ::CalcCntnt( this, TRUE );
        if( bBackLock )
            ((SwSectionFrm*)this)->SetFtnLock( FALSE );
    }
}


/*************************************************************************
|*
|*  SwRootFrm::InvalidateAllCntnt()
|*
|*  Ersterstellung      MA 13. Feb. 98
|*  Letzte Aenderung    MA 12. Aug. 00
|*
|*************************************************************************/

SwCntntFrm* lcl_InvalidateSection( SwFrm *pCnt, BYTE nInv )
{
    SwSectionFrm* pSect = pCnt->FindSctFrm();
    // Wenn unser CntntFrm in einer Tabelle oder Fussnote steht, sind nur
    // Bereiche gemeint, die ebenfalls innerhalb liegen.
    // Ausnahme: Wenn direkt eine Tabelle uebergeben wird.
    if( ( ( pCnt->IsInTab() && !pSect->IsInTab() ) ||
        ( pCnt->IsInFtn() && !pSect->IsInFtn() ) ) && !pCnt->IsTabFrm() )
        return NULL;
    if( nInv & INV_SIZE )
        pSect->_InvalidateSize();
    if( nInv & INV_POS )
        pSect->_InvalidatePos();
    if( nInv & INV_PRTAREA )
        pSect->_InvalidatePrt();
    SwFlowFrm *pFoll = pSect->GetFollow();
    // Temporary separation from follow
    pSect->SetFollow( NULL );
    SwCntntFrm* pRet = pSect->FindLastCntnt();
    pSect->SetFollow( pFoll );
    return pRet;
}

SwCntntFrm* lcl_InvalidateTable( SwTabFrm *pTable, BYTE nInv )
{
    if( ( nInv & INV_SECTION ) && pTable->IsInSct() )
        lcl_InvalidateSection( pTable, nInv );
    if( nInv & INV_SIZE )
        pTable->_InvalidateSize();
    if( nInv & INV_POS )
        pTable->_InvalidatePos();
    if( nInv & INV_PRTAREA )
        pTable->_InvalidatePrt();
    return pTable->FindLastCntnt();
}

void lcl_InvalidateAllCntnt( SwCntntFrm *pCnt, BYTE nInv );

void lcl_InvalidateCntnt( SwCntntFrm *pCnt, BYTE nInv )
{
    SwCntntFrm *pLastTabCnt = NULL;
    SwCntntFrm *pLastSctCnt = NULL;
    while ( pCnt )
    {
        if( nInv & INV_SECTION )
        {
            if( pCnt->IsInSct() )
            {
                // Siehe oben bei Tabellen
                if( !pLastSctCnt )
                    pLastSctCnt = lcl_InvalidateSection( pCnt, nInv );
                if( pLastSctCnt == pCnt )
                    pLastSctCnt = NULL;
            }
#ifndef PRODUCT
            else
                ASSERT( !pLastSctCnt, "Where's the last SctCntnt?" );
#endif
        }
        if( nInv & INV_TABLE )
        {
            if( pCnt->IsInTab() )
            {
                // Um nicht fuer jeden CntntFrm einer Tabelle das FindTabFrm() zu rufen
                // und wieder die gleiche Tabelle zu invalidieren, merken wir uns den letzten
                // CntntFrm der Tabelle und reagieren erst wieder auf IsInTab(), wenn wir
                // an diesem vorbei sind.
                // Beim Eintritt in die Tabelle wird der LastSctCnt auf Null gesetzt,
                // damit Bereiche im Innern der Tabelle richtig invalidiert werden.
                // Sollte die Tabelle selbst in einem Bereich stehen, so wird an
                // diesem die Invalidierung bis zu dreimal durchgefuehrt, das ist vertretbar.
                if( !pLastTabCnt )
                {
                    pLastTabCnt = lcl_InvalidateTable( pCnt->FindTabFrm(), nInv );
                    pLastSctCnt = NULL;
                }
                if( pLastTabCnt == pCnt )
                {
                    pLastTabCnt = NULL;
                    pLastSctCnt = NULL;
                }
            }
#ifndef PRODUCT
            else
                ASSERT( !pLastTabCnt, "Where's the last TabCntnt?" );
#endif
        }

        if( nInv & INV_SIZE )
            pCnt->Prepare( PREP_CLEAR, 0, FALSE );
        if( nInv & INV_POS )
            pCnt->_InvalidatePos();
        if( nInv & INV_PRTAREA )
            pCnt->_InvalidatePrt();
        if ( nInv & INV_LINENUM )
            pCnt->InvalidateLineNum();
        if ( pCnt->GetDrawObjs() )
            lcl_InvalidateAllCntnt( pCnt, nInv );
        pCnt = pCnt->GetNextCntntFrm();
    }
}

void lcl_InvalidateAllCntnt( SwCntntFrm *pCnt, BYTE nInv )
{
    SwDrawObjs &rObjs = *pCnt->GetDrawObjs();
    for ( USHORT i = 0; i < rObjs.Count(); ++i )
    {
        SdrObject *pO = rObjs[i];
        if ( pO->IsWriterFlyFrame() )
        {
            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
            if ( pFly->IsFlyInCntFrm() )
                ::lcl_InvalidateCntnt( pFly->ContainsCntnt(), nInv );
        }
    }
}

void SwRootFrm::InvalidateAllCntnt( BYTE nInv )
{
    // Erst werden alle Seitengebundenen FlyFrms abgearbeitet.
    SwPageFrm *pPage = (SwPageFrm*)Lower();
    while( pPage )
    {
        pPage->InvalidateFlyLayout();
        pPage->InvalidateFlyCntnt();
        pPage->InvalidateFlyInCnt();
        pPage->InvalidateLayout();
        pPage->InvalidateCntnt();
        pPage->InvalidatePage( pPage ); //Damit ggf. auch der Turbo verschwindet

        if ( pPage->GetSortedObjs() )
        {
            const SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
            for ( USHORT i = 0; i < rObjs.Count(); ++i )
            {
                SdrObject *pO = rObjs[i];
                if ( pO->IsWriterFlyFrame() )
                    ::lcl_InvalidateCntnt( ((SwVirtFlyDrawObj*)pO)->GetFlyFrm()->ContainsCntnt(),
                                         nInv );
            }
        }
        pPage = (SwPageFrm*)(pPage->GetNext());
    }

    //Hier den gesamten Dokumentinhalt und die zeichengebundenen Flys.
    ::lcl_InvalidateCntnt( ContainsCntnt(), nInv );

    if( nInv & INV_PRTAREA )
    {
        ViewShell *pSh  = GetShell();
        if( pSh )
            pSh->InvalidateWindows( Frm() );
    }
}


