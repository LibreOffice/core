/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdogrp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:24:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVDOGRP_HXX
#define _SVDOGRP_HXX

#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
    // BaseProperties section
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    // #110094# DrawContact section
private:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

protected:
    SdrObjList*                 pSub;    // Subliste (Kinder)
    long                        nDrehWink;
    long                        nShearWink;

    Point                       aRefPoint; // Referenzpunkt innerhalb der Objektgruppe
    FASTBOOL                    bRefPoint; // Ist ein RefPoint gesetzt?

public:
    TYPEINFO();
    SdrObjGroup();
    virtual ~SdrObjGroup();

    virtual UINT16 GetObjIdentifier() const;
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

    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;
    virtual void operator=(const SdrObject& rObj);

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual void RecalcSnapRect();
    virtual basegfx::B2DPolyPolygon TakeXorPoly(sal_Bool bDetail) const;

    virtual FASTBOOL BegDrag(SdrDragStat& rDrag)  const;
    virtual FASTBOOL BegCreate(SdrDragStat& rStat);

    virtual long GetRotateAngle() const;
    virtual long GetShearAngle(FASTBOOL bVertical=FALSE) const;

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

    virtual SdrObject* DoConvertToPolyObj(BOOL bBezier) const;
};

#endif //_SVDOGRP_HXX

