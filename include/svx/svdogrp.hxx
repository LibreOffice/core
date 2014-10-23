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

#include <svx/svdobj.hxx>
#include <svx/svxdllapi.h>


//   Vorausdeklarationen


class SdrObjList;
class SdrObjListIter;
class SfxItemSet;


//   SdrObjGroup


class SVX_DLLPUBLIC SdrObjGroup : public SdrObject
{
private:
protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() SAL_OVERRIDE;

    SdrObjList*                 pSub;    // Subliste (Kinder)
    Point                       aRefPoint; // Referenzpunkt innerhalb der Objektgruppe
    bool                        bRefPoint; // Ist ein RefPoint gesetzt?

public:
    TYPEINFO_OVERRIDE();
    SdrObjGroup();
    virtual ~SdrObjGroup();

    virtual void SetBoundRectDirty() SAL_OVERRIDE;
    virtual sal_uInt16 GetObjIdentifier() const SAL_OVERRIDE;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const SAL_OVERRIDE;
    virtual SdrLayerID GetLayer() const SAL_OVERRIDE;
    virtual void NbcSetLayer(SdrLayerID nLayer) SAL_OVERRIDE;
    virtual void SetObjList(SdrObjList* pNewObjList) SAL_OVERRIDE;
    virtual void SetPage(SdrPage* pNewPage) SAL_OVERRIDE;
    virtual void SetModel(SdrModel* pNewModel) SAL_OVERRIDE;
    virtual bool HasRefPoint() const SAL_OVERRIDE;
    virtual Point GetRefPoint() const SAL_OVERRIDE;
    virtual void SetRefPoint(const Point& rPnt) SAL_OVERRIDE;
    virtual SdrObjList* GetSubList() const SAL_OVERRIDE;

    virtual const Rectangle& GetCurrentBoundRect() const SAL_OVERRIDE;
    virtual const Rectangle& GetSnapRect() const SAL_OVERRIDE;

    virtual SdrObjGroup* Clone() const SAL_OVERRIDE;
    SdrObjGroup& operator=(const SdrObjGroup& rObj);

    virtual OUString TakeObjNameSingul() const SAL_OVERRIDE;
    virtual OUString TakeObjNamePlural() const SAL_OVERRIDE;

    virtual void RecalcSnapRect() SAL_OVERRIDE;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const SAL_OVERRIDE;

    // special drag methods
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const SAL_OVERRIDE;

    virtual bool BegCreate(SdrDragStat& rStat) SAL_OVERRIDE;

    virtual long GetRotateAngle() const SAL_OVERRIDE;
    virtual long GetShearAngle(bool bVertical = false) const SAL_OVERRIDE;

    virtual void Move(const Size& rSiz) SAL_OVERRIDE;
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true) SAL_OVERRIDE;
    virtual void Rotate(const Point& rRef, long nWink, double sn, double cs) SAL_OVERRIDE;
    virtual void Mirror(const Point& rRef1, const Point& rRef2) SAL_OVERRIDE;
    virtual void Shear(const Point& rRef, long nWink, double tn, bool bVShear) SAL_OVERRIDE;
    virtual void SetAnchorPos(const Point& rPnt) SAL_OVERRIDE;
    virtual void SetRelativePos(const Point& rPnt) SAL_OVERRIDE;
    virtual void SetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual void SetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;

    virtual void NbcMove(const Size& rSiz) SAL_OVERRIDE;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs) SAL_OVERRIDE;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) SAL_OVERRIDE;
    virtual void NbcShear(const Point& rRef, long nWink, double tn, bool bVShear) SAL_OVERRIDE;
    virtual void NbcSetAnchorPos(const Point& rPnt) SAL_OVERRIDE;
    virtual void NbcSetRelativePos(const Point& rPnt) SAL_OVERRIDE;
    virtual void NbcSetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual void NbcSetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;

    virtual void NbcReformatText() SAL_OVERRIDE;
    virtual void ReformatText() SAL_OVERRIDE;

    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const SAL_OVERRIDE;
};

#endif // INCLUDED_SVX_SVDOGRP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
