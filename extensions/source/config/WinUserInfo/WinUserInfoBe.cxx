/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WinUserInfoBe.hxx"
#include <o3tl/make_unique.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <com/sun/star/beans/Optional.hpp>
#include <comphelper/configurationhelper.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <string.h>
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
class ADsUserAccess : public extensions::config::WinUserInfo::WinUserInfoBe_Impl
{
public:
    ADsUserAccess(const css::uno::Reference<css::uno::XComponentContext>& xContext)
    {
        try
        {
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
            OUString sLdapUserDN = "LDAP://" + OUString(o3tl::toU(sUserDN));
            IADsUser* pUser;
            hr = ADsGetObject(o3tl::toW(sLdapUserDN.getStr()), IID_IADsUser, (void**)&pUser);
            if (FAILED(hr))
                throw css::uno::RuntimeException();
            CoIfPtr<IADsUser> pUserGuard(pUser);
            // Fetch all the required information right now, when we know to have access to AD
            // (later the connection may already be lost)
            m_sGivenName = Str(pUser, &IADsUser::get_FirstName);
            m_sSn = Str(pUser, &IADsUser::get_LastName);
            m_sInitials = Str(pUser, L"initials");
            m_sStreet = Str(pUser, L"streetAddress");
            m_sCity = Str(pUser, L"l");
            m_sState = Str(pUser, L"st");
            m_sPostalCode = Str(pUser, L"postalCode");
            m_sCountry = Str(pUser, L"co");
            m_sOrganization = Str(pUser, L"company");
            m_sTitle = Str(pUser, &IADsUser::get_Title);
            m_sHomePhone = Str(pUser, L"homePhone");
            m_sTelephoneNumber = Str(pUser, L"TelephoneNumber");
            m_sFaxNumber = Str(pUser, L"facsimileTelephoneNumber");
            m_sMail = Str(pUser, &IADsUser::get_EmailAddress);

            CacheData(xContext);
        }
        catch (css::uno::Exception&)
        {
            // Maybe we temporarily lost connection to AD; try to get cached data
            GetCachedData(xContext);
        }
    }
    ~ADsUserAccess() {}

    virtual OUString GetGivenName() override { return m_sGivenName; }
    virtual OUString GetSn() override { return m_sSn; }
    virtual OUString GetInitials() override { return m_sInitials; }
    virtual OUString GetStreet() override { return m_sStreet; }
    virtual OUString GetCity() override { return m_sCity; }
    virtual OUString GetState() override { return m_sState; }
    virtual OUString GetPostalCode() override { return m_sPostalCode; }
    virtual OUString GetCountry() override { return m_sCountry; }
    virtual OUString GetOrganization() override { return m_sOrganization; }
    virtual OUString GetTitle() override { return m_sTitle; }
    virtual OUString GetHomePhone() override { return m_sHomePhone; }
    virtual OUString GetTelephoneNumber() override { return m_sTelephoneNumber; }
    virtual OUString GetFaxNumber() override { return m_sFaxNumber; }
    virtual OUString GetMail() override { return m_sMail; }

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
    struct CoInitializeGuard
    {
        CoInitializeGuard()
        {
            if (FAILED(CoInitialize(nullptr)))
                throw css::uno::RuntimeException();
        }
        ~CoInitializeGuard() { CoUninitialize(); }
    } m_CoInitializeGuard;

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
            auto xIface = comphelper::ConfigurationHelper::openConfig(
                xContext, "org.openoffice.UserProfile/WinUserInfoCache",
                comphelper::EConfigurationModes::Standard);
            css::uno::Reference<css::container::XNameReplace> xNameReplace(
                xIface, css::uno::UNO_QUERY_THROW);
            xNameReplace->replaceByName("isSet", css::uno::makeAny(true));
            xNameReplace->replaceByName("givenname", css::uno::makeAny(m_sGivenName));
            xNameReplace->replaceByName("sn", css::uno::makeAny(m_sSn));
            xNameReplace->replaceByName("initials", css::uno::makeAny(m_sInitials));
            xNameReplace->replaceByName("street", css::uno::makeAny(m_sStreet));
            xNameReplace->replaceByName("l", css::uno::makeAny(m_sCity));
            xNameReplace->replaceByName("st", css::uno::makeAny(m_sState));
            xNameReplace->replaceByName("postalcode", css::uno::makeAny(m_sPostalCode));
            xNameReplace->replaceByName("c", css::uno::makeAny(m_sCountry));
            xNameReplace->replaceByName("o", css::uno::makeAny(m_sOrganization));
            xNameReplace->replaceByName("title", css::uno::makeAny(m_sTitle));
            xNameReplace->replaceByName("homephone", css::uno::makeAny(m_sHomePhone));
            xNameReplace->replaceByName("telephonenumber", css::uno::makeAny(m_sTelephoneNumber));
            xNameReplace->replaceByName("facsimiletelephonenumber",
                                        css::uno::makeAny(m_sFaxNumber));
            xNameReplace->replaceByName("mail", css::uno::makeAny(m_sMail));

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
        auto xIface = comphelper::ConfigurationHelper::openConfig(
            xContext, "org.openoffice.UserProfile/WinUserInfoCache",
            comphelper::EConfigurationModes::ReadOnly);
        css::uno::Reference<css::container::XNameAccess> xNameAccess(xIface,
                                                                     css::uno::UNO_QUERY_THROW);
        bool bSet = false;
        xNameAccess->getByName("isSet") >>= bSet;
        if (!bSet)
            throw css::uno::RuntimeException();

        xNameAccess->getByName("givenname") >>= m_sGivenName;
        xNameAccess->getByName("sn") >>= m_sSn;
        xNameAccess->getByName("initials") >>= m_sInitials;
        xNameAccess->getByName("street") >>= m_sStreet;
        xNameAccess->getByName("l") >>= m_sCity;
        xNameAccess->getByName("st") >>= m_sState;
        xNameAccess->getByName("postalcode") >>= m_sPostalCode;
        xNameAccess->getByName("c") >>= m_sCountry;
        xNameAccess->getByName("o") >>= m_sOrganization;
        xNameAccess->getByName("title") >>= m_sTitle;
        xNameAccess->getByName("homephone") >>= m_sHomePhone;
        xNameAccess->getByName("telephonenumber") >>= m_sTelephoneNumber;
        xNameAccess->getByName("facsimiletelephonenumber") >>= m_sFaxNumber;
        xNameAccess->getByName("mail") >>= m_sMail;
    }

    OUString m_sGivenName;
    OUString m_sSn;
    OUString m_sInitials;
    OUString m_sStreet;
    OUString m_sCity;
    OUString m_sState;
    OUString m_sPostalCode;
    OUString m_sCountry;
    OUString m_sOrganization;
    OUString m_sTitle;
    OUString m_sHomePhone;
    OUString m_sTelephoneNumber;
    OUString m_sFaxNumber;
    OUString m_sMail;
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
            auto pNameBuf(o3tl::make_unique<wchar_t[]>(nSize));
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
            auto pNameBuf(o3tl::make_unique<wchar_t[]>(nSize));
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
    if (sToken == "givenname")
    {
        sValue = m_pImpl->GetGivenName();
    }
    else if (sToken == "sn")
    {
        sValue = m_pImpl->GetSn();
    }
    else if (sToken == "fathersname")
    {
        sValue = m_pImpl->GetFathersname();
    }
    else if (sToken == "initials")
    {
        sValue = m_pImpl->GetInitials();
    }
    else if (sToken == "street")
    {
        sValue = m_pImpl->GetStreet();
    }
    else if (sToken == "l")
    {
        sValue = m_pImpl->GetCity();
    }
    else if (sToken == "st")
    {
        sValue = m_pImpl->GetState();
    }
    else if (sToken == "apartment")
    {
        sValue = m_pImpl->GetApartment();
    }
    else if (sToken == "postalcode")
    {
        sValue = m_pImpl->GetPostalCode();
    }
    else if (sToken == "c")
    {
        sValue = m_pImpl->GetCountry();
    }
    else if (sToken == "o")
    {
        sValue = m_pImpl->GetOrganization();
    }
    else if (sToken == "position")
    {
        sValue = m_pImpl->GetPosition();
    }
    else if (sToken == "title")
    {
        sValue = m_pImpl->GetTitle();
    }
    else if (sToken == "homephone")
    {
        sValue = m_pImpl->GetHomePhone();
    }
    else if (sToken == "telephonenumber")
    {
        sValue = m_pImpl->GetTelephoneNumber();
    }
    else if (sToken == "facsimiletelephonenumber")
    {
        sValue = m_pImpl->GetFaxNumber();
    }
    else if (sToken == "mail")
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
