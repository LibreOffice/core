/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "scdllapi.h"

namespace sc::op
{
SC_DLLPUBLIC extern const bool hasAVX512F;

// Plain old data structure, to be used by code compiled with CPU intrinsics without generating any
// code for it (so that code requiring intrinsics doesn't get accidentally selected as the one copy
// when merging duplicates).
struct KahanSumSimple
{
    double m_fSum;
    double m_fError;
};

/* Available methods */
SC_DLLPUBLIC KahanSumSimple executeAVX512F(size_t& i, size_t nSize, const double* pCurrent);
SC_DLLPUBLIC KahanSumSimple executeAVX(size_t& i, size_t nSize, const double* pCurrent);
SC_DLLPUBLIC KahanSumSimple executeSSE2(size_t& i, size_t nSize, const double* pCurrent);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
