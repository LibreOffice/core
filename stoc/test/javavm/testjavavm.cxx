/*************************************************************************
 *
 *  $RCSfile: testjavavm.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:36 $
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

//#include <iostream>
#include <stdio.h>
#include <uno/api.h>

#include <vos/diagnose.hxx>
#include <vos/dynload.hxx>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/weak.hxx>

#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/java/XJavaVM.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/java/XJavaThreadRegister/XJavaThreadRegister_11.hpp>

//#include <cppuhelper/implbase1.hxx>

using namespace std;
using namespace rtl;
using namespace cppu;
using namespace vos;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
//using namespace com::sun::star::reflection;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::java;
using namespace com::sun::star::java::XJavaThreadRegister;


sal_Bool testJavaVM(const Reference< XMultiServiceFactory > & xMgr )
{

    Reference<XInterface> xXInt= xMgr->createInstance(L"com.sun.star.java.JavaVirtualMachine");
    if( ! xXInt.is())
        return sal_False;
    Reference<XJavaVM> xVM( xXInt, UNO_QUERY);
    if( ! xVM.is())
        return sal_False;
    Reference<XJavaThreadRegister_11> xreg11(xVM, UNO_QUERY);
    if( ! xreg11.is())
        return sal_False;

    sal_Bool b= xreg11->isThreadAttached();
    xreg11->registerThread();
    b= xreg11->isThreadAttached();
    xreg11->revokeThread();
    b= xreg11->isThreadAttached();

    Uik aMachineId;
    aMachineId.m_Data1= 0;
    aMachineId.m_Data2= 0;
    aMachineId.m_Data3= 0;
    aMachineId.m_Data4= 0;
    aMachineId.m_Data5= 0;

    b= xVM->isVMEnabled();
    b= xVM->isVMStarted();

    Any anyVM = xVM->getJavaVM( aMachineId, UNO_getProcessIdentifier());

    b= xVM->isVMEnabled();
    b= xVM->isVMStarted();


    JavaVM* _jvm= *(JavaVM**) anyVM.getValue();
    JNIEnv *p_env;
    if( _jvm->AttachCurrentThread( &p_env, 0))
        return sal_False;

//  jclass aJProg = p_env->FindClass("TestJavaVM");
//  if( p_env->ExceptionOccurred()){
//      p_env->ExceptionDescribe();
//      p_env->ExceptionClear();
//  }
//
//  jmethodID mid= p_env->GetStaticMethodID( aJProg,"main", "([Ljava/lang/String;)V");

    jclass cls = p_env->FindClass( "TestJavaVM");
    if (cls == 0) {
        VOS_TRACE( "Can't find Prog class\n");
        exit(1);
    }

//   jmethodID methid = p_env->GetStaticMethodID( cls, "main", "([Ljava/lang/String;)V");
//    if (methid == 0) {
//        VOS_TRACE("Can't find Prog.main\n");
//        exit(1);
//    }

//    jstring jstr = p_env->NewStringUTF(" from C!");
//    if (jstr == 0) {
//        VOS_TRACE("Out of memory\n");
//        exit(1);
//    }
//    jobjectArray args = p_env->NewObjectArray( 1,
//                        p_env->FindClass("java/lang/String"), jstr);
//    if (args == 0) {
//        VOS_TRACE( "Out of memory\n");
//        exit(1);
//    }
//    p_env->CallStaticVoidMethod( cls, methid, args);


    jmethodID id = p_env->GetStaticMethodID( cls, "getInt", "()I");
    if( id)
    {
        jint _i= p_env->CallStaticIntMethod(cls, id);
    }

    if( p_env->ExceptionOccurred()){
        p_env->ExceptionDescribe();
        p_env->ExceptionClear();
    }


    _jvm->DetachCurrentThread();
    return sal_True;
}

extern
#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int __cdecl main( int argc, char * argv[] )
#endif
{
    Reference< XMultiServiceFactory > xMgr( createRegistryServiceFactory( "applicat.rdb" ) );

    sal_Bool bSucc = sal_False;
    try
    {
        Reference<com::sun::star::registry::XImplementationRegistration> xImplReg(
            xMgr->createInstance( L"com.sun.star.registry.ImplementationRegistration" ), UNO_QUERY );
        VOS_ENSHURE( xImplReg.is(), "### no impl reg!" );

        sal_Char pLibName[256];
        ORealDynamicLoader::computeLibraryName("je558mi", pLibName, 255);
        xImplReg->registerImplementation(
            L"com.sun.star.loader.SharedLibrary", OWString::createFromAscii(pLibName), Reference< XSimpleRegistry >() );

        bSucc = testJavaVM( xMgr );
    }
    catch (Exception & rExc)
    {
        VOS_ENSHURE( sal_False, "### exception occured!" );
        OString aMsg( OWStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        VOS_TRACE( "### exception occured: " );
        VOS_TRACE( aMsg.getStr() );
        VOS_TRACE( "\n" );
    }

    Reference< XComponent >( xMgr, UNO_QUERY )->dispose();
    printf("javavm %s", bSucc ? "succeeded" : "failed");
//  cout << "javavm " << (bSucc ? "succeeded" : "failed") << " !" << endl;
    return (bSucc ? 0 : -1);
}


