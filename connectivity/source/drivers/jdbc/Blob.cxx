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
#include "java/sql/Blob.hxx"
#include "java/tools.hxx"
#include "java/io/InputStream.hxx"
#ifndef _INC_MEMORY
//#include <memory.h>
#endif
#include <connectivity/dbexception.hxx>

#include <string.h>

using namespace connectivity;
//**************************************************************
//************ Class: java.sql.Blob
//**************************************************************

jclass java_sql_Blob::theClass = 0;
java_sql_Blob::java_sql_Blob( JNIEnv * pEnv, jobject myObj )
    : java_lang_Object( pEnv, myObj )
{
    SDBThreadAttach::addRef();
}
java_sql_Blob::~java_sql_Blob()
{
    SDBThreadAttach::releaseRef();
}

jclass java_sql_Blob::getMyClass() const
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass )
        theClass = findMyClass("java/sql/Blob");
    return theClass;
}

sal_Int64 SAL_CALL java_sql_Blob::length(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()J";
        static const char * cMethodName = "length";
        // Java-Call absetzen
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        out = t.pEnv->CallLongMethod( object, mID );
        ThrowSQLException(t.pEnv,*this);
    } //t.pEnv
    return (sal_Int64)out;
}
::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL java_sql_Blob::getBytes( sal_Int64 pos, sal_Int32 count ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "(JI)[B";
        static const char * cMethodName = "getBytes";
        // Java-Call absetzen
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        jbyteArray out = (jbyteArray)t.pEnv->CallObjectMethod( object, mID,pos,count);
        ThrowSQLException(t.pEnv,*this);
        if(out)
        {
            jboolean p = sal_False;
            aSeq.realloc(t.pEnv->GetArrayLength(out));
            memcpy(aSeq.getArray(),t.pEnv->GetByteArrayElements(out,&p),aSeq.getLength());
            t.pEnv->DeleteLocalRef(out);
        }
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return  aSeq;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_Blob::getBinaryStream(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    jobject out = callObjectMethod(t.pEnv,"getBinaryStream","()Ljava/io/InputStream;", mID);
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? 0 : new java_io_InputStream( t.pEnv, out );
}

sal_Int64 SAL_CALL java_sql_Blob::position( const ::com::sun::star::uno::Sequence< sal_Int8 >& pattern, sal_Int64 start ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        // temporaere Variable initialisieren
        static const char * cSignature = "([BI)J";
        static const char * cMethodName = "position";
        // Java-Call absetzen
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        // Parameter konvertieren
        jbyteArray pByteArray = t.pEnv->NewByteArray(pattern.getLength());
        t.pEnv->SetByteArrayRegion(pByteArray,0,pattern.getLength(),(jbyte*)pattern.getConstArray());
        out = t.pEnv->CallLongMethod( object, mID, pByteArray,start );
        t.pEnv->DeleteLocalRef(pByteArray);
        ThrowSQLException(t.pEnv,*this);
    } //t.pEnv
    return (sal_Int64)out;
}

sal_Int64 SAL_CALL java_sql_Blob::positionOfBlob( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& /*pattern*/, sal_Int64 /*start*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XBlob::positionOfBlob", *this );
    // this was put here in CWS warnings01. The previous implementation was defective, as it did ignore
    // the pattern parameter. Since the effort for proper implementation is rather high - we would need
    // to translated patter into a byte[] -, we defer this functionality for the moment (hey, it was
    // unusable, anyway)
    // 2005-11-15 / #i57457# / frank.schoenheit@sun.com
    return 0;
}

