/*************************************************************************
 *
 *  $RCSfile: Timestamp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_JAVA_SQL_TIMESTAMP_HXX_
#include "java/sql/Timestamp.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CONNECTIVITY_DATECONVERSION_HXX_
#include "DateConversion.hxx"
#endif
using namespace connectivity;
//**************************************************************
//************ Class: java.sql.Date
//**************************************************************

jclass java_sql_Date::theClass = 0;

java_sql_Date::~java_sql_Date()
{}

jclass java_sql_Date::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
        if( !t.pEnv ) return (jclass)0;
        jclass tempClass = t.pEnv->FindClass("java/sql/Date"); OSL_ENSHURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_sql_Date::saveClassRef( jclass pClass )
{
    if( SDBThreadAttach::IsJavaErrorOccured() || pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

//**************************************************************
//************ Class: java.sql.Time
//**************************************************************

jclass java_sql_Time::theClass = 0;

java_sql_Time::~java_sql_Time()
{}

jclass java_sql_Time::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
        if( !t.pEnv ) return (jclass)0;
        jclass tempClass = t.pEnv->FindClass("java/sql/Time"); OSL_ENSHURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_sql_Time::saveClassRef( jclass pClass )
{
    if( SDBThreadAttach::IsJavaErrorOccured() || pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

java_sql_Time::java_sql_Time( const ::com::sun::star::util::Time& _rOut ): java_util_Date( NULL, (jobject)NULL )
{
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    jvalue args[1];
    // Parameter konvertieren
    args[0].j = DateConversion::toINT32(_rOut);
    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    char * cSignature = "(J)V";
    jobject tempObj;
    jmethodID mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
    tempObj = t.pEnv->NewObjectA( getMyClass(), mID, args );
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    // und aufraeumen
}

//**************************************************************
//************ Class: java.sql.Timestamp
//**************************************************************

jclass java_sql_Timestamp::theClass = 0;

java_sql_Timestamp::~java_sql_Timestamp()
{}

jclass java_sql_Timestamp::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t; OSL_ENSHURE(t.pEnv,"Java Enviroment gelöscht worden!");
        if( !t.pEnv ) return (jclass)0;
        jclass tempClass = t.pEnv->FindClass("java/sql/Timestamp"); OSL_ENSHURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_sql_Timestamp::saveClassRef( jclass pClass )
{
    if( SDBThreadAttach::IsJavaErrorOccured() || pClass==0  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

java_sql_Timestamp::java_sql_Timestamp(const ::com::sun::star::util::DateTime& _rOut)
                   :java_util_Date( NULL, (jobject)NULL )
{
        SDBThreadAttach t;
    if( !t.pEnv )
        return;
    jvalue args[1];
        // Parameter konvertieren
    args[0].j = (jlong)DateConversion::toINT64(_rOut);
    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    char * cSignature = "(J)V";
    jobject tempObj;
    jmethodID mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
    tempObj = t.pEnv->NewObjectA( getMyClass(), mID, args );
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    // und aufraeumen
}

sal_Int32 java_sql_Timestamp::getNanos()
{
    jint out(0);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        char * cSignature = "()I";
        char * cMethodName = "getNanos";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}

void java_sql_Timestamp::setNanos( sal_Int32 _par0 )
{
    SDBThreadAttach t;
    if( t.pEnv ){
    jvalue args[1];
        // Parameter konvertieren
        args[0].i = (sal_Int32)_par0;
        // temporaere Variable initialisieren
        char * cSignature = "(I)V";
        char * cMethodName = "setNanos";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID ){
            t.pEnv->CallVoidMethod( object, mID, args[0].i );
            // und aufraeumen
        } //mID
    } //t.pEnv
}

