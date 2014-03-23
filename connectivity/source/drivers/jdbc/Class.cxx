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
#include "java/lang/Class.hxx"
#include "java/tools.hxx"
#include <rtl/ustring.hxx>

using namespace connectivity;
//**************************************************************
//************ Class: java.lang.Class
//**************************************************************

jclass java_lang_Class::theClass = 0;

java_lang_Class::~java_lang_Class()
{}

jclass java_lang_Class::getMyClass() const
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass )
        theClass = findMyClass("java/lang/Class");
    return theClass;
}

java_lang_Class * java_lang_Class::forName( const ::rtl::OUString& _par0 )
{
    jobject out(NULL);
    SDBThreadAttach t;

    {
        ::rtl::OString sClassName = ::rtl::OUStringToOString(_par0, RTL_TEXTENCODING_JAVA_UTF8);
        sClassName = sClassName.replace('.','/');
        out = t.pEnv->FindClass( sClassName.getStr());
        ThrowSQLException(t.pEnv,0);
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? NULL : new java_lang_Class( t.pEnv, out );
}

jobject java_lang_Class::newInstanceObject()
{
    SDBThreadAttach t;
    static jmethodID mID(NULL);
    return callObjectMethod(t.pEnv,"newInstance","()Ljava/lang/Object;", mID);
}

