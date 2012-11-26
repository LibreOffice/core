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
            const SdrObjGroup* pSdrObjGroup = dynamic_cast< const SdrObjGroup* >(&rObj);

            if(pSdrObjGroup)
            {
                SdrObjListIter aIter(*pSdrObjGroup->getChildrenOfSdrObject(), IM_DEEPNOGROUPS);
                maBroadcastVector.reserve(aIter.Count());

                while(aIter.IsMore())
                {
                    SdrObject* pObj = aIter.Next();

                    if(pObj)
                    {
                        maBroadcastVector.push_back(new SdrObjectChangeBroadcaster(*pObj, HINT_OBJCHG_ATTR));
                    }
                }
            }
            else
            {
                maBroadcastVector.push_back(new SdrObjectChangeBroadcaster(rObj, HINT_OBJCHG_ATTR));
            }
        }

        ItemChangeBroadcaster::~ItemChangeBroadcaster()
        {
            for(sal_uInt32 a(0); a < maBroadcastVector.size(); a++)
            {
                delete maBroadcastVector[a];
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
