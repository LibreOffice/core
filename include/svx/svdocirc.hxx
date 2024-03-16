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

#include <svx/svdorect.hxx>
#include <svx/svxdllapi.h>

namespace sdr::properties {
    class CircleProperties;
}

// Helper class SdrCircObjGeoData

class SdrCircObjGeoData final : public SdrTextObjGeoData
{
public:
    Degree100     nStartAngle;
    Degree100     nEndAngle;
};

enum class SdrCircKind { Full, Section, Cut, Arc };

extern SVXCORE_DLLPUBLIC SdrCircKind ToSdrCircKind(SdrObjKind);


class SVXCORE_DLLPUBLIC SdrCircObj final : public SdrRectObj
{
private:
    // to allow sdr::properties::CircleProperties access to ImpSetAttrToCircInfo()
    friend class sdr::properties::CircleProperties;

    SAL_DLLPRIVATE virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;
    SAL_DLLPRIVATE virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;

    SdrCircKind                 meCircleKind;
    Degree100                   nStartAngle;
    Degree100                   nEndAngle;

    SVX_DLLPRIVATE basegfx::B2DPolygon ImpCalcXPolyCirc(const SdrCircKind eKind, const tools::Rectangle& rRect1, Degree100 nStart, Degree100 nEnd) const;
    SVX_DLLPRIVATE static void ImpSetCreateParams(SdrDragStat& rStat);
    SVX_DLLPRIVATE void ImpSetAttrToCircInfo(); // copy values from pool
    SVX_DLLPRIVATE void ImpSetCircInfoToAttr(); // copy values into pool

    // returns sal_True if paint requires a XPolygon
    SVX_DLLPRIVATE bool PaintNeedsXPolyCirc() const; // PaintNeedsXPoly-> PaintNeedsXPolyCirc
    SVX_DLLPRIVATE virtual void RecalcXPoly() override;

    SAL_DLLPRIVATE virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

private:
    // protected destructor - due to final, make private
    SAL_DLLPRIVATE virtual ~SdrCircObj() override;

public:
    SAL_DLLPRIVATE SdrCircObj(
        SdrModel& rSdrModel,
        SdrCircKind eNewKind);
    SdrCircObj(
        SdrModel& rSdrModel,
        SdrCircKind eNewKind,
        const tools::Rectangle& rRect);
    // Copy constructor
    SAL_DLLPRIVATE SdrCircObj(SdrModel& rSdrModel, SdrCircObj const & rSource);

    // 0=0.00Deg=3h 9000=90.00Deg=12h 18000=180.00Deg=9h 27000=270.00Deg=6h
    // The circle is build up from StartAngle to EndWink anti-clockwise.
    // If nNewStartAngle==nNewEndWink, then arc has an angle of 0 degrees.
    // If nNewStartAngle+36000==nNewEndWink, then the arc has angle of 360 degrees.
    SdrCircObj(
        SdrModel& rSdrModel,
        SdrCircKind eNewKind,
        const tools::Rectangle& rRect,
        Degree100 nNewStartAngle,
        Degree100 nNewEndAngle);

    SdrCircKind GetCircleKind() const { return meCircleKind; }

    SAL_DLLPRIVATE virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    SAL_DLLPRIVATE virtual SdrObjKind GetObjIdentifier() const override;
    SAL_DLLPRIVATE virtual void TakeUnrotatedSnapRect(tools::Rectangle& rRect) const override;

    SAL_DLLPRIVATE virtual OUString TakeObjNameSingul() const override;
    SAL_DLLPRIVATE virtual OUString TakeObjNamePlural() const override;

    SAL_DLLPRIVATE virtual rtl::Reference<SdrObject> CloneSdrObject(SdrModel& rTargetModel) const override;

    SAL_DLLPRIVATE virtual void RecalcSnapRect() override;
    SAL_DLLPRIVATE virtual void NbcSetSnapRect(const tools::Rectangle& rRect) override;
    SAL_DLLPRIVATE virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;

    SAL_DLLPRIVATE virtual sal_uInt32 GetSnapPointCount() const override;
    SAL_DLLPRIVATE virtual Point GetSnapPoint(sal_uInt32 i) const override;

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
    SAL_DLLPRIVATE virtual void NbcMove(const Size& aSiz) override;
    SAL_DLLPRIVATE virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    SAL_DLLPRIVATE virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
    SAL_DLLPRIVATE virtual void NbcShear (const Point& rRef, Degree100 nAngle, double tn, bool bVShear) override;
    SAL_DLLPRIVATE virtual rtl::Reference<SdrObject> DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

private:
    SAL_DLLPRIVATE virtual std::unique_ptr<SdrObjGeoData> NewGeoData() const override;
    SAL_DLLPRIVATE virtual void SaveGeoData(SdrObjGeoData& rGeo) const override;
    SAL_DLLPRIVATE virtual void RestoreGeoData(const SdrObjGeoData& rGeo) override;
public:
    Degree100 GetStartAngle() const { return nStartAngle; }
    Degree100 GetEndAngle() const { return nEndAngle; }

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
