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

#ifndef _E3D_DLIGHT3D_HXX
#define _E3D_DLIGHT3D_HXX

#ifndef _E3D_LIGHT3D_HXX
#include <bf_svx/light3d.hxx>
#endif
namespace binfilter {

/*************************************************************************
|*
|* gerichtete Lichtquelle, aDirection zeigt ZUR Lichtquelle (also den
|* Lichtstrahlen entgegensetzt); die Position (geerbt von E3dLight) hat
|* auf die Lichtberechnung keinen Einfluss, sondern dient nur der
|* Anordnung in der Szene (fuer evtl. spaetere interaktive Bearbeitung)
|*
\************************************************************************/

class E3dDistantLight : public E3dLight
{
 protected:
    Vector3D aDirection;

    virtual void CreateLightObj();

 public:
    TYPEINFO();
    E3dDistantLight(const Vector3D& rPos,
                    const Vector3D& rDirection,
                    const Color&    rColor,
                    double          fLightIntensity = 1.0);

    E3dDistantLight() :
        aDirection(Vector3D(0, 1, 0))
    {
    }

    virtual ~E3dDistantLight();

    virtual UINT16 GetObjIdentifier() const;

    virtual FASTBOOL CalcLighting(Color& rNewColor,
                                  const Vector3D& rPnt,
                                  const Vector3D& rPntNormal,
                                  const Color& rPntColor);

    const Vector3D& GetDirection() const { return aDirection; }
    void SetDirection(const Vector3D& rNewDir);

    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);

};

}//end of namespace binfilter
#endif			// _E3D_DLIGHT3D_HXX
