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
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svditer.hxx>
#include <vcl/region.hxx>
#include <boost/scoped_ptr.hpp>

// class to remember broadcast start positions

namespace sdr
{
    namespace properties
    {
        ItemChangeBroadcaster::ItemChangeBroadcaster(const SdrObject& rObj)
        {
            if(rObj.ISA(SdrObjGroup))
            {
                SdrObjListIter aIter((const SdrObjGroup&)rObj, IM_DEEPNOGROUPS);
                mpData = new RectangleVector;
                DBG_ASSERT(mpData, "ItemChangeBroadcaster: No memory (!)");
                ((RectangleVector*)mpData)->reserve(aIter.Count());

                while(aIter.IsMore())
                {
                    SdrObject* pObj = aIter.Next();

                    if(pObj)
                    {
                        ((RectangleVector*)mpData)->push_back(pObj->GetLastBoundRect());
                    }
                }

                mnCount = ((RectangleVector*)mpData)->size();
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
                delete ((RectangleVector*)mpData);
            }
            else
            {
                delete ((Rectangle*)mpData);
            }
        }


        const Rectangle& ItemChangeBroadcaster::GetRectangle(sal_uInt32 nIndex) const
        {
            if(mnCount > 1)
            {
                return (*((RectangleVector*)mpData))[nIndex];
            }
            else
            {
                return *((Rectangle*)mpData);
            }
        }
    } // end of namespace properties
} // end of namespace sdr



namespace sdr
{
    namespace properties
    {
        void ScaleItemSet(SfxItemSet& rSet, const boost::rational<long>& rScale)
        {
            sal_Int32 nMul(rScale.numerator());
            sal_Int32 nDiv(rScale.denominator());

            SfxWhichIter aIter(rSet);
            sal_uInt16 nWhich(aIter.FirstWhich());
            const SfxPoolItem *pItem = NULL;

            while(nWhich)
            {
                if(SfxItemState::SET == rSet.GetItemState(nWhich, false, &pItem))
                {
                    if(pItem->HasMetrics())
                    {
                        boost::scoped_ptr<SfxPoolItem> pNewItem(pItem->Clone());
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
