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

#ifndef _SVDORECT_HXX
#define _SVDORECT_HXX

#include <bf_svx/svdotext.hxx>
namespace binfilter {

//************************************************************
//   Vorausdeklarationen
//************************************************************

class XPolygon;

//************************************************************
//   Hilfsklasse SdrRectObjGeoData
//************************************************************

class SdrRectObjGeoData : public SdrTextObjGeoData
{
public:
    long						nEckRad; // Eckenradius -> Undo soll dann auch mal ueber Items ...
};

//************************************************************
//   SdrRectObj
//
// Rechteck-Objekte (Rechteck,Kreis,...)
//
//************************************************************

class SdrRectObj : public SdrTextObj
{
    friend class				SdrTextObj; // wg SetXPolyDirty bei GrowAdjust

protected:
    XPolygon*					pXPoly;
    FASTBOOL					bXPolyIsLine;  // TRUE: pXPoly muss als PolyLine angesehen werden.

protected:
    // Liefert TRUE, wenn das Painten ein XPolygon erfordert.
    FASTBOOL PaintNeedsXPoly(long nEckRad) const;

protected:
    XPolygon ImpCalcXPoly(const Rectangle& rRect1, long nRad1, FASTBOOL bContour=FALSE) const;
    SdrObject* ImpCheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer, FASTBOOL bForceFilled, FASTBOOL bForceTol=FALSE) const;

    //void PaintIt(ExtOutputDevice& rOut, _BOOL bDown) const;
    void SetXPolyDirty();

    // RecalcXPoly sollte ueberladen werden. Dabei muss dann eine XPolygon
    // Instanz generiert (new) und an pXPoly zugewiesen werden.
    virtual void RecalcXPoly();
    const XPolygon& GetXPoly() const;
    virtual SdrObjGeoData* NewGeoData() const;
    virtual void           SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void           RestGeoData(const SdrObjGeoData& rGeo);

public:
    TYPEINFO();
    // Der Eckenradius-Parameter fliegt irgendwann raus. Der Eckenradius
    // ist dann (spaeter) ueber SfxItems einzustellen (SetAttributes()).
    // Konstruktion eines Rechteck-Zeichenobjekts

    SdrRectObj();
    SdrRectObj(const Rectangle& rRect);

    // Konstruktion eines Textrahmens
    SdrRectObj(SdrObjKind eNewTextKind);
    SdrRectObj(SdrObjKind eNewTextKind, const Rectangle& rRect);
    virtual ~SdrRectObj();

    virtual UINT16 GetObjIdentifier() const;
    virtual void RecalcBoundRect();
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;
    virtual FASTBOOL Paint(ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const;
    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;


    virtual void operator=(const SdrObject& rObj);
    virtual void RecalcSnapRect();
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);
    virtual void TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL bDetail) const;
    virtual void TakeContour(XPolyPolygon& rPoly) const;



    virtual Pointer GetCreatePointer() const;

    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear);


    virtual SdrGluePoint GetVertexGluePoint(USHORT nNum) const;


    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);

    // private support routines for ItemSet access. NULL pointer means clear item.
    virtual void ItemSetChanged(const SfxItemSet& rSet);

    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr);
};

}//end of namespace binfilter
#endif //_SVDORECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
