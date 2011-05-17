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
#include "java/sql/Blob.hxx"
#include "java/tools.hxx"
#include "java/io/InputStream.hxx"
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
    // the class must be fetched only once, therefore it's static
    if( !theClass )
        theClass = findMyClass("java/sql/Blob");
    return theClass;
}

sal_Int64 SAL_CALL java_sql_Blob::length(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        // initialize temporary variable
        static const char * cSignature = "()J";
        static const char * cMethodName = "length";
        // submit Java-Call
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
        // initialize temporary variable
        static const char * cSignature = "(JI)[B";
        static const char * cMethodName = "getBytes";
        // submit Java-Call
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
    // WARNING: the caller becomes the owner of the returned pointer
    return  aSeq;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_Blob::getBinaryStream(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    static jmethodID mID(NULL);
    jobject out = callObjectMethod(t.pEnv,"getBinaryStream","()Ljava/io/InputStream;", mID);
    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? 0 : new java_io_InputStream( t.pEnv, out );
}

sal_Int64 SAL_CALL java_sql_Blob::position( const ::com::sun::star::uno::Sequence< sal_Int8 >& pattern, sal_Int64 start ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        // initialize temporary variable
        static const char * cSignature = "([BI)J";
        static const char * cMethodName = "position";
        // submit Java-Call
        static jmethodID mID(NULL);
        obtainMethodId(t.pEnv, cMethodName,cSignature, mID);
        // convert Parameter
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
    // #i57457#
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
