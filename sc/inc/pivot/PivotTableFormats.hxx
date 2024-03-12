/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <vector>
#include <rtl/ustring.hxx>

namespace sc
{
struct PivotTableFormat
{
    sal_Int32 nField;
    sal_Int32 nDataIndex;
    OUString aStyleName;

    PivotTableFormat(sal_Int32 _nField, sal_Int32 _nDataIndex, OUString _aStyleName)
        : nField(_nField)
        , nDataIndex(_nDataIndex)
        , aStyleName(_aStyleName)
    {
    }
};

class PivotTableFormats
{
    std::vector<PivotTableFormat> maFormats;

public:
    void add(sal_Int32 nField, sal_Int32 nDataIndex, OUString const& rStyle)
    {
        maFormats.emplace_back(nField, nDataIndex, rStyle);
    }

    size_t size() { return maFormats.size(); }

    std::vector<PivotTableFormat> const& getVector() { return maFormats; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
