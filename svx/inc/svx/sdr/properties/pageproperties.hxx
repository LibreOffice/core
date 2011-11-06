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



#ifndef _SDR_PROPERTIES_PAGEPROPERTIES_HXX
#define _SDR_PROPERTIES_PAGEPROPERTIES_HXX

#include <svx/sdr/properties/emptyproperties.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        class PageProperties : public EmptyProperties
        {
        protected:
            // create a new object specific itemset with object specific ranges.
            virtual SfxItemSet& CreateObjectSpecificItemSet(SfxItemPool& pPool);

            // Do the ItemChange, may do special handling
            virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0);

            // Called after ItemChange() is done for all items.
            virtual void PostItemChange(const sal_uInt16 nWhich);

        public:
            // basic constructor
            PageProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            PageProperties(const PageProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~PageProperties();

            // Clone() operator, normally just calls the local copy constructor
            virtual BaseProperties& Clone(SdrObject& rObj) const;

            // get itemset. Overloaded here to allow creating the empty itemset
            // without asserting
            virtual const SfxItemSet& GetObjectItemSet() const;

            // get the installed StyleSheet
            virtual SfxStyleSheet* GetStyleSheet() const;

            // clear single item
            virtual void ClearObjectItem(const sal_uInt16 nWhich = 0);
        };
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_PROPERTIES_PAGEPROPERTIES_HXX

// eof


