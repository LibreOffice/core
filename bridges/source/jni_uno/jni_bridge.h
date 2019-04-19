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

#include <atomic>
#include <cstddef>

#include "jni_base.h"
#include "jni_helper.h"

#include <osl/diagnose.h>

#include <uno/mapping.h>
#include <uno/dispatcher.h>

#include <com/sun/star/uno/XInterface.hpp>


namespace jni_uno
{

class JNI_info;
struct Bridge;

struct Mapping : public uno_Mapping
{
    Bridge * m_bridge;
};

// Holds environments and mappings:
struct Bridge
{
    mutable std::atomic<std::size_t> m_ref;

    uno_ExtEnvironment *        m_uno_env;
    uno_Environment *           m_java_env;

    Mapping                     m_java2uno;
    Mapping                     m_uno2java;
    bool                        m_registered_java2uno;

    ~Bridge();
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

    JNI_info const * getJniInfo() const;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
