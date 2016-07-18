/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "java/lang/Class.hxx"
#include <connectivity/CommonTools.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include "java/tools.hxx"
#include "java/sql/SQLException.hxx"
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include "java/LocalRef.hxx"
#include "resource/jdbc_log.hrc"

#include <comphelper/logging.hxx>

#include <memory>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


::rtl::Reference< jvmaccess::VirtualMachine > const & getJavaVM2(const ::rtl::Reference< jvmaccess::VirtualMachine >& _rVM = ::rtl::Reference< jvmaccess::VirtualMachine >(),
                                                        bool _bSet = false)
{
    static ::rtl::Reference< jvmaccess::VirtualMachine > s_VM;
    if ( _rVM.is() || _bSet )
        s_VM = _rVM;
    return s_VM;
}

::rtl::Reference< jvmaccess::VirtualMachine > java_lang_Object::getVM(const Reference<XComponentContext >& _rxContext)
{
    ::rtl::Reference< jvmaccess::VirtualMachine > xVM = getJavaVM2();
    if ( !xVM.is() && _rxContext.is() )
        xVM = getJavaVM2(::connectivity::getJavaVM(_rxContext));

    return xVM;
}

SDBThreadAttach::SDBThreadAttach()
 : m_aGuard(java_lang_Object::getVM())
 , pEnv(nullptr)
{
    pEnv = m_aGuard.getEnvironment();
    OSL_ENSURE(pEnv,"Environment is nULL!");
}

SDBThreadAttach::~SDBThreadAttach()
{
}

oslInterlockedCount& getJavaVMRefCount()
{
    static oslInterlockedCount s_nRefCount = 0;
    return s_nRefCount;
}

void SDBThreadAttach::addRef()
{
    osl_atomic_increment(&getJavaVMRefCount());
}

void SDBThreadAttach::releaseRef()
{
    osl_atomic_decrement(&getJavaVMRefCount());
    if ( getJavaVMRefCount() == 0 )
    {
        getJavaVM2(::rtl::Reference< jvmaccess::VirtualMachine >(),true);
    }
}

// static variables of the class
jclass java_lang_Object::theClass = nullptr;

jclass java_lang_Object::getMyClass() const
{
    if( !theClass )
        theClass = findMyClass("java/lang/Object");
    return theClass;
}
// the actual constructor
java_lang_Object::java_lang_Object()
            : object( nullptr )
{
    SDBThreadAttach::addRef();
}

// the protected-constructor for the derived classes
java_lang_Object::java_lang_Object( JNIEnv * pXEnv, jobject myObj )
    : object( nullptr )
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
        clearObject(*t.pEnv);
    }
    SDBThreadAttach::releaseRef();
}
void java_lang_Object::clearObject(JNIEnv& rEnv)
{
    if( object )
    {
        rEnv.DeleteGlobalRef( object );
        object = nullptr;
    }
}

void java_lang_Object::clearObject()
{
    if( object )
    {
        SDBThreadAttach t;
        clearObject(*t.pEnv);
    }
}
// the protected-constructor for the derived classes
void java_lang_Object::saveRef( JNIEnv * pXEnv, jobject myObj )
{
    OSL_ENSURE( myObj, "object in c++ -> Java Wrapper" );
    if( myObj )
        object = pXEnv->NewGlobalRef( myObj );
}


OUString java_lang_Object::toString() const
{
    static jmethodID mID(nullptr);
    return callStringMethod("toString",mID);
}


namespace
{
    bool    lcl_translateJNIExceptionToUNOException(
        JNIEnv* _pEnvironment, const Reference< XInterface >& _rxContext, SQLException& _out_rException )
    {
        jthrowable jThrow = _pEnvironment ? _pEnvironment->ExceptionOccurred() : nullptr;
        if ( !jThrow )
            return false;

        _pEnvironment->ExceptionClear();
            // we have to clear the exception here because we want to handle it itself

        if ( _pEnvironment->IsInstanceOf( jThrow, java_sql_SQLException_BASE::st_getMyClass() ) )
        {
            std::unique_ptr< java_sql_SQLException_BASE > pException( new java_sql_SQLException_BASE( _pEnvironment, jThrow ) );
            _out_rException = SQLException( pException->getMessage(), _rxContext,
                pException->getSQLState(), pException->getErrorCode(), Any() );
            return true;
        }
        else if ( _pEnvironment->IsInstanceOf( jThrow, java_lang_Throwable::st_getMyClass() ) )
        {
            std::unique_ptr< java_lang_Throwable > pThrow( new java_lang_Throwable( _pEnvironment, jThrow ) );
#if OSL_DEBUG_LEVEL > 0
            pThrow->printStackTrace();
#endif
            OUString sMessage = pThrow->getMessage();
            if ( sMessage.isEmpty() )
                sMessage = pThrow->getLocalizedMessage();
            if(  sMessage.isEmpty() )
                sMessage = pThrow->toString();
            _out_rException = SQLException( sMessage, _rxContext, OUString(), -1, Any() );
            return true;
        }
        else
            _pEnvironment->DeleteLocalRef( jThrow );
        return false;
    }
}


void java_lang_Object::ThrowLoggedSQLException( const ::comphelper::ResourceBasedEventLogger& _rLogger, JNIEnv* _pEnvironment,
    const Reference< XInterface >& _rxContext )
{
    SQLException aException;
    if ( lcl_translateJNIExceptionToUNOException( _pEnvironment, _rxContext, aException ) )
    {
        _rLogger.log( css::logging::LogLevel::SEVERE, STR_LOG_THROWING_EXCEPTION, aException.Message, aException.SQLState, aException.ErrorCode );
        throw aException;
    }
}

void java_lang_Object::ThrowSQLException( JNIEnv* _pEnvironment, const Reference< XInterface>& _rxContext )
{
    SQLException aException;
    if ( lcl_translateJNIExceptionToUNOException( _pEnvironment, _rxContext, aException ) )
        throw aException;
}

void java_lang_Object::ThrowRuntimeException( JNIEnv* _pEnvironment, const Reference< XInterface>& _rxContext )
{
    try
    {
        ThrowSQLException(_pEnvironment, _rxContext);
    }
    catch (const SQLException& e)
    {
        throw WrappedTargetRuntimeException(e.Message, e.Context, makeAny(e));
    }
}

void java_lang_Object::obtainMethodId_throwSQL(JNIEnv* _pEnv,const char* _pMethodName, const char* _pSignature,jmethodID& _inout_MethodID) const
{
    if  ( !_inout_MethodID )
    {
        _inout_MethodID  = _pEnv->GetMethodID( getMyClass(), _pMethodName, _pSignature );
        OSL_ENSURE( _inout_MethodID, _pSignature );
        if  ( !_inout_MethodID )
            throw SQLException();
    } // if  ( !_inout_MethodID )
}

void java_lang_Object::obtainMethodId_throwRuntime(JNIEnv* _pEnv,const char* _pMethodName, const char* _pSignature,jmethodID& _inout_MethodID) const
{
    if  ( !_inout_MethodID )
    {
        _inout_MethodID  = _pEnv->GetMethodID( getMyClass(), _pMethodName, _pSignature );
        OSL_ENSURE( _inout_MethodID, _pSignature );
        if  ( !_inout_MethodID )
            throw RuntimeException();
    } // if  ( !_inout_MethodID )
}


bool java_lang_Object::callBooleanMethod( const char* _pMethodName, jmethodID& _inout_MethodID ) const
{
    jboolean out( false );

    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callBooleanMethod: no Java environment anymore!" );
    obtainMethodId_throwSQL(t.pEnv, _pMethodName,"()Z", _inout_MethodID);
    // call method
    out = t.pEnv->CallBooleanMethod( object, _inout_MethodID );
    ThrowSQLException( t.pEnv, nullptr );

    return out;
}

bool java_lang_Object::callBooleanMethodWithIntArg( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument ) const
{
    jboolean out( false );
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callBooleanMethodWithIntArg: no Java environment anymore!" );
    obtainMethodId_throwSQL(t.pEnv, _pMethodName,"(I)Z", _inout_MethodID);
    // call method
    out = t.pEnv->CallBooleanMethod( object, _inout_MethodID, _nArgument );
    ThrowSQLException( t.pEnv, nullptr );

    return out;
}

jobject java_lang_Object::callResultSetMethod( JNIEnv& _rEnv,const char* _pMethodName, jmethodID& _inout_MethodID ) const
{
    // call method
    jobject out = callObjectMethod(&_rEnv,_pMethodName,"()Ljava/sql/ResultSet;", _inout_MethodID);
    return out;
}

sal_Int32 java_lang_Object::callIntMethod_ThrowSQL(const char* _pMethodName, jmethodID& _inout_MethodID) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callIntMethod: no Java environment anymore!" );
    obtainMethodId_throwSQL(t.pEnv, _pMethodName,"()I", _inout_MethodID);
    // call method
    jint out( t.pEnv->CallIntMethod( object, _inout_MethodID ) );
    ThrowSQLException( t.pEnv, nullptr );
    return (sal_Int32)out;
}

sal_Int32 java_lang_Object::callIntMethod_ThrowRuntime(const char* _pMethodName, jmethodID& _inout_MethodID) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callIntMethod: no Java environment anymore!" );
    obtainMethodId_throwRuntime(t.pEnv, _pMethodName,"()I", _inout_MethodID);
    // call method
    jint out( t.pEnv->CallIntMethod( object, _inout_MethodID ) );
    ThrowRuntimeException(t.pEnv, nullptr);
    return (sal_Int32)out;
}

sal_Int32 java_lang_Object::callIntMethodWithIntArg_ThrowSQL( const char* _pMethodName, jmethodID& _inout_MethodID,sal_Int32 _nArgument ) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callIntMethod: no Java environment anymore!" );
    obtainMethodId_throwSQL(t.pEnv, _pMethodName,"(I)I", _inout_MethodID);
    // call method
    jint out( t.pEnv->CallIntMethod( object, _inout_MethodID , _nArgument) );
    ThrowSQLException( t.pEnv, nullptr );
    return (sal_Int32)out;
}

sal_Int32 java_lang_Object::callIntMethodWithIntArg_ThrowRuntime( const char* _pMethodName, jmethodID& _inout_MethodID,sal_Int32 _nArgument ) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callIntMethod: no Java environment anymore!" );
    obtainMethodId_throwRuntime(t.pEnv, _pMethodName,"(I)I", _inout_MethodID);
    // call method
    jint out( t.pEnv->CallIntMethod( object, _inout_MethodID , _nArgument) );
    ThrowRuntimeException(t.pEnv, nullptr);
    return (sal_Int32)out;
}

void java_lang_Object::callVoidMethod_ThrowSQL( const char* _pMethodName, jmethodID& _inout_MethodID) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callIntMethod: no Java environment anymore!" );
    obtainMethodId_throwSQL(t.pEnv, _pMethodName,"()V", _inout_MethodID);

    // call method
    t.pEnv->CallVoidMethod( object, _inout_MethodID );
    ThrowSQLException( t.pEnv, nullptr );
}

void java_lang_Object::callVoidMethod_ThrowRuntime( const char* _pMethodName, jmethodID& _inout_MethodID) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callIntMethod: no Java environment anymore!" );
    obtainMethodId_throwRuntime(t.pEnv, _pMethodName,"()V", _inout_MethodID);

    // call method
    t.pEnv->CallVoidMethod( object, _inout_MethodID );
    ThrowRuntimeException(t.pEnv, nullptr);
}

void java_lang_Object::callVoidMethodWithIntArg_ThrowSQL( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument ) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callIntMethod: no Java environment anymore!" );
    obtainMethodId_throwSQL(t.pEnv, _pMethodName,"(I)V", _inout_MethodID);

    // call method
    t.pEnv->CallVoidMethod( object, _inout_MethodID,_nArgument );
    ThrowSQLException( t.pEnv, nullptr );
}

void java_lang_Object::callVoidMethodWithIntArg_ThrowRuntime( const char* _pMethodName, jmethodID& _inout_MethodID, sal_Int32 _nArgument ) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callIntMethod: no Java environment anymore!" );
    obtainMethodId_throwRuntime(t.pEnv, _pMethodName,"(I)V", _inout_MethodID);

    // call method
    t.pEnv->CallVoidMethod( object, _inout_MethodID,_nArgument );
    ThrowRuntimeException(t.pEnv, nullptr);
}

void java_lang_Object::callVoidMethodWithBoolArg_ThrowSQL( const char* _pMethodName, jmethodID& _inout_MethodID, bool _nArgument ) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callIntMethod: no Java environment anymore!" );
    obtainMethodId_throwSQL(t.pEnv, _pMethodName,"(Z)V", _inout_MethodID);
    // call method
    t.pEnv->CallVoidMethod( object, _inout_MethodID,int(_nArgument) );
    ThrowSQLException( t.pEnv, nullptr );
}

void java_lang_Object::callVoidMethodWithBoolArg_ThrowRuntime( const char* _pMethodName, jmethodID& _inout_MethodID, bool _nArgument ) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callIntMethod: no Java environment anymore!" );
    obtainMethodId_throwRuntime(t.pEnv, _pMethodName,"(Z)V", _inout_MethodID);
    // call method
    t.pEnv->CallVoidMethod( object, _inout_MethodID,int(_nArgument) );
    ThrowRuntimeException(t.pEnv, nullptr);
}

OUString java_lang_Object::callStringMethod( const char* _pMethodName, jmethodID& _inout_MethodID ) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callStringMethod: no Java environment anymore!" );

    // call method
    jstring out = static_cast<jstring>(callObjectMethod(t.pEnv,_pMethodName,"()Ljava/lang/String;", _inout_MethodID));
    return JavaString2String( t.pEnv, out );
}

jobject java_lang_Object::callObjectMethod( JNIEnv * _pEnv,const char* _pMethodName,const char* _pSignature, jmethodID& _inout_MethodID ) const
{
    // obtain method ID
    obtainMethodId_throwSQL(_pEnv, _pMethodName,_pSignature, _inout_MethodID);
    // call method
    jobject out = _pEnv->CallObjectMethod( object, _inout_MethodID);
    ThrowSQLException( _pEnv, nullptr );
    return out;
}


jobject java_lang_Object::callObjectMethodWithIntArg( JNIEnv * _pEnv,const char* _pMethodName,const char* _pSignature, jmethodID& _inout_MethodID , sal_Int32 _nArgument) const
{
    obtainMethodId_throwSQL(_pEnv, _pMethodName,_pSignature, _inout_MethodID);
    // call method
    jobject out = _pEnv->CallObjectMethod( object, _inout_MethodID,_nArgument );
    ThrowSQLException( _pEnv, nullptr );
    return out;
}

OUString java_lang_Object::callStringMethodWithIntArg( const char* _pMethodName, jmethodID& _inout_MethodID , sal_Int32 _nArgument) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callStringMethod: no Java environment anymore!" );
    jstring out = static_cast<jstring>(callObjectMethodWithIntArg(t.pEnv,_pMethodName,"(I)Ljava/lang/String;",_inout_MethodID,_nArgument));
    return JavaString2String( t.pEnv, out );
}

void java_lang_Object::callVoidMethodWithStringArg( const char* _pMethodName, jmethodID& _inout_MethodID,const OUString& _nArgument ) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callIntMethod: no Java environment anymore!" );
    obtainMethodId_throwSQL(t.pEnv, _pMethodName,"(Ljava/lang/String;)V", _inout_MethodID);

    jdbc::LocalRef< jstring > str( t.env(),convertwchar_tToJavaString(t.pEnv,_nArgument));
    // call method
    t.pEnv->CallVoidMethod( object, _inout_MethodID , str.get());
    ThrowSQLException( t.pEnv, nullptr );
}

sal_Int32 java_lang_Object::callIntMethodWithStringArg( const char* _pMethodName, jmethodID& _inout_MethodID,const OUString& _nArgument ) const
{
    SDBThreadAttach t;
    OSL_ENSURE( t.pEnv, "java_lang_Object::callIntMethodWithStringArg: no Java environment anymore!" );
    obtainMethodId_throwSQL(t.pEnv, _pMethodName,"(Ljava/lang/String;)I", _inout_MethodID);

    //TODO: Check if the code below is needed
            //jdbc::LocalRef< jstring > str( t.env(), convertwchar_tToJavaString( t.pEnv, sql ) );
            //{
            //    jdbc::ContextClassLoaderScope ccl( t.env(),
            //        m_pConnection ? m_pConnection->getDriverClassLoader() : jdbc::GlobalRef< jobject >(),
            //        m_aLogger,
            //        *this
            //    );

    jdbc::LocalRef< jstring > str( t.env(),convertwchar_tToJavaString(t.pEnv,_nArgument));
    // call method
    jint out = t.pEnv->CallIntMethod( object, _inout_MethodID , str.get());
    ThrowSQLException( t.pEnv, nullptr );
    return (sal_Int32)out;
}

jclass java_lang_Object::findMyClass(const char* _pClassName)
{
    // the class must be fetched only once, therefore static
    SDBThreadAttach t;
    jclass tempClass = t.pEnv->FindClass(_pClassName); OSL_ENSURE(tempClass,"Java : FindClass nicht erfolgreich!");
    if(!tempClass)
    {
        t.pEnv->ExceptionDescribe();
        t.pEnv->ExceptionClear();
    }
    jclass globClass = static_cast<jclass>(t.pEnv->NewGlobalRef( tempClass ));
    t.pEnv->DeleteLocalRef( tempClass );
    return globClass;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
