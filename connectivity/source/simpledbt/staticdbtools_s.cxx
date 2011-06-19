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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include <connectivity/virtualdbtools.hxx>
#include "staticdbtools_s.hxx"
#include "connectivity/dbconversion.hxx"
#include <connectivity/dbtools.hxx>
#include <com/sun/star/sdb/SQLContext.hpp>

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
    double ODataAccessStaticTools::getValue(const Reference< XColumn>& _rxVariant, const Date& rNullDate ) const
    {
        return ::dbtools::DBTypeConversion::getValue( _rxVariant, rNullDate );
    }

    //----------------------------------------------------------------
    ::rtl::OUString ODataAccessStaticTools::getFormattedValue(const Reference< XColumn >& _rxColumn, const Reference< XNumberFormatter >& _rxFormatter,
        const Date& _rNullDate, sal_Int32 _nKey, sal_Int16 _nKeyType) const
    {
        return ::dbtools::DBTypeConversion::getFormattedValue(_rxColumn, _rxFormatter, _rNullDate, _nKey, _nKeyType);
    }

    //----------------------------------------------------------------
    ::rtl::OUString ODataAccessStaticTools::getFormattedValue( const Reference< XPropertySet>& _rxColumn, const Reference< XNumberFormatter>& _rxFormatter,
        const Locale& _rLocale, const Date& _rNullDate ) const
    {
        return ::dbtools::DBTypeConversion::getFormattedValue( _rxColumn, _rxFormatter, _rLocale, _rNullDate );
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
        SAL_THROW ( (SQLException, WrappedTargetException, RuntimeException) )
    {
        return ::dbtools::connectRowset( _rxRowSet, _rxFactory, _bSetAsActiveConnection);
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
    ::rtl::OUString ODataAccessStaticTools::composeTableNameForSelect( const Reference< XConnection >& _rxConnection, const ::rtl::OUString& _rCatalog, const ::rtl::OUString& _rSchema, const ::rtl::OUString& _rName ) const
    {
        return ::dbtools::composeTableNameForSelect( _rxConnection, _rCatalog, _rSchema, _rName );
    }

    //----------------------------------------------------------------
    ::rtl::OUString ODataAccessStaticTools::composeTableNameForSelect( const Reference< XConnection >& _rxConnection, const Reference< XPropertySet>& _xTable ) const
    {
        return ::dbtools::composeTableNameForSelect( _rxConnection, _xTable );
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

    // ------------------------------------------------
    bool ODataAccessStaticTools::isEmbeddedInDatabase( const Reference< XInterface >& _rxComponent, Reference< XConnection >& _rxActualConnection )
    {
        return ::dbtools::isEmbeddedInDatabase( _rxComponent, _rxActualConnection );
    }

//........................................................................
}   // namespace connectivity
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
