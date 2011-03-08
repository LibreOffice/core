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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "korder.hxx"
#include "kfields.hxx"

using namespace ::connectivity::kab;

KabOrder::~KabOrder()
{
}
// -----------------------------------------------------------------------------
KabSimpleOrder::KabSimpleOrder(::rtl::OUString &sColumnName, sal_Bool bAscending)
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
