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

#include <stdio.h>

#ifndef _STRING_H
#include <string.h>
#ifndef _STRING_H
#define _STRING_H
#endif
#endif

#include "xpoly.hxx"

#include "poly3d.hxx"


#include <tools/poly.hxx>


namespace binfilter {

/*N*/ DBG_NAME(Polygon3D)
/*N*/ DBG_NAME(PolyPolygon3D)

/*************************************************************************
|*
|* 3D-PolyPolygon-Implementierung
|*
\************************************************************************/

/*N*/ DECLARE_LIST(Polygon3DList, Polygon3D*)//STRIP008 ;

/*N*/ class ImpPolyPolygon3D
/*N*/ {
/*N*/ public:
/*N*/ 	Polygon3DList				aPoly3DList;
/*N*/ 	UINT16						nRefCount;
/*N*/ 
/*N*/ 	ImpPolyPolygon3D(UINT16 nInitSize = 4, UINT16 nResize = 4) 
/*N*/ 	:	aPoly3DList(1024, nInitSize, nResize) { nRefCount = 1; }
/*N*/ 	ImpPolyPolygon3D(const ImpPolyPolygon3D& rImpPolyPoly3D);
/*N*/ 	~ImpPolyPolygon3D();
/*N*/ 
/*N*/ 	BOOL operator==(const ImpPolyPolygon3D& rImpPolyPoly3D) const;
/*N*/ 	BOOL operator!=(const ImpPolyPolygon3D& rImpPolyPoly3D) const
/*N*/ 		{ return !operator==(rImpPolyPoly3D); }
/*N*/ };

/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

/*N*/ ImpPolygon3D::ImpPolygon3D(UINT16 nInitSize, UINT16 nPolyResize)
/*N*/ {
/*N*/ 	pPointAry = NULL;
/*N*/ 	bDeleteOldAry = FALSE;
/*N*/ 	bClosed = FALSE;
/*N*/ 	nSize = 0;
/*N*/ 	nResize = nPolyResize;
/*N*/ 	nPoints = 0;
/*N*/ 	nRefCount = 1;
/*N*/ 	Resize(nInitSize);
/*N*/ }

/*************************************************************************
|*
|* Copy-Konstruktor
|*
\************************************************************************/

/*N*/ ImpPolygon3D::ImpPolygon3D(const ImpPolygon3D& rImpPoly3D)
/*N*/ {
/*N*/ 	((ImpPolygon3D&)rImpPoly3D).CheckPointDelete();
/*N*/ 
/*N*/ 	pPointAry = NULL;
/*N*/ 	bDeleteOldAry = FALSE;
/*N*/ 	bClosed = rImpPoly3D.bClosed;
/*N*/ 	nSize = 0;
/*N*/ 	nResize = rImpPoly3D.nResize;
/*N*/ 	nPoints = 0;
/*N*/ 	nRefCount = 1;
/*N*/ 	Resize(rImpPoly3D.nSize);
/*N*/ 
/*N*/ 	// Kopieren
/*N*/ 	nPoints	= rImpPoly3D.nPoints;
/*N*/ 	memcpy(pPointAry, rImpPoly3D.pPointAry, nSize*sizeof(Vector3D));
/*N*/ }

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*N*/ ImpPolygon3D::~ImpPolygon3D()
/*N*/ {
/*N*/ 	delete[] (char*)pPointAry;
/*N*/ 
/*N*/ 	if(bDeleteOldAry)
/*N*/ 		delete[] (char*)pOldPointAry;
/*N*/ }

/*************************************************************************
|*
|* Gibt unter Beachtung eines Flags den Speicher fuer das Polygon frei.
|*
\************************************************************************/

/*N*/ void ImpPolygon3D::CheckPointDelete()
/*N*/ {
/*N*/ 	if(bDeleteOldAry)
/*N*/ 	{
/*N*/ 		delete[] (char*)pOldPointAry;
/*N*/ 		bDeleteOldAry = FALSE;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*    ImpPolygon3D::Resize()
|*
|*    !!! Polygongroesse aendern - wenn bDeletePoints FALSE, dann den
|*    Punkt-Array nicht loeschen, sondern in pOldPointAry sichern und
|*    das Flag bDeleteOldAry setzen. Beim naechsten Zugriff wird
|*    das Array dann geloescht.
|*    Damit wird verhindert, dass bei Poly3D[n] = Poly3D[0] durch ein
|*    Resize der fuer den rechten Ausdruck verwendete Vector3D-Array
|*    vorzeitig geloescht wird.
|*
\************************************************************************/

/*N*/ void ImpPolygon3D::Resize(UINT16 nNewSize, BOOL bDeletePoints)
/*N*/ {
/*N*/ 	if(nNewSize == nSize)
/*N*/ 		return;
/*N*/ 
/*N*/ 	UINT16 nOldSize = nSize;
/*N*/ 
/*N*/ 	CheckPointDelete();
/*N*/ 	pOldPointAry = pPointAry;
/*N*/ 
/*N*/ 	// Neue Groesse auf vielfaches von nResize runden, sofern Objekt
/*N*/ 	// nicht neu angelegt wurde (nSize != 0)
/*N*/ 	if(nSize && nNewSize > nSize)
/*N*/ 	{
/*N*/ 		DBG_ASSERT(nResize, "Resize-Versuch trotz nResize = 0 !");
/*N*/ 		nNewSize = nSize + ((nNewSize-nSize-1) / nResize + 1) * nResize;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Punkt-Array erzeugen
/*N*/ 	nSize = nNewSize;
/*N*/ 	pPointAry = (Vector3D*)new char[nSize * sizeof(Vector3D)];
/*N*/ 	memset(pPointAry, 0, nSize * sizeof(Vector3D));
/*N*/ 
/*N*/ 	// Eventuell umkopieren
/*N*/ 	if(nOldSize)
/*N*/ 	{
/*N*/ 		if(nOldSize < nSize)
/*N*/ 		{
/*N*/ 			memcpy(pPointAry, pOldPointAry, nOldSize * sizeof(Vector3D));
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			memcpy(pPointAry, pOldPointAry, nSize*sizeof(Vector3D));
/*N*/ 
/*N*/ 			// Anzahl der gueltigen Punkte anpassen
/*N*/ 			if(nPoints > nSize)
/*N*/ 				nPoints = nSize;
/*N*/ 		}
/*N*/ 		if(bDeletePoints)
/*N*/ 			delete[] (char*)pOldPointAry;
/*N*/ 		else					
/*N*/ 			bDeleteOldAry = TRUE;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Vektoren einfuegen
|*
\************************************************************************/


/*************************************************************************
|*
|* Vektoren entfernen
|*
\************************************************************************/

/*N*/ void ImpPolygon3D::Remove(UINT16 nPos, UINT16 nCount)
/*N*/ {
/*N*/ 	CheckPointDelete();
/*N*/ 
/*N*/ 	if(nPos + nCount <= nPoints)
/*N*/ 	{
/*N*/ 		UINT16 nMove = nPoints - nPos - nCount;
/*N*/ 		if(nMove)
/*N*/ 			memmove(&pPointAry[nPos], &pPointAry[nPos+nCount],
/*N*/ 					nMove * sizeof(Vector3D));
/*N*/ 
/*N*/ 		nPoints -= nCount;
/*N*/ 		memset(&pPointAry[nPoints], 0, nCount * sizeof(Vector3D));
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

/*N*/ Polygon3D::Polygon3D(UINT16 nSize, UINT16 nResize)
/*N*/ {
/*N*/ 	pImpPolygon3D = new ImpPolygon3D(nSize, nResize);
/*N*/ }

/*************************************************************************
|*
|* Copy-Konstruktor
|*
\************************************************************************/

/*N*/ Polygon3D::Polygon3D(const Polygon3D& rPoly3D)
/*N*/ {
/*N*/ 	pImpPolygon3D = rPoly3D.pImpPolygon3D;
/*N*/ 	pImpPolygon3D->nRefCount++;
/*N*/ }

/*************************************************************************
|*
|* Konstruktor aus Standard-Polygon
|*
\************************************************************************/

/*N*/ Polygon3D::Polygon3D(const Polygon& rPoly, double fScale)
/*N*/ {
/*N*/ 	UINT16 nSize(rPoly.GetSize());
/*N*/ 	pImpPolygon3D = new ImpPolygon3D(nSize);
/*N*/ 
/*N*/ 	if(fScale != 1.0)
/*N*/ 	{
/*?*/ 		for(UINT16 a=0; a<nSize; a++)
/*?*/ 		{
/*?*/ 			pImpPolygon3D->pPointAry[a].X() = rPoly[a].X() * fScale;
/*?*/ 			pImpPolygon3D->pPointAry[a].Y() = -rPoly[a].Y() * fScale;
/*?*/ 			pImpPolygon3D->pPointAry[a].Z() = 0.0;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for(UINT16 a=0; a<nSize; a++)
/*N*/ 		{
/*N*/ 			pImpPolygon3D->pPointAry[a].X() = rPoly[a].X();
/*N*/ 			pImpPolygon3D->pPointAry[a].Y() = -rPoly[a].Y();
/*N*/ 			pImpPolygon3D->pPointAry[a].Z() = 0.0;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	pImpPolygon3D->nPoints = nSize;
/*N*/ 
/*N*/ 	CheckClosed();
/*N*/ }

/*************************************************************************
|*
|* Konstruktor aus XPolygon - Achtung! Es werden nur die Punkte
|* uebernommen, ohne Konvertierung
|*
\************************************************************************/

/*N*/ Polygon3D::Polygon3D(const XPolygon& rXPoly, double fScale)
/*N*/ {
/*N*/ 	UINT16 nSize(rXPoly.GetPointCount());
/*N*/ 	pImpPolygon3D = new ImpPolygon3D(nSize);
/*N*/ 
/*N*/ 	if(fScale != 1.0)
/*N*/ 	{
/*?*/ 		for(UINT16 a=0; a<nSize; a++)
/*?*/ 		{
/*?*/ 			pImpPolygon3D->pPointAry[a].X() = rXPoly[a].X() * fScale;
/*?*/ 			pImpPolygon3D->pPointAry[a].Y() = -rXPoly[a].Y() * fScale;
/*?*/ 			pImpPolygon3D->pPointAry[a].Z() = 0.0;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for(UINT16 a=0; a<nSize; a++)
/*N*/ 		{
/*N*/ 			pImpPolygon3D->pPointAry[a].X() = rXPoly[a].X();
/*N*/ 			pImpPolygon3D->pPointAry[a].Y() = -rXPoly[a].Y();
/*N*/ 			pImpPolygon3D->pPointAry[a].Z() = 0.0;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	pImpPolygon3D->nPoints = nSize;
/*N*/ 
/*N*/ 	CheckClosed();
/*N*/ }

/*************************************************************************
|*
|* Closed-Status der Einzelpolygone korrigieren
|*
\************************************************************************/

/*N*/ void Polygon3D::CheckClosed()
/*N*/ {
/*N*/ 	// #97067# Test needs to look for > 1, not for != 0
/*N*/ 	if(pImpPolygon3D->nPoints > 1
/*N*/ 		&& pImpPolygon3D->pPointAry[0] == pImpPolygon3D->pPointAry[pImpPolygon3D->nPoints - 1])
/*N*/ 	{
/*N*/ 		pImpPolygon3D->bClosed = TRUE;
/*N*/ 		pImpPolygon3D->nPoints--;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*N*/ Polygon3D::~Polygon3D()
/*N*/ {
/*N*/ 	if( pImpPolygon3D->nRefCount > 1 )
/*N*/ 		pImpPolygon3D->nRefCount--;
/*N*/ 	else
/*N*/ 		delete pImpPolygon3D;
/*N*/ }

/*************************************************************************
|*
|* Closed-Zustand des Polygons testen
|*
\************************************************************************/

/*N*/ BOOL Polygon3D::IsClosed() const
/*N*/ {
/*N*/ 	return pImpPolygon3D->bClosed;
/*N*/ }

/*************************************************************************
|*
|* Closed-Zustand des Polygons aendern
|*
\************************************************************************/

/*N*/ void Polygon3D::SetClosed(BOOL bNew)
/*N*/ {
/*N*/ 	if(bNew != IsClosed())
/*N*/ 	{
/*N*/ 		CheckReference();
/*N*/ 		pImpPolygon3D->bClosed = bNew;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Referenzzaehler desImpPoly3D pruefen und ggf. von diesem abkoppeln
|*
\************************************************************************/

/*N*/ void Polygon3D::CheckReference()
/*N*/ {
/*N*/ 	if(pImpPolygon3D->nRefCount > 1)
/*N*/ 	{
/*N*/ 		pImpPolygon3D->nRefCount--;
/*N*/ 		pImpPolygon3D = new ImpPolygon3D(*pImpPolygon3D);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* neue Groesse setzen
|*
\************************************************************************/


/*************************************************************************
|*
|* Groesse zurueckgeben
|*
\************************************************************************/


/*************************************************************************
|*
|* Laenge der Polygonkante zurueckgeben
|*
\************************************************************************/

/*N*/ double Polygon3D::GetLength() const
/*N*/ {
/*N*/ 	double fRetval = 0.0;
/*N*/ 	Vector3D *pLast = &pImpPolygon3D->pPointAry[
/*N*/ 		IsClosed() ? pImpPolygon3D->nPoints - 1 : 0];
/*N*/ 
/*N*/ 	for(UINT16 a=IsClosed() ? 0 : 1;a<pImpPolygon3D->nPoints;a++)
/*N*/ 	{
/*N*/ 		Vector3D *pCandidate = &pImpPolygon3D->pPointAry[a];
/*N*/ 		Vector3D aVec = *pCandidate - *pLast;
/*N*/ 		fRetval += aVec.GetLength();
/*N*/ 		pLast = pCandidate;
/*N*/ 	}
/*N*/ 
/*N*/ 	return fRetval;
/*N*/ }

/*************************************************************************
|*
|* Anzahl der belegten Punkte setzen
|*
\************************************************************************/

/*N*/ void Polygon3D::SetPointCount(UINT16 nPoints)
/*N*/ {
/*N*/ 	pImpPolygon3D->CheckPointDelete();
/*N*/ 	CheckReference();
/*N*/ 
/*N*/ 	if(pImpPolygon3D->nSize < nPoints)
/*?*/ 		pImpPolygon3D->Resize(nPoints);
/*N*/ 
/*N*/ 	if(nPoints < pImpPolygon3D->nPoints)
/*N*/ 	{
/*N*/ 		UINT16 nSize = pImpPolygon3D->nPoints - nPoints;
/*N*/ 		memset(&pImpPolygon3D->pPointAry[nPoints], 0, nSize * sizeof(Vector3D));
/*N*/ 	}
/*N*/ 	pImpPolygon3D->nPoints = nPoints;
/*N*/ }

/*************************************************************************
|*
|* Anzahl der belegten Punkte zurueckgeben
|*
\************************************************************************/

/*N*/ UINT16 Polygon3D::GetPointCount() const
/*N*/ {
/*N*/ 	pImpPolygon3D->CheckPointDelete();
/*N*/ 	return pImpPolygon3D->nPoints;
/*N*/ }

/*************************************************************************
|*
|* Polygonpunkte entfernen
|*
\************************************************************************/


/*************************************************************************
|*
|* const-Arrayoperator
|*
\************************************************************************/

/*N*/ const Vector3D& Polygon3D::operator[]( UINT16 nPos ) const
/*N*/ {
/*N*/ 	DBG_ASSERT(nPos < pImpPolygon3D->nPoints, "Ungueltiger Index bei const-Arrayzugriff auf Polygon3D");
/*N*/ 	pImpPolygon3D->CheckPointDelete();
/*N*/ 	return pImpPolygon3D->pPointAry[nPos];
/*N*/ }

/*************************************************************************
|*
|* Arrayoperator
|*
\************************************************************************/

/*N*/ Vector3D& Polygon3D::operator[]( UINT16 nPos )
/*N*/ {
/*N*/ 	pImpPolygon3D->CheckPointDelete();
/*N*/ 	CheckReference();
/*N*/ 
/*N*/ 	if(nPos >= pImpPolygon3D->nSize)
/*N*/ 	{
/*N*/ 		DBG_ASSERT(pImpPolygon3D->nResize, "Ungueltiger Index bei Arrayzugriff auf Polygon3D");
/*N*/ 		pImpPolygon3D->Resize(nPos + 1, FALSE);
/*N*/ 	}
/*N*/ 
/*N*/ 	if(nPos >= pImpPolygon3D->nPoints)
/*N*/ 		pImpPolygon3D->nPoints = nPos + 1;
/*N*/ 
/*N*/ 	return pImpPolygon3D->pPointAry[nPos];
/*N*/ }

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/

/*N*/ Polygon3D& Polygon3D::operator=(const Polygon3D& rPoly3D)
/*N*/ {
/*N*/ 	pImpPolygon3D->CheckPointDelete();
/*N*/ 
/*N*/ 	rPoly3D.pImpPolygon3D->nRefCount++;
/*N*/ 
/*N*/ 	if(pImpPolygon3D->nRefCount > 1)
/*N*/ 		pImpPolygon3D->nRefCount--;
/*N*/ 	else
/*N*/ 		delete pImpPolygon3D;
/*N*/ 
/*N*/ 	pImpPolygon3D = rPoly3D.pImpPolygon3D;
/*N*/ 
/*N*/ 	return *this;
/*N*/ }

/*************************************************************************
|*
|* Vergleichsoperator
|*
\************************************************************************/

/*N*/ BOOL Polygon3D::operator==(const Polygon3D& rPoly3D) const
/*N*/ {
/*N*/ 	pImpPolygon3D->CheckPointDelete();
/*N*/ 	
/*N*/ 	if(rPoly3D.pImpPolygon3D == pImpPolygon3D)
/*N*/ 		return TRUE;
/*N*/ 
/*N*/ 	if(pImpPolygon3D->nPoints != rPoly3D.pImpPolygon3D->nPoints)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	// point-to point compare necessary
/*N*/ 	for(UINT16 a=0;a<pImpPolygon3D->nPoints;a++)
/*N*/ 		if(pImpPolygon3D->pPointAry[a] != rPoly3D.pImpPolygon3D->pPointAry[a])
/*N*/ 			return FALSE;
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*************************************************************************
|*
|* Ungleichoperator
|*
\************************************************************************/


/*************************************************************************
|*
|* aus einem Stream lesen
|*
*************************************************************************/

/*N*/ SvStream& operator>>(SvStream& rIStream, Polygon3D& rPoly3D)
/*N*/ {
/*N*/ 	DBG_CHKOBJ(&rPoly3D, Polygon3D, NULL);
/*N*/ 	UINT16 nPntCnt;
/*N*/ 
/*N*/ 	rPoly3D.pImpPolygon3D->CheckPointDelete();
/*N*/ 
/*N*/ 	// Anzahl der Punkte einlesen und Array erzeugen
/*N*/ 	rIStream >> nPntCnt;
/*N*/ 	rPoly3D.pImpPolygon3D->nPoints = nPntCnt;
/*N*/ 
/*N*/ 	if(rPoly3D.pImpPolygon3D->nRefCount != 1)
/*N*/ 	{
/*?*/ 		if(rPoly3D.pImpPolygon3D->nRefCount)
/*?*/ 			rPoly3D.pImpPolygon3D->nRefCount--;
/*?*/ 		rPoly3D.pImpPolygon3D = new ImpPolygon3D(nPntCnt);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rPoly3D.pImpPolygon3D->Resize(nPntCnt);
/*N*/ 
/*N*/ 	Vector3D* pPoint = rPoly3D.pImpPolygon3D->pPointAry;
/*N*/ 
/*N*/ 	for(UINT16 i = 0; i < nPntCnt; i++)
/*N*/ 		rIStream >> *pPoint++;
/*N*/ 
/*N*/ 	// auf geschlossenheit pruefen
/*N*/ 	if(*rPoly3D.pImpPolygon3D->pPointAry == *(pPoint-1))
/*N*/ 	{
/*N*/ 		rPoly3D.pImpPolygon3D->bClosed = TRUE;
/*N*/ 		rPoly3D.pImpPolygon3D->nPoints = nPntCnt-1;
/*N*/ 	}
/*N*/ 
/*N*/ 	INT16 nTmp;
/*N*/ 	rIStream >> nTmp; // war mal nConvexPoint
/*N*/ 	rIStream >> nTmp; // alt: rPoly3D.nOuterPoly;
/*N*/ 
/*N*/ 	return rIStream;
/*N*/ }

/*************************************************************************
|*
|* in einem Stream speichern
|*
*************************************************************************/

/*N*/ SvStream& operator<<(SvStream& rOStream, const Polygon3D& rPoly3D)
/*N*/ {
/*N*/ 	DBG_CHKOBJ(&rPoly3D, Polygon3D, NULL);
/*N*/ 
/*N*/ 	rPoly3D.pImpPolygon3D->CheckPointDelete();
/*N*/ 
/*N*/ 	Vector3D* pPoint = rPoly3D.pImpPolygon3D->pPointAry;
/*N*/ 	UINT16 nPntCnt = rPoly3D.GetPointCount();
/*N*/ 
/*N*/ 	if(rPoly3D.pImpPolygon3D->bClosed)
/*N*/ 	{
/*N*/ 		nPntCnt++;
/*N*/ 		rOStream << nPntCnt;
/*N*/ 		nPntCnt--;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		rOStream << nPntCnt;
/*N*/ 	}
/*N*/ 
/*N*/ 	for (UINT16 i = 0; i < nPntCnt; i++)
/*N*/ 		rOStream << *pPoint++;
/*N*/ 
/*N*/ 	if(rPoly3D.pImpPolygon3D->bClosed)
/*N*/ 	{
/*N*/ 		// ersten Punkt nochmal, um polygon auf geschlossen zu zwingen
/*N*/ 		pPoint = rPoly3D.pImpPolygon3D->pPointAry;
/*N*/ 		rOStream << *pPoint;
/*N*/ 	}
/*N*/ 
/*N*/ 	rOStream << (INT16)0; // war mal nConvexPoint
/*N*/ 	rOStream << (INT16)-1; // alt: rPoly3D.nOuterPoly;
/*N*/ 
/*N*/ 	return rOStream;
/*N*/ }

/*************************************************************************
|*
|* Laufrichtung des Polygons feststellen
|*
\************************************************************************/

/*N*/ BOOL Polygon3D::IsClockwise(const Vector3D &rNormal) const
/*N*/ {
/*N*/ 	double fZValue = rNormal.Scalar(GetNormal());
/*N*/ 	return (fZValue >= 0.0);
/*N*/ }

/*************************************************************************
|*
|* Eine garantier die Orientierung wiederspiegelnde Ecke des Polygons
|* liefern
|*
\************************************************************************/

/*N*/ UINT16 Polygon3D::GetHighestEdge() const
/*N*/ {
/*N*/ 	UINT16 nRetval = 0;
/*N*/ 	Vector3D *pHighest = &pImpPolygon3D->pPointAry[nRetval];
/*N*/ 	for(UINT16 a=1;a<pImpPolygon3D->nPoints;a++)
/*N*/ 	{
/*N*/ 		Vector3D *pCandidate = &pImpPolygon3D->pPointAry[a];
/*N*/ 		if(pCandidate->X() <= pHighest->X())
/*N*/ 		{
/*N*/ 			if(pCandidate->X() < pHighest->X())
/*N*/ 			{
/*N*/ 				pHighest = pCandidate;
/*N*/ 				nRetval = a;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if(pCandidate->Y() <= pHighest->Y())
/*N*/ 				{
/*N*/ 					if(pCandidate->Y() < pHighest->Y())
/*N*/ 					{
/*N*/ 						pHighest = pCandidate;
/*N*/ 						nRetval = a;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						if(pCandidate->Z() < pHighest->Z())
/*N*/ 						{
/*N*/ 							pHighest = pCandidate;
/*N*/ 							nRetval = a;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nRetval;
/*N*/ }

/*************************************************************************
|*
|* Normale des Polygons liefern
|*
|* Dabei eine Ecke waehlen, die die spezifische Orientierung des Polygons
|* besitzt und mit gleichen Punkten rechnen!
|*
\************************************************************************/

/*N*/ Vector3D Polygon3D::GetNormal() const
/*N*/ {
/*N*/ 	Vector3D aNormal(0.0, 0.0, -1.0);
/*N*/ 
/*N*/ 	if(pImpPolygon3D->nPoints > 2)
/*N*/ 	{
/*N*/ 		// HighestEdge bestimmen
/*N*/ 		UINT16 nPntCnt = pImpPolygon3D->nPoints;
/*N*/ 		UINT16 nHighest = GetHighestEdge();
/*N*/ 		const Vector3D &rHighest = (*this)[nHighest];
/*N*/ 
/*N*/ 		// Vorgaenger bestimmen
/*N*/ 		UINT16 nPrev(nHighest);
/*N*/ 		do {
/*N*/ 			nPrev = (nPrev == 0) ? nPntCnt-1 : nPrev-1;
/*N*/ 		} while((*this)[nPrev] == rHighest && nPrev != nHighest);
/*N*/ 		const Vector3D &rPrev = (*this)[nPrev];
/*N*/ 
/*N*/ 		// Nachfolger bestimmen
/*N*/ 		UINT16 nNext(nHighest);
/*N*/ 		do {
/*N*/ 			nNext = (nNext == nPntCnt-1) ? 0 : nNext+1;
/*N*/ 		} while((*this)[nNext] == rHighest && nNext != nHighest);
/*N*/ 		const Vector3D &rNext = (*this)[nNext];
/*N*/ 
/*N*/ 		// Fehlerfaelle abfangen
/*N*/ 		if(rHighest == rPrev || rHighest == rNext || rPrev == rNext)
/*N*/ 			return aNormal;
/*N*/ 
/*N*/ 		// Normale bilden
/*N*/ 		aNormal = (rPrev - rHighest)|(rNext - rHighest);
/*N*/ 		
/*N*/ 		// get length
/*N*/ 		double fLen = aNormal.GetLength();
/*N*/ 		
/*N*/ 		// correct length
/*N*/ 		if(fabs(fLen) < SMALL_DVALUE)
/*N*/ 			fLen = 0.0;
/*N*/ 		
/*N*/ 		if(0.0 != fLen)
/*N*/ 		{
/*N*/ 			// use own normalize here since we already know the vector length
/*N*/ 			// with square root applied
/*N*/ 			// aNormal.Normalize();
/*N*/ 			if(1.0 != fLen)
/*N*/ 			{
/*N*/ 				// normalize
/*N*/ 				double fFac = 1.0 / fLen;
/*N*/ 				aNormal = Vector3D(aNormal.X() * fFac, aNormal.Y() * fFac, aNormal.Z() * fFac);
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// points are on one line, use default normal
/*N*/ 			aNormal = Vector3D(0.0, 0.0, -1.0);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return aNormal;
/*N*/ }

/*************************************************************************
|*
|* Mittelpunkt des Polygons (in Bezug auf die Einzelpunkte) berechnen
|*
\************************************************************************/

/*N*/ Vector3D Polygon3D::GetMiddle() const
/*N*/ {
/*N*/ 	Vector3D aMiddle;
/*N*/ 	UINT16 nPntCnt = pImpPolygon3D->nPoints;
/*N*/ 
/*N*/ 	for(UINT16 i = 0; i < nPntCnt; i++)
/*N*/ 		aMiddle += pImpPolygon3D->pPointAry[i];
/*N*/ 
/*N*/ 	aMiddle /= (double)nPntCnt;
/*N*/ 
/*N*/ 	return aMiddle;
/*N*/ }

/*************************************************************************
|*
|* Laufrichtung des Polygons umkehren
|*
\************************************************************************/

/*N*/ void Polygon3D::FlipDirection()
/*N*/ {
/*N*/ 	pImpPolygon3D->CheckPointDelete();
/*N*/ 	CheckReference();
/*N*/ 
/*N*/ 	UINT16 nPntCnt = pImpPolygon3D->nPoints;
/*N*/ 	UINT16 nCnt = nPntCnt / 2;
/*N*/ 	Vector3D* pBeg = pImpPolygon3D->pPointAry;
/*N*/ 	Vector3D* pEnd = pBeg + nPntCnt - 1;
/*N*/ 
/*N*/ 	for(UINT16 i = 0; i < nCnt; i++)
/*N*/ 	{
/*N*/ 		Vector3D aTmp = *pBeg;
/*N*/ 		*pBeg++ = *pEnd;
/*N*/ 		*pEnd-- = aTmp;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Polygonpunkte mit uebergebener Matrix transformieren
|*
\************************************************************************/

/*N*/ void Polygon3D::Transform(const Matrix4D& rTfMatrix)
/*N*/ {
/*N*/ 	pImpPolygon3D->CheckPointDelete();
/*N*/ 	CheckReference();
/*N*/ 
/*N*/ 	Vector3D* pPoint = pImpPolygon3D->pPointAry;
/*N*/ 	UINT16 nPntCnt = pImpPolygon3D->nPoints;
/*N*/ 
/*N*/ 	for(UINT16 i = 0; i < nPntCnt; i++)
/*N*/ 		*pPoint++ *= rTfMatrix;
/*N*/ }

/*************************************************************************
|*
|* doppelte aufeinanderfolgende Polygonpunkte entfernen; ein
|* geschlossenes Polygon wird dabei ggf. "geoeffnet"
|*
\************************************************************************/

/*N*/ void Polygon3D::RemoveDoublePoints()
/*N*/ {
/*N*/ 	pImpPolygon3D->CheckPointDelete();
/*N*/ 	CheckReference();
/*N*/ 
/*N*/ 	UINT16 nPntCnt = pImpPolygon3D->nPoints;
/*N*/ 
/*N*/ 	if(nPntCnt)
/*N*/ 	{
/*N*/ 		Vector3D* pPoint = pImpPolygon3D->pPointAry;
/*N*/ 		Vector3D aP0 = *pPoint;
/*N*/ 
/*N*/ 		pPoint += nPntCnt;
/*N*/ 
/*N*/ 		while(*--pPoint == aP0 && nPntCnt > 3)
/*N*/ 		{
/*N*/ 			nPntCnt--;
/*N*/ 			pImpPolygon3D->bClosed = TRUE;
/*N*/ 		}
/*N*/ 
/*N*/ 		for(UINT16 i = nPntCnt-1; i > 0 && nPntCnt > 3; i--)
/*N*/ 		{
/*N*/ 			if(*pPoint == *(pPoint-1))
/*N*/ 			{ 
/*N*/ 				pImpPolygon3D->Remove(i, 1);
/*N*/ 				nPntCnt--;
/*N*/ 			}
/*N*/ 			pPoint--;
/*N*/ 		}
/*N*/ 
/*N*/ 		SetPointCount(nPntCnt);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		DBG_ASSERT(FALSE, "Empty polygon used!");
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Ueberlappen sich das aktuelle und das angegebene Polygon ?
|*
\************************************************************************/



/*************************************************************************
|*
|* Existiert ein Schnitt zwischen den Polys?
|*
\************************************************************************/


/*************************************************************************
|*
|* Crossings Test for point and whole polygon
|*
\************************************************************************/

/*N*/ BOOL Polygon3D::IsInside(const Vector3D& rPnt, BOOL bWithBorder) const
/*N*/ {
/*N*/ 	BOOL bInside(FALSE);
/*N*/ 	UINT16 nNumPoint(GetPointCount());
/*N*/ 	Vector3D* pPoints = pImpPolygon3D->pPointAry;
/*N*/ 
/*N*/ 	for(UINT16 a=0;a<nNumPoint;a++)
/*N*/ 	{
/*N*/ 		if(bWithBorder
/*N*/ 			&& (fabs(pPoints[a].X() - rPnt.X()) < SMALL_DVALUE)
/*N*/ 			&& (fabs(pPoints[a].Y() - rPnt.Y()) < SMALL_DVALUE))
/*N*/ 			return TRUE;
/*N*/ 
/*N*/ 		UINT16 nPrev((!a) ? nNumPoint-1 : a-1);
/*N*/ 
/*N*/ 		if((pPoints[nPrev].Y() - rPnt.Y() > -SMALL_DVALUE) != (pPoints[a].Y() - rPnt.Y() > -SMALL_DVALUE))
/*N*/ 		{
/*N*/ 			BOOL bXFlagOld(pPoints[nPrev].X() - rPnt.X() > -SMALL_DVALUE);
/*N*/ 
/*N*/ 			if(bXFlagOld == (pPoints[a].X() - rPnt.X() > -SMALL_DVALUE))
/*N*/ 			{
/*N*/ 				if(bXFlagOld)
/*N*/ 					bInside = !bInside;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				double fCmp = 
/*N*/ 					pPoints[a].X() - (pPoints[a].Y() - rPnt.Y()) *
/*N*/ 					(pPoints[nPrev].X() - pPoints[a].X()) /
/*N*/ 					(pPoints[nPrev].Y() - pPoints[a].Y());
/*N*/ 
/*N*/ 				if((bWithBorder && fCmp > rPnt.X()) || (!bWithBorder && fCmp - rPnt.X() > -SMALL_DVALUE))
/*N*/ 					bInside = !bInside;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bInside;
/*N*/ }

/*************************************************************************
|*
|* FG: Identisch wie oben, das Polygon herausgeben
|*
\************************************************************************/

/*N*/ Polygon Polygon3D::GetPolygon() const
/*N*/ {
/*N*/ 	BOOL bClosed = IsClosed();
/*N*/ 	UINT16 nSize = pImpPolygon3D->nPoints;
/*N*/ 	if(bClosed)
/*N*/ 		nSize++;
/*N*/ 	Polygon aPolygon(nSize);
/*N*/ 	Vector3D* pVec3D = pImpPolygon3D->pPointAry;
/*N*/ 
/*N*/ 	if (pVec3D)
/*N*/ 	{
/*N*/ 		UINT16 i;
/*N*/ 		for (i = 0; i < pImpPolygon3D->nPoints; i++)
/*N*/ 		{
/*N*/ 			// X und Y uebernehmen, Z vernachlaessigen
/*N*/ 			aPolygon.SetPoint(Point((long)  pVec3D[i].X(),
/*N*/ 									(long) -pVec3D[i].Y()), i);
/*N*/ 		}
/*N*/ 		if(bClosed)
/*N*/ 			aPolygon.SetPoint(Point((long)  pVec3D[0].X(),
/*N*/ 									(long) -pVec3D[0].Y()), i);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for (UINT16 i = 0; i < nSize; i++)
/*N*/ 		{
/*?*/ 			aPolygon.SetPoint(Point(), i);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aPolygon.SetSize(nSize);
/*N*/ 	return(aPolygon);
/*N*/ }

/*************************************************************************
|*
|* Ausdehnung des Polygons ermitteln
|*
\************************************************************************/

/*N*/ Volume3D Polygon3D::GetPolySize() const
/*N*/ {
/*N*/ 	Volume3D aRetval;
/*N*/ 	UINT16 nPntCnt = pImpPolygon3D->nPoints;
/*N*/ 
/*N*/ 	aRetval.Reset();
/*N*/ 	for(UINT16 i = 0; i < nPntCnt; i++)
/*N*/ 		aRetval.Union((*this)[i]);
/*N*/ 
/*N*/ 	return aRetval;
/*N*/ }

/*************************************************************************
|*
|* Flaeche des Polygons ermitteln
|*
\************************************************************************/


/*N*/ double Polygon3D::GetPolyArea(const Vector3D& rNormal) const
/*N*/ {
/*N*/ 	double fRetval = 0.0;
/*N*/ 	UINT16 nPntCnt = pImpPolygon3D->nPoints;
/*N*/ 
/*N*/ 	if(nPntCnt > 2)
/*N*/ 	{
/*N*/ 		const Vector3D& rFirst = (*this)[0];
/*N*/ 		Vector3D aLastVector = (*this)[1] - rFirst;
/*N*/ 
/*N*/ 		for (UINT16 i = 2; i < nPntCnt; i++)
/*N*/ 		{
/*N*/ 			const Vector3D& rPoint = (*this)[i];
/*N*/ 			Vector3D aNewVec = rPoint - rFirst;
/*N*/ 			Vector3D aArea = aLastVector;
/*N*/ 			aArea |= aNewVec;
/*N*/ 			fRetval += (rNormal.Scalar(aArea)) / 2.0;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return fabs(fRetval);
/*N*/ }

/*************************************************************************
|*
|* Schnitt zwischen den von den Punkten angegebenen Kanten ermitteln.
|* Dabei ist der Rueckgabewert != 0.0, wenn der Schnitt innerhalb
|* der Parameterbereiche der Kanten liegt und gibt den Wert ]0.0, 1.0]
|* innerhalb der ersten Kante an.
|*
\************************************************************************/


/*************************************************************************
|*
|* Diese Version arbeitet mit der Kante nEdge1 aus dem lokalen
|* Polygon und nEdge2 aus dem uebergebenen
|*
\************************************************************************/


/*************************************************************************
|*
|* test if point is on line in range ]0.0..1.0[ without
|* the points. If so, return TRUE and put the parameter
|* value in pCut (if provided)
|*
\************************************************************************/


/*************************************************************************
|*
|* Diese Version nimmt die Startpunkte und Vektoren (relative Angabe
|* des Endpunktes) zweier Kanten
|*
\************************************************************************/


/*************************************************************************
|*
|* Orientierung im Punkt nIndex liefern
|*
\************************************************************************/

/*N*/ BOOL Polygon3D::GetPointOrientation(UINT16 nIndex) const
/*N*/ {
/*N*/ 	UINT16 nPntCnt = pImpPolygon3D->nPoints;
/*N*/ 	BOOL bRetval(TRUE);
/*N*/ 
/*N*/ 	if(nIndex < nPntCnt)
/*N*/ 	{
/*N*/ 		const Vector3D& rMid = (*this)[nIndex];
/*N*/ 		const Vector3D& rPre = (*this)[(nIndex == 0) ? nPntCnt-1 : nIndex-1];
/*N*/ 		const Vector3D& rPos = (*this)[(nIndex == nPntCnt-1) ? 0 : nIndex+1];
/*N*/ 
/*N*/ 		Vector3D aNormal = (rPre - rMid)|(rPos - rMid);
/*N*/ 		bRetval = (aNormal.Z() > 0.0) ? TRUE : FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return bRetval;
/*N*/ }

/*************************************************************************
|*
|* get position on polypos, with clipping to start/end
|*
\************************************************************************/
 
/*N*/ Vector3D Polygon3D::GetPosition(double fPos) const
/*N*/ {
/*N*/ 	Vector3D aRetval((*this)[0]);
/*N*/ 
/*N*/ 	if(fPos <= 0.0 || pImpPolygon3D->nPoints < 2)
/*N*/ 		return aRetval;
/*N*/ 
/*N*/ 	double fLen = GetLength();
/*N*/ 
/*N*/ 	if(fPos >= fLen)
/*N*/ 	{
/*N*/ 		aRetval = (*this)[pImpPolygon3D->nPoints - 1];
/*N*/ 		return aRetval;
/*N*/ 	}
/*N*/ 
/*N*/ 	UINT16 nPos(0);
/*N*/ 	Vector3D aPart((*this)[0] - (*this)[1]);
/*N*/ 	double fPartLen = aPart.GetLength();
/*N*/ 
/*N*/ 	while(fPos > fPartLen)
/*N*/ 	{
/*N*/ 		fPos -= fPartLen;
/*N*/ 		nPos++;
/*N*/ 		aPart = Vector3D((*this)[nPos] - (*this)[nPos+1]);
/*N*/ 		fPartLen = aPart.GetLength();
/*N*/ 	}
/*N*/ 
/*N*/ 	aRetval.CalcInBetween((*this)[nPos], (*this)[nPos+1], fPos / fPartLen);
/*N*/ 
/*N*/ 	return aRetval;
/*N*/ }

//////////////////////////////////////////////////////////////////////////////
// create a expanded or compresssed poly with exactly nNum Points
//
/*N*/ Polygon3D Polygon3D::GetExpandedPolygon(sal_uInt32 nNum)
/*N*/ {
/*N*/ 	if(GetPointCount() && nNum && (sal_uInt32)GetPointCount() != nNum)
/*N*/ 	{
/*N*/ 		Polygon3D aDestPoly((sal_uInt16)nNum);
/*N*/ 
/*N*/ 		// length of step in dest poly
/*N*/ 		double fStep = GetLength() / (double)(IsClosed() ? nNum : nNum - 1);
/*N*/ 
/*N*/ 		// actual positions for run
/*N*/ 		double fDestPos = 0.0;
/*N*/ 		double fSrcPos = 0.0;
/*N*/ 
/*N*/ 		// actual indices for run
/*N*/ 		sal_uInt32 nSrcPos = 0;
/*N*/ 		sal_uInt32 nSrcPosNext = (nSrcPos+1 == (sal_uInt32)GetPointCount()) ? 0 : nSrcPos + 1;
/*N*/ 
/*N*/ 		// length of next source step
/*N*/ 		double fNextSrcLen = ((*this)[(sal_uInt16)nSrcPos] - (*this)[(sal_uInt16)nSrcPosNext]).GetLength();
/*N*/ 
/*N*/ 		for(sal_uInt32 b = 0; b < nNum; b++)
/*N*/ 		{
/*N*/ 			// calc fDestPos in source
/*N*/ 			while(fSrcPos + fNextSrcLen < fDestPos)
/*N*/ 			{
/*N*/ 				fSrcPos += fNextSrcLen;
/*N*/ 				nSrcPos++;
/*N*/ 				nSrcPosNext = (nSrcPos+1 == (sal_uInt32)GetPointCount()) ? 0 : nSrcPos + 1;
/*N*/ 				fNextSrcLen = ((*this)[(sal_uInt16)nSrcPos] - (*this)[(sal_uInt16)nSrcPosNext]).GetLength();
/*N*/ 			}
/*N*/ 
/*N*/ 			// fDestPos is between fSrcPos and (fSrcPos + fNextSrcLen)
/*N*/ 			double fLenA = (fDestPos - fSrcPos) / fNextSrcLen;
/*N*/ 			Vector3D aOld1 = (*this)[(sal_uInt16)nSrcPos];
/*N*/ 			Vector3D aOld2 = (*this)[(sal_uInt16)nSrcPosNext];
/*N*/ 			Vector3D aNewPoint;
/*N*/ 			aNewPoint.CalcInBetween(aOld1, aOld2, fLenA);
/*N*/ 
/*N*/ 			aDestPoly[(sal_uInt16)b] = aNewPoint;
/*N*/ 
/*N*/ 			// next step
/*N*/ 			fDestPos += fStep;
/*N*/ 		}
/*N*/ 
/*N*/ 		if(aDestPoly.GetPointCount() >= 3)
/*N*/ 			aDestPoly.SetClosed(IsClosed());
/*N*/ 
/*N*/ 		return aDestPoly;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return *this;
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+--------------- ImpPolyPolygon3D -------------------------------------+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*************************************************************************
|*
|* Copy-Konstruktor
|*
\************************************************************************/

/*N*/ ImpPolyPolygon3D::ImpPolyPolygon3D(const ImpPolyPolygon3D& rImpPolyPoly3D) :
/*N*/ 	aPoly3DList(rImpPolyPoly3D.aPoly3DList)
/*N*/ {
/*N*/ 	nRefCount = 1;
/*N*/ 	// Einzelne Elemente duplizieren
/*N*/ 	Polygon3D* pPoly3D = aPoly3DList.First();
/*N*/ 
/*N*/ 	while ( pPoly3D )
/*N*/ 	{
/*N*/ 		aPoly3DList.Replace(new Polygon3D(*(aPoly3DList.GetCurObject())));
/*N*/ 		pPoly3D = aPoly3DList.Next();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*N*/ ImpPolyPolygon3D::~ImpPolyPolygon3D()
/*N*/ {
/*N*/ 	Polygon3D* pPoly3D = aPoly3DList.First();
/*N*/ 
/*N*/ 	while( pPoly3D )
/*N*/ 	{
/*N*/ 		delete pPoly3D;
/*N*/ 		pPoly3D = aPoly3DList.Next();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Gleichheitsoperator
|*
\************************************************************************/

/*N*/ BOOL ImpPolyPolygon3D::operator==(const ImpPolyPolygon3D& rImpPolyPoly3D) const
/*N*/ {
/*N*/ 	UINT16 nCnt = (UINT16) aPoly3DList.Count();
/*N*/ 	const Polygon3DList& rCmpList = rImpPolyPoly3D.aPoly3DList;
/*N*/ 
/*N*/ 	if ( nCnt != (UINT16) rCmpList.Count() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	BOOL bEqual = TRUE;
/*N*/ 
/*N*/ 	for ( UINT16 i = 0; i < nCnt && bEqual; i++ )
/*N*/ 		bEqual = ( *aPoly3DList.GetObject(i) == *rCmpList.GetObject(i) );
/*N*/ 
/*N*/ 	return bEqual;
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+--------------- PolyPolygon3D ----------------------------------------+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ PolyPolygon3D::PolyPolygon3D(UINT16 nInitSize, UINT16 nResize)
/*N*/ {
/*N*/ 	DBG_CTOR(PolyPolygon3D, NULL);
/*N*/ 	pImpPolyPolygon3D = new ImpPolyPolygon3D(nInitSize, nResize);
/*N*/ }


/*************************************************************************
|*
|* Konstruktor mit Polygon3D
|*
\************************************************************************/

/*N*/ PolyPolygon3D::PolyPolygon3D(const Polygon3D& rPoly3D)
/*N*/ {
/*N*/ 	DBG_CTOR(PolyPolygon3D, NULL);
/*N*/ 	pImpPolyPolygon3D = new ImpPolyPolygon3D;
/*N*/ 	pImpPolyPolygon3D->aPoly3DList.Insert(new Polygon3D(rPoly3D));
/*N*/ }

/*************************************************************************
|*
|* Konstruktor mit PolyPolygon3D
|*
\************************************************************************/

/*N*/ PolyPolygon3D::PolyPolygon3D(const PolyPolygon3D& rPolyPoly3D)
/*N*/ {
/*N*/ 	DBG_CTOR(PolyPolygon3D,NULL);
/*N*/ 	pImpPolyPolygon3D = rPolyPoly3D.pImpPolyPolygon3D;
/*N*/ 	pImpPolyPolygon3D->nRefCount++;
/*N*/ }

/*************************************************************************
|*
|* Konstruktor mit SV-PolyPolygon
|*
\************************************************************************/

/*N*/ PolyPolygon3D::PolyPolygon3D(const PolyPolygon& rPolyPoly, double fScale)
/*N*/ {
/*N*/ 	DBG_CTOR(PolyPolygon3D, NULL);
/*N*/ 	pImpPolyPolygon3D = new ImpPolyPolygon3D;
/*N*/ 	UINT16 nCnt = rPolyPoly.Count();
/*N*/ 
/*N*/ 	for ( UINT16 i = 0; i < nCnt; i++ )
/*N*/ 		pImpPolyPolygon3D->aPoly3DList.Insert(
/*N*/ 			new Polygon3D(rPolyPoly.GetObject(i), fScale), LIST_APPEND);
/*N*/ }

/*************************************************************************
|*
|* Konstruktor mit XPolyPolygon
|*
\************************************************************************/

/*N*/ PolyPolygon3D::PolyPolygon3D(const XPolyPolygon& rXPolyPoly, double fScale)
/*N*/ {
/*N*/ 	DBG_CTOR(PolyPolygon3D, NULL);
/*N*/ 	pImpPolyPolygon3D = new ImpPolyPolygon3D;
/*N*/ 	UINT16 nCnt = rXPolyPoly.Count();
/*N*/ 
/*N*/ 	for ( UINT16 i = 0; i < nCnt; i++ )
/*N*/ 		pImpPolyPolygon3D->aPoly3DList.Insert(
/*N*/ 			new Polygon3D(rXPolyPoly.GetObject(i), fScale), LIST_APPEND);
/*N*/ }

/*************************************************************************
|*
|* Closed-Status der Einzelpolygone korrigieren
|*
\************************************************************************/


/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*N*/ PolyPolygon3D::~PolyPolygon3D()
/*N*/ {
/*N*/ 	DBG_DTOR(PolyPolygon3D, NULL);
/*N*/ 
/*N*/ 	if( pImpPolyPolygon3D->nRefCount > 1 )	pImpPolyPolygon3D->nRefCount--;
/*N*/ 	else									delete pImpPolyPolygon3D;
/*N*/ }

/*************************************************************************
|*
|* Feststellen, ob ein Punkt innerhalb liegt
|*
\************************************************************************/



/*************************************************************************
|*
|* Scnittpunkt angegebene Linie mit Polygon
|*
\************************************************************************/


/*************************************************************************
|*
|* Referenzzaehler pruefen und ggf. neuen Container erzeugen
|*
\************************************************************************/

/*N*/ void PolyPolygon3D::CheckReference()
/*N*/ {
/*N*/ 	if( pImpPolyPolygon3D->nRefCount > 1 )
/*N*/ 	{
/*N*/ 		pImpPolyPolygon3D->nRefCount--;
/*N*/ 		pImpPolyPolygon3D = new ImpPolyPolygon3D(*pImpPolyPolygon3D);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Polygon in die Liste einfuegen
|*
\************************************************************************/

/*N*/ void PolyPolygon3D::Insert(const Polygon3D& rPoly3D, UINT16 nPos)
/*N*/ {
/*N*/ 	CheckReference();
/*N*/ 	pImpPolyPolygon3D->aPoly3DList.Insert(new Polygon3D(rPoly3D), nPos);
/*N*/ }

/*************************************************************************
|*
|* PolyPolygon in die Liste einfuegen
|*
\************************************************************************/

/*N*/ void PolyPolygon3D::Insert(const PolyPolygon3D& rPolyPoly3D, UINT16 nPos)
/*N*/ {
/*N*/ 	CheckReference();
/*N*/ 	UINT16 nCnt = rPolyPoly3D.Count();
/*N*/ 
/*N*/ 	for ( UINT16 i = 0; i < nCnt; i++)
/*N*/ 	{
/*N*/ 		Polygon3D* pPoly3D = new Polygon3D(rPolyPoly3D[i]);
/*N*/ 		pImpPolyPolygon3D->aPoly3DList. Insert(pPoly3D, nPos);
/*N*/ 
/*N*/ 		if ( nPos != POLYPOLY3D_APPEND )
/*N*/ 			nPos++;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Polygon aus der Liste entfernen
|*
\************************************************************************/



/*************************************************************************
|*
|* Polygon in der Liste ersetzen
|*
\************************************************************************/


/*************************************************************************
|*
|* Referenz auf Polygon zurueckgeben, ggf. neues Polygon einfuegen
|*
\************************************************************************/

/*N*/ const Polygon3D& PolyPolygon3D::GetObject(UINT16 nPos) const
/*N*/ {
/*N*/ 	Polygon3D* pPoly3D = pImpPolyPolygon3D->aPoly3DList.GetObject(nPos);
/*N*/ 
/*N*/ 	if ( pPoly3D == NULL )
/*N*/ 	{	// Wenn noch kein Polygon an der Position vorhanden, neu erzeugen
/*N*/ 		pPoly3D = new Polygon3D;
/*N*/ 		pImpPolyPolygon3D->aPoly3DList.Insert(pPoly3D, nPos);
/*N*/ 	}
/*N*/ 	return *pPoly3D;
/*N*/ }

/*************************************************************************
|*
|* Liste loeschen
|*
\************************************************************************/

/*N*/ void PolyPolygon3D::Clear()
/*N*/ {
/*N*/ 	if ( pImpPolyPolygon3D->nRefCount > 1 )
/*N*/ 	{
/*?*/ 		pImpPolyPolygon3D->nRefCount--;
/*?*/ 		pImpPolyPolygon3D = new ImpPolyPolygon3D();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		Polygon3D* pPoly3D = pImpPolyPolygon3D->aPoly3DList.First();
/*N*/ 
/*N*/ 		while( pPoly3D )
/*N*/ 		{
/*N*/ 			delete pPoly3D;
/*N*/ 			pPoly3D = pImpPolyPolygon3D->aPoly3DList.Next();
/*N*/ 		}
/*N*/ 		pImpPolyPolygon3D->aPoly3DList.Clear();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Anzahl der Polygone zurueckgeben
|*
\************************************************************************/

/*N*/ UINT16 PolyPolygon3D::Count() const
/*N*/ {
/*N*/ 	return (UINT16)(pImpPolyPolygon3D->aPoly3DList.Count());
/*N*/ }

/*************************************************************************
|*
|* Arrayoperator
|*
\************************************************************************/

/*N*/ Polygon3D& PolyPolygon3D::operator[](UINT16 nPos)
/*N*/ {
/*N*/ 	CheckReference();
/*N*/ 	Polygon3D* pPoly3D = pImpPolyPolygon3D->aPoly3DList.GetObject(nPos);
/*N*/ 
/*N*/ 	if ( pPoly3D == NULL )
/*N*/ 	{	// Wenn noch kein Polygon an der Position vorhanden, neu erzeugen
/*N*/ 		pPoly3D = new Polygon3D;
/*N*/ 		pImpPolyPolygon3D->aPoly3DList.Insert(pPoly3D, nPos);
/*N*/ 	}
/*N*/ 	return *pPoly3D;
/*N*/ }

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/

/*N*/ PolyPolygon3D& PolyPolygon3D::operator=(const PolyPolygon3D& rPolyPoly3D)
/*N*/ {
/*N*/ 	rPolyPoly3D.pImpPolyPolygon3D->nRefCount++;
/*N*/ 
/*N*/ 	if( pImpPolyPolygon3D->nRefCount > 1 )	pImpPolyPolygon3D->nRefCount--;
/*N*/ 	else									delete pImpPolyPolygon3D;
/*N*/ 
/*N*/ 	pImpPolyPolygon3D = rPolyPoly3D.pImpPolyPolygon3D;
/*N*/ 	return *this;
/*N*/ }

/*************************************************************************
|*
|* Gleichheitsoperator
|*
\************************************************************************/


/*************************************************************************
|*
|* Ungleichheitsoperator
|*
\************************************************************************/

/*N*/ BOOL PolyPolygon3D::operator!=(const PolyPolygon3D& rPolyPoly3D) const
/*N*/ {
/*N*/ 	if ( pImpPolyPolygon3D == rPolyPoly3D.pImpPolyPolygon3D )
/*N*/ 		return FALSE;
/*N*/ 	else
/*N*/ 		return *pImpPolyPolygon3D != *rPolyPoly3D.pImpPolyPolygon3D;
/*N*/ }

/*************************************************************************
|*
|* alle Polygone mit uebergebener Matrix transformieren
|*
\************************************************************************/

/*N*/ void PolyPolygon3D::Transform(const Matrix4D& rTfMatrix)
/*N*/ {
/*N*/ 	CheckReference();
/*N*/ 	UINT16 nCnt = Count();
/*N*/ 
/*N*/ 	for ( UINT16 i = 0; i < nCnt; i++ )
/*N*/ 		pImpPolyPolygon3D->aPoly3DList.GetObject(i)->Transform(rTfMatrix);
/*N*/ }

/*************************************************************************
|*
|* Die Umlaufrichtungen ineinanderliegender Polygone so anpassen, dass
|* sie wechseln; Ausserdem wird ggf. das Polygon mit der geometrisch
|* betrachtet aeussersten Kontur an den Anfang der Liste verschoben
|* werden, damit Normalenberechnungen immer anhand des ersten Polygons
|* ausgefuehrt werden koennen.
|*
\************************************************************************/

/*N*/ void PolyPolygon3D::SetDirections(const Vector3D& rNormal)
/*N*/ {
/*N*/ 	CheckReference();
/*N*/ 
/*N*/ 	UINT16	nCnt = Count();
/*N*/ 	UINT16	nPoly;
/*N*/ 	short	nFirstPoly = -1;
/*N*/ 
/*N*/ 	if(nCnt)
/*N*/ 	{
/*N*/ 		for ( nPoly = 0; nPoly < nCnt; nPoly++ )
/*N*/ 		{
/*N*/ 			Polygon3D& rPoly3D = *pImpPolyPolygon3D->aPoly3DList.GetObject(nPoly);
/*N*/ 			BOOL bFlip = !rPoly3D.IsClockwise(rNormal);
/*N*/ 			short nDepth = 0;
/*N*/ 			const Vector3D& rPos = rPoly3D[0];
/*N*/ 
/*N*/ 			// bestimmen, wie tief das aktuelle Polygon in den anderen
/*N*/ 			// verschachtelt ist
/*N*/ 			for ( UINT16 i = 0; i < nCnt; i++ )
/*N*/ 			{
/*N*/ 				if ( i != nPoly &&
/*N*/ 					 pImpPolyPolygon3D->aPoly3DList.GetObject(i)->IsInside(rPos) )
/*N*/ 						nDepth++;
/*N*/ 			}
/*N*/ 			// ungerade nDepth-Werte bedeuten: das  Polygon ist ein "Loch"
/*N*/ 			// in der aeusseren Kontur
/*N*/ 			BOOL bHole = ((nDepth & 0x0001) == 1);
/*N*/ 
/*N*/ 			if ( (bFlip && !bHole) || (!bFlip && bHole) )
/*N*/ 				rPoly3D.FlipDirection();
/*N*/ 
/*N*/ 			// Den Index des (geometrisch) aeussersten Polygons merken
/*N*/ 			if ( nDepth == 0 && nFirstPoly == -1 )
/*N*/ 				nFirstPoly = (short) nPoly;
/*N*/ 		}
/*N*/ 		// liegt das aeussere Polygon nicht am Anfang, wird es dahin verschoben
/*N*/ 		if ( nFirstPoly > 0 )
/*N*/ 		{
/*N*/ 			Polygon3D* pOuter = pImpPolyPolygon3D->aPoly3DList.Remove(nFirstPoly);
/*N*/ 			pImpPolyPolygon3D->aPoly3DList.Insert(pOuter, (ULONG) 0);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* in allen Polygonen doppelte aufeinanderfolgende Polygonpunkte
|* entfernen; ein geschlossenes Polygon wird dabei ggf. "geoeffnet"
|*
\************************************************************************/

/*N*/ void PolyPolygon3D::RemoveDoublePoints()
/*N*/ {
/*N*/ 	CheckReference();
/*N*/ 	UINT16 nCnt = Count();
/*N*/ 
/*N*/ 	for ( UINT16 i = 0; i < nCnt; i++ )
/*N*/ 		pImpPolyPolygon3D->aPoly3DList.GetObject(i)->RemoveDoublePoints();
/*N*/ }

/*************************************************************************
|*
|* Ein mittels GrowPoly() geschrumpftes PolyPolygon in eventuell
|* entstandenen Selbstueberschneidungen in Eckpunkten ohne
|* Punktreduzierung korrigieren
|*
\************************************************************************/

/*N*/ void PolyPolygon3D::CorrectGrownPoly(const PolyPolygon3D& rPolyOrig)
/*N*/ {
/*N*/ 	if(Count() == rPolyOrig.Count())
/*N*/ 	{
/*N*/ 		for(UINT16 a=0;a<Count();a++)
/*N*/ 		{
/*N*/ 			const Polygon3D& rOrig = rPolyOrig[a];
/*N*/ 			Polygon3D& rPoly = (*this)[a];
/*N*/ 			UINT16 nPntCnt = rOrig.GetPointCount();
/*N*/ 
/*N*/ 			if(nPntCnt == rPoly.GetPointCount() && nPntCnt > 2)
/*N*/ 			{
/*N*/ 				UINT16 nNumDiff(0);
/*N*/ 				UINT16 nDoneStart(0xffff);
/*N*/ 
/*N*/ 				// Testen auf Anzahl Aenderungen
/*N*/ 				UINT16 b;
/*N*/ 				for(b=0;b<nPntCnt;b++)
/*N*/ 				{
/*N*/ 					if(rOrig.GetPointOrientation(b) != rPoly.GetPointOrientation(b))
/*N*/ 					{
/*N*/ 						nNumDiff++;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						if(nDoneStart == 0xffff)
/*N*/ 						{
/*N*/ 							// eventuellen Startpunkt auf gleiche Orientierung legen
/*N*/ 							nDoneStart = b;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				if(nNumDiff == nPntCnt)
/*N*/ 				{
/*N*/ 					// Komplett umgedreht, alles auf einen Punkt
/*N*/ 					Vector3D aMiddle = rPoly.GetMiddle();
/*N*/ 
/*N*/ 					for(b=0;b<nPntCnt;b++)
/*N*/ 					{
/*N*/ 						rPoly[b] = aMiddle;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else if(nNumDiff)
/*N*/ 				{
/*N*/ 					// es gibt welche, nDoneStart ist gesetzt. Erzeuge (und
/*N*/ 					// setze) nDoneEnd
/*N*/ 					UINT16 nDoneEnd(nDoneStart);
/*N*/ 					UINT16 nStartLoop;
/*N*/ 					BOOL bInLoop(FALSE);
/*N*/ 
/*N*/ 					// einen step mehr in der Schleife, um Loops abzuschliessen
/*N*/ 					BOOL bFirstStep(TRUE);
/*N*/ 
/*N*/ 					while(nDoneEnd != nDoneStart || bFirstStep)
/*N*/ 					{
/*N*/ 						bFirstStep = FALSE;
/*N*/ 
/*N*/ 						// nCandidate ist Kandidat fuer Test
/*N*/ 						UINT16 nCandidate = (nDoneEnd == nPntCnt-1) ? 0 : nDoneEnd+1;
/*N*/ 
/*N*/ 						if(rOrig.GetPointOrientation(nCandidate) == rPoly.GetPointOrientation(nCandidate))
/*N*/ 						{
/*N*/ 							// Orientierung ist gleich
/*N*/ 							if(bInLoop)
/*N*/ 							{
/*N*/ 								// Punkte innerhalb bInLoop auf ihr Zentrum setzen
/*N*/ 								Vector3D aMiddle;
/*N*/ 								UINT16 nCounter(0);
/*N*/ 								UINT16 nStart(nStartLoop);
/*N*/ 
/*N*/ 								while(nStart != nCandidate)
/*N*/ 								{
/*N*/ 									aMiddle += rPoly[nStart];
/*N*/ 									nCounter++;
/*N*/ 									nStart = (nStart == nPntCnt-1) ? 0 : nStart+1;
/*N*/ 								}
/*N*/ 
/*N*/ 								// Mittelwert bilden
/*N*/ 								aMiddle /= (double)nCounter;
/*N*/ 
/*N*/ 								// Punkte umsetzen
/*N*/ 								nStart = nStartLoop;
/*N*/ 								while(nStart != nCandidate)
/*N*/ 								{
/*N*/ 									rPoly[nStart] = aMiddle;
/*N*/ 									nStart = (nStart == nPntCnt-1) ? 0 : nStart+1;
/*N*/ 								}
/*N*/ 
/*N*/ 								// Loop beenden
/*N*/ 								bInLoop = FALSE;
/*N*/ 							}
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							// Orientierung unterschiedlich
/*N*/ 							if(!bInLoop)
/*N*/ 							{
/*N*/ 								// Start eines Loop mit geaenderter Orientierung
/*N*/ 								nStartLoop = nCandidate;
/*N*/ 								bInLoop = TRUE;
/*N*/ 							}
/*N*/ 						}
/*N*/ 
/*N*/ 						// Weitergehen
/*N*/ 						nDoneEnd = nCandidate;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Ueberlappen sich das aktuelle und das angegebene PolyPolygon (in X/Y) ?
|*
\************************************************************************/



/*************************************************************************
|*
|* Remove all completely overlapping polygons
|*
\************************************************************************/
 

/*************************************************************************
|*
|* aus Stream laden
|*
\************************************************************************/

/*N*/ SvStream& operator>>(SvStream& rIStream, PolyPolygon3D& rPolyPoly3D)
/*N*/ {
/*N*/ 	DBG_CHKOBJ(&rPolyPoly3D, PolyPolygon3D, NULL);
/*N*/ 
/*N*/ 	Polygon3D* pPoly3D;
/*N*/ 
/*N*/ 	// Anzahl der Polygone einlesen
/*N*/ 	UINT16 nPolyCount;
/*N*/ 	rIStream >> nPolyCount;
/*N*/ 
/*N*/ 	BOOL bTruncated = FALSE;
/*N*/ 	// Gesamtanzahl der Punkte mitzaehlen
/*N*/ 	ULONG nAllPointCount = 0;
/*N*/ 
/*N*/ 	if ( rPolyPoly3D.pImpPolyPolygon3D->nRefCount > 1 )
/*?*/ 		rPolyPoly3D.pImpPolyPolygon3D->nRefCount--;
/*N*/ 	else
/*N*/ 		delete rPolyPoly3D.pImpPolyPolygon3D;
/*N*/ 
/*N*/ 	rPolyPoly3D.pImpPolyPolygon3D = new ImpPolyPolygon3D(nPolyCount);
/*N*/ 
/*N*/ 	while ( nPolyCount > 0 )
/*N*/ 	{
/*N*/ 		pPoly3D = new Polygon3D;
/*N*/ 		rIStream >> *pPoly3D;
/*N*/ 		nAllPointCount += pPoly3D->GetPointCount();
/*N*/ 
/*N*/ 		if ( !bTruncated )
/*N*/ 		{
/*N*/ 			if ( nAllPointCount > POLY3D_MAXPOINTS )
/*N*/ 			{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 			}
/*N*/ 			rPolyPoly3D.pImpPolyPolygon3D->aPoly3DList.Insert(pPoly3D, LIST_APPEND);
/*N*/ 		}
/*N*/ 		else
/*?*/ 			delete pPoly3D;
/*N*/ 
/*N*/ 		nPolyCount--;
/*N*/ 	}
/*N*/ 
/*N*/ 	return rIStream;
/*N*/ }

/*************************************************************************
|*
|* in Stream speichern
|*
\************************************************************************/

/*N*/ SvStream& operator<<(SvStream& rOStream, const PolyPolygon3D& rPolyPoly3D)
/*N*/ {
/*N*/ 	DBG_CHKOBJ(&rPolyPoly3D, PolyPolygon3D, NULL);
/*N*/ 
/*N*/ 	// Anzahl der Polygone rausschreiben
/*N*/ 	rOStream << rPolyPoly3D.Count();
/*N*/ 
/*N*/ 	// Die einzelnen Polygone ausgeben
/*N*/ 	Polygon3D* pPoly3D = rPolyPoly3D.pImpPolyPolygon3D->aPoly3DList.First();
/*N*/ 
/*N*/ 	while( pPoly3D )
/*N*/ 	{
/*N*/ 		rOStream << *pPoly3D;
/*N*/ 		pPoly3D = rPolyPoly3D.pImpPolyPolygon3D->aPoly3DList.Next();
/*N*/ 	}
/*N*/ 
/*N*/ 	return rOStream;
/*N*/ }

/*N*/ Vector3D PolyPolygon3D::GetNormal() const
/*N*/ {
/*N*/ 	if(pImpPolyPolygon3D->aPoly3DList.Count())
/*N*/ 		return (*this)[0].GetNormal();
/*N*/ 	return Vector3D(0.0, 0.0, -1.0);
/*N*/ }

/*************************************************************************
|*
|* die Umlaufrichtung des ersten Polygons umkehren
|*
\************************************************************************/

/*N*/ void PolyPolygon3D::FlipDirections()
/*N*/ {
/*N*/ 	CheckReference();
/*N*/ 	UINT16 nCnt = Count();
/*N*/ 
/*N*/ 	for ( UINT16 i = 0; i < nCnt; i++ )
/*N*/ 		pImpPolyPolygon3D->aPoly3DList.GetObject(i)->FlipDirection();
/*N*/ }

/*N*/ Vector3D PolyPolygon3D::GetMiddle() const
/*N*/ {
/*N*/ 	if(pImpPolyPolygon3D->aPoly3DList.Count())
/*N*/ 		return (*this)[0].GetMiddle();
/*N*/ 	return Vector3D();
/*N*/ }

/*N*/ BOOL PolyPolygon3D::IsClosed() const
/*N*/ {
/*N*/ 	BOOL bClosed = TRUE;
/*N*/ 	UINT16 nCnt = Count();
/*N*/ 
/*N*/ 	for ( UINT16 i = 0; i < nCnt; i++ )
/*N*/ 		if(!pImpPolyPolygon3D->aPoly3DList.GetObject(i)->IsClosed())
/*N*/ 			bClosed = FALSE;
/*N*/ 	return bClosed;
/*N*/ }

/*************************************************************************
|*
|* export PolyPolygon
|*
\************************************************************************/

/*N*/ PolyPolygon PolyPolygon3D::GetPolyPolygon() const
/*N*/ {
/*N*/ 	PolyPolygon aPolyPolygon;
/*N*/ 	UINT16 nCnt = Count();
/*N*/ 
/*N*/ 	for(UINT16 i=0; i<nCnt;i++)
/*N*/ 		aPolyPolygon.Insert((*this)[i].GetPolygon());
/*N*/ 
/*N*/ 	return(aPolyPolygon);
/*N*/ }

/*************************************************************************
|*
|* Ausdehnung ermitteln
|*
\************************************************************************/

/*N*/ Volume3D PolyPolygon3D::GetPolySize() const
/*N*/ {
/*N*/ 	UINT16 nCnt = Count();
/*N*/ 	Volume3D aRetval;
/*N*/ 	Volume3D aSubVolume;
/*N*/ 
/*N*/ 	aRetval.Reset();
/*N*/ 	for ( UINT16 i = 0; i < nCnt; i++ )
/*N*/ 	{
/*N*/ 		aSubVolume = (*this)[i].GetPolySize();
/*N*/ 		aRetval.Union(aSubVolume);
/*N*/ 	}
/*N*/ 
/*N*/ 	return aRetval;
/*N*/ }

/*************************************************************************
|*
|* Flaeche des Polygons ermitteln
|*
\************************************************************************/

/*N*/ double PolyPolygon3D::GetPolyArea() const
/*N*/ {
/*N*/ 	UINT16 nCnt = Count();
/*N*/ 	double fRetval = 0.0;
/*N*/ 
/*N*/ 	// Einzelflaechen aufsummieren
/*N*/ 	Vector3D aNormal = GetNormal();
/*N*/ 	for ( UINT16 i = 0; i < nCnt; i++ )
/*N*/ 	{
/*N*/ 		if((*this)[i].IsClockwise(aNormal))
/*N*/ 		{
/*N*/ 			fRetval += (*this)[i].GetPolyArea(aNormal);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			fRetval -= (*this)[i].GetPolyArea(aNormal);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return fabs(fRetval);
/*N*/ }

/*************************************************************************
|*
|* Laenge des Polygons ermitteln
|*
\************************************************************************/

/*N*/ double PolyPolygon3D::GetLength() const
/*N*/ {
/*N*/ 	UINT16 nCnt = Count();
/*N*/ 	double fRetval = 0.0;
/*N*/ 
/*N*/ 	for ( UINT16 i = 0; i < nCnt; i++ )
/*N*/ 		fRetval += (*this)[i].GetLength();
/*N*/ 
/*N*/ 	return fRetval;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
