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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include <tools/poly.hxx>
#include <svx/svdpage.hxx>
#include "svx/globl3d.hxx"
#include <svx/lathe3d.hxx>
#include <svx/xpoly.hxx>
#include <svx/svxids.hrc>
#include <svx/svdopath.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/sdr/properties/e3dlatheproperties.hxx>
#include <svx/sdr/contact/viewcontactofe3dlathe.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// #110094# DrawContact section

sdr::contact::ViewContact* E3dLatheObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfE3dLathe(*this);
}

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dLatheObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dLatheProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dLatheObj, E3dCompoundObject);

/*************************************************************************
|*
|* Konstruktor aus 3D-Polygon, Scale gibt den Umrechnungsfaktor fuer
|* die Koordinaten an
|*
\************************************************************************/

E3dLatheObj::E3dLatheObj(E3dDefaultAttributes& rDefault, const basegfx::B2DPolyPolygon rPoly2D)
:   E3dCompoundObject(rDefault),
    maPolyPoly2D(rPoly2D)
{
    // since the old class PolyPolygon3D did mirror the given PolyPolygons in Y, do the same here
    basegfx::B2DHomMatrix aMirrorY;
    aMirrorY.scale(1.0, -1.0);
    maPolyPoly2D.transform(aMirrorY);

    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // Ueberfluessige Punkte entfernen, insbesondere doppelte
    // Start- und Endpunkte verhindern
    maPolyPoly2D.removeDoublePoints();

    if(maPolyPoly2D.count())
    {
        const basegfx::B2DPolygon rPoly(maPolyPoly2D.getB2DPolygon(0L));
        sal_uInt32 nSegCnt(rPoly.count());

        if(nSegCnt && !rPoly.isClosed())
        {
            nSegCnt -= 1;
        }

        GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nSegCnt));
    }
}

/*************************************************************************
|*
|* Leer-Konstruktor
|*
\************************************************************************/

E3dLatheObj::E3dLatheObj()
:    E3dCompoundObject()
{
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

void E3dLatheObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    GetProperties().SetObjectItemDirect(Svx3DSmoothNormalsItem(rDefault.GetDefaultLatheSmoothed()));
    GetProperties().SetObjectItemDirect(Svx3DSmoothLidsItem(rDefault.GetDefaultLatheSmoothFrontBack()));
    GetProperties().SetObjectItemDirect(Svx3DCharacterModeItem(rDefault.GetDefaultLatheCharacterMode()));
    GetProperties().SetObjectItemDirect(Svx3DCloseFrontItem(rDefault.GetDefaultLatheCloseFront()));
    GetProperties().SetObjectItemDirect(Svx3DCloseBackItem(rDefault.GetDefaultLatheCloseBack()));
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

sal_uInt16 E3dLatheObj::GetObjIdentifier() const
{
    return E3D_LATHEOBJ_ID;
}

E3dLatheObj* E3dLatheObj::Clone() const
{
    return CloneHelper< E3dLatheObj >();
}

/*************************************************************************
|*
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus n Polygonen
|*
\************************************************************************/

SdrObject *E3dLatheObj::DoConvertToPolyObj(sal_Bool /*bBezier*/) const
{
    return NULL;
}

/*************************************************************************
|*
|* Neue Segmentierung (Beschreibung siehe Header-File)
|*
\************************************************************************/

void E3dLatheObj::ReSegment(sal_uInt32 nHSegs, sal_uInt32 nVSegs)
{
    if ((nHSegs != GetHorizontalSegments() || nVSegs != GetVerticalSegments()) &&
        (nHSegs != 0 || nVSegs != 0))
    {
        GetProperties().SetObjectItemDirect(Svx3DHorizontalSegmentsItem(nHSegs));
        GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nVSegs));

        ActionChanged();
    }
}

/*************************************************************************
|*
|* Lokale Parameter setzen mit Geometrieneuerzeugung
|*
\************************************************************************/

void E3dLatheObj::SetPolyPoly2D(const basegfx::B2DPolyPolygon& rNew)
{
    if(maPolyPoly2D != rNew)
    {
        maPolyPoly2D = rNew;
        maPolyPoly2D.removeDoublePoints();

        if(maPolyPoly2D.count())
        {
            const basegfx::B2DPolygon rPoly(maPolyPoly2D.getB2DPolygon(0L));
            sal_uInt32 nSegCnt(rPoly.count());

            if(nSegCnt && !rPoly.isClosed())
            {
                nSegCnt -= 1;
            }

            GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nSegCnt));
        }

        ActionChanged();
    }
}

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/

void E3dLatheObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulLathe3d);

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

/*************************************************************************
|*
|* Get the name of the object (plural)
|*
\************************************************************************/

void E3dLatheObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralLathe3d);
}

/*************************************************************************
|*
|* Aufbrechen
|*
\************************************************************************/

sal_Bool E3dLatheObj::IsBreakObjPossible()
{
    return sal_True;
}

SdrAttrObj* E3dLatheObj::GetBreakObj()
{
    // create PathObj
    basegfx::B3DPolyPolygon aLathePoly3D(basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(maPolyPoly2D));
    basegfx::B2DPolyPolygon aTransPoly(TransformToScreenCoor(aLathePoly3D));
    SdrPathObj* pPathObj = new SdrPathObj(OBJ_PLIN, aTransPoly);

    if(pPathObj)
    {
        // Attribute setzen
        SfxItemSet aSet(GetObjectItemSet());

        // Linien aktivieren, um Objekt garantiert sichtbar zu machen
        aSet.Put(XLineStyleItem(XLINE_SOLID));

        pPathObj->SetMergedItemSet(aSet);
    }

    return pPathObj;
}

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
