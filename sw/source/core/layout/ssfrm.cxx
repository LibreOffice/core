/*************************************************************************
 *
 *  $RCSfile: ssfrm.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:13:14 $
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

#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _DVIEW_HXX
#include <dview.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>       // ClearPara()
#endif
#ifndef _CELLFRM_HXX
#include <cellfrm.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _TXTFTN_HXX
#include <txtftn.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif

#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTS_HXX
#include <hints.hxx>        //fuer SwFmtChg
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
// OD 2004-04-06 #i26791#
#ifndef _ANCHOREDOBJECT_HXX
#include <anchoredobject.hxx>
#endif
// OD 2004-03-10 #i11860#
#ifndef _FLOWFRM_HXX
#include <flowfrm.hxx>
#endif
// OD 2004-05-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif

    // No inline cause we need the function pointers
long SwFrm::GetTopMargin() const
    { return Prt().Top(); }
long SwFrm::GetBottomMargin() const
    { return Frm().Height() -Prt().Height() -Prt().Top(); }
long SwFrm::GetLeftMargin() const
    { return Prt().Left(); }
long SwFrm::GetRightMargin() const
    { return Frm().Width() - Prt().Width() - Prt().Left(); }
long SwFrm::GetPrtLeft() const
    { return Frm().Left() + Prt().Left(); }
long SwFrm::GetPrtBottom() const
    { return Frm().Top() + Prt().Height() + Prt().Top(); }
long SwFrm::GetPrtRight() const
    { return Frm().Left() + Prt().Width() + Prt().Left(); }
long SwFrm::GetPrtTop() const
    { return Frm().Top() + Prt().Top(); }

BOOL SwFrm::SetMinLeft( long nDeadline )
{
    SwTwips nDiff = nDeadline - Frm().Left();
    if( nDiff > 0 )
    {
        Frm().Left( nDeadline );
        Prt().Width( Prt().Width() - nDiff );
        return TRUE;
    }
    return FALSE;
}

BOOL SwFrm::SetMaxBottom( long nDeadline )
{
    SwTwips nDiff = Frm().Top() + Frm().Height() - nDeadline;
    if( nDiff > 0 )
    {
        Frm().Height( Frm().Height() - nDiff );
        Prt().Height( Prt().Height() - nDiff );
        return TRUE;
    }
    return FALSE;
}

BOOL SwFrm::SetMinTop( long nDeadline )
{
    SwTwips nDiff = nDeadline - Frm().Top();
    if( nDiff > 0 )
    {
        Frm().Top( nDeadline );
        Prt().Height( Prt().Height() - nDiff );
        return TRUE;
    }
    return FALSE;
}

BOOL SwFrm::SetMaxRight( long nDeadline )
{
    SwTwips nDiff = Frm().Left() + Frm().Width() - nDeadline;
    if( nDiff > 0 )
    {
        Frm().Width( Frm().Width() - nDiff );
        Prt().Width( Prt().Width() - nDiff );
        return TRUE;
    }
    return FALSE;
}

void SwFrm::MakeBelowPos( const SwFrm* pUp, const SwFrm* pPrv, BOOL bNotify )
{
    if( pPrv )
    {
        aFrm.Pos( pPrv->Frm().Pos() );
        aFrm.Pos().Y() += pPrv->Frm().Height();
    }
    else
    {
        aFrm.Pos( pUp->Frm().Pos() );
        aFrm.Pos() += pUp->Prt().Pos();
    }
    if( bNotify )
        aFrm.Pos().Y() += 1;
}

void SwFrm::MakeUpperPos( const SwFrm* pUp, const SwFrm* pPrv, BOOL bNotify )
{
    if( pPrv )
    {
        aFrm.Pos( pPrv->Frm().Pos() );
        aFrm.Pos().Y() -= Frm().Height();
    }
    else
    {
        aFrm.Pos( pUp->Frm().Pos() );
        aFrm.Pos() += pUp->Prt().Pos();
        aFrm.Pos().Y() += pUp->Prt().Height() - aFrm.Height();
    }
    if( bNotify )
        aFrm.Pos().Y() -= 1;
}

void SwFrm::MakeLeftPos( const SwFrm* pUp, const SwFrm* pPrv, BOOL bNotify )
{
    if( pPrv )
    {
        aFrm.Pos( pPrv->Frm().Pos() );
        aFrm.Pos().X() -= Frm().Width();
    }
    else
    {
        aFrm.Pos( pUp->Frm().Pos() );
        aFrm.Pos() += pUp->Prt().Pos();
        aFrm.Pos().X() += pUp->Prt().Width() - aFrm.Width();
    }
    if( bNotify )
        aFrm.Pos().X() -= 1;
}

void SwFrm::MakeRightPos( const SwFrm* pUp, const SwFrm* pPrv, BOOL bNotify )
{
    if( pPrv )
    {
        aFrm.Pos( pPrv->Frm().Pos() );
        aFrm.Pos().X() += pPrv->Frm().Width();
    }
    else
    {
        aFrm.Pos( pUp->Frm().Pos() );
        aFrm.Pos() += pUp->Prt().Pos();
    }
    if( bNotify )
        aFrm.Pos().X() += 1;
}

void SwFrm::SetTopBottomMargins( long nTop, long nBot )
{
    Prt().Top( nTop );
    Prt().Height( Frm().Height() - nTop - nBot );
}

void SwFrm::SetBottomTopMargins( long nBot, long nTop )
{
    Prt().Top( nTop );
    Prt().Height( Frm().Height() - nTop - nBot );
}

void SwFrm::SetLeftRightMargins( long nLeft, long nRight)
{
    Prt().Left( nLeft );
    Prt().Width( Frm().Width() - nLeft - nRight );
}

void SwFrm::SetRightLeftMargins( long nRight, long nLeft)
{
    Prt().Left( nLeft );
    Prt().Width( Frm().Width() - nLeft - nRight );
}

const USHORT nMinVertCellHeight = 1135;

/*-----------------11.9.2001 11:11------------------
 * SwFrm::CheckDirChange(..)
 * checks the layout direction and
 * invalidates the lower frames rekursivly, if necessary.
 * --------------------------------------------------*/

void SwFrm::CheckDirChange()
{
    BOOL bOldVert = GetVerticalFlag();
    BOOL bOldRev = IsReverse();
    BOOL bOldR2L = GetRightToLeftFlag();
    SetInvalidVert( TRUE );
    SetInvalidR2L( TRUE );
    BOOL bChg = bOldR2L != IsRightToLeft();
    if( ( IsVertical() != bOldVert ) || bChg || IsReverse() != bOldRev )
    {
        InvalidateAll();
        if( IsLayoutFrm() )
        {
            // set minimum row height for vertical cells in horizontal table:
            if ( IsCellFrm() && GetUpper() )
            {
                if ( IsVertical() != GetUpper()->IsVertical() )
                {
                    SwTableLine* pLine = (SwTableLine*)((SwCellFrm*)this)->GetTabBox()->GetUpper();
                    SwFrmFmt* pFrmFmt = pLine->GetFrmFmt();
                    SwFmtFrmSize aNew( pFrmFmt->GetFrmSize() );
                    if ( ATT_FIX_SIZE != aNew.GetHeightSizeType() )
                        aNew.SetHeightSizeType( ATT_MIN_SIZE );
                    if ( aNew.GetHeight() < nMinVertCellHeight )
                        aNew.SetHeight( nMinVertCellHeight );
                    SwDoc* pDoc = pFrmFmt->GetDoc();
                    pDoc->SetAttr( aNew, *pLine->ClaimFrmFmt() );
                }
            }

            SwFrm* pFrm = ((SwLayoutFrm*)this)->Lower();
            const SwFmtCol* pCol = NULL;
            SwLayoutFrm* pBody;
            if( pFrm )
            {
                if( IsPageFrm() )
                {
                    // If we're a page frame and we change our layout direction,
                    // we have to look for columns and rearrange them.
                    pBody = ((SwPageFrm*)this)->FindBodyCont();
                    if(pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm())
                        pCol = &((SwPageFrm*)this)->GetFmt()->GetCol();

                    SwSortedObjs* pObjs = ((SwPageFrm*)this)->GetSortedObjs();
                    if( pObjs )
                    {
                        sal_uInt32 nCnt = pObjs->Count();
                        for ( sal_uInt32 i = 0; i < nCnt; ++i )
                        {
                            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                            if ( pAnchoredObj->GetAnchorFrm() == this )
                            {
                                if ( pAnchoredObj->ISA(SwFlyFrm) )
                                {
                                    static_cast<SwFlyFrm*>(pAnchoredObj)->CheckDirChange();
                                }
                                else
                                {
                                    // OD 2004-04-06 #i26791# - direct object
                                    // positioning no longer needed. Instead
                                    // invalidate
                                    pAnchoredObj->InvalidateObjPos();
                                }
                            }
                        }
                    }
                }
                else if( pFrm->IsColumnFrm() )
                {
                    pBody = ((SwLayoutFrm*)this);
                    const SwFrmFmt *pFmt = pBody->GetFmt();
                    if( pFmt )
                        pCol = &pFmt->GetCol();
                }
            }
            while( pFrm )
            {
                pFrm->CheckDirChange();
                pFrm = pFrm->GetNext();
            }
            if( pCol )
                pBody->AdjustColumns( pCol, TRUE );
        }
        else if( IsTxtFrm() )
            ((SwTxtFrm*)this)->Prepare( PREP_CLEAR );

        if( !IsPageFrm() && GetDrawObjs() )
        {
            const SwSortedObjs *pObjs = GetDrawObjs();
            sal_uInt32 nCnt = pObjs->Count();
            for ( sal_uInt32 i = 0; i < nCnt; ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                if( pAnchoredObj->ISA(SwFlyFrm) )
                    static_cast<SwFlyFrm*>(pAnchoredObj)->CheckDirChange();
                else
                {
                    // OD 2004-04-06 #i26791# - direct object
                    // positioning no longer needed. Instead
                    // invalidate
                    pAnchoredObj->InvalidateObjPos();
                }
            }
        }
    }
}

/*-----------------13.9.2002 11:11------------------
 * SwFrm::GetFrmAnchorPos(..)
 * returns the position for anchors based on frame direction
 * --------------------------------------------------*/
// OD 2004-03-10 #i11860# - consider lower space and line spacing of
// previous frame according to new option 'Use former object positioning'
Point SwFrm::GetFrmAnchorPos( sal_Bool bIgnoreFlysAnchoredAtThisFrame ) const
{
    Point aAnchor = Frm().Pos();
    if ( IsVertical() || IsRightToLeft() )
        aAnchor.X() += Frm().Width();

    if ( IsTxtFrm() )
    {
        SwTwips nBaseOfstForFly =
            ((SwTxtFrm*)this)->GetBaseOfstForFly( bIgnoreFlysAnchoredAtThisFrame );
        if ( IsVertical() )
            aAnchor.Y() += nBaseOfstForFly;
        else
            aAnchor.X() += nBaseOfstForFly;

        // OD 2004-03-10 #i11860# - if option 'Use former object positioning'
        // is OFF, consider the lower space and the line spacing of the
        // previous frame and the spacing considered for the page grid
        const SwTxtFrm* pThisTxtFrm = static_cast<const SwTxtFrm*>(this);
        const SwTwips nUpperSpaceAmountConsideredForPrevFrmAndPageGrid =
                pThisTxtFrm->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
        if ( IsVertical() )
        {
            aAnchor.X() -= nUpperSpaceAmountConsideredForPrevFrmAndPageGrid;
        }
        else
        {
            aAnchor.Y() += nUpperSpaceAmountConsideredForPrevFrmAndPageGrid;
        }
    }

    return aAnchor;
}


/*************************************************************************
|*
|*  SwFrm::~SwFrm()
|*
|*  Ersterstellung      MA 02. Mar. 94
|*  Letzte Aenderung    MA 25. Jun. 95
|*
|*************************************************************************/


SwFrm::~SwFrm()
{
    // accessible objects for fly and cell frames have been already disposed
    // by the destructors of the derived classes.
    if( IsAccessibleFrm() && !(IsFlyFrm() || IsCellFrm()) && GetDep() )
    {
        SwRootFrm *pRootFrm = FindRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() )
        {
            ViewShell *pVSh = pRootFrm->GetCurrShell();
            if( pVSh && pVSh->Imp() )
            {
                ASSERT( !GetLower(), "Lowers should be dispose already!" );
                pVSh->Imp()->DisposeAccessibleFrm( this );
            }
        }
    }

    if( pDrawObjs )
    {
        for ( sal_uInt32 i = pDrawObjs->Count(); i; )
        {
            SwAnchoredObject* pAnchoredObj = (*pDrawObjs)[--i];
            if ( pAnchoredObj->ISA(SwFlyFrm) )
                delete pAnchoredObj;
            else
            {
                SdrObject* pSdrObj = pAnchoredObj->DrawObj();
                SwDrawContact* pContact =
                        static_cast<SwDrawContact*>(pSdrObj->GetUserCall());
                ASSERT( pContact,
                        "<SwFrm::~SwFrm> - missing contact for drawing object" );
                if ( pContact )
                {
                    pContact->DisconnectObjFromLayout( pSdrObj );
                }
            }
        }
        if ( pDrawObjs )
            delete pDrawObjs;
    }

#ifndef PRODUCT
    // JP 15.10.2001: for detection of access to deleted frames
    pDrawObjs = (SwSortedObjs*)0x33333333;
#endif
}

/*************************************************************************
|*
|*    SwLayoutFrm::SetFrmFmt()
|*    Ersterstellung    MA 22. Apr. 93
|*    Letzte Aenderung  MA 02. Nov. 94
|*
|*************************************************************************/


void SwLayoutFrm::SetFrmFmt( SwFrmFmt *pNew )
{
    if ( pNew != GetFmt() )
    {
        SwFmtChg aOldFmt( GetFmt() );
        pNew->Add( this );
        SwFmtChg aNewFmt( pNew );
        Modify( &aOldFmt, &aNewFmt );
    }
}

/*************************************************************************
|*
|*    SwCntntFrm::SwCntntFrm(), ~SwCntntFrm()
|*
|*    Ersterstellung    AK 15-Feb-1991
|*    Letzte Aenderung  MA 25. Apr. 95
|*
|*************************************************************************/


SwCntntFrm::SwCntntFrm( SwCntntNode * const pCntnt ) :
    SwFrm( pCntnt ),
    SwFlowFrm( (SwFrm&)*this )
{
}



SwCntntFrm::~SwCntntFrm()
{
    SwCntntNode* pCNd;
    if( 0 != ( pCNd = PTR_CAST( SwCntntNode, pRegisteredIn )) &&
        !pCNd->GetDoc()->IsInDtor() )
    {
        //Bei der Root abmelden wenn ich dort noch im Turbo stehe.
        SwRootFrm *pRoot = FindRootFrm();
        if( pRoot && pRoot->GetTurbo() == this )
        {
            pRoot->DisallowTurbo();
            pRoot->ResetTurbo();
        }
        if( IsTxtFrm() && ((SwTxtFrm*)this)->HasFtn() )
        {
            SwTxtNode *pTxtNd = ((SwTxtFrm*)this)->GetTxtNode();
            const SwFtnIdxs &rFtnIdxs = pCNd->GetDoc()->GetFtnIdxs();
            USHORT nPos;
            ULONG nIndex = pCNd->GetIndex();
            rFtnIdxs.SeekEntry( *pTxtNd, &nPos );
            SwTxtFtn* pTxtFtn;
            if( nPos < rFtnIdxs.Count() )
            {
                while( nPos && pTxtNd == &(rFtnIdxs[ nPos ]->GetTxtNode()) )
                    --nPos;
                if( nPos || pTxtNd != &(rFtnIdxs[ nPos ]->GetTxtNode()) )
                    ++nPos;
            }
            while( nPos < rFtnIdxs.Count() )
            {
                pTxtFtn = rFtnIdxs[ nPos ];
                if( pTxtFtn->GetTxtNode().GetIndex() > nIndex )
                    break;
                pTxtFtn->DelFrms();
                ++nPos;
            }
        }
    }
    if( IsTxtFrm() && ((SwTxtFrm*)this)->HasBlinkPor() )
        ((SwTxtFrm*)this)->ClearPara();
}

/*************************************************************************
|*
|*  SwLayoutFrm::~SwLayoutFrm
|*
|*  Ersterstellung      AK 28-Feb-1991
|*  Letzte Aenderung    MA 11. Jan. 95
|*
|*************************************************************************/


SwLayoutFrm::~SwLayoutFrm()
{
    SwFrm *pFrm = pLower;

    if( GetFmt() && !GetFmt()->GetDoc()->IsInDtor() )
    {
        while ( pFrm )
        {
            //Erst die Objs des Frm vernichten, denn diese koennen sich sonst nach
            //dem Remove nicht mehr bei der Seite abmelden.
            //Falls sich einer nicht abmeldet wollen wir nicht gleich
            //endlos schleifen.

            sal_uInt32 nCnt;
            while ( pFrm->GetDrawObjs() && pFrm->GetDrawObjs()->Count() )
            {
                nCnt = pFrm->GetDrawObjs()->Count();
                // --> OD 2004-06-30 #i28701#
                SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[0];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                    delete pAnchoredObj;
                else
                {
                    SdrObject* pSdrObj = pAnchoredObj->DrawObj();
                    SwDrawContact* pContact =
                            static_cast<SwDrawContact*>(pSdrObj->GetUserCall());
                    ASSERT( pContact,
                            "<SwFrm::~SwFrm> - missing contact for drawing object" );
                    if ( pContact )
                    {
                        pContact->DisconnectObjFromLayout( pSdrObj );
                    }
                }
                if ( pFrm->GetDrawObjs() &&
                     nCnt == pFrm->GetDrawObjs()->Count() )
                {
                    pFrm->GetDrawObjs()->Remove( *pAnchoredObj );
                }
                // <--
            }
            pFrm->Remove();
            delete pFrm;
            pFrm = pLower;
        }
        //Fly's vernichten. Der letzte loescht gleich das Array.
        sal_uInt32 nCnt;
        while ( GetDrawObjs() && GetDrawObjs()->Count() )
        {
            nCnt = GetDrawObjs()->Count();

            // --> OD 2004-06-30 #i28701#
            SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[0];
            if ( pAnchoredObj->ISA(SwFlyFrm) )
                delete pAnchoredObj;
            else
            {
                SdrObject* pSdrObj = pAnchoredObj->DrawObj();
                SwDrawContact* pContact =
                        static_cast<SwDrawContact*>(pSdrObj->GetUserCall());
                ASSERT( pContact,
                        "<SwFrm::~SwFrm> - missing contact for drawing object" );
                if ( pContact )
                {
                    pContact->DisconnectObjFromLayout( pSdrObj );
                }
            }
            if ( GetDrawObjs() && nCnt == GetDrawObjs()->Count() )
            {
                GetDrawObjs()->Remove( *pAnchoredObj );
            }
            // <--
        }
    }
    else
    {
        while( pFrm )
        {
            SwFrm *pNxt = pFrm->GetNext();
            delete pFrm;
            pFrm = pNxt;
        }
    }
}

/*************************************************************************
|*
|*  SwFrm::PaintArea()
|*
|*  Created     AMA 08/22/2000
|*  Last change AMA 08/23/2000
|*
|*  The paintarea is the area, in which the content of a frame is allowed
|*  to be displayed. This region could be larger than the printarea (Prt())
|*  of the upper, it includes e.g. often the margin of the page.
|*
|*************************************************************************/

const SwRect SwFrm::PaintArea() const
{
    SwRect aRect( Frm() );
    const FASTBOOL bVert = IsVertical();
    SwRectFn fnRect = bVert ? fnRectVert : fnRectHori;
    long nRight = (aRect.*fnRect->fnGetRight)();
    long nLeft  = (aRect.*fnRect->fnGetLeft)();
    const SwFrm* pTmp = this;
    BOOL bLeft = TRUE;
    BOOL bRight = TRUE;
    while( pTmp )
    {
        long nTmpRight = (pTmp->Frm().*fnRect->fnGetRight)();
        long nTmpLeft = (pTmp->Frm().*fnRect->fnGetLeft)();
        ASSERT( pTmp, "PaintArea lost in time and space" );
        if( pTmp->IsPageFrm() || pTmp->IsFlyFrm() ||
            pTmp->IsCellFrm() || pTmp->IsRowFrm() || //nobody leaves a table!
            pTmp->IsRootFrm() )
        {
            if( bLeft || nLeft < nTmpLeft )
                nLeft = nTmpLeft;
            if( bRight || nTmpRight < nRight )
                nRight = nTmpRight;
            if( pTmp->IsPageFrm() || pTmp->IsFlyFrm() || pTmp->IsRootFrm() )
                break;
            bLeft = FALSE;
            bRight = FALSE;
        }
        else if( pTmp->IsColumnFrm() )  // nobody enters neightbour columns
        {
            BOOL bR2L = pTmp->IsRightToLeft();
            // the first column has _no_ influence to the left range
            if( bR2L ? pTmp->GetNext() : pTmp->GetPrev() )
            {
                if( bLeft || nLeft < nTmpLeft )
                    nLeft = nTmpLeft;
                bLeft = FALSE;
            }
             // the last column has _no_ influence to the right range
            if( bR2L ? pTmp->GetPrev() : pTmp->GetNext() )
            {
                if( bRight || nTmpRight < nRight )
                    nRight = nTmpRight;
                bRight = FALSE;
            }
        }
        else if( bVert && pTmp->IsBodyFrm() )
        {
            // Header and footer frames have always horizontal direction and
            // limit the body frame.
            // A previous frame of a body frame must be a header,
            // the next frame of a body frame may be a footnotecontainer or
            // a footer. The footnotecontainer has the same direction like
            // the body frame.
            if( pTmp->GetPrev() && ( bLeft || nLeft < nTmpLeft ) )
            {
                nLeft = nTmpLeft;
                bLeft = FALSE;
            }
            if( pTmp->GetNext() &&
                ( pTmp->GetNext()->IsFooterFrm() || pTmp->GetNext()->GetNext() )
                && ( bRight || nTmpRight < nRight ) )
            {
                nRight = nTmpRight;
                bRight = FALSE;
            }
        }
        pTmp = pTmp->GetUpper();
    }
    (aRect.*fnRect->fnSetLeft)( nLeft );
    (aRect.*fnRect->fnSetRight)( nRight );
    return aRect;
}

/*************************************************************************
|*
|*  SwFrm::UnionFrm()
|*
|*  Created     AMA 08/22/2000
|*  Last change AMA 08/23/2000
|*
|*  The unionframe is the framearea (Frm()) of a frame expanded by the
|*  printarea, if there's a negative margin at the left or right side.
|*
|*************************************************************************/

const SwRect SwFrm::UnionFrm( BOOL bBorder ) const
{
    BOOL bVert = IsVertical();
    SwRectFn fnRect = bVert ? fnRectVert : fnRectHori;
    long nLeft = (Frm().*fnRect->fnGetLeft)();
    long nWidth = (Frm().*fnRect->fnGetWidth)();
    long nPrtLeft = (Prt().*fnRect->fnGetLeft)();
    long nPrtWidth = (Prt().*fnRect->fnGetWidth)();
    if( nPrtLeft + nPrtWidth > nWidth )
        nWidth = nPrtLeft + nPrtWidth;
    if( nPrtLeft < 0 )
    {
        nLeft += nPrtLeft;
        nWidth -= nPrtLeft;
    }
    SwTwips nRight = nLeft + nWidth;
    long nAdd = 0;
    if( bBorder )
    {
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        const SvxBoxItem &rBox = rAttrs.GetBox();
        if ( rBox.GetLeft() )
            nLeft -= rBox.CalcLineSpace( BOX_LINE_LEFT );
        else if ( rAttrs.IsBorderDist() )
            nLeft -= rBox.GetDistance( BOX_LINE_LEFT ) + 1;
        if ( rBox.GetRight() )
            nAdd += rBox.CalcLineSpace( BOX_LINE_RIGHT );
        else if ( rAttrs.IsBorderDist() )
            nAdd += rBox.GetDistance( BOX_LINE_RIGHT ) + 1;
        if( rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE )
        {
            const SvxShadowItem &rShadow = rAttrs.GetShadow();
            nLeft -= rShadow.CalcShadowSpace( SHADOW_LEFT );
            nAdd += rShadow.CalcShadowSpace( SHADOW_RIGHT );
        }
    }
    if( IsTxtFrm() && ((SwTxtFrm*)this)->HasPara() )
    {
        long nTmp = ((SwTxtFrm*)this)->HangingMargin();
        if( nTmp > nAdd )
            nAdd = nTmp;
    }
    nWidth = nRight + nAdd - nLeft;
    SwRect aRet( Frm() );
    (aRet.*fnRect->fnSetPosX)( nLeft );
    (aRet.*fnRect->fnSetWidth)( nWidth );
    return aRet;
}




