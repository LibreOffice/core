/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_EDITENG_MISSPELLRANGE_HXX
#define INCLUDED_EDITENG_MISSPELLRANGE_HXX

#include <editeng/editengdllapi.h>

#include <vector>

namespace editeng {

struct EDITENG_DLLPUBLIC MisspellRange
{
    size_t mnStart;
    size_t mnEnd;

    MisspellRange(size_t nStart, size_t nEnd);
};

struct EDITENG_DLLPUBLIC MisspellRanges
{
    sal_Int32 mnParagraph;
    std::vector<MisspellRange> maRanges;

    MisspellRanges(sal_Int32 nParagraph, const std::vector<MisspellRange>& rRanges);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
