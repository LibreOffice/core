/*************************************************************************
 *
 *  $RCSfile: svdomeas.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: dl $ $Date: 2001-03-28 08:06:48 $
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

#include "svdomeas.hxx"
#include <math.h>
#include "svditext.hxx" // enthaelt u.a. define ITEMID_FIELD fuer include flditem
#include "xpoly.hxx"
#include "xoutx.hxx"
#include "svdtrans.hxx"
#include "svdtouch.hxx"
#include "svdhdl.hxx"
#include "svdoutl.hxx"
#include "svddrag.hxx"
#include "svdpool.hxx"
#include "svdattrx.hxx"
#include "svdmodel.hxx"
#include "svdio.hxx"
#include "svdview.hxx"
#include "svdglob.hxx"   // StringCache
#include "svdstr.hrc"    // Objektname

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif

#ifndef _EEITEM_HXX //autogen
#include <eeitem.hxx>
#endif

#ifndef _SVX_XLNSTIT_HXX //autogen
#include "xlnstit.hxx"
#endif

#ifndef _SVX_XLNSTWIT_HXX //autogen
#include "xlnstwit.hxx"
#endif

#ifndef _SVX_XLNEDIT_HXX //autogen
#include "xlnedit.hxx"
#endif

#ifndef _SVX_XLNWTIT_HXX //autogen
#include "xlnwtit.hxx"
#endif

#ifndef _SVX_XLNEDWIT_HXX //autogen
#include "xlnedwit.hxx"
#endif

#ifndef _SVX_XLNSTCIT_HXX //autogen
#include "xlnstcit.hxx"
#endif

#ifndef _SVX_XLNEDCIT_HXX //autogen
#include "xlnedcit.hxx"
#endif

#ifndef _OUTLOBJ_HXX //autogen
#include <outlobj.hxx>
#endif

#ifndef _OUTLINER_HXX //autogen
#include "outliner.hxx"
#endif

#ifndef _EDITOBJ_HXX //autogen
#include <editobj.hxx>
#endif

#include "svdfield.hxx"
#include "flditem.hxx"

#include "svdogrp.hxx"
#include "svdopath.hxx"
#include "svdpage.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrMeasureObjGeoData::SdrMeasureObjGeoData() {}
SdrMeasureObjGeoData::~SdrMeasureObjGeoData() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

SV_IMPL_PERSIST1(SdrMeasureField,SvxFieldData);

__EXPORT SdrMeasureField::~SdrMeasureField()
{
}

SvxFieldData* __EXPORT SdrMeasureField::Clone() const
{
    return new SdrMeasureField(*this);
}

int __EXPORT SdrMeasureField::operator==(const SvxFieldData& rSrc) const
{
    return eMeasureFieldKind==((SdrMeasureField&)rSrc).GetMeasureFieldKind();
}

void __EXPORT SdrMeasureField::Load(SvPersistStream& rIn)
{
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrMeasureField");
#endif
    UINT16 nFieldKind;
    rIn>>nFieldKind;
    eMeasureFieldKind=(SdrMeasureFieldKind)nFieldKind;
}

void __EXPORT SdrMeasureField::Save(SvPersistStream& rOut)
{
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrMeasureField");
#endif
    rOut<<(UINT16)eMeasureFieldKind;
}

void SdrMeasureField::TakeRepresentation(const SdrMeasureObj& rObj, XubString& rStr) const
{
    rStr.Erase();
    Fraction aMeasureScale(1, 1);
    BOOL bTextRota90(FALSE);
    BOOL bShowUnit(FALSE);
    FieldUnit eMeasureUnit(FUNIT_NONE);
    FieldUnit eModUIUnit(FUNIT_NONE);

    const SfxItemSet& rSet = rObj.GetItemSet();
    bTextRota90 = ((SdrMeasureTextRota90Item&)rSet.Get(SDRATTR_MEASURETEXTROTA90)).GetValue();
    eMeasureUnit = ((SdrMeasureUnitItem&)rSet.Get(SDRATTR_MEASUREUNIT)).GetValue();
    aMeasureScale = ((SdrMeasureScaleItem&)rSet.Get(SDRATTR_MEASURESCALE)).GetValue();
    bShowUnit = ((SdrMeasureShowUnitItem&)rSet.Get(SDRATTR_MEASURESHOWUNIT)).GetValue();

    SdrModel* pModel = rObj.pModel;

    switch(eMeasureFieldKind)
    {
        case SDRMEASUREFIELD_VALUE:
        {
            if(pModel)
            {
                eModUIUnit = pModel->GetUIUnit();

                if(eMeasureUnit == FUNIT_NONE)
                    eMeasureUnit = eModUIUnit;

                INT32 nLen(GetLen(rObj.aPt2 - rObj.aPt1));
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

                pModel->TakeMetricStr(nLen, rStr, TRUE);

                if(!aFact.IsValid())
                {
                    rStr = String();
                    rStr += sal_Unicode('?');
                }

                International aInter(GetpApp()->GetAppInternational());
                sal_Unicode cDec(aInter.GetNumDecimalSep());

                if(rStr.Search(cDec) != STRING_NOTFOUND)
                {
                    xub_StrLen nLen(rStr.Len() - 1);

                    while(rStr.GetChar(nLen) == sal_Unicode('0'))
                    {
                        rStr.Erase(nLen);
                        nLen--;
                    }

                    if(rStr.GetChar(nLen) == cDec)
                    {
                        rStr.Erase(nLen);
                        nLen--;
                    }

                    if(!rStr.Len())
                        rStr += sal_Unicode('0');
                }
            }
            else
            {
                // falls kein Model da ... (z.B. Preview im Dialog)
                rStr = String();
                rStr.AppendAscii("4711");
            }

            break;
        }
        case SDRMEASUREFIELD_UNIT:
        {
            if(bShowUnit)
            {
                if(rObj.pModel)
                {
                    eModUIUnit = pModel->GetUIUnit();

                    if(eMeasureUnit == FUNIT_NONE)
                        eMeasureUnit = eModUIUnit;

                    if(bShowUnit)
                        pModel->TakeUnitStr(eMeasureUnit, rStr);
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

TYPEINIT1(SdrMeasureObj,SdrTextObj);

void SdrMeasureObj::ForceDefaultAttr()
{
    SdrTextObj::ForceDefaultAttr();

    //#71958# by default, the show units Bool-Item is set as hard
    // attribute to TRUE to aviod confusion when copying SdrMeasureObj's
    // from one application to another
    ImpForceItemSet();
    mpObjectItemSet->Put(SdrMeasureShowUnitItem(TRUE));

    XPolygon aXP(4);        //      []
    aXP[0] = Point(100,0);    // 0,4__[]__2,4
    aXP[1] = Point(200,400);  //    \    /
    aXP[2] = Point(0,400);    //     \  /
    aXP[3] = Point(100,0);    //      \/1,0

    mpObjectItemSet->Put(XLineStartItem(String(), aXP));
    mpObjectItemSet->Put(XLineStartWidthItem(200));
    mpObjectItemSet->Put(XLineEndItem(String(), aXP));
    mpObjectItemSet->Put(XLineEndWidthItem(200));
    mpObjectItemSet->Put(XLineStyleItem(XLINE_SOLID));
}

SdrMeasureObj::SdrMeasureObj():
    bTextDirty(FALSE)
{
}

SdrMeasureObj::SdrMeasureObj(const Point& rPt1, const Point& rPt2):
    aPt1(rPt1),
    aPt2(rPt2),
    bTextDirty(FALSE)
{
}

SdrMeasureObj::~SdrMeasureObj()
{
}

void SdrMeasureObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bSelectAllowed    =TRUE;
    rInfo.bMoveAllowed      =TRUE;
    rInfo.bResizeFreeAllowed=TRUE;
    rInfo.bResizePropAllowed=TRUE;
    rInfo.bRotateFreeAllowed=TRUE;
    rInfo.bRotate90Allowed  =TRUE;
    rInfo.bMirrorFreeAllowed=TRUE;
    rInfo.bMirror45Allowed  =TRUE;
    rInfo.bMirror90Allowed  =TRUE;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed     =TRUE;
    rInfo.bEdgeRadiusAllowed=FALSE;
    rInfo.bNoOrthoDesired   =TRUE;
    rInfo.bNoContortion     =FALSE;
    rInfo.bCanConvToPath    =FALSE;
    rInfo.bCanConvToPoly    =TRUE;
    rInfo.bCanConvToPathLineToArea=FALSE;
    rInfo.bCanConvToPolyLineToArea=FALSE;
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

UINT16 SdrMeasureObj::GetObjIdentifier() const
{
    return (UINT16)OBJ_MEASURE;
}

struct ImpMeasureRec
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
    FASTBOOL                    bBelowRefEdge;
    FASTBOOL                    bTextRota90;
    FASTBOOL                    bTextUpsideDown;
    long                        nMeasureOverhang;
    FieldUnit                   eMeasureUnit;
    Fraction                    aMeasureScale;
    FASTBOOL                    bShowUnit;
    String                      aFormatString;
    FASTBOOL                    bTextAutoAngle;
    long                        nTextAutoAngleView;
    FASTBOOL                    bTextIsFixedAngle;
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
    USHORT                      nMainlineAnz;
    SdrMeasureTextHPos          eUsedTextHPos;
    SdrMeasureTextVPos          eUsedTextVPos;
    long                        nLineWdt2;  // Halbe Strichstaerke
    long                        nArrow1Len; // Laenge des 1. Pfeils. Bei Center nur die Haelfte
    long                        nArrow2Len; // Laenge des 2. Pfeils. Bei Center nur die Haelfte
    long                        nArrow1Wdt; // Breite des 1. Pfeils
    long                        nArrow2Wdt; // Breite des 2. Pfeils
    long                        nShortLineLen; // Linienlaenge, wenn PfeileAussen
    FASTBOOL                    bArrow1Center; // Pfeil 1 zentriert?
    FASTBOOL                    bArrow2Center; // Pfeil 2 zentriert?
    FASTBOOL                    bAutoUpsideDown; // UpsideDown durch Automatik
    FASTBOOL                    bPfeileAussen;
    FASTBOOL                    bBreakedLine;
};

void SdrMeasureObj::ImpTakeAttr(ImpMeasureRec& rRec) const
{
    rRec.aPt1 = aPt1;
    rRec.aPt2 = aPt2;

    const SfxItemSet& rSet = GetItemSet();
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

void SdrMeasureObj::ImpCalcGeometrics(const ImpMeasureRec& rRec, ImpMeasurePoly& rPol) const
{
    Point aP1(rRec.aPt1);
    Point aP2(rRec.aPt2);
    Point aDelt(aP2); aDelt-=aP1;

    rPol.aTextSize=GetTextSize();
    rPol.nLineLen=GetLen(aDelt);

    rPol.nLineWdt2=0;
    long nArrow1Len=0; FASTBOOL bArrow1Center=FALSE;
    long nArrow2Len=0; FASTBOOL bArrow2Center=FALSE;
    long nArrow1Wdt=0;
    long nArrow2Wdt=0;
    rPol.nArrow1Wdt=0;
    rPol.nArrow2Wdt=0;
    long nArrowNeed=0;
    long nShortLen=0;
    FASTBOOL bPfeileAussen=FALSE;

    const SfxItemSet& rSet = GetItemSet();
    sal_Int32 nLineWdt = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue(); // Strichstaerke
    rPol.nLineWdt2 = (nLineWdt + 1) / 2;

    nArrow1Wdt = ((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
    if(nArrow1Wdt < 0)
        nArrow1Wdt = -nLineWdt * nArrow1Wdt / 100; // <0 = relativ

    nArrow2Wdt = ((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();
    if(nArrow2Wdt < 0)
        nArrow2Wdt = -nLineWdt * nArrow2Wdt / 100; // <0 = relativ

    XPolygon aPol1(((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetValue());
    XPolygon aPol2(((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetValue());
    bArrow1Center = ((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
    bArrow2Center = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
    nArrow1Len = XOutputDevice::InitLineStartEnd(aPol1, nArrow1Wdt, bArrow1Center) - 1;
    nArrow2Len = XOutputDevice::InitLineStartEnd(aPol2, nArrow2Wdt, bArrow2Center) - 1;

    // nArrowLen ist bei bCenter bereits halbiert
    // Bei 2 Pfeilen a 4mm ist unter 10mm Schluss.
    nArrowNeed=nArrow1Len+nArrow2Len+(nArrow1Wdt+nArrow2Wdt)/2;
    if (rPol.nLineLen<nArrowNeed) bPfeileAussen=TRUE;
    nShortLen=(nArrow1Len+nArrow1Wdt + nArrow2Len+nArrow2Wdt) /2;

    rPol.eUsedTextHPos=rRec.eWantTextHPos;
    rPol.eUsedTextVPos=rRec.eWantTextVPos;
    if (rPol.eUsedTextVPos==SDRMEASURE_TEXTVAUTO) rPol.eUsedTextVPos=SDRMEASURE_ABOVE;
    FASTBOOL bBrkLine=rPol.eUsedTextVPos==SDRMEASURETEXT_BREAKEDLINE;
    if (rPol.eUsedTextVPos==SDRMEASURETEXT_VERTICALCENTERED) {
        if (pOutlinerParaObject!=NULL && pOutlinerParaObject->GetTextObject().GetParagraphCount()==1) {
            bBrkLine=TRUE; // Unterbrochene Linie, wenn nur 1 Absatz.
        }
    }
    rPol.bBreakedLine=bBrkLine;
    if (rPol.eUsedTextHPos==SDRMEASURE_TEXTHAUTO) { // bei zu breitem Text diesen eventuell nach aussen schieben
        FASTBOOL bOutside=FALSE;
        long nNeedSiz=!rRec.bTextRota90 ? rPol.aTextSize.Width() : rPol.aTextSize.Height();
        if (nNeedSiz>rPol.nLineLen) bOutside=TRUE; // Text passt nicht in die Mitte
        if (bBrkLine) {
            if (nNeedSiz+nArrowNeed>rPol.nLineLen) bPfeileAussen=TRUE; // Text passt in die Mitte, wenn die Pfeile nach aussen kommen
        } else {
            long nSmallNeed=nArrow1Len+nArrow2Len+(nArrow1Wdt+nArrow2Wdt)/2/4;
            if (nNeedSiz+nSmallNeed>rPol.nLineLen) bPfeileAussen=TRUE; // Text passt in die Mitte, wenn die Pfeile nach aussen kommen
        }
        rPol.eUsedTextHPos=bOutside ? SDRMEASURE_TEXTLEFTOUTSIDE : SDRMEASURE_TEXTINSIDE;
    }
    if (rPol.eUsedTextHPos!=SDRMEASURE_TEXTINSIDE) bPfeileAussen=TRUE;
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

    rPol.bAutoUpsideDown=FALSE;
    if (rRec.bTextAutoAngle) {
        long nTmpWink=NormAngle360(rPol.nTextWink-rRec.nTextAutoAngleView);
        if (nTmpWink>=18000) {
            rPol.nTextWink+=18000;
            rPol.bAutoUpsideDown=TRUE;
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

void SdrMeasureObj::ImpCalcXPoly(const ImpMeasurePoly& rPol, XPolyPolygon& rXPP) const
{
    rXPP.Clear();
    XPolygon aXP(2);
    aXP[0]=rPol.aMainline1.aP1;
    aXP[1]=rPol.aMainline1.aP2;
    rXPP.Insert(aXP);
    if (rPol.nMainlineAnz>1) {
        aXP[0]=rPol.aMainline2.aP1;
        aXP[1]=rPol.aMainline2.aP2;
        rXPP.Insert(aXP);
    }
    if (rPol.nMainlineAnz>2) {
        aXP[0]=rPol.aMainline3.aP1;
        aXP[1]=rPol.aMainline3.aP2;
        rXPP.Insert(aXP);
    }
    aXP[0]=rPol.aHelpline1.aP1;
    aXP[1]=rPol.aHelpline1.aP2;
    rXPP.Insert(aXP);
    aXP[0]=rPol.aHelpline2.aP1;
    aXP[1]=rPol.aHelpline2.aP2;
    rXPP.Insert(aXP);
}

FASTBOOL SdrMeasureObj::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
{
    // Hidden objects on masterpages, draw nothing
    if((rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE) && bNotVisibleAsMaster)
        return TRUE;

    // prepare ItemSet of this object
    const SfxItemSet& rSet = GetItemSet();

    // perepare ItemSet to avoid old XOut line drawing
    SfxItemSet aEmptySet(*rSet.GetPool());
    aEmptySet.Put(XLineStyleItem(XLINE_NONE));

    // prepare line geometry
    BOOL bIsLineDraft(0 != (rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTLINE));
    ImpLineGeometry* pLineGeometry = ImpPrepareLineGeometry(rXOut, rSet, bIsLineDraft);

    // Shadows
    BOOL bShadOn = ((SdrShadowItem&)(rSet.Get(SDRATTR_SHADOW))).GetValue();
    if(bShadOn && pLineGeometry)
    {
        // draw the line geometry
        ImpDrawShadowLineGeometry(rXOut, rSet, *pLineGeometry);
    }

    // Before here the LineAttr were set: if(pLineAttr) rXOut.SetLineAttr(*pLineAttr);
    rXOut.SetLineAttr(aEmptySet);

    // !!! aGeo muss noch fuer Textausgabe gesetzt werden !!!
    // aRect ebenso
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    ImpTakeAttr(aRec);
    ImpCalcGeometrics(aRec,aMPol);
    FASTBOOL bMerk1=rXOut.IsLineStart();
    FASTBOOL bMerk2=rXOut.IsLineEnd();
    if (aMPol.nMainlineAnz>1) {
        // Je 1 Linienende temporaer abschalten
        rXOut.OverrideLineEnd(FALSE);
        rXOut.DrawLine(aMPol.aMainline1.aP1,aMPol.aMainline1.aP2);
        rXOut.OverrideLineEnd(bMerk2);
        rXOut.OverrideLineStart(FALSE);
        rXOut.DrawLine(aMPol.aMainline2.aP1,aMPol.aMainline2.aP2);
        rXOut.OverrideLineStart(bMerk1);
    } else {
        rXOut.DrawLine(aMPol.aMainline1.aP1,aMPol.aMainline1.aP2);
    }
    rXOut.OverrideLineStart(FALSE);
    rXOut.OverrideLineEnd(FALSE);
    if (aMPol.nMainlineAnz>2) {
        rXOut.DrawLine(aMPol.aMainline3.aP1,aMPol.aMainline3.aP2);
    }
    rXOut.DrawLine(aMPol.aHelpline1.aP1,aMPol.aHelpline1.aP2);
    rXOut.DrawLine(aMPol.aHelpline2.aP1,aMPol.aHelpline2.aP2);

    // Own line drawing
    if(pLineGeometry)
    {
        // draw the line geometry
        ImpDrawColorLineGeometry(rXOut, rSet, *pLineGeometry);
    }

    FASTBOOL bOk=TRUE;
    if (bTextDirty) UndirtyText();
    bOk=SdrTextObj::Paint(rXOut,rInfoRec);
    if (bOk && (rInfoRec.nPaintMode & SDRPAINTMODE_GLUEPOINTS) !=0) {
        bOk=PaintGluePoints(rXOut,rInfoRec);
    }

    // throw away line geometry
    if(pLineGeometry)
        delete pLineGeometry;

    return bOk;
}

FASTBOOL SdrMeasureObj::CalcFieldValue(const SvxFieldItem& rField, USHORT nPara, USHORT nPos,
    FASTBOOL bEdit,
    Color*& rpTxtColor, Color*& rpFldColor, XubString& rRet) const
{
    const SvxFieldData* pField=rField.GetField();
    SdrMeasureField* pMeasureField=PTR_CAST(SdrMeasureField,pField);
    if (pMeasureField!=NULL) {
        pMeasureField->TakeRepresentation(*this,rRet);
        if (rpFldColor!=NULL) {
            if (!bEdit)
            {
                delete rpFldColor;
                rpFldColor=NULL;
            }
        }
        return TRUE;
    } else {
        return SdrTextObj::CalcFieldValue(rField,nPara,nPos,bEdit,rpTxtColor,rpFldColor,rRet);
    }
}

void SdrMeasureObj::UndirtyText() const
{
    if (bTextDirty) {
        SdrOutliner& rOutliner=ImpGetDrawOutliner();
        if (pOutlinerParaObject==NULL) {
            rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_ROTA90BLANCS)),ESelection(0,0));
            rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_VALUE)),ESelection(0,1));
            rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_UNIT)),ESelection(0,2));
            rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_ROTA90BLANCS)),ESelection(0,3));

            if(GetStyleSheet())
                rOutliner.SetStyleSheet(0, GetStyleSheet());

            rOutliner.SetParaAttribs(0, GetItemSet());

            // casting auf nonconst
            ((SdrMeasureObj*)this)->pOutlinerParaObject=rOutliner.CreateParaObject();
        } else {
            rOutliner.SetText(*pOutlinerParaObject);
        }
        rOutliner.SetUpdateMode(TRUE);
        rOutliner.UpdateFields();
        Size aSiz(rOutliner.CalcTextSize());
        rOutliner.Clear();
        // 3x casting auf nonconst
        ((SdrMeasureObj*)this)->aTextSize=aSiz;
        ((SdrMeasureObj*)this)->bTextSizeDirty=FALSE;
        ((SdrMeasureObj*)this)->bTextDirty=FALSE;
    }
}

void SdrMeasureObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
{
    if (bTextDirty) UndirtyText();
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    ImpTakeAttr(aRec);
    ImpCalcGeometrics(aRec,aMPol);

    // TextSize ermitteln inkl. Textrahmenabstaende
    Size aTextSize(aMPol.aTextSize);
    if (aTextSize.Width()<1) aTextSize.Width()=1;
    if (aTextSize.Height()<1) aTextSize.Height()=1;
    aTextSize.Width()+=GetTextLeftDistance()+GetTextRightDistance();
    aTextSize.Height()+=GetTextUpperDistance()+GetTextLowerDistance();

    Point aPt1(aMPol.aMainline1.aP1);
    long nLen=aMPol.nLineLen;
    long nLWdt=aMPol.nLineWdt2;
    long nArr1Len=aMPol.nArrow1Len;
    long nArr2Len=aMPol.nArrow2Len;
    if (aMPol.bBreakedLine) {
        // Bei Unterbrochener Linie und Outside muss der Text nicht neben den
        // Pfeil sondern neben die Linie an dem Pfeil plaziert werden
        nArr1Len=aMPol.nShortLineLen+aMPol.nArrow1Wdt/4;
        nArr2Len=aMPol.nShortLineLen+aMPol.nArrow2Wdt/4;
    }

    Point aTextPos;
    FASTBOOL bRota90=aRec.bTextRota90;
    FASTBOOL bUpsideDown=aRec.bTextUpsideDown!=aMPol.bAutoUpsideDown;
    FASTBOOL bBelowRefEdge=aRec.bBelowRefEdge;
    SdrMeasureTextHPos eMH=aMPol.eUsedTextHPos;
    SdrMeasureTextVPos eMV=aMPol.eUsedTextVPos;
    if (!bRota90) {
        switch (eMH) {
            case SDRMEASURE_TEXTLEFTOUTSIDE: aTextPos.X()=aPt1.X()-aTextSize.Width()-nArr1Len-nLWdt; break;
            case SDRMEASURE_TEXTRIGHTOUTSIDE: aTextPos.X()=aPt1.X()+nLen+nArr2Len+nLWdt; break;
            default: aTextPos.X()=aPt1.X(); aTextSize.Width()=nLen;
        }
        switch (eMV) {
            case SDRMEASURETEXT_VERTICALCENTERED:
            case SDRMEASURETEXT_BREAKEDLINE: aTextPos.Y()=aPt1.Y()-aTextSize.Height()/2; break;
            case SDRMEASURE_BELOW: {
                if (!bUpsideDown) aTextPos.Y()=aPt1.Y()+nLWdt;
                else aTextPos.Y()=aPt1.Y()-aTextSize.Height()-nLWdt;
            } break;
            default: {
                if (!bUpsideDown) aTextPos.Y()=aPt1.Y()-aTextSize.Height()-nLWdt;
                else aTextPos.Y()=aPt1.Y()+nLWdt;
            }
        }
        if (bUpsideDown) {
            aTextPos.X()+=aTextSize.Width();
            aTextPos.Y()+=aTextSize.Height();
        }
    } else { // also wenn bTextRota90==TRUE
        switch (eMH) {
            case SDRMEASURE_TEXTLEFTOUTSIDE: aTextPos.X()=aPt1.X()-aTextSize.Height()-nArr1Len; break;
            case SDRMEASURE_TEXTRIGHTOUTSIDE: aTextPos.X()=aPt1.X()+nLen+nArr2Len; break;
            default: aTextPos.X()=aPt1.X(); aTextSize.Height()=nLen;
        }
        switch (eMV) {
            case SDRMEASURETEXT_VERTICALCENTERED:
            case SDRMEASURETEXT_BREAKEDLINE: aTextPos.Y()=aPt1.Y()+aTextSize.Width()/2; break;
            case SDRMEASURE_BELOW: {
                if (!bBelowRefEdge) aTextPos.Y()=aPt1.Y()+aTextSize.Width()+nLWdt;
                else aTextPos.Y()=aPt1.Y()-nLWdt;
            } break;
            default: {
                if (!bBelowRefEdge) aTextPos.Y()=aPt1.Y()-nLWdt;
                else aTextPos.Y()=aPt1.Y()+aTextSize.Width()+nLWdt;
            }
        }
        if (bUpsideDown) {
            aTextPos.X()+=aTextSize.Height();
            aTextPos.Y()-=aTextSize.Width();
        }
    }
    if (aMPol.nTextWink!=aGeo.nDrehWink) {
        ((SdrMeasureObj*)this)->aGeo.nDrehWink=aMPol.nTextWink;
        ((SdrMeasureObj*)this)->aGeo.RecalcSinCos();
    }
    RotatePoint(aTextPos,aPt1,aMPol.nLineSin,aMPol.nLineCos);
    aTextSize.Width()++; aTextSize.Height()++; // wg. des komischen Verhaltens beim Rect-Ctor
    rRect=Rectangle(aTextPos,aTextSize);
    rRect.Justify();
    ((SdrMeasureObj*)this)->aRect=rRect;

    if (aMPol.nTextWink!=aGeo.nDrehWink) {
        ((SdrMeasureObj*)this)->aGeo.nDrehWink=aMPol.nTextWink;
        ((SdrMeasureObj*)this)->aGeo.RecalcSinCos();
    }
}

SdrObject* SdrMeasureObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    FASTBOOL bHit=FALSE;
    if (pVisiLayer!=NULL && !pVisiLayer->IsSet(nLayerId)) return NULL;
    INT32 nMyTol=nTol;
    INT32 nWdt=ImpGetLineWdt()/2; // Halbe Strichstaerke
    if (nWdt>nMyTol) nMyTol=nWdt; // Bei dicker Linie keine Toleranz noetig
    Rectangle aR(rPnt,rPnt);
    aR.Left()  -=nMyTol;
    aR.Right() +=nMyTol;
    aR.Top()   -=nMyTol;
    aR.Bottom()+=nMyTol;

    if (bTextDirty) UndirtyText();
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    ImpTakeAttr(aRec);
    ImpCalcGeometrics(aRec,aMPol);
    bHit=IsRectTouchesLine(aMPol.aMainline1.aP1,aMPol.aMainline1.aP2,aR) ||
         IsRectTouchesLine(aMPol.aMainline2.aP1,aMPol.aMainline2.aP2,aR) ||
         IsRectTouchesLine(aMPol.aHelpline1.aP1,aMPol.aHelpline1.aP2,aR) ||
         IsRectTouchesLine(aMPol.aHelpline2.aP1,aMPol.aHelpline2.aP2,aR);
    // und nun noch ggf. den Textbereich checken
    bHit=bHit || SdrTextObj::CheckHit(rPnt,nTol,pVisiLayer)!=NULL;
    return bHit ? (SdrObject*)this : NULL;
}

void SdrMeasureObj::operator=(const SdrObject& rObj)
{
    SdrTextObj::operator=(rObj);
    aPt1=((SdrMeasureObj&)rObj).aPt1;
    aPt2=((SdrMeasureObj&)rObj).aPt2;
    bTextDirty=((SdrMeasureObj&)rObj).bTextDirty;
}

void SdrMeasureObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulMEASURE);
}

void SdrMeasureObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralMEASURE);
}

void SdrMeasureObj::TakeXorPoly(XPolyPolygon& rXPP, FASTBOOL bDetail) const
{
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    ImpTakeAttr(aRec);
    ImpCalcGeometrics(aRec,aMPol);
    ImpCalcXPoly(aMPol,rXPP);
}

void SdrMeasureObj::TakeContour(XPolyPolygon& rXPoly, SdrContourType eType) const
{
}

USHORT SdrMeasureObj::GetHdlCount() const
{
    return 6;
}

SdrHdl* SdrMeasureObj::GetHdl(USHORT nHdlNum) const
{
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    ImpTakeAttr(aRec);
    aRec.nHelplineDist=0;
    ImpCalcGeometrics(aRec,aMPol);
    Point aPt;
    SdrHdlKind eHdl=HDL_POLY;
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

void SdrMeasureObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    SdrTextObj::AddToHdlList(rHdlList);
}

FASTBOOL SdrMeasureObj::HasSpecialDrag() const
{
    return TRUE;
}

FASTBOOL SdrMeasureObj::BegDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl=rDrag.GetHdl();
    if (pHdl!=NULL) {
        USHORT nHdlNum=pHdl->GetObjHdlNum();
        if (nHdlNum!=2 && nHdlNum!=3) {
            rDrag.SetEndDragChangesAttributes(TRUE);
        }
        ImpMeasureRec* pMR=new ImpMeasureRec; // #48544#
        ImpTakeAttr(*pMR);
        rDrag.SetUser(pMR);
    }
    return pHdl!=NULL;
}

FASTBOOL SdrMeasureObj::MovDrag(SdrDragStat& rDrag) const
{
    ImpMeasureRec* pMR=(ImpMeasureRec*)rDrag.GetUser();
    if (pMR!=NULL) { // #48544#
        ImpTakeAttr(*pMR);
        ImpEvalDrag(*pMR,rDrag);
    }
    return TRUE;
}

FASTBOOL SdrMeasureObj::EndDrag(SdrDragStat& rDrag)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    ImpMeasureRec* pMR=(ImpMeasureRec*)rDrag.GetUser(); // #48544#
    ImpMeasureRec aRec0;
    ImpTakeAttr(aRec0);
    const SdrHdl* pHdl=rDrag.GetHdl();
    USHORT nHdlNum=pHdl->GetObjHdlNum();
    switch (nHdlNum) {
        case 2: aPt1=pMR->aPt1; SetTextDirty(); break;
        case 3: aPt2=pMR->aPt2; SetTextDirty(); break;
        default:
        {
            switch(nHdlNum)
            {
                case 0:
                case 1:
                {
                    if(pMR->nHelpline1Len!=aRec0.nHelpline1Len)
                    {
                        SetItem(SdrMeasureHelpline1LenItem(pMR->nHelpline1Len));
                    }

                    if(pMR->nHelpline2Len!=aRec0.nHelpline2Len)
                    {
                        SetItem(SdrMeasureHelpline2LenItem(pMR->nHelpline2Len));
                    }

                    break;
                }

                case 4:
                case 5:
                {
                    if (pMR->nLineDist!=aRec0.nLineDist)
                    {
                        SetItem(SdrMeasureLineDistItem(pMR->nLineDist));
                    }

                    if(pMR->bBelowRefEdge!=aRec0.bBelowRefEdge)
                    {
                        SetItem(SdrMeasureBelowRefEdgeItem(pMR->bBelowRefEdge));
                    }
                }
            }
        }
    } // switch
    SetRectsDirty();
    SendRepaintBroadcast();
    if (pMR!=NULL) {
        delete pMR; // #48544#
        rDrag.SetUser(NULL);
    }
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    return TRUE;
}

void SdrMeasureObj::BrkDrag(SdrDragStat& rDrag) const
{
    ImpMeasureRec* pMR=(ImpMeasureRec*)rDrag.GetUser();
    if (pMR!=NULL) {
        delete pMR; // #48544#
        rDrag.SetUser(NULL);
    }
}

XubString SdrMeasureObj::GetDragComment(const SdrDragStat& rDrag, FASTBOOL bUndoDragComment, FASTBOOL bCreateComment) const
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
    USHORT nHdlNum=pHdl->GetObjHdlNum();
    FASTBOOL bOrtho=rDrag.GetView()!=NULL && rDrag.GetView()->IsOrtho();
    FASTBOOL bBigOrtho=bOrtho && rDrag.GetView()->IsBigOrtho();
    FASTBOOL bBelow=rRec.bBelowRefEdge;
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
            FASTBOOL bAnf=nHdlNum==2;
            Point& rMov=bAnf ? rRec.aPt1 : rRec.aPt2;
            Point aMov(rMov);
            Point aFix(bAnf ? rRec.aPt2 : rRec.aPt1);
            if (bOrtho) {
                long ndx0=aMov.X()-aFix.X();
                long ndy0=aMov.Y()-aFix.Y();
                FASTBOOL bHLin=ndy0==0;
                FASTBOOL bVLin=ndx0==0;
                if (!bHLin || !bVLin) { // sonst ist aPt1==aPt2
                    long ndx=aPt.X()-aFix.X();
                    long ndy=aPt.Y()-aFix.Y();
                    double nXFact=0; if (!bVLin) nXFact=(double)ndx/(double)ndx0;
                    double nYFact=0; if (!bHLin) nYFact=(double)ndy/(double)ndy0;
                    FASTBOOL bHor=bHLin || (!bVLin && (nXFact>nYFact) ==bBigOrtho);
                    FASTBOOL bVer=bVLin || (!bHLin && (nXFact<=nYFact)==bBigOrtho);
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

void SdrMeasureObj::TakeDragPoly(const SdrDragStat& rDrag, XPolyPolygon& rXPP) const
{
    ImpMeasureRec* pMR=(ImpMeasureRec*)rDrag.GetUser(); // #48544#
    if (pMR!=NULL) {
        ImpMeasurePoly aMPol;
        ImpCalcGeometrics(*pMR,aMPol);
        ImpCalcXPoly(aMPol,rXPP);
    }
}

FASTBOOL SdrMeasureObj::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho8Possible();
    aPt1=rStat.GetStart();
    aPt2=rStat.GetNow();
    SetTextDirty();
    return TRUE;
}

FASTBOOL SdrMeasureObj::MovCreate(SdrDragStat& rStat)
{
    SdrView* pView=rStat.GetView();
    aPt1=rStat.GetStart();
    aPt2=rStat.GetNow();
    if (pView!=NULL && pView->IsCreate1stPointAsCenter()) {
        aPt1+=aPt1;
        aPt1-=rStat.Now();
    }
    SetTextDirty();
    bBoundRectDirty=TRUE;
    bSnapRectDirty=TRUE;
    return TRUE;
}

FASTBOOL SdrMeasureObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    SetTextDirty();
    SetRectsDirty();
    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

FASTBOOL SdrMeasureObj::BckCreate(SdrDragStat& rStat)
{
    return FALSE;
}

void SdrMeasureObj::BrkCreate(SdrDragStat& rStat)
{
}

void SdrMeasureObj::TakeCreatePoly(const SdrDragStat& rDrag, XPolyPolygon& rXPP) const
{
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    ImpTakeAttr(aRec);
    ImpCalcGeometrics(aRec,aMPol);
    ImpCalcXPoly(aMPol,rXPP);
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
    if (nLen1!=nLen0) { // Aha, Rundungsfehler
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

void SdrMeasureObj::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    SdrTextObj::NbcShear(rRef,nWink,tn,bVShear);
    ShearPoint(aPt1,rRef,tn,bVShear);
    ShearPoint(aPt2,rRef,tn,bVShear);
    SetRectsDirty();
    SetTextDirty();
}

const Rectangle& SdrMeasureObj::GetLogicRect() const
{
    return SdrTextObj::GetLogicRect();
}

void SdrMeasureObj::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrTextObj::NbcSetLogicRect(rRect);
}

long SdrMeasureObj::GetRotateAngle() const
{
    return GetAngle(aPt2-aPt1);
}

void SdrMeasureObj::RecalcBoundRect()
{
    if (bTextDirty) UndirtyText();
    ImpMeasureRec aRec;
    ImpMeasurePoly aMPol;
    ImpTakeAttr(aRec);
    ImpCalcGeometrics(aRec,aMPol);
    XPolyPolygon aXPP;
    ImpCalcXPoly(aMPol,aXPP);
    aOutRect=aXPP.GetBoundRect();

    // Strichstaerke und Linienenden drauftun
    long nLineWdt=ImpGetLineWdt();
    long nLEndWdt=ImpGetLineEndAdd();
    if (nLEndWdt>nLineWdt) nLineWdt=nLEndWdt;
    if (nLineWdt!=0) {
        aOutRect.Left  ()-=nLineWdt;
        aOutRect.Top   ()-=nLineWdt;
        aOutRect.Right ()+=nLineWdt;
        aOutRect.Bottom()+=nLineWdt;
    }

// AW 10082000 taken out, seems not to be necessary...
//  Rectangle aTempRect;
//  TakeUnrotatedSnapRect(aTempRect); // Damit aRect gesetzt ist
    ImpAddShadowToBoundRect();
    ImpAddTextToBoundRect();
}

void SdrMeasureObj::RecalcSnapRect()
{
    // !!!!! nur zu Testzwecken !!!!!
    maSnapRect=Rectangle(aPt1,aPt2);
    maSnapRect.Justify();
}

USHORT SdrMeasureObj::GetSnapPointCount() const
{
    return 2;
}

Point SdrMeasureObj::GetSnapPoint(USHORT i) const
{
    if (i==0) return aPt1;
    else return aPt2;
}

FASTBOOL SdrMeasureObj::IsPolyObj() const
{
    return TRUE;
}

USHORT SdrMeasureObj::GetPointCount() const
{
    return 2;
}

const Point& SdrMeasureObj::GetPoint(USHORT i) const
{
     return i==0 ? aPt1 : aPt2;
}

void SdrMeasureObj::NbcSetPoint(const Point& rPnt, USHORT i)
{
    if (i==0) aPt1=rPnt;
    if (i==1) aPt2=rPnt;
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

void SdrMeasureObj::CreateLinePoly(PolyPolygon3D& rPolyPolygon, PolyPolygon3D& rPolyLine, OutputDevice& rOut,
    BOOL bForceHair, BOOL bIsLineDraft) const
{
    // get XOR Poly as base
    XPolyPolygon aTmpPolyPolygon;
    TakeXorPoly(aTmpPolyPolygon, TRUE);

    // get LineStyleParameterPack
    LineStyleParameterPack aLineAttr(GetItemSet(), bForceHair || bIsLineDraft, rOut);
    LineGeometryCreator aLineCreator(aLineAttr, rPolyPolygon, rPolyLine, bIsLineDraft);
    UINT16 nCount(aTmpPolyPolygon.Count());
    Polygon3D aPoly3D;
    UINT16 nLoopStart(0);

    if(nCount == 3)
    {
        // three lines, first one is the middle one
        aPoly3D = Polygon3D(aTmpPolyPolygon[0]);
        aLineCreator.AddPolygon3D(aPoly3D);

        aLineAttr.ForceNoArrowsLeft(TRUE);
        aLineAttr.ForceNoArrowsRight(TRUE);
        nLoopStart = 1;
    }
    else if(nCount == 4)
    {
        // four lines, middle line with gap, so there are two lines used
        // which have one arrow each
        aLineAttr.ForceNoArrowsRight(TRUE);

        aPoly3D = Polygon3D(aTmpPolyPolygon[0]);
        aLineCreator.AddPolygon3D(aPoly3D);

        aLineAttr.ForceNoArrowsRight(FALSE);
        aLineAttr.ForceNoArrowsLeft(TRUE);

        aPoly3D = Polygon3D(aTmpPolyPolygon[1]);
        aLineCreator.AddPolygon3D(aPoly3D);

        aLineAttr.ForceNoArrowsRight(TRUE);
        nLoopStart = 2;
    }
    else if(nCount == 5)
    {
        // five lines, first two are the outer ones
        aLineAttr.ForceNoArrowsRight(TRUE);

        aPoly3D = Polygon3D(aTmpPolyPolygon[0]);
        aLineCreator.AddPolygon3D(aPoly3D);

        aLineAttr.ForceNoArrowsRight(FALSE);
        aLineAttr.ForceNoArrowsLeft(TRUE);

        aPoly3D = Polygon3D(aTmpPolyPolygon[1]);
        aLineCreator.AddPolygon3D(aPoly3D);

        aLineAttr.ForceNoArrowsRight(TRUE);
        nLoopStart = 2;
    }

    for(;nLoopStart<nCount;nLoopStart++)
    {
        aPoly3D = Polygon3D(aTmpPolyPolygon[nLoopStart]);
        aLineCreator.AddPolygon3D(aPoly3D);
    }
}

SdrObject* SdrMeasureObj::DoConvertToPolyObj(BOOL bBezier) const
{
    // get XOR Poly as base
    XPolyPolygon aTmpPolyPolygon;
    TakeXorPoly(aTmpPolyPolygon, TRUE);

    // get local ItemSet
    SfxItemSet aSet(GetItemSet());

    // prepare group
    SdrObjGroup* pGroup = new SdrObjGroup;
    pGroup->SetModel(GetModel());

    // prepare parameters
    XPolyPolygon aNewPoly;
    SdrPathObj* pPath;
    UINT16 nCount(aTmpPolyPolygon.Count());
    UINT16 nLoopStart(0);

    if(nCount == 3)
    {
        // three lines, first one is the middle one
        aNewPoly.Clear();
        aNewPoly.Insert(aTmpPolyPolygon[0]);
        pPath = new SdrPathObj(OBJ_PATHLINE, aNewPoly);
        pPath->SetModel(GetModel());

        pPath->SetItemSet(aSet);

        pGroup->GetSubList()->NbcInsertObject(pPath);

        aSet.Put(XLineStartWidthItem(0L));
        aSet.Put(XLineEndWidthItem(0L));
        nLoopStart = 1;
    }
    else if(nCount == 4)
    {
        // four lines, middle line with gap, so there are two lines used
        // which have one arrow each
        INT32 nStartWidth = ((const XLineStartWidthItem&)(aSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
        INT32 nEndWidth = ((const XLineEndWidthItem&)(aSet.Get(XATTR_LINEENDWIDTH))).GetValue();

        aSet.Put(XLineEndWidthItem(0L));

        aNewPoly.Clear();
        aNewPoly.Insert(aTmpPolyPolygon[0]);
        pPath = new SdrPathObj(OBJ_PATHLINE, aNewPoly);
        pPath->SetModel(GetModel());

        pPath->SetItemSet(aSet);

        pGroup->GetSubList()->NbcInsertObject(pPath);

        aSet.Put(XLineEndWidthItem(nEndWidth));
        aSet.Put(XLineStartWidthItem(0L));

        aNewPoly.Clear();
        aNewPoly.Insert(aTmpPolyPolygon[1]);
        pPath = new SdrPathObj(OBJ_PATHLINE, aNewPoly);
        pPath->SetModel(GetModel());

        pPath->SetItemSet(aSet);

        pGroup->GetSubList()->NbcInsertObject(pPath);

        aSet.Put(XLineEndWidthItem(0L));
        nLoopStart = 2;
    }
    else if(nCount == 5)
    {
        // five lines, first two are the outer ones
        INT32 nStartWidth = ((const XLineStartWidthItem&)(aSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
        INT32 nEndWidth = ((const XLineEndWidthItem&)(aSet.Get(XATTR_LINEENDWIDTH))).GetValue();

        aSet.Put(XLineEndWidthItem(0L));

        aNewPoly.Clear();
        aNewPoly.Insert(aTmpPolyPolygon[0]);
        pPath = new SdrPathObj(OBJ_PATHLINE, aNewPoly);
        pPath->SetModel(GetModel());

        pPath->SetItemSet(aSet);

        pGroup->GetSubList()->NbcInsertObject(pPath);

        aSet.Put(XLineEndWidthItem(nEndWidth));
        aSet.Put(XLineStartWidthItem(0L));

        aNewPoly.Clear();
        aNewPoly.Insert(aTmpPolyPolygon[1]);
        pPath = new SdrPathObj(OBJ_PATHLINE, aNewPoly);
        pPath->SetModel(GetModel());

        pPath->SetItemSet(aSet);

        pGroup->GetSubList()->NbcInsertObject(pPath);

        aSet.Put(XLineEndWidthItem(0L));
        nLoopStart = 2;
    }

    for(;nLoopStart<nCount;nLoopStart++)
    {
        aNewPoly.Clear();
        aNewPoly.Insert(aTmpPolyPolygon[nLoopStart]);
        pPath = new SdrPathObj(OBJ_PATHLINE, aNewPoly);
        pPath->SetModel(GetModel());

        pPath->SetItemSet(aSet);

        pGroup->GetSubList()->NbcInsertObject(pPath);
    }

    return ImpConvertAddText(pGroup, bBezier);
}

FASTBOOL SdrMeasureObj::BegTextEdit(SdrOutliner& rOutl)
{
    UndirtyText();
    return SdrTextObj::BegTextEdit(rOutl);
}

void SdrMeasureObj::EndTextEdit(SdrOutliner& rOutl)
{
    SdrTextObj::EndTextEdit(rOutl);
}

const Size& SdrMeasureObj::GetTextSize() const
{
    if (bTextDirty) UndirtyText();
    return SdrTextObj::GetTextSize();
}

OutlinerParaObject* SdrMeasureObj::GetOutlinerParaObject() const
{
    if (bTextDirty) UndirtyText();
    return pOutlinerParaObject;
}

void SdrMeasureObj::NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject)
{
    SdrTextObj::NbcSetOutlinerParaObject(pTextObject);
    if (pOutlinerParaObject==NULL) SetTextDirty(); // Text neu berechnen!
}

void SdrMeasureObj::TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, FASTBOOL bNoEditText,
    Rectangle* pAnchorRect, BOOL bLineWidth ) const
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

SdrObject* SdrMeasureObj::CheckTextEditHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    if (bTextDirty) UndirtyText();
    return SdrTextObj::CheckTextEditHit(rPnt,nTol,pVisiLayer);
}

USHORT SdrMeasureObj::GetOutlinerViewAnchorMode() const
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
    FASTBOOL bTextRota90=aRec.bTextRota90;
    FASTBOOL bTextUpsideDown=aRec.bTextUpsideDown;
    FASTBOOL bBelowRefEdge=aRec.bBelowRefEdge;

    // bTextUpsideDown muss hier noch ausgewertet werden!!!!
    if (!bTextRota90) {
        if (eMH==SDRMEASURE_TEXTLEFTOUTSIDE) eTH=SDRTEXTHORZADJUST_RIGHT;
        if (eMH==SDRMEASURE_TEXTRIGHTOUTSIDE) eTH=SDRTEXTHORZADJUST_LEFT;
        // bei eMH==SDRMEASURE_TEXTINSIDE kann horizontal geankert werden.
        if (eMV==SDRMEASURE_ABOVE) eTV=SDRTEXTVERTADJUST_BOTTOM;
        if (eMV==SDRMEASURE_BELOW) eTV=SDRTEXTVERTADJUST_TOP;
        if (eMV==SDRMEASURETEXT_BREAKEDLINE || eMV==SDRMEASURETEXT_VERTICALCENTERED) eTV=SDRTEXTVERTADJUST_CENTER;
    } else {
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
    return (USHORT)eRet;
}

void __EXPORT SdrMeasureObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
{
    if (HAS_BASE(SfxStyleSheet,&rBC)) {
        SfxSimpleHint* pSimple=PTR_CAST(SfxSimpleHint,&rHint);
        ULONG nId=pSimple==NULL ? 0 : pSimple->GetId();

        if(nId == SFX_HINT_DATACHANGED)
        {
            // Alten Bereich invalidieren
            SendRepaintBroadcast();

            // Text hart aufVeraenderung setzen
            SetTextDirty();
        }
    }
    SdrTextObj::SFX_NOTIFY(rBC,rBCType,rHint,rHintType);
}

void SdrMeasureObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    SetTextDirty();
    SdrTextObj::NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet access

SfxItemSet* SdrMeasureObj::CreateNewItemSet(SfxItemPool& rPool)
{
    // include ALL items, 2D and 3D
    return new SfxItemSet(rPool,
        // ranges from SdrAttrObj
        SDRATTR_START, SDRATTRSET_SHADOW,
        SDRATTRSET_OUTLINER, SDRATTRSET_MISC,

        // measure attributes
        SDRATTR_MEASURE_FIRST, SDRATTRSET_MEASURE,

        // outliner and end
        EE_ITEMS_START, EE_ITEMS_END,
        0, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access
void SdrMeasureObj::ItemSetChanged(const SfxItemSet& rSet)
{
    // call parent
    SdrTextObj::ItemSetChanged(rSet);

    // local changes
    SetTextDirty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

void SdrMeasureObj::PreSave()
{
    // call parent
    SdrTextObj::PreSave();

    // prepare SetItems for storage
    const SfxItemSet& rSet = GetUnmergedItemSet();
    const SfxItemSet* pParent = GetStyleSheet() ? &GetStyleSheet()->GetItemSet() : 0L;
    SdrMeasureSetItem aMeasAttr(rSet.GetPool());
    aMeasAttr.GetItemSet().Put(rSet);
    aMeasAttr.GetItemSet().SetParent(pParent);
    mpObjectItemSet->Put(aMeasAttr);
}

void SdrMeasureObj::PostSave()
{
    // call parent
    SdrTextObj::PostSave();

    // remove SetItems from local itemset
    mpObjectItemSet->ClearItem(SDRATTRSET_MEASURE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMeasureObj::WriteData(SvStream& rOut) const
{
    UndirtyText();

    SdrTextObj::WriteData(rOut);
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrMeasureObj");
#endif

    rOut << aPt1;
    rOut << aPt2;
    rOut << BOOL(FALSE); // bTextOverwritten wg. Kompatibilitaet. Gibt's nicht mehr.

    SfxItemPool* pPool=GetItemPool();

    if(pPool)
    {
        const SfxItemSet& rSet = GetUnmergedItemSet();

        pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_MEASURE));
    }
    else
    {
        rOut << sal_uInt16(SFX_ITEMS_NULL);
    }
}

void SdrMeasureObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if (rIn.GetError()!=0) return;
    SdrTextObj::ReadData(rHead,rIn);
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrMeasureObj");
#endif
    rIn>>aPt1;
    rIn>>aPt2;
    BOOL bTextOverwrittenTmp;
    rIn>>bTextOverwrittenTmp;
    SfxItemPool* pPool=GetItemPool();

    if(pPool)
    {
        sal_uInt16 nSetID = SDRATTRSET_MEASURE;
        const SdrMeasureSetItem* pMeasAttr = (const SdrMeasureSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
        if(pMeasAttr)
            SetItemSet(pMeasAttr->GetItemSet());
    }
    else
    {
        sal_uInt16 nSuroDum;
        rIn >> nSuroDum;
    }
}

