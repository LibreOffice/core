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

#ifndef _B3D_VOLUM_HXX
#include "b3dvolum.hxx"
#endif

namespace binfilter {
/*************************************************************************
|*
|* Konstruktor 1:                                               |
|* rPos: Zentrum oder minimale Koordinate links, unten, hinten  |__
|*       (abhaengig von bPosIsCenter)                          /
|*
\************************************************************************/

B3dVolume::B3dVolume(const Vector3D& rPos, const Vector3D& r3DSize,
                    BOOL bPosIsCenter)
{
    if ( bPosIsCenter )	aMinVec = rPos - r3DSize / 2;
    else				aMinVec = rPos;

    aMaxVec = aMinVec + r3DSize;
}

/*************************************************************************
|*
|* Konstruktor 2 - leeres Volumen, Werte als ungueltig markieren
|*
\************************************************************************/

B3dVolume::B3dVolume() :
    aMinVec( DBL_MAX, DBL_MAX, DBL_MAX),
    aMaxVec(-DBL_MAX,-DBL_MAX,-DBL_MAX)
{
}

/*************************************************************************
|*
|* Volumen zuruecksetzen
|*
\************************************************************************/

void B3dVolume::Reset()
{
    aMinVec = Vector3D( DBL_MAX, DBL_MAX, DBL_MAX);
    aMaxVec = Vector3D(-DBL_MAX,-DBL_MAX,-DBL_MAX);
}

/*************************************************************************
|*
|* testen, ob Volumen vollstaendig gueltige Werte enthaelt
|*
\************************************************************************/

BOOL B3dVolume::IsValid() const
{
    return ( aMinVec.X() !=  DBL_MAX && aMinVec.Y() !=  DBL_MAX &&
             aMinVec.Z() !=  DBL_MAX && aMaxVec.X() != -DBL_MAX &&
             aMaxVec.Y() != -DBL_MAX && aMaxVec.Z() != -DBL_MAX );
}

/*************************************************************************
|*
|* einen 3D-Vektor in das Volumen einschliessen
|*
\************************************************************************/

B3dVolume& B3dVolume::Union(const B3dVolume& rVol2)
{
    if ( rVol2.IsValid() )
    {
        aMinVec.Min(rVol2.aMinVec);
        aMaxVec.Max(rVol2.aMaxVec);
    }
    return *this;
}

/*************************************************************************
|*
|* Vereinigung mit Vector3D
|*
\************************************************************************/

B3dVolume& B3dVolume::Union(const Vector3D& rVec)
{
    aMinVec.Min(rVec);
    aMaxVec.Max(rVec);
    return *this;
}

/*************************************************************************
|*
|* Stream-In-Operator fuer B3dVolume
|*
\************************************************************************/

SvStream& operator>>(SvStream& rIStream, B3dVolume& rB3dVolume)
{
    rIStream >> rB3dVolume.aMinVec;
    rIStream >> rB3dVolume.aMaxVec;

    return rIStream;
}

/*************************************************************************
|*
|* Stream-Out-Operator fuer B3dVolume
|*
\************************************************************************/

SvStream& operator<<(SvStream& rOStream, const B3dVolume& rB3dVolume)
{
    rOStream << rB3dVolume.aMinVec;
    rOStream << rB3dVolume.aMaxVec;

    return rOStream;
}


}//end of namespace binfilter

// eof
