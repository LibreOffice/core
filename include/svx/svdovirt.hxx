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

#ifndef INCLUDED_SVX_SVDOVIRT_HXX
#define INCLUDED_SVX_SVDOVIRT_HXX

#include <svx/svdobj.hxx>
#include <svx/svxdllapi.h>

/**
 * FIXME: The virtual object is not yet fully implemented and tested.
 * At the moment we only use it in a derived class in Writer.
 */
class SVX_DLLPUBLIC SdrVirtObj : public SdrObject
{
    SdrVirtObj( const SdrVirtObj& ) = delete;
public:
    virtual sdr::properties::BaseProperties& GetProperties() const override;

protected:
    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;

    SdrObject&                  rRefObj; // Referenced drawing object
    tools::Rectangle            aSnapRect;

protected:
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    virtual SdrObjGeoData* NewGeoData() const override;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) override;

    // protected destructor
    virtual ~SdrVirtObj() override;

public:
    SdrVirtObj(
        SdrModel& rSdrModel,
        SdrObject& rNewObj);

    SdrObject& ReferencedObj();
    const SdrObject& GetReferencedObj() const;
    virtual void NbcSetAnchorPos(const Point& rAnchorPos) override;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual SdrInventor GetObjInventor() const override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual SdrObjList* GetSubList() const override;

    virtual const tools::Rectangle& GetCurrentBoundRect() const override;
    virtual const tools::Rectangle& GetLastBoundRect() const override;
    virtual void RecalcBoundRect() override;
    virtual SdrVirtObj* CloneSdrObject(SdrModel& rTargetModel) const override;
    SdrVirtObj& operator=(const SdrVirtObj& rObj);

    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    // RotGrfFlyFrame: If true, this SdrObject supports only limited rotation
    virtual bool HasLimitedRotation() const override;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;
    virtual sal_uInt32 GetHdlCount() const override;
    virtual void AddToPlusHdlList(SdrHdlList& rHdlList, SdrHdl& rHdl) const override;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const override;

    // special drag methods
    virtual bool hasSpecialDrag() const override;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const override;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) override;
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const override;
    virtual basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const override;

    // FullDrag support
    virtual bool supportsFullDrag() const override;
    virtual SdrObject* getFullDragClone() const override;

    virtual bool BegCreate(SdrDragStat& rStat) override;
    virtual bool MovCreate(SdrDragStat& rStat) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;
    virtual bool BckCreate(SdrDragStat& rStat) override;
    virtual void BrkCreate(SdrDragStat& rStat) override;
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const override;

    virtual void NbcMove(const Size& rSiz) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcRotate(const Point& rRef, long nAngle, double sn, double cs) override;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
    virtual void NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear) override;

    virtual void Move(const Size& rSiz) override;
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true) override;
    virtual void Rotate(const Point& rRef, long nAngle, double sn, double cs) override;
    virtual void Mirror(const Point& rRef1, const Point& rRef2) override;
    virtual void Shear(const Point& rRef, long nAngle, double tn, bool bVShear) override;

    virtual void RecalcSnapRect() override;
    virtual const tools::Rectangle& GetSnapRect() const override;
    virtual void SetSnapRect(const tools::Rectangle& rRect) override;
    virtual void NbcSetSnapRect(const tools::Rectangle& rRect) override;

    virtual const tools::Rectangle& GetLogicRect() const override;
    virtual void SetLogicRect(const tools::Rectangle& rRect) override;
    virtual void NbcSetLogicRect(const tools::Rectangle& rRect) override;

    virtual long GetRotateAngle() const override;
    virtual long GetShearAngle(bool bVertical = false) const override;

    virtual sal_uInt32 GetSnapPointCount() const override;
    virtual Point GetSnapPoint(sal_uInt32 i) const override;

    virtual bool IsPolyObj() const override;
    virtual sal_uInt32 GetPointCount() const override;
    virtual Point GetPoint(sal_uInt32 i) const override;
    virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i) override;

    virtual SdrObjGeoData* GetGeoData() const override;
    virtual void SetGeoData(const SdrObjGeoData& rGeo) override;

    virtual void NbcReformatText() override;

    virtual bool HasMacro() const override;
    virtual SdrObject* CheckMacroHit (const SdrObjMacroHitRec& rRec) const override;
    virtual PointerStyle GetMacroPointer (const SdrObjMacroHitRec& rRec) const override;
    virtual void PaintMacro (OutputDevice& rOut, const tools::Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec) const override;
    virtual bool DoMacro (const SdrObjMacroHitRec& rRec) override;

    // #i73248# for default SdrVirtObj, offset is aAnchor, not (0,0)
    virtual const Point GetOffset() const;
};

#endif // INCLUDED_SVX_SVDOVIRT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
