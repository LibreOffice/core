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
#include <svx/sdr/properties/defaultproperties.hxx>
#include <svx/sdr/properties/itemsettools.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>

#include <vector>
#include <svx/svdobj.hxx>
#include <svx/svddef.hxx>
#include <svx/svdpool.hxx>
#include <editeng/eeitem.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        SfxItemSet& DefaultProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // Basic implementation; Basic object has NO attributes
            return *(new SfxItemSet(rPool));
        }

        DefaultProperties::DefaultProperties(SdrObject& rObj)
        :   BaseProperties(rObj),
            mpItemSet(0)
        {
        }

        DefaultProperties::DefaultProperties(const DefaultProperties& rProps, SdrObject& rObj)
        :   BaseProperties(rObj),
            mpItemSet(0)
        {
            if(rProps.mpItemSet)
            {
                // Clone may be to another model and thus another ItemPool
                mpItemSet = rProps.mpItemSet->Clone(true, &rObj.getSdrModelFromSdrObject().GetItemPool());

                // do not keep parent info, this may be changed by later constructors.
                // This class just copies the ItemSet, ignore parent.
                if(mpItemSet && mpItemSet->GetParent())
                {
                    mpItemSet->SetParent(0);
                }
            }
        }

        BaseProperties& DefaultProperties::Clone(SdrObject& rObj) const
        {
            return *(new DefaultProperties(*this, rObj));
        }

        DefaultProperties::~DefaultProperties()
        {
            if(mpItemSet)
            {
                delete mpItemSet;
                mpItemSet = 0;
            }
        }

        const SfxItemSet& DefaultProperties::GetObjectItemSet() const
        {
            if(!mpItemSet)
            {
                ((DefaultProperties*)this)->mpItemSet = &(((DefaultProperties*)this)->CreateObjectSpecificItemSet(
                    GetSdrObject().GetObjectItemPool()));
                ((DefaultProperties*)this)->ForceDefaultAttributes();
            }

            DBG_ASSERT(mpItemSet, "Could not create an SfxItemSet(!)");

            return *mpItemSet;
        }

        void DefaultProperties::SetObjectItem(const SfxPoolItem& rItem)
        {
            const sal_uInt16 nWhichID(rItem.Which());

            if(AllowItemChange(nWhichID, &rItem))
            {
                ItemChange(nWhichID, &rItem);
                PostItemChange(nWhichID);

                SfxItemSet aSet(GetSdrObject().GetObjectItemPool(), nWhichID, nWhichID);
                aSet.Put(rItem);
                ItemSetChanged(aSet);
            }
        }

        void DefaultProperties::SetObjectItemDirect(const SfxPoolItem& rItem)
        {
            const sal_uInt16 nWhichID(rItem.Which());

            if(AllowItemChange(nWhichID, &rItem))
            {
                ItemChange(nWhichID, &rItem);
            }
        }

        void DefaultProperties::ClearObjectItem(const sal_uInt16 nWhich)
        {
            if(AllowItemChange(nWhich))
            {
                ItemChange(nWhich);
                PostItemChange(nWhich);

                if(nWhich)
                {
                    SfxItemSet aSet(GetSdrObject().GetObjectItemPool(), nWhich, nWhich, 0, 0);
                    ItemSetChanged(aSet);
                }
            }
        }

        void DefaultProperties::ClearObjectItemDirect(const sal_uInt16 nWhich)
        {
            if(AllowItemChange(nWhich))
            {
                ItemChange(nWhich);
            }
        }

        void DefaultProperties::SetObjectItemSet(const SfxItemSet& rSet)
        {
            SfxWhichIter aWhichIter(rSet);
            sal_uInt16 nWhich(aWhichIter.FirstWhich());
            const SfxPoolItem *pPoolItem;
            std::vector< sal_uInt16 > aPostItemChangeList;
            sal_Bool bDidChange(sal_False);
            SfxItemSet aSet(GetSdrObject().GetObjectItemPool(), SDRATTR_START, EE_ITEMS_END, 0, 0);

            // give a hint to STL_Vector
            aPostItemChangeList.reserve(rSet.Count());

            while(nWhich)
            {
                if(SFX_ITEM_SET == rSet.GetItemState(nWhich, sal_False, &pPoolItem))
                {
                    if(AllowItemChange(nWhich, pPoolItem))
                    {
                        bDidChange = sal_True;
                        ItemChange(nWhich, pPoolItem);
                        aPostItemChangeList.push_back( nWhich );
                        aSet.Put(*pPoolItem);
                    }
                }

                nWhich = aWhichIter.NextWhich();
            }

            if(bDidChange)
            {
                std::vector< sal_uInt16 >::iterator aIter = aPostItemChangeList.begin();
                const std::vector< sal_uInt16 >::iterator aEnd = aPostItemChangeList.end();

                while(aIter != aEnd)
                {
                    PostItemChange(*aIter);
                    aIter++;
                }

                ItemSetChanged(aSet);
            }
        }

        void DefaultProperties::ItemSetChanged(const SfxItemSet& /*rSet*/)
        {
        }

        bool DefaultProperties::AllowItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/) const
        {
            return sal_True;
        }

        void DefaultProperties::ItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/)
        {
        }

        void DefaultProperties::PostItemChange(const sal_uInt16 nWhich )
        {
            if( (nWhich == XATTR_FILLSTYLE) && (mpItemSet != NULL) )
                CleanupFillProperties(*mpItemSet);
        }

        void DefaultProperties::SetStyleSheet(SfxStyleSheet* /*pNewStyleSheet*/, bool /*bDontRemoveHardAttr*/)
        {
            // no StyleSheet in DefaultProperties
        }

        SfxStyleSheet* DefaultProperties::GetStyleSheet() const
        {
            // no StyleSheet in DefaultProperties
            return 0L;
        }

        void DefaultProperties::ForceDefaultAttributes()
        {
        }

        void DefaultProperties::Scale(const Fraction& rScale)
        {
            if(mpItemSet)
            {
                ScaleItemSet(*mpItemSet, rScale);
            }
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
