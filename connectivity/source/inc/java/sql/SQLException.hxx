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


#ifndef _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_
#define _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_

#include "java/lang/Exception.hxx"
#include <com/sun/star/sdbc/SQLException.hpp>

namespace connectivity
{
    namespace starsdbc  = ::com::sun::star::sdbc;
    //**************************************************************
    //************ Class: java.sql.SQLException
    //**************************************************************
    class java_sql_SQLException_BASE;
    class java_sql_SQLException :   public starsdbc::SQLException
    {
    public:
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_SQLException(){}
        java_sql_SQLException( const java_sql_SQLException_BASE& _rException,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> & _rContext);
    };

    class java_sql_SQLException_BASE :  public java_lang_Exception
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_sql_SQLException_BASE();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_SQLException_BASE( JNIEnv * pEnv, jobject myObj );

        ::rtl::OUString getSQLState() const;
        sal_Int32       getErrorCode() const;
        starsdbc::SQLException getNextException() const;

        static jclass st_getMyClass();
    };

}
#endif // _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_

