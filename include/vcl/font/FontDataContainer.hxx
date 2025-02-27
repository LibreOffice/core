/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>

namespace font
{
/** Font data container wraps binary content of a TTF font */
class FontDataContainer
{
    std::vector<sal_uInt8> const& mrFontBytes;

public:
    FontDataContainer(std::vector<sal_uInt8> const& rFontBytes)
        : mrFontBytes(rFontBytes)
    {
    }

    const char* getPointer() const { return reinterpret_cast<const char*>(mrFontBytes.data()); }

    size_t size() const { return mrFontBytes.size(); }

    std::vector<sal_uInt8>::const_iterator begin() const { return mrFontBytes.begin(); }
    std::vector<sal_uInt8>::const_iterator end() const { return mrFontBytes.end(); }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
