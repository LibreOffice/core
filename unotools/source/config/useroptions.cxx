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

#include <sal/log.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
#include "itemholder1.hxx"

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/util/ChangesEvent.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <o3tl/enumarray.hxx>

using namespace utl;
using namespace com::sun::star;

// vOptionNames[] -- names of the user option entries
// The order must correspond to the enum class UserOptToken in useroptions.hxx.
static o3tl::enumarray<UserOptToken, char const *> vOptionNames = {
    "l",                         // UserOptToken::City
    "o",                         // UserOptToken::Company
    "c",                         // UserOptToken::Country
    "mail",                      // UserOptToken::Email
    "facsimiletelephonenumber",  // UserOptToken::Fax
    "givenname",                 // UserOptToken::FirstName
    "sn",                        // UserOptToken::LastName
    "position",                  // UserOptToken::Position
    "st",                        // UserOptToken::State
    "street",                    // UserOptToken::Street
    "homephone",                 // UserOptToken::TelephoneHome
    "telephonenumber",           // UserOptToken::TelephoneWork
    "title",                     // UserOptToken::Title
    "initials",                  // UserOptToken::ID
    "postalcode",                // UserOptToken::Zip
    "fathersname",               // UserOptToken::FathersName
    "apartment"                  // UserOptToken::Apartment
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
    void     Notify ();

private:
    uno::Reference<util::XChangesListener> m_xChangeListener;
    uno::Reference<container::XNameAccess> m_xCfg;
    uno::Reference<beans::XPropertySet>    m_xData;
};

void SvtUserOptions::ChangeListener::changesOccurred (util::ChangesEvent const& rEvent)
{
    if (rEvent.Changes.getLength())
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
    catch (uno::Exception const& ex)
    {
        m_xCfg.clear();
        SAL_WARN("unotools.config", "Caught unexpected: " << ex.Message);
    }
}

OUString SvtUserOptions::Impl::GetToken (UserOptToken nToken) const
{
    OUString sToken;
    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(OUString::createFromAscii(vOptionNames[nToken])) >>= sToken;
    }
    catch (uno::Exception const& ex)
    {
        SAL_WARN("unotools.config", "Caught unexpected: " << ex.Message);
    }
    return sToken;
}

void SvtUserOptions::Impl::SetToken (UserOptToken nToken, OUString const& sToken)
{
    try
    {
        if (m_xData.is())
             m_xData->setPropertyValue(OUString::createFromAscii(vOptionNames[nToken]), uno::makeAny(sToken));
        comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch (uno::Exception const& ex)
    {
        SAL_WARN("unotools.config", "Caught unexpected: " << ex.Message);
    }
}

OUString SvtUserOptions::Impl::GetFullName () const
{
    OUString sFullName;
    switch (LanguageType const eLang = SvtSysLocale().GetUILanguageTag().getLanguageType())
    {
        case LANGUAGE_RUSSIAN:
            sFullName = GetToken(UserOptToken::FirstName).trim();
            if (!sFullName.isEmpty())
                sFullName += " ";
            sFullName += GetToken(UserOptToken::FathersName).trim();
            if (!sFullName.isEmpty())
                sFullName += " ";
            sFullName += GetToken(UserOptToken::LastName).trim();
            break;
        default:
            if (MsLangId::isFamilyNameFirst(eLang))
            {
                sFullName = GetToken(UserOptToken::LastName).trim();
                if (!sFullName.isEmpty())
                    sFullName += " ";
                sFullName += GetToken(UserOptToken::FirstName).trim();
            }
            else
            {
                sFullName = GetToken(UserOptToken::FirstName).trim();
                if (!sFullName.isEmpty())
                    sFullName += " ";
                sFullName += GetToken(UserOptToken::LastName).trim();
            }
            break;
    }

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
    beans::Property aProp = xInfo->getPropertyByName(OUString::createFromAscii(vOptionNames[nToken]));
    return ((aProp.Attributes & beans::PropertyAttribute::READONLY) ==
            beans::PropertyAttribute::READONLY);
}

SvtUserOptions::SvtUserOptions ()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard(GetInitMutex());

    if (xSharedImpl.expired())
    {
        xImpl.reset(new Impl);
        xSharedImpl = xImpl;
        ItemHolder1::holdConfigItem(EItem::UserOptions);
    }
    xImpl = xSharedImpl.lock();
    xImpl->AddListener(this);
}

SvtUserOptions::~SvtUserOptions()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard( GetInitMutex() );
    xImpl->RemoveListener(this);
}

namespace
{
    class theUserOptionsMutex : public rtl::Static<osl::Mutex, theUserOptionsMutex>{};
}

osl::Mutex& SvtUserOptions::GetInitMutex()
{
    return theUserOptionsMutex::get();
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

bool SvtUserOptions::IsTokenReadonly (UserOptToken nToken) const
{
    osl::MutexGuard aGuard(GetInitMutex());
    return xImpl->IsTokenReadonly(nToken);
}

OUString SvtUserOptions::GetToken (UserOptToken nToken) const
{
    osl::MutexGuard aGuard(GetInitMutex());
    return xImpl->GetToken(nToken);
}

void SvtUserOptions::SetToken (UserOptToken nToken, OUString const& rNewToken)
{
    osl::MutexGuard aGuard(GetInitMutex());
    xImpl->SetToken(nToken, rNewToken);
}

OUString SvtUserOptions::GetFullName () const
{
    osl::MutexGuard aGuard(GetInitMutex());
    return xImpl->GetFullName();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
