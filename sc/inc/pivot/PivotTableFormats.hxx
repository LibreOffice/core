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
#include <patattr.hxx>
#include <memory>

namespace sc
{
struct PivotTableFormat
{
    sal_Int32 nField;
    sal_Int32 nDataIndex;
    std::shared_ptr<ScPatternAttr> pPattern;

    PivotTableFormat(sal_Int32 _nField, sal_Int32 _nDataIndex,
                     std::shared_ptr<ScPatternAttr> _pPattern)
        : nField(_nField)
        , nDataIndex(_nDataIndex)
        , pPattern(_pPattern)
    {
    }
};

class PivotTableFormats
{
    std::vector<PivotTableFormat> maFormats;

public:
    void add(sal_Int32 nField, sal_Int32 nDataIndex,
             std::shared_ptr<ScPatternAttr> const& rpPattern)
    {
        maFormats.emplace_back(nField, nDataIndex, rpPattern);
    }

    size_t size() { return maFormats.size(); }

    std::vector<PivotTableFormat> const& getVector() { return maFormats; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
