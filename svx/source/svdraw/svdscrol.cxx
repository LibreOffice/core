/*************************************************************************
 *
 *  $RCSfile: svdscrol.cxx,v $
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

#include "svdscrol.hxx"
#include <math.h>
#include "svdtrans.hxx"
#include "svdpagv.hxx"
#include "svdview.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpMtfAnimationInfo::SaveBackground(const ImpSdrMtfAnimator& rAnimator, const Region* pClip)
{
    Point aZero;
    MapMode aMapMode(pOutDev->GetMapMode());
    aMapMode.SetOrigin(aZero);
    aBackground.SetMapMode(aMapMode);
    Size aOutputSize(rAnimator.aOutputRect.GetSize());
    Point aOutputTopLeft(rAnimator.aOutputRect.TopLeft());
    aOutputTopLeft+=aOffset;

//  if (pClip!=NULL) {
//      Region aRegion(*pClip);
//      aRegion.Move(-aOutputTopLeft.X(),-aOutputTopLeft.Y());
//      aBackground.SetClipRegion(aRegion);
//  }

    // #73647#
    Region aBackClipRegion = pOutDev->GetActiveClipRegion();
    aBackClipRegion.Move(-aOutputTopLeft.X(), -aOutputTopLeft.Y());
    aBackground.SetClipRegion(aBackClipRegion);

    aBackground.DrawOutDev(aZero,aOutputSize,aOutputTopLeft,aOutputSize,*pOutDev);

// see #68462#
//  if (pPageView!=NULL) { // Xor ggf. entfernen vom gesicherten Hintergrund
//      const SdrView& rView=pPageView->GetView();
//      if (rView.IsShownXorVisible(pOutDev)) {
//          Point aPaintPos;
//          aPaintPos-=aOutputTopLeft;
//          aPaintPos+=aOffset;
//          MapMode aMap(aBackground.GetMapMode());
//          aMap.SetOrigin(aPaintPos);
//          aBackground.SetMapMode(aMap);
//          rView.ToggleShownXor(&aBackground,NULL);
//          aMap.SetOrigin(aZero);
//          aBackground.SetMapMode(aMap); // MapMode wieder zurueksetzen, sonst geht das DrawOutDev spaeter schief
//      }
//  }
    aBackground.SetClipRegion();
}

void ImpMtfAnimationInfo::Paint(const ImpSdrMtfAnimator& rAnimator, OutputDevice& rOut) const
{
    Point aZero;
    Size aOutputSize(rAnimator.aOutputRect.GetSize());
    Point aOutputTopLeft(rAnimator.aOutputRect.TopLeft());
    aOutputTopLeft+=aOffset;
    Region aClipMerk;
    FASTBOOL bClipMerk;
    if (rAnimator.bClipRegion) {
        bClipMerk=rOut.IsClipRegion();
        aClipMerk=rOut.GetClipRegion();
        if (aOffset==aZero) {
            rOut.SetClipRegion(rAnimator.aClipRegion);
        } else {
            Region aTempRegion(rAnimator.aClipRegion);
            aTempRegion.Move(aOffset.X(),aOffset.Y());
            rOut.SetClipRegion(aTempRegion);
        }
    }
    rOut.DrawOutDev(aOutputTopLeft,aOutputSize,aZero,aOutputSize,aVirDev);
    if (rAnimator.bClipRegion) {
        if (bClipMerk) rOut.SetClipRegion(aClipMerk);
        else rOut.SetClipRegion();
    }
}

void ImpMtfAnimationInfo::AnimateOneStep(ImpSdrMtfAnimator& rAnimator)
{
    if (bReady) return;
    Point aZero;
    Size aOutputSize(rAnimator.aOutputRect.GetSize());
    Point aOutputTopLeft(rAnimator.aOutputRect.TopLeft());
    aOutputTopLeft+=aOffset;
    if (!bBackSaved) {
        MapMode aMapMode(pOutDev->GetMapMode());
        aMapMode.SetOrigin(aZero);
        aBackground.SetMapMode(aMapMode);
        if (!aBackground.SetOutputSize(aOutputSize)) {
            DBG_ERROR("AnimateOneStep(): Zuwenig Speicherplatz zum sichern des Hintergrunds!");
        }
        SaveBackground(rAnimator);
        bBackSaved=TRUE;
        aVirDev.SetMapMode(aMapMode);
        if (!aVirDev.SetOutputSize(aOutputSize)) {
            DBG_ERROR("AnimateOneStep(): Zuwenig Speicherplatz fuer das Paint-VDev!");
        }
    }
    // erstmal den Hintergrund ins VDev kopieren
    aVirDev.DrawOutDev(aZero,aOutputSize,aZero,aOutputSize,aBackground);

    SdrTextAniKind eAniKind=rAnimator.eAniKind;
    SdrTextAniDirection eDirection=rAnimator.eDirection;
    FASTBOOL bBlink=eAniKind==SDRTEXTANI_BLINK;
    FASTBOOL bSlide=eAniKind==SDRTEXTANI_SLIDE;
    FASTBOOL bHorz=eDirection==SDRTEXTANI_LEFT || eDirection==SDRTEXTANI_RIGHT;
    FASTBOOL bVert=eDirection==SDRTEXTANI_UP || eDirection==SDRTEXTANI_DOWN;
    FASTBOOL bRev=eDirection==SDRTEXTANI_RIGHT || eDirection==SDRTEXTANI_DOWN;
    if (bBackTrack) bRev=!bRev; // Rueckweg bei Alternate (oder Slide)
    FASTBOOL bStartInside=rAnimator.bStartInside && !bSlide;
    FASTBOOL bStopInside=rAnimator.bStopInside || bSlide;
    Rectangle aScrollFrameRect(rAnimator.aScrollFrameRect);
    Rectangle aMtfBoundRect(rAnimator.aMtfBoundRect);
    long nFL=bHorz?aScrollFrameRect.Left():aScrollFrameRect.Top();     // Frame Links bzw. Oben
    long nFR=bHorz?aScrollFrameRect.Right():aScrollFrameRect.Bottom(); // Frame Rechts bzw. Unten
    long nFW=nFR-nFL;                                                  // Frame Breite bzw. Hoehe
    long nTL=bHorz?aMtfBoundRect.Left():aMtfBoundRect.Top();           // Text Links bzw. Oben
    long nTR=bHorz?aMtfBoundRect.Right():aMtfBoundRect.Bottom();       // Text Rechts bzw. Unten
    long nTW=nTR-nTL;                                                  // Text Breite bzw. Hoehe
    if (bNeu) {
        bNeu=FALSE;
        bShown=TRUE;
        if (bBlink) {
            bShown=bStartInside;
        } else {
            if (bStartInside && nLoopNum==0) {
                // Wenn der Text kleiner als der Scrollbereich, dann andere StartPos (wie bRev)
                nLoopStep=0; // Die durch die Verankerung vorgegebene Position verwenden!
            } else {
                if (!bRev) nLoopStep=nFR-nTL;
                else nLoopStep=nFL-nTR;
            }
        }
    } else {
        USHORT nMaxLoop=rAnimator.nMaxCount;
        if (bSlide && nMaxLoop==0) nMaxLoop=1;
        if (bBlink && nMaxLoop!=0 && !bStartInside && !bStopInside) nMaxLoop++; // etwas andere Zaehlung beim Blinken
        FASTBOOL bLastLoop=nMaxLoop>0 && nLoopNum>=nMaxLoop-1 && (!bSlide || !bBackTrack);
        if (bBlink) {
            bShown=!bShown;
            if (bShown) nLoopNum++;
            if (bLastLoop && bShown==bStopInside) bReady=TRUE;
        } else {
            bShown=TRUE;
            long nStep=rAnimator.nAmount;
            if (nStep==0) nStep=-1;
            if (nStep<0) {
                nStep=pOutDev->PixelToLogic(Size(-nStep,0)).Width();
            }
            if (!bRev) nStep=-nStep;
            nLoopStep+=nStep;
            // Nun den Maximalwert fuer nLoopStep berechnen
            long nLoopStepMax=0;
            FASTBOOL bScrollOut=FALSE;
            if (eAniKind==SDRTEXTANI_SCROLL) {
                if (bLastLoop && bStopInside) nLoopStepMax=0; // Die durch die Verankerung vorgegebene Position verwenden!
                else bScrollOut=TRUE;
            } else if (eAniKind==SDRTEXTANI_ALTERNATE) {
                if (bLastLoop) {
                    if (!bStopInside) bScrollOut=TRUE;
                    else nLoopStepMax=0; // Die durch die Verankerung vorgegebene Position verwenden!
                } else { // Am Rand wenden
                    // Je nachdem, ob die Textbreite oder der sichtbare Bereich
                    // groesser ist, ist der eine oder der andere Rand relevant.
                    if ((!bRev) != (nTW>=nFW)) nLoopStepMax=nFL-nTL;
                    else nLoopStepMax=nFR-nTR;
                }
            } else if (bSlide) {
                if (bBackTrack) bScrollOut=TRUE;
                else nLoopStepMax=0; // Die durch die Verankerung vorgegebene Position verwenden!
            }
            if (bScrollOut) { // Rausschieben
                if (!bRev) nLoopStepMax=nFL-nTR;
                else nLoopStepMax=nFR-nTL;
            }
            if ((!bRev && nLoopStep<=nLoopStepMax) || (bRev && nLoopStep>=nLoopStepMax)) {
                // Anschlag erreicht
                nLoopStep=nLoopStepMax;
                if (bSlide) {
                    if (!bBackTrack) nLoopNum++;
                } else {
                    nLoopNum++;
                }
                if (!bLastLoop) {
                    if (eAniKind==SDRTEXTANI_SCROLL) bNeu=TRUE; // Naechste Runde wieder von vorn
                    else bBackTrack=!bBackTrack; // Sonst Alternate oder Slide
                } else {
                    bReady=TRUE;
                }
            }
        }
    }

    if (rAnimator.pMtf!=NULL && bShown) {
        rAnimator.pMtf->WindStart();
        Point aPaintPos;
        if (!bBlink) { // hier wird nun die Position bestimmt
            long nRoundedDistance=nLoopStep;
            if (rAnimator.nRotateAngle!=0) {
                // ggf. auf Pixel alignen um tanzende Buchstaben
                // bei gedrehtem Text einzuschraenken #33360#
                long nWink=rAnimator.nRotateAngle;
                if (bVert) nWink+=9000;
                nWink=NormAngle360(nWink);
                if (nWink>18000) nWink-=18000;
                if ((nWink>=3000 && nWink<=6000) || (nWink>=3000+9000 && nWink<=6000+9000)) {
                    // Andere Winkel sind zu Flach
                    double nSin=sin(nWink*nPi180);
                    long nUnrotDist=Round(nRoundedDistance*nSin);
                    nUnrotDist=pOutDev->LogicToPixel(Size(nUnrotDist,0)).Width();
                    nUnrotDist=pOutDev->PixelToLogic(Size(nUnrotDist,0)).Width();
                    nRoundedDistance=Round(nUnrotDist/nSin);
                }
            }
            if (bHorz) aPaintPos.X()+=nRoundedDistance;
            if (bVert) aPaintPos.Y()+=nRoundedDistance;
            if (rAnimator.nRotateAngle!=0) {
                RotatePoint(aPaintPos,aZero,rAnimator.nSin,rAnimator.nCos);
            }
        }
        aPaintPos-=aOutputTopLeft;
        aPaintPos+=aOffset;
        MapMode aMap(aVirDev.GetMapMode());
        aMap.SetOrigin(aPaintPos);
        aVirDev.SetMapMode(aMap);
        rAnimator.pMtf->Play(&aVirDev);
        aMap.SetOrigin(aZero);
        aVirDev.SetMapMode(aMap); // MapMode wieder zurueksetzen, sonst geht das DrawOutDev spaeter schief
    }

// see #68462#
//  if (pPageView!=NULL) {
//      const SdrView& rView=pPageView->GetView();
//      if (rView.IsShownXorVisible(pOutDev)) {
//          Point aPaintPos;
//          aPaintPos-=aOutputTopLeft;
//          aPaintPos+=aOffset;
//          MapMode aMap(aVirDev.GetMapMode());
//          aMap.SetOrigin(aPaintPos);
//          aVirDev.SetMapMode(aMap);
//          rView.ToggleShownXor(&aVirDev,NULL);
//          aMap.SetOrigin(aZero);
//          aVirDev.SetMapMode(aMap); // MapMode wieder zurueksetzen, sonst geht das DrawOutDev spaeter schief
//      }
//  }
    Paint(rAnimator,*pOutDev);
}

void ImpMtfAnimationInfo::Restart()
{
    bPause=FALSE;
    bReady=FALSE;
    bBackSaved=FALSE;
    bBackTrack=FALSE;
    bNeu=TRUE;
    nLoopNum=0;
    nLoopStep=0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ImpSdrMtfAnimator::ImpSdrMtfAnimator():
    aInfoList(1024,16,16),
    pMtf(NULL),
    nRotateAngle(0),
    nSin(0.0),
    nCos(0.0),
    bClipRegion(FALSE),
    eAniKind(SDRTEXTANI_NONE),
    eDirection(SDRTEXTANI_LEFT),
    bStartInside(FALSE),
    bStopInside(FALSE),
    nMaxCount(0),
    nDelay(0),
    nAmount(0)
{
    aTimer.SetTimeoutHdl(LINK(this,ImpSdrMtfAnimator,ImpTimerHdl));
}

ImpSdrMtfAnimator::~ImpSdrMtfAnimator()
{
    ImpClearInfoList();
    if (pMtf!=NULL) delete pMtf;
}

ULONG ImpSdrMtfAnimator::FindInfo(const OutputDevice& rOut, const Point& rOffset, long nExtraData) const
{
    ULONG nAnz=GetInfoCount();
    for (ULONG nNum=0; nNum<nAnz; nNum++) {
        const ImpMtfAnimationInfo* pInfo=GetInfo(nNum);
        if (pInfo->pOutDev==&rOut && pInfo->aOffset==rOffset && pInfo->nExtraData==nExtraData) {
            return nNum;
        }

    }
    return CONTAINER_ENTRY_NOTFOUND;
}

void ImpSdrMtfAnimator::ImpClearInfoList()
{
    ULONG nAnz=GetInfoCount();
    for (ULONG nNum=nAnz; nNum>0;) {
        nNum--;
        RemoveInfo(nNum);
    }
}

IMPL_LINK(ImpSdrMtfAnimator,ImpTimerHdl,AutoTimer*,pTimer)
{
    aNotifyLink.Call(this);
    ULONG nAnz=GetInfoCount();
    for (ULONG nNum=0; nNum<nAnz; nNum++) {
        ImpMtfAnimationInfo* pInfo=GetInfo(nNum);
        if (!pInfo->bPause) pInfo->AnimateOneStep(*this);
    }
    if (nAnz==0) aTimer.Stop();
    return 0;
}

void ImpSdrMtfAnimator::SetGDIMetaFile(GDIMetaFile* pMetaFile)
{
    if (pMtf!=NULL) delete pMtf;
    pMtf=pMetaFile;
}

void ImpSdrMtfAnimator::SetRotateAngle(long nWink)
{
    if (nRotateAngle!=nWink) {
        nRotateAngle=nWink;
        double a=nWink*nPi180;
        nSin=sin(a);
        nCos=cos(a);
    }
}

void ImpSdrMtfAnimator::SetAttributes(const SfxItemSet& rSet)
{
    eAniKind    =((SdrTextAniKindItem       &)rSet.Get(SDRATTR_TEXT_ANIKIND       )).GetValue();
    eDirection  =((SdrTextAniDirectionItem  &)rSet.Get(SDRATTR_TEXT_ANIDIRECTION  )).GetValue();
    bStartInside=((SdrTextAniStartInsideItem&)rSet.Get(SDRATTR_TEXT_ANISTARTINSIDE)).GetValue();
    bStopInside =((SdrTextAniStopInsideItem &)rSet.Get(SDRATTR_TEXT_ANISTOPINSIDE )).GetValue();
    nMaxCount   =((SdrTextAniCountItem      &)rSet.Get(SDRATTR_TEXT_ANICOUNT      )).GetValue();
    nDelay      =((SdrTextAniDelayItem      &)rSet.Get(SDRATTR_TEXT_ANIDELAY      )).GetValue();
    nAmount     =((SdrTextAniAmountItem     &)rSet.Get(SDRATTR_TEXT_ANIAMOUNT     )).GetValue();
}

ImpMtfAnimationInfo* ImpSdrMtfAnimator::Start(OutputDevice& rOutDev, const Point& rOffset, long nExtraData)
{
    ImpMtfAnimationInfo* pInfo=NULL;
    ULONG nPos=FindInfo(rOutDev,rOffset,nExtraData);
    if (nPos==CONTAINER_ENTRY_NOTFOUND) {
        pInfo=new ImpMtfAnimationInfo;
        pInfo->pOutDev=&rOutDev;
        pInfo->aOffset=rOffset;
        pInfo->nExtraData=nExtraData;
        ImpInsertInfo(pInfo);
    } else {
        pInfo=GetInfo(nPos);
    }
    pInfo->Restart();
    long nTimeout=nDelay;
    if (nTimeout==0) {
        if (eAniKind==SDRTEXTANI_BLINK) nTimeout=250;
        else nTimeout=50;
    }
    aTimer.SetTimeout(nTimeout);
    aTimer.Start();
    return pInfo;
}

void ImpSdrMtfAnimator::Stop()
{
    ImpClearInfoList();
    aTimer.Stop();
}

void ImpSdrMtfAnimator::Stop(OutputDevice& rOutDev)
{
    ULONG nAnz=GetInfoCount();
    for (ULONG nNum=nAnz; nNum>0;) {
        nNum--;
        ImpMtfAnimationInfo* pInfo=GetInfo(nNum);
        if (pInfo->pOutDev==&rOutDev) RemoveInfo(nNum);
    }
    if (GetInfoCount()==0) aTimer.Stop();
}

void ImpSdrMtfAnimator::Stop(OutputDevice& rOutDev, const Point& rOffset)
{
    ULONG nAnz=GetInfoCount();
    for (ULONG nNum=nAnz; nNum>0;) {
        nNum--;
        ImpMtfAnimationInfo* pInfo=GetInfo(nNum);
        if (pInfo->pOutDev==&rOutDev && pInfo->aOffset==rOffset) RemoveInfo(nNum);
    }
    if (GetInfoCount()==0) aTimer.Stop();
}

