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


#include "macaborder.hxx"
#include "MacabHeader.hxx"
#include "MacabRecord.hxx"

using namespace ::connectivity::macab;

MacabOrder::~MacabOrder()
{
}

MacabSimpleOrder::MacabSimpleOrder(MacabHeader *header, OUString &sColumnName, sal_Bool bAscending)
    : MacabOrder(),
      m_nFieldNumber(header->getColumnNumber(sColumnName)),
      m_bAscending(bAscending)
{
}

sal_Int32 MacabSimpleOrder::compare(const MacabRecord *record1, const MacabRecord *record2) const
{
    sal_Int32 result;

    result = MacabRecord::compareFields(record1->get(m_nFieldNumber), record2->get(m_nFieldNumber));

    if (!m_bAscending) result = -result;

    return result;
}

MacabComplexOrder::MacabComplexOrder()
    : MacabOrder(),
      m_aOrders()
{
}

MacabComplexOrder::~MacabComplexOrder()
{
    for (sal_uInt32 i = 0; i < m_aOrders.size(); i++)
        delete m_aOrders[i];
}

void MacabComplexOrder::addOrder(MacabOrder *pOrder)
{
    m_aOrders.push_back(pOrder);
}

sal_Int32 MacabComplexOrder::compare(const MacabRecord *record1, const MacabRecord *record2) const
{
    for (sal_uInt32 i = 0; i < m_aOrders.size(); i++)
    {
        const MacabOrder *pOrder = m_aOrders[i];
        sal_Int32 result = pOrder->compare(record1, record2);

        if (result) return result;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
