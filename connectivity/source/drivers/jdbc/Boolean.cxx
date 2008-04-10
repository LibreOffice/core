/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Boolean.cxx,v $
 * $Revision: 1.9 $
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
#include "java/lang/Boolean.hxx"
using namespace connectivity;
//**************************************************************
//************ Class: java.lang.Boolean
//**************************************************************

jclass java_lang_Boolean::theClass = 0;

java_lang_Boolean::~java_lang_Boolean()
{}

jclass java_lang_Boolean::getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass ){
        SDBThreadAttach t;
        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass("java/lang/Boolean"); OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
        jclass globClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
        saveClassRef( globClass );
    }
    return theClass;
}

void java_lang_Boolean::saveClassRef( jclass pClass )
{
    if( pClass==NULL  )
        return;
    // der uebergebe Klassen-Handle ist schon global, daher einfach speichern
    theClass = pClass;
}

java_lang_Boolean::java_lang_Boolean( sal_Bool _par0 ): java_lang_Object( NULL, (jobject)NULL ){
    SDBThreadAttach t;
    if( !t.pEnv )
        return;
    jvalue args[1];
        // Parameter konvertieren
        args[0].z = _par0;
    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    static const char * cSignature = "(Z)V";
    jobject tempObj;
    static jmethodID mID = NULL;
    if ( !mID  )
        mID  = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );OSL_ENSURE(mID,"Unknown method id!");
    tempObj = t.pEnv->NewObjectA( getMyClass(), mID, args );
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    // und aufraeumen
}


