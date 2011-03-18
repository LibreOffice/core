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

#ifndef _SVDCAPT_HXX
#define _SVDCAPT_HXX

#include <svx/svdorect.hxx>
#include "svx/svxdllapi.h"

//************************************************************
//   Vorausdeklarationen
//************************************************************

class ImpCaptParams;

namespace sdr { namespace properties {
    class CaptionProperties;
}}

#define SDRSETITEM_CAPTION_ATTR     SDRSETITEM_ATTR_COUNT

//************************************************************
//   Hilfsklasse SdrCaptObjGeoData
//************************************************************

// #109872#
class SdrCaptObjGeoData : public SdrTextObjGeoData
{
public:
    Polygon                     aTailPoly;
};

//************************************************************
//   SdrCaptionObj
//************************************************************

class SVX_DLLPUBLIC SdrCaptionObj : public SdrRectObj
{
private:
    // to allow sdr::properties::CaptionProperties access to ImpRecalcTail()
    friend class sdr::properties::CaptionProperties;
    friend class                SdrTextObj; // fuer ImpRecalcTail() bei AutoGrow

protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

private:
    Polygon                     aTailPoly;  // das ganze Polygon des Schwanzes
    sal_Bool                    mbSpecialTextBoxShadow; // for calc special shadow, default FALSE
    sal_Bool                    mbFixedTail; // for calc note box fixed tail, default FALSE
    Point                       maFixedTailPos; // for calc note box fixed tail position.

private:
    SVX_DLLPRIVATE void ImpGetCaptParams(ImpCaptParams& rPara) const;
    SVX_DLLPRIVATE void ImpCalcTail1(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const;
    SVX_DLLPRIVATE void ImpCalcTail2(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const;
    SVX_DLLPRIVATE void ImpCalcTail3(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const;
    SVX_DLLPRIVATE void ImpCalcTail4(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const;
    SVX_DLLPRIVATE void ImpCalcTail (const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const;
    SVX_DLLPRIVATE void ImpRecalcTail();

public:
    TYPEINFO();
    SdrCaptionObj();
    SdrCaptionObj(const Rectangle& rRect);
    SdrCaptionObj(const Rectangle& rRect, const Point& rTail);
    virtual ~SdrCaptionObj();

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void operator=(const SdrObject& rObj);

    // for calc: special shadow only for text box
    void SetSpecialTextBoxShadow() { mbSpecialTextBoxShadow = sal_True; }
    sal_Bool GetSpecialTextBoxShadow() const { return mbSpecialTextBoxShadow; }

    // for calc: fixed note tail position.
    void SetFixedTail() { mbFixedTail = sal_True; }

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual void SetModel(SdrModel* pNewModel);
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    virtual sal_uInt32 GetHdlCount() const;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const;

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;

    virtual bool BegCreate(SdrDragStat& rStat);
    virtual bool MovCreate(SdrDragStat& rStat);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;
    virtual Pointer GetCreatePointer() const;

    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);

    virtual void NbcSetRelativePos(const Point& rPnt);
    virtual Point GetRelativePos() const;
    virtual void NbcSetAnchorPos(const Point& rPnt);
    virtual const Point& GetAnchorPos() const;

    virtual void RecalcSnapRect();
    virtual const Rectangle& GetSnapRect() const;
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual const Rectangle& GetLogicRect() const;
    virtual void NbcSetLogicRect(const Rectangle& rRect);

    virtual sal_uInt32 GetSnapPointCount() const;
    virtual Point GetSnapPoint(sal_uInt32 i) const;

protected:
    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);

public:
    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier) const;

    const Point& GetTailPos() const;
    void SetTailPos(const Point& rPos);
    void NbcSetTailPos(const Point& rPos);

    // #i32599#
    // Add own implementation for TRSetBaseGeometry to handle TailPos over changes
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon);

    inline const Point& GetFixedTailPos() const  {return maFixedTailPos;}

    // geometry access
    ::basegfx::B2DPolygon getTailPolygon() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOCAPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
