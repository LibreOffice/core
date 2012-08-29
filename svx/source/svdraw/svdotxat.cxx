/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <comphelper/string.hxx>
#include <svl/style.hxx>
#include <svx/svdotext.hxx>
#include "svx/svditext.hxx"
#include <svx/svdmodel.hxx> // for GetMaxObjSize and GetStyleSheetPool
#include <svx/svdoutl.hxx>
#include <svx/svdorect.hxx> // for SetDirty at NbcAdjustTextFrameWidthAndHeight
#include <svx/svdocapt.hxx> // for SetDirty at NbcAdjustTextFrameWidthAndHeight
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
#include <editeng/editobj.hxx>
#include <editeng/fhgtitem.hxx>

#include <editeng/charscaleitem.hxx>
#include <svl/itemiter.hxx>
#include <editeng/lrspitem.hxx>
#include <svl/itempool.hxx>
#include <editeng/numitem.hxx>
#include <editeng/postitem.hxx>

#include <set>


bool SdrTextObj::AdjustTextFrameWidthAndHeight(Rectangle& rR, bool bHgt, bool bWdt) const
{
    if (bTextFrame && pModel!=NULL && !rR.IsEmpty())
    {
        bool bFitToSize(IsFitToSize());
        bool bWdtGrow=bWdt && IsAutoGrowWidth();
        bool bHgtGrow=bHgt && IsAutoGrowHeight();
        SdrTextAniKind eAniKind=GetTextAniKind();
        SdrTextAniDirection eAniDir=GetTextAniDirection();
        bool bScroll=eAniKind==SDRTEXTANI_SCROLL || eAniKind==SDRTEXTANI_ALTERNATE || eAniKind==SDRTEXTANI_SLIDE;
        bool bHScroll=bScroll && (eAniDir==SDRTEXTANI_LEFT || eAniDir==SDRTEXTANI_RIGHT);
        bool bVScroll=bScroll && (eAniDir==SDRTEXTANI_UP || eAniDir==SDRTEXTANI_DOWN);
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
            if (aSiz.Width()<2) aSiz.Width()=2;
            if (aSiz.Height()<2) aSiz.Height()=2;

            sal_Bool bInEditMode = IsInEditMode();

            if(!bInEditMode)
            {
                if (bHScroll) aSiz.Width()=0x0FFFFFFF; // don't break ticker text
                if (bVScroll) aSiz.Height()=0x0FFFFFFF;
            }

            if(pEdtOutl)
            {
                pEdtOutl->SetMaxAutoPaperSize(aSiz);
                if (bWdtGrow) {
                    Size aSiz2(pEdtOutl->CalcTextSize());
                    nWdt=aSiz2.Width()+1; // a little tolerance
                    if (bHgtGrow) nHgt=aSiz2.Height()+1; // a little tolerance
                } else {
                    nHgt=pEdtOutl->GetTextHeight()+1; // a little tolerance
                }
            } else {
                Outliner& rOutliner=ImpGetDrawOutliner();
                rOutliner.SetPaperSize(aSiz);
                rOutliner.SetUpdateMode(sal_True);
                // TODO: add the optimization with bPortionInfoChecked etc. here
                OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
                if ( pOutlinerParaObject != NULL )
                {
                    rOutliner.SetText(*pOutlinerParaObject);
                    rOutliner.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
                }
                if (bWdtGrow)
                {
                    Size aSiz2(rOutliner.CalcTextSize());
                    nWdt=aSiz2.Width()+1; // a little tolerance
                    if (bHgtGrow) nHgt=aSiz2.Height()+1; // a little tolerance
                } else {
                    nHgt=rOutliner.GetTextHeight()+1; // a little tolerance
                }
                rOutliner.Clear();
            }
            if (nWdt<nMinWdt) nWdt=nMinWdt;
            if (nWdt>nMaxWdt) nWdt=nMaxWdt;
            nWdt+=nHDist;
            if (nWdt<1) nWdt=1; // nHDist may be negative
            if (nHgt<nMinHgt) nHgt=nMinHgt;
            if (nHgt>nMaxHgt) nHgt=nMaxHgt;
            nHgt+=nVDist;
            if (nHgt<1) nHgt=1; // nVDist may be negative
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

bool SdrTextObj::NbcAdjustTextFrameWidthAndHeight(bool bHgt, bool bWdt)
{
    bool bRet=AdjustTextFrameWidthAndHeight(aRect,bHgt,bWdt);
    if (bRet) {
        SetRectsDirty();
        if (HAS_BASE(SdrRectObj,this)) { // this is a hack
            ((SdrRectObj*)this)->SetXPolyDirty();
        }
        if (HAS_BASE(SdrCaptionObj,this)) { // this is a hack
            ((SdrCaptionObj*)this)->ImpRecalcTail();
        }
    }
    return bRet;
}

bool SdrTextObj::AdjustTextFrameWidthAndHeight(bool bHgt, bool bWdt)
{
    Rectangle aNeuRect(aRect);
    bool bRet=AdjustTextFrameWidthAndHeight(aNeuRect,bHgt,bWdt);
    if (bRet) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        aRect=aNeuRect;
        SetRectsDirty();
        if (HAS_BASE(SdrRectObj,this)) { // this is a hack
            ((SdrRectObj*)this)->SetXPolyDirty();
        }
        if (HAS_BASE(SdrCaptionObj,this)) { // this is a hack
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
        std::vector<XubString*> aStyleNames;
        OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
        if (pOutlinerParaObject!=NULL)
        {
            // First, we collect all stylesheets contained in the ParaObject in
            // the container aStyles. The Family is always appended to the name
            // of the stylesheet.
            const EditTextObject& rTextObj=pOutlinerParaObject->GetTextObject();
            XubString aStyleName;
            SfxStyleFamily eStyleFam;
            sal_uInt16 nParaAnz=rTextObj.GetParagraphCount();

            for(sal_uInt16 nParaNum(0); nParaNum < nParaAnz; nParaNum++)
            {
                rTextObj.GetStyleSheet(nParaNum, aStyleName, eStyleFam);

                if(aStyleName.Len())
                {
                    OUStringBuffer aFam;
                    aFam.append(static_cast<sal_Int32>(eStyleFam));
                    comphelper::string::padToLength(aFam, 5, ' ');

                    aStyleName += sal_Unicode('|');
                    aStyleName += aFam.makeStringAndClear();

                    sal_Bool bFnd(sal_False);
                    sal_uInt32 nNum(aStyleNames.size());

                    while(!bFnd && nNum > 0)
                    {
                        // we don't want duplicate stylesheets
                        nNum--;
                        bFnd = aStyleName.Equals(*aStyleNames[nNum]);
                    }

                    if(!bFnd)
                    {
                        aStyleNames.push_back(new XubString(aStyleName));
                    }
                }
            }
        }

        // now convert the strings in the vector from names to StyleSheet*
        std::set<SfxStyleSheet*> aStyleSheets;
        while (!aStyleNames.empty()) {
            XubString* pName=aStyleNames.back();
            aStyleNames.pop_back();

            String aFam = pName->Copy(0, pName->Len() - 6);

            aFam.Erase(0,1);
            aFam = comphelper::string::stripEnd(aFam, ' ');

            sal_uInt16 nFam = (sal_uInt16)aFam.ToInt32();

            SfxStyleFamily eFam=(SfxStyleFamily)nFam;
            SfxStyleSheetBase* pStyleBase=pStylePool->Find(*pName,eFam);
            SfxStyleSheet* pStyle=PTR_CAST(SfxStyleSheet,pStyleBase);
            delete pName;
            if (pStyle!=NULL && pStyle!=GetStyleSheet()) {
                aStyleSheets.insert(pStyle);
            }
        }
        // now remove all superfluous stylesheets
        sal_uIntPtr nNum=GetBroadcasterCount();
        while (nNum>0) {
            nNum--;
            SfxBroadcaster* pBroadcast=GetBroadcasterJOE((sal_uInt16)nNum);
            SfxStyleSheet* pStyle=PTR_CAST(SfxStyleSheet,pBroadcast);
            if (pStyle!=NULL && pStyle!=GetStyleSheet()) { // special case for stylesheet of the object
                if (aStyleSheets.find(pStyle)==aStyleSheets.end()) {
                    EndListening(*pStyle);
                }
            }
        }
        // and finally, merge all stylesheets that are contained in aStyles with previous broadcasters
        for(std::set<SfxStyleSheet*>::const_iterator it = aStyleSheets.begin(); it != aStyleSheets.end(); ++it) {
            SfxStyleSheet* pStyle=*it;
            // let StartListening see for itself if there's already a listener registered
            StartListening(*pStyle,sal_True);
        }
    }
}

/**  iterates over the paragraphs of a given SdrObject and removes all
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
