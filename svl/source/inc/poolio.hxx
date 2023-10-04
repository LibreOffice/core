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

#ifndef INCLUDED_SVL_SOURCE_INC_POOLIO_HXX
#define INCLUDED_SVL_SOURCE_INC_POOLIO_HXX

#include <rtl/ref.hxx>
#include <svl/itempool.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <tools/debug.hxx>
#include <memory>
#include <o3tl/sorted_vector.hxx>
#include <utility>

class SfxPoolItem;
class SfxItemPoolUser;

struct SfxItemPool_Impl
{
    SfxBroadcaster                  aBC;
    OUString                        aName;
    std::vector<SfxPoolItem*>       maPoolDefaults;
    std::vector<SfxPoolItem*>*      mpStaticDefaults;
    SfxItemPool*                    mpMaster;
    rtl::Reference<SfxItemPool>     mpSecondary;
    WhichRangesContainer            mpPoolRanges;
    sal_uInt16                      mnStart;
    sal_uInt16                      mnEnd;
    MapUnit                         eDefMetric;

    SfxItemPool_Impl( SfxItemPool* pMaster, OUString _aName, sal_uInt16 nStart, sal_uInt16 nEnd )
        : aName(std::move(_aName))
        , maPoolDefaults(nEnd - nStart + 1)
        , mpStaticDefaults(nullptr)
        , mpMaster(pMaster)
        , mnStart(nStart)
        , mnEnd(nEnd)
        , eDefMetric(MapUnit::MapCM)
    {
        DBG_ASSERT(mnStart, "Start-Which-Id must be greater 0" );
    }

    ~SfxItemPool_Impl()
    {
        DeleteItems();
    }

    void DeleteItems()
    {
        maPoolDefaults.clear();
        mpPoolRanges.reset();
    }
};

#endif // INCLUDED_SVL_SOURCE_INC_POOLIO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
