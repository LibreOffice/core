/*************************************************************************
 *
 *  $RCSfile: jni_base.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2002-10-28 18:20:03 $
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
#ifndef _JNI_BASE_H_
#define _JNI_BASE_H_

#include <memory>

#include <osl/diagnose.h>
#include <rtl/alloc.h>

#include <typelib/typedescription.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>

#include "jni_info.h"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


struct JavaVMContext;

namespace jni_bridge
{

//==================================================================================================
struct BridgeRuntimeError
{
    ::rtl::OUString m_message;

    inline BridgeRuntimeError( ::rtl::OUString const & message )
        : m_message( message )
        {}
};

//##################################################################################################

//==================================================================================================
struct rtl_mem
{
    inline static void * operator new ( size_t nSize ) SAL_THROW( () )
        { return rtl_allocateMemory( nSize ); }
    inline static void operator delete ( void * mem ) SAL_THROW( () )
        { if (mem) rtl_freeMemory( mem ); }
    inline static void * operator new ( size_t, void * mem ) SAL_THROW( () )
        { return mem; }
    inline static void operator delete ( void *, void * ) SAL_THROW( () )
        {}

    static inline ::std::auto_ptr< rtl_mem > allocate( ::std::size_t bytes );
};
//--------------------------------------------------------------------------------------------------
inline ::std::auto_ptr< rtl_mem > rtl_mem::allocate( ::std::size_t bytes )
{
    void * p = rtl_allocateMemory( bytes );
    if (0 == p)
        throw BridgeRuntimeError( OUSTR("out of memory!") );
    return ::std::auto_ptr< rtl_mem >( (rtl_mem *)p );
}

//##################################################################################################

//==================================================================================================
class TypeDescr
{
    typelib_TypeDescription * m_td;

    inline TypeDescr( TypeDescr const & );
    inline TypeDescr & operator = ( TypeDescr const & );

public:
    inline TypeDescr( typelib_TypeDescriptionReference * td_ref );
    inline ~TypeDescr() SAL_THROW( () )
        { TYPELIB_DANGER_RELEASE( m_td ); }

    inline typelib_TypeDescription * get() const SAL_THROW( () )
        { return m_td; }
};
//__________________________________________________________________________________________________
inline TypeDescr::TypeDescr( typelib_TypeDescriptionReference * td_ref )
    : m_td( 0 )
{
    TYPELIB_DANGER_GET( &m_td, td_ref );
    if (0 == m_td)
    {
        throw BridgeRuntimeError(
            OUSTR("cannot get comprehensive type description for ") +
            *reinterpret_cast< ::rtl::OUString const * >( &td_ref->pTypeName ) );
    }
}

//##################################################################################################

//==================================================================================================
class JNI_attach
{
    sal_uInt32                  m_thread_id;
    ::JavaVMContext *           m_context;
    JNI_info const *            m_jni_info;
    JavaVM *                    m_vm;
    JNIEnv *                    m_env;
    bool                        m_detach;

public:
    JNI_attach( uno_Environment * java_env, JNIEnv * jni_env ) SAL_THROW( () );
    JNI_attach( uno_Environment * java_env );
    ~JNI_attach() SAL_THROW( () );

    inline JavaVM * get_vm() const SAL_THROW( () )
        { return m_vm; }
    inline JNIEnv * get_jni_env() const SAL_THROW( () )
        { return m_env; }
    inline JNI_info const * get_jni_info() const SAL_THROW( () )
        { return m_jni_info; }

    inline JNIEnv * operator -> () const SAL_THROW( () )
        { return m_env; }

    void throw_bridge_error() const;
    inline void ensure_no_exception() const
        { if (JNI_FALSE != m_env->ExceptionCheck()) throw_bridge_error(); }
};

//##################################################################################################

//--------------------------------------------------------------------------------------------------
inline void jstring_to_ustring( JNI_attach const & attach, rtl_uString ** out_ustr, jstring jstr )
{
    if (0 == jstr)
    {
        rtl_uString_new( out_ustr );
    }
    else
    {
        OSL_ASSERT( sizeof (sal_Unicode) == sizeof (jchar) );
        jsize len = attach->GetStringLength( jstr );
        ::std::auto_ptr< rtl_mem > mem(
            rtl_mem::allocate( sizeof (rtl_uString) + (len * sizeof (sal_Unicode)) ) );
        rtl_uString * ustr = (rtl_uString *)mem.get();
        attach->GetStringRegion( jstr, 0, len, ustr->buffer );
        attach.ensure_no_exception();
        ustr->refCount = 1;
        ustr->length = len;
        ustr->buffer[ len ] = '\0';
        mem.release();
        if (0 != *out_ustr)
            rtl_uString_release( *out_ustr );
        *out_ustr = ustr;
    }
}
//--------------------------------------------------------------------------------------------------
inline ::rtl::OUString jstring_to_oustring( JNI_attach const & attach, jstring jstr )
{
    rtl_uString * ustr = 0;
    jstring_to_ustring( attach, &ustr, jstr );
    return ::rtl::OUString( ustr, SAL_NO_ACQUIRE );
}
//--------------------------------------------------------------------------------------------------
inline jstring ustring_to_jstring( JNI_attach const & attach, rtl_uString * ustr )
{
    OSL_ASSERT( sizeof (sal_Unicode) == sizeof (jchar) );
    jstring jstr = attach->NewString( ustr->buffer, ustr->length );
    attach.ensure_no_exception();
    return jstr;
}

//##################################################################################################

//==================================================================================================
class JLocalAutoRef
{
    JNI_attach const * m_attach;
    mutable jobject m_jo;

public:
    inline JLocalAutoRef() SAL_THROW( () )
        : m_attach( 0 ),
          m_jo( 0 )
        {}
    inline explicit JLocalAutoRef( JNI_attach const & attach, jobject jo ) SAL_THROW( () )
        : m_attach( &attach ),
          m_jo( jo )
        {}
    inline JLocalAutoRef( JLocalAutoRef const & auto_ref ) SAL_THROW( () );
    inline ~JLocalAutoRef() SAL_THROW( () );

    inline jobject get() const SAL_THROW( () )
        { return m_jo; }
    inline bool is() const SAL_THROW( () )
        { return (0 != m_jo); }
    inline jobject release() SAL_THROW( () );
    inline void reset() SAL_THROW( () );
    inline void reset( JNI_attach const & attach, jobject jo ) SAL_THROW( () );
    inline JLocalAutoRef & operator = ( JLocalAutoRef const & auto_ref ) SAL_THROW( () );

    //
    inline JLocalAutoRef get_class() const;
    inline ::rtl::OUString get_class_name() const;

    //
//     inline JNI_attach const & get_jni_attach() const SAL_THROW( () )
//         { return *m_attach; }
};
//__________________________________________________________________________________________________
inline JLocalAutoRef::~JLocalAutoRef() SAL_THROW( () )
{
    if (0 != m_jo)
        (*m_attach)->DeleteLocalRef( m_jo );
}
//__________________________________________________________________________________________________
inline JLocalAutoRef::JLocalAutoRef( JLocalAutoRef const & auto_ref ) SAL_THROW( () )
    : m_attach( auto_ref.m_attach ),
      m_jo( auto_ref.m_jo )
{
    auto_ref.m_jo = 0;
}
//__________________________________________________________________________________________________
inline jobject JLocalAutoRef::release() SAL_THROW( () )
{
    jobject jo = m_jo;
    m_jo = 0;
    return jo;
}
//__________________________________________________________________________________________________
inline void JLocalAutoRef::reset() SAL_THROW( () )
{
    if (m_jo)
        (*m_attach)->DeleteLocalRef( m_jo );
    m_jo = 0;
}
//__________________________________________________________________________________________________
inline void JLocalAutoRef::reset( JNI_attach const & attach, jobject jo ) SAL_THROW( () )
{
    if (jo != m_jo)
    {
        if (m_jo)
            (*m_attach)->DeleteLocalRef( m_jo );
        m_jo = jo;
    }
    m_attach = &attach;
}
//__________________________________________________________________________________________________
inline JLocalAutoRef & JLocalAutoRef::operator = ( JLocalAutoRef const & auto_ref ) SAL_THROW( () )
{
    reset( *auto_ref.m_attach, auto_ref.m_jo );
    auto_ref.m_jo = 0;
    return *this;
}
//__________________________________________________________________________________________________
inline JLocalAutoRef JLocalAutoRef::get_class() const
{
    jobject jo_class = 0;
    if (0 != m_jo)
    {
        jo_class = (*m_attach)->CallObjectMethodA(
            m_jo, m_attach->get_jni_info()->m_method_Object_getClass, 0 );
        m_attach->ensure_no_exception();
    }
    return JLocalAutoRef( *m_attach, jo_class );
}
//__________________________________________________________________________________________________
inline ::rtl::OUString JLocalAutoRef::get_class_name() const
{
    JLocalAutoRef jo_class( get_class() );
    JLocalAutoRef jo_name(
        *m_attach, (*m_attach)->CallObjectMethodA(
            jo_class.get(), m_attach->get_jni_info()->m_method_Class_getName, 0 ) );
    m_attach->ensure_no_exception();
    return jstring_to_oustring( *m_attach, (jstring)jo_name.get() );
}

//##################################################################################################

//--------------------------------------------------------------------------------------------------
inline JLocalAutoRef find_class( JNI_attach const & attach, char const * class_name )
{
    jclass jo_class = attach->FindClass( class_name );
    attach.ensure_no_exception();
    return JLocalAutoRef( attach, jo_class );
}
//--------------------------------------------------------------------------------------------------
inline JLocalAutoRef find_class( JNI_attach const & attach, ::rtl::OUString const & class_name )
{
    ::rtl::OString cstr_name( ::rtl::OUStringToOString( class_name, RTL_TEXTENCODING_ASCII_US ) );
    return find_class( attach, cstr_name );
}

//--------------------------------------------------------------------------------------------------
inline JLocalAutoRef create_type( JNI_attach const & attach, jclass clazz )
{
    JNI_info const * jni_info = attach.get_jni_info();

    jvalue arg;
    arg.l = clazz;
    JLocalAutoRef jo_type(
        attach, attach->NewObjectA(
            jni_info->m_class_Type, jni_info->m_ctor_Type_with_Class, &arg ) );
    attach.ensure_no_exception();

    return jo_type;
}
//--------------------------------------------------------------------------------------------------
inline JLocalAutoRef create_type(
    JNI_attach const & attach, typelib_TypeDescriptionReference * type )
{
    JNI_info const * jni_info = attach.get_jni_info();

    jvalue args[ 2 ];
    // get type class
    args[ 0 ].i = type->eTypeClass;
    JLocalAutoRef jo_type_class(
        attach, attach->CallStaticObjectMethodA(
            jni_info->m_class_TypeClass, jni_info->m_method_TypeClass_fromInt, args ) );
    attach.ensure_no_exception();
    // construct type
    JLocalAutoRef jo_type_name( attach, ustring_to_jstring( attach, type->pTypeName ) );
    args[ 0 ].l = jo_type_name.get();
    args[ 1 ].l = jo_type_class.get();
    JLocalAutoRef jo_type(
        attach, attach->NewObjectA(
            jni_info->m_class_Type, jni_info->m_ctor_Type_with_Name_TypeClass, args ) );
    attach.ensure_no_exception();

    return jo_type;
}

//--------------------------------------------------------------------------------------------------
inline JLocalAutoRef compute_oid( JNI_attach const & attach, jobject jo )
{
    JNI_info const * jni_info = attach.get_jni_info();
    jvalue arg;
    arg.l= jo;
    JLocalAutoRef jo_oid(
        attach, attach->CallStaticObjectMethodA(
            jni_info->m_class_UnoRuntime, jni_info->m_method_UnoRuntime_generateOid, &arg ) );
    attach.ensure_no_exception();
    return jo_oid;
}

}

#endif
