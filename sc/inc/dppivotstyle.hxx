/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "address.hxx"
#include "dpoutput.hxx"

class ScDocument;

namespace sc
{
struct PivotTableStyleInfo;

/// Applies a named pivot table style (resolved from
/// ScDocument::GetTableStyles()) to the cell range described by rRoles. The
/// style's element patterns (wholeTable, headerRow, subheadings, subtotals,
/// page-field cells, etc.) are merged onto the existing cell patterns in
/// ECMA-376 §18.18.83 application order — most-general first, most-specific
/// last so the latter override the former on overlaps. No-op if rStyleInfo
/// has an empty name or the named style is not registered.
void applyPivotTableStyle(ScDocument& rDoc, SCTAB nTab,
                          const PivotTableStyleInfo& rStyleInfo,
                          const ScDPOutput::StyleRoles& rRoles);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
