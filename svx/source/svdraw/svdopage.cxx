/*************************************************************************
 *
 *  $RCSfile: svdopage.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-11-01 13:27:31 $
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

#include "svdopage.hxx"
#include "svdglob.hxx"  // Stringcache
#include "svdstr.hrc"   // Objektname
#include "svdtrans.hxx"
#include "svdetc.hxx"
#include "svdio.hxx"
#include "svdxout.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svdpagv.hxx"

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrPageObj,SdrObject);


SdrPageObj::SdrPageObj(USHORT nNewPageNum):
    nPageNum(nNewPageNum),
    bPainting(FALSE),
    bNotifying(FALSE)
{
}


SdrPageObj::SdrPageObj(const Rectangle& rRect, USHORT nNewPageNum):
    nPageNum(nNewPageNum),
    bPainting(FALSE),
    bNotifying(FALSE)
{
    aOutRect=rRect;
}


void __EXPORT SdrPageObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId&, const SfxHint& rHint, const TypeId&)
{
    SdrHint* pSdrHint = PTR_CAST(SdrHint, &rHint);

    if(pSdrHint)
    {
        SdrHintKind eHint = pSdrHint->GetKind();

        if(eHint == HINT_PAGEORDERCHG)
        {
            SendRepaintBroadcast();
        }
        else
        {
            const SdrPage* pChangedPage = pSdrHint->GetPage();

            if(pSdrHint->GetObject() != this
                && pModel
                && bInserted
                && pChangedPage
                && pChangedPage != pPage)
            {
                const SdrPage* pShownPage = pModel->GetPage(nPageNum);

                if(pShownPage)
                {
                    if(pShownPage == pChangedPage)
                    {
                        if(eHint == HINT_OBJCHG || eHint == HINT_OBJLISTCLEARED)
                        {
                            // do nothing, else loop with HINT_OBJCHG cause
                            // it can not be decided if hint comes from 21 lines
                            // above (SendRepaintBroadcast())or normally from
                            // object. Doing nothing leads to not updating
                            // object moves on a 2nd opened view showing the page
                            // as handout or notice page. (AW 06052000)
                            //
                            // one solution would be to invalidate without using
                            // SendRepaintBroadcast(), so I made some tries, but it
                            // did'nt work:
                            //
                            //SdrHint aHint(*this);
                            //aHint.SetNeedRepaint(TRUE);
                            //aHint.SetKind(HINT_PAGECHG);
                            //pModel->Broadcast(aHint);
                            //SdrPageView* pPV;
                            //if(pModel && (pPV = pModel->GetPaintingPageView()))
                            //{
                            //  pPV->InvalidateAllWin(pSdrHint->GetRect(), TRUE);
                            //}
                            // GetPageView(pChangedPage);
                            // SdrPageView::InvalidateAllWin(pSdrHint->GetRect(), TRUE);
                        }
                        else
                        {
                            // send normal
                            SendRepaintBroadcast();
                        }
                    }
                    else if(pChangedPage->IsMasterPage())
                    {
                        UINT16 nMaPgAnz = pShownPage->GetMasterPageCount();
                        BOOL bDone(FALSE);

                        for(UINT16 i=0; i<nMaPgAnz && !bDone; i++)
                        {
                            const SdrPage* pMaster = pShownPage->GetMasterPage(i);

                            if(pMaster == pChangedPage)
                            {
                                SendRepaintBroadcast();
                                bDone = TRUE;
                            }
                        }
                    }
                }
            }
        }
    }
}


void SdrPageObj::SetModel(SdrModel* pNewModel)
{
    SdrModel* pOldMod=pModel;
    SdrObject::SetModel(pNewModel);
    if (pModel!=pOldMod) {
        if (pOldMod!=NULL) EndListening(*pOldMod);
        if (pModel!=NULL) StartListening(*pModel);
    }
}


UINT16 SdrPageObj::GetObjIdentifier() const
{
    return UINT16(OBJ_PAGE);
}


void SdrPageObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=FALSE;
    rInfo.bRotate90Allowed  =FALSE;
    rInfo.bMirrorFreeAllowed=FALSE;
    rInfo.bMirror45Allowed  =FALSE;
    rInfo.bMirror90Allowed  =FALSE;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed     =FALSE;
    rInfo.bNoOrthoDesired   =FALSE;
    rInfo.bCanConvToPath    =FALSE;
    rInfo.bCanConvToPoly    =FALSE;
    rInfo.bCanConvToPathLineToArea=FALSE;
    rInfo.bCanConvToPolyLineToArea=FALSE;
}

#define NORELMAPMODE


FASTBOOL SdrPageObj::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
{
    // Hidden objects on masterpages, draw nothing
    if((rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE) && bNotVisibleAsMaster)
        return TRUE;

    SdrPageView* pPV = NULL;
    if( pModel && ( pPV = pModel->GetPaintingPageView() ) )
    {
        pPV->SetPaintingPageObj( (SdrPageObj*) this );
    }

    FASTBOOL bOk=TRUE;
    SdrPage* pMainPage= pModel==NULL ? NULL : pModel->GetPage(nPageNum);
    FASTBOOL bWindow    =rXOut.GetOutDev()->GetOutDevType()==OUTDEV_WINDOW;
    FASTBOOL bPrinter   =rXOut.GetOutDev()->GetOutDevType()==OUTDEV_PRINTER;
    FASTBOOL bPageValid =pMainPage!=NULL;
    FASTBOOL bPaintArea =bPainting && bPageValid; // Grau fuellen, wenn Rekursion
    FASTBOOL bPaintFrame=bPaintArea || bWindow || bPageValid; // Nur auf dem Drucker bei ungueltiter PageNum kein Rahmen Zeichnen
    FASTBOOL bPaintObjs =!bPainting && bPageValid;

    if(bPaintArea)
    {
        OutputDevice& rOut=*rXOut.GetOutDev();
        rOut.SetFillColor( Color( COL_LIGHTGRAY ) );
        rOut.SetLineColor( Color(COL_GRAY ) );
        rOut.DrawRect(aOutRect);
    }

    if(bPaintObjs)
    {
        // Casten auf nonconst. Flag setzen um Rekursion zu erkennen, wenn
        // naemlich das PageObj auf der Seite sitzt, die es anzeigen soll
        // oder auf einer MasterPage dieser Seite, ...
        ((SdrPageObj*)this)->bPainting=TRUE;
        if (pModel!=NULL) {
            SdrPage* pMainPage=pModel->GetPage(nPageNum);
            if (pMainPage!=NULL) {
                // Checken, ob das setzen eines Clippings erforderlich ist
                Rectangle aPageRect(0,0,pMainPage->GetWdt(),pMainPage->GetHgt());
                FASTBOOL bClipIt=!aPageRect.IsInside(pMainPage->GetAllObjBoundRect());
                if (!bClipIt) { // MasterPages auch checken
                    USHORT nMaPgAnz=pMainPage->GetMasterPageCount();
                    for (USHORT i=0; i<nMaPgAnz && !bClipIt; i++) {
                        SdrPage* pPg=pMainPage->GetMasterPage(i);
                        bClipIt=!aPageRect.IsInside(pPg->GetAllObjBoundRect());
                    }
                }
                FASTBOOL bClip0=FALSE;
                Region aClip0;
                if (bClipIt) {
                    // Hier koennte ich mal noch einbauen, dass eine eventuelle
                    // Metafileaufzeichnung pausiert wird, damit keine
                    // SetClipRegion-Actions erzeugt werden.
                    bClip0=rXOut.GetOutDev()->IsClipRegion();
                    aClip0=rXOut.GetOutDev()->GetClipRegion();
                }
#ifndef NORELMAPMODE
                Point aZero;
                Fraction aFact1(1,1);
                Point aOfs(aOutRect.TopLeft());
                rXOut.GetOutDev()->SetMapMode(MapMode(MAP_RELATIVE,aOfs,aFact1,aFact1));
                long nXMul=aOutRect.GetWidth()-1;
                long nYMul=aOutRect.GetHeight()-1;
                long nXDiv=pMainPage->GetWdt();
                long nYDiv=pMainPage->GetHgt();
                Fraction aXScl(nXMul,nXDiv);
                Fraction aYScl(nYMul,nYDiv);
                // nun auf 10 Binaerstellen kuerzen (ca. 3 Dezimalstellen). Joe, 01-12-1995, BugId 21483
                Kuerzen(aXScl,10); // auf 7 Binaerstellen Kuerzen = ca. 2 Dezimalstellen
                Kuerzen(aYScl,10); // auf 7 Binaerstellen Kuerzen = ca. 2 Dezimalstellen
                nXMul=aXScl.GetNumerator();
                nXDiv=aXScl.GetDenominator();
                nYMul=aYScl.GetNumerator();
                nYDiv=aYScl.GetDenominator();
                rXOut.GetOutDev()->SetMapMode(MapMode(MAP_RELATIVE,aZero,aXScl,aYScl));
#else
                MapMode aMapMerk(rXOut.GetOutDev()->GetMapMode());
                MapMode aMapNeu(aMapMerk);
                Point aOrg(aMapNeu.GetOrigin());
                Fraction aSclx(aMapNeu.GetScaleX());
                Fraction aScly(aMapNeu.GetScaleY());
                Point aOfs(aOutRect.TopLeft());
                aOrg+=aOfs;
                ResizePoint(aOrg,Point(),Fraction(pMainPage->GetWdt(),aOutRect.GetWidth()),
                                         Fraction(pMainPage->GetHgt(),aOutRect.GetHeight()));
                aSclx*=Fraction(aOutRect.GetWidth(),pMainPage->GetWdt());
                aScly*=Fraction(aOutRect.GetHeight(),pMainPage->GetHgt());
                // nun auf 10 Binaerstellen kuerzen (ca. 3 Dezimalstellen). Joe, 01-12-1995, BugId 21483
                Kuerzen(aSclx,10); // auf 7 Binaerstellen Kuerzen = ca. 2 Dezimalstellen
                Kuerzen(aScly,10); // auf 7 Binaerstellen Kuerzen = ca. 2 Dezimalstellen
                aMapNeu.SetOrigin(aOrg);
                aMapNeu.SetScaleX(aSclx);
                aMapNeu.SetScaleY(aScly);
                rXOut.GetOutDev()->SetMapMode(aMapNeu);
#endif
                if (bClipIt) {
                    // Hier koennte ich mal noch einbauen, dass eine eventuelle
                    // Metafileaufzeichnung pausiert wird, damit keine
                    // SetClipRegion-Actions erzeugt werden.
                    rXOut.GetOutDev()->IntersectClipRegion(aPageRect);
                }
                SdrPaintInfoRec aInfoRec(rInfoRec);
                aInfoRec.aCheckRect=Rectangle(); // alles Malen!
                USHORT nMaPgAnz=pMainPage->GetMasterPageCount();
                USHORT i=0;
                do {
                    aInfoRec.nPaintMode=rInfoRec.nPaintMode & ~SDRPAINTMODE_MASTERPAGE;
                    aInfoRec.nPaintMode&=~SDRPAINTMODE_GLUEPOINTS;
                    aInfoRec.nPaintMode|=SDRPAINTMODE_ANILIKEPRN;
                    SdrPage* pPg;
                    const SetOfByte* pMLayers=NULL;
                    if (i<nMaPgAnz) {
                        pPg=pMainPage->GetMasterPage(i);
                        pMLayers=&pMainPage->GetMasterPageVisibleLayers(i);
                        aInfoRec.nPaintMode|=SDRPAINTMODE_MASTERPAGE;
                    }
                    else
                        pPg=pMainPage;

                    i++;
                    if (pPg!=NULL)
                    {   // ansonsten evtl. ungueltige Masterpage
                        ULONG nObjAnz=pPg->GetObjCount();
                        for ( ULONG i=0; i<nObjAnz; i++ )
                        {
                            if( i == 0 && pPg->IsMasterPage() && pMainPage->GetBackgroundObj() )
                            {
                                SdrObject* pBackgroundObj = pMainPage->GetBackgroundObj();
                                if( pBackgroundObj->GetLogicRect() != aPageRect )
                                {
                                    pBackgroundObj->SetLogicRect( aPageRect );
                                    pBackgroundObj->RecalcBoundRect();
                                }
                                pBackgroundObj->Paint( rXOut, aInfoRec );
                            }
                            else
                            {
                                SdrObject* pObj=pPg->GetObj(i);
                                const Rectangle& rBoundRect=pObj->GetBoundRect();
                                if (rInfoRec.aPaintLayer.IsSet(pObj->GetLayer()) &&        // Layer des Obj nicht sichtbar
                                    (pMLayers==NULL || pMLayers->IsSet(pObj->GetLayer()))) // MasterPageLayer visible
                                {
                                     pObj->Paint(rXOut,aInfoRec);
                                }
                            }
                        }
                    }
                } while (i<=nMaPgAnz);
#ifndef NORELMAPMODE
                rXOut.GetOutDev()->SetMapMode(MapMode(MAP_RELATIVE,aZero,Fraction(nXDiv,nXMul),Fraction(nYDiv,nYMul)));
                aOfs.X()=-aOfs.X();
                aOfs.Y()=-aOfs.Y();
                rXOut.GetOutDev()->SetMapMode(MapMode(MAP_RELATIVE,aOfs,aFact1,aFact1));
#else
                rXOut.GetOutDev()->SetMapMode(aMapMerk);
#endif
                if (bClipIt) {
                    if (bClip0) rXOut.GetOutDev()->SetClipRegion(aClip0);
                    else rXOut.GetOutDev()->SetClipRegion();
                }
            }
        }
        // IsInPainting-Flag zuruecksetzen
        ((SdrPageObj*)this)->bPainting=FALSE;
    }

    if(bPaintFrame)
    {
        OutputDevice& rOut=*rXOut.GetOutDev();
        rOut.SetFillColor();
        rOut.SetLineColor( Color( COL_GRAY ) );
        rOut.DrawRect(aOutRect);
    }

    if (bOk && (rInfoRec.nPaintMode & SDRPAINTMODE_GLUEPOINTS) !=0) {
        bOk=PaintGluePoints(rXOut,rInfoRec);
    }

    if( pPV )
        pPV->SetPaintingPageObj( NULL );

    return bOk;
}


void SdrPageObj::operator=(const SdrObject& rObj)
{
    SdrObject::operator=(rObj);
    nPageNum=((const SdrPageObj&)rObj).nPageNum;
}


void SdrPageObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulPAGE);
}


void SdrPageObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralPAGE);
}


const Rectangle& SdrPageObj::GetBoundRect() const
{
    return SdrObject::GetBoundRect();
}


const Rectangle& SdrPageObj::GetSnapRect() const
{
    return SdrObject::GetSnapRect();
}


const Rectangle& SdrPageObj::GetLogicRect() const
{
    return SdrObject::GetLogicRect();
}


void SdrPageObj::NbcSetSnapRect(const Rectangle& rRect)
{
    SdrObject::NbcSetSnapRect(rRect);
}


void SdrPageObj::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrObject::NbcSetLogicRect(rRect);
}


void SdrPageObj::TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL bDetail) const
{
    SdrObject::TakeXorPoly(rPoly,bDetail);
}

void SdrPageObj::TakeContour(XPolyPolygon& rXPoly, SdrContourType eType) const
{
}


void SdrPageObj::WriteData(SvStream& rOut) const
{
    SdrObject::WriteData(rOut);
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrPageObj");
#endif
    rOut<<nPageNum;
}

void SdrPageObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if (rIn.GetError()!=0) return;
    SdrObject::ReadData(rHead,rIn);
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrPageObj");
#endif
    rIn>>nPageNum;
}

SfxItemSet* SdrPageObj::CreateNewItemSet(SfxItemPool& rPool)
{
    return new SfxItemSet(rPool);
}

////////////////////////////////////////////////////////////////////////////////////////////////////


