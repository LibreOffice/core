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


#ifndef _CONNECTIVITY_JAVA_SQL_TIMESTAMP_HXX_
#define _CONNECTIVITY_JAVA_SQL_TIMESTAMP_HXX_

#include "java/util/Date.hxx"
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>


namespace connectivity
{
    //**************************************************************
    //************ Class: java.sql.Date
    //**************************************************************

    class java_sql_Date : public java_util_Date
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_sql_Date();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_Date( JNIEnv * pEnv, jobject myObj ) : java_util_Date(pEnv,myObj){}
        java_sql_Date( const ::com::sun::star::util::Date& _rOut );

        operator ::com::sun::star::util::Date();
        static jclass st_getMyClass();
    };


    //**************************************************************
    //************ Class: java.sql.Time
    //**************************************************************

    class java_sql_Time : public java_util_Date
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_sql_Time();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_Time( JNIEnv * pEnv, jobject myObj ) : java_util_Date( pEnv, myObj ){}
        java_sql_Time( const ::com::sun::star::util::Time& _rOut );
        operator ::com::sun::star::util::Time();
        static jclass st_getMyClass();
    };

    //**************************************************************
    //************ Class: java.sql.Timestamp
    //**************************************************************
    class java_sql_Timestamp : public java_util_Date
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_sql_Timestamp();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_Timestamp( JNIEnv * pEnv, jobject myObj ) : java_util_Date( pEnv, myObj ){}
        java_sql_Timestamp( const ::com::sun::star::util::DateTime& _rOut);
        operator ::com::sun::star::util::DateTime();

        sal_Int32 getNanos();
        void  setNanos(sal_Int32 n);
        static jclass st_getMyClass();
    };
}
#endif // _CONNECTIVITY_JAVA_SQL_TIMESTAMP_HXX_

