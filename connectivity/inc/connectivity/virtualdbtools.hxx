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

#ifndef CONNECTIVITY_VIRTUAL_DBTOOLS_HXX
#define CONNECTIVITY_VIRTUAL_DBTOOLS_HXX

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <vector>
#include <memory>
#include "connectivity/dbtoolsdllapi.hxx"

//========================================================================
//= forward declarations
//========================================================================
namespace com {
    namespace sun {
        namespace star {
            namespace util {
                class XNumberFormatter;
                class XNumberFormatTypes;
                class XNumberFormatsSupplier;
            }
            namespace beans {
                class XPropertySet;
            }
            namespace lang {
                class XMultiServiceFactory;
                class XComponent;
                class WrappedTargetException;
                struct Locale;
            }
            namespace sdbc {
                class XDatabaseMetaData;
                class XConnection;
                class XRowSet;
                class XDataSource;
            }
            namespace sdb {
                class XColumn;
                class SQLContext;
            }
            namespace container {
                class XNameAccess;
            }
        }
    }
}

namespace dbtools {
    class SQLExceptionInfo;
    class FormattedColumnValue;
}

namespace comphelper {
    class ComponentContext;
}

//========================================================================
//= entry into this library
//========================================================================
/** this is the entry point for the load-on-call usage of the DBTOOLS
    library.
    <p>When you need one of the simple objects in this library, load the lib
    and call this method. The returned pointer is a pointer to an IDataAccessToolsFactory
    instance, which is acquired <em>once</em>.</p>
    @return
        a pointer to an object implementing the IDataAccessToolsFactory interface,
        aquired exactly <em>once</em>.
*/
extern "C" OOO_DLLPUBLIC_DBTOOLS void* SAL_CALL createDataAccessToolsFactory();

//========================================================================
//=
//========================================================================
//........................................................................
namespace connectivity
{
//........................................................................

    class IParseContext;
    //....................................................................
    namespace simple
    {
    //....................................................................

        typedef void* (SAL_CALL * createDataAccessToolsFactoryFunction)( );

        //================================================================
        //= IDataAccessTools
        //================================================================
        class OOO_DLLPUBLIC_DBTOOLS IDataAccessTools : public ::rtl::IReference
        {
        public:
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection_withFeedback(
                const ::rtl::OUString& _rDataSourceName,
                const ::rtl::OUString& _rUser,
                const ::rtl::OUString& _rPwd,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory
            ) const SAL_THROW ( (::com::sun::star::sdbc::SQLException) ) = 0;

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> connectRowset(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
                sal_Bool _bSetAsActiveConnection
            ) const SAL_THROW ( ( ::com::sun::star::sdbc::SQLException
                                , ::com::sun::star::lang::WrappedTargetException
                                , ::com::sun::star::uno::RuntimeException ) ) = 0;

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getRowSetConnection(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet)
                const SAL_THROW ( (::com::sun::star::uno::RuntimeException) ) = 0;

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier> getNumberFormats(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn,
                sal_Bool _bAllowDefault
            ) const = 0;

            virtual sal_Int32 getDefaultNumberFormat(
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn,
                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatTypes >& _rxTypes,
                const ::com::sun::star::lang::Locale& _rLocale
            ) const = 0;

            virtual void TransferFormComponentProperties(
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxOld,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxNew,
                const ::com::sun::star::lang::Locale& _rLocale
            ) const = 0;

            virtual ::rtl::OUString quoteName(
                const ::rtl::OUString& _rQuote,
                const ::rtl::OUString& _rName
            ) const = 0;

            virtual ::rtl::OUString composeTableNameForSelect(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const ::rtl::OUString& _rCatalog,
                const ::rtl::OUString& _rSchema,
                const ::rtl::OUString& _rName
            ) const = 0;

            virtual ::rtl::OUString composeTableNameForSelect(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xTable
            ) const = 0;

            virtual ::com::sun::star::sdb::SQLContext prependContextInfo(
                ::com::sun::star::sdbc::SQLException& _rException,
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
                const ::rtl::OUString& _rContextDescription,
                const ::rtl::OUString& _rContextDetails
            ) const = 0;

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource > getDataSource(
                const ::rtl::OUString& _rsRegisteredName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory
            ) const = 0;

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                getFieldsByCommandDescriptor(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                    const sal_Int32 _nCommandType,
                    const ::rtl::OUString& _rCommand,
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxKeepFieldsAlive,
                    ::dbtools::SQLExceptionInfo* _pErrorInfo = NULL
                )   SAL_THROW( ( ) ) = 0;

            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
                getFieldNamesByCommandDescriptor(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                    const sal_Int32 _nCommandType,
                    const ::rtl::OUString& _rCommand,
                    ::dbtools::SQLExceptionInfo* _pErrorInfo = NULL
                )   SAL_THROW( ( ) ) = 0;

            /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::INSERT
                @param      _rxCursorSet    the property set
            */
            virtual sal_Bool canInsert(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet) const = 0;

            /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::UPDATE
                @param      _rxCursorSet    the property set
            */
            virtual sal_Bool canUpdate(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet) const = 0;

            /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::DELETE
                @param      _rxCursorSet    the property set
            */
            virtual sal_Bool canDelete(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet) const = 0;

            /** determines whether the given component is part of a document which is an embedded database
                document (such as a form)
            */
            virtual bool isEmbeddedInDatabase(
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent,
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxActualConnection
            ) = 0;
        };

        //================================================================
        //= IDataAccessCharSet
        //================================================================
        /** simple wrapper for the <type>OCharsetMap</type>
        */
        class OOO_DLLPUBLIC_DBTOOLS IDataAccessCharSet :
            public ::rtl::IReference
        {
            // to be extended if necessary ....
        public:
            /** enumerates all supported char sets
            @return the number of charsets supported
            */
            virtual sal_Int32   getSupportedTextEncodings(
                ::std::vector< rtl_TextEncoding >& /* [out] */ _rEncs
            ) const = 0;
        };

        //================================================================
        //= IDataAccessTypeConversion
        //================================================================
        class OOO_DLLPUBLIC_DBTOOLS IDataAccessTypeConversion :
            public ::rtl::IReference
        {
        public:
            virtual ::com::sun::star::util::Date getStandardDate() const = 0;

            virtual double getValue(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>& _rxVariant,
                const ::com::sun::star::util::Date& rNullDate ) const = 0;

            virtual ::rtl::OUString getFormattedValue(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxColumn,
                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
                const ::com::sun::star::util::Date& _rNullDate,
                sal_Int32 _nKey,
                sal_Int16 _nKeyType) const = 0;

            virtual ::rtl::OUString getFormattedValue(
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxColumn,
                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter>& _rxFormatter,
                const ::com::sun::star::lang::Locale& _rLocale,
                const ::com::sun::star::util::Date& _rNullDate
            ) const = 0;
        };

        //================================================================
        //= ISQLParseNode
        //================================================================
        /** a simple version of the OSQLParseNode, with all methods beeing virtual
        */
        class OOO_DLLPUBLIC_DBTOOLS ISQLParseNode : public ::rtl::IReference
        {
        public:
            virtual void parseNodeToStr(::rtl::OUString& _rString,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const IParseContext* _pContext
            ) const = 0;

            virtual void parseNodeToPredicateStr(::rtl::OUString& _rString,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxField,
                const ::com::sun::star::lang::Locale& _rIntl,
                const sal_Char _cDecSeparator,
                const IParseContext* _pContext
            ) const = 0;
        };

        //================================================================
        //= ISQLParser
        //================================================================
        /** a simple version of the OSQLParser, with all methods beeing virtual
        */
        class OOO_DLLPUBLIC_DBTOOLS ISQLParser : public ::rtl::IReference
        {
        public:
            virtual ::rtl::Reference< ISQLParseNode > predicateTree(
                ::rtl::OUString& rErrorMessage,
                const ::rtl::OUString& rStatement,
                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxField
            ) const = 0;

            virtual const IParseContext& getContext() const = 0;
        };

        //================================================================
        //= IDataAccessToolsFactory
        //================================================================
        /** the main factory for runtime-loadable tools in the DBTOOLS library
        */
        class OOO_DLLPUBLIC_DBTOOLS IDataAccessToolsFactory :
            public ::rtl::IReference
        {
        public:
            /// creates a simple version of the class OSQLParser
            virtual ::rtl::Reference< ISQLParser > createSQLParser(
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceFactory,
                const IParseContext* _pContext
                ) const = 0;

            /// creates a helper for charset related functionality (<type>OCharsetMap</type>)
            virtual ::rtl::Reference< IDataAccessCharSet > createCharsetHelper( ) const = 0;

            /// creates a simple version of the DBTypeConversion helper
            virtual ::rtl::Reference< IDataAccessTypeConversion > getTypeConversionHelper() = 0;

            /// creates a helper which can be used to access the static methods in dbtools.hxx
            virtual ::rtl::Reference< IDataAccessTools > getDataAccessTools() = 0;

            virtual ::std::auto_ptr< ::dbtools::FormattedColumnValue >  createFormattedColumnValue(
                const ::comphelper::ComponentContext& _rContext,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn
            ) = 0;
        };

    //....................................................................
    }   //  namespace simple
    //....................................................................

//........................................................................
}   // namespace connectivity
//........................................................................

#endif // CONNECTIVITY_VIRTUAL_DBTOOLS_HXX

