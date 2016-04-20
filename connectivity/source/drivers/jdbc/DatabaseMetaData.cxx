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
    static jmethodID nID(nullptr);
    return impl_callResultSetMethod( "getTypeInfo", nID );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getCatalogs(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callResultSetMethod( "getCatalogs", nID );
}

OUString java_sql_DatabaseMetaData::impl_getCatalogSeparator_throw(  )
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getCatalogSeparator", nID );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getSchemas(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callResultSetMethod( "getSchemas", nID );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getColumnPrivileges(
        const Any& catalog, const OUString& schema, const OUString& table, const OUString& columnNamePattern ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callResultSetMethodWithStrings( "getColumnPrivileges", nID, catalog, schema, table, &columnNamePattern );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getColumns(
        const Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const OUString& columnNamePattern ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callResultSetMethodWithStrings( "getColumns", nID, catalog, schemaPattern, tableNamePattern, &columnNamePattern );
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
        static jmethodID nID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, nID);
        OSL_VERIFY( !isExceptionOccurred(t.pEnv, true) );
        jvalue args[4];

        args[3].l = nullptr;
        sal_Int32 typeFilterCount = _types.getLength();
        if ( typeFilterCount )
        {
            jobjectArray pObjArray = static_cast< jobjectArray >( t.pEnv->NewObjectArray( (jsize)typeFilterCount, java_lang_String::st_getMyClass(), nullptr ) );
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
        out = t.pEnv->CallObjectMethod( object, nID, args[0].l, args[1].l,args[2].l,args[3].l);
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
    static jmethodID nID(nullptr);
    return impl_callResultSetMethodWithStrings( "getProcedureColumns", nID, catalog, schemaPattern, procedureNamePattern, &columnNamePattern );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getProcedures( const Any&
                catalog, const OUString& schemaPattern, const OUString& procedureNamePattern ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callResultSetMethodWithStrings( "getProcedures", nID, catalog, schemaPattern, procedureNamePattern );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getVersionColumns(
        const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callResultSetMethodWithStrings( "getVersionColumns", nID, catalog, schema, table );
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxBinaryLiteralLength", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxRowSize(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxRowSize", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxCatalogNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxCatalogNameLength", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxCharLiteralLength", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxColumnNameLength", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxColumnsInIndex", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxCursorNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxCursorNameLength", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxConnections(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxConnections", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxColumnsInTable", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxStatementLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxStatementLength", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxTableNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxTableNameLength", nID);
}

sal_Int32 java_sql_DatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxTablesInSelect", nID);
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getExportedKeys(
        const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callResultSetMethodWithStrings( "getExportedKeys", nID, catalog, schema, table );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getImportedKeys(
        const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callResultSetMethodWithStrings( "getImportedKeys", nID, catalog, schema, table );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getPrimaryKeys(
        const Any& catalog, const OUString& schema, const OUString& table ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callResultSetMethodWithStrings( "getPrimaryKeys", nID, catalog, schema, table );
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
        static jmethodID nID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, nID);
        jvalue args[5];
        // convert Parameter
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : nullptr;
        args[1].l = schema.toChar() == '%' ? nullptr : convertwchar_tToJavaString(t.pEnv,schema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,table);
        args[3].z = unique;
        args[4].z = approximate;
        out = t.pEnv->CallObjectMethod( object, nID, args[0].l,args[1].l,args[2].l,args[3].z,args[4].z );

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
        static jmethodID nID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, nID);
        jvalue args[3];
        // convert Parameter
        args[0].l = catalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(catalog)) : nullptr;
        args[1].l = schema.toChar() == '%' ? nullptr : convertwchar_tToJavaString(t.pEnv,schema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,table);
        out = t.pEnv->CallObjectMethod( object, nID, args[0].l,args[1].l,args[2].l,scope,nullable);

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

    static jmethodID nID(nullptr);
    Reference< XResultSet > xReturn( impl_callResultSetMethodWithStrings( "getTablePrivileges", nID, catalog, schemaPattern, tableNamePattern ) );

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
                static const OUString sPrivs[] = {
                                            OUString("TABLE_CAT"),
                                            OUString("TABLE_SCHEM"),
                                            OUString("TABLE_NAME"),
                                            OUString("GRANTOR"),
                                            OUString("GRANTEE"),
                                            OUString("PRIVILEGE"),
                                            OUString("IS_GRANTABLE")
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
        static jmethodID nID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, nID);
        jvalue args[6];
        // convert Parameter
        args[0].l = primaryCatalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(primaryCatalog)) : nullptr;
        args[1].l = primarySchema.toChar() == '%' ? nullptr : convertwchar_tToJavaString(t.pEnv,primarySchema);
        args[2].l = convertwchar_tToJavaString(t.pEnv,primaryTable);
        args[3].l = foreignCatalog.hasValue() ? convertwchar_tToJavaString(t.pEnv,comphelper::getString(foreignCatalog)) : nullptr;
        args[4].l = foreignSchema.toChar() == '%' ? nullptr : convertwchar_tToJavaString(t.pEnv,foreignSchema);
        args[5].l = convertwchar_tToJavaString(t.pEnv,foreignTable);
        out = t.pEnv->CallObjectMethod( object, nID, args[0].l,args[2].l,args[2].l,args[3].l,args[4].l,args[5].l );

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
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "doesMaxRowSizeIncludeBlobs", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesLowerCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "storesLowerCaseQuotedIdentifiers", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesLowerCaseIdentifiers(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "storesLowerCaseIdentifiers", nID );
}

bool java_sql_DatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "storesMixedCaseQuotedIdentifiers", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "storesMixedCaseIdentifiers", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesUpperCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "storesUpperCaseQuotedIdentifiers", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::storesUpperCaseIdentifiers(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "storesUpperCaseIdentifiers", nID );
}

bool java_sql_DatabaseMetaData::impl_supportsAlterTableWithAddColumn_throw(  )
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsAlterTableWithAddColumn", nID );
}

bool java_sql_DatabaseMetaData::impl_supportsAlterTableWithDropColumn_throw(  )
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsAlterTableWithDropColumn", nID );
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxIndexLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxIndexLength", nID);
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsNonNullableColumns(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsNonNullableColumns", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getCatalogTerm(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getCatalogTerm", nID );
}

OUString java_sql_DatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getIdentifierQuoteString", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getExtraNameCharacters", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsDifferentTableCorrelationNames(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsDifferentTableCorrelationNames", nID );
}

bool java_sql_DatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "isCatalogAtStart", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::dataDefinitionIgnoredInTransactions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "dataDefinitionIgnoredInTransactions", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::dataDefinitionCausesTransactionCommit(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "dataDefinitionCausesTransactionCommit", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsDataManipulationTransactionsOnly(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsDataManipulationTransactionsOnly", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsDataDefinitionAndDataManipulationTransactions", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsPositionedDelete(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsPositionedDelete", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsPositionedUpdate(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsPositionedUpdate", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOpenStatementsAcrossRollback(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsOpenStatementsAcrossRollback", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOpenStatementsAcrossCommit(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsOpenStatementsAcrossCommit", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOpenCursorsAcrossCommit(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsOpenCursorsAcrossCommit", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOpenCursorsAcrossRollback(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsOpenCursorsAcrossRollback", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 level ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethodWithIntArg( "supportsTransactionIsolationLevel", nID, level );
}

bool java_sql_DatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsSchemasInDataManipulation", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsANSI92FullSQL(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsANSI92FullSQL", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsANSI92EntryLevelSQL(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsANSI92EntryLevelSQL", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsIntegrityEnhancementFacility", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSchemasInIndexDefinitions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsSchemasInIndexDefinitions", nID );
}

bool java_sql_DatabaseMetaData::impl_supportsSchemasInTableDefinitions_throw(  )
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsSchemasInTableDefinitions", nID );
}

bool java_sql_DatabaseMetaData::impl_supportsCatalogsInTableDefinitions_throw(  )
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsCatalogsInTableDefinitions", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCatalogsInIndexDefinitions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsCatalogsInIndexDefinitions", nID );
}

bool java_sql_DatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsCatalogsInDataManipulation", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOuterJoins(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsOuterJoins", nID );
}

Reference< XResultSet > SAL_CALL java_sql_DatabaseMetaData::getTableTypes(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callResultSetMethod( "getTableTypes", nID );
}

sal_Int32 java_sql_DatabaseMetaData::impl_getMaxStatements_throw(  )
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxStatements", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxProcedureNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxProcedureNameLength", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxSchemaNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxSchemaNameLength", nID);
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsTransactions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsTransactions", nID );
}


sal_Bool SAL_CALL java_sql_DatabaseMetaData::allProceduresAreCallable(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "allProceduresAreCallable", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsStoredProcedures(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsStoredProcedures", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSelectForUpdate(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsSelectForUpdate", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::allTablesAreSelectable(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "allTablesAreSelectable", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::isReadOnly(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "isReadOnly", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::usesLocalFiles(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "usesLocalFiles", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::usesLocalFilePerTable(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "usesLocalFilePerTable", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsTypeConversion(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsTypeConversion", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullPlusNonNullIsNull(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "nullPlusNonNullIsNull", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsColumnAliasing(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsColumnAliasing", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsTableCorrelationNames(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsTableCorrelationNames", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsConvert( sal_Int32 fromType, sal_Int32 toType ) throw(SQLException, RuntimeException, std::exception)
{
    static const char* pMethodName = "supportsConvert";
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD_ARG2, pMethodName, fromType, toType );

    jboolean out( sal_False );
    SDBThreadAttach t;

    {
        static jmethodID nID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, pMethodName,"(II)Z", nID);
        out = t.pEnv->CallBooleanMethod( object, nID, fromType, toType );
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    }

    m_aLogger.log< const sal_Char*, bool >( LogLevel::FINEST, STR_LOG_META_DATA_RESULT, pMethodName, out );
    return out;
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsExpressionsInOrderBy(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsExpressionsInOrderBy", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsGroupBy(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsGroupBy", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsGroupByBeyondSelect(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsGroupByBeyondSelect", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsGroupByUnrelated(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsGroupByUnrelated", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsMultipleTransactions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsMultipleTransactions", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsMultipleResultSets(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsMultipleResultSets", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsLikeEscapeClause(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsLikeEscapeClause", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsOrderByUnrelated(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsOrderByUnrelated", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsUnion(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsUnion", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsUnionAll(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsUnionAll", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsMixedCaseIdentifiers", nID );
}

bool java_sql_DatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsMixedCaseQuotedIdentifiers", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullsAreSortedAtEnd(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "nullsAreSortedAtEnd", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullsAreSortedAtStart(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "nullsAreSortedAtStart", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullsAreSortedHigh(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "nullsAreSortedHigh", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::nullsAreSortedLow(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "nullsAreSortedLow", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSchemasInProcedureCalls(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsSchemasInProcedureCalls", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsSchemasInPrivilegeDefinitions", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCatalogsInProcedureCalls(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsCatalogsInProcedureCalls", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsCatalogsInPrivilegeDefinitions", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCorrelatedSubqueries(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsCorrelatedSubqueries", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSubqueriesInComparisons(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsSubqueriesInComparisons", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSubqueriesInExists(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsSubqueriesInExists", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSubqueriesInIns(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsSubqueriesInIns", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsSubqueriesInQuantifieds(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsSubqueriesInQuantifieds", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsANSI92IntermediateSQL(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsANSI92IntermediateSQL", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException, std::exception)
{
    OUString sURL = m_pConnection->getURL();
    if ( sURL.isEmpty() )
    {
        static jmethodID nID(nullptr);
        sURL = impl_callStringMethod( "getURL", nID );
    }
    return sURL;
}

OUString SAL_CALL java_sql_DatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getUserName", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getDriverName", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getDriverVersion(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getDriverVersion", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getDatabaseProductVersion", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getDatabaseProductName(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getDatabaseProductName", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getProcedureTerm", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getSchemaTerm(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getSchemaTerm", nID );
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getDriverMajorVersion(  ) throw(RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowRuntime("getDriverMajorVersion", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getDefaultTransactionIsolation(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getDefaultTransactionIsolation", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getDriverMinorVersion(  ) throw(RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowRuntime("getDriverMinorVersion", nID);
}

OUString SAL_CALL java_sql_DatabaseMetaData::getSQLKeywords(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getSQLKeywords", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getSearchStringEscape", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getStringFunctions", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getTimeDateFunctions", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getSystemFunctions", nID );
}

OUString SAL_CALL java_sql_DatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callStringMethod( "getNumericFunctions", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsExtendedSQLGrammar(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsExtendedSQLGrammar", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsCoreSQLGrammar(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsCoreSQLGrammar", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsMinimumSQLGrammar(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsMinimumSQLGrammar", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsFullOuterJoins(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsFullOuterJoins", nID );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsLimitedOuterJoins(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsLimitedOuterJoins", nID );
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInGroupBy(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxColumnsInGroupBy", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInOrderBy(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxColumnsInOrderBy", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxColumnsInSelect(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxColumnsInSelect", nID);
}

sal_Int32 SAL_CALL java_sql_DatabaseMetaData::getMaxUserNameLength(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callIntMethod_ThrowSQL("getMaxUserNameLength", nID);
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsResultSetType( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethodWithIntArg( "supportsResultSetType", nID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency ) throw(SQLException, RuntimeException, std::exception)
{
    static const char* pMethodName = "supportsResultSetConcurrency";
    m_aLogger.log( LogLevel::FINEST, STR_LOG_META_DATA_METHOD_ARG2, pMethodName, setType, concurrency );

    jboolean out( sal_False );
    SDBThreadAttach t;

    {
        static jmethodID nID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, pMethodName,"(II)Z", nID);
        out =   t.pEnv->CallBooleanMethod( object, nID, setType, concurrency);
        ThrowLoggedSQLException( m_aLogger, t.pEnv, *this );
    }

    m_aLogger.log< const sal_Char*, bool >( LogLevel::FINEST, STR_LOG_META_DATA_RESULT, pMethodName, out );
    return out;
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::ownUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethodWithIntArg( "ownUpdatesAreVisible", nID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::ownDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethodWithIntArg( "ownDeletesAreVisible", nID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::ownInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethodWithIntArg( "ownInsertsAreVisible", nID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::othersUpdatesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethodWithIntArg( "othersUpdatesAreVisible", nID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::othersDeletesAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethodWithIntArg( "othersDeletesAreVisible", nID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::othersInsertsAreVisible( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethodWithIntArg( "othersInsertsAreVisible", nID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::updatesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethodWithIntArg( "updatesAreDetected", nID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::deletesAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethodWithIntArg( "deletesAreDetected", nID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::insertsAreDetected( sal_Int32 setType ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethodWithIntArg( "insertsAreDetected", nID, setType );
}

sal_Bool SAL_CALL java_sql_DatabaseMetaData::supportsBatchUpdates(  ) throw(SQLException, RuntimeException, std::exception)
{
    static jmethodID nID(nullptr);
    return impl_callBooleanMethod( "supportsBatchUpdates", nID );
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
        static jmethodID nID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, nID);
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

            out = t.pEnv->CallObjectMethod( object, nID, args[0].l, args[1].l,args[2].l,args[3].l);

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
