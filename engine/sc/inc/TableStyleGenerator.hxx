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

namespace model
{
class ColorSet;
}
class ScDocument;

/// Generates the 60 default OOXML table styles from the ECMA-376 spec data,
/// resolving theme-relative colors against the given ColorSet.
///
/// The generation is split internally into discrete steps: resolveColors,
/// buildFills, buildBorders, buildFonts, buildDxfPatterns, and assembleTableStyles.
class SC_DLLPUBLIC ScTableStyleGenerator
{
public:
    static void generateDefaultStyles(ScDocument& rDoc, const model::ColorSet& rColorSet);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
