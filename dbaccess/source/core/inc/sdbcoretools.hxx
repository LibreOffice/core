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

#ifndef DBACORE_SDBCORETOOLS_HXX
#define DBACORE_SDBCORETOOLS_HXX

/** === begin UNO includes === **/
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
/** === end UNO includes === **/

namespace comphelper
{
    class ComponentContext;
}

namespace dbaccess
{

    void notifyDataSourceModified(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxObject,sal_Bool _bModified);

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        getDataSource( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDependentObject );

    /** retrieves a to-be-displayed string for a given caught exception;
    */
    ::rtl::OUString extractExceptionMessage( const ::comphelper::ComponentContext& _rContext, const ::com::sun::star::uno::Any& _rError );

    namespace tools
    {
        namespace stor
        {
            bool    storageIsWritable_nothrow(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxStorage
                    );

            /// commits a given storage if it's not readonly
            bool    commitStorageIfWriteable(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxStorage
                    )
                    SAL_THROW((
                        ::com::sun::star::io::IOException,
                        ::com::sun::star::lang::WrappedTargetException
                ));
        }

    }

}   // namespace dbaccess

#endif // DBACORE_SDBCORETOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
