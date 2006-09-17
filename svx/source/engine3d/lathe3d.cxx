/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lathe3d.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:56:43 $
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

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif

#ifndef _XOUTX_HXX
#include "xoutx.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _E3D_LATHE3D_HXX
#include "lathe3d.hxx"
#endif

#ifndef _POLY3D_HXX
#include "poly3d.hxx"
#endif

#ifndef _XPOLY_HXX
#include "xpoly.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
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

#ifndef _SDR_PROPERTIES_E3DLATHEPROPERTIES_HXX
#include <svx/sdr/properties/e3dlatheproperties.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dLatheObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dLatheProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dLatheObj, E3dCompoundObject);

/*************************************************************************
|*
|* Konstruktor aus SV-Polygon, Scale gibt den Umrechnungsfaktor fuer
|* die Koordinaten an
|*
\************************************************************************/

E3dLatheObj::E3dLatheObj(E3dDefaultAttributes& rDefault, const PolyPolygon& rPoly)
:   E3dCompoundObject(rDefault),
    aPolyPoly3D (PolyPolygon3D(rPoly, rDefault.GetDefaultLatheScale()))
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // Ueberfluessige Punkte entfernen, insbesondere doppelte
    // Start- und Endpunkte verhindern
    aPolyPoly3D.RemoveDoublePoints();

    const Polygon3D rPoly3D = aPolyPoly3D[0];
    sal_uInt32 nSegCnt((sal_uInt32)rPoly3D.GetPointCount());
    if(nSegCnt && !rPoly3D.IsClosed())
        nSegCnt -= 1;

    GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nSegCnt));

    // Geometrie erzeugen
    CreateGeometry();
}

/*************************************************************************
|*
|* wie voriger Konstruktor, nur mit XPolygon; das XPolygon wird
|* jedoch nicht Bezier-konvertiert, sondern es werden nur seine
|* Punktkoordinaten uebernommen
|*
\************************************************************************/

E3dLatheObj::E3dLatheObj(E3dDefaultAttributes& rDefault, const XPolyPolygon& rXPoly)
:   E3dCompoundObject(rDefault),
    aPolyPoly3D (PolyPolygon3D(rXPoly, rDefault.GetDefaultLatheScale()))
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // Ueberfluessige Punkte entfernen, insbesondere doppelte
    // Start- und Endpunkte verhindern
    aPolyPoly3D.RemoveDoublePoints();
    const Polygon3D rPoly = aPolyPoly3D[0];
    sal_uInt32 nSegCnt((sal_uInt32)rPoly.GetPointCount());
    if(nSegCnt && !rPoly.IsClosed())
        nSegCnt -= 1;

    GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nSegCnt));

    // Geometrie erzeugen
    CreateGeometry();
}

/*************************************************************************
|*
\************************************************************************/

E3dLatheObj::E3dLatheObj(E3dDefaultAttributes& rDefault, const XPolygon& rXPoly)
:   E3dCompoundObject(rDefault),
    aPolyPoly3D (PolyPolygon3D(rXPoly, rDefault.GetDefaultLatheScale()))
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // Ueberfluessige Punkte entfernen, insbesondere doppelte
    // Start- und Endpunkte verhindern
    aPolyPoly3D.RemoveDoublePoints();
    const Polygon3D rPoly = aPolyPoly3D[0];
    sal_uInt32 nSegCnt((sal_uInt32)rPoly.GetPointCount());
    if(nSegCnt && !rPoly.IsClosed())
        nSegCnt -= 1;

    GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nSegCnt));

    // Geometrie erzeugen
    CreateGeometry();
}

/*************************************************************************
|*
|* Konstruktor aus 3D-Polygon, Scale gibt den Umrechnungsfaktor fuer
|* die Koordinaten an
|*
\************************************************************************/

E3dLatheObj::E3dLatheObj (E3dDefaultAttributes& rDefault, const PolyPolygon3D rPoly3D)
:   E3dCompoundObject(rDefault),
    aPolyPoly3D(rPoly3D)
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // Ueberfluessige Punkte entfernen, insbesondere doppelte
    // Start- und Endpunkte verhindern
    aPolyPoly3D.RemoveDoublePoints();
    const Polygon3D rPoly = aPolyPoly3D[0];
    sal_uInt32 nSegCnt((sal_uInt32)rPoly.GetPointCount());
    if(nSegCnt && !rPoly.IsClosed())
        nSegCnt -= 1;

    GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nSegCnt));

    // Geometrie erzeugen
    CreateGeometry();
}

/*************************************************************************
|*
|* Leer-Konstruktor
|*
\************************************************************************/

E3dLatheObj::E3dLatheObj()
:    aPolyPoly3D(Polygon3D())
{
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

void E3dLatheObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    fLatheScale = rDefault.GetDefaultLatheScale();

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

::basegfx::B3DPolyPolygon E3dLatheObj::Get3DLineGeometry() const
{
    ::basegfx::B3DPolyPolygon aRetval;

    aRetval.append(maLinePolyPolygon.getB3DPolyPolygon());

    return aRetval;
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
    maLinePolyPolygon.Clear();

    // Polygon erzeugen
    PolyPolygon3D aLathePoly3D(aPolyPoly3D);

    // Eventuelle Anpassung der Segmentanzahlen
    aLathePoly3D = CreateLathePolyPoly(aPolyPoly3D, GetVerticalSegments());

    // Normale holen
    Vector3D aNormal = aLathePoly3D.GetNormal();
    if(aNormal.Z() > 0.0)
    {
        aLathePoly3D.FlipDirections();
        aNormal = aLathePoly3D.GetNormal();
    }

    // Orientierung evtl. vorhandener Loecher in einen definierten
    // Ausgangszustand bringen
    aLathePoly3D.SetDirections();

    // #i28528#
    PolyPolygon3D aFrontLines;
    PolyPolygon3D aBackLines;
    PolyPolygon3D aInBetweenLines;

    // Spezialfall Einzelnes Polygon erzeugen
    BOOL bSinglePoly = (GetEndAngle() == 0 || GetHorizontalSegments() == 0);
    if(bSinglePoly)
    {
        // nur ein Polygon erzeugen
        GetProperties().SetObjectItemDirect(Svx3DDoubleSidedItem(TRUE));

        // Fuer evtl. selbst erzeugte Normalen
        PolyPolygon3D aNormalsFront;

        // Normalen und Vorderseite selbst erzeugen
        AddFrontNormals(aLathePoly3D, aNormalsFront, aNormal);
        CreateFront(aLathePoly3D, aNormalsFront, GetCreateNormals(), GetCreateTexture());

        // #i28528#
        aInBetweenLines.Insert(aLathePoly3D);
    }
    else
    {
        // Eventuell doppelseitig erzeugen?
        if(!aLathePoly3D.IsClosed())
        {
            GetProperties().SetObjectItemDirect(Svx3DDoubleSidedItem(TRUE));
        }

        // Seiten genenrieren?
        BOOL bCreateSides = ((GetEndAngle() < 3600 && !GetDoubleSided())
            || (GetBackScale() != 100));

        // Polygone vorbereiten
        PolyPolygon3D aPrev, aFront, aBack, aNext;

        // Rotation vorbereiten
        double fAng = DEG2RAD(double(GetEndAngle()) / 10);
        Matrix4D aRotMat;

        // Skalierung vorbereiten
        double fScalePerStep(0.0);
        if(GetBackScale() != 100)
            fScalePerStep = (((double)GetBackScale() - 100.0) / 100.0) / (double)GetHorizontalSegments();

        // Texturen erzeugen?
        double fTextureDepth=1.0;
        double fTextureStart=0.0;
        if(!GetCreateTexture())
            fTextureStart = fTextureDepth = 0.0;

        // aPrev bis aBack ausfuellen als Startvorbereitung
        aRotMat.RotateY(-(fAng / (double)GetHorizontalSegments()));
        aPrev = aLathePoly3D;
        RotatePoly(aPrev, aRotMat);
        if(GetBackScale() != 100)
        {
            ScalePoly(aPrev, 1.0 - fScalePerStep);
        }
        aRotMat.Identity();
        aRotMat.RotateY(fAng / (double)GetHorizontalSegments());
        aFront = aLathePoly3D;
        aBack = aLathePoly3D;
        RotatePoly(aBack, aRotMat);
        if(GetBackScale() != 100)
        {
            ScalePoly(aBack, 1.0 + fScalePerStep);
        }

        // Werte fuer Textur-Zwischensegmenterzeugung berechnen
        double fTmpStart = 0.0;
        double fTmpLength = fTextureDepth / (double)GetHorizontalSegments();
        sal_uInt16 nUpperBound = (sal_uInt16)GetHorizontalSegments();

        for(UINT16 a=0;a<nUpperBound;a++)
        {
            // Naechstes Polygon vorbereiten
            aNext = aLathePoly3D;

            // Rotieren
            if(!(a+2 == nUpperBound && GetEndAngle() == 3600))
            {
                aRotMat.Identity();
                aRotMat.RotateY((fAng * (double)(a+2))/ (double)nUpperBound);
                RotatePoly(aNext, aRotMat);
            }

            // Skalieren
            if(GetBackScale() != 100)
            {
                ScalePoly(aNext, 1.0 + (fScalePerStep * (double)(a+2)));
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
    maLinePolyPolygon.Insert(aFrontLines);
    maLinePolyPolygon.Insert(aInBetweenLines);
    maLinePolyPolygon.Insert(aBackLines);

    // #i28528#
    sal_Bool bClosedLines((3600 == GetEndAngle()) && (100 == GetBackScale()));
    PolyPolygon3D aNewPolyPoly = ImpCompleteLinePolygon(maLinePolyPolygon, aLathePoly3D.Count(), bClosedLines);

    if(GetReducedLineGeometry())
    {
        // replace vertical with horizontal lines
        maLinePolyPolygon = aNewPolyPoly;

        // append front lines
        maLinePolyPolygon.Insert(aFrontLines);

        // append back lines
        maLinePolyPolygon.Insert(aBackLines);
    }
    else
    {
        // append horizontal lines
        maLinePolyPolygon.Insert(aNewPolyPoly);
    }

    ImpCorrectLinePolygon(maLinePolyPolygon, aLathePoly3D.Count());

    // call parent
    E3dCompoundObject::CreateGeometry();
}

PolyPolygon3D E3dLatheObj::CreateLathePolyPoly(PolyPolygon3D& rPolyPoly3D, long nVSegs)
{
    PolyPolygon3D aLathePolyPolygon3D = rPolyPoly3D;
    sal_uInt16 nCnt = aLathePolyPolygon3D.Count();
    sal_uInt16 nOrigSegmentCnt = aPolyPoly3D[0].GetPointCount();

    if(nOrigSegmentCnt && !aPolyPoly3D[0].IsClosed())
        nOrigSegmentCnt -= 1;

    if(nVSegs && nVSegs != nOrigSegmentCnt)
    {
        // make sure minimum is not too small, 3 edges for closed
        // and 2 edges for open obects
        sal_Int32 nMinVSegs = aPolyPoly3D[0].IsClosed() ? 3 : 2;
        if(nVSegs <= nMinVSegs)
            nVSegs = nMinVSegs;

        if(nVSegs != nOrigSegmentCnt)
        {
            // Erstes Polygon anpassen
            aLathePolyPolygon3D[0] = CreateLathePoly(aLathePolyPolygon3D[0], nVSegs);

            GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nVSegs));

            // andere Polygone im richtigen Verhaeltnis anpassen,
            // aber nur, wenn Wert fuer erstes angepasst werden musste
            for(UINT16 i = 1; i < nCnt; i++ )
            {
                Polygon3D &rPoly3D = aLathePolyPolygon3D[i];
                sal_uInt16 nSegCnt(rPoly3D.GetPointCount());
                if(nSegCnt && !rPoly3D.IsClosed())
                    nSegCnt -= 1;
                long nNewVSegs = (nSegCnt * nVSegs) / nOrigSegmentCnt;

                // make sure min is not too small for subpolys, too
                if(nNewVSegs <= nMinVSegs)
                    nNewVSegs = nMinVSegs;

                if(nNewVSegs && nNewVSegs != nSegCnt)
                {
                    aLathePolyPolygon3D[i] = CreateLathePoly(aLathePolyPolygon3D[i], nNewVSegs);
                }
            }
        }
    }
    return aLathePolyPolygon3D;
}

Polygon3D E3dLatheObj::CreateLathePoly(Polygon3D& rPoly3D, long nVSegs)
{
    // attention: Here number of SEGMENTS is given, while GetExpandedPolygon()
    // takes number of points. Calc PntNum first
    long nNumPts = rPoly3D.IsClosed() ? nVSegs : nVSegs + 1;
    if(nNumPts != rPoly3D.GetPointCount())
        return rPoly3D.GetExpandedPolygon(nNumPts);
    return rPoly3D;
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
|* Wireframe erzeugen
|*
\************************************************************************/

//BFS01void E3dLatheObj::CreateWireframe(Polygon3D& rWirePoly, const Matrix4D* pTf, E3dDragDetail eDetail)
//BFS01{
//BFS01 // Nur selbst erzeugen, wenn alle Linien angezeigt werden sollen
//BFS01 if ( eDetail == E3DDETAIL_ALLLINES ||
//BFS01     (eDetail == E3DDETAIL_DEFAULT && GetDragDetail() == E3DDETAIL_ALLLINES) )
//BFS01 {
//BFS01     // Detailliert erzeugen
//BFS01 }
//BFS01 else
//BFS01 {
//BFS01     // call parent
//BFS01     E3dObject::CreateWireframe(rWirePoly, pTf, eDetail);
//BFS01 }
//BFS01}

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

    aPolyPoly3D  = r3DObj.aPolyPoly3D;
    fLatheScale  = r3DObj.fLatheScale;

    // #95519# copy LinePolygon info, too
    maLinePolyPolygon = r3DObj.maLinePolyPolygon;

    // #107245# These properties are now items and are copied with the ItemSet
    // // Ab Version 374 (15.12.97)
    // bLatheSmoothed = r3DObj.bLatheSmoothed;
    // bLatheSmoothFrontBack = r3DObj.bLatheSmoothFrontBack;
    // bLatheCharacterMode = r3DObj.bLatheCharacterMode;
    // bLatheCloseFront = r3DObj.bLatheCloseFront;
    // bLatheCloseBack = r3DObj.bLatheCloseBack;
}

/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

//BFS01void E3dLatheObj::WriteData(SvStream& rOut) const
//BFS01{
//BFS01#ifndef SVX_LIGHT
//BFS01 long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
//BFS01 if(nVersion < 3800)
//BFS01 {
//BFS01     // Alte Geometrie erzeugen, um die E3dPolyObj's zu haben
//BFS01     ((E3dCompoundObject*)this)->ReCreateGeometry(TRUE);
//BFS01 }
//BFS01
//BFS01 // leider kann das E3dLatheObj nicht auf E3dObject abgestuetzt werden,
//BFS01 // da neue Member hinzugekommen sind und die Kompatibilitaet erhalten
//BFS01 // bleiben muss.
//BFS01 SdrAttrObj::WriteData(rOut);
//BFS01
//BFS01 // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01 SdrDownCompat aCompat(rOut, STREAM_WRITE);
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("E3dLatheObj");
//BFS01#endif
//BFS01
//BFS01 pSub->Save(rOut);
//BFS01
//BFS01 // Parameter aus E3dObject speichern
//BFS01 rOut << aLocalBoundVol;
//BFS01 Old_Matrix3D aMat3D;
//BFS01 aMat3D = aTfMatrix;
//BFS01 rOut << aMat3D;
//BFS01 rOut << nLogicalGroup;
//BFS01 rOut << nObjTreeLevel;
//BFS01 rOut << nPartOfParent;
//BFS01 rOut << UINT16(eDragDetail);
//BFS01
//BFS01 // neue Member
//BFS01 // Alte version schreibt Polygon3D raus, neue Version
//BFS01 // benutzt dafuer das erste Teilpolygon von PolyPolygon3D
//BFS01 // rOut << aPolyPoly3D;
//BFS01 rOut << aPolyPoly3D[0];
//BFS01
//BFS01 rOut << GetHorizontalSegments();
//BFS01
//BFS01 rOut << GetEndAngle();
//BFS01
//BFS01 rOut << ((E3dLatheObj*)this)->GetDoubleSided();
//BFS01 rOut << fLatheScale;
//BFS01
//BFS01 // Ab Version 364f (19.06.97)
//BFS01
//BFS01 // #83965# internally the real number of segments (edges) is
//BFS01 // used, no longer the number of points
//BFS01 sal_Int32 nVSegs = GetVerticalSegments();
//BFS01 if(!aPolyPoly3D[0].IsClosed())
//BFS01     nVSegs += 1;
//BFS01
//BFS01 rOut << nVSegs;
//BFS01
//BFS01 // Ab Version 374 (15.12.97)
//BFS01 rOut << aPolyPoly3D;
//BFS01
//BFS01 rOut << ((double)GetBackScale() / 100.0);
//BFS01
//BFS01 rOut << ((double)GetPercentDiagonal() / 200.0);
//BFS01
//BFS01 rOut << GetSmoothNormals(); // #107245# (BOOL)bLatheSmoothed;
//BFS01 rOut << GetSmoothLids(); // #107245# (BOOL)bLatheSmoothFrontBack;
//BFS01 rOut << GetCharacterMode(); // #107245# (BOOL)bLatheCharacterMode;
//BFS01
//BFS01 // Ab Version 395 (8.6.98): Parameter aus dem Objekt
//BFS01 // E3dCompoundObject. Da irgendwann mal jemand die Ableitungs-
//BFS01 // hierarchie beim FileFormat unterbrochen hat, wurden diese Attribute
//BFS01 // bisher NOCH NIE gespeichert (Grrr). Diese Stelle muss nun natuerlich
//BFS01 // auch IMMER MITGEPFLEGT werden, wenn sich Parameter in
//BFS01 // E3dCompoundObject oder E3dObject aendern.
//BFS01 rOut << GetDoubleSided();
//BFS01
//BFS01 rOut << BOOL(bCreateNormals);
//BFS01 rOut << BOOL(bCreateTexture);
//BFS01
//BFS01 sal_uInt16 nVal = GetNormalsKind();
//BFS01 rOut << BOOL(nVal > 0);
//BFS01 rOut << BOOL(nVal > 1);
//BFS01
//BFS01 nVal = GetTextureProjectionX();
//BFS01 rOut << BOOL(nVal > 0);
//BFS01 rOut << BOOL(nVal > 1);
//BFS01
//BFS01 nVal = GetTextureProjectionY();
//BFS01 rOut << BOOL(nVal > 0);
//BFS01 rOut << BOOL(nVal > 1);
//BFS01
//BFS01 rOut << BOOL(GetShadow3D());
//BFS01
//BFS01 rOut << GetMaterialAmbientColor();
//BFS01 rOut << GetMaterialColor();
//BFS01 rOut << GetMaterialSpecular();
//BFS01 rOut << GetMaterialEmission();
//BFS01 rOut << GetMaterialSpecularIntensity();
//BFS01
//BFS01 aBackMaterial.WriteData(rOut);
//BFS01
//BFS01 rOut << (UINT16)GetTextureKind();
//BFS01
//BFS01 rOut << (UINT16)GetTextureMode();
//BFS01
//BFS01 rOut << BOOL(GetNormalsInvert());
//BFS01
//BFS01 // Ab Version 513a (5.2.99): Parameter fuer das
//BFS01 // Erzeugen der Vorder/Rueckwand
//BFS01 rOut << GetCloseFront(); // #107245# BOOL(bLatheCloseFront);
//BFS01 rOut << GetCloseBack(); // #107245# BOOL(bLatheCloseBack);
//BFS01
//BFS01 // neu ab 534: (hat noch gefehlt)
//BFS01 rOut << BOOL(GetTextureFilter());
//BFS01
//BFS01 if(nVersion < 3800)
//BFS01 {
//BFS01     // Geometrie neu erzeugen, um E3dPolyObj's wieder loszuwerden
//BFS01     ((E3dCompoundObject*)this)->ReCreateGeometry();
//BFS01 }
//BFS01#endif
//BFS01}

/*************************************************************************
|*
|* Objektdaten aus Stream laden
|*
\************************************************************************/

//BFS01void E3dLatheObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
//BFS01{
//BFS01 if (ImpCheckSubRecords (rHead, rIn))
//BFS01 {
//BFS01     // leider kann das E3dLatheObj nicht auf E3dObject abgestuetzt werden,
//BFS01     // da neue Member hinzugekommen sind und die Kompatibilitaet erhalten
//BFS01     // bleiben muss.
//BFS01     SdrAttrObj::ReadData(rHead, rIn);
//BFS01
//BFS01     // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01     SdrDownCompat aCompat(rIn, STREAM_READ);
//BFS01#ifdef DBG_UTIL
//BFS01     aCompat.SetID("E3dLatheObj");
//BFS01#endif
//BFS01     // dann die Member
//BFS01     UINT16  nTmp16;
//BFS01
//BFS01     // #106240# Flag if poly was loaded (all versions above 3.0 and 3.1)
//BFS01     sal_Bool bPolyWasRead(sal_False);
//BFS01
//BFS01     pSub->Load(rIn, *pPage);
//BFS01
//BFS01     // Parameter aus E3dObject laden
//BFS01     rIn >> aLocalBoundVol;
//BFS01     Old_Matrix3D aMat3D;
//BFS01     rIn >> aMat3D;
//BFS01     aTfMatrix = Matrix4D(aMat3D);
//BFS01     rIn >> nLogicalGroup;
//BFS01     rIn >> nObjTreeLevel;
//BFS01     rIn >> nPartOfParent;
//BFS01     rIn >> nTmp16; eDragDetail = E3dDragDetail(nTmp16);
//BFS01
//BFS01     // BoundVolume muss neu berechnet werden
//BFS01     bBoundVolValid = FALSE;
//BFS01
//BFS01     if (aCompat.GetBytesLeft ())
//BFS01     {
//BFS01         // neue Member
//BFS01         BOOL bTmp;
//BFS01         sal_Int32 nTmp;
//BFS01
//BFS01         // alte Version holt sich nur ein Polygon3D, wird hier durch
//BFS01         // Eintragen als erstes Teilpolygon geladen
//BFS01         // rIn >> aPolyPoly3D;
//BFS01         rIn >> aPolyPoly3D[0];
//BFS01
//BFS01         // #106240# OK, this file does have a saved polygon
//BFS01         bPolyWasRead = sal_True;
//BFS01
//BFS01         rIn >> nTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DHorizontalSegmentsItem(nTmp));
//BFS01
//BFS01         rIn >> nTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DEndAngleItem(nTmp));
//BFS01
//BFS01         rIn >> bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DDoubleSidedItem(bTmp));
//BFS01
//BFS01         rIn >> fLatheScale;
//BFS01     }
//BFS01
//BFS01     // #106240# No PolyPolygon as base for the lathe object was saved.
//BFS01     // Reconstruct it from the objects in the SubList.
//BFS01     if(!bPolyWasRead)
//BFS01     {
//BFS01         // This is really a old 3.0 or 3.1 file, reconstruct
//BFS01         // the not saved polygon using the SubList.
//BFS01         SdrObjList* pSubList = GetSubList();
//BFS01
//BFS01         if(pSubList && pSubList->GetObjCount())
//BFS01         {
//BFS01             sal_uInt16 nHorSegCount = (sal_uInt16)GetHorizontalSegments() / 2;
//BFS01             sal_uInt16 nVerSegCount = (sal_uInt16)(pSubList->GetObjCount() / nHorSegCount);
//BFS01             Polygon3D aNewBasePoly;
//BFS01
//BFS01             for(sal_uInt16 a(0); a < nVerSegCount; a++)
//BFS01             {
//BFS01                 E3dPolyObj* pCandidate = (E3dPolyObj*)pSubList->GetObj(a * nHorSegCount);
//BFS01                 if(pCandidate->ISA(E3dPolyObj))
//BFS01                 {
//BFS01                     const PolyPolygon3D& rCandPoly = ((E3dPolyObj*)pCandidate)->GetPolyPolygon3D();
//BFS01
//BFS01                     if(rCandPoly[0].GetPointCount() > 1)
//BFS01                     {
//BFS01                         aNewBasePoly[a] = rCandPoly[0][1];
//BFS01                     }
//BFS01                 }
//BFS01             }
//BFS01
//BFS01             aPolyPoly3D.Clear();
//BFS01             aNewBasePoly.SetClosed(sal_True);
//BFS01             aPolyPoly3D.Insert(aNewBasePoly);
//BFS01         }
//BFS01     }
//BFS01
//BFS01     if (aCompat.GetBytesLeft())
//BFS01     {
//BFS01         // Ab Version 364f (19.06.97)
//BFS01         sal_Int32 nTmp;
//BFS01         rIn >> nTmp;
//BFS01
//BFS01         // #83965# internally the real number of segments (edges) is
//BFS01         // used, no longer the number of points
//BFS01         if(!aPolyPoly3D[0].IsClosed())
//BFS01             nTmp -= 1;
//BFS01
//BFS01         GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nTmp));
//BFS01     }
//BFS01
//BFS01     if (aCompat.GetBytesLeft())
//BFS01     {
//BFS01         // Ab Version 374 (15.12.97)
//BFS01         // Gesamtes PolyPolygon laden
//BFS01         BOOL bTmp;
//BFS01         double fTmp;
//BFS01
//BFS01         aPolyPoly3D.Clear();
//BFS01         rIn >> aPolyPoly3D;
//BFS01
//BFS01         rIn >> fTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DBackscaleItem((sal_uInt16)(fTmp * 100.0)));
//BFS01
//BFS01         rIn >> fTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DPercentDiagonalItem(sal_uInt16(fTmp * 200.0)));
//BFS01
//BFS01         rIn >> bTmp; // #107245# bLatheSmoothed = bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DSmoothNormalsItem(bTmp));
//BFS01
//BFS01         rIn >> bTmp; // #107245# bLatheSmoothFrontBack = bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DSmoothLidsItem(bTmp));
//BFS01
//BFS01         rIn >> bTmp; // #107245# bLatheCharacterMode = bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DCharacterModeItem(bTmp));
//BFS01     }
//BFS01     else
//BFS01     {
//BFS01         // Geometrie aus erzeugten PolyObj's rekonstruieren
//BFS01         GetProperties().SetObjectItemDirect(Svx3DBackscaleItem(100));
//BFS01         GetProperties().SetObjectItemDirect(Svx3DPercentDiagonalItem(10));
//BFS01
//BFS01         // #107245# bLatheSmoothed = TRUE;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DSmoothNormalsItem(sal_True));
//BFS01
//BFS01         // #107245# bLatheSmoothFrontBack = FALSE;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DSmoothLidsItem(sal_False));
//BFS01
//BFS01         // #107245# bLatheCharacterMode = FALSE;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DCharacterModeItem(sal_False));
//BFS01     }
//BFS01
//BFS01     if (aCompat.GetBytesLeft())
//BFS01     {
//BFS01         // Ab Version 395 (8.6.98): Parameter aus dem Objekt
//BFS01         // E3dCompoundObject. Da irgendwann mal jemand die Ableitungs-
//BFS01         // hierarchie beim FileFormat unterbrochen hat, wurden diese Attribute
//BFS01         // bisher NOCH NIE gespeichert (Grrr). Diese Stelle muss nun natuerlich
//BFS01         // auch IMMER MITGEPFLEGT werden, wenn sich Parameter in
//BFS01         // E3dCompoundObject oder E3dObject aendern.
//BFS01         BOOL bTmp, bTmp2;
//BFS01         sal_uInt16 nTmp;
//BFS01
//BFS01         rIn >> bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DDoubleSidedItem(bTmp));
//BFS01
//BFS01         rIn >> bTmp; bCreateNormals = bTmp;
//BFS01         rIn >> bTmp; bCreateTexture = bTmp;
//BFS01
//BFS01         rIn >> bTmp;
//BFS01         rIn >> bTmp2;
//BFS01         if(bTmp == FALSE && bTmp2 == FALSE)
//BFS01             nTmp = 0;
//BFS01         else if(bTmp == TRUE && bTmp2 == FALSE)
//BFS01             nTmp = 1;
//BFS01         else
//BFS01             nTmp = 2;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DNormalsKindItem(nTmp));
//BFS01
//BFS01         rIn >> bTmp;
//BFS01         rIn >> bTmp2;
//BFS01         if(bTmp == FALSE && bTmp2 == FALSE)
//BFS01             nTmp = 0;
//BFS01         else if(bTmp == TRUE && bTmp2 == FALSE)
//BFS01             nTmp = 1;
//BFS01         else
//BFS01             nTmp = 2;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DTextureProjectionXItem(nTmp));
//BFS01
//BFS01         rIn >> bTmp;
//BFS01         rIn >> bTmp2;
//BFS01         if(bTmp == FALSE && bTmp2 == FALSE)
//BFS01             nTmp = 0;
//BFS01         else if(bTmp == TRUE && bTmp2 == FALSE)
//BFS01             nTmp = 1;
//BFS01         else
//BFS01             nTmp = 2;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DTextureProjectionYItem(nTmp));
//BFS01
//BFS01         rIn >> bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DShadow3DItem(bTmp));
//BFS01
//BFS01         Color aCol;
//BFS01
//BFS01         rIn >> aCol;
//BFS01         SetMaterialAmbientColor(aCol);
//BFS01
//BFS01         rIn >> aCol;
//BFS01         // do NOT use, this is the old 3D-Color(!)
//BFS01         // SetItem(XFillColorItem(String(), aCol));
//BFS01
//BFS01         rIn >> aCol;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DMaterialSpecularItem(aCol));
//BFS01
//BFS01         rIn >> aCol;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DMaterialEmissionItem(aCol));
//BFS01
//BFS01         rIn >> nTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DMaterialSpecularIntensityItem(nTmp));
//BFS01
//BFS01         aBackMaterial.ReadData(rIn);
//BFS01
//BFS01         rIn >> nTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DTextureKindItem(nTmp));
//BFS01
//BFS01         rIn >> nTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DTextureModeItem(nTmp));
//BFS01
//BFS01         rIn >> bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DNormalsInvertItem(bTmp));
//BFS01     }
//BFS01
//BFS01     if (aCompat.GetBytesLeft())
//BFS01     {
//BFS01         // Ab Version 513a (5.2.99): Parameter fuer das
//BFS01         // Erzeugen der Vorder/Rueckwand
//BFS01         BOOL bTmp;
//BFS01
//BFS01         rIn >> bTmp; // #107245# bLatheCloseFront = bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DCloseFrontItem(bTmp));
//BFS01
//BFS01         rIn >> bTmp; // #107245# bLatheCloseBack = bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DCloseBackItem(bTmp));
//BFS01     }
//BFS01     else
//BFS01     {
//BFS01         // #107245# bLatheCloseFront = TRUE;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DCloseFrontItem(sal_True));
//BFS01
//BFS01         // #107245# bLatheCloseBack = TRUE;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DCloseBackItem(sal_True));
//BFS01     }
//BFS01
//BFS01     // neu ab 534: (hat noch gefehlt)
//BFS01     if (aCompat.GetBytesLeft () >= sizeof (BOOL))
//BFS01     {
//BFS01         BOOL bTmp;
//BFS01         rIn >> bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DTextureFilterItem(bTmp));
//BFS01     }
//BFS01 }
//BFS01
//BFS01 // correct position of extrude polygon, in case it's not positioned
//BFS01 // at the Z==0 layer
//BFS01 if(aPolyPoly3D.Count() && aPolyPoly3D[0].GetPointCount())
//BFS01 {
//BFS01     const Vector3D& rFirstPoint = aPolyPoly3D[0][0];
//BFS01     if(rFirstPoint.Z() != 0.0)
//BFS01     {
//BFS01         // change transformation so that source poly lies in Z == 0,
//BFS01         // so it can be exported as 2D polygon
//BFS01         //
//BFS01         // ATTENTION: the translation has to be multiplied from LEFT
//BFS01         // SIDE since it was executed as the first translate for this
//BFS01         // 3D object during it's creation.
//BFS01         double fTransDepth(rFirstPoint.Z());
//BFS01         Matrix4D aTransMat;
//BFS01         aTransMat.TranslateZ(fTransDepth);
//BFS01         NbcSetTransform(GetTransform() * aTransMat); // #112587#
//BFS01
//BFS01         // correct polygon itself
//BFS01         aTransMat.Identity();
//BFS01         aTransMat.TranslateZ(-fTransDepth);
//BFS01         aPolyPoly3D.Transform(aTransMat);
//BFS01     }
//BFS01 }
//BFS01
//BFS01 // Geometrie neu erzeugen
//BFS01 ReCreateGeometry();
//BFS01}

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

void E3dLatheObj::ReSegment(long nHSegs, long nVSegs)
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

void E3dLatheObj::SetPolyPoly3D(const PolyPolygon3D& rNew)
{
    if(aPolyPoly3D != rNew)
    {
        aPolyPoly3D = rNew;

        // #83965# take care of vertical segments, too.
        sal_Int32 nNumVSegs = aPolyPoly3D[0].GetPointCount();
        if(!aPolyPoly3D[0].IsClosed())
            nNumVSegs -= 1;

        GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nNumVSegs));

        bGeometryValid = FALSE;
    }
}

void E3dLatheObj::SetLatheScale(double fNew)
{
    if(fLatheScale != fNew)
    {
        fLatheScale = fNew;
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
    PolyPolygon3D aTransPoly = TransformToScreenCoor(GetPolyPolygon());
    XPolyPolygon aPoly(aTransPoly.GetXPolyPolygon());
    SdrPathObj* pPathObj = new SdrPathObj(OBJ_PLIN, aPoly);

    if(pPathObj)
    {
        // set position ans size
        Rectangle aNewPosSize(aPoly.GetBoundRect());
        pPathObj->SetSnapRect(aNewPosSize);

        // Objekt ggf. schliessen
        BOOL bDistSmallerTen = FALSE;
        for(UINT16 nCnt=0;nCnt<pPathObj->GetPathPoly().Count();nCnt++)
        if(((XPolygon)(pPathObj->GetPathPoly()[0])).CalcDistance(0, pPathObj->GetPathPoly()[0].GetPointCount()-1) < 10)
        bDistSmallerTen = TRUE;
        if (!pPathObj->IsClosed() && bDistSmallerTen)
            pPathObj->ToggleClosed(0);

        // Attribute setzen
        SfxItemSet aSet(GetObjectItemSet());

        // Linien aktivieren, um Objekt garantiert sichtbar zu machen
        aSet.Put(XLineStyleItem (XLINE_SOLID));

        pPathObj->SetMergedItemSet(aSet);
    }

    return pPathObj;
}

// eof
