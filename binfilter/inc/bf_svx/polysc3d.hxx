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

#ifndef _E3D_POLYSC3D_HXX
#define _E3D_POLYSC3D_HXX

#ifndef _SVDPAGE_HXX
#include <bf_svx/svdpage.hxx>
#endif

#ifndef _E3D_SCENE3D_HXX
#include <bf_svx/scene3d.hxx>
#endif

#ifndef _B3D_HMATRIX_HXX
#include <bf_goodies/hmatrix.hxx>
#endif
namespace binfilter {

/*************************************************************************
|*
|* 3D-Szene mit Darstellung durch 2D-Polygone
|*
\************************************************************************/

class E3dPolyScene : public E3dScene
{
public:
    TYPEINFO();
    E3dPolyScene();

    virtual UINT16 GetObjIdentifier() const;

    // Zeichenmethode
    virtual FASTBOOL Paint(ExtOutputDevice&, const SdrPaintInfoRec&) const;

    // Die Kontur fuer TextToContour


protected:



};

}//end of namespace binfilter
#endif          // _E3D_POLYSC3D_HXX
