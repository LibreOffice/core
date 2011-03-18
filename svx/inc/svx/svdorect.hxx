/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVDORECT_HXX
#define _SVDORECT_HXX

#include <svx/svdotext.hxx>
#include "svx/svxdllapi.h"

//************************************************************
//   Vorausdeklarationen
//************************************************************

class XPolygon;

namespace sdr { namespace properties {
    class RectangleProperties;
}}

//************************************************************
//   SdrRectObj
//
// Rechteck-Objekte (Rechteck,Kreis,...)
//
//************************************************************

class SVX_DLLPUBLIC SdrRectObj : public SdrTextObj
{
private:
    // to allow sdr::properties::RectangleProperties access to SetXPolyDirty()
    friend class sdr::properties::RectangleProperties;
    friend class                SdrTextObj; // wg SetXPolyDirty bei GrowAdjust

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    XPolygon*                   mpXPoly;

protected:
    // Liefert sal_True, wenn das Painten ein Polygon erfordert.
    bool PaintNeedsXPoly(long nEckRad) const;

protected:
    XPolygon ImpCalcXPoly(const Rectangle& rRect1, long nRad1) const;
    void SetXPolyDirty();

    // RecalcXPoly sollte ueberladen werden. Dabei muss dann eine XPolygon
    // Instanz generiert (new) und an mpXPoly zugewiesen werden.
    virtual void RecalcXPoly();
    const XPolygon& GetXPoly() const;
    virtual void           RestGeoData(const SdrObjGeoData& rGeo);

public:
    TYPEINFO();
    // Der Eckenradius-Parameter fliegt irgendwann raus. Der Eckenradius
    // ist dann (spaeter) ueber SfxItems einzustellen (SetAttributes()).
    // Konstruktion eines Rechteck-Zeichenobjekts

    SdrRectObj();
    SdrRectObj(const Rectangle& rRect);

    // Konstruktion eines Textrahmens
    SdrRectObj(SdrObjKind eNewTextKind);
    SdrRectObj(SdrObjKind eNewTextKind, const Rectangle& rRect);
    // der sal_uInt16 eFormat nimmt Werte des enum EETextFormat entgegen
    SdrRectObj(SdrObjKind eNewTextKind, const Rectangle& rNewRect, SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat);
    virtual ~SdrRectObj();

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual void operator=(const SdrObject& rObj);
    virtual void RecalcSnapRect();
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;

    virtual sal_uInt32 GetHdlCount() const;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const;

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;

    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;
    virtual Pointer GetCreatePointer() const;

    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear(const Point& rRef, long nWink, double tn, bool bVShear);

    virtual bool DoMacro(const SdrObjMacroHitRec& rRec);
    virtual XubString GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const;

    virtual SdrGluePoint GetVertexGluePoint(sal_uInt16 nNum) const;
    virtual SdrGluePoint GetCornerGluePoint(sal_uInt16 nNum) const;

    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier) const;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);
};

#endif //_SVDORECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
