/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DriverManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:21:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_JAVA_SQL_DRIVERMANAGER_HXX_
#define _CONNECTIVITY_JAVA_SQL_DRIVERMANAGER_HXX_

#ifndef _CONNECTIVITY_JAVA_LANG_OBJECT_HXX_
#include "java/lang/Object.hxx"
#endif

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

