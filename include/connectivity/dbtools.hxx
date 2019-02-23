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

#ifndef INCLUDED_CONNECTIVITY_DBTOOLS_HXX
#define INCLUDED_CONNECTIVITY_DBTOOLS_HXX

#include <connectivity/dbexception.hxx>
#include <comphelper/stl_types.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <connectivity/FValue.hxx>

namespace com { namespace sun { namespace star {

namespace sdb {
    class XSingleSelectQueryComposer;
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

class SvStream;

namespace dbtools
{
    class ISQLStatementHelper;
    typedef ::utl::SharedUNOComponent< css::sdbc::XConnection > SharedConnection;

    enum class EComposeRule
    {
        InTableDefinitions,
        InIndexDefinitions,
        InDataManipulation,
        InProcedureCalls,
        InPrivilegeDefinitions,
        Complete
    };

    // date conversion

    // calculates the default numberformat for a given datatype and a give language
    OOO_DLLPUBLIC_DBTOOLS
    sal_Int32 getDefaultNumberFormat(const css::uno::Reference< css::beans::XPropertySet >& _xColumn,
                                     const css::uno::Reference< css::util::XNumberFormatTypes >& _xTypes,
                                     const css::lang::Locale& _rLocale);

    // calculates the default numberformat for a given datatype and a give language
    // @param  _nDataType @see com.sun.star.sdbc.DataType
    // @param _nScale       can be zero
    OOO_DLLPUBLIC_DBTOOLS
    sal_Int32 getDefaultNumberFormat(sal_Int32 _nDataType,
                                     sal_Int32 _nScale,
                                     bool _bIsCurrency,
                                     const css::uno::Reference< css::util::XNumberFormatTypes >& _xTypes,
                                     const css::lang::Locale& _rLocale);


    /** creates a connection which can be used for the rowset given

        The function tries to obtain a connection for the row set with the following
        steps (in this order):
        <nl>
            <li>If the rowset already has an ActiveConnection (means a non-<NULL/> value for this property),
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

        The calculated connection is set as ActiveConnection property on the rowset.

        If the connection was newly created by the method, then
        the ownership of the connection is delivered to a temporary object, which observes the
        row set: As soon as a connection-relevant property of the row set changes, or as soon
        as somebody else sets another ActiveConnection at the row set, the original
        connection (the one which this function calculated) is disposed and discarded. At this
        very moment, also the temporary observer object dies. This way, it is ensured that
        there's no resource leak from an un-owned connection object.

        @param _rxRowSet
            the row set

        @param _rxFactory
            a service factory, which can be used to create data sources, interaction handler etc (the usual stuff)

    */
    OOO_DLLPUBLIC_DBTOOLS
    css::uno::Reference< css::sdbc::XConnection> connectRowset(
        const css::uno::Reference< css::sdbc::XRowSet>& _rxRowSet,
        const css::uno::Reference< css::uno::XComponentContext>& _rxContext
    );

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
                    <li>If the connection was newly created, then the returned SharedConnection
                        instance will have the ownership of the XConnection.</li>
                </ul>
            </li>
        </ul>
    */
    OOO_DLLPUBLIC_DBTOOLS SharedConnection    ensureRowSetConnection(
        const css::uno::Reference< css::sdbc::XRowSet>& _rxRowSet,
        const css::uno::Reference< css::uno::XComponentContext>& _rxContext
    );

    /** returns the connection the RowSet is currently working with (which is the ActiveConnection property)

        @throws css::uno::RuntimeException
    */
    OOO_DLLPUBLIC_DBTOOLS css::uno::Reference< css::sdbc::XConnection> getConnection(const css::uno::Reference< css::sdbc::XRowSet>& _rxRowSet);
    OOO_DLLPUBLIC_DBTOOLS css::uno::Reference< css::sdbc::XConnection> getConnection_withFeedback(
            const OUString& _rDataSourceName,
            const OUString& _rUser,
            const OUString& _rPwd,
            const css::uno::Reference< css::uno::XComponentContext>& _rxContext);


    /** determines whether the given component is part of a document which is an embedded database
        document (such as a form)
    */
    OOO_DLLPUBLIC_DBTOOLS bool    isEmbeddedInDatabase(
                const css::uno::Reference< css::uno::XInterface >& _rxComponent,
                css::uno::Reference< css::sdbc::XConnection >& _rxActualConnection
            );

    /** returns the columns of the named table of the given connection
    */
    OOO_DLLPUBLIC_DBTOOLS css::uno::Reference< css::container::XNameAccess> getTableFields(
        const css::uno::Reference< css::sdbc::XConnection>& _rxConn,
        const OUString& _rName
    );

    /** returns the primary key columns of the table
    */
    OOO_DLLPUBLIC_DBTOOLS css::uno::Reference< css::container::XNameAccess> getPrimaryKeyColumns_throw(
        const css::uno::Any& i_aTable
    );
    OOO_DLLPUBLIC_DBTOOLS css::uno::Reference< css::container::XNameAccess> getPrimaryKeyColumns_throw(
        const css::uno::Reference< css::beans::XPropertySet >& i_xTable
    );

    /** get fields for a result set given by a "command descriptor"

        <p>A command descriptor here means:
        <ul><li>a SDB-level connection (com.sun.star.sdb::Connection</li>
            <li>a string specifying the name of an object relative to the connection</li>
            <li>a com.sun.star.sdb::CommandType value specifying the type
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
            the owner which controls their life time is transferred to this parameter upon return.<br/>

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
    css::uno::Reference< css::container::XNameAccess >
        getFieldsByCommandDescriptor(
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const sal_Int32 _nCommandType,
            const OUString& _rCommand,
            css::uno::Reference< css::lang::XComponent >& _rxKeepFieldsAlive,
            SQLExceptionInfo* _pErrorInfo = nullptr
        );


    /** get fields for a result set given by a "command descriptor"

        <p>A command descriptor here means:
        <ul><li>a SDB-level connection (com.sun.star.sdb::Connection</li>
            <li>a string specifying the name of an object relative to the connection</li>
            <li>a com.sun.star.sdb::CommandType value specifying the type
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
    OOO_DLLPUBLIC_DBTOOLS css::uno::Sequence< OUString >
        getFieldNamesByCommandDescriptor(
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const sal_Int32 _nCommandType,
            const OUString& _rCommand,
            SQLExceptionInfo* _pErrorInfo = nullptr
        );


    /** create a new css::sdbc::SQLContext, fill it with the given descriptions and the given source,
        and <i>append</i> _rException (i.e. put it into the NextException member of the SQLContext).
    */
    OOO_DLLPUBLIC_DBTOOLS
    css::sdbc::SQLException prependErrorInfo(
        const css::sdbc::SQLException& _rChainedException,
        const css::uno::Reference< css::uno::XInterface >& _rxContext,
        const OUString& _rAdditionalError,
        const StandardSQLState _eSQLState = StandardSQLState::ERROR_UNSPECIFIED);

    /** search the parent hierarchy for a data source.
    */
    OOO_DLLPUBLIC_DBTOOLS css::uno::Reference< css::sdbc::XDataSource> findDataSource(const css::uno::Reference< css::uno::XInterface >& _xParent);

    /** determines the value of a boolean data source setting, given by ASCII name

        @param _rxConnection
            the connection belonging to the data source whose setting is to be retrieved
        @param _pAsciiSettingName
            the ASCII name of the setting
    */
    OOO_DLLPUBLIC_DBTOOLS bool getBooleanDataSourceSetting(
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const sal_Char* _pAsciiSettingName
        );

    /** check if a specific property is enabled in the info sequence
        @deprecated
            Use getBooleanDataSourceSetting instead, which cares for the default of the property itself,
            instead of spreading this knowledge through all callers.
    */
    OOO_DLLPUBLIC_DBTOOLS
    bool isDataSourcePropertyEnabled(const css::uno::Reference< css::uno::XInterface>& _xProp,
                                     const OUString& _sProperty,
                                           bool _bDefault);

    /** retrieves a particular indirect data source setting

        @param _rxDataSource
            a data source component
        @param _pAsciiSettingsName
            the ASCII name of the setting to obtain
        @param _rSettingsValue
            the value of the setting, upon successful return

        @return
            <FALSE/> if the setting is not present in the <member scope="css::sdb">DataSource::Info</member>
            member of the data source
            <TRUE/> otherwise
    */
    OOO_DLLPUBLIC_DBTOOLS
    bool    getDataSourceSetting(
        const css::uno::Reference< css::uno::XInterface >& _rxDataSource,
        const sal_Char* _pAsciiSettingsName,
        css::uno::Any& /* [out] */ _rSettingsValue
    );
    OOO_DLLPUBLIC_DBTOOLS
    bool    getDataSourceSetting(
        const css::uno::Reference< css::uno::XInterface >& _rxDataSource,
        const OUString& _sSettingsName,
        css::uno::Any& /* [out] */ _rSettingsValue
    );

    OOO_DLLPUBLIC_DBTOOLS OUString getDefaultReportEngineServiceName(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory);

    /** quote the given name with the given quote string.
    */
    OOO_DLLPUBLIC_DBTOOLS OUString quoteName(const OUString& _rQuote, const OUString& _rName);

    /** quote the given table name (which may contain a catalog and a schema) according to the rules provided by the meta data
    */
    OOO_DLLPUBLIC_DBTOOLS
    OUString quoteTableName(const css::uno::Reference< css::sdbc::XDatabaseMetaData>& _rxMeta
                                    , const OUString& _rName
                                    ,EComposeRule _eComposeRule);

    /** split a fully qualified table name (including catalog and schema, if applicable) into its component parts.
        @param  _rxConnMetaData     meta data describing the connection where you got the table name from
        @param  _rQualifiedName     fully qualified table name
        @param  _rCatalog           (out parameter) upon return, contains the catalog name
        @param  _rSchema            (out parameter) upon return, contains the schema name
        @param  _rName              (out parameter) upon return, contains the table name
        @param  _eComposeRule       where do you need the name for
    */
    OOO_DLLPUBLIC_DBTOOLS void qualifiedNameComponents(const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _rxConnMetaData,
        const OUString& _rQualifiedName, OUString& _rCatalog, OUString& _rSchema, OUString& _rName,EComposeRule _eComposeRule);

    /** calculate a NumberFormatsSupplier for use with an given connection
        @param      _rxConn         the connection for which the formatter is requested
        @param      _bAllowDefault  if the connection (and related components, such as its parent) cannot supply
                                    a formatter, we can ask the DatabaseEnvironment for a default one. This parameter
                                    states if this is allowed.
        @param      _rxFactory      required (only of _bAllowDefault is sal_True) for creating the DatabaseEnvironment.
        @return     the formatter all object related to the given connection should work with.
    */
    OOO_DLLPUBLIC_DBTOOLS css::uno::Reference< css::util::XNumberFormatsSupplier> getNumberFormats(
        const css::uno::Reference< css::sdbc::XConnection>& _rxConn,
        bool _bAllowDefault = false,
        const css::uno::Reference< css::uno::XComponentContext>& _rxContext = css::uno::Reference< css::uno::XComponentContext>()
    );

    /** create an css::sdb::XSingleSelectQueryComposer which represents
        the current settings (Command/CommandType/Filter/Order) of the given rowset.

        As such an instance can be obtained from a css::sdb::Connection
        only the function searches for the connection the RowSet is using via connectRowset.
        This implies that a connection will be set on the RowSet if needed.
        (need to changes this sometimes ...)
    */
    OOO_DLLPUBLIC_DBTOOLS css::uno::Reference< css::sdb::XSingleSelectQueryComposer > getCurrentSettingsComposer(
        const css::uno::Reference< css::beans::XPropertySet>& _rxRowSetProps,
        const css::uno::Reference< css::uno::XComponentContext>& _rxContext
    );

    /** transfer and translate properties between two FormComponents
        @param      _rxOld      the source property set
        @param      _rxNew      the destination property set
        @param      _rLocale    the locale for converting number related properties
    */
    OOO_DLLPUBLIC_DBTOOLS void TransferFormComponentProperties(
        const css::uno::Reference< css::beans::XPropertySet>& _rxOld,
        const css::uno::Reference< css::beans::XPropertySet>& _rxNew,
        const css::lang::Locale& _rLocale
        );

    /** check if the property "Privileges" supports css::sdbcx::Privilege::INSERT
        @param      _rxCursorSet    the property set
    */
    OOO_DLLPUBLIC_DBTOOLS bool canInsert(const css::uno::Reference< css::beans::XPropertySet>& _rxCursorSet);
    /** check if the property "Privileges" supports css::sdbcx::Privilege::UPDATE
        @param      _rxCursorSet    the property set
    */
    OOO_DLLPUBLIC_DBTOOLS bool canUpdate(const css::uno::Reference< css::beans::XPropertySet>& _rxCursorSet);
    /** check if the property "Privileges" supports css::sdbcx::Privilege::DELETE
        @param      _rxCursorSet    the property set
    */
    OOO_DLLPUBLIC_DBTOOLS bool canDelete(const css::uno::Reference< css::beans::XPropertySet>& _rxCursorSet);


    /** compose a complete table name from its up to three parts, regarding to the database meta data composing rules
    */
    OOO_DLLPUBLIC_DBTOOLS OUString composeTableName( const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _rxMetaData,
                            const OUString& _rCatalog,
                            const OUString& _rSchema,
                            const OUString& _rName,
                            bool _bQuote,
                            EComposeRule _eComposeRule);

    /** composes a table name for usage in a SELECT statement

        This includes quoting of the table as indicated by the connection's meta data, plus respecting
        the settings "UseCatalogInSelect" and "UseSchemaInSelect", which might be present
        in the data source which the connection belongs to.
    */
    OOO_DLLPUBLIC_DBTOOLS OUString composeTableNameForSelect(
                            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                            const OUString& _rCatalog,
                            const OUString& _rSchema,
                            const OUString& _rName );

    /** composes a table name for usage in a SELECT statement

        This includes quoting of the table as indicated by the connection's meta data, plus respecting
        the settings "UseCatalogInSelect" and "UseSchemaInSelect", which might be present
        in the data source which the connection belongs to.
    */
    OOO_DLLPUBLIC_DBTOOLS OUString composeTableNameForSelect(
                            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                            const css::uno::Reference< css::beans::XPropertySet>& _xTable );

    /** compose the table name out of the property set which must support the properties from the service <member scope= "css::sdbcx">table</member>
        @param  _xMetaData
            The metadata from the connection.
        @param  _xTable
            The table.
    */
    OOO_DLLPUBLIC_DBTOOLS OUString composeTableName(
        const css::uno::Reference< css::sdbc::XDatabaseMetaData>& _xMetaData,
        const css::uno::Reference< css::beans::XPropertySet>& _xTable,
        EComposeRule _eComposeRule,
        bool _bQuote);


    OOO_DLLPUBLIC_DBTOOLS sal_Int32 getSearchColumnFlag( const css::uno::Reference< css::sdbc::XConnection>& _rxConn,
                                    sal_Int32 _nDataType);
    // return the datasource for the given datasource name
    OOO_DLLPUBLIC_DBTOOLS css::uno::Reference< css::sdbc::XDataSource> getDataSource(const OUString& _rsDataSourceName,
                        const css::uno::Reference< css::uno::XComponentContext>& _rxContext);

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
    OUString createUniqueName(const css::uno::Reference< css::container::XNameAccess>& _rxContainer,
                                     const OUString& _rBaseName,
                                     bool _bStartWithNumber = true);

    /** creates a unique name which is not already used in the given name array
    */
    OOO_DLLPUBLIC_DBTOOLS OUString createUniqueName(
        const css::uno::Sequence< OUString >& _rNames,
        const OUString& _rBaseName,
        bool _bStartWithNumber
    );

    /** create a name which is a valid SQL 92 identifier name
        @param      _rName          the string which should be converted
        @param      _rSpecials      @see com.sun.star.sdbc.XDatabaseMetaData.getExtraNameCharacters

        @see isValidSQLName
    */
    OOO_DLLPUBLIC_DBTOOLS OUString convertName2SQLName(const OUString& _rName,const OUString& _rSpecials);

    /** checks whether the given name is a valid SQL name

        @param      _rName          the string which should be converted
        @param      _rSpecials      @see com.sun.star.sdbc.XDatabaseMetaData.getExtraNameCharacters

        @see convertName2SQLName
    */
    OOO_DLLPUBLIC_DBTOOLS bool isValidSQLName( const OUString& _rName, const OUString& _rSpecials );

    OOO_DLLPUBLIC_DBTOOLS
    void showError( const SQLExceptionInfo& _rInfo,
                    const css::uno::Reference< css::awt::XWindow>& _pParent,
                    const css::uno::Reference< css::uno::XComponentContext>& _rxContext);

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
    bool implUpdateObject(  const css::uno::Reference< css::sdbc::XRowUpdate >& _rxUpdatedObject,
                                const sal_Int32 _nColumnIndex,
                                const css::uno::Any& _rValue);


    /** ask the user for parameters if the prepared statement needs some and sets them in the prepared statement
        @param _xConnection     the connection must be able to create css::sdb::SingleSelectQueryComposers
        @param _xPreparedStmt   the prepared statement where the parameters could be set when needed
        @param _aParametersSet  contains which parameters have to asked for and which already have set.
    */
    OOO_DLLPUBLIC_DBTOOLS
    void askForParameters(  const css::uno::Reference< css::sdb::XSingleSelectQueryComposer >& _xComposer,
                            const css::uno::Reference< css::sdbc::XParameters>& _xParameters,
                            const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                            const css::uno::Reference< css::task::XInteractionHandler >& _rxHandler,
                            const ::std::vector<bool, std::allocator<bool> >& _aParametersSet = ::std::vector<bool, std::allocator<bool> >());

    /** call the appropriate set method for the specific sql type @see css::sdbc::DataType
        @param  _xParams        the parameters where to set the value
        @param  parameterIndex  the index of the parameter, 1 based
        @param  x               the value to set
        @param  sqlType         the corresponding sql type @see css::sdbc::DataType
        @param  scale           the scale of the sql type can be 0
        @throws css::sdbc::SQLException
        @throws css::uno::RuntimeException
    */
    OOO_DLLPUBLIC_DBTOOLS
    void setObjectWithInfo( const css::uno::Reference< css::sdbc::XParameters>& _xParameters,
                            sal_Int32 parameterIndex,
                            const css::uno::Any& x,
                            sal_Int32 sqlType,
                            sal_Int32 scale=0);

    /** call the appropriate set method for the specific sql type @see css::sdbc::DataType
        @param  _xParams        the parameters where to set the value
        @param  parameterIndex  the index of the parameter, 1 based
        @param  x               the value to set
        @param  sqlType         the corresponding sql type @see css::sdbc::DataType
        @param  scale           the scale of the sql type can be 0
        @throws css::sdbc::SQLException
        @throws css::uno::RuntimeException
    */
    OOO_DLLPUBLIC_DBTOOLS
    void setObjectWithInfo( const css::uno::Reference< css::sdbc::XParameters>& _xParameters,
                            sal_Int32 parameterIndex,
                            const ::connectivity::ORowSetValue& x,
                            sal_Int32 sqlType,
                            sal_Int32 scale);


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
    bool implSetObject( const css::uno::Reference< css::sdbc::XParameters>& _rxParameters,
                            const sal_Int32 _nColumnIndex,
                            const css::uno::Any& _rValue);

    /** creates the standard sql create table statement without the key part.
        @param  descriptor
            The descriptor of the new table.
        @param  _xConnection
            The connection.
        @param  _bAddScale
            The scale will also be added when the value is 0.
    */
    OOO_DLLPUBLIC_DBTOOLS
    OUString createStandardCreateStatement(  const css::uno::Reference< css::beans::XPropertySet >& descriptor,
                                                    const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                                                    ISQLStatementHelper* _pHelper,
                                                    const OUString& _sCreatePattern);

    /** creates the standard sql statement for the key part of a create table statement.
        @param  descriptor
            The descriptor of the new table.
        @param  _xConnection
            The connection.
    */
    OOO_DLLPUBLIC_DBTOOLS
    OUString createStandardKeyStatement( const css::uno::Reference< css::beans::XPropertySet >& descriptor,
                                                const css::uno::Reference< css::sdbc::XConnection>& _xConnection);

    /** creates the standard sql statement for the type part of a create or alter table statement.
        @param  _pHelper
            Allow to add special SQL constructs.
        @param  descriptor
            The descriptor of the column.
        @param  _xConnection
            The connection.
    */
    OOO_DLLPUBLIC_DBTOOLS
    OUString createStandardTypePart(     const css::uno::Reference< css::beans::XPropertySet >& descriptor
                                                ,const css::uno::Reference< css::sdbc::XConnection>& _xConnection
                                                ,const OUString& _sCreatePattern = OUString());

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
    OUString createStandardColumnPart(   const css::uno::Reference< css::beans::XPropertySet >& descriptor
                                                ,const css::uno::Reference< css::sdbc::XConnection>& _xConnection
                                                ,ISQLStatementHelper* _pHelper = nullptr
                                                ,const OUString& _sCreatePattern = OUString());

    /** creates a SQL CREATE TABLE statement

        @param  descriptor
            The descriptor of the new table.
        @param  _xConnection
            The connection.

        @return
            The CREATE TABLE statement.
    */
    OOO_DLLPUBLIC_DBTOOLS
    OUString createSqlCreateTableStatement(  const css::uno::Reference< css::beans::XPropertySet >& descriptor
                                                    ,const css::uno::Reference< css::sdbc::XConnection>& _xConnection);

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
    css::uno::Reference< css::beans::XPropertySet>
            createSDBCXColumn(  const css::uno::Reference< css::beans::XPropertySet>& _xTable,
                                const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                                const OUString& _rName,
                                bool _bCase,
                                bool _bQueryForInfo,
                                bool _bIsAutoIncrement,
                                bool _bIsCurrency,
                                sal_Int32 _nDataType);

    /** tries to locate the corresponding DataDefinitionSupplier for the given url and connection
        @param  _rsUrl
            The URL used to connect to the database.
        @param  _xConnection
            The connection used to find the correct driver.
        @param  _rxContext
            Used to create the drivermanager.
        @return
            The datadefinition object.
    */
    OOO_DLLPUBLIC_DBTOOLS css::uno::Reference< css::sdbcx::XTablesSupplier> getDataDefinitionByURLAndConnection(
            const OUString& _rsUrl,
            const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
            const css::uno::Reference< css::uno::XComponentContext>& _rxContext);

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
    sal_Int32 getTablePrivileges(const css::uno::Reference< css::sdbc::XDatabaseMetaData>& _xMetaData,
                                 const OUString& _sCatalog,
                                 const OUString& _sSchema,
                                 const OUString& _sTable);

    typedef ::std::pair<bool,bool> TBoolPair;
    typedef ::std::pair< TBoolPair,sal_Int32 > ColumnInformation;
    typedef ::std::multimap< OUString, ColumnInformation, ::comphelper::UStringMixLess> ColumnInformationMap;
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
    void collectColumnInformation(  const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                                    const OUString& _sComposedTableName,
                                    const OUString& _rName,
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
    OOO_DLLPUBLIC_DBTOOLS void getBooleanComparisonPredicate(
            const OUString& _rExpression,
            const bool  _bValue,
            const sal_Int32 _nBooleanComparisonMode,
            OUStringBuffer& _out_rSQLPredicate
        );

    /** is this field an aggregate?

        @param _xComposer
            a query composer that knows the field by name
        @param _xField
            the field
    */
    OOO_DLLPUBLIC_DBTOOLS bool isAggregateColumn(
            const css::uno::Reference< css::sdb::XSingleSelectQueryComposer > &_xComposer,
            const css::uno::Reference< css::beans::XPropertySet > &_xField
        );

    /** is this column an aggregate?

        @param _xColumns collection of columns
            look for column sName in there
        @param _sName
            name of the column
    */
    OOO_DLLPUBLIC_DBTOOLS bool isAggregateColumn(
            const css::uno::Reference< css::container::XNameAccess > &_xColumns,
            const OUString &_sName
        );

    /** is this column an aggregate?

        @param _xColumn
    */
    OOO_DLLPUBLIC_DBTOOLS bool isAggregateColumn(
            const css::uno::Reference< css::beans::XPropertySet > &_xColumn
        );

}   // namespace dbtools

namespace connectivity
{
namespace dbase
{
    enum DBFType  { dBaseIII         = 0x03,
                    dBaseIV          = 0x04,
                    dBaseV           = 0x05,
                    VisualFoxPro     = 0x30,
                    VisualFoxProAuto = 0x31, // Visual FoxPro with AutoIncrement field
                    dBaseFS          = 0x43,
                    dBaseFSMemo      = 0xB3,
                    dBaseIIIMemo     = 0x83,
                    dBaseIVMemo      = 0x8B,
                    dBaseIVMemoSQL   = 0x8E,
                    FoxProMemo       = 0xF5
    };

    /** decode a DBase file's codepage byte to a RTL charset
        @param  _out_nCharset
            in case of success, the decoded RTL charset is written there.
            else, this is not written to.
        @param  nType
            the file's type byte
        @param  nCodepage
            the file's codepage byte
        @return
            true if a RTL charset was successfully decoded and written to _out_nCharset
            false if nothing was written to _out_nCharset
    */
    OOO_DLLPUBLIC_DBTOOLS bool dbfDecodeCharset(rtl_TextEncoding &_out_nCharset, sal_uInt8 nType, sal_uInt8 nCodepage);

    /** decode a DBase file's codepage byte to a RTL charset
        @param  _out_nCharset
            in case of success, the decoded RTL charset is written there.
            else, this is not written to.
        @param  dbf_Stream
            pointer to a SvStream encapsulating the DBase file.
            The stream will be rewinded and read from.
            No guarantee is made on its position afterwards. Caller must reposition it itself.
        @return
            true if a RTL charset was successfully decoded and written to _out_nCharset
            false if nothing was written to _out_nCharset
    */
    OOO_DLLPUBLIC_DBTOOLS bool dbfReadCharset(rtl_TextEncoding &nCharSet, SvStream* dbf_Stream);

} // namespace connectivity::dbase
} // namespace connectivity

#endif // INCLUDED_CONNECTIVITY_DBTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
