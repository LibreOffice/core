/*************************************************************************
 *
 *  $RCSfile: dbtools.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-05 08:50:51 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

using namespace ::comphelper;
//.........................................................................
namespace dbtools
{
//.........................................................................

    namespace starsdbcx     = ::com::sun::star::sdbcx;
    namespace starform      = ::com::sun::star::form;

//==============================================================================
//==============================================================================
//------------------------------------------------------------------
sal_Int32 getDefaultNumberFormat(const staruno::Reference< starbeans::XPropertySet >& _xColumn,
                                 const staruno::Reference< starutil::XNumberFormatTypes >& _xTypes,
                                 const starlang::Locale& _rLocale)
{
    OSL_ENSHURE(_xTypes.is() && _xColumn.is(), "dbtools::getDefaultNumberFormat: invalid arg !");
    if (!_xTypes.is() || !_xColumn.is())
        return starutil::NumberFormat::UNDEFINED;

    sal_Int32 nDataType;
    try
    {
        // determine the datatype of the column
        _xColumn->getPropertyValue(::rtl::OUString::createFromAscii("Type")) >>= nDataType;
    }
    catch (...)
    {
        return starutil::NumberFormat::UNDEFINED;
    }

    sal_Int32 nFormat;
    switch (nDataType)
    {
        case starsdbc::DataType::BIT:
            nFormat = _xTypes->getStandardFormat(starutil::NumberFormat::LOGICAL, _rLocale);
            break;
        case starsdbc::DataType::TINYINT:
        case starsdbc::DataType::SMALLINT:
        case starsdbc::DataType::INTEGER:
        case starsdbc::DataType::BIGINT:
            nFormat = _xTypes->getStandardFormat(starutil::NumberFormat::NUMBER, _rLocale);
            break;
        case starsdbc::DataType::FLOAT:
        case starsdbc::DataType::REAL:
        case starsdbc::DataType::DOUBLE:
        case starsdbc::DataType::NUMERIC:
        case starsdbc::DataType::DECIMAL:
        {
            try
            {
                if (getBOOL(_xColumn->getPropertyValue(::rtl::OUString::createFromAscii("IsCurrency"))))
                    nFormat = _xTypes->getStandardFormat(starutil::NumberFormat::CURRENCY, _rLocale);
                else
                {
                    sal_Int32 nScale = 2;
                    if (starsdbc::DataType::NUMERIC == nDataType || starsdbc::DataType::DECIMAL == nDataType)
                        _xColumn->getPropertyValue(::rtl::OUString::createFromAscii("Scale")) >>= nScale;

                    // generate a new format if necessary
                    staruno::Reference< starutil::XNumberFormats > xFormats(_xTypes, staruno::UNO_QUERY);
                    ::rtl::OUString sNewFormat = xFormats->generateFormat( 0L, _rLocale, sal_False, sal_False, nScale, sal_True);

                    // and add it to the formatter if necessary
                    nFormat = xFormats->queryKey(sNewFormat, _rLocale, sal_False);
                    if (nFormat == (sal_Int32)-1)
                        nFormat = xFormats->addNew(sNewFormat, _rLocale);
                }
            }
            catch (...)
            {
                nFormat = _xTypes->getStandardFormat(starutil::NumberFormat::NUMBER, _rLocale);
            }
        }   break;
        case starsdbc::DataType::CHAR:
        case starsdbc::DataType::VARCHAR:
        case starsdbc::DataType::LONGVARCHAR:
            nFormat = _xTypes->getStandardFormat(starutil::NumberFormat::TEXT, _rLocale);
            break;
        case starsdbc::DataType::DATE:
            nFormat = _xTypes->getStandardFormat(starutil::NumberFormat::DATE, _rLocale);
            break;
        case starsdbc::DataType::TIME:
            nFormat = _xTypes->getStandardFormat(starutil::NumberFormat::TIME, _rLocale);
            break;
        case starsdbc::DataType::TIMESTAMP:
            nFormat = _xTypes->getStandardFormat(starutil::NumberFormat::DATETIME, _rLocale);
            break;
        case starsdbc::DataType::BINARY:
        case starsdbc::DataType::VARBINARY:
        case starsdbc::DataType::LONGVARBINARY:
        case starsdbc::DataType::SQLNULL:
        case starsdbc::DataType::OTHER:
        case starsdbc::DataType::OBJECT:
        case starsdbc::DataType::DISTINCT:
        case starsdbc::DataType::STRUCT:
        case starsdbc::DataType::ARRAY:
        case starsdbc::DataType::BLOB:
        case starsdbc::DataType::CLOB:
        case starsdbc::DataType::REF:
        default:
            nFormat = starutil::NumberFormat::UNDEFINED;
    }
    return nFormat;
}

//==============================================================================
//------------------------------------------------------------------------------
staruno::Reference<starsdbc::XConnection> findConnection(const staruno::Reference< staruno::XInterface >& xParent)
{
    staruno::Reference<starsdbc::XConnection> xConnection(xParent, staruno::UNO_QUERY);
    if (!xConnection.is())
    {
        staruno::Reference<starcontainer::XChild> xChild(xParent, staruno::UNO_QUERY);
        if (xChild.is())
            return findConnection(xChild->getParent());
    }
    return xConnection;
}

//------------------------------------------------------------------------------
staruno::Reference<starsdbc::XDataSource> getDataSource(
            const ::rtl::OUString& _rsTitleOrPath,
            const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    OSL_ENSHURE(_rsTitleOrPath.getLength(), "::getDataSource : invalid arg !");

    staruno::Reference<starsdbc::XDataSource>  xReturn;

    // is it a file url ?
    staruno::Reference<starcontainer::XNameAccess> xNamingContext(
        _rxFactory->createInstance(
            ::rtl::OUString::createFromAscii("com.sun.star.sdb.DatabaseAccessContext")),
        staruno::UNO_QUERY);

    if (xNamingContext.is() && xNamingContext->hasByName(_rsTitleOrPath))
    {
        OSL_ENSHURE(staruno::Reference<staruno::XNamingService>(xNamingContext, staruno::UNO_QUERY).is(), "::getDataSource : no NamingService interface on the DatabaseAccessContext !");
        xReturn = staruno::Reference<starsdbc::XDataSource>(
            staruno::Reference<staruno::XNamingService>(xNamingContext, staruno::UNO_QUERY)->getRegisteredObject(_rsTitleOrPath),
            staruno::UNO_QUERY);
    }
    else
    {   // is it a favorite title ?
        staruno::Reference<starsdb::XDatabaseEnvironment> xEnvironment(
            _rxFactory->createInstance(
                ::rtl::OUString::createFromAscii("com.sun.star.sdb.DatabaseEnvironment")),
            staruno::UNO_QUERY);

        if (xEnvironment.is())
        {
            try
            {
                xReturn = staruno::Reference<starsdbc::XDataSource>(xEnvironment->getDatabaseAccess(_rsTitleOrPath), staruno::UNO_QUERY);
            }
            catch(starsdbc::SQLException& e)
            {   // allowed, the env may throw an exception in case of an invalid name
                e; // make compiler happy
            }
        }

    }

    return xReturn;
}

//------------------------------------------------------------------------------
staruno::Reference<starsdbc::XConnection> getConnection(
            const ::rtl::OUString& _rsTitleOrPath,
            const ::rtl::OUString& _rsUser,
            const ::rtl::OUString& _rsPwd,
            const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    staruno::Reference<starsdbc::XDataSource> xDataSource( getDataSource(_rsTitleOrPath, _rxFactory) );
    if (xDataSource.is())
        return xDataSource->getConnection(_rsUser, _rsPwd);

    return staruno::Reference<starsdbc::XConnection>();
}

//------------------------------------------------------------------------------
staruno::Reference<starsdbc::XConnection> getConnection(const staruno::Reference<starsdbc::XRowSet>& _rxRowSet) throw (staruno::RuntimeException)
{
    staruno::Reference<starsdbc::XConnection> xReturn;
    staruno::Reference<starbeans::XPropertySet> xRowSetProps(_rxRowSet, staruno::UNO_QUERY);
    if (xRowSetProps.is())
    {
        staruno::Any aConn(xRowSetProps->getPropertyValue(::rtl::OUString::createFromAscii("ActiveConnection")));
        if  (aConn.getValueType().equals(
            getCppuType(reinterpret_cast<staruno::Reference<starsdbc::XConnection>*>(NULL))
            ))
            xReturn = *(staruno::Reference<starsdbc::XConnection>*)aConn.getValue();
    }
    return xReturn;
}

//------------------------------------------------------------------------------
staruno::Reference<starsdbc::XConnection> calcConnection(
            const staruno::Reference<starsdbc::XRowSet>& _rxRowSet,
            const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
            throw (starsdbc::SQLException, staruno::RuntimeException)
{
    staruno::Reference<starsdbc::XConnection> xReturn;
    staruno::Reference<starbeans::XPropertySet> xRowSetProps(_rxRowSet, staruno::UNO_QUERY);
    if (xRowSetProps.is())
    {
        staruno::Any aConn( xRowSetProps->getPropertyValue(::rtl::OUString::createFromAscii("ActiveConnection")) );
        if (aConn.getValueType().getTypeClass() == staruno::TypeClass_INTERFACE)
            xReturn = staruno::Reference<starsdbc::XConnection>(*(staruno::Reference<staruno::XInterface>*)aConn.getValue(), staruno::UNO_QUERY);

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
                    staruno::Reference<starsdb::XDatabaseEnvironment> xEnvironment(
                        _rxFactory->createInstance(
                            ::rtl::OUString::createFromAscii("com.sun.star.sdb.DatabaseEnvironment")),
                        staruno::UNO_QUERY);
                    if (xEnvironment.is())
                    {
                        ::rtl::OUString sUser, sPwd;
                        if (hasProperty(s_sUserProp, xRowSetProps))
                            xRowSetProps->getPropertyValue(s_sUserProp) >>= sUser;
                        if (hasProperty(s_sPwdProp, xRowSetProps))
                            xRowSetProps->getPropertyValue(s_sPwdProp) >>= sPwd;
                        if (sUser.getLength())
                        {   // use user and pwd together with the url
                            staruno::Sequence<starbeans::PropertyValue> aInfo(2);
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
                    xRowSetProps->setPropertyValue(::rtl::OUString::createFromAscii("ActiveConnection"), staruno::makeAny(xReturn));
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
staruno::Reference<starcontainer::XNameAccess> getTableFields(
            const staruno::Reference<starsdbc::XConnection>& _rxConn,
            const ::rtl::OUString& _rName)
{
    staruno::Reference<starsdbcx::XTablesSupplier> xSupplyTables(_rxConn, staruno::UNO_QUERY);
    OSL_ENSHURE(xSupplyTables.is(), "::getTableFields : invalid connection !");
        // the conn already said it would support the service sdb::Connection
    staruno::Reference<starcontainer::XNameAccess> xTables( xSupplyTables->getTables());
    if (xTables.is() && xTables->hasByName(_rName))
    {
        staruno::Reference<starsdbcx::XColumnsSupplier> xTableCols(*(staruno::Reference< staruno::XInterface >*)xTables->getByName(_rName).getValue(), staruno::UNO_QUERY);
        OSL_ENSHURE(xTableCols.is(), "::getTableFields : invalid table !");
            // the table is expected to support the service sddb::Table, which requires an XColumnsSupplier interface

        staruno::Reference<starcontainer::XNameAccess> xFieldNames(xTableCols->getColumns(), staruno::UNO_QUERY);
        OSL_ENSHURE(xFieldNames.is(), "::getTableFields : TableCols->getColumns doesn't export a NameAccess !");
        return xFieldNames;
    }

    return staruno::Reference<starcontainer::XNameAccess>();
}

//------------------------------------------------------------------------------
starsdb::SQLContext prependContextInfo(starsdbc::SQLException& _rException, const staruno::Reference< staruno::XInterface >& _rxContext, const ::rtl::OUString& _rContextDescription, const ::rtl::OUString& _rContextDetails)
{
    // determine the type of the exception
    SQLExceptionInfo aInfo(_rException);

    // the new first chain element
    starsdb::SQLContext aContextDescription(_rContextDescription, _rxContext, ::rtl::OUString(), 0, aInfo.get(), _rContextDetails);
    return aContextDescription;
}

//------------------------------------------------------------------------------
::rtl::OUString quoteName(const ::rtl::OUString& _rQuote, const ::rtl::OUString& _rName)
{
    return _rQuote + _rName + _rQuote;
}

//------------------------------------------------------------------------------
::rtl::OUString quoteTableName(const staruno::Reference<starsdbc::XDatabaseMetaData>& _rxMeta, const ::rtl::OUString& _rName)
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
void qualifiedNameComponents(const staruno::Reference< starsdbc::XDatabaseMetaData >& _rxConnMetaData, const ::rtl::OUString& _rQualifiedName, ::rtl::OUString& _rCatalog, ::rtl::OUString& _rSchema, ::rtl::OUString& _rName)
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
staruno::Reference<starutil::XNumberFormatsSupplier> getNumberFormats(
            const staruno::Reference<starsdbc::XConnection>& _rxConn,
            sal_Bool _bAlloweDefault,
            const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    // ask the parent of the connection (should be an DatabaseAccess)
    staruno::Reference<starutil::XNumberFormatsSupplier> xReturn;
    staruno::Reference<starcontainer::XChild> xConnAsChild(_rxConn, staruno::UNO_QUERY);
    ::rtl::OUString sPropFormatsSupplier = ::rtl::OUString::createFromAscii("NumberFormatsSupplier");
    if (xConnAsChild.is())
    {
        staruno::Reference<starbeans::XPropertySet> xConnParentProps(xConnAsChild->getParent(), staruno::UNO_QUERY);
        if (xConnParentProps.is() && hasProperty(sPropFormatsSupplier, xConnParentProps))
        {
            staruno::Any aSupplier( xConnParentProps->getPropertyValue(sPropFormatsSupplier) );
            if (aSupplier.getValueType().getTypeClass() == staruno::TypeClass_INTERFACE)
                xReturn = staruno::Reference<starutil::XNumberFormatsSupplier>(*(staruno::Reference< staruno::XInterface >*)aSupplier.getValue(), staruno::UNO_QUERY);
        }
    }

    // no result 'til now ? -> ask the environment for the default formatter
    if (!xReturn.is() && _bAlloweDefault)
    {
        staruno::Reference<starsdb::XDatabaseEnvironment> xEnv(_rxFactory->createInstance(
            ::rtl::OUString::createFromAscii("com.sun.star.sdb.DatabaseEnvironment")),
            staruno::UNO_QUERY);

        staruno::Reference<starbeans::XPropertySet> xEnvProps(xEnv, staruno::UNO_QUERY);
        if (xEnv.is() && hasProperty(sPropFormatsSupplier, xEnvProps))
        {
            staruno::Any aSupplier( xEnvProps->getPropertyValue(sPropFormatsSupplier) );
            if (aSupplier.getValueType().getTypeClass() == staruno::TypeClass_INTERFACE)
                xReturn = staruno::Reference<starutil::XNumberFormatsSupplier>(*(staruno::Reference< staruno::XInterface >*)aSupplier.getValue(), staruno::UNO_QUERY);
        }
    }

    return xReturn;
}

//==============================================================================
//------------------------------------------------------------------------------
void TransferFormComponentProperties(
            const staruno::Reference<starbeans::XPropertySet>& xOldProps,
            const staruno::Reference<starbeans::XPropertySet>& xNewProps,
            const starlang::Locale& _rLocale)
{
    // kopieren wir erst mal alle Props, die in Quelle und Ziel vorhanden sind und identische Beschreibungen haben
    staruno::Reference<starbeans::XPropertySetInfo> xOldInfo( xOldProps->getPropertySetInfo());
    staruno::Reference<starbeans::XPropertySetInfo> xNewInfo( xNewProps->getPropertySetInfo());

    staruno::Sequence<starbeans::Property> aOldProperties = xOldInfo->getProperties();
    staruno::Sequence<starbeans::Property> aNewProperties = xNewInfo->getProperties();
    int nNewLen = aNewProperties.getLength();

    starbeans::Property* pOldProps = aOldProperties.getArray();
    starbeans::Property* pNewProps = aNewProperties.getArray();

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
            starbeans::Property* pResult = (starbeans::Property*) bsearch(pOldProps + i, (void*)pNewProps, nNewLen, sizeof(starbeans::Property),
                &PropertyCompare);
            if (pResult && (pResult->Attributes == pOldProps[i].Attributes)
                && ((pResult->Attributes & starbeans::PropertyAttribute::READONLY) == 0)
                && (pResult->Type.equals(pOldProps[i].Type)))
            {   // Attribute stimmen ueberein und Property ist nicht read-only
                try
                {
                    xNewProps->setPropertyValue(pResult->Name, xOldProps->getPropertyValue(pResult->Name));
                }
                catch(starlang::IllegalArgumentException& e)
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
        staruno::Any aFormatKey( xOldProps->getPropertyValue(sPropFormatKey) );
        if (aFormatKey.hasValue())
        {
            staruno::Any aSupplier( xOldProps->getPropertyValue(sPropFormatsSupplier) );
            OSL_ENSHURE(isAReference(aSupplier.getValueType(), static_cast<starutil::XNumberFormatsSupplier*>(NULL)),
                "TransferFormComponentProperties : invalid property type !");
            staruno::Reference<starutil::XNumberFormatsSupplier> xSupplier;
            if (aSupplier.hasValue())
                xSupplier = *(staruno::Reference<starutil::XNumberFormatsSupplier>*)aSupplier.getValue();
            if (xSupplier.is())
            {
                staruno::Reference<starutil::XNumberFormats> xFormats(xSupplier->getNumberFormats());
                staruno::Reference<starbeans::XPropertySet> xFormat(xFormats->getByKey(getINT32(aFormatKey)));
                if (hasProperty(sPropCurrencySymbol, xFormat))
                {
                    staruno::Any aVal( xFormat->getPropertyValue(sPropCurrencySymbol) );
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
        staruno::Any aEffectiveMin( xOldProps->getPropertyValue(sPropEffectiveMin) );
        if (aEffectiveMin.hasValue())
        {   // im Gegensatz zu ValueMin kann EffectiveMin void sein
            if (hasProperty(sPropValueMin, xNewProps))
            {
                OSL_ENSHURE(aEffectiveMin.getValueType().getTypeClass() == staruno::TypeClass_DOUBLE,
                    "TransferFormComponentProperties : invalid property type !");
                xNewProps->setPropertyValue(sPropValueMin, aEffectiveMin);
            }
        }
        staruno::Any aEffectiveMax( xOldProps->getPropertyValue(sPropEffectiveMax) );
        if (aEffectiveMax.hasValue())
        {   // analog
            if (hasProperty(sPropValueMax, xNewProps))
            {
                OSL_ENSHURE(aEffectiveMax.getValueType().getTypeClass() == staruno::TypeClass_DOUBLE,
                    "TransferFormComponentProperties : invalid property type !");
                xNewProps->setPropertyValue(sPropValueMax, aEffectiveMax);
            }
        }

        // dann koennen wir noch Default-Werte konvertieren und uebernehmen
        staruno::Any aEffectiveDefault( xOldProps->getPropertyValue(sPropEffectiveDefault) );
        if (aEffectiveDefault.hasValue())
        {
            sal_Bool bIsString = aEffectiveDefault.getValueType().getTypeClass() == staruno::TypeClass_STRING;
            OSL_ENSHURE(bIsString || aEffectiveDefault.getValueType().getTypeClass() == staruno::TypeClass_DOUBLE,
                "TransferFormComponentProperties : invalid property type !");
                // die Effective-Properties sollten immer void oder string oder double sein ....

            if (hasProperty(sPropDefaultDate, xNewProps) && !bIsString)
            {   // (einen String in ein Datum zu konvertieren muss nicht immer klappen, denn das ganze kann ja an
                // eine Textspalte gebunden gewesen sein, aber mit einem double koennen wir was anfangen)
                starutil::Date aDate = DBTypeConversion::toDate(getDouble(aEffectiveDefault));
                xNewProps->setPropertyValue(sPropDefaultDate, staruno::makeAny(aDate));
            }

            if (hasProperty(sPropDefaultTime, xNewProps) && !bIsString)
            {   // voellig analog mit Zeit
                starutil::Time aTime = DBTypeConversion::toTime(getDouble(aEffectiveDefault));
                xNewProps->setPropertyValue(sPropDefaultTime, staruno::makeAny(aTime));
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
        staruno::Any aSupplier( xNewProps->getPropertyValue(sPropFormatsSupplier) );
        OSL_ENSHURE(isAReference(aSupplier, static_cast<starutil::XNumberFormatsSupplier*>(NULL)),
            "TransferFormComponentProperties : invalid property type !");
        staruno::Reference<starutil::XNumberFormatsSupplier> xSupplier;
        if (aSupplier.hasValue())
            xSupplier = *(staruno::Reference<starutil::XNumberFormatsSupplier>*)aSupplier.getValue();
        if (xSupplier.is())
        {
            staruno::Reference<starutil::XNumberFormats> xFormats(xSupplier->getNumberFormats());

            // Sprache des neuen Formats
//          ::rtl::OString sLanguage, sCountry;
//          ConvertLanguageToIsoNames(Application::GetAppInternational().GetLanguage(), String(sLanguage.getStr()), String(sCountry.getStr()));
//          starlang::Locale aNewLanguage(
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
                staruno::Reference<starutil::XNumberFormatTypes> xTypeList(xFormats, staruno::UNO_QUERY);
                if (xTypeList.is())
                {
                    sal_Int16 nClassId;
                    xOldProps->getPropertyValue(sPropClassId) >>= nClassId;
                    switch (nClassId)
                    {
                        case starform::FormComponentType::DATEFIELD :
                            nBaseKey = xTypeList->getStandardFormat(starutil::NumberFormat::DATE, _rLocale);
                            break;

                        case starform::FormComponentType::TIMEFIELD :
                            nBaseKey = xTypeList->getStandardFormat(starutil::NumberFormat::TIME, _rLocale);
                            break;

                        case starform::FormComponentType::CURRENCYFIELD :
                            nBaseKey = xTypeList->getStandardFormat(starutil::NumberFormat::CURRENCY, _rLocale);
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

            xNewProps->setPropertyValue(sPropFormatKey, staruno::makeAny((sal_Int32)nKey));
        }

        // min-/max-Werte
        staruno::Any aNewMin, aNewMax;
        if (hasProperty(sPropValueMin, xOldProps))
            aNewMin = xOldProps->getPropertyValue(sPropValueMin);
        if (hasProperty(sPropValueMax, xOldProps))
            aNewMax = xOldProps->getPropertyValue(sPropValueMax);
        xNewProps->setPropertyValue(sPropEffectiveMin, aNewMin);
        xNewProps->setPropertyValue(sPropEffectiveMax, aNewMax);

        // Default-Wert
        staruno::Any aNewDefault;
        if (hasProperty(sPropDefaultDate, xOldProps))
        {
            staruno::Any aDate( xOldProps->getPropertyValue(sPropDefaultDate) );
            if (aDate.hasValue())
                aNewDefault <<= DBTypeConversion::toDouble(*(starutil::Date*)aDate.getValue());
        }

        if (hasProperty(sPropDefaultTime, xOldProps))
        {
            staruno::Any aTime( xOldProps->getPropertyValue(sPropDefaultTime) );
            if (aTime.hasValue())
                aNewDefault <<= DBTypeConversion::toDouble(*(starutil::Time*)aTime.getValue());
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
sal_Bool canInsert(const staruno::Reference< starbeans::XPropertySet>& _rxCursorSet)
{
    return ((_rxCursorSet.is() && (getINT32(_rxCursorSet->getPropertyValue(::rtl::OUString::createFromAscii("Privileges"))) & starsdbcx::Privilege::INSERT) != 0));
}

//------------------------------------------------------------------------------
sal_Bool canUpdate(const staruno::Reference< starbeans::XPropertySet>& _rxCursorSet)
{
    return ((_rxCursorSet.is() && (getINT32(_rxCursorSet->getPropertyValue(::rtl::OUString::createFromAscii("Privileges"))) & starsdbcx::Privilege::UPDATE) != 0));
}

//------------------------------------------------------------------------------
sal_Bool canDelete(const staruno::Reference< starbeans::XPropertySet>& _rxCursorSet)
{
    return ((_rxCursorSet.is() && (getINT32(_rxCursorSet->getPropertyValue(::rtl::OUString::createFromAscii("Privileges"))) & starsdbcx::Privilege::DELETE) != 0));
}


//------------------------------------------------------------------------------
staruno::Reference<starsdb::XSQLQueryComposer> getCurrentSettingsComposer(
                const staruno::Reference<starbeans::XPropertySet>& _rxRowSetProps,
                const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    staruno::Reference<starsdb::XSQLQueryComposer> xReturn;
    staruno::Reference<starsdbc::XRowSet> xRowSet(_rxRowSetProps, staruno::UNO_QUERY);
    staruno::Reference<starsdbc::XConnection> xConn( calcConnection(xRowSet, _rxFactory));
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
                    case starsdb::CommandType::COMMAND:
                        if (!bEscapeProcessing)
                        {   // native sql -> no parsable statement
                            sStatement = ::rtl::OUString();
                        }
                        else
                        {
                            sStatement = sCommand;
                        }
                        break;
                    case starsdb::CommandType::TABLE:
                    {
                        if (!sCommand.getLength())
                            break;

                        ::rtl::OUString sTableName = quoteTableName(xConn->getMetaData(), sCommand);
                        sStatement = ::rtl::OUString::createFromAscii("SELECT * FROM ");
                        sStatement += sTableName;
                    }
                    break;
                    case starsdb::CommandType::QUERY:
                    {
                        // ask the connection for the query
                        staruno::Reference<starsdb::XQueriesSupplier> xSupplyQueries(xConn, staruno::UNO_QUERY);
                        if (!xSupplyQueries.is())
                            break;

                        staruno::Reference<starcontainer::XNameAccess> xQueries(xSupplyQueries->getQueries(), staruno::UNO_QUERY);
                        if (!xQueries.is() || !xQueries->hasByName(sCommand))
                            break;

                        staruno::Reference<starbeans::XPropertySet> xQueryProps(*(staruno::Reference<staruno::XInterface>*)xQueries->getByName(sCommand).getValue(), staruno::UNO_QUERY);
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
                        staruno::Reference<starsdb::XSQLQueryComposerFactory> xFactory(xConn, staruno::UNO_QUERY);
                        staruno::Reference<starsdb::XSQLQueryComposer> xLocalComposer;
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
                staruno::Reference<starsdb::XSQLQueryComposerFactory> xFactory(xConn, staruno::UNO_QUERY);
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

//.........................................................................
}   // namespace dbtools
//.........................................................................


/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 29.09.00 08:16:59  fs
 ************************************************************************/

