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



#ifndef _CONNECTIVITY_JAVA_SQL_SQLWARNING_HXX_
#define _CONNECTIVITY_JAVA_SQL_SQLWARNING_HXX_

#include "java/sql/SQLException.hxx"
//#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
//#include <com/sun/star/sdbc/SQLWarning.hpp>
//#endif

namespace connectivity
{
    //  namespace starsdbc  = ::com::sun::star::sdbc;
    //  namespace ::com::sun::star::uno = ::com::sun::star::uno;
    //**************************************************************
    //************ Class: java.sql.SQLWarning
    //**************************************************************
    class java_sql_SQLWarning_BASE : public java_sql_SQLException_BASE
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_sql_SQLWarning_BASE();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_SQLWarning_BASE( JNIEnv * pEnv, jobject myObj ) : java_sql_SQLException_BASE( pEnv, myObj ){}

    };

    class java_sql_SQLWarning : public java_sql_SQLException
    {
    public:
        java_sql_SQLWarning(const java_sql_SQLWarning_BASE& _rW,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> & _rContext)
            : java_sql_SQLException(_rW,_rContext) {}
    };
}
#endif // _CONNECTIVITY_JAVA_SQL_SQLWARNING_HXX_

