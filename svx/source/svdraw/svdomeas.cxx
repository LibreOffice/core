/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "svdconv.hxx"
#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdstr.hrc"    // the object's name

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editobj.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/measfld.hxx>
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <math.h>
#include <svl/smplhint.hxx>
#include <svl/style.hxx>

#include <svx/sdr/contact/viewcontactofsdrmeasureobj.hxx>
#include <svx/sdr/properties/measureproperties.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdomeas.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdview.hxx>
#include <svx/sxmbritm.hxx>
#include <svx/sxmfsitm.hxx>
#include <svx/sxmkitm.hxx>
#include <svx/sxmlhitm.hxx>
#include <svx/sxmoitm.hxx>
#include <svx/sxmsitm.hxx>
#include <svx/sxmsuitm.hxx>
#include <svx/sxmtaitm.hxx>
#include <svx/sxmtfitm.hxx>
#include <svx/sxmtpitm.hxx>
#include <svx/sxmtritm.hxx>
#include <svx/sxmtritm.hxx>
#include <svx/sxmuitm.hxx>
#include <svx/xlnedcit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xpoly.hxx>
#include <unotools/syslocale.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrMeasureObjGeoData::SdrMeasureObjGeoData() {}
SdrMeasureObjGeoData::~SdrMeasureObjGeoData() {}

OUString SdrMeasureObj::TakeRepresentation(SdrMeasureFieldKind eMeasureFieldKind) const
{
    String aStr;
    Fraction aMeasureScale(1, 1);
    sal_Bool bTextRota90(sal_False);
    sal_Bool bShowUnit(sal_False);
    FieldUnit eMeasureUnit(FUNIT_NONE);
    FieldUnit eModUIUnit(FUNIT_NONE);

    const SfxItemSet& rSet = GetMergedItemSet();
    bTextRota90 = ((SdrMeasureTextRota90Item&)rSet.Get(SDRATTR_MEASURETEXTROTA90)).GetValue();
    eMeasureUnit = ((SdrMeasureUnitItem&)rSet.Get(SDRATTR_MEASUREUNIT)).GetValue();
    aMeasureScale = ((SdrMeasureScaleItem&)rSet.Get(SDRATTR_MEASURESCALE)).GetValue();
    bShowUnit = ((SdrMeasureShowUnitItem&)rSet.Get(SDRATTR_MEASURESHOWUNIT)).GetValue();
    sal_Int16 nNumDigits = ((SdrMeasureDecimalPlacesItem&)rSet.Get(SDRATTR_MEASUREDECIMALPLACES)).GetValue();

    switch(eMeasureFieldKind)
    {
        case SDRMEASUREFIELD_VALUE:
        {
            if(pModel)
            {
                eModUIUnit = pModel->GetUIUnit();

                if(eMeasureUnit == FUNIT_NONE)
                    eMeasureUnit = eModUIUnit;

                sal_Int32 nLen(GetLen(aPt2 - aPt1));
                Fraction aFact(1,1);

                if(eMeasureUnit != eModUIUnit)
                {
                    // for the unit conversion
                    aFact *= GetMapFactor(eModUIUnit, eMeasureUnit).X();
                }

                if(aMeasureScale.GetNumerator() != aMeasureScale.GetDenominator())
                {
                    aFact *= aMeasureScale;
                }

                if(aFact.GetNumerator() != aFact.GetDenominator())
                {
                    // scale via BigInt, to avoid overruns
                    nLen = BigMulDiv(nLen, aFact.GetNumerator(), aFact.GetDenominator());
                }

                OUString aTmp;
                pModel->TakeMetricStr(nLen, aTmp, true, nNumDigits);
                aStr = aTmp;

                if(!aFact.IsValid())
                {
                    aStr = String();
                    aStr += sal_Unicode('?');
                }

                sal_Unicode cDec(SvtSysLocale().GetLocaleData().getNumDecimalSep()[0]);

                if(aStr.Search(cDec) != STRING_NOTFOUND)
                {
                    xub_StrLen nLen2(aStr.Len() - 1);

                    while(aStr.GetChar(nLen2) == sal_Unicode('0'))
                    {
                        aStr.Erase(nLen2);
                        nLen2--;
                    }

                    if(aStr.GetChar(nLen2) == cDec)
                    {
                        aStr.Erase(nLen2);
                        nLen2--;
                    }

                    if(!aStr.Len())
                        aStr += sal_Unicode('0');
                }
            }
            else
            {
                // if there's no Model ... (e. g. preview in dialog)
                aStr = String();
                aStr.AppendAscii("4711");
            }

            break;
        }
        case SDRMEASUREFIELD_UNIT:
        {
            if(bShowUnit)
            {
                if(pModel)
                {
                    eModUIUnit = pModel->GetUIUnit();

                    if(eMeasureUnit == FUNIT_NONE)
                        eMeasureUnit = eModUIUnit;

                    if(bShowUnit)
                        pModel->TakeUnitStr(eMeasureUnit, aStr);
                }
            }

            break;
        }
        case SDRMEASUREFIELD_ROTA90BLANCS:
        {
            if(bTextRota90)
            {
                aStr = String();
                aStr += sal_Unicode(' ');
            }

            break;
        }
    }
    return aStr;
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

TYPEINIT1(SdrMeasureObj,SdrTextObj);

SdrMeasureObj::SdrMeasureObj():
    bTextDirty(sal_False)
{
    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_False;
}

SdrMeasureObj::SdrMeasureObj(const Point& rPt1, const Point& rPt2):
    aPt1(rPt1),
    aPt2(rPt2),
    bTextDirty(sal_False)
{
    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_False;
}

SdrMeasureObj::~SdrMeasureObj()
{
}

void SdrMeasureObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bSelectAllowed    =sal_True;
    rInfo.bMoveAllowed      =sal_True;
    rInfo.bResizeFreeAllowed=sal_True;
    rInfo.bResizePropAllowed=sal_True;
    rInfo.bRotateFreeAllowed=sal_True;
    rInfo.bRotate90Allowed  =sal_True;
    rInfo.bMirrorFreeAllowed=sal_True;
    rInfo.bMirror45Allowed  =sal_True;
    rInfo.bMirror90Allowed  =sal_True;
    rInfo.bTransparenceAllowed = sal_False;
    rInfo.bGradientAllowed = sal_False;
    rInfo.bShearAllowed     =sal_True;
    rInfo.bEdgeRadiusAllowed=sal_False;
    rInfo.bNoOrthoDesired   =sal_True;
    rInfo.bNoContortion     =sal_False;
    rInfo.bCanConvToPath    =sal_False;
    rInfo.bCanConvToPoly    =sal_True;
    rInfo.bCanConvToPathLineToArea=sal_False;
    rInfo.bCanConvToPolyLineToArea=sal_False;
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
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
    bool                        bBelowRefEdge;
    bool                        bTextRota90;
    bool                        bTextUpsideDown;
    long                        nMeasureOverhang;
    FieldUnit                   eMeasureUnit;
    Fraction                    aMeasureScale;
    bool                        bShowUnit;
    String                      aFormatString;
    bool                        bTextAutoAngle;
    long                        nTextAutoAngleView;
    bool                        bTextIsFixedAngle;
    long                        nTextFixedAngle;
};

struct ImpLineRec
{
    Point                       aP1;
    Point                       aP2;
};

struct ImpMeasurePoly
{
    ImpLineRec                  aMainline1; // those with the 1st arrowhead
    ImpLineRec                  aMainline2; // those with the 2nd arrowhead
    ImpLineRec                  aMainline3; // those in between
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
    long                        nLineWdt2;  // half the line width
    long                        nArrow1Len; // length of 1st arrowhead; for Center, use only half
    long                        nArrow2Len; // length of 2nd arrowhead; for Center, use only half
    long                        nArrow1Wdt; // width of 1st arrow
    long                        nArrow2Wdt; // width of 2nd arrow
    long                        nShortLineLen; // line length, if PfeileAussen (arrowheads on the outside)
    bool                        bArrow1Center; // arrowhead 1 centered?
    bool                        bArrow2Center; // arrowhead 2 centered?
    bool                        bAutoUpsideDown; // UpsideDown via automation
    bool                        bPfeileAussen; // arrowheads on the outside
    bool                        bBreakedLine;
};

void SdrMeasureObj::ImpTakeAttr(ImpMeasureRec& rRec) const
{
    rRec.aPt1 = aPt1;
    rRec.aPt2 = aPt2;

    const SfxItemSet& rSet = GetObjectItemSet();
    rRec.eKind            =((SdrMeasureKindItem&            )rSet.Get(SDRATTR_MEASUREKIND            )).GetValue();
    rRec.eWantTextHPos    =((SdrMeasureTextHPosItem&        )rSet.Get(SDRATTR_MEASURETEXTHPOS        )).GetValue();
    rRec.eWantTextVPos    =((SdrMeasureTextVPosItem&        )rSet.Get(SDRATTR_MEASURETEXTVPOS        )).GetValue();
    rRec.nLineDist        =((SdrMeasureLineDistItem&        )rSet.Get(SDRATTR_MEASURELINEDIST        )).GetValue();
    rRec.nHelplineOverhang=((SdrMeasureHelplineOverhangItem&)rSet.Get(SDRATTR_MEASUREHELPLINEOVERHANG)).GetValue();
    rRec.nHelplineDist    =((SdrMeasureHelplineDistItem&    )rSet.Get(SDRATTR_MEASUREHELPLINEDIST    )).GetValue();
    rRec.nHelpline1Len    =((SdrMeasureHelpline1LenItem&    )rSet.Get(SDRATTR_MEASUREHELPLINE1LEN    )).GetValue();
    rRec.nHelpline2Len    =((SdrMeasureHelpline2LenItem&    )rSet.Get(SDRATTR_MEASUREHELPLINE2LEN    )).GetValue();
    rRec.bBelowRefEdge    =((SdrMeasureBelowRefEdgeItem&    )rSet.Get(SDRATTR_MEASUREBELOWREFEDGE    )).GetValue();
    rRec.bTextRota90      =((SdrMeasureTextRota90Item&      )rSet.Get(SDRATTR_MEASURETEXTROTA90      )).GetValue();
    rRec.bTextUpsideDown  =((SdrMeasureTextUpsideDownItem&  )rSet.Get(SDRATTR_MEASURETEXTUPSIDEDOWN  )).GetValue();
    rRec.nMeasureOverhang =((SdrMeasureOverhangItem&        )rSet.Get(SDRATTR_MEASUREOVERHANG        )).GetValue();
    rRec.eMeasureUnit     =((SdrMeasureUnitItem&            )rSet.Get(SDRATTR_MEASUREUNIT            )).GetValue();
    rRec.aMeasureScale    =((SdrMeasureScaleItem&           )rSet.Get(SDRATTR_MEASURESCALE           )).GetValue();
    rRec.bShowUnit        =((SdrMeasureShowUnitItem&        )rSet.Get(SDRATTR_MEASURESHOWUNIT        )).GetValue();
    rRec.aFormatString    =((SdrMeasureFormatStringItem&    )rSet.Get(SDRATTR_MEASUREFORMATSTRING    )).GetValue();
    rRec.bTextAutoAngle    =((SdrMeasureTextAutoAngleItem&    )rSet.Get(SDRATTR_MEASURETEXTAUTOANGLE    )).GetValue();
    rRec.nTextAutoAngleView=((SdrMeasureTextAutoAngleViewItem&)rSet.Get(SDRATTR_MEASURETEXTAUTOANGLEVIEW)).GetValue();
    rRec.bTextIsFixedAngle =((SdrMeasureTextIsFixedAngleItem& )rSet.Get(SDRATTR_MEASURETEXTISFIXEDANGLE )).GetValue();
    rRec.nTextFixedAngle   =((SdrMeasureTextFixedAngleItem&   )rSet.Get(SDRATTR_MEASURETEXTFIXEDANGLE   )).GetValue();
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
    bool bPfeileAussen = false;

    const SfxItemSet& rSet = GetObjectItemSet();
    sal_Int32 nLineWdt = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue(); // line width
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

    // nArrowLen is already halved at bCenter.
    // In the case of 2 arrowheads each 4mm long, we can't go below 10mm.
    nArrowNeed=nArrow1Len+nArrow2Len+(nArrow1Wdt+nArrow2Wdt)/2;
    if (rPol.nLineLen<nArrowNeed) bPfeileAussen = true;
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
            bBrkLine=sal_True; // dashed line if there's only on paragraph.
        }
    }
    rPol.bBreakedLine=bBrkLine;
    if (rPol.eUsedTextHPos==SDRMEASURE_TEXTHAUTO) { // if text is too wide, push it outside
        bool bOutside = false;
        long nNeedSiz=!rRec.bTextRota90 ? rPol.aTextSize.Width() : rPol.aTextSize.Height();
        if (nNeedSiz>rPol.nLineLen) bOutside = true; // text doesn't fit in between
        if (bBrkLine) {
            if (nNeedSiz+nArrowNeed>rPol.nLineLen) bPfeileAussen = true; // text fits in between, if arrowheads are on the outside
        } else {
            long nSmallNeed=nArrow1Len+nArrow2Len+(nArrow1Wdt+nArrow2Wdt)/2/4;
            if (nNeedSiz+nSmallNeed>rPol.nLineLen) bPfeileAussen = true; // text fits in between, if arrowheads are on the outside
        }
        rPol.eUsedTextHPos=bOutside ? SDRMEASURE_TEXTLEFTOUTSIDE : SDRMEASURE_TEXTINSIDE;
    }
    if (rPol.eUsedTextHPos!=SDRMEASURE_TEXTINSIDE) bPfeileAussen = true;
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

    // extension line 1
    rPol.aHelpline1.aP1=Point(aP1.X()+dxh1a,aP1.Y()+dyh1a);
    rPol.aHelpline1.aP2=Point(aP1.X()+dxh2,aP1.Y()+dyh2);

    // extension line 2
    rPol.aHelpline2.aP1=Point(aP2.X()+dxh1b,aP2.Y()+dyh1b);
    rPol.aHelpline2.aP2=Point(aP2.X()+dxh2,aP2.Y()+dyh2);

    // dimension line
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
        long nLen1=nShortLen; // arrowhead's width as line length outside of the arrowhead
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

bool SdrMeasureObj::CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos,
    bool bEdit,
    Color*& rpTxtColor, Color*& rpFldColor, XubString& rRet) const
{
    const SvxFieldData* pField=rField.GetField();
    SdrMeasureField* pMeasureField=PTR_CAST(SdrMeasureField,pField);
    if (pMeasureField!=NULL) {
        rRet = TakeRepresentation(pMeasureField->GetMeasureFieldKind());
        if (rpFldColor!=NULL) {
            if (!bEdit)
            {
                delete rpFldColor;
                rpFldColor=NULL;
            }
        }
        return sal_True;
    } else {
        return SdrTextObj::CalcFieldValue(rField,nPara,nPos,bEdit,rpTxtColor,rpFldColor,rRet);
    }
}

void SdrMeasureObj::UndirtyText() const
{
    if (bTextDirty)
    {
        SdrOutliner& rOutliner=ImpGetDrawOutliner();
        OutlinerParaObject* pOutlinerParaObject = SdrTextObj::GetOutlinerParaObject();
        if(pOutlinerParaObject==NULL)
        {
            rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_ROTA90BLANCS), EE_FEATURE_FIELD), ESelection(0,0));
            rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_VALUE), EE_FEATURE_FIELD),ESelection(0,1));
            rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_UNIT), EE_FEATURE_FIELD),ESelection(0,2));
            rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_ROTA90BLANCS), EE_FEATURE_FIELD),ESelection(0,3));

            if(GetStyleSheet())
                rOutliner.SetStyleSheet(0, GetStyleSheet());

            rOutliner.SetParaAttribs(0, GetObjectItemSet());

            // cast to nonconst
            const_cast<SdrMeasureObj*>(this)->NbcSetOutlinerParaObject( rOutliner.CreateParaObject() );
        }
        else
        {
            rOutliner.SetText(*pOutlinerParaObject);
        }

        rOutliner.SetUpdateMode(sal_True);
        rOutliner.UpdateFields();
        Size aSiz(rOutliner.CalcTextSize());
        rOutliner.Clear();
        // cast to nonconst three times
        ((SdrMeasureObj*)this)->aTextSize=aSiz;
        ((SdrMeasureObj*)this)->bTextSizeDirty=sal_False;
        ((SdrMeasureObj*)this)->bTextDirty=sal_False;
    }
}

void SdrMeasureObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
{
    if (bTextDirty) UndirtyText();
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    ImpTakeAttr(aRec);
    ImpCalcGeometrics(aRec,aMPol);

    // determine TextSize including text frame margins
    Size aTextSize2(aMPol.aTextSize);
    if (aTextSize2.Width()<1) aTextSize2.Width()=1;
    if (aTextSize2.Height()<1) aTextSize2.Height()=1;
    aTextSize2.Width()+=GetTextLeftDistance()+GetTextRightDistance();
    aTextSize2.Height()+=GetTextUpperDistance()+GetTextLowerDistance();

    Point aPt1b(aMPol.aMainline1.aP1);
    long nLen=aMPol.nLineLen;
    long nLWdt=aMPol.nLineWdt2;
    long nArr1Len=aMPol.nArrow1Len;
    long nArr2Len=aMPol.nArrow2Len;
    if (aMPol.bBreakedLine) {
        // In the case of a dashed line and Outside, the text should be
        // placed next to the line at the arrowhead instead of directly
        // at the arrowhead.
        nArr1Len=aMPol.nShortLineLen+aMPol.nArrow1Wdt/4;
        nArr2Len=aMPol.nShortLineLen+aMPol.nArrow2Wdt/4;
    }

    Point aTextPos;
    bool bRota90=aRec.bTextRota90;
    bool bUpsideDown=aRec.bTextUpsideDown!=aMPol.bAutoUpsideDown;
    bool bBelowRefEdge=aRec.bBelowRefEdge;
    SdrMeasureTextHPos eMH=aMPol.eUsedTextHPos;
    SdrMeasureTextVPos eMV=aMPol.eUsedTextVPos;
    if (!bRota90) {
        switch (eMH) {
            case SDRMEASURE_TEXTLEFTOUTSIDE: aTextPos.X()=aPt1b.X()-aTextSize2.Width()-nArr1Len-nLWdt; break;
            case SDRMEASURE_TEXTRIGHTOUTSIDE: aTextPos.X()=aPt1b.X()+nLen+nArr2Len+nLWdt; break;
            default: aTextPos.X()=aPt1b.X(); aTextSize2.Width()=nLen;
        }
        switch (eMV) {
            case SDRMEASURETEXT_VERTICALCENTERED:
            case SDRMEASURETEXT_BREAKEDLINE: aTextPos.Y()=aPt1b.Y()-aTextSize2.Height()/2; break;
            case SDRMEASURE_BELOW: {
                if (!bUpsideDown) aTextPos.Y()=aPt1b.Y()+nLWdt;
                else aTextPos.Y()=aPt1b.Y()-aTextSize2.Height()-nLWdt;
            } break;
            default: {
                if (!bUpsideDown) aTextPos.Y()=aPt1b.Y()-aTextSize2.Height()-nLWdt;
                else aTextPos.Y()=aPt1b.Y()+nLWdt;
            }
        }
        if (bUpsideDown) {
            aTextPos.X()+=aTextSize2.Width();
            aTextPos.Y()+=aTextSize2.Height();
        }
    } else { // also if bTextRota90==TRUE
        switch (eMH) {
            case SDRMEASURE_TEXTLEFTOUTSIDE: aTextPos.X()=aPt1b.X()-aTextSize2.Height()-nArr1Len; break;
            case SDRMEASURE_TEXTRIGHTOUTSIDE: aTextPos.X()=aPt1b.X()+nLen+nArr2Len; break;
            default: aTextPos.X()=aPt1b.X(); aTextSize2.Height()=nLen;
        }
        switch (eMV) {
            case SDRMEASURETEXT_VERTICALCENTERED:
            case SDRMEASURETEXT_BREAKEDLINE: aTextPos.Y()=aPt1b.Y()+aTextSize2.Width()/2; break;
            case SDRMEASURE_BELOW: {
                if (!bBelowRefEdge) aTextPos.Y()=aPt1b.Y()+aTextSize2.Width()+nLWdt;
                else aTextPos.Y()=aPt1b.Y()-nLWdt;
            } break;
            default: {
                if (!bBelowRefEdge) aTextPos.Y()=aPt1b.Y()-nLWdt;
                else aTextPos.Y()=aPt1b.Y()+aTextSize2.Width()+nLWdt;
            }
        }
        if (bUpsideDown) {
            aTextPos.X()+=aTextSize2.Height();
            aTextPos.Y()-=aTextSize2.Width();
        }
    }
    if (aMPol.nTextWink!=aGeo.nDrehWink) {
        ((SdrMeasureObj*)this)->aGeo.nDrehWink=aMPol.nTextWink;
        ((SdrMeasureObj*)this)->aGeo.RecalcSinCos();
    }
    RotatePoint(aTextPos,aPt1b,aMPol.nLineSin,aMPol.nLineCos);
    aTextSize2.Width()++; aTextSize2.Height()++; // because of the Rect-Ctor's odd behavior
    rRect=Rectangle(aTextPos,aTextSize2);
    rRect.Justify();
    ((SdrMeasureObj*)this)->aRect=rRect;

    if (aMPol.nTextWink!=aGeo.nDrehWink) {
        ((SdrMeasureObj*)this)->aGeo.nDrehWink=aMPol.nTextWink;
        ((SdrMeasureObj*)this)->aGeo.RecalcSinCos();
    }
}

SdrMeasureObj* SdrMeasureObj::Clone() const
{
    return CloneHelper< SdrMeasureObj >();
}

OUString SdrMeasureObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(ImpGetResStr(STR_ObjNameSingulMEASURE));

    OUString aName( GetName() );
    if (!aName.isEmpty())
    {
        sName.append(' ');
        sName.append('\'');
        sName.append(aName);
        sName.append('\'');
    }

    return sName.makeStringAndClear();
}

OUString SdrMeasureObj::TakeObjNamePlural() const
{
    return ImpGetResStr(STR_ObjNamePluralMEASURE);
}

basegfx::B2DPolyPolygon SdrMeasureObj::TakeXorPoly() const
{
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    ImpTakeAttr(aRec);
    ImpCalcGeometrics(aRec,aMPol);
    return ImpCalcXPoly(aMPol);
}

sal_uInt32 SdrMeasureObj::GetHdlCount() const
{
    return 6L;
}

SdrHdl* SdrMeasureObj::GetHdl(sal_uInt32 nHdlNum) const
{
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    ImpTakeAttr(aRec);
    aRec.nHelplineDist=0;
    ImpCalcGeometrics(aRec,aMPol);
    Point aPt;

    switch (nHdlNum) {
        case 0: aPt=aMPol.aHelpline1.aP1; break;
        case 1: aPt=aMPol.aHelpline2.aP1; break;
        case 2: aPt=aPt1;       break;
        case 3: aPt=aPt2;       break;
        case 4: aPt=aMPol.aHelpline1.aP2; break;
        case 5: aPt=aMPol.aHelpline2.aP2; break;
    } // switch
    SdrHdl* pHdl=new ImpMeasureHdl(aPt,HDL_USER);
    pHdl->SetObjHdlNum(nHdlNum);
    pHdl->SetDrehWink(aMPol.nLineWink);
    return pHdl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrMeasureObj::hasSpecialDrag() const
{
    return true;
}

bool SdrMeasureObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl = rDrag.GetHdl();

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
    const SdrHdl* pHdl = rDrag.GetHdl();
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
                        SetObjectItem(SdrMeasureHelpline1LenItem(aMeasureRec.nHelpline1Len));
                    }

                    if(aMeasureRec.nHelpline2Len != aOrigMeasureRec.nHelpline2Len)
                    {
                        SetObjectItem(SdrMeasureHelpline2LenItem(aMeasureRec.nHelpline2Len));
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
                        SetObjectItem(SdrMeasureLineDistItem(aMeasureRec.nLineDist));
                    }

                    if(aMeasureRec.bBelowRefEdge != aOrigMeasureRec.bBelowRefEdge)
                    {
                        SetObjectItem(SdrMeasureBelowRefEdgeItem(aMeasureRec.bBelowRefEdge));
                    }
                }
            }
        }
    } // switch

    SetRectsDirty();
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

    const SdrHdl* pHdl=rDrag.GetHdl();
    sal_uInt32 nHdlNum(pHdl->GetObjHdlNum());
    bool bOrtho=rDrag.GetView()!=NULL && rDrag.GetView()->IsOrtho();
    bool bBigOrtho=bOrtho && rDrag.GetView()->IsBigOrtho();
    bool bBelow=rRec.bBelowRefEdge;
    Point aPt(rDrag.GetNow());

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
                if (!bHLin || !bVLin) { // else aPt1==aPt2
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
    aPt1=rStat.GetStart();
    aPt2=rStat.GetNow();
    SetTextDirty();
    return sal_True;
}

bool SdrMeasureObj::MovCreate(SdrDragStat& rStat)
{
    SdrView* pView=rStat.GetView();
    aPt1=rStat.GetStart();
    aPt2=rStat.GetNow();
    if (pView!=NULL && pView->IsCreate1stPointAsCenter()) {
        aPt1+=aPt1;
        aPt1-=rStat.Now();
    }
    SetTextDirty();
    SetBoundRectDirty();
    bSnapRectDirty=sal_True;
    return sal_True;
}

bool SdrMeasureObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    SetTextDirty();
    SetRectsDirty();
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

Pointer SdrMeasureObj::GetCreatePointer() const
{
    return Pointer(POINTER_CROSS);
}

void SdrMeasureObj::NbcMove(const Size& rSiz)
{
    SdrTextObj::NbcMove(rSiz);
    MovePoint(aPt1,rSiz);
    MovePoint(aPt2,rSiz);
}

void SdrMeasureObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrTextObj::NbcResize(rRef,xFact,yFact);
    ResizePoint(aPt1,rRef,xFact,yFact);
    ResizePoint(aPt2,rRef,xFact,yFact);
    SetTextDirty();
}

void SdrMeasureObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    SdrTextObj::NbcRotate(rRef,nWink,sn,cs);
    long nLen0=GetLen(aPt2-aPt1);
    RotatePoint(aPt1,rRef,sn,cs);
    RotatePoint(aPt2,rRef,sn,cs);
    long nLen1=GetLen(aPt2-aPt1);
    if (nLen1!=nLen0) { // rounding error!
        long dx=aPt2.X()-aPt1.X();
        long dy=aPt2.Y()-aPt1.Y();
        dx=BigMulDiv(dx,nLen0,nLen1);
        dy=BigMulDiv(dy,nLen0,nLen1);
        if (rRef==aPt2) {
            aPt1.X()=aPt2.X()-dx;
            aPt1.Y()=aPt2.Y()-dy;
        } else {
            aPt2.X()=aPt1.X()+dx;
            aPt2.Y()=aPt1.Y()+dy;
        }
    }
    SetRectsDirty();
}

void SdrMeasureObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SdrTextObj::NbcMirror(rRef1,rRef2);
    MirrorPoint(aPt1,rRef1,rRef2);
    MirrorPoint(aPt2,rRef1,rRef2);
    SetRectsDirty();
}

void SdrMeasureObj::NbcShear(const Point& rRef, long nWink, double tn, bool bVShear)
{
    SdrTextObj::NbcShear(rRef,nWink,tn,bVShear);
    ShearPoint(aPt1,rRef,tn,bVShear);
    ShearPoint(aPt2,rRef,tn,bVShear);
    SetRectsDirty();
    SetTextDirty();
}

long SdrMeasureObj::GetRotateAngle() const
{
    return GetAngle(aPt2-aPt1);
}

void SdrMeasureObj::RecalcSnapRect()
{
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    XPolyPolygon aXPP;

    ImpTakeAttr(aRec);
    ImpCalcGeometrics(aRec, aMPol);
    aXPP = XPolyPolygon(ImpCalcXPoly(aMPol));
    maSnapRect = aXPP.GetBoundRect();
}

sal_uInt32 SdrMeasureObj::GetSnapPointCount() const
{
    return 2L;
}

Point SdrMeasureObj::GetSnapPoint(sal_uInt32 i) const
{
    if (i==0) return aPt1;
    else return aPt2;
}

sal_Bool SdrMeasureObj::IsPolyObj() const
{
    return sal_True;
}

sal_uInt32 SdrMeasureObj::GetPointCount() const
{
    return 2L;
}

Point SdrMeasureObj::GetPoint(sal_uInt32 i) const
{
     return (0L == i) ? aPt1 : aPt2;
}

void SdrMeasureObj::NbcSetPoint(const Point& rPnt, sal_uInt32 i)
{
    if (0L == i)
        aPt1=rPnt;
    if (1L == i)
        aPt2=rPnt;
    SetRectsDirty();
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

SdrObject* SdrMeasureObj::DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const
{
    // get XOR Poly as base
    XPolyPolygon aTmpPolyPolygon(TakeXorPoly());

    // get local ItemSet and StyleSheet
    SfxItemSet aSet(GetObjectItemSet());
    SfxStyleSheet* pStyleSheet = GetStyleSheet();

    // prepare group
    SdrObjGroup* pGroup = new SdrObjGroup;
    pGroup->SetModel(GetModel());

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

        pPath = new SdrPathObj(OBJ_PATHLINE, aPolyPoly);
        pPath->SetModel(GetModel());
        pPath->SetMergedItemSet(aSet);
        pPath->SetStyleSheet(pStyleSheet, true);
        pGroup->GetSubList()->NbcInsertObject(pPath);
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
        pPath = new SdrPathObj(OBJ_PATHLINE, aPolyPoly);
        pPath->SetModel(GetModel());
        pPath->SetMergedItemSet(aSet);
        pPath->SetStyleSheet(pStyleSheet, true);

        pGroup->GetSubList()->NbcInsertObject(pPath);

        aSet.Put(XLineEndWidthItem(nEndWidth));
        aSet.Put(XLineStartWidthItem(0L));

        aPolyPoly.clear();
        aPolyPoly.append(aTmpPolyPolygon[1].getB2DPolygon());
        pPath = new SdrPathObj(OBJ_PATHLINE, aPolyPoly);
        pPath->SetModel(GetModel());
        pPath->SetMergedItemSet(aSet);
        pPath->SetStyleSheet(pStyleSheet, true);

        pGroup->GetSubList()->NbcInsertObject(pPath);

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
        pPath = new SdrPathObj(OBJ_PATHLINE, aPolyPoly);
        pPath->SetModel(GetModel());
        pPath->SetMergedItemSet(aSet);
        pPath->SetStyleSheet(pStyleSheet, true);

        pGroup->GetSubList()->NbcInsertObject(pPath);

        aSet.Put(XLineEndWidthItem(nEndWidth));
        aSet.Put(XLineStartWidthItem(0L));

        aPolyPoly.clear();
        aPolyPoly.append(aTmpPolyPolygon[1].getB2DPolygon());
        pPath = new SdrPathObj(OBJ_PATHLINE, aPolyPoly);
        pPath->SetModel(GetModel());
        pPath->SetMergedItemSet(aSet);
        pPath->SetStyleSheet(pStyleSheet, true);

        pGroup->GetSubList()->NbcInsertObject(pPath);

        aSet.Put(XLineEndWidthItem(0L));
        nLoopStart = 2;
    }

    for(;nLoopStart<nCount;nLoopStart++)
    {
        aPolyPoly.clear();
        aPolyPoly.append(aTmpPolyPolygon[nLoopStart].getB2DPolygon());
        pPath = new SdrPathObj(OBJ_PATHLINE, aPolyPoly);
        pPath->SetModel(GetModel());
        pPath->SetMergedItemSet(aSet);
        pPath->SetStyleSheet(pStyleSheet, true);

        pGroup->GetSubList()->NbcInsertObject(pPath);
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

sal_Bool SdrMeasureObj::BegTextEdit(SdrOutliner& rOutl)
{
    UndirtyText();
    return SdrTextObj::BegTextEdit(rOutl);
}

const Size& SdrMeasureObj::GetTextSize() const
{
    if (bTextDirty) UndirtyText();
    return SdrTextObj::GetTextSize();
}

OutlinerParaObject* SdrMeasureObj::GetOutlinerParaObject() const
{
    if(bTextDirty)
        UndirtyText();
    return SdrTextObj::GetOutlinerParaObject();
}

void SdrMeasureObj::NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject)
{
    SdrTextObj::NbcSetOutlinerParaObject(pTextObject);
    if(SdrTextObj::GetOutlinerParaObject())
        SetTextDirty(); // recalculate text
}

void SdrMeasureObj::TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, bool bNoEditText,
    Rectangle* pAnchorRect, bool bLineWidth ) const
{
    if (bTextDirty) UndirtyText();
    SdrTextObj::TakeTextRect( rOutliner, rTextRect, bNoEditText, pAnchorRect, bLineWidth );
}

void SdrMeasureObj::TakeTextAnchorRect(Rectangle& rAnchorRect) const
{
    if (bTextDirty) UndirtyText();
    SdrTextObj::TakeTextAnchorRect(rAnchorRect);
}

void SdrMeasureObj::TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const
{
    if (bTextDirty) UndirtyText();
    SdrTextObj::TakeTextEditArea(pPaperMin,pPaperMax,pViewInit,pViewMin);
}

sal_uInt16 SdrMeasureObj::GetOutlinerViewAnchorMode() const
{
    if (bTextDirty) UndirtyText();
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

    // TODO: bTextUpsideDown should be interpreted here!
    if (!bTextRota90) {
        if (eMH==SDRMEASURE_TEXTLEFTOUTSIDE) eTH=SDRTEXTHORZADJUST_RIGHT;
        if (eMH==SDRMEASURE_TEXTRIGHTOUTSIDE) eTH=SDRTEXTHORZADJUST_LEFT;
        // at eMH==SDRMEASURE_TEXTINSIDE we can anchor horizontally
        if (eMV==SDRMEASURE_ABOVE) eTV=SDRTEXTVERTADJUST_BOTTOM;
        if (eMV==SDRMEASURE_BELOW) eTV=SDRTEXTVERTADJUST_TOP;
        if (eMV==SDRMEASURETEXT_BREAKEDLINE || eMV==SDRMEASURETEXT_VERTICALCENTERED) eTV=SDRTEXTVERTADJUST_CENTER;
    } else {
        if (eMH==SDRMEASURE_TEXTLEFTOUTSIDE) eTV=SDRTEXTVERTADJUST_BOTTOM;
        if (eMH==SDRMEASURE_TEXTRIGHTOUTSIDE) eTV=SDRTEXTVERTADJUST_TOP;
        // at eMH==SDRMEASURE_TEXTINSIDE we can anchor vertically
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
// #i97878#
// TRGetBaseGeometry/TRSetBaseGeometry needs to be based on two positions,
// same as line geometry in SdrPathObj. Thus needs to be overloaded and
// implemented since currently it is derived from SdrTextObj which uses
// a functionality based on SnapRect which is not useful here

sal_Bool SdrMeasureObj::TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& /*rPolyPolygon*/) const
{
    // handle the same as a simple line since the definition is based on two points
    const basegfx::B2DRange aRange(aPt1.X(), aPt1.Y(), aPt2.X(), aPt2.Y());
    basegfx::B2DTuple aScale(aRange.getRange());
    basegfx::B2DTuple aTranslate(aRange.getMinimum());

    // position maybe relative to anchor position, convert
    if( pModel->IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate -= basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // force MapUnit to 100th mm
    SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // position
                aTranslate.setX(ImplTwipsToMM(aTranslate.getX()));
                aTranslate.setY(ImplTwipsToMM(aTranslate.getY()));

                // size
                aScale.setX(ImplTwipsToMM(aScale.getX()));
                aScale.setY(ImplTwipsToMM(aScale.getY()));

                break;
            }
            default:
            {
                OSL_FAIL("TRGetBaseGeometry: Missing unit translation to 100th mm!");
            }
        }
    }

    // build return value matrix
    rMatrix = basegfx::tools::createScaleTranslateB2DHomMatrix(aScale, aTranslate);

    return sal_True;
}

void SdrMeasureObj::TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& /*rPolyPolygon*/)
{
    // use given transformation to derive the two defining points from unit line
    basegfx::B2DPoint aPosA(rMatrix * basegfx::B2DPoint(0.0, 0.0));
    basegfx::B2DPoint aPosB(rMatrix * basegfx::B2DPoint(1.0, 0.0));

    // force metric to pool metric
    SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // position
                aPosA.setX(ImplMMToTwips(aPosA.getX()));
                aPosA.setY(ImplMMToTwips(aPosA.getY()));
                aPosB.setX(ImplMMToTwips(aPosB.getX()));
                aPosB.setY(ImplMMToTwips(aPosB.getY()));

                break;
            }
            default:
            {
                OSL_FAIL("TRSetBaseGeometry: Missing unit translation to PoolMetric!");
            }
        }
    }

    if( pModel->IsWriter() )
    {
        // if anchor is used, make position relative to it
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            const basegfx::B2DVector aAnchorOffset(GetAnchorPos().X(), GetAnchorPos().Y());

            aPosA += aAnchorOffset;
            aPosB += aAnchorOffset;
        }
    }

    // derive new model data
    const Point aNewPt1(basegfx::fround(aPosA.getX()), basegfx::fround(aPosA.getY()));
    const Point aNewPt2(basegfx::fround(aPosB.getX()), basegfx::fround(aPosB.getY()));

    if(aNewPt1 != aPt1 || aNewPt2 != aPt2)
    {
        // set model values and broadcast
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();

        aPt1 = aNewPt1;
        aPt2 = aNewPt2;

        SetTextDirty();
        ActionChanged();
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
