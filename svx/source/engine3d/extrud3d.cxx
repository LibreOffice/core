/*************************************************************************
 *
 *  $RCSfile: extrud3d.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-27 13:25:53 $
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

#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _E3D_POLYOB3D_HXX
#include "polyob3d.hxx"
#endif

#ifndef _E3D_EXTRUD3D_HXX
#include "extrud3d.hxx"
#endif

#ifndef _E3D_E3DIOCMPT_HXX
#include "e3dcmpt.hxx"
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
    mpObjectItemSet->Put(Svx3DDepthItem((sal_uInt32)(fDepth + 0.5)));

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
    mpObjectItemSet->Put(Svx3DDepthItem((sal_uInt32)(fDepth + 0.5)));

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
    // Defaults setzen
    ImpForceItemSet();

    fExtrudeScale = rDefault.GetDefaultExtrudeScale();

    // #107245#
    mpObjectItemSet->Put(Svx3DSmoothNormalsItem(rDefault.GetDefaultExtrudeSmoothed()));
    mpObjectItemSet->Put(Svx3DSmoothLidsItem(rDefault.GetDefaultExtrudeSmoothFrontBack()));
    mpObjectItemSet->Put(Svx3DCharacterModeItem(rDefault.GetDefaultExtrudeCharacterMode()));
    mpObjectItemSet->Put(Svx3DCloseFrontItem(rDefault.GetDefaultExtrudeCloseFront()));
    mpObjectItemSet->Put(Svx3DCloseBackItem(rDefault.GetDefaultExtrudeCloseBack()));

    // Bei extrudes defaultmaessig StdTexture in X und Y
    mpObjectItemSet->Put(Svx3DTextureProjectionXItem(1));
    mpObjectItemSet->Put(Svx3DTextureProjectionYItem(1));
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
    if((aNormal.Z() > 0.0) != (GetExtrudeDepth() != 0))
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

void E3dExtrudeObj::GetLineGeometry(PolyPolygon3D& rLinePolyPolygon) const
{
    // #78972# add extrude line polys
    rLinePolyPolygon.Insert(maLinePolyPolygon);

    // don't call parent
    // E3dCompoundObject::GetLineGeometry(rLinePolyPolygon);
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
            mpObjectItemSet->Put(Svx3DDoubleSidedItem(TRUE));

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
            &maLinePolyPolygon);
    }
    else
    {
        // nur ein Polygon erzeugen
        mpObjectItemSet->Put(Svx3DDoubleSidedItem(TRUE));

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

    // #78972#
    ImpCompleteLinePolygon(maLinePolyPolygon, aFrontSide.Count(), FALSE);

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

void E3dExtrudeObj::CreateWireframe(Polygon3D& rWirePoly, const Matrix4D* pTf,
    E3dDragDetail eDetail)
{
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
|* Objektdaten in Stream speichern
|*
\************************************************************************/

void E3dExtrudeObj::WriteData(SvStream& rOut) const
{
#ifndef SVX_LIGHT
    long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
    if(nVersion < 3800)
    {
        // Alte Geometrie erzeugen, um die E3dPolyObj's zu haben
        ((E3dCompoundObject*)this)->ReCreateGeometry(TRUE);
    }

    // call parent
    E3dCompoundObject::WriteData(rOut);

    E3dIOCompat aCompat(rOut, STREAM_WRITE, 1);
    rOut << aExtrudePolygon;
    rOut << fExtrudeScale;

    rOut << (double)GetExtrudeDepth();

    rOut << (double)GetPercentBackScale() / 100.0;

    rOut << (double)GetPercentDiagonal() / 200.0;

    rOut << GetSmoothNormals(); // #107245# (BOOL)bExtrudeSmoothed;
    rOut << GetSmoothLids(); // #107245# (BOOL)bExtrudeSmoothFrontBack;
    rOut << GetCharacterMode(); // #107245# (BOOL)bExtrudeCharacterMode;

    // Ab Version 513a (5.2.99): Parameter fuer das
    // Erzeugen der Vorder/Rueckwand
    rOut << GetCloseFront(); // #107245# (BOOL)bExtrudeCloseFront;
    rOut << GetCloseBack(); // #107245# (BOOL)bExtrudeCloseBack;

    if(nVersion < 3800)
    {
        // Geometrie neu erzeugen, um E3dPolyObj's wieder loszuwerden
        ((E3dCompoundObject*)this)->ReCreateGeometry();
    }
#endif
}

/*************************************************************************
|*
|* Objektdaten aus Stream laden
|*
\************************************************************************/

void E3dExtrudeObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    // call parent
    E3dCompoundObject::ReadData(rHead, rIn);

    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    BOOL bAllDone(FALSE);

    if(AreBytesLeft())
    {
        E3dIOCompat aIoCompat(rIn, STREAM_READ);
        if(aIoCompat.GetVersion() >= 1)
        {
            BOOL bTmp;
            double fTmp;

            rIn >> aExtrudePolygon;
            rIn >> fExtrudeScale;

            rIn >> fTmp;
            mpObjectItemSet->Put(Svx3DDepthItem(sal_uInt32(fTmp + 0.5)));

            rIn >> fTmp;
            mpObjectItemSet->Put(Svx3DBackscaleItem(sal_uInt16(fTmp * 100.0)));

            rIn >> fTmp;
            mpObjectItemSet->Put(Svx3DPercentDiagonalItem(sal_uInt16(fTmp * 200.0)));

            rIn >> bTmp; // #107245# bExtrudeSmoothed = bTmp;
            mpObjectItemSet->Put(Svx3DSmoothNormalsItem(bTmp));

            rIn >> bTmp; // #107245# bExtrudeSmoothFrontBack = bTmp;
            mpObjectItemSet->Put(Svx3DSmoothLidsItem(bTmp));

            rIn >> bTmp; // #107245# bExtrudeCharacterMode = bTmp;
            mpObjectItemSet->Put(Svx3DCharacterModeItem(bTmp));

            bAllDone = TRUE;

            if(aIoCompat.GetBytesLeft())
            {
                // Ab Version 513a (5.2.99): Parameter fuer das
                // Erzeugen der Vorder/Rueckwand
                BOOL bTmp;

                rIn >> bTmp; // #107245# bExtrudeCloseFront = bTmp;
                mpObjectItemSet->Put(Svx3DCloseFrontItem(bTmp));

                rIn >> bTmp; // #107245# bExtrudeCloseBack = bTmp;
                mpObjectItemSet->Put(Svx3DCloseBackItem(bTmp));
            }
            else
            {
                // #107245# bExtrudeCloseFront = TRUE;
                mpObjectItemSet->Put(Svx3DCloseFrontItem(sal_True));

                // #107245# bExtrudeCloseBack = TRUE;
                mpObjectItemSet->Put(Svx3DCloseBackItem(sal_True));
            }
        }
    }

    if(!bAllDone)
    {
        // Geometrie aus geladenen PolyObj's rekonstruieren
        SdrObjList* pSubList = GetSubList();
        if(pSubList && pSubList->GetObjCount())
        {
            // Vorderseite und Rueckseite sind die ersten
            // PolyObj's in der Liste, hole diese
            E3dPolyObj* pFront = NULL;
            E3dPolyObj* pBack = NULL;
            E3dPolyObj* pOther = NULL;

            UINT16 a;
            for(a=0;a<pSubList->GetObjCount();a++)
            {
                E3dPolyObj* pCandidate = (E3dPolyObj*)pSubList->GetObj(a);
                if(pCandidate->ISA(E3dPolyObj))
                {
                    // Die Nromalen der Vorder/Rueckseiten zeigen in Z-Richtung,
                    // nutze dies aus
                    const Vector3D& rNormal = pCandidate->GetNormal();
                    if(fabs(rNormal.X()) < 0.0000001 && fabs(rNormal.Y()) < 0.0000001)
                    {
                        if(rNormal.Z() > 0.0)
                        {
                            // Vorderseite
                            pFront = pCandidate;
                        }
                        else
                        {
                            // Rueckseite
                            pBack = pCandidate;
                        }
                    }
                    else
                    {
                        if(!pOther)
                            pOther = pCandidate;
                    }
                }
            }

            // Extrude-Tiefe feststellen
            if(pOther)
            {
                const PolyPolygon3D& rOtherPoly = pOther->GetPolyPolygon3D();
                // Hintereinanderliegende Paare in der alten Version waren
                // 0,1 und 3,2 (0,3 vorne)
                double fVal = (rOtherPoly[0][1] - rOtherPoly[0][0]).GetLength();
                mpObjectItemSet->Put(Svx3DDepthItem(sal_uInt32(fVal + 0.5)));
            }
            else
                // Einen Default vorsehen, kann aber eigentlich nie geschehen
                mpObjectItemSet->Put(Svx3DDepthItem(100));

            // Polygon fuer Vorderseite holen
            if(pFront)
            {
                aExtrudePolygon = pFront->GetPolyPolygon3D();
            }
            else
            {
                if(pBack)
                {
                    // Rueckseite benutzen und um -fExtrudeDepth in Z
                    // verschieben
                    aExtrudePolygon = pBack->GetPolyPolygon3D();
                    Matrix4D aMat;
                    aMat.Translate(Vector3D(0.0, 0.0, -(double)GetExtrudeDepth()));
                    aExtrudePolygon.Transform(aMat);
                }
                else
                {
                    // Die Polygondaten koennen aus den Vorderkanten
                    // der weiteren Polygone (Punkte 0,3) restauriert werden.
                    // evtl. spaeter ergaenzen
                    aExtrudePolygon.Clear();
                }
            }

            // Bestimmen, ob die Teilpolygone von aExtrudePolygon
            // geschlossen waren. Sie waren geschlossen, wenn ein
            // entsprechendes PolyObj existiert
            for(a=0;a<aExtrudePolygon.Count();a++)
            {
                Polygon3D &rPoly = aExtrudePolygon[a];
                USHORT nCnt = rPoly.GetPointCount();

                if(nCnt)
                {
                    Vector3D& rFirst = rPoly[0];
                    Vector3D& rLast = rPoly[nCnt - 1];
                    BOOL bClosePoly(FALSE);

                    for(UINT16 b=0;b<pSubList->GetObjCount();b++)
                    {
                        E3dPolyObj* pCandidate = (E3dPolyObj*)pSubList->GetObj(b);
                        if(pCandidate->ISA(E3dPolyObj)
                            && pCandidate != pFront && pCandidate != pBack)
                        {
                            const PolyPolygon3D& rCandPoly = pCandidate->GetPolyPolygon3D();
                            if(rCandPoly[0].GetPointCount() > 2)
                            {
                                if(rCandPoly[0][0] == rFirst && rCandPoly[0][3] == rLast)
                                    bClosePoly = TRUE;
                                if(rCandPoly[0][3] == rFirst && rCandPoly[0][0] == rLast)
                                    bClosePoly = TRUE;
                            }
                        }
                    }

                    rPoly.SetClosed(bClosePoly);
                }
            }

            // Setze die weiteren Parameter auf die defaults
            fExtrudeScale = 1.0;

            mpObjectItemSet->Put(Svx3DBackscaleItem(100));

            mpObjectItemSet->Put(Svx3DPercentDiagonalItem(10));

            // #107245# bExtrudeSmoothed = TRUE;
            mpObjectItemSet->Put(Svx3DSmoothNormalsItem(sal_True));

            // #107245# bExtrudeSmoothFrontBack = FALSE;
            mpObjectItemSet->Put(Svx3DSmoothLidsItem(sal_False));

            // #107245# bExtrudeCharacterMode = FALSE;
            mpObjectItemSet->Put(Svx3DCharacterModeItem(sal_False));
        }
    }

    // correct position of extrude polygon, in case it's not positioned
    // at the Z==0 layer
    if(aExtrudePolygon.Count() && aExtrudePolygon[0].GetPointCount())
    {
        const Vector3D& rFirstPoint = aExtrudePolygon[0][0];
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
            NbcSetTransform(GetTransform() * aTransMat); // #112587#

            // correct polygon itself
            aTransMat.Identity();
            aTransMat.TranslateZ(-fTransDepth);
            aExtrudePolygon.Transform(aTransMat);
        }
    }

    // Geometrie neu erzeugen
    ReCreateGeometry();
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

// #107245#
// void E3dExtrudeObj::SetExtrudeSmoothed(BOOL bNew)
// {
//  if(bExtrudeSmoothed != bNew)
//  {
//      bExtrudeSmoothed = bNew;
//      bGeometryValid = FALSE;
//  }
// }

// #107245#
// void E3dExtrudeObj::SetExtrudeSmoothFrontBack(BOOL bNew)
// {
//  if(bExtrudeSmoothFrontBack != bNew)
//  {
//      bExtrudeSmoothFrontBack = bNew;
//      bGeometryValid = FALSE;
//  }
// }

// #107245#
// void E3dExtrudeObj::SetExtrudeCharacterMode(BOOL bNew)
// {
//  if(bExtrudeCharacterMode != bNew)
//  {
//      bExtrudeCharacterMode = bNew;
//      bGeometryValid = FALSE;
//  }
// }

// #107245#
// void E3dExtrudeObj::SetExtrudeCloseFront(BOOL bNew)
// {
//  if(bExtrudeCloseFront != bNew)
//  {
//      bExtrudeCloseFront = bNew;
//      bGeometryValid = FALSE;
//  }
// }

// #107245#
// void E3dExtrudeObj::SetExtrudeCloseBack(BOOL bNew)
// {
//  if(bExtrudeCloseBack != bNew)
//  {
//      bExtrudeCloseBack = bNew;
//      bGeometryValid = FALSE;
//  }
// }

//////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access

void E3dExtrudeObj::PostItemChange(const sal_uInt16 nWhich)
{
    // call parent
    E3dCompoundObject::PostItemChange(nWhich);

    switch(nWhich)
    {
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
        case SDRATTR_3DOBJ_DEPTH:
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
        SfxItemSet aSet(GetItemSet());

        // Linien aktivieren, um Objekt garantiert sichtbar zu machen
        aSet.Put(XLineStyleItem (XLINE_SOLID));

        pPathObj->SetItemSet(aSet);
    }

    return pPathObj;
}

// EOF
