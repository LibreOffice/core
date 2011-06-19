/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef UCB_UCPEXT_DATA_SUPPLIER_HXX
#define UCB_UCPEXT_DATA_SUPPLIER_HXX

#include <rtl/ref.hxx>
#include <ucbhelper/resultset.hxx>

#include <boost/scoped_ptr.hpp>

//......................................................................................................................
namespace ucb { namespace ucp { namespace ext
{
//......................................................................................................................

    struct DataSupplier_Impl;
    class Content;

    //==================================================================================================================
    //= DataSupplier
    //==================================================================================================================
    typedef ::ucbhelper::ResultSetDataSupplier  DataSupplier_Base;
    class DataSupplier : public ::ucbhelper::ResultSetDataSupplier
    {
    public:
        DataSupplier(
            const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& i_rORB,
            const rtl::Reference< Content >& rContent,
            const sal_Int32 nOpenMode
        );

        void    fetchData();

    protected:
        virtual ~DataSupplier();

        virtual rtl::OUString queryContentIdentifierString( sal_uInt32 nIndex );
        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > queryContentIdentifier( sal_uInt32 nIndex );
        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent > queryContent( sal_uInt32 nIndex );

        virtual sal_Bool getResult( sal_uInt32 nIndex );

        virtual sal_uInt32 totalCount();
        virtual sal_uInt32 currentCount();
        virtual sal_Bool isCountFinal();

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > queryPropertyValues( sal_uInt32 nIndex  );
        virtual void releasePropertyValues( sal_uInt32 nIndex );

        virtual void close();

        virtual void validate() throw( com::sun::star::ucb::ResultSetException );

    private:
        ::boost::scoped_ptr< DataSupplier_Impl >    m_pImpl;
    };

//......................................................................................................................
} } }   // namespace ucp::ext
//......................................................................................................................

#endif  // UCB_UCPEXT_DATA_SUPPLIER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
