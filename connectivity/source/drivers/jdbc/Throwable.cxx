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
#include "java/lang/Throwable.hxx"
#include "java/tools.hxx"
using namespace connectivity;
//**************************************************************
//************ Class: java.lang.Throwable
//**************************************************************

jclass java_lang_Throwable::theClass = 0;

java_lang_Throwable::~java_lang_Throwable()
{}

jclass java_lang_Throwable::getMyClass() const
{
    return st_getMyClass();
}
jclass java_lang_Throwable::st_getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass )
        theClass = findMyClass("java/lang/Throwable");
    return theClass;
}
// -----------------------------------------------------------------------------

::rtl::OUString java_lang_Throwable::getMessage() const
{
    static jmethodID mID(NULL);
    return callStringMethod("getMessage",mID);
}
// -----------------------------------------------------------------------------

::rtl::OUString java_lang_Throwable::getLocalizedMessage() const
{
    static jmethodID mID(NULL);
    return callStringMethod("getLocalizedMessage",mID);
}
// -----------------------------------------------------------------------------


