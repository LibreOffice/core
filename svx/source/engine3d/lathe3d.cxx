/*************************************************************************
 *
 *  $RCSfile: lathe3d.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2001-02-15 18:38:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "svdstr.hrc"
#include "svdglob.hxx"

#ifndef _SV_POLY_HXX
#include <vcl/poly.hxx>
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

#ifndef _SVDIO_HXX
#include "svdio.hxx"
#endif

#ifndef _E3D_POLYOB3D_HXX
#include "polyob3d.hxx"
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
    mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nSegCnt));

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
    mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nSegCnt));

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
    mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nSegCnt));

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
    mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nSegCnt));

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
    // Defaults setzen
    ImpForceItemSet();

    fLatheScale = rDefault.GetDefaultLatheScale();
    bLatheSmoothed = rDefault.GetDefaultLatheSmoothed();
    bLatheSmoothFrontBack = rDefault.GetDefaultLatheSmoothFrontBack();
    bLatheCharacterMode = rDefault.GetDefaultLatheCharacterMode();
    bLatheCloseFront = rDefault.GetDefaultLatheCloseFront();
    bLatheCloseBack = rDefault.GetDefaultLatheCloseBack();
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

    // Spezialfall Einzelnes Polygon erzeugen
    BOOL bSinglePoly = (GetEndAngle() == 0 || GetHorizontalSegments() == 0);
    if(bSinglePoly)
    {
        // nur ein Polygon erzeugen
        mpObjectItemSet->Put(Svx3DDoubleSidedItem(TRUE));

        // Fuer evtl. selbst erzeugte Normalen
        PolyPolygon3D aNormalsFront;

        // Normalen und Vorderseite selbst erzeugen
        AddFrontNormals(aLathePoly3D, aNormalsFront, aNormal);
        CreateFront(aLathePoly3D, aNormalsFront, GetCreateNormals(), GetCreateTexture());
    }
    else
    {
        // Eventuell doppelseitig erzeugen?
        if(!aLathePoly3D.IsClosed())
            mpObjectItemSet->Put(Svx3DDoubleSidedItem(TRUE));

        // Seiten genenrieren?
        BOOL bCreateSides = ((GetEndAngle() < 3600 && !GetDoubleSided())
            || (GetBackScale() != 100));

        // Polygone vorbereiten
        PolyPolygon3D aPrev, aFront, aBack, aNext;

        // Rotation vorbereiten
        double fAng = DEG2RAD(double(GetEndAngle()) / 10);
        Matrix4D aRotMat;

        // Skalierung vorbereiten
        double fScalePerStep;
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
            CreateSegment(
                aFront,
                aBack,
                &aPrev,
                &aNext,
                (a == 0) && bCreateSides && bLatheCloseFront,
                (a == nUpperBound-1) && bCreateSides && bLatheCloseBack,
                ((double)GetPercentDiagonal() / 200.0)
                    * (double(nUpperBound) / 6.0),
                GetLatheSmoothed(),
                GetLatheSmoothed(),
                GetLatheSmoothFrontBack(),
                1.0,
                fTmpStart,
                fTmpLength,
                GetCreateTexture(),
                GetCreateNormals(),
                GetLatheCharacterMode(),
                TRUE);

            // naechsten Schritt vorbereiten
            fTmpStart += fTmpLength;
            aPrev = aFront;
            aFront = aBack;
            aBack = aNext;
        }
    }

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
            mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nVSegs));

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

void E3dLatheObj::CreateWireframe(Polygon3D& rWirePoly, const Matrix4D* pTf,
    E3dDragDetail eDetail)
{
    // Nur selbst erzeugen, wenn alle Linien angezeigt werden sollen
    if ( eDetail == E3DDETAIL_ALLLINES ||
        (eDetail == E3DDETAIL_DEFAULT && GetDragDetail() == E3DDETAIL_ALLLINES) )
    {
        // Detailliert erzeugen
    }
    else
    {
        // call parent
        E3dObject::CreateWireframe(rWirePoly, pTf, eDetail);
    }
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

    aPolyPoly3D  = r3DObj.aPolyPoly3D;
    fLatheScale  = r3DObj.fLatheScale;

    // Ab Version 374 (15.12.97)
    bLatheSmoothed = r3DObj.bLatheSmoothed;
    bLatheSmoothFrontBack = r3DObj.bLatheSmoothFrontBack;
    bLatheCharacterMode = r3DObj.bLatheCharacterMode;
    bLatheCloseFront = r3DObj.bLatheCloseFront;
    bLatheCloseBack = r3DObj.bLatheCloseBack;
}

/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

void E3dLatheObj::WriteData(SvStream& rOut) const
{
    long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
    if(nVersion < 3800)
    {
        // Alte Geometrie erzeugen, um die E3dPolyObj's zu haben
        ((E3dCompoundObject*)this)->ReCreateGeometry(TRUE);
    }

    // leider kann das E3dLatheObj nicht auf E3dObject abgestuetzt werden,
    // da neue Member hinzugekommen sind und die Kompatibilitaet erhalten
    // bleiben muss.
    SdrAttrObj::WriteData(rOut);

    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rOut, STREAM_WRITE);
#ifdef DBG_UTIL
    aCompat.SetID("E3dLatheObj");
#endif

    pSub->Save(rOut);

    // Parameter aus E3dObject speichern
    rOut << aLocalBoundVol;
    Old_Matrix3D aMat3D;
    aMat3D = aTfMatrix;
    rOut << aMat3D;
    rOut << nLogicalGroup;
    rOut << nObjTreeLevel;
    rOut << nPartOfParent;
    rOut << UINT16(eDragDetail);

    // neue Member
    // Alte version schreibt Polygon3D raus, neue Version
    // benutzt dafuer das erste Teilpolygon von PolyPolygon3D
    // rOut << aPolyPoly3D;
    rOut << aPolyPoly3D[0];

    rOut << GetHorizontalSegments();

    rOut << GetEndAngle();

    rOut << ((E3dLatheObj*)this)->GetDoubleSided();
    rOut << fLatheScale;

    // Ab Version 364f (19.06.97)

    // #83965# internally the real number of segments (edges) is
    // used, no longer the number of points
    sal_Int32 nVSegs = GetVerticalSegments();
    if(!aPolyPoly3D[0].IsClosed())
        nVSegs += 1;

    rOut << nVSegs;

    // Ab Version 374 (15.12.97)
    rOut << aPolyPoly3D;

    rOut << ((double)GetBackScale() / 100.0);

    rOut << ((double)GetPercentDiagonal() / 200.0);

    rOut << (BOOL)bLatheSmoothed;
    rOut << (BOOL)bLatheSmoothFrontBack;
    rOut << (BOOL)bLatheCharacterMode;

    // Ab Version 395 (8.6.98): Parameter aus dem Objekt
    // E3dCompoundObject. Da irgendwann mal jemand die Ableitungs-
    // hierarchie beim FileFormat unterbrochen hat, wurden diese Attribute
    // bisher NOCH NIE gespeichert (Grrr). Diese Stelle muss nun natuerlich
    // auch IMMER MITGEPFLEGT werden, wenn sich Parameter in
    // E3dCompoundObject oder E3dObject aendern.
    rOut << GetDoubleSided();

    rOut << BOOL(bCreateNormals);
    rOut << BOOL(bCreateTexture);

    sal_uInt16 nVal = GetNormalsKind();
    rOut << BOOL(nVal > 0);
    rOut << BOOL(nVal > 1);

    nVal = GetTextureProjectionX();
    rOut << BOOL(nVal > 0);
    rOut << BOOL(nVal > 1);

    nVal = GetTextureProjectionY();
    rOut << BOOL(nVal > 0);
    rOut << BOOL(nVal > 1);

    rOut << BOOL(GetShadow3D());

    rOut << GetMaterialAmbientColor();
    rOut << GetMaterialColor();
    rOut << GetMaterialSpecular();
    rOut << GetMaterialEmission();
    rOut << GetMaterialSpecularIntensity();

    aBackMaterial.WriteData(rOut);

    rOut << (UINT16)GetTextureKind();

    rOut << (UINT16)GetTextureMode();

    rOut << BOOL(GetNormalsInvert());

    // Ab Version 513a (5.2.99): Parameter fuer das
    // Erzeugen der Vorder/Rueckwand
    rOut << BOOL(bLatheCloseFront);
    rOut << BOOL(bLatheCloseBack);

    // neu ab 534: (hat noch gefehlt)
    rOut << BOOL(GetTextureFilter());

    if(nVersion < 3800)
    {
        // Geometrie neu erzeugen, um E3dPolyObj's wieder loszuwerden
        ((E3dCompoundObject*)this)->ReCreateGeometry();
    }
}

/*************************************************************************
|*
|* Objektdaten aus Stream laden
|*
\************************************************************************/

void E3dLatheObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if (ImpCheckSubRecords (rHead, rIn))
    {
        // leider kann das E3dLatheObj nicht auf E3dObject abgestuetzt werden,
        // da neue Member hinzugekommen sind und die Kompatibilitaet erhalten
        // bleiben muss.
        SdrAttrObj::ReadData(rHead, rIn);

        // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
        SdrDownCompat aCompat(rIn, STREAM_READ);
#ifdef DBG_UTIL
        aCompat.SetID("E3dLatheObj");
#endif
        // dann die Member
        UINT16  nTmp16;

        pSub->Load(rIn, *pPage);

        // Parameter aus E3dObject laden
        rIn >> aLocalBoundVol;
        Old_Matrix3D aMat3D;
        rIn >> aMat3D;
        aTfMatrix = Matrix4D(aMat3D);
        rIn >> nLogicalGroup;
        rIn >> nObjTreeLevel;
        rIn >> nPartOfParent;
        rIn >> nTmp16; eDragDetail = E3dDragDetail(nTmp16);

        // BoundVolume muss neu berechnet werden
        bBoundVolValid = FALSE;

        if (aCompat.GetBytesLeft ())
        {
            // neue Member
            BOOL bTmp;
            sal_Int32 nTmp;

            // alte Version holt sich nur ein Polygon3D, wird hier durch
            // Eintragen als erstes Teilpolygon geladen
            // rIn >> aPolyPoly3D;
            rIn >> aPolyPoly3D[0];

            rIn >> nTmp;
            mpObjectItemSet->Put(Svx3DHorizontalSegmentsItem(nTmp));

            rIn >> nTmp;
            mpObjectItemSet->Put(Svx3DEndAngleItem(nTmp));

            rIn >> bTmp;
            mpObjectItemSet->Put(Svx3DDoubleSidedItem(bTmp));

            rIn >> fLatheScale;
        }

        if (aCompat.GetBytesLeft())
        {
            // Ab Version 364f (19.06.97)
            sal_Int32 nTmp;
            rIn >> nTmp;

            // #83965# internally the real number of segments (edges) is
            // used, no longer the number of points
            if(!aPolyPoly3D[0].IsClosed())
                nTmp -= 1;

            mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nTmp));
        }

        if (aCompat.GetBytesLeft())
        {
            // Ab Version 374 (15.12.97)
            // Gesamtes PolyPolygon laden
            BOOL bTmp;
            double fTmp;

            aPolyPoly3D.Clear();
            rIn >> aPolyPoly3D;

            rIn >> fTmp;
            mpObjectItemSet->Put(Svx3DBackscaleItem((sal_uInt16)(fTmp * 100.0)));

            rIn >> fTmp;
            mpObjectItemSet->Put(Svx3DPercentDiagonalItem(sal_uInt16(fTmp * 200.0)));

            rIn >> bTmp; bLatheSmoothed = bTmp;
            rIn >> bTmp; bLatheSmoothFrontBack = bTmp;
            rIn >> bTmp; bLatheCharacterMode = bTmp;
        }
        else
        {
            // Geometrie aus erzeugten PolyObj's rekonstruieren
            mpObjectItemSet->Put(Svx3DBackscaleItem(100));

            mpObjectItemSet->Put(Svx3DPercentDiagonalItem(10));

            bLatheSmoothed = TRUE;
            bLatheSmoothFrontBack = FALSE;
            bLatheCharacterMode = FALSE;
        }

        if (aCompat.GetBytesLeft())
        {
            // Ab Version 395 (8.6.98): Parameter aus dem Objekt
            // E3dCompoundObject. Da irgendwann mal jemand die Ableitungs-
            // hierarchie beim FileFormat unterbrochen hat, wurden diese Attribute
            // bisher NOCH NIE gespeichert (Grrr). Diese Stelle muss nun natuerlich
            // auch IMMER MITGEPFLEGT werden, wenn sich Parameter in
            // E3dCompoundObject oder E3dObject aendern.
            BOOL bTmp, bTmp2;
            sal_uInt16 nTmp;

            rIn >> bTmp;
            mpObjectItemSet->Put(Svx3DDoubleSidedItem(bTmp));

            rIn >> bTmp; bCreateNormals = bTmp;
            rIn >> bTmp; bCreateTexture = bTmp;

            rIn >> bTmp;
            rIn >> bTmp2;
            if(bTmp == FALSE && bTmp2 == FALSE)
                nTmp = 0;
            else if(bTmp == TRUE && bTmp2 == FALSE)
                nTmp = 1;
            else
                nTmp = 2;
            mpObjectItemSet->Put(Svx3DNormalsKindItem(nTmp));

            rIn >> bTmp;
            rIn >> bTmp2;
            if(bTmp == FALSE && bTmp2 == FALSE)
                nTmp = 0;
            else if(bTmp == TRUE && bTmp2 == FALSE)
                nTmp = 1;
            else
                nTmp = 2;
            mpObjectItemSet->Put(Svx3DTextureProjectionXItem(nTmp));

            rIn >> bTmp;
            rIn >> bTmp2;
            if(bTmp == FALSE && bTmp2 == FALSE)
                nTmp = 0;
            else if(bTmp == TRUE && bTmp2 == FALSE)
                nTmp = 1;
            else
                nTmp = 2;
            mpObjectItemSet->Put(Svx3DTextureProjectionYItem(nTmp));

            rIn >> bTmp;
            mpObjectItemSet->Put(Svx3DShadow3DItem(bTmp));

            Color aCol;

            rIn >> aCol;
            SetMaterialAmbientColor(aCol);

            rIn >> aCol;
            // do NOT use, this is the old 3D-Color(!)
            // SetItem(XFillColorItem(String(), aCol));

            rIn >> aCol;
            mpObjectItemSet->Put(Svx3DMaterialSpecularItem(aCol));

            rIn >> aCol;
            mpObjectItemSet->Put(Svx3DMaterialEmissionItem(aCol));

            rIn >> nTmp;
            mpObjectItemSet->Put(Svx3DMaterialSpecularIntensityItem(nTmp));

            aBackMaterial.ReadData(rIn);

            rIn >> nTmp;
            mpObjectItemSet->Put(Svx3DTextureKindItem(nTmp));

            rIn >> nTmp;
            mpObjectItemSet->Put(Svx3DTextureModeItem(nTmp));

            rIn >> bTmp;
            mpObjectItemSet->Put(Svx3DNormalsInvertItem(bTmp));
        }

        if (aCompat.GetBytesLeft())
        {
            // Ab Version 513a (5.2.99): Parameter fuer das
            // Erzeugen der Vorder/Rueckwand
            BOOL bTmp;

            rIn >> bTmp; bLatheCloseFront = bTmp;
            rIn >> bTmp; bLatheCloseBack = bTmp;
        }
        else
        {
            bLatheCloseFront = TRUE;
            bLatheCloseBack = TRUE;
        }

        // neu ab 534: (hat noch gefehlt)
        if (aCompat.GetBytesLeft () >= sizeof (BOOL))
        {
            BOOL bTmp;
            rIn >> bTmp;
            mpObjectItemSet->Put(Svx3DTextureFilterItem(bTmp));
        }
    }

    // correct position of extrude polygon, in case it's not positioned
    // at the Z==0 layer
    if(aPolyPoly3D.Count() && aPolyPoly3D[0].GetPointCount())
    {
        const Vector3D& rFirstPoint = aPolyPoly3D[0][0];
        if(rFirstPoint.Z() != 0.0)
        {
            // change transformation so that source poly lies in Z == 0,
            // so it can be exported as 2D polygon
            //
            // ATTENTION: the translation has to be multiplied from LEFT
            // SIDE since it was executed as the first translate for this
            // 3D object during it's creation.
            double fTransDepth(rFirstPoint.Z());
            Matrix4D aTransMat;
            aTransMat.TranslateZ(fTransDepth);
            NbcSetTransform(aTransMat * GetTransform());

            // correct polygon itself
            aTransMat.Identity();
            aTransMat.TranslateZ(-fTransDepth);
            aPolyPoly3D.Transform(aTransMat);
        }
    }

    // Geometrie neu erzeugen
    ReCreateGeometry();
}

/*************************************************************************
|*
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus n Polygonen
|*
\************************************************************************/

SdrObject *E3dLatheObj::DoConvertToPolyObj(BOOL bBezier) const
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
        mpObjectItemSet->Put(Svx3DHorizontalSegmentsItem(nHSegs));

        mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nVSegs));

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
        ImpForceItemSet();
        mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nNumVSegs));

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

void E3dLatheObj::SetLatheSmoothed(BOOL bNew)
{
    if(bLatheSmoothed != bNew)
    {
        bLatheSmoothed = bNew;
        bGeometryValid = FALSE;
    }
}

void E3dLatheObj::SetLatheSmoothFrontBack(BOOL bNew)
{
    if(bLatheSmoothFrontBack != bNew)
    {
        bLatheSmoothFrontBack = bNew;
        bGeometryValid = FALSE;
    }
}

void E3dLatheObj::SetLatheCharacterMode(BOOL bNew)
{
    if(bLatheCharacterMode != bNew)
    {
        bLatheCharacterMode = bNew;
        bGeometryValid = FALSE;
    }
}

void E3dLatheObj::SetLatheCloseFront(BOOL bNew)
{
    if(bLatheCloseFront != bNew)
    {
        bLatheCloseFront = bNew;
        bGeometryValid = FALSE;
    }
}

void E3dLatheObj::SetLatheCloseBack(BOOL bNew)
{
    if(bLatheCloseBack != bNew)
    {
        bLatheCloseBack = bNew;
        bGeometryValid = FALSE;
    }
}

//////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access

void E3dLatheObj::PostItemChange(const sal_uInt16 nWhich)
{
    // call parent
    E3dCompoundObject::PostItemChange(nWhich);

    switch(nWhich)
    {
        case SDRATTR_3DOBJ_HORZ_SEGS:
        {
            bGeometryValid = FALSE;
            break;
        }
        case SDRATTR_3DOBJ_VERT_SEGS:
        {
            bGeometryValid = FALSE;
            break;
        }
        case SDRATTR_3DOBJ_PERCENT_DIAGONAL:
        {
            bGeometryValid = FALSE;
            break;
        }
        case SDRATTR_3DOBJ_BACKSCALE:
        {
            bGeometryValid = FALSE;
            break;
        }
        case SDRATTR_3DOBJ_END_ANGLE:
        {
            bGeometryValid = FALSE;
            break;
        }
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
        SfxItemSet aSet(GetItemSet());

        // Linien aktivieren, um Objekt garantiert sichtbar zu machen
        aSet.Put(XLineStyleItem (XLINE_SOLID));

        pPathObj->SetItemSet(aSet);
    }

    return pPathObj;
}

// EOF
