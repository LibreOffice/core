/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macaborder.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:56:57 $
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

#ifndef _CONNECTIVITY_MACAB_ORDER_HXX_
#define _CONNECTIVITY_MACAB_ORDER_HXX_

#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

#ifndef _CONNECTIVITY_MACAB_HEADER_HXX_
#include "MacabHeader.hxx"
#endif

#ifndef _CONNECTIVITY_MACAB_RECORD_HXX_
#include "MacabRecord.hxx"
#endif

#include <vector>

namespace connectivity
{
    namespace macab
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
            sal_Bool m_bAscending;

        public:
            MacabSimpleOrder(MacabHeader *header, ::rtl::OUString &sColumnName, sal_Bool bAscending);

            virtual sal_Int32 compare(const MacabRecord *record1, const MacabRecord *record2) const;
        };

        class MacabComplexOrder : public MacabOrder
        {
            ::std::vector<MacabOrder *> m_aOrders;

        public:
            MacabComplexOrder();
            virtual ~MacabComplexOrder();

            void addOrder(MacabOrder *pOrder);
            virtual sal_Int32 compare(const MacabRecord *record1, const MacabRecord *record2) const;
        };
    }
}

#endif
