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
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass )
        theClass = findMyClass("java/math/BigDecimal");
    return theClass;
}

java_math_BigDecimal::java_math_BigDecimal( const ::rtl::OUString& _par0 ): java_lang_Object( NULL, (jobject)NULL )
{
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
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
    // und aufraeumen
}

java_math_BigDecimal::java_math_BigDecimal( const double& _par0 ): java_lang_Object( NULL, (jobject)NULL )
{
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    static const char * cSignature = "(D)V";
    jobject tempObj;
    static jmethodID mID(NULL);
    obtainMethodId(t.pEnv, "<init>",cSignature, mID);
    tempObj = t.pEnv->NewObject( getMyClass(), mID, _par0 );
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    ThrowSQLException( t.pEnv, NULL );
    // und aufraeumen
}

