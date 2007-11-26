/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdorect.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 14:49:22 $
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

#ifndef _SVDORECT_HXX
#define _SVDORECT_HXX

#ifndef _SVDOTEXT_HXX
#include <svx/svdotext.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//************************************************************
//   Vorausdeklarationen
//************************************************************

class XPolygon;

namespace sdr
{
    namespace properties
    {
        class RectangleProperties;
    } // end of namespace properties
} // end of namespace sdr

//************************************************************
//   SdrRectObj
//
// Rechteck-Objekte (Rechteck,Kreis,...)
//
//************************************************************

class SVX_DLLPUBLIC SdrRectObj : public SdrTextObj
{
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    // to allow sdr::properties::RectangleProperties access to SetXPolyDirty()
    friend class sdr::properties::RectangleProperties;

    friend class                SdrTextObj; // wg SetXPolyDirty bei GrowAdjust

protected:
    XPolygon*                   mpXPoly;

protected:
    // Liefert TRUE, wenn das Painten ein Polygon erfordert.
    FASTBOOL PaintNeedsXPoly(long nEckRad) const;

protected:
    XPolygon ImpCalcXPoly(const Rectangle& rRect1, long nRad1) const;
    SdrObject* ImpCheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer, FASTBOOL bForceFilled, FASTBOOL bForceTol=FALSE) const;

    //void PaintIt(XOutputDevice& rOut, _BOOL bDown) const;
    void SetXPolyDirty();

    // RecalcXPoly sollte ueberladen werden. Dabei muss dann eine XPolygon
    // Instanz generiert (new) und an mpXPoly zugewiesen werden.
    virtual void RecalcXPoly();
    const XPolygon& GetXPoly() const;
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
    // der USHORT eFormat nimmt Werte des enum EETextFormat entgegen
    SdrRectObj(SdrObjKind eNewTextKind, const Rectangle& rNewRect, SvStream& rInput, const String& rBaseURL, USHORT eFormat);
    virtual ~SdrRectObj();

    // #i25616#
    void ImpDoPaintRectObjShadow(XOutputDevice& rXOut, sal_Bool bPaintFill, sal_Bool bPaintLine) const;
    void ImpDoPaintRectObj(XOutputDevice& rXOut, sal_Bool bPaintFill, sal_Bool bPaintLine) const;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual UINT16 GetObjIdentifier() const;
    virtual void RecalcBoundRect();
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;
    virtual sal_Bool DoPaintObject(XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const;
    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual void operator=(const SdrObject& rObj);
    virtual void RecalcSnapRect();
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);
    virtual basegfx::B2DPolyPolygon TakeXorPoly(sal_Bool bDetail) const;

    virtual sal_uInt32 GetHdlCount() const;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const;
    virtual FASTBOOL HasSpecialDrag() const;
    virtual FASTBOOL BegDrag(SdrDragStat& rDrag) const;
    virtual FASTBOOL MovDrag(SdrDragStat& rDrag) const;
    virtual FASTBOOL EndDrag(SdrDragStat& rDrag);
    virtual void BrkDrag(SdrDragStat& rDrag) const;

    virtual String GetDragComment(const SdrDragStat& rDrag, FASTBOOL bUndoDragComment, FASTBOOL bCreateComment) const;

    virtual basegfx::B2DPolyPolygon TakeDragPoly(const SdrDragStat& rDrag) const;
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;
    virtual Pointer GetCreatePointer() const;

    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear);

    virtual FASTBOOL DoMacro(const SdrObjMacroHitRec& rRec);
    virtual XubString GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const;

    virtual SdrGluePoint GetVertexGluePoint(USHORT nNum) const;
    virtual SdrGluePoint GetCornerGluePoint(USHORT nNum) const;

    virtual SdrObject* DoConvertToPolyObj(BOOL bBezier) const;

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);
};

#endif //_SVDORECT_HXX

