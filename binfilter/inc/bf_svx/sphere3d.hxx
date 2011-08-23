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

#ifndef _E3D_SPHERE3D_HXX
#define _E3D_SPHERE3D_HXX

#ifndef _E3D_OBJ3D_HXX
#include <bf_svx/obj3d.hxx>
#endif
namespace binfilter {

/*************************************************************************
|*
|* Kugelobjekt mit Durchmesser r3DSize; Anzahl der Flaechen wird durch
|* die horizontale und vertikale Segmentanzahl vorgegeben
|*
\************************************************************************/

class E3dSphereObj : public E3dCompoundObject
{
    Vector3D		aCenter;
    Vector3D		aSize;

protected:
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

public:
    TYPEINFO();

    // HorizontalSegments:
    sal_Int32 GetHorizontalSegments() const 
        { return ((const Svx3DHorizontalSegmentsItem&)GetUnmergedItemSet().Get(SDRATTR_3DOBJ_HORZ_SEGS)).GetValue(); }

    // VerticalSegments:
    sal_Int32 GetVerticalSegments() const 
        { return ((const Svx3DVerticalSegmentsItem&)GetUnmergedItemSet().Get(SDRATTR_3DOBJ_VERT_SEGS)).GetValue(); }


    virtual SdrObject* DoConvertToPolyObj(BOOL bBezier) const;

    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);

    const Vector3D Center() const { return aCenter; }
    const Vector3D Size() const { return aSize; }

    // FG: Dieser Konstruktor wird nur von MakeObject aus der 3d-Objectfactory beim
    //     Laden von Dokumenten mit Kugeln aufgerufen. Dieser Konstruktor ruft kein
    //     CreateSphere auf, er erzeugt also keine Kugel.
    E3dSphereObj(int dummy);

    // FG: Liest die Kugel im Format der Version 31 ein. Diese Funktion wurde aus
    //     Gruenden der Uebersichtlichkeit ausgelagert.
    void ReadData31(const SdrObjIOHeader& rHead, SvStream& rIn);

    // Geometrieerzeugung
    virtual void CreateGeometry();

    // Give out simple line geometry
    virtual void GetLineGeometry(PolyPolygon3D& rLinePolyPolygon) const;

    // Lokale Parameter setzen mit Geometrieneuerzeugung
    void SetCenter(const Vector3D& rNew);
    void SetSize(const Vector3D& rNew);

    // private support routines for ItemSet access. NULL pointer means clear item.
    virtual void PostItemChange(const sal_uInt16 nWhich);

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
};

}//end of namespace binfilter
#endif			// _E3D_SPHERE3D_HXX
