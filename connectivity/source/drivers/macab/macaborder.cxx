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
