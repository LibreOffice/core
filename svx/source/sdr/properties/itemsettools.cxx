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
#include <tools/debug.hxx>
#include <tools/fract.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svditer.hxx>
#include <vcl/region.hxx>
#include <vcl/outdev.hxx>
#include <memory>

// class to remember broadcast start positions

namespace sdr
{
    namespace properties
    {
        ItemChangeBroadcaster::ItemChangeBroadcaster(const SdrObject& rObj)
        {
            if(dynamic_cast<const SdrObjGroup*>( &rObj ) !=  nullptr)
            {
                SdrObjListIter aIter(static_cast<const SdrObjGroup&>(rObj), SdrIterMode::DeepNoGroups);
                mpData = new RectangleVector;
                DBG_ASSERT(mpData, "ItemChangeBroadcaster: No memory (!)");
                static_cast<RectangleVector*>(mpData)->reserve(aIter.Count());

                while(aIter.IsMore())
                {
                    SdrObject* pObj = aIter.Next();

                    if(pObj)
                    {
                        static_cast<RectangleVector*>(mpData)->push_back(pObj->GetLastBoundRect());
                    }
                }

                mnCount = static_cast<RectangleVector*>(mpData)->size();
            }
            else
            {
                mpData = new Rectangle(rObj.GetLastBoundRect());
                mnCount = 1L;
            }
        }

        ItemChangeBroadcaster::~ItemChangeBroadcaster()
        {
            if(mnCount > 1)
            {
                delete static_cast<RectangleVector*>(mpData);
            }
            else
            {
                delete static_cast<Rectangle*>(mpData);
            }
        }


        const Rectangle& ItemChangeBroadcaster::GetRectangle(sal_uInt32 nIndex) const
        {
            if(mnCount > 1)
            {
                return (*static_cast<RectangleVector*>(mpData))[nIndex];
            }
            else
            {
                return *static_cast<Rectangle*>(mpData);
            }
        }
    } // end of namespace properties
} // end of namespace sdr


namespace sdr
{
    namespace properties
    {
        void ScaleItemSet(SfxItemSet& rSet, const Fraction& rScale)
        {
            sal_Int32 nMul(rScale.GetNumerator());
            sal_Int32 nDiv(rScale.GetDenominator());

            if ( !rScale.IsOkay() || nDiv == 0 )
                return;

            SfxWhichIter aIter(rSet);
            sal_uInt16 nWhich(aIter.FirstWhich());
            const SfxPoolItem *pItem = nullptr;

            while(nWhich)
            {
                if(SfxItemState::SET == rSet.GetItemState(nWhich, false, &pItem))
                {
                    if(pItem->HasMetrics())
                    {
                        std::unique_ptr<SfxPoolItem> pNewItem(pItem->Clone());
                        pNewItem->ScaleMetrics(nMul, nDiv);
                        rSet.Put(*pNewItem);
                    }
                }
                nWhich = aIter.NextWhich();
            }
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
