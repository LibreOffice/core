/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVDOGRP_HXX
#define _SVDOGRP_HXX

#include <tools/datetime.hxx>
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
    FASTBOOL                    bRefPoint; // Ist ein RefPoint gesetzt?

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
    virtual FASTBOOL HasRefPoint() const;
    virtual Point GetRefPoint() const;
    virtual void SetRefPoint(const Point& rPnt);
    virtual SdrObjList* GetSubList() const;

    virtual const Rectangle& GetCurrentBoundRect() const;
    virtual const Rectangle& GetSnapRect() const;

    virtual void operator=(const SdrObject& rObj);

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual void RecalcSnapRect();
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;

    // special drag methods
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;

    virtual FASTBOOL BegCreate(SdrDragStat& rStat);

    virtual long GetRotateAngle() const;
    virtual long GetShearAngle(FASTBOOL bVertical=sal_False) const;

    virtual void Move(const Size& rSiz);
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void Rotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void Mirror(const Point& rRef1, const Point& rRef2);
    virtual void Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear);
    virtual void SetAnchorPos(const Point& rPnt);
    virtual void SetRelativePos(const Point& rPnt);
    virtual void SetSnapRect(const Rectangle& rRect);
    virtual void SetLogicRect(const Rectangle& rRect);

    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear);
    virtual void NbcSetAnchorPos(const Point& rPnt);
    virtual void NbcSetRelativePos(const Point& rPnt);
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);

    virtual void NbcReformatText();
    virtual void ReformatText();

    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;
};

#endif //_SVDOGRP_HXX

