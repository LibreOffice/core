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

#include <rtl/ustring.hxx>
#include <svl/itemset.hxx>
#include <memory>
#include <vector>
#include <unordered_map>

class StylePoolImpl;

class SVL_DLLPUBLIC StylePool final
{
private:
    std::unique_ptr<StylePoolImpl> pImpl;
public:
    explicit StylePool( SfxItemSet const * pIgnorableItems = nullptr );
    ~StylePool();

    /** Insert a SfxItemSet into the style pool.

        The pool makes a copy of the provided SfxItemSet.

        @param SfxItemSet
        the SfxItemSet to insert

        @param pParentName
        Name of the parent of rSet. If set, createIterator() can be more deterministic by iterating
        over item sets ordered by parent names.

        @return a shared pointer to the SfxItemSet
    */
    std::shared_ptr<SfxItemSet> insertItemSet( const SfxItemSet& rSet, const OUString* pParentName = nullptr );

    void populateCacheMap(std::unordered_map< OUString, std::shared_ptr<SfxItemSet> >& rCacheMap);

    // skips unused and ignorable items
    void getAllStyles( std::vector<std::shared_ptr<SfxItemSet>> &rStyles );

    static OUString nameOf( const std::shared_ptr<SfxItemSet>& pSet );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
