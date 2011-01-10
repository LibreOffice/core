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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <osl/endian.h>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/poly.hxx>

#include <svx/xpoly.hxx>
#include "xpolyimp.hxx"
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <vcl/salbtype.hxx>     // FRound
#include <basegfx/range/b2drange.hxx>
#include <basegfx/numeric/ftools.hxx>

#define GLOBALOVERFLOW

DBG_NAME(XPolygon);
DBG_NAME(XPolyPolygon);

/*************************************************************************
|*
|*    ImpXPolygon::ImpXPolygon()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  12.01.95 ESO
|*
*************************************************************************/

ImpXPolygon::ImpXPolygon( sal_uInt16 nInitSize, sal_uInt16 _nResize )
{
    pPointAry               = NULL;
    pFlagAry                = NULL;
    bDeleteOldPoints        = sal_False;
    nSize                   = 0;
    nResize                 = _nResize;
    nPoints                 = 0;
    nRefCount               = 1;

    Resize( nInitSize );
}

/*************************************************************************
|*
|*    ImpXPolygon::ImpXPolygon()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  12.01.95 ESO
|*
*************************************************************************/

ImpXPolygon::ImpXPolygon( const ImpXPolygon& rImpXPoly )
{
    ( (ImpXPolygon&) rImpXPoly ).CheckPointDelete();

    pPointAry               = NULL;
    pFlagAry                = NULL;
    bDeleteOldPoints        = sal_False;
    nSize                   = 0;
    ImpXPolygon::nResize    = rImpXPoly.nResize;
    nPoints                 = 0;
    nRefCount               = 1;

    Resize( rImpXPoly.nSize );

    // Kopieren
    nPoints = rImpXPoly.nPoints;
    memcpy( pPointAry, rImpXPoly.pPointAry, nSize*sizeof( Point ) );
    memcpy( pFlagAry, rImpXPoly.pFlagAry, nSize );
}

/*************************************************************************
|*
|*    ImpXPolygon::~ImpXPolygon()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  12.01.95 ESO
|*
*************************************************************************/

ImpXPolygon::~ImpXPolygon()
{
    delete[] (char*) pPointAry;
    delete[] pFlagAry;
    if ( bDeleteOldPoints )
        delete[] (char*) pOldPointAry;
}

/*************************************************************************
|*
|*    ImpXPolygon::operator==()
|*
|*    Ersterstellung    Joe 26-09-95
|*    Letzte Aenderung
|*
*************************************************************************/


bool ImpXPolygon::operator==(const ImpXPolygon& rImpXPoly) const
{
    return nPoints==rImpXPoly.nPoints &&
           (nPoints==0 ||
            (memcmp(pPointAry,rImpXPoly.pPointAry,nPoints*sizeof(Point))==0 &&
             memcmp(pFlagAry,rImpXPoly.pFlagAry,nPoints)==0));
}

/*************************************************************************
|*
|*    ImpXPolygon::Resize()
|*
|*    !!! Polygongroesse aendern - wenn bDeletePoints sal_False, dann den
|*    Point-Array nicht loeschen, sondern in pOldPointAry sichern und
|*    das Flag bDeleteOldPoints setzen. Beim naechsten Zugriff wird
|*    das Array dann geloescht.
|*    Damit wird verhindert, dass bei XPoly[n] = XPoly[0] durch ein
|*    Resize der fuer den rechten Ausdruck verwendete Point-Array
|*    vorzeitig geloescht wird.
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  12.01.95 ESO
|*
*************************************************************************/

void ImpXPolygon::Resize( sal_uInt16 nNewSize, sal_Bool bDeletePoints )
{
    if( nNewSize == nSize )
        return;

    sal_uInt8*   pOldFlagAry  = pFlagAry;
    sal_uInt16  nOldSize     = nSize;

    CheckPointDelete();
    pOldPointAry = pPointAry;

    // Neue Groesse auf vielfaches von nResize runden, sofern Objekt
    // nicht neu angelegt wurde (nSize != 0)
    if ( nSize != 0 && nNewSize > nSize )
    {
        DBG_ASSERT(nResize, "Resize-Versuch trotz nResize = 0 !");
        nNewSize = nSize + ((nNewSize-nSize-1) / nResize + 1) * nResize;
    }
    // Punkt Array erzeugen
    nSize     = nNewSize;
    pPointAry = (Point*)new char[ nSize*sizeof( Point ) ];
    memset( pPointAry, 0, nSize*sizeof( Point ) );

    // Flag Array erzeugen
    pFlagAry = new sal_uInt8[ nSize ];
    memset( pFlagAry, 0, nSize );

    // Eventuell umkopieren
    if( nOldSize )
    {
        if( nOldSize < nSize )
        {
            memcpy( pPointAry, pOldPointAry, nOldSize*sizeof( Point ) );
            memcpy( pFlagAry,  pOldFlagAry, nOldSize );
        }
        else
        {
            memcpy( pPointAry, pOldPointAry, nSize*sizeof( Point ) );
            memcpy( pFlagAry, pOldFlagAry, nSize );

            // Anzahl der gueltigen Punkte anpassen
            if( nPoints > nSize )
                nPoints = nSize;
        }
        if ( bDeletePoints )    delete[] (char*) pOldPointAry;
        else                    bDeleteOldPoints = sal_True;
        delete[] pOldFlagAry;
    }
}


/*************************************************************************
|*
|*    ImpXPolygon::InsertSpace()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  29.03.95 ESO
|*
*************************************************************************/

void ImpXPolygon::InsertSpace( sal_uInt16 nPos, sal_uInt16 nCount )
{
    CheckPointDelete();

    if ( nPos > nPoints )
        nPos = nPoints;

    // Wenn Polygon zu klein dann groesser machen
    if( (nPoints + nCount) > nSize )
        Resize( nPoints + nCount );

    // Wenn nicht hinter dem letzten Punkt eingefuegt wurde,
    // den Rest nach hinten schieben
    if( nPos < nPoints )
    {
        sal_uInt16 nMove = nPoints - nPos;
        memmove( &pPointAry[nPos+nCount], &pPointAry[nPos],
                 nMove * sizeof(Point) );
        memmove( &pFlagAry[nPos+nCount], &pFlagAry[nPos], nMove );
    }
    memset( &pPointAry[nPos], 0, nCount * sizeof( Point ) );
    memset( &pFlagAry [nPos], 0, nCount );

    nPoints = nPoints + nCount;
}


/*************************************************************************
|*
|*    ImpXPolygon::Remove()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  12.01.94 ESO
|*
*************************************************************************/

void ImpXPolygon::Remove( sal_uInt16 nPos, sal_uInt16 nCount )
{
    CheckPointDelete();

    if( (nPos + nCount) <= nPoints )
    {
        sal_uInt16 nMove = nPoints - nPos - nCount;

        if( nMove )
        {
            memmove( &pPointAry[nPos], &pPointAry[nPos+nCount],
                     nMove * sizeof(Point) );
            memmove( &pFlagAry[nPos], &pFlagAry[nPos+nCount], nMove );
        }
        memset( &pPointAry[nPoints - nCount], 0, nCount * sizeof( Point ) );
        memset( &pFlagAry [nPoints - nCount], 0, nCount );
        nPoints = nPoints - nCount;
    }
}


/*************************************************************************
|*
|*    XPolygon::XPolygon()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  08.11.94
|*
*************************************************************************/

XPolygon::XPolygon( sal_uInt16 nSize, sal_uInt16 nResize )
{
    DBG_CTOR(XPolygon,NULL);
    pImpXPolygon = new ImpXPolygon( nSize, nResize );
}

/*************************************************************************
|*
|*    XPolygon::XPolygon()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  08.11.94
|*
*************************************************************************/

XPolygon::XPolygon( const XPolygon& rXPoly )
{
    DBG_CTOR(XPolygon,NULL);
    pImpXPolygon = rXPoly.pImpXPolygon;
    pImpXPolygon->nRefCount++;
}

/*************************************************************************
|*
|*    XPolygon::XPolygon()
|*
|*    XPolygon aus einem Standardpolygon erstellen
|*    Ersterstellung    18.01.95 ESO
|*    Letzte Aenderung  18.01.95 ESO
|*
*************************************************************************/

XPolygon::XPolygon( const Polygon& rPoly )
{
    DBG_CTOR(XPolygon,NULL);

    sal_uInt16 nSize = rPoly.GetSize();
    pImpXPolygon = new ImpXPolygon( nSize );
    pImpXPolygon->nPoints = nSize;

    for( sal_uInt16 i = 0; i < nSize;  i++ )
    {
        pImpXPolygon->pPointAry[i] = rPoly[i];
        pImpXPolygon->pFlagAry[i] = (sal_uInt8) rPoly.GetFlags( i );
    }
}

/*************************************************************************
|*
|*    XPolygon::XPolygon()
|*
|*    Rechteck (auch mit abgerundeten Ecken) als Bezierpolygon erzeugen
|*    Ersterstellung    09.01.95 ESO
|*    Letzte Aenderung  09.01.95 ESO
|*
*************************************************************************/

XPolygon::XPolygon(const Rectangle& rRect, long nRx, long nRy)
{
    DBG_CTOR(XPolygon,NULL);
    pImpXPolygon = new ImpXPolygon(17);
    long nWh = (rRect.GetWidth()  - 1) / 2;
    long nHh = (rRect.GetHeight() - 1) / 2;

    if ( nRx > nWh )    nRx = nWh;
    if ( nRy > nHh )    nRy = nHh;

    // Rx negativ, damit Umlauf im Uhrzeigersinn erfolgt
    nRx = -nRx;

    // Faktor fuer Kontrollpunkte der Bezierkurven: 8/3 * (sin(45g) - 0.5)
    long    nXHdl = (long)(0.552284749 * nRx);
    long    nYHdl = (long)(0.552284749 * nRy);
    sal_uInt16  nPos = 0;

    if ( nRx && nRy )
    {
        Point aCenter;

        for (sal_uInt16 nQuad = 0; nQuad < 4; nQuad++)
        {
            switch ( nQuad )
            {
                case 0: aCenter = rRect.TopLeft();
                        aCenter.X() -= nRx;
                        aCenter.Y() += nRy;
                        break;
                case 1: aCenter = rRect.TopRight();
                        aCenter.X() += nRx;
                        aCenter.Y() += nRy;
                        break;
                case 2: aCenter = rRect.BottomRight();
                        aCenter.X() += nRx;
                        aCenter.Y() -= nRy;
                        break;
                case 3: aCenter = rRect.BottomLeft();
                        aCenter.X() -= nRx;
                        aCenter.Y() -= nRy;
                        break;
            }
            GenBezArc(aCenter, nRx, nRy, nXHdl, nYHdl, 0, 900, nQuad, nPos);
            pImpXPolygon->pFlagAry[nPos  ] = (sal_uInt8) XPOLY_SMOOTH;
            pImpXPolygon->pFlagAry[nPos+3] = (sal_uInt8) XPOLY_SMOOTH;
            nPos += 4;
        }
    }
    else
    {
        pImpXPolygon->pPointAry[nPos++] = rRect.TopLeft();
        pImpXPolygon->pPointAry[nPos++] = rRect.TopRight();
        pImpXPolygon->pPointAry[nPos++] = rRect.BottomRight();
        pImpXPolygon->pPointAry[nPos++] = rRect.BottomLeft();
    }
    pImpXPolygon->pPointAry[nPos] = pImpXPolygon->pPointAry[0];
    pImpXPolygon->nPoints = nPos + 1;
}

/*************************************************************************
|*
|*    XPolygon::XPolygon()
|*
|*    Ellipsen(bogen) als Bezierpolygon erzeugen
|*    Ersterstellung    09.01.95
|*    Letzte Aenderung  09.01.95
|*
*************************************************************************/

XPolygon::XPolygon(const Point& rCenter, long nRx, long nRy,
                   sal_uInt16 nStartAngle, sal_uInt16 nEndAngle, sal_Bool bClose)
{
    DBG_CTOR(XPolygon,NULL);
    pImpXPolygon = new ImpXPolygon(17);

    nStartAngle %= 3600;
    if ( nEndAngle > 3600 ) nEndAngle %= 3600;
    sal_Bool bFull = (nStartAngle == 0 && nEndAngle == 3600);

    // Faktor fuer Kontrollpunkte der Bezierkurven: 8/3 * (sin(45g) - 0.5)
    long    nXHdl = (long)(0.552284749 * nRx);
    long    nYHdl = (long)(0.552284749 * nRy);
    sal_uInt16  nPos = 0;
    sal_Bool    bLoopEnd = sal_False;

    do
    {
        sal_uInt16 nA1, nA2;
        sal_uInt16 nQuad = nStartAngle / 900;
        if ( nQuad == 4 ) nQuad = 0;
        bLoopEnd = CheckAngles(nStartAngle, nEndAngle, nA1, nA2);
        GenBezArc(rCenter, nRx, nRy, nXHdl, nYHdl, nA1, nA2, nQuad, nPos);
        nPos += 3;
        if ( !bLoopEnd )
            pImpXPolygon->pFlagAry[nPos] = (sal_uInt8) XPOLY_SMOOTH;

    } while ( !bLoopEnd );

    // Wenn kein Vollkreis, dann ggf. Enden mit Mittelpunkt verbinden
    if ( !bFull && bClose )
        pImpXPolygon->pPointAry[++nPos] = rCenter;

    if ( bFull )
    {
        pImpXPolygon->pFlagAry[0   ] = (sal_uInt8) XPOLY_SMOOTH;
        pImpXPolygon->pFlagAry[nPos] = (sal_uInt8) XPOLY_SMOOTH;
    }
    pImpXPolygon->nPoints = nPos + 1;
}

/*************************************************************************
|*
|*    XPolygon::~XPolygon()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  08.11.94
|*
*************************************************************************/

XPolygon::~XPolygon()
{
    DBG_DTOR(XPolygon,NULL);
    if( pImpXPolygon->nRefCount > 1 )
        pImpXPolygon->nRefCount--;
    else
        delete pImpXPolygon;
}

/*************************************************************************
|*
|*    XPolygon::CheckReference()
|*
|*    Referenzzaehler desImpXPoly pruefen und ggf. von diesem abkoppeln
|*    Ersterstellung    17.01.95 ESO
|*    Letzte Aenderung  17.01.95 ESO
|*
*************************************************************************/

void XPolygon::CheckReference()
{
    if( pImpXPolygon->nRefCount > 1 )
    {
        pImpXPolygon->nRefCount--;
        pImpXPolygon = new ImpXPolygon( *pImpXPolygon );
    }
}

/*************************************************************************
|*
|*    XPolygon::SetSize()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  08.11.94
|*
*************************************************************************/

void XPolygon::SetSize( sal_uInt16 nNewSize )
{
    CheckReference();
    pImpXPolygon->Resize( nNewSize );
}

/*************************************************************************
|*
|*    XPolygon::GetSize()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  12.01.95 ESO
|*
*************************************************************************/

sal_uInt16 XPolygon::GetSize() const
{
    pImpXPolygon->CheckPointDelete();
    return pImpXPolygon->nSize;
}

/*************************************************************************
|*
|*    XPolygon::SetPointCount()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  12.01.95 ESO
|*
*************************************************************************/

void XPolygon::SetPointCount( sal_uInt16 nPoints )
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();

    if( pImpXPolygon->nSize < nPoints )
        pImpXPolygon->Resize( nPoints );

    if ( nPoints < pImpXPolygon->nPoints )
    {
        sal_uInt16 nSize = pImpXPolygon->nPoints - nPoints;
        memset( &pImpXPolygon->pPointAry[nPoints], 0, nSize * sizeof( Point ) );
        memset( &pImpXPolygon->pFlagAry [nPoints], 0, nSize );
    }
    pImpXPolygon->nPoints = nPoints;
}

/*************************************************************************
|*
|*    XPolygon::GetPointCount()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  12.01.95 ESO
|*
*************************************************************************/

sal_uInt16 XPolygon::GetPointCount() const
{
    pImpXPolygon->CheckPointDelete();
    return pImpXPolygon->nPoints;
}

/*************************************************************************
|*
|*    XPolygon::Insert()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  08.11.94
|*
*************************************************************************/

void XPolygon::Insert( sal_uInt16 nPos, const Point& rPt, XPolyFlags eFlags )
{
    CheckReference();
    if (nPos>pImpXPolygon->nPoints) nPos=pImpXPolygon->nPoints;
    pImpXPolygon->InsertSpace( nPos, 1 );
    pImpXPolygon->pPointAry[nPos] = rPt;
    pImpXPolygon->pFlagAry[nPos]  = (sal_uInt8)eFlags;
}

/*************************************************************************
|*
|*    XPolygon::Insert()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  08.11.94
|*
*************************************************************************/

void XPolygon::Insert( sal_uInt16 nPos, const XPolygon& rXPoly )
{
    CheckReference();
    if (nPos>pImpXPolygon->nPoints) nPos=pImpXPolygon->nPoints;

    sal_uInt16 nPoints = rXPoly.GetPointCount();

    pImpXPolygon->InsertSpace( nPos, nPoints );

    memcpy( &(pImpXPolygon->pPointAry[nPos]),
            rXPoly.pImpXPolygon->pPointAry,
            nPoints*sizeof( Point ) );
    memcpy( &(pImpXPolygon->pFlagAry[nPos]),
            rXPoly.pImpXPolygon->pFlagAry,
            nPoints );
}

/*************************************************************************
|*
|*    XPolygon::Insert()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  08.11.94
|*
*************************************************************************/

void XPolygon::Insert( sal_uInt16 nPos, const Polygon& rPoly )
{
    CheckReference();
    if (nPos>pImpXPolygon->nPoints) nPos=pImpXPolygon->nPoints;

    sal_uInt16 nPoints = rPoly.GetSize();

    pImpXPolygon->InsertSpace( nPos, nPoints );

    sal_uInt16 i;
    for( i=0; i < nPoints; i++ )
        pImpXPolygon->pPointAry[i] = rPoly[i];

    // Die Flags sind durch das InsertSpace bereits auf 0 gesetzt
}

/*************************************************************************
|*
|*    XPolygon::Remove()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  08.11.94
|*
*************************************************************************/

void XPolygon::Remove( sal_uInt16 nPos, sal_uInt16 nCount )
{
    CheckReference();
    pImpXPolygon->Remove( nPos, nCount );
}

/*************************************************************************
|*
|*    XPolygon::Move()
|*
|*    Beschreibung
|*    Ersterstellung    09.11.94
|*    Letzte Aenderung  09.11.94
|*
*************************************************************************/

void XPolygon::Move( long nHorzMove, long nVertMove )
{
    if ( !nHorzMove && !nVertMove )
        return;

    CheckReference();

    // Punkte verschieben
    sal_uInt16 nCount = pImpXPolygon->nPoints;
    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        Point* pPt = &(pImpXPolygon->pPointAry[i]);
        pPt->X() += nHorzMove;
        pPt->Y() += nVertMove;
    }
}

/*************************************************************************
|*
|*    XPolygon::GetBoundRect()
|*
|*    Beschreibung
|*    Ersterstellung    09.11.94
|*    Letzte Aenderung  12.01.95 ESO
|*
*************************************************************************/

Rectangle XPolygon::GetBoundRect() const
{
    pImpXPolygon->CheckPointDelete();
    Rectangle aRetval;

    if(pImpXPolygon->nPoints)
    {
        // #i37709#
        // For historical reasons the control points are not part of the
        // BoundRect. This makes it necessary to subdivide the polygon to
        // get a relatively correct BoundRect. Numerically, this is not
        // correct and never was.

        const basegfx::B2DRange aPolygonRange(basegfx::tools::getRange(getB2DPolygon()));
        aRetval = Rectangle(
            FRound(aPolygonRange.getMinX()), FRound(aPolygonRange.getMinY()),
            FRound(aPolygonRange.getMaxX()), FRound(aPolygonRange.getMaxY()));
    }

    return aRetval;
}

/*************************************************************************
|*
|*    XPolygon::operator[]()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  12.01.95
|*
*************************************************************************/

const Point& XPolygon::operator[]( sal_uInt16 nPos ) const
{
    DBG_ASSERT(nPos < pImpXPolygon->nPoints, "Ungueltiger Index bei const-Arrayzugriff auf XPolygon");

    pImpXPolygon->CheckPointDelete();
    return pImpXPolygon->pPointAry[nPos];
}

/*************************************************************************
|*
|*    XPolygon::operator[]()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  12.01.95 ESO
|*
*************************************************************************/

Point& XPolygon::operator[]( sal_uInt16 nPos )
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();

    if( nPos >= pImpXPolygon->nSize )
    {
        DBG_ASSERT(pImpXPolygon->nResize, "Ungueltiger Index bei Arrayzugriff auf XPolygon");
        pImpXPolygon->Resize(nPos + 1, sal_False);
    }
    if( nPos >= pImpXPolygon->nPoints )
        pImpXPolygon->nPoints = nPos + 1;

    return pImpXPolygon->pPointAry[nPos];
}

/*************************************************************************
|*
|*    XPolygon::operator=()
|*
|*    Beschreibung      Zuweisungsoperator
|*    Ersterstellung    ESO 22.11.94
|*    Letzte Aenderung  ESO 12.01.95
|*
*************************************************************************/

XPolygon& XPolygon::operator=( const XPolygon& rXPoly )
{
    pImpXPolygon->CheckPointDelete();

    rXPoly.pImpXPolygon->nRefCount++;

    if( pImpXPolygon->nRefCount > 1 )
        pImpXPolygon->nRefCount--;
    else
        delete pImpXPolygon;

    pImpXPolygon = rXPoly.pImpXPolygon;
    return *this;
}

/*************************************************************************
|*
|*    XPolygon::operator==()
|*
|*    Beschreibung      Gleichheitsoperator
|*    Ersterstellung    ESO 22.11.94
|*    Letzte Aenderung  Joe 26.09.95
|*
*************************************************************************/

sal_Bool XPolygon::operator==( const XPolygon& rXPoly ) const
{
    pImpXPolygon->CheckPointDelete();
    if (rXPoly.pImpXPolygon==pImpXPolygon) return sal_True;
    return *rXPoly.pImpXPolygon == *pImpXPolygon;
}

/*************************************************************************
|*
|*    XPolygon::operator!=()
|*
|*    Beschreibung      Ungleichheitsoperator
|*    Ersterstellung    ESO 22.11.94
|*    Letzte Aenderung  Joe 26.09.95
|*
*************************************************************************/

sal_Bool XPolygon::operator!=( const XPolygon& rXPoly ) const
{
    pImpXPolygon->CheckPointDelete();
    if (rXPoly.pImpXPolygon==pImpXPolygon) return sal_False;
    return *rXPoly.pImpXPolygon != *pImpXPolygon;
}

/*************************************************************************
|*
|*    XPolygon::GetFlags()
|*
|*    Flags fuer den Punkt an der Position nPos zurueckgeben
|*    Ersterstellung    ESO 11.11.94
|*    Letzte Aenderung  ESO 12.01.95
|*
*************************************************************************/

XPolyFlags XPolygon::GetFlags( sal_uInt16 nPos ) const
{
    pImpXPolygon->CheckPointDelete();
    return (XPolyFlags) pImpXPolygon->pFlagAry[nPos];
}

/*************************************************************************
|*
|*    XPolygon::SetFlags()
|*
|*    Flags fuer den Punkt an der Position nPos setzen
|*    Ersterstellung    ESO 11.11.94
|*    Letzte Aenderung  ESO 12.01.95
|*
*************************************************************************/

void XPolygon::SetFlags( sal_uInt16 nPos, XPolyFlags eFlags )
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();
    pImpXPolygon->pFlagAry[nPos] = (sal_uInt8) eFlags;
}

/*************************************************************************
|*
|*    XPolygon::IsControl()
|*
|*    Kurzform zur Abfrage des CONTROL-Flags
|*    Ersterstellung    ESO 09.01.95
|*    Letzte Aenderung  ESO 12.01.95
|*
*************************************************************************/

sal_Bool XPolygon::IsControl(sal_uInt16 nPos) const
{
    return ( (XPolyFlags) pImpXPolygon->pFlagAry[nPos] == XPOLY_CONTROL );
}

/*************************************************************************
|*
|*    XPolygon::IsSmooth()
|*
|*    Kurzform zur Abfrage von SMOOTH- und SYMMTR-Flag
|*    Ersterstellung    ESO 18.04.95
|*    Letzte Aenderung  ESO 18.04.95
|*
*************************************************************************/

sal_Bool XPolygon::IsSmooth(sal_uInt16 nPos) const
{
    XPolyFlags eFlag = (XPolyFlags) pImpXPolygon->pFlagAry[nPos];
    return ( eFlag == XPOLY_SMOOTH || eFlag == XPOLY_SYMMTR );
}

/*************************************************************************
|*
|*    XPolygon::CalcDistance()
|*
|*    Abstand zwischen zwei Punkten berechnen
|*    Ersterstellung    ESO 09.01.95
|*    Letzte Aenderung  ESO 09.01.95
|*
*************************************************************************/

double XPolygon::CalcDistance(sal_uInt16 nP1, sal_uInt16 nP2)
{
    const Point& rP1 = pImpXPolygon->pPointAry[nP1];
    const Point& rP2 = pImpXPolygon->pPointAry[nP2];
    double fDx = rP2.X() - rP1.X();
    double fDy = rP2.Y() - rP1.Y();
    return sqrt(fDx * fDx + fDy * fDy);
}

/*************************************************************************
|*
|*    XPolygon::SubdivideBezier()
|*
|*    Bezierkurve unterteilen
|*    Ersterstellung    ESO 09.01.95
|*    Letzte Aenderung  ESO 09.01.95
|*
*************************************************************************/

void XPolygon::SubdivideBezier(sal_uInt16 nPos, sal_Bool bCalcFirst, double fT)
{
    Point*  pPoints = pImpXPolygon->pPointAry;
    double  fT2 = fT * fT;
    double  fT3 = fT * fT2;
    double  fU = 1.0 - fT;
    double  fU2 = fU * fU;
    double  fU3 = fU * fU2;
    sal_uInt16  nIdx = nPos;
    short   nPosInc, nIdxInc;

    if ( bCalcFirst )
    {
        nPos += 3;
        nPosInc = -1;
        nIdxInc = 0;
    }
    else
    {
        nPosInc = 1;
        nIdxInc = 1;
    }
    pPoints[nPos].X() = (long) (fU3 *       pPoints[nIdx  ].X() +
                                fT  * fU2 * pPoints[nIdx+1].X() * 3 +
                                fT2 * fU  * pPoints[nIdx+2].X() * 3 +
                                fT3 *       pPoints[nIdx+3].X());
    pPoints[nPos].Y() = (long) (fU3 *       pPoints[nIdx  ].Y() +
                                fT  * fU2 * pPoints[nIdx+1].Y() * 3 +
                                fT2 * fU  * pPoints[nIdx+2].Y() * 3 +
                                fT3 *       pPoints[nIdx+3].Y());
    nPos = nPos + nPosInc;
    nIdx = nIdx + nIdxInc;
    pPoints[nPos].X() = (long) (fU2 *       pPoints[nIdx  ].X() +
                                fT  * fU *  pPoints[nIdx+1].X() * 2 +
                                fT2 *       pPoints[nIdx+2].X());
    pPoints[nPos].Y() = (long) (fU2 *       pPoints[nIdx  ].Y() +
                                fT  * fU *  pPoints[nIdx+1].Y() * 2 +
                                fT2 *       pPoints[nIdx+2].Y());
    nPos = nPos + nPosInc;
    nIdx = nIdx + nIdxInc;
    pPoints[nPos].X() = (long) (fU * pPoints[nIdx  ].X() +
                                fT * pPoints[nIdx+1].X());
    pPoints[nPos].Y() = (long) (fU * pPoints[nIdx  ].Y() +
                                fT * pPoints[nIdx+1].Y());
}

/************************************************************************/

void XPolygon::GenBezArc(const Point& rCenter, long nRx, long nRy,
                         long nXHdl, long nYHdl, sal_uInt16 nStart, sal_uInt16 nEnd,
                         sal_uInt16 nQuad, sal_uInt16 nFirst)
{
    Point* pPoints = pImpXPolygon->pPointAry;
    pPoints[nFirst  ] = rCenter;
    pPoints[nFirst+3] = rCenter;

    if ( nQuad == 1 || nQuad == 2 )
    {
        nRx   = -nRx; nXHdl = -nXHdl;
    }
    if ( nQuad == 0 || nQuad == 1 )
    {
        nRy   = -nRy; nYHdl = -nYHdl;
    }

    if ( nQuad == 0 || nQuad == 2 )
    {
        pPoints[nFirst].X() += nRx; pPoints[nFirst+3].Y() += nRy;
    }
    else
    {
        pPoints[nFirst].Y() += nRy; pPoints[nFirst+3].X() += nRx;
    }
    pPoints[nFirst+1] = pPoints[nFirst];
    pPoints[nFirst+2] = pPoints[nFirst+3];

    if ( nQuad == 0 || nQuad == 2 )
    {
        pPoints[nFirst+1].Y() += nYHdl; pPoints[nFirst+2].X() += nXHdl;
    }
    else
    {
        pPoints[nFirst+1].X() += nXHdl; pPoints[nFirst+2].Y() += nYHdl;
    }
    if ( nStart > 0 )
        SubdivideBezier(nFirst, sal_False, (double)nStart / 900);
    if ( nEnd < 900 )
        SubdivideBezier(nFirst, sal_True, (double)(nEnd-nStart) / (900-nStart));
    SetFlags(nFirst+1, XPOLY_CONTROL);
    SetFlags(nFirst+2, XPOLY_CONTROL);
}

/************************************************************************/

sal_Bool XPolygon::CheckAngles(sal_uInt16& nStart, sal_uInt16 nEnd, sal_uInt16& nA1, sal_uInt16& nA2)
{
    if ( nStart == 3600 ) nStart = 0;
    if ( nEnd == 0 ) nEnd = 3600;
    sal_uInt16 nStPrev = nStart;
    sal_uInt16 nMax = (nStart / 900 + 1) * 900;
    sal_uInt16 nMin = nMax - 900;

    if ( nEnd >= nMax || nEnd <= nStart )   nA2 = 900;
    else                                    nA2 = nEnd - nMin;
    nA1 = nStart - nMin;
    nStart = nMax;

    // sal_True zurueck, falls letztes Segment berechnet wurde
    return (nStPrev < nEnd && nStart >= nEnd);
}

/*************************************************************************
|*
|*    XPolygon::CalcSmoothJoin()
|*
|*    glatten Uebergang zu einer Bezierkurve berechnen, indem der
|*    entsprechende Punkt auf die Verbindungslinie von zwei anderen
|*    Punkten projiziert wird
|*     Center = End- bzw. Anfangspunkt der Bezierkurve
|*     Drag   = der bewegte Punkt, der die Verschiebung von Pnt vorgibt
|*     Pnt    = der zu modifizierende Punkt
|*    Wenn Center am Anfang bzw. Ende des Polygons liegt, wird Pnt
|*    auf die entgegengesetzte Seite verlegt
|*    Ersterstellung    ESO 09.01.95
|*    Letzte Aenderung  ESO 18.04.95
|*
\************************************************************************/

void XPolygon::CalcSmoothJoin(sal_uInt16 nCenter, sal_uInt16 nDrag, sal_uInt16 nPnt)
{
    CheckReference();

//  sal_uInt16  nMaxPnt = pImpXPolygon->nPoints - 1;

//  if ( nCenter == nMaxPnt )   nPnt = 1;
//  else if ( nCenter == 0 )    nPnt = nMaxPnt - 1;

    // Wenn nPnt kein Control-Punkt, d.h. nicht verschiebbar, dann
    // statt dessen nDrag auf der Achse nCenter-nPnt verschieben
    if ( !IsControl(nPnt) )
    {
        sal_uInt16 nTmp = nDrag;
        nDrag = nPnt;
        nPnt = nTmp;
    }
    Point*  pPoints = pImpXPolygon->pPointAry;
    Point   aDiff   = pPoints[nDrag] - pPoints[nCenter];
    double  fDiv    = CalcDistance(nCenter, nDrag);

    if ( fDiv )
    {
        double fRatio = CalcDistance(nCenter, nPnt) / fDiv;
        // bei SMOOTH bisherige Laenge beibehalten
        if ( GetFlags(nCenter) == XPOLY_SMOOTH || !IsControl(nDrag) )
        {
            aDiff.X() = (long) (fRatio * aDiff.X());
            aDiff.Y() = (long) (fRatio * aDiff.Y());
        }
        pPoints[nPnt] = pPoints[nCenter] - aDiff;
    }
}

/*************************************************************************
|*
|*    XPolygon::CalcTangent()
|*
|*    Tangente fuer den Uebergang zwischen zwei Bezierkurven berechnen
|*     Center = End- bzw. Anfangspunkt der Bezierkurven
|*     Prev   = vorheriger Zugpunkt
|*     Next   = naechster Zugpunkt
|*    Ersterstellung    ESO 09.01.95
|*    Letzte Aenderung  ESO 18.04.95
|*
\************************************************************************/

void XPolygon::CalcTangent(sal_uInt16 nCenter, sal_uInt16 nPrev, sal_uInt16 nNext)
{
    CheckReference();

    double fAbsLen = CalcDistance(nNext, nPrev);

    if ( fAbsLen )
    {
        const Point& rCenter = pImpXPolygon->pPointAry[nCenter];
        Point&  rNext = pImpXPolygon->pPointAry[nNext];
        Point&  rPrev = pImpXPolygon->pPointAry[nPrev];
        Point   aDiff = rNext - rPrev;
        double  fNextLen = CalcDistance(nCenter, nNext) / fAbsLen;
        double  fPrevLen = CalcDistance(nCenter, nPrev) / fAbsLen;

        // bei SYMMTR gleiche Laenge fuer beide Seiten
        if ( GetFlags(nCenter) == XPOLY_SYMMTR )
        {
            fPrevLen = (fNextLen + fPrevLen) / 2;
            fNextLen = fPrevLen;
        }
        rNext.X() = rCenter.X() + (long) (fNextLen * aDiff.X());
        rNext.Y() = rCenter.Y() + (long) (fNextLen * aDiff.Y());
        rPrev.X() = rCenter.X() - (long) (fPrevLen * aDiff.X());
        rPrev.Y() = rCenter.Y() - (long) (fPrevLen * aDiff.Y());
    }
}

/*************************************************************************
|*
|*    XPolygon::PointsToBezier()
|*
|*    wandelt vier Polygonpunkte in eine Bezierkurve durch diese Punkte um
|*    Ersterstellung    ESO 09.01.95
|*    Letzte Aenderung  ESO 09.01.95
|*
\************************************************************************/

void XPolygon::PointsToBezier(sal_uInt16 nFirst)
{
    double  nFullLength, nPart1Length, nPart2Length;
    double  fX0, fY0, fX1, fY1, fX2, fY2, fX3, fY3;
    double  fTx1, fTx2, fTy1, fTy2;
    double  fT1, fU1, fT2, fU2, fV;
    Point*  pPoints = pImpXPolygon->pPointAry;

    if ( nFirst > pImpXPolygon->nPoints - 4 || IsControl(nFirst) ||
         IsControl(nFirst+1) || IsControl(nFirst+2) || IsControl(nFirst+3) )
        return;

    CheckReference();

    fTx1 = pPoints[nFirst+1].X();
    fTy1 = pPoints[nFirst+1].Y();
    fTx2 = pPoints[nFirst+2].X();
    fTy2 = pPoints[nFirst+2].Y();
    fX0  = pPoints[nFirst  ].X();
    fY0  = pPoints[nFirst  ].Y();
    fX3  = pPoints[nFirst+3].X();
    fY3  = pPoints[nFirst+3].Y();

    nPart1Length = CalcDistance(nFirst, nFirst+1);
    nPart2Length = nPart1Length + CalcDistance(nFirst+1, nFirst+2);
    nFullLength  = nPart2Length + CalcDistance(nFirst+2, nFirst+3);
    if ( nFullLength < 20 )
        return;

    if ( nPart2Length == nFullLength )
        nPart2Length -= 1;
    if ( nPart1Length == nFullLength )
        nPart1Length = nPart2Length - 1;
    if ( nPart1Length <= 0 )
        nPart1Length = 1;
    if ( nPart2Length <= 0 || nPart2Length == nPart1Length )
        nPart2Length = nPart1Length + 1;

    fT1 = nPart1Length / nFullLength;
    fU1 = 1.0 - fT1;
    fT2 = nPart2Length / nFullLength;
    fU2 = 1.0 - fT2;
    fV = 3 * (1.0 - (fT1 * fU2) / (fT2 * fU1));

    fX1 = fTx1 / (fT1 * fU1 * fU1) - fTx2 * fT1 / (fT2 * fT2 * fU1 * fU2);
    fX1 /= fV;
    fX1 -= fX0 * ( fU1 / fT1 + fU2 / fT2) / 3;
    fX1 += fX3 * ( fT1 * fT2 / (fU1 * fU2)) / 3;

    fY1 = fTy1 / (fT1 * fU1 * fU1) - fTy2 * fT1 / (fT2 * fT2 * fU1 * fU2);
    fY1 /= fV;
    fY1 -= fY0 * ( fU1 / fT1 + fU2 / fT2) / 3;
    fY1 += fY3 * ( fT1 * fT2 / (fU1 * fU2)) / 3;

    fX2 = fTx2 / (fT2 * fT2 * fU2 * 3) - fX0 * fU2 * fU2 / ( fT2 * fT2 * 3);
    fX2 -= fX1 * fU2 / fT2;
    fX2 -= fX3 * fT2 / (fU2 * 3);

    fY2 = fTy2 / (fT2 * fT2 * fU2 * 3) - fY0 * fU2 * fU2 / ( fT2 * fT2 * 3);
    fY2 -= fY1 * fU2 / fT2;
    fY2 -= fY3 * fT2 / (fU2 * 3);

    pPoints[nFirst+1] = Point((long) fX1, (long) fY1);
    pPoints[nFirst+2] = Point((long) fX2, (long) fY2);
    SetFlags(nFirst+1, XPOLY_CONTROL);
    SetFlags(nFirst+2, XPOLY_CONTROL);
}

/*************************************************************************
|*
|*    XPolygon::Translate()
|*
|*    Polygon auf den uebergebenen Punkt verschieben
|*    Ersterstellung    ESO 17.01.95
|*    Letzte Aenderung  ESO 17.01.95
|*
*************************************************************************/

void XPolygon::Translate(const Point& rTrans)
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();

    sal_uInt16 nPntCnt = pImpXPolygon->nPoints;

    for (sal_uInt16 i = 0; i < nPntCnt; i++)
        pImpXPolygon->pPointAry[i] += rTrans;
}

/*************************************************************************
|*
|*    XPolygon::Rotate()
|*
|*    Alle Punkte um den Punkt rCenter drehen, Sinus und Cosinus
|*    muessen uebergeben werden
|*    Ersterstellung    ESO 09.01.95
|*    Letzte Aenderung  ESO 17.01.95
|*
*************************************************************************/

void XPolygon::Rotate(const Point& rCenter, double fSin, double fCos)
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();

    long nX;
    long nY;
    long nNewX;
    long nNewY;
    long nCenterX = rCenter.X();
    long nCenterY = rCenter.Y();

    sal_uInt16 nPntCnt = pImpXPolygon->nPoints;

    for (sal_uInt16 i = 0; i < nPntCnt; i++)
    {
        Point *pPt = &(pImpXPolygon->pPointAry[i]);
        nX = pPt->X()-nCenterX;
        nY = pPt->Y()-nCenterY;
        nNewX =  (long)floor(fCos * nX + fSin * nY + 0.5);
        nNewY = -(long)floor(fSin * nX - fCos * nY + 0.5);
        pPt->X() = nNewX + nCenterX;
        pPt->Y() = nNewY + nCenterY;

    /* und so stand das in einem anderen File auf T:
       dass ich am 29-11-1995 gegettet habe. Joe M.
    sal_uInt16 nPntCnt = pImpXPolygon->nPoints;

    for (sal_uInt16 i = 0; i < nPntCnt; i++)
    {
        Point P = pImpXPolygon->pPointAry[i] - rCenter;
        long X = P.X();
        long Y = P.Y();
        P.X() =  (long)floor(fCos * X + fSin * Y + 0.5);
        P.Y() = -(long)floor(fSin * X - fCos * Y + 0.5);
        pImpXPolygon->pPointAry[i] = P + rCenter;
    */
    }
}

/*************************************************************************
|*
|*    XPolygon::Rotate()
|*
|*    Alle Punkte um den Punkt rCenter mit dem Winkel nAngle drehen
|*    Winkel in 10tel Grad, Wertebereich 0 - 3600
|*    Ersterstellung    ESO 17.01.95
|*    Letzte Aenderung  ESO 17.01.95
|*
*************************************************************************/

void XPolygon::Rotate(const Point& rCenter, sal_uInt16 nAngle)
{
    nAngle %= 3600;

    if ( nAngle != 0 )
    {
        double fAngle = F_PI * nAngle / 1800;
        double fSin = sin(fAngle);
        double fCos = cos(fAngle);
        Rotate(rCenter, fSin, fCos);
    }
}

/*************************************************************************
|*
|*    XPolygon::Scale()
|*
|*    XPolygon in X- und/oder Y-Richtung skalieren
|*    Ersterstellung    ESO 01.02.95
|*    Letzte Aenderung  ESO 01.02.95
|*
*************************************************************************/

void XPolygon::Scale(double fSx, double fSy)
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();

    sal_uInt16 nPntCnt = pImpXPolygon->nPoints;

    for (sal_uInt16 i = 0; i < nPntCnt; i++)
    {
        Point& rPnt = pImpXPolygon->pPointAry[i];
        rPnt.X() = (long)(fSx * rPnt.X());
        rPnt.Y() = (long)(fSy * rPnt.Y());
    }
}

/*************************************************************************
|*
|*    XPolygon::SlantX()
|*
|*    XPolygon in X-Richtung um einen beliebigen Winkel kippen,
|*    bezogen auf eine Referenz-Y-Koordinate
|*    Ersterstellung    ESO 01.02.95
|*    Letzte Aenderung  ESO 01.02.95
|*
*************************************************************************/

void XPolygon::SlantX(long nYRef, double fSin, double fCos)
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();

    sal_uInt16 nPntCnt = pImpXPolygon->nPoints;

    for (sal_uInt16 i = 0; i < nPntCnt; i++)
    {
        Point& rPnt = pImpXPolygon->pPointAry[i];
        long nDy = rPnt.Y() - nYRef;
        rPnt.X() += (long)(fSin * nDy);
        rPnt.Y() = nYRef + (long)(fCos * nDy);
    }
}

/*************************************************************************
|*
|*    XPolygon::SlantY()
|*
|*    XPolygon in Y-Richtung um einen beliebigen Winkel kippen,
|*    bezogen auf eine Referenz-X-Koordinate
|*    Ersterstellung    ESO 01.02.95
|*    Letzte Aenderung  ESO 01.02.95
|*
*************************************************************************/

void XPolygon::SlantY(long nXRef, double fSin, double fCos)
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();

    sal_uInt16 nPntCnt = pImpXPolygon->nPoints;

    for (sal_uInt16 i = 0; i < nPntCnt; i++)
    {
        Point& rPnt = pImpXPolygon->pPointAry[i];
        long nDx = rPnt.X() - nXRef;
        rPnt.X() = nXRef + (long)(fCos * nDx);
        rPnt.Y() -= (long)(fSin * nDx);
    }
}

/*************************************************************************
|*
|*    XPolygon::Distort()
|*
|*    XPolygon verzerren, indem die Koordinaten relativ zu einem
|*    Referenzrechteck in ein beliebiges Viereck skaliert werden
|*    Zuordnung der Viereck-Punkte im Polygon zum Referenzrechteck:
|*    0: links oben      0----1
|*    1: rechts oben     |    |
|*    2: rechts unten    3----2
|*    3: links unten
|*    Ersterstellung    ESO 07.07.95
|*    Letzte Aenderung  ESO 07.07.95
|*
*************************************************************************/

void XPolygon::Distort(const Rectangle& rRefRect,
                       const XPolygon& rDistortedRect)
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();

    long    Xr, Wr, X1, X2, X3, X4;
    long    Yr, Hr, Y1, Y2, Y3, Y4;
    double  fTx, fTy, fUx, fUy;

    Xr = rRefRect.Left();
    Yr = rRefRect.Top();
    Wr = rRefRect.GetWidth();
    Hr = rRefRect.GetHeight();

    if ( Wr && Hr )
    {
        DBG_ASSERT(rDistortedRect.pImpXPolygon->nPoints >= 4,
                   "Distort-Rechteck zu klein");

        X1 = rDistortedRect[0].X();
        Y1 = rDistortedRect[0].Y();
        X2 = rDistortedRect[1].X();
        Y2 = rDistortedRect[1].Y();
        X3 = rDistortedRect[3].X();
        Y3 = rDistortedRect[3].Y();
        X4 = rDistortedRect[2].X();
        Y4 = rDistortedRect[2].Y();

        sal_uInt16 nPntCnt = pImpXPolygon->nPoints;

        for (sal_uInt16 i = 0; i < nPntCnt; i++)
        {
            Point& rPnt = pImpXPolygon->pPointAry[i];

            fTx = (double)(rPnt.X() - Xr) / Wr;
            fTy = (double)(rPnt.Y() - Yr) / Hr;
            fUx = 1.0 - fTx;
            fUy = 1.0 - fTy;

            rPnt.X() = (long) ( fUy * (fUx * X1 + fTx * X2) +
                                fTy * (fUx * X3 + fTx * X4) );
            rPnt.Y() = (long) ( fUx * (fUy * Y1 + fTy * Y3) +
                                fTx * (fUy * Y2 + fTy * Y4) );
        }
    }
}

/*************************************************************************
|*
|* Bestimme den linken, unteren Punkt des Polygons und richte das
|* Polygon so aus, dass dieser Punkt auf dem Index 0 liegt
|*
\************************************************************************/

void XPolygon::Rotate20()
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();

    double   fMinY   = pImpXPolygon->pPointAry->Y();
    double   fMinX   = pImpXPolygon->pPointAry->X();
    long     nPntCnt = pImpXPolygon->nPoints;
    long     nIndex0 = 0;

    for (long nPoints = 1;
              nPoints < nPntCnt;
              nPoints ++)
    {
        Point &rPnt = pImpXPolygon->pPointAry[nPoints];

        if ((rPnt.X () < fMinX) || (fMinX == rPnt.X ()) &&
                                   (fMinY >= rPnt.Y ()))
        {
            fMinX   = rPnt.X ();
            fMinY   = rPnt.Y ();
            nIndex0 = nPoints;
        }
    }

    if (nIndex0 < nPntCnt)
    {
        Point *pTemp = new Point [nIndex0];
        memcpy (pTemp, pImpXPolygon->pPointAry, nIndex0 * sizeof (Point));
        memcpy (pImpXPolygon->pPointAry, &pImpXPolygon->pPointAry [nIndex0], (nPntCnt - nIndex0) * sizeof (Point));
        memcpy (&pImpXPolygon->pPointAry [nIndex0], pTemp, nIndex0 * sizeof (Point));
        delete[] pTemp;
    }
}

basegfx::B2DPolygon XPolygon::getB2DPolygon() const
{
    // #i74631# use tools Polygon class for conversion to not have the code doubled
    // here. This needs one more conversion but avoids different convertors in
    // the long run
    DBG_ASSERT(pImpXPolygon != 0, "XPolygon::getB2DPolygon(): XPolygon has no implementation incarnated (!)");
    const Polygon aSource(GetPointCount(), pImpXPolygon->pPointAry, pImpXPolygon->pFlagAry);

    return aSource.getB2DPolygon();
}

XPolygon::XPolygon(const basegfx::B2DPolygon& rPolygon)
{
    // #i74631# use tools Polygon class for conversion to not have the code doubled
    // here. This needs one more conversion but avoids different convertors in
    // the long run
    DBG_CTOR(XPolygon,NULL);

    const Polygon aSource(rPolygon);
    sal_uInt16 nSize = aSource.GetSize();
    pImpXPolygon = new ImpXPolygon( nSize );
    pImpXPolygon->nPoints = nSize;

    for( sal_uInt16 i = 0; i < nSize;  i++ )
    {
        pImpXPolygon->pPointAry[i] = aSource[i];
        pImpXPolygon->pFlagAry[i] = (sal_uInt8) aSource.GetFlags( i );
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+--------------- XPolyPolygon -----------------------------------------+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*************************************************************************
|*
|*    ImpXPolyPolygon::ImpXPolyPolygon()
|*
|*    Beschreibung      Erzeugt das XPolygon-Array
|*    Ersterstellung    CL 09.11.94
|*    Letzte Aenderung  MM 09.11.94
|*
*************************************************************************/

ImpXPolyPolygon::ImpXPolyPolygon( const ImpXPolyPolygon& rImpXPolyPoly ) :
                     aXPolyList( rImpXPolyPoly.aXPolyList )
{
    nRefCount = 1;

    // Einzelne Elemente duplizieren
    XPolygon* pXPoly = aXPolyList.First();
    while ( pXPoly )
    {
        aXPolyList.Replace( new XPolygon( *(aXPolyList.GetCurObject()) ) );
        pXPoly = aXPolyList.Next();
    }
}


/*************************************************************************
|*
|*    ImpXPolyPolygon::~ImpXPolyPolygon()
|*
|*    Beschreibung      Loescht das Polygon-Array
|*    Ersterstellung    CL 09.06.93
|*    Letzte Aenderung  CL 09.06.93
|*
*************************************************************************/

ImpXPolyPolygon::~ImpXPolyPolygon()
{
    XPolygon* pXPoly = aXPolyList.First();
    while( pXPoly )
    {
        delete pXPoly;
        pXPoly = aXPolyList.Next();
    }
}

/*************************************************************************
|*
|*    ImpXPolyPolygon::operator==()
|*
|*    Ersterstellung    Joe 26-09-95
|*    Letzte Aenderung
|*
*************************************************************************/


bool ImpXPolyPolygon::operator==(const ImpXPolyPolygon& rImpXPolyPoly) const
{
    sal_uInt16 nAnz=(sal_uInt16)aXPolyList.Count();
    const XPolygonList& rCmpList=rImpXPolyPoly.aXPolyList;
    if (nAnz!=(sal_uInt16)rCmpList.Count()) return sal_False;
    bool bEq=true;
    for (sal_uInt16 i=nAnz; i>0 && bEq;) {
        i--;
        bEq= *aXPolyList.GetObject(i) == *rCmpList.GetObject(i);
    }
    return bEq;
}

/*************************************************************************
|*
|*    XPolyPolygon::XPolyPolygon()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    CL 27.01.93
|*    Letzte Aenderung  CL 27.01.93
|*
*************************************************************************/

XPolyPolygon::XPolyPolygon( sal_uInt16 nInitSize, sal_uInt16 nResize )
{
    DBG_CTOR(XPolyPolygon,NULL);
    pImpXPolyPolygon = new ImpXPolyPolygon( nInitSize, nResize );
}


/*************************************************************************
|*
|*    XPolyPolygon::XPolyPolygon()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    CL 27.01.93
|*    Letzte Aenderung  CL 27.01.93
|*
*************************************************************************/

XPolyPolygon::XPolyPolygon( const XPolygon& rXPoly )
{
    DBG_CTOR(XPolyPolygon,NULL);
    pImpXPolyPolygon = new ImpXPolyPolygon;
    pImpXPolyPolygon->aXPolyList.Insert( new XPolygon( rXPoly ) );
}

/*************************************************************************
|*
|*    XPolyPolygon::XPolyPolygon()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    CL 27.01.93
|*    Letzte Aenderung  CL 27.01.93
|*
*************************************************************************/

XPolyPolygon::XPolyPolygon( const XPolyPolygon& rXPolyPoly )
{
    DBG_CTOR(XPolyPolygon,NULL);
    pImpXPolyPolygon = rXPolyPoly.pImpXPolyPolygon;
    pImpXPolyPolygon->nRefCount++;
}

/*************************************************************************
|*
|*    XPolyPolygon::XPolyPolygon()
|*
|*    XPolyPolygon aus einen Standard-PolyPolygon erzeugen
|*    Ersterstellung    18.01.95 ESO
|*    Letzte Aenderung  18.01.95 ESO
|*
*************************************************************************/

XPolyPolygon::XPolyPolygon( const PolyPolygon& rPolyPoly )
{
    DBG_CTOR(XPolyPolygon,NULL);
    pImpXPolyPolygon = new ImpXPolyPolygon;

    for (sal_uInt16 i = 0; i < rPolyPoly.Count(); i++)
        pImpXPolyPolygon->aXPolyList.Insert(
                                    new XPolygon(rPolyPoly.GetObject(i)) );
}

/*************************************************************************
|*
|*    XPolyPolygon::~XPolyPolygon()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    CL 27.01.93
|*    Letzte Aenderung  CL 27.01.93
|*
*************************************************************************/

XPolyPolygon::~XPolyPolygon()
{
    DBG_DTOR(XPolyPolygon,NULL);
    if( pImpXPolyPolygon->nRefCount > 1 )
        pImpXPolyPolygon->nRefCount--;
    else
        delete pImpXPolyPolygon;
}

/*************************************************************************
|*
|*    XPolygon::CheckReference()
|*
|*    Referenzzaehler desImpXPolyPoly pruefen und ggf. von diesem abkoppeln
|*    Ersterstellung    18.01.95 ESO
|*    Letzte Aenderung  18.01.95 ESO
|*
*************************************************************************/

void XPolyPolygon::CheckReference()
{
    if( pImpXPolyPolygon->nRefCount > 1 )
    {
        pImpXPolyPolygon->nRefCount--;
        pImpXPolyPolygon = new ImpXPolyPolygon( *pImpXPolyPolygon );
    }
}

/*************************************************************************
|*
|*    XPolyPolygon::Insert()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    CL 27.01.93
|*    Letzte Aenderung  CL 27.01.93
|*
*************************************************************************/

void XPolyPolygon::Insert( const XPolygon& rXPoly, sal_uInt16 nPos )
{
    CheckReference();
    XPolygon* pXPoly = new XPolygon( rXPoly );
    pImpXPolyPolygon->aXPolyList.Insert( pXPoly, nPos );
}

/*************************************************************************
|*
|*    XPolyPolygon::Insert()
|*
|*    saemtliche XPolygone aus einem XPolyPolygon einfuegen
|*    Ersterstellung    18.01.95 ESO
|*    Letzte Aenderung  18.01.95 ESO
|*
*************************************************************************/

void XPolyPolygon::Insert( const XPolyPolygon& rXPolyPoly, sal_uInt16 nPos )
{
    CheckReference();

    for (sal_uInt16 i = 0; i < rXPolyPoly.Count(); i++)
    {
        XPolygon* pXPoly = new XPolygon(rXPolyPoly[i]);
        pImpXPolyPolygon->aXPolyList.Insert(pXPoly, nPos);
        if ( nPos != XPOLYPOLY_APPEND )
            nPos++;
    }
}

/*************************************************************************
|*
|*    XPolyPolygon::Remove()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    CL 27.01.93
|*    Letzte Aenderung  CL 27.01.93
|*
*************************************************************************/

XPolygon XPolyPolygon::Remove( sal_uInt16 nPos )
{
    CheckReference();
    XPolygon* pTmpXPoly = pImpXPolyPolygon->aXPolyList.Remove( nPos );
    XPolygon  aXPoly( *pTmpXPoly );
    delete pTmpXPoly;
    return aXPoly;
}


/*************************************************************************
|*
|*    XPolyPolygon::Replace()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    CL 27.01.93
|*    Letzte Aenderung  CL 27.01.93
|*
*************************************************************************/

XPolygon XPolyPolygon::Replace( const XPolygon& rXPoly, sal_uInt16 nPos )
{
    CheckReference();
    XPolygon* pXPoly = new XPolygon( rXPoly );
    XPolygon* pTmpXPoly = pImpXPolyPolygon->aXPolyList.Replace( pXPoly, nPos );
    XPolygon  aXPoly( *pTmpXPoly );
    delete pTmpXPoly;
    return aXPoly;
}


/*************************************************************************
|*
|*    XPolyPolygon::GetObject()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    CL 27.01.93
|*    Letzte Aenderung  CL 27.01.93
|*
*************************************************************************/

const XPolygon& XPolyPolygon::GetObject( sal_uInt16 nPos ) const
{
    return *(pImpXPolyPolygon->aXPolyList.GetObject( nPos ));
}


/*************************************************************************
|*
|*    XPolyPolygon::Clear()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    CL 27.01.93
|*    Letzte Aenderung  TH 17.10.94
|*
*************************************************************************/

void XPolyPolygon::Clear()
{
    if ( pImpXPolyPolygon->nRefCount > 1 )
    {
        pImpXPolyPolygon->nRefCount--;
        pImpXPolyPolygon = new ImpXPolyPolygon();
    }
    else
    {
        XPolygon* pXPoly = pImpXPolyPolygon->aXPolyList.First();
        while( pXPoly )
        {
            delete pXPoly;
            pXPoly = pImpXPolyPolygon->aXPolyList.Next();
        }
        pImpXPolyPolygon->aXPolyList.Clear();
    }
}


/*************************************************************************
|*
|*    XPolyPolygon::Count()
|*
|*    Beschreibung
|*    Ersterstellung    CL 27.01.93
|*    Letzte Aenderung  CL 27.01.93
|*
*************************************************************************/

sal_uInt16 XPolyPolygon::Count() const
{
    return (sal_uInt16)(pImpXPolyPolygon->aXPolyList.Count());
}


/*************************************************************************
|*
|*    XPolyPolygon::Move()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    TH 04.10.94
|*    Letzte Aenderung  TH 04.10.94
|*
*************************************************************************/

void XPolyPolygon::Move( long nHorzMove, long nVertMove )
{
    // Diese Abfrage sollte man fuer die DrawEngine durchfuehren
    if ( !nHorzMove && !nVertMove )
        return;

    // Referenzcounter beruecksichtigen
    CheckReference();

    // Punkte verschieben
    XPolygon* pXPoly = pImpXPolyPolygon->aXPolyList.First();
    while( pXPoly )
    {
        pXPoly->Move( nHorzMove, nVertMove );
        pXPoly = pImpXPolyPolygon->aXPolyList.Next();
    }
}

/*************************************************************************
|*
|*    XPolyPolygon::GetBoundRect()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    TH 04.10.94
|*    Letzte Aenderung  TH 04.10.94
|*
*************************************************************************/

Rectangle XPolyPolygon::GetBoundRect() const
{
    sal_uInt16    nXPoly = (sal_uInt16)pImpXPolyPolygon->aXPolyList.Count();
    Rectangle aRect;

    for ( sal_uInt16 n = 0; n < nXPoly; n++ )
    {
        const XPolygon* pXPoly = pImpXPolyPolygon->aXPolyList.GetObject( n );
        aRect.Union( pXPoly->GetBoundRect() );
    }

    return aRect;
}


/*************************************************************************
|*
|*    XPolyPolygon::operator[]()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    TH 28.10.94
|*    Letzte Aenderung  TH 28.10.94
|*
*************************************************************************/

XPolygon& XPolyPolygon::operator[]( sal_uInt16 nPos )
{
    CheckReference();
    return *(pImpXPolyPolygon->aXPolyList.GetObject( nPos ));
}

/*************************************************************************
|*
|*    XPolyPolygon::operator=()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    CL 27.01.93
|*    Letzte Aenderung  CL 27.01.93
|*
*************************************************************************/

XPolyPolygon& XPolyPolygon::operator=( const XPolyPolygon& rXPolyPoly )
{
    rXPolyPoly.pImpXPolyPolygon->nRefCount++;

    if( pImpXPolyPolygon->nRefCount > 1 )
        pImpXPolyPolygon->nRefCount--;
    else
        delete pImpXPolyPolygon;

    pImpXPolyPolygon = rXPolyPoly.pImpXPolyPolygon;
    return *this;
}


/*************************************************************************
|*
|*    XPolyPolygon::operator==()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    CL  27.01.93
|*    Letzte Aenderung  Joe 27.01.93
|*
*************************************************************************/

sal_Bool XPolyPolygon::operator==( const XPolyPolygon& rXPolyPoly ) const
{
    if (pImpXPolyPolygon==rXPolyPoly.pImpXPolyPolygon) return sal_True;
    return *pImpXPolyPolygon == *rXPolyPoly.pImpXPolyPolygon;
}


/*************************************************************************
|*
|*    XPolyPolygon::operator!=()
|*
|*    Beschreibung      POLY.SDW
|*    Ersterstellung    CL  27.01.93
|*    Letzte Aenderung  Joe 27.01.93
|*
*************************************************************************/

sal_Bool XPolyPolygon::operator!=( const XPolyPolygon& rXPolyPoly ) const
{
    if (pImpXPolyPolygon==rXPolyPoly.pImpXPolyPolygon) return sal_False;
    return *pImpXPolyPolygon != *rXPolyPoly.pImpXPolyPolygon;
}

/*************************************************************************
|*
|*    XPolyPolygon::Translate()
|*
|*    Alle Polygone auf den uebergebenen Punkt verschieben
|*    Ersterstellung    ESO 25.01.95
|*    Letzte Aenderung  ESO 25.01.95
|*
*************************************************************************/

void XPolyPolygon::Translate(const Point& rTrans)
{
    CheckReference();

    for (sal_uInt16 i = 0; i < Count(); i++)
        pImpXPolyPolygon->aXPolyList.GetObject(i)->Translate(rTrans);
}

/*************************************************************************
|*
|*    XPolyPolygon::Rotate()
|*
|*    Alle Polygone um den Punkt rCenter drehen, Sinus und Cosinus
|*    muessen uebergeben werden
|*    Ersterstellung    ESO 25.01.95
|*    Letzte Aenderung  ESO 25.01.95
|*
*************************************************************************/

void XPolyPolygon::Rotate(const Point& rCenter, double fSin, double fCos)
{
    CheckReference();

    for (sal_uInt16 i = 0; i < Count(); i++)
        pImpXPolyPolygon->aXPolyList.GetObject(i)->Rotate(rCenter, fSin, fCos);
}

/*************************************************************************
|*
|* Bestimme den linken, unteren Punkt des Polygons und richte das
|* Polygon so aus, dass dieser Punkt auf dem Index 0 liegt
|*
\************************************************************************/

void XPolyPolygon::Rotate20()
{
    CheckReference();

    for (sal_uInt16 i = 0; i < Count(); i++)
        pImpXPolyPolygon->aXPolyList.GetObject(i)->Rotate20();
}

/*************************************************************************
|*
|*    XPolyPolygon::Rotate()
|*
|*    Alle Poylgone um den Punkt rCenter mit dem Winkel nAngle drehen
|*    Winkel in 10tel Grad, Wertebereich 0 - 3600
|*    Ersterstellung    ESO 25.01.95
|*    Letzte Aenderung  ESO 25.01.95
|*
*************************************************************************/

void XPolyPolygon::Rotate(const Point& rCenter, sal_uInt16 nAngle)
{
    nAngle %= 3600;

    if ( nAngle != 0 )
    {
        double fAngle = F_PI * nAngle / 1800;
        double fSin = sin(fAngle);
        double fCos = cos(fAngle);
        Rotate(rCenter, fSin, fCos);
    }
}

/*************************************************************************
|*
|*    XPolyPolygon::Scale()
|*
|*    Alle Polygone in X- und/oder Y-Richtung skalieren
|*    Ersterstellung    ESO 01.02.95
|*    Letzte Aenderung  ESO 01.02.95
|*
*************************************************************************/

void XPolyPolygon::Scale(double fSx, double fSy)
{
    CheckReference();

    for (sal_uInt16 i = 0; i < Count(); i++)
        pImpXPolyPolygon->aXPolyList.GetObject(i)->Scale(fSx, fSy);
}

/*************************************************************************
|*
|*    XPolyPolygon::SlantX()
|*
|*    Alle Polygone in X-Richtung um einen beliebigen Winkel kippen,
|*    bezogen auf eine Referenz-Y-Koordinate
|*    Ersterstellung    ESO 01.02.95
|*    Letzte Aenderung  ESO 01.02.95
|*
*************************************************************************/

void XPolyPolygon::SlantX(long nYRef, double fSin, double fCos)
{
    CheckReference();

    for (sal_uInt16 i = 0; i < Count(); i++)
        pImpXPolyPolygon->aXPolyList.GetObject(i)->SlantX(nYRef, fSin, fCos);
}

/*************************************************************************
|*
|*    XPolyPolygon::SlantY()
|*
|*    Alle Polygone in Y-Richtung um einen beliebigen Winkel kippen,
|*    bezogen auf eine Referenz-X-Koordinate
|*    Ersterstellung    ESO 01.02.95
|*    Letzte Aenderung  ESO 01.02.95
|*
*************************************************************************/

void XPolyPolygon::SlantY(long nXRef, double fSin, double fCos)
{
    CheckReference();

    for (sal_uInt16 i = 0; i < Count(); i++)
        pImpXPolyPolygon->aXPolyList.GetObject(i)->SlantY(nXRef, fSin, fCos);
}

/*************************************************************************
|*
|*    XPolygon::Distort()
|*
|*    XPolygon verzerren, indem die Koordinaten relativ zu einem
|*    Referenzrechteck in ein beliebiges Viereck skaliert werden
|*    Zuordnung der Viereck-Punkte im Polygon zum Referenzrechteck:
|*    0: links oben      0----1
|*    1: rechts oben     |    |
|*    2: rechts unten    3----2
|*    3: links unten
|*    Ersterstellung    ESO 07.07.95
|*    Letzte Aenderung  ESO 07.07.95
|*
*************************************************************************/

void XPolyPolygon::Distort(const Rectangle& rRefRect,
                           const XPolygon& rDistortedRect)
{
    CheckReference();

    for (sal_uInt16 i = 0; i < Count(); i++)
        pImpXPolyPolygon->aXPolyList.GetObject(i)->Distort(rRefRect,
                                                           rDistortedRect);
}

basegfx::B2DPolyPolygon XPolyPolygon::getB2DPolyPolygon() const
{
    basegfx::B2DPolyPolygon aRetval;

    for(sal_uInt16 a(0L); a < Count(); a++)
    {
        const XPolygon& rPoly = (*this)[a];
        aRetval.append(rPoly.getB2DPolygon());
    }

    return aRetval;
}

XPolyPolygon::XPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    DBG_CTOR(XPolyPolygon,NULL);
    pImpXPolyPolygon = new ImpXPolyPolygon( 16, 16 );

    for(sal_uInt32 a(0L); a < rPolyPolygon.count(); a++)
    {
        basegfx::B2DPolygon aCandidate = rPolyPolygon.getB2DPolygon(a);
        XPolygon aNewPoly(aCandidate);
        Insert(aNewPoly);
    }
}

// eof
