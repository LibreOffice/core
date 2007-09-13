/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macaborder.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:56:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "macaborder.hxx"

#ifndef _CONNECTIVITY_MACAB_HEADER_HXX_
#include "MacabHeader.hxx"
#endif

#ifndef _CONNECTIVITY_MACAB_RECORD_HXX_
#include "MacabRecord.hxx"
#endif

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
