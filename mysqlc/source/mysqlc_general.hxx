/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
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
************************************************************************/

#ifndef _MYSQLC_GENERAL_
#define _MYSQLC_GENERAL_

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

#include <cppconn/exception.h>

namespace mysqlc_sdbc_driver
{
    rtl::OUString getStringFromAny(const ::com::sun::star::uno::Any& _rAny);

    void throwFeatureNotImplementedException(
            const sal_Char* _pAsciiFeatureName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
            const ::com::sun::star::uno::Any* _pNextException = NULL
        )
        throw (::com::sun::star::sdbc::SQLException);

    void throwInvalidArgumentException(
            const sal_Char* _pAsciiFeatureName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
            const ::com::sun::star::uno::Any* _pNextException = NULL
        )
        throw (::com::sun::star::sdbc::SQLException);

    void translateAndThrow(const ::sql::SQLException& _error, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _context, const rtl_TextEncoding encoding);

    int mysqlToOOOType(int mysqlType) throw ();


    ::rtl::OUString convert(const ::std::string& _string, const rtl_TextEncoding encoding);

    ::std::string convert(const ::rtl::OUString& _string, const rtl_TextEncoding encoding);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
