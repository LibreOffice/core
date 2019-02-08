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

#include <java/sql/Timestamp.hxx>
#include <java/tools.hxx>
#include <connectivity/dbconversion.hxx>

using namespace ::comphelper;
using namespace connectivity;

//************ Class: java.sql.Date


jclass java_sql_Date::theClass = nullptr;
java_sql_Date::java_sql_Date( const css::util::Date& _rOut ) : java_util_Date( nullptr, nullptr )
{
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    jvalue args[1];
    // Convert parameters
    OUString sDateStr;
    sDateStr = ::dbtools::DBTypeConversion::toDateString(_rOut);
    args[0].l = convertwchar_tToJavaString(t.pEnv,sDateStr);

    // Turn of Java-Call for the constructor
    // initialise temporary variables
    jobject tempObj;
    static jmethodID mID(nullptr);
    if ( !mID )
    {
        static const char * const cSignature = "(Ljava/lang/String;)Ljava/sql/Date;";
        mID  = t.pEnv->GetStaticMethodID( getMyClass(), "valueOf", cSignature );
    }
    OSL_ENSURE(mID,"Unknown method id!");
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


java_sql_Date::operator css::util::Date()
{
    return ::dbtools::DBTypeConversion::toDate(toString());
}


//************ Class: java.sql.Time


jclass java_sql_Time::theClass = nullptr;

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
java_sql_Time::java_sql_Time( const css::util::Time& _rOut ): java_util_Date( nullptr, nullptr )
{
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    jvalue args[1];
    // Convert parameters
    OUString sDateStr;
    // java.sql.Time supports only whole seconds...
    sDateStr = ::dbtools::DBTypeConversion::toTimeStringS(_rOut);
    args[0].l = convertwchar_tToJavaString(t.pEnv,sDateStr);

    // Turn off Java-Call for the constructor
    // initialise temporary variables
    jobject tempObj;
    static jmethodID mID(nullptr);
    if ( !mID )
    {
        static const char * const cSignature = "(Ljava/lang/String;)Ljava/sql/Time;";
        mID  = t.pEnv->GetStaticMethodID( getMyClass(), "valueOf", cSignature );
    }
    OSL_ENSURE(mID,"Unknown method id!");
    tempObj = t.pEnv->CallStaticObjectMethod( getMyClass(), mID, args[0].l );
    t.pEnv->DeleteLocalRef(static_cast<jstring>(args[0].l));
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    // and clean
}

java_sql_Time::operator css::util::Time()
{
    return ::dbtools::DBTypeConversion::toTime(toString());
}

//************ Class: java.sql.Timestamp


jclass java_sql_Timestamp::theClass = nullptr;

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

java_sql_Timestamp::java_sql_Timestamp(const css::util::DateTime& _rOut)
                   :java_util_Date( nullptr, nullptr )
{
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    jvalue args[1];
    // Convert parameters
    OUString sDateStr;
    sDateStr = ::dbtools::DBTypeConversion::toDateTimeString(_rOut);

    args[0].l = convertwchar_tToJavaString(t.pEnv,sDateStr);

    // Turn off Java-Call for the constructor
    // initialise temporary variables
    jobject tempObj;
    static jmethodID mID(nullptr);
    if ( !mID )
    {
        static const char * const cSignature = "(Ljava/lang/String;)Ljava/sql/Timestamp;";
        mID  = t.pEnv->GetStaticMethodID( getMyClass(), "valueOf", cSignature );
    }
    OSL_ENSURE(mID,"Unknown method id!");
    tempObj = t.pEnv->CallStaticObjectMethod( getMyClass(), mID, args[0].l );

    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    // and clean
}


java_sql_Timestamp::operator css::util::DateTime()
{
    return ::dbtools::DBTypeConversion::toDateTime(toString());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
