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

#include "PivotLayoutTreeListLabel.hxx"
#include "PivotLayoutDialog.hxx"

#include <svtools/treelistentry.hxx>
#include "pivot.hxx"
#include "scabstdlg.hxx"

using namespace std;

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeScPivotLayoutTreeListLabel(Window *pParent, VclBuilder::stringmap& )
{
    return new ScPivotLayoutTreeListLabel(pParent, WB_BORDER | WB_TABSTOP | WB_CLIPCHILDREN | WB_FORCE_MAKEVISIBLE);
}

ScPivotLayoutTreeListLabel::ScPivotLayoutTreeListLabel(Window* pParent, WinBits nBits) :
    ScPivotLayoutTreeListBase(pParent, nBits, LABEL_LIST)
{}

ScPivotLayoutTreeListLabel::~ScPivotLayoutTreeListLabel()
{}

void ScPivotLayoutTreeListLabel::FillLabelFields(ScDPLabelDataVector& rLabelVector)
{
    Clear();
    maItemValues.clear();

    ScDPLabelDataVector::iterator it;
    for (it = rLabelVector.begin(); it != rLabelVector.end(); ++it)
    {
        const ScDPLabelData& rLabelData = *it;

        ScItemValue* pValue = new ScItemValue(rLabelData.maName, rLabelData.mnCol, rLabelData.mnFuncMask);
        maItemValues.push_back(pValue);
        if (rLabelData.mbDataLayout)
        {
            maDataItem = maItemValues.size() - 1;
        }

        if (rLabelData.mnOriginalDim < 0 && !rLabelData.mbDataLayout)
        {
            SvTreeListEntry* pEntry = InsertEntry(rLabelData.maName);
            pEntry->SetUserData(pValue);
        }
    }
}

void ScPivotLayoutTreeListLabel::InsertEntryForSourceTarget(SvTreeListEntry* /*pSource*/, SvTreeListEntry* /*pTarget*/)
{
    if(mpParent->mpPreviouslyFocusedListBox != this)
        mpParent->mpPreviouslyFocusedListBox->RemoveSelection();
}

bool ScPivotLayoutTreeListLabel::IsDataElement(SCCOL nColumn)
{
    return (nColumn == PIVOT_DATA_FIELD || nColumn == maDataItem);
}

ScItemValue* ScPivotLayoutTreeListLabel::GetItem(SCCOL nColumn)
{
    if (nColumn == PIVOT_DATA_FIELD)
        return &maItemValues[maDataItem];
    return &maItemValues[nColumn];
}

void ScPivotLayoutTreeListLabel::KeyInput(const KeyEvent& rKeyEvent)
{
    KeyCode aCode = rKeyEvent.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    switch (nCode)
    {
        case KEY_DELETE:
            GetModel()->Remove(GetCurEntry());
            return;
    }
    SvTreeListBox::KeyInput(rKeyEvent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
