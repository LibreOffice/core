/*************************************************************************
 *
 *  $RCSfile: Object.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:14:11 $
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
#ifndef _CONNECTIVITY_JAVA_LANG_OBJECT_HXX_
#define _CONNECTIVITY_JAVA_LANG_OBJECT_HXX_

#if STLPORT_VERSION>=321
// jni.h needs cstdarg for std::va_list
#include <cstdarg>
#endif

#ifdef OS2
#include <typedefs.h>
#endif

#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef INCLUDED_JVMACCESS_VIRTUALMACHINE_HXX
#include <jvmaccess/virtualmachine.hxx>
#endif // INCLUDED_JVMACCESS_VIRTUALMACHINE_HXX
#include <memory>

//=====================================================================

#ifdef HAVE_64BIT_POINTERS
#error "no 64 bit pointer"
#else
#ifdef OS2
#define INT64_TO_PVOID(x) (void *)x.lo
inline jlong Make_Os2_Int64( sal_Int32 hi, sal_Int32 lo ) {jlong x = CONST64( hi, lo ); return x; }
#define PVOID_TO_INT64(x) Make_Os2_Int64( 0, (sal_Int32)x )
#else //OS2
#define PVOID_TO_INT64(x) (jlong)(sal_Int32)x
#define INT64_TO_PVOID(x) (void *)x
#endif //Os2
#endif //HAVE_64BIT_POINTERS


namespace connectivity
{
    class SDBThreadAttach
    {
        ::std::auto_ptr< jvmaccess::VirtualMachine::AttachGuard> m_aGuard;
        SDBThreadAttach(SDBThreadAttach&);
    public:
        SDBThreadAttach();
        ~SDBThreadAttach();

        JNIEnv* pEnv;
        void addRef();
        void releaseRef();
    };
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
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    protected:
        // der JAVA Handle zu dieser Klasse
        jobject object;
        // Klassendefinition

        // neu in SJ2:
        static jclass theClass;             // die Klasse braucht nur einmal angefordert werden !
        static jclass theJSbxObjectClass;   // die Klasse braucht nur einmal angefordert werden !
        static sal_uInt32 nObjCount;                // Zaehler fuer die Anzahl der Instanzen

    public:
        // der Konstruktor, der fuer die abgeleiteten Klassen verwendet
        // werden soll.
        java_lang_Object( JNIEnv * pEnv, jobject myObj );
        // der eigentliche Konstruktor
        java_lang_Object(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory=NULL);

        virtual ~java_lang_Object();

        void                saveRef( JNIEnv * pEnv, jobject myObj );
        jobject             getJavaObject() const { return object; }
        java_lang_Object *  GetWrapper() { return this; }

        java_lang_Class *   getClass();

        ::rtl::OUString     toString();
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() { return m_xFactory; }
        static void ThrowSQLException(JNIEnv * pEnv,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> & _rContext) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        static ::rtl::Reference< jvmaccess::VirtualMachine > getVM(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory=NULL);
    };
}
#endif //_CONNECTIVITY_JAVA_LANG_OBJJECT_HXX_


