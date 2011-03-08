/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "java/sql/Array.hxx"
#include "java/tools.hxx"
#include "java/sql/ResultSet.hxx"

using namespace connectivity;
//**************************************************************
//************ Class: java.sql.Array
//**************************************************************

jclass java_sql_Array::theClass = 0;

java_sql_Array::~java_sql_Array()
{}

jclass java_sql_Array::getMyClass() const
{
    // the class must be fetched once, therefore its static
    if( !theClass )
        theClass = findMyClass("java/sql/Array");

    return theClass;
}

::rtl::OUString SAL_CALL java_sql_Array::getBaseTypeName(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    static jmethodID mID(NULL);
    return callStringMethod("getBaseTypeName",mID);
}

sal_Int32 SAL_CALL java_sql_Array::getBaseType(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    static jmethodID mID(NULL);
    return callIntMethod("getBaseType",mID);
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL java_sql_Array::getArray( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        static const char * cSignature = "(Ljava/util/Map;)[Ljava/lang/Object;";
        static const char * cMethodName = "getArray";
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        // submit Java-Call
        (jobjectArray)t.pEnv->CallObjectMethod( object, mID, obj);
        ThrowSQLException(t.pEnv,*this);
        // and clean up
        t.pEnv->DeleteLocalRef(obj);
    } //t.pEnv
    return ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >();
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL java_sql_Array::getArrayAtIndex( sal_Int32 index, sal_Int32 count, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        static const char * cSignature = "(IILjava/util/Map;)[Ljava/lang/Object;";
        static const char * cMethodName = "getArray";
        // submit Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        (jobjectArray)t.pEnv->CallObjectMethod( object, mID, index,count,obj);
        ThrowSQLException(t.pEnv,*this);
        // and clean up
        t.pEnv->DeleteLocalRef(obj);
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >();
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL java_sql_Array::getResultSet( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // convert Parameter
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        // initialize temporary variable
        static const char * cSignature = "(Ljava/util/Map;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getResultSet";
        // submit Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        t.pEnv->CallObjectMethod( object, mID, obj);
        ThrowSQLException(t.pEnv,*this);
        // and cleanup
        t.pEnv->DeleteLocalRef(obj);
    }
    return NULL;
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL java_sql_Array::getResultSetAtIndex( sal_Int32 index, sal_Int32 count, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // convert parameter
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        // initialize temporary variable
        static const char * cSignature = "(Ljava/util/Map;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getResultSetAtIndex";
        // Java-Call absetzen
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        t.pEnv->CallObjectMethod( object, mID, index,count,obj);
        ThrowSQLException(t.pEnv,*this);
        // and cleanup
        t.pEnv->DeleteLocalRef(obj);
    }
    return NULL;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
