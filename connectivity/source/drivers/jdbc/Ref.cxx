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
#include "java/sql/Ref.hxx"
#include "java/tools.hxx"
using namespace connectivity;
//**************************************************************
//************ Class: java.sql.Ref
//**************************************************************

jclass java_sql_Ref::theClass = 0;
java_sql_Ref::java_sql_Ref( JNIEnv * pEnv, jobject myObj )
: java_lang_Object( pEnv, myObj )
{
    SDBThreadAttach::addRef();
}
java_sql_Ref::~java_sql_Ref()
{
    SDBThreadAttach::releaseRef();
}

jclass java_sql_Ref::getMyClass() const
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass )
        theClass = findMyClass("java/sql/Ref");
    return theClass;
}

::rtl::OUString SAL_CALL java_sql_Ref::getBaseTypeName(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    static jmethodID mID(NULL);
    return callStringMethod("getBaseTypeName",mID);
}

