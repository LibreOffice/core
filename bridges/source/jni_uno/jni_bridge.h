/*************************************************************************
 *
 *  $RCSfile: jni_bridge.h,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 10:50:04 $
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
        JNI_interface_type_info const * info, sal_Int32 function_pos,
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
