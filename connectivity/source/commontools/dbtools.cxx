/*************************************************************************
 *
 *  $RCSfile: dbtools.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-24 15:00:32 $
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

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_DATETIME_HXX_
#include <comphelper/datetime.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XDATABASEENVIRONMENT_HPP_
#include <com/sun/star/sdb/XDatabaseEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif


using namespace ::comphelper;
//.........................................................................
namespace dbtools
{
//.........................................................................

//==============================================================================
//==============================================================================
//------------------------------------------------------------------
sal_Int32 getDefaultNumberFormat(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xColumn,
                                 const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatTypes >& _xTypes,
                                 const ::com::sun::star::lang::Locale& _rLocale)
{
    OSL_ENSHURE(_xTypes.is() && _xColumn.is(), "dbtools::getDefaultNumberFormat: invalid arg !");
    if (!_xTypes.is() || !_xColumn.is())
        return ::com::sun::star::util::NumberFormat::UNDEFINED;

    sal_Int32 nDataType;
    try
    {
        // determine the datatype of the column
        _xColumn->getPropertyValue(::rtl::OUString::createFromAscii("Type")) >>= nDataType;
    }
    catch (...)
    {
        return ::com::sun::star::util::NumberFormat::UNDEFINED;
    }

    sal_Int32 nFormat;
    switch (nDataType)
    {
        case ::com::sun::star::sdbc::DataType::BIT:
            nFormat = _xTypes->getStandardFormat(::com::sun::star::util::NumberFormat::LOGICAL, _rLocale);
            break;
        case ::com::sun::star::sdbc::DataType::TINYINT:
        case ::com::sun::star::sdbc::DataType::SMALLINT:
        case ::com::sun::star::sdbc::DataType::INTEGER:
        case ::com::sun::star::sdbc::DataType::BIGINT:
            nFormat = _xTypes->getStandardFormat(::com::sun::star::util::NumberFormat::NUMBER, _rLocale);
            break;
        case ::com::sun::star::sdbc::DataType::FLOAT:
        case ::com::sun::star::sdbc::DataType::REAL:
        case ::com::sun::star::sdbc::DataType::DOUBLE:
        case ::com::sun::star::sdbc::DataType::NUMERIC:
        case ::com::sun::star::sdbc::DataType::DECIMAL:
        {
            try
            {
                if (getBOOL(_xColumn->getPropertyValue(::rtl::OUString::createFromAscii("IsCurrency"))))
                    nFormat = _xTypes->getStandardFormat(::com::sun::star::util::NumberFormat::CURRENCY, _rLocale);
                else
                {
                    sal_Int32 nScale = 2;
                    if (::com::sun::star::sdbc::DataType::NUMERIC == nDataType || ::com::sun::star::sdbc::DataType::DECIMAL == nDataType)
                        _xColumn->getPropertyValue(::rtl::OUString::createFromAscii("Scale")) >>= nScale;

                    // generate a new format if necessary
                    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats > xFormats(_xTypes, ::com::sun::star::uno::UNO_QUERY);
                    ::rtl::OUString sNewFormat = xFormats->generateFormat( 0L, _rLocale, sal_False, sal_False, nScale, sal_True);

                    // and add it to the formatter if necessary
                    nFormat = xFormats->queryKey(sNewFormat, _rLocale, sal_False);
                    if (nFormat == (sal_Int32)-1)
                        nFormat = xFormats->addNew(sNewFormat, _rLocale);
                }
            }
            catch (...)
            {
                nFormat = _xTypes->getStandardFormat(::com::sun::star::util::NumberFormat::NUMBER, _rLocale);
            }
        }   break;
        case ::com::sun::star::sdbc::DataType::CHAR:
        case ::com::sun::star::sdbc::DataType::VARCHAR:
        case ::com::sun::star::sdbc::DataType::LONGVARCHAR:
            nFormat = _xTypes->getStandardFormat(::com::sun::star::util::NumberFormat::TEXT, _rLocale);
            break;
        case ::com::sun::star::sdbc::DataType::DATE:
            nFormat = _xTypes->getStandardFormat(::com::sun::star::util::NumberFormat::DATE, _rLocale);
            break;
        case ::com::sun::star::sdbc::DataType::TIME:
            nFormat = _xTypes->getStandardFormat(::com::sun::star::util::NumberFormat::TIME, _rLocale);
            break;
        case ::com::sun::star::sdbc::DataType::TIMESTAMP:
            nFormat = _xTypes->getStandardFormat(::com::sun::star::util::NumberFormat::DATETIME, _rLocale);
            break;
        case ::com::sun::star::sdbc::DataType::BINARY:
        case ::com::sun::star::sdbc::DataType::VARBINARY:
        case ::com::sun::star::sdbc::DataType::LONGVARBINARY:
        case ::com::sun::star::sdbc::DataType::SQLNULL:
        case ::com::sun::star::sdbc::DataType::OTHER:
        case ::com::sun::star::sdbc::DataType::OBJECT:
        case ::com::sun::star::sdbc::DataType::DISTINCT:
        case ::com::sun::star::sdbc::DataType::STRUCT:
        case ::com::sun::star::sdbc::DataType::ARRAY:
        case ::com::sun::star::sdbc::DataType::BLOB:
        case ::com::sun::star::sdbc::DataType::CLOB:
        case ::com::sun::star::sdbc::DataType::REF:
        default:
            nFormat = ::com::sun::star::util::NumberFormat::UNDEFINED;
    }
    return nFormat;
}

//==============================================================================
//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> findConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xParent)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> xConnection(xParent, ::com::sun::star::uno::UNO_QUERY);
    if (!xConnection.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xChild(xParent, ::com::sun::star::uno::UNO_QUERY);
        if (xChild.is())
            return findConnection(xChild->getParent());
    }
    return xConnection;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> getDataSource(
            const ::rtl::OUString& _rsTitleOrPath,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory)
{
    OSL_ENSHURE(_rsTitleOrPath.getLength(), "::getDataSource : invalid arg !");

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>  xReturn;

    // is it a file url ?
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> xNamingContext(
        _rxFactory->createInstance(
            ::rtl::OUString::createFromAscii("com.sun.star.sdb.DatabaseAccessContext")),
        ::com::sun::star::uno::UNO_QUERY);

    if (xNamingContext.is() && xNamingContext->hasByName(_rsTitleOrPath))
    {
        OSL_ENSHURE(::com::sun::star::uno::Reference< ::com::sun::star::uno::XNamingService>(xNamingContext, ::com::sun::star::uno::UNO_QUERY).is(), "::getDataSource : no NamingService interface on the DatabaseAccessContext !");
        xReturn = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>(
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XNamingService>(xNamingContext, ::com::sun::star::uno::UNO_QUERY)->getRegisteredObject(_rsTitleOrPath),
            ::com::sun::star::uno::UNO_QUERY);
    }
    else
    {   // is it a favorite title ?
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseEnvironment> xEnvironment(
            _rxFactory->createInstance(
                ::rtl::OUString::createFromAscii("com.sun.star.sdb.DatabaseEnvironment")),
            ::com::sun::star::uno::UNO_QUERY);

        if (xEnvironment.is())
        {
            try
            {
                xReturn = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>(xEnvironment->getDatabaseAccess(_rsTitleOrPath), ::com::sun::star::uno::UNO_QUERY);
            }
            catch(::com::sun::star::sdbc::SQLException& e)
            {   // allowed, the env may throw an exception in case of an invalid name
                e; // make compiler happy
            }
        }

    }

    return xReturn;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection(
            const ::rtl::OUString& _rsTitleOrPath,
            const ::rtl::OUString& _rsUser,
            const ::rtl::OUString& _rsPwd,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> xDataSource( getDataSource(_rsTitleOrPath, _rxFactory) );
    if (xDataSource.is())
        return xDataSource->getConnection(_rsUser, _rsPwd);

    return ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>();
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> xReturn;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xRowSetProps(_rxRowSet, ::com::sun::star::uno::UNO_QUERY);
    if (xRowSetProps.is())
    {
        ::com::sun::star::uno::Any aConn(xRowSetProps->getPropertyValue(::rtl::OUString::createFromAscii("ActiveConnection")));
        if  (aConn.getValueType().equals(
            getCppuType(reinterpret_cast< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>*>(NULL))
            ))
            xReturn = *(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>*)aConn.getValue();
    }
    return xReturn;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> calcConnection(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory)
            throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> xReturn;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xRowSetProps(_rxRowSet, ::com::sun::star::uno::UNO_QUERY);
    if (xRowSetProps.is())
    {
        ::com::sun::star::uno::Any aConn( xRowSetProps->getPropertyValue(::rtl::OUString::createFromAscii("ActiveConnection")) );
        if (aConn.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_INTERFACE)
            xReturn = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)aConn.getValue(), ::com::sun::star::uno::UNO_QUERY);

        if (!xReturn.is())
        {
            // first look if there is a connection in the parent hierarchy
            xReturn = findConnection(_rxRowSet);
            if (!xReturn.is())
            {
                static const ::rtl::OUString s_sUserProp = ::rtl::OUString::createFromAscii("User");
                static const ::rtl::OUString s_sPwdProp = ::rtl::OUString::createFromAscii("Password");

                // the row set didn't supply a connection -> build one with it's current settings
                ::rtl::OUString sDataSourceName;
                xRowSetProps->getPropertyValue(::rtl::OUString::createFromAscii("DataSourceName")) >>= sDataSourceName;
                ::rtl::OUString sURL;
                xRowSetProps->getPropertyValue(::rtl::OUString::createFromAscii("URL")) >>= sURL;
                if (sDataSourceName.getLength())
                {   // the row set's data source property is set
                    // -> try to connect, get user and pwd setting for that
                    ::rtl::OUString sUser, sPwd;

                    if (hasProperty(s_sUserProp, xRowSetProps))
                        xRowSetProps->getPropertyValue(s_sUserProp) >>= sUser;
                    if (hasProperty(s_sPwdProp, xRowSetProps))
                        xRowSetProps->getPropertyValue(s_sPwdProp) >>= sPwd;
                    xReturn = getConnection(sDataSourceName, sUser, sPwd, _rxFactory);
                }
                else if (sURL.getLength())
                {   // the row set has no data source, but a connection url set
                    // -> try to connection with that url
                    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseEnvironment> xEnvironment(
                        _rxFactory->createInstance(
                            ::rtl::OUString::createFromAscii("com.sun.star.sdb.DatabaseEnvironment")),
                        ::com::sun::star::uno::UNO_QUERY);
                    if (xEnvironment.is())
                    {
                        ::rtl::OUString sUser, sPwd;
                        if (hasProperty(s_sUserProp, xRowSetProps))
                            xRowSetProps->getPropertyValue(s_sUserProp) >>= sUser;
                        if (hasProperty(s_sPwdProp, xRowSetProps))
                            xRowSetProps->getPropertyValue(s_sPwdProp) >>= sPwd;
                        if (sUser.getLength())
                        {   // use user and pwd together with the url
                            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> aInfo(2);
                            aInfo.getArray()[0].Name = s_sUserProp;
                            aInfo.getArray()[0].Value <<= sUser;
                            aInfo.getArray()[1].Name = s_sPwdProp;
                            aInfo.getArray()[1].Value <<= sPwd;
                            xReturn = xEnvironment->getConnectionWithInfo(sURL, aInfo);
                        }
                        else
                            // just use the url
                            xReturn = xEnvironment->getConnection(sURL);
                    }
                }
            }

            // now if we got a connection, forward it to the row set
            // (The row set will take ownership of that conn and use it for the next execute.
            // If one of the properties affecting the connection (DataSource, URL) is set afterwards,
            // it will free our connection and build a new one with the new parameters on the next execute.
            // At least the service descriptions says so :)
            if (xReturn.is())
            {
                try
                {
                    xRowSetProps->setPropertyValue(::rtl::OUString::createFromAscii("ActiveConnection"), ::com::sun::star::uno::makeAny(xReturn));
                }
                catch(...)
                {
                }
            }
        }
    }

    return xReturn;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getTableFields(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn,
            const ::rtl::OUString& _rName)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier> xSupplyTables(_rxConn, ::com::sun::star::uno::UNO_QUERY);
    OSL_ENSHURE(xSupplyTables.is(), "::getTableFields : invalid connection !");
        // the conn already said it would support the service sdb::Connection
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> xTables( xSupplyTables->getTables());
    if (xTables.is() && xTables->hasByName(_rName))
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier> xTableCols(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >*)xTables->getByName(_rName).getValue(), ::com::sun::star::uno::UNO_QUERY);
        OSL_ENSHURE(xTableCols.is(), "::getTableFields : invalid table !");
            // the table is expected to support the service sddb::Table, which requires an XColumnsSupplier interface

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> xFieldNames(xTableCols->getColumns(), ::com::sun::star::uno::UNO_QUERY);
        OSL_ENSHURE(xFieldNames.is(), "::getTableFields : TableCols->getColumns doesn't export a NameAccess !");
        return xFieldNames;
    }

    return ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>();
}

//------------------------------------------------------------------------------
::com::sun::star::sdb::SQLContext prependContextInfo(::com::sun::star::sdbc::SQLException& _rException, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext, const ::rtl::OUString& _rContextDescription, const ::rtl::OUString& _rContextDetails)
{
    // determine the type of the exception
    SQLExceptionInfo aInfo(_rException);

    // the new first chain element
    ::com::sun::star::sdb::SQLContext aContextDescription(_rContextDescription, _rxContext, ::rtl::OUString(), 0, aInfo.get(), _rContextDetails);
    return aContextDescription;
}

//------------------------------------------------------------------------------
::rtl::OUString quoteName(const ::rtl::OUString& _rQuote, const ::rtl::OUString& _rName)
{
    return _rQuote + _rName + _rQuote;
}

//------------------------------------------------------------------------------
::rtl::OUString quoteTableName(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _rxMeta, const ::rtl::OUString& _rName)
{
    ::rtl::OUString sQuote = _rxMeta->getIdentifierQuoteString();
    ::rtl::OUString sQuotedName;
    ::rtl::OUString aTableName(_rName);

    static ::rtl::OUString s_sEmptyString;

    sal_Unicode aGenericSep = '.';

    if (_rxMeta->supportsCatalogsInDataManipulation())
    {
        sal_Unicode aSeparator = aGenericSep;
        ::rtl::OUString sCatalogSep = _rxMeta->getCatalogSeparator();
        if (sCatalogSep.getLength())
            aSeparator = sCatalogSep[0];

        if (aTableName.getTokenCount(aSeparator) >= 2)
        {
            ::rtl::OUString aDatabaseName(aTableName.getToken(0, aSeparator));
            sQuotedName += quoteName(sQuote, aDatabaseName);
            sQuotedName.concat(::rtl::OUString(&aSeparator, 1));
            aTableName.replaceAt(0, aDatabaseName.getLength() + 1, s_sEmptyString);
                // have no "erase" so simulate this with replaceAt
        }
    }
    if (_rxMeta->supportsSchemasInDataManipulation())
    {
        if (aTableName.getTokenCount(aGenericSep) == 2)
        {
            static ::rtl::OUString s_aGenericSep(&aGenericSep, 1);
                // need a method on the OUString to cancat a single unicode character ....
            sQuotedName += quoteName(sQuote, aTableName.getToken(0, aGenericSep));
            sQuotedName.concat(s_aGenericSep);
            sQuotedName += quoteName(sQuote, aTableName.getToken(1, aGenericSep));
        }
        else
            sQuotedName += quoteName(sQuote, aTableName);
    }
    else
        sQuotedName += quoteName(sQuote, aTableName);

    return sQuotedName;
}

//------------------------------------------------------------------------------
void qualifiedNameComponents(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxConnMetaData, const ::rtl::OUString& _rQualifiedName, ::rtl::OUString& _rCatalog, ::rtl::OUString& _rSchema, ::rtl::OUString& _rName)
{
    OSL_ENSHURE(_rxConnMetaData.is(), "QualifiedNameComponents : invalid meta data!");
    ::rtl::OUString sSeparator = _rxConnMetaData->getCatalogSeparator();

    ::rtl::OUString sName(_rQualifiedName);
    // do we have catalogs ?
    if (_rxConnMetaData->supportsCatalogsInDataManipulation())
    {
        if (_rxConnMetaData->isCatalogAtStart())
        {
            // search for the catalog name at the beginning
            sal_Int32 nIndex = sName.indexOf(sSeparator);
            if (-1 != nIndex)
            {
                _rCatalog = sName.copy(0, nIndex);
                sName = sName.copy(nIndex + 1);
            }
        }
        else
        {
            // Katalogname am Ende
            sal_Int32 nIndex = sName.lastIndexOf(sSeparator);
            if (-1 != nIndex)
            {
                _rCatalog = sName.copy(nIndex + 1);
                sName = sName.copy(0, nIndex);
            }
        }
    }

    if (_rxConnMetaData->supportsSchemasInDataManipulation())
    {
        sal_Int32 nIndex = sName.indexOf((sal_Unicode)'.');
        OSL_ENSHURE(-1 != nIndex, "QualifiedNameComponents : no schema separator!");
        _rSchema = sName.copy(0, nIndex);
        sName = sName.copy(nIndex + 1);
    }

    _rName = sName;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier> getNumberFormats(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn,
            sal_Bool _bAlloweDefault,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory)
{
    // ask the parent of the connection (should be an DatabaseAccess)
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier> xReturn;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xConnAsChild(_rxConn, ::com::sun::star::uno::UNO_QUERY);
    ::rtl::OUString sPropFormatsSupplier = ::rtl::OUString::createFromAscii("NumberFormatsSupplier");
    if (xConnAsChild.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xConnParentProps(xConnAsChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
        if (xConnParentProps.is() && hasProperty(sPropFormatsSupplier, xConnParentProps))
        {
            ::com::sun::star::uno::Any aSupplier( xConnParentProps->getPropertyValue(sPropFormatsSupplier) );
            if (aSupplier.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_INTERFACE)
                xReturn = ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >*)aSupplier.getValue(), ::com::sun::star::uno::UNO_QUERY);
        }
    }

    // no result 'til now ? -> ask the environment for the default formatter
    if (!xReturn.is() && _bAlloweDefault)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseEnvironment> xEnv(_rxFactory->createInstance(
            ::rtl::OUString::createFromAscii("com.sun.star.sdb.DatabaseEnvironment")),
            ::com::sun::star::uno::UNO_QUERY);

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xEnvProps(xEnv, ::com::sun::star::uno::UNO_QUERY);
        if (xEnv.is() && hasProperty(sPropFormatsSupplier, xEnvProps))
        {
            ::com::sun::star::uno::Any aSupplier( xEnvProps->getPropertyValue(sPropFormatsSupplier) );
            if (aSupplier.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_INTERFACE)
                xReturn = ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >*)aSupplier.getValue(), ::com::sun::star::uno::UNO_QUERY);
        }
    }

    return xReturn;
}

//==============================================================================
//------------------------------------------------------------------------------
void TransferFormComponentProperties(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xOldProps,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xNewProps,
            const ::com::sun::star::lang::Locale& _rLocale)
{
    // kopieren wir erst mal alle Props, die in Quelle und Ziel vorhanden sind und identische Beschreibungen haben
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> xOldInfo( xOldProps->getPropertySetInfo());
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> xNewInfo( xNewProps->getPropertySetInfo());

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> aOldProperties = xOldInfo->getProperties();
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> aNewProperties = xNewInfo->getProperties();
    int nNewLen = aNewProperties.getLength();

    ::com::sun::star::beans::Property* pOldProps = aOldProperties.getArray();
    ::com::sun::star::beans::Property* pNewProps = aNewProperties.getArray();

    ::rtl::OUString sPropDefaultControl(::rtl::OUString::createFromAscii("DefaultControl"));
    ::rtl::OUString sPropLabelControl(::rtl::OUString::createFromAscii("LabelControl"));
    ::rtl::OUString sPropFormatsSupplier(::rtl::OUString::createFromAscii("FormatsSupplier"));
    ::rtl::OUString sPropFormatKey(::rtl::OUString::createFromAscii("FormatKey"));
    ::rtl::OUString sPropCurrencySymbol(::rtl::OUString::createFromAscii("CurrencySymbol"));
    ::rtl::OUString sPropDecimals(::rtl::OUString::createFromAscii("Decimals"));
    ::rtl::OUString sPropEffectiveMin(::rtl::OUString::createFromAscii("EffectiveMin"));
    ::rtl::OUString sPropEffectiveMax(::rtl::OUString::createFromAscii("EffectiveMax"));
    ::rtl::OUString sPropEffectiveDefault(::rtl::OUString::createFromAscii("EffectiveDefault"));
    ::rtl::OUString sPropDefaultValue(::rtl::OUString::createFromAscii("DefaultValue"));
    ::rtl::OUString sPropDefaultText(::rtl::OUString::createFromAscii("DefaultText"));
    ::rtl::OUString sPropDefaultDate(::rtl::OUString::createFromAscii("DefaultDate"));
    ::rtl::OUString sPropDefaultTime(::rtl::OUString::createFromAscii("DefaultTime"));
    ::rtl::OUString sPropValueMin(::rtl::OUString::createFromAscii("ValueMin"));
    ::rtl::OUString sPropValueMax(::rtl::OUString::createFromAscii("ValueMax"));
    ::rtl::OUString sPropDecimalAccuracy(::rtl::OUString::createFromAscii("DecimalAccuracy"));
    ::rtl::OUString sPropClassId(::rtl::OUString::createFromAscii("ClassId"));

    for (sal_Int16 i=0; i<aOldProperties.getLength(); ++i)
    {
        if  (   (!pOldProps[i].Name.equals(sPropDefaultControl))
            &&  (!pOldProps[i].Name.equals(sPropLabelControl))
            )
        {
            // binaere Suche
            ::com::sun::star::beans::Property* pResult = (::com::sun::star::beans::Property*) bsearch(pOldProps + i, (void*)pNewProps, nNewLen, sizeof(::com::sun::star::beans::Property),
                &PropertyCompare);
            if (pResult && (pResult->Attributes == pOldProps[i].Attributes)
                && ((pResult->Attributes & ::com::sun::star::beans::PropertyAttribute::READONLY) == 0)
                && (pResult->Type.equals(pOldProps[i].Type)))
            {   // Attribute stimmen ueberein und Property ist nicht read-only
                try
                {
                    xNewProps->setPropertyValue(pResult->Name, xOldProps->getPropertyValue(pResult->Name));
                }
                catch(::com::sun::star::lang::IllegalArgumentException& e)
                {
                    e;
#ifdef DBG_UTIL
                    ::rtl::OUString sMessage = ::rtl::OUString::createFromAscii("TransferFormComponentProperties : could not transfer the value for property \"");
                    sMessage += pResult->Name;
                    sMessage += ::rtl::OUString::createFromAscii("\"");;
                    OSL_ENSHURE(sal_False, ::rtl::OUStringToOString(sMessage, RTL_TEXTENCODING_ASCII_US));
#endif
                }
            }
        }
    }


    // fuer formatierte Felder (entweder alt oder neu) haben wir ein paar Sonderbehandlungen
    sal_Bool bOldIsFormatted = hasProperty(sPropFormatsSupplier, xOldProps) && hasProperty(sPropFormatKey, xOldProps);
    sal_Bool bNewIsFormatted = hasProperty(sPropFormatsSupplier, xNewProps) && hasProperty(sPropFormatKey, xNewProps);

    if (!bOldIsFormatted && !bNewIsFormatted)
        return; // nothing to do

    if (bOldIsFormatted && bNewIsFormatted)
        // nein, wenn beide formatierte Felder sind, dann machen wir keinerlei Konvertierungen
        // Das geht zu weit ;)
        return;

    if (bOldIsFormatted)
    {
        // aus dem eingestellten Format ein paar Properties rausziehen und zum neuen Set durchschleifen
        ::com::sun::star::uno::Any aFormatKey( xOldProps->getPropertyValue(sPropFormatKey) );
        if (aFormatKey.hasValue())
        {
            ::com::sun::star::uno::Any aSupplier( xOldProps->getPropertyValue(sPropFormatsSupplier) );
            OSL_ENSHURE(isAReference(aSupplier.getValueType(), static_cast< ::com::sun::star::util::XNumberFormatsSupplier*>(NULL)),
                "TransferFormComponentProperties : invalid property type !");
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier> xSupplier;
            if (aSupplier.hasValue())
                xSupplier = *(::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>*)aSupplier.getValue();
            if (xSupplier.is())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats> xFormats(xSupplier->getNumberFormats());
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xFormat(xFormats->getByKey(getINT32(aFormatKey)));
                if (hasProperty(sPropCurrencySymbol, xFormat))
                {
                    ::com::sun::star::uno::Any aVal( xFormat->getPropertyValue(sPropCurrencySymbol) );
                    if (aVal.hasValue() && hasProperty(sPropCurrencySymbol, xNewProps))
                        // (wenn die Quelle das nicht gesetzt hat, dann auch nicht kopieren, um den
                        // Default-Wert nicht zu ueberschreiben
                        xNewProps->setPropertyValue(sPropCurrencySymbol, aVal);
                }
                if (hasProperty(sPropDecimals, xFormat) && hasProperty(sPropDecimals, xNewProps))
                    xNewProps->setPropertyValue(sPropDecimals, xFormat->getPropertyValue(sPropDecimals));
            }
        }

        // eine eventuelle-Min-Max-Konvertierung
        ::com::sun::star::uno::Any aEffectiveMin( xOldProps->getPropertyValue(sPropEffectiveMin) );
        if (aEffectiveMin.hasValue())
        {   // im Gegensatz zu ValueMin kann EffectiveMin void sein
            if (hasProperty(sPropValueMin, xNewProps))
            {
                OSL_ENSHURE(aEffectiveMin.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_DOUBLE,
                    "TransferFormComponentProperties : invalid property type !");
                xNewProps->setPropertyValue(sPropValueMin, aEffectiveMin);
            }
        }
        ::com::sun::star::uno::Any aEffectiveMax( xOldProps->getPropertyValue(sPropEffectiveMax) );
        if (aEffectiveMax.hasValue())
        {   // analog
            if (hasProperty(sPropValueMax, xNewProps))
            {
                OSL_ENSHURE(aEffectiveMax.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_DOUBLE,
                    "TransferFormComponentProperties : invalid property type !");
                xNewProps->setPropertyValue(sPropValueMax, aEffectiveMax);
            }
        }

        // dann koennen wir noch Default-Werte konvertieren und uebernehmen
        ::com::sun::star::uno::Any aEffectiveDefault( xOldProps->getPropertyValue(sPropEffectiveDefault) );
        if (aEffectiveDefault.hasValue())
        {
            sal_Bool bIsString = aEffectiveDefault.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_STRING;
            OSL_ENSHURE(bIsString || aEffectiveDefault.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_DOUBLE,
                "TransferFormComponentProperties : invalid property type !");
                // die Effective-Properties sollten immer void oder string oder double sein ....

            if (hasProperty(sPropDefaultDate, xNewProps) && !bIsString)
            {   // (einen String in ein Datum zu konvertieren muss nicht immer klappen, denn das ganze kann ja an
                // eine Textspalte gebunden gewesen sein, aber mit einem double koennen wir was anfangen)
                ::com::sun::star::util::Date aDate = DBTypeConversion::toDate(getDouble(aEffectiveDefault));
                xNewProps->setPropertyValue(sPropDefaultDate, ::com::sun::star::uno::makeAny(aDate));
            }

            if (hasProperty(sPropDefaultTime, xNewProps) && !bIsString)
            {   // voellig analog mit Zeit
                ::com::sun::star::util::Time aTime = DBTypeConversion::toTime(getDouble(aEffectiveDefault));
                xNewProps->setPropertyValue(sPropDefaultTime, ::com::sun::star::uno::makeAny(aTime));
            }

            if (hasProperty(sPropDefaultValue, xNewProps) && !bIsString)
            {   // hier koennen wir einfach das double durchreichen
                xNewProps->setPropertyValue(sPropDefaultValue, aEffectiveDefault);
            }

            if (hasProperty(sPropDefaultText, xNewProps) && bIsString)
            {   // und hier den String
                xNewProps->setPropertyValue(sPropDefaultText, aEffectiveDefault);
            }

            // nyi: die Uebersetzung zwischen doubles und String wuerde noch mehr Moeglichkeien eroeffnen
        }
    }

    // die andere Richtung : das neu Control soll formatiert sein
    if (bNewIsFormatted)
    {
        // zuerst die Formatierung
        // einen Supplier koennen wir nicht setzen, also muss das neue Set schon einen mitbringen
        ::com::sun::star::uno::Any aSupplier( xNewProps->getPropertyValue(sPropFormatsSupplier) );
        OSL_ENSHURE(isAReference(aSupplier, static_cast< ::com::sun::star::util::XNumberFormatsSupplier*>(NULL)),
            "TransferFormComponentProperties : invalid property type !");
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier> xSupplier;
        if (aSupplier.hasValue())
            xSupplier = *(::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>*)aSupplier.getValue();
        if (xSupplier.is())
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats> xFormats(xSupplier->getNumberFormats());

            // Sprache des neuen Formats
//          ::rtl::OString sLanguage, sCountry;
//          ConvertLanguageToIsoNames(Application::GetAppInternational().GetLanguage(), String(sLanguage.getStr()), String(sCountry.getStr()));
//          ::com::sun::star::lang::Locale aNewLanguage(
//              ::rtl::OStringToOUString(sLanguage, RTL_TEXTENCODING_ASCII_US),
//              ::rtl::OStringToOUString(sCountry, RTL_TEXTENCODING_ASCII_US),
//              ::rtl::OUString());

            // Dezimal-Stellen
            sal_Int16 nDecimals = 2;
            if (hasProperty(sPropDecimalAccuracy, xOldProps))
                xOldProps->getPropertyValue(sPropDecimalAccuracy) >>= nDecimals;

            // Grund-Format (je nach ClassId des alten Sets)
            sal_Int32 nBaseKey = 0;
            if (hasProperty(sPropClassId, xOldProps))
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatTypes> xTypeList(xFormats, ::com::sun::star::uno::UNO_QUERY);
                if (xTypeList.is())
                {
                    sal_Int16 nClassId;
                    xOldProps->getPropertyValue(sPropClassId) >>= nClassId;
                    switch (nClassId)
                    {
                        case ::com::sun::star::form::FormComponentType::DATEFIELD :
                            nBaseKey = xTypeList->getStandardFormat(::com::sun::star::util::NumberFormat::DATE, _rLocale);
                            break;

                        case ::com::sun::star::form::FormComponentType::TIMEFIELD :
                            nBaseKey = xTypeList->getStandardFormat(::com::sun::star::util::NumberFormat::TIME, _rLocale);
                            break;

                        case ::com::sun::star::form::FormComponentType::CURRENCYFIELD :
                            nBaseKey = xTypeList->getStandardFormat(::com::sun::star::util::NumberFormat::CURRENCY, _rLocale);
                            break;
                    }
                }
            }

            // damit koennen wir ein neues Format basteln ...
            ::rtl::OUString sNewFormat = xFormats->generateFormat(nBaseKey, _rLocale, sal_False, sal_False, nDecimals, 0);
                // kein Tausender-Trennzeichen, negative Zahlen nicht in Rot, keine fuehrenden Nullen

            // ... und zum FormatsSupplier hinzufuegen (wenn noetig)
            sal_Int32 nKey = xFormats->queryKey(sNewFormat, _rLocale, sal_False);
            if (nKey == (sal_Int32)-1)
            {   // noch nicht vorhanden in meinem Formatter ...
                nKey = xFormats->addNew(sNewFormat, _rLocale);
            }

            xNewProps->setPropertyValue(sPropFormatKey, ::com::sun::star::uno::makeAny((sal_Int32)nKey));
        }

        // min-/max-Werte
        ::com::sun::star::uno::Any aNewMin, aNewMax;
        if (hasProperty(sPropValueMin, xOldProps))
            aNewMin = xOldProps->getPropertyValue(sPropValueMin);
        if (hasProperty(sPropValueMax, xOldProps))
            aNewMax = xOldProps->getPropertyValue(sPropValueMax);
        xNewProps->setPropertyValue(sPropEffectiveMin, aNewMin);
        xNewProps->setPropertyValue(sPropEffectiveMax, aNewMax);

        // Default-Wert
        ::com::sun::star::uno::Any aNewDefault;
        if (hasProperty(sPropDefaultDate, xOldProps))
        {
            ::com::sun::star::uno::Any aDate( xOldProps->getPropertyValue(sPropDefaultDate) );
            if (aDate.hasValue())
                aNewDefault <<= DBTypeConversion::toDouble(*(::com::sun::star::util::Date*)aDate.getValue());
        }

        if (hasProperty(sPropDefaultTime, xOldProps))
        {
            ::com::sun::star::uno::Any aTime( xOldProps->getPropertyValue(sPropDefaultTime) );
            if (aTime.hasValue())
                aNewDefault <<= DBTypeConversion::toDouble(*(::com::sun::star::util::Time*)aTime.getValue());
        }

        // double oder String werden direkt uebernommen
        if (hasProperty(sPropDefaultValue, xOldProps))
            aNewDefault = xOldProps->getPropertyValue(sPropDefaultValue);
        if (hasProperty(sPropDefaultText, xOldProps))
            aNewDefault = xOldProps->getPropertyValue(sPropDefaultText);

        if (aNewDefault.hasValue())
            xNewProps->setPropertyValue(sPropEffectiveDefault, aNewDefault);
    }
}

//------------------------------------------------------------------------------
sal_Bool canInsert(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet)
{
    return ((_rxCursorSet.is() && (getINT32(_rxCursorSet->getPropertyValue(::rtl::OUString::createFromAscii("Privileges"))) & ::com::sun::star::sdbcx::Privilege::INSERT) != 0));
}

//------------------------------------------------------------------------------
sal_Bool canUpdate(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet)
{
    return ((_rxCursorSet.is() && (getINT32(_rxCursorSet->getPropertyValue(::rtl::OUString::createFromAscii("Privileges"))) & ::com::sun::star::sdbcx::Privilege::UPDATE) != 0));
}

//------------------------------------------------------------------------------
sal_Bool canDelete(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet)
{
    return ((_rxCursorSet.is() && (getINT32(_rxCursorSet->getPropertyValue(::rtl::OUString::createFromAscii("Privileges"))) & ::com::sun::star::sdbcx::Privilege::DELETE) != 0));
}


//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer> getCurrentSettingsComposer(
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxRowSetProps,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer> xReturn;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet> xRowSet(_rxRowSetProps, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> xConn( calcConnection(xRowSet, _rxFactory));
    try
    {
        if (xConn.is())     // implies xRowSet.is() implies _rxRowSetProps.is()
        {
            // build the statement the row set is based on (can't use the ActiveCommand property of the set
            // as this reflects the status after the last execute, not the currently set properties

            ::rtl::OUString sStatement;
            const ::rtl::OUString sPropCommand = ::rtl::OUString::createFromAscii("Command");
            const ::rtl::OUString sPropCommandType = ::rtl::OUString::createFromAscii("CommandType");
            const ::rtl::OUString sPropFilter = ::rtl::OUString::createFromAscii("Filter");
            const ::rtl::OUString sPropOrder = ::rtl::OUString::createFromAscii("Order");
            const ::rtl::OUString sPropApplyFilter = ::rtl::OUString::createFromAscii("ApplyFilter");
            const ::rtl::OUString sPropEscapeProcessing = ::rtl::OUString::createFromAscii("EscapeProcessing");

            // first ensure we have all properties needed
            if (hasProperty(sPropCommand, _rxRowSetProps) && hasProperty(sPropCommandType, _rxRowSetProps)
                && hasProperty(sPropFilter, _rxRowSetProps) && hasProperty(sPropOrder, _rxRowSetProps)
                && hasProperty(sPropEscapeProcessing, _rxRowSetProps) && hasProperty(sPropApplyFilter, _rxRowSetProps))
            {
                sal_Int32 nCommandType = getINT32(_rxRowSetProps->getPropertyValue(sPropCommandType));
                ::rtl::OUString sCommand = getString(_rxRowSetProps->getPropertyValue(sPropCommand));
                sal_Bool bEscapeProcessing = getBOOL(_rxRowSetProps->getPropertyValue(sPropEscapeProcessing));
                switch (nCommandType)
                {
                    case ::com::sun::star::sdb::CommandType::COMMAND:
                        if (!bEscapeProcessing)
                        {   // native sql -> no parsable statement
                            sStatement = ::rtl::OUString();
                        }
                        else
                        {
                            sStatement = sCommand;
                        }
                        break;
                    case ::com::sun::star::sdb::CommandType::TABLE:
                    {
                        if (!sCommand.getLength())
                            break;

                        ::rtl::OUString sTableName = quoteTableName(xConn->getMetaData(), sCommand);
                        sStatement = ::rtl::OUString::createFromAscii("SELECT * FROM ");
                        sStatement += sTableName;
                    }
                    break;
                    case ::com::sun::star::sdb::CommandType::QUERY:
                    {
                        // ask the connection for the query
                        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XQueriesSupplier> xSupplyQueries(xConn, ::com::sun::star::uno::UNO_QUERY);
                        if (!xSupplyQueries.is())
                            break;

                        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> xQueries(xSupplyQueries->getQueries(), ::com::sun::star::uno::UNO_QUERY);
                        if (!xQueries.is() || !xQueries->hasByName(sCommand))
                            break;

                        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xQueryProps(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)xQueries->getByName(sCommand).getValue(), ::com::sun::star::uno::UNO_QUERY);
                        if (!xQueryProps.is())
                            break;

                        //  a native query ?
                        if (!hasProperty(sPropEscapeProcessing, xQueryProps))
                            break;
                        if (!getBOOL(xQueryProps->getPropertyValue(sPropEscapeProcessing)))
                            break;

                        if (!hasProperty(sPropCommand, xQueryProps))
                            break;

                        // the command used by the query
                        sStatement = getString(xQueryProps->getPropertyValue(sPropCommand));

                        // use an additional composer to build a statement from the query filter/order props
                        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposerFactory> xFactory(xConn, ::com::sun::star::uno::UNO_QUERY);
                        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer> xLocalComposer;
                        if (xFactory.is())
                            xLocalComposer = xFactory->createQueryComposer();
                        if (!xLocalComposer.is())
                            break;

                        xLocalComposer->setQuery(sStatement);
                        // the sort order
                        if (hasProperty(sPropOrder, xQueryProps))
                            xLocalComposer->setOrder(getString(xQueryProps->getPropertyValue(sPropOrder)));

                        sal_Bool bApplyFilter = sal_False;
                        if (hasProperty(sPropApplyFilter, xQueryProps))
                            bApplyFilter = getBOOL(xQueryProps->getPropertyValue(sPropApplyFilter));

                        if (bApplyFilter)
                        {
                            if (hasProperty(sPropFilter, xQueryProps))
                                xLocalComposer->setFilter(getString(xQueryProps->getPropertyValue(sPropFilter)));
                        }
                        sStatement = xLocalComposer->getComposedQuery();
                    }
                    break;
                    default:
                        OSL_ENSHURE(sal_False, "::getCurrentSettingsComposer : no table, no query, no statement - what else ?!");
                        break;
                }
            }

            if (sStatement.getLength())
            {
                // create an composer
                ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposerFactory> xFactory(xConn, ::com::sun::star::uno::UNO_QUERY);
                if (xFactory.is())
                    xReturn = xFactory->createQueryComposer();
                if (xReturn.is())
                {
                    xReturn->setQuery(sStatement);
                    // append filter/sort
                    xReturn->setOrder(getString(_rxRowSetProps->getPropertyValue(sPropOrder)));
                    sal_Bool bApplyFilter = getBOOL(_rxRowSetProps->getPropertyValue(sPropApplyFilter));
                    if (bApplyFilter)
                        xReturn->setFilter(getString(_rxRowSetProps->getPropertyValue(sPropFilter)));
                }
            }
        }
    }
    catch(...)
    {
        OSL_ENSHURE(sal_False, "::getCurrentSettingsComposer : catched an exception !");
        xReturn = NULL;
    }


    return xReturn;
}
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace cppu;
using namespace osl;

//--------------------------------------------------------------------------
void composeTableName(  const Reference< XDatabaseMetaData >& _rxMetaData,
                        const ::rtl::OUString& _rCatalog,
                        const ::rtl::OUString& _rSchema,
                        const ::rtl::OUString& _rName,
                        ::rtl::OUString& _rComposedName,
                        sal_Bool _bQuote)
{
    OSL_ENSHURE(_rxMetaData.is(), "composeTableName : invalid meta data !");
    OSL_ENSHURE(_rName.getLength(), "composeTableName : at least the name should be non-empty !");

    ::rtl::OUString sQuoteString = _rxMetaData->getIdentifierQuoteString();
#define QUOTE(s) if (_bQuote) s += sQuoteString

    static ::rtl::OUString sEmpty;
    static ::rtl::OUString sSeparator = ::rtl::OUString::createFromAscii(".");
    _rComposedName = sEmpty;

    if (_rCatalog.getLength() && _rxMetaData->isCatalogAtStart() && !_rxMetaData->usesLocalFiles())
    {
        QUOTE(_rComposedName);
        _rComposedName += _rCatalog;
        QUOTE(_rComposedName);
        _rComposedName += _rxMetaData->getCatalogSeparator();
    }

    if (_rSchema.getLength())
    {
        QUOTE(_rComposedName);
        _rComposedName += _rSchema;
        QUOTE(_rComposedName);
        _rComposedName += sSeparator;
        QUOTE(_rComposedName);
        _rComposedName += _rName;
        QUOTE(_rComposedName);
    }
    else
    {
        QUOTE(_rComposedName);
        _rComposedName += _rName;
        QUOTE(_rComposedName);
    }

    if (_rCatalog.getLength() && !_rxMetaData->isCatalogAtStart() && !_rxMetaData->usesLocalFiles())
    {
        _rComposedName += _rxMetaData->getCatalogSeparator();
        QUOTE(_rComposedName);
        _rComposedName += _rCatalog;
        QUOTE(_rComposedName);
    }
}


//.........................................................................
}   // namespace dbtools
//.........................................................................


/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/10/05 08:50:51  fs
 *  moved the files from unotools to here
 *
 *
 *  Revision 1.0 29.09.00 08:16:59  fs
 ************************************************************************/

