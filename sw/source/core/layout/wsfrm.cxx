/*************************************************************************
 *
 *  $RCSfile: wsfrm.cxx,v $
 *
 *  $Revision: 1.58 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:13:55 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _PSTM_HXX
#include <tools/pstm.hxx>
#endif
#ifndef _OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _SFXITEMITER_HXX
#include <svtools/itemiter.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX
#include <svx/brkitem.hxx>
#endif

#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef _FMTFORDR_HXX
#include <fmtfordr.hxx>
#endif
#ifndef _FMTFSIZE_HXX
#include <fmtfsize.hxx>
#endif
#ifndef _FMTPDSC_HXX
#include <fmtpdsc.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX
#include <fmtftn.hxx>
#endif
#ifndef _FMTSRND_HXX
#include <fmtsrnd.hxx>
#endif
#ifndef _FTNFRM_HXX
#include <ftnfrm.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _HTMLTBL_HXX
#include <htmltbl.hxx>
#endif
#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>
#endif
#ifndef _FMTCLDS_HXX
#include <fmtclds.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _BODYFRM_HXX
#include <bodyfrm.hxx>
#endif
#ifndef _CELLFRM_HXX //autogen
#include <cellfrm.hxx>
#endif
#ifndef _DBG_LAY_HXX
#include <dbg_lay.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
// OD 2004-05-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif

#if OSL_DEBUG_LEVEL > 1

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
    bFlag01 = bFlag02 = bFlag03 = bFlag04 = bFlag05 = 0;
#if OSL_DEBUG_LEVEL > 1
    static USHORT nStopAt = USHRT_MAX;
    if ( nFrmId == nStopAt )
    {
        int bla = 5;
    }
#endif
#endif

    ASSERT( pMod, "Kein Frameformat uebergeben." );
    bInvalidR2L = bInvalidVert = 1;
    bDerivedR2L = bDerivedVert = bRightToLeft = bVertical = bReverse = 0;
    bValidPos = bValidPrtArea = bValidSize = bValidLineNum = bRetouche =
    bFixSize = bColLocked = FALSE;
    bCompletePaint = bInfInvalid = TRUE;
}

void SwFrm::CheckDir( UINT16 nDir, BOOL bVert, BOOL bOnlyBiDi, BOOL bBrowse )
{
    if( FRMDIR_ENVIRONMENT == nDir || ( bVert && bOnlyBiDi ) )
    {
        bDerivedVert = 1;
        if( FRMDIR_ENVIRONMENT == nDir )
            bDerivedR2L = 1;
        SetDirFlags( bVert );
    }
    else if( bVert )
    {
        bInvalidVert = 0;
        if( FRMDIR_HORI_LEFT_TOP == nDir || FRMDIR_HORI_RIGHT_TOP == nDir
            || bBrowse )
            bVertical = 0;
        else
            bVertical = 1;
    }
    else
    {
        bInvalidR2L = 0;
        if( FRMDIR_HORI_RIGHT_TOP == nDir )
            bRightToLeft = 1;
        else
            bRightToLeft = 0;
    }
}

void SwFrm::CheckDirection( BOOL bVert )
{
    if( bVert )
    {
        if( !IsHeaderFrm() && !IsFooterFrm() )
        {
            bDerivedVert = 1;
            SetDirFlags( bVert );
        }
    }
    else
    {
        bDerivedR2L = 1;
        SetDirFlags( bVert );
    }
}

void SwSectionFrm::CheckDirection( BOOL bVert )
{
    const SwFrmFmt* pFmt = GetFmt();
    if( pFmt )
        CheckDir(((SvxFrameDirectionItem&)pFmt->GetAttr(RES_FRAMEDIR)).GetValue(),
                    bVert, sal_True, pFmt->GetDoc()->IsBrowseMode() );
    else
        SwFrm::CheckDirection( bVert );
}

void SwFlyFrm::CheckDirection( BOOL bVert )
{
    const SwFrmFmt* pFmt = GetFmt();
    if( pFmt )
        CheckDir(((SvxFrameDirectionItem&)pFmt->GetAttr(RES_FRAMEDIR)).GetValue(),
                    bVert, sal_False, pFmt->GetDoc()->IsBrowseMode() );
    else
        SwFrm::CheckDirection( bVert );
}

void SwTabFrm::CheckDirection( BOOL bVert )
{
    const SwFrmFmt* pFmt = GetFmt();
    if( pFmt )
        CheckDir(((SvxFrameDirectionItem&)pFmt->GetAttr(RES_FRAMEDIR)).GetValue(),
                    bVert, sal_True, pFmt->GetDoc()->IsBrowseMode() );
    else
        SwFrm::CheckDirection( bVert );
}

void SwCellFrm::CheckDirection( BOOL bVert )
{
    const SwFrmFmt* pFmt = GetFmt();
    if( pFmt )
    {
        CheckDir(((SvxFrameDirectionItem&)pFmt->GetAttr(RES_FRAMEDIR)).GetValue(),
                    bVert, sal_False, pFmt->GetDoc()->IsBrowseMode() );
    }
    else
        SwFrm::CheckDirection( bVert );
}

void SwTxtFrm::CheckDirection( BOOL bVert )
{
    CheckDir( GetTxtNode()->GetSwAttrSet().GetFrmDir().GetValue(), bVert,
              sal_True, GetTxtNode()->GetDoc()->IsBrowseMode() );
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

        case RES_HEADER_FOOTER_EAT_SPACING:
            rInvFlags |= 0x03;
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

        case RES_ROW_SPLIT:
        {
            if ( IsRowFrm() )
            {
                BOOL bInFollowFlowRow = 0 != IsInFollowFlowRow();
                if ( bInFollowFlowRow || 0 != IsInSplitTableRow() )
                {
                    SwTabFrm* pTab = FindTabFrm();
                    if ( bInFollowFlowRow )
                        pTab = pTab->FindMaster();
                    pTab->SetRemoveFollowFlowLinePending( TRUE );
                }
            }
            break;
        }
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
#if (OSL_DEBUG_LEVEL > 1) && !defined(PRODUCT)
    static USHORT nStop = 0;
    if ( nStop == GetFrmId() )
    {
        int bla = 5;
    }
#endif

    if ( !pPage )
    {
        pPage = FindPageFrm();
        // --> OD 2004-07-02 #i28701# - for at-character and as-character
        // anchored Writer fly frames additionally invalidate also page frame
        // its 'anchor character' is on.
        if ( pPage && pPage->GetUpper() && IsFlyFrm() )
        {
            const SwFlyFrm* pFlyFrm = static_cast<const SwFlyFrm*>(this);
            if ( pFlyFrm->IsAutoPos() || pFlyFrm->IsFlyInCntFrm() )
            {
                const SwPageFrm* pPageFrmOfAnchor =
                        &(const_cast<SwFlyFrm*>(pFlyFrm)->GetPageFrmOfAnchor());
                if ( pPageFrmOfAnchor != pPage )
                {
                    InvalidatePage( pPageFrmOfAnchor );
                }
            }
        }
        // <--
    }

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
                            pFly->GetAnchorFrm()->InvalidatePage();
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
            {
                if ( !pFly->IsLocked() )
                {
                    if ( pFly->IsFlyInCntFrm() )
                    {
                        pPage->InvalidateFlyInCnt();
                        ((SwFlyInCntFrm*)pFly)->InvalidateLayout();
                        pFly->GetAnchorFrm()->InvalidatePage();
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
    bFixSize = TRUE;
    const Size aOldSize( Frm().SSize() );
    if ( aNewSize == aOldSize )
        return;

    if ( GetUpper() )
    {
        SWRECTFN2( this )
        SwRect aNew( Point(0,0), aNewSize );
        (aFrm.*fnRect->fnSetWidth)( (aNew.*fnRect->fnGetWidth)() );
        long nNew = (aNew.*fnRect->fnGetHeight)();
        long nDiff = nNew - (aFrm.*fnRect->fnGetHeight)();
        if( nDiff )
        {
            if ( GetUpper()->IsFtnBossFrm() && HasFixSize() &&
                 NA_GROW_SHRINK !=
                 ((SwFtnBossFrm*)GetUpper())->NeighbourhoodAdjustment( this ) )
            {
                (aFrm.*fnRect->fnSetHeight)( nNew );
                SwTwips nReal = ((SwLayoutFrm*)this)->AdjustNeighbourhood(nDiff);
                if ( nReal != nDiff )
                    (aFrm.*fnRect->fnSetHeight)( nNew - nDiff + nReal );
            }
            else
            {
                // OD 24.10.2002 #97265# - grow/shrink not for neighbour frames
                // NOTE: neighbour frames are cell and column frames.
                if ( !bNeighb )
                {
                    if ( nDiff > 0 )
                        Grow( nDiff );
                    else
                        Shrink( -nDiff );

                    if ( GetUpper() && (aFrm.*fnRect->fnGetHeight)() != nNew )
                        GetUpper()->_InvalidateSize();
                }

                // Auch wenn das Grow/Shrink noch nicht die gewuenschte Breite eingestellt hat,
                // wie z.B. beim Aufruf durch ChgColumns, um die Spaltenbreiten einzustellen,
                // wird die Breite jetzt gesetzt.
                (aFrm.*fnRect->fnSetHeight)( nNew );
            }
        }
    }
    else
        aFrm.SSize( aNewSize );

    if ( Frm().SSize() != aOldSize )
    {
        SwPageFrm *pPage = FindPageFrm();
        if ( GetNext() )
        {
            GetNext()->_InvalidatePos();
            GetNext()->InvalidatePage( pPage );
        }
        if( IsLayoutFrm() )
        {
            if( IsRightToLeft() )
                _InvalidatePos();
            if( ((SwLayoutFrm*)this)->Lower() )
                ((SwLayoutFrm*)this)->Lower()->_InvalidateSize();
        }
        _InvalidatePrt();
        _InvalidateSize();
        InvalidatePage( pPage );
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

#ifdef ACCESSIBLE_LAYOUT
    // inform accessibility API
    if ( IsInTab() )
    {
        SwTabFrm* pTableFrm = FindTabFrm();
        if( pTableFrm != NULL  &&
            pTableFrm->IsAccessibleFrm()  &&
            pTableFrm->GetFmt() != NULL )
        {
            SwRootFrm *pRootFrm = pTableFrm->FindRootFrm();
            if( pRootFrm != NULL &&
                pRootFrm->IsAnyShellAccessible() )
            {
                ViewShell* pShell = pRootFrm->GetCurrShell();
                if( pShell != NULL )
                    pShell->Imp()->DisposeAccessibleFrm( pTableFrm, sal_True );
            }
        }
    }
#endif

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
    ASSERT( !pSibling || pSibling->IsFlowFrm(),
            "<SwCntntFrm::Paste(..)> - sibling not of expected type." )

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
        pParent->Grow( Frm().Height() PHEIGHT );

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
            // OD 18.02.2003 #104989# - force complete paint of previous frame,
            // if frame is inserted at the end of a section frame, in order to
            // get subsidiary lines repainted for the section.
            if ( pParent->IsSctFrm() && !GetNext() )
            {
                // force complete paint of previous frame, if new inserted frame
                // in the section is the last one.
                GetPrev()->SetCompletePaint();
            }
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
        // --> OD 2004-07-15 #i26250# - invalidate printing area of previous
        // table frame.
        else if ( pFrm && pFrm->IsTabFrm() )
        {
            pFrm->InvalidatePrt();
        }
        // <--
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
            || ( pUp->IsInSct() &&
                // -->  FME 2004-06-03 #i29438#
                // We have to consider the case that the section may be "empty"
                // except from a temporary empty table frame.
                // This can happen due to the new cell split feature.
                !pUp->IsCellFrm() &&
                // <--
                 !(pSct = pUp->FindSctFrm())->ContainsCntnt() ) ) )
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
        else
        {
            SWRECTFN( this )
            long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
            if( nFrmHeight )
                pUp->Shrink( nFrmHeight );
        }
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

    // OD 24.10.2002 #103517# - correct setting of variable <fnRect>
    // <fnRect> is used for the following:
    // (1) To invalidate the frame's size, if its size, which has to be the
    //      same as its upper/parent, differs from its upper's/parent's.
    // (2) To adjust/grow the frame's upper/parent, if it has a dimension in its
    //      size, which is not determined by its upper/parent.
    // Which size is which depends on the frame type and the layout direction
    // (vertical or horizontal).
    // There are the following cases:
    // (A) Header and footer frames both in vertical and in horizontal layout
    //      have to size the width to the upper/parent. A dimension in the height
    //      has to cause a adjustment/grow of the upper/parent.
    //      --> <fnRect> = fnRectHori
    // (B) Cell and column frames in vertical layout, the width has to be the
    //          same as upper/parent and a dimension in height causes adjustment/grow
    //          of the upper/parent.
    //          --> <fnRect> = fnRectHori
    //      in horizontal layout the other way around
    //          --> <fnRect> = fnRectVert
    // (C) Other frames in vertical layout, the height has to be the
    //          same as upper/parent and a dimension in width causes adjustment/grow
    //          of the upper/parent.
    //          --> <fnRect> = fnRectVert
    //      in horizontal layout the other way around
    //          --> <fnRect> = fnRectHori
    //SwRectFn fnRect = IsVertical() ? fnRectHori : fnRectVert;
    SwRectFn fnRect;
    if ( IsHeaderFrm() || IsFooterFrm() )
        fnRect = fnRectHori;
    else if ( IsCellFrm() || IsColumnFrm() )
        fnRect = GetUpper()->IsVertical() ? fnRectHori : fnRectVert;
    else
        fnRect = GetUpper()->IsVertical() ? fnRectVert : fnRectHori;

    if( (Frm().*fnRect->fnGetWidth)() != (pParent->Prt().*fnRect->fnGetWidth)())
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

    if( (Frm().*fnRect->fnGetHeight)() )
    {
        // AdjustNeighbourhood wird jetzt auch in Spalten aufgerufen,
        // die sich nicht in Rahmen befinden
        BYTE nAdjust = GetUpper()->IsFtnBossFrm() ?
                ((SwFtnBossFrm*)GetUpper())->NeighbourhoodAdjustment( this )
                : NA_GROW_SHRINK;
        SwTwips nGrow = (Frm().*fnRect->fnGetHeight)();
        if( NA_ONLY_ADJUST == nAdjust )
            AdjustNeighbourhood( nGrow );
        else
        {
            SwTwips nReal = 0;
            if( NA_ADJUST_GROW == nAdjust )
                nReal = AdjustNeighbourhood( nGrow );
            if( nReal < nGrow )
                nReal += pParent->Grow( nGrow - nReal );
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

    SWRECTFN( this )
    SwTwips nShrink = (Frm().*fnRect->fnGetHeight)();

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
                    SwTwips nOldHeight = (Frm().*fnRect->fnGetHeight)();
                    (Frm().*fnRect->fnSetHeight)( 0 );
                    nReal += pUp->Shrink( nShrink - nReal );
                    (Frm().*fnRect->fnSetHeight)( nOldHeight );
                }
                if( NA_GROW_ADJUST == nAdjust && nReal < nShrink )
                    AdjustNeighbourhood( nReal - nShrink );
            }
            Remove();
        }
        else
        {
            Remove();
            pUp->Shrink( nShrink );
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
SwTwips SwFrm::Grow( SwTwips nDist, BOOL bTst, BOOL bInfo )
{
    ASSERT( nDist >= 0, "Negatives Wachstum?" );

    PROTOCOL_ENTER( this, bTst ? PROT_GROW_TST : PROT_GROW, 0, &nDist )

    if ( nDist )
    {
        SWRECTFN( this )

        SwTwips nPrtHeight = (Prt().*fnRect->fnGetHeight)();
        if( nPrtHeight > 0 && nDist > (LONG_MAX - nPrtHeight) )
            nDist = LONG_MAX - nPrtHeight;

        if ( IsFlyFrm() )
            return ((SwFlyFrm*)this)->_Grow( nDist, bTst );
        else if( IsSctFrm() )
            return ((SwSectionFrm*)this)->_Grow( nDist, bTst );
        else
        {
            if ( IsCellFrm() )
            {
                SwTabFrm* pTab = FindTabFrm();
                if ( ( 0 != pTab->IsVertical() ) != ( 0 != IsVertical() ) )
                    return 0;;
            }

            const SwTwips nReal = GrowFrm( nDist, bTst, bInfo );
            if( !bTst )
            {
                nPrtHeight = (Prt().*fnRect->fnGetHeight)();
                (Prt().*fnRect->fnSetHeight)( nPrtHeight +
                        ( IsCntntFrm() ? nDist : nReal ) );
            }
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
SwTwips SwFrm::Shrink( SwTwips nDist, BOOL bTst, BOOL bInfo )
{
    ASSERT( nDist >= 0, "Negative Verkleinerung?" );

    PROTOCOL_ENTER( this, bTst ? PROT_SHRINK_TST : PROT_SHRINK, 0, &nDist )

    if ( nDist )
    {
        if ( IsFlyFrm() )
            return ((SwFlyFrm*)this)->_Shrink( nDist, bTst );
        else if( IsSctFrm() )
            return ((SwSectionFrm*)this)->_Shrink( nDist, bTst );
        else
        {
            if ( IsCellFrm() )
            {
                SwTabFrm* pTab = FindTabFrm();
                if ( ( 0 != pTab->IsVertical() ) != ( 0 != IsVertical() ) )
                    return 0;;
            }

            SWRECTFN( this )
            SwTwips nReal = (Frm().*fnRect->fnGetHeight)();
            ShrinkFrm( nDist, bTst, bInfo );
            nReal -= (Frm().*fnRect->fnGetHeight)();
            if( !bTst )
            {
                SwTwips nPrtHeight = (Prt().*fnRect->fnGetHeight)();
                (Prt().*fnRect->fnSetHeight)( nPrtHeight -
                        ( IsCntntFrm() ? nDist : nReal ) );
            }
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
    SWRECTFN( this )

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
                        nMinH += (pFtn->Frm().*fnRect->fnGetHeight)();
                        bFtn = TRUE;
                    }
                    pFtn = (SwFtnFrm*)pFtn->GetNext();
                }
                if( bFtn )
                    nMinH += (pCont->Prt().*fnRect->fnGetTop)();
                nReal = (pCont->Frm().*fnRect->fnGetHeight)() - nMinH;
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
        nReal = (pFrm->Frm().*fnRect->fnGetHeight)();
        if( nReal > nDiff )
            nReal = nDiff;
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
                pFrm->GetNext() && pFrm->GetNext()->IsFtnContFrm()
                && ( pFrm->GetNext()->IsVertical() == IsVertical() )
                )
            {
                //Wenn der Body nicht genuegend her gibt, kann ich noch mal
                //schauen ob es eine Fussnote gibt, falls ja kann dieser
                //entsprechend viel gemopst werden.
                const SwTwips nAddMax = (pFrm->GetNext()->Frm().*fnRect->
                                        fnGetHeight)();
                nAdd = nDiff - nReal;
                if ( nAdd > nAddMax )
                    nAdd = nAddMax;
                if ( !bTst )
                {
                    (pFrm->GetNext()->Frm().*fnRect->fnSetHeight)(nAddMax-nAdd);
                    if( bVert && !bRev )
                        pFrm->GetNext()->Frm().Pos().X() += nAdd;
                    pFrm->GetNext()->InvalidatePrt();
                    if ( pFrm->GetNext()->GetNext() )
                        pFrm->GetNext()->GetNext()->_InvalidatePos();
                }
            }
        }
    }

    if ( !bTst && nReal )
    {
        SwTwips nTmp = (pFrm->Frm().*fnRect->fnGetHeight)();
        (pFrm->Frm().*fnRect->fnSetHeight)( nTmp - nReal );
        if( bVert && !bRev )
            pFrm->Frm().Pos().X() += nReal;
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
            const SwSortedObjs &rObjs = *pBoss->GetDrawObjs();
            ASSERT( pBoss->IsPageFrm(), "Header/Footer out of page?" );
            SwPageFrm *pPage = (SwPageFrm*)pBoss;
            for ( USHORT i = 0; i < rObjs.Count(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
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
/** method to perform additional actions on an invalidation

    OD 2004-05-19 #i28701#

    @author OD
*/
void SwFrm::_ActionOnInvalidation( const InvalidationType _nInvalid )
{
    // default behaviour is to perform no additional action
}

/** method to determine, if an invalidation is allowed.

    OD 2004-05-19 #i28701#

    @author OD
*/
bool SwFrm::_InvalidationAllowed( const InvalidationType _nInvalid ) const
{
    // default behaviour is to allow invalidation
    return true;
}

void SwFrm::ImplInvalidateSize()
{
    if ( _InvalidationAllowed( INVALID_SIZE ) )
    {
        bValidSize = FALSE;
        if ( IsFlyFrm() )
            ((SwFlyFrm*)this)->_Invalidate();
        else
            InvalidatePage();

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_SIZE );
    }
}

void SwFrm::ImplInvalidatePrt()
{
    if ( _InvalidationAllowed( INVALID_PRTAREA ) )
    {
        bValidPrtArea = FALSE;
        if ( IsFlyFrm() )
            ((SwFlyFrm*)this)->_Invalidate();
        else
            InvalidatePage();

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_PRTAREA );
    }
}

void SwFrm::ImplInvalidatePos()
{
    if ( _InvalidationAllowed( INVALID_POS ) )
    {
        bValidPos = FALSE;
        if ( IsFlyFrm() )
        {
            ((SwFlyFrm*)this)->_Invalidate();
        }
        else
        {
            InvalidatePage();
        }

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_POS );
    }
}

void SwFrm::ImplInvalidateLineNum()
{
    if ( _InvalidationAllowed( INVALID_LINENUM ) )
    {
        bValidLineNum = FALSE;
        ASSERT( IsTxtFrm(), "line numbers are implemented for text only" );
        InvalidatePage();

        // OD 2004-05-19 #i28701#
        _ActionOnInvalidation( INVALID_LINENUM );
    }
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
    if ( ATT_VAR_SIZE == rFmtSize.GetHeightSizeType() ||
         ATT_MIN_SIZE == rFmtSize.GetHeightSizeType())
    {
        bFixSize = FALSE;
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
    else if ( rFmtSize.GetHeightSizeType() == ATT_FIX_SIZE )
    {
        if( IsVertical() )
            ChgSize( Size( rFmtSize.GetWidth(), Frm().Height()));
        else
            ChgSize( Size( Frm().Width(), rFmtSize.GetHeight()));
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
SwTwips SwCntntFrm::GrowFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
{
    SWRECTFN( this )

    SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
    if( nFrmHeight > 0 &&
         nDist > (LONG_MAX - nFrmHeight ) )
        nDist = LONG_MAX - nFrmHeight;

    const FASTBOOL bBrowse = GetUpper()->GetFmt()->GetDoc()->IsBrowseMode();
    const USHORT nType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse mit Body
    if( !(GetUpper()->GetType() & nType) && GetUpper()->HasFixSize() )
    {
        if ( !bTst )
        {
            (Frm().*fnRect->fnSetHeight)( nFrmHeight + nDist );
            if( IsVertical() && !IsReverse() )
                Frm().Pos().X() -= nDist;
            if ( GetNext() )
            {
                GetNext()->InvalidatePos();
            }
            // --> OD 2004-07-05 #i28701# - Due to the new object positioning the
            // frame on the next page/column can flow backward (e.g. it was moved forward
            // due to the positioning of its objects ). Thus, invalivate this next frame,
            // if document compatibility option 'Consider wrapping style influence on
            // object positioning' is ON.
            else if ( GetUpper()->GetFmt()->GetDoc()->ConsiderWrapOnObjPos() )
            {
                InvalidateNextPos();
            }
            // <--
        }
        return 0;
    }

    SwTwips nReal = (GetUpper()->Prt().*fnRect->fnGetHeight)();
    SwFrm *pFrm = GetUpper()->Lower();
    while( pFrm && nReal > 0 )
    {   nReal -= (pFrm->Frm().*fnRect->fnGetHeight)();
        pFrm = pFrm->GetNext();
    }

    if ( !bTst )
    {
        //Cntnts werden immer auf den gewuenschten Wert gebracht.
        long nOld = (Frm().*fnRect->fnGetHeight)();
        (Frm().*fnRect->fnSetHeight)( nOld + nDist );
        if( IsVertical() && !IsReverse() )
            Frm().Pos().X() -= nDist;
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
    {
        if( GetUpper() )
        {
            if( bTst || !GetUpper()->IsFooterFrm() )
                nReal = GetUpper()->Grow( nDist - (nReal > 0 ? nReal : 0),
                                          bTst, bInfo );
            else
            {
                nReal = 0;
                GetUpper()->InvalidateSize();
            }
        }
        else
            nReal = 0;
    }
    else
        nReal = nDist;

    // --> OD 2004-07-05 #i28701# - Due to the new object positioning the
    // frame on the next page/column can flow backward (e.g. it was moved forward
    // due to the positioning of its objects ). Thus, invalivate this next frame,
    // if document compatibility option 'Consider wrapping style influence on
    // object positioning' is ON.
    if ( !bTst )
    {
        if ( GetNext() )
        {
            GetNext()->InvalidatePos();
        }
        else if ( GetUpper()->GetFmt()->GetDoc()->ConsiderWrapOnObjPos() )
        {
            InvalidateNextPos();
        }
    }
    // <--

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
SwTwips SwCntntFrm::ShrinkFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
{
    SWRECTFN( this )
    ASSERT( nDist >= 0, "nDist < 0" );
    ASSERT( nDist <= (Frm().*fnRect->fnGetHeight)(),
            "nDist > als aktuelle Grosse." );

    if ( !bTst )
    {
        SwTwips nRstHeight;
        if( GetUpper() )
            nRstHeight = (Frm().*fnRect->fnBottomDist)
                         ( (GetUpper()->*fnRect->fnGetPrtBottom)() );
        else
            nRstHeight = 0;
        if( nRstHeight < 0 )
            nRstHeight = nDist + nRstHeight;
        else
            nRstHeight = nDist;
        (Frm().*fnRect->fnSetHeight)( (Frm().*fnRect->fnGetHeight)() - nDist );
        if( IsVertical() )
            Frm().Pos().X() += nDist;
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

    SwTwips nReal;
    if( GetUpper() && nDist > 0 )
    {
        if( bTst || !GetUpper()->IsFooterFrm() )
            nReal = GetUpper()->Shrink( nDist, bTst, bInfo );
        else
        {
            nReal = 0;

            // #108745# Sorry, dear old footer friend, I'm not gonna invalidate you,
            // if there are any objects anchored inside your content, which
            // overlap with the shrinking frame.
            // This may lead to a footer frame that is too big, but this is better
            // than looping.
            // #109722# : The fix for #108745# was too strict.

            bool bInvalidate = true;
            const SwRect aRect( Frm() );
            const SwPageFrm* pPage = FindPageFrm();
            const SwSortedObjs* pSorted;
            if( pPage && ( pSorted = pPage->GetSortedObjs() ) )
            {
                for ( USHORT i = 0; i < pSorted->Count(); ++i )
                {
                    const SwAnchoredObject* pAnchoredObj = (*pSorted)[i];
                    const SwRect aBound( pAnchoredObj->GetObjRectWithSpaces() );

                    if( aBound.Left() > aRect.Right() )
                        continue;

                    if( aBound.IsOver( aRect ) )
                    {
                        const SwFrmFmt& rFmt = pAnchoredObj->GetFrmFmt();
                        if( SURROUND_THROUGHT != rFmt.GetSurround().GetSurround() )
                        {
                            const SwFrm* pAnchor = pAnchoredObj->GetAnchorFrm();
                            if ( pAnchor && pAnchor->FindFooterOrHeader() == GetUpper() )
                            {
                                bInvalidate = false;
                                break;
                            }
                        }
                    }
                }
            }

            if ( bInvalidate )
                GetUpper()->InvalidateSize();
        }
    }
    else
        nReal = 0;

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
        SwFrm* pNextFrm = GetIndNext();
        if ( pNextFrm && nInvFlags & 0x10)
        {
            pNextFrm->_InvalidatePrt();
            pNextFrm->InvalidatePage( pPage );
        }
        if ( pNextFrm && nInvFlags & 0x80 )
        {
            pNextFrm->SetCompletePaint();
        }
        if ( nInvFlags & 0x20 )
        {
            SwFrm* pPrevFrm = GetPrev();
            if ( pPrevFrm )
            {
                pPrevFrm->_InvalidatePrt();
                pPrevFrm->InvalidatePage( pPage );
            }
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
                // OD 2004-02-18 #106629# - correction
                // Invalidation of the printing area of next frame, not only
                // for footnote content.
                if ( !GetIndNext() )
                {
                    SwFrm* pNxt = FindNext();
                    if ( pNxt )
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
                // OD 2004-03-17 #i11860#
                if ( GetIndNext() &&
                     !GetAttrSet()->GetDoc()->IsFormerObjectPositioning() )
                {
                    // OD 2004-07-01 #i28701# - use new method <InvalidateObjs(..)>
                    GetIndNext()->InvalidateObjs( true );
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

        // OD 2004-02-26 #i25029#
        case RES_PARATR_CONNECT_BORDER:
        {
            rInvFlags |= 0x01;
            if ( IsTxtFrm() )
            {
                InvalidateNextPrtArea();
            }
            if ( !GetIndNext() && IsInTab() && IsInSplitTableRow() )
            {
                FindTabFrm()->InvalidateSize();
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
    const SwFmtFrmSize &rFmtSize = pFmt->GetFrmSize();
    if ( rFmtSize.GetHeightSizeType() == ATT_FIX_SIZE )
        BFIXHEIGHT = TRUE;
}

// --> OD 2004-06-29 #i28701#
TYPEINIT1(SwLayoutFrm,SwFrm);
// <--
/*-----------------10.06.99 09:42-------------------
 * SwLayoutFrm::InnerHeight()
 * --------------------------------------------------*/

SwTwips SwLayoutFrm::InnerHeight() const
{
    if( !Lower() )
        return 0;
    SwTwips nRet = 0;
    const SwFrm* pCnt = Lower();
    SWRECTFN( this )
    if( pCnt->IsColumnFrm() || pCnt->IsCellFrm() )
    {
        do
        {
            SwTwips nTmp = ((SwLayoutFrm*)pCnt)->InnerHeight();
            if( pCnt->GetValidPrtAreaFlag() )
                nTmp += (pCnt->Frm().*fnRect->fnGetHeight)() -
                        (pCnt->Prt().*fnRect->fnGetHeight)();
            if( nRet < nTmp )
                nRet = nTmp;
            pCnt = pCnt->GetNext();
        } while ( pCnt );
    }
    else
    {
        do
        {
            nRet += (pCnt->Frm().*fnRect->fnGetHeight)();
            if( pCnt->IsCntntFrm() && ((SwTxtFrm*)pCnt)->IsUndersized() )
                nRet += ((SwTxtFrm*)pCnt)->GetParHeight() -
                        (pCnt->Prt().*fnRect->fnGetHeight)();
            if( pCnt->IsLayoutFrm() && !pCnt->IsTabFrm() )
                nRet += ((SwLayoutFrm*)pCnt)->InnerHeight() -
                        (pCnt->Prt().*fnRect->fnGetHeight)();
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
SwTwips SwLayoutFrm::GrowFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
{
    const FASTBOOL bBrowse = GetFmt()->GetDoc()->IsBrowseMode();
    const USHORT nType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse mit Body
    if( !(GetType() & nType) && HasFixSize() )
        return 0;

    SWRECTFN( this )
    SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
    if ( nFrmHeight > 0 && nDist > (LONG_MAX - nFrmHeight) )
        nDist = LONG_MAX - nFrmHeight;

    SwTwips nMin = 0;
    if ( GetUpper() && !IsCellFrm() )
    {
        SwFrm *pFrm = GetUpper()->Lower();
        while( pFrm )
        {   nMin += (pFrm->Frm().*fnRect->fnGetHeight)();
            pFrm = pFrm->GetNext();
        }
        nMin = (GetUpper()->Prt().*fnRect->fnGetHeight)() - nMin;
        if ( nMin < 0 )
            nMin = 0;
    }

    SwRect aOldFrm( Frm() );
    sal_Bool bMoveAccFrm = sal_False;

    BOOL bChgPos = IsVertical() && !IsReverse();
    if ( !bTst )
    {
        (Frm().*fnRect->fnSetHeight)( nFrmHeight + nDist );
        if( bChgPos )
            Frm().Pos().X() -= nDist;
        bMoveAccFrm = sal_True;
    }

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
                    nGrow += GetUpper()->Grow( nReal - nGrow, bTst, bInfo );
                if( NA_GROW_ADJUST == nAdjust && nGrow < nReal )
                    nReal += AdjustNeighbourhood( nReal - nGrow, bTst );
                if ( IsFtnFrm() && (nGrow != nReal) && GetNext() )
                {
                    //Fussnoten koennen ihre Nachfolger verdraengen.
                    SwTwips nSpace = bTst ? 0 : -nDist;
                    const SwFrm *pFrm = GetUpper()->Lower();
                    do
                    {   nSpace += (pFrm->Frm().*fnRect->fnGetHeight)();
                        pFrm = pFrm->GetNext();
                    } while ( pFrm != GetNext() );
                    nSpace = (GetUpper()->Prt().*fnRect->fnGetHeight)() -nSpace;
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
        if( nReal != nDist && !IsCellFrm() )
        {
            nDist -= nReal;
            (Frm().*fnRect->fnSetHeight)( (Frm().*fnRect->fnGetHeight)()
                                          - nDist );
            if( bChgPos )
                Frm().Pos().X() += nDist;
            bMoveAccFrm = sal_True;
        }

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
                NotifyLowerObjs();

            if( IsCellFrm() )
                InvaPercentLowers( nReal );

            const SvxGraphicPosition ePos = GetFmt()->GetBackground().GetGraphicPos();
            if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
                SetCompletePaint();
        }
    }

    if( bMoveAccFrm && IsAccessibleFrm() )
    {
        SwRootFrm *pRootFrm = FindRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
            pRootFrm->GetCurrShell() )
        {
            pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( this, aOldFrm );
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
SwTwips SwLayoutFrm::ShrinkFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
{
    const FASTBOOL bBrowse = GetFmt()->GetDoc()->IsBrowseMode();
    const USHORT nType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse mit Body
    if( !(GetType() & nType) && HasFixSize() )
        return 0;

    ASSERT( nDist >= 0, "nDist < 0" );
    SWRECTFN( this )
    SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
    if ( nDist > nFrmHeight )
        nDist = nFrmHeight;

    SwTwips nMin = 0;
    BOOL bChgPos = IsVertical() && !IsReverse();
    if ( Lower() )
    {
        if( !Lower()->IsNeighbourFrm() )
        {   const SwFrm *pFrm = Lower();
            const long nTmp = (Prt().*fnRect->fnGetHeight)();
            while( pFrm && nMin < nTmp )
            {   nMin += (pFrm->Frm().*fnRect->fnGetHeight)();
                pFrm = pFrm->GetNext();
            }
        }
    }
    SwTwips nReal = nDist;
    SwTwips nMinDiff = (Prt().*fnRect->fnGetHeight)() - nMin;
    if( nReal > nMinDiff )
        nReal = nMinDiff;
    if( nReal <= 0 )
        return nDist;

    SwRect aOldFrm( Frm() );
    sal_Bool bMoveAccFrm = sal_False;

    SwTwips nRealDist = nReal;
    if ( !bTst )
    {
        (Frm().*fnRect->fnSetHeight)( nFrmHeight - nReal );
        if( bChgPos )
            Frm().Pos().X() += nReal;
        bMoveAccFrm = sal_True;
    }

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
            {
                (Frm().*fnRect->fnSetHeight)( (Frm().*fnRect->fnGetHeight)()
                                            + nRealDist - nReal );
                if( bChgPos )
                    Frm().Pos().X() += nRealDist - nReal;
                ASSERT( !IsAccessibleFrm(), "bMoveAccFrm has to be set!" );
            }
        }
    }
    else if( IsColumnFrm() || IsColBodyFrm() )
    {
        SwTwips nTmp = GetUpper()->Shrink( nReal, bTst, bInfo );
        if ( nTmp != nReal )
        {
            (Frm().*fnRect->fnSetHeight)( (Frm().*fnRect->fnGetHeight)()
                                          + nReal - nTmp );
            if( bChgPos )
                Frm().Pos().X() += nTmp - nReal;
            ASSERT( !IsAccessibleFrm(), "bMoveAccFrm has to be set!" );
            nReal = nTmp;
        }
    }
    else
    {
        SwTwips nShrink = nReal;
        nReal = GetUpper() ? GetUpper()->Shrink( nShrink, bTst, bInfo ) : 0;
        if( ( NA_GROW_ADJUST == nAdjust || NA_ADJUST_GROW == nAdjust )
            && nReal < nShrink )
            AdjustNeighbourhood( nReal - nShrink );
    }

    if( bMoveAccFrm && IsAccessibleFrm() )
    {
        SwRootFrm *pRootFrm = FindRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
            pRootFrm->GetCurrShell() )
        {
            pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( this, aOldFrm );
        }
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
            NotifyLowerObjs();

        if( IsCellFrm() )
            InvaPercentLowers( nReal );

        SwCntntFrm *pCnt;
        if( IsFtnFrm() && !((SwFtnFrm*)this)->GetAttr()->GetFtn().IsEndNote() &&
            ( GetFmt()->GetDoc()->GetFtnInfo().ePos != FTNPOS_CHAPTER ||
              ( IsInSct() && FindSctFrm()->IsFtnAtEnd() ) ) &&
              0 != (pCnt = ((SwFtnFrm*)this)->GetRefFromAttr() ) )
        {
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
    // no change of lower properties for root frame or if no lower exists.
    if ( IsRootFrm() || !Lower() )
        return;

    // declare and init <SwFrm* pLowerFrm> with first lower
    SwFrm *pLowerFrm = Lower();

    // declare and init const booleans <bHeightChgd> and <bWidthChg>
    const bool bHeightChgd = rOldSize.Height() != Prt().Height();
    const bool bWidthChgd  = rOldSize.Width()  != Prt().Width();

    // declare and init variables <bVert>, <bRev> and <fnRect>
    SWRECTFN( this )

    // This shortcut basically tries to handle only lower frames that
    // are affected by the size change. Otherwise much more lower frames
    // are invalidated.
    if ( !( bVert ? bHeightChgd : bWidthChgd ) &&
         ! Lower()->IsColumnFrm() &&
           ( ( IsBodyFrm() && IsInDocBody() && ( !IsInSct() || !FindSctFrm()->IsColLocked() ) ) ||
                // --> FME 2004-07-21 #i10826# Section frames without columns should not
                // invalidate all lowers!
               IsSctFrm() ) )
               // <--
    {
        // Determine page frame the body frame belongs to.
        SwPageFrm *pPage = FindPageFrm();
        // Determine last lower by traveling through them using <GetNext()>.
        // During travel check each section frame, if it will be sized to
        // maximum. If Yes, invalidate size of section frame and set
        // corresponding flags at the page.
        do
        {
            if( pLowerFrm->IsSctFrm() &&((SwSectionFrm*)pLowerFrm)->_ToMaximize() )
            {
                pLowerFrm->_InvalidateSize();
                pLowerFrm->InvalidatePage( pPage );
            }
            if( pLowerFrm->GetNext() )
                pLowerFrm = pLowerFrm->GetNext();
            else
                break;
        } while( TRUE );
        // If found last lower is a section frame containing no section
        // (section frame isn't valid and will be deleted in the future),
        // travel backwards.
        while( pLowerFrm->IsSctFrm() && !((SwSectionFrm*)pLowerFrm)->GetSection() &&
               pLowerFrm->GetPrev() )
            pLowerFrm = pLowerFrm->GetPrev();
        // If found last lower is a section frame, set <pLowerFrm> to its last
        // content, if the section frame is valid and is not sized to maximum.
        // Otherwise set <pLowerFrm> to NULL - In this case body frame only
        //      contains invalid section frames.
        if( pLowerFrm->IsSctFrm() )
            pLowerFrm = ((SwSectionFrm*)pLowerFrm)->GetSection() &&
                   !((SwSectionFrm*)pLowerFrm)->ToMaximize( FALSE ) ?
                   ((SwSectionFrm*)pLowerFrm)->FindLastCntnt() : NULL;

        // continue with found last lower, probably the last content of a section
        if ( pLowerFrm )
        {
            // If <pLowerFrm> is in a table frame, set <pLowerFrm> to this table
            // frame and continue.
            if ( pLowerFrm->IsInTab() )
            {
                // OD 28.10.2002 #97265# - safeguard for setting <pLowerFrm> to
                // its table frame - check, if the table frame is also a lower
                // of the body frame, in order to assure that <pLowerFrm> is not
                // set to a frame, which is an *upper* of the body frame.
                SwFrm* pTableFrm = pLowerFrm->FindTabFrm();
                if ( IsAnLower( pTableFrm ) )
                {
                    pLowerFrm = pTableFrm;
                }
            }
            // Check, if variable size of body frame has grown
            // OD 28.10.2002 #97265# - correct check, if variable size has grown.
            //SwTwips nOldHeight = bVert ? rOldSize.Height() : rOldSize.Width();
            SwTwips nOldHeight = bVert ? rOldSize.Width() : rOldSize.Height();
            if( nOldHeight < (Prt().*fnRect->fnGetHeight)() )
            {
                // If variable size of body frame has grown, only found last lower
                // and the position of the its next have to be invalidated.
                pLowerFrm->_InvalidateAll();
                pLowerFrm->InvalidatePage( pPage );
                if( !pLowerFrm->IsFlowFrm() ||
                    !SwFlowFrm::CastFlowFrm( pLowerFrm )->HasFollow() )
                    pLowerFrm->InvalidateNextPos( TRUE );
                if ( pLowerFrm->IsTxtFrm() )
                    ((SwCntntFrm*)pLowerFrm)->Prepare( PREP_ADJUST_FRM );
                if ( pLowerFrm->IsInSct() )
                {
                    pLowerFrm = pLowerFrm->FindSctFrm();
                    if( IsAnLower( pLowerFrm ) )
                    {
                        pLowerFrm->_InvalidateSize();
                        pLowerFrm->InvalidatePage( pPage );
                    }
                }
            }
            else
            {
                // variable size of body frame has shrinked. Thus, invalidate
                // all lowers not matching the new body size and the dedicated
                // new last lower.
                if( bVert )
                {
                    SwTwips nBot = Frm().Left() + Prt().Left();
                    while ( pLowerFrm->GetPrev() && pLowerFrm->Frm().Left() < nBot )
                    {
                        pLowerFrm->_InvalidateAll();
                        pLowerFrm->InvalidatePage( pPage );
                        pLowerFrm = pLowerFrm->GetPrev();
                    }
                }
                else
                {
                    SwTwips nBot = Frm().Top() + Prt().Bottom();
                    while ( pLowerFrm->GetPrev() && pLowerFrm->Frm().Top() > nBot )
                    {
                        pLowerFrm->_InvalidateAll();
                        pLowerFrm->InvalidatePage( pPage );
                        pLowerFrm = pLowerFrm->GetPrev();
                    }
                }
                if ( pLowerFrm )
                {
                    pLowerFrm->_InvalidateSize();
                    pLowerFrm->InvalidatePage( pPage );
                    if ( pLowerFrm->IsTxtFrm() )
                        ((SwCntntFrm*)pLowerFrm)->Prepare( PREP_ADJUST_FRM );
                    if ( pLowerFrm->IsInSct() )
                    {
                        pLowerFrm = pLowerFrm->FindSctFrm();
                        if( IsAnLower( pLowerFrm ) )
                        {
                            pLowerFrm->_InvalidateSize();
                            pLowerFrm->InvalidatePage( pPage );
                        }
                    }
                }
            }
        }
        return;
    } // end of { special case }


    // Invalidate page for content only once.
    bool bInvaPageForCntnt = true;

    // Declare booleans <bFixChgd> and <bVarChgd>, indicating for text frame
    // adjustment, if fixed/variable size has changed.
    bool bFixChgd, bVarChgd;
    if( bVert == pLowerFrm->IsNeighbourFrm() )
    {
        bFixChgd = bWidthChgd;
        bVarChgd = bHeightChgd;
    }
    else
    {
        bFixChgd = bHeightChgd;
        bVarChgd = bWidthChgd;
    }

    // Declare const unsigned short <nFixWidth> and init it this frame types
    // which has fixed width in vertical respectively horizontal layout.
    // In vertical layout these are neighbour frames (cell and column frames),
    //      header frames and footer frames.
    // In horizontal layout these are all frames, which aren't neighbour frames.
    const USHORT nFixWidth = bVert ? (FRM_NEIGHBOUR | FRM_HEADFOOT)
                                   : ~FRM_NEIGHBOUR;

    // Declare const unsigned short <nFixHeight> and init it this frame types
    // which has fixed height in vertical respectively horizontal layout.
    // In vertical layout these are all frames, which aren't neighbour frames,
    //      header frames, footer frames, body frames or foot note container frames.
    // In horizontal layout these are neighbour frames.
    const USHORT nFixHeight= bVert ? ~(FRM_NEIGHBOUR | FRM_HEADFOOT | FRM_BODYFTNC)
                                   : FRM_NEIGHBOUR;

    // Travel through all lowers using <GetNext()>
    while ( pLowerFrm )
    {
        if ( pLowerFrm->IsTxtFrm() )
        {
            // Text frames will only be invalidated - prepare invalidation
            if ( bFixChgd )
                static_cast<SwCntntFrm*>(pLowerFrm)->Prepare( PREP_FIXSIZE_CHG );
            if ( bVarChgd )
                static_cast<SwCntntFrm*>(pLowerFrm)->Prepare( PREP_ADJUST_FRM );
        }
        else
        {
            // If lower isn't a table, row, cell or section frame, adjust its
            // frame size.
            USHORT nType = pLowerFrm->GetType();
            if ( !(nType & (FRM_TAB|FRM_ROW|FRM_CELL|FRM_SECTION)) )
            {
                if ( bWidthChgd )
                {
                    if( nType & nFixWidth )
                    {
                        // Considering previous conditions:
                        // In vertical layout set width of column, header and
                        // footer frames to its upper width.
                        // In horizontal layout set width of header, footer,
                        // foot note container, foot note, body and no-text
                        // frames to its upper width.
                        pLowerFrm->Frm().Width( Prt().Width() );
                    }
                    else if( rOldSize.Width() && !pLowerFrm->IsFtnFrm() )
                    {
                        // Adjust frame width proportional, if lower isn't a
                        // foot note frame and condition <nType & nFixWidth>
                        // isn't true.
                        // Considering previous conditions:
                        // In vertical layout these are foot note container,
                        // body and no-text frames.
                        // In horizontal layout these are column and no-text frames.
                        // OD 24.10.2002 #97265# - <double> calculation
                        // Perform <double> calculation of new width, if
                        // one of the coefficients is greater than 50000
                        SwTwips nNewWidth;
                        if ( (pLowerFrm->Frm().Width() > 50000) ||
                             (Prt().Width() > 50000) )
                        {
                            double nNewWidthTmp =
                                ( double(pLowerFrm->Frm().Width())
                                  * double(Prt().Width()) )
                                / double(rOldSize.Width());
                            nNewWidth = SwTwips(nNewWidthTmp);
                        }
                        else
                        {
                            nNewWidth =
                                (pLowerFrm->Frm().Width() * Prt().Width()) / rOldSize.Width();
                        }
                        pLowerFrm->Frm().Width( nNewWidth );
                    }
                }
                if ( bHeightChgd )
                {
                    if( nType & nFixHeight )
                    {
                        // Considering previous conditions:
                        // In vertical layout set height of foot note and
                        // no-text frames to its upper height.
                        // In horizontal layout set height of column frames
                        // to its upper height.
                        pLowerFrm->Frm().Height( Prt().Height() );
                    }
                    // OD 01.10.2002 #102211#
                    // add conditions <!pLowerFrm->IsHeaderFrm()> and
                    // <!pLowerFrm->IsFooterFrm()> in order to avoid that
                    // the <Grow> of header or footer are overwritten.
                    // NOTE: Height of header/footer frame is determined by contents.
                    else if ( rOldSize.Height() &&
                              !pLowerFrm->IsFtnFrm() &&
                              !pLowerFrm->IsHeaderFrm() &&
                              !pLowerFrm->IsFooterFrm()
                            )
                    {
                        // Adjust frame height proportional, if lower isn't a
                        // foot note, a header or a footer frame and
                        // condition <nType & nFixHeight> isn't true.
                        // Considering previous conditions:
                        // In vertical layout these are column, foot note container,
                        // body and no-text frames.
                        // In horizontal layout these are column, foot note
                        // container, body and no-text frames.

                        // OD 29.10.2002 #97265# - special case for page lowers
                        // The page lowers that have to be adjusted on page height
                        // change are the body frame and the foot note container
                        // frame.
                        // In vertical layout the height of both is directly
                        // adjusted to the page height change.
                        // In horizontal layout the height of the body frame is
                        // directly adjsuted to the page height change and the
                        // foot note frame height isn't touched, because its
                        // determined by its content.
                        // OD 31.03.2003 #108446# - apply special case for page
                        // lowers - see description above - also for section columns.
                        if ( IsPageFrm() ||
                             ( IsColumnFrm() && IsInSct() )
                           )
                        {
                            ASSERT( pLowerFrm->IsBodyFrm() || pLowerFrm->IsFtnContFrm(),
                                    "ChgLowersProp - only for body or foot note container" );
                            if ( pLowerFrm->IsBodyFrm() || pLowerFrm->IsFtnContFrm() )
                            {
                                if ( IsVertical() || pLowerFrm->IsBodyFrm() )
                                {
                                    SwTwips nNewHeight =
                                            pLowerFrm->Frm().Height() +
                                            ( Prt().Height() - rOldSize.Height() );
                                    if ( nNewHeight < 0)
                                    {
                                        // OD 01.04.2003 #108446# - adjust assertion condition and text
                                        ASSERT( !( IsPageFrm() &&
                                                   (pLowerFrm->Frm().Height()>0) &&
                                                   (pLowerFrm->IsValid()) ),
                                                    "ChgLowersProg - negative height for lower.");
                                        nNewHeight = 0;
                                    }
                                    pLowerFrm->Frm().Height( nNewHeight );
                                }
                            }
                        }
                        else
                        {
                            SwTwips nNewHeight;
                            // OD 24.10.2002 #97265# - <double> calculation
                            // Perform <double> calculation of new height, if
                            // one of the coefficients is greater than 50000
                            if ( (pLowerFrm->Frm().Height() > 50000) ||
                                 (Prt().Height() > 50000) )
                            {
                                double nNewHeightTmp =
                                    ( double(pLowerFrm->Frm().Height())
                                      * double(Prt().Height()) )
                                    / double(rOldSize.Height());
                                nNewHeight = SwTwips(nNewHeightTmp);
                            }
                            else
                            {
                                nNewHeight = ( pLowerFrm->Frm().Height()
                                             * Prt().Height() ) / rOldSize.Height();
                            }
                            if( !pLowerFrm->GetNext() )
                            {
                                SwTwips nSum = Prt().Height();
                                SwFrm* pTmp = Lower();
                                while( pTmp->GetNext() )
                                {
                                    if( !pTmp->IsFtnContFrm() || !pTmp->IsVertical() )
                                        nSum -= pTmp->Frm().Height();
                                    pTmp = pTmp->GetNext();
                                }
                                if( nSum - nNewHeight == 1 &&
                                    nSum == pLowerFrm->Frm().Height() )
                                    nNewHeight = nSum;
                            }
                            pLowerFrm->Frm().Height( nNewHeight );
                        }
                    }
                }
            }
        } // end of else { NOT text frame }

        pLowerFrm->_InvalidateAll();
        if ( bInvaPageForCntnt && pLowerFrm->IsCntntFrm() )
        {
            pLowerFrm->InvalidatePage();
            bInvaPageForCntnt = false;
        }

        if ( !pLowerFrm->GetNext() && pLowerFrm->IsRetoucheFrm() )
        {
            //Wenn ein Wachstum stattgefunden hat, und die untergeordneten
            //zur Retouche faehig sind (derzeit Tab, Section und Cntnt), so
            //trigger ich sie an.
            if ( rOldSize.Height() < Prt().SSize().Height() ||
                 rOldSize.Width() < Prt().SSize().Width() )
                pLowerFrm->SetRetouche();
        }
        pLowerFrm = pLowerFrm->GetNext();
    }

    // Finally adjust the columns if width is set to auto
    // Possible optimisation: execute this code earlier in this function and
    // return???
    if ( ( bVert && bHeightChgd || ! bVert && bWidthChgd ) &&
           Lower()->IsColumnFrm() )
    {
        // get column attribute
        const SwFmtCol* pColAttr = NULL;
        if ( IsPageBodyFrm() )
        {
            ASSERT( GetUpper()->IsPageFrm(), "Upper is not page frame" )
            pColAttr = &GetUpper()->GetFmt()->GetCol();
        }
        else
        {
            ASSERT( IsFlyFrm() || IsSctFrm(), "Columns not in fly or section" )
            pColAttr = &GetFmt()->GetCol();
        }

        if ( pColAttr->IsOrtho() && pColAttr->GetNumCols() > 1 )
            AdjustColumns( pColAttr, sal_False );
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

    const USHORT nLeft = (USHORT)pAttrs->CalcLeft( this );
    const USHORT nUpper = pAttrs->CalcTop();

    const USHORT nRight = (USHORT)((SwBorderAttrs*)pAttrs)->CalcRight( this );
    const USHORT nLower = pAttrs->CalcBottom();
    BOOL bVert = IsVertical() && !IsPageFrm();
    SwRectFn fnRect = bVert ? fnRectVert : fnRectHori;
    if ( !bValidPrtArea )
    {
        bValidPrtArea = TRUE;
        (this->*fnRect->fnSetXMargins)( nLeft, nRight );
        (this->*fnRect->fnSetYMargins)( nUpper, nLower );
    }

    if ( !bValidSize )
    {
        if ( !HasFixSize() )
        {
            const SwTwips nBorder = nUpper + nLower;
            const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
            SwTwips nMinHeight = rSz.GetHeightSizeType() == ATT_MIN_SIZE ? rSz.GetHeight() : 0;
            do
            {   bValidSize = TRUE;

                //Die Groesse in der VarSize wird durch den Inhalt plus den
                //Raendern bestimmt.
                SwTwips nRemaining = 0;
                SwFrm *pFrm = Lower();
                while ( pFrm )
                {   nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)();
                    if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
                    // Dieser TxtFrm waere gern ein bisschen groesser
                        nRemaining += ((SwTxtFrm*)pFrm)->GetParHeight()
                                      - (pFrm->Prt().*fnRect->fnGetHeight)();
                    else if( pFrm->IsSctFrm() && ((SwSectionFrm*)pFrm)->IsUndersized() )
                        nRemaining += ((SwSectionFrm*)pFrm)->Undersize();
                    pFrm = pFrm->GetNext();
                }
                nRemaining += nBorder;
                nRemaining = Max( nRemaining, nMinHeight );
                const SwTwips nDiff = nRemaining-(Frm().*fnRect->fnGetHeight)();
                const long nOldLeft = (Frm().*fnRect->fnGetLeft)();
                const long nOldTop = (Frm().*fnRect->fnGetTop)();
                if ( nDiff )
                {
                    if ( nDiff > 0 )
                        Grow( nDiff );
                    else
                        Shrink( -nDiff );
                    //Schnell auf dem kurzen Dienstweg die Position updaten.
                    MakePos();
                }
                //Unterkante des Uppers nicht ueberschreiten.
                if ( GetUpper() && (Frm().*fnRect->fnGetHeight)() )
                {
                    const SwTwips nLimit = (GetUpper()->*fnRect->fnGetPrtBottom)();
                    if( (this->*fnRect->fnSetLimit)( nLimit ) &&
                        nOldLeft == (Frm().*fnRect->fnGetLeft)() &&
                        nOldTop  == (Frm().*fnRect->fnGetTop)() )
                        bValidSize = bValidPrtArea = TRUE;
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
static void InvaPercentFlys( SwFrm *pFrm, SwTwips nDiff )
{
    ASSERT( pFrm->GetDrawObjs(), "Can't find any Objects" );
    for ( USHORT i = 0; i < pFrm->GetDrawObjs()->Count(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
        if ( pAnchoredObj->ISA(SwFlyFrm) )
        {
            SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
            const SwFmtFrmSize &rSz = pFly->GetFmt()->GetFrmSize();
            if ( rSz.GetWidthPercent() || rSz.GetHeightPercent() )
            {
                BOOL bNotify = TRUE;
                // If we've a fly with more than 90% relative height...
                if( rSz.GetHeightPercent() > 90 && pFly->GetAnchorFrm() &&
                    rSz.GetHeightPercent() != 0xFF && nDiff )
                {
                    const SwFrm *pRel = pFly->IsFlyLayFrm() ? pFly->GetAnchorFrm():
                                        pFly->GetAnchorFrm()->GetUpper();
                    // ... and we have already more than 90% height and we
                    // not allow the text to go through...
                    // then a notifycation could cause an endless loop, e.g.
                    // 100% height and no text wrap inside a cell of a table.
                    if( pFly->Frm().Height()*10 >
                        ( nDiff + pRel->Prt().Height() )*9 &&
                        pFly->GetFmt()->GetSurround().GetSurround() !=
                        SURROUND_THROUGHT )
                       bNotify = FALSE;
                }
                if( bNotify )
                    pFly->InvalidateSize();
            }
        }
    }
}

void SwLayoutFrm::InvaPercentLowers( SwTwips nDiff )
{
    if ( GetDrawObjs() )
        ::InvaPercentFlys( this, nDiff );

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
                ::InvaPercentFlys( pFrm, nDiff );
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
    {
        if( pFrm && pFrm->IsBodyFrm() )
            pFrm = ((SwBodyFrm*)pFrm)->Lower();
        if ( pFrm && pFrm->IsTxtFrm() )
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
    {
        if ( pFrm->IsInTab() )
            pFrm = pFrm->FindTabFrm();

        if ( pFrm->GetDrawObjs() )
        {
            sal_uInt32 nCnt = pFrm->GetDrawObjs()->Count();
            for ( USHORT i = 0; i < nCnt; ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                    if ( pFly->IsHeightClipped() &&
                         ( !pFly->IsFlyFreeFrm() || pFly->GetPageFrm() ) )
                        return TRUE;
                }
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
        SWRECTFN( this )
        if( IsSctFrm() )
        {
            nMaximum = (Frm().*fnRect->fnGetHeight)() - nBorder +
                       (Frm().*fnRect->fnBottomDist)(
                                        (GetUpper()->*fnRect->fnGetPrtBottom)() );
            nMaximum += GetUpper()->Grow( LONG_MAX PHEIGHT, TRUE );
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
                ( !(Frm().*fnRect->fnGetHeight)() && pAny ) )
            {
                long nTop = (this->*fnRect->fnGetTopMargin)();
                (Frm().*fnRect->fnAddBottom)( nMaximum );
                if( nTop > nMaximum )
                    nTop = nMaximum;
                (this->*fnRect->fnSetYMargins)( nTop, 0 );
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

            // --> FME 2004-07-19 #i27399#
            // Simply setting the column width based on the values returned by
            // CalcColWidth does not work for automatic column width.
            AdjustColumns( &rCol, sal_False );
            // <--

            for ( USHORT i = 0; i < nNumCols; ++i )
            {
                pCol->Calc();
                // ColumnFrms besitzen jetzt einen BodyFrm, der auch kalkuliert werden will
                pCol->Lower()->Calc();
                if( pCol->Lower()->GetNext() )
                    pCol->Lower()->GetNext()->Calc();  // SwFtnCont
                pCol = (SwLayoutFrm*)pCol->GetNext();
            }

            ::CalcCntnt( this );

            pCol = (SwLayoutFrm*)Lower();
            ASSERT( pCol && pCol->GetNext(), ":-( Spalten auf Urlaub?");
            // bMinDiff wird gesetzt, wenn es keine leere Spalte gibt
            BOOL bMinDiff = TRUE;
            // OD 28.03.2003 #108446# - check for all column content and all columns
            while ( bMinDiff && pCol )
            {
                bMinDiff = 0 != pCol->ContainsCntnt();
                pCol = (SwLayoutFrm*)pCol->GetNext();
            }
            pCol = (SwLayoutFrm*)Lower();
            // OD 28.03.2003 #108446# - initialize local variable
            SwFrm *pLow = NULL;
            SwTwips nDiff = 0;
            SwTwips nMaxFree = 0;
            SwTwips nAllFree = LONG_MAX;
            // bFoundLower wird gesetzt, wenn es mind. eine nichtleere Spalte gibt
            BOOL bFoundLower = FALSE;
            while( pCol )
            {
                SwLayoutFrm* pLay = (SwLayoutFrm*)pCol->Lower();
                SwTwips nInnerHeight = (pLay->Frm().*fnRect->fnGetHeight)() -
                                       (pLay->Prt().*fnRect->fnGetHeight)();
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
                    nInnerHeight += (pLay->Frm().*fnRect->fnGetHeight)() -
                                    (pLay->Prt().*fnRect->fnGetHeight)();
                }
                nInnerHeight -= (pCol->Prt().*fnRect->fnGetHeight)();
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
                    long nPrtHeight = (Prt().*fnRect->fnGetHeight)();
                    // Das Minimum darf nicht kleiner sein als unsere PrtHeight,
                    // solange noch etwas herausragt.
                    if( nMinimum < nPrtHeight )
                        nMinimum = nPrtHeight;
                    // Es muss sichergestellt sein, dass das Maximum nicht kleiner
                    // als die PrtHeight ist, wenn noch etwas herausragt
                    if( nMaximum < nPrtHeight )
                        nMaximum = nPrtHeight;  // Robust, aber kann das ueberhaupt eintreten?
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
                        long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
                        if ( nFrmHeight > nMinHeight || nPrtHeight >= nMinDiff )
                            nDiff = Max( nDiff, nMinDiff );
                        else if( nDiff < nMinDiff )
                            nDiff = nMinDiff - nPrtHeight + 1;
                    }
                    // nMaximum ist eine Groesse, in der der Inhalt gepasst hat,
                    // oder der von der Umgebung vorgegebene Wert, deshalb
                    // brauchen wir nicht ueber diesen Wrt hinauswachsen.
                    if( nDiff + nPrtHeight > nMaximum )
                        nDiff = nMaximum - nPrtHeight;
                }
                else if( nMaximum > nMinimum ) // Wir passen, haben wir auch noch Spielraum?
                {
                    long nPrtHeight = (Prt().*fnRect->fnGetHeight)();
                    if ( nMaximum < nPrtHeight )
                        nDiff = nMaximum - nPrtHeight; // wir sind ueber eine funktionierende
                        // Hoehe hinausgewachsen und schrumpfen wieder auf diese zurueck,
                        // aber kann das ueberhaupt eintreten?
                    else
                    {   // Wir haben ein neues Maximum, eine Groesse, fuer die der Inhalt passt.
                        nMaximum = nPrtHeight;
                        // Wenn der Freiraum in den Spalten groesser ist als nMinDiff und wir
                        // nicht dadurch wieder unter das Minimum rutschen, wollen wir ein wenig
                        // Luft herauslassen.
                        if( !bNoBalance && ( nMaxFree >= nMinDiff && (!nAllFree
                            || nMinimum < nPrtHeight - nMinDiff ) ) )
                        {
                            nMaxFree /= nNumCols; // auf die Spalten verteilen
                            nDiff = nMaxFree < nMinDiff ? -nMinDiff : -nMaxFree; // mind. nMinDiff
                            if( nPrtHeight + nDiff <= nMinimum ) // Unter das Minimum?
                                nDiff = ( nMinimum - nMaximum ) / 2; // dann lieber die Mitte
                        }
                        else if( nAllFree )
                        {
                            nDiff = -nAllFree;
                            if( nPrtHeight + nDiff <= nMinimum ) // Less than minimum?
                                nDiff = ( nMinimum - nMaximum ) / 2; // Take the center
                        }
                    }
                }
                if( nDiff ) // jetzt wird geschrumpft oder gewachsen..
                {
                    Size aOldSz( Prt().SSize() );
                    long nTop = (this->*fnRect->fnGetTopMargin)();
                    nDiff = (Prt().*fnRect->fnGetHeight)() + nDiff + nBorder -
                            (Frm().*fnRect->fnGetHeight)();
                    (Frm().*fnRect->fnAddBottom)( nDiff );
                    (this->*fnRect->fnSetYMargins)( nTop, nBorder - nTop );
                    ChgLowersProp( aOldSz );
                    NotifyLowerObjs();

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
    // OD 01.04.2003 #108446# - Don't collect endnotes for sections. Thus, set
    // 2nd parameter to <true>.
    ::CalcCntnt( this, true );
    if( IsSctFrm() )
    {
        // OD 14.03.2003 #i11760# - adjust 2nd parameter - TRUE --> true
        ::CalcCntnt( this, true );
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
    SwSortedObjs &rObjs = *pCnt->GetDrawObjs();
    for ( USHORT i = 0; i < rObjs.Count(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = rObjs[i];
        if ( pAnchoredObj->ISA(SwFlyFrm) )
        {
            SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
            if ( pFly->IsFlyInCntFrm() )
            {
                ::lcl_InvalidateCntnt( pFly->ContainsCntnt(), nInv );
                if( nInv & INV_DIRECTION )
                    pFly->CheckDirChange();
            }
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
            const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
            for ( USHORT i = 0; i < rObjs.Count(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                    ::lcl_InvalidateCntnt( pFly->ContainsCntnt(), nInv );
                    if ( nInv & INV_DIRECTION )
                        pFly->CheckDirChange();
                }
            }
        }
        if( nInv & INV_DIRECTION )
            pPage->CheckDirChange();
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

/** method to invalidate/re-calculate the position of all floating
    screen objects (Writer fly frames and drawing objects), which are
    anchored to paragraph or to character.

    OD 2004-03-16 #i11860#

    @author OD
*/
void SwRootFrm::InvalidateAllObjPos()
{
    const SwPageFrm* pPageFrm = static_cast<const SwPageFrm*>(Lower());
    while( pPageFrm )
    {
        pPageFrm->InvalidateFlyLayout();

        if ( pPageFrm->GetSortedObjs() )
        {
            const SwSortedObjs& rObjs = *(pPageFrm->GetSortedObjs());
            for ( sal_uInt8 i = 0; i < rObjs.Count(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                const SwFmtAnchor& rAnch = pAnchoredObj->GetFrmFmt().GetAnchor();
                if ( rAnch.GetAnchorId() != FLY_AT_CNTNT &&
                     rAnch.GetAnchorId() != FLY_AUTO_CNTNT )
                {
                    // only to paragraph and to character anchored objects are considered.
                    continue;
                }
                // --> OD 2004-07-07 #i28701# - special invalidation for anchored
                // objects, whose wrapping style influence has to be considered.
                if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() )
                    pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence( true );
                else
                    pAnchoredObj->InvalidateObjPos();
                // <--
            }
        }

        pPageFrm = static_cast<const SwPageFrm*>(pPageFrm->GetNext());
    }
}
