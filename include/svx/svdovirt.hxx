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



//   SdrVirtObj

// Achtung! Das virtuelle Objekt ist noch nicht bis in alle Feinheiten
// durchprogrammiert und getestet. Z.Zt. kommt es nur in abgeleiteter
// beim Writer zum Einsatz.



class SVX_DLLPUBLIC SdrVirtObj : public SdrObject
{
    SdrVirtObj( const SdrVirtObj& ) = delete;
public:
    virtual sdr::properties::BaseProperties& GetProperties() const SAL_OVERRIDE;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;

    SdrObject&                  rRefObj; // Referenziertes Zeichenobjekt
    Rectangle                   aSnapRect;

protected:
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) SAL_OVERRIDE;

    virtual SdrObjGeoData* NewGeoData() const SAL_OVERRIDE;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const SAL_OVERRIDE;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();
    SdrVirtObj(SdrObject& rNewObj);
    virtual ~SdrVirtObj();
    SdrObject& ReferencedObj();
    const SdrObject& GetReferencedObj() const;
    virtual void NbcSetAnchorPos(const Point& rAnchorPos) SAL_OVERRIDE;
    virtual void SetModel(SdrModel* pNewModel) SAL_OVERRIDE;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const SAL_OVERRIDE;
    virtual sal_uInt32 GetObjInventor() const SAL_OVERRIDE;
    virtual sal_uInt16 GetObjIdentifier() const SAL_OVERRIDE;
    virtual SdrObjList* GetSubList() const SAL_OVERRIDE;

    virtual const Rectangle& GetCurrentBoundRect() const SAL_OVERRIDE;
    virtual const Rectangle& GetLastBoundRect() const SAL_OVERRIDE;
    virtual void RecalcBoundRect() SAL_OVERRIDE;
    virtual void SetChanged() SAL_OVERRIDE;
    virtual SdrVirtObj* Clone() const SAL_OVERRIDE;
    SdrVirtObj& operator=(const SdrVirtObj& rObj);

    virtual OUString TakeObjNameSingul() const SAL_OVERRIDE;
    virtual OUString TakeObjNamePlural() const SAL_OVERRIDE;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const SAL_OVERRIDE;
    virtual sal_uInt32 GetHdlCount() const SAL_OVERRIDE;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const SAL_OVERRIDE;
    virtual sal_uInt32 GetPlusHdlCount(const SdrHdl& rHdl) const SAL_OVERRIDE;
    virtual SdrHdl* GetPlusHdl(const SdrHdl& rHdl, sal_uInt32 nPlNum) const SAL_OVERRIDE;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const SAL_OVERRIDE;

    // special drag methods
    virtual bool hasSpecialDrag() const SAL_OVERRIDE;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const SAL_OVERRIDE;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) SAL_OVERRIDE;
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const SAL_OVERRIDE;
    virtual basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const SAL_OVERRIDE;

    // FullDrag support
    virtual bool supportsFullDrag() const SAL_OVERRIDE;
    virtual SdrObject* getFullDragClone() const SAL_OVERRIDE;

    virtual bool BegCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual bool MovCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) SAL_OVERRIDE;
    virtual bool BckCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual void BrkCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const SAL_OVERRIDE;

    virtual void NbcMove(const Size& rSiz) SAL_OVERRIDE;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;
    virtual void NbcRotate(const Point& rRef, long nAngle, double sn, double cs) SAL_OVERRIDE;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) SAL_OVERRIDE;
    virtual void NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear) SAL_OVERRIDE;

    virtual void Move(const Size& rSiz) SAL_OVERRIDE;
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true) SAL_OVERRIDE;
    virtual void Rotate(const Point& rRef, long nAngle, double sn, double cs) SAL_OVERRIDE;
    virtual void Mirror(const Point& rRef1, const Point& rRef2) SAL_OVERRIDE;
    virtual void Shear(const Point& rRef, long nAngle, double tn, bool bVShear) SAL_OVERRIDE;

    virtual void RecalcSnapRect() SAL_OVERRIDE;
    virtual const Rectangle& GetSnapRect() const SAL_OVERRIDE;
    virtual void SetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual void NbcSetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;

    virtual const Rectangle& GetLogicRect() const SAL_OVERRIDE;
    virtual void SetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual void NbcSetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;

    virtual long GetRotateAngle() const SAL_OVERRIDE;
    virtual long GetShearAngle(bool bVertical = false) const SAL_OVERRIDE;

    virtual sal_uInt32 GetSnapPointCount() const SAL_OVERRIDE;
    virtual Point GetSnapPoint(sal_uInt32 i) const SAL_OVERRIDE;

    virtual bool IsPolyObj() const SAL_OVERRIDE;
    virtual sal_uInt32 GetPointCount() const SAL_OVERRIDE;
    virtual Point GetPoint(sal_uInt32 i) const SAL_OVERRIDE;
    virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i) SAL_OVERRIDE;

    virtual SdrObjGeoData* GetGeoData() const SAL_OVERRIDE;
    virtual void SetGeoData(const SdrObjGeoData& rGeo) SAL_OVERRIDE;

    virtual void NbcReformatText() SAL_OVERRIDE;
    virtual void ReformatText() SAL_OVERRIDE;

    virtual bool HasMacro() const SAL_OVERRIDE;
    virtual SdrObject* CheckMacroHit (const SdrObjMacroHitRec& rRec) const SAL_OVERRIDE;
    virtual Pointer GetMacroPointer (const SdrObjMacroHitRec& rRec) const SAL_OVERRIDE;
    virtual void PaintMacro (OutputDevice& rOut, const Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec) const SAL_OVERRIDE;
    virtual bool DoMacro (const SdrObjMacroHitRec& rRec) SAL_OVERRIDE;
    virtual OUString GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const SAL_OVERRIDE;

    // OD 30.06.2003 #108784# - virtual <GetOffset()> returns Point(0,0)
    // #i73248# for default SdrVirtObj, offset is aAnchor, not (0,0)
    virtual const Point GetOffset() const;
};



#endif // INCLUDED_SVX_SVDOVIRT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
