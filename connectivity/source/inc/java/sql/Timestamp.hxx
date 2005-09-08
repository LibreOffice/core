/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Timestamp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:24:22 $
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
#ifndef _CONNECTIVITY_JAVA_SQL_TIMESTAMP_HXX_
#define _CONNECTIVITY_JAVA_SQL_TIMESTAMP_HXX_

#ifndef _CONNECTIVITY_JAVA_UTIL_DATE_HXX_
#include "java/util/Date.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif


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
        // der Destruktor um den Object-Counter zu aktualisieren
        static void saveClassRef( jclass pClass );
    public:
        static jclass getMyClass();
        virtual ~java_sql_Date();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_Date( JNIEnv * pEnv, jobject myObj ) : java_util_Date(pEnv,myObj){}
        java_sql_Date( const ::com::sun::star::util::Date& _rOut );

        operator ::com::sun::star::util::Date();
    };


    //**************************************************************
    //************ Class: java.sql.Time
    //**************************************************************

    class java_sql_Time : public java_util_Date
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
        // der Destruktor um den Object-Counter zu aktualisieren
        static void saveClassRef( jclass pClass );
    public:
        static jclass getMyClass();
        virtual ~java_sql_Time();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_Time( JNIEnv * pEnv, jobject myObj ) : java_util_Date( pEnv, myObj ){}
        java_sql_Time( const ::com::sun::star::util::Time& _rOut );
        operator ::com::sun::star::util::Time();
    };

    //**************************************************************
    //************ Class: java.sql.Timestamp
    //**************************************************************
    class java_sql_Timestamp : public java_util_Date
    {
    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;
        // der Destruktor um den Object-Counter zu aktualisieren
        static void saveClassRef( jclass pClass );
    public:
        static jclass getMyClass();
        virtual ~java_sql_Timestamp();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_Timestamp( JNIEnv * pEnv, jobject myObj ) : java_util_Date( pEnv, myObj ){}
        java_sql_Timestamp( const ::com::sun::star::util::DateTime& _rOut);
        operator ::com::sun::star::util::DateTime();

        sal_Int32 getNanos();
        void  setNanos(sal_Int32 n);
    };
}
#endif // _CONNECTIVITY_JAVA_SQL_TIMESTAMP_HXX_

