/*************************************************************************
 *
 *  $RCSfile: preload.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 12:23:48 $
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

#include "jni.h"

#include "rtl/ustring.hxx"
#include "osl/module.h"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

#if ! defined SAL_DLLPREFIX
#define SAL_DLLPREFIX ""
#endif

using ::rtl::OUString;

extern "C"
{
typedef jboolean (JNICALL * fptr_writeInfo)(
    JNIEnv *, jclass, jstring, jobject, jobject );
typedef jobject (JNICALL * fptr_getFactory)(
    JNIEnv *, jclass, jstring, jstring, jobject, jobject );
typedef jobject (JNICALL * fptr_createRegistryServiceFactory)(
    JNIEnv *, jclass, jstring, jstring, jboolean );
typedef jobject (JNICALL * fptr_bootstrap)(
    JNIEnv *_env, jclass, jstring, jobjectArray );

static fptr_writeInfo s_writeInfo;
static fptr_getFactory s_getFactory;
static fptr_createRegistryServiceFactory s_createRegistryServiceFactory;
static fptr_bootstrap s_bootstrap;
static bool s_inited = false;

//--------------------------------------------------------------------------------------------------
static bool inited_juhx( JNIEnv * jni_env )
{
    if (s_inited)
        return true;
    OUString lib_name = OUSTR(SAL_DLLPREFIX "juhx" SAL_DLLEXTENSION);
    oslModule hModule =
        osl_loadModule( lib_name.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
    if (0 == hModule)
    {
        jclass c = jni_env->FindClass( "java/lang/RuntimeException" );
        jni_env->ThrowNew(
            c, "error loading " SAL_DLLPREFIX "juhx" SAL_DLLEXTENSION "!" );
        return false;
    }
    else
    {
        OUString symbol =
              OUSTR("Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1writeInfo");
        s_writeInfo = (fptr_writeInfo)osl_getSymbol( hModule, symbol.pData );
        symbol =
            OUSTR("Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1getFactory");
        s_getFactory = (fptr_getFactory)osl_getSymbol( hModule, symbol.pData );
        symbol =
            OUSTR("Java_com_sun_star_comp_helper_RegistryServiceFactory_createRegistryServiceFactory");
        s_createRegistryServiceFactory =
            (fptr_createRegistryServiceFactory)osl_getSymbol( hModule, symbol.pData );
        symbol =
            OUSTR("Java_com_sun_star_comp_helper_Bootstrap_cppuhelper_1bootstrap");
        s_bootstrap =
            (fptr_bootstrap)osl_getSymbol( hModule, symbol.pData );

        if (0 == s_writeInfo ||
            0 == s_getFactory ||
            0 == s_createRegistryServiceFactory ||
            0 == s_bootstrap)
        {
            jclass c = jni_env->FindClass( "java/lang/RuntimeException" );
            jni_env->ThrowNew(
                c, "error resolving symbols of " SAL_DLLPREFIX "juhx" SAL_DLLEXTENSION "!" );
            return false;
        }
    }
    s_inited = true;
    return true;
}

//==================================================================================================
JNIEXPORT jboolean JNICALL
Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1writeInfo(
    JNIEnv * pJEnv, jclass jClass, jstring jLibName, jobject jSMgr, jobject jRegKey )
{
    if (inited_juhx( pJEnv ))
        return (*s_writeInfo)( pJEnv, jClass, jLibName, jSMgr, jRegKey );
    return JNI_FALSE;
}
//==================================================================================================
JNIEXPORT jobject JNICALL
Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1getFactory(
    JNIEnv * pJEnv, jclass jClass, jstring jLibName, jstring jImplName,
    jobject jSMgr, jobject jRegKey )
{
    if (inited_juhx( pJEnv ))
        return (*s_getFactory)( pJEnv, jClass, jLibName, jImplName, jSMgr, jRegKey );
    return 0;
}
//==================================================================================================
JNIEXPORT jobject JNICALL
Java_com_sun_star_comp_helper_RegistryServiceFactory_createRegistryServiceFactory(
    JNIEnv * pJEnv, jclass jClass, jstring jWriteRegFile,
    jstring jReadRegFile, jboolean jbReadOnly )
{
    if (inited_juhx( pJEnv ))
    {
        return (*s_createRegistryServiceFactory)(
            pJEnv, jClass, jWriteRegFile, jReadRegFile, jbReadOnly );
    }
    return 0;
}
//==================================================================================================
JNIEXPORT jobject JNICALL
Java_com_sun_star_comp_helper_Bootstrap_cppuhelper_1bootstrap(
    JNIEnv * jni_env, jclass jClass, jstring juno_rc, jobjectArray jpairs )
{
    if (inited_juhx( jni_env ))
        return (*s_bootstrap)( jni_env, jClass, juno_rc, jpairs );
    return 0;
}
}
