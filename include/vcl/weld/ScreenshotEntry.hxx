/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <basegfx/range/b2irange.hxx>
#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>

#include <vector>

namespace weld
{
class VCL_DLLPUBLIC ScreenShotEntry
{
public:
    ScreenShotEntry(OUString aHelpId, const basegfx::B2IRange& rB2IRange)
        : msHelpId(std::move(aHelpId))
        , maB2IRange(rB2IRange)
    {
    }

    const basegfx::B2IRange& getB2IRange() const { return maB2IRange; }

    const OUString& GetHelpId() const { return msHelpId; }

private:
    OUString msHelpId;
    basegfx::B2IRange maB2IRange;
};

typedef std::vector<ScreenShotEntry> ScreenShotCollection;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
