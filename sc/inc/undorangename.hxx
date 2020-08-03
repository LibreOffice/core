/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <undobase.hxx>

#include <memory>
#include <map>

class ScDocShell;

/**
 * Undo object for named ranges, both in global and sheet-local scopes.
 */
class ScUndoAllRangeNames final : public ScSimpleUndo
{
public:
    ScUndoAllRangeNames(ScDocShell* pDocSh, const std::map<OUString, ScRangeName*>& rOldNames,
                        const std::map<OUString, std::unique_ptr<ScRangeName>>& rNewNames);

    virtual ~ScUndoAllRangeNames() override;

    virtual void Undo() override;
    virtual void Redo() override;
    virtual void Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool CanRepeat(SfxRepeatTarget& rTarget) const override;
    virtual OUString GetComment() const override;

private:
    void DoChange(const std::map<OUString, std::unique_ptr<ScRangeName>>& rNames);

private:
    std::map<OUString, std::unique_ptr<ScRangeName>> m_OldNames;
    std::map<OUString, std::unique_ptr<ScRangeName>> m_NewNames;
};

class ScUndoAddRangeData final : public ScSimpleUndo
{
public:
    // nTab = -1 for global range names
    ScUndoAddRangeData(ScDocShell* pDocSh, const ScRangeData* pRangeData, SCTAB nTab);

    virtual ~ScUndoAddRangeData() override;

    virtual void Undo() override;
    virtual void Redo() override;
    virtual void Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool CanRepeat(SfxRepeatTarget& rTarget) const override;
    virtual OUString GetComment() const override;

private:
    std::unique_ptr<ScRangeData> mpRangeData;
    SCTAB mnTab;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
