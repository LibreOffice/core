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
#include "precompiled_chart2.hxx"
#include "Linear3DTransformation.hxx"
#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;

namespace chart
{

    Linear3DTransformation::Linear3DTransformation( const drawing::HomogenMatrix& rHomMatrix, bool bSwapXAndY )
    : m_Matrix(rHomMatrix)
    , m_bSwapXAndY(bSwapXAndY)
{}

Linear3DTransformation::~Linear3DTransformation()
{}

// ____ XTransformation ____
Sequence< double > SAL_CALL Linear3DTransformation::transform(
                        const Sequence< double >& rSourceValues )
    throw (RuntimeException,
           lang::IllegalArgumentException)
{
    double fX = rSourceValues[0];
    double fY = rSourceValues[1];
    double fZ = rSourceValues[2];
    if(m_bSwapXAndY)
        std::swap(fX,fY);
    Sequence< double > aNewVec(3);
    double fZwi;

    fZwi = m_Matrix.Line1.Column1 * fX
         + m_Matrix.Line1.Column2 * fY
         + m_Matrix.Line1.Column3 * fZ
         + m_Matrix.Line1.Column4;
    aNewVec[0] = fZwi;

    fZwi = m_Matrix.Line2.Column1 * fX
         + m_Matrix.Line2.Column2 * fY
         + m_Matrix.Line2.Column3 * fZ
         + m_Matrix.Line2.Column4;
    aNewVec[1] = fZwi;

    fZwi = m_Matrix.Line3.Column1 * fX
         + m_Matrix.Line3.Column2 * fY
         + m_Matrix.Line3.Column3 * fZ
         + m_Matrix.Line3.Column4;
    aNewVec[2] = fZwi;

    fZwi = m_Matrix.Line4.Column1 * fX
         + m_Matrix.Line4.Column2 * fY
         + m_Matrix.Line4.Column3 * fZ
         + m_Matrix.Line4.Column4;
    if(fZwi != 1.0 && fZwi != 0.0)
    {
        aNewVec[0] /= fZwi;
        aNewVec[1] /= fZwi;
        aNewVec[2] /= fZwi;
    }
    return aNewVec;
}

sal_Int32 SAL_CALL Linear3DTransformation::getSourceDimension()
    throw (RuntimeException)
{
    return 3;
}

sal_Int32 SAL_CALL Linear3DTransformation::getTargetDimension()
    throw (RuntimeException)
{
    return 3;
}


}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
