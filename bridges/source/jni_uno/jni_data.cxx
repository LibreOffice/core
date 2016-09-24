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

#include <sal/config.h>

#include <cassert>
#include <memory>

#include "jni_bridge.h"

#include "rtl/strbuf.hxx"
#include "uno/sequence2.h"

namespace jni_uno
{


inline std::unique_ptr<rtl_mem> seq_allocate(
    sal_Int32 nElements, sal_Int32 nSize )
{
    std::unique_ptr< rtl_mem > seq(
        rtl_mem::allocate( SAL_SEQUENCE_HEADER_SIZE + (nElements * nSize) ) );
    uno_Sequence * p = reinterpret_cast<uno_Sequence *>(seq.get());
    p->nRefCount = 1;
    p->nElements = nElements;
    return seq;
}


namespace {

void createDefaultUnoValue(
    JNI_context const & jni, void * uno_data,
    typelib_TypeDescriptionReference * type,
    JNI_type_info const * info /* maybe 0 */, bool assign)
{
    switch (type->eTypeClass) {
    case typelib_TypeClass_BOOLEAN:
        *static_cast< sal_Bool * >(uno_data) = false;
        break;

    case typelib_TypeClass_BYTE:
        *static_cast< sal_Int8 * >(uno_data) = 0;
        break;

    case typelib_TypeClass_SHORT:
        *static_cast< sal_Int16 * >(uno_data) = 0;
        break;

    case typelib_TypeClass_UNSIGNED_SHORT:
        *static_cast< sal_uInt16 * >(uno_data) = 0;
        break;

    case typelib_TypeClass_LONG:
        *static_cast< sal_Int32 * >(uno_data) = 0;
        break;

    case typelib_TypeClass_UNSIGNED_LONG:
        *static_cast< sal_uInt32 * >(uno_data) = 0;
        break;

    case typelib_TypeClass_HYPER:
        *static_cast< sal_Int64 * >(uno_data) = 0;
        break;

    case typelib_TypeClass_UNSIGNED_HYPER:
        *static_cast< sal_uInt64 * >(uno_data) = 0;
        break;

    case typelib_TypeClass_FLOAT:
        *static_cast< float * >(uno_data) = 0;
        break;

    case typelib_TypeClass_DOUBLE:
        *static_cast< double * >(uno_data) = 0;
        break;

    case typelib_TypeClass_CHAR:
        *static_cast< sal_Unicode * >(uno_data) = 0;
        break;

    case typelib_TypeClass_STRING:
        if (!assign) {
            *static_cast< rtl_uString ** >(uno_data) = nullptr;
        }
        rtl_uString_new(static_cast< rtl_uString ** >(uno_data));
        break;

    case typelib_TypeClass_TYPE:
        if (assign) {
            typelib_typedescriptionreference_release(
                *static_cast< typelib_TypeDescriptionReference ** >(uno_data));
        }
        *static_cast< typelib_TypeDescriptionReference ** >(uno_data)
            = *typelib_static_type_getByTypeClass(typelib_TypeClass_VOID);
        assert(
            *static_cast< typelib_TypeDescriptionReference ** >(uno_data) != nullptr);
        typelib_typedescriptionreference_acquire(
            *static_cast< typelib_TypeDescriptionReference ** >(uno_data));
        break;

    case typelib_TypeClass_ANY:
        if (assign) {
            uno_any_destruct(static_cast< uno_Any * >(uno_data), nullptr);
        }
        uno_any_construct(
            static_cast< uno_Any * >(uno_data), nullptr,
            jni.get_info()->m_XInterface_type_info->m_td.get(), nullptr);
        break;

    case typelib_TypeClass_SEQUENCE:
        {
            std::unique_ptr< rtl_mem > seq(seq_allocate(0, 0));
            if (assign) {
                uno_type_destructData(uno_data, type, nullptr);
            }
            *static_cast< uno_Sequence ** >(uno_data)
                = reinterpret_cast< uno_Sequence * >(seq.release());
            break;
        }

    case typelib_TypeClass_ENUM:
        {
            typelib_TypeDescription * td = nullptr;
            TYPELIB_DANGER_GET(&td, type);
            *static_cast< sal_Int32 * >(uno_data)
                = (reinterpret_cast< typelib_EnumTypeDescription * >(td)->
                   nDefaultEnumValue);
            TYPELIB_DANGER_RELEASE(td);
            break;
        }

    case typelib_TypeClass_STRUCT:
        {
            if (info == nullptr) {
                info = jni.get_info()->get_type_info(jni, type);
            }
            JNI_compound_type_info const * comp_info
                = static_cast< JNI_compound_type_info const * >(info);
            typelib_CompoundTypeDescription * comp_td
                = reinterpret_cast< typelib_CompoundTypeDescription * >(
                    comp_info->m_td.get());
            sal_Int32 nPos = 0;
            sal_Int32 nMembers = comp_td->nMembers;
            try {
                if (comp_td->pBaseTypeDescription != nullptr) {
                    createDefaultUnoValue(
                        jni, uno_data,
                        comp_td->pBaseTypeDescription->aBase.pWeakRef,
                        comp_info->m_base, assign);
                }
                for (; nPos < nMembers; ++nPos) {
                    createDefaultUnoValue(
                        jni,
                        (static_cast< char * >(uno_data)
                         + comp_td->pMemberOffsets[nPos]),
                        comp_td->ppTypeRefs[nPos], nullptr, assign);
                }
            } catch (...) {
                if (!assign) {
                    for (sal_Int32 i = 0; i < nPos; ++i) {
                        uno_type_destructData(
                            (static_cast< char * >(uno_data)
                             + comp_td->pMemberOffsets[i]),
                            comp_td->ppTypeRefs[i], nullptr);
                    }
                    if (comp_td->pBaseTypeDescription != nullptr) {
                        uno_destructData(
                            uno_data, &comp_td->pBaseTypeDescription->aBase, nullptr);
                    }
                }
                throw;
            }
        }
        break;

    case typelib_TypeClass_INTERFACE:
        if (assign) {
            uno_Interface * p = *static_cast< uno_Interface ** >(uno_data);
            if (p != nullptr) {
                (*p->release)(p);
            }
        }
        *static_cast< uno_Interface ** >(uno_data) = nullptr;
        break;

    default:
        assert(false);
        break;
    }
}

}

void Bridge::map_to_uno(
    JNI_context const & jni,
    void * uno_data, jvalue java_data,
    typelib_TypeDescriptionReference * type,
    JNI_type_info const * info /* maybe 0 */,
    bool assign, bool out_param,
    bool special_wrapped_integral_types ) const
{
    assert(
        !out_param ||
        (1 == jni->GetArrayLength( static_cast<jarray>(java_data.l) )) );

    switch (type->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        if (out_param)
        {
            jni->GetCharArrayRegion(
                static_cast<jcharArray>(java_data.l), 0, 1, static_cast<jchar *>(uno_data) );
            jni.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *static_cast<jchar *>(uno_data) = jni->CallCharMethodA(
                java_data.l, getJniInfo()->m_method_Character_charValue, nullptr );
            jni.ensure_no_exception();
        }
        else
        {
            *static_cast<jchar *>(uno_data) = java_data.c;
        }
        break;
    case typelib_TypeClass_BOOLEAN:
        if (out_param)
        {
            jni->GetBooleanArrayRegion(
                static_cast<jbooleanArray>(java_data.l), 0, 1, static_cast<jboolean *>(uno_data) );
            jni.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *static_cast<jboolean *>(uno_data) = jni->CallBooleanMethodA(
                java_data.l, getJniInfo()->m_method_Boolean_booleanValue, nullptr );
            jni.ensure_no_exception();
        }
        else
        {
            *static_cast<jboolean *>(uno_data) = java_data.z;
        }
        break;
    case typelib_TypeClass_BYTE:
        if (out_param)
        {
            jni->GetByteArrayRegion(
                static_cast<jbyteArray>(java_data.l), 0, 1, static_cast<jbyte *>(uno_data) );
            jni.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *static_cast<jbyte *>(uno_data) = jni->CallByteMethodA(
                java_data.l, getJniInfo()->m_method_Byte_byteValue, nullptr );
            jni.ensure_no_exception();
        }
        else
        {
            *static_cast<jbyte *>(uno_data) = java_data.b;
        }
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        if (out_param)
        {
            jni->GetShortArrayRegion(
                static_cast<jshortArray>(java_data.l), 0, 1, static_cast<jshort *>(uno_data) );
            jni.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *static_cast<jshort *>(uno_data) = jni->CallShortMethodA(
                java_data.l, getJniInfo()->m_method_Short_shortValue, nullptr );
            jni.ensure_no_exception();
        }
        else
        {
            *static_cast<jshort *>(uno_data) = java_data.s;
        }
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        if (out_param)
        {
            jni->GetIntArrayRegion(
                static_cast<jintArray>(java_data.l), 0, 1, static_cast<jint *>(uno_data) );
            jni.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *static_cast<jint *>(uno_data) = jni->CallIntMethodA(
                java_data.l, getJniInfo()->m_method_Integer_intValue, nullptr );
            jni.ensure_no_exception();
        }
        else
        {
            *static_cast<jint *>(uno_data) = java_data.i;
        }
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        if (out_param)
        {
            jni->GetLongArrayRegion(
                static_cast<jlongArray>(java_data.l), 0, 1, static_cast<jlong *>(uno_data) );
            jni.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *static_cast<jlong *>(uno_data) = jni->CallLongMethodA(
                java_data.l, getJniInfo()->m_method_Long_longValue, nullptr );
            jni.ensure_no_exception();
        }
        else
        {
            *static_cast<jlong *>(uno_data) = java_data.j;
        }
        break;
    case typelib_TypeClass_FLOAT:
        if (out_param)
        {
            jni->GetFloatArrayRegion(
                static_cast<jfloatArray>(java_data.l), 0, 1, static_cast<jfloat *>(uno_data) );
            jni.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *static_cast<jfloat *>(uno_data) = jni->CallFloatMethodA(
                java_data.l, getJniInfo()->m_method_Float_floatValue, nullptr );
            jni.ensure_no_exception();
        }
        else
        {
            *static_cast<jfloat *>(uno_data) = java_data.f;
        }
        break;
    case typelib_TypeClass_DOUBLE:
        if (out_param)
        {
            jni->GetDoubleArrayRegion(
                static_cast<jdoubleArray>(java_data.l), 0, 1, static_cast<jdouble *>(uno_data) );
            jni.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *static_cast<jdouble *>(uno_data) = jni->CallDoubleMethodA(
                java_data.l, getJniInfo()->m_method_Double_doubleValue, nullptr );
            jni.ensure_no_exception();
        }
        else
        {
            *static_cast<jdouble *>(uno_data) = java_data.d;
        }
        break;
    case typelib_TypeClass_STRING:
    {
        JLocalAutoRef jo_out_holder( jni );
        if (out_param)
        {
            jo_out_holder.reset(
                jni->GetObjectArrayElement( static_cast<jobjectArray>(java_data.l), 0 ) );
            jni.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }
        if (nullptr == java_data.l)
        {
            throw BridgeRuntimeError(
                "[map_to_uno():" + OUString::unacquired( &type->pTypeName )
                + "] null-ref given!" + jni.get_stack_trace() );
        }
        if (! assign)
            *static_cast<rtl_uString **>(uno_data) = nullptr;
        jstring_to_ustring(
            jni, static_cast<rtl_uString **>(uno_data), static_cast<jstring>(java_data.l) );
        break;
    }
    case typelib_TypeClass_TYPE:
    {
        JLocalAutoRef jo_out_holder( jni );
        if (out_param)
        {
            jo_out_holder.reset(
                jni->GetObjectArrayElement( static_cast<jobjectArray>(java_data.l), 0 ) );
            jni.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }
        if (nullptr == java_data.l)
        {
            throw BridgeRuntimeError(
                "[map_to_uno():" + OUString::unacquired( &type->pTypeName )
                + "] null-ref given!" + jni.get_stack_trace() );
        }

        // type name
        JLocalAutoRef jo_type_name(
            jni, jni->GetObjectField(
                java_data.l, getJniInfo()->m_field_Type_typeName ) );
        if (! jo_type_name.is())
        {
            throw BridgeRuntimeError(
                "[map_to_uno():" + OUString::unacquired( &type->pTypeName )
                + "] incomplete type object: no type name!"
                + jni.get_stack_trace() );
        }
        OUString type_name(
            jstring_to_oustring( jni, static_cast<jstring>(jo_type_name.get()) ) );
        ::com::sun::star::uno::TypeDescription td( type_name );
        if (! td.is())
        {
            throw BridgeRuntimeError(
                "[map_to_uno():" + OUString::unacquired( &type->pTypeName )
                + "] UNO type not found: " + type_name
                + jni.get_stack_trace() );
        }
        typelib_typedescriptionreference_acquire( td.get()->pWeakRef );
        if (assign)
        {
            typelib_typedescriptionreference_release(
                *static_cast<typelib_TypeDescriptionReference **>(uno_data) );
        }
        *static_cast<typelib_TypeDescriptionReference **>(uno_data) = td.get()->pWeakRef;
        break;
    }
    case typelib_TypeClass_ANY:
    {
        JLocalAutoRef jo_out_holder( jni );
        if (out_param)
        {
            jo_out_holder.reset(
                jni->GetObjectArrayElement( static_cast<jobjectArray>(java_data.l), 0 ) );
            jni.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }

        uno_Any * pAny = static_cast<uno_Any *>(uno_data);
        if (nullptr == java_data.l) // null-ref maps to XInterface null-ref
        {
            if (assign)
                uno_any_destruct( pAny, nullptr );
            uno_any_construct(
                pAny, nullptr, getJniInfo()->m_XInterface_type_info->m_td.get(), nullptr );
            break;
        }

        JLocalAutoRef jo_type( jni );
        JLocalAutoRef jo_wrapped_holder( jni );

        if (jni->IsInstanceOf( java_data.l, getJniInfo()->m_class_Any ))
        {
            // boxed any
            jo_type.reset( jni->GetObjectField(
                               java_data.l, getJniInfo()->m_field_Any_type ) );
            if (! jo_type.is())
            {
                throw BridgeRuntimeError(
                    "[map_to_uno():" + OUString::unacquired( &type->pTypeName )
                    + "] no type set at com.sun.star.uno.Any!"
                    + jni.get_stack_trace() );
            }
            // wrapped value
            jo_wrapped_holder.reset(
                jni->GetObjectField(
                    java_data.l, getJniInfo()->m_field_Any_object ) );
            java_data.l = jo_wrapped_holder.get();
        }
        else
        {
            // create type out of class
            JLocalAutoRef jo_class( jni, jni->GetObjectClass( java_data.l ) );
            jo_type.reset( create_type( jni, static_cast<jclass>(jo_class.get()) ) );
        }

        // get type name
        JLocalAutoRef jo_type_name(
            jni, jni->GetObjectField(
                jo_type.get(), getJniInfo()->m_field_Type_typeName ) );
        jni.ensure_no_exception();
        OUString type_name(
            jstring_to_oustring( jni, static_cast<jstring>(jo_type_name.get()) ) );

        ::com::sun::star::uno::TypeDescription value_td( type_name );
        if (! value_td.is())
        {
            throw BridgeRuntimeError(
                "[map_to_uno():" + OUString::unacquired( &type->pTypeName )
                + "] UNO type not found: " + type_name
                + jni.get_stack_trace() );
        }
        typelib_TypeClass type_class = value_td.get()->eTypeClass;

        if (assign)
        {
            uno_any_destruct( pAny, nullptr );
        }
        try
        {
            switch (type_class)
            {
            case typelib_TypeClass_VOID:
                pAny->pData = &pAny->pReserved;
                break;
            case typelib_TypeClass_CHAR:
                pAny->pData = &pAny->pReserved;
                *static_cast<jchar *>(pAny->pData) = jni->CallCharMethodA(
                    java_data.l, getJniInfo()->m_method_Character_charValue, nullptr );
                jni.ensure_no_exception();
                break;
            case typelib_TypeClass_BOOLEAN:
                pAny->pData = &pAny->pReserved;
                *static_cast<jboolean *>(pAny->pData) = jni->CallBooleanMethodA(
                    java_data.l, getJniInfo()->m_method_Boolean_booleanValue, nullptr );
                jni.ensure_no_exception();
                break;
            case typelib_TypeClass_BYTE:
                pAny->pData = &pAny->pReserved;
                *static_cast<jbyte *>(pAny->pData) = jni->CallByteMethodA(
                    java_data.l, getJniInfo()->m_method_Byte_byteValue, nullptr );
                jni.ensure_no_exception();
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
                pAny->pData = &pAny->pReserved;
                *static_cast<jshort *>(pAny->pData) = jni->CallShortMethodA(
                    java_data.l, getJniInfo()->m_method_Short_shortValue, nullptr );
                jni.ensure_no_exception();
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
                pAny->pData = &pAny->pReserved;
                *static_cast<jint *>(pAny->pData) = jni->CallIntMethodA(
                    java_data.l, getJniInfo()->m_method_Integer_intValue, nullptr );
                jni.ensure_no_exception();
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                if (sizeof (sal_Int64) <= sizeof (void *))
                {
                    pAny->pData = &pAny->pReserved;
                    *static_cast<jlong *>(pAny->pData) = jni->CallLongMethodA(
                        java_data.l, getJniInfo()->m_method_Long_longValue, nullptr );
                    jni.ensure_no_exception();
                }
                else
                {
                    std::unique_ptr< rtl_mem > mem(
                        rtl_mem::allocate( sizeof (sal_Int64) ) );
                    *reinterpret_cast<jlong *>(mem.get()) = jni->CallLongMethodA(
                        java_data.l, getJniInfo()->m_method_Long_longValue, nullptr );
                    jni.ensure_no_exception();
                    pAny->pData = mem.release();
                }
                break;
            case typelib_TypeClass_FLOAT:
                if (sizeof (float) <= sizeof (void *))
                {
                    pAny->pData = &pAny->pReserved;
                    *static_cast<jfloat *>(pAny->pData) = jni->CallFloatMethodA(
                        java_data.l, getJniInfo()->m_method_Float_floatValue, nullptr );
                    jni.ensure_no_exception();
                }
                else
                {
                    std::unique_ptr< rtl_mem > mem(
                        rtl_mem::allocate( sizeof (float) ) );
                    *reinterpret_cast<jfloat *>(mem.get()) = jni->CallFloatMethodA(
                        java_data.l, getJniInfo()->m_method_Float_floatValue, nullptr );
                    jni.ensure_no_exception();
                    pAny->pData = mem.release();
                }
                break;
            case typelib_TypeClass_DOUBLE:
                if (sizeof (double) <= sizeof (void *))
                {
                    pAny->pData = &pAny->pReserved;
                    *static_cast<jdouble *>(pAny->pData) =
                        jni->CallDoubleMethodA(
                            java_data.l,
                            getJniInfo()->m_method_Double_doubleValue, nullptr );
                    jni.ensure_no_exception();
                }
                else
                {
                    std::unique_ptr< rtl_mem > mem(
                        rtl_mem::allocate( sizeof (double) ) );
                    *reinterpret_cast<jdouble *>(mem.get()) =
                        jni->CallDoubleMethodA(
                            java_data.l,
                            getJniInfo()->m_method_Double_doubleValue, nullptr );
                    jni.ensure_no_exception();
                    pAny->pData = mem.release();
                }
                break;
            case typelib_TypeClass_STRING:
                // opt: anies often contain strings; copy string directly
                pAny->pReserved = nullptr;
                pAny->pData = &pAny->pReserved;
                jstring_to_ustring(
                    jni, static_cast<rtl_uString **>(pAny->pData),
                    static_cast<jstring>(java_data.l) );
                break;
            case typelib_TypeClass_TYPE:
            case typelib_TypeClass_ENUM:
            case typelib_TypeClass_SEQUENCE:
            case typelib_TypeClass_INTERFACE:
                pAny->pData = &pAny->pReserved;
                map_to_uno(
                    jni, pAny->pData, java_data,
                    value_td.get()->pWeakRef, nullptr,
                    false /* no assign */, false /* no out param */ );
                break;
            case typelib_TypeClass_STRUCT:
            case typelib_TypeClass_EXCEPTION:
            {
                std::unique_ptr< rtl_mem > mem(
                    rtl_mem::allocate( value_td.get()->nSize ) );
                map_to_uno(
                    jni, mem.get(), java_data, value_td.get()->pWeakRef, nullptr,
                    false /* no assign */, false /* no out param */ );
                pAny->pData = mem.release();
                break;
            }
            default:
            {
                throw BridgeRuntimeError(
                    "[map_to_uno():" + type_name
                    + "] unsupported value type of any!"
                    + jni.get_stack_trace() );
            }
            }
        }
        catch (...)
        {
            if (assign)
            {
                // restore to valid any
                uno_any_construct( pAny, nullptr, nullptr, nullptr );
            }
            throw;
        }
        typelib_typedescriptionreference_acquire( value_td.get()->pWeakRef );
        pAny->pType = value_td.get()->pWeakRef;
        break;
    }
    case typelib_TypeClass_ENUM:
    {
        JLocalAutoRef jo_out_holder( jni );
        if (out_param)
        {
            jo_out_holder.reset(
                jni->GetObjectArrayElement( static_cast<jobjectArray>(java_data.l), 0 ) );
            jni.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }
        if (nullptr == java_data.l)
        {
            throw BridgeRuntimeError(
                "[map_to_uno():" + OUString::unacquired( &type->pTypeName )
                + "] null-ref given!" + jni.get_stack_trace() );
        }

        *static_cast<jint *>(uno_data) = jni->GetIntField(
            java_data.l, getJniInfo()->m_field_Enum_m_value );
        break;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        JLocalAutoRef jo_out_holder( jni );
        if (out_param)
        {
            jo_out_holder.reset(
                jni->GetObjectArrayElement( static_cast<jobjectArray>(java_data.l), 0 ) );
            jni.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }
        if (nullptr == java_data.l)
        {
            throw BridgeRuntimeError(
                "[map_to_uno():" + OUString::unacquired( &type->pTypeName )
                + "] null-ref given!" + jni.get_stack_trace() );
        }

        if (nullptr == info)
            info = getJniInfo()->get_type_info( jni, type );
        JNI_compound_type_info const * comp_info =
            static_cast< JNI_compound_type_info const * >( info );

        typelib_CompoundTypeDescription * comp_td =
            reinterpret_cast<typelib_CompoundTypeDescription *>(comp_info->m_td.get());
        bool polymorphic
            = comp_td->aBase.eTypeClass == typelib_TypeClass_STRUCT
            && reinterpret_cast< typelib_StructTypeDescription * >(
                comp_td)->pParameterizedTypes != nullptr;

        sal_Int32 nPos = 0;
        sal_Int32 nMembers = comp_td->nMembers;
        try
        {
            if (nullptr != comp_td->pBaseTypeDescription)
            {
                map_to_uno(
                    jni, uno_data, java_data,
                    comp_td->pBaseTypeDescription->aBase.pWeakRef,
                    comp_info->m_base,
                    assign, false /* no out param */ );
            }

            for ( ; nPos < nMembers; ++nPos )
            {
                void * p = static_cast<char *>(uno_data) + comp_td->pMemberOffsets[ nPos ];
                typelib_TypeDescriptionReference * member_type =
                    comp_td->ppTypeRefs[ nPos ];
                jfieldID field_id = comp_info->m_fields[ nPos ];
                bool parameterizedType = polymorphic
                    && reinterpret_cast< typelib_StructTypeDescription * >(
                        comp_td)->pParameterizedTypes[nPos];
                switch (member_type->eTypeClass)
                {
                case typelib_TypeClass_CHAR:
                    if (parameterizedType) {
                        JLocalAutoRef jo(
                            jni, jni->GetObjectField( java_data.l, field_id ) );
                        if ( jo.get() == nullptr ) {
                            *static_cast<jchar *>(p) = 0;
                        } else {
                            jvalue val;
                            val.l = jo.get();
                            map_to_uno(
                                jni, p, val, member_type, nullptr, assign, false,
                                true );
                        }
                    } else {
                        *static_cast<jchar *>(p) = jni->GetCharField(
                            java_data.l, field_id );
                    }
                    break;
                case typelib_TypeClass_BOOLEAN:
                    if (parameterizedType) {
                        JLocalAutoRef jo(
                            jni, jni->GetObjectField( java_data.l, field_id ) );
                        if ( jo.get() == nullptr ) {
                            *static_cast<jboolean *>(p) = false;
                        } else {
                            jvalue val;
                            val.l = jo.get();
                            map_to_uno(
                                jni, p, val, member_type, nullptr, assign, false,
                                true );
                        }
                    } else {
                        *static_cast<jboolean *>(p) = jni->GetBooleanField(
                            java_data.l, field_id );
                    }
                    break;
                case typelib_TypeClass_BYTE:
                    if (parameterizedType) {
                        JLocalAutoRef jo(
                            jni, jni->GetObjectField( java_data.l, field_id ) );
                        if ( jo.get() == nullptr ) {
                            *static_cast<jbyte *>(p) = 0;
                        } else {
                            jvalue val;
                            val.l = jo.get();
                            map_to_uno(
                                jni, p, val, member_type, nullptr, assign, false,
                                true );
                        }
                    } else {
                        *static_cast<jbyte *>(p) = jni->GetByteField(
                            java_data.l, field_id );
                    }
                    break;
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                    if (parameterizedType) {
                        JLocalAutoRef jo(
                            jni, jni->GetObjectField( java_data.l, field_id ) );
                        if ( jo.get() == nullptr ) {
                            *static_cast<jshort *>(p) = 0;
                        } else {
                            jvalue val;
                            val.l = jo.get();
                            map_to_uno(
                                jni, p, val, member_type, nullptr, assign, false,
                                true );
                        }
                    } else {
                        *static_cast<jshort *>(p) = jni->GetShortField(
                            java_data.l, field_id );
                    }
                    break;
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                    if (parameterizedType) {
                        JLocalAutoRef jo(
                            jni, jni->GetObjectField( java_data.l, field_id ) );
                        if ( jo.get() == nullptr ) {
                            *static_cast<jint *>(p) = 0;
                        } else {
                            jvalue val;
                            val.l = jo.get();
                            map_to_uno(
                                jni, p, val, member_type, nullptr, assign, false,
                                true );
                        }
                    } else {
                        *static_cast<jint *>(p) = jni->GetIntField( java_data.l, field_id );
                    }
                    break;
                case typelib_TypeClass_HYPER:
                case typelib_TypeClass_UNSIGNED_HYPER:
                    if (parameterizedType) {
                        JLocalAutoRef jo(
                            jni, jni->GetObjectField( java_data.l, field_id ) );
                        if ( jo.get() == nullptr ) {
                            *static_cast<jlong *>(p) = 0;
                        } else {
                            jvalue val;
                            val.l = jo.get();
                            map_to_uno(
                                jni, p, val, member_type, nullptr, assign, false,
                                true );
                        }
                    } else {
                        *static_cast<jlong *>(p) = jni->GetLongField(
                            java_data.l, field_id );
                    }
                    break;
                case typelib_TypeClass_FLOAT:
                    if (parameterizedType) {
                        JLocalAutoRef jo(
                            jni, jni->GetObjectField( java_data.l, field_id ) );
                        if ( jo.get() == nullptr ) {
                            *static_cast<jfloat *>(p) = 0;
                        } else {
                            jvalue val;
                            val.l = jo.get();
                            map_to_uno(
                                jni, p, val, member_type, nullptr, assign, false,
                                true );
                        }
                    } else {
                        *static_cast<jfloat *>(p) = jni->GetFloatField(
                            java_data.l, field_id );
                    }
                    break;
                case typelib_TypeClass_DOUBLE:
                    if (parameterizedType) {
                        JLocalAutoRef jo(
                            jni, jni->GetObjectField( java_data.l, field_id ) );
                        if ( jo.get() == nullptr ) {
                            *static_cast<jdouble *>(p) = 0;
                        } else {
                            jvalue val;
                            val.l = jo.get();
                            map_to_uno(
                                jni, p, val, member_type, nullptr, assign, false,
                                true );
                        }
                    } else {
                        *static_cast<jdouble *>(p) = jni->GetDoubleField(
                            java_data.l, field_id );
                    }
                    break;
                default:
                {
                    JLocalAutoRef jo_field( jni );
                    bool checkNull;
                    if (nullptr == field_id)
                    {
                        // special for Message: call Throwable.getMessage()
                        assert(
                            type_equals(
                                type,
                                getJniInfo()->m_Exception_type.getTypeLibType() )
                            || type_equals(
                                type,
                                getJniInfo()->m_RuntimeException_type.
                                getTypeLibType() ) );
                        assert( 0 == nPos ); // first member
                        // call getMessage()
                        jo_field.reset(
                            jni->CallObjectMethodA(
                                java_data.l,
                                getJniInfo()->m_method_Throwable_getMessage, nullptr )
                            );
                        jni.ensure_no_exception();
                        checkNull = true;
                    }
                    else
                    {
                        jo_field.reset(
                            jni->GetObjectField( java_data.l, field_id ) );
                        checkNull = parameterizedType;
                    }
                    if (checkNull && !jo_field.is()) {
                        createDefaultUnoValue(jni, p, member_type, nullptr, assign);
                    } else {
                        jvalue val;
                        val.l = jo_field.get();
                        map_to_uno(
                            jni, p, val, member_type, nullptr,
                            assign, false /* no out param */ );
                    }
                    break;
                }
                }
            }
        }
        catch (...)
        {
            if (! assign)
            {
                // cleanup
                for ( sal_Int32 nCleanup = 0; nCleanup < nPos; ++nCleanup )
                {
                    void * p =
                        static_cast<char *>(uno_data) + comp_td->pMemberOffsets[ nCleanup ];
                    uno_type_destructData(
                        p, comp_td->ppTypeRefs[ nCleanup ], nullptr );
                }
                if (nullptr != comp_td->pBaseTypeDescription)
                {
                    uno_destructData(
                        uno_data, &comp_td->pBaseTypeDescription->aBase, nullptr );
                }
            }
            throw;
        }
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        JLocalAutoRef jo_out_holder( jni );
        if (out_param)
        {
            jo_out_holder.reset(
                jni->GetObjectArrayElement( static_cast<jobjectArray>(java_data.l), 0 ) );
            jni.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }
        if (nullptr == java_data.l)
        {
            throw BridgeRuntimeError(
                "[map_to_uno():" + OUString::unacquired( &type->pTypeName )
                + "] null-ref given!" + jni.get_stack_trace() );
        }

        TypeDescr td( type );
        typelib_TypeDescriptionReference * element_type =
            reinterpret_cast<typelib_IndirectTypeDescription *>(td.get())->pType;

        std::unique_ptr< rtl_mem > seq;
        sal_Int32 nElements = jni->GetArrayLength( static_cast<jarray>(java_data.l) );

        switch (element_type->eTypeClass)
        {
        case typelib_TypeClass_CHAR:
            seq = seq_allocate( nElements, sizeof (sal_Unicode) );
            jni->GetCharArrayRegion(
                static_cast<jcharArray>(java_data.l), 0, nElements,
                reinterpret_cast<jchar *>(reinterpret_cast<uno_Sequence *>(seq.get())->elements) );
            jni.ensure_no_exception();
            break;
        case typelib_TypeClass_BOOLEAN:
            seq = seq_allocate( nElements, sizeof (sal_Bool) );
            jni->GetBooleanArrayRegion(
                static_cast<jbooleanArray>(java_data.l), 0, nElements,
                reinterpret_cast<jboolean *>(reinterpret_cast<uno_Sequence *>(seq.get())->elements) );
            jni.ensure_no_exception();
            break;
        case typelib_TypeClass_BYTE:
            seq = seq_allocate( nElements, sizeof (sal_Int8) );
            jni->GetByteArrayRegion(
                static_cast<jbyteArray>(java_data.l), 0, nElements,
                reinterpret_cast<jbyte *>(reinterpret_cast<uno_Sequence *>(seq.get())->elements) );
            jni.ensure_no_exception();
            break;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            seq = seq_allocate( nElements, sizeof (sal_Int16) );
            jni->GetShortArrayRegion(
                static_cast<jshortArray>(java_data.l), 0, nElements,
                reinterpret_cast<jshort *>(reinterpret_cast<uno_Sequence *>(seq.get())->elements) );
            jni.ensure_no_exception();
            break;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
            seq = seq_allocate( nElements, sizeof (sal_Int32) );
            jni->GetIntArrayRegion(
                static_cast<jintArray>(java_data.l), 0, nElements,
                reinterpret_cast<jint *>(reinterpret_cast<uno_Sequence *>(seq.get())->elements) );
            jni.ensure_no_exception();
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            seq = seq_allocate( nElements, sizeof (sal_Int64) );
            jni->GetLongArrayRegion(
                static_cast<jlongArray>(java_data.l), 0, nElements,
                reinterpret_cast<jlong *>(reinterpret_cast<uno_Sequence *>(seq.get())->elements) );
            jni.ensure_no_exception();
            break;
        case typelib_TypeClass_FLOAT:
            seq = seq_allocate( nElements, sizeof (float) );
            jni->GetFloatArrayRegion(
                static_cast<jfloatArray>(java_data.l), 0, nElements,
                reinterpret_cast<jfloat *>(reinterpret_cast<uno_Sequence *>(seq.get())->elements) );
            jni.ensure_no_exception();
            break;
        case typelib_TypeClass_DOUBLE:
            seq = seq_allocate( nElements, sizeof (double) );
            jni->GetDoubleArrayRegion(
                static_cast<jdoubleArray>(java_data.l), 0, nElements,
                reinterpret_cast<jdouble *>(reinterpret_cast<uno_Sequence *>(seq.get())->elements) );
            jni.ensure_no_exception();
            break;
        case typelib_TypeClass_STRING:
        case typelib_TypeClass_TYPE:
        case typelib_TypeClass_ANY:
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        case typelib_TypeClass_SEQUENCE:
        case typelib_TypeClass_INTERFACE:
        {
            TypeDescr element_td( element_type );
            seq = seq_allocate( nElements, element_td.get()->nSize );

            JNI_type_info const * element_info;
            if (typelib_TypeClass_STRUCT == element_type->eTypeClass ||
                typelib_TypeClass_EXCEPTION == element_type->eTypeClass ||
                typelib_TypeClass_INTERFACE == element_type->eTypeClass)
            {
                element_info =
                    getJniInfo()->get_type_info( jni, element_td.get() );
            }
            else
            {
                element_info = nullptr;
            }

            for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
            {
                try
                {
                    JLocalAutoRef jo(
                        jni, jni->GetObjectArrayElement(
                            static_cast<jobjectArray>(java_data.l), nPos ) );
                    jni.ensure_no_exception();
                    jvalue val;
                    val.l = jo.get();
                    void * p =
                        reinterpret_cast<uno_Sequence *>(seq.get())->elements +
                        (nPos * element_td.get()->nSize);
                    map_to_uno(
                        jni, p, val, element_td.get()->pWeakRef, element_info,
                        false /* no assign */, false /* no out param */ );
                }
                catch (...)
                {
                    // cleanup
                    for ( sal_Int32 nCleanPos = 0;
                          nCleanPos < nPos; ++nCleanPos )
                    {
                        void * p =
                            reinterpret_cast<uno_Sequence *>(seq.get())->elements +
                            (nCleanPos * element_td.get()->nSize);
                        uno_destructData( p, element_td.get(), nullptr );
                    }
                    throw;
                }
            }
            break;
        }
        default:
        {
            throw BridgeRuntimeError(
                "[map_to_uno():" + OUString::unacquired( &type->pTypeName )
                + "] unsupported sequence element type: "
                + OUString::unacquired( &element_type->pTypeName )
                + jni.get_stack_trace() );
        }
        }

        if (assign)
            uno_destructData( uno_data, td.get(), nullptr );
        *static_cast<uno_Sequence **>(uno_data) = reinterpret_cast<uno_Sequence *>(seq.release());
        break;
    }
    case typelib_TypeClass_INTERFACE:
    {
        JLocalAutoRef jo_out_holder( jni );
        if (out_param)
        {
            jo_out_holder.reset(
                jni->GetObjectArrayElement( static_cast<jobjectArray>(java_data.l), 0 ) );
            jni.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }

        if (nullptr == java_data.l) // null-ref
        {
            if (assign)
            {
                uno_Interface * p = *static_cast<uno_Interface **>(uno_data);
                if (nullptr != p)
                    (*p->release)( p );
            }
            *static_cast<uno_Interface **>(uno_data) = nullptr;
        }
        else
        {
            if (nullptr == info)
                info = getJniInfo()->get_type_info( jni, type );
            JNI_interface_type_info const * iface_info =
                static_cast< JNI_interface_type_info const * >( info );
            uno_Interface * pUnoI = map_to_uno( jni, java_data.l, iface_info );
            if (assign)
            {
                uno_Interface * p = *static_cast<uno_Interface **>(uno_data);
                if (nullptr != p)
                    (*p->release)( p );
            }
            *static_cast<uno_Interface **>(uno_data) = pUnoI;
        }
        break;
    }
    default:
    {
        throw BridgeRuntimeError(
            "[map_to_uno():" + OUString::unacquired( &type->pTypeName )
            + "] unsupported type!" + jni.get_stack_trace() );
    }
    }
}


void Bridge::map_to_java(
    JNI_context const & jni,
    jvalue * java_data, void const * uno_data,
    typelib_TypeDescriptionReference * type,
    JNI_type_info const * info /* maybe 0 */,
    bool in_param, bool out_param,
    bool special_wrapped_integral_types ) const
{
    // 4th param of Set*ArrayRegion changed from pointer to non-const to pointer
    // to const between <http://docs.oracle.com/javase/6/docs/technotes/guides/
    // jni/spec/functions.html#wp22933> and <http://docs.oracle.com/javase/7/
    // docs/technotes/guides/jni/spec/functions.html#wp22933>; work around that
    // difference in a way that doesn't trigger loplugin:redundantcast:
    void * data = const_cast<void *>(uno_data);

    switch (type->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        if (out_param)
        {
            if (nullptr == java_data->l)
            {
                JLocalAutoRef jo_ar( jni, jni->NewCharArray( 1 ) );
                jni.ensure_no_exception();
                if (in_param)
                {
                    jni->SetCharArrayRegion(
                        static_cast<jcharArray>(jo_ar.get()), 0, 1, static_cast<jchar *>(data) );
                    jni.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    jni->SetCharArrayRegion(
                        static_cast<jcharArray>(java_data->l), 0, 1, static_cast<jchar *>(data) );
                    jni.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.c = *static_cast<jchar const *>(uno_data);
            java_data->l = jni->NewObjectA(
                getJniInfo()->m_class_Character,
                getJniInfo()->m_ctor_Character_with_char, &arg );
            jni.ensure_no_exception();
        }
        else
        {
            java_data->c = *static_cast<jchar const *>(uno_data);
        }
        break;
    case typelib_TypeClass_BOOLEAN:
        if (out_param)
        {
            if (nullptr == java_data->l)
            {
                JLocalAutoRef jo_ar( jni, jni->NewBooleanArray( 1 ) );
                jni.ensure_no_exception();
                if (in_param)
                {
                    jni->SetBooleanArrayRegion(
                        static_cast<jbooleanArray>(jo_ar.get()),
                        0, 1, static_cast<jboolean *>(data) );
                    jni.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    jni->SetBooleanArrayRegion(
                        static_cast<jbooleanArray>(java_data->l),
                        0, 1, static_cast<jboolean *>(data) );
                    jni.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.z = *static_cast<jboolean const *>(uno_data);
            java_data->l = jni->NewObjectA(
                getJniInfo()->m_class_Boolean,
                getJniInfo()->m_ctor_Boolean_with_boolean, &arg );
            jni.ensure_no_exception();
        }
        else
        {
            java_data->z = *static_cast<jboolean const *>(uno_data);
        }
        break;
    case typelib_TypeClass_BYTE:
        if (out_param)
        {
            if (nullptr == java_data->l)
            {
                JLocalAutoRef jo_ar( jni, jni->NewByteArray( 1 ) );
                jni.ensure_no_exception();
                if (in_param)
                {
                    jni->SetByteArrayRegion(
                        static_cast<jbyteArray>(jo_ar.get()), 0, 1, static_cast<jbyte *>(data) );
                    jni.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    jni->SetByteArrayRegion(
                        static_cast<jbyteArray>(java_data->l), 0, 1, static_cast<jbyte *>(data) );
                    jni.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.b = *static_cast<jbyte const *>(uno_data);
            java_data->l = jni->NewObjectA(
                getJniInfo()->m_class_Byte,
                getJniInfo()->m_ctor_Byte_with_byte, &arg );
            jni.ensure_no_exception();
        }
        else
        {
            java_data->b = *static_cast<jbyte const *>(uno_data);
        }
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        if (out_param)
        {
            if (nullptr == java_data->l)
            {
                JLocalAutoRef jo_ar( jni, jni->NewShortArray( 1 ) );
                jni.ensure_no_exception();
                if (in_param)
                {
                    jni->SetShortArrayRegion(
                        static_cast<jshortArray>(jo_ar.get()), 0, 1, static_cast<jshort *>(data) );
                    jni.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    jni->SetShortArrayRegion(
                        static_cast<jshortArray>(java_data->l), 0, 1, static_cast<jshort *>(data) );
                    jni.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.s = *static_cast<jshort const *>(uno_data);
            java_data->l = jni->NewObjectA(
                getJniInfo()->m_class_Short,
                getJniInfo()->m_ctor_Short_with_short, &arg );
            jni.ensure_no_exception();
        }
        else
        {
            java_data->s = *static_cast<jshort const *>(uno_data);
        }
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        if (out_param)
        {
            if (nullptr == java_data->l)
            {
                JLocalAutoRef jo_ar( jni, jni->NewIntArray( 1 ) );
                jni.ensure_no_exception();
                if (in_param)
                {
                    jni->SetIntArrayRegion(
                        static_cast<jintArray>(jo_ar.get()), 0, 1, static_cast<jint *>(data) );
                    jni.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    jni->SetIntArrayRegion(
                        static_cast<jintArray>(java_data->l), 0, 1, static_cast<jint *>(data) );
                    jni.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.i = *static_cast<jint const *>(uno_data);
            java_data->l = jni->NewObjectA(
                getJniInfo()->m_class_Integer,
                getJniInfo()->m_ctor_Integer_with_int, &arg );
            jni.ensure_no_exception();
        }
        else
        {
            java_data->i = *static_cast<jint const *>(uno_data);
        }
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        if (out_param)
        {
            if (nullptr == java_data->l)
            {
                JLocalAutoRef jo_ar( jni, jni->NewLongArray( 1 ) );
                jni.ensure_no_exception();
                if (in_param)
                {
                    jni->SetLongArrayRegion(
                        static_cast<jlongArray>(jo_ar.get()), 0, 1, static_cast<jlong *>(data) );
                    jni.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    jni->SetLongArrayRegion(
                        static_cast<jlongArray>(java_data->l), 0, 1, static_cast<jlong *>(data) );
                    jni.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.j = *static_cast<jlong const *>(uno_data);
            java_data->l = jni->NewObjectA(
                getJniInfo()->m_class_Long,
                getJniInfo()->m_ctor_Long_with_long, &arg );
            jni.ensure_no_exception();
        }
        else
        {
            java_data->j = *static_cast<jlong const *>(uno_data);
        }
        break;
    case typelib_TypeClass_FLOAT:
        if (out_param)
        {
            if (nullptr == java_data->l)
            {
                JLocalAutoRef jo_ar( jni, jni->NewFloatArray( 1 ) );
                jni.ensure_no_exception();
                if (in_param)
                {
                    jni->SetFloatArrayRegion(
                        static_cast<jfloatArray>(jo_ar.get()), 0, 1, static_cast<jfloat *>(data) );
                    jni.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    jni->SetFloatArrayRegion(
                        static_cast<jfloatArray>(java_data->l), 0, 1, static_cast<jfloat *>(data) );
                    jni.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.f = *static_cast<jfloat const *>(uno_data);
            java_data->l = jni->NewObjectA(
                getJniInfo()->m_class_Float,
                getJniInfo()->m_ctor_Float_with_float, &arg );
            jni.ensure_no_exception();
        }
        else
        {
            java_data->f = *static_cast<jfloat const *>(uno_data);
        }
        break;
    case typelib_TypeClass_DOUBLE:
        if (out_param)
        {
            if (nullptr == java_data->l)
            {
                JLocalAutoRef jo_ar( jni, jni->NewDoubleArray( 1 ) );
                jni.ensure_no_exception();
                if (in_param)
                {
                    jni->SetDoubleArrayRegion(
                        static_cast<jdoubleArray>(jo_ar.get()),
                        0, 1, static_cast<jdouble *>(data) );
                    jni.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    jni->SetDoubleArrayRegion(
                        static_cast<jdoubleArray>(java_data->l),
                        0, 1, static_cast<jdouble *>(data) );
                    jni.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.d = *static_cast<double const *>(uno_data);
            java_data->l = jni->NewObjectA(
                getJniInfo()->m_class_Double,
                getJniInfo()->m_ctor_Double_with_double, &arg );
            jni.ensure_no_exception();
        }
        else
        {
            java_data->d = *static_cast<jdouble const *>(uno_data);
        }
        break;
    case typelib_TypeClass_STRING:
    {
        if (out_param)
        {
            JLocalAutoRef jo_in( jni );
            if (in_param)
            {
                jo_in.reset(
                    ustring_to_jstring(
                        jni, *static_cast<rtl_uString * const *>(uno_data) ) );
            }
            if (nullptr == java_data->l)
            {
                java_data->l = jni->NewObjectArray(
                    1, getJniInfo()->m_class_String, jo_in.get() );
                jni.ensure_no_exception();
            }
            else
            {
                jni->SetObjectArrayElement(
                    static_cast<jobjectArray>(java_data->l), 0, jo_in.get() );
                jni.ensure_no_exception();
            }
        }
        else
        {
            assert( in_param );
            java_data->l =
                ustring_to_jstring( jni, *static_cast<rtl_uString * const *>(uno_data) );
        }
        break;
    }
    case typelib_TypeClass_TYPE:
    {
        if (out_param)
        {
            JLocalAutoRef jo_in( jni );
            if (in_param)
            {
                jo_in.reset(
                    create_type(
                        jni,
                        *static_cast<typelib_TypeDescriptionReference * const *>(uno_data) )
                    );
            }
            if (nullptr == java_data->l)
            {
                java_data->l = jni->NewObjectArray(
                    1, getJniInfo()->m_class_Type, jo_in.get() );
                jni.ensure_no_exception();
            }
            else
            {
                jni->SetObjectArrayElement(
                    static_cast<jobjectArray>(java_data->l), 0, jo_in.get() );
                jni.ensure_no_exception();
            }
        }
        else
        {
            assert( in_param );
            java_data->l =
                create_type(
                    jni,
                    *static_cast<typelib_TypeDescriptionReference * const *>(uno_data) );
        }
        break;
    }
    case typelib_TypeClass_ANY:
    {
        JLocalAutoRef jo_any( jni );
        if (in_param)
        {
            uno_Any const * pAny = static_cast<uno_Any const *>(uno_data);
            switch (pAny->pType->eTypeClass)
            {
            case typelib_TypeClass_VOID:
                jo_any.reset(
                    jni->NewLocalRef( getJniInfo()->m_object_Any_VOID ) );
                break;
            case typelib_TypeClass_UNSIGNED_SHORT:
            {
                jvalue args[ 2 ];
                args[ 0 ].s = *static_cast<jshort const *>(pAny->pData);
                JLocalAutoRef jo_val(
                    jni, jni->NewObjectA(
                        getJniInfo()->m_class_Short,
                        getJniInfo()->m_ctor_Short_with_short, args ) );
                jni.ensure_no_exception();
                // box up in com.sun.star.uno.Any
                args[ 0 ].l = getJniInfo()->m_object_Type_UNSIGNED_SHORT;
                args[ 1 ].l = jo_val.get();
                jo_any.reset(
                    jni->NewObjectA(
                        getJniInfo()->m_class_Any,
                        getJniInfo()->m_ctor_Any_with_Type_Object, args ) );
                jni.ensure_no_exception();
                break;
            }
            case typelib_TypeClass_UNSIGNED_LONG:
            {
                jvalue args[ 2 ];
                args[ 0 ].i = *static_cast<jint const *>(pAny->pData);
                JLocalAutoRef jo_val(
                    jni, jni->NewObjectA(
                        getJniInfo()->m_class_Integer,
                        getJniInfo()->m_ctor_Integer_with_int, args ) );
                jni.ensure_no_exception();
                // box up in com.sun.star.uno.Any
                args[ 0 ].l = getJniInfo()->m_object_Type_UNSIGNED_LONG;
                args[ 1 ].l = jo_val.get();
                jo_any.reset(
                    jni->NewObjectA(
                        getJniInfo()->m_class_Any,
                        getJniInfo()->m_ctor_Any_with_Type_Object, args ) );
                jni.ensure_no_exception();
                break;
            }
            case typelib_TypeClass_UNSIGNED_HYPER:
            {
                jvalue args[ 2 ];
                args[ 0 ].j = *static_cast<jlong const *>(pAny->pData);
                JLocalAutoRef jo_val(
                    jni, jni->NewObjectA(
                        getJniInfo()->m_class_Long,
                        getJniInfo()->m_ctor_Long_with_long, args ) );
                jni.ensure_no_exception();
                // box up in com.sun.star.uno.Any
                args[ 0 ].l = getJniInfo()->m_object_Type_UNSIGNED_HYPER;
                args[ 1 ].l = jo_val.get();
                jo_any.reset(
                    jni->NewObjectA(
                        getJniInfo()->m_class_Any,
                        getJniInfo()->m_ctor_Any_with_Type_Object, args ) );
                jni.ensure_no_exception();
                break;
            }
            case typelib_TypeClass_STRING: // opt strings
                jo_any.reset( ustring_to_jstring(
                                  jni, static_cast<rtl_uString *>(pAny->pReserved) ) );
                break;
            case typelib_TypeClass_SEQUENCE:
            {
                jvalue java_data2;
                // prefetch sequence td
                TypeDescr seq_td( pAny->pType );
                map_to_java(
                    jni, &java_data2, pAny->pData, seq_td.get()->pWeakRef, nullptr,
                    true /* in */, false /* no out */,
                    true /* create integral wrappers */ );
                jo_any.reset( java_data2.l );

                // determine inner element type
                ::com::sun::star::uno::Type element_type(
                    reinterpret_cast<typelib_IndirectTypeDescription *>(seq_td.get())->pType );
                while (typelib_TypeClass_SEQUENCE ==
                         element_type.getTypeLibType()->eTypeClass)
                {
                    TypeDescr element_td( element_type.getTypeLibType() );
                    typelib_typedescriptionreference_assign(
                        reinterpret_cast< typelib_TypeDescriptionReference ** >(
                            &element_type ),
                        reinterpret_cast<typelib_IndirectTypeDescription *>(element_td.get())
                          ->pType );
                }
                // box up only if unsigned element type
                switch (element_type.getTypeLibType()->eTypeClass)
                {
                case typelib_TypeClass_UNSIGNED_SHORT:
                case typelib_TypeClass_UNSIGNED_LONG:
                case typelib_TypeClass_UNSIGNED_HYPER:
                {
                    jvalue args[ 2 ];
                    JLocalAutoRef jo_type(
                        jni, create_type( jni, seq_td.get()->pWeakRef ) );
                    args[ 0 ].l = jo_type.get();
                    args[ 1 ].l = jo_any.get();
                    jo_any.reset(
                        jni->NewObjectA(
                            getJniInfo()->m_class_Any,
                            getJniInfo()->m_ctor_Any_with_Type_Object, args ) );
                    jni.ensure_no_exception();
                    break;
                }
                default:
                    break;
                }
                break;
            }
            case typelib_TypeClass_INTERFACE:
            {
                uno_Interface * pUnoI = static_cast<uno_Interface *>(pAny->pReserved);
                if (is_XInterface( pAny->pType ))
                {
                    if (nullptr != pUnoI)
                    {
                        jo_any.reset(
                            map_to_java(
                                jni, pUnoI,
                                getJniInfo()->m_XInterface_type_info ) );
                    }
                    // else: empty XInterface ref maps to null-ref
                }
                else
                {
                    JNI_interface_type_info const * iface_info =
                        static_cast< JNI_interface_type_info const * >(
                            getJniInfo()->get_type_info( jni, pAny->pType ) );
                    if (nullptr != pUnoI)
                    {
                        jo_any.reset( map_to_java( jni, pUnoI, iface_info ) );
                    }
                    // box up in com.sun.star.uno.Any
                    jvalue args[ 2 ];
                    args[ 0 ].l = iface_info->m_type;
                    args[ 1 ].l = jo_any.get();
                    jo_any.reset(
                        jni->NewObjectA(
                            getJniInfo()->m_class_Any,
                            getJniInfo()->m_ctor_Any_with_Type_Object, args ) );
                    jni.ensure_no_exception();
                }
                break;
            }
            case typelib_TypeClass_STRUCT:
            {
                // Do not lose information about type arguments of instantiated
                // polymorphic struct types:
                OUString const & name = OUString::unacquired(
                    &pAny->pType->pTypeName);
                assert(!name.isEmpty());
                if (name[name.getLength() - 1] == '>')
                {
                    // Box up in com.sun.star.uno.Any:
                    JLocalAutoRef jo_type(jni, create_type(jni, pAny->pType));
                    jvalue java_data2;
                    map_to_java(
                        jni, &java_data2, pAny->pData, pAny->pType, nullptr, true,
                        false);
                    jo_any.reset(java_data2.l);
                    jvalue args[2];
                    args[0].l = jo_type.get();
                    args[1].l = jo_any.get();
                    jo_any.reset(
                        jni->NewObjectA(
                            getJniInfo()->m_class_Any,
                            getJniInfo()->m_ctor_Any_with_Type_Object, args));
                    jni.ensure_no_exception();
                    break;
                }
                SAL_FALLTHROUGH;
            }
            default:
            {
                jvalue java_data2;
                map_to_java(
                    jni, &java_data2, pAny->pData, pAny->pType, nullptr,
                    true /* in */, false /* no out */,
                    true /* create integral wrappers */ );
                jo_any.reset( java_data2.l );
                break;
            }
            }
        }

        if (out_param)
        {
            if (nullptr == java_data->l)
            {
                java_data->l = jni->NewObjectArray(
                    1, getJniInfo()->m_class_Object, jo_any.get() );
                jni.ensure_no_exception();
            }
            else
            {
                jni->SetObjectArrayElement(
                    static_cast<jobjectArray>(java_data->l), 0, jo_any.get() );
                jni.ensure_no_exception();
            }
        }
        else
        {
            java_data->l = jo_any.release();
        }
        break;
    }
    case typelib_TypeClass_ENUM:
    {
        OUString const & type_name = OUString::unacquired( &type->pTypeName );
        OString class_name(
            OUStringToOString( type_name, RTL_TEXTENCODING_JAVA_UTF8 ) );
        JLocalAutoRef jo_enum_class(
            jni, find_class( jni, class_name.getStr() ) );

        JLocalAutoRef jo_enum( jni );
        if (in_param)
        {
            // call static <enum_class>.fromInt( int )
            OStringBuffer sig_buf( 5 + class_name.getLength() );
            sig_buf.append( "(I)L" );
            sig_buf.append( class_name.replace( '.', '/' ) );
            sig_buf.append( ';' );
            OString sig( sig_buf.makeStringAndClear() );
            jmethodID method_id = jni->GetStaticMethodID(
                static_cast<jclass>(jo_enum_class.get()), "fromInt", sig.getStr() );
            jni.ensure_no_exception();
            assert( nullptr != method_id );

            jvalue arg;
            arg.i = *static_cast<jint const *>(uno_data);
            jo_enum.reset(
                jni->CallStaticObjectMethodA(
                    static_cast<jclass>(jo_enum_class.get()), method_id, &arg ) );
            jni.ensure_no_exception();
        }
        if (out_param)
        {
            if (nullptr == java_data->l)
            {
                java_data->l = jni->NewObjectArray(
                    1, static_cast<jclass>(jo_enum_class.get()), jo_enum.get() );
                jni.ensure_no_exception();
            }
            else
            {
                jni->SetObjectArrayElement(
                    static_cast<jobjectArray>(java_data->l), 0, jo_enum.get() );
                jni.ensure_no_exception();
            }
        }
        else
        {
            java_data->l = jo_enum.release();
        }
        break;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        if (nullptr == info)
            info = getJniInfo()->get_type_info( jni, type );
        JNI_compound_type_info const * comp_info =
            static_cast< JNI_compound_type_info const * >( info );

        JLocalAutoRef jo_comp( jni );
        if (in_param)
        {
            if (typelib_TypeClass_EXCEPTION == type->eTypeClass)
            {
                JLocalAutoRef jo_message(
                    jni, ustring_to_jstring( jni, *static_cast<rtl_uString * const *>(uno_data) ) );
                jvalue arg;
                arg.l = jo_message.get();
                jo_comp.reset(
                    jni->NewObjectA(
                        comp_info->m_class, comp_info->m_exc_ctor, &arg ) );
                jni.ensure_no_exception();
            }
            else
            {
                jo_comp.reset( jni->AllocObject( comp_info->m_class ) );
                jni.ensure_no_exception();
            }

            for ( JNI_compound_type_info const * linfo = comp_info;
                  nullptr != linfo;
                  linfo = static_cast< JNI_compound_type_info const * >(
                      linfo->m_base ) )
            {
                typelib_CompoundTypeDescription * comp_td =
                    reinterpret_cast<typelib_CompoundTypeDescription *>(linfo->m_td.get());
                typelib_TypeDescriptionReference ** ppMemberTypeRefs =
                    comp_td->ppTypeRefs;
                sal_Int32 * pMemberOffsets = comp_td->pMemberOffsets;
                bool polymorphic
                    = comp_td->aBase.eTypeClass == typelib_TypeClass_STRUCT
                    && reinterpret_cast< typelib_StructTypeDescription * >(
                        comp_td)->pParameterizedTypes != nullptr;
                for ( sal_Int32 nPos = comp_td->nMembers; nPos--; )
                {
                    jfieldID field_id = linfo->m_fields[ nPos ];
                    if (nullptr != field_id)
                    {
                        void const * p =
                            static_cast<char const *>(uno_data) + pMemberOffsets[ nPos ];
                        typelib_TypeDescriptionReference * member_type =
                            ppMemberTypeRefs[ nPos ];
                        bool parameterizedType = polymorphic
                            && (reinterpret_cast<
                                typelib_StructTypeDescription * >(comp_td)->
                                pParameterizedTypes[nPos]);
                        switch (member_type->eTypeClass)
                        {
                        case typelib_TypeClass_CHAR:
                            if (parameterizedType) {
                                jvalue arg;
                                arg.c = *static_cast<jchar const *>(p);
                                JLocalAutoRef jo(
                                    jni,
                                    jni->NewObjectA(
                                        getJniInfo()->m_class_Character,
                                        getJniInfo()->m_ctor_Character_with_char,
                                        &arg ) );
                                jni.ensure_no_exception();
                                jni->SetObjectField(
                                    jo_comp.get(), field_id, jo.get() );
                            } else {
                                jni->SetCharField(
                                    jo_comp.get(),
                                    field_id, *static_cast<jchar const *>(p) );
                            }
                            break;
                        case typelib_TypeClass_BOOLEAN:
                            if (parameterizedType) {
                                jvalue arg;
                                arg.z = *static_cast<jboolean const *>(p);
                                JLocalAutoRef jo(
                                    jni,
                                    jni->NewObjectA(
                                        getJniInfo()->m_class_Boolean,
                                        getJniInfo()->m_ctor_Boolean_with_boolean,
                                        &arg ) );
                                jni.ensure_no_exception();
                                jni->SetObjectField(
                                    jo_comp.get(), field_id, jo.get() );
                            } else {
                                jni->SetBooleanField(
                                    jo_comp.get(),
                                    field_id, *static_cast<jboolean const *>(p) );
                            }
                            break;
                        case typelib_TypeClass_BYTE:
                            if (parameterizedType) {
                                jvalue arg;
                                arg.b = *static_cast<jbyte const *>(p);
                                JLocalAutoRef jo(
                                    jni,
                                    jni->NewObjectA(
                                        getJniInfo()->m_class_Byte,
                                        getJniInfo()->m_ctor_Byte_with_byte,
                                        &arg ) );
                                jni.ensure_no_exception();
                                jni->SetObjectField(
                                    jo_comp.get(), field_id, jo.get() );
                            } else {
                                jni->SetByteField(
                                    jo_comp.get(),
                                    field_id, *static_cast<jbyte const *>(p) );
                            }
                            break;
                        case typelib_TypeClass_SHORT:
                        case typelib_TypeClass_UNSIGNED_SHORT:
                            if (parameterizedType) {
                                jvalue arg;
                                arg.s = *static_cast<jshort const *>(p);
                                JLocalAutoRef jo(
                                    jni,
                                    jni->NewObjectA(
                                        getJniInfo()->m_class_Short,
                                        getJniInfo()->m_ctor_Short_with_short,
                                        &arg ) );
                                jni.ensure_no_exception();
                                jni->SetObjectField(
                                    jo_comp.get(), field_id, jo.get() );
                            } else {
                                jni->SetShortField(
                                    jo_comp.get(),
                                    field_id, *static_cast<jshort const *>(p) );
                            }
                            break;
                        case typelib_TypeClass_LONG:
                        case typelib_TypeClass_UNSIGNED_LONG:
                            if (parameterizedType) {
                                jvalue arg;
                                arg.i = *static_cast<jint const *>(p);
                                JLocalAutoRef jo(
                                    jni,
                                    jni->NewObjectA(
                                        getJniInfo()->m_class_Integer,
                                        getJniInfo()->m_ctor_Integer_with_int,
                                        &arg ) );
                                jni.ensure_no_exception();
                                jni->SetObjectField(
                                    jo_comp.get(), field_id, jo.get() );
                            } else {
                                jni->SetIntField(
                                    jo_comp.get(),
                                    field_id, *static_cast<jint const *>(p) );
                            }
                            break;
                        case typelib_TypeClass_HYPER:
                        case typelib_TypeClass_UNSIGNED_HYPER:
                            if (parameterizedType) {
                                jvalue arg;
                                arg.j = *static_cast<jlong const *>(p);
                                JLocalAutoRef jo(
                                    jni,
                                    jni->NewObjectA(
                                        getJniInfo()->m_class_Long,
                                        getJniInfo()->m_ctor_Long_with_long,
                                        &arg ) );
                                jni.ensure_no_exception();
                                jni->SetObjectField(
                                    jo_comp.get(), field_id, jo.get() );
                            } else {
                                jni->SetLongField(
                                    jo_comp.get(),
                                    field_id, *static_cast<jlong const *>(p) );
                            }
                            break;
                        case typelib_TypeClass_FLOAT:
                            if (parameterizedType) {
                                jvalue arg;
                                arg.f = *static_cast<jfloat const *>(p);
                                JLocalAutoRef jo(
                                    jni,
                                    jni->NewObjectA(
                                        getJniInfo()->m_class_Float,
                                        getJniInfo()->m_ctor_Float_with_float,
                                        &arg ) );
                                jni.ensure_no_exception();
                                jni->SetObjectField(
                                    jo_comp.get(), field_id, jo.get() );
                            } else {
                                jni->SetFloatField(
                                    jo_comp.get(),
                                    field_id, *static_cast<jfloat const *>(p) );
                            }
                            break;
                        case typelib_TypeClass_DOUBLE:
                            if (parameterizedType) {
                                jvalue arg;
                                arg.d = *static_cast<jdouble const *>(p);
                                JLocalAutoRef jo(
                                    jni,
                                    jni->NewObjectA(
                                        getJniInfo()->m_class_Double,
                                        getJniInfo()->m_ctor_Double_with_double,
                                        &arg ) );
                                jni.ensure_no_exception();
                                jni->SetObjectField(
                                    jo_comp.get(), field_id, jo.get() );
                            } else {
                                jni->SetDoubleField(
                                    jo_comp.get(),
                                    field_id, *static_cast<jdouble const *>(p) );
                            }
                            break;
                        case typelib_TypeClass_STRING: // string opt here
                        {
                            JLocalAutoRef jo_string(
                                jni, ustring_to_jstring(
                                    jni, *static_cast<rtl_uString * const *>(p) ) );
                            jni->SetObjectField(
                                jo_comp.get(), field_id, jo_string.get() );
                            break;
                        }
                        default:
                        {
                            jvalue java_data2;
                            map_to_java(
                                jni, &java_data2, p, member_type, nullptr,
                                true /* in */, false /* no out */ );
                            JLocalAutoRef jo_obj( jni, java_data2.l );
                            jni->SetObjectField(
                                jo_comp.get(), field_id, jo_obj.get() );
                            break;
                        }
                        }
                    }
                }
            }
        }
        if (out_param)
        {
            if (nullptr == java_data->l)
            {
                java_data->l =
                    jni->NewObjectArray( 1, comp_info->m_class, jo_comp.get() );
                jni.ensure_no_exception();
            }
            else
            {
                jni->SetObjectArrayElement(
                    static_cast<jobjectArray>(java_data->l), 0, jo_comp.get() );
                jni.ensure_no_exception();
            }
        }
        else
        {
            java_data->l = jo_comp.release();
        }
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        // xxx todo: possible opt for pure out sequences
        JLocalAutoRef jo_ar( jni );

        sal_Int32 nElements;
        uno_Sequence * seq = nullptr;
        if (in_param)
        {
            seq = *static_cast<uno_Sequence * const *>(uno_data);
            nElements = seq->nElements;
        }
        else
        {
            nElements = 0;
        }

        TypeDescr td( type );
        typelib_TypeDescriptionReference * element_type =
            reinterpret_cast<typelib_IndirectTypeDescription *>(td.get())->pType;

        switch (element_type->eTypeClass)
        {
        case typelib_TypeClass_CHAR:
            jo_ar.reset( jni->NewCharArray( nElements ) );
            jni.ensure_no_exception();
            if (0 < nElements)
            {
                jni->SetCharArrayRegion(
                    static_cast<jcharArray>(jo_ar.get()),
                    0, nElements, reinterpret_cast<jchar *>(seq->elements) );
                jni.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_BOOLEAN:
            jo_ar.reset( jni->NewBooleanArray( nElements ) );
            jni.ensure_no_exception();
            if (0 < nElements)
            {
                jni->SetBooleanArrayRegion(
                    static_cast<jbooleanArray>(jo_ar.get()),
                    0, nElements, reinterpret_cast<jboolean *>(seq->elements) );
                jni.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_BYTE:
            jo_ar.reset( jni->NewByteArray( nElements ) );
            jni.ensure_no_exception();
            if (0 < nElements)
            {
                jni->SetByteArrayRegion(
                    static_cast<jbyteArray>(jo_ar.get()),
                    0, nElements, reinterpret_cast<jbyte *>(seq->elements) );
                jni.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            jo_ar.reset( jni->NewShortArray( nElements ) );
            jni.ensure_no_exception();
            if (0 < nElements)
            {
                jni->SetShortArrayRegion(
                    static_cast<jshortArray>(jo_ar.get()),
                    0, nElements, reinterpret_cast<jshort *>(seq->elements) );
                jni.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
            jo_ar.reset( jni->NewIntArray( nElements ) );
            jni.ensure_no_exception();
            if (0 < nElements)
            {
                jni->SetIntArrayRegion(
                    static_cast<jintArray>(jo_ar.get()),
                    0, nElements, reinterpret_cast<jint *>(seq->elements) );
                jni.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            jo_ar.reset( jni->NewLongArray( nElements ) );
            jni.ensure_no_exception();
            if (0 < nElements)
            {
                jni->SetLongArrayRegion(
                    static_cast<jlongArray>(jo_ar.get()),
                    0, nElements, reinterpret_cast<jlong *>(seq->elements) );
                jni.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_FLOAT:
            jo_ar.reset( jni->NewFloatArray( nElements ) );
            jni.ensure_no_exception();
            if (0 < nElements)
            {
                jni->SetFloatArrayRegion(
                    static_cast<jfloatArray>(jo_ar.get()),
                    0, nElements, reinterpret_cast<jfloat *>(seq->elements) );
                jni.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_DOUBLE:
            jo_ar.reset( jni->NewDoubleArray( nElements ) );
            jni.ensure_no_exception();
            if (0 < nElements)
            {
                jni->SetDoubleArrayRegion(
                    static_cast<jdoubleArray>(jo_ar.get()),
                    0, nElements, reinterpret_cast<jdouble *>(seq->elements) );
                jni.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_STRING:
            jo_ar.reset(
                jni->NewObjectArray(
                    nElements, getJniInfo()->m_class_String, nullptr ) );
            jni.ensure_no_exception();
            if (in_param)
            {
                rtl_uString * const * pp =
                    reinterpret_cast<rtl_uString * const *>(seq->elements);
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    JLocalAutoRef jo_string(
                        jni, ustring_to_jstring( jni, pp[ nPos ] ) );
                    jni->SetObjectArrayElement(
                        static_cast<jobjectArray>(jo_ar.get()), nPos, jo_string.get() );
                    jni.ensure_no_exception();
                }
            }
            break;
        case typelib_TypeClass_TYPE:
            jo_ar.reset(
                jni->NewObjectArray( nElements, getJniInfo()->m_class_Type, nullptr ) );
            jni.ensure_no_exception();
            if (in_param)
            {
                typelib_TypeDescriptionReference * const * pp =
                    reinterpret_cast<typelib_TypeDescriptionReference * const *>(seq->elements);
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue val;
                    map_to_java(
                        jni, &val, &pp[ nPos ], element_type, nullptr,
                        true /* in */, false /* no out */ );
                    JLocalAutoRef jo_element( jni, val.l );
                    jni->SetObjectArrayElement(
                        static_cast<jobjectArray>(jo_ar.get()), nPos, jo_element.get() );
                    jni.ensure_no_exception();
                }
            }
            break;
        case typelib_TypeClass_ANY:
            jo_ar.reset(
                jni->NewObjectArray(
                    nElements, getJniInfo()->m_class_Object, nullptr ) );
            jni.ensure_no_exception();
            if (in_param)
            {
                uno_Any const * p = reinterpret_cast<uno_Any const *>(seq->elements);
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue val;
                    map_to_java(
                        jni, &val, &p[ nPos ], element_type, nullptr,
                        true /* in */, false /* no out */ );
                    JLocalAutoRef jo_element( jni, val.l );
                    jni->SetObjectArrayElement(
                        static_cast<jobjectArray>(jo_ar.get()), nPos, jo_element.get() );
                    jni.ensure_no_exception();
                }
            }
            break;
        case typelib_TypeClass_ENUM:
        {
            OUString const & element_type_name =
                OUString::unacquired( &element_type->pTypeName );
            OString class_name(
                OUStringToOString(
                    element_type_name, RTL_TEXTENCODING_JAVA_UTF8 ) );
            JLocalAutoRef jo_enum_class(
                jni, find_class( jni, class_name.getStr() ) );

            jo_ar.reset(
                jni->NewObjectArray(
                    nElements, static_cast<jclass>(jo_enum_class.get()), nullptr ) );
            jni.ensure_no_exception();

            if (0 < nElements)
            {
                // call static <enum_class>.fromInt( int )
                OStringBuffer sig_buf( 5 + class_name.getLength() );
                sig_buf.append( "(I)L" );
                sig_buf.append( class_name.replace( '.', '/' ) );
                sig_buf.append( ';' );
                OString sig( sig_buf.makeStringAndClear() );
                jmethodID method_id = jni->GetStaticMethodID(
                    static_cast<jclass>(jo_enum_class.get()), "fromInt", sig.getStr() );
                jni.ensure_no_exception();
                assert( nullptr != method_id );

                sal_Int32 const * p = reinterpret_cast<sal_Int32 const *>(seq->elements);
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue arg;
                    arg.i = p[ nPos ];
                    JLocalAutoRef jo_enum(
                        jni, jni->CallStaticObjectMethodA(
                            static_cast<jclass>(jo_enum_class.get()), method_id, &arg ) );
                    jni.ensure_no_exception();
                    jni->SetObjectArrayElement(
                        static_cast<jobjectArray>(jo_ar.get()), nPos, jo_enum.get() );
                    jni.ensure_no_exception();
                }
            }
            break;
        }
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        {
            JNI_type_info const * element_info =
                getJniInfo()->get_type_info( jni, element_type );

            jo_ar.reset(
                jni->NewObjectArray( nElements, element_info->m_class, nullptr ) );
            jni.ensure_no_exception();

            if (0 < nElements)
            {
                char * p = const_cast<char *>(seq->elements);
                sal_Int32 nSize = element_info->m_td.get()->nSize;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue val;
                    map_to_java(
                        jni, &val, p + (nSize * nPos),
                        element_type, element_info,
                        true /* in */, false /* no out */ );
                    JLocalAutoRef jo_element( jni, val.l );
                    jni->SetObjectArrayElement(
                        static_cast<jobjectArray>(jo_ar.get()), nPos, jo_element.get() );
                    jni.ensure_no_exception();
                }
            }
            break;
        }
        case typelib_TypeClass_SEQUENCE:
        {
            OStringBuffer buf( 64 );
            JNI_info::append_sig(
                &buf, element_type, false /* use class XInterface */,
                false /* '.' instead of '/' */ );
            OString class_name( buf.makeStringAndClear() );
            JLocalAutoRef jo_seq_class(
                jni, find_class( jni, class_name.getStr() ) );

            jo_ar.reset(
                jni->NewObjectArray(
                    nElements, static_cast<jclass>(jo_seq_class.get()), nullptr ) );
            jni.ensure_no_exception();

            if (0 < nElements)
            {
                TypeDescr element_td( element_type );
                uno_Sequence * const * elements = reinterpret_cast<uno_Sequence * const *>(seq->elements);
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue java_data2;
                    map_to_java(
                        jni, &java_data2, elements + nPos, element_type, nullptr,
                        true /* in */, false /* no out */ );
                    JLocalAutoRef jo_seq( jni, java_data2.l );
                    jni->SetObjectArrayElement(
                        static_cast<jobjectArray>(jo_ar.get()), nPos, jo_seq.get() );
                    jni.ensure_no_exception();
                }
            }
            break;
        }
        case typelib_TypeClass_INTERFACE:
        {
            JNI_interface_type_info const * iface_info =
                static_cast< JNI_interface_type_info const * >(
                    getJniInfo()->get_type_info( jni, element_type ) );

            jo_ar.reset(
                jni->NewObjectArray( nElements, iface_info->m_class, nullptr ) );
            jni.ensure_no_exception();

            if (0 < nElements)
            {
                uno_Interface * const * pp = reinterpret_cast<uno_Interface * const *>(seq->elements);
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    uno_Interface * pUnoI = pp[ nPos ];
                    if (nullptr != pUnoI)
                    {
                        JLocalAutoRef jo_element(
                            jni, map_to_java( jni, pUnoI, iface_info ) );
                        jni->SetObjectArrayElement(
                            static_cast<jobjectArray>(jo_ar.get()),
                            nPos, jo_element.get() );
                        jni.ensure_no_exception();
                    }
                }
            }
            break;
        }
        default:
        {
            throw BridgeRuntimeError(
                "[map_to_java():" + OUString::unacquired( &type->pTypeName )
                + "] unsupported element type: "
                + OUString::unacquired( &element_type->pTypeName )
                + jni.get_stack_trace() );
        }
        }

        if (out_param)
        {
            if (nullptr == java_data->l)
            {
                JLocalAutoRef jo_element_class(
                    jni, jni->GetObjectClass( jo_ar.get() ) );
                if (in_param)
                {
                    java_data->l = jni->NewObjectArray(
                        1, static_cast<jclass>(jo_element_class.get()), jo_ar.get() );
                }
                else
                {
                    java_data->l = jni->NewObjectArray(
                        1, static_cast<jclass>(jo_element_class.get()), nullptr );
                }
                jni.ensure_no_exception();
            }
            else
            {
                jni->SetObjectArrayElement(
                    static_cast<jobjectArray>(java_data->l), 0, jo_ar.get() );
                jni.ensure_no_exception();
            }
        }
        else
        {
            java_data->l = jo_ar.release();
        }
        break;
    }
    case typelib_TypeClass_INTERFACE:
    {
        JLocalAutoRef jo_iface( jni );
        if (in_param)
        {
            uno_Interface * pUnoI = *static_cast<uno_Interface * const *>(uno_data);
            if (nullptr != pUnoI)
            {
                if (nullptr == info)
                    info = getJniInfo()->get_type_info( jni, type );
                JNI_interface_type_info const * iface_info =
                    static_cast< JNI_interface_type_info const * >( info );
                jo_iface.reset( map_to_java( jni, pUnoI, iface_info ) );
            }
        }
        if (out_param)
        {
            if (nullptr == java_data->l)
            {
                if (nullptr == info)
                    info = getJniInfo()->get_type_info( jni, type );
                java_data->l =
                    jni->NewObjectArray( 1, info->m_class, jo_iface.get() );
                jni.ensure_no_exception();
            }
            else
            {
                jni->SetObjectArrayElement(
                    static_cast<jobjectArray>(java_data->l), 0, jo_iface.get() );
                jni.ensure_no_exception();
            }
        }
        else
        {
            java_data->l = jo_iface.release();
        }
        break;
    }
    default:
    {
        throw BridgeRuntimeError(
            "[map_to_java():" + OUString::unacquired( &type->pTypeName )
            + "] unsupported type!" + jni.get_stack_trace() );
    }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
