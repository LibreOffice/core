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

#include "java/sql/Clob.hxx"
#include "java/tools.hxx"
#include "java/io/Reader.hxx"
#include <connectivity/dbexception.hxx>

using namespace connectivity;

//************ Class: java.sql.Clob


jclass java_sql_Clob::theClass = nullptr;

java_sql_Clob::java_sql_Clob( JNIEnv * pEnv, jobject myObj )
    : java_lang_Object( pEnv, myObj )
{
    SDBThreadAttach::addRef();
}
java_sql_Clob::~java_sql_Clob()
{
    SDBThreadAttach::releaseRef();
}

jclass java_sql_Clob::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/sql/Clob");
    return theClass;
}

sal_Int64 SAL_CALL java_sql_Clob::length(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        // initialize temporary variable
        static const char * cSignature = "()J";
        static const char * cMethodName = "length";
        // execute Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        out = t.pEnv->CallLongMethod( object, mID );
        ThrowSQLException(t.pEnv,*this);
    } //t.pEnv
    return (sal_Int64)out;
}

OUString SAL_CALL java_sql_Clob::getSubString( sal_Int64 pos, sal_Int32 subStringLength ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    OUString aStr;
    {
        // initialize temporary variable
        static const char * cSignature = "(JI)Ljava/lang/String;";
        static const char * cMethodName = "getSubString";
        // execute Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        jstring out = static_cast<jstring>(t.pEnv->CallObjectMethod( object, mID,pos,subStringLength));
        ThrowSQLException(t.pEnv,*this);
        aStr = JavaString2String(t.pEnv,out);
    } //t.pEnv
    // WARNING: the caller becomes the owner of the returned pointer
    return  aStr;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL java_sql_Clob::getCharacterStream(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    SDBThreadAttach t;
    static jmethodID mID(nullptr);
    jobject out = callObjectMethod(t.pEnv,"getCharacterStream","()Ljava/io/Reader;", mID);

    // WARNING: the caller becomes the owner of the returned pointer
    return out==nullptr ? nullptr : new java_io_Reader( t.pEnv, out );
}

sal_Int64 SAL_CALL java_sql_Clob::position( const OUString& searchstr, sal_Int32 start ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    jlong out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");

    {
        jvalue args[1];
        // convert Parameter
        args[0].l = convertwchar_tToJavaString(t.pEnv,searchstr);
        // initialize temporary Variable
        static const char * cSignature = "(Ljava/lang/String;I)J";
        static const char * cMethodName = "position";
        // execute Java-Call
        static jmethodID mID(nullptr);
        obtainMethodId_throwSQL(t.pEnv, cMethodName,cSignature, mID);
        out = t.pEnv->CallLongMethod( object, mID, args[0].l,start );
        ThrowSQLException(t.pEnv,*this);
        t.pEnv->DeleteLocalRef(static_cast<jstring>(args[0].l));
    } //t.pEnv
    return (sal_Int64)out;
}

sal_Int64 SAL_CALL java_sql_Clob::positionOfClob( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob >& /*pattern*/, sal_Int64 /*start*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XClob::positionOfClob", *this );
    // this was put here in CWS warnings01. The previous implementation was defective, as it did ignore
    // the pattern parameter. Since the effort for proper implementation is rather high - we would need
    // to translated patter into a byte[] -, we defer this functionality for the moment (hey, it was
    // unusable, anyway)
    // 2005-11-15 / #i57457# / frank.schoenheit@sun.com
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
