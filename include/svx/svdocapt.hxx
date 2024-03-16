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

#pragma once

#include <svx/svdorect.hxx>
#include <svx/svxdllapi.h>


//   Forward Declarations
class ImpCaptParams;

namespace sdr::properties {
    class CaptionProperties;
}

//   Helper Class SdrCaptObjGeoData
class SdrCaptObjGeoData final : public SdrTextObjGeoData
{
public:
    tools::Polygon aTailPoly;
};

//   SdrCaptionObj
class SVXCORE_DLLPUBLIC SdrCaptionObj final : public SdrRectObj
{
private:
    // to allow sdr::properties::CaptionProperties access to ImpRecalcTail()
    friend class sdr::properties::CaptionProperties;
    friend class                SdrTextObj; // for ImpRecalcTail() during AutoGrow

    // tdf#118662 exclusive friend function and setter for SuppressGetBitmap
    friend void setSuppressGetBitmapFromXclObjComment(SdrCaptionObj* pSdrCaptionObj, bool bValue);
    void setSuppressGetBitmap(bool bNew)
    {
        mbSuppressGetBitmap = bNew;
    }

    SAL_DLLPRIVATE virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;
    SAL_DLLPRIVATE virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;

    tools::Polygon  aTailPoly;              // the whole tail polygon
    bool            mbSpecialTextBoxShadow; // for calc special shadow, default FALSE
    bool            mbFixedTail;            // for calc note box fixed tail, default FALSE
    bool            mbSuppressGetBitmap;    // tdf#118662
    Point           maFixedTailPos;         // for calc note box fixed tail position.

    SVX_DLLPRIVATE void ImpGetCaptParams(ImpCaptParams& rPara) const;
    SVX_DLLPRIVATE static void ImpCalcTail1(const ImpCaptParams& rPara, tools::Polygon& rPoly, tools::Rectangle const & rRect);
    SVX_DLLPRIVATE static void ImpCalcTail2(const ImpCaptParams& rPara, tools::Polygon& rPoly, tools::Rectangle const & rRect);
    SVX_DLLPRIVATE static void ImpCalcTail3(const ImpCaptParams& rPara, tools::Polygon& rPoly, tools::Rectangle const & rRect);
    SVX_DLLPRIVATE static void ImpCalcTail (const ImpCaptParams& rPara, tools::Polygon& rPoly, tools::Rectangle const & rRect);
    SVX_DLLPRIVATE void ImpRecalcTail();

    // protected destructor
    SAL_DLLPRIVATE virtual ~SdrCaptionObj() override;

public:
    SAL_DLLPRIVATE SdrCaptionObj(SdrModel& rSdrModel);
    // Copy constructor
    SAL_DLLPRIVATE SdrCaptionObj(SdrModel& rSdrModel, SdrCaptionObj const & rSource);
    SdrCaptionObj(
        SdrModel& rSdrModel,
        const tools::Rectangle& rRect,
        const Point& rTail);

    // tdf#118662 getter for SuppressGetBitmap
    bool isSuppressGetBitmap() const { return mbSuppressGetBitmap; }

    SAL_DLLPRIVATE virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    SAL_DLLPRIVATE virtual SdrObjKind GetObjIdentifier() const override;
    SAL_DLLPRIVATE virtual rtl::Reference<SdrObject> CloneSdrObject(SdrModel& rTargetModel) const override;

    // for calc: special shadow only for text box
    void SetSpecialTextBoxShadow() { mbSpecialTextBoxShadow = true; }
    bool GetSpecialTextBoxShadow() const { return mbSpecialTextBoxShadow; }

    // for calc: fixed note tail position.
    void SetFixedTail() { mbFixedTail = true; }

    SAL_DLLPRIVATE virtual OUString TakeObjNameSingul() const override;
    SAL_DLLPRIVATE virtual OUString TakeObjNamePlural() const override;

    SAL_DLLPRIVATE virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;
    SAL_DLLPRIVATE virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    SAL_DLLPRIVATE virtual sal_uInt32 GetHdlCount() const override;
    SAL_DLLPRIVATE virtual void AddToHdlList(SdrHdlList& rHdlList) const override;

    // special drag methods
    SAL_DLLPRIVATE virtual bool hasSpecialDrag() const override;
    SAL_DLLPRIVATE virtual bool beginSpecialDrag(SdrDragStat& rDrag) const override;
    SAL_DLLPRIVATE virtual bool applySpecialDrag(SdrDragStat& rDrag) override;
    SAL_DLLPRIVATE virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const override;

    SAL_DLLPRIVATE virtual bool BegCreate(SdrDragStat& rStat) override;
    SAL_DLLPRIVATE virtual bool MovCreate(SdrDragStat& rStat) override;
    SAL_DLLPRIVATE virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;
    SAL_DLLPRIVATE virtual bool BckCreate(SdrDragStat& rStat) override;
    SAL_DLLPRIVATE virtual void BrkCreate(SdrDragStat& rStat) override;
    SAL_DLLPRIVATE virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const override;
    SAL_DLLPRIVATE virtual PointerStyle GetCreatePointer() const override;

    SAL_DLLPRIVATE virtual void NbcMove(const Size& rSiz) override;
    SAL_DLLPRIVATE virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;

    SAL_DLLPRIVATE virtual void NbcSetRelativePos(const Point& rPnt) override;
    SAL_DLLPRIVATE virtual Point GetRelativePos() const override;

    virtual const tools::Rectangle& GetLogicRect() const override;
    SAL_DLLPRIVATE virtual void NbcSetLogicRect(const tools::Rectangle& rRect) override;

    SAL_DLLPRIVATE virtual sal_uInt32 GetSnapPointCount() const override;
    SAL_DLLPRIVATE virtual Point GetSnapPoint(sal_uInt32 i) const override;

private:
    SAL_DLLPRIVATE virtual std::unique_ptr<SdrObjGeoData> NewGeoData() const override;
    SAL_DLLPRIVATE virtual void SaveGeoData(SdrObjGeoData& rGeo) const override;
    SAL_DLLPRIVATE virtual void RestoreGeoData(const SdrObjGeoData& rGeo) override;

public:
    SAL_DLLPRIVATE virtual rtl::Reference<SdrObject> DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    const Point& GetTailPos() const;
    void SetTailPos(const Point& rPos);
    SAL_DLLPRIVATE void NbcSetTailPos(const Point& rPos);

    // #i32599#
    // Add own implementation for TRSetBaseGeometry to handle TailPos over changes
    SAL_DLLPRIVATE virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon) override;

    const Point& GetFixedTailPos() const  {return maFixedTailPos;}

    // geometry access
    ::basegfx::B2DPolygon getTailPolygon() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
