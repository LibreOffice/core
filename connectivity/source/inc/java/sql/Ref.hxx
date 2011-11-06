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



#ifndef _CONNECTIVITY_JAVA_SQL_REF_HXX_
#define _CONNECTIVITY_JAVA_SQL_REF_HXX_

#include "java/lang/Object.hxx"
#include <com/sun/star/sdbc/XRef.hpp>
#include <cppuhelper/implbase1.hxx>

namespace connectivity
{
    //**************************************************************
    //************ Class: java.sql.Ref
    //**************************************************************
    class java_sql_Ref :    public java_lang_Object,
                            public ::cppu::WeakImplHelper1< ::com::sun::star::sdbc::XRef>
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
        virtual ~java_sql_Ref();
    public:
        virtual jclass getMyClass() const;

        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_Ref( JNIEnv * pEnv, jobject myObj );

        // XRef
        virtual ::rtl::OUString SAL_CALL getBaseTypeName(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    };
}
#endif // _CONNECTIVITY_JAVA_SQL_REF_HXX_

