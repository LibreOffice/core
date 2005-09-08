/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdbcoretools.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:47:28 $
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

//.........................................................................
}   // namespace dbaccess
//.........................................................................

#endif // DBACORE_SDBCORETOOLS_HXX

