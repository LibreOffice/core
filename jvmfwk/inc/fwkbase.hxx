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
#ifndef INCLUDED_JVMFWK_INC_FWKBASE_HXX
#define INCLUDED_JVMFWK_INC_FWKBASE_HXX

#include <sal/config.h>

#include <optional>
#include <string_view>
#include <vector>

#include <rtl/ustring.hxx>
#include "libxmlutil.hxx"

namespace jfw
{
struct VersionInfo;

class VendorSettings
{
    CXmlDocPtr m_xmlDocVendorSettings;
    CXPathContextPtr m_xmlPathContextVendorSettings;

public:
    VendorSettings();

    std::optional<VersionInfo> getVersionInformation(std::u16string_view sVendor) const;
};

/* The class offers functions to retrieve verified bootstrap parameters.
 */
namespace BootParams
{
/* Gets the file URL to the JRE which has been determined by the
   bootstrap parameter UNO_JAVA_JFW_JREHOME or UNO_JAVA_JFW_ENV_JREHOME.

   In direct mode either of them must be set. If not an exception is thrown.
*/
OUString getJREHome();

::std::vector<OString> getVMParameters();

OUString getUserData();

OUString getSharedData();

/* returns the file URL to the vendor settings xml file.
 */
OUString getVendorSettings();

/* User the parameter UNO_JAVA_JFW_CLASSPATH and UNO_JAVA_JFW_ENV_CLASSPATH
   to compose a classpath
 */
OString getClasspath();

OUString getClasspathUrls();

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
OString makeClassPathOption(std::u16string_view sUserClassPath);

OString getSettingsPath(const OUString& sURL);

/** Get the system path to the javasettings.xml
    Converts the URL returned from getUserSettingsURL to a
    Systempath. An empty string is returned if the file
    does not exist.
   @throws FrameworkException
 */
OString getUserSettingsPath();

/** Returns the system path of the share settings file.
    Returns a valid string or throws an exception.
   @throws FrameworkException
 */
OString getSharedSettingsPath();

/* returns a valid string or throws an exception.
   @throws FrameworkException
 */
OString getVendorSettingsPath();

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
OUString getApplicationClassPath();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
