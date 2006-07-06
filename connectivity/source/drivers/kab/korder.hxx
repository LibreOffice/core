/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: korder.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-06 14:24:04 $
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

#ifndef _CONNECTIVITY_KAB_ORDER_HXX_
#define _CONNECTIVITY_KAB_ORDER_HXX_

#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

#define KDE_HEADERS_WANT_KABC_ADDRESSEE
#include "kde_headers.hxx"

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
