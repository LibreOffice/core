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

#ifndef _SVDOGRP_HXX
#define _SVDOGRP_HXX

#include <svx/svdobj.hxx>
#include "svx/svxdllapi.h"

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrObjList;
class SdrObjListIter;
class SfxItemSet;

//************************************************************
//   SdrObjGroup
//************************************************************

class SVX_DLLPUBLIC SdrObjGroup : public SdrObject
{
private:
protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    SdrObjList*                 pSub;    // Subliste (Kinder)
    Point                       aRefPoint; // Referenzpunkt innerhalb der Objektgruppe
    bool                        bRefPoint; // Ist ein RefPoint gesetzt?

public:
    TYPEINFO();
    SdrObjGroup();
    virtual ~SdrObjGroup();

    virtual void SetBoundRectDirty();
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual SdrLayerID GetLayer() const;
    virtual void NbcSetLayer(SdrLayerID nLayer);
    virtual void SetObjList(SdrObjList* pNewObjList);
    virtual void SetPage(SdrPage* pNewPage);
    virtual void SetModel(SdrModel* pNewModel);
    virtual bool HasRefPoint() const;
    virtual Point GetRefPoint() const;
    virtual void SetRefPoint(const Point& rPnt);
    virtual SdrObjList* GetSubList() const;

    virtual const Rectangle& GetCurrentBoundRect() const;
    virtual const Rectangle& GetSnapRect() const;

    virtual SdrObjGroup* Clone() const;
    SdrObjGroup& operator=(const SdrObjGroup& rObj);

    virtual OUString TakeObjNameSingul() const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual void RecalcSnapRect();
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;

    // special drag methods
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;

    virtual bool BegCreate(SdrDragStat& rStat);

    virtual long GetRotateAngle() const;
    virtual long GetShearAngle(bool bVertical = false) const;

    virtual void Move(const Size& rSiz);
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true);
    virtual void Rotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void Mirror(const Point& rRef1, const Point& rRef2);
    virtual void Shear(const Point& rRef, long nWink, double tn, bool bVShear);
    virtual void SetAnchorPos(const Point& rPnt);
    virtual void SetRelativePos(const Point& rPnt);
    virtual void SetSnapRect(const Rectangle& rRect);
    virtual void SetLogicRect(const Rectangle& rRect);

    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear(const Point& rRef, long nWink, double tn, bool bVShear);
    virtual void NbcSetAnchorPos(const Point& rPnt);
    virtual void NbcSetRelativePos(const Point& rPnt);
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);

    virtual void NbcReformatText();
    virtual void ReformatText();

    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;
};

#endif //_SVDOGRP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
