/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "swdllapi.h"
#include <rtl/string.hxx>

class SwTableAutoFormat;
struct SwTableStyleSettings;

namespace sw
{
/// Renders a small grid previewing one named table style with the given row/column roles
/// switched on, encoded as a "data:image/png;base64,..." string for embedding directly in
/// JSON sent to the Online browser.
SW_DLLPUBLIC OString CreateTableStylePreviewDataUri(const SwTableAutoFormat& rStyle,
                                                    const SwTableStyleSettings& rSettings,
                                                    bool bIsPageDark);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
