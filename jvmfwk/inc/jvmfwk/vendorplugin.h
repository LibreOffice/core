/*************************************************************************
 *
 *  $RCSfile: vendorplugin.h,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: jl $ $Date: 2004-05-18 15:11:57 $
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

/** @HTML */
#if !defined INCLUDED_JVMFWK_VENDORPLUGIN_H
#define INCLUDED_JVMFWK_VENDORPLUGIN_H

#include "jvmfwk/framework.h"
#include "rtl/ustring.h"
#include "jni.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
   @file
   Libraries which implement this interface will be dynamically loaded and
   unloaded. Therefore do not keep global variables.
 */

typedef enum
{
    JFW_PLUGIN_E_NONE,
    JFW_PLUGIN_E_ERROR,
    JFW_PLUGIN_E_INVALID_ARG,
    JFW_PLUGIN_E_WRONG_VERSION_FORMAT,
    JFW_PLUGIN_E_FAILED_VERSION,
    JFW_PLUGIN_E_NO_JRE,
    JFW_PLUGIN_E_WRONG_VENDOR,
    JFW_PLUGIN_E_VM_CREATION_FAILED
} javaPluginError;



/** obtains information about installations of Java Runtime Environments (JREs).
    <p>
    The function has parameters which determines which versions of the respective
    JREs are supported. A JRE which does not meet the version requirements will
    be ignored.</p>
    The JavaInfo structurs returned in <code>parJavaInfo</code> should be ordered
    according to their version. The one, representing a JRE with the highest
    version should be the first in the array. </p>
    <p>
    The function allocates memory for an array and all the the JavaInfo objects returned
    in <code>parJavaInfo</code>. The caller must free each JavaInfo object by calling
    <code>jfw_freeJavaInfo</code>. The array is to be freed by rtl_freeMemory.
    In case an error occurred parJavaInfo does not to be freed.
    </p>

    @param sMinVersion
        [in] represents the minimum version of a JRE. It can be NULL.
    @param sMaxVersion
        [in] represents the maximum version of a JRE. It can be NULL.
    @param arExcludeList
        [in] contains a list of &quot;bad&quot; versions. JREs which have one of these
        versions must not be returned by this function. It can be NULL.
    @param nSizeExcludeList
        [in] the number of version strings contained in <code>arExcludeList</code>.
    @param parJavaInfo
        [out] if the function runs successfully then <code>parJavaInfo</code> contains
        on return an array of pointers to <code>JavaInfo</code> objects.
    @param nSizeJavaInfo
       [out] the number of <code>JavaInfo</code> pointers contained in
       <code>parJavaInfo</code>.

    @return
    JFW_PLUGIN_E_NONE </br>
    JFW_PLUGIN_E_ERROR </br>
    JFW_PLUGIN_E_INVALID_ARG </br>
    JFW_PLUGIN_E_WRONG_VERSION_FORMAT
 */
javaPluginError jfw_plugin_getAllJavaInfos(
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString * * arExcludeList,
    sal_Int32  nSizeExcludeList,
    JavaInfo*** parJavaInfo,
    sal_Int32 *nSizeJavaInfo);

/**
   @return
   JFW_PLUGIN_E_NONE
    JFW_PLUGIN_E_ERROR
    JFW_PLUGIN_E_INVALID_ARG
    JFW_PLUGIN_E_WRONG_VERSION_FORMAT
    JFW_PLUGIN_E_FAILED_VERSION
    JFW_PLUGIN_E_NO_JRE
 */
javaPluginError jfw_plugin_getJavaInfoByPath(
    rtl_uString *path,
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString * *arExcludeList,
    sal_Int32  nLenList,
    JavaInfo ** ppInfo);

/** starts a Java Virtual Machine.
    <p>
    The function shall ensure, that the VM does not abort the process
    during instantiation.</p>
    @param
    JFW_PLUGIN_E_NONE,
    JFW_PLUGIN_E_ERROR,
    JFW_PLUGIN_E_WRONG_VENDOR
    JFW_PLUGIN_E_INVALID_ARG,
    JFW_PLUGIN_E_VM_CREATION_FAILED

 */
javaPluginError jfw_plugin_startJavaVirtualMachine(
    const JavaInfo *info,
    const JavaVMOption* options,
    sal_Int32 cOptions,
    JavaVM ** ppVM,
    JNIEnv ** ppEnv);



#ifdef __cplusplus
}
#endif


#endif
