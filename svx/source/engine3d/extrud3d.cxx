/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: extrud3d.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:55:59 $
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

#ifndef _POLY3D_HXX
#include "poly3d.hxx"
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

E3dExtrudeObj::E3dExtrudeObj(E3dDefaultAttributes& rDefault, const PolyPolygon& rPP, double fDepth)
:   E3dCompoundObject(rDefault),
    aExtrudePolygon(rPP, rDefault.GetDefaultExtrudeScale())
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // set extrude depth
    GetProperties().SetObjectItemDirect(Svx3DDepthItem((sal_uInt32)(fDepth + 0.5)));

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

E3dExtrudeObj::E3dExtrudeObj(E3dDefaultAttributes& rDefault, const XPolyPolygon& rXPP, double fDepth)
:   E3dCompoundObject(rDefault),
    aExtrudePolygon(rXPP, rDefault.GetDefaultExtrudeScale())
{
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
    fExtrudeScale = rDefault.GetDefaultExtrudeScale();

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

PolyPolygon3D E3dExtrudeObj::GetFrontSide()
{
    // Polygon als Grundlage holen
    PolyPolygon3D aPolyPoly3D(aExtrudePolygon);

    // Ueberfluessige Punkte entfernen, insbesondere doppelte
    // Start- und Endpunkte verhindern
    aPolyPoly3D.RemoveDoublePoints();

    // Normale holen
    Vector3D aNormal = aPolyPoly3D.GetNormal();

    if( (aNormal.Z() > 0.0) != (GetExtrudeDepth() != 0) )
        aPolyPoly3D.FlipDirections();

    // Orientierung evtl. vorhandener Loecher in einen definierten
    // Ausgangszustand bringen
    aPolyPoly3D.SetDirections();

    return aPolyPoly3D;
}

PolyPolygon3D E3dExtrudeObj::GetBackSide(const PolyPolygon3D& rFrontSide)
{
    PolyPolygon3D aBackSide(rFrontSide);

    if(GetExtrudeDepth() != 0)
    {
        // Extrudevektor bilden
        Vector3D aNormal = aBackSide.GetNormal();
        if(aNormal.Z() < 0.0)
            aNormal.Z() = -aNormal.Z();
        Vector3D aOffset = aNormal * (double)GetExtrudeDepth();

        // eventuell Skalieren
        if(GetPercentBackScale() != 100)
            ScalePoly(aBackSide, (double)GetPercentBackScale() / 100.0);

        // Verschieben
        Matrix4D aTrans;
        aTrans.Translate(aOffset);
        aBackSide.Transform(aTrans);
    }

    return aBackSide;
}

/*************************************************************************
|*
|* Give out simple line geometry
|*
\************************************************************************/

::basegfx::B3DPolyPolygon E3dExtrudeObj::Get3DLineGeometry() const
{
    ::basegfx::B3DPolyPolygon aRetval;

    aRetval.append(maLinePolyPolygon.getB3DPolyPolygon());

    return aRetval;
}

void E3dExtrudeObj::CreateGeometry()
{
    // Start der Geometrieerzeugung ankuendigen
    StartCreateGeometry();

    // #78972# prepare new line geometry creation
    maLinePolyPolygon.Clear();

    // Polygon als Grundlage holen
    PolyPolygon3D aFrontSide = GetFrontSide();

    if(GetExtrudeDepth() != 0)
    {
        // Hinteres Polygon erzeugen
        PolyPolygon3D aBackSide = GetBackSide(aFrontSide);

        // Was muss erzeugt werden?
        if(!aFrontSide.IsClosed())
        {
            GetProperties().SetObjectItemDirect(Svx3DDoubleSidedItem(TRUE));
        }

        double fTextureDepth=1.0;
        double fTextureStart=0.0;

        // Texturen erzeugen?
        if(!GetCreateTexture())
            fTextureStart = fTextureDepth = 0.0;

        // Falls Texturen erzeugen Randbreite fuer diese bestimmen
        double fSurroundFactor = 1.0;
        if(GetCreateTexture())
        {
            fSurroundFactor = aFrontSide.GetLength() / sqrt(aFrontSide.GetPolyArea());
            fSurroundFactor = (double)((long)(fSurroundFactor - 0.5));
            if(fSurroundFactor == 0.0)
                fSurroundFactor = 1.0;
        }

        // #i28528#
        PolyPolygon3D aFrontLines;
        PolyPolygon3D aBackLines;
        PolyPolygon3D aInBetweenLines;

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
        maLinePolyPolygon.Insert(aFrontLines);
        maLinePolyPolygon.Insert(aInBetweenLines);
        maLinePolyPolygon.Insert(aBackLines);
    }
    else
    {
        // nur ein Polygon erzeugen
        GetProperties().SetObjectItemDirect(Svx3DDoubleSidedItem(TRUE));

        // Fuer evtl. selbst erzeugte Normalen
        PolyPolygon3D aNormalsFront;

        // Extrudevektor bilden
        Vector3D aNormal = aFrontSide.GetNormal();
        Vector3D aOffset = aNormal * (double)GetExtrudeDepth();

        // Normalen und Vorderseite selbst erzeugen
        AddFrontNormals(aFrontSide, aNormalsFront, aOffset);
        CreateFront(aFrontSide, aNormalsFront, GetCreateNormals(), GetCreateTexture());

        // #78972#
        maLinePolyPolygon.Insert(aFrontSide);
    }

    // #i28528#
    if(!GetReducedLineGeometry())
    {
        PolyPolygon3D aNewPolyPoly = ImpCompleteLinePolygon(maLinePolyPolygon, aFrontSide.Count(), sal_False);
        // append horizontal lines
        maLinePolyPolygon.Insert(aNewPolyPoly);
    }

    ImpCorrectLinePolygon(maLinePolyPolygon, aFrontSide.Count());

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
|* Wireframe erzeugen
|*
\************************************************************************/

//BFS01void E3dExtrudeObj::CreateWireframe(Polygon3D& rWirePoly, const Matrix4D* pTf, E3dDragDetail eDetail)
//BFS01{
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
|* Objektdaten in Stream speichern
|*
\************************************************************************/

//BFS01void E3dExtrudeObj::WriteData(SvStream& rOut) const
//BFS01{
//BFS01#ifndef SVX_LIGHT
//BFS01 long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
//BFS01 if(nVersion < 3800)
//BFS01 {
//BFS01     // Alte Geometrie erzeugen, um die E3dPolyObj's zu haben
//BFS01     ((E3dCompoundObject*)this)->ReCreateGeometry(TRUE);
//BFS01 }
//BFS01
//BFS01 // call parent
//BFS01 E3dCompoundObject::WriteData(rOut);
//BFS01
//BFS01 E3dIOCompat aCompat(rOut, STREAM_WRITE, 1);
//BFS01 rOut << aExtrudePolygon;
//BFS01 rOut << fExtrudeScale;
//BFS01
//BFS01 rOut << (double)GetExtrudeDepth();
//BFS01
//BFS01 rOut << (double)GetPercentBackScale() / 100.0;
//BFS01
//BFS01 rOut << (double)GetPercentDiagonal() / 200.0;
//BFS01
//BFS01 rOut << GetSmoothNormals(); // #107245# (BOOL)bExtrudeSmoothed;
//BFS01 rOut << GetSmoothLids(); // #107245# (BOOL)bExtrudeSmoothFrontBack;
//BFS01 rOut << GetCharacterMode(); // #107245# (BOOL)bExtrudeCharacterMode;
//BFS01
//BFS01 // Ab Version 513a (5.2.99): Parameter fuer das
//BFS01 // Erzeugen der Vorder/Rueckwand
//BFS01 rOut << GetCloseFront(); // #107245# (BOOL)bExtrudeCloseFront;
//BFS01 rOut << GetCloseBack(); // #107245# (BOOL)bExtrudeCloseBack;
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

//BFS01void E3dExtrudeObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
//BFS01{
//BFS01 // call parent
//BFS01 E3dCompoundObject::ReadData(rHead, rIn);
//BFS01
//BFS01 // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01 BOOL bAllDone(FALSE);
//BFS01
//BFS01 if(AreBytesLeft())
//BFS01 {
//BFS01     E3dIOCompat aIoCompat(rIn, STREAM_READ);
//BFS01     if(aIoCompat.GetVersion() >= 1)
//BFS01     {
//BFS01         BOOL bTmp;
//BFS01         double fTmp;
//BFS01
//BFS01         rIn >> aExtrudePolygon;
//BFS01         rIn >> fExtrudeScale;
//BFS01
//BFS01         rIn >> fTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DDepthItem(sal_uInt32(fTmp + 0.5)));
//BFS01
//BFS01         rIn >> fTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DBackscaleItem(sal_uInt16(fTmp * 100.0)));
//BFS01
//BFS01         rIn >> fTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DPercentDiagonalItem(sal_uInt16(fTmp * 200.0)));
//BFS01
//BFS01         rIn >> bTmp; // #107245# bExtrudeSmoothed = bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DSmoothNormalsItem(bTmp));
//BFS01
//BFS01         rIn >> bTmp; // #107245# bExtrudeSmoothFrontBack = bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DSmoothLidsItem(bTmp));
//BFS01
//BFS01         rIn >> bTmp; // #107245# bExtrudeCharacterMode = bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DCharacterModeItem(bTmp));
//BFS01
//BFS01         bAllDone = TRUE;
//BFS01
//BFS01         if(aIoCompat.GetBytesLeft())
//BFS01         {
//BFS01             // Ab Version 513a (5.2.99): Parameter fuer das
//BFS01             // Erzeugen der Vorder/Rueckwand
//BFS01             BOOL bTmp;
//BFS01
//BFS01             rIn >> bTmp; // #107245# bExtrudeCloseFront = bTmp;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DCloseFrontItem(bTmp));
//BFS01
//BFS01             rIn >> bTmp; // #107245# bExtrudeCloseBack = bTmp;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DCloseBackItem(bTmp));
//BFS01         }
//BFS01         else
//BFS01         {
//BFS01             // #107245# bExtrudeCloseFront = TRUE;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DCloseFrontItem(sal_True));
//BFS01
//BFS01             // #107245# bExtrudeCloseBack = TRUE;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DCloseBackItem(sal_True));
//BFS01         }
//BFS01     }
//BFS01 }
//BFS01
//BFS01 if(!bAllDone)
//BFS01 {
//BFS01     // Geometrie aus geladenen PolyObj's rekonstruieren
//BFS01     SdrObjList* pSubList = GetSubList();
//BFS01     if(pSubList && pSubList->GetObjCount())
//BFS01     {
//BFS01         // Vorderseite und Rueckseite sind die ersten
//BFS01         // PolyObj's in der Liste, hole diese
//BFS01         E3dPolyObj* pFront = NULL;
//BFS01         E3dPolyObj* pBack = NULL;
//BFS01         E3dPolyObj* pOther = NULL;
//BFS01
//BFS01         UINT16 a;
//BFS01         for(a=0;a<pSubList->GetObjCount();a++)
//BFS01         {
//BFS01             E3dPolyObj* pCandidate = (E3dPolyObj*)pSubList->GetObj(a);
//BFS01             if(pCandidate->ISA(E3dPolyObj))
//BFS01             {
//BFS01                 // Die Nromalen der Vorder/Rueckseiten zeigen in Z-Richtung,
//BFS01                 // nutze dies aus
//BFS01                 const Vector3D& rNormal = pCandidate->GetNormal();
//BFS01                 if(fabs(rNormal.X()) < 0.0000001 && fabs(rNormal.Y()) < 0.0000001)
//BFS01                 {
//BFS01                     if(rNormal.Z() > 0.0)
//BFS01                     {
//BFS01                         // Vorderseite
//BFS01                         pFront = pCandidate;
//BFS01                     }
//BFS01                     else
//BFS01                     {
//BFS01                         // Rueckseite
//BFS01                         pBack = pCandidate;
//BFS01                     }
//BFS01                 }
//BFS01                 else
//BFS01                 {
//BFS01                     if(!pOther)
//BFS01                         pOther = pCandidate;
//BFS01                 }
//BFS01             }
//BFS01         }
//BFS01
//BFS01         // Extrude-Tiefe feststellen
//BFS01         if(pOther)
//BFS01         {
//BFS01             const PolyPolygon3D& rOtherPoly = pOther->GetPolyPolygon3D();
//BFS01             // Hintereinanderliegende Paare in der alten Version waren
//BFS01             // 0,1 und 3,2 (0,3 vorne)
//BFS01             double fVal = (rOtherPoly[0][1] - rOtherPoly[0][0]).GetLength();
//BFS01             GetProperties().SetObjectItemDirect(Svx3DDepthItem(sal_uInt32(fVal + 0.5)));
//BFS01         }
//BFS01         else
//BFS01         {
//BFS01             // Einen Default vorsehen, kann aber eigentlich nie geschehen
//BFS01             GetProperties().SetObjectItemDirect(Svx3DDepthItem(100));
//BFS01         }
//BFS01
//BFS01         // Polygon fuer Vorderseite holen
//BFS01         if(pFront)
//BFS01         {
//BFS01             aExtrudePolygon = pFront->GetPolyPolygon3D();
//BFS01         }
//BFS01         else
//BFS01         {
//BFS01             if(pBack)
//BFS01             {
//BFS01                 // Rueckseite benutzen und um -fExtrudeDepth in Z
//BFS01                 // verschieben
//BFS01                 aExtrudePolygon = pBack->GetPolyPolygon3D();
//BFS01                 Matrix4D aMat;
//BFS01                 aMat.Translate(Vector3D(0.0, 0.0, -(double)GetExtrudeDepth()));
//BFS01                 aExtrudePolygon.Transform(aMat);
//BFS01             }
//BFS01             else
//BFS01             {
//BFS01                 // Die Polygondaten koennen aus den Vorderkanten
//BFS01                 // der weiteren Polygone (Punkte 0,3) restauriert werden.
//BFS01                 // evtl. spaeter ergaenzen
//BFS01                 aExtrudePolygon.Clear();
//BFS01             }
//BFS01         }
//BFS01
//BFS01         // Bestimmen, ob die Teilpolygone von aExtrudePolygon
//BFS01         // geschlossen waren. Sie waren geschlossen, wenn ein
//BFS01         // entsprechendes PolyObj existiert
//BFS01         for(a=0;a<aExtrudePolygon.Count();a++)
//BFS01         {
//BFS01             Polygon3D &rPoly = aExtrudePolygon[a];
//BFS01             USHORT nCnt = rPoly.GetPointCount();
//BFS01
//BFS01             if(nCnt)
//BFS01             {
//BFS01                 Vector3D& rFirst = rPoly[0];
//BFS01                 Vector3D& rLast = rPoly[nCnt - 1];
//BFS01                 BOOL bClosePoly(FALSE);
//BFS01
//BFS01                 for(UINT16 b=0;b<pSubList->GetObjCount();b++)
//BFS01                 {
//BFS01                     E3dPolyObj* pCandidate = (E3dPolyObj*)pSubList->GetObj(b);
//BFS01                     if(pCandidate->ISA(E3dPolyObj)
//BFS01                         && pCandidate != pFront && pCandidate != pBack)
//BFS01                     {
//BFS01                         const PolyPolygon3D& rCandPoly = pCandidate->GetPolyPolygon3D();
//BFS01                         if(rCandPoly[0].GetPointCount() > 2)
//BFS01                         {
//BFS01                             if(rCandPoly[0][0] == rFirst && rCandPoly[0][3] == rLast)
//BFS01                                 bClosePoly = TRUE;
//BFS01                             if(rCandPoly[0][3] == rFirst && rCandPoly[0][0] == rLast)
//BFS01                                 bClosePoly = TRUE;
//BFS01                         }
//BFS01                     }
//BFS01                 }
//BFS01
//BFS01                 rPoly.SetClosed(bClosePoly);
//BFS01             }
//BFS01         }
//BFS01
//BFS01         // Setze die weiteren Parameter auf die defaults
//BFS01         fExtrudeScale = 1.0;
//BFS01
//BFS01         GetProperties().SetObjectItemDirect(Svx3DBackscaleItem(100));
//BFS01         GetProperties().SetObjectItemDirect(Svx3DPercentDiagonalItem(10));
//BFS01
//BFS01         // #107245# bExtrudeSmoothed = TRUE;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DSmoothNormalsItem(sal_True));
//BFS01
//BFS01         // #107245# bExtrudeSmoothFrontBack = FALSE;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DSmoothLidsItem(sal_False));
//BFS01
//BFS01         // #107245# bExtrudeCharacterMode = FALSE;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DCharacterModeItem(sal_False));
//BFS01     }
//BFS01 }
//BFS01
//BFS01 // correct position of extrude polygon, in case it's not positioned
//BFS01 // at the Z==0 layer
//BFS01 if(aExtrudePolygon.Count() && aExtrudePolygon[0].GetPointCount())
//BFS01 {
//BFS01     const Vector3D& rFirstPoint = aExtrudePolygon[0][0];
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
//BFS01         aExtrudePolygon.Transform(aTransMat);
//BFS01     }
//BFS01 }
//BFS01
//BFS01 // Geometrie neu erzeugen
//BFS01 ReCreateGeometry();
//BFS01}

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

    aExtrudePolygon = r3DObj.aExtrudePolygon;
    fExtrudeScale = r3DObj.fExtrudeScale;

    // #95519# copy LinePolygon info, too
    maLinePolyPolygon = r3DObj.maLinePolyPolygon;

    // #107245# These properties are now items and are copied with the ItemSet
    // bExtrudeSmoothed = r3DObj.bExtrudeSmoothed;
    // bExtrudeSmoothFrontBack = r3DObj.bExtrudeSmoothFrontBack;
    // bExtrudeCharacterMode = r3DObj.bExtrudeCharacterMode;
    // bExtrudeCloseFront = r3DObj.bExtrudeCloseFront;
    // bExtrudeCloseBack = r3DObj.bExtrudeCloseBack;
}

/*************************************************************************
|*
|* Lokale Parameter setzen mit Geometrieneuerzeugung
|*
\************************************************************************/

void E3dExtrudeObj::SetExtrudePolygon(const PolyPolygon3D &rNew)
{
    if(aExtrudePolygon != rNew)
    {
        aExtrudePolygon = rNew;
        bGeometryValid = FALSE;
    }
}

void E3dExtrudeObj::SetExtrudeScale(double fNew)
{
    if(fExtrudeScale != fNew)
    {
        fExtrudeScale = fNew;
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
    XPolyPolygon aPoly = TransformToScreenCoor(GetBackSide(GetFrontSide()));
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
