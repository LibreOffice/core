/*************************************************************************
 *
 *  $RCSfile: vdraw.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:29 $
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

#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _XOUTX_HXX //autogen
#include <svx/xoutx.hxx>
#endif

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif

#ifndef PRODUCT
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#endif

#include "fesh.hxx"
#include "doc.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "viewimp.hxx"
#include "dflyobj.hxx"
#include "viewopt.hxx"
#include "dcontact.hxx"
#include "dview.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"


/*************************************************************************
|*
|*  SwSaveHdl
|*
|*  Ersterstellung      MA 14. Feb. 95
|*  Letzte Aenderung    MA 02. Jun. 98
|*
|*************************************************************************/
SwSaveHdl::SwSaveHdl( SwViewImp *pI ) :
    pImp( pI ),
    bXorVis( FALSE )
{
    if ( pImp->HasDrawView() )
    {
        bXorVis = pImp->GetDrawView()->IsShownXorVisible( pImp->GetShell()->GetOut());
        if ( bXorVis )
            pImp->GetDrawView()->HideShownXor( pImp->GetShell()->GetOut() );
    }
}


SwSaveHdl::~SwSaveHdl()
{
    if ( bXorVis )
        pImp->GetDrawView()->ShowShownXor( pImp->GetShell()->GetOut() );
}


/*************************************************************************
|*
|*  SwViewImp::StartAction(), EndAction()
|*
|*  Ersterstellung      MA 14. Feb. 95
|*  Letzte Aenderung    MA 14. Sep. 98
|*
|*************************************************************************/


void SwViewImp::StartAction()
{
    if ( HasDrawView() )
    {
        SET_CURR_SHELL( GetShell() );
        if ( pSh->ISA(SwFEShell) )
            ((SwFEShell*)pSh)->HideChainMarker();   //Kann sich geaendert haben
        bResetXorVisibility = GetDrawView()->IsShownXorVisible( GetShell()->GetOut());
        GetDrawView()->HideShownXor( GetShell()->GetOut() );
    }
}



void SwViewImp::EndAction()
{
    if ( HasDrawView() )
    {
        SET_CURR_SHELL( GetShell() );
        if ( bResetXorVisibility )
            GetDrawView()->ShowShownXor( GetShell()->GetOut() );
        if ( pSh->ISA(SwFEShell) )
            ((SwFEShell*)pSh)->SetChainMarker();    //Kann sich geaendert haben
    }
}

/*************************************************************************
|*
|*  SwViewImp::LockPaint(), UnlockPaint()
|*
|*  Ersterstellung      MA 11. Jun. 96
|*  Letzte Aenderung    MA 11. Jun. 96
|*
|*************************************************************************/


void SwViewImp::LockPaint()
{
    if ( HasDrawView() )
    {
        bShowHdlPaint = GetDrawView()->IsMarkHdlShown();
        if ( bShowHdlPaint )
            GetDrawView()->HideMarkHdl( GetShell()->GetOut() );
        bResetHdlHiddenPaint = !GetDrawView()->IsMarkHdlHidden();
        GetDrawView()->SetMarkHdlHidden( TRUE );
    }
    else
    {
        bShowHdlPaint = FALSE;
        bResetHdlHiddenPaint = FALSE;
    }
}



void SwViewImp::UnlockPaint()
{
    if ( bResetHdlHiddenPaint )
        GetDrawView()->SetMarkHdlHidden( FALSE );
    if ( bShowHdlPaint )
        GetDrawView()->ShowMarkHdl( GetShell()->GetOut() );
}


/*************************************************************************
|*
|*  SwViewImp::PaintLayer(), PaintDispatcher()
|*
|*  Ersterstellung      MA 20. Dec. 94
|*  Letzte Aenderung    AMA 04. Jun. 98
|*
|*************************************************************************/


void SwViewImp::PaintLayer( const BYTE nLayerID, const SwRect &rRect ) const
{
    Link aLnk( LINK( this, SwViewImp, PaintDispatcher ) );
    if ( HasDrawView() )
    {
        GetPageView()->RedrawOneLayer( nLayerID, rRect.SVRect(),
                        GetShell()->GetOut(),
                        GetShell()->IsPreView() ? SDRPAINTMODE_ANILIKEPRN : 0,
                        &aLnk );
    }
}


IMPL_LINK( SwViewImp, PaintDispatcher, SdrPaintProcRec *, pRec )
{
    SdrObject *pObj = pRec->pObj;

    //Controls muessen im Control-Layer liegen. Dort duerfen aber auch
    //Gruppenobjekte oder mit Controls gruppierte Objekte liegen.
    ASSERT( FmFormInventor != pObj->GetObjInventor() ||
            GetShell()->GetDoc()->GetControlsId() == pObj->GetLayer(),
            "PaintDispatcher: Wrong Layer" );

    if ( !SwFlyFrm::IsPaint( pObj, GetShell() ) )
        return 0;

    const BYTE nHellId = GetShell()->GetDoc()->GetHellId();
    if ( pObj->IsWriterFlyFrame() )
    {
        if( pObj->GetLayer() == nHellId )
        {
            //Fuer Rahmen in der Hoelle gelten andere Regeln:
            //1. Rahmen mit einem Parent werden nie direkt, sondern von ihren
            //   Parents gepaintet.
            //1a.Es sei denn, der Parent steht nicht in der Hoelle.
            //2. Rahmen mit Childs painten zuerst die Childs in
            //   umgekehrter Z-Order.
            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
            const FASTBOOL bInFly = pFly->GetAnchor()->IsInFly();
            if ( !bInFly ||
                 (bInFly && pFly->GetAnchor()->FindFlyFrm()->
                                   GetVirtDrawObj()->GetLayer() != nHellId))
                PaintFlyChilds( pFly, pRec->rOut, pRec->rInfoRec );
        }
        else
            pObj->Paint( pRec->rOut, pRec->rInfoRec );
    }
    else
    {
        SwRect aTmp( pRec->rInfoRec.aDirtyRect );
        ::InvertSizeBorderRect( aTmp, GetShell() );

        OutputDevice *pOut = pRec->rOut.GetOutDev();
        pOut->Push( PUSH_CLIPREGION );
        pOut->IntersectClipRegion( aTmp.SVRect() );

        //Um zu verhindern, dass der Dispatcher fr jedes Gruppenobjekt
        //gerufen wird, muessen wir die Struktur manipulieren
        //(Absprache mit JOE).
        const Link *pSave = 0;
        if ( pObj->IsGroupObject() )
        {
            pSave = pRec->rInfoRec.pPaintProc;
            ((SdrPaintInfoRec&)pRec->rInfoRec).pPaintProc = 0;
        }

        pObj->Paint( pRec->rOut, pRec->rInfoRec );

        if ( pSave )
            ((SdrPaintInfoRec&)pRec->rInfoRec).pPaintProc = pSave;

        pOut->Pop();
    }
    return 0;
}

/*************************************************************************
|*
|*  SwViewImp::PaintFlyChilds()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 02. Aug. 95
|*
|*************************************************************************/


void SwViewImp::PaintFlyChilds( SwFlyFrm *pFly, ExtOutputDevice& rOut,
                                 const SdrPaintInfoRec& rInfoRec )
{
    SdrObject *pFlyObj  = pFly->GetVirtDrawObj();
    SdrPage   *pPage = pFlyObj->GetPage();
    OutputDevice *pOut = rOut.GetOutDev();

    //Zuerst den am weitesten oben liegenden Child suchen.
    ULONG i;
    for ( i = pFlyObj->GetOrdNumDirect()+1; i < pPage->GetObjCount(); ++i )
    {
        SdrObject *pObj = pPage->GetObj( i );
        SwFlyFrm *pF;
        if ( pObj->IsWriterFlyFrame() )
            pF = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
        else
        {
            SwFrm *pFrm = ((SwDrawContact*)GetUserCall(pObj))->GetAnchor();
            pF = pFrm ? pFrm->FindFlyFrm() : 0;
        }
        if ( pF && pF != pFly && !pF->IsLowerOf( pFly ) )
            break;
    }
    --i;    //Ich bin auf immer einen zu weit gelaufen.
    if ( i != pFlyObj->GetOrdNumDirect() )
    {
        for ( UINT32 j = i; j > pFlyObj->GetOrdNumDirect(); --j )
        {
            SdrObject *pObj = pPage->GetObj( j );
            if ( pObj->IsWriterFlyFrame() )
            {
                SwFlyFrm *pF = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
                if ( pF->GetAnchor()->FindFlyFrm() == pFly )
                    PaintFlyChilds( pF, rOut, rInfoRec );
            }
            else
            {
                SwFrm *pFrm = ((SwDrawContact*)GetUserCall(pObj))->GetAnchor();
                if( pFrm && pFrm->FindFlyFrm() == pFly )
                {
                    pOut->Push( PUSH_LINECOLOR );
                    pObj->Paint( rOut, rInfoRec );
                    pOut->Pop();
                }
            }
        }
    }
    pFlyObj->Paint( rOut, rInfoRec );
}

/*************************************************************************
|*
|*  SwViewImp::IsDragPossible()
|*
|*  Ersterstellung      MA 19. Jan. 93
|*  Letzte Aenderung    MA 16. Jan. 95
|*
|*************************************************************************/



#define WIEDUWILLST 400

BOOL SwViewImp::IsDragPossible( const Point &rPoint )
{
    if ( !HasDrawView() )
        return FALSE;

    const SdrMarkList &rMrkList = GetDrawView()->GetMarkList();

    if( !rMrkList.GetMarkCount() )
        return FALSE;

    SdrObject *pO = rMrkList.GetMark(rMrkList.GetMarkCount()-1)->GetObj();

    SwRect aRect;
    if( ::CalcClipRect( pO, aRect, FALSE ) )
    {
        SwRect aTmp;
        ::CalcClipRect( pO, aTmp, TRUE );
        aRect.Union( aTmp );
    }
    else
        aRect = GetShell()->GetLayout()->Frm();

    aRect.Top(    aRect.Top()    - WIEDUWILLST );
    aRect.Bottom( aRect.Bottom() + WIEDUWILLST );
    aRect.Left(   aRect.Left()   - WIEDUWILLST );
    aRect.Right(  aRect.Right()  + WIEDUWILLST );
    return aRect.IsInside( rPoint );
}

/*************************************************************************
|*
|*  SwViewImp::NotifySizeChg()
|*
|*  Ersterstellung      MA 23. Jun. 93
|*  Letzte Aenderung    MA 05. Oct. 98
|*
|*************************************************************************/

void SwViewImp::NotifySizeChg( const Size &rNewSz )
{
    if ( !HasDrawView() )
        return;

    if ( GetPageView() )
        GetPageView()->GetPage()->SetSize( rNewSz );

    //Begrenzung des Arbeitsbereiches.
    Rectangle aRect( Point( DOCUMENTBORDER, DOCUMENTBORDER ), rNewSz );
    const Rectangle &rOldWork = GetDrawView()->GetWorkArea();
    BOOL bCheckDrawObjs = FALSE;
    if ( aRect != rOldWork )
    {
        if ( rOldWork.Bottom() > aRect.Bottom() || rOldWork.Right() > aRect.Right())
            bCheckDrawObjs = TRUE;
        GetDrawView()->SetWorkArea( aRect );
    }
    if ( !bCheckDrawObjs )
        return;

    ASSERT( pSh->GetDoc()->GetDrawModel(), "NotifySizeChg without DrawModel" );
    SdrPage* pPage = pSh->GetDoc()->GetDrawModel()->GetPage( 0 );
    const ULONG nObjs = pPage->GetObjCount();
    for( ULONG nObj = 0; nObj < nObjs; ++nObj )
    {
        SdrObject *pObj = pPage->GetObj( nObj );
        if( !pObj->IsWriterFlyFrame() )
        {
            //Teilfix(26793): Objekte, die in Rahmen verankert sind, brauchen
            //nicht angepasst werden.
            const SwDrawContact *pCont = (SwDrawContact*)GetUserCall(pObj);
            //JP - 16.3.00 Bug 73920: this function might be called by the
            //              InsertDocument, when a PageDesc-Attribute is
            //              set on a node. Then the SdrObject must not have
            //              an UserCall.
            if( !pCont )
                continue;

            ASSERT( pCont->ISA(SwDrawContact), "DrawObj, wrong UserCall" );
            const SwFrm *pAnchor = pCont->GetAnchor();
            if ( !pAnchor || pAnchor->IsInFly() || !pAnchor->IsValid() ||
                 !pAnchor->GetUpper() ||
                 FLY_IN_CNTNT == pCont->GetFmt()->GetAnchor().GetAnchorId() )
                continue;

            const Rectangle aBound( pObj->GetBoundRect() );
            if ( !aRect.IsInside( aBound ) )
            {
                Size aSz;
                if ( aBound.Left() > aRect.Right() )
                    aSz.Width() = (aRect.Right() - aBound.Left()) - MINFLY;
                if ( aBound.Top() > aRect.Bottom() )
                    aSz.Height() = (aRect.Bottom() - aBound.Top()) - MINFLY;
                if ( aSz.Width() || aSz.Height() )
                    pObj->Move( aSz );

                //Notanker: Grosse Objekte nicht nach oben verschwinden lassen.
                aSz.Width() = aSz.Height() = 0;
                if ( aBound.Bottom() < aRect.Top() )
                    aSz.Width() = (aBound.Bottom() - aRect.Top()) - MINFLY;
                if ( aBound.Right() < aRect.Left() )
                    aSz.Height() = (aBound.Right() - aRect.Left()) - MINFLY;
                if ( aSz.Width() || aSz.Height() )
                    pObj->Move( aSz );
            }
        }
    }
}

/****************************************************************************

    $Log: not supported by cvs2svn $
    Revision 1.117  2000/09/18 16:04:37  willem.vandorp
    OpenOffice header added.

    Revision 1.116  2000/03/16 21:27:20  jp
    Bug #73920#: NotifySizeChg - SdrObject UserCall pointer can be zero

    Revision 1.115  1999/09/09 15:04:06  hr
    #65293#: added missing closing brace

    Revision 1.114  1999/09/06 13:18:48  aw
    changes due to support of new handles


      Rev 1.112   13 Aug 1999 15:11:36   MA
   adoption to new markers, but still inkomplete

      Rev 1.111   23 Jul 1999 16:19:22   AW
   changes for new markers

      Rev 1.110   06 Jul 1999 19:11:22   JP
   Bug #67439#: PaintFlyChilds - check for null pointer

      Rev 1.109   08 Mar 1999 16:59:10   MA
   #62907# Notify noch ein wenig toleranter

      Rev 1.108   22 Feb 1999 08:35:12   MA
   1949globale Shell entsorgt, Shells am RootFrm

      Rev 1.107   07 Jan 1999 11:32:32   AMA
   Fix #60246#: Linienstyle restaurieren

      Rev 1.106   12 Nov 1998 14:17:24   MA
   #59385# XOR und UserMarker bekommen die nie richtig hin

      Rev 1.105   22 Oct 1998 14:00:14   MA
   #58316# Assertion richtig formuliert

      Rev 1.104   05 Oct 1998 11:09:18   MA
   #67489# Keine Objekte clippen wenn der Anker nicht Valid ist

      Rev 1.103   14 Sep 1998 13:49:00   MA
   #56335# ChainMarker

      Rev 1.102   29 Jun 1998 11:14:56   MA
   Push/Pop au fVCL umgestellt

      Rev 1.101   15 Jun 1998 11:04:10   AMA
   Chg: Gruppenobjekte duerfen jetzt bearbeitet werden => GetUserCall-Umbau

      Rev 1.100   10 Jun 1998 17:39:40   AMA
   New: SdrUnoObj statt VControls

      Rev 1.99   04 Jun 1998 18:21:26   AMA
   Chg: UNO-Controls jetzt im eigenen Drawing-Layer

      Rev 1.98   03 Jun 1998 09:25:24   MA
   #50392# Handles und Xor aufgeraeumt

      Rev 1.97   21 Nov 1997 15:59:48   TJ
   include fuer SdrPageView

      Rev 1.96   20 Nov 1997 12:37:30   MA
   includes

      Rev 1.95   03 Nov 1997 13:07:32   MA
   precomp entfernt

      Rev 1.94   29 Oct 1997 11:04:14   MA
   opt: Link auf Dispatcher bei Gruppenobjekten temporaer entfernen

      Rev 1.93   13 Oct 1997 10:30:36   MA
   Umbau/Vereinfachung Paint

      Rev 1.92   18 Aug 1997 10:37:06   OS
   includes

      Rev 1.91   15 Aug 1997 12:24:08   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.90   14 Apr 1997 11:30:32   MH
   add: header

      Rev 1.89   04 Apr 1997 16:55:00   MA
   opt+includes

      Rev 1.88   25 Feb 1997 08:45:44   MA
   chg: SolidHdl ueberm Berg

      Rev 1.87   16 Jan 1997 17:34:36   MA
   chg: Paint oder nicht sagt uns jetzt SwFlyFrm::IsPaint

      Rev 1.86   02 Dec 1996 19:38:52   MA
   #33918#

      Rev 1.85   02 Dec 1996 16:57:32   MA
   #33630# PaintMode fuer PreView

      Rev 1.84   02 Dec 1996 09:18:38   NF
   PaintFlyChild() ohne BOOL-Parameter...

      Rev 1.83   25 Nov 1996 12:09:26   MA
   fix: PaintFlyChilds, keinen auslassen

      Rev 1.82   20 Nov 1996 19:42:38   MA
   #33447# Paint auf Background schalten wenn Zeichenobjekte im Spiel sind

      Rev 1.81   20 Nov 1996 18:26:50   MA
   BorderRect wieder herausrechnen fuer Zeichenobj

      Rev 1.80   08 Nov 1996 13:00:14   HJS
   include w.g. positivdefine

      Rev 1.79   04 Nov 1996 20:01:36   MA
   #32989#

      Rev 1.78   25 Sep 1996 12:55:04   AMA
   Opt: ::CalcClipRect-Methode statt aClipMove/aClipStretch-Member

      Rev 1.77   25 Sep 1996 10:07:04   MA
   fix: CurShell setzen

      Rev 1.76   09 Sep 1996 09:55:56   MA
   opt: Protect vom DrawObj

**************************************************************************/


