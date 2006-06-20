/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: korder.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:41:25 $
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

#include "korder.hxx"

#ifndef _CONNECTIVITY_KAB_FIELDS_HXX_
#include "kfields.hxx"
#endif

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
