/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <sal/types.h>
namespace sc
{
/** The ID of a sheet view */
typedef sal_Int32 SheetViewID;

/** Defines the value to identify the default view of the sheet */
constexpr SheetViewID DefaultSheetViewID = -1;

/** Invalid sheet view ID */
constexpr SheetViewID InvalidSheetViewID = SAL_MIN_INT32;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
