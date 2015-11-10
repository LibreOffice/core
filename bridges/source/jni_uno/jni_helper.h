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

#ifndef INCLUDED_BRIDGES_SOURCE_JNI_UNO_JNI_HELPER_H
#define INCLUDED_BRIDGES_SOURCE_JNI_UNO_JNI_HELPER_H

#include <sal/config.h>

#include <memory>

#include "jni_base.h"
#include "jni_info.h"


namespace jni_uno
{

inline void jstring_to_ustring(
    JNI_context const & jni, rtl_uString ** out_ustr, jstring jstr )
{
    if (NULL == jstr)
    {
        rtl_uString_new( out_ustr );
    }
    else
    {
        jsize len = jni->GetStringLength( jstr );
        std::unique_ptr< rtl_mem > mem(
            rtl_mem::allocate(
                sizeof (rtl_uString) + (len * sizeof (sal_Unicode)) ) );
        rtl_uString * ustr = reinterpret_cast<rtl_uString *>(mem.get());
        jni->GetStringRegion( jstr, 0, len, reinterpret_cast<jchar *>(ustr->buffer) );
        jni.ensure_no_exception();
        ustr->refCount = 1;
        ustr->length = len;
        ustr->buffer[ len ] = '\0';
        mem.release();
        if (NULL != *out_ustr)
            rtl_uString_release( *out_ustr );
        *out_ustr = ustr;
    }
}

inline OUString jstring_to_oustring(
    JNI_context const & jni, jstring jstr )
{
    rtl_uString * ustr = NULL;
    jstring_to_ustring( jni, &ustr, jstr );
    return OUString( ustr, SAL_NO_ACQUIRE );
}

inline jstring ustring_to_jstring(
    JNI_context const & jni, rtl_uString const * ustr )
{
    jstring jstr = jni->NewString( reinterpret_cast<jchar const *>(ustr->buffer), ustr->length );
    jni.ensure_no_exception();
    return jstr;
}


// if inException, does not handle exceptions, in which case returned value will
// be null if exception occurred:
inline jclass find_class(
    JNI_context const & jni, char const * class_name, bool inException = false )
{
    // find_class may be called before the JNI_info is set:
    jclass c=NULL;
    jmethodID m;
    JNI_info const * info = jni.get_info();
    if (info == NULL) {
        jni.getClassForName(&c, &m);
        if (c == NULL) {
            if (inException) {
                return NULL;
            }
            jni.ensure_no_exception();
        }
    } else {
        c = info->m_class_Class;
        m = info->m_method_Class_forName;
    }
    return jni.findClass(class_name, c, m, inException);
}


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
