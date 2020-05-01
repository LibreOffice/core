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

constexpr sal_Int64 convertTwipToMm100(sal_Int64 n)
{
    return (n >= 0) ? (n * 127 + 36) / 72 : (n * 127 - 36) / 72;
}

constexpr sal_Int64 convertPointToMm100(sal_Int64 n) { return convertTwipToMm100(n * 20); }

constexpr sal_Int64 convertMm100ToTwip(sal_Int64 n)
{
    return (n >= 0) ? (n * 72 + 63) / 127 : (n * 72 - 63) / 127;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
