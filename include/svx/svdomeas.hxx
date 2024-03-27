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

#pragma once

#include <svx/svdotext.hxx>
#include <svx/svxdllapi.h>
#include <editeng/measfld.hxx>

class SdrOutliner;
struct ImpMeasureRec;
struct ImpMeasurePoly;

namespace sdr::properties {
    class MeasureProperties;
}

class SdrMeasureObjGeoData final : public SdrTextObjGeoData
{
public:
    Point                       aPt1;
    Point                       aPt2;

public:
    SdrMeasureObjGeoData();
    virtual ~SdrMeasureObjGeoData() override;
};

class SVXCORE_DLLPUBLIC SdrMeasureObj final : public SdrTextObj
{
private:
    // to allow sdr::properties::MeasureProperties access to SetTextDirty()
    friend class sdr::properties::MeasureProperties;
    friend class                SdrMeasureField;

    SAL_DLLPRIVATE virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;
    SAL_DLLPRIVATE virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;

    Point                       aPt1;
    Point                       aPt2;
    bool                        bTextDirty;

    SAL_DLLPRIVATE void ImpTakeAttr(ImpMeasureRec& rRec) const;
    SAL_DLLPRIVATE OUString TakeRepresentation(SdrMeasureFieldKind eMeasureFieldKind) const;
    SAL_DLLPRIVATE void ImpCalcGeometrics(const ImpMeasureRec& rRec, ImpMeasurePoly& rPol) const;
    SAL_DLLPRIVATE static basegfx::B2DPolyPolygon ImpCalcXPoly(const ImpMeasurePoly& rPol);
    SAL_DLLPRIVATE void ImpEvalDrag(ImpMeasureRec& rRec, const SdrDragStat& rDrag) const;
    void SetTextDirty()
    {
        bTextDirty=true;
        SetTextSizeDirty();
        if (!getOutRectangle().IsEmpty())
        {
            SetBoundRectDirty();
            SetBoundAndSnapRectsDirty(/*bNotMyself*/true);
        }
    }
    SAL_DLLPRIVATE void UndirtyText() const;

    SAL_DLLPRIVATE virtual std::unique_ptr<SdrObjGeoData> NewGeoData() const override;
    SAL_DLLPRIVATE virtual void SaveGeoData(SdrObjGeoData& rGeo) const override;
    SAL_DLLPRIVATE virtual void RestoreGeoData(const SdrObjGeoData& rGeo) override;

public:
    SAL_DLLPRIVATE SdrMeasureObj(SdrModel& rSdrModel);
    // Copy constructor
    SAL_DLLPRIVATE SdrMeasureObj(SdrModel& rSdrModel, SdrMeasureObj const & rSource);
    SdrMeasureObj(
        SdrModel& rSdrModel,
        const Point& rPt1,
        const Point& rPt2);
    SAL_DLLPRIVATE virtual ~SdrMeasureObj() override;

    SAL_DLLPRIVATE virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    SAL_DLLPRIVATE virtual SdrObjKind GetObjIdentifier() const override;
    virtual void TakeUnrotatedSnapRect(tools::Rectangle& rRect) const override;
    SAL_DLLPRIVATE virtual rtl::Reference<SdrObject> CloneSdrObject(SdrModel& rTargetModel) const override;

    SAL_DLLPRIVATE virtual OUString TakeObjNameSingul() const override;
    SAL_DLLPRIVATE virtual OUString TakeObjNamePlural() const override;

    SAL_DLLPRIVATE virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;
    SAL_DLLPRIVATE virtual sal_uInt32 GetHdlCount() const override;
    SAL_DLLPRIVATE virtual void AddToHdlList(SdrHdlList& rHdlList) const override;

    // special drag methods
    SAL_DLLPRIVATE virtual bool hasSpecialDrag() const override;
    SAL_DLLPRIVATE virtual bool beginSpecialDrag(SdrDragStat& rDrag) const override;
    SAL_DLLPRIVATE virtual bool applySpecialDrag(SdrDragStat& rDrag) override;
    SAL_DLLPRIVATE virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const override;

    SAL_DLLPRIVATE virtual bool BegCreate(SdrDragStat& rStat) override;
    SAL_DLLPRIVATE virtual bool MovCreate(SdrDragStat& rStat) override;
    SAL_DLLPRIVATE virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;
    SAL_DLLPRIVATE virtual bool BckCreate(SdrDragStat& rStat) override;
    SAL_DLLPRIVATE virtual void BrkCreate(SdrDragStat& rStat) override;
    SAL_DLLPRIVATE virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const override;
    SAL_DLLPRIVATE virtual PointerStyle GetCreatePointer() const override;

    SAL_DLLPRIVATE virtual void NbcMove(const Size& rSiz) override;
    SAL_DLLPRIVATE virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    SAL_DLLPRIVATE virtual void NbcRotate(const Point& rRef, Degree100 nAngle, double sn, double cs) override;
    SAL_DLLPRIVATE virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
    SAL_DLLPRIVATE virtual void NbcShear(const Point& rRef, Degree100 nAngle, double tn, bool bVShear) override;
    SAL_DLLPRIVATE virtual Degree100 GetRotateAngle() const override;
    SAL_DLLPRIVATE virtual void RecalcSnapRect() override;

    SAL_DLLPRIVATE virtual sal_uInt32 GetSnapPointCount() const override;
    SAL_DLLPRIVATE virtual Point GetSnapPoint(sal_uInt32 i) const override;

    SAL_DLLPRIVATE virtual bool IsPolyObj() const override;
    SAL_DLLPRIVATE virtual sal_uInt32 GetPointCount() const override;
    virtual Point GetPoint(sal_uInt32 i) const override;
    SAL_DLLPRIVATE virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i) override;

    SAL_DLLPRIVATE virtual rtl::Reference<SdrObject> DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    SAL_DLLPRIVATE virtual bool BegTextEdit(SdrOutliner& rOutl) override;
    SAL_DLLPRIVATE virtual const Size& GetTextSize() const override;
    SAL_DLLPRIVATE virtual void TakeTextRect( SdrOutliner& rOutliner, tools::Rectangle& rTextRect, bool bNoEditText,
        tools::Rectangle* pAnchorRect, bool bLineWidth = true ) const override;
    SAL_DLLPRIVATE virtual void TakeTextAnchorRect(tools::Rectangle& rAnchorRect) const override;
    SAL_DLLPRIVATE virtual void TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, tools::Rectangle* pViewInit, tools::Rectangle* pViewMin) const override;
    SAL_DLLPRIVATE virtual EEAnchorMode GetOutlinerViewAnchorMode() const override;
    SAL_DLLPRIVATE virtual void NbcSetOutlinerParaObject(std::optional<OutlinerParaObject> pTextObject) override;
    SAL_DLLPRIVATE virtual OutlinerParaObject* GetOutlinerParaObject() const override;

    SAL_DLLPRIVATE virtual bool CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos,
        bool bEdit, std::optional<Color>& rpTxtColor, std::optional<Color>& rpFldColor, std::optional<FontLineStyle>& rpFldLineStyle, OUString& rRet) const override;

    // #i97878#
    SAL_DLLPRIVATE virtual bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const override;
    SAL_DLLPRIVATE virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon) override;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
