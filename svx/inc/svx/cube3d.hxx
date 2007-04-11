/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cube3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:43:12 $
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

#ifndef _E3D_CUBE3D_HXX
#define _E3D_CUBE3D_HXX

#ifndef _E3D_OBJ3D_HXX
#include <svx/obj3d.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

/*************************************************************************
|*
|*                                                              |
|* 3D-Quader erzeugen; aPos: Zentrum oder links, unten, hinten  |__
|*                           (abhaengig von bPosIsCenter)      /
|* Mit nSideFlags kann angegeben werden, ob nur ein Teil der
|* Quaderflaechen erzeugt werden kann; die entsprechenden Bits
|* sind in dem enum definiert. Das Flag bDblSided legt fest,
|* ob die erzeugten Flaechen doppelseitig sind (nur sinnvoll,
|* wenn nicht alle Flaechen erzeugt wurden).
|*
\************************************************************************/

enum { CUBE_BOTTOM = 0x0001, CUBE_BACK = 0x0002, CUBE_LEFT = 0x0004,
       CUBE_TOP = 0x0008, CUBE_RIGHT = 0x0010, CUBE_FRONT = 0x0020,
       CUBE_FULL = 0x003F, CUBE_OPEN_TB = 0x0036, CUBE_OPEN_LR = 0x002B,
       CUBE_OPEN_FB = 0x001D };

class SVX_DLLPUBLIC E3dCubeObj : public E3dCompoundObject
{
private:
    // Zur Geometrieerzeugung eines Cubes notwendige
    // Parameter
    basegfx::B3DPoint           aCubePos;
    basegfx::B3DVector      aCubeSize;
    UINT16              nSideFlags;

    // BOOLeans
    unsigned            bPosIsCenter            : 1;

protected:
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

public:
    TYPEINFO();
    E3dCubeObj(E3dDefaultAttributes& rDefault, basegfx::B3DPoint aPos, const basegfx::B3DVector& r3DSize);
    E3dCubeObj();

    virtual UINT16 GetObjIdentifier() const;
    virtual SdrObject* DoConvertToPolyObj(BOOL bBezier) const;

    virtual void operator=(const SdrObject&);

    // Geometrieerzeugung
    virtual void CreateGeometry();

    // Give out simple line geometry
    virtual basegfx::B3DPolyPolygon Get3DLineGeometry() const;

    // Lokale Parameter setzen mit Geometrieneuerzeugung
    void SetCubePos(const basegfx::B3DPoint& rNew);
    const basegfx::B3DPoint& GetCubePos() { return aCubePos; }

    void SetCubeSize(const basegfx::B3DVector& rNew);
    const basegfx::B3DVector& GetCubeSize() { return aCubeSize; }

    void SetPosIsCenter(BOOL bNew);
    BOOL GetPosIsCenter() { return (BOOL)bPosIsCenter; }

    void SetSideFlags(UINT16 nNew);
    UINT16 GetSideFlags() { return nSideFlags; }

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
};

#endif          // _E3D_CUBE3D_HXX
