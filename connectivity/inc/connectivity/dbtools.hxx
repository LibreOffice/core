/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#define _CONNECTIVITY_DBTOOLS_HXX_

#include <connectivity/dbexception.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <comphelper/stl_types.hxx>
#include <unotools/sharedunocomponent.hxx>
#include "connectivity/dbtoolsdllapi.hxx"
#include "connectivity/FValue.hxx"

namespace com { namespace sun { namespace star {

namespace sdb {
    class XSingleSelectQueryComposer;
    class SQLContext;
}
namespace sdbcx {
    class XTablesSupplier;
}
namespace sdbc {
    class XConnection;
    class XDatabaseMetaData;
    class XRowSet;
    class XDataSource;
    class SQLException;
    class XParameters;
    class XRowUpdate;
}
namespace beans {
    class XPropertySet;
}
namespace awt {
    class XWindow;
}
namespace lang {
    struct Locale;
    class XMultiServiceFactory;
    class WrappedTargetException;
}
namespace container {
    class XNameAccess;
}
namespace uno {
    class XComponentContext;
}
namespace util {
    class XNumberFormatTypes;
    class XNumberFormatsSupplier;
}
namespace task {
    class XInteractionHandler;
}

} } }

namespace rtl
{
    class OUStringBuffer;
}

//.........................................................................
namespace dbtools
{
    class ISQLStatementHelper;
    typedef ::utl::SharedUNOComponent< ::com::sun::star::sdbc::XConnection > SharedConnection;

    enum EComposeRule
    {
        eInTableDefinitions,
        eInIndexDefinitions,
        eInDataManipulation,
        eInProcedureCalls,
        eInPrivilegeDefinitions,
        eComplete
    };
//=========================================================================
    // date conversion

    // calculates the default numberformat for a given datatype and a give language
    OOO_DLLPUBLIC_DBTOOLS
    sal_Int32 getDefaultNumberFormat(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xColumn,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatTypes >& _xTypes,
                                     const ::com::sun::star::lang::Locale& _rLocale);

    // calculates the default numberformat for a given datatype and a give language
    // @param  _nDataType @see com.sun.star.sdbc.DataType
    // @param _nScale       can be zero
    OOO_DLLPUBLIC_DBTOOLS
    sal_Int32 getDefaultNumberFormat(sal_Int32 _nDataType,
                                     sal_Int32 _nScale,
                                     sal_Bool _bIsCurrency,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatTypes >& _xTypes,
                                     const ::com::sun::star::lang::Locale& _rLocale);

//=========================================================================

    /** creates a connection which can be used for the rowset given

        The function tries to obtain a connection for the row set with the following
        steps (in this order):
        <nl>
            <li>If the rowset already has an ActiveConnection (means a non-<NULL/> value vor this property),
                this one is used.</li>
            <li>If row set is part of a database form document (see ->isEmbeddedInDatabase),
                a connection for the respective database is used.</li>
            <li>If in the parent hierarchy of the row set, there is an object supporting
                the XConnection interface, this one is returned.</li>
            <li>If the DataSourceName property of the row set is not empty, a connection for this
                data source is retrieved.</li>
            <li>If the URL property of the row set is not empty, an connection for this URL is
                retrieved from the driver manager.
        </nl>

        @param _rxRowSet
            the row set

        @param _rxFactory
            a service factory, which can be used to create data sources, interaction handler etc (the usual stuff)

        @param _bSetAsActiveConnection
            If <TRUE/>, the calculated connection is set as ActiveConnection property on the rowset.

            If the connection was newly created by the method, and this parameter is <TRUE/>, then
            the ownership of the connection is delivered to a temporary object, which observes the
            row set: As soon as a connection-relevant property of the row set changes, or as soon
            as somebody else sets another ActiveConnection at the row set, the original
            connection (the one which this function calculated) is disposed and discarded. At this
            very moment, also the temporary observer object dies. This way, it is ensured that
            there's no resource leak from an un-owned connection object.
    */
    OOO_DLLPUBLIC_DBTOOLS
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> connectRowset(
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
        sal_Bool _bSetAsActiveConnection
    )   SAL_THROW ( ( ::com::sun::star::sdbc::SQLException
                    , ::com::sun::star::lang::WrappedTargetException
                    , ::com::sun::star::uno::RuntimeException ) );

    /** ensures that a row set has a valid ActiveConnection, if possible

        This function does nearly the same as ->connectRowset. In fact, it is to be preferred over
        ->connectRowset, if possible.

        There are a few differences:
        <ul><li>If a connection could be determined for the given RowSet, it is always
                set as ActiveConnection.</li>
            <li>Definition of the ownership of the created connection allows for more scenarios:
                <ul><li>If the connection was not newly created, the returned ->SharedConnection
                        instance will not have the ownership, since in this case it's assumed
                        that there already is an instance which has the ownership.</li>
                    <li>If the connection was newly created, and ->_bUseAutoConnectionDisposer
                        is <TRUE/>, then the returned SharedConnection instance will <em>not</em>
                        be the owner of the connection. Instead, the ownership will be delivered
                        to a temporary object as described for connectRowset.</li>
                    <li>If the connection was newly created, and ->_bUseAutoConnectionDisposer
                        is <FALSE/>, then the returned SharedConnection instance will have the
                        ownership of the XConnection.</li>
                </ul>
            </li>
        </ul>
    */
    OOO_DLLPUBLIC_DBTOOLS SharedConnection    ensureRowSetConnection(
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
        bool _bUseAutoConnectionDisposer
    )   SAL_THROW ( ( ::com::sun::star::sdbc::SQLException
                    , ::com::sun::star::lang::WrappedTargetException
                    , ::com::sun::star::uno::RuntimeException ) );

    /** returns the connection the RowSet is currently working with (which is the ActiveConnection property)
    */
    OOO_DLLPUBLIC_DBTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet) throw (::com::sun::star::uno::RuntimeException);
    OOO_DLLPUBLIC_DBTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection(
            const ::rtl::OUString& _rsTitleOrPath,
            const ::rtl::OUString& _rsUser,
            const ::rtl::OUString& _rsPwd,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    OOO_DLLPUBLIC_DBTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection_withFeedback(
            const ::rtl::OUString& _rDataSourceName,
            const ::rtl::OUString& _rUser,
            const ::rtl::OUString& _rPwd,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxContext)
        SAL_THROW ( (::com::sun::star::sdbc::SQLException) );


    /** determines whether the given component is part of a document which is an embedded database
        document (such as a form)
    */
    OOO_DLLPUBLIC_DBTOOLS bool    isEmbeddedInDatabase(
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent,
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxActualConnection
            );

    /** returns the columns of the named table of the given connection
    */
    OOO_DLLPUBLIC_DBTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getTableFields(
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn,
        const ::rtl::OUString& _rName
    );

    /** returns the primary key columns of the table
    */
    OOO_DLLPUBLIC_DBTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getPrimaryKeyColumns_throw(
        const ::com::sun::star::uno::Any& i_aTable
    );
    OOO_DLLPUBLIC_DBTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getPrimaryKeyColumns_throw(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& i_xTable
    );

    /** get fields for a result set given by a "command descriptor"

        <p>A command descriptor here means:
        <ul><li>a SDB-level connection (<type scope="com.sun.star.sdb">Connection</type></li>
            <li>a string specifying the name of an object relative to the connection</li>
            <li>a <type scope="com.sun.star.sdb">CommandType</type> value specifying the type
                of the object</type></li>
        </ul>
        </p>

        @param _rxConnection
            the connection relative to which the to-be-examined object exists

        @param _nCommandType
            the type of the object

        @param _rCommand
            the object. This may be a table name, a query name, or an SQL statement, depending on the value
            of <arg>_nCommandType</arg>

        @param _rxCollectionOner
            If (and only if) <arg>CommandType</arg> is CommandType.COMMAND, the fields collection which is returned
            by this function here is a temporary object. It is kept alive by another object, which is to be
            created temporarily, too. To ensure that the fields you get are valid as long as you need them,
            the owner which controls their life time is transfered to this parameter upon return.<br/>

            Your fields live as long as this component lives.<br/>

            Additionally, you are encouraged to dispose this component as soon as you don't need the fields anymore.
            It depends on the connection's implementation if this is necessary, but the is no guarantee, so to
            be on the safe side with respect to resource leaks, you should dispose the component.

        @param _pErrorInfo
            If not <NULL/>, then upon return from the function the instance pointed to by this argument will
            contain any available error information in case something went wrong.

        @return
            the container of the columns (aka fields) of the object
    */
    OOO_DLLPUBLIC_DBTOOLS
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
        getFieldsByCommandDescriptor(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const sal_Int32 _nCommandType,
            const ::rtl::OUString& _rCommand,
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxKeepFieldsAlive,
            SQLExceptionInfo* _pErrorInfo = NULL
        )   SAL_THROW( ( ) );


    /** get fields for a result set given by a "command descriptor"

        <p>A command descriptor here means:
        <ul><li>a SDB-level connection (<type scope="com.sun.star.sdb">Connection</type></li>
            <li>a string specifying the name of an object relative to the connection</li>
            <li>a <type scope="com.sun.star.sdb">CommandType</type> value specifying the type
                of the object</type></li>
        </ul>
        </p>

        @param _rxConnection
            the connection relative to which the to-be-examined object exists

        @param _nCommandType
            the type of the object

        @param _rCommand
            the object. This may be a table name, a query name, or an SQL statement, depending on the value
            of <arg>_nCommandType</arg>

        @param _pErrorInfo
            If not <NULL/>, then upon return from the function the instance pointed to by this argument will
            contain any available error information in case something went wrong.

        @return
            an array of strings containing the names of the columns (aka fields) of the object
    */
    OOO_DLLPUBLIC_DBTOOLS ::com::sun::star::uno::Sequence< ::rtl::OUString >
        getFieldNamesByCommandDescriptor(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const sal_Int32 _nCommandType,
            const ::rtl::OUString& _rCommand,
            SQLExceptionInfo* _pErrorInfo = NULL
        )   SAL_THROW( ( ) );


    /** create a new ::com::sun::star::sdbc::SQLContext, fill it with the given descriptions and the given source,
        and <i>append</i> _rException (i.e. put it into the NextException member of the SQLContext).
    */
    OOO_DLLPUBLIC_DBTOOLS ::com::sun::star::sdb::SQLContext prependContextInfo(const ::com::sun::star::sdbc::SQLException& _rException, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext, const ::rtl::OUString& _rContextDescription, const ::rtl::OUString& _rContextDetails );

    OOO_DLLPUBLIC_DBTOOLS
    ::com::sun::star::sdbc::SQLException prependErrorInfo(
        const ::com::sun::star::sdbc::SQLException& _rChainedException,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
        const ::rtl::OUString& _rAdditionalError,
        const StandardSQLState _eSQLState = SQL_ERROR_UNSPECIFIED,
        const sal_Int32 _nErrorCode = 0);

    /** search the parent hierachy for a data source.
    */
    OOO_DLLPUBLIC_DBTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> findDataSource(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xParent);

    /** determines the value of a booolean data source setting, given by ASCII name

        @param _rxConnection
            the connection belonging to the data source whose setting is to be retrieved
        @param _pAsciiSettingName
            the ASCII name of the setting
    */
    OOO_DLLPUBLIC_DBTOOLS bool getBooleanDataSourceSetting(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const sal_Char* _pAsciiSettingName
        );

    /** check if a specific property is enabled in the info sequence
        @deprecated
            Use getBooleanDataSourceSetting instead, which cares for the default of the property itself,
            instead of spreading this knowledge through all callers.
    */
    OOO_DLLPUBLIC_DBTOOLS
    sal_Bool isDataSourcePropertyEnabled(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xProp
                                        ,const ::rtl::OUString& _sProperty,
                                        sal_Bool _bDefault = sal_False);

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
    OOO_DLLPUBLIC_DBTOOLS
    bool    getDataSourceSetting(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDataSource,
        const sal_Char* _pAsciiSettingsName,
        ::com::sun::star::uno::Any& /* [out] */ _rSettingsValue
    );
    OOO_DLLPUBLIC_DBTOOLS
    bool    getDataSourceSetting(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDataSource,
        const ::rtl::OUString& _sSettingsName,
        ::com::sun::star::uno::Any& /* [out] */ _rSettingsValue
    );

    OOO_DLLPUBLIC_DBTOOLS ::rtl::OUString getDefaultReportEngineServiceName(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    /** quote the given name with the given quote string.
    */
    OOO_DLLPUBLIC_DBTOOLS ::rtl::OUString quoteName(const ::rtl::OUString& _rQuote, const ::rtl::OUString& _rName);

    /** quote the given table name (which may contain a catalog and a schema) according to the rules provided by the meta data
    */
    OOO_DLLPUBLIC_DBTOOLS
    ::rtl::OUString quoteTableName(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _rxMeta
                                    , const ::rtl::OUString& _rName
                                    ,EComposeRule _eComposeRule);

    /** split a fully qualified table name (including catalog and schema, if appliable) into it's component parts.
        @param  _rxConnMetaData     meta data describing the connection where you got the table name from
        @param  _rQualifiedName     fully qualified table name
        @param  _rCatalog           (out parameter) upon return, contains the catalog name
        @param  _rSchema            (out parameter) upon return, contains the schema name
        @param  _rName              (out parameter) upon return, contains the table name
        @param  _eComposeRule       where do you need the name for
    */
    OOO_DLLPUBLIC_DBTOOLS void qualifiedNameComponents(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxConnMetaData,
        const ::rtl::OUString& _rQualifiedName, ::rtl::OUString& _rCatalog, ::rtl::OUString& _rSchema, ::rtl::OUString& _rName,EComposeRule _eComposeRule);

    /** calculate a NumberFormatsSupplier for use with an given connection
        @param      _rxConn         the connection for which the formatter is requested
        @param      _bAllowDefault  if the connection (and related components, such as it's parent) cannot supply
                                    a formatter, we can ask the DatabaseEnvironment for a default one. This parameter
                                    states if this is allowed.
        @param      _rxFactory      required (only of _bAllowDefault is sal_True) for creating the DatabaseEnvironment.
        @return     the formatter all object related to the given connection should work with.
    */
    OOO_DLLPUBLIC_DBTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier> getNumberFormats(
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn,
        sal_Bool _bAllowDefault = sal_False,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>()
    );

    /** create an <type scope="com::sun::star::sdb">XSingleSelectQueryComposer</type> which represents
        the current settings (Command/CommandType/Filter/Order) of the given rowset.

        As such an instance can be obtained from a <type scope="com::sun::star::sdb">Connection</type>
        only the function searches for the connection the RowSet is using via connectRowset.
        This implies that a connection will be set on the RowSet if needed.
        (need to changes this sometimes ...)
    */
    OOO_DLLPUBLIC_DBTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer > getCurrentSettingsComposer(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxRowSetProps,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory
    );

    /** transfer and translate properties between two FormComponents
        @param      _rxOld      the source property set
        @param      _rxNew      the destination property set
        @param      _rLocale    the locale for converting number related properties
    */
    OOO_DLLPUBLIC_DBTOOLS void TransferFormComponentProperties(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxOld,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxNew,
        const ::com::sun::star::lang::Locale& _rLocale
        );

    /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::INSERT
        @param      _rxCursorSet    the property set
    */
    OOO_DLLPUBLIC_DBTOOLS sal_Bool canInsert(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet);
    /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::UPDATE
        @param      _rxCursorSet    the property set
    */
    OOO_DLLPUBLIC_DBTOOLS sal_Bool canUpdate(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet);
    /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::DELETE
        @param      _rxCursorSet    the property set
    */
    OOO_DLLPUBLIC_DBTOOLS sal_Bool canDelete(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet);

    //----------------------------------------------------------------------------------
    /** compose a complete table name from it's up to three parts, regarding to the database meta data composing rules
    */
    OOO_DLLPUBLIC_DBTOOLS ::rtl::OUString composeTableName( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxMetaData,
                            const ::rtl::OUString& _rCatalog,
                            const ::rtl::OUString& _rSchema,
                            const ::rtl::OUString& _rName,
                            sal_Bool _bQuote,
                            EComposeRule _eComposeRule);

    /** composes a table name for usage in a SELECT statement

        This includes quoting of the table as indicated by the connection's meta data, plus respecting
        the settings "UseCatalogInSelect" and "UseSchemaInSelect", which might be present
        in the data source which the connection belongs to.
    */
    OOO_DLLPUBLIC_DBTOOLS ::rtl::OUString composeTableNameForSelect(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                            const ::rtl::OUString& _rCatalog,
                            const ::rtl::OUString& _rSchema,
                            const ::rtl::OUString& _rName );

    /** composes a table name for usage in a SELECT statement

        This includes quoting of the table as indicated by the connection's meta data, plus respecting
        the settings "UseCatalogInSelect" and "UseSchemaInSelect", which might be present
        in the data source which the connection belongs to.
    */
    OOO_DLLPUBLIC_DBTOOLS ::rtl::OUString composeTableNameForSelect(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xTable );
    //----------------------------------------------------------------------------------
    /** compose the table name out of the property set which must support the properties from the service <member scope= "com::sun::star::sdbcx">table</member>
        @param  _xMetaData
            The metadata from the connection.
        @param  _xTable
            The table.
    */
    OOO_DLLPUBLIC_DBTOOLS ::rtl::OUString composeTableName(
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xTable,
        EComposeRule _eComposeRule,
        bool _bSuppressCatalogName,
        bool _bSuppressSchemaName,
        bool _bQuote);

    //----------------------------------------------------------------------------------
    OOO_DLLPUBLIC_DBTOOLS sal_Int32 getSearchColumnFlag( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn,
                                    sal_Int32 _nDataType);
    // return the datasource for the given datasource name
    OOO_DLLPUBLIC_DBTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> getDataSource(const ::rtl::OUString& _rsDataSourceName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxContext);

    /** search for a name that is NOT in the NameAcces
        @param  _rxContainer
            the NameAccess container to search in
        @param  _rBaseName
            the base name that should be used to create the new name
        @param  _bStartWithNumber
            When <TRUE/> the name ends with number even when the name itself doesn't occur in the collection.
        @return
            A name which doesn't exist in the collection.
    */
    OOO_DLLPUBLIC_DBTOOLS
    ::rtl::OUString createUniqueName(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxContainer,
                                     const ::rtl::OUString& _rBaseName,
                                     sal_Bool _bStartWithNumber = sal_True);

    /** creates a unique name which is not already used in the given name array
    */
    OOO_DLLPUBLIC_DBTOOLS ::rtl::OUString createUniqueName(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rNames,
        const ::rtl::OUString& _rBaseName,
        sal_Bool _bStartWithNumber = sal_True
    );

    /** create a name which is a valid SQL 92 identifier name
        @param      _rName          the string which should be converted
        @param      _rSpecials      @see com.sun.star.sdbc.XDatabaseMetaData.getExtraNameCharacters

        @see isValidSQLName
    */
    OOO_DLLPUBLIC_DBTOOLS ::rtl::OUString convertName2SQLName(const ::rtl::OUString& _rName,const ::rtl::OUString& _rSpecials);

    /** checks whether the given name is a valid SQL name

        @param      _rName          the string which should be converted
        @param      _rSpecials      @see com.sun.star.sdbc.XDatabaseMetaData.getExtraNameCharacters

        @see convertName2SQLName
    */
    OOO_DLLPUBLIC_DBTOOLS sal_Bool isValidSQLName( const ::rtl::OUString& _rName, const ::rtl::OUString& _rSpecials );

    OOO_DLLPUBLIC_DBTOOLS
    void showError( const SQLExceptionInfo& _rInfo,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>& _pParent,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    /** implements <method scope="com.sun.star.sdb">XRowUpdate::updateObject</method>
        <p>The object which is to be set is analyzed, and in case it is a simlpe scalar type for which there
        is another updateXXX method, this other method is used.</p>
        @param _rxUpdatedObject
            the interface to forward all updateXXX calls to (except updateObject)
        @param _nColumnIndex
            the column index to update
        @param _rValue
            the value to update
        @return
            <TRUE/> if the update request was successfully re-routed to one of the other updateXXX methods
    */
    OOO_DLLPUBLIC_DBTOOLS
    sal_Bool implUpdateObject(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowUpdate >& _rxUpdatedObject,
                                const sal_Int32 _nColumnIndex,
                                const ::com::sun::star::uno::Any& _rValue)  SAL_THROW   (   (   ::com::sun::star::sdbc::SQLException,   ::com::sun::star::uno::RuntimeException)    );



    /** ask the user for parameters if the prepared statement needs some and sets them in the prepared statement
        @param _xConnection     the connection must be able to create <type scope="com::sun::star::sdb">SingleSelectQueryComposer</type>s
        @param _xPreparedStmt   the prepared statement where the parameters could be set when needed
        @param _aParametersSet  contains which parameters have to asked for and which already have set.
    */
    OOO_DLLPUBLIC_DBTOOLS
    void askForParameters(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >& _xComposer,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XParameters>& _xParameters,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxHandler,
                            const ::std::vector<bool, std::allocator<bool> >& _aParametersSet = ::std::vector<bool, std::allocator<bool> >());

    /** call the appropriate set method for the specific sql type @see com::sun::star::sdbc::DataType
        @param  _xParams        the parameters where to set the value
        @param  parameterIndex  the index of the parameter, 1 based
        @param  x               the value to set
        @param  sqlType         the corresponding sql type @see com::sun::star::sdbc::DataType
        @param  scale           the scale of the sql type can be 0
    */
    OOO_DLLPUBLIC_DBTOOLS
    void setObjectWithInfo( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XParameters>& _xParameters,
                            sal_Int32 parameterIndex,
                            const ::com::sun::star::uno::Any& x,
                            sal_Int32 sqlType,
                            sal_Int32 scale=0) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    /** call the appropriate set method for the specific sql type @see com::sun::star::sdbc::DataType
        @param  _xParams        the parameters where to set the value
        @param  parameterIndex  the index of the parameter, 1 based
        @param  x               the value to set
        @param  sqlType         the corresponding sql type @see com::sun::star::sdbc::DataType
        @param  scale           the scale of the sql type can be 0
    */
    OOO_DLLPUBLIC_DBTOOLS
    void setObjectWithInfo( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XParameters>& _xParameters,
                            sal_Int32 parameterIndex,
                            const ::connectivity::ORowSetValue& x,
                            sal_Int32 sqlType,
                            sal_Int32 scale=0) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);


    /** implements <method scope="com.sun.star.sdb">XParameters::setObject</method>
        <p>The object which is to be set is analyzed, and in case it is a simlpe scalar type for which there
        is another setXXX method, this other method is used.</p>
        @param _rxParameters
            the interface to forward all setXXX calls to (except setObject)
        @param _nColumnIndex
            the column index to update
        @param _rValue
            the value to update
        @return
            <TRUE/> if the update request was successfully re-routed to one of the other updateXXX methods
    */
    OOO_DLLPUBLIC_DBTOOLS
    sal_Bool implSetObject( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XParameters>& _rxParameters,
                            const sal_Int32 _nColumnIndex,
                            const ::com::sun::star::uno::Any& _rValue) SAL_THROW ( ( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) );

    /** creates the standard sql create table statement without the key part.
        @param  descriptor
            The descriptor of the new table.
        @param  _xConnection
            The connection.
        @param  _bAddScale
            The scale will also be added when the value is 0.
    */
    OOO_DLLPUBLIC_DBTOOLS
    ::rtl::OUString createStandardCreateStatement(  const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                                                    ISQLStatementHelper* _pHelper,
                                                    const ::rtl::OUString& _sCreatePattern = ::rtl::OUString());

    /** creates the standard sql statement for the key part of a create table statement.
        @param  descriptor
            The descriptor of the new table.
        @param  _xConnection
            The connection.
    */
    OOO_DLLPUBLIC_DBTOOLS
    ::rtl::OUString createStandardKeyStatement( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor,
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

    /** creates the standard sql statement for the column part of a create table statement.
        @param  _pHelper
            Allow to add special SQL constructs.
        @param  descriptor
            The descriptor of the column.
        @param  _xConnection
            The connection.
        @param  _pHelper
            Allow to add special SQL constructs.
    */
    OOO_DLLPUBLIC_DBTOOLS
    ::rtl::OUString createStandardColumnPart(   const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor
                                                ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection
                                                ,ISQLStatementHelper* _pHelper = NULL
                                                ,const ::rtl::OUString& _sCreatePattern = ::rtl::OUString());

    /** creates a SQL CREATE TABLE statement

        @param  descriptor
            The descriptor of the new table.
        @param  _xConnection
            The connection.
        @param  _pHelper
            Allow to add special SQL constructs.
        @param  _sCreatePattern

        @return
            The CREATE TABLE statement.
    */
    OOO_DLLPUBLIC_DBTOOLS
    ::rtl::OUString createSqlCreateTableStatement(  const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor
                                                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection
                                                    ,ISQLStatementHelper* _pHelper = NULL
                                                    ,const ::rtl::OUString& _sCreatePattern = ::rtl::OUString());

    /** creates a SDBC column with the help of getColumns.
        @param  _xTable
            The table.
        @param  _rName
            The name of the column.
        @param  _bCase
            Is the column case sensitive.
        @param  _bQueryForInfo
            If <TRUE/> the autoincrement and currency field will be read from the meta data, otherwise the following parameters will be used instead
        @param  _bIsAutoIncrement
            <TRUE/> if the column is an autoincrement.
        @param  _bIsCurrency
            <TRUE/> if the column is a currency field.
        @param  _nDataType
            The data type of the column.
    */
    OOO_DLLPUBLIC_DBTOOLS
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>
            createSDBCXColumn(  const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xTable,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                                const ::rtl::OUString& _rName,
                                sal_Bool _bCase,
                                sal_Bool _bQueryForInfo = sal_True,
                                sal_Bool _bIsAutoIncrement = sal_False,
                                sal_Bool _bIsCurrency = sal_False,
                                sal_Int32 _nDataType = com::sun::star::sdbc::DataType::OTHER);

    /** tries to locate the corresponding DataDefinitionSupplier for the given url and connection
        @param  _rsUrl
            The URL used to connect to the database.
        @param  _xConnection
            The connection used to find the correct driver.
        @param  _rxFactory
            Used to create the drivermanager.
        @return
            The datadefintion object.
    */
    OOO_DLLPUBLIC_DBTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier> getDataDefinitionByURLAndConnection(
            const ::rtl::OUString& _rsUrl,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    /** returns the table privileges to the given parameters
        @param  _xMetaData
            The meta data.
        @param  _sCatalog
            contains the catalog name
        @param  _sSchema
            contains the schema name
        @param  _sTable
            contains the table name
    */
    OOO_DLLPUBLIC_DBTOOLS
    sal_Int32 getTablePrivileges(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData,
                                 const ::rtl::OUString& _sCatalog,
                                 const ::rtl::OUString& _sSchema,
                                 const ::rtl::OUString& _sTable);

    typedef ::std::pair<sal_Bool,sal_Bool> TBoolPair;
    typedef ::std::pair< TBoolPair,sal_Int32 > ColumnInformation;
    typedef ::std::multimap< ::rtl::OUString, ColumnInformation, ::comphelper::UStringMixLess> ColumnInformationMap;
    /** collects the information about auto increment, currency and data type for the given column name.
        The column must be quoted, * is also valid.
        @param  _xConnection
            The connection.
        @param  _sComposedTableName
            The quoted table name. ccc.sss.ttt
        @param  _sName
            The name of the column, or *
        @param  _rInfo
            The information about the column(s).
    */
    OOO_DLLPUBLIC_DBTOOLS
    void collectColumnInformation(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                                    const ::rtl::OUString& _sComposedTableName,
                                    const ::rtl::OUString& _rName,
                                    ColumnInformationMap& _rInfo);


    /** adds a boolean comparison clause to the given SQL predicate

        @param _rExpression
            the expression which is to be compared with a boolean value
        @param _bValue
            the boolean value which the expression is to be compared with
        @param _nBooleanComparisonMode
            the boolean comparison mode to be used. Usually obtained from
            a css.sdb.DataSource's Settings member.
        @param _out_rSQLPredicate
            the buffer to which the comparison predicate will be appended
    */
    OOO_DLLPUBLIC_DBTOOLS void getBoleanComparisonPredicate(
            const ::rtl::OUString& _rExpression,
            const sal_Bool  _bValue,
            const sal_Int32 _nBooleanComparisonMode,
            ::rtl::OUStringBuffer& _out_rSQLPredicate
        );

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _CONNECTIVITY_DBTOOLS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
