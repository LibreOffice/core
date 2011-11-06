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


#ifndef CONNECTIVITY_java_util_Properties
#define CONNECTIVITY_java_util_Properties

#include "java/lang/Object.hxx"

namespace connectivity
{
    class java_util_Properties : public java_lang_Object
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_util_Properties();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_util_Properties( JNIEnv * pEnv, jobject myObj ) : java_lang_Object( pEnv, myObj ){}
        java_util_Properties( );
        void setProperty(const ::rtl::OUString key, const ::rtl::OUString& value);
    };
}

#endif // CONNECTIVITY_java_util_Properties
