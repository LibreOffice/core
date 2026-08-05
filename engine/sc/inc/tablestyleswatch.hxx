/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <tools/color.hxx>

#include <utility>
#include <vector>

// A table-style gallery swatch resolved to the values its SVG template needs. The
// index selects one of the built-in swatch templates. Each replacement is a
// placeholder token (for example "${headerColor}") paired with the string that
// takes its place; together they are the full set the chosen template contains, so
// a consumer only has to substitute each pair to obtain the finished SVG.
struct ScTableStyleSwatch
{
    sal_Int32 nTemplateIndex = 0;
    std::vector<std::pair<OUString, OUString>> aReplacements;
};

// Resolve a style's swatch from its four defining fills. The colour derivation
// (accent, contrast strokes, separator) and template selection happen here, so the
// desktop bitmap and the online client both draw from one identical set of values
// and cannot drift apart.
ScTableStyleSwatch ScComputeTableStyleSwatch(const OUString& rName, bool bIsBuiltin,
                                             const Color& rHeader, const Color& rBody,
                                             const Color& rFirstStripe, const Color& rSecondStripe);

// The swatch SVG: the selected template with every placeholder replaced.
OUString ScApplyTableStyleSwatch(const ScTableStyleSwatch& rSwatch);

// The swatch SVG templates, so the online client can be handed them once and fill
// them with the values it is sent. The index matches ScTableStyleSwatch::nTemplateIndex.
sal_Int32 ScTableStyleSvgTemplateCount();
OUString ScTableStyleSvgTemplate(sal_Int32 nIndex);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
