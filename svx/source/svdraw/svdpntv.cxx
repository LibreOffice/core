/*************************************************************************
 *
 *  $RCSfile: svdpntv.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:25 $
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

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif

#include "svdpntv.hxx"
#include "editdata.hxx"
#include "svdmrkv.hxx"
#include "svdxout.hxx"
#include "svdpagv.hxx"
#include "svdpage.hxx"
#include "svdmodel.hxx"
#include "svdvmark.hxx"
#include "svdio.hxx"
#include "svdundo.hxx"
#include "svdview.hxx"
#include "svdglue.hxx"
#include "svdobj.hxx"
#include "svdograf.hxx"
#include "svdattrx.hxx"
#include "svdibrow.hxx"
#include "svditer.hxx"
#include "svdouno.hxx"

#ifndef _EEITEM_HXX //autogen
#include <eeitem.hxx>
#endif

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _B2D_MBMP_HXX
#include <goodies/b2dmbmp.hxx>
#endif

#ifndef _B2D_MTRI_HXX
#include <goodies/b2dmtri.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////

void RollingRect::DrawHor(OutputDevice& rOut, long x1, long x2, long y, BOOL bMov) const
{
    USHORT nLen2=nLen*2;
    BOOL bBck=x1>x2;
    long nOfs=nIdx;
    if (bBck) {
        long nTmp;
        nTmp=x1-x2+1+nLen2+nLen;
        nTmp%=nLen2;
        nOfs=nLen2-nOfs+nTmp;
        nTmp=x1; x1=x2; x2=nTmp;
    }
    while (nOfs>0) nOfs-=nLen2;
    long a1,a2,a;
    a=x1+nOfs;
    while (a<=x2+1) {
        a1=a;
        a2=a+(nLen-1);
        if (bMov) {
            if (!bBck) a2++; else a1--;
            if (a1>=x1 && a1<=x2) ((Window&)rOut).Invert(Rectangle(a1,y,a1,y));
            if (a2>=x1 && a2<=x2) ((Window&)rOut).Invert(Rectangle(a2,y,a2,y));
        } else {
            if (a1<x1) a1=x1;
            if (a2>x2) a2=x2;
            if (a1<=a2) ((Window&)rOut).Invert(Rectangle(a1,y,a2,y));
        }
        a+=nLen2;
    }
}

void RollingRect::DrawVer(OutputDevice& rOut, long x, long y1, long y2, BOOL bMov) const
{
    USHORT nLen2=nLen*2;
    BOOL bBck=y1>y2;
    long nOfs=nIdx;
    if (bBck) {
        long nTmp;
        nTmp=y1-y2+1+nLen2+nLen;
        nTmp%=nLen2;
        nOfs=nLen2-nOfs+nTmp;
        nTmp=y1; y1=y2; y2=nTmp;
    }
    while (nOfs>0) nOfs-=nLen2;
    long a1,a2,a;
    a=y1+nOfs;
    while (a<=y2+1) {
        a1=a;
        a2=a+nLen-1;
        if (bMov) {
            if (!bBck) a2++; else a1--;
            if (a1>=y1 && a1<=y2) ((Window&)rOut).Invert(Rectangle(x,a1,x,a1));
            if (a2>=y1 && a2<=y2) ((Window&)rOut).Invert(Rectangle(x,a2,x,a2));
        } else {
            if (a1<y1) a1=y1;
            if (a2>y2) a2=y2;
            if (a1<=a2) ((Window&)rOut).Invert(Rectangle(x,a1,x,a2));
        }
        a+=nLen2;
    }
}

void RollingRect::DrawRect(OutputDevice& rOut, BOOL bMov) const
{
    BOOL bMap0=rOut.IsMapModeEnabled();
    Point aPt1(rOut.LogicToPixel(aP1));
    Point aPt2(rOut.LogicToPixel(aP2));
    rOut.EnableMapMode(FALSE);
    Size aS(rOut.GetOutputSizePixel());
    long dx=Abs(aPt2.X()-aPt1.X());
    long dy=Abs(aPt2.Y()-aPt1.Y());
    if (IsStripes()) {
        DrawHor(rOut,aPt1.X(),0,aPt1.Y(),bMov);
        DrawHor(rOut,aPt2.X(),aS.Width(),aPt1.Y(),bMov);
        if (dy!=0) {
            DrawHor(rOut,aPt1.X(),0,aPt2.Y(),bMov);
            DrawHor(rOut,aPt2.X(),aS.Width(),aPt2.Y(),bMov);
        }
        DrawVer(rOut,aPt1.X(),aPt1.Y(),0,bMov);
        DrawVer(rOut,aPt1.X(),aPt2.Y(),aS.Height(),bMov);
        if (dx!=0) {
            DrawVer(rOut,aPt2.X(),aPt1.Y(),0,bMov);
            DrawVer(rOut,aPt2.X(),aPt2.Y(),aS.Height(),bMov);
        }
    } else if (IsCrossHair()) {
        DrawHor(rOut,aPt1.X(),0,aPt1.Y(),bMov);
        DrawHor(rOut,aPt1.X(),aS.Width(),aPt1.Y(),bMov);
        DrawVer(rOut,aPt1.X(),aPt1.Y(),0,bMov);
        DrawVer(rOut,aPt1.X(),aPt1.Y(),aS.Height(),bMov);
    } else {
        if (dx!=0) {
            DrawHor(rOut,aPt1.X(),aPt2.X(),aPt1.Y(),bMov);
            if (dy>0) DrawHor(rOut,aPt2.X(),aPt1.X(),aPt2.Y(),bMov);
        }
        if (dy!=0 || (dy==1 && dx>1)) {
            DrawVer(rOut,aPt1.X(),aPt1.Y(),aPt2.Y(),bMov);
            if (dx>0) DrawVer(rOut,aPt2.X(),aPt2.Y(),aPt1.Y(),bMov);
        }
    }
    rOut.EnableMapMode(bMap0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameAnimator::FrameAnimator(SdrView& rNewView):
    rView(rNewView)
{
    aTim.SetTimeoutHdl(LINK(this,FrameAnimator,Hdl));
    aTim.SetTimeout(50);
    pOut=NULL;
}

void FrameAnimator::Start() const
{
    if (!Application::IsRemoteServer())
    {
          // Die Animation ist in der Remote-Version zu langsam.
          // Daher nur in der Nicht-Remote-Version Animation starten
          ((FrameAnimator*)this)->aTim.Start();
    }
}

void FrameAnimator::Stop() const
{
    ((FrameAnimator*)this)->aTim.Stop();
}

IMPL_LINK(FrameAnimator,Hdl,AutoTimer*,pTim)
{
    if (rView.aDragStat.IsShown()) {
        USHORT i=0;
        do {
            OutputDevice* pO=rView.pDragWin;
            if (pO==NULL) {
                pO=rView.GetWin(i);
                i++;
            }
            if (pO!=NULL) {
                RollIt(*pO,FALSE);
            }
        } while (pOut==NULL && i<rView.GetWinCount());
        IncRollIdx();
    }
    return 0;
}

void FrameAnimator::Invert(OutputDevice* pNewOut) const
{
    USHORT i=0;
    ((FrameAnimator*)this)->pOut=pNewOut;
    do {
        OutputDevice* pO=pNewOut;
        if (pO==NULL) {
            pO=rView.GetWin(i);
            i++;
        }
        if (pO!=NULL) {
            RollingRect::Invert(*pO);
        }
    } while (pOut==NULL && i<rView.GetWinCount());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//#define   TEST_IAO

SdrViewWinRec::SdrViewWinRec(OutputDevice* pW)
:   pWin(pW),
//  pVDev(NULL),
    pIAOManager(NULL),
    bXorVisible(FALSE)
{
    // is it a window?
    if(pW && pW->GetOutDevType() == OUTDEV_WINDOW)
    {
        // create B2dIAOManager for this window
        pIAOManager = new B2dIAOManager((Window*)pW);
#ifdef TEST_IAO
        if(pIAOManager)
        {
            // create some demo shapes
            B2dIAOLine* pLine;
            B2dIAOMarker* pMarker;
            B2dIAOBitmapObj* pBitmap;
            B2dIAOTriangle* pTriangle;

            static BOOL bCreateLines = FALSE;
            static BOOL bCreateMarkers = FALSE;
            static BOOL bCreateField = FALSE;
            static BOOL bCreateSingleMarker = FALSE;
            static BOOL bCreateBitmap = FALSE;
            static BOOL bCreateBitmapField = FALSE;
            static BOOL bCreateTriangle = FALSE;

            if(bCreateLines)
            {
                pLine = new B2dIAOLine(pIAOManager, Point(5000, 5200), Point(5000, 7000));
                pLine->SetBaseColor(Color(COL_CYAN));
                pLine = new B2dIAOLineStriped(pIAOManager, Point(5100, 5200), Point(5100, 7000));
                pLine->SetBaseColor(Color(COL_BLUE));
                B2dIAOLineTwoColor* p2Line = new B2dIAOLineTwoColor(pIAOManager, Point(5200, 5200), Point(5200, 7000));
                p2Line->SetBaseColor(Color(COL_YELLOW));
                p2Line->Set2ndColor(Color(COL_BLACK));
                B2dIAOLineTwoColorAnim* p3Line = new B2dIAOLineTwoColorAnim(pIAOManager, Point(5300, 5200), Point(5300, 7000));
                p3Line->SetBaseColor(Color(COL_YELLOW));
                p3Line->Set2ndColor(Color(COL_BLACK));
            }

            if(bCreateMarkers)
            {
                pMarker = new B2dIAOMarker(pIAOManager, Point(5000, 5000), B2D_IAO_MARKER_POINT);
                pMarker->SetBaseColor(Color(COL_LIGHTRED));
                pMarker = new B2dIAOMarker(pIAOManager, Point(5100, 5000), B2D_IAO_MARKER_PLUS);
                pMarker->SetBaseColor(Color(COL_LIGHTGREEN));
                pMarker = new B2dIAOMarker(pIAOManager, Point(5200, 5000), B2D_IAO_MARKER_CROSS);
                pMarker->SetBaseColor(Color(COL_LIGHTBLUE));

                pMarker = new B2dIAOMarker(pIAOManager, Point(5000, 5100), B2D_IAO_MARKER_RECT_5X5);
                pMarker->SetBaseColor(Color(COL_LIGHTGREEN));
                pMarker->Set2ndColor(Color(COL_BLACK));
                pMarker = new B2dIAOMarker(pIAOManager, Point(5100, 5100), B2D_IAO_MARKER_RECT_7X7);
                pMarker->SetBaseColor(Color(COL_LIGHTGREEN));
                pMarker->Set2ndColor(Color(COL_BLACK));
                pMarker = new B2dIAOMarker(pIAOManager, Point(5200, 5100), B2D_IAO_MARKER_RECT_9X9);
                pMarker->SetBaseColor(Color(COL_LIGHTGREEN));
                pMarker->Set2ndColor(Color(COL_BLACK));
            }

            if(bCreateField || bCreateBitmapField)
            {
                static UINT16 nNumX = 10;
                static UINT16 nNumY = 10;
                static UINT16 nStart = 2000;
                static UINT16 nEnd = 16000;

                for(UINT16 a=nStart;a<nEnd;a+=(nEnd-nStart)/nNumX)
                {
                    for(UINT16 b=nStart;b<nEnd;b+=(nEnd-nStart)/nNumY)
                    {
                        if(bCreateField)
                        {
                            pMarker = new B2dIAOMarker(pIAOManager, Point(a, b), B2D_IAO_MARKER_RECT_7X7);
                            pMarker->SetBaseColor(Color(
                                (((a-nStart)*256L)/(nEnd-nStart)),
                                (((b-nStart)*256L)/(nEnd-nStart)),
                                0x80));
                            pMarker->Set2ndColor(Color(COL_BLACK));
                        }
                        if(bCreateBitmapField)
                        {
                            Bitmap aBitmap(Size(10, 10), 8);
                            pBitmap = new B2dIAOBitmapObj(pIAOManager, Point(a, b), aBitmap);
                        }
                    }
                }
            }
            if(bCreateSingleMarker)
            {
                pMarker = new B2dIAOMarker(pIAOManager, Point(5000, 5000), B2D_IAO_MARKER_RECT_7X7);
                pMarker->SetBaseColor(Color(COL_LIGHTGREEN));
                pMarker->Set2ndColor(Color(COL_BLACK));
            }
            if(bCreateBitmap)
            {
                Bitmap aBitmap(Size(10, 10), 8);
                pBitmap = new B2dIAOBitmapObj(pIAOManager, Point(6000, 6000), aBitmap);
            }
            if(bCreateTriangle)
            {
                pTriangle = new B2dIAOTriangle(pIAOManager, Point(5000, 5000), Point(7000, 5000), Point(6000, 7000), Color(COL_YELLOW));
                pLine = new B2dIAOLine(pIAOManager, pTriangle->GetBasePosition(), pTriangle->Get2ndPosition());
                pLine = new B2dIAOLine(pIAOManager, pTriangle->Get2ndPosition(), pTriangle->Get3rdPosition());
                pLine = new B2dIAOLine(pIAOManager, pTriangle->Get3rdPosition(), pTriangle->GetBasePosition());

                pTriangle = new B2dIAOBitmapTriangle(pIAOManager, Point(8000, 5000), Point(10000, 5000), Point(9000, 7000), Color(COL_RED));
                pLine = new B2dIAOLine(pIAOManager, pTriangle->GetBasePosition(), pTriangle->Get2ndPosition());
                pLine = new B2dIAOLine(pIAOManager, pTriangle->Get2ndPosition(), pTriangle->Get3rdPosition());
                pLine = new B2dIAOLine(pIAOManager, pTriangle->Get3rdPosition(), pTriangle->GetBasePosition());
            }
        }
#endif
    }
}

SdrViewWinRec::~SdrViewWinRec()
{
//  if (pVDev!=NULL)
//      delete pVDev;

    // cleanup IAOManager for this window
    if(pIAOManager)
        delete pIAOManager;
    pIAOManager = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrViewWinList::Clear()
{
    USHORT nAnz=GetCount();
    for (USHORT i=0; i<nAnz; i++) {
        delete GetObject(i);
    }
    aList.Clear();
}

USHORT SdrViewWinList::Find(OutputDevice* pW) const
{
    USHORT nAnz=GetCount();
    USHORT nRet=SDRVIEWWIN_NOTFOUND;
    for (USHORT nNum=0; nNum<nAnz && nRet==SDRVIEWWIN_NOTFOUND; nNum++) {
        if (GetObject(nNum)->pWin==pW) nRet=nNum;
    }
    return nRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@   @@@@  @@ @@  @@ @@@@@@  @@ @@ @@ @@@@@ @@   @@
//  @@  @@ @@  @@ @@ @@@ @@   @@    @@ @@ @@ @@    @@   @@
//  @@  @@ @@  @@ @@ @@@@@@   @@    @@ @@ @@ @@    @@ @ @@
//  @@@@@  @@@@@@ @@ @@@@@@   @@    @@@@@ @@ @@@@  @@@@@@@
//  @@     @@  @@ @@ @@ @@@   @@     @@@  @@ @@    @@@@@@@
//  @@     @@  @@ @@ @@  @@   @@     @@@  @@ @@    @@@ @@@
//  @@     @@  @@ @@ @@  @@   @@      @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT2(SdrPaintView,SfxListener,SfxRepeatTarget);

DBG_NAME(SdrPaintView);

void SdrPaintView::ImpClearVars()
{
    pXOut=NULL;
    bForeignXOut=FALSE;
    pItemBrowser=NULL;
    bLayerSortedRedraw=FALSE;
    bPageVisible=TRUE;
    bBordVisible=TRUE;
    bGridVisible=TRUE;
    bGridFront  =FALSE;
    bHlplVisible=TRUE;
    bHlplFront  =TRUE;
    bGlueVisible=FALSE;
    bGlueVisible2=FALSE;
    bGlueVisible3=FALSE;
    bGlueVisible4=FALSE;
    bSwapAsynchron=FALSE;
    bPrintPreview=FALSE;
    bLineDraft=FALSE;
    bFillDraft=FALSE;
    bGrafDraft=FALSE;
    bHideGrafDraft=FALSE;
    bTextDraft=FALSE;
    bLineDraftPrn=FALSE;
    bFillDraftPrn=FALSE;
    bGrafDraftPrn=FALSE;
    bTextDraftPrn=FALSE;

    bObjectPaintIgnoresClipping=FALSE;

    eAnimationMode = SDR_ANIMATION_ANIMATE;

    nHitTolPix=2;
    nMinMovPix=3;
    nHitTolLog=0;
    nMinMovLog=0;
    pActualOutDev=NULL;

    bSaveHiddenPages=FALSE;
    bPageTwice=FALSE;
    pDragWin=NULL;
    bRestoreColors=TRUE;
    pDisabledAttr=NULL;
    pDefaultStyleSheet=NULL;
    bEncircle=FALSE;
    bSomeObjChgdFlag=FALSE;

    bMasterBmp=FALSE;
    pMasterBmp=NULL;
    nMasterCacheMode = SDR_MASTERPAGECACHE_DEFAULT;

    aComeBackTimer.SetTimeout(1);
    aComeBackTimer.SetTimeoutHdl(LINK(this,SdrPaintView,ImpComeBackHdl));
    aAfterPaintTimer.SetTimeout(1);
    aAfterPaintTimer.SetTimeoutHdl(LINK(this,SdrPaintView,ImpAfterPaintHdl));
    aUserMarkerAnimator.SetTimeout(50);
    aUserMarkerAnimator.SetTimeoutHdl(LINK(this,SdrPaintView,ImpUserMarkerAnimatorHdl));

    String aNam;    // System::GetUserName() just return an empty string

    if (pMod)
        SetDefaultStyleSheet(pMod->GetDefaultStyleSheet(), TRUE);

    aNam.ToUpperAscii();
}


SdrPaintView::SdrPaintView(SdrModel* pModel1, OutputDevice* pOut):
    aPagV(1024,16,16),
    aPagHide(1024,16,16),
    aAni(*(SdrView*)this),
    aDefaultAttr(pModel1->GetItemPool()),
    aUserMarkers(1024,16,16)
{
    DBG_CTOR(SdrPaintView,NULL);
    pMod=pModel1;
    ImpClearVars();
    if (pOut!=NULL) AddWin(pOut);
    pXOut=new ExtOutputDevice(pOut);

    // Flag zur Visualisierung von Gruppen
    bVisualizeEnteredGroup = TRUE;
}

SdrPaintView::SdrPaintView(SdrModel* pModel1, ExtOutputDevice* pExtOut):
    aPagV(1024,16,16),
    aPagHide(1024,16,16),
    aAni(*(SdrView*)this),
    aDefaultAttr(pModel1->GetItemPool()),
    aUserMarkers(1024,16,16)
{
    DBG_CTOR(SdrPaintView,NULL);
    pMod=pModel1;
    ImpClearVars();
    pMod=pModel1;
    if (pExtOut!=NULL) {
        bForeignXOut=TRUE;
        pXOut=pExtOut;
        OutputDevice* pO=pXOut->GetOutDev();
        if (pO!=NULL) AddWin(pO);
    } else {
        pXOut=new ExtOutputDevice(NULL);
    }

    // Flag zur Visualisierung von Gruppen
    bVisualizeEnteredGroup = TRUE;
}

SdrPaintView::~SdrPaintView()
{
    DBG_DTOR(SdrPaintView,NULL);
    aAfterPaintTimer.Stop();

    ClearAll();
    if (!bForeignXOut && pXOut!=NULL) {
        delete pXOut;
    }
    if (pDisabledAttr!=NULL) {
        delete pDisabledAttr;
    }
    if (pMasterBmp!=NULL) {
        delete pMasterBmp;
    }
    if (pItemBrowser!=NULL) {
        delete pItemBrowser;
    }
    USHORT nAnz=ImpGetUserMarkerCount();
    for (USHORT nNum=0; nNum<nAnz; nNum++) {
        SdrViewUserMarker* pUM=ImpGetUserMarker(nNum);
        pUM->pView=NULL; // Weil's mich gleich nichtmehr gibt.
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void __EXPORT SdrPaintView::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
{
    BOOL bObjChg=!bSomeObjChgdFlag; // TRUE= auswerten fuer ComeBack-Timer
    BOOL bMaster=pMasterBmp!=NULL;  // TRUE= auswerten fuer MasterPagePaintCache
    if (bObjChg || bMaster) {
        SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
        if (pSdrHint!=NULL) {
            SdrHintKind eKind=pSdrHint->GetKind();
            if (eKind==HINT_OBJCHG || eKind==HINT_OBJINSERTED || eKind==HINT_OBJREMOVED) {
                if (bObjChg) {
                    bSomeObjChgdFlag=TRUE;
                    aComeBackTimer.Start();
                }
                if (bMaster) {
                    const SdrPage* pPg=pSdrHint->GetPage();
                    if (pPg!=NULL && pPg->IsMasterPage() && pPg->GetPageNum()==pMasterBmp->GetMasterPageNum()) {
                        ReleaseMasterPagePaintCache();
                    }
                }
            }
            if (eKind==HINT_PAGEORDERCHG) {
                const SdrPage* pPg=pSdrHint->GetPage();
                if (!pPg->IsInserted()) { // aha, Seite geloescht: also hiden
                    USHORT nAnz=GetPageViewCount();
                    USHORT nv;
                    for (nv=nAnz; nv>0;) {
                        nv--;
                        SdrPageView* pPV=GetPageViewPvNum(nv);
                        if (pPV->GetPage()==pPg) {
                            HidePagePvNum(nv);
                        }
                    }
                }
                if (bMaster) ReleaseMasterPagePaintCache();
            }
            if (eKind==HINT_PAGECHG) {
                if (bMaster) ReleaseMasterPagePaintCache();
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IMPL_LINK_INLINE_START(SdrPaintView,ImpComeBackHdl,Timer*,pTimer)
{
    if (bSomeObjChgdFlag) {
        bSomeObjChgdFlag=FALSE;
        ModelHasChanged();
    }
    return 0;
}

IMPL_LINK_INLINE_END(SdrPaintView,ImpComeBackHdl,Timer*,pTimer)

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ImpAsyncStruct
{
    const SdrObject*    mpObj;
    const OutputDevice* mpOut;
    const Rectangle     maRectPix;

                        ImpAsyncStruct( const SdrObject* pObj, const OutputDevice* pOut ) :
                            mpObj( pObj ), mpOut( pOut ), maRectPix( pOut->LogicToPixel( pObj->GetBoundRect() ) ) {}
};

void SdrPaintView::ImpAddAsyncObj( const SdrObject* pObj, const OutputDevice* pOut )
{
    aAsyncPaintList.Insert( new ImpAsyncStruct( pObj, pOut ), LIST_APPEND );
}

IMPL_LINK(SdrPaintView,ImpAfterPaintHdl,Timer*,pTimer)
{
    while( aAsyncPaintList.Count() )
    {
        Rectangle           aInvRect;
        const OutputDevice* pOut = NULL;

        for( void* p = aAsyncPaintList.First(); p;  )
        {
            ImpAsyncStruct* pAsync = (ImpAsyncStruct*) p;
            BOOL            bMatch = TRUE;

            if( pAsync->mpObj && pAsync->mpObj->ISA( SdrGrafObj) )
                ( (SdrGrafObj*) pAsync->mpObj )->ForceSwapIn();

            if( !pOut )
                pOut = pAsync->mpOut;
            else if( pOut != pAsync->mpOut )
                bMatch = FALSE;

            if( bMatch )
            {
                aInvRect.Union( pAsync->maRectPix );
                delete (ImpAsyncStruct*) aAsyncPaintList.Remove();
                p = aAsyncPaintList.GetCurObject();
            }
            else
                p = aAsyncPaintList.Next();
        }

        if( OUTDEV_WINDOW == pOut->GetOutDevType() )
            ( (Window*) pOut )->Invalidate( pOut->PixelToLogic( aInvRect ) );
    }

    ((SdrMarkView*)this)->ImpAfterPaint();

    return 0;
}

void SdrPaintView::FlushComeBackTimer() const
{
    if (bSomeObjChgdFlag) {
        // casting auf nonconst
        ((SdrPaintView*)this)->ImpComeBackHdl(&((SdrPaintView*)this)->aComeBackTimer);
        ((SdrPaintView*)this)->aComeBackTimer.Stop();
    }
}

void SdrPaintView::ModelHasChanged()
{
    // Auch alle PageViews benachrichtigen
    USHORT nv;
    USHORT nAnz;
    nAnz=GetPageViewCount();
    for (nv=nAnz; nv>0;) {
        nv--;
        SdrPageView* pPV=GetPageViewPvNum(nv);
        if (!pPV->GetPage()->IsInserted()) {
            HidePage(pPV);
        }
    }
    nAnz=GetPageViewCount();
    for (nv=0; nv<nAnz; nv++) {
        SdrPageView* pPV=GetPageViewPvNum(nv);
        pPV->ModelHasChanged();
    }
    nAnz=GetPageHideCount();
    for (nv=0; nv<nAnz; nv++) {
        SdrPageView* pPV=GetPageHidePvNum(nv);
        pPV->ModelHasChanged();
    }
    if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IMPL_LINK_INLINE_START(SdrPaintView,ImpUserMarkerAnimatorHdl,AutoTimer*,pTimer)
{
    USHORT nAnz=ImpGetUserMarkerCount();
    for (USHORT nNum=0; nNum<nAnz; nNum++) {
        SdrViewUserMarker* pUM=ImpGetUserMarker(nNum);
        if (pUM->IsAnimate() && pUM->IsVisible()) pUM->DoAnimateOneStep();
    }
    return 0;
}

IMPL_LINK_INLINE_END(SdrPaintView,ImpUserMarkerAnimatorHdl,AutoTimer*,pTimer)

void SdrPaintView::ImpInsertUserMarker(SdrViewUserMarker* pMarker)
{
    aUserMarkers.Insert(pMarker);
    ImpCheckMarkerAnimator();
}

void SdrPaintView::ImpRemoveUserMarker(SdrViewUserMarker* pMarker)
{
    aUserMarkers.Remove(pMarker);
    ImpCheckMarkerAnimator();
}

void SdrPaintView::ImpCheckMarkerAnimator()
{
    BOOL bNeed=FALSE;
    USHORT nAnz=ImpGetUserMarkerCount();
    for (USHORT nNum=0; nNum<nAnz && !bNeed; nNum++) {
        SdrViewUserMarker* pUM=ImpGetUserMarker(nNum);
        bNeed=pUM->IsAnimate();
    }
    if (bNeed) aUserMarkerAnimator.Start();
    else aUserMarkerAnimator.Stop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrPaintView::IsAction() const
{
    return IsEncirclement();
}

void SdrPaintView::MovAction(const Point& rPnt)
{
    if (IsEncirclement()) {
        MovEncirclement(rPnt);
    }
}

void SdrPaintView::EndAction()
{
    if (IsEncirclement()) EndEncirclement();
}

void SdrPaintView::BckAction()
{
    BrkEncirclement();
}

void SdrPaintView::BrkAction()
{
    BrkEncirclement();
}

void SdrPaintView::TakeActionRect(Rectangle& rRect) const
{
    if (IsEncirclement()) {
        rRect=Rectangle(aDragStat.GetStart(),aDragStat.GetNow());
    }
}

void SdrPaintView::ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const
{
    if (IsEncirclement() && aDragStat.IsShown()) {
        DrawEncirclement(pOut);
    }
    USHORT nAnz=ImpGetUserMarkerCount();
    for (USHORT nNum=0; nNum<nAnz; nNum++) {
        SdrViewUserMarker* pUM=ImpGetUserMarker(nNum);
        if (pUM->IsVisible()) pUM->Draw(pOut,FALSE,!bRestoreColors); // den 3. Parameter hier noch richtig setzen !!!!!
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

USHORT SdrPaintView::ImpGetMinMovLogic(short nMinMov, const OutputDevice* pOut) const
{
    if (nMinMov>=0) return USHORT(nMinMov);
    if (pOut==NULL) pOut=GetWin(0);
    if (pOut!=NULL) {
        return short(-pOut->PixelToLogic(Size(nMinMov,0)).Width());
    } else {
        return 0;
    }
}

USHORT SdrPaintView::ImpGetHitTolLogic(short nHitTol, const OutputDevice* pOut) const
{
    if (nHitTol>=0) return USHORT(nHitTol);
    if (pOut==NULL) pOut=GetWin(0);
    if (pOut!=NULL) {
        return short(-pOut->PixelToLogic(Size(nHitTol,0)).Width());
    } else {
        return 0;
    }
}

void SdrPaintView::TheresNewMapMode()
{
    if (pActualOutDev!=NULL) {
        nHitTolLog=(USHORT)((OutputDevice*)pActualOutDev)->PixelToLogic(Size(nHitTolPix,0)).Width();
        nMinMovLog=(USHORT)((OutputDevice*)pActualOutDev)->PixelToLogic(Size(nMinMovPix,0)).Width();
    }
}

void SdrPaintView::SetActualWin(const OutputDevice* pWin)
{
    pActualOutDev=pWin;
    TheresNewMapMode();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintView::BegEncirclement(const Point& rPnt, OutputDevice* pOut, short nMinMov)
{
    BrkAction();
    aDragStat.Reset(rPnt);
    aDragStat.SetMinMove(ImpGetMinMovLogic(nMinMov,pOut));
    if (nMinMov==0) aDragStat.SetMinMoved();
    aAni.Reset();
    aDragStat.NextPoint();
    pDragWin=pOut;
    bEncircle=TRUE;
    if (aDragStat.IsMinMoved()) ShowEncirclement(pOut);
}

void SdrPaintView::MovEncirclement(const Point& rPnt)
{
    if (IsEncirclement()) {
        if (aDragStat.IsMinMoved()) HideEncirclement(pDragWin);
        aDragStat.NextMove(rPnt);
        if (aDragStat.CheckMinMoved(rPnt)) ShowEncirclement(pDragWin);
    }
}

Rectangle SdrPaintView::EndEncirclement(BOOL bNoJustify)
{
    if (IsEncirclement() && aDragStat.IsMinMoved()) {
        HideEncirclement(pDragWin);
        bEncircle=FALSE;
        Rectangle aRect(aDragStat.GetStart(),aDragStat.GetNow());
        if (!bNoJustify) aRect.Justify();
        return aRect;
    }
    bEncircle=FALSE;
    return Rectangle();
}

void SdrPaintView::BrkEncirclement()
{
    if (IsEncirclement()) {
        HideEncirclement(pDragWin);
        bEncircle=FALSE;
    }
}

void SdrPaintView::ShowEncirclement(OutputDevice* pOut)
{
    if (IsEncirclement() && !aDragStat.IsShown()) {
        DrawEncirclement(pOut);
        aDragStat.SetShown(TRUE);
        aAni.Start();
    }
}

void SdrPaintView::HideEncirclement(OutputDevice* pOut)
{
    if (IsEncirclement() && aDragStat.IsShown()) {
        aAni.Stop();
        DrawEncirclement(pOut);
        aDragStat.SetShown(FALSE);
    }
}

void SdrPaintView::DrawEncirclement(OutputDevice* pOut) const
{
    if (IsEncirclement()) {
        aAni.SetP1(aDragStat.GetStart());
        aAni.SetP2(aDragStat.GetNow());
        aAni.Invert(pOut);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintView::ShowShownXor(OutputDevice* pOut, BOOL bShow)
{
    USHORT nAnz=GetWinCount();
    USHORT nw=0;
    BOOL bWeiter=TRUE;
    do {
        if (pOut!=NULL) {
            nw=aWinList.Find(pOut);
            bWeiter=FALSE;
        }
        if (nw<nAnz && nw!=SDRVIEWWIN_NOTFOUND) {
            BOOL bOn=IsShownXorVisibleWinNum(nw);
            if (bOn!=bShow) {
                ToggleShownXor(GetWin(nw),NULL);
                SetShownXorVisible(nw,bShow);
            }
        } else bWeiter=FALSE;
    } while (bWeiter);
}

BOOL SdrPaintView::IsShownXorVisible(OutputDevice* pOut) const
{
    USHORT nPos=aWinList.Find(pOut);
    if (nPos!=SDRVIEWWIN_NOTFOUND) {
        return IsShownXorVisibleWinNum(nPos);
    } else {
        return FALSE;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintView::ClearPageViews()
{
    BrkAction();
    for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
        SdrPageView* pPV=GetPageViewPvNum(nv);
        InvalidateAllWin(pPV->GetPageRect());
        delete pPV;
    }
    aPagV.Clear();
}

void SdrPaintView::ClearHideViews()
{
    for (USHORT nh=0; nh<GetPageHideCount(); nh++) {
        SdrPageView* pPV=GetPageHidePvNum(nh);
        delete pPV;
    }
    aPagHide.Clear();
}

void SdrPaintView::Clear()
{
    ClearPageViews();
    ClearHideViews();
}

void SdrPaintView::ClearAll()
{
    for( void* p = aAsyncPaintList.First(); p; p = aAsyncPaintList.Next() )
        delete (ImpAsyncStruct*) p;

    aAsyncPaintList.Clear();
    ClearPageViews();
    ClearHideViews();
}

SdrPageView* SdrPaintView::ShowPage(SdrPage* pPage, const Point& rOffs)
{
    SdrPageView* pPV=NULL;
    if (pPage!=NULL) {
        SdrPageView* pTmpPV=NULL;
        if (!bPageTwice) pTmpPV=GetPageView(pPage); // Evtl. jede Seite nur einmal!
        if (pTmpPV==NULL) {
            USHORT nPos=GetHiddenPV(pPage);   // War die schon mal da?
            if (nPos<GetPageHideCount()) {
                pPV=GetPageHidePvNum(nPos);
                aPagHide.Remove(nPos);
                pPV->SetOffset(rOffs);
            } else {
                pPV=new SdrPageView(pPage,rOffs,*((SdrView*)this));
            }
            if (pPV!=NULL) {
                aPagV.Insert(pPV,CONTAINER_APPEND);
                pPV->Show();
            }
        }
    }
    return pPV;
}

SdrPageView* SdrPaintView::ShowPagePgNum(USHORT nPgNum, const Point& rOffs)
{
    return ShowPage(pMod->GetPage(nPgNum),rOffs);
}

SdrPageView* SdrPaintView::ShowMasterPagePgNum(USHORT nPgNum, const Point& rOffs)
{
    return ShowPage(pMod->GetMasterPage(nPgNum),rOffs);
}

void SdrPaintView::HidePage(SdrPageView* pPV)
{
    if (pPV!=NULL) {
        ULONG nPos=aPagV.GetPos(pPV);
        if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
            aPagV.Remove(nPos);
            pPV->Hide();
            if (bSaveHiddenPages) {
                aPagHide.Insert(pPV,CONTAINER_APPEND);
            } else {
                delete pPV;
            }
        }
    }
}

void SdrPaintView::HidePagePgNum(USHORT nPgNum)
{
    HidePage(pMod->GetPage(nPgNum));
}

void SdrPaintView::HideAllPages()
{
    while (GetPageViewCount()>0) HidePagePvNum(0);
}

void SdrPaintView::SetPagePos(SdrPageView* pPV, const Point& rOffs)
{
    if (pPV!=NULL) {
        pPV->SetOffset(rOffs);
    }
}

void SdrPaintView::SetPagePosPgNum(USHORT nPgNum, const Point& rOffs)
{
    SetPagePos(pMod->GetPage(nPgNum),rOffs);
}

SdrPageView* SdrPaintView::GetPageView(const SdrPage* pPage) const
{
    if (pPage==NULL) return NULL;
    BOOL bWeiter=TRUE;
    SdrPageView* pPV=NULL;
    for (USHORT i=0; i<GetPageViewCount() && bWeiter; i++) {
        pPV=GetPageViewPvNum(i);
        bWeiter=(pPV->GetPage()!=pPage);
    }
    if (bWeiter) return NULL;
    else return pPV;
}

SdrPageView* SdrPaintView::GetPageViewPgNum(USHORT nPgNum) const
{
    return GetPageView(pMod->GetPage(nPgNum));
}

SdrPageView* SdrPaintView::GetPageView(const Point& rPnt) const
{
    SdrPageView* pHit=NULL;
    SdrPageView* pBest=GetPageViewPvNum(0);
    ULONG        nBest=0xFFFFFFFF;

    for (USHORT i=GetPageViewCount(); i>0 && pHit==NULL;) {
        i--;
        SdrPageView* pPV=GetPageViewPvNum(i);
        Rectangle aRect(pPV->GetPageRect());
        if (aRect.IsInside(rPnt)) {
            pHit=pPV;
            pBest=pHit;
        } else {
            ULONG dx=0,dy=0;
            if (rPnt.X()<aRect.Left  ()) dx=ULONG(aRect.Left  ()-rPnt.X());
            if (rPnt.Y()<aRect.Top   ()) dy=ULONG(aRect.Top   ()-rPnt.Y());
            if (rPnt.X()>aRect.Right ()) dx=ULONG(rPnt.X()-aRect.Left  ());
            if (rPnt.Y()>aRect.Bottom()) dy=ULONG(rPnt.Y()-aRect.Bottom());
            ULONG nDist=dx+dy;
            if (nDist<nBest) {
                nBest=nDist;
                pBest=pPV;
            }
        }
    }
    return pBest;
}

USHORT SdrPaintView::GetPageViewNum(const SdrPageView* pPV) const
{
    if (pPV==NULL) return 0xFFFF;
    ULONG nNum=aPagV.GetPos(pPV);
    if (nNum==CONTAINER_ENTRY_NOTFOUND) nNum=0xFFFF;
    return USHORT(nNum);
}

SdrPageView* SdrPaintView::HitPage(const Point& rPnt) const
{
    SdrPageView* pHit=NULL;
    for (USHORT i=GetPageViewCount(); i>0 && pHit==NULL;) {
        i--;
        SdrPageView* pPV=GetPageViewPvNum(i);
        if (pPV->GetPageRect().IsInside(rPnt)) pHit=pPV;
    }
    return pHit;
}

USHORT SdrPaintView::GetHiddenPV(const SdrPage* pPage) const
{
    BOOL bWeiter=TRUE;
    USHORT i;
    for (i=0; i<GetPageHideCount() && bWeiter;) {
        SdrPageView* pPV=GetPageHidePvNum(i);
        bWeiter=(pPV->GetPage()!=pPage);
        if (bWeiter) i++;
    }
    return i;
}

USHORT SdrPaintView::GetPageHideNum(const SdrPageView* pPV) const
{
    if (pPV==NULL) return 0xFFFF;
    ULONG nNum=aPagHide.GetPos(pPV);
    if (nNum==CONTAINER_ENTRY_NOTFOUND) nNum=0xFFFF;
    return USHORT(nNum);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintView::AddWin(OutputDevice* pWin1)
{
    SdrViewWinRec* pWinRec=new SdrViewWinRec(pWin1);
    pWinRec->bXorVisible=TRUE; // Normalerweise ist Xor nicht unterdrueckt
    aWinList.Insert(pWinRec);
    for (USHORT i=0; i<GetPageViewCount(); i++) {
        GetPageViewPvNum(i)->AddWin(pWin1);
    }
    if (pItemBrowser!=NULL) pItemBrowser->ForceParent();
}

void SdrPaintView::DelWin(OutputDevice* pWin1)
{
    USHORT nPos=aWinList.Find(pWin1);
    if (nPos!=SDRVIEWWIN_NOTFOUND) {
        for (USHORT i=0; i<GetPageViewCount(); i++) {
            GetPageViewPvNum(i)->DelWin(pWin1);
        }
        aWinList.Delete(nPos);
    }
    if (pItemBrowser!=NULL) pItemBrowser->ForceParent();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TRISTATE SdrPaintView::IsLayerSetVisible(const XubString& rName) const
{
    TRISTATE nRet=FALSE;
    USHORT i=0;
    BOOL b1st=TRUE;
    while (i<GetPageViewCount() && nRet!=FUZZY) {
        SdrPageView* pPV=GetPageViewPvNum(i);
        BOOL bOn=pPV->IsLayerSetVisible(rName);
        if (b1st) {
            nRet=bOn;
            b1st=FALSE;
        } else {
            if (nRet!=bOn) nRet=FUZZY;
        }
        i++;
    }
    return nRet;
}

void SdrPaintView::ShowLayerSet(const XubString& rName, BOOL bShow)
{
    USHORT i;
    for (i=0; i<GetPageViewCount(); i++) {
        SdrPageView* pPV=GetPageViewPvNum(i);
        pPV->ShowLayerSet(rName,bShow);
    }
    InvalidateAllWin();
}

void SdrPaintView::SetLayerVisible(const XubString& rName, BOOL bShow)
{
    USHORT i;
    for (i=0; i<GetPageViewCount(); i++) {
        SdrPageView* pPV=GetPageViewPvNum(i);
        pPV->SetLayerVisible(rName,bShow);
    }
    InvalidateAllWin();
}

TRISTATE SdrPaintView::IsLayerVisible(const XubString& rName) const
{
    TRISTATE nRet=FALSE;
    USHORT i=0;
    BOOL b1st=TRUE;
    while (i<GetPageViewCount() && nRet!=FUZZY) {
        SdrPageView* pPV=GetPageViewPvNum(i);
        BOOL bOn=pPV->IsLayerVisible(rName);
        if (b1st) {
            nRet=bOn;
            b1st=FALSE;
        } else {
            if (nRet!=bOn) nRet=FUZZY;
        }
        i++;
    }
    return nRet;
}

void SdrPaintView::SetAllLayersVisible(BOOL bShow)
{
    USHORT i;
    for (i=0; i<GetPageViewCount(); i++) {
        SdrPageView* pPV=GetPageViewPvNum(i);
        pPV->SetAllLayersVisible(bShow);
    }
    InvalidateAllWin();
}

void SdrPaintView::SetLayerLocked(const XubString& rName, BOOL bLock)
{
    USHORT i;
    for (i=0; i<GetPageViewCount(); i++) {
        SdrPageView* pPV=GetPageViewPvNum(i);
        pPV->SetLayerLocked(rName,bLock);
    }
}

TRISTATE SdrPaintView::IsLayerLocked(const XubString& rName) const
{
    TRISTATE nRet=FALSE;
    USHORT i=0;
    BOOL b1st=TRUE;
    while (i<GetPageViewCount() && nRet!=FUZZY) {
        SdrPageView* pPV=GetPageViewPvNum(i);
        BOOL bLock=pPV->IsLayerLocked(rName);
        if (b1st) {
            nRet=bLock;
            b1st=FALSE;
        } else {
            if (nRet!=bLock) nRet=FUZZY;
        }
        i++;
    }
    return nRet;
}

void SdrPaintView::SetAllLayersLocked(BOOL bLock)
{
    USHORT i;
    for (i=0; i<GetPageViewCount(); i++) {
        SdrPageView* pPV=GetPageViewPvNum(i);
        pPV->SetAllLayersLocked(bLock);
    }
}

void SdrPaintView::SetLayerPrintable(const XubString& rName, BOOL bPrn)
{
    USHORT i;
    for (i=0; i<GetPageViewCount(); i++) {
        SdrPageView* pPV=GetPageViewPvNum(i);
        pPV->SetLayerPrintable(rName,bPrn);
    }
}

TRISTATE SdrPaintView::IsLayerPrintable(const XubString& rName) const
{
    TRISTATE nRet=FALSE;
    USHORT i=0;
    BOOL b1st=TRUE;
    while (i<GetPageViewCount() && nRet!=FUZZY) {
        SdrPageView* pPV=GetPageViewPvNum(i);
        BOOL bPrn=pPV->IsLayerPrintable(rName);
        if (b1st) {
            nRet=bPrn;
            b1st=FALSE;
        } else {
            if (nRet!=bPrn) nRet=FUZZY;
        }
        i++;
    }
    return nRet;
}

void SdrPaintView::SetAllLayersPrintable(BOOL bPrn)
{
    for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
        SdrPageView* pPV=GetPageViewPvNum(nv);
        pPV->SetAllLayersPrintable(bPrn);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintView::InitRedraw(OutputDevice* pOut, const Region& rReg, USHORT nPaintMode)
{
    for (USHORT i=0; i<GetPageViewCount(); i++) {
        SdrPageView* pPV=GetPageViewPvNum(i);
        pPV->InitRedraw(pOut,rReg,nPaintMode,NULL);
    }
    USHORT nWinNum=aWinList.Find(pOut);
    if (nWinNum!=SDRVIEWWIN_NOTFOUND) {
        ((SdrMarkView*)this)->AfterInitRedraw(nWinNum);
        if (IsShownXorVisibleWinNum(nWinNum)) { // Durch Invalidate zerstoerte Handles wiederherstellen
            OutputDevice* pOut=GetWin(nWinNum);
            if (pOut!=NULL && pOut->GetOutDevType()!=OUTDEV_PRINTER) {
                ToggleShownXor(pOut,&rReg);
            }
        }
    }

    RestartAfterPaintTimer();
}

B2dIAOManager* SdrPaintView::GetIAOManager(OutputDevice* pOut)
{
    UINT16 nWinIndex = aWinList.Find(pOut);
    if(nWinIndex != SDRVIEWWIN_NOTFOUND)
    {
        if(aWinList[nWinIndex].pIAOManager)
        {
            return aWinList[nWinIndex].pIAOManager;
        }
    }
    return NULL;
}

B2dIAOManager* SdrPaintView::GetFirstIAOManager()
{
    if(aWinList.GetCount())
    {
        if(aWinList[0].pIAOManager)
        {
            return aWinList[0].pIAOManager;
        }
    }
    return NULL;
}

void SdrPaintView::RefreshAllIAOManagers()
{
    for(UINT16 a=0;a<aWinList.GetCount();a++)
    {
        if(aWinList[a].pIAOManager)
        {
            aWinList[a].pIAOManager->UpdateDisplay();
        }
    }
}

#ifdef TEST_IAO
static UINT16 nStepWidthForMove = 100;
static UINT32 nDirectObjectNum = 0L;
static UINT32 nInsertXPos = 4000;
static UINT32 nInsertYPos = 4000;
static UINT32 nInsertIncrement = 100;
#endif

BOOL SdrPaintView::KeyInput(const KeyEvent& rKEvt, Window* pWin)
{
#ifdef TEST_IAO
    B2dIAOManager* pIAOManager = NULL;
    if(aWinList.GetCount() && aWinList[0].pIAOManager)
        pIAOManager = aWinList[0].pIAOManager;

    if(pIAOManager)
    {
        switch(rKEvt.GetCharCode())
        {
            case '1' :  // On/Off
            {
                pIAOManager->SetVisible(!pIAOManager->IsVisible());
                break;
            }
            case 'a':
            case 's':
            case 'w':
            case 'y':   // move all
            {
                Vector2D aMove;

                if(rKEvt.GetCharCode() == 'a') aMove.X() -= (double)nStepWidthForMove;
                if(rKEvt.GetCharCode() == 's') aMove.X() += (double)nStepWidthForMove;
                if(rKEvt.GetCharCode() == 'w') aMove.Y() -= (double)nStepWidthForMove;
                if(rKEvt.GetCharCode() == 'y') aMove.Y() += (double)nStepWidthForMove;

                Matrix3D aTrans;
                aTrans.Translate(aMove);
                pIAOManager->Transform(aTrans);
                break;
            }
            case '3':
            {
                nDirectObjectNum++;
                if(nDirectObjectNum >= pIAOManager->GetIAOCount())
                    nDirectObjectNum = 0L;
                break;
            }
            case '2':
            {
                if(nDirectObjectNum)
                    nDirectObjectNum--;
                else
                    nDirectObjectNum = pIAOManager->GetIAOCount()-1;
                break;
            }
            case '4':
            {
                B2dIAObject* pAct = pIAOManager->GetIAObject(nDirectObjectNum);
                if(pAct)
                {
                    delete pAct;
                }
                break;
            }
            case '5':
            {
                B2dIAOMarker* pMarker = new B2dIAOMarker(pIAOManager, Point(nInsertXPos, nInsertYPos), B2D_IAO_MARKER_RECT_7X7);
                nInsertXPos += nInsertIncrement;
                nInsertYPos += nInsertIncrement;
                pMarker->SetBaseColor(Color(COL_YELLOW));
                pMarker->Set2ndColor(Color(COL_BLACK));
                break;
            }
            case 'd':
            case 'f':
            case 'r':
            case 'c':   // move
            {
                Vector2D aMove;

                if(rKEvt.GetCharCode() == 'd') aMove.X() -= (double)nStepWidthForMove;
                if(rKEvt.GetCharCode() == 'f') aMove.X() += (double)nStepWidthForMove;
                if(rKEvt.GetCharCode() == 'r') aMove.Y() -= (double)nStepWidthForMove;
                if(rKEvt.GetCharCode() == 'c') aMove.Y() += (double)nStepWidthForMove;

                Matrix3D aTrans;
                aTrans.Translate(aMove);

                B2dIAObject* pAct = pIAOManager->GetIAObject(nDirectObjectNum);
                if(pAct)
                    pAct->Transform(aTrans);
                break;
            }
            case 't':
            case 'z':
            case 'u':
            case 'i':   // rotate all
            {
                B2dIAObject* pCenterObj = pIAOManager->GetIAObject(nDirectObjectNum);
                if(pCenterObj)
                {
                    Vector2D aCenter(pCenterObj->GetBasePosition());
                    Matrix3D aTrans;

                    aTrans.Translate(-aCenter);
                    if(rKEvt.GetCharCode() == 't') aTrans.Rotate(  1.0 * (F_PI / 180.0));
                    if(rKEvt.GetCharCode() == 'z') aTrans.Rotate( 10.0 * (F_PI / 180.0));
                    if(rKEvt.GetCharCode() == 'u') aTrans.Rotate(-10.0 * (F_PI / 180.0));
                    if(rKEvt.GetCharCode() == 'i') aTrans.Rotate( -1.0 * (F_PI / 180.0));
                    aTrans.Translate(aCenter);

                    pIAOManager->Transform(aTrans);
                }
                break;
            }
        }

        // update display if necessary
        pIAOManager->UpdateDisplay();
    }
#endif
    return FALSE;
}

void SdrPaintView::InitRedraw(USHORT nWinNum, const Region& rReg, USHORT nPaintMode)
{
    for (USHORT i=0; i<GetPageViewCount(); i++) {
        SdrPageView* pPV=GetPageViewPvNum(i);
        pPV->InitRedraw(nWinNum,rReg,nPaintMode,NULL);
    }
    ((SdrMarkView*)this)->AfterInitRedraw(nWinNum);
    if (IsShownXorVisibleWinNum(nWinNum)) { // Durch Invalidate zerstoerte Handles wiederherstellen
        OutputDevice* pOut=GetWin(nWinNum);
        if (pOut!=NULL && pOut->GetOutDevType()!=OUTDEV_PRINTER) {
            ToggleShownXor(pOut,&rReg);
        }
    }

    RestartAfterPaintTimer();
}

void SdrPaintView::PostPaint()
{
    // refresh with Paint-functionality
    RefreshAllIAOManagers();
}

void SdrPaintView::RestartAfterPaintTimer()
{
    // start timer for ImpAfterPaint()
    aAfterPaintTimer.Start();
}

BOOL SdrPaintView::IsRedrawReady() const
{
    BOOL bOk=TRUE;
    for (USHORT i=0; i<GetPageViewCount() && bOk; i++) {
        SdrPageView* pPV=GetPageViewPvNum(i);
        bOk=pPV->IsReady();
    }
    return bOk;
}

BOOL SdrPaintView::RedrawOne(USHORT nBrkEvent)
{
    return TRUE;
}

BOOL SdrPaintView::RedrawUntilInput(USHORT nBrkEvent)
{
    return TRUE;
}

void SdrPaintView::GlueInvalidate() const
{
    USHORT nPvAnz=GetPageViewCount();
    USHORT nWinAnz=GetWinCount();
    for (USHORT nWinNum=0; nWinNum<nWinAnz; nWinNum++) {
        OutputDevice* pOut=GetWin(nWinNum);
        if (pOut->GetOutDevType()==OUTDEV_WINDOW) {
            pXOut->SetOutDev(pOut);
            for (USHORT nPvNum=0; nPvNum<nPvAnz; nPvNum++) {
                const SdrPageView* pPV=GetPageViewPvNum(nPvNum);
                const SdrObjList* pOL=pPV->GetObjList();
                pXOut->SetOffset(pPV->GetOffset());
                ULONG nObjAnz=pOL->GetObjCount();
                for (ULONG nObjNum=0; nObjNum<nObjAnz; nObjNum++) {
                    const SdrObject* pObj=pOL->GetObj(nObjNum);
                    const SdrGluePointList* pGPL=pObj->GetGluePointList();
                    if (pGPL!=NULL && pGPL->GetCount()!=0) {
                        pGPL->Invalidate(*(Window*)pOut,pObj);
                    }
                }
            }
        }
        pXOut->SetOffset(Point(0,0));
    }
}

void SdrPaintView::InvalidateAllWin()
{
    for (USHORT i=0; i<GetWinCount(); i++) {
        OutputDevice* pOut=GetWin(i);
        if (pOut->GetOutDevType()==OUTDEV_WINDOW) {
            InvalidateOneWin(*(Window*)pOut);
        }
    }
}

void SdrPaintView::InvalidateAllWin(const Rectangle& rRect, BOOL bPlus1Pix)
{
    USHORT nWinAnz=GetWinCount();
    for (USHORT i=0; i<nWinAnz; i++) {
        OutputDevice* pOut=GetWin(i);
        if (pOut->GetOutDevType()==OUTDEV_WINDOW) {
            Rectangle aRect(rRect);
            if (bPlus1Pix) {
                Size aPixSiz(1,1);
                Size aSiz(pOut->PixelToLogic(aPixSiz));
                aRect.Left  ()-=aSiz.Width();
                aRect.Top   ()-=aSiz.Height();
                aRect.Right ()+=aSiz.Width();
                aRect.Bottom()+=aSiz.Height();
            }
            Point aOrg(pOut->GetMapMode().GetOrigin());
            aOrg.X()=-aOrg.X(); aOrg.Y()=-aOrg.Y();
            Rectangle aOutRect(aOrg,pOut->GetOutputSize());
            if (aRect.IsOver(aOutRect)) {
                InvalidateOneWin(*(Window*)pOut,aRect);
            }
        }
    }
}

void SdrPaintView::InvalidateOneWin(Window& rWin)
{
    rWin.Invalidate();
}

void SdrPaintView::InvalidateOneWin(Window& rWin, const Rectangle& rRect)
{
    rWin.Invalidate(rRect);
}

void SdrPaintView::LeaveOneGroup()
{
    for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
        SdrPageView* pPV=GetPageViewPvNum(nv);
        pPV->LeaveOneGroup();
    }
}

void SdrPaintView::LeaveAllGroup()
{
    for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
        SdrPageView* pPV=GetPageViewPvNum(nv);
        pPV->LeaveAllGroup();
    }
}

BOOL SdrPaintView::IsGroupEntered() const
{
    BOOL bRet=FALSE;
    USHORT nPVAnz=GetPageViewCount();
    for (USHORT nv=0; nv<nPVAnz && !bRet; nv++) {
        SdrPageView* pPV=GetPageViewPvNum(nv);
        if (pPV->GetEnteredLevel()!=0) bRet=TRUE;
    }
    return bRet;
}

void SdrPaintView::SetMasterPagePaintCaching( BOOL bOn, ULONG nCacheMode )
{
    bMasterBmp = bOn;

    if( bOn )
    {
        if( SDR_MASTERPAGECACHE_DEFAULT == nCacheMode || SDR_MASTERPAGECACHE_NONE == nCacheMode )
            nMasterCacheMode = SDR_MASTERPAGECACHE_FULL;
        else
            nMasterCacheMode = nCacheMode;

        ReleaseMasterPagePaintCache();
    }
    else
        nMasterCacheMode = SDR_MASTERPAGECACHE_NONE;
}

// z.B. rufen, wenn Obj der MPg geaendert
void SdrPaintView::ReleaseMasterPagePaintCache()
{
    if (pMasterBmp!=NULL) {
        delete pMasterBmp;
        pMasterBmp=NULL;
    }
}

void SdrPaintView::SetNotPersistDefaultAttr(const SfxItemSet& rAttr, BOOL bReplaceAll)
{
    // bReplaceAll hat hier keinerlei Wirkung
    BOOL bMeasure=ISA(SdrView) && ((SdrView*)this)->IsMeasureTool();
    const SfxPoolItem *pPoolItem=NULL;
    if (rAttr.GetItemState(SDRATTR_LAYERID,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        SdrLayerID nLayerId=((const SdrLayerIdItem*)pPoolItem)->GetValue();
        const SdrLayer* pLayer=pMod->GetLayerAdmin().GetLayerPerID(nLayerId);
        if (pLayer!=NULL) {
            if (bMeasure) aMeasureLayer=pLayer->GetName();
            else aAktLayer=pLayer->GetName();
        }
    }
    if (rAttr.GetItemState(SDRATTR_LAYERNAME,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        if (bMeasure) aMeasureLayer=((const SdrLayerNameItem*)pPoolItem)->GetValue();
        else aAktLayer=((const SdrLayerNameItem*)pPoolItem)->GetValue();
    }
}

void SdrPaintView::MergeNotPersistDefaultAttr(SfxItemSet& rAttr, BOOL bOnlyHardAttr) const
{
    // bOnlyHardAttr hat hier keinerlei Wirkung
    BOOL bMeasure=ISA(SdrView) && ((SdrView*)this)->IsMeasureTool();
    const XubString& aNam=bMeasure?aMeasureLayer:aAktLayer;
    rAttr.Put(SdrLayerNameItem(aNam));
    SdrLayerID nLayer=pMod->GetLayerAdmin().GetLayerID(aNam,TRUE);
    if (nLayer!=SDRLAYER_NOTFOUND) {
        rAttr.Put(SdrLayerIdItem(nLayer));
    }
}

void SdrPaintView::SetDefaultAttr(const SfxItemSet& rAttr, BOOL bReplaceAll)
{
#ifdef DBG_UTIL
    {
        BOOL bHasEEFeatureItems=FALSE;
        SfxItemIter aIter(rAttr);
        const SfxPoolItem* pItem=aIter.FirstItem();
        while (!bHasEEFeatureItems && pItem!=NULL) {
            if (!IsInvalidItem(pItem)) {
                USHORT nW=pItem->Which();
                if (nW>=EE_FEATURE_START && nW<=EE_FEATURE_END) bHasEEFeatureItems=TRUE;
            }
            pItem=aIter.NextItem();
        }

        if(bHasEEFeatureItems)
        {
            String aMessage;
            aMessage.AppendAscii("SdrPaintView::SetDefaultAttr(): Das setzen von EE_FEATURE-Items an der SdrView macht keinen Sinn! Es fuehrt nur zu Overhead und nicht mehr lesbaren Dokumenten.");
            InfoBox(NULL, aMessage).Execute();
        }
    }
#endif
    if (bReplaceAll) aDefaultAttr.Set(rAttr);
    else aDefaultAttr.Put(rAttr,FALSE); // FALSE= InvalidItems nicht als Default, sondern als "Loecher" betrachten
    SetNotPersistDefaultAttr(rAttr,bReplaceAll);
    if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
}

void SdrPaintView::SetDefaultStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr)
{
    pDefaultStyleSheet=pStyleSheet;
    if (pStyleSheet!=NULL && !bDontRemoveHardAttr) {
        SfxWhichIter aIter(pStyleSheet->GetItemSet());
        USHORT nWhich=aIter.FirstWhich();
        while (nWhich!=0) {
            if (pStyleSheet->GetItemSet().GetItemState(nWhich,TRUE)==SFX_ITEM_SET) {
                aDefaultAttr.ClearItem(nWhich);
            }
            nWhich=aIter.NextWhich();
        }
    }
    if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
}

/* new interface src537 */
BOOL SdrPaintView::GetAttributes(SfxItemSet& rTargetSet, BOOL bOnlyHardAttr) const
{
    if(bOnlyHardAttr || !pDefaultStyleSheet)
    {
        rTargetSet.Put(aDefaultAttr, FALSE);
    }
    else
    {
        // sonst DefStyleSheet dazumergen
        rTargetSet.Put(pDefaultStyleSheet->GetItemSet(), FALSE);
        rTargetSet.Put(aDefaultAttr, FALSE);
    }
    MergeNotPersistDefaultAttr(rTargetSet, bOnlyHardAttr);
    return TRUE;
}

BOOL SdrPaintView::SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll)
{
    SetDefaultAttr(rSet,bReplaceAll);
    return TRUE;
}

SfxStyleSheet* SdrPaintView::GetStyleSheet(BOOL& rOk) const
{
    rOk=TRUE;
    return GetDefaultStyleSheet();
}

BOOL SdrPaintView::SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr)
{
    SetDefaultStyleSheet(pStyleSheet,bDontRemoveHardAttr);
    return TRUE;
}

void SdrPaintView::SetDisabledAttr(const SfxItemSet* pNewDisabledAttr)
{
    if (pDisabledAttr!=NULL || pNewDisabledAttr!=NULL) {
        if (pDisabledAttr!=NULL) delete pDisabledAttr;
        pDisabledAttr=NULL;
        if (pNewDisabledAttr!=NULL) {
            pDisabledAttr=new SfxItemSet(*pNewDisabledAttr);
        }
        for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
            SdrPageView* pPV=GetPageViewPvNum(nv);
            if (pPV->GetEnteredLevel()!=0) {
                InvalidateAllWin(pPV->GetPageRect());
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintView::ShowItemBrowser(BOOL bShow)
{
    if (bShow) {
        if (pItemBrowser==NULL) {
            pItemBrowser=new SdrItemBrowser(*(SdrView*)this);
            pItemBrowser->SetFloatingMode(TRUE);
        }
        pItemBrowser->Show();
        pItemBrowser->GrabFocus();
    } else {
        if (pItemBrowser!=NULL) {
            pItemBrowser->Hide();
            delete pItemBrowser;
            pItemBrowser=NULL;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintView::WriteRecords(SvStream& rOut) const
{
    {
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWPAGEVIEWS);
        USHORT nv;
        for (nv=0; nv<GetPageViewCount(); nv++) {
            SdrPageView* pPV=GetPageViewPvNum(nv);
            if (pPV->GetPage()->IsInserted()) {
                rOut<<*pPV;
            }
        }
        for (nv=0; nv<GetPageHideCount(); nv++) {
            SdrPageView* pPV=GetPageHidePvNum(nv);
            if (pPV->GetPage()->IsInserted()) {
                rOut<<*pPV;
            }
        }
    } {
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWVISIELEM);
        rOut<<BOOL(bLayerSortedRedraw);
        rOut<<BOOL(bPageVisible);
        rOut<<BOOL(bBordVisible);
        rOut<<BOOL(bGridVisible);
        rOut<<BOOL(bGridFront);
        rOut<<BOOL(bHlplVisible);
        rOut<<BOOL(bHlplFront);
        rOut<<BOOL(bGlueVisible);
        rOut<<aGridBig;
        rOut<<aGridFin;
        rOut<<aGridWdtX;
        rOut<<aGridWdtY;
        rOut<<aGridSubdiv;
    } {
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWAKTLAYER);

        // UNICODE: rOut << aAktLayer;
        rOut.WriteByteString(aAktLayer);

        // UNICODE: rOut << aMeasureLayer;
        rOut.WriteByteString(aMeasureLayer);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrPaintView::ReadRecord(const SdrIOHeader& rViewHead,
    const SdrNamedSubRecord& rSubHead,
    SvStream& rIn)
{
    BOOL bRet=FALSE;
    if (rSubHead.GetInventor()==SdrInventor) {
        bRet=TRUE;
        switch (rSubHead.GetIdentifier()) {
            case SDRIORECNAME_VIEWPAGEVIEWS: {
                while (rSubHead.GetBytesLeft()>0 && rIn.GetError()==0 && !rIn.IsEof()) {
                    SdrPageView* pPV=new SdrPageView(NULL,Point(),*(SdrView*)this);
                    rIn>>*pPV;
                    if (pPV->GetPage()!=NULL) {
                        if (pPV->IsVisible()) {
                            aPagV.Insert(pPV,CONTAINER_APPEND);
                        } else aPagHide.Insert(pPV,CONTAINER_APPEND);
                    } else {
                        DBG_ERROR("SdrPaintView::ReadRecord(): Seite der PageView nicht gefunden");
                        delete pPV;
                    }
                }
            } break;

            case SDRIORECNAME_VIEWVISIELEM:
            {
                BOOL bTemp;

                rIn>>bTemp; bLayerSortedRedraw=bTemp;
                rIn>>bTemp; bPageVisible      =bTemp;
                rIn>>bTemp; bBordVisible      =bTemp;
                rIn>>bTemp; bGridVisible      =bTemp;
                rIn>>bTemp; bGridFront        =bTemp;
                rIn>>bTemp; bHlplVisible      =bTemp;
                rIn>>bTemp; bHlplFront        =bTemp;
                rIn>>bTemp; bGlueVisible      =bTemp;
                rIn>>aGridBig;
                rIn>>aGridFin;

                if(rSubHead.GetBytesLeft() > 0)
                {
                    rIn >> aGridWdtX;
                    rIn >> aGridWdtY;
                    rIn >> aGridSubdiv;
                }
                break;
            }
            case SDRIORECNAME_VIEWAKTLAYER:
            {
                // UNICODE: rIn >> aAktLayer;
                rIn.ReadByteString(aAktLayer);

                if(rSubHead.GetBytesLeft() > 0)
                {
                    // UNICODE: rIn >> aMeasureLayer;
                    rIn.ReadByteString(aMeasureLayer);
                }
                break;
            }

            default:
                bRet = FALSE;
        }
    }
    return bRet;
}

void SdrPaintView::MakeVisible(const Rectangle& rRect, Window& rWin)
{
    MapMode aMap(rWin.GetMapMode());
    Size aActualSize(rWin.GetOutputSize());

    if( aActualSize.Height() > 0 && aActualSize.Width() > 0 )
    {
        Size aNewSize(rRect.GetSize());
        BOOL bNewScale=FALSE;
        BOOL bNeedMoreX=aNewSize.Width()>aActualSize.Width();
        BOOL bNeedMoreY=aNewSize.Height()>aActualSize.Height();
        if (bNeedMoreX || bNeedMoreY)
        {
            bNewScale=TRUE;
            // Neuen MapMode (Size+Org) setzen und dabei alles invalidieren
            Fraction aXFact(aNewSize.Width(),aActualSize.Width());
            Fraction aYFact(aNewSize.Height(),aActualSize.Height());
            if (aYFact>aXFact) aXFact=aYFact;
            aXFact*=aMap.GetScaleX();
            aXFact.ReduceInaccurate(10); // Um Ueberlaeufe und BigInt-Mapping zu vermeiden
            aMap.SetScaleX(aXFact);
            aMap.SetScaleY(aYFact);
            rWin.SetMapMode(aMap);
            aActualSize=rWin.GetOutputSize();
        }
        Point aOrg(aMap.GetOrigin());
        long dx=0,dy=0;
        long l=-aOrg.X();
        long r=-aOrg.X()+aActualSize.Width()-1;
        long o=-aOrg.Y();
        long u=-aOrg.Y()+aActualSize.Height()-1;
        if (l>rRect.Left()) dx=rRect.Left()-l;
        else if (r<rRect.Right()) dx=rRect.Right()-r;
        if (o>rRect.Top()) dy=rRect.Top()-o;
        else if (u<rRect.Bottom()) dy=rRect.Bottom()-u;
        aMap.SetOrigin(Point(aOrg.X()-dx,aOrg.Y()-dy));
        if (!bNewScale) {
            if (dx!=0 || dy!=0) {
                BOOL bXor=IsShownXorVisible(&rWin);
                if (bXor) HideShownXor(&rWin);
                rWin.Scroll(-dx,-dy);
                rWin.SetMapMode(aMap);
                rWin.Update();
                if (bXor) ShowShownXor(&rWin);
            }
        } else {
            rWin.SetMapMode(aMap);
            InvalidateOneWin(rWin);
        }
    }
}

void SdrPaintView::DoConnect(SdrOle2Obj* pOleObj)
{
}

void SdrPaintView::VisAreaChanged(const OutputDevice* pOut)
{
    USHORT nCount = GetPageViewCount();

    for (USHORT i = 0; i < nCount; i++)
    {
        SdrPageView* pPV = GetPageViewPvNum(i);

        if (pOut)
        {
            // Nur dieses eine OutDev
            USHORT nPos = pPV->GetWinList().Find((OutputDevice*) pOut);

            if (nPos != SDRPAGEVIEWWIN_NOTFOUND)
            {
                VisAreaChanged(pPV->GetWinList()[nPos]);
            }
        }
        else
        {
            // Alle OutDevs
            USHORT nWinAnz = pPV->GetWinList().GetCount();

            for (USHORT nWinNum = 0; nWinNum < nWinAnz; nWinNum++)
            {
                VisAreaChanged(pPV->GetWinList()[nWinNum]);
            }
        }
    }
}

void SdrPaintView::VisAreaChanged(const SdrPageViewWinRec& rPVWR)
{
    if (rPVWR.GetControlContainerRef().is())
    {
        const SdrUnoControlList& rCList = rPVWR.GetControlList();

        for (ULONG i = 0; i < rCList.GetCount(); i++)
        {
            const SdrUnoControlRec& rControlRec = rCList[i];
            uno::Reference< awt::XControl > xUnoControl = rControlRec.GetControl();

            if (xUnoControl.is())
            {
                uno::Reference< awt::XWindow > xVclComponent( xUnoControl, uno::UNO_QUERY );

                if (xVclComponent.is() && rControlRec.GetUnoObj())
                {
                    Rectangle aRect = rControlRec.GetUnoObj()->GetLogicRect();
                    OutputDevice* pOut = rPVWR.GetOutputDevice();
                    Point aPixPos(pOut->LogicToPixel(aRect.TopLeft()));
                    Size aPixSize(pOut->LogicToPixel(aRect.GetSize()));

                    xVclComponent->setPosSize(aPixPos.X(), aPixPos.Y(),
                        aPixSize.Width(), aPixSize.Height(), awt::PosSize::POSSIZE);
                }
            }
        }
    }
}

