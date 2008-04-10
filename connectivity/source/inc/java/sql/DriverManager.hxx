/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DriverManager.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_JAVA_SQL_DRIVERMANAGER_HXX_
#define _CONNECTIVITY_JAVA_SQL_DRIVERMANAGER_HXX_

#include "java/lang/Object.hxx"

namespace connectivity
{
    class java_sql_Driver;
    class java_sql_Connection;
    //  class java_util_Properties;
    //**************************************************************
    //************ Class: java.sql.DriverManager
    //**************************************************************
    class java_sql_DriverManager : public java_lang_Object
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
        // der Destruktor um den Object-Counter zu aktualisieren
        static void saveClassRef( jclass pClass );
    public:
        static jclass getMyClass();
        virtual ~java_sql_DriverManager();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_DriverManager( JNIEnv * pEnv, jobject myObj ) : java_lang_Object( pEnv, myObj ){}

        static jobject getDriver(const ::rtl::OUString &url);

        //  static java_sql_Connection * getConnection( const ::rtl::OUString* _par0 );
        //  static java_sql_Connection * getConnection( const String* _par0, java_util_Properties * _par1 );
        //  static java_sql_Connection * getConnection( const String* _par0, const String* _par1, const String* _par2 );
        //  static java_util_Enumeration * getDrivers();
        static void setLoginTimeout(sal_Int32 _par0);
    };
}
#endif // _CONNECTIVITY_JAVA_SQL_DRIVERMANAGER_HXX_

