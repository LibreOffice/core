/*************************************************************************
 *
 *  $RCSfile: virtualdbtools.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:49:51 $
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
#define CONNECTIVITY_VIRTUAL_DBTOOLS_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#include <vector>

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
extern "C" void* SAL_CALL createDataAccessToolsFactory();

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
        class IDataAccessTools : public ::rtl::IReference
        {
        public:
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection_withFeedback(
                const ::rtl::OUString& _rDataSourceName,
                const ::rtl::OUString& _rUser,
                const ::rtl::OUString& _rPwd,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory
            ) const SAL_THROW ( (::com::sun::star::sdbc::SQLException) ) = 0;

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> calcConnection(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory
            ) const SAL_THROW ( (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) ) = 0;

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> connectRowset(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
                sal_Bool _bSetAsActiveConnection
            ) const SAL_THROW ( (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) ) = 0;

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

            virtual ::rtl::OUString quoteTableName(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _rxMeta
                ,const ::rtl::OUString& _rName
                ,sal_Bool _bUseCatalogInSelect = sal_True
                ,sal_Bool _bUseSchemaInSelect = sal_True
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

            /** check if a specific property is enabled in the info sequence
                @param  _xProp
                    The datasource or a child of it.
                @param  _sProperty
                    The property to search in the info property of the data source.
                @param  _bDefault
                    This value will be returned, if the property doesn't exist in the data source.
                @return
                    <TRUE/> if so otherwise <FALSE/>
            */
            virtual sal_Bool isDataSourcePropertyEnabled(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xProp
                                        ,const ::rtl::OUString& _sProperty,
                                        sal_Bool _bDefault = sal_False) const = 0;
        };

        //================================================================
        //= IDataAccessCharSet
        //================================================================
        /** simple wrapper for the <type>OCharsetMap</type>
        */
        class IDataAccessCharSet : public ::rtl::IReference
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
        class IDataAccessTypeConversion : public ::rtl::IReference
        {
        public:
            virtual ::com::sun::star::util::Date getStandardDate() const = 0;

            virtual double getValue(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>& _rxVariant,
                const ::com::sun::star::util::Date& rNullDate,
                sal_Int16 nKeyType) const = 0;

            virtual ::rtl::OUString getValue(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxColumn,
                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
                const ::com::sun::star::util::Date& _rNullDate,
                sal_Int32 _nKey,
                sal_Int16 _nKeyType) const = 0;

            virtual ::rtl::OUString getValue(
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
        class ISQLParseNode : public ::rtl::IReference
        {
        public:
            virtual void parseNodeToStr(::rtl::OUString& _rString,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxMeta,
                const IParseContext* _pContext
            ) const = 0;

            virtual void parseNodeToPredicateStr(::rtl::OUString& _rString,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxMeta,
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
        class ISQLParser : public ::rtl::IReference
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
        class IDataAccessToolsFactory : public ::rtl::IReference
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
        };

    //....................................................................
    }   //  namespace simple
    //....................................................................

//........................................................................
}   // namespace connectivity
//........................................................................

#endif // CONNECTIVITY_VIRTUAL_DBTOOLS_HXX

