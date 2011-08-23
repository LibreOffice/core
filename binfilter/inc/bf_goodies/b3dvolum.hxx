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

#ifndef _B3D_VOLUM_HXX
#define _B3D_VOLUM_HXX

#include "vector3d.hxx"

#ifndef _INC_FLOAT
#include <float.h>
#endif

#include <tools/stream.hxx>

/*************************************************************************
|*
|* dreidimensionales Volumen, symmetrisch zu den Koordinatenachsen
|*
\************************************************************************/

namespace binfilter {
class Vol3DPointIterator;

class B3dVolume
{
// friend class binfilter::Vol3DPointIterator; //STRIP008 
 friend class Vol3DPointIterator;
 protected:
    Vector3D aMinVec;
    Vector3D aMaxVec;

 public:
    B3dVolume(const Vector3D& rPos, const Vector3D& r3DSize,
        BOOL bPosIsCenter = TRUE);
    B3dVolume();

    void Reset();
    BOOL IsValid() const;

    B3dVolume& Union(const B3dVolume& rVol2);
    B3dVolume& Union(const Vector3D& rVec);

    const Vector3D& MinVec() const { return aMinVec; }
    Vector3D& MinVec() { return aMinVec; }

    const Vector3D& MaxVec() const { return aMaxVec; }
    Vector3D& MaxVec() { return aMaxVec; }

    double GetWidth()  const { return aMaxVec.X() - aMinVec.X(); }
    double GetHeight() const { return aMaxVec.Y() - aMinVec.Y(); }
    double GetDepth()  const { return aMaxVec.Z() - aMinVec.Z(); }

    friend SvStream& operator>>(SvStream& rIStream, B3dVolume&);
    friend SvStream& operator<<(SvStream& rOStream, const B3dVolume&);
};
}//end of namespace binfilter


#endif			// _B3D_VOLUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
