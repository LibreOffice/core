/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vcl/virdev.hxx>

#include <chrono>

class VCL_DLLPUBLIC Benchmark
{
    ScopedVclPtr<VirtualDevice> mpVirtualDevice;
    tools::Rectangle maVDRectangle;
    std::chrono::steady_clock::time_point m_xStart;
    std::chrono::steady_clock::time_point m_xEnd;

    static const Color constBackgroundColor;
    static const Color constLineColor;
    static const Color constFillColor;

    void initialSetup(tools::Long nWidth, tools::Long nHeight, Color aColor);

public:
    sal_Int64 getElapsedTime();
    Bitmap setupWavelines();
    Bitmap setupMultiplePolygonsWithPolyPolygon();
};
