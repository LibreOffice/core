/*************************************************************************
 *
 *  $RCSfile: dview.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:02:28 $
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

#include "hintids.hxx"

#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _FM_FMMODEL_HXX
#include <svx/fmmodel.hxx>
#endif
#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif


#include "swtypes.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "flyfrm.hxx"
#include "frmfmt.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "frmatr.hxx"
#include "viewsh.hxx"
#include "viewimp.hxx"
#include "dview.hxx"
#include "dpage.hxx"
#include "doc.hxx"
#include "mdiexp.hxx"

#ifndef _NDOLE_HXX //autogen
#include <ndole.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#include "shellres.hxx"

// #i7672#
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif

// OD 18.06.2003 #108784#
#ifndef _SVDVMARK_HXX //autogen
#include <svx/svdvmark.hxx>
#endif
#include <vector>
// --> OD 2004-06-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif
#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif
// <--

class SwSdrHdl : public SdrHdl
{
public:
    SwSdrHdl(const Point& rPnt, bool bTopRight ) :
        SdrHdl( rPnt, bTopRight ? HDL_ANCHOR_TR : HDL_ANCHOR ) {}
    virtual BOOL IsFocusHdl() const;
};

BOOL SwSdrHdl::IsFocusHdl() const
{
    if( HDL_ANCHOR == eKind || HDL_ANCHOR_TR == eKind )
        return TRUE;
    return SdrHdl::IsFocusHdl();
}

const SwFrm *lcl_FindAnchor( const SdrObject *pObj, FASTBOOL bAll )
{
    const SwVirtFlyDrawObj *pVirt = pObj->ISA(SwVirtFlyDrawObj) ?
                                            (SwVirtFlyDrawObj*)pObj : 0;
    if ( pVirt )
    {
        if ( bAll || !pVirt->GetFlyFrm()->IsFlyInCntFrm() )
            return pVirt->GetFlyFrm()->GetAnchorFrm();
    }
    else
    {
        const SwDrawContact *pCont = (const SwDrawContact*)GetUserCall(pObj);
        if ( pCont )
            return pCont->GetAnchorFrm( pObj );
    }
    return 0;
}

/*************************************************************************
|*
|*  SwDrawView::Ctor
|*
|*  Ersterstellung      OK 18.11.94
|*  Letzte Aenderung    MA 22. Jul. 96
|*
*************************************************************************/



SwDrawView::SwDrawView( SwViewImp &rI, SdrModel *pMd, OutputDevice *pOutDev) :
    FmFormView( (FmFormModel*)pMd, pOutDev ),
    rImp( rI )
{
    SetPageVisible( FALSE );
    SetBordVisible( FALSE );
    SetGridVisible( FALSE );
    SetHlplVisible( FALSE );
    SetGlueVisible( FALSE );
    SetFrameDragSingles( TRUE );
    SetVirtualObjectBundling( TRUE );
    SetSwapAsynchron( TRUE );

    EnableExtendedKeyInputDispatcher( FALSE );
    EnableExtendedMouseEventDispatcher( FALSE );
    EnableExtendedCommandEventDispatcher( FALSE );

    SetHitTolerancePixel( GetMarkHdlSizePixel()/2 );

    SetPrintPreview( rI.GetShell()->IsPreView() );
}

/*************************************************************************
|*
|*  SwDrawView::AddCustomHdl()
|*
|*  Gets called every time the handles need to be build
|*
|*  Ersterstellung      AW 06. Sep. 99
|*  Letzte Aenderung    AW 06. Sep. 99
|*
*************************************************************************/

void SwDrawView::AddCustomHdl()
{
    const SdrMarkList &rMrkList = GetMarkedObjectList();

    if(rMrkList.GetMarkCount() != 1 || !GetUserCall(rMrkList.GetMark( 0 )->GetObj()))
        return;

    SdrObject *pObj = rMrkList.GetMark(0)->GetObj();
    const SwFmtAnchor &rAnchor = ::FindFrmFmt(pObj)->GetAnchor();

    if(FLY_IN_CNTNT == rAnchor.GetAnchorId())
        return;

    const SwFrm* pAnch;
    if(0 == (pAnch = CalcAnchor()))
        return;

    ViewShell &rSh = *Imp().GetShell();
    Point aPos(aAnchorPoint);

    if ( FLY_AUTO_CNTNT == rAnchor.GetAnchorId() )
    {
        // --> OD 2004-06-24 #i28701# - use last character rectangle saved at object
        // in order to avoid a format of the anchor frame
        SwAnchoredObject* pAnchoredObj = ::GetUserCall( pObj )->GetAnchoredObj( pObj );
        SwRect aAutoPos = pAnchoredObj->GetLastCharRect();
        if ( aAutoPos.Height() )
        {
            aPos = aAutoPos.Pos();
        }
    }

    // add anchor handle:
    aHdl.AddHdl( new SwSdrHdl( aPos, pAnch->IsVertical() ||
                                     pAnch->IsRightToLeft() ) );
}

/*************************************************************************
|*
|*  SwDrawView::GetMaxToTopObj(), _GetMaxToTopObj()
|*
|*  Ersterstellung      MA 13. Jan. 95
|*  Letzte Aenderung    MA 18. Mar. 97
|*
*************************************************************************/


SdrObject* SwDrawView::GetMaxToTopObj( SdrObject* pObj ) const
{
    if ( GetUserCall(pObj) )
    {
        const SwFrm *pAnch = ::lcl_FindAnchor( pObj, FALSE );
        if ( pAnch )
        {
            //Das oberste Obj innerhalb des Ankers darf nicht ueberholt
            //werden.
            const SwFlyFrm *pFly = pAnch->FindFlyFrm();
            if ( pFly )
            {
                const SwPageFrm *pPage = pFly->FindPageFrm();
                if ( pPage->GetSortedObjs() )
                {
                    UINT32 nOrdNum = 0;
                    for ( USHORT i = 0; i < pPage->GetSortedObjs()->Count(); ++i )
                    {
                        const SdrObject *pO =
                                    (*pPage->GetSortedObjs())[i]->GetDrawObj();

                        if ( pO->GetOrdNumDirect() > nOrdNum )
                        {
                            const SwFrm *pAnch = ::lcl_FindAnchor( pO, FALSE );
                            if ( pFly->IsAnLower( pAnch ) )
                            {
                                nOrdNum = pO->GetOrdNumDirect();
                            }
                        }
                    }
                    if ( nOrdNum )
                    {
                        SdrPage *pPage = GetModel()->GetPage( 0 );
                        ++nOrdNum;
                        if ( nOrdNum < pPage->GetObjCount() )
                        {
                            return pPage->GetObj( nOrdNum );
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/*************************************************************************
|*
|*  SwDrawView::GetMaxToBtmObj()
|*
|*  Ersterstellung      MA 13. Jan. 95
|*  Letzte Aenderung    MA 05. Sep. 96
|*
*************************************************************************/


SdrObject* SwDrawView::GetMaxToBtmObj(SdrObject* pObj) const
{
    if ( GetUserCall(pObj) )
    {
        const SwFrm *pAnch = ::lcl_FindAnchor( pObj, FALSE );
        if ( pAnch )
        {
            //Der Fly des Ankers darf nicht "unterflogen" werden.
            const SwFlyFrm *pFly = pAnch->FindFlyFrm();
            if ( pFly )
            {
                SdrObject *pRet = (SdrObject*)pFly->GetVirtDrawObj();
                return pRet != pObj ? pRet : 0;
            }
        }
    }
    return 0;
}

/*************************************************************************
|*
|*  SwDrawView::ObjOrderChanged()
|*
|*  Ersterstellung      MA 31. Jul. 95
|*  Letzte Aenderung    MA 18. Mar. 97
|*
*************************************************************************/

inline BOOL lcl_IsChild( SdrObject *pParent, SdrObject *pChild )
{
    if ( pParent->ISA(SwVirtFlyDrawObj) )
    {
        const SwFrm *pAnch = lcl_FindAnchor( pChild, FALSE );
        if ( pAnch && ((SwVirtFlyDrawObj*)pParent)->GetFlyFrm()->IsAnLower( pAnch ))
        {
            return TRUE;
        }
    }
    return FALSE;
}

inline SdrObject *lcl_FindParent( SdrObject *pObj )
{
    const SwFrm *pAnch = lcl_FindAnchor( pObj, FALSE );
    if ( pAnch && pAnch->IsInFly() )
        return (SdrObject*)pAnch->FindFlyFrm()->GetVirtDrawObj();
    return 0;
}



void SwDrawView::ObjOrderChanged( SdrObject* pObj, ULONG nOldPos,
                                          ULONG nNewPos )
{
    SdrPage *pPg = GetModel()->GetPage( 0 );
    if ( pPg->IsObjOrdNumsDirty() )
        pPg->RecalcObjOrdNums();
    const BOOL bBtm = nOldPos > nNewPos;
    ULONG nMoveTo = ULONG_MAX;

    //Wenn ein Object nach oben geschoben werden soll, so muss es wenigstens
    //seine Kinder plus einem ueberspringen.
    if ( !bBtm && nNewPos < pPg->GetObjCount() - 1 )
    {
        ULONG nPos = nOldPos;
        SdrObject *pTmp = pPg->GetObj( nPos );
        while ( pTmp == pObj || lcl_IsChild( pObj, pTmp ) )
        {
            ++nPos;
            pTmp = pPg->GetObj( nPos );
        }
        if ( nPos > nNewPos )
            nMoveTo = nPos;
    }
    if ( nMoveTo != ULONG_MAX )
    {
        if ( nMoveTo <= nNewPos )
            ++nMoveTo;
        pPg->SetObjectOrdNum( nNewPos, nMoveTo );
        if ( pPg->IsObjOrdNumsDirty() )
            pPg->RecalcObjOrdNums();
        nNewPos = nMoveTo;
        nMoveTo = ULONG_MAX;
    }

    //Kein Objekt darf in eine Schachtelung von Rahmen/Objekten eindringen,
    //die Kette muss ggf. uebersprungen werden.
    if ( bBtm )
    {
        if ( nNewPos > 0 )
        {
            SdrObject *pMax = GetMaxToBtmObj( pObj ),
                      *pO = pPg->GetObj( nNewPos + 1 ),
                      *pPre = pO;
            while ( pO && 0 != (pO = GetMaxToBtmObj( pO )))
            {
                if ( pO != pMax )
                    nMoveTo = pO->GetOrdNumDirect();
                if ( pO == pPre )
                    break;
            }
        }
    }
    else
    {
        if ( nNewPos < pPg->GetObjCount() - 1 )
        {
            ULONG nPos = nNewPos;
            SdrObject *pMyParent = lcl_FindParent( pObj ),
                      *pNxt      = pPg->GetObj( nPos + 1 ),
                      *pNxtParent= lcl_FindParent( pNxt );
            while ( pNxtParent && pNxtParent != pMyParent )
            {
                nMoveTo = ++nPos;
                if ( nPos < pPg->GetObjCount() - 1 )
                {
                    pNxt       = pPg->GetObj( nPos + 1 );
                    pNxtParent = lcl_FindParent( pNxt );
                }
                else
                    break;
            }
        }
    }
    if ( nMoveTo != ULONG_MAX )
    {
        pPg->SetObjectOrdNum( nNewPos, nMoveTo );
        if ( pPg->IsObjOrdNumsDirty() )
            pPg->RecalcObjOrdNums();
        nNewPos = nMoveTo;
    }

    if ( pObj->ISA(SwVirtFlyDrawObj) )
    {
        //Ein Rahmen wurde in seiner Order veraendert. Hier muss nachtraeglich
        //dafuer gesorgt werden, dass seine 'Kinder' nachgezogen werden.
        const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
        if ( pPg->IsObjOrdNumsDirty() )
            pPg->RecalcObjOrdNums();

        rImp.DisposeAccessibleFrm( pFly );
        rImp.AddAccessibleFrm( pFly );

        if ( bBtm )
            ++nNewPos;
        BOOL bFound = FALSE;
        for ( ULONG i = nOldPos; i < pPg->GetObjCount(); ++i )
        {
            SdrObject *pO = pPg->GetObj( i );
            if ( pO == pObj )
                break;
            const SwFrm *pAnch;
            const BOOL bFly = pO->ISA(SwVirtFlyDrawObj);
            if ( bFly )
            {
                pAnch = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm()->GetAnchorFrm();
            }
            else
            {
                pAnch = ((SwDrawContact*)GetUserCall(pO))->GetAnchorFrm( pO );
            }
            const SwFlyFrm *pF = pAnch ? pAnch->FindFlyFrm() : NULL;
            if ( pF && (pF == pFly || pFly->IsUpperOf( *pF ) ) )
            {
                //Kind gefunden, verschieben.
                pPg->SetObjectOrdNum( i, nNewPos );
                pPg->RecalcObjOrdNums();
                --i;    //keinen auslassen
                if ( bFly )
                {
                    const SwFlyFrm *pFF =
                        static_cast< const SwVirtFlyDrawObj *>(pO)->GetFlyFrm();
                    rImp.DisposeAccessibleFrm( pFF );
                    rImp.AddAccessibleFrm( pFF );
                }
            }
        }
    }
    else
    {
        rImp.DisposeAccessibleObj( pObj );
        rImp.AddAccessibleObj( pObj );
    }
}

/*************************************************************************
|*
|*  SwDrawView::TakeDragLimit()
|*
|*  Ersterstellung      AMA 26. Apr. 96
|*  Letzte Aenderung    MA 03. May. 96
|*
*************************************************************************/


BOOL SwDrawView::TakeDragLimit( SdrDragMode eMode,
                                            Rectangle& rRect ) const
{
    const SdrMarkList &rMrkList = GetMarkedObjectList();
    BOOL bRet = FALSE;
    if( 1 == rMrkList.GetMarkCount() )
    {
        const SdrObject *pObj = rMrkList.GetMark( 0 )->GetObj();
        SwRect aRect;
        if( ::CalcClipRect( pObj, aRect, eMode == SDRDRAG_MOVE ) )
        {
            rRect = aRect.SVRect();
             bRet = TRUE;
        }
    }
    return bRet;
}

/*************************************************************************
|*
|*  SwDrawView::CalcAnchor()
|*
|*  Ersterstellung      MA 13. Jan. 95
|*  Letzte Aenderung    MA 08. Nov. 96
|*
*************************************************************************/


const SwFrm* SwDrawView::CalcAnchor()
{
    const SdrMarkList &rMrkList = GetMarkedObjectList();
    if ( rMrkList.GetMarkCount() != 1 )
        return NULL;

    SdrObject* pObj = rMrkList.GetMark( 0 )->GetObj();

    //Fuer Absatzgebundene Objekte suchen, andernfalls einfach nur
    //der aktuelle Anker. Nur suchen wenn wir gerade draggen.
    const SwFrm* pAnch;
    Rectangle aMyRect;
    const BOOL bFly = pObj->ISA(SwVirtFlyDrawObj);
    if ( bFly )
    {
        pAnch = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->GetAnchorFrm();
        aMyRect = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->Frm().SVRect();
    }
    else
    {
        SwDrawContact *pC = (SwDrawContact*)GetUserCall(pObj);
        // OD 17.06.2003 #108784# - determine correct anchor position for
        // 'virtual' drawing objects.
        // OD 2004-03-25 #i26791#
        pAnch = pC->GetAnchorFrm( pObj );
        if( !pAnch )
        {
            pC->ConnectToLayout();
            // OD 17.06.2003 #108784# - determine correct anchor position for
            // 'virtual' drawing objects.
            // OD 2004-03-25 #i26791#
            pAnch = pC->GetAnchorFrm( pObj );
        }
        aMyRect = pObj->GetSnapRect();
    }

    const sal_Bool bTopRight = pAnch && ( pAnch->IsVertical() ||
                                          pAnch->IsRightToLeft() );

    const Point aMyPt = bTopRight ? aMyRect.TopRight() : aMyRect.TopLeft();

    Point aPt;
    if ( IsAction() )
    {
        if ( !TakeDragObjAnchorPos( aPt, bTopRight ) )
            return NULL;
    }
    else
    {
        Rectangle aRect = pObj->GetSnapRect();
        aPt = bTopRight ? aRect.TopRight() : aRect.TopLeft();
    }

    if ( aPt != aMyPt )
    {
        if ( pAnch->IsCntntFrm() )
        {
            // OD 26.06.2003 #108784# - allow drawing objects in header/footer,
            // but exclude control objects.
            bool bBodyOnly = CheckControlLayer( pObj );
            pAnch = ::FindAnchor( (SwCntntFrm*)pAnch, aPt, bBodyOnly );
        }
        else if ( !bFly )
        {
            const SwRect aRect( aPt.X(), aPt.Y(), 1, 1 );

            SwDrawContact* pContact = (SwDrawContact*)GetUserCall(pObj);
            if ( pContact->GetAnchorFrm( pObj ) &&
                 pContact->GetAnchorFrm( pObj )->IsPageFrm() )
                pAnch = pContact->GetPageFrm();
            else
                pAnch = pContact->FindPage( aRect );
        }
    }
    if( pAnch && !pAnch->IsProtected() )
        aAnchorPoint = pAnch->GetFrmAnchorPos( ::HasWrap( pObj ) );
    else
        pAnch = 0;
    return pAnch;
}


Rectangle *SwDrawView::IsAnchorAtPos( const Point &rPt ) const
{
    SdrHdl* pHdl = aHdl.GetHdl(HDL_ANCHOR);
    if ( ! pHdl )
        pHdl = aHdl.GetHdl(HDL_ANCHOR_TR);

    if(pHdl)
    {
        const ViewShell &rSh = *Imp().GetShell();
        const OutputDevice *pOut = rSh.GetOut();

        if(pHdl->IsHit(rPt, *pOut))
        {
            B2dIAObject* pIAO = pHdl->GetIAOGroup().GetIAObject(0);

            // #i21114#
            // B2dIAOBitmapExReference is no longer used, change to B2dIAOBitmapEx
            if(pIAO && pIAO->ISA(B2dIAOBitmapEx))
            {
                Rectangle aRect(
                    pIAO->GetBasePosition(),
                    pOut->PixelToLogic(((B2dIAOBitmapEx*)pIAO)->GetBitmapEx().GetSizePixel()));
                return new Rectangle(aRect);
            }
        }
    }

    return NULL;
}

/*************************************************************************
|*
|*  SwDrawView::ShowDragXor(), HideDragXor()
|*
|*  Ersterstellung      MA 17. Jan. 95
|*  Letzte Aenderung    MA 27. Jan. 95
|*
*************************************************************************/


void SwDrawView::ShowDragAnchor()
{
    SdrHdl* pHdl = aHdl.GetHdl(HDL_ANCHOR);
    if ( ! pHdl )
        pHdl = aHdl.GetHdl(HDL_ANCHOR_TR);

    if(pHdl)
    {
        CalcAnchor();
        pHdl->SetPos(aAnchorPoint);
        RefreshAllIAOManagers();
    }
}



/*************************************************************************
|*
|*  SwDrawView::MarkListHasChanged()
|*
|*  Ersterstellung      OM 02. Feb. 95
|*  Letzte Aenderung    OM 07. Jul. 95
|*
*************************************************************************/


void SwDrawView::MarkListHasChanged()
{
    Imp().GetShell()->DrawSelChanged(this);
    FmFormView::MarkListHasChanged();
}

// #i7672#
void SwDrawView::ModelHasChanged()
{
    // The ModelHasChanged() call in DrawingLayer also updates
    // a eventually active text edit view (OutlinerView). This also leads
    // to newly setting the background color for that edit view. Thus,
    // this method rescues the current background color if a OutlinerView
    // exists and re-establishes it then. To be more safe, the OutlinerView
    // will be fetched again (maybe textedit has ended).
    OutlinerView* pView = GetTextEditOutlinerView();
    Color aBackColor;
    sal_Bool bColorWasSaved(sal_False);

    if(pView)
    {
        aBackColor = pView->GetBackgroundColor();
        bColorWasSaved = sal_True;
    }

    // call parent
    FmFormView::ModelHasChanged();

    if(bColorWasSaved)
    {
        pView = GetTextEditOutlinerView();

        if(pView)
        {
            pView->SetBackgroundColor(aBackColor);
        }
    }
}

void SwDrawView::MakeVisible( const Rectangle &rRect, Window &rWin )
{
    ASSERT( rImp.GetShell()->GetWin() && &rWin, "MakeVisible, unknown Window");
    rImp.GetShell()->MakeVisible( SwRect( rRect ) );
}

#if SUPD<500
#define SVOBJ_MISCSTATUS_NOTRESIZEABLE 0
#endif

void SwDrawView::CheckPossibilities()
{
    FmFormView::CheckPossibilities();

    //Zusaetzlich zu den bestehenden Flags der Objekte selbst, die von der
    //DrawingEngine ausgewertet werden, koennen weitere Umstaende zu einem
    //Schutz fuehren.
    //Objekte, die in Rahmen verankert sind, muessen genau dann geschuetzt
    //sein, wenn der Inhalt des Rahmens geschuetzt ist.
    //OLE-Objekte konnen selbst einen Resize-Schutz wuenschen (StarMath)

    const SdrMarkList &rMrkList = GetMarkedObjectList();
    FASTBOOL bProtect = FALSE,
             bSzProtect = FALSE;
    for ( USHORT i = 0; !bProtect && i < rMrkList.GetMarkCount(); ++i )
    {
        const SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
        const SwFrm *pFrm = NULL;
        if ( pObj->ISA(SwVirtFlyDrawObj) )
        {
            const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
            if ( pFly  )
            {
                pFrm = pFly->GetAnchorFrm();
                if ( pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
                {
                    SwOLENode *pNd = ((SwCntntFrm*)pFly->Lower())->GetNode()->GetOLENode();
                    if ( pNd )
                    {
                        SvInPlaceObjectRef aRef = pNd->GetOLEObj().GetOleRef();
                        if ( aRef.Is() )
                        {
                            bSzProtect = SVOBJ_MISCSTATUS_NOTRESIZEABLE & aRef->GetMiscStatus()
                                            ? TRUE : FALSE;
                        }
                    }
                }
            }
        }
        else
        {
            SwDrawContact *pC = (SwDrawContact*)GetUserCall(pObj);
            if ( pC )
                pFrm = pC->GetAnchorFrm( pObj );
        }
        if ( pFrm )
            bProtect = pFrm->IsProtected(); //Rahmen, Bereiche usw.
        if ( FLY_IN_CNTNT == ::FindFrmFmt( (SdrObject*)pObj )->GetAnchor().GetAnchorId() &&
             rMrkList.GetMarkCount() > 1 )
            bProtect = TRUE;
    }
    bMoveProtect    |= bProtect;
    bResizeProtect  |= bProtect | bSzProtect;
}

/** replace marked <SwDrawVirtObj>-objects by its reference object for delete
    marked objects.

    OD 18.06.2003 #108784#

    @author OD
*/
void SwDrawView::ReplaceMarkedDrawVirtObjs( SdrMarkView& _rMarkView )
{
    SdrPageView* pDrawPageView = _rMarkView.GetPageViewPgNum(0);
    const SdrMarkList& rMarkList = _rMarkView.GetMarkedObjectList();

    if( rMarkList.GetMarkCount() )
    {
        // collect marked objects in a local data structure
        std::vector<SdrObject*> aMarkedObjs;
        for( sal_uInt32 i = 0; i < rMarkList.GetMarkCount(); ++i )
        {
            SdrObject* pMarkedObj = rMarkList.GetMark( i )->GetObj();
            aMarkedObjs.push_back( pMarkedObj );
        }
        // unmark all objects
        _rMarkView.UnmarkAllObj();
        // re-mark objects, but for marked <SwDrawVirtObj>-objects marked its
        // reference object.
        while ( !aMarkedObjs.empty() )
        {
            SdrObject* pMarkObj = aMarkedObjs.back();
            if ( pMarkObj->ISA(SwDrawVirtObj) )
            {
                SdrObject* pRefObj = &(static_cast<SwDrawVirtObj*>(pMarkObj)->ReferencedObj());
                if ( !_rMarkView.IsObjMarked( pRefObj )  )
                {
                    _rMarkView.MarkObj( pRefObj, pDrawPageView );
                }
            }
            else
            {
                _rMarkView.MarkObj( pMarkObj, pDrawPageView );
            }

            aMarkedObjs.pop_back();
        }
        // sort marked list in order to assure consistent state in drawing layer
        _rMarkView.SortMarkedObjects();
    }
}

void SwDrawView::DeleteMarked()
{
    SwDoc* pDoc = Imp().GetShell()->GetDoc();
    if ( pDoc->GetRootFrm() )
        pDoc->GetRootFrm()->StartAllAction();
    pDoc->StartUndo();
    // OD 18.06.2003 #108784# - replace marked <SwDrawVirtObj>-objects by its
    // reference objects.
    {
        SdrPageView* pDrawPageView = rImp.GetPageView();
        if ( pDrawPageView )
        {
            SdrMarkView* pMarkView = PTR_CAST( SdrMarkView, &(pDrawPageView->GetView()) );
            if ( pMarkView )
            {
                ReplaceMarkedDrawVirtObjs( *pMarkView );
            }
        }
    }
    if ( pDoc->DeleteSelection( *this ) )
    {
        FmFormView::DeleteMarked();
        ::FrameNotify( Imp().GetShell(), FLY_DRAG_END );
    }
    pDoc->EndUndo();
    if( pDoc->GetRootFrm() )
        pDoc->GetRootFrm()->EndAllAction();
}

/********
JP 02.10.98: sollte als Fix fuer 57153 gelten, hatte aber Nebenwirkungen,
            wie Bug 57475
const SdrMarkList& SwDrawView::GetMarkedObjectList() const
{
    FlushComeBackTimer();
    return FmFormView::GetMarkedObjectList();
}
*************/




