/*************************************************************************
 *
 *  $RCSfile: staticdbtools_s.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:17:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONNECTIVITY_VIRTUAL_DBTOOLS_HXX
#include <connectivity/virtualdbtools.hxx>
#endif
#ifndef CONNECTIVITY_STATIC_DBTOOLS_SIMPLE_HXX
#include "staticdbtools_s.hxx"
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif

//........................................................................
namespace connectivity
{
//........................................................................

    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;

    //================================================================
    //= ODataAccessStaticTools
    //================================================================
    //----------------------------------------------------------------
    ODataAccessStaticTools::ODataAccessStaticTools()
    {
    }

    //----------------------------------------------------------------
    Date ODataAccessStaticTools::getStandardDate() const
    {
        return ::dbtools::DBTypeConversion::getStandardDate();
    }

    //----------------------------------------------------------------
    double ODataAccessStaticTools::getValue(const Reference< XColumn>& _rxVariant, const Date& rNullDate, sal_Int16 nKeyType) const
    {
        return ::dbtools::DBTypeConversion::getValue(_rxVariant, rNullDate, nKeyType);
    }

    //----------------------------------------------------------------
    ::rtl::OUString ODataAccessStaticTools::getValue(const Reference< XColumn >& _rxColumn, const Reference< XNumberFormatter >& _rxFormatter,
        const Date& _rNullDate, sal_Int32 _nKey, sal_Int16 _nKeyType) const
    {
        return ::dbtools::DBTypeConversion::getValue(_rxColumn, _rxFormatter, _rNullDate, _nKey, _nKeyType);
    }

    //----------------------------------------------------------------
    ::rtl::OUString ODataAccessStaticTools::getValue( const Reference< XPropertySet>& _rxColumn, const Reference< XNumberFormatter>& _rxFormatter,
        const Locale& _rLocale, const Date& _rNullDate ) const
    {
        return ::dbtools::DBTypeConversion::getValue( _rxColumn, _rxFormatter, _rLocale, _rNullDate );
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL ODataAccessStaticTools::acquire()
    {
        return ORefBase::acquire();
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL ODataAccessStaticTools::release()
    {
        return ORefBase::release();
    }

    //----------------------------------------------------------------
    Reference< XConnection> ODataAccessStaticTools::getConnection_withFeedback(const ::rtl::OUString& _rDataSourceName, const ::rtl::OUString& _rUser,
        const ::rtl::OUString& _rPwd, const Reference< XMultiServiceFactory>& _rxFactory) const SAL_THROW ( (SQLException) )
    {
        return ::dbtools::getConnection_withFeedback(_rDataSourceName, _rUser, _rPwd, _rxFactory);
    }

    //----------------------------------------------------------------
    Reference< XConnection> ODataAccessStaticTools::connectRowset(const Reference< XRowSet>& _rxRowSet, const Reference< XMultiServiceFactory>& _rxFactory, sal_Bool _bSetAsActiveConnection) const
        SAL_THROW ( (SQLException, RuntimeException) )
    {
        return ::dbtools::connectRowset( _rxRowSet, _rxFactory, _bSetAsActiveConnection);
    }

    //----------------------------------------------------------------
    Reference< XConnection> ODataAccessStaticTools::calcConnection(const Reference< XRowSet>& _rxRowSet, const Reference< XMultiServiceFactory>& _rxFactory) const
        SAL_THROW ( (SQLException, RuntimeException) )
    {
        return ::dbtools::calcConnection(_rxRowSet, _rxFactory);
    }

    // ------------------------------------------------
    Reference< XConnection> ODataAccessStaticTools::getRowSetConnection(
                const Reference< XRowSet>& _rxRowSet)
                const SAL_THROW ( (RuntimeException) )
    {
        return ::dbtools::getConnection(_rxRowSet);
    }

    //----------------------------------------------------------------
    Reference< XNumberFormatsSupplier> ODataAccessStaticTools::getNumberFormats(const Reference< XConnection>& _rxConn, sal_Bool _bAllowDefault) const
    {
        return ::dbtools::getNumberFormats(_rxConn, _bAllowDefault);
    }

    //----------------------------------------------------------------
    sal_Int32 ODataAccessStaticTools::getDefaultNumberFormat( const Reference< XPropertySet >& _rxColumn, const Reference< XNumberFormatTypes >& _rxTypes,
        const Locale& _rLocale ) const
    {
        return ::dbtools::getDefaultNumberFormat( _rxColumn, _rxTypes, _rLocale );
    }

    //----------------------------------------------------------------
    void ODataAccessStaticTools::TransferFormComponentProperties(const Reference< XPropertySet>& _rxOld, const Reference< XPropertySet>& _rxNew, const Locale& _rLocale) const
    {
        ::dbtools::TransferFormComponentProperties(_rxOld, _rxNew, _rLocale);
    }

    //----------------------------------------------------------------
    ::rtl::OUString ODataAccessStaticTools::quoteName(const ::rtl::OUString& _rQuote, const ::rtl::OUString& _rName) const
    {
        return ::dbtools::quoteName(_rQuote, _rName);
    }

    //----------------------------------------------------------------
    ::rtl::OUString ODataAccessStaticTools::quoteTableName(const Reference< XDatabaseMetaData>& _rxMeta, const ::rtl::OUString& _rName
                                , sal_Bool _bUseCatalogInSelect
                                , sal_Bool _bUseSchemaInSelect) const
    {
        return ::dbtools::quoteTableName(_rxMeta, _rName,::dbtools::eInDataManipulation,_bUseCatalogInSelect,_bUseSchemaInSelect);
    }

    //----------------------------------------------------------------
    SQLContext ODataAccessStaticTools::prependContextInfo(SQLException& _rException, const Reference< XInterface >& _rxContext,
        const ::rtl::OUString& _rContextDescription, const ::rtl::OUString& _rContextDetails) const
    {
        return ::dbtools::prependContextInfo(_rException, _rxContext, _rContextDescription, _rContextDetails);
    }

    //----------------------------------------------------------------
    Reference< XDataSource > ODataAccessStaticTools::getDataSource( const ::rtl::OUString& _rsRegisteredName, const Reference< XMultiServiceFactory>& _rxFactory ) const
    {
        return ::dbtools::getDataSource( _rsRegisteredName, _rxFactory );
    }

    //----------------------------------------------------------------
    sal_Bool ODataAccessStaticTools::canInsert(const Reference< XPropertySet>& _rxCursorSet) const
    {
        return ::dbtools::canInsert( _rxCursorSet );
    }

    //----------------------------------------------------------------
    sal_Bool ODataAccessStaticTools::canUpdate(const Reference< XPropertySet>& _rxCursorSet) const
    {
        return ::dbtools::canUpdate( _rxCursorSet );
    }

    //----------------------------------------------------------------
    sal_Bool ODataAccessStaticTools::canDelete(const Reference< XPropertySet>& _rxCursorSet) const
    {
        return ::dbtools::canDelete( _rxCursorSet );
    }

    //----------------------------------------------------------------
    Reference< XNameAccess > ODataAccessStaticTools::getFieldsByCommandDescriptor( const Reference< XConnection >& _rxConnection,
        const sal_Int32 _nCommandType, const ::rtl::OUString& _rCommand,
            Reference< XComponent >& _rxKeepFieldsAlive, ::dbtools::SQLExceptionInfo* _pErrorInfo ) SAL_THROW( ( ) )
    {
        return ::dbtools::getFieldsByCommandDescriptor( _rxConnection, _nCommandType, _rCommand,
            _rxKeepFieldsAlive, _pErrorInfo );
    }

    //----------------------------------------------------------------
    Sequence< ::rtl::OUString > ODataAccessStaticTools::getFieldNamesByCommandDescriptor(
        const Reference< XConnection >& _rxConnection, const sal_Int32 _nCommandType,
        const ::rtl::OUString& _rCommand, ::dbtools::SQLExceptionInfo* _pErrorInfo ) SAL_THROW( ( ) )
    {
        return ::dbtools::getFieldNamesByCommandDescriptor( _rxConnection, _nCommandType,
            _rCommand, _pErrorInfo );
    }

    //----------------------------------------------------------------
    sal_Bool ODataAccessStaticTools::isDataSourcePropertyEnabled(const Reference< XInterface>& _xProp
                                        ,const ::rtl::OUString& _sProperty,
                                        sal_Bool _bDefault) const
    {
        return ::dbtools::isDataSourcePropertyEnabled( _xProp,_sProperty ,_bDefault );
    }

//........................................................................
}   // namespace connectivity
//........................................................................

