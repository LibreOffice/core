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

#include <unotools/useroptions.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
#include "itemholder1.hxx"

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

using namespace utl;
using namespace com::sun::star;

namespace
{

// vOptionNames[] -- names of the user option entries
// The order corresponds to the #define USER_OPT_* list in useroptions.hxx.
char const * const vOptionNames[] = {
    "l",                         // USER_OPT_CITY
    "o",                         // USER_OPT_COMPANY
    "c",                         // USER_OPT_COUNTRY
    "mail",                      // USER_OPT_EMAIL
    "facsimiletelephonenumber",  // USER_OPT_FAX
    "givenname",                 // USER_OPT_FIRSTNAME
    "sn",                        // USER_OPT_LASTNAME
    "position",                  // USER_OPT_POSITION
    "st",                        // USER_OPT_STATE
    "street",                    // USER_OPT_STREET
    "homephone",                 // USER_OPT_TELEPHONEHOME
    "telephonenumber",           // USER_OPT_TELEPHONEWORK
    "title",                     // USER_OPT_TITLE
    "initials",                  // USER_OPT_ID
    "postalcode",                // USER_OPT_ZIP
    "fathersname",               // USER_OPT_FATHERSNAME
    "apartment",                 // USER_OPT_APARTMENT
    "customernumber"             // USER_OPT_CUSTOMERNUMBER
};
const sal_uInt16 nOptionNameCount = SAL_N_ELEMENTS(vOptionNames);

} // namespace

boost::weak_ptr<SvtUserOptions::Impl> SvtUserOptions::pSharedImpl;

class SvtUserOptions::ChangeListener : public cppu::WeakImplHelper1<util::XChangesListener>
{
public:
    ChangeListener (Impl& rParent): m_rParent(rParent) { }

    // XChangesListener
    virtual void SAL_CALL changesOccurred (util::ChangesEvent const& Event) throw(uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XEventListener
    virtual void SAL_CALL disposing (lang::EventObject const& Source) throw(uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    Impl& m_rParent;
};

class SvtUserOptions::Impl : public utl::ConfigurationBroadcaster
{
public:
    Impl ();

    OUString GetFullName () const;

    bool IsTokenReadonly (sal_uInt16 nToken) const;
    OUString GetToken (sal_uInt16 nToken) const;
    void     SetToken (sal_uInt16 nToken, OUString const& rNewToken);
    void     Notify ();

private:
    uno::Reference<util::XChangesListener> m_xChangeListener;
    uno::Reference<container::XNameAccess> m_xCfg;
    uno::Reference<beans::XPropertySet>    m_xData;
};

void SvtUserOptions::ChangeListener::changesOccurred (util::ChangesEvent const& rEvent) throw(uno::RuntimeException, std::exception)
{
    if (rEvent.Changes.getLength())
        m_rParent.Notify();
}

void SvtUserOptions::ChangeListener::disposing (lang::EventObject const& rSource) throw(uno::RuntimeException, std::exception)
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
        m_xCfg = uno::Reference<container::XNameAccess>(
            comphelper::ConfigurationHelper::openConfig(
                comphelper::getProcessComponentContext(),
                "org.openoffice.UserProfile/Data",
                comphelper::ConfigurationHelper::E_STANDARD
            ),
            uno::UNO_QUERY
        );

        m_xData = uno::Reference<beans::XPropertySet>(m_xCfg, uno::UNO_QUERY);
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

OUString SvtUserOptions::Impl::GetToken (sal_uInt16 nToken) const
{
    OUString sToken;
    if (nToken < nOptionNameCount)
    {
        try
        {
            if (m_xData.is())
                m_xData->getPropertyValue(OUString::createFromAscii(vOptionNames[nToken])) >>= sToken;
        }
        catch (uno::Exception const& ex)
        {
            SAL_WARN("unotools.config", "Caught unexpected: " << ex.Message);
        }
    }
    else
        SAL_WARN("unotools.config", "SvtUserOptions::Impl::GetToken(): invalid token");
    return sToken;
}

void SvtUserOptions::Impl::SetToken (sal_uInt16 nToken, OUString const& sToken)
{
    if (nToken < nOptionNameCount)
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
    else
        SAL_WARN("unotools.config", "SvtUserOptions::Impl::GetToken(): invalid token");
}

OUString SvtUserOptions::Impl::GetFullName () const
{
    OUString sFullName;
    switch (LanguageType const eLang = SvtSysLocale().GetUILanguageTag().getLanguageType())
    {
        case LANGUAGE_RUSSIAN:
            sFullName = GetToken(USER_OPT_FIRSTNAME).trim();
            if (!sFullName.isEmpty())
                sFullName += " ";
            sFullName += GetToken(USER_OPT_FATHERSNAME).trim();
            if (!sFullName.isEmpty())
                sFullName += " ";
            sFullName += GetToken(USER_OPT_LASTNAME).trim();
            break;
        default:
            if (MsLangId::isFamilyNameFirst(eLang))
            {
                sFullName = GetToken(USER_OPT_LASTNAME).trim();
                if (!sFullName.isEmpty())
                    sFullName += " ";
                sFullName += GetToken(USER_OPT_FIRSTNAME).trim();
            }
            else
            {
                sFullName = GetToken(USER_OPT_FIRSTNAME).trim();
                if (!sFullName.isEmpty())
                    sFullName += " ";
                sFullName += GetToken(USER_OPT_LASTNAME).trim();
            }
            break;
    }

    return sFullName;
}

void SvtUserOptions::Impl::Notify ()
{
    NotifyListeners(0);
}

bool SvtUserOptions::Impl::IsTokenReadonly (sal_uInt16 nToken) const
{
    if (nToken < nOptionNameCount)
    {
        uno::Reference<beans::XPropertySet> xData(m_xCfg, uno::UNO_QUERY);
        uno::Reference<beans::XPropertySetInfo> xInfo = xData->getPropertySetInfo();
        beans::Property aProp = xInfo->getPropertyByName(OUString::createFromAscii(vOptionNames[nToken]));
        return ((aProp.Attributes & beans::PropertyAttribute::READONLY) ==
            beans::PropertyAttribute::READONLY);
    }
    else
    {
        SAL_WARN("unotools.config", "SvtUserOptions::Impl::IsTokenReadonly(): invalid token");
        return false;
    }
}

SvtUserOptions::SvtUserOptions ()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard(GetInitMutex());

    if (pSharedImpl.expired())
    {
        pImpl.reset(new Impl);
        pSharedImpl = pImpl;
        ItemHolder1::holdConfigItem(E_USEROPTIONS);
    }
    pImpl = pSharedImpl.lock();
    pImpl->AddListener(this);
}

SvtUserOptions::~SvtUserOptions()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard( GetInitMutex() );
    pImpl->RemoveListener(this);
}

namespace
{
    class theUserOptionsMutex : public rtl::Static<osl::Mutex, theUserOptionsMutex>{};
}

osl::Mutex& SvtUserOptions::GetInitMutex()
{
    return theUserOptionsMutex::get();
}

OUString SvtUserOptions::GetCompany        () const { return GetToken(USER_OPT_COMPANY); }
OUString SvtUserOptions::GetFirstName      () const { return GetToken(USER_OPT_FIRSTNAME); }
OUString SvtUserOptions::GetLastName       () const { return GetToken(USER_OPT_LASTNAME); }
OUString SvtUserOptions::GetID             () const { return GetToken(USER_OPT_ID); }
OUString SvtUserOptions::GetStreet         () const { return GetToken(USER_OPT_STREET); }
OUString SvtUserOptions::GetCity           () const { return GetToken(USER_OPT_CITY); }
OUString SvtUserOptions::GetState          () const { return GetToken(USER_OPT_STATE); }
OUString SvtUserOptions::GetZip            () const { return GetToken(USER_OPT_ZIP); }
OUString SvtUserOptions::GetCountry        () const { return GetToken(USER_OPT_COUNTRY); }
OUString SvtUserOptions::GetPosition       () const { return GetToken(USER_OPT_POSITION); }
OUString SvtUserOptions::GetTitle          () const { return GetToken(USER_OPT_TITLE); }
OUString SvtUserOptions::GetTelephoneHome  () const { return GetToken(USER_OPT_TELEPHONEHOME); }
OUString SvtUserOptions::GetTelephoneWork  () const { return GetToken(USER_OPT_TELEPHONEWORK); }
OUString SvtUserOptions::GetFax            () const { return GetToken(USER_OPT_FAX); }
OUString SvtUserOptions::GetEmail          () const { return GetToken(USER_OPT_EMAIL); }
OUString SvtUserOptions::GetCustomerNumber () const { return GetToken(USER_OPT_CUSTOMERNUMBER); }

void SvtUserOptions::SetCustomerNumber (OUString const& sToken) { SetToken(USER_OPT_CUSTOMERNUMBER, sToken); }

bool SvtUserOptions::IsTokenReadonly (sal_uInt16 nToken) const
{
    osl::MutexGuard aGuard(GetInitMutex());
    return pImpl->IsTokenReadonly(nToken);
}

OUString SvtUserOptions::GetToken (sal_uInt16 nToken) const
{
    osl::MutexGuard aGuard(GetInitMutex());
    return pImpl->GetToken(nToken);
}

void SvtUserOptions::SetToken (sal_uInt16 nToken, OUString const& rNewToken)
{
    osl::MutexGuard aGuard(GetInitMutex());
    pImpl->SetToken(nToken, rNewToken);
}

OUString SvtUserOptions::GetFullName () const
{
    osl::MutexGuard aGuard(GetInitMutex());
    return pImpl->GetFullName();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
