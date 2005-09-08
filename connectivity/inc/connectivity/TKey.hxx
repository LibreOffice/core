/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TKey.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:59:26 $
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

#ifndef CONNECTIVITY_TKEY_HXX
#define CONNECTIVITY_TKEY_HXX

#ifndef _CONNECTIVITY_SDBCX_KEY_HXX_
#include <connectivity/sdbcx/VKey.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif

namespace connectivity
{
    class OTableHelper;
    class OTableKeyHelper : public connectivity::sdbcx::OKey
    {
        OTableHelper*   m_pTable;
    public:
        virtual void refreshColumns();
    public:
        OTableKeyHelper(    OTableHelper* _pTable);
        OTableKeyHelper(    OTableHelper* _pTable,
                const ::rtl::OUString& _Name,
                const ::rtl::OUString& _ReferencedTable,
                sal_Int32       _Type,
                sal_Int32       _UpdateRule,
                sal_Int32       _DeleteRule
            );
        inline OTableHelper* getTable() const { return m_pTable; }
    };
}
#endif // CONNECTIVITY_TKEY_HXX


