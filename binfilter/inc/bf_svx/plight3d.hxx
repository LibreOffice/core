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

#ifndef _E3D_PLIGHT3D_HXX
#define _E3D_PLIGHT3D_HXX

#include "light3d.hxx"
namespace binfilter {

/*************************************************************************
|*
|* Punktlichtquelle, die gleichmaessig in alle Richtungen strahlt, und
|* zwar von der von E3dLight geerbten Position aus
|*
\************************************************************************/

class E3dPointLight : public E3dLight
{
 protected:
    virtual void CreateLightObj();

 public:
    TYPEINFO();
    E3dPointLight(const Vector3D& rPos,
                  const Color&    rColor,
                  double          fLightIntensity = 1.0);

    E3dPointLight()
    {
    }

    virtual ~E3dPointLight();


    virtual FASTBOOL CalcLighting(Color& rNewColor,
                                  const Vector3D& rPnt,
                                  const Vector3D& rPntNormal,
                                  const Color& rPntColor);
};



}//end of namespace binfilter
#endif			// _E3D_PLIGHT3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
