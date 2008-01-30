/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JBigDecimal.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 07:54:07 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "java/math/BigDecimal.hxx"
#include "java/tools.hxx"
#include "resource/jdbc_log.hrc"
using namespace connectivity;
//**************************************************************
//************ Class: java.lang.Boolean
//**************************************************************

jclass java_math_BigDecimal::theClass = 0;

java_math_BigDecimal::~java_math_BigDecimal()
{}

jclass java_math_BigDecimal::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t;
        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass("java/math/BigDecimal"); OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_math_BigDecimal::saveClassRef( jclass pClass )
{
    if( pClass==NULL  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

java_math_BigDecimal::java_math_BigDecimal( const ::rtl::OUString& _par0 ): java_lang_Object( NULL, (jobject)NULL ){
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    static const char * cSignature = "(Ljava/lang/String;)V";
    jobject tempObj;
    static jmethodID mID = NULL;
    if ( !mID  )
        mID  = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );OSL_ENSURE(mID,"Unknown method id!");

    jstring str = convertwchar_tToJavaString(t.pEnv,_par0.replace(',','.'));
    tempObj = t.pEnv->NewObject( getMyClass(), mID, str );
    t.pEnv->DeleteLocalRef(str);
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    ThrowSQLException( t.pEnv, NULL );
    // und aufraeumen
}

java_math_BigDecimal::java_math_BigDecimal( const double& _par0 ): java_lang_Object( NULL, (jobject)NULL ){
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    static const char * cSignature = "(D)V";
    jobject tempObj;
    static jmethodID mID = NULL;
    if ( !mID  )
        mID  = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );OSL_ENSURE(mID,"Unknown method id!");
    tempObj = t.pEnv->NewObject( getMyClass(), mID, _par0 );
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    ThrowSQLException( t.pEnv, NULL );
    // und aufraeumen
}

