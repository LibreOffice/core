/*************************************************************************
 *
 *  $RCSfile: dbtools.hxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:49:27 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
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

#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#define _CONNECTIVITY_DBTOOLS_HXX_

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

namespace com { namespace sun { namespace star {

namespace sdb {
    class XSQLQueryComposer;
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
}
namespace container {
    class XNameAccess;
}

namespace util {
    class XNumberFormatTypes;
    class XNumberFormatsSupplier;
}
namespace task {
    class XInteractionHandler;
}

} } }

//.........................................................................
namespace dbtools
{
    enum EComposeRule
    {
        eInTableDefinitions,
        eInIndexDefinitions,
        eInDataManipulation,
        eInProcedureCalls,
        eInPrivilegeDefinitions
    };
//=========================================================================
    // date conversion

//  extern ::com::sun::star::util::Date STANDARD_DB_DATE;
//  double ToStandardDbDate(const ::com::sun::star::util::Date& rNullDate, double rVal);
//  double ToNullDate(const ::com::sun::star::util::Date& rNullDate, double rVal);

    // calculates the default numberformat for a given datatype and a give language
    sal_Int32 getDefaultNumberFormat(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xColumn,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatTypes >& _xTypes,
                                     const ::com::sun::star::lang::Locale& _rLocale);

    // calculates the default numberformat for a given datatype and a give language
    // @param  _nDataType @see com.sun.star.sdbc.DataType
    // @param _nScale       can be zero
    sal_Int32 getDefaultNumberFormat(sal_Int32 _nDataType,
                                     sal_Int32 _nScale,
                                     sal_Bool _bIsCurrency,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatTypes >& _xTypes,
                                     const ::com::sun::star::lang::Locale& _rLocale);

//=========================================================================

    /** calculates the connection the given RowSet works - or should work - with.
        <p>If the set has an active connection (ActiveConnection property), this one is returned.
        Else the parent hierarchy is searched for an object with an XConnection interface. If found, this
        one is returned.</p>
        <p>If we still haven't a connection, a new one is calculated from the current RowSet settings (such as
        DataSource, URL, User, Password) and returned.</p>
        <p>In any of these cases the calculated connection is <b>forwarded</b> to the RowSet, that means before
        returning from the function the connection is set as ActiveConnection property on the RowSet !<p>
        <p>This function  is deprecated, please use connectRowset.</p>
    @deprecated
    @see connectRowset
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> calcConnection(
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory)
            throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    /** creates a connection which can be used for the rowset given
        <p>If the rowset already has an ActiveConnection (means a value vor this property), this connection is returned.</p>
        <p>The connection is calculated from the settings in the row set (data source name, URL, user, pwd).</p>
        @param _rxRowSet
            the row set
        @param _rxFactory
            a service factory, which can be used to create data sources, interaction handler etc (the usual stuff)
        @param _bSetAsActiveConnection
            If <TRUE/>, the calculated connection is set as ActiveConnection property on the rowset.
            In this case, the method behaves exactly like calcConnection.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> connectRowset(
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
        sal_Bool _bSetAsActiveConnection
    )   SAL_THROW ( (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) );

    /** returns the connection the RowSet is currently working with (which is the ActiveConnection property)
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection(
            const ::rtl::OUString& _rsTitleOrPath,
            const ::rtl::OUString& _rsUser,
            const ::rtl::OUString& _rsPwd,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection_withFeedback(
            const ::rtl::OUString& _rDataSourceName,
            const ::rtl::OUString& _rUser,
            const ::rtl::OUString& _rPwd,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory)
        SAL_THROW ( (::com::sun::star::sdbc::SQLException) );


    /** return the active connection which is got from the first model which occurs and has the active connection set as argument.
        @param  _xChild
            The child to ask for the XModel interface, if not supported it goes the parent hierachy up.

        @return ::com::sun::star::uno::Reference<::com::sun::star::sdbc::XConnection>
        @see XModel::getArgs
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>  getActiveConnectionFromParent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xChild);

    /** returns the columns of the named table of the given connection
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getTableFields(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn, const ::rtl::OUString& _rName);

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
    ::com::sun::star::uno::Sequence< ::rtl::OUString >
        getFieldNamesByCommandDescriptor(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const sal_Int32 _nCommandType,
            const ::rtl::OUString& _rCommand,
            SQLExceptionInfo* _pErrorInfo = NULL
        )   SAL_THROW( ( ) );


    /** create a new ::com::sun::star::sdbc::SQLContext, fill it with the given descriptions and the given source,
        and <i>append</i> _rException (i.e. put it into the NextException member of the SQLContext).
    */
    ::com::sun::star::sdb::SQLContext prependContextInfo(const ::com::sun::star::sdbc::SQLException& _rException, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext, const ::rtl::OUString& _rContextDescription, const ::rtl::OUString& _rContextDetails = ::rtl::OUString());

    /** search the parent hierachy for a data source.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> findDataSource(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xParent);

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
    sal_Bool isDataSourcePropertyEnabled(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xProp
                                        ,const ::rtl::OUString& _sProperty,
                                        sal_Bool _bDefault = sal_False);

    /** quote the given name with the given quote string.
    */
    ::rtl::OUString quoteName(const ::rtl::OUString& _rQuote, const ::rtl::OUString& _rName);

    /** quote the given table name (which may contain a catalog and a schema) according to the rules provided by the meta data
    */
    ::rtl::OUString quoteTableName(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _rxMeta
                                    , const ::rtl::OUString& _rName
                                    ,EComposeRule _eComposeRule
                                    ,sal_Bool _bUseCatalogInSelect = sal_True
                                    ,sal_Bool _bUseSchemaInSelect = sal_True);

    /** split a fully qualified table name (including catalog and schema, if appliable) into it's component parts.
        @param  _rxConnMetaData     meta data describing the connection where you got the table name from
        @param  _rQualifiedName     fully qualified table name
        @param  _rCatalog           (out parameter) upon return, contains the catalog name
        @param  _rSchema            (out parameter) upon return, contains the schema name
        @param  _rName              (out parameter) upon return, contains the table name
        @param  _eComposeRule       where do you need the name for
    */
    void qualifiedNameComponents(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxConnMetaData,
        const ::rtl::OUString& _rQualifiedName, ::rtl::OUString& _rCatalog, ::rtl::OUString& _rSchema, ::rtl::OUString& _rName,EComposeRule _eComposeRule);

    /** calculate a NumberFormatsSupplier for use with an given connection
        @param      _rxConn         the connection for which the formatter is requested
        @param      _bAllowDefault  if the connection (and related components, such as it's parent) cannot supply
                                    a formatter, we can ask the DatabaseEnvironment for a default one. This parameter
                                    states if this is allowed.
        @param      _rxFactory      required (only of _bAllowDefault is sal_True) for creating the DatabaseEnvironment.
        @return     the formatter all object related to the given connection should work with.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier> getNumberFormats(
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn,
        sal_Bool _bAllowDefault = sal_False,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>()
    );

    /** returns the statement which is composed from the current settings of a row set

        If the row set is currently not connected, it is attempted to do so with it's current settings.
        (TODO: isn't this a resource leak? Do we really need this behaviour?).

        @param _rxRowSet
            the row set whose settings should be examined. Must not be <NULL/>
        @param _rxFactory
            a service factory which may be needed to connect the row set
        @param _bUseRowSetFilter
            If <TRUE/>, the <b>Filter</b> property of the row set will also be examined. In case it is
            not empty, and <b>ApplyFilter</b> is <TRUE/>, it will also be added to the composed statement.
        @param _bUseRowSetOrder
            If <TRUE/>, the <b>Order</b> property of the row set will also be examined. In case it is
            not empty, it will also be added to the composed statement.
    */
    ::rtl::OUString getComposedRowSetStatement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
            sal_Bool _bUseRowSetFilter = sal_True,
            sal_Bool _bUseRowSetOrder = sal_True
    )   SAL_THROW( ( ::com::sun::star::sdbc::SQLException ) );

    /** create an XSQLQueryComposer which represents the current settings (Command/CommandType/Filter/Order)
        of the given rowset.
        As a XSQLQueryComposerFactory is needed for that, the functions searches for the connection the RowSet
        is using via calcConnection. This implies that a connection will be set on the RowSet if needed.
        (need to changes this sometimes ...)
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer> getCurrentSettingsComposer(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxRowSetProps,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    /** transfer and translate properties between two FormComponents
        @param      _rxOld      the source property set
        @param      _rxNew      the destination property set
        @param      _rLocale    the locale for converting number related properties
    */
    void TransferFormComponentProperties(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxOld,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxNew,
        const ::com::sun::star::lang::Locale& _rLocale
        );

    /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::INSERT
        @param      _rxCursorSet    the property set
    */
    sal_Bool canInsert(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet);
    /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::UPDATE
        @param      _rxCursorSet    the property set
    */
    sal_Bool canUpdate(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet);
    /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::DELETE
        @param      _rxCursorSet    the property set
    */
    sal_Bool canDelete(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet);

    //----------------------------------------------------------------------------------
    /** compose a complete table name from it's up to three parts, regarding to the database meta data composing rules
    */
    void composeTableName(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxMetaData,
                            const ::rtl::OUString& _rCatalog,
                            const ::rtl::OUString& _rSchema,
                            const ::rtl::OUString& _rName,
                            ::rtl::OUString& _rComposedName,
                            sal_Bool _bQuote,
                            EComposeRule _eComposeRule
                            ,sal_Bool _bUseCatalogInSelect = sal_True
                            ,sal_Bool _bUseSchemaInSelect = sal_True);

    //----------------------------------------------------------------------------------
    /** compose the table name out of the property set which must support the properties from the service <member scope= "com::sun::star::sdbcx">table</member>
        @param  _xMetaData
            The metadata from the connection.
        @param  _xTable
            The table.
    */
    ::rtl::OUString composeTableName(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xTable,
                                     sal_Bool _bQuote,
                                     EComposeRule _eComposeRule
                                     ,sal_Bool _bUseCatalogInSelect = sal_True
                                     ,sal_Bool _bUseSchemaInSelect = sal_True);

    //----------------------------------------------------------------------------------
    sal_Int32 getSearchColumnFlag( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn,
                                    sal_Int32 _nDataType);
    // return the datasource for the given datasource name
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> getDataSource(const ::rtl::OUString& _rsDataSourceName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

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
    ::rtl::OUString createUniqueName(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxContainer,
                                     const ::rtl::OUString& _rBaseName,
                                     sal_Bool _bStartWithNumber = sal_True);

    /** create a name which is a valid SQL 92 identifier name
        @param      _rName          the string which should be converted
        @param      _rSpecials      @see com.sun.star.sdbc.XDatabaseMetaData.getExtraNameCharacters
    */
    ::rtl::OUString convertName2SQLName(const ::rtl::OUString& _rName,const ::rtl::OUString& _rSpecials);

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
    sal_Bool implUpdateObject(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowUpdate >& _rxUpdatedObject,
                                const sal_Int32 _nColumnIndex,
                                const ::com::sun::star::uno::Any& _rValue)  SAL_THROW   (   (   ::com::sun::star::sdbc::SQLException,   ::com::sun::star::uno::RuntimeException)    );



    /** ask the user for parameters if the prepared statement needs some and sets them in the prepared statement
        @param _xConnection     the connection must support the iterface @see com::sun::star::sdb::XSQLQueryComposerFactory
        @param _xPreparedStmt   the prepared statement where the parameters could be set when needed
    */
    void askForParameters(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer> & _xComposer,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XParameters>& _xParameters,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxHandler);

    /** call the appropiate set method for the specific sql type @see com::sun::star::sdbc::DataType
        @param  _xParams        the parameters where to set the value
        @param  parameterIndex  the index of the parameter, 1 based
        @param  x               the value to set
        @param  sqlType         the corresponding sql type @see com::sun::star::sdbc::DataType
        @param  scale           the scale of the sql type can be 0
    */
    void setObjectWithInfo( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XParameters>& _xParameters,
                            sal_Int32 parameterIndex,
                            const ::com::sun::star::uno::Any& x,
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
    sal_Bool implSetObject( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XParameters>& _rxParameters,
                            const sal_Int32 _nColumnIndex,
                            const ::com::sun::star::uno::Any& _rValue) SAL_THROW ( ( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) );

    /** creates the standard sql create table statement without the key part.
        @param  descriptor
            The descriptor of the new table.
        @param  _xConnection
            The connection.
    */
    ::rtl::OUString createStandardCreateStatement(  const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

    /** creates the standard sql statement for the key part of a create table statement.
        @param  descriptor
            The descriptor of the new table.
        @param  _xConnection
            The connection.
    */
    ::rtl::OUString createStandardKeyStatement( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor,
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

    /** creates the standard sql statement for the column part of a create table statement.
        @param  descriptor
            The descriptor of the column.
        @param  _xConnection
            The connection.
    */
    ::rtl::OUString createStandardColumnPart(   const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor,
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

    /** creates a SQL CREATE TABLE statement
        @param  descriptor
            The descriptor of the new table.
        @param  _xConnection
            The connection.
        @return
            The CREATE TABLE statement.
    */
    ::rtl::OUString createSqlCreateTableStatement(  const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

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
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier> getDataDefinitionByURLAndConnection(
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
    sal_Int32 getTablePrivileges(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData,
                                 const ::rtl::OUString& _sCatalog,
                                 const ::rtl::OUString& _sSchema,
                                 const ::rtl::OUString& _sTable);

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
    typedef ::std::pair<sal_Bool,sal_Bool> TBoolPair;
    typedef ::std::pair< TBoolPair,sal_Int32 > ColumnInformation;
    typedef ::std::multimap< ::rtl::OUString, ColumnInformation, ::comphelper::UStringMixLess> ColumnInformationMap;
    void collectColumnInformation(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                                    const ::rtl::OUString& _sComposedTableName,
                                    const ::rtl::OUString& _rName,
                                    ColumnInformationMap& _rInfo);
//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _CONNECTIVITY_DBTOOLS_HXX_

