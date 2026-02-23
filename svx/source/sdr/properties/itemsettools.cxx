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

#include <sdr/properties/itemsettools.hxx>
#include <tools/fract.hxx>
#include <svl/itemset.hxx>
#include <svl/itemiter.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svditer.hxx>
#include <memory>

// class to remember broadcast start positions

namespace sdr::properties
{
        ItemChangeBroadcaster::ItemChangeBroadcaster(const SdrObject& rObj)
        {
            if (rObj.GetObjIdentifier() == SdrObjKind::Group)
            {
                const SdrObjGroup* pGroupObj = static_cast<const SdrObjGroup*>(&rObj);
                SdrObjListIter aIter(pGroupObj->GetSubList(), SdrIterMode::DeepNoGroups);
                maRectangles.reserve(aIter.Count());

                while(aIter.IsMore())
                {
                    SdrObject* pObj = aIter.Next();

                    if(pObj)
                    {
                        maRectangles.push_back(pObj->GetLastBoundRect());
                    }
                }
            }
            else
            {
                maRectangles.push_back(rObj.GetLastBoundRect());
            }
        }


        void ScaleItemSet(SfxItemSet& rSet, const Fraction& rScale)
        {
            sal_Int32 nMul(rScale.GetNumerator());
            sal_Int32 nDiv(rScale.GetDenominator());

            if(!rScale.IsValid() || !nDiv)
            {
                return;
            }

            for (SfxItemIter aIter(rSet); !aIter.IsAtEnd(); aIter.Next())
            {
                const SfxPoolItem *pItem = aIter.GetCurItem();
                if(!IsDisabledItem(pItem) && pItem->HasMetrics())
                {
                    std::unique_ptr<SfxPoolItem> pNewItem(pItem->Clone());
                    pNewItem->ScaleMetrics(nMul, nDiv);
                    rSet.Put(std::move(pNewItem));
                }
            }
        }
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
