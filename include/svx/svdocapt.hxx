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

    // tdf#118662 exclusive friend function and setter for SuppressGetBitmap
    friend void setSuppressGetBitmapFromXclObjComment(SdrCaptionObj* pSdrCaptionObj, bool bValue);
    void setSuppressGetBitmap(bool bNew)
    {
        mbSuppressGetBitmap = bNew;
    }

protected:
    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;
    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;

private:
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

protected:
    // protected destructor
    virtual ~SdrCaptionObj() override;

public:
    SdrCaptionObj(SdrModel& rSdrModel);
    SdrCaptionObj(
        SdrModel& rSdrModel,
        const tools::Rectangle& rRect,
        const Point& rTail);

    // tdf#118662 getter for SuppressGetBitmap
    bool isSuppressGetBitmap() const { return mbSuppressGetBitmap; }

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual SdrCaptionObj* CloneSdrObject(SdrModel& rTargetModel) const override;

    // implemented mainly for the purposes of Clone()
    SdrCaptionObj& operator=(const SdrCaptionObj& rObj);

    // for calc: special shadow only for text box
    void SetSpecialTextBoxShadow() { mbSpecialTextBoxShadow = true; }
    bool GetSpecialTextBoxShadow() const { return mbSpecialTextBoxShadow; }

    // for calc: fixed note tail position.
    void SetFixedTail() { mbFixedTail = true; }

    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    virtual sal_uInt32 GetHdlCount() const override;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const override;

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
    virtual PointerStyle GetCreatePointer() const override;

    virtual void NbcMove(const Size& rSiz) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;

    virtual void NbcSetRelativePos(const Point& rPnt) override;
    virtual Point GetRelativePos() const override;

    virtual const tools::Rectangle& GetLogicRect() const override;
    virtual void NbcSetLogicRect(const tools::Rectangle& rRect) override;

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

    const Point& GetFixedTailPos() const  {return maFixedTailPos;}

    // geometry access
    ::basegfx::B2DPolygon getTailPolygon() const;
};


#endif //_SVDOCAPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
