/*************************************************************************
 *
 *  $RCSfile: sphere3d.cxx,v $
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

#ifndef _SVDIO_HXX
#include "svdio.hxx"
#endif

#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif

#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif

#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _E3D_POLYOB3D_HXX
#include "polyob3d.hxx"
#endif

#ifndef _E3D_SPHERE3D_HXX
#include "sphere3d.hxx"
#endif

#ifndef _POLY3D_HXX
#include "poly3d.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

TYPEINIT1(E3dSphereObj, E3dCompoundObject);

/*************************************************************************
|*
|* Kugel aus Polygonfacetten nach Laengen und Breitengrad aufbauen
|*
\************************************************************************/

E3dSphereObj::E3dSphereObj(E3dDefaultAttributes& rDefault, const Vector3D& rCenter, const Vector3D& r3DSize)
:   E3dCompoundObject(rDefault)
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // Uebergebene drueberbuegeln
    aCenter = rCenter;
    aSize = r3DSize;

    // Geometrie erzeugen
    CreateGeometry();
}

E3dSphereObj::E3dSphereObj()
:   E3dCompoundObject()
{
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

/*************************************************************************
|*
|* Kugel erzeugen ohne die Polygone darin zu erzeugen
|*
\************************************************************************/

// FG: Dieser Aufruf erfolgt von der 3D-Object Factory (objfac3d.cxx) und zwar ausschliesslich beim
//     laden von Dokumenten. Hier braucht man keinen CreateSphere-Aufruf, denn die wirkliche
//     Anzahl Segmente ist ja noch nicht bekannt. Dies war bis zum 10.2.97 ein (kleines)
//     Speicherleck.
E3dSphereObj::E3dSphereObj(int dummy) // den Parameter braucht es um unterscheiden zu koennen, welcher
{                                     // der beiden Konstruktoren gemeint ist. Der obige halt per Default
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

void E3dSphereObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    // Defaults setzen
    nHSegments = rDefault.GetDefaultHSegments();
    nVSegments = rDefault.GetDefaultVSegments();
    aCenter = rDefault.GetDefaultSphereCenter();
    aSize = rDefault.GetDefaultSphereSize();
}

/*************************************************************************
|*
|* Geometrieerzeugung
|*
\************************************************************************/

void E3dSphereObj::CreateGeometry()
{
    Polygon3D   aRect3D(4);
    Polygon3D   aNormal3D(4);
    Polygon3D   aTexture3D(4);

    // Start der Geometrieerzeugung ankuendigen
    StartCreateGeometry();

    if ( nHSegments <  3  )  nHSegments =   3;
    if ( nHSegments > 100 )  nHSegments = 100;
    if ( nVSegments <  2  )  nVSegments =   2;
    if ( nVSegments > 100 )  nVSegments = 100;

    Vector3D aRadius = aSize / 2;
    double fHSin1, fHSin2, fHCos1, fHCos2;
    double fHAng = 0;
    double fHInc = DEG2RAD(360) / nHSegments;
    double fVInc = DEG2RAD(180) / nVSegments;

    fHSin2 = 0.0; // sin(0)
    fHCos2 = 1.0; // cos(0)

    for (USHORT nH = 0; nH < nHSegments; nH++)
    {
        fHAng += fHInc;
        fHSin1 = fHSin2;
        fHSin2 = sin(fHAng);
        fHCos1 = fHCos2;
        fHCos2 = cos(fHAng);

        double fRx, fRz;
        double fVAng = DEG2RAD(90);
        double fVSin1, fVSin2, fVCos1, fVCos2;
        fVSin2 = 1.0; // sin(90)
        fVCos2 = 0.0; // cos(90)

        for (USHORT nV = 0; nV < nVSegments; nV++)
        {
            Vector3D aPos;

            fVAng -= fVInc;
            fVSin1 = fVSin2;
            fVSin2 = sin(fVAng);
            fVCos1 = fVCos2;
            fVCos2 = cos(fVAng);

            aPos = aCenter;
            fRx = aRadius.X() * fVCos1;
            fRz = aRadius.Z() * fVCos1;
            aPos.X() += fRx * fHCos1;
            aPos.Y() += aRadius.Y() * fVSin1;
            aPos.Z() += fRz * fHSin1;
            aRect3D[1] = aPos;

            aPos.X() = aCenter.X() + fRx * fHCos2;
            aPos.Z() = aCenter.Z() + fRz * fHSin2;
            aRect3D[2] = aPos;

            aPos = aCenter;
            fRx = aRadius.X() * fVCos2;
            fRz = aRadius.Z() * fVCos2;
            aPos.X() += fRx * fHCos1;
            aPos.Y() += aRadius.Y() * fVSin2;
            aPos.Z() += fRz * fHSin1;
            aRect3D[0] = aPos;

            aPos.X() = aCenter.X() + fRx * fHCos2;
            aPos.Z() = aCenter.Z() + fRz * fHSin2;
            aRect3D[3] = aPos;

            if(GetCreateTexture())
            {
                aTexture3D[1].X() =
                    (double)(nHSegments - (nH + 1)) / (double)nHSegments;
                aTexture3D[1].Y() =
                    (double)nV / (double)nVSegments;

                aTexture3D[2].X() =
                    (double)((nHSegments - (nH + 1)) - 1) / (double)nHSegments;
                aTexture3D[2].Y() = aTexture3D[1].Y();

                aTexture3D[3].X() = aTexture3D[2].X();
                aTexture3D[3].Y() =
                    (double)(nV+1) / (double)nVSegments;

                aTexture3D[0].X() = aTexture3D[1].X();
                aTexture3D[0].Y() = aTexture3D[3].Y();
            }

            if(GetCreateNormals())
            {
                aNormal3D = aRect3D;
                aNormal3D[0].Normalize();
                aNormal3D[1].Normalize();
                aNormal3D[2].Normalize();
                aNormal3D[3].Normalize();

                if(GetCreateTexture())
                    AddGeometry(aRect3D, aNormal3D, aTexture3D, FALSE);
                else
                    AddGeometry(aRect3D, aNormal3D, FALSE);
            }
            else
            {
                AddGeometry(aRect3D, FALSE);
            }
        }
    }

    // call parent
    E3dCompoundObject::CreateGeometry();
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

UINT16 E3dSphereObj::GetObjIdentifier() const
{
    return E3D_SPHEREOBJ_ID;
}

/*************************************************************************
|*
|* Wireframe erzeugen
|*
\************************************************************************/

void E3dSphereObj::CreateWireframe(Polygon3D& rWirePoly, const Matrix4D* pTf,
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
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus n Polygonen
|*
\************************************************************************/

SdrObject *E3dSphereObj::DoConvertToPolyObj(BOOL bBezier) const
{
    return NULL;
}

/*************************************************************************
|*
|* Leer-Konstruktor
|*
\************************************************************************/

void E3dSphereObj::ReSegment(long nHSegs, long nVSegs)
{
    if ((nHSegs != nHSegments || nVSegs != nVSegments) &&
        (nHSegs != 0 || nVSegs != 0))
    {
        nHSegments = nHSegs;
        nVSegments = nVSegs;

        bGeometryValid = FALSE;
    }
}

/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

void E3dSphereObj::WriteData(SvStream& rOut) const
{
    long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
    if(nVersion < 3800)
    {
        // Alte Geometrie erzeugen, um die E3dPolyObj's zu haben
        ((E3dCompoundObject*)this)->ReCreateGeometry(TRUE);
    }

    // call parent
    SdrAttrObj::WriteData(rOut);

    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rOut, STREAM_WRITE);
#ifdef DBG_UTIL
    aCompat.SetID("E3dSphereObj");
#endif

    if (rOut.GetVersion() < 3560) // FG: kleiner als die Final Beta der Version 4.0
    {
        pSub->Save(rOut);
    }
    else
    {
        // [FG] Jetzt wird die Kindliste abgeklapptert, allerdings weiss ich im Gegensatz zu
        // Joe dass es nur E3dPolyObj - Kindobjekte sein koennen.
        // Jedes dieser Objekte frage ich ob es eigene Attribute enthaelt.  Falls OwnStyle ()
        // true liefert, werde ich das Polygon nicht wegspeichern.

        SdrObjListIter aIter(*pSub,IM_FLAT);
        while (aIter.IsMore()) {
            E3dPolyObj* pObj=(E3dPolyObj *) aIter.Next();
            if ((!pObj->IsNotPersistent()) && (pObj->OwnAttrs() || pObj->OwnStyle()))
            {
                rOut<<*pObj;
            }
            if (pSub->GetModel()!=NULL) pSub->GetModel()->IncProgress();
        }
        SdrIOHeader(rOut,STREAM_WRITE,SdrIOEndeID); // Endemarke
    }

    // Daß hier gehört zum E3dObject (ohne Basisklassen);
    if (rOut.GetVersion() < 3560)
    {
        rOut << aLocalBoundVol;

        Old_Matrix3D aMat3D;
        aMat3D = aTfMatrix;
        rOut << aMat3D;

        rOut << nLogicalGroup;
        rOut << nObjTreeLevel;
        rOut << nPartOfParent;
        rOut << UINT16(eDragDetail);
    }
    else
    {
        E3dObject::WriteOnlyOwnMembers(rOut);
    }
    // Das gehört zu E3dSphere
    rOut << nHSegments;
    rOut << nVSegments;
    rOut << aCenter;
    rOut << aSize;

    // Das hier ist ein Merkmal eines Compound-Objektes
    rOut << (BOOL) ((E3dCompoundObject *) this)->GetDoubleSided();

    // Ab Version 395 (8.6.98): Parameter aus dem Objekt
    // E3dCompoundObject. Da irgendwann mal jemand die Ableitungs-
    // hierarchie beim FileFormat unterbrochen hat, wurden diese Attribute
    // bisher NOCH NIE gespeichert (Grrr). Diese Stelle muss nun natuerlich
    // auch IMMER MITGEPFLEGT werden, wenn sich Parameter in
    // E3dCompoundObject oder E3dObject aendern.
    rOut << BOOL(bDoubleSided);
    rOut << BOOL(bCreateNormals);
    rOut << BOOL(bCreateTexture);
    rOut << BOOL(bUseStdNormals);
    rOut << BOOL(bUseStdNormalsUseSphere);
    rOut << BOOL(bUseStdTextureX);
    rOut << BOOL(bUseStdTextureXUseSphere);
    rOut << BOOL(bUseStdTextureY);
    rOut << BOOL(bUseStdTextureYUseSphere);
    rOut << BOOL(bShadow3D);
    aFrontMaterial.WriteData(rOut);
    aBackMaterial.WriteData(rOut);
    rOut << (UINT16)eTextureKind;
    rOut << (UINT16)eTextureMode;
    rOut << BOOL(bInvertNormals);

    // neu ab 534: (hat noch gefehlt)
    rOut << BOOL(bFilterTexture);

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

void E3dSphereObj::ReadData31(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    SdrDownCompat aCompat(rIn, STREAM_READ);
#ifdef DBG_UTIL
    aCompat.SetID("E3dSphereObj");
#endif
    // dann die Member
    UINT16  nTmp16;
    pSub->Load(rIn, *pPage);
    // FG: Die Daten des 3D-Objektes
    rIn >> aLocalBoundVol;

    Old_Matrix3D aMat3D;
    rIn >> aMat3D;
    aTfMatrix = Matrix4D(aMat3D);

    rIn >> nLogicalGroup;
    rIn >> nObjTreeLevel;
    rIn >> nPartOfParent;
    rIn >> nTmp16; eDragDetail = E3dDragDetail(nTmp16);
    // FG: Die Daten der Kugel
    rIn >> nHSegments;
    rIn >> nVSegments;
    rIn >> aCenter;
    rIn >> aSize;

    bBoundVolValid = FALSE;

    // Geometrie neu erzeugen
    ReCreateGeometry();
}

/*************************************************************************
|*
|* Objektdaten aus Stream laden
|*
\************************************************************************/

void E3dSphereObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    // FG (10.2.97) Der Pointer ist ein Kunstgriff er wird in der ReadData-Routine erzeugt dort werden
    //    die abgespeicherten attributierten Flaechen abgelegt. Danach wird eine Kugel mit Default-Paramtern
    //    erzeugt und dann werden die per Default erzeugten einfach, falls noetig mit denen aus dieser Liste
    //    ausgetauscht. Am Ende von ReadData wird die Liste wieder zerstoert.
    E3dObjList *pLoadedE3dPolyObjs;

    if (ImpCheckSubRecords (rHead, rIn))
    {
        // leider kann das E3dLatheObj nicht auf E3dObject abgestuetzt werden,
        // da neue Member hinzugekommen sind und die Kompatibilitaet erhalten
        // bleiben muss.
        SdrAttrObj::ReadData(rHead, rIn);
        if ((rIn.GetVersion() < 3560) || (rHead.GetVersion() <= 12))
        {
            ReadData31(rHead, rIn);
            return;
        }


        // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
        SdrDownCompat aCompat(rIn, STREAM_READ);
#ifdef DBG_UTIL
        aCompat.SetID("E3dSphereObj");
#endif
        // dann die Member
        BOOL    bMyDoubleSided;

        // [FG]: Es wird erstmal gelesen was da ist, ob es vollstaendig ist zeigt sich bei
        // dem Aufruf von CreateSphere, dort werden die fehlenden Flaechen einfach
        // erzeugt.
        pLoadedE3dPolyObjs = new E3dObjList(NULL, NULL);
        pLoadedE3dPolyObjs->SetOwnerObj(this);
        pLoadedE3dPolyObjs->SetListKind(SDROBJLIST_GROUPOBJ);
        pLoadedE3dPolyObjs->Load(rIn, *pPage);
        E3dObject::ReadOnlyOwnMembers(rHead, rIn);

        if (aCompat.GetBytesLeft ())
        {
            // neue Member
            rIn >> nHSegments;
            rIn >> nVSegments;
            rIn >> aCenter;
            rIn >> aSize;
            if (aCompat.GetBytesLeft ())
            {
                rIn >> bMyDoubleSided;
                SetDoubleSided (bMyDoubleSided); // FG: Member des E3dCompounds wird dort eigentlich
            }                                    //     sowieso nochmal gemacht, ist aber sicherer so.

            if (aCompat.GetBytesLeft())
            {
                // Ab Version 395 (8.6.98): Parameter aus dem Objekt
                // E3dCompoundObject. Da irgendwann mal jemand die Ableitungs-
                // hierarchie beim FileFormat unterbrochen hat, wurden diese Attribute
                // bisher NOCH NIE gespeichert (Grrr). Diese Stelle muss nun natuerlich
                // auch IMMER MITGEPFLEGT werden, wenn sich Parameter in
                // E3dCompoundObject oder E3dObject aendern.
                BOOL bTmp;
                UINT16 nTmp16;

                rIn >> bTmp; bDoubleSided = bTmp;
                rIn >> bTmp; bCreateNormals = bTmp;
                rIn >> bTmp; bCreateTexture = bTmp;
                rIn >> bTmp; bUseStdNormals = bTmp;
                rIn >> bTmp; bUseStdNormalsUseSphere = bTmp;
                rIn >> bTmp; bUseStdTextureX = bTmp;
                rIn >> bTmp; bUseStdTextureXUseSphere = bTmp;
                rIn >> bTmp; bUseStdTextureY = bTmp;
                rIn >> bTmp; bUseStdTextureYUseSphere = bTmp;
                rIn >> bTmp; bShadow3D = bTmp;
                aFrontMaterial.ReadData(rIn);
                aBackMaterial.ReadData(rIn);
                rIn >> nTmp16; eTextureKind = Base3DTextureKind(nTmp16);
                rIn >> nTmp16; eTextureMode = Base3DTextureMode(nTmp16);
                rIn >> bTmp; bInvertNormals = bTmp;
            }

            // neu ab 534: (hat noch gefehlt)
            if (aCompat.GetBytesLeft () >= sizeof (BOOL))
            {
                BOOL bTmp;
                rIn >> bTmp; bFilterTexture = bTmp;
            }
        }
        else
        {
            DBG_ERROR("AW: Kugel laden: nicht vorgesehener Fall");
        }
        pLoadedE3dPolyObjs->Clear();
        delete pLoadedE3dPolyObjs;
    }

    // Geometrie neu erzeugen
    ReCreateGeometry();
}

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/

void E3dSphereObj::operator=(const SdrObject& rObj)
{
    // erstmal alle Childs kopieren
    E3dCompoundObject::operator=(rObj);

    // weitere Parameter kopieren
    const E3dSphereObj& r3DObj = (const E3dSphereObj&) rObj;

    nHSegments = r3DObj.nHSegments;
    nVSegments = r3DObj.nVSegments;
    aCenter       = r3DObj.aCenter;
    aSize         = r3DObj.aSize;
}

/*************************************************************************
|*
|* uebergebene Transformationsmatrix anwenden
|*
\************************************************************************/

void E3dSphereObj::ApplyTransform(const Matrix4D& rMatrix)
{
    // call parent
    E3dCompoundObject::ApplyTransform(rMatrix);

    // Anwenden auf lokale geometrie
    Vector3D aOuterEdge = aCenter + aSize;
    aCenter *= rMatrix;
    aOuterEdge *= rMatrix;
    aSize = aOuterEdge - aCenter;
}

/*************************************************************************
|*
|* Lokale Parameter setzen mit Geometrieneuerzeugung
|*
\************************************************************************/

void E3dSphereObj::SetHSegments(long nNew)
{
    if(nHSegments != nNew)
    {
        nHSegments = nNew;
        bGeometryValid = FALSE;
    }
}

void E3dSphereObj::SetVSegments(long nNew)
{
    if(nVSegments != nNew)
    {
        nVSegments = nNew;
        bGeometryValid = FALSE;
    }
}

void E3dSphereObj::SetCenter(const Vector3D& rNew)
{
    if(aCenter != rNew)
    {
        aCenter = rNew;
        bGeometryValid = FALSE;
    }
}

void E3dSphereObj::SetSize(const Vector3D& rNew)
{
    if(aSize != rNew)
    {
        aSize = rNew;
        bGeometryValid = FALSE;
    }
}

/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/

void E3dSphereObj::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
{
    // call parent
    E3dCompoundObject::NbcSetAttributes(rAttr, bReplaceAll);

    // special Attr for E3dSphereObj
    const SfxPoolItem* pPoolItem = NULL;

    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_HORZ_SEGS, TRUE, &pPoolItem ) )
    {
        UINT32 nNew = ((const SfxUInt32Item*)pPoolItem)->GetValue();
        SetHSegments(nNew);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_VERT_SEGS, TRUE, &pPoolItem ) )
    {
        UINT32 nNew = ((const SfxUInt32Item*)pPoolItem)->GetValue();
        SetVSegments(nNew);
    }
}

/*************************************************************************
|*
|* Attribute lesen
|*
\************************************************************************/

void E3dSphereObj::TakeAttributes(SfxItemSet& rAttr, FASTBOOL bMerge, FASTBOOL bOnlyHardAttr) const
{
    // call parent
    E3dCompoundObject::TakeAttributes(rAttr, bMerge, bOnlyHardAttr);

    // special Attr for E3dSphereObj
    const SfxPoolItem* pPoolItem = NULL;
    SfxItemState eState;

    long nObjHorzSegs = GetHSegments();
    long nObjVertSegs = GetVSegments();

    // HorizSegs
    eState = rAttr.GetItemState(SID_ATTR_3D_HORZ_SEGS, FALSE, &pPoolItem);
    if(eState == SFX_ITEM_SET)
    {
        // Ist gesetzt
        if((UINT32)nObjHorzSegs != ((const SfxUInt32Item*)pPoolItem)->GetValue())
        {
            // SfxPoolItem muss invalidiert werden
            rAttr.InvalidateItem(SID_ATTR_3D_HORZ_SEGS);
        }
    }
    else
    {
        if(!(eState == SFX_ITEM_DONTCARE))
        {
            // Item gab es noch nicht, setze es
            rAttr.Put(SfxUInt32Item(SID_ATTR_3D_HORZ_SEGS, nObjHorzSegs));
        }
    }

    // VertSegs
    eState = rAttr.GetItemState(SID_ATTR_3D_VERT_SEGS, FALSE, &pPoolItem);
    if(eState == SFX_ITEM_SET)
    {
        // Ist gesetzt
        if((UINT32)nObjVertSegs != ((const SfxUInt32Item*)pPoolItem)->GetValue())
        {
            // SfxPoolItem muss invalidiert werden
            rAttr.InvalidateItem(SID_ATTR_3D_VERT_SEGS);
        }
    }
    else
    {
        if(!(eState == SFX_ITEM_DONTCARE))
        {
            // Item gab es noch nicht, setze es
            rAttr.Put(SfxUInt32Item(SID_ATTR_3D_VERT_SEGS, nObjVertSegs));
        }
    }
}

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/

void E3dSphereObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulSphere3d);
}

/*************************************************************************
|*
|* Get the name of the object (plural)
|*
\************************************************************************/

void E3dSphereObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralSphere3d);
}


