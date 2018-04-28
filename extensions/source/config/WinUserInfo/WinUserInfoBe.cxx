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

#include "WinUserInfoBe.hxx"
#include <o3tl/make_unique.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <com/sun/star/beans/Optional.hpp>
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
    ADsUserAccess()
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
        m_pUser.reset(pUser);
    }
    ~ADsUserAccess() {}

    virtual OUString GetGivenName() override { return Str(&IADsUser::get_FirstName); }
    virtual OUString GetSn() override { return Str(&IADsUser::get_LastName); }
    virtual OUString GetInitials() override { return Str(L"initials"); }
    virtual OUString GetStreet() override { return Str(L"streetAddress"); }
    virtual OUString GetCity() override { return Str(L"l"); }
    virtual OUString GetState() override { return Str(L"st"); }
    virtual OUString GetPostalCode() override { return Str(L"postalCode"); }
    virtual OUString GetCountry() override { return Str(L"co"); }
    virtual OUString GetOrganization() override { return Str(L"company"); }
    virtual OUString GetTitle() override { return Str(&IADsUser::get_Title); }
    virtual OUString GetHomePhone() override { return Str(L"homePhone"); }
    virtual OUString GetTelephoneNumber() override { return Str(L"TelephoneNumber"); }
    virtual OUString GetFaxNumber() override { return Str(L"facsimileTelephoneNumber"); }
    virtual OUString GetMail() override { return Str(&IADsUser::get_EmailAddress); }

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
    OUString Str(getstrfunc func)
    {
        BSTR sBstr;
        if (FAILED((m_pUser.get()->*func)(&sBstr)))
            return "";
        BSTRGuard aBstrGuard(sBstr, SysFreeString);
        return o3tl::toU(sBstr);
    }
    OUString Str(const wchar_t* property)
    {
        BSTRGuard sBstrProp(SysAllocString(property), SysFreeString);
        struct AutoVariant : public VARIANT
        {
            AutoVariant() { VariantInit(this); }
            ~AutoVariant() { VariantClear(this); }
        } varArr;
        if (FAILED(m_pUser->GetEx(sBstrProp.get(), &varArr)))
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

    CoIfPtr<IADsUser> m_pUser;
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
WinUserInfoBe::WinUserInfoBe(const css::uno::Reference<css::uno::XComponentContext>&)
    : WinUserInfoMutexHolder()
    , BackendBase(mMutex)
{
    try
    {
        m_pImpl.reset(new ADsUserAccess);
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
