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

#ifndef INCLUDED_SVX_SVDOCIRC_HXX
#define INCLUDED_SVX_SVDOCIRC_HXX

#include <svx/svdorect.hxx>
#include <svx/svxdllapi.h>

namespace sdr { namespace properties {
    class CircleProperties;
}}

// Helper class SdrCircObjGeoData

class SdrCircObjGeoData : public SdrTextObjGeoData
{
public:
    long                        nStartAngle;
    long                        nEndAngle;
};

// class SdrCircObj

class SVX_DLLPUBLIC SdrCircObj final : public SdrRectObj
{
private:
    // to allow sdr::properties::CircleProperties access to ImpSetAttrToCircInfo()
    friend class sdr::properties::CircleProperties;

    // only for SdrCircleAttributes
    SdrObjKind GetCircleKind() const { return meCircleKind; }

    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;
    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;

    SdrObjKind                  meCircleKind;
    long                        nStartAngle;
    long                        nEndAngle;

    SVX_DLLPRIVATE basegfx::B2DPolygon ImpCalcXPolyCirc(const SdrObjKind eKind, const tools::Rectangle& rRect1, long nStart, long nEnd) const;
    SVX_DLLPRIVATE static void ImpSetCreateParams(SdrDragStat& rStat);
    SVX_DLLPRIVATE void ImpSetAttrToCircInfo(); // copy values from pool
    SVX_DLLPRIVATE void ImpSetCircInfoToAttr(); // copy values into pool

    // returns sal_True if paint requires a XPolygon
    SVX_DLLPRIVATE bool PaintNeedsXPolyCirc() const; // PaintNeedsXPoly-> PaintNeedsXPolyCirc
    SVX_DLLPRIVATE virtual void RecalcXPoly() override;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

private:
    // protected destructor - due to final, make private
    virtual ~SdrCircObj() override;

public:
    SdrCircObj(
        SdrModel& rSdrModel,
        SdrObjKind eNewKind); // Circ, CArc, Sect or CCut
    SdrCircObj(
        SdrModel& rSdrModel,
        SdrObjKind eNewKind,
        const tools::Rectangle& rRect);

    // 0=0.00Deg=3h 9000=90.00Deg=12h 18000=180.00Deg=9h 27000=270.00Deg=6h
    // The circle is build up from StartAngle to EndWink anti-clockwise.
    // If nNewStartAngle==nNewEndWink, then arc has an angle of 0 degrees.
    // If nNewStartAngle+36000==nNewEndWink, then the arc has angle of 360 degrees.
    SdrCircObj(
        SdrModel& rSdrModel,
        SdrObjKind eNewKind,
        const tools::Rectangle& rRect,
        long nNewStartAngle,
        long nNewEndWink);

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual void TakeUnrotatedSnapRect(tools::Rectangle& rRect) const override;

    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    virtual SdrCircObj* CloneSdrObject(SdrModel& rTargetModel) const override;

    // implemented mainly for the purposes of Clone()
    SdrCircObj& operator=(const SdrCircObj& rObj);

    virtual void RecalcSnapRect() override;
    virtual void NbcSetSnapRect(const tools::Rectangle& rRect) override;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;

    virtual sal_uInt32 GetSnapPointCount() const override;
    virtual Point GetSnapPoint(sal_uInt32 i) const override;

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
    virtual void NbcMove(const Size& aSiz) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
    virtual void NbcShear (const Point& rRef, long nAngle, double tn, bool bVShear) override;
    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

private:
    virtual SdrObjGeoData* NewGeoData() const override;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) override;
public:
    long GetStartAngle() const { return nStartAngle; }
    long GetEndAngle() const { return nEndAngle; }

};

#endif // INCLUDED_SVX_SVDOCIRC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
