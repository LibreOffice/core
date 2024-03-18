/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <string_view>
#include <vector>

#include <svx/svditer.hxx>
#include <tools/long.hxx>
#include "types.hxx"
#include <optional>

class ScDocShell;
class SdrOle2Obj;

namespace sc::tools {

enum class ChartSourceType
{
    CELL_RANGE,
    PIVOT_TABLE
};

class ChartIterator
{
private:
    std::optional<SdrObjListIter> m_oIterator;
    ChartSourceType m_eChartSourceType;
public:
    ChartIterator(ScDocShell* pDocShell, SCTAB nTab, ChartSourceType eChartSourceType);
    SdrOle2Obj* next();
};

SdrOle2Obj* findChartsByName(ScDocShell* pDocShell, SCTAB nTab,
                             std::u16string_view rName,
                             ChartSourceType eChartSourceType);

SdrOle2Obj* getChartByIndex(ScDocShell* pDocShell, SCTAB nTab,
                            ::tools::Long nIndex, ChartSourceType eChartSourceType);

std::vector<SdrOle2Obj*> getAllPivotChartsConnectedTo(std::u16string_view sPivotTableName, ScDocShell* pDocShell);

} // end sc::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
