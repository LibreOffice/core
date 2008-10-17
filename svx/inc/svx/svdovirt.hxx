/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svdovirt.hxx,v $
 * $Revision: 1.4.18.1 $
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

private:
    // AW, OD 2004-05-03 #i27224#
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

protected:
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
    SdrVirtObj(SdrObject& rNewObj, const Point& rAnchorPos);
    virtual ~SdrVirtObj();
    virtual SdrObject& ReferencedObj();
    virtual const SdrObject& GetReferencedObj() const;
    virtual void NbcSetAnchorPos(const Point& rAnchorPos);
    virtual void SetModel(SdrModel* pNewModel);

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual UINT32 GetObjInventor() const;
    virtual UINT16 GetObjIdentifier() const;
    virtual SdrObjList* GetSubList() const;

    virtual const Rectangle& GetCurrentBoundRect() const;
    virtual const Rectangle& GetLastBoundRect() const;
    virtual void RecalcBoundRect();
    virtual void SetChanged();
    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;
    virtual SdrObject* Clone() const;
    virtual void operator=(const SdrObject& rObj);

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual sal_uInt32 GetHdlCount() const;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const;
    virtual sal_uInt32 GetPlusHdlCount(const SdrHdl& rHdl) const;
    virtual SdrHdl* GetPlusHdl(const SdrHdl& rHdl, sal_uInt32 nPlNum) const;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const;
    virtual FASTBOOL HasSpecialDrag() const;
    virtual FASTBOOL BegDrag(SdrDragStat& rDrag) const;
    virtual FASTBOOL MovDrag(SdrDragStat& rDrag) const;
    virtual FASTBOOL EndDrag(SdrDragStat& rDrag);
    virtual void BrkDrag(SdrDragStat& rDrag) const;

    virtual String GetDragComment(const SdrDragStat& rDrag, FASTBOOL bUndoDragComment, FASTBOOL bCreateComment) const;
    virtual basegfx::B2DPolyPolygon TakeDragPoly(const SdrDragStat& rDrag) const;

    virtual FASTBOOL BegCreate(SdrDragStat& rStat);
    virtual FASTBOOL MovCreate(SdrDragStat& rStat);
    virtual FASTBOOL EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual FASTBOOL BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;

    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear);

    virtual void Move(const Size& rSiz);
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void Rotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void Mirror(const Point& rRef1, const Point& rRef2);
    virtual void Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear);

    virtual void RecalcSnapRect();
    virtual const Rectangle& GetSnapRect() const;
    virtual void SetSnapRect(const Rectangle& rRect);
    virtual void NbcSetSnapRect(const Rectangle& rRect);

    virtual const Rectangle& GetLogicRect() const;
    virtual void SetLogicRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);

    virtual long GetRotateAngle() const;
    virtual long GetShearAngle(FASTBOOL bVertical=FALSE) const;

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

    virtual FASTBOOL HasMacro() const;
    virtual SdrObject* CheckMacroHit (const SdrObjMacroHitRec& rRec) const;
    virtual Pointer GetMacroPointer (const SdrObjMacroHitRec& rRec) const;
    virtual void PaintMacro (OutputDevice& rOut, const Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec) const;
    virtual FASTBOOL DoMacro (const SdrObjMacroHitRec& rRec);
    virtual XubString GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const;

    // OD 30.06.2003 #108784# - virtual <GetOffset()> returns Point(0,0)
    // #i73248# for default SdrVirtObj, offset is aAnchor, not (0,0)
    virtual const Point GetOffset() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOVIRT_HXX

