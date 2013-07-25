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
    SdrCaptionObj(const Rectangle& rRect, const Point& rTail);
    virtual ~SdrCaptionObj();

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual SdrCaptionObj* Clone() const;

    // for calc: special shadow only for text box
    void SetSpecialTextBoxShadow() { mbSpecialTextBoxShadow = sal_True; }
    sal_Bool GetSpecialTextBoxShadow() const { return mbSpecialTextBoxShadow; }

    // for calc: fixed note tail position.
    void SetFixedTail() { mbFixedTail = sal_True; }

    virtual OUString TakeObjNameSingul() const;
    virtual OUString TakeObjNamePlural() const;

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
    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;

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
