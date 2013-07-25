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
//  Initial Declarations
//************************************************************

class SdrOutliner;
struct ImpMeasureRec;
struct ImpMeasurePoly;

namespace sdr { namespace properties {
    class MeasureProperties;
}}

//************************************************************
//   Auxiliary Class SdrMeasureObjGeoData
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
    OUString TakeRepresentation(SdrMeasureFieldKind eMeasureFieldKind) const;
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

    virtual OUString TakeObjNameSingul() const;
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

    virtual bool CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos,
        bool bEdit, Color*& rpTxtColor, Color*& rpFldColor, String& rRet) const;

    // #i97878#
    virtual sal_Bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const;
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon);
};

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Creating:
// ~~~~~~~~~
// dragging reference point 1 to reference point 2 -> reference edge
//
// Defaults:
// ~~~~~~~~~~~~~
// dimension line and auxiliary dimension lines:  solid black hairlines
// arrows :     2mm x 4mm
// text size
//                              ___
//     |        dimension      | 2mm
//     |<--------------------->|---
//     |                       | 8mm
//     |                       |
//    Pt1============#        Pt2-- <----reference edge (from Pt1 to Pt2)
//     #             #         |___ <-excess length of the auxiliary dimention line(s)
//     #             #=========#
//     # objected to be dimensioned #
//     #=======================#
//
// Attributes:
// ~~~~~~~~~~
// 1. Where is the text: middle, right or left (default=automatic)
// 2. Text above or below the line or line broken by text (default=automatic)
//    Default=8mm
// 3. The distance from the dimension line to the reference edge (= to the dimesnioned object)
// 4. dimension line below the reference edge (default=no))
// 5. excess lenth of the auxiliary dimension lines beyond the reference edge (2x, default=0)
// 6. excess lenth of the auxiliary dimension lines past the dimesion line (default=2mm)
// 7. distance between the auxiliary dimension line and the reference edge
//
// Dragging:                    Handle          Shift
// ~~~~~~~~~
// -  reference points        SolidQuadHdl   only the length
// 1.+2. Anpacken des Textes
// 3.+4. Hdl on arrow (2x)    SolidQuadHdl   only the  Bool
// 5.    Hdl one end point      CircHdl        both lengths ?
// 6.+7. no dragging
//
// Open:
// ~~~~~~
// - radiuses  (anchor as type immediately)
//
// Special:
// ~~~~~~~~
// Connecting to a maximum of two objects
// -> during Copy, etc. use the respective code of the connectors?!?
// this probably will be pretty complicated ...
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOMEAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
