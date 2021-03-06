/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>

#include <address.hxx>

#include <memory>
#include <vector>
#include <map>

class ScRangeName;
class ScRangeData;

struct ScRangeNameLine
{
    OUString aName;
    OUString aExpression;
    OUString aScope;
};

class SC_DLLPUBLIC ScRangeManagerTable
{
private:
    std::unique_ptr<weld::TreeView> m_xTreeView;

    OUString maGlobalString;

    // should be const because we should not modify it here
    const std::map<OUString, std::unique_ptr<ScRangeName>>& m_RangeMap;
    // for performance, save which entries already have the formula entry
    // otherwise opening the dialog with a lot of range names is extremely slow because
    // we would calculate all formula strings during opening
    std::map<OUString, bool> maCalculatedFormulaEntries;
    const ScAddress maPos;

    int m_nId;

    bool mbNeedUpdate;

    void GetLine(ScRangeNameLine& aLine, const weld::TreeIter& rEntry);
    void Init();
    const ScRangeData* findRangeData(const ScRangeNameLine& rLine);

    DECL_LINK(SizeAllocHdl, const Size&, void);
    DECL_LINK(VisRowsScrolledHdl, weld::TreeView&, void);

public:
    ScRangeManagerTable(std::unique_ptr<weld::TreeView>,
        const std::map<OUString, std::unique_ptr<ScRangeName>>& rTabRangeNames,
        const ScAddress& rPos);

    void CheckForFormulaString();

    int n_children() const { return m_xTreeView->n_children(); }
    void connect_changed(const Link<weld::TreeView&, void>& rLink) { m_xTreeView->connect_changed(rLink); }
    void set_cursor(int nPos) { m_xTreeView->set_cursor(nPos); }

    void addEntry(const ScRangeNameLine& rLine, bool bSetCurEntry);
    void DeleteSelectedEntries();
    void SetEntry( const ScRangeNameLine& rLine );

    void GetCurrentLine(ScRangeNameLine& rLine);
    bool IsMultiSelection() const;
    std::vector<ScRangeNameLine> GetSelectedEntries();

    void BlockUpdate()
    {
        mbNeedUpdate = false;
    }

    bool UpdatesBlocked() const
    {
        return !mbNeedUpdate;
    }

    void UnblockUpdate()
    {
        mbNeedUpdate = true;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
