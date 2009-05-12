/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vendorplugin.h,v $
 * $Revision: 1.15 $
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

/** @HTML */
#if !defined INCLUDED_JVMFWK_VENDORPLUGIN_H
#define INCLUDED_JVMFWK_VENDORPLUGIN_H

#include "jvmfwk/framework.h"
#include "rtl/ustring.h"
#ifdef SOLAR_JAVA
#include "jni.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/**
   @file
   <p>
   This API shall be implemented if one wants to support a Java Runtime
   Environment (JRE) of a particular vendor. Because there is currently no
   specification which rules the structure and location of JRE installations
   and the format of version strings it is not possible to supply a general
   implementation for all possible vendors. If an application determines exactly
   what version a JRE must have then it relies on certain features and bug
   fixes of that version. Because a version 1.4.2_1 from vendor X may contain
   different fixes as the same version from vendor Y it is important to see
   version an vendor as one entity. One without the other does not guarantee
   the existence of a particular set of features or bug fixes. An implementation
   of this API may support multiple vendors. </p>
   <p>
   Libraries which implement this interface will be dynamically loaded and
   unloaded by the java framework (jvmfwk/framework.h). Therefore they must not
   keep global variables.
   </p>
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

    <p>The function gathers information about available JREs which have the same
    vendor as determined by the <code>sVendor</code> parameter. Only information
    about those JREs which match the version requirements are returned. These
    requirements are specified by the parameters <code>sMinVersion</code>,
    <code>sMaxVersion</code> and <code>arExcludeList</code>.
    </p>
    <p>
    The JavaInfo structures returned in <code>parJavaInfo</code> should be ordered
    according to their version. The one, representing a JRE with the highest
    version should be the first in the array. </p>
    <p>
    The function allocates memory for an array and all the JavaInfo objects returned
    in <code>parJavaInfo</code>. The caller must free each JavaInfo object by calling
    <code>jfw_freeJavaInfo</code> (#include "jvmfwk/framework.h"). The array is to be
    freed by rtl_freeMemory.
    In case an error occurred <code>parJavaInfo</code> need not be freed.
    </p>
    @param sVendor
        [in] only JREs from this vendor are examined. This parameter always contains
        a vendor string. That is, the string it is not empty.
    @param sMinVersion
        [in] represents the minimum version of a JRE. The string can be empty but
        a null pointer is not allowed.
    @param sMaxVersion
        [in] represents the maximum version of a JRE. The string can be empty but
        a null pointer is not allowed.
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
    JFW_PLUGIN_E_NONE the function ran successfully.</br>
    JFW_PLUGIN_E_ERROR an error occurred during execution.</br>
    JFW_PLUGIN_E_INVALID_ARG an argument was not valid. For example
    <code>nSizeExcludeList</code> is greater null but <code>arExcludeList</code>
    is NULL or NULL pointer were passed for at least on of the strings.</br>
    JFW_PLUGIN_E_WRONG_VERSION_FORMAT the version strings in
    <code>sMinVersion,sMaxVersion,arExcludeList</code> are not recognized as valid
    version strings.
 */
javaPluginError jfw_plugin_getAllJavaInfos(
    rtl_uString *sVendor,
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString * * arExcludeList,
    sal_Int32  nSizeExcludeList,
    JavaInfo*** parJavaInfo,
    sal_Int32 *nSizeJavaInfo);

/** obtains information for a JRE at a given location.

   <p>If the given location belongs to a JRE whoose vendor matches the
   sVendor argument and the JRE has a version which meets the requirements as
   specified by <code>sMinVersion, sMaxVersion, arExcludeList</code> then
   this function shall return a JavaInfo object for this JRE if this implementation
   supports this vendor.</p>

   @param sLocation
       [in] a file URL to the directory of the JRE.
   @param sVendor
      [in] a name of a vendor. This parameter always contains
        a vendor string. That is, the string it is not empty.
   @param sMinVersion
       [in] represents the minimum version of a JRE.
   @param sMaxVersion
       [in] represents the maximum version of a JRE.
   @param arExcludeList
       [in] contains a list of &quot;bad&quot; versions. JREs which have one of these
        versions must not be returned by this function. It can be NULL.
   @param nSizeExcludeList
       [in] the number of version strings contained in <code>arExcludeList</code>.
   @param ppInfo
       [out] if the function runs successfully then <code>ppInfo</code> contains
        on return a pointer to a <code>JavaInfo</code> object.

   @return
   JFW_PLUGIN_E_NONE the function ran successfully.</br>
   JFW_PLUGIN_E_ERROR an error occurred during execution.</br>
   JFW_PLUGIN_E_INVALID_ARG an argument was not valid. For example
    <code>nSizeExcludeList</code> is greater null but <code>arExcludeList</code>
    is NULL, NULL pointer were passed for at least on of the strings, sLocation
    is an empty string.</br>
   JFW_PLUGIN_E_WRONG_VERSION_FORMAT the version strings in
    <code>sMinVersion,sMaxVersion,arExcludeList</code> are not recognized as valid
    version strings.
   JFW_PLUGIN_E_FAILED_VERSION there is a JRE at the given location but it does not
   meet the version requirements.
   JFW_PLUGIN_E_NO_JRE no JRE could be detected at the given location. However, that
   does not mean necessarily that there is no JRE. There could be a JRE but it has
   a vendor which is not supported by this API implementation.
 */
javaPluginError jfw_plugin_getJavaInfoByPath(
    rtl_uString *sLocation,
    rtl_uString *sVendor,
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString * *arExcludeList,
    sal_Int32  nSizeExcludeList,
    JavaInfo ** ppInfo);

/** starts a Java Virtual Machine.

    <p>The caller should provide all essential JavaVMOptions, such as the
    class path (-Djava.class.path=xxx). It is assumed that the caller
    knows what JRE is used. Hence the implementation does not need to check
    the options for validity. If a user configured the application to
    use specific options, such as -X..., then it is in his responsibility to
    ensure that the application works properly. The function may add or modify
    properties. For example, it may add to the class path property.
    <p>
    The function must ensure, that the VM does not abort the process
    during instantiation.</p>
    <p>
    The function receives a <code>JavaInfo</code> object that was created
    by the functions <code>jfw_plugin_getJavaInfoByPath</code> or
    <code>jfw_plugin_getAllJavaInfos</code> from the same library. This can be
    guaranteed if an application uses exactly one library for one vendor.
    Therefore the functions which create the <code>JavaInfo</code> can store all
    necessary information which are needed for starting the VM into that
    structure. </p>

    @param pInfo
        [in] the JavaInfo object with information about the JRE.
    @param arOptions
        [in] the options which are passed into the JNI_CreateJavaVM function.
        Can be NULL.
    @param nSizeOptions
        [in] the number of elements in <code>arOptions</code>.
    @param ppVM
        [out] the JavaVM pointer of the created VM.
    @param ppEnv
        [out] the JNIEnv pointer of the created VM.

    @return
    JFW_PLUGIN_E_NONE the function ran successfully.</br>
    JFW_PLUGIN_E_ERROR an error occurred during execution.</br>
    JFW_PLUGIN_E_WRONG_VENDOR the <code>JavaInfo</code> object was not created
    in by this library and the VM cannot be started.</br>
    JFW_PLUGIN_E_INVALID_ARG an argument was not valid. For example
    <code>pInfo</code> or , <code>ppVM</code> or <code>ppEnv</code> are NULL.
    </br>
    JFW_PLUGIN_E_VM_CREATION_FAILED a VM could not be created. The error was caused
    by the JRE.
 */
javaPluginError jfw_plugin_startJavaVirtualMachine(
    const JavaInfo *pInfo,
    const JavaVMOption *arOptions,
    sal_Int32 nSizeOptions,
    JavaVM ** ppVM,
    JNIEnv ** ppEnv);



#ifdef __cplusplus
}
#endif


#endif
