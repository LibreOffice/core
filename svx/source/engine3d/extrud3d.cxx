/*************************************************************************
 *
 *  $RCSfile: extrud3d.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:15 $
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

TYPEINIT1(E3dExtrudeObj, E3dCompoundObject);

/*************************************************************************
|*
|* Konstruktor, erzeugt zwei Deckelflaechen-PolyPolygone und (PointCount-1)
|* Seitenflaechen-Rechtecke aus dem uebergebenen PolyPolygon
|*
\************************************************************************/

E3dExtrudeObj::E3dExtrudeObj(E3dDefaultAttributes& rDefault, const PolyPolygon& rPP, double fDepth)
:   E3dCompoundObject(rDefault),
    aExtrudePolygon(rPP, rDefault.GetDefaultExtrudeScale()),
    fExtrudeDepth(fDepth)
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

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
    aExtrudePolygon(rXPP, rDefault.GetDefaultExtrudeScale()),
    fExtrudeDepth(fDepth)
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // Geometrie erzeugen
    CreateGeometry();
}

E3dExtrudeObj::E3dExtrudeObj()
:   E3dCompoundObject()
{
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    fExtrudeDepth = aDefault.GetDefaultExtrudeDepth();
    SetDefaultAttributes(aDefault);
}

void E3dExtrudeObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    // Defaults setzen
    fExtrudeScale = rDefault.GetDefaultExtrudeScale();
    fExtrudeBackScale = rDefault.GetDefaultBackScale();
    fExtrudePercentDiag = rDefault.GetDefaultPercentDiag();
    bExtrudeSmoothed = rDefault.GetDefaultExtrudeSmoothed();
    bExtrudeSmoothFrontBack = rDefault.GetDefaultExtrudeSmoothFrontBack();
    bExtrudeCharacterMode = rDefault.GetDefaultExtrudeCharacterMode();
    bExtrudeCloseFront = rDefault.GetDefaultExtrudeCloseFront();
    bExtrudeCloseBack = rDefault.GetDefaultExtrudeCloseBack();

    // Bei extrudes defaultmaessig StdTexture in X und Y
    bUseStdTextureX = TRUE;
    bUseStdTextureY = TRUE;
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
    if((aNormal.Z() > 0.0) != (fExtrudeDepth > 0.0))
        aPolyPoly3D.FlipDirections();

    // Orientierung evtl. vorhandener Loecher in einen definierten
    // Ausgangszustand bringen
    aPolyPoly3D.SetDirections();

    return aPolyPoly3D;
}

PolyPolygon3D E3dExtrudeObj::GetBackSide(const PolyPolygon3D& rFrontSide)
{
    PolyPolygon3D aBackSide(rFrontSide);

    if(fExtrudeDepth != 0.0)
    {
        // Extrudevektor bilden
        Vector3D aNormal = aBackSide.GetNormal();
        if(aNormal.Z() < 0.0)
            aNormal.Z() = -aNormal.Z();
        Vector3D aOffset = aNormal * fExtrudeDepth;

        // eventuell Skalieren
        if(fExtrudeBackScale != 1.0)
            ScalePoly(aBackSide, fExtrudeBackScale);

        // Verschieben
        Matrix4D aTrans;
        aTrans.Translate(aOffset);
        aBackSide.Transform(aTrans);
    }

    return aBackSide;
}

void E3dExtrudeObj::CreateGeometry()
{
    // Start der Geometrieerzeugung ankuendigen
    StartCreateGeometry();

    // Polygon als Grundlage holen
    PolyPolygon3D aFrontSide = GetFrontSide();

    if(fExtrudeDepth != 0.0)
    {
        // Hinteres Polygon erzeugen
        PolyPolygon3D aBackSide = GetBackSide(aFrontSide);

        // Was muss erzeugt werden?
        if(!aFrontSide.IsClosed())
            bDoubleSided = TRUE;

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
        CreateSegment(
            aFrontSide,
            aBackSide,
            0L,
            0L,
            bExtrudeCloseFront,
            bExtrudeCloseBack,
            fExtrudePercentDiag,
            GetExtrudeSmoothed(),
            GetExtrudeSmoothed(),
            GetExtrudeSmoothFrontBack(),
            fSurroundFactor,
            fTextureStart,
            fTextureDepth,
            GetCreateNormals(),
            GetCreateTexture(),
            bExtrudeCharacterMode,
            FALSE);
    }
    else
    {
        // nur ein Polygon erzeugen
        SetDoubleSided(TRUE);

        // Fuer evtl. selbst erzeugte Normalen
        PolyPolygon3D aNormalsFront;

        // Extrudevektor bilden
        Vector3D aNormal = aFrontSide.GetNormal();
        Vector3D aOffset = aNormal * fExtrudeDepth;

        // Normalen und Vorderseite selbst erzeugen
        AddFrontNormals(aFrontSide, aNormalsFront, aOffset);
        CreateFront(aFrontSide, aNormalsFront, GetCreateNormals(), GetCreateTexture());
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
|* uebergebene Transformationsmatrix anwenden, BoundVolume anpassen
|*
\************************************************************************/

void E3dExtrudeObj::ApplyTransform(const Matrix4D& rMatrix)
{
    // call parent
    E3dCompoundObject::ApplyTransform(rMatrix);

    // Anwenden auf lokale geometrie
    aExtrudePolygon.Transform(rMatrix);
}

/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

void E3dExtrudeObj::WriteData(SvStream& rOut) const
{
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
    rOut << fExtrudeDepth;
    rOut << fExtrudeBackScale;
    rOut << fExtrudePercentDiag;

    rOut << (BOOL)bExtrudeSmoothed;
    rOut << (BOOL)bExtrudeSmoothFrontBack;
    rOut << (BOOL)bExtrudeCharacterMode;

    // Ab Version 513a (5.2.99): Parameter fuer das
    // Erzeugen der Vorder/Rueckwand
    rOut << (BOOL)bExtrudeCloseFront;
    rOut << (BOOL)bExtrudeCloseBack;

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

            rIn >> aExtrudePolygon;
            rIn >> fExtrudeScale;
            rIn >> fExtrudeDepth;
            rIn >> fExtrudeBackScale;
            rIn >> fExtrudePercentDiag;

            rIn >> bTmp; bExtrudeSmoothed = bTmp;
            rIn >> bTmp; bExtrudeSmoothFrontBack = bTmp;
            rIn >> bTmp; bExtrudeCharacterMode = bTmp;

            bAllDone = TRUE;

            if(aIoCompat.GetBytesLeft())
            {
                // Ab Version 513a (5.2.99): Parameter fuer das
                // Erzeugen der Vorder/Rueckwand
                BOOL bTmp;

                rIn >> bTmp; bExtrudeCloseFront = bTmp;
                rIn >> bTmp; bExtrudeCloseBack = bTmp;
            }
            else
            {
                bExtrudeCloseFront = TRUE;
                bExtrudeCloseBack = TRUE;
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
                fExtrudeDepth = (rOtherPoly[0][1] - rOtherPoly[0][0]).GetLength();
            }
            else
                // Einen Default vorsehen, kann aber eigentlich nie geschehen
                fExtrudeDepth = 100.0;

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
                    aMat.Translate(Vector3D(0.0, 0.0, -fExtrudeDepth));
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
            fExtrudeBackScale = 1.0;
            fExtrudePercentDiag = 0.05;

            bExtrudeSmoothed = TRUE;
            bExtrudeSmoothFrontBack = FALSE;
            bExtrudeCharacterMode = FALSE;
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
    fExtrudeDepth = r3DObj.fExtrudeDepth;
    fExtrudeBackScale = r3DObj.fExtrudeBackScale;
    fExtrudePercentDiag = r3DObj.fExtrudePercentDiag;

    bExtrudeSmoothed = r3DObj.bExtrudeSmoothed;
    bExtrudeSmoothFrontBack = r3DObj.bExtrudeSmoothFrontBack;
    bExtrudeCharacterMode = r3DObj.bExtrudeCharacterMode;
    bExtrudeCloseFront = r3DObj.bExtrudeCloseFront;
    bExtrudeCloseBack = r3DObj.bExtrudeCloseBack;
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

void E3dExtrudeObj::SetExtrudeDepth(double fNew)
{
    if(fExtrudeDepth != fNew)
    {
        fExtrudeDepth = fNew;
        bGeometryValid = FALSE;
    }
}

void E3dExtrudeObj::SetExtrudeBackScale(double fNew)
{
    if(fExtrudeBackScale != fNew)
    {
        fExtrudeBackScale = fNew;
        bGeometryValid = FALSE;
    }
}

void E3dExtrudeObj::SetExtrudePercentDiag(double fNew)
{
    if(fExtrudePercentDiag != fNew)
    {
        fExtrudePercentDiag = fNew;
        bGeometryValid = FALSE;
    }
}

void E3dExtrudeObj::SetExtrudeSmoothed(BOOL bNew)
{
    if(bExtrudeSmoothed != bNew)
    {
        bExtrudeSmoothed = bNew;
        bGeometryValid = FALSE;
    }
}

void E3dExtrudeObj::SetExtrudeSmoothFrontBack(BOOL bNew)
{
    if(bExtrudeSmoothFrontBack != bNew)
    {
        bExtrudeSmoothFrontBack = bNew;
        bGeometryValid = FALSE;
    }
}

void E3dExtrudeObj::SetExtrudeCharacterMode(BOOL bNew)
{
    if(bExtrudeCharacterMode != bNew)
    {
        bExtrudeCharacterMode = bNew;
        bGeometryValid = FALSE;
    }
}

void E3dExtrudeObj::SetExtrudeCloseFront(BOOL bNew)
{
    if(bExtrudeCloseFront != bNew)
    {
        bExtrudeCloseFront = bNew;
        bGeometryValid = FALSE;
    }
}

void E3dExtrudeObj::SetExtrudeCloseBack(BOOL bNew)
{
    if(bExtrudeCloseBack != bNew)
    {
        bExtrudeCloseBack = bNew;
        bGeometryValid = FALSE;
    }
}

/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/

void E3dExtrudeObj::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
{
    // call parent
    E3dCompoundObject::NbcSetAttributes(rAttr, bReplaceAll);

    // special Attr for E3dExtrudeObj
    const SfxPoolItem* pPoolItem = NULL;

    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_PERCENT_DIAGONAL, TRUE, &pPoolItem ) )
    {
        UINT16 nNew = ((const SfxUInt16Item*)pPoolItem)->GetValue();
        SetExtrudePercentDiag((double)nNew / 200.0);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_BACKSCALE, TRUE, &pPoolItem ) )
    {
        UINT16 nNew = ((const SfxUInt16Item*)pPoolItem)->GetValue();
        SetExtrudeBackScale((double)nNew / 100.0);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_DEPTH, TRUE, &pPoolItem ) )
    {
        UINT32 nNew = ((const SfxUInt32Item*)pPoolItem)->GetValue();
        SetExtrudeDepth((double)nNew);
    }
}

/*************************************************************************
|*
|* Attribute lesen
|*
\************************************************************************/

void E3dExtrudeObj::TakeAttributes(SfxItemSet& rAttr, FASTBOOL bMerge, FASTBOOL bOnlyHardAttr) const
{
    // call parent
    E3dCompoundObject::TakeAttributes(rAttr, bMerge, bOnlyHardAttr);

    // special Attr for E3dExtrudeObj
    const SfxPoolItem* pPoolItem = NULL;
    SfxItemState eState;

    UINT16 nObjPercentDiagonal = (UINT16)((GetExtrudePercentDiag() * 200.0) + 0.5);
    UINT16 nObjBackScale = (UINT16)((GetExtrudeBackScale() * 100.0) + 0.5);
    UINT32 nObjDeepth = (UINT32)(GetExtrudeDepth() + 0.5);

    // PercentDiagonal
    eState = rAttr.GetItemState(SID_ATTR_3D_PERCENT_DIAGONAL, FALSE, &pPoolItem);
    if(eState == SFX_ITEM_SET)
    {
        // Ist gesetzt
        if(nObjPercentDiagonal != ((const SfxUInt16Item*)pPoolItem)->GetValue())
        {
            // SfxPoolItem muss invalidiert werden
            rAttr.InvalidateItem(SID_ATTR_3D_PERCENT_DIAGONAL);
        }
    }
    else
    {
        if(!(eState == SFX_ITEM_DONTCARE))
        {
            // Item gab es noch nicht, setze es
            rAttr.Put(SfxUInt16Item(SID_ATTR_3D_PERCENT_DIAGONAL, nObjPercentDiagonal));
        }
    }

    // BackScale
    eState = rAttr.GetItemState(SID_ATTR_3D_BACKSCALE, FALSE, &pPoolItem);
    if(eState == SFX_ITEM_SET)
    {
        // Ist gesetzt
        if(nObjBackScale != ((const SfxUInt16Item*)pPoolItem)->GetValue())
        {
            // SfxPoolItem muss invalidiert werden
            rAttr.InvalidateItem(SID_ATTR_3D_BACKSCALE);
        }
    }
    else
    {
        if(!(eState == SFX_ITEM_DONTCARE))
        {
            // Item gab es noch nicht, setze es
            rAttr.Put(SfxUInt16Item(SID_ATTR_3D_BACKSCALE, nObjBackScale));
        }
    }

    // ExtrudeDepth
    eState = rAttr.GetItemState(SID_ATTR_3D_DEPTH, FALSE, &pPoolItem);
    if(eState == SFX_ITEM_SET)
    {
        // Ist gesetzt
        if(nObjDeepth != ((const SfxUInt32Item*)pPoolItem)->GetValue())
        {
            // SfxPoolItem muss invalidiert werden
            rAttr.InvalidateItem(SID_ATTR_3D_DEPTH);
        }
    }
    else
    {
        if(!(eState == SFX_ITEM_DONTCARE))
        {
            // Item gab es noch nicht, setze es
            rAttr.Put(SfxUInt32Item(SID_ATTR_3D_DEPTH, nObjDeepth));
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
        SfxItemSet aAttr(GetModel()->GetItemPool());
        TakeAttributes(aAttr, TRUE, FALSE);

        // Linien aktivieren, um Objekt garantiert sichtbar zu machen
        aAttr.Put(XLineStyleItem (XLINE_SOLID));

        pPathObj->NbcSetAttributes(aAttr, FALSE);
    }

    return pPathObj;
}


