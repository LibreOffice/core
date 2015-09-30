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

class SVX_DLLPUBLIC SdrCircObj : public SdrRectObj
{
private:
    // to allow sdr::properties::CircleProperties access to ImpSetAttrToCircInfo()
    friend class sdr::properties::CircleProperties;

    // only for SdrCircleAttributes
    SdrObjKind GetCircleKind() const { return meCircleKind; }

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() SAL_OVERRIDE;

    SdrObjKind                  meCircleKind;
    long                        nStartAngle;
    long                        nEndAngle;
private:
     SVX_DLLPRIVATE basegfx::B2DPolygon ImpCalcXPolyCirc(const SdrObjKind eKind, const Rectangle& rRect1, long nStart, long nEnd) const;
    SVX_DLLPRIVATE void ImpSetCreateParams(SdrDragStat& rStat) const;
    SVX_DLLPRIVATE void ImpSetAttrToCircInfo(); // copy values from pool
    SVX_DLLPRIVATE void ImpSetCircInfoToAttr(); // copy values into pool

    // returns sal_True if paint requires a XPolygon
    SVX_DLLPRIVATE bool PaintNeedsXPolyCirc() const; // PaintNeedsXPoly-> PaintNeedsXPolyCirc
    SVX_DLLPRIVATE virtual void RecalcXPoly() SAL_OVERRIDE;

protected:
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();
    SdrCircObj(SdrObjKind eNewKind); // Circ, CArc, Sect or CCut
    SdrCircObj(SdrObjKind eNewKind, const Rectangle& rRect);

    // 0=0.00Deg=3h 9000=90.00Deg=12h 18000=180.00Deg=9h 27000=270.00Deg=6h
    // The circle is build up from StartAngle to EndWink anti-clockwise.
    // If nNewStartAngle==nNewEndWink, then arc has an angle of 0 degrees.
    // If nNewStartAngle+36000==nNewEndWink, then the arc has angle of 360 degrees.
    SdrCircObj(SdrObjKind eNewKind, const Rectangle& rRect, long nNewStartAngle, long nNewEndWink);
    virtual ~SdrCircObj();

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const SAL_OVERRIDE;
    virtual sal_uInt16 GetObjIdentifier() const SAL_OVERRIDE;
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const SAL_OVERRIDE;

    virtual OUString TakeObjNameSingul() const SAL_OVERRIDE;
    virtual OUString TakeObjNamePlural() const SAL_OVERRIDE;

    virtual SdrCircObj* Clone() const SAL_OVERRIDE;
    virtual void RecalcSnapRect() SAL_OVERRIDE;
    virtual void NbcSetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const SAL_OVERRIDE;

    virtual sal_uInt32 GetSnapPointCount() const SAL_OVERRIDE;
    virtual Point GetSnapPoint(sal_uInt32 i) const SAL_OVERRIDE;

    virtual sal_uInt32 GetHdlCount() const SAL_OVERRIDE;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const SAL_OVERRIDE;

    // special drag methods
    virtual bool hasSpecialDrag() const SAL_OVERRIDE;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const SAL_OVERRIDE;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) SAL_OVERRIDE;
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const SAL_OVERRIDE;

    virtual bool BegCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual bool MovCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) SAL_OVERRIDE;
    virtual bool BckCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual void BrkCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const SAL_OVERRIDE;
    virtual Pointer GetCreatePointer() const SAL_OVERRIDE;
    virtual void NbcMove(const Size& aSiz) SAL_OVERRIDE;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) SAL_OVERRIDE;
    virtual void NbcShear (const Point& rRef, long nAngle, double tn, bool bVShear) SAL_OVERRIDE;
    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const SAL_OVERRIDE;

protected:
    virtual SdrObjGeoData* NewGeoData() const SAL_OVERRIDE;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const SAL_OVERRIDE;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) SAL_OVERRIDE;
public:
    long GetStartAngle() const { return nStartAngle; }
    long GetEndAngle() const { return nEndAngle; }

};

#endif // INCLUDED_SVX_SVDOCIRC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
