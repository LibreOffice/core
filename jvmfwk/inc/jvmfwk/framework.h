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

#ifndef INCLUDED_JVMFWK_FRAMEWORK_H
#define INCLUDED_JVMFWK_FRAMEWORK_H

#include "jvmfwkdllapi.h"
#include "rtl/ustring.h"
#include "osl/mutex.h"
#ifdef SOLAR_JAVA
#include "jni.h"
#else
struct JavaVMOption;
struct JavaVM;
struct JNIEnv;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** @file
    <p>This library can operate in two modes, application mode and direct mode.</p>


    <h2>Application Mode</h2>
    In application mode the Java related settings are stored in files.
    There are currently three files which need to be accessed. They are determined
    by bootstrap parameters:</p>
    <dl>
    <dt>UNO_JAVA_JFW_VENDOR_SETTINGS</dt>
    <dd>contains vendor and version information about JREs as well as the
    location of plugin-libraries which are responsible for providing information
    about these JREs as well as starting the VMs.</dd>
    <dt>UNO_JAVA_JFW_USER_DATA</dt>
    <dd>The file contains settings for a particular user. One can use the macro
    $SYSUSERCONFIG in the URL which expands to a directory whery the user's data are
    kept. On UNIX this would be the home directory and on Windows some sub-directory
    of the &quot;Documents and Settings&quot; folder.The content of this file is an
    implementation detail and may change in the future.</dd>
    <dt>UNO_JAVA_JFW_SHARED_DATA</dt>
    <dd>The file contains settings valid for all users. If a user changes a setting
    then it takes precedence over the setting from UNO_JAVA_JFW_SHARED_DATA.
    The content of this file is an implementation detail and may change in the future.</dd>
    </dl>

    <p>The values for these parameters must be file URLs and include the file name, for
    example:<br>
    file:///d:/MyApp/javavendors.xml<br>
    All files are XML files and must have the extension .xml.</p>
    <p>
    Modifying the shared settings is currently not supported by the framework. To provide
    Java settings for all users one can run OOo and change the settings in the
    options dialog. These settings are made persistent in the UNO_JAVA_JFW_USER_DATA.
    The file can then be copied into the base installation.
    Other users will use automatically these data but can override the settings in
    the options dialog. This mechanism may change in the future.
    </p>
    <p>If shared Java settings are not supported by an application then it is not
    necessary to specify the bootstrap parameter <code>UNO_JAVA_JFW_SHARED_DATA</code>.
    </p>

    <p>Setting the class path used by a Java VM should not be necesarry. The locations
    of Jar files should be knows by a class loader. If a jar file depends on another
    jar file then it can be referenced in the manifest file of the first jar. However,
    a user may add jars to the class path by using this API. If it becomes necessary
    to add files to the class path which is to be used by all users then one can use
    the bootrap parameter UNO_JAVA_JFW_CLASSPATH_URLS. The value contains of file URLs
    which must be separated by spaces.</p>


    <h2>Direct Mode</h2>

    <p>The direct mode is intended for a scenario where no configuration files
    are available and a Java VM  shall be run. That is,
    the files containing the user and shared settings are not specified by the
    bootstrap parameters UNO_JAVA_JFW_SHARED_DATA and UNO_JAVA_JFW_USER_DATA.
    For example, tools, such as regcomp, may use this  framework in a build
    environment. Then one would want to use settings which have been specified
    by the build environment. The framework would automatically use the
    current settings when they change in the environment.
    </p>

    <p> Here are examples how regcomp could be invoked using bootstrap parameters:
    </p>
    <p>
    regcomp -env:UNO_JAVA_JFW_JREHOME=file:///d:/j2re1.4.2
    -env:&quot;UNO_JAVA_JFW_CLASSPATH=d:\\solver\\bin\\classes.jar;d:\\solver\\bin\\jurt.jar&quot;
    -register ....
    </p>
    <p>If UNO_JAVA_JFW_VENDOR_SETTINGS is not set then a plugin library must be specified. For example:</p>
    <p>
    regcomp -env:UNO_JAVA_JFW_JREHOME=file:///d:/j2re1.4.2
    -env:&quot;UNO_JAVA_JFW_CLASSPATH=d:\\solver\\bin\\classes.jar;d:\\solver\\bin\\jurt.jar&quot;
    -env:UNO_JAVA_JFW_PLUGIN=file:\\solver\\bin\\libsunjavaplugin.dll -register ....
    </p>
    <p>Additionall parameters for the Java VM can be provided. For every parameter
    a seperate bootstrap parameter must be specified. The names are
    <code>UNO_JAVA_JFW_PARAMETER_X</code>, where X is 1,2, .. n. For example:</p>
    <p>
    regcomp -env:UNO_JAVA_JFW_PARAMETER_1=-Xdebug
    -env:UNO_JAVA_JFW_PARAMETER_2=-Xrunjdwp:transport=dt_socket,server=y,address=8100
    -env:UNO_JAVA_JFW_JREHOME=file:///d:/j2re1.4.2
    -env:&quot;UNO_JAVA_JFW_CLASSPATH=d:\\solver\\bin\\classes.jar;d:\\solver\\bin\\jurt.jar&quot;
    -register ....</p>
    <p>
    Here is a complete list of the bootstrap parameter for the direct mode:
    </p>
    <dl>
    <dt>UNO_JAVA_JFW_JREHOME</dt>
    <dd>Specifies a file URL to a JRE installation.It must ALWAYS be specified
    in direct mode</dd>
    <dt>UNO_JAVA_JFW_ENV_JREHOME</dt>
    <dd>Setting this parameter, for example to &quot;1&quot; or &quot;true&quot;,
    causes the framework to use the environment variable JAVA_HOME. It is expected
    that JAVA_HOME contains a system path rather than a file URL. This parameter
    and UNO_JAVA_JFW_JREHOME are mutually exclusive</dd>
    <dt>UNO_JAVA_JFW_CLASSPATH</dt>
    <dd>Contains the class path which is to be used by the VM. Special character,
    such as '\','{','}','$' must be preceded with '\'. See documentation about the
    bootstrap parameter.</dd>
    <dt>UNO_JAVA_JFW_ENV_CLASSPATH</dt>
    <dd>Setting this parameter,for example to &quot;1&quot; or &quot;true&quot;,
    causes the framework to use the
    environment variable CLASSPATH. If this variable and UNO_JAVA_JFW_CLASSPATH are
    set then the class path is composed from UNO_JAVA_JFW_CLASSPATH and the environment
    variable CLASSPATH.</dd>
    <dt>UNO_JAVA_JFW_PLUGIN</dt>
    <dd>Specified a file URL to a plugin library. If this variable is provided
    then a javavendors.xml is ignored. It must be provided if no
    javavendors.xml is available.</dd>
    <dt>UNO_JAVA_JFW_PARAMETER_X</dt>
    <dd>Specifies a parameter for the Java VM. The X is replaced by
    non-negative natural numbers starting with 1.</dd>
    </dl>

    <p>A note about bootstrap parameters. The implementation of the bootstrap
    parameter mechanism interprets the characters '\', '$', '{', '}' as
    escape characters. Thats why the Windows path contain double back-slashes.
    One should also take into account that a console may have also special
    escape characters.</p>

    <h2>What mode is used</h2>
    <p>
    The default mode is application mode. If at least one bootstrap parameter
    for the direct mode is provided then direct mode is used. </p>

    <p>
    All settings made by this API are done for the current user if not
    mentioned differently.</p>

    <h2>Other bootstrap variables</h2>
    <dl>
    <dt>JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY</dt>
    <dd>This is a unofficial variable which was introduced to workaround external issues.
    It may be removed in the future. By setting it to 1, the framework will not try to
    find out if the system is configured to use accessibility tools or if a JRE has an
    accessibiliy bridge installed</dd>
    </dl>
*/

/** indicates that a JRE has an accessibility bridge installed.
    <p>
    The flag is used with JavaInfo::nFeatures.</p>
 */
#define JFW_FEATURE_ACCESSBRIDGE 0x1l
/** indicates that there must be an environment set up before the Java process
    runs.
    <p>Therefore, when a Java is selected in OO then the office must be
    restarted, so that the changes can take effect.</p>
 */
#define JFW_REQUIRE_NEEDRESTART  0x1l

/** error codes which are returned by functions of this API.
 */
typedef enum _javaFrameworkError
{
    JFW_E_NONE,
    JFW_E_ERROR,
    JFW_E_INVALID_ARG,
    JFW_E_NO_SELECT,
    JFW_E_INVALID_SETTINGS,
    JFW_E_NEED_RESTART,
    JFW_E_RUNNING_JVM,
    JFW_E_JAVA_DISABLED,
    JFW_E_NO_PLUGIN,
    JFW_E_NOT_RECOGNIZED,
    JFW_E_FAILED_VERSION,
    JFW_E_NO_JAVA_FOUND,
    JFW_E_VM_CREATION_FAILED,
    JFW_E_CONFIGURATION,
    JFW_E_DIRECT_MODE
} javaFrameworkError;

/** an instance of this struct represents an installation of a Java
    Runtime Environment (JRE).

    <p>
    Instances of this struct are created by the plug-in libraries which are used by
    this framework (jvmfwk/vendorplugin.h). The memory of the instances is created
    by <code>rtl_allocateMemory</code> (rtl/alloc.h). Therefore, the memory must
    be freed by <code>rtl_freeMemory</code>. Also the contained members must be
    freed particularly.
    For convenience this API provides the function <code>jfw_freeJavaInfo</code>
    which frees the objects properly. </p>
 */
struct _JavaInfo
{
    /** contains the vendor.

        <p>string must be the same as the one obtained from the
        Java system property <code>java.vendor</code>.
        </p>
     */
    rtl_uString *sVendor;
    /** contains the file URL to the installation directory.
    */
    rtl_uString *sLocation;
    /** contains the version of this Java distribution.

        <p>The version string  must adhere to the rules
        about how a version string has to be formed. These rules may
        be vendor-dependent. Essentially the strings must syntactically
        equal the Java system property <code>java.version</code>.
        </p>
    */
    rtl_uString *sVersion;
    /** indicates supported special features.

        <p>For example, <code>JFW_FEATURE_ACCESSBRIDGE</code> indicates that
        assistive technology tools are supported.</p>
     */
    sal_uInt64 nFeatures;
    /** indicates requirments for running the java runtime.

        <p>For example, it may be necessary to prepare the environment before
        the runtime is created. That could mean, setting the
        <code>LD_LIBRARY_PATH</code>
        when <code>nRequirements</code> contains the flag
        <code>JFW_REQUIRE_NEEDRESTART</code></p>
     */
    sal_uInt64 nRequirements;
    /** contains data needed for the creation of the java runtime.

        <p>There is no rule about the format and content of the sequence's
        values. The plug-in libraries can put all data, necessary for
        starting the java runtime into this sequence. </p>
     */
    sal_Sequence * arVendorData;
};

typedef struct _JavaInfo JavaInfo;

/** frees the memory of a <code>JavaInfo</code> object.
    @param pInfo
    The object which is to be freed. It can be NULL;
 */
JVMFWK_DLLPUBLIC void SAL_CALL jfw_freeJavaInfo(JavaInfo *pInfo);


/** compares two <code>JavaInfo</code> objects for equality.

   <p>Two <code>JavaInfo</code> objects are said to be equal if the contained
   members of the first <code>JavaInfo</code> are equal to their counterparts
   in the second <code>JavaInfo</code> object. The equality of the
   <code>rtl_uString</code> members is determined
   by the respective comparison function (see
   <code>rtl::OUString::equals</code>).
   Similiarly the equality of the <code>sal_Sequence</code> is
   also determined by a comparison
   function (see <code>rtl::ByteSequence::operator ==</code>). </p>
   <p>
   Both argument pointers  must be valid.</p>
   @param pInfoA
   the first argument.
   @param pInfoB
   the second argument which is compared with the first.
   @return
   sal_True - both object represent the same JRE.</br>
   sal_False - the objects represend different JREs
 */
JVMFWK_DLLPUBLIC sal_Bool SAL_CALL jfw_areEqualJavaInfo(
    JavaInfo const * pInfoA,JavaInfo const * pInfoB);

/** determines if a Java Virtual Machine is already running.

    <p>As long as the the office and the JREs only support one
    Virtual Machine per process the Java settings, particulary the
    selected Java, are not effective immediatly after changing when
    a VM has already been running. That is, if a JRE A was used to start
    a VM and then a JRE B is selected, then JRE B will only be used
    after a restart of the office.</p>
    <p>
    By determining if a VM is running, the user can be presented a message,
    that the changed setting may not be effective immediately.</p>

    @param bRunning
    [out] sal_True - a VM is running. <br/>
    sal_False - no VM is running.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_INVALID_ARG the parameter <code>bRunning</code> was NULL.
*/
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_isVMRunning(sal_Bool *bRunning);

/** detects a suitable JRE and configures the framework to use it.

    <p>Which JREs can be used is determined by the file javavendors.xml,
    which contains version requirements, as well as information about available
    plug-in libraries. Only these libraries are responsible for locating JRE
    installations.</p>
    <p>
    JREs can be provided by different vendors. In order to find the JREs of
    a certain vendor a plug-in library must be provided. There must be only one
    library for one vendor. The names of locations of those libraries have to
    be put into the javavendors.xml file.<br/>
    The function uses the plug-in libraries to obtain information about JRE
    installation and checks if they there is one among them that supports
    a set of features (currently only accessibilty is possible). If none was
    found then it also uses a list of paths, which have been registered
    by <code>jfw_addJRELocation</code> or <code>jfw_setJRELocations</code>
    to find JREs. Found JREs are examined in the same way.</p>
    <p>
    A JRE installation is only selected if it meets the version requirements.
    Information about the selected JRE are made persistent so that
    subsequent calls to <code>jfw_getSelectedJRE</code> returns this
    information.</p>
    <p>
    While determining a proper JRE this function takes into account if a
    user requires support for assistive technology tools. If user
    need that support they have to set up their system accordingly. When support
    for assistive technology is required, then the lists of
    <code>JavaInfo</code> objects,
    which are provided by the <code>getJavaInfo</code> functions of the plug-ins, are
    examined for a suitable JRE. That is, the <code>JavaInfo</code> objects
    from the list
    obtained from the first plug-in, are examined. If no <code>JavaInfo</code>
    object has the flag
    <code>JFW_FEATURE_ACCESSBRIDGE</code> in the member <code>nFeatures</code>
    then the
    next plug-in is used to obtain a list of <code>JavaInfo</code> objects.
    This goes on until a <code>JavaInfo</code> object was found which
    represents a suitable JRE. Or neither plug-in provided such a
    <code>JavaInfo</code> object. In that case the first
    <code>JavaInfo</code> object from the first plug-in is used to determine
    the JRE which is to be used.</p>
    <p>
    If there is no need for the support of assistive technology tools then
    the first <code>JavaInfo</code> object from the list obtained by the
    first plug-in is used. If this plug-in does not find any JREs then the
    next plug-in is used, and so on.</p>

    @param ppInfo
    [out] a <code>JavaInfo</code> pointer, representing the selected JRE.
    The caller has to free it by calling <code>jfw_freeJavaInfo<code>. The
    <code>JavaInfo</code> is for informational purposes only. It is not
    necessary to call <code>jfw_setSelectedJRE</code> afterwards.<br/>
    <code>ppInfo</code>can be NULL. If <code>*ppInfo</code> is not null, then it is
    overwritten, without attempting to free <code>*ppInfo</code>.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_ERROR an error occurred. <br/>
    JFW_E_NO_PLUGIN a plug-in library could not be found.<br/>
    JFW_E_NO_JAVA_FOUND no JRE was found that meets the requirements.</br>
    JFW_E_DIRECT_MODE the function cannot be used in this mode. </br>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_findAndSelectJRE(JavaInfo **pInfo);

/** provides information about all availabe JRE installations.

    <p>The function determines dynamically what JREs are available. It uses
    the plug-in libraries to provide lists of available <code>JavaInfo</code>
    objects where each object represents a JRE (see vendorplugin.h,
    getAllJavaInfos). It also uses a list of paths, which have been registered
    by <code>jfw_addJRELocation</code> or <code>jfw_setJRELocations</code>.
    It is checked if the path still contains a valid JRE and if so the respective
    <code>JavaInfo</code> object will be appended to the array unless there is
    already an equal object.</p>

    @param parInfo
    [out] on returns it contains a pointer to an array of <code>JavaInfo</code>
    pointers.
    The caller must free the array with <code>rtl_freeMemory</code> and each
    element of the array must be freed with <code>jfw_freeJavaInfo</code>.
    @param pSize
    [out] on return contains the size of array returned in <code>parInfo</code>.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_INVALID_ARG at least on of the parameters was NULL<br/>
    JFW_E_ERROR an error occurred. <br/>
    JFW_E_NO_PLUGIN a plug-in library could not be found.<br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.
*/
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_findAllJREs(
    JavaInfo ***parInfo, sal_Int32 *pSize);

/** determines if a path points to a Java installation.

   <p>If the path belongs to a JRE installation then it returns the
   respective <code>JavaInfo</code> object. The function uses the
   <code>getJavaInfoByPath</code> function of the plug-ins to obtain the
   <code>JavaInfo</code> object. Only if the JRE found at the specified location
   meets the version requirements as specified in the javavendors.xml file a
   <code>JavaInfo</code> object is returned.<br/>
   <p>
   The functions only checks if a JRE exists but does not modify any settings.
   To make the found JRE the &quot;selected JRE&quot; one has
   to call <code>jfw_setSelectedJRE</code>.</p>

   @param pPath
   [in] a file URL to a directory.
   @param pInfo
   [out] the <code>JavaInfo</code> object which represents a JRE found at the
   location specified by <code>pPath</code>

   @return
   JFW_E_NONE function ran successfully.<br/>
   JFW_E_INVALID_ARG at least on of the parameters was NULL<br/>
   JFW_E_ERROR an error occurred. <br/>
   JFW_E_CONFIGURATION mode was not properly set or their prerequisites
   were not met.</br>
   JFW_E_NO_PLUGIN a plug-in library could not be found.<br/>
   JFW_E_NOT_RECOGNIZED neither plug-in library could detect a JRE. <br/>
   JFW_E_FAILED_VERSION a JRE was detected but if failed the version
   requirements as determined by the javavendors.xml
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_getJavaInfoByPath(
    rtl_uString *pPath, JavaInfo **ppInfo);


/** starts a Java Virtual Machine (JVM).

    <p>The function uses the current settings to start a JVM. The actual
    start-up code, however, is provided by the plug-in libraries. The setting
    of the &quot;selected Java&quot; contains the information as to what vendor
    the respective JRE comes from. In the javavendors.xml there is a mapping of
    vendor names to the respective plug-in libraries.</p>
    <p>
    The function ultimately calls <code>startJavaVirtualMachine</code> from
    the plug-in library.</p>
    <p>
    The <code>arOptions</code>
    argument contains start arguments which are passed in JavaVMOption structures
    to the VM during its creation. These
    could be things, such as language settings, proxy settings or any other
    properties which shall be obtainable by
    <code>java.lang.System.getProperties</code>. One can also pass options which
    have a certain meaning to the runtime behaviour such as -ea or -X... However,
    one must be sure that these options can be interpreted by the VM.<br/>
    The class path cannot be set this way. The class path is internally composed by
    the paths to archives in a certain directory, which is preconfigured in
    the internal data store and the respective user setting (see
    <code>jfw_setUserClassPath</code>.</p>
    <p>
    If a JRE was selected at runtime which was different from the previous
    setting and that JRE needs a prepared environment, for example an adapted
    <code>LD_LIBRARY_PATH</code> environment variable, then the VM will not be
    created and JFW_E_NEED_RESTART error is returned. If a VM is already running
    then a JFW_E_RUNNING_JVM is returned.</p>

    @param arOptions
    [in] the array containing additional start arguments or NULL.
    @param nSize
    [in] the size of the array <code>arOptions</code>.
    @param ppVM
    [out] the <code>JavaVM</code> pointer.
    @param ppEnv
    [out] the <code>JNIenv</code> pointer.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_INVALID_ARG <code>ppVM</code>, <code>ppEnv</code> are NULL or
    <code>arOptions</code> was NULL but <code>nSize</code> was greater 0.<br/>
    JFW_E_ERROR an error occurred. <br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.</br>
    JFW_E_NO_PLUGIN the plug-in library responsible for creating the VM
    could not be found.<br/>
    JFW_E_JAVA_DISABLED the use of Java is currently disabled. <br/>
    JFW_E_NO_SELECT there is no JRE selected yet. <br/>
    JFW_E_RUNNIN_JVM there is already a VM running.<br/>
    JFW_E_INVALID_SETTINGS the javavendors.xml has been changed and no
    JRE has been selected afterwards. <br/>
    JFW_E_NEED_RESTART in the current process a different JRE has been selected
    which needs a prepared environment, which has to be done before the office
    process. Therefore the new JRE may not be used until the office was restarted.<br/>
    JFW_E_NEED_RESTART is also returned when Java was disabled at the beginning and
    then the user enabled it. If then the selected  JRE has the requirement
    JFW_REQUIRE_NEEDRESTART then this error is returned. </br>
    JFW_E_VM_CREATION_FAILED the creation of the JVM failed. The creation is performed
    by a plug-in library and not by this API.
    JFW_E_FAILED_VERSION the &quot;Default Mode&quot; is active. The JRE determined by
    <code>JAVA_HOME</code>does not meet the version requirements.
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_startVM(JavaVMOption *arOptions,
                                 sal_Int32 nSize, JavaVM **ppVM,
                                 JNIEnv **ppEnv);

/** determines the JRE that is to be used.

    <p>When calling <code>jfw_startVM</code> then a VM is startet from
    the JRE that is determined by this function.<br/>
    It is not verified if the JRE represented by the <code>JavaInfo</code>
    argument meets the requirements as specified by the javavendors.xml file.
    However, usually one obtains the <code>JavaInfo</code> object from the
    functions <code>jfw_findAllJREs</code> or <code>jfw_getJavaInfoByPath</code>,
    which do verify the JREs and pass out only <code>JavaInfo</code> objects
    which comply with the version requirements.</p>
    <p>
    If <code>pInfo</code> is NULL then the meaning is that no JRE will be
    selected. <code>jfw_startVM</code> will then return
    <code>JFW_E_NO_SELECT</code>.</p>

    @param pInfo
      [in] pointer to <code>JavaInfo</code> structure, containing data about a
      JRE. The caller must still free <code>pInfo</code>.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_ERROR An error occurred.<br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
    JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_setSelectedJRE(JavaInfo const *pInfo);


/** provides information about the JRE that is to be used.

    <p>If no JRE is currently selected then <code>ppInfo</code> will contain
    NULL on return.</br>
    If the value of the element <updated> in the javavendors.xml file was
    changed since the time when the last Java was selected then this
    function returns <code>JFW_E_INVALID_SETTINGS</code>. This could happen during
    a product patch. Then new version requirements may be introduced, so that
    the currently selected JRE may not meet these requirements anymore.
    </p>
    <p>In direct mode the function returns information about a JRE that was
    set by the bootstrap parameter UNO_JAVA_JFW_JREHOME.
    </p>
    @param ppInfo
    [out] on return it contains a pointer to a <code>JavaInfo</code> object
    that represents the currently selected JRE. When <code>*ppInfo</code> is not
    NULL then the function overwrites the pointer. It is not attempted to free
    the pointer.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_INVALIDARG <code>ppInfo</code> is a NULL.<br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
    JFW_E_INVALID_SETTINGS the javavendors.xml has been changed and no
    JRE has been selected afterwards. <br/>
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_getSelectedJRE(JavaInfo **ppInfo);


/** determines if Java can be used.

   <p>If <code>bEnabled</code> is <code>sal_False</code> then a call
   to jfw_startVM will result in an error with the errorcode
   <code>JFW_E_JAVA_DISABLED</code></p>

   @param bEnabled
   [in] use of Java enabled/disabled.

   @return
   JFW_E_NONE function ran successfully.<br/>
   JFW_E_ERROR An error occurred.<br/>
   JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
   JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_setEnabled(sal_Bool bEnabled);

/** provides the information if Java can be used.

    <p>That is if the user enabled or disabled the use of Java.
    </p>

   @return
   JFW_E_NONE function ran successfully.<br/>
   JFW_E_INVALIDARG pbEnabled is NULL<br/>
   JFW_E_ERROR An error occurred.<br/>
   JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
    JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_getEnabled(sal_Bool *pbEnabled);

/** determines parameters which are passed to VM during its creation.

    <p>The strings must be exactly as they are passed on the command line.
    For example, one could pass<br/>
    -Xdebug <br/>
    -Xrunjdw:transport=dt_socket,server=y,address=8000<br/>
    in order to enable debugging support.
    </p>

    @param arParameters
    [in] contains the arguments. It can be NULL if nSize is 0.
    @param nSize
    [i] the size of <code>arArgs</code>

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_INVALIDARG arArgs is NULL and nSize is not 0
    JFW_E_ERROR An error occurred.<br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
    JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_setVMParameters(
    rtl_uString **  arArgs, sal_Int32 nSize);

/** obtains the currently used start parameters.

    <p>The caller needs to free the returned array with
    <code>rtl_freeMemory</code>. The contained strings must be released with
    <code>rtl_uString_release</code>.
    </p>

    @param parParameters
    [out] on returns contains a pointer to the array of the start arguments.
    If *parParameters is not NULL then the value is overwritten.
    @param pSize
    [out] on return contains the size of array returned in
    <code>parParameters</code>

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_INVALIDARG parParameters or pSize are  NULL<br/>
    JFW_E_ERROR An error occurred.<br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
    JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_getVMParameters(
    rtl_uString *** parParameters,
    sal_Int32 * pSize);

/** sets the user class path.

   <p>When the VM is started then it is passed the class path. The
   class path also contains the user class path set by this function.
   The paths contained in <code>pCP</code> must be separated with a
   system dependent path separator.</p>

   @param pCP
   [in] the user class path.

   @return
   JFW_E_NONE function ran successfully.<br/>
   JFW_E_INVALIDARG pCP is NULL.<br/>
   JFW_E_ERROR An error occurred.<br/>
   JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
   JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_setUserClassPath(rtl_uString * pCP);
/** provides the value of the current user class path.

   <p>The function returns an empty string if no user class path is set.
   </p>

   @param ppCP
   [out] contains the user class path on return. If <code>*ppCP</code> was
   not NULL then the value is overwritten. No attempt at freeing that string
   is made.

   @return
   JFW_E_NONE function ran successfully.<br/>
   JFW_E_INVALIDARG ppCP is NULL.<br/>
   JFW_E_ERROR An error occurred.<br/>
   JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
   JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_getUserClassPath(rtl_uString ** ppCP);

/** saves the location of a JRE.

    <p>When <code>jfw_findAllJREs</code> is called then the paths added by this
    function are evaluated. If the location still represents a
    JRE then a <code>JavaInfo</code> object is created which is returned along
    with all other <code>JavaInfo</code> objects by
    <code>jfw_findAllJREs</code>. If the location
    cannot be recognized then the location string is ignored. </p>
    <p>
    A validation if <code>sLocation</code> points to a JRE is not
    performed. To do that one has to use <code>jfw_getJavaInfoByPath</code>.
    </p>
    <p>
    Adding a path that is already stored causes no error.</p>

    @param sLocation
    [in] file URL to a directory which contains a JRE.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_INVALIDARG sLocation is NULL.<br/>
    JFW_E_ERROR An error occurred.<br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
    JFW_E_DIRECT_MODE the function cannot be used in this mode.
    @see jfw_setJRELocations
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_addJRELocation(rtl_uString * sLocation);

/** saves the locations of a number of JREs.

    <p>
    The function does not verify if the paths points to JRE. However,
    it makes sure that every path is unique. That is, if the array
    contains strings which are the same then only one is stored.</p>
    <p>
    If <code>arLocations</code> is NULL or it has the length null (nSize = 0)
    then all previously stored paths are deleted. Otherwise,
    the old values are overwritten.</p>

    @param arLocations
    [in] array of paths to locations of JREs.

    @param nSize
    [in] the size of the array <code>arLocations</code>

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_INVALIDARG arLocation is NULL and nSize is not null.<br/>
    JFW_E_ERROR An error occurred.<br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
    JFW_E_DIRECT_MODE the function cannot be used in this mode.
    @see jfw_addJRELocations
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_setJRELocations(
    rtl_uString ** arLocations, sal_Int32 nSize);
/** obtains an array containing paths to JRE installations.

    <p>
    It is not guaranteed that the returned paths represent
    a valid JRE. One can use <code>jfw_getJavaInfoByPath</code> to check this.
    </p>

    @param parLocations
    [out] on return it contains the array of paths.
    @param pSize
    [out] on return it contains the size of the array <code>parLocations</code>.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_INVALIDARG parLocation is NULL or pSize is NULL.<br/>
    JFW_E_ERROR An error occurred.<br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
    JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_getJRELocations(
    rtl_uString *** parLocations, sal_Int32 * pSize);


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
    JFW_E_NONE the function ran successfully.</br>
    JFW_E_ERROR an error occurred during execution.</br>
    JFW_E_INVALID_ARG pInfo contains invalid data</br>
    JFW_E_NO_PLUGIN a plug-in library could not be found.<br/>
 */
JVMFWK_DLLPUBLIC javaFrameworkError SAL_CALL jfw_existJRE(const JavaInfo *pInfo, sal_Bool *exist);


/** locks this API so that it cannot be used by other threads.

    <p>If a different thread called this function before then the
    current call is blocked until the other thread has called
    <code>jfw_unlock()</code>. The function should be called if one
    needs an exact snapshot of the current settings. Then the settings
    are retrieved one by one without risk that the settings may be changed
    by a different thread. Similiary if one needs to make settings which
    should become effective at the same time then <code>jfw_lock</code>
    should be called. That is, <code>jfw_startVM</code> which uses the
    settings cannot be called before all settings have be made.</p>
    <p>
    The only functions which are not effected by <code>jfw_lock</code> are
    <code>jfw_freeJavaInfo</code> and <code>jfw_areEqualJavaInfo</code>.
 */
JVMFWK_DLLPUBLIC void SAL_CALL jfw_lock();

/** unlocks this API.

    <p>This function is called after <code>jfw_lock</code>. It allows other
    threads to use this API concurrently.</p>
*/
JVMFWK_DLLPUBLIC void SAL_CALL jfw_unlock();


#ifdef __cplusplus
}
#endif


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
