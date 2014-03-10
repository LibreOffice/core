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

// #109872#
class SdrCircObjGeoData : public SdrTextObjGeoData
{
public:
    long                        nStartWink;
    long                        nEndWink;
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
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    SdrObjKind                  meCircleKind;
    long                        nStartWink;
    long                        nEndWink;
private:
     SVX_DLLPRIVATE basegfx::B2DPolygon ImpCalcXPolyCirc(const SdrObjKind eKind, const Rectangle& rRect1, long nStart, long nEnd) const;
    SVX_DLLPRIVATE void ImpSetCreateParams(SdrDragStat& rStat) const;
    SVX_DLLPRIVATE void ImpSetAttrToCircInfo(); // copy values from pool
    SVX_DLLPRIVATE void ImpSetCircInfoToAttr(); // copy values into pool

    // returns sal_True if paint requires a XPolygon
    SVX_DLLPRIVATE bool PaintNeedsXPolyCirc() const; // PaintNeedsXPoly-> PaintNeedsXPolyCirc
    SVX_DLLPRIVATE virtual void RecalcXPoly();

protected:
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

public:
    TYPEINFO();
    SdrCircObj(SdrObjKind eNewKind); // Circ, CArc, Sect or CCut
    SdrCircObj(SdrObjKind eNewKind, const Rectangle& rRect);

    // 0=0.00Deg=3h 9000=90.00Deg=12h 18000=180.00Deg=9h 27000=270.00Deg=6h
    // The circle is build up from StartWink to EndWink anti-clockwise.
    // If nNewStartWink==nNewEndWink, then arc has an angle of 0 degrees.
    // If nNewStartWink+36000==nNewEndWink, then the arc has angle of 360 degrees.
    SdrCircObj(SdrObjKind eNewKind, const Rectangle& rRect, long nNewStartWink, long nNewEndWink);
    virtual ~SdrCircObj();

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;

    virtual OUString TakeObjNameSingul() const;
    virtual OUString TakeObjNamePlural() const;

    virtual SdrCircObj* Clone() const;
    virtual void RecalcSnapRect();
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;

    virtual sal_uInt32 GetSnapPointCount() const;
    virtual Point GetSnapPoint(sal_uInt32 i) const;

    virtual sal_uInt32 GetHdlCount() const;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const;

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const;

    virtual bool BegCreate(SdrDragStat& rStat);
    virtual bool MovCreate(SdrDragStat& rStat);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;
    virtual Pointer GetCreatePointer() const;
    virtual void NbcMove(const Size& aSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear (const Point& rRef, long nWink, double tn, bool bVShear);
    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const;

protected:
    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);
public:
    long GetStartWink() const { return nStartWink; }
    long GetEndWink() const { return nEndWink; }

};

#endif // INCLUDED_SVX_SVDOCIRC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
