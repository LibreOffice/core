/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TKey.hxx,v $
 * $Revision: 1.3 $
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

#ifndef CONNECTIVITY_TKEY_HXX
#define CONNECTIVITY_TKEY_HXX

#include <connectivity/sdbcx/VKey.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>

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


