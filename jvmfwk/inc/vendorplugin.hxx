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

/** @HTML */
#ifndef INCLUDED_JVMFWK_INC_VENDORPLUGIN_HXX
#define INCLUDED_JVMFWK_INC_VENDORPLUGIN_HXX

#include <jvmfwk/framework.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.h>
#include <jni.h>

#include <memory>
#include <vector>
#include "vendorbase.hxx"

namespace jfw { class VendorSettings; }

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
   version and vendor as one entity. One without the other does not guarantee
   the existence of a particular set of features or bug fixes. An implementation
   of this API may support multiple vendors. </p>
 */

enum class javaPluginError
{
    NONE,
    Error,
    InvalidArg,
    WrongVersionFormat,
    FailedVersion,
    NoJre,
    WrongArch,
    VmCreationFailed
};


/** obtains information about installations of Java Runtime Environments (JREs).

    <p>The function gathers information about available JREs. Only information
    about those JREs which match the version requirements are returned. These
    requirements are specified by the parameter <code>vendorSettings</code>.
    </p>
    <p>
    The JavaInfo structures returned in <code>parJavaInfo</code> should be ordered
    according to their version. The one, representing a JRE with the highest
    version should be the first in the array. </p>
    @param parJavaInfo
        [out] if the function runs successfully then <code>parJavaInfo</code> contains
        on return a vector of pointers to <code>JavaInfo</code> objects.

    @return
    javaPluginError::NONE the function ran successfully.</br>
    javaPluginError::Error an error occurred during execution.</br>
    javaPluginError::WrongVersionFormat the version strings in
    <code>vendorSettings</code> are not recognized as valid
    version strings.
 */
javaPluginError jfw_plugin_getAllJavaInfos(
    bool checkJavaHomeAndPath,
    jfw::VendorSettings const & vendorSettings,
    std::vector<std::unique_ptr<JavaInfo>> * parJavaInfo,
    std::vector<rtl::Reference<jfw_plugin::VendorBase>> & infos);

/** obtains information for a JRE at a given location.

   <p>If the given location belongs to a JRE
   and the JRE has a version which meets the requirements as
   specified by <code>vendorSettings</code> then
   this function shall return a JavaInfo object for this JRE.</p>

   @param sLocation
       [in] a file URL to the directory of the JRE.
   @param ppInfo
       [out] if the function runs successfully then <code>ppInfo</code> contains
        on return a pointer to a <code>JavaInfo</code> object.

   @return
   javaPluginError::NONE the function ran successfully.</br>
   javaPluginError::Error an error occurred during execution.</br>
   javaPluginError::InvalidArg an argument was not valid. For example, sLocation
    is an empty string.</br>
   javaPluginError::FailedVersion there is a JRE at the given location but it does not
   meet the version requirements.
   javaPluginError::NoJre no JRE could be detected at the given location.
 */
javaPluginError jfw_plugin_getJavaInfoByPath(
    OUString const& sLocation,
    jfw::VendorSettings const & vendorSettings,
    std::unique_ptr<JavaInfo> * ppInfo);


/** obtains information for a JRE referenced by the JAVA_HOME environment variable.

   <p>If the JAVA_HOME environment variable is set and points to a JRE that
   matches the requirements given by vendorSettings (i.e.
   the version requirements, if any, for the vendor are met),
   then this function shall return a JavaInfo object for this JRE.</p>

   @param ppInfo
       [out] if the JAVA_HOME environment variable is set and points to a suitable
       JRE, then <code>ppInfo</code> contains
        on return a pointer to its <code>JavaInfo</code> object.

   @return
   javaPluginError::NONE the function ran successfully.</br>
   javaPluginError::NoJre no suitable JRE could be detected at the given location. However, that
   does not mean necessarily that there is no JRE. There could be a JRE but
   it does not
   meet the version requirements.
 */
javaPluginError jfw_plugin_getJavaInfoFromJavaHome(
    jfw::VendorSettings const & vendorSettings,
    std::unique_ptr<JavaInfo> * ppInfo,
    std::vector<rtl::Reference<jfw_plugin::VendorBase>> & infos);


/** obtains information about installations of Java Runtime Environments (JREs)
    whose executable is in the PATH.

    <p>The function gathers information about available JREs which are on the PATH
    (PATH environment variable) and meet the version requirements given by
    <code>vendorSettings</code> (i.e.
    the version requirements, if any, for the vendor are met).
    </p>
    <p>
    The JavaInfo structures returned in <code>vecJavaInfosFromPath</code> should be ordered
    according to their occurrence in the PATH. The one that is the first one on the PATH
    is also the first element in the vector.</p>
    @param vecJavaInfosFromPath
        [out] if the function runs successfully then <code>vecJavaInfosFromPath</code>
        contains on return a vector of pointers to <code>JavaInfo</code> objects.
        On return of this function, <code>vecJavaInfosFromPath</code> references
        a newly created vector rather than the same vector as before with
        the <code>JavaInfo</code> objects inserted into the existing vector.

    @return
    javaPluginError::NONE the function ran successfully and at least one JRE
    that meets the requirements was found.</br>
    javaPluginError::NoJre no JavaInfo that meets the version criteria was found
    when inspecting the PATH
 */

javaPluginError jfw_plugin_getJavaInfosFromPath(
    jfw::VendorSettings const & vendorSettings,
    std::vector<std::unique_ptr<JavaInfo>> & vecJavaInfosFromPath,
    std::vector<rtl::Reference<jfw_plugin::VendorBase>> & infos);

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
    javaPluginError::NONE the function ran successfully.</br>
    javaPluginError::Error an error occurred during execution.</br>
    JFW_PLUGIN_E_VM_CREATION_FAILED a VM could not be created. The error was caused
    by the JRE.
 */
javaPluginError jfw_plugin_startJavaVirtualMachine(
    const JavaInfo *pInfo,
    const JavaVMOption *arOptions,
    sal_Int32 nSizeOptions,
    JavaVM ** ppVM,
    JNIEnv ** ppEnv);


/** checks if the installation of the jre still exists.

    This function checks if the JRE described by pInfo still
    exists. The check must be very quick because it is called by javaldx
    (Linux, Solaris) at start up.

    @param pInfo
        [in]  the JavaInfo object with information about the JRE.
    @param pp_exist
        [out] the parameter is set to either sal_True or sal_False. The value is
        only valid if the function returns JFW_E_NONE.

   @return
    javaPluginError::NONE the function ran successfully.</br>
    javaPluginError::Error an error occurred during execution.</br>
    javaPluginError::InvalidArg pInfo contains invalid data</br>
 */
javaPluginError jfw_plugin_existJRE(const JavaInfo *pInfo, bool *exist);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
