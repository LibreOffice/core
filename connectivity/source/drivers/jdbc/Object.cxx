/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Object.cxx,v $
 * $Revision: 1.24 $
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


#ifndef _CONNECTIVITY_JAVA_LANG_OBJJECT_HXX_
#include "java/lang/Class.hxx"
#endif
#include "connectivity/CommonTools.hxx"
#include <com/sun/star/uno/Exception.hpp>
#include "java/tools.hxx"
#include "java/sql/SQLException.hxx"
#include <vos/process.hxx>
#include <vos/mutex.hxx>
#include <osl/thread.h>
#include <com/sun/star/uno/Sequence.hxx>

#include "resource/jdbc_log.hrc"

#include <comphelper/logging.hxx>

#include <memory>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


// -----------------------------------------------------------------------------
::rtl::Reference< jvmaccess::VirtualMachine > getJavaVM2(const ::rtl::Reference< jvmaccess::VirtualMachine >& _rVM = ::rtl::Reference< jvmaccess::VirtualMachine >(),
                                                        sal_Bool _bSet = sal_False)
{
    static ::rtl::Reference< jvmaccess::VirtualMachine > s_VM;
    if ( _rVM.is() || _bSet )
        s_VM = _rVM;
    return s_VM;
}
// -----------------------------------------------------------------------------
::rtl::Reference< jvmaccess::VirtualMachine > java_lang_Object::getVM(const Reference<XMultiServiceFactory >& _rxFactory)
{
    ::rtl::Reference< jvmaccess::VirtualMachine > xVM = getJavaVM2();
    if ( !xVM.is() && _rxFactory.is() )
        xVM = getJavaVM2(::connectivity::getJavaVM(_rxFactory));

    return xVM;
}
// -----------------------------------------------------------------------------
SDBThreadAttach::SDBThreadAttach()
 : m_aGuard(java_lang_Object::getVM())
 , pEnv(NULL)
{
    pEnv = m_aGuard.getEnvironment();
    OSL_ENSURE(pEnv,"Environment is nULL!");
}
// -----------------------------------------------------------------------------
SDBThreadAttach::~SDBThreadAttach()
{
}
// -----------------------------------------------------------------------------
oslInterlockedCount& getJavaVMRefCount()
{
    static oslInterlockedCount s_nRefCount = 0;
    return s_nRefCount;
}
// -----------------------------------------------------------------------------
void SDBThreadAttach::addRef()
{
    osl_incrementInterlockedCount(&getJavaVMRefCount());
}
// -----------------------------------------------------------------------------
void SDBThreadAttach::releaseRef()
{
    osl_decrementInterlockedCount(&getJavaVMRefCount());
    if ( getJavaVMRefCount() == 0 )
    {
        getJavaVM2(::rtl::Reference< jvmaccess::VirtualMachine >(),sal_True);
    }
}
// -----------------------------------------------------------------------------
// statische Variablen der Klasse:
jclass java_lang_Object::theClass = 0;

jclass java_lang_Object::getMyClass()
{
    if( !theClass )
    {
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
            : m_xFactory(_rxFactory),object( 0 )
{
    SDBThreadAttach::addRef();
}

// der protected-Konstruktor fuer abgeleitete Klassen
java_lang_Object::java_lang_Object( JNIEnv * pXEnv, jobject myObj )
    : object( NULL )
{
    SDBThreadAttach::addRef();
    if( pXEnv && myObj )
        object = pXEnv->NewGlobalRef( myObj );
}

java_lang_Object::~java_lang_Object()
{
    if( object )
    {
        SDBThreadAttach t;
        if( t.pEnv )
            t.pEnv->DeleteGlobalRef( object );
        object = NULL;
    }
    SDBThreadAttach::releaseRef();
}
void java_lang_Object::clearObject(JNIEnv& rEnv)
{
    if( object )
    {
        rEnv.DeleteGlobalRef( object );
        object = NULL;
    }
}

void java_lang_Object::clearObject()
{
    if( object )
    {
        SDBThreadAttach t;
        if( t.pEnv )
            t.pEnv->DeleteGlobalRef( object );
        object = NULL;
    }
}
// der protected-Konstruktor fuer abgeleitete Klassen
void java_lang_Object::saveRef( JNIEnv * pXEnv, jobject myObj )
{
    OSL_ENSURE( myObj, "object in c++ -> Java Wrapper" );
    if( pXEnv && myObj )
        object = pXEnv->NewGlobalRef( myObj );
}


java_lang_Class * java_lang_Object::getClass()
{
    jobject out;
    SDBThreadAttach t;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/Class;";
        static const char * cMethodName = "getClass";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            out = t.pEnv->CallObjectMethodA( object, mID, NULL );
            ThrowSQLException(t.pEnv,NULL);
            return new java_lang_Class( t.pEnv, out );
        } //mID
    } //pEnv
    return NULL;
}

::rtl::OUString java_lang_Object::toString() const
{

    SDBThreadAttach t;
    ::rtl::OUString aStr;
    if( t.pEnv )
    {
        // temporaere Variable initialisieren
        static const char * cSignature = "()Ljava/lang/String;";
        static const char * cMethodName = "toString";
        // Java-Call absetzen
        static jmethodID mID = NULL;
        if ( !mID  )
            mID  = t.pEnv->GetMethodID( getMyClass(), cMethodName, cSignature );OSL_ENSURE(mID,"Unknown method id!");
        if( mID )
        {
            jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID);
            ThrowSQLException(t.pEnv,NULL);
            aStr = JavaString2String(t.pEnv,out);
        } //mID
    } //pEnv
    return  aStr;
}

// --------------------------------------------------------------------------------
namespace
{
    bool    lcl_translateJNIExceptionToUNOException(
        JNIEnv* _pEnvironment, const Reference< XInterface >& _rxContext, SQLException& _out_rException )
    {
        jthrowable jThrow = _pEnvironment ? _pEnvironment->ExceptionOccurred() : NULL;
        if ( !jThrow )
            return false;

        _pEnvironment->ExceptionClear();
            // we have to clear the exception here because we want to handle it itself

        if ( _pEnvironment->IsInstanceOf( jThrow, java_sql_SQLException_BASE::getMyClass() ) )
        {
            ::std::auto_ptr< java_sql_SQLException_BASE > pException( new java_sql_SQLException_BASE( _pEnvironment, jThrow ) );
            _out_rException = SQLException( pException->getMessage(), _rxContext,
                pException->getSQLState(), pException->getErrorCode(), Any() );
            return true;
        }
        else if ( _pEnvironment->IsInstanceOf( jThrow, java_lang_Throwable::getMyClass() ) )
        {
            ::std::auto_ptr< java_lang_Throwable > pThrow( new java_lang_Throwable( _pEnvironment, jThrow ) );
            ::rtl::OUString sMessage = pThrow->getMessage();
            if ( !sMessage.getLength() )
                sMessage = pThrow->getLocalizedMessage();
            if(  !sMessage.getLength() )
                sMessage = pThrow->toString();
            _out_rException = SQLException( sMessage, _rxContext, ::rtl::OUString(), -1, Any() );
            return true;
        }
        else
            _pEnvironment->DeleteLocalRef( jThrow );
        return false;
    }
}

// --------------------------------------------------------------------------------
void java_lang_Object::ThrowLoggedSQLException( const ::comphelper::ResourceBasedEventLogger& _rLogger, JNIEnv* _pEnvironment,
    const Reference< XInterface >& _rxContext )
{
    SQLException aException;
    if ( lcl_translateJNIExceptionToUNOException( _pEnvironment, _rxContext, aException ) )
    {
        _rLogger.log( ::com::sun::star::logging::LogLevel::SEVERE, STR_LOG_THROWING_EXCEPTION, aException.Message, aException.SQLState, aException.ErrorCode );
        throw aException;
    }
}

// --------------------------------------------------------------------------------
void java_lang_Object::ThrowSQLException( JNIEnv* _pEnvironment, const Reference< XInterface>& _rxContext )
{
    SQLException aException;
    if ( lcl_translateJNIExceptionToUNOException( _pEnvironment, _rxContext, aException ) )
        throw aException;
}


