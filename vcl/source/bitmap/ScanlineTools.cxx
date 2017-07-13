/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "ScanlineTools.hxx"

namespace vcl {
namespace scanline {

bool swapABCDtoCBAD(sal_uInt8* pSource, sal_uInt8* pDestination, sal_Int32 nScanlineSize)
{
    for (sal_Int32 i = 0; i < nScanlineSize; i += 4, pSource += 4, pDestination += 4)
    {
        pDestination[0] = pSource[2];
        pDestination[1] = pSource[1];
        pDestination[2] = pSource[0];
        pDestination[3] = pSource[3];
    }
    return true;
}

}} // end vcl::scanline

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
