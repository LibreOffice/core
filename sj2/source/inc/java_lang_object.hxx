/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: java_lang_object.hxx,v $
 * $Revision: 1.3 $
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
#include <jni.h>
#ifdef OS2
#include <typedefs.h>
#endif

#ifndef _SJ_JAVA_LANG_OBJECT_HXX
#define _SJ_JAVA_LANG_OBJECT_HXX

#include <tools/string.hxx>


#ifdef HAVE_64BIT_POINTERS
#error "no 64 bit pointer"
#else
#ifdef OS2
#define INT64_TO_PVOID(x) (void *)x.lo
inline jlong Make_Os2_Int64( INT32 hi, INT32 lo ) {jlong x = CONST64( hi, lo ); return x; }
#define PVOID_TO_INT64(x) Make_Os2_Int64( 0, (INT32)x )
#else //OS2
#define PVOID_TO_INT64(x) (jlong)(INT32)x
#define INT64_TO_PVOID(x) (void *)x
#endif //Os2
#endif

//=====================================================================
class java_lang_Class;
class  java_lang_Object
{
    // Zuweisungsoperator und Copy Konstruktor sind verboten
    java_lang_Object& operator = (java_lang_Object&) { return *this;};
    java_lang_Object(java_lang_Object&) {};

    static jclass getMyClass();
    // nur zum Zerstoeren des C++ Pointers in vom JSbxObject
    // abgeleiteten Java Objekten
    //static jclass getJSbxObjectClass();

protected:
    // der JAVA Handle zu dieser Klasse
    jobject object;
    // Klassendefinition

    // neu in SJ2:
    static jclass theClass;             // die Klasse braucht nur einmal angefordert werden !
    static jclass theJSbxObjectClass;   // die Klasse braucht nur einmal angefordert werden !
    static ULONG nObjCount;             // Zaehler fuer die Anzahl der Instanzen

public:
    // der Konstruktor, der fuer die abgeleiteten Klassen verwendet
    // werden soll.
    java_lang_Object( JNIEnv * pEnv, jobject myObj );
    // der eigentliche Konstruktor
    java_lang_Object();

    virtual ~java_lang_Object();

    void                saveRef( JNIEnv * pEnv, jobject myObj );
    jobject             getJavaObject() const { return object; }
    java_lang_Object *  GetWrapper() { return this; }

    java_lang_Class * getClass();

};

#endif
