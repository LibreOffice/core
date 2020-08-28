/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_INC_CHARTTOOLS_HXX
#define INCLUDED_SC_INC_CHARTTOOLS_HXX

#include <memory>
#include <vector>

#include <svx/svditer.hxx>
#include <rtl/ustring.hxx>

#include "types.hxx"

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
    std::unique_ptr<SdrObjListIter> m_pIterator;
    ChartSourceType m_eChartSourceType;
public:
    ChartIterator(ScDocShell* pDocShell, SCTAB nTab, ChartSourceType eChartSourceType);
    SdrOle2Obj* next();
};

SdrOle2Obj* findChartsByName(ScDocShell* pDocShell, SCTAB nTab,
                             OUString const & rName,
                             ChartSourceType eChartSourceType);

SdrOle2Obj* getChartByIndex(ScDocShell* pDocShell, SCTAB nTab,
                            long nIndex, ChartSourceType eChartSourceType);

std::vector<SdrOle2Obj*> getAllPivotChartsConnectedTo(OUString const & sPivotTableName, ScDocShell* pDocShell);

} // end sc::tools

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
