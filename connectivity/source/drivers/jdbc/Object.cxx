/*************************************************************************
 *
 *  $RCSfile: Object.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:22 $
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


#ifndef _CONNECTIVITY_JAVA_LANG_OBJJECT_HXX_
#include "java/lang/Class.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_JAVA_XJAVATHREADREGISTER_11_HPP_
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>
#endif
#ifndef _COM_SUN_STAR_JAVA_XJAVAVM_HPP_
#include <com/sun/star/java/XJavaVM.hpp>
#endif
#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#include "java/tools.hxx"
#endif
#ifndef _CONNECTIVITY_JAVA_SQL_SQLEXCEPTION_HXX_
#include "java/sql/SQLException.hxx"
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_DYNLOAD_HXX_
#include <vos/dynload.hxx>
#endif

#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

using namespace connectivity;

namespace starjava  = com::sun::star::java;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


JavaVM * pJVM;
Reference< ::starjava::XJavaThreadRegister_11 > xRG11Ref;
sal_Bool        bJRE_Error = sal_False;
JavaVM_ *   pJRE_javaVM = NULL;

int SDB_JRE_InitJava(const Reference<XMultiServiceFactory >& _rxFactory)
{
    int result = 0;
    JNIEnv * pEnv = NULL;

    try
    {
        Reference< ::starjava::XJavaVM > xVM(_rxFactory->createInstance(
            rtl::OUString::createFromAscii("com.sun.star.java.JavaVirtualMachine")), UNO_QUERY);

        OSL_ENSHURE(_rxFactory.is(),"SDB_JRE_InitJava: I have no factory!");
        if (!xVM.is() || !_rxFactory.is())
            throw Exception(); // -2;

        Sequence<sal_Int8> processID(16);
        rtl_getGlobalProcessId( (sal_uInt8*) processID.getArray() );

        Any uaJVM = xVM->getJavaVM( processID );

        if (!uaJVM.hasValue())
            throw Exception(); // -5
        else
        {
            sal_Int32 nValue;
            uaJVM >>= nValue;
            pJVM = (JavaVM *)nValue;
        }

        xRG11Ref = Reference< ::starjava::XJavaThreadRegister_11 >(xVM, UNO_QUERY);
        if (xRG11Ref.is())
            xRG11Ref->registerThread();

          pJVM->AttachCurrentThread(reinterpret_cast<void**>(&pEnv), NULL);

        /**
         * Ist zur Zeit nicht funktionsfaehig, wegen Solaris Deadlock.
         */
//      initStreamObserver(pEnv);
//      if(pEnv->ExceptionOccurred())
//      {
//          pEnv->ExceptionDescribe();
//          pEnv->ExceptionClear();
//      }

    }
    catch (Exception e)
    {
        if (pEnv && pEnv->ExceptionOccurred())
        {
            pEnv->ExceptionDescribe();
            pEnv->ExceptionClear();
        }

        result = -1;
    }

    if(pEnv)
    {
        if(xRG11Ref.is())
            xRG11Ref->revokeThread();

        if(!xRG11Ref.is() || !xRG11Ref->isThreadAttached())
            pJVM->DetachCurrentThread();
    }

    return result;
}

SDBThreadAttach::SDBThreadAttach() : bDetach(sal_False), pEnv(NULL)
{
    attachThread(pEnv);
}

SDBThreadAttach::SDBThreadAttach(const Reference<XMultiServiceFactory >& _rxFactory) : bDetach(sal_False), pEnv(NULL)
{
    attachThread(pEnv,_rxFactory);
}

SDBThreadAttach::~SDBThreadAttach()
{
    detachThread();
}

void SDBThreadAttach::attachThread(JNIEnv * &pEnv,const Reference<XMultiServiceFactory >& _rxFactory)
{
    xInit(_rxFactory);

    if(xRG11Ref.is())
    {
        xRG11Ref->registerThread();
        pJVM->AttachCurrentThread(reinterpret_cast<void**>(&pEnv), NULL);
    }
}

void SDBThreadAttach::detachThread()
{
    xInit();

    if(xRG11Ref.is())
    {
        xRG11Ref->revokeThread();
        if(!xRG11Ref->isThreadAttached())
            pJVM->DetachCurrentThread();
    }
}

void SDBThreadAttach::xInit(const Reference<XMultiServiceFactory >& _rxFactory)
{
    StartJava(_rxFactory);
}

sal_Bool SDBThreadAttach::IsJavaErrorOccured()
{
    return bJRE_Error;
}

void SDBThreadAttach::setError()
{
    bJRE_Error = sal_True;
}

static sal_Bool bStarted = sal_False;
int SDBThreadAttach::StartJava(const Reference<XMultiServiceFactory >& _rxFactory)
{
    static sal_Bool bRecFlag = sal_False;
    static int err = 0;
    if( !bStarted )
    {
        vos::OGuard aGuard( vos::OMutex::getGlobalMutex() );
        if( !bStarted && !bRecFlag)
        {
            bRecFlag = sal_True;
            //  Application::EnterMultiThread();
            // bStarted = sal_True must set after the VM is complete initialized
            err = SDB_JRE_InitJava(_rxFactory);
            if(err)
                setError();
            bStarted = sal_True;
        }
    }
//  return bJRE_Error ? -1 : 0;
    return err;
}

// statische Variablen der Klasse:
jclass java_lang_Object::theClass = 0;
sal_uInt32 java_lang_Object::nObjCount = 0;

jclass java_lang_Object::getMyClass()
{
    if( !theClass )
    {
//      JNIEnv * pEnv = SDBThreadAttach::GetEnv();
        SDBThreadAttach t;

        if( !t.pEnv ) return (jclass)NULL;
        jclass tempClass = t.pEnv->FindClass( "java/lang/Object" );
        theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
        t.pEnv->DeleteLocalRef( tempClass );
    }
    return theClass;
}
// der eigentliche Konstruktor
java_lang_Object::java_lang_Object(const Reference<XMultiServiceFactory >& _rxFactory)
            : object( 0 ),m_xFactory(_rxFactory)
{
    SDBThreadAttach t(m_xFactory);
    if( !t.pEnv )
        return;

    // Java-Call fuer den Konstruktor absetzen
    // temporaere Variable initialisieren
    char * cSignature = "()V";
    jclass tempClass;
    jmethodID mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
    tempClass = (jclass)t.pEnv->NewObjectA( getMyClass(), mID, NULL );
    saveRef( t.pEnv, tempClass );
    t.pEnv->DeleteLocalRef( tempClass );
}

// der protected-Konstruktor fuer abgeleitete Klassen
java_lang_Object::java_lang_Object( JNIEnv * pXEnv, jobject myObj )
    : object( NULL )
{
    SDBThreadAttach t;
    if( t.pEnv && myObj )
        object = t.pEnv->NewGlobalRef( myObj );
}

java_lang_Object::~java_lang_Object()
{
    if( object )
    {
        SDBThreadAttach t;
        if( t.pEnv )
            t.pEnv->DeleteGlobalRef( object );
    }
}

// der protected-Konstruktor fuer abgeleitete Klassen
void java_lang_Object::saveRef( JNIEnv * pXEnv, jobject myObj )
{
    OSL_ENSHURE( myObj, "object in c++ -> Java Wrapper" );
    SDBThreadAttach t;
    if( t.pEnv && myObj )
        object = t.pEnv->NewGlobalRef( myObj );
}


java_lang_Class * java_lang_Object::getClass()
{
    jobject out;
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/Class;";
        char * cMethodName = "getClass";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            out = t.pEnv->CallObjectMethodA( object, mID, NULL );

            return new java_lang_Class( t.pEnv, out );
        } //mID
    } //pEnv
    return NULL;
}

::rtl::OUString java_lang_Object::toString()
{

    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        char * cSignature = "()Ljava/lang/String;";
        char * cMethodName = "toString";
        // Java-Call absetzen
        jmethodID mID = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );
        if( mID )
        {
            jstring out(0);
            out = (jstring)t.pEnv->CallObjectMethod( object, mID);

            if(out)
                aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //pEnv
    return  aStr;
}
// --------------------------------------------------------------------------------
void java_lang_Object::ThrowSQLException(JNIEnv * pEnv,const Reference< XInterface> & _rContext) throw(SQLException, RuntimeException)
{
    jthrowable jThrow = NULL;
    if(pEnv && (jThrow = pEnv->ExceptionOccurred()))
    {
        if(pEnv->IsInstanceOf(jThrow,java_sql_SQLException_BASE::getMyClass()))
        {
            throw java_sql_SQLException(java_sql_SQLException_BASE(pEnv,jThrow),_rContext);
        }
        else if(pEnv->IsInstanceOf(jThrow,java_lang_Throwable::getMyClass()))
        {
            java_lang_Throwable *pThrow = new java_lang_Throwable(pEnv,jThrow);
            ::rtl::OUString aMsg = pThrow->getMessage();
            delete pThrow;
            throw RuntimeException(aMsg,_rContext);
        }
        pEnv->ExceptionClear();
    }
}


