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

#include <svx/svdotext.hxx>
#include <editeng/editdata.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svditext.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editstat.hxx>
#include <svl/itemset.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdtfchim.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/svdtrans.hxx>
#include <svx/sdrtexthelpers.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrTextObj::HasTextEdit() const
{
    // lt. Anweisung von MB duerfen gelinkte Textobjekte nun doch
    // geaendert werden (kein automatisches Reload)
    return true;
}

bool SdrTextObj::BegTextEdit(SdrOutliner& rOutl)
{
    if (pEdtOutl!=0) return sal_False; // Textedit laeuft evtl. schon an einer anderen View!
    pEdtOutl=&rOutl;

    // #101684#
    mbInEditMode = true;

    sal_uInt16 nOutlinerMode = OUTLINERMODE_OUTLINEOBJECT;
    if ( !IsOutlText() )
        nOutlinerMode = OUTLINERMODE_TEXTOBJECT;
    rOutl.Init( nOutlinerMode );
    rOutl.SetRefDevice( getSdrModelFromSdrObject().GetReferenceDevice() );

    SdrFitToSizeType eFit=GetFitToSize();
    bool bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
    bool bContourFrame=IsContourTextFrame();
    ImpSetTextEditParams();

    if (!bContourFrame) {
        sal_uIntPtr nStat=rOutl.GetControlWord();
        nStat|=EE_CNTRL_AUTOPAGESIZE;
        if (bFitToSize) nStat|=EE_CNTRL_STRETCHING; else nStat&=~EE_CNTRL_STRETCHING;
        rOutl.SetControlWord(nStat);
    }

    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
    if(pOutlinerParaObject!=0)
    {
        rOutl.SetText(*GetOutlinerParaObject());
        rOutl.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
    }

    // ggf. Rahmenattribute am 1. (neuen) Absatz des Outliners setzen
    if( !HasTextImpl( &rOutl ) )
    {
        // Outliner has no text so we must set some
        // empty text so the outliner initialise itself
        rOutl.SetText( String(), rOutl.GetParagraph( 0 ) );

        if(GetStyleSheet())
            rOutl.SetStyleSheet( 0, GetStyleSheet());

        // Beim setzen der harten Attribute an den ersten Absatz muss
        // der Parent pOutlAttr (=die Vorlage) temporaer entfernt
        // werden, da sonst bei SetParaAttribs() auch alle in diesem
        // Parent enthaltenen Items hart am Absatz attributiert werden.
        // -> BugID 22467
        const SfxItemSet& rSet = GetObjectItemSet();
        SfxItemSet aFilteredSet(*rSet.GetPool(), EE_ITEMS_START, EE_ITEMS_END);
        aFilteredSet.Put(rSet);
        rOutl.SetParaAttribs(0, aFilteredSet);
    }
    if (bFitToSize)
    {
        basegfx::B2DRange aAnchorRange;
        basegfx::B2DRange aTextRange;

        TakeTextRange(rOutl, aTextRange, aAnchorRange);
        ImpSetCharStretching(rOutl, aTextRange, aAnchorRange);
    }

    if(pOutlinerParaObject)
    {
        if(IsFontwork() || sdr::legacy::GetRotateAngle(*this))
        {
            const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
        }
    }

    rOutl.UpdateFields();
    rOutl.ClearModifyFlag();

    return sal_True;
}

void SdrTextObj::TakeTextEditArea(basegfx::B2DVector* pPaperMin, basegfx::B2DVector* pPaperMax, basegfx::B2DRange* pViewInit, basegfx::B2DRange* pViewMin) const
{
    const SdrFitToSizeType eFit(GetFitToSize());
    const bool bFitToSize(SDRTEXTFIT_PROPORTIONAL == eFit || SDRTEXTFIT_ALLLINES == eFit);
    basegfx::B2DVector aPaperMin;
    basegfx::B2DVector aPaperMax;

    // get TextRange without shear, rotate and mirror, just scaled
    // and centered in logic coordinates
    basegfx::B2DRange aViewInit(getScaledCenteredTextRange(*this));

    basegfx::B2DVector aAnkSiz(aViewInit.getRange());
    basegfx::B2DVector aMaxSiz(1000000.0, 1000000.0);

    if(!basegfx::fTools::equalZero(getSdrModelFromSdrObject().GetMaxObjectScale().getX()))
    {
        aMaxSiz.setX(getSdrModelFromSdrObject().GetMaxObjectScale().getX());
    }

    if(!basegfx::fTools::equalZero(getSdrModelFromSdrObject().GetMaxObjectScale().getY()))
    {
        aMaxSiz.setY(getSdrModelFromSdrObject().GetMaxObjectScale().getY());
    }

    // #106879#
    // Done earlier since used in else tree below
    SdrTextHorzAdjust eHAdj(GetTextHorizontalAdjust());
    SdrTextVertAdjust eVAdj(GetTextVerticalAdjust());

    if(IsTextFrame())
    {
        double fMinWdt(std::max(1.0, (double)GetMinTextFrameWidth()));
        double fMinHgt(std::max(1.0, (double)GetMinTextFrameHeight()));
        double fMaxWdt(GetMaxTextFrameWidth());
        double fMaxHgt(GetMaxTextFrameHeight());

        if(!bFitToSize)
        {
            if(basegfx::fTools::equalZero(fMaxWdt) || basegfx::fTools::more(fMaxWdt, aMaxSiz.getX()))
            {
                fMaxWdt = aMaxSiz.getX();
            }

            if(basegfx::fTools::equalZero(fMaxHgt) || basegfx::fTools::more(fMaxHgt, aMaxSiz.getY()))
            {
                fMaxHgt = aMaxSiz.getY();
            }

            if(!IsAutoGrowWidth())
            {
                fMaxWdt = aAnkSiz.getX();
                fMinWdt = fMaxWdt;
            }

            if(!IsAutoGrowHeight())
            {
                fMaxHgt = aAnkSiz.getY();
                fMinHgt = fMaxHgt;
            }

            const SdrTextAniKind eAniKind(GetTextAniKind());
            // #101684#
            bool bInEditMode = IsInEditMode();

            if(!bInEditMode && (SDRTEXTANI_SCROLL == eAniKind || SDRTEXTANI_ALTERNATE == eAniKind || SDRTEXTANI_SLIDE == eAniKind))
            {
                const SdrTextAniDirection eAniDirection(GetTextAniDirection());

                // Grenzenlose Papiergroesse fuer Laufschrift
                if(SDRTEXTANI_LEFT == eAniDirection || SDRTEXTANI_RIGHT == eAniDirection)
                {
                    fMaxWdt = 1000000.0;
                }
                else if(SDRTEXTANI_UP == eAniDirection || SDRTEXTANI_DOWN == eAniDirection)
                {
                    fMaxHgt = 1000000.0;
                }
            }

            // #119885# Do not limit/force height to geometrical frame (vice versa for vertical writing)
            if(IsVerticalWriting())
            {
                fMaxWdt = 1000000.0;
            }
            else
            {
                fMaxHgt = 1000000.0;
            }

            aPaperMax.setX(fMaxWdt);
            aPaperMax.setY(fMaxHgt);
        }
        else
        {
            aPaperMax=aMaxSiz;
        }

        aPaperMin.setX(fMinWdt);
        aPaperMin.setY(fMinHgt);
    }
    else
    {
        // #106879#
        // aPaperMin needs to be set to object's size if full width is activated
        // for hor or ver writing respectively
        if((SDRTEXTHORZADJUST_BLOCK == eHAdj && !IsVerticalWriting())
            || (SDRTEXTVERTADJUST_BLOCK == eVAdj && IsVerticalWriting()))
        {
            aPaperMin = aAnkSiz;
        }

        aPaperMax=aMaxSiz;
    }

    if(pViewMin)
    {
        *pViewMin=aViewInit;
        const double fXFree(aAnkSiz.getX() - aPaperMin.getX());

        if(SDRTEXTHORZADJUST_LEFT == eHAdj)
        {
            *pViewMin = basegfx::B2DRange(
                pViewMin->getMinX(),
                pViewMin->getMinY(),
                pViewMin->getMaxX() - fXFree,
                pViewMin->getMaxY());
        }
        else if(SDRTEXTHORZADJUST_RIGHT == eHAdj)
        {
            *pViewMin = basegfx::B2DRange(
                pViewMin->getMinX() + fXFree,
                pViewMin->getMinY(),
                pViewMin->getMaxX(),
                pViewMin->getMaxY());
        }
        else
        {
            const double fNewMinX(pViewMin->getMinX() + (fXFree * 0.5));
            *pViewMin = basegfx::B2DRange(
                fNewMinX,
                pViewMin->getMinY(),
                fNewMinX + aPaperMin.getX(),
                pViewMin->getMaxY());
        }

        const double fYFree(aAnkSiz.getY() - aPaperMin.getY());

        if(SDRTEXTVERTADJUST_TOP == eVAdj)
        {
            *pViewMin = basegfx::B2DRange(
                pViewMin->getMinX(),
                pViewMin->getMinY(),
                pViewMin->getMaxX(),
                pViewMin->getMaxY() - fYFree);
        }
        else if(SDRTEXTVERTADJUST_BOTTOM == eVAdj)
        {
            *pViewMin = basegfx::B2DRange(
                pViewMin->getMinX(),
                pViewMin->getMinY() + fYFree,
                pViewMin->getMaxX(),
                pViewMin->getMaxY());
        }
        else
        {
            const double fNewMinY(pViewMin->getMinY() + (fYFree * 0.5));
            *pViewMin = basegfx::B2DRange(
                pViewMin->getMinX(),
                fNewMinY,
                pViewMin->getMaxX(),
                fNewMinY + aPaperMin.getY());
        }
    }

    // Die PaperSize soll in den meisten Faellen von selbst wachsen
    // #89459#
    if(IsVerticalWriting())
    {
        aPaperMin.setX(0.0);
    }
    else
    {
        // #33102#
        aPaperMin.setY(0.0);
    }

    if(SDRTEXTHORZADJUST_BLOCK != eHAdj || bFitToSize)
    {
        aPaperMin.setX(0.0);
    }

    // #103516# For complete ver adjust support, set paper min height to 0, here.
    if(SDRTEXTVERTADJUST_BLOCK != eVAdj || bFitToSize)
    {
        aPaperMin.setY(0.0);
    }

    if(pPaperMin)
    {
        *pPaperMin = aPaperMin;
    }

    if(pPaperMax)
    {
        *pPaperMax = aPaperMax;
    }

    if(pViewInit)
    {
        *pViewInit = aViewInit;
    }

}

void SdrTextObj::EndTextEdit(SdrOutliner& rOutl)
{
    if(rOutl.IsModified())
    {
        OutlinerParaObject* pNewText = 0;

        if(HasTextImpl( &rOutl ) )
        {
            // Damit der grauen Feldhintergrund wieder verschwindet
            rOutl.UpdateFields();

            sal_uInt16 nParaAnz = static_cast< sal_uInt16 >( rOutl.GetParagraphCount() );
            pNewText = rOutl.CreateParaObject( 0, nParaAnz );
        }

        // need to end edit mode early since SetOutlinerParaObject already
        // uses getObjectRange() which needs to take the text into account
        // to work correct
        mbInEditMode = false;
        SetOutlinerParaObject(pNewText);
    }

    pEdtOutl = 0;
    rOutl.Clear();
    sal_uInt32 nStat = rOutl.GetControlWord();
    nStat &= ~EE_CNTRL_AUTOPAGESIZE;
    rOutl.SetControlWord(nStat);

    // #101684#
    mbInEditMode = false;
}

sal_uInt16 SdrTextObj::GetOutlinerViewAnchorMode() const
{
    SdrTextHorzAdjust eH=GetTextHorizontalAdjust();
    SdrTextVertAdjust eV=GetTextVerticalAdjust();
    EVAnchorMode eRet=ANCHOR_TOP_LEFT;
    if (IsContourTextFrame()) return (sal_uInt16)eRet;
    if (eH==SDRTEXTHORZADJUST_LEFT) {
        if (eV==SDRTEXTVERTADJUST_TOP) {
            eRet=ANCHOR_TOP_LEFT;
        } else if (eV==SDRTEXTVERTADJUST_BOTTOM) {
            eRet=ANCHOR_BOTTOM_LEFT;
        } else {
            eRet=ANCHOR_VCENTER_LEFT;
        }
    } else if (eH==SDRTEXTHORZADJUST_RIGHT) {
        if (eV==SDRTEXTVERTADJUST_TOP) {
            eRet=ANCHOR_TOP_RIGHT;
        } else if (eV==SDRTEXTVERTADJUST_BOTTOM) {
            eRet=ANCHOR_BOTTOM_RIGHT;
        } else {
            eRet=ANCHOR_VCENTER_RIGHT;
        }
    } else {
        if (eV==SDRTEXTVERTADJUST_TOP) {
            eRet=ANCHOR_TOP_HCENTER;
        } else if (eV==SDRTEXTVERTADJUST_BOTTOM) {
            eRet=ANCHOR_BOTTOM_HCENTER;
        } else {
            eRet=ANCHOR_VCENTER_HCENTER;
        }
    }
    return (sal_uInt16)eRet;
}

void SdrTextObj::ImpSetTextEditParams() const
{
    if(pEdtOutl)
    {
        bool bUpdMerk(pEdtOutl->GetUpdateMode());

        if(bUpdMerk)
        {
            pEdtOutl->SetUpdateMode(false);
        }

        basegfx::B2DVector aPaperMin;
        basegfx::B2DVector aPaperMax;
//      basegfx::B2DRange aEditArea; // TTTT: aEditArea not used, check if calculation changes when leaving out

//      TakeTextEditArea(&aPaperMin, &aPaperMax, &aEditArea, 0);
        TakeTextEditArea(&aPaperMin, &aPaperMax, 0, 0);

        const Size aOldPaperMin(basegfx::fround(aPaperMin.getX()), basegfx::fround(aPaperMin.getY()));
        const Size aOldPaperMax(basegfx::fround(aPaperMax.getX()), basegfx::fround(aPaperMax.getY()));

        pEdtOutl->SetMinAutoPaperSize(aOldPaperMin);
        pEdtOutl->SetMaxAutoPaperSize(aOldPaperMax);
        pEdtOutl->SetPaperSize(Size());

        if(IsContourTextFrame())
        {
            basegfx::B2DPolyPolygon aContourOutline(getAlignedTextContourPolyPolygon(*this));
            aContourOutline.transform(basegfx::tools::createTranslateB2DHomMatrix(-aContourOutline.getB2DRange().getMinimum()));
            pEdtOutl->SetPolygon(aContourOutline);
        }

        if(bUpdMerk)
        {
            pEdtOutl->ClearPolygon();
            pEdtOutl->SetUpdateMode(true);
        }
    }
}

// eof
