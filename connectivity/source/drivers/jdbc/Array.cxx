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

#include "java/sql/Array.hxx"
#include "java/tools.hxx"
#include "java/sql/ResultSet.hxx"

using namespace connectivity;

//************ Class: java.sql.Array


jclass java_sql_Array::theClass = nullptr;

java_sql_Array::~java_sql_Array()
{}

jclass java_sql_Array::getMyClass() const
{
    // the class must be fetched once, therefore its static
    if( !theClass )
        theClass = findMyClass("java/sql/Array");

    return theClass;
}

OUString SAL_CALL java_sql_Array::getBaseTypeName(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callStringMethod("getBaseTypeName",mID);
}

sal_Int32 SAL_CALL java_sql_Array::getBaseType(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    static jmethodID mID(nullptr);
    return callIntMethod_ThrowSQL("getBaseType", mID);
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL java_sql_Array::getArray( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        static const char * cSignature = "(Ljava/util/Map;)[Ljava/lang/Object;";
        static const char * cMethodName = "getArray";
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        // submit Java-Call
        t.pEnv->CallObjectMethod( object, mID, obj);
        ThrowSQLException(t.pEnv,*this);
        // and clean up
        t.pEnv->DeleteLocalRef(obj);
    } //t.pEnv
    return ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >();
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL java_sql_Array::getArrayAtIndex( sal_Int32 index, sal_Int32 count, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        static const char * cSignature = "(IILjava/util/Map;)[Ljava/lang/Object;";
        static const char * cMethodName = "getArray";
        // submit Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        t.pEnv->CallObjectMethod( object, mID, index,count,obj);
        ThrowSQLException(t.pEnv,*this);
        // and clean up
        t.pEnv->DeleteLocalRef(obj);
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >();
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL java_sql_Array::getResultSet( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // convert Parameter
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        // initialize temporary variable
        static const char * cSignature = "(Ljava/util/Map;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getResultSet";
        // submit Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        t.pEnv->CallObjectMethod( object, mID, obj);
        ThrowSQLException(t.pEnv,*this);
        // and cleanup
        t.pEnv->DeleteLocalRef(obj);
    }
    return nullptr;
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL java_sql_Array::getResultSetAtIndex( sal_Int32 index, sal_Int32 count, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // convert parameter
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        // initialize temporary variable
        static const char * cSignature = "(Ljava/util/Map;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getResultSetAtIndex";
        // submit Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        t.pEnv->CallObjectMethod( object, mID, index,count,obj);
        ThrowSQLException(t.pEnv,*this);
        // and cleanup
        t.pEnv->DeleteLocalRef(obj);
    }
    return nullptr;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
