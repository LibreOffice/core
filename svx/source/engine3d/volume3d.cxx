/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: volume3d.cxx,v $
 * $Revision: 1.7 $
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
#include <svx/volume3d.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <tools/debug.hxx>

/*************************************************************************
|*
|* Konstruktor 1:                                               |
|* rPos: Zentrum oder minimale Koordinate links, unten, hinten  |__
|*       (abhaengig von bPosIsCenter)                          /
|*
\************************************************************************/

Volume3D::Volume3D(const basegfx::B3DPoint& rPos, const basegfx::B3DPoint& r3DSize, bool bPosIsCenter)
:   basegfx::B3DRange()
{
    if(bPosIsCenter)
    {
        expand(rPos - r3DSize / 2.0);
    }
    else
    {
        expand(rPos);
    }

    expand(getMinimum() + r3DSize);
}

Volume3D::Volume3D(const basegfx::B3DRange& rVol)
:   basegfx::B3DRange(rVol)
{
}

/*************************************************************************
|*
|* Konstruktor 2 - leeres Volumen, Werte als ungueltig markieren
|*
\************************************************************************/

Volume3D::Volume3D()
:   basegfx::B3DRange()
{
}

/*************************************************************************
|*
|* Transformation des Volumens berechnen und als neues Volumen
|* zurueckgeben
|*
\************************************************************************/

Volume3D Volume3D::GetTransformVolume(const basegfx::B3DHomMatrix& rTfMatrix) const
{
    Volume3D aTfVol;

    if(!isEmpty())
    {
        basegfx::B3DPoint aTfVec;
        Vol3DPointIterator aIter(*this, &rTfMatrix);

        while(aIter.Next(aTfVec))
        {
            aTfVol.expand(aTfVec);
        }
    }
    return aTfVol;
}

/*************************************************************************
|*
|* Drahtgitter-Linien fuer das Volumen berechnen und in rPoly3D ablegen
|*
\************************************************************************/

void Volume3D::CreateWireframe(basegfx::B3DPolygon& rPoly3D, const basegfx::B3DHomMatrix* pTf) const
{
    if(isEmpty())
        return;

    basegfx::B3DVector aDiff(getRange());
    basegfx::B3DPolygon aVolPnts;
    sal_uInt32 nZeroCnt(0L);

    // Alle Punkte holen
    Vol3DPointIterator aIter(*this, pTf);
    basegfx::B3DPoint aTfVec;

    while(aIter.Next(aTfVec))
    {
        aVolPnts.append(aTfVec);
    }

    // 0-Ausmasse des BoundVolumes zaehlen
    if(0.0 == aDiff.getX())
        nZeroCnt++;
    if(0.0 == aDiff.getY())
        nZeroCnt++;
    if(0.0 == aDiff.getZ())
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

    rPoly3D.append(aVolPnts.getB3DPoint(0));

    if(nZeroCnt < 3L)
    {
        // wenn keine Ausdehnung, dann nur den ersten Punkt einfuegen
        rPoly3D.append(aVolPnts.getB3DPoint(1L));
        rPoly3D.append(aVolPnts.getB3DPoint(0L));
        rPoly3D.append(aVolPnts.getB3DPoint(4L));
        rPoly3D.append(aVolPnts.getB3DPoint(0L));
        rPoly3D.append(aVolPnts.getB3DPoint(3L));
    }
    if(nZeroCnt < 2L)
    {
        if(nZeroCnt == 0L || aDiff.getX() == 0.0)
        {
            //   4
            //  /
            // 7---6
            // |
            // |
            // 3
            rPoly3D.append(aVolPnts.getB3DPoint(7L));
            rPoly3D.append(aVolPnts.getB3DPoint(6L));
            rPoly3D.append(aVolPnts.getB3DPoint(7L));
            rPoly3D.append(aVolPnts.getB3DPoint(3L));
            rPoly3D.append(aVolPnts.getB3DPoint(7L));
            rPoly3D.append(aVolPnts.getB3DPoint(4L));
        }
        if(nZeroCnt == 0L || (aDiff.getY() == 0.0))
        {
            //     6
            //     | 1
            //     |/
            // 3---2
            rPoly3D.append(aVolPnts.getB3DPoint(2L));
            rPoly3D.append(aVolPnts.getB3DPoint(3L));
            rPoly3D.append(aVolPnts.getB3DPoint(2L));
            rPoly3D.append(aVolPnts.getB3DPoint(6L));
            rPoly3D.append(aVolPnts.getB3DPoint(2L));
            rPoly3D.append(aVolPnts.getB3DPoint(1L));
        }
        if(nZeroCnt == 0L || (aDiff.getZ() == 0.0))
        {
            //   4---5
            //      /|
            //     6 |
            //       1
            rPoly3D.append(aVolPnts.getB3DPoint(5L));
            rPoly3D.append(aVolPnts.getB3DPoint(4L));
            rPoly3D.append(aVolPnts.getB3DPoint(5L));
            rPoly3D.append(aVolPnts.getB3DPoint(1L));
            rPoly3D.append(aVolPnts.getB3DPoint(5L));
            rPoly3D.append(aVolPnts.getB3DPoint(6L));
        }
    }
}

/*************************************************************************
|*
|* Konstruktor des Point-Iterators
|*
\************************************************************************/

Vol3DPointIterator::Vol3DPointIterator(const basegfx::B3DRange& rVol, const basegfx::B3DHomMatrix* pTf)
:   rVolume(rVol),
    pTransform(pTf),
    nIndex(0)
{
    // #i72532# Assert not needed, nothing bad happens when the volume is empty.
    // DBG_ASSERT(!rVol.isEmpty(), "Vol3DPointIterator-Aufruf mit ungueltigem Volume3D!");
    a3DExtent = rVolume.getMaximum() - rVolume.getMinimum();
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

bool Vol3DPointIterator::Next(basegfx::B3DPoint& rVec)
{
    if(nIndex > 7)
    {
        return false;
    }
    else
    {
        rVec = rVolume.getMinimum();

        if(nIndex >= 4)
        {
            rVec.setY(rVec.getY() + a3DExtent.getY());
        }

        switch(nIndex)
        {
            case 6:
            case 2: rVec.setZ(rVec.getZ() + a3DExtent.getZ());
            case 5:
            case 1: rVec.setX(rVec.getX() + a3DExtent.getX());
                    break;
            case 7:
            case 3: rVec.setZ(rVec.getZ() + a3DExtent.getZ());
                    break;
        }
        nIndex++;

        if(pTransform)
        {
            rVec *= *pTransform;
        }

        return true;
    }
}

// eof
