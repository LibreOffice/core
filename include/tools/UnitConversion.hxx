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

#include <sal/types.h>

constexpr sal_Int64 convertTwipToMm100(sal_Int64 n)
{
    return (n >= 0) ? (n * 127 + 36) / 72 : (n * 127 - 36) / 72;
}

constexpr sal_Int64 convertMm100ToTwip(sal_Int64 n)
{
    return (n >= 0) ? (n * 72 + 63) / 127 : (n * 72 - 63) / 127;
}

constexpr sal_Int64 convertPointToTwip(sal_Int64 nNumber) { return nNumber * 20; }

constexpr sal_Int64 convertPointToMm100(sal_Int64 nNumber)
{
    return convertTwipToMm100(convertPointToTwip(nNumber));
}

constexpr double convertPointToTwip(double fNumber) { return fNumber * 20.0; }

constexpr double convertPointToMm100(double fNumber) { return fNumber * 35.27777777778; }

// Convert PPT's "master unit" (1/576 inch) to twips
constexpr sal_Int64 convertMasterUnitToTwip(sal_Int64 n) { return n * 2540.0 / 576.0; }

// Convert twips to PPT's "master unit"
constexpr sal_Int64 convertTwipToMasterUnit(sal_Int64 n) { return n / (2540.0 / 576.0); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
