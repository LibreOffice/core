/*************************************************************************
 *
 *  $RCSfile: jni_java2uno.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2002-09-26 14:37:03 $
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

#include <rtl/ustring.hxx>


using namespace ::rtl;

namespace jni_bridge
{

//__________________________________________________________________________________________________
jobject jni_Bridge::map_uno2java(
    JNI_attach const & attach,
    uno_Interface * pUnoI, JNI_type_info const * info ) const
{
    // get oid
    rtl_uString * pOid = 0;
    (*m_uno_env->getObjectIdentifier)( m_uno_env, &pOid, pUnoI );
    OSL_ASSERT( 0 != pOid );
    OUString oid( pOid, SAL_NO_ACQUIRE );

    // getRegisteredInterface()
    JLocalAutoRef jo_oid( ustring_to_jstring( oid.pData, attach ), attach );

    jvalue args[ 2 ];
    args[ 0 ].l = jo_oid.get();
    args[ 1 ].l = info->m_jo_type;
    JLocalAutoRef jo_iface(
        attach->CallObjectMethodA(
            m_jni_class_data->m_object_java_env,
            m_jni_class_data->m_method_IEnvironment_getRegisteredInterface, args ),
        attach );
    attach.ensure_no_exception();

    if (! jo_iface.is()) // no registered iface
    {
        // register uno interface
        (*m_uno_env->registerInterface)(
            m_uno_env, reinterpret_cast< void ** >( &pUnoI ), oid.pData,
            (typelib_InterfaceTypeDescription *)info->m_td );

        try
        {
            // create java proxy
            jvalue args[ 5 ];
            acquire();
            args[ 0 ].j = reinterpret_cast< sal_Int64 >( this );
            (*pUnoI->acquire)( pUnoI );
            args[ 1 ].j = reinterpret_cast< sal_Int64 >( pUnoI );
            typelib_typedescription_acquire( (typelib_TypeDescription *)info->m_td );
            args[ 2 ].j = reinterpret_cast< sal_Int64 >( info->m_td );
            args[ 3 ].l = info->m_jo_type;
            args[ 4 ].l = jo_oid.get();
            jo_iface.reset(
                attach->CallStaticObjectMethodA(
                    m_jni_class_data->m_class_JNI_proxy,
                    m_jni_class_data->m_method_JNI_proxy_create, args ),
                attach );

            // register at java env
            jo_iface.reset(
                m_jni_class_data->java_env_registerInterface(
                    jo_iface.get(), (jstring)jo_oid.get(), info->m_jo_type, attach ),
                attach );
        }
        catch (...)
        {
            (*m_uno_env->revokeInterface)( m_uno_env, pUnoI );
            throw;
        }
    }

    return jo_iface.release();
}

}

using namespace ::jni_bridge;

//==================================================================================================
extern "C" JNIEXPORT void JNICALL Java_jni_1uno_JNI_1proxy_free_1handles(
    JNIEnv * jni_env, jclass clazz,
    jlong bridge_handle, jlong receiver_handle, jlong td_handle )
    SAL_THROW_EXTERN_C()
{
    typelib_TypeDescription * td = reinterpret_cast< typelib_TypeDescription * >( td_handle );
    uno_Interface * pUnoI = reinterpret_cast< uno_Interface * >( receiver_handle );
    jni_Bridge * bridge = reinterpret_cast< jni_Bridge * >( bridge_handle );

    // revoke from uno env
    (*bridge->m_uno_env->revokeInterface)( bridge->m_uno_env, pUnoI );
    (*pUnoI->release)( pUnoI );
    typelib_typedescription_release( td );
    bridge->release();
}
//==================================================================================================
extern "C" JNIEXPORT jobject JNICALL Java_jni_1uno_JNI_1proxy_dispatch_1method(
    JNIEnv * jni_env, jclass clazz,
    jlong bridge_handle, jlong receiver_handle, jlong td_handle,
    jstring jo_method, jobjectArray jo_args )
    SAL_THROW_EXTERN_C()
{
    OSL_TRACE("### received call!");
    return 0;
}
