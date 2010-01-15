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
jclass java_lang_Boolean::st_getMyClass()
{
    // die Klasse muss nur einmal geholt werden, daher statisch
    if( !theClass )
        theClass = findMyClass("java/lang/Boolean");
    return theClass;
}

jclass java_lang_Boolean::getMyClass() const
{
    return st_getMyClass();
}

java_lang_Boolean::java_lang_Boolean( sal_Bool _par0 ): java_lang_Object( NULL, (jobject)NULL )
{
    SDBThreadAttach t;
    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    static const char * cSignature = "(Z)V";
    jobject tempObj;
    static jmethodID mID(NULL);
    obtainMethodId(t.pEnv, "<init>",cSignature, mID);
    tempObj = t.pEnv->NewObject( getMyClass(), mID, _par0 );
    saveRef( t.pEnv, tempObj );
    t.pEnv->DeleteLocalRef( tempObj );
    // und aufraeumen
}


