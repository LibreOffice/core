/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dragmt3d.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 12:35:14 $
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

#ifndef _E3D_DRAGMT3D_HXX
#define _E3D_DRAGMT3D_HXX

#ifndef _SVDDRGMT_HXX
#include "svddrgmt.hxx"
#endif

#ifndef _E3D_VIEW3D_HXX
#include "view3d.hxx"
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

class E3dScene;

/*************************************************************************
|*
|* Parameter fuer Interaktion eines 3D-Objektes
|*
\************************************************************************/

class E3dDragMethodUnit
{
public:
    E3dObject*          p3DObj;
    basegfx::B3DPolygon         aWireframePoly;
    basegfx::B3DHomMatrix           aDisplayTransform;
    basegfx::B3DHomMatrix           aInvDisplayTransform;
    basegfx::B3DHomMatrix            aInitTransform;
    basegfx::B3DHomMatrix           aTransform;
    INT32               nStartAngle;
    INT32               nLastAngle;

    // TimingVars
    UINT8                       nOrigQuality;

    E3dDragMethodUnit() {}
};

/*************************************************************************
|*
|* Parameter fuer Interaktion eines 3D-Objektes
|*
\************************************************************************/

SV_DECL_PTRARR_DEL(E3dDragMethodUnitGroup, E3dDragMethodUnit*, 1, 3)

/*************************************************************************
|*
|* Ableitung von SdrDragMethod fuer 3D-Objekte
|*
\************************************************************************/

class E3dDragMethod : public SdrDragMethod
{
protected:
    E3dDragMethodUnitGroup      aGrp;
    E3dDragConstraint           eConstraint;
    Point                       aLastPos;
    Rectangle                   aFullBound;
    BOOL                        bMoveFull;
    BOOL                        bMovedAtAll;
    Timer                       aCallbackTimer;

public:
    TYPEINFO();
    E3dDragMethod(SdrDragView &rView,
        const SdrMarkList& rMark,
        E3dDragConstraint eConstr = E3DDRAG_CONSTR_XYZ,
        BOOL bFull=FALSE);

    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void Mov(const Point& rPnt);
    virtual void Brk();
    virtual FASTBOOL End(FASTBOOL bCopy);

    //virtual void DrawXor(XOutputDevice& rXOut, FASTBOOL bFull) const;
    E3dView& Get3DView()  { return (E3dView&)rView;  }

    DECL_LINK(TimerInterruptHdl, void*);

    // for migration from XOR to overlay
    virtual void CreateOverlayGeometry(::sdr::overlay::OverlayManager& rOverlayManager, ::sdr::overlay::OverlayObjectList& rOverlayList);
};


/*************************************************************************
|*
|* Ableitung von SdrDragMethod zum Drehen von 3D-Objekten
|*
\************************************************************************/

class E3dDragRotate : public E3dDragMethod
{
    basegfx::B3DPoint           aGlobalCenter;

public:
    TYPEINFO();
    E3dDragRotate(SdrDragView &rView,
        const SdrMarkList& rMark,
        E3dDragConstraint eConstr = E3DDRAG_CONSTR_XYZ,
        BOOL bFull=FALSE);

    virtual void Mov(const Point& rPnt);
    virtual Pointer GetPointer() const;
};


/*************************************************************************
|*
|* Ableitung von SdrDragMethod zum Verschieben von 3D-Subobjekten
|*
\************************************************************************/

class E3dDragMove : public E3dDragMethod
{
    SdrHdlKind              eWhatDragHdl;
    Point                   aScaleFixPos;

public:
    TYPEINFO();
    E3dDragMove(SdrDragView &rView,
        const SdrMarkList& rMark,
        SdrHdlKind eDrgHdl = HDL_MOVE,
        E3dDragConstraint eConstr = E3DDRAG_CONSTR_XYZ,
        BOOL bFull=FALSE);

    virtual void Mov(const Point& rPnt);
    virtual Pointer GetPointer() const;
};


#endif          // _E3D_DRAGMT3D_HXX
