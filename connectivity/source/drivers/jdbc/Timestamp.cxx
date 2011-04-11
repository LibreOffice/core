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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "java/sql/Timestamp.hxx"
#include "java/tools.hxx"
#include <comphelper/types.hxx>
#include "connectivity/dbconversion.hxx"

using namespace ::comphelper;
using namespace connectivity;
//**************************************************************
//************ Class: java.sql.Date
//**************************************************************
const double fMilliSecondsPerDay = 86400000.0;
jclass java_sql_Date::theClass = 0;
java_sql_Date::java_sql_Date( const ::com::sun::star::util::Date& _rOut ) : java_util_Date( NULL, (jobject)NULL )
{
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    jvalue args[1];
    // Convert parameters
    ::rtl::OUString sDateStr;
    sDateStr = ::dbtools::DBTypeConversion::toDateString(_rOut);
    args[0].l = convertwchar_tToJavaString(t.pEnv,sDateStr);

    // Turn of Java-Call for the constructor
    // initialise temporary variables
    static const char * cSignature = "(Ljava/lang/String;)Ljava/sql/Date;";
    jobject tempObj;
    static jmethodID mID(NULL);
    if ( !mID  )
        mID  = t.pEnv->GetStaticMethodID( getMyClass(), "valueOf", cSignature );OSL_ENSURE(mID,"Unknown method id!");
    tempObj = t.pEnv->CallStaticObjectMethod( getMyClass(), mID, args[0].l );
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    // and clean
}

java_sql_Date::~java_sql_Date()
{}

jclass java_sql_Date::getMyClass() const
{
    return st_getMyClass();
}
jclass java_sql_Date::st_getMyClass()
{
    // the class needs only be fetched once, that is why it is static
    if( !theClass )
        theClass = findMyClass("java/sql/Date");
    return theClass;
}
// -----------------------------------------------------------------------------

java_sql_Date::operator ::com::sun::star::util::Date()
{
    return ::dbtools::DBTypeConversion::toDate(toString());
}

//**************************************************************
//************ Class: java.sql.Time
//**************************************************************

jclass java_sql_Time::theClass = 0;

java_sql_Time::~java_sql_Time()
{}

jclass java_sql_Time::getMyClass() const
{
    return st_getMyClass();
}
jclass java_sql_Time::st_getMyClass()
{
    // the class needs only be fetched once, that is why it is static
    if( !theClass )
        theClass = findMyClass("java/sql/Time");
    return theClass;
}
java_sql_Time::java_sql_Time( const ::com::sun::star::util::Time& _rOut ): java_util_Date( NULL, (jobject)NULL )
{
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    jvalue args[1];
    // Convert parameters
    ::rtl::OUString sDateStr;
    sDateStr = ::dbtools::DBTypeConversion::toTimeString(_rOut);
    args[0].l = convertwchar_tToJavaString(t.pEnv,sDateStr);

    // Turn off Java-Call for the constructor
    // intialise temporary variables
    static const char * cSignature = "(Ljava/lang/String;)Ljava/sql/Time;";
    jobject tempObj;
    static jmethodID mID(NULL);
    if ( !mID  )
        mID  = t.pEnv->GetStaticMethodID( getMyClass(), "valueOf", cSignature );OSL_ENSURE(mID,"Unknown method id!");
    tempObj = t.pEnv->CallStaticObjectMethod( getMyClass(), mID, args[0].l );
    t.pEnv->DeleteLocalRef((jstring)args[0].l);
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    // and clean
}
// -----------------------------------------------------------------------------
java_sql_Time::operator ::com::sun::star::util::Time()
{
    return ::dbtools::DBTypeConversion::toTime(toString());
}
//**************************************************************
//************ Class: java.sql.Timestamp
//**************************************************************

jclass java_sql_Timestamp::theClass = 0;

java_sql_Timestamp::~java_sql_Timestamp()
{}

jclass java_sql_Timestamp::getMyClass() const
{
    return st_getMyClass();
}

jclass java_sql_Timestamp::st_getMyClass()
{
    // the class needs only be fetched once, that is why it is static
    if( !theClass )
        theClass = findMyClass("java/sql/Timestamp");
    return theClass;
}

java_sql_Timestamp::java_sql_Timestamp(const ::com::sun::star::util::DateTime& _rOut)
                   :java_util_Date( NULL, (jobject)NULL )
{
        SDBThreadAttach t;
    if( !t.pEnv )
        return;
    jvalue args[1];
    // Convert parameters
    ::rtl::OUString sDateStr;
    sDateStr = ::dbtools::DBTypeConversion::toDateTimeString(_rOut);

    args[0].l = convertwchar_tToJavaString(t.pEnv,sDateStr);

    // Turn off Java-Call for the constructor
    // initialise temporary variables
    static const char * cSignature = "(Ljava/lang/String;)Ljava/sql/Timestamp;";
    jobject tempObj;
    static jmethodID mID(NULL);
    if ( !mID  )
        mID  = t.pEnv->GetStaticMethodID( getMyClass(), "valueOf", cSignature );OSL_ENSURE(mID,"Unknown method id!");
    tempObj = t.pEnv->CallStaticObjectMethod( getMyClass(), mID, args[0].l );

    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    // and clean
}

// -----------------------------------------------------------------------------
java_sql_Timestamp::operator ::com::sun::star::util::DateTime()
{
    return ::dbtools::DBTypeConversion::toDateTime(toString());
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
