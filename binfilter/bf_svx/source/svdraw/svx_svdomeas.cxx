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

#include "svdomeas.hxx"
#include <math.h>
#include "xoutx.hxx"
#include "svdoutl.hxx"
#include "svdpool.hxx"
#include "svdattrx.hxx"
#include "svdio.hxx"
#include "svdview.hxx"
#include "svdstr.hrc"    // Objektname

#include <bf_svtools/style.hxx>



#include "xlnstit.hxx"

#include "xlnstwit.hxx"

#include "xlnedit.hxx"

#include "xlnwtit.hxx"

#include "xlnedwit.hxx"

#include "xlnstcit.hxx"

#include "xlnedcit.hxx"



#include <editobj.hxx>


#include "itemdata.hxx"

#include "svdfield.hxx"


#include <bf_svtools/syslocale.hxx>

namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SV_IMPL_PERSIST1(SdrMeasureField,SvxFieldData);
/*N*/ 
/*N*/ __EXPORT SdrMeasureField::~SdrMeasureField()
/*N*/ {
/*N*/ }

/*N*/ SvxFieldData* __EXPORT SdrMeasureField::Clone() const
/*N*/ {
/*N*/ 	return new SdrMeasureField(*this);
/*N*/ }

/*N*/ int __EXPORT SdrMeasureField::operator==(const SvxFieldData& rSrc) const
/*N*/ {
/*N*/ 	return eMeasureFieldKind==((SdrMeasureField&)rSrc).GetMeasureFieldKind();
/*N*/ }

/*N*/ void __EXPORT SdrMeasureField::Load(SvPersistStream& rIn)
/*N*/ {
/*N*/ 	SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrMeasureField");
/*N*/ #endif
/*N*/ 	UINT16 nFieldKind;
/*N*/ 	rIn>>nFieldKind;
/*N*/ 	eMeasureFieldKind=(SdrMeasureFieldKind)nFieldKind;
/*N*/ }

/*N*/ void __EXPORT SdrMeasureField::Save(SvPersistStream& rOut)
/*N*/ {
/*N*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrMeasureField");
/*N*/ #endif
/*N*/ 	rOut<<(UINT16)eMeasureFieldKind;
/*N*/ }

/*?*/ void SdrMeasureField::TakeRepresentation(const SdrMeasureObj& rObj, XubString& rStr) const
/*?*/ {
/*?*/ 	rStr.Erase();
/*?*/ 	Fraction aMeasureScale(1, 1);
/*?*/ 	BOOL bTextRota90(FALSE);
/*?*/ 	BOOL bShowUnit(FALSE);
/*?*/ 	FieldUnit eMeasureUnit(FUNIT_NONE);
/*?*/ 	FieldUnit eModUIUnit(FUNIT_NONE);
/*?*/ 
/*?*/ 	const SfxItemSet& rSet = rObj.GetItemSet();
/*?*/ 	bTextRota90 = ((SdrMeasureTextRota90Item&)rSet.Get(SDRATTR_MEASURETEXTROTA90)).GetValue();
/*?*/ 	eMeasureUnit = ((SdrMeasureUnitItem&)rSet.Get(SDRATTR_MEASUREUNIT)).GetValue();
/*?*/ 	aMeasureScale = ((SdrMeasureScaleItem&)rSet.Get(SDRATTR_MEASURESCALE)).GetValue();
/*?*/ 	bShowUnit = ((SdrMeasureShowUnitItem&)rSet.Get(SDRATTR_MEASURESHOWUNIT)).GetValue();
/*?*/ 	sal_Int16 nNumDigits = ((SdrMeasureDecimalPlacesItem&)rSet.Get(SDRATTR_MEASUREDECIMALPLACES)).GetValue();
/*?*/ 
/*?*/ 	SdrModel* pModel = rObj.pModel;
/*?*/ 
/*?*/ 	switch(eMeasureFieldKind)
/*?*/ 	{
/*?*/ 		case SDRMEASUREFIELD_VALUE:
/*?*/ 		{
/*?*/ 			if(pModel)
/*?*/ 			{
/*?*/ 				eModUIUnit = pModel->GetUIUnit();
/*?*/ 
/*?*/ 				if(eMeasureUnit == FUNIT_NONE)
/*?*/ 					eMeasureUnit = eModUIUnit;
/*?*/ 
/*?*/ 				INT32 nLen(GetLen(rObj.aPt2 - rObj.aPt1));
/*?*/ 				Fraction aFact(1,1);
/*?*/ 
/*?*/ 				if(eMeasureUnit != eModUIUnit)
/*?*/ 				{
/*?*/ 					// Zur Umrechnung der Einheiten
/*?*/ 					aFact *= GetMapFactor(eModUIUnit, eMeasureUnit).X();
/*?*/ 				}
/*?*/ 
/*?*/ 				if(aMeasureScale.GetNumerator() != aMeasureScale.GetDenominator())
/*?*/ 				{
/*?*/ 					aFact *= aMeasureScale;
/*?*/ 				}
/*?*/ 
/*?*/ 				if(aFact.GetNumerator() != aFact.GetDenominator())
/*?*/ 				{
/*?*/ 					// Scaling ueber BigInt, um Ueberlaeufe zu vermeiden
/*?*/ 					nLen = BigMulDiv(nLen, aFact.GetNumerator(), aFact.GetDenominator());
/*?*/ 				}
/*?*/ 
/*?*/ 				pModel->TakeMetricStr(nLen, rStr, TRUE, nNumDigits);
/*?*/ 
/*?*/ 				if(!aFact.IsValid())
/*?*/ 				{
/*?*/ 					rStr = String();
/*?*/ 					rStr += sal_Unicode('?');
/*?*/ 				}
/*?*/ 
/*?*/                 sal_Unicode cDec(SvtSysLocale().GetLocaleData().getNumDecimalSep().GetChar(0));
/*?*/ 
/*?*/ 				if(rStr.Search(cDec) != STRING_NOTFOUND)
/*?*/ 				{
/*?*/ 					xub_StrLen nLen(rStr.Len() - 1);
/*?*/ 
/*?*/ 					while(rStr.GetChar(nLen) == sal_Unicode('0'))
/*?*/ 					{
/*?*/ 						rStr.Erase(nLen);
/*?*/ 						nLen--;
/*?*/ 					}
/*?*/ 
/*?*/ 					if(rStr.GetChar(nLen) == cDec)
/*?*/ 					{
/*?*/ 						rStr.Erase(nLen);
/*?*/ 						nLen--;
/*?*/ 					}
/*?*/ 
/*?*/ 					if(!rStr.Len())
/*?*/ 						rStr += sal_Unicode('0');
/*?*/ 				}
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				// falls kein Model da ... (z.B. Preview im Dialog)
/*?*/ 				rStr = String();
/*?*/ 				rStr.AppendAscii("4711");
/*?*/ 			}
/*?*/ 
/*?*/ 			break;
/*?*/ 		}
/*?*/ 		case SDRMEASUREFIELD_UNIT:
/*?*/ 		{
/*?*/ 			if(bShowUnit)
/*?*/ 			{
/*?*/ 				if(rObj.pModel)
/*?*/ 				{
/*?*/ 					eModUIUnit = pModel->GetUIUnit();
/*?*/ 
/*?*/ 					if(eMeasureUnit == FUNIT_NONE)
/*?*/ 						eMeasureUnit = eModUIUnit;
/*?*/ 
/*?*/ 					if(bShowUnit)
/*?*/ 						pModel->TakeUnitStr(eMeasureUnit, rStr);
/*?*/ 				}
/*?*/ 			}
/*?*/ 
/*?*/ 			break;
/*?*/ 		}
/*?*/ 		case SDRMEASUREFIELD_ROTA90BLANCS:
/*?*/ 		{
/*?*/ 			if(bTextRota90)
/*?*/ 			{
/*?*/ 				rStr = String();
/*?*/ 				rStr += sal_Unicode(' ');
/*?*/ 			}
/*?*/ 
/*?*/ 			break;
/*?*/ 		}
/*?*/ 	}
/*?*/ }

/*N*/ TYPEINIT1(SdrMeasureObj,SdrTextObj);

/*N*/ void SdrMeasureObj::ForceDefaultAttr()
/*N*/ {
/*N*/ 	SdrTextObj::ForceDefaultAttr();
/*N*/ 
/*N*/ 	//#71958# by default, the show units Bool-Item is set as hard
/*N*/ 	// attribute to TRUE to aviod confusion when copying SdrMeasureObj's
/*N*/ 	// from one application to another
/*N*/ 	ImpForceItemSet();
/*N*/ 	mpObjectItemSet->Put(SdrMeasureShowUnitItem(TRUE));
/*N*/ 
/*N*/ 	XPolygon aXP(4);        //      []
/*N*/ 	aXP[0] = Point(100,0);    // 0,4__[]__2,4
/*N*/ 	aXP[1] = Point(200,400);  //    \    /
/*N*/ 	aXP[2] = Point(0,400);    //     \  /
/*N*/ 	aXP[3] = Point(100,0);    //      \/1,0
/*N*/ 
/*N*/ 	mpObjectItemSet->Put(XLineStartItem(String(), aXP));
/*N*/ 	mpObjectItemSet->Put(XLineStartWidthItem(200));
/*N*/ 	mpObjectItemSet->Put(XLineEndItem(String(), aXP));
/*N*/ 	mpObjectItemSet->Put(XLineEndWidthItem(200));
/*N*/ 	mpObjectItemSet->Put(XLineStyleItem(XLINE_SOLID));
/*N*/ }

/*N*/ SdrMeasureObj::SdrMeasureObj():
/*N*/ 	bTextDirty(FALSE)
/*N*/ {
/*N*/ }

/*N*/ SdrMeasureObj::SdrMeasureObj(const Point& rPt1, const Point& rPt2):
/*N*/ 	aPt1(rPt1),
/*N*/ 	aPt2(rPt2),
/*N*/ 	bTextDirty(FALSE)
/*N*/ {
/*N*/ }

/*N*/ SdrMeasureObj::~SdrMeasureObj()
/*N*/ {
/*N*/ }


/*N*/ UINT16 SdrMeasureObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return (UINT16)OBJ_MEASURE;
/*N*/ }

/*N*/ struct ImpMeasureRec
/*N*/ {
/*N*/ 	Point						aPt1;
/*N*/ 	Point						aPt2;
/*N*/ 	SdrMeasureKind				eKind;
/*N*/ 	SdrMeasureTextHPos			eWantTextHPos;
/*N*/ 	SdrMeasureTextVPos			eWantTextVPos;
/*N*/ 	long						nLineDist;
/*N*/ 	long						nHelplineOverhang;
/*N*/ 	long						nHelplineDist;
/*N*/ 	long						nHelpline1Len;
/*N*/ 	long						nHelpline2Len;
/*N*/ 	FASTBOOL					bBelowRefEdge;
/*N*/ 	FASTBOOL					bTextRota90;
/*N*/ 	FASTBOOL					bTextUpsideDown;
/*N*/ 	long						nMeasureOverhang;
/*N*/ 	FieldUnit					eMeasureUnit;
/*N*/ 	Fraction					aMeasureScale;
/*N*/ 	FASTBOOL					bShowUnit;
/*N*/ 	String						aFormatString;
/*N*/ 	FASTBOOL					bTextAutoAngle;
/*N*/ 	long						nTextAutoAngleView;
/*N*/ 	FASTBOOL					bTextIsFixedAngle;
/*N*/ 	long						nTextFixedAngle;
/*N*/ };
/*N*/ 
/*N*/ struct ImpLineRec
/*N*/ {
/*N*/ 	Point						aP1;
/*N*/ 	Point						aP2;
/*N*/ };
/*N*/ 
/*N*/ struct ImpMeasurePoly
/*N*/ {
/*N*/ 	ImpLineRec					aMainline1; // die mit dem 1. Pfeil
/*N*/ 	ImpLineRec					aMainline2; // die mit dem 2. Pfeil
/*N*/ 	ImpLineRec					aMainline3; // die dazwischen
/*N*/ 	ImpLineRec					aHelpline1;
/*N*/ 	ImpLineRec					aHelpline2;
/*N*/ 	Rectangle					aTextRect;
/*N*/ 	Size						aTextSize;
/*N*/ 	long						nLineLen;
/*N*/ 	long						nLineWink;
/*N*/ 	long						nTextWink;
/*N*/ 	long						nHlpWink;
/*N*/ 	double						nLineSin;
/*N*/ 	double						nLineCos;
/*N*/ 	double						nHlpSin;
/*N*/ 	double						nHlpCos;
/*N*/ 	USHORT						nMainlineAnz;
/*N*/ 	SdrMeasureTextHPos			eUsedTextHPos;
/*N*/ 	SdrMeasureTextVPos			eUsedTextVPos;
/*N*/ 	long						nLineWdt2;  // Halbe Strichstaerke
/*N*/ 	long						nArrow1Len; // Laenge des 1. Pfeils. Bei Center nur die Haelfte
/*N*/ 	long						nArrow2Len; // Laenge des 2. Pfeils. Bei Center nur die Haelfte
/*N*/ 	long						nArrow1Wdt; // Breite des 1. Pfeils
/*N*/ 	long						nArrow2Wdt; // Breite des 2. Pfeils
/*N*/ 	long						nShortLineLen; // Linienlaenge, wenn PfeileAussen
/*N*/ 	FASTBOOL					bArrow1Center; // Pfeil 1 zentriert?
/*N*/ 	FASTBOOL					bArrow2Center; // Pfeil 2 zentriert?
/*N*/ 	FASTBOOL					bAutoUpsideDown; // UpsideDown durch Automatik
/*N*/ 	FASTBOOL					bPfeileAussen;
/*N*/ 	FASTBOOL					bBreakedLine;
/*N*/ };

/*N*/ void SdrMeasureObj::ImpTakeAttr(ImpMeasureRec& rRec) const
/*N*/ {
/*N*/ 	rRec.aPt1 = aPt1;
/*N*/ 	rRec.aPt2 = aPt2;
/*N*/ 
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	rRec.eKind            =((SdrMeasureKindItem&            )rSet.Get(SDRATTR_MEASUREKIND            )).GetValue();
/*N*/ 	rRec.eWantTextHPos    =((SdrMeasureTextHPosItem&        )rSet.Get(SDRATTR_MEASURETEXTHPOS        )).GetValue();
/*N*/ 	rRec.eWantTextVPos    =((SdrMeasureTextVPosItem&        )rSet.Get(SDRATTR_MEASURETEXTVPOS        )).GetValue();
/*N*/ 	rRec.nLineDist        =((SdrMeasureLineDistItem&        )rSet.Get(SDRATTR_MEASURELINEDIST        )).GetValue();
/*N*/ 	rRec.nHelplineOverhang=((SdrMeasureHelplineOverhangItem&)rSet.Get(SDRATTR_MEASUREHELPLINEOVERHANG)).GetValue();
/*N*/ 	rRec.nHelplineDist    =((SdrMeasureHelplineDistItem&    )rSet.Get(SDRATTR_MEASUREHELPLINEDIST    )).GetValue();
/*N*/ 	rRec.nHelpline1Len    =((SdrMeasureHelpline1LenItem&    )rSet.Get(SDRATTR_MEASUREHELPLINE1LEN    )).GetValue();
/*N*/ 	rRec.nHelpline2Len    =((SdrMeasureHelpline2LenItem&    )rSet.Get(SDRATTR_MEASUREHELPLINE2LEN    )).GetValue();
/*N*/ 	rRec.bBelowRefEdge    =((SdrMeasureBelowRefEdgeItem&    )rSet.Get(SDRATTR_MEASUREBELOWREFEDGE    )).GetValue();
/*N*/ 	rRec.bTextRota90      =((SdrMeasureTextRota90Item&      )rSet.Get(SDRATTR_MEASURETEXTROTA90      )).GetValue();
/*N*/ 	rRec.bTextUpsideDown  =((SdrMeasureTextUpsideDownItem&  )rSet.Get(SDRATTR_MEASURETEXTUPSIDEDOWN  )).GetValue();
/*N*/ 	rRec.nMeasureOverhang =((SdrMeasureOverhangItem&        )rSet.Get(SDRATTR_MEASUREOVERHANG        )).GetValue();
/*N*/ 	rRec.eMeasureUnit     =((SdrMeasureUnitItem&            )rSet.Get(SDRATTR_MEASUREUNIT            )).GetValue();
/*N*/ 	rRec.aMeasureScale    =((SdrMeasureScaleItem&           )rSet.Get(SDRATTR_MEASURESCALE           )).GetValue();
/*N*/ 	rRec.bShowUnit        =((SdrMeasureShowUnitItem&        )rSet.Get(SDRATTR_MEASURESHOWUNIT        )).GetValue();
/*N*/ 	rRec.aFormatString    =((SdrMeasureFormatStringItem&    )rSet.Get(SDRATTR_MEASUREFORMATSTRING    )).GetValue();
/*N*/ 	rRec.bTextAutoAngle    =((SdrMeasureTextAutoAngleItem&    )rSet.Get(SDRATTR_MEASURETEXTAUTOANGLE    )).GetValue();
/*N*/ 	rRec.nTextAutoAngleView=((SdrMeasureTextAutoAngleViewItem&)rSet.Get(SDRATTR_MEASURETEXTAUTOANGLEVIEW)).GetValue();
/*N*/ 	rRec.bTextIsFixedAngle =((SdrMeasureTextIsFixedAngleItem& )rSet.Get(SDRATTR_MEASURETEXTISFIXEDANGLE )).GetValue();
/*N*/ 	rRec.nTextFixedAngle   =((SdrMeasureTextFixedAngleItem&   )rSet.Get(SDRATTR_MEASURETEXTFIXEDANGLE   )).GetValue();
/*N*/ }

/*N*/ void SdrMeasureObj::ImpCalcGeometrics(const ImpMeasureRec& rRec, ImpMeasurePoly& rPol) const
/*N*/ {
/*N*/ 	Point aP1(rRec.aPt1);
/*N*/ 	Point aP2(rRec.aPt2);
/*N*/ 	Point aDelt(aP2); aDelt-=aP1;
/*N*/ 
/*N*/ 	rPol.aTextSize=GetTextSize();
/*N*/ 	rPol.nLineLen=GetLen(aDelt);
/*N*/ 
/*N*/ 	rPol.nLineWdt2=0;
/*N*/ 	long nArrow1Len=0; FASTBOOL bArrow1Center=FALSE;
/*N*/ 	long nArrow2Len=0; FASTBOOL bArrow2Center=FALSE;
/*N*/ 	long nArrow1Wdt=0;
/*N*/ 	long nArrow2Wdt=0;
/*N*/ 	rPol.nArrow1Wdt=0;
/*N*/ 	rPol.nArrow2Wdt=0;
/*N*/ 	long nArrowNeed=0;
/*N*/ 	long nShortLen=0;
/*N*/ 	FASTBOOL bPfeileAussen=FALSE;
/*N*/ 
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	sal_Int32 nLineWdt = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue(); // Strichstaerke
/*N*/ 	rPol.nLineWdt2 = (nLineWdt + 1) / 2;
/*N*/ 
/*N*/ 	nArrow1Wdt = ((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
/*N*/ 	if(nArrow1Wdt < 0)
/*N*/ 		nArrow1Wdt = -nLineWdt * nArrow1Wdt / 100; // <0 = relativ
/*N*/ 
/*N*/ 	nArrow2Wdt = ((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();
/*N*/ 	if(nArrow2Wdt < 0)
/*N*/ 		nArrow2Wdt = -nLineWdt * nArrow2Wdt / 100; // <0 = relativ
/*N*/ 
/*N*/ 	XPolygon aPol1(((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetValue());
/*N*/ 	XPolygon aPol2(((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetValue());
/*N*/ 	bArrow1Center = ((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
/*N*/ 	bArrow2Center = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
/*N*/ 	nArrow1Len = XOutputDevice::InitLineStartEnd(aPol1, nArrow1Wdt, bArrow1Center) - 1;
/*N*/ 	nArrow2Len = XOutputDevice::InitLineStartEnd(aPol2, nArrow2Wdt, bArrow2Center) - 1;
/*N*/ 
/*N*/ 	// nArrowLen ist bei bCenter bereits halbiert
/*N*/ 	// Bei 2 Pfeilen a 4mm ist unter 10mm Schluss.
/*N*/ 	nArrowNeed=nArrow1Len+nArrow2Len+(nArrow1Wdt+nArrow2Wdt)/2;
/*N*/ 	if (rPol.nLineLen<nArrowNeed) bPfeileAussen=TRUE;
/*N*/ 	nShortLen=(nArrow1Len+nArrow1Wdt + nArrow2Len+nArrow2Wdt) /2;
/*N*/ 
/*N*/ 	rPol.eUsedTextHPos=rRec.eWantTextHPos;
/*N*/ 	rPol.eUsedTextVPos=rRec.eWantTextVPos;
/*N*/ 	if (rPol.eUsedTextVPos==SDRMEASURE_TEXTVAUTO) rPol.eUsedTextVPos=SDRMEASURE_ABOVE;
/*N*/ 	FASTBOOL bBrkLine=rPol.eUsedTextVPos==SDRMEASURETEXT_BREAKEDLINE;
/*N*/ 	if (rPol.eUsedTextVPos==SDRMEASURETEXT_VERTICALCENTERED) {
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if (pOutlinerParaObject!=NULL && pOutlinerParaObject->GetTextObject().GetParagraphCount()==1) {
/*N*/ 	}
/*N*/ 	rPol.bBreakedLine=bBrkLine;
/*N*/ 	if (rPol.eUsedTextHPos==SDRMEASURE_TEXTHAUTO) { // bei zu breitem Text diesen eventuell nach aussen schieben
/*N*/ 		FASTBOOL bOutside=FALSE;
/*N*/ 		long nNeedSiz=!rRec.bTextRota90 ? rPol.aTextSize.Width() : rPol.aTextSize.Height();
/*N*/ 		if (nNeedSiz>rPol.nLineLen) bOutside=TRUE; // Text passt nicht in die Mitte
/*N*/ 		if (bBrkLine) {
/*N*/ 			if (nNeedSiz+nArrowNeed>rPol.nLineLen) bPfeileAussen=TRUE; // Text passt in die Mitte, wenn die Pfeile nach aussen kommen
/*N*/ 		} else {
/*N*/ 			long nSmallNeed=nArrow1Len+nArrow2Len+(nArrow1Wdt+nArrow2Wdt)/2/4;
/*N*/ 			if (nNeedSiz+nSmallNeed>rPol.nLineLen) bPfeileAussen=TRUE; // Text passt in die Mitte, wenn die Pfeile nach aussen kommen
/*N*/ 		}
/*N*/ 		rPol.eUsedTextHPos=bOutside ? SDRMEASURE_TEXTLEFTOUTSIDE : SDRMEASURE_TEXTINSIDE;
/*N*/ 	}
/*N*/ 	if (rPol.eUsedTextHPos!=SDRMEASURE_TEXTINSIDE) bPfeileAussen=TRUE;
/*N*/ 	rPol.nArrow1Wdt=nArrow1Wdt;
/*N*/ 	rPol.nArrow2Wdt=nArrow2Wdt;
/*N*/ 	rPol.nShortLineLen=nShortLen;
/*N*/ 	rPol.bPfeileAussen=bPfeileAussen;
/*N*/ 	rPol.nArrow1Len=nArrow1Len;
/*N*/ 	rPol.bArrow1Center=bArrow1Center;
/*N*/ 	rPol.nArrow2Len=nArrow2Len;
/*N*/ 	rPol.bArrow2Center=bArrow2Center;
/*N*/ 
/*N*/ 	rPol.nLineWink=GetAngle(aDelt);
/*N*/ 	double a=rPol.nLineWink*nPi180;
/*N*/ 	double nLineSin=sin(a);
/*N*/ 	double nLineCos=cos(a);
/*N*/ 	rPol.nLineSin=nLineSin;
/*N*/ 	rPol.nLineCos=nLineCos;
/*N*/ 
/*N*/ 	rPol.nTextWink=rPol.nLineWink;
/*N*/ 	if (rRec.bTextRota90) rPol.nTextWink+=9000;
/*N*/ 
/*N*/ 	rPol.bAutoUpsideDown=FALSE;
/*N*/ 	if (rRec.bTextAutoAngle) {
/*N*/ 		long nTmpWink=NormAngle360(rPol.nTextWink-rRec.nTextAutoAngleView);
/*N*/ 		if (nTmpWink>=18000) {
/*N*/ 			rPol.nTextWink+=18000;
/*N*/ 			rPol.bAutoUpsideDown=TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rRec.bTextUpsideDown) rPol.nTextWink+=18000;
/*N*/ 	rPol.nTextWink=NormAngle360(rPol.nTextWink);
/*N*/ 	rPol.nHlpWink=rPol.nLineWink+9000;
/*N*/ 	if (rRec.bBelowRefEdge) rPol.nHlpWink+=18000;
/*N*/ 	rPol.nHlpWink=NormAngle360(rPol.nHlpWink);
/*N*/ 	double nHlpSin=nLineCos;
/*N*/ 	double nHlpCos=-nLineSin;
/*N*/ 	if (rRec.bBelowRefEdge) {
/*N*/ 		nHlpSin=-nHlpSin;
/*N*/ 		nHlpCos=-nHlpCos;
/*N*/ 	}
/*N*/ 	rPol.nHlpSin=nHlpSin;
/*N*/ 	rPol.nHlpCos=nHlpCos;
/*N*/ 
/*N*/ 	long nLineDist=rRec.nLineDist;
/*N*/ 	long nOverhang=rRec.nHelplineOverhang;
/*N*/ 	long nHelplineDist=rRec.nHelplineDist;
/*N*/ 
/*N*/ 	long dx= Round(nLineDist*nHlpCos);
/*N*/ 	long dy=-Round(nLineDist*nHlpSin);
/*N*/ 	long dxh1a= Round((nHelplineDist-rRec.nHelpline1Len)*nHlpCos);
/*N*/ 	long dyh1a=-Round((nHelplineDist-rRec.nHelpline1Len)*nHlpSin);
/*N*/ 	long dxh1b= Round((nHelplineDist-rRec.nHelpline2Len)*nHlpCos);
/*N*/ 	long dyh1b=-Round((nHelplineDist-rRec.nHelpline2Len)*nHlpSin);
/*N*/ 	long dxh2= Round((nLineDist+nOverhang)*nHlpCos);
/*N*/ 	long dyh2=-Round((nLineDist+nOverhang)*nHlpSin);
/*N*/ 
/*N*/ 	// Masshilfslinie 1
/*N*/ 	rPol.aHelpline1.aP1=Point(aP1.X()+dxh1a,aP1.Y()+dyh1a);
/*N*/ 	rPol.aHelpline1.aP2=Point(aP1.X()+dxh2,aP1.Y()+dyh2);
/*N*/ 
/*N*/ 	// Masshilfslinie 2
/*N*/ 	rPol.aHelpline2.aP1=Point(aP2.X()+dxh1b,aP2.Y()+dyh1b);
/*N*/ 	rPol.aHelpline2.aP2=Point(aP2.X()+dxh2,aP2.Y()+dyh2);
/*N*/ 
/*N*/ 	// Masslinie(n)
/*N*/ 	Point aMainlinePt1(aP1.X()+dx,aP1.Y()+dy);
/*N*/ 	Point aMainlinePt2(aP2.X()+dx,aP2.Y()+dy);
/*N*/ 	if (!bPfeileAussen) {
/*N*/ 		rPol.aMainline1.aP1=aMainlinePt1;
/*N*/ 		rPol.aMainline1.aP2=aMainlinePt2;
/*N*/ 		rPol.aMainline2=rPol.aMainline1;
/*N*/ 		rPol.aMainline3=rPol.aMainline1;
/*N*/ 		rPol.nMainlineAnz=1;
/*N*/ 		if (bBrkLine) {
/*?*/ 			long nNeedSiz=!rRec.bTextRota90 ? rPol.aTextSize.Width() : rPol.aTextSize.Height();
/*?*/ 			long nHalfLen=(rPol.nLineLen-nNeedSiz-nArrow1Wdt/4-nArrow2Wdt/4) /2;
/*?*/ 			rPol.nMainlineAnz=2;
/*?*/ 			rPol.aMainline1.aP2=aMainlinePt1;
/*?*/ 			rPol.aMainline1.aP2.X()+=nHalfLen;
/*?*/ 			RotatePoint(rPol.aMainline1.aP2,rPol.aMainline1.aP1,nLineSin,nLineCos);
/*?*/ 			rPol.aMainline2.aP1=aMainlinePt2;
/*?*/ 			rPol.aMainline2.aP1.X()-=nHalfLen;
/*?*/ 			RotatePoint(rPol.aMainline2.aP1,rPol.aMainline2.aP2,nLineSin,nLineCos);
/*N*/ 		}
/*N*/ 	} else {
/*N*/ 		long nLen1=nShortLen; // Pfeilbreite als Linienlaenge ausserhalb des Pfeils
/*N*/ 		long nLen2=nShortLen;
/*N*/ 		long nTextWdt=rRec.bTextRota90 ? rPol.aTextSize.Height() : rPol.aTextSize.Width();
/*N*/ 		if (!bBrkLine) {
/*N*/ 			if (rPol.eUsedTextHPos==SDRMEASURE_TEXTLEFTOUTSIDE) nLen1=nArrow1Len+nTextWdt;
/*N*/ 			if (rPol.eUsedTextHPos==SDRMEASURE_TEXTRIGHTOUTSIDE) nLen2=nArrow2Len+nTextWdt;
/*N*/ 		}
/*N*/ 		rPol.aMainline1.aP1=aMainlinePt1;
/*N*/ 		rPol.aMainline1.aP2=aMainlinePt1; rPol.aMainline1.aP2.X()-=nLen1; RotatePoint(rPol.aMainline1.aP2,aMainlinePt1,nLineSin,nLineCos);
/*N*/ 		rPol.aMainline2.aP1=aMainlinePt2; rPol.aMainline2.aP1.X()+=nLen2; RotatePoint(rPol.aMainline2.aP1,aMainlinePt2,nLineSin,nLineCos);
/*N*/ 		rPol.aMainline2.aP2=aMainlinePt2;
/*N*/ 		rPol.aMainline3.aP1=aMainlinePt1;
/*N*/ 		rPol.aMainline3.aP2=aMainlinePt2;
/*N*/ 		rPol.nMainlineAnz=3;
/*N*/ 		if (bBrkLine && rPol.eUsedTextHPos==SDRMEASURE_TEXTINSIDE) rPol.nMainlineAnz=2;
/*N*/ 	}
/*N*/ }

/*N*/ void SdrMeasureObj::ImpCalcXPoly(const ImpMeasurePoly& rPol, XPolyPolygon& rXPP) const
/*N*/ {
/*N*/ 	rXPP.Clear();
/*N*/ 	XPolygon aXP(2);
/*N*/ 	aXP[0]=rPol.aMainline1.aP1;
/*N*/ 	aXP[1]=rPol.aMainline1.aP2;
/*N*/ 	rXPP.Insert(aXP);
/*N*/ 	if (rPol.nMainlineAnz>1) {
/*N*/ 		aXP[0]=rPol.aMainline2.aP1;
/*N*/ 		aXP[1]=rPol.aMainline2.aP2;
/*N*/ 		rXPP.Insert(aXP);
/*N*/ 	}
/*N*/ 	if (rPol.nMainlineAnz>2) {
/*N*/ 		aXP[0]=rPol.aMainline3.aP1;
/*N*/ 		aXP[1]=rPol.aMainline3.aP2;
/*N*/ 		rXPP.Insert(aXP);
/*N*/ 	}
/*N*/ 	aXP[0]=rPol.aHelpline1.aP1;
/*N*/ 	aXP[1]=rPol.aHelpline1.aP2;
/*N*/ 	rXPP.Insert(aXP);
/*N*/ 	aXP[0]=rPol.aHelpline2.aP1;
/*N*/ 	aXP[1]=rPol.aHelpline2.aP2;
/*N*/ 	rXPP.Insert(aXP);
/*N*/ }


/*N*/ FASTBOOL SdrMeasureObj::CalcFieldValue(const SvxFieldItem& rField, USHORT nPara, USHORT nPos,
/*N*/ 	FASTBOOL bEdit,
/*N*/ 	Color*& rpTxtColor, Color*& rpFldColor, XubString& rRet) const
/*N*/ {
/*N*/ 	const SvxFieldData* pField=rField.GetField();
/*N*/ 	SdrMeasureField* pMeasureField=PTR_CAST(SdrMeasureField,pField);
/*N*/ 	if (pMeasureField!=NULL) {
/*N*/ 		pMeasureField->TakeRepresentation(*this,rRet);
/*N*/ 		if (rpFldColor!=NULL) {
/*?*/ 			if (!bEdit)
/*?*/ 			{
/*?*/ 				delete rpFldColor;
/*?*/ 				rpFldColor=NULL;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		return TRUE;
/*N*/ 	} else {
/*?*/ 		return SdrTextObj::CalcFieldValue(rField,nPara,nPos,bEdit,rpTxtColor,rpFldColor,rRet);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrMeasureObj::UndirtyText() const
/*N*/ {
/*N*/ 	if (bTextDirty) {
/*N*/ 		SdrOutliner& rOutliner=ImpGetDrawOutliner();
/*N*/ 		if (pOutlinerParaObject==NULL) {
/*N*/ 			rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_ROTA90BLANCS)),ESelection(0,0));
/*N*/ 			rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_VALUE)),ESelection(0,1));
/*N*/ 			rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_UNIT)),ESelection(0,2));
/*N*/ 			rOutliner.QuickInsertField(SvxFieldItem(SdrMeasureField(SDRMEASUREFIELD_ROTA90BLANCS)),ESelection(0,3));
/*N*/ 
/*N*/ 			if(GetStyleSheet())
/*N*/ 				rOutliner.SetStyleSheet(0, GetStyleSheet());
/*N*/ 
/*N*/ 			rOutliner.SetParaAttribs(0, GetItemSet());
/*N*/ 
/*N*/ 			// casting auf nonconst
/*N*/ 			((SdrMeasureObj*)this)->pOutlinerParaObject=rOutliner.CreateParaObject();
/*N*/ 		} else {
/*N*/ 			rOutliner.SetText(*pOutlinerParaObject);
/*N*/ 		}
/*N*/ 		rOutliner.SetUpdateMode(TRUE);
/*N*/ 		rOutliner.UpdateFields();
/*N*/ 		Size aSiz(rOutliner.CalcTextSize());
/*N*/ 		rOutliner.Clear();
/*N*/ 		// 3x casting auf nonconst
/*N*/ 		((SdrMeasureObj*)this)->aTextSize=aSiz;
/*N*/ 		((SdrMeasureObj*)this)->bTextSizeDirty=FALSE;
/*N*/ 		((SdrMeasureObj*)this)->bTextDirty=FALSE;
/*N*/ 	}
/*N*/ }

/*N*/ void SdrMeasureObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
/*N*/ {
/*N*/ 	if (bTextDirty) UndirtyText();
/*N*/ 	ImpMeasureRec aRec;
/*N*/ 	ImpMeasurePoly aMPol;
/*N*/ 	ImpTakeAttr(aRec);
/*N*/ 	ImpCalcGeometrics(aRec,aMPol);
/*N*/ 
/*N*/ 	// TextSize ermitteln inkl. Textrahmenabstaende
/*N*/ 	Size aTextSize(aMPol.aTextSize);
/*N*/ 	if (aTextSize.Width()<1) aTextSize.Width()=1;
/*N*/ 	if (aTextSize.Height()<1) aTextSize.Height()=1;
/*N*/ 	aTextSize.Width()+=GetTextLeftDistance()+GetTextRightDistance();
/*N*/ 	aTextSize.Height()+=GetTextUpperDistance()+GetTextLowerDistance();
/*N*/ 
/*N*/ 	Point aPt1(aMPol.aMainline1.aP1);
/*N*/ 	long nLen=aMPol.nLineLen;
/*N*/ 	long nLWdt=aMPol.nLineWdt2;
/*N*/ 	long nArr1Len=aMPol.nArrow1Len;
/*N*/ 	long nArr2Len=aMPol.nArrow2Len;
/*N*/ 	if (aMPol.bBreakedLine) {
/*N*/ 		// Bei Unterbrochener Linie und Outside muss der Text nicht neben den
/*N*/ 		// Pfeil sondern neben die Linie an dem Pfeil plaziert werden
/*?*/ 		nArr1Len=aMPol.nShortLineLen+aMPol.nArrow1Wdt/4;
/*?*/ 		nArr2Len=aMPol.nShortLineLen+aMPol.nArrow2Wdt/4;
/*N*/ 	}
/*N*/ 
/*N*/ 	Point aTextPos;
/*N*/ 	FASTBOOL bRota90=aRec.bTextRota90;
/*N*/ 	FASTBOOL bUpsideDown=aRec.bTextUpsideDown!=aMPol.bAutoUpsideDown;
/*N*/ 	FASTBOOL bBelowRefEdge=aRec.bBelowRefEdge;
/*N*/ 	SdrMeasureTextHPos eMH=aMPol.eUsedTextHPos;
/*N*/ 	SdrMeasureTextVPos eMV=aMPol.eUsedTextVPos;
/*N*/ 	if (!bRota90) {
/*N*/ 		switch (eMH) {
/*N*/ 			case SDRMEASURE_TEXTLEFTOUTSIDE: aTextPos.X()=aPt1.X()-aTextSize.Width()-nArr1Len-nLWdt; break;
/*N*/ 			case SDRMEASURE_TEXTRIGHTOUTSIDE: aTextPos.X()=aPt1.X()+nLen+nArr2Len+nLWdt; break;
/*N*/ 			default: aTextPos.X()=aPt1.X(); aTextSize.Width()=nLen;
/*N*/ 		}
/*N*/ 		switch (eMV) {
/*N*/ 			case SDRMEASURETEXT_VERTICALCENTERED:
/*N*/ 			case SDRMEASURETEXT_BREAKEDLINE: aTextPos.Y()=aPt1.Y()-aTextSize.Height()/2; break;
/*N*/ 			case SDRMEASURE_BELOW: {
/*N*/ 				if (!bUpsideDown) aTextPos.Y()=aPt1.Y()+nLWdt;
/*N*/ 				else aTextPos.Y()=aPt1.Y()-aTextSize.Height()-nLWdt;
/*N*/ 			} break;
/*N*/ 			default: {
/*N*/ 				if (!bUpsideDown) aTextPos.Y()=aPt1.Y()-aTextSize.Height()-nLWdt;
/*N*/ 				else aTextPos.Y()=aPt1.Y()+nLWdt;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if (bUpsideDown) {
/*N*/ 			aTextPos.X()+=aTextSize.Width();
/*N*/ 			aTextPos.Y()+=aTextSize.Height();
/*N*/ 		}
/*N*/ 	} else { // also wenn bTextRota90==TRUE
/*N*/ 		switch (eMH) {
/*N*/ 			case SDRMEASURE_TEXTLEFTOUTSIDE: aTextPos.X()=aPt1.X()-aTextSize.Height()-nArr1Len; break;
/*N*/ 			case SDRMEASURE_TEXTRIGHTOUTSIDE: aTextPos.X()=aPt1.X()+nLen+nArr2Len; break;
/*N*/ 			default: aTextPos.X()=aPt1.X(); aTextSize.Height()=nLen;
/*N*/ 		}
/*N*/ 		switch (eMV) {
/*N*/ 			case SDRMEASURETEXT_VERTICALCENTERED:
/*N*/ 			case SDRMEASURETEXT_BREAKEDLINE: aTextPos.Y()=aPt1.Y()+aTextSize.Width()/2; break;
/*N*/ 			case SDRMEASURE_BELOW: {
/*N*/ 				if (!bBelowRefEdge) aTextPos.Y()=aPt1.Y()+aTextSize.Width()+nLWdt;
/*N*/ 				else aTextPos.Y()=aPt1.Y()-nLWdt;
/*N*/ 			} break;
/*N*/ 			default: {
/*N*/ 				if (!bBelowRefEdge) aTextPos.Y()=aPt1.Y()-nLWdt;
/*N*/ 				else aTextPos.Y()=aPt1.Y()+aTextSize.Width()+nLWdt;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if (bUpsideDown) {
/*N*/ 			aTextPos.X()+=aTextSize.Height();
/*N*/ 			aTextPos.Y()-=aTextSize.Width();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (aMPol.nTextWink!=aGeo.nDrehWink) {
/*N*/ 		((SdrMeasureObj*)this)->aGeo.nDrehWink=aMPol.nTextWink;
/*N*/ 		((SdrMeasureObj*)this)->aGeo.RecalcSinCos();
/*N*/ 	}
/*N*/ 	RotatePoint(aTextPos,aPt1,aMPol.nLineSin,aMPol.nLineCos);
/*N*/ 	aTextSize.Width()++; aTextSize.Height()++; // wg. des komischen Verhaltens beim Rect-Ctor
/*N*/ 	rRect=Rectangle(aTextPos,aTextSize);
/*N*/ 	rRect.Justify();
/*N*/ 	((SdrMeasureObj*)this)->aRect=rRect;
/*N*/ 
/*N*/ 	if (aMPol.nTextWink!=aGeo.nDrehWink) {
/*N*/ 		((SdrMeasureObj*)this)->aGeo.nDrehWink=aMPol.nTextWink;
/*N*/ 		((SdrMeasureObj*)this)->aGeo.RecalcSinCos();
/*N*/ 	}
/*N*/ }

























/*N*/ void SdrMeasureObj::NbcMove(const Size& rSiz)
/*N*/ {
/*N*/ 	SdrTextObj::NbcMove(rSiz);
/*N*/ 	MovePoint(aPt1,rSiz);
/*N*/ 	MovePoint(aPt2,rSiz);
/*N*/ }

/*N*/ void SdrMeasureObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	SdrTextObj::NbcResize(rRef,xFact,yFact);
/*N*/ 	ResizePoint(aPt1,rRef,xFact,yFact);
/*N*/ 	ResizePoint(aPt2,rRef,xFact,yFact);
/*N*/ 	SetTextDirty();
/*N*/ }






/*N*/ long SdrMeasureObj::GetRotateAngle() const
/*N*/ {
/*N*/ 	return GetAngle(aPt2-aPt1);
/*N*/ }

/*N*/ void SdrMeasureObj::RecalcBoundRect()
/*N*/ {
/*N*/ 	if (bTextDirty) UndirtyText();
/*N*/ 	ImpMeasureRec aRec;
/*N*/ 	ImpMeasurePoly aMPol;
/*N*/ 	ImpTakeAttr(aRec);
/*N*/ 	ImpCalcGeometrics(aRec,aMPol);
/*N*/ 	XPolyPolygon aXPP;
/*N*/ 	ImpCalcXPoly(aMPol,aXPP);
/*N*/ 	aOutRect=aXPP.GetBoundRect();
/*N*/ 
/*N*/ 	// Strichstaerke und Linienenden drauftun
/*N*/ 	long nLineWdt=ImpGetLineWdt();
/*N*/ 	long nLEndWdt=ImpGetLineEndAdd();
/*N*/ 	if (nLEndWdt>nLineWdt) nLineWdt=nLEndWdt;
/*N*/ 	if (nLineWdt!=0) {
/*N*/ 		aOutRect.Left  ()-=nLineWdt;
/*N*/ 		aOutRect.Top   ()-=nLineWdt;
/*N*/ 		aOutRect.Right ()+=nLineWdt;
/*N*/ 		aOutRect.Bottom()+=nLineWdt;
/*N*/ 	}
/*N*/ 
/*N*/ // AW 10082000 taken out, seems not to be necessary...
/*N*/ //	Rectangle aTempRect;
/*N*/ //	TakeUnrotatedSnapRect(aTempRect); // Damit aRect gesetzt ist
/*N*/ 	ImpAddShadowToBoundRect();
/*N*/ 	ImpAddTextToBoundRect();
/*N*/ }

/*N*/ void SdrMeasureObj::RecalcSnapRect()
/*N*/ {
/*N*/ 	// !!!!! nur zu Testzwecken !!!!!
/*N*/ 	//maSnapRect=Rectangle(aPt1,aPt2);
/*N*/ 	//maSnapRect.Justify();
/*N*/ 
/*N*/ 	// #94520# Added correct implementation here.
/*N*/ 	ImpMeasureRec aRec;
/*N*/ 	ImpMeasurePoly aMPol;
/*N*/ 	XPolyPolygon aXPP;
/*N*/ 
/*N*/ 	ImpTakeAttr(aRec);
/*N*/ 	ImpCalcGeometrics(aRec, aMPol);
/*N*/ 	ImpCalcXPoly(aMPol, aXPP);
/*N*/ 	maSnapRect = aXPP.GetBoundRect();
/*N*/ }





/*N*/ const Point& SdrMeasureObj::GetPoint(USHORT i) const
/*N*/ {
/*N*/ 	 return i==0 ? aPt1 : aPt2;
/*N*/ }

/*N*/ void SdrMeasureObj::NbcSetPoint(const Point& rPnt, USHORT i)
/*N*/ {
/*N*/ 	if (i==0) aPt1=rPnt;
/*N*/ 	if (i==1) aPt2=rPnt;
/*N*/ 	SetRectsDirty();
/*N*/ 	SetTextDirty();
/*N*/ }






/*N*/ FASTBOOL SdrMeasureObj::BegTextEdit(SdrOutliner& rOutl)
/*N*/ {DBG_BF_ASSERT(0, "STRIP");return FALSE; //STRIP001 
/*N*/ }

/*N*/ void SdrMeasureObj::EndTextEdit(SdrOutliner& rOutl)
/*N*/ {
/*N*/ 	SdrTextObj::EndTextEdit(rOutl);
/*N*/ }

/*N*/ const Size& SdrMeasureObj::GetTextSize() const
/*N*/ {
/*N*/ 	if (bTextDirty) UndirtyText();
/*N*/ 	return SdrTextObj::GetTextSize();
/*N*/ }

/*N*/ OutlinerParaObject* SdrMeasureObj::GetOutlinerParaObject() const
/*N*/ {
/*N*/ 	if (bTextDirty) UndirtyText();
/*N*/ 	return pOutlinerParaObject;
/*N*/ }

/*N*/ void SdrMeasureObj::NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject)
/*N*/ {
/*N*/ 	SdrTextObj::NbcSetOutlinerParaObject(pTextObject);
/*N*/ 	if (pOutlinerParaObject==NULL) SetTextDirty(); // Text neu berechnen!
/*N*/ }

/*N*/ void SdrMeasureObj::TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, FASTBOOL bNoEditText,
/*N*/ 	Rectangle* pAnchorRect, BOOL bLineWidth ) const
/*N*/ {
/*N*/ 	if (bTextDirty) UndirtyText();
/*N*/ 	SdrTextObj::TakeTextRect( rOutliner, rTextRect, bNoEditText, pAnchorRect, bLineWidth );
/*N*/ }

/*N*/ void SdrMeasureObj::TakeTextAnchorRect(Rectangle& rAnchorRect) const
/*N*/ {
/*N*/ 	if (bTextDirty) UndirtyText();
/*N*/ 	SdrTextObj::TakeTextAnchorRect(rAnchorRect);
/*N*/ }




/*N*/ void __EXPORT SdrMeasureObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
/*N*/ {
/*N*/ 	if (HAS_BASE(SfxStyleSheet,&rBC)) {
/*?*/ 		SfxSimpleHint* pSimple=PTR_CAST(SfxSimpleHint,&rHint);
/*?*/ 		ULONG nId=pSimple==NULL ? 0 : pSimple->GetId();
/*?*/ 
/*?*/ 		if(nId == SFX_HINT_DATACHANGED)
/*?*/ 		{
/*?*/ 			// Alten Bereich invalidieren
/*?*/ 			SendRepaintBroadcast();
/*?*/ 
/*?*/ 			// Text hart aufVeraenderung setzen
/*?*/ 			SetTextDirty();
/*?*/ 		}
/*N*/ 	}
/*N*/ 	SdrTextObj::SFX_NOTIFY(rBC,rBCType,rHint,rHintType);
/*N*/ }

/*N*/ void SdrMeasureObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ 	SetTextDirty();
/*N*/ 	SdrTextObj::NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet access

/*N*/ SfxItemSet* SdrMeasureObj::CreateNewItemSet(SfxItemPool& rPool)
/*N*/ {
/*N*/ 	// include ALL items, 2D and 3D
/*N*/ 	return new SfxItemSet(rPool,
/*N*/ 		// ranges from SdrAttrObj
/*N*/ 		SDRATTR_START, SDRATTRSET_SHADOW,
/*N*/ 		SDRATTRSET_OUTLINER, SDRATTRSET_MISC,
/*N*/ 		SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
/*N*/ 
/*N*/ 		// measure attributes
/*N*/ 		SDRATTR_MEASURE_FIRST, SDRATTRSET_MEASURE,
/*N*/ 
/*N*/ 		// outliner and end
/*N*/ 		EE_ITEMS_START, EE_ITEMS_END,
/*N*/ 		0, 0);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access
/*N*/ void SdrMeasureObj::ItemSetChanged(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrTextObj::ItemSetChanged(rSet);
/*N*/ 
/*N*/ 	// local changes
/*N*/ 	SetTextDirty();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

/*N*/ void SdrMeasureObj::PreSave()
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrTextObj::PreSave();
/*N*/ 
/*N*/ 	// prepare SetItems for storage
/*N*/ 	const SfxItemSet& rSet = GetUnmergedItemSet();
/*N*/ 	const SfxItemSet* pParent = GetStyleSheet() ? &GetStyleSheet()->GetItemSet() : 0L;
/*N*/ 	SdrMeasureSetItem aMeasAttr(rSet.GetPool());
/*N*/ 	aMeasAttr.GetItemSet().Put(rSet);
/*N*/ 	aMeasAttr.GetItemSet().SetParent(pParent);
/*N*/ 	mpObjectItemSet->Put(aMeasAttr);
/*N*/ }

/*N*/ void SdrMeasureObj::PostSave()
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrTextObj::PostSave();
/*N*/ 
/*N*/ 	// remove SetItems from local itemset
/*N*/ 	mpObjectItemSet->ClearItem(SDRATTRSET_MEASURE);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrMeasureObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	UndirtyText();
/*N*/ 
/*N*/ 	SdrTextObj::WriteData(rOut);
/*N*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrMeasureObj");
/*N*/ #endif
/*N*/ 
/*N*/ 	rOut << aPt1;
/*N*/ 	rOut << aPt2;
/*N*/ 	rOut << BOOL(FALSE); // bTextOverwritten wg. Kompatibilitaet. Gibt's nicht mehr.
/*N*/ 
/*N*/ 	SfxItemPool* pPool=GetItemPool();
/*N*/ 
/*N*/ 	if(pPool)
/*N*/ 	{
/*N*/ 		const SfxItemSet& rSet = GetUnmergedItemSet();
/*N*/ 
/*N*/ 		pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_MEASURE));
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		rOut << sal_uInt16(SFX_ITEMS_NULL);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrMeasureObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (rIn.GetError()!=0) return;
/*N*/ 	SdrTextObj::ReadData(rHead,rIn);
/*N*/ 	SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrMeasureObj");
/*N*/ #endif
/*N*/ 	rIn>>aPt1;
/*N*/ 	rIn>>aPt2;
/*N*/ 	BOOL bTextOverwrittenTmp;
/*N*/ 	rIn>>bTextOverwrittenTmp;
/*N*/ 	SfxItemPool* pPool=GetItemPool();
/*N*/ 
/*N*/ 	if(pPool)
/*N*/ 	{
/*N*/ 		sal_uInt16 nSetID = SDRATTRSET_MEASURE;
/*N*/ 		const SdrMeasureSetItem* pMeasAttr = (const SdrMeasureSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
/*N*/ 		if(pMeasAttr)
/*N*/ 			SetItemSet(pMeasAttr->GetItemSet());
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		sal_uInt16 nSuroDum;
/*N*/ 		rIn >> nSuroDum;
/*N*/ 	}
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
