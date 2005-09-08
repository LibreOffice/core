/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BIndex.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:43:39 $
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

#ifndef _CONNECTIVITY_ADABAS_INDEX_HXX_
#define _CONNECTIVITY_ADABAS_INDEX_HXX_

#ifndef _CONNECTIVITY_SDBCX_INDEX_HXX_
#include "connectivity/sdbcx/VIndex.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif

namespace connectivity
{
    namespace adabas
    {
        class OAdabasTable;
        class OAdabasIndex : public sdbcx::OIndex
        {
            OAdabasTable*   m_pTable;
        public:
            virtual void refreshColumns();
        public:
            OAdabasIndex(OAdabasTable* _pTable);
            OAdabasIndex(   OAdabasTable* _pTable,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Catalog,
                    sal_Bool _isUnique,
                    sal_Bool _isPrimaryKeyIndex,
                    sal_Bool _isClustered
                );
            OAdabasTable* getTable() const { return m_pTable; }
        };
    }
}
#endif // _CONNECTIVITY_ADABAS_INDEX_HXX_

