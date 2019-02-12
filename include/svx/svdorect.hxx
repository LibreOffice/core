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

#ifndef INCLUDED_SVX_SVDORECT_HXX
#define INCLUDED_SVX_SVDORECT_HXX

#include <memory>
#include <svx/svdotext.hxx>
#include <svx/svxdllapi.h>


// Forward declaration
class XPolygon;

namespace sdr { namespace properties {
    class RectangleProperties;
}}


//   SdrRectObj

/// Rectangle objects (rectangle, circle, ...)
class SVX_DLLPUBLIC SdrRectObj : public SdrTextObj
{
private:
    // to allow sdr::properties::RectangleProperties access to SetXPolyDirty()
    friend class sdr::properties::RectangleProperties;
    friend class SdrTextObj; // Due to SetXPolyDirty for GrowAdjust

protected:
    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;
    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;

    std::unique_ptr<XPolygon> mpXPoly;

protected:
    XPolygon ImpCalcXPoly(const tools::Rectangle& rRect1, long nRad1) const;
    void SetXPolyDirty();

    /// Subclasses should override RecalcXPoly() by creating an XPolygon
    /// instance with new and assigning it to mpXPoly.
    virtual void RecalcXPoly();
    const XPolygon& GetXPoly() const;
    virtual void           RestGeoData(const SdrObjGeoData& rGeo) override;

    // protected destructor
    virtual ~SdrRectObj() override;

public:
    /**
     * The corner radius parameter is dropped at some point.
     * We need to set the corner radius via SfxItems later on, using SetAttributes()
     *
     * Constructor of a rectangular drawing object
     */
    SdrRectObj(SdrModel& rSdrModel);
    SdrRectObj(
        SdrModel& rSdrModel,
        const tools::Rectangle& rRect);

    SdrRectObj& operator=(const SdrRectObj& rCopy);

    // Constructor of a text frame
    SdrRectObj(
        SdrModel& rSdrModel,
        SdrObjKind eNewTextKind);
    SdrRectObj(
        SdrModel& rSdrModel,
        SdrObjKind eNewTextKind,
        const tools::Rectangle& rRect);

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual void TakeUnrotatedSnapRect(tools::Rectangle& rRect) const override;

    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    virtual SdrRectObj* CloneSdrObject(SdrModel& rTargetModel) const override;
    virtual void RecalcSnapRect() override;
    virtual void NbcSetSnapRect(const tools::Rectangle& rRect) override;
    virtual void NbcSetLogicRect(const tools::Rectangle& rRect) override;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;

    virtual sal_uInt32 GetHdlCount() const override;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const override;

    // Special drag methods
    virtual bool hasSpecialDrag() const override;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const override;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) override;
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const override;

    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const override;
    virtual PointerStyle GetCreatePointer() const override;

    virtual void NbcMove(const Size& rSiz) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcRotate(const Point& rRef, long nAngle, double sn, double cs) override;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
    virtual void NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear) override;

    virtual SdrGluePoint GetVertexGluePoint(sal_uInt16 nNum) const override;
    virtual SdrGluePoint GetCornerGluePoint(sal_uInt16 nNum) const override;

    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;
};

#endif // INCLUDED_SVX_SVDORECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
