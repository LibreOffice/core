/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
