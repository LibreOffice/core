/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: extrud3d.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:19:00 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "svdstr.hrc"
#include "svdglob.hxx"

#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _E3D_EXTRUD3D_HXX
#include "extrud3d.hxx"
#endif

#ifndef _E3D_SCENE3D_HXX
#include "scene3d.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _XPOLY_HXX
#include "xpoly.hxx"
#endif

#ifndef _SVDOPATH_HXX
#include "svdopath.hxx"
#endif

#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif

#ifndef _SVX3DITEMS_HXX
#include "svx3ditems.hxx"
#endif

#ifndef _SDR_PROPERTIES_E3DEXTRUDEPROPERTIES_HXX
#include <svx/sdr/properties/e3dextrudeproperties.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolygontools.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dExtrudeObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dExtrudeProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dExtrudeObj, E3dCompoundObject);

/*************************************************************************
|*
|* Konstruktor, erzeugt zwei Deckelflaechen-PolyPolygone und (PointCount-1)
|* Seitenflaechen-Rechtecke aus dem uebergebenen PolyPolygon
|*
\************************************************************************/

E3dExtrudeObj::E3dExtrudeObj(E3dDefaultAttributes& rDefault, const basegfx::B2DPolyPolygon& rPP, double fDepth)
:   E3dCompoundObject(rDefault),
    maExtrudePolygon(rPP)
{
    // since the old class PolyPolygon3D did mirror the given PolyPolygons in Y, do the same here
    basegfx::B2DHomMatrix aMirrorY;
    aMirrorY.scale(1.0, -1.0);
    maExtrudePolygon.transform(aMirrorY);

    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // set extrude depth
    GetProperties().SetObjectItemDirect(Svx3DDepthItem((sal_uInt32)(fDepth + 0.5)));

    // Geometrie erzeugen
    CreateGeometry();
}

E3dExtrudeObj::E3dExtrudeObj()
:   E3dCompoundObject()
{
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

void E3dExtrudeObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    GetProperties().SetObjectItemDirect(Svx3DSmoothNormalsItem(rDefault.GetDefaultExtrudeSmoothed()));
    GetProperties().SetObjectItemDirect(Svx3DSmoothLidsItem(rDefault.GetDefaultExtrudeSmoothFrontBack()));
    GetProperties().SetObjectItemDirect(Svx3DCharacterModeItem(rDefault.GetDefaultExtrudeCharacterMode()));
    GetProperties().SetObjectItemDirect(Svx3DCloseFrontItem(rDefault.GetDefaultExtrudeCloseFront()));
    GetProperties().SetObjectItemDirect(Svx3DCloseBackItem(rDefault.GetDefaultExtrudeCloseBack()));

    // Bei extrudes defaultmaessig StdTexture in X und Y
    GetProperties().SetObjectItemDirect(Svx3DTextureProjectionXItem(1));
    GetProperties().SetObjectItemDirect(Svx3DTextureProjectionYItem(1));
}

/*************************************************************************
|*
|* Geometrieerzeugung
|*
\************************************************************************/

basegfx::B3DPolyPolygon E3dExtrudeObj::GetFrontSide()
{
    basegfx::B3DPolyPolygon aRetval;

    if(maExtrudePolygon.count())
    {
        basegfx::B2DPolyPolygon aTemp(maExtrudePolygon);
        aTemp.removeDoublePoints();
        aTemp = basegfx::tools::correctOrientations(aTemp);
        const basegfx::B2VectorOrientation aOrient = basegfx::tools::getOrientation(aTemp.getB2DPolygon(0L));

        if(basegfx::ORIENTATION_POSITIVE == aOrient)
        {
            aTemp.flip();
        }

        aRetval = basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(aTemp);
    }

    return aRetval;
}

basegfx::B3DPolyPolygon E3dExtrudeObj::GetBackSide(const basegfx::B3DPolyPolygon& rFrontSide)
{
    basegfx::B3DPolyPolygon aBackSide(rFrontSide);

    if(GetExtrudeDepth() != 0)
    {
        // eventuell Skalieren
        if(GetPercentBackScale() != 100)
            ImpScalePoly(aBackSide, (double)GetPercentBackScale() / 100.0);

        // Verschieben
        basegfx::B3DHomMatrix aTrans;
        aTrans.translate(0.0, 0.0, (double)GetExtrudeDepth());
        aBackSide.transform(aTrans);
    }

    return aBackSide;
}

/*************************************************************************
|*
|* Give out simple line geometry
|*
\************************************************************************/

basegfx::B3DPolyPolygon E3dExtrudeObj::Get3DLineGeometry() const
{
    return maLinePolyPolygon;
}

void E3dExtrudeObj::CreateGeometry()
{
    // Start der Geometrieerzeugung ankuendigen
    StartCreateGeometry();

    // #78972# prepare new line geometry creation
    maLinePolyPolygon.clear();

    // Polygon als Grundlage holen
    basegfx::B3DPolyPolygon aFrontSide(GetFrontSide());

    if(aFrontSide.count())
    {
        if(GetExtrudeDepth() != 0)
        {
            // Hinteres Polygon erzeugen
            basegfx::B3DPolyPolygon aBackSide(GetBackSide(aFrontSide));

            // Was muss erzeugt werden?
            if(!aFrontSide.isClosed())
            {
                GetProperties().SetObjectItemDirect(Svx3DDoubleSidedItem(TRUE));
            }

            double fTextureDepth(1.0);
            double fTextureStart(0.0);

            // Texturen erzeugen?
            if(!GetCreateTexture())
            {
                fTextureStart = fTextureDepth = 0.0;
            }

            // Falls Texturen erzeugen Randbreite fuer diese bestimmen
            double fSurroundFactor(1.0);

            if(GetCreateTexture())
            {
                const basegfx::B3DPolygon aFirstPolygon(aFrontSide.getB3DPolygon(0L));
                const double fLength(basegfx::tools::getLength(aFirstPolygon));
                const double fArea(basegfx::tools::getArea(aFirstPolygon));
                fSurroundFactor = fLength / sqrt(fArea);
                fSurroundFactor = (double)((long)(fSurroundFactor - 0.5));
                if(fSurroundFactor == 0.0)
                    fSurroundFactor = 1.0;
            }

            // #i28528#
            basegfx::B3DPolyPolygon aFrontLines;
            basegfx::B3DPolyPolygon aBackLines;
            basegfx::B3DPolyPolygon aInBetweenLines;

            // Segment erzeugen
            ImpCreateSegment(
                aFrontSide,
                aBackSide,
                0L,
                0L,
                GetCloseFront(), // #107245# bExtrudeCloseFront,
                GetCloseBack(), // #107245# bExtrudeCloseBack,
                (double)GetPercentDiagonal() / 200.0,
                GetSmoothNormals(), // #107245# GetExtrudeSmoothed(),
                GetSmoothNormals(), // #107245# GetExtrudeSmoothed(),
                GetSmoothLids(), // #107245# GetExtrudeSmoothFrontBack(),
                fSurroundFactor,
                fTextureStart,
                fTextureDepth,
                GetCreateNormals(),
                GetCreateTexture(),
                GetCharacterMode(), // #107245# bExtrudeCharacterMode,
                FALSE,
                // #78972#
                &aFrontLines,
                &aBackLines,
                &aInBetweenLines);

            // #78972#
            // Simply add them for Extrudes
            maLinePolyPolygon.append(aFrontLines);
            maLinePolyPolygon.append(aInBetweenLines);
            maLinePolyPolygon.append(aBackLines);
        }
        else
        {
            // nur ein Polygon erzeugen
            GetProperties().SetObjectItemDirect(Svx3DDoubleSidedItem(TRUE));

            // Fuer evtl. selbst erzeugte Normalen
            basegfx::B3DPolyPolygon aNormalsFront(ImpCreateByPattern(aFrontSide));

            // Extrudevektor bilden
            basegfx::B3DVector aNormal(0.0, 0.0, (double)GetExtrudeDepth());

            // Normalen und Vorderseite selbst erzeugen
            aNormalsFront = ImpAddFrontNormals(aNormalsFront, aNormal);
            ImpCreateFront(aFrontSide, aNormalsFront, GetCreateNormals(), GetCreateTexture());

            // #78972#
            maLinePolyPolygon.append(aFrontSide);
        }

        // #i28528#
        if(!GetReducedLineGeometry())
        {
            basegfx::B3DPolyPolygon aNewPolyPoly(ImpCompleteLinePolygon(maLinePolyPolygon, aFrontSide.count(), sal_False));
            // append horizontal lines
            maLinePolyPolygon.append(aNewPolyPoly);
        }

        //ImpCorrectLinePolygon(maLinePolyPolygon, aFrontSide.count());
    }

    // call parent
    E3dCompoundObject::CreateGeometry();
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

UINT16 E3dExtrudeObj::GetObjIdentifier() const
{
    return E3D_EXTRUDEOBJ_ID;
}

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/

void E3dExtrudeObj::operator=(const SdrObject& rObj)
{
    // erstmal alle Childs kopieren
    E3dCompoundObject::operator=(rObj);

    // weitere Parameter kopieren
    const E3dExtrudeObj& r3DObj = (const E3dExtrudeObj&)rObj;

    maExtrudePolygon = r3DObj.maExtrudePolygon;

    // #95519# copy LinePolygon info, too
    maLinePolyPolygon = r3DObj.maLinePolyPolygon;
}

/*************************************************************************
|*
|* Lokale Parameter setzen mit Geometrieneuerzeugung
|*
\************************************************************************/

void E3dExtrudeObj::SetExtrudePolygon(const basegfx::B2DPolyPolygon &rNew)
{
    if(maExtrudePolygon != rNew)
    {
        maExtrudePolygon = rNew;
        bGeometryValid = FALSE;
    }
}

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/

void E3dExtrudeObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulExtrude3d);

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

void E3dExtrudeObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralExtrude3d);
}

/*************************************************************************
|*
|* Aufbrechen
|*
\************************************************************************/

BOOL E3dExtrudeObj::IsBreakObjPossible()
{
    return TRUE;
}

SdrAttrObj* E3dExtrudeObj::GetBreakObj()
{
    // create PathObj
    basegfx::B2DPolyPolygon aPoly = TransformToScreenCoor(GetBackSide(GetFrontSide()));
    SdrPathObj* pPathObj = new SdrPathObj(OBJ_PLIN, aPoly);

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
