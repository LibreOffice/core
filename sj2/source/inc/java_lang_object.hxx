/*************************************************************************
 *
 *  $RCSfile: java_lang_object.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:54:03 $
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
