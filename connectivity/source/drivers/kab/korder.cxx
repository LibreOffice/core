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


#include "korder.hxx"
#include "kfields.hxx"

using namespace ::connectivity::kab;

KabOrder::~KabOrder()
{
}
// -----------------------------------------------------------------------------
KabSimpleOrder::KabSimpleOrder(OUString &sColumnName, sal_Bool bAscending)
    : KabOrder(),
      m_nFieldNumber(findKabField(sColumnName)),
      m_bAscending(bAscending)
{
}
// -----------------------------------------------------------------------------
sal_Int32 KabSimpleOrder::compare(const ::KABC::Addressee &aAddressee1, const ::KABC::Addressee &aAddressee2) const
{
    sal_Int32 result;

    result = QString::compare(
        valueOfKabField(aAddressee1, m_nFieldNumber),
        valueOfKabField(aAddressee2, m_nFieldNumber));
// Timestamps should be compared differently than with their string value

    if (!m_bAscending) result = -result;

    return result;
}
// -----------------------------------------------------------------------------
KabComplexOrder::KabComplexOrder()
    : KabOrder(),
      m_aOrders()
{
}
// -----------------------------------------------------------------------------
KabComplexOrder::~KabComplexOrder()
{
    for (sal_uInt32 i = 0; i < m_aOrders.size(); i++)
        delete m_aOrders[i];
}
// -----------------------------------------------------------------------------
void KabComplexOrder::addOrder(KabOrder *pOrder)
{
    m_aOrders.push_back(pOrder);
}
// -----------------------------------------------------------------------------
sal_Int32 KabComplexOrder::compare(const ::KABC::Addressee &aAddressee1, const ::KABC::Addressee &aAddressee2) const
{
    for (sal_uInt32 i = 0; i < m_aOrders.size(); i++)
    {
        const KabOrder *pOrder = m_aOrders[i];
        sal_Int32 result = pOrder->compare(aAddressee1, aAddressee2);

        if (result) return result;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
