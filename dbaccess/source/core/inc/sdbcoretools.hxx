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

#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

namespace comphelper
{
    class ComponentContext;
}

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
    /** retrieves a particular indirect data source setting

        @param _rxDataSource
            a data source component
        @param _pAsciiSettingsName
            the ASCII name of the setting to obtain
        @param _rSettingsValue
            the value of the setting, upon successfull return

        @return
            <FALSE/> if the setting is not present in the <member scope="com::sun::star::sdb">DataSource::Info</member>
            member of the data source
            <TRUE/> otherwise
    */
    bool    getDataSourceSetting(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDataSource,
        const sal_Char* _pAsciiSettingsName,
        ::com::sun::star::uno::Any& /* [out] */ _rSettingsValue
    );

    // -----------------------------------------------------------------------------
    /** retrieves a to-be-displayed string for a given caught exception;
    */
    ::rtl::OUString extractExceptionMessage( const ::comphelper::ComponentContext& _rContext, const ::com::sun::star::uno::Any& _rError );

//.........................................................................
}   // namespace dbaccess
//.........................................................................

#endif // DBACORE_SDBCORETOOLS_HXX

