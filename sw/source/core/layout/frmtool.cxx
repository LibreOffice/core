/*************************************************************************
 *
 *  $RCSfile: frmtool.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-02 10:54:01 $
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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#define ITEMID_SIZE         SID_ATTR_PAGE_SIZE
#include <hintids.hxx>


#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX //autogen
#include <sfx2/progress.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif


#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTCNCT_HXX //autogen
#include <fmtcnct.hxx>
#endif
#ifndef _DOCSTAT_HXX //autogen
#include <docstat.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef SW_LINEINFO_HXX //autogen
#include <lineinfo.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "cntfrm.hxx"
#include "colfrm.hxx"
#include "flyfrm.hxx"
#include "doc.hxx"
#include "fesh.hxx"
#include "viewimp.hxx"
#include "pam.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "frmtool.hxx"
#include "docsh.hxx"
#include "swtable.hxx"
#include "errhdl.hxx"
#include "tabfrm.hxx"
#include "rowfrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "notxtfrm.hxx"
#include "flyfrms.hxx"
#include "frmsh.hxx"
#include "layact.hxx"
#include "pagedesc.hxx"
#include "section.hxx"
#include "sectfrm.hxx"
#include "node2lay.hxx"
#include "ndole.hxx"
#include "ndtxt.hxx"

#include "mdiexp.hxx"
#include "statstr.hrc"

// ftnfrm.cxx:
void lcl_RemoveFtns( SwFtnBossFrm* pBoss, BOOL bPageOnly, BOOL bEndNotes );

FASTBOOL bObjsDirect = TRUE;
FASTBOOL bDontCreateObjects = FALSE;
FASTBOOL bSetCompletePaintOnInvalidate = FALSE;

BYTE StackHack::nCnt = 0;
BOOL StackHack::bLocked = FALSE;

/*************************************************************************
|*
|*  SwFrmNotify::SwFrmNotify()
|*
|*  Ersterstellung      MA 27. Nov. 92
|*  Letzte Aenderung    MA 09. Apr. 97
|*
|*************************************************************************/

SwFrmNotify::SwFrmNotify( SwFrm *pF ) :
    pFrm( pF ),
    aFrm( pF->Frm() ),
    aPrt( pF->Prt() ),
    bInvaKeep( FALSE )
{
    bHadFollow = pF->IsCntntFrm() ?
                    (((SwCntntFrm*)pF)->GetFollow() ? TRUE : FALSE) :
                    FALSE;
}

/*************************************************************************
|*
|*  SwFrmNotify::~SwFrmNotify()
|*
|*  Ersterstellung      MA 27. Nov. 92
|*  Letzte Aenderung    MA 09. Apr. 97
|*
|*************************************************************************/

SwFrmNotify::~SwFrmNotify()
{
    const FASTBOOL bAbsP = aFrm.Pos() != pFrm->Frm().Pos();

    if ( pFrm->IsFlowFrm() && !pFrm->IsInFtn() )
    {
        SwFlowFrm *pFlow = SwFlowFrm::CastFlowFrm( pFrm );

        if ( !pFlow->IsFollow() )
        {
            if ( !pFrm->GetIndPrev() )
            {
                if ( bInvaKeep )
                {
                    //Wenn der Vorgaenger das Attribut fuer Zusammenhalten traegt
                    //muss er angestossen werden.
                    SwFrm *pPre;
                    if ( 0 != (pPre = pFrm->FindPrev()) &&
                         pPre->GetAttrSet()->GetKeep().GetValue() )
                        pPre->InvalidatePos();
                }
            }
            else if ( !pFlow->HasFollow() &&
                      (aFrm.Height() > pFrm->Frm().Height() ||
                       (!aFrm.Height() && pFrm->Frm().Height()) ) )
                pFlow->CheckKeep();
        }
    }

    if ( bAbsP )
    {
        pFrm->SetCompletePaint();

        SwFrm* pNxt = pFrm->GetIndNext();

        if ( pNxt )
            pNxt->InvalidatePos();
        else
        {
            if( pFrm->IsRetoucheFrm() && aFrm.Pos().Y() > pFrm->Frm().Pos().Y() )
                pFrm->SetRetouche();

            //Wenn ein TxtFrm gerade einen Follow erzeugt hat, so ist dieser
            //frisch formatiert und braucht nicht nocheinmal angestossen werden.
            if ( bHadFollow || !pFrm->IsCntntFrm() ||
                 !((SwCntntFrm*)pFrm)->GetFollow() )
                pFrm->InvalidateNextPos();
        }
    }

    //Fuer Hintergrundgrafiken muss bei Groessenaenderungen ein Repaint her.
    const FASTBOOL bPrtS = aPrt.SSize()!= pFrm->Prt().SSize();
    if ( bPrtS )
    {
        const SvxGraphicPosition ePos = pFrm->GetAttrSet()->GetBackground().GetGraphicPos();
        if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
            pFrm->SetCompletePaint();
    }

    //Auch die Flys wollen etwas von den Veraenderungen mitbekommen,
    //FlyInCnts brauchen hier nicht benachrichtigt werden.
    if ( pFrm->GetDrawObjs() )
    {
        const FASTBOOL bPrtP = aPrt.Pos() != pFrm->Prt().Pos();
        const FASTBOOL bFrmS = aFrm.SSize()!= pFrm->Frm().SSize();

        if ( bAbsP || bPrtP || bFrmS || bPrtS )
        {
            const SwDrawObjs &rObjs = *pFrm->GetDrawObjs();
            SwPageFrm *pPage = 0;
            for ( USHORT i = 0; i < rObjs.Count(); ++i )
            {
                FASTBOOL bNotify = FALSE;
                FASTBOOL bNotifySize = FALSE;
                SdrObject *pObj = rObjs[i];
                if ( pObj->IsWriterFlyFrame() )
                {
                    SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
                    if ( !pFly->IsFlyInCntFrm() )
                    {
                        //Wenn sich die AbsPos geaendert hat oder der Anker kein
                        //CntntFrm ist, so benachrichten wir auf jeden Fall.
                        if ( bAbsP || !pFly->GetAnchor()->IsCntntFrm() )
                        {
                            bNotify = TRUE;
                            if ( bAbsP )
                            {
                                if ( !pPage )
                                    pPage = pFrm->FindPageFrm();
                                SwPageFrm *pFlyPage = pFly->FindPageFrm();
                                // Am Rahmen gebundene Objekte wandern stets mit,
                                // an TxtFrms gebundene nicht unbedingt.
                                //MA 09. Jul. 98: An TxtFrms gebundene wurden
                                //bereits im MakeAll formatiert und sollten
                                //damit auf der richtigen Seite stehen.
                                if ( pPage != pFlyPage && pFrm->IsFlyFrm() )
//                                   (pFrm->IsFlyFrm() || pOldPage != pPage ||
//                                    WEIT_WECH == pFly->Frm().Top()) )
                                {
                                    ASSERT( pFlyPage, "~SwFrmNotify: Fly from Nowhere" );
                                    if( pFlyPage )
                                        pFlyPage->MoveFly( pFly, pPage );
                                    else
                                        pPage->SwPageFrm::AppendFly( pFly );
                                }
                            }
                        }
                        else
                        {
                            //Andere benachrichtigen wir nur wenn sie eine
                            //automatische Ausrichtung haben.
                            //MA 16. Oct. 95: (fix:21063) Verfeinert.
                            const SwFmtVertOrient &rVert =
                                        pFly->GetFmt()->GetVertOrient();
                            const SwFmtHoriOrient &rHori =
                                        pFly->GetFmt()->GetHoriOrient();
                            if ( (rVert.GetVertOrient()    == VERT_CENTER  ||
                                  rVert.GetVertOrient()    == VERT_BOTTOM  ||
                                  rVert.GetRelationOrient()== PRTAREA)  &&
                                 (aFrm.Height() != pFrm->Frm().Height() ||
                                  aPrt.Height() != pFrm->Prt().Height()) )
                            {
                                bNotify = TRUE;
                            }
                            if ( rHori.GetHoriOrient() != HORI_NONE &&
                                 (aFrm.Width() != pFrm->Frm().Width() ||
                                  aPrt.Width() != pFrm->Prt().Width()) )
                            {
                                bNotify = TRUE;
                            }
                        }
                    }
                    else if ( aPrt.Width() != pFrm->Prt().Width() )
                    {
                        bNotify = TRUE;
                        bNotifySize = TRUE;
                    }
                    if ( bNotify )
                    {
                        if ( bNotifySize )
                            pFly->_InvalidateSize();
                        pFly->_InvalidatePos();
                        pFly->_Invalidate();
                    }
                }
                else if ( bAbsP )
                {
                    SwFrmFmt *pFrmFmt = FindFrmFmt( pObj );
                    if( !pFrmFmt ||
                        FLY_IN_CNTNT != pFrmFmt->GetAnchor().GetAnchorId() )
                    {
                        pObj->SetAnchorPos( pFrm->Frm().Pos() );
                        ((SwDrawContact*)GetUserCall(pObj))->ChkPage();
                    }
                }
            }
        }
    }
}

/*************************************************************************
|*
|*  SwLayNotify::SwLayNotify()
|*
|*  Ersterstellung      MA 17. Nov. 92
|*  Letzte Aenderung    MA 03. Jun. 93
|*
|*************************************************************************/


SwLayNotify::SwLayNotify( SwLayoutFrm *pLayFrm ) :
    SwFrmNotify( pLayFrm ),
    nHeightOfst( 0 ),
    nWidthOfst ( 0 ),
    bLowersComplete( FALSE )
{
}

/*************************************************************************
|*
|*  SwLayNotify::~SwLayNotify()
|*
|*  Ersterstellung      MA 17. Nov. 92
|*  Letzte Aenderung    MA 13. Jun. 96
|*
|*************************************************************************/

void MA_FASTCALL lcl_MoveDrawObjs( SwFrm *pLow, const Point &rDiff,
                                   SwPageFrm *pNewPage )
{
    for ( USHORT i = 0; pLow->GetDrawObjs() && i < pLow->GetDrawObjs()->Count();
            ++i )
    {
        SdrObject *pObj = (*pLow->GetDrawObjs())[i];
        if ( pObj->IsWriterFlyFrame() )
        {
            SwFlyFrm *pF = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
            if ( pF->Frm().Left() != WEIT_WECH )
            {
                {//Scope fuer Notify
                    SwFlyNotify aNotify( pF );
                    pF->Frm().Pos() += rDiff;
                    //Wenn ein Fly die Position wechselt muss er
                    //natuerlich an der Seite umgemeldet werden.
                    if ( pF->IsFlyFreeFrm() )
                    {
                        if ( aNotify.GetOldPage() != pNewPage )
                            aNotify.GetOldPage()->MoveFly( pF, pNewPage );
                    }
                }
            }
        }
        else
        {
            pObj->SetAnchorPos( pObj->GetAnchorPos() + rDiff );
            ((SwDrawContact*)GetUserCall(pObj))->ChkPage();
        }
    }
}

void MA_FASTCALL lcl_MoveLowerFlys( SwLayoutFrm *pLay, const Point &rDiff,
                                    SwPageFrm *pNewPage )
{
    if( pLay->IsFlyFrm() )
        ::lcl_MoveDrawObjs( pLay, rDiff, pNewPage );

    SwFrm *pLow = pLay->Lower();
    if( !pLow )
        return ;

    do
    {   if ( pLow->GetDrawObjs() )
            ::lcl_MoveDrawObjs( pLow, rDiff, pNewPage );
        pLow->Frm().Pos() += rDiff;
        pLow->InvalidatePos();
        if ( pLow->IsTxtFrm() )
            ((SwTxtFrm*)pLow)->Prepare( PREP_POS_CHGD );
        else if ( pLow->IsTabFrm() )
            pLow->InvalidatePrt();
        if ( pLow->IsLayoutFrm() )
            ::lcl_MoveLowerFlys( (SwLayoutFrm*)pLow, rDiff, pNewPage );

        pLow = pLow->GetNext();
    } while ( pLow );
}

SwLayNotify::~SwLayNotify()
{
    SwLayoutFrm *pLay = GetLay();
    FASTBOOL bNotify = FALSE;
    if ( pLay->Prt().SSize() != aPrt.SSize() )
    {
        if ( !IsLowersComplete() )
        {
            BOOL bInvaPercent;

            if ( pLay->IsRowFrm() )
            {
                bInvaPercent = TRUE;
                if ( pLay->Prt().Height() != aPrt.Height() )
                     ((SwRowFrm*)pLay)->AdjustCells( pLay->Prt().Height(),TRUE);
                if ( pLay->Prt().Width() != aPrt.Width() )
                     ((SwRowFrm*)pLay)->AdjustCells( 0, FALSE );
            }
            else
            {
                //Proportionale Anpassung der innenliegenden.
                //1. Wenn der Formatierte kein Fly ist
                //2. Wenn er keine Spalten enthaelt
                //3. Wenn der Fly eine feste Hoehe hat und die Spalten in der
                //   Hoehe danebenliegen.
                //4. niemals bei SectionFrms.
                BOOL bLow;
                if( pLay->IsFlyFrm() )
                {
                    if ( pLay->Lower() )
                    {
                        bLow = !pLay->Lower()->IsColumnFrm() ||
                            pLay->Lower()->Frm().Height() != pLay->Prt().Height();
                    }
                    else
                        bLow = FALSE;
                }
                else if( pLay->IsSctFrm() )
                {
                    if ( pLay->Lower() )
                    {
                        if( pLay->Lower()->IsColumnFrm() && pLay->Lower()->GetNext() )
                            bLow = pLay->Lower()->Frm().Height() != pLay->Prt().Height();
                        else
                            bLow = pLay->Prt().Width() != aPrt.Width();
                    }
                    else
                        bLow = FALSE;
                }
                else
                    bLow = TRUE;
                bInvaPercent = bLow;
                if ( bLow )
                {
                    if ( nHeightOfst || nWidthOfst )
                    {
                        const Size aSz( aPrt.Width()  + nWidthOfst,
                                        aPrt.Height() + nHeightOfst );
                        if ( pLay->Prt().SSize() != aSz )
                            pLay->ChgLowersProp( aSz );
                    }
                    else
                        pLay->ChgLowersProp( aPrt.SSize() );

                }
                //Wenn die PrtArea gewachsen ist, so ist es moeglich, dass die
                //Kette der Untergeordneten einen weiteren Frm aufnehmen kann,
                //mithin muss also der 'moeglicherweise passende' Invalidiert werden.
                //Das invalidieren lohnt nur, wenn es sich beim mir bzw. meinen
                //Uppers um eine Moveable-Section handelt.
                //Die PrtArea ist gewachsen, wenn die Breite oder die Hoehe groesser
                //geworden ist.
                if ( (pLay->Prt().Height() > aPrt.Height() ||
                      pLay->Prt().Width()  > aPrt.Width()) &&
                     (pLay->IsMoveable() || pLay->IsFlyFrm()) )
                {
                    SwFrm *pFrm = pLay->Lower();
                    if ( pFrm && pFrm->IsFlowFrm() )
                    {
                        while ( pFrm->GetNext() )
                            pFrm = pFrm->GetNext();
                        pFrm->InvalidateNextPos();
                    }
                }
            }
            bNotify = TRUE;
            //TEUER!! aber wie macht man es geschickter?
            if( bInvaPercent )
                pLay->InvaPercentLowers();
        }
        if ( pLay->IsTabFrm() )
            //Damit _nur_ der Shatten bei Groessenaenderungen gemalt wird.
            ((SwTabFrm*)pLay)->SetComplete();
        else if ( !pLay->GetFmt()->GetDoc()->IsBrowseMode() ||
                  !(pLay->GetType() & (FRM_BODY | FRM_PAGE)) )
            //Damit die untergeordneten sauber retouchiert werden.
            //Problembsp: Flys an den Henkeln packen und verkleinern.
            //Nicht fuer Body und Page, sonst flackerts beim HTML-Laden.
            pLay->SetCompletePaint();

    }
    //Lower benachrichtigen wenn sich die Position veraendert hat.
    const BOOL bPrtPos = pLay->Prt().Pos() != aPrt.Pos();
    const BOOL bPos = bPrtPos || pLay->Frm().Pos() != aFrm.Pos();

    if ( bPos && pLay->Lower() && !IsLowersComplete() )
        pLay->Lower()->InvalidatePos();

    if ( bPrtPos )
        pLay->SetCompletePaint();

    //Nachfolger benachrichtigen wenn sich die SSize geaendert hat.
    if ( pLay->Frm().SSize() != aFrm.SSize() )
    {
        if( pLay->GetNext() )
        {
            if ( pLay->GetNext()->IsLayoutFrm() )
                pLay->GetNext()->_InvalidatePos();
            else
                pLay->GetNext()->InvalidatePos();
        }
        else if( pLay->IsSctFrm() )
            pLay->InvalidateNextPos();
    }
    if ( !IsLowersComplete() &&
         !((pLay->GetType()&FRM_FLY|FRM_SECTION) &&
            pLay->Lower() && pLay->Lower()->IsColumnFrm()) &&
         (bPos || bNotify) && !(pLay->GetType() & 0x1823) )  //Tab, Row, FtnCont, Root, Page
    {
        pLay->NotifyFlys();
    }
    if ( bPos && pLay->IsFtnFrm() && pLay->Lower() )
    {
        Point aDiff( pLay->Frm().Pos() );
        aDiff -= aFrm.Pos();
        lcl_MoveLowerFlys( pLay, aDiff, pLay->FindPageFrm() );
    }
}

/*************************************************************************
|*
|*  SwFlyNotify::SwFlyNotify()
|*
|*  Ersterstellung      MA 17. Nov. 92
|*  Letzte Aenderung    MA 26. Aug. 93
|*
|*************************************************************************/

SwFlyNotify::SwFlyNotify( SwFlyFrm *pFlyFrm ) :
    SwLayNotify( pFlyFrm ),
    pOldPage( pFlyFrm->FindPageFrm() ),
    aFrmAndSpace( pFlyFrm->AddSpacesToFrm() )
{
}

/*************************************************************************
|*
|*  SwFlyNotify::~SwFlyNotify()
|*
|*  Ersterstellung      MA 17. Nov. 92
|*  Letzte Aenderung    MA 09. Nov. 95
|*
|*************************************************************************/

SwFlyNotify::~SwFlyNotify()
{
    SwFlyFrm *pFly = GetFly();
    if ( pFly->IsNotifyBack() )
    {
        ViewShell *pSh = pFly->GetShell();
        SwViewImp *pImp = pSh ? pSh->Imp() : 0;
        if ( !pImp || !pImp->IsAction() || !pImp->GetLayAction().IsAgain() )
        {
            //Wenn in der LayAction das IsAgain gesetzt ist kann es sein,
            //dass die alte Seite inzwischen vernichtet wurde!
            ::Notify( pFly, pOldPage, aFrmAndSpace );
        }
        pFly->ResetNotifyBack();
    }

    //Haben sich Groesse oder Position geaendert, so sollte die View
    //das wissen.
    const FASTBOOL bPosChgd = pFly->Frm().Pos() != aFrm.Pos();
    if ( bPosChgd || pFly->Frm().SSize() != aFrm.SSize() )
    {
        pFly->NotifyDrawObj();
    }
    if ( bPosChgd && aFrm.Pos().X() != WEIT_WECH )
    {
        //Bei Spalten sind die Lower wahrscheinlich bereits Formatiert und
        //Positioniert. Bei zeichengebundenen Rahmen mit Spalten macht dies
        //heftige Probleme #42867#
        if ( pFly->Lower() &&
             (!pFly->IsFlyInCntFrm() || !pFly->Lower()->IsColumnFrm()) )
        {
            Point aDiff( pFly->Frm().Pos() );
            aDiff -= aFrm.Pos();
            lcl_MoveLowerFlys( pFly, aDiff, pFly->FindPageFrm() );
        }

        if ( pFly->IsFlyAtCntFrm() )
        {
            SwFrm *pNxt = pFly->GetAnchor()->FindNext();
            if ( pNxt )
                pNxt->InvalidatePos();
        }
    }
}
/*************************************************************************
|*
|*  SwCntntNotify::SwCntntNotify()
|*
|*  Ersterstellung      MA 24. Nov. 92
|*  Letzte Aenderung    MA 16. May. 95
|*
|*************************************************************************/

SwCntntNotify::SwCntntNotify( SwCntntFrm *pCntntFrm ) :
    SwFrmNotify( pCntntFrm )
{
}

/*************************************************************************
|*
|*  SwCntntNotify::~SwCntntNotify()
|*
|*  Ersterstellung      MA 24. Nov. 92
|*  Letzte Aenderung    MA 09. Apr. 97
|*
|*************************************************************************/

SwCntntNotify::~SwCntntNotify()
{
    SwCntntFrm *pCnt = GetCnt();
    if ( bSetCompletePaintOnInvalidate )
        pCnt->SetCompletePaint();


    //Wenn sich meine PrtArea in der Fix-Size geaendert hat, so muss mein
    //Nachfolger dazu angeregt werden sich auch neu zu Formatieren.

//MA: Ist das wirklich noetig? Auf keinen Fall sollte das doch notwendig sein,
//wenn der Frm das erste Mal formatiert wurde (alte PrtArea == 0).
/*  if ( pCnt->GetNext() &&
         pCnt->Prt().Width() != aPrt.Width() )
    {
        pCnt->GetNext()->Prepare( PREP_FIXSIZE_CHG );
        pCnt->GetNext()->_InvalidatePrt();
        pCnt->GetNext()->InvalidateSize();
    }
*/
    if ( pCnt->IsInTab() && (pCnt->Frm().Pos() != aFrm.Pos() ||
                             pCnt->Frm().SSize() != aFrm.SSize()))
    {
        SwLayoutFrm* pCell = pCnt->GetUpper();
        while( !pCell->IsCellFrm() && pCell->GetUpper() )
            pCell = pCell->GetUpper();
        ASSERT( pCell->IsCellFrm(), "Where's my cell?" );
        if ( VERT_NONE != pCell->GetFmt()->GetVertOrient().GetVertOrient() )
            pCell->InvalidatePrt(); //fuer vertikale Ausrichtung.
    }

    FASTBOOL bFirst = aFrm.Width() == 0 && aFrm.Height() == 0;

    if ( pCnt->IsNoTxtFrm() )
    {
        //Aktive PlugIn's oder OLE-Objekte sollten etwas von der Veraenderung
        //mitbekommen, damit sie Ihr Window entsprechend verschieben.
        ViewShell *pSh  = pCnt->GetShell();
        if ( pSh )
        {
            SwOLENode *pNd;
            if ( 0 != (pNd = pCnt->GetNode()->GetOLENode()) &&
                 (pNd->GetOLEObj().IsOleRef() ||
                  pNd->IsOLESizeInvalid()) )
            {
                ASSERT( pCnt->IsInFly(), "OLE not in FlyFrm" );
                SwFlyFrm *pFly = pCnt->FindFlyFrm();
                SvEmbeddedObjectRef xObj( (SvInPlaceObject*) pNd->GetOLEObj().GetOleRef() );
                SwFEShell *pFESh = 0;
                ViewShell *pTmp = pSh;
                do
                {   if ( pTmp->ISA( SwCrsrShell ) )
                    {
                        pFESh = (SwFEShell*)pTmp;
                        if ( !bFirst )
                            pFESh->CalcAndSetScale( xObj, &pFly->Prt(), &pFly->Frm());
                    }
                    pTmp = (ViewShell*)pTmp->GetNext();
                } while ( pTmp != pSh );

                if ( pFESh && pNd->IsOLESizeInvalid() )
                {
                    pNd->SetOLESizeInvalid( FALSE );
                    xObj->OnDocumentPrinterChanged( pNd->GetDoc()->GetPrt() );
                    pFESh->CalcAndSetScale( xObj );//Client erzeugen lassen.
                }
            }
            //dito Animierte Grafiken
            if ( Frm().HasArea() && ((SwNoTxtFrm*)pCnt)->HasAnimation() )
            {
                ((SwNoTxtFrm*)pCnt)->StopAnimation();
                pSh->InvalidateWindows( Frm() );
            }
        }
    }

    if ( bFirst )
    {
        pCnt->SetRetouche();    //fix(13870)

        SwDoc *pDoc = pCnt->GetNode()->GetDoc();
        if ( pDoc->GetSpzFrmFmts()->Count() &&
             !pDoc->IsLoaded() && !pDoc->IsNewDoc() )
        {
            //Der Frm wurde wahrscheinlich zum ersten mal formatiert.
            //Wenn ein Filter Flys oder Zeichenobjekte einliest und diese
            //Seitengebunden sind, hat er ein Problem, weil er i.d.R. die
            //Seitennummer nicht kennt. Er weiss lediglich welches der Inhalt
            //(CntntNode) an dieser Stelle ist.
            //Die Filter stellen dazu das Ankerattribut der Objekte so ein, dass
            //sie vom Typ zwar Seitengebunden sind, aber der Index des Ankers
            //auf diesen CntntNode zeigt.
            //Hier werden diese vorlauefigen Verbindungen aufgeloest.

            const SwPageFrm *pPage = 0;
            SwNodeIndex   *pIdx  = 0;
            SwSpzFrmFmts *pTbl = pDoc->GetSpzFrmFmts();

            for ( USHORT i = 0; i < pTbl->Count(); ++i )
            {
                if ( !pPage )
                    pPage = pCnt->FindPageFrm();
                SwFrmFmt *pFmt = (*pTbl)[i];
                const SwFmtAnchor &rAnch = pFmt->GetAnchor();

                if ( FLY_PAGE       != rAnch.GetAnchorId() &&
                     FLY_AT_CNTNT   != rAnch.GetAnchorId() )
                    continue;   //#60878# nicht etwa zeichengebundene.

                FASTBOOL bCheckPos = FALSE;
                if ( rAnch.GetCntntAnchor() )
                {
                    if ( !pIdx )
                    {
                        pIdx = new SwNodeIndex( *pCnt->GetNode() );
                    }
                    if ( rAnch.GetCntntAnchor()->nNode == *pIdx )
                    {
                        bCheckPos = TRUE;
                        if ( FLY_PAGE == rAnch.GetAnchorId() )
                        {
                            SwFmtAnchor aAnch( rAnch );
                            aAnch.SetAnchor( 0 );
                            aAnch.SetPageNum( pPage->GetPhyPageNum() );
                            pFmt->SetAttr( aAnch );
                            if ( RES_DRAWFRMFMT != pFmt->Which() )
                                pFmt->MakeFrms();
                        }
                    }
                }
                if ( !bCheckPos || RES_DRAWFRMFMT != pFmt->Which() )
                    continue;

                SdrObject *pObj = pFmt->FindSdrObject();
                const Point aAktPos( pObj->GetSnapRect().TopLeft() );
                Point aPos( aAktPos );
                FASTBOOL bSetPos = FALSE;
                SwFmtVertOrient *pVert;
                if ( SFX_ITEM_SET == pFmt->GetAttrSet().GetItemState(
                            RES_VERT_ORIENT, FALSE, (const SfxPoolItem**)&pVert ) )
                {
                    bSetPos = TRUE;
                    switch ( pVert->GetRelationOrient() )
                    {
                        case REL_PG_FRAME:      aPos.Y() = pPage->Frm().Top(); break;
                        case REL_PG_PRTAREA:    aPos.Y() = pPage->Frm().Top();
                                                aPos.Y() += pPage->Prt().Top(); break;
                        case PRTAREA:           aPos.Y() = pCnt->Frm().Top();
                                                aPos.Y() += pCnt->Prt().Top(); break;
                        case FRAME:             aPos.Y() = pCnt->Frm().Top(); break;
                        default:
                            bSetPos = FALSE;
                            ASSERT( !this,"neuer Trick vom WW Reader?" );
                    }
                    aPos.Y() += pVert->GetPos();
                    pFmt->ResetAttr( RES_VERT_ORIENT );
                }
                SwFmtHoriOrient *pHori;
                if ( SFX_ITEM_SET == pFmt->GetAttrSet().GetItemState(
                            RES_HORI_ORIENT, FALSE, (const SfxPoolItem**)&pHori ) )
                {
                    bSetPos = TRUE;
                    switch ( pHori->GetRelationOrient() )
                    {
                        case REL_PG_FRAME:      aPos.X() = pPage->Frm().Left(); break;
                        case REL_PG_PRTAREA:    aPos.X() = pPage->Frm().Left();
                                                aPos.X() += pPage->Prt().Left();    break;
                        case PRTAREA:
                        case FRAME:
                            // da es fuer den WW95/97 Import ist und die
                            // Horizontal nur Spalten kennen, muss hier die
                            // Spalte gesucht werden. Wenn es keine gibt,
                            // ist es die PrtArea der Seite.
                            {
                                SwFrm* pColFrm = pCnt->FindColFrm();
                                if( pColFrm )
                                    aPos.X() = pColFrm->Frm().Left() +
                                               pColFrm->Prt().Left();
                                else
                                    aPos.X() = pPage->Frm().Left() +
                                               pPage->Prt().Left();
                            }
                            break;
                        default:
                            bSetPos = FALSE;
                            ASSERT( !this,"neuer Trick vom WW Reader?" );
                    }
                    aPos.X() += pHori->GetPos();
                    pFmt->ResetAttr( RES_HORI_ORIENT );
                }
                if ( bSetPos )
                {
                    aPos -= aAktPos;
                    pObj->Move( Size( aPos.X(), aPos.Y() ) );
                }
            }
            delete pIdx;
        }
    }
}

/*************************************************************************
|*
|*  InsertCnt
|*
|*  Beschreibung        Hilfsfunktionen, die friend von irgendwem sind, damit
|*                      nicht immer gleich 'ne ganze Klasse befreundet werden
|*                      muss.
|*  Ersterstellung      MA 13. Apr. 93
|*  Letzte Aenderung    MA 11. May. 95
|*
|*************************************************************************/

void AppendObjs( const SwSpzFrmFmts *pTbl, ULONG nIndex,
                        SwFrm *pFrm, SwPageFrm *pPage )
{
    for ( USHORT i = 0; i < pTbl->Count(); ++i )
    {
        SwFrmFmt *pFmt = (SwFrmFmt*)(*pTbl)[i];
        const SwFmtAnchor &rAnch = pFmt->GetAnchor();
        if ( rAnch.GetCntntAnchor() &&
             (rAnch.GetCntntAnchor()->nNode.GetIndex() == nIndex) )
        {
            BOOL bFlyAtFly = rAnch.GetAnchorId() == FLY_AT_FLY; // LAYER_IMPL

            if( bFlyAtFly || rAnch.GetAnchorId() == FLY_AT_CNTNT ||
                rAnch.GetAnchorId() == FLY_AUTO_CNTNT )
            {
                //Wird ein Rahmen oder ein SdrObject beschrieben?
                const BOOL bSdrObj = RES_DRAWFRMFMT == pFmt->Which();
                SdrObject *pSdrObj = 0;
                if ( bSdrObj  && 0 == (pSdrObj = pFmt->FindSdrObject()) )
                {
                    ASSERT( !bSdrObj, "DrawObject not found." );
                    pFmt->GetDoc()->DelFrmFmt( pFmt );
                    --i;
                    continue;
                }
                if ( pSdrObj )
                {
                    if ( !pSdrObj->GetPage() )
                        pFmt->GetDoc()->GetDrawModel()->GetPage(0)->
                                InsertObject(pSdrObj, pSdrObj->GetOrdNumDirect());
                    pFrm->AppendDrawObj((SwDrawContact*)GetUserCall(pSdrObj) );
                }
                else
                {
                    SwFlyFrm *pFly;
                    if( bFlyAtFly )
                        pFly = new SwFlyLayFrm( (SwFlyFrmFmt*)pFmt, pFrm );
                    else
                        pFly = new SwFlyAtCntFrm( (SwFlyFrmFmt*)pFmt, pFrm );
                    pFly->Lock();
                    pFrm->AppendFly( pFly );
                    pFly->Unlock();
                    if ( pPage )
                        ::RegistFlys( pPage, pFly );
                }
            }
        }
    }
}

FASTBOOL MA_FASTCALL lcl_ObjConnected( SwFrmFmt *pFmt )
{
    SwClientIter aIter( *pFmt );
    if ( RES_FLYFRMFMT == pFmt->Which() )
        return 0 != aIter.First( TYPE(SwFlyFrm) );
    else
    {
        SwDrawContact *pContact;
        if ( 0 != (pContact = (SwDrawContact*)aIter.First( TYPE(SwDrawContact))))
            return pContact->GetAnchor() != 0;
    }
    return FALSE;
}

void AppendAllObjs( const SwSpzFrmFmts *pTbl )
{
    //Verbinden aller Objekte, die in der SpzTbl beschrieben sind mit dem
    //Layout.
    //Wenn sich nix mehr tut hoeren wir auf. Dann koennen noch Formate
    //uebrigbleiben, weil wir weder zeichengebunde Rahmen verbinden noch
    //Objecte die in zeichengebundenen verankert sind.

    SwSpzFrmFmts aCpy( 255, 255 );
    aCpy.Insert( pTbl, 0 );

    USHORT nOldCnt = USHRT_MAX;

    while ( aCpy.Count() && aCpy.Count() != nOldCnt )
    {
        nOldCnt = aCpy.Count();
        for ( int i = 0; i < int(aCpy.Count()); ++i )
        {
            SwFrmFmt *pFmt = (SwFrmFmt*)aCpy[ USHORT(i) ];
            const SwFmtAnchor &rAnch = pFmt->GetAnchor();
            FASTBOOL bRemove = FALSE;
            if ( rAnch.GetAnchorId() == FLY_PAGE || rAnch.GetAnchorId() == FLY_IN_CNTNT )
                //Seitengebunde sind bereits verankert, zeichengebundene
                //will ich hier nicht.
                bRemove = TRUE;
            else if ( FALSE == (bRemove = ::lcl_ObjConnected( pFmt )) )
            {
                //Fuer Flys und DrawObjs nur dann ein MakeFrms rufen wenn noch
                //keine abhaengigen Existieren, andernfalls, oder wenn das
                //MakeFrms keine abhaengigen erzeugt, entfernen.
                pFmt->MakeFrms();
                bRemove = ::lcl_ObjConnected( pFmt );
            }
            if ( bRemove )
            {
                aCpy.Remove( USHORT(i) );
                --i;
            }
        }
    }
    aCpy.Remove( 0, aCpy.Count() );
}

BOOL MA_FASTCALL lcl_CheckInsertPage( SwFrm *pFrm, SwPageFrm *&rpPage,
                                      SwLayoutFrm *&rpLay,
                                      ULONG nParagraphCnt,
                                      USHORT nMaxParaPerPage, BOOL &rbBreakAfter )
{
    FASTBOOL bEnd = 0 == rpPage->GetNext();
    const SwAttrSet *pAttr = pFrm->GetAttrSet();
    const SvxFmtBreakItem &rBrk = pAttr->GetBreak();
    const SwFmtPageDesc &rDesc = pAttr->GetPageDesc();
    const SwPageDesc *pDesc = rDesc.GetPageDesc();

    BOOL bBrk = nParagraphCnt > nMaxParaPerPage || rbBreakAfter;
    rbBreakAfter = rBrk.GetBreak() == SVX_BREAK_PAGE_AFTER ||
                   rBrk.GetBreak() == SVX_BREAK_PAGE_BOTH;
    if ( !bBrk )
        bBrk = rBrk.GetBreak() == SVX_BREAK_PAGE_BEFORE ||
               rBrk.GetBreak() == SVX_BREAK_PAGE_BOTH;

    if ( bBrk || pDesc )
    {
        USHORT nPgNum = 0;
        if ( !pDesc )
            pDesc = rpPage->GetPageDesc()->GetFollow();
        else
        {
            if ( 0 != (nPgNum = rDesc.GetNumOffset()) )
                ((SwRootFrm*)rpPage->GetUpper())->SetVirtPageNum(TRUE);
        }
        BOOL bOdd = !rpPage->OnRightPage();
        BOOL bInsertEmpty = FALSE;
        if( nPgNum && bOdd != ( ( nPgNum % 2 ) != 0 ) )
        {
            bOdd = !bOdd;
            bInsertEmpty = TRUE;
        }
        ::InsertNewPage( (SwPageDesc&)*pDesc, rpPage->GetUpper(),
                         bOdd, bInsertEmpty, FALSE, rpPage->GetNext() );
        if ( bEnd )
        {
            ASSERT( rpPage->GetNext(), "Keine neue Seite?" );
            do
            {   rpPage = (SwPageFrm*)rpPage->GetNext();
            } while ( rpPage->GetNext() );
        }
        else
        {
            ASSERT( rpPage->GetNext(), "Keine neue Seite?" );
            rpPage = (SwPageFrm*)rpPage->GetNext();
            if ( rpPage->IsEmptyPage() )
            {
                ASSERT( rpPage->GetNext(), "Keine neue Seite?" );
                rpPage = (SwPageFrm*)rpPage->GetNext();
            }
        }
        rpLay = rpPage->FindBodyCont();
        while( rpLay->Lower() )
            rpLay = (SwLayoutFrm*)rpLay->Lower();
        return TRUE;
    }
    return FALSE;
}

//Solange eine Section "offen" ist ist im InsertCnt ein Pointer auf die
//Hilfsklasse SwActualSection vorhanden.
//Bei Seitenumbruchen wird fuer "offene" Sections entsprechend ein Follow
//erzeugt.
//Da Bereiche ineinander verschachtelt sein koennen, sich aber wiederum im
//Layout nicht ineinander verschachteln, hat die Klasse eine Pointer auf ihren
//Upper wenn eine "innere" Section notwendig ist. Wenn diese abgeschlossen ist
//wird einfach wieder der "Upper" weiterverwendet (im Layout muss dafuer
//natuerlich wieder ein entsprechender Frame erzeugt werden.

//!!Nicht fuer im Layout verschachtelte Bereiche funktionsfaehig.

class SwActualSection
{
    SwActualSection *pUpper;
    SwSectionFrm    *pSectFrm;
    SwSectionNode   *pSectNode;
public:
    SwActualSection( SwActualSection *pUpper,
                     SwSectionFrm    *pSect,
                     SwSectionNode   *pNd );

    SwSectionFrm    *GetSectionFrm()                    { return pSectFrm; }
    void             SetSectionFrm( SwSectionFrm *p )   { pSectFrm = p; }
    SwSectionNode   *GetSectionNode()                   { return pSectNode;}
    SwActualSection *GetUpper()                         { return pUpper; }
};

SwActualSection::SwActualSection( SwActualSection *pUp,
                                  SwSectionFrm    *pSect,
                                  SwSectionNode   *pNd ) :
    pUpper( pUp ),
    pSectFrm( pSect ),
    pSectNode( pNd )
{
    if ( !pSectNode )
    {
        const SwNodeIndex *pIndex = pSect->GetFmt()->GetCntnt().GetCntntIdx();
        pSectNode = pSect->GetFmt()->GetDoc()->GetNodes()[*pIndex]->
                                                            FindSectionNode();
    }
}

void MA_FASTCALL _InsertCnt( SwLayoutFrm *pLay, SwDoc *pDoc,
                             ULONG nIndex, BOOL bPages, ULONG nEndIndex,
                             SwFrm *pPrv )
{
    const BOOL bOldIdle = pDoc->IsIdleTimerActive();
    pDoc->StopIdleTimer();
    const BOOL bOldCallbackActionEnabled = pDoc->GetRootFrm()->IsCallbackActionEnabled();
    pDoc->GetRootFrm()->SetCallbackActionEnabled( FALSE );

    //Bei der Erzeugung des Layouts wird bPages mit TRUE uebergeben. Dann
    //werden schon mal alle x Absaetze neue Seiten angelegt. Bei umbruechen
    //und/oder Pagedescriptorwechseln werden gleich die entsprechenden Seiten
    //angelegt.
    //Vorteil ist, das einerseits schon eine annaehernd realistische Zahl von
    //Seiten angelegt wird, vor allem aber gibt es nicht mehr eine schier
    //lange Kette von Absaetzen teuer verschoben werden muss, bis sie sich auf
    //ertraegliches mass reduziert hat.
    //Wir gehen mal davon aus, daá 20 Absaetze auf eine Seite passen
    //Damit es in extremen Faellen nicht gar so heftig rechenen wir je nach
    //Node noch etwas drauf.
    //Wenn in der DocStatistik eine brauchebare Seitenzahl angegeben ist
    //(wird beim Schreiben gepflegt), so wird von dieser Seitenanzahl
    //ausgegengen.
    ULONG nMaxParaPerPage = 25;
    ULONG nPgCount;
    BOOL bStartPercent = bPages && !nEndIndex &&
                                   !SfxProgress::GetActiveProgress() &&
                                   !SfxProgress::GetActiveProgress( pDoc->GetDocShell() );
    if ( bStartPercent )
    {
        nPgCount = pDoc->GetDocStat().nPage;
        if ( nPgCount <= 10 ) //darunter machen wir es nicht.
            nPgCount = 0;
        ULONG nNdCount = pDoc->GetDocStat().nPara;
        if ( nNdCount <= 1 )
        {
            //Anzahl der Absaetze schaetzen.
            ULONG nTmp = pDoc->GetNodes().GetEndOfContent().GetIndex() -
                        pDoc->GetNodes().GetEndOfExtras().GetIndex();
            //Fuer Die Tabellen ziehen wir einiges wg. der Start-/EndNodes ab.
            nTmp -= pDoc->GetTblFrmFmts()->Count() * 25;
            //Fuer die Rahmen ziehen auch nocheinmal etwa 5 Absaetze ab.
            nTmp -= (pDoc->GetNodes().GetEndOfAutotext().GetIndex() -
                       pDoc->GetNodes().GetEndOfInserts().GetIndex()) / 3 * 5;
            if ( nTmp > 0 )
                nNdCount = nTmp;
        }
        if ( nNdCount < 1000 )
            bStartPercent = FALSE;
        if ( nNdCount > 100 ) //darunter machen wir es nicht
        {
            if ( nPgCount > 0 )
                nMaxParaPerPage = nNdCount / nPgCount;
            else
            {
                nMaxParaPerPage = Max( ULONG(20),
                                       ULONG(20 + nNdCount / 1000 * 3) );
                //Standard ASCII-Leerzeilen
#ifdef PM2
                const ULONG nMax = 49;
#elif MAC
                const ULONG nMax = 56;
#elif UNIX
                const ULONG nMax = 57;
#else
                const ULONG nMax = 53;
#endif
                nMaxParaPerPage = Min( nMaxParaPerPage, nMax );
                nPgCount = nNdCount / nMaxParaPerPage;
            }
            if ( pDoc->IsBrowseMode() )
                nMaxParaPerPage *= 6;
        }
    }

    //Wenn das Layout erzeugt wird (bPages == TRUE) steuern wir den Progress
    //an. Flys und DrawObjekte werden dann nicht gleich verbunden, dies
    //passiert erst am Ende der Funktion.
    if ( bPages && bStartPercent )
    {
        ::StartProgress( STR_STATSTR_LAYOUTINIT, 1, nPgCount, pDoc->GetDocShell());
        bObjsDirect = FALSE;
    }

    SwPageFrm *pPage = pLay->FindPageFrm();
    const SwSpzFrmFmts *pTbl = pDoc->GetSpzFrmFmts();
    SwFrm       *pFrm = 0;
    USHORT nParagraphCnt = 0;
    BOOL   bBreakAfter   = FALSE;
    BOOL   bFirst = TRUE;

    SwActualSection *pActualSection = 0;    //Siehe Dokumentation oben bei der
    if( pLay->IsInSct() &&                  //Klassendefinition
        ( pLay->IsSctFrm() || pLay->GetUpper() ) ) // Hierdurch werden Frischlinge
            // abgefangen, deren Flags noch nicht ermittelt werden koennen,
            // so z.B. beim Einfuegen einer Tabelle
    {
        SwSectionFrm* pSct = pLay->FindSctFrm();
        // Wenn Inhalt in eine Fussnote eingefuegt wird, die in einem spaltigen
        // Bereich liegt, so darf der spaltige Bereich nicht aufgebrochen werden.
        // Nur wenn im Innern der Fussnote ein Bereich liegt, ist dies ein
        // Kandidat fuer pActualSection.
        // Gleiches gilt fuer Bereiche in Tabellen, wenn innerhalb einer Tabelle
        // eingefuegt wird, duerfen nur Bereiche, die ebenfalls im Innern liegen,
        // aufgebrochen werden.
        if( ( !pLay->IsInFtn() || pSct->IsInFtn() ) &&
            ( !pLay->IsInTab() || pSct->IsInTab() ) )
        {
            pActualSection = new SwActualSection( 0, pSct, 0 );
            ASSERT( !pLay->Lower() || !pLay->Lower()->IsColumnFrm(),
                "_InsertCnt: Wrong Call" );
        }
    }

    while( TRUE )
    {
        SwNode *pNd = pDoc->GetNodes()[nIndex];
        if ( pNd->IsCntntNode() )
        {
            SwCntntNode* pNode = (SwCntntNode*)pNd;
            pFrm = pNode->IsTxtNode() ? new SwTxtFrm( (SwTxtNode*)pNode ) :
                                        pNode->MakeFrm();
            if ( bPages )
            {
                ++nParagraphCnt;
                if ( !bFirst || nEndIndex )
                {
                    if ( bFirst )
                        nParagraphCnt = USHRT_MAX;

                    if ( lcl_CheckInsertPage( pFrm, pPage, pLay, nParagraphCnt,
                                              nMaxParaPerPage, bBreakAfter ) )
                    {
                        pPrv = 0;
                        nParagraphCnt = 0;
                        if ( bPages && bStartPercent )
                            ::SetProgressState( pPage->GetPhyPageNum(), pDoc->GetDocShell());

                        if ( pActualSection )
                        {
                            //Hatte der SectionFrm ueberhaupt Inhalt? Wenn
                            //nicht kann er gleich umgehaengt werden.
                            SwFrm *pFrm;
                            if ( !pActualSection->GetSectionFrm()->ContainsCntnt())
                            {
                                pFrm = pActualSection->GetSectionFrm();
                                pFrm->Remove();
                            }
                            else
                            {
                                pFrm = new SwSectionFrm(
                                    *pActualSection->GetSectionFrm(), FALSE );
                                pActualSection->GetSectionFrm()->SimpleFormat();
                                pFrm->Frm().Width( pLay->Prt().Width() );
                                pFrm->Prt().Width( pLay->Prt().Width() );
                            }
                            pActualSection->SetSectionFrm( (SwSectionFrm*)pFrm );
                            pFrm->InsertBehind( pLay, 0 );
                            pFrm->Frm().Pos() = pLay->Frm().Pos();
                            pFrm->Frm().Pos().Y() += 1; //wg. Benachrichtigungen.

                            pLay = (SwLayoutFrm*)pFrm;
                            if ( pLay->Lower() && pLay->Lower()->IsLayoutFrm() )
                                pLay = pLay->GetNextLayoutLeaf();
                        }
                    }
                }
                bFirst = FALSE;
            }
            pFrm->InsertBehind( pLay, pPrv );
            pFrm->Frm().Pos() = pLay->Frm().Pos();
            pFrm->Frm().Pos().Y() += 1; //wg. Benachrichtigungen.
            pPrv = pFrm;

            if ( pTbl->Count() && bObjsDirect && !bDontCreateObjects )
                AppendObjs( pTbl, nIndex, pFrm, pPage );
        }
        else if ( pNd->IsTableNode() )
        {   //Sollten wir auf eine Tabelle gestossen sein?
            SwTableNode *pTblNode = (SwTableNode*)pNd;
            pFrm = pTblNode->MakeFrm();

            if ( bPages )
            {
                //Fuer die Seiten zaehlt jede Zeile als ein Absatz.
                SwFrm *pLow = ((SwTabFrm*)pFrm)->Lower();
                do
                {   ++nParagraphCnt;
                    pLow = pLow->GetNext();
                } while ( pLow );

                if ( !bFirst || nEndIndex )
                {
                    if ( bFirst )
                        nParagraphCnt = USHRT_MAX;

                    if ( lcl_CheckInsertPage( pFrm, pPage, pLay, nParagraphCnt,
                                                nMaxParaPerPage, bBreakAfter ) )
                    {
                        pPrv = 0;
                        nParagraphCnt = 0;
                        if ( bPages && bStartPercent )
                            ::SetProgressState( pPage->GetPhyPageNum(), pDoc->GetDocShell());

                        if ( pActualSection )
                        {
                            //Hatte der SectionFrm ueberhaupt Inhalt? Wenn
                            //nicht kann er gleich umgehaengt werden.
                            SwFrm *pFrm;
                            if ( !pActualSection->GetSectionFrm()->ContainsCntnt())
                            {
                                pFrm = pActualSection->GetSectionFrm();
                                pFrm->Remove();
                            }
                            else
                            {
                                pFrm = new SwSectionFrm(
                                    *pActualSection->GetSectionFrm(), FALSE );
                                pActualSection->GetSectionFrm()->SimpleFormat();
                                pFrm->Frm().Width( pLay->Prt().Width() );
                                pFrm->Prt().Width( pLay->Prt().Width() );
                            }
                            pActualSection->SetSectionFrm( (SwSectionFrm*)pFrm );
                            pFrm->InsertBehind( pLay, 0 );
                            pFrm->Frm().Pos() = pLay->Frm().Pos();
                            pFrm->Frm().Pos().Y() += 1; //wg. Benachrichtigungen.

                            pLay = (SwLayoutFrm*)pFrm;
                            if ( pLay->Lower() && pLay->Lower()->IsLayoutFrm() )
                                pLay = pLay->GetNextLayoutLeaf();
                        }
                    }
                }
            }

            pFrm->InsertBehind( pLay, pPrv );
            if ( bObjsDirect && pTbl->Count() )
                ((SwTabFrm*)pFrm)->RegistFlys();
            pFrm->Frm().Pos() = pLay->Frm().Pos();
            pFrm->Frm().Pos().Y() += 1; //wg. Benachrichtigungen.
            pPrv = pFrm;
            //Index auf den Endnode der Tabellensection setzen.
            nIndex = pTblNode->EndOfSectionIndex();
        }
        else if ( pNd->IsSectionNode() )
        {
            SwSectionNode *pNode = (SwSectionNode*)pNd;
            if( pNode->GetSection().CalcHiddenFlag() )
                // ist versteckt, ueberspringe den Bereich
                nIndex = pNode->EndOfSectionIndex();
            else
            {
                pFrm = pNode->MakeFrm();
                pActualSection = new SwActualSection( pActualSection,
                                                (SwSectionFrm*)pFrm, pNode );
                if ( pActualSection->GetUpper() )
                {
                    //Hinter den Upper einsetzen, beim EndNode wird der "Follow"
                    //des Uppers erzeugt.
                    SwSectionFrm *pTmp = pActualSection->GetUpper()->GetSectionFrm();
                    pFrm->InsertBehind( pTmp->GetUpper(), pTmp );
                }
                else
                {
                    pFrm->InsertBehind( pLay, pPrv );
                    if( pPrv && pPrv->IsInFtn() )
                    {
                        if( pPrv->IsSctFrm() )
                            pPrv = ((SwSectionFrm*)pPrv)->ContainsCntnt();
                        if( pPrv && pPrv->IsTxtFrm() )
                            ((SwTxtFrm*)pPrv)->Prepare( PREP_QUOVADIS, 0, FALSE );
                    }
                }
                pFrm->Frm().Pos() = pLay->Frm().Pos();
                pFrm->Frm().Pos().Y() += 1; //wg. Benachrichtigungen.

                pLay = (SwLayoutFrm*)pFrm;
                if ( pLay->Lower() && pLay->Lower()->IsLayoutFrm() )
                    pLay = pLay->GetNextLayoutLeaf();
                pPrv = 0;
            }
        }
        else if ( pNd->IsEndNode() && pNd->FindStartNode()->IsSectionNode() )
        {
            ASSERT( pActualSection, "Sectionende ohne Anfang?" );
            ASSERT( pActualSection->GetSectionNode() == pNd->FindStartNode(),
                            "Sectionende mit falschen Start Node?" );

            //Section schliessen, ggf. die umgebende Section wieder
            //aktivieren.
            SwActualSection *pTmp = pActualSection->GetUpper();
            delete pActualSection;
            pLay = pLay->FindSctFrm();
            if ( 0 != (pActualSection = pTmp) )
            {
                //Koennte noch sein, das der letzte SectionFrm leer geblieben
                //ist. Dann ist es jetzt an der Zeit ihn zu entfernen.
                if ( !pLay->ContainsCntnt() )
                {
                    SwFrm *pTmp = pLay;
                    pLay = pTmp->GetUpper();
                    pPrv = pTmp->GetPrev();
                    pTmp->Remove();
                    delete pTmp;
                }
                else
                {
                    pPrv = pLay;
                    pLay = pLay->GetUpper();
                }
                // Wenn der umgebende Bereich bereits einen Follow besitzt,
                // wird dieser benutzt. Es muss allerdings die Master/Follow-
                // verkettung geloest werden, unterbrochenene SectionFrms sind
                // _nicht_ durch Master/Follow verbunden.
                pFrm = pActualSection->GetSectionFrm()->GetFollow();
                if( pFrm )
                {
                    ((SwSectionFrm*)pFrm)->_SetIsFollow( FALSE );
                    pActualSection->GetSectionFrm()->SetFollow( NULL );
                    pActualSection->GetSectionFrm()->InvalidateSize();
                }
                else
                {
                    pFrm = pActualSection->GetSectionNode()->MakeFrm();
                    pFrm->InsertBehind( pLay, pPrv );
                    pFrm->Frm().Pos() = pLay->Frm().Pos();
                    pFrm->Frm().Pos().Y() += 1; //wg. Benachrichtigungen.
                }
                SwSectionFrm* pSectFrm = pActualSection->GetSectionFrm();
                // Wir wollen keine leeren Teile zuruecklassen
                if( !pSectFrm->IsColLocked() && !pSectFrm->ContainsCntnt() )
                {
                    pSectFrm->DelEmpty( TRUE );
                    delete pSectFrm;
                }
                pActualSection->SetSectionFrm( (SwSectionFrm*)pFrm );

                pLay = (SwLayoutFrm*)pFrm;
                if ( pLay->Lower() && pLay->Lower()->IsLayoutFrm() )
                    pLay = pLay->GetNextLayoutLeaf();
                pPrv = 0;
            }
            else
            {
                //Nix mehr mit Sections, es geht direkt hinter dem SectionFrame
                //weiter.
                pPrv = pLay;
                pLay = pLay->GetUpper();
            }
        }
        else if( pNd->IsStartNode() &&
                 SwFlyStartNode == ((SwStartNode*)pNd)->GetStartNodeType() )
        {
            if ( pTbl->Count() && bObjsDirect && !bDontCreateObjects )
            {
                SwFlyFrm* pFly = pLay->FindFlyFrm();
                if( pFly )
                    AppendObjs( pTbl, nIndex, pFly, pPage );
            }
        }
        else
            // Weder Cntnt noch Tabelle noch Section,
            // also muessen wir fertig sein.
            break;

        ++nIndex;
        // Der Endnode wird nicht mehr mitgenommen, es muss vom
        // Aufrufenden (Section/MakeFrms()) sichergestellt sein, dass das Ende
        // des Bereichs vor dem EndIndex liegt!
        if ( nEndIndex && nIndex >= nEndIndex )
            break;
    }

    if ( pActualSection )
    {
        //Kann passieren, dass noch eine leere (Follow-)Section uebrig geblieben ist.
        if ( !(pLay = pActualSection->GetSectionFrm())->ContainsCntnt() )
        {
            pLay->Remove();
            delete pLay;
        }
        delete pActualSection;
    }

    if ( bPages )       //Jetzt noch die Flys verbinden lassen.
    {
        if ( !bDontCreateObjects )
            AppendAllObjs( pTbl );
        bObjsDirect = TRUE;
        if ( bStartPercent )
            ::EndProgress( pDoc->GetDocShell() );
    }

    if ( bOldIdle )
        pDoc->StartIdleTimer();
    pDoc->GetRootFrm()->SetCallbackActionEnabled( bOldCallbackActionEnabled );
}

void MakeFrms( SwDoc *pDoc, const SwNodeIndex &rSttIdx,
               const SwNodeIndex &rEndIdx )
{
    bObjsDirect = FALSE;

    SwNodeIndex aTmp( rSttIdx );
    ULONG nEndIdx = rEndIdx.GetIndex();
    SwNode *pNd = pDoc->GetNodes().FindPrvNxtFrmNode( aTmp,
                                            pDoc->GetNodes()[ nEndIdx-1 ]);
    if ( pNd )
    {
        BOOL bApres = aTmp < rSttIdx;
        SwNode2Layout aNode2Layout( *pNd, rSttIdx.GetIndex() );
        SwFrm* pFrm;
        while( 0 != (pFrm = aNode2Layout.NextFrm()) )
        {
            //Fuer klare Verhaeltnisse sorgen. Wenn ein Prev da ist
            //alle Nachfolger von diesem auf die folgenden Seite
            //schieben
            SwLayoutFrm *pUpper = pFrm->GetUpper();
            SwFtnFrm* pFtnFrm = pUpper->FindFtnFrm();
            BOOL bOldLock, bOldFtn;
            if( pFtnFrm )
            {
                bOldFtn = pFtnFrm->IsColLocked();
                pFtnFrm->ColLock();
            }
            else
                bOldFtn = TRUE;
            SwSectionFrm* pSct = pUpper->FindSctFrm();
            // Es sind innerhalb von Fussnoten nur die Bereiche interessant,
            // die in den Fussnoten liegen, nicht etwa die (spaltigen) Bereiche,
            // in denen die Fussnoten(Container) liegen.
            if( pSct && pFtnFrm && !pSct->IsInFtn() )
                pSct = NULL;
            if( pSct )
            {   // damit der SectionFrm nicht zerstoert wird durch pTmp->MoveFwd()
                bOldLock = pSct->IsColLocked();
                pSct->ColLock();
            }
            else
                bOldLock = TRUE;
            // Wenn pFrm sich nicht bewegen kann, koennen wir auch niemanden
            // auf die naechste Seite schieben. Innerhalb eines Rahmens auch
            // nicht ( in der 1. Spalte eines Rahmens waere pFrm Moveable()! )
            // Auch in spaltigen Bereichen in Tabellen waere pFrm Moveable.
            if ( !pFrm->IsInFly() && pFrm->IsMoveable() &&
                 (!pFrm->IsInTab() || pFrm->IsTabFrm() ) )
            {
                SwFrm *pMove = pFrm;
                SwFrm *pPrev = pFrm->GetPrev();
                SwFlowFrm *pTmp = SwFlowFrm::CastFlowFrm( pMove );
                ASSERT( pTmp, "Missing FlowFrm" );

                if ( bApres )
                {
                    // Wir wollen, dass der Rest der Seite leer ist, d.h.
                    // der naechste muss auf die naechste Seite wandern.
                    // Dieser kann auch in der naechsten Spalte stehen!
                    ASSERT( !pTmp->HasFollow(), "Follows forbidden" );
                    pPrev = pFrm;
                    // Wenn unser umgebender SectionFrm einen Next besitzt,
                    // so soll dieser ebenfalls gemoved werden!
                    pMove = pFrm->GetIndNext();
                    SwColumnFrm* pCol = (SwColumnFrm*)pFrm->FindColFrm();
                    if( pCol )
                        pCol = (SwColumnFrm*)pCol->GetNext();
                    do
                    {
                        if( pCol && !pMove )
                        {   // Bisher haben wir keinen Nachfolger gefunden
                            // jetzt gucken wir in die naechste Spalte
                            pMove = pCol->ContainsAny();
                            if( pCol->GetNext() )
                                pCol = (SwColumnFrm*)pCol->GetNext();
                            else if( pCol->IsInSct() )
                            {   // Wenn es keine naechste Spalte gibt, wir aber
                                // innerhalb eines spaltigen Bereichs sind,
                                // koennte es noch ausserhalb des Bereich
                                // (Seiten-)Spalten geben
                                pCol = (SwColumnFrm*)pCol->FindSctFrm()->FindColFrm();
                                if( pCol )
                                    pCol = (SwColumnFrm*)pCol->GetNext();
                            }
                            else
                                pCol = NULL;
                        }
                        // Falls hier verschrottete SectionFrms herumgammeln,
                        // muessen diese uebersprungen werden.
                        while( pMove && pMove->IsSctFrm() &&
                               !((SwSectionFrm*)pMove)->GetSection() )
                            pMove = pMove->GetNext();
                    } while( !pMove && pCol );

                    if( pMove )
                    {
                        if ( pMove->IsCntntFrm() )
                            pTmp = (SwCntntFrm*)pMove;
                        else if ( pMove->IsTabFrm() )
                            pTmp = (SwTabFrm*)pMove;
                        else if ( pMove->IsSctFrm() )
                        {
                            pMove = ((SwSectionFrm*)pMove)->ContainsAny();
                            if( pMove )
                                pTmp = SwFlowFrm::CastFlowFrm( pMove );
                            else
                                pTmp = NULL;
                        }
                    }
                    else
                        pTmp = 0;
                }
                else
                {
                    ASSERT( !pTmp->IsFollow(), "Follows really forbidden" );
                    // Bei Bereichen muss natuerlich der Inhalt auf die Reise
                    // geschickt werden.
                    if( pMove->IsSctFrm() )
                    {
                        while( pMove && pMove->IsSctFrm() &&
                               !((SwSectionFrm*)pMove)->GetSection() )
                            pMove = pMove->GetNext();
                        if( pMove && pMove->IsSctFrm() )
                            pMove = ((SwSectionFrm*)pMove)->ContainsAny();
                        if( pMove )
                            pTmp = SwFlowFrm::CastFlowFrm( pMove );
                        else
                            pTmp = NULL;
                    }
                }

                if( pTmp )
                {
                    SwFrm* pOldUp = pTmp->GetFrm()->GetUpper();
                    // MoveFwd==TRUE bedeutet, dass wir auf der gleichen
                    // Seite geblieben sind, wir wollen aber die Seite wechseln,
                    // sofern dies moeglich ist
                    BOOL bOldLock = pTmp->IsJoinLocked();
                    pTmp->LockJoin();
                    while( pTmp->MoveFwd( TRUE, FALSE, TRUE ) )
                    {
                        if( pOldUp == pTmp->GetFrm()->GetUpper() )
                            break;
                        pOldUp = pTmp->GetFrm()->GetUpper();
                    }
                    if( !bOldLock )
                        pTmp->UnlockJoin();
                }
                ::_InsertCnt( pUpper, pDoc, rSttIdx.GetIndex(),
                              pFrm->IsInDocBody(), nEndIdx, pPrev );
            }
            else
            {
                BOOL bSplit;
                SwFrm* pPrv = bApres ? pFrm : pFrm->GetPrev();
                // Wenn in einen SectionFrm ein anderer eingefuegt wird,
                // muss dieser aufgebrochen werden
                if( pSct && rSttIdx.GetNode().IsSectionNode() )
                {
                    bSplit = pSct->SplitSect( pFrm, bApres );
                    // Wenn pSct nicht aufgespalten werden konnte
                    if( !bSplit && !bApres )
                    {
                        pUpper = pSct->GetUpper();
                        pPrv = pSct->GetPrev();
                    }
                }
                else
                    bSplit = FALSE;
                ::_InsertCnt( pUpper, pDoc, rSttIdx.GetIndex(), FALSE, nEndIdx,
                              pPrv );
                // Wenn nichts eingefuegt wurde, z.B. ein ausgeblendeter Bereich,
                // muss das Splitten rueckgaengig gemacht werden
                if( bSplit && pSct && pSct->GetNext()
                    && pSct->GetNext()->IsSctFrm() )
                    pSct->MergeNext( (SwSectionFrm*)pSct->GetNext() );
                if( pFrm->IsInFly() )
                    pFrm->FindFlyFrm()->_Invalidate();
                if( pFrm->IsInTab() )
                    pFrm->InvalidateSize();
            }

            SwPageFrm *pPage = pUpper->FindPageFrm();
            SwFrm::CheckPageDescs( pPage, FALSE );
            if( !bOldFtn )
                pFtnFrm->ColUnlock();
            if( !bOldLock )
            {
                pSct->ColUnlock();
                // Zum Beispiel beim Einfuegen von gelinkten Bereichen,
                // die wiederum Bereiche enthalten, kann pSct jetzt leer sein
                // und damit ruhig zerstoert werden.
                if( !pSct->ContainsCntnt() )
                {
                    pSct->DelEmpty( TRUE );
                    pDoc->GetRootFrm()->RemoveFromList( pSct );
                    delete pSct;
                }
            }
        }
    }

    bObjsDirect = TRUE;
}

/*************************************************************************
|*
|*  SwBorderAttrs::Ctor, DTor
|*
|*  Ersterstellung      MA 19. May. 93
|*  Letzte Aenderung    MA 25. Jan. 97
|*
|*************************************************************************/

SwBorderAttrs::SwBorderAttrs( const SwModify *pMod, const SwFrm *pConstructor ) :
    SwCacheObj( pMod ),
    rAttrSet( pConstructor->IsCntntFrm()
                    ? ((SwCntntFrm*)pConstructor)->GetNode()->GetSwAttrSet()
                    : ((SwLayoutFrm*)pConstructor)->GetFmt()->GetAttrSet() ),
    rUL     ( rAttrSet.GetULSpace() ),
    rLR     ( rAttrSet.GetLRSpace() ),
    rBox    ( rAttrSet.GetBox()     ),
    rShadow ( rAttrSet.GetShadow()  ),
    aFrmSize( rAttrSet.GetFrmSize().GetSize() )
{
    //Achtung: Die USHORTs fuer die gecache'ten Werte werden absichtlich
    //nicht initialisiert!

    //Muessen alle einmal berechnet werden:
    bTopLine = bBottomLine = bLeftLine = bRightLine =
    bTop     = bBottom     = bRight     = bLine  = TRUE;

    bCacheGetLine = bCachedGetTopLine = bCachedGetBottomLine = FALSE;

    bBorderDist = 0 != (pConstructor->GetType() & (FRM_CELL));
}

SwBorderAttrs::~SwBorderAttrs()
{
    ((SwModify*)pOwner)->SetInCache( FALSE );
}

/*************************************************************************
|*
|*  SwBorderAttrs::CalcTop(), CalcBottom(), CalcLeft(), CalcRight()
|*
|*  Beschreibung        Die Calc-Methoden errechnen zusaetzlich zu den
|*      von den Attributen vorgegebenen Groessen einen Sicherheitsabstand.
|*      der Sicherheitsabstand wird nur einkalkuliert, wenn Umrandung und/oder
|*      Schatten im Spiel sind; er soll vermeiden, dass aufgrund der
|*      groben physikalischen Gegebenheiten Raender usw. uebermalt werden.
|*  Ersterstellung      MA 19. May. 93
|*  Letzte Aenderung    MA 08. Jul. 93
|*
|*************************************************************************/

void SwBorderAttrs::_CalcTop()
{
    nTop = CalcTopLine() + rUL.GetUpper();
    bTop = FALSE;
}

void SwBorderAttrs::_CalcBottom()
{
    nBottom = CalcBottomLine() + rUL.GetLower();
    bBottom = FALSE;
}

long SwBorderAttrs::CalcLeft( const SwFrm *pCaller ) const
{
    long nLeft = rLR.GetLeft() + CalcLeftLine();
#ifdef NUM_RELSPACE
    if ( pCaller->IsTxtFrm() )
        nLeft += ((SwTxtFrm*)pCaller)->GetTxtNode()->GetLeftMarginWithNum();
#endif
    return nLeft;
}

void SwBorderAttrs::_CalcRight()
{
    nRight = CalcRightLine() + rLR.GetRight();
    bRight = FALSE;
}

/*************************************************************************
|*
|*  SwBorderAttrs::CalcTopLine(), CalcBottomLine(),
|*                 CalcLeftLine(), CalcRightLine()
|*
|*  Beschreibung        Berechnung der Groessen fuer Umrandung und Schatten.
|*                      Es kann auch ohne Linien ein Abstand erwuenscht sein,
|*                      dieser wird  dann nicht vom Attribut sondern hier
|*                      beruecksichtigt (bBorderDist, z.B. fuer Zellen).
|*  Ersterstellung      MA 21. May. 93
|*  Letzte Aenderung    MA 07. Jun. 99
|*
|*************************************************************************/

void SwBorderAttrs::_CalcTopLine()
{
    nTopLine = (bBorderDist && !rBox.GetTop())
                            ? rBox.GetDistance  (BOX_LINE_TOP)
                            : rBox.CalcLineSpace(BOX_LINE_TOP);
    nTopLine += rShadow.CalcShadowSpace(SHADOW_TOP);
    bTopLine = FALSE;
}

void SwBorderAttrs::_CalcBottomLine()
{
    nBottomLine = (bBorderDist && !rBox.GetBottom())
                            ? rBox.GetDistance  (BOX_LINE_BOTTOM)
                            : rBox.CalcLineSpace(BOX_LINE_BOTTOM);
    nBottomLine += rShadow.CalcShadowSpace(SHADOW_BOTTOM);
    bBottomLine = FALSE;
}

void SwBorderAttrs::_CalcLeftLine()
{
    nLeftLine = (bBorderDist && !rBox.GetLeft())
                            ? rBox.GetDistance  (BOX_LINE_LEFT)
                            : rBox.CalcLineSpace(BOX_LINE_LEFT);
    nLeftLine += rShadow.CalcShadowSpace(SHADOW_LEFT);
    bLeftLine = FALSE;
}

void SwBorderAttrs::_CalcRightLine()
{
    nRightLine = (bBorderDist && !rBox.GetRight())
                            ? rBox.GetDistance  (BOX_LINE_RIGHT)
                            : rBox.CalcLineSpace(BOX_LINE_RIGHT);
    nRightLine += rShadow.CalcShadowSpace(SHADOW_RIGHT);
    bRightLine = FALSE;
}

/*************************************************************************
|*
|*  SwBorderAttrs::_IsLine()
|*
|*  Ersterstellung      MA 29. Sep. 94
|*  Letzte Aenderung    MA 29. Sep. 94
|*
|*************************************************************************/

void SwBorderAttrs::_IsLine()
{
    bIsLine = rBox.GetTop() || rBox.GetBottom() ||
              rBox.GetLeft()|| rBox.GetRight();
    bLine = FALSE;
}

/*************************************************************************
|*
|*  SwBorderAttrs::CmpLeftRightLine(), IsTopLine(), IsBottomLine()
|*
|*      Die Umrandungen benachbarter Absaetze werden nach folgendem
|*      Algorithmus zusammengefasst:
|*
|*      1. Die Umrandung oben faellt weg, wenn der Vorgaenger dieselbe
|*         Umrandung oben aufweist und 3. Zutrifft.
|*         Zusaetzlich muss der Absatz mindestens rechts oder links oder
|*         unten eine Umrandung haben.
|*      2. Die Umrandung unten faellt weg, wenn der Nachfolger dieselbe
|*         Umrandung untern aufweist und 3. Zustrifft.
|*         Zusaetzlich muss der Absatz mindestens rechts oder links oder
|*         oben eine Umrandung haben.
|*      3. Die Umrandungen links und rechts vor Vorgaenger bzw. Nachfolger
|*         sind identisch.
|*
|*  Ersterstellung      MA 22. Mar. 95
|*  Letzte Aenderung    MA 22. May. 95
|*
|*************************************************************************/
inline int CmpLines( const SvxBorderLine *pL1, const SvxBorderLine *pL2 )
{
    return ( ((pL1 && pL2) && (*pL1 == *pL2)) || (!pL1 && !pL2) );
}

BOOL SwBorderAttrs::CmpLeftRight( const SwBorderAttrs &rAttrs,
                                  const SwFrm *pCaller,
                                  const SwFrm *pCmp ) const
{
    return ( CmpLines( rAttrs.GetBox().GetLeft(), GetBox().GetLeft()  ) &&
             CmpLines( rAttrs.GetBox().GetRight(),GetBox().GetRight() ) &&
             CalcLeft( pCaller ) == rAttrs.CalcLeft( pCmp ) &&  //Bereucksichtigt TxtFrms ueber den Node
             rLR.GetRight() == rAttrs.GetLRSpace().GetRight() );
}

void SwBorderAttrs::_GetTopLine( const SwFrm *pFrm )
{
    USHORT nRet = CalcTopLine();
    if ( nRet && pFrm->GetPrev() && pFrm->IsCntntFrm() && pFrm->GetPrev()->IsCntntFrm() )
    {
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm->GetPrev() );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( nRet == rAttrs.CalcTopLine() )
        {
            if ( (GetBox().GetLeft() || GetBox().GetRight() || GetBox().GetBottom()) &&
                 rAttrs.GetShadow() == rShadow   &&
                 CmpLines( rAttrs.GetBox().GetTop(), rBox.GetTop() ) &&
                 CmpLeftRight( rAttrs, pFrm, pFrm->GetPrev() ) )
            {
                nRet = 0;
            }
        }
    }
    if ( bCacheGetLine )
        bCachedGetTopLine = TRUE;

    nGetTopLine = nRet;
}

void SwBorderAttrs::_GetBottomLine( const SwFrm *pFrm )
{
    USHORT nRet = CalcBottomLine();
    if ( nRet && pFrm->GetNext() && pFrm->IsCntntFrm() && pFrm->GetNext()->IsCntntFrm() )
    {
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm->GetNext() );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( nRet == rAttrs.CalcBottomLine() )
        {
            if ( (GetBox().GetLeft() || GetBox().GetRight() || GetBox().GetTop()) &&
                 rAttrs.GetShadow() == rShadow   &&
                 CmpLines( rAttrs.GetBox().GetBottom(), rBox.GetBottom() ) &&
                 CmpLeftRight( rAttrs, pFrm, pFrm->GetNext() ) )
            {
                nRet = 0;
            }
        }
    }
    if ( bCacheGetLine )
        bCachedGetBottomLine = TRUE;

    nGetBottomLine = nRet;
}

/*************************************************************************
|*
|*  SwBorderAttrAccess::CTor
|*
|*  Ersterstellung      MA 20. Mar. 95
|*  Letzte Aenderung    MA 29. Nov. 95
|*
|*************************************************************************/

SwBorderAttrAccess::SwBorderAttrAccess( SwCache &rCache, const SwFrm *pFrm ) :
    SwCacheAccess( rCache, (pFrm->IsCntntFrm() ?
                                (void*)((SwCntntFrm*)pFrm)->GetNode() :
                                (void*)((SwLayoutFrm*)pFrm)->GetFmt()),
                           (BOOL)(pFrm->IsCntntFrm() ?
                (BOOL)((SwModify*)((SwCntntFrm*)pFrm)->GetNode())->IsInCache() :
                (BOOL)((SwModify*)((SwLayoutFrm*)pFrm)->GetFmt())->IsInCache()) ),
    pConstructor( pFrm )
{
}

/*************************************************************************
|*
|*  SwBorderAttrAccess::NewObj, Get
|*
|*  Ersterstellung      MA 20. Mar. 95
|*  Letzte Aenderung    MA 20. Mar. 95
|*
|*************************************************************************/

SwCacheObj *SwBorderAttrAccess::NewObj()
{
    ((SwModify*)pOwner)->SetInCache( TRUE );
    return new SwBorderAttrs( (SwModify*)pOwner, pConstructor );
}

SwBorderAttrs *SwBorderAttrAccess::Get()
{
    return (SwBorderAttrs*)SwCacheAccess::Get();
}

/*************************************************************************
|*
|*  SwOrderIter::Ctor
|*
|*  Ersterstellung      MA 06. Jan. 95
|*  Letzte Aenderung    MA 22. Nov. 95
|*
|*************************************************************************/

SwOrderIter::SwOrderIter( const SwPageFrm *pPg, FASTBOOL bFlys ) :
    pPage( pPg ),
    pCurrent( 0 ),
    bFlysOnly( bFlys )
{
}

/*************************************************************************
|*
|*  SwOrderIter::Top()
|*
|*  Ersterstellung      MA 06. Jan. 95
|*  Letzte Aenderung    MA 22. Nov. 95
|*
|*************************************************************************/

const SdrObject *SwOrderIter::Top()
{
    pCurrent = 0;
    if ( pPage->GetSortedObjs() )
    {
        UINT32 nTopOrd = 0;
        const SwSortDrawObjs *pObjs = pPage->GetSortedObjs();
        if ( pObjs->Count() )
        {
            (*pObjs)[0]->GetOrdNum();   //Aktualisieren erzwingen!
            for ( USHORT i = 0; i < pObjs->Count(); ++i )
            {
                const SdrObject *pObj = (*pObjs)[i];
                if ( bFlysOnly && !pObj->IsWriterFlyFrame() )
                    continue;
                UINT32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp >= nTopOrd )
                {
                    nTopOrd = nTmp;
                    pCurrent = pObj;
                }
            }
        }
    }
    return pCurrent;
}

/*************************************************************************
|*
|*  SwOrderIter::Bottom()
|*
|*  Ersterstellung      MA 06. Jan. 95
|*  Letzte Aenderung    MA 22. Nov. 95
|*
|*************************************************************************/

const SdrObject *SwOrderIter::Bottom()
{
    pCurrent = 0;
    if ( pPage->GetSortedObjs() )
    {
        UINT32 nBotOrd = USHRT_MAX;
        const SwSortDrawObjs *pObjs = pPage->GetSortedObjs();
        if ( pObjs->Count() )
        {
            (*pObjs)[0]->GetOrdNum();   //Aktualisieren erzwingen!
            for ( USHORT i = 0; i < pObjs->Count(); ++i )
            {
                SdrObject *pObj = (*pObjs)[i];
                if ( bFlysOnly && !pObj->IsWriterFlyFrame() )
                    continue;
                UINT32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp < nBotOrd )
                {
                    nBotOrd = nTmp;
                    pCurrent = pObj;
                }
            }
        }
    }
    return pCurrent;
}

/*************************************************************************
|*
|*  SwOrderIter::Next()
|*
|*  Ersterstellung      MA 06. Jan. 95
|*  Letzte Aenderung    MA 22. Nov. 95
|*
|*************************************************************************/

const SdrObject *SwOrderIter::Next()
{
    const UINT32 nCurOrd = pCurrent ? pCurrent->GetOrdNumDirect() : 0;
    pCurrent = 0;
    if ( pPage->GetSortedObjs() )
    {
        UINT32 nOrd = USHRT_MAX;
        const SwSortDrawObjs *pObjs = pPage->GetSortedObjs();
        if ( pObjs->Count() )
        {
            (*pObjs)[0]->GetOrdNum();   //Aktualisieren erzwingen!
            for ( USHORT i = 0; i < pObjs->Count(); ++i )
            {
                SdrObject *pObj = (*pObjs)[i];
                if ( bFlysOnly && !pObj->IsWriterFlyFrame() )
                    continue;
                UINT32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp > nCurOrd && nTmp < nOrd )
                {
                    nOrd = nTmp;
                    pCurrent = pObj;
                }
            }
        }
    }
    return pCurrent;
}

/*************************************************************************
|*
|*  SwOrderIter::Prev()
|*
|*  Ersterstellung      MA 06. Jan. 95
|*  Letzte Aenderung    MA 22. Nov. 95
|*
|*************************************************************************/

const SdrObject *SwOrderIter::Prev()
{
    const UINT32 nCurOrd = pCurrent ? pCurrent->GetOrdNumDirect() : 0;
    pCurrent = 0;
    if ( pPage->GetSortedObjs() )
    {
        UINT32 nOrd = 0;
        const SwSortDrawObjs *pObjs = pPage->GetSortedObjs();
        if ( pObjs->Count() )
        {
            (*pObjs)[0]->GetOrdNum();   //Aktualisieren erzwingen!
            for ( USHORT i = 0; i < pObjs->Count(); ++i )
            {
                SdrObject *pObj = (*pObjs)[i];
                if ( bFlysOnly && !pObj->IsWriterFlyFrame() )
                    continue;
                UINT32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp < nCurOrd && nTmp >= nOrd )
                {
                    nOrd = nTmp;
                    pCurrent = pObj;
                }
            }
        }
    }
    return pCurrent;
}

/*************************************************************************
|*
|*  SaveCntnt(), RestoreCntnt()
|*
|*  Ersterstellung      MA 10. Jun. 93
|*  Letzte Aenderung    MA 07. Mar. 95
|*
|*************************************************************************/

//Unterstruktur eines LayoutFrms fuer eine Aktion aufheben und wieder
//restaurieren.
//Neuer Algorithmus: Es ist unuetz jeden Nachbarn einzeln zu betrachten und
//die Pointer sauber zu setzen (Upper, Nachbarn, usw.)
//Es reicht vollkommen jeweils eine Einzelkette zu loesen, und mit dem
//Letzen der Einzelkette nachzuschauen ob noch eine weitere Kette
//angeheangt werden muss. Es brauchen nur die Pointer korrigiert werden,
//die zur Verkettung notwendig sind. So koennen Beipspielsweise die Pointer
//auf die Upper auf den alten Uppern stehenbleiben. Korrigiert werden die
//Pointer dann im RestoreCntnt. Zwischenzeitlich ist sowieso jeder Zugriff
//verboten.
//Unterwegs werden die Flys bei der Seite abgemeldet.

void MA_FASTCALL lcl_RemoveFlysFromPage( SwCntntFrm *pCntnt )
{
    ASSERT( pCntnt->GetDrawObjs(), "Keine DrawObjs fuer lcl_RemoveFlysFromPage." );
    SwDrawObjs &rObjs = *pCntnt->GetDrawObjs();
    for ( USHORT i = 0; i < rObjs.Count(); ++i )
    {
        SdrObject *pO = rObjs[i];
        SwVirtFlyDrawObj *pObj = pO->IsWriterFlyFrame() ?
                                                        (SwVirtFlyDrawObj*)pO : 0;
        if ( pObj && pObj->GetFlyFrm()->IsFlyFreeFrm() )
        {
            SwCntntFrm *pCnt = pObj->GetFlyFrm()->ContainsCntnt();
            while ( pCnt )
            {
                if ( pCnt->GetDrawObjs() )
                    ::lcl_RemoveFlysFromPage( pCnt );
                pCnt = pCnt->GetNextCntntFrm();
            }
            ((SwFlyFreeFrm*)pObj->GetFlyFrm())->GetPage()->
                    SwPageFrm::RemoveFly( pObj->GetFlyFrm() );
        }
    }
}

SwFrm *SaveCntnt( SwLayoutFrm *pLay, SwFrm *pStart )
{
    if( pLay->IsSctFrm() && pLay->Lower() && pLay->Lower()->IsColumnFrm() )
        lcl_RemoveFtns( (SwColumnFrm*)pLay->Lower(), TRUE, TRUE );

    SwFrm *pSav;
    if ( 0 == (pSav = pLay->ContainsAny()) )
        return 0;

    if( pSav->IsInFtn() && !pLay->IsInFtn() )
    {
        do
            pSav = pSav->FindNext();
        while( pSav && pSav->IsInFtn() );
        if( !pSav || !pLay->IsAnLower( pSav ) )
            return NULL;
    }
    // Tabellen sollen immer komplett gesichert werden, es sei denn, es wird
    // der Inhalt eines Bereichs innerhalb einer Tabelle gesichert.
    if ( pSav->IsInTab() && !( pLay->IsSctFrm() && pLay->IsInTab() ) )
        while ( !pSav->IsTabFrm() )
            pSav = pSav->GetUpper();

    if( pSav->IsInSct() )
    { // Jetzt wird der oberste Bereich gesucht, der innerhalb von pLay ist.
        SwFrm* pSect = pLay->FindSctFrm();
        SwFrm *pTmp = pSav;
        do
        {
            pSav = pTmp;
            pTmp = pSav->GetUpper() ? pSav->GetUpper()->FindSctFrm() : NULL;
        } while ( pTmp != pSect );
    }

    SwFrm *pFloat = pSav;
    if( !pStart )
        pStart = pSav;
    BOOL bGo = pStart == pSav;
    do
    {
        if( bGo )
            pFloat->GetUpper()->pLower = 0;     //Die Teilkette ausklinken.

        //Das Ende der Teilkette suchen, unterwegs die Flys abmelden.
        do
        {
            if( bGo )
            {
                if ( pFloat->IsCntntFrm() )
                {
                    if ( pFloat->GetDrawObjs() )
                        ::lcl_RemoveFlysFromPage( (SwCntntFrm*)pFloat );
                }
                else if ( pFloat->IsTabFrm() || pFloat->IsSctFrm() )
                {
                    SwCntntFrm *pCnt = ((SwLayoutFrm*)pFloat)->ContainsCntnt();
                    if( pCnt )
                    {
                        do
                        {   if ( pCnt->GetDrawObjs() )
                                ::lcl_RemoveFlysFromPage( pCnt );
                            pCnt = pCnt->GetNextCntntFrm();
                        } while ( pCnt && ((SwLayoutFrm*)pFloat)->IsAnLower( pCnt ) );
                    }
                }
                else
                    ASSERT( !pFloat, "Neuer Float-Frame?" );
            }
            if ( pFloat->GetNext()  )
            {
                if( bGo )
                    pFloat->pUpper = NULL;
                pFloat = pFloat->GetNext();
                if( !bGo && pFloat == pStart )
                {
                    bGo = TRUE;
                    pFloat->pPrev->pNext = NULL;
                    pFloat->pPrev = NULL;
                }
            }
            else
                break;

        } while ( pFloat );

        //Die naechste Teilkette suchen und die Ketten miteinander verbinden.
        SwFrm *pTmp = pFloat->FindNext();
        if( bGo )
            pFloat->pUpper = NULL;

        if( !pLay->IsInFtn() )
            while( pTmp && pTmp->IsInFtn() )
                pTmp = pTmp->FindNext();

        if ( !pLay->IsAnLower( pTmp ) )
            pTmp = 0;

        if ( pTmp && bGo )
        {
            pFloat->pNext = pTmp;           //Die beiden Ketten verbinden.
            pFloat->pNext->pPrev = pFloat;
        }
        pFloat = pTmp;
        bGo = bGo || ( pStart == pFloat );
    }  while ( pFloat );

    return bGo ? pStart : NULL;
}

void MA_FASTCALL lcl_AddFlysToPage( SwCntntFrm *pCntnt, SwPageFrm *pPage )
{
    ASSERT( pCntnt->GetDrawObjs(), "Keine DrawObjs fuer lcl_AddFlysToPage." );
    SwDrawObjs &rObjs = *pCntnt->GetDrawObjs();
    for ( USHORT i = 0; i < rObjs.Count(); ++i )
    {
        SdrObject *pO = rObjs[i];
        SwVirtFlyDrawObj *pObj = pO->IsWriterFlyFrame() ?
                                                        (SwVirtFlyDrawObj*)pO : 0;
        if ( pObj && pObj->GetFlyFrm()->IsFlyFreeFrm() )
        {
            SwFlyFrm *pFly = pObj->GetFlyFrm();
            pPage->SwPageFrm::AppendFly( pFly );
            pFly->_InvalidatePos();
            pFly->_InvalidateSize();
            pFly->InvalidatePage( pPage );
            SwCntntFrm *pCnt = pFly->ContainsCntnt();
            while ( pCnt )
            {
                if ( pCnt->GetDrawObjs() )
                    ::lcl_AddFlysToPage( pCnt, pPage );
                pCnt = pCnt->GetNextCntntFrm();
            }
        }
    }
}

void RestoreCntnt( SwFrm *pSav, SwLayoutFrm *pParent, SwFrm *pSibling )
{
    ASSERT( pSav && pParent, "Kein Save oder Parent fuer Restore." );

    //Wenn es bereits FlowFrms unterhalb des neuen Parent gibt, so wird die
    //Kette, beginnend mit pSav,  hinter dem letzten angehaengt.
    //Die Teile werden kurzerhand insertet und geeignet invalidiert.
    //Unterwegs werden die Flys der CntntFrms bei der Seite angemeldet.

    SwPageFrm *pPage = pParent->FindPageFrm();

    if ( pPage )
        pPage->InvalidatePage( pPage ); //Invalides Layout anmelden.

    //Vorgaenger festellen und die Verbindung herstellen bzw. initialisieren.
    pSav->pPrev = pSibling;
    SwFrm* pNxt;
    if ( pSibling )
    {
        pNxt = pSibling->pNext;
        pSibling->pNext = pSav;
        pSibling->_InvalidatePrt();
        ((SwCntntFrm*)pSibling)->InvalidatePage( pPage );//Invaliden Cntnt anmelden.
        if ( ((SwCntntFrm*)pSibling)->GetFollow() )
            pSibling->Prepare( PREP_CLEAR, 0, sal_False );
    }
    else
    {   pNxt = pParent->pLower;
        pParent->pLower = pSav;
        pSav->pUpper = pParent;     //Schon mal setzen, sonst ist fuer das
                                    //invalidate der Parent (z.B. ein Fly) nicht klar.
        //Invaliden Cntnt anmelden.
        if ( pSav->IsCntntFrm() )
            ((SwCntntFrm*)pSav)->InvalidatePage( pPage );
        else
        {   // pSav koennte auch ein leerer SectFrm sein
            SwCntntFrm* pCnt = pParent->ContainsCntnt();
            if( pCnt )
                pCnt->InvalidatePage( pPage );
        }
    }

    //Der Parent muss entsprechend gegrow'ed werden.
    SwTwips nGrowVal = 0;
    SwFrm* pLast;
    do
    {   pSav->pUpper = pParent;
        nGrowVal += pSav->Frm().Height();
        pSav->_InvalidateAll();

        //Jetzt die Flys anmelden, fuer TxtFrms gleich geeignet invalidieren.
        if ( pSav->IsCntntFrm() )
        {
            if ( pSav->IsTxtFrm() &&
                 ((SwTxtFrm*)pSav)->GetCacheIdx() != USHRT_MAX )
                ((SwTxtFrm*)pSav)->Init();  //Ich bin sein Freund.

            if ( pPage && pSav->GetDrawObjs() )
                ::lcl_AddFlysToPage( (SwCntntFrm*)pSav, pPage );
        }
        else
        {   SwCntntFrm *pBlub = ((SwLayoutFrm*)pSav)->ContainsCntnt();
            if( pBlub )
            {
                do
                {   if ( pPage && pBlub->GetDrawObjs() )
                        ::lcl_AddFlysToPage( pBlub, pPage );
                    if( pBlub->IsTxtFrm() && ((SwTxtFrm*)pBlub)->HasFtn() &&
                         ((SwTxtFrm*)pBlub)->GetCacheIdx() != USHRT_MAX )
                        ((SwTxtFrm*)pBlub)->Init(); //Ich bin sein Freund.
                    pBlub = pBlub->GetNextCntntFrm();
                } while ( pBlub && ((SwLayoutFrm*)pSav)->IsAnLower( pBlub ));
            }
        }
        pLast = pSav;
        pSav = pSav->GetNext();

    } while ( pSav );

    if( pNxt )
    {
        pLast->pNext = pNxt;
        pNxt->pPrev = pLast;
    }
    pParent->Grow( nGrowVal, pHeight );
}

/*************************************************************************
|*
|*  SqRt()              Berechnung der Quadratwurzel, damit die math.lib
|*      nicht auch noch dazugelinkt werden muss.
|*
|*  Ersterstellung      OK ??
|*  Letzte Aenderung    MA 09. Jan. 97
|*
|*************************************************************************/

ULONG MA_FASTCALL SqRt( BigInt nX )
{
    BigInt nErg = 1;

    if ( !nX.IsNeg() )
    {
        BigInt nOldErg = 1;
        for ( int i = 0; i <= 5; i++ )
        {
            nErg = (nOldErg + (nX / nOldErg)) / BigInt(2);
            nOldErg = nErg;
        }
    }
    return nErg >= BigInt((ULONG)ULONG_MAX) ? ULONG_MAX : (ULONG)nErg;
}

/*************************************************************************
|*
|*  InsertNewPage()     Einsetzen einer neuen Seite.
|*
|*  Ersterstellung      MA 01. Jul. 93
|*  Letzte Aenderung    MA 31. Jul. 95
|*
|*************************************************************************/

SwPageFrm * MA_FASTCALL InsertNewPage( SwPageDesc &rDesc, SwFrm *pUpper,
                          BOOL bOdd, BOOL bInsertEmpty, BOOL bFtn,
                          SwFrm *pSibling )
{
    SwPageFrm *pRet;
    SwDoc *pDoc = ((SwLayoutFrm*)pUpper)->GetFmt()->GetDoc();
    SwFrmFmt *pFmt = bOdd ? rDesc.GetRightFmt() : rDesc.GetLeftFmt();
    //Wenn ich kein FrmFmt fuer die Seite gefunden habe, muss ich eben
    //eine Leerseite einfuegen.
    if ( !pFmt )
    {
        pFmt = bOdd ? rDesc.GetLeftFmt() : rDesc.GetRightFmt();
        ASSERT( pFmt, "Descriptor without any format?!" );
        bInsertEmpty = !bInsertEmpty;
    }
    if( bInsertEmpty )
    {
        SwPageDesc *pTmpDesc = pSibling && pSibling->GetPrev() ?
                ((SwPageFrm*)pSibling->GetPrev())->GetPageDesc() : &rDesc;
        pRet = new SwPageFrm( pDoc->GetEmptyPageFmt(), pTmpDesc );
        pRet->Paste( pUpper, pSibling );
        pRet->PreparePage( bFtn );
    }
    pRet = new SwPageFrm( pFmt, &rDesc );
    pRet->Paste( pUpper, pSibling );
    pRet->PreparePage( bFtn );
    if ( pRet->GetNext() )
        ((SwRootFrm*)pRet->GetUpper())->AssertPageFlys( pRet );
    return pRet;
}


/*************************************************************************
|*
|*  RegistFlys(), Regist()  Die beiden folgenden Methoden durchsuchen rekursiv
|*      eine Layoutstruktur und melden alle FlyFrms, die einen beliebigen Frm
|*      innerhalb der Struktur als Anker haben bei der Seite an.
|*
|*  Ersterstellung      MA 08. Jul. 93
|*  Letzte Aenderung    MA 07. Jul. 95
|*
|*************************************************************************/

void MA_FASTCALL lcl_Regist( SwPageFrm *pPage, const SwFrm *pAnch )
{
    SwDrawObjs *pObjs = (SwDrawObjs*)pAnch->GetDrawObjs();
    for ( USHORT i = 0; i < pObjs->Count(); ++i )
    {
        SdrObject *pObj = (*pObjs)[i];
        SwVirtFlyDrawObj *pFObj = pObj->IsWriterFlyFrame() ?
                                                    (SwVirtFlyDrawObj*)pObj : 0;
        if ( pFObj )
        {
            SwFlyFrm *pFly = pFObj->GetFlyFrm();
            //Ggf. ummelden, nicht anmelden wenn bereits bekannt.
            SwPageFrm *pPg = pFly->FindPageFrm();
            if ( pPg != pPage )
            {
                if ( pPg )
                    pPg->SwPageFrm::RemoveFly( pFly );
                pPage->AppendFly( pFly );
            }
            ::RegistFlys( pPage, pFly );
        }
        else
        {
            SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
            if ( pContact->GetPage() != pPage )
            {
                if ( pContact->GetPage() )
                    pContact->GetPage()->SwPageFrm::RemoveDrawObj( pContact );
                pPage->AppendDrawObj( pContact );
            }
        }

        const SwFlyFrm *pFly = pAnch->FindFlyFrm();
        if ( pFly && pObj->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() )
            pObj->GetPage()->SetObjectOrdNum( pObj->GetOrdNumDirect(),
                                    pFly->GetVirtDrawObj()->GetOrdNumDirect() + 1 );
    }
}

void RegistFlys( SwPageFrm *pPage, const SwLayoutFrm *pLay )
{
    if ( pLay->GetDrawObjs() )
        ::lcl_Regist( pPage, pLay );
    const SwFrm *pFrm = pLay->Lower();
    while ( pFrm )
    {
        if ( pFrm->IsLayoutFrm() )
            ::RegistFlys( pPage, (const SwLayoutFrm*)pFrm );
        else if ( pFrm->GetDrawObjs() )
            ::lcl_Regist( pPage, pFrm );
        pFrm = pFrm->GetNext();
    }
}

/*************************************************************************
|*
|*  void Notify()
|*
|*  Beschreibung        Benachrichtigt den Hintergrund je nach der
|*      Veraenderung zwischen altem und neuem Rechteckt.
|*  Ersterstellung      MA 18. Jun. 93
|*  Letzte Aenderung    MA 06. Jun. 96
|*
|*************************************************************************/

void Notify( SwFlyFrm *pFly, SwPageFrm *pOld, const SwRect &rOld )
{
    const SwRect aFrm( pFly->AddSpacesToFrm() );
    if ( rOld.Pos() != aFrm.Pos() )
    {   //Positionsaenderung, alten und neuen Bereich invalidieren
        if ( rOld.HasArea() &&
             rOld.Left()+pFly->GetFmt()->GetLRSpace().GetLeft() < WEIT_WECH )
        {
            pFly->NotifyBackground( pOld, rOld, PREP_FLY_LEAVE );
        }
        pFly->NotifyBackground( pFly->FindPageFrm(), aFrm, PREP_FLY_ARRIVE );
    }
    else if ( rOld.SSize() != aFrm.SSize() )
    {   //Groessenaenderung, den Bereich der Verlassen wurde bzw. jetzt
        //ueberdeckt wird invalidieren.
        //Der Einfachheit halber wird hier bewusst jeweils ein Twip
        //unnoetig invalidiert.

        ViewShell *pSh = pFly->GetShell();
        if( pSh && rOld.HasArea() )
            pSh->InvalidateWindows( rOld );

        if ( rOld.Left() != aFrm.Left() )
        {   SwRect aTmp( rOld );
            aTmp.Union( aFrm );
            aTmp.Left(  Min(aFrm.Left(), rOld.Left()) );
            aTmp.Right( Max(aFrm.Left(), rOld.Left()) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
        SwTwips nOld = rOld.Right();
        SwTwips nNew = aFrm.Right();
        if ( nOld != nNew )
        {   SwRect aTmp( rOld );
            aTmp.Union( aFrm );
            aTmp.Left(  Min(nNew, nOld) );
            aTmp.Right( Max(nNew, nOld) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
        if ( rOld.Top() != aFrm.Top() )
        {   SwRect aTmp( rOld );
            aTmp.Union( aFrm );
            aTmp.Top(    Min(aFrm.Top(), rOld.Top()) );
            aTmp.Bottom( Max(aFrm.Top(), rOld.Top()) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
        nOld = rOld.Bottom();
        nNew = aFrm.Bottom();
        if ( nOld != nNew )
        {   SwRect aTmp( rOld );
            aTmp.Union( aFrm );
            aTmp.Top(    Min(nNew, nOld) );
            aTmp.Bottom( Max(nNew, nOld) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
    }
}

/*************************************************************************
|*
|*  NotifyBackground()
|*
|*************************************************************************/
#pragma optimize("",off)

void MA_FASTCALL lcl_NotifyCntnt( SdrObject *pThis, SwCntntFrm *pCnt,
    const SwRect &rRect, const PrepareHint eHint )
{
    if ( pCnt->IsTxtFrm() )
    {
        SwRect aCntPrt( pCnt->Prt() );
        aCntPrt.Pos() += pCnt->Frm().Pos();
        if ( eHint == PREP_FLY_ATTR_CHG )
        {
            if ( aCntPrt.IsOver( pThis->GetBoundRect() ) )
                pCnt->Prepare( PREP_FLY_ATTR_CHG );
        }
        else if ( aCntPrt.IsOver( rRect ) || pCnt->IsFollow() || pCnt->HasFollow() )
            pCnt->Prepare( eHint, (void*)&aCntPrt._Intersection( rRect ) );
        if ( pCnt->GetDrawObjs() )
        {
            const SwDrawObjs &rObjs = *pCnt->GetDrawObjs();
            for ( USHORT i = 0; i < rObjs.Count(); ++i )
            {
                SdrObject *pO = rObjs[i];
                if ( pO->IsWriterFlyFrame() )
                {
                    SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                    if ( pFly->IsFlyInCntFrm() )
                    {
                        SwCntntFrm *pCntnt = pFly->ContainsCntnt();
                        while ( pCntnt )
                        {
                            ::lcl_NotifyCntnt( pThis, pCntnt, rRect, eHint );
                            pCntnt = pCntnt->GetNextCntntFrm();
                        }
                    }
                }
            }
        }
    }
}

void Notify_Background( SdrObject *pObj, SwPageFrm *pPage, const SwRect& rRect,
                        const PrepareHint eHint, const BOOL bInva )
{

    //Wenn der Frm gerade erstmalig sinnvoll positioniert wurde, braucht der
    //alte Bereich nicht benachrichtigt werden.
    if ( eHint == PREP_FLY_LEAVE && rRect.Top() == WEIT_WECH )
         return;

    SwLayoutFrm *pArea;
    SwFlyFrm *pFlyFrm = 0;
    SwFrm* pAnchor;
    if( pObj->IsWriterFlyFrame() )
    {
        pFlyFrm = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();

        //MA: Wozu ausserhalb des Ankers invalidieren? Dort wird ja eh nicht
        //auf den Rahmen Ruecksicht genommen; normalerweise kann er dort
        //gar nicht hin, ausser temporaer beim Formatieren.
        pAnchor = pFlyFrm->GetAnchor();
    }
    else
    {
        pFlyFrm = NULL;
        pAnchor = ((SwDrawContact*)GetUserCall(pObj))->GetAnchor();
    }
    if( PREP_FLY_LEAVE != eHint && pAnchor->IsInFly() )
        pArea = pAnchor->FindFlyFrm();
    else
        pArea = pPage;
    SwCntntFrm *pCnt = 0;
    if ( pArea )
    {
        //Es reagieren sowieso nur die auf den Anker folgenden auf den Fly, also
        //brauchen diese nicht abgeklappert werden.
        //Ausnahme sind ist natuerlich das LEAVE, denn der Fly koennte ja von
        //"oben" kommen.
        // Wenn der Anker auf der vorhergehenden Seite liegt, muss ebenfalls
        // die gesamte Seite abgearbeitet werden. (47722)
        if ( PREP_FLY_LEAVE != eHint && pAnchor->IsCntntFrm() &&
            pArea->IsAnLower( pAnchor ) )
            pCnt = (SwCntntFrm*)pAnchor;
        else
            pCnt = pArea->ContainsCntnt();
    }
    SwFrm *pLastTab = 0;

    while ( pCnt && pArea->IsAnLower( pCnt ) )
    {
        ::lcl_NotifyCntnt( pObj, pCnt, rRect, eHint );
        if ( pCnt->IsInTab() )
        {
            SwLayoutFrm* pCell = pCnt->GetUpper();
            if( pCell->IsCellFrm() &&
                ( (pCell->Frm().IsOver( pObj->GetBoundRect() ) ||
                    pCell->Frm().IsOver( rRect )) ) )
            {
                const SwFmtVertOrient &rOri = pCell->GetFmt()->GetVertOrient();
                if ( VERT_NONE != rOri.GetVertOrient() )
                    pCell->InvalidatePrt();
            }
            SwTabFrm *pTab = pCnt->FindTabFrm();
            if ( pTab != pLastTab )
            {
                pLastTab = pTab;
                if ( pTab->Frm().IsOver( pObj->GetBoundRect() ) ||
                        pTab->Frm().IsOver( rRect ) )
                {
                    if ( !pFlyFrm || !pFlyFrm->IsLowerOf( pTab ) )
                        pTab->InvalidatePrt();
                }
            }
        }
        pCnt = pCnt->GetNextCntntFrm();
    }
    if( pPage->Lower() )
    {
        SwFrm* pFrm = pPage->Lower();
        while( pFrm->GetNext() )
            pFrm = pFrm->GetNext();
        if( pFrm->IsFooterFrm() &&
            ( ( pFrm->Frm().IsOver( pObj->GetBoundRect() ) ||
                pFrm->Frm().IsOver( rRect ) ) ) )
            pFrm->InvalidateSize();
    }
    if( pPage->GetSortedObjs() )
    {
        pObj->GetOrdNum();
        const SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
        for ( USHORT i = 0; i < rObjs.Count(); ++i )
        {
            SdrObject *pO = rObjs[i];
            if ( pO->IsWriterFlyFrame() )
            {
                if( pO == pObj )
                    continue;
                SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                if ( pFly->Frm().Top() == WEIT_WECH )
                    continue;

                if ( !pFlyFrm ||
                        (!pFly->IsLowerOf( pFlyFrm ) &&
                        pFly->GetVirtDrawObj()->GetOrdNumDirect() < pObj->GetOrdNumDirect()))
                {
                    pCnt = pFly->ContainsCntnt();
                    while ( pCnt )
                    {
                        ::lcl_NotifyCntnt( pObj, pCnt, rRect, eHint );
                        pCnt = pCnt->GetNextCntntFrm();
                    }
                }
                if( pFly->IsFlyLayFrm() )
                {
                    if( pFly->Lower() && pFly->Lower()->IsColumnFrm() &&
                        pFly->Frm().Bottom() >= rRect.Top() &&
                        pFly->Frm().Top() <= rRect.Bottom() &&
                        pFly->Frm().Right() >= rRect.Left() &&
                        pFly->Frm().Left() <= rRect.Right() )
                     {
                        const SwFmtFrmSize &rSz = pFly->GetFmt()->GetFrmSize();
                        pFly->InvalidateSize();
                     }
                }
                //Flys, die ueber mir liegen muessen/mussten evtl.
                //ausweichen, wenn sie eine automatische Ausrichtung haben.
                //das ist unabhaengig von meinem Attribut, weil dies sich
                //gerade geaendert haben kann und eben deshalb
                //umformatiert wurde.
                else if ( pFly->IsFlyAtCntFrm() &&
                        pObj->GetOrdNumDirect() <
                        pFly->GetVirtDrawObj()->GetOrdNumDirect() &&
                        pFlyFrm && !pFly->IsLowerOf( pFlyFrm ) )
                {
                    const SwFmtHoriOrient &rH = pFly->GetFmt()->GetHoriOrient();
                    if ( HORI_NONE != rH.GetHoriOrient()  &&
                            HORI_CENTER != rH.GetHoriOrient()  &&
                            ( !pFly->IsAutoPos() || REL_CHAR != rH.GetRelationOrient() ) &&
                            (pFly->Frm().Bottom() >= rRect.Top() &&
                            pFly->Frm().Top() <= rRect.Bottom()) )
                        pFly->InvalidatePos();
                }
            }
        }
    }
    if ( pFlyFrm && pAnchor->GetUpper() && pAnchor->IsInTab() )//MA_FLY_HEIGHT
        pAnchor->GetUpper()->InvalidateSize();

    ViewShell *pSh;
    if( bInva && 0 != (pSh = pPage->GetShell()) )
        pSh->InvalidateWindows( rRect );
}

#pragma optimize("",on)

/*************************************************************************
|*
|*  GetVirtualUpper() liefert bei absatzgebundenen Objekten den Upper
|*  des Ankers. Falls es sich dabei um verkettete Rahmen oder
|*  Fussnoten handelt, wird ggf. der "virtuelle" Upper ermittelt.
|*
|*************************************************************************/

const SwFrm* GetVirtualUpper( const SwFrm* pFrm, const Point& rPos )
{
    if( pFrm->IsTxtFrm() )
    {
        pFrm = pFrm->GetUpper();
        if( !pFrm->Frm().IsInside( rPos ) )
        {
            if( pFrm->IsFtnFrm() )
            {
                const SwFtnFrm* pTmp = ((SwFtnFrm*)pFrm)->GetFollow();
                while( pTmp )
                {
                    if( pTmp->Frm().IsInside( rPos ) )
                        return pTmp;
                    pTmp = pTmp->GetFollow();
                }
            }
            else
            {
                SwFlyFrm* pTmp = (SwFlyFrm*)pFrm->FindFlyFrm();
                while( pTmp )
                {
                    if( pTmp->Frm().IsInside( rPos ) )
                        return pTmp;
                    pTmp = pTmp->GetNextLink();
                }
            }
        }
    }
    return pFrm;
}

/*************************************************************************
|*
|*  IsLowerOf()
|*
|*************************************************************************/

BOOL Is_Lower_Of( const SwFrm *pCurrFrm, const SdrObject* pObj )
{
    Point aPos;
    const SwFrm* pFrm;
    if( pObj->IsWriterFlyFrame() )
    {
        const SwFlyFrm* pFly = ( (SwVirtFlyDrawObj*)pObj )->GetFlyFrm();
        pFrm = pFly->GetAnchor();
        aPos = pFly->Frm().Pos();
    }
    else
    {
        pFrm = ( (SwDrawContact*)GetUserCall(pObj) )->GetAnchor();
        aPos = pObj->GetBoundRect().TopLeft();
    }
    ASSERT( pFrm, "8-( Fly is lost in Space." );
    pFrm = GetVirtualUpper( pFrm, aPos );
    do
    {   if ( pFrm == pCurrFrm )
            return TRUE;
        if( pFrm->IsFlyFrm() )
        {
            aPos = pFrm->Frm().Pos();
            pFrm = GetVirtualUpper( ((const SwFlyFrm*)pFrm)->GetAnchor(), aPos );
        }
        else
            pFrm = pFrm->GetUpper();
    } while ( pFrm );
    return FALSE;
}

const SwFrm *FindKontext( const SwFrm *pFrm, USHORT nAdditionalKontextTyp )
{
    //Liefert die Umgebung des Frm in die kein Fly aus einer anderen
    //Umgebung hineinragen kann.
    const USHORT nTyp = FRM_ROOT | FRM_HEADER   | FRM_FOOTER | FRM_FTNCONT  |
                        FRM_FTN  | FRM_FLY      |
                        FRM_TAB  | FRM_ROW      | FRM_CELL |
                        nAdditionalKontextTyp;
    do
    {   if ( pFrm->GetType() & nTyp )
            break;
        pFrm = pFrm->GetUpper();
    } while( pFrm );
    return pFrm;
}

BOOL IsFrmInSameKontext( const SwFrm *pInnerFrm, const SwFrm *pFrm )
{
    const SwFrm *pKontext = FindKontext( pInnerFrm, 0 );

    const USHORT nTyp = FRM_ROOT | FRM_HEADER   | FRM_FOOTER | FRM_FTNCONT  |
                        FRM_FTN  | FRM_FLY      |
                        FRM_TAB  | FRM_ROW      | FRM_CELL;
    do
    {   if ( pFrm->GetType() & nTyp )
        {
            if( pFrm == pKontext )
                return TRUE;
            if( pFrm->IsCellFrm() )
                return FALSE;
        }
        if( pFrm->IsFlyFrm() )
        {
            Point aPos( pFrm->Frm().Pos() );
            pFrm = GetVirtualUpper( ((const SwFlyFrm*)pFrm)->GetAnchor(), aPos );
        }
        else
            pFrm = pFrm->GetUpper();
    } while( pFrm );

    return FALSE;
}


//---------------------------------

SwTwips MA_FASTCALL lcl_CalcCellRstHeight( SwLayoutFrm *pCell )
{
    if ( pCell->Lower()->IsCntntFrm() || pCell->Lower()->IsSctFrm() )
    {
        SwFrm *pLow = pCell->Lower();
        long nHeight = 0, nFlyAdd = 0;
        do
        {
            long nLow = pLow->Frm().Height();
            if( pLow->IsTxtFrm() && ((SwTxtFrm*)pLow)->IsUndersized() )
                nLow += ((SwTxtFrm*)pLow)->GetParHeight()-pLow->Prt().Height();
            else if( pLow->IsSctFrm() && ((SwSectionFrm*)pLow)->IsUndersized() )
                nLow += ((SwSectionFrm*)pLow)->Undersize();
            nFlyAdd = Max( 0L, nFlyAdd - nLow );
            nFlyAdd = Max( nFlyAdd, ::CalcHeightWidthFlys( pLow ) );
            nHeight += nLow;
            pLow = pLow->GetNext();
        } while ( pLow );
        if ( nFlyAdd )
            nHeight += nFlyAdd;

        //Der Border will natuerlich auch mitspielen, er kann leider nicht
        //aus PrtArea und Frm errechnet werden, da diese in beliebiger
        //Kombination ungueltig sein koennen.
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pCell );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        nHeight += rAttrs.CalcTop() + rAttrs.CalcBottom();

        return pCell->Frm().Height() - nHeight;
    }
    else
    {
        long nRstHeight = 0;
        SwFrm *pLow = pCell->Lower();
        do
        {   nRstHeight += ::CalcRowRstHeight( (SwLayoutFrm*)pLow );
            pLow = pLow->GetNext();

        } while ( pLow );

        return nRstHeight;
    }
}

SwTwips MA_FASTCALL CalcRowRstHeight( SwLayoutFrm *pRow )
{
    SwTwips nRstHeight = LONG_MAX;
    SwLayoutFrm *pLow = (SwLayoutFrm*)pRow->Lower();
    while ( pLow )
    {
        nRstHeight = Min( nRstHeight, ::lcl_CalcCellRstHeight( pLow ) );
        pLow = (SwLayoutFrm*)pLow->GetNext();
    }
    return nRstHeight;
}


void MA_ParkCrsr( SwPageDesc *pDesc, SwCrsrShell &rSh )
{
    //Footer
    const SwFrmFmt  *pFmt = pDesc->GetMaster().GetFooter().GetFooterFmt();
    const SwFmtCntnt  *pCnt;
    if( pFmt )
    {
        pCnt = &pFmt->GetCntnt();
        if ( pCnt->GetCntntIdx() )
            rSh.ParkCrsr( *pCnt->GetCntntIdx() );
    }
    pFmt = pDesc->GetLeft().GetFooter().GetFooterFmt();
    if( pFmt )
    {
        pCnt = &pFmt->GetCntnt();
        if ( pCnt->GetCntntIdx() )
            rSh.ParkCrsr( *pCnt->GetCntntIdx() );
    }

    //Header
    pFmt = pDesc->GetMaster().GetHeader().GetHeaderFmt();
    if( pFmt )
    {
        pCnt = &pFmt->GetCntnt();
        if ( pCnt->GetCntntIdx() )
            rSh.ParkCrsr( *pCnt->GetCntntIdx() );
    }
    pFmt = pDesc->GetLeft().GetHeader().GetHeaderFmt();
    if( pFmt )
    {
        pCnt = &pFmt->GetCntnt();
        if ( pCnt->GetCntntIdx() )
            rSh.ParkCrsr( *pCnt->GetCntntIdx() );
    }
}

const SwFrm* MA_FASTCALL FindPage( const SwRect &rRect, const SwFrm *pPage )
{
    if ( !rRect.IsOver( pPage->Frm() ) )
    {
        BOOL bPrvAllowed = TRUE;
        BOOL bNxtAllowed = TRUE;
        do
        {   if ( pPage->Frm().Top() > rRect.Top() && bPrvAllowed )
            {
                if ( pPage->GetPrev() )
                {
                    bNxtAllowed = FALSE;
                    pPage = pPage->GetPrev();
                }
                else
                    break;
            }
            else if ( pPage->Frm().Bottom() < rRect.Top() && bNxtAllowed )
            {
                if ( pPage->GetNext() )
                {
                    bPrvAllowed = FALSE;
                    pPage = pPage->GetNext();
                }
                else
                    break;
            }
            else
                break;

        } while ( !rRect.IsOver( pPage->Frm() ) );
    }
    return pPage;
}


SwFrm* GetFrmOfModify( SwModify& rMod, USHORT nFrmType, const Point* pPoint,
                        const SwPosition *pPos, const BOOL bCalcFrm )
{
    SwFrm *pMinFrm = 0, *pTmpFrm;
    SwRect aCalcRect;

    SwClientIter aIter( rMod );
    do {
        pMinFrm = 0;
        Size aMinSize;

        for( pTmpFrm = (SwFrm*)aIter.First( TYPE( SwFrm )); pTmpFrm;
                pTmpFrm = (SwFrm*)aIter.Next() )
            if( pTmpFrm->GetType() & nFrmType &&
                (!pTmpFrm->IsFlowFrm() ||
                 !SwFlowFrm::CastFlowFrm( pTmpFrm )->IsFollow() ))
            {
                if( pPoint )
                {
                    if( bCalcFrm )
                        pTmpFrm->Calc();

                    if( aIter.IsChanged() )     // der Liste hat sich ver-
                        break;                  // aendert, neu anfangen !!

                    // bei Flys ggfs. ueber den Parent gehen wenn sie selbst
                    // nocht nicht "formatiert" sind
                    if( !bCalcFrm && nFrmType & FRM_FLY &&
                        ((SwFlyFrm*)pTmpFrm)->GetAnchor() &&
                        WEIT_WECH == pTmpFrm->Frm().Pos().X() &&
                        WEIT_WECH == pTmpFrm->Frm().Pos().Y() )
                        aCalcRect = ((SwFlyFrm*)pTmpFrm)->GetAnchor()->Frm();
                    else
                        aCalcRect = pTmpFrm->Frm();

                    // fasse den Point und das Recteck zusammen, falls
                    // er Point nicht innerhalb liegt. Liegt er ausserhalb,
                    // wird nach dem kleinsten Rectangle gesucht, also das,
                    // wo der Point am dichtesten dran liegt. Ist der Point im
                    // Rechteck, wird die Schleife beendet.
                    {
                        BOOL bInside = TRUE;
                        // die Left/Right-Position erweitern
                        if( pPoint->X() < aCalcRect.Left() )
                            {   bInside = FALSE; aCalcRect.Left( pPoint->X() ); }
                        if( pPoint->X() > aCalcRect.Right() )
                            {   bInside = FALSE; aCalcRect.Right( pPoint->X() ); }

                        if( pPoint->Y() > aCalcRect.Bottom() )
                            {   bInside = FALSE; aCalcRect.Bottom( pPoint->Y() ); }
                        if( pPoint->Y() < aCalcRect.Top() )
                            {   bInside = FALSE; aCalcRect.Top( pPoint->Y() ); }
                        if( bInside )
                        {
                            pMinFrm = pTmpFrm;
                            break;
                        }
                    }

                    if( pMinFrm )
                    {
                        long nDiffW = aMinSize.Width() - aCalcRect.Width();
                        long nDiffH = aMinSize.Height() - aCalcRect.Height();

                            // gleiche Hoehe, dann entscheided die Breite
                        if( !nDiffH )       { if( 0 >= nDiffW ) continue; }
                            // gleiche Breite, dann entscheided die Hoehe
                        else if( !nDiffW )  { if( 0 >= nDiffH ) continue; }

                            // hoehere Gewichtung auf die Hoehe !!
                        else if( !(0 < nDiffW && 0 < nDiffH ) &&
                                ((0 > nDiffW && 0 > nDiffH ) ||
                                0 >= nDiffH ))
                            continue;
                    }
                }
                else
                {
                    // Wenn kein pPoint angegeben ist, dann reichen
                    // wir irgendeinen raus: den ersten!
                    pMinFrm = pTmpFrm;
                    break;
                }
                pMinFrm = pTmpFrm;
                aMinSize = aCalcRect.SSize();
            }
    } while( aIter.IsChanged() );

    if( pPos && pMinFrm && pMinFrm->IsTxtFrm() )
        return ((SwTxtFrm*)pMinFrm)->GetFrmAtPos( *pPos );

    return pMinFrm;
}

FASTBOOL IsExtraData( const SwDoc *pDoc )
{
    const SwLineNumberInfo &rInf = pDoc->GetLineNumberInfo();
    return rInf.IsPaintLineNumbers() ||
           rInf.IsCountInFlys() ||
           ((SwHoriOrient)SW_MOD()->GetRedlineMarkPos() != HORI_NONE &&
            pDoc->GetRedlineTbl().Count());
}

