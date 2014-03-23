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

#include <svx/svdomeas.hxx>
#include <math.h>
#include <editeng/editdata.hxx>
#include <svx/svditext.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdattrx.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdview.hxx>
#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdstr.hrc"    // Objektname
#include <svl/style.hxx>
#include <svl/smplhint.hxx>
#include <editeng/eeitem.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnedcit.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editobj.hxx>
#include <editeng/measfld.hxx>
#include <editeng/flditem.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdpage.hxx>
#include <unotools/syslocale.hxx>
#include <svx/sdr/properties/measureproperties.hxx>
#include <svx/sdr/contact/viewcontactofsdrmeasureobj.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrMeasureObjGeoData::SdrMeasureObjGeoData() {}
SdrMeasureObjGeoData::~SdrMeasureObjGeoData() {}

void SdrMeasureObj::SetTextDirty()
{
    bTextDirty = true;
    SetTextSizeDirty();
    ActionChanged();
}

void SdrMeasureObj::TakeRepresentation( XubString& rStr, SdrMeasureFieldKind eMeasureFieldKind ) const
{
    rStr.Erase();
    Fraction aMeasureScale(1, 1);
    sal_Bool bTextRota90(sal_False);
    sal_Bool bShowUnit(sal_False);
    FieldUnit eMeasureUnit(FUNIT_NONE);
    FieldUnit eModUIUnit(FUNIT_NONE);

    const SfxItemSet& rSet = GetMergedItemSet();
    bTextRota90 = ((SdrYesNoItem&)rSet.Get(SDRATTR_MEASURETEXTROTA90)).GetValue();
    eMeasureUnit = ((SdrMeasureUnitItem&)rSet.Get(SDRATTR_MEASUREUNIT)).GetValue();
    aMeasureScale = ((SdrScaleItem&)rSet.Get(SDRATTR_MEASURESCALE)).GetValue();
    bShowUnit = ((SdrYesNoItem&)rSet.Get(SDRATTR_MEASURESHOWUNIT)).GetValue();
    sal_Int16 nNumDigits = ((SfxInt16Item&)rSet.Get(SDRATTR_MEASUREDECIMALPLACES)).GetValue();

    switch(eMeasureFieldKind)
    {
        case SDRMEASUREFIELD_VALUE:
        {
            eModUIUnit = getSdrModelFromSdrObject().GetUIUnit();

            if(eMeasureUnit == FUNIT_NONE)
                eMeasureUnit = eModUIUnit;

            sal_Int32 nLen(GetLen(aPt2 - aPt1));
            Fraction aFact(1,1);

            if(eMeasureUnit != eModUIUnit)
            {
                // Zur Umrechnung der Einheiten
                aFact *= GetMapFactor(eModUIUnit, eMeasureUnit).X();
            }

            if(aMeasureScale.GetNumerator() != aMeasureScale.GetDenominator())
            {
                aFact *= aMeasureScale;
            }

            if(aFact.GetNumerator() != aFact.GetDenominator())
            {
                // Scaling ueber BigInt, um Ueberlaeufe zu vermeiden
                nLen = BigMulDiv(nLen, aFact.GetNumerator(), aFact.GetDenominator());
            }

            getSdrModelFromSdrObject().TakeMetricStr(nLen, rStr, true, nNumDigits);

            if(!aFact.IsValid())
            {
                rStr = String();
                rStr += sal_Unicode('?');
            }

            sal_Unicode cDec(SvtSysLocale().GetLocaleData().getNumDecimalSep().GetChar(0));

            if(rStr.Search(cDec) != STRING_NOTFOUND)
            {
                xub_StrLen nLen2(rStr.Len() - 1);

                while(rStr.GetChar(nLen2) == sal_Unicode('0'))
                {
                    rStr.Erase(nLen2);
                    nLen2--;
                }

                if(rStr.GetChar(nLen2) == cDec)
                {
                    rStr.Erase(nLen2);
                    nLen2--;
                }

                if(!rStr.Len())
                    rStr += sal_Unicode('0');
            }

            break;
        }
        case SDRMEASUREFIELD_UNIT:
        {
            if(bShowUnit)
            {
                eModUIUnit = getSdrModelFromSdrObject().GetUIUnit();

                if(FUNIT_NONE == eMeasureUnit)
                {
                    eMeasureUnit = eModUIUnit;
                }

                if(bShowUnit)
                {
                    getSdrModelFromSdrObject().TakeUnitStr(eMeasureUnit, rStr);
                }
            }

            break;
        }
        case SDRMEASUREFIELD_ROTA90BLANCS:
        {
            if(bTextRota90)
            {
                rStr = String();
                rStr += sal_Unicode(' ');
            }

            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrMeasureObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::MeasureProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrMeasureObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrMeasureObj(*this);
}

//////////////////////////////////////////////////////////////////////////////

SdrMeasureObj::SdrMeasureObj(
    SdrModel& rSdrModel,
    const Point& rPt1,
    const Point& rPt2)
:   SdrTextObj(rSdrModel),
    aPt1(rPt1),
    aPt2(rPt2),
    bTextDirty(false)
{
}

SdrMeasureObj::~SdrMeasureObj()
{
}

void SdrMeasureObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SdrMeasureObj* pSource = dynamic_cast< const SdrMeasureObj* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrTextObj::copyDataFromSdrObject(rSource);

            // copy local data
            aPt1 = pSource->aPt1;
            aPt2 = pSource->aPt2;
            bTextDirty = pSource->bTextDirty;
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* SdrMeasureObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    SdrMeasureObj* pClone = new SdrMeasureObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

bool SdrMeasureObj::DoesSupportTextIndentingOnLineWidthChange() const
{
    return false;
}

void SdrMeasureObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.mbSelectAllowed = true;
    rInfo.mbMoveAllowed = true;
    rInfo.mbResizeFreeAllowed = true;
    rInfo.mbResizePropAllowed = true;
    rInfo.mbRotateFreeAllowed = true;
    rInfo.mbRotate90Allowed = true;
    rInfo.mbMirrorFreeAllowed = true;
    rInfo.mbMirror45Allowed = true;
    rInfo.mbMirror90Allowed = true;
    rInfo.mbTransparenceAllowed = false;
    rInfo.mbGradientAllowed = false;
    rInfo.mbShearAllowed = true;
    rInfo.mbEdgeRadiusAllowed = false;
    rInfo.mbNoOrthoDesired = true;
    rInfo.mbNoContortion = false;
    rInfo.mbCanConvToPath = false;
    rInfo.mbCanConvToPoly = true;
    rInfo.mbCanConvToPathLineToArea = false;
    rInfo.mbCanConvToPolyLineToArea = false;
    rInfo.mbCanConvToContour = (rInfo.mbCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrMeasureObj::GetObjIdentifier() const
{
    return (sal_uInt16)OBJ_MEASURE;
}

struct ImpMeasureRec : public SdrDragStatUserData
{
    Point                       aPt1;
    Point                       aPt2;
    SdrMeasureKind              eKind;
    SdrMeasureTextHPos          eWantTextHPos;
    SdrMeasureTextVPos          eWantTextVPos;
    long                        nLineDist;
    long                        nHelplineOverhang;
    long                        nHelplineDist;
    long                        nHelpline1Len;
    long                        nHelpline2Len;
    bool                    bBelowRefEdge;
    bool                    bTextRota90;
    bool                    bTextUpsideDown;
    long                        nMeasureOverhang;
    FieldUnit                   eMeasureUnit;
    Fraction                    aMeasureScale;
    bool                    bShowUnit;
    String                      aFormatString;
    bool                    bTextAutoAngle;
    long                        nTextAutoAngleView;
    bool                    bTextIsFixedAngle;
    long                        nTextFixedAngle;
};

struct ImpLineRec
{
    Point                       aP1;
    Point                       aP2;
};

struct ImpMeasurePoly
{
    ImpLineRec                  aMainline1; // die mit dem 1. Pfeil
    ImpLineRec                  aMainline2; // die mit dem 2. Pfeil
    ImpLineRec                  aMainline3; // die dazwischen
    ImpLineRec                  aHelpline1;
    ImpLineRec                  aHelpline2;
    Rectangle                   aTextRect;
    Size                        aTextSize;
    long                        nLineLen;
    long                        nLineWink;
    long                        nTextWink;
    long                        nHlpWink;
    double                      nLineSin;
    double                      nLineCos;
    double                      nHlpSin;
    double                      nHlpCos;
    sal_uInt16                      nMainlineAnz;
    SdrMeasureTextHPos          eUsedTextHPos;
    SdrMeasureTextVPos          eUsedTextVPos;
    long                        nLineWdt2;  // Halbe Strichstaerke
    long                        nArrow1Len; // Laenge des 1. Pfeils. Bei Center nur die Haelfte
    long                        nArrow2Len; // Laenge des 2. Pfeils. Bei Center nur die Haelfte
    long                        nArrow1Wdt; // Breite des 1. Pfeils
    long                        nArrow2Wdt; // Breite des 2. Pfeils
    long                        nShortLineLen; // Linienlaenge, wenn PfeileAussen
    bool                    bArrow1Center; // Pfeil 1 zentriert?
    bool                    bArrow2Center; // Pfeil 2 zentriert?
    bool                    bAutoUpsideDown; // UpsideDown durch Automatik
    bool                    bPfeileAussen;
    bool                    bBreakedLine;
};

void SdrMeasureObj::ImpTakeAttr(ImpMeasureRec& rRec) const
{
    rRec.aPt1 = aPt1;
    rRec.aPt2 = aPt2;

    const SfxItemSet& rSet = GetObjectItemSet();
    rRec.eKind            =((SdrMeasureKindItem&            )rSet.Get(SDRATTR_MEASUREKIND            )).GetValue();
    rRec.eWantTextHPos    =((SdrMeasureTextHPosItem&        )rSet.Get(SDRATTR_MEASURETEXTHPOS        )).GetValue();
    rRec.eWantTextVPos    =((SdrMeasureTextVPosItem&        )rSet.Get(SDRATTR_MEASURETEXTVPOS        )).GetValue();
    rRec.nLineDist         = ((SdrMetricItem&         )rSet.Get(SDRATTR_MEASURELINEDIST         )).GetValue();
    rRec.nHelplineOverhang = ((SdrMetricItem&         )rSet.Get(SDRATTR_MEASUREHELPLINEOVERHANG )).GetValue();
    rRec.nHelplineDist     = ((SdrMetricItem&         )rSet.Get(SDRATTR_MEASUREHELPLINEDIST     )).GetValue();
    rRec.nHelpline1Len     = ((SdrMetricItem&         )rSet.Get(SDRATTR_MEASUREHELPLINE1LEN     )).GetValue();
    rRec.nHelpline2Len     = ((SdrMetricItem&         )rSet.Get(SDRATTR_MEASUREHELPLINE2LEN     )).GetValue();
    rRec.bBelowRefEdge     = ((SdrYesNoItem&          )rSet.Get(SDRATTR_MEASUREBELOWREFEDGE     )).GetValue();
    rRec.bTextRota90       = ((SdrYesNoItem&          )rSet.Get(SDRATTR_MEASURETEXTROTA90       )).GetValue();
    rRec.bTextUpsideDown   = ((SdrYesNoItem&          )rSet.Get(SDRATTR_MEASURETEXTUPSIDEDOWN   )).GetValue();
    rRec.nMeasureOverhang  = ((SdrMetricItem&         )rSet.Get(SDRATTR_MEASUREOVERHANG         )).GetValue();
    rRec.eMeasureUnit     =((SdrMeasureUnitItem&            )rSet.Get(SDRATTR_MEASUREUNIT            )).GetValue();
    rRec.aMeasureScale     = ((SdrScaleItem&          )rSet.Get(SDRATTR_MEASURESCALE            )).GetValue();
    rRec.bShowUnit         = ((SdrYesNoItem&          )rSet.Get(SDRATTR_MEASURESHOWUNIT         )).GetValue();
    rRec.aFormatString     = ((SfxStringItem&         )rSet.Get(SDRATTR_MEASUREFORMATSTRING     )).GetValue();
    rRec.bTextAutoAngle    = ((SdrYesNoItem&          )rSet.Get(SDRATTR_MEASURETEXTAUTOANGLE    )).GetValue();
    rRec.nTextAutoAngleView= ((SdrAngleItem&          )rSet.Get(SDRATTR_MEASURETEXTAUTOANGLEVIEW)).GetValue();
    rRec.bTextIsFixedAngle = ((SdrYesNoItem&          )rSet.Get(SDRATTR_MEASURETEXTISFIXEDANGLE )).GetValue();
    rRec.nTextFixedAngle   = ((SdrAngleItem&          )rSet.Get(SDRATTR_MEASURETEXTFIXEDANGLE   )).GetValue();
}

long impGetLineStartEndDistance(const basegfx::B2DPolyPolygon& rPolyPolygon, long nNewWidth, bool bCenter)
{
    const basegfx::B2DRange aPolygonRange(rPolyPolygon.getB2DRange());
    const double fOldWidth(aPolygonRange.getWidth() > 1.0 ? aPolygonRange.getWidth() : 1.0);
    const double fScale((double)nNewWidth / fOldWidth);
    long nHeight(basegfx::fround(aPolygonRange.getHeight() * fScale));

    if(bCenter)
    {
        nHeight /= 2L;
    }

    return nHeight;
}

void SdrMeasureObj::ImpCalcGeometrics(const ImpMeasureRec& rRec, ImpMeasurePoly& rPol) const
{
    Point aP1(rRec.aPt1);
    Point aP2(rRec.aPt2);
    Point aDelt(aP2); aDelt-=aP1;

    rPol.aTextSize=GetTextSize();
    rPol.nLineLen=GetLen(aDelt);

    rPol.nLineWdt2=0;
    long nArrow1Len=0; bool bArrow1Center=false;
    long nArrow2Len=0; bool bArrow2Center=false;
    long nArrow1Wdt=0;
    long nArrow2Wdt=0;
    rPol.nArrow1Wdt=0;
    rPol.nArrow2Wdt=0;
    long nArrowNeed=0;
    long nShortLen=0;
    bool bPfeileAussen=sal_False;

    const SfxItemSet& rSet = GetObjectItemSet();
    sal_Int32 nLineWdt = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue(); // Strichstaerke
    rPol.nLineWdt2 = (nLineWdt + 1) / 2;

    nArrow1Wdt = ((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
    if(nArrow1Wdt < 0)
        nArrow1Wdt = -nLineWdt * nArrow1Wdt / 100; // <0 = relativ

    nArrow2Wdt = ((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();
    if(nArrow2Wdt < 0)
        nArrow2Wdt = -nLineWdt * nArrow2Wdt / 100; // <0 = relativ

    basegfx::B2DPolyPolygon aPol1(((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetLineStartValue());
    basegfx::B2DPolyPolygon aPol2(((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetLineEndValue());
    bArrow1Center = ((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
    bArrow2Center = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
    nArrow1Len = impGetLineStartEndDistance(aPol1, nArrow1Wdt, bArrow1Center) - 1;
    nArrow2Len = impGetLineStartEndDistance(aPol2, nArrow2Wdt, bArrow2Center) - 1;

    // nArrowLen ist bei bCenter bereits halbiert
    // Bei 2 Pfeilen a 4mm ist unter 10mm Schluss.
    nArrowNeed=nArrow1Len+nArrow2Len+(nArrow1Wdt+nArrow2Wdt)/2;
    if (rPol.nLineLen<nArrowNeed) bPfeileAussen=sal_True;
    nShortLen=(nArrow1Len+nArrow1Wdt + nArrow2Len+nArrow2Wdt) /2;

    rPol.eUsedTextHPos=rRec.eWantTextHPos;
    rPol.eUsedTextVPos=rRec.eWantTextVPos;
    if (rPol.eUsedTextVPos==SDRMEASURE_TEXTVAUTO) rPol.eUsedTextVPos=SDRMEASURE_ABOVE;
    bool bBrkLine=rPol.eUsedTextVPos==SDRMEASURETEXT_BREAKEDLINE;
    if (rPol.eUsedTextVPos==SDRMEASURETEXT_VERTICALCENTERED)
    {
        OutlinerParaObject* pOutlinerParaObject = SdrTextObj::GetOutlinerParaObject();
        if (pOutlinerParaObject!=NULL && pOutlinerParaObject->GetTextObject().GetParagraphCount()==1)
        {
            bBrkLine=sal_True; // Unterbrochene Linie, wenn nur 1 Absatz.
        }
    }
    rPol.bBreakedLine=bBrkLine;
    if (rPol.eUsedTextHPos==SDRMEASURE_TEXTHAUTO) { // bei zu breitem Text diesen eventuell nach aussen schieben
        bool bOutside=sal_False;
        long nNeedSiz=!rRec.bTextRota90 ? rPol.aTextSize.Width() : rPol.aTextSize.Height();
        if (nNeedSiz>rPol.nLineLen) bOutside=sal_True; // Text passt nicht in die Mitte
        if (bBrkLine) {
            if (nNeedSiz+nArrowNeed>rPol.nLineLen) bPfeileAussen=sal_True; // Text passt in die Mitte, wenn die Pfeile nach aussen kommen
        } else {
            long nSmallNeed=nArrow1Len+nArrow2Len+(nArrow1Wdt+nArrow2Wdt)/2/4;
            if (nNeedSiz+nSmallNeed>rPol.nLineLen) bPfeileAussen=sal_True; // Text passt in die Mitte, wenn die Pfeile nach aussen kommen
        }
        rPol.eUsedTextHPos=bOutside ? SDRMEASURE_TEXTLEFTOUTSIDE : SDRMEASURE_TEXTINSIDE;
    }
    if (rPol.eUsedTextHPos!=SDRMEASURE_TEXTINSIDE) bPfeileAussen=sal_True;
    rPol.nArrow1Wdt=nArrow1Wdt;
    rPol.nArrow2Wdt=nArrow2Wdt;
    rPol.nShortLineLen=nShortLen;
    rPol.bPfeileAussen=bPfeileAussen;
    rPol.nArrow1Len=nArrow1Len;
    rPol.bArrow1Center=bArrow1Center;
    rPol.nArrow2Len=nArrow2Len;
    rPol.bArrow2Center=bArrow2Center;

    rPol.nLineWink=GetAngle(aDelt);
    double a=rPol.nLineWink*nPi180;
    double nLineSin=sin(a);
    double nLineCos=cos(a);
    rPol.nLineSin=nLineSin;
    rPol.nLineCos=nLineCos;

    rPol.nTextWink=rPol.nLineWink;
    if (rRec.bTextRota90) rPol.nTextWink+=9000;

    rPol.bAutoUpsideDown=sal_False;
    if (rRec.bTextAutoAngle) {
        long nTmpWink=NormAngle360(rPol.nTextWink-rRec.nTextAutoAngleView);
        if (nTmpWink>=18000) {
            rPol.nTextWink+=18000;
            rPol.bAutoUpsideDown=sal_True;
        }
    }

    if (rRec.bTextUpsideDown) rPol.nTextWink+=18000;
    rPol.nTextWink=NormAngle360(rPol.nTextWink);
    rPol.nHlpWink=rPol.nLineWink+9000;
    if (rRec.bBelowRefEdge) rPol.nHlpWink+=18000;
    rPol.nHlpWink=NormAngle360(rPol.nHlpWink);
    double nHlpSin=nLineCos;
    double nHlpCos=-nLineSin;
    if (rRec.bBelowRefEdge) {
        nHlpSin=-nHlpSin;
        nHlpCos=-nHlpCos;
    }
    rPol.nHlpSin=nHlpSin;
    rPol.nHlpCos=nHlpCos;

    long nLineDist=rRec.nLineDist;
    long nOverhang=rRec.nHelplineOverhang;
    long nHelplineDist=rRec.nHelplineDist;

    long dx= Round(nLineDist*nHlpCos);
    long dy=-Round(nLineDist*nHlpSin);
    long dxh1a= Round((nHelplineDist-rRec.nHelpline1Len)*nHlpCos);
    long dyh1a=-Round((nHelplineDist-rRec.nHelpline1Len)*nHlpSin);
    long dxh1b= Round((nHelplineDist-rRec.nHelpline2Len)*nHlpCos);
    long dyh1b=-Round((nHelplineDist-rRec.nHelpline2Len)*nHlpSin);
    long dxh2= Round((nLineDist+nOverhang)*nHlpCos);
    long dyh2=-Round((nLineDist+nOverhang)*nHlpSin);

    // Masshilfslinie 1
    rPol.aHelpline1.aP1=Point(aP1.X()+dxh1a,aP1.Y()+dyh1a);
    rPol.aHelpline1.aP2=Point(aP1.X()+dxh2,aP1.Y()+dyh2);

    // Masshilfslinie 2
    rPol.aHelpline2.aP1=Point(aP2.X()+dxh1b,aP2.Y()+dyh1b);
    rPol.aHelpline2.aP2=Point(aP2.X()+dxh2,aP2.Y()+dyh2);

    // Masslinie(n)
    Point aMainlinePt1(aP1.X()+dx,aP1.Y()+dy);
    Point aMainlinePt2(aP2.X()+dx,aP2.Y()+dy);
    if (!bPfeileAussen) {
        rPol.aMainline1.aP1=aMainlinePt1;
        rPol.aMainline1.aP2=aMainlinePt2;
        rPol.aMainline2=rPol.aMainline1;
        rPol.aMainline3=rPol.aMainline1;
        rPol.nMainlineAnz=1;
        if (bBrkLine) {
            long nNeedSiz=!rRec.bTextRota90 ? rPol.aTextSize.Width() : rPol.aTextSize.Height();
            long nHalfLen=(rPol.nLineLen-nNeedSiz-nArrow1Wdt/4-nArrow2Wdt/4) /2;
            rPol.nMainlineAnz=2;
            rPol.aMainline1.aP2=aMainlinePt1;
            rPol.aMainline1.aP2.X()+=nHalfLen;
            RotatePoint(rPol.aMainline1.aP2,rPol.aMainline1.aP1,nLineSin,nLineCos);
            rPol.aMainline2.aP1=aMainlinePt2;
            rPol.aMainline2.aP1.X()-=nHalfLen;
            RotatePoint(rPol.aMainline2.aP1,rPol.aMainline2.aP2,nLineSin,nLineCos);
        }
    } else {
        long nLen1=nShortLen; // Pfeilbreite als Linienlaenge ausserhalb des Pfeils
        long nLen2=nShortLen;
        long nTextWdt=rRec.bTextRota90 ? rPol.aTextSize.Height() : rPol.aTextSize.Width();
        if (!bBrkLine) {
            if (rPol.eUsedTextHPos==SDRMEASURE_TEXTLEFTOUTSIDE) nLen1=nArrow1Len+nTextWdt;
            if (rPol.eUsedTextHPos==SDRMEASURE_TEXTRIGHTOUTSIDE) nLen2=nArrow2Len+nTextWdt;
        }
        rPol.aMainline1.aP1=aMainlinePt1;
        rPol.aMainline1.aP2=aMainlinePt1; rPol.aMainline1.aP2.X()-=nLen1; RotatePoint(rPol.aMainline1.aP2,aMainlinePt1,nLineSin,nLineCos);
        rPol.aMainline2.aP1=aMainlinePt2; rPol.aMainline2.aP1.X()+=nLen2; RotatePoint(rPol.aMainline2.aP1,aMainlinePt2,nLineSin,nLineCos);
        rPol.aMainline2.aP2=aMainlinePt2;
        rPol.aMainline3.aP1=aMainlinePt1;
        rPol.aMainline3.aP2=aMainlinePt2;
        rPol.nMainlineAnz=3;
        if (bBrkLine && rPol.eUsedTextHPos==SDRMEASURE_TEXTINSIDE) rPol.nMainlineAnz=2;
    }
}

basegfx::B2DPolyPolygon SdrMeasureObj::ImpCalcXPoly(const ImpMeasurePoly& rPol) const
{
    basegfx::B2DPolyPolygon aRetval;
    basegfx::B2DPolygon aPartPolyA;
    aPartPolyA.append(basegfx::B2DPoint(rPol.aMainline1.aP1.X(), rPol.aMainline1.aP1.Y()));
    aPartPolyA.append(basegfx::B2DPoint(rPol.aMainline1.aP2.X(), rPol.aMainline1.aP2.Y()));
    aRetval.append(aPartPolyA);

    if(rPol.nMainlineAnz > 1)
    {
        aPartPolyA.clear();
        aPartPolyA.append(basegfx::B2DPoint(rPol.aMainline2.aP1.X(), rPol.aMainline2.aP1.Y()));
        aPartPolyA.append(basegfx::B2DPoint(rPol.aMainline2.aP2.X(), rPol.aMainline2.aP2.Y()));
        aRetval.append(aPartPolyA);
    }

    if(rPol.nMainlineAnz > 2)
    {
        aPartPolyA.clear();
        aPartPolyA.append(basegfx::B2DPoint(rPol.aMainline3.aP1.X(), rPol.aMainline3.aP1.Y()));
        aPartPolyA.append(basegfx::B2DPoint(rPol.aMainline3.aP2.X(), rPol.aMainline3.aP2.Y()));
        aRetval.append(aPartPolyA);
    }

    aPartPolyA.clear();
    aPartPolyA.append(basegfx::B2DPoint(rPol.aHelpline1.aP1.X(), rPol.aHelpline1.aP1.Y()));
    aPartPolyA.append(basegfx::B2DPoint(rPol.aHelpline1.aP2.X(), rPol.aHelpline1.aP2.Y()));
    aRetval.append(aPartPolyA);

    aPartPolyA.clear();
    aPartPolyA.append(basegfx::B2DPoint(rPol.aHelpline2.aP1.X(), rPol.aHelpline2.aP1.Y()));
    aPartPolyA.append(basegfx::B2DPoint(rPol.aHelpline2.aP2.X(), rPol.aHelpline2.aP2.Y()));
    aRetval.append(aPartPolyA);

    return aRetval;
}

bool SdrMeasureObj::CalcFieldValue(const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos,
    bool bEdit, Color*& rpTxtColor, Color*& rpFldColor, XubString& rRet) const
{
    const SvxFieldData* pField=rField.GetField();
    const SdrMeasureField* pMeasureField = dynamic_cast< const SdrMeasureField* >( pField);

    if (pMeasureField)
    {
        TakeRepresentation(rRet, pMeasureField->GetMeasureFieldKind());

        if(rpFldColor)
        {
            if (!bEdit)
            {
                delete rpFldColor;

                rpFldColor = 0;
            }
        }

        return true;
    }
    else
    {
        return SdrTextObj::CalcFieldValue(rField,nPara,nPos,bEdit,rpTxtColor,rpFldColor,rRet);
    }
}

void SdrMeasureObj::UndirtyText() const
{
    if(bTextDirty)
    {
        SdrOutliner& rOutliner = ImpGetDrawOutliner();
        OutlinerParaObject* pOutlinerParaObject = SdrTextObj::GetOutlinerParaObject();
        SdrMeasureObj* pThat = const_cast<SdrMeasureObj*>(this);

        if(!pOutlinerParaObject)
        {
            rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_ROTA90BLANCS), EE_FEATURE_FIELD), ESelection(0,0));
            rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_VALUE), EE_FEATURE_FIELD),ESelection(0,1));
            rOutliner.QuickInsertText(String(sal_Unicode(' ')), ESelection(0,2));
            rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_UNIT), EE_FEATURE_FIELD),ESelection(0,3));
            rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_ROTA90BLANCS), EE_FEATURE_FIELD),ESelection(0,4));

            if(GetStyleSheet())
            {
                rOutliner.SetStyleSheet(0, GetStyleSheet());
            }

            rOutliner.SetParaAttribs(0, GetObjectItemSet());

            // need to reset bTextDirty first, else loop is triggered
            pThat->bTextDirty = false;
            pThat->SetOutlinerParaObject( rOutliner.CreateParaObject() );
        }
        else
        {
            rOutliner.SetText(*pOutlinerParaObject);
        }

        rOutliner.SetUpdateMode(sal_True);
        rOutliner.UpdateFields();
        pThat->aTextSize = rOutliner.CalcTextSize();
        pThat->bTextSizeDirty=sal_False;
        rOutliner.Clear();
    }
}

void SdrMeasureObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulMEASURE);

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

void SdrMeasureObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralMEASURE);
}

basegfx::B2DPolyPolygon SdrMeasureObj::TakeXorPoly() const
{
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    ImpTakeAttr(aRec);
    ImpCalcGeometrics(aRec,aMPol);
    return ImpCalcXPoly(aMPol);
}

void SdrMeasureObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    ImpTakeAttr(aRec);
    aRec.nHelplineDist=0;
    ImpCalcGeometrics(aRec,aMPol);

    SdrHdl* pHdl = new ImpMeasureHdl(rHdlList, *this, HDL_USER, basegfx::B2DPoint(aMPol.aHelpline1.aP1.X(), aMPol.aHelpline1.aP1.Y()));
    pHdl->SetObjHdlNum(0);

    pHdl = new ImpMeasureHdl(rHdlList, *this, HDL_USER, basegfx::B2DPoint(aMPol.aHelpline2.aP1.X(), aMPol.aHelpline2.aP1.Y()));
    pHdl->SetObjHdlNum(1);

    pHdl = new ImpMeasureHdl(rHdlList, *this, HDL_USER, basegfx::B2DPoint(aPt1.X(), aPt1.Y()));
    pHdl->SetObjHdlNum(2);

    pHdl = new ImpMeasureHdl(rHdlList, *this, HDL_USER, basegfx::B2DPoint(aPt2.X(), aPt2.Y()));
    pHdl->SetObjHdlNum(3);

    pHdl = new ImpMeasureHdl(rHdlList, *this, HDL_USER, basegfx::B2DPoint(aMPol.aHelpline1.aP2.X(), aMPol.aHelpline1.aP2.Y()));
    pHdl->SetObjHdlNum(4);

    pHdl = new ImpMeasureHdl(rHdlList, *this, HDL_USER, basegfx::B2DPoint(aMPol.aHelpline1.aP2.X(), aMPol.aHelpline1.aP2.Y()));
    pHdl->SetObjHdlNum(5);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrMeasureObj::hasSpecialDrag() const
{
    return true;
}

bool SdrMeasureObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl = rDrag.GetActiveHdl();

    if(pHdl)
    {
        const sal_uInt32 nHdlNum(pHdl->GetObjHdlNum());

        if(nHdlNum != 2 && nHdlNum != 3)
        {
            rDrag.SetEndDragChangesAttributes(true);
        }

        return true;
    }

    return false;
}

bool SdrMeasureObj::applySpecialDrag(SdrDragStat& rDrag)
{
    ImpMeasureRec aMeasureRec;
    const SdrHdl* pHdl = rDrag.GetActiveHdl();
    const sal_uInt32 nHdlNum(pHdl->GetObjHdlNum());

    ImpTakeAttr(aMeasureRec);
    ImpEvalDrag(aMeasureRec, rDrag);

    switch (nHdlNum)
    {
        case 2:
        {
            aPt1 = aMeasureRec.aPt1;
            SetTextDirty();
            break;
        }
        case 3:
        {
            aPt2 = aMeasureRec.aPt2;
            SetTextDirty();
            break;
        }
        default:
        {
            switch(nHdlNum)
            {
                case 0:
                case 1:
                {
                    ImpMeasureRec aOrigMeasureRec;
                    ImpTakeAttr(aOrigMeasureRec);

                    if(aMeasureRec.nHelpline1Len != aOrigMeasureRec.nHelpline1Len)
                    {
                        SetObjectItem(SdrMetricItem(SDRATTR_MEASUREHELPLINE1LEN, aMeasureRec.nHelpline1Len));
                    }

                    if(aMeasureRec.nHelpline2Len != aOrigMeasureRec.nHelpline2Len)
                    {
                        SetObjectItem(SdrMetricItem(SDRATTR_MEASUREHELPLINE2LEN, aMeasureRec.nHelpline2Len));
                    }

                    break;
                }

                case 4:
                case 5:
                {
                    ImpMeasureRec aOrigMeasureRec;
                    ImpTakeAttr(aOrigMeasureRec);

                    if(aMeasureRec.nLineDist != aOrigMeasureRec.nLineDist)
                    {
                        SetObjectItem(SdrMetricItem(SDRATTR_MEASURELINEDIST, aMeasureRec.nLineDist));
                    }

                    if(aMeasureRec.bBelowRefEdge != aOrigMeasureRec.bBelowRefEdge)
                    {
                        SetObjectItem(SdrYesNoItem(SDRATTR_MEASUREBELOWREFEDGE, aMeasureRec.bBelowRefEdge));
                    }
                }
            }
        }
    } // switch

    SetChanged();

    return true;
}

String SdrMeasureObj::getSpecialDragComment(const SdrDragStat& /*rDrag*/) const
{
    XubString aStr;
    return aStr;
}

void SdrMeasureObj::ImpEvalDrag(ImpMeasureRec& rRec, const SdrDragStat& rDrag) const
{
    long nLineWink=GetAngle(rRec.aPt2-rRec.aPt1);
    double a=nLineWink*nPi180;
    double nSin=sin(a);
    double nCos=cos(a);

    const SdrHdl* pHdl=rDrag.GetActiveHdl();
    sal_uInt32 nHdlNum(pHdl->GetObjHdlNum());
    bool bOrtho(rDrag.GetSdrViewFromSdrDragStat().IsOrthogonal());
    bool bBigOrtho(bOrtho && rDrag.GetSdrViewFromSdrDragStat().IsBigOrthogonal());
    bool bBelow(rRec.bBelowRefEdge);
    Point aPt(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));

    switch (nHdlNum) {
        case 0: {
            RotatePoint(aPt,aPt1,nSin,-nCos);
            rRec.nHelpline1Len=aPt1.Y()-aPt.Y();
            if (bBelow) rRec.nHelpline1Len=-rRec.nHelpline1Len;
            if (bOrtho) rRec.nHelpline2Len=rRec.nHelpline1Len;
        } break;
        case 1: {
            RotatePoint(aPt,aPt2,nSin,-nCos);
            rRec.nHelpline2Len=aPt2.Y()-aPt.Y();
            if (bBelow) rRec.nHelpline2Len=-rRec.nHelpline2Len;
            if (bOrtho) rRec.nHelpline1Len=rRec.nHelpline2Len;
        } break;
        case 2: case 3: {
            bool bAnf=nHdlNum==2;
            Point& rMov=bAnf ? rRec.aPt1 : rRec.aPt2;
            Point aMov(rMov);
            Point aFix(bAnf ? rRec.aPt2 : rRec.aPt1);
            if (bOrtho) {
                long ndx0=aMov.X()-aFix.X();
                long ndy0=aMov.Y()-aFix.Y();
                bool bHLin=ndy0==0;
                bool bVLin=ndx0==0;
                if (!bHLin || !bVLin) { // sonst ist aPt1==aPt2
                    long ndx=aPt.X()-aFix.X();
                    long ndy=aPt.Y()-aFix.Y();
                    double nXFact=0; if (!bVLin) nXFact=(double)ndx/(double)ndx0;
                    double nYFact=0; if (!bHLin) nYFact=(double)ndy/(double)ndy0;
                    bool bHor=bHLin || (!bVLin && (nXFact>nYFact) ==bBigOrtho);
                    bool bVer=bVLin || (!bHLin && (nXFact<=nYFact)==bBigOrtho);
                    if (bHor) ndy=long(ndy0*nXFact);
                    if (bVer) ndx=long(ndx0*nYFact);
                    aPt=aFix;
                    aPt.X()+=ndx;
                    aPt.Y()+=ndy;
                } // else Ortho8
            }
            rMov=aPt;
        } break;
        case 4: case 5: {
            long nVal0=rRec.nLineDist;
            RotatePoint(aPt,(nHdlNum==4 ? aPt1 : aPt2),nSin,-nCos);
            rRec.nLineDist=aPt.Y()- (nHdlNum==4 ? aPt1.Y() : aPt2.Y());
            if (bBelow) rRec.nLineDist=-rRec.nLineDist;
            if (rRec.nLineDist<0) {
                rRec.nLineDist=-rRec.nLineDist;
                rRec.bBelowRefEdge=!bBelow;
            }
            rRec.nLineDist-=rRec.nHelplineOverhang;
            if (bOrtho) rRec.nLineDist=nVal0;
        } break;
    } // switch
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrMeasureObj::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho8Possible();
    aPt1=Point(basegfx::fround(rStat.GetStart().getX()), basegfx::fround(rStat.GetStart().getY()));
    aPt2=Point(basegfx::fround(rStat.GetNow().getX()), basegfx::fround(rStat.GetNow().getY()));
    SetTextDirty();

    return true;
}

bool SdrMeasureObj::MovCreate(SdrDragStat& rStat)
{
    SdrView& rView = rStat.GetSdrViewFromSdrDragStat();
    aPt1=Point(basegfx::fround(rStat.GetStart().getX()), basegfx::fround(rStat.GetStart().getY()));
    aPt2=Point(basegfx::fround(rStat.GetNow().getX()), basegfx::fround(rStat.GetNow().getY()));

    if (rView.IsCreate1stPointAsCenter())
{
        aPt1+=aPt1;
        aPt1-=Point(basegfx::fround(rStat.GetNow().getX()), basegfx::fround(rStat.GetNow().getY()));
    }
    SetTextDirty();
    ActionChanged();

    return true;
}

bool SdrMeasureObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    SetTextDirty();
    ActionChanged();

    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

bool SdrMeasureObj::BckCreate(SdrDragStat& /*rStat*/)
{
    return false;
}

void SdrMeasureObj::BrkCreate(SdrDragStat& /*rStat*/)
{
}

basegfx::B2DPolyPolygon SdrMeasureObj::TakeCreatePoly(const SdrDragStat& /*rDrag*/) const
{
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;

    ImpTakeAttr(aRec);
    ImpCalcGeometrics(aRec, aMPol);

    return ImpCalcXPoly(aMPol);
}

sal_uInt32 SdrMeasureObj::GetSnapPointCount() const
{
    return 2L;
}

basegfx::B2DPoint SdrMeasureObj::GetSnapPoint(sal_uInt32 i) const
{
    if(!i)
{
        return basegfx::B2DPoint(aPt1.X(), aPt1.Y());
}
    else
{
        return basegfx::B2DPoint(aPt2.X(), aPt2.Y());
}
}

bool SdrMeasureObj::IsPolygonObject() const
{
    return sal_True;
}

sal_uInt32 SdrMeasureObj::GetObjectPointCount() const
{
    return 2;
}

basegfx::B2DPoint SdrMeasureObj::GetObjectPoint(sal_uInt32 i) const
{
     return (i) ? basegfx::B2DPoint(aPt2.X(), aPt2.Y()) : basegfx::B2DPoint(aPt1.X(), aPt1.Y());
}

void SdrMeasureObj::SetObjectPoint(const basegfx::B2DPoint& rPnt, sal_uInt32 i)
{
    if(0 == i)
{
        aPt1 = Point(basegfx::fround(rPnt.getX()), basegfx::fround(rPnt.getY()));
}
    else if (1 == i)
{
        aPt2 = Point(basegfx::fround(rPnt.getX()), basegfx::fround(rPnt.getY()));
}

    ActionChanged();
    SetTextDirty();
}

SdrObjGeoData* SdrMeasureObj::NewGeoData() const
{
    return new SdrMeasureObjGeoData;
}

void SdrMeasureObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrTextObj::SaveGeoData(rGeo);
    SdrMeasureObjGeoData& rMGeo=(SdrMeasureObjGeoData&)rGeo;
    rMGeo.aPt1=aPt1;
    rMGeo.aPt2=aPt2;
}

void SdrMeasureObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrTextObj::RestGeoData(rGeo);
    SdrMeasureObjGeoData& rMGeo=(SdrMeasureObjGeoData&)rGeo;
    aPt1=rMGeo.aPt1;
    aPt2=rMGeo.aPt2;
    SetTextDirty();
}

SdrObject* SdrMeasureObj::DoConvertToPolygonObject(bool bBezier, bool bAddText) const
{
    // get XOR Poly as base
    XPolyPolygon aTmpPolyPolygon(TakeXorPoly());

    // get local ItemSet and StyleSheet
    SfxItemSet aSet(GetObjectItemSet());
    SfxStyleSheet* pStyleSheet = GetStyleSheet();

    // prepare group
    SdrObjGroup* pGroup = new SdrObjGroup(getSdrModelFromSdrObject());

    // prepare parameters
    basegfx::B2DPolyPolygon aPolyPoly;
    SdrPathObj* pPath;
    sal_uInt16 nCount(aTmpPolyPolygon.Count());
    sal_uInt16 nLoopStart(0);

    if(nCount == 3)
    {
        // three lines, first one is the middle one
        aPolyPoly.clear();
        aPolyPoly.append(aTmpPolyPolygon[0].getB2DPolygon());

        pPath = new SdrPathObj(getSdrModelFromSdrObject(), aPolyPoly);
        pPath->SetMergedItemSet(aSet);
        pPath->SetStyleSheet(pStyleSheet, true);
        pGroup->InsertObjectToSdrObjList(*pPath);
        aSet.Put(XLineStartWidthItem(0L));
        aSet.Put(XLineEndWidthItem(0L));
        nLoopStart = 1;
    }
    else if(nCount == 4)
    {
        // four lines, middle line with gap, so there are two lines used
        // which have one arrow each
        sal_Int32 nEndWidth = ((const XLineEndWidthItem&)(aSet.Get(XATTR_LINEENDWIDTH))).GetValue();
        aSet.Put(XLineEndWidthItem(0L));

        aPolyPoly.clear();
        aPolyPoly.append(aTmpPolyPolygon[0].getB2DPolygon());
        pPath = new SdrPathObj(getSdrModelFromSdrObject(), aPolyPoly);
        pPath->SetMergedItemSet(aSet);
        pPath->SetStyleSheet(pStyleSheet, true);

        pGroup->InsertObjectToSdrObjList(*pPath);

        aSet.Put(XLineEndWidthItem(nEndWidth));
        aSet.Put(XLineStartWidthItem(0L));

        aPolyPoly.clear();
        aPolyPoly.append(aTmpPolyPolygon[1].getB2DPolygon());
        pPath = new SdrPathObj(getSdrModelFromSdrObject(), aPolyPoly);
        pPath->SetMergedItemSet(aSet);
        pPath->SetStyleSheet(pStyleSheet, true);

        pGroup->InsertObjectToSdrObjList(*pPath);

        aSet.Put(XLineEndWidthItem(0L));
        nLoopStart = 2;
    }
    else if(nCount == 5)
    {
        // five lines, first two are the outer ones
        sal_Int32 nEndWidth = ((const XLineEndWidthItem&)(aSet.Get(XATTR_LINEENDWIDTH))).GetValue();

        aSet.Put(XLineEndWidthItem(0L));

        aPolyPoly.clear();
        aPolyPoly.append(aTmpPolyPolygon[0].getB2DPolygon());
        pPath = new SdrPathObj(getSdrModelFromSdrObject(), aPolyPoly);
        pPath->SetMergedItemSet(aSet);
        pPath->SetStyleSheet(pStyleSheet, true);

        pGroup->InsertObjectToSdrObjList(*pPath);

        aSet.Put(XLineEndWidthItem(nEndWidth));
        aSet.Put(XLineStartWidthItem(0L));

        aPolyPoly.clear();
        aPolyPoly.append(aTmpPolyPolygon[1].getB2DPolygon());
        pPath = new SdrPathObj(getSdrModelFromSdrObject(), aPolyPoly);
        pPath->SetMergedItemSet(aSet);
        pPath->SetStyleSheet(pStyleSheet, true);

        pGroup->InsertObjectToSdrObjList(*pPath);

        aSet.Put(XLineEndWidthItem(0L));
        nLoopStart = 2;
    }

    for(;nLoopStart<nCount;nLoopStart++)
    {
        aPolyPoly.clear();
        aPolyPoly.append(aTmpPolyPolygon[nLoopStart].getB2DPolygon());
        pPath = new SdrPathObj(getSdrModelFromSdrObject(), aPolyPoly);
        pPath->SetMergedItemSet(aSet);
        pPath->SetStyleSheet(pStyleSheet, true);

        pGroup->InsertObjectToSdrObjList(*pPath);
    }

    if(bAddText)
    {
        return ImpConvertAddText(pGroup, bBezier);
    }
    else
    {
        return pGroup;
    }
}

bool SdrMeasureObj::BegTextEdit(SdrOutliner& rOutl)
{
    UndirtyText();
    return SdrTextObj::BegTextEdit(rOutl);
}

const Size& SdrMeasureObj::GetTextSize() const
{
    if (bTextDirty)
        UndirtyText();
    return SdrTextObj::GetTextSize();
}

OutlinerParaObject* SdrMeasureObj::GetOutlinerParaObject() const
{
    if(bTextDirty)
        UndirtyText();
    return SdrTextObj::GetOutlinerParaObject();
}

void SdrMeasureObj::SetOutlinerParaObject(OutlinerParaObject* pTextObject)
{
    SdrTextObj::SetOutlinerParaObject(pTextObject);

    if(SdrTextObj::GetOutlinerParaObject())
    {
        SetTextDirty(); // Text neu berechnen!
    }
}

void SdrMeasureObj::TakeTextRange(SdrOutliner& rOutliner, basegfx::B2DRange& rTextRange, basegfx::B2DRange& rAnchorRange) const
{
    if(bTextDirty)
    {
        UndirtyText();
    }

    SdrTextObj::TakeTextRange(rOutliner, rTextRange, rAnchorRange);
}

basegfx::B2DRange SdrMeasureObj::getUnifiedTextRange() const
{
    if(bTextDirty)
    {
        UndirtyText();
    }

    // call parent
    return SdrTextObj::getUnifiedTextRange();
}

void SdrMeasureObj::TakeTextEditArea(basegfx::B2DVector* pPaperMin, basegfx::B2DVector* pPaperMax, basegfx::B2DRange* pViewInit, basegfx::B2DRange* pViewMin) const
{
    if (bTextDirty)
        UndirtyText();

    SdrTextObj::TakeTextEditArea(pPaperMin,pPaperMax,pViewInit,pViewMin);
}

sal_uInt16 SdrMeasureObj::GetOutlinerViewAnchorMode() const
{
    if (bTextDirty)
        UndirtyText();
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    ImpTakeAttr(aRec);
    ImpCalcGeometrics(aRec,aMPol);

    SdrTextHorzAdjust eTH=GetTextHorizontalAdjust();
    SdrTextVertAdjust eTV=GetTextVerticalAdjust();
    SdrMeasureTextHPos eMH=aMPol.eUsedTextHPos;
    SdrMeasureTextVPos eMV=aMPol.eUsedTextVPos;
    bool bTextRota90=aRec.bTextRota90;
    bool bBelowRefEdge=aRec.bBelowRefEdge;

    if (!bTextRota90)
    {
        if (eMH==SDRMEASURE_TEXTLEFTOUTSIDE) eTH=SDRTEXTHORZADJUST_RIGHT;
        if (eMH==SDRMEASURE_TEXTRIGHTOUTSIDE) eTH=SDRTEXTHORZADJUST_LEFT;
        // bei eMH==SDRMEASURE_TEXTINSIDE kann horizontal geankert werden.
        if (eMV==SDRMEASURE_ABOVE) eTV=SDRTEXTVERTADJUST_BOTTOM;
        if (eMV==SDRMEASURE_BELOW) eTV=SDRTEXTVERTADJUST_TOP;
        if (eMV==SDRMEASURETEXT_BREAKEDLINE || eMV==SDRMEASURETEXT_VERTICALCENTERED) eTV=SDRTEXTVERTADJUST_CENTER;
    }
    else
    {
        if (eMH==SDRMEASURE_TEXTLEFTOUTSIDE) eTV=SDRTEXTVERTADJUST_BOTTOM;
        if (eMH==SDRMEASURE_TEXTRIGHTOUTSIDE) eTV=SDRTEXTVERTADJUST_TOP;
        // bei eMH==SDRMEASURE_TEXTINSIDE kann vertikal geankert werden.
        if (!bBelowRefEdge) {
            if (eMV==SDRMEASURE_ABOVE) eTH=SDRTEXTHORZADJUST_LEFT;
            if (eMV==SDRMEASURE_BELOW) eTH=SDRTEXTHORZADJUST_RIGHT;
        } else {
            if (eMV==SDRMEASURE_ABOVE) eTH=SDRTEXTHORZADJUST_RIGHT;
            if (eMV==SDRMEASURE_BELOW) eTH=SDRTEXTHORZADJUST_LEFT;
        }
        if (eMV==SDRMEASURETEXT_BREAKEDLINE || eMV==SDRMEASURETEXT_VERTICALCENTERED) eTH=SDRTEXTHORZADJUST_CENTER;
    }

    EVAnchorMode eRet=ANCHOR_BOTTOM_HCENTER;
    if (eTH==SDRTEXTHORZADJUST_LEFT) {
        if (eTV==SDRTEXTVERTADJUST_TOP) eRet=ANCHOR_TOP_LEFT;
        else if (eTV==SDRTEXTVERTADJUST_BOTTOM) eRet=ANCHOR_BOTTOM_LEFT;
        else eRet=ANCHOR_VCENTER_LEFT;
    } else if (eTH==SDRTEXTHORZADJUST_RIGHT) {
        if (eTV==SDRTEXTVERTADJUST_TOP) eRet=ANCHOR_TOP_RIGHT;
        else if (eTV==SDRTEXTVERTADJUST_BOTTOM) eRet=ANCHOR_BOTTOM_RIGHT;
        else eRet=ANCHOR_VCENTER_RIGHT;
    } else {
        if (eTV==SDRTEXTVERTADJUST_TOP) eRet=ANCHOR_TOP_HCENTER;
        else if (eTV==SDRTEXTVERTADJUST_BOTTOM) eRet=ANCHOR_BOTTOM_HCENTER;
        else eRet=ANCHOR_VCENTER_HCENTER;
    }
    return (sal_uInt16)eRet;
}

//////////////////////////////////////////////////////////////////////////////
// eof
