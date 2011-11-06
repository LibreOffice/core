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
    // Parameter konvertieren
    ::rtl::OUString sDateStr;
    sDateStr = ::dbtools::DBTypeConversion::toDateString(_rOut);
    args[0].l = convertwchar_tToJavaString(t.pEnv,sDateStr);

    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    static const char * cSignature = "(Ljava/lang/String;)Ljava/sql/Date;";
    jobject tempObj;
    static jmethodID mID(NULL);
    if ( !mID  )
        mID  = t.pEnv->GetStaticMethodID( getMyClass(), "valueOf", cSignature );OSL_ENSURE(mID,"Unknown method id!");
    tempObj = t.pEnv->CallStaticObjectMethod( getMyClass(), mID, args[0].l );
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    // und aufraeumen
}

java_sql_Date::~java_sql_Date()
{}

jclass java_sql_Date::getMyClass() const
{
    return st_getMyClass();
}
jclass java_sql_Date::st_getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
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
    // die Klasse muss nur einmal geholt werden, daher statisch
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
    // Parameter konvertieren
    ::rtl::OUString sDateStr;
    sDateStr = ::dbtools::DBTypeConversion::toTimeString(_rOut);
    args[0].l = convertwchar_tToJavaString(t.pEnv,sDateStr);

    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    static const char * cSignature = "(Ljava/lang/String;)Ljava/sql/Time;";
    jobject tempObj;
    static jmethodID mID(NULL);
    if ( !mID  )
        mID  = t.pEnv->GetStaticMethodID( getMyClass(), "valueOf", cSignature );OSL_ENSURE(mID,"Unknown method id!");
    tempObj = t.pEnv->CallStaticObjectMethod( getMyClass(), mID, args[0].l );
    t.pEnv->DeleteLocalRef((jstring)args[0].l);
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    // und aufraeumen
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
    // die Klasse muss nur einmal geholt werden, daher statisch
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
    // Parameter konvertieren
    ::rtl::OUString sDateStr;
    sDateStr = ::dbtools::DBTypeConversion::toDateTimeString(_rOut);

    args[0].l = convertwchar_tToJavaString(t.pEnv,sDateStr);

    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    static const char * cSignature = "(Ljava/lang/String;)Ljava/sql/Timestamp;";
    jobject tempObj;
    static jmethodID mID(NULL);
    if ( !mID  )
        mID  = t.pEnv->GetStaticMethodID( getMyClass(), "valueOf", cSignature );OSL_ENSURE(mID,"Unknown method id!");
    tempObj = t.pEnv->CallStaticObjectMethod( getMyClass(), mID, args[0].l );

    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    // und aufraeumen
}

sal_Int32 java_sql_Timestamp::getNanos()
{
    static jmethodID mID(NULL);
    return callIntMethod("getNanos",mID);
}

void java_sql_Timestamp::setNanos( sal_Int32 _par0 )
{
    static jmethodID mID(NULL);
    callVoidMethodWithIntArg("setNanos",mID,_par0);
}
// -----------------------------------------------------------------------------
java_sql_Timestamp::operator ::com::sun::star::util::DateTime()
{
    return ::dbtools::DBTypeConversion::toDateTime(toString());
}
// -----------------------------------------------------------------------------


