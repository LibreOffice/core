/*************************************************************************
 *
 *  $RCSfile: framework.h,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: jl $ $Date: 2004-05-21 15:07:05 $
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
 *   *  Copyright 2000 by Sun Microsystems, Inc.
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
 *   *  The contents of this file are subject to the Sun Industry Standards
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

#if !defined INCLUDED_JVMFWK_FRAMEWORK_H
#define INCLUDED_JVMFWK_FRAMEWORK_H

#include "rtl/ustring.h"
#include "osl/mutex.h"
#include "jni.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @file
    <p>
    This library currently has to operational modes. In office mode it
    expects that it is located in the program directory and the javavendors.xml
    file must be in the folder <office>/share/config. A Java Virtual Machine (JVM)
    can only be created if a JRE has been selected. That is either
    <code>jfw_setSelectedJRE</code> or <code>jfw_findAndSelectJRE</code> must have
    run successfully.</p>
    <p>
    In &quot;default mode&quot; the framework uses the environment variables
    <code>JAVA_HOME</code> and <code>CLASSPATH</code> to determine the JRE which
    is to be used and the class path. If the JRE does not meet the version
    requirements as specified by the javavendors.xml then jfw_startVM will return
    JFW_E_FAILED_VERSION. It is expected that the javavendors.xml and the plug-in
    libraries are located in the same folder as this library. In this mode no
    settings are written nor read. The functions will return JFW_E_DEFAULT_MODE.
    </p>

    All settings made by this API are done for the current user if not
    mentioned differently.
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
typedef enum
{
    JFW_E_NONE,
    JFW_E_ERROR,
    JFW_E_INVALID_ARG,
    JFW_E_NO_SELECT,
    JFW_E_INVALID_SETTINGS,
    JFW_E_NEED_RESTART,
    JFW_E_RUNNING_JVM,
    JFW_E_JAVA_DISABLED,
    JFW_E_CONFIG_READWRITE,
    JFW_E_FORMAT_STORE,
    JFW_E_NO_PLUGIN,
    JFW_E_NOT_RECOGNIZED,
    JFW_E_FAILED_VERSION,
    JFW_E_NO_JAVA_FOUND,
    JFW_E_VM_CREATION_FAILED,
    JFW_E_DEFAULT_MODE
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
void SAL_CALL jfw_freeJavaInfo(JavaInfo *pInfo);


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
sal_Bool SAL_CALL jfw_areEqualJavaInfo(
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
javaFrameworkError SAL_CALL jfw_isVMRunning(sal_Bool *bRunning);

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
    JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
    the internally used data store. <br/>
    JFW_E_FORMAT_STORE the internally used data store has not the
    expected format<br/>
    JFW_E_NO_PLUGIN a plug-in library could not be found.<br/>
    JFW_E_NO_JAVA_FOUND no JRE was found that meets the requirements.</br>
    JFW_E_DEFAULT_MODE because of this mode no settings are written.
 */
javaFrameworkError SAL_CALL jfw_findAndSelectJRE(JavaInfo **pInfo);

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
    JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
    the internally used data store. <br/>
    JFW_E_FORMAT_STORE the internally used data store has not the
    expected format<br/>
    JFW_E_NO_PLUGIN a plug-in library could not be found.<br/>
*/
javaFrameworkError SAL_CALL jfw_findAllJREs(
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
   JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
   the internally used data store. <br/>
   JFW_E_FORMAT_STORE the internally used data store has not the
   expected format<br/>
   JFW_E_NO_PLUGIN a plug-in library could not be found.<br/>
   JFW_E_NOT_RECOGNIZED neither plug-in library could detect a JRE. <br/>
   JFW_E_FAILED_VERSION a JRE was detected but if failed the version
   requirements as determined by the javavendors.xml
 */
javaFrameworkError SAL_CALL jfw_getJavaInfoByPath(
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
    JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
    the internally used data store. <br/>
    JFW_E_FORMAT_STORE the internally used data store has not the
    expected format<br/>
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
javaFrameworkError SAL_CALL jfw_startVM(JavaVMOption *arOptions,
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
    JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
    the internally used data store. <br/>
    JFW_E_FORMAT_STORE the internally used data store has not the
    expected format<br/>
    JFW_E_DEFAULT_MODE because of this mode no settings are written.
 */
javaFrameworkError SAL_CALL jfw_setSelectedJRE(JavaInfo const *pInfo);


/** provides information about the JRE that is to be used.
    <p>
    If no JRE is currently selected then <code>ppInfo</code> will contain
    NULL on return.</br>
    If the value of the element <updated> in the javavendors.xml file was
    changed since the time when the last Java was selected then this
    function returns <code>JFW_E_INVALID_SETTINGS</code>. This could happen during
    a product patch. Then new version requirements may be introduces, so that
    the currently selected JRE may not meet these requirements anymore.
    </p>
    @param ppInfo
    [out] on return it contains a pointer to a <code>JavaInfo</code> object
    that represents the currently selected JRE. When <code>*ppInfo</code> is not
    NULL then the function overwrites the pointer. It is not attempted to free
    the pointer.

    @return
    JFW_E_NONE function ran successfully.<br/>
    JFW_E_INVALIDARG <code>ppInfo</code> is a NULL.<br/>
    JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
    the internally used data store. <br/>
    JFW_E_FORMAT_STORE the internally used data store has not the
    expected format<br/>
    JFW_E_INVALID_SETTINGS the javavendors.xml has been changed and no
    JRE has been selected afterwards. <br/>
    JFW_E_DEFAULT_MODE because of this mode no settings are read.
 */
javaFrameworkError SAL_CALL jfw_getSelectedJRE(JavaInfo **ppInfo);


/** determines if Java can be used.

   <p>If <code>bEnabled</code> is <code>sal_False</code> then a call
   to jfw_startVM will result in an error with the errorcode
   <code>JFW_E_JAVA_DISABLED</code></p>

   @param bEnabled
   [in] use of Java enabled/disabled.

   @return
   JFW_E_NONE function ran successfully.<br/>
   JFW_E_ERROR An error occurred.<br/>
   JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
   the internally used data store. <br/>
   JFW_E_FORMAT_STORE the internally used data store has not the
   expected format<br/>
   JFW_E_DEFAULT_MODE because of this mode no settings are written.
 */
javaFrameworkError SAL_CALL jfw_setEnabled(sal_Bool bEnabled);

/** provides the information if Java can be used.

   @return
   JFW_E_NONE function ran successfully.<br/>
   JFW_E_INVALIDARG pbEnabled is NULL<br/>
   JFW_E_ERROR An error occurred.<br/>
   JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
   the internally used data store. <br/>
   JFW_E_FORMAT_STORE the internally used data store has not the
   expected format<br/>
   JFW_E_DEFAULT_MODE because of this mode no settings are read.
 */
javaFrameworkError SAL_CALL jfw_getEnabled(sal_Bool *pbEnabled);

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
    JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
    the internally used data store. <br/>
    JFW_E_FORMAT_STORE the internally used data store has not the
    expected format<br/>
    JFW_E_DEFAULT_MODE because of this mode no settings are written.
 */
javaFrameworkError SAL_CALL jfw_setVMParameters(
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
    JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
    the internally used data store. <br/>
    JFW_E_FORMAT_STORE the internally used data store has not the
    expected format<br/>
    JFW_E_DEFAULT_MODE because of this mode no settings read.
 */
javaFrameworkError SAL_CALL jfw_getVMParameters(
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
   JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
   the internally used data store. <br/>
   JFW_E_FORMAT_STORE the internally used data store has not the
   expected format<br/></br>
   JFW_E_DEFAULT_MODE because of this mode no settings are written.
 */
javaFrameworkError SAL_CALL jfw_setUserClassPath(rtl_uString * pCP);
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
   JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
   the internally used data store. <br/>
   JFW_E_FORMAT_STORE the internally used data store has not the
   expected format<br/>
   JFW_E_DEFAULT_MODE because of this mode no settings read.
 */
javaFrameworkError SAL_CALL jfw_getUserClassPath(rtl_uString ** ppCP);

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
    JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
    the internally used data store. <br/>
    JFW_E_FORMAT_STORE the internally used data store has not the
    expected format</br>
    JFW_E_DEFAULT_MODE because of this mode no settings are written.

    @see jfw_setJRELocations
 */
javaFrameworkError SAL_CALL jfw_addJRELocation(rtl_uString * sLocation);

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
    JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
    the internally used data store. <br/>
    JFW_E_FORMAT_STORE the internally used data store has not the
    expected format</br>
    JFW_E_DEFAULT_MODE because of this mode no settings are written.

    @see jfw_addJRELocations
 */
javaFrameworkError SAL_CALL jfw_setJRELocations(
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
    JFW_E_CONFIG_READWRITE an error occurred while reading or writing to
    the internally used data store. <br/>
    JFW_E_FORMAT_STORE the internally used data store has not the
    expected format</br>
    JFW_E_DEFAULT_MODE because of this mode no settings are read.
 */
javaFrameworkError SAL_CALL jfw_getJRELocations(
    rtl_uString *** parLocations, sal_Int32 * pSize);

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
void SAL_CALL jfw_lock();

/** unlocks this API.

    <p>This function is called after <code>jfw_lock</code>. It allows other
    threads to use this API concurrently.</p>
*/
void SAL_CALL jfw_unlock();


#ifdef __cplusplus
}
#endif


#endif
