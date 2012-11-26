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
#include <svx/sdr/properties/pageproperties.hxx>
#include <svl/itemset.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet& PageProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // overloaded to legally return a valid ItemSet
            return *(new SfxItemSet(rPool));
        }

        PageProperties::PageProperties(SdrObject& rObj)
        :   EmptyProperties(rObj)
        {
        }

        PageProperties::PageProperties(const PageProperties& rProps, SdrObject& rObj)
        :   EmptyProperties(rProps, rObj)
        {
        }

        PageProperties::~PageProperties()
        {
        }

        BaseProperties& PageProperties::Clone(SdrObject& rObj) const
        {
            return *(new PageProperties(*this, rObj));
        }

        // get itemset. Overloaded here to allow creating the empty itemset
        // without asserting
        const SfxItemSet& PageProperties::GetObjectItemSet() const
        {
            if(!mpEmptyItemSet)
            {
                ((PageProperties*)this)->mpEmptyItemSet = &(((PageProperties*)this)->CreateObjectSpecificItemSet(
                    GetSdrObject().GetObjectItemPool()));
            }

            DBG_ASSERT(mpEmptyItemSet, "Could not create an SfxItemSet(!)");

            return *mpEmptyItemSet;
        }

        void PageProperties::ItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/)
        {
            // #86481# simply ignore item setting on page objects
        }

        SfxStyleSheet* PageProperties::GetStyleSheet() const
        {
            // overloaded to legally return a 0L pointer here
            return 0L;
        }

        void PageProperties::PostItemChange(const sal_uInt16 nWhich )
        {
            if( (nWhich == XATTR_FILLSTYLE) && (mpEmptyItemSet != NULL) )
                CleanupFillProperties(*mpEmptyItemSet);
        }

        void PageProperties::ClearObjectItem(const sal_uInt16 /*nWhich*/)
        {
            // simply ignore item clearing on page objects
        }
    } // end of namespace properties
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

// eof
