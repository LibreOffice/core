/*************************************************************************
 *
 *  $RCSfile: dbtools.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: fs $ $Date: 2001-08-06 14:46:37 $
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

//#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
//#include <com/sun/star/sdbc/XConnection.hpp>
//#endif
//#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
//#include <com/sun/star/sdbc/XRowSet.hpp>
//#endif
//#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
//#include <com/sun/star/sdb/SQLContext.hpp>
//#endif
//#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
//#include <com/sun/star/lang/XMultiServiceFactory.hpp>
//#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

namespace com { namespace sun { namespace star {

namespace sdb {
    class XSQLQueryComposer;
    class SQLContext;
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
//.........................................................................
    class SQLExceptionInfo;
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

    /** returns the columns of the named table of the given connection
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getTableFields(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn, const ::rtl::OUString& _rName);

    /** create a new ::com::sun::star::sdbc::SQLContext, fill it with the given descriptions and the given source,
        and <i>append</i> _rException (i.e. put it into the NextException member of the SQLContext).
    */
    ::com::sun::star::sdb::SQLContext prependContextInfo(::com::sun::star::sdbc::SQLException& _rException, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext, const ::rtl::OUString& _rContextDescription, const ::rtl::OUString& _rContextDetails = ::rtl::OUString());

    /** quote the given name with the given quote string.
    */
    ::rtl::OUString quoteName(const ::rtl::OUString& _rQuote, const ::rtl::OUString& _rName);

    /** quote the given table name (which may contain a catalog and a schema) according to the rules provided by the meta data
    */
    ::rtl::OUString quoteTableName(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _rxMeta, const ::rtl::OUString& _rName);

    /** split a fully qualified table name (including catalog and schema, if appliable) into it's component parts.
        @param  _rxConnMetaData     meta data describing the connection where you got the table name from
        @param  _rQualifiedName     fully qualified table name
        @param  _rCatalog           (out parameter) upon return, contains the catalog name
        @param  _rSchema            (out parameter) upon return, contains the schema name
        @param  _rName              (out parameter) upon return, contains the table name
    */
    void qualifiedNameComponents(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxConnMetaData,
        const ::rtl::OUString& _rQualifiedName, ::rtl::OUString& _rCatalog, ::rtl::OUString& _rSchema, ::rtl::OUString& _rName);

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
                            sal_Bool _bQuote);

    sal_Int32 getSearchColumnFlag( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn,
                                    sal_Int32 _nDataType);
    // return the datasource for the given datasource name
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> getDataSource(const ::rtl::OUString& _rsDataSourceName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    /** search for a name that is NOT in the NameAcces
        @param      _rxContainer    the NameAcces container to search in
        @param      _rBaseName      the base name that should be created
    */
    ::rtl::OUString createUniqueName(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxContainer,
                                     const ::rtl::OUString& _rBaseName);

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

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _CONNECTIVITY_DBTOOLS_HXX_

