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
#include "java/math/BigDecimal.hxx"
#include "java/tools.hxx"
#include "resource/jdbc_log.hrc"
using namespace connectivity;
//**************************************************************
//************ Class: java.lang.Boolean
//**************************************************************

jclass java_math_BigDecimal::theClass = 0;

java_math_BigDecimal::~java_math_BigDecimal()
{}

jclass java_math_BigDecimal::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/math/BigDecimal");
    return theClass;
}

java_math_BigDecimal::java_math_BigDecimal( const ::rtl::OUString& _par0 ): java_lang_Object( NULL, (jobject)NULL )
{
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    // Java-Call for the Constructor
    // initialize temporary Variable
    static const char * cSignature = "(Ljava/lang/String;)V";
    jobject tempObj;
    static jmethodID mID(NULL);
    obtainMethodId(t.pEnv, "<init>",cSignature, mID);

    jstring str = convertwchar_tToJavaString(t.pEnv,_par0.replace(',','.'));
    tempObj = t.pEnv->NewObject( getMyClass(), mID, str );
    t.pEnv->DeleteLocalRef(str);
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    ThrowSQLException( t.pEnv, NULL );
    // and cleanup
}

java_math_BigDecimal::java_math_BigDecimal( const double& _par0 ): java_lang_Object( NULL, (jobject)NULL )
{
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    // Java-Call for the Constructor
    // initialize temporary Variable
    static const char * cSignature = "(D)V";
    jobject tempObj;
    static jmethodID mID(NULL);
    obtainMethodId(t.pEnv, "<init>",cSignature, mID);
    tempObj = t.pEnv->NewObject( getMyClass(), mID, _par0 );
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    ThrowSQLException( t.pEnv, NULL );
    // and cleanup
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
