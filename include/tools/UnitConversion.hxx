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

// Convert PPT's "master unit" (1/576 inch) to twips
constexpr sal_Int64 convertMasterUnitToTwip(sal_Int64 n) { return n * 2540.0 / 576.0; }

// Convert twips to PPT's "master unit"
constexpr sal_Int64 convertTwipToMasterUnit(sal_Int64 n) { return n / (2540.0 / 576.0); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
