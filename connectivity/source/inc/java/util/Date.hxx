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



#ifndef _CONNECTIVITY_JAVA_UTIL_DATE_HXX_
#define _CONNECTIVITY_JAVA_UTIL_DATE_HXX_

#include "java/lang/Object.hxx"
#include <com/sun/star/util/Date.hpp>

namespace connectivity
{

    //**************************************************************
    //************ Class: java.util.Date
    //**************************************************************

    class java_util_Date : public java_lang_Object
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_util_Date();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_util_Date( JNIEnv * pEnv, jobject myObj ) : java_lang_Object( pEnv, myObj ){}
    };
}
#endif // _CONNECTIVITY_JAVA_UTIL_DATE_HXX_

