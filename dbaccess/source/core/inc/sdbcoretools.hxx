/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdbcoretools.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif

//.........................................................................
namespace dbaccess
{
//.........................................................................

    // -----------------------------------------------------------------------------
    void notifyDataSourceModified(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxObject,sal_Bool _bModified);

    // -----------------------------------------------------------------------------
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        getDataSource( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDependentObject );

    // -----------------------------------------------------------------------------
//.........................................................................
}   // namespace dbaccess
//.........................................................................

#endif // DBACORE_SDBCORETOOLS_HXX

