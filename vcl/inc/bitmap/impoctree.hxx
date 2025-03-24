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

#pragma once

#include <vcl/BitmapColor.hxx>

class ImpErrorQuad
{
    sal_Int16 nRed;
    sal_Int16 nGreen;
    sal_Int16 nBlue;

public:
    ImpErrorQuad()
        : nRed(0)
        , nGreen(0)
        , nBlue(0)
    {
    }

    ImpErrorQuad(const BitmapColor& rColor)
        : nRed(rColor.GetRed() << 5)
        , nGreen(rColor.GetGreen() << 5)
        , nBlue(rColor.GetBlue() << 5)
    {
    }

    inline void operator=(const BitmapColor& rColor);
    inline ImpErrorQuad& operator-=(const BitmapColor& rColor);

    inline void ImplAddColorError1(const ImpErrorQuad& rErrQuad);
    inline void ImplAddColorError3(const ImpErrorQuad& rErrQuad);
    inline void ImplAddColorError5(const ImpErrorQuad& rErrQuad);
    inline void ImplAddColorError7(const ImpErrorQuad& rErrQuad);

    inline BitmapColor ImplGetColor() const;
};

inline void ImpErrorQuad::operator=(const BitmapColor& rColor)
{
    nRed = rColor.GetRed() << 5;
    nGreen = rColor.GetGreen() << 5;
    nBlue = rColor.GetBlue() << 5;
}

inline ImpErrorQuad& ImpErrorQuad::operator-=(const BitmapColor& rColor)
{
    nRed -= rColor.GetRed() << 5;
    nGreen -= rColor.GetGreen() << 5;
    nBlue -= rColor.GetBlue() << 5;

    return *this;
}

inline void ImpErrorQuad::ImplAddColorError1(const ImpErrorQuad& rErrQuad)
{
    nRed += rErrQuad.nRed >> 4;
    nGreen += rErrQuad.nGreen >> 4;
    nBlue += rErrQuad.nBlue >> 4;
}

inline void ImpErrorQuad::ImplAddColorError3(const ImpErrorQuad& rErrQuad)
{
    nRed += rErrQuad.nRed * 3L >> 4;
    nGreen += rErrQuad.nGreen * 3L >> 4;
    nBlue += rErrQuad.nBlue * 3L >> 4;
}

inline void ImpErrorQuad::ImplAddColorError5(const ImpErrorQuad& rErrQuad)
{
    nRed += rErrQuad.nRed * 5L >> 4;
    nGreen += rErrQuad.nGreen * 5L >> 4;
    nBlue += rErrQuad.nBlue * 5L >> 4;
}

inline void ImpErrorQuad::ImplAddColorError7(const ImpErrorQuad& rErrQuad)
{
    nRed += rErrQuad.nRed * 7L >> 4;
    nGreen += rErrQuad.nGreen * 7L >> 4;
    nBlue += rErrQuad.nBlue * 7L >> 4;
}

inline BitmapColor ImpErrorQuad::ImplGetColor() const
{
    return BitmapColor(std::clamp(nRed, sal_Int16(0), sal_Int16(8160)) >> 5,
                       std::clamp(nGreen, sal_Int16(0), sal_Int16(8160)) >> 5,
                       std::clamp(nBlue, sal_Int16(0), sal_Int16(8160)) >> 5);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
