/*************************************************************************
 *
 *  $RCSfile: staticdbtools_s.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:18:16 $
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

#ifndef CONNECTIVITY_STATIC_DBTOOLS_SIMPLE_HXX
#define CONNECTIVITY_STATIC_DBTOOLS_SIMPLE_HXX

#ifndef CONNECTIVITY_VIRTUAL_DBTOOLS_HXX
#include <connectivity/virtualdbtools.hxx>
#endif
#ifndef CONNECTIVITY_DBTOOLS_REFBASE_HXX
#include "refbase.hxx"
#endif

//........................................................................
namespace connectivity
{
//........................................................................

    //================================================================
    //= ODataAccessStaticTools
    //================================================================
    class ODataAccessStaticTools
            :public simple::IDataAccessTypeConversion
            ,public simple::IDataAccessTools
            ,public ORefBase
    {
    public:
        ODataAccessStaticTools();

        // IDataAccessTypeConversion
        // ------------------------------------------------
        virtual ::com::sun::star::util::Date getStandardDate() const;

        // ------------------------------------------------
        virtual double getValue(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>& _rxVariant,
            const ::com::sun::star::util::Date& rNullDate,
            sal_Int16 nKeyType) const;

        // ------------------------------------------------
        virtual ::rtl::OUString getValue(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxColumn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::util::Date& _rNullDate,
            sal_Int32 _nKey,
            sal_Int16 _nKeyType) const;

        // ------------------------------------------------
        virtual ::rtl::OUString getValue(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxColumn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter>& _rxFormatter,
            const ::com::sun::star::lang::Locale& _rLocale,
            const ::com::sun::star::util::Date& _rNullDate
        ) const;

        // IDataAccessTools
        // ------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection_withFeedback(
            const ::rtl::OUString& _rDataSourceName,
            const ::rtl::OUString& _rUser,
            const ::rtl::OUString& _rPwd,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory
        ) const SAL_THROW ( (::com::sun::star::sdbc::SQLException) );

        // ------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> calcConnection(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory
        ) const SAL_THROW ( (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) );

        // ------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> connectRowset(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
            sal_Bool _bSetAsActiveConnection
        ) const SAL_THROW ( (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) );

        // ------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getRowSetConnection(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet)
                const SAL_THROW ( (::com::sun::star::uno::RuntimeException) );

        // ------------------------------------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier> getNumberFormats(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn,
            sal_Bool _bAllowDefault
        ) const;

        // ------------------------------------------------
        virtual sal_Int32  getDefaultNumberFormat(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatTypes >& _rxTypes,
            const ::com::sun::star::lang::Locale& _rLocale
        ) const;

        // ------------------------------------------------
        virtual void TransferFormComponentProperties(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxOld,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxNew,
            const ::com::sun::star::lang::Locale& _rLocale
        ) const;

        // ------------------------------------------------
        virtual ::rtl::OUString quoteName(
            const ::rtl::OUString& _rQuote,
            const ::rtl::OUString& _rName
        ) const;

        // ------------------------------------------------
        virtual ::rtl::OUString quoteTableName(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _rxMeta,
            const ::rtl::OUString& _rName
            , sal_Bool _bUseCatalogInSelect = sal_True
            , sal_Bool _bUseSchemaInSelect = sal_True
        ) const;

        // ------------------------------------------------
        virtual ::com::sun::star::sdb::SQLContext prependContextInfo(
            ::com::sun::star::sdbc::SQLException& _rException,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
            const ::rtl::OUString& _rContextDescription,
            const ::rtl::OUString& _rContextDetails
        ) const;

        // ------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource > getDataSource(
            const ::rtl::OUString& _rsRegisteredName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory
        ) const;

        // ------------------------------------------------
        /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::INSERT
            @param      _rxCursorSet    the property set
        */
        virtual sal_Bool canInsert(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet) const;

        // ------------------------------------------------
        /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::UPDATE
            @param      _rxCursorSet    the property set
        */
        virtual sal_Bool canUpdate(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet) const;

        // ------------------------------------------------
        /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::DELETE
            @param      _rxCursorSet    the property set
        */
        virtual sal_Bool canDelete(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet) const;

        // ------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
            getFieldsByCommandDescriptor(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const sal_Int32 _nCommandType,
                const ::rtl::OUString& _rCommand,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxKeepFieldsAlive,
                ::dbtools::SQLExceptionInfo* _pErrorInfo = NULL
            )   SAL_THROW( ( ) );

        // ------------------------------------------------
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
            getFieldNamesByCommandDescriptor(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const sal_Int32 _nCommandType,
                const ::rtl::OUString& _rCommand,
                ::dbtools::SQLExceptionInfo* _pErrorInfo = NULL
            )   SAL_THROW( ( ) );

        virtual sal_Bool isDataSourcePropertyEnabled(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xProp
                                        ,const ::rtl::OUString& _sProperty,
                                        sal_Bool _bDefault = sal_False) const;
        // disambiguate IReference
        // ------------------------------------------------
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();
    };

//........................................................................
}   // namespace connectivity
//........................................................................

#endif // CONNECTIVITY_STATIC_DBTOOLS_SIMPLE_HXX

