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

#ifndef CONNECTIVITY_DBTOOLS_DBTFACTORY_HXX
#define CONNECTIVITY_DBTOOLS_DBTFACTORY_HXX

#include <connectivity/virtualdbtools.hxx>
#include "refbase.hxx"

//........................................................................
namespace connectivity
{
//........................................................................

    //================================================================
    //= ODataAccessToolsFactory
    //================================================================
    class ODataAccessToolsFactory
            :public simple::IDataAccessToolsFactory
            ,public ORefBase
    {
    protected:
        ::rtl::Reference< simple::IDataAccessTypeConversion >   m_xTypeConversionHelper;
        ::rtl::Reference< simple::IDataAccessTools >            m_xToolsHelper;

    public:
        ODataAccessToolsFactory();

        // IDataAccessToolsFactory
        virtual ::rtl::Reference< simple::ISQLParser >  createSQLParser(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceFactory,
            const IParseContext* _pContext
        ) const;

        virtual ::rtl::Reference< simple::IDataAccessCharSet > createCharsetHelper( ) const;

        virtual ::rtl::Reference< simple::IDataAccessTypeConversion > getTypeConversionHelper();

        virtual ::rtl::Reference< simple::IDataAccessTools > getDataAccessTools();

        virtual ::std::auto_ptr< ::dbtools::FormattedColumnValue >  createFormattedColumnValue(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn
        );

        // IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();
    };

//........................................................................
}   // namespace connectivity
//........................................................................

#endif // CONNECTIVITY_DBTOOLS_DBTFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
