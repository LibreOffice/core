/*************************************************************************
 *
 *  $RCSfile: svdedxv.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:24 $
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

#include "svdedxv.hxx"
#include <svtools/solar.hrc>

#ifndef _STRING_H
#include <tools/string.h>
#endif

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _EDITSTAT_HXX //autogen
#include <editstat.hxx>
#endif

#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif

#ifndef _SV_CURSOR_HXX //autogen
#include <vcl/cursor.hxx>
#endif

#include "svditext.hxx"
#include "svdoutl.hxx"
#include "svdxout.hxx"
#include "svdotext.hxx"
#include "svdundo.hxx"
#include "svditer.hxx"
#include "svdpagv.hxx"
#include "svdpage.hxx"
#include "svdetc.hxx"   // fuer GetDraftFillColor
#include "svdibrow.hxx"
#include "svdoutl.hxx"
#include "svddrgv.hxx"  // fuer SetSolidDragging()
#include "svdstr.hrc"   // Namen aus der Resource
#include "svdglob.hxx"  // StringCache

#ifndef _OUTLINER_HXX //autogen
#include "outliner.hxx"
#endif

#ifndef _SVX_ADJITEM_HXX //autogen
#include "adjitem.hxx"
#endif

#define SPOTCOUNT   5

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjEditView::ImpClearVars()
{
    bQuickTextEditMode=TRUE;
    bMacroMode=TRUE;
    pTextEditObj=NULL;
    pTextEditOutliner=NULL;
    pTextEditOutlinerView=NULL;
    pTextEditPV=NULL;
    pTextEditWin=NULL;
    pTextEditCursorMerker=NULL;
    pEditPara=NULL;
    bTextEditNewObj=FALSE;
    bMacroDown=FALSE;
    pMacroObj=NULL;
    pMacroPV=NULL;
    pMacroWin=NULL;
    nMacroTol=0;
    bTextEditDontDelete=FALSE;
    bTextEditOnlyOneView=FALSE;
}

SdrObjEditView::SdrObjEditView(SdrModel* pModel1, OutputDevice* pOut):
    SdrGlueEditView(pModel1,pOut)
{
    ImpClearVars();
}

SdrObjEditView::SdrObjEditView(SdrModel* pModel1, ExtOutputDevice* pXOut):
    SdrGlueEditView(pModel1,pXOut)
{
    ImpClearVars();
}

SdrObjEditView::~SdrObjEditView()
{
    pTextEditWin = NULL;            // Damit es in EndTextEdit kein ShowCursor gibt
    if (IsTextEdit()) EndTextEdit();
    if (pTextEditOutliner!=NULL) {
        delete pTextEditOutliner;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrObjEditView::IsAction() const
{
    return IsMacroObj() || SdrGlueEditView::IsAction();
}

void SdrObjEditView::MovAction(const Point& rPnt)
{
    if (IsMacroObj()) MovMacroObj(rPnt);
    SdrGlueEditView::MovAction(rPnt);
}

void SdrObjEditView::EndAction()
{
    if (IsMacroObj()) EndMacroObj();
    SdrGlueEditView::EndAction();
}

void SdrObjEditView::BckAction()
{
    BrkMacroObj();
    SdrGlueEditView::BckAction();
}

void SdrObjEditView::BrkAction()
{
    BrkMacroObj();
    SdrGlueEditView::BrkAction();
}

void SdrObjEditView::TakeActionRect(Rectangle& rRect) const
{
    if (IsMacroObj()) {
        rRect=pMacroObj->GetBoundRect()+pMacroPV->GetOffset();
    } else {
        SdrGlueEditView::TakeActionRect(rRect);
    }
}

void __EXPORT SdrObjEditView::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
{
    SdrGlueEditView::SFX_NOTIFY(rBC,rBCType,rHint,rHintType);
    // Printerwechsel waerend des Editierens
    SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
    if (pSdrHint!=NULL && pTextEditOutliner!=NULL) {
        SdrHintKind eKind=pSdrHint->GetKind();
        if (eKind==HINT_REFDEVICECHG) {
            pTextEditOutliner->SetRefDevice(pMod->GetRefDevice());
        }
        if (eKind==HINT_DEFAULTTABCHG) {
            pTextEditOutliner->SetDefTab(pMod->GetDefaultTabulator());
        }
        if (eKind==HINT_DEFFONTHGTCHG) {
            // ...
        }
        if (eKind==HINT_MODELSAVED) { // #43095#
            pTextEditOutliner->ClearModifyFlag();
        }
    }
}

void SdrObjEditView::ModelHasChanged()
{
    SdrGlueEditView::ModelHasChanged();
    if (IsTextEdit() && !pTextEditObj->IsInserted()) EndTextEdit(); // Objekt geloescht
    // TextEditObj geaendert?
    if (IsTextEdit()) {
        SdrTextObj* pTextObj=PTR_CAST(SdrTextObj,pTextEditObj);
        if (pTextObj!=NULL) {
            ULONG nOutlViewAnz=pTextEditOutliner->GetViewCount();
            BOOL bAreaChg=FALSE;
            BOOL bAnchorChg=FALSE;
            BOOL bColorChg=FALSE;
            BOOL bContourFrame=pTextObj->IsContourTextFrame();
            EVAnchorMode eNewAnchor;
            Rectangle aOldArea(aMinTextEditArea);
            aOldArea.Union(aTextEditArea);
            Color aNewColor;
            { // Area Checken
                Size aPaperMin1;
                Size aPaperMax1;
                Rectangle aEditArea1;
                Rectangle aMinArea1;
                pTextObj->TakeTextEditArea(&aPaperMin1,&aPaperMax1,&aEditArea1,&aMinArea1);
                Point aPvOfs(pTextEditPV->GetOffset());
                aEditArea1.Move(aPvOfs.X(),aPvOfs.Y());
                aMinArea1.Move(aPvOfs.X(),aPvOfs.Y());
                Rectangle aNewArea(aMinArea1);
                aNewArea.Union(aEditArea1);
                if (aNewArea!=aOldArea || aEditArea1!=aTextEditArea || aMinArea1!=aMinTextEditArea ||
                    pTextEditOutliner->GetMinAutoPaperSize()!=aPaperMin1 || pTextEditOutliner->GetMaxAutoPaperSize()!=aPaperMax1) {
                    aTextEditArea=aEditArea1;
                    aMinTextEditArea=aMinArea1;
                    pTextEditOutliner->SetUpdateMode(FALSE);
                    pTextEditOutliner->SetMinAutoPaperSize(aPaperMin1);
                    pTextEditOutliner->SetMaxAutoPaperSize(aPaperMax1);
                    pTextEditOutliner->SetPaperSize(Size(0,0)); // Damit der Outliner neu formatiert
                    if (!bContourFrame) {
                        pTextEditOutliner->ClearPolygon();
                        ULONG nStat=pTextEditOutliner->GetControlWord();
                        nStat|=EE_CNTRL_AUTOPAGESIZE;
                        pTextEditOutliner->SetControlWord(nStat);
                    } else {
                        ULONG nStat=pTextEditOutliner->GetControlWord();
                        nStat&=~EE_CNTRL_AUTOPAGESIZE;
                        pTextEditOutliner->SetControlWord(nStat);
                        Rectangle aAnchorRect;
                        pTextObj->TakeTextAnchorRect(aAnchorRect);
                        pTextObj->ImpSetContourPolygon(*pTextEditOutliner,aAnchorRect, TRUE);
                    }
                    for (ULONG nOV=0; nOV<nOutlViewAnz; nOV++) {
                        OutlinerView* pOLV=pTextEditOutliner->GetView(nOV);
                        ULONG nStat0=pOLV->GetControlWord();
                        ULONG nStat=nStat0;
                        // AutoViewSize nur wenn nicht KontourFrame.
                        if (!bContourFrame) nStat|=EV_CNTRL_AUTOSIZE;
                        else nStat&=~EV_CNTRL_AUTOSIZE;
                        if (nStat!=nStat0) pOLV->SetControlWord(nStat);
                    }
                    pTextEditOutliner->SetUpdateMode(TRUE);
                    bAreaChg=TRUE;
                }
            }
            if (pTextEditOutlinerView!=NULL) { // Fuellfarbe und Anker checken
                EVAnchorMode eOldAnchor=pTextEditOutlinerView->GetAnchorMode();
                eNewAnchor=(EVAnchorMode)pTextObj->GetOutlinerViewAnchorMode();
                bAnchorChg=eOldAnchor!=eNewAnchor;
                Color aOldColor(pTextEditOutlinerView->GetBackgroundColor());
                aNewColor=ImpGetTextEditBackgroundColor();
                bColorChg=aOldColor!=aNewColor;
            }
            if (bAreaChg || bAnchorChg || bColorChg)
            {
                for (ULONG nOV=0; nOV<nOutlViewAnz; nOV++)
                {
                    OutlinerView* pOLV=pTextEditOutliner->GetView(nOV);
                    { // Alten OutlinerView-Bereich invalidieren
                        Window* pWin=pOLV->GetWindow();
                        Rectangle aTmpRect(aOldArea);
                        USHORT nPixSiz=pOLV->GetInvalidateMore()+1;
                        Size aMore(pWin->PixelToLogic(Size(nPixSiz,nPixSiz)));
                        aTmpRect.Left()-=aMore.Width();
                        aTmpRect.Right()+=aMore.Width();
                        aTmpRect.Top()-=aMore.Height();
                        aTmpRect.Bottom()+=aMore.Height();
                        InvalidateOneWin(*pWin,aTmpRect);
                    }
                    if (bAnchorChg)
                        pOLV->SetAnchorMode(eNewAnchor);
                    if (bColorChg)
                        pOLV->SetBackgroundColor( aNewColor );

                    pOLV->SetOutputArea(aTextEditArea); // weil sonst scheinbar nicht richtig umgeankert wird
                    ImpPaintOutlinerView(*pOLV);
                }
                pTextEditOutlinerView->ShowCursor();
            }
        }
        ImpMakeTextCursorAreaVisible();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@@ @@@@@ @@   @@ @@@@@@  @@@@@ @@@@@  @@ @@@@@@
//    @@   @@    @@@ @@@   @@    @@    @@  @@ @@   @@
//    @@   @@     @@@@@    @@    @@    @@  @@ @@   @@
//    @@   @@@@    @@@     @@    @@@@  @@  @@ @@   @@
//    @@   @@     @@@@@    @@    @@    @@  @@ @@   @@
//    @@   @@    @@@ @@@   @@    @@    @@  @@ @@   @@
//    @@   @@@@@ @@   @@   @@    @@@@@ @@@@@  @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjEditView::ImpPaintOutlinerView(OutlinerView& rOutlView, const Rectangle* pRect, BOOL bDrawButPaint) const
{
    const SdrObject* pTextObjTmp=GetTextEditObject();
    const SdrTextObj* pText=PTR_CAST(SdrTextObj,pTextObjTmp);
    BOOL bTextFrame=pText!=NULL && pText->IsTextFrame();
    BOOL bFitToSize=(pTextEditOutliner->GetControlWord() & EE_CNTRL_STRETCHING) !=0;
    BOOL bContourFrame=pText!=NULL && pText->IsContourTextFrame();
    Window* pWin=rOutlView.GetWindow();
    Rectangle aBlankRect(rOutlView.GetOutputArea());
    aBlankRect.Union(aMinTextEditArea);
    Rectangle aPixRect(pWin->LogicToPixel(aBlankRect));
    if (pRect!=NULL) aBlankRect.Intersection(*pRect);
    BOOL bModifyMerk=pTextEditOutliner->IsModified(); // #43095#
    if (bDrawButPaint) {
        pTextEditOutliner->Draw(pWin,rOutlView.GetOutputArea());
    } else {
        rOutlView.GetOutliner()->SetUpdateMode(TRUE); // Bugfix #22596#
        rOutlView.Paint(aBlankRect);
    }
    if (!bModifyMerk) pTextEditOutliner->ClearModifyFlag(); // #43095#
    if (bTextFrame && !bFitToSize) {
        aPixRect.Left()--;
        aPixRect.Top()--;
        aPixRect.Right()++;
        aPixRect.Bottom()++;
        USHORT nPixSiz=rOutlView.GetInvalidateMore()-1;
        { // xPixRect Begrenzen, wegen Treiberproblem bei zu weit hinausragenden Pixelkoordinaten
            Size aMaxXY=pWin->GetOutputSizePixel();
            long a=2*nPixSiz;
            long nMaxX=aMaxXY.Width()+a;
            long nMaxY=aMaxXY.Height()+a;
            if (aPixRect.Left  ()<-a) aPixRect.Left()=-a;
            if (aPixRect.Top   ()<-a) aPixRect.Top ()=-a;
            if (aPixRect.Right ()>nMaxX) aPixRect.Right ()=nMaxX;
            if (aPixRect.Bottom()>nMaxY) aPixRect.Bottom()=nMaxY;
        }
        Rectangle aOuterPix(aPixRect);
        aOuterPix.Left()-=nPixSiz;
        aOuterPix.Top()-=nPixSiz;
        aOuterPix.Right()+=nPixSiz;
        aOuterPix.Bottom()+=nPixSiz;

        BOOL bXor=(bDrawButPaint && IsShownXorVisible(pWin));
        if (bXor) ((SdrPaintView*)this)->HideShownXor(pWin);
        BOOL bMerk=pWin->IsMapModeEnabled();
        pWin->EnableMapMode(FALSE);

        Color aGrayColor( COL_GRAY );
        pWin->SetFillColor( aGrayColor );
        pWin->SetLineColor();
        pWin->DrawRect(Rectangle(aOuterPix.Left(),aOuterPix.Top()  ,aPixRect.Left()  ,aOuterPix.Bottom()));
        pWin->DrawRect(Rectangle(aOuterPix.Left(),aOuterPix.Top()  ,aOuterPix.Right(),aPixRect.Top()    ));
        pWin->DrawRect(Rectangle(aOuterPix.Left(),aPixRect.Bottom(),aOuterPix.Right(),aOuterPix.Bottom()));
        pWin->DrawRect(Rectangle(aPixRect.Right(),aOuterPix.Top()  ,aOuterPix.Right(),aOuterPix.Bottom()));
        pWin->EnableMapMode(bMerk);
        if (bXor) ((SdrPaintView*)this)->ShowShownXor(pWin);
    }
    rOutlView.ShowCursor();
}

Color SdrObjEditView::ImpGetTextEditBackgroundColor() const
{
    Color aBackground(COL_WHITE);
    BOOL bFound=FALSE;
    SdrTextObj* pText=PTR_CAST(SdrTextObj,pTextEditObj);
    if (pText!=NULL && pText->IsClosedObj())
    {
        SfxItemSet aSet(pMod->GetItemPool());
        pText->TakeAttributes(aSet,FALSE,FALSE);
        bFound=GetDraftFillColor(aSet,aBackground);
    }
    if (!bFound && pTextEditPV!=NULL && pTextEditObj!=NULL)
    {
        // Ok, dann eben die Page durchsuchen!
        Point aPvOfs(pTextEditPV->GetOffset());
        const SdrPage* pPg=pTextEditPV->GetPage();

        Point aSpotPos[SPOTCOUNT];
        Color aSpotColor[SPOTCOUNT];
        Rectangle aSnapRect( pTextEditObj->GetSnapRect() );
        ULONG nHeight( aSnapRect.GetSize().Height() );
        ULONG nWidth( aSnapRect.GetSize().Width() );
        ULONG nWidth14  = nWidth / 4;
        ULONG nHeight14 = nHeight / 4;
        ULONG nWidth34  = ( 3 * nWidth ) / 4;
        ULONG nHeight34 = ( 3 * nHeight ) / 4;

        const SetOfByte& rVisiLayers = pTextEditPV->GetVisibleLayers();

        USHORT i;
        for ( i = 0; i < SPOTCOUNT; i++ )
        {
            // Es wird anhand von fuenf Spots die Farbe untersucht
            switch ( i )
            {
                case 0 :
                {
                    // Center-Spot
                    aSpotPos[i] = aSnapRect.Center();
                    aSpotPos[i] += aPvOfs;
                }
                break;

                case 1 :
                {
                    // TopLeft-Spot
                    aSpotPos[i] = aSnapRect.TopLeft();
                    aSpotPos[i].X() += nWidth14;
                    aSpotPos[i].Y() += nHeight14;
                    aSpotPos[i] += aPvOfs;
                }
                break;

                case 2 :
                {
                    // TopRight-Spot
                    aSpotPos[i] = aSnapRect.TopLeft();
                    aSpotPos[i].X() += nWidth34;
                    aSpotPos[i].Y() += nHeight14;
                    aSpotPos[i] += aPvOfs;
                }
                break;

                case 3 :
                {
                    // BottomLeft-Spot
                    aSpotPos[i] = aSnapRect.TopLeft();
                    aSpotPos[i].X() += nWidth14;
                    aSpotPos[i].Y() += nHeight34;
                    aSpotPos[i] += aPvOfs;
                }
                break;

                case 4 :
                {
                    // BottomRight-Spot
                    aSpotPos[i] = aSnapRect.TopLeft();
                    aSpotPos[i].X() += nWidth34;
                    aSpotPos[i].Y() += nHeight34;
                    aSpotPos[i] += aPvOfs;
                }
                break;

            }

            aSpotColor[i] = Color( COL_WHITE );
            pPg->GetFillColor(aSpotPos[i], rVisiLayers, bLayerSortedRedraw, aSpotColor[i]);
        }

        USHORT aMatch[SPOTCOUNT];

        for ( i = 0; i < SPOTCOUNT; i++ )
        {
            // Wurden gleiche Spot-Farben gefuden?
            aMatch[i] = 0;

            for ( USHORT j = 0; j < SPOTCOUNT; j++ )
            {
                if( j != i )
                {
                    if( aSpotColor[i] == aSpotColor[j] )
                    {
                        aMatch[i]++;
                    }
                }
            }
        }

        // Das hoechste Gewicht hat der Spot in der Mitte
        aBackground = aSpotColor[0];

        for ( USHORT nMatchCount = SPOTCOUNT - 1; nMatchCount > 1; nMatchCount-- )
        {
            // Welche Spot-Farbe wurde am haeufigsten gefunden?
            for ( USHORT i = 0; i < SPOTCOUNT; i++ )
            {
                if( aMatch[i] == nMatchCount )
                {
                    aBackground = aSpotColor[i];
                    nMatchCount = 1;   // Abbruch auch der aeusseren for-Schleife
                    break;
                }
            }
        }
    }

    return aBackground;
}

OutlinerView* SdrObjEditView::ImpMakeOutlinerView(Window* pWin, BOOL bNoPaint, OutlinerView* pGivenView) const
{
    // Hintergrund
    Color aBackground(ImpGetTextEditBackgroundColor());
    SdrTextObj* pText=PTR_CAST(SdrTextObj,pTextEditObj);
    BOOL bTextFrame=pText!=NULL && pText->IsTextFrame();
    BOOL bContourFrame=pText!=NULL && pText->IsContourTextFrame();
    // OutlinerView erzeugen
    OutlinerView* pOutlView=pGivenView;
    pTextEditOutliner->SetUpdateMode(FALSE);
    if (pOutlView==NULL) pOutlView=new OutlinerView(pTextEditOutliner,pWin);
    else pOutlView->SetWindow(pWin);
    // Scrollen verbieten
    ULONG nStat=pOutlView->GetControlWord();
    nStat&=~EV_CNTRL_AUTOSCROLL;
    // AutoViewSize nur wenn nicht KontourFrame.
    if (!bContourFrame) nStat|=EV_CNTRL_AUTOSIZE;
    if (bTextFrame) {
        USHORT nPixSiz=aHdl.GetHdlSize()*2+1;
        nStat|=EV_CNTRL_INVONEMORE;
        pOutlView->SetInvalidateMore(nPixSiz);
    }
    pOutlView->SetControlWord(nStat);
    pOutlView->SetBackgroundColor( aBackground );
    if (pText!=NULL) {
        pOutlView->SetAnchorMode((EVAnchorMode)(pText->GetOutlinerViewAnchorMode()));
    }
    pOutlView->SetOutputArea(aTextEditArea);
    pTextEditOutliner->SetUpdateMode(TRUE);
    ImpPaintOutlinerView(*pOutlView,NULL,TRUE);
    return pOutlView;
}

BOOL SdrObjEditView::IsTextEditFrame() const
{
    SdrTextObj* pText=PTR_CAST(SdrTextObj,pTextEditObj);
    return pText!=NULL && pText->IsTextFrame();
}

IMPL_LINK(SdrObjEditView,ImpOutlinerStatusEventHdl,EditStatus*,pEditStat)
{
    if (pTextEditOutliner==NULL || pTextEditObj==NULL) return 0;
    ULONG nStat=pEditStat->GetStatusWord();
    BOOL bGrowX=(nStat & EE_STAT_TEXTWIDTHCHANGED) !=0;
    BOOL bGrowY=(nStat & EE_STAT_TEXTHEIGHTCHANGED) !=0;
    SdrTextObj* pTextObj=PTR_CAST(SdrTextObj,pTextEditObj);
    BOOL bTextFrame=pTextObj!=NULL && pTextObj->IsTextFrame();
    if (!bTextFrame || (!bGrowX && !bGrowY)) return 0;
    BOOL bAutoGrowHgt=bTextFrame && pTextObj->IsAutoGrowHeight();
    BOOL bAutoGrowWdt=bTextFrame && pTextObj->IsAutoGrowWidth();

    if ((bGrowX && bAutoGrowWdt) || (bGrowY && bAutoGrowHgt)) {
        if (pTextObj->AdjustTextFrameWidthAndHeight()) {
        }
    }
    return 0;
}

IMPL_LINK(SdrObjEditView,ImpOutlinerCalcFieldValueHdl,EditFieldInfo*,pFI)
{
    BOOL bOk=FALSE;
    String& rStr=pFI->GetRepresentation();
    rStr.Erase();
    SdrTextObj* pTextObj=PTR_CAST(SdrTextObj,pTextEditObj);
    if (pTextObj!=NULL) {
        Color* pTxtCol=NULL;
        Color* pFldCol=NULL;
        bOk=pTextObj->CalcFieldValue(pFI->GetField(),pFI->GetPara(),pFI->GetPos(),TRUE,pTxtCol,pFldCol,rStr);
        if (bOk) {
            if (pTxtCol!=NULL) {
                pFI->SetTxtColor(*pTxtCol);
                delete pTxtCol;
            }
            if (pFldCol!=NULL) {
                pFI->SetFldColor(*pFldCol);
                delete pFldCol;
            } else {
                pFI->SetFldColor(Color(COL_LIGHTGRAY)); // kann spaeter (357) raus
            }
        }
    }
    Outliner& rDrawOutl=pMod->GetDrawOutliner(pTextObj);
    Link aDrawOutlLink=rDrawOutl.GetCalcFieldValueHdl();
    if (!bOk && aDrawOutlLink.IsSet()) {
        aDrawOutlLink.Call(pFI);
        bOk = (BOOL)rStr.Len();
    }
    if (!bOk && aOldCalcFieldValueLink.IsSet()) {
        return aOldCalcFieldValueLink.Call(pFI);
    }
    return 0;
}

BOOL SdrObjEditView::BegTextEdit(SdrObject* pObj, SdrPageView* pPV, Window* pWin,
    SdrOutliner* pGivenOutliner, OutlinerView* pGivenOutlinerView,
    BOOL bDontDeleteOutliner, BOOL bOnlyOneView)
{
    return BegTextEdit(pObj,pPV,pWin,FALSE,pGivenOutliner,pGivenOutlinerView,bDontDeleteOutliner,bOnlyOneView);
}

BOOL SdrObjEditView::BegTextEdit(SdrObject* pObj, SdrPageView* pPV, Window* pWin, BOOL bIsNewObj,
    SdrOutliner* pGivenOutliner, OutlinerView* pGivenOutlinerView,
    BOOL bDontDeleteOutliner, BOOL bOnlyOneView)
{
    EndTextEdit();
    if (!HAS_BASE(SdrTextObj,pObj)) return FALSE; // z.Zt. nur mit meinen Textobjekten
    bTextEditDontDelete=bDontDeleteOutliner && pGivenOutliner!=NULL;
    bTextEditOnlyOneView=bOnlyOneView;
    bTextEditNewObj=bIsNewObj;
    USHORT nWinAnz=GetWinCount();
    USHORT i;
    BOOL bBrk=FALSE;
    // Abbruch, wenn kein Objekt angegeben.
    if (pObj==NULL) bBrk=TRUE;
    if (!bBrk && pWin==NULL) {
        for (i=0; i<nWinAnz && pWin==NULL; i++) {
            OutputDevice* pOut=GetWin(i);
            if (pOut->GetOutDevType()==OUTDEV_WINDOW) pWin=(Window*)pOut;
        }
        // Abbruch, wenn kein Window da.
        if (pWin==NULL) bBrk=TRUE;
    }
    if (!bBrk && pPV==NULL) {
        SdrPage* pPage=pObj->GetPage();
        pPV=GetPageView(pPage);
        // Abbruch, wenn keine PageView zu dem Objekt vorhanden.
        if (pPV==NULL) bBrk=TRUE;
    }
    if (pObj!=NULL && pPV!=NULL) {
        // Kein TextEdit an Objekten im gesperrten Layer
        if (pPV->GetLockedLayers().IsSet(pObj->GetLayer())) {
            bBrk=TRUE;
        }
    }

    if (pTextEditOutliner!=NULL) {
        DBG_ERROR("SdrObjEditView::BegTextEdit() da stand noch ein alter Outliner rum");
        delete pTextEditOutliner;
        pTextEditOutliner=NULL;
    }

    if (!bBrk) {
        pTextEditWin=pWin;
        pTextEditPV=pPV;
        pTextEditObj=pObj;
        pTextEditOutliner=pGivenOutliner;
        if (pTextEditOutliner==NULL)
            pTextEditOutliner = SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, pTextEditObj->GetModel() );

        BOOL bEmpty=pTextEditObj->GetOutlinerParaObject()==NULL;

        aOldCalcFieldValueLink=pTextEditOutliner->GetCalcFieldValueHdl();
        // Der FieldHdl muss von BegTextEdit gesetzt sein, da dor ein UpdateFields gerufen wird.
        pTextEditOutliner->SetCalcFieldValueHdl(LINK(this,SdrObjEditView,ImpOutlinerCalcFieldValueHdl));
        if (pTextEditObj->BegTextEdit(*pTextEditOutliner)) {
            // alten Cursor merken
            if (pTextEditOutliner->GetViewCount()!=0) {
                OutlinerView* pTmpOLV=pTextEditOutliner->RemoveView(ULONG(0));
                if (pTmpOLV!=NULL && pTmpOLV!=pGivenOutlinerView) delete pTmpOLV;
            }
            // EditArea ueberTakeTextEditArea bestimmen
            // Das koennte eigentlich entfallen, da TakeTextRect() die Berechnung der aTextEditArea vornimmt
            // Die aMinTextEditArea muss jedoch wohl auch erfolgen (darum bleibt es voerst drinnen)
            ((SdrTextObj*)pTextEditObj)->TakeTextEditArea(NULL,NULL,&aTextEditArea,&aMinTextEditArea);

            Rectangle aTextRect;
            Rectangle aAnchorRect;
            ((SdrTextObj*)pTextEditObj)->TakeTextRect(*pTextEditOutliner, aTextRect, TRUE, &aAnchorRect, FALSE);

            if ( !((SdrTextObj*)pTextEditObj)->IsContourTextFrame() )
            {
                // FitToSize erstmal nicht mit ContourFrame
                SdrFitToSizeType eFit = ((SdrTextObj*)pTextEditObj)->GetFitToSize();
                if (eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES)
                    aTextRect = aAnchorRect;
            }

            aTextEditArea = aTextRect;

            Point aPvOfs(pTextEditPV->GetOffset());
            aTextEditArea.Move(aPvOfs.X(),aPvOfs.Y());
            aMinTextEditArea.Move(aPvOfs.X(),aPvOfs.Y());
            pTextEditCursorMerker=pWin->GetCursor();

            aHdl.SetMoveOutside(TRUE);
            RefreshAllIAOManagers();

            pTextEditOutlinerView=ImpMakeOutlinerView(pWin,!bEmpty,pGivenOutlinerView);
            pTextEditOutliner->InsertView(pTextEditOutlinerView,0);

            aHdl.SetMoveOutside(FALSE);
            aHdl.SetMoveOutside(TRUE);
            RefreshAllIAOManagers();

            // alle Wins als OutlinerView beim Outliner anmelden
            if (!bOnlyOneView) {
                for (i=0; i<nWinAnz; i++) {
                    OutputDevice* pOut=GetWin(i);
                    if (pOut!=pWin && pOut->GetOutDevType()==OUTDEV_WINDOW) {
                        OutlinerView* pOutlView=ImpMakeOutlinerView((Window*)pOut,!bEmpty,NULL);
                        pTextEditOutliner->InsertView(pOutlView,i);
                    }
                }
            }
            pWin->GrabFocus(); // Damit der Cursor hier auch blinkt
            pTextEditOutlinerView->ShowCursor();
            pTextEditOutliner->SetStatusEventHdl(LINK(this,SdrObjEditView,ImpOutlinerStatusEventHdl));
            if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
            pTextEditOutliner->ClearModifyFlag();
            return TRUE; // Gut gelaufen, TextEdit laeuft nun
        } else {
            bBrk=TRUE;
            pTextEditOutliner->SetCalcFieldValueHdl(aOldCalcFieldValueLink);
        }
    }
    // wenn hier angekommen, dann ist irgendwas schief gelaufen
    if (!bDontDeleteOutliner) {
        if (pGivenOutliner!=NULL) { delete pGivenOutliner; pTextEditOutliner=NULL; }
        if (pGivenOutlinerView!=NULL) delete pGivenOutlinerView;
    }
    if (pTextEditOutliner!=NULL) delete pTextEditOutliner;
    pTextEditOutliner=NULL;
    pTextEditOutlinerView=NULL;
    pTextEditObj=NULL;
    pTextEditPV=NULL;
    pTextEditWin=NULL;
    if (bMarkHdlWhenTextEdit) {
        HideMarkHdl(NULL);
    }
    aHdl.SetMoveOutside(FALSE);
    RefreshAllIAOManagers();
    ShowMarkHdl(NULL);
    return FALSE;
}

SdrEndTextEditKind SdrObjEditView::EndTextEdit(BOOL bDontDeleteReally)
{
    SdrEndTextEditKind eRet=SDRENDTEXTEDIT_UNCHANGED;
    SdrObject*    pTEObj         =pTextEditObj;
    SdrPageView*  pTEPV          =pTextEditPV;
    Window*       pTEWin         =pTextEditWin;
    SdrOutliner*  pTEOutliner    =pTextEditOutliner;
    OutlinerView* pTEOutlinerView=pTextEditOutlinerView;
    Cursor*       pTECursorMerker=pTextEditCursorMerker;

    pTextEditObj=NULL;
    pTextEditPV=NULL;
    pTextEditWin=NULL;
    pTextEditOutliner=NULL;
    pTextEditOutlinerView=NULL;
    pTextEditCursorMerker=NULL;
    aTextEditArea=Rectangle();

    if (pTEOutliner!=NULL) {
        BOOL bModified=pTEOutliner->IsModified();
        if (pTEOutlinerView!=NULL) {
            pTEOutlinerView->CompleteAutoCorrect(); // #43095#
            pTEOutlinerView->HideCursor();
        }
        if (pTEObj!=NULL) {
            pTEOutliner->CompleteOnlineSpelling();
            Point aPvOfs(pTEPV->GetOffset());
            SdrUndoObjSetText* pTxtUndo=bModified ? new SdrUndoObjSetText(*pTEObj) : NULL;
            // Den alten CalcFieldValue-Handler wieder setzen
            // Muss vor Obj::EndTextEdit() geschehen, da dort ein UpdateFields() gemacht wird.
            pTEOutliner->SetCalcFieldValueHdl(aOldCalcFieldValueLink);

            pTEObj->EndTextEdit(*pTEOutliner);

            if ( pTEObj->GetRotateAngle() != 0 )
                pTEObj->SendRepaintBroadcast();  // Sonst wird nicht alles restauriert

            if (pTxtUndo!=NULL) {
                pTxtUndo->AfterSetText();
                if (!pTxtUndo->IsDifferent()) { delete pTxtUndo; pTxtUndo=NULL; }
            }
            // Loeschung des gesamten TextObj checken
            SdrUndoDelObj* pDelUndo=NULL;
            BOOL bDelObj=FALSE;
            SdrTextObj* pTextObj=PTR_CAST(SdrTextObj,pTEObj);
            if (pTextObj!=NULL && bTextEditNewObj) {
                bDelObj=pTextObj->IsTextFrame() &&
                        pTextObj->GetOutlinerParaObject()==NULL &&
                        !pTextObj->IsEmptyPresObj() &&
                        !pTextObj->HasFill() &&
                        !pTextObj->HasLine();
                if (pTEObj->IsInserted() && bDelObj && pTextObj->GetObjInventor()==SdrInventor && !bDontDeleteReally) {
                    SdrObjKind eIdent=(SdrObjKind)pTextObj->GetObjIdentifier();
                    if (eIdent==OBJ_TEXT || eIdent==OBJ_TEXTEXT) {
                        pDelUndo=new SdrUndoDelObj(*pTEObj);
                    }
                }
            }
            XubString aObjName;
            pTEObj->TakeObjNameSingul(aObjName);
            BegUndo(ImpGetResStr(STR_UndoObjSetText),aObjName);
            if (pTxtUndo!=NULL) { AddUndo(pTxtUndo); eRet=SDRENDTEXTEDIT_CHANGED; }
            if (pDelUndo!=NULL) {
                AddUndo(pDelUndo);
                eRet=SDRENDTEXTEDIT_DELETED;
                DBG_ASSERT(pTEObj->GetObjList()!=NULL,"SdrObjEditView::EndTextEdit(): Fatal: Editiertes Objekt hat keine ObjList!");
                if (pTEObj->GetObjList()!=NULL) {
                    pTEObj->GetObjList()->RemoveObject(pTEObj->GetOrdNum());
                    CheckMarked(); // und gleich die Maekierung entfernen...
                }
            } else if (bDelObj) { // Fuer den Writer: Loeschen muss die App nachholen.
                eRet=SDRENDTEXTEDIT_SHOULDBEDELETED;
            }
            EndUndo(); // EndUndo hinter Remove, falls der UndoStack gleich weggehaun' wird
        }
        // alle OutlinerViews loeschen
        for (ULONG i=pTEOutliner->GetViewCount(); i>0;) {
            i--;
            OutlinerView* pOLV=pTEOutliner->GetView(i);
            USHORT nMorePix=pOLV->GetInvalidateMore();
            Window* pWin=pOLV->GetWindow();
            Rectangle aRect(pOLV->GetOutputArea());
            pTEOutliner->RemoveView(i);
            if (!bTextEditDontDelete || i!=0) {
                // die nullte gehoert mir u.U. nicht.
                delete pOLV;
            }
            aRect.Union(aTextEditArea);
            aRect.Union(aMinTextEditArea);
            aRect=pWin->LogicToPixel(aRect);
            aRect.Left()-=nMorePix;
            aRect.Top()-=nMorePix;
            aRect.Right()+=nMorePix;
            aRect.Bottom()+=nMorePix;
            aRect=pWin->PixelToLogic(aRect);
            InvalidateOneWin(*pWin,aRect);
        }
        // und auch den Outliner selbst
        if (!bTextEditDontDelete) delete pTEOutliner;
        else pTEOutliner->Clear();
        if (pTEWin!=NULL) {
            pTEWin->SetCursor(pTECursorMerker);
        }
        if (bMarkHdlWhenTextEdit) {
            HideMarkHdl(NULL);
        }
        aHdl.SetMoveOutside(FALSE);
        RefreshAllIAOManagers();
        if (eRet==SDRENDTEXTEDIT_UNCHANGED) {
            ShowMarkHdl(NULL); // Handles kommen ansonsten via Broadcast
        } else aMark.SetNameDirty();
        if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
    }
    return eRet;
}

OutlinerView* SdrObjEditView::ImpFindOutlinerView(Window* pWin) const
{
    if (pWin==NULL) return NULL;
    if (pTextEditOutliner==NULL) return NULL;
    OutlinerView* pNewView=NULL;
    ULONG nWinAnz=pTextEditOutliner->GetViewCount();
    for (ULONG i=0; i<nWinAnz && pNewView==NULL; i++) {
        OutlinerView* pView=pTextEditOutliner->GetView(i);
        if (pView->GetWindow()==pWin) pNewView=pView;
    }
    return pNewView;
}

void SdrObjEditView::SetTextEditWin(Window* pWin)
{
    if (pTextEditObj!=NULL && pWin!=NULL && pWin!=pTextEditWin) {
        OutlinerView* pNewView=ImpFindOutlinerView(pWin);
        if (pNewView!=NULL && pNewView!=pTextEditOutlinerView) {
            if (pTextEditOutlinerView!=NULL) {
                pTextEditOutlinerView->HideCursor();
            }
            pTextEditOutlinerView=pNewView;
            pTextEditWin=pWin;
            pWin->GrabFocus(); // Damit der Cursor hier auch blinkt
            pNewView->ShowCursor();
            ImpMakeTextCursorAreaVisible();
        }
    }
}

BOOL SdrObjEditView::IsTextEditHit(const Point& rHit, short nTol) const
{
    BOOL bOk=FALSE;
    if (pTextEditObj!=NULL) {
        nTol=ImpGetHitTolLogic(nTol,NULL);
        // nur drittel Toleranz hier, damit die Handles
        // noch vernuenftig getroffen werden koennen
        nTol=nTol/3;
        nTol=0; // Joe am 6.3.1997: Keine Hittoleranz mehr hier
        if (!bOk) {
            Rectangle aEditArea;
            OutlinerView* pOLV=pTextEditOutliner->GetView(0);
            if (pOLV!=NULL) {
                aEditArea.Union(pOLV->GetOutputArea());
            }
            aEditArea.Left()-=nTol;
            aEditArea.Top()-=nTol;
            aEditArea.Right()+=nTol;
            aEditArea.Bottom()+=nTol;
            bOk=aEditArea.IsInside(rHit);
            if (bOk) { // Nun noch checken, ob auch wirklich Buchstaben getroffen wurden
                Point aPnt(rHit); aPnt-=aEditArea.TopLeft();
                long nHitTol = 2000;
                OutputDevice* pRef = pTextEditOutliner->GetRefDevice();
                if( pRef )
                    nHitTol = pRef->LogicToLogic( nHitTol, MAP_100TH_MM, pRef->GetMapMode().GetMapUnit() );

                bOk = pTextEditOutliner->IsTextPos( aPnt, nHitTol );
            }
        }
    }
    return bOk;
}

BOOL SdrObjEditView::IsTextEditFrameHit(const Point& rHit) const
{
    BOOL bOk=FALSE;
    if (pTextEditObj!=NULL) {
        SdrTextObj* pText=PTR_CAST(SdrTextObj,pTextEditObj);
        OutlinerView* pOLV=pTextEditOutliner->GetView(0);
        if( pOLV )
        {
            Window* pWin=pOLV->GetWindow();
            if (pText!=NULL && pText->IsTextFrame() && pOLV!=NULL && pWin!=NULL) {
                USHORT nPixSiz=pOLV->GetInvalidateMore();
                Rectangle aEditArea(aMinTextEditArea);
                aEditArea.Union(pOLV->GetOutputArea());
                if (!aEditArea.IsInside(rHit)) {
                    Size aSiz(pWin->PixelToLogic(Size(nPixSiz,nPixSiz)));
                    aEditArea.Left()-=aSiz.Width();
                    aEditArea.Top()-=aSiz.Height();
                    aEditArea.Right()+=aSiz.Width();
                    aEditArea.Bottom()+=aSiz.Height();
                    bOk=aEditArea.IsInside(rHit);
                }
            }
        }
    }
    return bOk;
}

void SdrObjEditView::AddTextEditOfs(MouseEvent& rMEvt) const
{
    if (pTextEditObj!=NULL) {
        Point aPvOfs(pTextEditPV->GetOffset());
        Point aObjOfs(pTextEditObj->GetLogicRect().TopLeft());
        (Point&)(rMEvt.GetPosPixel())+=aPvOfs+aObjOfs;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrObjEditView::KeyInput(const KeyEvent& rKEvt, Window* pWin)
{
    if(pTextEditOutlinerView)
    {
        if(rKEvt.GetKeyCode().GetCode() == KEY_RETURN && pTextEditOutliner->GetParagraphCount() == 1)
        {
            ByteString aLine(
                pTextEditOutliner->GetText(pTextEditOutliner->GetParagraph( 0 ), 1),
                gsl_getSystemTextEncoding());
            aLine = aLine.ToUpperAscii();

            if(aLine == "HELLO JOE, PLEASE SHOW THE ITEMBROWSER")
                ShowItemBrowser();
        }
        if (pTextEditOutlinerView->PostKeyEvent(rKEvt))
        {
            if( pMod && !pMod->IsChanged() )
            {
                if( pTextEditOutliner && pTextEditOutliner->IsModified() )
                    pMod->SetChanged( TRUE );
            }

            if (pWin!=NULL && pWin!=pTextEditWin) SetTextEditWin(pWin);
            if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
            ImpMakeTextCursorAreaVisible();
            return TRUE;
        }
    }
    return SdrGlueEditView::KeyInput(rKEvt,pWin);
}

BOOL SdrObjEditView::MouseButtonDown(const MouseEvent& rMEvt, Window* pWin)
{
    if (pTextEditOutlinerView!=NULL) {
        BOOL bPostIt=pTextEditOutliner->IsInSelectionMode();
        if (!bPostIt) {
            Point aPt(rMEvt.GetPosPixel());
            if (pWin!=NULL) aPt=pWin->PixelToLogic(aPt);
            else if (pTextEditWin!=NULL) aPt=pTextEditWin->PixelToLogic(aPt);
            bPostIt=IsTextEditHit(aPt,nHitTolLog);
        }
        if (bPostIt) {
            Point aPixPos(rMEvt.GetPosPixel());
            Rectangle aR(pWin->LogicToPixel(pTextEditOutlinerView->GetOutputArea()));
            if (aPixPos.X()<aR.Left  ()) aPixPos.X()=aR.Left  ();
            if (aPixPos.X()>aR.Right ()) aPixPos.X()=aR.Right ();
            if (aPixPos.Y()<aR.Top   ()) aPixPos.Y()=aR.Top   ();
            if (aPixPos.Y()>aR.Bottom()) aPixPos.Y()=aR.Bottom();
            MouseEvent aMEvt(aPixPos,rMEvt.GetClicks(),rMEvt.GetMode(),
                             rMEvt.GetButtons(),rMEvt.GetModifier());
            if (pTextEditOutlinerView->MouseButtonDown(aMEvt)) {
                if (pWin!=NULL && pWin!=pTextEditWin) SetTextEditWin(pWin);
                if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
                ImpMakeTextCursorAreaVisible();
                return TRUE;
            }
        }
    }
    return SdrGlueEditView::MouseButtonDown(rMEvt,pWin);
}

BOOL SdrObjEditView::MouseButtonUp(const MouseEvent& rMEvt, Window* pWin)
{
    if (pTextEditOutlinerView!=NULL) {
        BOOL bPostIt=pTextEditOutliner->IsInSelectionMode();
        if (!bPostIt) {
            Point aPt(rMEvt.GetPosPixel());
            if (pWin!=NULL) aPt=pWin->PixelToLogic(aPt);
            else if (pTextEditWin!=NULL) aPt=pTextEditWin->PixelToLogic(aPt);
            bPostIt=IsTextEditHit(aPt,nHitTolLog);
        }
        if (bPostIt) {
            Point aPixPos(rMEvt.GetPosPixel());
            Rectangle aR(pWin->LogicToPixel(pTextEditOutlinerView->GetOutputArea()));
            if (aPixPos.X()<aR.Left  ()) aPixPos.X()=aR.Left  ();
            if (aPixPos.X()>aR.Right ()) aPixPos.X()=aR.Right ();
            if (aPixPos.Y()<aR.Top   ()) aPixPos.Y()=aR.Top   ();
            if (aPixPos.Y()>aR.Bottom()) aPixPos.Y()=aR.Bottom();
            MouseEvent aMEvt(aPixPos,rMEvt.GetClicks(),rMEvt.GetMode(),
                             rMEvt.GetButtons(),rMEvt.GetModifier());
            if (pTextEditOutlinerView->MouseButtonUp(aMEvt)) {
                if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
                ImpMakeTextCursorAreaVisible();
                return TRUE;
            }
        }
    }
    return SdrGlueEditView::MouseButtonUp(rMEvt,pWin);
}

BOOL SdrObjEditView::MouseMove(const MouseEvent& rMEvt, Window* pWin)
{
    if (pTextEditOutlinerView!=NULL) {
        BOOL bSelMode=pTextEditOutliner->IsInSelectionMode();
        BOOL bPostIt=bSelMode;
        if (!bPostIt) {
            Point aPt(rMEvt.GetPosPixel());
            if (pWin!=NULL) aPt=pWin->PixelToLogic(aPt);
            else if (pTextEditWin!=NULL) aPt=pTextEditWin->PixelToLogic(aPt);
            bPostIt=IsTextEditHit(aPt,nHitTolLog);
        }
        if (bPostIt) {
            Point aPixPos(rMEvt.GetPosPixel());
            Rectangle aR(pWin->LogicToPixel(pTextEditOutlinerView->GetOutputArea()));
            if (aPixPos.X()<aR.Left  ()) aPixPos.X()=aR.Left  ();
            if (aPixPos.X()>aR.Right ()) aPixPos.X()=aR.Right ();
            if (aPixPos.Y()<aR.Top   ()) aPixPos.Y()=aR.Top   ();
            if (aPixPos.Y()>aR.Bottom()) aPixPos.Y()=aR.Bottom();
            MouseEvent aMEvt(aPixPos,rMEvt.GetClicks(),rMEvt.GetMode(),
                             rMEvt.GetButtons(),rMEvt.GetModifier());
            if (pTextEditOutlinerView->MouseMove(aMEvt) && bSelMode) {
                if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
                ImpMakeTextCursorAreaVisible();
                return TRUE;
            }
        }
    }
    return SdrGlueEditView::MouseMove(rMEvt,pWin);
}

BOOL SdrObjEditView::Command(const CommandEvent& rCEvt, Window* pWin)
{
    // solange bis die OutlinerView einen BOOL zurueckliefert
    // bekommt sie nur COMMAND_STARTDRAG
    if (pTextEditOutlinerView!=NULL)
    {
        if (rCEvt.GetCommand()==COMMAND_STARTDRAG) {
            BOOL bPostIt=pTextEditOutliner->IsInSelectionMode() || !rCEvt.IsMouseEvent();
            if (!bPostIt && rCEvt.IsMouseEvent()) {
                Point aPt(rCEvt.GetMousePosPixel());
                if (pWin!=NULL) aPt=pWin->PixelToLogic(aPt);
                else if (pTextEditWin!=NULL) aPt=pTextEditWin->PixelToLogic(aPt);
                bPostIt=IsTextEditHit(aPt,nHitTolLog);
            }
            if (bPostIt) {
                Point aPixPos(rCEvt.GetMousePosPixel());
                if (rCEvt.IsMouseEvent()) {
                    Rectangle aR(pWin->LogicToPixel(pTextEditOutlinerView->GetOutputArea()));
                    if (aPixPos.X()<aR.Left  ()) aPixPos.X()=aR.Left  ();
                    if (aPixPos.X()>aR.Right ()) aPixPos.X()=aR.Right ();
                    if (aPixPos.Y()<aR.Top   ()) aPixPos.Y()=aR.Top   ();
                    if (aPixPos.Y()>aR.Bottom()) aPixPos.Y()=aR.Bottom();
                }
                CommandEvent aCEvt(aPixPos,rCEvt.GetCommand(),rCEvt.IsMouseEvent());
                // Command ist an der OutlinerView leider void
                pTextEditOutlinerView->Command(aCEvt);
                if (pWin!=NULL && pWin!=pTextEditWin) SetTextEditWin(pWin);
                if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
                ImpMakeTextCursorAreaVisible();
                return TRUE;
            }
        }
        else // if (rCEvt.GetCommand() == COMMAND_VOICE )
        {
            pTextEditOutlinerView->Command(rCEvt);
            return TRUE;
        }
    }
    return SdrGlueEditView::Command(rCEvt,pWin);
}

BOOL SdrObjEditView::Cut(ULONG nFormat)
{
    if (pTextEditOutliner!=NULL) {
        pTextEditOutlinerView->Cut();
        if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
        ImpMakeTextCursorAreaVisible();
        return TRUE;
    } else {
        return SdrGlueEditView::Cut(nFormat);
    }
}

BOOL SdrObjEditView::Yank(ULONG nFormat)
{
    if (pTextEditOutliner!=NULL) {
        pTextEditOutlinerView->Copy();
        return TRUE;
    } else {
        return SdrGlueEditView::Yank(nFormat);
    }
}

BOOL SdrObjEditView::Paste(Window* pWin, ULONG nFormat)
{
    if (pTextEditOutliner!=NULL) {
        if (pWin!=NULL) {
            OutlinerView* pNewView=ImpFindOutlinerView(pWin);
            if (pNewView!=NULL) {
                pNewView->Paste();
            }
        } else {
            pTextEditOutlinerView->Paste();
        }
        if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
        ImpMakeTextCursorAreaVisible();
        return TRUE;
    } else {
        return SdrGlueEditView::Paste(pWin,nFormat);
    }
}

BOOL SdrObjEditView::QueryDrop(DropEvent& rDEvt, Window* pWin, ULONG nFormat, USHORT nItemNum)
{
    Point aPt(rDEvt.GetPosPixel());
    if (pWin!=NULL) aPt=pWin->PixelToLogic(aPt);
    else if (pTextEditWin!=NULL) aPt=pTextEditWin->PixelToLogic(aPt);
    if (IsTextEditHit(aPt,nHitTolLog)) {
        OutlinerView* pNewView=ImpFindOutlinerView(pWin);
        if (pNewView!=NULL) {
            pNewView->QueryDrop(rDEvt);
        } else {
            if (pTextEditOutlinerView!=NULL) pTextEditOutlinerView->QueryDrop(rDEvt);
        }
        return TRUE;
    }
    return SdrGlueEditView::QueryDrop(rDEvt,pWin,nFormat,nItemNum);
}

BOOL SdrObjEditView::Drop(const DropEvent& rDEvt, Window* pWin, ULONG nFormat, USHORT nItemNum)
{
    Point aPt(rDEvt.GetPosPixel());
    if (pWin!=NULL) aPt=pWin->PixelToLogic(aPt);
    else if (pTextEditWin!=NULL) aPt=pTextEditWin->PixelToLogic(aPt);
    if (IsTextEditHit(aPt,nHitTolLog)) {
        OutlinerView* pNewView=ImpFindOutlinerView(pWin);
        if (pNewView!=NULL) {
            pNewView->Drop(rDEvt);
        } else {
            if (pTextEditOutlinerView!=NULL)  pTextEditOutlinerView->Drop(rDEvt);
        }
        if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
        ImpMakeTextCursorAreaVisible();
        return TRUE;
    }
    return SdrGlueEditView::Drop(rDEvt,pWin,nFormat,nItemNum);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrObjEditView::ImpIsTextEditAllSelected() const
{
    BOOL bRet=FALSE;
    if (pTextEditOutliner!=NULL && pTextEditOutlinerView!=NULL) {
        BOOL bEmpty=FALSE;
        ULONG nParaAnz=pTextEditOutliner->GetParagraphCount();
        Paragraph* p1stPara=pTextEditOutliner->GetParagraph( 0 );
        Paragraph* pLastPara=pTextEditOutliner->GetParagraph( nParaAnz > 1 ? nParaAnz - 1 : 0 );
        if (p1stPara==NULL) nParaAnz=0;
        if (nParaAnz==1) { // bei nur einem Para nachsehen ob da ueberhaupt was drin steht
            XubString aStr(pTextEditOutliner->GetText(p1stPara));

            // Aha, steht nix drin!
            if(!aStr.Len())
                nParaAnz = 0;
        }
        bEmpty=nParaAnz==0;
        if (!bEmpty) {
            ESelection aESel(pTextEditOutlinerView->GetSelection());
            if (aESel.nStartPara==0 && aESel.nStartPos==0 && aESel.nEndPara==USHORT(nParaAnz-1)) {
                XubString aStr(pTextEditOutliner->GetText(pLastPara));

                if(aStr.Len() == aESel.nEndPos)
                    bRet = TRUE;
            }
            // und nun auch noch fuer den Fall, das rueckwaerts selektiert wurde
            if (!bRet && aESel.nEndPara==0 && aESel.nEndPos==0 && aESel.nStartPara==USHORT(nParaAnz-1)) {
                XubString aStr(pTextEditOutliner->GetText(pLastPara));

                if(aStr.Len() == aESel.nStartPos)
                    bRet = TRUE;
            }
        } else {
            bRet=TRUE;
        }
    }
    return bRet;
}

void SdrObjEditView::ImpMakeTextCursorAreaVisible()
{
    if (pTextEditOutlinerView!=NULL && pTextEditWin!=NULL) {
        Cursor* pCsr=pTextEditWin->GetCursor();
        if (pCsr!=NULL) {
            Size aSiz(pCsr->GetSize());
            if (aSiz.Width()!=0 && aSiz.Height()!=0) { // #38450#
                MakeVisible(Rectangle(pCsr->GetPos(),aSiz),*pTextEditWin);
            }
        }
    }
}

/* new interface src537 */
BOOL SdrObjEditView::GetAttributes(SfxItemSet& rTargetSet, BOOL bOnlyHardAttr) const
{
    if(IsTextEdit())
    {
        DBG_ASSERT(pTextEditOutlinerView!=NULL,"SdrObjEditView::GetAttributes(): pTextEditOutlinerView=NULL");
        DBG_ASSERT(pTextEditOutliner!=NULL,"SdrObjEditView::GetAttributes(): pTextEditOutliner=NULL");
        pTextEditObj->TakeAttributes(rTargetSet, TRUE, bOnlyHardAttr);

        if(pTextEditOutlinerView)
        {
            // FALSE= InvalidItems nicht al Default, sondern als "Loecher" betrachten
            rTargetSet.Put(pTextEditOutlinerView->GetAttribs(), FALSE);
        }

        if(aMark.GetMarkCount()==1 && aMark.GetMark(0)->GetObj()==pTextEditObj)
        {
            MergeNotPersistAttrFromMarked(rTargetSet, bOnlyHardAttr);
        }

        return TRUE;
    }
    else
    {
        return SdrGlueEditView::GetAttributes(rTargetSet, bOnlyHardAttr);
    }
}

BOOL SdrObjEditView::SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll)
{
    BOOL bRet=FALSE;
    BOOL bTextEdit=pTextEditOutlinerView!=NULL;
    BOOL bAllTextSelected=ImpIsTextEditAllSelected();
    SfxItemSet* pModifiedSet=NULL;
    const SfxItemSet* pSet=&rSet;
    const SvxAdjustItem* pParaJust=NULL;
    if (!bTextEdit) {
        // Kein TextEdit aktiv -> alle Items ans Zeichenobjekt
        bRet=SdrGlueEditView::SetAttributes(*pSet,bReplaceAll);
    } else {
#ifdef DBG_UTIL
        {
            BOOL bHasEEFeatureItems=FALSE;
            SfxItemIter aIter(rSet);
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
                aMessage.AppendAscii("SdrObjEditView::SetAttributes(): Das setzen von EE_FEATURE-Items an der SdrView macht keinen Sinn! Es fuehrt nur zu Overhead und nicht mehr lesbaren Dokumenten.");
                InfoBox(NULL, aMessage).Execute();
            }
        }
#endif
        BOOL bOnlyEEItems;
        BOOL bNoEEItems=!SearchOutlinerItems(*pSet,bReplaceAll,&bOnlyEEItems);
        // alles selektiert? -> Attrs auch an den Rahmen
        // und falls keine EEItems, dann Attrs nur an den Rahmen
        if (bAllTextSelected || bNoEEItems) {
            String aStr;
            ImpTakeDescriptionStr(STR_EditSetAttributes,aStr);
            BegUndo(aStr);
            AddUndo(new SdrUndoGeoObj(*pTextEditObj));
            AddUndo(new SdrUndoAttrObj(*pTextEditObj,FALSE,!bNoEEItems));
            EndUndo();
            pTextEditObj->SetAttributes(*pSet,bReplaceAll);
            FlushComeBackTimer(); // Damit ModeHasChanged sofort kommt
            bRet=TRUE;
        } else if (!bOnlyEEItems) { // sonst Set ggf. splitten
            // Es wird nun ein ItemSet aSet gemacht, in den die EE_Items von
            // *pSet nicht enhalten ist (ansonsten ist es eine Kopie).
            USHORT* pNewWhichTable=RemoveWhichRange(pSet->GetRanges(),EE_ITEMS_START,EE_ITEMS_END);
            SfxItemSet aSet(pMod->GetItemPool(),pNewWhichTable);
            delete pNewWhichTable;
            SfxWhichIter aIter(aSet);
            USHORT nWhich=aIter.FirstWhich();
            while (nWhich!=0) {
                const SfxPoolItem* pItem;
                SfxItemState eState=pSet->GetItemState(nWhich,FALSE,&pItem);
                if (eState==SFX_ITEM_SET) aSet.Put(*pItem);
                nWhich=aIter.NextWhich();
            }
            String aStr;
            ImpTakeDescriptionStr(STR_EditSetAttributes,aStr);
            BegUndo(aStr);
            AddUndo(new SdrUndoGeoObj(*pTextEditObj));
            AddUndo(new SdrUndoAttrObj(*pTextEditObj,FALSE,FALSE));
            EndUndo();
            pTextEditObj->SetAttributes(aSet,bReplaceAll);
            if (aMark.GetMarkCount()==1 && aMark.GetMark(0)->GetObj()==pTextEditObj) {
                SetNotPersistAttrToMarked(aSet,bReplaceAll);
            }
            FlushComeBackTimer();
            bRet=TRUE;
        }
        if (!bNoEEItems) {
            // und nun die Attribute auch noch an die EditEngine
            if (bReplaceAll) {
                // Am Outliner kann man leider nur alle Attribute platthauen
                pTextEditOutlinerView->RemoveAttribs( TRUE );
            }
            pTextEditOutlinerView->SetAttribs(rSet);
            if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
            ImpMakeTextCursorAreaVisible();
        }
        bRet=TRUE;
    }
    if (pModifiedSet!=NULL) delete pModifiedSet;
    return bRet;
}

SfxStyleSheet* SdrObjEditView::GetStyleSheet(BOOL& rOk) const
{
    if ( pTextEditOutlinerView )
    {
        rOk=TRUE;
        SfxStyleSheet* pSheet = pTextEditOutlinerView->GetStyleSheet();
        return pSheet;
    }
    else
    {
        return SdrGlueEditView::GetStyleSheet(rOk);
    }
}

BOOL SdrObjEditView::SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr)
{
    if (pTextEditOutlinerView!=NULL) {
        BOOL bAllSelected=ImpIsTextEditAllSelected();
        if (bAllSelected) {
            String aStr;
            if (pStyleSheet!=NULL) ImpTakeDescriptionStr(STR_EditSetStylesheet,aStr);
            else ImpTakeDescriptionStr(STR_EditDelStylesheet,aStr);
            BegUndo(aStr);
            AddUndo(new SdrUndoGeoObj(*pTextEditObj));
            AddUndo(new SdrUndoAttrObj(*pTextEditObj,TRUE,TRUE));
            EndUndo();
            pTextEditObj->SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
        }
        if (pStyleSheet!=NULL) {
            // Der Outliner entfernt bei der Zuweisung eines StyleSheets immer
            // dir entsprechenden harten Attribute. Jedoch nur vom Absatz,
            // keine Zeichenattribute. Und genau so soll das scheinbar auch
            // sein, auch wenn es nicht ganz meinem Interface entspricht.
            // (Joe M. 27-11-1995)
            pTextEditOutlinerView->SetStyleSheet(pStyleSheet);
        }
        if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
        ImpMakeTextCursorAreaVisible();
        return TRUE;
    } else {
        return SdrGlueEditView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjEditView::AddWin(OutputDevice* pWin1)
{
    SdrGlueEditView::AddWin(pWin1);
    if (pTextEditObj!=NULL && !bTextEditOnlyOneView && pWin1->GetOutDevType()==OUTDEV_WINDOW) {
        OutlinerView* pOutlView=ImpMakeOutlinerView((Window*)pWin1,FALSE,NULL);
        pTextEditOutliner->InsertView(pOutlView);
    }
}

void SdrObjEditView::DelWin(OutputDevice* pWin1)
{
    SdrGlueEditView::DelWin(pWin1);
    if (pTextEditObj!=NULL && !bTextEditOnlyOneView && pWin1->GetOutDevType()==OUTDEV_WINDOW) {
        for (ULONG i=pTextEditOutliner->GetViewCount(); i>0;) {
            i--;
            OutlinerView* pOLV=pTextEditOutliner->GetView(i);
            if (pOLV && pOLV->GetWindow()==(Window*)pWin1) {
                delete pTextEditOutliner->RemoveView(i);
            }
        }
    }
}

BOOL SdrObjEditView::IsTextEditInSelectionMode() const
{
    return pTextEditOutliner!=NULL && pTextEditOutliner->IsInSelectionMode();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@   @@  @@@@   @@@@  @@@@@   @@@@   @@   @@  @@@@  @@@@@  @@@@@
//  @@@ @@@ @@  @@ @@  @@ @@  @@ @@  @@  @@@ @@@ @@  @@ @@  @@ @@
//  @@@@@@@ @@  @@ @@     @@  @@ @@  @@  @@@@@@@ @@  @@ @@  @@ @@
//  @@@@@@@ @@@@@@ @@     @@@@@  @@  @@  @@@@@@@ @@  @@ @@  @@ @@@@
//  @@ @ @@ @@  @@ @@     @@  @@ @@  @@  @@ @ @@ @@  @@ @@  @@ @@
//  @@   @@ @@  @@ @@  @@ @@  @@ @@  @@  @@   @@ @@  @@ @@  @@ @@
//  @@   @@ @@  @@  @@@@  @@  @@  @@@@   @@   @@  @@@@  @@@@@  @@@@@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrObjEditView::BegMacroObj(const Point& rPnt, short nTol, SdrObject* pObj, SdrPageView* pPV, Window* pWin)
{
    BOOL bRet=FALSE;
    BrkMacroObj();
    if (pObj!=NULL && pPV!=NULL && pWin!=NULL && pObj->HasMacro()) {
        nTol=ImpGetHitTolLogic(nTol,NULL);
        pMacroObj=pObj;
        pMacroPV=pPV;
        pMacroWin=pWin;
        bMacroDown=FALSE;
        nMacroTol=USHORT(nTol);
        aMacroDownPos=rPnt;
        aMacroDownPos-=pMacroPV->GetOffset();
        MovMacroObj(rPnt);
    }
    return bRet;
}

void SdrObjEditView::ImpMacroUp(const Point& rUpPos)
{
    if (pMacroObj!=NULL && bMacroDown) {
        pXOut->SetOutDev(pMacroWin);
        BOOL bOn=IsShownXorVisible(pMacroWin);
        if (bOn) HideShownXor(pMacroWin);
        Point aOfs(pMacroPV->GetOffset());
        pXOut->SetOffset(aOfs);
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=rUpPos;
        aHitRec.aDownPos=aMacroDownPos;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        aHitRec.pOut=pMacroWin;
        pMacroObj->PaintMacro(*pXOut,Rectangle(),aHitRec);
        pXOut->SetOffset(Point(0,0));
        if (bOn) ShowShownXor(pMacroWin);
        bMacroDown=FALSE;
    }
}

void SdrObjEditView::ImpMacroDown(const Point& rDownPos)
{
    if (pMacroObj!=NULL && !bMacroDown) {
        pXOut->SetOutDev(pMacroWin);
        BOOL bOn=IsShownXorVisible(pMacroWin);
        if (bOn) HideShownXor(pMacroWin);
        Point aOfs(pMacroPV->GetOffset());
        pXOut->SetOffset(aOfs);
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=rDownPos;
        aHitRec.aDownPos=aMacroDownPos;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        aHitRec.bDown=TRUE;
        aHitRec.pOut=pMacroWin;
        pMacroObj->PaintMacro(*pXOut,Rectangle(),aHitRec);
        pXOut->SetOffset(Point(0,0));
        if (bOn) ShowShownXor(pMacroWin);
        bMacroDown=TRUE;
    }
}

void SdrObjEditView::MovMacroObj(const Point& rPnt)
{
    if (pMacroObj!=NULL) {
        Point aPnt(rPnt-pMacroPV->GetOffset());
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=aPnt;
        aHitRec.aDownPos=aMacroDownPos;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        aHitRec.bDown=bMacroDown;
        aHitRec.pOut=pMacroWin;
        BOOL bDown=pMacroObj->IsMacroHit(aHitRec);
        if (bDown) ImpMacroDown(aPnt);
        else ImpMacroUp(aPnt);
    }
}

void SdrObjEditView::BrkMacroObj()
{
    if (pMacroObj!=NULL) {
        ImpMacroUp(aMacroDownPos);
        pMacroObj=NULL;
        pMacroPV=NULL;
        pMacroWin=NULL;
    }
}

BOOL SdrObjEditView::EndMacroObj()
{
    if (pMacroObj!=NULL && bMacroDown) {
        ImpMacroUp(aMacroDownPos);
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=aMacroDownPos;
        aHitRec.aDownPos=aMacroDownPos;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        aHitRec.bDown=TRUE;
        aHitRec.pOut=pMacroWin;
        BOOL bRet=pMacroObj->DoMacro(aHitRec);
        pMacroObj=NULL;
        pMacroPV=NULL;
        pMacroWin=NULL;
        return bRet;
    } else {
        BrkMacroObj();
        return FALSE;
    }
}

