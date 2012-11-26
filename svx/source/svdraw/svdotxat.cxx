/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svl/style.hxx>
#include <svx/svdotext.hxx>
#include <editeng/editdata.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svditext.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdocapt.hxx>
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
#include <svx/svdlegacy.hxx>
#include <svx/svdtrans.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

basegfx::B2DRange SdrTextObj::AdjustTextFrameWidthAndHeight(const basegfx::B2DRange& rRange, bool bHgt, bool bWdt) const
{
    if(IsTextFrame() && !rRange.isEmpty())
    {
        const SdrFitToSizeType eFit(GetFitToSize());
        const bool bFitToSize(SDRTEXTFIT_PROPORTIONAL == eFit || SDRTEXTFIT_ALLLINES == eFit);

        if(!bFitToSize)
        {
            return ImpAdjustTextFrameWidthAndHeight(rRange, bHgt, bWdt, true);
        }
    }

    return rRange;
}

basegfx::B2DRange SdrTextObj::ImpAdjustTextFrameWidthAndHeight(const basegfx::B2DRange& rRange, bool bHgt, bool bWdt, bool bCheckAnimation) const
{
    bool bWdtGrow(bWdt && IsAutoGrowWidth());
    bool bHgtGrow(bHgt && IsAutoGrowHeight());
    basegfx::B2DRange aRetval(rRange);

    if(bWdtGrow || bHgtGrow)
    {
        basegfx::B2DVector aSize(aRetval.getRange());
        basegfx::B2DVector aMaxSize(100000.0, 100000.0);
        double fHeight(0.0), fMinHeight(0.0), fMaxHeight(0.0);
        double fWidth(0.0), fMinWidth(0.0), fMaxWidth(0.0);
        const basegfx::B2DPoint aOriginalMinimum(aRetval.getMinimum());

        if(!basegfx::fTools::equalZero(getSdrModelFromSdrObject().GetMaxObjectScale().getX()))
        {
            aMaxSize.setX(fabs(getSdrModelFromSdrObject().GetMaxObjectScale().getX()));
        }

        if(!basegfx::fTools::equalZero(getSdrModelFromSdrObject().GetMaxObjectScale().getY()))
        {
            aMaxSize.setY(fabs(getSdrModelFromSdrObject().GetMaxObjectScale().getY()));
        }

        if(bWdtGrow)
        {
            fMinWidth = GetMinTextFrameWidth();
            fMaxWidth = GetMaxTextFrameWidth();

            if(basegfx::fTools::equalZero(fMaxWidth) || basegfx::fTools::more(fMaxWidth, aMaxSize.getX()))
            {
                fMaxWidth = aMaxSize.getX();
            }

            if(basegfx::fTools::less(fMinWidth, 1.0))
            {
                fMinWidth = 1.0;
            }

            aSize.setX(fMaxWidth);
        }

        if(bHgtGrow)
        {
            fMinHeight = GetMinTextFrameHeight();
            fMaxHeight = GetMaxTextFrameHeight();

            if(basegfx::fTools::equalZero(fMaxHeight) || basegfx::fTools::more(fMaxHeight, aMaxSize.getY()))
            {
                fMaxHeight = aMaxSize.getY();
            }

            if(basegfx::fTools::less(fMinHeight, 1.0))
            {
                fMinHeight = 1.0;
            }

            aSize.setY(fMaxHeight);
        }

        const basegfx::B2DVector aBorders(
            GetTextLeftDistance() + GetTextRightDistance(),
            GetTextUpperDistance() + GetTextLowerDistance());

        // substract orders
        aSize -= aBorders;

        // minimum size is 2.0
        aSize = basegfx::maximum(basegfx::B2DTuple(2.0, 2.0), aSize);

        if(bCheckAnimation && !IsInEditMode())
        {
            // do not wrap animated text
            const SdrTextAniKind eAniKind(GetTextAniKind());
            const SdrTextAniDirection eAniDir(GetTextAniDirection());
            const bool bScroll(SDRTEXTANI_SCROLL == eAniKind || SDRTEXTANI_ALTERNATE == eAniKind || SDRTEXTANI_SLIDE == eAniKind);
            const bool bHScroll(bScroll && (SDRTEXTANI_LEFT == eAniDir || SDRTEXTANI_RIGHT == eAniDir));
            const bool bVScroll(bScroll && (SDRTEXTANI_UP == eAniDir || SDRTEXTANI_DOWN == eAniDir));

            if(bHScroll)
            {
                aSize.setX(268435455.0);
            }

            if(bVScroll)
            {
                aSize.setY(268435455.0);
            }
        }

        if(IsTextEditActive())
        {
            GetTextEditOutliner()->SetMaxAutoPaperSize(Size(basegfx::fround(aSize.getX()), basegfx::fround(aSize.getY())));

            if(bWdtGrow)
            {
                const Size aSiz2(GetTextEditOutliner()->CalcTextSize());

                fWidth = aSiz2.Width() + 1; // lieber etwas Tolleranz

                if(bHgtGrow)
                {
                    fHeight = aSiz2.Height() + 1; // lieber etwas Tolleranz
                }
            }
            else
            {
                fHeight = GetTextEditOutliner()->GetTextHeight() + 1; // lieber etwas Tolleranz
            }
        }
        else
        {
            Outliner& rOutliner = ImpGetDrawOutliner();

            rOutliner.SetPaperSize(Size(basegfx::fround(aSize.getX()), basegfx::fround(aSize.getY())));
            rOutliner.SetUpdateMode(true);
            OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();

            if(pOutlinerParaObject)
            {
                rOutliner.SetText(*pOutlinerParaObject);
                rOutliner.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
            }

            if(bWdtGrow)
            {
                const Size aSiz2(rOutliner.CalcTextSize());

                fWidth = aSiz2.Width() + 1; // lieber etwas Tolleranz

                if(bHgtGrow)
                {
                    fHeight = aSiz2.Height() + 1; // lieber etwas Tolleranz
                }
            }
            else
            {
                fHeight = rOutliner.GetTextHeight() + 1; // lieber etwas Tolleranz
            }

            rOutliner.Clear();
        }

        // fMinWidth < fWidth < fMaxWidth
        fWidth = std::min(fMaxWidth, std::max(fWidth, fMinWidth));
        fWidth = std::max(1.0, fWidth += aBorders.getX()); // aBorders.getX() may be negative

        // fMinHeight < fHeight < fMaxHeight
        fHeight = std::min(fMaxHeight, std::max(fHeight, fMinHeight));
        fHeight = std::max(1.0, fHeight += aBorders.getY()); // aBorders.getY() may be negative

        // get grow sizes
        const double fWidthGrow(fWidth - aRetval.getWidth());
        const double fHeightGrow(fHeight - aRetval.getHeight());

        if(basegfx::fTools::equalZero(fWidthGrow))
        {
            bWdtGrow = false;
        }

        if(basegfx::fTools::equalZero(fHeightGrow))
        {
            bHgtGrow = false;
        }

        if(bWdtGrow || bHgtGrow)
        {
            if(bWdtGrow)
            {
                const SdrTextHorzAdjust eHAdj(GetTextHorizontalAdjust());

                if(SDRTEXTHORZADJUST_LEFT == eHAdj)
                {
                    aRetval = basegfx::B2DRange(aRetval.getMinX(), aRetval.getMinY(), aRetval.getMaxX() + fWidthGrow, aRetval.getMaxY());
                }
                else if(SDRTEXTHORZADJUST_RIGHT == eHAdj)
                {
                    aRetval = basegfx::B2DRange(aRetval.getMinX() - fWidthGrow, aRetval.getMinY(), aRetval.getMaxX(), aRetval.getMaxY());
                }
                else
                {
                    aRetval = basegfx::B2DRange(aRetval.getMinX() - (fWidthGrow * 0.5), aRetval.getMinY(), aRetval.getMaxX() + (fWidthGrow * 0.5), aRetval.getMaxY());
                }
            }

            if(bHgtGrow)
            {
                const SdrTextVertAdjust eVAdj(GetTextVerticalAdjust());

                if(SDRTEXTVERTADJUST_TOP == eVAdj)
                {
                    aRetval = basegfx::B2DRange(aRetval.getMinX(), aRetval.getMinY(), aRetval.getMaxX(), aRetval.getMaxY() + fHeightGrow);
                }
                else if(SDRTEXTVERTADJUST_BOTTOM == eVAdj)
                {
                    aRetval = basegfx::B2DRange(aRetval.getMinX(), aRetval.getMinY() - fHeightGrow, aRetval.getMaxX(), aRetval.getMaxY());
                }
                else
                {
                    aRetval = basegfx::B2DRange(aRetval.getMinX(), aRetval.getMinY() - (fHeightGrow * 0.5), aRetval.getMaxX(), aRetval.getMaxY() + (fHeightGrow * 0.5));
                }
            }

            if(!aOriginalMinimum.equal(aRetval.getMinimum()) && isRotatedOrSheared())
            {
                basegfx::B2DHomMatrix aCorrector(
                    basegfx::tools::createScaleTranslateB2DHomMatrix(
                        getSdrObjectScale(),
                        basegfx::absolute(getSdrObjectTranslate())));

                aCorrector.invert();
                aCorrector = getSdrObjectTransformation() * aCorrector;

                const basegfx::B2DPoint aCorrectedTopLeft(aCorrector * aRetval.getMinimum());

                aCorrector.identity();
                aCorrector.translate(aCorrectedTopLeft - aRetval.getMinimum());

                aRetval.transform(aCorrector);

                // TTTT: Check if the above solution works
                //
                //const sal_Int32 aOldRotation(sdr::legacy::GetRotateAngle(*this));
                //
                //if (aOldRotation)
                //{
                //    Point aD1(rR.TopLeft());
                //    aD1-=aOriginalMinimum;
                //    Point aD2(aD1);
                //    RotatePoint(aD2,Point(),sin(aOldRotation*nPi180), cos(aOldRotation*nPi180));
                //    aD2-=aD1;
                //    rR.Move(aD2.X(),aD2.Y());
                //}
            }
        }
    }

    return aRetval;
}

bool SdrTextObj::AdjustTextFrameWidthAndHeight(bool bHgt, bool bWdt)
{
    const basegfx::B2DRange aOldRange(getSdrObjectTranslate(), getSdrObjectTranslate() + basegfx::absolute(getSdrObjectScale()));
    const basegfx::B2DRange aNewRange(AdjustTextFrameWidthAndHeight(aOldRange, bHgt, bWdt));

    if(!aNewRange.equal(aOldRange))
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
        sdr::legacy::SetLogicRange(*this, aNewRange);

        if(dynamic_cast< SdrCaptionObj* >(this))
        {
            // mal wieder 'nen Hack
            ((SdrCaptionObj*)this)->ImpRecalcTail();
        }

        SetChanged();

        return true;
    }

    return false;
}

void SdrTextObj::ImpSetTextStyleSheetListeners()
{
    SfxStyleSheetBasePool* pStylePool = getSdrModelFromSdrObject().GetStyleSheetPool();

    if (pStylePool!=0)
    {
        Container aStyles(1024,64,64);
        OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();

        if(pOutlinerParaObject)
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

                    bool bFnd(false);
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

        while (nNum>0)
        {
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
            SfxStyleSheet* pStyle = dynamic_cast< SfxStyleSheet* >( pStyleBase);
            delete pName;

            if (pStyle!=0 && pStyle!=GetStyleSheet())
            {
                aStyles.Replace(pStyle,nNum);
            }
            else
            {
                aStyles.Remove(nNum);
            }
        }

        // jetzt alle ueberfluessigen StyleSheets entfernen
        nNum=GetBroadcasterCount();

        while (nNum>0)
        {
            nNum--;
            SfxBroadcaster* pBroadcast=GetBroadcasterJOE((sal_uInt16)nNum);
            SfxStyleSheet* pStyle = dynamic_cast< SfxStyleSheet* >( pBroadcast);
            if (pStyle!=0 && pStyle!=GetStyleSheet())
            {
                // Sonderbehandlung fuer den StyleSheet des Objekts
                if (aStyles.GetPos(pStyle)==CONTAINER_ENTRY_NOTFOUND)
                {
                    EndListening(*pStyle);
                }
            }
        }

        // und schliesslich alle in aStyles enthaltenen StyleSheets mit den vorhandenen Broadcastern mergen
        nNum=aStyles.Count();

        while (nNum>0)
        {
            nNum--;
            SfxStyleSheet* pStyle=(SfxStyleSheet*)aStyles.GetObject(nNum);
            // StartListening soll selbst nachsehen, ob hier nicht evtl. schon gehorcht wird
            StartListening(*pStyle,true);
        }
    }
}

void SdrTextObj::ResizeTextAttributes(const Fraction& xFact, const Fraction& yFact)
{
    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();

    if (pOutlinerParaObject!=0 && xFact.IsValid() && yFact.IsValid())
    {
        Fraction n100(100,1);
        sal_Int32 nX=long(xFact*n100);
        sal_Int32 nY=long(yFact*n100);

        if (nX<0)
            nX=-nX;

        if (nX<1)
            nX=1;

        if (nX>0xFFFF)
            nX=0xFFFF;

        if (nY<0)
            nY=-nY;

        if (nY<1)
            nY=1;

        if (nY>0xFFFF)
            nY=0xFFFF;

        if (nX!=100 || nY!=100)
        {
            // Rahmenattribute
            const SfxItemSet& rSet = GetObjectItemSet();
            const SvxCharScaleWidthItem& rOldWdt=(SvxCharScaleWidthItem&)rSet.Get(EE_CHAR_FONTWIDTH);
            const SvxFontHeightItem& rOldHgt=(SvxFontHeightItem&)rSet.Get(EE_CHAR_FONTHEIGHT);

            // erstmal die alten Werte holen
            sal_Int32 nRelWdt=rOldWdt.GetValue();
            sal_Int32 nAbsHgt=rOldHgt.GetHeight();
            sal_Int32 nRelHgt=rOldHgt.GetProp();

            // Relative Breite aendern
            nRelWdt*=nX;
            nRelWdt/=nY;

            // nicht negativ
            if (nRelWdt<0)
                nRelWdt=-nRelWdt;

            // und mind. 1%
            if (nRelWdt<=0)
                nRelWdt=1;

            if (nRelWdt>0xFFFF)
                nRelWdt=0xFFFF;

            // Absolute Hoehe aendern
            nAbsHgt*=nY;
            nAbsHgt/=100;

            // nicht negativ
            if (nAbsHgt<0)
                nAbsHgt=-nAbsHgt;

            // und mind. 1
            if (nAbsHgt<=0)
                nAbsHgt=1;

            if (nAbsHgt>0xFFFF)
                nAbsHgt=0xFFFF;

            // und nun attributieren
            SetObjectItem(SvxCharScaleWidthItem( (sal_uInt16) nRelWdt, EE_CHAR_FONTWIDTH));
            SetObjectItem(SvxFontHeightItem(nAbsHgt,(sal_uInt16)nRelHgt, EE_CHAR_FONTHEIGHT));
            // Zeichen- und Absatzattribute innerhalb des OutlinerParaObjects
            Outliner& rOutliner=ImpGetDrawOutliner();
            rOutliner.SetPaperSize(Size(LONG_MAX,LONG_MAX));
            rOutliner.SetText(*pOutlinerParaObject);
            rOutliner.DoStretchChars((sal_uInt16)nX,(sal_uInt16)nY);
            OutlinerParaObject* pNewPara=rOutliner.CreateParaObject();
            SetOutlinerParaObject(pNewPara);
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

            if( IsTextEditActive() || (pText == getActiveText()) )
                pOutliner = GetTextEditOutliner();

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

            if(!IsTextEditActive() || (pText != getActiveText()) )
            {
                const sal_uInt32 nParaCount = pOutliner->GetParagraphCount();
                OutlinerParaObject* pTemp = pOutliner->CreateParaObject(0, (sal_uInt16)nParaCount);
                pOutliner->Clear();
                SetOutlinerParaObjectForText(pTemp, pText);
            }
        }
    }
}

bool SdrTextObj::HasText() const
{
    if( IsTextEditActive() )
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

// eof
