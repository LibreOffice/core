/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//ScRangeManagerTable
#include <memory>
#include <global.hxx>
#include <globstr.hrc>
#include <o3tl/safeint.hxx>
#include <scresid.hxx>
#include <globalnames.hxx>
#include <namemgrtable.hxx>
#include <rangenam.hxx>

#include <unotools/charclass.hxx>
#include <vcl/weld.hxx>
#include <tools/link.hxx>

void ScRangeManagerTable::GetCurrentLine(ScRangeNameLine& rLine)
{
    std::unique_ptr<weld::TreeIter> xCurrentEntry(m_xTreeView->make_iterator());
    if (m_xTreeView->get_cursor(xCurrentEntry.get()))
        GetLine(rLine, *xCurrentEntry);
}

void ScRangeManagerTable::DeleteSelectedEntries()
{
    std::vector<int> aRows = m_xTreeView->get_selected_rows();
    std::sort(aRows.begin(), aRows.end());
    for (auto it = aRows.rbegin(); it != aRows.rend(); ++it)
        m_xTreeView->remove(*it);
}

bool ScRangeManagerTable::IsMultiSelection() const
{
    return m_xTreeView->count_selected_rows() > 1;
}

void ScRangeManagerTable::SetEntry(const ScRangeNameLine& rLine)
{
    for (int i = 0, nEntryCount = m_xTreeView->n_children(); i < nEntryCount; ++i)
    {
        if (rLine.aName == m_xTreeView->get_text(i, 0)
            && rLine.aScope == m_xTreeView->get_text(i, 2))
        {
            m_xTreeView->set_cursor(i);
        }
    }
}

ScRangeManagerTable::ScRangeManagerTable(
    std::unique_ptr<weld::TreeView> xTreeView,
    const std::map<OUString, std::unique_ptr<ScRangeName>>& rRangeMap, const ScAddress& rPos)
    : m_xTreeView(std::move(xTreeView))
    , maGlobalString(ScResId(STR_GLOBAL_SCOPE))
    , m_RangeMap(rRangeMap)
    , maPos(rPos)
    , m_nId(0)
    , mbNeedUpdate(true)
{
    auto nColWidth = m_xTreeView->get_size_request().Width() / 7;
    std::vector<int> aWidths{ o3tl::narrowing<int>(nColWidth * 2),
                              o3tl::narrowing<int>(nColWidth * 3) };
    m_xTreeView->set_column_fixed_widths(aWidths);

    Init();
    m_xTreeView->set_selection_mode(SelectionMode::Multiple);
    m_xTreeView->connect_size_allocate(LINK(this, ScRangeManagerTable, SizeAllocHdl));
    m_xTreeView->connect_visible_range_changed(LINK(this, ScRangeManagerTable, VisRowsScrolledHdl));
}

IMPL_LINK_NOARG(ScRangeManagerTable, VisRowsScrolledHdl, weld::TreeView&, void)
{
    CheckForFormulaString();
}

const ScRangeData* ScRangeManagerTable::findRangeData(const ScRangeNameLine& rLine)
{
    const ScRangeName* pRangeName;
    if (rLine.aScope == maGlobalString)
        pRangeName = m_RangeMap.find(OUString(STR_GLOBAL_RANGE_NAME))->second.get();
    else
        pRangeName = m_RangeMap.find(rLine.aScope)->second.get();

    return pRangeName->findByUpperName(ScGlobal::getCharClass().uppercase(rLine.aName));
}

void ScRangeManagerTable::CheckForFormulaString()
{
    if (UpdatesBlocked())
        return;

    auto lambda = [this](weld::TreeIter& rEntry) {
        OUString sId(m_xTreeView->get_id(rEntry));
        std::map<OUString, bool>::const_iterator itr = maCalculatedFormulaEntries.find(sId);
        if (itr == maCalculatedFormulaEntries.end() || !itr->second)
        {
            ScRangeNameLine aLine;
            GetLine(aLine, rEntry);
            const ScRangeData* pData = findRangeData(aLine);
            OUString aFormulaString;
            pData->GetSymbol(aFormulaString, maPos);
            m_xTreeView->set_text(rEntry, aFormulaString, 1);
            maCalculatedFormulaEntries.insert(std::pair<OUString, bool>(sId, true));
        }
        return false;
    };

    // ensure all visible entries are up to date
    m_xTreeView->visible_foreach(lambda);
    // and ensure all selected entries are up to date
    m_xTreeView->selected_foreach(lambda);
}

IMPL_LINK_NOARG(ScRangeManagerTable, SizeAllocHdl, const Size&, void) { CheckForFormulaString(); }

void ScRangeManagerTable::addEntry(const ScRangeNameLine& rLine, bool bSetCurEntry)
{
    int nRow = m_xTreeView->n_children();
    m_xTreeView->append();
    m_xTreeView->set_text(nRow, rLine.aName, 0);
    m_xTreeView->set_text(nRow, rLine.aExpression, 1);
    m_xTreeView->set_text(nRow, rLine.aScope, 2);
    // just unique to track which one has been cached by maCalculatedFormulaEntries
    m_xTreeView->set_id(nRow, OUString::number(m_nId++));
    if (bSetCurEntry)
        m_xTreeView->set_cursor(nRow);
}

void ScRangeManagerTable::GetLine(ScRangeNameLine& rLine, const weld::TreeIter& rEntry)
{
    rLine.aName = m_xTreeView->get_text(rEntry, 0);
    rLine.aExpression = m_xTreeView->get_text(rEntry, 1);
    rLine.aScope = m_xTreeView->get_text(rEntry, 2);
}

void ScRangeManagerTable::Init()
{
    m_xTreeView->freeze();
    m_xTreeView->clear();
    for (auto const& itr : m_RangeMap)
    {
        const ScRangeName* const pLocalRangeName = itr.second.get();
        ScRangeNameLine aLine;
        if (itr.first == STR_GLOBAL_RANGE_NAME)
            aLine.aScope = maGlobalString;
        else
            aLine.aScope = itr.first;
        for (const auto& rEntry : *pLocalRangeName)
        {
            if (!rEntry.second->HasType(ScRangeData::Type::Database))
            {
                aLine.aName = rEntry.second->GetName();
                addEntry(aLine, false);
            }
        }
    }
    m_xTreeView->thaw();
}

std::vector<ScRangeNameLine> ScRangeManagerTable::GetSelectedEntries()
{
    std::vector<ScRangeNameLine> aSelectedEntries;
    m_xTreeView->selected_foreach([this, &aSelectedEntries](weld::TreeIter& rEntry) {
        ScRangeNameLine aLine;
        GetLine(aLine, rEntry);
        aSelectedEntries.push_back(aLine);
        return false;
    });
    return aSelectedEntries;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
