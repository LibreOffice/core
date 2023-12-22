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

#include "editattr.hxx"
#include "edtspell.hxx"
#include "eerdll2.hxx"
#include <editeng/svxfont.hxx>
#include <editeng/EPaM.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svl/itempool.hxx>
#include <svl/languageoptions.hxx>
#include <tools/lineend.hxx>
#include <o3tl/typed_flags_set.hxx>
#include "TextPortion.hxx"

#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>

class ItemList
{
private:
    typedef std::vector<const SfxPoolItem*> DummyItemList;
    DummyItemList aItemPool;
    sal_Int32 CurrentItem;

public:
    ItemList();
    const SfxPoolItem* First();
    const SfxPoolItem* Next();
    sal_Int32 Count() const { return aItemPool.size(); };
    void Insert(const SfxPoolItem* pItem);
    void Clear() { aItemPool.clear(); };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
