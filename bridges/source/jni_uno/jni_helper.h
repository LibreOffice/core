/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: jni_helper.h,v $
 * $Revision: 1.10 $
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

#if ! defined INCLUDED_JNI_HELPER_H
#define INCLUDED_JNI_HELPER_H

#include "jni_base.h"
#include "jni_info.h"


namespace jni_uno
{

//------------------------------------------------------------------------------
inline void jstring_to_ustring(
    JNI_context const & jni, rtl_uString ** out_ustr, jstring jstr )
{
    if (0 == jstr)
    {
        rtl_uString_new( out_ustr );
    }
    else
    {
        jsize len = jni->GetStringLength( jstr );
        ::std::auto_ptr< rtl_mem > mem(
            rtl_mem::allocate(
                sizeof (rtl_uString) + (len * sizeof (sal_Unicode)) ) );
        rtl_uString * ustr = (rtl_uString *)mem.get();
        jni->GetStringRegion( jstr, 0, len, (jchar *) ustr->buffer );
        jni.ensure_no_exception();
        ustr->refCount = 1;
        ustr->length = len;
        ustr->buffer[ len ] = '\0';
        mem.release();
        if (0 != *out_ustr)
            rtl_uString_release( *out_ustr );
        *out_ustr = ustr;
    }
}

//------------------------------------------------------------------------------
inline ::rtl::OUString jstring_to_oustring(
    JNI_context const & jni, jstring jstr )
{
    rtl_uString * ustr = 0;
    jstring_to_ustring( jni, &ustr, jstr );
    return ::rtl::OUString( ustr, SAL_NO_ACQUIRE );
}

//------------------------------------------------------------------------------
inline jstring ustring_to_jstring(
    JNI_context const & jni, rtl_uString const * ustr )
{
    jstring jstr = jni->NewString( (jchar const *) ustr->buffer, ustr->length );
    jni.ensure_no_exception();
    return jstr;
}


//------------------------------------------------------------------------------
// if inException, does not handle exceptions, in which case returned value will
// be null if exception occurred:
inline jclass find_class(
    JNI_context const & jni, char const * class_name, bool inException = false )
{
    // find_class may be called before the JNI_info is set:
    jclass c=0;
    jmethodID m;
    JNI_info const * info = jni.get_info();
    if (info == 0) {
        jni.getClassForName(&c, &m);
        if (c == 0) {
            if (inException) {
                return 0;
            }
            jni.ensure_no_exception();
        }
    } else {
        c = info->m_class_Class;
        m = info->m_method_Class_forName;
    }
    return jni.findClass(class_name, c, m, inException);
}


//------------------------------------------------------------------------------
inline jobject create_type( JNI_context const & jni, jclass clazz )
{
    JNI_info const * jni_info = jni.get_info();
    jvalue arg;
    arg.l = clazz;
    jobject jo_type = jni->NewObjectA(
        jni_info->m_class_Type, jni_info->m_ctor_Type_with_Class, &arg );
    jni.ensure_no_exception();
    return jo_type;
}

//------------------------------------------------------------------------------
inline jobject create_type(
    JNI_context const & jni, typelib_TypeDescriptionReference * type )
{
    JNI_info const * jni_info = jni.get_info();
    jvalue args[ 2 ];
    // get type class
    args[ 0 ].i = type->eTypeClass;
    JLocalAutoRef jo_type_class(
        jni, jni->CallStaticObjectMethodA(
            jni_info->m_class_TypeClass,
            jni_info->m_method_TypeClass_fromInt, args ) );
    jni.ensure_no_exception();
    // construct type
    JLocalAutoRef jo_type_name(
        jni, ustring_to_jstring( jni, type->pTypeName ) );
    args[ 0 ].l = jo_type_name.get();
    args[ 1 ].l = jo_type_class.get();
    jobject jo_type = jni->NewObjectA(
        jni_info->m_class_Type,
        jni_info->m_ctor_Type_with_Name_TypeClass, args );
    jni.ensure_no_exception();
    return jo_type;
}

//------------------------------------------------------------------------------
inline jobject compute_oid( JNI_context const & jni, jobject jo )
{
    JNI_info const * jni_info = jni.get_info();
    jvalue arg;
    arg.l= jo;
    jobject jo_oid = jni->CallStaticObjectMethodA(
        jni_info->m_class_UnoRuntime,
        jni_info->m_method_UnoRuntime_generateOid, &arg );
    jni.ensure_no_exception();
    return jo_oid;
}

}

#endif
