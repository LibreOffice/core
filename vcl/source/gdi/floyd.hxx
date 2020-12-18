/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#define CALC_ERRORS                                                             \
                        nTemp = p1T[nX++] >> 12;                              \
                        nBErr = MinMax( nTemp, 0, 255 );                        \
                        nBErr = nBErr - FloydIndexMap[ nBC = FloydMap[nBErr] ]; \
                        nTemp = p1T[nX++] >> 12;                              \
                        nGErr = MinMax( nTemp, 0, 255 );                        \
                        nGErr = nGErr - FloydIndexMap[ nGC = FloydMap[nGErr] ]; \
                        nTemp = p1T[nX] >> 12;                                \
                        nRErr = MinMax( nTemp, 0, 255 );                        \
                        nRErr = nRErr - FloydIndexMap[ nRC = FloydMap[nRErr] ];

#define CALC_TABLES3                                        \
                        p2T[nX++] += FloydError3[nBErr];    \
                        p2T[nX++] += FloydError3[nGErr];    \
                        p2T[nX++] += FloydError3[nRErr];

#define CALC_TABLES5                                        \
                        p2T[nX++] += FloydError5[nBErr];    \
                        p2T[nX++] += FloydError5[nGErr];    \
                        p2T[nX++] += FloydError5[nRErr];

#define CALC_TABLES7                                        \
                        p1T[++nX] += FloydError7[nBErr];    \
                        p2T[nX++] += FloydError1[nBErr];    \
                        p1T[nX] += FloydError7[nGErr];      \
                        p2T[nX++] += FloydError1[nGErr];    \
                        p1T[nX] += FloydError7[nRErr];      \
                        p2T[nX] += FloydError1[nRErr];

const tools::Long FloydMap[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

const tools::Long FloydError1[61] =
{
    -7680, -7424, -7168, -6912, -6656, -6400, -6144,
    -5888, -5632, -5376, -5120, -4864, -4608, -4352,
    -4096, -3840, -3584, -3328, -3072, -2816, -2560,
    -2304, -2048, -1792, -1536, -1280, -1024, -768,
    -512, -256, 0, 256, 512, 768, 1024, 1280, 1536,
    1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584,
    3840, 4096, 4352, 4608, 4864, 5120, 5376, 5632,
    5888, 6144, 6400, 6656, 6912, 7168, 7424, 7680
};

const tools::Long FloydError3[61] =
{
    -23040, -22272, -21504, -20736, -19968, -19200,
    -18432, -17664, -16896, -16128, -15360, -14592,
    -13824, -13056, -12288, -11520, -10752, -9984,
    -9216, -8448, -7680, -6912, -6144, -5376, -4608,
    -3840, -3072, -2304, -1536, -768, 0, 768, 1536,
    2304, 3072, 3840, 4608, 5376, 6144, 6912, 7680,
    8448, 9216, 9984, 10752, 11520, 12288, 13056,
    13824, 14592, 15360, 16128, 16896, 17664, 18432,
    19200, 19968, 20736, 21504, 22272, 23040
};

const tools::Long FloydError5[61] =
{
    -38400, -37120, -35840, -34560, -33280, -32000,
    -30720, -29440, -28160, -26880, -25600, -24320,
    -23040, -21760, -20480, -19200, -17920, -16640,
    -15360, -14080, -12800, -11520, -10240, -8960,
    -7680, -6400, -5120, -3840, -2560, -1280,   0,
    1280, 2560, 3840, 5120, 6400, 7680, 8960, 10240,
    11520, 12800, 14080, 15360, 16640, 17920, 19200,
    20480, 21760, 23040, 24320, 25600, 26880, 28160,
    29440, 30720, 32000, 33280, 34560, 35840, 37120,
    38400
};

const tools::Long FloydError7[61] =
{
    -53760, -51968, -50176, -48384, -46592, -44800,
    -43008, -41216, -39424, -37632, -35840, -34048,
    -32256, -30464, -28672, -26880, -25088, -23296,
    -21504, -19712, -17920, -16128, -14336, -12544,
    -10752, -8960, -7168, -5376, -3584, -1792,  0,
    1792, 3584, 5376, 7168, 8960, 10752, 12544, 14336,
    16128, 17920, 19712, 21504, 23296, 25088, 26880,
    28672, 30464, 32256, 34048, 35840, 37632, 39424,
    41216, 43008, 44800, 46592, 48384, 50176, 51968,
    53760
};

const tools::Long FloydIndexMap[6] =
{
    -30,  21, 72, 123, 174, 225
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
