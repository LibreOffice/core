/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "editdataarray.hxx"

ScEditDataArray::ScEditDataArray()
{
}

ScEditDataArray::~ScEditDataArray()
{
}

void ScEditDataArray::AddItem(SCTAB nTab, SCCOL nCol, SCROW nRow,
                              EditTextObject* pOldData, EditTextObject* pNewData)
{
    maArray.push_back(Item(nTab, nCol, nRow, pOldData, pNewData));
}

const ScEditDataArray::Item* ScEditDataArray::First()
{
    maIter = maArray.begin();
    if (maIter == maArray.end())
        return NULL;
    return &(*maIter++);
}

const ScEditDataArray::Item* ScEditDataArray::Next()
{
    if (maIter == maArray.end())
        return NULL;
    return &(*maIter++);
}

ScEditDataArray::Item::Item(SCTAB nTab, SCCOL nCol, SCROW nRow,
                            EditTextObject* pOldData, EditTextObject* pNewData) :
    mnTab(nTab),
    mnCol(nCol),
    mnRow(nRow)
{
    mpOldData.reset(pOldData);
    mpNewData.reset(pNewData);
}

ScEditDataArray::Item::~Item()
{
}

const EditTextObject* ScEditDataArray::Item::GetOldData() const
{
    return mpOldData.get();
}

const EditTextObject* ScEditDataArray::Item::GetNewData() const
{
    return mpNewData.get();
}

SCTAB ScEditDataArray::Item::GetTab() const
{
    return mnTab;
}

SCCOL ScEditDataArray::Item::GetCol() const
{
    return mnCol;
}

SCROW ScEditDataArray::Item::GetRow() const
{
    return mnRow;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
