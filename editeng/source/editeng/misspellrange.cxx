/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "editeng/misspellrange.hxx"

namespace editeng {

MisspellRange::MisspellRange(size_t nStart, size_t nEnd) : mnStart(nStart), mnEnd(nEnd) {}

MisspellRanges::MisspellRanges(sal_Int32 nParagraph, const std::vector<MisspellRange>& rRanges) :
    mnParagraph(nParagraph), maRanges(rRanges) {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
