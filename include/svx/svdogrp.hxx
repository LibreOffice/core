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

#ifndef INCLUDED_SVX_SVDOGRP_HXX
#define INCLUDED_SVX_SVDOGRP_HXX

#include <memory>
#include <svx/svdobj.hxx>
#include <svx/svxdllapi.h>
#include <svx/svdpage.hxx>

// Forward declarations
class SfxItemSet;

//   SdrObjGroup
class SVX_DLLPUBLIC SdrObjGroup final : public SdrObject, public SdrObjList
{
private:
    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;
    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;

    Point                       aRefPoint;      // Reference point inside the object group

private:
    // protected destructor - due to final, make private
    virtual ~SdrObjGroup() override;

public:
    SdrObjGroup(SdrModel& rSdrModel);

    // derived from SdrObjList
    virtual SdrPage* getSdrPageFromSdrObjList() const override;
    virtual SdrObject* getSdrObjectFromSdrObjList() const override;

    // derived from SdrObject
    virtual SdrObjList* getChildrenOfSdrObject() const override;

    virtual void SetBoundRectDirty() override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual SdrLayerID GetLayer() const override;
    virtual void NbcSetLayer(SdrLayerID nLayer) override;

    // react on model/page change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage) override;

    virtual SdrObjList* GetSubList() const override;

    virtual const tools::Rectangle& GetCurrentBoundRect() const override;
    virtual const tools::Rectangle& GetSnapRect() const override;

    virtual SdrObjGroup* CloneSdrObject(SdrModel& rTargetModel) const override;
    SdrObjGroup& operator=(const SdrObjGroup& rObj);

    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    virtual void RecalcSnapRect() override;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;

    // special drag methods
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const override;

    virtual bool BegCreate(SdrDragStat& rStat) override;

    virtual long GetRotateAngle() const override;
    virtual long GetShearAngle(bool bVertical = false) const override;

    virtual void Move(const Size& rSiz) override;
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true) override;
    virtual void Rotate(const Point& rRef, long nAngle, double sn, double cs) override;
    virtual void Mirror(const Point& rRef1, const Point& rRef2) override;
    virtual void Shear(const Point& rRef, long nAngle, double tn, bool bVShear) override;
    virtual void SetAnchorPos(const Point& rPnt) override;
    virtual void SetRelativePos(const Point& rPnt) override;
    virtual void SetSnapRect(const tools::Rectangle& rRect) override;
    virtual void SetLogicRect(const tools::Rectangle& rRect) override;

    virtual void NbcMove(const Size& rSiz) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcRotate(const Point& rRef, long nAngle, double sn, double cs) override;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
    virtual void NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear) override;
    virtual void NbcSetAnchorPos(const Point& rPnt) override;
    virtual void NbcSetRelativePos(const Point& rPnt) override;
    virtual void NbcSetSnapRect(const tools::Rectangle& rRect) override;
    virtual void NbcSetLogicRect(const tools::Rectangle& rRect) override;

    virtual void NbcReformatText() override;

    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

#endif // INCLUDED_SVX_SVDOGRP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
