/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <functional>
#include <string>

/// The functions that carry the outcome of one convert-to request back to a caller in this process,
/// in place of a response written to a socket. Each function is empty until it is installed, and is
/// called at most once.
struct ConvertResponders
{
    /// The outcome of an extract request: true and the extracted payload on success, false and the
    /// failure kind on failure.
    std::function<void(bool success, const std::string& payload)> extract;

    /// The outcome of a conversion or transform: true, the path of the produced document, the
    /// result file name and a note on success, or false, a message, an empty file name and an empty
    /// note on failure. The file at the path is there until the function returns. The note holds
    /// the warnings of a transform that ran with a problem, and is empty otherwise.
    std::function<void(bool success, const std::string& pathOrError, const std::string& filename,
                       const std::string& note)>
        saveAs;

    /// The warnings of a transform that ran with a problem, as one sentence for the produced
    /// document's note, or empty when it ran without one.
    std::string transformNote;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
