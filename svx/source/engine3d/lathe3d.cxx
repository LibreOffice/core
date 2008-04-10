/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lathe3d.cxx,v $
 * $Revision: 1.26 $
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

#include "svdstr.hrc"
#include "svdglob.hxx"
#include <tools/poly.hxx>
#include <svx/svdpage.hxx>
#include <svx/xoutx.hxx>
#include "globl3d.hxx"
#include <svx/lathe3d.hxx>
#include <svx/xpoly.hxx>

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <svx/svdopath.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/sdr/properties/e3dlatheproperties.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

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

    // Geometrie erzeugen
    CreateGeometry();
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
|* Give out simple line geometry
|*
\************************************************************************/

basegfx::B3DPolyPolygon E3dLatheObj::Get3DLineGeometry() const
{
    return maLinePolyPolygon;
}

/*************************************************************************
|*
|* Die eigentliche Konstruktionmethode, erzeugt einen Koerper durch
|* Rotation des uebergebenen Polygons um die senkrechte Y-Achse. Wenn
|* nEndAngle < 3600 ist, werden ausserdem zwei Deckelflaechen-Polygone
|* erzeugt, die den Koerper abschliessen. Das Polygon sollte in der
|* XY-Ebene liegen, mit X-Koordinaten >= 0; wenn die Anfangs- und End-
|* X-Koordinaten nicht 0 sind, sollte das Polygon geschlossen sein.
|* Wenn bDblSided TRUE ist, werden die Rotationsflaechen doppelseitig
|* angelegt und keine Deckelflaechen erzeugt.
|*
\************************************************************************/

// Geometrieerzeugung
void E3dLatheObj::CreateGeometry()
{
    // Start der Geometrieerzeugung ankuendigen
    StartCreateGeometry();

    // #78972#
    maLinePolyPolygon.clear();

    if(maPolyPoly2D.count())
    {
        // Polygon erzeugen
        // Eventuelle Anpassung der Segmentanzahlen
        basegfx::B2DPolyPolygon aLathePoly2D(CreateLathePolyPoly(maPolyPoly2D, GetVerticalSegments()));
        aLathePoly2D = basegfx::tools::correctOrientations(aLathePoly2D);
        const basegfx::B2VectorOrientation aOrient = basegfx::tools::getOrientation(aLathePoly2D.getB2DPolygon(0L));

        if(basegfx::ORIENTATION_NEGATIVE == aOrient)
        {
            aLathePoly2D.flip();
        }

        // #i28528#
        basegfx::B3DPolyPolygon aFrontLines;
        basegfx::B3DPolyPolygon aBackLines;
        basegfx::B3DPolyPolygon aInBetweenLines;
        basegfx::B3DPolyPolygon aLathePoly3D(basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(aLathePoly2D));

        // Spezialfall Einzelnes Polygon erzeugen
        BOOL bSinglePoly = (GetEndAngle() == 0 || GetHorizontalSegments() == 0);
        if(bSinglePoly)
        {
            // nur ein Polygon erzeugen
            GetProperties().SetObjectItemDirect(Svx3DDoubleSidedItem(TRUE));

            // Fuer evtl. selbst erzeugte Normalen
            basegfx::B3DPolyPolygon aNormalsFront(ImpCreateByPattern(aLathePoly3D));

            // Normalen und Vorderseite selbst erzeugen
            aNormalsFront = ImpAddFrontNormals(aNormalsFront, basegfx::B3DVector(0.0, 0.0, 1.0));
            ImpCreateFront(aLathePoly3D, aNormalsFront, GetCreateNormals(), GetCreateTexture());

            // #i28528#
            aInBetweenLines.append(aLathePoly3D);
        }
        else
        {
            // Eventuell doppelseitig erzeugen?
            if(!aLathePoly3D.isClosed())
            {
                GetProperties().SetObjectItemDirect(Svx3DDoubleSidedItem(TRUE));
            }

            // Seiten genenrieren?
            BOOL bCreateSides = ((GetEndAngle() < 3600 && !GetDoubleSided()) || (GetBackScale() != 100));

            // Polygone vorbereiten
            basegfx::B3DPolyPolygon aPrev, aFront, aBack, aNext;

            // Rotation vorbereiten
            double fAng = DEG2RAD(double(GetEndAngle()) / 10);
            basegfx::B3DHomMatrix aRotMat;

            // Skalierung vorbereiten
            double fScalePerStep(1.0);

            if(GetBackScale() != 100)
            {
                fScalePerStep = (((double)GetBackScale() - 100.0) / 100.0) / (double)GetHorizontalSegments();
            }

            // Texturen erzeugen?
            double fTextureDepth(1.0);
            double fTextureStart(0.0);

            if(!GetCreateTexture())
            {
                fTextureStart = fTextureDepth = 0.0;
            }

            // aPrev bis aBack ausfuellen als Startvorbereitung
            aRotMat.rotate(0.0, -(fAng / (double)GetHorizontalSegments()), 0.0);
            aPrev = aLathePoly3D;
            aPrev.transform(aRotMat);
            if(GetBackScale() != 100)
            {
                // #i74056#
                aPrev = ImpScalePoly(aPrev, 1.0 - fScalePerStep);
            }
            aRotMat.identity();
            aRotMat.rotate(0.0, fAng / (double)GetHorizontalSegments(), 0.0);
            aFront = aLathePoly3D;
            aBack = aLathePoly3D;
            aBack.transform(aRotMat);
            if(GetBackScale() != 100)
            {
                // #i74056#
                aBack = ImpScalePoly(aBack, 1.0 + fScalePerStep);
            }

            // Werte fuer Textur-Zwischensegmenterzeugung berechnen
            double fTmpStart(0.0);
            double fTmpLength(fTextureDepth / (double)GetHorizontalSegments());
            sal_uInt16 nUpperBound((sal_uInt16)GetHorizontalSegments());

            for(UINT16 a=0;a<nUpperBound;a++)
            {
                // Naechstes Polygon vorbereiten
                aNext = aLathePoly3D;

                // Rotieren
                if(!(a+2 == nUpperBound && GetEndAngle() == 3600))
                {
                    aRotMat.identity();
                    aRotMat.rotate(0.0, (fAng * (double)(a+2))/ (double)nUpperBound, 0.0);
                    aNext.transform(aRotMat);
                }

                // Skalieren
                if(GetBackScale() != 100)
                {
                    // #i74056#
                    aNext = ImpScalePoly(aNext, 1.0 + (fScalePerStep * (double)(a+2)));
                }

                // Jetzt Segment erzeugen
                ImpCreateSegment(
                    aFront,
                    aBack,
                    &aPrev,
                    &aNext,
                    (a == 0) && bCreateSides && GetCloseFront(), // #107245# bLatheCloseFront,
                    (a == nUpperBound-1) && bCreateSides && GetCloseBack(), // #107245# bLatheCloseBack,
                    ((double)GetPercentDiagonal() / 200.0)
                        * (double(nUpperBound) / 6.0),
                    GetSmoothNormals(), // #107245# GetLatheSmoothed(),
                    GetSmoothNormals(), // #107245# GetLatheSmoothed(),
                    GetSmoothLids(), // #107245# GetLatheSmoothFrontBack(),
                    1.0,
                    fTmpStart,
                    fTmpLength,
                    GetCreateTexture(),
                    GetCreateNormals(),
                    GetCharacterMode(), // #107245# GetLatheCharacterMode(),
                    TRUE,
                    // #78972#
                    &aFrontLines,
                    &aBackLines,
                    &aInBetweenLines);

                // naechsten Schritt vorbereiten
                fTmpStart += fTmpLength;
                aPrev = aFront;
                aFront = aBack;
                aBack = aNext;
            }
        }

        // #78972#
        // Simply add them for preparing line geometry
        maLinePolyPolygon.append(aFrontLines);
        maLinePolyPolygon.append(aInBetweenLines);
        maLinePolyPolygon.append(aBackLines);

        // #i28528#
        sal_Bool bClosedLines((3600 == GetEndAngle()) && (100 == GetBackScale()));
        basegfx::B3DPolyPolygon aNewPolyPoly(ImpCompleteLinePolygon(maLinePolyPolygon, aLathePoly3D.count(), bClosedLines));

        if(GetReducedLineGeometry())
        {
            // replace vertical with horizontal lines
            maLinePolyPolygon = aNewPolyPoly;

            // append front lines
            maLinePolyPolygon.append(aFrontLines);

            // append back lines
            maLinePolyPolygon.append(aBackLines);
        }
        else
        {
            // append horizontal lines
            maLinePolyPolygon.append(aNewPolyPoly);
        }

        //ImpCorrectLinePolygon(maLinePolyPolygon, aLathePoly3D.count());
    }

    // call parent
    E3dCompoundObject::CreateGeometry();
}

basegfx::B2DPolyPolygon E3dLatheObj::CreateLathePolyPoly(const basegfx::B2DPolyPolygon& rPolyPoly2D, sal_uInt32 nVSegs)
{
    basegfx::B2DPolyPolygon aRetval(rPolyPoly2D);
    const sal_uInt32 nCnt(aRetval.count());
    const basegfx::B2DPolygon aFirstOriginal(aRetval.getB2DPolygon(0L));
    sal_uInt32 nOrigSegmentCnt(aFirstOriginal.count());

    if(nOrigSegmentCnt && !aFirstOriginal.isClosed())
    {
        nOrigSegmentCnt -= 1;
    }

    if(nVSegs && nVSegs != nOrigSegmentCnt)
    {
        // make sure minimum is not too small, 3 edges for closed
        // and 2 edges for open obects
        sal_uInt32 nMinVSegs(aFirstOriginal.isClosed() ? 3L : 2L);

        if(nVSegs <= nMinVSegs)
        {
            nVSegs = nMinVSegs;
        }

        if(nVSegs != nOrigSegmentCnt)
        {
            // Erstes Polygon anpassen
            aRetval.setB2DPolygon(0L, CreateLathePoly(aRetval.getB2DPolygon(0L), nVSegs));
            GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nVSegs));

            // andere Polygone im richtigen Verhaeltnis anpassen,
            // aber nur, wenn Wert fuer erstes angepasst werden musste
            for(sal_uInt32 i(1L); i < nCnt; i++ )
            {
                basegfx::B2DPolygon aPoly2D(aRetval.getB2DPolygon(i));
                sal_uInt32 nSegCnt(aPoly2D.count());

                if(nSegCnt && !aPoly2D.isClosed())
                {
                    nSegCnt -= 1;
                }

                sal_uInt32 nNewVSegs((nSegCnt * nVSegs) / nOrigSegmentCnt);

                // make sure min is not too small for subpolys, too
                if(nNewVSegs <= nMinVSegs)
                {
                    nNewVSegs = nMinVSegs;
                }

                if(nNewVSegs && nNewVSegs != nSegCnt)
                {
                    aRetval.setB2DPolygon(i, CreateLathePoly(aPoly2D, nNewVSegs));
                }
            }
        }
    }

    return aRetval;
}

basegfx::B2DPolygon E3dLatheObj::CreateLathePoly(const basegfx::B2DPolygon& rPoly2D, sal_uInt32 nVSegs)
{
    // attention: Here number of SEGMENTS is given, while GetExpandedPolygon()
    // takes number of points. Calc PntNum first
    sal_uInt32 nNumPts(rPoly2D.isClosed() ? nVSegs : nVSegs + 1L);

    if(nNumPts && rPoly2D.count() && nNumPts != rPoly2D.count())
    {
        // create a expanded or compresssed poly with exactly nNum Points
        basegfx::B2DPolygon aRetval;
        const double fLength(basegfx::tools::getLength(rPoly2D));
        const double fDivisor(rPoly2D.isClosed() ? ((double)nNumPts) : ((double)(nNumPts - 1L)));

        for(sal_uInt32 a(0L); a < nNumPts; a++)
        {
            const double fRelativePos((double)a / fDivisor); // 0.0 .. 1.0 for open, less for closed (without 1.0 e.g. last point)
            const basegfx::B2DPoint aNewPoint(basegfx::tools::getPositionRelative(rPoly2D, fRelativePos, fLength));
            aRetval.append(aNewPoint);
        }

        // copy closed flag
        aRetval.setClosed(rPoly2D.isClosed());
        return aRetval;
    }

    return rPoly2D;
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

UINT16 E3dLatheObj::GetObjIdentifier() const
{
    return E3D_LATHEOBJ_ID;
}

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/

void E3dLatheObj::operator=(const SdrObject& rObj)
{
    // erstmal alle Childs kopieren
    E3dCompoundObject::operator=(rObj);

    // weitere Parameter kopieren
    const E3dLatheObj& r3DObj = (const E3dLatheObj&)rObj;

    maPolyPoly2D  = r3DObj.maPolyPoly2D;

    // #95519# copy LinePolygon info, too
    maLinePolyPolygon = r3DObj.maLinePolyPolygon;
}

/*************************************************************************
|*
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus n Polygonen
|*
\************************************************************************/

SdrObject *E3dLatheObj::DoConvertToPolyObj(BOOL /*bBezier*/) const
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

        bGeometryValid = FALSE;
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

        bGeometryValid = FALSE;
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

BOOL E3dLatheObj::IsBreakObjPossible()
{
    return TRUE;
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
