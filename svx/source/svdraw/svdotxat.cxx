/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svl/style.hxx>
#include <svx/svdotext.hxx>
#include "svx/svditext.hxx"
#include <svx/svdmodel.hxx> // fuer GetMaxObjSize und GetStyleSheetPool
#include <svx/svdoutl.hxx>
#include <svx/svdorect.hxx> // fuer SetDirty bei NbcAdjustTextFrameWidthAndHeight
#include <svx/svdocapt.hxx> // fuer SetDirty bei NbcAdjustTextFrameWidthAndHeight
#include <svx/svdetc.hxx>
#include <editeng/writingmodeitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <svx/sdtfchim.hxx>


#include <editeng/editview.hxx>
#include <svl/smplhint.hxx>
#include <svl/whiter.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/outliner.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/fhgtitem.hxx>

#include <editeng/charscaleitem.hxx>
#include <svl/style.hxx>
#include <svl/itemiter.hxx>
#include <editeng/lrspitem.hxx>
#include <svl/itempool.hxx>
#include <editeng/numitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/postitem.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@@ @@@@@ @@   @@ @@@@@@  @@@@  @@@@@  @@@@@@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@     @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@@@    @@@     @@   @@  @@ @@@@@      @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@ @@  @@
//    @@   @@@@@ @@   @@   @@    @@@@  @@@@@   @@@@
//
//  Attribute, StyleSheets und AutoGrow
//
////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrTextObj::AdjustTextFrameWidthAndHeight(Rectangle& rR, FASTBOOL bHgt, FASTBOOL bWdt) const
{
    if (bTextFrame && pModel!=NULL && !rR.IsEmpty())
    {
        SdrFitToSizeType eFit=GetFitToSize();
        FASTBOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
        FASTBOOL bWdtGrow=bWdt && IsAutoGrowWidth();
        FASTBOOL bHgtGrow=bHgt && IsAutoGrowHeight();
        SdrTextAniKind eAniKind=GetTextAniKind();
        SdrTextAniDirection eAniDir=GetTextAniDirection();
        FASTBOOL bScroll=eAniKind==SDRTEXTANI_SCROLL || eAniKind==SDRTEXTANI_ALTERNATE || eAniKind==SDRTEXTANI_SLIDE;
        FASTBOOL bHScroll=bScroll && (eAniDir==SDRTEXTANI_LEFT || eAniDir==SDRTEXTANI_RIGHT);
        FASTBOOL bVScroll=bScroll && (eAniDir==SDRTEXTANI_UP || eAniDir==SDRTEXTANI_DOWN);
        if (!bFitToSize && (bWdtGrow || bHgtGrow))
        {
            Rectangle aR0(rR);
            long nHgt=0,nMinHgt=0,nMaxHgt=0;
            long nWdt=0,nMinWdt=0,nMaxWdt=0;
            Size aSiz(rR.GetSize()); aSiz.Width()--; aSiz.Height()--;
            Size aMaxSiz(100000,100000);
            Size aTmpSiz(pModel->GetMaxObjSize());
            if (aTmpSiz.Width()!=0) aMaxSiz.Width()=aTmpSiz.Width();
            if (aTmpSiz.Height()!=0) aMaxSiz.Height()=aTmpSiz.Height();
            if (bWdtGrow)
            {
                nMinWdt=GetMinTextFrameWidth();
                nMaxWdt=GetMaxTextFrameWidth();
                if (nMaxWdt==0 || nMaxWdt>aMaxSiz.Width()) nMaxWdt=aMaxSiz.Width();
                if (nMinWdt<=0) nMinWdt=1;
                aSiz.Width()=nMaxWdt;
            }
            if (bHgtGrow)
            {
                nMinHgt=GetMinTextFrameHeight();
                nMaxHgt=GetMaxTextFrameHeight();
                if (nMaxHgt==0 || nMaxHgt>aMaxSiz.Height()) nMaxHgt=aMaxSiz.Height();
                if (nMinHgt<=0) nMinHgt=1;
                aSiz.Height()=nMaxHgt;
            }
            long nHDist=GetTextLeftDistance()+GetTextRightDistance();
            long nVDist=GetTextUpperDistance()+GetTextLowerDistance();
            aSiz.Width()-=nHDist;
            aSiz.Height()-=nVDist;
            if (aSiz.Width()<2) aSiz.Width()=2;   // Mindestgroesse 2
            if (aSiz.Height()<2) aSiz.Height()=2; // Mindestgroesse 2

            // #101684#
            sal_Bool bInEditMode = IsInEditMode();

            if(!bInEditMode)
            {
                if (bHScroll) aSiz.Width()=0x0FFFFFFF; // Laufschrift nicht umbrechen
                if (bVScroll) aSiz.Height()=0x0FFFFFFF;
            }

            if(pEdtOutl)
            {
                pEdtOutl->SetMaxAutoPaperSize(aSiz);
                if (bWdtGrow) {
                    Size aSiz2(pEdtOutl->CalcTextSize());
                    nWdt=aSiz2.Width()+1; // lieber etwas Tolleranz
                    if (bHgtGrow) nHgt=aSiz2.Height()+1; // lieber etwas Tolleranz
                } else {
                    nHgt=pEdtOutl->GetTextHeight()+1; // lieber etwas Tolleranz
                }
            } else {
                Outliner& rOutliner=ImpGetDrawOutliner();
                rOutliner.SetPaperSize(aSiz);
                rOutliner.SetUpdateMode(sal_True);
                // !!! hier sollte ich wohl auch noch mal die Optimierung mit
                // bPortionInfoChecked usw einbauen
                OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
                if ( pOutlinerParaObject != NULL )
                {
                    rOutliner.SetText(*pOutlinerParaObject);
                    rOutliner.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
                }
                if (bWdtGrow)
                {
                    Size aSiz2(rOutliner.CalcTextSize());
                    nWdt=aSiz2.Width()+1; // lieber etwas Tolleranz
                    if (bHgtGrow) nHgt=aSiz2.Height()+1; // lieber etwas Tolleranz
                } else {
                    nHgt=rOutliner.GetTextHeight()+1; // lieber etwas Tolleranz
                }
                rOutliner.Clear();
            }
            if (nWdt<nMinWdt) nWdt=nMinWdt;
            if (nWdt>nMaxWdt) nWdt=nMaxWdt;
            nWdt+=nHDist;
            if (nWdt<1) nWdt=1; // nHDist kann auch negativ sein
            if (nHgt<nMinHgt) nHgt=nMinHgt;
            if (nHgt>nMaxHgt) nHgt=nMaxHgt;
            nHgt+=nVDist;
            if (nHgt<1) nHgt=1; // nVDist kann auch negativ sein
            long nWdtGrow=nWdt-(rR.Right()-rR.Left());
            long nHgtGrow=nHgt-(rR.Bottom()-rR.Top());
            if (nWdtGrow==0) bWdtGrow=sal_False;
            if (nHgtGrow==0) bHgtGrow=sal_False;
            if (bWdtGrow || bHgtGrow) {
                if (bWdtGrow) {
                    SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
                    if (eHAdj==SDRTEXTHORZADJUST_LEFT) rR.Right()+=nWdtGrow;
                    else if (eHAdj==SDRTEXTHORZADJUST_RIGHT) rR.Left()-=nWdtGrow;
                    else {
                        long nWdtGrow2=nWdtGrow/2;
                        rR.Left()-=nWdtGrow2;
                        rR.Right()=rR.Left()+nWdt;
                    }
                }
                if (bHgtGrow) {
                    SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
                    if (eVAdj==SDRTEXTVERTADJUST_TOP) rR.Bottom()+=nHgtGrow;
                    else if (eVAdj==SDRTEXTVERTADJUST_BOTTOM) rR.Top()-=nHgtGrow;
                    else {
                        long nHgtGrow2=nHgtGrow/2;
                        rR.Top()-=nHgtGrow2;
                        rR.Bottom()=rR.Top()+nHgt;
                    }
                }
                if (aGeo.nDrehWink!=0) {
                    Point aD1(rR.TopLeft());
                    aD1-=aR0.TopLeft();
                    Point aD2(aD1);
                    RotatePoint(aD2,Point(),aGeo.nSin,aGeo.nCos);
                    aD2-=aD1;
                    rR.Move(aD2.X(),aD2.Y());
                }
                return sal_True;
            }
        }
    }
    return sal_False;
}

FASTBOOL SdrTextObj::NbcAdjustTextFrameWidthAndHeight(FASTBOOL bHgt, FASTBOOL bWdt)
{
    FASTBOOL bRet=AdjustTextFrameWidthAndHeight(aRect,bHgt,bWdt);
    if (bRet) {
        SetRectsDirty();
        if (HAS_BASE(SdrRectObj,this)) { // mal wieder 'nen Hack
            ((SdrRectObj*)this)->SetXPolyDirty();
        }
        if (HAS_BASE(SdrCaptionObj,this)) { // mal wieder 'nen Hack
            ((SdrCaptionObj*)this)->ImpRecalcTail();
        }
    }
    return bRet;
}

FASTBOOL SdrTextObj::AdjustTextFrameWidthAndHeight(FASTBOOL bHgt, FASTBOOL bWdt)
{
    Rectangle aNeuRect(aRect);
    FASTBOOL bRet=AdjustTextFrameWidthAndHeight(aNeuRect,bHgt,bWdt);
    if (bRet) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        // #110094#-14 SendRepaintBroadcast();
        aRect=aNeuRect;
        SetRectsDirty();
        if (HAS_BASE(SdrRectObj,this)) { // mal wieder 'nen Hack
            ((SdrRectObj*)this)->SetXPolyDirty();
        }
        if (HAS_BASE(SdrCaptionObj,this)) { // mal wieder 'nen Hack
            ((SdrCaptionObj*)this)->ImpRecalcTail();
        }
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
    return bRet;
}

void SdrTextObj::ImpSetTextStyleSheetListeners()
{
    SfxStyleSheetBasePool* pStylePool=pModel!=NULL ? pModel->GetStyleSheetPool() : NULL;
    if (pStylePool!=NULL)
    {
        Container aStyles(1024,64,64);
        OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
        if (pOutlinerParaObject!=NULL)
        {
            // Zunaechst werden alle im ParaObject enthaltenen StyleSheets
            // im Container aStyles gesammelt. Dazu wird die Family jeweils
            // ans Ende des StyleSheet-Namen drangehaengt.
            const EditTextObject& rTextObj=pOutlinerParaObject->GetTextObject();
            XubString aStyleName;
            SfxStyleFamily eStyleFam;
            sal_uInt16 nParaAnz=rTextObj.GetParagraphCount();

            for(sal_uInt16 nParaNum(0); nParaNum < nParaAnz; nParaNum++)
            {
                rTextObj.GetStyleSheet(nParaNum, aStyleName, eStyleFam);

                if(aStyleName.Len())
                {
                    XubString aFam = UniString::CreateFromInt32((sal_uInt16)eStyleFam);
                    aFam.Expand(5);

                    aStyleName += sal_Unicode('|');
                    aStyleName += aFam;

                    sal_Bool bFnd(sal_False);
                    sal_uInt32 nNum(aStyles.Count());

                    while(!bFnd && nNum > 0)
                    {
                        // kein StyleSheet doppelt!
                        nNum--;
                        bFnd = (aStyleName.Equals(*(XubString*)aStyles.GetObject(nNum)));
                    }

                    if(!bFnd)
                    {
                        aStyles.Insert(new XubString(aStyleName), CONTAINER_APPEND);
                    }
                }
            }
        }

        // nun die Strings im Container durch StyleSheet* ersetzten
        sal_uIntPtr nNum=aStyles.Count();
        while (nNum>0) {
            nNum--;
            XubString* pName=(XubString*)aStyles.GetObject(nNum);

            // UNICODE: String aFam(pName->Cut(pName->Len()-6));
            String aFam = pName->Copy(0, pName->Len() - 6);

            aFam.Erase(0,1);
            aFam.EraseTrailingChars();

            // UNICODE: sal_uInt16 nFam=sal_uInt16(aFam);
            sal_uInt16 nFam = (sal_uInt16)aFam.ToInt32();

            SfxStyleFamily eFam=(SfxStyleFamily)nFam;
            SfxStyleSheetBase* pStyleBase=pStylePool->Find(*pName,eFam);
            SfxStyleSheet* pStyle=PTR_CAST(SfxStyleSheet,pStyleBase);
            delete pName;
            if (pStyle!=NULL && pStyle!=GetStyleSheet()) {
                aStyles.Replace(pStyle,nNum);
            } else {
                aStyles.Remove(nNum);
            }
        }
        // jetzt alle ueberfluessigen StyleSheets entfernen
        nNum=GetBroadcasterCount();
        while (nNum>0) {
            nNum--;
            SfxBroadcaster* pBroadcast=GetBroadcasterJOE((sal_uInt16)nNum);
            SfxStyleSheet* pStyle=PTR_CAST(SfxStyleSheet,pBroadcast);
            if (pStyle!=NULL && pStyle!=GetStyleSheet()) { // Sonderbehandlung fuer den StyleSheet des Objekts
                if (aStyles.GetPos(pStyle)==CONTAINER_ENTRY_NOTFOUND) {
                    EndListening(*pStyle);
                }
            }
        }
        // und schliesslich alle in aStyles enthaltenen StyleSheets mit den vorhandenen Broadcastern mergen
        nNum=aStyles.Count();
        while (nNum>0) {
            nNum--;
            SfxStyleSheet* pStyle=(SfxStyleSheet*)aStyles.GetObject(nNum);
            // StartListening soll selbst nachsehen, ob hier nicht evtl. schon gehorcht wird
            StartListening(*pStyle,sal_True);
        }
    }
}

void SdrTextObj::NbcResizeTextAttributes(const Fraction& xFact, const Fraction& yFact)
{
    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
    if (pOutlinerParaObject!=NULL && xFact.IsValid() && yFact.IsValid())
    {
        Fraction n100(100,1);
        long nX=long(xFact*n100);
        long nY=long(yFact*n100);
        if (nX<0) nX=-nX;
        if (nX<1) nX=1;
        if (nX>0xFFFF) nX=0xFFFF;
        if (nY<0) nY=-nY;
        if (nY<1) nY=1;
        if (nY>0xFFFF) nY=0xFFFF;
        if (nX!=100 || nY!=100)
        {
            // Rahmenattribute
            const SfxItemSet& rSet = GetObjectItemSet();
            const SvxCharScaleWidthItem& rOldWdt=(SvxCharScaleWidthItem&)rSet.Get(EE_CHAR_FONTWIDTH);
            const SvxFontHeightItem& rOldHgt=(SvxFontHeightItem&)rSet.Get(EE_CHAR_FONTHEIGHT);

            // erstmal die alten Werte holen
            long nRelWdt=rOldWdt.GetValue();
            long nAbsHgt=rOldHgt.GetHeight();
            long nRelHgt=rOldHgt.GetProp();

            // Relative Breite aendern
            nRelWdt*=nX;
            nRelWdt/=nY;
            if (nRelWdt<0) nRelWdt=-nRelWdt; // nicht negativ
            if (nRelWdt<=0) nRelWdt=1;       // und mind. 1%
            if (nRelWdt>0xFFFF) nRelWdt=0xFFFF;

            // Absolute Hoehe aendern
            nAbsHgt*=nY;
            nAbsHgt/=100;
            if (nAbsHgt<0) nAbsHgt=-nAbsHgt; // nicht negativ
            if (nAbsHgt<=0) nAbsHgt=1;       // und mind. 1
            if (nAbsHgt>0xFFFF) nAbsHgt=0xFFFF;

            // und nun attributieren
            SetObjectItem(SvxCharScaleWidthItem( (sal_uInt16) nRelWdt, EE_CHAR_FONTWIDTH));
            SetObjectItem(SvxFontHeightItem(nAbsHgt,(sal_uInt16)nRelHgt, EE_CHAR_FONTHEIGHT));
            // Zeichen- und Absatzattribute innerhalb des OutlinerParaObjects
            Outliner& rOutliner=ImpGetDrawOutliner();
            rOutliner.SetPaperSize(Size(LONG_MAX,LONG_MAX));
            rOutliner.SetText(*pOutlinerParaObject);
            rOutliner.DoStretchChars((sal_uInt16)nX,(sal_uInt16)nY);
            OutlinerParaObject* pNewPara=rOutliner.CreateParaObject();
            NbcSetOutlinerParaObject(pNewPara);
            rOutliner.Clear();
        }
    }
}

/** #103836# iterates over the paragraphs of a given SdrObject and removes all
             hard set character attributes with the which ids contained in the
             given vector
*/
void SdrTextObj::RemoveOutlinerCharacterAttribs( const std::vector<sal_uInt16>& rCharWhichIds )
{
    sal_Int32 nText = getTextCount();

    while( --nText >= 0 )
    {
        SdrText* pText = getText( nText );
        OutlinerParaObject* pOutlinerParaObject = pText ? pText->GetOutlinerParaObject() : 0;

        if(pOutlinerParaObject)
        {
            Outliner* pOutliner = 0;

            if( pEdtOutl || (pText == getActiveText()) )
                pOutliner = pEdtOutl;

            if(!pOutliner)
            {
                pOutliner = &ImpGetDrawOutliner();
                pOutliner->SetText(*pOutlinerParaObject);
            }

            ESelection aSelAll( 0, 0, 0xffff, 0xffff );
            std::vector<sal_uInt16>::const_iterator aIter( rCharWhichIds.begin() );
            while( aIter != rCharWhichIds.end() )
            {
                pOutliner->RemoveAttribs( aSelAll, false, (*aIter++) );
            }

            if(!pEdtOutl || (pText != getActiveText()) )
            {
                const sal_uInt32 nParaCount = pOutliner->GetParagraphCount();
                OutlinerParaObject* pTemp = pOutliner->CreateParaObject(0, (sal_uInt16)nParaCount);
                pOutliner->Clear();
                NbcSetOutlinerParaObjectForText(pTemp, pText);
            }
        }
    }
}

bool SdrTextObj::HasText() const
{
    if( pEdtOutl )
        return HasEditText();

    OutlinerParaObject* pOPO = GetOutlinerParaObject();

    bool bHasText = false;
    if( pOPO )
    {
        const EditTextObject& rETO = pOPO->GetTextObject();
        sal_uInt16 nParaCount = rETO.GetParagraphCount();

        if( nParaCount > 0 )
            bHasText = (nParaCount > 1) || (rETO.GetText( 0 ).Len() != 0);
    }

    return bHasText;
}
