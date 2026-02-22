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

#include "scdllapi.h"
#include <tools/simdplatform.hxx>
#include <cstddef>

#ifdef LO_X86_SIMD_AVAILABLE
namespace sc::op
{
SC_DLLPUBLIC void executeSSE2(size_t& i, size_t nSize, const double* pCurrent, double& outSum,
                              double& outErr);
SC_DLLPUBLIC void executeAVX(size_t& i, size_t nSize, const double* pCurrent, double& outSum,
                             double& outErr);
SC_DLLPUBLIC void executeAVX512F(size_t& i, size_t nSize, const double* pCurrent, double& outSum,
                                 double& outErr);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
