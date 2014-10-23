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

#include <svx/svdotext.hxx>
#include <svx/svxdllapi.h>


//   Vorausdeklarationen


class XPolygon;

namespace sdr { namespace properties {
    class RectangleProperties;
}}


//   SdrRectObj

// Rechteck-Objekte (Rechteck,Kreis,...)



class SVX_DLLPUBLIC SdrRectObj : public SdrTextObj
{
private:
    // to allow sdr::properties::RectangleProperties access to SetXPolyDirty()
    friend class sdr::properties::RectangleProperties;
    friend class                SdrTextObj; // wg SetXPolyDirty bei GrowAdjust

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() SAL_OVERRIDE;

    XPolygon*                   mpXPoly;

protected:
    XPolygon ImpCalcXPoly(const Rectangle& rRect1, long nRad1) const;
    void SetXPolyDirty();

    // RecalcXPoly sollte ueberladen werden. Dabei muss dann eine XPolygon
    // Instanz generiert (new) und an mpXPoly zugewiesen werden.
    virtual void RecalcXPoly();
    const XPolygon& GetXPoly() const;
    virtual void           RestGeoData(const SdrObjGeoData& rGeo) SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();
    // Der Eckenradius-Parameter fliegt irgendwann raus. Der Eckenradius
    // ist dann (spaeter) ueber SfxItems einzustellen (SetAttributes()).
    // Konstruktion eines Rechteck-Zeichenobjekts

    SdrRectObj();
    SdrRectObj(const Rectangle& rRect);

    SdrRectObj& operator=(const SdrRectObj& rCopy);

    // Konstruktion eines Textrahmens
    SdrRectObj(SdrObjKind eNewTextKind);
    SdrRectObj(SdrObjKind eNewTextKind, const Rectangle& rRect);
    virtual ~SdrRectObj();

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const SAL_OVERRIDE;
    virtual sal_uInt16 GetObjIdentifier() const SAL_OVERRIDE;
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const SAL_OVERRIDE;

    virtual OUString TakeObjNameSingul() const SAL_OVERRIDE;
    virtual OUString TakeObjNamePlural() const SAL_OVERRIDE;

    virtual SdrRectObj* Clone() const SAL_OVERRIDE;
    virtual void RecalcSnapRect() SAL_OVERRIDE;
    virtual void NbcSetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual void NbcSetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const SAL_OVERRIDE;

    virtual sal_uInt32 GetHdlCount() const SAL_OVERRIDE;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const SAL_OVERRIDE;

    // special drag methods
    virtual bool hasSpecialDrag() const SAL_OVERRIDE;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const SAL_OVERRIDE;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) SAL_OVERRIDE;
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const SAL_OVERRIDE;

    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const SAL_OVERRIDE;
    virtual Pointer GetCreatePointer() const SAL_OVERRIDE;

    virtual void NbcMove(const Size& rSiz) SAL_OVERRIDE;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs) SAL_OVERRIDE;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) SAL_OVERRIDE;
    virtual void NbcShear(const Point& rRef, long nWink, double tn, bool bVShear) SAL_OVERRIDE;

    virtual bool DoMacro(const SdrObjMacroHitRec& rRec) SAL_OVERRIDE;
    virtual OUString GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const SAL_OVERRIDE;

    virtual SdrGluePoint GetVertexGluePoint(sal_uInt16 nNum) const SAL_OVERRIDE;
    virtual SdrGluePoint GetCornerGluePoint(sal_uInt16 nNum) const SAL_OVERRIDE;

    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const SAL_OVERRIDE;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) SAL_OVERRIDE;
};

#endif // INCLUDED_SVX_SVDORECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
