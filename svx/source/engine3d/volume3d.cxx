/*************************************************************************
 *
 *  $RCSfile: volume3d.cxx,v $
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

#ifndef _VOLUME3D_HXX
#include "volume3d.hxx"
#endif

#ifndef _POLY3D_HXX
#include "poly3d.hxx"
#endif

#ifndef _SVX_MATRIX3D_HXX
#include "matrix3d.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

/*************************************************************************
|*
|* Konstruktor 1:                                               |
|* rPos: Zentrum oder minimale Koordinate links, unten, hinten  |__
|*       (abhaengig von bPosIsCenter)                          /
|*
\************************************************************************/

Volume3D::Volume3D(const Vector3D& rPos, const Vector3D& r3DSize, BOOL bPosIsCenter)
:   B3dVolume(rPos, r3DSize, bPosIsCenter)
{
}

/*************************************************************************
|*
|* Konstruktor 2 - leeres Volumen, Werte als ungueltig markieren
|*
\************************************************************************/

Volume3D::Volume3D() : B3dVolume()
{
}

/*************************************************************************
|*
|* Transformation des Volumens berechnen und als neues Volumen
|* zurueckgeben
|*
\************************************************************************/

Volume3D Volume3D::GetTransformVolume(const Matrix4D& rTfMatrix) const
{
    Volume3D aTfVol;

    if(IsValid())
    {
        Vector3D aTfVec;
        Vol3DPointIterator aIter(*this, &rTfMatrix);

        while(aIter.Next(aTfVec))
            aTfVol.Union(aTfVec);
    }
    return aTfVol;
}

/*************************************************************************
|*
|* Drahtgitter-Linien fuer das Volumen berechnen und in rPoly3D ablegen
|*
\************************************************************************/

void Volume3D::CreateWireframe(Polygon3D& rPoly3D, const Matrix4D* pTf) const
{
    if(!IsValid())
        return;

    Vector3D aDiff = aMaxVec - aMinVec;
    Polygon3D aVolPnts(8);
    UINT16 nZeroCnt(0);
    UINT16 nIdx = rPoly3D.GetPointCount();

    // Alle Punkte holen
    Vol3DPointIterator aIter(*this, pTf);
    Vector3D aTfVec;
    UINT16 i(0);

    while(aIter.Next(aTfVec))
        aVolPnts[i++] = aTfVec;

    // 0-Ausmasse des BoundVolumes zaehlen
    for(i = 0; i < 3; i++)
        if(aDiff[i] == 0)
            nZeroCnt++;

    // Die drei Ecksegemente des Volumens mit je drei Linien ausgeben;
    // falls Koordinatenanteile 0 sind, nicht alle Segmente verwenden,
    // um das gegenseitige Ausloeschen bei XOR-Ausgabe zu verhindern
    //   4
    //   |     Dieses Segment immer
    //   |
    //   0---1
    //  /
    // 3
    // Die Liniensegmente eines Segments werden immer in der Reihenfolge
    // X-, Y- und dann Z-Richtung ausgegeben (gilt natuerlich nur fuer
    // untransformierte Koordinaten)

    rPoly3D[nIdx++] = aVolPnts[0];

    if(nZeroCnt < 3)
    {
        // wenn keine Ausdehnung, dann nur den ersten Punkt einfuegen
        rPoly3D[nIdx++] = aVolPnts[1];
        rPoly3D[nIdx++] = aVolPnts[0];
        rPoly3D[nIdx++] = aVolPnts[4];
        rPoly3D[nIdx++] = aVolPnts[0];
        rPoly3D[nIdx++] = aVolPnts[3];
    }
    if(nZeroCnt < 2)
    {
        if(nZeroCnt == 0 || aDiff.X() == 0)
        {
            //   4
            //  /
            // 7---6
            // |
            // |
            // 3
            rPoly3D[nIdx++] = aVolPnts[7];
            rPoly3D[nIdx++] = aVolPnts[6];
            rPoly3D[nIdx++] = aVolPnts[7];
            rPoly3D[nIdx++] = aVolPnts[3];
            rPoly3D[nIdx++] = aVolPnts[7];
            rPoly3D[nIdx++] = aVolPnts[4];
        }
        if(nZeroCnt == 0 || (aDiff.Y() == 0))
        {
            //     6
            //     | 1
            //     |/
            // 3---2
            rPoly3D[nIdx++] = aVolPnts[2];
            rPoly3D[nIdx++] = aVolPnts[3];
            rPoly3D[nIdx++] = aVolPnts[2];
            rPoly3D[nIdx++] = aVolPnts[6];
            rPoly3D[nIdx++] = aVolPnts[2];
            rPoly3D[nIdx++] = aVolPnts[1];
        }
        if(nZeroCnt == 0 || (aDiff.Z() == 0))
        {
            //   4---5
            //      /|
            //     6 |
            //       1
            rPoly3D[nIdx++] = aVolPnts[5];
            rPoly3D[nIdx++] = aVolPnts[4];
            rPoly3D[nIdx++] = aVolPnts[5];
            rPoly3D[nIdx++] = aVolPnts[1];
            rPoly3D[nIdx++] = aVolPnts[5];
            rPoly3D[nIdx++] = aVolPnts[6];
        }
    }
}

/*************************************************************************
|*
|* Konstruktor des Point-Iterators
|*
\************************************************************************/

Vol3DPointIterator::Vol3DPointIterator(const Volume3D& rVol, const Matrix4D* pTf)
:   rVolume(rVol),
    pTransform(pTf),
    nIndex(0)
{
    DBG_ASSERT(rVol.IsValid(), "Vol3DPointIterator-Aufruf mit ungueltigem Volume3D!");
    a3DExtent = rVolume.aMaxVec - rVolume.aMinVec;
}

/*************************************************************************
|*
|* Gibt die einzelnen Punkte des (ggf. transformierten) Volumens zurueck
|*
|*    4---5  -> Reihenfolge der Punktausgabe (untransformiert)
|*   /|  /|
|*  7---6 |
|*  | 0-|-1
|*  |/  |/
|*  3---2
|*
\************************************************************************/

BOOL Vol3DPointIterator::Next(Vector3D& rVec)
{
    if(nIndex > 7)
    {
        return FALSE;
    }
    else
    {
        rVec = rVolume.aMinVec;

        if(nIndex >= 4)
            rVec.Y() += a3DExtent.Y();

        switch(nIndex)
        {
            case 6:
            case 2: rVec.Z() += a3DExtent.Z();
            case 5:
            case 1: rVec.X() += a3DExtent.X();
                    break;
            case 7:
            case 3: rVec.Z() += a3DExtent.Z();
                    break;
        }
        nIndex++;

        if(pTransform)
            rVec *= *pTransform;

        return TRUE;
    }
}


