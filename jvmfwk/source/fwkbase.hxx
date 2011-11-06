/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#if !defined INCLUDED_JVMFWK_FWKBASE_HXX
#define INCLUDED_JVMFWK_FWKBASE_HXX
#include "rtl/ustring.hxx"
#include "libxmlutil.hxx"
namespace jfw
{

class VendorSettings
{
    ::rtl::OUString m_xmlDocVendorSettingsFileUrl;
    CXmlDocPtr m_xmlDocVendorSettings;
    CXPathContextPtr m_xmlPathContextVendorSettings;

public:
    VendorSettings();

    /** Gets all plugin library URLs with the corresponding vendor name.

    It uses the /javaSelection/plugins/library element from the javavendors.xml
    to locate the library.
    Is is verified that the plug-in exist. If a plug-in does not exist then an
    exception is thrown containing the error JFW_E_CONFIGURATION
    */
    ::std::vector<PluginLibrary> getPluginData();

    /* returns the file URL to the plugin.
     */
    ::rtl::OUString getPluginLibrary(const ::rtl::OUString& sVendor);

    VersionInfo getVersionInformation(const ::rtl::OUString & sVendor);

    ::std::vector< ::rtl::OUString> getSupportedVendors();
};

/* The class offers functions to retrieve verified bootstrap parameters.
 */
namespace  BootParams
{

/* Gets the file URL to the JRE which has been determined by the
   bootstrap parameter UNO_JAVA_JFW_JREHOME or UNO_JAVA_JFW_ENV_JREHOME.

   In direct mode either of them must be set. If not an exception is thrown.
*/
::rtl::OUString getJREHome();

::std::vector< ::rtl::OString> getVMParameters();

::rtl::OUString getUserData();

::rtl::OUString getSharedData();

::rtl::OUString getInstallData();
/* returns the file URL to the vendor settings xml file.
 */
::rtl::OUString getVendorSettings();

/* User the parameter UNO_JAVA_JFW_CLASSPATH and UNO_JAVA_JFW_ENV_CLASSPATH
   to compose a classpath
 */
::rtl::OString getClasspath();

::rtl::OUString getClasspathUrls();

/** returns the content of UNO_JAVA_JFW_INSTALL_EXPIRE or a pretermined
    value. If the bootstrap variable contains a string that cannot be
    converted by OUString then it returns the predetermined value.
*/
::sal_uInt32 getInstallDataExpiration();

} //end namespace



enum JFW_MODE
{
    JFW_MODE_APPLICATION,

    JFW_MODE_DIRECT
};

JFW_MODE getMode();

/** creates the -Djava.class.path option with the complete classpath, including
    the paths which are set by UNO_JAVA_JFW_CLASSPATH_URLS.
 */
::rtl::OString makeClassPathOption(::rtl::OUString const & sUserClassPath);

::rtl::OString getSettingsPath( const ::rtl::OUString & sURL);

/** Get the system path to the javasettings.xml
    Converts the URL returned from getUserSettingsURL to a
    Systempath. An empty string is returned if the file
    does not exist.
   @throws FrameworkException
 */
::rtl::OString getUserSettingsPath();

::rtl::OString getInstallSettingsPath();

/** Returns the system path of the share settings file.
    Returns a valid string or throws an exception.
   @throws FrameworkException
 */
::rtl::OString getSharedSettingsPath();

/* returns a valid string or throws an exception.
   @throws FrameworkException
 */
::rtl::OString getVendorSettingsPath();

::rtl::OUString buildClassPathFromDirectory(const ::rtl::OUString & relPath);

/** Called from writeJavaInfoData. It sets the process identifier. When
java is to be started, then the current id is compared to the one set by
this function. If they are identical then the Java was selected in the
same process. If that Java needs a prepared environment, such as a
LD_LIBRARY_PATH, then it must not be started in this process.
*/
void setJavaSelected();

/** Determines if the currently selected Java was set in this process.

    @see setProcessId()
 */
bool wasJavaSelectedInSameProcess();
/* Only for application mode.
 */
::rtl::OUString getApplicationClassPath();
}

#endif
