/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    // die Klasse muss nur einmal geholt werden, daher statisch
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
    jobjectArray out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        static const char * cSignature = "(Ljava/util/Map;)[Ljava/lang/Object;";
        static const char * cMethodName = "getArray";
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        // Java-Call absetzen
        out = (jobjectArray)t.pEnv->CallObjectMethod( object, mID, obj);
        ThrowSQLException(t.pEnv,*this);
        // und aufraeumen
        t.pEnv->DeleteLocalRef(obj);
    } //t.pEnv
    return ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >();//copyArrayAndDelete< ::com::sun::star::uno::Any,jobject>(t.pEnv,out);
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL java_sql_Array::getArrayAtIndex( sal_Int32 index, sal_Int32 count, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jobjectArray out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        static const char * cSignature = "(IILjava/util/Map;)[Ljava/lang/Object;";
        static const char * cMethodName = "getArray";
        // Java-Call absetzen
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        out = (jobjectArray)t.pEnv->CallObjectMethod( object, mID, index,count,obj);
        ThrowSQLException(t.pEnv,*this);
        // und aufraeumen
        t.pEnv->DeleteLocalRef(obj);
    } //t.pEnv
    return ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >();//copyArrayAndDelete< ::com::sun::star::uno::Any,jobject>(t.pEnv,out);
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL java_sql_Array::getResultSet( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // Parameter konvertieren
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/util/Map;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getResultSet";
        // Java-Call absetzen
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        out = t.pEnv->CallObjectMethod( object, mID, obj);
        ThrowSQLException(t.pEnv,*this);
        // und aufraeumen
        t.pEnv->DeleteLocalRef(obj);
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    //  return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
    return NULL;
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL java_sql_Array::getResultSetAtIndex( sal_Int32 index, sal_Int32 count, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jobject out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    {
        // Parameter konvertieren
        jobject obj = convertTypeMapToJavaMap(t.pEnv,typeMap);
        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/util/Map;)Ljava/sql/ResultSet;";
        static const char * cMethodName = "getResultSetAtIndex";
        // Java-Call absetzen
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        out = t.pEnv->CallObjectMethod( object, mID, index,count,obj);
        ThrowSQLException(t.pEnv,*this);
        // und aufraeumen
        t.pEnv->DeleteLocalRef(obj);
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    //  return out==0 ? 0 : new java_sql_ResultSet( t.pEnv, out );
    return NULL;
}



