/*************************************************************************
 *
 *  $RCSfile: jni_helper.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2002-11-01 14:24:57 $
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
#ifndef _JNI_HELPER_H_
#define _JNI_HELPER_H_

#include "jni_base.h"
#include "jni_info.h"


namespace jni_bridge
{

//--------------------------------------------------------------------------------------------------
inline void jstring_to_ustring( JNI_attach const & attach, rtl_uString ** out_ustr, jstring jstr )
{
    if (0 == jstr)
    {
        rtl_uString_new( out_ustr );
    }
    else
    {
        jsize len = attach->GetStringLength( jstr );
        ::std::auto_ptr< rtl_mem > mem(
            rtl_mem::allocate( sizeof (rtl_uString) + (len * sizeof (sal_Unicode)) ) );
        rtl_uString * ustr = (rtl_uString *)mem.get();
        attach->GetStringRegion( jstr, 0, len, (jchar *)ustr->buffer );
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
inline jstring ustring_to_jstring( JNI_attach const & attach, rtl_uString const * ustr )
{
    jstring jstr = attach->NewString( (jchar const *)ustr->buffer, ustr->length );
    attach.ensure_no_exception();
    return jstr;
}

//##################################################################################################

//--------------------------------------------------------------------------------------------------
inline jclass find_class( JNI_attach const & attach, char const * class_name )
{
    jclass jo_class = attach->FindClass( class_name );
    attach.ensure_no_exception();
    return jo_class;
}
//--------------------------------------------------------------------------------------------------
inline jclass find_class( JNI_attach const & attach, ::rtl::OUString const & class_name )
{
    ::rtl::OString cstr_name( ::rtl::OUStringToOString( class_name, RTL_TEXTENCODING_ASCII_US ) );
    return find_class( attach, cstr_name );
}
//--------------------------------------------------------------------------------------------------
inline jclass get_class( JNI_attach const & attach, jobject jo )
{
    jclass jo_class = (jclass)attach->CallObjectMethodA(
        jo, attach.get_jni_info()->m_method_Object_getClass, 0 );
    attach.ensure_no_exception();
    return jo_class;
}
//--------------------------------------------------------------------------------------------------
inline ::rtl::OUString get_class_name( JNI_attach const & attach, jobject jo )
{
    JLocalAutoRef jo_class( attach, get_class( attach, jo ) );
    JLocalAutoRef jo_name(
        attach, attach->CallObjectMethodA(
            jo_class.get(), attach.get_jni_info()->m_method_Class_getName, 0 ) );
    attach.ensure_no_exception();
    return jstring_to_oustring( attach, (jstring)jo_name.get() );
}

//##################################################################################################

//--------------------------------------------------------------------------------------------------
inline jobject create_type( JNI_attach const & attach, jclass clazz )
{
    JNI_info const * jni_info = attach.get_jni_info();
    jvalue arg;
    arg.l = clazz;
    jobject jo_type =
        attach->NewObjectA( jni_info->m_class_Type, jni_info->m_ctor_Type_with_Class, &arg );
    attach.ensure_no_exception();
    return jo_type;
}
//--------------------------------------------------------------------------------------------------
inline jobject create_type(
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
    jobject jo_type = attach->NewObjectA(
        jni_info->m_class_Type, jni_info->m_ctor_Type_with_Name_TypeClass, args );
    attach.ensure_no_exception();
    return jo_type;
}

//--------------------------------------------------------------------------------------------------
inline jobject compute_oid( JNI_attach const & attach, jobject jo )
{
    JNI_info const * jni_info = attach.get_jni_info();
    jvalue arg;
    arg.l= jo;
    jobject jo_oid = attach->CallStaticObjectMethodA(
        jni_info->m_class_UnoRuntime, jni_info->m_method_UnoRuntime_generateOid, &arg );
    attach.ensure_no_exception();
    return jo_oid;
}

}

#endif
