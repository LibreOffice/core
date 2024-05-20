/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <unicode/regex.h>

#include <comphelper/configuration.hxx>
#include <officecfg/Office/Common.hxx>
#include <opencl/openclconfig.hxx>
#include <opencl/platforminfo.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <o3tl/string_view.hxx>

OpenCLConfig::OpenCLConfig() :
    mbUseOpenCL(true)
{
    // This entry we have had for some time (when denylisting was
    // done elsewhere in the code), so presumably there is a known
    // good reason for it.
    maDenyList.insert(ImplMatcher(u"Windows"_ustr, u""_ustr, u"Intel\\(R\\) Corporation"_ustr, u""_ustr, u"9\\.17\\.10\\.2884"_ustr));
    // This was reported to produce bogus values in unit tests
    maDenyList.insert(ImplMatcher(u"Windows"_ustr, u""_ustr, u"Intel\\(R\\) Corporation"_ustr, u""_ustr, u"27\\.20\\.100\\.8681"_ustr));

    // For now, assume that AMD, Intel and NVIDIA drivers are good
    maAllowList.insert(ImplMatcher(u""_ustr, u""_ustr, u"Advanced Micro Devices, Inc\\."_ustr, u""_ustr, u""_ustr));
    maAllowList.insert(ImplMatcher(u""_ustr, u""_ustr, u"Intel\\(R\\) Corporation"_ustr, u""_ustr, u""_ustr));
    maAllowList.insert(ImplMatcher(u""_ustr, u""_ustr, u"NVIDIA Corporation"_ustr, u""_ustr, u""_ustr));
}

bool OpenCLConfig::operator== (const OpenCLConfig& r) const
{
    return (mbUseOpenCL == r.mbUseOpenCL &&
            maDenyList == r.maDenyList &&
            maAllowList == r.maAllowList);
}

bool OpenCLConfig::operator!= (const OpenCLConfig& r) const
{
    return !operator== (r);
}

namespace {

css::uno::Sequence<OUString> SetOfImplMatcherToStringSequence(const OpenCLConfig::ImplMatcherSet& rSet)
{
    css::uno::Sequence<OUString> result(rSet.size());
    auto resultRange = asNonConstRange(result);
    size_t n(0);
    for (const auto& rItem : rSet)
    {
        resultRange[n++] =
            rItem.maOS.replaceAll("%", "%25").replaceAll("/", "%2F").replaceAll(";", "%3B") + "/" +
            rItem.maOSVersion.replaceAll("%", "%25").replaceAll("/", "%2F").replaceAll(";", "%3B") + "/" +
            rItem.maPlatformVendor.replaceAll("%", "%25").replaceAll("/", "%2F").replaceAll(";", "%3B") + "/" +
            rItem.maDevice.replaceAll("%", "%25").replaceAll("/", "%2F").replaceAll(";", "%3B") + "/" +
            rItem.maDriverVersion.replaceAll("%", "%25").replaceAll("/", "%2F").replaceAll(";", "%3B");
    }

    return result;
}

OUString getToken(std::u16string_view string, sal_Int32& index)
{
    std::u16string_view token(o3tl::getToken(string, 0, '/', index));
    OUStringBuffer result;
    sal_Int32 i(0);
    size_t p;
    while ((p = token.find('%', i)) != std::u16string_view::npos)
    {
        if (static_cast<sal_Int32>(p) > i)
            result.append(token.substr(i, p - i));
        if (p < token.size() - 2)
        {
            result.append(sal_Unicode(o3tl::toInt32(token.substr(p+1, 2), 16)));
            i = p + 3;
        }
        else
        {
            i = token.size();
        }
    }
    result.append(token.substr(i));

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

    result.maDenyList = StringSequenceToSetOfImplMatcher(officecfg::Office::Common::Misc::OpenCLDenyList::get());
    result.maAllowList = StringSequenceToSetOfImplMatcher(officecfg::Office::Common::Misc::OpenCLAllowList::get());

    return result;
}

void OpenCLConfig::set()
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    officecfg::Office::Common::Misc::UseOpenCL::set(mbUseOpenCL, batch);
    officecfg::Office::Common::Misc::OpenCLDenyList::set(SetOfImplMatcherToStringSequence(maDenyList), batch);
    officecfg::Office::Common::Misc::OpenCLAllowList::set(SetOfImplMatcherToStringSequence(maAllowList), batch);

    batch->commit();
}

bool OpenCLConfig::checkImplementation(const OpenCLPlatformInfo& rPlatform, const OpenCLDeviceInfo& rDevice) const
{
    // Check denylist of known bad OpenCL implementations
    if (match(maDenyList, rPlatform, rDevice, "denylist"))
    {
        SAL_INFO("opencl", "Rejecting");
        return true;
    }

    // Check for allowlist of known good OpenCL implementations
    if (match(maAllowList, rPlatform, rDevice, "allowlist"))
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
        "DenyList=" << rConfig.maDenyList << ","
        "AllowList=" << rConfig.maAllowList <<
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
