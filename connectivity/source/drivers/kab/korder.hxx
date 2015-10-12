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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_KAB_KORDER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_KAB_KORDER_HXX

#include <rtl/ustring.hxx>
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
            bool m_bAscending;

            QString value(const ::KABC::Addressee &aAddressee) const;
        public:
            KabSimpleOrder(OUString &sColumnName, bool bAscending);

            virtual sal_Int32 compare(const ::KABC::Addressee &aAddressee1, const ::KABC::Addressee &aAddressee2) const override;
        };

        class KabComplexOrder : public KabOrder
        {
            ::std::vector<KabOrder *> m_aOrders;

        public:
            KabComplexOrder();
            virtual ~KabComplexOrder();

            void addOrder(KabOrder *pOrder);
            virtual sal_Int32 compare(const ::KABC::Addressee &aAddressee1, const ::KABC::Addressee &aAddressee2) const override;
        };
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
