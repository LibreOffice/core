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
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() override;
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() override;

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
    SdrCaptionObj();
    SdrCaptionObj(const Rectangle& rRect, const Point& rTail);
    virtual ~SdrCaptionObj();

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual SdrCaptionObj* Clone() const override;

    // for calc: special shadow only for text box
    void SetSpecialTextBoxShadow() { mbSpecialTextBoxShadow = true; }
    bool GetSpecialTextBoxShadow() const { return mbSpecialTextBoxShadow; }

    // for calc: fixed note tail position.
    void SetFixedTail() { mbFixedTail = true; }

    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;
    virtual void SetModel(SdrModel* pNewModel) override;
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    virtual sal_uInt32 GetHdlCount() const override;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const override;

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
    virtual Pointer GetCreatePointer() const override;

    virtual void NbcMove(const Size& rSiz) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;

    virtual void NbcSetRelativePos(const Point& rPnt) override;
    virtual Point GetRelativePos() const override;
    virtual void NbcSetAnchorPos(const Point& rPnt) override;
    virtual const Point& GetAnchorPos() const override;

    virtual void RecalcSnapRect() override;
    virtual const Rectangle& GetSnapRect() const override;
    virtual void NbcSetSnapRect(const Rectangle& rRect) override;
    virtual const Rectangle& GetLogicRect() const override;
    virtual void NbcSetLogicRect(const Rectangle& rRect) override;

    virtual sal_uInt32 GetSnapPointCount() const override;
    virtual Point GetSnapPoint(sal_uInt32 i) const override;

protected:
    virtual SdrObjGeoData* NewGeoData() const override;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) override;

public:
    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    const Point& GetTailPos() const;
    void SetTailPos(const Point& rPos);
    void NbcSetTailPos(const Point& rPos);

    // #i32599#
    // Add own implementation for TRSetBaseGeometry to handle TailPos over changes
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon) override;

    inline const Point& GetFixedTailPos() const  {return maFixedTailPos;}

    // geometry access
    ::basegfx::B2DPolygon getTailPolygon() const;
};



#endif //_SVDOCAPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
