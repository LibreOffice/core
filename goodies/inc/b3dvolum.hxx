/*************************************************************************
 *
 *  $RCSfile: b3dvolum.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:08 $
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

#ifndef _B3D_VOLUM_HXX
#define _B3D_VOLUM_HXX

#ifndef _SVX_VECTOR3D_HXX
#include "vector3d.hxx"
#endif

#ifndef _INC_FLOAT
#include <float.h>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

/*************************************************************************
|*
|* dreidimensionales Volumen, symmetrisch zu den Koordinatenachsen
|*
\************************************************************************/

class B3dVolume
{
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
    Vector3D GetSize() const;
    BOOL IsInside(const Vector3D& rVec);

    friend SvStream& operator>>(SvStream& rIStream, B3dVolume&);
    friend SvStream& operator<<(SvStream& rOStream, const B3dVolume&);
};


#endif          // _B3D_VOLUM_HXX
