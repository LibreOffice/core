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

#pragma once

#include <editeng/editobj.hxx>
#include "types.hxx"

#include <memory>
#include <vector>

class EditTextObject;

class ScEditDataArray
{
public:
    class Item
    {
    public:
        Item() = delete;
        explicit Item(SCTAB nTab, SCCOL nCol, SCROW nRow,
                      std::unique_ptr<EditTextObject> pOldData, std::unique_ptr<EditTextObject> pNewData);
        Item(Item const &) = delete; // due to mpOldData
        Item(Item&&) = default;
        ~Item();

        const EditTextObject* GetOldData() const;
        const EditTextObject* GetNewData() const;
        SCTAB GetTab() const { return mnTab;}
        SCCOL GetCol() const { return mnCol;}
        SCROW GetRow() const { return mnRow;}

    private:
        std::unique_ptr<EditTextObject> mpOldData;
        std::unique_ptr<EditTextObject> mpNewData;
        SCTAB mnTab;
        SCCOL mnCol;
        SCROW mnRow;

    };

    ScEditDataArray();
    ~ScEditDataArray();

    void AddItem(SCTAB nTab, SCCOL nCol, SCROW nRow,
                 std::unique_ptr<EditTextObject> pOldData, std::unique_ptr<EditTextObject> pNewData);

    const Item* First();
    const Item* Next();

private:
    ::std::vector<Item>::const_iterator maIter;
    ::std::vector<Item> maArray;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
