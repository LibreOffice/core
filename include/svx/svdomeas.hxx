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

#ifndef _SVDOMEAS_HXX
#define _SVDOMEAS_HXX

#include <svx/svdotext.hxx>
#include "svx/svxdllapi.h"
#include <editeng/measfld.hxx>

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrOutliner;
struct ImpMeasureRec;
struct ImpMeasurePoly;

namespace sdr { namespace properties {
    class MeasureProperties;
}}

//************************************************************
//   Hilfsklasse SdrMeasureObjGeoData
//************************************************************

class SdrMeasureObjGeoData : public SdrTextObjGeoData
{
public:
    Point                       aPt1;
    Point                       aPt2;

public:
    SdrMeasureObjGeoData();
    virtual ~SdrMeasureObjGeoData();
};

//************************************************************
//   SdrMeasureObj
//************************************************************

class SVX_DLLPUBLIC SdrMeasureObj : public SdrTextObj
{
private:
    // to allow sdr::properties::MeasureProperties access to SetTextDirty()
    friend class sdr::properties::MeasureProperties;

    friend class                SdrMeasureField;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    Point                       aPt1;
    Point                       aPt2;
    bool                        bTextDirty;

protected:
    void ImpTakeAttr(ImpMeasureRec& rRec) const;
    void TakeRepresentation( XubString& rStr, SdrMeasureFieldKind eMeasureFieldKind ) const;
    void ImpCalcGeometrics(const ImpMeasureRec& rRec, ImpMeasurePoly& rPol) const;
    basegfx::B2DPolyPolygon ImpCalcXPoly(const ImpMeasurePoly& rPol) const;
    void ImpEvalDrag(ImpMeasureRec& rRec, const SdrDragStat& rDrag) const;
    void SetTextDirty() { bTextDirty=sal_True; SetTextSizeDirty(); if (!aOutRect.IsEmpty()) { SetBoundRectDirty(); SetRectsDirty(sal_True); } }
    void UndirtyText() const;

    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);

public:
    TYPEINFO();
    SdrMeasureObj();
    SdrMeasureObj(const Point& rPt1, const Point& rPt2);
    virtual ~SdrMeasureObj();

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;
    virtual SdrMeasureObj* Clone() const;

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual sal_uInt32 GetHdlCount() const;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const;

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;

    virtual bool BegCreate(SdrDragStat& rStat);
    virtual bool MovCreate(SdrDragStat& rStat);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;
    virtual Pointer GetCreatePointer() const;

    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear(const Point& rRef, long nWink, double tn, bool bVShear);
    virtual long GetRotateAngle() const;
    virtual void RecalcSnapRect();

    virtual sal_uInt32 GetSnapPointCount() const;
    virtual Point GetSnapPoint(sal_uInt32 i) const;

    virtual sal_Bool IsPolyObj() const;
    virtual sal_uInt32 GetPointCount() const;
    virtual Point GetPoint(sal_uInt32 i) const;
    virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i);

    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;

    virtual sal_Bool BegTextEdit(SdrOutliner& rOutl);
    virtual const Size& GetTextSize() const;
    virtual void TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, bool bNoEditText = false,
        Rectangle* pAnchorRect=NULL, bool bLineWidth = true ) const;
    virtual void TakeTextAnchorRect(Rectangle& rAnchorRect) const;
    virtual void TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const;
    virtual sal_uInt16 GetOutlinerViewAnchorMode() const;
    virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject);
    virtual OutlinerParaObject* GetOutlinerParaObject() const;

    virtual bool CalcFieldValue(const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos,
        bool bEdit, Color*& rpTxtColor, Color*& rpFldColor, String& rRet) const;

    // #i97878#
    virtual sal_Bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const;
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon);
};

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Creating:
// ~~~~~~~~~
// Dragging von Bezugspunkt 1 zu Bezugspunkt 2 -> Bezugskante
//
// Die Defaults:
// ~~~~~~~~~~~~~
// Masslinie und Masshilfslinien: Haarlinien solid schwarz
// Pfeile:     2mm x 4mm
// Textgroesse
//                              ___
//     |        Masszahl       | 2mm
//     |<--------------------->|---
//     |                       | 8mm
//     |                       |
//    Pt1============#        Pt2-- <----Bezugskante (von Pt1 nach Pt2)
//     #             #         |___ <- Ueberstand der Masshilfslinie(n)
//     #             #=========#
//     # Zu bemassendes Objekt #
//     #=======================#
//
// Attribute:
// ~~~~~~~~~~
// 1. Wo steht der Text: mitte, rechts oder links (def=automatik)
// 2. Text oberhalb der Linie oder unterhalb oder Linie unterbrochen durch Text (def=automatik)
// 3. Den Abstand der Masslinie zur Bezugskante (=zum bemassten Objekt).
//    Default=8mm
// 4. Masslinie unterhalb der Bezugskante (default=nein)
// 5. Die Ueberlaenge(n) der Masshilfslinien ueber die Bezugskante (2x, default=0)
// 6. Den Ueberhang der Masshilfslinien ueber die Masslinie (default=2mm)
// 7. Den Abstand der Masshilfslinien zur Bezugskante
//
// Dragging:                    Handle          Shift
// ~~~~~~~~~
// -  Die Bezugspunkte        SolidQuadHdl   nur die Laenge
// 1.+2. Anpacken des Textes
// 3.+4. Hdl am Pfeil (2x)    SolidQuadHdl   nur den Bool
// 5.    Hdl am Endpunkt      CircHdl        beide Laengen?
// 6.+7. Kein Dragging
//
// Offen:
// ~~~~~~
// - Radien (gleich als Typ verankern
//
// Special:
// ~~~~~~~~
// Connecting an max. 2 Objekte
// -> Bei Copy, etc. den entspr. Code der Verbinder verwenden?!?
// wird wohl recht kompliziert werden ...
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOMEAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
