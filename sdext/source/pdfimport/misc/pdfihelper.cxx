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


#include <pdfihelper.hxx>

#include <rtl/ustrbuf.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <rtl/math.hxx>

#include <math.h>

using namespace pdfi;
using namespace com::sun::star;

double pdfi::GetAverageTransformationScale(const basegfx::B2DHomMatrix& matrix)
{
    double rotate, shearX;
    basegfx::B2DTuple scale, translation;
    matrix.decompose(scale, translation, rotate, shearX);
    return (fabs(scale.getX()) + fabs(scale.getY())) / 2.0;
}

void pdfi::FillDashStyleProps(PropertyMap& props, const std::vector<double>& dashArray, double scale)
{
    size_t pairCount = dashArray.size() / 2;

    double distance = 0.0;
    for (size_t i = 0; i < pairCount; i++)
        distance += dashArray[i * 2 + 1];
    distance /= pairCount;

    props[u"draw:style"_ustr] = "rect";
    props[u"draw:distance"_ustr] = convertPixelToUnitString(distance * scale);

    int dotStage = 0;
    int dotCounts[3] = {0, 0, 0};
    double dotLengths[3] = {0.0, 0.0, 0.0};

    for (size_t i = 0; i < pairCount; i++)
    {
        if (!rtl::math::approxEqual(dotLengths[dotStage], dashArray[i * 2]))
        {
            dotStage++;
            if (dotStage == 3)
                break;

            dotCounts[dotStage] = 1;
            dotLengths[dotStage] = dashArray[i * 2];
        }
        else
        {
            dotCounts[dotStage]++;
        }
    }

    for (int i = 1; i < 3; i++)
    {
        if (dotCounts[i] == 0)
            continue;
        props["draw:dots" + OUString::number(i)] = OUString::number(dotCounts[i]);
        props["draw:dots" + OUString::number(i) + "-length"] =
            convertPixelToUnitString(dotLengths[i] * scale);
    }
}

OUString pdfi::getColorString( const rendering::ARGBColor& rCol )
{
    OUStringBuffer aBuf( 7 );
    const sal_uInt8 nRed  ( sal::static_int_cast<sal_Int8>( basegfx::fround( rCol.Red * 255.0 ) ) );
    const sal_uInt8 nGreen( sal::static_int_cast<sal_Int8>( basegfx::fround( rCol.Green * 255.0 ) ) );
    const sal_uInt8 nBlue ( sal::static_int_cast<sal_Int8>( basegfx::fround( rCol.Blue * 255.0 ) ) );
    aBuf.append( '#' );
    if( nRed < 16 )
        aBuf.append( '0' );
    aBuf.append( sal_Int32(nRed), 16 );
    if( nGreen < 16 )
        aBuf.append( '0' );
    aBuf.append( sal_Int32(nGreen), 16 );
    if( nBlue < 16 )
        aBuf.append( '0' );
    aBuf.append( sal_Int32(nBlue), 16 );

    return aBuf.makeStringAndClear();
}

OUString pdfi::getPercentString(double value)
{
    return OUString::number(value) + "%";
}

OUString pdfi::unitMMString( double fMM )
{
    return OUString::number(rtl_math_round( fMM, 2, rtl_math_RoundingMode_Floor )) + "mm";
}

OUString pdfi::convertPixelToUnitString( double fPix )
{
    return OUString::number( rtl_math_round( convPx2mm( fPix ), 2, rtl_math_RoundingMode_Floor ) ) + "mm";
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
