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

#ifndef _SVDOCIRC_HXX
#define _SVDOCIRC_HXX

#include <svx/svdorect.hxx>
#include "svx/svxdllapi.h"

//************************************************************
//   Vorausdeklarationen
//************************************************************

namespace sdr { namespace properties {
    class CircleProperties;
}}

//************************************************************
//   Hilfsklasse SdrCircObjGeoData
//************************************************************

// #109872#
class SdrCircObjGeoData : public SdrTextObjGeoData
{
public:
    long                        nStartWink;
    long                        nEndWink;
};

//************************************************************
//   SdrCircObj
//************************************************************

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

    // bitfield
    unsigned                    mbPolygonIsLine : 1;

private:
     SVX_DLLPRIVATE basegfx::B2DPolygon ImpCalcXPolyCirc(const SdrObjKind eKind, const Rectangle& rRect1, long nStart, long nEnd) const;
    SVX_DLLPRIVATE void ImpSetCreateParams(SdrDragStat& rStat) const;
    SVX_DLLPRIVATE void ImpSetAttrToCircInfo(); // Werte vom Pool kopieren
    SVX_DLLPRIVATE void ImpSetCircInfoToAttr(); // Werte in den Pool kopieren

    // Liefert sal_True, wenn das Painten ein XPolygon erfordert.
    SVX_DLLPRIVATE FASTBOOL PaintNeedsXPolyCirc() const; // PaintNeedsXPoly-> PaintNeedsXPolyCirc
    SVX_DLLPRIVATE virtual void RecalcXPoly();

protected:
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

public:
    TYPEINFO();
    SdrCircObj(SdrObjKind eNewKind); // Circ, CArc, Sect oder CCut
    SdrCircObj(SdrObjKind eNewKind, const Rectangle& rRect);

    // 0=0.00Deg=3h 9000=90.00Deg=12h 18000=180.00Deg=9h 27000=270.00Deg=6h
    // Der Verlauf des Kreises von StartWink nach EndWink ist immer entgegen
    // dem Uhrzeigersinn.
    // Wenn nNewStartWink==nNewEndWink hat der Kreisbogen einen Verlaufswinkel
    // von 0 Grad. Bei nNewStartWink+36000==nNewEndWink ist der Verlaufswinkel
    // 360.00 Grad.
    SdrCircObj(SdrObjKind eNewKind, const Rectangle& rRect, long nNewStartWink, long nNewEndWink);
    virtual ~SdrCircObj();

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual void operator=(const SdrObject& rObj);
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
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;

    virtual FASTBOOL BegCreate(SdrDragStat& rStat);
    virtual FASTBOOL MovCreate(SdrDragStat& rStat);
    virtual FASTBOOL EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual FASTBOOL BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;
    virtual Pointer GetCreatePointer() const;
    virtual void NbcMove(const Size& aSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear (const Point& rRef, long nWink, double tn, FASTBOOL bVShear);
    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier) const;

protected:
    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);
public:
    long GetStartWink() const { return nStartWink; }
    long GetEndWink() const { return nEndWink; }

};

#endif //_SVDOCIRC_HXX

