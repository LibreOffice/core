/*************************************************************************
 *
 *  $RCSfile: vendorplugin.h,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:49:40 $
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
   the existence of a particular set of features or bug fixes. To keep the
   API simple it was designed so that an implementation may act on behalf of
   only ONE vendor. </p>
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
    <p>
    The function has parameters which determines which versions of the respective
    JREs are supported. A JRE which does not meet the version requirements will
    be ignored.</p>
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
        [in] only JRE from this vendor are examined.
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
    JFW_PLUGIN_E_NONE the function ran successfully.</br>
    JFW_PLUGIN_E_ERROR an error occurred during execution.</br>
    JFW_PLUGIN_E_INVALID_ARG an argument was not valid. For example
    <code>nSizeExcludeList</code> is greater null but <code>arExcludeList</code>
    is NULL.</br>
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
   <p>
   If the given location belongs to a JRE whoose vendor is supported by
   this library and the JRE has a version which meets the requirements as
   specified by <code>sMinVersion, sMaxVersion, arExcludeList</code> then
   this function returns a JavaInfo object for this JRE.</p>

   @param sLocation
       [in] a file URL to the directory of the JRE.
   @param sMinVersion
       [in] represents the minimum version of a JRE. It can be NULL.
   @param sMaxVersion
       [in] represents the maximum version of a JRE. It can be NULL.
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
    is NULL.</br>
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
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString * *arExcludeList,
    sal_Int32  nSizeExcludeList,
    JavaInfo ** ppInfo);

/** starts a Java Virtual Machine.
    <p>
    The caller should provide all essential JavaVMOptions, such as the
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
    in by this library.</br>
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
