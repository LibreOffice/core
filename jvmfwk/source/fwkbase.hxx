/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fwkbase.hxx,v $
 * $Revision: 1.7 $
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

/* Determines the application base which is either the BaseInstallation
   (see getBaseInstallation) or the directory where the executable is resides.

   This function always returns a URL or throws an exception.
*/
::rtl::OUString getApplicationBase();


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
