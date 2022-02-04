/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 */

#include <memory>
#include <PivotLayoutTreeListLabel.hxx>
#include <PivotLayoutDialog.hxx>

#include <vcl/event.hxx>
#include <pivot.hxx>

ScPivotLayoutTreeListLabel::ScPivotLayoutTreeListLabel(std::unique_ptr<weld::TreeView> xControl)
    : ScPivotLayoutTreeListBase(std::move(xControl), LABEL_LIST)
    , maDataItem(0)
{
    mxControl->connect_key_press(LINK(this, ScPivotLayoutTreeListLabel, KeyInputHdl));
}

ScPivotLayoutTreeListLabel::~ScPivotLayoutTreeListLabel()
{}

void ScPivotLayoutTreeListLabel::FillLabelFields(ScDPLabelDataVector& rLabelVector)
{
    mxControl->clear();
    maItemValues.clear();

    for (std::unique_ptr<ScDPLabelData> const & pLabelData : rLabelVector)
    {
        ScItemValue* pValue = new ScItemValue(pLabelData->maName, pLabelData->mnCol, pLabelData->mnFuncMask);
        maItemValues.push_back(std::unique_ptr<ScItemValue>(pValue));
        if (pLabelData->mbDataLayout)
        {
            maDataItem = maItemValues.size() - 1;
        }

        if (pLabelData->mnOriginalDim < 0 && !pLabelData->mbDataLayout)
        {
            mxControl->append(weld::toId(pValue), pLabelData->maName);
        }
    }
}

void ScPivotLayoutTreeListLabel::InsertEntryForSourceTarget(weld::TreeView& rSource, int /*nTarget*/)
{
    if (&rSource == mxControl.get())
        return;
    rSource.remove(rSource.get_selected_index());
}

bool ScPivotLayoutTreeListLabel::IsDataElement(SCCOL nColumn)
{
    return (nColumn == PIVOT_DATA_FIELD || nColumn == maDataItem);
}

ScItemValue* ScPivotLayoutTreeListLabel::GetItem(SCCOL nColumn)
{
    if (nColumn == PIVOT_DATA_FIELD)
        return maItemValues[maDataItem].get();
    return maItemValues[nColumn].get();
}

IMPL_LINK(ScPivotLayoutTreeListLabel, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    vcl::KeyCode aCode = rKeyEvent.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    if (nCode == KEY_DELETE)
    {
        int nEntry = mxControl->get_cursor_index();
        if (nEntry != -1)
            mxControl->remove(nEntry);
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
