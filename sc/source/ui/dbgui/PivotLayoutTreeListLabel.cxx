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

#include <vcl/builderfactory.hxx>
#include <vcl/treelistentry.hxx>
#include <pivot.hxx>

VCL_BUILDER_FACTORY_ARGS(ScPivotLayoutTreeListLabel,
                         WB_BORDER | WB_TABSTOP | WB_CLIPCHILDREN);

ScPivotLayoutTreeListLabel::ScPivotLayoutTreeListLabel(vcl::Window* pParent, WinBits nBits)
    : ScPivotLayoutTreeListBase(pParent, nBits, LABEL_LIST)
    , maDataItem(0)
{
    SetForceMakeVisible(true);
}

ScPivotLayoutTreeListLabel::~ScPivotLayoutTreeListLabel()
{}

void ScPivotLayoutTreeListLabel::FillLabelFields(ScDPLabelDataVector& rLabelVector)
{
    Clear();
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
            SvTreeListEntry* pEntry = InsertEntry(pLabelData->maName);
            pEntry->SetUserData(pValue);
        }
    }
}

void ScPivotLayoutTreeListLabel::InsertEntryForSourceTarget(SvTreeListEntry* pSource, SvTreeListEntry* /*pTarget*/)
{
    ScPivotLayoutTreeListBase *pSourceTree = mpParent->FindListBoxFor(pSource);
    if (pSourceTree)
        pSourceTree->RemoveSelection();
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

void ScPivotLayoutTreeListLabel::KeyInput(const KeyEvent& rKeyEvent)
{
    vcl::KeyCode aCode = rKeyEvent.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    if (nCode == KEY_DELETE)
    {
        const SvTreeListEntry* pEntry = GetCurEntry();
        if (pEntry)
            GetModel()->Remove(pEntry);
        return;
    }

    SvTreeListBox::KeyInput(rKeyEvent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
