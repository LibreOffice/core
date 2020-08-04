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

#pragma once

#include <sal/config.h>

#include <cassert>

#include <jvmaccess/unovirtualmachine.hxx>
#include <jvmaccess/virtualmachine.hxx>

#include <osl/diagnose.h>

#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#include <uno/environment.h>
#include <typelib/typedescription.h>


namespace jni_uno
{

class JNI_info;

struct BridgeRuntimeError
{
    OUString m_message;

    explicit BridgeRuntimeError( OUString const & message )
        : m_message( message )
        {}
};


class JNI_context
{
    JNI_info const * m_jni_info;
    JNIEnv *         m_env;
    jobject          m_class_loader;

    JNI_context( JNI_context const & ) = delete;
    JNI_context& operator = ( JNI_context const &) = delete;

    void java_exc_occurred() const;
public:
    explicit JNI_context(
        JNI_info const * jni_info, JNIEnv * env, jobject class_loader )
        : m_jni_info( jni_info ),
          m_env( env ),
          m_class_loader( class_loader )
        {}

    JNI_info const * get_info() const
        { return m_jni_info; }

    JNIEnv * operator -> () const
        { return m_env; }
    JNIEnv * get_jni_env() const
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

    OUString get_stack_trace( jobject jo_exc = nullptr ) const;
};

inline void JNI_context::ensure_no_exception() const
{
    if (m_env->ExceptionCheck())
    {
        java_exc_occurred();
    }
}

inline bool JNI_context::assert_no_exception() const
{
    if (m_env->ExceptionCheck())
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


class JNI_guarded_context
    : private ::jvmaccess::VirtualMachine::AttachGuard,
      public JNI_context
{
    JNI_guarded_context( JNI_guarded_context const & ) = delete;
    JNI_guarded_context& operator = ( JNI_guarded_context const &) = delete;

public:
    explicit JNI_guarded_context(
        JNI_info const * jni_info,
        rtl::Reference<jvmaccess::UnoVirtualMachine> const & vm_access)
        : AttachGuard( vm_access->getVirtualMachine() ),
          JNI_context(
              jni_info, AttachGuard::getEnvironment(),
              static_cast< jobject >(vm_access->getClassLoader()) )
        {}
};


class JLocalAutoRef
{
    JNI_context const & m_jni;
    jobject m_jo;

public:
    explicit JLocalAutoRef( JNI_context const & jni )
        : m_jni( jni ),
          m_jo( nullptr )
        {}
    explicit JLocalAutoRef( JNI_context const & jni, jobject jo )
        : m_jni( jni ),
          m_jo( jo )
        {}
    inline JLocalAutoRef( JLocalAutoRef & auto_ref );
    inline ~JLocalAutoRef();

    jobject get() const
        { return m_jo; }
    bool is() const
        { return (nullptr != m_jo); }
    inline jobject release();
    inline void reset( jobject jo );
    inline JLocalAutoRef & operator = ( JLocalAutoRef & auto_ref );
};

inline JLocalAutoRef::~JLocalAutoRef()
{
    if (nullptr != m_jo)
        m_jni->DeleteLocalRef( m_jo );
}

inline JLocalAutoRef::JLocalAutoRef( JLocalAutoRef & auto_ref )
    : m_jni( auto_ref.m_jni ),
      m_jo( auto_ref.m_jo )
{
    auto_ref.m_jo = nullptr;
}

inline jobject JLocalAutoRef::release()
{
    jobject jo = m_jo;
    m_jo = nullptr;
    return jo;
}

inline void JLocalAutoRef::reset( jobject jo )
{
    if (jo != m_jo)
    {
        if (nullptr != m_jo)
            m_jni->DeleteLocalRef( m_jo );
        m_jo = jo;
    }
}

inline JLocalAutoRef & JLocalAutoRef::operator = ( JLocalAutoRef & auto_ref )
{
    assert( m_jni.get_jni_env() == auto_ref.m_jni.get_jni_env() );
    reset( auto_ref.m_jo );
    auto_ref.m_jo = nullptr;
    return *this;
}



struct rtl_mem
{
    static void * operator new ( size_t nSize )
        { return std::malloc( nSize ); }
    static void operator delete ( void * mem )
        { std::free( mem ); }
    static void * operator new ( size_t, void * mem )
        { return mem; }
    static void operator delete ( void *, void * )
        {}

    static inline rtl_mem * allocate( std::size_t bytes );
};

inline rtl_mem * rtl_mem::allocate( std::size_t bytes )
{
    void * p = std::malloc( bytes );
    if (nullptr == p)
        throw BridgeRuntimeError( "out of memory!" );
    return static_cast<rtl_mem *>(p);
}


class TypeDescr
{
    typelib_TypeDescription * m_td;

    TypeDescr( TypeDescr const & ) = delete;
    TypeDescr& operator = ( TypeDescr const & ) = delete;

public:
    inline explicit TypeDescr( typelib_TypeDescriptionReference * td_ref );
    ~TypeDescr()
        { TYPELIB_DANGER_RELEASE( m_td ); }

    typelib_TypeDescription * get() const
        { return m_td; }
};

inline TypeDescr::TypeDescr( typelib_TypeDescriptionReference * td_ref )
    : m_td( nullptr )
{
    TYPELIB_DANGER_GET( &m_td, td_ref );
    if (nullptr == m_td)
    {
        throw BridgeRuntimeError(
            "cannot get comprehensive type description for " +
            OUString::unacquired( &td_ref->pTypeName ) );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
