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

#ifndef INCLUDED_JVMFWK_FRAMEWORK_HXX
#define INCLUDED_JVMFWK_FRAMEWORK_HXX

#include <sal/config.h>

#include <memory>
#include <vector>

#include <jvmfwk/jvmfwkdllapi.hxx>
#include <rtl/byteseq.hxx>
#include <rtl/ustring.hxx>
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"
#endif
#include <jni.h>
#if defined __clang__
#pragma clang diagnostic pop
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
    $SYSUSERCONFIG in the URL which expands to a directory where the user's data are
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

    <p>Setting the class path used by a Java VM should not be necessary. The locations
    of Jar files should be known by a class loader. If a jar file depends on another
    jar file then it can be referenced in the manifest file of the first jar. However,
    a user may add jars to the class path by using this API. If it becomes necessary
    to add files to the class path which is to be used by all users then one can use
    the bootstrap parameter UNO_JAVA_JFW_CLASSPATH_URLS. The value contains of file URLs
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
    -env:&quot;UNO_JAVA_JFW_CLASSPATH=d:\\solver\\bin\\classes.jar;d:\\solver\\bin\\libreoffice.jar&quot;
    -register...
    </p>
    <p>If UNO_JAVA_JFW_VENDOR_SETTINGS is not set then a plugin library must be specified. For example:</p>
    <p>
    regcomp -env:UNO_JAVA_JFW_JREHOME=file:///d:/j2re1.4.2
    -env:&quot;UNO_JAVA_JFW_CLASSPATH=d:\\solver\\bin\\classes.jar;d:\\solver\\bin\\libreoffice.jar&quot;
    -register...
    </p>
    <p>Additional parameters for the Java VM can be provided. For every parameter
    a separate bootstrap parameter must be specified. The names are
    <code>UNO_JAVA_JFW_PARAMETER_X</code>, where X is 1,2, .. n. For example:</p>
    <p>
    regcomp -env:UNO_JAVA_JFW_PARAMETER_1=-Xdebug
    -env:UNO_JAVA_JFW_PARAMETER_2=-Xrunjdwp:transport=dt_socket,server=y,address=8100
    -env:UNO_JAVA_JFW_JREHOME=file:///d:/j2re1.4.2
    -env:&quot;UNO_JAVA_JFW_CLASSPATH=d:\\solver\\bin\\classes.jar;d:\\solver\\bin\\libreoffice.jar&quot;
    -register...</p>
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
    <dt>UNO_JAVA_JFW_PARAMETER_X</dt>
    <dd>Specifies a parameter for the Java VM. The X is replaced by
    non-negative natural numbers starting with 1.</dd>
    </dl>

    <p>A note about bootstrap parameters. The implementation of the bootstrap
    parameter mechanism interprets the characters '\', '$', '{', '}' as
    escape characters. That's why the Windows path contain double back-slashes.
    One should also take into account that a console may have also special
    escape characters.</p>

    <h2>What mode is used</h2>
    <p>
    The default mode is application mode. If at least one bootstrap parameter
    for the direct mode is provided then direct mode is used. </p>

    <p>
    All settings made by this API are done for the current user if not
    mentioned differently.</p>
*/

/** indicates that there must be an environment set up before the Java process
    runs.
    <p>Therefore, when a Java is selected in OO then the office must be
    restarted, so that the changes can take effect.</p>
 */
#define JFW_REQUIRE_NEEDRESTART 0x1l

/** error codes which are returned by functions of this API.
 */
enum javaFrameworkError
{
    JFW_E_NONE,
    JFW_E_ERROR,
    JFW_E_NO_SELECT,
    JFW_E_INVALID_SETTINGS,
    JFW_E_NEED_RESTART,
    JFW_E_RUNNING_JVM,
    JFW_E_JAVA_DISABLED,
    JFW_E_NOT_RECOGNIZED,
    JFW_E_FAILED_VERSION,
    JFW_E_NO_JAVA_FOUND,
    JFW_E_VM_CREATION_FAILED,
    JFW_E_CONFIGURATION,
    JFW_E_DIRECT_MODE
};

/** an instance of this struct represents an installation of a Java
    Runtime Environment (JRE).

    <p>
    Instances of this struct are created by the plug-in libraries which are used by
    this framework (jvmfwk/vendorplugin.h).</p>
 */
struct JavaInfo
{
    /** contains the vendor.

        <p>string must be the same as the one obtained from the
        Java system property <code>java.vendor</code>.
        </p>
     */
    OUString sVendor;
    /** contains the file URL to the installation directory.
    */
    OUString sLocation;
    /** contains the version of this Java distribution.

        <p>The version string  must adhere to the rules
        about how a version string has to be formed. These rules may
        be vendor-dependent. Essentially the strings must syntactically
        equal the Java system property <code>java.version</code>.
        </p>
    */
    OUString sVersion;
    /** indicates requirements for running the java runtime.

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
    rtl::ByteSequence arVendorData;
};

/** compares two <code>JavaInfo</code> objects for equality.

   <p>Two <code>JavaInfo</code> objects are said to be equal if the contained
   members of the first <code>JavaInfo</code> are equal to their counterparts
   in the second <code>JavaInfo</code> object. The equality of the
   <code>OUString</code> members is determined
   by <code>operator ==</code>.
   Similarly the equality of the <code>rtl::ByteSequence</code> is
   also determined by a comparison
   function (see <code>rtl::ByteSequence::operator ==</code>). </p>
   <p>
   Both argument pointers  must be valid.</p>
   @param pInfoA
   the first argument.
   @param pInfoB
   the second argument which is compared with the first.
   @return
   true - both object represent the same JRE.</br>
   false - the objects represent different JREs
 */
JVMFWK_DLLPUBLIC bool jfw_areEqualJavaInfo(JavaInfo const* pInfoA, JavaInfo const* pInfoB);

/** determines if a Java Virtual Machine is already running.

    <p>As long as the office and the JREs only support one
    Virtual Machine per process the Java settings, particularly the
    selected Java, are not effective immediately after changing when
    a VM has already been running. That is, if a JRE A was used to start
    a VM and then a JRE B is selected, then JRE B will only be used
    after a restart of the office.</p>
    <p>
    By determining if a VM is running, the user can be presented a message,
    that the changed setting may not be effective immediately.</p>

    @return
    true iff a VM is running.
*/
JVMFWK_DLLPUBLIC bool jfw_isVMRunning();

/** detects a suitable JRE and configures the framework to use it.

    <p>Which JREs can be used is determined by the file javavendors.xml,
    which contains version requirements.</p>
    <p>
    JREs can be provided by different vendors.
    The function obtains information about JRE installations. If none was
    found then it also uses a list of paths, which have been registered
    by <code>jfw_addJRELocation</code>
    to find JREs. Found JREs are examined in the same way.</p>
    <p>
    A JRE installation is only selected if it meets the version requirements.
    Information about the selected JRE are made persistent so that
    subsequent calls to <code>jfw_getSelectedJRE</code> returns this
    information.</p>
    <p>
    While determining a proper JRE this function takes into account if a
    user requires support for assistive technology tools. If user
    need that support they have to set up their system accordingly.</p>
    <p>
    If the JAVA_HOME environment variable is set, this function prefers
    the JRE which the variable refers to over other JREs.
    If JAVA_HOME is not set or does not refer to a suitable JRE,
    the PATH environment variable is inspected and the respective JREs
    are checked for their suitability next.</p>
    <p>
    The first <code>JavaInfo</code> object that is detected by the algorithm
    as described above is used.</p>

    @param pInfo
    [out] a <code>JavaInfo</code> pointer, representing the selected JRE.
    The <code>JavaInfo</code> is for informational purposes only. It is not
    necessary to call <code>jfw_setSelectedJRE</code> afterwards.<br/>
    <code>pInfo</code>can be NULL.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_ERROR an error occurred. <br/>
    JFW_E_NO_JAVA_FOUND no JRE was found that meets the requirements.</br>
    JFW_E_DIRECT_MODE the function cannot be used in this mode. </br>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.
 */
JVMFWK_DLLPUBLIC javaFrameworkError jfw_findAndSelectJRE(std::unique_ptr<JavaInfo>* pInfo);

/** provides information about all available JRE installations.

    <p>The function determines dynamically what JREs are available. It uses
    the plug-in libraries to provide lists of available <code>JavaInfo</code>
    objects where each object represents a JRE (see vendorplugin.h,
    getAllJavaInfos). It also uses a list of paths, which have been registered
    by <code>jfw_addJRELocation</code>.
    It is checked if the path still contains a valid JRE and if so the respective
    <code>JavaInfo</code> object will be appended to the array unless there is
    already an equal object.</p>

    @param parInfo
    [out] on returns it contains a vector of <code>JavaInfo</code> pointers.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_ERROR an error occurred. <br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.
*/
JVMFWK_DLLPUBLIC javaFrameworkError
jfw_findAllJREs(std::vector<std::unique_ptr<JavaInfo>>* parInfo);

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
   @param ppInfo
   [out] the <code>JavaInfo</code> object which represents a JRE found at the
   location specified by <code>pPath</code>

   @return
   JFW_E_NONE function ran successfully.<br/>
   JFW_E_ERROR an error occurred. <br/>
   JFW_E_CONFIGURATION mode was not properly set or their prerequisites
   were not met.</br>
   JFW_E_NOT_RECOGNIZED neither plug-in library could detect a JRE. <br/>
   JFW_E_FAILED_VERSION a JRE was detected but if failed the version
   requirements as determined by the javavendors.xml
 */
JVMFWK_DLLPUBLIC javaFrameworkError jfw_getJavaInfoByPath(OUString const& pPath,
                                                          std::unique_ptr<JavaInfo>* ppInfo);

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

    @param pInfo
    [in] optional pointer to a specific JRE; must be caller-freed if not NULL
    @param arOptions
    [in] the vector containing additional start arguments.
    @param ppVM
    [out] the <code>JavaVM</code> pointer.
    @param ppEnv
    [out] the <code>JNIenv</code> pointer.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_ERROR an error occurred. <br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.</br>
    JFW_E_JAVA_DISABLED the use of Java is currently disabled. <br/>
    JFW_E_NO_SELECT there is no JRE selected yet. <br/>
    JFW_E_RUNNING_JVM there is already a VM running.<br/>
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
JVMFWK_DLLPUBLIC javaFrameworkError jfw_startVM(JavaInfo const* pInfo,
                                                std::vector<OUString> const& arOptions,
                                                JavaVM** ppVM, JNIEnv** ppEnv);

/** determines the JRE that is to be used.

    <p>When calling <code>jfw_startVM</code> then a VM is started from
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
JVMFWK_DLLPUBLIC javaFrameworkError jfw_setSelectedJRE(JavaInfo const* pInfo);

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
    NULL then the function sets the pointer.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
    JFW_E_INVALID_SETTINGS the javavendors.xml has been changed and no
    JRE has been selected afterwards. <br/>
 */
JVMFWK_DLLPUBLIC javaFrameworkError jfw_getSelectedJRE(std::unique_ptr<JavaInfo>* ppInfo);

/** determines if Java can be used.

   <p>If <code>bEnabled</code> is <code>false</code> then a call
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
JVMFWK_DLLPUBLIC javaFrameworkError jfw_setEnabled(bool bEnabled);

/** provides the information if Java can be used.

    <p>That is if the user enabled or disabled the use of Java.
    </p>

   @return
   JFW_E_NONE function ran successfully.<br/>
   JFW_E_ERROR An error occurred.<br/>
   JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
    JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError jfw_getEnabled(bool* pbEnabled);

/** determines parameters which are passed to VM during its creation.

    <p>The strings must be exactly as they are passed on the command line.
    For example, one could pass<br/>
    -Xdebug <br/>
    -Xrunjdw:transport=dt_socket,server=y,address=8000<br/>
    in order to enable debugging support.
    </p>

    @param arParameters
    [in] contains the arguments.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_ERROR An error occurred.<br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
    JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError jfw_setVMParameters(std::vector<OUString> const& arArgs);

/** obtains the currently used start parameters.

    @param parParameters
    [out] on returns contains a pointer to the array of the start arguments.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_ERROR An error occurred.<br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
    JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError jfw_getVMParameters(std::vector<OUString>* parParameters);

/** sets the user class path.

   <p>When the VM is started then it is passed the class path. The
   class path also contains the user class path set by this function.
   The paths contained in <code>pCP</code> must be separated with a
   system dependent path separator.</p>

   @param pCP
   [in] the user class path.

   @return
   JFW_E_NONE function ran successfully.<br/>
   JFW_E_ERROR An error occurred.<br/>
   JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
   JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError jfw_setUserClassPath(OUString const& pCP);
/** provides the value of the current user class path.

   <p>The function returns an empty string if no user class path is set.
   </p>

   @param ppCP
   [out] contains the user class path on return.

   @return
   JFW_E_NONE function ran successfully.<br/>
   JFW_E_ERROR An error occurred.<br/>
   JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
   JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError jfw_getUserClassPath(OUString* ppCP);

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
    JFW_E_ERROR An error occurred.<br/>
    JFW_E_CONFIGURATION mode was not properly set or their prerequisites
    were not met.<br/>
    JFW_E_DIRECT_MODE the function cannot be used in this mode.
 */
JVMFWK_DLLPUBLIC javaFrameworkError jfw_addJRELocation(OUString const& sLocation);

/** checks if the installation of the jre still exists.

    This function checks if the JRE described by pInfo still
    exists. The check must be very quick because it is called by javaldx
    (Linux, Solaris) at start up.

    @param pInfo
        [in]  the JavaInfo object with information about the JRE.
    @param pp_exist
        [out] the parameter is set to either true or false. The value is
        only valid if the function returns JFW_E_NONE.

   @return
    JFW_E_NONE the function ran successfully.</br>
    JFW_E_ERROR an error occurred during execution.</br>
 */
JVMFWK_DLLPUBLIC javaFrameworkError jfw_existJRE(const JavaInfo* pInfo, bool* exist);

/** locks this API so that it cannot be used by other threads.

    <p>If a different thread called this function before then the
    current call is blocked until the other thread has called
    <code>jfw_unlock()</code>. The function should be called if one
    needs an exact snapshot of the current settings. Then the settings
    are retrieved one by one without risk that the settings may be changed
    by a different thread. Similarity if one needs to make settings which
    should become effective at the same time then <code>jfw_lock</code>
    should be called. That is, <code>jfw_startVM</code> which uses the
    settings cannot be called before all settings have be made.</p>
    <p>
    The only functions which is not effected by <code>jfw_lock</code> is
    <code>jfw_areEqualJavaInfo</code>.
 */
JVMFWK_DLLPUBLIC void jfw_lock();

/** unlocks this API.

    <p>This function is called after <code>jfw_lock</code>. It allows other
    threads to use this API concurrently.</p>
*/
JVMFWK_DLLPUBLIC void jfw_unlock();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
