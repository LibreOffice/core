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

#include <sal/config.h>

#include <unotools/useroptions.hxx>
#include <unotools/syslocale.hxx>
#include <com/sun/star/uno/Any.hxx>
#include "itemholder1.hxx"

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/util/ChangesEvent.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/enumarray.hxx>
#include <o3tl/string_view.hxx>
#include <comphelper/diagnose_ex.hxx>

using namespace utl;
using namespace com::sun::star;

// vOptionNames[] -- names of the user option entries
// The order must correspond to the enum class UserOptToken in useroptions.hxx.
constexpr o3tl::enumarray<UserOptToken, OUString> vOptionNames = {
    u"l"_ustr,                         // UserOptToken::City
    u"o"_ustr,                         // UserOptToken::Company
    u"c"_ustr,                         // UserOptToken::Country
    u"mail"_ustr,                      // UserOptToken::Email
    u"facsimiletelephonenumber"_ustr,  // UserOptToken::Fax
    u"givenname"_ustr,                 // UserOptToken::FirstName
    u"sn"_ustr,                        // UserOptToken::LastName
    u"position"_ustr,                  // UserOptToken::Position
    u"st"_ustr,                        // UserOptToken::State
    u"street"_ustr,                    // UserOptToken::Street
    u"homephone"_ustr,                 // UserOptToken::TelephoneHome
    u"telephonenumber"_ustr,           // UserOptToken::TelephoneWork
    u"title"_ustr,                     // UserOptToken::Title
    u"initials"_ustr,                  // UserOptToken::ID
    u"postalcode"_ustr,                // UserOptToken::Zip
    u"fathersname"_ustr,               // UserOptToken::FathersName
    u"apartment"_ustr,                 // UserOptToken::Apartment
    u"signingkey"_ustr,                // UserOptToken::SigningKey
    u"encryptionkey"_ustr,             // UserOptToken::EncryptionKey
    u"encrypttoself"_ustr              // UserOptToken::EncryptToSelf
};

std::weak_ptr<SvtUserOptions::Impl> SvtUserOptions::xSharedImpl;

class SvtUserOptions::ChangeListener : public cppu::WeakImplHelper<util::XChangesListener>
{
public:
    explicit ChangeListener (Impl& rParent): m_rParent(rParent) { }

    // XChangesListener
    virtual void SAL_CALL changesOccurred (util::ChangesEvent const& Event) override;
    // XEventListener
    virtual void SAL_CALL disposing (lang::EventObject const& Source) override;

private:
    Impl& m_rParent;
};

class SvtUserOptions::Impl : public utl::ConfigurationBroadcaster
{
public:
    Impl ();

    OUString GetFullName () const;

    bool IsTokenReadonly (UserOptToken nToken) const;
    OUString GetToken (UserOptToken nToken) const;
    void     SetToken (UserOptToken nToken, OUString const& rNewToken);
    bool     GetBoolValue (UserOptToken nToken) const;
    void     SetBoolValue (UserOptToken nToken, bool bNewValue);
    void     Notify ();

private:
    uno::Reference<util::XChangesListener> m_xChangeListener;
    uno::Reference<container::XNameAccess> m_xCfg;
    uno::Reference<beans::XPropertySet>    m_xData;

    template < typename ValueType >
    ValueType GetValue_Impl( UserOptToken nToken ) const;
    template < typename ValueType >
    void SetValue_Impl( UserOptToken nToken, ValueType const& rNewValue );
};

void SvtUserOptions::ChangeListener::changesOccurred (util::ChangesEvent const& rEvent)
{
    if (rEvent.Changes.hasElements())
        m_rParent.Notify();
}

void SvtUserOptions::ChangeListener::disposing (lang::EventObject const& rSource)
{
    try
    {
        uno::Reference<util::XChangesNotifier> xChgNot(rSource.Source, uno::UNO_QUERY_THROW);
        xChgNot->removeChangesListener(this);
    }
    catch (uno::Exception&)
    {
    }
}

SvtUserOptions::Impl::Impl() :
    m_xChangeListener( new ChangeListener(*this) )
{
    try
    {
        m_xCfg.set(
            comphelper::ConfigurationHelper::openConfig(
                comphelper::getProcessComponentContext(),
                "org.openoffice.UserProfile/Data",
                comphelper::EConfigurationModes::Standard
            ),
            uno::UNO_QUERY
        );

        m_xData.set(m_xCfg, uno::UNO_QUERY);
        uno::Reference<util::XChangesNotifier> xChgNot(m_xCfg, uno::UNO_QUERY);
        try
        {
            xChgNot->addChangesListener(m_xChangeListener);
        }
        catch (uno::RuntimeException&)
        {
        }
    }
    catch (uno::Exception const&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
        m_xCfg.clear();
    }
}

template < typename ValueType >
ValueType SvtUserOptions::Impl::GetValue_Impl (UserOptToken nToken) const
{
    ValueType sToken = ValueType();
    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(vOptionNames[nToken]) >>= sToken;
    }
    catch (uno::Exception const&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }
    return sToken;
}

template < typename ValueType >
void SvtUserOptions::Impl::SetValue_Impl (UserOptToken nToken, ValueType const& sToken)
{
    try
    {
        if (m_xData.is())
             m_xData->setPropertyValue(vOptionNames[nToken], uno::Any(sToken));
        comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch (uno::Exception const&)
    {
        DBG_UNHANDLED_EXCEPTION("unotools.config");
    }
}

OUString SvtUserOptions::Impl::GetToken (UserOptToken nToken) const
{
    return GetValue_Impl<OUString>( nToken );
}

void SvtUserOptions::Impl::SetToken (UserOptToken nToken, OUString const& sToken)
{
    SetValue_Impl<OUString>( nToken, sToken );
}

bool SvtUserOptions::Impl::GetBoolValue (UserOptToken nToken) const
{
    return GetValue_Impl<bool>( nToken );
}

void SvtUserOptions::Impl::SetBoolValue (UserOptToken nToken, bool bNewValue)
{
    SetValue_Impl<bool>( nToken, bNewValue );
}

OUString SvtUserOptions::Impl::GetFullName () const
{
    OUString sFullName;
    LanguageType const eLang = SvtSysLocale().GetUILanguageTag().getLanguageType();
    if (eLang == LANGUAGE_RUSSIAN)
    {
        sFullName = GetToken(UserOptToken::FirstName).trim();
        if (!sFullName.isEmpty())
            sFullName += " ";
        sFullName += o3tl::trim(GetToken(UserOptToken::FathersName));
        if (!sFullName.isEmpty())
            sFullName += " ";
        sFullName += o3tl::trim(GetToken(UserOptToken::LastName));
    }
    else
    {
        if (MsLangId::isFamilyNameFirst(eLang))
        {
            sFullName = GetToken(UserOptToken::LastName).trim();
            if (!sFullName.isEmpty())
                sFullName += " ";
            sFullName += o3tl::trim(GetToken(UserOptToken::FirstName));
        }
        else
        {
            sFullName = GetToken(UserOptToken::FirstName).trim();
            if (!sFullName.isEmpty())
                sFullName += " ";
            sFullName += o3tl::trim(GetToken(UserOptToken::LastName));
        }
    }
    sFullName = sFullName.trim();

    return sFullName;
}

void SvtUserOptions::Impl::Notify ()
{
    NotifyListeners(ConfigurationHints::NONE);
}

bool SvtUserOptions::Impl::IsTokenReadonly (UserOptToken nToken) const
{
    uno::Reference<beans::XPropertySet> xData(m_xCfg, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySetInfo> xInfo = xData->getPropertySetInfo();
    beans::Property aProp = xInfo->getPropertyByName(vOptionNames[nToken]);
    return ((aProp.Attributes & beans::PropertyAttribute::READONLY) ==
            beans::PropertyAttribute::READONLY);
}

static std::recursive_mutex& GetInitMutex()
{
    static std::recursive_mutex gMutex;
    return gMutex;
}


SvtUserOptions::SvtUserOptions ()
{
    // Global access, must be guarded (multithreading)
    std::unique_lock aGuard(GetInitMutex());

    xImpl = xSharedImpl.lock();
    if (!xImpl)
    {
        xImpl = std::make_shared<Impl>();
        xSharedImpl = xImpl;
        aGuard.unlock(); // because holdConfigItem will call this constructor
        ItemHolder1::holdConfigItem(EItem::UserOptions);
    }
    xImpl->AddListener(this);
}

SvtUserOptions::~SvtUserOptions()
{
    // Global access, must be guarded (multithreading)
    std::unique_lock aGuard( GetInitMutex() );
    xImpl->RemoveListener(this);
}

OUString SvtUserOptions::GetCompany        () const { return GetToken(UserOptToken::Company); }
OUString SvtUserOptions::GetFirstName      () const { return GetToken(UserOptToken::FirstName); }
OUString SvtUserOptions::GetLastName       () const { return GetToken(UserOptToken::LastName); }
OUString SvtUserOptions::GetID             () const { return GetToken(UserOptToken::ID); }
OUString SvtUserOptions::GetStreet         () const { return GetToken(UserOptToken::Street); }
OUString SvtUserOptions::GetCity           () const { return GetToken(UserOptToken::City); }
OUString SvtUserOptions::GetState          () const { return GetToken(UserOptToken::State); }
OUString SvtUserOptions::GetZip            () const { return GetToken(UserOptToken::Zip); }
OUString SvtUserOptions::GetCountry        () const { return GetToken(UserOptToken::Country); }
OUString SvtUserOptions::GetPosition       () const { return GetToken(UserOptToken::Position); }
OUString SvtUserOptions::GetTitle          () const { return GetToken(UserOptToken::Title); }
OUString SvtUserOptions::GetTelephoneHome  () const { return GetToken(UserOptToken::TelephoneHome); }
OUString SvtUserOptions::GetTelephoneWork  () const { return GetToken(UserOptToken::TelephoneWork); }
OUString SvtUserOptions::GetFax            () const { return GetToken(UserOptToken::Fax); }
OUString SvtUserOptions::GetEmail          () const { return GetToken(UserOptToken::Email); }
OUString SvtUserOptions::GetSigningKey     () const { return GetToken(UserOptToken::SigningKey); }
OUString SvtUserOptions::GetEncryptionKey  () const { return GetToken(UserOptToken::EncryptionKey); }

bool SvtUserOptions::IsTokenReadonly (UserOptToken nToken) const
{
    std::unique_lock aGuard(GetInitMutex());
    return xImpl->IsTokenReadonly(nToken);
}

OUString SvtUserOptions::GetToken (UserOptToken nToken) const
{
    std::unique_lock aGuard(GetInitMutex());
    return xImpl->GetToken(nToken);
}

void SvtUserOptions::SetToken (UserOptToken nToken, OUString const& rNewToken)
{
    std::unique_lock aGuard(GetInitMutex());
    xImpl->SetToken(nToken, rNewToken);
}

void SvtUserOptions::SetBoolValue (UserOptToken nToken, bool bNewValue)
{
    std::unique_lock aGuard(GetInitMutex());
    xImpl->SetBoolValue(nToken, bNewValue);
}

bool SvtUserOptions::GetEncryptToSelf() const
{
    std::unique_lock aGuard(GetInitMutex());
    return xImpl->GetBoolValue(UserOptToken::EncryptToSelf);
}

OUString SvtUserOptions::GetFullName () const
{
    std::unique_lock aGuard(GetInitMutex());
    return xImpl->GetFullName();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
