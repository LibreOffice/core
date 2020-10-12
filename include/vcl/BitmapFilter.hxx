/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_BITMAPFILTER_HXX
#define INCLUDED_VCL_BITMAPFILTER_HXX

#include <vcl/bitmapex.hxx>
#include <functional>

class Animation;

namespace vcl::bitmap
{
// Generates strip ranges and run the input function with the start and
// end as parameters. The additional parameter bLast denotes if the
// iteration is the last one.
//
// Example:
// first = 0, last = 100, STRIP_SIZE = 32
// this will generate:
// [0, 31, false], [32, 63, false], [64, 95, false], [96, 100, true]
template <int STRIP_SIZE>
void generateStripRanges(
    tools::Long nFirst, tools::Long nLast,
    std::function<void(tools::Long const nStart, tools::Long const nEnd, bool const bLast)>
        aFunction)
{
    tools::Long nStart = nFirst;
    for (; nStart < nLast - STRIP_SIZE; nStart += STRIP_SIZE)
    {
        tools::Long nEnd = nStart + STRIP_SIZE - 1;
        aFunction(nStart, nEnd, false);
    }
    aFunction(nStart, nLast, true);
}

} // end vcl::bitmap

class VCL_DLLPUBLIC BitmapFilter
{
public:
    BitmapFilter();
    virtual ~BitmapFilter();

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) const = 0;

    static bool Filter(BitmapEx& rBmpEx, BitmapFilter const& rFilter);
    static bool Filter(Animation& rBmpEx, BitmapFilter const& rFilter);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
