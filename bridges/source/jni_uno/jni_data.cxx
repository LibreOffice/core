/*************************************************************************
 *
 *  $RCSfile: jni_data.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2002-09-26 14:37:02 $
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

#include "jni_bridge.h"

#include <rtl/strbuf.hxx>
#include <uno/sequence2.h>


using namespace ::std;
using namespace ::rtl;

namespace jni_bridge
{

//--------------------------------------------------------------------------------------------------
inline auto_ptr< rtl_mem > seq_allocate( sal_Int32 nElements, sal_Int32 nSize )
{
    auto_ptr< rtl_mem > seq(
        rtl_mem::allocate( SAL_SEQUENCE_HEADER_SIZE + (nElements * nSize) ) );
    uno_Sequence * p = (uno_Sequence *)seq.get();
    p->nRefCount = 1;
    p->nElements = nElements;
    return seq;
}

//__________________________________________________________________________________________________
void jni_Bridge::map_to_uno(
    JNI_attach const & attach,
    void * uno_data, jvalue java_data,
    typelib_TypeDescriptionReference * type, JNI_type_info const * info /* maybe 0 */,
    bool assign, bool extract_out_param ) const
{
    OSL_ASSERT( JNI_FALSE == sal_False );
    OSL_ASSERT( JNI_TRUE == sal_True );
    OSL_ASSERT( sizeof (jboolean) == sizeof (sal_Bool) );
    OSL_ASSERT( sizeof (jchar) == sizeof (sal_Unicode) );
    OSL_ASSERT( sizeof (jdouble) == sizeof (double) );
    OSL_ASSERT( sizeof (jfloat) == sizeof (float) );
    OSL_ASSERT( sizeof (jbyte) == sizeof (sal_Int8) );
    OSL_ASSERT( sizeof (jshort) == sizeof (sal_Int16) );
    OSL_ASSERT( sizeof (jint) == sizeof (sal_Int32) );
    OSL_ASSERT( sizeof (jlong) == sizeof (sal_Int64) );

    OSL_ASSERT( !extract_out_param || (1 == attach->GetArrayLength( (jarray)java_data.l )) );

    switch (type->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        if (extract_out_param)
        {
            attach->GetCharArrayRegion(
                (jcharArray)java_data.l, 0, 1, (sal_Unicode *)uno_data );
            attach.ensure_no_exception();
        }
        else
        {
            *(sal_Unicode *)uno_data = java_data.c;
        }
        break;
    case typelib_TypeClass_BOOLEAN:
        if (extract_out_param)
        {
            attach->GetBooleanArrayRegion(
                (jbooleanArray)java_data.l, 0, 1, (sal_Bool *)uno_data );
            attach.ensure_no_exception();
        }
        else
        {
            *(sal_Bool *)uno_data = java_data.z;
        }
        break;
    case typelib_TypeClass_BYTE:
        if (extract_out_param)
        {
            attach->GetByteArrayRegion(
                (jbyteArray)java_data.l, 0, 1, (sal_Int8 *)uno_data );
            attach.ensure_no_exception();
        }
        else
        {
            *(sal_Int8 *)uno_data = java_data.b;
        }
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        if (extract_out_param)
        {
            attach->GetShortArrayRegion(
                (jshortArray)java_data.l, 0, 1, (sal_Int16 *)uno_data );
            attach.ensure_no_exception();
        }
        else
        {
            *(sal_Int16 *)uno_data = java_data.s;
        }
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        if (extract_out_param)
        {
            attach->GetIntArrayRegion(
                (jintArray)java_data.l, 0, 1, (sal_Int32 *)uno_data );
            attach.ensure_no_exception();
        }
        else
        {
            *(sal_Int32 *)uno_data = java_data.i;
        }
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        if (extract_out_param)
        {
            attach->GetLongArrayRegion(
                (jlongArray)java_data.l, 0, 1, (sal_Int64 *)uno_data );
            attach.ensure_no_exception();
        }
        else
        {
            *(sal_Int64 *)uno_data = java_data.j;
        }
        break;
    case typelib_TypeClass_FLOAT:
        if (extract_out_param)
        {
            attach->GetFloatArrayRegion(
                (jfloatArray)java_data.l, 0, 1, (float *)uno_data );
            attach.ensure_no_exception();
        }
        else
        {
            *(float *)uno_data = java_data.f;
        }
        break;
    case typelib_TypeClass_DOUBLE:
        if (extract_out_param)
        {
            attach->GetDoubleArrayRegion(
                (jdoubleArray)java_data.l, 0, 1, (double *)uno_data );
            attach.ensure_no_exception();
        }
        else
        {
            *(double *)uno_data = java_data.d;
        }
        break;
    case typelib_TypeClass_STRING:
    {
        JLocalAutoRef jo_out_holder;
        if (extract_out_param)
        {
            jo_out_holder.reset(
                attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ), attach );
            attach.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }
        if (! assign)
            *(rtl_uString **)uno_data = 0;
        jstring_to_ustring(
            (rtl_uString **)uno_data, (jstring)java_data.l, attach );
        break;
    }
    case typelib_TypeClass_TYPE:
    {
        JLocalAutoRef jo_out_holder;
        if (extract_out_param)
        {
            jo_out_holder.reset(
                attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ), attach );
            attach.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }

        // type name
        JLocalAutoRef jo_type_name(
            attach->GetObjectField( java_data.l, m_jni_class_data->m_field_Type__typeName ),
            attach );
        if (! jo_type_name.is())
            throw BridgeRuntimeError( OUSTR("incomplete type object: no type name!") );
        OUString type_name( jstring_to_oustring( (jstring)jo_type_name.get(), attach ) );
        ::com::sun::star::uno::TypeDescription td( type_name );
        if (! td.is())
            throw BridgeRuntimeError( OUSTR("type not found: ") + type_name );
        typelib_typedescriptionreference_acquire( td.get()->pWeakRef );
        if (assign)
        {
            typelib_typedescriptionreference_release(
                *(typelib_TypeDescriptionReference **)uno_data );
        }
        *(typelib_TypeDescriptionReference **)uno_data = td.get()->pWeakRef;

// xxx todo: measure whether it is better to just get the name and
// lookup the td, thus saving the _typeClass field jni call
//         // type class
//         JLocalAutoRef jo_type_class(
//             attach->GetObjectField( java_data.l, m_jni_class_data->m_field_Type__typeClass ),
//             attach );
//         if (! jo_type_class.is())
//             throw BridgeRuntimeError( OUSTR("incomplete type object: no type class!") );
//         typelib_TypeClass type_class = (typelib_TypeClass)attach->GetIntField(
//             jo_type_class.get(), m_jni_class_data->m_field_Enum_m_value );
//         if (! assign)
//             *(typelib_TypeDescriptionReference **)uno_data = 0;
//         typelib_typedescriptionreference_new(
//             (typelib_TypeDescriptionReference **)uno_data, type_class, type_name.pData );
        break;
    }
    case typelib_TypeClass_ANY:
    {
        JLocalAutoRef jo_out_holder;
        if (extract_out_param)
        {
            jo_out_holder.reset(
                attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ), attach );
            attach.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }

        uno_Any * pAny = (uno_Any *)uno_data;
        if (! java_data.l) // null-ref maps to empty any
        {
            if (assign)
                uno_any_destruct( pAny, 0 );
            uno_any_construct( pAny, 0, 0, 0 );
            break;
        }

        // dynamic type
        JLocalAutoRef jo_class(
            attach->CallObjectMethodA( java_data.l, m_jni_class_data->m_method_Object_getClass, 0 ),
            attach );
        attach.ensure_no_exception();
        jvalue arg;
        arg.l = jo_class.get();
        jboolean equals = attach->CallBooleanMethodA(
            m_jni_class_data->m_class_Any, m_jni_class_data->m_method_Object_equals, &arg );
        attach.ensure_no_exception();

        JLocalAutoRef jo_type;
        JLocalAutoRef jo_wrapped;

        if (JNI_FALSE != equals) // is com.sun.star.uno.Any
        {
            // wrapped value type
            jo_type.reset(
                attach->GetObjectField( java_data.l, m_jni_class_data->m_field_Any__type ),
                attach );
            if (! jo_type.is())
                throw BridgeRuntimeError( OUSTR("no type set at com.sun.star.uno.Any!") );
            // wrapped value
            jo_wrapped.reset(
                attach->GetObjectField( java_data.l, m_jni_class_data->m_field_Any__object ),
                attach );
            java_data.l = jo_wrapped.get();
        }
        else
        {
            jo_type = create_type( (jclass)jo_class.get(), attach );
        }

        // get type name
        JLocalAutoRef jo_type_name(
            attach->GetObjectField( jo_type.get(), m_jni_class_data->m_field_Type__typeName ),
            attach );
        attach.ensure_no_exception();
        OUString type_name( jstring_to_oustring( (jstring)jo_type_name.get(), attach ) );

        ::com::sun::star::uno::TypeDescription value_td( type_name );
        if (! value_td.is())
            throw BridgeRuntimeError( OUSTR("type not found: ") + type_name );
        typelib_TypeClass type_class = value_td.get()->eTypeClass;

        // just as fallback: should never happen
        OSL_ENSURE( typelib_TypeClass_ANY != type_class, "forbidden nested any!" );
        if (typelib_TypeClass_ANY == type_class)
        {
            map_to_uno(
                attach, uno_data, java_data, value_td.get()->pWeakRef, 0,
                assign, false /* no out param */ );
            break;
        }

        if (assign)
            uno_any_destruct( pAny, 0 );
        try
        {
            switch (type_class)
            {
            case typelib_TypeClass_VOID:
                pAny->pData = &pAny->pReserved;
                break;
            case typelib_TypeClass_CHAR:
                pAny->pData = &pAny->pReserved;
                *(sal_Unicode *)&pAny->pReserved = attach->CallCharMethodA(
                    java_data.l, m_jni_class_data->m_method_Character_charValue, 0 );
                attach.ensure_no_exception();
                break;
            case typelib_TypeClass_BOOLEAN:
                pAny->pData = &pAny->pReserved;
                *(sal_Bool *)&pAny->pReserved =
                    attach->CallBooleanMethodA(
                        java_data.l, m_jni_class_data->m_method_Boolean_booleanValue, 0 );
                attach.ensure_no_exception();
                break;
            case typelib_TypeClass_BYTE:
                pAny->pData = &pAny->pReserved;
                *(sal_Int8 *)&pAny->pReserved = attach->CallByteMethodA(
                    java_data.l, m_jni_class_data->m_method_Byte_byteValue, 0 );
                attach.ensure_no_exception();
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
                pAny->pData = &pAny->pReserved;
                *(sal_Int16 *)&pAny->pReserved = attach->CallShortMethodA(
                    java_data.l, m_jni_class_data->m_method_Short_shortValue, 0 );
                attach.ensure_no_exception();
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
                pAny->pData = &pAny->pReserved;
                *(sal_Int32 *)&pAny->pReserved = attach->CallIntMethodA(
                    java_data.l, m_jni_class_data->m_method_Integer_intValue, 0 );
                attach.ensure_no_exception();
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                if (sizeof (sal_Int64) <= sizeof (void *))
                {
                    pAny->pData = &pAny->pReserved;
                    *(sal_Int64 *)&pAny->pReserved = attach->CallLongMethodA(
                        java_data.l, m_jni_class_data->m_method_Long_longValue, 0 );
                    attach.ensure_no_exception();
                }
                else
                {
                    auto_ptr< rtl_mem > mem( rtl_mem::allocate( sizeof (sal_Int64) ) );
                    *(sal_Int64 *)mem.get() = attach->CallLongMethodA(
                        java_data.l, m_jni_class_data->m_method_Long_longValue, 0 );
                    attach.ensure_no_exception();
                    pAny->pData = mem.release();
                }
                break;
            case typelib_TypeClass_FLOAT:
            case typelib_TypeClass_DOUBLE:

                break;
            case typelib_TypeClass_STRING: // anies often contain strings; copy string directly
                pAny->pData = &pAny->pReserved;
                pAny->pReserved = 0;
                jstring_to_ustring(
                    (rtl_uString **)&pAny->pReserved, (jstring)java_data.l, attach );
                break;
            case typelib_TypeClass_TYPE:
            case typelib_TypeClass_ENUM:
            case typelib_TypeClass_SEQUENCE:
            case typelib_TypeClass_INTERFACE:
                pAny->pData = &pAny->pReserved;
                pAny->pReserved = 0;
                map_to_uno(
                    attach, &pAny->pReserved, java_data, value_td.get()->pWeakRef, 0,
                    false /* no assign */, false /* no out param */ );
                break;
            case typelib_TypeClass_STRUCT:
            case typelib_TypeClass_EXCEPTION:
            {
                auto_ptr< rtl_mem > mem( rtl_mem::allocate( value_td.get()->nSize ) );
                map_to_uno(
                    attach, mem.get(), java_data, value_td.get()->pWeakRef, 0,
                    false /* no assign */, false /* no out param */ );
                pAny->pData = mem.release();
                break;
            }
            default:
                throw BridgeRuntimeError( OUSTR("unsupported value type of any: ") + type_name );
            }
        }
        catch (...)
        {
            if (assign)
                uno_any_construct( pAny, 0, 0, 0 ); // restore some valid any
            throw;
        }
        typelib_typedescriptionreference_acquire( value_td.get()->pWeakRef );
        pAny->pType = value_td.get()->pWeakRef;
        break;
    }
    case typelib_TypeClass_ENUM:
    {
        JLocalAutoRef jo_out_holder;
        if (extract_out_param)
        {
            jo_out_holder.reset(
                attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ), attach );
            attach.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }

        *(sal_Int32 *)uno_data = attach->GetIntField(
            java_data.l, m_jni_class_data->m_field_Enum_m_value );
        break;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        JLocalAutoRef jo_out_holder;
        if (extract_out_param)
        {
            jo_out_holder.reset(
                attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ), attach );
            attach.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }

        if (0 == info)
        {
            TypeDescr td( type );
            info = m_jni_class_data->get_type_info( td.get(), attach );
        }
        typelib_CompoundTypeDescription * comp_td =
            (typelib_CompoundTypeDescription *)info->m_td;

        sal_Int32 nPos = 0;
        sal_Int32 nMembers = comp_td->nMembers;
        try
        {
            if (0 != comp_td->pBaseTypeDescription)
            {
                map_to_uno(
                    attach, uno_data, java_data,
                    ((typelib_TypeDescription *)comp_td->pBaseTypeDescription)->pWeakRef,
                    info->m_base,
                    assign, false /* no out param */ );
            }

            for ( ; nPos < nMembers; ++nPos )
            {
                void * p = (char *)uno_data + comp_td->pMemberOffsets[ nPos ];
                typelib_TypeDescriptionReference * member_type = comp_td->ppTypeRefs[ nPos ];
                jfieldID field_id = info->m_fields[ nPos ];
                switch (member_type->eTypeClass)
                {
                case typelib_TypeClass_CHAR:
                    *(sal_Unicode *)p = attach->GetCharField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_BOOLEAN:
                    *(sal_Bool *)p = attach->GetBooleanField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_BYTE:
                    *(sal_Int8 *)p = attach->GetByteField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                    *(sal_Int16 *)p = attach->GetShortField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                    *(sal_Int32 *)p = attach->GetIntField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_HYPER:
                case typelib_TypeClass_UNSIGNED_HYPER:
                    *(sal_Int64 *)p = attach->GetLongField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_FLOAT:
                    *(float *)p = attach->GetFloatField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_DOUBLE:
                    *(double *)p = attach->GetDoubleField( java_data.l, field_id );
                    break;
                default:
                {
                    JLocalAutoRef jo_field;
                    if (0 == field_id) // special for Message: call Throwable.getMessage()
                    {
                        OSL_ASSERT(
                            typelib_typedescriptionreference_equals(
                                type, m_jni_class_data->m_Exception_td_ref ) ||
                            typelib_typedescriptionreference_equals(
                                type, m_jni_class_data->m_RuntimeException_td_ref ) );
                        OSL_ASSERT( 0 == nPos ); // first member
                        // call getMessage()
                        jo_field.reset(
                            attach->CallObjectMethodA(
                                java_data.l, m_jni_class_data->m_method_Throwable_getMessage, 0 ),
                            attach );
                    }
                    else
                    {
                        jo_field.reset(
                            attach->GetObjectField( java_data.l, field_id ), attach );
                    }
                    jvalue val;
                    val.l = jo_field.get();
                    map_to_uno( attach, p, val, member_type, 0, assign, false /* no out param */ );
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
                    void * p = (char *)uno_data + comp_td->pMemberOffsets[ nCleanup ];
                    uno_type_destructData(
                        uno_data, comp_td->ppTypeRefs[ nCleanup ], 0 );
                }
                if (0 != comp_td->pBaseTypeDescription)
                {
                    uno_destructData(
                        uno_data, (typelib_TypeDescription *)comp_td->pBaseTypeDescription, 0 );
                }
            }
            throw;
        }
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        JLocalAutoRef jo_out_holder;
        if (extract_out_param)
        {
            jo_out_holder.reset(
                attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ), attach );
            attach.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }

        TypeDescr td( type );
        typelib_TypeDescriptionReference * element_type =
            ((typelib_IndirectTypeDescription *)td.get())->pType;
        auto_ptr< rtl_mem > seq;
        sal_Int32 nElements = attach->GetArrayLength( (jarray)java_data.l );
        switch (element_type->eTypeClass)
        {
        case typelib_TypeClass_CHAR:
            seq = seq_allocate( nElements, sizeof (sal_Unicode) );
            attach->GetCharArrayRegion(
                (jcharArray)java_data.l, 0, nElements,
                (sal_Unicode *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_BOOLEAN:
            seq = seq_allocate( nElements, sizeof (sal_Bool) );
            attach->GetBooleanArrayRegion(
                (jbooleanArray)java_data.l, 0, nElements,
                (sal_Bool *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_BYTE:
            seq = seq_allocate( nElements, sizeof (sal_Int8) );
            attach->GetByteArrayRegion(
                (jbyteArray)java_data.l, 0, nElements,
                (sal_Int8 *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            seq = seq_allocate( nElements, sizeof (sal_Int16) );
            attach->GetShortArrayRegion(
                (jshortArray)java_data.l, 0, nElements,
                (sal_Int16 *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
            seq = seq_allocate( nElements, sizeof (sal_Int32) );
            attach->GetIntArrayRegion(
                (jintArray)java_data.l, 0, nElements,
                (sal_Int32 *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            seq = seq_allocate( nElements, sizeof (sal_Int64) );
            attach->GetLongArrayRegion(
                (jlongArray)java_data.l, 0, nElements,
                (sal_Int64 *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_FLOAT:
            seq = seq_allocate( nElements, sizeof (float) );
            attach->GetFloatArrayRegion(
                (jfloatArray)java_data.l, 0, nElements,
                (float *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_DOUBLE:
            seq = seq_allocate( nElements, sizeof (double) );
            attach->GetDoubleArrayRegion(
                (jdoubleArray)java_data.l, 0, nElements,
                (double *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
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
                element_info = m_jni_class_data->get_type_info( element_td.get(), attach );
            }
            else
            {
                element_info = 0;
            }

            for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
            {
                try
                {
                    JLocalAutoRef jo(
                        attach->GetObjectArrayElement( (jobjectArray)java_data.l, nPos ), attach );
                    attach.ensure_no_exception();
                    jvalue val;
                    val.l = jo.get();
                    void * p =
                        ((uno_Sequence *)seq.get())->elements + (nPos * element_td.get()->nSize);
                    map_to_uno(
                        attach, p, val, element_td.get()->pWeakRef, element_info,
                        false /* no assign */, false /* no out param */ );
                }
                catch (...)
                {
                    // cleanup
                    for ( sal_Int32 nCleanPos = 0; nCleanPos < nPos; ++nCleanPos )
                    {
                        void * p =
                            ((uno_Sequence *)seq.get())->elements +
                            (nCleanPos * element_td.get()->nSize);
                        uno_destructData( p, element_td.get(), 0 );
                    }
                    throw;
                }
            }
            break;
        }
        default:
            throw BridgeRuntimeError( OUSTR("unsupported sequence element type!") );
        }

        if (assign)
            uno_destructData( uno_data, td.get(), 0 );
        *(uno_Sequence **)uno_data = (uno_Sequence *)seq.release();
        break;
    }
    case typelib_TypeClass_INTERFACE:
    {
        JLocalAutoRef jo_out_holder;
        if (extract_out_param)
        {
            jo_out_holder.reset(
                attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ), attach );
            attach.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }

        if (0 == java_data.l) // null-ref
        {
            if (assign)
            {
                uno_Interface * pUnoI = *(uno_Interface **)uno_data;
                if (0 != pUnoI)
                    (*pUnoI->release)( pUnoI );
            }
            *(uno_Interface **)uno_data = 0;
        }
        else
        {
            if (0 == info)
            {
                TypeDescr td( type );
                info = m_jni_class_data->get_type_info( td.get(), attach );
            }
            uno_Interface * pUnoI = map_java2uno( attach, java_data.l, info );
            if (assign)
            {
                uno_Interface * p = *(uno_Interface **)uno_data;
                if (0 != p)
                    (*p->release)( p );
            }
            *(uno_Interface **)uno_data = pUnoI;
        }
        break;
    }
    default:
        throw BridgeRuntimeError(
            OUSTR("unsupported data type: ") +
            *reinterpret_cast< OUString const * >( &type->pTypeName ) );
    }
}

//##################################################################################################
//##################################################################################################
//##################################################################################################

//__________________________________________________________________________________________________
void jni_Bridge::map_to_java(
    JNI_attach const & attach,
    jvalue * java_data, void * uno_data,
    typelib_TypeDescriptionReference * type, JNI_type_info const * info /* maybe 0 */,
    bool in_param, bool out_param, bool special_wrap_integral_types ) const
{
    OSL_ASSERT( JNI_FALSE == sal_False );
    OSL_ASSERT( JNI_TRUE == sal_True );
    OSL_ASSERT( sizeof (jboolean) == sizeof (sal_Bool) );
    OSL_ASSERT( sizeof (jchar) == sizeof (sal_Unicode) );
    OSL_ASSERT( sizeof (jdouble) == sizeof (double) );
    OSL_ASSERT( sizeof (jfloat) == sizeof (float) );
    OSL_ASSERT( sizeof (jbyte) == sizeof (sal_Int8) );
    OSL_ASSERT( sizeof (jshort) == sizeof (sal_Int16) );
    OSL_ASSERT( sizeof (jint) == sizeof (sal_Int32) );
    OSL_ASSERT( sizeof (jlong) == sizeof (sal_Int64) );

    switch (type->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        if (out_param)
        {
            JLocalAutoRef jo_ar( attach->NewCharArray( 1 ), attach );
            attach.ensure_no_exception();
            if (in_param)
            {
                attach->SetCharArrayRegion(
                    (jcharArray)jo_ar.get(), 0, 1, (sal_Unicode *)uno_data );
                attach.ensure_no_exception();
            }
            java_data->l = jo_ar.release();
        }
        else if (special_wrap_integral_types)
        {
            jvalue arg;
            arg.c = *(sal_Unicode const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_class_data->m_class_Character, m_jni_class_data->m_ctor_Character_with_char,
                &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->c = *(sal_Unicode const *)uno_data;
        }
        break;
    case typelib_TypeClass_BOOLEAN:
        if (out_param)
        {
            JLocalAutoRef jo_ar( attach->NewBooleanArray( 1 ), attach );
            attach.ensure_no_exception();
            if (in_param)
            {
                attach->SetBooleanArrayRegion(
                    (jbooleanArray)jo_ar.get(), 0, 1, (sal_Bool *)uno_data );
                attach.ensure_no_exception();
            }
            java_data->l = jo_ar.release();
        }
        else if (special_wrap_integral_types)
        {
            jvalue arg;
            arg.z = *(sal_Bool const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_class_data->m_class_Boolean, m_jni_class_data->m_ctor_Boolean_with_boolean,
                &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->z = (sal_False != *(sal_Bool const *)uno_data);
        }
        break;
    case typelib_TypeClass_BYTE:
        if (out_param)
        {
            JLocalAutoRef jo_ar( attach->NewByteArray( 1 ), attach );
            attach.ensure_no_exception();
            if (in_param)
            {
                attach->SetByteArrayRegion(
                    (jbyteArray)jo_ar.get(), 0, 1, (sal_Int8 *)uno_data );
                attach.ensure_no_exception();
            }
            java_data->l = jo_ar.release();
        }
        else if (special_wrap_integral_types)
        {
            jvalue arg;
            arg.b = *(sal_Int8 const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_class_data->m_class_Byte, m_jni_class_data->m_ctor_Byte_with_byte,
                &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->b = *(sal_Int8 const *)uno_data;
        }
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        if (out_param)
        {
            JLocalAutoRef jo_ar( attach->NewShortArray( 1 ), attach );
            attach.ensure_no_exception();
            if (in_param)
            {
                attach->SetShortArrayRegion(
                    (jshortArray)jo_ar.get(), 0, 1, (sal_Int16 *)uno_data );
                attach.ensure_no_exception();
            }
            java_data->l = jo_ar.release();
        }
        else if (special_wrap_integral_types)
        {
            jvalue arg;
            arg.s = *(sal_Int16 const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_class_data->m_class_Short, m_jni_class_data->m_ctor_Short_with_short,
                &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->s = *(sal_Int16 const *)uno_data;
        }
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        if (out_param)
        {
            JLocalAutoRef jo_ar( attach->NewIntArray( 1 ), attach );
            attach.ensure_no_exception();
            if (in_param)
            {
                attach->SetIntArrayRegion(
                    (jintArray)jo_ar.get(), 0, 1, (sal_Int32 *)uno_data );
                attach.ensure_no_exception();
            }
            java_data->l = jo_ar.release();
        }
        else if (special_wrap_integral_types)
        {
            jvalue arg;
            arg.i = *(sal_Int32 const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_class_data->m_class_Integer, m_jni_class_data->m_ctor_Integer_with_int,
                &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->i = *(sal_Int32 const *)uno_data;
        }
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        if (out_param)
        {
            JLocalAutoRef jo_ar( attach->NewLongArray( 1 ), attach );
            attach.ensure_no_exception();
            if (in_param)
            {
                attach->SetLongArrayRegion(
                    (jlongArray)jo_ar.get(), 0, 1, (sal_Int64 *)uno_data );
                attach.ensure_no_exception();
            }
            java_data->l = jo_ar.release();
        }
        else if (special_wrap_integral_types)
        {
            jvalue arg;
            arg.j = *(sal_Int64 const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_class_data->m_class_Long, m_jni_class_data->m_ctor_Long_with_long,
                &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->j = *(sal_Int64 const *)uno_data;
        }
        break;
    case typelib_TypeClass_FLOAT:
        if (out_param)
        {
            JLocalAutoRef jo_ar( attach->NewFloatArray( 1 ), attach );
            attach.ensure_no_exception();
            if (in_param)
            {
                attach->SetFloatArrayRegion(
                    (jfloatArray)jo_ar.get(), 0, 1, (float *)uno_data );
                attach.ensure_no_exception();
            }
            java_data->l = jo_ar.release();
        }
        else if (special_wrap_integral_types)
        {
            jvalue arg;
            arg.f = *(float const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_class_data->m_class_Float, m_jni_class_data->m_ctor_Float_with_float,
                &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->f = *(float const *)uno_data;
        }
        break;
    case typelib_TypeClass_DOUBLE:
        if (out_param)
        {
            JLocalAutoRef jo_ar( attach->NewDoubleArray( 1 ), attach );
            attach.ensure_no_exception();
            if (in_param)
            {
                attach->SetDoubleArrayRegion(
                    (jdoubleArray)jo_ar.get(), 0, 1, (double *)uno_data );
                attach.ensure_no_exception();
            }
            java_data->l = jo_ar.release();
        }
        else if (special_wrap_integral_types)
        {
            jvalue arg;
            arg.d = *(double const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_class_data->m_class_Double, m_jni_class_data->m_ctor_Double_with_double,
                &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->d = *(double const *)uno_data;
        }
        break;
    case typelib_TypeClass_STRING:
    {
        if (in_param)
        {
            java_data->l = ustring_to_jstring( *(rtl_uString * const *)uno_data, attach );
            if (out_param)
            {
                JLocalAutoRef jo_str( java_data->l, attach );
                java_data->l = attach->NewObjectArray(
                    1, m_jni_class_data->m_class_String, jo_str.get() );
                attach.ensure_no_exception();
            }
        }
        else // pure out
        {
            java_data->l = attach->NewObjectArray(
                1, m_jni_class_data->m_class_String, 0 );
            attach.ensure_no_exception();
        }
        break;
    }
    case typelib_TypeClass_TYPE:
    {
        JLocalAutoRef jo_type;
        if (in_param)
        {
            typelib_TypeDescriptionReference * type =
                *(typelib_TypeDescriptionReference **)uno_data;
            jo_type = create_type( type, attach );
        }
        if (out_param)
        {
            java_data->l = attach->NewObjectArray(
                1, m_jni_class_data->m_class_Type, jo_type.get() );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->l = jo_type.release();
        }
        break;
    }
    case typelib_TypeClass_ANY:
    {
        JLocalAutoRef jo_any;
        if (in_param)
        {
            uno_Any const * pAny = (uno_Any const *)uno_data;
            jvalue args[ 2 ];
            // map value
            map_to_java(
                attach,
                args, pAny->pData, pAny->pType, 0,
                true /* in data */, false /* no out array */, true /* create integral wrappers */ );
            JLocalAutoRef jo_val( args[ 0 ].l, attach );
            JLocalAutoRef jo_type( create_type( pAny->pType, attach ) );
            args[ 0 ].l = jo_type.get();
            args[ 1 ].l = jo_val.get();
            // build up any
            jo_any.reset(
                attach->NewObjectA(
                    m_jni_class_data->m_class_Any, m_jni_class_data->m_ctor_Any_with_Type_Object,
                    args ),
                attach );
            attach.ensure_no_exception();
        }
        if (out_param)
        {
            java_data->l = attach->NewObjectArray(
                1, m_jni_class_data->m_class_Object, jo_any.get() );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->l = jo_any.release();
        }
        break;
    }
    case typelib_TypeClass_ENUM:
    {
        OUString const & type_name = *reinterpret_cast< OUString const * >( &type->pTypeName );
        OString class_name(
            OUStringToOString( type_name.replace( '.', '/' ), RTL_TEXTENCODING_ASCII_US ) );
        JLocalAutoRef jo_enum_class( find_class( class_name.getStr(), attach ) );

        JLocalAutoRef jo_enum;
        if (in_param)
        {
            // call static <enum_class>.fromInt( int )
            OStringBuffer sig_buf( 3 + class_name.getLength() );
            sig_buf.append( RTL_CONSTASCII_STRINGPARAM("(I)") );
            sig_buf.append( class_name );
            OString sig( sig_buf.makeStringAndClear() );
            jmethodID method_id = attach->GetStaticMethodID(
                (jclass)jo_enum_class.get(), "fromInt", sig.getStr() );
            attach.ensure_no_exception();
            OSL_ASSERT( 0 != method_id );

            jvalue arg;
            arg.i = *(sal_Int32 const *)uno_data;
            jo_enum.reset(
                attach->CallStaticObjectMethodA(
                    (jclass)jo_enum_class.get(), method_id, &arg ),
                attach );
            attach.ensure_no_exception();
        }
        if (out_param)
        {
            java_data->l = attach->NewObjectArray( 1, (jclass)jo_enum_class.get(), jo_enum.get() );
            attach.ensure_no_exception();
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
        if (0 == info)
        {
            TypeDescr td( type );
            info = m_jni_class_data->get_type_info( td.get(), attach );
        }

        // xxx todo
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        JLocalAutoRef jo_ar;

        sal_Int32 nElements;
        uno_Sequence * seq = 0;
        if (in_param)
        {
            seq = *(uno_Sequence **)uno_data;
            nElements = seq->nElements;
        }
        else
        {
            nElements = 0;
        }

        TypeDescr td( type );
        typelib_TypeDescriptionReference * element_type =
            ((typelib_IndirectTypeDescription *)td.get())->pType;

        switch (element_type->eTypeClass)
        {
        case typelib_TypeClass_CHAR:
            jo_ar.reset( attach->NewCharArray( nElements ), attach );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetCharArrayRegion(
                    (jcharArray)jo_ar.get(), 0, nElements, (sal_Unicode *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_BOOLEAN:
            jo_ar.reset( attach->NewBooleanArray( nElements ), attach );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetBooleanArrayRegion(
                    (jbooleanArray)jo_ar.get(), 0, nElements, (sal_Bool *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_BYTE:
            jo_ar.reset( attach->NewByteArray( nElements ), attach );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetByteArrayRegion(
                    (jbyteArray)jo_ar.get(), 0, nElements, (sal_Int8 *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            jo_ar.reset( attach->NewShortArray( nElements ), attach );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetShortArrayRegion(
                    (jshortArray)jo_ar.get(), 0, nElements, (sal_Int16 *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
            jo_ar.reset( attach->NewIntArray( nElements ), attach );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetIntArrayRegion(
                    (jintArray)jo_ar.get(), 0, nElements, (sal_Int32 *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            jo_ar.reset( attach->NewLongArray( nElements ), attach );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetLongArrayRegion(
                    (jlongArray)jo_ar.get(), 0, nElements, (sal_Int64 *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_FLOAT:
            jo_ar.reset( attach->NewFloatArray( nElements ), attach );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetFloatArrayRegion(
                    (jfloatArray)jo_ar.get(), 0, nElements, (float *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_DOUBLE:
            jo_ar.reset( attach->NewDoubleArray( nElements ), attach );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetDoubleArrayRegion(
                    (jdoubleArray)jo_ar.get(), 0, nElements, (double *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_STRING:
            jo_ar.reset(
                attach->NewObjectArray( nElements, m_jni_class_data->m_class_String, 0 ),
                attach );
            attach.ensure_no_exception();
            if (in_param)
            {
                rtl_uString ** pp = (rtl_uString **)seq->elements;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    JLocalAutoRef jo_string( ustring_to_jstring( pp[ nPos ], attach ), attach );
                    attach->SetObjectArrayElement(
                        (jobjectArray)jo_ar.get(), nPos, jo_string.get() );
                    attach.ensure_no_exception();
                }
            }
            break;
        case typelib_TypeClass_TYPE:
            jo_ar.reset(
                attach->NewObjectArray( nElements, m_jni_class_data->m_class_Type, 0 ),
                attach );
            attach.ensure_no_exception();
            if (in_param)
            {
                typelib_TypeDescriptionReference ** pp =
                    (typelib_TypeDescriptionReference **)seq->elements;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue val;
                    map_to_java(
                        attach, &val, &pp[ nPos ], element_type, 0,
                        true /* in */, false /* no out */ );
                    JLocalAutoRef jo_element( val.l, attach );
                    attach->SetObjectArrayElement(
                        (jobjectArray)jo_ar.get(), nPos, jo_element.get() );
                    attach.ensure_no_exception();
                }
            }
            break;
        case typelib_TypeClass_ANY:
            jo_ar.reset(
                attach->NewObjectArray( nElements, m_jni_class_data->m_class_Object, 0 ),
                attach );
            attach.ensure_no_exception();
            if (in_param)
            {
                uno_Any * p = (uno_Any *)seq->elements;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue val;
                    map_to_java(
                        attach, &val, &p[ nPos ], element_type, 0,
                        true /* in */, false /* no out */ );
                    JLocalAutoRef jo_element( val.l, attach );
                    attach->SetObjectArrayElement(
                        (jobjectArray)jo_ar.get(), nPos, jo_element.get() );
                    attach.ensure_no_exception();
                }
            }
            break;
        case typelib_TypeClass_ENUM:
        {
            OUString const & element_type_name =
                *reinterpret_cast< OUString const * >( &element_type->pTypeName );
            OString class_name(
                OUStringToOString(
                    element_type_name.replace( '.', '/' ),
                    RTL_TEXTENCODING_ASCII_US ) );
            JLocalAutoRef jo_enum_class( find_class( class_name.getStr(), attach ) );

            jo_ar.reset(
                attach->NewObjectArray( nElements, (jclass)jo_enum_class.get(), 0 ),
                attach );
            attach.ensure_no_exception();

            if (0 < nElements)
            {
                // static <enum_class>.fromInt( int )
                OStringBuffer sig_buf( 3 + class_name.getLength() );
                sig_buf.append( RTL_CONSTASCII_STRINGPARAM("(I)") );
                sig_buf.append( class_name );
                OString sig( sig_buf.makeStringAndClear() );
                jmethodID method_id = attach->GetStaticMethodID(
                    (jclass)jo_enum_class.get(), "fromInt", sig.getStr() );
                attach.ensure_no_exception();
                OSL_ASSERT( 0 != method_id );

                sal_Int32 * p = (sal_Int32 *)seq->elements;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue arg;
                    arg.i = p[ nPos ];
                    JLocalAutoRef jo_enum(
                        attach->CallStaticObjectMethodA(
                            (jclass)jo_enum_class.get(), method_id, &arg ),
                        attach );
                    attach.ensure_no_exception();
                    attach->SetObjectArrayElement(
                        (jobjectArray)jo_ar.get(), nPos, jo_enum.get() );
                    attach.ensure_no_exception();
                }
            }
            break;
        }
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        {
            TypeDescr element_td( element_type );
            JNI_type_info const * element_info =
                m_jni_class_data->get_type_info( element_td.get(), attach );

            jo_ar.reset( attach->NewObjectArray( nElements, element_info->m_class, 0 ), attach );
            attach.ensure_no_exception();

            if (0 < nElements)
            {
                char * p = (char *)seq->elements;
                sal_Int32 nSize = element_td.get()->nSize;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue val;
                    map_to_java(
                        attach, &val, p + (nSize * nPos), element_type, element_info,
                        true /* in */, false /* no out */ );
                    JLocalAutoRef jo_element( val.l, attach );
                    attach->SetObjectArrayElement(
                        (jobjectArray)jo_ar.get(), nPos, jo_element.get() );
                    attach.ensure_no_exception();
                }
            }
            break;
        }
        case typelib_TypeClass_SEQUENCE:
        {
            // xxx todo
            break;
        }
        case typelib_TypeClass_INTERFACE:
        {
            TypeDescr element_td( element_type );
            JNI_type_info const * element_info =
                m_jni_class_data->get_type_info( element_td.get(), attach );

            jo_ar.reset( attach->NewObjectArray( nElements, element_info->m_class, 0 ), attach );
            attach.ensure_no_exception();

            if (0 < nElements)
            {
                char * p = (char *)seq->elements;
                sal_Int32 nSize = element_td.get()->nSize;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue val;
                    map_to_java(
                        attach, &val, p + (nSize * nPos), element_type, element_info,
                        true /* in */, false /* no out */ );
                    JLocalAutoRef jo_element( val.l, attach );
                    attach->SetObjectArrayElement(
                        (jobjectArray)jo_ar.get(), nPos, jo_element.get() );
                    attach.ensure_no_exception();
                }
            }
            break;
        }
        default:
            throw BridgeRuntimeError(
                OUSTR("unsupported element data type: ") +
                *reinterpret_cast< OUString const * >( &element_type->pTypeName ) );
        }

        if (out_param)
        {
            JLocalAutoRef jo_element_class( jo_ar.get_class() );
            if (in_param)
            {
                java_data->l =
                    attach->NewObjectArray( 1, (jclass)jo_element_class.get(), jo_ar.get() );
            }
            else
            {
                java_data->l =
                    attach->NewObjectArray( 1, (jclass)jo_element_class.get(), 0 );
            }
            attach.ensure_no_exception();
        }
        else
        {
            java_data->l = jo_ar.release();
        }
        break;
    }
    case typelib_TypeClass_INTERFACE:
    {
        JLocalAutoRef jo_iface;
        if (in_param)
        {
            uno_Interface * pUnoI = *(uno_Interface * const *)uno_data;
            if (0 != pUnoI)
            {
                if (0 == info)
                {
                    TypeDescr td( type );
                    info = m_jni_class_data->get_type_info( td.get(), attach );
                }
                jo_iface.reset( map_uno2java( attach, pUnoI, info ), attach );
            }
        }
        if (out_param)
        {
            if (typelib_typedescriptionreference_equals(
                    type, m_jni_class_data->m_XInterface_td_ref ))
            {
                java_data->l = attach->NewObjectArray(
                    1, m_jni_class_data->m_class_Object, jo_iface.get() );
                attach.ensure_no_exception();
            }
            else
            {
                OUString const & type_name =
                    *reinterpret_cast< OUString const * >( &type->pTypeName );
                OString class_name(
                    OUStringToOString( type_name.replace( '.', '/' ), RTL_TEXTENCODING_ASCII_US ) );
                JLocalAutoRef jo_iface_class( find_class( class_name.getStr(), attach ) );

                java_data->l = attach->NewObjectArray(
                    1, (jclass)jo_iface_class.get(), jo_iface.get() );
                attach.ensure_no_exception();
            }
        }
        else
        {
            java_data->l = jo_iface.release();
        }
        break;
    }
    default:
        throw BridgeRuntimeError(
            OUSTR("unsupported data type: ") +
            *reinterpret_cast< OUString const * >( &type->pTypeName ) );
    }
}

}
