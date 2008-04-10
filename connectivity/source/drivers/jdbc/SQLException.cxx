/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SQLException.cxx,v $
 * $Revision: 1.13 $
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
#include "java/sql/SQLException.hxx"
#include "java/tools.hxx"

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
//**************************************************************
//************ Class: java.sql.SQLException
//**************************************************************
java_sql_SQLException::java_sql_SQLException( const java_sql_SQLException_BASE& _rException,const Reference< XInterface> & _rContext)
    : starsdbc::SQLException(   _rException.getMessage(),
                                _rContext,
                                _rException.getSQLState(),
                                _rException.getErrorCode(),
                                makeAny(_rException.getNextException())
                            )
{
}

java_sql_SQLException_BASE::java_sql_SQLException_BASE( JNIEnv * pEnv, jobject myObj ) : java_lang_Exception( pEnv, myObj )
{
}

jclass java_sql_SQLException_BASE::theClass = 0;

java_sql_SQLException_BASE::~java_sql_SQLException_BASE()
{}


jclass java_sql_SQLException_BASE::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t;
        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass("java/sql/SQLException");
        OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
        if(!tempClass)
        {
            t.pEnv->ExceptionDescribe();
            t.pEnv->ExceptionClear();
        }
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_sql_SQLException_BASE::saveClassRef( jclass pClass )
{
    if( pClass==NULL  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

starsdbc::SQLException java_sql_SQLException_BASE::getNextException()  const
{
    jobject out = NULL;
    SDBThreadAttach t;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/sql/SQLException;";
        static const char * cMethodName = "getNextException";
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
    if( out )
    {
        java_sql_SQLException_BASE  warn_base(t.pEnv,out);
        return (starsdbc::SQLException)java_sql_SQLException(warn_base,0);
    }

    return starsdbc::SQLException();
}

::rtl::OUString java_sql_SQLException_BASE::getSQLState() const
{
    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "getSQLState";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            jstring out = (jstring) t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,0);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //t.pEnv
    // ACHTUNG: der Aufrufer wird Eigentuemer des zurueckgelieferten Zeigers !!!
    return aStr;
}
sal_Int32 java_sql_SQLException_BASE::getErrorCode() const
{
    jint out(0);
    SDBThreadAttach t; OSL_ENSURE(t.pEnv,"Java Enviroment geloescht worden!");
    if( t.pEnv ){

        // temporaere Variable initialisieren
        static const char * cSignature = "()I";
        static const char * cMethodName = "getErrorCode";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID ){
            out = t.pEnv->CallIntMethod( object, mID);
            ThrowSQLException(t.pEnv,0);
        } //mID
    } //t.pEnv
    return (sal_Int32)out;
}

