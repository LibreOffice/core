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

#include "java/sql/Blob.hxx"
#include "java/tools.hxx"
#include "java/io/InputStream.hxx"
#include <connectivity/dbexception.hxx>

#include <string.h>

using namespace connectivity;

//************ Class: java.sql.Blob


jclass java_sql_Blob::theClass = nullptr;
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
    // the class must be fetched only once, therefore it's static
    if( !theClass )
        theClass = findMyClass("java/sql/Blob");
    return theClass;
}

sal_Int64 SAL_CALL java_sql_Blob::length(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        // initialize temporary variable
        static const char * cSignature = "()J";
        static const char * cMethodName = "length";
        // submit Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        out = t.pEnv->CallLongMethod( object, mID );
        ThrowSQLException(t.pEnv,*this);
    } //t.pEnv
    return (sal_Int64)out;
}
::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL java_sql_Blob::getBytes( sal_Int64 pos, sal_Int32 count ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{

    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
    {
        // initialize temporary variable
        static const char * cSignature = "(JI)[B";
        static const char * cMethodName = "getBytes";
        // submit Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        jbyteArray out = static_cast<jbyteArray>(t.pEnv->CallObjectMethod( object, mID,pos,count));
        ThrowSQLException(t.pEnv,*this);
        if(out)
        {
            jboolean p = sal_False;
            aSeq.realloc(t.pEnv->GetArrayLength(out));
            memcpy(aSeq.getArray(),t.pEnv->GetByteArrayElements(out,&p),aSeq.getLength());
            t.pEnv->DeleteLocalRef(out);
        }
    } //t.pEnv
    // WARNING: the caller becomes the owner of the returned pointer
    return  aSeq;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_Blob::getBinaryStream(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(nullptr);
    jobject out = callObjectMethod(t.pEnv,"getBinaryStream","()Ljava/io/InputStream;", mID);
    // WARNING: the caller becomes the owner of the returned pointer
    return out==nullptr ? nullptr : new java_io_InputStream( t.pEnv, out );
}

sal_Int64 SAL_CALL java_sql_Blob::position( const ::com::sun::star::uno::Sequence< sal_Int8 >& pattern, sal_Int64 start ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        // initialize temporary variable
        static const char * cSignature = "([BI)J";
        static const char * cMethodName = "position";
        // submit Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        // convert Parameter
        jbyteArray pByteArray = t.pEnv->NewByteArray(pattern.getLength());
        jbyte * patternData = reinterpret_cast<jbyte *>(
            const_cast<sal_Int8 *>(pattern.getConstArray()));
            // 4th param of Set*ArrayRegion changed from pointer to non-const to
            // pointer to const between <http://docs.oracle.com/javase/6/docs/
            // technotes/guides/jni/spec/functions.html#wp22933> and
            // <http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/
            // functions.html#wp22933>; work around that difference in a way
            // that doesn't trigger loplugin:redundantcast
        t.pEnv->SetByteArrayRegion(pByteArray,0,pattern.getLength(),patternData);
        out = t.pEnv->CallLongMethod( object, mID, pByteArray,start );
        t.pEnv->DeleteLocalRef(pByteArray);
        ThrowSQLException(t.pEnv,*this);
    } //t.pEnv
    return (sal_Int64)out;
}

sal_Int64 SAL_CALL java_sql_Blob::positionOfBlob( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& /*pattern*/, sal_Int64 /*start*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XBlob::positionOfBlob", *this );
    // this was put here in CWS warnings01. The previous implementation was defective, as it did ignore
    // the pattern parameter. Since the effort for proper implementation is rather high - we would need
    // to translated patter into a byte[] -, we defer this functionality for the moment (hey, it was
    // unusable, anyway)
    // #i57457#
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
