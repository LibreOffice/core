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
#include <comphelper/configuration.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <map>
#include <o3tl/char16_t2wchar_t.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <officecfg/UserProfile.hxx>

#include <Iads.h>
#include <Adshlp.h>
#include <Lmcons.h>
#define SECURITY_WIN32
#include <Security.h>

#include <systools/win32/comtools.hxx>
#include <systools/win32/oleauto.hxx>

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
constexpr OUString _givenname(u"givenname"_ustr);
constexpr OUString _sn(u"sn"_ustr);
constexpr char _fathersname[]("fathersname");
constexpr OUString _initials(u"initials"_ustr);
constexpr OUString _street(u"street"_ustr);
constexpr OUString _l(u"l"_ustr);
constexpr OUString _st(u"st"_ustr);
constexpr char _apartment[]("apartment");
constexpr OUString _postalcode(u"postalcode"_ustr);
constexpr OUString _c(u"c"_ustr);
constexpr OUString _o(u"o"_ustr);
constexpr char _position[]("position");
constexpr OUString _title(u"title"_ustr);
constexpr OUString _homephone(u"homephone"_ustr);
constexpr OUString _telephonenumber(u"telephonenumber"_ustr);
constexpr OUString _facsimiletelephonenumber(u"facsimiletelephonenumber"_ustr);
constexpr OUString _mail(u"mail"_ustr);

// Backend class implementing access to Active Directory user data. It caches its encoded data
// in a configuration entry, to allow reusing it when user later doesn't have access to AD DC
// (otherwise the user would get different data when connected vs not connected).
class ADsUserAccess : public extensions::config::WinUserInfo::WinUserInfoBe_Impl
{
public:
    ADsUserAccess()
    {
        try
        {
            sal::systools::CoInitializeGuard aCoInitializeGuard(COINIT_APARTMENTTHREADED);

            sal::systools::COMReference<IADsADSystemInfo> pADsys(CLSID_ADSystemInfo, nullptr,
                                                                 CLSCTX_INPROC_SERVER);

            sal::systools::BStr sUserDN;
            sal::systools::ThrowIfFailed(pADsys->get_UserName(&sUserDN), "get_UserName failed");
            // If this user is an AD user, then without an active connection to the domain, all the
            // above will succeed, and m_sUserDN will be correctly initialized, but the following
            // call to ADsGetObject will fail, and we will attempt reading cached values.
            m_sUserDN = sUserDN;
            OUString sLdapUserDN = "LDAP://" + m_sUserDN;
            sal::systools::COMReference<IADsUser> pUser;
            sal::systools::ThrowIfFailed(ADsGetObject(o3tl::toW(sLdapUserDN.getStr()), IID_IADsUser,
                                                      reinterpret_cast<void**>(&pUser)),
                                         "ADsGetObject failed");
            // Fetch all the required information right now, when we know to have access to AD
            // (later the connection may already be lost)
            m_aMap[_givenname] = Str(pUser, &IADsUser::get_FirstName);
            m_aMap[_sn] = Str(pUser, &IADsUser::get_LastName);
            m_aMap[_initials] = Str(pUser, L"initials");
            m_aMap[_street] = Str(pUser, L"streetAddress");
            m_aMap[_l] = Str(pUser, L"l");
            m_aMap[_st] = Str(pUser, L"st");
            m_aMap[_postalcode] = Str(pUser, L"postalCode");
            m_aMap[_c] = Str(pUser, L"co");
            m_aMap[_o] = Str(pUser, L"company");
            m_aMap[_title] = Str(pUser, &IADsUser::get_Title);
            m_aMap[_homephone] = Str(pUser, L"homePhone");
            m_aMap[_telephonenumber] = Str(pUser, L"TelephoneNumber");
            m_aMap[_facsimiletelephonenumber] = Str(pUser, L"facsimileTelephoneNumber");
            m_aMap[_mail] = Str(pUser, &IADsUser::get_EmailAddress);

            CacheData();
        }
        catch (sal::systools::ComError&)
        {
            // Maybe we temporarily lost connection to AD; try to get cached data
            GetCachedData();
        }
    }

    virtual OUString GetGivenName() override { return m_aMap[_givenname]; }
    virtual OUString GetSn() override { return m_aMap[_sn]; }
    virtual OUString GetInitials() override { return m_aMap[_initials]; }
    virtual OUString GetStreet() override { return m_aMap[_street]; }
    virtual OUString GetCity() override { return m_aMap[_l]; }
    virtual OUString GetState() override { return m_aMap[_st]; }
    virtual OUString GetPostalCode() override { return m_aMap[_postalcode]; }
    virtual OUString GetCountry() override { return m_aMap[_c]; }
    virtual OUString GetOrganization() override { return m_aMap[_o]; }
    virtual OUString GetTitle() override { return m_aMap[_title]; }
    virtual OUString GetHomePhone() override { return m_aMap[_homephone]; }
    virtual OUString GetTelephoneNumber() override { return m_aMap[_telephonenumber]; }
    virtual OUString GetFaxNumber() override { return m_aMap[_facsimiletelephonenumber]; }
    virtual OUString GetMail() override { return m_aMap[_mail]; }

private:
    typedef HRESULT (__stdcall IADsUser::*getstrfunc)(BSTR*);
    static OUString Str(IADsUser* pUser, getstrfunc func)
    {
        sal::systools::BStr sBstr;
        if (FAILED((pUser->*func)(&sBstr)))
            return "";
        return OUString(sBstr);
    }
    static OUString Str(IADsUser* pUser, const wchar_t* property)
    {
        struct AutoVariant : public VARIANT
        {
            AutoVariant() { VariantInit(this); }
            ~AutoVariant() { VariantClear(this); }
        } varArr;
        if (FAILED(pUser->GetEx(sal::systools::BStr(o3tl::toU(property)), &varArr)))
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
                return OUString(o3tl::toU(V_BSTR(&varItem)));
            VariantClear(&varItem);
        }
        return "";
    }

    void CacheData()
    {
        try
        {
            OUString sCachedData = "user=" + m_sUserDN // user DN
                                   + "\0" + _givenname + "=" + GetGivenName() // 1st name
                                   + "\0" + _sn + "=" + GetSn() // sn
                                   + "\0" + _initials + "=" + GetInitials() // initials
                                   + "\0" + _street + "=" + GetStreet() // street
                                   + "\0" + _l + "=" + GetCity() // l
                                   + "\0" + _st + "=" + GetState() // st
                                   + "\0" + _postalcode + "=" + GetPostalCode() // p.code
                                   + "\0" + _c + "=" + GetCountry() // c
                                   + "\0" + _o + "=" + GetOrganization() // o
                                   + "\0" + _title + "=" + GetTitle() // title
                                   + "\0" + _homephone + "=" + GetHomePhone() // h.phone
                                   + "\0" + _telephonenumber + "=" + GetTelephoneNumber() // tel
                                   + "\0" + _facsimiletelephonenumber + "=" + GetFaxNumber() // fax
                                   + "\0" + _mail + "=" + GetMail(); // mail
            const css::uno::Sequence<sal_Int8> seqCachedData(
                reinterpret_cast<const sal_Int8*>(sCachedData.getStr()),
                sCachedData.getLength() * sizeof(sal_Unicode));
            OUStringBuffer sOutBuf;
            comphelper::Base64::encode(sOutBuf, seqCachedData);

            std::shared_ptr<comphelper::ConfigurationChanges> batch(
                comphelper::ConfigurationChanges::create());
            officecfg::UserProfile::WinUserInfo::Cache::set(sOutBuf.makeStringAndClear(), batch);
            batch->commit();
        }
        catch (const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("extensions.config",
                                 "ADsUserAccess: access to configuration data failed:");
        }
    }

    void GetCachedData()
    {
        if (m_sUserDN.isEmpty())
            throw css::uno::RuntimeException();

        OUString sCache = officecfg::UserProfile::WinUserInfo::Cache::get();

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
WinUserInfoBe::WinUserInfoBe()
    : WinUserInfoMutexHolder()
    , BackendBase(mMutex)
{
    try
    {
        m_pImpl.reset(new ADsUserAccess());
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
    if (sToken == _givenname)
    {
        sValue = m_pImpl->GetGivenName();
    }
    else if (sToken == _sn)
    {
        sValue = m_pImpl->GetSn();
    }
    else if (sToken == _fathersname)
    {
        sValue = m_pImpl->GetFathersname();
    }
    else if (sToken == _initials)
    {
        sValue = m_pImpl->GetInitials();
    }
    else if (sToken == _street)
    {
        sValue = m_pImpl->GetStreet();
    }
    else if (sToken == _l)
    {
        sValue = m_pImpl->GetCity();
    }
    else if (sToken == _st)
    {
        sValue = m_pImpl->GetState();
    }
    else if (sToken == _apartment)
    {
        sValue = m_pImpl->GetApartment();
    }
    else if (sToken == _postalcode)
    {
        sValue = m_pImpl->GetPostalCode();
    }
    else if (sToken == _c)
    {
        sValue = m_pImpl->GetCountry();
    }
    else if (sToken == _o)
    {
        sValue = m_pImpl->GetOrganization();
    }
    else if (sToken == _position)
    {
        sValue = m_pImpl->GetPosition();
    }
    else if (sToken == _title)
    {
        sValue = m_pImpl->GetTitle();
    }
    else if (sToken == _homephone)
    {
        sValue = m_pImpl->GetHomePhone();
    }
    else if (sToken == _telephonenumber)
    {
        sValue = m_pImpl->GetTelephoneNumber();
    }
    else if (sToken == _facsimiletelephonenumber)
    {
        sValue = m_pImpl->GetFaxNumber();
    }
    else if (sToken == _mail)
    {
        sValue = m_pImpl->GetMail();
    }
    else
        throw css::beans::UnknownPropertyException(sToken, static_cast<cppu::OWeakObject*>(this));

    return css::uno::Any(css::beans::Optional<css::uno::Any>(
        !sValue.isEmpty(), sValue.isEmpty() ? css::uno::Any() : css::uno::Any(sValue)));
}

OUString SAL_CALL WinUserInfoBe::getImplementationName()
{
    return "com.sun.star.comp.configuration.backend.WinUserInfoBe";
}

sal_Bool SAL_CALL WinUserInfoBe::supportsService(const OUString& aServiceName)
{
    return cppu::supportsService(this, aServiceName);
}

css::uno::Sequence<OUString> SAL_CALL WinUserInfoBe::getSupportedServiceNames()
{
    return { "com.sun.star.configuration.backend.WinUserInfoBe" };
}
}
}
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_WinUserInfoBe_get_implementation(css::uno::XComponentContext*,
                                            css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new extensions::config::WinUserInfo::WinUserInfoBe());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
