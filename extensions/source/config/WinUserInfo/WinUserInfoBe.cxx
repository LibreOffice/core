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
#include <comphelper/base64.hxx>
#include <comphelper/configurationhelper.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <map>
#include <o3tl/char16_t2wchar_t.hxx>
#include <sal/log.hxx>

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
constexpr char givenname[]("givenname");
constexpr char sn[]("sn");
constexpr char fathersname[]("fathersname");
constexpr char initials[]("initials");
constexpr char street[]("street");
constexpr char l[]("l");
constexpr char st[]("st");
constexpr char apartment[]("apartment");
constexpr char postalcode[]("postalcode");
constexpr char c[]("c");
constexpr char o[]("o");
constexpr char position[]("position");
constexpr char title[]("title");
constexpr char homephone[]("homephone");
constexpr char telephonenumber[]("telephonenumber");
constexpr char facsimiletelephonenumber[]("facsimiletelephonenumber");
constexpr char mail[]("mail");

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
                                          IID_IADsADSystemInfo, reinterpret_cast<void**>(&pADsys));
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
            m_sUserDN = o3tl::toU(sUserDN);
            OUString sLdapUserDN = "LDAP://" + m_sUserDN;
            IADsUser* pUser;
            hr = ADsGetObject(o3tl::toW(sLdapUserDN.getStr()), IID_IADsUser,
                              reinterpret_cast<void**>(&pUser));
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
        return o3tl::toU(sBstr);
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
                return o3tl::toU(V_BSTR(&varItem));
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
            sCachedData.append("user=").append(m_sUserDN);
            sCachedData.append(sNul).append(givenname).append("=").append(GetGivenName());
            sCachedData.append(sNul).append(sn).append("=").append(GetSn());
            sCachedData.append(sNul).append(initials).append("=").append(GetInitials());
            sCachedData.append(sNul).append(street).append("=").append(GetStreet());
            sCachedData.append(sNul).append(l).append("=").append(GetCity());
            sCachedData.append(sNul).append(st).append("=").append(GetState());
            sCachedData.append(sNul).append(postalcode).append("=").append(GetPostalCode());
            sCachedData.append(sNul).append(c).append("=").append(GetCountry());
            sCachedData.append(sNul).append(o).append("=").append(GetOrganization());
            sCachedData.append(sNul).append(title).append("=").append(GetTitle());
            sCachedData.append(sNul).append(homephone).append("=").append(GetHomePhone());
            sCachedData.append(sNul)
                .append(telephonenumber)
                .append("=")
                .append(GetTelephoneNumber());
            sCachedData.append(sNul)
                .append(facsimiletelephonenumber)
                .append("=")
                .append(GetFaxNumber());
            sCachedData.append(sNul).append(mail).append("=").append(GetMail());
            const css::uno::Sequence<sal_Int8> seqCachedData(
                reinterpret_cast<const sal_Int8*>(sCachedData.getStr()),
                sCachedData.getLength() * sizeof(sal_Unicode));
            OUStringBuffer sOutBuf;
            comphelper::Base64::encode(sOutBuf, seqCachedData);

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
            SAL_WARN("extensions.config",
                     "ADsUserAccess: access to configuration data failed: " << e);
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
            comphelper::Base64::decode(seqCachedData, sCache);
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
            auto pNameBuf(std::make_unique<wchar_t[]>(nSize));
            if (!GetUserNameExW(NameDisplay, pNameBuf.get(), &nSize))
                throw css::uno::RuntimeException();
            m_sName = o3tl::toU(pNameBuf.get());
        }
        catch (css::uno::RuntimeException&)
        {
            // GetUserNameEx may fail in some cases (e.g., for built-in AD domain
            // administrator account on non-DC systems), where GetUserName will
            // still give a name.
            DWORD nSize = UNLEN + 1;
            auto pNameBuf(std::make_unique<wchar_t[]>(nSize));
            if (!GetUserNameW(pNameBuf.get(), &nSize))
                throw css::uno::RuntimeException();
            m_sName = o3tl::toU(pNameBuf.get());
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
