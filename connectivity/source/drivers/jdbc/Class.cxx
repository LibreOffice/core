/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Class.cxx,v $
 * $Revision: 1.14 $
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
#include "java/lang/Class.hxx"
#include "java/tools.hxx"
#include <rtl/ustring.hxx>

using namespace connectivity;
//**************************************************************
//************ Class: java.lang.Class
//**************************************************************

jclass java_lang_Class::theClass = 0;

java_lang_Class::~java_lang_Class()
{}

jclass java_lang_Class::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t;
        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass("java/lang/Class"); OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_lang_Class::saveClassRef( jclass pClass )
{
    if( pClass==NULL  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

java_lang_Class * java_lang_Class::forName( const ::rtl::OUString& _par0 )
{
    jobject out(NULL);
    SDBThreadAttach t;
    if( t.pEnv )
    {
        ::rtl::OString sClassName = ::rtl::OUStringToOString(_par0, RTL_TEXTENCODING_JAVA_UTF8);
        sClassName = sClassName.replace('.','/');
        out = t.pEnv->FindClass(sClassName);
        ThrowSQLException(t.pEnv,0);
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? NULL : new java_lang_Class( t.pEnv, out );
}

sal_Bool java_lang_Class::isAssignableFrom( java_lang_Class * _par0 )
{
    jboolean out(0);
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "(Ljava/lang/Class;)Z";
        static const char * cMethodName = "isAssignableFrom";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jvalue args[1];
            // Parameter konvertieren
            args[0].l = _par0 != NULL ? ((java_lang_Object *)_par0)->getJavaObject() : NULL;
            out = t.pEnv->CallBooleanMethod( object, mID, args[0].l );
            ThrowSQLException(t.pEnv,0);
            // und aufraeumen
        } //mID
    } //t.pEnv
    return out;
}

java_lang_Object * java_lang_Class::newInstance()
{
    jobject out(NULL);
    SDBThreadAttach t;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/Object;";
        static const char * cMethodName = "newInstance";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,0);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out==0 ? NULL : new java_lang_Object( t.pEnv, out );
}

jobject java_lang_Class::newInstanceObject()
{
    jobject out(NULL);
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/Object;";
        static const char * cMethodName = "newInstance";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,0);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return out;
}

::rtl::OUString java_lang_Class::getName()
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getName";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,0);
            aStr = JavaString2String(t.pEnv, (jstring)out );
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}

