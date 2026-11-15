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
#include <com/sun/star/beans/PropertyValue.hpp>
#include <rtl/ustring.hxx>
#include <functional>
#include <optional>
#include <set>
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

/** Whether the picker gives the file the user picked, where it stays, rather than a copy of it,
    so that a document may link to it. */
SFX2_DLLPUBLIC bool picksFilesInPlace();

/** Whether the app can ask the user where a new file goes, a file the document may link to. */
SFX2_DLLPUBLIC bool canPickSaveLocation();

/** Whether the host can give a file to link to that the engine cannot read where it is, with
    the URL the document links to and a copy to read. */
SFX2_DLLPUBLIC bool canPickLinkTarget();

/** The answer of a pick of a file to link to: the URL the document links to, absolute or
    relative to the document, and the URL of the copy the engine reads now. */
struct LinkTarget
{
    OUString sLinkUrl;
    OUString sReadUrl;
};

/** Ask the app's native file picker for a file. Returns at once; rOnPicked runs later on
    the engine's main-loop thread, with the picked file's URL, or without a value when the
    user cancelled. Does nothing when isAvailable() is false. */
SFX2_DLLPUBLIC void pick(const OUString& rTitle, const std::vector<Filter>& rFilters,
                         std::function<void(std::optional<OUString>)> aOnPicked);

/** Ask the app's native save-file picker where a new file goes, starting with the folder and
    the name of rSuggestedUrl. Returns at once; rOnPicked runs later on the engine's main-loop
    thread, with the URL of the file to create, or without a value when the user cancelled.
    Does nothing when canPickSaveLocation() is false. */
SFX2_DLLPUBLIC void pickSaveLocation(const OUString& rTitle, const OUString& rSuggestedUrl,
                                     const std::vector<Filter>& rFilters,
                                     std::function<void(std::optional<OUString>)> aOnPicked);

/** Ask the host for a file to link to. Returns at once; rOnPicked runs later on the engine's
    main-loop thread, with the link target, or without a value when the user cancelled. Does
    nothing when canPickLinkTarget() is false. */
SFX2_DLLPUBLIC void pickLinkTarget(const OUString& rTitle, const std::vector<Filter>& rFilters,
                                   std::function<void(std::optional<LinkTarget>)> aOnPicked);

/** Ask the app's native file picker for a file and dispatch rCommand with the picked
    file's URL as the string parameter named rArgument, followed by rExtraArguments, on the
    view that is current now. A cancelled pick dispatches nothing.

    Returns true when the picker was asked, so the caller skips its own file dialog;
    false when isAvailable() is false. */
SFX2_DLLPUBLIC bool
requestAndRedispatch(const OUString& rCommand, const OUString& rArgument,
                     const std::vector<Filter>& rFilters, const OUString& rTitle,
                     const std::vector<css::beans::PropertyValue>& rExtraArguments = {});

/** Joins wildcards with semicolons ("*.bmp;*.emf;..."), the form a picker filter and
    WildCard take. */
SFX2_DLLPUBLIC OUString joinWildcards(const std::set<OUString>& rWildcards);

/** The graphic import formats as lowercase wildcards, each once. */
SFX2_DLLPUBLIC std::set<OUString> graphicImportWildcards();

/** graphicImportWildcards() as one unnamed picker filter. */
SFX2_DLLPUBLIC std::vector<Filter> graphicImportFilters();

/** The import formats of the given document factory (for example Writer documents) as picker
    filters, one entry per import filter. */
SFX2_DLLPUBLIC std::vector<Filter> documentImportFilters(const OUString& rFactoryName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
