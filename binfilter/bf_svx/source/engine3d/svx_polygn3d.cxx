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

#ifndef _E3D_POLYGON3D_HXX
#include "polygn3d.hxx"
#endif


#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif


#ifndef _E3D_E3DIOCMPT_HXX
#include "e3dcmpt.hxx"
#endif
namespace binfilter {

/*N*/ TYPEINIT1(E3dPolygonObj, E3dCompoundObject);

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ E3dPolygonObj::E3dPolygonObj(
/*N*/ 	E3dDefaultAttributes& rDefault,
/*N*/ 	const PolyPolygon3D& rPolyPoly3D,
/*N*/ 	BOOL bLinOnly)
/*N*/ :	E3dCompoundObject(rDefault),
/*N*/ 	bLineOnly(bLinOnly)
/*N*/ {
/*N*/ 	// Geometrie setzen
/*N*/ 	SetPolyPolygon3D(rPolyPoly3D);
/*N*/ 
/*N*/ 	// Default-Normals erzeugen
/*N*/ 	CreateDefaultNormals();
/*N*/ 
/*N*/ 	// Default-Texturkoordinaten erzeugen
/*N*/ 	CreateDefaultTexture();
/*N*/ 
/*N*/ 	// Geometrie erzeugen
/*N*/ 	CreateGeometry();
/*N*/ }

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*?*/ E3dPolygonObj::E3dPolygonObj(
/*?*/ 	E3dDefaultAttributes& rDefault,
/*?*/ 	const PolyPolygon3D& rPolyPoly3D,
/*?*/ 	const PolyPolygon3D& rPolyNormals3D,
/*?*/ 	BOOL bLinOnly)
/*?*/ :	E3dCompoundObject(rDefault),
/*?*/ 	bLineOnly(bLinOnly)
/*?*/ {
/*?*/ 	// Geometrie und Normalen setzen
/*?*/ 	SetPolyPolygon3D(rPolyPoly3D);
/*?*/ 	SetPolyNormals3D(rPolyNormals3D);
/*?*/ 
/*?*/ 	// Default-Texturkoordinaten erzeugen
/*?*/ 	CreateDefaultTexture();
/*?*/ 
/*?*/ 	// Geometrie erzeugen
/*?*/ 	CreateGeometry();
/*?*/ }

/*************************************************************************
|*
|* Linien-Konstruktor
|*
\************************************************************************/

/*N*/ E3dPolygonObj::E3dPolygonObj(
/*N*/ 	E3dDefaultAttributes& rDefault,
/*N*/ 	const Vector3D& rP1, const Vector3D& rP2,
/*N*/ 	BOOL bLinOnly)
/*N*/ :	E3dCompoundObject(rDefault),
/*N*/ 	aPolyPoly3D(1),
/*N*/ 	bLineOnly(bLinOnly)
/*N*/ {
/*N*/ 	Polygon3D aPoly3D(2);
/*N*/ 	aPoly3D[0] = rP1;
/*N*/ 	aPoly3D[1] = rP2;
/*N*/ 	aPolyPoly3D.Insert(aPoly3D);
/*N*/ 
/*N*/ 	// Geometrie erzeugen
/*N*/ 	CreateGeometry();
/*N*/ }

/*************************************************************************
|*
|* Leer-Konstruktor
|*
\************************************************************************/

/*N*/ E3dPolygonObj::E3dPolygonObj()
/*N*/ :	E3dCompoundObject()
/*N*/ {
/*N*/ 	// Keine Geometrie erzeugen
/*N*/ }

/*************************************************************************
|*
|* Default-Normalen erzeugen
|*
\************************************************************************/

/*N*/ void E3dPolygonObj::CreateDefaultNormals()
/*N*/ {
/*N*/ 	PolyPolygon3D aPolyNormals(aPolyPoly3D.Count());
/*N*/ 
/*N*/ 	// Komplettes PolyPolygon mit den Ebenennormalen anlegen
/*N*/ 	for(UINT16 a=0;a<aPolyPoly3D.Count();a++)
/*N*/ 	{
/*N*/ 		// Quellpolygon finden
/*N*/ 		const Polygon3D& rPolygon = aPolyPoly3D[a];
/*N*/ 
/*N*/ 		// Neues Polygon fuer Normalen anlegen
/*N*/ 		Polygon3D aNormals(rPolygon.GetPointCount());
/*N*/ 
/*N*/ 		// Normale holen (und umdrehen)
/*N*/ 		Vector3D aNormal = -rPolygon.GetNormal();
/*N*/ 
/*N*/ 		// Neues Polygon fuellen
/*N*/ 		for(UINT16 b=0;b<rPolygon.GetPointCount();b++)
/*N*/ 			aNormals[b] = aNormal;
/*N*/ 
/*N*/ 		// Neues Polygon in PolyPolygon einfuegen
/*N*/ 		aPolyNormals.Insert(aNormals);
/*N*/ 	}
/*N*/ 
/*N*/ 	// Default-Normalen setzen
/*N*/ 	SetPolyNormals3D(aPolyNormals);
/*N*/ }

/*************************************************************************
|*
|* Default-Texturkoordinaten erzeugen
|*
\************************************************************************/

/*N*/ void E3dPolygonObj::CreateDefaultTexture()
/*N*/ {
/*N*/ 	PolyPolygon3D aPolyTexture(aPolyPoly3D.Count());
/*N*/ 
/*N*/ 	// Komplettes PolyPolygon mit den Texturkoordinaten anlegen
/*N*/ 	// Die Texturkoordinaten erstrecken sich ueber X,Y und Z
/*N*/ 	// ueber die gesamten Extremwerte im Bereich 0.0 .. 1.0
/*N*/ 	for(UINT16 a=0;a<aPolyPoly3D.Count();a++)
/*N*/ 	{
/*N*/ 		// Quellpolygon finden
/*N*/ 		const Polygon3D& rPolygon = aPolyPoly3D[a];
/*N*/ 
/*N*/ 		// Gesamtgroesse des Objektes feststellen
/*N*/ 		Volume3D aVolume = rPolygon.GetPolySize();
/*N*/ 
/*N*/ 		// Neues Polygon fuer Texturkoordinaten anlegen
/*N*/ 		Polygon3D aTexture(rPolygon.GetPointCount());
/*N*/ 
/*N*/ 		// Normale holen
/*N*/ 		Vector3D aNormal = rPolygon.GetNormal();
/*N*/ 		aNormal.Abs();
/*N*/ 
/*N*/ 		// Entscheiden, welche Koordinaten als Source fuer das
/*N*/ 		// Mapping benutzt werden sollen
/*N*/ 		UINT16 nSourceMode = 0;
/*N*/ 
/*N*/ 		// Groessten Freiheitsgrad ermitteln
/*N*/ 		if(!(aNormal.X() > aNormal.Y() && aNormal.X() > aNormal.Z()))
/*N*/ 		{
/*N*/ 			if(aNormal.Y() > aNormal.Z())
/*N*/ 			{
/*N*/ 				// Y ist am groessten, benutze X,Z als mapping
/*N*/ 				nSourceMode = 1;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// Z ist am groessten, benutze X,Y als mapping
/*N*/ 				nSourceMode = 2;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// Neues Polygon fuellen
/*N*/ 		for(UINT16 b=0;b<rPolygon.GetPointCount();b++)
/*N*/ 		{
/*N*/ 			Vector3D& aTex = aTexture[b];
/*N*/ 			const Vector3D& aPoly = rPolygon[b];
/*N*/ 
/*N*/ 			switch(nSourceMode)
/*N*/ 			{
/*N*/ 				case 0:	// Quelle ist Y,Z
/*N*/ 					if(aVolume.GetHeight())
/*N*/ 						aTex.X() = (aPoly.Y() - aVolume.MinVec().Y()) / aVolume.GetHeight();
/*N*/ 					else
/*N*/ 						aTex.X() = 0.0;
/*N*/ 
/*N*/ 					if(aVolume.GetDepth())
/*N*/ 						aTex.Y() = (aPoly.Z() - aVolume.MinVec().Z()) / aVolume.GetDepth();
/*N*/ 					else
/*N*/ 						aTex.Y() = 0.0;
/*N*/ 					break;
/*N*/ 
/*N*/ 				case 1:	// Quelle ist X,Z
/*N*/ 					if(aVolume.GetWidth())
/*N*/ 						aTex.X() = (aPoly.X() - aVolume.MinVec().X()) / aVolume.GetWidth();
/*N*/ 					else
/*N*/ 						aTex.X() = 0.0;
/*N*/ 
/*N*/ 					if(aVolume.GetDepth())
/*N*/ 						aTex.Y() = (aPoly.Z() - aVolume.MinVec().Z()) / aVolume.GetDepth();
/*N*/ 					else
/*N*/ 						aTex.Y() = 0.0;
/*N*/ 					break;
/*N*/ 
/*N*/ 				case 2:	// Quelle ist X,Y
/*N*/ 					if(aVolume.GetWidth())
/*N*/ 						aTex.X() = (aPoly.X() - aVolume.MinVec().X()) / aVolume.GetWidth();
/*N*/ 					else
/*N*/ 						aTex.X() = 0.0;
/*N*/ 
/*N*/ 					if(aVolume.GetHeight())
/*N*/ 						aTex.Y() = (aPoly.Y() - aVolume.MinVec().Y()) / aVolume.GetHeight();
/*N*/ 					else
/*N*/ 						aTex.Y() = 0.0;
/*N*/ 					break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// Neues Polygon in PolyPolygon einfuegen
/*N*/ 		aPolyTexture.Insert(aTexture);
/*N*/ 	}
/*N*/ 
/*N*/ 	// Default-Texturkoordinaten setzen
/*N*/ 	SetPolyTexture3D(aPolyTexture);
/*N*/ }

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*N*/ E3dPolygonObj::~E3dPolygonObj()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

/*N*/ UINT16 E3dPolygonObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return E3D_POLYGONOBJ_ID;
/*N*/ }

/*************************************************************************
|*
|* Wireframe erzeugen
|*
\************************************************************************/


/*************************************************************************
|*
|* Polygon setzen
|*
\************************************************************************/

/*N*/ void E3dPolygonObj::SetPolyPolygon3D(const PolyPolygon3D& rNewPolyPoly3D)
/*N*/ {
/*N*/ 	if ( aPolyPoly3D != rNewPolyPoly3D )
/*N*/ 	{
/*N*/ 		// Neues PolyPolygon; kopieren
/*N*/ 		aPolyPoly3D = rNewPolyPoly3D;
/*N*/ 
/*N*/ 		// Geometrie neu erzeugen
/*N*/ 		bGeometryValid = FALSE;
/*N*/ 	}
/*N*/ }

/*N*/ void E3dPolygonObj::SetPolyNormals3D(const PolyPolygon3D& rNewPolyNormals3D)
/*N*/ {
/*N*/ 	if ( aPolyNormals3D != rNewPolyNormals3D )
/*N*/ 	{
/*N*/ 		// Neue Normalen; kopieren
/*N*/ 		aPolyNormals3D = rNewPolyNormals3D;
/*N*/ 
/*N*/ 		// Geometrie neu erzeugen
/*N*/ 		bGeometryValid = FALSE;
/*N*/ 	}
/*N*/ }

/*N*/ void E3dPolygonObj::SetPolyTexture3D(const PolyPolygon3D& rNewPolyTexture3D)
/*N*/ {
/*N*/ 	if ( aPolyTexture3D != rNewPolyTexture3D )
/*N*/ 	{
/*N*/ 		// Neue Texturkoordinaten; kopieren
/*N*/ 		aPolyTexture3D = rNewPolyTexture3D;
/*N*/ 
/*N*/ 		// Geometrie neu erzeugen
/*N*/ 		bGeometryValid = FALSE;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus 6 Polygonen
|*
\************************************************************************/


/*************************************************************************
|*
|* Give out simple line geometry
|*
\************************************************************************/


/*************************************************************************
|*
|* Geometrieerzeugung
|*
\************************************************************************/


/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

/*N*/ void E3dPolygonObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
/*N*/ 	if(nVersion < 3800)
/*N*/ 	{
/*N*/ 		// Alte Geometrie erzeugen, um die E3dPolyObj's zu haben
/*N*/ 		((E3dCompoundObject*)this)->ReCreateGeometry(TRUE);
/*N*/ 	}
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	E3dCompoundObject::WriteData(rOut);
/*N*/ 
/*N*/ 	E3dIOCompat aCompat(rOut, STREAM_WRITE, 1);
/*N*/ 	rOut << BOOL(bLineOnly);
/*N*/ 
/*N*/ 	if(nVersion < 3800)
/*N*/ 	{
/*N*/ 		// Geometrie neu erzeugen, um E3dPolyObj's wieder loszuwerden
/*N*/ 		((E3dCompoundObject*)this)->ReCreateGeometry();
/*N*/ 	}
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|* Objektdaten aus Stream laden
|*
\************************************************************************/

/*N*/ void E3dPolygonObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	E3dCompoundObject::ReadData(rHead, rIn);
/*N*/ 
/*N*/ 	// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 	if(AreBytesLeft())
/*N*/ 	{
/*N*/ 		E3dIOCompat aIoCompat(rIn, STREAM_READ);
/*N*/ 		if(aIoCompat.GetVersion() >= 1)
/*N*/ 		{
/*N*/ 			rIn >> bLineOnly;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Geometrie neu erzeugen, mit oder ohne E3dPolyObj's
/*N*/ 	ReCreateGeometry();
/*N*/ }

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/


/*************************************************************************
|*
|* LineOnly setzen
|*
\************************************************************************/

/*N*/ void E3dPolygonObj::SetLineOnly(BOOL bNew)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

}
