/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3dgeometry.cxx,v $
 *
 * $Revision: 1.2 $
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

#include <osl/diagnose.h>
#include <basegfx/polygon/b3dgeometry.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    B3DGeometry::B3DGeometry()
    :   mbUnifiedVectorValid(false)
    {
    }

    B3DGeometry::~B3DGeometry()
    {
    }

    bool B3DGeometry::operator==(const B3DGeometry& rGeometry) const
    {
        return (maPolyPolygon == maPolyPolygon
            && maPolygonTo3D == maPolygonTo3D
            && maPolyNormal == maPolyNormal
            && maPolyTexture == maPolyTexture
    }

} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
