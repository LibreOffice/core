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

#ifndef INCLUDED_JNI_BASE_H
#define INCLUDED_JNI_BASE_H

#if defined (__SUNPRO_CC) || defined (__SUNPRO_C)
// workaround solaris include trouble on jumbo
#include <stdarg.h>
namespace std
{
typedef __va_list va_list;
}
#endif
#include <memory>

#include "jvmaccess/unovirtualmachine.hxx"
#include "jvmaccess/virtualmachine.hxx"

#include "osl/diagnose.h"

#include "rtl/alloc.h"
#include "rtl/ustring.hxx"

#include "uno/environment.h"
#include "typelib/typedescription.h"


namespace jni_uno
{

class JNI_info;

//==============================================================================
struct BridgeRuntimeError
{
    OUString m_message;

    inline BridgeRuntimeError( OUString const & message )
        : m_message( message )
        {}
};


//==============================================================================
class JNI_context
{
    JNI_info const * m_jni_info;
    JNIEnv *         m_env;
    jobject          m_class_loader;

    JNI_context( JNI_context & ); // not impl
    void operator = ( JNI_context ); // not impl

    void java_exc_occurred() const;
public:
    inline explicit JNI_context(
        JNI_info const * jni_info, JNIEnv * env, jobject class_loader )
        : m_jni_info( jni_info ),
          m_env( env ),
          m_class_loader( class_loader )
        {}

    inline JNI_info const * get_info() const
        { return m_jni_info; }

    inline JNIEnv * operator -> () const
        { return m_env; }
    inline JNIEnv * get_jni_env() const
        { return m_env; }

    // does not handle exceptions, *classClass will be null if exception
    // occurred:
    void getClassForName(jclass * classClass, jmethodID * methodForName) const;

    // if inException, does not handle exceptions, in which case returned value
    // will be null if exception occurred:
    jclass findClass(
        char const * name, jclass classClass, jmethodID methodForName,
        bool inException) const;

    inline void ensure_no_exception() const; // throws BridgeRuntimeError
    inline bool assert_no_exception() const; // asserts and clears exception

    OUString get_stack_trace( jobject jo_exc = 0 ) const;
};

//______________________________________________________________________________
inline void JNI_context::ensure_no_exception() const
{
    if (JNI_FALSE != m_env->ExceptionCheck())
    {
        java_exc_occurred();
    }
}

//______________________________________________________________________________
inline bool JNI_context::assert_no_exception() const
{
    if (JNI_FALSE != m_env->ExceptionCheck())
    {
        SAL_WARN("bridges", "unexpected java exception occurred");
#if OSL_DEBUG_LEVEL > 0
        m_env->ExceptionDescribe();
#endif
        m_env->ExceptionClear();
        return false;
    }
    return true;
}


//==============================================================================
class JNI_guarded_context
    : private ::jvmaccess::VirtualMachine::AttachGuard,
      public JNI_context
{
    JNI_guarded_context( JNI_guarded_context & ); // not impl
    void operator = ( JNI_guarded_context ); // not impl

public:
    inline explicit JNI_guarded_context(
        JNI_info const * jni_info, ::jvmaccess::UnoVirtualMachine * vm_access )
        : AttachGuard( vm_access->getVirtualMachine() ),
          JNI_context(
              jni_info, AttachGuard::getEnvironment(),
              static_cast< jobject >(vm_access->getClassLoader()) )
        {}
};


//==============================================================================
class JLocalAutoRef
{
    JNI_context const & m_jni;
    jobject m_jo;

public:
    inline JLocalAutoRef( JNI_context const & jni )
        : m_jni( jni ),
          m_jo( 0 )
        {}
    inline explicit JLocalAutoRef( JNI_context const & jni, jobject jo )
        : m_jni( jni ),
          m_jo( jo )
        {}
    inline JLocalAutoRef( JLocalAutoRef & auto_ref );
    inline ~JLocalAutoRef() SAL_THROW(());

    inline jobject get() const
        { return m_jo; }
    inline bool is() const
        { return (0 != m_jo); }
    inline jobject release();
    inline void reset();
    inline void reset( jobject jo );
    inline JLocalAutoRef & operator = ( JLocalAutoRef & auto_ref );
};

//______________________________________________________________________________
inline JLocalAutoRef::~JLocalAutoRef() SAL_THROW(())
{
    if (0 != m_jo)
        m_jni->DeleteLocalRef( m_jo );
}

//______________________________________________________________________________
inline JLocalAutoRef::JLocalAutoRef( JLocalAutoRef & auto_ref )
    : m_jni( auto_ref.m_jni ),
      m_jo( auto_ref.m_jo )
{
    auto_ref.m_jo = 0;
}

//______________________________________________________________________________
inline jobject JLocalAutoRef::release()
{
    jobject jo = m_jo;
    m_jo = 0;
    return jo;
}

//______________________________________________________________________________
inline void JLocalAutoRef::reset()
{
    if (0 != m_jo)
        m_jni->DeleteLocalRef( m_jo );
    m_jo = 0;
}

//______________________________________________________________________________
inline void JLocalAutoRef::reset( jobject jo )
{
    if (jo != m_jo)
    {
        if (0 != m_jo)
            m_jni->DeleteLocalRef( m_jo );
        m_jo = jo;
    }
}

//______________________________________________________________________________
inline JLocalAutoRef & JLocalAutoRef::operator = ( JLocalAutoRef & auto_ref )
{
    OSL_ASSERT( m_jni.get_jni_env() == auto_ref.m_jni.get_jni_env() );
    reset( auto_ref.m_jo );
    auto_ref.m_jo = 0;
    return *this;
}


//==============================================================================
struct rtl_mem
{
    inline static void * operator new ( size_t nSize )
        { return rtl_allocateMemory( nSize ); }
    inline static void operator delete ( void * mem )
        { if (mem) rtl_freeMemory( mem ); }
    inline static void * operator new ( size_t, void * mem )
        { return mem; }
    inline static void operator delete ( void *, void * )
        {}

    static inline ::std::auto_ptr< rtl_mem > allocate( ::std::size_t bytes );
};

//______________________________________________________________________________
inline ::std::auto_ptr< rtl_mem > rtl_mem::allocate( ::std::size_t bytes )
{
    void * p = rtl_allocateMemory( bytes );
    if (0 == p)
        throw BridgeRuntimeError( "out of memory!" );
    return ::std::auto_ptr< rtl_mem >( (rtl_mem *)p );
}


//==============================================================================
class TypeDescr
{
    typelib_TypeDescription * m_td;

    TypeDescr( TypeDescr & ); // not impl
    void operator = ( TypeDescr ); // not impl

public:
    inline explicit TypeDescr( typelib_TypeDescriptionReference * td_ref );
    inline ~TypeDescr() SAL_THROW(())
        { TYPELIB_DANGER_RELEASE( m_td ); }

    inline typelib_TypeDescription * get() const
        { return m_td; }
};

//______________________________________________________________________________
inline TypeDescr::TypeDescr( typelib_TypeDescriptionReference * td_ref )
    : m_td( 0 )
{
    TYPELIB_DANGER_GET( &m_td, td_ref );
    if (0 == m_td)
    {
        throw BridgeRuntimeError(
            "cannot get comprehensive type description for " +
            OUString::unacquired( &td_ref->pTypeName ) );
    }
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
