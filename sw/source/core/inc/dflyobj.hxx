/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dflyobj.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:10:15 $
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
#ifndef _DFLYOBJ_HXX
#define _DFLYOBJ_HXX
#ifndef _SVDOVIRT_HXX //autogen
#include <svx/svdovirt.hxx>
#endif

class SwFlyFrm;
class SwFrmFmt;
class SdrObjMacroHitRec;

const UINT32 SWGInventor =  UINT32('S')*0x00000001+
                            UINT32('W')*0x00000100+
                            UINT32('G')*0x00010000;

const UINT16 SwFlyDrawObjIdentifier = 0x0001;
const UINT16 SwDrawFirst            = 0x0001;

//---------------------------------------
//SwFlyDrawObj, Die DrawObjekte fuer Flys.

class SwFlyDrawObj : public SdrObject
{
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

public:
    TYPEINFO();

    SwFlyDrawObj();
    ~SwFlyDrawObj();

    virtual sal_Bool DoPaintObject(XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const;

    //Damit eine Instanz dieser Klasse beim laden erzeugt werden kann
    //(per Factory).
    virtual UINT32 GetObjInventor()     const;
    virtual UINT16 GetObjIdentifier()   const;
    virtual UINT16 GetObjVersion()      const;
};

//---------------------------------------
//SwVirtFlyDrawObj, die virtuellen Objekte fuer Flys.
//Flys werden immer mit virtuellen Objekten angezeigt. Nur so koennen sie
//ggf. mehrfach angezeigt werden (Kopf-/Fusszeilen).

class SwVirtFlyDrawObj : public SdrVirtObj
{
    SwFlyFrm *pFlyFrm;

public:
    TYPEINFO();

    SwVirtFlyDrawObj(SdrObject& rNew, SwFlyFrm* pFly);
    ~SwVirtFlyDrawObj();

    //Ueberladene Methoden der Basisklasse SdrVirtObj
    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;
    virtual sal_Bool DoPaintObject(XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const;
    virtual void     TakeObjInfo( SdrObjTransformInfoRec& rInfo ) const;

    //Wir nehemen die Groessenbehandlung vollstaendig selbst in die Hand.
    virtual const Rectangle& GetCurrentBoundRect() const;
    virtual       void       RecalcBoundRect();
    virtual       void       RecalcSnapRect();
    virtual const Rectangle& GetSnapRect()  const;
    virtual       void       SetSnapRect(const Rectangle& rRect);
    virtual       void       NbcSetSnapRect(const Rectangle& rRect);
    virtual const Rectangle& GetLogicRect() const;
    virtual       void       SetLogicRect(const Rectangle& rRect);
    virtual       void       NbcSetLogicRect(const Rectangle& rRect);
    virtual ::basegfx::B2DPolyPolygon TakeXorPoly(sal_Bool bDetail) const;
    virtual       void       NbcMove  (const Size& rSiz);
    virtual       void       NbcResize(const Point& rRef, const Fraction& xFact,
                                       const Fraction& yFact);
    virtual       void       Move  (const Size& rSiz);
    virtual       void       Resize(const Point& rRef, const Fraction& xFact,
                                    const Fraction& yFact);

    const SwFrmFmt *GetFmt() const;
          SwFrmFmt *GetFmt();

    // Get Methoden fuer die Fly Verpointerung
          SwFlyFrm* GetFlyFrm()         { return pFlyFrm; }
    const SwFlyFrm* GetFlyFrm() const   { return pFlyFrm; }

    void SetRect() const;

    // ist eine URL an einer Grafik gesetzt, dann ist das ein Makro-Object
    virtual FASTBOOL HasMacro() const;
    virtual SdrObject* CheckMacroHit       (const SdrObjMacroHitRec& rRec) const;
    virtual Pointer    GetMacroPointer     (const SdrObjMacroHitRec& rRec) const;
};


#endif
