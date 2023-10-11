/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <broadcast.hxx>
#include <address.hxx>
#include <formulacell.hxx>

namespace sc
{
BroadcasterState::CellListener::CellListener(const ScFormulaCell* p)
    : pData(p)
{
}

BroadcasterState::CellListener::CellListener(const SvtListener* p)
    : pData(p)
{
}

BroadcasterState::AreaListener::AreaListener(const ScFormulaCell* p)
    : pData(p)
{
}

BroadcasterState::AreaListener::AreaListener(const sc::FormulaGroupAreaListener* p)
    : pData(p)
{
}

BroadcasterState::AreaListener::AreaListener(const SvtListener* p)
    : pData(p)
{
}

bool BroadcasterState::hasFormulaCellListener(const ScAddress& rBroadcasterPos,
                                              const ScAddress& rFormulaPos) const
{
    auto it = aCellListenerStore.find(rBroadcasterPos);
    if (it == aCellListenerStore.end())
        return false;

    for (const auto& rLis : it->second)
    {
        if (rLis.pData.index() == 0)
        {
            auto pFC = std::get<const ScFormulaCell*>(rLis.pData);
            if (pFC->aPos == rFormulaPos)
                return true;
        }
    }

    return false;
}

bool BroadcasterState::hasFormulaCellListener(const ScRange& rBroadcasterRange,
                                              const ScAddress& rFormulaPos) const
{
    auto it = aAreaListenerStore.find(rBroadcasterRange);
    if (it == aAreaListenerStore.end())
        return false;

    for (const auto& rLis : it->second)
    {
        if (rLis.pData.index() == 0)
        {
            auto pFC = std::get<const ScFormulaCell*>(rLis.pData);
            if (pFC->aPos == rFormulaPos)
                return true;
        }
    }

    return false;
}

void BroadcasterState::dump(std::ostream& rStrm, const ScDocument* pDoc) const
{
    constexpr ScRefFlags nPosFlags = ScRefFlags::VALID | ScRefFlags::TAB_3D;

    rStrm << "---" << std::endl;

    for (const auto & [ rPos, rListeners ] : aCellListenerStore)
    {
        rStrm << "- type: cell-broadcaster\n";
        rStrm << "  position: " << rPos.Format(nPosFlags, pDoc) << std::endl;

        if (!rListeners.empty())
            rStrm << "  listeners:\n";

        for (const auto& rLis : rListeners)
        {
            switch (rLis.pData.index())
            {
                case 0:
                {
                    auto* pFC = std::get<const ScFormulaCell*>(rLis.pData);
                    rStrm << "  - type: formula-cell\n";
                    rStrm << "    position: " << pFC->aPos.Format(nPosFlags, pDoc) << std::endl;
                    break;
                }
                case 1:
                {
                    rStrm << "  - type: unknown" << std::endl;
                    break;
                }
            }
        }
    }

    for (const auto & [ rRange, rListeners ] : aAreaListenerStore)
    {
        rStrm << "- type: area-broadcaster\n";
        rStrm << "  range: " << rRange.Format(*pDoc, nPosFlags) << std::endl;

        if (!rListeners.empty())
            rStrm << "  listeners:\n";

        for (const auto& rLis : rListeners)
        {
            switch (rLis.pData.index())
            {
                case 0:
                {
                    auto* pFC = std::get<const ScFormulaCell*>(rLis.pData);
                    rStrm << "  - type: formula-cell\n";
                    rStrm << "    position: " << pFC->aPos.Format(nPosFlags, pDoc) << std::endl;
                    break;
                }
                case 1:
                {
                    auto* pFGL = std::get<const sc::FormulaGroupAreaListener*>(rLis.pData);

                    auto pTopCell = pFGL->getTopCell();
                    if (auto xFG = pTopCell->GetCellGroup(); xFG)
                    {
                        ScRange aGR(pTopCell->aPos);
                        aGR.aEnd.IncRow(xFG->mnLength - 1);
                        rStrm << "  - type: formula-group\n";
                        rStrm << "    range: " << aGR.Format(*pDoc, nPosFlags) << std::endl;
                    }
                    break;
                }
                case 2:
                {
                    rStrm << "  - type: unknown" << std::endl;
                    break;
                }
            }
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
