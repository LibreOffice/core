/*************************************************************************
 *
 *  $RCSfile: dbtools.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-05 08:56:29 $
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

#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLERROREVENT_HPP_
#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

namespace com { namespace sun { namespace star {

namespace sdb {
    class XSQLQueryComposer;
}

namespace lang {
    struct Locale;
}

namespace util {
    class XNumberFormatTypes;
}

} } }

//.........................................................................
namespace dbtools
{
//.........................................................................

    namespace starutil      = ::com::sun::star::util;
    namespace staruno       = ::com::sun::star::uno;
    namespace starsdb       = ::com::sun::star::sdb;
    namespace starsdbc      = ::com::sun::star::sdbc;
    namespace starcontainer = ::com::sun::star::container;
    namespace starbeans     = ::com::sun::star::beans;
    namespace starlang      = ::com::sun::star::lang;

//=========================================================================
    // date conversion

//  extern starutil::Date STANDARD_DB_DATE;
//  double ToStandardDbDate(const starutil::Date& rNullDate, double rVal);
//  double ToNullDate(const starutil::Date& rNullDate, double rVal);

    // calculates the default numberformat for a given datatype and a give language
    sal_Int32 getDefaultNumberFormat(const staruno::Reference< starbeans::XPropertySet >& _xColumn,
                                     const staruno::Reference< starutil::XNumberFormatTypes >& _xTypes,
                                     const starlang::Locale& _rLocale);

//=========================================================================

    /** calculates the connection the given RowSet works - or should work - with.
        If the set has an active connection (ActiveConnection property), this one is returned.
        Else the parent hierarchy is searched for an object with an XConnection interface. If found, this
        one is returned.
        If we still haven't a connection, a new one is calculated from the current RowSet settings (such as
        DataSource, URL, User, Password) and returned.
        In any of these cases the calculated connection is <b>forwarded</b> to the RowSet, that means before
        returning from the function the connection is set as ActiveConnection property on the RowSet !
    */
    staruno::Reference<starsdbc::XConnection> calcConnection(
        const staruno::Reference<starsdbc::XRowSet>& _rxRowSet,
        const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
            throw (starsdbc::SQLException, staruno::RuntimeException);

    /** returns the connection the RowSet is currently working with (which is the ActiveConnection property)
    */
    staruno::Reference<starsdbc::XConnection> getConnection(const staruno::Reference<starsdbc::XRowSet>& _rxRowSet) throw (staruno::RuntimeException);

    /** returns the columns of the named table of the given connection
    */
    staruno::Reference<starcontainer::XNameAccess> getTableFields(const staruno::Reference<starsdbc::XConnection>& _rxConn, const ::rtl::OUString& _rName);

    /** create a new ::com::sun::star::sdbc::SQLContext, fill it with the given descriptions and the given source,
        and <i>append</i> _rException (i.e. put it into the NextException member of the SQLContext).
    */
    starsdb::SQLContext prependContextInfo(starsdbc::SQLException& _rException, const staruno::Reference< staruno::XInterface >& _rxContext, const ::rtl::OUString& _rContextDescription, const ::rtl::OUString& _rContextDetails = ::rtl::OUString());

    /** quote the given name with the given quote string.
    */
    ::rtl::OUString quoteName(const ::rtl::OUString& _rQuote, const ::rtl::OUString& _rName);

    /** quote the given table name (which may contain a catalog and a schema) according to the rules provided by the meta data
    */
    ::rtl::OUString quoteTableName(const staruno::Reference<starsdbc::XDatabaseMetaData>& _rxMeta, const ::rtl::OUString& _rName);

    /** split a fully qualified table name (including catalog and schema, if appliable) into it's component parts.
        @param  _rxConnMetaData     meta data describing the connection where you got the table name from
        @param  _rQualifiedName     fully qualified table name
        @param  _rCatalog           (out parameter) upon return, contains the catalog name
        @param  _rSchema            (out parameter) upon return, contains the schema name
        @param  _rName              (out parameter) upon return, contains the table name
    */
    void qualifiedNameComponents(const staruno::Reference< starsdbc::XDatabaseMetaData >& _rxConnMetaData,
        const ::rtl::OUString& _rQualifiedName, ::rtl::OUString& _rCatalog, ::rtl::OUString& _rSchema, ::rtl::OUString& _rName);

    /** calculate a NumberFormatsSupplier for use with an given connection
        @param      _rxConn         the connection for which the formatter is requested
        @param      _bAllowDefault  if the connection (and related components, such as it's parent) cannot supply
                                    a formatter, we can ask the DatabaseEnvironment for a default one. This parameter
                                    states if this is allowed.
        @param      _rxFactory      required (only of _bAllowDefault is sal_True) for creating the DatabaseEnvironment.
        @return     the formatter all object related to the given connection should work with.
    */
    staruno::Reference<starutil::XNumberFormatsSupplier> getNumberFormats(
        const staruno::Reference<starsdbc::XConnection>& _rxConn,
        sal_Bool _bAllowDefault = sal_False,
        const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory = staruno::Reference<starlang::XMultiServiceFactory>()
    );

    /** create an XSQLQueryComposer which represents the current settings (Command/CommandType/Filter/Order)
        of the given rowset.
        As a XSQLQueryComposerFactory is needed for that, the functions searches for the connection the RowSet
        is using via calcConnection. This implies that a connection will be set on the RowSet if needed.
        (need to changes this sometimes ...)
    */
    staruno::Reference<starsdb::XSQLQueryComposer> getCurrentSettingsComposer(
                    const staruno::Reference<starbeans::XPropertySet>& _rxRowSetProps,
                    const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);

    /** transfer and translate properties between two FormComponents
        @param      _rxOld      the source property set
        @param      _rxNew      the destination property set
        @param      _rLocale    the locale for converting number related properties
    */
    void TransferFormComponentProperties(
        const staruno::Reference<starbeans::XPropertySet>& _rxOld,
        const staruno::Reference<starbeans::XPropertySet>& _rxNew,
        const starlang::Locale& _rLocale
        );

    /** check if the property "Privileges" supports starsdbcx::Privilege::INSERT
        @param      _rxCursorSet    the property set
    */
    sal_Bool canInsert(const staruno::Reference< starbeans::XPropertySet>& _rxCursorSet);
    /** check if the property "Privileges" supports starsdbcx::Privilege::UPDATE
        @param      _rxCursorSet    the property set
    */
    sal_Bool canUpdate(const staruno::Reference< starbeans::XPropertySet>& _rxCursorSet);
    /** check if the property "Privileges" supports starsdbcx::Privilege::DELETE
        @param      _rxCursorSet    the property set
    */
    sal_Bool canDelete(const staruno::Reference< starbeans::XPropertySet>& _rxCursorSet);

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _CONNECTIVITY_DBTOOLS_HXX_

