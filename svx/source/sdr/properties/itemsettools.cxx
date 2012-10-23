/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/properties/itemsettools.hxx>
#include <tools/debug.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svditer.hxx>
#include <vcl/region.hxx>

//////////////////////////////////////////////////////////////////////////////
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

        sal_uInt32 ItemChangeBroadcaster::GetRectangleCount() const
        {
            return mnCount;
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

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        void ScaleItemSet(SfxItemSet& rSet, const Fraction& rScale)
        {
            sal_Int32 nMul(rScale.GetNumerator());
            sal_Int32 nDiv(rScale.GetDenominator());

            if(!rScale.IsValid() || !nDiv)
            {
                return;
            }

            SfxWhichIter aIter(rSet);
            sal_uInt16 nWhich(aIter.FirstWhich());
            const SfxPoolItem *pItem = NULL;

            while(nWhich)
            {
                if(SFX_ITEM_SET == rSet.GetItemState(nWhich, sal_False, &pItem))
                {
                    if(pItem->HasMetrics())
                    {
                        SfxPoolItem* pNewItem = pItem->Clone();
                        pNewItem->ScaleMetrics(nMul, nDiv);
                        rSet.Put(*pNewItem);
                    }
                }
                nWhich = aIter.NextWhich();
            }
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
