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

#ifndef INCLUDED_SVX_SVDOMEAS_HXX
#define INCLUDED_SVX_SVDOMEAS_HXX

#include <svx/svdotext.hxx>
#include <svx/svxdllapi.h>
#include <editeng/measfld.hxx>

class SdrOutliner;
struct ImpMeasureRec;
struct ImpMeasurePoly;

namespace sdr { namespace properties {
    class MeasureProperties;
}}

class SdrMeasureObjGeoData : public SdrTextObjGeoData
{
public:
    Point                       aPt1;
    Point                       aPt2;

public:
    SdrMeasureObjGeoData();
    virtual ~SdrMeasureObjGeoData() override;
};

class SVX_DLLPUBLIC SdrMeasureObj : public SdrTextObj
{
private:
    // to allow sdr::properties::MeasureProperties access to SetTextDirty()
    friend class sdr::properties::MeasureProperties;

    friend class                SdrMeasureField;

protected:
    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;
    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;

    Point                       aPt1;
    Point                       aPt2;
    bool                        bTextDirty;

protected:
    void ImpTakeAttr(ImpMeasureRec& rRec) const;
    OUString TakeRepresentation(SdrMeasureFieldKind eMeasureFieldKind) const;
    void ImpCalcGeometrics(const ImpMeasureRec& rRec, ImpMeasurePoly& rPol) const;
    static basegfx::B2DPolyPolygon ImpCalcXPoly(const ImpMeasurePoly& rPol);
    void ImpEvalDrag(ImpMeasureRec& rRec, const SdrDragStat& rDrag) const;
    void SetTextDirty() { bTextDirty=true; SetTextSizeDirty(); if (!aOutRect.IsEmpty()) { SetBoundRectDirty(); SetRectsDirty(true); } }
    void UndirtyText() const;

    virtual SdrObjGeoData* NewGeoData() const override;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) override;

    // protected destructor
    virtual ~SdrMeasureObj() override;

public:
    SdrMeasureObj(SdrModel& rSdrModel);
    SdrMeasureObj(
        SdrModel& rSdrModel,
        const Point& rPt1,
        const Point& rPt2);

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual void TakeUnrotatedSnapRect(tools::Rectangle& rRect) const override;
    virtual SdrMeasureObj* CloneSdrObject(SdrModel& rTargetModel) const override;

    // implemented mainly for the purposes of Clone()
    SdrMeasureObj& operator=(const SdrMeasureObj& rObj);

    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;
    virtual sal_uInt32 GetHdlCount() const override;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const override;

    // special drag methods
    virtual bool hasSpecialDrag() const override;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const override;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) override;
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const override;

    virtual bool BegCreate(SdrDragStat& rStat) override;
    virtual bool MovCreate(SdrDragStat& rStat) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;
    virtual bool BckCreate(SdrDragStat& rStat) override;
    virtual void BrkCreate(SdrDragStat& rStat) override;
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const override;
    virtual PointerStyle GetCreatePointer() const override;

    virtual void NbcMove(const Size& rSiz) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcRotate(const Point& rRef, long nAngle, double sn, double cs) override;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
    virtual void NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear) override;
    virtual long GetRotateAngle() const override;
    virtual void RecalcSnapRect() override;

    virtual sal_uInt32 GetSnapPointCount() const override;
    virtual Point GetSnapPoint(sal_uInt32 i) const override;

    virtual bool IsPolyObj() const override;
    virtual sal_uInt32 GetPointCount() const override;
    virtual Point GetPoint(sal_uInt32 i) const override;
    virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i) override;

    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    virtual bool BegTextEdit(SdrOutliner& rOutl) override;
    virtual const Size& GetTextSize() const override;
    virtual void TakeTextRect( SdrOutliner& rOutliner, tools::Rectangle& rTextRect, bool bNoEditText,
        tools::Rectangle* pAnchorRect, bool bLineWidth = true ) const override;
    virtual void TakeTextAnchorRect(tools::Rectangle& rAnchorRect) const override;
    virtual void TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, tools::Rectangle* pViewInit, tools::Rectangle* pViewMin) const override;
    virtual EEAnchorMode GetOutlinerViewAnchorMode() const override;
    virtual void NbcSetOutlinerParaObject(std::unique_ptr<OutlinerParaObject> pTextObject) override;
    virtual OutlinerParaObject* GetOutlinerParaObject() const override;

    virtual bool CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos,
        bool bEdit, boost::optional<Color>& rpTxtColor, boost::optional<Color>& rpFldColor, OUString& rRet) const override;

    // #i97878#
    virtual bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const override;
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon) override;
};

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
//     #             #         |___ <-excess length of the auxiliary dimension line(s)
//     #             #=========#
//     # objected to be dimensioned #
//     #=======================#
//
// Attributes:
// ~~~~~~~~~~
// 1. Where is the text: middle, right or left (default=automatic)
// 2. Text above or below the line or line broken by text (default=automatic)
//    Default=8mm
// 3. The distance from the dimension line to the reference edge (= to the dimensioned object)
// 4. dimension line below the reference edge (default=no))
// 5. excess length of the auxiliary dimension lines beyond the reference edge (2x, default=0)
// 6. excess length of the auxiliary dimension lines past the dimension line (default=2mm)
// 7. distance between the auxiliary dimension line and the reference edge
//
// Dragging:                    Handle          Shift
// ~~~~~~~~~
// -  reference points        SolidQuadHdl   only the length
// 1.+2. Seizing the text
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

#endif // INCLUDED_SVX_SVDOMEAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
