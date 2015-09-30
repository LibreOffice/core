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

#ifndef INCLUDED_SVX_SVDOCAPT_HXX
#define INCLUDED_SVX_SVDOCAPT_HXX

#include <svx/svdorect.hxx>
#include <svx/svxdllapi.h>


//   Forward Declarations


class ImpCaptParams;

namespace sdr { namespace properties {
    class CaptionProperties;
}}


//   Helper Class SdrCaptObjGeoData


class SdrCaptObjGeoData : public SdrTextObjGeoData
{
public:
    tools::Polygon aTailPoly;
};


//   SdrCaptionObj


class SVX_DLLPUBLIC SdrCaptionObj : public SdrRectObj
{
private:
    // to allow sdr::properties::CaptionProperties access to ImpRecalcTail()
    friend class sdr::properties::CaptionProperties;
    friend class                SdrTextObj; // for ImpRecalcTail() during AutoGrow

protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() SAL_OVERRIDE;
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;

private:
    tools::Polygon aTailPoly;  // the whole tail polygon
    bool                        mbSpecialTextBoxShadow; // for calc special shadow, default FALSE
    bool                        mbFixedTail; // for calc note box fixed tail, default FALSE
    Point                       maFixedTailPos; // for calc note box fixed tail position.

private:
    SVX_DLLPRIVATE void ImpGetCaptParams(ImpCaptParams& rPara) const;
    SVX_DLLPRIVATE void ImpCalcTail1(const ImpCaptParams& rPara, tools::Polygon& rPoly, Rectangle& rRect) const;
    SVX_DLLPRIVATE void ImpCalcTail2(const ImpCaptParams& rPara, tools::Polygon& rPoly, Rectangle& rRect) const;
    SVX_DLLPRIVATE void ImpCalcTail3(const ImpCaptParams& rPara, tools::Polygon& rPoly, Rectangle& rRect) const;
    SVX_DLLPRIVATE void ImpCalcTail4(const ImpCaptParams& rPara, tools::Polygon& rPoly, Rectangle& rRect) const;
    SVX_DLLPRIVATE void ImpCalcTail (const ImpCaptParams& rPara, tools::Polygon& rPoly, Rectangle& rRect) const;
    SVX_DLLPRIVATE void ImpRecalcTail();

public:
    TYPEINFO_OVERRIDE();
    SdrCaptionObj();
    SdrCaptionObj(const Rectangle& rRect, const Point& rTail);
    virtual ~SdrCaptionObj();

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const SAL_OVERRIDE;
    virtual sal_uInt16 GetObjIdentifier() const SAL_OVERRIDE;
    virtual SdrCaptionObj* Clone() const SAL_OVERRIDE;

    // for calc: special shadow only for text box
    void SetSpecialTextBoxShadow() { mbSpecialTextBoxShadow = true; }
    bool GetSpecialTextBoxShadow() const { return mbSpecialTextBoxShadow; }

    // for calc: fixed note tail position.
    void SetFixedTail() { mbFixedTail = true; }

    virtual OUString TakeObjNameSingul() const SAL_OVERRIDE;
    virtual OUString TakeObjNamePlural() const SAL_OVERRIDE;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const SAL_OVERRIDE;
    virtual void SetModel(SdrModel* pNewModel) SAL_OVERRIDE;
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) SAL_OVERRIDE;

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

    virtual void NbcMove(const Size& rSiz) SAL_OVERRIDE;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;

    virtual void NbcSetRelativePos(const Point& rPnt) SAL_OVERRIDE;
    virtual Point GetRelativePos() const SAL_OVERRIDE;
    virtual void NbcSetAnchorPos(const Point& rPnt) SAL_OVERRIDE;
    virtual const Point& GetAnchorPos() const SAL_OVERRIDE;

    virtual void RecalcSnapRect() SAL_OVERRIDE;
    virtual const Rectangle& GetSnapRect() const SAL_OVERRIDE;
    virtual void NbcSetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual const Rectangle& GetLogicRect() const SAL_OVERRIDE;
    virtual void NbcSetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;

    virtual sal_uInt32 GetSnapPointCount() const SAL_OVERRIDE;
    virtual Point GetSnapPoint(sal_uInt32 i) const SAL_OVERRIDE;

protected:
    virtual SdrObjGeoData* NewGeoData() const SAL_OVERRIDE;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const SAL_OVERRIDE;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) SAL_OVERRIDE;

public:
    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const SAL_OVERRIDE;

    const Point& GetTailPos() const;
    void SetTailPos(const Point& rPos);
    void NbcSetTailPos(const Point& rPos);

    // #i32599#
    // Add own implementation for TRSetBaseGeometry to handle TailPos over changes
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon) SAL_OVERRIDE;

    inline const Point& GetFixedTailPos() const  {return maFixedTailPos;}

    // geometry access
    ::basegfx::B2DPolygon getTailPolygon() const;
};



#endif //_SVDOCAPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
