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
#include <svx/sdr/properties/emptyproperties.hxx>
#include <tools/debug.hxx>
#include <svl/itemset.hxx>
#include <svx/svddef.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet& EmptyProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // Basic implementation; Basic object has NO attributes
            DBG_ASSERT(sal_False, "EmptyProperties::CreateObjectSpecificItemSet() should never be called");
            return *(new SfxItemSet(rPool));
        }

        EmptyProperties::EmptyProperties(SdrObject& rObj)
        :   BaseProperties(rObj),
            mpEmptyItemSet(0L)
        {
        }

        EmptyProperties::EmptyProperties(const EmptyProperties& rProps, SdrObject& rObj)
        :   BaseProperties(rProps, rObj),
            mpEmptyItemSet(0L)
        {
            // #115593#
            // do not gererate an assert, else derivations like PageProperties will generate an assert
            // using the Clone() operator path.
        }

        EmptyProperties::~EmptyProperties()
        {
            if(mpEmptyItemSet)
            {
                delete mpEmptyItemSet;
                mpEmptyItemSet = 0L;
            }
        }

        BaseProperties& EmptyProperties::Clone(SdrObject& rObj) const
        {
            return *(new EmptyProperties(*this, rObj));
        }

        const SfxItemSet& EmptyProperties::GetObjectItemSet() const
        {
            if(!mpEmptyItemSet)
            {
                ((EmptyProperties*)this)->mpEmptyItemSet = &(((EmptyProperties*)this)->CreateObjectSpecificItemSet(
                    GetSdrObject().GetObjectItemPool()));
            }

            DBG_ASSERT(mpEmptyItemSet, "Could not create an SfxItemSet(!)");
            DBG_ASSERT(sal_False, "EmptyProperties::GetObjectItemSet() should never be called (!)");

            return *mpEmptyItemSet;
        }

        void EmptyProperties::SetObjectItem(const SfxPoolItem& /*rItem*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::SetObjectItem() should never be called (!)");
        }

        void EmptyProperties::SetObjectItemDirect(const SfxPoolItem& /*rItem*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::SetObjectItemDirect() should never be called (!)");
        }

        void EmptyProperties::ClearObjectItem(const sal_uInt16 /*nWhich*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::ClearObjectItem() should never be called (!)");
        }

        void EmptyProperties::ClearObjectItemDirect(const sal_uInt16 /*nWhich*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::ClearObjectItemDirect() should never be called (!)");
        }

        void EmptyProperties::SetObjectItemSet(const SfxItemSet& /*rSet*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::SetObjectItemSet() should never be called (!)");
        }

        void EmptyProperties::ItemSetChanged(const SfxItemSet& /*rSet*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::ItemSetChanged() should never be called (!)");
        }

        bool EmptyProperties::AllowItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/) const
        {
            DBG_ASSERT(sal_False, "EmptyProperties::AllowItemChange() should never be called (!)");
            return sal_True;
        }

        void EmptyProperties::ItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::ItemChange() should never be called (!)");
        }

        void EmptyProperties::PostItemChange(const sal_uInt16 /*nWhich*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::PostItemChange() should never be called (!)");
        }

        void EmptyProperties::SetStyleSheet(SfxStyleSheet* /*pNewStyleSheet*/, bool /*bDontRemoveHardAttr*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::SetStyleSheet() should never be called (!)");
        }

        SfxStyleSheet* EmptyProperties::GetStyleSheet() const
        {
            DBG_ASSERT(sal_False, "EmptyProperties::GetStyleSheet() should never be called (!)");
            return 0L;
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
