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

#include <vcl/dllapi.h>

#include <vector>

namespace vcl::font
{
/** Read the glyph ID to CID mapping from the charset of a CID-keyed CFF font.

    For name-keyed fonts rCIDs is left empty.

    Returns false if the CFF data could not be parsed. */
VCL_DLLPUBLIC bool ReadCFFGlyphCIDs(const sal_uInt8* pData, sal_uInt32 nLen,
                                    std::vector<sal_uInt16>& rCIDs);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
