/*************************************************************************
 *
 *  $RCSfile: jni_data.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: dbo $ $Date: 2002-11-22 11:54:00 $
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
    bool assign, bool out_param,
    bool special_wrapped_integral_types ) const
{
    OSL_ASSERT( !out_param || (1 == attach->GetArrayLength( (jarray)java_data.l )) );

    switch (type->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        if (out_param)
        {
            attach->GetCharArrayRegion(
                (jcharArray)java_data.l, 0, 1, (jchar *)uno_data );
            attach.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *(jchar *)uno_data = attach->CallCharMethodA(
                java_data.l, m_jni_info->m_method_Character_charValue, 0 );
            attach.ensure_no_exception();
        }
        else
        {
            *(jchar *)uno_data = java_data.c;
        }
        break;
    case typelib_TypeClass_BOOLEAN:
        if (out_param)
        {
            attach->GetBooleanArrayRegion(
                (jbooleanArray)java_data.l, 0, 1, (jboolean *)uno_data );
            attach.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *(jboolean *)uno_data = attach->CallBooleanMethodA(
                java_data.l, m_jni_info->m_method_Boolean_booleanValue, 0 );
            attach.ensure_no_exception();
        }
        else
        {
            *(jboolean *)uno_data = java_data.z;
        }
        break;
    case typelib_TypeClass_BYTE:
        if (out_param)
        {
            attach->GetByteArrayRegion(
                (jbyteArray)java_data.l, 0, 1, (jbyte *)uno_data );
            attach.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *(jbyte *)uno_data = attach->CallByteMethodA(
                java_data.l, m_jni_info->m_method_Byte_byteValue, 0 );
            attach.ensure_no_exception();
        }
        else
        {
            *(jbyte *)uno_data = java_data.b;
        }
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        if (out_param)
        {
            attach->GetShortArrayRegion(
                (jshortArray)java_data.l, 0, 1, (jshort *)uno_data );
            attach.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *(jshort *)uno_data = attach->CallShortMethodA(
                java_data.l, m_jni_info->m_method_Short_shortValue, 0 );
            attach.ensure_no_exception();
        }
        else
        {
            *(jshort *)uno_data = java_data.s;
        }
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        if (out_param)
        {
            attach->GetIntArrayRegion(
                (jintArray)java_data.l, 0, 1, (jint *)uno_data );
            attach.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *(jint *)uno_data = attach->CallIntMethodA(
                java_data.l, m_jni_info->m_method_Integer_intValue, 0 );
            attach.ensure_no_exception();
        }
        else
        {
            *(jint *)uno_data = java_data.i;
        }
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        if (out_param)
        {
            attach->GetLongArrayRegion(
                (jlongArray)java_data.l, 0, 1, (jlong *)uno_data );
            attach.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *(jlong *)uno_data = attach->CallLongMethodA(
                java_data.l, m_jni_info->m_method_Long_longValue, 0 );
            attach.ensure_no_exception();
        }
        else
        {
            *(jlong *)uno_data = java_data.j;
        }
        break;
    case typelib_TypeClass_FLOAT:
        if (out_param)
        {
            attach->GetFloatArrayRegion(
                (jfloatArray)java_data.l, 0, 1, (jfloat *)uno_data );
            attach.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *(jfloat *)uno_data = attach->CallFloatMethodA(
                java_data.l, m_jni_info->m_method_Float_floatValue, 0 );
            attach.ensure_no_exception();
        }
        else
        {
            *(jfloat *)uno_data = java_data.f;
        }
        break;
    case typelib_TypeClass_DOUBLE:
        if (out_param)
        {
            attach->GetDoubleArrayRegion(
                (jdoubleArray)java_data.l, 0, 1, (jdouble *)uno_data );
            attach.ensure_no_exception();
        }
        else if (special_wrapped_integral_types)
        {
            *(jdouble *)uno_data = attach->CallDoubleMethodA(
                java_data.l, m_jni_info->m_method_Double_doubleValue, 0 );
            attach.ensure_no_exception();
        }
        else
        {
            *(jdouble *)uno_data = java_data.d;
        }
        break;
    case typelib_TypeClass_STRING:
    {
        JLocalAutoRef jo_out_holder;
        if (out_param)
        {
            jo_out_holder.reset(
                attach, attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ) );
            attach.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }
        if (! assign)
            *(rtl_uString **)uno_data = 0;
        jstring_to_ustring(
            attach, (rtl_uString **)uno_data, (jstring)java_data.l );
        break;
    }
    case typelib_TypeClass_TYPE:
    {
        JLocalAutoRef jo_out_holder;
        if (out_param)
        {
            jo_out_holder.reset(
                attach, attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ) );
            attach.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }

        // type name
        JLocalAutoRef jo_type_name(
            attach, attach->GetObjectField( java_data.l, m_jni_info->m_field_Type__typeName ) );
        if (! jo_type_name.is())
        {
            throw BridgeRuntimeError(
                OUSTR("[map_to_uno():typelib_TypeClass_TYPE] "
                      "incomplete type object: no type name!") );
        }
        OUString type_name( jstring_to_oustring( attach, (jstring)jo_type_name.get() ) );
        ::com::sun::star::uno::TypeDescription td( type_name );
        if (! td.is())
        {
            throw BridgeRuntimeError(
                OUSTR("[map_to_uno():typelib_TypeClass_TYPE] "
                      "UNO type not found: ") + type_name );
        }
        typelib_typedescriptionreference_acquire( td.get()->pWeakRef );
        if (assign)
        {
            typelib_typedescriptionreference_release(
                *(typelib_TypeDescriptionReference **)uno_data );
        }
        *(typelib_TypeDescriptionReference **)uno_data = td.get()->pWeakRef;
        break;
    }
    case typelib_TypeClass_ANY: // xxx todo: possible opt on anys
    {
        JLocalAutoRef jo_out_holder;
        if (out_param)
        {
            jo_out_holder.reset(
                attach, attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ) );
            attach.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }

        uno_Any * pAny = (uno_Any *)uno_data;
        if (0 == java_data.l) // null-ref maps to empty any
        {
            if (assign)
                uno_any_destruct( pAny, 0 );
            uno_any_construct( pAny, 0, 0, 0 );
            break;
        }

        // dynamic type
        JLocalAutoRef jo_class(
            attach, attach->CallObjectMethodA(
                java_data.l, m_jni_info->m_method_Object_getClass, 0 ) );
        attach.ensure_no_exception();
        jvalue arg;
        arg.l = jo_class.get();
        bool is_any =
            (JNI_FALSE != attach->CallBooleanMethodA(
                m_jni_info->m_class_Any, m_jni_info->m_method_Object_equals, &arg ));
        attach.ensure_no_exception();

        JLocalAutoRef jo_type;
        JLocalAutoRef jo_wrapped_holder;

        if (is_any)
        {
            // wrapped value type
            jo_type.reset(
                attach, attach->GetObjectField( java_data.l, m_jni_info->m_field_Any__type ) );
            if (! jo_type.is())
            {
                throw BridgeRuntimeError(
                    OUSTR("[map_to_uno():typelib_TypeClass_ANY] "
                          "no type set at com.sun.star.uno.Any!") );
            }
            // wrapped value
            jo_wrapped_holder.reset(
                attach, attach->GetObjectField( java_data.l, m_jni_info->m_field_Any__object ) );
            java_data.l = jo_wrapped_holder.get();
        }
        else
        {
            // check whether this is a proxy
            if (JNI_FALSE != attach->IsAssignableFrom(
                    (jclass)jo_class.get(), m_jni_info->m_class_TypedProxy ))
            {
                // is proxy => determine exact type
                jo_type.reset(
                    attach, attach->CallObjectMethodA(
                        java_data.l, m_jni_info->m_method_TypedProxy_getType, 0 ) );
                attach.ensure_no_exception();
            }
            else
            {
                jo_type.reset(
                    attach, create_type( attach, (jclass)jo_class.get() ) );
            }
        }

        // get type name
        JLocalAutoRef jo_type_name(
            attach, attach->GetObjectField( jo_type.get(), m_jni_info->m_field_Type__typeName ) );
        attach.ensure_no_exception();
        OUString type_name( jstring_to_oustring( attach, (jstring)jo_type_name.get() ) );

        ::com::sun::star::uno::TypeDescription value_td( type_name );
        if (! value_td.is())
        {
            throw BridgeRuntimeError(
                OUSTR("[map_to_uno():typelib_TypeClass_ANY] "
                      "UNO type not found: ") + type_name );
        }
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
                *(jchar *)&pAny->pReserved = attach->CallCharMethodA(
                    java_data.l, m_jni_info->m_method_Character_charValue, 0 );
                attach.ensure_no_exception();
                break;
            case typelib_TypeClass_BOOLEAN:
                pAny->pData = &pAny->pReserved;
                *(jboolean *)&pAny->pReserved = attach->CallBooleanMethodA(
                    java_data.l, m_jni_info->m_method_Boolean_booleanValue, 0 );
                attach.ensure_no_exception();
                break;
            case typelib_TypeClass_BYTE:
                pAny->pData = &pAny->pReserved;
                *(jbyte *)&pAny->pReserved = attach->CallByteMethodA(
                    java_data.l, m_jni_info->m_method_Byte_byteValue, 0 );
                attach.ensure_no_exception();
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
                pAny->pData = &pAny->pReserved;
                *(jshort *)&pAny->pReserved = attach->CallShortMethodA(
                    java_data.l, m_jni_info->m_method_Short_shortValue, 0 );
                attach.ensure_no_exception();
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
                pAny->pData = &pAny->pReserved;
                *(jint *)&pAny->pReserved = attach->CallIntMethodA(
                    java_data.l, m_jni_info->m_method_Integer_intValue, 0 );
                attach.ensure_no_exception();
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                if (sizeof (sal_Int64) <= sizeof (void *))
                {
                    pAny->pData = &pAny->pReserved;
                    *(jlong *)&pAny->pReserved = attach->CallLongMethodA(
                        java_data.l, m_jni_info->m_method_Long_longValue, 0 );
                    attach.ensure_no_exception();
                }
                else
                {
                    auto_ptr< rtl_mem > mem( rtl_mem::allocate( sizeof (sal_Int64) ) );
                    *(jlong *)mem.get() = attach->CallLongMethodA(
                        java_data.l, m_jni_info->m_method_Long_longValue, 0 );
                    attach.ensure_no_exception();
                    pAny->pData = mem.release();
                }
                break;
            case typelib_TypeClass_FLOAT:
                if (sizeof (float) <= sizeof (void *))
                {
                    pAny->pData = &pAny->pReserved;
                    *(jfloat *)&pAny->pReserved = attach->CallFloatMethodA(
                        java_data.l, m_jni_info->m_method_Float_floatValue, 0 );
                    attach.ensure_no_exception();
                }
                else
                {
                    auto_ptr< rtl_mem > mem( rtl_mem::allocate( sizeof (float) ) );
                    *(jfloat *)mem.get() = attach->CallFloatMethodA(
                        java_data.l, m_jni_info->m_method_Float_floatValue, 0 );
                    attach.ensure_no_exception();
                    pAny->pData = mem.release();
                }
                break;
            case typelib_TypeClass_DOUBLE:
                if (sizeof (double) <= sizeof (void *))
                {
                    pAny->pData = &pAny->pReserved;
                    *(jdouble *)&pAny->pReserved = attach->CallDoubleMethodA(
                        java_data.l, m_jni_info->m_method_Double_doubleValue, 0 );
                    attach.ensure_no_exception();
                }
                else
                {
                    auto_ptr< rtl_mem > mem( rtl_mem::allocate( sizeof (double) ) );
                    *(jdouble *)mem.get() = attach->CallDoubleMethodA(
                        java_data.l, m_jni_info->m_method_Double_doubleValue, 0 );
                    attach.ensure_no_exception();
                    pAny->pData = mem.release();
                }
                break;
            case typelib_TypeClass_STRING: // anies often contain strings; copy string directly
                pAny->pData = &pAny->pReserved;
                pAny->pReserved = 0;
                jstring_to_ustring(
                    attach, (rtl_uString **)&pAny->pReserved, (jstring)java_data.l );
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
                throw BridgeRuntimeError(
                    OUSTR("[map_to_uno():typelib_TypeClass_ANY] "
                          "unsupported value type of any: ") + type_name );
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
        if (out_param)
        {
            jo_out_holder.reset(
                attach, attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ) );
            attach.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }

        *(jint *)uno_data = attach->GetIntField(
            java_data.l, m_jni_info->m_field_Enum_m_value );
        break;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        JLocalAutoRef jo_out_holder;
        if (out_param)
        {
            jo_out_holder.reset(
                attach, attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ) );
            attach.ensure_no_exception();
            java_data.l = jo_out_holder.get();
        }

        if (0 == info)
        {
            TypeDescr td( type );
            info = m_jni_info->get_type_info( attach, td.get() );
        }
        typelib_CompoundTypeDescription * comp_td =
            (typelib_CompoundTypeDescription *)info->m_td.get();

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
                    *(jchar *)p = attach->GetCharField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_BOOLEAN:
                    *(jboolean *)p = attach->GetBooleanField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_BYTE:
                    *(jbyte *)p = attach->GetByteField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                    *(jshort *)p = attach->GetShortField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                    *(jint *)p = attach->GetIntField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_HYPER:
                case typelib_TypeClass_UNSIGNED_HYPER:
                    *(jlong *)p = attach->GetLongField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_FLOAT:
                    *(jfloat *)p = attach->GetFloatField( java_data.l, field_id );
                    break;
                case typelib_TypeClass_DOUBLE:
                    *(jdouble *)p = attach->GetDoubleField( java_data.l, field_id );
                    break;
                default:
                {
                    JLocalAutoRef jo_field;
                    if (0 == field_id) // special for Message: call Throwable.getMessage()
                    {
                        OSL_ASSERT(
                            typelib_typedescriptionreference_equals(
                                type, m_jni_info->m_Exception_type.getTypeLibType() ) ||
                            typelib_typedescriptionreference_equals(
                                type, m_jni_info->m_RuntimeException_type.getTypeLibType() ) );
                        OSL_ASSERT( 0 == nPos ); // first member
                        // call getMessage()
                        jo_field.reset(
                            attach, attach->CallObjectMethodA(
                                java_data.l, m_jni_info->m_method_Throwable_getMessage, 0 ) );
                    }
                    else
                    {
                        jo_field.reset(
                            attach, attach->GetObjectField( java_data.l, field_id ) );
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
        if (out_param)
        {
            jo_out_holder.reset(
                attach, attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ) );
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
                (jchar *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_BOOLEAN:
            seq = seq_allocate( nElements, sizeof (sal_Bool) );
            attach->GetBooleanArrayRegion(
                (jbooleanArray)java_data.l, 0, nElements,
                (jboolean *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_BYTE:
            seq = seq_allocate( nElements, sizeof (sal_Int8) );
            attach->GetByteArrayRegion(
                (jbyteArray)java_data.l, 0, nElements,
                (jbyte *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            seq = seq_allocate( nElements, sizeof (sal_Int16) );
            attach->GetShortArrayRegion(
                (jshortArray)java_data.l, 0, nElements,
                (jshort *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
            seq = seq_allocate( nElements, sizeof (sal_Int32) );
            attach->GetIntArrayRegion(
                (jintArray)java_data.l, 0, nElements,
                (jint *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            seq = seq_allocate( nElements, sizeof (sal_Int64) );
            attach->GetLongArrayRegion(
                (jlongArray)java_data.l, 0, nElements,
                (jlong *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_FLOAT:
            seq = seq_allocate( nElements, sizeof (float) );
            attach->GetFloatArrayRegion(
                (jfloatArray)java_data.l, 0, nElements,
                (jfloat *)((uno_Sequence *)seq.get())->elements );
            attach.ensure_no_exception();
            break;
        case typelib_TypeClass_DOUBLE:
            seq = seq_allocate( nElements, sizeof (double) );
            attach->GetDoubleArrayRegion(
                (jdoubleArray)java_data.l, 0, nElements,
                (jdouble *)((uno_Sequence *)seq.get())->elements );
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
                element_info = m_jni_info->get_type_info( attach, element_td.get() );
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
                        attach, attach->GetObjectArrayElement( (jobjectArray)java_data.l, nPos ) );
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
            throw BridgeRuntimeError(
                OUSTR("[map_to_uno():typelib_TypeClass_SEQUENCE] "
                      "unsupported sequence element type: ") +
                *reinterpret_cast< OUString const * >( &element_type->pTypeName ) );
        }

        if (assign)
            uno_destructData( uno_data, td.get(), 0 );
        *(uno_Sequence **)uno_data = (uno_Sequence *)seq.release();
        break;
    }
    case typelib_TypeClass_INTERFACE:
    {
        JLocalAutoRef jo_out_holder;
        if (out_param)
        {
            jo_out_holder.reset(
                attach, attach->GetObjectArrayElement( (jobjectArray)java_data.l, 0 ) );
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
                info = m_jni_info->get_type_info( attach, td.get() );
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
            OUSTR("[map_to_uno()] unsupported data type: ") +
            *reinterpret_cast< OUString const * >( &type->pTypeName ) );
    }
}

//##################################################################################################
//##################################################################################################
//##################################################################################################

//__________________________________________________________________________________________________
void jni_Bridge::map_to_java(
    JNI_attach const & attach,
    jvalue * java_data, void const * uno_data,
    typelib_TypeDescriptionReference * type, JNI_type_info const * info /* maybe 0 */,
    bool in_param, bool out_param,
    bool special_wrapped_integral_types ) const
{
    switch (type->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        if (out_param)
        {
            if (0 == java_data->l)
            {
                JLocalAutoRef jo_ar( attach, attach->NewCharArray( 1 ) );
                attach.ensure_no_exception();
                if (in_param)
                {
                    attach->SetCharArrayRegion(
                        (jcharArray)jo_ar.get(), 0, 1, (jchar *)uno_data );
                    attach.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    attach->SetCharArrayRegion(
                        (jcharArray)java_data->l, 0, 1, (jchar *)uno_data );
                    attach.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.c = *(jchar const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_info->m_class_Character, m_jni_info->m_ctor_Character_with_char, &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->c = *(jchar const *)uno_data;
        }
        break;
    case typelib_TypeClass_BOOLEAN:
        if (out_param)
        {
            if (0 == java_data->l)
            {
                JLocalAutoRef jo_ar( attach, attach->NewBooleanArray( 1 ) );
                attach.ensure_no_exception();
                if (in_param)
                {
                    attach->SetBooleanArrayRegion(
                        (jbooleanArray)jo_ar.get(), 0, 1, (jboolean *)uno_data );
                    attach.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    attach->SetBooleanArrayRegion(
                        (jbooleanArray)java_data->l, 0, 1, (jboolean *)uno_data );
                    attach.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.z = *(jboolean const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_info->m_class_Boolean, m_jni_info->m_ctor_Boolean_with_boolean, &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->z = *(jboolean const *)uno_data;
        }
        break;
    case typelib_TypeClass_BYTE:
        if (out_param)
        {
            if (0 == java_data->l)
            {
                JLocalAutoRef jo_ar( attach, attach->NewByteArray( 1 ) );
                attach.ensure_no_exception();
                if (in_param)
                {
                    attach->SetByteArrayRegion(
                        (jbyteArray)jo_ar.get(), 0, 1, (jbyte *)uno_data );
                    attach.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    attach->SetByteArrayRegion(
                        (jbyteArray)java_data->l, 0, 1, (jbyte *)uno_data );
                    attach.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.b = *(jbyte const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_info->m_class_Byte, m_jni_info->m_ctor_Byte_with_byte, &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->b = *(jbyte const *)uno_data;
        }
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        if (out_param)
        {
            if (0 == java_data->l)
            {
                JLocalAutoRef jo_ar( attach, attach->NewShortArray( 1 ) );
                attach.ensure_no_exception();
                if (in_param)
                {
                    attach->SetShortArrayRegion(
                        (jshortArray)jo_ar.get(), 0, 1, (jshort *)uno_data );
                    attach.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    attach->SetShortArrayRegion(
                        (jshortArray)java_data->l, 0, 1, (jshort *)uno_data );
                    attach.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.s = *(jshort const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_info->m_class_Short, m_jni_info->m_ctor_Short_with_short, &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->s = *(jshort const *)uno_data;
        }
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        if (out_param)
        {
            if (0 == java_data->l)
            {
                JLocalAutoRef jo_ar( attach, attach->NewIntArray( 1 ) );
                attach.ensure_no_exception();
                if (in_param)
                {
                    attach->SetIntArrayRegion(
                        (jintArray)jo_ar.get(), 0, 1, (jint *)uno_data );
                    attach.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    attach->SetIntArrayRegion(
                        (jintArray)java_data->l, 0, 1, (jint *)uno_data );
                    attach.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.i = *(jint const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_info->m_class_Integer, m_jni_info->m_ctor_Integer_with_int, &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->i = *(jint const *)uno_data;
        }
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        if (out_param)
        {
            if (0 == java_data->l)
            {
                JLocalAutoRef jo_ar( attach, attach->NewLongArray( 1 ) );
                attach.ensure_no_exception();
                if (in_param)
                {
                    attach->SetLongArrayRegion(
                        (jlongArray)jo_ar.get(), 0, 1, (jlong *)uno_data );
                    attach.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    attach->SetLongArrayRegion(
                        (jlongArray)java_data->l, 0, 1, (jlong *)uno_data );
                    attach.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.j = *(jlong const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_info->m_class_Long, m_jni_info->m_ctor_Long_with_long, &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->j = *(jlong const *)uno_data;
        }
        break;
    case typelib_TypeClass_FLOAT:
        if (out_param)
        {
            if (0 == java_data->l)
            {
                JLocalAutoRef jo_ar( attach, attach->NewFloatArray( 1 ) );
                attach.ensure_no_exception();
                if (in_param)
                {
                    attach->SetFloatArrayRegion(
                        (jfloatArray)jo_ar.get(), 0, 1, (jfloat *)uno_data );
                    attach.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    attach->SetFloatArrayRegion(
                        (jfloatArray)java_data->l, 0, 1, (jfloat *)uno_data );
                    attach.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.f = *(jfloat const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_info->m_class_Float, m_jni_info->m_ctor_Float_with_float, &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->f = *(jfloat const *)uno_data;
        }
        break;
    case typelib_TypeClass_DOUBLE:
        if (out_param)
        {
            if (0 == java_data->l)
            {
                JLocalAutoRef jo_ar( attach, attach->NewDoubleArray( 1 ) );
                attach.ensure_no_exception();
                if (in_param)
                {
                    attach->SetDoubleArrayRegion(
                        (jdoubleArray)jo_ar.get(), 0, 1, (jdouble *)uno_data );
                    attach.ensure_no_exception();
                }
                java_data->l = jo_ar.release();
            }
            else
            {
                if (in_param)
                {
                    attach->SetDoubleArrayRegion(
                        (jdoubleArray)java_data->l, 0, 1, (jdouble *)uno_data );
                    attach.ensure_no_exception();
                }
            }
        }
        else if (special_wrapped_integral_types)
        {
            jvalue arg;
            arg.d = *(double const *)uno_data;
            java_data->l = attach->NewObjectA(
                m_jni_info->m_class_Double, m_jni_info->m_ctor_Double_with_double, &arg );
            attach.ensure_no_exception();
        }
        else
        {
            java_data->d = *(jdouble const *)uno_data;
        }
        break;
    case typelib_TypeClass_STRING:
    {
        if (out_param)
        {
            JLocalAutoRef jo_in;
            if (in_param)
            {
                jo_in.reset(
                    attach, ustring_to_jstring( attach, *(rtl_uString * const *)uno_data ) );
            }
            if (0 == java_data->l)
            {
                java_data->l = attach->NewObjectArray(
                    1, m_jni_info->m_class_String, jo_in.get() );
                attach.ensure_no_exception();
            }
            else
            {
                attach->SetObjectArrayElement(
                    (jobjectArray)java_data->l, 0, jo_in.get() );
                attach.ensure_no_exception();
            }
        }
        else
        {
            OSL_ASSERT( in_param );
            java_data->l = ustring_to_jstring( attach, *(rtl_uString * const *)uno_data );
        }
        break;
    }
    case typelib_TypeClass_TYPE:
    {
        if (out_param)
        {
            JLocalAutoRef jo_in;
            if (in_param)
            {
                jo_in.reset(
                    attach, create_type(
                        attach, *(typelib_TypeDescriptionReference * const *)uno_data ) );
            }
            if (0 == java_data->l)
            {
                java_data->l = attach->NewObjectArray(
                    1, m_jni_info->m_class_Type, jo_in.get() );
                attach.ensure_no_exception();
            }
            else
            {
                attach->SetObjectArrayElement(
                    (jobjectArray)java_data->l, 0, jo_in.get() );
                attach.ensure_no_exception();
            }
        }
        else
        {
            OSL_ASSERT( in_param );
            java_data->l = create_type(
                attach, *(typelib_TypeDescriptionReference * const *)uno_data );
        }
        break;
    }
    case typelib_TypeClass_ANY:
    {
        JLocalAutoRef jo_any;
        if (in_param)
        {
            uno_Any const * pAny = (uno_Any const *)uno_data;
            if (typelib_TypeClass_VOID != pAny->pType->eTypeClass)
            {
                jvalue args[ 2 ];
                // map value
                map_to_java(
                    attach, args, pAny->pData, pAny->pType, 0,
                    true /* in */, false /* no out */, true /* create integral wrappers */ );
                switch (pAny->pType->eTypeClass)
                {
                case typelib_TypeClass_UNSIGNED_SHORT:
                case typelib_TypeClass_UNSIGNED_LONG:
                case typelib_TypeClass_UNSIGNED_HYPER:
                {
                    // build up com.sun.star.uno.Any
                    JLocalAutoRef jo_val( attach, args[ 0 ].l );
                    JLocalAutoRef jo_type( attach, create_type( attach, pAny->pType ) );
                    args[ 0 ].l = jo_type.get();
                    args[ 1 ].l = jo_val.get();
                    jo_any.reset(
                        attach, attach->NewObjectA(
                            m_jni_info->m_class_Any,
                            m_jni_info->m_ctor_Any_with_Type_Object, args ) );
                    attach.ensure_no_exception();
                    break;
                }
                default:
                    jo_any.reset( attach, args[ 0 ].l );
                    break;
                }
            }
        }
        if (out_param)
        {
            if (0 == java_data->l)
            {
                java_data->l = attach->NewObjectArray(
                    1, m_jni_info->m_class_Object, jo_any.get() );
                attach.ensure_no_exception();
            }
            else
            {
                attach->SetObjectArrayElement(
                    (jobjectArray)java_data->l, 0, jo_any.get() );
                attach.ensure_no_exception();
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
        OUString const & type_name = *reinterpret_cast< OUString const * >( &type->pTypeName );
        OString class_name(
            OUStringToOString( type_name.replace( '.', '/' ), RTL_TEXTENCODING_ASCII_US ) );
        JLocalAutoRef jo_enum_class( attach, find_class( attach, class_name.getStr() ) );

        JLocalAutoRef jo_enum;
        if (in_param)
        {
            // call static <enum_class>.fromInt( int )
            OStringBuffer sig_buf( 3 + class_name.getLength() );
            sig_buf.append( RTL_CONSTASCII_STRINGPARAM("(I)L") );
            sig_buf.append( class_name );
            sig_buf.append( ';' );
            OString sig( sig_buf.makeStringAndClear() );
            jmethodID method_id = attach->GetStaticMethodID(
                (jclass)jo_enum_class.get(), "fromInt", sig.getStr() );
            attach.ensure_no_exception();
            OSL_ASSERT( 0 != method_id );

            jvalue arg;
            arg.i = *(jint const *)uno_data;
            jo_enum.reset(
                attach, attach->CallStaticObjectMethodA(
                    (jclass)jo_enum_class.get(), method_id, &arg ) );
            attach.ensure_no_exception();
        }
        if (out_param)
        {
            if (0 == java_data->l)
            {
                java_data->l = attach->NewObjectArray(
                    1, (jclass)jo_enum_class.get(), jo_enum.get() );
                attach.ensure_no_exception();
            }
            else
            {
                attach->SetObjectArrayElement(
                    (jobjectArray)java_data->l, 0, jo_enum.get() );
                attach.ensure_no_exception();
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
        if (0 == info)
        {
            TypeDescr td( type );
            info = m_jni_info->get_type_info( attach, td.get() );
        }

        JLocalAutoRef jo_comp;
        if (in_param)
        {
            if (typelib_TypeClass_EXCEPTION == type->eTypeClass)
            {
                JLocalAutoRef jo_message(
                    attach, ustring_to_jstring( attach, *(rtl_uString **)uno_data ) );
                jvalue arg;
                arg.l = jo_message.get();
                jo_comp.reset(
                    attach, attach->NewObjectA( info->m_class, info->m_ctor, &arg ) );
                attach.ensure_no_exception();
            }
            else
            {
                jo_comp.reset(
                    attach, attach->NewObjectA( info->m_class, info->m_ctor, 0 ) );
                attach.ensure_no_exception();
            }

            for ( JNI_type_info const * linfo = info; 0 != linfo; linfo = linfo->m_base )
            {
                typelib_CompoundTypeDescription * comp_td =
                    (typelib_CompoundTypeDescription *)linfo->m_td.get();
                typelib_TypeDescriptionReference ** ppMemberTypeRefs = comp_td->ppTypeRefs;
                sal_Int32 * pMemberOffsets = comp_td->pMemberOffsets;
                for ( sal_Int32 nPos = comp_td->nMembers; nPos--; )
                {
                    jfieldID field_id = linfo->m_fields[ nPos ];
                    if (0 != field_id)
                    {
                        void const * p = (char const *)uno_data + pMemberOffsets[ nPos ];
                        typelib_TypeDescriptionReference * member_type = ppMemberTypeRefs[ nPos ];
                        switch (member_type->eTypeClass)
                        {
                        case typelib_TypeClass_CHAR:
                            attach->SetCharField(
                                jo_comp.get(), field_id, *(jchar const *)p );
                            break;
                        case typelib_TypeClass_BOOLEAN:
                            attach->SetBooleanField(
                                jo_comp.get(), field_id, *(jboolean const *)p );
                            break;
                        case typelib_TypeClass_BYTE:
                            attach->SetByteField(
                                jo_comp.get(), field_id, *(jbyte const *)p );
                            break;
                        case typelib_TypeClass_SHORT:
                        case typelib_TypeClass_UNSIGNED_SHORT:
                            attach->SetShortField(
                                jo_comp.get(), field_id, *(jshort const *)p );
                            break;
                        case typelib_TypeClass_LONG:
                        case typelib_TypeClass_UNSIGNED_LONG:
                            attach->SetIntField(
                                jo_comp.get(), field_id, *(jint const *)p );
                            break;
                        case typelib_TypeClass_HYPER:
                        case typelib_TypeClass_UNSIGNED_HYPER:
                            attach->SetLongField(
                                jo_comp.get(), field_id, *(jlong const *)p );
                            break;
                        case typelib_TypeClass_FLOAT:
                            attach->SetFloatField(
                                jo_comp.get(), field_id, *(jfloat const *)p );
                            break;
                        case typelib_TypeClass_DOUBLE:
                            attach->SetDoubleField(
                                jo_comp.get(), field_id, *(jdouble const *)p );
                            break;
                        case typelib_TypeClass_STRING: // string opt here
                        {
                            JLocalAutoRef jo_string(
                                attach, ustring_to_jstring( attach, *(rtl_uString * const *)p ) );
                            attach->SetObjectField( jo_comp.get(), field_id, jo_string.get() );
                            break;
                        }
                        default:
                        {
                            jvalue java_data;
                            map_to_java(
                                attach, &java_data, p, member_type, 0,
                                true /* in */, false /* no out */ );
                            JLocalAutoRef jo_obj( attach, java_data.l );
                            attach->SetObjectField( jo_comp.get(), field_id, jo_obj.get() );
                            break;
                        }
                        }
                    }
                }
            }
        }
        if (out_param)
        {
            if (0 == java_data->l)
            {
                java_data->l = attach->NewObjectArray( 1, info->m_class, jo_comp.get() );
                attach.ensure_no_exception();
            }
            else
            {
                attach->SetObjectArrayElement(
                    (jobjectArray)java_data->l, 0, jo_comp.get() );
                attach.ensure_no_exception();
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
        JLocalAutoRef jo_ar;

        sal_Int32 nElements;
        uno_Sequence const * seq = 0;
        if (in_param)
        {
            seq = *(uno_Sequence * const *)uno_data;
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
            jo_ar.reset( attach, attach->NewCharArray( nElements ) );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetCharArrayRegion(
                    (jcharArray)jo_ar.get(), 0, nElements, (jchar *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_BOOLEAN:
            jo_ar.reset( attach, attach->NewBooleanArray( nElements ) );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetBooleanArrayRegion(
                    (jbooleanArray)jo_ar.get(), 0, nElements, (jboolean *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_BYTE:
            jo_ar.reset( attach, attach->NewByteArray( nElements ) );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetByteArrayRegion(
                    (jbyteArray)jo_ar.get(), 0, nElements, (jbyte *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            jo_ar.reset( attach, attach->NewShortArray( nElements ) );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetShortArrayRegion(
                    (jshortArray)jo_ar.get(), 0, nElements, (jshort *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
            jo_ar.reset( attach, attach->NewIntArray( nElements ) );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetIntArrayRegion(
                    (jintArray)jo_ar.get(), 0, nElements, (jint *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            jo_ar.reset( attach, attach->NewLongArray( nElements ) );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetLongArrayRegion(
                    (jlongArray)jo_ar.get(), 0, nElements, (jlong *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_FLOAT:
            jo_ar.reset( attach, attach->NewFloatArray( nElements ) );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetFloatArrayRegion(
                    (jfloatArray)jo_ar.get(), 0, nElements, (jfloat *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_DOUBLE:
            jo_ar.reset( attach, attach->NewDoubleArray( nElements ) );
            attach.ensure_no_exception();
            if (0 < nElements)
            {
                attach->SetDoubleArrayRegion(
                    (jdoubleArray)jo_ar.get(), 0, nElements, (jdouble *)seq->elements );
                attach.ensure_no_exception();
            }
            break;
        case typelib_TypeClass_STRING:
            jo_ar.reset(
                attach, attach->NewObjectArray( nElements, m_jni_info->m_class_String, 0 ) );
            attach.ensure_no_exception();
            if (in_param)
            {
                rtl_uString * const * pp = (rtl_uString * const *)seq->elements;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    JLocalAutoRef jo_string( attach, ustring_to_jstring( attach, pp[ nPos ] ) );
                    attach->SetObjectArrayElement(
                        (jobjectArray)jo_ar.get(), nPos, jo_string.get() );
                    attach.ensure_no_exception();
                }
            }
            break;
        case typelib_TypeClass_TYPE:
            jo_ar.reset(
                attach, attach->NewObjectArray( nElements, m_jni_info->m_class_Type, 0 ) );
            attach.ensure_no_exception();
            if (in_param)
            {
                typelib_TypeDescriptionReference * const * pp =
                    (typelib_TypeDescriptionReference * const *)seq->elements;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue val;
                    map_to_java(
                        attach, &val, &pp[ nPos ], element_type, 0,
                        true /* in */, false /* no out */ );
                    JLocalAutoRef jo_element( attach, val.l );
                    attach->SetObjectArrayElement(
                        (jobjectArray)jo_ar.get(), nPos, jo_element.get() );
                    attach.ensure_no_exception();
                }
            }
            break;
        case typelib_TypeClass_ANY:
            jo_ar.reset(
                attach, attach->NewObjectArray( nElements, m_jni_info->m_class_Object, 0 ) );
            attach.ensure_no_exception();
            if (in_param)
            {
                uno_Any const * p = (uno_Any const *)seq->elements;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue val;
                    map_to_java(
                        attach, &val, &p[ nPos ], element_type, 0,
                        true /* in */, false /* no out */ );
                    JLocalAutoRef jo_element( attach, val.l );
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
            JLocalAutoRef jo_enum_class( attach, find_class( attach, class_name.getStr() ) );

            jo_ar.reset(
                attach, attach->NewObjectArray( nElements, (jclass)jo_enum_class.get(), 0 ) );
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

                sal_Int32 const * p = (sal_Int32 const *)seq->elements;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue arg;
                    arg.i = p[ nPos ];
                    JLocalAutoRef jo_enum(
                        attach, attach->CallStaticObjectMethodA(
                            (jclass)jo_enum_class.get(), method_id, &arg ) );
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
                m_jni_info->get_type_info( attach, element_td.get() );

            jo_ar.reset( attach, attach->NewObjectArray( nElements, element_info->m_class, 0 ) );
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
                    JLocalAutoRef jo_element( attach, val.l );
                    attach->SetObjectArrayElement(
                        (jobjectArray)jo_ar.get(), nPos, jo_element.get() );
                    attach.ensure_no_exception();
                }
            }
            break;
        }
        case typelib_TypeClass_SEQUENCE:
        {
            OStringBuffer buf( 64 );
            m_jni_info->append_sig( &buf, element_type );
            OString class_name( buf.makeStringAndClear() );
            JLocalAutoRef jo_seq_class( attach, find_class( attach, class_name.getStr() ) );

            jo_ar.reset(
                attach, attach->NewObjectArray( nElements, (jclass)jo_seq_class.get(), 0 ) );
            attach.ensure_no_exception();

            if (0 < nElements)
            {
                TypeDescr element_td( element_type );
                uno_Sequence ** elements = (uno_Sequence **)seq->elements;
                for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                {
                    jvalue java_data;
                    map_to_java(
                        attach, &java_data, elements + nPos, element_type, 0,
                        true /* in */, false /* no out */ );
                    JLocalAutoRef jo_seq( attach, java_data.l );
                    attach->SetObjectArrayElement(
                        (jobjectArray)jo_ar.get(), nPos, jo_seq.get() );
                    attach.ensure_no_exception();
                }
            }
            break;
        }
        case typelib_TypeClass_INTERFACE:
        {
            TypeDescr element_td( element_type );
            JNI_type_info const * element_info =
                m_jni_info->get_type_info( attach, element_td.get() );

            jo_ar.reset( attach, attach->NewObjectArray( nElements, element_info->m_class, 0 ) );
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
                    JLocalAutoRef jo_element( attach, val.l );
                    attach->SetObjectArrayElement(
                        (jobjectArray)jo_ar.get(), nPos, jo_element.get() );
                    attach.ensure_no_exception();
                }
            }
            break;
        }
        default:
            throw BridgeRuntimeError(
                OUSTR("[map_to_java():typelib_TypeClass_SEQUENCE] "
                      "unsupported element data type: ") +
                *reinterpret_cast< OUString const * >( &element_type->pTypeName ) );
        }

        if (out_param)
        {
            if (0 == java_data->l)
            {
                JLocalAutoRef jo_element_class( attach, get_class( attach, jo_ar.get() ) );
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
                attach->SetObjectArrayElement(
                    (jobjectArray)java_data->l, 0, jo_ar.get() );
                attach.ensure_no_exception();
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
        JLocalAutoRef jo_iface;
        if (in_param)
        {
            uno_Interface * pUnoI = *(uno_Interface * const *)uno_data;
            if (0 != pUnoI)
            {
                if (0 == info)
                {
                    TypeDescr td( type );
                    info = m_jni_info->get_type_info( attach, td.get() );
                }
                jo_iface.reset( attach, map_uno2java( attach, pUnoI, info ) );
            }
        }
        if (out_param)
        {
            if (0 == java_data->l)
            {
                if (typelib_typedescriptionreference_equals(
                        type, m_jni_info->m_XInterface_td.get()->pWeakRef ))
                {
                    java_data->l = attach->NewObjectArray(
                        1, m_jni_info->m_class_Object, jo_iface.get() );
                    attach.ensure_no_exception();
                }
                else
                {
                    OUString const & type_name =
                        *reinterpret_cast< OUString const * >( &type->pTypeName );
                    OString class_name(
                        OUStringToOString(
                            type_name.replace( '.', '/' ),
                            RTL_TEXTENCODING_ASCII_US ) );
                    JLocalAutoRef jo_iface_class(
                        attach, find_class( attach, class_name.getStr() ) );

                    java_data->l = attach->NewObjectArray(
                        1, (jclass)jo_iface_class.get(), jo_iface.get() );
                    attach.ensure_no_exception();
                }
            }
            else
            {
                attach->SetObjectArrayElement(
                    (jobjectArray)java_data->l, 0, jo_iface.get() );
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
            OUSTR("[map_to_java()] unsupported data type: ") +
            *reinterpret_cast< OUString const * >( &type->pTypeName ) );
    }
}

}
