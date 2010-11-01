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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"
#include <basegfx/vector/b3ivector.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

namespace basegfx
{
    B3IVector& B3IVector::operator*=( const B3DHomMatrix& rMat )
    {
        mnX = fround( rMat.get(0,0)*mnX + rMat.get(0,1)*mnY + rMat.get(0,2)*mnZ );
        mnY = fround( rMat.get(1,0)*mnX + rMat.get(1,1)*mnY + rMat.get(1,2)*mnZ );
        mnZ = fround( rMat.get(2,0)*mnX + rMat.get(2,1)*mnY + rMat.get(2,2)*mnZ );

        return *this;
    }

    B3IVector operator*( const B3DHomMatrix& rMat, const B3IVector& rVec )
    {
        B3IVector aRes( rVec );
        return aRes*=rMat;
    }
} // end of namespace basegfx

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
