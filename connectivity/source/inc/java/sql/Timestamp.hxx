/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
    // static data for the class
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
    // static data for the class
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
    // static data for the class
        static jclass theClass;
    public:
        virtual jclass getMyClass() const;
        virtual ~java_sql_Timestamp();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_Timestamp( JNIEnv * pEnv, jobject myObj ) : java_util_Date( pEnv, myObj ){}
        java_sql_Timestamp( const ::com::sun::star::util::DateTime& _rOut);
        operator ::com::sun::star::util::DateTime();

        static jclass st_getMyClass();
    };
}
#endif // _CONNECTIVITY_JAVA_SQL_TIMESTAMP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
