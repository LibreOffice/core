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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "config.hxx"
#include "myconfigurationhelper.hxx"
#include <rtl/ustrbuf.hxx>


using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


namespace
{
    using namespace oooimprovement;

    static const OUString CFG_COUNTERS = OUString::createFromAscii("Counters");
    static const OUString CFG_ENABLINGALLOWED = OUString::createFromAscii("EnablingAllowed");
    static const OUString CFG_EVENTSCOUNT = OUString::createFromAscii("LoggedEvents");
    static const OUString CFG_EXTENSION = OUString::createFromAscii("ooSetupExtension");
    static const OUString CFG_FAILEDATTEMPTS = OUString::createFromAscii("FailedAttempts");
    static const OUString CFG_INVACCEPT = OUString::createFromAscii("InvitationAccepted");
    static const OUString CFG_L10N = OUString::createFromAscii("L10N");
    static const OUString CFG_LOCALE = OUString::createFromAscii("ooLocale");
    static const OUString CFG_LOGGING = OUString::createFromAscii("/org.openoffice.Office.Logging");
    static const OUString CFG_LOGPATH = OUString::createFromAscii("LogPath");
    static const OUString CFG_NAME = OUString::createFromAscii("ooName");
    static const OUString CFG_OFFICESTARTCOUNTDOWN = OUString::createFromAscii("OfficeStartCounterdown");
    static const OUString CFG_OOOIMPROVEMENT = OUString::createFromAscii("OOoImprovement");
    static const OUString CFG_OOOIMPROVEMENTPACK = OUString::createFromAscii("/org.openoffice.Office.OOoImprovement.Settings");
    static const OUString CFG_PARTICIPATION = OUString::createFromAscii("Participation");
    static const OUString CFG_PRODUCT = OUString::createFromAscii("Product");
    static const OUString CFG_REPORTCOUNT = OUString::createFromAscii("UploadedReports");
    static const OUString CFG_REPORTEREMAIL = OUString::createFromAscii("ReporterEmail");
    static const OUString CFG_SETUP = OUString::createFromAscii("/org.openoffice.Setup");
    static const OUString CFG_SHOWEDINV = OUString::createFromAscii("ShowedInvitation");
    static const OUString CFG_SOAPIDADD = OUString::createFromAscii("SoapIdAdditions");
    static const OUString CFG_SOAPURL = OUString::createFromAscii("SoapUrl");
    static const OUString CFG_UPLOAD = OUString::createFromAscii("Upload");
    static const OUString CFG_VERSION = OUString::createFromAscii("ooSetupVersion");

    static const OUString SOAPID = OUString::createFromAscii("OpenOffice.org Improvement Report - Version 1\n");

    static sal_Int32 incrementCfgValue(
        const Reference<XMultiServiceFactory> sm,
        const OUString& package,
        const OUString& rel_path,
        const OUString& key,
        sal_Int32 increment_by)
    {
        sal_Int32 value;
        Reference<XInterface> cfg =
            MyConfigurationHelper::openConfig(
                sm,
                package, MyConfigurationHelper::E_STANDARD);
        MyConfigurationHelper::readRelativeKey(
            cfg,
            rel_path, key) >>= value;
        value += increment_by;
        MyConfigurationHelper::writeRelativeKey(
            cfg,
            rel_path, key,
            Any(value));
        MyConfigurationHelper::flush(cfg);
        return value;
    };
}

namespace oooimprovement
{
    Config::Config(const Reference<XMultiServiceFactory>& sf)
        : m_ServiceFactory(sf)
    {}

    OUString Config::getSoapUrl() const
    {
        OUString result;
        MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_UPLOAD, CFG_SOAPURL,
            MyConfigurationHelper::E_READONLY) >>= result;
        return result;
    }

    OUString Config::getSoapId() const
    {
        OUString value;
        OUStringBuffer result = SOAPID;
        MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_UPLOAD, CFG_SOAPIDADD,
            MyConfigurationHelper::E_READONLY) >>= value;
        result.append(value);
        return result.makeStringAndClear();
    }

    OUString Config::getReporterEmail() const
    {
        OUString result;
        MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_UPLOAD, CFG_REPORTEREMAIL,
            MyConfigurationHelper::E_READONLY) >>= result;
        return result;
    }

    OUString Config::getLogPath() const
    {
        OUString result;
        MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_LOGGING, CFG_OOOIMPROVEMENT, CFG_LOGPATH,
            MyConfigurationHelper::E_READONLY) >>= result;
        return result;
    }

    bool Config::getEnablingAllowed() const
    {
        bool result = false;
        MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_LOGGING, CFG_OOOIMPROVEMENT, CFG_ENABLINGALLOWED,
            MyConfigurationHelper::E_READONLY) >>= result;
        return result;
    }

    bool Config::getInvitationAccepted() const
    {
       bool result = false;
       MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_PARTICIPATION, CFG_INVACCEPT,
            MyConfigurationHelper::E_READONLY) >>= result;
       return result;
    };

    bool Config::getShowedInvitation() const
    {
       bool result = false;
       MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_PARTICIPATION, CFG_SHOWEDINV,
            MyConfigurationHelper::E_READONLY) >>= result;
       return result;
    };

    OUString Config::getCompleteProductname() const
    {
        OUStringBuffer result;
        OUString value;
        MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_SETUP, CFG_PRODUCT, CFG_NAME,
            MyConfigurationHelper::E_READONLY) >>= value;
        result.append(value);

        value = OUString::createFromAscii("");
        MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_SETUP, CFG_PRODUCT, CFG_VERSION,
            MyConfigurationHelper::E_READONLY) >>= value;
        if(value.getLength()) result.appendAscii(" ").append(value);

        value = OUString::createFromAscii("");
        MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_SETUP, CFG_PRODUCT, CFG_EXTENSION,
            MyConfigurationHelper::E_READONLY) >>= value;
        if(value.getLength()) result.appendAscii(" ").append(value);

        return result.makeStringAndClear();
    }

    OUString Config::getSetupLocale() const
    {
        OUString result;
        MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_SETUP, CFG_L10N, CFG_LOCALE,
            MyConfigurationHelper::E_READONLY) >>= result;
        return result;
    }

    sal_Int32 Config::getReportCount() const
    {
        sal_Int32 result = 0;
        MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_COUNTERS, CFG_REPORTCOUNT,
            MyConfigurationHelper::E_READONLY) >>= result;
        return result;
    }

#ifdef FUTURE
    sal_Int32 Config::getFailedAttempts() const
    {
        sal_Int32 result = 0;
        MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_COUNTERS, CFG_FAILEDATTEMPTS,
            MyConfigurationHelper::E_READONLY) >>= result;
        return result;
    }
#endif

    sal_Int32 Config::getOfficeStartCounterdown() const
    {
        sal_Int32 result = 0;
        MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_PARTICIPATION, CFG_OFFICESTARTCOUNTDOWN,
            MyConfigurationHelper::E_READONLY) >>= result;
        return result;
    }

    sal_Int32 Config::incrementReportCount(sal_Int32 by)
    {
        return incrementCfgValue(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_COUNTERS, CFG_REPORTCOUNT,
            by);
    }

    sal_Int32 Config::incrementEventCount(sal_Int32 by)
    {
        return incrementCfgValue(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_COUNTERS, CFG_EVENTSCOUNT,
            by);
    }

    sal_Int32 Config::incrementFailedAttempts(sal_Int32 by)
    {
        return incrementCfgValue(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_COUNTERS, CFG_FAILEDATTEMPTS,
            by);
    }

    sal_Int32 Config::decrementOfficeStartCounterdown(sal_Int32 by)
    {
        return incrementCfgValue(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_PARTICIPATION, CFG_OFFICESTARTCOUNTDOWN,
            -by);
    }

    void Config::resetFailedAttempts()
    {
        sal_Int32 zero = 0;
        MyConfigurationHelper::writeDirectKey(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_COUNTERS, CFG_FAILEDATTEMPTS,
            Any(zero),
            MyConfigurationHelper::E_STANDARD);
    }

    void Config::giveupUploading()
    {
        sal_Bool f = false;
        MyConfigurationHelper::writeDirectKey(
            m_ServiceFactory,
            CFG_OOOIMPROVEMENTPACK, CFG_PARTICIPATION, CFG_INVACCEPT,
            Any(f),
            MyConfigurationHelper::E_STANDARD);
        resetFailedAttempts();
    }
}
