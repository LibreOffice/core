/*************************************************************************
 *
 *  $RCSfile: framework.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2004-04-21 12:16:18 $
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

#if !defined INCLUDED_JVMFWK_FRAMEWORK_H
#define INCLUDED_JVMFWK_FRAMEWORK_H

#include "rtl/ustring.h"
#include "osl/mutex.h"
#include "jni.h"

#ifdef __cplusplus
extern "C" {
#endif

#define JFW_FEATURE_ACCESSBRIDGE 0x1l
/** Indicates that there must be an environment set up before the Java process
    runs. Therefore, when a Java is selected in OO then the office must be
    restarted, so that the changes can take effect.
 */
#define JFW_REQUIRE_NEEDRESTART  0x1l

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
    JFW_E_NO_JAVA_FOUND
} javaFrameworkError;


struct _JavaInfo
{
    rtl_uString *sVendor;
    rtl_uString *sLocation;
    rtl_uString *sVersion;
    sal_uInt64 nFeatures;
    sal_uInt64 nRequirements;
    sal_Sequence * arVendorData;
};

typedef struct _JavaInfo JavaInfo;

/** frees the memory of a JavaInfo object.
    @param pInfo
    The object which is to be freed. It can be NULL;
 */
void SAL_CALL jfw_freeJavaInfo(JavaInfo *pInfo);

/**
   Both arguments must me valid JavaInfo*.
 */
sal_Bool SAL_CALL jfw_areEqualJavaInfo(
    JavaInfo const * pInfoA,JavaInfo const * pInfoB);
/** When a Java is already running and a user changes the Java configuration
    in the options dialog then the user must be informed that the new
    settings come into effect after re-starting the office. The options dialog
    can use this function to find out if the user needs to be informed.
    If one day multiple JVMs can exist in one process, then this function may
    be obsolete.

    @return
    JFW_E_NONE
    JFW_E_INVALID_ARG;
*/
javaFrameworkError SAL_CALL jfw_isJavaRunning(sal_Bool *bRunning);

/** Detects a suitable Java and configures the framework to use this Java.
    <p>
    The function uses the requirements from the file javavendors.xml.
    A JRE installation is only selected if it meets those requirements.
    Information about the selected Java are made persistent as user data so that
    subsequent calls to jfw_getSelectedJava returns this information.</p>
    <p>
    If the JRE represented by pInfo has the flag JFW_REQUIRE_NEEDRESTART
    set in pInfo->nRequirements, then the function jfw_startJava will not
    create a JVM until the office has been restarted.</p>
    <p>
    The new
    settings are effective immediatly. That is, jfw_useNewSettings does
    not need to be called.</p>
    <p>
    //ToDo algorith for selecting a JRE

    @param ppInfo
    [out] The function returns a JavaInfo pointer.
    It can be NULL. If *pInfo is not null, then it is overwritten, without
    attempting to free pInfo.

    @return
    JFW_E_ERROR
    JFW_E_CONFIGREADWRITE An error occurred while reading or writing the
    configuration files. <br/>
    JFW_E_FORMAT_STORE <br/>
    JFW_E_NOPLUGIN A plug-in library could not be found. <br/>
    JFW_E_NOJAVAFOUND No Java was found that meets the requirements.
 */
javaFrameworkError SAL_CALL jfw_findAndSelectJava(JavaInfo **pInfo);

/** provides information about all availabe Java installations.
    <p>
    It determines dynamically what JREs are available. It uses the plugin
    libraries to provide lists of available JREs (see vendorplugin.h,
    getAllJavaInfos). Also it uses a list of path, which have been registered
    by jfw_addJRELocation. It is checked if the path still contains a valid
    JRE and if so the respective JavaInfo object will be appended to the array
    pparInfo. If the path for some reason is no JRE then the path will be
    removed from an internal list.</p>
 */
javaFrameworkError SAL_CALL jfw_findAllJREs(
    JavaInfo ***pparInfo, sal_Int32 *pSize);

/** determines if a path belongs to a Java installation.
   <p>
   If the path belongs to a Java installation then it returns the
   respective JavaInfo object. To make that JRE the "selected" JRE one has
   to call jfw_setJava and jfw_applyChanges.
   </p>
   @param pPath
   [in]
   @param pInfo
   [out]

   @return
   JFW_E_NONE
   JFW_E_ERROR
   JFW_E_INVALID_ARG
   JFW_E_NOT_RECOGNIZED

 */
javaFrameworkError SAL_CALL jfw_getJavaInfoByPath(
    rtl_uString *pPath, JavaInfo **ppInfo);


/** Starts a JVM.
    <p>
    Uses the information in the javasetttings.xml to start a JVM. The arOptions
    argument contains options, which shall be passed on to the JVM. These
    could be things, such as language settings which are currently obtained
    from the office's configuration in the JavaVirtualMachine service. The
    java.class.path property cannot be set this way.    </p>
    <p>
    If there are neither user or global settings which contain data about
    a JRE which is to be used, then the function returns
    <code>JFW_E_NO_SELECT</code>.</p>
    <p>
    If the function was called successfully then calling it again will
    not create a new VM and JFW_E_RUNNING_JVM is returned. When at one time, one
    can have  multiple VMs in one process than this function may allow to start
    a second VM.</p>


    @param arOptions
    Can be NULL
    @return
    JFW_E_INVALID_ARG ppVM is NULL,
    JFW_E_CONFIG_READWRITE
    JFW_E_FORMAT_STORE
    JFW_E_NO_PLUGIN A plug-in library could not be found. <br/>
    JFW_E_JAVA_DISABLED the use of Java is currently disabled. <br/>
    JFW_E_NO_SELECT javasettings.xml does not contain data about
    JFW_E_RUNNIN_GJVM
    a JRE which is to be used.<br/>
    JFW_E_INVALID_SETTINGS the javavendors.xml has been changed and no
    Java has been selected afterwards. <br/>
    JFW_E_NEED_RESTART In the current process a different Java has been selected
    which needs a prepared environment, which has to be done before the office
    process. For example, setting the LD_LIBRARY_PATH. Therefore the new
    Java may not be created until office was restarted.<br/>
 */
javaFrameworkError SAL_CALL jfw_startJava(JavaVMOption *arOptions,
                                 sal_Int32 cOptions, JavaVM **ppVM,
                                 JNIEnv **ppEnv);

/** Configure the framework to use a particular Java.
    <p>
    Called by options dialog if someone choose an entry from the list.
    It is not checked if the JRE represented by pInfo meets the  requirements
    from the javavendors.xml file. </p>
    <p>
    If pInfo is NULL then the meaning is that no java will be selected. jfw_startJava
    will later return JFW_E_NO_SELECT. </p>
    <p>
    This setting will only become effective after calling jfw_applyChanges.
    </p>
    If the JRE represented by pInfo has the flag JFW_REQUIRE_NEEDRESTART
    set in pInfo->nRequirements, then the function startJava will not
    create a JVM until the office has been restarted.</p>

    @param pInfo
      [in] pointer to JavaInfo structure, containing data about Java
      installation. The caller must still free pInfo.
 */
javaFrameworkError SAL_CALL jfw_setSelectedJava(JavaInfo const *pInfo);


/** Provides information about the currently selected Java installation.
    <p>
    It is not guaranteed that the selected JRE does not change after a
    call to this function. However, it can be used by Java preparation tools
    which run prior to the office process and set up the necessary
    environment required by that JRE.
    </p>
    <p>
    If the value of the element <updated> in the javavendors.xml file was
    changed since the time when the last Java was selected then this
    function returns JFW_E_INVALID_SETTINGS.
    </p>
    @param pInfo [out]

    @return
    JFW_E_NONE on success <br/>

    JFW_E_INVALIDARG, pInfo is a NULL pointer or a member of pInfo is
    invalid. For example, if pInfo->sVendor is not NULL because the member
    was not properly initialized, then it will be tried to release the string
    which will fail. <br/>
    JFW_E_CONFIG_READWRITE
    JFW_E_FORMAT_STORE
    JFW_E_NO_SELECT there is no Java selected yet.
    JFW_E_INVALID_SETTINGS
 */
javaFrameworkError SAL_CALL jfw_getSelectedJava(JavaInfo **pInfo);


/** sets the the &quot;enabled&quot parameter.
    <p>
   Used from option dialog.
   If bGlobal is true then this value is stored globally, that is
   the value is valid for all users unless they have made own settings.
   The user must have sufficient file access rights to modify the
   global settings.</p>
   <p>
   The new value comes only into effect after jfw_applyChanges is
   called.</p>
   @return
   JFW_E_ERROR An error occurred.E.g. the user has not sufficient rights.
   JFW_E_INVALIDARG
   JFW_E_JAVADISABLED
 */
javaFrameworkError SAL_CALL jfw_setEnabled(sal_Bool bEnabled);

/** determines the value of the Java setting &quot;enabled&quot;.
    <p>
    The returned value in <code>pbEnabled</code> has been taken from
    a snapshot of the internal data. The value may change later on.</p>
    <p>
    This function can be called from
    Java preparation tools, such as javaldx, which run prior to the
    office process. The status is usually only changed by the
    options dialog, where a user can enable or disable Java. </p>
    <p>
    If the function jfw_setEnabled has never been called before
    by the current user, then the value returned in <code>pbEnabled</code>
    is determined by global settings. If there are no global settings
    then the default is that Java is enabled.

    @return
    JFW_E_NONE The function was successful.<br/>
    JFW_E_ERROR An error occurred.<br/>
    JFW_E_CONFIG_READWRITE Error during access of internal data store.<br/>
    JFW_E_FORMAT_STORE The structure of the internal data store is not
    as expected. <br/>
    JFW_E_INVALIDARG pbEnabled is NULL<br/>
 */
javaFrameworkError SAL_CALL jfw_getEnabled(sal_Bool *pbEnabled);

/** sets options, such as debug options.

    arOptions can be null if nSize is also 0.

    @return
    JFW_E_INVALIDARG arOptions is NULL and nSize is not 0
 */
javaFrameworkError SAL_CALL jfw_setVMParameters(
    rtl_uString **  arParameters, sal_Int32 nSize);
/**
   Caller needs to free the returned array with rtl_freeMemory. The
   containes rtl_uStrings must be released with rtl_uString_release.

   @return
    JFW_E_NONE The function was successful.<br/>
    JFW_E_ERROR An error occurred.<br/>
    JFW_E_CONFIG_READWRITE Error during access of internal data store.<br/>
    JFW_E_FORMAT_STORE The structure of the internal data store is not
    as expected. <br/>
    JFW_E_INVALIDARG parOptions or pSize are  NULL<br/>
 */
javaFrameworkError SAL_CALL jfw_getVMParameters(
    rtl_uString *** parParameters,
    sal_Int32 * pSize);

/**
   The argument pC must be a valid string. If the value is NULL then
   a JFW_E_INVALID_ARG is returned.
 */
javaFrameworkError SAL_CALL jfw_setUserClassPath(rtl_uString * pCP);
/**
   returns an empty string if no user class path is set.
 */
javaFrameworkError SAL_CALL jfw_getUserClassPath(rtl_uString ** ppCP);

/** saves the location of a JRE.
    <p>
    When jfw_findAllJREs is called then the paths added by this
    function are evaluated. If the location still represents a
    JRE then a JavaInfo object is created which is returned along with
    all other JavaInfo objects by jfw_findAllJREs. If the location
    cannot be recognized then the location string is ignored.
    </p>
    <p>
    A validation if <code> sLocation </code> points to a JRE is not
    performed. To do that one has to use jfw_getJavaInfoByPath.
    </p>
    <p>
    Adding a path that is already stored causes no error.
    @param sLocation
    File URL to an directory which contains a JRE.
 */
javaFrameworkError SAL_CALL jfw_addJRELocation(rtl_uString * sLocation);

/** stores an array containing paths to JRE installations.
    <p>
    The function does not verify if the paths points to JRE. However,
    it makes sure that every path is unique. That is, if the array
    contains string which are the same then only one is stored.</p>
    <p>
    If arLocations is NULL or arLocations has the length null
    then all previously stored paths are deleted. Otherwise,
    the old values are overwritten.</p>
 */
javaFrameworkError SAL_CALL jfw_setJRELocations(
    rtl_uString ** arLocations, sal_Int32 nSize);
/** obtains an array containing paths to JRE installations.
    <p>
    It is not guaranteed that the returned paths represent
    a valid JRE. One can use jfw_getJavaInfoByPath to check this.
    </p>
 */
javaFrameworkError SAL_CALL jfw_getJRELocations(
    rtl_uString *** parLocations, sal_Int32 * pSize);


void SAL_CALL jfw_lock();

void SAL_CALL jfw_unlock();


#ifdef __cplusplus
}
#endif


#endif
