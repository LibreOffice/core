/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    long                        nDrehWink;
    long                        nShearWink;

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

    virtual void TakeObjNameSingul(String& rName) const;
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

    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier) const;
};

#endif //_SVDOGRP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
