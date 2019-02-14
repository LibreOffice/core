/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <unicode/regex.h>

#include <comphelper/configuration.hxx>
#include <officecfg/Office/Common.hxx>
#include <opencl/openclconfig.hxx>
#include <opencl/platforminfo.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <sal/types.h>

OpenCLConfig::OpenCLConfig() :
    mbUseOpenCL(true)
{
    // This entry we have had for some time (when blacklisting was
    // done elsewhere in the code), so presumably there is a known
    // good reason for it.
    maBlackList.insert(ImplMatcher("Windows", "", "Intel\\(R\\) Corporation", "", "9\\.17\\.10\\.2884"));

    // This is what I have tested on Linux and it works for our unit tests.
    maWhiteList.insert(ImplMatcher("Linux", "", "Advanced Micro Devices, Inc\\.", "", "1445\\.5 \\(sse2,avx\\)"));

    // For now, assume that AMD, Intel and NVIDIA drivers are good
    maWhiteList.insert(ImplMatcher("", "", "Advanced Micro Devices, Inc\\.", "", ""));
    maWhiteList.insert(ImplMatcher("", "", "Intel\\(R\\) Corporation", "", ""));
    maWhiteList.insert(ImplMatcher("", "", "NVIDIA Corporation", "", ""));
}

bool OpenCLConfig::operator== (const OpenCLConfig& r) const
{
    return (mbUseOpenCL == r.mbUseOpenCL &&
            maBlackList == r.maBlackList &&
            maWhiteList == r.maWhiteList);
}

bool OpenCLConfig::operator!= (const OpenCLConfig& r) const
{
    return !operator== (r);
}

namespace {

css::uno::Sequence<OUString> SetOfImplMatcherToStringSequence(const OpenCLConfig::ImplMatcherSet& rSet)
{
    css::uno::Sequence<OUString> result(rSet.size());

    size_t n(0);
    for (const auto& rItem : rSet)
    {
        result[n++] =
            rItem.maOS.replaceAll("%", "%25").replaceAll("/", "%2F").replaceAll(";", "%3B") + "/" +
            rItem.maOSVersion.replaceAll("%", "%25").replaceAll("/", "%2F").replaceAll(";", "%3B") + "/" +
            rItem.maPlatformVendor.replaceAll("%", "%25").replaceAll("/", "%2F").replaceAll(";", "%3B") + "/" +
            rItem.maDevice.replaceAll("%", "%25").replaceAll("/", "%2F").replaceAll(";", "%3B") + "/" +
            rItem.maDriverVersion.replaceAll("%", "%25").replaceAll("/", "%2F").replaceAll(";", "%3B");
    }

    return result;
}

OUString getToken(const OUString& string, sal_Int32& index)
{
    OUString token(string.getToken(0, '/', index));
    OUStringBuffer result;
    sal_Int32 i(0);
    sal_Int32 p;
    while ((p = token.indexOf('%', i)) >= 0)
    {
        if (p > i)
            result.append(std::u16string_view(token).substr(i, p - i));
        if (p < token.getLength() - 2)
        {
            result.append(OUStringLiteral1(token.copy(p+1, 2).toInt32(16)));
            i = p + 3;
        }
        else
        {
            i = token.getLength();
        }
    }
    result.append(std::u16string_view(token).substr(i));

    return result.makeStringAndClear();
}

OpenCLConfig::ImplMatcherSet StringSequenceToSetOfImplMatcher(const css::uno::Sequence<OUString>& rSequence)
{
    OpenCLConfig::ImplMatcherSet result;

    for (const auto& rItem : rSequence)
    {
        OpenCLConfig::ImplMatcher m;
        sal_Int32 index(0);
        m.maOS = getToken(rItem, index);
        m.maOSVersion = getToken(rItem, index);
        m.maPlatformVendor = getToken(rItem, index);
        m.maDevice = getToken(rItem, index);
        m.maDriverVersion = getToken(rItem, index);

        result.insert(m);
    }

    return result;
}

bool match(const OUString& rPattern, const OUString& rInput)
{
    if (rPattern.isEmpty())
        return true;

    UErrorCode nIcuError(U_ZERO_ERROR);
    icu::UnicodeString sIcuPattern(reinterpret_cast<const UChar*>(rPattern.getStr()), rPattern.getLength());
    icu::UnicodeString sIcuInput(reinterpret_cast<const UChar*>(rInput.getStr()), rInput.getLength());
    icu::RegexMatcher aMatcher(sIcuPattern, sIcuInput, 0, nIcuError);

    return U_SUCCESS(nIcuError) && aMatcher.matches(nIcuError) && U_SUCCESS(nIcuError);
}

bool match(const OpenCLConfig::ImplMatcher& rListEntry, const OpenCLPlatformInfo& rPlatform, const OpenCLDeviceInfo& rDevice)
{
#if defined(_WIN32)
    if (!rListEntry.maOS.isEmpty() && rListEntry.maOS != "Windows")
        return false;
#elif defined LINUX
    if (!rListEntry.maOS.isEmpty() && rListEntry.maOS != "Linux")
        return false;
#elif defined MACOSX
    if (!rListEntry.maOS.isEmpty() && rListEntry.maOS != "OS X")
        return false;
#endif

    // OS version check not yet implemented

    if (!match(rListEntry.maPlatformVendor, rPlatform.maVendor))
        return false;

    if (!match(rListEntry.maDevice, rDevice.maName))
        return false;

    if (!match(rListEntry.maDriverVersion, rDevice.maDriver))
        return false;

    return true;
}

bool match(const OpenCLConfig::ImplMatcherSet& rList, const OpenCLPlatformInfo& rPlatform, const OpenCLDeviceInfo& rDevice, const char* sKindOfList)
{
    for (const auto& rListEntry : rList)
    {
        SAL_INFO("opencl", "Looking for match for platform=" << rPlatform << ", device=" << rDevice <<
                 " in " << sKindOfList << " entry=" << rListEntry);

        if (match(rListEntry, rPlatform, rDevice))
        {
            SAL_INFO("opencl", "Match!");
            return true;
        }
    }
    return false;
}

} // anonymous namespace

OpenCLConfig OpenCLConfig::get()
{
    OpenCLConfig result;

    result.mbUseOpenCL = officecfg::Office::Common::Misc::UseOpenCL::get();

    result.maBlackList = StringSequenceToSetOfImplMatcher(officecfg::Office::Common::Misc::OpenCLBlackList::get());
    result.maWhiteList = StringSequenceToSetOfImplMatcher(officecfg::Office::Common::Misc::OpenCLWhiteList::get());

    return result;
}

void OpenCLConfig::set()
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    officecfg::Office::Common::Misc::UseOpenCL::set(mbUseOpenCL, batch);
    officecfg::Office::Common::Misc::OpenCLBlackList::set(SetOfImplMatcherToStringSequence(maBlackList), batch);
    officecfg::Office::Common::Misc::OpenCLWhiteList::set(SetOfImplMatcherToStringSequence(maWhiteList), batch);

    batch->commit();
}

bool OpenCLConfig::checkImplementation(const OpenCLPlatformInfo& rPlatform, const OpenCLDeviceInfo& rDevice) const
{
    // Check blacklist of known bad OpenCL implementations
    if (match(maBlackList, rPlatform, rDevice, "blacklist"))
    {
        SAL_INFO("opencl", "Rejecting");
        return true;
    }

    // Check for whitelist of known good OpenCL implementations
    if (match(maWhiteList, rPlatform, rDevice, "whitelist"))
    {
        SAL_INFO("opencl", "Approving");
        return false;
    }

    // Fallback: reject
    SAL_INFO("opencl", "Fallback: rejecting platform=" << rPlatform << ", device=" << rDevice);
    return true;
}

std::ostream& operator<<(std::ostream& rStream, const OpenCLConfig& rConfig)
{
    rStream << "{"
        "UseOpenCL=" << (rConfig.mbUseOpenCL ? "YES" : "NO") << ","
        "BlackList=" << rConfig.maBlackList << ","
        "WhiteList=" << rConfig.maWhiteList <<
        "}";
    return rStream;
}

std::ostream& operator<<(std::ostream& rStream, const OpenCLConfig::ImplMatcher& rImpl)
{
    rStream << "{"
        "OS=" << rImpl.maOS << ","
        "OSVersion=" << rImpl.maOSVersion << ","
        "PlatformVendor=" << rImpl.maPlatformVendor << ","
        "Device=" << rImpl.maDevice << ","
        "DriverVersion=" << rImpl.maDriverVersion <<
        "}";

    return rStream;
}

std::ostream& operator<<(std::ostream& rStream, const OpenCLConfig::ImplMatcherSet& rSet)
{
    rStream << "{";
    for (auto i = rSet.cbegin(); i != rSet.cend(); ++i)
    {
        if (i != rSet.cbegin())
            rStream << ",";
        rStream << *i;
    }
    rStream << "}";
    return rStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
