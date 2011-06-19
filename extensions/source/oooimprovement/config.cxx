/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

    static const OUString CFG_COUNTERS(RTL_CONSTASCII_USTRINGPARAM("Counters"));
    static const OUString CFG_ENABLINGALLOWED(RTL_CONSTASCII_USTRINGPARAM("EnablingAllowed"));
    static const OUString CFG_EVENTSCOUNT(RTL_CONSTASCII_USTRINGPARAM("LoggedEvents"));
    static const OUString CFG_EXTENSION(RTL_CONSTASCII_USTRINGPARAM("ooSetupExtension"));
    static const OUString CFG_FAILEDATTEMPTS(RTL_CONSTASCII_USTRINGPARAM("FailedAttempts"));
    static const OUString CFG_INVACCEPT(RTL_CONSTASCII_USTRINGPARAM("InvitationAccepted"));
    static const OUString CFG_L10N(RTL_CONSTASCII_USTRINGPARAM("L10N"));
    static const OUString CFG_LOCALE(RTL_CONSTASCII_USTRINGPARAM("ooLocale"));
    static const OUString CFG_LOGGING(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.Logging"));
    static const OUString CFG_LOGPATH(RTL_CONSTASCII_USTRINGPARAM("LogPath"));
    static const OUString CFG_NAME(RTL_CONSTASCII_USTRINGPARAM("ooName"));
    static const OUString CFG_OFFICESTARTCOUNTDOWN(RTL_CONSTASCII_USTRINGPARAM("OfficeStartCounterdown"));
    static const OUString CFG_OOOIMPROVEMENT(RTL_CONSTASCII_USTRINGPARAM("OOoImprovement"));
    static const OUString CFG_OOOIMPROVEMENTPACK(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.OOoImprovement.Settings"));
    static const OUString CFG_PARTICIPATION(RTL_CONSTASCII_USTRINGPARAM("Participation"));
    static const OUString CFG_PRODUCT(RTL_CONSTASCII_USTRINGPARAM("Product"));
    static const OUString CFG_REPORTCOUNT(RTL_CONSTASCII_USTRINGPARAM("UploadedReports"));
    static const OUString CFG_REPORTEREMAIL(RTL_CONSTASCII_USTRINGPARAM("ReporterEmail"));
    static const OUString CFG_SETUP(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup"));
    static const OUString CFG_SHOWEDINV(RTL_CONSTASCII_USTRINGPARAM("ShowedInvitation"));
    static const OUString CFG_SOAPIDADD(RTL_CONSTASCII_USTRINGPARAM("SoapIdAdditions"));
    static const OUString CFG_SOAPURL(RTL_CONSTASCII_USTRINGPARAM("SoapUrl"));
    static const OUString CFG_UPLOAD(RTL_CONSTASCII_USTRINGPARAM("Upload"));
    static const OUString CFG_VERSION(RTL_CONSTASCII_USTRINGPARAM("ooSetupVersion"));

    static const OUString SOAPID(RTL_CONSTASCII_USTRINGPARAM("OpenOffice.org Improvement Report - Version 1\n"));

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

        value = OUString();
        MyConfigurationHelper::readDirectKey(
            m_ServiceFactory,
            CFG_SETUP, CFG_PRODUCT, CFG_VERSION,
            MyConfigurationHelper::E_READONLY) >>= value;
        if(value.getLength()) result.appendAscii(" ").append(value);

        value = OUString();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
