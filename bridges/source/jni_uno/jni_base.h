/*************************************************************************
 *
 *  $RCSfile: jni_base.h,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 10:49:45 $
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

#if ! defined INCLUDED_JNI_BASE_H
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

#include "jvmaccess/virtualmachine.hxx"

#include "osl/diagnose.h"

#include "rtl/alloc.h"
#include "rtl/ustring.hxx"

#include "uno/environment.h"
#include "typelib/typedescription.h"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


namespace jni_uno
{

class JNI_info;

//==============================================================================
struct BridgeRuntimeError
{
    ::rtl::OUString m_message;

    inline BridgeRuntimeError( ::rtl::OUString const & message )
        : m_message( message )
        {}
};


//==============================================================================
class JNI_context
{
    JNI_info const * m_jni_info;
    JNIEnv *         m_env;

    JNI_context( JNI_context & ); // not impl
    void operator = ( JNI_context ); // not impl

    void java_exc_occured() const;
public:
    inline explicit JNI_context( JNI_info const * jni_info, JNIEnv * env )
        : m_jni_info( jni_info ),
          m_env( env )
        {}

    inline JNI_info const * get_info() const
        { return m_jni_info; }

    inline JNIEnv * operator -> () const
        { return m_env; }
    inline JNIEnv * get_jni_env() const
        { return m_env; }

    inline void ensure_no_exception() const; // throws BridgeRuntimeError
    inline bool assert_no_exception() const; // asserts and clears exception

    ::rtl::OUString get_stack_trace( jobject jo_exc = 0 ) const;
};

//______________________________________________________________________________
inline void JNI_context::ensure_no_exception() const
{
    if (JNI_FALSE != m_env->ExceptionCheck())
    {
        java_exc_occured();
    }
}

//______________________________________________________________________________
inline bool JNI_context::assert_no_exception() const
{
    if (JNI_FALSE != m_env->ExceptionCheck())
    {
        m_env->ExceptionClear();
        OSL_ENSURE( 0, "unexpected java exception occured!" );
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
        JNI_info const * jni_info, ::jvmaccess::VirtualMachine * vm_access )
        : AttachGuard( vm_access ),
          JNI_context( jni_info, AttachGuard::getEnvironment() )
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
    inline ~JLocalAutoRef() SAL_THROW( () );

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
inline JLocalAutoRef::~JLocalAutoRef() SAL_THROW( () )
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
        throw BridgeRuntimeError( OUSTR("out of memory!") );
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
    inline ~TypeDescr() SAL_THROW( () )
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
            OUSTR("cannot get comprehensive type description for ") +
            ::rtl::OUString::unacquired( &td_ref->pTypeName ) );
    }
}

}

#endif
