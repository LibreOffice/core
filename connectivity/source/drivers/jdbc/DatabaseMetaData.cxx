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

#include <sal/macros.h>
#include "java/sql/DatabaseMetaData.hxx"
#include "java/sql/Connection.hxx"
#include "java/sql/ResultSet.hxx"
#include "java/tools.hxx"
#include "java/lang/String.hxx"
#include <connectivity/CommonTools.hxx>
#include "FDatabaseMetaDataResultSet.hxx"
#include <comphelper/types.hxx>
#include "TPrivilegesResultSet.hxx"
#include "resource/jdbc_log.hrc"

using namespace ::comphelper;

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

jclass java_sql_DatabaseMetaData::theClass              = nullptr;

java_sql_DatabaseMetaData::~java_sql_DatabaseMetaData()
{
    SDBThreadAttach::releaseRef();
}

jclass java_sql_DatabaseMetaData::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/DatabaseMetaData");
    return theClass;
}

java_sql_DatabaseMetaData::java_sql_DatabaseMetaData( JNIEnv * pEnv, jobject myObj, java_sql_Connection& _rConnection )
    :ODatabaseMetaDataBase( &_rConnection,_rConnection.getConnectionInfo() )
    ,java_lang_Object( pEnv, myObj )
    ,m_pConnection( &_rConnection )
    ,m_aLogger( _rConnection.getLogger() )
{
    SDBThreadAttach::addRef();
}


Reference< XResultSet > java_sql_DatabaseMetaData::impl_getTypeInfo_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callResultSetMethod( "getTypeInfo", mID );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getCatalogs(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callResultSetMethod( "getCatalogs", mID );
}

OUString java_sql_DatabaseMetaData::impl_getCatalogSeparator_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getCatalogSeparator", mID );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getSchemas(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callResultSetMethod( "getSchemas", mID );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getColumnPrivileges(
        const Any& catalog, const OUString& schema, const OUString& table, const OUString& columnNamePattern ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callResultSetMethodWithStrings( "getColumnPrivileges", mID, catalog, schema, table, &columnNamePattern );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getColumns(
        const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const OUString& columnNamePattern ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callResultSetMethodWithStrings( "getColumns", mID, catalog, schemaPattern, tableNamePattern, &columnNamePattern );
}


Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getTables(
        const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const Sequence< OUString >& _types ) throw(SQLException, RuntimeException, std::exception)
{
    static const char * cMethodName = "getTables";

    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD, cMethodName );

    jobject out(nullptr);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)Ljava/sql/ResultSet;";
        // execute Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        OSL_VERIFY( !isExceptionOccurred(t.pEnv, true) );
        jvalue args[4];

        args[3].l = nullptr;
        sal_Int32 typeFilterCount = _types.getLength();
        if ( typeFilterCount )
        {
            jobjectArray pObjArray = t.pEnv->NewObjectArray( (jsize)typeFilterCount, java_lang_String::st_getMyClass(), nullptr );
            OSL_VERIFY( !isExceptionOccurred( t.pEnv, true ) );
            const OUString* typeFilter = _types.getConstArray();
            bool bIncludeAllTypes = false;
            for ( sal_Int32 i=0; i<typeFilterCount; ++i, ++typeFilter )
            {
                if ( *typeFilter == "%" )
                {
                    bIncludeAllTypes = true;
                    break;
                }
                jstring aT = convertwchar_tToJavaString( t.pEnv, *typeFilter );
                t.pEnv->SetObjectArrayElement( pObjArray, (jsize)i, aT );
                OSL_VERIFY( !isExceptionOccurred( t.pEnv, true ) );
            }

            if ( bIncludeAllTypes )
            {
                // the SDBC API allows to pass "%" as table type filter, but in JDBC, "all table types"
                // is represented by the table type being <null/>
                t.pEnv->DeleteLocalRef( pObjArray );
                OSL_VERIFY( !isExceptionOccurred( t.pEnv, true ) );
            }
            else
            {
                args[3].l = pObjArray;
            }
        }
        // if we are to display "all catalogs", then respect m_aCatalogRestriction
        Any aCatalogFilter( catalog );
        if ( !aCatalogFilter.hasValue() )
            aCatalogFilter = m_pConnection->getCatalogRestriction();
        // similar for schema
        Any aSchemaFilter;
        if ( schemaPattern == "%" )
            aSchemaFilter = m_pConnection->getSchemaRestriction();
        else
            aSchemaFilter <<= schemaPattern;

        args[0].l = aCatalogFilter.hasValue() ? convertwchar_tToJavaString( t.pEnv, ::comphelper::getString( aCatalogFilter ) ) : nullptr;
        args[1].l = aSchemaFilter.hasValue() ? convertwchar_tToJavaString( t.pEnv, ::comphelper::getString( aSchemaFilter ) ) : nullptr;
        args[2].l = convertwchar_tToJavaString(t.pEnv,tableNamePattern);
        out = t.pEnv->CallObjectMethod( object, mID, args[0].l, args[1].l,args[2].l,args[3].l);
        jthrowable jThrow = t.pEnv->ExceptionOccurred();
        if ( jThrow )
            t.pEnv->ExceptionClear();// we have to clear the exception here because we want to handle it below
        if ( aCatalogFilter.hasValue() )
        {
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[0].l));
            OSL_VERIFY( !isExceptionOccurred( t.pEnv, true ) );
        }
        if(args[1].l)
        {
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[1].l));
            OSL_VERIFY( !isExceptionOccurred( t.pEnv, true ) );
        }
        if(!tableNamePattern.isEmpty())
        {
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[2].l));
            OSL_VERIFY( !isExceptionOccurred( t.pEnv, true ) );
        }
        //for(INT16 i=0;i<len;i++)
        if ( args[3].l )
        {
            t.pEnv->DeleteLocalRef( static_cast<jobjectArray>(args[3].l) );
            OSL_VERIFY( !isExceptionOccurred( t.pEnv, true ) );
        }

        if ( jThrow )
        {
            if ( t.pEnv->IsInstanceOf( jThrow,java_sql_SQLException_BASE::st_getMyClass() ) )
            {
                java_sql_SQLException_BASE* pException = new java_sql_SQLException_BASE( t.pEnv, jThrow );
                SQLException e( pException->getMessage(),
                                    *this,
                                    pException->getSQLState(),
                                    pException->getErrorCode(),
                                    Any()
                                );
                delete pException;
                throw  e;
            }
        }
    }

    if ( !out )
        return nullptr;

    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_SUCCESS, cMethodName );
    return new java_sql_ResultSet( t.pEnv, out, m_aLogger,*m_pConnection);
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getProcedureColumns(
        const Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern, const OUString& columnNamePattern ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callResultSetMethodWithStrings( "getProcedureColumns", mID, catalog, schemaPattern, procedureNamePattern, &columnNamePattern );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getProcedures( const Any&
                catalog, const OUString& schemaPattern, const OUString& procedureNamePattern ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callResultSetMethodWithStrings( "getProcedures", mID, catalog, schemaPattern, procedureNamePattern );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getVersionColumns(
        const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callResultSetMethodWithStrings( "getVersionColumns", mID, catalog, schema, table );
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxBinaryLiteralLength", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxRowSize(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxRowSize", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxCatalogNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxCatalogNameLength", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxCharLiteralLength", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxColumnNameLength", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxColumnsInIndex", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxCursorNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxCursorNameLength", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxConnections(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxConnections", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxColumnsInTable", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxStatementLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxStatementLength", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxTableNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxTableNameLength", mID);
}

sal_Int32 java_sql_DatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxTablesInSelect", mID);
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getExportedKeys(
        const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callResultSetMethodWithStrings( "getExportedKeys", mID, catalog, schema, table );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getImportedKeys(
        const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callResultSetMethodWithStrings( "getImportedKeys", mID, catalog, schema, table );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getPrimaryKeys(
        const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callResultSetMethodWithStrings( "getPrimaryKeys", mID, catalog, schema, table );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getIndexInfo(
        const Any& catalog, const OUString& schema, const OUString& table,
        sal_Bool unique, sal_Bool approximate ) throw(SQLException, RuntimeException, std::exception)
{
    static const char * cMethodName = "getIndexInfo";

    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD, cMethodName );

    jobject out(nullptr);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZ)Ljava/sql/ResultSet;";
        // execute Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        jvalue args[5];
        // convert Parameter
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : nullptr;
        args[1].l = schema.toChar() == '%' ? nullptr : convertwchar_tToJavaString(t.pEnv,schema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,table);
        args[3].z = unique;
        args[4].z = approximate;
        out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l,args[3].z,args[4].z );

        // and clean up
        if(catalog.hasValue())
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[0].l));
        if(args[1].l)
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[1].l));
        if(!table.isEmpty())
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[2].l));
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    }
    if ( !out )
        return nullptr;

    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_SUCCESS, cMethodName );
    return new java_sql_ResultSet( t.pEnv, out, m_aLogger,*m_pConnection);
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getBestRowIdentifier(
        const Any& catalog, const OUString& schema, const OUString& table, sal_Int32 scope,
        sal_Bool nullable ) throw(SQLException, RuntimeException, std::exception)
{
    static const char * cMethodName = "getBestRowIdentifier";

    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD, cMethodName );

    jobject out(nullptr);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IZ)Ljava/sql/ResultSet;";
        // execute Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        jvalue args[3];
        // convert Parameter
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : nullptr;
        args[1].l = schema.toChar() == '%' ? nullptr : convertwchar_tToJavaString(t.pEnv,schema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,table);
        out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[1].l,args[2].l,scope,nullable);

        // and cleanup
        if(catalog.hasValue())
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[0].l));
        if(args[1].l)
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[1].l));
        if(!table.isEmpty())
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[2].l));
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    }

    if ( !out )
        return nullptr;

    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_SUCCESS, cMethodName );
    return new java_sql_ResultSet( t.pEnv, out, m_aLogger,*m_pConnection);
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getTablePrivileges(
        const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern ) throw(SQLException, RuntimeException, std::exception)
{
    if ( m_pConnection->isIgnoreDriverPrivilegesEnabled() )
        return new OResultSetPrivileges(this,catalog,schemaPattern,tableNamePattern);

    static jmethodID mID(nullptr);
    Reference< XResultSet > xReturn( impl_callResultSetMethodWithStrings( "getTablePrivileges", mID, catalog, schemaPattern, tableNamePattern ) );

    if ( xReturn.is() )
    {
        // we have to check the result columns for the tables privileges
        Reference< XResultSetMetaDataSupplier > xMetaSup(xReturn,UNO_QUERY);
        if ( xMetaSup.is() )
        {
            Reference< XResultSetMetaData> xMeta = xMetaSup->getMetaData();
            if ( xMeta.is() && xMeta->getColumnCount() != 7 )
            {
                // here we know that the count of column doesn't match
                ::std::map<sal_Int32,sal_Int32> aColumnMatching;
                static const OUStringLiteral sPrivs[] = {
                                            OUStringLiteral("TABLE_CAT"),
                                            OUStringLiteral("TABLE_SCHEM"),
                                            OUStringLiteral("TABLE_NAME"),
                                            OUStringLiteral("GRANTOR"),
                                            OUStringLiteral("GRANTEE"),
                                            OUStringLiteral("PRIVILEGE"),
                                            OUStringLiteral("IS_GRANTABLE")
                                        };

                OUString sColumnName;
                sal_Int32 nCount = xMeta->getColumnCount();
                for (sal_Int32 i = 1 ; i <= nCount ; ++i)
                {
                    sColumnName = xMeta->getColumnName(i);
                    for (sal_uInt32 j = 0 ; j < SAL_N_ELEMENTS(sPrivs); ++j)
                    {
                        if ( sPrivs[j] == sColumnName )
                        {
                            aColumnMatching.insert( ::std::map<sal_Int32,sal_Int32>::value_type(i,j+1) );
                            break;
                        }
                    }

                }
                // fill our own resultset
                ODatabaseMetaDataResultSet* pNewPrivRes = new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTablePrivileges );
                Reference< XResultSet > xTemp = xReturn;
                xReturn = pNewPrivRes;
                ODatabaseMetaDataResultSet::ORows aRows;
                Reference< XRow > xRow(xTemp,UNO_QUERY);
                OUString sValue;

                ODatabaseMetaDataResultSet::ORow aRow(8);
                while ( xRow.is() && xTemp->next() )
                {
                    ::std::map<sal_Int32,sal_Int32>::const_iterator aIter = aColumnMatching.begin();
                    ::std::map<sal_Int32,sal_Int32>::const_iterator aEnd  = aColumnMatching.end();
                    for (;aIter != aEnd ; ++aIter)
                    {
                        sValue = xRow->getString(aIter->first);
                        if ( xRow->wasNull() )
                            aRow[aIter->second] = ODatabaseMetaDataResultSet::getEmptyValue();
                        else
                            aRow[aIter->second] = new ORowSetValueDecorator(sValue);
                    }

                    aRows.push_back(aRow);
                }
                pNewPrivRes->setRows(aRows);
            }
        }
    }
    return xReturn;
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getCrossReference(
        const Any& primaryCatalog, const OUString& primarySchema,
        const OUString& primaryTable, const Any& foreignCatalog,
        const OUString& foreignSchema, const OUString& foreignTable ) throw(SQLException, RuntimeException, std::exception)
{
    static const char * cMethodName = "getCrossReference";
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD, cMethodName );

    jobject out(nullptr);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        // execute Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        jvalue args[6];
        // convert Parameter
        args[0].l = primaryCatalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(primaryCatalog)) : nullptr;
        args[1].l = primarySchema.toChar() == '%' ? nullptr : convertwchar_tToJavaString(t.pEnv,primarySchema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,primaryTable);
        args[3].l = foreignCatalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(foreignCatalog)) : nullptr;
        args[4].l = foreignSchema.toChar() == '%' ? nullptr : convertwchar_tToJavaString(t.pEnv,foreignSchema);
        args[5].l = convertwchar_tToJavaString(t.pEnv,foreignTable);
        out = t.pEnv->CallObjectMethod( object, mID, args[0].l,args[2].l,args[2].l,args[3].l,args[4].l,args[5].l );

        // and clean up
        if(primaryCatalog.hasValue())
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[0].l));
        if(args[1].l)
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[1].l));
        if(!primaryTable.isEmpty())
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[2].l));
        if(foreignCatalog.hasValue())
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[3].l));
        if(args[4].l)
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[4].l));
        if(!foreignTable.isEmpty())
            t.pEnv->DeleteLocalRef(static_cast<jstring>(args[5].l));
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    }

    if ( !out )
        return nullptr;

    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_SUCCESS, cMethodName );
    return new java_sql_ResultSet( t.pEnv, out, m_aLogger,*m_pConnection);
}


bool java_sql_DatabaseMetaData::impl_callBooleanMethod( const char* _pMethodName, jmethodID& _inout_MethodID )
{
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD, _pMethodName );
    jboolean out( java_lang_Object::callBooleanMethod(_pMethodName,_inout_MethodID) );
    m_aLogger.log< const sal_Char*, bool>( LogLevel::FINEST, STR_LOG_META_DATA_RESULT, _pMethodName, out );
    return out;
}


OUString java_sql_DatabaseMetaData::impl_callStringMethod( const char* _pMethodName, jmethodID& _inout_MethodID )
{
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD, _pMethodName );

    const OUString sReturn( callStringMethod(_pMethodName,_inout_MethodID) );
    if ( m_aLogger.isLoggable( LogLevel::FINEST ) )
    {
        OUString sLoggedResult( sReturn );
        if ( sLoggedResult.isEmpty() )
            sLoggedResult = "<empty string>";
        m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_RESULT, _pMethodName, sLoggedResult );
    }

    return sReturn;
}

sal_Int32 java_sql_DatabaseMetaData::impl_callIntMethod_ThrowSQL(const char* _pMethodName, jmethodID& _inout_MethodID)
{
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD, _pMethodName );
    sal_Int32 out( (sal_Int32)callIntMethod_ThrowSQL(_pMethodName,_inout_MethodID) );
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_RESULT, _pMethodName, (sal_Int32)out );
    return out;
}

sal_Int32 java_sql_DatabaseMetaData::impl_callIntMethod_ThrowRuntime(const char* _pMethodName, jmethodID& _inout_MethodID)
{
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD, _pMethodName );
    sal_Int32 out( (sal_Int32)callIntMethod_ThrowRuntime(_pMethodName,_inout_MethodID) );
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_RESULT, _pMethodName, (sal_Int32)out );
    return out;
}

bool java_sql_DatabaseMetaData::impl_callBooleanMethodWithIntArg( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument )
{
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD_ARG1, _pMethodName, _nArgument );

    jboolean out( callBooleanMethodWithIntArg(_pMethodName,_inout_MethodID,_nArgument) );

    m_aLogger.log< const sal_Char*, bool >( LogLevel::FINEST, STR_LOG_META_DATA_RESULT, _pMethodName, out );
    return out;
}


Reference< XResultSet > java_sql_DatabaseMetaData::impl_callResultSetMethod( const char* _pMethodName, jmethodID& _inout_MethodID )
{
    SDBThreadAttach t;
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD, _pMethodName );
    jobject out(callResultSetMethod(t.env(),_pMethodName,_inout_MethodID));
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_SUCCESS, _pMethodName );
    return new java_sql_ResultSet( t.pEnv, out, m_aLogger,*m_pConnection);
}


Reference< XResultSet > java_sql_DatabaseMetaData::impl_callResultSetMethodWithStrings( const char* _pMethodName, jmethodID& _inout_MethodID,
    const Any& _rCatalog, const OUString& _rSchemaPattern, const OUString& _rLeastPattern,
    const OUString* _pOptionalAdditionalString )
{
    bool bCatalog = _rCatalog.hasValue();
    OUString sCatalog;
    _rCatalog >>= sCatalog;

    bool bSchema = _rSchemaPattern.toChar() != '%';

    // log the call
    if ( m_aLogger.isLoggable( LogLevel::FINEST ) )
    {
        OUString sCatalogLog = bCatalog ? sCatalog : OUString( "null" );
        OUString sSchemaLog = bSchema ? _rSchemaPattern : OUString( "null" );
        if ( _pOptionalAdditionalString )
            m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD_ARG4, _pMethodName, sCatalogLog, sSchemaLog, _rLeastPattern, *_pOptionalAdditionalString );
        else
            m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD_ARG3, _pMethodName, sCatalogLog, sSchemaLog, _rLeastPattern );
    }

    jobject out(nullptr);

    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_sql_DatabaseMetaData::impl_callResultSetMethodWithStrings: no Java environment anymore!" );

    {
        const  char* pSignature = _pOptionalAdditionalString
            ?   "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;"
            :   "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/sql/ResultSet;";
        // obtain method ID
        obtainMethodId_throwSQL(t.pEnv, _pMethodName,pSignature, _inout_MethodID);

        // call method

        {
            jvalue args[4];
            // convert parameters
            args[0].l = bCatalog ? convertwchar_tToJavaString( t.pEnv, sCatalog ) : nullptr;
            args[1].l = bSchema ? convertwchar_tToJavaString( t.pEnv, _rSchemaPattern ) : nullptr;
            args[2].l = convertwchar_tToJavaString( t.pEnv, _rLeastPattern );
            args[3].l = _pOptionalAdditionalString ? convertwchar_tToJavaString( t.pEnv, *_pOptionalAdditionalString ) : nullptr;

            // actually do the call
            if ( _pOptionalAdditionalString )
                out = t.pEnv->CallObjectMethod( object, _inout_MethodID, args[0].l, args[1].l, args[2].l, args[3].l );
            else
                out = t.pEnv->CallObjectMethod( object, _inout_MethodID, args[0].l, args[1].l, args[2].l );

            // clean up
            if ( args[0].l )
                t.pEnv->DeleteLocalRef( static_cast<jstring>(args[0].l) );
            if ( args[1].l )
                t.pEnv->DeleteLocalRef( static_cast<jstring>(args[1].l) );
            if ( args[2].l )
                t.pEnv->DeleteLocalRef( static_cast<jstring>(args[2].l) );
            if ( args[3].l )
                t.pEnv->DeleteLocalRef( static_cast<jstring>(args[3].l) );

            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    }

    if ( !out )
        return nullptr;

    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_SUCCESS, _pMethodName );
    return new java_sql_ResultSet( t.pEnv, out, m_aLogger,*m_pConnection);
}


sal_Bool SAL_CALL java_sql_DatabaseMetaData::doesMaxRowSizeIncludeBlobs(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "doesMaxRowSizeIncludeBlobs", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesLowerCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "storesLowerCaseQuotedIdentifiers", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesLowerCaseIdentifiers(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "storesLowerCaseIdentifiers", mID );
}

bool java_sql_DatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "storesMixedCaseQuotedIdentifiers", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "storesMixedCaseIdentifiers", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesUpperCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "storesUpperCaseQuotedIdentifiers", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesUpperCaseIdentifiers(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "storesUpperCaseIdentifiers", mID );
}

bool java_sql_DatabaseMetaData::impl_supportsAlterTableWithAddColumn_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsAlterTableWithAddColumn", mID );
}

bool java_sql_DatabaseMetaData::impl_supportsAlterTableWithDropColumn_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsAlterTableWithDropColumn", mID );
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxIndexLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxIndexLength", mID);
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsNonNullableColumns(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsNonNullableColumns", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getCatalogTerm(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getCatalogTerm", mID );
}

OUString java_sql_DatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getIdentifierQuoteString", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getExtraNameCharacters", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsDifferentTableCorrelationNames(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsDifferentTableCorrelationNames", mID );
}

bool java_sql_DatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "isCatalogAtStart", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::dataDefinitionIgnoredInTransactions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "dataDefinitionIgnoredInTransactions", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::dataDefinitionCausesTransactionCommit(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "dataDefinitionCausesTransactionCommit", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsDataManipulationTransactionsOnly(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsDataManipulationTransactionsOnly", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsDataDefinitionAndDataManipulationTransactions", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsPositionedDelete(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsPositionedDelete", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsPositionedUpdate(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsPositionedUpdate", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOpenStatementsAcrossRollback(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsOpenStatementsAcrossRollback", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOpenStatementsAcrossCommit(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsOpenStatementsAcrossCommit", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOpenCursorsAcrossCommit(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsOpenCursorsAcrossCommit", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOpenCursorsAcrossRollback(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsOpenCursorsAcrossRollback", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 level ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethodWithIntArg( "supportsTransactionIsolationLevel", mID, level );
}

bool java_sql_DatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsSchemasInDataManipulation", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsANSI92FullSQL(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsANSI92FullSQL", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsANSI92EntryLevelSQL(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsANSI92EntryLevelSQL", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsIntegrityEnhancementFacility", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSchemasInIndexDefinitions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsSchemasInIndexDefinitions", mID );
}

bool java_sql_DatabaseMetaData::impl_supportsSchemasInTableDefinitions_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsSchemasInTableDefinitions", mID );
}

bool java_sql_DatabaseMetaData::impl_supportsCatalogsInTableDefinitions_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsCatalogsInTableDefinitions", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCatalogsInIndexDefinitions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsCatalogsInIndexDefinitions", mID );
}

bool java_sql_DatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsCatalogsInDataManipulation", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOuterJoins(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsOuterJoins", mID );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getTableTypes(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callResultSetMethod( "getTableTypes", mID );
}

sal_Int32 java_sql_DatabaseMetaData::impl_getMaxStatements_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxStatements", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxProcedureNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxProcedureNameLength", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxSchemaNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxSchemaNameLength", mID);
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsTransactions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsTransactions", mID );
}


sal_Bool SAL_CALL java_sql_DatabaseMetaData::allProceduresAreCallable(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "allProceduresAreCallable", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsStoredProcedures(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsStoredProcedures", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSelectForUpdate(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsSelectForUpdate", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::allTablesAreSelectable(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "allTablesAreSelectable", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::isReadOnly(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "isReadOnly", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::usesLocalFiles(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "usesLocalFiles", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::usesLocalFilePerTable(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "usesLocalFilePerTable", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsTypeConversion(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsTypeConversion", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullPlusNonNullIsNull(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "nullPlusNonNullIsNull", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsColumnAliasing(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsColumnAliasing", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsTableCorrelationNames(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsTableCorrelationNames", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsConvert( sal_Int32 fromType, sal_Int32 toType ) throw(SQLException, RuntimeException, std::exception)
{
    static const char* pMethodName = "supportsConvert";
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD_ARG2, pMethodName, fromType, toType );

    jboolean out( false );
    SDBThreadAttach t;

    {
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, pMethodName,"(II)Z", mID);
        out = t.pEnv->CallBooleanMethod( object, mID, fromType, toType );
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    }

    m_aLogger.log< const sal_Char*, bool >( LogLevel::FINEST, STR_LOG_META_DATA_RESULT, pMethodName, out );
    return out;
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsExpressionsInOrderBy(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsExpressionsInOrderBy", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsGroupBy(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsGroupBy", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsGroupByBeyondSelect(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsGroupByBeyondSelect", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsGroupByUnrelated(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsGroupByUnrelated", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsMultipleTransactions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsMultipleTransactions", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsMultipleResultSets(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsMultipleResultSets", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsLikeEscapeClause(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsLikeEscapeClause", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOrderByUnrelated(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsOrderByUnrelated", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsUnion(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsUnion", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsUnionAll(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsUnionAll", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsMixedCaseIdentifiers", mID );
}

bool java_sql_DatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsMixedCaseQuotedIdentifiers", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullsAreSortedAtEnd(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "nullsAreSortedAtEnd", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullsAreSortedAtStart(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "nullsAreSortedAtStart", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullsAreSortedHigh(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "nullsAreSortedHigh", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullsAreSortedLow(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "nullsAreSortedLow", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSchemasInProcedureCalls(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsSchemasInProcedureCalls", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsSchemasInPrivilegeDefinitions", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCatalogsInProcedureCalls(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsCatalogsInProcedureCalls", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsCatalogsInPrivilegeDefinitions", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCorrelatedSubqueries(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsCorrelatedSubqueries", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSubqueriesInComparisons(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsSubqueriesInComparisons", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSubqueriesInExists(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsSubqueriesInExists", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSubqueriesInIns(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsSubqueriesInIns", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSubqueriesInQuantifieds(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsSubqueriesInQuantifieds", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsANSI92IntermediateSQL(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsANSI92IntermediateSQL", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException, std::exception)
{
    OUString sURL = m_pConnection->getURL();
    if ( sURL.isEmpty() )
    {
        static jmethodID mID(nullptr);
        sURL = impl_callStringMethod( "getURL", mID );
    }
    return sURL;
}

OUString SAL_CALL java_sql_DatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getUserName", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getDriverName", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getDriverVersion(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getDriverVersion", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getDatabaseProductVersion", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getDatabaseProductName(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getDatabaseProductName", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getProcedureTerm", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getSchemaTerm(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getSchemaTerm", mID );
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getDriverMajorVersion(  ) throw(RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowRuntime("getDriverMajorVersion", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getDefaultTransactionIsolation(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getDefaultTransactionIsolation", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getDriverMinorVersion(  ) throw(RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowRuntime("getDriverMinorVersion", mID);
}

OUString SAL_CALL java_sql_DatabaseMetaData::getSQLKeywords(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getSQLKeywords", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getSearchStringEscape", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getStringFunctions", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getTimeDateFunctions", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getSystemFunctions", mID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callStringMethod( "getNumericFunctions", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsExtendedSQLGrammar(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsExtendedSQLGrammar", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCoreSQLGrammar(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsCoreSQLGrammar", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsMinimumSQLGrammar(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsMinimumSQLGrammar", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsFullOuterJoins(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsFullOuterJoins", mID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsLimitedOuterJoins(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsLimitedOuterJoins", mID );
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInGroupBy(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxColumnsInGroupBy", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInOrderBy(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxColumnsInOrderBy", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInSelect(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxColumnsInSelect", mID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxUserNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxUserNameLength", mID);
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsResultSetType( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethodWithIntArg( "supportsResultSetType", mID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency ) throw(SQLException, RuntimeException, std::exception)
{
    static const char* pMethodName = "supportsResultSetConcurrency";
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD_ARG2, pMethodName, setType, concurrency );

    jboolean out( false );
    SDBThreadAttach t;

    {
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, pMethodName,"(II)Z", mID);
        out =   t.pEnv->CallBooleanMethod( object, mID, setType, concurrency);
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    }

    m_aLogger.log< const sal_Char*, bool >( LogLevel::FINEST, STR_LOG_META_DATA_RESULT, pMethodName, out );
    return out;
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::ownUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethodWithIntArg( "ownUpdatesAreVisible", mID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::ownDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethodWithIntArg( "ownDeletesAreVisible", mID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::ownInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethodWithIntArg( "ownInsertsAreVisible", mID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::othersUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethodWithIntArg( "othersUpdatesAreVisible", mID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::othersDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethodWithIntArg( "othersDeletesAreVisible", mID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::othersInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethodWithIntArg( "othersInsertsAreVisible", mID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::updatesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethodWithIntArg( "updatesAreDetected", mID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::deletesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethodWithIntArg( "deletesAreDetected", mID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::insertsAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethodWithIntArg( "insertsAreDetected", mID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsBatchUpdates(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return impl_callBooleanMethod( "supportsBatchUpdates", mID );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getUDTs(
        const Any& catalog, const OUString& schemaPattern, const OUString& typeNamePattern,
        const Sequence< sal_Int32 >& types ) throw(SQLException, RuntimeException, std::exception)
{
    jobject out(nullptr);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {


        static const char * cSignature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[I)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getUDTs";
        // dismiss Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        {
            jvalue args[4];
            // initialize temporary Variable
            args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : nullptr;
            args[1].l = schemaPattern.toChar() == '%' ? nullptr : convertwchar_tToJavaString(t.pEnv,schemaPattern);
            args[2].l = convertwchar_tToJavaString(t.pEnv,typeNamePattern);
            jintArray pArray = t.pEnv->NewIntArray(types.getLength());
            jint * typesData = reinterpret_cast<jint *>(
               const_cast<sal_Int32 *>(types.getConstArray()));
            // 4th param of Set*ArrayRegion changed from pointer to non-const to
            // pointer to const between <http://docs.oracle.com/javase/6/docs/
            // technotes/guides/jni/spec/functions.html#wp22933> and
            // <http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/
            // functions.html#wp22933>; work around that difference in a way
            // that doesn't trigger loplugin:redundantcast
            t.pEnv->SetIntArrayRegion(pArray,0,types.getLength(),typesData);
            args[3].l = pArray;

            out = t.pEnv->CallObjectMethod( object, mID, args[0].l, args[1].l,args[2].l,args[3].l);

            if(catalog.hasValue())
                t.pEnv->DeleteLocalRef(static_cast<jstring>(args[0].l));
            if(!schemaPattern.isEmpty())
                t.pEnv->DeleteLocalRef(static_cast<jstring>(args[1].l));
            if(!typeNamePattern.isEmpty())
                t.pEnv->DeleteLocalRef(static_cast<jstring>(args[2].l));
            if(args[3].l)
                t.pEnv->DeleteLocalRef(static_cast<jintArray>(args[3].l));
            ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
        }
    }

    return out ? new java_sql_ResultSet( t.pEnv, out, m_aLogger,*m_pConnection ) : nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
