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



#ifndef _CONNECTIVITY_KAB_ORDER_HXX_
#define _CONNECTIVITY_KAB_ORDER_HXX_

#include "rtl/ustring.hxx"
#include <shell/kde_headers.h>

#include <vector>

namespace connectivity
{
    namespace kab
    {
        class KabOrder
        {
        public:
            virtual ~KabOrder();

            virtual sal_Int32 compare(const ::KABC::Addressee &aAddressee1, const ::KABC::Addressee &aAddressee2) const = 0;
        };

        class KabSimpleOrder : public KabOrder
        {
            sal_Int32 m_nFieldNumber;
            sal_Bool m_bAscending;

            QString value(const ::KABC::Addressee &aAddressee) const;
        public:
            KabSimpleOrder(::rtl::OUString &sColumnName, sal_Bool bAscending);

            virtual sal_Int32 compare(const ::KABC::Addressee &aAddressee1, const ::KABC::Addressee &aAddressee2) const;
        };

        class KabComplexOrder : public KabOrder
        {
            ::std::vector<KabOrder *> m_aOrders;

        public:
            KabComplexOrder();
            virtual ~KabComplexOrder();

            void addOrder(KabOrder *pOrder);
            virtual sal_Int32 compare(const ::KABC::Addressee &aAddressee1, const ::KABC::Addressee &aAddressee2) const;
        };
    }
}

#endif
