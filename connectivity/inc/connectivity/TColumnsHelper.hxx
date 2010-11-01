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

#ifndef CONNECTIVITY_COLUMNSHELPER_HXX
#define CONNECTIVITY_COLUMNSHELPER_HXX

#include "connectivity/sdbcx/VCollection.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "connectivity/sdbcx/IRefreshable.hxx"
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    class OTableHelper;
    class OColumnsHelperImpl;
    /** contains generell column handling to creat default columns and default sql statements.
    */
    class OOO_DLLPUBLIC_DBTOOLS OColumnsHelper : public sdbcx::OCollection
    {
        OColumnsHelperImpl* m_pImpl;
    protected:
        OTableHelper*   m_pTable;

        virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
        virtual sdbcx::ObjectType appendObject( const ::rtl::OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
        virtual void dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName);
    public:
        OColumnsHelper( ::cppu::OWeakObject& _rParent
                        ,sal_Bool _bCase
                        ,::osl::Mutex& _rMutex
                        ,const TStringVector &_rVector
                        ,sal_Bool _bUseHardRef = sal_True
                    );
        virtual ~OColumnsHelper();

        /** set the parent of the columns. Can also be <NULL/>.
            @param  _pTable
                The parent.
        */
        inline void setParent(OTableHelper* _pTable) { m_pTable = _pTable;}
    };
}
#endif // CONNECTIVITY_COLUMNSHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
