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

#pragma once

#include <rtl/ustring.hxx>
#include "MacabHeader.hxx"
#include "MacabRecord.hxx"

#include <memory>
#include <string_view>
#include <vector>

namespace connectivity::macab
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
            bool m_bAscending;

        public:
            MacabSimpleOrder(MacabHeader const *header, std::u16string_view sColumnName, bool bAscending);

            virtual sal_Int32 compare(const MacabRecord *record1, const MacabRecord *record2) const override;
        };

        class MacabComplexOrder : public MacabOrder
        {
            std::vector<std::unique_ptr<MacabOrder>> m_aOrders;

        public:
            MacabComplexOrder();
            virtual ~MacabComplexOrder() override;

            void addOrder(MacabOrder *pOrder);
            virtual sal_Int32 compare(const MacabRecord *record1, const MacabRecord *record2) const override;
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
