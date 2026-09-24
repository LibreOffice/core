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

#include <sal/config.h>
#include <test/testdllapi.hxx>

#include <rtl/string.hxx>

namespace vcl::filter
{
class PDFDocument;
}

/// Test helpers for reading back an exported PDF.
class OOO_DLLPUBLIC_TEST PdfTestTools
{
public:
    /// Return what the object streams hold, inflated; a search of the raw file misses those bytes.
    static OString getObjectStreamsData(vcl::filter::PDFDocument& rDocument);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
