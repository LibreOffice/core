/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <Linear3DTransformation.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Sequence;

namespace chart
{

    Linear3DTransformation::Linear3DTransformation( const drawing::HomogenMatrix& rHomMatrix, bool bSwapXAndY )
    : m_Matrix(rHomMatrix)
    , m_bSwapXAndY(bSwapXAndY)
{}

Linear3DTransformation::~Linear3DTransformation()
{}

// ____ XTransformation2 ____
css::drawing::Position3D Linear3DTransformation::transform(
                        const Sequence< double >& rSourceValues ) const
{
    double fX = rSourceValues[0];
    double fY = rSourceValues[1];
    double fZ = rSourceValues[2];
    if(m_bSwapXAndY)
        std::swap(fX,fY);
    css::drawing::Position3D aNewVec;
    double fZwi;

    fZwi = m_Matrix.Line1.Column1 * fX
         + m_Matrix.Line1.Column2 * fY
         + m_Matrix.Line1.Column3 * fZ
         + m_Matrix.Line1.Column4;
    aNewVec.PositionX = fZwi;

    fZwi = m_Matrix.Line2.Column1 * fX
         + m_Matrix.Line2.Column2 * fY
         + m_Matrix.Line2.Column3 * fZ
         + m_Matrix.Line2.Column4;
    aNewVec.PositionY = fZwi;

    fZwi = m_Matrix.Line3.Column1 * fX
         + m_Matrix.Line3.Column2 * fY
         + m_Matrix.Line3.Column3 * fZ
         + m_Matrix.Line3.Column4;
    aNewVec.PositionZ = fZwi;

    fZwi = m_Matrix.Line4.Column1 * fX
         + m_Matrix.Line4.Column2 * fY
         + m_Matrix.Line4.Column3 * fZ
         + m_Matrix.Line4.Column4;
    if(fZwi != 1.0 && fZwi != 0.0)
    {
        aNewVec.PositionX /= fZwi;
        aNewVec.PositionY /= fZwi;
        aNewVec.PositionZ /= fZwi;
    }
    return aNewVec;
}

css::drawing::Position3D Linear3DTransformation::transform(
                        const css::drawing::Position3D& rSourceValues ) const
{
    double fX = rSourceValues.PositionX;
    double fY = rSourceValues.PositionY;
    double fZ = rSourceValues.PositionZ;
    if(m_bSwapXAndY)
        std::swap(fX,fY);
    css::drawing::Position3D aNewVec;
    double fZwi;

    fZwi = m_Matrix.Line1.Column1 * fX
         + m_Matrix.Line1.Column2 * fY
         + m_Matrix.Line1.Column3 * fZ
         + m_Matrix.Line1.Column4;
    aNewVec.PositionX = fZwi;

    fZwi = m_Matrix.Line2.Column1 * fX
         + m_Matrix.Line2.Column2 * fY
         + m_Matrix.Line2.Column3 * fZ
         + m_Matrix.Line2.Column4;
    aNewVec.PositionY = fZwi;

    fZwi = m_Matrix.Line3.Column1 * fX
         + m_Matrix.Line3.Column2 * fY
         + m_Matrix.Line3.Column3 * fZ
         + m_Matrix.Line3.Column4;
    aNewVec.PositionZ = fZwi;

    fZwi = m_Matrix.Line4.Column1 * fX
         + m_Matrix.Line4.Column2 * fY
         + m_Matrix.Line4.Column3 * fZ
         + m_Matrix.Line4.Column4;
    if(fZwi != 1.0 && fZwi != 0.0)
    {
        aNewVec.PositionX /= fZwi;
        aNewVec.PositionY /= fZwi;
        aNewVec.PositionZ /= fZwi;
    }
    return aNewVec;
}

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
