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



#ifndef _CONNECTIVITY_MACAB_ORDER_HXX_
#define _CONNECTIVITY_MACAB_ORDER_HXX_

#include "rtl/ustring.hxx"
#include "MacabHeader.hxx"
#include "MacabRecord.hxx"

#include <vector>

namespace connectivity
{
    namespace macab
    {
        class MacabOrder
        {
        public:
            virtual ~MacabOrder();

            virtual sal_Int32 compare(const MacabRecord *record1, const MacabRecord *record2) const = 0;
        };

        class MacabSimpleOrder : public MacabOrder
        {
            sal_Int32 m_nFieldNumber;
            sal_Bool m_bAscending;

        public:
            MacabSimpleOrder(MacabHeader *header, ::rtl::OUString &sColumnName, sal_Bool bAscending);

            virtual sal_Int32 compare(const MacabRecord *record1, const MacabRecord *record2) const;
        };

        class MacabComplexOrder : public MacabOrder
        {
            ::std::vector<MacabOrder *> m_aOrders;

        public:
            MacabComplexOrder();
            virtual ~MacabComplexOrder();

            void addOrder(MacabOrder *pOrder);
            virtual sal_Int32 compare(const MacabRecord *record1, const MacabRecord *record2) const;
        };
    }
}

#endif
