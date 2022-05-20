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

#include <undobase.hxx>
#include <memory>

namespace sc
{
/** Previous sparkline group data, which is restored at Undo grouping */
struct UndoGroupSparklinesData
{
    UndoGroupSparklinesData(ScAddress const& rAddress, ScRangeList const& rDataRangeList,
                            std::shared_ptr<sc::SparklineGroup> const& rpGroup)
        : m_aAddress(rAddress)
        , m_aDataRangeList(rDataRangeList)
        , m_pSparklineGroup(rpGroup)
    {
    }

    ScAddress m_aAddress;
    ScRangeList m_aDataRangeList;
    std::shared_ptr<sc::SparklineGroup> m_pSparklineGroup;
};

/** Undo action for grouping sparklines */
class UndoGroupSparklines : public ScSimpleUndo
{
private:
    ScRange m_aRange;
    std::shared_ptr<sc::SparklineGroup> m_pSparklineGroup;
    std::vector<UndoGroupSparklinesData> m_aUndoData;

public:
    UndoGroupSparklines(ScDocShell& rDocShell, ScRange const& rRange,
                        std::shared_ptr<sc::SparklineGroup> const& rpSparklineGroup);
    virtual ~UndoGroupSparklines() override;

    void Undo() override;
    void Redo() override;
    bool CanRepeat(SfxRepeatTarget& rTarget) const override;
    void Repeat(SfxRepeatTarget& rTarget) override;
    OUString GetComment() const override;
};

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
