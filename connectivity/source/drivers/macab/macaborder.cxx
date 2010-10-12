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

#include "macaborder.hxx"
#include "MacabHeader.hxx"
#include "MacabRecord.hxx"

using namespace ::connectivity::macab;

MacabOrder::~MacabOrder()
{
}
// -----------------------------------------------------------------------------
MacabSimpleOrder::MacabSimpleOrder(MacabHeader *header, ::rtl::OUString &sColumnName, sal_Bool bAscending)
    : MacabOrder(),
      m_nFieldNumber(header->getColumnNumber(sColumnName)),
      m_bAscending(bAscending)
{
}
// -----------------------------------------------------------------------------
sal_Int32 MacabSimpleOrder::compare(const MacabRecord *record1, const MacabRecord *record2) const
{
    sal_Int32 result;

    result = MacabRecord::compareFields(record1->get(m_nFieldNumber), record2->get(m_nFieldNumber));

    if (!m_bAscending) result = -result;

    return result;
}
// -----------------------------------------------------------------------------
MacabComplexOrder::MacabComplexOrder()
    : MacabOrder(),
      m_aOrders()
{
}
// -----------------------------------------------------------------------------
MacabComplexOrder::~MacabComplexOrder()
{
    for (sal_uInt32 i = 0; i < m_aOrders.size(); i++)
        delete m_aOrders[i];
}
// -----------------------------------------------------------------------------
void MacabComplexOrder::addOrder(MacabOrder *pOrder)
{
    m_aOrders.push_back(pOrder);
}
// -----------------------------------------------------------------------------
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
