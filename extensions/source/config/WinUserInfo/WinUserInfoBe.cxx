/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WinUserInfoBe.hxx"

#include <com/sun/star/beans/Optional.hpp>
#include <comphelper/configurationhelper.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <map>
#include <sax/tools/converter.hxx>

#include <Iads.h>
#include <Adshlp.h>
#include <Lmcons.h>
#define SECURITY_WIN32
#include <Security.h>

namespace extensions
{
namespace config
{
namespace WinUserInfo
{
class WinUserInfoBe_Impl
{
public:
    virtual ~WinUserInfoBe_Impl(){};
    virtual OUString GetGivenName() = 0;
    virtual OUString GetSn() { return ""; }
    virtual OUString GetFathersname() { return ""; }
    virtual OUString GetInitials() { return ""; }
    virtual OUString GetStreet() { return ""; }
    virtual OUString GetCity() { return ""; }
    virtual OUString GetState() { return ""; }
    virtual OUString GetApartment() { return ""; }
    virtual OUString GetPostalCode() { return ""; }
    virtual OUString GetCountry() { return ""; }
    virtual OUString GetOrganization() { return ""; }
    virtual OUString GetPosition() { return ""; }
    virtual OUString GetTitle() { return ""; }
    virtual OUString GetHomePhone() { return ""; }
    virtual OUString GetTelephoneNumber() { return ""; }
    virtual OUString GetFaxNumber() { return ""; }
    virtual OUString GetMail() { return ""; }
};
}
}
}

namespace
{
const char givenname[]("givenname");
const char sn[]("sn");
const char fathersname[]("fathersname");
const char initials[]("initials");
const char street[]("street");
const char l[]("l");
const char st[]("st");
const char apartment[]("apartment");
const char postalcode[]("postalcode");
const char c[]("c");
const char o[]("o");
const char position[]("position");
const char title[]("title");
const char homephone[]("homephone");
const char telephonenumber[]("telephonenumber");
const char facsimiletelephonenumber[]("facsimiletelephonenumber");
const char mail[]("mail");

// Backend class implementing access to Active Directory user data. It caches its encoded data
// in a configuration entry, to allow reusing it when user later doesn't have access to AD DC
// (otherwise the user would get different data when connected vs not connected).
class ADsUserAccess : public extensions::config::WinUserInfo::WinUserInfoBe_Impl
{
public:
    ADsUserAccess(const css::uno::Reference<css::uno::XComponentContext>& xContext)
    {
        try
        {
            struct CoInitializeGuard
            {
                CoInitializeGuard()
                {
                    if (FAILED(CoInitialize(nullptr)))
                        throw css::uno::RuntimeException();
                }
                ~CoInitializeGuard() { CoUninitialize(); }
            } aCoInitializeGuard;

            IADsADSystemInfo* pADsys;
            HRESULT hr = CoCreateInstance(CLSID_ADSystemInfo, nullptr, CLSCTX_INPROC_SERVER,
                                          IID_IADsADSystemInfo, (void**)&pADsys);
            if (FAILED(hr))
                throw css::uno::RuntimeException();
            CoIfPtr<IADsADSystemInfo> aADsysGuard(pADsys);

            BSTR sUserDN;
            hr = pADsys->get_UserName(&sUserDN);
            if (FAILED(hr))
                throw css::uno::RuntimeException();
            BSTRGuard aUserNameGuard(sUserDN, SysFreeString);
            // If this user is an AD user, then without an active connection to the domain, all the
            // above will succeed, and m_sUserDN will be correctly initialized, but the following
            // call to ADsGetObject will fail, and we will attempt reading cached values.
            m_sUserDN = reinterpret_cast<const sal_Unicode*>(sUserDN);
            OUString sLdapUserDN = "LDAP://" + m_sUserDN;
            IADsUser* pUser;
            hr = ADsGetObject(reinterpret_cast<const wchar_t*>(sLdapUserDN.getStr()), IID_IADsUser, (void**)&pUser);
            if (FAILED(hr))
                throw css::uno::RuntimeException();
            CoIfPtr<IADsUser> pUserGuard(pUser);
            // Fetch all the required information right now, when we know to have access to AD
            // (later the connection may already be lost)
            m_aMap[givenname] = Str(pUser, &IADsUser::get_FirstName);
            m_aMap[sn] = Str(pUser, &IADsUser::get_LastName);
            m_aMap[initials] = Str(pUser, L"initials");
            m_aMap[street] = Str(pUser, L"streetAddress");
            m_aMap[l] = Str(pUser, L"l");
            m_aMap[st] = Str(pUser, L"st");
            m_aMap[postalcode] = Str(pUser, L"postalCode");
            m_aMap[c] = Str(pUser, L"co");
            m_aMap[o] = Str(pUser, L"company");
            m_aMap[title] = Str(pUser, &IADsUser::get_Title);
            m_aMap[homephone] = Str(pUser, L"homePhone");
            m_aMap[telephonenumber] = Str(pUser, L"TelephoneNumber");
            m_aMap[facsimiletelephonenumber] = Str(pUser, L"facsimileTelephoneNumber");
            m_aMap[mail] = Str(pUser, &IADsUser::get_EmailAddress);

            CacheData(xContext);
        }
        catch (css::uno::Exception&)
        {
            // Maybe we temporarily lost connection to AD; try to get cached data
            GetCachedData(xContext);
        }
    }
    ~ADsUserAccess() {}

    virtual OUString GetGivenName() override { return m_aMap[givenname]; }
    virtual OUString GetSn() override { return m_aMap[sn]; }
    virtual OUString GetInitials() override { return m_aMap[initials]; }
    virtual OUString GetStreet() override { return m_aMap[street]; }
    virtual OUString GetCity() override { return m_aMap[l]; }
    virtual OUString GetState() override { return m_aMap[st]; }
    virtual OUString GetPostalCode() override { return m_aMap[postalcode]; }
    virtual OUString GetCountry() override { return m_aMap[c]; }
    virtual OUString GetOrganization() override { return m_aMap[o]; }
    virtual OUString GetTitle() override { return m_aMap[title]; }
    virtual OUString GetHomePhone() override { return m_aMap[homephone]; }
    virtual OUString GetTelephoneNumber() override { return m_aMap[telephonenumber]; }
    virtual OUString GetFaxNumber() override { return m_aMap[facsimiletelephonenumber]; }
    virtual OUString GetMail() override { return m_aMap[mail]; }

private:
    static void ReleaseIUnknown(IUnknown* p)
    {
        if (p)
            p->Release();
    }
    template <class If> class CoIfPtr : public std::unique_ptr<If, decltype(&ReleaseIUnknown)>
    {
    public:
        CoIfPtr(If* p = nullptr)
            : std::unique_ptr<If, decltype(&ReleaseIUnknown)>(p, ReleaseIUnknown)
        {
        }
    };
    typedef std::unique_ptr<OLECHAR, decltype(&SysFreeString)> BSTRGuard;

    typedef HRESULT (__stdcall IADsUser::*getstrfunc)(BSTR*);
    static OUString Str(IADsUser* pUser, getstrfunc func)
    {
        BSTR sBstr;
        if (FAILED((pUser->*func)(&sBstr)))
            return "";
        BSTRGuard aBstrGuard(sBstr, SysFreeString);
        return reinterpret_cast<const sal_Unicode*>(sBstr);
    }
    static OUString Str(IADsUser* pUser, const wchar_t* property)
    {
        BSTRGuard sBstrProp(SysAllocString(property), SysFreeString);
        struct AutoVariant : public VARIANT
        {
            AutoVariant() { VariantInit(this); }
            ~AutoVariant() { VariantClear(this); }
        } varArr;
        if (FAILED(pUser->GetEx(sBstrProp.get(), &varArr)))
            return "";
        SAFEARRAY* sa = V_ARRAY(&varArr);
        LONG nStart, nEnd;
        if (FAILED(SafeArrayGetLBound(sa, 1, &nStart)) || FAILED(SafeArrayGetUBound(sa, 1, &nEnd)))
            return "";
        AutoVariant varItem;
        for (LONG i = nStart; i <= nEnd; i++)
        {
            if (FAILED(SafeArrayGetElement(sa, &i, &varItem)))
                continue;
            if (varItem.vt == VT_BSTR)
                return reinterpret_cast<const sal_Unicode*>(V_BSTR(&varItem));
            VariantClear(&varItem);
        }
        return "";
    }

    void CacheData(const css::uno::Reference<css::uno::XComponentContext>& xContext)
    {
        try
        {
            const OUString sNul('\0');
            OUStringBuffer sCachedData(200);
            sCachedData.append("user=" + m_sUserDN);
            sCachedData.append(sNul + givenname + "=" + GetGivenName());
            sCachedData.append(sNul + sn + "=" + GetSn());
            sCachedData.append(sNul + initials + "=" + GetInitials());
            sCachedData.append(sNul + street + "=" + GetStreet());
            sCachedData.append(sNul + l + "=" + GetCity());
            sCachedData.append(sNul + st + "=" + GetState());
            sCachedData.append(sNul + postalcode + "=" + GetPostalCode());
            sCachedData.append(sNul + c + "=" + GetCountry());
            sCachedData.append(sNul + o + "=" + GetOrganization());
            sCachedData.append(sNul + title + "=" + GetTitle());
            sCachedData.append(sNul + homephone + "=" + GetHomePhone());
            sCachedData.append(sNul + telephonenumber + "=" + GetTelephoneNumber());
            sCachedData.append(sNul + facsimiletelephonenumber + "=" + GetFaxNumber());
            sCachedData.append(sNul + mail + "=" + GetMail());
            const css::uno::Sequence<sal_Int8> seqCachedData(
                reinterpret_cast<const sal_Int8*>(sCachedData.getStr()),
                sCachedData.getLength() * sizeof(sal_Unicode));
            OUStringBuffer sOutBuf;
            sax::Converter::encodeBase64(sOutBuf, seqCachedData);

            auto xIface = comphelper::ConfigurationHelper::openConfig(
                xContext, "org.openoffice.UserProfile/WinUserInfo",
                comphelper::EConfigurationModes::Standard);
            css::uno::Reference<css::container::XNameReplace> xNameReplace(
                xIface, css::uno::UNO_QUERY_THROW);
            xNameReplace->replaceByName("Cache", css::uno::makeAny(sOutBuf.makeStringAndClear()));

            css::uno::Reference<css::util::XChangesBatch> xChangesBatch(xIface,
                                                                        css::uno::UNO_QUERY_THROW);
            xChangesBatch->commitChanges();
        }
        catch (const css::uno::Exception& e)
        {
            OSL_TRACE("ADsUserAccess: access to configuration data failed: %s",
                OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        }
    }

    void GetCachedData(const css::uno::Reference<css::uno::XComponentContext>& xContext)
    {
        if (m_sUserDN.isEmpty())
            throw css::uno::RuntimeException();

        auto xIface = comphelper::ConfigurationHelper::openConfig(
            xContext, "org.openoffice.UserProfile/WinUserInfo",
            comphelper::EConfigurationModes::ReadOnly);
        css::uno::Reference<css::container::XNameAccess> xNameAccess(xIface,
                                                                     css::uno::UNO_QUERY_THROW);
        OUString sCache;
        xNameAccess->getByName("Cache") >>= sCache;
        if (sCache.isEmpty())
            throw css::uno::RuntimeException();

        {
            css::uno::Sequence<sal_Int8> seqCachedData;
            sax::Converter::decodeBase64(seqCachedData, sCache);
            sCache = OUString(reinterpret_cast<const sal_Unicode*>(seqCachedData.getConstArray()),
                              seqCachedData.getLength() / sizeof(sal_Unicode));
        }

        OUString sUserDN;
        std::map<const OUString, OUString> aMap;
        sal_Int32 nIndex = 0;
        do
        {
            const OUString sEntry = sCache.getToken(0, '\0', nIndex);
            sal_Int32 nEqIndex = 0;
            const OUString sEntryName = sEntry.getToken(0, '=', nEqIndex);
            OUString sEntryVal;
            if (nEqIndex >= 0)
                sEntryVal = sEntry.copy(nEqIndex);
            if (sEntryName == "user")
                sUserDN = sEntryVal;
            else
                aMap[sEntryName] = sEntryVal;
        } while (nIndex >= 0);

        if (sUserDN != m_sUserDN)
            throw css::uno::RuntimeException();
        m_aMap = std::move(aMap);
    }

    OUString m_sUserDN; // used to check if the cached data is for current user
    std::map<const OUString, OUString> m_aMap;
};

class SysInfoUserAccess : public extensions::config::WinUserInfo::WinUserInfoBe_Impl
{
public:
    SysInfoUserAccess()
    {
        try
        {
            ULONG nSize = 0;
            GetUserNameExW(NameDisplay, nullptr, &nSize);
            if (GetLastError() != ERROR_MORE_DATA)
                throw css::uno::RuntimeException();
            std::unique_ptr<wchar_t[]> pNameBuf(new wchar_t[nSize]);
            if (!GetUserNameExW(NameDisplay, pNameBuf.get(), &nSize))
                throw css::uno::RuntimeException();
            m_sName = reinterpret_cast<const sal_Unicode*>(pNameBuf.get());
        }
        catch (css::uno::RuntimeException&)
        {
            // GetUserNameEx may fail in some cases (e.g., for built-in AD domain
            // administrator account on non-DC systems), where GetUserName will
            // still give a name.
            DWORD nSize = UNLEN + 1;
            std::unique_ptr<wchar_t[]> pNameBuf(new wchar_t[nSize]);
            if (!GetUserNameW(pNameBuf.get(), &nSize))
                throw css::uno::RuntimeException();
            m_sName = reinterpret_cast<const sal_Unicode*>(pNameBuf.get());
        }
    }

    virtual OUString GetGivenName() override { return m_sName; }

private:
    OUString m_sName;
};
}

namespace extensions
{
namespace config
{
namespace WinUserInfo
{
WinUserInfoBe::WinUserInfoBe(const css::uno::Reference<css::uno::XComponentContext>& xContext)
    : WinUserInfoMutexHolder()
    , BackendBase(mMutex)
{
    try
    {
        m_pImpl.reset(new ADsUserAccess(xContext));
    }
    catch (css::uno::RuntimeException&)
    {
        m_pImpl.reset(new SysInfoUserAccess);
    }
}

WinUserInfoBe::~WinUserInfoBe() {}

void WinUserInfoBe::setPropertyValue(OUString const&, css::uno::Any const&)
{
    throw css::lang::IllegalArgumentException("setPropertyValue not supported",
                                              static_cast<cppu::OWeakObject*>(this), -1);
}

css::uno::Any WinUserInfoBe::getPropertyValue(OUString const& PropertyName)
{
    OUString sValue;
    // Only process the first argument of possibly multiple space- or comma-separated arguments
    OUString sToken = PropertyName.getToken(0, ' ').getToken(0, ',');
    if (sToken == givenname)
    {
        sValue = m_pImpl->GetGivenName();
    }
    else if (sToken == sn)
    {
        sValue = m_pImpl->GetSn();
    }
    else if (sToken == fathersname)
    {
        sValue = m_pImpl->GetFathersname();
    }
    else if (sToken == initials)
    {
        sValue = m_pImpl->GetInitials();
    }
    else if (sToken == street)
    {
        sValue = m_pImpl->GetStreet();
    }
    else if (sToken == l)
    {
        sValue = m_pImpl->GetCity();
    }
    else if (sToken == st)
    {
        sValue = m_pImpl->GetState();
    }
    else if (sToken == apartment)
    {
        sValue = m_pImpl->GetApartment();
    }
    else if (sToken == postalcode)
    {
        sValue = m_pImpl->GetPostalCode();
    }
    else if (sToken == c)
    {
        sValue = m_pImpl->GetCountry();
    }
    else if (sToken == o)
    {
        sValue = m_pImpl->GetOrganization();
    }
    else if (sToken == position)
    {
        sValue = m_pImpl->GetPosition();
    }
    else if (sToken == title)
    {
        sValue = m_pImpl->GetTitle();
    }
    else if (sToken == homephone)
    {
        sValue = m_pImpl->GetHomePhone();
    }
    else if (sToken == telephonenumber)
    {
        sValue = m_pImpl->GetTelephoneNumber();
    }
    else if (sToken == facsimiletelephonenumber)
    {
        sValue = m_pImpl->GetFaxNumber();
    }
    else if (sToken == mail)
    {
        sValue = m_pImpl->GetMail();
    }
    else
        throw css::beans::UnknownPropertyException(sToken, static_cast<cppu::OWeakObject*>(this));

    return css::uno::makeAny(css::beans::Optional<css::uno::Any>(
        !sValue.isEmpty(), sValue.isEmpty() ? css::uno::Any() : css::uno::makeAny(sValue)));
}

OUString WinUserInfoBe::getWinUserInfoBeName()
{
    return OUString("com.sun.star.comp.configuration.backend.WinUserInfoBe");
}

OUString SAL_CALL WinUserInfoBe::getImplementationName() { return getWinUserInfoBeName(); }

css::uno::Sequence<OUString> WinUserInfoBe::getWinUserInfoBeServiceNames()
{
    css::uno::Sequence<OUString> aServices{ "com.sun.star.configuration.backend.WinUserInfoBe" };
    return aServices;
}

sal_Bool SAL_CALL WinUserInfoBe::supportsService(const OUString& aServiceName)
{
    return cppu::supportsService(this, aServiceName);
}

css::uno::Sequence<OUString> SAL_CALL WinUserInfoBe::getSupportedServiceNames()
{
    return getWinUserInfoBeServiceNames();
}
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
