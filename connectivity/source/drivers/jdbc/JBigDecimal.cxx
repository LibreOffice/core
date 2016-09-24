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

#include "java/math/BigDecimal.hxx"
#include "java/tools.hxx"
#include "resource/jdbc_log.hrc"
using namespace connectivity;

//************ Class: java.lang.Boolean


jclass java_math_BigDecimal::theClass = nullptr;

java_math_BigDecimal::~java_math_BigDecimal()
{}

jclass java_math_BigDecimal::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/math/BigDecimal");
    return theClass;
}

java_math_BigDecimal::java_math_BigDecimal( const OUString& _par0 ): java_lang_Object( nullptr, nullptr )
{
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    // Java-Call for the Constructor
    // initialize temporary Variable
    static const char * cSignature = "(Ljava/lang/String;)V";
    jobject tempObj;
    static jmethodID mID(nullptr);
    obtainMethodId_throwSQL(t.pEnv, "<init>",cSignature, mID);

    jstring str = convertwchar_tToJavaString(t.pEnv,_par0.replace(',','.'));
    tempObj = t.pEnv->NewObject( getMyClass(), mID, str );
    t.pEnv->DeleteLocalRef(str);
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    ThrowSQLException( t.pEnv, nullptr );
    // and cleanup
}

java_math_BigDecimal::java_math_BigDecimal( const double& _par0 ): java_lang_Object( nullptr, nullptr )
{
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    // Java-Call for the Constructor
    // initialize temporary Variable
    static const char * cSignature = "(D)V";
    jobject tempObj;
    static jmethodID mID(nullptr);
    obtainMethodId_throwSQL(t.pEnv, "<init>",cSignature, mID);
    tempObj = t.pEnv->NewObject( getMyClass(), mID, _par0 );
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    ThrowSQLException( t.pEnv, nullptr );
    // and cleanup
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
