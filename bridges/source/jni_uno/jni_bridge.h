/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: jni_bridge.h,v $
 * $Revision: 1.12 $
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

#if ! defined INCLUDED_JNI_BRIDGE_H
#define INCLUDED_JNI_BRIDGE_H

#include "jni_base.h"
#include "jni_info.h"
#include "jni_helper.h"

#include "osl/diagnose.h"
#include "osl/interlck.h"

#include "uno/mapping.h"
#include "uno/dispatcher.h"

#include "com/sun/star/uno/XInterface.hpp"


namespace jni_uno
{

//==== holds environments and mappings =========================================
struct Bridge;
struct Mapping : public uno_Mapping
{
    Bridge * m_bridge;
};

//==============================================================================
struct Bridge
{
    mutable oslInterlockedCount m_ref;

    uno_ExtEnvironment *        m_uno_env;
    uno_Environment *           m_java_env;

    Mapping                     m_java2uno;
    Mapping                     m_uno2java;
    bool                        m_registered_java2uno;

    JNI_info const *            m_jni_info;

    //
    ~Bridge() SAL_THROW( () );
    explicit Bridge(
        uno_Environment * java_env, uno_ExtEnvironment * uno_env,
        bool registered_java2uno );

    void acquire() const;
    void release() const;

    // jni_data.cxx
    void map_to_uno(
        JNI_context const & jni,
        void * uno_data, jvalue java_data,
        typelib_TypeDescriptionReference * type,
        JNI_type_info const * info /* maybe 0 */,
        bool assign, bool out_param,
        bool special_wrapped_integral_types = false ) const;
    void map_to_java(
        JNI_context const & jni,
        jvalue * java_data, void const * uno_data,
        typelib_TypeDescriptionReference * type,
        JNI_type_info const * info /* maybe 0 */,
        bool in_param, bool out_param,
        bool special_wrapped_integral_types = false ) const;

    // jni_uno2java.cxx
    void handle_uno_exc(
        JNI_context const & jni, uno_Any * uno_exc ) const;
    void call_java(
        jobject javaI,
        typelib_InterfaceTypeDescription * iface_td,
        sal_Int32 local_member_index, sal_Int32 function_pos_offset,
        typelib_TypeDescriptionReference * return_type,
        typelib_MethodParameter * params, sal_Int32 nParams,
        void * uno_ret, void * uno_args [], uno_Any ** uno_exc ) const;
    jobject map_to_java(
        JNI_context const & jni,
        uno_Interface * pUnoI, JNI_interface_type_info const * info ) const;

    // jni_java2uno.cxx
    void handle_java_exc(
        JNI_context const & jni,
        JLocalAutoRef const & jo_exc, uno_Any * uno_exc ) const;
    jobject call_uno(
        JNI_context const & jni,
        uno_Interface * pUnoI, typelib_TypeDescription * member_td,
        typelib_TypeDescriptionReference * return_tdref,
        sal_Int32 nParams, typelib_MethodParameter const * pParams,
        jobjectArray jo_args ) const;
    uno_Interface * map_to_uno(
        JNI_context const & jni,
        jobject javaI, JNI_interface_type_info const * info ) const;
};

}

#endif
