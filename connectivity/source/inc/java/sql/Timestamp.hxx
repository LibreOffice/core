/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_TIMESTAMP_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_TIMESTAMP_HXX

#include "java/util/Date.hxx"
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>


namespace connectivity
{

    //************ Class: java.sql.Date


    class java_sql_Date : public java_util_Date
    {
    protected:
    // static data for the class
        static jclass theClass;
    public:
        virtual jclass getMyClass() const override;
        virtual ~java_sql_Date();
        // A ctor that is needed for returning the object
        java_sql_Date( JNIEnv * pEnv, jobject myObj ) : java_util_Date(pEnv,myObj){}
        java_sql_Date( const ::com::sun::star::util::Date& _rOut );

        operator ::com::sun::star::util::Date();
        static jclass st_getMyClass();
    };


    //************ Class: java.sql.Time


    class java_sql_Time : public java_util_Date
    {
    protected:
    // static data for the class
        static jclass theClass;
    public:
        virtual jclass getMyClass() const override;
        virtual ~java_sql_Time();
        // A ctor that is needed for returning the object
        java_sql_Time( JNIEnv * pEnv, jobject myObj ) : java_util_Date( pEnv, myObj ){}
        java_sql_Time( const ::com::sun::star::util::Time& _rOut );
        operator ::com::sun::star::util::Time();
        static jclass st_getMyClass();
    };


    //************ Class: java.sql.Timestamp

    class java_sql_Timestamp : public java_util_Date
    {
    protected:
    // static data for the class
        static jclass theClass;
    public:
        virtual jclass getMyClass() const override;
        virtual ~java_sql_Timestamp();
        // A ctor that is needed for returning the object
        java_sql_Timestamp( JNIEnv * pEnv, jobject myObj ) : java_util_Date( pEnv, myObj ){}
        java_sql_Timestamp( const ::com::sun::star::util::DateTime& _rOut);
        operator ::com::sun::star::util::DateTime();

        static jclass st_getMyClass();
    };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_TIMESTAMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
