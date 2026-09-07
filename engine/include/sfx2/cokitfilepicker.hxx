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

#include <sfx2/dllapi.h>
#include <rtl/ustring.hxx>
#include <functional>
#include <optional>
#include <vector>

struct COKitFilePickerProvider;

namespace sfx2::COKitFilePicker
{
/** One file-type filter for a requested file picker: a name the picker can show, and the
    wildcards that match the type, separated by semicolons ("*.png;*.jpg"). */
struct Filter
{
    OUString sName;
    OUString sWildcards;
};

/** Remember the app's native file picker. The struct is copied; nullptr forgets the
    current provider. */
SFX2_DLLPUBLIC void installProvider(const COKitFilePickerProvider* pProvider);

/** Whether this is a COKit app with a native file picker installed. */
SFX2_DLLPUBLIC bool isAvailable();

/** Ask the app's native file picker for a file. Returns at once; rOnPicked runs later on
    the engine's main-loop thread, with the picked file's URL, or without a value when the
    user cancelled. Does nothing when isAvailable() is false. */
SFX2_DLLPUBLIC void pick(const OUString& rTitle, const std::vector<Filter>& rFilters,
                         std::function<void(std::optional<OUString>)> aOnPicked);

/** Ask the app's native file picker for a file and dispatch rCommand with the picked
    file's URL as the string parameter named rArgument, on the view that is current now.
    A cancelled pick dispatches nothing.

    Returns true when the picker was asked, so the caller skips its own file dialog;
    false when isAvailable() is false. */
SFX2_DLLPUBLIC bool requestAndRedispatch(const OUString& rCommand, const OUString& rArgument,
                                         const std::vector<Filter>& rFilters,
                                         const OUString& rTitle);

/** The graphic import formats as picker filters, one entry per format the graphic filter can
    load. */
SFX2_DLLPUBLIC std::vector<Filter> graphicImportFilters();

/** The import formats of the given document factory (for example Writer documents) as picker
    filters, one entry per import filter. */
SFX2_DLLPUBLIC std::vector<Filter> documentImportFilters(const OUString& rFactoryName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
