/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_SCANLINETOOLS_HXX
#define INCLUDED_VCL_INC_SCANLINETOOLS_HXX

#include <vcl/dllapi.h>

namespace vcl {
namespace scanline {

bool VCL_DLLPUBLIC swapABCDtoCBAD(sal_uInt8* pSource, sal_uInt8* pDestination, sal_Int32 nScanlineSize);
bool VCL_DLLPUBLIC swapABCDtoCBAD_SSE2(sal_uInt8* pSource, sal_uInt8* pDestination, sal_Int32 nScanlineSize);
bool VCL_DLLPUBLIC swapABCDtoCBAD_SSSE3(sal_uInt8* pSource, sal_uInt8* pDestination, sal_Int32 nScanlineSize);
bool VCL_DLLPUBLIC swapABCDtoCBAD_AVX2(sal_uInt8* pSource, sal_uInt8* pDestination, sal_Int32 nScanlineSize);

}} // end vcl::scanline

#endif // INCLUDED_VCL_INC_BITMAPSYMMETRYCHECK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
