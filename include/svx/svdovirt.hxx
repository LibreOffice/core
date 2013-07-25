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

#ifndef _SVDOVIRT_HXX
#define _SVDOVIRT_HXX

#include <svx/svdobj.hxx>
#include "svx/svxdllapi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   SdrVirtObj
//
// Achtung! Das virtuelle Objekt ist noch nicht bis in alle Feinheiten
// durchprogrammiert und getestet. Z.Zt. kommt es nur in abgeleiteter
// beim Writer zum Einsatz.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrVirtObj : public SdrObject
{
public:
    virtual sdr::properties::BaseProperties& GetProperties() const;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    SdrObject&                  rRefObj; // Referenziertes Zeichenobjekt
    Rectangle                   aSnapRect;

protected:
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);

public:
    TYPEINFO();
    SdrVirtObj(SdrObject& rNewObj);
    virtual ~SdrVirtObj();
    virtual SdrObject& ReferencedObj();
    virtual const SdrObject& GetReferencedObj() const;
    virtual void NbcSetAnchorPos(const Point& rAnchorPos);
    virtual void SetModel(SdrModel* pNewModel);

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt32 GetObjInventor() const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual SdrObjList* GetSubList() const;

    virtual const Rectangle& GetCurrentBoundRect() const;
    virtual const Rectangle& GetLastBoundRect() const;
    virtual void RecalcBoundRect();
    virtual void SetChanged();
    virtual SdrVirtObj* Clone() const;
    SdrVirtObj& operator=(const SdrVirtObj& rObj);

    virtual OUString TakeObjNameSingul() const;
    virtual OUString TakeObjNamePlural() const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual sal_uInt32 GetHdlCount() const;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const;
    virtual sal_uInt32 GetPlusHdlCount(const SdrHdl& rHdl) const;
    virtual SdrHdl* GetPlusHdl(const SdrHdl& rHdl, sal_uInt32 nPlNum) const;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const;

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;
    virtual basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const;

    // FullDrag support
    virtual bool supportsFullDrag() const;
    virtual SdrObject* getFullDragClone() const;

    virtual bool BegCreate(SdrDragStat& rStat);
    virtual bool MovCreate(SdrDragStat& rStat);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;

    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear(const Point& rRef, long nWink, double tn, bool bVShear);

    virtual void Move(const Size& rSiz);
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true);
    virtual void Rotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void Mirror(const Point& rRef1, const Point& rRef2);
    virtual void Shear(const Point& rRef, long nWink, double tn, bool bVShear);

    virtual void RecalcSnapRect();
    virtual const Rectangle& GetSnapRect() const;
    virtual void SetSnapRect(const Rectangle& rRect);
    virtual void NbcSetSnapRect(const Rectangle& rRect);

    virtual const Rectangle& GetLogicRect() const;
    virtual void SetLogicRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);

    virtual long GetRotateAngle() const;
    virtual long GetShearAngle(bool bVertical = false) const;

    virtual sal_uInt32 GetSnapPointCount() const;
    virtual Point GetSnapPoint(sal_uInt32 i) const;

    virtual sal_Bool IsPolyObj() const;
    virtual sal_uInt32 GetPointCount() const;
    virtual Point GetPoint(sal_uInt32 i) const;
    virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i);

    virtual SdrObjGeoData* GetGeoData() const;
    virtual void SetGeoData(const SdrObjGeoData& rGeo);

    virtual void NbcReformatText();
    virtual void ReformatText();

    virtual bool HasMacro() const;
    virtual SdrObject* CheckMacroHit (const SdrObjMacroHitRec& rRec) const;
    virtual Pointer GetMacroPointer (const SdrObjMacroHitRec& rRec) const;
    virtual void PaintMacro (OutputDevice& rOut, const Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec) const;
    virtual bool DoMacro (const SdrObjMacroHitRec& rRec);
    virtual OUString GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const;

    // OD 30.06.2003 #108784# - virtual <GetOffset()> returns Point(0,0)
    // #i73248# for default SdrVirtObj, offset is aAnchor, not (0,0)
    virtual const Point GetOffset() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOVIRT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
